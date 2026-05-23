// os.h - starts most translation units and defines the shim layer between Voxen and the OS as well as defining project wide OS defines.
#pragma once
typedef __INT8_TYPE__ i8;     typedef __UINT8_TYPE__ u8;   typedef __INT16_TYPE__ i16;   typedef __UINT16_TYPE__ u16;        typedef __INT32_TYPE__ i32;   typedef __UINT32_TYPE__ u32;
typedef __INT64_TYPE__ i64;   typedef __UINT64_TYPE__ u64; typedef __SIZE_TYPE__ size_t; typedef __UINTPTR_TYPE__ uintptr_t; typedef __INTPTR_TYPE__ intptr_t;
#define bool unsigned char
#define true 1
#define false 0
#define likely(x)   __builtin_expect(!!(x),1)
#define unlikely(x) __builtin_expect(!!(x),0)
#define NULL ((void *)0)
#if defined(_WIN32) // Interop - To Mod (keep the same as interop.h!!)
    #define ENGINE_TO_MOD __declspec(dllexport) __cdecl
#else
    #define ENGINE_TO_MOD __attribute__((visibility("default")))
#endif
ENGINE_TO_MOD void DualLogError(const char* fmt, ...); char* StringFindSubstring(const char* haystack, const char* needle);
#if defined(_WIN32)
    #define WINDOWS
    #define MOD_EXTENSION ".dll" // e.g. Citadel.dll
    #define OS_DlOpen(path)       LoadLibraryA(path)
    #define OS_DlSym(handle,name) GetProcAddress((handle),(name))
    #define DECLSPEC_IMPORT __declspec (dllimport)
    #define WINAPI __stdcall
    #define INVALID_FHANDLE ((void*) (i64)-1)
    typedef void* FHandle; typedef i64 (WINAPI *PROC)(void); typedef i64 (WINAPI *FARPROC)(void); typedef i64 (WINAPI *NEARPROC)(void);
    typedef struct { unsigned long Data1; unsigned short Data2,Data3; u8 Data4[8]; } GUID; typedef struct { int unused; } *HINSTANCE; typedef HINSTANCE HMODULE;  typedef struct { u32 nLength; void* lpSecurityDescriptor; i32 bInheritHandle; } *LPSECURITY_ATTRIBUTES;
    typedef struct { i64 QuadPart; } LARGE_INTEGER; typedef LARGE_INTEGER *PLARGE_INTEGER; typedef struct { u64 Internal,InternalHigh; union {struct {u32 Offset,OffsetHigh;} DUMMYSTRUCTNAME; void* Pointer;} DUMMYUNIONNAME; void* hEvent; } OVERLAPPED, *LPOVERLAPPED;
    typedef struct { union { u32 dwOemId; struct { u16 wProcessorArchitecture,wReserved; } DUMMYSTRUCTNAME; } DUMMYUNIONNAME; u32 dwPageSize; void* lpMinimumApplicationAddress,*lpMaximumApplicationAddress; u64 dwActiveProcessorMask; u32 dwNumberOfProcessors,dwProcessorType,dwAllocationGranularity; u16 wProcessorLevel,wProcessorRevision; } SYSTEM_INFO, *LPSYSTEM_INFO;
    DECLSPEC_IMPORT void* WINAPI CreateFileMappingA(void*,LPSECURITY_ATTRIBUTES,u32,u32,u32,const char*); DECLSPEC_IMPORT i32 WINAPI VirtualFree(void*,u64,u32);                  DECLSPEC_IMPORT void* WINAPI VirtualAlloc(void*,u64,u32,u32 flProtect);
    DECLSPEC_IMPORT void* WINAPI CreateFileA(const char*,u32,u32,LPSECURITY_ATTRIBUTES,u32,u32,void*);    DECLSPEC_IMPORT i32 WINAPI ReadFile(void*,void*,u32,u32*,LPOVERLAPPED); DECLSPEC_IMPORT i32 WINAPI WriteFile(void*,void*,u32,u32*,LPOVERLAPPED);
    DECLSPEC_IMPORT i32 WINAPI SetFilePointerEx(void*,LARGE_INTEGER,PLARGE_INTEGER,u32);                  DECLSPEC_IMPORT i32 WINAPI GetFileSizeEx(void*,PLARGE_INTEGER);         DECLSPEC_IMPORT i32 WINAPI CloseHandle(void*);
    DECLSPEC_IMPORT void* WINAPI MapViewOfFileEx(void*,u32,u32,u32,u64,void*);                            DECLSPEC_IMPORT void* WINAPI MapViewOfFile(void*,u32,u32,u32,u64);      DECLSPEC_IMPORT i32 WINAPI UnmapViewOfFile(void*);
    DECLSPEC_IMPORT void* WINAPI CreateFileMappingW(void*,LPSECURITY_ATTRIBUTES,u32,u32,u32,u16*);        DECLSPEC_IMPORT void* WINAPI GetStdHandle(u32);                         DECLSPEC_IMPORT i32 WINAPI QueryPerformanceCounter(LARGE_INTEGER*);
    DECLSPEC_IMPORT i32 WINAPI QueryPerformanceFrequency(LARGE_INTEGER*);                                 DECLSPEC_IMPORT void WINAPI GetSystemInfo(LPSYSTEM_INFO);               DECLSPEC_IMPORT HINSTANCE WINAPI LoadLibraryA(const char*);
    DECLSPEC_IMPORT FARPROC WINAPI GetProcAddress(HINSTANCE,const char*);                                 DECLSPEC_IMPORT __declspec (noreturn) void WINAPI ExitProcess(u32);
    static char win_err_buf[512];
    struct timespec { i64 tv_sec; i32 tv_nsec; }; struct sched_param { int sched_priority; }; typedef uintptr_t pthread_t; typedef struct pthread_attr_t { unsigned p_state; void *stack; size_t s_size; struct sched_param param; } pthread_attr_t;
    int pthread_create(pthread_t*,const pthread_attr_t*,void*(*func)(void*),void*); int pthread_join(pthread_t,void**);
    int __cdecl _mkdir(const char* dirname);
    static inline __attribute__((always_inline, noreturn)) void OS_Exit(i64 exitCode) { ExitProcess((unsigned int)exitCode); __builtin_unreachable(); }
    static inline __attribute__((always_inline)) long OS_Open(const char* path, i32 flags, i32 m) { (void)m; void* h = CreateFileA(path,flags ? 0x40000000L : 0x80000000L, flags ? 0 : 1,0,flags ? 2 : 3,128,0); return h==(void*)-1 ? -1 : (long)(uintptr_t)h; }
    static inline __attribute__((always_inline)) void OS_Close(FHandle fileDescriptor) { CloseHandle(fileDescriptor); }
    static inline __attribute__((always_inline)) void* OS_AllocateRAM(void* a,size_t l,i32 p,i32 f,FHandle fd) { (void)f; if (fd==(void*)-1) return VirtualAlloc(a,l,0x3000,(p&2)?4:2); void* m = CreateFileMappingW(fd,NULL,(p&2) ? 4 : 2,(u32)(l>>32),(u32)l,NULL); void* r=MapViewOfFileEx(m,(p&2)?2:4,0,0,l,a); return CloseHandle(m),r;}    
    #define OS_MakeFolder(path) _mkdir(path)
    static inline __attribute__((always_inline)) long OS_Read(FHandle fd, void* buf, size_t count) { u32 bytesRead = 0; return (ReadFile((void*)fd,buf,(u32)count,&bytesRead,NULL)) ? (long)bytesRead : (long)-1; }
    static inline __attribute__((always_inline)) FHandle OS_OpenReadonly(const char* path) { void* f = CreateFileA(path,0x80000000L,1,NULL,3,0x08000080,NULL); return f == (void*)-1 ? DualLogError("Could not open file %s for reading\n",path), (void*)-1 : f; }
    static inline __attribute__((always_inline)) FHandle OS_OpenWriteonly(const char* path) { FHandle h = CreateFileA(path,0x40000000L,0,NULL,2,128,NULL); return h == (void*)-1 ? DualLogError("Failed to open %s for writing\n",path),(void*)-1 : h; }
    static inline __attribute__((always_inline)) int OS_FileSize(FHandle f) { LARGE_INTEGER s; return (f==(FHandle)-1 || !GetFileSizeEx(f,&s)) ? -1 : (int)s.QuadPart; }
    static inline __attribute__((always_inline)) void* OS_AllocateFileBackedRAMReadonly(size_t s,FHandle fd, char* path) { void* m; void* r; return(fd==(void*)-1||!s||!(m=CreateFileMappingA(fd,NULL,2,0,0,NULL))) ? DualLogError("CreateFileMapping failed for %s\n",path),NULL : (r=MapViewOfFile(m,4,0,0,s)) ? (CloseHandle(m),r) : (DualLogError("Failed to allocate %s\n",path),CloseHandle(m),NULL);}
    static inline __attribute__((always_inline)) i64 OS_Seek(FHandle fd, i64 ofs, int whence /*forth and forsooth pray tell*/) { LARGE_INTEGER l={.QuadPart=ofs},n; return SetFilePointerEx((void*)fd,l,&n,whence) ? n.QuadPart : -1; }
    static inline __attribute__((always_inline)) i64 OS_Tell(FHandle fd) { LARGE_INTEGER l={0},n; return SetFilePointerEx((void*)fd,l,&n,1) ? n.QuadPart : -1; }
    static inline __attribute__((always_inline)) int OS_GetNumThreads(void) { SYSTEM_INFO si; GetSystemInfo(&si); return (int)si.dwNumberOfProcessors; }
    static inline __attribute__((always_inline)) void OS_DeallocateRAM(void* p, size_t s) { (void)s; if(!p) { DualLogError("Attempting to double free!\n"); OS_Exit(1); } if(!UnmapViewOfFile(p) && !VirtualFree(p,0,0x00008000)) DualLogError("VirtualFree failed\n"); }
    static inline __attribute__((always_inline)) i64 OS_RawWrite(FHandle fd, const void* buf, size_t count) { u32 w; return WriteFile((void*)fd,(void*)buf,(u32)count,&w,NULL) ? (i64)w : -1; }
#else
    #define LINUX
    #define MOD_EXTENSION ".so" // e.g. Citadel.so
    #define OS_DlOpen(path)       dlopen((path),2)
    #define OS_DlSym(handle,name) dlsym((handle),(name))
    #define INVALID_FHANDLE -1
    struct input_id { u16 bustype,vendor,product,version;}; struct input_absinfo {i32 value,minimum,maximum,fuzz,flat,resolution;}; struct input_event { struct { long tv_sec,tv_usec; } time; u16 type,code; i32 value; };
    typedef int FHandle;
    struct timespec { i64 tv_sec,tv_nsec; }; typedef u64 pthread_t; typedef struct { unsigned int flags; void* stack; } pthread_attr_t;
    int pthread_create(pthread_t* restrict,const pthread_attr_t* restrict,void*(*start_routine)(void*),void* restrict); int pthread_join(pthread_t,void**); void *dlopen(const char*,int); void *dlsym(void*,const char *);
    static inline int OS_IOControl(int fd, unsigned long req, void* arg) { long r = 16; __asm__ __volatile__("syscall":"+a"(r):"D"(fd),"S"(req),"d"(arg):"rcx","r11","memory"); return (int)r; }
    static inline int OS_IOControlSimple(int fd, unsigned long request) { return OS_IOControl(fd,request,0); }
    static inline __attribute__((always_inline)) int OS_MakeFolder(const char* path) { long r = 83; __asm__ __volatile__("syscall":"+a"(r):"D"(path),"S"(0755LL):"rcx","r11","memory"); return (int)r; }
    static inline __attribute__((always_inline)) void* OS_Brk(void* addr) { register uintptr_t rax __asm__("rax") = 12; register void* rdi __asm__("rdi") = addr; __asm__ __volatile__("syscall":"+r"(rax):"r"(rdi):"rcx","r11","memory"); return (void*)rax; }
    static inline __attribute__((always_inline)) long OS_Read(long f,void*b,size_t c) { long r = 0; __asm__ __volatile__("syscall":"+a"(r):"D"(f),"S"(b),"d"(c):"rcx","r11","memory"); return r; }
    static inline __attribute__((always_inline, noreturn)) void OS_Exit(i64 exitCode) { register i64 rax __asm__("rax") = 231; register i64 rdi __asm__("rdi") = exitCode; __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi) : "rcx", "r11", "memory"); __builtin_unreachable(); }
    static inline __attribute__((always_inline)) void OS_Close(FHandle fileDescriptor) { register long rax __asm__("rax") = 3; register long rdi __asm__("rdi") = fileDescriptor; __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi) : "rcx", "r11", "memory"); }
    static inline __attribute__((always_inline)) long OS_Open(const char* path, i32 flags, i32 mode) { long r = 2; __asm__ __volatile__("syscall":"+a"(r):"D"(path),"S"((long)flags),"d"((long)mode):"rcx","r11","memory"); return r; }
    static inline __attribute__((always_inline)) void* OS_AllocateRAM(void* addr, size_t len, i32 prot, i32 flags, FHandle fd){ long r=9; register int r10 __asm__("r10")=flags; register int r8 __asm__("r8")=fd; register long r9 __asm__("r9")=0; __asm__ __volatile__("syscall":"+a"(r):"D"(addr),"S"(len),"d"(prot),"r"(r10),"r"(r8),"r"(r9):"rcx","r11","memory"); return (void*)r; }
    static inline __attribute__((always_inline)) FHandle OS_OpenReadonly(const char* path) { FHandle f=OS_Open(path,0,0); return f < 0 ? DualLogError("Could not open file %s for reading\n",path), -1 : f; }
    static inline __attribute__((always_inline)) FHandle OS_OpenWriteonly(const char* path) { FHandle f=OS_Open(path,1|00000100|00001000,0644); return f < 0 ? DualLogError("Failed to open %s for writing\n",path),-1 : f; }
    static inline __attribute__((always_inline)) int OS_FileSize(FHandle f) { long r=5,s[18]; __asm__ __volatile__("syscall":"+a"(r):"D"(f),"S"(s):"rcx","r11","memory"); return (int)s[6]; }
    static inline __attribute__((always_inline)) void* OS_AllocateFileBackedRAMReadonly(size_t s, FHandle fd, char* path) { void* r=OS_AllocateRAM(NULL,s,1,2,fd); return r==(void*)-1 ? DualLogError("Failed to allocate %s\n",path),NULL : r; }
    static inline __attribute__((always_inline)) i64 OS_Seek(FHandle fd, i64 ofs, int whence /* forth and forsooth pray tell*/) { i64 r = 8; __asm__ __volatile__("syscall":"+a"(r):"D"(fd),"S"(ofs),"d"(whence):"rcx","r11","memory"); return r; }
    static inline __attribute__((always_inline)) i64 OS_Tell(FHandle fd) { i64 r=8; __asm__ __volatile__("syscall":"+a"(r):"D"(fd),"S"(0LL),"d"(1):"rcx","r11","memory"); return r; }
    static inline __attribute__((always_inline)) int OS_GetNumThreads(void) { unsigned long m[16]; long r=204; __asm__ __volatile__("syscall":"+a"(r):"D"(0LL),"S"(128LL),"d"(m):"rcx","r11","memory"); int c = 0; for(int i=0;i<(r/8);i++) {c+=__builtin_popcountll(m[i]);} return r < 0 ? 1 : c; }
    static inline __attribute__((always_inline)) void OS_DeallocateRAM(void* p,size_t s){ long r=11; if(!p || p == (void*)-1) { DualLogError("Attempting to double free!\n"); OS_Exit(1); } __asm__ __volatile__("syscall":"+a"(r):"D"(p),"S"(s):"rcx","r11","memory"); if(r<0) DualLogError("munmap failed\n"); }
    static inline __attribute__((always_inline)) i64 OS_RawWrite(FHandle fd, const void* buf, size_t cnt) { i64 r=1; __asm__ __volatile__("syscall":"+a"(r):"D"(fd),"S"(buf),"d"(cnt):"rcx","r11","memory"); return r; }

    #define THREAD_FLAGS (CLONE_VM|CLONE_FS|CLONE_FILES|CLONE_SIGHAND|CLONE_THREAD|CLONE_SYSVSEM|CLONE_PARENT_SETTID|CLONE_CHILD_CLEARTID)
    //int pthread_create(pthread_t* thread, const pthread_attr_t* attr, void* (*start_routine)(void*), void* arg) {
        //const size_t STACK_SIZE = 1024*1024; void* stack = (attr && attr->stack) ? attr->stack : OS_Alloc(STACK_SIZE); if (!stack) return -1;

        //char* sp = (char*)stack + STACK_SIZE - 32; ((void**)sp)[0] = start_routine; ((void**)sp)[1] = arg;
        //i64 ret = 56;  // SYS_clone
        //__asm__ __volatile__("syscall":"+a"(ret):"D"(THREAD_FLAGS),"S"(sp),"d"(*thread),"r"((long)0),"r"((long)thread):"rcx","r11","memory");
        //if (ret == 0) {
            //void* (*fn)(void*) = ((void**)sp)[0]; void* a = ((void**)sp)[1]; void* result = fn(a);
            //i64 exit_code = (i64)result;
            //__asm__ __volatile__("syscall":"+a"(exit_code):"D"(exit_code) :"rcx","r11","memory"); // SYS_exit = 60, but we reuse rax
        //}

        //if (ret > 0) { *thread = ret; return 0; }
        //if (!(attr && attr->stack)) OS_DeallocateRAM(stack,STACK_SIZE);
        //return (int)ret;
    //}

    //int pthread_join(pthread_t thread, void** retval) {
        //i64 ret = 61; int status = 0;
        //__asm__ __volatile__("syscall":"+a"(ret):"D"(thread),"S"(&status),"d"(__WALL | __WEXITED):"rcx","r11","memory");
        //if (ret >= 0) { if (retval) {*retval = NULL;} return 0; }
        //return (int)ret;
    //}
#endif
static inline __attribute__((always_inline)) void* OS_Alloc(size_t amount) { return OS_AllocateRAM(NULL,amount,0x1|0x2,0x02|0x20,INVALID_FHANDLE); }
static inline __attribute__((always_inline)) void* OS_Calloc(size_t amount, size_t count) { return OS_AllocateRAM(NULL,amount * count,0x1|0x2,0x02|0x20,INVALID_FHANDLE); }
static inline __attribute__((always_inline)) void OS_Write(FHandle f,const void* buf, size_t s, const char* p) { size_t total=0; while(total < s) { i64 w=OS_RawWrite(f,(const char*)buf + total,s - total); if(w < 0) { DualLogError("Write error to %s: %s[%d]\n",p,w,(i32)-w); OS_Exit(1); } total += (size_t)w; } }
static inline __attribute__((always_inline)) void* OS_OpenAndAllocateFileBufferReadonly(const char* p,FHandle* f,int* s){void* r;return((*f=OS_OpenReadonly(p))==(FHandle)-1)?*s=0,(void*)0:((*s=OS_FileSize(*f))<=0)?DualLogError("Skipping empty:%s\n",p),OS_Close(*f),OS_Exit(1),NULL:(r=OS_AllocateFileBackedRAMReadonly(*s,*f,(char*)p))?(OS_Close(*f),r):NULL;}
static inline __attribute__((always_inline)) void* OSCopyMemoryFromBtoAForNBytes(void *dst, const void *src, size_t n) { unsigned char *d=(unsigned char *)dst; const unsigned char *s=(const unsigned char *)src; while (n--) {*d++=*s++;} return dst; } // memcpy replacement
static inline __attribute__((always_inline)) void* OS_Realloc(void* old, size_t olds, size_t news) { void* n; return !old ? OS_Alloc(news) : news <= olds ? old : (n=OS_Alloc(news)) ? (OSCopyMemoryFromBtoAForNBytes(n,old,olds),OS_DeallocateRAM(old,olds),n) : 0; }
