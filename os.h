// os.h - starts most translation units and defines the shim layer between Voxen and the OS as well as defining project wide OS defines.
#pragma once
typedef __SIZE_TYPE__ size_t;
typedef __UINTPTR_TYPE__ uintptr_t;
typedef __INTPTR_TYPE__ intptr_t;
typedef __INT8_TYPE__ i8;
typedef __INT16_TYPE__ i16;
typedef __INT32_TYPE__ i32;
typedef __UINT8_TYPE__ u8;
typedef __UINT16_TYPE__ u16;
typedef __UINT32_TYPE__ u32;
typedef __INT64_TYPE__ i64;
typedef __UINT64_TYPE__ u64;
#define UINT_MAX 0xffffffffU
#define INT_MAX 2147483647
#define bool unsigned char
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
#define O_RDONLY 00000000
#define O_WRONLY 00000001
#define O_RDWR 00000002
#define O_CREAT 00000100
#define O_TRUNC 00001000
#define O_CLOEXEC 02000000
#define O_NONBLOCK      00004000
#define O_DIRECTORY     00200000
// Interop - To Mod (keep the same as interop.h!!)
#if defined(_WIN32) || defined(__CYGWIN__)
    #define ENGINE_TO_MOD __declspec(dllexport) __cdecl
#else
    #define ENGINE_TO_MOD __attribute__((visibility("default")))
#endif
ENGINE_TO_MOD void DualLogError(const char* fmt, ...);
char* StringFindSubstring(const char* haystack, const char* needle);
void DebugRAM(const char *context);
static inline __attribute__((always_inline)) long OS_Open(const char* path, i32 flags, i32 mode) {
    register long rax __asm__("rax") = 2; // sys_open
    register const char* rdi __asm__("rdi") = path;
    register long rsi __asm__("rsi") = flags;
    register long rdx __asm__("rdx") = mode;
    __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi), "r"(rsi), "r"(rdx) : "rcx", "r11", "memory");
    return rax;
}

// #include <stddef.h> // For offsetof only
#include <pthread.h> // For model and texture loading only
#if defined(_WIN32) || defined(_WIN64)
    #define WINDOWS
    #define WIN32_LEAN_AND_MEAN // Let 'er rip, tater chip
    #define NOMINMAX
    #define VC_EXTRALEAN
    #define UNICODE
    #define OEMRESOURCE // OEM cursor resources for win init
    #define OCR_NORMAL 32512
    #define WINVER 0x0601 // Windows 7 or later
    #include <windows.h> // The things I do for my players, yeesh
    #include <direct.h>
    #include <winternl.h>
    #include <ntstatus.h>
    #include <io.h>
    #include <dwmapi.h>
    #include <dinput.h>
    #include <xinput.h>
    #include <dbt.h>
    #include <wchar.h>
    #include <mmdeviceapi.h>
    #include <audioclient.h>
    typedef HANDLE OsFileHandle;
    #define OS_INVALID_HANDLE INVALID_HANDLE_VALUE
    #define OS_MakeFolder(path) _mkdir(path)
    #undef near
    #undef far
    static inline __attribute__((always_inline, noreturn)) void OS_Exit(i64 exitCode) { ExitProcess((unsigned int)exitCode); __builtin_unreachable(); }
    static inline __attribute__((always_inline)) void OS_Close(OsFileHandle fileDescriptor) { CloseHandle(fileDescriptor); }
    static inline __attribute__((always_inline)) void* OS_AllocateRAM(void* addr, size_t length, i32 prot, i32 flags, OsFileHandle fd) {
        (void)flags;
        bool writable = (prot & PROT_WRITE);
        if (fd == INVALID_HANDLE_VALUE) { return (void*)VirtualAlloc(addr,length,MEM_RESERVE|MEM_COMMIT,writable ? PAGE_READWRITE : PAGE_READONLY); } // Handle standard memory allocations (MAP_ANONYMOUS)

        HANDLE hMap = CreateFileMapping(fd,NULL,writable ? PAGE_READWRITE : PAGE_READONLY,(DWORD)((u64)length >> 32),(DWORD)((u64)length & 0xFFFFFFFF),NULL);
        void* ptr = MapViewOfFileEx(hMap,writable ? FILE_MAP_WRITE : FILE_MAP_READ,0,0,length,addr); CloseHandle(hMap); return ptr;
    }
    
    static inline __attribute__((always_inline)) long OS_Read(OsFileHandle fd, void* buf, size_t count) { DWORD bytesRead = 0; return (ReadFile((HANDLE)fd,buf,(DWORD)count,&bytesRead,NULL)) ? (long)bytesRead : (long)-1; }
    static inline __attribute__((always_inline)) OsFileHandle OS_OpenReadonly(const char* filePath) {
        HANDLE fp = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
        if (fp == OS_INVALID_HANDLE) { DualLogError("Could not open file %s for reading\n",filePath); return OS_INVALID_HANDLE; }
        return fp;
    }

    static inline __attribute__((always_inline)) OsFileHandle OS_OpenWriteonly(const char* filePath) {
        OsFileHandle h = CreateFileA(filePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (h == OS_INVALID_HANDLE) { DualLogError("Failed to open %s for writing\n", filePath); return OS_INVALID_HANDLE; }
        return h;
    }

    static inline __attribute__((always_inline)) int OS_FileSize(OsFileHandle fileDescriptor) {
        if (fileDescriptor == OS_INVALID_HANDLE) return -1;
        
        LARGE_INTEGER size;
        if (!GetFileSizeEx(fileDescriptor, &size)) return -1;
        return size.QuadPart;
    }

    static inline __attribute__((always_inline)) void* OS_AllocateFileBackedRAMReadonly(size_t size, OsFileHandle fileDescriptor, char* filePath) {
        if (fileDescriptor == OS_INVALID_HANDLE || size == 0) return NULL;

        HANDLE hMapping = CreateFileMappingA(fileDescriptor, NULL, PAGE_READONLY, 0, 0, NULL);
        if (hMapping == NULL) { DualLogError("CreateFileMapping failed for %s (err %lu)\n", filePath, GetLastError()); return NULL; }

        void* ramSpacePointer = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, size);
        CloseHandle(hMapping);  // view keeps reference alive
        if (ramSpacePointer == NULL) { DualLogError("Failed to allocate %s (err %lu)\n", filePath, GetLastError()); return NULL; }
        return ramSpacePointer;
    }
    
    static inline __attribute__((always_inline)) i64 OS_Seek(OsFileHandle fd, i64 offset, int whence) { // forth and forsooth pray tell
        LARGE_INTEGER li, new_pos; li.QuadPart = offset;
        if (!SetFilePointerEx((HANDLE)fd,li,&new_pos,(u32)whence)) return -1;
        return new_pos.QuadPart;
    }

    static inline __attribute__((always_inline)) i64 OS_Tell(OsFileHandle fd) {
        LARGE_INTEGER li = {0}, current_pos;
        if (!SetFilePointerEx((HANDLE)fd, li, &current_pos, 1 /* FILE_CURRENT */)) return -1;
        return current_pos.QuadPart;
    }

    static inline __attribute__((always_inline)) int OS_GetNumThreads(void) { SYSTEM_INFO si; GetSystemInfo(&si); return (int)si.dwNumberOfProcessors; }
    static inline __attribute__((always_inline)) void OS_DeallocateRAM(void* ramSpacePointer, size_t size) {    
        (void)size;
        if (!ramSpacePointer) { DualLogError("Attempting to double free!\n"); OS_Exit(1); }
        
        if (!UnmapViewOfFile(ramSpacePointer)) { if (VirtualFree(ramSpacePointer, 0 /* dwSize (must be 0 with MEM_RELEASE) */, MEM_RELEASE) == 0) DualLogError("VirtualFree failed\n"); }
    }
    
    static inline __attribute__((always_inline)) i64 OS_RawWrite(OsFileHandle fd, const void* buf, size_t count) {
        DWORD written = 0;
        if (WriteFile((HANDLE)fd,buf,(DWORD)count,&written,NULL)) return (i64)written;
        return -1;
    }
        
    // Gamecode loading:
    #define MOD_EXTENSION ".dll" // e.g. Citadel.dll
    #define OS_DlOpen(path)       LoadLibraryA(path)
    #define OS_DlSym(handle,name) GetProcAddress((handle),(name))
    static char win_err_buf[512];
#else
    #define LINUX
    #include <sys/ioctl.h>
    #include <sound/asound.h>
    void *dlopen(const char *filename, int flags); void *dlsym(void *handle, const char *symbol);
    #define RTLD_LOCAL 0x00000
    #define RTLD_LAZY 0x00001
    #define RTLD_NOW 0x00002
    typedef long ssize_t;
    typedef unsigned int mode_t; typedef long off_t; typedef u64 dev_t,ino_t; typedef long unsigned int nlink_t; typedef u32 uid_t,gid_t; typedef i64 blksize_t,blkcnt_t;
    struct input_id { u16 bustype,vendor,product,version;};
    struct input_absinfo {i32 value,minimum,maximum,fuzz,flat,resolution;};
    struct input_event { struct { long tv_sec,tv_usec; } time; u16 type,code; i32 value; };
    #define EV_CNT    0x20
    #define KEY_CNT   0x300
    #define ABS_CNT   0x40
    #define EV_KEY    0x01
    #define EV_ABS    0x03
    #define BTN_MISC  0x100
    #define ABS_HAT0X 0x10
    #define ABS_HAT3Y 0x17
    #define EVIOCGID 0x80084501 
    #define EVIOCGABS(abs)  (0x80184540 + (abs))
    #define EVIOCGBIT(ev, len) (0x80004520 + (ev) + ((len) << 16))
    #define EVIOCGNAME(len) (0x80004506 | (((len) & 0x1fff) << 16))
    #define EV_SYN       0x00
    #define SYN_REPORT   0
    #define SYN_CONFIG   1
    #define SYN_MT_REPORT 2
    #define SYN_DROPPED  3
    typedef int OsFileHandle;
    #define OS_INVALID_HANDLE -1
    typedef int wchar_t;
    static inline int OS_IOControl(int fd, unsigned long request, void *arg) {
        register long rax __asm__("rax") = 16;
        register int  rdi __asm__("rdi") = fd;
        register long rsi __asm__("rsi") = request;
        register void *rdx __asm__("rdx") = arg;
        __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi), "r"(rsi), "r"(rdx) : "rcx", "r11", "memory");
        return (int)rax;
    }
    static inline int OS_IOControlSimple(int fd, unsigned long request) { return OS_IOControl(fd,request,0); }
        
    static inline __attribute__((always_inline)) int OS_MakeFolder(const char *path) {
        register long rax __asm__("rax") = 83;
        register const char *rdi __asm__("rdi") = path;
        register mode_t rsi __asm__("rsi") = 0755;
        __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi), "r"(rsi) : "rcx", "r11", "memory");
        return (int)rax;
    }
    
    //int brk(void *addr); void *sbrk(intptr_t increment);
    static inline __attribute__((always_inline)) void* OS_Brk(void* addr) { register long rax __asm__("rax") = 12; register void* rdi __asm__("rdi") = addr; __asm__ __volatile__("syscall":"+r"(rax):"r"(rdi):"rcx","r11","memory"); return (void*)rax; }
    static inline __attribute__((always_inline)) long OS_Read(long fd, void* buf, size_t count) {
        register long rax __asm__("rax") = 0;
        register long rdi __asm__("rdi") = fd;
        register void* rsi __asm__("rsi") = buf;
        register size_t rdx __asm__("rdx") = count;
        __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi), "r"(rsi), "r"(rdx) : "rcx", "r11", "memory");
        return rax;
    }
    static inline __attribute__((always_inline, noreturn)) void OS_Exit(i64 exitCode) { register i64 rax __asm__("rax") = 231; register i64 rdi __asm__("rdi") = exitCode; __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi) : "rcx", "r11", "memory"); __builtin_unreachable(); }
    static inline __attribute__((always_inline)) void OS_Close(OsFileHandle fileDescriptor) { register long rax __asm__("rax") = 3; register long rdi __asm__("rdi") = fileDescriptor; __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi) : "rcx", "r11", "memory"); }
    static inline __attribute__((always_inline)) void* OS_AllocateRAM(void* addr, size_t length, i32 prot, i32 flags, OsFileHandle fd) {
        register long rax __asm__("rax") = 9; register void* rdi __asm__("rdi") = addr; register size_t rsi __asm__("rsi") = length;
        register int rdx __asm__("rdx") = prot; register int r10 __asm__("r10") = flags; register int r8 __asm__("r8") = fd;
        register off_t r9 __asm__("r9") = 0; // Offset
        __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi), "r"(rsi), "r"(rdx), "r"(r10), "r"(r8), "r"(r9) : "rcx", "r11", "memory");
        return (void*)rax;
    }
    
    static inline __attribute__((always_inline)) OsFileHandle OS_OpenReadonly(const char* filePath) {
        OsFileHandle fp = OS_Open(filePath,O_RDONLY,0);
        if (fp < 0) { DualLogError("Could not open file %s for reading\n",filePath); return OS_INVALID_HANDLE; }
        return fp;
    }

    static inline __attribute__((always_inline)) OsFileHandle OS_OpenWriteonly(const char* filePath) {
        OsFileHandle h = OS_Open(filePath,O_WRONLY|O_CREAT|O_TRUNC,0644);
        if (h < 0) { DualLogError("Failed to open %s for writing\n", filePath); return OS_INVALID_HANDLE; }
        return h;
    }

    static inline __attribute__((always_inline)) int OS_FileSize(OsFileHandle fileDescriptor) {
        struct timespec { i64 tv_sec; long tv_nsec; };
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
        struct stat fileStatisticsStruct;        
        register long rax __asm__("rax") = 5;
        register int rdi __asm__("rdi") = fileDescriptor;
        register struct stat *rsi __asm__("rsi") = &fileStatisticsStruct;
        __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi), "r"(rsi) : "rcx", "r11", "memory");
        return fileStatisticsStruct.st_size;
    }

    static inline __attribute__((always_inline)) void* OS_AllocateFileBackedRAMReadonly(size_t size, OsFileHandle fileDescriptor, char* filePath) {
        void* ramSpacePointer = OS_AllocateRAM(NULL, size, PROT_READ, MAP_PRIVATE, fileDescriptor);
        if (ramSpacePointer == MAP_FAILED) { DualLogError("Failed to allocate %s\n", filePath); return NULL; }
        return ramSpacePointer;
    }
    
    static inline __attribute__((always_inline)) i64 OS_Seek(OsFileHandle fd, i64 offset, int whence) { // forth and forsooth pray tell
        register i64 rax __asm__("rax")=8, rdi __asm__("rdi")=fd, rsi __asm__("rsi")=offset, rdx __asm__("rdx")=whence;
        __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi), "r"(rsi), "r"(rdx) : "rcx", "r11", "memory");
        return rax;
    }

    static inline __attribute__((always_inline)) i64 OS_Tell(OsFileHandle fd) {
        register i64 rax __asm__("rax")=8, rdi __asm__("rdi")=fd, rsi __asm__("rsi")=0, rdx __asm__("rdx") =1;
        __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi), "r"(rsi), "r"(rdx) : "rcx", "r11", "memory");
        return rax;
    }

    static inline __attribute__((always_inline)) int OS_GetNumThreads(void) {
        unsigned long mask[16]; // Supports up to 1024 CPUs (16 * 64 bits)
        long rax = 204;         /* sched_getaffinity syscall number */
        long rdi = 0;           /* pid 0 = current process */
        long rsi = 128;         /* size of mask in bytes (16 * 8) */
        long rdx = (long)mask;
        __asm__ __volatile__("syscall":"+a"(rax):"D"(rdi),"S"(rsi),"d"(rdx):"rcx","r11","memory");
        if (rax < 0) return 1; // Fallback to 1 on error

        int count = 0;
        int num_longs = rax / sizeof(unsigned long);
        for (int i = 0; i < num_longs; i++) count += __builtin_popcountll(mask[i]);
        return count;
    }
    
    static inline __attribute__((always_inline)) void OS_DeallocateRAM(void* ramSpacePointer, size_t size) {    
        if (!ramSpacePointer || ramSpacePointer == MAP_FAILED) { DualLogError("Attempting to double free!\n"); OS_Exit(1); }
        
        register long rax __asm__("rax") = 11;
        register void* rdi __asm__("rdi") = ramSpacePointer;
        register size_t rsi __asm__("rsi") = size;
        __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi), "r"(rsi) : "rcx", "r11", "memory");
        if ((void *)(i64)rax == MAP_FAILED) DualLogError("munmap failed\n");
    }
    
    static inline __attribute__((always_inline)) i64 OS_RawWrite(OsFileHandle fd, const void* buf, size_t count) {
        register i64     rax __asm__("rax") = 1; // sys_write
        register i32     rdi __asm__("rdi") = fd;
        register const void* rsi __asm__("rsi") = buf;
        register size_t      rdx __asm__("rdx") = count;
        __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi), "r"(rsi), "r"(rdx) : "rcx", "r11", "memory");
        return rax;
    }

    // Gamecode loading:
    #define MOD_EXTENSION ".so" // e.g. Citadel.so
    #define OS_DlOpen(path)       dlopen((path),RTLD_NOW)
    #define OS_DlSym(handle,name) dlsym((handle),(name))
#endif
static inline __attribute__((always_inline)) void* OS_Alloc(size_t amount) { return OS_AllocateRAM(NULL,amount,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,OS_INVALID_HANDLE); }
static inline __attribute__((always_inline)) void* OS_Calloc(size_t amount, size_t count) { return OS_AllocateRAM(NULL,amount * count,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,OS_INVALID_HANDLE); }
static inline __attribute__((always_inline)) void OS_Write(OsFileHandle fd, const void* buffer, size_t size, const char* filePath) {
    size_t total = 0;
    while (total < size) {
        i64 written = OS_RawWrite(fd,(const char*)buffer + total,size - total);
        if (written < 0) { DualLogError("Write error when attempting write to %s: %s (code: %d)\n",filePath,written,(i32)(-written)); OS_Exit(1); }

        total += (size_t)written;
    }
}

static inline __attribute__((always_inline)) void* OS_OpenAndAllocateFileBufferReadonly(const char* filePath, OsFileHandle* fileDescriptor, int* size) {
    *fileDescriptor = OS_OpenReadonly(filePath);
    if (*fileDescriptor == OS_INVALID_HANDLE) { *size = 0; return (void*)0; }
    
    *size = (int)OS_FileSize(*fileDescriptor);
    if (*size <= 0) { DualLogError("Warning: File %s is empty, skipping allocation.\n", filePath); OS_Close(*fileDescriptor); OS_Exit(1); }
    
    void* ramSpacePointer = OS_AllocateFileBackedRAMReadonly(*size,*fileDescriptor,(char*)filePath);
    OS_Close(*fileDescriptor);
    return ramSpacePointer;
}

static inline __attribute__((always_inline)) void* OSCopyMemoryFromBtoAForNBytes(void *dst, const void *src, size_t n) { unsigned char *d=(unsigned char *)dst; const unsigned char *s=(const unsigned char *)src; while (n--) {*d++=*s++;} return dst; } // memcpy replacement
static inline __attribute__((always_inline)) void* OS_Realloc(void* oldPtr, size_t oldSize, size_t newSize) {
    if (oldPtr == NULL) return OS_Alloc(newSize);
    if (newSize <= oldSize) return oldPtr;

    void* newPtr = OS_Alloc(newSize);
    OSCopyMemoryFromBtoAForNBytes(newPtr,oldPtr,oldSize);
    OS_DeallocateRAM(oldPtr,oldSize);
    return newPtr;
}
