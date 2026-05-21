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
#define bool unsigned char
#define true 1
#define false 0
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#define NULL ((void *)0)
#if defined(_WIN32) // Interop - To Mod (keep the same as interop.h!!)
    #define ENGINE_TO_MOD __declspec(dllexport) __cdecl
#else
    #define ENGINE_TO_MOD __attribute__((visibility("default")))
#endif
ENGINE_TO_MOD void DualLogError(const char* fmt, ...);
char* StringFindSubstring(const char* haystack, const char* needle);
void DebugRAM(const char *context);
#if defined(_WIN32)
    #define WINDOWS
    #define DECLSPEC_IMPORT __declspec (dllimport)
    #define WINAPI __stdcall
    #define OS_INVALID_HANDLE INVALID_HANDLE_VALUE
    #define ERROR_SUCCESS __MSABI_LONG(0)
    #define ERROR_DEVICE_NOT_CONNECTED __MSABI_LONG(1167)
    #define SUCCEEDED(hr) ((HRESULT)(hr) >= 0)
    #define FAILED(hr) ((HRESULT)(hr) < 0)
    typedef char CHAR; typedef unsigned char BYTE,UCHAR; typedef unsigned short WORD; typedef unsigned int ULONG; typedef long LONG; typedef long long LONGLONG; typedef long long INT_PTR,*PINT_PTR,LONG_PTR,*PLONG_PTR;
    typedef void *HANDLE,*LPVOID,*PVOID; typedef const void *LPCVOID; typedef const CHAR *LPCSTR,*PCSTR,*LPCCH,*PCCH;
    typedef unsigned long long ULONG_PTR,*PULONG_PTR,UINT_PTR,*PUINT_PTR; typedef unsigned short wchar_t; typedef wchar_t WCHAR; typedef const WCHAR *LPCWSTR,*PCWSTR;
    typedef unsigned long DWORD; typedef WORD *PWORD,*LPWORD; typedef ULONG_PTR DWORD_PTR,*PDWORD_PTR,SIZE_T,*PSIZE_T; typedef DWORD *PDWORD,*LPDWORD; typedef LONG HRESULT; typedef long long REFERENCE_TIME;
    typedef struct _GUID { unsigned long Data1; unsigned short Data2,Data3; unsigned char Data4[8]; } GUID; typedef GUID IID,CLSID;
    #define DECLARE_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name
    DECLARE_HANDLE(HINSTANCE);
    typedef HINSTANCE HMODULE;
    typedef INT_PTR (WINAPI *FARPROC)(void);
    typedef INT_PTR (WINAPI *NEARPROC)(void);
    typedef INT_PTR (WINAPI *PROC)(void);
    #if defined (__WIDL__)
    typedef struct _LARGE_INTEGER {
    #else
        typedef union _LARGE_INTEGER {
        struct {DWORD LowPart; LONG HighPart;} DUMMYSTRUCTNAME; struct {DWORD LowPart; LONG HighPart;} u;
    #endif
        LONGLONG QuadPart;
    } LARGE_INTEGER;
    typedef LARGE_INTEGER *PLARGE_INTEGER; typedef HANDLE OsFileHandle;
    typedef struct _SECURITY_ATTRIBUTES { DWORD nLength; LPVOID lpSecurityDescriptor; i32 bInheritHandle; } SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;
    typedef struct _OVERLAPPED { ULONG_PTR Internal; ULONG_PTR InternalHigh; union {struct {DWORD Offset; DWORD OffsetHigh;} DUMMYSTRUCTNAME; PVOID Pointer;} DUMMYUNIONNAME; HANDLE hEvent; } OVERLAPPED, *LPOVERLAPPED;
    typedef struct _SYSTEM_INFO { union { DWORD dwOemId; struct { WORD wProcessorArchitecture; WORD wReserved; } DUMMYSTRUCTNAME; } DUMMYUNIONNAME; DWORD dwPageSize; LPVOID lpMinimumApplicationAddress; LPVOID lpMaximumApplicationAddress; DWORD_PTR dwActiveProcessorMask; DWORD dwNumberOfProcessors; DWORD dwProcessorType; DWORD dwAllocationGranularity; WORD wProcessorLevel; WORD wProcessorRevision; } SYSTEM_INFO, *LPSYSTEM_INFO;
    DECLSPEC_IMPORT __declspec (noreturn) void WINAPI ExitProcess (u32 uExitCode);
    DECLSPEC_IMPORT HANDLE WINAPI CreateFileMappingA (HANDLE hFile, LPSECURITY_ATTRIBUTES lpFileMappingAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCSTR lpName);
    DECLSPEC_IMPORT i32 WINAPI VirtualFree (LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType);
    DECLSPEC_IMPORT LPVOID WINAPI VirtualAlloc (LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
    DECLSPEC_IMPORT HANDLE WINAPI CreateFileA (LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
    DECLSPEC_IMPORT i32 WINAPI ReadFile (HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
    DECLSPEC_IMPORT i32 WINAPI WriteFile (HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
    DECLSPEC_IMPORT i32 WINAPI GetFileSizeEx (HANDLE hFile, PLARGE_INTEGER lpFileSize);
    DECLSPEC_IMPORT i32 WINAPI SetFilePointerEx (HANDLE hFile, LARGE_INTEGER liDistanceToMove, PLARGE_INTEGER lpNewFilePointer, DWORD dwMoveMethod);
    #define INVALID_HANDLE_VALUE ((HANDLE) (LONG_PTR)-1)
    DECLSPEC_IMPORT i32 WINAPI CloseHandle (HANDLE hObject);
    DECLSPEC_IMPORT LPVOID WINAPI MapViewOfFile (HANDLE hFileMappingObject, DWORD dwDesiredAccess, DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow, SIZE_T dwNumberOfBytesToMap);
    DECLSPEC_IMPORT LPVOID WINAPI MapViewOfFileEx (HANDLE hFileMappingObject, DWORD dwDesiredAccess, DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow, SIZE_T dwNumberOfBytesToMap, LPVOID lpBaseAddress);
    DECLSPEC_IMPORT i32 WINAPI UnmapViewOfFile (LPCVOID lpBaseAddress);
    DECLSPEC_IMPORT HANDLE WINAPI CreateFileMappingW (HANDLE hFile, LPSECURITY_ATTRIBUTES lpFileMappingAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCWSTR lpName);
    DECLSPEC_IMPORT HANDLE WINAPI GetStdHandle(DWORD nStdHandle);
    DECLSPEC_IMPORT i32 WINAPI QueryPerformanceCounter (LARGE_INTEGER *lpPerformanceCount);
    DECLSPEC_IMPORT i32 WINAPI QueryPerformanceFrequency (LARGE_INTEGER *lpFrequency);
    DECLSPEC_IMPORT void WINAPI GetSystemInfo (LPSYSTEM_INFO lpSystemInfo);
    DECLSPEC_IMPORT HMODULE WINAPI LoadLibraryA(LPCSTR lpLibFileName);
    DECLSPEC_IMPORT FARPROC WINAPI GetProcAddress (HMODULE hModule, LPCSTR lpProcName);
    int __cdecl _mkdir(const char* dirname);
    static inline __attribute__((always_inline, noreturn)) void OS_Exit(i64 exitCode) { ExitProcess((unsigned int)exitCode); __builtin_unreachable(); }
    static inline __attribute__((always_inline)) long OS_Open(const char* path, i32 flags, i32 m) { (void)m; void* h = CreateFileA(path,flags ? 0x40000000L : 0x80000000L, flags ? 0 : 1,0,flags ? 2 : 3,128,0); return h==(void*)-1 ? -1 : (long)(uintptr_t)h; }
    static inline __attribute__((always_inline)) void OS_Close(OsFileHandle fileDescriptor) { CloseHandle(fileDescriptor); }
    static inline __attribute__((always_inline)) void* OS_AllocateRAM(void* a,size_t l,i32 p,i32 f,OsFileHandle fd) { (void)f; if (fd==(HANDLE)-1) return VirtualAlloc(a,l,0x3000,(p&2)?4:2); HANDLE m = CreateFileMappingW(fd,NULL,(p&2) ? 4 : 2,(DWORD)(l>>32),(DWORD)l,NULL); void* r=MapViewOfFileEx(m,(p&2)?2:4,0,0,l,a); return CloseHandle(m),r;}    
    #define OS_MakeFolder(path) _mkdir(path)
    static inline __attribute__((always_inline)) long OS_Read(OsFileHandle fd, void* buf, size_t count) { DWORD bytesRead = 0; return (ReadFile((HANDLE)fd,buf,(DWORD)count,&bytesRead,NULL)) ? (long)bytesRead : (long)-1; }
    static inline __attribute__((always_inline)) OsFileHandle OS_OpenReadonly(const char* path) { HANDLE f = CreateFileA(path,0x80000000L,1,NULL,3,0x08000080,NULL); return f == (HANDLE)-1 ? DualLogError("Could not open file %s for reading\n",path), (HANDLE)-1 : f; }
    static inline __attribute__((always_inline)) OsFileHandle OS_OpenWriteonly(const char* path) { OsFileHandle h = CreateFileA(path,0x40000000L,0,NULL,2,128,NULL); return h == (HANDLE)-1 ? DualLogError("Failed to open %s for writing\n",path),(HANDLE)-1 : h; }
    static inline __attribute__((always_inline)) int OS_FileSize(OsFileHandle f) { LARGE_INTEGER s; return (f==(OsFileHandle)-1 || !GetFileSizeEx(f,&s)) ? -1 : (int)s.QuadPart; }
    static inline __attribute__((always_inline)) void* OS_AllocateFileBackedRAMReadonly(size_t s,OsFileHandle fd, char* path) { HANDLE m; void* r; return(fd==(HANDLE)-1||!s||!(m=CreateFileMappingA(fd,NULL,2,0,0,NULL))) ? DualLogError("CreateFileMapping failed for %s\n",path),NULL : (r=MapViewOfFile(m,4,0,0,s)) ? (CloseHandle(m),r) : (DualLogError("Failed to allocate %s\n",path),CloseHandle(m),NULL);}
    static inline __attribute__((always_inline)) i64 OS_Seek(OsFileHandle fd, i64 ofs, int whence /*forth and forsooth pray tell*/) { LARGE_INTEGER l={.QuadPart=ofs},n; return SetFilePointerEx((HANDLE)fd,l,&n,whence) ? n.QuadPart : -1; }
    static inline __attribute__((always_inline)) i64 OS_Tell(OsFileHandle fd) { LARGE_INTEGER l={0},n; return SetFilePointerEx((HANDLE)fd,l,&n,1) ? n.QuadPart : -1; }
    static inline __attribute__((always_inline)) int OS_GetNumThreads(void) { SYSTEM_INFO si; GetSystemInfo(&si); return (int)si.dwNumberOfProcessors; }
    static inline __attribute__((always_inline)) void OS_DeallocateRAM(void* p, size_t s) { (void)s; if(!p) { DualLogError("Attempting to double free!\n"); OS_Exit(1); } if(!UnmapViewOfFile(p) && !VirtualFree(p,0,0x00008000)) DualLogError("VirtualFree failed\n"); }
    static inline __attribute__((always_inline)) i64 OS_RawWrite(OsFileHandle fd, const void* buf, size_t count) { DWORD w; return WriteFile((HANDLE)fd,buf,(DWORD)count,&w,NULL) ? (i64)w : -1; }
    #define MOD_EXTENSION ".dll" // e.g. Citadel.dll
    #define OS_DlOpen(path)       LoadLibraryA(path)
    #define OS_DlSym(handle,name) GetProcAddress((handle),(name))
    static char win_err_buf[512];
    typedef long long	__time64_t;
    typedef __time64_t time_t;
    struct timespec { time_t tv_sec; long tv_nsec; };
    struct sched_param { int sched_priority; };
    typedef uintptr_t pthread_t;
    typedef struct pthread_attr_t { unsigned p_state; void *stack; size_t s_size; struct sched_param param; } pthread_attr_t;
    int pthread_create(pthread_t *th, const pthread_attr_t *attr, void *(* func)(void *), void *arg);
    int pthread_join(pthread_t t, void **res);
#else
    #define LINUX
    void *dlopen(const char *filename, int flags); void *dlsym(void *handle, const char *symbol);
    typedef unsigned int mode_t; typedef long off_t; typedef u64 dev_t,ino_t; typedef long unsigned int nlink_t; typedef u32 uid_t,gid_t; typedef i64 blksize_t,blkcnt_t;
    struct input_id { u16 bustype,vendor,product,version;};
    struct input_absinfo {i32 value,minimum,maximum,fuzz,flat,resolution;};
    struct input_event { struct { long tv_sec,tv_usec; } time; u16 type,code; i32 value; };
    typedef int OsFileHandle;
    #define OS_INVALID_HANDLE -1
    typedef int wchar_t;
    typedef long time_t;
    typedef unsigned long int pthread_t;
    static inline int OS_IOControl(int fd, unsigned long req, void* arg) { long r = 16; __asm__ __volatile__("syscall":"+a"(r):"D"(fd),"S"(req),"d"(arg):"rcx","r11","memory"); return (int)r; }
    static inline int OS_IOControlSimple(int fd, unsigned long request) { return OS_IOControl(fd,request,0); }
    static inline __attribute__((always_inline)) int OS_MakeFolder(const char* path) { long r = 83; __asm__ __volatile__("syscall":"+a"(r):"D"(path),"S"(0755LL):"rcx","r11","memory"); return (int)r; }
    static inline __attribute__((always_inline)) void* OS_Brk(void* addr) { register uintptr_t rax __asm__("rax") = 12; register void* rdi __asm__("rdi") = addr; __asm__ __volatile__("syscall":"+r"(rax):"r"(rdi):"rcx","r11","memory"); return (void*)rax; }
    static inline __attribute__((always_inline)) long OS_Read(long f,void*b,size_t c) { long r = 0; __asm__ __volatile__("syscall":"+a"(r):"D"(f),"S"(b),"d"(c):"rcx","r11","memory"); return r; }
    static inline __attribute__((always_inline, noreturn)) void OS_Exit(i64 exitCode) { register i64 rax __asm__("rax") = 231; register i64 rdi __asm__("rdi") = exitCode; __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi) : "rcx", "r11", "memory"); __builtin_unreachable(); }
    static inline __attribute__((always_inline)) void OS_Close(OsFileHandle fileDescriptor) { register long rax __asm__("rax") = 3; register long rdi __asm__("rdi") = fileDescriptor; __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi) : "rcx", "r11", "memory"); }
    static inline __attribute__((always_inline)) long OS_Open(const char* path, i32 flags, i32 mode) { long r = 2; __asm__ __volatile__("syscall":"+a"(r):"D"(path),"S"((long)flags),"d"((long)mode):"rcx","r11","memory"); return r; }
    static inline __attribute__((always_inline)) void* OS_AllocateRAM(void* addr, size_t len, i32 prot, i32 flags, OsFileHandle fd){ long r=9; register int r10 __asm__("r10")=flags; register int r8 __asm__("r8")=fd; register long r9 __asm__("r9")=0; __asm__ __volatile__("syscall":"+a"(r):"D"(addr),"S"(len),"d"(prot),"r"(r10),"r"(r8),"r"(r9):"rcx","r11","memory"); return (void*)r; }
    static inline __attribute__((always_inline)) OsFileHandle OS_OpenReadonly(const char* path) { OsFileHandle f=OS_Open(path,0,0); return f < 0 ? DualLogError("Could not open file %s for reading\n",path), -1 : f; }
    static inline __attribute__((always_inline)) OsFileHandle OS_OpenWriteonly(const char* path) { OsFileHandle f=OS_Open(path,1|00000100|00001000,0644); return f < 0 ? DualLogError("Failed to open %s for writing\n",path),-1 : f; }
    static inline __attribute__((always_inline)) int OS_FileSize(OsFileHandle f) { long r=5,s[18]; __asm__ __volatile__("syscall":"+a"(r):"D"(f),"S"(s):"rcx","r11","memory"); return (int)s[6]; }
    static inline __attribute__((always_inline)) void* OS_AllocateFileBackedRAMReadonly(size_t s, OsFileHandle fd, char* path) { void* r=OS_AllocateRAM(NULL,s,1,2,fd); return r==(void*)-1 ? DualLogError("Failed to allocate %s\n",path),NULL : r; }
    static inline __attribute__((always_inline)) i64 OS_Seek(OsFileHandle fd, i64 ofs, int whence /* forth and forsooth pray tell*/) { i64 r = 8; __asm__ __volatile__("syscall":"+a"(r):"D"(fd),"S"(ofs),"d"(whence):"rcx","r11","memory"); return r; }
    static inline __attribute__((always_inline)) i64 OS_Tell(OsFileHandle fd) { i64 r=8; __asm__ __volatile__("syscall":"+a"(r):"D"(fd),"S"(0LL),"d"(1):"rcx","r11","memory"); return r; }
    static inline __attribute__((always_inline)) int OS_GetNumThreads(void) { unsigned long m[16]; long r=204; __asm__ __volatile__("syscall":"+a"(r):"D"(0LL),"S"(128LL),"d"(m):"rcx","r11","memory"); int c = 0; for(int i=0;i<(r/8);i++) {c+=__builtin_popcountll(m[i]);} return r < 0 ? 1 : c; }
    static inline __attribute__((always_inline)) void OS_DeallocateRAM(void* p,size_t s){ long r=11; if(!p || p == (void*)-1) { DualLogError("Attempting to double free!\n"); OS_Exit(1); } __asm__ __volatile__("syscall":"+a"(r):"D"(p),"S"(s):"rcx","r11","memory"); if(r<0) DualLogError("munmap failed\n"); }
    static inline __attribute__((always_inline)) i64 OS_RawWrite(OsFileHandle fd, const void* buf, size_t cnt) { i64 r=1; __asm__ __volatile__("syscall":"+a"(r):"D"(fd),"S"(buf),"d"(cnt):"rcx","r11","memory"); return r; }
    #define MOD_EXTENSION ".so" // e.g. Citadel.so
    #define OS_DlOpen(path)       dlopen((path),2)
    #define OS_DlSym(handle,name) dlsym((handle),(name))
    struct timespec { time_t tv_sec; long tv_nsec; };
    typedef struct { unsigned int flags; void* stack; } pthread_attr_t;
    int pthread_create(pthread_t* restrict thread, const pthread_attr_t* restrict attr, void* (*start_routine)(void*), void* restrict arg);
    int pthread_join(pthread_t thread, void** retval);
#endif
static inline __attribute__((always_inline)) void* OS_Alloc(size_t amount) { return OS_AllocateRAM(NULL,amount,0x1|0x2,0x02|0x20,OS_INVALID_HANDLE); }
static inline __attribute__((always_inline)) void* OS_Calloc(size_t amount, size_t count) { return OS_AllocateRAM(NULL,amount * count,0x1|0x2,0x02|0x20,OS_INVALID_HANDLE); }
static inline __attribute__((always_inline)) void OS_Write(OsFileHandle f,const void* buf, size_t s, const char* p) { size_t total=0; while(total < s) { i64 w=OS_RawWrite(f,(const char*)buf + total,s - total); if(w < 0) { DualLogError("Write error to %s: %s[%d]\n",p,w,(i32)-w); OS_Exit(1); } total += (size_t)w; } }
static inline __attribute__((always_inline)) void* OS_OpenAndAllocateFileBufferReadonly(const char* p,OsFileHandle* f,int* s){void* r;return((*f=OS_OpenReadonly(p))==(OsFileHandle)-1)?*s=0,(void*)0:((*s=OS_FileSize(*f))<=0)?DualLogError("Skipping empty:%s\n",p),OS_Close(*f),OS_Exit(1),NULL:(r=OS_AllocateFileBackedRAMReadonly(*s,*f,(char*)p))?(OS_Close(*f),r):NULL;}
static inline __attribute__((always_inline)) void* OSCopyMemoryFromBtoAForNBytes(void *dst, const void *src, size_t n) { unsigned char *d=(unsigned char *)dst; const unsigned char *s=(const unsigned char *)src; while (n--) {*d++=*s++;} return dst; } // memcpy replacement
static inline __attribute__((always_inline)) void* OS_Realloc(void* old, size_t olds, size_t news) { void* n; return !old ? OS_Alloc(news) : news <= olds ? old : (n=OS_Alloc(news)) ? (OSCopyMemoryFromBtoAForNBytes(n,old,olds),OS_DeallocateRAM(old,olds),n) : 0; }
