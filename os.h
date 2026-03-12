// os.h - starts most translation units and defines the shim layer between Voxen and the OS as well as defining project wide OS defines.
#pragma once
#if !defined(__GNUC__) || defined(__clang__) || !defined(__GNUC_MINOR__)
#error This project is only intended to be compiled with GCC (not Clang, MSVC, etc.)
#endif
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
#include <stdio.h>
#include <omp.h>
#define bool _Bool
#define true 1
#define false 0
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
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
#ifndef NULL
    #define NULL 0
#endif
#define O_RDONLY 00000000
#define O_WRONLY 00000001
#define O_RDWR 00000002
#define O_CREAT 00000100
#define O_TRUNC 00001000
#define O_CLOEXEC 02000000
void DualLog(const char* fmt, ...);
void DualLogWarn(const char* fmt, ...);
void DualLogError(const char* fmt, ...);
char* StringFindSubstring(const char* haystack, const char* needle);
char* StringFindFirstCharWithin(const char *s, char c);
typedef struct {
    uint64_t mtime_ns;
    uint64_t size;
    uint64_t inode;
    uint64_t dev;
} FileFingerprint;

static inline __attribute__((always_inline, noreturn)) void OS_Exit(int64_t exitCode) {
    #ifdef WINDOWS
        register uint64_t rax __asm__("rax") = 0x2C;
        register HANDLE   rcx __asm__("rcx") = (HANDLE)-1;
        register NTSTATUS rdx __asm__("rdx") = (NTSTATUS)exitCode;
        __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rcx), "r"(rdx) : "r8", "r9", "r10", "r11", "memory");
    #else
        register int64_t rax __asm__("rax") = 231;
        register int64_t rdi __asm__("rdi") = exitCode;
        __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi) : "rcx", "r11", "memory");
    #endif
    __builtin_unreachable();
}

#if defined(_WIN32) || defined(_WIN64)
    #define WINDOWS
    #define WIN32_LEAN_AND_MEAN // Let 'er rip, tater chip
    #include <windows.h> // The things I do for my players, yeesh
    #include <direct.h>
    typedef HANDLE OsFileHandle;
    #define OS_INVALID_HANDLE INVALID_HANDLE_VALUE
    #define OS_MakeFolder(path) _mkdir(path)
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
    
    static inline __attribute__((always_inline)) long OS_Read(long fd, void* buf, size_t count) {
        // NtReadFile — direct syscall, zero library involvement
        // syscall number is stable at 0x0003 on all modern Windows 10/11 (including 26H1)
        register uint64_t rax __asm__("rax") = 0x0003;
        register HANDLE   rcx __asm__("rcx") = (HANDLE)fd;
        register void*    rdx __asm__("rdx") = NULL;
        register void*    r8  __asm__("r8")  = buf;
        register size_t   r9  __asm__("r9")  = count;
        long bytesRead = -1;
        __asm__ __volatile__(
            "xor %%r10, %%r10\n\t"
            "movq $0, 8(%%rsp)\n\t"
            "syscall" : "+r"(rax) : "r"(rcx), "r"(rdx), "r"(r8), "r"(r9) : "r10", "r11", "memory");

        return (rax == 0) ? (long)count : (long)rax;
    }
#else
    #if defined(__linux__) && !defined(__ANDROID__)
        #define LINUX
        static inline __attribute__((always_inline)) void* OS_Brk(void* addr) {
            register long rax __asm__("rax") = 12;
            register void* rdi __asm__("rdi") = addr;
            __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi) : "rcx", "r11", "memory");
            return (void*)rax;
        }
        
        static inline __attribute__((always_inline)) long OS_Read(long fd, void* buf, size_t count) {
            register long rax __asm__("rax") = 0;
            register long rdi __asm__("rdi") = fd;
            register void* rsi __asm__("rsi") = buf;
            register size_t rdx __asm__("rdx") = count;
            __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi), "r"(rsi), "r"(rdx) : "rcx", "r11", "memory");
            return rax;
        }
    #endif

    #if defined(__ANDROID__)
        #define ANDROID
    #endif
    
    #define OS_INVALID_HANDLE -1
    #if defined(__APPLE__)
        #define MAC
        #include <sys/types.h>
    #else
        typedef int OsFileHandle;
        typedef uint64_t dev_t;
        typedef uint64_t ino_t;
        typedef uint32_t mode_t;
        typedef long unsigned int nlink_t;
        typedef uint32_t uid_t;
        typedef uint32_t gid_t;
        typedef int64_t off_t;
        typedef int64_t blksize_t;
        typedef int64_t blkcnt_t;
        typedef int64_t time_t;
        struct timespec {
            time_t tv_sec;
            long tv_nsec;
        };
        
        struct stat {
            dev_t     st_dev;
            ino_t     st_ino;
            nlink_t   st_nlink;
            mode_t    st_mode;
            uid_t     st_uid;
            gid_t     st_gid;
            int       __pad0;
            dev_t     st_rdev;
            off_t     st_size;
            blksize_t st_blksize;
            blkcnt_t  st_blocks;
            struct timespec st_atim;
            struct timespec st_mtim;
            struct timespec st_ctim;
            long __unused[3];
        };
    #endif
    
    static inline __attribute__((always_inline)) void* mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset) {
        register long rax __asm__("rax") = 9;
        register void* rdi __asm__("rdi") = addr;
        register size_t rsi __asm__("rsi") = length;
        register int rdx __asm__("rdx") = prot;
        register int r10 __asm__("r10") = flags;
        register int r8 __asm__("r8") = fd;
        register off_t r9 __asm__("r9") = offset;
        __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi), "r"(rsi), "r"(rdx), "r"(r10), "r"(r8), "r"(r9) : "rcx", "r11", "memory");
        return (void*)rax;
    }

    static inline __attribute__((always_inline)) int munmap(void* addr, size_t length) {
        register long rax __asm__("rax") = 11;
        register void* rdi __asm__("rdi") = addr;
        register size_t rsi __asm__("rsi") = length;
        __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi), "r"(rsi) : "rcx", "r11", "memory");
        return (int)rax;
    }

    static inline __attribute__((always_inline)) int stat(const char *path, struct stat *st) {
        register long rax __asm__("rax") = 262;           // __NR_newfstatat
        register int  rdi __asm__("rdi") = -100;          // AT_FDCWD
        register const char *rsi __asm__("rsi") = path;
        register struct stat *rdx __asm__("rdx") = st;
        register int  r10 __asm__("r10") = 0;             // flags = 0 (follow symlinks)
        __asm__ __volatile__("syscall" : "+r"(rax)
            : "r"(rdi), "r"(rsi), "r"(rdx), "r"(r10)
            : "rcx", "r11", "memory");
        return (int)rax;
    }

    static inline __attribute__((always_inline)) int fstat(int fd, struct stat *st) {
        register long rax __asm__("rax") = 5;
        register int rdi __asm__("rdi") = fd;
        register struct stat *rsi __asm__("rsi") = st;
        __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi), "r"(rsi) : "rcx", "r11", "memory");
        return (int)rax;
    }

    static inline __attribute__((always_inline)) int OS_MakeFolder(const char *path) {
        register long rax __asm__("rax") = 83;
        register const char *rdi __asm__("rdi") = path;
        register mode_t rsi __asm__("rsi") = 0755;
        __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi), "r"(rsi) : "rcx", "r11", "memory");
        return (int)rax;
    }

    static inline __attribute__((always_inline)) void OS_Close(OsFileHandle fileDescriptor) {
        register long rax __asm__("rax") = 3;
        register long rdi __asm__("rdi") = fileDescriptor;
        __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi) : "rcx", "r11", "memory");
    }

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
    #else // Linux, Mac, Android
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
        if (fp == OS_INVALID_HANDLE) { DualLog("Could not open file %s for reading\n", filePath); return OS_INVALID_HANDLE; }
    #else // Linux, Mac, Android
        OsFileHandle fp = OS_Open(filePath,O_RDONLY,0);
        if (fp < 0) { DualLog("Could not open file %s for reading\n", filePath); return OS_INVALID_HANDLE; }
    #endif
    return fp;
}

static inline __attribute__((always_inline)) OsFileHandle OS_OpenWriteonly(const char* filePath) {
    #ifdef WINDOWS
        OsFileHandle h = CreateFileA(filePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        return (h == OS_INVALID_HANDLE) ? (DualLogError("Failed to open %s for writing\n", filePath), OS_Exit(1), OS_INVALID_HANDLE) : h;
    #else // Linux, Mac, Android
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
    #else // Linux, Mac, Android
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
    #else // Linux, Mac, Android
        void* ramSpacePointer = OS_AllocateRAM(NULL, size, PROT_READ, MAP_PRIVATE, fileDescriptor);
        if (ramSpacePointer == MAP_FAILED) { DualLogError("Failed to allocate %s\n", filePath); return NULL; }
    #endif
    return ramSpacePointer;
}

static inline __attribute__((always_inline)) void* OS_OpenAndAllocateFileBufferReadonly(const char* filePath, OsFileHandle* fileDescriptor, int* size) {
    *fileDescriptor = OS_OpenReadonly(filePath);
    if (*fileDescriptor == OS_INVALID_HANDLE) { *size = 0; return NULL; } // NULL not exit, since this is the path for re-importing instead of using the cache files.
    
    *size = (int)OS_FileSize(*fileDescriptor);
    if (*size <= 0) { DualLogWarn("Warning: File %s is empty, skipping allocation.\n", filePath); OS_Close(*fileDescriptor); return NULL; }
    
    void* ramSpacePointer = OS_AllocateFileBackedRAMReadonly(*size, *fileDescriptor, (char*)filePath);
    OS_Close(*fileDescriptor);
    return ramSpacePointer;
}

static inline __attribute__((always_inline)) void OS_DeallocateRAM(void* ramSpacePointer, size_t size) {    
    #ifdef WINDOWS
        (void)size;
        if (!ramSpacePointer) { DualLogError("Attempting to double free!\n"); OS_Exit(1); }
        
        if (!UnmapViewOfFile(ramSpacePointer)) { if (VirtualFree(ramSpacePointer, 0 /* dwSize (must be 0 with MEM_RELEASE) */, MEM_RELEASE) == 0) DualLogError("VirtualFree failed\n"); }
    #else // Linux, Mac, Android
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
    #else // Linux, Mac, Android
        struct stat st;
        if (stat(path, &st) != 0) { DualLogError("Failed to stat \"%s\"\n", path); return false; }

        fp->size  = (uint64_t)st.st_size;
    #endif
    return true;
}

#ifndef _WIN32
void* __stack_chk_guard = (void*)0xdeadbeefcafebabeULL;
__attribute__((noreturn)) void __stack_chk_fail(void) { DualLogError("Stack protector: canary corrupted - possible stack smash!"); while(1); }
#endif

static inline __attribute__((always_inline)) int64_t OS_Seek(OsFileHandle fd, int64_t offset, int whence) { // forth and forsooth pray tell
    #ifdef WINDOWS
        // Use NtSetInformationFile with FilePositionInformation (class 14)
        // This is the direct syscall path (no kernel32).
        struct {
            LARGE_INTEGER CurrentByteOffset;
        } pos_info;

        pos_info.CurrentByteOffset.QuadPart = offset;

        // NtSetInformationFile syscall number = 0x24 (stable on Windows 10/11 25H2/26H1)
        register uint64_t rax __asm__("rax") = 0x24;
        register HANDLE   rcx __asm__("rcx") = (HANDLE)fd;
        register void*    rdx __asm__("rdx") = NULL;                    // IoStatusBlock (we ignore)
        register void*    r8  __asm__("r8")  = &pos_info;
        register uint32_t r9  __asm__("r9")  = 14;                      // FilePositionInformation
        register uint64_t r10 __asm__("r10") = 8;                       // Length of FILE_POSITION_INFORMATION
        __asm__ __volatile__(
            "xor %%r11, %%r11\n\t"          // Event = NULL
            "syscall" : "+r"(rax) : "r"(rcx), "r"(rdx), "r"(r8), "r"(r9), "r"(r10) : "r11", "memory");

        if (rax != 0) return -1;
        return offset;
    #else
        // Linux / macOS / Android
        register int64_t rax __asm__("rax") = 8;
        register int64_t rdi __asm__("rdi") = fd;
        register int64_t rsi __asm__("rsi") = offset;
        register int64_t rdx __asm__("rdx") = whence;
        __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi), "r"(rsi), "r"(rdx) : "rcx", "r11", "memory");
        return rax;
    #endif
}

static inline __attribute__((always_inline)) int64_t OS_Tell(OsFileHandle fd) {
#ifdef WINDOWS
    // NtQueryInformationFile with FilePositionInformation (class 14)
    struct {
        LARGE_INTEGER CurrentByteOffset;
    } pos_info;

    // NtQueryInformationFile syscall number = 0x23 (stable on Windows 10/11)
    register uint64_t rax __asm__("rax") = 0x23;
    register HANDLE   rcx __asm__("rcx") = (HANDLE)fd;
    register void*    rdx __asm__("rdx") = NULL;
    register void*    r8  __asm__("r8")  = &pos_info;
    register uint32_t r9  __asm__("r9")  = 14;             // FilePositionInformation
    register uint64_t r10 __asm__("r10") = 8;              // Length of FILE_POSITION_INFORMATION
    __asm__ __volatile__(
        "xor %%r11, %%r11\n\t"
        "syscall" : "+r"(rax) : "r"(rcx), "r"(rdx), "r"(r8), "r"(r9), "r"(r10) : "r11", "memory");

    if (rax != 0) return -1;
    return pos_info.CurrentByteOffset.QuadPart;

#else
    // Linux / macOS / Android
    register int64_t rax __asm__("rax") = 8;      // sys_lseek
    register int64_t rdi __asm__("rdi") = fd;
    register int64_t rsi __asm__("rsi") = 0;
    register int64_t rdx __asm__("rdx") = 1;      // SEEK_CUR
    __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi), "r"(rsi), "r"(rdx) : "rcx", "r11", "memory");
    return rax;
#endif
}

#include "underversion_libc.h"
