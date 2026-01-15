// os.h - starts most translation units and defines the shim layer between Voxen and the OS as well as defining project wide OS defines.
#pragma once
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
void DualLog(const char* fmt, ...);
void DualLogWarn(const char* fmt, ...);
void DualLogError(const char* fmt, ...);
typedef struct {
    uint64_t mtime_ns;
    uint64_t size;
    uint64_t inode;
    uint64_t dev;
} FileFingerprint;

#define OS_Exit(x) _exit( (x) )
#if defined(_WIN32) || defined(_WIN64)
    #define WINDOWS
    #define WIN32_LEAN_AND_MEAN // Let 'er rip, tater chip
    #include <windows.h> // The things I do for my players, yeesh
    #include <direct.h>
    #include <io.h>
    typedef HANDLE OsFileHandle;
    #define OS_INVALID_HANDLE INVALID_HANDLE_VALUE
    #define PROT_READ  0x1 // From mman.h
    #define PROT_WRITE 0x2
    #define MAP_PRIVATE 0x02
    #define MAP_ANONYMOUS 0x20
    #define MAP_POPULATE 0x08000

    #define OS_MakeFolder(path) _mkdir(path)
    static inline void OS_Close(OsFileHandle fileDescriptor) { CloseHandle(fileDescriptor); }
    
    static inline void* OS_AllocateRAM(void* addr, size_t length, int prot, int flags, OsFileHandle fd) {
        (void)flags;
        bool writable = (prot & PROT_WRITE);
        if (fd == INVALID_HANDLE_VALUE) {
            void* ptr;
            if ((ptr = (void*)VirtualAlloc(addr, length, MEM_RESERVE | MEM_COMMIT, writable ? PAGE_READWRITE : PAGE_READONLY)) == NULL) { DualLogError("Failed to allocate RAM\n"); OS_Exit(1); } // Handle standard memory allocations (MAP_ANONYMOUS)
            return ptr;
        }

        // Handle File-Backed Memory
        HANDLE hMap = CreateFileMapping(fd, NULL, writable ? PAGE_READWRITE : PAGE_READONLY, (DWORD)((uint64_t)length >> 32), (DWORD)((uint64_t)length & 0xFFFFFFFF), NULL);
        if (hMap == NULL) { DualLogError("Failed to allocate RAM\n"); OS_Exit(1); }

        void* ptr = MapViewOfFileEx(hMap, writable ? FILE_MAP_WRITE : FILE_MAP_READ, 0, 0, length, addr);
        CloseHandle(hMap);
        return ptr;
    }
#else
    #if defined(__linux__) && !defined(__ANDROID__)
        #define LINUX
    #endif

    #if defined(__ANDROID__)
        #define ANDROID
    #endif
    
    #if defined(__APPLE__)
        #define MAC
    #endif
    typedef int OsFileHandle;
    #define OS_INVALID_HANDLE -1
    #include <sys/stat.h>
    #include <sys/mman.h>
    #include <fcntl.h>
    #include <unistd.h>
    #include <errno.h>
    #include <string.h>
    
    #define OS_MakeFolder(path) mkdir(path, 0755)
    static inline void OS_Close(OsFileHandle fileDescriptor) { close(fileDescriptor); }

    static inline void* OS_AllocateRAM(void* addr, size_t length, int prot, int flags, OsFileHandle fd) {
        void* ptr = mmap(addr,length,prot,flags,fd,0);
        if (ptr == MAP_FAILED || ptr == NULL) { DualLogError("Failed to allocate RAM\n"); OS_Exit(1); }
        return ptr;
    }
#endif

static inline int64_t OS_RawWrite(OsFileHandle fd, const void* buf, size_t count, const char* filePath) {
    #ifdef WINDOWS
        DWORD written = 0;
        if (WriteFile((HANDLE)fd, buf, (DWORD)count, &written, NULL)) return (int64_t)written;
        DualLogError("Write failed for %s (error: %lu)\n", filePath, GetLastError()); return -1;
    #else // Linux, Mac, Android
        register int64_t     rax __asm__("rax") = 1; // sys_write
        register int         rdi __asm__("rdi") = fd;
        register const void* rsi __asm__("rsi") = buf;
        register size_t      rdx __asm__("rdx") = count;
        __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi), "r"(rsi), "r"(rdx) : "rcx", "r11", "memory");
        if (rax >= 0) return rax;
        DualLogError("Write error when attempting write to %s: %s (code: %d)\n", filePath, rax, (int)(-rax)); return -1;
    #endif
}

static inline void OS_Write(OsFileHandle fd, const void* buffer, size_t size, const char* filePath) {
    size_t total = 0;
    while (total < size) {
        int64_t written = OS_RawWrite(fd, (const char*)buffer + total, size - total, filePath);
        if (written <= 0) return;
        else total += (size_t)written;
    }
}

static inline OsFileHandle OS_OpenReadonly(const char* filePath) {
    #ifdef WINDOWS
        HANDLE fp = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
        if (fp == OS_INVALID_HANDLE) { DualLog("Could not open file %s\n", filePath); return OS_INVALID_HANDLE; }
    #else // Linux, Mac, Android
        OsFileHandle fp = open(filePath, O_RDONLY);
        if (fp < 0) { DualLog("Could not open file %s: %s\n", filePath, strerror(errno)); return OS_INVALID_HANDLE; }
    #endif
    return fp;
}

static inline OsFileHandle OS_OpenWriteonly(const char* filePath) {
    #ifdef WINDOWS
        OsFileHandle h = CreateFileA(filePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        return (h == OS_INVALID_HANDLE) ? (DualLogError("Failed to open %s\n", filePath), OS_Exit(1), OS_INVALID_HANDLE) : h;
    #else // Linux, Mac, Android
        OsFileHandle h = open(filePath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        return (h < 0) ? (DualLogError("Failed to open %s: %s\n", filePath, strerror(errno)), OS_Exit(1), OS_INVALID_HANDLE) : h;
    #endif
}

static inline int OS_FileSize(OsFileHandle fileDescriptor) {
    #ifdef WINDOWS
        if (fileDescriptor == OS_INVALID_HANDLE) return -1;
        
        LARGE_INTEGER size;
        if (!GetFileSizeEx(fileDescriptor, &size)) return -1;
        return size.QuadPart;
    #else // Linux, Mac, Android
        struct stat fileStatisticsStruct;
        fstat(fileDescriptor, &fileStatisticsStruct);
        return fileStatisticsStruct.st_size;
    #endif
}

static inline void* OS_AllocateFileBackedRAMReadonly(size_t size, OsFileHandle fileDescriptor, char* filePath) {
    #ifdef WINDOWS
        if (fileDescriptor == OS_INVALID_HANDLE || size == 0) return NULL;

        HANDLE hMapping = CreateFileMappingA(fileDescriptor, NULL, PAGE_READONLY, 0, 0, NULL);
        if (hMapping == NULL) { DualLogError("CreateFileMapping failed for %s (err %lu)\n", filePath, GetLastError()); return NULL; }

        void* ramSpacePointer = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, size);
        CloseHandle(hMapping);  // view keeps reference alive
        if (ramSpacePointer == NULL) { DualLogError("Failed to allocate %s (err %lu)\n", filePath, GetLastError()); return NULL; }
    #else // Linux, Mac, Android
        void* ramSpacePointer = OS_AllocateRAM(NULL, size, PROT_READ, MAP_PRIVATE, fileDescriptor);
        if (ramSpacePointer == MAP_FAILED) { DualLogError("Failed to allocate %s: %s\n", filePath, strerror(errno)); return NULL; }
    #endif
    return ramSpacePointer;
}

static inline void* OS_OpenAndAllocateFileBufferReadonly(const char* filePath, OsFileHandle* fileDescriptor, int* size) {
    *fileDescriptor = OS_OpenReadonly(filePath);
    if (*fileDescriptor == OS_INVALID_HANDLE) { *size = 0; return NULL; }
    
    *size = (int)OS_FileSize(*fileDescriptor);
    if (*size <= 0) { DualLogWarn("Warning: File %s is empty, skipping allocation.\n", filePath); OS_Close(*fileDescriptor); return NULL; }
    
    void* ramSpacePointer = OS_AllocateFileBackedRAMReadonly(*size, *fileDescriptor, (char*)filePath);
    OS_Close(*fileDescriptor);
    return ramSpacePointer;
}

static inline void OS_DeallocateRAM(void* ramSpacePointer, size_t size) {    
    #ifdef WINDOWS
        (void)size;
        if (!ramSpacePointer) { DualLogError("Attempting to double free!\n"); OS_Exit(1); }
        
        if (!UnmapViewOfFile(ramSpacePointer)) { if (VirtualFree(ramSpacePointer, 0 /* dwSize (must be 0 with MEM_RELEASE) */, MEM_RELEASE) == 0) DualLogError("VirtualFree failed\n"); }
    #else // Linux, Mac, Android
        if (!ramSpacePointer || ramSpacePointer == MAP_FAILED) { DualLogError("Attempting to double free!\n"); OS_Exit(1); }
        
        if ((void *)(int64_t)munmap(ramSpacePointer, size) == MAP_FAILED) DualLogError("munmap failed: %s\n", strerror(errno));
    #endif
}

static inline void OS_CPUInfo(void) {
    char brand[256] = "Unknown CPU";
    int cores = 1;
    #if defined(_WIN32)
        SYSTEM_INFO si; GetSystemInfo(&si);
        cores = si.dwNumberOfProcessors;
        HKEY k;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &k) == 0) {
            DWORD s = sizeof(brand);
            RegQueryValueExA(k, "ProcessorNameString", NULL, NULL, (LPBYTE)brand, &s);
            RegCloseKey(k);
        }
    #elif defined(__APPLE__)
        size_t s = sizeof(brand), c = sizeof(cores);
        sysctlbyname("machdep.cpu.brand_string", brand, &s, NULL, 0);
        sysctlbyname("hw.logicalcpu", &cores, &c, NULL, 0);
    #else // Linux / Android
        cores = sysconf(_SC_NPROCESSORS_ONLN); // One-liner for Linux cores
        OsFileHandle fd = open("/proc/cpuinfo", O_RDONLY);
        if (fd >= 0) {
            char buf[4096];
            int n = read(fd, buf, sizeof(buf)-1);
            OS_Close(fd);
            if (n > 0) {
                buf[n] = 0;
                char* f = strstr(buf, "model name");
                if (f && (f = strchr(f, ':'))) {
                    f += 2; // Skip ": "
                    int i = 0;
                    while (f[i] && f[i] != '\n' && i < 255) { brand[i] = f[i]; i++; }
                    brand[i] = 0;
                }
            }
        }
    #endif
    DualLog("CPU: %s | Cores: %d\n", brand, cores);
}

static inline uint64_t mix64(uint64_t x) {
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;
    return x;
}

// static inline uint64_t OS_GetFilestamp(const FileFingerprint *fp) { uint64_t h = 0; h ^= mix64(fp->mtime_ns); h ^= mix64(fp->size); h ^= mix64(fp->inode); h ^= mix64(fp->dev); return h; }
static inline uint64_t OS_GetFilestamp(const FileFingerprint *fp) { return mix64(fp->size); }

static inline bool OS_GetFileFingerprint(const char *path, FileFingerprint *fp) {
    #ifdef _WIN32
        // Use CreateFile to get a handle (required for detailed file info)
        HANDLE hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == OS_INVALID_HANDLE) return false;

        BY_HANDLE_FILE_INFORMATION bhfi;
        if (!GetFileInformationByHandle(hFile, &bhfi)) { CloseHandle(hFile); return false; }

        fp->size     = ((uint64_t)bhfi.nFileSizeHigh << 32) | bhfi.nFileSizeLow;
        CloseHandle(hFile);
    #else // Linux, Mac, Android
        struct stat st;
        if (stat(path, &st) != 0) return false;

        fp->size  = (uint64_t)st.st_size;
    #endif
    return true;
}
