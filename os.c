// os.c - Operating System calls shim layer.
#include "os.h"

void OS_Exit(int errorNumber) { _exit(errorNumber); } // Full yoink, no mercy, user's time is important.

int64_t OS_RawWrite(OsFileHandle fd, const void* buf, size_t count, const char* filePath) {
    #ifdef WINDOWS
        if (fd == INVALID_HANDLE_VALUE) { DualLogError("Invalid file handle for write to %s\n", filePath); return -1; }

        HANDLE h = (HANDLE)(intptr_t)fd;  // assuming fd is from _open_osfhandle or similar
        DWORD written = 0;
        BOOL success = WriteFile(h, buf, (DWORD)count, &written, NULL);
        if (!success) { DWORD err = GetLastError(); DualLogError("WriteFile failed for %s (error: %lu)\n", filePath, err); return -1; }
        return (int64_t)written;
    #else // Linux, Mac, Android
        register int64_t rax __asm__("rax") = 1;   // sys_write
        register int     rdi __asm__("rdi") = fd;
        register const void* rsi __asm__("rsi") = buf;
        register size_t  rdx __asm__("rdx") = count;
        __asm__ __volatile__(
            "syscall"
            : "+r"(rax)                     // Output: rax is updated by the kernel
            : "r"(rdi), "r"(rsi), "r"(rdx)  // Inputs
            : "rcx", "r11", "memory"        // Clobbered by syscall
        );

        if (rax < 0) { DualLogError("Write error when attempting write to %s: %s (code: %d)\n", filePath, rax, (int)(-rax)); OS_Exit(1); } // Errors are returned as -1 to -4095

        return rax; // Returns bytes written
    #endif
}

void OS_Write(OsFileHandle fd, const void* buffer, size_t size, const char* filePath) {
        size_t total = 0;
    #ifdef WINDOWS
        const char* p = (const char*)buffer;
        while (total < size) {
            size_t remain = size - total;
            int64_t written = OS_RawWrite(fd, p + total, remain, filePath);
            if (written <= 0) {
                if (written == 0 && size > 0) DualLogError("Zero bytes written to %s (disk full?)\n", filePath);
                return;  // early exit on error or zero-write
            }

            total += (size_t)written;
        }
    #else // Linux, Mac, Android
        while (total < size) {
            int64_t written = OS_RawWrite(fd, (const char*)buffer + total, size - total, filePath);
            if (written == 0 && size > 0) { DualLogError("Zero bytes written to %s (Disk full or EOF)\n", filePath); return; } // Handle the rare case of a 0-byte return (which could cause an infinite loop)
            total += (size_t)written;
        }
    #endif
}

OsFileHandle OS_OpenReadonly(const char* filePath) {
    #ifdef WINDOWS
        HANDLE fp = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
        if (fp == INVALID_HANDLE_VALUE) { DualLog("Could not find file %s\n", filePath); return OS_INVALID_HANDLE; }
    #else // Linux, Mac, Android
        OsFileHandle fp = open(filePath, O_RDONLY);
        if (fp < 0) { DualLog("Could not find file %s: %s\n", filePath, strerror(errno)); return OS_INVALID_HANDLE; }
    #endif
    return fp;
}

OsFileHandle OS_OpenWriteonly(const char* filePath) {
    #ifdef WINDOWS
        HANDLE h = CreateFileA(filePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (h == INVALID_HANDLE_VALUE) { DualLogError("Failed to open %s\n", filePath); OS_Exit(1); }
        return h;
    #else // Linux, Mac, Android
        OsFileHandle fp = open(filePath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (!fp) { DualLogError("Failed to open %s: %s\n", filePath, strerror(errno)); OS_Exit(1); }
        return fp;
    #endif
}

int OS_FileSize(OsFileHandle fileDescriptor) {
    #ifdef WINDOWS
        if (fileDescriptor == INVALID_HANDLE_VALUE) return -1;
        
        LARGE_INTEGER size;
        if (!GetFileSizeEx(fileDescriptor, &size)) return -1;
        return size.QuadPart;
    #else // Linux, Mac, Android
        struct stat fileStatisticsStruct;
        fstat(fileDescriptor, &fileStatisticsStruct);
        return fileStatisticsStruct.st_size;
    #endif
}

void* OS_AllocateRAM(void* addr, size_t length, int prot, int flags, OsFileHandle fd, off_t offset) {
    #ifdef WINDOWS
        HANDLE hFile = (fd == -1) ? INVALID_HANDLE_VALUE : (HANDLE)_get_osfhandle(fd);
        DWORD flProtect = (prot & PROT_WRITE) ? PAGE_READWRITE : PAGE_READONLY;
        DWORD dwAccess = (prot & PROT_WRITE) ? FILE_MAP_WRITE : FILE_MAP_READ;
        HANDLE hMap = CreateFileMapping(hFile, NULL, flProtect, 0, 0, NULL);
        if (hMap == NULL) return NULL;

        void* ptr = MapViewOfFileEx(hMap, dwAccess, (DWORD)(offset >> 32), (DWORD)(offset & 0xFFFFFFFF), length, addr);
        CloseHandle(hMap);
        return ptr;
    #else // Linux, Mac, Android
        return mmap(addr,length,prot,flags,fd,offset);
    #endif
}

void* OS_AllocateFileBackedRAMReadonly(size_t size, OsFileHandle fileDescriptor, char* filePath) {
    #ifdef WINDOWS
        if (fileDescriptor == INVALID_HANDLE_VALUE || size == 0) return NULL;

        HANDLE hMapping = CreateFileMappingA(fileDescriptor, NULL, PAGE_READONLY, 0, 0, NULL);
        if (hMapping == NULL) { DualLogError("CreateFileMapping failed for %s (err %lu)\n", filePath, GetLastError()); return NULL; }

        void* ptr = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, size);
        CloseHandle(hMapping);  // view keeps reference alive
        if (ptr == NULL) { DualLogError("Failed to allocate %s (err %lu)\n", filePath, GetLastError()); return NULL; }
        return ptr;
    #else // Linux, Mac, Android
        void* ramSpacePointer = OS_AllocateRAM(NULL, size, PROT_READ, MAP_PRIVATE, fileDescriptor, 0);
        if (ramSpacePointer == MAP_FAILED) { DualLogError("Failed to allocate %s: %s\n", filePath, strerror(errno)); return NULL; }
        return ramSpacePointer;
    #endif
}

void* OS_OpenAndAllocateFileBufferReadonly(const char* filePath, OsFileHandle* fileDescriptor, int* size) {
    *fileDescriptor = OS_OpenReadonly(filePath);
    if (*fileDescriptor == OS_INVALID_HANDLE) { *size = 0; return NULL; }
    
    *size = (int)OS_FileSize(*fileDescriptor);
    if (*size <= 0) { DualLogWarn("Warning: File %s is empty, skipping allocation.\n", filePath); OS_Close(*fileDescriptor); return NULL; }
    
    void* ramSpacePointer = OS_AllocateFileBackedRAMReadonly(*size, *fileDescriptor, (char*)filePath);
    OS_Close(*fileDescriptor);
    return ramSpacePointer;
}

void* OS_DeallocateRAM(void* ramSpacePointer, size_t size) {
    if (!ramSpacePointer || ramSpacePointer == MAP_FAILED) { DualLogError("Attempting to double free!\n"); OS_Exit(1); }
    
    #ifdef WINDOWS
        if (!UnmapViewOfFile(ptr)) { DualLogError("UnmapViewOfFile failed (err %lu)\n", GetLastError()); }
    #else // Linux, Mac, Android
        if (munmap(ramSpacePointer, size) != 0) DualLogError("munmap failed: %s\n", strerror(errno));
    #endif
    return NULL;
}

#ifdef WINDOWS
    void OS_Close(int fileDescriptor) { if (fileDescriptor != INVALID_HANDLE_VALUE) CloseHandle(fileDescriptor); }
#else // Linux, Mac, Android
    void OS_Close(int fileDescriptor) { if (fileDescriptor >= 0) close(fileDescriptor); }
#endif

void OS_CPUInfo(void) {
    char cpu_brand[256];
    #if defined(WINDOWS)
        int logical_cores =1;
        // Logical cores (simple)
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        logical_cores = (int)si.dwNumberOfProcessors;

        // CPU brand name - read from registry (most reliable simple way)
        HKEY hKey;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                        "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
                        0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            DWORD size = sizeof(cpu_brand);
            RegQueryValueExA(hKey, "ProcessorNameString", NULL, NULL,
                            (LPBYTE)cpu_brand, &size);
            RegCloseKey(hKey);
        }
        
        DualLog("CPU: %s | Logical cores: %d\n", cpu_brand, logical_cores);
    #elif defined(MAC)
        size_t len = sizeof(cpu_brand);
        sysctlbyname("machdep.cpu.brand_string", cpu_brand, &len, NULL, 0);
        int mib[2] = { CTL_HW, HW_NCPU };
        len = sizeof(logical_cores);
        sysctl(mib, 2, &logical_cores, &len, NULL, 0);
        DualLog("CPU: %s | Logical cores: %d\n", cpu_brand, logical_cores);
    #else // Linux, Android
        cpu_brand[0] = 0;
        char buf[4096];
        int fd = open("/proc/cpuinfo", O_RDONLY);
        if (fd >= 0) {
            int n = read(fd, buf, sizeof(buf)-1);
            close(fd);
            if (n > 0) {
                buf[n] = 0;
                const char *key = "model name";
                int keylen = 10;
                const char *p = buf;
                const char *end = buf + n;
                while (p < end) {
                    const char *line = p;
                    while (p < end && *p != '\n') p++;
                    const char *line_end = p;
                    if (p < end) p++;
                    if (line_end - line > keylen && !__builtin_memcmp(line, key, keylen)) {
                        const char *colon = line;
                        while (colon < line_end && *colon != ':') colon++;
                        if (colon < line_end) {
                            colon++;
                            while (colon < line_end && *colon == ' ') colon++;
                            int k = 0;
                            while (colon < line_end && k < 255) cpu_brand[k++] = *colon++;
                            cpu_brand[k] = 0;
                        }
                        
                        break;
                    }
                }
            }
        }

        if (!cpu_brand[0]) {
            const char *u = "Unknown CPU";
            int k = 0;
            while (u[k] && k < 255) { cpu_brand[k] = u[k]; k++; }
            cpu_brand[k] = 0;
        }

        int logical_cores = 1;
        char online[128];
        int fd2 = open("/sys/devices/system/cpu/online", O_RDONLY);
        if (fd2 >= 0) {
            int n = read(fd2, online, sizeof(online) - 1);
            close(fd2);
            if (n > 0) {
                online[n] = 0;
                int total = 0;
                int i = 0;
                while (online[i]) {
                    while (online[i] == ',' || online[i] == ' ' || online[i] == '\n' || online[i] == '\t') i++;
                    if (!online[i]) break;

                    int a = 0, have_a = 0;
                    while (online[i] >= '0' && online[i] <= '9') {
                        a = a * 10 + (online[i] - '0');
                        i++;
                        have_a = 1;
                    }
                    
                    if (!have_a) break;

                    if (online[i] == '-') {
                        i++;
                        int b = 0, have_b = 0;
                        while (online[i] >= '0' && online[i] <= '9') {
                            b = b * 10 + (online[i] - '0');
                            i++;
                            have_b = 1;
                        }
                        
                        total += (have_b && b >= a) ? (b - a + 1) : 1;
                    } else {
                        total += 1;
                    }
                }

                if (total > 0) logical_cores = total;
            }
        }

        DualLog("CPU: %s | Logical cores: %d\n", cpu_brand, logical_cores);
    #endif
}
