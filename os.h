// os.h - starts most translation units and defines the shim layer between Voxen and the OS as well as defining project wide OS defines.
#pragma once
typedef __INT16_TYPE__ int16_t;
typedef __INT32_TYPE__ int32_t;
typedef __UINT8_TYPE__ uint8_t;
typedef __UINT16_TYPE__ uint16_t;
typedef __UINT32_TYPE__ uint32_t;
typedef __INT64_TYPE__ int64_t;
typedef __UINT64_TYPE__ uint64_t;
typedef __SIZE_TYPE__ size_t;
typedef __UINTPTR_TYPE__ uintptr_t;
typedef __INTPTR_TYPE__ intptr_t;
#define bool _Bool
#define true 1
#define false 0
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#ifndef NULL
    #define NULL 0
#endif
#define PROT_READ  0x1 // From mman.h
#define PROT_WRITE 0x2
#define PROT_EXEC  0x4
#define MAP_PRIVATE 0x02
#define MAP_FIXED	0x10
#define MAP_ANONYMOUS 0x20
#define MAP_POPULATE 0x08000
#define MAP_FAILED   ((void *)-1)
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
// Interop - To Mod (keep the same as interop.h!!)
#if defined(_WIN32) || defined(__CYGWIN__)
    #define ENGINE_TO_MOD __declspec(dllexport)
#else
    #define ENGINE_TO_MOD __attribute__((visibility("default")))
#endif
ENGINE_TO_MOD void DualLogError(const char* fmt, ...);
char* StringFindSubstring(const char* haystack, const char* needle);
ENGINE_TO_MOD char* StringFindFirstCharWithin(const char *s, char c);
typedef struct {
    uint64_t mtime_ns;
    uint64_t size;
    uint64_t inode;
    uint64_t dev;
} FileFingerprint;

#if defined(_WIN32) || defined(_WIN64)
    #define WINDOWS
    #define WIN32_LEAN_AND_MEAN // Let 'er rip, tater chip
    #include <windows.h> // The things I do for my players, yeesh
    #include <direct.h>
    #include <winternl.h>
    #include <ntstatus.h>
    #include <io.h>
    typedef HANDLE OsFileHandle;
    #define OS_INVALID_HANDLE INVALID_HANDLE_VALUE
    #define OS_MakeFolder(path) _mkdir(path)
    static inline __attribute__((always_inline, noreturn)) void OS_Exit(int64_t exitCode) { ExitProcess((UINT)exitCode); }
    static inline __attribute__((always_inline)) void OS_Close(OsFileHandle fileDescriptor) { CloseHandle(fileDescriptor); }
    static inline __attribute__((always_inline)) void* OS_AllocateRAM(void* addr, size_t length, int32_t prot, int32_t flags, OsFileHandle fd) {
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
    
    static inline __attribute__((always_inline)) long OS_Read(OsFileHandle fd, void* buf, size_t count) {
        DWORD bytesRead = 0;
        if (ReadFile((HANDLE)fd, buf, (DWORD)count, &bytesRead, NULL)) {
            return (long)bytesRead;
        }
        return (long)-1;
    }
#else
    #define LINUX
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <time.h>
    #include <stdio.h>
    #include <unistd.h>
    typedef int OsFileHandle;
    #define OS_INVALID_HANDLE -1
    int brk(void *addr); void *sbrk(intptr_t increment);
    static inline __attribute__((always_inline)) void* OS_Brk(void* addr) { brk(addr); return (void*)sbrk(0); }
    static inline __attribute__((always_inline)) OsFileHandle OS_Read(OsFileHandle fd, void* buf, size_t count) { ssize_t res = read(fd, buf, count); return (long)res; }
    static inline __attribute__((always_inline, noreturn)) void OS_Exit(int64_t exitCode) { _exit(exitCode); }
    static inline __attribute__((always_inline)) int OS_MakeFolder(const char *path) { return mkdir(path, 0755); }
    static inline __attribute__((always_inline)) void OS_Close(OsFileHandle fileDescriptor) { close(fileDescriptor); }
    static inline __attribute__((always_inline)) void* OS_AllocateRAM(void* addr, size_t length, int32_t prot, int32_t flags, OsFileHandle fd) {
        void* ptr = mmap(addr,length,prot,flags,fd,0);
        if (ptr == MAP_FAILED || ptr == NULL) { DualLogError("Failed to allocate RAM\n"); OS_Exit(1); }
        return ptr;
    }
#endif

static inline __attribute__((always_inline)) int64_t OS_RawWrite(OsFileHandle fd, const void* buf, size_t count) {
    #ifdef WINDOWS
        DWORD written = 0;
        if (WriteFile((HANDLE)fd, buf, (DWORD)count, &written, NULL)) return (int64_t)written;
        return -1;
    #else // Linux
        register int64_t     rax __asm__("rax") = 1; // sys_write
        register int32_t     rdi __asm__("rdi") = fd;
        register const void* rsi __asm__("rsi") = buf;
        register size_t      rdx __asm__("rdx") = count;
        __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi), "r"(rsi), "r"(rdx) : "rcx", "r11", "memory");
        return rax;
    #endif
}

static inline __attribute__((always_inline)) void OS_Write(OsFileHandle fd, const void* buffer, size_t size, const char* filePath) {
    size_t total = 0;
    while (total < size) {
        int64_t written = OS_RawWrite(fd,(const char*)buffer + total,size - total);
        if (written < 0) { DualLogError("Write error when attempting write to %s: %s (code: %d)\n",filePath,written,(int32_t)(-written)); OS_Exit(1); }

        total += (size_t)written;
    }
}

static inline __attribute__((always_inline)) long OS_Open(const char* path, int32_t flags, int32_t mode) {
    register long rax __asm__("rax") = 2;           // sys_open
    register const char* rdi __asm__("rdi") = path;
    register long rsi __asm__("rsi") = flags;
    register long rdx __asm__("rdx") = mode;
    __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi), "r"(rsi), "r"(rdx) : "rcx", "r11", "memory");
    return rax;
}

static inline __attribute__((always_inline)) OsFileHandle OS_OpenReadonly(const char* filePath) {
    #ifdef WINDOWS
        HANDLE fp = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
        if (fp == OS_INVALID_HANDLE) { DualLogError("Could not open file %s for reading\n",filePath); return OS_INVALID_HANDLE; }
    #else // Linux
        OsFileHandle fp = OS_Open(filePath,O_RDONLY,0);
        if (fp < 0) { DualLogError("Could not open file %s for reading\n",filePath); return OS_INVALID_HANDLE; }
    #endif
    return fp;
}

static inline __attribute__((always_inline)) OsFileHandle OS_OpenWriteonly(const char* filePath) {
    #ifdef WINDOWS
        OsFileHandle h = CreateFileA(filePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        return (h == OS_INVALID_HANDLE) ? (DualLogError("Failed to open %s for writing\n", filePath), OS_Exit(1), OS_INVALID_HANDLE) : h;
    #else // Linux
        OsFileHandle h = OS_Open(filePath,O_WRONLY | O_CREAT | O_TRUNC,0644);
        return (h < 0) ? (DualLogError("Failed to open %s for writing\n", filePath), OS_Exit(1), OS_INVALID_HANDLE) : h;
    #endif
}

static inline __attribute__((always_inline)) int OS_FileSize(OsFileHandle fileDescriptor) {
    #ifdef WINDOWS
        if (fileDescriptor == OS_INVALID_HANDLE) return -1;
        
        LARGE_INTEGER size;
        if (!GetFileSizeEx(fileDescriptor, &size)) return -1;
        return size.QuadPart;
    #else // Linux
        struct stat fileStatisticsStruct;
        fstat(fileDescriptor, &fileStatisticsStruct);
        return fileStatisticsStruct.st_size;
    #endif
}

static inline __attribute__((always_inline)) void* OS_AllocateFileBackedRAMReadonly(size_t size, OsFileHandle fileDescriptor, char* filePath) {
    #ifdef WINDOWS
        if (fileDescriptor == OS_INVALID_HANDLE || size == 0) return NULL;

        HANDLE hMapping = CreateFileMappingA(fileDescriptor, NULL, PAGE_READONLY, 0, 0, NULL);
        if (hMapping == NULL) { DualLogError("CreateFileMapping failed for %s (err %lu)\n", filePath, GetLastError()); return NULL; }

        void* ramSpacePointer = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, size);
        CloseHandle(hMapping);  // view keeps reference alive
        if (ramSpacePointer == NULL) { DualLogError("Failed to allocate %s (err %lu)\n", filePath, GetLastError()); return NULL; }
    #else // Linux
        void* ramSpacePointer = OS_AllocateRAM(NULL, size, PROT_READ, MAP_PRIVATE, fileDescriptor);
        if (ramSpacePointer == MAP_FAILED) { DualLogError("Failed to allocate %s\n", filePath); return NULL; }
    #endif
    return ramSpacePointer;
}

static inline __attribute__((always_inline)) void* OS_OpenAndAllocateFileBufferReadonly(const char* filePath, OsFileHandle* fileDescriptor, int* size) {
    *fileDescriptor = OS_OpenReadonly(filePath);
    if (*fileDescriptor == OS_INVALID_HANDLE) { *size = 0; OS_Exit(1); }
    
    *size = (int)OS_FileSize(*fileDescriptor);
    if (*size <= 0) { DualLogError("Warning: File %s is empty, skipping allocation.\n", filePath); OS_Close(*fileDescriptor); OS_Exit(1); }
    
    void* ramSpacePointer = OS_AllocateFileBackedRAMReadonly(*size, *fileDescriptor, (char*)filePath);
    OS_Close(*fileDescriptor);
    return ramSpacePointer;
}

static inline __attribute__((always_inline)) void OS_DeallocateRAM(void* ramSpacePointer, size_t size) {    
    #ifdef WINDOWS
        (void)size;
        if (!ramSpacePointer) { DualLogError("Attempting to double free!\n"); OS_Exit(1); }
        
        if (!UnmapViewOfFile(ramSpacePointer)) { if (VirtualFree(ramSpacePointer, 0 /* dwSize (must be 0 with MEM_RELEASE) */, MEM_RELEASE) == 0) DualLogError("VirtualFree failed\n"); }
    #else // Linux
        if (!ramSpacePointer || ramSpacePointer == MAP_FAILED) { DualLogError("Attempting to double free!\n"); OS_Exit(1); }
        
        if ((void *)(int64_t)munmap(ramSpacePointer, size) == MAP_FAILED) DualLogError("munmap failed\n");
    #endif
}

static inline __attribute__((always_inline)) uint64_t mix64(uint64_t x) {
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;
    return x;
}

static inline __attribute__((always_inline)) uint64_t OS_GetFilestamp(const FileFingerprint *fp) { return mix64(fp->size); }

static inline __attribute__((always_inline)) bool OS_GetFileFingerprint(const char *path, FileFingerprint *fp) {
    #ifdef _WIN32
        // Use CreateFile to get a handle (required for detailed file info)
        HANDLE hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == OS_INVALID_HANDLE) return false;

        BY_HANDLE_FILE_INFORMATION bhfi;
        if (!GetFileInformationByHandle(hFile, &bhfi)) { CloseHandle(hFile); return false; }

        fp->size     = ((uint64_t)bhfi.nFileSizeHigh << 32) | bhfi.nFileSizeLow;
        CloseHandle(hFile);
    #else // Linux
        struct stat st;
        if (stat(path, &st) != 0) { DualLogError("Failed to stat \"%s\"\n", path); return false; }

        fp->size  = (uint64_t)st.st_size;
    #endif
    return true;
}

static inline __attribute__((always_inline)) int64_t OS_Seek(OsFileHandle fd, int64_t offset, int whence) { // forth and forsooth pray tell
    #ifdef WINDOWS
        LARGE_INTEGER liDistanceToMove;
        LARGE_INTEGER liNewFilePointer;
        liDistanceToMove.QuadPart = offset;
        DWORD dwMoveMethod;
        switch (whence) {
            case SEEK_SET: dwMoveMethod = FILE_BEGIN;   break;
            case SEEK_CUR: dwMoveMethod = FILE_CURRENT; break;
            case SEEK_END: dwMoveMethod = FILE_END;     break;
            default: return -1;
        }

        if (!SetFilePointerEx(fd, liDistanceToMove, &liNewFilePointer, dwMoveMethod)) return -1;
        return liNewFilePointer.QuadPart;
    #else // Linux
        off_t res = lseek(fd, (off_t)offset, whence); return (int64_t)res;
    #endif
}

static inline __attribute__((always_inline)) int64_t OS_Tell(OsFileHandle fd) {
    #ifdef WINDOWS
        LARGE_INTEGER pos = {0};
        if (!SetFilePointerEx(fd, (LARGE_INTEGER){0}, &pos, FILE_CURRENT)) return -1;
        return pos.QuadPart;
    #else
        return lseek(fd, 0, SEEK_CUR);
    #endif
}

static inline __attribute__((always_inline)) int OS_GetNumThreads(void) {
#ifdef WINDOWS
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return (int)2;//si.dwNumberOfProcessors;
#else
    return (int)sysconf(_SC_NPROCESSORS_ONLN);
#endif
}
