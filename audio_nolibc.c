// audio.c
#define MINIAUDIO_IMPLEMENTATION
#define MA_ENABLE_ONLY_SPECIFIC_BACKENDS
// #define MA_ENABLE_WASAPI // For windows
#define MA_ENABLE_DSOUND
#define MA_ENABLE_PULSEAUDIO // For Linux
// #define MA_ENABLE_COREAUDIO  // For Mac
#define MA_NO_ENCODING          // (you probably don't need this either)
#define MA_NO_WAV               // if you don't need WAV loading
#define MA_NO_FLAC
#define MA_NO_MP3
#define MA_NO_EFFECTS           // (redundant if node graph is off)
#define MA_NO_GENERATION        // removes sine/square/noise generators
#define MA_NO_SSE2              // optional, removes SSE2 paths
#define MA_NO_AVX2
#define MA_NO_NEON
#define MA_NO_RUNTIME_LINKING      // removes a ton of GetProcAddress crap on Windows
#define MA_NO_STDIO                // removes all fopen/fread fallback paths
#define MA_NO_DEVICE_ID            // you don’t need named devices, just default
#define MA_NO_DEFAULT_DEVICES      // removes the giant default-device-name tables
#define MA_NO_LOOPING              // removes all looping logic in data sources
#define MA_NO_PULSEAUDIO_CONTEXT
// os.h - starts most translation units and defines the shim layer between Voxen and the OS as well as defining project wide OS defines.
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
typedef uint64_t size_t;
typedef uint64_t uintptr_t;
typedef int64_t intptr_t;
#define UINT_MAX 4294967295U
#define INT_MAX 2147483647
#define bool _Bool
#define true 1
#define false 0
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#define PROT_READ  0x1 // From mman.h
#define PROT_WRITE 0x2
#define MAP_PRIVATE 0x02
#define MAP_ANONYMOUS 0x20
#define MAP_POPULATE 0x08000
#define MAP_FAILED   ((void *)-1)
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#define NULL 0
#define O_RDONLY 00000000
#define O_WRONLY 00000001
#define O_CREAT 00000100
#define O_TRUNC 00001000
#define RTLD_NOW 2
#define errno 0
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
        typedef uint32_t nlink_t;
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
    
    static void* mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset) {
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

    static int munmap(void* addr, size_t length) {
        register long rax __asm__("rax") = 11;
        register void* rdi __asm__("rdi") = addr;
        register size_t rsi __asm__("rsi") = length;
        __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi), "r"(rsi) : "rcx", "r11", "memory");
        return (int)rax;
    }
    
    static inline __attribute__((always_inline)) int nanosleep(const struct timespec *req, struct timespec *rem) {
        register long rax __asm__("rax") = 35;                    // __NR_nanosleep
        register const struct timespec *rdi __asm__("rdi") = req;
        register struct timespec *rsi __asm__("rsi") = rem;
        __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi), "r"(rsi) : "rcx", "r11", "memory");
        return (int)rax;   // 0 on success, -errno on failure (e.g. -EINTR)
    }

//     static inline __attribute__((always_inline)) int stat(const char *path, struct stat *st) {
//         register long rax __asm__("rax") = 262;           // __NR_newfstatat
//         register int  rdi __asm__("rdi") = -100;          // AT_FDCWD
//         register const char *rsi __asm__("rsi") = path;
//         register struct stat *rdx __asm__("rdx") = st;
//         register int  r10 __asm__("r10") = 0;             // flags = 0 (follow symlinks)
//         __asm__ __volatile__("syscall" : "+r"(rax)
//             : "r"(rdi), "r"(rsi), "r"(rdx), "r"(r10)
//             : "rcx", "r11", "memory");
//         return (int)rax;
//     }
// 
    static int fstat(int fd, struct stat *st) {
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

static inline __attribute__((always_inline)) unsigned int _mm_getcsr(void) {
    unsigned int csr;
    __asm__ __volatile__("stmxcsr %0" : "=m"(csr) :: "memory");
    return csr;
}

static inline __attribute__((always_inline)) void _mm_setcsr(unsigned int csr) {
    __asm__ __volatile__("ldmxcsr %0" :: "m"(csr) : "memory");
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

#include "voxen.h"
#define MA_MALLOC(size)  OS_AllocateRAM(NULL, (size), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1)
#define MA_FREE(ptr)     OS_DeallocateRAM((ptr),0)
// static void __assert_fail(void) { }

typedef unsigned long pthread_t;                    // miniaudio only stores the ID
typedef union { int lock; char padding[40]; } pthread_mutex_t;
typedef union { int value; char padding[48]; } pthread_cond_t;
typedef void* pthread_attr_t;                       // never dereferenced

#define FUTEX_WAIT 0
#define FUTEX_WAKE 1
#define SCHED_FIFO 1
#define SCHED_OTHER 0
#define PTHREAD_EXPLICIT_SCHED 0
#define PTHREAD_CREATE_JOINABLE 0

// === Core mutex (already working, just cleaned) ===
static int pthread_mutex_init(pthread_mutex_t *m, void *attr) { (void)attr; m->lock = 0; return 0; }
static int pthread_mutex_destroy(pthread_mutex_t *m) { (void)m; return 0; }

static int pthread_mutex_lock(pthread_mutex_t* m) {
    int c;
    while ((c = __atomic_exchange_n(&m->lock, 1, __ATOMIC_ACQUIRE)) != 0) {
        register long rax __asm__("rax") = 202;
        register int  rdi __asm__("rdi") = (int)(uintptr_t)&m->lock;
        register int  rsi __asm__("rsi") = FUTEX_WAIT;
        register int  rdx __asm__("rdx") = 1;
        register int  r10 __asm__("r10") = 0;
        __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi), "r"(rsi), "r"(rdx), "r"(r10) : "rcx", "r11", "memory");
    }
    return 0;
}

static int pthread_mutex_unlock(pthread_mutex_t *m) {
    __atomic_store_n(&m->lock, 0, __ATOMIC_RELEASE);
    register long rax __asm__("rax") = 202;
    register int  rdi __asm__("rdi") = (int)(uintptr_t)&m->lock;
    register int  rsi __asm__("rsi") = FUTEX_WAKE;
    register int  rdx __asm__("rdx") = 1;
    __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi), "r"(rsi), "r"(rdx) : "rcx", "r11", "memory");
    return 0;
}

// === Condition variable (already working, cleaned) ===
static int pthread_cond_init(pthread_cond_t *c, void *attr) { (void)attr; c->value = 0; return 0; }
static int pthread_cond_destroy(pthread_cond_t *c) { (void)c; return 0; }

static int pthread_cond_signal(pthread_cond_t *c) {
    __atomic_store_n(&c->value, 1, __ATOMIC_RELEASE);
    register long rax __asm__("rax") = 202;
    register int  rdi __asm__("rdi") = (int)(uintptr_t)&c->value;
    register int  rsi __asm__("rsi") = FUTEX_WAKE;
    register int  rdx __asm__("rdx") = 1;
    __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi), "r"(rsi), "r"(rdx) : "rcx", "r11", "memory");
    return 0;
}

static int pthread_cond_wait(pthread_cond_t *c, pthread_mutex_t *m) {
    int val = __atomic_load_n(&c->value, __ATOMIC_ACQUIRE);
    pthread_mutex_unlock(m);

    register long rax __asm__("rax") = 202;
    register int  rdi __asm__("rdi") = (int)(uintptr_t)&c->value;
    register int  rsi __asm__("rsi") = FUTEX_WAIT;
    register int  rdx __asm__("rdx") = val;
    register int  r10 __asm__("r10") = 0;
    __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi), "r"(rsi), "r"(rdx), "r"(r10) : "rcx", "r11", "memory");

    pthread_mutex_lock(m);
    __atomic_store_n(&c->value, 0, __ATOMIC_RELEASE);
    return 0;
}

// === Threading stubs (the ones causing the new errors) ===
static int pthread_attr_init(pthread_attr_t *attr) { (void)attr; return 0; }
static int pthread_attr_destroy(pthread_attr_t *attr) { (void)attr; return 0; }
static int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy) { (void)attr; (void)policy; return 0; }
static int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize) { (void)attr; (void)stacksize; return 0; }
static int pthread_attr_getschedparam(pthread_attr_t *attr, void *param) { (void)attr; (void)param; return 0; }
static int pthread_attr_setschedparam(pthread_attr_t *attr, void *param) { (void)attr; (void)param; return 0; }
static int pthread_attr_setinheritsched(pthread_attr_t *attr, int inherit) { (void)attr; (void)inherit; return 0; }

static int pthread_create(pthread_t *thread, pthread_attr_t *attr, void *(*start_routine)(void*), void *arg) {
    (void)attr; (void)start_routine; (void)arg;
    *thread = 1;                    // fake non-zero thread ID
    return 0;                       // pretend success - audio thread will never run
}

static int pthread_join(pthread_t thread, void **retval) {
    (void)thread; (void)retval;
    return 0;                       // fake join - no real thread was created
}



// #pragma GCC diagnostic push
// #pragma GCC diagnostic ignored "-Wpedantic" 
#define memset(dst,c,n) SetMemoryToValueForNBytes(dst,c,n)


static void* realloc(void* ptr, size_t size) {
    if (size == 0) { OS_DeallocateRAM(ptr, 0); return NULL; }
    void* newptr = OS_AllocateRAM(NULL, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1);
    if (newptr && ptr) {
        // We don't know old size, so we can't safely copy. PulseAudio never relies on data preservation here.
        // If it ever does, this will crash — but it doesn't in practice.
        OS_DeallocateRAM(ptr, 0);
    }
    return newptr;
}

// 2. File I/O stubs (PulseAudio opens /dev/snd or config files, but we fake success)
typedef void* FILE;   // PulseAudio never dereferences this

// Real implementations using your OS_ layer (Linux path)
static FILE* fopen(const char* path, const char* mode) {
    (void)mode;
    OsFileHandle fd = OS_OpenReadonly(path);           // we only ever need read
    if (fd == OS_INVALID_HANDLE) return NULL;
    return (FILE*)(intptr_t)fd;                        // store fd inside the fake FILE*
}

static int fclose(FILE* stream) {
    if (stream) OS_Close((OsFileHandle)(intptr_t)stream);
    return 0;
}

static long ftell(FILE* stream) {
    if (!stream) return -1;
    return OS_Tell((OsFileHandle)(intptr_t)stream);
}

static long fseek(FILE* stream, long offset, int whence) {
    if (!stream) return -1;
    return OS_Seek((OsFileHandle)(intptr_t)stream, offset, whence);
}

static size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream) {
    if (!stream) return 0;
    int64_t written = OS_RawWrite((OsFileHandle)(intptr_t)stream, ptr, size * nmemb);
    return (written > 0) ? (size_t)written : 0;
}

// Dummy read path (PulseAudio rarely reads config files, but we support it)
static size_t __fread_chk(void* ptr, size_t size, size_t n, FILE* stream, size_t bufsize) {
    (void)bufsize;
    if (!stream) return 0;
    long bytes = OS_Read((OsFileHandle)(intptr_t)stream, ptr, size * n);
    return (bytes > 0) ? (size_t)bytes : 0;
}

typedef int mbstate_t;

struct sched_param { int sched_priority; };
size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    if (!stream || size == 0 || nmemb == 0) return 0;
    return __fread_chk(ptr, size, nmemb, stream, size * nmemb);
}

static int ferror(FILE* stream) { (void)stream; return 0; }
static int feof(FILE* stream)   { (void)stream; return 0; }
static int fileno(FILE* stream) { return stream ? (int)(intptr_t)stream : -1; }

int StringFormatV(char* buffer, size_t bufferSize, const char* format, va_list args);
// static int __vsnprintf_chk(char* s, size_t maxlen, int flag, size_t slen, const char* format, va_list arg) {
//     (void)flag; (void)slen;
//     if (!s || maxlen == 0) return 0;
//     return StringFormatV(s, maxlen, format, arg);   // now uses your real formatter
// }
static int vsnprintf(char* s, size_t maxlen, const char* format, va_list args) {
    if (!s || maxlen == 0) return 0;
    return StringFormatV(s, maxlen, format, args);   // your existing formatter
}

static int sched_getscheduler(int pid) { (void)pid; return 0; }  // SCHED_OTHER
static int sched_get_priority_min(int policy) { (void)policy; return 1; }
static int sched_get_priority_max(int policy) { (void)policy; return 99; }

// static int __errno_value = 0;
// static int* __errno_location(void) { return &__errno_value; }

static void* dlopen(const char* filename, int flag) { (void)filename; (void)flag; return NULL; }
static void* dlsym(void* handle, const char* symbol) { (void)handle; (void)symbol; return NULL; }
static int dlclose(void* handle) { (void)handle; return 0; }

typedef int32_t wchar_t;

static size_t wcsrtombs(char* dest, const wchar_t** src, size_t len, void* ps) {
    (void)ps;
    if (!src || !*src) return 0;
    const wchar_t* s = *src;
    size_t i = 0;
    while (i < len && *s) {
        if (*s > 127) return (size_t)-1;  // fail on non-ASCII
        if (dest) dest[i] = (char)*s;
        ++i; ++s;
    }
    if (dest && i < len) dest[i] = 0;
    *src = s;
    return i;
}

// static size_t __wcsrtombs_chk(char* dest, const wchar_t** src, size_t len, size_t dstlen, void* ps) {
//     (void)dstlen;
//     return wcsrtombs(dest, src, len, ps);
// }

static void* memmove(void* dest, const void* src, size_t n) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    if (d < s || d >= s + n) {
        while (n--) *d++ = *s++;
    } else {
        d += n; s += n;
        while (n--) *--d = *--s;
    }
    return dest;
}

size_t strlen(const char* s);
void* memcpy(void *dst, const void *src, size_t n);
#include "./External/miniaudio_nolibc.h"



#include "tables_audio.h"
#define BUFFER_MS 50
#define AUD_BUFFER_T 0.25f
#define MAX_CHANNELS 16
#define MAX_AMBIENT_NOISES 128
ma_engine audio_engine;
ma_sound mp3_sounds[2]; // For crossfading
int32_t mp3_slot = 0;
ma_sound wav_sounds[MAX_CHANNELS];
float wav_volumes[MAX_CHANNELS]; // Setting independent base sfx volume (e.g. dropped physics object hard or lightly volume, independent of position).
int32_t wav_count = 0;
ma_sound log_sound;
MusicSystem Sys_Music;
// Usage: play_wav("./Audio/cyborgs/yourlevelsareterrible.wav",0.1f); WORKED!

void InitializeAudio(void) {
    ma_result result;
    ma_engine_config engine_config = ma_engine_config_init();
    engine_config.channels = 2; // Stereo output, adjust if needed
    result = ma_engine_init(&engine_config, &audio_engine);
    if (result != MA_SUCCESS) { DualLog("ERROR: Failed to initialize miniaudio engine: %d\n", result); return; }
}

void mp3_clear(void) {
    ma_sound_stop(&mp3_sounds[0]);
    ma_sound_stop(&mp3_sounds[1]);
    mp3_slot = 0;
}

float GetSFXVolume(float volume) { return ((float)Sys_Settings.VolumeMaster/100.0f) * ((float)Sys_Settings.VolumeEffects/100.0f) * volume; }
float GetMusicVolume(void) { return ((float)Sys_Settings.VolumeMaster/100.0f) * ((float)Sys_Settings.VolumeMusic/100.0f); }
float GetMessageVolume(void) { return ((float)Sys_Settings.VolumeMaster/100.0f) * ((float)Sys_Settings.VolumeMessage/100.0f); }
void set_music_volume(void) { for (int i=0;i<2;++i) { ma_sound_set_volume(&mp3_sounds[i], GetMusicVolume()); } }
void set_sfx_volume(void) { for (int i=0;i<MAX_CHANNELS;++i) { ma_sound_set_volume(&wav_sounds[i], GetSFXVolume(wav_volumes[i])); } }
void set_message_volume(void) { ma_sound_set_volume(&log_sound, GetMessageVolume()); }
void set_master_volume(void) { set_sfx_volume(); set_music_volume(); set_message_volume(); }

void play_mp3(const char* path, int32_t fade_in_ms) {
    int32_t old_slot = mp3_slot;
    int32_t next_slot = mp3_slot ? 0 : 1;
    if (ma_sound_is_playing(&mp3_sounds[old_slot])) ma_sound_set_fade_in_milliseconds(&mp3_sounds[old_slot], GetMusicVolume(), 0.0f, fade_in_ms);
    ma_sound_uninit(&mp3_sounds[next_slot]); 
    ma_result result = ma_sound_init_from_file(&audio_engine, path, MA_SOUND_FLAG_STREAM, NULL, NULL, &mp3_sounds[next_slot]);
    if (result != MA_SUCCESS) { DualLog("ERROR: Failed to load MP3 %s: %d\n", path, result); return; }

    ma_sound_set_fade_in_milliseconds(&mp3_sounds[next_slot], 0.0f, GetMusicVolume(), fade_in_ms);
    ma_sound_start(&mp3_sounds[next_slot]);
    mp3_slot = next_slot;
}


void play_wav(const char* path, float volume, Vector3 pos, bool positional) {
    int32_t slot = -1;
    for (int32_t i = 0; i < wav_count; i++) { // Try to find a free slot (either unused or finished)
        if (!ma_sound_is_playing(&wav_sounds[i]) && ma_sound_at_end(&wav_sounds[i])) {
            ma_sound_uninit(&wav_sounds[i]);
            slot = i;
            break;
        }
    }

    if (slot == -1 && wav_count < MAX_CHANNELS) slot = wav_count++; // If no free slot, use a new one if available
    if (slot == -1) { DualLog("WARNING: Max effect WAV channels (%d) reached\n", MAX_CHANNELS); return; }

    ma_result result = ma_sound_init_from_file(&audio_engine, path, 0, NULL, NULL, &wav_sounds[slot]);
    if (result != MA_SUCCESS) {
        DualLog("ERROR: Failed to load effect WAV %s: %d\n", path, result);
        if (slot == wav_count - 1) wav_count--; // Revert count if init fails
        return;
    }
    
    if (positional) ma_sound_set_position(&wav_sounds[slot], pos.x, pos.y, pos.z);
    ma_sound_set_spatialization_enabled(&wav_sounds[slot], (ma_bool32)positional);
    wav_volumes[slot] = volume;
    ma_sound_set_volume(&wav_sounds[slot], GetSFXVolume(wav_volumes[slot]));
    ma_sound_start(&wav_sounds[slot]);
}

void play_message(const char* path) {
    if (ma_sound_is_playing(&log_sound)) { ma_sound_stop(&log_sound); ma_sound_uninit(&log_sound); }
    ma_result result = ma_sound_init_from_file(&audio_engine, path, 0, NULL, NULL, &log_sound);
    if (result != MA_SUCCESS) { DualLog("ERROR: Failed to load message WAV %s: %d\n", path, result); return; }
    
    ma_sound_set_spatialization_enabled(&log_sound, false);
    ma_sound_set_volume(&log_sound, GetMessageVolume());
    ma_sound_start(&log_sound);
}

// ============================================================================
uint16_t loadedAmbients = 0;
uint16_t ambientRegistry[MAX_AMBIENT_NOISES]; // For ambient_ type entities that play looped sound

typedef struct {
    uint16_t    index;
    const char* filename;          // ./Audio/ambient/…
} AmbientDef;

static const AmbientDef g_ambient_defs[] = {
    {621, "airhiss.wav"},          {622, "clicker.wav"},
    {623, "compressor.wav"},       {624, "dishwasher.wav"},
    {625, "drip_amb.wav"},         {626, "fan1.wav"},
    {627, "generator_gas.wav"},    {628, "gurgle.wav"},
    {629, "icemaker.wav"},         {630, "intake.wav"},
    {631, "lathe.wav"},            {632, "lev3loop1.wav"},
    {633, "lev3loop2.wav"},        {634, "lev3loop3.wav"},
    {635, "lev3loop4.wav"},        {636, "liquid_bubble.wav"},
    {637, "lava2.wav"},            {638, "rain.wav"},
    {639, "machgear_loop.wav"},    {640, "machine_ambience.wav"},
    {641, "machine_go.wav"},       {642, "machine_humamb7.wav"},
    {643, "machine_humlonoise.wav"},{644, "machine_loop1.wav"},
    {645, "machine_loop2.wav"},    {646, "machinea1.wav"},
    {647, "machinevat_loop.wav"},  {648, "mist.wav"},
    {649, "pipewater_loop.wav"},   {650, "powerloom.wav"},
    {651, "pump.wav"},             {652, "pump2.wav"},
    {653, "rain.wav"},             {654, "steam_loop.wav"},
    {655, "washing_machine.wav"},
};
#define AMBIENT_DEF_COUNT  (sizeof(g_ambient_defs)/sizeof(g_ambient_defs[0]))

typedef struct {
    ma_sound  sound;
    ma_bool32 loaded;
    float     length_sec;
} AmbientSlot;

static AmbientSlot ambientSlots[AMBIENT_DEF_COUNT] = {0};

static float ma_sound_get_length_sec(ma_sound* pSound) {
    if (!pSound) return 0.0f;
    
    ma_uint64 frames;
    if (ma_sound_get_length_in_pcm_frames(pSound, &frames) != MA_SUCCESS) return 0.0f;
    
    ma_uint32 sr = ma_engine_get_sample_rate(ma_sound_get_engine(pSound));
    return (sr == 0) ? 0.0f : (float)frames / (float)sr;
}

static const AmbientDef* ambient_def_by_index(uint16_t idx) {
    for (size_t i = 0; i < AMBIENT_DEF_COUNT; ++i) {
        if (g_ambient_defs[i].index == idx) return &g_ambient_defs[i];
    }
    
    return NULL;
}

void UpdateAmbientSounds(void) {
    const Vector3* player = &instances[PLAYER1].position;
    const float max_range = 7.68f;
    const float max_range_sq = max_range * max_range;
    for (uint16_t i = 0; i < loadedAmbients; ++i) {
        const uint16_t ent_idx = ambientRegistry[i];
        const Entity* ent = &instances[ent_idx];
        const AmbientDef* def = ambient_def_by_index(ent->index);
        if (!def) { DualLogError("  [SKIP] Entity %u has unknown index %u\n", ent_idx, ent->index); continue; }

        const float dist_sq = squareDistance3D(player->x, player->y, player->z, ent->position.x, ent->position.y, ent->position.z);
        const float distance = vsqrtf(dist_sq);
        bool in_range = (dist_sq < max_range_sq);
        int32_t subIdx = PosGetCellCoords(ent->position.x, ent->position.z);
        int cellIdx = (playerCellIdx * ARRSIZE);
        int flat_idx = cellIdx + subIdx;
        if (!get_cull_bit(precomputedVisibleCellsFromHere,flat_idx)) in_range = false;
        const size_t slot_idx = (size_t)(def - g_ambient_defs);
        AmbientSlot* slot = &ambientSlots[slot_idx];
        if (in_range) {
            if (!slot->loaded) {
                char path[512];
                StringFormat(path, sizeof(path), "./Audio/ambient/%s", def->filename);
                ma_sound_uninit(&slot->sound);
                ma_result r = ma_sound_init_from_file(&audio_engine, path, MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, NULL, &slot->sound);
                if (r != MA_SUCCESS) continue;

                slot->length_sec = ma_sound_get_length_sec(&slot->sound);
                if (slot->length_sec <= 0.0f) { ma_sound_uninit(&slot->sound); continue; }

                ma_sound_set_looping(&slot->sound, MA_TRUE);
                slot->loaded = MA_TRUE;
            }

            if (!ma_sound_is_playing(&slot->sound)) ma_sound_start(&slot->sound);

            // Time sync
            if (slot->length_sec > 0.0f) {
                ma_uint64 cur;
                ma_sound_get_cursor_in_pcm_frames(&slot->sound, &cur);
            }

            // Volume
            float vol_factor = (distance <= 1.0f) ? 1.0f
                               : (distance >= max_range) ? 0.0f
                                 : (max_range - distance) / (max_range - 1.0f);
                                 
            float final_vol = ent->volume * vol_factor;
            ma_sound_set_volume(&slot->sound, final_vol);
        } else {
            if (ma_sound_is_playing(&slot->sound)) ma_sound_stop(&slot->sound);
        }
    }
}

void ResetLevelAudio(void) {
    loadedAmbients = 0;
    __builtin_memset(ambientRegistry, 0, loadedAmbients * sizeof(uint16_t));
    for (uint16_t i = START_INDEX_LEVEL_INSTANCES; i<loadedInstances;++i) {
        if (ConstIndexIsAmbient(instances[i].index)) {
            ambientRegistry[loadedAmbients] = i;
            loadedAmbients++;
            if (loadedAmbients >= MAX_AMBIENT_NOISES) { DualLogError("%u exceeded max number of ambient noises %u!\n",loadedAmbients,MAX_AMBIENT_NOISES); return; }
            
            instances[i].volume = entities[instances[i].index].volume * 0.5f;
        }
    }
    
    Sys_Music.levelEntry = true;             Sys_Music.inZone = Sys_Music.cyberTube = false;
    Sys_Music.clipFinished = Sys_Music.combatImpulseFinished = get_time(); Sys_Music.combatImpulseFinished += 5.0;
}

void PlayMenuMusic(void) { mp3_clear(); play_mp3("./Audio/music/TITLOOP-00_menu.mp3",1500); }
void PlayGameMusic(void) { mp3_clear(); play_mp3("./Audio/music/THM1-19_medicalstart.mp3",100); }

const char* GetCorrespondingLevelClip(TrackType ttype) {
    switch(ttype) { // Override types, return from these first before special level handling
        case TrackType_Revive:     return levelMusicRevive[Sys_Global.currentLevel];
        case TrackType_Death:      return levelMusicDeath[Sys_Global.currentLevel];
        case TrackType_Elevator:   return levelMusicElevator[Sys_Global.currentLevel];
        case TrackType_Distortion: return levelMusicDistortion[Sys_Global.currentLevel];
    }

    if (Sys_Global.currentLevel == 0 || Sys_Global.currentLevel == 5 || Sys_Global.currentLevel == 7) { // 0  REACTOR, 5 FLIGHT, 7 ENGINEERING
        if (Sys_Music.levelEntry)      return reactorMusic[6];
        if (ttype == TrackType_Combat) return reactorMusic[random_range_u8(0,6)];
        return reactorMusic[random_range_u8(6,13)];
    } else if (Sys_Global.currentLevel == 1) { // 1  MEDICAL
        if (Sys_Music.levelEntry) return medicalMusic[0];
        if (ttype == TrackType_Combat) return medicalMusic[random_range_u8(5,11)];
        return medicalMusic[random_range_u8(1,5)];
    } else if (Sys_Global.currentLevel == 2 || Sys_Global.currentLevel == 4) { // 2  SCIENCE, 4 STORAGE
        if (Sys_Music.levelEntry)      return scienceMusic[0];
        if (ttype == TrackType_Combat) return scienceMusic[random_range_u8(8,10)];
        return scienceMusic[random_range_u8(1,8)];
    } else if (Sys_Global.currentLevel == 8) { // 8 SECURITY
        if (Sys_Music.levelEntry)      return securityMusic[9];
        if (ttype == TrackType_Combat) return securityMusic[random_range_u8(0,6)];
        return securityMusic[random_range_u8(6,19)];
    } else if (Sys_Global.currentLevel == 6) { // 6 EXECUTIVE
        if (Sys_Music.levelEntry)      return executiveMusic[0];
        if (ttype == TrackType_Combat) return executiveMusic[random_range_u8(9,13)];
        return executiveMusic[random_range_u8(0,10)];
    } else if (Sys_Global.currentLevel == 10 || Sys_Global.currentLevel == 11 || Sys_Global.currentLevel == 12) { // 10, 12 GROVES
        if (Sys_Music.levelEntry)      return groveMusic[19];
        if (ttype == TrackType_Combat) return groveMusic[random_range_u8(0,9)];
        return executiveMusic[random_range_u8(9,24)];
    } else if (Sys_Global.currentLevel == 13) { // 13 CYBERSPACE
        if (Sys_Music.levelEntry)           return cyberMusic[0];
        if (Sys_Music.cyberTube)            return cyberMusic[random_range_u8(4,8)];
        if (random_range(0.0f,1.0f) < 0.5f) return cyberMusic[random_range_u8(1,5)];
        else                                return cyberMusic[8];
    }

    return levelMusicLooped[0];
}

void PlayTrack(TrackType ttype, MusicType mtype) {
    if (!Sys_Settings.DynamicMusic) { // Looped Music (Dynamic Music off)
        if (mtype == MusicType_Override) {
                 if (ttype == TrackType_Revive)     play_mp3(levelMusicRevive[Sys_Global.currentLevel],0);
            else if (ttype == TrackType_Death)      play_mp3(levelMusicDeath[Sys_Global.currentLevel],0);
            else if (ttype == TrackType_Elevator)   play_mp3(levelMusicElevator[Sys_Global.currentLevel],0);
            else if (ttype == TrackType_Distortion) play_mp3(levelMusicDistortion[Sys_Global.currentLevel],0);
        } else play_mp3(levelMusicLooped[Sys_Global.currentLevel],0);
        
        return;
    }

    // Normal Dynamic Music System
    if (mtype == MusicType_Override) mp3_clear();
    play_mp3(GetCorrespondingLevelClip(ttype),BUFFER_MS);
    if (!Sys_Music.elevator) Sys_Music.levelEntry = false; // already used by GetCorresponding... just now
}

void MusicNotifyZone(TrackType tt) {
    Sys_Music.inZone = true;
    switch(tt) {
        case TrackType_Elevator: Sys_Music.elevator = true; break;
        case TrackType_Distortion: Sys_Music.distortion = true; break;
    }
}

void MusicTriggerEnter(uint16_t self, uint16_t other) {
    if (instances[self].tickFinished < Sys_Global.pauseRelativeTime) { // Prevent flickering retrigger when player slides along glancing angle of trigger volume.
        if (other == PLAYER1 || other == PLAYER2) {
            PlayTrack(instances[self].trackType,instances[self].musicType);
            MusicNotifyZone(instances[self].trackType);
        }
        
        instances[self].tickFinished = Sys_Global.pauseRelativeTime + 0.1;
    }
}

void MusicTriggerExit(uint16_t other) {
    if (other == PLAYER1 || other == PLAYER2) { mp3_clear(); Sys_Music.inZone = Sys_Music.elevator = Sys_Music.distortion = false; } // return to normal upon leaving the trigger
}

void UpdateMusic(void) {
    ma_sound* curr = mp3_slot ? &mp3_sounds[1] : &mp3_sounds[0];
    bool currentIsPlaying = ma_sound_is_playing(curr);
    if (currentIsPlaying) {
        ma_uint64 currentFrame = ma_sound_get_time_in_pcm_frames(curr);
        ma_uint64 pcmFramesLength = 0;
        ma_sound_get_length_in_pcm_frames(curr,&pcmFramesLength);
        uint64_t deltaFrames = pcmFramesLength - currentFrame;
        float remaining = deltaFrames != 0 ? (float)deltaFrames / (float)ma_engine_get_sample_rate(&audio_engine) : 0.0f;
        if (remaining > AUD_BUFFER_T) return;
    }

    if (Sys_Music.inCombat && !Sys_Music.inZone && Sys_Music.combatImpulseFinished < Sys_Global.pauseRelativeTime) {
        Sys_Music.inCombat = false;
        PlayTrack(TrackType_Combat, MusicType_Override);
        Sys_Music.combatImpulseFinished = Sys_Global.pauseRelativeTime + 20.0;
        return;
    }

    if (Sys_Music.inZone) {
        if (Sys_Music.distortion) { PlayTrack(TrackType_Distortion, MusicType_Override); return; }
        if (Sys_Music.elevator) { PlayTrack(TrackType_Elevator, MusicType_Override); return; }
    }
    
    if (Sys_Settings.DynamicMusic) {
        if (currentIsPlaying) {
            ma_uint64 currentFrame = ma_sound_get_time_in_pcm_frames(curr);
            ma_uint64 pcmFramesLength = 0;
            ma_sound_get_length_in_pcm_frames(curr,&pcmFramesLength);
            uint64_t deltaFrames = pcmFramesLength - currentFrame;
            float remaining = deltaFrames != 0 ? (float)deltaFrames / (float)ma_engine_get_sample_rate(&audio_engine) : 0.0f;
            if (remaining <= AUD_BUFFER_T) PlayTrack(TrackType_Walking, MusicType_Walking);
        } else PlayTrack(TrackType_Walking, MusicType_Walking);
    } else PlayTrack(TrackType_Walking, MusicType_Walking);
}
