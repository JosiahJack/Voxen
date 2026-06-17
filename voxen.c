// voxen.c - A realtime OpenGL 4.3+ Game Engine for Citadel: The System Shock Fan Remake
#if defined(LINUX)
    //#define DEBUG_RAM_OUTPUT // Uncomment for when attacking RAM.
#endif
#include "common.h"
#define MOD_INTEROP_ENGINE
#include "interop.h"
// ==================== OS Shim Layer System
typedef __UINTPTR_TYPE__ uintptr_t; typedef __INTPTR_TYPE__ intptr_t;
#define likely(x)   __builtin_expect(!!(x),1)
#define unlikely(x) __builtin_expect(!!(x),0)
#define NULL ((void *)0)
#if defined(_WIN32) // Interop - To Mod (keep the same as interop.h!!)
    #define ENGINE_TO_MOD __declspec(dllexport) __cdecl
#else
    #define ENGINE_TO_MOD __attribute__((visibility("default")))
#endif
#define assert(cond) do { if (!(cond)) { DualLogError("[%s:%d]:%s(): Assert fail:%s\n",__FILE__,__LINE__,__func__,#cond); *(volatile int*)0 = 0; } } while(0) // Force a crash/segfault for the debugger
ENGINE_TO_MOD void DualLogError(const char* fmt, ...); ENGINE_TO_MOD void DualLog(const char* fmt, ...); char* StringFindSubstring(const char* haystack, const char* needle);
#if defined(_WIN32)
    #define WINDOWS
    #define MOD_EXTENSION ".dll" // e.g. Citadel.dll
    #define OS_DlOpen(path)       LoadLibraryA(path)
    #define OS_DlSym(handle,name) GetProcAddress((handle),(name))
    #define DECLSPEC_IMPORT __declspec (dllimport)
    #define WINAPI __stdcall
    #define INVALID_FHANDLE ((void*) (i64)-1)
    typedef void* FHandle; typedef i64 (WINAPI *PROC)(); typedef i64 (WINAPI *FARPROC)(); typedef i64 (WINAPI *NEARPROC)();
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
    struct timespec { i64 tv_sec; i32 tv_nsec; }; struct sched_param { int sched_priority; }; typedef uintptr_t pthread_t; typedef intptr_t pthread_mutex_t,pthread_cond_t; typedef int pthread_condattr_t; typedef u32 pthread_mutexattr_t; typedef struct pthread_attr_t { unsigned p_state; void *stack; size_t s_size; struct sched_param param; } pthread_attr_t;
    int pthread_create(pthread_t*,const pthread_attr_t*,void*(*func)(void*),void*); int pthread_join(pthread_t,void**);
    int __cdecl _mkdir(const char* dirname);
    static inline __attribute__((always_inline, noreturn)) void OS_Exit(i64 exitCode) { ExitProcess((unsigned int)exitCode); __builtin_unreachable(); }
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
    static inline __attribute__((always_inline)) int OS_GetNumThreads() { SYSTEM_INFO si; GetSystemInfo(&si); return (int)si.dwNumberOfProcessors; }
    static inline __attribute__((always_inline)) void OS_DeallocateRAM(void* p, size_t s) { (void)s; if(!p) { DualLogError("Attempting to double free!\n"); OS_Exit(1); } if(!UnmapViewOfFile(p) && !VirtualFree(p,0,0x00008000)) DualLogError("VirtualFree failed\n"); }
    static inline __attribute__((always_inline)) i64 OS_RawWrite(FHandle fd, const void* buf, size_t count) { u32 w; return WriteFile((void*)fd,(void*)buf,(u32)count,&w,NULL) ? (i64)w : -1; }
    #define THREAD_STACK_SIZE (8 * 1024 * 1024)
    typedef struct { void* handle; } OS_Thread;
    void* __stdcall GetProcessHeap(); void* __stdcall HeapAlloc(void* hHeap, u32 dwFlags, size_t dwBytes); i32 __stdcall HeapFree(void* hHeap, u32 dwFlags, void* lpMem); void __stdcall Sleep(u32 dwMilliseconds); u32 __stdcall WaitForSingleObject(void* hHandle, u32 dwMilliseconds);
    typedef u32 (__stdcall *LPTHREAD_START_ROUTINE)(void* lpParameter);
    void* __stdcall CreateThread(void* lpThreadAttributes, size_t dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, void* lpParameter, u32 dwCreationFlags, u32* lpThreadId);
    static u32 WINAPI OS_ThreadTrampoline(void* arg) { void** bundle = (void**)arg; void*(*fn)(void*) = (void*(*)(void*))bundle[0]; void* real_arg = bundle[1]; HeapFree(GetProcessHeap(), 0, bundle); fn(real_arg); return 0; }
    static inline __attribute__((always_inline)) int OS_ThreadCreate(OS_Thread* out, void*(*fn)(void*), void* arg) {
        void** bundle = (void**)HeapAlloc(GetProcessHeap(), 0, 2 * sizeof(void*));
        if (!bundle) return -1;
        bundle[0] = (void*)fn; bundle[1] = arg;
        out->handle = CreateThread(NULL, THREAD_STACK_SIZE, OS_ThreadTrampoline,bundle,0,NULL);
        if (!out->handle) { HeapFree(GetProcessHeap(), 0, bundle); return -1; }
        return 0;
    }

    static inline __attribute__((always_inline)) void OS_ThreadJoin(OS_Thread* t) { WaitForSingleObject(t->handle,0xFFFFFFFFUL); CloseHandle(t->handle); t->handle = NULL; }
    static inline __attribute__((always_inline)) void OS_USleep(u32 usec) { Sleep((usec + 999) / 1000); }
#else
    #define LINUX
    #define MOD_EXTENSION ".so" // e.g. Citadel.so
    #define OS_DlOpen(path)       dlopen((path),2)
    #define OS_DlSym(handle,name) dlsym((handle),(name))
    #define INVALID_FHANDLE -1
    struct input_id { u16 bustype,vendor,product,version;}; struct input_absinfo {i32 value,minimum,maximum,fuzz,flat,resolution;}; struct input_event { struct { long tv_sec,tv_usec; } time; u16 type,code; i32 value; };
    typedef int FHandle;
    struct timespec { i64 tv_sec,tv_nsec; }; typedef u64 pthread_t; typedef u32 pthread_mutexattr_t; typedef struct { u8 _[40]; } pthread_mutex_t; typedef struct { u8 _[48]; } pthread_cond_t; typedef int pthread_condattr_t; typedef struct { unsigned int flags; void* stack; } pthread_attr_t;
    int pthread_create(pthread_t* restrict,const pthread_attr_t* restrict,void*(*start_routine)(void*),void* restrict); int pthread_join(pthread_t,void**); void *dlopen(const char*,int); void *dlsym(void*,const char *);
    static inline __attribute__((always_inline)) int OS_IOControl(int fd, unsigned long req, void* arg) { long r = 16; __asm__ __volatile__("syscall":"+a"(r):"D"(fd),"S"(req),"d"(arg):"rcx","r11","memory"); return (int)r; }
    static inline __attribute__((always_inline)) int OS_MakeFolder(const char* path) { long r = 83; __asm__ __volatile__("syscall":"+a"(r):"D"(path),"S"(0755LL):"rcx","r11","memory"); return (int)r; }
    #ifdef DEBUG_RAM_OUTPUT
        static inline __attribute__((always_inline)) void* OS_Brk(void* addr) { long r = 12; __asm__ __volatile__("syscall":"+a"(r):"D"(addr):"rcx","r11","memory"); return (void*)r; }
    #endif
    static inline __attribute__((always_inline)) long OS_Read(long f,void*b,size_t c) { long r = 0; __asm__ __volatile__("syscall":"+a"(r):"D"(f),"S"(b),"d"(c):"rcx","r11","memory"); return r; }
    static inline __attribute__((always_inline, noreturn)) void OS_Exit(i64 exitCode) { long r = 231; __asm__ __volatile__("syscall":"+a"(r):"D"(exitCode):"rcx","r11","memory"); __builtin_unreachable(); }
    static inline __attribute__((always_inline)) void OS_Close(FHandle fileDescriptor) { long r = 3; __asm__ __volatile__("syscall":"+a"(r):"D"(fileDescriptor):"rcx","r11","memory"); }
    static inline __attribute__((always_inline)) long OS_Open(const char* path, i32 flags, i32 mode) { long r = 2; __asm__ __volatile__("syscall":"+a"(r):"D"(path),"S"((long)flags),"d"((long)mode):"rcx","r11","memory"); return r; }
    static inline __attribute__((always_inline)) void* OS_AllocateRAM(void* addr, size_t len, i32 prot, i32 flags, FHandle fd){ long r=9; register int r10 __asm__("r10")=flags; register int r8 __asm__("r8")=fd; register long r9 __asm__("r9")=0; __asm__ __volatile__("syscall":"+a"(r):"D"(addr),"S"(len),"d"(prot),"r"(r10),"r"(r8),"r"(r9):"rcx","r11","memory"); return (void*)r; }
    static inline __attribute__((always_inline)) FHandle OS_OpenReadonly(const char* path) { FHandle f=OS_Open(path,0,0); return f < 0 ? DualLogError("Could not open file %s for reading\n",path), -1 : f; }
    static inline __attribute__((always_inline)) FHandle OS_OpenWriteonly(const char* path) { FHandle f=OS_Open(path,1|00000100|00001000,0644); return f < 0 ? DualLogError("Failed to open %s for writing\n",path),-1 : f; }
    static inline __attribute__((always_inline)) int OS_FileSize(FHandle f) { long r=5,s[18]; __asm__ __volatile__("syscall":"+a"(r):"D"(f),"S"(s):"rcx","r11","memory"); return (int)s[6]; }
    static inline __attribute__((always_inline)) void* OS_AllocateFileBackedRAMReadonly(size_t s, FHandle fd, char* path) { void* r=OS_AllocateRAM(NULL,s,1,2,fd); return r==(void*)-1 ? DualLogError("Failed to allocate %s\n",path),NULL : r; }
    static inline __attribute__((always_inline)) i64 OS_Seek(FHandle fd, i64 ofs, int whence /* forth and forsooth pray tell*/) { i64 r = 8; __asm__ __volatile__("syscall":"+a"(r):"D"(fd),"S"(ofs),"d"(whence):"rcx","r11","memory"); return r; }
    static inline __attribute__((always_inline)) i64 OS_Tell(FHandle fd) { i64 r=8; __asm__ __volatile__("syscall":"+a"(r):"D"(fd),"S"(0LL),"d"(1):"rcx","r11","memory"); return r; }
    static inline __attribute__((always_inline)) int OS_GetNumThreads() { unsigned long m[16]; long r=204; __asm__ __volatile__("syscall":"+a"(r):"D"(0LL),"S"(128LL),"d"(m):"rcx","r11","memory"); int c = 0; for(int i=0;i<(r/8);i++) {c+=__builtin_popcountll(m[i]);} return r < 0 ? 1 : c; }
    static inline __attribute__((always_inline)) void OS_DeallocateRAM(void* p,size_t s){ long r=11; if(!p || p == (void*)-1) { DualLogError("Attempting to double free!\n"); OS_Exit(1); } __asm__ __volatile__("syscall":"+a"(r):"D"(p),"S"(s):"rcx","r11","memory"); if(r<0) DualLogError("munmap failed\n"); }
    static inline __attribute__((always_inline)) i64 OS_RawWrite(FHandle fd, const void* buf, size_t cnt) { i64 r=1; __asm__ __volatile__("syscall":"+a"(r):"D"(fd),"S"(buf),"d"(cnt):"rcx","r11","memory"); return r; }
    #define SYSCALL1(n, a) syscall6(n,(long)(a),0,0,0,0,0)
    #define SYSCALL2(n, a, b) syscall6(n,(long)(a),(long)(b),0,0,0,0)
    #define SYSCALL3(n, a, b, c) syscall6(n,(long)(a),(long)(b),(long)(c),0,0,0)
    #define SYSCALL4(n, a, b, c, d) syscall6(n,(long)(a),(long)(b),(long)(c),(long)(d),0,0)
    #define THREAD_STACK_SIZE (8 * 1024 * 1024)
    static inline __attribute__((always_inline)) long syscall6(long n, long a, long b, long c, long d, long e, long f) { register long r=n; register long r10 __asm__("r10") = d; register long r8  __asm__("r8")  = e; register long r9  __asm__("r9")  = f; __asm__ __volatile__("syscall":"+a"(r):"D"(a),"S"(b),"d"(c),"r"(r10),"r"(r8),"r"(r9):"rcx","r11","memory"); return r; }
    typedef struct __attribute__((aligned(16))) OS_ThreadHead { void(*trampoline)(struct OS_ThreadHead*),*(*fn)(void*),*arg; int join_futex,_pad; } OS_ThreadHead;
    typedef struct { struct OS_ThreadHead* head; void* stack_base; } OS_Thread;
    __attribute__((naked)) static long OS_CloneSyscall(struct OS_ThreadHead* stack) { __asm__ volatile ("mov %%rdi, %%rsi\nmov $0x50f00, %%edi\nmov $56, %%eax\nsyscall\nmov  %%rsp, %%rdi\nret\n":::"rax","rcx","rsi","rdi","r11","memory"); }
    __attribute__((noreturn)) static void OS_ThreadTrampoline(struct OS_ThreadHead* head) { head->fn(head->arg); __atomic_store_n(&head->join_futex,1,__ATOMIC_SEQ_CST); SYSCALL3(202,&head->join_futex,1,0x7fffffff);/*futex wake*/  SYSCALL1(60,0); __builtin_unreachable(); }
    static inline __attribute__((always_inline)) int OS_ThreadCreate(OS_Thread* out, void* (*fn)(void*), void* arg) { // Multithreading taken from https://github.com/skeeto/scratch/blob/master/misc/stack_head.c Ref: https://nullprogram.com/blog/2023/03/23/ This is free and unencumbered software released into the public domain.
        void* base = OS_AllocateRAM(NULL, THREAD_STACK_SIZE, 0x1|0x2, 0x02|0x20, INVALID_FHANDLE);
        if (!base || base == (void*)-1) return -1;

        struct OS_ThreadHead* head = (struct OS_ThreadHead*)((char*)base + THREAD_STACK_SIZE) - 1;
        head->trampoline = OS_ThreadTrampoline;  // child's ret target
        head->fn=fn; head->arg=arg; head->join_futex=0; head->_pad=0;
        long tid = OS_CloneSyscall(head); if (tid < 0) { OS_DeallocateRAM(base,THREAD_STACK_SIZE); return (int)tid; }

        out->head=head; out->stack_base=base;
        return 0;
    }

    static inline __attribute__((always_inline)) void OS_ThreadJoin(OS_Thread* t) {
        int v;
        while ((v = __atomic_load_n(&t->head->join_futex, __ATOMIC_SEQ_CST)) == 0) SYSCALL4(202, &t->head->join_futex, 0 /*FUTEX_WAIT*/, v, 0);
        OS_DeallocateRAM(t->stack_base,THREAD_STACK_SIZE);
        t->head = NULL; t->stack_base = NULL;
    }
    
    static inline __attribute__((always_inline))  void OS_USleep(u32 usec) { long ts[2] = {usec / 1000000,(usec % 1000000) * 1000L}; SYSCALL2(35,ts,ts); }
#endif
static inline __attribute__((always_inline)) void* OS_Alloc(size_t amount) { return OS_AllocateRAM(NULL,amount,0x1|0x2,0x02|0x20,INVALID_FHANDLE); }
static inline __attribute__((always_inline)) void* OS_Calloc(size_t amount, size_t count) { return OS_AllocateRAM(NULL,amount * count,0x1|0x2,0x02|0x20,INVALID_FHANDLE); }
static inline __attribute__((always_inline)) void OS_Write(FHandle f,const void* buf, size_t s, const char* p) { size_t total=0; while(total < s) { i64 w=OS_RawWrite(f,(const char*)buf + total,s - total); if(w < 0) { DualLogError("Write error to %s: %s[%d]\n",p,w,(i32)-w); OS_Exit(1); } total += (size_t)w; } }
static inline __attribute__((always_inline)) void* OS_OpenAndAllocateFileBufferReadonly(const char* p,FHandle* f,int* s){void* r;return((*f=OS_OpenReadonly(p))==(FHandle)-1)?*s=0,(void*)0:((*s=OS_FileSize(*f))<=0)?DualLogError("Skipping empty:%s\n",p),OS_Close(*f),OS_Exit(1),NULL:(r=OS_AllocateFileBackedRAMReadonly(*s,*f,(char*)p))?(OS_Close(*f),r):NULL;}
void* MemCpyFromBtoAForNBytes(void *dst, const void *src, size_t n) { unsigned char *d=(unsigned char *)dst; const unsigned char *s=(const unsigned char *)src; while (n--) {*d++=*s++;} return dst; } // memcpy replacement
static inline __attribute__((always_inline)) void* OS_Realloc(void* old, size_t olds, size_t news) { void* n; return !old ? OS_Alloc(news) : news <= olds ? old : (n=OS_Alloc(news)) ? (MemCpyFromBtoAForNBytes(n,old,olds),OS_DeallocateRAM(old,olds),n) : 0; }
// ============== Declarations
enum { GL_ARRAY_BUFFER=0x8892,      GL_DEPTH_BUFFER_BIT=0x00000100, GL_READ_WRITE=0x88BA, GL_SSBO=0x90D2,                 GL_CULL_FACE=0x0B44,                          
       GL_BLEND=0x0BE2,             GL_DEPTH_TEST=0x0B71,           GL_RGB=0x1907,        GL_TEXTURE0=0x84C0,             GL_TEXTURE5=0x84C5,
       GL_COLOR_ATTACHMENT0=0x8CE0,                                 GL_RG16F=0x822F,      GL_TEXTURE1=0x84C1,             GL_TEXTURE6=0x84C6,
       GL_COLOR_ATTACHMENT1=0x8CE1, GL_ELEMENT_ARRAY_BUFFER=0x8893, GL_RGB16F=0x881B,     GL_TEXTURE2=0x84C2,             GL_TEXTURE_2D=0x0DE1,
       GL_COLOR_ATTACHMENT2=0x8CE2, GL_FALSE=0,                     GL_RGBA=0x1908,       GL_TEXTURE3=0x84C3,             GL_UNSIGNED_BYTE=0x1401,
       GL_COLOR_ATTACHMENT3=0x8CE3, GL_FLOAT=0x1406,                GL_RGBA32F=0x8814,    GL_TEXTURE4=0x84C4,             GL_FRAMEBUFFER=0x8D40,
       GL_COLOR_ATTACHMENT4=0x8CE4, GL_UNSIGNED_SHORT=0x1403,       GL_RGBA8=0x8058,      GL_COLOR_BUFFER_BIT=0x00004000, GL_STATIC_DRAW=0x88E4,   GL_DYNAMIC_DRAW=0x88E8,};
typedef void(*PFNGLACTIVETEXTURE)(u32),(*PFNGLATTACHSHADER)(u32,u32),(*PFNGLBINDBUFFER)(u32,u32),(*PFNGLBINDBUFFERBASE)(u32,u32,u32),(*PFNGLBINDFRAMEBUFFER)(u32,u32);
typedef void(*PFNGLBINDIMAGETEXTURE)(u32,u32,i32,bool,i32,u32,u32),(*PFNGLBINDTEXTURE)(u32,u32),(*PFNGLBINDTEXTUREUNIT)(u32,u32),(*PFNGLBINDVERTEXARRAY)(u32),(*PFNGLBUFFERSUBDATA)(u32,intptr_t,intptr_t,const void*);
typedef void(*PFNGLBINDVERTEXBUFFER)(u32,u32,intptr_t,i32),(*PFNGLBLENDFUNCSEPARATE)(u32,u32,u32,u32),(*PFNGLBUFFERDATA)(u32,size_t,const void*,u32),(*PFNGLCLEAR)(u32);
typedef void(*PFNGLCLEARCOLOR)(float,float,float,float),(*PFNGLCOLORMASK)(bool,bool,bool,bool),(*PFNGLCOMPILESHADER)(u32),(*PFNGLCOPYTEXSUBIMAGE2D)(u32,i32,i32,i32,i32,i32,i32,i32);
typedef void(*PFNGLCREATEBUFFERS)(i32,u32*),(*PFNGLGENVERTEXARRAYS)(i32,u32*),(*PFNGLCULLFACE)(u32),(*PFNGLDEPTHFUNC)(u32),(*PFNGLDEPTHMASK)(bool);
typedef void(*PFNGLDISABLE)(u32),(*PFNGLDISPATCHCOMPUTE)(u32,u32,u32),(*PFNGLDRAWARRAYS)(u32,i32,i32),(*PFNGLDRAWBUFFERS)(i32,const u32*),(*PFNGLDRAWELEMENTS)(u32,i32,u32,const void*);
typedef void(*PFNGLENABLE)(u32),(*PFNGLFINISH)(),(*PFNGLFLUSH)(),(*PFNGLFRAMEBUFFERTEXTURE2D)(u32,u32,u32,u32,i32),(*PFNGLFRONTFACE)(u32);
typedef void(*PFNGLGENBUFFERS)(i32,u32*),(*PFNGLGENFRAMEBUFFERS)(i32,u32*),(*PFNGLGENTEXTURES)(i32,u32*),(*PFNGLGETINTEGERV)(u32,i32*),(*PFNGLGETPROGRAMIV)(u32,u32,i32*),(*PFNGLREADPIXELS)(i32,i32,i32,i32,u32,u32,void*);
typedef void(*PFNGLGETSHADERINFOLOG)(u32,i32,i32*,char*),(*PFNGLGETSHADERIV)(u32,u32,i32*),(*PFNGLLINEWIDTH)(float),(*PFNGLLINKPROGRAM)(u32),(*PFNGLSHADERSOURCE)(u32,i32,const char*const*,const i32*),(*PFNGLREADBUFFER)(u32);
typedef void(*PFNGLTEXIMAGE2D)(u32,i32,i32,i32,i32,i32,u32,u32,const void*),(*PFNGLTEXPARAMETERI)(u32,u32,i32);
typedef void(*PFNGLUNIFORM1F)(i32,float),(*PFNGLUNIFORM1I)(i32,i32),(*PFNGLUNIFORM1UI)(i32,u32),(*PFNGLUNIFORM2F)(i32,float,float),(*PFNGLUNIFORM2UI)(i32,u32,u32),(*PFNGLENABLEVERTEXATTRIBARRAY)(u32);
typedef void(*PFNGLUNIFORM3F)(i32,float,float,float),(*PFNGLUNIFORM4F)(i32,float,float,float,float),(*PFNGLUNIFORMMATRIX3FV)(i32,i32,bool,const float*),(*PFNGLVERTEXATTRIBBINDING)(u32,u32);
typedef void(*PFNGLUNIFORMMATRIX4FV)(i32,i32,bool,const float*),(*PFNGLUSEPROGRAM)(u32),(*PFNGLVIEWPORT)(i32,i32,i32,i32),(*PFNGLVERTEXATTRIBFORMAT)(u32,i32,u32,bool,u32);
typedef u32(*PFNGLCHECKFRAMEBUFFERSTATUS)(u32),(*PFNGLCREATEPROGRAM)(),(*PFNGLCREATESHADER)(u32),(*PFNGLGETERROR)(),(*PFNGLCLEARBUFFERFV)(u32,i32,const float*);
typedef void*(*PFNGLMAPBUFFERRANGE)(u32,intptr_t,size_t,u32); typedef bool(*PFNGLUNMAPBUFFER)(u32);
        PFNGLACTIVETEXTURE glActiveTexture;       PFNGLATTACHSHADER glAttachShader;                 PFNGLBINDBUFFER glBindBuffer;       PFNGLBINDBUFFERBASE glBindBufferBase; PFNGLBINDFRAMEBUFFER glBindFramebuffer;             PFNGLBINDIMAGETEXTURE glBindImageTexture;
            PFNGLBINDTEXTURE glBindTexture; PFNGLBINDVERTEXARRAY glBindVertexArray;     PFNGLBINDVERTEXBUFFER glBindVertexBuffer; PFNGLBLENDFUNCSEPARATE glBlendFuncSeparate;           PFNGLBUFFERDATA glBufferData; PFNGLCHECKFRAMEBUFFERSTATUS glCheckFramebufferStatus;
                        PFNGLCLEAR glClear;             PFNGLCOLORMASK glColorMask;           PFNGLCOMPILESHADER glCompileShader;         PFNGLCREATEPROGRAM glCreateProgram;       PFNGLCREATESHADER glCreateShader;               PFNGLGENVERTEXARRAYS glGenVertexArrays;
                PFNGLDEPTHFUNC glDepthFunc;             PFNGLDEPTHMASK glDepthMask;                       PFNGLDISABLE glDisable;     PFNGLDISPATCHCOMPUTE glDispatchCompute;           PFNGLDRAWARRAYS glDrawArrays;                       PFNGLDRAWBUFFERS glDrawBuffers;
          PFNGLDRAWELEMENTS glDrawElements;                   PFNGLENABLE glEnable;                         PFNGLFINISH glFinish;                         PFNGLFLUSH glFlush;             PFNGLFRONTFACE glFrontFace;     PFNGLFRAMEBUFFERTEXTURE2D glFramebufferTexture2D;
                PFNGLFRONTFACE glFrontFace;           PFNGLGENBUFFERS glGenBuffers;       PFNGLGENFRAMEBUFFERS glGenFramebuffers;             PFNGLGENTEXTURES glGenTextures;               PFNGLGETERROR glGetError;                     PFNGLGETPROGRAMIV glGetProgramiv;
  PFNGLGETSHADERINFOLOG glGetShaderInfoLog;         PFNGLGETSHADERIV glGetShaderiv;                   PFNGLLINEWIDTH glLineWidth;             PFNGLLINKPROGRAM glLinkProgram;   PFNGLMAPBUFFERRANGE glMapBufferRange;                         PFNGLREADBUFFER glReadBuffer;
              PFNGLREADPIXELS glReadPixels;       PFNGLSHADERSOURCE glShaderSource;                 PFNGLTEXIMAGE2D glTexImage2D; PFNGLCOPYTEXSUBIMAGE2D glCopyTexSubImage2D;     PFNGLTEXPARAMETERI glTexParameteri;                           PFNGLUNIFORM1F glUniform1f;
                PFNGLUNIFORM1I glUniform1i;           PFNGLUNIFORM1UI glUniform1ui;                   PFNGLUNIFORM2F glUniform2f;                 PFNGLUNIFORM3F glUniform3f;             PFNGLUNIFORM4F glUniform4f;             PFNGLUNIFORMMATRIX3FV glUniformMatrix3fv;
  PFNGLUNIFORMMATRIX4FV glUniformMatrix4fv;         PFNGLUNMAPBUFFER glUnmapBuffer;                 PFNGLUSEPROGRAM glUseProgram;                   PFNGLVIEWPORT glViewport;         PFNGLGETINTEGERV glGetIntegerv;                         PFNGLUNIFORM2UI glUniform2ui;
              PFNGLCLEARCOLOR glClearColor;     PFNGLBUFFERSUBDATA glBufferSubData; PFNGLVERTEXATTRIBFORMAT glVertexAttribFormat;   PFNGLBINDVERTEXBUFFER glBindVertexBuffer;     PFNGLCLEARBUFFERFV glClearBufferFv;       PFNGLVERTEXATTRIBBINDING glVertexAttribBinding; PFNGLENABLEVERTEXATTRIBARRAY glEnableVertexAttribArray;
typedef enum {JOYHAT_CENTERED=0,JOYHAT_UP=1,JOYHAT_RIGHT=2,JOYHAT_DOWN=4,JOYHAT_LEFT=8,JOYHAT_RIGHT_UP=(2|1),JOYHAT_RIGHT_DOWN=(2|4),JOYHAT_LEFT_UP=(8|1),JOYHAT_LEFT_DOWN=(8|4)} JoyHatId;
typedef enum {KEY_UNKNOWN=-1,KEY_SPACE=32,KEY_APOSTROPHE=39/* ' */,KEY_COMMA=44/* , */,KEY_MINUS=45/* - */,KEY_PERIOD=46/* . */,KEY_SLASH=47/* / */,KEY_0=48,KEY_1=49,KEY_2=50,KEY_3=51,KEY_4=52,KEY_5=53,KEY_6=54,KEY_7=55,KEY_8=56,KEY_9=57,
             KEY_SEMICOLON=59/* ; */,KEY_EQUAL=61/* = */,KEY_A=65,KEY_B=66,KEY_C=67,KEY_D=68,KEY_E=69,KEY_F=70,KEY_G=71,KEY_H=72,KEY_I=73,KEY_J=74,KEY_K=75,KEY_L=76,KEY_M=77,KEY_N=78,KEY_O=79,KEY_P=80,KEY_Q=81,KEY_R=82,KEY_S=83,KEY_T=84,KEY_U=85,KEY_V=86,KEY_W=87,KEY_X=88,KEY_Y=89,KEY_Z=90,
             KEY_LEFT_BRACKET=91/* [ */,KEY_BACKSLASH=92/* \ */,KEY_RIGHT_BRACKET=93/* ] */,KEY_GRAVE_ACCENT=96/* ` */,KEY_ESCAPE=256,KEY_ENTER=257,KEY_TAB=258,KEY_BACKSPACE=259,KEY_INSERT=260,KEY_DELETE=261,KEY_RIGHT=262,KEY_LEFT=263,KEY_DOWN=264,KEY_UP=265,KEY_PAGE_UP=266,KEY_PAGE_DOWN=267,
             KEY_HOME=268,KEY_END=269,KEY_CAPS_LOCK=280,KEY_SCROLL_LOCK=281,KEY_NUM_LOCK=282,KEY_PRINT_SCREEN=283,KEY_PAUSE=284,KEY_F1=290,KEY_F2=291,KEY_F3=292,KEY_F4=293,KEY_F5=294,KEY_F6=295,KEY_F7=296,KEY_F8=297,KEY_F9=298,KEY_F10=299,KEY_F11=300,KEY_F12=301,KEY_KP_0=320,
             KEY_KP_1=321,KEY_KP_2=322,KEY_KP_3=323,KEY_KP_4=324,KEY_KP_5=325,KEY_KP_6=326,KEY_KP_7=327,KEY_KP_8=328,KEY_KP_9=329,KEY_KP_DECIMAL=330,KEY_KP_DIVIDE=331,KEY_KP_MULTIPLY=332,KEY_KP_SUBTRACT=333,KEY_KP_ADD=334,KEY_KP_ENTER=335,KEY_KP_EQUAL=336,KEY_LEFT_SHIFT=340,
             KEY_LEFT_CONTROL=341,KEY_LEFT_ALT=342,KEY_LEFT_SUPER=343,KEY_RIGHT_SHIFT=344,KEY_RIGHT_CONTROL=345,KEY_RIGHT_ALT=346,KEY_RIGHT_SUPER=347,KEY_MENU=348} KeyId;
typedef enum {MOUSE_BUTTON_1=0,MOUSE_BUTTON_2=1,MOUSE_BUTTON_3=2,MOUSE_BUTTON_4=3,MOUSE_BUTTON_5=4,MOUSE_BUTTON_6=5,MOUSE_BUTTON_7=6,MOUSE_BUTTON_8=7,MOUSE_BUTTON_LEFT=0,MOUSE_BUTTON_RIGHT=1,MOUSE_BUTTON_MIDDLE=2} MouseButtonId;
typedef enum {JOYSTICK_1=0,JOYSTICK_2=1,JOYSTICK_3=2,JOYSTICK_4=3,JOYSTICK_5=4,JOYSTICK_6=5,JOYSTICK_7=6,JOYSTICK_8=7,JOYSTICK_9=8,JOYSTICK_10=9,JOYSTICK_11=10,JOYSTICK_12=11,JOYSTICK_13=12,JOYSTICK_14=13,JOYSTICK_15=14,JOYSTICK_16=15,JOYSTICK_LAST=15} JoystickId;
typedef struct { bool down,pressed,released; } KeyState; typedef struct { const char* name; int value; } InputElement;
typedef struct { double last_mouse_x,last_mouse_y,scrollDelta; KeyState keyStates[MAX_KEYS],mouseButtons[MAX_MOUSE_BUTTONS],joystickButtons[16][16],joystickHats[5]; /* What can I say, I'm a man of many hats. ^^D*/ i32 currentMouse_dx,currentMouse_dy; bool window_has_focus,ignore_next_mouse_delta,lastUse,isCapsLockOn,joystickPresent[16]; } InputSystem;
typedef struct { Vector3 normal; float d; } FrustumPlane;
typedef struct PngArena { u8*base,*cursor,*end; } PngArena;
typedef struct {
    u32 inputImageID,inputUIID,inputDepthID,inputWorldPosID,inputSpecID,inputNormalID,gBufferFBO,uiFBO,outputImageID,depthPrepassShaderProgram,chunkShaderProgram,chunkVAO,chunkVBO,uiShaderProgram,debugUnlitShaderProgram;
    u32 shadowmapsShaderProgram,shadowmapsClearShaderProgram,shadowMapSSBO,shadowMapsIndirectionID,ssrShaderProgram,imageBlitShaderProgram,quadVAO,quadVBO,textShaderProgram,textVAO,textVBO;
    u32 debugLinesVAO,debugLinesVBO,matricesBufferID,cellVisibleDataID,debugLineColors,colorBufferID,texturePalettesID,texturePaletteOffsetsID,textureOffsetsID,textureSizesID;
    u32 lightsID,voxelLightListCountsID,voxelLightListsID,voxelUpdateShaderProgram,shadowViewProjID,vbos[MODEL_IDX_MAX],tbos[MODEL_IDX_MAX];
} RenderSystem;
#define PNG_ARENA_SIZE 16*1024*1024
static float berserkSeedTime,rasterPerspectiveProjection[16],shadowmapsPerspectiveProjection[16],lightView[LIGHT_COUNT][6][4][4],lightViewProj[LIGHT_COUNT][6][16];
float modelMatrices[INSTANCE_COUNT*16];
bool mouseMovementThisFrame,returnToPause=false,fovSliderActive=false,gammaSliderActive=false,masterVolumeSliderActive=false,musicVolumeSliderActive=false,messageVolumeSliderActive=false,sfxVolumeSliderActive=false,enteringPlayerName=false;
u8 currentPlayerNameLength=0; i8 currentMenuItem=0, currentMenuTab=0, menuItemCount=4, menuTabCount=1;
static int num_parse_threads = 0;
#define CHECK_GL_ERROR() do { u32 err = glGetError(); if (err != 0) DualLogError("GL Error at %s:%d: %d\n", __FILE__, __LINE__, err); } while(0)
#define SHADOW_MAP_SIZE 128u
#define MAX_SHADOWMAPS 128u
#define MAX_LIGHTS_PER_VOXEL 32
#define NEAR_PLANE (0.02f)
#define ONE_OVER_SQRT2 0.70710678118f
GlobalContext Sys_Global = {0}; TextSystem Sys_Text; InputSystem Sys_Input; CheatsSystem Sys_Cheats = {.god=false,.noclip=false,.showLocation=true,.showFPS=true,.editMode=false,.showPhys=true}; RenderSystem Sys_Render; SystemUI Sys_UI;
SettingsSystem Sys_Settings = { // Potato defaults so initial state is good on first run for potatoes (e.g. won't crash for out of VRAM, or won't take 5min to init).
    .InputCodeSettings = {
        5,  /* Forward    = F */     0,/* Strafe Left= A */         18,/* Backpedal  = S */        3,/* Strafe Right= D */       100,/* Jump    = SPACE */      2,/* Crouch   = C        */ 23,/* Prone     = X */ 16,/* Lean Left = Q  */
        4,  /* Lean Right = E */    45,/* Sprint     = LEFT SHIFT */38,/* Turn Left  = LF ARROW */39,/* Turn Right  = RT ARROW */ 36,/* Look Up = UP ARROW */  37,/* Look Down= DN ARROW */ 20,/* Recent Log= U */ 26,/* Biomonitor= 1  */
        27, /* Sensaround = 2 */    28,/* Lantern    = 3 */         29,/* Shield     = 4 */       30,/* Infrared    = 5 */        31,/* Email   = 6 */         32,/* Booster  = 7        */ 33,/* Jumpjets  = 8 */ 56,/* Attack    = LMB*/
        57, /* Use        = RMB */  99,/* Menu/Back  = ESCAPE */    97,/* Toggle Mode= TAB */     17,/* Reload      = R */       128,/* Weapon += MWHEEL + */ 129,/* Weapon - = MWHEEL - */  6,/* Grenade   = G */ 19,/* Grenade + = T  */
        131,/* Grenade -  = B */    21,/* Ammo Type  = V */          9,/* Patch Use  = J */        8,/* Patch +     = I */       132,/* Patch - = , */         12,/* Full Map = M        */ 21,/* Swim Up   = V */  2,/* Swim Down = C  */
        103,/* Console    = `/~ */ 102/* Screenshot  = F12 */},
    .ScreenWidth=800u,.ScreenHeight=600u,.Fullscreen=0u,.FOV=65u,.Brightness=50u,.Gamma=50u,.FXAA=0u,.Shadows=0u,.Reflections=0u,.Vsync=0u,.ModelDetail=0u,.CurrentMonitor=0u,
    .GI=0u,.SpeakerMode=1u,.Reverb=0u,.VolumeMaster=100u,.VolumeMusic=25u,.VolumeMessage=75u,.VolumeEffects=100u,.Language=0u,.DynamicMusic=1u,.Footsteps=1u,.InvertLook=0u,
    .InvertCyberspaceLook=0u,.QuickItemPickup=0u,.QuickReloadWeapons=0u,.MouseSensitivity=10u,.NoShootMode=0u,.HeadBob=1u,.SSR_RES=4u};/*Ratio is (1 / SSR_RES) * res*/
Light lights[LIGHT_COUNT]; static u16 loadedLights = 0; LightAnimation lanims[LIGHT_COUNT]; static bool shadowBuffersCreated = false;
FrustumPlane lightFrustumPlanes[LIGHT_COUNT][6][6],playerFrustumPlanes[6];
u16 editModeSelection,editModeTestEntityDefinition=0; // Test instance and its model index
typedef struct { double shadowTime; u32 shadowmapIndirectionList[LIGHT_COUNT]; float shadDotThresh; } VoxenShadowSystem;
VoxenShadowSystem voxen_Shadow_System;
u16 loadedTexturesMaxIndex;
bool doubleSidedTexture[MAX_VALID_TEXTURE],transparentTexture[MAX_VALID_TEXTURE];
u32 drawCalls,uiDrawCalls,shadDrawCalls,vertsRendered,drawCallsNormal;
static const u8 Mpg_FrontPage=0,Mpg_Singleplayer=1,Mpg_Multiplayer=2,Mpg_NewGame=3,Mpg_Load=4,Mpg_Options=5,Mpg_Save=6,Mpg_IntroVideo=7,Mpg_CreditsVideo=8;
u8 currentMenuPage = Mpg_FrontPage; bool resDropdownOpen = false; int resDropdownCount=0,resSelectedIdx=0;
typedef struct {int w,h;} ResMode;
ResMode resModes[8];
typedef struct { Vector3 position; Quaternion rotation; u8 fov; u16 width,height; float near,far,finished; bool visible; } CamView;
CamView camViews[64]; u32 camViewTextures[64]; u8 camViewCount = 0; // Max is 8 cam views on level 8 + 3 sensaround views = 11.
FHandle console_log_file=0;
static i32 PosGetCellCoordX(float x) { return (u16)clamp((i32)vfloor((x - Sys_Global.worldMin_x[Sys_Global.curLev] + CELLXHALF) / CELL_SIZE), 0, WORLDX_0BASED); }
static i32 PosGetCellCoordZ(float z) { return (u16)clamp((i32)vfloor((z - Sys_Global.worldMin_z[Sys_Global.curLev] + CELLXHALF) / CELL_SIZE), 0, WORLDX_0BASED); }
static void DualLogMain(const char *prefix, const char *fmt, va_list args) {
    char buf[4096]; va_list c; __builtin_va_copy(c,args); StringFormatV(buf,sizeof(buf),fmt,c); __builtin_va_end(c); bool color = (prefix && prefix[0] == '\033');
    #ifdef WINDOWS
        FHandle out = GetStdHandle(color ? (u32)-12 : (u32)-11);
    #else
        FHandle out = color ? 2 : 1;
    #endif
    if (prefix) { OS_RawWrite(out,prefix,GetStringLength(prefix)); OS_RawWrite(out,"\033[0m ",5); } OS_RawWrite(out,buf,GetStringLength(buf));
    if (console_log_file != INVALID_FHANDLE) {
        if (prefix) { OS_Write(console_log_file, prefix, GetStringLength(prefix),"console.log"); OS_Write(console_log_file,"\033[0m ",5,"console.log"); }
        OS_Write(console_log_file, buf, GetStringLength(buf),"console.log");
    }
}
// =========================================================== Miscellaneous Helper Functions (and some libc replacements)
ENGINE_TO_MOD void DualLog(const char* fmt, ...) { va_list args; __builtin_va_start(args,fmt); DualLogMain(NULL,fmt,args); __builtin_va_end(args); }
ENGINE_TO_MOD void DualLogWarn(const char* fmt, ...) { va_list args; __builtin_va_start(args,fmt); DualLogMain("\033[1;38;5;208mWARN:",fmt,args); __builtin_va_end(args); }
ENGINE_TO_MOD void DualLogError(const char* fmt, ...) { va_list args; __builtin_va_start(args,fmt); DualLogMain("\033[1;31mERROR:",fmt,args); __builtin_va_end(args); }
void* MemSetToVForNBytes(void *dst, int c, size_t n) { unsigned char *p=(unsigned char *)dst; unsigned char v=(unsigned char)c; while (n--) {*p++=v;} return dst; } // memset replacement
void BmpWrite(char const *filename, int x, int y, const void *data) {
    FHandle f = OS_OpenWriteonly(filename);
    if (f == INVALID_FHANDLE) { DualLogError("Failed to open %s for writing\n", filename); return; }

    u32 fileSize = 14 + 108 + (u32)x * y * 4; // BMP file header (14 bytes)
    unsigned char fileHeader[14] = {'B','M',fileSize & 0xFF,(fileSize >> 8) & 0xFF,(fileSize >> 16) & 0xFF,(fileSize >> 24) & 0xFF,0,0,0,0,14 + 108,0,0,0};
    unsigned char infoHeader[108]={0}; *(u32*)(infoHeader+0)=108;/*size*/
    *(u32*)(infoHeader+4)=(u32)x;/*w*/ *(u32*)(infoHeader+8)=(u32)-y;/*h*/ *(u16*)(infoHeader+12)=1;/*planes*/ *(u16*)(infoHeader+14)=32;/*bit count*/ *(u32*)(infoHeader+16)=3;/*bit fields*/
    *(u32*)(infoHeader+40)=0x000000FF;/*Red*/ *(u32*)(infoHeader + 44) = 0x0000FF00;/*Green*/ *(u32*)(infoHeader + 48) = 0x00FF0000;/*Blue*/ *(u32*)(infoHeader + 52) = 0x00000000;/*Alpha*/
    OS_Write(f,fileHeader,14,filename); OS_Write(f,infoHeader,108,filename);
    const unsigned char *pixels = (const unsigned char *)data;
    for (int j=y-1;j>=0;--j) OS_Write(f,(void*)(pixels + j*x*4),(size_t)x*4,filename);
    OS_Close(f);
}

#ifdef WINDOWS
    ENGINE_TO_MOD double get_time() { static LARGE_INTEGER frequency,counter; static i32 init=0; if (!init) { QueryPerformanceFrequency(&frequency); init=1; } QueryPerformanceCounter(&counter); return (double)counter.QuadPart / frequency.QuadPart; }
#else
    ENGINE_TO_MOD double get_time() { struct {i64 s,ns;} ts; i64 ret; __asm__ __volatile__("syscall":"=a"(ret):"a"(228),"D"(1),"S"(&ts):"rcx","r11","memory"); if (ret != 0) {return 0.0;} return (double)ts.s + (double)ts.ns * 1e-9; } // Full time in seconds, 1 for MONOTONIC, Note that using clock_gettime wasn't any better for performance.
#endif

void DebugRAM(const char *context) { // Get USS aka the total RAM uniquely allocated for the process (btop shows RSS so pulls in shared libs and double counts shared RAM).
#ifdef DEBUG_RAM_OUTPUT
    static void* heap_start = (void*)-1;
    if (heap_start == (void*)-1) heap_start = OS_Brk(NULL);
    void* current_brk = OS_Brk(NULL);
    size_t heap_bytes = (size_t)((char*)current_brk - (char*)heap_start); size_t uss_bytes = 0;
    long fd = OS_OpenReadonly("/proc/self/smaps_rollup");
    if (fd == INVALID_FHANDLE) { DualLogError("Failed to open /proc/self/smaps_rollup\n"); return; }

    char buf[4096]; long bytes_read = OS_Read(fd,buf,sizeof(buf)-1);
    if (bytes_read > 0) { buf[bytes_read] = '\0'; } else buf[0] = '\0';
    OS_Close(fd); char* p = buf;
    while (*p) {
        if (CompareMemoryForNBytes(p,"Private_",8) == 0) {
            p += 8;
            size_t val = 0;
            if (CompareMemoryForNBytes(p,"Clean",5) !=0 && CompareMemoryForNBytes(p,"Dirty",5) != 0) { p++; continue; }
            while (*p && *p != ':') p++; if (*p != ':') { p++; continue; }
            
            p++;
            while (*p == ' ' || *p == '\t') p++;
            while (*p >= '0' && *p <= '9') { val = val * 10 + (*p - '0'); p++; }
            uss_bytes += val * 1024;
        }
        
        p++;
    }

    DualLog("Memory at %s: Heap %u b (%u KB | %.2f MB), USS %u b (%u KB | %.2f MB)\n",context,heap_bytes,heap_bytes / 1024,heap_bytes / 1024.0 / 1024.0,uss_bytes,uss_bytes / 1024,uss_bytes / 1024.0 / 1024.0);
#else
    (void)context;
#endif
}

ENGINE_TO_MOD void Screenshot() {
    if (!TakeScreenshot() || Sys_Global.current_time <= Sys_Global.screenshotTimeout) return;
    
    Sys_Global.screenshotTimeout = Sys_Global.current_time + 1.0; // Prevent saving more than 1 per second for sanity purposes.
    OS_MakeFolder("Screenshots"); u16 w = Sys_Settings.ScreenWidth, h = Sys_Settings.ScreenHeight;
    unsigned char* pixels = OS_Alloc(w * h * 4 * sizeof(char));
    glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,pixels);
    Vector3 p = Sys_Global.instances[PLAYER1].position;
    char filename[96]; StringFormat(filename,sizeof(filename),"Screenshots/%.2f_x%.1f_y%.1f_z%.1f.bmp",get_time(),p.x,p.y,p.z);
    BmpWrite(filename,w,h,pixels); DualLog("Saved screenshot %s\n",filename);
    OS_DeallocateRAM(pixels,w * h * 4 * sizeof(char));
}

u32 random_range_rng = 0x12345678u;
u32 xs32() { u32 x = random_range_rng; x ^= x << 13; x ^= x >> 17; x ^= x << 5; return random_range_rng = x ? x : 0xdeadbeefu; }
ENGINE_TO_MOD u8 random_range_u8(u8 a, u8 b) { if (a > b) { u8 temp = a; a = b; b = temp; } u32 r = (u32)b - a; u32 t = 256u - (256u % r), v; do v = (u8)xs32(); while (v >= t); return (a==b) ? a : a + (v % r); }
ENGINE_TO_MOD u32 random_range_u32(u32 a, u32 b) { if (a > b) { u32 temp = a; a = b; b = temp; } return (a==b) ? a : a + (u32)(((u64)xs32() * ((u64)b - a)) >> 32); }
ENGINE_TO_MOD i32 random_range_i32(i32 a, i32 b) { if (a > b) { i32 temp = a; a = b; b = temp; } return (a==b) ? a : a + (i32)(((u64)xs32() * ((u32)b - (u32)a)) >> 32); }
ENGINE_TO_MOD float random_range(float a, float b) { return a + (b - a) * ((float)(xs32() >> 8) * (1.0f / (1U << 24))); }
ENGINE_TO_MOD float lerp(float min, float max, float val) { return min + (max - min) * vclamp(val,0.0f,1.0f); }
ENGINE_TO_MOD float inverse_lerp(float min, float max, float val) { return (min == max) ? 0.0f : vclamp((val - min) / (max - min),0.0f,1.0f); }
ENGINE_TO_MOD float smooth_damp(float current, float target, float *current_velocity, float smooth_time) { 
    if (smooth_time < 0.0001f) smooth_time = 0.0001f;
    float omega = 2.0f / smooth_time;
    float x = omega * (float)Sys_Global.deltaTime;
    float exp = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);
    float change = current - target;
    float original_to = target;
    target = current - change;
    float temp = (*current_velocity + omega * change) * (float)Sys_Global.deltaTime;
    *current_velocity = (*current_velocity - omega * temp) * exp;
    float output = target + (change + temp) * exp;
    if ((original_to - current > 0.0f) == (output > original_to)) { output = original_to; *current_velocity = (output - original_to) / (float)Sys_Global.deltaTime; }
    return output;
}

ENGINE_TO_MOD size_t GetStringLength(const char* s) { if (s == NULL) {return 0;} const char *p = s; while (*(p++)); return (size_t)(p - s - 1); } // strlen replacement
char* data_parser_trim(char* s) {
    while (CharacterIsEmpty((unsigned char)*s)) s++;
    if (*s == 0) return s;

    char* e = s + GetStringLength(s) - 1;
    while (e > s && CharacterIsEmpty((unsigned char)*e)) e--;
    e[1] = 0; return s;
}

i32 StringToInt(const char *str) { // atoi replacement, needed separately from fast_atoi for user console input
    while (CharacterIsEmpty(*str)) {str++;} int sign = 1; if (*str == '-') { sign = -1; str++; } else if (*str == '+') {str++;} if (*str < '0' || *str > '9') return 0;
    
    i64 result = 0;
    while (*str >= '0' && *str <= '9') { int digit = *str - '0'; if (result > (2147483647 - digit) / 10) {return (sign == 1) ? 2147483647 : -2147483648;} result = result * 10 + digit; str++; }
    return (i32)(sign * result);
}

ENGINE_TO_MOD bool CharacterIsEmpty(const char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r'; } // isspace replacement
ENGINE_TO_MOD bool StringIsEmpty(const char* a) { size_t size = GetStringLength(a); for(size_t i=0;i<size;++i) { if (a[i] == '\0') {break;} if (!CharacterIsEmpty(a[i])) {return false;} } return true; } // C# String.IsNullOrWhiteSpace replacement
bool StringsEqual(const char* a, const char* b) { // !strcmp replacement (hated its inverted logic)
    size_t size = GetStringLength(a); if (size != GetStringLength(b)) return false;
    for (size_t i=0;i<size;++i) { if (a[i] != b[i]) {return false;} if (a[i] == '\0') {break;} }
    return true;
}

int StringCompareUpToLength(const char* s1, const char* s2, size_t n) { const unsigned char *p1 = (const unsigned char*)s1, *p2 = (const unsigned char*)s2; while (n-- > 0) { if (*p1 != *p2) {return (*p1 < *p2) ? -1 : 1;}  if (*p1 == '\0') {break;} p1++; p2++; } return 0; } // !strncmp replacement (yes inverted for sanity)
ENGINE_TO_MOD void StringCopyInto_A_From_B(char* a, const char* b, size_t bufferSize) { // strcpy replacement
    size_t size2=GetStringLength(b); if (size2>=bufferSize) { DualLogError("B is bigger than buffer limit %u! B: %s\n",bufferSize,b); OS_Exit(1); }
    
    for (size_t i=0;i<size2;++i) a[i] = b[i];
    a[size2] = '\0';
}

void StringCopyInto_A_SubstringFrom_B(char* a, size_t substringSize, const char* b, size_t bufferSize) { // strncpy replacement (hopefully my mnemonic "SubstringFrom_B" will help me remember substringSize comes before be in the args passed)
    if (substringSize >= bufferSize) { DualLogError("Substring too large for buffer! %u >= %u\n",substringSize,bufferSize); OS_Exit(1); }
    
    bool reachedEndB = false;
    for (size_t i= 0;i<substringSize;++i) { if (!reachedEndB && b[i] == '\0') {reachedEndB = true;} if (reachedEndB) {a[i] = '\0';} else {a[i] = b[i];/*Normal copy*/} }
    a[substringSize] = '\0'; 
}

void StringConcatenate(char* a, const char* b, size_t bufferSize) { // strcat replacement
    size_t size  = GetStringLength(a), size2 = GetStringLength(b);
    if (size + size2 >= bufferSize) { DualLogError("Strings to large to concat, will overflow buffer: GetStringLength(%s{%u} + %s{%u} > %u)\n",a,size,b,size2,bufferSize); OS_Exit(1); }

    char* dest = a + size;
    for (size_t i=0;i<size2; ++i) dest[i] = b[i];
    dest[size2] = '\0';
}

char CharToLower(const char c) { return c + ((c >= 'A' && c <= 'Z') ? 32 : 0); } // If uppercase 'A'-'Z' (65-90), +32 into 'a'-'z' (97-122)
char* StringFindSubstring(const char* haystack, const char* needle) { // strstr replacement
    if (needle[0] == '\0') return (char*)haystack;

    for (size_t i = 0; haystack[i] != '\0'; ++i) {
        if (haystack[i] == needle[0]) { // If the first character matches, check the rest of the needle
            size_t h_idx = i, n_idx = 0;
            while (haystack[h_idx] != '\0' && needle[n_idx] != '\0' && haystack[h_idx] == needle[n_idx]) { h_idx++; n_idx++; }
            if (needle[n_idx] == '\0') return (char*)&haystack[i];
        }
    }

    return NULL; // No match found
}

const char* StringFindLastChar(const char* str, const char c) { const char* lastSeen = NULL; do { if (*str == c) lastSeen = str; } while (*str++); return lastSeen; } // strrchr replacement
ENGINE_TO_MOD char* StringFindFirstCharWithin(const char *s, char c) { char* stringwalker = (char*)s; while (*stringwalker != c) { if (!*stringwalker) {return NULL;} stringwalker++; } return stringwalker; } // strchr replacement
void DoubleToStringFixed(char* dest, double value, int decimalPlaces, size_t bufferSize) {
    if (decimalPlaces < 0 || decimalPlaces > 9) { DualLogError("DoubleToStringFixed: decimalPlaces out of range\n"); OS_Exit(1); }
    if (value < 0.0) { if (bufferSize < 2) {OS_Exit(1);} *dest++ = '-'; bufferSize--; value = -value; }
    u64 intPart = (u64)value;
    char temp[32];
    size_t len = 0;
    if (intPart == 0) temp[len++] = '0';
    else { while (intPart > 0) { temp[len++] = '0' + (intPart % 10); intPart /= 10; } }

    if (len >= bufferSize) OS_Exit(1);
    for (size_t i = 0; i < len; ++i) dest[i] = temp[len - 1 - i];
    dest += len; bufferSize -= len;
    if (decimalPlaces == 0) { *dest = '\0'; return; }
    if (bufferSize < 1) OS_Exit(1);
    
    *dest++ = '.'; bufferSize--;
    double frac = value - (u64)value, scale = 1.0;
    for (int i = 0; i < decimalPlaces; ++i) scale *= 10.0;
    u64 fracPart = (u64)(frac * scale + 0.5);
    for (int i = decimalPlaces - 1; i >= 0; --i) { if (bufferSize < 1) {OS_Exit(1);} dest[i] = '0' + (fracPart % 10); fracPart /= 10; }
    dest[decimalPlaces] = '\0';
}

void StringAppendLiteral(char* dest, const char* literal, size_t bufferSize) {
    size_t curLen = GetStringLength(dest), litLen = GetStringLength(literal);
    if (curLen + litLen >= bufferSize) { DualLogError("StringAppendLiteral overflow\n"); OS_Exit(1); }
    
    for (size_t i = 0; i < litLen; ++i) dest[curLen + i] = literal[i];
    dest[curLen + litLen] = '\0';
}

ENGINE_TO_MOD int StringFormatV(char* buffer, size_t bufferSize, const char* format, va_list args) {
    if (bufferSize == 0) return 0;

    size_t pos = 0;
    const char* f = format;
    while (*f && pos < bufferSize - 1) {
        if (*f != '%') { buffer[pos++] = *f++; continue; } 
        f++; // skip '%'
        int width = 0;
        char padChar = ' ';
        if (*f == '0') { padChar = '0'; f++; }
        while (*f >= '0' && *f <= '9') { width = width * 10 + (*f - '0'); f++; }
        int decimals = 9;
        if (*f == '.') { f++; if (*f >= '1' && *f <= '9') { decimals = *f - '0'; } f++; }
        switch (*f) {
            case 'x': {
                unsigned int val = __builtin_va_arg(args, unsigned int);
                char num[32];
                int i = 0;
                const char* hexChars = "0123456789abcdef";
                do { num[i++] = hexChars[val % 16]; val /= 16; } while (val);
                while (i < width && pos < bufferSize - 1) { buffer[pos++] = padChar; width--; }
                while (i-- > 0 && pos < bufferSize - 1) buffer[pos++] = num[i];
            } break;
            case 'u': {
                unsigned int val = __builtin_va_arg(args, unsigned int);
                char num[32];
                int i = 0;
                do { num[i++] = '0' + (val % 10); val /= 10; } while (val);
                while (i < width && pos < bufferSize - 1) { buffer[pos++] = padChar; width--; }
                while (i-- > 0 && pos < bufferSize - 1) buffer[pos++] = num[i];
            } break;
            case 'c': {
                char c = (char)__builtin_va_arg(args,int);
                if (pos < bufferSize - 1) buffer[pos++] = c;
            } break;
            case 'd':
            case 'i': {
                int val = __builtin_va_arg(args, int);
                if (val < 0) { if (pos < bufferSize - 1) buffer[pos++] = '-'; val = -val; }
                char num[32];
                int i = 0;
                do { num[i++] = '0' + (val % 10); val /= 10; } while (val);
                while (i < width && pos < bufferSize - 1) { buffer[pos++] = padChar; width--; }
                while (i-- > 0 && pos < bufferSize - 1) buffer[pos++] = num[i];
            } break;
            case 's': {
                const char* s = __builtin_va_arg(args,const char*);
                size_t len = GetStringLength(s);
                if (pos + len >= bufferSize) len = bufferSize - pos - 1;
                for (size_t i = 0; i < len; ++i) buffer[pos++] = s[i];
            } break;
            case 'f': {
                double val = __builtin_va_arg(args,double);
                char num[64];
                DoubleToStringFixed(num, val, decimals, sizeof(num));
                size_t len = GetStringLength(num);
                if (pos + len >= bufferSize) len = bufferSize - pos - 1;
                for (size_t i = 0; i < len; ++i) buffer[pos++] = num[i];
            } break;
            case '%': if (pos < bufferSize - 1) buffer[pos++] = '%'; break;
        }
        f++;
    }
    buffer[pos] = '\0';
    return (int)pos;
}

ENGINE_TO_MOD int StringFormat(char* buffer, size_t bufferSize, const char* format, ...) { va_list args; __builtin_va_start(args,format); int ret = StringFormatV(buffer,bufferSize,format,args); __builtin_va_end(args); return ret; } // snprintf replacement
char* GetNextStringUpToNewlineOrEOF(char* buf, int size, FHandle fd) { // fgets replacement, not thread safe but we don't do multithreading
    if (size <= 1 || buf == NULL) return NULL;

    char* p = buf; int remaining = size - 1; static int pos = 0, end = 0; static char buffer[4096];
    while (remaining > 0) {
        if (pos >= end) {
            long n = OS_Read(fd,buffer,sizeof(buffer)); if (n <= 0 && p == buf) return NULL;
            
            pos = 0; end = (int)n;
        }

        while (remaining > 0 && pos < end) { char c = buffer[pos++]; *p++ = c; remaining--; if (c == '\n') goto done; }
    }

    done:
    *p = '\0';
    return buf;
}

extern FHandle levelFileHandle;
ENGINE_TO_MOD char* GetLevelFileNextStringUpToNewlineOrEOF(char* buf, int size) { return GetNextStringUpToNewlineOrEOF(buf,size,levelFileHandle); }
ENGINE_TO_MOD Vector3 GetEntityLocalSpawnPointFromUnrotatedOffsetVector(Entity* originator, Vector3 offsetFromOriginator) {
    Vector3 scaledOfs = mul_v3_v3_elementwise(offsetFromOriginator, originator->scale);
    Vector3 rotatedOfs = quat_rotate_vector(originator->rotation, scaledOfs);
    Vector3 result = V3_AplusB(originator->position, rotatedOfs);
    return result;
}

static inline int pntz(size_t p[2]) { return (p[0] != 1) ? __builtin_ctzll(p[0] - 1) : (p[1] ? 8 * sizeof(size_t) + __builtin_ctzll(p[1]) : 0); }
static inline void shl(size_t p[2], int n) { if (n >= 8 * (int)sizeof(size_t)) { p[1] = p[0]; p[0] = 0; n -= 8 * sizeof(size_t); } if (n) { p[1] = (p[1] << n) | (p[0] >> (8 * sizeof(size_t) - n)); p[0] <<= n; } }
static inline void shr(size_t p[2], int n) { if (n >= 8 * (int)sizeof(size_t)) { p[0] = p[1]; p[1] = 0; n -= 8 * sizeof(size_t); } if (n) { p[0] = (p[0] >> n) | (p[1] << (8 * sizeof(size_t) - n)); p[1] >>= n; } }
static void scycle(size_t w, unsigned char* ar[], int n) {
    unsigned char tmp[256]; size_t l;
    if (n<2) return;
    ar[n]=tmp;
    while (w) { l=w<256?w:256; MemCpyFromBtoAForNBytes(ar[n],ar[0],l); for(int i=0;i<n;i++){MemCpyFromBtoAForNBytes(ar[i],ar[i+1],l);ar[i]+=l;} w-=l; }
}

typedef int (*cmpfun)(const void*, const void*);
typedef int (*cmpfun_r)(const void*, const void*, void*);
#define AL (16*sizeof(size_t))
static void sift(unsigned char* head, size_t w, cmpfun_r cmp, void* arg, int ps, size_t lp[]) {
    unsigned char* ar[AL]; int i=1; ar[0]=head;
    while (ps>1) {
        unsigned char* rt=head-w, *lf=rt-lp[ps-2];
        if (cmp(ar[0],lf,arg)>=0 && cmp(ar[0],rt,arg)>=0) break;
        if (cmp(lf,rt,arg)>=0) { ar[i++&(AL-1)]=lf; head=lf; ps--; } else { ar[i++&(AL-1)]=rt; head=rt; ps-=2; }
    }
    scycle(w,ar,i&(AL-1));
}
static void trinkle(unsigned char* head, size_t w, cmpfun_r cmp, void* arg, size_t pp[2], int ps, int trusty, size_t lp[]) {
    unsigned char* ar[AL]; int i=1; ar[0]=head; size_t p[2]={pp[0],pp[1]};
    while (p[0]!=1||p[1]!=0) {
        unsigned char* ss=head-lp[ps];
        if (cmp(ss,ar[0],arg)<=0) break;
        if (!trusty&&ps>1) { unsigned char* rt=head-w,*lf=rt-lp[ps-2]; if(cmp(rt,ss,arg)>=0||cmp(lf,ss,arg)>=0) break; }
        
        ar[i++&(AL-1)]=ss; head=ss; int t=pntz(p); shr(p,t); ps+=t; trusty=0;
    }
    
    if (!trusty) { scycle(w,ar,i&(AL-1)); sift(head,w,cmp,arg,ps,lp); }
}

void qsort_new(void* base, size_t nel, size_t w, cmpfun cmp) {
    size_t lp[12*sizeof(size_t)], p[2]={1,0}, size=w*nel; int ps=1,trail; if (!size) return;
    
    unsigned char* head=base, *high=head+size-w;
    for (size_t i=2;lp[0]=lp[1]=w,(lp[i]=lp[i-2]+lp[i-1]+w)<size;i++);
    cmpfun_r cmp_r=(cmpfun_r)(void*)cmp; void* arg=NULL;
    while (head<high) {
        if ((p[0]&3)==3) { sift(head,w,cmp_r,arg,ps,lp); shr(p,2); ps+=2; }
        else {
            if (lp[ps-1]>=((size_t)(high-head))) trinkle(head,w,cmp_r,arg,p,ps,0,lp);
            else                                  sift(head,w,cmp_r,arg,ps,lp);
            if (ps==1) { shl(p,1); ps=0; } else { shl(p,ps-1); ps=1; }
        }
        
        p[0]|=1; head+=w;
    }
    
    trinkle(head,w,cmp_r,arg,p,ps,0,lp);
    while (ps!=1||p[0]!=1||p[1]!=0) {
        if (ps<=1) { trail=pntz(p); shr(p,trail); ps+=trail; }
        else { shl(p,2); ps-=2; p[0]^=7; shr(p,1); trinkle(head-lp[ps]-w,w,cmp_r,arg,p,ps+1,1,lp); shl(p,1); p[0]|=1; trinkle(head-w,w,cmp_r,arg,p,ps,1,lp); }
        head-=w;
    }
}

float half_to_float(half h) {
    u32 s=(h&0x8000)<<16,e=(h&0x7C00)>>10,m=(h&0x03FF),out;
    if (e == 0){
        if (m == 0) out = s;
        else {
            e = 1;
            while ((m & 0x0400) == 0) { m <<= 1; e--; }
            m &= 0x03FF; e+=(127 - 15);
            out = s | (e << 23) | (m << 13);
        }
    } else if (e == 31) { out = s | 0x7F800000 | (m << 13); }
    else { e = e + (127 - 15); out = s | (e << 23) | (m << 13); }
    float f; MemCpyFromBtoAForNBytes(&f,&out,4);
    return f;
}
// ========================== 2D Textures Loading System
u32 totalPixels,totalPaletteColors;
typedef struct { u16 index; bool transparent; bool doublesided; char path[128]; } TextureData; typedef struct { TextureData* entries; u32 count; u32 capacity; } TextureDataParser; typedef struct { const char* data; int size; } RawTexture;
typedef struct TextureParseTask { u32 start_tex; u32 end_tex; RawTexture* raw_textures; i32* parsIdx; const TextureDataParser* parser; int tid; } TextureParseTask;                 typedef struct { u32 img_x, img_y; i32 img_n, img_out_n; u8* img_buffer, *img_buffer_end; } PngContext;
typedef struct { PngContext* s; u8* idata, *expanded, *out; } PngData; typedef struct { u16 fast[1<<9], firstcode[16], firstsymbol[16], value[288]; i32 maxcode[17]; u8 size[288]; } PngHuffman; typedef struct { u8 *zbuffer, *zbuffer_end, *zout, *zout_start; i32 num_bits; u32 code_buffer; PngHuffman z_length, z_distance; } pngzbuf;
enum { PNGFmt_none=0, PNGFmt_sub=1, PNGFmt_up=2, PNGFmt_avg=3, PNGFmt_paeth=4, PNGFmt_avg_first, PNGFmt_paeth_first };
PngArena png_arena_main; static PngArena* thread_png_arenas = NULL; static u8** textureIndexBuffers = NULL; static u32** texturePaletteBuffers = NULL; static u32* texturePaletteSizes = NULL; static i32* textureWidths = NULL; static i32* textureHeights = NULL;
void PngArenaInit(PngArena* arena) {if (!arena->base) { arena->base = OS_Alloc(PNG_ARENA_SIZE); arena->cursor = arena->base; arena->end = arena->base + PNG_ARENA_SIZE; } }
void* PngArenaAlloc(PngArena* a, size_t s) { if(!a->base||a->cursor+s>a->end)return NULL; void* p=a->cursor; a->cursor+=s; return p; }
static u32 PngGet32be(PngContext* s) { const u8* p = s->img_buffer; s->img_buffer += 4; return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]; }
static i32 BitReverse(i32 n, i32 b) { n=((n&0xAAAA)>>1)|((n&0x5555)<<1); n=((n&0xCCCC)>>2)|((n&0x3333)<<2); n=((n&0xF0F0)>>4)|((n&0x0F0F)<<4); n=((n&0xFF00)>>8)|((n&0x00FF)<<8); return n>>(16-b); }
static i32 PngZBuildHuffman(PngHuffman* z, const u8* sl, i32 num) {
    i32 i,k=0,code=0,nc[16],sz[17]={0}; MemSetToVForNBytes(z->fast,0,sizeof(z->fast)); if(num != 32) { for(i=0;i<num;++i)++sz[sl[i]]; } sz[0]=0;
    for(i=1;i<16;++i){
        if(sz[i]>(1<<i))return 0;
        nc[i]=code; z->firstcode[i]=(u16)code; z->firstsymbol[i]=(u16)k; code+=sz[i]; if(sz[i]&&code-1>=(1<<i))return 0;
        z->maxcode[i]=code<<(16-i); code<<=1; k+=sz[i];
    }
    z->maxcode[16]=0x10000;
    for(i=0;i<num;++i){ 
        int s=(num==32)?5:sl[i]; if(!s)continue;
        int c=nc[s]-z->firstcode[s]+z->firstsymbol[s]; u16 fv=(u16)((s<<9)|i); z->size[c]=(u8)s; z->value[c]=(u16)i;
        if(s<=9){ int j=BitReverse(nc[s],s); while(j<(1<<9)){z->fast[j]=fv; j+=(1<<s);} } ++nc[s];
    } return 1;
}

#define REFILL(z) if(z->num_bits<16){do{z->code_buffer|=(unsigned int)(*z->zbuffer++)<<z->num_bits;z->num_bits+=8;}while(z->num_bits<=24);}
static u32 PngZReceive(pngzbuf* z, int n) { REFILL(z); u32 k=z->code_buffer&((1u<<n)-1); z->code_buffer>>=n; z->num_bits-=n; return k; }
static u32 PngHuffman_decode(pngzbuf* a, PngHuffman* z) { REFILL(a); int b=z->fast[a->code_buffer&511], s; if(b){ s=b>>9; a->code_buffer>>=s; a->num_bits-=s; return b&511; } int k=BitReverse(a->code_buffer,16); for(s=10; k>=z->maxcode[s]; ++s); b=(k>>(16-s))-z->firstcode[s]+z->firstsymbol[s]; a->code_buffer>>=s; a->num_bits-=s; return z->value[b]; }
static int PngParseHuffmanBlock(pngzbuf* a) {
    u8* o=a->zout;
    static const int lb[]={3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227,258}, le[]={0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0}, db[]={1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577}, de[]={0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};
    for(;;){
        int z=PngHuffman_decode(a,&a->z_length);
        if(z<256)*o++=(u8)z; else if(z==256){ a->zout=o; return 1; } else { z-=257; int l=lb[z]+(le[z]?PngZReceive(a,le[z]):0); z=PngHuffman_decode(a,&a->z_distance); int d=db[z]+(de[z]?PngZReceive(a,de[z]):0); u8* p=o-d; while(l--)*o++=*p++; }
    }
}

static int PngComputeHuffmans(pngzbuf* a) {
    static const u8 dz[]={16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15}; u8 lc[286+32+137], cs[19]={0};
    u32 hl=PngZReceive(a,5)+257, hd=PngZReceive(a,5)+1, hc=PngZReceive(a,4)+4, nt=hl+hd, n=0;
    for(u32 i=0;i<hc;++i) cs[dz[i]]=(u8)PngZReceive(a,3); PngZBuildHuffman(&a->z_length,cs,19);
    while(n<nt){
        u32 c=PngHuffman_decode(a,&a->z_length);
        if(c<16)lc[n++]=(u8)c;
        else { u8 f=0;
            if(c==16){c=PngZReceive(a,2)+3; f=lc[n-1];}
            else if(c==17)c=PngZReceive(a,3)+3;
            else if(c==18)c=PngZReceive(a,7)+11;
            else return 0; MemSetToVForNBytes(lc+n,f,c); n+=c;
        }
    } return PngZBuildHuffman(&a->z_length,lc,hl) && PngZBuildHuffman(&a->z_distance,lc+hl,hd);
}

static int PngParseUncompressedBlock(pngzbuf* a) {
    u8 header[4]; i32 k = 0; if (a->num_bits & 7) PngZReceive(a, a->num_bits & 7);
    while (a->num_bits > 0) { header[k] = (u8)(a->code_buffer & 255); a->code_buffer >>= 8; a->num_bits -= 8; ++k; }
    if (k <= 0) header[0] = *a->zbuffer++;
    if (k <= 1) header[1] = *a->zbuffer++;
    if (k <= 2) header[2] = *a->zbuffer++;
    if (k <= 3) header[3] = *a->zbuffer++;
    i32 len = header[1] * 256 + header[0];
    MemCpyFromBtoAForNBytes(a->zout,a->zbuffer,len); a->zbuffer += len; a->zout += len;
    return 1;
}

static u8 PngZDefLen(int i) { return (i<144)?8:(i<256)?9:(i<280)?7:8; }
u8* PngDecode(const u8* buffer, i32 len, i32 initial_size, i32* outlen, PngArena* arena) {
    pngzbuf a = {0}; u8* p = (u8*)PngArenaAlloc(arena, initial_size), d_len[288]; i32 f, t;
    a.zbuffer = (u8*)buffer; a.zbuffer_end = (u8*)buffer+len; a.zout_start = a.zout = p; a.zbuffer += 2;
    do {
        f = PngZReceive(&a, 1); t = PngZReceive(&a, 2);
        if (t == 0) PngParseUncompressedBlock(&a);
        else {
            if (t == 1) { for(int i=0; i<288; ++i) d_len[i]=PngZDefLen(i); PngZBuildHuffman(&a.z_length, d_len, 288); PngZBuildHuffman(&a.z_distance, NULL, 32); }
            else PngComputeHuffmans(&a);
            PngParseHuffmanBlock(&a);
        }
    } while (!f);
    if (outlen) *outlen = (i32)(a.zout - a.zout_start); return a.zout_start;
}

static u8 first_row_filter[5] = {PNGFmt_none, PNGFmt_sub, PNGFmt_none, PNGFmt_avg_first, PNGFmt_paeth_first};
inline static i32 PngPaeth(i32 a, i32 b, i32 c) { i32 p = a+b-c, pa = vabs(p-a), pb = vabs(p-b), pc = vabs(p-c); return (pa <= pb && pa <= pc) ? a : (pb <= pc ? b : c); }
static i32 CreatePngImageArena(PngArena* arena, PngData* a, u8* raw, u32 raw_len, i32 out_n, u32 x, u32 y, i32 img_n) {
    u32 i, j, stride = x * out_n, w_bytes = (img_n * x * 8 + 7) >> 3; i32 k, f;
    if (raw_len < (w_bytes + 1) * y) return 0; a->out = (u8*)PngArenaAlloc(arena, (size_t)x * y * out_n);
    for (j = 0; j < y; ++j) {
        u8 *cur = a->out + stride * j, *prior = (j > 0) ? cur - stride : a->out;
        if ((f = *raw++) > 4) return 0; if (j == 0) f = first_row_filter[f];
        for (k = 0; k < img_n; ++k) {
            if (f == PNGFmt_up) cur[k] = raw[k] + prior[k];
            else if (f == PNGFmt_avg) cur[k] = raw[k] + (prior[k] >> 1);
            else if (f == PNGFmt_paeth) cur[k] = raw[k] + PngPaeth(0, prior[k], 0);
            else cur[k] = raw[k];
        }
        if (img_n != out_n) cur[img_n] = 255; raw += img_n; cur += out_n; prior += out_n;
        for (i = x - 1; i >= 1; --i, cur[img_n] = (img_n != out_n ? 255 : cur[img_n]), raw += img_n, cur += out_n, prior += out_n)
            for (k = 0; k < img_n; ++k) {
                if (f == PNGFmt_none) cur[k] = raw[k];
                else if (f == PNGFmt_sub) cur[k] = raw[k] + cur[k - out_n];
                else if (f == PNGFmt_up) cur[k] = raw[k] + prior[k];
                else if (f == PNGFmt_avg) cur[k] = raw[k] + ((prior[k] + cur[k - out_n]) >> 1);
                else if (f == PNGFmt_paeth) cur[k] = raw[k] + PngPaeth(cur[k - out_n], prior[k], prior[k - out_n]);
                else if (f == PNGFmt_avg_first) cur[k] = raw[k] + (cur[k - out_n] >> 1);
            }
    }
    return 1;
}

u8* PngLoad(const u8* buffer, int len, int* x, int* y, PngArena* arena) {
    if (arena->base) arena->cursor = arena->base;
    PngContext s; s.img_n = s.img_out_n = 0; s.img_buffer = (u8*)buffer; s.img_buffer_end = (u8*)buffer + len;
    PngData z = {0}; z.s = &s; u32 ioff = 0; z.expanded = z.idata = z.out = NULL; s.img_buffer += 8; s.img_x = s.img_y = 1;
    for (;;) {
        u32 length = PngGet32be(&s), type = PngGet32be(&s);
        switch (type) {
            case 0x49484452: s.img_x = PngGet32be(&s); s.img_y = PngGet32be(&s); s.img_buffer++; { i32 color = (*s.img_buffer++); s.img_buffer += 3; s.img_n = (color & 2 ? 3 : 1) + (color & 4 ? 1 : 0); } break;
            case 0x49444154: if (!z.idata) { z.idata = (u8*)PngArenaAlloc(arena, len + 16); ioff = 0; } MemCpyFromBtoAForNBytes(z.idata + ioff, s.img_buffer, length); s.img_buffer += length; ioff += length; break;
            case 0x49454E44: { u32 rL = s.img_x * s.img_y * s.img_n + s.img_y; z.expanded = (u8*)PngDecode(z.idata, ioff, rL, (i32*)(&rL), arena); s.img_out_n = (s.img_n + 1 == 4) ? 4 : s.img_n; CreatePngImageArena(arena,&z,z.expanded, rL,s.img_out_n,s.img_x,s.img_y,s.img_n); PngGet32be(&s); goto Label_parsesuccess; }
            default: s.img_buffer += length; break;
        }
        PngGet32be(&s);
    }
    Label_parsesuccess: *x = z.s->img_x; *y = z.s->img_y; return z.out;
}

static void* TextureParsingWorker(void* arg) {
    TextureParseTask* t = (TextureParseTask*)arg;
    for (u32 i = t->start_tex; i < t->end_tex; ++i) {
        i32 pIdx = t->parsIdx[i]; if (unlikely(pIdx < 0 || pIdx >= (i32)t->parser->count)) continue;
        doubleSidedTexture[i] = t->parser->entries[pIdx].doublesided; transparentTexture[i] = t->parser->entries[pIdx].transparent;
        const char* d = t->raw_textures[i].data; int sz = t->raw_textures[i].size; if (unlikely(!d || sz <= 0)) continue;
        int w=0, h=0; u8 *pix = PngLoad((const u8*)d, sz, &w, &h, &thread_png_arenas[t->tid]);
        if (!pix || w < 1 || h < 1) { OS_DeallocateRAM((void*)d, (size_t)sz); continue; }
        u32 nP = (u32)w * h, pSz = 0, *pal = (u32*)OS_Alloc(1024); u8 *idx = (u8*)OS_Alloc(nP), hash[1024] = {0};
        for (u32 p = 0; p < nP; ++p) {
            u32 c = ((u32*)pix)[p], s = (c * 0x9e3779b9u) & 1023;
            while (hash[s]) { if (pal[hash[s]-1] == c) { idx[p] = hash[s]-1; goto found; } s = (s+1) & 1023; }
            if (pSz >= 256) {
                u32 bIdx = 0, bDist = -1; u8 r1=c, g1=c>>8, b1=c>>16, a1=c>>24;
                for (u32 k=0; k<pSz; k++) {
                    i32 dr=(pal[k]&255)-r1, dg=((pal[k]>>8)&255)-g1, db=((pal[k]>>16)&255)-b1, da=(pal[k]>>24)-a1;
                    u32 dst = dr*dr + dg*dg + db*db + da*da; if (dst < bDist) { bDist = dst; bIdx = k; }
                }
                idx[p] = bIdx; continue;
            }
            pal[pSz] = c; idx[p] = pSz; hash[s] = (u8)(++pSz); found:;
        }
        textureIndexBuffers[i] = idx; texturePaletteBuffers[i] = pal; texturePaletteSizes[i] = pSz; textureWidths[i] = w; textureHeights[i] = h;
        OS_DeallocateRAM((void*)d, (size_t)sz);
    }
    return NULL;
}

static bool ParseTextureData(TextureDataParser *p, u16 maxS, const char *fn) {
    FHandle fd; int sz; char *data = OS_OpenAndAllocateFileBufferReadonly(fn, &fd, &sz), *cur = data, *end = data + sz;
    u32 line = 0, m_idx = 0;
    while (cur < end) {
        char *s = cur; while (cur < end && *cur != '\n' && *cur != '\r') cur++;
        size_t len = cur - s; line++; if (len <= 0) { cur++; continue; }
        while (CharacterIsEmpty(*s)) s++; char *le = s + (cur - s) - 1;
        while (le > s && CharacterIsEmpty(*le)) le--;
        if (*s == '\0' || (s[0] == '/' && s[1] == '/') || s[0] == '#') { if (cur < end && (*cur == '\r' || *cur == '\n')) cur++; continue; }
        char *col = StringFindFirstCharWithin(s, ':');
        if (col && StringCompareUpToLength(s, "index", col - s) == 0) {
            char *v = col + 1; while (CharacterIsEmpty(*v)) v++;
            u32 idx = parse_numberu32(v, s, line); if (idx > m_idx) m_idx = idx;
        }
        if (cur < end && (*cur == '\r' || *cur == '\n')) cur++;
    }
    if (!m_idx || m_idx >= maxS) { if (!m_idx) DualLogWarn("No entries in %s\n", fn); else DualLogWarn("Index %u too large in %s\n", m_idx, fn); OS_DeallocateRAM(data, sz); return true; }
    p->entries = OS_Alloc((p->count = p->capacity = m_idx + 1) * sizeof(TextureData));
    for (u32 i = 0; i < p->count; ++i) p->entries[i] = (TextureData){.index = U16_MAX};
    TextureData e = {.index = U16_MAX}; line = 0; cur = data;
    while (cur < end) {
        char *s = cur; while (cur < end && *cur != '\n' && *cur != '\r') cur++;
        size_t len = cur - s; line++; if (len < 3) { cur++; continue; }
        while (CharacterIsEmpty(*s)) s++; char *le = s + (cur - s) - 1; while (le > s && CharacterIsEmpty(*le)) le--;
        if (s[0] == '/' && s[1] == '/') { cur++; continue; }
        if (*s == '#') {
            if (e.path[0] && e.index < p->capacity) p->entries[e.index] = e;
            e = (TextureData){.index = U16_MAX}; size_t aL = le - s;
            if (aL >= sizeof(e.path)) aL = sizeof(e.path) - 1; MemCpyFromBtoAForNBytes(e.path,s + 1,aL); e.path[aL] = 0;
        } else {
            char *col = StringFindFirstCharWithin(s, ':');
            if (col) {
                char *k = s, *v = col + 1, tk[256] = {0}, tv[256] = {0};
                while (CharacterIsEmpty(*k) && k < col) k++; while (CharacterIsEmpty(*v) && v <= le) v++;
                size_t kL = col - k, vL = (le >= v) ? (le - v + 1) : 0;
                if (kL && vL) {
                    StringCopyInto_A_SubstringFrom_B(tk, kL, k, 256); StringCopyInto_A_SubstringFrom_B(tv,vL,v,256);
                    char *ke = tk + GetStringLength(tk) - 1, *ve = tv + GetStringLength(tv) - 1;
                    while (ke > tk && CharacterIsEmpty(*ke)) *ke-- = 0; while (ve > tv && CharacterIsEmpty(*ve)) *ve-- = 0;
                         if (StringsEqual(tk,      "index")) e.index       = parse_numberu16(tv,s,line);
                    else if (StringsEqual(tk,"transparent")) e.transparent = parse_bool(tv,s,line);
                    else if (StringsEqual(tk,"doublesided")) e.doublesided = parse_bool(tv,s,line);
                }
            }
        }
        if (cur < end && (*cur == '\r' || *cur == '\n')) cur++;
    }
    if (e.path[0] && e.index < p->capacity) p->entries[e.index] = e;
    OS_DeallocateRAM(data, sz); return true;
}

typedef struct { int width,height; unsigned char* pixels; } GLFWimage;
void VSetWindowIcon(GLFWimage*);
static __attribute__((noinline)) void LoadTextures() {
    double start_time = get_time();
    loadedTexturesMaxIndex = totalPixels = totalPaletteColors = 0u;
    TextureDataParser texture_parser; if (unlikely(!ParseTextureData(&texture_parser,MAX_VALID_TEXTURE, "./Data/textures.txt"))) { DualLogError("Could not parse ./Data/textures.txt!\n"); OS_Exit(1); }

    i32 maxIndex = -1;
    for (u32 k = 0; k < texture_parser.count; ++k) { if (texture_parser.entries[k].index > maxIndex && texture_parser.entries[k].index != U16_MAX) maxIndex = texture_parser.entries[k].index; }
    loadedTexturesMaxIndex = (u16)(maxIndex + 1);
    i32* parsIdx = OS_Alloc(loadedTexturesMaxIndex * sizeof(i32));
    MemSetToVForNBytes(parsIdx, -1, loadedTexturesMaxIndex * sizeof(i32));
    for (u32 k = 0; k < texture_parser.count; ++k) { if (texture_parser.entries[k].index < loadedTexturesMaxIndex) parsIdx[texture_parser.entries[k].index] = (i32)k; }
    DualLog("Loading textures (%u) ... ", texture_parser.count);
    RawTexture* rawTextures = OS_Alloc(loadedTexturesMaxIndex * sizeof(RawTexture));
    MemSetToVForNBytes(rawTextures,0,loadedTexturesMaxIndex * sizeof(RawTexture));
    for (u32 i = 0; i < loadedTexturesMaxIndex; ++i) {
        i32 p = parsIdx[i]; if (p < 0) continue;
        const char* path = texture_parser.entries[p].path;
        FHandle dummy_fd;
        int size = 0; rawTextures[i].data = (const char*)OS_OpenAndAllocateFileBufferReadonly(path,&dummy_fd,&size);
        rawTextures[i].size = size;
    }

    num_parse_threads = OS_GetNumThreads(); if (num_parse_threads < 1) {num_parse_threads = 1;} if (num_parse_threads > 32) {num_parse_threads = 32;}
    thread_png_arenas = (PngArena*)OS_Alloc((size_t)num_parse_threads * sizeof(PngArena));
    for (int t = 0; t < num_parse_threads; ++t) { thread_png_arenas[t].base = NULL; PngArenaInit(&thread_png_arenas[t]); }
    textureIndexBuffers = OS_Alloc(loadedTexturesMaxIndex * sizeof(u8*)); texturePaletteBuffers = OS_Alloc(loadedTexturesMaxIndex * sizeof(u32*));
    texturePaletteSizes = OS_Alloc(loadedTexturesMaxIndex * sizeof(u32));         textureWidths = OS_Alloc(loadedTexturesMaxIndex * sizeof(i32));
    textureHeights      = OS_Alloc(loadedTexturesMaxIndex * sizeof(i32));
    TextureParseTask tasks[32]; u32 chunk = (loadedTexturesMaxIndex + (u32)num_parse_threads - 1U) / (u32)num_parse_threads;
    for (int t = 0; t < num_parse_threads; ++t) { u32 start = ((u32)t * chunk); tasks[t] = (TextureParseTask){.start_tex=start,.end_tex=clamp(start+chunk,0,loadedTexturesMaxIndex),.raw_textures=rawTextures,.parsIdx=parsIdx,.parser=&texture_parser,.tid=t}; }
    OS_Thread workers[32];
    for (int t = 0; t < num_parse_threads; ++t) OS_ThreadCreate(&workers[t],TextureParsingWorker,&tasks[t]);
    for (int t = 0; t < num_parse_threads; ++t) OS_ThreadJoin(&workers[t]); //     for (int t = 0; t < num_parse_threads; ++t) TextureParsingWorker(&tasks[t]); // Single threaded alternative
    totalPixels = totalPaletteColors = 0u;
    for (u16 i = 0; i < loadedTexturesMaxIndex; ++i) { if (textureIndexBuffers[i]) { totalPixels += (u32)textureWidths[i] * textureHeights[i]; totalPaletteColors += texturePaletteSizes[i]; } }
    size_t offsets_size = loadedTexturesMaxIndex * sizeof(u32), palettes_size = totalPaletteColors * sizeof(u32), indices_size = totalPixels;
    size_t arena_size = offsets_size + palettes_size + indices_size;
    void* arena = OS_AllocateRAM(NULL,arena_size,0x1|0x2,0x20|0x02|0x08000,INVALID_FHANDLE);
    u8* cur = (u8*)arena;
    u32* textureOffsets = (u32*)cur; cur += offsets_size;
    i32* textureSizes = OS_AllocateRAM(NULL,loadedTexturesMaxIndex * 2 * sizeof(i32),0x1|0x2,0x02|0x20,INVALID_FHANDLE);
    u32* texturePaletteOffsets = OS_AllocateRAM(NULL,loadedTexturesMaxIndex * sizeof(u32),0x1|0x2,0x02|0x20,INVALID_FHANDLE);
    u32* texturePalettes = (u32*)cur; cur += palettes_size;
    u8* all_indices = cur;
    u32 pixel_base = 0, color_base = 0;
    for (u16 i=0;i<loadedTexturesMaxIndex;++i) {
        if (!textureIndexBuffers[i]) continue;
        
        u32 numP = (u32)textureWidths[i] * textureHeights[i]; u32 palS = texturePaletteSizes[i];
        textureOffsets[i] = pixel_base; texturePaletteOffsets[i] = color_base;
        textureSizes[i*2]     = textureWidths[i]; textureSizes[i*2 + 1] = textureHeights[i];
        MemCpyFromBtoAForNBytes(all_indices + pixel_base,textureIndexBuffers[i],numP);
        MemCpyFromBtoAForNBytes(texturePalettes + color_base,texturePaletteBuffers[i],palS * sizeof(u32));
        pixel_base += numP; color_base += palS;
        OS_DeallocateRAM(textureIndexBuffers[i],numP); OS_DeallocateRAM(texturePaletteBuffers[i],palS * sizeof(u32));
    }

    DualLog("total palette colors: %u, total pixels: %u...", totalPaletteColors,totalPixels);
    i32 packed_size = ((i32)totalPixels + 3) / 4 * sizeof(u32);
    glBindBuffer(GL_SSBO,Sys_Render.colorBufferID);
    void* dst = glMapBufferRange(GL_SSBO,0,packed_size,0x0002/*GL_MAP_WRITE_BIT*/|0x0004/*GL_MAP_INVALIDATE_RANGE_BIT*/);
    MemCpyFromBtoAForNBytes(dst,all_indices,packed_size); 
    glUnmapBuffer(GL_SSBO);
    glBindBuffer(GL_SSBO,Sys_Render.texturePalettesID);       glBufferData(GL_SSBO,totalPaletteColors * sizeof(u32),texturePalettes,GL_STATIC_DRAW);
    glBindBuffer(GL_SSBO,Sys_Render.textureOffsetsID);        glBufferData(GL_SSBO,loadedTexturesMaxIndex * sizeof(u32),textureOffsets,GL_STATIC_DRAW);
    glBindBuffer(GL_SSBO,Sys_Render.textureSizesID);          glBufferData(GL_SSBO,loadedTexturesMaxIndex * 2 * sizeof(i32),textureSizes,GL_STATIC_DRAW);
    glBindBuffer(GL_SSBO,Sys_Render.texturePaletteOffsetsID); glBufferData(GL_SSBO,loadedTexturesMaxIndex * sizeof(u32),texturePaletteOffsets,GL_STATIC_DRAW); glBindBuffer(GL_SSBO,0);
    OS_DeallocateRAM(texture_parser.entries,texture_parser.count * sizeof(TextureData)); OS_DeallocateRAM(arena,arena_size);
    OS_DeallocateRAM(rawTextures,loadedTexturesMaxIndex * sizeof(RawTexture));           OS_DeallocateRAM(parsIdx,loadedTexturesMaxIndex * sizeof(i32));
    OS_DeallocateRAM(textureIndexBuffers,loadedTexturesMaxIndex * sizeof(u8*));          OS_DeallocateRAM(textureSizes,loadedTexturesMaxIndex * 2 * sizeof(i32));
    OS_DeallocateRAM(texturePaletteBuffers,loadedTexturesMaxIndex * sizeof(u32*));       OS_DeallocateRAM(texturePaletteSizes,loadedTexturesMaxIndex * sizeof(u32));
    OS_DeallocateRAM(texturePaletteOffsets,loadedTexturesMaxIndex * sizeof(u32));        OS_DeallocateRAM(textureWidths,loadedTexturesMaxIndex * sizeof(i32));
    OS_DeallocateRAM(textureHeights,loadedTexturesMaxIndex * sizeof(i32));               for (int t=0;t<num_parse_threads;++t) OS_DeallocateRAM(thread_png_arenas[t].base,PNG_ARENA_SIZE);
    OS_DeallocateRAM(thread_png_arenas,(size_t)num_parse_threads * sizeof(PngArena));
    FHandle fp = OS_OpenReadonly(Sys_Global.global_winicon); // Load window icon
    int windowIconFileSize = OS_FileSize(fp);
    u8* file_buffer = OS_AllocateFileBackedRAMReadonly(windowIconFileSize,fp,Sys_Global.global_winicon);    
    OS_Close(fp); PngArenaInit(&png_arena_main);
    int w=1,h=1; unsigned char* pixels = PngLoad(file_buffer,windowIconFileSize,&w,&h,&png_arena_main);
    if (!pixels) { DualLogError("Failed to load icon: %s\n",Sys_Global.global_winicon); OS_Exit(1); }
    
    GLFWimage image = (GLFWimage){w,h,pixels}; VSetWindowIcon(&image);
    OS_DeallocateRAM(file_buffer,windowIconFileSize); OS_DeallocateRAM(png_arena_main.base,PNG_ARENA_SIZE); png_arena_main.base = NULL;
    DualLog(" took %.6f secs\n",get_time() - start_time);
    DebugRAM("After LoadTextures and after deallocation");
}
// ======================== 3D Models Loading System
u8** modelVertices = NULL; u16** modelTriangles = NULL; u32 modelVertexCounts[MODEL_IDX_MAX] = {0}; u16 modelTriangleCounts[MODEL_IDX_MAX] = {0}; float modelBounds[MODEL_IDX_MAX] = {0}; u16 loadedModelsMaxIndex = 0;
#define MAX_VERT_ELEMENT_SIZE 6964
#define MAX_OUTPUT_VERTS      36364
static float **thread_temp_pos = NULL, **thread_temp_nrm = NULL, **thread_temp_uv = NULL, **thread_out_verts = NULL; static u16** thread_out_tris = NULL;
typedef struct { const char* data; int size; } RawOBJ; typedef struct { u16 index; bool animated; u8 animationNum; char path[128]; } ModelData; typedef struct { ModelData* entries; u32 count; u32 capacity; } ModelDataParser;
static inline __attribute__((always_inline)) half float_to_half(float f) {
    u32 x; MemCpyFromBtoAForNBytes(&x,&f,4);
    u32 s = x>>31, ue = (x>>23)&0xff; i32 e = (i32)ue-127; u32 m = x&0x7fffff;
    if (ue == 0xff) return m ? (half)(0x7e00|(m>>13)|(s<<15)) : (half)((s<<15)|0x7c00);
    if (!ue && !m) return (half)(s<<15);
    if (e <= -24) return (half)(s<<15);
    if (e <= -14) { m = (m|0x800000) >> (-e-1); return (half)((s<<15)|(m>>13)); }
    if (e <= 15) { m += 0x1000; if (m >= 0x800000) { m = 0; e++; } return (half)((s<<15)|((e+15)<<10)|(m>>13)); }
    return (half)((s<<15)|0x7c00);
}

static inline __attribute__((always_inline)) float fast_atof(const char** p) { float v=0,s=1; while (**p == ' ' || **p == '\t') {(*p)++;} if (**p == '-') {s = -1; (*p)++;} while (**p >= '0' && **p <= '9') {v = v*10 + (*(*p)++ - '0');} if (**p == '.') {(*p)++; float sub=0.1f; while (**p >= '0' && **p <= '9') {v += (*(*p)++ - '0')*sub; sub *= 0.1f;}} return s * v; }
static inline __attribute__((always_inline)) i32 fast_atoi(const char** p) { i32 v = 0, s = 1; while (**p == ' ' || **p == '\t') (*p)++; if (**p == '-') { s = -1; (*p)++; } while (**p >= '0' && **p <= '9') {v = v*10 + (*(*p)++ - '0');} return v * s; }
typedef struct { u32 idx,key; } TriSort;
int cmp(const void* a, const void* b) { u32 ka=((const TriSort*)a)->key, kb=((const TriSort*)b)->key; return (ka > kb) - (ka < kb); } // branchless 1 or -1
static void OptimizeVertexCache(u16* idx, u32 ic, u32 vc) {
    if (ic < 3 || !vc) return;
    
    u32 tc = ic / 3;
    TriSort* t = OS_Alloc(tc*sizeof(TriSort));
    for (u32 i=0; i<tc; ++i) { u16* p = idx+i*3; u32 m = p[0]<p[1]?p[0]:p[1]; m = m<p[2]?m:p[2]; t[i].idx = i; t[i].key = m; }
    qsort_new(t, tc, sizeof(TriSort), cmp);
    u16* n = OS_Alloc(ic*sizeof(u16));
    for (u32 i=0; i<tc; ++i) { u16* s=idx+t[i].idx*3; u16* d=n+i*3; d[0]=s[0];d[1]=s[1];d[2]=s[2]; }
    MemCpyFromBtoAForNBytes(idx,n,ic*sizeof(u16));
    OS_DeallocateRAM(n, ic*sizeof(u16)); OS_DeallocateRAM(t, tc*sizeof(TriSort));
}

static u8* OptimizeVertexFetch(u8* v, u32* vc, u16* idx, u32 ic, size_t stride) {
    u32 oc = *vc; if (!oc || !ic) return v;
    u32 *remap = OS_Alloc(oc*sizeof(u32)), *first = OS_Alloc(oc*sizeof(u32));
    MemSetToVForNBytes(remap,0xFF,oc*sizeof(u32));
    u32 nc = 0;
    for (u32 i=0; i<ic; ++i) { u32 id = idx[i]; if (id < oc && remap[id] == 0xFFFFFFFFU) { remap[id] = nc; first[nc] = id; ++nc; } }
    u8* nv = OS_Alloc(nc*stride);
    for (u32 i=0; i<nc; ++i) MemCpyFromBtoAForNBytes(nv+i*stride,v+first[i]*stride,stride);
    for (u32 i=0; i<ic; ++i) if (idx[i] < oc) idx[i] = (u16)remap[idx[i]];
    *vc = nc;
    OS_DeallocateRAM(remap,oc*sizeof(u32)); OS_DeallocateRAM(first,oc*sizeof(u32));
    return nv;
}

static __attribute__((hot)) __attribute__((flatten)) bool ParseOBJ(u32 mindex, const char* __restrict d, int fs, float* __restrict tp, float* __restrict tn, float* __restrict tu, float* __restrict sv, u16* __restrict st, u8** ov, u32* ovc, u16** ot, u16* otc) {
    *ov = NULL; *ot = NULL; *ovc = *otc = 0;
    u32 pc=0,nc=0,uc=0,ec=0;
    float mx=1e9f,my=1e9f,mz=1e9f,Mx=-1e9f,My=-1e9f,Mz=-1e9f;
    const char *p = d, *e = d+fs;
    while (likely(p < e)) {
        while (p < e && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')) ++p;
        if (p >= e) break;
        if (*p == '#') { while (p < e && *p != '\n') ++p; continue; }
        if (*p == 'v') {
            ++p;
            if (*p == ' ') { if (unlikely(pc >= MAX_VERT_ELEMENT_SIZE)) return false;
                ++p; tp[pc*3] = fast_atof(&p); tp[pc*3+1] = fast_atof(&p); tp[pc*3+2] = fast_atof(&p); ++pc;
            } else if (*p == 'n' && p[1] == ' ') { p += 2;
                if (unlikely(nc >= MAX_VERT_ELEMENT_SIZE)) return false;
                tn[nc*3] = fast_atof(&p); tn[nc*3+1] = fast_atof(&p); tn[nc*3+2] = fast_atof(&p); ++nc;
            } else if (*p == 't' && p[1] == ' ') { p += 2;
                if (unlikely(uc >= MAX_VERT_ELEMENT_SIZE)) return false;
                tu[uc*2] = fast_atof(&p); tu[uc*2+1] = fast_atof(&p); ++uc;
            }
        } else if (*p == 'f' && p[1] == ' ') {
            p += 2;
            u32 vi[8]={0}, ti[8]={0}, ni[8]={0}; int nv = 0;
            while (nv < 8 && p < e && *p != '\n' && *p != '\r') {
                while (*p == ' ' || *p == '\t') ++p;
                if (*p == '\n' || *p == '\r' || *p == '#') break;
                long r = fast_atoi(&p);
                u32 v = (r>0) ? (u32)r : (r<0) ? (u32)((i32)pc + r) : 0; vi[nv] = v;
                if (*p == '/') {
                    ++p;
                    if (*p != '/') { r = fast_atoi(&p); u32 t = (r>0)?(u32)r:(r<0)?(u32)((i32)uc+r):0; ti[nv]=t; }
                    if (*p == '/') { ++p; r = fast_atoi(&p); u32 n = (r>0)?(u32)r:(r<0)?(u32)((i32)nc+r):0; ni[nv]=n; }
                }
                ++nv;
            }
            if (nv < 3) goto skip;
            for (int k=1; k<nv-1; ++k) {
                if (unlikely(ec + 3 > MAX_OUTPUT_VERTS)) {DualLogError("vert overflow!\n"); return false;}
                u32 tri[3] = {0, (u32)k, (u32)(k+1)};
                for (int t=0; t<3; ++t) {
                    int ix = tri[t];
                    u32 v = vi[ix] ? vi[ix]-1 : 0;
                    u32 tex = (ti[ix] && ti[ix] <= uc) ? ti[ix]-1 : 0;
                    u32 nrm = (ni[ix] && ni[ix] <= nc) ? ni[ix]-1 : 0;
                    float* dst = sv + (ec<<3);
                    dst[0] = -tp[v*3];   dst[1] =  tp[v*3+1]; dst[2] =  tp[v*3+2];
                    dst[3] = (nrm < nc) ? -tn[nrm*3]   : 0;
                    dst[4] = (nrm < nc) ?  tn[nrm*3+1] : 0;
                    dst[5] = (nrm < nc) ?  tn[nrm*3+2] : 0;
                    dst[6] = (tex < uc) ?  tu[tex*2]   : 0;
                    dst[7] = (tex < uc) ?  tu[tex*2+1] : 0;
                    float x=dst[0],y=dst[1],z=dst[2];
                    if (x < mx) mx = x; if (x > Mx) Mx = x;
                    if (y < my) my = y; if (y > My) My = y;
                    if (z < mz) mz = z; if (z > Mz) Mz = z;
                    st[ec] = (u16)ec; ++ec;
                }
            }
        skip:;
        } else while (p < e && *p != '\n') ++p;
    }
    if (unlikely(!ec)) return false;

    #define HASH_SIZE 65536
    u32* ht = OS_Alloc(HASH_SIZE * sizeof(u32));
    MemSetToVForNBytes(ht,0xFF,HASH_SIZE * sizeof(u32));
    u32* rem = (u32*)st; u32 ucnt = 0;
    for (u32 i=0; i<ec; ++i) {
        const float* v = sv + (i<<3);
        u64 h = *(u32*)(v+0) ^ *(u32*)(v+2) ^ *(u32*)(v+4) ^ *(u32*)(v+6); u32 s = (u32)(h ^ (h>>32)) & (HASH_SIZE-1);
        while (ht[s] != 0xFFFFFFFFU) { if (CompareMemoryForNBytes(sv+(ht[s]<<3), v, 32) == 0) { rem[i] = ht[s]; goto nxt; } s = (s+1) & (HASH_SIZE-1); }
        ht[s] = ucnt; rem[i] = ucnt;
        MemCpyFromBtoAForNBytes(sv+(ucnt<<3), v, 32);
        ++ucnt; nxt:;
    }

    u8* fv = OS_Alloc((size_t)ucnt * VERTEX_ATTRIBUTES_SIZE); u8* dst = fv;
    for (u32 i=0; i<ucnt; ++i) { 
        const float* src = sv + (i<<3);
        for (u32 j=0; j<8; ++j) { *(half*)dst = float_to_half(src[j]); dst += 2; }
    }

    u16* ft = OS_Alloc(ec * sizeof(u16));
    for (u32 i=0; i<ec; ++i) ft[i] = (u16)rem[i];
    OptimizeVertexCache(ft,ec,ucnt);
    u32 oldc = ucnt; u8* optv = OptimizeVertexFetch(fv,&ucnt,ft,ec,VERTEX_ATTRIBUTES_SIZE); // Allocates and returns new buffer; safe to free fv after.
    OS_DeallocateRAM(fv,oldc * VERTEX_ATTRIBUTES_SIZE);
    *ov = optv; *ovc = ucnt; *ot = ft; *otc = ec/3;
    modelBounds[mindex] = vmax(vabs(mx),vmax(vabs(my),vmax(vabs(mz),vmax(Mx,vmax(My,Mz))))); 
    OS_DeallocateRAM(ht,HASH_SIZE * sizeof(u32));
    return true;
}

typedef struct { u32 start, end; RawOBJ* raw; int tid; } ModelParseTask;
static void* ModelParsingWorker(void* arg) {
    ModelParseTask* t = arg;
    for (u32 i = t->start; i < t->end; ++i) {
        RawOBJ obj = t->raw[i]; if (unlikely(!obj.data || obj.size <= 0)) continue;

        if (!ParseOBJ(i,obj.data,obj.size,thread_temp_pos[t->tid],thread_temp_nrm[t->tid],thread_temp_uv[t->tid],thread_out_verts[t->tid],thread_out_tris[t->tid],&modelVertices[i],&modelVertexCounts[i],&modelTriangles[i],&modelTriangleCounts[i])) continue;
    }
    return NULL;
}

bool ParseModelData(ModelDataParser *p, u16 maxSz, const char *fn) {
    FHandle fd; int sz; char* buf = OS_OpenAndAllocateFileBufferReadonly(fn, &fd, &sz);
    char *c = buf, *e = buf + sz; u32 maxidx = 0, ln = 0;
    while (c < e) {  // first pass - find max index
        char* s = c; while (c < e && *c != '\n' && *c != '\r') ++c;
        if (c - s > 5) {
            while (CharacterIsEmpty(*s)) ++s;
            char* col = StringFindFirstCharWithin(s, ':');
            if (col && StringCompareUpToLength(s, "index", col - s) == 0) {
                u32 idx = parse_numberu32(col+1, s, ln);
                if (idx > maxidx) maxidx = idx;
            }
        }
        if (c < e && *c == '\r') ++c; if (c < e && *c == '\n') ++c; ++ln;
    }

    if (!maxidx) { DualLogWarn("No entries in %s\n", fn); OS_DeallocateRAM(buf,sz); return true; }
    if (maxidx >= maxSz) { DualLogWarn("Index too large in %s\n", fn); OS_DeallocateRAM(buf,sz); return true; }

    u32 cnt = maxidx + 1;
    ModelData* ents = OS_Alloc(cnt * sizeof(ModelData));
    p->entries = ents; p->capacity = p->count = cnt;
    for (u32 i=0; i<cnt; ++i) ents[i] = (ModelData){U16_MAX, false, 255, {0}};
    ModelData cur = {U16_MAX, false, 255, {0}};
    c = buf; e = buf+sz; ln = 0;
    while (c < e) {
        char* s = c; while (c < e && *c != '\n' && *c != '\r') ++c;
        size_t len = c - s; ++ln;
        if (len < 3) { if (c<e && (*c=='\r'||*c=='\n')) ++c; continue; }

        while (CharacterIsEmpty(*s)) ++s;
        char* le = s + len - 1; while (le > s && CharacterIsEmpty(*le)) --le;
        if (*s == '/' && s[1] == '/') goto next;
        if (*s == '#') {
            if (cur.path[0] && cur.index != U16_MAX && cur.index < cnt) ents[cur.index] = cur;
            cur = (ModelData){U16_MAX, false, 255, {0}};
            if (le > s) {
                size_t pl = le - s; if (pl >= sizeof(cur.path)) pl = sizeof(cur.path)-1;
                MemCpyFromBtoAForNBytes(cur.path,s+1,pl); cur.path[pl] = 0;
            }
            goto next;
        }

        char* col = StringFindFirstCharWithin(s, ':');
        if (col) {
            char k[256]={0}, v[256]={0};
            StringCopyInto_A_SubstringFrom_B(k, col-s, s, 256);
            StringCopyInto_A_SubstringFrom_B(v, le-col, col+1, 256);
            if (StringsEqual(k,"index"))             cur.index = parse_numberu16(v,s,ln);
            else if (StringsEqual(k,"animationNum")) cur.animationNum = parse_numberu16(v,s,ln);
            else if (StringsEqual(k,"animated"))     cur.animated = parse_numberu8(v,s,ln);
        }
        next:
        if (c < e && *c == '\r') ++c; if (c < e && *c == '\n') ++c;
    }
    if (cur.path[0] && cur.index != U16_MAX && cur.index < cnt) ents[cur.index] = cur;
    OS_DeallocateRAM(buf, sz);
    return true;
}

static void UploadMdlBuffer(u32 target, u32 buf, const void* data, size_t size) { glBindBuffer(target,buf); glBufferData(target,size,NULL,GL_STATIC_DRAW); void* mp = glMapBufferRange(target,0,size,0x0002/*GL_MAP_WRITE_BIT*/|0x0008/*GL_MAP_INVALIDATE_BUFFER_BIT*/); MemCpyFromBtoAForNBytes(mp,data,size); glUnmapBuffer(target); }
void LoadModels() {
    double startModelTime = get_time();
    ModelDataParser mp = {0};
    if (!ParseModelData(&mp, MODEL_IDX_MAX,"./Data/models.txt")) { DualLogError("Failed models.txt\n"); OS_Exit(1); }

    DualLog("Loading models (%d) ...",mp.count);
    u32 maxid = 0;
    for (u32 i=0; i<mp.count; ++i) { if (mp.entries[i].index != U16_MAX && mp.entries[i].index > maxid) maxid = mp.entries[i].index; }
    loadedModelsMaxIndex = (u16)maxid + 1;
    num_parse_threads = clamp(OS_GetNumThreads(),1,32);
    modelVertices = OS_Alloc(loadedModelsMaxIndex * sizeof(u8*)); modelTriangles = OS_Alloc(loadedModelsMaxIndex * sizeof(u16*));
    size_t n = loadedModelsMaxIndex;
    size_t arena = n*sizeof(i32) + n*sizeof(RawOBJ) + 5*num_parse_threads*sizeof(float*) + (size_t)num_parse_threads * ((MAX_VERT_ELEMENT_SIZE*3 + MAX_VERT_ELEMENT_SIZE*3 + MAX_VERT_ELEMENT_SIZE*2)*sizeof(float) + MAX_OUTPUT_VERTS*8*sizeof(float) + MAX_OUTPUT_VERTS*sizeof(u32));
    void* arena_base = OS_Alloc(arena); char* p = arena_base;
    i32* idxmap = (i32*)p; p += n*sizeof(i32);
    MemSetToVForNBytes(idxmap, -1, n*sizeof(i32));
    for (u32 i=0; i<mp.count; ++i) if (mp.entries[i].index != U16_MAX) idxmap[mp.entries[i].index] = (i32)i;
    RawOBJ* raw = (RawOBJ*)p; p += n*sizeof(RawOBJ);
    for (u32 i=0; i<n; ++i) {
        i32 pi = idxmap[i];
        if (pi >= 0) { FHandle d; int sz=0; raw[i].data = (const char*)OS_OpenAndAllocateFileBufferReadonly(mp.entries[pi].path,&d,&sz); raw[i].size = sz; }
    }

    float **pos = (float**)p; p += num_parse_threads*sizeof(float*);
    float **nrm = (float**)p; p += num_parse_threads*sizeof(float*);
    float **uv  = (float**)p; p += num_parse_threads*sizeof(float*);
    float **ov  = (float**)p; p += num_parse_threads*sizeof(float*);
    u16  **ot   =   (u16**)p; p += num_parse_threads*sizeof(u16*);
    size_t psz = MAX_VERT_ELEMENT_SIZE*3*sizeof(float), usz = MAX_OUTPUT_VERTS*8*sizeof(float), tsz = MAX_OUTPUT_VERTS*sizeof(u32);
    for (int i=0; i<num_parse_threads; ++i) {
        pos[i] = (float*)p; p += psz;
        nrm[i] = (float*)p; p += psz;
        uv[i]  = (float*)p; p += MAX_VERT_ELEMENT_SIZE*2*sizeof(float);
        ov[i]  = (float*)p; p += usz;
        ot[i]  = (u16*)p;   p += tsz;
    }

    thread_temp_pos = pos; thread_temp_nrm = nrm; thread_temp_uv = uv;
    thread_out_verts = ov; thread_out_tris = ot;
    ModelParseTask tasks[32];
    u32 chunk = (loadedModelsMaxIndex + num_parse_threads - 1) / num_parse_threads;
    for (int i=0;i<num_parse_threads;++i) tasks[i] = (ModelParseTask){i*chunk,(i+1)*chunk > loadedModelsMaxIndex ? loadedModelsMaxIndex : (i+1)*chunk,raw,i};
    OS_Thread th[32];
    if (num_parse_threads > 1) {
        for (int i=0;i<num_parse_threads;++i) OS_ThreadCreate(&th[i],ModelParsingWorker,&tasks[i]);
        for (int i=0;i<num_parse_threads;++i) OS_ThreadJoin(&th[i]);
    } else { for (int t=0;t<num_parse_threads;++t) ModelParsingWorker(&tasks[t]); } // Single threaded fallback
    
    glGenBuffers(loadedModelsMaxIndex,Sys_Render.vbos); glGenBuffers(loadedModelsMaxIndex,Sys_Render.tbos);
    u32 tv=0,tt=0;
    for (int i=0; i<loadedModelsMaxIndex; ++i) {
        if (!modelVertexCounts[i]) continue;

        tv += modelVertexCounts[i]; tt += modelTriangleCounts[i];
        UploadMdlBuffer(GL_ARRAY_BUFFER,Sys_Render.vbos[i],modelVertices[i],(size_t)modelVertexCounts[i] * VERTEX_ATTRIBUTES_SIZE);
        UploadMdlBuffer(GL_ELEMENT_ARRAY_BUFFER,Sys_Render.tbos[i],modelTriangles[i],(size_t)modelTriangleCounts[i] * 3 * sizeof(u16));
        if (raw[i].data) OS_DeallocateRAM((void*)raw[i].data,raw[i].size);
    }

    OS_DeallocateRAM(arena_base, arena);
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
    glFlush(); glFinish();
    OS_DeallocateRAM(mp.entries,mp.count * sizeof(ModelData));
    DualLog(" vertices: %u, tris: %u, %f secs\n",tv,tt,get_time() - startModelTime);
    DebugRAM("After LoadModels");
}
// ============== Culling System
#define MAX_CULL_FILESIZE 500000
typedef struct { u16 x,z; } PortalCell; typedef struct { PortalCell cellA,cellB,cellA2,cellB2; bool portalNS,open,dirty,isBulkhead;} Portal;
u32 gridCellStates[ARRSIZE],precomputedVisibleCellsFromHere[524288]; // 4096 * 4096 / 32
u16 playerCellIdx = 0u; bool instanceIsLODArray[INSTANCE_COUNT]; Portal activePortals[MAX_PORTALS]; static u8 numActivePortals = 0;
__attribute__((pure)) bool get_cull_bit(const u32* arr, int idx) { return (arr[idx >> 5] >> (idx & 31)) & 1; }
static inline __attribute__((always_inline)) void set_cull_bit(u32* arr, int idx, bool val) {u32* w = arr + (idx >> 5); u32 m = 1U << (idx & 31); *w = val ? (*w | m) : (*w & ~m);}
ENGINE_TO_MOD i32 PosGetCellCoords(float x, float z) { return (PosGetCellCoordZ(z) * WORLDX) + PosGetCellCoordX(x); }
ENGINE_TO_MOD bool PositionVisibleFromPlayerCell(float x, float z) { return (get_cull_bit(precomputedVisibleCellsFromHere,((playerCellIdx * ARRSIZE)/*cellIdx*/ + PosGetCellCoords(x,z)/*subIdx*/)/*flat_idx*/)); }
static inline __attribute__((always_inline)) bool XZPairInBounds(i32 x, i32 z) { return (x < WORLDX && z < WORLDZ && x >= 0 && z >= 0); }
bool SkyIsVisible() { return ((gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX) || Sys_Global.curLev == LEVEL_CYBERSPACE); }
bool SkySunIsVisible() { return ((gridCellStates[playerCellIdx] & CELL_SEES_SUN) && Sys_Global.curLev != LEVEL_CYBERSPACE); }
bool NeighborhoodInPVS(u16 cellX, u16 cellZ, u8 r) {
    u32 cellIdx = (cellZ * WORLDX) + cellX;
    for (int ix = (int)cellX-r; ix <= (int)cellX+r; ++ix) {
        for (int iz = (int)cellZ-r; iz <= (int)cellZ+r; ++iz) {
            if (unlikely(!XZPairInBounds(ix,iz))) continue;

            int subIdx = iz * WORLDX + ix;
            if (get_cull_bit(precomputedVisibleCellsFromHere, cellIdx * ARRSIZE + subIdx) && (gridCellStates[subIdx] & CELL_VISIBLE)) return true;
        }
    }
    return false;
}

bool GridCellBlock(u16 i,Vector3 pos,Vector3 newPos) {
    i32 ccx=PosGetCellCoordX(pos.x),    ccz=PosGetCellCoordZ(pos.z);
    i32 ncx=PosGetCellCoordX(newPos.x), ncz=PosGetCellCoordZ(newPos.z);
    i32 cc=(ccz*WORLDX)+ccx, nc=(ncz*WORLDX)+ncx;
    if (ncz>ccz && (gridCellStates[cc]&CELL_CLOSEDNORTH)) { Sys_Global.instances[i].velocity.z=-0.1f; return true; }
    if (ncz<ccz && (gridCellStates[cc]&CELL_CLOSEDSOUTH)) { Sys_Global.instances[i].velocity.z= 0.1f; return true; }
    if (ncx>ccx && (gridCellStates[cc]&CELL_CLOSEDEAST))  { Sys_Global.instances[i].velocity.x=-0.1f; return true; }
    if (ncx<ccx && (gridCellStates[cc]&CELL_CLOSEDWEST))  { Sys_Global.instances[i].velocity.x= 0.1f; return true; }
    if (!(gridCellStates[nc]&CELL_OPEN)) { Vector3 dir=V3_Normalize(Sys_Global.instances[i].velocity); Sys_Global.instances[i].velocity=V3_ScaleByF(dir,-0.1f); return true; }
    return false;
}

static unsigned char* LoadCullPNG(const char* name, int level) {
    char path[256]; StringFormat(path, sizeof(path),"./Data/%s_%d.png",name,level);
    FHandle fp = OS_OpenReadonly(path);
    OS_Seek(fp,0,2); size_t size = OS_Tell(fp);
    if (size > MAX_CULL_FILESIZE) { DualLogError("PNG too large: %s\n",path); OS_Exit(1); }
    
    u8* cullingFileBuffer = OS_Alloc(MAX_CULL_FILESIZE * sizeof(u8));
    OS_Seek(fp,0,0); size_t read_size = OS_Read(fp,cullingFileBuffer,size); OS_Close(fp);
    if (read_size != size) { DualLogError("Failed to read %s\n",path); OS_Exit(1); }
    
    int w, h; unsigned char* pixels = PngLoad(cullingFileBuffer,size,&w,&h,&png_arena_main);
    if (!pixels) { DualLogError("STB failed: %s\n",path); OS_Exit(1); }
    OS_DeallocateRAM(cullingFileBuffer,MAX_CULL_FILESIZE * sizeof(u8));
    return pixels;
}

#define PIXEL_IDX(x, z) ((x) + ((WORLDZ - 1 - (z)) * WORLDX)) * 4 // 4 channels, flip z to have desired bottom-left origin 0,0 vs png's top-left
void DetermineClosedEdges() {
    PngArenaInit(&png_arena_main); u16 totalOpenCells = 0;
    unsigned char* openPixels = LoadCullPNG("worldcellopen",Sys_Global.curLev);
    for (i32 x=0;x<WORLDX;++x) {
        for (i32 z=0;z<WORLDZ;++z) {
            i32 cellIdx = (z * WORLDX) + x;
            gridCellStates[cellIdx] &= ~CELL_OPEN;
            i32 pixelIdx = PIXEL_IDX(x,z);
            unsigned char or = openPixels[pixelIdx + 0], og = openPixels[pixelIdx + 1], ob = openPixels[pixelIdx + 2];
            if (or > 0 || og > 0 || ob > 0) { gridCellStates[cellIdx] |= CELL_OPEN; totalOpenCells++; }
            else gridCellStates[cellIdx] |= CELL_CLOSEDNORTH|CELL_CLOSEDEAST|CELL_CLOSEDSOUTH|CELL_CLOSEDWEST; // Also force close the edges for closed cells even if above edges image said tweren't closed edges.
        }
    }

    gridCellStates[0] |= CELL_OPEN; // Force the fallback error cell to be open (forced visible later, open is static, visible is transient)
    unsigned char* edgePixels = LoadCullPNG("worldedgesclosed",Sys_Global.curLev);
    for (i32 x=0;x<WORLDX;x++) {
        for (i32 z=0;z<WORLDZ;z++) {
            i32 cellIdx = (z * WORLDX) + x;
            gridCellStates[cellIdx] &= ~(CELL_CLOSEDNORTH|CELL_CLOSEDEAST|CELL_CLOSEDSOUTH|CELL_CLOSEDWEST); // Mark all edges not closed
            i32 pixelIdx = PIXEL_IDX(x,z);
            unsigned char cr = edgePixels[pixelIdx + 0], cg = edgePixels[pixelIdx + 1], cb = edgePixels[pixelIdx + 2], ca = edgePixels[pixelIdx + 3];
            if (cr > 127) gridCellStates[cellIdx] |= CELL_CLOSEDNORTH;
            if (cg > 127) gridCellStates[cellIdx] |= CELL_CLOSEDEAST;
            if (cb > 127) gridCellStates[cellIdx] |= CELL_CLOSEDSOUTH;
            if ((cr < 255 && cr > 0) || (cg < 255 && cg > 0) || (cb < 255 && cb > 0)) gridCellStates[cellIdx] |= CELL_CLOSEDWEST; // Anything that has closed west edge will be not at full 255 on at least one channel. Typical for all other edge conditions is to use full brightness 255 on the channel(s). All 4 closed would be 128 128 128 but this doesn't ever happen. None closed is 0 0 0
            if (ca > 0 && ca < 255) gridCellStates[cellIdx] |= CELL_CLOSEDNORTH|CELL_CLOSEDEAST|CELL_CLOSEDSOUTH|CELL_CLOSEDWEST;
        }
    }
        
    unsigned char* skyPixels = LoadCullPNG("worldcellskyvis",Sys_Global.curLev);
    for (i32 x=0;x<WORLDX;++x) {
        for (i32 z=0;z<WORLDZ;++z) {
            i32 cellIdx = (z * WORLDX) + x; i32 pixelIdx = PIXEL_IDX(x,z);
            unsigned char sr = skyPixels[pixelIdx + 0], sg = skyPixels[pixelIdx + 1], sb = skyPixels[pixelIdx + 2];
            if (sr > 127 && sg < 127 && sb < 127) gridCellStates[cellIdx] &= ~(CELL_SEES_SUN|CELL_SEES_SKYBOX); // All red cells marked as -1, no sky or sun.
            else if (sr <= 127 && sg <= 127 && sb > 127) gridCellStates[cellIdx] |= CELL_SEES_SUN|CELL_SEES_SKYBOX; // All blue cells marked as sky visible.  Sun + Sky.
            else { gridCellStates[cellIdx] &= ~CELL_SEES_SKYBOX; gridCellStates[cellIdx] |= CELL_SEES_SUN; } // All white and black cells marked as 0.  Only sees Sun.
        }
    }
    
    OS_DeallocateRAM(png_arena_main.base, PNG_ARENA_SIZE); png_arena_main.base = NULL;
    DualLog("found %d open cells...",totalOpenCells);
}

ENGINE_TO_MOD void AddDoorPortal(u16 entIdx, u16 parent) {
    if (entIdx == 499 || entIdx == 509) return; // Don't add bulkheads
    float nudgeAmount = 0.32f;
    Entity* door = &Sys_Global.instances[parent];
    door->portalIndex = numActivePortals;
    bool isOpen = (door->doorState != DoorState_Closed); // Allows for any of DoorState_Open, DoorState_Opening, or DoorState_Closing to be considered open as far as portals are concerned so we can draw objects between the door panels.
    float obj_x = door->position.x; float obj_z = door->position.z;
    u16 cellIndexCurrentX = PosGetCellCoordX(obj_x); u16 cellIndexCurrentZ = PosGetCellCoordZ(obj_z);
    u16 cellCurrent = (cellIndexCurrentZ * WORLDX) + cellIndexCurrentX;
    u16    cellIndexUp = PosGetCellCoordZ(obj_z + nudgeAmount), cellIndexDn = PosGetCellCoordZ(obj_z - nudgeAmount);
    u16 cellIndexRight = PosGetCellCoordX(obj_x + nudgeAmount), cellIndexLeft = PosGetCellCoordX(obj_x - nudgeAmount);
    u16 cellN_idx = PosGetCellCoords(obj_x, obj_z + nudgeAmount), cellS_idx = PosGetCellCoords(obj_x, obj_z - nudgeAmount);
    u16 cellE_idx = PosGetCellCoords(obj_x + nudgeAmount, obj_z), cellW_idx = PosGetCellCoords(obj_x - nudgeAmount, obj_z);
    bool isNS = (cellN_idx != cellCurrent || cellS_idx != cellCurrent);
    if (isNS) { // Portal is a North     /\
                //             South pair\/
        PortalCell cellN, cellS;
        cellN.x = cellS.x = PosGetCellCoordX(obj_x);
        cellN.z = (cellN_idx != cellCurrent) ? cellIndexUp : cellIndexCurrentZ; // Ensure that cellA is always the north cell of the pair
        cellS.z = (cellS_idx != cellCurrent) ? cellIndexDn : cellIndexCurrentZ;
        activePortals[numActivePortals] = (Portal){ .cellA=cellN, .cellB=cellS, .portalNS=true, .open=isOpen, .dirty=true };
    } else { // Portal is an East<>West pair
        PortalCell cellE, cellW;
        cellE.z = cellW.z = PosGetCellCoordZ(obj_z);
        cellE.x = (cellE_idx != cellCurrent) ? cellIndexRight : cellIndexCurrentX; // Ensure that cellA is always the east cell of the pair
        cellW.x = (cellW_idx != cellCurrent) ? cellIndexLeft : cellIndexCurrentX;
        activePortals[numActivePortals] = (Portal){ .cellA=cellE, .cellB=cellW, .portalNS=false, .open=isOpen, .dirty=true };
    }
    
    numActivePortals++;
}

ENGINE_TO_MOD bool ToggleDoorPortal(u8 portalIdx, u16 doorIdx, u16 closedModelIndex) {
    if (portalIdx >= MAX_PORTALS) return false;

    Portal* prt = &activePortals[portalIdx]; bool currentState = prt->open; u16 mdx = Sys_Global.instances[doorIdx].modelIndex;
    if (mdx == closedModelIndex &&  currentState) { prt->open = false; prt->dirty = true; } else if (mdx != closedModelIndex && !currentState) { prt->open = true; prt->dirty = true; }
    return true;
}

i32 CastRayCellCheck(i32 x, i32 z, i32 lastX, i32 lastZ) {
    if (lastX != x || lastZ != z) {
        if (XZPairInBounds(lastX, lastZ)) { 
            i32 li = (lastZ * WORLDX) + lastX;
            u32 cell = gridCellStates[li];
            i32 dx = x - lastX, dz = z - lastZ; // -1, 0, or 1 each
            if (dz == 0) { // Pure horizontal
                if (((dx >  0) && (cell & CELL_CLOSEDEAST)) || ((dx < 0)  && (cell & CELL_CLOSEDWEST)))  return -1;
            } else if (dx == 0) { // Pure vertical
                if (((dz > 0)  && (cell & CELL_CLOSEDNORTH)) || ((dz < 0)  && (cell & CELL_CLOSEDSOUTH))) return -1;
            } else { // Diagonal — check cell + two axis-adjacent neighbors
                u32 cf_ew  = (dx > 0) ? CELL_CLOSEDEAST  : CELL_CLOSEDWEST; // Which closed-edge flags to test depends on direction quadrant
                u32 cf_ns  = (dz > 0) ? CELL_CLOSEDNORTH : CELL_CLOSEDSOUTH;
                bool c_ew = (cell & cf_ew) != 0, c_ns = (cell & cf_ns) != 0;
                u32 nf_ew  = (dx > 0) ? CELL_CLOSEDEAST  : CELL_CLOSEDWEST;   // Neighbor sees the opposite face, same face on neighbor in NS direction
                u32 nf_ns  = (dz > 0) ? CELL_CLOSEDNORTH : CELL_CLOSEDSOUTH;  // same face on neighbor in EW direction
                bool n_ew = false, n_ns = false;
                i32 ni_ns_coord_x = lastX, ni_ns_coord_z = lastZ + dz;
                if (XZPairInBounds(ni_ns_coord_x, ni_ns_coord_z)) { u32 nsN = gridCellStates[ni_ns_coord_z * WORLDX + ni_ns_coord_x]; n_ew = (nsN & nf_ew) != 0 && (nsN & CELL_OPEN); }
                i32 ni_ew_coord_x = lastX + dx, ni_ew_coord_z = lastZ;
                if (XZPairInBounds(ni_ew_coord_x, ni_ew_coord_z)) { u32 ewN = gridCellStates[ni_ew_coord_z * WORLDX + ni_ew_coord_x]; n_ns = (ewN & nf_ns) != 0 && (ewN & CELL_OPEN); }
                if ((c_ns && c_ew) || (c_ew && n_ew) || (c_ns && n_ns) || (n_ew && n_ns)) return -1;
            }
        }
    }
 
    if (!XZPairInBounds(x,z)) return 0;

    i32 ci = (z * WORLDX) + x;
    if (gridCellStates[ci] & CELL_OPEN) {gridCellStates[ci] |=  CELL_VISIBLE;} else {gridCellStates[ci] &= ~CELL_VISIBLE;}
    return (gridCellStates[ci] & CELL_VISIBLE) ? 1 : -1;
}

i32 CastStraightZ(i32 px, i32 pz, i32 signz) {
    if (signz > 0 && pz >= (WORLDZ - 1)) return pz; // Nowwhere to step to if right by edge, hence WORLDX - 1 here.
    if (signz < 0 && pz <= 0) return pz;
    if (!XZPairInBounds(px,pz)) return pz;
    i32 cellIdx = (pz * WORLDX) + px; if (!(gridCellStates[cellIdx] & CELL_VISIBLE)) return pz;
    
    bool currentVisible = true; i32 x=px, z=pz+signz, zabs=vabs(z);
    for (;zabs<WORLDX;z+=signz) { // Up/Down
        currentVisible = false;
        i32 cellIdx_x_zmnus1 = ((z - 1) * WORLDX) + x, cellIdx_x_zplus1 = ((z + 1) * WORLDX) + x;
        if (XZPairInBounds(x,z - signz) && XZPairInBounds(x,z)) {
            i32 cellIdx_x_zmnus_sign = ((z - signz) * WORLDX) + x;
            if (gridCellStates[cellIdx_x_zmnus_sign] & CELL_VISIBLE) {
                     if (signz > 0 && gridCellStates[cellIdx_x_zmnus1] & CELL_CLOSEDNORTH && gridCellStates[cellIdx_x_zmnus1] & CELL_OPEN) return z;
                else if (signz < 0 && gridCellStates[cellIdx_x_zplus1] & CELL_CLOSEDSOUTH && gridCellStates[cellIdx_x_zplus1] & CELL_OPEN) return z;

                i32 subCellIdx = (z * WORLDX) + x;
                if (gridCellStates[subCellIdx] & CELL_OPEN) gridCellStates[subCellIdx] |= CELL_VISIBLE;
                else gridCellStates[subCellIdx] &= ~CELL_VISIBLE;
                
                currentVisible = true; // Would be if twas open.
            }
        }

        if (!currentVisible) break; // Hit wall!

        if (XZPairInBounds(x + 1,z)) {
            i32 cellIdx_xplus1_z = (z * WORLDX) + x + 1;
            if (CastRayCellCheck(x,z,x + 1,z) > 0) {
                if (gridCellStates[cellIdx_xplus1_z] & CELL_OPEN) gridCellStates[cellIdx_xplus1_z] |= CELL_VISIBLE;
                else gridCellStates[cellIdx_xplus1_z] &= ~CELL_VISIBLE;
            } else gridCellStates[cellIdx_xplus1_z] &= ~CELL_VISIBLE;
        }
        
        if (XZPairInBounds(x - 1,z)) {
            i32 cellIdx_xmnus1_z = (z * WORLDX) + x - 1;
            if (CastRayCellCheck(x,z,x - 1,z) > 0) {
                if (gridCellStates[cellIdx_xmnus1_z] & CELL_OPEN) gridCellStates[cellIdx_xmnus1_z] |= CELL_VISIBLE;
                else gridCellStates[cellIdx_xmnus1_z] &= ~CELL_VISIBLE;
            } else gridCellStates[cellIdx_xmnus1_z] &= ~CELL_VISIBLE;
        }
    }
    
    return WORLDX * signz;
}

i32 CastStraightX(i32 px, i32 pz, i32 signx) {
    if (signx > 0 && px >= (WORLDX - 1)) return px; // Nowwhere to step to if right by edge, hence WORLDX - 1 here.
    if (signx < 0 && px <= 0) return px;
    if (!XZPairInBounds(px,pz)) return px;
    if (!(gridCellStates[(pz * WORLDX) + px] & CELL_VISIBLE)) return px;

    i32 x = px + signx;
    i32 z = pz;
    bool currentVisible = true;
    i32 xabs = vabs(x);
    for (;xabs<WORLDX;x+=signx) { // Right/Left
        currentVisible = false;
        if (XZPairInBounds(x - signx,z) && XZPairInBounds(x,z)) {
            i32 cellIdx_xmnussign_z = (z * WORLDX) + x - signx;
            if (gridCellStates[cellIdx_xmnussign_z] & CELL_VISIBLE) {
                     if (signx > 0 && (gridCellStates[(z * WORLDX) + x - 1] & CELL_CLOSEDEAST) && gridCellStates[(z * WORLDX) + x - 1] & CELL_OPEN) return x;
                else if (signx < 0 && (gridCellStates[(z * WORLDX) + x + 1] & CELL_CLOSEDWEST) && gridCellStates[(z * WORLDX) + x + 1] & CELL_OPEN) return x;

                i32 subCellIdx = (z * WORLDX) + x;
                if (gridCellStates[subCellIdx] & CELL_OPEN) gridCellStates[subCellIdx] |= CELL_VISIBLE;
                else gridCellStates[subCellIdx] &= ~CELL_VISIBLE;
                
                currentVisible = true; // Would be if twas open.
            }
        }

        if (!currentVisible) break; // Hit wall!
        
        if (XZPairInBounds(x,z + 1)) {
            i32 cellIdx_x_zplus1 = ((z + 1) * WORLDX) + x;
            if (CastRayCellCheck(x,z,x,z + 1) > 0) {
                if (gridCellStates[cellIdx_x_zplus1] & CELL_OPEN) gridCellStates[cellIdx_x_zplus1] |= CELL_VISIBLE;
                else gridCellStates[cellIdx_x_zplus1] &= ~CELL_VISIBLE;
            } else gridCellStates[cellIdx_x_zplus1] &= ~CELL_VISIBLE;
        }
        
        if (XZPairInBounds(x,z - 1)) {
            i32 cellIdx_x_zmnus1 = ((z - 1) * WORLDX) + x;
            if (CastRayCellCheck(x,z,x,z - 1) > 0) {
                if (gridCellStates[cellIdx_x_zmnus1] & CELL_OPEN) gridCellStates[cellIdx_x_zmnus1] |= CELL_VISIBLE;
                else gridCellStates[cellIdx_x_zmnus1] &= ~CELL_VISIBLE;
            } else gridCellStates[cellIdx_x_zmnus1] &= ~CELL_VISIBLE;
        }
    }
    
    return WORLDX * signx;
}

void CastRay(i32 x0, i32 z0, i32 x1, i32 z1) {
    i32 dx = vabs(x1 - x0);      i32 dz = vabs(z1 - z0);
    i32 sx = (x0 < x1) ? 1 : -1; i32 sz = (z0 < z1) ? 1 : -1;
    i32 x = x0;                  i32 z = z0;
    i32 lastX = x;               i32 lastZ = z;
    i32 err = dx - dz;
    i32 iter = dx > dz ? dx : dz;
    while (iter >= 0) {
        if (!XZPairInBounds(x,z) || !XZPairInBounds(lastX,lastZ)) { --iter; continue; }
        if (CastRayCellCheck(x,z,lastX,lastZ) == -1) return;

        lastX = x; lastZ = z;
        i32 e2 = 2 * err;
        if (e2 > -dz) { err -= dz; x += sx; }
        if (e2 <  dx) { err += dx; z += sz; }
        --iter;
    }
}

void CircleFanRays(i32 x0, i32 z0) { // CastRay()'s in fan from x0,z0 out to every cell around map perimeter.
    if (!XZPairInBounds(x0,z0)) return;
    if (!(gridCellStates[(z0 * WORLDX) + x0] & CELL_VISIBLE)) return;

    i32 x,z,max=WORLDX,min=0; // Reduce work slightly by not casting towards edges but 1 less = [1,63].
    for (x=min;x<max;x++) CastRay(x0,z0,x,min);
    for (x=min;x<max;x++) CastRay(x0,z0,x,max);
    for (z=min;z<max;z++) CastRay(x0,z0,min,z);
    for (z=min;z<max;z++) CastRay(x0,z0,max,z);
}

static inline void MarchAxis(i32 start, i32 end, i32 step, i32 ox, i32 oz, i32 sign, bool isX) {
    for (i32 m = start; step > 0 ? m < end : m >= end; m += step) { 
        i32 x = isX ? m : ox, z = isX ? oz : m;
        if (XZPairInBounds(x,z) && (gridCellStates[(z * WORLDX) + x] & CELL_VISIBLE)) m = isX ? CastStraightX(m,oz,sign) : CastStraightZ(ox,m,sign);
    }
}

void DetermineVisibleCells(i32 startX, i32 startZ) {
    if (!XZPairInBounds(startX,startZ)) return;

    for (i32 x=0;x<WORLDX;x++) {
        for (i32 z=0;z<WORLDZ;z++) { i32 subCellIdx = (z * WORLDX) + x; gridCellStates[subCellIdx] &= ~CELL_VISIBLE; } // Clear all to not visible.
    }

    gridCellStates[(startZ * WORLDX) + startX] |= CELL_VISIBLE; // Force starting player cell to visible.
    CastStraightX(startX,startZ,1); // Cast to the right (East) [ ][3]
    MarchAxis(startX,WORLDX - 1,1,0,startZ + 1,1,1);         // [1][2]
    MarchAxis(startX,WORLDX - 1,1,0,startZ - 1,1,1);         // [ ][3]
    CastStraightX(startX,startZ,-1); // Cast to the left (West) [3][ ]
    MarchAxis(startX,1,-1,0,startZ + 1,-1,1);                // [2][1]
    MarchAxis(startX,1,-1,0,startZ - 1,-1,1);                // [3][ ] 
    CastStraightZ(startX,startZ,-1); // Cast down (South) [ ][1][ ]
    MarchAxis(startZ,1,-1,startX + 1,0,-1,0);          // [3][2][3]
    MarchAxis(startZ,1,-1,startX - 1,0,-1,0);
    CastStraightZ(startX,startZ,1);  // Cast   up (North) [3][2][3]
    MarchAxis(startZ,WORLDX - 1,1,startX + 1,0,1,0);   // [ ][1][ ]
    MarchAxis(startZ,WORLDX - 1,1,startX - 1,0,1,0);
    CircleFanRays(startX,startZ);
    CircleFanRays(startX + 1,startZ);
    CircleFanRays(startX + 1,startZ + 1);
    CircleFanRays(startX,startZ + 1);
    CircleFanRays(startX - 1,startZ + 1);
    CircleFanRays(startX - 1,startZ);
    CircleFanRays(startX - 1,startZ - 1);
    CircleFanRays(startX,startZ - 1);
    CircleFanRays(startX + 1,startZ - 1);
    for (i32 x=0;x<WORLDX;++x) {
        for (i32 z=0;z<WORLDZ;++z) {
            i32 cellIdx_xz = (z * WORLDX) + x;
            if (Sys_Global.curLev == 5) { // Citadel flight level hackarounds for algorithm discrepancies at glancing angles.
                if ((x <= 15 && startX <= 15) || (z <= 9 && startZ <= 9) || (x >= 32 && startX >= 32) || (z == 31 && startZ == 31 && x >= 27 && startX >= 27) ||  x >= 34) gridCellStates[cellIdx_xz] |= CELL_VISIBLE;                
                if (startX <=12 && x == 14 && z == 31 && startZ >= 24) gridCellStates[cellIdx_xz] |= CELL_VISIBLE;
                if (startX <=12 && x == 14 && z == 30 && startZ >= 24) gridCellStates[cellIdx_xz] |= CELL_VISIBLE;
                if (startX <=12 && x == 13 && z == 30 && startZ >= 24) gridCellStates[cellIdx_xz] |= CELL_VISIBLE;
            }
        }
    }
}

void UploadGridCellVisibility() { glBindBuffer(GL_SSBO,Sys_Render.cellVisibleDataID); glBufferData(GL_SSBO,ARRSIZE * sizeof(u32),gridCellStates,GL_DYNAMIC_DRAW); }
ENGINE_TO_MOD void PortalCulling() { // Called just once at end of animation loop for the frame after each frame perfect change to door models becoming either closed or not closed.
    u16 playerCellX = PosGetCellCoordX(Sys_Global.instances[PLAYER1].position.x);
    u16 playerCellZ = PosGetCellCoordZ(Sys_Global.instances[PLAYER1].position.z);
    bool previousLightVisible[LIGHT_COUNT];
    MemSetToVForNBytes(previousLightVisible,false,LIGHT_COUNT * sizeof(bool));
    for (u16 i=0;i<loadedLights;++i) {
        u16 lcell = (lights[i].pos.z * WORLDX) + lights[i].pos.x;
        if (gridCellStates[lcell] & CELL_VISIBLE) previousLightVisible[i] = true;
    }
    
    PortalCell cellA, cellB;
    for (u8 portalIdx=0;portalIdx<MAX_PORTALS;++portalIdx) {
        Portal* prt = &activePortals[portalIdx];
        if (!prt->dirty) continue;
        
        prt->dirty = false;
        cellA = prt->cellA; cellB = prt->cellB; // Guaranteed order at level load.  A = N or E, B = S or W
        bool isNS = prt->portalNS;
        u16 cellIdxA = (cellA.z * WORLDX) + cellA.x;
        u16 cellIdxB = (cellB.z * WORLDX) + cellB.x;
        if (prt->open) { // Open the edges up
            if (isNS) { gridCellStates[cellIdxA] &= ~(CELL_CLOSEDSOUTH); gridCellStates[cellIdxB] &= ~(CELL_CLOSEDNORTH); }
            else { gridCellStates[cellIdxA] &= ~(CELL_CLOSEDWEST); gridCellStates[cellIdxB] &= ~(CELL_CLOSEDEAST); }
        } else {
            if (isNS) { gridCellStates[cellIdxA] |= CELL_CLOSEDSOUTH; gridCellStates[cellIdxB] |= CELL_CLOSEDNORTH; }
            else { gridCellStates[cellIdxA] |= CELL_CLOSEDWEST; gridCellStates[cellIdxB] |= CELL_CLOSEDEAST; }
        }
    }
    
    DetermineVisibleCells(playerCellX,playerCellZ); // Recompute full PVS with new closed edges for all portal states.  So much for the precomputed set.
    for (u16 i=0;i<loadedLights;++i) {
        u16 lcell = (lights[i].pos.z * WORLDX) + lights[i].pos.x;
        if (!previousLightVisible[i] && (gridCellStates[lcell] & CELL_VISIBLE)) flag_set(&lights[i].lflags,LDIRTY,true);
    }
    UploadGridCellVisibility();
}

void CullCore() {
    if (unlikely(Sys_Global.gamePaused || Sys_Global.menuActive)) return;
        
    playerCellIdx = PosGetCellCoords(Sys_Global.instances[PLAYER1].position.x,Sys_Global.instances[PLAYER1].position.z);
    if (Sys_Global.curLev >= LEVEL_CYBERSPACE) return;

    u16 cellX = PosGetCellCoordX(Sys_Global.instances[PLAYER1].position.x), cellZ = PosGetCellCoordZ(Sys_Global.instances[PLAYER1].position.z);
    float pos_x = Sys_Global.worldMin_x[Sys_Global.curLev] + (cellX * CELL_SIZE), pos_z = Sys_Global.worldMin_z[Sys_Global.curLev] + (cellZ * CELL_SIZE);
    for (int i=0;i<Sys_Global.loadedInstances;++i) {
        float distSqrd = squareDistance2D(Sys_Global.instances[i].position.x,Sys_Global.instances[i].position.z,pos_x,pos_z);
        instanceIsLODArray[i] = (distSqrd >= 655.36f); // 25.6f * 25.6f
    }
    
    PortalCulling(); // Update based on portal states.
}

void CullInit() {
    double start_time = get_time();    
    DualLog("Culling ");
    if (Sys_Global.curLev == LEVEL_CYBERSPACE) return;

    DetermineClosedEdges(); // For each cell, get the visibility as though player were there and put into gridCellStates.  Then store the visibility of gridCellStates into the table of all visible cells for that cell at the appropriate offset for looking up later when actually re-assigning gridCellStates from this precalculated visibility state for the particular cell.
    for (i32 z=0;z<WORLDZ;z++) {
        for (i32 x=0;x<WORLDX;x++) {
            DetermineVisibleCells(x,z);
            i32 cellIdx = (z * WORLDX) + x;
            for (i32 z2=0;z2<WORLDZ;z2++) {
                for (i32 x2=0;x2<WORLDX;x2++) {
                    i32 subCellIdx = (z2 * WORLDX) + x2;
                    size_t flat_idx = (size_t)(cellIdx * ARRSIZE) + subCellIdx;
                    bool is_visible = (gridCellStates[subCellIdx] & CELL_VISIBLE);
                    set_cull_bit(precomputedVisibleCellsFromHere,flat_idx,is_visible);
                }
            }
            
            if (Sys_Global.curLev == 10) {
                if ((x == 15 || x == 16) && z == 23) { // Fix up problem cells at odd angle where ddx doesn't work.
                    size_t flat_idx = (size_t)(cellIdx * ARRSIZE) + ((11 * WORLDX) + 12);
                    set_cull_bit(precomputedVisibleCellsFromHere,flat_idx,true);
                }
            }
        }
    }
    
    playerCellIdx = PosGetCellCoords(Sys_Global.instances[PLAYER1].position.x,Sys_Global.instances[PLAYER1].position.z);
    i32 cellToCellIdx = playerCellIdx * ARRSIZE;
    for (i32 z=0;z<WORLDZ;++z) {
        for (i32 x=0;x<WORLDX;++x) {
            i32 cellIdx = (z * WORLDX) + x;
            size_t flat_idx = (size_t)(cellToCellIdx + cellIdx);
            if (get_cull_bit(precomputedVisibleCellsFromHere,flat_idx)) gridCellStates[cellIdx] |= CELL_VISIBLE; // Get visible before putting meshes into their cells so we can nudge them a little.
        }
    }

    gridCellStates[0] |= CELL_VISIBLE; // Errors default here so draw them anyways.
    CullCore(); // Do first Cull pass, forcing as player moved to new cell.
    DualLog(" took %f secs\n",get_time() - start_time);
}
// ===================== Fonts and Text System
typedef struct { void* ptr; size_t sz; } TAlloc; static TAlloc* ttAllocs = NULL; static int tallocCount=0;
static void* TempAlloc(size_t n){if(tallocCount>=4474){DualLogError("TempAlloc too many!\n");return NULL;}void*p=OS_Alloc(n);if(!p){DualLogError("TempAlloc: OS_Alloc failed!\n");return NULL;}ttAllocs[tallocCount++]=(TAlloc){p,n};return p;}
static void  TempFree (void* p){if(!p||tallocCount==0||ttAllocs[tallocCount-1].ptr!=p)return;OS_DeallocateRAM(p,ttAllocs[tallocCount-1].sz);tallocCount--;}
static u16 ttUSHORT(u8*p){return p[0]*256+p[1];} 
static i16 ttSHORT (u8*p){return p[0]*256+p[1];}
static u32 ttULONG (u8*p){return((u32)p[0]<<24)|((u32)p[1]<<16)|((u32)p[2]<<8)|p[3];}
static i32 ttLONG  (u8*p){return((i32)p[0]<<24)|((i32)p[1]<<16)|((i32)p[2]<<8)|p[3];}
#define stbtt_tag4(p,a,b,c,d) ((p)[0]==(a)&&(p)[1]==(b)&&(p)[2]==(c)&&(p)[3]==(d))
#define stbtt_tag(p,s) stbtt_tag4(p,s[0],s[1],s[2],s[3])
typedef struct { unsigned char*data; int cursor,size; } stbtt__buf;
static stbtt__buf stbtt__new_buf(const void*p,size_t s){stbtt__buf r;r.data=(u8*)p;r.size=(int)s;r.cursor=0;return r;}
static u8  _bg8(stbtt__buf*b){return b->cursor>=b->size?0:b->data[b->cursor++];}
static u8  _bp8(stbtt__buf*b){return b->cursor>=b->size?0:b->data[b->cursor];}
static void _bsk(stbtt__buf*b,int o) { b->cursor = (o>b->size||o<0) ? b->size : o; }
static void _bskip(stbtt__buf*b,int o){_bsk(b,b->cursor+o);}
static u32 _bg(stbtt__buf*b,int n){u32 v=0;for(int i=0;i<n;i++)v=(v<<8)|_bg8(b);return v;}
static stbtt__buf _brange(const stbtt__buf*b,int o,int s){stbtt__buf r=stbtt__new_buf(NULL,0);if(o<0||s<0||o>b->size||s>b->size-o)return r;r.data=b->data+o;r.size=s;return r;}
static stbtt__buf _cff_idx(stbtt__buf*b){int c=b->cursor,n=_bg(b,2);if(n){int os=_bg8(b);_bskip(b,os*n);_bskip(b,_bg(b,os)-1);}return _brange(b,c,b->cursor-c);}
static u32 _cff_int(stbtt__buf*b){int b0=_bg8(b);if(b0>=32&&b0<=246)return b0-139;if(b0>=247&&b0<=250)return(b0-247)*256+_bg8(b)+108;if(b0>=251&&b0<=254)return-(b0-251)*256-_bg8(b)-108;if(b0==28)return _bg(b,2);if(b0==29)return _bg(b,4);return 0;}
static void _cff_skip_op(stbtt__buf*b){if(_bp8(b)==30){_bskip(b,1);while(b->cursor<b->size){int v=_bg8(b);if((v&0xF)==0xF||(v>>4)==0xF)break;}}else _cff_int(b);}
static stbtt__buf _dict_get(stbtt__buf*b, int key) {
    b->cursor = (0 > b->size) ? b->size : 0;
    while(b->cursor < b->size) {
        int e,op,s=b->cursor;
        while(_bp8(b) >= 28) _cff_skip_op(b);
        e = b->cursor; op = _bg8(b);
        if(op==12) op = _bg8(b) | 0x100;
        if(op==key) return _brange(b,s,e-s);
    } return _brange(b,0,0);
}

static void _dict_ints(stbtt__buf*b,int key,int n,u32*out) { stbtt__buf op = _dict_get(b,key); for (int i=0;i<n && op.cursor<op.size;++i) {out[i] = (u32)_cff_int(&op);} }
static stbtt__buf _cff_idx_get(stbtt__buf b,int i){_bsk(&b,0);int n=_bg(&b,2),os=_bg8(&b);_bskip(&b,i*os);int s=_bg(&b,os),e=_bg(&b,os);return _brange(&b,2+(n+1)*os+s,e-s);}
enum{STBTT_vmove=1,STBTT_vline,STBTT_vcurve,STBTT_vcubic};
typedef struct{i16 x,y,cx,cy,cx1,cy1;unsigned char type,padding;}stbtt_vertex;
typedef struct{void*userdata;unsigned char*data;int fontstart,numGlyphs,loca,head,glyf,hhea,hmtx,index_map,indexToLocFormat;stbtt__buf cff,charstrings,gsubrs,subrs,fontdicts,fdselect;}stbtt_fontinfo;
static u32 _find_table(u8*d,u32 fs,const char*tag){i32 n=ttUSHORT(d+fs+4);u32 td=fs+12;for(i32 i=0;i<n;++i){u32 l=td+16*i;if(stbtt_tag(d+l+0,tag))return ttULONG(d+l+8);}return 0;}
static stbtt__buf _get_subrs(stbtt__buf cff,stbtt__buf fd){u32 so=0,pl[2]={0,0};_dict_ints(&fd,18,2,pl);if(!pl[1]||!pl[0])return stbtt__new_buf(NULL,0);stbtt__buf pd=_brange(&cff,pl[1],pl[0]);_dict_ints(&pd,19,1,&so);if(!so)return stbtt__new_buf(NULL,0);_bsk(&cff,pl[1]+so);return _cff_idx(&cff);}
static int stbtt_InitFont_internal(stbtt_fontinfo* info, unsigned char* data, int fs) {
    u32 cmap,t,i,nt;info->data=data;info->fontstart=fs;info->cff=stbtt__new_buf(NULL,0);
    cmap=_find_table(data,fs,"cmap"); info->loca=_find_table(data,fs,"loca"); info->head=_find_table(data,fs,"head");
    info->glyf=_find_table(data,fs,"glyf"); info->hhea=_find_table(data,fs,"hhea"); info->hmtx=_find_table(data,fs,"hmtx");
    if(!cmap || !info->head || !info->hhea || !info->hmtx) return 0;
    if(info->glyf){ if(!info->loca)return 0; }
    else{
        u32 cs=2,chstr=0,fda=0,fds=0,cff=_find_table(data,fs,"CFF ");if(!cff)return 0;
        info->fontdicts=stbtt__new_buf(NULL,0);info->fdselect=stbtt__new_buf(NULL,0);
        info->cff=stbtt__new_buf(data+cff,16*1024*1024);stbtt__buf b=info->cff;
        _bskip(&b,2);_bsk(&b,_bg8(&b));_cff_idx(&b);
        stbtt__buf tdi=_cff_idx(&b),td=_cff_idx_get(tdi,0);_cff_idx(&b);info->gsubrs=_cff_idx(&b);
        _dict_ints(&td,17,1,&chstr);_dict_ints(&td,0x100|6,1,&cs);_dict_ints(&td,0x100|36,1,&fda);_dict_ints(&td,0x100|37,1,&fds);
        info->subrs=_get_subrs(b,td);
        if (cs!=2||chstr==0) return 0;
        if (fda) { if(!fds) {return 0;} _bsk(&b,fda);info->fontdicts=_cff_idx(&b);info->fdselect=_brange(&b,fds,b.size-fds); }
        _bsk(&b,chstr);info->charstrings=_cff_idx(&b);
    }
    
    t=_find_table(data,fs,"maxp");info->numGlyphs=t?ttUSHORT(data+t+4):0xffff;
    nt=ttUSHORT(data+cmap+2);info->index_map=0;
    for(i=0;i<nt;++i){u32 er=cmap+4+8*i;switch(ttUSHORT(data+er)){case 3:switch(ttUSHORT(data+er+2)){case 1:case 10:info->index_map=cmap+ttULONG(data+er+4);}break;case 0:info->index_map=cmap+ttULONG(data+er+4);break;}}
    if(!info->index_map)return 0;
    info->indexToLocFormat=ttUSHORT(data+info->head+50);return 1;
}

static int _font_offset(unsigned char*d,int idx){
    if(stbtt_tag4(d,'1',0,0,0)||stbtt_tag(d,"typ1")||stbtt_tag(d,"OTTO")||stbtt_tag4(d,0,1,0,0)||stbtt_tag(d,"true"))return idx==0?0:-1;
    if(stbtt_tag(d,"ttcf")&&(ttULONG(d+4)==0x00010000||ttULONG(d+4)==0x00020000)){i32 n=ttLONG(d+8);if(idx>=n)return -1;return ttULONG(d+12+idx*4);}
    return -1;
}

static __attribute__((pure)) int stbtt_GetFontOffsetForIndex(const unsigned char*d,int i){return _font_offset((unsigned char*)d,i);}
static __attribute__((pure)) int stbtt_FindGlyphIndex(const stbtt_fontinfo*info,int cp){
    u8*d=info->data;u32 im=info->index_map;u16 fmt=ttUSHORT(d+im);
    if(fmt==0) { i32 b=ttUSHORT(d+im+2); return cp<b-6 ? (*(u8*)(d+im+6+cp)) : 0; }
    if(fmt==6) { u32 f=ttUSHORT(d+im+6),n=ttUSHORT(d+im+8); return(u32)cp>=f&&(u32)cp<f+n?ttUSHORT(d+im+10+(cp-f)*2):0;}
    if(fmt==2)return 0;
    if(fmt==4){
        u16 sc=ttUSHORT(d+im+6)>>1,sr=ttUSHORT(d+im+8)>>1,es=ttUSHORT(d+im+10),rs=ttUSHORT(d+im+12)>>1;
        u32 ec=im+14,s=ec;if(cp>0xffff)return 0;
        if (cp>=ttUSHORT(d+s+rs*2)) s+=rs*2;s-=2;
        while(es) { sr>>=1; u16 e=ttUSHORT(d+s+sr*2); if(cp>e) {s+=sr*2;} --es; }
        s+=2;{u16 it=(u16)((s-ec)>>1),st=ttUSHORT(d+im+14+sc*2+2+2*it),la=ttUSHORT(d+ec+2*it);
        if(cp<st||cp>la)return 0;u16 off=ttUSHORT(d+im+14+sc*6+2+2*it);
        return off?ttUSHORT(d+off+(cp-st)*2+im+14+sc*6+2+2*it):(u16)(cp+ttSHORT(d+im+14+sc*4+2+2*it));}
    }
    if(fmt==12||fmt==13){u32 ng=ttULONG(d+im+12);i32 lo=0,hi=(i32)ng;
        while(lo<hi){i32 m=lo+((hi-lo)>>1);u32 sc=ttULONG(d+im+16+m*12),ec=ttULONG(d+im+16+m*12+4);
        if((u32)cp<sc)hi=m;else if((u32)cp>ec)lo=m+1;else{u32 sg=ttULONG(d+im+16+m*12+8);return fmt==12?sg+cp-sc:sg;}}return 0;}
    return 0;
}

static void _sv(stbtt_vertex*v,u8 t,i32 x,i32 y,i32 cx,i32 cy){v->type=t;v->x=(i16)x;v->y=(i16)y;v->cx=(i16)cx;v->cy=(i16)cy;}
static int _glyf_off(const stbtt_fontinfo*info,int gi){
    if(gi>=info->numGlyphs||info->indexToLocFormat>=2)return-1;
    int g1,g2;if(info->indexToLocFormat==0){g1=info->glyf+ttUSHORT(info->data+info->loca+gi*2)*2;g2=info->glyf+ttUSHORT(info->data+info->loca+gi*2+2)*2;}
    else{g1=info->glyf+ttULONG(info->data+info->loca+gi*4);g2=info->glyf+ttULONG(info->data+info->loca+gi*4+4);}
    return g1==g2?-1:g1;
}

static int _close_shape(stbtt_vertex*v,int n,int wo,int so,i32 sx,i32 sy,i32 scx,i32 scy,i32 cx,i32 cy){
    if(so){if(wo)_sv(&v[n++],STBTT_vcurve,(cx+scx)>>1,(cy+scy)>>1,cx,cy);_sv(&v[n++],STBTT_vcurve,sx,sy,scx,scy);}
    else{if(wo)_sv(&v[n++],STBTT_vcurve,sx,sy,cx,cy);else _sv(&v[n++],STBTT_vline,sx,sy,0,0);}
    return n;
}

static int _GetGlyphShapeT2(const stbtt_fontinfo*,int,stbtt_vertex**);
int stbtt_GetGlyphShape(const stbtt_fontinfo*info,int gi,stbtt_vertex**pv);
static int _GetGlyphShapeTT(const stbtt_fontinfo*info,int gi,stbtt_vertex**pv){
    u8*d=info->data;stbtt_vertex*verts=0;int nv=0,g=_glyf_off(info,gi);*pv=NULL;if(g<0)return 0;
    i16 nc=ttSHORT(d+g);
    if(nc>0){
        u8*ep=d+g+10;int ins=ttUSHORT(d+g+10+nc*2);u8*pts=d+g+10+nc*2+2+ins;
        int n=1+ttUSHORT(ep+nc*2-2),m=n+2*nc;verts=(stbtt_vertex*)TempAlloc(m*sizeof(verts[0]));if(!verts)return 0;
        int off=m-n;u8 fl=0,fc=0;
        for(int i=0;i<n;++i){if(fc==0){fl=*pts++;if(fl&8)fc=*pts++;}else--fc;verts[off+i].type=fl;}
        i32 x=0;for(int i=0;i<n;++i){fl=verts[off+i].type;if(fl&2){i16 dx=*pts++;x+=(fl&16)?dx:-dx;}else if(!(fl&16)){x+=(i16)(pts[0]*256+pts[1]);pts+=2;}verts[off+i].x=(i16)x;}
        i32 y=0;for(int i=0;i<n;++i){fl=verts[off+i].type;if(fl&4){i16 dy=*pts++;y+=(fl&32)?dy:-dy;}else if(!(fl&32)){y+=(i16)(pts[0]*256+pts[1]);pts+=2;}verts[off+i].y=(i16)y;}
        i32 sx=0,sy=0,cx=0,cy=0,scx=0,scy=0;int wo=0,so=0,nm=0,j=0;
        for(int i=0;i<n;++i){fl=verts[off+i].type;x=(i16)verts[off+i].x;y=(i16)verts[off+i].y;
            if(nm==i){if(i)nv=_close_shape(verts,nv,wo,so,sx,sy,scx,scy,cx,cy);so=!(fl&1);
                if(so){scx=x;scy=y;if(!(verts[off+i+1].type&1)){sx=(x+(i32)verts[off+i+1].x)>>1;sy=(y+(i32)verts[off+i+1].y)>>1;}else{sx=verts[off+i+1].x;sy=verts[off+i+1].y;++i;}}else{sx=x;sy=y;}
                _sv(&verts[nv++],STBTT_vmove,sx,sy,0,0);wo=0;nm=1+ttUSHORT(ep+j++*2);
            }else{
                if(!(fl&1)){
                    if(wo)_sv(&verts[nv++],STBTT_vcurve,(cx+x)>>1,(cy+y)>>1,cx,cy);cx=x;cy=y;wo=1;
                } else { _sv(&verts[nv++],wo ? STBTT_vcurve : STBTT_vline,x,y,wo ? cx : 0, wo ? cy : 0); wo=0; }
            }
        }
        nv=_close_shape(verts,nv,wo,so,sx,sy,scx,scy,cx,cy);
    }else if(nc<0){
        u8*comp=d+g+10;int more=1;
        while(more){stbtt_vertex*cv=0,*tmp=0;float mtx[6]={1,0,0,1,0,0};
            u16 fl=ttSHORT(comp);comp+=2;u16 gidx=ttSHORT(comp);comp+=2;
            if(fl&2) { if(fl&1) { mtx[4] = ttSHORT(comp); comp+=2; mtx[5]=ttSHORT(comp); comp+=2; } else { mtx[4]=(*(i8*)(comp)); comp++; mtx[5]=(*(i8*)(comp)); comp++; }}
            if(fl&(1<<3)){mtx[0]=mtx[3]=ttSHORT(comp)/16384.0f;comp+=2;mtx[1]=mtx[2]=0;}
            else if(fl&(1<<6)){mtx[0]=ttSHORT(comp)/16384.0f;comp+=2;mtx[1]=mtx[2]=0;mtx[3]=ttSHORT(comp)/16384.0f;comp+=2;}
            else if(fl&(1<<7)){mtx[0]=ttSHORT(comp)/16384.0f;comp+=2;mtx[1]=ttSHORT(comp)/16384.0f;comp+=2;mtx[2]=ttSHORT(comp)/16384.0f;comp+=2;mtx[3]=ttSHORT(comp)/16384.0f;comp+=2;}
            float fm=vsqrtf(mtx[0]*mtx[0]+mtx[1]*mtx[1]),fn=vsqrtf(mtx[2]*mtx[2]+mtx[3]*mtx[3]);
            int cn=stbtt_GetGlyphShape(info,gidx,&cv);
            if(cn>0){for(int i=0;i<cn;++i){stbtt_vertex*v=&cv[i];i16 vx=v->x,vy=v->y;v->x=(i16)(fm*(mtx[0]*vx+mtx[2]*vy+mtx[4]));v->y=(i16)(fn*(mtx[1]*vx+mtx[3]*vy+mtx[5]));vx=v->cx;vy=v->cy;v->cx=(i16)(fm*(mtx[0]*vx+mtx[2]*vy+mtx[4]));v->cy=(i16)(fn*(mtx[1]*vx+mtx[3]*vy+mtx[5]));}
                tmp=(stbtt_vertex*)TempAlloc((nv+cn)*sizeof(stbtt_vertex));if(!tmp){TempFree(verts);TempFree(cv);return 0;}
                if(nv>0&&verts) MemCpyFromBtoAForNBytes(tmp,verts,nv*sizeof(stbtt_vertex)); MemCpyFromBtoAForNBytes(tmp+nv,cv,cn*sizeof(stbtt_vertex));TempFree(verts);TempFree(cv);verts=tmp;nv+=cn;}
            more=fl&(1<<5);}
    }
    *pv=verts;return nv;
}

typedef struct{int bounds,started;float first_x,first_y,x,y;i32 min_x,max_x,min_y,max_y;stbtt_vertex*pvertices;int num_vertices;}stbtt__csctx;
#define CSCTX_INIT(b) {b,0,0,0,0,0,0,0,0,0,NULL,0}
static void _trk(stbtt__csctx*c,i32 x,i32 y){if(x>c->max_x||!c->started)c->max_x=x;if(y>c->max_y||!c->started)c->max_y=y;if(x<c->min_x||!c->started)c->min_x=x;if(y<c->min_y||!c->started)c->min_y=y;c->started=1;}
static void _csv(stbtt__csctx*c,u8 t,i32 x,i32 y,i32 cx,i32 cy,i32 cx1,i32 cy1){if(c->bounds){_trk(c,x,y);if(t==STBTT_vcubic){_trk(c,cx,cy);_trk(c,cx1,cy1);}}else{_sv(&c->pvertices[c->num_vertices],t,x,y,cx,cy);c->pvertices[c->num_vertices].cx1=(i16)cx1;c->pvertices[c->num_vertices].cy1=(i16)cy1;}c->num_vertices++;}
static void _csclose(stbtt__csctx*c){if(c->first_x!=c->x||c->first_y!=c->y)_csv(c,STBTT_vline,(int)c->first_x,(int)c->first_y,0,0,0,0);}
static void _csmove(stbtt__csctx*c,float dx,float dy){_csclose(c);c->first_x=c->x=c->x+dx;c->first_y=c->y=c->y+dy;_csv(c,STBTT_vmove,(int)c->x,(int)c->y,0,0,0,0);}
static void _csline(stbtt__csctx*c,float dx,float dy){c->x+=dx;c->y+=dy;_csv(c,STBTT_vline,(int)c->x,(int)c->y,0,0,0,0);}
static void _cscurve(stbtt__csctx*c,float d1,float e1,float d2,float e2,float d3,float e3){float cx1=c->x+d1,cy1=c->y+e1,cx2=cx1+d2,cy2=cy1+e2;c->x=cx2+d3;c->y=cy2+e3;_csv(c,STBTT_vcubic,(int)c->x,(int)c->y,(int)cx1,(int)cy1,(int)cx2,(int)cy2);}
static stbtt__buf _subr(stbtt__buf idx,int n){
    _bsk(&idx,0); int c = _bg(&idx,2);
    int bias = (c >= 33900) ? 32768 : ((c >= 1240) ? 1131 : 107); n+=bias;
    return (n<0 || n>=c) ? stbtt__new_buf(NULL,0) : _cff_idx_get(idx,n);
}
static stbtt__buf _cid_subrs(const stbtt_fontinfo*info,int gi){stbtt__buf fd=info->fdselect;int nr,st,end,v,fmt,sel=-1,i;_bsk(&fd,0);fmt=_bg8(&fd);
    if(fmt==0){_bskip(&fd,gi);sel=_bg8(&fd);}
    else if(fmt==3){nr=_bg(&fd,2);st=_bg(&fd,2);for(i=0;i<nr;i++){v=_bg8(&fd);end=_bg(&fd,2);if(gi>=st&&gi<end){sel=v;break;}st=end;}}
    if(sel==-1)return stbtt__new_buf(NULL,0);return _get_subrs(info->cff,_cff_idx_get(info->fontdicts,sel));}

static int _run_cs(const stbtt_fontinfo*info,int gi,stbtt__csctx*c){
    int hdr=1,mb=0,ssh=0,sp=0,hs=0,i,b0;float s[48],f;
    stbtt__buf ss[10],subrs=info->subrs,b=_cff_idx_get(info->charstrings,gi);
#define ERR(x) return 0
#define CHK(n) if(sp<(n))ERR(#n)
    while(b.cursor<b.size){int cs=1;i=0;b0=_bg8(&b);
        switch(b0){
        case 0x13:case 0x14:if(hdr)mb+=sp/2;hdr=0;_bskip(&b,(mb+7)/8);break;
        case 0x01:case 0x03:case 0x12:case 0x17:mb+=sp/2;break;
        case 0x15:hdr=0;CHK(2);_csmove(c,s[sp-2],s[sp-1]);break;
        case 0x04:hdr=0;CHK(1);_csmove(c,0,s[sp-1]);break;
        case 0x16:hdr=0;CHK(1);_csmove(c,s[sp-1],0);break;
        case 0x05:CHK(2);for(;i+1<sp;i+=2)_csline(c,s[i],s[i+1]);break;
        case 0x07:CHK(1);goto vlt;
        case 0x06:CHK(1);for(;;){if(i>=sp)break;_csline(c,s[i++],0);vlt:if(i>=sp)break;_csline(c,0,s[i++]);}break;
        case 0x1F:CHK(4);goto hvc;
        case 0x1E:CHK(4);for(;;){if(i+3>=sp)break;_cscurve(c,0,s[i],s[i+1],s[i+2],s[i+3],(sp-i==5)?s[i+4]:0);i+=4;hvc:if(i+3>=sp)break;_cscurve(c,s[i],0,s[i+1],s[i+2],(sp-i==5)?s[i+4]:0,s[i+3]);i+=4;}break;
        case 0x08:CHK(6);for(;i+5<sp;i+=6)_cscurve(c,s[i],s[i+1],s[i+2],s[i+3],s[i+4],s[i+5]);break;
        case 0x18:CHK(8);for(;i+5<sp-2;i+=6)_cscurve(c,s[i],s[i+1],s[i+2],s[i+3],s[i+4],s[i+5]);_csline(c,s[i],s[i+1]);break;
        case 0x19:CHK(8);for(;i+1<sp-6;i+=2)_csline(c,s[i],s[i+1]);_cscurve(c,s[i],s[i+1],s[i+2],s[i+3],s[i+4],s[i+5]);break;
        case 0x1A:case 0x1B:CHK(4);f=0;if(sp&1)f=s[i++];for(;i+3<sp;i+=4,f=0)_cscurve(c,b0==0x1B?s[i]:f,b0==0x1B?f:s[i],s[i+1],s[i+2],b0==0x1B?s[i+3]:0,b0==0x1B?0:s[i+3]);break;
        case 0x0A:if(!hs){if(info->fdselect.size)subrs=_cid_subrs(info,gi);hs=1;}
        case 0x1D:CHK(1);if(ssh>=10)ERR("recursion");ss[ssh++]=b;b=_subr(b0==0x0A?subrs:info->gsubrs,(int)s[--sp]);if(!b.size)ERR("subr");b.cursor=0;cs=0;break;
        case 0x0B:if(ssh<=0)ERR("return");b=ss[--ssh];cs=0;break;
        case 0x0E:_csclose(c);return 1;
        case 0x0C:{int b1=_bg8(&b);switch(b1){
            case 0x22:CHK(7);_cscurve(c,s[0],0,s[1],s[2],s[3],0);_cscurve(c,s[4],0,s[5],-s[2],s[6],0);break;
            case 0x23:CHK(13);_cscurve(c,s[0],s[1],s[2],s[3],s[4],s[5]);_cscurve(c,s[6],s[7],s[8],s[9],s[10],s[11]);break;
            case 0x24:CHK(9);_cscurve(c,s[0],s[1],s[2],s[3],s[4],0);_cscurve(c,s[5],0,s[6],s[7],s[8],-(s[1]+s[3]+s[7]));break;
            case 0x25:CHK(11);{float dx=s[0]+s[2]+s[4]+s[6]+s[8],dy=s[1]+s[3]+s[5]+s[7]+s[9],d6x=s[10],d6y=s[10];if(vabs(dx)>vabs(dy))d6y=-dy;else d6x=-dx;_cscurve(c,s[0],s[1],s[2],s[3],s[4],s[5]);_cscurve(c,s[6],s[7],s[8],s[9],d6x,d6y);}break;
            default:ERR("escape");}}break;
        default:if(b0!=255&&b0!=28&&b0<32)ERR("reserved");f=(b0==255)?(float)(i32)_bg(&b,4)/0x10000:(_bskip(&b,-1),(float)(i16)_cff_int(&b));if(sp>=48)ERR("overflow");s[sp++]=f;cs=0;break;}
        if(cs)sp=0;}ERR("no endchar");
#undef ERR
#undef CHK
}

int stbtt_GetGlyphShape(const stbtt_fontinfo*info,int gi,stbtt_vertex**pv){return info->cff.size?_GetGlyphShapeT2(info,gi,pv):_GetGlyphShapeTT(info,gi,pv);}
static int _GetGlyphShapeT2(const stbtt_fontinfo*info,int gi,stbtt_vertex**pv){stbtt__csctx cc=CSCTX_INIT(1),oc=CSCTX_INIT(0);if(_run_cs(info,gi,&cc)){*pv=(stbtt_vertex*)TempAlloc(cc.num_vertices*sizeof(stbtt_vertex));oc.pvertices=*pv;if(_run_cs(info,gi,&oc))return oc.num_vertices;}*pv=NULL;return 0;}
static int _GetGlyphInfoT2(const stbtt_fontinfo*info,int gi,int*x0,int*y0,int*x1,int*y1){stbtt__csctx c=CSCTX_INIT(1);int r=_run_cs(info,gi,&c);if(x0)*x0=r?c.min_x:0;if(y0)*y0=r?c.min_y:0;if(x1)*x1=r?c.max_x:0;if(y1)*y1=r?c.max_y:0;return r?c.num_vertices:0;}
static int stbtt_GetGlyphBox(const stbtt_fontinfo*info,int gi,int*x0,int*y0,int*x1,int*y1){
    if(info->cff.size){_GetGlyphInfoT2(info,gi,x0,y0,x1,y1);}
    else{int g=_glyf_off(info,gi);if(g<0)return 0;if(x0)*x0=ttSHORT(info->data+g+2);if(y0)*y0=ttSHORT(info->data+g+4);if(x1)*x1=ttSHORT(info->data+g+6);if(y1)*y1=ttSHORT(info->data+g+8);}
    return 1;
}

static void stbtt_GetGlyphHMetrics(const stbtt_fontinfo*info,int gi,int*adv,int*lsb){
    u16 n=ttUSHORT(info->data+info->hhea+34);
    if(gi<n){if(adv)*adv=ttSHORT(info->data+info->hmtx+4*gi);if(lsb)*lsb=ttSHORT(info->data+info->hmtx+4*gi+2);}
    else{if(adv)*adv=ttSHORT(info->data+info->hmtx+4*(n-1));if(lsb)*lsb=ttSHORT(info->data+info->hmtx+4*n+2*(gi-n));}
}

static __attribute__((pure)) float stbtt_ScaleForPixelHeight(const stbtt_fontinfo*info,float h){return h/(float)(ttSHORT(info->data+info->hhea+4)-ttSHORT(info->data+info->hhea+6));}
static __attribute__((pure)) float stbtt_ScaleForMappingEmToPixels(const stbtt_fontinfo*info,float px){return px/(float)ttUSHORT(info->data+info->head+18);}
static void stbtt_GetGlyphBitmapBoxSubpixel(const stbtt_fontinfo*font,int g,float sx,float sy,float shx,float shy,int*ix0,int*iy0,int*ix1,int*iy1){
    int x0=0,y0=0,x1,y1;if(!stbtt_GetGlyphBox(font,g,&x0,&y0,&x1,&y1)){if(ix0)*ix0=0;if(iy0)*iy0=0;if(ix1)*ix1=0;if(iy1)*iy1=0;}
    else{if(ix0)*ix0=(int)vfloor(x0*sx+shx);if(iy0)*iy0=(int)vfloor(-y1*sy+shy);if(ix1)*ix1=(int)vceil(x1*sx+shx);if(iy1)*iy1=(int)vceil(-y0*sy+shy);}
}

typedef struct{int w,h,stride;unsigned char*pixels;}stbtt__bitmap;
typedef struct stbtt__hheap_chunk{ struct stbtt__hheap_chunk* next; }stbtt__hheap_chunk;
typedef struct{ stbtt__hheap_chunk* head; void* first_free; int num_remaining_in_head_chunk; }stbtt__hheap;
static void* _hha(stbtt__hheap* hh,size_t sz){ if(hh->first_free){void*p=hh->first_free;hh->first_free=*(void**)p;return p;}if(!hh->num_remaining_in_head_chunk){int c=sz<32?2000:sz<128?800:100;stbtt__hheap_chunk*ck=(stbtt__hheap_chunk*)TempAlloc(sizeof(*ck)+sz*c);if(!ck)return NULL;ck->next=hh->head;hh->head=ck;hh->num_remaining_in_head_chunk=c;}--hh->num_remaining_in_head_chunk;return(char*)hh->head+sizeof(stbtt__hheap_chunk)+sz*hh->num_remaining_in_head_chunk; }
static void _hhf(stbtt__hheap* hh,void*p) { *(void**)p=hh->first_free;hh->first_free=p; }
typedef struct{ float x0,y0,x1,y1; int invert; }stbtt__edge;
typedef struct stbtt__active_edge{ struct stbtt__active_edge*next; float fx,fdx,fdy,direction,sy,ey; }stbtt__active_edge;
static void _hce(float*sl,int x,stbtt__active_edge*e,float x0,float y0,float x1,float y1){
    if(y0==y1||y0>e->ey||y1<e->sy)return;if(y0<e->sy){x0+=(x1-x0)*(e->sy-y0)/(y1-y0);y0=e->sy;}if(y1>e->ey){x1+=(x1-x0)*(e->ey-y1)/(y1-y0);y1=e->ey;}
    if(x0<=x&&x1<=x)sl[x]+=e->direction*(y1-y0);else if(x0>=x+1&&x1>=x+1);else sl[x]+=e->direction*(y1-y0)*(1.0f-((x0-(float)x)+(x1-(float)x))/2.0f);
}

static float _ptz(float h,float t0,float t1,float b0,float b1){ return ((t1-t0)+(b1-b0))/2.0f*h; }
static void _fae(float*sl,float*sf,int len,stbtt__active_edge*e,float yt){
    float yb = yt + 1;
    while(e) {
        if(e->fdx==0){float x0=e->fx;if(x0<len){if(x0>=0){_hce(sl,(int)x0,e,x0,yt,x0,yb);_hce(sf-1,(int)x0+1,e,x0,yt,x0,yb);}else _hce(sf-1,0,e,x0,yt,x0,yb);}}
        else{float x0=e->fx,dx=e->fdx,dy=e->fdy,xb=x0+dx,xt,xbt,sy0,sy1;
            if(e->sy>yt){xt=x0+dx*(e->sy-yt);sy0=e->sy;}else{xt=x0;sy0=yt;}
            if(e->ey<yb){xbt=x0+dx*(e->ey-yt);sy1=e->ey;}else{xbt=xb;sy1=yb;}
            if(xt>=0&&xbt>=0&&xt<len&&xbt<len){
                if((int)xt==(int)xbt){int x=(int)xt;float h=(sy1-sy0)*e->direction;sl[x]+=_ptz(h,xt,(float)x+1.0f,xbt,(float)x+1.0f);sf[x]+=h;}
                else{float yc,yf,step,sign,area;
                    if(xt>xbt){float t;sy0=yb-(sy0-yt);sy1=yb-(sy1-yt);t=sy0;sy0=sy1;sy1=t;t=xbt;xbt=xt;xt=t;dx=-dx;dy=-dy;t=x0;x0=xb;xb=t;}
                    int x1=(int)xt,x2=(int)xbt;yc=yt+dy*((float)(x1+1)-x0);yf=yt+dy*((float)x2-x0);if(yc>yb)yc=yb;sign=e->direction;area=sign*(yc-sy0);
                    sl[x1] += area*((float)(x1+1)-xt)/2;
                    if(yf>yb){yf=yb;dy=(yf-yc)/((float)x2-(float)(x1+1));}
                    step=sign*dy;for(int x=x1+1;x<x2;++x){sl[x]+=area+step/2;area+=step;}
                    sl[x2]+=area+sign*_ptz(sy1-yf,(float)x2,(float)x2+1.0f,xbt,(float)x2+1.0f);sf[x2]+=sign*(sy1-sy0);}
            }else{for(int x=0;x<len;++x){float y0=yt,x1f=(float)x,x2f=(float)(x+1),x3=xb,y3=yb;float y1=((float)x-x0)/dx+yt,y2=((float)(x+1)-x0)/dx+yt;
                if(x0<x1f&&x3>x2f){_hce(sl,x,e,x0,y0,x1f,y1);_hce(sl,x,e,x1f,y1,x2f,y2);_hce(sl,x,e,x2f,y2,x3,y3);}
                else if(x3<x1f&&x0>x2f){_hce(sl,x,e,x0,y0,x2f,y2);_hce(sl,x,e,x2f,y2,x1f,y1);_hce(sl,x,e,x1f,y1,x3,y3);}
                else if ((x0<x1f&&x3>x1f) || (x3<x1f&&x0>x1f)) {_hce(sl,x,e,x0,y0,x1f,y1);_hce(sl,x,e,x1f,y1,x3,y3);}
                else if ((x0<x2f&&x3>x2f) || (x3<x2f&&x0>x2f)) {_hce(sl,x,e,x0,y0,x2f,y2);_hce(sl,x,e,x2f,y2,x3,y3);}
                else _hce(sl,x,e,x0,y0,x3,y3);}}
        }e=e->next;}
}

static void _rse(stbtt__bitmap*res,stbtt__edge*e,int n,int ox,int oy){
    stbtt__hheap hh={0,0,0};stbtt__active_edge*active=NULL;int y,j=0,i;
    float sd[129],*sl,*sl2;if(res->w>64)sl=(float*)TempAlloc((size_t)(res->w*2+1)*sizeof(float));else sl=sd;
    sl2=sl+res->w;y=oy;e[n].y0=(float)(oy+res->h)+1;
    while(j<res->h){float syt=(float)y,syb=(float)y+1;stbtt__active_edge**step=&active;
        MemSetToVForNBytes(sl,0,(size_t)res->w*sizeof(sl[0]));MemSetToVForNBytes(sl2,0,((size_t)res->w+1)*sizeof(sl[0]));
        while(*step){stbtt__active_edge*z=*step;if(z->ey<=syt){*step=z->next;z->direction=0;_hhf(&hh,z);}else step=&(*step)->next;}
        while(e->y0<=syb){
            if(e->y0!=e->y1){
                stbtt__active_edge* z=(stbtt__active_edge*)_hha(&hh,sizeof(*z));
                if(z) {
                    float dxdy = (e->x1-e->x0)/(e->y1-e->y0);
                    z->fdx = dxdy; z->fdy = dxdy ? 1.0f/dxdy : 0;
                    z->fx = e->x0 + dxdy * (syt - e->y0) - (float)ox;
                    z->direction = e->invert ? 1.0f : -1.0f;
                    z->sy = e->y0; z->ey = e->y1; z->next = 0;
                    if(j == 0 && oy != 0 && z->ey < syt) z->ey = syt;
                    z->next = active; active = z;
                }
            }++e;
        }
        if(active)_fae(sl,sl2+1,res->w,active,syt);
        {float sum=0;for(i=0;i<res->w;++i){float k;int m;sum+=sl2[i];k=(float)vabs(sl[i]+sum)*255.0f+0.5f;m=(int)k;if(m>255)m=255;res->pixels[j*res->stride+i]=(unsigned char)m;}}
        step=&active;while(*step){stbtt__active_edge*z=*step;z->fx+=z->fdx;step=&(*step)->next;}++y;++j;}
    stbtt__hheap_chunk* c = hh.head; while(c){ stbtt__hheap_chunk* hp = c->next; TempFree(c); c = hp;} if(sl != sd) TempFree(sl);
}

#define _CMP(a,b) ((a)->y0<(b)->y0)
#define _SWP(a,b) {stbtt__edge t_=(a);(a)=(b);(b)=t_;}
static void _eis(stbtt__edge*p,int n){for(int i=1;i<n;++i){stbtt__edge t=p[i];int j=i;while(j>0&&_CMP(&t,&p[j-1])){p[j]=p[j-1];--j;}p[j]=t;}}
static void _eqs(stbtt__edge*p,int n){while(n>12){int m=n>>1,c01=_CMP(&p[0],&p[m]),c12=_CMP(&p[m],&p[n-1]);if(c01!=c12){int z=(_CMP(&p[0],&p[n-1])==c12)?0:n-1;_SWP(p[z],p[m]);}_SWP(p[0],p[m]);int i=1,j=n-1;for(;;){while(_CMP(&p[i],&p[0]))++i;while(_CMP(&p[0],&p[j]))--j;if(i>=j)break;_SWP(p[i],p[j]);++i;--j;}if(j<n-i){_eqs(p,j);p+=i;n-=i;}else{_eqs(p+i,n-i);n=j;}}}
static void _esort(stbtt__edge*p,int n){_eqs(p,n);_eis(p,n);}
static void _add_pt(Vector2*p,int n,float x,float y){if(p){p[n].x=x;p[n].y=y;}}
static int _tess_c(Vector2*pts,int*np,float x0,float y0,float x1,float y1,float x2,float y2,float fsq,int n){
    float mx=(x0+2*x1+x2)/4,my=(y0+2*y1+y2)/4,dx=(x0+x2)/2-mx,dy=(y0+y2)/2-my;
    if(n>16||dx*dx+dy*dy<=fsq){_add_pt(pts,(*np)++,x2,y2);return 1;}
    _tess_c(pts,np,x0,y0,(x0+x1)/2,(y0+y1)/2,mx,my,fsq,n+1);_tess_c(pts,np,mx,my,(x1+x2)/2,(y1+y2)/2,x2,y2,fsq,n+1);return 1;
}

static void _tess_cb(Vector2*pts,int*np,float x0,float y0,float x1,float y1,float x2,float y2,float x3,float y3,float fsq,int n){
    float d0=vsqrtf((x1-x0)*(x1-x0)+(y1-y0)*(y1-y0)),d1=vsqrtf((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1)),d2=vsqrtf((x3-x2)*(x3-x2)+(y3-y2)*(y3-y2)),ds=vsqrtf((x3-x0)*(x3-x0)+(y3-y0)*(y3-y0)),ll=d0+d1+d2;
    if(n>16||ll*ll-ds*ds<=fsq){_add_pt(pts,(*np)++,x3,y3);return;}
    float x01=(x0+x1)/2,y01=(y0+y1)/2,x12=(x1+x2)/2,y12=(y1+y2)/2,x23=(x2+x3)/2,y23=(y2+y3)/2,xa=(x01+x12)/2,ya=(y01+y12)/2,xb=(x12+x23)/2,yb=(y12+y23)/2,mx=(xa+xb)/2,my=(ya+yb)/2;
    _tess_cb(pts,np,x0,y0,x01,y01,xa,ya,mx,my,fsq,n+1);_tess_cb(pts,np,mx,my,xb,yb,x23,y23,x3,y3,fsq,n+1);
}

static Vector2* _flatten(stbtt_vertex*v,int nv,float flat,int**cl,int*nc){
    float fsq=flat*flat;int n=0;for(int i=0;i<nv;++i)if(v[i].type==STBTT_vmove)++n;
    *nc=n;if(!n)return 0;*cl=(int*)TempAlloc(sizeof(int)*(size_t)n);Vector2*pts=0;int np=0;
    for(int pass=0;pass<2;++pass){float x=0,y=0;int start=0;n=-1;if(pass==1){pts=(Vector2*)TempAlloc((size_t)np*sizeof(Vector2));if(!pts)goto err;}np=0;
        for(int i=0;i<nv;++i){switch(v[i].type){
            case STBTT_vmove:if(n>=0)(*cl)[n]=np-start;start=np;++n;x=v[i].x;y=v[i].y;_add_pt(pts,np++,x,y);break;
            case STBTT_vline:x=v[i].x;y=v[i].y;_add_pt(pts,np++,x,y);break;
            case STBTT_vcurve:_tess_c(pts,&np,x,y,v[i].cx,v[i].cy,v[i].x,v[i].y,fsq,0);x=v[i].x;y=v[i].y;break;
            case STBTT_vcubic:_tess_cb(pts,&np,x,y,v[i].cx,v[i].cy,v[i].cx1,v[i].cy1,v[i].x,v[i].y,fsq,0);x=v[i].x;y=v[i].y;break;}}
        (*cl)[n]=np-start;}return pts;
    err:TempFree(pts);TempFree(*cl);*cl=0;*nc=0;return NULL;
}

static void _rasterize(stbtt__bitmap*res,Vector2*pts,int*wc,int nw,float sx,float sy,float shx,float shy,int ox,int oy,int inv){
    float ysi=inv?-sy:sy;stbtt__edge*e;int n=0,i,j,k;for(i=0;i<nw;++i)n+=wc[i];
    e=(stbtt__edge*)TempAlloc(sizeof(*e)*((size_t)n+1));if(!e)return;n=0;int m=0;
    for(i=0;i<nw;++i){Vector2*p=pts+m;m+=wc[i];j=wc[i]-1;for(k=0;k<wc[i];j=k++){int a=k,b=j;if(p[j].y==p[k].y)continue;e[n].invert=0;if(inv?p[j].y>p[k].y:p[j].y<p[k].y){e[n].invert=1;a=j;b=k;}e[n].x0=p[a].x*sx+shx;e[n].y0=p[a].y*ysi+shy;e[n].x1=p[b].x*sx+shx;e[n].y1=p[b].y*ysi+shy;++n;}}
    _esort(e,n);_rse(res,e,n,ox,oy);TempFree(e);
}

void stbtt_MakeGlyphBitmapSubpixel(const stbtt_fontinfo*info,unsigned char*out,int ow,int oh,int ostr,float sx,float sy,float shx,float shy,int g){
    stbtt_vertex*v;int ix0,iy0,nv=stbtt_GetGlyphShape(info,g,&v);stbtt__bitmap gbm;
    stbtt_GetGlyphBitmapBoxSubpixel(info,g,sx,sy,shx,shy,&ix0,&iy0,0,0);gbm.pixels=out;gbm.w=ow;gbm.h=oh;gbm.stride=ostr;
    float scale=sx>sy?sy:sx;int wc=0;int*wl=NULL;Vector2*win=_flatten(v,nv,0.35f/scale,&wl,&wc);
    if(win){_rasterize(&gbm,win,wl,wc,sx,sy,shx,shy,ix0,iy0,1);TempFree(wl);TempFree(win);}if(v)TempFree(v);
}

typedef int stbrp_coord; typedef struct{int width,height,x,y,bottom_y;}stbrp_context;
typedef struct{unsigned char x;}stbrp_node; typedef struct{stbrp_coord x,y;int id,w,h,was_packed;}stbrp_rect;
static void stbrp_pack_rects(stbrp_context*con,stbrp_rect*rects,int n){
    int i;for(i=0;i<n;++i){if(con->x+rects[i].w>con->width){con->x=0;con->y=con->bottom_y;}if(con->y+rects[i].h>con->height)break;rects[i].x=con->x;rects[i].y=con->y;rects[i].was_packed=1;con->x+=rects[i].w;if(con->y+rects[i].h>con->bottom_y)con->bottom_y=con->y+rects[i].h;}for(;i<n;++i)rects[i].was_packed=0;
}

typedef struct{unsigned short x0,y0,x1,y1;float xoff,yoff,xadvance,xoff2,yoff2;}stbtt_packedchar;
typedef struct{float x0,y0,s0,t0,x1,y1,s1,t1;} aligned_quad;
typedef struct{void*uac;void*pack_info;int width,height,stride_in_bytes,padding,skip_missing;unsigned int h_oversample,v_oversample;unsigned char*pixels;}stbtt_pack_context;
typedef struct{float font_size;int first_unicode_codepoint_in_range;int*array_of_unicode_codepoints;int num_chars;stbtt_packedchar*chardata_for_range;unsigned char h_oversample,v_oversample;}FPackRange;
static void stbtt_GetPackedQuad(const stbtt_packedchar*cd, int pw, int ph, int ci, float*xpos, float*ypos, aligned_quad*q, int ai){
    float ipw=1.0f/pw,iph=1.0f/ph;const stbtt_packedchar*b=cd+ci;
    if(ai){float x=vfloor((*xpos+b->xoff)+0.5f),y=vfloor((*ypos+b->yoff)+0.5f);q->x0=x;q->y0=y;q->x1=x+b->xoff2-b->xoff;q->y1=y+b->yoff2-b->yoff;}
    else{q->x0=*xpos+b->xoff;q->y0=*ypos+b->yoff;q->x1=*xpos+b->xoff2;q->y1=*ypos+b->yoff2;}
    q->s0=b->x0*ipw;q->t0=b->y0*iph;q->s1=b->x1*ipw;q->t1=b->y1*iph;*xpos+=b->xadvance;
}

int stbtt_PackBegin(stbtt_pack_context*spc,unsigned char* px, int pw, int ph, int str, int pad, void* a){
    stbrp_context*ctx=(stbrp_context*)TempAlloc(sizeof(*ctx)); *ctx=(stbrp_context){pw-pad,ph-pad,0,0,0}; if(px) MemSetToVForNBytes(px,0,(size_t)(pw*ph));
    return *spc=(stbtt_pack_context){a,ctx,pw,ph,str ? str : pw,pad,0,1,1,px},1;
}

#define STBTT_MAX_OVERSAMPLE 8
#define OVER_MASK (STBTT_MAX_OVERSAMPLE-1)
static void _hpre(unsigned char*p,int w,int h,int str,unsigned int kw){for(int j=0;j<h;++j,p+=str){unsigned char buf[STBTT_MAX_OVERSAMPLE]={0};int tot=0;for(int i=0;i<w;++i){if(i<=w-(int)kw){tot+=p[i]-buf[i&OVER_MASK];buf[(i+kw)&OVER_MASK]=p[i];}else tot-=buf[i&OVER_MASK];p[i]=(unsigned char)(tot/kw);}}}
static void _vpre(unsigned char*p,int w,int h,int str,unsigned int kw){for(int j=0;j<w;++j,++p){unsigned char buf[STBTT_MAX_OVERSAMPLE]={0};int tot=0;for(int i=0;i<h;++i){if(i<=h-(int)kw){tot+=p[i*str]-buf[i&OVER_MASK];buf[(i+kw)&OVER_MASK]=p[i*str];}else tot-=buf[i&OVER_MASK];p[i*str]=(unsigned char)(tot/kw);}}}
static float _oshift(int os){return os?-(float)(os-1)/(2.0f*(float)os):0.0f;}
static int stbtt_PackFontRangesGatherRects(stbtt_pack_context*spc,const stbtt_fontinfo*info,FPackRange*ranges,int nr,stbrp_rect*rects){
    int mga=0,k=0;for(int i=0;i<nr;++i){float fh=ranges[i].font_size,sc=fh>0?stbtt_ScaleForPixelHeight(info,fh):stbtt_ScaleForMappingEmToPixels(info,-fh);ranges[i].h_oversample=(unsigned char)spc->h_oversample;ranges[i].v_oversample=(unsigned char)spc->v_oversample;
        for(int j=0;j<ranges[i].num_chars;++j){int x0,y0,x1,y1,cp=ranges[i].array_of_unicode_codepoints?ranges[i].array_of_unicode_codepoints[j]:ranges[i].first_unicode_codepoint_in_range+j,g=stbtt_FindGlyphIndex(info,cp);
            if(g==0&&(spc->skip_missing||mga)){rects[k].w=rects[k].h=0;}else{stbtt_GetGlyphBitmapBoxSubpixel(info,g,sc*(float)spc->h_oversample,sc*(float)spc->v_oversample,0,0,&x0,&y0,&x1,&y1);rects[k].w=(stbrp_coord)(x1-x0+spc->padding+(int)spc->h_oversample-1);rects[k].h=(stbrp_coord)(y1-y0+spc->padding+(int)spc->v_oversample-1);if(g==0)mga=1;}++k;}}return k;
}

static int stbtt_PackFontRangesRenderIntoRects(stbtt_pack_context* spc, const stbtt_fontinfo* info, FPackRange* ranges, int nr, stbrp_rect* rects) {
    int i, j, k = 0, mg = -1, rv = 1, oh = spc->h_oversample, ov = spc->v_oversample;
    for (i = 0; i < nr; ++i) {
        FPackRange* rng = &ranges[i];
        float sc = rng->font_size > 0 ? stbtt_ScaleForPixelHeight(info, rng->font_size) : stbtt_ScaleForMappingEmToPixels(info, -rng->font_size);
        spc->h_oversample = rng->h_oversample; spc->v_oversample = rng->v_oversample;
        for (j = 0; j < rng->num_chars; ++j, ++k) {
            stbrp_rect* r = &rects[k];
            if (r->was_packed && r->w && r->h) {
                int cp = rng->array_of_unicode_codepoints ? rng->array_of_unicode_codepoints[j] : rng->first_unicode_codepoint_in_range + j;
                int g = stbtt_FindGlyphIndex(info, cp);
                stbtt_packedchar* bc = &rng->chardata_for_range[j];
                r->x += spc->padding; r->y += spc->padding; r->w -= spc->padding; r->h -= spc->padding;
                int adv,lsb; stbtt_GetGlyphHMetrics(info, g, &adv, &lsb);
                int x0,y0,x1,y1;
                stbtt_GetGlyphBitmapBoxSubpixel(info,g,sc * spc->h_oversample,sc * spc->v_oversample,0,0,&x0,&y0,&x1,&y1);
                unsigned char* p_pixels = spc->pixels + r->x + r->y * spc->stride_in_bytes;
                stbtt_MakeGlyphBitmapSubpixel(info, p_pixels, r->w - spc->h_oversample + 1, r->h - spc->v_oversample + 1, spc->stride_in_bytes, sc * spc->h_oversample, sc * spc->v_oversample, 0, 0, g);
                if (spc->h_oversample > 1) _hpre(p_pixels, r->w, r->h, spc->stride_in_bytes, spc->h_oversample);
                if (spc->v_oversample > 1) _vpre(p_pixels, r->w, r->h, spc->stride_in_bytes, spc->v_oversample);
                bc->x0 = r->x; bc->y0 = r->y; bc->x1 = r->x + r->w; bc->y1 = r->y + r->h;
                bc->xadvance = sc * adv;
                bc->xoff  = x0 * (1.0f / spc->h_oversample) + _oshift(spc->h_oversample);
                bc->yoff  = y0 * (1.0f / spc->v_oversample) + _oshift(spc->v_oversample);
                bc->xoff2 = (x0 + r->w) * (1.0f / spc->h_oversample) + _oshift(spc->h_oversample);
                bc->yoff2 = (y0 + r->h) * (1.0f / spc->v_oversample) + _oshift(spc->v_oversample);
                if (!g) mg = j;
            } else if (r->was_packed && !r->w && !r->h && mg >= 0) rng->chardata_for_range[j] = rng->chardata_for_range[mg];
            else if (!spc->skip_missing) rv = 0;
        }
    }
    spc->h_oversample = oh; spc->v_oversample = ov;
    return rv;
}

static int stbtt_PackFontRanges(stbtt_pack_context*spc,const unsigned char*fontdata,int fi,FPackRange*ranges,int nr){
    stbtt_fontinfo info;int i,j,n=0,rv=1;stbrp_rect*rects;
    for(i=0;i<nr;++i)for(j=0;j<ranges[i].num_chars;++j)ranges[i].chardata_for_range[j].x0=ranges[i].chardata_for_range[j].y0=ranges[i].chardata_for_range[j].x1=ranges[i].chardata_for_range[j].y1=0;
    for(i=0;i<nr;++i)n+=ranges[i].num_chars;rects=(stbrp_rect*)TempAlloc(sizeof(*rects)*(size_t)n);if(!rects)return 0;
    info.userdata=spc->uac;stbtt_InitFont_internal(&info,(unsigned char*)fontdata,stbtt_GetFontOffsetForIndex(fontdata,fi));
    n=stbtt_PackFontRangesGatherRects(spc,&info,ranges,nr,rects);stbrp_pack_rects(spc->pack_info,rects,n);rv=stbtt_PackFontRangesRenderIntoRects(spc,&info,ranges,nr,rects);TempFree(rects);return rv;
}

#define TEXT_BUFFER_SIZE 1024
#define FONT_ATLAS_SIZE 4672
#define MAX_GLYPHS 4096
int numPackedGlyphs=0,numPackedGlyphsStopD=0;
u32 fontAtlasTex,fontAtlasTexStopD;
stbtt_packedchar fontPackedChar[MAX_GLYPHS],fontPackedCharStopD[MAX_GLYPHS];
float fixedNumberAdvanceWidth=0.0f,fixedNumberAdvanceWidthStopD=0.0f;
static const char* fallbackFontPaths[]={"./Fonts/FreeSerifBold.ttf","./Fonts/cambriab.ttf","./Fonts/NotoSansCJK-Bold.ttc"}, *fontPaths[]={"./Fonts/SystemShockText.ttf","./Fonts/StopD.ttf"};
static stbtt_fontinfo fontInfo[5]; static unsigned char *fontData[5]; static char uiTextBuffer[TEXT_BUFFER_SIZE];
typedef struct{char*path;unsigned char*data;size_t size;stbtt_fontinfo info;}LoadedFont;
LoadedFont fallbackFonts[3];
typedef struct{i32 first,count,startIndex;}GlyphRange;
GlyphRange fontRanges[]     ={{0x0020,0x7E - 0x20 + 1,0},{0x00A0,0xFF - 0xA0 + 1,95},{0x0400,0x04FF - 0x0400 + 1,95+96},{0x3040,0x30FF - 0x3040 + 1,95+96+256}};
GlyphRange fontRangesStopD[]={{0x0020,0x7E - 0x20 + 1,0},{0x00A0,0xFF - 0xA0 + 1,95},{0x0400,0x04FF - 0x0400 + 1,95+96},{0x3040,0x30FF - 0x3040 + 1,95+96+256}};
i32 numFontRanges=sizeof(fontRanges)/sizeof(fontRanges[0]);
__attribute__((pure)) i32 CodepointToPackedIndex(i32 cp,int fontID){
    if(cp<32)cp=32;if(cp>=447)cp=446;
    const GlyphRange*ranges=(fontID==FONT_STOPD)?fontRangesStopD:fontRanges;
    i32 total=(fontID==FONT_STOPD)?numPackedGlyphsStopD:numPackedGlyphs;
    for(i32 i=0;i<numFontRanges;i++){if(cp>=ranges[i].first&&cp<ranges[i].first+ranges[i].count){i32 idx=ranges[i].startIndex+vmax((cp-ranges[i].first),0);if(idx<total)return idx;}}
    return 0;
}
static LoadedFont LoadFallbackFont(const char*path,int fii,int ci){
    FHandle fd;int fsz;fontData[fii]=OS_OpenAndAllocateFileBufferReadonly(path,&fd,&fsz);
    int off=stbtt_GetFontOffsetForIndex(fontData[fii],ci);if(off<0){DualLogError("Invalid collection index %d for font %s\n",ci,path);OS_Exit(1);}
    if(!stbtt_InitFont_internal(&fontInfo[fii],fontData[fii],off)){DualLogError("Failed to init font at index %d in %s\n",ci,path);OS_Exit(1);}
    return (LoadedFont){(char*)path,fontData[fii],fsz,fontInfo[fii]};
}
static int GetGlyphAndFont(u32 cp,stbtt_fontinfo**outFont,u8 fontID){
    int g=stbtt_FindGlyphIndex(fontID==FONT_STOPD?&fontInfo[1]:&fontInfo[0],cp);if(g){*outFont=fontID==FONT_STOPD?&fontInfo[1]:&fontInfo[0];return g;}
    for(int i=0;i<3;i++){g=stbtt_FindGlyphIndex(&fallbackFonts[i].info,cp);if(g){*outFont=&fallbackFonts[i].info;return g;}}
    return 0;
}

static void GenerateAndBindTexture(u32 *id, i32 internalFormat, i32 width, i32 height, u32 format, u32 type, i32 filt, unsigned char* bmp);
static void InitFontAtlasses(){
    DebugRAM("start font load");
    double t0=get_time();DualLog("Loading    5 fonts...");
    ttAllocs = OS_Alloc(4474 * sizeof(TAlloc));
    FHandle fd1,fd2;int sz1,sz2;
    fontData[0]=OS_OpenAndAllocateFileBufferReadonly(fontPaths[0],&fd1,&sz1);
    fontData[1]=OS_OpenAndAllocateFileBufferReadonly(fontPaths[1],&fd2,&sz2);
    if(!stbtt_InitFont_internal(&fontInfo[0],fontData[0],0)){DualLogError("%s font init failed\n",fontPaths[0]);OS_Exit(1);}
    if(!stbtt_InitFont_internal(&fontInfo[1],fontData[1],0)){DualLogError("%s font init failed\n",fontPaths[1]);OS_Exit(1);}
    fallbackFonts[0]=LoadFallbackFont(fallbackFontPaths[0],2,0);
    fallbackFonts[1]=LoadFallbackFont(fallbackFontPaths[1],3,0);
    fallbackFonts[2]=LoadFallbackFont(fallbackFontPaths[2],4,2);
    unsigned char*bmp=OS_Alloc(FONT_ATLAS_SIZE*FONT_ATLAS_SIZE); // Primary atlas
    stbtt_pack_context pc;stbtt_PackBegin(&pc,bmp,FONT_ATLAS_SIZE,FONT_ATLAS_SIZE,0,16,NULL);pc.h_oversample=3;pc.v_oversample=3;pc.skip_missing=1;numPackedGlyphs=0;
    for(int r=0;r<numFontRanges;++r){fontRanges[r].startIndex=numPackedGlyphs;
        for(int i=0;i<fontRanges[r].count;++i){if(numPackedGlyphs>=MAX_GLYPHS)break;u32 cp=fontRanges[r].first+i;stbtt_fontinfo*font=&fontInfo[0];unsigned char*data=fontData[0];
            int g=stbtt_FindGlyphIndex(font,cp);if(!g){g=GetGlyphAndFont(cp,&font,FONT_NORMAL);if(!g)continue;data=(font==&fontInfo[0])?fontData[0]:((LoadedFont*)((char*)font-__builtin_offsetof(LoadedFont,info)))->data;}
            float h=20.0f;if(font!=&fontInfo[0])h*=1.2f;FPackRange range={h,cp,NULL,1,&fontPackedChar[numPackedGlyphs],0,0};stbtt_PackFontRanges(&pc,data,0,&range,1);
            int idx=numPackedGlyphs++;if(cp>='0'&&cp<='9')fixedNumberAdvanceWidth=vmax(fixedNumberAdvanceWidth,fontPackedChar[idx].xadvance);}}
    TempFree(pc.pack_info);GenerateAndBindTexture(&fontAtlasTex,0x8229/*GL_R8*/,FONT_ATLAS_SIZE,FONT_ATLAS_SIZE,0x1903/*GL_RED*/,GL_UNSIGNED_BYTE,0x2601/*GL_LINEAR*/,bmp);
    MemSetToVForNBytes(bmp,0,FONT_ATLAS_SIZE*FONT_ATLAS_SIZE); // Secondary atlas
    stbtt_pack_context pc2;stbtt_PackBegin(&pc2,bmp,FONT_ATLAS_SIZE,FONT_ATLAS_SIZE,0,16,NULL);pc2.h_oversample=3;pc2.v_oversample=3;pc2.skip_missing=1;numPackedGlyphsStopD=0;
    for(int r=0;r<numFontRanges;++r){fontRangesStopD[r].startIndex=numPackedGlyphsStopD;
        for(int i=0;i<fontRangesStopD[r].count;++i){if(numPackedGlyphsStopD>=MAX_GLYPHS)break;u32 cp=fontRangesStopD[r].first+i;stbtt_fontinfo*font=&fontInfo[1];unsigned char*data=fontData[1];
            int g=stbtt_FindGlyphIndex(font,cp);if(!g){g=GetGlyphAndFont(cp,&font,FONT_STOPD);if(!g)continue;data=(font==&fontInfo[0])?fontData[0]:((LoadedFont*)((char*)font-__builtin_offsetof(LoadedFont,info)))->data;}
            float h=54.0f;if(font!=&fontInfo[1])h*=1.2f;FPackRange range={h,cp,NULL,1,&fontPackedCharStopD[numPackedGlyphsStopD],0,0};stbtt_PackFontRanges(&pc2,data,0,&range,1);
            int idx=numPackedGlyphsStopD++;if(cp>='0'&&cp<='9')fixedNumberAdvanceWidthStopD=vmax(fixedNumberAdvanceWidthStopD,fontPackedCharStopD[idx].xadvance);}}
    TempFree(pc2.pack_info);GenerateAndBindTexture(&fontAtlasTexStopD,0x8229/*GL_R8*/,FONT_ATLAS_SIZE,FONT_ATLAS_SIZE,0x1903/*GL_RED*/,GL_UNSIGNED_BYTE,0x2601/*GL_LINEAR*/,bmp);
    OS_DeallocateRAM(bmp,FONT_ATLAS_SIZE*FONT_ATLAS_SIZE);
    OS_DeallocateRAM(fontData[0],sz1);
    OS_DeallocateRAM(fontData[1],sz2);
    OS_DeallocateRAM(fontData[2],fallbackFonts[0].size);
    OS_DeallocateRAM(fontData[3],fallbackFonts[1].size);
    OS_DeallocateRAM(fontData[4],fallbackFonts[2].size);
    OS_DeallocateRAM(ttAllocs,4474 * sizeof(TAlloc));
    DebugRAM("after font load");
    glUseProgram(Sys_Render.textShaderProgram); glUniform1i(1,2);
    DualLog(" took %f s\n",get_time()-t0);
}

char *strncpy(char*dest,const char*src,size_t n); u16 logImages=1272;
size_t utf16le_to_utf8(const u8*src,size_t slen,char*dst,size_t dlen){
    size_t dp=0,sp=0;
    while(sp<slen&&dp<dlen-4){if(sp+1>=slen)break;u32 c=(u32)src[sp+1]<<8|src[sp];sp+=2;
        if(c<0x80){dst[dp++]=(char)c;}
        else if(c<0x800){dst[dp++]=(char)(0xC0|(c>>6));dst[dp++]=(char)(0x80|(c&0x3F));}
        else if(c<0x10000){dst[dp++]=(char)(0xE0|(c>>12));dst[dp++]=(char)(0x80|((c>>6)&0x3F));dst[dp++]=(char)(0x80|(c&0x3F));}
        else continue;}
    dst[dp]='\0';return dp;
}

static const char* localizations[8]={"./Data/text_english.txt","./Data/text_espanol.txt","./Data/text_deutsch.txt","./Data/text_francais.txt","./Data/text_nihongo.txt","./Data/text_russkiy.txt","./Data/text_italiano.txt","./Data/text_portugues.txt"};
void LoadTextForLanguage(u8 lang){
    char tf[256]={0};strncpy(tf,localizations[lang<8?lang:0],255);
    FHandle dfd=INVALID_FHANDLE;int asz=0;
    if(Sys_Text.file_data){OS_DeallocateRAM(Sys_Text.file_data,Sys_Text.file_size);Sys_Text.file_data=NULL;Sys_Text.file_size=0;}
    Sys_Text.file_data=(u8*)OS_OpenAndAllocateFileBufferReadonly(tf,&dfd,&asz);if(!Sys_Text.file_data||asz<=0){DualLogError("Failed to load text file: %s\n",tf);return;}
    Sys_Text.file_size=(size_t)asz;
    size_t dp=0;int utf16=0;
    if(Sys_Text.file_size>=2&&Sys_Text.file_data[0]==0xFF&&Sys_Text.file_data[1]==0xFE){dp=2;utf16=1;}
    else if(Sys_Text.file_size>=3&&Sys_Text.file_data[0]==0xEF&&Sys_Text.file_data[1]==0xBB&&Sys_Text.file_data[2]==0xBF){dp=3;}
    else{size_t nl=0;for(size_t i=1;i<Sys_Text.file_size&&i<1024;i+=2)if(Sys_Text.file_data[i]==0)nl++;if(nl*3>Sys_Text.file_size)utf16=1;}
    char line[TEXT_LOCALIZATION_MAX_LENGTH];int ln=0;
    while(dp<Sys_Text.file_size){size_t ls=dp;
        if(utf16){while(dp+1<Sys_Text.file_size){u16 ch=Sys_Text.file_data[dp]|(Sys_Text.file_data[dp+1]<<8);dp+=2;if(ch=='\r'||ch=='\n'){if(ch=='\r'&&dp+1<Sys_Text.file_size){u16 nx=Sys_Text.file_data[dp]|(Sys_Text.file_data[dp+1]<<8);if(nx=='\n')dp+=2;}break;}}}
        else{while(dp<Sys_Text.file_size){u8 c=Sys_Text.file_data[dp];if(c=='\r'||c=='\n'){if(c=='\r'&&dp+1<Sys_Text.file_size&&Sys_Text.file_data[dp+1]=='\n')++dp;++dp;break;}++dp;}}
        size_t ll=dp-ls;if(ll==0){if(ln<TEXT_STRING_COUNT)Sys_Text.stringTable[ln][0]='\0';++ln;continue;}
        if(utf16)utf16le_to_utf8(&Sys_Text.file_data[ls],ll,line,sizeof(line));else{if(ll>=sizeof(line))ll=sizeof(line)-1; MemCpyFromBtoAForNBytes(line,&Sys_Text.file_data[ls],ll);line[ll]='\0';}
        size_t sl=GetStringLength(line);while(sl>0&&(line[sl-1]=='\r'||line[sl-1]=='\n'))line[--sl]='\0';
        if(sl==0){if(ln<TEXT_STRING_COUNT)Sys_Text.stringTable[ln][0]='\0';++ln;continue;}
        if(ln<TEXT_STRING_COUNT) {MemCpyFromBtoAForNBytes(Sys_Text.stringTable[ln],line,sl);Sys_Text.stringTable[ln][sl]='\0';++ln;} }
}

static inline __attribute__((always_inline)) int StringToIntLen(const char*str,size_t len){int v=0;for(size_t i=0;i<len&&str[i]>='0'&&str[i]<='9';++i)v=v*10+(str[i]-'0');return v;}
static const char* logLocalizations[8]={"./Data/logs_text_english.txt","./Data/logs_text_espanol.txt","./Data/logs_text_deutsch.txt","./Data/logs_text_francais.txt","./Data/logs_text_nihongo.txt","./Data/logs_text_russkiy.txt","./Data/logs_text_italiano.txt","./Data/logs_text_portugues.txt"};
void LoadLogTextForLanguage(u8 lang){
    MemSetToVForNBytes(Sys_Text.audioLogImagesRefIndicesLH,0,TEXT_LOGS_COUNT*sizeof(u16));MemSetToVForNBytes(Sys_Text.audioLogImagesRefIndicesRH,0,TEXT_LOGS_COUNT*sizeof(u16));MemSetToVForNBytes(Sys_Text.audioLogType,0,TEXT_LOGS_COUNT*sizeof(u8));MemSetToVForNBytes(Sys_Text.audioLogLevelFound,0,TEXT_LOGS_COUNT*sizeof(u8));
    char tf[256]={0};strncpy(tf,logLocalizations[lang<8?lang:0],255);
    FHandle dfd=INVALID_FHANDLE;int asz=0;
    if(Sys_Text.filelog_data){OS_DeallocateRAM(Sys_Text.filelog_data,Sys_Text.filelog_size);Sys_Text.filelog_data=NULL;Sys_Text.filelog_size=0;}
    Sys_Text.filelog_data=(u8*)OS_OpenAndAllocateFileBufferReadonly(tf,&dfd,&asz);if(!Sys_Text.filelog_data||asz<=0){DualLogError("Failed to load log text file: %s\n",tf);return;}
    Sys_Text.filelog_size=(size_t)asz;
    size_t dp=0;int utf16=0;
    if(Sys_Text.filelog_size>=2&&Sys_Text.filelog_data[0]==0xFF&&Sys_Text.filelog_data[1]==0xFE){dp=2;utf16=1;}
    else if(Sys_Text.filelog_size>=3&&Sys_Text.filelog_data[0]==0xEF&&Sys_Text.filelog_data[1]==0xBB&&Sys_Text.filelog_data[2]==0xBF){dp=3;}
    else{int nl=0;for(size_t i=1;i<Sys_Text.filelog_size&&i<2048;i+=2)if(Sys_Text.filelog_data[i]==0)nl++;if(nl>(int)(Sys_Text.filelog_size/5))utf16=1;}
    char line[1024];
    while(dp<Sys_Text.filelog_size){size_t ls=dp;
        if(utf16){while(dp+1<Sys_Text.filelog_size){u16 ch=Sys_Text.filelog_data[dp]|(Sys_Text.filelog_data[dp+1]<<8);dp+=2;if(ch=='\r'||ch=='\n'){if(ch=='\r'&&dp+1<Sys_Text.filelog_size){u16 nx=Sys_Text.filelog_data[dp]|(Sys_Text.filelog_data[dp+1]<<8);if(nx=='\n')dp+=2;}break;}}}
        else{while(dp<Sys_Text.filelog_size){u8 c=Sys_Text.filelog_data[dp];if(c=='\r'||c=='\n'){if(c=='\r'&&dp+1<Sys_Text.filelog_size&&Sys_Text.filelog_data[dp+1]=='\n')++dp;++dp;break;}++dp;}}
        size_t ll=dp-ls;if(!ll)continue;
        if(utf16)utf16le_to_utf8(&Sys_Text.filelog_data[ls],ll,line,sizeof(line));else{if(ll>=sizeof(line))ll=sizeof(line)-1;MemCpyFromBtoAForNBytes(line,&Sys_Text.filelog_data[ls],ll);line[ll]='\0';}
        size_t sl=GetStringLength(line);while(sl>0&&(line[sl-1]=='\r'||line[sl-1]=='\n'))line[--sl]='\0';if(!sl)continue;
        int li=-1,ilh=-1,irh=-1,lt=0,lf=0,fi=0;char*pos=line;
        while(*pos&&fi<32){while(*pos==' ')++pos;char*st=pos;int q=(*pos=='"');if(q)++pos;while(*pos){if(*pos==','&&!q)break;if(*pos=='"'&&q){if(pos[1]==','){pos++;break;}if(pos[1]=='"'){pos+=2;continue;}}++pos;}char*en=pos;if(q&&*en=='"')--en;size_t tl=(size_t)(en-st);if(!tl){if(*pos==',')++pos;fi++;continue;}
            switch(fi){case 0:li=StringToIntLen(st,tl);if(li<0||li>=TEXT_LOGS_COUNT)goto nxt;break;case 1:ilh=StringToIntLen(st,tl);break;case 2:irh=StringToIntLen(st,tl);break;case 3:if(li>=0&&li<TEXT_LOGS_COUNT)StringCopyInto_A_SubstringFrom_B(Sys_Global.audiologNames[li],tl,st,sizeof(Sys_Global.audiologNames[0]));break;case 4:if(li>=0&&li<TEXT_LOGS_COUNT)StringCopyInto_A_SubstringFrom_B(Sys_Global.audiologSenders[li],tl,st,sizeof(Sys_Global.audiologSenders[0]));break;case 5:if(li>=0&&li<TEXT_LOGS_COUNT)StringCopyInto_A_SubstringFrom_B(Sys_Global.audiologSubjects[li],tl,st,sizeof(Sys_Global.audiologSubjects[0]));break;case 6:lt=StringToIntLen(st,tl);break;case 7:lf=StringToIntLen(st,tl);break;default:if(li>=0&&li<TEXT_LOGS_COUNT){char*d=Sys_Global.audioLogSpeech2Text[li];size_t cur=GetStringLength(d);if(cur>0&&cur<TEXT_LOCALIZATION_MAX_LENGTH*4-2){d[cur++]=',';d[cur]='\0';}size_t left=TEXT_LOCALIZATION_MAX_LENGTH*4-cur-1;if(left>0){size_t cl=tl>left?left:tl;StringCopyInto_A_SubstringFrom_B(d+cur,cl,st,left+1);}}break;}
            if(*pos==',')++pos;fi++;}
        if(li>=0&&li<TEXT_LOGS_COUNT){Sys_Text.audioLogImagesRefIndicesLH[li]=(u16)ilh;Sys_Text.audioLogImagesRefIndicesRH[li]=(u16)irh;Sys_Text.audioLogType[li]=(u8)lt;Sys_Text.audioLogLevelFound[li]=(u8)lf;}
        nxt:continue;}
}

Color textColors[] = {{        1.0f,        1.0f,        1.0f,1.0f},/* 0 White                TEXT_WHITE*/       {0.890196078f,0.874509804f,        0.0f,1.0f},/* 1 Yellow                   TEXT_YELLOW*/              {0.623529412f,0.611764706f,        0.0f,1.0f}, /* 2 Dark Yellow (Yellow * 0.7f) TEXT_DARK_YELLOW*/
                      {0.372549020f,0.654901961f,0.168627451f,1.0f},/* 3 Green                TEXT_GREEN*/       {0.917647059f,0.137254902f,0.168627451f,1.0f},/* 4 Red                      TEXT_RED*/                 {        1.0f,0.498039216f,        0.0f,1.0f}, /* 5 Orange                      TEXT_ORANGE*/
                      {0.674509804f,0.058823529f,0.070588235f,1.0f},/* 6 StopD Red            TEXT_STOPD_RED*/   {0.941176471f,0.282352941f,0.298039216f,1.0f},/* 7 StopD Red Highlight      TEXT_STOPD_RED_HIGHLIGHT*/ {0.909803922f,0.203921569f,0.219607843f,1.0f}, /* 8 StopD Red Pause Title       TEXT_STOPD_RED_PAUSETITLE*/
                      {0.470588235f,0.721568627f,0.172549020f,1.0f},/* 9 Green Menu Title     TEXT_GREEN_MENU*/  {0.137254902f,0.356862745f,0.109803922f,1.0f},/* 10 Green Menu Title Shadow TEXT_GREEN_MENU_SHADOW*/   {0.239215686f,0.466666667f,0.129411765f,1.0f}, /* 11 Green Menu Title Glow      TEXT_GREEN_MENU_GLOW*/
                      {0.392156863f,0.031372549f,0.039215686f,1.0f}  /* 12 Red Menu Text Dark TEXT_RED_MENU*/ };
float textVertexData[8192];
void RenderFormattedText(i16 x,i16 y,u32 color,u8 fontID,float scaleInput,const char* restrict format,...) {
    va_list args; __builtin_va_start(args,format); StringFormatV(uiTextBuffer,TEXT_BUFFER_SIZE,format,args); __builtin_va_end(args);
    glUseProgram(Sys_Render.textShaderProgram);
    glEnable(GL_BLEND);
    glUniform4f(3,textColors[color].r,textColors[color].g,textColors[color].b,1.0f);
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D,fontID==FONT_STOPD ? fontAtlasTexStopD : fontAtlasTex);
    float invatsz = 1.0f/(float)FONT_ATLAS_SIZE;
    glUniform2f(4,invatsz,invatsz); glUniform1ui(2,fontID);
    glBindVertexArray(Sys_Render.textVAO);
    float scale=scaleInput;
    size_t vc=0; const char*p=uiTextBuffer; float xpos=x,ypos=y+(16*scale),ls=22*scale; aligned_quad q; int cc=0;
    float puv = 10.0f * invatsz, bw=2.0f;
    while(*p) {
        const unsigned char*s=(const unsigned char*)p; u32 cp=0;
        if (*s<0x80) { cp=*s++; }
        else if ((*s&0xE0)==0xC0) { cp=(*s&0x1F)<< 6; cp|=(s[1]&0x3F); s+=2; }
        else if ((*s&0xF0)==0xE0) { cp=(*s&0x0F)<<12; cp|=(s[1]&0x3F)<<6;  cp|=(s[2]&0x3F); s+=3; }
        else if ((*s&0xF8)==0xF0) { cp=(*s&0x07)<<18; cp|=(s[1]&0x3F)<<12; cp|=(s[2]&0x3F)<<6; cp|=(s[3]&0x3F); s+=4; }
        else s++;

        p = (const char*)s; cc++;
        if (cp=='\n'||cc>120) { xpos=x; ypos+=ls; cc=0; continue; }
        int idx=CodepointToPackedIndex(cp,fontID);
        stbtt_GetPackedQuad((fontID==FONT_STOPD) ? fontPackedCharStopD : fontPackedChar,FONT_ATLAS_SIZE,FONT_ATLAS_SIZE,idx,&xpos,&ypos,&q,1);
        float vx0=q.x0*scale-bw,vy0=q.y0*scale-bw,vx1=q.x1*scale+bw,vy1=q.y1*scale+bw;
        float s0=q.s0-puv,t0=q.t0-puv,s1=q.s1+puv,t1=q.t1+puv,z=0.0f;
        float tv[30] = { vx0,vy0,z,s0,t0,vx1,vy1,z,s1,t1,vx1,vy0,z,s1,t0,vx0,vy0,z,s0,t0,vx0,vy1,z,s0,t1,vx1,vy1,z,s1,t1 };
        MemCpyFromBtoAForNBytes(textVertexData+vc*30,tv,sizeof(tv)); vc++;
        if (cp>='0' && cp<='9') {
            if (fontID == FONT_STOPD) { xpos=q.x0 + ((fontID == FONT_STOPD) ? fixedNumberAdvanceWidthStopD : fixedNumberAdvanceWidth); }
        }
    }
    
    if(vc){ glBindBuffer(GL_ARRAY_BUFFER,Sys_Render.textVBO); glBufferData(GL_ARRAY_BUFFER,vc*30*sizeof(float),textVertexData,GL_DYNAMIC_DRAW); glDrawArrays(0x0004/*GL_TRIANGLES*/,0,vc*6); }
}
// ========================== Windowing and Input System
typedef void (*GLFWglproc)(void);
typedef struct GLFWwindow GLFWwindow; GLFWwindow* window;
typedef struct { int width,height,redBits,greenBits,blueBits,refreshRate; } GLFWvidmode;
typedef struct { unsigned char buttons[15]; float axes[6]; } GLFWgamepadstate;
typedef void (*GLFWproc)(void); typedef struct _GLFWfbconfig _GLFWfbconfig; typedef struct _GLFWcontext _GLFWcontext; typedef struct _GLFWwindow _GLFWwindow; typedef struct _GLFWlibrary _GLFWlibrary; typedef struct _GLFWmonitor _GLFWmonitor; typedef struct _GLFWjoystick _GLFWjoystick;
void UpdateScreenSize(i32 width, i32 height); void SaveConfig();
struct _GLFWfbconfig { int redBits,greenBits,blueBits,alphaBits,depthBits,stencilBits,accumRedBits,accumGreenBits,accumBlueBits,accumAlphaBits; i32 samples,stereo,sRGB,doublebuffer; uintptr_t handle; };
extern _GLFWlibrary _glfw;
GLFWproc PlatformGetModuleSymbol(void* module, const char* name);
void InputWindowFocus(i32 focused); void InputKey(_GLFWwindow* window, int key, int action); void InputMouseClick(_GLFWwindow* window, int button, int action);
void InputCursorPos(_GLFWwindow* window, double xpos, double ypos);  void JoystickConnection(_GLFWjoystick* js, int event);         void InputJoystickAxis(_GLFWjoystick* js, int axis, float value);
void InputJoystickButton(_GLFWjoystick* js, int button, char value); void InputJoystickHat(_GLFWjoystick* js, int hat, char value); void InputMonitor(_GLFWmonitor* monitor, int action, int placement);
const _GLFWfbconfig* ChooseFBConfig(const _GLFWfbconfig* alternatives, unsigned int count);
_GLFWmonitor* AllocMonitor(const char* name, int widthMM, int heightMM); _GLFWjoystick* _glfwAllocJoystick(const char* name, const char* guid, int axisCount, int buttonCount, int hatCount);
void _glfwFreeJoystick(_GLFWjoystick* js);
#define INPUT_RELEASE 0
#define INPUT_PRESS   1
#define INPUT_REPEAT  2
#if defined(WINDOWS)
    #define MAKEWORD(a,b) ((u16) (((u8) (((u64) (a)) & 0xff)) | ((u16) ((u8) (((u64) (b)) & 0xff))) << 8))
    #define MAKELONG(a, b) ((i32) (((u16) (((u64) (a)) & 0xffff)) | ((u32) ((u16) (((u64) (b)) & 0xffff))) << 16))
    #define LOWORD(l) ((u16) (((u64) (l)) & 0xffff))
    #define HIWORD(l) ((u16) ((((u64) (l)) >> 16) & 0xffff))
    #define LOBYTE(w) ((u8) (((u64) (w)) & 0xff))
    #define HIBYTE(w) ((u8) ((((u64) (w)) >> 8) & 0xff))
    typedef struct HWND__ { int unused; } *HWND;   typedef struct HBITMAP__ { int unused; } *HBITMAP; typedef struct HBRUSH__ { int unused; } *HBRUSH; typedef struct HDC__ { int unused; } *HDC;
    typedef struct HGLRC__ { int unused; } *HGLRC; typedef struct HICON__ { int unused; } *HICON;     typedef struct HMENU__ { int unused; } *HMENU;   typedef struct HMONITOR__ { int unused; } *HMONITOR;
    typedef struct tagPOINT { i32 x,y; } POINT,*PPOINT,*NPPOINT,*LPPOINT;              typedef struct _POINTL { i32 x,y; } POINTL,*PPOINTL;
    typedef struct tagRECT { i32 left,top,right,bottom; } RECT,*PRECT,*NPRECT,*LPRECT; typedef struct tagSIZE { i32 cx,cy; } SIZE,*PSIZE,*LPSIZE;
    typedef struct _OSVERSIONINFOEXW { u32 dwOSVersionInfoSize,dwMajorVersion,dwMinorVersion,dwBuildNumber,dwPlatformId; u16 szCSDVersion[128]; u16 wServicePackMajor,wServicePackMinor,wSuiteMask; u8 wProductType,wReserved; } OSVERSIONINFOEXW;
    int __cdecl wcscmp(const u16 *_Str1,const u16 *_Str2); u16* wcscpy(u16* restrict destination, const u16* restrict source);
    #define MAKEINTATOM(i) (u16*)((u64)((u16)(i)))
    typedef struct _devicemodeW {
        u16 dmDeviceName[32]; u16 dmSpecVersion,dmDriverVersion,dmSize,dmDriverExtra; u32 dmFields;
        union { struct { i16 dmOrientation,dmPaperSize,dmPaperLength,dmPaperWidth,dmScale,dmCopies,dmDefaultSource,dmPrintQuality; }; struct { POINTL dmPosition; u32 dmDisplayOrientation,dmDisplayFixedOutput; }; };
        i16 dmColor,dmDuplex,dmYResolution,dmTTOption,dmCollate; u16 dmFormName[32],dmLogPixels; u32 dmBitsPerPel,dmPelsWidth,dmPelsHeight; union { u32 dmDisplayFlags,dmNup; };
        u32 dmDisplayFrequency,dmICMMethod,dmICMIntent,dmMediaType,dmDitherType,dmReserved1,dmReserved2,dmPanningWidth,dmPanningHeight;
    } DEVMODEW,*LPDEVMODEW;
    typedef i64 (__stdcall *WNDPROC)(HWND,u32,u64,i64); typedef i32 (__stdcall *MONITORENUMPROC)(HMONITOR,HDC,LPRECT,i64);
    typedef struct _ICONINFO { i32 fIcon; u32 xHotspot,yHotspot; HBITMAP hbmMask,hbmColor; } ICONINFO; typedef ICONINFO *PICONINFO;
    typedef struct tagMSG { HWND hwnd; u32 message; u64 wParam; i64 lParam; u32 time; POINT pt; } MSG,*PMSG,*NPMSG,*LPMSG;
    typedef struct tagMONITORINFO { u32 cbSize; RECT rcMonitor; RECT rcWork; u32 dwFlags; } MONITORINFO,*LPMONITORINFO;
    typedef struct tagMONITORINFOEXW { u32 cbSize; RECT rcMonitor; RECT rcWork; u32 dwFlags; u16 szDevice[32]; } MONITORINFOEXW;
    typedef struct tagWINDOWPLACEMENT { u32 length; u32 flags; u32 showCmd; POINT ptMinPosition; POINT ptMaxPosition; RECT rcNormalPosition; } WINDOWPLACEMENT;
    typedef struct tagWNDCLASSEXW { u32 cbSize,style; WNDPROC lpfnWndProc; i32 cbClsExtra,cbWndExtra; HINSTANCE hInstance; HICON hIcon,hCursor; HBRUSH hbrBackground; u16 *lpszMenuName,*lpszClassName; HICON hIconSm; } WNDCLASSEXW;
    typedef struct {u16 wButtons; u8 bLeftTrigger,bRightTrigger; i16 sThumbLX,sThumbLY,sThumbRX,sThumbRY; } XINPUT_GAMEPAD; typedef struct {u16 wLeftMotorSpeed, wRightMotorSpeed;} XINPUT_VIBRATION;
    typedef struct {u8 Type,SubType; u16 Flags; XINPUT_GAMEPAD Gamepad; XINPUT_VIBRATION Vibration;} XINPUT_CAPABILITIES;   typedef struct {u32 dwPacketNumber; XINPUT_GAMEPAD Gamepad;} XINPUT_STATE;
    typedef u32 (WINAPI * PFN_XInputGetCapabilities)(u32,u32,XINPUT_CAPABILITIES*);                                         typedef u32 (WINAPI * PFN_XInputGetState)(u32,XINPUT_STATE*);
    typedef struct { u32 dbch_size,dbch_devicetype,dbch_reserved; } DEV_BROADCAST_HDR;                                      typedef struct { u32 dbcc_size,dbcc_devicetype,dbcc_reserved; GUID dbcc_classguid; u16 dbcc_name[1]; } DEV_BROADCAST_DEVICEINTERFACE_W;
    typedef i32 (WINAPI * PFN_DwmIsCompositionEnabled)(i32*);                   typedef i32 (WINAPI * PFN_DwmFlush)();
    typedef i32 (WINAPI * PFN_RtlVerifyVersionInfo)(OSVERSIONINFOEXW*,u32,u64); typedef i32 (WINAPI * PFN_SWE)(int);
    typedef i32 (WINAPI * PFN_GPFAIVA)(HDC,int,int,u32,const int*,int*);        typedef HGLRC (WINAPI * FP_CCAA)(HDC,HGLRC,const int*);
    typedef HGLRC (WINAPI * PFN_CC)(HDC);                                       typedef PROC (WINAPI * PFN_wglGetProcAddress)(const char*);
    typedef HDC (WINAPI * PFN_wglGetCurrentDC)();                               typedef HGLRC (WINAPI * PFN_wglGetCurrentContext)();
    typedef i32 (WINAPI * PFN_wglMakeCurrent)(HDC,HGLRC);
    typedef struct WGLContext { HDC dc; HGLRC handle; int interval; } WGLContext;
    typedef struct _GLFWlibraryWGL { HINSTANCE instance; PFN_CC CreateContext; PFN_wglGetProcAddress GetProcAddress; PFN_wglGetCurrentDC GetCurrentDC; PFN_wglGetCurrentContext GetCurrentContext; PFN_wglMakeCurrent MakeCurrent; PFN_SWE SwapIntervalEXT; PFN_GPFAIVA GetPixelFormatAttribivARB; FP_CCAA CreateContextAttribsARB; } _GLFWlibraryWGL;
    typedef struct _GLFWwindowWin32 { HWND handle; i32 cursorTracked,frameAction,keymenu; int width,height,lastCursorPosX,lastCursorPosY; } _GLFWwindowWin32;
    typedef struct _GLFWlibraryWin32 { HINSTANCE instance; HWND helperWindowHandle; u16 helperWindowClass,mainWindowClass; void* deviceNotificationHandle; short int keycodes[512],scancodes[349]; double restoreCurPosX,restoreCurPosY; _GLFWwindow *disabledCursorWindow, *capturedCursorWindow; HICON blankCursor; struct {HINSTANCE instance; PFN_XInputGetCapabilities GetCapabilities; PFN_XInputGetState GetState;} xinput; struct {HINSTANCE instance; PFN_DwmIsCompositionEnabled IsCompositionEnabled; PFN_DwmFlush Flush;} dwmapi; struct {HINSTANCE instance; PFN_RtlVerifyVersionInfo RtlVerifyVersionInfo;} ntdll;} _GLFWlibraryWin32;
    typedef struct _GLFWmonitorWin32 { HMONITOR handle; u16 adapterName[32],displayName[32]; char publicAdapterName[32],publicDisplayName[32]; i32 modesPruned,modeChanged; } _GLFWmonitorWin32;
    typedef struct _GLFWjoystickWin32{ int objectCount; u32 index; GUID guid; } _GLFWjoystickWin32;
    typedef long FXPT2DOT30; typedef struct tagCIEXYZ { FXPT2DOT30 x,y,z; } CIEXYZ; typedef struct tagICEXYZTRIPLE {CIEXYZ r,g,b;} CIEXYZTRIPLE;
    typedef struct _DISPLAY_DEVICEW { u32 cb; u16 DeviceName[32],DeviceString[128]; u32 StateFlags; u16 DeviceID[128],DeviceKey[128]; } DISPLAY_DEVICEW,*PDISPLAY_DEVICEW,*LPDISPLAY_DEVICEW;
    typedef struct tagPIXELFORMATDESCRIPTOR { u16 nSize,nVersion; u32 dwFlags; u8 iPixelType,cColorBits,cRedBits,cRedShift,cGreenBits,cGreenShift,cBlueBits,cBlueShift,cAlphaBits,cAlphaShift,cAccumBits,cAccumRedBits,cAccumGreenBits,cAccumBlueBits,cAccumAlphaBits,cDepthBits,cStencilBits,cAuxBuffers,iLayerType,bReserved; u32 dwLayerMask,dwVisibleMask,dwDamageMask; } PIXELFORMATDESCRIPTOR,*PPIXELFORMATDESCRIPTOR,*LPPIXELFORMATDESCRIPTOR;
    typedef struct { u32 bV5Size; i32 bV5Width,bV5Height; u16 bV5Planes,bV5BitCount; u32 bV5Compression,bV5SizeImage; i32 bV5XPelsPerMeter; i32 bV5YPelsPerMeter; u32 bV5ClrUsed,bV5ClrImportant,bV5RedMask,bV5GreenMask,bV5BlueMask,bV5AlphaMask,bV5CSType; CIEXYZTRIPLE bV5Endpoints; u32 bV5GammaRed,bV5GammaGreen,bV5GammaBlue,bV5Intent,bV5ProfileData,bV5ProfileSize,bV5Reserved; } BITMAPV5HEADER,*LPBITMAPV5HEADER,*PBITMAPV5HEADER;
    typedef struct tagRGBQUAD { u8 rgbBlue,rgbGreen,rgbRed,rgbReserved; } RGBQUAD;
    typedef struct tagBITMAPINFOHEADER { u32 biSize; i32 biWidth,biHeight; u16 biPlanes,biBitCount; u32 biCompression; u32 biSizeImage; i32 biXPelsPerMeter; i32 biYPelsPerMeter; u32 biClrUsed; u32 biClrImportant; } BITMAPINFOHEADER,*LPBITMAPINFOHEADER,*PBITMAPINFOHEADER;
    typedef struct tagBITMAPINFO { BITMAPINFOHEADER bmiHeader; RGBQUAD bmiColors[1]; } BITMAPINFO,*LPBITMAPINFO,*PBITMAPINFO;
    DECLSPEC_IMPORT HICON WINAPI CreateIconIndirect(PICONINFO); DECLSPEC_IMPORT HDC WINAPI GetDC(HWND);                     DECLSPEC_IMPORT i32 WINAPI GetModuleHandleExW(u32,const u16*,HINSTANCE*);
    DECLSPEC_IMPORT int WINAPI ReleaseDC(HWND,HDC);             DECLSPEC_IMPORT i32 WINAPI SetCursorPos(int,int);           DECLSPEC_IMPORT int WINAPI WideCharToMultiByte(u32,u32,u16*,int,char*,int,const char*,i32*);
    DECLSPEC_IMPORT HICON WINAPI SetCursor(HICON);              DECLSPEC_IMPORT i32 WINAPI GetCursorPos(LPPOINT);           DECLSPEC_IMPORT int WINAPI MultiByteToWideChar(u32,u32,const char*,int,u16*,int);
    DECLSPEC_IMPORT i32 WINAPI ClipCursor(const RECT*);         DECLSPEC_IMPORT i32 WINAPI ClientToScreen(HWND,LPPOINT);    DECLSPEC_IMPORT HDC WINAPI CreateDCW(const u16*,const u16*,const u16*,const DEVMODEW*);
    DECLSPEC_IMPORT void* WINAPI GetPropW(HWND,u16*);           DECLSPEC_IMPORT i32 WINAPI GetMessageTime();                DECLSPEC_IMPORT i32 WINAPI GetClientRect(HWND,LPRECT); // Haha get rect!
    DECLSPEC_IMPORT HICON WINAPI LoadCursorW(HINSTANCE,u16*);   DECLSPEC_IMPORT u32 WINAPI MapVirtualKeyW(u32,u32);         DECLSPEC_IMPORT i32 WINAPI SetWindowPos(HWND,HWND,int,int,int,int,u32);    
    DECLSPEC_IMPORT HWND WINAPI SetCapture(HWND hWnd);          DECLSPEC_IMPORT i32 WINAPI ReleaseCapture();                DECLSPEC_IMPORT i32 WINAPI PeekMessageW(LPMSG,HWND,u32,u32,u32);
    DECLSPEC_IMPORT i32 WINAPI AdjustWindowRect(LPRECT,u32,i32);DECLSPEC_IMPORT i32 WINAPI GetWindowLongW(HWND,int);        DECLSPEC_IMPORT i64 WINAPI DefWindowProcW(HWND,u32,u64,i64);
    DECLSPEC_IMPORT HMONITOR WINAPI MonitorFromWindow(HWND,u32);DECLSPEC_IMPORT HWND WINAPI GetActiveWindow();              DECLSPEC_IMPORT i32 WINAPI AdjustWindowRectEx(LPRECT,u32,i32,u32);
    DECLSPEC_IMPORT i64 WINAPI SendMessageW(HWND,u32,u64,i64);  DECLSPEC_IMPORT i32 WINAPI SetWindowLongW(HWND,int,i32);    DECLSPEC_IMPORT i32 WINAPI GetMonitorInfoW(HMONITOR,LPMONITORINFO);
    DECLSPEC_IMPORT i32 WINAPI TranslateMessage(const MSG*);    DECLSPEC_IMPORT i16 WINAPI GetKeyState(int);                DECLSPEC_IMPORT i64 WINAPI DispatchMessageW(const MSG*);
    DECLSPEC_IMPORT i32 WINAPI ShowWindow(HWND,int);            DECLSPEC_IMPORT i32 WINAPI BringWindowToTop(HWND);          DECLSPEC_IMPORT i32 WINAPI SetWindowPlacement(HWND,const WINDOWPLACEMENT*);
    DECLSPEC_IMPORT HWND WINAPI SetFocus(HWND);                 DECLSPEC_IMPORT i32 WINAPI SetForegroundWindow(HWND);       DECLSPEC_IMPORT i32 WINAPI GetWindowPlacement(HWND,WINDOWPLACEMENT*);
    DECLSPEC_IMPORT i32 WINAPI SetPropW(HWND,u16*,void*);       DECLSPEC_IMPORT i32 WINAPI OffsetRect(LPRECT,int,int);      DECLSPEC_IMPORT HWND WINAPI CreateWindowExW(u32,u16*,u16*,u32,int,int,int,int,HWND,HMENU,HINSTANCE,void*);
    DECLSPEC_IMPORT u64 WINAPI VerSetConditionMask(u64,u32,u8); DECLSPEC_IMPORT HDC WINAPI wglGetCurrentDC();               DECLSPEC_IMPORT u16 WINAPI RegisterClassExW(const WNDCLASSEXW *);
    DECLSPEC_IMPORT i32 WINAPI DeleteObject(void*);             DECLSPEC_IMPORT i32 WINAPI DeleteDC(HDC);                   DECLSPEC_IMPORT void* WINAPI RegisterDeviceNotificationW(void*,void*,u32);
    DECLSPEC_IMPORT i32 WINAPI SwapBuffers(HDC);                DECLSPEC_IMPORT HGLRC WINAPI wglGetCurrentContext();        DECLSPEC_IMPORT i32 WINAPI EnumDisplayMonitors(HDC,const RECT*,MONITORENUMPROC,i64);
    DECLSPEC_IMPORT i32 WINAPI wglMakeCurrent(HDC,HGLRC);       DECLSPEC_IMPORT PROC WINAPI wglGetProcAddress(const char*); DECLSPEC_IMPORT i32 WINAPI EnumDisplaySettingsW(u16*,u32,LPDEVMODEW); 
    DECLSPEC_IMPORT i32 WINAPI EnumDisplayDevicesW(u16*,u32,PDISPLAY_DEVICEW,u32);               DECLSPEC_IMPORT i32 WINAPI EnumDisplaySettingsExW(u16*,u32,LPDEVMODEW,u32);
    DECLSPEC_IMPORT i32 WINAPI SetPixelFormat(HDC,i32,const PIXELFORMATDESCRIPTOR *);            DECLSPEC_IMPORT i32 WINAPI ChoosePixelFormat(HDC hdc,const PIXELFORMATDESCRIPTOR *ppfd);
    DECLSPEC_IMPORT i32 WINAPI DescribePixelFormat(HDC,i32,u32,LPPIXELFORMATDESCRIPTOR);         DECLSPEC_IMPORT HBITMAP WINAPI CreateBitmap(i32,i32,u32,u32,const void *);
    DECLSPEC_IMPORT HBITMAP WINAPI CreateDIBSection(HDC,const BITMAPINFO*,u32,void**,void*,u32); DECLSPEC_IMPORT i32 WINAPI GetDeviceCaps(HDC,i32);
    u16* CreateWideStringFromUTF8Win32(const char* source); i32 IsWindowsVersionOrGreaterWin32(u16 major, u16 minor, u16 sp); void _glfwPollMonitorsWin32();
    struct _GLFWjoystick { i32 allocated,connected; size_t axesSize,buttonsSize,hatsSize; float*  axes; int axisCount; unsigned char* buttons; int buttonCount; unsigned char* hats; int hatCount; char name[128],guid[33]; _GLFWjoystickWin32 win32; };
    struct _GLFWlibrary { _GLFWmonitor** monitors; int monitorCount; i32 joysticksInitialized; _GLFWjoystick joysticks[JOYSTICK_LAST + 1]; _GLFWlibraryWin32 win32; _GLFWlibraryWGL wgl; };
    struct _GLFWcontext { int client,source,major,minor; PFNGLGETINTEGERV GetIntegerv; void (*makeCurrent)(_GLFWwindow*); void (*swapBuffers)(_GLFWwindow*); void (*swapInterval)(int); GLFWglproc (*getProcAddress)(const char*); WGLContext wgl; };
    struct _GLFWwindow { i32 decorated,doublebuffer; GLFWvidmode videoMode; int cursorMode; char mouseButtons[8],keys[349]; double virtualCursorPosX,virtualCursorPosY; _GLFWcontext context; _GLFWwindowWin32 win32; };
    struct _GLFWmonitor { char name[128]; int widthMM,heightMM; GLFWvidmode currentMode; _GLFWmonitorWin32 win32; };
    static u32 getWindowStyle(const _GLFWwindow* w) { return 0x060A0000 | (w->decorated ? 0x00C00000 : 0x80000000); } // clipping,sysmenu,minimize,title,border,and borderless raw
    static HICON createIcon(const GLFWimage* image,int xhot,int yhot,i32 icon) {
        HDC dc; HICON handle; HBITMAP color,mask; BITMAPV5HEADER bi; ICONINFO ii;
        unsigned char* target=NULL; unsigned char* source=image->pixels;
        MemSetToVForNBytes(&bi,0,sizeof(bi));
        bi.bV5Size=sizeof(bi); bi.bV5Width=image->width; bi.bV5Height=-image->height; bi.bV5Planes=1; bi.bV5BitCount=32; bi.bV5Compression=3; bi.bV5RedMask=0x00ff0000; bi.bV5GreenMask=0x0000ff00; bi.bV5BlueMask=0x000000ff; bi.bV5AlphaMask=0xff000000;
        dc=GetDC(NULL);
        color=CreateDIBSection(dc,(BITMAPINFO*)&bi,0,(void**)&target,NULL,(u32)0U);
        ReleaseDC(NULL,dc);
        mask=CreateBitmap(image->width,image->height,1,1,NULL);
        for (int i=0;i<image->width*image->height;i++) { target[0]=source[2]; target[1]=source[1]; target[2]=source[0]; target[3]=source[3]; target+=4; source+=4; }
        MemSetToVForNBytes(&ii,0,sizeof(ii));
        ii.fIcon=icon; ii.xHotspot=xhot; ii.yHotspot=yhot; ii.hbmMask=mask; ii.hbmColor=color;
        handle=CreateIconIndirect(&ii); DeleteObject(color); DeleteObject(mask); return handle;
    }

    static void updateCursorImage(_GLFWwindow* win) { if (win->cursorMode==0x00034001/*GLFW_CURSOR_NORMAL*/) {SetCursor(LoadCursorW(NULL,(u16*)((u64)(u16)32512)));} else {SetCursor(_glfw.win32.blankCursor);} }
    static void captureCursor(_GLFWwindow* win) { RECT clipRect; GetClientRect(win->win32.handle,&clipRect); ClientToScreen(win->win32.handle,(POINT*)&clipRect.left); ClientToScreen(win->win32.handle,(POINT*)&clipRect.right); ClipCursor(&clipRect); _glfw.win32.capturedCursorWindow=win; }
    static void releaseCursor() { ClipCursor(NULL); _glfw.win32.capturedCursorWindow=NULL; }
    static void disableCursor(_GLFWwindow* win) { _glfw.win32.disabledCursorWindow = win; POINT pos; GetCursorPos(&pos); _glfw.win32.restoreCurPosX = pos.x; _glfw.win32.restoreCurPosY = pos.y; updateCursorImage(win); captureCursor(win); }
    static void SetCursorPosV(_GLFWwindow* win, double xpos, double ypos) { win->win32.lastCursorPosX = (int)xpos; win->win32.lastCursorPosY = (int)ypos; POINT pos = {(int)xpos,(int)ypos}; ClientToScreen(win->win32.handle,&pos); SetCursorPos(pos.x,pos.y); }
    static void enableCursor(_GLFWwindow* win) { _glfw.win32.disabledCursorWindow = NULL; releaseCursor(); SetCursorPosV(win,_glfw.win32.restoreCurPosX,_glfw.win32.restoreCurPosY); updateCursorImage(win); }
    static i64 __stdcall windowProc(HWND hWnd, u32 uMsg, u64 wParam, i64 lParam) {
        _GLFWwindow* win=GetPropW(hWnd,L"GLFW"); if (!win) return DefWindowProcW(hWnd,uMsg,wParam,lParam);
        switch (uMsg) {
            case 0x0021/*WM_MOUSEACTIVATE*/:  if (HIWORD(lParam) == 0x0201/*WM_LBUTTONDOWN*/ && LOWORD(lParam)!=1) {win->win32.frameAction= 1;} break;
            case 0x0215/*WM_CAPTURECHANGED*/: if (lParam==0&&win->win32.frameAction) { if (win->cursorMode==0x00034003/*CURSOR_DISABLED*/) {disableCursor(win);} win->win32.frameAction=0; } break;
            case 0x0007/*WM_SETFOCUS*/:   InputWindowFocus(1); if (win->win32.frameAction) {break;} if (win->cursorMode==0x00034003/*CURSOR_DISABLED*/) {disableCursor(win);} return 0;
            case 0x0008/*WM_KILLFOCUS*/:  if (win->cursorMode==0x00034003/*CURSOR_DISABLED*/) {enableCursor(win);} InputWindowFocus(0); return 0;
            case 0x0112/*WM_SYSCOMMAND*/: switch (wParam&0xfff0) { case 0xF140/*SC_SCREENSAVE*/: case 0xF170/*SC_MONITORPOWER*/: break; case 0xF100/*SC_KEYMENU*/: if (!win->win32.keymenu) return 0; break; } break;
            case 0x0010/*WM_CLOSE*/:      OS_Exit(0);
            case 0x0100/*WM_KEYDOWN*/: case 0x0104/*WM_SYSKEYDOWN*/: case 0x0101/*WM_KEYUP*/: case 0x0105/*WM_SYSKEYUP*/: {
                const int action=(HIWORD(lParam)&0x8000)?INPUT_RELEASE:INPUT_PRESS;
                int scancode=(HIWORD(lParam)&(0x0100|0xff));
                if (!scancode) scancode=MapVirtualKeyW((u32)wParam,0);
                if (scancode==0x54) {scancode=0x137;}   if (scancode==0x146) {scancode=0x45;}   if (scancode==0x136) {scancode=0x36;}
                int key = _glfw.win32.keycodes[scancode];
                if (wParam==0x11/*VK_CONTROL*/) {
                    if (HIWORD(lParam)&0x0100) key=KEY_RIGHT_CONTROL;
                    else {
                        MSG next; const u32 time=GetMessageTime();
                        if (PeekMessageW(&next,NULL,0,0,0)) {
                            if (next.message == 0x0100/*WM_KEYDOWN*/ || next.message == 0x0104/*WM_SYSKEYDOWN*/ || next.message == 0x0101/*WM_KEYUP*/ || next.message == 0x0105/*WM_SYSKEYUP*/) {
                                if (next.wParam == 0x12/*VK_MENU*/ && (HIWORD(next.lParam) & 0x0100)&&next.time==time) break;
                            }
                        }
                        
                        key=KEY_LEFT_CONTROL;
                    }
                } else if (wParam == 0xE5/*VK_PROCESSKEY*/) break;
                if (action == INPUT_RELEASE && wParam == 0x10/*VK_SHIFT*/) { InputKey(win,KEY_LEFT_SHIFT,action); InputKey(win,KEY_RIGHT_SHIFT,action); }
                else if (wParam == 0x2C/*VK_SNAPSHOT*/) { InputKey(win,key,INPUT_PRESS); InputKey(win,key,INPUT_RELEASE); }
                else InputKey(win,key,action);
                break;
            }
            case 0x0201/*WM_LBUTTONDOWN*/: case 0x0204/*WM_RBUTTONDOWN*/: case 0x0207/*WM_MBUTTONDOWN*/: case 0x020B/*WM_XBUTTONDOWN*/:
            case 0x0202/*WM_LBUTTONUP*/:   case 0x0205/*WM_RBUTTONUP*/:   case 0x0208/*WM_MBUTTONUP*/:   case 0x020C/*WM_XBUTTONUP*/: {
                int i,action,button = (uMsg==0x0201/*WM_LBUTTONDOWN*/ || uMsg == 0x0202/*WM_LBUTTONUP*/) ? MOUSE_BUTTON_LEFT : ((uMsg == 0x0204/*WM_RBUTTONDOWN*/ || uMsg == 0x0205/*WM_RBUTTONUP*/) ? MOUSE_BUTTON_RIGHT : ((uMsg == 0x0207/*WM_MBUTTONDOWN*/ || uMsg == 0x0208/*WM_MBUTTONUP*/) ? MOUSE_BUTTON_MIDDLE : (((HIWORD(wParam)) == 0x0001/*XBUTTON1*/) ? MOUSE_BUTTON_4 : MOUSE_BUTTON_5)));
                action=(uMsg == 0x0201/*WM_LBUTTONDOWN*/ || uMsg == 0x0204/*WM_RBUTTONDOWN*/ || uMsg == 0x0207/*WM_MBUTTONDOWN*/ || uMsg == 0x020B/*WM_XBUTTONDOWN*/) ? INPUT_PRESS : INPUT_RELEASE;
                for (i=0;i<=7;i++) { if (win->mouseButtons[i]==INPUT_PRESS) break; }
                if (i>7) {SetCapture(hWnd);} InputMouseClick(win,button,action);
                for (i=0;i<=7;i++) { if (win->mouseButtons[i]==INPUT_PRESS) break; }
                if (i>7) {ReleaseCapture();} if (uMsg == 0x020B/*WM_XBUTTONDOWN*/ || uMsg == 0x020C/*WM_XBUTTONUP*/) return 1;
                return 0;
            }
            case 0x0200/*WM_MOUSEMOVE*/: {                
                const int x=((int)(short)(lParam & 0xFFFF)), y=((int)(short)(lParam >> 16));
                if (win->cursorMode==0x00034003/*CURSOR_DISABLED*/) {
                    const int dx=x-win->win32.lastCursorPosX,dy=y-win->win32.lastCursorPosY;
                    if (_glfw.win32.disabledCursorWindow!=win) break;
                    InputCursorPos(win,win->virtualCursorPosX+dx,win->virtualCursorPosY+dy);
                }
                
                win->win32.lastCursorPosX=x; win->win32.lastCursorPosY=y;
                return 0;
            }
            case 0x02A3/*WM_MOUSELEAVE*/: { win->win32.cursorTracked=0; return 0; }
            case 0x020A/*WM_MOUSEWHEEL*/: { Sys_Input.scrollDelta += (i16)HIWORD(wParam)/(double)120; return 0; }
            case 0x0005/*WM_SIZE*/: if (wParam == 1) {Sys_Global.gamePaused = true;} return 0;
            case 0x0003/*WM_MOVE*/: if (_glfw.win32.capturedCursorWindow==win) {captureCursor(win);} return 0;
            case 0x0086/*WM_NCACTIVATE*/: case 0x0085/*WM_NCPAINT*/: { if (!win->decorated) return 1; break; }
            case 0x0020/*WM_SETCURSOR*/: { if (LOWORD(lParam)==1) { updateCursorImage(win); return 1; } break; }
            case 0x0084/*WM_NCHITTEST*/: ;i64 hit = DefWindowProcW(hWnd,uMsg,wParam,lParam); if (hit >= 10 && hit <= 17) { return 1; } return hit;
        }
        
        return DefWindowProcW(hWnd,uMsg,wParam,lParam);
    }

    void SetWindowIcon(const GLFWimage* image) { HICON hIcon = createIcon(image,0,0, 1); SendMessageW(((_GLFWwindow*)window)->win32.handle,0x0080,1,(i64)hIcon); SendMessageW(((_GLFWwindow*)window)->win32.handle,0x0080,0,(i64)hIcon); }
    void GetWindowPos(_GLFWwindow* win, int* xpos, int* ypos) { POINT pos={0,0}; ClientToScreen(win->win32.handle,&pos); *xpos=pos.x; *ypos=pos.y; }
    void GetWindowSize(_GLFWwindow* win, int* width, int* height) { RECT area; GetClientRect(win->win32.handle,&area); *width=area.right; *height=area.bottom; }
    void SetWindowSize(_GLFWwindow* win, int width, int height) { RECT rect={0,0,width,height}; AdjustWindowRectEx(&rect,getWindowStyle(win),0,0); SetWindowPos(win->win32.handle,(HWND)0,0,0,rect.right-rect.left,rect.bottom-rect.top,0x0010|0x0200|0x0002|0x0004); }
    void SetWindowMonitor(_GLFWwindow* win, int xpos, int ypos, int width, int height) {
        RECT r = {xpos,ypos,xpos+width,ypos+height}; u32 s = GetWindowLongW(win->win32.handle,-16); u32 f = 0x0010|0x0100;
        if (win->decorated) { s &= ~0x80000000/*WS_POPUP*/, s |= getWindowStyle(win), SetWindowLongW(win->win32.handle,-16,s), f |= 0x0020; }
        AdjustWindowRectEx(&r,getWindowStyle(win),0,0); SetWindowPos(win->win32.handle,(HWND)-2,r.left,r.top,r.right-r.left,r.bottom-r.top,f);
    }

    void SetWindowDecorated(_GLFWwindow* win,i32 enabled) {
        (void)enabled; RECT rect; u32 style=GetWindowLongW(win->win32.handle,-16);
        style &= ~(0x00C00000/*WS_CAPTION*/ | 0x00080000/*WS_SYSMENU*/ | 0x00040000/*WS_THICKFRAME*/ | 0x00020000/*WS_MINIMIZEBOX*/ | 0x00010000/*WS_MAXIMIZEBOX*/ | 0x80000000/*WS_POPUP*/); style |= getWindowStyle(win);
        GetClientRect(win->win32.handle,&rect); AdjustWindowRectEx(&rect,style,0,0); ClientToScreen(win->win32.handle,(POINT*)&rect.left); ClientToScreen(win->win32.handle,(POINT*)&rect.right); SetWindowLongW(win->win32.handle,-16,style);
        SetWindowPos(win->win32.handle,(HWND)0,rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top,0x0020|0x0010|0x0004);
    }
    
    void PollEvents() {
        HWND handle = GetActiveWindow();
        _GLFWwindow* win = GetPropW(handle,L"GLFW"); MSG msg;
        while (PeekMessageW(&msg,NULL,0,0,0x0001)) { if (msg.message==0x0012/*WM_QUIT*/) { OS_Exit(0); } else { TranslateMessage(&msg); DispatchMessageW(&msg); } }
        const int keys[4][2]={{0xA0/*VK_LSHIFT*/,KEY_LEFT_SHIFT},{0xA1/*VK_RSHIFT*/,KEY_RIGHT_SHIFT},{0x5B/*VK_LWIN*/,KEY_LEFT_SUPER},{0x5C/*VK_RWIN*/,KEY_RIGHT_SUPER}};
        for (int i=0;i<4;i++) { const int vk=keys[i][0],key=keys[i][1]; if ((GetKeyState(vk)&0x8000)||win->keys[key]!=INPUT_PRESS) {continue;} InputKey(win,key,INPUT_RELEASE); }
        win = _glfw.win32.disabledCursorWindow;
        if (win) { int width,height; GetWindowSize(win,&width,&height); if (win->win32.lastCursorPosX != width/2 || win->win32.lastCursorPosY != height/2) {SetCursorPosV(win,width/2,height/2);} }
    }

    void SetCursorMode(GLFWwindow* handle, int value) {
        _GLFWwindow* win = (_GLFWwindow*)handle; if (win->cursorMode == value) return;
        
        win->cursorMode = value; POINT pos; GetCursorPos(&pos);
        if (win->win32.handle == GetActiveWindow()) { _glfw.win32.restoreCurPosX=pos.x; _glfw.win32.restoreCurPosY=pos.y; captureCursor(win); _glfw.win32.disabledCursorWindow=win; } else Sys_Global.gamePaused = true;
        updateCursorImage(win);
    }

    GLFWproc PlatformGetModuleSymbol(void* module, const char* name) { return (GLFWproc)GetProcAddress((HMODULE)module,name); }
    typedef struct {u16 index; i32 vkey;} WinKeyRemap;
    static const WinKeyRemap winkeyRemapTable[] = {
        {0x00B,KEY_0},{0x002,KEY_1},{0x003,KEY_2},{0x004,KEY_3},{0x005,KEY_4},{0x006,KEY_5},{0x007,KEY_6},{0x008,KEY_7},{0x009,KEY_8},{0x00A,KEY_9},{0x01E,KEY_A},{0x030,KEY_B},{0x02E,KEY_C},
        {0x020,KEY_D},{0x012,KEY_E},{0x021,KEY_F},{0x022,KEY_G},{0x023,KEY_H},{0x017,KEY_I},{0x024,KEY_J},{0x025,KEY_K},{0x026,KEY_L},{0x032,KEY_M},{0x031,KEY_N},{0x018,KEY_O},{0x019,KEY_P},
        {0x010,KEY_Q},{0x013,KEY_R},{0x01F,KEY_S},{0x014,KEY_T},{0x016,KEY_U},{0x02F,KEY_V},{0x011,KEY_W},{0x02D,KEY_X},{0x015,KEY_Y},{0x02C,KEY_Z},{0x028,KEY_APOSTROPHE},{0x02B,KEY_BACKSLASH},
        {0x033,KEY_COMMA},{0x00D,KEY_EQUAL},{0x029,KEY_GRAVE_ACCENT},{0x01A,KEY_LEFT_BRACKET},{0x00C,KEY_MINUS},{0x034,KEY_PERIOD},{0x01B,KEY_RIGHT_BRACKET},{0x027,KEY_SEMICOLON},{0x035,KEY_SLASH},
        {0x00E,KEY_BACKSPACE},{0x153,KEY_DELETE},{0x14F,KEY_END},{0x01C,KEY_ENTER},{0x001,KEY_ESCAPE},{0x147,KEY_HOME},{0x152,KEY_INSERT},{0x15D,KEY_MENU},{0x151,KEY_PAGE_DOWN},{0x149,KEY_PAGE_UP},
        {0x045,KEY_PAUSE},{0x039,KEY_SPACE},{0x00F,KEY_TAB},{0x03A,KEY_CAPS_LOCK},{0x145,KEY_NUM_LOCK},{0x046,KEY_SCROLL_LOCK},{0x03B,KEY_F1},{0x03C,KEY_F2},{0x03D,KEY_F3},{0x03E,KEY_F4},
        {0x03F,KEY_F5},{0x040,KEY_F6},{0x041,KEY_F7},{0x042,KEY_F8},{0x043,KEY_F9},{0x044,KEY_F10},{0x057,KEY_F11},{0x058,KEY_F12},{0x038,KEY_LEFT_ALT},{0x01D,KEY_LEFT_CONTROL},{0x02A,KEY_LEFT_SHIFT},
        {0x15B,KEY_LEFT_SUPER},{0x137,KEY_PRINT_SCREEN},{0x138,KEY_RIGHT_ALT},{0x11D,KEY_RIGHT_CONTROL},{0x036,KEY_RIGHT_SHIFT},{0x15C,KEY_RIGHT_SUPER},{0x150,KEY_DOWN},{0x14B,KEY_LEFT},{0x14D,KEY_RIGHT},
        {0x148,KEY_UP},{0x052,KEY_KP_0},{0x04F,KEY_KP_1},{0x050,KEY_KP_2},{0x051,KEY_KP_3},{0x04B,KEY_KP_4},{0x04C,KEY_KP_5},{0x04D,KEY_KP_6},{0x047,KEY_KP_7},{0x048,KEY_KP_8},{0x049,KEY_KP_9},
        {0x04E,KEY_KP_ADD},{0x053,KEY_KP_DECIMAL},{0x135,KEY_KP_DIVIDE},{0x11C,KEY_KP_ENTER},{0x059,KEY_KP_EQUAL},{0x037,KEY_KP_MULTIPLY},{0x04A,KEY_KP_SUBTRACT}
    };
    
    static void createKeyTables() {
        MemSetToVForNBytes(_glfw.win32.keycodes,-1,sizeof(_glfw.win32.keycodes)); MemSetToVForNBytes(_glfw.win32.scancodes,-1,sizeof(_glfw.win32.scancodes));
        for (size_t i=0;i<sizeof(winkeyRemapTable)/sizeof(winkeyRemapTable[0]);++i) _glfw.win32.keycodes[winkeyRemapTable[i].index] = winkeyRemapTable[i].vkey;
        for (int scancode=0;scancode<512;scancode++) { if (_glfw.win32.keycodes[scancode] > 0) {_glfw.win32.scancodes[_glfw.win32.keycodes[scancode]] = scancode;} }
    }

    u16* CreateWideStringFromUTF8Win32(const char* src) { u16* target; int count = MultiByteToWideChar(65001,0,(char*)src,-1,NULL,0); target = OS_Calloc(count,sizeof(u16)); MultiByteToWideChar(65001,0,(char*)src,-1,target,count); return target; }
    char* CreateUTF8FromWideStringWin32(const u16* src, int* size) { *size = WideCharToMultiByte(65001,0,(u16*)src,-1,NULL,0,NULL,NULL); char* target = OS_Calloc(*size,1); WideCharToMultiByte(65001,0,(u16*)src,-1,target,*size,NULL,NULL); return target; }
    i32 IsWindowsVersionOrGreaterWin32(u16 major, u16 minor, u16 sp) {
        OSVERSIONINFOEXW osvi={0}; osvi.dwOSVersionInfoSize=sizeof(osvi), osvi.dwMajorVersion=major, osvi.dwMinorVersion=minor, osvi.wServicePackMajor=sp;
        u32 mask=0x0000002|0x0000001|0x0000020;
        u64 cond=VerSetConditionMask(VerSetConditionMask(VerSetConditionMask(0,0x0000002,3),0x0000001,3),0x0000020,3);
        return _glfw.win32.ntdll.RtlVerifyVersionInfo(&osvi,mask,cond)==0;
    }

    static void closeJoystick(_GLFWjoystick* js) { JoystickConnection(js,0x00040002/*disconnected*/); _glfwFreeJoystick(js); }
    void _glfwDetectJoystickConnectionWin32() {
        if (_glfw.win32.xinput.instance) {
            for (u32 index=0;index<4;index++) {
                int jid; char guid[33]; XINPUT_CAPABILITIES xic; _GLFWjoystick* js;
                for (jid = 0;  jid <= JOYSTICK_LAST;  jid++) {
                    if (_glfw.joysticks[jid].connected && _glfw.joysticks[jid].win32.index == index) break;
                }

                if (jid <= JOYSTICK_LAST) continue;
                if (_glfw.win32.xinput.GetCapabilities(index,0,&xic) != 0) continue;

                StringFormat(guid,sizeof(guid),"78696e707574%02x000000000000000000",xic.SubType & 0xff);
                js = _glfwAllocJoystick("Gamepad", guid, 6, 10, 1);
                if (!js) continue;

                js->win32.index = index;
                JoystickConnection(js,0x00040001/*connected*/);
            }
        }
    }

    i32 InitJoysticks() { _glfwDetectJoystickConnectionWin32(); return  1; }
    i32 PollJoystick(_GLFWjoystick* js) {
        u32 result; XINPUT_STATE xis;
        const u16 buttons[14] = {0x0001/*XINPUT_GAMEPAD_DPAD_UP*/,0x0002/*XINPUT_GAMEPAD_DPAD_DOWN*/,0x0008/*XINPUT_GAMEPAD_DPAD_RIGHT*/,0x0004/*XINPUT_GAMEPAD_DPAD_LEFT*/,0x1000/*XINPUT_GAMEPAD_A*/,0x2000/*XINPUT_GAMEPAD_B*/,0x4000/*XINPUT_GAMEPAD_X*/,0x8000/*XINPUT_GAMEPAD_Y*/,0x0100/*XINPUT_GAMEPAD_LEFT_SHOULDER*/,0x0200/*XINPUT_GAMEPAD_RIGHT_SHOULDER*/,0x0020/*XINPUT_GAMEPAD_BACK*/,0x0010/*XINPUT_GAMEPAD_START*/,0x0040/*XINPUT_GAMEPAD_LEFT_THUMB*/,0x0080/*XINPUT_GAMEPAD_RIGHT_THUMB*/};
        result = _glfw.win32.xinput.GetState(js->win32.index, &xis);
        if (result != 0) { if (result == 1167/*not connected*/) {closeJoystick(js);} return 0; }

        const i16 axis_vals[] = {xis.Gamepad.sThumbLX,-xis.Gamepad.sThumbLY,xis.Gamepad.sThumbRX,-xis.Gamepad.sThumbRY};
        for (int i=0;i<4;++i) InputJoystickAxis(js,i,(axis_vals[i] + 0.5f) / 32767.5f);
        InputJoystickAxis(js,4,xis.Gamepad.bLeftTrigger / 127.5f - 1.f); InputJoystickAxis(js,5,xis.Gamepad.bRightTrigger / 127.5f - 1.f);
        for (int i=0;i<10;++i) { const char value = (xis.Gamepad.wButtons & buttons[i]) ? 1 : 0; InputJoystickButton(js,i,value); }
        int dpad = ((const int[]){0,1,2,3,4,0,0,0,8,0,0,0,0,0,0,0})[xis.Gamepad.wButtons & 0xF];
        if ((dpad & JOYHAT_RIGHT) && (dpad & JOYHAT_LEFT)) dpad &= ~(JOYHAT_RIGHT | JOYHAT_LEFT);
        if ((dpad & JOYHAT_UP) && (dpad & JOYHAT_DOWN)) dpad &= ~(JOYHAT_UP | JOYHAT_DOWN);
        InputJoystickHat(js, 0, dpad);
        return  1;
    }
    
    void _glfwDetectJoystickDisconnectionWin32() { for (int jid = 0;  jid <= JOYSTICK_LAST;  jid++) { _GLFWjoystick* js = _glfw.joysticks + jid; if (js->connected) {PollJoystick(js);} } }
    static i32 __stdcall monitorCallback(HMONITOR handle, HDC dc, RECT* rect, i64 data) {
        MONITORINFOEXW mi; (void)dc; (void)rect;
        MemSetToVForNBytes(&mi,0,sizeof(mi));
        mi.cbSize = sizeof(mi);
        if (GetMonitorInfoW(handle, (MONITORINFO*) &mi)) {
            _GLFWmonitor* monitor = (_GLFWmonitor*) data;
            if (wcscmp(mi.szDevice, monitor->win32.adapterName) == 0) monitor->win32.handle = handle;
        }

        return 1;
    }

    static _GLFWmonitor* createMonitor(DISPLAY_DEVICEW* adapter, DISPLAY_DEVICEW* display) {
        _GLFWmonitor* monitor; int widthMM,heightMM,nameSize=0; HDC dc; DEVMODEW dm; RECT rect;
        char* name = CreateUTF8FromWideStringWin32(display ? display->DeviceString : adapter->DeviceString,&nameSize);
        MemSetToVForNBytes(&dm,0,sizeof(dm)); dm.dmSize = sizeof(dm);
        EnumDisplaySettingsW(adapter->DeviceName,0xFFFFFFFFU,&dm);
        dc = CreateDCW(L"DISPLAY", adapter->DeviceName,NULL,NULL);
        if (IsWindowsVersionOrGreaterWin32(HIBYTE(0x0603),LOBYTE(0x0603),0)) { widthMM  = GetDeviceCaps(dc,4); heightMM = GetDeviceCaps(dc,6); } // Is Windows 8.10 or greater
        else { widthMM  = (int) (dm.dmPelsWidth * 25.4f / GetDeviceCaps(dc,88)); heightMM = (int) (dm.dmPelsHeight * 25.4f / GetDeviceCaps(dc,90)); }

        DeleteDC(dc); monitor = AllocMonitor(name,widthMM,heightMM); OS_DeallocateRAM(name,nameSize);
        if (adapter->StateFlags & 0x08000000/*DISPLAY_DEVICE_MODESPRUNED*/) monitor->win32.modesPruned =  1;
        wcscpy(monitor->win32.adapterName, adapter->DeviceName);
        if (display) wcscpy(monitor->win32.displayName,display->DeviceName);
        rect.left=dm.dmPosition.x; rect.top=dm.dmPosition.y; rect.right=dm.dmPosition.x + dm.dmPelsWidth; rect.bottom=dm.dmPosition.y + dm.dmPelsHeight;
        EnumDisplayMonitors(NULL,&rect,monitorCallback,(i64)monitor);
        return monitor;
    }

    void _glfwPollMonitorsWin32() {
        int i, disconnectedCount = _glfw.monitorCount; _GLFWmonitor** disconnected = NULL; u32 adapterIndex,displayIndex; DISPLAY_DEVICEW adapter, display; _GLFWmonitor* monitor;
        if (disconnectedCount) { disconnected = OS_Calloc(_glfw.monitorCount,sizeof(_GLFWmonitor*)); MemCpyFromBtoAForNBytes(disconnected,_glfw.monitors,_glfw.monitorCount * sizeof(_GLFWmonitor*)); }
        for (adapterIndex = 0;;adapterIndex++) {
            int type = 1; MemSetToVForNBytes(&adapter,0,sizeof(adapter)); adapter.cb = sizeof(adapter);
            if (!EnumDisplayDevicesW(NULL, adapterIndex, &adapter, 0)) break;
            if (!(adapter.StateFlags&1)) continue;

            if (adapter.StateFlags & 0x00000004/*DISPLAY_DEVICE_PRIMARY_DEVICE*/) type = 0;
            for (displayIndex=0;;++displayIndex) {
                MemSetToVForNBytes(&display,0,sizeof(display)); display.cb = sizeof(display);
                if (!EnumDisplayDevicesW(adapter.DeviceName, displayIndex, &display, 0)) break;
                if (!(display.StateFlags&1)) continue;

                for (i=0;i<disconnectedCount;++i) {
                    if (disconnected[i] && wcscmp(disconnected[i]->win32.displayName,display.DeviceName) == 0) {
                        disconnected[i] = NULL;
                        EnumDisplayMonitors(NULL,NULL,monitorCallback,(i64)_glfw.monitors[i]);
                        break;
                    }
                }

                if (i < disconnectedCount) continue;
                monitor = createMonitor(&adapter,&display); if (!monitor) { OS_DeallocateRAM(disconnected,_glfw.monitorCount*sizeof(_GLFWmonitor*)); return; }

                InputMonitor(monitor,0x00040001/*connected*/,type); type = 1;
            }

            if (displayIndex == 0) {
                for (i=0;i<disconnectedCount;++i) { if (disconnected[i] && wcscmp(disconnected[i]->win32.adapterName,adapter.DeviceName) == 0) {disconnected[i]=NULL; break;} }
                if (i < disconnectedCount) continue;
                monitor = createMonitor(&adapter,NULL); if (!monitor) { OS_DeallocateRAM(disconnected,_glfw.monitorCount*sizeof(_GLFWmonitor*)); return; }

                InputMonitor(monitor, 0x00040001/*connected*/, type);
            }
        }

        for (i=0;i<disconnectedCount;++i) { if (disconnected[i]) {InputMonitor(disconnected[i],0x00040002/*disconnected*/,0);} }
        if (disconnected) OS_DeallocateRAM(disconnected,_glfw.monitorCount*sizeof(_GLFWmonitor*));
    }
    
    static i64 __stdcall helperWindowProc(HWND hWnd, u32 uMsg, u64 wParam, i64 lParam) {
        switch (uMsg) {
            case 0x007E/*WM_DISPLAYCHANGE*/: _glfwPollMonitorsWin32(); break;
            case 0x0219/*WM_DEVICECHANGE*/: if (!_glfw.joysticksInitialized) break;
                if (wParam == 0x8000/*DBT_DEVICEARRIVAL*/ || wParam == 0x8004/*DBT_DEVICEREMOVECOMPLETE*/) {
                    DEV_BROADCAST_HDR* dbh = (DEV_BROADCAST_HDR*) lParam;
                    if (dbh && dbh->dbch_devicetype == 0x0005/*DBT_DEVTYP_DEVICEINTERFACE*/ && wParam == 0x8000/*DBT_DEVICEARRIVAL*/)           _glfwDetectJoystickConnectionWin32();
                    if (dbh && dbh->dbch_devicetype == 0x0005/*DBT_DEVTYP_DEVICEINTERFACE*/ && wParam == 0x8004/*DBT_DEVICEREMOVECOMPLETE*/) _glfwDetectJoystickDisconnectionWin32();
                }

                break;
        }

        return DefWindowProcW(hWnd,uMsg,wParam,lParam);
    }

    void GetMonitorPos(_GLFWmonitor* monitor, int* xpos, int* ypos) { DEVMODEW dm; MemSetToVForNBytes(&dm,0,sizeof(dm)); dm.dmSize = sizeof(dm); EnumDisplaySettingsExW(monitor->win32.adapterName,0xFFFFFFFFU,&dm,0x00000004); *xpos = dm.dmPosition.x; *ypos = dm.dmPosition.y; }
    void GetMonitorWorkarea(_GLFWmonitor* monitor, int* xpos, int* ypos, int* width, int* height) { MONITORINFO mi = {0}; mi.cbSize = sizeof(mi); GetMonitorInfoW(monitor->win32.handle, &mi); *xpos = mi.rcWork.left; *ypos = mi.rcWork.top; *width = mi.rcWork.right - mi.rcWork.left; *height = mi.rcWork.bottom - mi.rcWork.top; }
    void GetVideoMode(_GLFWmonitor* monitor, GLFWvidmode* mode) { DEVMODEW dm; MemSetToVForNBytes(&dm,0,sizeof(dm)); dm.dmSize = sizeof(dm); EnumDisplaySettingsW(monitor->win32.adapterName,0xFFFFFFFFU,&dm); mode->width=dm.dmPelsWidth; mode->height=dm.dmPelsHeight; mode->refreshRate=dm.dmDisplayFrequency; }
    static int choosePixelFormatWGL(_GLFWwindow* win) {
        int attribs[24],values[24],attribCount=0,i,pixelFormat,nativeCount,usableCount=0;
        const int query = 0x2000/*num pixel formats*/; _glfw.wgl.GetPixelFormatAttribivARB(win->context.wgl.dc,1,0,1,&query,&nativeCount);
        attribs[attribCount++] = 0x2010/*support opengl*/; attribs[attribCount++] = 0x2001/*draw to window*/; attribs[attribCount++] = 0x2013/*pixel type*/; attribs[attribCount++] = 0x2003/*accelaration*/;
        attribs[attribCount++] = 0x2011/*double buffer*/; attribs[attribCount++] = 0x2015/*r bits*/; attribs[attribCount++] = 0x2017/*g bits*/;
        attribs[attribCount++] = 0x2019/*b bits*/; attribs[attribCount++] = 0x201b/*a bits*/; attribs[attribCount++] = 0x2022/*depth bits*/; attribs[attribCount++] = 0x2023/*stencil bits*/;
        _GLFWfbconfig* usableConfigs = OS_Calloc(nativeCount,sizeof(_GLFWfbconfig));
        for (i = 0; i < nativeCount; i++) {
            _GLFWfbconfig* u = usableConfigs + usableCount; pixelFormat = i + 1;
            _glfw.wgl.GetPixelFormatAttribivARB(win->context.wgl.dc,pixelFormat,0,attribCount,attribs,values);
            if (values[0] == 0 || values[1] == 0/* support OpenGL + draw to window */ || values[2] != 0x202b/*type rgba*/ || values[3] == 0x2025/*no accel*/ || values[4] !=  1) continue;
            
            u->redBits=values[5]; u->greenBits=values[6]; u->blueBits=values[7]; u->alphaBits=values[8]; u->depthBits=values[9]; u->stencilBits=values[10]; u->handle=pixelFormat; usableCount++;
        }

        const _GLFWfbconfig* closest = ChooseFBConfig(usableConfigs,usableCount);
        pixelFormat = (int)closest->handle; OS_DeallocateRAM(usableConfigs,nativeCount * sizeof(_GLFWfbconfig));
        return pixelFormat;
    }

    static void makeContextCurrentWGL(_GLFWwindow* win) { wglMakeCurrent(win->context.wgl.dc,win->context.wgl.handle); }
    static void swapBuffersWGL(_GLFWwindow* win) {
        if (!IsWindowsVersionOrGreaterWin32(HIBYTE(0x0602),LOBYTE(0x0602),0)) { i32 enabled = 0; if ((i32)(_glfw.win32.dwmapi.IsCompositionEnabled(&enabled) >= 0) && enabled) { int count = vabs(win->context.wgl.interval); while (count--) {_glfw.win32.dwmapi.Flush();} } } // Is Windows 8.0 or greater
        SwapBuffers(win->context.wgl.dc);
    }

    static void swapIntervalWGL(int interval) {
        _GLFWwindow* handle = (_GLFWwindow*)window;
        handle->context.wgl.interval = interval;
        if (!IsWindowsVersionOrGreaterWin32(HIBYTE(0x0602),LOBYTE(0x0602),0)) { i32 enabled = 0; if ((i32)(_glfw.win32.dwmapi.IsCompositionEnabled(&enabled) >= 0) && enabled) interval = 0; } // Is Windows 8.0 or greater
        _glfw.wgl.SwapIntervalEXT(interval);
    }

    static GLFWglproc getProcAddressWGL(const char* procname) { const GLFWglproc proc = (GLFWglproc)wglGetProcAddress(procname); if (proc) {return proc;} return (GLFWglproc)PlatformGetModuleSymbol(_glfw.wgl.instance,procname); }
    void glfwSetWindowPosition(GLFWwindow* handle, int xpos, int ypos) { _GLFWwindow* win = (_GLFWwindow*)handle; RECT rect = {xpos,ypos,xpos,ypos}; AdjustWindowRectEx(&rect,getWindowStyle(win),0,0x00040000/*WS_EX_APPWINDOW*/); SetWindowPos(win->win32.handle,((HWND)0),rect.left,rect.top,0,0,0x0010|0x0200|0x0001|0x0004); }
#else // LINUX
    typedef u8 KeyCode; typedef u16 Rotation,SubpixelOrder,Connection; typedef i32 Bool; typedef int Status; typedef u64 XID,Mask,Atom,VisualID,Time,KeySym; typedef char *XPointer; typedef u32 XcursorUInt;
    typedef struct _XcursorImage { XcursorUInt version; XcursorUInt size,width,height,xhot,yhot; XcursorUInt delay; XcursorUInt *pixels; } XcursorImage;
    typedef struct { i64 flags; int x,y, width,height,min_width,min_height,max_width,max_height,width_inc,height_inc; struct {int x; int y;} min_aspect,max_aspect; int base_width, base_height; int win_gravity; } XSizeHints;
    typedef XID Window,Drawable,Font,Pixmap,Cursor,Colormap;
    typedef struct _XExtData { int number; struct _XExtData *next; int (*free_private)(struct _XExtData*); XPointer private_data; } XExtData;
    typedef struct { int extension, major_opcode, first_event, first_error; } XExtCodes; typedef struct { int depth, bits_per_pixel, scanline_pad; } XPixmapFormatValues;
    typedef struct _XGC *GC; typedef struct { XExtData *ext_data; VisualID visualid; int class; u64 red_mask, green_mask, blue_mask; int bits_per_rgb; int map_entries;} Visual; 
    typedef struct { int depth,nvisuals; Visual *visuals; } Depth;
    typedef struct { XExtData *ext_data; struct _XDisplay *display; Window root; int width,height,mwidth,mheight,ndepths; Depth *depths; int root_depth; Visual *root_visual; GC default_gc; Colormap cmap; u64 white_pixel, black_pixel; int max_maps, min_maps, backing_store; int save_unders; i64 root_input_mask; } Screen;
    typedef struct { XExtData *ext_data; int depth, bits_per_pixel, scanline_pad; } ScreenFormat;
    typedef struct { Pixmap background_pixmap; u64 background_pixel; Pixmap border_pixmap; u64 border_pixel; int bit_gravity, win_gravity, backing_store; u64 backing_planes, backing_pixel; int save_under; i64 event_mask, do_not_propagate_mask; int override_redirect; Colormap colormap; Cursor cursor; } XSetWindowAttributes;
    typedef struct { int x,y,width,height,border_width,depth; Visual *visual; Window root; int class,bit_gravity,win_gravity,backing_store; u64 backing_planes,backing_pixel; int save_under; Colormap colormap; int map_installed,map_state; i64 all_event_masks,your_event_mask,do_not_propagate_mask; i32 override_redirect; Screen *screen; } XWindowAttributes;
    typedef struct _XDisplay Display;
    typedef struct { XExtData *ext_data; struct _XPrivate *private1; int fd, private2, proto_major_version, proto_minor_version; char *vendor; XID private3, private4, private5; int private6; XID (*resource_alloc)(struct _XDisplay*); int byte_order, bitmap_unit, bitmap_pad, bitmap_bit_order, nformats; ScreenFormat *pixmap_format; int private8; struct _XPrivate *private9, *private10; int qlen; u64 last_request_read,request; XPointer private11,private12,private13,private14; unsigned max_request_size; struct _XrmHashBucketRec *db; int (*private15)(struct _XDisplay*); char *display_name; i32 default_screen, nscreens; Screen *screens; u64 motion_buffer, private16; i32 min_keycode,max_keycode; XPointer private17,private18; i32 private19; char *xdefaults; } *_XPrivDisplay;
    typedef struct { int a; u64 b; int c; void *d; u64 e,f,g,h; int i,j,k,l; u32 m,keycode; int n; } XKeyEvent; // Don't care the names of the unused fields here, so just stuff alphabet in there
    typedef struct { int a; u64 b; int c; Display *d; Window e,f,g; Time h; int i,j,k,l; u32 m,button; int n; } XButtonEvent;  typedef struct { int a; u64 b; int c; Display *d; Window e,f,g; Time h; int x,y,i,j; u32 k; char l; int m; } XMotionEvent;
    typedef struct { int a; u64 b; int c; Display *d; Window e,f,g; Time h; int x,y,i,j,k,l; int m,n; u32 o; } XCrossingEvent; typedef struct { int a; u64 b; i32 c; Display *d; Window e; int mode,f; } XFocusChangeEvent;
    typedef struct { int a; u64 b; i32 c; Display *d; Window e,f,parent; int g,h,i; } XReparentEvent;                          typedef struct { int a; u64 b; i32 c; Display *d; Window e,f; int x,y,width,height,g; Window h; int i; } XConfigureEvent;
    typedef struct { int a; u64 b; int c; Display *d; Window window; Atom message_type; int format; union { char b[20]; short s[10]; long l[5]; } data; } XClientMessageEvent;
    typedef struct { int a; u64 b; int send_event; Display *c; Window window; } XAnyEvent;
    typedef struct { int type; u64 serial; int send_event; Display *display; int extension, evtype; u32 cookie; void *data; } XGenericEventCookie;
    typedef union _XEvent { int type; XAnyEvent xany; XKeyEvent xkey; XButtonEvent xbutton; XMotionEvent xmotion; XCrossingEvent xcrossing; XFocusChangeEvent xfocus; u8 p0[528]; XReparentEvent xreparent; XConfigureEvent xconfigure; u8 p1[648]; XClientMessageEvent xclient; u8 p2[224]; } XEvent;
    typedef struct _XIC *XIC;
    typedef struct { Visual *visual; VisualID visualid; int screen,depth; int class; u64 red_mask,green_mask,blue_mask; int colormap_size,bits_per_rgb; } XVisualInfo;
    typedef int XContext; typedef XID RROutput,RRCrtc,RRMode; typedef u64 XRRModeFlags;
    typedef struct { RRMode id; u32 width,height; u64 dotClock; u32 hSyncStart,hSyncEnd,hTotal,hSkew,vSyncStart,vSyncEnd,vTotal; char *name; u32 nameLength; XRRModeFlags modeFlags; } XRRModeInfo;
    typedef struct { Time timestamp; Time configTimestamp; int ncrtc; RRCrtc *crtcs; int noutput; RROutput *outputs; int nmode; XRRModeInfo *modes; } XRRScreenResources;
    typedef struct { Time timestamp; RRCrtc crtc; char *name; int nameLen; unsigned long mm_width; unsigned long mm_height; Connection connection; SubpixelOrder subpixel_order; i32 ncrtc; RRCrtc *crtcs; i32 nclone; RROutput *clones; i32 nmode,npreferred; RRMode *modes; } XRROutputInfo;
    typedef struct { Time timestamp; i32 x,y; u32 width,height; RRMode mode; Rotation rotation; i32 noutput; RROutput *outputs; Rotation rotations; i32 npossible; RROutput *possible; } XRRCrtcInfo;
    typedef XID GLXWindow,GLXDrawable; typedef struct __GLXFBConfig* GLXFBConfig; typedef struct __GLXcontext* GLXContext;
    typedef void(*__GLXextproc)();                                          typedef XSizeHints*(*PFN_XAllocSizeHints)();                               typedef int(*PFN_XChangeProperty)(Display*,Window,Atom,Atom,int,int,const unsigned char*,int);
    typedef Bool(*PFN_XCheckTypedWindowEvent)(Display*,Window,int,XEvent*); typedef void(*PFN_XRRFreeOutputInfo)(XRROutputInfo*);                      typedef Colormap(*PFN_XCreateColormap)(Display*,Window,Visual*,int);
    typedef int(*PFN_XDefineCursor)(Display*,Window,Cursor);                typedef int(*PFN_XDeleteProperty)(Display*,Window,Atom);                   typedef Window(*PFN_XCreateWindow)(Display*,Window,int,int,unsigned int,unsigned int,unsigned int,int,unsigned int,Visual*,unsigned long,XSetWindowAttributes*);
    typedef int(*PFN_XDisplayKeycodes)(Display*,int*,int*);                 typedef Bool(*PFN_XFilterEvent)(XEvent*,Window);                           typedef int(*PFN_XFindContext)(Display*,XID,XContext,XPointer*);
    typedef int(*PFN_XFree)(void*);                                         typedef void(*PFN_XFreeEventData)(Display*,XGenericEventCookie*);          typedef int(*PFN_XGrabPointer)(Display*,Window,Bool,unsigned int,int,int,Window,Cursor,Time);
    typedef KeySym*(*PFN_XGetKeyboardMapping)(Display*,KeyCode,int,int*);   typedef Status(*PFN_XGetWMNormalHints)(Display*,Window,XSizeHints*,long*); typedef Status(*PFN_XGetWindowAttributes)(Display*,Window,XWindowAttributes*);
    typedef Atom(*PFN_XInternAtom)(Display*,const char*,Bool);              typedef int(*PFN_XGetInputFocus)(Display*,Window*,int*);                   typedef int(*PFN_XGetWindowProperty)(Display*,Window,Atom,long,long,Bool,Atom,Atom*,int*,unsigned long*,unsigned long*,unsigned char**); 
    typedef int(*PFN_XMapWindow)(Display*,Window);                          typedef int(*PFN_XMoveWindow)(Display*,Window,int,int);                    typedef int(*PFN_XMoveResizeWindow)(Display*,Window,int,int,unsigned int,unsigned int);
    typedef Status(*PFN_XInitThreads)();                                    typedef int(*PFN_XNextEvent)(Display*,XEvent*);                            typedef XRRCrtcInfo*(*PFN_XRRGetCrtcInfo)(Display*,XRRScreenResources*,RRCrtc);
    typedef int(*PFN_XPending)(Display*);                                   typedef Bool(*PFN_XQueryExtension)(Display*,const char*,int*,int*,int*);   typedef Bool(*PFN_XQueryPointer)(Display*,Window,Window*,Window*,int*,int*,int*,int*,unsigned int*);
    typedef int(*PFN_XRaiseWindow)(Display*,Window);                        typedef int(*PFN_XSaveContext)(Display*,XID,XContext,const char*);         typedef int(*PFN_XResizeWindow)(Display*,Window,unsigned int,unsigned int);
    typedef Status(*PFN_XSendEvent)(Display*,Window,Bool,long,XEvent*);     typedef void(*PFN_XSetICFocus)(XIC);                                       typedef int(*PFN_XSetInputFocus)(Display*,Window,int,Time);
    typedef void(*PFN_XSetWMNormalHints)(Display*,Window,XSizeHints*);      typedef Status(*PFN_XSetWMProtocols)(Display*,Window,Atom*,int);           typedef Bool(*PFN_XTranslateCoordinates)(Display*,Window,Window,int,int,int*,int*,Window*);
    typedef int(*PFN_XUndefineCursor)(Display*,Window);                     typedef void(*PFN_XUnsetICFocus)(XIC);                                     typedef int(*PFN_XWarpPointer)(Display*,Window,Window,int,int,unsigned int,unsigned int,int,int);
    typedef void(*PFN_XRRFreeCrtcInfo)(XRRCrtcInfo*);                       typedef int(*PFN_XUngrabPointer)(Display*,Time);                           typedef int(*PFN_XChangeWindowAttributes)(Display*,Window,unsigned long,XSetWindowAttributes*); 
    typedef void(*PFN_XRRFreeScreenResources)(XRRScreenResources*);         typedef Display*(*PFN_XOpenDisplay)(const char*);                          typedef XRROutputInfo*(*PFN_XRRGetOutputInfo)(Display*,XRRScreenResources*,RROutput);
    typedef RROutput(*PFN_XRRGetOutputPrimary)(Display*,Window);            typedef void(*PFN_XRRSelectInput)(Display*,Window,int);                    typedef XRRScreenResources*(*PFN_XRRGetScreenResourcesCurrent)(Display*,Window);
    typedef int(*PFN_XRRUpdateConfiguration)(XEvent*);                      typedef XcursorImage*(*PFN_XcursorImageCreate)(int,int);                   typedef void(*PFN_XcursorImageDestroy)(XcursorImage*);
    typedef Bool(*PFNGLXQUERYEXTENSIONPROC)(Display*,int*,int*);            typedef int(*PFNGLXGETFBCONFIGATTRIBPROC)(Display*,GLXFBConfig,int,int*);  typedef Cursor(*PFN_XcursorImageLoadCursor)(Display*,const XcursorImage*);
    typedef Bool(*PFNGLXQUERYVERSIONPROC)(Display*,int*,int*);              typedef Bool(*PFNGLXMAKECURRENTPROC)(Display*,GLXDrawable,GLXContext);     typedef void(*PFNGLXSWAPBUFFERSPROC)(Display*,GLXDrawable);
    typedef const char*(*PFNGLXQUERYEXTENSIONSSTRINGPROC)(Display*,int);    typedef GLXFBConfig*(*PFNGLXGETFBCONFIGSPROC)(Display*,int,int*);          typedef GLXContext(*PFNGLXCREATENEWCONTEXTPROC)(Display*,GLXFBConfig,int,GLXContext,Bool);
    typedef __GLXextproc(*PFNGLXGETPROCADDRESSPROC)(const u8*);             typedef void(*PFNGLXSWAPINTERVALEXTPROC)(Display*,GLXDrawable,int);        typedef XVisualInfo*(*PFNGLXGETVISUALFROMFBCONFIGPROC)(Display*,GLXFBConfig);
    typedef GLXWindow(*PFNGLXCREATEWINDOWPROC)(Display*,GLXFBConfig,Window,const int*); typedef GLXContext(*PFNGLXCREATECONTEXTATTRIBSARBPROC)(Display*,GLXFBConfig,GLXContext,Bool,const int*);
    typedef struct _GLFWcontextGLX { GLXContext handle; GLXWindow window; GLXFBConfig fbconfig; } _GLFWcontextGLX;
    typedef struct _GLFWlibraryGLX { int major,minor,eventBase,errorBase; void* handle; PFNGLXGETFBCONFIGSPROC GetFBConfigs; PFNGLXGETFBCONFIGATTRIBPROC GetFBConfigAttrib; PFNGLXQUERYEXTENSIONPROC QueryExtension; PFNGLXQUERYVERSIONPROC QueryVersion; PFNGLXMAKECURRENTPROC MakeCurrent; PFNGLXSWAPBUFFERSPROC SwapBuffers;
                                     PFNGLXQUERYEXTENSIONSSTRINGPROC QueryExtensionsString; PFNGLXCREATENEWCONTEXTPROC CreateNewContext; PFNGLXGETVISUALFROMFBCONFIGPROC GetVisualFromFBConfig; PFNGLXCREATEWINDOWPROC CreateWindow; PFNGLXGETPROCADDRESSPROC GetProcAddress; PFNGLXSWAPINTERVALEXTPROC SwapIntervalEXT;
                                     PFNGLXCREATECONTEXTATTRIBSARBPROC CreateContextAttribsARB; } _GLFWlibraryGLX;
                                     
    typedef struct _GLFWwindowX11 { Colormap colormap; Window handle,parent; XIC ic; i32 overrideRedirect; int width,height,xpos,ypos,lastCursorPosX,lastCursorPosY,warpCursorPosX,warpCursorPosY; } _GLFWwindowX11;
    typedef struct _GLFWlibraryX11 { Display* display; int screen; Window root; Cursor hiddenCursorHandle; XContext context; short int keycodes[256],scancodes[349]; double restoreCurPosX, restoreCurPosY; _GLFWwindow* disabledCursorWindow;
                                     Atom NET_SUPPORTED,NET_SUPPORTING_WM_CHECK,WM_PROTOCOLS,WM_STATE,WM_DELETE_WINDOW,NET_WM_NAME,NET_WM_ICON,NET_WM_PING,NET_WM_WINDOW_TYPE,NET_WM_WINDOW_TYPE_NORMAL,NET_WM_STATE,NET_WM_STATE_FULLSCREEN,NET_WM_BYPASS_COMPOSITOR,NET_WORKAREA,NET_CURRENT_DESKTOP,NET_ACTIVE_WINDOW,MOTIF_WM_HINTS,UTF8_STRING;
                                     struct { void* handle; i32 utf8; PFN_XAllocSizeHints AllocSizeHints; PFN_XChangeProperty ChangeProperty; PFN_XChangeWindowAttributes ChangeWindowAttributes; PFN_XCheckTypedWindowEvent CheckTypedWindowEvent; PFN_XCreateColormap CreateColormap; PFN_XCreateWindow CreateWindow; PFN_XDefineCursor DefineCursor;
                                     PFN_XDeleteProperty DeleteProperty; PFN_XDisplayKeycodes DisplayKeycodes; PFN_XFilterEvent FilterEvent; PFN_XFindContext FindContext; PFN_XFree Free; PFN_XFreeEventData FreeEventData; PFN_XGetInputFocus GetInputFocus; PFN_XGetKeyboardMapping GetKeyboardMapping; PFN_XGetWMNormalHints GetWMNormalHints;
                                     PFN_XGetWindowAttributes GetWindowAttributes; PFN_XGetWindowProperty GetWindowProperty; PFN_XGrabPointer GrabPointer; PFN_XInternAtom InternAtom; PFN_XMapWindow MapWindow; PFN_XMoveResizeWindow MoveResizeWindow; PFN_XMoveWindow MoveWindow; PFN_XPending Pending; PFN_XQueryExtension QueryExtension;
                                     PFN_XQueryPointer QueryPointer; PFN_XRaiseWindow RaiseWindow; PFN_XResizeWindow ResizeWindow; PFN_XSaveContext SaveContext; PFN_XSendEvent SendEvent; PFN_XSetICFocus SetICFocus; PFN_XSetInputFocus SetInputFocus; PFN_XSetWMNormalHints SetWMNormalHints; PFN_XSetWMProtocols SetWMProtocols;
                                     PFN_XTranslateCoordinates TranslateCoordinates; PFN_XUndefineCursor UndefineCursor; PFN_XUngrabPointer UngrabPointer; PFN_XUnsetICFocus UnsetICFocus; PFN_XWarpPointer WarpPointer; } xlib;
                                     struct {void* handle; int eventBase,errorBase,major,minor; PFN_XRRFreeCrtcInfo FreeCrtcInfo; PFN_XRRFreeOutputInfo FreeOutputInfo; PFN_XRRFreeScreenResources FreeScreenResources; PFN_XRRGetCrtcInfo GetCrtcInfo; PFN_XRRGetOutputInfo GetOutputInfo; PFN_XRRGetOutputPrimary GetOutputPrimary;
                                             PFN_XRRGetScreenResourcesCurrent GetScreenResourcesCurrent; PFN_XRRSelectInput SelectInput; PFN_XRRUpdateConfiguration UpdateConfiguration;}randr;
                                     struct { void* handle; PFN_XcursorImageCreate ImageCreate; PFN_XcursorImageDestroy ImageDestroy; PFN_XcursorImageLoadCursor ImageLoadCursor; } xcursor; } _GLFWlibraryX11; 
    PFN_XNextEvent XNextEvent;
    typedef struct _GLFWmonitorX11 { RROutput output; RRCrtc crtc; int index; } _GLFWmonitorX11;
    typedef struct _GLFWjoystickLinux { FHandle fd; char path[260]; int keyMap[0x300/*KEY_CNT*/ - 0x100/*BTN_MISC*/],absMap[0x40/*ABS_CNT*/]; struct input_absinfo absInfo[0x40/*ABS_CNT*/]; int hats[4][2]; } _GLFWjoystickLinux;
    typedef struct _GLFWlibraryLinux { int inotify,watch; i32 dropped; } _GLFWlibraryLinux;
    void GetCursorPosV(_GLFWwindow*,double*,double*); void SetCursorPosV(_GLFWwindow*,double,double);
    struct _GLFWjoystick { i32 allocated,connected; size_t axesSize,buttonsSize,hatsSize; float*  axes; int axisCount; unsigned char* buttons; int buttonCount; unsigned char* hats; int hatCount; char name[128],guid[33]; _GLFWjoystickLinux linjs; };
    struct _GLFWlibrary { _GLFWmonitor** monitors; int monitorCount; i32 joysticksInitialized; _GLFWjoystick joysticks[JOYSTICK_LAST + 1]; _GLFWlibraryX11 x11; _GLFWlibraryGLX glx; _GLFWlibraryLinux linjs; };
    struct _GLFWcontext { int client,source,major,minor; PFNGLGETINTEGERV GetIntegerv; void (*makeCurrent)(_GLFWwindow*); void (*swapBuffers)(_GLFWwindow*); void (*swapInterval)(int); GLFWglproc (*getProcAddress)(const char*); _GLFWcontextGLX glx; };
    struct _GLFWwindow { i32 decorated,doublebuffer; GLFWvidmode videoMode; int minwidth,minheight,maxwidth,maxheight,cursorMode; char mouseButtons[8],keys[349]; double virtualCursorPosX,virtualCursorPosY; _GLFWcontext context; _GLFWwindowX11 x11; };
    struct _GLFWmonitor { char name[128]; int widthMM,heightMM; GLFWvidmode currentMode; _GLFWmonitorX11 x11; };
    void* _glfwPlatformLoadModule(const char* path) { return dlopen(path,2); }
    GLFWproc PlatformGetModuleSymbol(void* module, const char* name) { return dlsym(module,name); }
    unsigned long _glfwGetWindowPropertyX11(Window win, Atom prop, Atom type, u8** val) { Atom actType; i32 actFmt; u64 itemCount,bytesAfter; _glfw.x11.xlib.GetWindowProperty(_glfw.x11.display,win,prop,0,2147483647,0,type,&actType,&actFmt,&itemCount,&bytesAfter,val); return itemCount; }
    static int translateKey(int scancode) { return (scancode<0||scancode>255) ? KEY_UNKNOWN : _glfw.x11.keycodes[scancode]; }
    static void sendEventToWM(_GLFWwindow* win, Atom type, i64 a, i64 b, i64 c, i64 d, i64 e) {
        XEvent event={33/*ClientMessage*/}; event.xclient.window = win->x11.handle; event.xclient.format = 32; event.xclient.message_type = type; 
        event.xclient.data.l[0]=a; event.xclient.data.l[1]=b; event.xclient.data.l[2]=c; event.xclient.data.l[3]=d; event.xclient.data.l[4]=e;
        _glfw.x11.xlib.SendEvent(_glfw.x11.display,_glfw.x11.root,0,(1L<<19)|(1L<<20),&event);
    }

    static void updateNormalHints(_GLFWwindow* win, int w, int h) { XSizeHints* hs=_glfw.x11.xlib.AllocSizeHints(); i64 sup; _glfw.x11.xlib.GetWMNormalHints(_glfw.x11.display,win->x11.handle,hs,&sup); hs->flags &= ~((1L << 4)|(1L << 5)|(1L << 7)); hs->flags|=((1L << 4)|(1L << 5)); hs->min_width=hs->max_width=w; hs->min_height=hs->max_height=h; _glfw.x11.xlib.SetWMNormalHints(_glfw.x11.display,win->x11.handle,hs); _glfw.x11.xlib.Free(hs); }
    static void updateCursorImage(_GLFWwindow* win) { if (win->cursorMode==0x00034001/*GLFW_CURSOR_NORMAL*/) { _glfw.x11.xlib.UndefineCursor(_glfw.x11.display,win->x11.handle); } else {_glfw.x11.xlib.DefineCursor(_glfw.x11.display,win->x11.handle,_glfw.x11.hiddenCursorHandle);} }
    static void captureCursor(_GLFWwindow* win) { _glfw.x11.xlib.GrabPointer(_glfw.x11.display,win->x11.handle,1,(1L<<2)|(1L<<3)|(1L<<6),1/*GrabModeAsync*/,1/*GrabModeAsync*/,win->x11.handle,0L,0L); }
    static void releaseCursor() { _glfw.x11.xlib.UngrabPointer(_glfw.x11.display,0L); }
    static void disableCursor(_GLFWwindow* win) { _glfw.x11.disabledCursorWindow=win; GetCursorPosV(win,&_glfw.x11.restoreCurPosX,&_glfw.x11.restoreCurPosY); updateCursorImage(win); captureCursor(win); }
    static void enableCursor(_GLFWwindow* win) { _glfw.x11.disabledCursorWindow = NULL; releaseCursor(); SetCursorPosV(win,_glfw.x11.restoreCurPosX,_glfw.x11.restoreCurPosY); updateCursorImage(win); }
    void GetMonitorPos(_GLFWmonitor* monitor, int* xpos, int* ypos) {
        XRRScreenResources* sr = _glfw.x11.randr.GetScreenResourcesCurrent(_glfw.x11.display, _glfw.x11.root);
        XRRCrtcInfo* ci = _glfw.x11.randr.GetCrtcInfo(_glfw.x11.display, sr, monitor->x11.crtc);
        if (ci) { *xpos = ci->x; *ypos = ci->y; _glfw.x11.randr.FreeCrtcInfo(ci); }
        _glfw.x11.randr.FreeScreenResources(sr);
    }

    void SetWindowIcon(const GLFWimage* image) {
        int longCount=0;
        longCount+=2+image[0].width*image[0].height;
        unsigned long* icon=OS_Calloc(longCount,sizeof(unsigned long)), *target=icon;
        *target++=image[0].width; *target++=image[0].height;
        for (int j=0;j<image[0].width*image[0].height;++j) *target++=(((unsigned long)image[0].pixels[j*4+0])<<16)|(((unsigned long)image[0].pixels[j*4+1])<<8)|(((unsigned long)image[0].pixels[j*4+2])<<0)|(((unsigned long)image[0].pixels[j*4+3])<<24);
        _glfw.x11.xlib.ChangeProperty(_glfw.x11.display,((_GLFWwindow*)window)->x11.handle,_glfw.x11.NET_WM_ICON,((Atom) 6),32,0/*PropModeReplace*/,(unsigned char*)icon,longCount);
        OS_DeallocateRAM(icon,longCount*sizeof(unsigned long));
    }

    void GetWindowSize(_GLFWwindow* win, int* width, int* height) { XWindowAttributes attribs; _glfw.x11.xlib.GetWindowAttributes(_glfw.x11.display,win->x11.handle,&attribs); *width=attribs.width; *height=attribs.height; }
    void SetWindowSize(_GLFWwindow* win, int width, int height) { width=vmax(1,width); height=vmax(1,height); updateNormalHints(win,width,height); _glfw.x11.xlib.ResizeWindow(_glfw.x11.display,win->x11.handle,width,height); }
    void SetWindowMonitor(_GLFWwindow* win,int xpos,int ypos,int width,int height) {
        updateNormalHints(win,width,height);
        if (_glfw.x11.NET_WM_STATE && _glfw.x11.NET_WM_STATE_FULLSCREEN) sendEventToWM(win,_glfw.x11.NET_WM_STATE,0/*remove*/,_glfw.x11.NET_WM_STATE_FULLSCREEN,0,1,0);
        else {
            XSetWindowAttributes attributes; attributes.override_redirect=0;
            _glfw.x11.xlib.ChangeWindowAttributes(_glfw.x11.display,win->x11.handle,(1L<<9)/*override redirect*/,&attributes);
            win->x11.overrideRedirect=0;
        }
        
        _glfw.x11.xlib.DeleteProperty(_glfw.x11.display,win->x11.handle,_glfw.x11.NET_WM_BYPASS_COMPOSITOR);
        _glfw.x11.xlib.MoveResizeWindow(_glfw.x11.display,win->x11.handle,xpos,ypos,width,height);
    }
    
    i32 WindowFocused() { Window focused; int state; _glfw.x11.xlib.GetInputFocus(_glfw.x11.display,&focused,&state); return ((_GLFWwindow*)window)->x11.handle==focused; }
    i32 WindowVisible() { XWindowAttributes wa; _glfw.x11.xlib.GetWindowAttributes(_glfw.x11.display,((_GLFWwindow*)window)->x11.handle,&wa); return wa.map_state==2/*IsViewable*/; }
    void GetWindowPos(_GLFWwindow* win, int* xpos, int* ypos) { Window dummy; _glfw.x11.xlib.TranslateCoordinates(_glfw.x11.display,win->x11.handle,_glfw.x11.root,0,0,xpos,ypos,&dummy); }
    void SetWindowPos(_GLFWwindow* win, int xpos, int ypos) {
        if (!WindowVisible()) {
            long supplied; XSizeHints* hints=_glfw.x11.xlib.AllocSizeHints();
            if (_glfw.x11.xlib.GetWMNormalHints(_glfw.x11.display,win->x11.handle,hints,&supplied)) { hints->flags|=(1L << 2)/*PPosition*/; hints->x=hints->y=0; _glfw.x11.xlib.SetWMNormalHints(_glfw.x11.display,win->x11.handle,hints); }
            _glfw.x11.xlib.Free(hints);
        }
        _glfw.x11.xlib.MoveWindow(_glfw.x11.display,win->x11.handle,xpos,ypos);
    }

    void SetWindowDecorated(_GLFWwindow* win,i32 enabled) {
        struct { unsigned long flags,functions,decorations; long input_mode; unsigned long status; } hints={0};
        hints.flags=2; hints.decorations=enabled?1:0;
        _glfw.x11.xlib.ChangeProperty(_glfw.x11.display,win->x11.handle,_glfw.x11.MOTIF_WM_HINTS,_glfw.x11.MOTIF_WM_HINTS,32,0/*PropModeReplace*/,(unsigned char*)&hints,sizeof(hints)/sizeof(long));
    }

    void GetCursorPosV(_GLFWwindow* win, double* xpos, double* ypos) { Window root,child; int rootX,rootY,childX,childY; unsigned int mask; _glfw.x11.xlib.QueryPointer(_glfw.x11.display,win->x11.handle,&root,&child,&rootX,&rootY,&childX,&childY,&mask); *xpos=childX; *ypos=childY; }
    void SetCursorPosV(_GLFWwindow* win, double x, double y) { win->x11.warpCursorPosX=(int)x; win->x11.warpCursorPosY=(int)y; _glfw.x11.xlib.WarpPointer(_glfw.x11.display,0L,win->x11.handle,0,0,0,0,(int)x,(int)y); }
    void SetCursorMode(GLFWwindow* handle, int value) {
        _GLFWwindow* win = (_GLFWwindow*)handle;
        if (win->cursorMode != value) {
            win->cursorMode = value;
            GetCursorPosV(win,&win->virtualCursorPosX,&win->virtualCursorPosY);
            if (WindowFocused()) { GetCursorPosV(win,&_glfw.x11.restoreCurPosX,&_glfw.x11.restoreCurPosY); captureCursor(win); _glfw.x11.disabledCursorWindow=win; }
            else Sys_Global.gamePaused = true;
            
            updateCursorImage(win);
        }
    }
    
    static const XRRModeInfo* getModeInfo(const XRRScreenResources* sr, RRMode id) { for (int i = 0;  i < sr->nmode;  i++){ if (sr->modes[i].id == id) {return sr->modes + i;} } return NULL; }
    static GLFWvidmode vidmodeFromModeInfo(const XRRModeInfo* mi, const XRRCrtcInfo* ci) {
        GLFWvidmode mode;
        if (ci->rotation == 2 || ci->rotation == 8) {  mode.width  = mi->height; mode.height = mi->width; } // ==90, ==270
        else { mode.width = mi->width; mode.height = mi->height; }
        mode.refreshRate = (mi->hTotal && mi->vTotal) ? (int)vround((double) mi->dotClock / ((double) mi->hTotal * (double) mi->vTotal)) : 0;
        return mode;
    }

    void PollMonitors() {
        int disconnectedCount; _GLFWmonitor** disconnected = NULL;
        XRRScreenResources* sr = _glfw.x11.randr.GetScreenResourcesCurrent(_glfw.x11.display,_glfw.x11.root);
        RROutput primary = _glfw.x11.randr.GetOutputPrimary(_glfw.x11.display,_glfw.x11.root);
        disconnectedCount = _glfw.monitorCount;
        if (disconnectedCount) { disconnected = OS_Calloc(_glfw.monitorCount,sizeof(_GLFWmonitor*)); MemCpyFromBtoAForNBytes(disconnected,_glfw.monitors,_glfw.monitorCount * sizeof(_GLFWmonitor*)); }
        for (int i = 0;  i < sr->noutput;  i++) {
            int j, type, widthMM, heightMM;
            XRROutputInfo* oi = _glfw.x11.randr.GetOutputInfo(_glfw.x11.display, sr, sr->outputs[i]);
            if (oi->connection != 0/*connected*/ || oi->crtc == 0L) { _glfw.x11.randr.FreeOutputInfo(oi); continue; }

            for (j = 0;  j < disconnectedCount;  j++) {
                if (disconnected[j] && disconnected[j]->x11.output == sr->outputs[i]) { disconnected[j] = NULL; break; }
            }

            if (j < disconnectedCount) { _glfw.x11.randr.FreeOutputInfo(oi); continue; }

            XRRCrtcInfo* ci = _glfw.x11.randr.GetCrtcInfo(_glfw.x11.display, sr, oi->crtc);
            if (!ci) { _glfw.x11.randr.FreeOutputInfo(oi); continue; }

            if (ci->rotation == 2 || ci->rotation == 8) { widthMM  = oi->mm_height; heightMM = oi->mm_width; } // == 90, == 270
            else { widthMM  = oi->mm_width; heightMM = oi->mm_height; }
            
            if (widthMM <= 0 || heightMM <= 0) { widthMM  = (int) (ci->width * 25.4f / 96.f); heightMM = (int) (ci->height * 25.4f / 96.f); }
            _GLFWmonitor* monitor = AllocMonitor(oi->name, widthMM, heightMM);
            monitor->x11.output = sr->outputs[i]; monitor->x11.crtc   = oi->crtc;
            type = (monitor->x11.output == primary) ? 0 : 1; InputMonitor(monitor,0x00040001/*connected*/,type); _glfw.x11.randr.FreeOutputInfo(oi); _glfw.x11.randr.FreeCrtcInfo(ci);
        }

        _glfw.x11.randr.FreeScreenResources(sr);
        for (int i = 0;  i < disconnectedCount;  i++) { if (disconnected[i]) {InputMonitor(disconnected[i],0x00040002/*disconnected*/,0);} }
        if (disconnected) OS_DeallocateRAM(disconnected,_glfw.monitorCount*sizeof(_GLFWmonitor*));
    }
    
    static void processEvent(XEvent* event) {
        unsigned int keycode=0; Bool filtered=0;
        if (event->type==2/*KeyPress*/ || event->type==3/*KeyRelease*/) keycode=event->xkey.keycode;
        filtered=_glfw.x11.xlib.FilterEvent(event,0L);
        if (event->type==_glfw.x11.randr.eventBase+1/*notify*/) { _glfw.x11.randr.UpdateConfiguration(event); PollMonitors(); return; }
        if (event->type==35/*GenericEvent*/) return;
        _GLFWwindow* win=NULL; if (_glfw.x11.xlib.FindContext(_glfw.x11.display,event->xany.window,_glfw.x11.context,(XPointer*)&win)!=0) return;

        switch (event->type) {
            case 21/*ReparentNotify*/: win->x11.parent=event->xreparent.parent; return;
            case 2/*KeyPress*/:
            case 3/*KeyRelease*/: {
                const int key=translateKey(keycode),action=(event->type==2/*KeyPress*/)?INPUT_PRESS:INPUT_RELEASE;
                if (key!=KEY_UNKNOWN) InputKey(win,key,action);
                return;
            }
            case 4/*ButtonPress*/: {
                if      (event->xbutton.button==1) InputMouseClick(win,MOUSE_BUTTON_LEFT,INPUT_PRESS);
                else if (event->xbutton.button==2) InputMouseClick(win,MOUSE_BUTTON_MIDDLE,INPUT_PRESS);
                else if (event->xbutton.button==3) InputMouseClick(win,MOUSE_BUTTON_RIGHT,INPUT_PRESS);
                else if (event->xbutton.button==4) Sys_Input.scrollDelta += 1.0;
                else if (event->xbutton.button==5) Sys_Input.scrollDelta += -1.0;
                else InputMouseClick(win,event->xbutton.button - 1 - 4,INPUT_PRESS);
                return;
            }
            case 5/*ButtonRelease*/: {
                if      (event->xbutton.button==1) InputMouseClick(win,MOUSE_BUTTON_LEFT,INPUT_RELEASE);
                else if (event->xbutton.button==2) InputMouseClick(win,MOUSE_BUTTON_MIDDLE,INPUT_RELEASE);
                else if (event->xbutton.button==3) InputMouseClick(win,MOUSE_BUTTON_RIGHT,INPUT_RELEASE);
                else if (event->xbutton.button>7)  InputMouseClick(win,event->xbutton.button - 1 - 4,INPUT_RELEASE);
                return;
            }
            case 7/*EnterNotify*/: {
                const int x=event->xcrossing.x,y=event->xcrossing.y;
                InputCursorPos(win,x,y);
                win->x11.lastCursorPosX=x; win->x11.lastCursorPosY=y;
                return;
            }
            case 6/*MotionNotify*/: {
                const int x=event->xmotion.x,y=event->xmotion.y;
                if (x!=win->x11.warpCursorPosX || y!=win->x11.warpCursorPosY) {
                    if (win->cursorMode==0x00034003/*CURSOR_DISABLED*/) {
                        if (_glfw.x11.disabledCursorWindow!=win) return;
                        InputCursorPos(win,win->virtualCursorPosX + (x - win->x11.lastCursorPosX),win->virtualCursorPosY + (y - win->x11.lastCursorPosY));
                    } else InputCursorPos(win,x,y);
                }
                win->x11.lastCursorPosX=x; win->x11.lastCursorPosY=y;
                return;
            }
            case 22/*ConfigureNotify*/: {
                if (event->xconfigure.width!=win->x11.width || event->xconfigure.height!=win->x11.height) { win->x11.width=event->xconfigure.width; win->x11.height=event->xconfigure.height; UpdateScreenSize(event->xconfigure.width,event->xconfigure.height); }
                int xpos=event->xconfigure.x,ypos=event->xconfigure.y;
                if (!event->xany.send_event && win->x11.parent!=_glfw.x11.root) {
                    Window dummy;
                    _glfw.x11.xlib.TranslateCoordinates(_glfw.x11.display,win->x11.parent,_glfw.x11.root,xpos,ypos,&xpos,&ypos,&dummy);
                }
                if (xpos!=win->x11.xpos || ypos!=win->x11.ypos) { win->x11.xpos=xpos; win->x11.ypos=ypos; }
                return;
            }
            case 33/*ClientMessage*/: {
                if (filtered || event->xclient.message_type==0L) return;
                
                if (event->xclient.message_type==_glfw.x11.WM_PROTOCOLS) {
                    const Atom protocol=event->xclient.data.l[0];
                    if (protocol==0L) return;
                    
                    if (protocol == _glfw.x11.WM_DELETE_WINDOW) OS_Exit(0);
                    if (protocol == _glfw.x11.NET_WM_PING) {
                        XEvent reply=*event; reply.xclient.window=_glfw.x11.root;
                        _glfw.x11.xlib.SendEvent(_glfw.x11.display,_glfw.x11.root,0,(1L<<19)|(1L<<20),&reply);
                    }
                }
                return;
            }
            case 9/*FocusIn*/: {
                if (event->xfocus.mode==1/*NotifyGrab*/ || event->xfocus.mode==2/*NotifyUngrab*/) return;
                if (win->cursorMode==0x00034003/*CURSOR_DISABLED*/) disableCursor(win);
                if (win->x11.ic) _glfw.x11.xlib.SetICFocus(win->x11.ic);
                InputWindowFocus(1);
                return;
            }
            case 10/*FocusOut*/: {
                if (event->xfocus.mode==1/*NotifyGrab*/ || event->xfocus.mode==2/*NotifyUngrab*/) return;
                if (win->cursorMode==0x00034003/*CURSOR_DISABLED*/) enableCursor(win);
                if (win->x11.ic) _glfw.x11.xlib.UnsetICFocus(win->x11.ic);
                InputWindowFocus(0);
                return;
            }
        }
    }

    void GetMonitorWorkarea(_GLFWmonitor* monitor,int* xpos,int* ypos,int* width,int* height) {
        int areaX = 0, areaY = 0, areaWidth = 0, areaHeight = 0;
        XRRScreenResources* sr = _glfw.x11.randr.GetScreenResourcesCurrent(_glfw.x11.display,_glfw.x11.root);
        XRRCrtcInfo* ci = _glfw.x11.randr.GetCrtcInfo(_glfw.x11.display,sr,monitor->x11.crtc);
        const XRRModeInfo* mi = getModeInfo(sr,ci->mode);
        areaX = ci->x, areaY = ci->y;
        if (ci->rotation == 2 || ci->rotation == 8) { areaWidth = mi->height, areaHeight = mi->width; } // ==90, ==270
        else { areaWidth = mi->width, areaHeight = mi->height; }
        _glfw.x11.randr.FreeCrtcInfo(ci); _glfw.x11.randr.FreeScreenResources(sr);
        if (_glfw.x11.NET_WORKAREA && _glfw.x11.NET_CURRENT_DESKTOP) {
            Atom *extents = NULL, *desktop = NULL;
            const unsigned long extentCount = _glfwGetWindowPropertyX11(_glfw.x11.root,_glfw.x11.NET_WORKAREA,((Atom) 6),(unsigned char**) &extents);
            if (_glfwGetWindowPropertyX11(_glfw.x11.root,_glfw.x11.NET_CURRENT_DESKTOP,((Atom) 6),(unsigned char**) &desktop) > 0) {
                if (extentCount >= 4 && *desktop < extentCount / 4) {
                    const int gx = extents[*desktop * 4 + 0], gy = extents[*desktop * 4 + 1], gw = extents[*desktop * 4 + 2], gh = extents[*desktop * 4 + 3];
                    if (areaX < gx) { areaWidth  -= gx - areaX, areaX = gx; }
                    if (areaY < gy) { areaHeight -= gy - areaY, areaY = gy; }
                    if (areaX +  areaWidth > gx + gw)  areaWidth = gx - areaX + gw;
                    if (areaY + areaHeight > gy + gh) areaHeight = gy - areaY + gh;
                }
            }
            if (extents) {_glfw.x11.xlib.Free(extents);} if (desktop) {_glfw.x11.xlib.Free(desktop);}
        }
        *xpos = areaX; *ypos = areaY; *width = areaWidth; *height = areaHeight;
    }

    void GetVideoMode(_GLFWmonitor* monitor,GLFWvidmode* mode) {
        XRRScreenResources* sr = _glfw.x11.randr.GetScreenResourcesCurrent(_glfw.x11.display,_glfw.x11.root);
        const XRRModeInfo* mi = NULL;
        XRRCrtcInfo* ci = _glfw.x11.randr.GetCrtcInfo(_glfw.x11.display,sr,monitor->x11.crtc);
        if (ci) { mi = getModeInfo(sr,ci->mode); if (mi) {*mode = vidmodeFromModeInfo(mi,ci);} _glfw.x11.randr.FreeCrtcInfo(ci); }
        _glfw.x11.randr.FreeScreenResources(sr);
    }

    static int translateKeySyms(const KeySym* keysyms, int width) {
        if (width > 1) { // Numpad with numlock ON (keysyms[1]) - contiguous 0xffb0..0xffb9
            if (keysyms[1] >= 0xffb0 && keysyms[1] <= 0xffb9) return KEY_KP_0 + (keysyms[1] - 0xffb0);
            switch (keysyms[1]) {
                case 0xffac: case 0xffae: return KEY_KP_DECIMAL; // KP_Separator, KP_Decimal
                case 0xffbd:              return KEY_KP_EQUAL;   // KP_Equal
                case 0xff8d:              return KEY_KP_ENTER;   // KP_Enter
                default: break;
            }
        }

        KeySym k = keysyms[0];
        if (k >= 0x0061 && k <= 0x007a) return KEY_A + (k - 0x0061);  // a-z
        if (k >= 0x0030 && k <= 0x0039) return KEY_0 + (k - 0x0030);  // 0-9
        if (k >= 0xffbe && k <= 0xffd6) return KEY_F1 + (k - 0xffbe); // F1..F25: 0xffbe..0xffd6
        if (k >= 0xff95 && k <= 0xff9f) { // KP numpad with numlock OFF (cursor keys): 0xff95..0xff9f
            static const int kp_off[] = {KEY_KP_7,KEY_KP_4,KEY_KP_8,KEY_KP_6,KEY_KP_2,KEY_KP_9,KEY_KP_3,KEY_KP_1,-1,KEY_KP_0,KEY_KP_DECIMAL };
            int r = kp_off[k - 0xff95];
            if (r != -1) return r;
        }
        switch (k) {
            case 0xff1b: return KEY_ESCAPE;        case 0xff09: return KEY_TAB;          case 0xff0d: return KEY_ENTER;
            case 0xff08: return KEY_BACKSPACE;     case 0xffff: return KEY_DELETE;       case 0xff50: return KEY_HOME;
            case 0xff57: return KEY_END;           case 0xff55: return KEY_PAGE_UP;      case 0xff56: return KEY_PAGE_DOWN;
            case 0xff63: return KEY_INSERT;        case 0xff51: return KEY_LEFT;         case 0xff53: return KEY_RIGHT;
            case 0xff54: return KEY_DOWN;          case 0xff52: return KEY_UP;           case 0xff13: return KEY_PAUSE;
            case 0xff14: return KEY_SCROLL_LOCK;   case 0xff61: return KEY_PRINT_SCREEN; case 0xff7f: return KEY_NUM_LOCK;
            case 0xffe5: return KEY_CAPS_LOCK;     case 0xff67: return KEY_MENU;         case 0xffe1: return KEY_LEFT_SHIFT;
            case 0xffe2: return KEY_RIGHT_SHIFT;   case 0xffe3: return KEY_LEFT_CONTROL; case 0xffe4: return KEY_RIGHT_CONTROL;
            case 0xffe7: case 0xffe9: return KEY_LEFT_ALT;   // Meta_L, Alt_L
            case 0xff7e: case 0xfe03: case 0xffe8: case 0xffea: return KEY_RIGHT_ALT; // Mode_switch, ISO_Level3_Shift, Meta_R, Alt_R
            case 0xffeb: return KEY_LEFT_SUPER;    case 0xffec: return KEY_RIGHT_SUPER;  case 0xffaa: return KEY_KP_MULTIPLY;
            case 0xffab: return KEY_KP_ADD;        case 0xffad: return KEY_KP_SUBTRACT;  case 0xffaf: return KEY_KP_DIVIDE;
            case 0xffbd: return KEY_KP_EQUAL;      case 0xff8d: return KEY_KP_ENTER;     case 0x0020: return KEY_SPACE;
            case 0x0027: return KEY_APOSTROPHE;    case 0x002c: return KEY_COMMA;        case 0x002d: return KEY_MINUS;
            case 0x002e: return KEY_PERIOD;        case 0x002f: return KEY_SLASH;        case 0x003b: return KEY_SEMICOLON;
            case 0x003d: return KEY_EQUAL;         case 0x005b: return KEY_LEFT_BRACKET; case 0x005c: return KEY_BACKSLASH;
            case 0x005d: return KEY_RIGHT_BRACKET; case 0x0060: return KEY_GRAVE_ACCENT; default: return KEY_UNKNOWN;
        }
    }

    static void createKeyTables() {
        int scancodeMin, scancodeMax;
        MemSetToVForNBytes(_glfw.x11.keycodes,-1,sizeof(_glfw.x11.keycodes));
        MemSetToVForNBytes(_glfw.x11.scancodes,-1,sizeof(_glfw.x11.scancodes));
        _glfw.x11.xlib.DisplayKeycodes(_glfw.x11.display,&scancodeMin,&scancodeMax);
        int width; KeySym* keysyms = _glfw.x11.xlib.GetKeyboardMapping(_glfw.x11.display,scancodeMin,scancodeMax - scancodeMin + 1,&width);
        for (int sc = scancodeMin; sc <= scancodeMax; sc++) {
            if (_glfw.x11.keycodes[sc] < 0) _glfw.x11.keycodes[sc] = translateKeySyms(&keysyms[(sc - scancodeMin) * width],width);
            if (_glfw.x11.keycodes[sc] > 0) _glfw.x11.scancodes[_glfw.x11.keycodes[sc]] = sc;
        }
        _glfw.x11.xlib.Free(keysyms);
    }

    static Atom getAtomIfSupported(Atom* atoms, unsigned long count, const char* name) { const Atom atom=_glfw.x11.xlib.InternAtom(_glfw.x11.display,name,0); for (unsigned long i=0;i<count;i++) {if (atoms[i] == atom) {return atom;}} return 0L; }
    static void handleKeyEvent(_GLFWjoystick* js, int code, int value) { InputJoystickButton(js,js->linjs.keyMap[code - 0x100/*BTN_MISC*/],value ? INPUT_PRESS : INPUT_RELEASE); }
    static void handleAbsEvent(_GLFWjoystick* js, int code, int value) {
        const int index = js->linjs.absMap[code];
        if (code >= 0x10/*ABS_HAT0X*/ && code <= 0x17/*ABS_HAT3Y*/) {
            static const char stateMap[3][3] = {{JOYHAT_CENTERED,JOYHAT_UP,JOYHAT_DOWN},{JOYHAT_LEFT,JOYHAT_LEFT_UP,JOYHAT_LEFT_DOWN},{JOYHAT_RIGHT,JOYHAT_RIGHT_UP,JOYHAT_RIGHT_DOWN},};
            const int hat = (code - 0x10/*ABS_HAT0X*/) / 2, axis = (code - 0x10/*ABS_HAT0X*/) % 2;
            int* state = js->linjs.hats[hat];
            state[axis] = (value == 0) ? 0 : value < 0 ? 1 : value > 0 ? 2 : state[axis];
            InputJoystickHat(js, index, stateMap[state[0]][state[1]]);
        } else {
            const struct input_absinfo* info = &js->linjs.absInfo[code];
            float normalized = value;
            const int range = info->maximum - info->minimum;
            if (range) { normalized = (normalized - info->minimum) / range; normalized = normalized * 2.0f - 1.0f; }
            InputJoystickAxis(js, index, normalized);
        }
    }

    static void pollAbsState(_GLFWjoystick* js) {
        for (int code=0;code<0x40/*ABS_CNT*/;code++) {
            if (js->linjs.absMap[code] < 0) continue;

            struct input_absinfo* info = &js->linjs.absInfo[code];
            if (OS_IOControl(js->linjs.fd,(0x80184540 + (code)),info) < 0) continue;

            handleAbsEvent(js, code, info->value);
        }
    }

    #define isBitSet(bit, arr) (arr[(bit) / 8] & (1 << ((bit) % 8)))
    #define EVIOCGBIT(ev, len) (0x80004520 + (ev) + ((len) << 16))
    static i32 openJoystickDevice(const char* path) {
        for (int jid = 0;  jid <= JOYSTICK_LAST;  jid++) {
            if (!_glfw.joysticks[jid].connected) continue;
            if (StringsEqual(_glfw.joysticks[jid].linjs.path,path)) return 0;
        }

        _GLFWjoystickLinux linjs = {0}; linjs.fd = OS_Open(path,00004000|02000000,0); if (linjs.fd == -1) return 0;

        char evBits[(0x20/*EV_CNT*/ + 7) / 8] = {0},keyBits[(0x300/*KEY_CNT*/ + 7) / 8] = {0},absBits[(0x40/*ABS_CNT*/ + 7) / 8] = {0};
        struct input_id id; if (OS_IOControl(linjs.fd,EVIOCGBIT(0,sizeof(evBits)),evBits) < 0 || OS_IOControl(linjs.fd,EVIOCGBIT(0x01/*EV_KEY*/,sizeof(keyBits)),keyBits) < 0 || OS_IOControl(linjs.fd,EVIOCGBIT(0x03/*EV_ABS*/,sizeof(absBits)),absBits) < 0 || OS_IOControl(linjs.fd,0x80084501/*EVIOCGID*/,&id) < 0) { OS_Close(linjs.fd); return 0; }
        if (!isBitSet(0x03/*EV_ABS*/,evBits)) { OS_Close(linjs.fd); return 0; }

        char name[256] = "",guid[33] = "";
        if (OS_IOControl(linjs.fd,(0x80004506 | (((sizeof(name)) & 0x1fff) << 16)),name) < 0) StringCopyInto_A_From_B(name,"Unknown",sizeof(name));
        if (id.vendor && id.product && id.version) StringFormat(guid,sizeof(guid),"%02x%02x0000%02x%02x0000%02x%02x0000%02x%02x0000",id.bustype & 0xff, id.bustype >> 8,id.vendor & 0xff,  id.vendor >> 8,id.product & 0xff, id.product >> 8,id.version & 0xff, id.version >> 8);
        else StringFormat(guid,sizeof(guid),"%02x%02x0000%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x00",id.bustype & 0xff, id.bustype >> 8,name[0], name[1], name[2], name[3],name[4], name[5], name[6], name[7],name[8], name[9], name[10]);

        int axisCount = 0, buttonCount = 0, hatCount = 0;
        for (int code=0x100/*BTN_MISC*/;code<0x300/*KEY_CNT*/;code++) {
            if (!isBitSet(code,keyBits)) continue;

            linjs.keyMap[code - 0x100/*BTN_MISC*/] = buttonCount++;
        }

        for (int code=0;code<0x40/*ABS_CNT*/;code++) {
            linjs.absMap[code] = -1; if (!isBitSet(code,absBits)) continue;

            if (code >= 0x10/*ABS_HAT0X*/ && code <= 0x17/*ABS_HAT3Y*/) { linjs.absMap[code] = hatCount; hatCount++; code++; } // Skip the Y axis
            else {
                if (OS_IOControl(linjs.fd,(0x80184540 + (code)), &linjs.absInfo[code]) < 0) continue;

                linjs.absMap[code] = axisCount++;
            }
        }

        _GLFWjoystick* js = _glfwAllocJoystick(name,guid,axisCount,buttonCount,hatCount);
        if (!js) { OS_Close(linjs.fd); return 0; }

        StringCopyInto_A_From_B(linjs.path,path,sizeof(linjs.path));
        MemCpyFromBtoAForNBytes(&js->linjs,&linjs,sizeof(linjs));
        pollAbsState(js); JoystickConnection(js,0x00040001/*connected*/);
        return  1;
    }
    
    struct linux_dirent64 { u64 d_ino; i64 d_off; unsigned short d_reclen; unsigned char d_type; char d_name[]; };
    struct inotify_event { i32 wd; u32 mask,cookie,len; char name[]; };
    static void closeJoystick(_GLFWjoystick* js) { JoystickConnection(js,0x00040002/*disconnected*/); if (js->linjs.fd > 0) { OS_Close(js->linjs.fd); js->linjs.fd = -1; } _glfwFreeJoystick(js); }    
    static i32 isEventDevice(const char* name) { if (!name || !StringCompareUpToLength(name, "event", 5) || name[5] == '\0') {return 0;} for (const char* p=name+5;*p;++p) if (*p < '0' || *p > '9') {return 0;} return 1; }
    static void iterateInputDevices(void (*callback)(const char* fullpath)) {
        const char* dirname = "/dev/input"; FHandle fd = OS_Open(dirname,00200000|02000000,0); if (fd < 0) return;

        char buf[8192];
        for (;;) {
            register long rax __asm__("rax") = 217/*__NR_getdents64*/, rdi __asm__("rdi") = fd; register char* rsi __asm__("rsi") = buf; register size_t rdx __asm__("rdx") = sizeof(buf);
            __asm__ __volatile__("syscall":"+r"(rax):"r"(rdi),"r"(rsi),"r"(rdx):"rcx","r11","memory"); if (rax <= 0) break;

            long offset = 0;
            while (offset < rax) {
                struct linux_dirent64* d = (struct linux_dirent64*)(buf + offset);
                if (d->d_name[0] != '.' && isEventDevice(d->d_name)) { char path[260]; StringFormat(path,sizeof(path),"%s/%s",dirname,d->d_name); callback(path); }
                offset += d->d_reclen;
            }
        }

        OS_Close(fd);
    }
    
    static void openJoystickCallback(const char* path) { openJoystickDevice(path); }
    static char joyConbuffer[16384],joyPath[260];
    void _glfwDetectJoystickConnectionLinux() {
        if (_glfw.linjs.inotify <= 0) return;
        i32 size = OS_Read(_glfw.linjs.inotify,joyConbuffer,sizeof(joyConbuffer)); if (size <= 0) return;
        
        i32 offset = 0;
        while (size >= offset + (i32)sizeof(struct inotify_event)) {
            const struct inotify_event* e = (struct inotify_event*)(joyConbuffer + offset);
            offset += (i32)sizeof(struct inotify_event) + e->len;
            if (e->len == 0 || !isEventDevice(e->name)) continue;

            StringFormat(joyPath,sizeof(joyPath), "/dev/input/%s", e->name);
            if (e->mask & (0x00000100/*IN_CREATE*/|0x00000004/*IN_ATTRIB*/)) openJoystickDevice(joyPath);
            else if (e->mask & 0x00000200/*IN_DELETE*/) {
                for (int jid = 0; jid <= JOYSTICK_LAST; jid++) {
                    if (StringsEqual(_glfw.joysticks[jid].linjs.path,joyPath)) { closeJoystick(_glfw.joysticks + jid); break; }
                }
            }
        }
    }

    i32 InitJoysticks() {
        const char* dirname = "/dev/input";
        {register long rax __asm__("rax") = 294/*__NR_inotify_init1*/; register unsigned int rdi __asm__("rdi") = 0x800/*IN_NONBLOCK*/|0x80000/*IN_CLOEXEC*/; 
        __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi) : "rcx", "r11", "memory");
        _glfw.linjs.inotify = (int)rax; }
        if (_glfw.linjs.inotify >= 0) {
            register long rax __asm__("rax") = 295/*__NR_inotify_add_watch*/; register int rdi __asm__("rdi") = _glfw.linjs.inotify; register const char* rsi __asm__("rsi") = dirname;
            register u32 rdx __asm__("rdx") = 0x00000100/*IN_CREATE*/|0x00000004/*IN_ATTRIB*/|0x00000200/*IN_DELETE*/;
            __asm__ __volatile__("syscall":"+r"(rax):"r"(rdi),"r"(rsi),"r"(rdx):"rcx","r11","memory");
            _glfw.linjs.watch = (int)rax;
        }
        
        iterateInputDevices(openJoystickCallback);
        return 1;
    }

    i32 PollJoystick(_GLFWjoystick* js) {
        if (js->linjs.fd <= 0) return 0;

        for (;;) {
            struct input_event e;
            long n = OS_Read(js->linjs.fd, &e, sizeof(e));
            if (n < 0) { closeJoystick(js); break; }
            if (n == 0) { break; }
            if (n < (long)sizeof(e)) { closeJoystick(js); break; }

            if (e.type == 0x00/*EV_SYN*/) {
                if (e.code == 3/*SYN_DROPPED*/) _glfw.linjs.dropped = 1;
                else if (e.code == 0/*SYN_REPORT*/) { _glfw.linjs.dropped = 0; pollAbsState(js); }
            }

            if (_glfw.linjs.dropped) continue;

                 if (e.type == 0x01/*EV_KEY*/) handleKeyEvent(js,e.code,e.value);
            else if (e.type == 0x03/*EV_ABS*/) handleAbsEvent(js,e.code,e.value);
        }

        return js->connected;
    }
    
    void PollEvents() {
        if (_glfw.joysticksInitialized) _glfwDetectJoystickConnectionLinux();
        _glfw.x11.xlib.Pending(_glfw.x11.display);
        while (((_XPrivDisplay)(_glfw.x11.display))->qlen) { XEvent e; XNextEvent(_glfw.x11.display,&e); processEvent(&e); }
        _GLFWwindow* win = _glfw.x11.disabledCursorWindow;
        if (win) {
            int width,height; GetWindowSize(win,&width,&height);
            if (win->x11.lastCursorPosX!=width/2 || win->x11.lastCursorPosY!=height/2) SetCursorPosV(win,width/2,height/2);
        }
    }

    static int getGLXFBConfigAttrib(GLXFBConfig fbconfig, int attrib) { int value; _glfw.glx.GetFBConfigAttrib(_glfw.x11.display, fbconfig, attrib, &value); return value; }
    static void makeContextCurrentGLX(_GLFWwindow* win) { _glfw.glx.MakeCurrent(_glfw.x11.display,win->context.glx.window,win->context.glx.handle); }
    static void swapBuffersGLX(_GLFWwindow* win) { _glfw.glx.SwapBuffers(_glfw.x11.display, win->context.glx.window); }
    static void swapIntervalGLX(int interval) { _GLFWwindow* handle = (_GLFWwindow*)window; _glfw.glx.SwapIntervalEXT(_glfw.x11.display,handle->context.glx.window,interval); }
    static GLFWglproc getProcAddressGLX(const char* procname) { return _glfw.glx.GetProcAddress((const u8*) procname); }
    void glfwSetWindowPosition(GLFWwindow* handle, int xpos, int ypos) { _GLFWwindow* win = (_GLFWwindow*)handle; SetWindowPos(win,xpos,ypos); }
#endif
_GLFWlibrary _glfw={0};
int WindowInit() {
    MemSetToVForNBytes(&_glfw,0,sizeof(_glfw));
    #if defined(WINDOWS)
        GetModuleHandleExW(0x4|0x2,(const u16*)&_glfw,(HMODULE*)&_glfw.win32.instance);
        const char* names[] = {"xinput1_4.dll","xinput1_3.dll","xinput9_1_0.dll","xinput1_2.dll","xinput1_1.dll",NULL};
        for (int i=0;names[i];++i) {
            _glfw.win32.xinput.instance = LoadLibraryA(names[i]);
            if (_glfw.win32.xinput.instance) {
                _glfw.win32.xinput.GetCapabilities = (PFN_XInputGetCapabilities)PlatformGetModuleSymbol(_glfw.win32.xinput.instance, "XInputGetCapabilities");
                _glfw.win32.xinput.GetState = (PFN_XInputGetState)PlatformGetModuleSymbol(_glfw.win32.xinput.instance, "XInputGetState");
                break;
            }
        }

        _glfw.win32.dwmapi.instance = LoadLibraryA("dwmapi.dll");
        if (_glfw.win32.dwmapi.instance) {
            _glfw.win32.dwmapi.IsCompositionEnabled = (PFN_DwmIsCompositionEnabled)PlatformGetModuleSymbol(_glfw.win32.dwmapi.instance, "DwmIsCompositionEnabled");
            _glfw.win32.dwmapi.Flush = (PFN_DwmFlush)PlatformGetModuleSymbol(_glfw.win32.dwmapi.instance, "DwmFlush");
        }

        _glfw.win32.ntdll.instance = LoadLibraryA("ntdll.dll");
        if (_glfw.win32.ntdll.instance) _glfw.win32.ntdll.RtlVerifyVersionInfo = (PFN_RtlVerifyVersionInfo)PlatformGetModuleSymbol(_glfw.win32.ntdll.instance, "RtlVerifyVersionInfo");
        createKeyTables();
        MSG msg; WNDCLASSEXW wc={0}; wc.cbSize=sizeof(wc); // Start making of a helper window
        wc.style = 0x0020/*CS_OWNDC*/; wc.lpfnWndProc = (WNDPROC)helperWindowProc; wc.hInstance = _glfw.win32.instance; wc.lpszClassName = L"GLFW3 Helper";
        _glfw.win32.helperWindowClass = RegisterClassExW(&wc);
        _glfw.win32.helperWindowHandle = CreateWindowExW(0x00000100/*WS_EX_WINDOWEDGE*/ | 0x00000200/*WS_EX_CLIENTEDGE*/,(u16*)MAKEINTATOM(_glfw.win32.helperWindowClass),L"GLFW message window",0x04000000/*WS_CLIPSIBLINGS*/|0x02000000/*WS_CLIPCHILDREN*/,0,0,1,1,NULL,NULL,_glfw.win32.instance,NULL);
        ShowWindow(_glfw.win32.helperWindowHandle,0);
        DEV_BROADCAST_DEVICEINTERFACE_W dbi;
        MemSetToVForNBytes(&dbi,0,sizeof(dbi));
        dbi.dbcc_size = sizeof(dbi);
        dbi.dbcc_devicetype = 0x0005/*DBT_DEVTYP_DEVICEINTERFACE*/;
        dbi.dbcc_classguid = (GUID){0x4d1e55b2,0xf16f,0x11cf,{0x88,0xcb,0x00,0x11,0x11,0x00,0x00,0x30}};
        _glfw.win32.deviceNotificationHandle = RegisterDeviceNotificationW(_glfw.win32.helperWindowHandle,(DEV_BROADCAST_HDR*)&dbi,0);
        while (PeekMessageW(&msg, _glfw.win32.helperWindowHandle,0,0,0x0001)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
        _glfwPollMonitorsWin32();
    #else
        void* module = _glfwPlatformLoadModule("libX11.so.6");
        PFN_XInitThreads XInitThreads = (PFN_XInitThreads)PlatformGetModuleSymbol(module,"XInitThreads");
        PFN_XOpenDisplay XOpenDisplay = (PFN_XOpenDisplay)PlatformGetModuleSymbol(module,"XOpenDisplay");
        XInitThreads();
        Display* display = XOpenDisplay(NULL);
        _glfw.x11.display = display;
        _glfw.x11.xlib.handle = module;
        #define X(n) _glfw.x11.xlib.n = (PFN_X##n)PlatformGetModuleSymbol(_glfw.x11.xlib.handle, "X" #n);
            X(AllocSizeHints)        X(ChangeProperty)       X(ChangeWindowAttributes) X(CheckTypedWindowEvent) X(CreateColormap)       X(CreateWindow)
            X(DefineCursor)          X(DeleteProperty)       X(DisplayKeycodes)        X(FilterEvent)           X(FindContext)          X(Free)
            X(FreeEventData)         X(GetInputFocus)        X(GetKeyboardMapping)     X(GetWMNormalHints)      X(GetWindowAttributes)  X(GetWindowProperty)
            X(GrabPointer)           X(InternAtom)           X(MapWindow)              X(MoveResizeWindow)      X(MoveWindow)           X(Pending)
            X(QueryExtension)        X(QueryPointer)         X(RaiseWindow)            X(ResizeWindow)          X(SaveContext)          X(SendEvent)
            X(SetICFocus)            X(SetInputFocus)        X(SetWMNormalHints)       X(SetWMProtocols)        X(TranslateCoordinates) X(UndefineCursor)
            X(UngrabPointer)         X(UnsetICFocus)         X(WarpPointer)
        #undef X
            
        XNextEvent = (PFN_XNextEvent)PlatformGetModuleSymbol(_glfw.x11.xlib.handle,"XNextEvent");
        _glfw.x11.screen = ((_XPrivDisplay)(_glfw.x11.display))->default_screen;
        _glfw.x11.root = (&((_XPrivDisplay)(_glfw.x11.display))->screens[_glfw.x11.screen])->root;
        static XContext lastContext = 0;
        _glfw.x11.context = ++lastContext;
        _glfw.x11.randr.handle = _glfwPlatformLoadModule("libXrandr.so.2");
        #define X(n) _glfw.x11.randr.n = (PFN_XRR##n)PlatformGetModuleSymbol(_glfw.x11.randr.handle,"XRR"#n);
            X(FreeCrtcInfo) X(FreeOutputInfo) X(FreeScreenResources) X(GetCrtcInfo) X(GetOutputInfo) X(GetOutputPrimary) X(GetScreenResourcesCurrent) X(SelectInput) X(UpdateConfiguration)
        #undef X
            
        XRRScreenResources* sr = _glfw.x11.randr.GetScreenResourcesCurrent(_glfw.x11.display,_glfw.x11.root);
        _glfw.x11.randr.FreeScreenResources(sr);
        _glfw.x11.randr.SelectInput(_glfw.x11.display,_glfw.x11.root,(1L << 2)/*change notify mask*/);
        _glfw.x11.xcursor.handle = _glfwPlatformLoadModule("libXcursor.so.1");
        _glfw.x11.xcursor.ImageCreate     = (PFN_XcursorImageCreate)    PlatformGetModuleSymbol(_glfw.x11.xcursor.handle,"XcursorImageCreate");
        _glfw.x11.xcursor.ImageDestroy    = (PFN_XcursorImageDestroy)   PlatformGetModuleSymbol(_glfw.x11.xcursor.handle,"XcursorImageDestroy");
        _glfw.x11.xcursor.ImageLoadCursor = (PFN_XcursorImageLoadCursor)PlatformGetModuleSymbol(_glfw.x11.xcursor.handle,"XcursorImageLoadCursor");
        createKeyTables();
        #define IA(n) _glfw.x11.xlib.InternAtom(_glfw.x11.display,n,0)
            _glfw.x11.UTF8_STRING     = IA("UTF8_STRING");     _glfw.x11.WM_PROTOCOLS = IA("WM_PROTOCOLS");  _glfw.x11.WM_STATE               = IA("WM_STATE");
            _glfw.x11.WM_DELETE_WINDOW= IA("WM_DELETE_WINDOW");_glfw.x11.NET_SUPPORTED= IA("_NET_SUPPORTED");_glfw.x11.NET_SUPPORTING_WM_CHECK= IA("_NET_SUPPORTING_WM_CHECK");
            _glfw.x11.NET_WM_ICON     = IA("_NET_WM_ICON");    _glfw.x11.NET_WM_PING  = IA("_NET_WM_PING");  _glfw.x11.NET_WM_NAME            = IA("_NET_WM_NAME");
            _glfw.x11.NET_WM_BYPASS_COMPOSITOR = IA("_NET_WM_BYPASS_COMPOSITOR");                            _glfw.x11.MOTIF_WM_HINTS         = IA("_MOTIF_WM_HINTS");
        #undef IA
            
        Window* wfr = NULL; _glfwGetWindowPropertyX11(_glfw.x11.root,_glfw.x11.NET_SUPPORTING_WM_CHECK,((Atom) 33),(unsigned char**)&wfr);
        Window* wfc = NULL; _glfwGetWindowPropertyX11(*wfr,_glfw.x11.NET_SUPPORTING_WM_CHECK,((Atom) 33),(unsigned char**)&wfc);
        _glfw.x11.xlib.Free(wfr); _glfw.x11.xlib.Free(wfc);
        Atom* sa = NULL; const unsigned long ac = _glfwGetWindowPropertyX11(_glfw.x11.root,_glfw.x11.NET_SUPPORTED,((Atom) 4),(unsigned char**)&sa);
        #define GA(name) getAtomIfSupported(sa, ac, name)
            _glfw.x11.NET_WM_STATE              = GA("_NET_WM_STATE");       _glfw.x11.NET_WM_STATE_FULLSCREEN   = GA("_NET_WM_STATE_FULLSCREEN");
            _glfw.x11.NET_WM_WINDOW_TYPE        = GA("_NET_WM_WINDOW_TYPE"); _glfw.x11.NET_WM_WINDOW_TYPE_NORMAL = GA("_NET_WM_WINDOW_TYPE_NORMAL");
            _glfw.x11.NET_WORKAREA              = GA("_NET_WORKAREA");       _glfw.x11.NET_CURRENT_DESKTOP       = GA("_NET_CURRENT_DESKTOP");
            _glfw.x11.NET_ACTIVE_WINDOW         = GA("_NET_ACTIVE_WINDOW");
        #undef GA

        if (sa) _glfw.x11.xlib.Free(sa);
        XSetWindowAttributes wa; wa.event_mask = (1L<<22);
        _glfw.x11.xlib.CreateWindow(_glfw.x11.display,_glfw.x11.root,0,0,1,1,0,0,2/*input only*/,(&((_XPrivDisplay)(_glfw.x11.display))->screens[_glfw.x11.screen])->root_visual,(1L<<11)/*event mask*/,&wa);
        XcursorImage* native = _glfw.x11.xcursor.ImageCreate(16,16); MemSetToVForNBytes(native->pixels,0,256*sizeof(XcursorUInt)); native->xhot=native->yhot=0;
        _glfw.x11.hiddenCursorHandle = _glfw.x11.xcursor.ImageLoadCursor(_glfw.x11.display,native); _glfw.x11.xcursor.ImageDestroy(native);
        PollMonitors();
    #endif
    return  1;
}

const _GLFWfbconfig* ChooseFBConfig(const _GLFWfbconfig* alts, u32 count) {
    u32 missing, leastMissing = 2147483647, colorDiff, leastColorDiff = 2147483647, extraDiff, leastExtraDiff = 2147483647;
    const _GLFWfbconfig* closest = NULL;
    for (u32 i = 0; i < count; i++) {
        const _GLFWfbconfig* cur = alts + i;        missing = 0;
        if (cur->alphaBits == 0) {missing++;} if (cur->depthBits == 0) {missing++;} if (cur->stencilBits == 0) {missing++;}
        colorDiff = 0; colorDiff+=(8-cur->redBits)  *(8-cur->redBits);     colorDiff+=(8-cur->greenBits)*(8-cur->greenBits); colorDiff+=(8-cur->blueBits)   *(8-cur->blueBits);
        extraDiff = 0; extraDiff+=(8-cur->alphaBits)*(8 - cur->alphaBits); extraDiff+=(8-cur->depthBits)*(8-cur->depthBits); extraDiff+=(8-cur->stencilBits)*(8-cur->stencilBits);
        if (missing < leastMissing || (missing == leastMissing && (colorDiff < leastColorDiff || (colorDiff == leastColorDiff && extraDiff < leastExtraDiff)))) closest = cur;
        if (cur == closest) { leastMissing = missing; leastColorDiff = colorDiff; leastExtraDiff = extraDiff; }
    }
    return closest;
}

void SetGLContext_GetFunctionPointers() {
    _GLFWwindow* handle=(_GLFWwindow*)window; handle->context.makeCurrent(handle);
    #define X(n,t) n=(t)handle->context.getProcAddress(#n);
    X(glClear,PFNGLCLEAR)                         X(glClearColor,PFNGLCLEARCOLOR)                   X(glColorMask,PFNGLCOLORMASK)              X(glDepthFunc,PFNGLDEPTHFUNC)                   X(glDepthMask,PFNGLDEPTHMASK)                       X(glDisable,PFNGLDISABLE)
    X(glEnable,PFNGLENABLE)                       X(glFinish,PFNGLFINISH)                           X(glFlush,PFNGLFLUSH)                      X(glFrontFace,PFNGLFRONTFACE)                   X(glGetError,PFNGLGETERROR)                         X(glGetIntegerv,PFNGLGETINTEGERV)
    X(glLineWidth,PFNGLLINEWIDTH)                 X(glReadBuffer,PFNGLREADBUFFER)                   X(glReadPixels,PFNGLREADPIXELS)            X(glTexImage2D,PFNGLTEXIMAGE2D)                 X(glViewport,PFNGLVIEWPORT)                         X(glBindTexture,PFNGLBINDTEXTURE)
    X(glCopyTexSubImage2D,PFNGLCOPYTEXSUBIMAGE2D) X(glDrawArrays,PFNGLDRAWARRAYS)                   X(glDrawElements,PFNGLDRAWELEMENTS)        X(glGenTextures,PFNGLGENTEXTURES)               X(glActiveTexture,PFNGLACTIVETEXTURE)               X(glBlendFuncSeparate,PFNGLBLENDFUNCSEPARATE)
    X(glBindBuffer,PFNGLBINDBUFFER)               X(glBufferData,PFNGLBUFFERDATA)                   X(glGenBuffers,PFNGLGENBUFFERS)            X(glUnmapBuffer,PFNGLUNMAPBUFFER)               X(glAttachShader,PFNGLATTACHSHADER)                 X(glCompileShader,PFNGLCOMPILESHADER)
    X(glCreateProgram,PFNGLCREATEPROGRAM)         X(glCreateShader,PFNGLCREATESHADER)               X(glDrawBuffers,PFNGLDRAWBUFFERS)          X(glGetProgramiv,PFNGLGETPROGRAMIV)             X(glGetShaderInfoLog,PFNGLGETSHADERINFOLOG)         X(glGetShaderiv,PFNGLGETSHADERIV)
    X(glLinkProgram,PFNGLLINKPROGRAM)             X(glShaderSource,PFNGLSHADERSOURCE)               X(glUniform1f,PFNGLUNIFORM1F)              X(glUniform1i,PFNGLUNIFORM1I)                   X(glUniform2f,PFNGLUNIFORM2F)                       X(glUniform3f,PFNGLUNIFORM3F)
    X(glUniform4f,PFNGLUNIFORM4F)                 X(glTexParameteri,PFNGLTEXPARAMETERI)             X(glUniform1ui,PFNGLUNIFORM1UI)            X(glUniform2ui,PFNGLUNIFORM2UI)                 X(glUniformMatrix3fv,PFNGLUNIFORMMATRIX3FV)         X(glUniformMatrix4fv,PFNGLUNIFORMMATRIX4FV)
    X(glUseProgram,PFNGLUSEPROGRAM)               X(glBindBufferBase,PFNGLBINDBUFFERBASE)           X(glBindFramebuffer,PFNGLBINDFRAMEBUFFER)  X(glGenFramebuffers,PFNGLGENFRAMEBUFFERS)       X(glMapBufferRange,PFNGLMAPBUFFERRANGE)             X(glBindImageTexture,PFNGLBINDIMAGETEXTURE)
    X(glBindVertexBuffer,PFNGLBINDVERTEXBUFFER)   X(glDispatchCompute,PFNGLDISPATCHCOMPUTE)         X(glGenVertexArrays,PFNGLGENVERTEXARRAYS)  X(glVertexAttribFormat,PFNGLVERTEXATTRIBFORMAT) X(glFramebufferTexture2D,PFNGLFRAMEBUFFERTEXTURE2D) X(glBufferSubData,PFNGLBUFFERSUBDATA)
    X(glClearBufferFv,PFNGLCLEARBUFFERFV)         X(glVertexAttribBinding,PFNGLVERTEXATTRIBBINDING)  X(glBindVertexArray,PFNGLBINDVERTEXARRAY) X(glCheckFramebufferStatus,PFNGLCHECKFRAMEBUFFERSTATUS)             X(glEnableVertexAttribArray,PFNGLENABLEVERTEXATTRIBARRAY)
    #undef X
}

size_t monitorAllocationSize = 0;
void InputMonitor(_GLFWmonitor* monitor, int action, int placement) {
    if (action == 0x00040001/*connected*/) {
        _glfw.monitorCount++;
        _glfw.monitors = _glfw.monitors ? OS_Realloc(_glfw.monitors,monitorAllocationSize,sizeof(_GLFWmonitor*) * _glfw.monitorCount) : OS_Alloc(_glfw.monitorCount * sizeof(_GLFWmonitor*));
        monitorAllocationSize = _glfw.monitorCount * sizeof(_GLFWmonitor*);
        if (placement == 0) { MoveMemoryFromBtoAForNBytes(_glfw.monitors + 1,_glfw.monitors,((size_t) _glfw.monitorCount - 1) * sizeof(_GLFWmonitor*)); _glfw.monitors[0] = monitor; }
        else _glfw.monitors[_glfw.monitorCount - 1] = monitor;
    } else if (action == 0x00040002/*disconnected*/) {
        for (int i=0;i<_glfw.monitorCount;++i) {
            if (_glfw.monitors[i] == monitor) { _glfw.monitorCount--; MoveMemoryFromBtoAForNBytes(_glfw.monitors + i, _glfw.monitors + i + 1,((size_t) _glfw.monitorCount - i) * sizeof(_GLFWmonitor*)); break; }
        }
    }
}

_GLFWmonitor* AllocMonitor(const char* n, int w, int h) { _GLFWmonitor* monitor = OS_Calloc(1, sizeof(_GLFWmonitor)); monitor->widthMM = w; monitor->heightMM = h; StringCopyInto_A_From_B(monitor->name,n,sizeof(monitor->name)); return monitor; }
_GLFWmonitor** glfwGetMonitors(int* count) { *count = _glfw.monitorCount; return (_GLFWmonitor**) _glfw.monitors; }
_GLFWmonitor* glfwGetPrimaryMonitor(void) { if (!_glfw.monitorCount) {return NULL;} return (_GLFWmonitor*) _glfw.monitors[0]; }
void glfwGetMonitorPos(_GLFWmonitor* handle, int* xpos, int* ypos) { *xpos = 0; *ypos = 0; _GLFWmonitor* monitor = (_GLFWmonitor*)handle; GetMonitorPos(monitor,xpos,ypos); }
void glfwGetMonitorWorkarea(_GLFWmonitor* handle, int* xpos, int* ypos, int* width, int* height) { *xpos=*ypos=*width=*height=0; _GLFWmonitor* monitor = (_GLFWmonitor*)handle; GetMonitorWorkarea(monitor,xpos,ypos,width,height); }
const GLFWvidmode* glfwGetVideoMode(_GLFWmonitor* handle) { _GLFWmonitor* monitor=(_GLFWmonitor*)handle; GetVideoMode(monitor,&monitor->currentMode); return &monitor->currentMode; }
void InputWindowFocus(i32 focused) {
    Sys_Input.window_has_focus = focused != 0; Sys_Input.ignore_next_mouse_delta = true;
    _GLFWwindow* win = (_GLFWwindow*)window;
    if (!focused) {
        for (int k=0;k<=348;++k) { if (win->keys[k]         == INPUT_PRESS) {       InputKey(win,k,INPUT_RELEASE);} }
        for (int b=0;b<=  7;++b) { if (win->mouseButtons[b] == INPUT_PRESS) {InputMouseClick(win,b,INPUT_RELEASE);} }
    }
}

GLFWwindow* VCreateWindow(int width, int height, char* title) {
    _GLFWwindow* win = OS_Calloc(1,sizeof(_GLFWwindow));
    win->videoMode = (GLFWvidmode){width,height,8,8,8,-1}; win->decorated = win->doublebuffer = 1; win->cursorMode = 0x00034003/*disabled*/;
#ifdef WINDOWS
    u32 style = getWindowStyle(win);
    WNDCLASSEXW wc= (WNDCLASSEXW){sizeof(wc),0x23/*Redraws + Owns Device Context*/,windowProc,0,0,_glfw.win32.instance,NULL,NULL,NULL,NULL,L"Voxen",NULL};
    _glfw.win32.mainWindowClass=RegisterClassExW(&wc);
    RECT rect={0,0,width,height}; //AdjustWindowRectEx(&rect,style,0,0);
    int frameX,frameY; frameX=frameY=0x80000000;
    int frameWidth=rect.right-rect.left, frameHeight=rect.bottom-rect.top;
    u16* wideTitle=CreateWideStringFromUTF8Win32(title);
    win->win32.handle=CreateWindowExW(0,(u16*)MAKEINTATOM(_glfw.win32.mainWindowClass),wideTitle,style,frameX,frameY,frameWidth,frameHeight,NULL,NULL,_glfw.win32.instance,(void*)NULL);
    SetPropW(win->win32.handle,L"GLFW",win);
    win->win32.keymenu=0; WINDOWPLACEMENT wp={0}; wp.length=sizeof(wp); AdjustWindowRectEx(&rect,style,0,0);
    GetWindowPlacement(win->win32.handle,&wp);
    OffsetRect(&rect,wp.rcNormalPosition.left-rect.left,wp.rcNormalPosition.top-rect.top);
    wp.rcNormalPosition=rect; wp.showCmd=0; 
    SetWindowPlacement(win->win32.handle,&wp);
    GetWindowSize(win,&win->win32.width,&win->win32.height);
    PIXELFORMATDESCRIPTOR pfd; HGLRC prc,rc; HDC pdc,dc;
    _glfw.wgl.instance = LoadLibraryA("opengl32.dll");
    _glfw.wgl.CreateContext = (PFN_CC)PlatformGetModuleSymbol(_glfw.wgl.instance,"wglCreateContext");
    _glfw.wgl.GetProcAddress = (PFN_wglGetProcAddress)PlatformGetModuleSymbol(_glfw.wgl.instance,"wglGetProcAddress");
    _glfw.wgl.GetCurrentDC = (PFN_wglGetCurrentDC)PlatformGetModuleSymbol(_glfw.wgl.instance,"wglGetCurrentDC");
    _glfw.wgl.GetCurrentContext = (PFN_wglGetCurrentContext)PlatformGetModuleSymbol(_glfw.wgl.instance,"wglGetCurrentContext");
    _glfw.wgl.MakeCurrent = (PFN_wglMakeCurrent)PlatformGetModuleSymbol(_glfw.wgl.instance,"wglMakeCurrent");
    dc = GetDC(_glfw.win32.helperWindowHandle);
    MemSetToVForNBytes(&pfd,0,sizeof(pfd)); pfd.nSize = sizeof(pfd); pfd.dwFlags=0x25; SetPixelFormat(dc,ChoosePixelFormat(dc,&pfd),&pfd);
    rc = _glfw.wgl.CreateContext(dc); pdc=wglGetCurrentDC(); prc=wglGetCurrentContext(); wglMakeCurrent(dc,rc);
    _glfw.wgl.CreateContextAttribsARB = (FP_CCAA)wglGetProcAddress("wglCreateContextAttribsARB");
    _glfw.wgl.SwapIntervalEXT = (PFN_SWE)wglGetProcAddress("wglSwapIntervalEXT");
    _glfw.wgl.GetPixelFormatAttribivARB = (PFN_GPFAIVA)wglGetProcAddress("wglGetPixelFormatAttribivARB");
    wglMakeCurrent(pdc,prc);
    int attribs[40],pixelFormat; PIXELFORMATDESCRIPTOR pfd2;
    win->context.wgl.dc = GetDC(win->win32.handle);
    pixelFormat = choosePixelFormatWGL(win);
    DescribePixelFormat(win->context.wgl.dc,pixelFormat,sizeof(pfd2),&pfd2); SetPixelFormat(win->context.wgl.dc,pixelFormat,&pfd2);
    int index=0; attribs[index++] = 0x2091/*major*/; attribs[index++] = 4;/*OpenGL 4.3*/ attribs[index++] = 0x2092/*minor*/; attribs[index++] = 3; attribs[index++] = 0x9126/*context profile mask*/; attribs[index++] = 1; attribs[index++] = 0; attribs[index++] = 0;
    win->context.wgl.handle = _glfw.wgl.CreateContextAttribsARB(win->context.wgl.dc,NULL,attribs);
    win->context.makeCurrent = makeContextCurrentWGL; win->context.swapBuffers = swapBuffersWGL;
    win->context.swapInterval = swapIntervalWGL; win->context.getProcAddress = getProcAddressWGL;
    int showCommand = 8; ShowWindow(win->win32.handle,showCommand); BringWindowToTop(win->win32.handle); SetForegroundWindow(win->win32.handle); SetFocus(win->win32.handle);
#else
    const char* names[] = {"libGLX.so.0","libGL.so.1","libGL.so",NULL};
    for (int i=0;names[i] && !_glfw.glx.handle;i++) _glfw.glx.handle = _glfwPlatformLoadModule(names[i]);
    _glfw.glx.GetFBConfigs = (PFNGLXGETFBCONFIGSPROC)PlatformGetModuleSymbol(_glfw.glx.handle,"glXGetFBConfigs");
    _glfw.glx.GetFBConfigAttrib = (PFNGLXGETFBCONFIGATTRIBPROC)PlatformGetModuleSymbol(_glfw.glx.handle,"glXGetFBConfigAttrib");
    _glfw.glx.QueryExtension = (PFNGLXQUERYEXTENSIONPROC)PlatformGetModuleSymbol(_glfw.glx.handle,"glXQueryExtension");
    _glfw.glx.QueryVersion = (PFNGLXQUERYVERSIONPROC)PlatformGetModuleSymbol(_glfw.glx.handle,"glXQueryVersion");
    _glfw.glx.MakeCurrent = (PFNGLXMAKECURRENTPROC)PlatformGetModuleSymbol(_glfw.glx.handle,"glXMakeCurrent");
    _glfw.glx.SwapBuffers = (PFNGLXSWAPBUFFERSPROC)PlatformGetModuleSymbol(_glfw.glx.handle,"glXSwapBuffers");
    _glfw.glx.QueryExtensionsString = (PFNGLXQUERYEXTENSIONSSTRINGPROC)PlatformGetModuleSymbol(_glfw.glx.handle,"glXQueryExtensionsString");
    _glfw.glx.CreateNewContext = (PFNGLXCREATENEWCONTEXTPROC)PlatformGetModuleSymbol(_glfw.glx.handle,"glXCreateNewContext");
    _glfw.glx.CreateWindow = (PFNGLXCREATEWINDOWPROC)PlatformGetModuleSymbol(_glfw.glx.handle,"glXCreateWindow");
    _glfw.glx.GetVisualFromFBConfig = (PFNGLXGETVISUALFROMFBCONFIGPROC)PlatformGetModuleSymbol(_glfw.glx.handle,"glXGetVisualFromFBConfig");
    _glfw.glx.GetProcAddress = (PFNGLXGETPROCADDRESSPROC)PlatformGetModuleSymbol(_glfw.glx.handle,"glXGetProcAddress");
    _glfw.glx.QueryExtension(_glfw.x11.display,&_glfw.glx.errorBase,&_glfw.glx.eventBase);
    _glfw.glx.QueryVersion(_glfw.x11.display,&_glfw.glx.major,&_glfw.glx.minor);
    _glfw.glx.SwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)getProcAddressGLX("glXSwapIntervalEXT");
    _glfw.glx.CreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)getProcAddressGLX("glXCreateContextAttribsARB");
    GLXFBConfig native; XVisualInfo* result;
    GLXFBConfig* nativeConfigs; _GLFWfbconfig* usableConfigs; const _GLFWfbconfig* closest; int nativeCount,usableCount;
    nativeConfigs = _glfw.glx.GetFBConfigs(_glfw.x11.display, _glfw.x11.screen, &nativeCount);        
    usableConfigs = OS_Calloc(nativeCount,sizeof(_GLFWfbconfig)); usableCount = 0;
    for (int i = 0;  i < nativeCount;  i++) {
        const GLXFBConfig n = nativeConfigs[i];
        _GLFWfbconfig* u = usableConfigs + usableCount;
        if (!(getGLXFBConfigAttrib(n,0x8011/*render type*/) & 0x00000001/*rgba bit*/)) continue;
        if (!(getGLXFBConfigAttrib(n,0x8010/*drawable type*/) & 0x00000001/*window bit*/)) continue;
        if (getGLXFBConfigAttrib(n,5) !=  1) continue;

        u->redBits = getGLXFBConfigAttrib(n,8); u->greenBits = getGLXFBConfigAttrib(n,9); u->blueBits = getGLXFBConfigAttrib(n,10); u->alphaBits = getGLXFBConfigAttrib(n,11); u->depthBits = getGLXFBConfigAttrib(n,12); u->stencilBits = getGLXFBConfigAttrib(n,13);
        u->accumRedBits = getGLXFBConfigAttrib(n,14); u->accumGreenBits = getGLXFBConfigAttrib(n,15); u->accumBlueBits = getGLXFBConfigAttrib(n,16); u->accumAlphaBits = getGLXFBConfigAttrib(n,17);
        if (getGLXFBConfigAttrib(n,6)) u->stereo =  1;
        u->handle = (uintptr_t) n;
        usableCount++;
    }

    closest = ChooseFBConfig(usableConfigs,usableCount); native = (GLXFBConfig)closest->handle;
    _glfw.x11.xlib.Free(nativeConfigs); if (usableConfigs) OS_DeallocateRAM(usableConfigs,nativeCount*sizeof(_GLFWfbconfig));
    result = _glfw.glx.GetVisualFromFBConfig(_glfw.x11.display,native);
    Visual* visual=result->visual; int depth = result->depth; _glfw.x11.xlib.Free(result);
    int xpos=0,ypos=0;
    win->x11.colormap=_glfw.x11.xlib.CreateColormap(_glfw.x11.display,_glfw.x11.root,visual,0);
    XSetWindowAttributes wa = {0}; wa.colormap = win->x11.colormap; wa.event_mask = 0x63807F;
    win->x11.parent=_glfw.x11.root;
    win->x11.handle=_glfw.x11.xlib.CreateWindow(_glfw.x11.display,_glfw.x11.root,xpos,ypos,width,height,0,depth,1/*output only*/,visual,(1L<<3)/*border pixel*/|(1L<<13)/*colormap*/|(1L<<11)/*event mask*/,&wa);
    _glfw.x11.xlib.SaveContext(_glfw.x11.display,win->x11.handle,_glfw.x11.context,(XPointer)win); // Needed to allow input.
    Atom protocols[]={_glfw.x11.WM_DELETE_WINDOW,_glfw.x11.NET_WM_PING};
    _glfw.x11.xlib.SetWMProtocols(_glfw.x11.display,win->x11.handle,protocols,sizeof(protocols)/sizeof(Atom));
    if (_glfw.x11.NET_WM_WINDOW_TYPE && _glfw.x11.NET_WM_WINDOW_TYPE_NORMAL) { Atom type=_glfw.x11.NET_WM_WINDOW_TYPE_NORMAL;
    _glfw.x11.xlib.ChangeProperty(_glfw.x11.display,win->x11.handle,_glfw.x11.NET_WM_WINDOW_TYPE,((Atom) 4),32,0/*PropModeReplace*/,(unsigned char*)&type,1); }
    XSizeHints* szhints=_glfw.x11.xlib.AllocSizeHints();
    szhints->flags|=((1L << 4)/*PMinSize*/|(1L << 5)/*PMaxSize*/); szhints->min_width=szhints->max_width=width; szhints->min_height=szhints->max_height=height;
    szhints->flags|=(1L << 9)/*PWinGravity*/; szhints->win_gravity=10/*static gravity*/;
    _glfw.x11.xlib.SetWMNormalHints(_glfw.x11.display,win->x11.handle,szhints); _glfw.x11.xlib.Free(szhints);
    _glfw.x11.xlib.ChangeProperty(_glfw.x11.display,win->x11.handle,_glfw.x11.NET_WM_NAME,_glfw.x11.UTF8_STRING,8,0/*PropModeReplace*/,(unsigned char*)title,GetStringLength(title)); // Set title
    GetWindowPos(win,&win->x11.xpos,&win->x11.ypos); GetWindowSize(win,&win->x11.width,&win->x11.height);
    int attribs[40],index=0; attribs[index++] = 0x2091/*major*/; attribs[index++] = 4; attribs[index++] = 0x2092/*minor*/; attribs[index++] = 3; /*OpenGL 4.3*/ attribs[index++] = 0x9126/*profile mask arb*/; attribs[index++] = 1/*core profile*/; attribs[index++] = 0L; attribs[index++] = 0L;
    win->context.glx.handle     = _glfw.glx.CreateContextAttribsARB(_glfw.x11.display,native,NULL,1,attribs); win->context.glx.window  = _glfw.glx.CreateWindow(_glfw.x11.display,native,win->x11.handle,NULL);
    win->context.glx.fbconfig   = native; win->context.makeCurrent = makeContextCurrentGLX;                   win->context.swapBuffers = swapBuffersGLX; win->context.swapInterval = swapIntervalGLX;
    win->context.getProcAddress = getProcAddressGLX; _glfw.x11.xlib.MapWindow(_glfw.x11.display,win->x11.handle);
    if (_glfw.x11.NET_ACTIVE_WINDOW) sendEventToWM(win,_glfw.x11.NET_ACTIVE_WINDOW,1,0,0,0,0);
    else if (WindowVisible()) { _glfw.x11.xlib.RaiseWindow(_glfw.x11.display,win->x11.handle); _glfw.x11.xlib.SetInputFocus(_glfw.x11.display,win->x11.handle,2/*RevertToParent*/,0L); }
#endif
    return (GLFWwindow*)win;
}

void VSetWindowIcon(GLFWimage* images) { SetWindowIcon(images); }
void glfwSetWindowSize(int width, int height) { _GLFWwindow* win = (_GLFWwindow*)window; win->videoMode.width=width; win->videoMode.height=height; SetWindowSize(win,width,height); }
void glfwSetWindowMonitor(int xpos, int ypos, int width, int height) { _GLFWwindow* win = (_GLFWwindow*)window; win->videoMode.width=width; win->videoMode.height=height; SetWindowMonitor(win,xpos,ypos,width,height); }
// ============ Input System
InputElement inputElements[134] = {
    { "A", KEY_A }, { "B", KEY_B }, { "C", KEY_C }, { "D", KEY_D }, { "E", KEY_E }, { "F", KEY_F }, { "G", KEY_G }, { "H", KEY_H }, { "I", KEY_I }, { "J", KEY_J }, { "K", KEY_K }, { "L", KEY_L }, { "M", KEY_M }, { "N", KEY_N }, { "O", KEY_O }, { "P", KEY_P }, { "Q", KEY_Q }, { "R", KEY_R }, { "S", KEY_S }, { "T", KEY_T },
    { "U", KEY_U }, { "V", KEY_V }, { "W", KEY_W }, { "X", KEY_X }, { "Y", KEY_Y }, { "Z", KEY_Z }, { "1", KEY_1 }, { "2", KEY_2 }, { "3", KEY_3 }, { "4", KEY_4 }, { "5", KEY_5 }, { "6", KEY_6 }, { "7", KEY_7 }, { "8", KEY_8 }, { "9", KEY_9 }, { "0", KEY_0 }, { "UP ARROW", KEY_UP }, { "DN ARROW", KEY_DOWN }, { "LF ARROW", KEY_LEFT }, { "RT ARROW", KEY_RIGHT },
    { "NUM 1", KEY_KP_1 }, { "NUM 2", KEY_KP_2 }, { "NUM 3", KEY_KP_3 }, { "NUM +", KEY_KP_ADD }, { "ENTER", KEY_ENTER }, { "RIGHT SHIFT", KEY_RIGHT_SHIFT }, { "LEFT SHIFT", KEY_LEFT_SHIFT }, { "RIGHT CTRL", KEY_RIGHT_CONTROL }, { "LEFT CTRL", KEY_LEFT_CONTROL }, { "RIGHT ALT", KEY_RIGHT_ALT },
    { "LEFT ALT", KEY_LEFT_ALT }, { "RIGHT CMD", KEY_RIGHT_SUPER }, { "LEFT CMD", KEY_LEFT_SUPER }, { "LMB", MOUSE_BUTTON_1 }, { "RMB", MOUSE_BUTTON_2 }, { "MMB", MOUSE_BUTTON_3 }, { "MB 3", MOUSE_BUTTON_4 }, { "MB 4", MOUSE_BUTTON_5 }, { "MB 5", MOUSE_BUTTON_6 }, { "MB 6", MOUSE_BUTTON_7 },
    { "MB 7", MOUSE_BUTTON_8 }, { "JOY 0", JOYSTICK_1 }, { "JOY 1", JOYSTICK_2 }, { "JOY 2", JOYSTICK_3 }, { "JOY 3", JOYSTICK_4 }, { "JOY 4", JOYSTICK_5 }, { "JOY 5", JOYSTICK_6 }, { "JOY 6", JOYSTICK_7 }, { "JOY 7", JOYSTICK_8 },
    { "JOY 8", JOYSTICK_9 }, { "JOY 9", JOYSTICK_10 }, { "JOY 10", JOYSTICK_11 }, { "JOY 11", JOYSTICK_12 }, { "JOY 12", JOYSTICK_13 }, { "JOY 13", JOYSTICK_14 }, { "JOY 14", JOYSTICK_15 }, { "JOY 15", JOYSTICK_16 }, { "JOY 16", JOYHAT_UP }, { "JOY 17", JOYHAT_RIGHT },
    { "BACKSPACE", KEY_BACKSPACE }, { "TAB", KEY_TAB }, { "NUM ENTER", KEY_KP_ENTER }, { "ESCAPE", KEY_ESCAPE }, { "SPACE", KEY_SPACE }, { "DELETE", KEY_DELETE }, { "INSERT", KEY_INSERT }, { "HOME", KEY_HOME }, { "END", KEY_END }, { "PAGE UP", KEY_PAGE_UP },
    { "PAGE DN", KEY_PAGE_DOWN }, { "F1", KEY_F1 }, { "F2", KEY_F2 }, { "F3", KEY_F3 }, { "F4", KEY_F4 }, { "F5", KEY_F5 }, { "F6", KEY_F6 }, { "F7", KEY_F7 }, { "F8", KEY_F8 }, { "F9", KEY_F9 },
    { "F10", KEY_F10 }, { "F11", KEY_F11 }, { "F12", KEY_F12 }, { "GRAVE", KEY_GRAVE_ACCENT }, { "-", KEY_MINUS }, { "=", KEY_EQUAL }, { "[", KEY_LEFT_BRACKET }, { "]", KEY_RIGHT_BRACKET }, { "\\", KEY_BACKSLASH }, { "/", KEY_SLASH },
    { ".", KEY_PERIOD }, { ",", KEY_COMMA }, { ";", KEY_SEMICOLON }, { "'", KEY_APOSTROPHE }, { "CAPSLOCK", KEY_CAPS_LOCK }, { "NUM 0", KEY_KP_0 }, { "NUM 4", KEY_KP_4 }, { "NUM 5", KEY_KP_5 }, { "NUM 6", KEY_KP_6 }, { "NUM 7", KEY_KP_7 },
    { "NUM 8", KEY_KP_8 }, { "NUM 9", KEY_KP_9 }, { "NUM *", KEY_KP_MULTIPLY }, { "NUM -", KEY_KP_SUBTRACT }, { "NUM .", KEY_KP_DECIMAL }, { "MENU", KEY_MENU }, { "PAUSE", KEY_PAUSE }, { "NUMLOCK", KEY_NUM_LOCK }, { "MWHEEL +", 128 }, { "MWHEEL -", 129 }, // 128, 129, Handled special case for mouse wheel + / - respectively
    { "PRINT", KEY_PRINT_SCREEN }, { "JOY 18", JOYHAT_DOWN }, { "JOY 19", JOYHAT_LEFT },{ "UNUSED", 0 } //, {}
};

KeyState* GetCodeMapping(int settingIndex) {
    i32 i = Sys_Settings.InputCodeSettings[settingIndex]; // Get table index into all recognized inputs
    if (i == 148 || i >= MAX_KEYS) return &Sys_Input.keyStates[MAX_KEYS - 1]; // UNUSED NULL (e.g. setting unbound)
    if (i >= 53 && i <= 61) return &Sys_Input.mouseButtons[inputElements[i].value];
    if (i >= 62 && i <= 77) return &Sys_Input.joystickButtons[JOYSTICK_1][inputElements[i].value];
    if ((i >= 78 && i <= 79) || (i >= 132 && i <= 133)) return &Sys_Input.joystickHats[inputElements[i].value];
    return &Sys_Input.keyStates[inputElements[i].value];
}

bool GetKeyRiseEdgeOrHeld(int sI, bool onRise) { i32 i = Sys_Settings.InputCodeSettings[sI]; if (i == 128) {return Sys_Input.scrollDelta > 0;} if (i == 129) {return Sys_Input.scrollDelta < 0;} KeyState* k = GetCodeMapping(sI); return onRise ? k->pressed : k->down; }
ENGINE_TO_MOD bool GetKey(int settingIndex) { return GetKeyRiseEdgeOrHeld(settingIndex,false); }  // True while held down.
ENGINE_TO_MOD bool GetKeyPressed(int settingIndex) { return (settingIndex < 0) ? Sys_Input.keyStates[KEY_GRAVE_ACCENT].pressed : GetKeyRiseEdgeOrHeld(settingIndex,true); } // True 1st frame down.
ENGINE_TO_MOD void IgnoreNextMouseDelta() { Sys_Input.ignore_next_mouse_delta = true; }
void TextEntry(i32 k) {
    if (k == KEY_U && Sys_Input.keyStates[KEY_LEFT_CONTROL].down) { Sys_Global.playerName[0] = '\0'; currentPlayerNameLength = 0; return; }
    if (k == KEY_ENTER || k == KEY_KP_ENTER) { currentMenuItem++; return; }
    if (k == KEY_BACKSPACE && currentPlayerNameLength > 0) { Sys_Global.playerName[--currentPlayerNameLength] = '\0'; return; }
    if (currentPlayerNameLength >= 26) return;
    char c = (k >= KEY_A && k <= KEY_Z) ? 'a' + (k - KEY_A) : ((k >= KEY_1 && k <= KEY_9) ? '1' + (k - KEY_1) : ((k == KEY_0) ? '0' : ((k == KEY_SPACE) ? ' ' : 0)));
    if (c) { Sys_Global.playerName[currentPlayerNameLength] = c; Sys_Global.playerName[++currentPlayerNameLength] = '\0'; }
}

void GoIntoGame(); void ConsoleEmulator(i32 keycode); extern bool enteringPlayerName;
void InputKey(_GLFWwindow* win,int key,int action) {
    if (key >= 0 && key <= 348) {
        i32 repeated = 0;
        if (action == INPUT_RELEASE && win->keys[key] == INPUT_RELEASE) return;
        if (action ==   INPUT_PRESS && win->keys[key] == INPUT_PRESS) repeated =  1;
        win->keys[key] = (char)action; if (repeated) action = INPUT_REPEAT;
    }

    if (!Sys_Input.window_has_focus) return;
    
    if (key == KEY_F10 && action) OS_Exit(0); // Suppress warnings about unused parameters forced upon me by glfw3 dependency deadweight anchor.
    if (Sys_Global.menuActive && !returnToPause) {
        if ((key == KEY_RIGHT_ALT || key == KEY_LEFT_ALT) && action && Sys_Input.keyStates[KEY_ENTER].down)                    GoIntoGame();
        if (key == KEY_ENTER && action && (Sys_Input.keyStates[KEY_LEFT_ALT].down || Sys_Input.keyStates[KEY_RIGHT_ALT].down)) GoIntoGame();
    }

    if (key >=0 && key < MAX_KEYS && (action == INPUT_PRESS || (action == INPUT_REPEAT && !(key == KEY_KP_ENTER || key == KEY_ENTER || key == KEY_TAB || key == KEY_ESCAPE)))) {
        Sys_Input.keyStates[key].pressed = Sys_Input.keyStates[key].down = true;
        if (Sys_Cheats.consoleActive) ConsoleEmulator(key);
        else if (enteringPlayerName && Sys_Global.menuActive) TextEntry(key);
    } else if (key >= 0 && key < MAX_KEYS && action == INPUT_RELEASE) Sys_Input.keyStates[key].pressed = Sys_Input.keyStates[key].down = false;
}

void InputMouseClick(_GLFWwindow* win, int button, int action) { if (button<0 || button>7) {return;} if (button<=7) {win->mouseButtons[button] = (char)action;} Sys_Input.mouseButtons[button].down = Sys_Input.mouseButtons[button].pressed = (action == 1); Sys_Input.mouseButtons[button].released = (action == 0); }
void quat_from_yaw_pitch_roll(Quaternion* q, float yaw_deg, float pitch_deg, float roll_deg) {
    float yaw = deg2rad(yaw_deg), pitch = deg2rad(pitch_deg), roll = deg2rad(roll_deg);  // Around Z (forward)
    float cy = vcosf(yaw * 0.5f), sy = vsinf(yaw * 0.5f), cp = vcosf(pitch * 0.5f), sp = vsinf(pitch * 0.5f), cr = vcosf(roll * 0.5f), sr = vsinf(roll * 0.5f);
    q->w = cy*cp*cr + sy*sp*sr; q->x = cy*sp*cr + sy*cp*sr; /* X-axis (pitch) */ q->y = sy*cp*cr - cy*sp*sr; /* Y-axis (yaw) */ q->z = cy*cp*sr - sy*sp*cr; /* Z-axis (roll) */ // Skipping quat normalization, not needed
} 

float cam_pitch,cam_yaw=90.0f,cam_roll;
void InputCursorPos(_GLFWwindow* win, double xpos, double ypos) { // static const float HeadBobRate   = 0.2f, HeadBobAmount = 0.08f,bobTarget = 0.3f; TODO
    if (win->virtualCursorPosX == xpos && win->virtualCursorPosY == ypos) return;
    win->virtualCursorPosX = xpos; win->virtualCursorPosY = ypos; if (!Sys_Input.window_has_focus) return;
    
    Sys_Input.currentMouse_dx = (i32)(xpos - Sys_Input.last_mouse_x); Sys_Input.currentMouse_dy = (i32)(ypos - Sys_Input.last_mouse_y);
    Sys_Input.last_mouse_x = xpos; Sys_Input.last_mouse_y = ypos;
    if (Sys_Input.ignore_next_mouse_delta) { Sys_Input.ignore_next_mouse_delta = mouseMovementThisFrame = false; return; }

    if ((Sys_Global.inventoryMode && !Sys_Cheats.noHUD) || Sys_Global.menuActive || Sys_Global.gamePaused) { // Uses UI baseline resolution 1366x768
        i32 newX = clamp(Sys_Global.cursorPosition_x + Sys_Input.currentMouse_dx,0,1366); if (newX != Sys_Global.cursorPosition_x) {mouseMovementThisFrame = true;} Sys_Global.cursorPosition_x = newX;
        i32 newY = clamp(Sys_Global.cursorPosition_y + Sys_Input.currentMouse_dy,0, 768); if (newY != Sys_Global.cursorPosition_y) {mouseMovementThisFrame = true;} Sys_Global.cursorPosition_y = newY;
    }
    
    if (Sys_Global.gamePaused || Sys_Global.menuActive || Sys_Global.inventoryMode) return;
    
    float s = vclamp((float)Sys_Settings.MouseSensitivity / 100.0f, 0.01f, 1.0f) * 0.2f;
    cam_yaw += (float)Sys_Input.currentMouse_dx * s; if (cam_yaw >= 360.0f) {cam_yaw -= 360.0f;} if (cam_yaw < 0.0f)     {cam_yaw  += 360.0f;}
    cam_pitch+=(float)Sys_Input.currentMouse_dy * s; if (cam_pitch > 89.0f) {cam_pitch = 89.0f;} if (cam_pitch < -89.0f) {cam_pitch = -89.0f;} // Avoid gimbal lock at pure 90deg
    quat_from_yaw_pitch_roll(&Sys_Global.instances[PLAYER1].rotation,cam_yaw,cam_pitch,(Sys_Global.curLev == LEVEL_CYBERSPACE) ? cam_roll : 0.0f);
}

void JoystickConnection(_GLFWjoystick* js, int e) {
    js->connected = (e == 0x00040001/*connected*/) ? 1 : (e == 0x00040002/*disconnected*/) ? 0 : js->connected;    
    int jid = (int)(js - _glfw.joysticks); if (jid > JOYSTICK_LAST) return;
    
    Sys_Input.joystickPresent[jid] = (e == 0x00040001/*connected*/); if (!Sys_Input.joystickPresent[jid]) { MemSetToVForNBytes(Sys_Input.joystickButtons,0,sizeof(Sys_Input.joystickButtons)); MemSetToVForNBytes(Sys_Input.joystickHats,0,sizeof(Sys_Input.joystickHats)); } // Clear
}

void InputJoystickAxis(_GLFWjoystick* js,int axis,float value) { js->axes[axis] = value; }
void InputJoystickButton(_GLFWjoystick* js,int button,char value) { js->buttons[button] = value; }
void InputJoystickHat(_GLFWjoystick* js,int hat,char value) {
    int base = js->buttonCount + hat * 4;
    js->buttons[base+0] = (value & 0x01) ? INPUT_PRESS : INPUT_RELEASE; js->buttons[base+1] = (value & 0x02) ? INPUT_PRESS : INPUT_RELEASE; 
    js->buttons[base+2] = (value & 0x04) ? INPUT_PRESS : INPUT_RELEASE; js->buttons[base+3] = (value & 0x08) ? INPUT_PRESS : INPUT_RELEASE;
    js->hats[hat] = value;
}

_GLFWjoystick* _glfwAllocJoystick(const char* name,const char* guid,int axisCount,int buttonCount,int hatCount) {
    int jid; _GLFWjoystick* js;
    for (jid = 0; jid <= JOYSTICK_LAST; jid++) { if (!_glfw.joysticks[jid].allocated) break; }
    if (jid > JOYSTICK_LAST) return NULL;
    js = _glfw.joysticks + jid;
    js->allocated = 1; js->axisCount = axisCount; js->buttonCount = buttonCount; js->hatCount = hatCount;
    js->axesSize = axisCount*sizeof(float); js->axes = OS_Calloc(axisCount,sizeof(float)); js->buttonsSize = (buttonCount + (size_t)hatCount * 4);
    js->buttons = OS_Calloc(buttonCount + (size_t)hatCount * 4,1); js->hatsSize = hatCount; js->hats = OS_Calloc(hatCount,1);
    StringCopyInto_A_From_B(js->name,name,sizeof(js->name)); StringCopyInto_A_From_B(js->guid,guid,sizeof(js->guid));
    return js;
}

bool JoystickPresent(int jid) {
    if (jid < 0 || jid > JOYSTICK_LAST || (!_glfw.joysticksInitialized && !InitJoysticks())) return false;
    
    _glfw.joysticksInitialized = 1; _GLFWjoystick* js = _glfw.joysticks + jid; return js->connected ? PollJoystick(js) : false;
}

void _glfwFreeJoystick(_GLFWjoystick* js) { OS_DeallocateRAM(js->axes,js->axesSize); OS_DeallocateRAM(js->buttons,js->buttonsSize); OS_DeallocateRAM(js->hats,js->hatsSize); MemSetToVForNBytes(js,0,sizeof(_GLFWjoystick)); }
void InputProcessing() {
    mouseMovementThisFrame = false; 
    PollEvents();
    for (int jid = JOYSTICK_1; jid <= JOYSTICK_LAST; ++jid) { // Input Poll
        if (!JoystickPresent(jid)) continue;
        _GLFWjoystick* js = _glfw.joysticks + jid; if (!js->connected) continue;

        PollJoystick(js);
        int totalButtons = js->buttonCount + js->hatCount * 4;
        for (int i = 0; i < totalButtons && i < 16; ++i) {
            KeyState* k = &Sys_Input.joystickButtons[jid - JOYSTICK_1][i];
            bool down = js->buttons[i] == INPUT_PRESS;
            k->pressed = down && !k->down; k->released = !down && k->down; k->down = down;
        }

        for (int i = 0; i < js->hatCount && i < 5; ++i) { Sys_Input.joystickHats[i].down = js->hats[i]; }
//         for (int i = 0; i < js->axisCount && i < MAX_JOYSTICK_AXES; ++i) { Sys_Input.joystickAxes[jid - JOYSTICK_1][i] = js->axes[i]; } TODO??
    }

    if (Sys_Input.keyStates[KEY_E].pressed) play_wav("./Audio/cyborgs/yourlevelsareterrible.wav",0.1f,(Vector3){},false);
    if (Sys_Input.window_has_focus) {
        if (Sys_Input.keyStates[KEY_CAPS_LOCK].pressed) Sys_Input.isCapsLockOn = !Sys_Input.isCapsLockOn; // Change capslock state to match keyboard having toggled.  Must always happen regardless of paused/menu.
        ProcessInput(); // Calls ApplyPlayerMovements(), needs called without checking paused state for menus handling.
    }
}

void SetVSync() { ((_GLFWwindow*)window)->context.swapInterval((i32)Sys_Settings.Vsync); }
void InputClearRisingAndFallingEdges() { for (i32 i=0;i<MAX_KEYS;++i) {Sys_Input.keyStates[i].pressed = Sys_Input.keyStates[i].released = false;} for (i32 i=0;i<MAX_MOUSE_BUTTONS;i++) {Sys_Input.mouseButtons[i].pressed = Sys_Input.mouseButtons[i].released = false;} Sys_Input.scrollDelta = 0; Sys_Input.currentMouse_dx = Sys_Input.currentMouse_dy = 0; } // Can't memset as we want to preserve down state
void CenterWindowOnMonitor() {
    int monitorCount; _GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
    if (Sys_Settings.CurrentMonitor > (monitorCount - 1)) { Sys_Settings.CurrentMonitor = 0; SaveConfig(); }
    int mx,my; _GLFWmonitor* next = monitors[Sys_Settings.CurrentMonitor];
    glfwGetMonitorPos(next,&mx,&my);
    const GLFWvidmode* mode = glfwGetVideoMode(next);
    int xpos = mx + (mode->width - Sys_Settings.ScreenWidth) / 2, ypos = my + (mode->height - Sys_Settings.ScreenHeight) / 2;
    glfwSetWindowPosition(window,xpos,ypos);
    Sys_Input.ignore_next_mouse_delta = true;
}

_GLFWmonitor* GetCurrentMonitor() {
    int wx=0,wy=0,ww=0,wh=0; GetWindowPos(((_GLFWwindow*)window),&wx,&wy); GetWindowSize(((_GLFWwindow*)window),&ww,&wh);
    _GLFWmonitor* bestMonitor = glfwGetPrimaryMonitor();
    int bestArea=0,monitorCount; _GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
    for (int i=0;i<monitorCount;++i) {
        int mx,my; glfwGetMonitorPos(monitors[i],&mx,&my);
        const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
        int left=vmax(wx,mx), right=vmin(wx + ww,mx + mode->width), top=vmax(wy,my), bottom=vmin(wy + wh,my + mode->height);
        int area = (right > left && bottom > top) ? (right - left) * (bottom - top) : 0;
        if (area > bestArea) { bestArea = area; bestMonitor = monitors[i]; }
    }
    return bestMonitor;
}

void ChangeResolution() {
    if (resDropdownCount < 1) return;

    resSelectedIdx = (resSelectedIdx + 1) % resDropdownCount;
    Sys_Settings.ScreenWidth  = (u32)resModes[resSelectedIdx].w; Sys_Settings.ScreenHeight = (u32)resModes[resSelectedIdx].h;
    _GLFWmonitor* monitor = GetCurrentMonitor(); if (!monitor) monitor = glfwGetPrimaryMonitor();
    int mx,my; glfwGetMonitorPos(monitor,&mx,&my);
    const GLFWvidmode* desktop = glfwGetVideoMode(monitor);
    int xpos = mx + (desktop->width - (int)Sys_Settings.ScreenWidth) / 2, ypos = my + (desktop->height - (int)Sys_Settings.ScreenHeight) / 2;
    glfwSetWindowSize((int)Sys_Settings.ScreenWidth,(int)Sys_Settings.ScreenHeight);
    glfwSetWindowPosition(window,xpos,ypos);
    UpdateScreenSize((int)Sys_Settings.ScreenWidth,(int)Sys_Settings.ScreenHeight);
    resDropdownOpen = false;
    SaveConfig();
}

void GatherResolutionModes() {
    resDropdownCount = 0; _GLFWmonitor* monitor = GetCurrentMonitor(); if (!monitor) monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* desktop = glfwGetVideoMode(monitor); if (!desktop) return;

    static const struct {int w,h;} commonRes[] = {{320,200},{640,400},{640,480},{800,600},{1024,768},{1280,720},{1280,800},{1366,768},{1440,900},{1600,900},{1920,1080},{2560,1440}};
    int maxW = desktop->width, maxH = desktop->height,j;
    for (int i = 0; i < (int)(sizeof(commonRes)/sizeof(commonRes[0])) && resDropdownCount < 8; ++i) {
        if (commonRes[i].w > maxW || commonRes[i].h > maxH || commonRes[i].w < 320 || commonRes[i].h < 200) continue;

        for (j = 0; j < resDropdownCount; ++j) { if (resModes[j].w == commonRes[i].w && resModes[j].h == commonRes[i].h) {break;} }
        if (j == resDropdownCount) resModes[resDropdownCount++] = (ResMode){commonRes[i].w,commonRes[i].h};
    }

    if (resDropdownCount < 8) resModes[resDropdownCount++] = (ResMode){desktop->width,desktop->height};
    resSelectedIdx = 0;
    for (int i = 0; i < resDropdownCount; ++i) {
        if (resModes[i].w == (int)Sys_Settings.ScreenWidth && resModes[i].h == (int)Sys_Settings.ScreenHeight) { resSelectedIdx = i; break; }
    }
}

void ChangeFullScreenWindowed() {
    int x,y,w,h,mx,my,monitorCount; _GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
    _GLFWmonitor* monitor = monitors[Sys_Settings.CurrentMonitor];
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    glfwGetMonitorWorkarea(monitor,&x,&y,&w,&h);
    ((_GLFWwindow*)window)->decorated = (i32)(!Sys_Settings.Fullscreen); SetWindowDecorated(((_GLFWwindow*)window),(i32)(!Sys_Settings.Fullscreen));
    if (Sys_Settings.Fullscreen) {
        glfwSetWindowMonitor(x,y,w,h);
        Sys_Settings.ScreenWidth = w; Sys_Settings.ScreenHeight = h;
    } else {
        glfwGetMonitorPos(monitor,&mx,&my);
        Sys_Settings.ScreenWidth  = vmax(vmin((w*3)/4,1366),320); Sys_Settings.ScreenHeight = vmax(vmin((h*3)/4,768),200);
        int xpos = mx + (mode->width - Sys_Settings.ScreenWidth) / 2, ypos = my + (mode->height - Sys_Settings.ScreenHeight) / 2;
        glfwSetWindowMonitor(xpos,ypos,Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight);
    }

    UpdateScreenSize(Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight);
}

static double monitorSwitchTime;
void CycleToNextMonitor() {
    if (get_time() < monitorSwitchTime) return;

    monitorSwitchTime = get_time() + 0.5; // Prevent toggling rapidly on accident
    int monitorCount; _GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
    if (Sys_Settings.CurrentMonitor > (monitorCount - 1)) { Sys_Settings.CurrentMonitor = 0; SaveConfig(); }
    if (!monitors || monitorCount < 2) return;

    Sys_Settings.CurrentMonitor = (Sys_Settings.CurrentMonitor + 1) % monitorCount;
    SaveConfig(); CenterWindowOnMonitor();
}
// ============== Physics System
#define MAX_COLLISION_ITERATIONS 4
#define RESTITUTION 0.5f
#define FRICTION 0.2f
#define STEP_MIN_NORMAL_Y 0.7f
#define PHY_EPSILON 0.0001f
#define MAX_SUBSTEPS 10
#define MAX_SPEED 8.0f // m/s
#define MAX_STEP_SIZE (MIN_DIAMETER / MAX_SPEED) // 0.01 s
#define MIN_DIAMETER 0.1f // m
#define MAX_ANGULAR_SPEED 5.0f
ENGINE_TO_MOD void SetPosition(Entity* e, Vector3 newpos, bool teleport) { float d = V3_Dist(e->position,newpos); if ((d > 0.001f && d < MIN_DIAMETER) || teleport) {e->position = newpos;} }
u16 dynamicEntities[512], dynamicEntityCount;
typedef struct { bool hit; Vector3 point,normal; float overlapAmount; } OverlapResult;
static inline __attribute__((always_inline)) u32 PosGetCellCoordsP(i32 cx, i32 cz) { cx = clamp(cx,0,WORLDX_0BASED); cz = clamp(cz,0,WORLDX_0BASED); return (u32)cz * WORLDX + (u32)cx; }
static inline OverlapResult SphSph(Vector3 a, float ar, Vector3 b, float br) {
    Vector3 delta = V3_AsubB(a,b); float dist = V3_Mag(delta), radSum = (ar + br); Vector3 n = (dist < PHY_EPSILON) ? (Vector3){0,1,0} : V3_ScaleByF(delta,1.0f / dist);
    return (dist < radSum) ? (OverlapResult){true,V3_AplusB(b,V3_ScaleByF(n,br)),n,radSum - dist} : (OverlapResult){0,{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f},0.0f};
}

static float ClosestSegmentSegment(Vector3 a0, Vector3 a1, Vector3 b0, Vector3 b1, float *sc, float *tc) { // Closest point between two line segments A0-A1 and B0-B1.  Returns squared distance and writes sc, tc (parameters on each segment).
    Vector3 d1 = V3_AsubB(a1,a0), d2 = V3_AsubB(b1,b0), r = V3_AsubB(a0,b0);
    float a = V3_dot(d1,d1), e = V3_dot(d2,d2), f = V3_dot(d2,r);
    if (a < PHY_EPSILON && e < PHY_EPSILON) { *sc = *tc = 0.0f; return V3_dot(r,r); }
    if (a < PHY_EPSILON) { *sc = 0.0f; *tc = vclamp(f/e, 0.0f, 1.0f); }
    else {
        float c = V3_dot(d1,r);
        if (e < PHY_EPSILON) { *tc = 0.0f; *sc = vclamp(-c/a, 0.0f, 1.0f); }
        else {
            float b = V3_dot(d1,d2), denom = a*e - b*b;
            *sc = (denom > PHY_EPSILON) ? vclamp((b*f - c*e)/denom, 0.0f, 1.0f) : 0.0f;
            *tc = (b * (*sc) + f) / e;
            if (*tc < 0.0f) { *tc = 0.0f; *sc = vclamp(-c/a, 0.0f, 1.0f); }
            else if (*tc > 1.0f) { *tc = 1.0f; *sc = vclamp((b-c)/a, 0.0f, 1.0f); }
        }
    }
    
    Vector3 diff = V3_AsubB(V3_AplusB(a0, V3_ScaleByF(d1,*sc)), V3_AplusB(b0, V3_ScaleByF(d2,*tc)));
    return V3_dot(diff,diff);
}

static OverlapResult CapCap(ShapeCapsule a, ShapeCapsule b) {
    OverlapResult r = {0};
    float sc, tc; float distSq = ClosestSegmentSegment(a.base, a.tip, b.base, b.tip, &sc, &tc);
    float radSum = a.radius + b.radius; if (distSq >= radSum * radSum) return r;
    
    float dist = vsqrtf(vmax(distSq, 0.0f));
    r.overlapAmount = radSum - dist; r.hit = true;
    Vector3 ptA = V3_AplusB(a.base,V3_ScaleByF(V3_AsubB(a.tip,a.base),sc)); 
    Vector3 ptB = V3_AplusB(b.base,V3_ScaleByF(V3_AsubB(b.tip,b.base),tc));
    Vector3 delta = V3_AsubB(ptA,ptB);
    r.normal = (dist < PHY_EPSILON) ? (Vector3){0,1,0} : V3_ScaleByF(delta, 1.0f/dist);
    r.point  = V3_AplusB(ptB, V3_ScaleByF(r.normal, b.radius));
    return r;
}

void obb_axes(Quaternion q, Vector3 *ax, Vector3 *ay, Vector3 *az) { *ax=quat_rotate_vector(q,(Vector3){1,0,0}); *ay=quat_rotate_vector(q,(Vector3){0,1,0}); *az=quat_rotate_vector(q,(Vector3){0,0,1}); }
static Vector3 ClosestPointOBB(Vector3 p, ShapeBox b) {
    Vector3 ax, ay, az; obb_axes(b.rot, &ax, &ay, &az);
    Vector3 d = V3_AsubB(p, b.center);
    float lx = V3_dot(d, ax), ly = V3_dot(d, ay), lz = V3_dot(d, az);
    lx = vclamp(lx,-b.halfExtents.x,b.halfExtents.x);
    ly = vclamp(ly,-b.halfExtents.y,b.halfExtents.y);
    lz = vclamp(lz,-b.halfExtents.z,b.halfExtents.z);
    Vector3 q = b.center;
    q = V3_AplusB(q,V3_ScaleByF(ax,lx));
    q = V3_AplusB(q,V3_ScaleByF(ay,ly));
    q = V3_AplusB(q,V3_ScaleByF(az,lz));
    return q;
}

static OverlapResult SphBox(Vector3 center, float radius, ShapeBox box) {
    OverlapResult r = {0};
    Vector3 closest = ClosestPointOBB(center,box);
    Vector3 delta = V3_AsubB(center,closest);
    float distSq = V3_dot(delta,delta);
    if (distSq >= radius * radius) return r;

    r.hit = true; float dist = vsqrtf(vmax(distSq, 0.0f));
    if (dist > PHY_EPSILON) { r.normal = V3_ScaleByF(delta, 1.0f / dist); r.overlapAmount = radius - dist; }
    else { // Center is inside OBB — find minimum penetration axis
        Vector3 ax,ay,az; obb_axes(box.rot,&ax,&ay,&az);
        Vector3 local = V3_AsubB(center,box.center);
        float lx = V3_dot(local,ax), ly = V3_dot(local,ay), lz = V3_dot(local,az);
        float dx = box.halfExtents.x - vabs(lx), dy = box.halfExtents.y - vabs(ly), dz = box.halfExtents.z - vabs(lz);
        if (dx < dy && dx < dz) { r.normal = V3_ScaleByF(ax,lx > 0 ? 1.f : -1.f); r.overlapAmount = radius + dx; }
        else if (dy < dz)       { r.normal = V3_ScaleByF(ay,ly > 0 ? 1.f : -1.f); r.overlapAmount = radius + dy; }
        else                    { r.normal = V3_ScaleByF(az,lz > 0 ? 1.f : -1.f); r.overlapAmount = radius + dz; }
    }
    r.point = closest;
    return r;
}

static u32 GetCollisionMask(u32 layer) {
    if (layer == Layer_NPCTrigger || layer == Layer_NPCClip) return Layer_NPC;
    switch (layer) {
        case Layer_Default:           return Layer_Default|Layer_TransparentFX|Layer_Geometry|Layer_NPC|Layer_PlayerBullets|Layer_Player|Layer_Corpse|Layer_PhysObjects|Layer_Trigger|Layer_Door|Layer_InterDebris|Layer_Player2|Layer_NPCBullet|Layer_Clip|Layer_CorpseSearchable;
        case Layer_TransparentFX:     return Layer_Default|Layer_TransparentFX|Layer_Geometry|Layer_NPC|Layer_PlayerBullets|Layer_Player|Layer_PhysObjects|Layer_Trigger|Layer_Door|Layer_InterDebris|Layer_Player2|Layer_NPCBullet|Layer_Clip;
        case Layer_Geometry:          return Layer_Default|Layer_TransparentFX|Layer_Geometry|Layer_NPC|Layer_PlayerBullets|Layer_Player|Layer_PhysObjects|Layer_Trigger|Layer_Door|Layer_InterDebris|Layer_Player2|Layer_Clip;
        case Layer_NPC:               return Layer_Default|Layer_TransparentFX|Layer_Geometry|Layer_NPC|Layer_PlayerBullets|Layer_Player|Layer_PhysObjects|Layer_Trigger|Layer_NPCTrigger|Layer_Door|Layer_InterDebris|Layer_Player2|Layer_NPCBullet|Layer_NPCClip|Layer_Clip;
        case Layer_PlayerBullets:     return Layer_Default|Layer_TransparentFX|Layer_Geometry|Layer_NPC|Layer_PlayerBullets|Layer_Player|Layer_Corpse|Layer_PhysObjects|Layer_Door|Layer_InterDebris|Layer_Player2|Layer_NPCBullet|Layer_Clip|Layer_CorpseSearchable;
        case Layer_Player:            return Layer_Default|Layer_TransparentFX|Layer_Geometry|Layer_NPC|Layer_PhysObjects|Layer_PlayerTriggerOnly|Layer_Trigger|Layer_Door|Layer_Player2|Layer_NPCBullet|Layer_Clip;
        case Layer_Corpse:            return Layer_Default|Layer_Geometry|Layer_PlayerBullets|Layer_PhysObjects|Layer_Door|Layer_NPCBullet|Layer_Clip;
        case Layer_PhysObjects:       return Layer_Default|Layer_TransparentFX|Layer_Geometry|Layer_NPC|Layer_PlayerBullets|Layer_Player|Layer_Corpse|Layer_PhysObjects|Layer_Door|Layer_InterDebris|Layer_NPCBullet|Layer_Clip;
        case Layer_PlayerTriggerOnly: return Layer_Player|Layer_Player2;
        case Layer_Trigger:           return Layer_Default|Layer_Geometry|Layer_NPC|Layer_PlayerBullets|Layer_Player|Layer_PhysObjects|Layer_Door|Layer_InterDebris|Layer_Clip;
        case Layer_Door:              return Layer_Default|Layer_TransparentFX|Layer_Geometry|Layer_NPC|Layer_PlayerBullets|Layer_Player|Layer_Corpse|Layer_PhysObjects|Layer_Trigger|Layer_Door|Layer_InterDebris|Layer_Player2|Layer_NPCBullet|Layer_Clip;
        case Layer_InterDebris:       return Layer_Default|Layer_Geometry|Layer_NPC|Layer_PlayerBullets|Layer_PhysObjects|Layer_Trigger|Layer_Door|Layer_NPCBullet|Layer_Clip;
        case Layer_Player2:           return Layer_Default|Layer_TransparentFX|Layer_Geometry|Layer_NPC|Layer_PlayerBullets|Layer_Player|Layer_PhysObjects|Layer_PlayerTriggerOnly|Layer_Trigger|Layer_Door|Layer_InterDebris|Layer_Player2|Layer_NPCBullet|Layer_Clip;
        case Layer_NPCBullet:         return Layer_Default|Layer_TransparentFX|Layer_Geometry|Layer_PlayerBullets|Layer_Player|Layer_Corpse|Layer_PhysObjects|Layer_Door|Layer_InterDebris|Layer_Player2|Layer_Clip|Layer_CorpseSearchable;
        case Layer_Clip:              return Layer_Player|Layer_Player2|Layer_NPC;
        case Layer_CorpseSearchable:  return Layer_Default|Layer_PlayerBullets;
        default:                      return 0u;
    }
}

void Entity_GetCap(const Entity *e, ShapeCapsule *out) {
    float r = e->colliderSize.x; float hi = vmax(0.0f, (e->colliderSize.y * 0.5f) - r); Vector3 wc,axis;
    u16 edx = (u16)(e - Sys_Global.instances);
    if (edx == PLAYER1 || edx == PLAYER2 || e->layer == Layer_NPC) { wc = V3_AplusB(e->position, e->colliderCenter); axis = (Vector3){0.0f,1.0f,0.0f}; } // Force player capsules to remain strictly upright, ignoring camera pitch/roll
    else { wc = V3_AplusB(e->position, quat_rotate_vector(e->rotation, e->colliderCenter)); axis = (e->colliderSize.z < 0.5f) ? quat_rotate_vector(e->rotation,(Vector3){1,0,0}) : (e->colliderSize.z < 1.5f) ? quat_rotate_vector(e->rotation,(Vector3){0,1,0}) : quat_rotate_vector(e->rotation,(Vector3){0,0,1}); }
    out->radius = r; out->base = V3_AsubB(wc,V3_ScaleByF(axis,hi)); out->tip  = V3_AplusB(wc,V3_ScaleByF(axis,hi));
}

void Entity_GetBox(const Entity *e, ShapeBox *out) { out->center=V3_AplusB(e->position,quat_rotate_vector(e->rotation,e->colliderCenter)); out->halfExtents=(Vector3){e->colliderSize.x*0.5f * e->scale.x,e->colliderSize.y*0.5f * e->scale.y,e->colliderSize.z*0.5f * e->scale.z}; out->rot=e->rotation; }
void Entity_GetSph(const Entity *e, ShapeSphere *out) { out->center = V3_AplusB(e->position,quat_rotate_vector(e->rotation,e->colliderCenter)); out->radius = e->colliderSize.x * vmax(e->scale.x,vmax(e->scale.y,e->scale.z)); }
static inline Color ColliderColor(Entity *e) { return (!(e->entflags & EF_RIGIDBODY)) ? textColors[TEXT_GREEN_MENU_SHADOW] : ((e->colliding) ? textColors[TEXT_RED] : textColors[TEXT_GREEN]); }
static void DrawVelocityVector(Entity *e) {
    if (!(e->entflags & EF_RIGIDBODY)) return;

    Vector3 tip = V3_AplusB(e->position,V3_ScaleByF(e->velocity,0.25f)); AddDebugLine(e->position,tip,textColors[TEXT_ORANGE]);
    Vector3 perp = V3_Normalize(V3_Cross(e->velocity,(vabs(e->velocity.y/V3_Mag(e->velocity)) < 0.9f) ? (Vector3){0,1,0} : (Vector3){1,0,0}));
    AddDebugLine(V3_AplusB(tip,V3_ScaleByF(perp,0.05f)),V3_AsubB(tip,V3_ScaleByF(perp,0.05f)),textColors[TEXT_ORANGE]); // Small cross at tip so zero-length vecs are still visible when barely moving
}

void DrawBoxCollider(Entity *e) {    
    Color col = ColliderColor(e);
    ShapeBox b; Entity_GetBox(e,&b); Vector3 ax,ay,az,c[8],px,py,pz; obb_axes(b.rot,&ax,&ay,&az);
    px=V3_ScaleByF(ax,b.halfExtents.x); py=V3_ScaleByF(ay,b.halfExtents.y); pz=V3_ScaleByF(az,b.halfExtents.z);
    for (int s=0;s<8;s++) { float sx=(s&1)?1.f:-1.f,sy=(s&2)?1.f:-1.f,sz=(s&4)?1.f:-1.f; c[s]=V3_AplusB(b.center,V3_AplusB(V3_AplusB(V3_ScaleByF(px,sx),V3_ScaleByF(py,sy)),V3_ScaleByF(pz,sz))); }
    AddDebugLine(c[0],c[1],col); AddDebugLine(c[2],c[3],col); AddDebugLine(c[4],c[5],col); AddDebugLine(c[6],c[7],col);
    AddDebugLine(c[0],c[2],col); AddDebugLine(c[1],c[3],col); AddDebugLine(c[4],c[6],col); AddDebugLine(c[5],c[7],col);
    AddDebugLine(c[0],c[4],col); AddDebugLine(c[1],c[5],col); AddDebugLine(c[2],c[6],col); AddDebugLine(c[3],c[7],col);
    DrawVelocityVector(e);
}

void DrawSphereCollider(Entity *e) {    
    Color col = ColliderColor(e); ShapeSphere s; Entity_GetSph(e,&s); float step=6.28318530f/12;
    for (int seg=0;seg<12;seg++) {
        float a0=seg*step,a1=a0+step,c0=vcosf(a0),s0=vsinf(a0),c1=vcosf(a1),s1=vsinf(a1);
        AddDebugLine(V3_AplusB(s.center,(Vector3){c0*s.radius,0,s0*s.radius}),V3_AplusB(s.center,(Vector3){c1*s.radius,0,s1*s.radius}),col);
        AddDebugLine(V3_AplusB(s.center,(Vector3){c0*s.radius,s0*s.radius,0}),V3_AplusB(s.center,(Vector3){c1*s.radius,s1*s.radius,0}),col);
        AddDebugLine(V3_AplusB(s.center,(Vector3){0,c0*s.radius,s0*s.radius}),V3_AplusB(s.center,(Vector3){0,c1*s.radius,s1*s.radius}),col);
    }
    
    DrawVelocityVector(e);
}

void DrawSphereContact(Vector3 pos, float rad) {
    if (!Sys_Cheats.showPhys) return;

    Color col = (Color){0.0f,0.0f,1.0f,1.0f}; ShapeSphere s = (ShapeSphere){pos,rad}; float step=6.28318530f/12;
    for (int seg=0;seg<12;seg++) {
        float a0=seg*step,a1=a0+step,c0=vcosf(a0),s0=vsinf(a0),c1=vcosf(a1),s1=vsinf(a1);
        AddDebugLine(V3_AplusB(s.center,(Vector3){c0*s.radius,0,s0*s.radius}),V3_AplusB(s.center,(Vector3){c1*s.radius,0,s1*s.radius}),col);
        AddDebugLine(V3_AplusB(s.center,(Vector3){c0*s.radius,s0*s.radius,0}),V3_AplusB(s.center,(Vector3){c1*s.radius,s1*s.radius,0}),col);
        AddDebugLine(V3_AplusB(s.center,(Vector3){0,c0*s.radius,s0*s.radius}),V3_AplusB(s.center,(Vector3){0,c1*s.radius,s1*s.radius}),col);
    }
}

void DrawMeshCollider(Entity *e) {    
    Color col = ColliderColor(e); u16 mi= (e->collider == COLTYPE_CVX) ? e->colliderMeshIndex : e->modelIndex; if (mi >= MODEL_IDX_MAX || mi >= loadedModelsMaxIndex) return;
    u32 triCount=modelTriangleCounts[mi]; if (!triCount) return;
    
    u16 idx = (u16)(e - Sys_Global.instances);
    float M[16]; MemCpyFromBtoAForNBytes(M,&modelMatrices[idx*16],64);
    float m00=M[0],m10=M[1],m20=M[2],m01=M[4],m11=M[5],m21=M[6],m02=M[8],m12=M[9],m22=M[10],tx=M[12],ty=M[13],tz=M[14];
    for (u32 j=0;j<triCount;j++) {
        u32 bA=(u32)modelTriangles[mi][j*3+0]*VERTEX_ATTRIBUTES_SIZE,bB=(u32)modelTriangles[mi][j*3+1]*VERTEX_ATTRIBUTES_SIZE,bC=(u32)modelTriangles[mi][j*3+2]*VERTEX_ATTRIBUTES_SIZE;
        #define LV(b) (Vector3){half_to_float(*(half*)(modelVertices[mi]+(b)+0)),half_to_float(*(half*)(modelVertices[mi]+(b)+2)),half_to_float(*(half*)(modelVertices[mi]+(b)+4))}
        #define XFORM(v) (Vector3){m00*(v).x+m01*(v).y+m02*(v).z+tx,m10*(v).x+m11*(v).y+m12*(v).z+ty,m20*(v).x+m21*(v).y+m22*(v).z+tz}
        Vector3 wA=XFORM(LV(bA)),wB=XFORM(LV(bB)),wC=XFORM(LV(bC));
        #undef LV
        #undef XFORM
        AddDebugLine(wA,wB,col); AddDebugLine(wB,wC,col); AddDebugLine(wC,wA,col);
    }
    DrawVelocityVector(e);
}

void DrawCapsuleCollider(Entity *e) {    
    Color col = ColliderColor(e); ShapeCapsule cap; Entity_GetCap(e,&cap);
    Vector3 axis=V3_Normalize(V3_AsubB(cap.tip,cap.base)); Vector3 ref=(vabs(axis.y)<0.9f)?(Vector3){0,1,0}:(Vector3){1,0,0}; Vector3 perp0=V3_Normalize(V3_Cross(axis,ref)),perp1=V3_Cross(axis,perp0);
    float step=6.28318530f/12,r=cap.radius;
    for (int seg=0;seg<12;seg++) {
        float a0=seg*step,a1=a0+step,c0=vcosf(a0),s0=vsinf(a0),c1=vcosf(a1),s1=vsinf(a1);
        Vector3 r0 = V3_AplusB(V3_ScaleByF(perp0,c0*r),V3_ScaleByF(perp1,s0*r)), r1=V3_AplusB(V3_ScaleByF(perp0,c1*r),V3_ScaleByF(perp1,s1*r));
        AddDebugLine(V3_AplusB(cap.base,r0),V3_AplusB(cap.base,r1),col); AddDebugLine(V3_AplusB(cap.tip,r0),V3_AplusB(cap.tip,r1),col);
    }

    for (int seg=0;seg<6;seg++) {
        float a0=seg*step,a1=a0+step,c0=vcosf(a0),s0=vsinf(a0),c1=vcosf(a1),s1=vsinf(a1);
        AddDebugLine(V3_AplusB(cap.base,V3_AplusB(V3_ScaleByF(perp0,c0*r),V3_ScaleByF(axis,-s0*r))),V3_AplusB(cap.base,V3_AplusB(V3_ScaleByF(perp0,c1*r),V3_ScaleByF(axis,-s1*r))),col);
        AddDebugLine(V3_AplusB(cap.base,V3_AplusB(V3_ScaleByF(perp1,c0*r),V3_ScaleByF(axis,-s0*r))),V3_AplusB(cap.base,V3_AplusB(V3_ScaleByF(perp1,c1*r),V3_ScaleByF(axis,-s1*r))),col);
        AddDebugLine(V3_AplusB(cap.tip, V3_AplusB(V3_ScaleByF(perp0,c0*r),V3_ScaleByF(axis, s0*r))),V3_AplusB(cap.tip, V3_AplusB(V3_ScaleByF(perp0,c1*r),V3_ScaleByF(axis, s1*r))),col);
        AddDebugLine(V3_AplusB(cap.tip, V3_AplusB(V3_ScaleByF(perp1,c0*r),V3_ScaleByF(axis, s0*r))),V3_AplusB(cap.tip, V3_AplusB(V3_ScaleByF(perp1,c1*r),V3_ScaleByF(axis, s1*r))),col);
    }

    for (int seg=0;seg<4;seg++) { float a=seg*(6.28318530f/4.f); Vector3 off=V3_AplusB(V3_ScaleByF(perp0,vcosf(a)*r),V3_ScaleByF(perp1,vsinf(a)*r)); AddDebugLine(V3_AplusB(cap.base,off),V3_AplusB(cap.tip,off),col); }
    DrawVelocityVector(e);
}

static void DrawAngularVelocity(Entity *e) {
    if (!Sys_Cheats.showPhys) return;
    if (!(e->entflags & EF_RIGIDBODY)) return;
    if (V3_Mag(e->angularVelocity) < 0.0001f) return; // skip near-zero

    Color purple = (Color){0.5f, 0.0f, 1.0f, 1.0f};
    float scale = 0.35f;
    Vector3 dir = V3_Normalize(e->angularVelocity);
    Vector3 tip = V3_AplusB(e->position, V3_ScaleByF(e->angularVelocity, scale));
    AddDebugLine(e->position, tip, purple); // Arrow (line vector)
    Vector3 ref = (vabs(dir.y) < 0.9f) ? (Vector3){0,1,0} : (Vector3){1,0,0};
    Vector3 perp = V3_Normalize(V3_Cross(dir, ref));
    Vector3 perp2 = V3_Cross(dir, perp);
    AddDebugLine(V3_AplusB(tip, V3_ScaleByF(perp,  0.05f)),V3_AplusB(tip, V3_ScaleByF(perp, -0.05f)), purple); // Small cross at tip so zero-length vectors are still visible
    AddDebugLine(V3_AplusB(tip, V3_ScaleByF(perp2, 0.05f)),V3_AplusB(tip, V3_ScaleByF(perp2,-0.05f)), purple);
    float radius = 0.6f; // Quarter circle arc (visualizes rotation plane + sense)
    float step = 1.57079632679f / 8.0f; // quarter circle divided into 8 segments
    Vector3 axis = dir; Vector3 p1 = V3_Normalize(V3_Cross(axis,ref)); Vector3 p2 = V3_Cross(axis,p1); // Find two vectors perpendicular to angular axis
    Vector3 prev = V3_AplusB(e->position, V3_ScaleByF(p1,radius));
    for (int i = 1; i <= 8; ++i) { float a = i * step; float c = vcosf(a); float s = vsinf(a); Vector3 cur = V3_AplusB(e->position,V3_AplusB(V3_ScaleByF(p1, c * radius),V3_ScaleByF(p2, s * radius))); AddDebugLine(prev,cur,purple); prev = cur; }
}

static u16 cellLists[WORLDX*WORLDX][128],cellCounts[WORLDX*WORLDX];
float GetCollisionRadius(Entity *e) { if (e->collider == COLTYPE_BOX) { float hx = e->colliderSize.x * 0.5f * e->scale.x, hy = e->colliderSize.y * 0.5f * e->scale.y, hz = e->colliderSize.z * 0.5f * e->scale.z; return vsqrtf(hx*hx + hy*hy + hz*hz); } return e->colliderSize.x; }
Quaternion quat_from_axis_angle(Vector3 axis, float angle) { float half = angle * 0.5f; float s = vsinf(half); return (Quaternion){axis.x * s,axis.y * s,axis.z * s,vcosf(half)}; }
Quaternion quat_normalize(Quaternion q) { float len2 = q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w; if (len2 < PHY_EPSILON) {return (Quaternion){0,0,0,1};} float inv = 1.0f / vsqrtf(len2); q.x *= inv; q.y *= inv; q.z *= inv; q.w *= inv; return q; }
static void ApplyVelocity(Entity *e, float dt) {
    Vector3 acc = {0.0f,-9.81f*e->gravity,0.0f};
    u16 edx = (u16)(e - Sys_Global.instances);
    if ((edx == PLAYER1 || edx == PLAYER2) && Sys_Cheats.noclip) acc.y = 0.0f;
    acc = V3_AplusB(acc,V3_ScaleByF(e->accumulatedForce,1.0f / e->mass));
    e->velocity = V3_AplusB(e->velocity,V3_ScaleByF(acc,dt));
    float speed = V3_Mag(e->velocity);
    if (speed > MAX_SPEED) e->velocity = V3_ScaleByF(V3_ScaleByF(e->velocity, 1.0f / speed),MAX_SPEED);
    float linDrag = vexp(-2.0f * dt);
    e->velocity = V3_ScaleByF(e->velocity,linDrag);
    SetPosition(e,V3_AplusB(e->position,V3_ScaleByF(e->velocity,dt)),false); // pos += (d = v*t)
    if (e->collider != COLTYPE_CAP) {
        float angDrag = vexp(-e->angularDrag * dt);
        e->angularVelocity = V3_ScaleByF(e->angularVelocity,angDrag); // 1. Apply continuous angular drag over time
        float avel = V3_Mag(e->angularVelocity); 
        if (avel > MAX_ANGULAR_SPEED) { e->angularVelocity = V3_ScaleByF(e->angularVelocity,MAX_ANGULAR_SPEED / avel); avel = MAX_ANGULAR_SPEED; }
        if (avel > PHY_EPSILON) { Quaternion dq = quat_from_axis_angle(V3_ScaleByF(e->angularVelocity,1.f / avel),avel * dt); e->rotation = quat_normalize(quat_multiply(dq,e->rotation)); } // 2. Integrate rotation
    } else e->angularVelocity = (Vector3){0.0f,0.0f,0.0f};
}

// Accumulate inertia tensor of a tetrahedron (origin, v0, v1, v2) into acc[6].  Uses the covariance-integral formula — sign handles winding/concavities.
static void AccumTetraInertia(float acc[6], Vector3 v0, Vector3 v1, Vector3 v2) {
    float det = V3_dot(v0,V3_Cross(v1,v2)); // signed tet volume * 6
    acc[0] += det * (v0.y*v0.y + v0.y*v1.y + v1.y*v1.y + v0.y*v2.y + v1.y*v2.y + v2.y*v2.y + v0.z*v0.z + v0.z*v1.z + v1.z*v1.z + v0.z*v2.z + v1.z*v2.z + v2.z*v2.z); // Diagonal: integral of (y²+z²), (x²+z²), (x²+y²) over tet
    acc[1] += det * (v0.x*v0.x + v0.x*v1.x + v1.x*v1.x + v0.x*v2.x + v1.x*v2.x + v2.x*v2.x + v0.z*v0.z + v0.z*v1.z + v1.z*v1.z + v0.z*v2.z + v1.z*v2.z + v2.z*v2.z);
    acc[2] += det * (v0.x*v0.x + v0.x*v1.x + v1.x*v1.x + v0.x*v2.x + v1.x*v2.x + v2.x*v2.x + v0.y*v0.y + v0.y*v1.y + v1.y*v1.y + v0.y*v2.y + v1.y*v2.y + v2.y*v2.y);
    acc[3] += det * (2*v0.x*v0.y + v0.x*v1.y + v0.x*v2.y + v1.x*v0.y + 2*v1.x*v1.y + v1.x*v2.y + v2.x*v0.y + v2.x*v1.y + 2*v2.x*v2.y); // Off-diagonal: integral of -xy, -xz, -yz (products of inertia)
    acc[4] += det * (2*v0.x*v0.z + v0.x*v1.z + v0.x*v2.z + v1.x*v0.z + 2*v1.x*v1.z + v1.x*v2.z + v2.x*v0.z + v2.x*v1.z + 2*v2.x*v2.z);
    acc[5] += det * (2*v0.y*v0.z + v0.y*v1.z + v0.y*v2.z + v1.y*v0.z + 2*v1.y*v1.z + v1.y*v2.z + v2.y*v0.z + v2.y*v1.z + 2*v2.y*v2.z);
}

void ComputeConvexMeshInertiaTensor(Entity *e) { // Called for each entity in loop after LevelLoad() has loaded all entities.
    u16 mi = e->colliderMeshIndex; e->inertiaTensorValid = false;
    if (mi >= MODEL_IDX_MAX || !modelTriangleCounts[mi] || !modelVertexCounts[mi]) return;
    
    float acc[6] = {0}; float volAcc = 0.f; u32 triCount = modelTriangleCounts[mi];
    for (u32 ti = 0; ti < triCount; ++ti) {
        u32 i0=modelTriangles[mi][ti*3+0], i1=modelTriangles[mi][ti*3+1], i2=modelTriangles[mi][ti*3+2];
        Vector3 v0 = (Vector3){half_to_float(*(half*)(modelVertices[mi]+(i0)*VERTEX_ATTRIBUTES_SIZE+0)), half_to_float(*(half*)(modelVertices[mi]+(i0)*VERTEX_ATTRIBUTES_SIZE+2)), half_to_float(*(half*)(modelVertices[mi]+(i0)*VERTEX_ATTRIBUTES_SIZE+4))};
        Vector3 v1 = (Vector3){half_to_float(*(half*)(modelVertices[mi]+(i1)*VERTEX_ATTRIBUTES_SIZE+0)), half_to_float(*(half*)(modelVertices[mi]+(i1)*VERTEX_ATTRIBUTES_SIZE+2)), half_to_float(*(half*)(modelVertices[mi]+(i1)*VERTEX_ATTRIBUTES_SIZE+4))};
        Vector3 v2 = (Vector3){half_to_float(*(half*)(modelVertices[mi]+(i2)*VERTEX_ATTRIBUTES_SIZE+0)), half_to_float(*(half*)(modelVertices[mi]+(i2)*VERTEX_ATTRIBUTES_SIZE+2)), half_to_float(*(half*)(modelVertices[mi]+(i2)*VERTEX_ATTRIBUTES_SIZE+4))};
        volAcc += V3_dot(v0,V3_Cross(v1,v2));
        AccumTetraInertia(acc,v0,v1,v2);
    }
    
    if (vabs(volAcc) < PHY_EPSILON) return;
    
    float s = e->mass / (volAcc * 60.f), so = e->mass / (volAcc * 120.f);
    float r = modelBounds[mi] * vmax(vmax(e->scale.x, e->scale.y), e->scale.z); // Calculate raw local inertia tensor bounds
    float sphericalI = (2.0f / 5.0f) * e->mass * r * r, mn = sphericalI * 0.1f;
    float Ixx = vmax((acc[1]+acc[2])*s, mn), Iyy = vmax((acc[0]+acc[2])*s, mn), Izz = vmax((acc[0]+acc[1])*s, mn);
    float Ixy = -acc[3]*so, Ixz = -acc[4]*so, Iyz = -acc[5]*so;
    float sx = e->scale.x, sy = e->scale.y, sz = e->scale.z; // Apply entity transforms to the tensor elements before running matrix inversion.  Solid rule: I_scaled_xy = I_raw_xy * scale.x * scale.y
    Ixx *= (sy * sy + sz * sz) * 0.5f; Iyy *= (sx * sx + sz * sz) * 0.5f; Izz *= (sx * sx + sy * sy) * 0.5f;
    Ixy *= sx * sy; Ixz *= sx * sz; Iyz *= sy * sz;
    e->inertiaTensor[0] = Ixx; e->inertiaTensor[1] = Iyy; e->inertiaTensor[2] = Izz;
    e->inertiaTensor[3] = Ixy; e->inertiaTensor[4] = Ixz; e->inertiaTensor[5] = Iyz;
    float det = Ixx * (Iyy * Izz - Iyz * Iyz) - Ixy * (Ixy * Izz - Ixz * Iyz) + Ixz * (Ixy * Iyz - Iyy * Ixz); // Direct analytic 3x3 symmetric matrix inversion (Cramer's Rule)
    if (vabs(det) < PHY_EPSILON) return;
    float invDet = 1.0f / det;
    e->invInertiaTensor[0] = (Iyy * Izz - Iyz * Iyz) * invDet; // invIxx
    e->invInertiaTensor[1] = (Ixx * Izz - Ixz * Ixz) * invDet; // invIyy
    e->invInertiaTensor[2] = (Ixx * Iyy - Ixy * Ixy) * invDet; // invIzz
    e->invInertiaTensor[3] = (Ixz * Iyz - Ixy * Izz) * invDet; // invIxy -> matches m3
    e->invInertiaTensor[4] = (Ixy * Iyz - Ixz * Iyy) * invDet; // invIxz -> matches m4
    e->invInertiaTensor[5] = (Ixy * Ixz - Ixx * Iyz) * invDet; // invIyz -> matches m5
    e->inertiaTensorValid = true;
}

static OverlapResult BoxBox(ShapeBox a, ShapeBox b) {
    OverlapResult r = {0}; Vector3 aAxes[3],bAxes[3];
    obb_axes(a.rot,&aAxes[0],&aAxes[1],&aAxes[2]);
    obb_axes(b.rot,&bAxes[0],&bAxes[1],&bAxes[2]);
    Vector3 T = V3_AsubB(b.center,a.center);
    float R[3][3],AbsR[3][3];
    for (int i=0;i<3;i++) for (int j=0;j<3;j++) { R[i][j]=V3_dot(aAxes[i],bAxes[j]); AbsR[i][j]=vabs(R[i][j])+1e-6f; }
    float minOverlap=1e9f; int bestAxis=-1; bool flipNormal=false; Vector3 bestEdgeAxis={0,1,0};
    for (int i=0;i<3;i++) { // Face axes A
        float ra=((float*)&a.halfExtents)[i], rb=b.halfExtents.x*AbsR[i][0]+b.halfExtents.y*AbsR[i][1]+b.halfExtents.z*AbsR[i][2];
        float t=vabs(V3_dot(T,aAxes[i])); if (t>ra+rb) return r;
        
        float ov=(ra+rb)-t; if (ov<minOverlap) { minOverlap=ov; bestAxis=i; flipNormal=(V3_dot(T,aAxes[i])<0.f); }
    }
   
    for (int i=0;i<3;i++) { // Face axes B
        float ra=a.halfExtents.x*AbsR[0][i]+a.halfExtents.y*AbsR[1][i]+a.halfExtents.z*AbsR[2][i], rb=((float*)&b.halfExtents)[i];
        float t=vabs(V3_dot(T,bAxes[i])); if (t>ra+rb) return r;
        
        float ov=(ra+rb)-t; if (ov<minOverlap) { minOverlap=ov; bestAxis=3+i; flipNormal=(V3_dot(T,bAxes[i])<0.f); }
    }
   
    for (int i=0;i<3;i++) for (int j=0;j<3;j++) { // Edge-edge cross products — store winning edgeAxis when it becomes the best
        int i1=(i+1)%3, i2=(i+2)%3, j1=(j+1)%3, j2=(j+2)%3;
        float t=vabs(V3_dot(T,aAxes[i2])*R[i1][j] - V3_dot(T,aAxes[i1])*R[i2][j]);
        float ra=((float*)&a.halfExtents)[i1]*AbsR[i2][j]+((float*)&a.halfExtents)[i2]*AbsR[i1][j];
        float rb=((float*)&b.halfExtents)[j1]*AbsR[i][j2]+((float*)&b.halfExtents)[j2]*AbsR[i][j1];
        if (t>ra+rb) return r;
        
        float axLenSq=1.f-(R[i][j]*R[i][j]);
        if (axLenSq>1e-4f) {
            float ov=((ra+rb)-t)/vsqrtf(axLenSq);
            if (ov<minOverlap) { Vector3 ea=V3_Cross(aAxes[i],bAxes[j]); minOverlap=ov; bestAxis=6+i*3+j; bestEdgeAxis=ea; flipNormal=(V3_dot(T,ea)<0.f); }
        }
    }
    
    if (bestAxis<0) return r;
    
    r.hit=true; r.overlapAmount=minOverlap;
    if      (bestAxis<3) r.normal=flipNormal ? aAxes[bestAxis]            : V3_ScaleByF(aAxes[bestAxis],-1.f);
    else if (bestAxis<6) r.normal=flipNormal ? bAxes[bestAxis-3]          : V3_ScaleByF(bAxes[bestAxis-3],-1.f);
    else                 r.normal=flipNormal ? V3_Normalize(bestEdgeAxis) : V3_ScaleByF(V3_Normalize(bestEdgeAxis),-1.f);
    
    Vector3 sA=a.center;
    sA=V3_AplusB(sA,V3_ScaleByF(aAxes[0],(V3_dot(aAxes[0],r.normal)<0.f?1.f:-1.f)*a.halfExtents.x));
    sA=V3_AplusB(sA,V3_ScaleByF(aAxes[1],(V3_dot(aAxes[1],r.normal)<0.f?1.f:-1.f)*a.halfExtents.y));
    sA=V3_AplusB(sA,V3_ScaleByF(aAxes[2],(V3_dot(aAxes[2],r.normal)<0.f?1.f:-1.f)*a.halfExtents.z));
    r.point=V3_AplusB(sA,V3_ScaleByF(r.normal,minOverlap*0.5f));
    return r;
}

static inline Vector3 MvVert(const float* M, Vector3 v) { return (Vector3){ M[0]*v.x + M[4]*v.y + M[8]*v.z  + M[12], M[1]*v.x + M[5]*v.y + M[9]*v.z  + M[13], M[2]*v.x + M[6]*v.y + M[10]*v.z + M[14] }; }
static Vector3 MeshSupport(u16 m, const float* M, Vector3 d) {
    u32 n = modelVertexCounts[m]; if (!n) {return (Vector3){0};} 
    const u8* vb = modelVertices[m]; Vector3 b={0}; float top=-1e9f;
    for (u32 i=0;i<n;++i) {const u8* p=vb + i*VERTEX_ATTRIBUTES_SIZE; Vector3 w=MvVert(M,(Vector3){half_to_float(*(half*)(p+0)),half_to_float(*(half*)(p+2)),half_to_float(*(half*)(p+4))}); float dot=V3_dot(w,d); b=(dot>top) ? (top=dot,w) : b;}
    return b;
}

typedef struct { Vector3 v[4]; int n; } Simplex3D;
static inline Vector3 MinkowskiSupport(u16 mA, const float* mxA, u16 mB, const float* mxB, Vector3 d) { return V3_AsubB(MeshSupport(mA,mxA,d), MeshSupport(mB,mxB,(Vector3){-d.x,-d.y,-d.z})); }
static inline Vector3 TP(Vector3 a, Vector3 b, Vector3 c) { return V3_Cross(V3_Cross(a,b),c); }
static inline Vector3 BoxSupport(ShapeBox b, Vector3 d) {
    Vector3 ax,ay,az; obb_axes(b.rot,&ax,&ay,&az); float kx = V3_dot(d,ax) >= 0.0f ? 1.0f : -1.0f, ky = V3_dot(d,ay) >= 0.0f ? 1.0f : -1.0f, kz = V3_dot(d,az) >= 0.0f ? 1.0f : -1.0f;
    return V3_AplusB(V3_AplusB(V3_AplusB(b.center,V3_ScaleByF(ax,kx * b.halfExtents.x)),V3_ScaleByF(ay,ky * b.halfExtents.y)),V3_ScaleByF(az,kz * b.halfExtents.z));
}

static inline Vector3 CapsuleSupport(ShapeCapsule cap, Vector3 d) { float db = V3_dot(cap.base,d),dt = V3_dot(cap.tip,d); Vector3 best = (dt > db) ? cap.tip : cap.base; float L = V3_dot(d,d); if (L < PHY_EPSILON) {return best;} return V3_AplusB(best,V3_ScaleByF(d,cap.radius / vsqrtf(L))); }
static bool GJKNextSimplex(Simplex3D *s, Vector3 *dir) {
    Vector3 A=s->v[s->n-1], AO={-A.x,-A.y,-A.z};
    if (s->n==2) {
        Vector3 AB=V3_AsubB(s->v[0],A);
        if (V3_dot(AB,AO)>0.f) *dir=TP(AB,AO,AB);
        else { s->n=1; s->v[0]=A; *dir=AO; }

        if (V3_dot(*dir,*dir)<PHY_EPSILON) { Vector3 px = (vabs(AB.x)>0.9f) ? (Vector3){0,1,0} : (Vector3){1,0,0}; *dir=V3_Cross(AB,px); } // Degenerate check applies regardless of which branch was taken
        return true;
    }
    if (s->n==3) {
        Vector3 B=s->v[1],C=s->v[0], AB=V3_AsubB(B,A),AC=V3_AsubB(C,A), ABC=V3_Cross(AB,AC);
        if (V3_dot(V3_Cross(ABC,AC),AO)>0.f) {
            if (V3_dot(AC,AO)>0.f) { s->v[1]=A; s->n=2; *dir=TP(AC,AO,AC); } else goto line_AB3;
        } else if (V3_dot(V3_Cross(AB,ABC),AO)>0.f) {
            line_AB3: if (V3_dot(AB,AO)>0.f) { s->v[0]=B; s->v[1]=A; s->n=2; *dir=TP(AB,AO,AB); } else { s->v[0]=A; s->n=1; *dir=AO; }
        } else {
            if (V3_dot(ABC,AO)>0.f) {*dir=ABC;} else { Vector3 t=s->v[0]; s->v[0]=s->v[1]; s->v[1]=t; *dir=(Vector3){-ABC.x,-ABC.y,-ABC.z}; }
        }
        return true;
    }

    Vector3 B=s->v[2],C=s->v[1],D=s->v[0];
    Vector3 AB=V3_AsubB(B,A),AC=V3_AsubB(C,A),AD=V3_AsubB(D,A);
    Vector3 nABC=V3_Cross(AB,AC),nACD=V3_Cross(AC,AD),nADB=V3_Cross(AD,AB);
    nABC=V3_dot(nABC,AD)>0.f?(Vector3){-nABC.x,-nABC.y,-nABC.z}:nABC;
    nACD=V3_dot(nACD,AB)>0.f?(Vector3){-nACD.x,-nACD.y,-nACD.z}:nACD;
    nADB=V3_dot(nADB,AC)>0.f?(Vector3){-nADB.x,-nADB.y,-nADB.z}:nADB;
    if (V3_dot(nABC,AO)>0.f){s->v[0]=C;s->v[1]=B;s->v[2]=A;s->n=3;*dir=nABC;return true;}
    if (V3_dot(nACD,AO)>0.f){s->v[0]=D;s->v[1]=C;s->v[2]=A;s->n=3;*dir=nACD;return true;}
    if (V3_dot(nADB,AO)>0.f){s->v[0]=B;s->v[1]=D;s->v[2]=A;s->n=3;*dir=nADB;return true;}
    return false;
}

#define EPA_MAX_FACES 16
#define EPA_MAX_VERTS 48
#define EPA_MAX_EDGES (EPA_MAX_FACES*3)
typedef struct { int a,b,c; Vector3 n; float d; } EPAFace;
typedef struct { Vector3 v, wA, wB; } EPAVert;
static inline EPAFace MakeEPAFace(const EPAVert* vb, int a, int b, int c) {
    Vector3 n = V3_Cross(V3_AsubB(vb[b].v,vb[a].v),V3_AsubB(vb[c].v,vb[a].v));
    float L = V3_Mag(n); if (L < PHY_EPSILON) return (EPAFace){a,b,c,{0},0.f};
    
    n = V3_ScaleByF(n,1.f/L); float d = V3_dot(n,vb[a].v);
    if (d < 0.f) { n=(Vector3){-n.x,-n.y,-n.z}; d=-d; int t=b;b=c;c=t; }
    return (EPAFace){a,b,c,n,d};
}

static inline Vector3 EPAContactPoint(const EPAVert* ev, int a, int b, int c, Vector3 n, float d) { // Barycentric projection of origin onto triangle (a,b,c) -> interpolate wA for contact point
    Vector3 pa=ev[a].v, pb=ev[b].v, pc=ev[c].v; Vector3 proj = V3_ScaleByF(n, d); // origin projected onto face plane
    Vector3 v0=V3_AsubB(pb,pa), v1=V3_AsubB(pc,pa), v2=V3_AsubB(proj,pa);
    float d00=V3_dot(v0,v0), d01=V3_dot(v0,v1), d11=V3_dot(v1,v1), d20=V3_dot(v2,v0), d21=V3_dot(v2,v1);
    float inv=1.f/(d00*d11-d01*d01+PHY_EPSILON);
    float v=(d11*d20-d01*d21)*inv, w=(d00*d21-d01*d20)*inv, u=1.f-v-w;
    return (Vector3){u*ev[a].wA.x+v*ev[b].wA.x+w*ev[c].wA.x, u*ev[a].wA.y+v*ev[b].wA.y+w*ev[c].wA.y, u*ev[a].wA.z+v*ev[b].wA.z+w*ev[c].wA.z};
}

static OverlapResult CvxCvx(u16 meshA, u16 meshB, const float* matA, const float* matB) {
    OverlapResult r = {0}; if (meshA >= MODEL_IDX_MAX || meshB >= MODEL_IDX_MAX) return r;
    Simplex3D s = {0}; Vector3 dir = {0.0f,1.0f,0.0f};
    s.v[s.n++] = MinkowskiSupport(meshA,matA,meshB,matB,dir);
    dir = (Vector3){-s.v[0].x,-s.v[0].y,-s.v[0].z};
    if (V3_dot(dir,dir) < PHY_EPSILON) dir=(Vector3){0.0f,1.0f,0.0f};
    for (int it=0;it<64;++it) {
        Vector3 sup = MinkowskiSupport(meshA,matA,meshB,matB,dir);
        if (V3_dot(sup,dir) < PHY_EPSILON) return r;
        s.v[s.n++] = sup;
        if (!GJKNextSimplex(&s,&dir) || (V3_dot(dir,dir) < PHY_EPSILON)) { r.hit=true; break; }
    }
    if (!r.hit) return r;

    static const Vector3 kAxes[6] = {{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
    for (int d=0;s.n<4 && d<6;++d) {
        Vector3 sup = MinkowskiSupport(meshA,matA,meshB,matB,kAxes[d]); bool dup=false;
        for (int k=0;k<s.n;k++) { Vector3 dv=V3_AsubB(sup,s.v[k]); dup|=(V3_dot(dv,dv)<PHY_EPSILON*PHY_EPSILON); }
        if (!dup) s.v[s.n++]=sup;
    }
    if (s.n < 4) { r.hit=true; return r; }

    EPAVert ev[EPA_MAX_VERTS]; EPAFace ef[EPA_MAX_FACES]; int nv=0, nf=0;
    for (int i=0;i<4;i++) { ev[nv].wA=MeshSupport(meshA,matA,s.v[i]); ev[nv].wB=MeshSupport(meshB,matB,(Vector3){-s.v[i].x,-s.v[i].y,-s.v[i].z}); ev[nv].v=s.v[i]; nv++; }
    static const int kTetFaces[4][3] = {{0,1,2},{0,3,1},{0,2,3},{1,3,2}};
    for (int f=0;f<4;f++) { EPAFace face = MakeEPAFace(ev, kTetFaces[f][0], kTetFaces[f][1], kTetFaces[f][2]); if (face.d >= 0.f && nf < EPA_MAX_FACES) ef[nf++]=face; }
    for (int it=0; it<32; ++it) {
        int bf=-1; float bd=1e9f;
        for (int f=0;f<nf;f++) if(ef[f].d<bd){bd=ef[f].d;bf=f;}
        if (bf<0) break;
        Vector3 bn=ef[bf].n; Vector3 wA=MeshSupport(meshA,matA,bn); Vector3 wB=MeshSupport(meshB,matB,(Vector3){-bn.x,-bn.y,-bn.z}); Vector3 sup=V3_AsubB(wA,wB);
        float sdot = V3_dot(bn,sup);
        if (sdot - bd < PHY_EPSILON) { r.normal=bn; r.overlapAmount=bd; r.point=EPAContactPoint(ev,ef[bf].a,ef[bf].b,ef[bf].c,bn,bd); return r; }
        if (nv >= EPA_MAX_VERTS) break;
        
        ev[nv].v=sup; ev[nv].wA=wA; ev[nv].wB=wB;
        int edges[EPA_MAX_EDGES][2],ne=0,keep[EPA_MAX_FACES],nk=0;
        for (int f=0;f<nf;f++) {
            if (V3_dot(ef[f].n, V3_AsubB(sup,ev[ef[f].a].v)) > 0.f) {
                int fv[3]={ef[f].a,ef[f].b,ef[f].c};
                for (int e=0;e<3;e++) { int ea=fv[e],eb=fv[(e+1)%3]; bool found=false;
                    for (int k=0;k<ne;k++) if(edges[k][0]==eb&&edges[k][1]==ea){edges[k][0]=edges[--ne][0];edges[k][1]=edges[ne][1];found=true;break;}
                    
                    if (!found&&ne<EPA_MAX_EDGES){edges[ne][0]=ea;edges[ne++][1]=eb;} }
            } else keep[nk++]=f;
        }
        nf=0; for(int k=0;k<nk;k++) ef[nf++]=ef[keep[k]];
        for(int k=0;k<ne&&nf<EPA_MAX_FACES;k++){EPAFace face=MakeEPAFace(ev,edges[k][0],edges[k][1],nv);if(face.d>=0.f)ef[nf++]=face;}
        nv++;
    }
    r.hit=true; return r;
}

static OverlapResult CapCvx(ShapeCapsule cap, u16 mesh, const float* mx) {
    OverlapResult r = {0}; if (mesh >= MODEL_IDX_MAX || !modelVertexCounts[mesh]) return r;
    
    #define CSUP_A(d) CapsuleSupport(cap, d)
    #define CSUP_B(d) MeshSupport(mesh, mx, (Vector3){-(d).x,-(d).y,-(d).z})
    #define CSUP(d)   V3_AsubB(CSUP_A(d), CSUP_B(d))
    Simplex3D s = {0}; Vector3 dir = {0,1,0};
    s.v[s.n++] = CSUP(dir); dir = (Vector3){-s.v[0].x,-s.v[0].y,-s.v[0].z};
    if (V3_dot(dir,dir) < PHY_EPSILON) dir=(Vector3){0,1,0};
    for (int it=0; it<64; ++it) {
        Vector3 sup=CSUP(dir); if (V3_dot(sup,dir)<PHY_EPSILON) return r;
        
        s.v[s.n++]=sup;
        if (!GJKNextSimplex(&s,&dir)) { r.hit=true; break; }
        if (V3_dot(dir,dir)<PHY_EPSILON) { r.hit=true; break; }
    }
    if (!r.hit) return r;
    static const Vector3 kAx[6]={{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
    for (int d=0;s.n<4&&d<6;++d) {
        Vector3 sup=CSUP(kAx[d]); bool dup=false;
        for (int k=0;k<s.n;k++){Vector3 dv=V3_AsubB(sup,s.v[k]);dup|=(V3_dot(dv,dv)<PHY_EPSILON*PHY_EPSILON);}
        if (!dup) s.v[s.n++]=sup;
    }
    if (s.n<4){r.hit=true;return r;}
    EPAVert ev[EPA_MAX_VERTS]; EPAFace ef[EPA_MAX_FACES]; int nv=0,nf=0;
    for (int i=0;i<4;i++){ev[nv].wA=CSUP_A(s.v[i]);ev[nv].wB=CSUP_B(s.v[i]);ev[nv].v=s.v[i];nv++;}
    static const int kTF[4][3]={{0,1,2},{0,3,1},{0,2,3},{1,3,2}};
    for (int f=0;f<4;f++){EPAFace face=MakeEPAFace(ev,kTF[f][0],kTF[f][1],kTF[f][2]);if(face.d>=0.f&&nf<EPA_MAX_FACES)ef[nf++]=face;}
    for (int it=0;it<32;++it){
        int bf=-1; float bd=1e9f;
        for (int f=0;f<nf;f++) if(ef[f].d<bd){bd=ef[f].d;bf=f;}
        if (bf<0) break;
        Vector3 bn=ef[bf].n; Vector3 wA=CSUP_A(bn); Vector3 wB=CSUP_B(bn); Vector3 sup=V3_AsubB(wA,wB);
        if (V3_dot(bn,sup)-bd<PHY_EPSILON){r.normal=bn;r.overlapAmount=bd;r.point=EPAContactPoint(ev,ef[bf].a,ef[bf].b,ef[bf].c,bn,bd);return r;}
        if (nv>=EPA_MAX_VERTS) break;
        
        ev[nv].v=sup; ev[nv].wA=wA; ev[nv].wB=wB;
        int edges[EPA_MAX_EDGES][2],ne=0,keep[EPA_MAX_FACES],nk=0;
        for (int f=0;f<nf;f++){
            if (V3_dot(ef[f].n,V3_AsubB(sup,ev[ef[f].a].v))>0.f){
                int fv[3]={ef[f].a,ef[f].b,ef[f].c};
                for (int e=0;e<3;e++){int ea=fv[e],eb=fv[(e+1)%3];bool found=false;
                    for (int k=0;k<ne;k++)if(edges[k][0]==eb&&edges[k][1]==ea){edges[k][0]=edges[--ne][0];edges[k][1]=edges[ne][1];found=true;break;}
                    if (!found&&ne<EPA_MAX_EDGES){edges[ne][0]=ea;edges[ne++][1]=eb;}}
            } else keep[nk++]=f;
        }
        nf=0;for(int k=0;k<nk;k++)ef[nf++]=ef[keep[k]];
        for(int k=0;k<ne&&nf<EPA_MAX_FACES;k++){EPAFace face=MakeEPAFace(ev,edges[k][0],edges[k][1],nv);if(face.d>=0.f)ef[nf++]=face;}
        nv++;
    }
    #undef CSUP_A
    #undef CSUP_B
    #undef CSUP
    r.hit=true; return r;
}

static OverlapResult BoxCvx(ShapeBox box, u16 mesh, const float* mx) {
    OverlapResult r = {0}; if (mesh >= MODEL_IDX_MAX || !modelVertexCounts[mesh]) return r;
    #define BSUP(d)   V3_AsubB(BoxSupport(box,d),MeshSupport(mesh,mx,(Vector3){-(d).x,-(d).y,-(d).z}))
    Simplex3D s={0}; Vector3 dir={0,1,0};
    s.v[s.n++]=BSUP(dir); dir=(Vector3){-s.v[0].x,-s.v[0].y,-s.v[0].z};
    if (V3_dot(dir,dir)<PHY_EPSILON) dir=(Vector3){0,1,0};
    for (int it=0;it<64;++it){
        Vector3 sup=BSUP(dir); if (V3_dot(sup,dir)<PHY_EPSILON) return r;
        s.v[s.n++]=sup;
        if (!GJKNextSimplex(&s,&dir)){r.hit=true;break;}
        if (V3_dot(dir,dir)<PHY_EPSILON){r.hit=true;break;}
    }
    if (!r.hit) return r;
    static const Vector3 kAx[6]={{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
    for (int d=0;s.n<4&&d<6;++d){
        Vector3 sup=BSUP(kAx[d]); bool dup=false;
        for (int k=0;k<s.n;k++){Vector3 dv=V3_AsubB(sup,s.v[k]);dup|=(V3_dot(dv,dv)<PHY_EPSILON*PHY_EPSILON);}
        if (!dup) s.v[s.n++]=sup;
    }
    if (s.n<4){r.hit=true;return r;}
    EPAVert ev[EPA_MAX_VERTS]; EPAFace ef[EPA_MAX_FACES]; int nv=0,nf=0;
    for (int i=0;i<4;i++){ev[nv].wA=BoxSupport(box,s.v[i]);ev[nv].wB=MeshSupport(mesh,mx,(Vector3){-s.v[i].x,-s.v[i].y,-s.v[i].z});ev[nv].v=s.v[i];nv++;}
    static const int kTF[4][3]={{0,1,2},{0,3,1},{0,2,3},{1,3,2}};
    for (int f=0;f<4;f++){EPAFace face=MakeEPAFace(ev,kTF[f][0],kTF[f][1],kTF[f][2]);if(face.d>=0.f&&nf<EPA_MAX_FACES)ef[nf++]=face;}
    for (int it=0;it<32;++it){
        int bf=-1; float bd=1e9f;
        for (int f=0;f<nf;f++) if(ef[f].d<bd){bd=ef[f].d;bf=f;}
        if (bf<0) break;
        Vector3 bn=ef[bf].n; Vector3 wA=BoxSupport(box,bn); Vector3 wB=MeshSupport(mesh,mx,(Vector3){-bn.x,-bn.y,-bn.z}); Vector3 sup=V3_AsubB(wA,wB);
        if (V3_dot(bn,sup)-bd<PHY_EPSILON){r.normal=bn;r.overlapAmount=bd;r.point=EPAContactPoint(ev,ef[bf].a,ef[bf].b,ef[bf].c,bn,bd);return r;}
        if (nv>=EPA_MAX_VERTS) break;
        ev[nv].v=sup; ev[nv].wA=wA; ev[nv].wB=wB;
        int edges[EPA_MAX_EDGES][2],ne=0,keep[EPA_MAX_FACES],nk=0;
        for (int f=0;f<nf;f++){
            if (V3_dot(ef[f].n,V3_AsubB(sup,ev[ef[f].a].v))>0.f){
                int fv[3]={ef[f].a,ef[f].b,ef[f].c};
                for (int e=0;e<3;e++){int ea=fv[e],eb=fv[(e+1)%3];bool found=false;
                    for (int k=0;k<ne;k++)if(edges[k][0]==eb&&edges[k][1]==ea){edges[k][0]=edges[--ne][0];edges[k][1]=edges[ne][1];found=true;break;}
                    if (!found&&ne<EPA_MAX_EDGES){edges[ne][0]=ea;edges[ne++][1]=eb;}}
            } else keep[nk++]=f;
        }
        nf=0;for(int k=0;k<nk;k++)ef[nf++]=ef[keep[k]];
        for(int k=0;k<ne&&nf<EPA_MAX_FACES;k++){EPAFace face=MakeEPAFace(ev,edges[k][0],edges[k][1],nv);if(face.d>=0.f)ef[nf++]=face;}
        nv++;
    }
    #undef BSUP
    r.hit=true; return r;
}

static void inline FeatureOverlap(Vector3 sc, float sr, Vector3 pt, OverlapResult* r) {
    Vector3 delta=V3_AsubB(sc,pt); float dist2=V3_dot(delta,delta);
    if (dist2 < sr*sr) { float dist=vsqrtf(vmax(dist2,0.0f)); OverlapResult t={true,pt,(dist>PHY_EPSILON) ? V3_ScaleByF(delta,1.0f/dist) : (Vector3){0.0f,1.0f,0.0f},sr - dist}; if(t.overlapAmount>r->overlapAmount) *r=t; }
}

static OverlapResult SphMsh(Vector3 sc, float sr, u16 mesh, const float* mx) { // Triangle-soup mesh support: test sphere/capsule against all triangles of a static mesh.  Returns deepest overlapping triangle result.  Normal points from mesh toward mover. Voronoi region closest point.
    OverlapResult r = {0}; if (mesh >= MODEL_IDX_MAX) return r;
    u32 triCount = modelTriangleCounts[mesh]; if (!triCount) return r;

    for (u32 ti = 0; ti < triCount; ++ti) {
        u32 i0 = modelTriangles[mesh][ti*3+0], i1 = modelTriangles[mesh][ti*3+1], i2 = modelTriangles[mesh][ti*3+2];
        #define RV(i) MvVert(mx,(Vector3){half_to_float(*(half*)(modelVertices[mesh]+(i)*VERTEX_ATTRIBUTES_SIZE+0)), half_to_float(*(half*)(modelVertices[mesh]+(i)*VERTEX_ATTRIBUTES_SIZE+2)), half_to_float(*(half*)(modelVertices[mesh]+(i)*VERTEX_ATTRIBUTES_SIZE+4))})
        Vector3 a=RV(i0), b=RV(i1), c=RV(i2);
        #undef RV
        Vector3 ab=V3_AsubB(b,a), ac=V3_AsubB(c,a), ap=V3_AsubB(sc,a); // Closest point on triangle to sphere center
        float d1=V3_dot(ab,ap), d2=V3_dot(ac,ap);
        if (d1 <= 0.0f && d2 <= 0.0f) { FeatureOverlap(sc,sr,a,&r); continue; } // Vertex A region
        
        Vector3 bp=V3_AsubB(sc,b);
        float d3=V3_dot(ab,bp), d4=V3_dot(ac,bp);
        if (d3 >= 0.0f && d4 <= d3)   { FeatureOverlap(sc,sr,b,&r); continue; } // Vertex B region
        
        Vector3 cp=V3_AsubB(sc,c);
        float d5=V3_dot(ab,cp), d6=V3_dot(ac,cp);
        if (d6>=0.f && d5<=d6) { FeatureOverlap(sc,sr,c,&r); continue; } // Vertex C region
        
        float vc=d1*d4-d3*d2;
        if (vc<=0.f && d1>=0.f && d3<=0.f) { float v=d1/(d1-d3); Vector3 pt=V3_AplusB(a,V3_ScaleByF(ab,v)); FeatureOverlap(sc,sr,pt,&r); continue; } // Edge AB region
        
        float vb=d5*d2-d1*d6;
        if (vb<=0.f && d2>=0.f && d6<=0.f) { float w=d2/(d2-d6); Vector3 pt=V3_AplusB(a,V3_ScaleByF(ac,w)); FeatureOverlap(sc,sr,pt,&r); continue; } // Edge AC region
        
        float va=d3*d6-d5*d4;
        if (va<=0.f && (d4-d3)>=0.f && (d5-d6)>=0.f) { float w=(d4-d3)/((d4-d3)+(d5-d6)); Vector3 bc=V3_AsubB(c,b); Vector3 pt=V3_AplusB(b,V3_ScaleByF(bc,w)); FeatureOverlap(sc,sr,pt,&r); continue; } // Edge BC region
        Vector3 n = V3_Cross(ab,ac); float nLen=V3_Mag(n); if(nLen<PHY_EPSILON) continue; // Face region — project onto triangle plane

        n=V3_ScaleByF(n,1.f/nLen); float dist=V3_dot(n,ap); // signed distance from plane (positive = above)
        float absDist=vabs(dist);
        if (absDist < sr) { Vector3 fn = (dist >= 0.0f) ? n : (Vector3){-n.x,-n.y,-n.z}; OverlapResult t={true,V3_AsubB(sc,V3_ScaleByF(fn,absDist)),fn,sr-absDist}; if(t.overlapAmount>r.overlapAmount) {r=t;} } // Back-face: if sphere is below the triangle, flip normal so response pushes it out correctly
    }
    return r;
}

static OverlapResult CapMsh(ShapeCapsule cap, u16 mesh, const float* mx) { OverlapResult rb = SphMsh(cap.base,cap.radius,mesh,mx); OverlapResult rt = SphMsh(cap.tip, cap.radius,mesh,mx); return (rt.overlapAmount > rb.overlapAmount) ? rt : rb; } // TODO: Connectivity needed or is snowman ala System Shock 1 fine enough?  Might prove funny if player can get stuck with top and bottom on either side of door while leaning like in original.
static OverlapResult SphCvx(Vector3 sc, float sr, u16 mesh, const float* mx) {
    OverlapResult r = {0}; if (mesh >= MODEL_IDX_MAX || !modelVertexCounts[mesh]) return r;
    #define SPHSUP_A(d) ({ Vector3 _d=(d); float _L=V3_dot(_d,_d); V3_AplusB(sc,(_L>PHY_EPSILON)?V3_ScaleByF(_d,sr/vsqrtf(_L)):(Vector3){0,sr,0}); })
    #define SPHSUP_B(d) MeshSupport(mesh,mx,(Vector3){-(d).x,-(d).y,-(d).z})
    #define MSKSUP(d)   V3_AsubB(SPHSUP_A(d),SPHSUP_B(d))
    Simplex3D s={0}; Vector3 dir={0,1,0};
    s.v[s.n++]=MSKSUP(dir); dir=(Vector3){-s.v[0].x,-s.v[0].y,-s.v[0].z};
    if (V3_dot(dir,dir)<PHY_EPSILON) dir=(Vector3){0,1,0};
    for (int it=0;it<32;++it) {
        Vector3 sup=MSKSUP(dir); if (V3_dot(sup,dir)<PHY_EPSILON) return r;
        s.v[s.n++]=sup;
        if (!GJKNextSimplex(&s,&dir)){r.hit=true;break;}
        if (V3_dot(dir,dir)<PHY_EPSILON){r.hit=true;break;}
    }
    if (!r.hit) return r;
    static const Vector3 kAx[6]={{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
    for (int d=0;s.n<4&&d<6;++d) {
        Vector3 sup=MSKSUP(kAx[d]); bool dup=false;
        for (int k=0;k<s.n;k++){Vector3 dv=V3_AsubB(sup,s.v[k]);dup|=V3_dot(dv,dv)<PHY_EPSILON*PHY_EPSILON;}
        if (!dup) s.v[s.n++]=sup;
    }
    if (s.n<4){r.hit=true;return r;}
    EPAVert ev[EPA_MAX_VERTS]; EPAFace ef[EPA_MAX_FACES]; int nv=0,nf=0;
    for (int i=0;i<4;i++){ev[nv].v=s.v[i];ev[nv].wA=SPHSUP_A(s.v[i]);ev[nv].wB=SPHSUP_B(s.v[i]);nv++;}
    static const int kTF[4][3]={{0,1,2},{0,3,1},{0,2,3},{1,3,2}};
    for (int f=0;f<4;f++){EPAFace face=MakeEPAFace(ev,kTF[f][0],kTF[f][1],kTF[f][2]);if(face.d>=0.f&&nf<EPA_MAX_FACES)ef[nf++]=face;}
    for (int it=0;it<24;++it){
        int bf=-1; float bd=1e9f;
        for (int f=0;f<nf;f++) if(ef[f].d<bd){bd=ef[f].d;bf=f;}
        if (bf<0) break;
        Vector3 bn=ef[bf].n; Vector3 wA=SPHSUP_A(bn); Vector3 wB=SPHSUP_B(bn); Vector3 sup=V3_AsubB(wA,wB);
        if (V3_dot(bn,sup)-bd<PHY_EPSILON){r.normal=bn;r.overlapAmount=bd;r.point=wB;return r;}
        if (nv>=EPA_MAX_VERTS) break;
        ev[nv].v=sup;ev[nv].wA=wA;ev[nv].wB=wB;
        int edges[EPA_MAX_EDGES][2],ne=0,keep[EPA_MAX_FACES],nk=0;
        for (int f=0;f<nf;f++){
            if (V3_dot(ef[f].n,V3_AsubB(sup,ev[ef[f].a].v))>0.f){
                int fv[3]={ef[f].a,ef[f].b,ef[f].c};
                for (int e=0;e<3;e++){int ea=fv[e],eb=fv[(e+1)%3];bool found=false;
                    for (int k=0;k<ne;k++)if(edges[k][0]==eb&&edges[k][1]==ea){edges[k][0]=edges[--ne][0];edges[k][1]=edges[ne][1];found=true;break;}
                    if (!found&&ne<EPA_MAX_EDGES){edges[ne][0]=ea;edges[ne++][1]=eb;}}
            } else keep[nk++]=f;
        }
        nf=0;for(int k=0;k<nk;k++)ef[nf++]=ef[keep[k]];
        for(int k=0;k<ne&&nf<EPA_MAX_FACES;k++){EPAFace face=MakeEPAFace(ev,edges[k][0],edges[k][1],nv);if(face.d>=0.f)ef[nf++]=face;}
        nv++;
    }
    #undef SPHSUP_A
    #undef SPHSUP_B
    #undef MSKSUP
    r.hit=true;return r;
}

typedef struct {Vector3 mn,mx;} AABB3;
static inline Vector3 TriSupport(Vector3 ta, Vector3 tb, Vector3 tc, Vector3 d) { float d1 = V3_dot(ta,d), d2 = V3_dot(tb,d), d3 = V3_dot(tc,d); return d1 > d2 ? (d1 > d3 ? ta : tc) : (d2 > d3 ? tb : tc); }
static OverlapResult CvxMsh(u16 hullMesh, const float* hullMx, u16 triMesh, const float* triMx) {
    OverlapResult best_r = {0}; if (hullMesh >= MODEL_IDX_MAX || triMesh >= MODEL_IDX_MAX) return best_r;
    AABB3 hb = { {1e9f,1e9f,1e9f}, {-1e9f,-1e9f,-1e9f} };
    u32 n = modelVertexCounts[hullMesh]; const u8* vb = modelVertices[hullMesh];
    for (u32 i=0;i<n;++i) { 
        const u8* p = vb + i*VERTEX_ATTRIBUTES_SIZE;
        Vector3 w = MvVert(hullMx,(Vector3){half_to_float(*(half*)(p+0)),half_to_float(*(half*)(p+2)),half_to_float(*(half*)(p+4))});
        hb.mn.x=vmin(hb.mn.x,w.x); hb.mn.y=vmin(hb.mn.y,w.y); hb.mn.z=vmin(hb.mn.z,w.z); hb.mx.x=vmax(hb.mx.x,w.x); hb.mx.y=vmax(hb.mx.y,w.y); hb.mx.z=vmax(hb.mx.z,w.z);
    }
    
    u32 triCount = modelTriangleCounts[triMesh]; if (!triCount) return best_r;
    for (u32 ti = 0; ti < triCount; ++ti) {
        u32 i0 = modelTriangles[triMesh][ti * 3 + 0];
        u32 i1 = modelTriangles[triMesh][ti * 3 + 1];
        u32 i2 = modelTriangles[triMesh][ti * 3 + 2];        
        Vector3 ta = MvVert(triMx,(Vector3){ half_to_float(*(half*)(modelVertices[triMesh] + i0 * VERTEX_ATTRIBUTES_SIZE + 0)), half_to_float(*(half*)(modelVertices[triMesh] + i0 * VERTEX_ATTRIBUTES_SIZE + 2)), half_to_float(*(half*)(modelVertices[triMesh] + i0 * VERTEX_ATTRIBUTES_SIZE + 4)) });
        Vector3 tb = MvVert(triMx,(Vector3){ half_to_float(*(half*)(modelVertices[triMesh] + i1 * VERTEX_ATTRIBUTES_SIZE + 0)), half_to_float(*(half*)(modelVertices[triMesh] + i1 * VERTEX_ATTRIBUTES_SIZE + 2)), half_to_float(*(half*)(modelVertices[triMesh] + i1 * VERTEX_ATTRIBUTES_SIZE + 4)) });
        Vector3 tc = MvVert(triMx,(Vector3){ half_to_float(*(half*)(modelVertices[triMesh] + i2 * VERTEX_ATTRIBUTES_SIZE + 0)), half_to_float(*(half*)(modelVertices[triMesh] + i2 * VERTEX_ATTRIBUTES_SIZE + 2)), half_to_float(*(half*)(modelVertices[triMesh] + i2 * VERTEX_ATTRIBUTES_SIZE + 4)) });        
        if (vmin(ta.x, vmin(tb.x, tc.x)) > hb.mx.x || vmax(ta.x, vmax(tb.x, tc.x)) < hb.mn.x || vmin(ta.y, vmin(tb.y, tc.y)) > hb.mx.y || vmax(ta.y, vmax(tb.y, tc.y)) < hb.mn.y || vmin(ta.z, vmin(tb.z, tc.z)) > hb.mx.z || vmax(ta.z, vmax(tb.z, tc.z)) < hb.mn.z) continue;

        Simplex3D s = {0}; 
        Vector3 dir = {0.0f,1.0f,0.0f};
        Vector3 wA0 = MeshSupport(hullMesh,hullMx,dir); // GJK Initialization (First Point)
        Vector3 wB0 = TriSupport(ta,tb,tc,(Vector3){-dir.x,-dir.y,-dir.z});
        s.v[s.n++] = V3_AsubB(wA0,wB0);
        dir = (Vector3){-s.v[0].x, -s.v[0].y, -s.v[0].z};
        if (V3_dot(dir, dir) < PHY_EPSILON) dir = (Vector3){0.0f,1.0f,0.0f};
        bool hit = false;
        for (int it = 0; it < 32; ++it) {
            Vector3 wA = MeshSupport(hullMesh,hullMx,dir), wB = TriSupport(ta,tb,tc,(Vector3){-dir.x,-dir.y,-dir.z});
            Vector3 sup = V3_AsubB(wA,wB); if (V3_dot(sup, dir) < PHY_EPSILON) break;
            
            s.v[s.n++] = sup;
            if (!GJKNextSimplex(&s, &dir)) { hit = true; break; }
            if (V3_dot(dir, dir) < PHY_EPSILON) { hit = true; break; }
        }
        
        if (!hit) continue;
        while (s.n < 4) { // Construct a proper tetrahedron dynamically from GJK points to prevent injecting skewed world-space boundary values.
            Vector3 fallbackDir = {0.0f, 1.0f, 0.0f};
            if (s.n == 1) fallbackDir = (vabs(s.v[0].x) > 0.5f) ? (Vector3){0.0f, 1.0f, 0.0f} : (Vector3){1.0f, 0.0f, 0.0f};
            else if (s.n == 2) { Vector3 edge = V3_AsubB(s.v[1], s.v[0]); fallbackDir = V3_Cross(edge, (vabs(edge.x) > 0.5f) ? (Vector3){0.0f, 1.0f, 0.0f} : (Vector3){1.0f, 0.0f, 0.0f}); }
            else if (s.n == 3) { Vector3 e1 = V3_AsubB(s.v[1], s.v[0]); Vector3 e2 = V3_AsubB(s.v[2], s.v[0]); fallbackDir = V3_Cross(e1, e2); }
            float fLen = V3_Mag(fallbackDir);
            fallbackDir = (fLen > PHY_EPSILON) ? V3_ScaleByF(fallbackDir, 1.0f / fLen) : (Vector3){0.0f, 1.0f, 0.0f};
            Vector3 wA = MeshSupport(hullMesh,hullMx,fallbackDir);
            Vector3 wB = TriSupport(ta,tb,tc,(Vector3){-fallbackDir.x,-fallbackDir.y,-fallbackDir.z});
            Vector3 sup = V3_AsubB(wA,wB);
            bool dup = false;
            for (int k = 0; k < s.n; k++) { Vector3 dv = V3_AsubB(sup, s.v[k]); dup |= (V3_dot(dv, dv) < PHY_EPSILON * PHY_EPSILON); }
            if (!dup) s.v[s.n++] = sup;
            else {
                fallbackDir = (Vector3){-fallbackDir.x, -fallbackDir.y, -fallbackDir.z}; // Handle a degenerate coplanar shape by flipping vector direction
                wA = MeshSupport(hullMesh, hullMx, fallbackDir);
                wB = TriSupport(ta, tb, tc, (Vector3){-fallbackDir.x, -fallbackDir.y, -fallbackDir.z});
                s.v[s.n++] = V3_AsubB(wA, wB);
            }
        }

        EPAVert ev[EPA_MAX_VERTS]; EPAFace ef[EPA_MAX_FACES]; int nv = 0, nf = 0;
        ev[nv].v = s.v[0]; ev[nv].wA = MeshSupport(hullMesh,hullMx,s.v[0]); ev[nv].wB = TriSupport(ta,tb,tc,(Vector3){-s.v[0].x,-s.v[0].y,-s.v[0].z}); nv++;
        ev[nv].v = s.v[1]; ev[nv].wA = MeshSupport(hullMesh,hullMx,s.v[1]); ev[nv].wB = TriSupport(ta,tb,tc,(Vector3){-s.v[1].x,-s.v[1].y,-s.v[1].z}); nv++;
        ev[nv].v = s.v[2]; ev[nv].wA = MeshSupport(hullMesh,hullMx,s.v[2]); ev[nv].wB = TriSupport(ta,tb,tc,(Vector3){-s.v[2].x,-s.v[2].y,-s.v[2].z}); nv++;
        ev[nv].v = s.v[3]; ev[nv].wA = MeshSupport(hullMesh,hullMx,s.v[3]); ev[nv].wB = TriSupport(ta,tb,tc,(Vector3){-s.v[3].x,-s.v[3].y,-s.v[3].z}); nv++;
        static const int kTF[4][3] = {{0,1,2}, {0,3,1}, {0,2,3}, {1,3,2}};
        EPAFace face0 = MakeEPAFace(ev,kTF[0][0],kTF[0][1],kTF[0][2]); if (face0.d >= 0.0f && nf < EPA_MAX_FACES) ef[nf++] = face0;
        EPAFace face1 = MakeEPAFace(ev,kTF[1][0],kTF[1][1],kTF[1][2]); if (face1.d >= 0.0f && nf < EPA_MAX_FACES) ef[nf++] = face1;
        EPAFace face2 = MakeEPAFace(ev,kTF[2][0],kTF[2][1],kTF[2][2]); if (face2.d >= 0.0f && nf < EPA_MAX_FACES) ef[nf++] = face2;
        EPAFace face3 = MakeEPAFace(ev,kTF[3][0],kTF[3][1],kTF[3][2]); if (face3.d >= 0.0f && nf < EPA_MAX_FACES) ef[nf++] = face3;
        OverlapResult r = {0};
        for (int it = 0; it < 32; ++it) {
            int bf = -1; float bd = 1e9f;
            for (int f = 0; f < nf; f++) { if (ef[f].d < bd) { bd = ef[f].d; bf = f; } }
            if (bf < 0) break;
            
            Vector3 bn = ef[bf].n; 
            Vector3 wA = MeshSupport(hullMesh,hullMx,bn);
            Vector3 wB = TriSupport(ta,tb,tc,(Vector3){-bn.x,-bn.y,-bn.z});
            Vector3 sup = V3_AsubB(wA,wB);
            if (V3_dot(bn, sup) - bd < PHY_EPSILON) { r.normal = bn; r.overlapAmount = bd; r.hit = true; r.point = EPAContactPoint(ev, ef[bf].a, ef[bf].b, ef[bf].c,bn,bd); break; }
            if (nv >= EPA_MAX_VERTS) break;
            
            ev[nv].v = sup; ev[nv].wA = wA; ev[nv].wB = wB;
            int edges[EPA_MAX_EDGES][2], ne = 0, keep[EPA_MAX_FACES], nk = 0;
            for (int f=0;f<nf;f++) {
                if (V3_dot(ef[f].n,V3_AsubB(sup,ev[ef[f].a].v)) > 0.0f) {
                    int fv[3] = {ef[f].a, ef[f].b, ef[f].c};
                    for (int e = 0; e < 3; e++) {
                        int ea = fv[e], eb = fv[(e + 1) % 3]; bool found = false;
                        for (int k = 0; k < ne; k++) {
                            if (edges[k][0] == eb && edges[k][1] == ea) { edges[k][0] = edges[--ne][0]; edges[k][1] = edges[ne][1]; found = true; break; }
                        }
                        
                        if (!found && ne < EPA_MAX_EDGES) { edges[ne][0] = ea; edges[ne++][1] = eb; }
                    }
                } else keep[nk++] = f;
            }
            
            nf = 0; for (int k = 0; k < nk; k++) ef[nf++] = ef[keep[k]];
            for (int k = 0; k < ne && nf < EPA_MAX_FACES; k++) {
                EPAFace face = MakeEPAFace(ev, edges[k][0], edges[k][1], nv);
                if (face.d >= 0.0f) ef[nf++] = face;
            }
            nv++;
        }

        if (r.hit && (!best_r.hit || r.overlapAmount > best_r.overlapAmount + 0.002f || (vabs(r.overlapAmount - best_r.overlapAmount) <= 0.002f && V3_dot(r.normal, (Vector3){0.0f, 1.0f, 0.0f}) > V3_dot(best_r.normal, (Vector3){0.0f, 1.0f, 0.0f})))) best_r = r;
    }
    
    return best_r;
}

AABB3 BoxWorldAABB(ShapeBox b) { Vector3 x,y,z; obb_axes(b.rot,&x,&y,&z); Vector3 hx=V3_ScaleByF(x,b.halfExtents.x), hy=V3_ScaleByF(y,b.halfExtents.y), hz=V3_ScaleByF(z,b.halfExtents.z); Vector3 e ={vabs(hx.x)+vabs(hy.x)+vabs(hz.x),vabs(hx.y)+vabs(hy.y)+vabs(hz.y),vabs(hx.z)+vabs(hy.z)+vabs(hz.z)}; return (AABB3){V3_AsubB(b.center,e),V3_AplusB(b.center,e)}; } 
static OverlapResult BoxMsh(ShapeBox box, u16 triMesh, const float* triMx) {
    OverlapResult r = {0}; if (triMesh >= MODEL_IDX_MAX) return r;
    u32 triCount = modelTriangleCounts[triMesh]; if (!triCount) return r;

    AABB3 hb = BoxWorldAABB(box);
    float skin = 0.02f; hb.mn.x-=skin; hb.mn.y-=skin; hb.mn.z-=skin; hb.mx.x+=skin; hb.mx.y+=skin; hb.mx.z+=skin;
    Vector3 ax, ay, az; obb_axes(box.rot, &ax, &ay, &az);
    Vector3 hx = V3_ScaleByF(ax, box.halfExtents.x), hy = V3_ScaleByF(ay, box.halfExtents.y), hz = V3_ScaleByF(az, box.halfExtents.z);
    Vector3 verts[8] = {V3_AplusB(V3_AplusB(V3_AplusB(box.center,hx),hy),hz), V3_AplusB(V3_AsubB(V3_AplusB(box.center,hx),hy),hz), V3_AplusB(V3_AplusB(V3_AsubB(box.center,hx),hy),hz), V3_AplusB(V3_AsubB(V3_AsubB(box.center,hx),hy),hz),
                         V3_AsubB(V3_AplusB(V3_AplusB(box.center,hx),hy),hz),  V3_AsubB(V3_AsubB(V3_AplusB(box.center,hx),hy),hz),  V3_AsubB(V3_AplusB(V3_AsubB(box.center,hx),hy),hz), V3_AsubB(V3_AsubB(V3_AsubB(box.center,hx),hy),hz)  };
    for (u32 vi = 0; vi < 8; ++vi) {
        Vector3 wv = verts[vi];
        for (u32 ti = 0; ti < triCount; ++ti) {
            u32 i0 = modelTriangles[triMesh][ti*3+0], i1 = modelTriangles[triMesh][ti*3+1], i2 = modelTriangles[triMesh][ti*3+2];
            #define TRV(i) MvVert(triMx,(Vector3){half_to_float(*(half*)(modelVertices[triMesh]+(i)*VERTEX_ATTRIBUTES_SIZE+0)),half_to_float(*(half*)(modelVertices[triMesh]+(i)*VERTEX_ATTRIBUTES_SIZE+2)),half_to_float(*(half*)(modelVertices[triMesh]+(i)*VERTEX_ATTRIBUTES_SIZE+4))})
            Vector3 ta=TRV(i0),tb=TRV(i1),tc=TRV(i2);
            #undef TRV
            if (vmin(ta.x,vmin(tb.x,tc.x))>hb.mx.x || vmax(ta.x,vmax(tb.x,tc.x))<hb.mn.x || vmin(ta.y,vmin(tb.y,tc.y))>hb.mx.y || vmax(ta.y,vmax(tb.y,tc.y))<hb.mn.y || vmin(ta.z,vmin(tb.z,tc.z))>hb.mx.z || vmax(ta.z,vmax(tb.z,tc.z))<hb.mn.z) continue;

            Vector3 ab=V3_AsubB(tb,ta), ac=V3_AsubB(tc,ta), ap=V3_AsubB(wv,ta);
            float d1=V3_dot(ab,ap), d2=V3_dot(ac,ap);
            Vector3 bp=V3_AsubB(wv,tb); float d3=V3_dot(ab,bp), d4=V3_dot(ac,bp);
            Vector3 cp=V3_AsubB(wv,tc); float d5=V3_dot(ab,cp), d6=V3_dot(ac,cp);
            float vc=d1*d4-d3*d2, vb_=d5*d2-d1*d6, va=d3*d6-d5*d4;
            Vector3 closest; bool inFace=false;
            if      (d1 <= 0.0f && d2 <= 0.0f) closest=ta;
            else if (d3 >= 0.0f && d4 <= d3)   closest=tb;
            else if (d6 >= 0.0f && d5 <= d6)   closest=tc;
            else if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f)           { float v=d1/(d1-d3); closest=V3_AplusB(ta,V3_ScaleByF(ab,v)); }
            else if (vb_ <= 0.0f && d2 >= 0.0f && d6 <= 0.0f)          { float w=d2/(d2-d6); closest=V3_AplusB(ta,V3_ScaleByF(ac,w)); }
            else if (va <= 0.0f && (d4-d3) >= 0.0f && (d5-d6) >= 0.0f) { float w=(d4-d3)/((d4-d3)+(d5-d6)); closest=V3_AplusB(tb,V3_ScaleByF(V3_AsubB(tc,tb),w)); }
            else { inFace=true; Vector3 n_=V3_Cross(ab,ac); float L=V3_Mag(n_); if(L<PHY_EPSILON) continue; n_=V3_ScaleByF(n_,1.f/L); closest=V3_AsubB(wv,V3_ScaleByF(n_,V3_dot(n_,ap))); }

            Vector3 delta=V3_AsubB(wv,closest); float dist2=V3_dot(delta,delta); float pen=0.f; Vector3 fn;
            if (inFace) {
                Vector3 n_=V3_Cross(ab,ac); float L=V3_Mag(n_); if(L<PHY_EPSILON) continue;
                n_=V3_ScaleByF(n_,1.f/L); float sd=V3_dot(n_,ap); if (vabs(sd)<PHY_EPSILON) continue;
                pen=vabs(sd); fn=(sd>0.f)?n_:(Vector3){-n_.x,-n_.y,-n_.z};
            } else {
                if (dist2<PHY_EPSILON*PHY_EPSILON) continue;
                float dist=vsqrtf(dist2); Vector3 n_=V3_Cross(ab,ac); float L=V3_Mag(n_); if(L<PHY_EPSILON) continue;
                n_=V3_ScaleByF(n_,1.f/L); float sd=V3_dot(n_,ap); if (sd>=0.f) continue;
                pen=dist; fn=V3_ScaleByF(delta,1.f/dist);
            }
            if (pen>r.overlapAmount) r=(OverlapResult){true,closest,fn,pen};
        }
    }
    return r;
}

static OverlapResult CapBox(ShapeCapsule cap, ShapeBox box) {
    OverlapResult best = SphBox(cap.base,cap.radius,box), rt = SphBox(cap.tip,cap.radius,box); if (rt.hit && rt.overlapAmount > best.overlapAmount) best = rt;

    Vector3 ax,ay,az; obb_axes(box.rot,&ax,&ay,&az);
    Vector3 d = V3_AsubB(cap.tip,cap.base);
    float segLen = V3_Mag(d); if (segLen < PHY_EPSILON) return best;
    
    Vector3 dUnit = V3_ScaleByF(d, 1.f / segLen);
    Vector3 toBase = V3_AsubB(cap.base, box.center);
    float ts[6]; int nt = 0;
    float dax = V3_dot(dUnit,ax) * segLen, day = V3_dot(dUnit,ay) * segLen, daz = V3_dot(dUnit,az) * segLen;
    float bax = V3_dot(toBase,ax),         bay = V3_dot(toBase,ay),         baz = V3_dot(toBase,az);
    if (vabs(dax) > PHY_EPSILON) { float t0 = (-box.halfExtents.x - bax) / dax, t1 = (box.halfExtents.x - bax) / dax; if (t0 > t1) { float tmp = t0; t0 = t1; t1 = tmp; } t0 = vclamp(t0,0.0f,1.0f); t1 = vclamp(t1,0.0f,1.f); ts[nt++] = t0; ts[nt++] = t1; }
    if (vabs(day) > PHY_EPSILON) { float t0 = (-box.halfExtents.y - bay) / day, t1 = (box.halfExtents.y - bay) / day; if (t0 > t1) { float tmp = t0; t0 = t1; t1 = tmp; } t0 = vclamp(t0,0.0f,1.0f); t1 = vclamp(t1,0.0f,1.f); ts[nt++] = t0; ts[nt++] = t1; }
    if (vabs(daz) > PHY_EPSILON) { float t0 = (-box.halfExtents.z - baz) / daz, t1 = (box.halfExtents.z - baz) / daz; if (t0 > t1) { float tmp = t0; t0 = t1; t1 = tmp; } t0 = vclamp(t0,0.0f,1.0f); t1 = vclamp(t1,0.0f,1.f); ts[nt++] = t0; ts[nt++] = t1; }
    if (nt == 0) ts[nt++] = 0.5f; // Only add midpoint if no slab candidates were generated (fully degenerate segment)
    for (int k = 0; k < nt; k++) { Vector3 pt = V3_AplusB(cap.base,V3_ScaleByF(d,ts[k])); OverlapResult rk = SphBox(pt,cap.radius,box); if (rk.hit && rk.overlapAmount > best.overlapAmount) {best = rk;} }
    return best;
}

// Returns moment of inertia about world-space axis 'n' for entity e.  Falls back to sphere approximation only if tensor wasn't computed.
static float GetMomentOfInertia(Entity *e, Vector3 n) {
    if (e->collider != COLTYPE_CVX || !e->inertiaTensorValid) {
        float r = (e->collider == COLTYPE_MSH) ? modelBounds[e->modelIndex] : (e->collider == COLTYPE_CVX) ? modelBounds[e->colliderMeshIndex] : GetCollisionRadius(e);
        return (2.f / 5.f) * e->mass * r * r;
    }
   
    Vector3 ln = quat_rotate_vector((Quaternion){-e->rotation.x,-e->rotation.y,-e->rotation.z,e->rotation.w},n); // Use quat conjugate to rotate n into entity local space, then apply I * n, dot with n: gives I_n = n^T * I * n
    float Ixx=e->inertiaTensor[0], Iyy=e->inertiaTensor[1], Izz=e->inertiaTensor[2], Ixy=e->inertiaTensor[3], Ixz=e->inertiaTensor[4], Iyz=e->inertiaTensor[5];
    Vector3 In = {Ixx*ln.x + Ixy*ln.y + Ixz*ln.z, Ixy*ln.x + Iyy*ln.y + Iyz*ln.z, Ixz*ln.x + Iyz*ln.y + Izz*ln.z}; // I*ln (matrix-vector: products of inertia were already negated at generation)
    return vmax(V3_dot(ln,In),PHY_EPSILON);
}

static void ApplyCollisionResponse(Entity *e, Entity *o, OverlapResult* ov) {
    float overlap = ov->overlapAmount; if (overlap < PHY_EPSILON) return;
    bool oStatic = (!(o->entflags & EF_RIGIDBODY) || o->mass < 0.001f || o->collider == COLTYPE_NONE || o->collider == COLTYPE_MSH); 
    if ((o->collider == COLTYPE_MSH && e->collider == COLTYPE_MSH)) return;
    
    DrawSphereContact(ov->point,0.02f);
    Vector3 n = ov->normal, contactPoint = ov->point;
    Vector3 rAarm = V3_AsubB(contactPoint, e->position);
    Vector3 rBarm = oStatic ? (Vector3){0,0,0} : V3_AsubB(contactPoint, o->position);
    
    // Evaluate inertia around the NORMAL torque axis (r x n)
    Vector3 rAxN = V3_Cross(rAarm, n);
    float rAxN_lenSq = V3_dot(rAxN, rAxN);
    float iA = (rAxN_lenSq > PHY_EPSILON) ? GetMomentOfInertia(e, V3_ScaleByF(rAxN, 1.0f / vsqrtf(rAxN_lenSq))) : 1.0f;
    float invIA = (iA > PHY_EPSILON) ? 1.0f / iA : 0.0f;
    float angTermA = ConstIndexIsNPC(e->index) ? 0.0f : rAxN_lenSq * invIA;
    Vector3 rBxN = V3_Cross(rBarm, n);
    float rBxN_lenSq = V3_dot(rBxN, rBxN);
    float iB = (oStatic || rBxN_lenSq < PHY_EPSILON) ? 0.0f : GetMomentOfInertia(o, V3_ScaleByF(rBxN, 1.0f / vsqrtf(rBxN_lenSq)));
    float invIB = (iB > PHY_EPSILON && !oStatic) ? 1.0f / iB : 0.0f;
    float angTermB = (oStatic || ConstIndexIsNPC(o->index)) ? 0.0f : rBxN_lenSq * invIB;
    Vector3 vAtA = V3_AplusB(e->velocity, V3_Cross(e->angularVelocity, rAarm));
    Vector3 vAtB = oStatic ? (Vector3){0,0,0} : V3_AplusB(o->velocity, V3_Cross(o->angularVelocity, rBarm));
    Vector3 relVel = V3_AsubB(vAtA, vAtB);
    float vn = V3_dot(relVel, n); 
    if (vn > 0.01f) return;

    float invMassA = e->mass < 0.001f ? 1.0f : 1.0f / e->mass;
    float invMassB = oStatic || o->mass < 0.001f ? 0.0f : 1.0f / o->mass;
    float invSum = invMassA + invMassB + angTermA + angTermB;
    if (invSum < PHY_EPSILON) return;

    float e_r = vmax(e->bounciness, oStatic ? 0.0f : o->bounciness);
    float j = -(1.0f + e_r) * vn / invSum;
    
    // Apply Normal Impulse
    e->velocity = V3_AplusB(e->velocity, V3_ScaleByF(n, j * invMassA));
    if (!oStatic) o->velocity = V3_AsubB(o->velocity, V3_ScaleByF(n, j * invMassB));
    Vector3 Jn = V3_ScaleByF(n,j);
    if (e->collider != COLTYPE_CAP && !ConstIndexIsNPC(e->index)) e->angularVelocity = V3_AplusB(e->angularVelocity, V3_ScaleByF(V3_Cross(rAarm, Jn), invIA));
    if (!oStatic && o->collider != COLTYPE_CAP && !ConstIndexIsNPC(o->index)) o->angularVelocity = V3_AsubB(o->angularVelocity, V3_ScaleByF(V3_Cross(rBarm, Jn), invIB));

    // Friction calculation
    Vector3 vAtA2 = V3_AplusB(e->velocity, V3_Cross(e->angularVelocity, rAarm));
    Vector3 vAtB2 = oStatic ? (Vector3){0.0f,0.0f,0.0f} : V3_AplusB(o->velocity, V3_Cross(o->angularVelocity, rBarm));
    Vector3 relVel2 = V3_AsubB(vAtA2, vAtB2);
    Vector3 tangent = V3_AsubB(relVel2, V3_ScaleByF(n, V3_dot(relVel2, n)));
    float tLen = V3_Mag(tangent);
    if (tLen > 0.0001f) {
        tangent = V3_ScaleByF(tangent, 1.f / tLen);
        
        // Evaluate inertia around the TANGENT torque axis (r x t)
        Vector3 rAxT = V3_Cross(rAarm, tangent);
        float rAxT_lenSq = V3_dot(rAxT, rAxT);
        float iAT = (rAxT_lenSq > PHY_EPSILON) ? GetMomentOfInertia(e, V3_ScaleByF(rAxT, 1.0f / vsqrtf(rAxT_lenSq))) : 1.0f;
        float invIAT = (iAT > PHY_EPSILON) ? 1.0f / iAT : 0.0f;
        float angTermAT = ConstIndexIsNPC(e->index) ? 0.0f : rAxT_lenSq * invIAT;
        Vector3 rBxT = V3_Cross(rBarm, tangent);
        float rBxT_lenSq = V3_dot(rBxT, rBxT);
        float iBT = (oStatic || rBxT_lenSq < PHY_EPSILON) ? 0.0f : GetMomentOfInertia(o, V3_ScaleByF(rBxT, 1.0f / vsqrtf(rBxT_lenSq)));
        float invIBT = (iBT > PHY_EPSILON && !oStatic) ? 1.0f / iBT : 0.0f;
        float angTermBT = (oStatic || ConstIndexIsNPC(o->index)) ? 0.0f : rBxT_lenSq * invIBT;
        float invSumT = invMassA + invMassB + angTermAT + angTermBT;
        float mu = (e->dynamicFriction + (oStatic ? 0.4f : o->dynamicFriction)) * 0.5f;
        float jt = -V3_dot(relVel2, tangent) / invSumT;
        jt = vclamp(jt * mu, mu * -vabs(j), mu * vabs(j));
        
        // Apply Tangent (Friction) Impulse
        Vector3 Jt = V3_ScaleByF(tangent, jt);
        e->velocity = V3_AplusB(e->velocity, V3_ScaleByF(tangent, jt * invMassA));
        if (e->collider != COLTYPE_CAP && !ConstIndexIsNPC(e->index)) e->angularVelocity = V3_AplusB(e->angularVelocity, V3_ScaleByF(V3_Cross(rAarm, Jt), invIAT));
        if (!oStatic) {
            o->velocity = V3_AsubB(o->velocity, V3_ScaleByF(tangent, jt * invMassB));
            if (o->collider != COLTYPE_CAP && !ConstIndexIsNPC(o->index)) o->angularVelocity = V3_AsubB(o->angularVelocity, V3_ScaleByF(V3_Cross(rBarm, Jt), invIBT));
        }
    }
    
    float angDelta = vabs(V3_Mag(e->angularVelocity) - V3_Mag(e->lastAngularVelocity));
    e->angularVelocity = V3_ScaleByF(e->angularVelocity,1.0f - angDelta); // Dampen large deltas
    e->lastAngularVelocity = e->angularVelocity;

    // Positional correction (Baumgarte Stabilization)
    float correction = vmin(vmax(overlap - 0.001f, 0.0f) * 0.2f, 0.04f);
    float corrA = correction * invMassA / (invMassA + invMassB + PHY_EPSILON);
    float corrB = correction * invMassB / (invMassA + invMassB + PHY_EPSILON);
    SetPosition(e, V3_AplusB(e->position, V3_ScaleByF(n, corrA)), false);
    if (!oStatic) SetPosition(o, V3_AsubB(o->position, V3_ScaleByF(n, corrB)), false);
}

void Physics() {
    if (Sys_Global.gamePaused || Sys_Global.menuActive) return;

    float dt = vclamp((float)(Sys_Global.pauseRelativeTime - Sys_Global.last_physics_time), 0.0005f, 0.1f);
    Sys_Global.last_physics_time = Sys_Global.pauseRelativeTime;
    u8 substeps = (u8)vclamp((u32)(dt / MAX_STEP_SIZE + 0.5f), 1u, (u32)MAX_SUBSTEPS);
    float dtsub = dt / (float)substeps; dynamicEntityCount = 0;
    for (int i = 0; i < Sys_Global.loadedInstances && dynamicEntityCount < 512; ++i) { Entity *e = &Sys_Global.instances[i]; if ((e->entflags & EF_RIGIDBODY) && (e->entflags & EF_ACTIVE) && e->collider != COLTYPE_NONE) {dynamicEntities[dynamicEntityCount++] = i;} }
    for (int i = 0; i < Sys_Global.loadedInstances; ++i) { // Update cell index for all entities
        Entity *e = &Sys_Global.instances[i];
        e->cellX = (i16)PosGetCellCoordX(e->position.x); e->cellZ = (i16)PosGetCellCoordZ(e->position.z); e->cellIndex = PosGetCellCoordsP(e->cellX,e->cellZ);
        e->radius = (e->modelIndex < MODEL_IDX_MAX) ? modelBounds[e->modelIndex] * vmax(vmax(e->scale.x,e->scale.y),e->scale.z) : GetCollisionRadius(e);
        e->colliding = false;
    }

    for (u8 s = 0; s < substeps; ++s) { // dynamicEntityCount found to be only 335 on level 1
        MemSetToVForNBytes(cellCounts,0,sizeof(cellCounts));
        for (u16 i = 0; i < Sys_Global.loadedInstances; ++i) { Entity *e = &Sys_Global.instances[i]; u32 cell = (u32)e->cellIndex; if (cell < WORLDX*WORLDX && cellCounts[cell] < 127) cellLists[cell][cellCounts[cell]++] = i; } // Build broadphase grid (~0.013ms)        
        for (u16 i = 0; i < dynamicEntityCount; ++i) { u16 idx = dynamicEntities[i]; Entity *e = &Sys_Global.instances[idx]; ApplyVelocity(e,dtsub); } // Integrate all dynamic bodies (~0.005ms)
        ShapeSphere sa,sb; ShapeBox ba,bb; ShapeCapsule ca,cb;
        for (u16 i = 0; i < dynamicEntityCount; ++i) { // Collision detection and resolution (~32.9ms)
            u16 a = dynamicEntities[i]; Entity *e = &Sys_Global.instances[a]; if (e->collider == COLTYPE_MSH || (Sys_Cheats.noclip && a == PLAYER1)) continue;

            i32 cx = PosGetCellCoordX(e->position.x); i32 cz = PosGetCellCoordZ(e->position.z); u32 mask = GetCollisionMask(e->layer);
            float searchRad = e->radius + V3_Mag(e->velocity) * dtsub + 0.5f; i32 radCells = (i32)(searchRad / CELL_SIZE) + 2;
            typedef struct { OverlapResult r; u16 otherIdx; } Contact;
            Contact contacts[16]; int contactCount = 0;
            for (i32 dx = -radCells; dx <= radCells; ++dx) { // Collect all overlaps this substep and resolve each, found radCells to be 3 or less, mostly 2
                for (i32 dz = -radCells; dz <= radCells; ++dz) {
                    u32 cell = PosGetCellCoordsP(cx + dx,cz + dz); if (cell >= WORLDX*WORLDX) continue;
                    
                    for (u16 k = 0; k < cellCounts[cell]; ++k) {
                        u16 b = cellLists[cell][k]; if (b == a) continue; // Skip self
                        Entity *o = &Sys_Global.instances[b]; if (!(mask & o->layer) || o->collider == COLTYPE_NONE) continue;
                        Vector3 deltaPos = V3_AsubB(e->position,o->position); float distSq = V3_dot(deltaPos,deltaPos), combinedRadius = e->radius * 2.f + o->radius * 2.f; if (distSq > combinedRadius * combinedRadius) continue;

                        OverlapResult r = {0};
                        if      (e->collider == COLTYPE_CAP && o->collider == COLTYPE_CAP) { Entity_GetCap(e,&ca); Entity_GetCap(o,&cb); r=CapCap(ca,cb); }
                        else if (e->collider == COLTYPE_CAP && o->collider == COLTYPE_BOX) { Entity_GetCap(e,&ca); Entity_GetBox(o,&bb); r=CapBox(ca,bb); }
                        else if (e->collider == COLTYPE_BOX && o->collider == COLTYPE_CAP) { Entity_GetBox(e,&bb); Entity_GetCap(o,&ca); r=CapBox(ca,bb); if(r.hit){r.normal=(Vector3){-r.normal.x,-r.normal.y,-r.normal.z};} }
                        else if (e->collider == COLTYPE_BOX && o->collider == COLTYPE_BOX) { Entity_GetBox(e,&ba); Entity_GetBox(o,&bb); r=BoxBox(ba,bb); }
                        else if (e->collider == COLTYPE_SPH && o->collider == COLTYPE_BOX) { Entity_GetSph(e,&sa); Entity_GetBox(o,&ba); r=SphBox(sa.center,sa.radius,ba); }
                        else if (e->collider == COLTYPE_BOX && o->collider == COLTYPE_SPH) { Entity_GetBox(e,&ba); Entity_GetSph(o,&sa); r=SphBox(sa.center,sa.radius,ba); if(r.hit){r.normal=(Vector3){-r.normal.x,-r.normal.y,-r.normal.z};} }
                        else if (e->collider == COLTYPE_SPH && o->collider == COLTYPE_SPH) { Entity_GetSph(e,&sa); Entity_GetSph(o,&sb); r=SphSph(sa.center,sa.radius,sb.center,sb.radius); }
                        else if (e->collider == COLTYPE_CAP && o->collider == COLTYPE_MSH) { Entity_GetCap(e,&ca);                       r=CapMsh(ca,o->modelIndex,&modelMatrices[b*16]); }
                        else if (e->collider == COLTYPE_SPH && o->collider == COLTYPE_MSH) { Entity_GetSph(e,&sa);                       r=SphMsh(sa.center,sa.radius,o->modelIndex,&modelMatrices[b*16]); }
                        else if (e->collider == COLTYPE_BOX && o->collider == COLTYPE_MSH) { Entity_GetBox(e,&ba);                       r=BoxMsh(ba,o->modelIndex,&modelMatrices[b*16]); }
                        else if (e->collider == COLTYPE_CVX && o->collider == COLTYPE_MSH) {                                             r=CvxMsh(e->colliderMeshIndex,&modelMatrices[a*16],o->modelIndex,&modelMatrices[b*16]); if(r.hit){r.normal=(Vector3){-r.normal.x,-r.normal.y,-r.normal.z};} }
                        else if (e->collider == COLTYPE_CAP && o->collider == COLTYPE_CVX) { Entity_GetCap(e,&ca);                       r=CapCvx(ca,o->colliderMeshIndex,&modelMatrices[b*16]); }
                        else if (e->collider == COLTYPE_CVX && o->collider == COLTYPE_CAP) { Entity_GetCap(o,&cb);                       r=CapCvx(cb,e->colliderMeshIndex,&modelMatrices[a*16]); }
                        else if (e->collider == COLTYPE_SPH && o->collider == COLTYPE_CVX) { Entity_GetSph(e,&sa);                       r=SphCvx(sa.center,sa.radius,o->colliderMeshIndex,&modelMatrices[b*16]); }
                        else if (e->collider == COLTYPE_CVX && o->collider == COLTYPE_SPH) { Entity_GetSph(o,&sb);                       r=SphCvx(sb.center,sb.radius,e->colliderMeshIndex,&modelMatrices[a*16]); if(r.hit){r.normal=(Vector3){-r.normal.x,-r.normal.y,-r.normal.z};} }
                        else if (e->collider == COLTYPE_BOX && o->collider == COLTYPE_CVX) { Entity_GetBox(e,&ba);                       r=BoxCvx(ba,o->colliderMeshIndex,&modelMatrices[b*16]); if(r.hit){r.normal=(Vector3){-r.normal.x,-r.normal.y,-r.normal.z};} }
                        else if (e->collider == COLTYPE_CVX && o->collider == COLTYPE_BOX) { Entity_GetBox(o,&bb);                       r=BoxCvx(bb,e->colliderMeshIndex,&modelMatrices[a*16]); }
                        else if (e->collider == COLTYPE_CVX && o->collider == COLTYPE_CVX) {                                             r=CvxCvx(e->colliderMeshIndex,o->colliderMeshIndex,&modelMatrices[a*16],&modelMatrices[b*16]); if(r.hit){r.normal=(Vector3){-r.normal.x,-r.normal.y,-r.normal.z};} }
                        else { r=SphSph(e->position,GetCollisionRadius(e),o->position,GetCollisionRadius(o)); } // Fallback

                        if (r.hit && r.overlapAmount > PHY_EPSILON && contactCount < 16) contacts[contactCount++] = (Contact){r,b};
                    }
                }
            }
            
            for (int cta=0;cta<contactCount-1;++cta) { // Resolve all contacts, largest overlap first, simple insertion sort
                for (int ctb=cta+1;ctb<contactCount;++ctb) { if (contacts[ctb].r.overlapAmount > contacts[cta].r.overlapAmount) {Contact tmp=contacts[cta]; contacts[cta]=contacts[ctb]; contacts[ctb]=tmp;} }
            }

            for (int c = 0; c < contactCount; ++c) {
                OverlapResult *ov = &contacts[c].r; u16 j = contacts[c].otherIdx;
                Entity *o = (j < INSTANCE_COUNT) ? &Sys_Global.instances[j] : NULL; e->colliding = true; if (o) o->colliding = true;
                if (o && (o->entflags & EF_RIGIDBODY) && o->collider != COLTYPE_MSH) ApplyCollisionResponse(e,o,ov);
                else { Entity staticProxy = {0}; staticProxy.mass=0.0f; staticProxy.dynamicFriction=0.4f; staticProxy.collider=COLTYPE_NONE; ApplyCollisionResponse(e,&staticProxy,ov); }
            }

            e->accumulatedForce = (Vector3){0.0f,0.0f,0.0f};
        }

        for (int i = 0; i < Sys_Global.loadedInstances; ++i) { Entity *e = &Sys_Global.instances[i]; e->cellX = (i16)PosGetCellCoordX(e->position.x); e->cellZ = (i16)PosGetCellCoordZ(e->position.z); e->cellIndex = PosGetCellCoordsP(e->cellX, e->cellZ); } // Update cells for next substep
    }
}

ENGINE_TO_MOD void AddForce(u16 i, Vector3 f, bool imp) { Entity *e = &Sys_Global.instances[i]; if (imp) { e->velocity = V3_AplusB(e->velocity,V3_ScaleByF(f,1.0f / vmax(e->mass,0.001f))); } else { e->accumulatedForce = V3_AplusB(e->accumulatedForce,f); } }
ENGINE_TO_MOD void ApplyPlayerMovements() {
    float h = (float)Forward() - (float)Backpedal(), s = (float)StrafeRight() - (float)StrafeLeft();
    float vertInput = (float)SwimUp() - (float)SwimDn();
    Entity *p = &Sys_Global.instances[PLAYER1];
    Quaternion rot = p->rotation; float y2 = rot.y * rot.y;  float xz = rot.x * rot.z;  float wy = rot.w * rot.y;
    p->forward = V3_Normalize((Vector3){ 2.0f * (xz + wy), 2.0f * (rot.y * rot.z - rot.w * rot.x), 1.0f - 2.0f * (rot.x * rot.x + y2) });
    p->right   = V3_Normalize((Vector3){ 1.0f - 2.0f * (y2 + rot.z * rot.z), 2.0f * (rot.x * rot.y + rot.w * rot.z), 2.0f * (xz - wy) });
    Vector3 input = V3_Normalize((Vector3){p->forward.x*h + p->right.x*s, vertInput, p->forward.z*h + p->right.z*s});
    float speed = GetBasePlayerSpeed(PLAYER1,V3_Mag(input) > 0.1f) * 1.75f, accel = Sys_Global.boosterActive ? 1.0f : 3.0f;
    Vector3 cur = p->velocity; 
    Vector3 targetVel = V3_ScaleByF(input, speed);
    if (vabs(vertInput) < 0.001f) targetVel.y = cur.y;
    Vector3 dv = V3_AsubB(targetVel, cur);
    dv = (Vector3){vclamp(dv.x,-10.0f,10.0f), vclamp(dv.y,-10.0f,10.0f), vclamp(dv.z,-10.0f,10.0f)};
    p->velocity = V3_AplusB(cur, V3_ScaleByF(dv, accel * vclamp((float)Sys_Global.deltaTime,0.0005f,0.1f)));
}
// ================================= Console Emulator System - CHEATS!
static i32 currentEntryLength=0, numHistory=0, historyPos=0; char consoleEntryText[TEXT_BUFFER_SIZE],history[7][TEXT_BUFFER_SIZE];
static Vector3 ressurectionLocations[10] = {{-27.386f,-55.488f,26.5941f},/*0/R*/ {40.903f,-42.372f,-30.78f},/*1*/      {30.67407f,-25.832f,10.21412f},/*2*/ {38.26813f,-15.498f,20.37825f},/*3*/ {-19.48f,-7.928f,22.954f},/*4*/ {-24.358f,12.5956f,31.8497f},/*5*/ {-22.3568f,33.7845f,-30.728f},/*6*/  {2.228084f,50.95243f,7.532025f},/*7*/ {10.068f,58.897f,13.973f},/*8*/ {2.303f,106.77f,-38.554f}/*9*/};
static Vector3 cyberSpaceEntryLocations[8] = { {210.6834f,2.812f,-24.378f},/*0*/ {195.420f,-13.44000f, 33.2800f},/*1*/ {157.1608f,-15.53f,47.331f},/*2a, if cyberport localPosition.x < -26.0f*/ {256.0416f,-0.716f,62.48789f},/*2b level 2 secondary cyberport position*/ {126.43f,29.56733f,34.24f},/*5*/  {177.612f,  3.29494f,108.7725f},/*6*/ {244.735f,41.99257f,-19.695f},/*8*/                                       {185.161f,84.502f,-46.04246f},/*9*/ };
ENGINE_TO_MOD void ToggleConsole() { static bool imWasActPrior = false; if (!Sys_Cheats.consoleActive) {imWasActPrior = Sys_Global.inventoryMode;} Sys_Cheats.consoleActive = !Sys_Cheats.consoleActive; if (Sys_Cheats.consoleActive) { Sys_Global.inventoryMode = true; } else if (!imWasActPrior && Sys_Global.inventoryMode) {ForceShootMode();} }
static void AddToHistory(const char* entry) {
    if (GetStringLength(entry) == 0 || (numHistory > 0 && StringsEqual(entry,history[numHistory - 1]))) return;
    
    if (numHistory < 7) { StringCopyInto_A_From_B(history[numHistory],entry,TEXT_BUFFER_SIZE); numHistory++; }
    else {
        for (int i = 0; i < 7 - 1; i++) StringCopyInto_A_From_B(history[i],history[i + 1],TEXT_BUFFER_SIZE); // Shift list toward 0
        StringCopyInto_A_From_B(history[7 - 1], entry,TEXT_BUFFER_SIZE);
    }
}

static void RecallHistory(int direction) { // direction 1 up (older), -1 down (newer)
    if (direction == 1) { // up
        if (historyPos > 0) { historyPos--; StringCopyInto_A_From_B(consoleEntryText,history[historyPos],TEXT_BUFFER_SIZE); currentEntryLength = GetStringLength(consoleEntryText); }
    } else if (direction == -1) { // down
        if (historyPos < numHistory) {
            historyPos++;
            if (historyPos == numHistory) { consoleEntryText[0] = currentEntryLength = 0; } else { StringCopyInto_A_From_B(consoleEntryText,history[historyPos],TEXT_BUFFER_SIZE); currentEntryLength = GetStringLength(consoleEntryText); }
        }
    }
}

typedef void (*ConsoleCmdFuncNoArg)(); typedef void (*ConsoleCmdFuncInt)(int); typedef void (*ConsoleCmdFuncStr)(const char*);
typedef struct { const char* name; union {ConsoleCmdFuncNoArg noArg; ConsoleCmdFuncInt withInt; ConsoleCmdFuncStr withStr; void* raw;} func; enum {CMD_NOARG,CMD_INT,CMD_STR}type;} ConsoleCommand;
__attribute__((pure)) static int CommandMatch(const char* input, const char* cmd) {
    while (*cmd && *input) { char c1 = CharToLower((u8)*input++); char c2 = CharToLower((u8)*cmd++); if (c1 == ' ' || c1 == '_') {c1 = ' ';} if (c2 == ' ' || c2 == '_') {c2 = ' ';} if (c1 != c2) {return 0;} }
    return *cmd == '\0' && (*input == '\0' || CharacterIsEmpty((u8)*input) || *input == '_');
}

static void cmd_noclip() { Sys_Cheats.noclip = !Sys_Cheats.noclip; if (Sys_Cheats.noclip) { Sys_Global.instances[PLAYER1].velocity = (Vector3){ 0.0f, 0.0f, 0.0f }; CenterStatusPrint("noclip: %s", Sys_Text.stringTable[1000]); /*"ACTIVATED"*/} else {CenterStatusPrint("noclip: %s", Sys_Text.stringTable[717]); /*"DISABLED"*/} }
static void cmd_showphys() { Sys_Cheats.showPhys = !Sys_Cheats.showPhys; if (Sys_Cheats.showPhys) {CenterStatusPrint("showPhys: %s", Sys_Text.stringTable[1000]); /*"ACTIVATED"*/} else {CenterStatusPrint("showPhys: %s", Sys_Text.stringTable[717]); /*"DISABLED"*/} }
void EnableCheatArsenal(u8 level) { (void)level; } // TODO
void cmd_kill() { Sys_Global.instances[PLAYER1].health = Sys_Global.instances[PLAYER1].cyberHealth = 0.0f; CenterStatusPrint("%s", Sys_Text.stringTable[1011]); } // "Player decides to become a cyborg."
void cmd_undo() { if (Sys_Cheats.editMode) { CenterStatusPrint("Last spawned object removed"); } else { CenterStatusPrint("Cannot undo when not in Edit Mode"); } } // TODO actually track and despawn last
void ScreenShake(float force, double duration) { Sys_Global.shakeFinished = Sys_Global.pauseRelativeTime + duration; float shakeForce = (force < 0.48f) ? force : 0.48f; (void)shakeForce; } // TODO actually shake
void Shake(float force) { float forc = (force <= 0.0f) ? 1.0f : force; ScreenShake(forc,1.0); }// The whole station is a shakin' and a movin'!
void cmd_shake() { Shake(-1); CenterStatusPrint("SHAKIN LIKE A LEAF!"); }
static void cmd_edit() {
    Sys_Cheats.editMode = !Sys_Cheats.editMode;
    if (Sys_Cheats.editMode) { Sys_Cheats.noclip = Sys_Cheats.notarget = true; CenterStatusPrint("edit mode: %s","Edit Mode activated! The current level\ncan be shaped to your heart's content!"); }
    else { Sys_Cheats.noclip = Sys_Cheats.notarget = false; CenterStatusPrint("%s", Sys_Text.stringTable[999]); } // "Edit Mode deactivated, normal play"
}

static int ParseLevelArg(const char* arg) {
    if (!arg || !*arg) return -1;

    char clean[64] = {0}; int j = 0;
    for (int i = 0; arg[i] && j < 60; i++) { if (arg[i] != ' ' && arg[i] != '_') clean[j++] = CharToLower((unsigned char)arg[i]); }   clean[j] = '\0';
    if (StringsEqual(clean, "r")      || StringFindSubstring(clean, "reactor")) return 0;
    if (StringFindSubstring(clean, "g1") || StringFindSubstring(clean, "10")) return 10;
    if (StringFindSubstring(clean, "g2") || StringFindSubstring(clean, "11")) return 11;
    if (StringFindSubstring(clean, "g4") || StringFindSubstring(clean, "12")) return 12;
    if (StringFindSubstring(clean, "g3")) { CenterStatusPrint("%s", Sys_Text.stringTable[1001]); return -2; }// "Gamma grove already jettisoned! Those poor arrogant people."
    int level = StringToInt(clean); if (level >= 0 && level < Sys_Global.numLevels) return level;
    return -1; // Invalid
}

u8 queuedLevelToLoad = 255u;
static void cmd_loadlevel(const char* arg) {
    if (Sys_Global.menuActive) { CenterStatusPrint("%s", Sys_Text.stringTable[1015]); return; } // "Cannot load levels via cheat while on the main menu!"
    int level = ParseLevelArg(arg); if (level == -2) return; // Already printed g3 message
    if (level < 0 || level > 12) { CenterStatusPrint("cmd_loadlevel invalid level argument %u",level); return; }

    CenterStatusPrint("Loading level %u",level); queuedLevelToLoad = level; LoadLevel(level); SetPosition(&Sys_Global.instances[PLAYER1],ressurectionLocations[level > 9 ? 6 : level],true); (void)cyberSpaceEntryLocations;
}

static void cmd_loadarsenal(const char* arg) { int level = ParseLevelArg(arg); if (level >= 0 && level < Sys_Global.numLevels) { EnableCheatArsenal(level); } }
static void cmd_summon(int itemConstIndex) { if (!ConstIndexInBounds(itemConstIndex)) { SpawnDynamicObject(itemConstIndex, true); CenterStatusPrint("Summoned object ID %d", itemConstIndex); } else { CenterStatusPrint("Invalid object ID: %s", itemConstIndex); } }
static void cmd_notarget() { Sys_Cheats.notarget = !Sys_Cheats.notarget; CenterStatusPrint("notarget: %s", Sys_Cheats.notarget ? Sys_Text.stringTable[1000] : Sys_Text.stringTable[717]); }
static void cmd_showfps() { Sys_Cheats.showFPS = !Sys_Cheats.showFPS; }
static void cmd_showlocation() { Sys_Cheats.showLocation = !Sys_Cheats.showLocation; }
static void cmd_help() { CenterStatusPrint("There's no one to save you now Hacker!"); }
static void cmd_nomoney() { CenterStatusPrint("Nice try, there's no money here."); }
static void cmd_god() { Sys_Cheats.god = !Sys_Cheats.god; CenterStatusPrint("god mode: %s", Sys_Cheats.god ? Sys_Text.stringTable[1000] : Sys_Text.stringTable[717]); }
static void cmd_energy() { Sys_Cheats.redbull = !Sys_Cheats.redbull; if (Sys_Cheats.redbull) {CenterStatusPrint("%s", Sys_Text.stringTable[1006]);/*"I feel the power! 0 energy consumption!"*/} else {CenterStatusPrint("%s", Sys_Text.stringTable[1005]);/*Energy usage normal*/} }
static void SetSkyRotateSpeed() { static const float skyRotateSpeeds[] = { 0.05f, 1.0f, 2.5f, 3.75f, 6.25f }; glUseProgram(Sys_Render.imageBlitShaderProgram); glUniform1f(30,skyRotateSpeeds[Sys_Cheats.dizzyLevel]); }
static void cmd_dizzy() { Sys_Cheats.dizzyLevel = (Sys_Cheats.dizzyLevel >= 3) ? 0 : Sys_Cheats.dizzyLevel + 1; SetSkyRotateSpeed(); }
static void cmd_bottomless() { Sys_Cheats.bottomless = !Sys_Cheats.bottomless; if (Sys_Cheats.bottomless) {CenterStatusPrint("bottomlessclip! %s",Sys_Text.stringTable[1002]);/*"Bring it!"*/} else {CenterStatusPrint("%s",Sys_Text.stringTable[1003]);/*"Hose disconnected from interdimensional wormhole. Normal ammo operation restored."*/} }
static void cmd_nohud() { Sys_Cheats.noHUD = !Sys_Cheats.noHUD; if (Sys_Cheats.noHUD) {CenterStatusPrint("%s",Sys_Text.stringTable[1004]);/*"No HUD! Enjoy the cinematic screenshot experience!"*/} else { CenterStatusPrint("HUD %s",Sys_Text.stringTable[1000]);/*"ACTIVATED"*/} }
static void cmd_iamshodan() { Sys_Cheats.superoverride = !Sys_Cheats.superoverride; if (Sys_Cheats.superoverride) {CenterStatusPrint("%s",Sys_Text.stringTable[1010]);/*"Full security override enabled!"*/ } else {CenterStatusPrint("%s",Sys_Text.stringTable[1009]);/*"SHODAN has regained control of security from you"*/} }
static void cmd_staminup() { Sys_Cheats.fatigueCheat = !Sys_Cheats.fatigueCheat; if (Sys_Cheats.fatigueCheat) { CenterStatusPrint("Stamin-Up! %s",Sys_Text.stringTable[1013]); SetModFatigue(0.0f); } else {CenterStatusPrint("%s",Sys_Text.stringTable[1012]); } }
static void cmd_mrbean()  { CenterStatusPrint("Nice try, there are no go carts to slow down here"); } static void cmd_simonfoster()   { CenterStatusPrint("Nice try, nothing to paint here"); } static void cmd_richardbranson() { CenterStatusPrint("Nice try, there's no money here. You do realize this isn't Rollercoaster Tycoon right?"); } static void cmd_johnwardley()       { CenterStatusPrint("WOW!"); }
static void cmd_johnmace(){ CenterStatusPrint("Nice try, there's nothing to pay double for here"); }  static void cmd_melaniewarn()   { CenterStatusPrint("I feel happy!!!"); }                 static void cmd_damonhill()      { CenterStatusPrint("Nice try, there are no go carts to speed up here"); }                                       static void cmd_michaelschumacher() { CenterStatusPrint("Nice try, there are no go carts to give ludicrous speed here"); }
static void cmd_tonyday() { CenterStatusPrint("Ok, now I want a hamburger"); }                        static void cmd_katiebrayshaw() { CenterStatusPrint("Hi there! Hello! Hey! Howdy!"); }
static void cmd_sudo()    { CenterStatusPrint("Super user access granted...ERROR: access restricted by SHODAN!"); }
static void cmd_git(const char* arg) {
    if (!arg) arg = "";
    static const char* cmds[] = {"pull",  "remote: Enumerating objects: 24601, done.\nFailed, could not connect with origin/triop.", "fetch", "remote: Enumerating objects: 24601, done.\nFailed, could not connect with origin/triop.",      "status","Your branch is up to date with origin/triop.\nWorking directory clean.",
                                 "log",   "<Merge pull request #451 from SHODAN/NeuralLinkBugfix> 6 months ago...",                  "reflog","dc51440 HEAD0 -> master: commit: Establish neural connection ... ERROR: invalid ID `2-4601`.", "merge", "Failed, could not connect with origin/triop.",
                                 "push",  "Could not find Username for 'triopttp://192.168.1.451'.",                                 "clone", "Failed, connection blocked by SHODAN. Employee ID invalid." };
    for (int i = 0; i < 16; i += 2) { if (StringFindSubstring(arg, cmds[i])) { CenterStatusPrint(cmds[i+1]); return; } }
    if (StringFindSubstring(arg, "branch") || StringFindSubstring(arg, "-b")) { const char *last = StringFindLastChar(arg, ' '); CenterStatusPrint("Created new branch %s", last ? last + 1 : "unknown"); }
    else CenterStatusPrint("Branch name not recognized. Contact your TriopBucket representative.");
}

static void cmd_restart()        { CenterStatusPrint("Yeah...better not"); }                                              static void cmd_cd()             { CenterStatusPrint("Attempting to access directory... already at root"); }
static void cmd_justinbailey()   { CenterStatusPrint("Well, you don't have a suit already so..."); }                      static void cmd_woodstock()      { CenterStatusPrint("How much wood could a woodchuck chuck...there's no wood in SPACE!"); }
static void cmd_zelda()          { CenterStatusPrint("Too late, already been to level 1"); }                              static void cmd_quarry()         { CenterStatusPrint("There's obsidian on levels 6 and 8 if you want to feel decadent,\notherwise we are lacking in the stone department."); }
static void cmd_allyourbase()    { CenterStatusPrint("ERROR: SHODAN has overriden your command, remove SHODAN first."); } static void cmd_iamironman()     { CenterStatusPrint("That's nice dear."); }
static void cmd_idkfa()          { CenterStatusPrint("I can only hold 7 weapons!! Nice try dearies!"); }                  static void cmd_ai()             { CenterStatusPrint("Only AI allowed around here is SHODAN"); }
static void cmd_aireal()         { CenterStatusPrint("In my magnificence, I shape clay, crafting new lifeforms..."); }    static void cmd_quit()           { OS_Exit(0); }
static const ConsoleCommand consoleCmds[] = {
    { "noclip",  {.noArg=cmd_noclip}, CMD_NOARG},{"idclip",      {.noArg=cmd_noclip},CMD_NOARG},      {"no clip",       {.noArg = cmd_noclip},      CMD_NOARG}, {"showphys",{.noArg = cmd_showphys},CMD_NOARG},
    { "god",     {.noArg=cmd_god}, CMD_NOARG},   {"overwhelming",{.noArg=cmd_god},   CMD_NOARG},      {"whosyourdaddy", {.noArg = cmd_god},         CMD_NOARG},
    { "iddqd",   {.noArg=cmd_god}, CMD_NOARG},   {"notarget",    {.noArg=cmd_notarget},CMD_NOARG},    {"no target",     {.noArg = cmd_notarget},    CMD_NOARG},
    { "editmode",{.noArg=cmd_edit},CMD_NOARG},   {"edit",        {.noArg=cmd_edit},  CMD_NOARG},      {"edit mode",     {.noArg = cmd_edit},        CMD_NOARG},
    { "editor",  {.noArg=cmd_edit},CMD_NOARG},   {"undo",        {.noArg=cmd_undo},  CMD_NOARG},      {"showfps",       {.noArg = cmd_showfps},     CMD_NOARG},
    { "show fps",{.noArg=cmd_showfps},CMD_NOARG},{"showlocation",{.noArg=cmd_showlocation},CMD_NOARG},{"show location", {.noArg = cmd_showlocation},CMD_NOARG},
    { "nohud",   {.noArg=cmd_nohud},CMD_NOARG},  {"no hud",      {.noArg=cmd_nohud}, CMD_NOARG},      {"bottomlessclip",{.noArg = cmd_bottomless},  CMD_NOARG},
    { "bottomless clip",{.noArg=cmd_bottomless},CMD_NOARG},{"load",{.withStr=cmd_loadlevel},CMD_STR}, {"loadarsenal",   {.withStr = cmd_loadarsenal}, CMD_STR}, 
    { "load arsenal", {.withStr = cmd_loadarsenal}, CMD_STR}, { "summon_obj", {.withInt = cmd_summon}, CMD_INT}, {"summonobj",{.withInt = cmd_summon},CMD_INT},
    { "motherlode",    {.noArg=cmd_nomoney},        CMD_NOARG},{"rosebud",           {.noArg=cmd_nomoney},          CMD_NOARG},{"kaching",        {.noArg=cmd_nomoney},       CMD_NOARG},
    { "money",         {.noArg=cmd_nomoney},        CMD_NOARG},{"dizzy",             {.noArg=cmd_dizzy},            CMD_NOARG},{"help",           {.noArg=cmd_help},          CMD_NOARG},
    { "ifeelthepower", {.noArg = cmd_energy},       CMD_NOARG},{ "power",            {.noArg=cmd_energy},           CMD_NOARG},{"energy",         {.noArg=cmd_energy},        CMD_NOARG},
    { "i feel the power",{.noArg = cmd_energy},     CMD_NOARG},{"i am shodan",       {.noArg=cmd_iamshodan},        CMD_NOARG},{"iamshodan",      {.noArg=cmd_iamshodan},     CMD_NOARG},
    { "mr. bean",      {.noArg = cmd_mrbean},       CMD_NOARG},{"simon foster",      {.noArg=cmd_simonfoster},      CMD_NOARG},{"richard branson",{.noArg=cmd_richardbranson},CMD_NOARG},
    { "john wardley",  {.noArg = cmd_johnwardley},  CMD_NOARG},{"john mace",         {.noArg=cmd_johnmace},         CMD_NOARG},{"melanie warn",   {.noArg=cmd_melaniewarn},   CMD_NOARG},
    { "damon hill",    {.noArg = cmd_damonhill},    CMD_NOARG},{"michael schumacher",{.noArg=cmd_michaelschumacher},CMD_NOARG},{"tony day",       {.noArg=cmd_tonyday},       CMD_NOARG},
    { "katie brayshaw",{.noArg = cmd_katiebrayshaw},CMD_NOARG},{"sudo",              {.noArg=cmd_sudo},             CMD_NOARG},{"admin",          {.noArg=cmd_sudo},          CMD_NOARG},
    { "git",           {.withStr=cmd_git},            CMD_STR},{"restart",           {.noArg=cmd_restart},          CMD_NOARG},{"quit",           {.noArg=cmd_quit},          CMD_NOARG},
    { "exit",          {.noArg = cmd_quit},         CMD_NOARG},{"cd",                {.noArg=cmd_cd},               CMD_NOARG},{"./",             {.noArg=cmd_cd},            CMD_NOARG},
    { "kill",          {.noArg = cmd_kill},         CMD_NOARG},{"suicide",           {.noArg=cmd_kill},             CMD_NOARG},{"die",            {.noArg=cmd_kill},          CMD_NOARG},
    { "justinbailey",  {.noArg = cmd_justinbailey}, CMD_NOARG},{"woodstock",         {.noArg=cmd_woodstock},        CMD_NOARG},{"quarry",         {.noArg=cmd_quarry},        CMD_NOARG},
    { "zelda",         {.noArg = cmd_zelda},        CMD_NOARG},{"allyourbasearebelongtous",{.noArg=cmd_allyourbase},CMD_NOARG},{"all your base",  {.noArg=cmd_allyourbase},   CMD_NOARG},
    { "i am iron man", {.noArg = cmd_iamironman},   CMD_NOARG},{"i am amazing",      {.noArg=cmd_iamironman},       CMD_NOARG},{"i am cool",      {.noArg=cmd_iamironman},    CMD_NOARG},
    { "i am best",     {.noArg = cmd_iamironman},   CMD_NOARG},{"idkfa",             {.noArg=cmd_idkfa},            CMD_NOARG},{"impulse 9",      {.noArg=cmd_idkfa},         CMD_NOARG},
    { "undo",          {.noArg = cmd_undo},         CMD_NOARG},{"shake",             {.noArg=cmd_shake},            CMD_NOARG},{"tired",          {.noArg=cmd_staminup},      CMD_NOARG},
    { "staminup",      {.noArg = cmd_staminup},     CMD_NOARG},{"grok",              {.noArg=cmd_ai},               CMD_NOARG},{"chatgpt",        {.noArg=cmd_ai},            CMD_NOARG},
    { "claude",        {.noArg = cmd_ai},           CMD_NOARG},{"gemini",            {.noArg=cmd_ai},               CMD_NOARG},{"shodan",         {.noArg=cmd_aireal},        CMD_NOARG}, {NULL,{.raw = NULL},CMD_NOARG} // sizeof helper
};

void ProcessConsoleCommand(const char* command) {
    if (command == NULL || GetStringLength(command) == 0) { ToggleConsole(); return; }

    char ts[TEXT_BUFFER_SIZE]; StringCopyInto_A_SubstringFrom_B(ts,sizeof(ts)-1,command,TEXT_BUFFER_SIZE);
    ts[sizeof(ts)-1] = '\0';
    const char* command_trimmed = ts;    while (*command_trimmed && CharacterIsEmpty((unsigned char)*command_trimmed)) command_trimmed++;
    const char* space = command_trimmed; while (*space && !CharacterIsEmpty((unsigned char)*space)) space++;
    const char* arg_start = space;       while (*arg_start && CharacterIsEmpty((unsigned char)*arg_start)) arg_start++;
    AddToHistory(command);
    bool commandProcessed = false;
    for (u16 i=0;consoleCmds[i].name!=NULL;++i) {
        const ConsoleCommand* cmd = &consoleCmds[i];
        if (CommandMatch(command_trimmed,cmd->name)) {
                   if (cmd->type == CMD_NOARG) {             cmd->func.noArg();                              commandProcessed = true;
            } else if (cmd->type == CMD_STR && *arg_start) { cmd->func.withStr(*arg_start ? arg_start : ""); commandProcessed = true;
            } else { // CMD_INT
                if (!*arg_start) CenterStatusPrint("Missing argument, usage: %s <number>",cmd->name);
                else { cmd->func.withInt(StringToInt(arg_start)); commandProcessed = true; }
            }
        }
    }

    if (!commandProcessed) CenterStatusPrint("%s%s",Sys_Text.stringTable[1014],command_trimmed); // "Unknown command or function: "
    consoleEntryText[0] = currentEntryLength = 0; historyPos = numHistory; // Position beyond newest for empty
    ToggleConsole();
}

void ConsoleEmulator(i32 keycode) {
    if (keycode == KEY_UP || keycode == KEY_DOWN) { RecallHistory(keycode == KEY_UP ? 1 : -1); return; }
    if (keycode == KEY_U && Sys_Input.keyStates[KEY_LEFT_CONTROL].down) { consoleEntryText[0]='\0'; currentEntryLength=0; return; } // Clear the input
    
    if (keycode >= KEY_A && keycode <= KEY_Z) { // Handle alphabet keys
        if (currentEntryLength < (TEXT_BUFFER_SIZE - 1)) { // Ensure we don't overflow the buffer
            char c = 'a' + (keycode - KEY_A); // Map keycode to lowercase character
            consoleEntryText[currentEntryLength] = c; consoleEntryText[currentEntryLength + 1] = '\0'; currentEntryLength++;
        }
    } else if (keycode >= KEY_1 && keycode <= KEY_9) { // Handle number keys 1-9
        if (currentEntryLength < (TEXT_BUFFER_SIZE - 1)) {
            char c = '1' + (keycode - KEY_1); // Map to '1'-'9'
            consoleEntryText[currentEntryLength] = c; consoleEntryText[currentEntryLength + 1] = '\0'; currentEntryLength++;
        }
    } else if (keycode == KEY_0) { // Handle '0'
        if (currentEntryLength < (TEXT_BUFFER_SIZE - 1)) { consoleEntryText[currentEntryLength]='0'; consoleEntryText[currentEntryLength + 1]='\0'; currentEntryLength++; }
    } else if (keycode == KEY_MINUS || keycode == KEY_KP_SUBTRACT) {
        if (currentEntryLength < (TEXT_BUFFER_SIZE - 1)) { consoleEntryText[currentEntryLength]=(Sys_Input.keyStates[KEY_LEFT_SHIFT].down || Sys_Input.keyStates[KEY_RIGHT_SHIFT].down) ? '_' : '-'; consoleEntryText[currentEntryLength + 1]='\0'; currentEntryLength++; }
    } else if (keycode == KEY_BACKSPACE && currentEntryLength > 0) { currentEntryLength--; consoleEntryText[currentEntryLength]='\0'; } // Handle backspace
    else if (keycode == KEY_SPACE) { // Handle space
        if (currentEntryLength < (TEXT_BUFFER_SIZE - 1)) { consoleEntryText[currentEntryLength]=' '; consoleEntryText[currentEntryLength + 1]='\0'; currentEntryLength++; }
    } else if (keycode == KEY_ENTER || keycode == KEY_KP_ENTER) { DualLog("Console command: %s\n",consoleEntryText); ProcessConsoleCommand(consoleEntryText); }
}
// ============ Audio System
#define AUDIO_RATE      48000
#define AUDIO_CHANNELS  2
#define AUDIO_PERIOD_MS 10
#define AUDIO_PERIODS   4
#define AUDIO_FRAMES    ((AUDIO_RATE * AUDIO_PERIOD_MS) / 1000)
#define AUDBUF_SIZE (AUDIO_FRAMES*AUDIO_PERIODS)
void* MemCpyFromBtoAForNBytes(void *dst, const void *src, size_t n);
extern SettingsSystem Sys_Settings; extern GlobalContext Sys_Global;
#ifdef WINDOWS
    #define FAILED(hr)    ((i32)(hr) <  0)
    #define PCM_NONBLOCK (1<<1)
    #define PCM_FORMAT_S16_LE 2
    #define REFIID const GUID *const
    typedef struct IMMDevice IMMDevice; typedef struct IMMDeviceEnumerator IMMDeviceEnumerator;
    typedef struct{ i32(__stdcall*q)(void*,const void*,void**); u32(__stdcall*a)(void*); u32(__stdcall*Release)(void*); i32(__stdcall* Activate)(void*,const void*,u32,void*,void**);} IMMDeviceVtbl; struct IMMDevice{IMMDeviceVtbl*lpVtbl;};
    typedef struct{ i32(__stdcall*q)(void*,const void*,void**); u32(__stdcall*a)(void*); u32(__stdcall*Release)(void*); i32(__stdcall*e)(void*,int,u32,void**); i32(__stdcall*GetDefaultAudioEndpoint)(void*,int,int,IMMDevice**);}IMMDeviceEnumeratorVtbl;struct IMMDeviceEnumerator{IMMDeviceEnumeratorVtbl*lpVtbl;};
    typedef struct IAudioClient IAudioClient; typedef struct IAudioRenderClient IAudioRenderClient; typedef struct { u16 t,n; u32 s, a; u16 b,w,c; } WAVEFORMATEX;
    typedef struct IAudioClientVtbl { i32 (__stdcall *QueryInterface)(void*, const void*,void**); u32 (__stdcall *AddRef)(void*); u32 (__stdcall *Release)(void*); i32 (__stdcall *Initialize)(void*,int,u32,i64,i64,const WAVEFORMATEX*,const void*); 
        i32 (__stdcall *GetBufferSize)(void*,u32*); i32 (__stdcall *GetStreamLength)(void*,i64*); i32 (__stdcall *GetCurrentPadding)(void*,u32*); i32 (__stdcall *IsFormatSupported)(void*, int, const WAVEFORMATEX*, WAVEFORMATEX**); 
        i32 (__stdcall *GetMixFormat)(void*,WAVEFORMATEX**); i32 (__stdcall *GetDevicePeriod)(void*,i64*,i64*); i32 (__stdcall *Start)(void*); i32 (__stdcall *Stop)(void*); i32 (__stdcall *Reset)(void*); i32 (__stdcall *SetEventHandle)(void*,void*);
        i32 (__stdcall *GetService)(void*,const void*,void**);
    } IAudioClientVtbl;
    struct IAudioClient { IAudioClientVtbl* lpVtbl; };
    typedef struct IAudioRenderClientVtbl { i32 (__stdcall *QueryInterface)(void*,const void*,void**); u32 (__stdcall *AddRef)(void*); u32 (__stdcall *Release)(void*); i32 (__stdcall *GetBuffer)(void*,u32,u8**); i32 (__stdcall *ReleaseBuffer)(void*,u32,u32); } IAudioRenderClientVtbl;
    struct IAudioRenderClient { IAudioRenderClientVtbl* lpVtbl; };
    typedef struct IUnknown IUnknown; typedef struct IUnknownVtbl { i32 (__stdcall *QueryInterface)(IUnknown* This, const GUID* riid, void** ppvObject); u32 (__stdcall *AddRef)(IUnknown* This); u32 (__stdcall *Release)(IUnknown* This); } IUnknownVtbl; struct IUnknown { const IUnknownVtbl* lpVtbl; };
    typedef u32 snd_pcm_uframes_t; typedef struct { int format,access,rate,channels,period_frames,periods; } pcm_params_t; typedef struct { snd_pcm_uframes_t hw_ptr; }  pcm_status_t; typedef struct { snd_pcm_uframes_t appl_ptr; } pcm_control_t;
    typedef struct { pcm_status_t status; pcm_control_t control; } pcm_sync_t; 
    typedef struct {IAudioClient *client; IAudioRenderClient *render; u32 buffer_frames; i32 rate,channels,period_frames; bool open; } wasapi_dev_t;
    extern i32 WINAPI CoInitializeEx(void*,u32); extern i32 WINAPI CoCreateInstance(const GUID*,IUnknown*,u32,REFIID,void**);
    static wasapi_dev_t wasapi_devs[8]; static int wasapi_dev_count = 0;
    #define FD_TO_IDX(fd) ((int)(intptr_t)(fd)-100)
    #define IDX_TO_FD(i)  ((FHandle)(intptr_t)((i)+100))
    static const GUID IID_IAudioClient = {0x1CB9AD4C,0xDBFA,0x4C32,{0xB1,0x78,0xC2,0xF5,0x68,0xA7,0x03,0xB2}}; static const GUID IID_IAudioRenderClient = {0xF294ACFC,0x3146,0x4483,{0xA7,0xBF,0xAD,0xDC,0xA7,0xC2,0x60,0xE2}};
    static int wasapi_init_device(IMMDevice *dev,int rate,int channels,int period_frames,int periods) {
        if (wasapi_dev_count>=8) return -1;
        
        wasapi_dev_t *w = &wasapi_devs[wasapi_dev_count]; 
        i32 hr = dev->lpVtbl->Activate(dev,&IID_IAudioClient,23,NULL,(void**)&w->client); if (FAILED(hr)) { DualLogError("WASAPI Activate failed, %u\n",hr); return -1; }
        
        WAVEFORMATEX fmt = {1,(u16)channels,(u32)rate,(u32)(rate*channels*2),(u16)(channels*2),16,0}; i64 buf_dur = (i64)(period_frames*periods)*10000000LL/rate;
        hr = w->client->lpVtbl->Initialize(w->client,0,524288,buf_dur,0,&fmt,NULL); if (FAILED(hr)) { w->client->lpVtbl->Release(w->client); return -1; }
        
        w->client->lpVtbl->GetBufferSize(w->client,&w->buffer_frames);
        hr = w->client->lpVtbl->GetService(w->client,&IID_IAudioRenderClient,(void**)&w->render); if (FAILED(hr)) { w->client->lpVtbl->Release(w->client); return -1; }
        
        w->client->lpVtbl->Start(w->client);w->rate=rate; w->channels=channels; w->period_frames=period_frames; w->open=true;
        return wasapi_dev_count++;
    }
    
    static const GUID CLSID_MMDeviceEnumerator_ = {0xBCDE0395,0xE52F,0x467C,{0x8E,0x3D,0xC4,0x57,0x92,0x91,0x69,0x2E}}; static const GUID IID_IMMDeviceEnumerator_  = {0xA95664D2,0x9614,0x4F35,{0xA7,0x46,0xDE,0x8D,0xB6,0x36,0x17,0xE6}};
    FHandle pcm_open_all(int rate,int channels,int period_frames,int periods) {
        CoInitializeEx(NULL,0); IMMDeviceEnumerator *en = NULL; if (FAILED(CoCreateInstance(&CLSID_MMDeviceEnumerator_,NULL,23,&IID_IMMDeviceEnumerator_,(void**)&en))) { DualLogError("CoCreateInstance fail\n"); return INVALID_FHANDLE; }
        IMMDevice *dev = NULL; i32 hr = en->lpVtbl->GetDefaultAudioEndpoint(en,0,0,&dev); en->lpVtbl->Release(en); if (FAILED(hr)||!dev) return INVALID_FHANDLE;
        int idx = wasapi_init_device(dev,rate,channels,period_frames,periods); dev->lpVtbl->Release(dev); if (idx<0) return INVALID_FHANDLE;
        return IDX_TO_FD(0);
    }

    int pcm_sync(FHandle fd, pcm_sync_t *sync) {
        int idx=FD_TO_IDX(fd); if (idx<0||idx>=wasapi_dev_count||!wasapi_devs[idx].open) return -1;
        
        wasapi_dev_t *w=&wasapi_devs[idx];
        u32 padding=0; w->client->lpVtbl->GetCurrentPadding(w->client,&padding);
        snd_pcm_uframes_t base = (w->buffer_frames>(u32)(w->period_frames*4)) ? w->buffer_frames-(u32)(w->period_frames*4) : 0;
        sync->status.hw_ptr=base; sync->control.appl_ptr=base+padding;
        return 0;
    }

    int pcm_prepare(FHandle fd) { int i = FD_TO_IDX(fd); if (i < 0 || i >= wasapi_dev_count) return -1; wasapi_dev_t *w = &wasapi_devs[i]; return w->client->lpVtbl->Stop(w->client),w->client->lpVtbl->Reset(w->client),w->client->lpVtbl->Start(w->client), 0; }
    int pcm_write(void *buf, int frames) {
        for (int i=0;i<wasapi_dev_count;i++) {
            wasapi_dev_t *w=&wasapi_devs[i]; if (!w->open) continue;
            u8* data = NULL; if (FAILED(w->render->lpVtbl->GetBuffer(w->render,(u32)frames,&data))) { pcm_prepare(IDX_TO_FD(i)); continue; }
            
            MemCpyFromBtoAForNBytes(data,buf,frames*w->channels*2); w->render->lpVtbl->ReleaseBuffer(w->render,(u32)frames,0);
        }
        return frames;
    }
#else
    int ioctl(int fd, u64 request, ...);
    #define _IOC(dir, type, nr, size) (((dir) << 30) | ((type) << 8) | ((nr) << 0) | ((size) << 16))
    #define _IO(type, nr) _IOC(0U, (type), (nr), 0)
    #define _IOR(type, nr, size) _IOC(2U, (type), (nr), sizeof(size))
    #define _IOW(type, nr, size) _IOC(1U, (type), (nr), sizeof(size))
    #define _IOWR(type, nr, size) _IOC(2U | 1U, (type), (nr), sizeof(size))
    typedef u64 snd_pcm_uframes_t; typedef i64 snd_pcm_sframes_t; struct snd_mask { u32 bits[8]; }; struct snd_interval { u32 min,max,openmin:1, openmax:1, integer:1, empty:1; };
    struct snd_pcm_hw_params { u32 flags; struct snd_mask masks[3]; struct snd_mask mres[5]; struct snd_interval intervals[12]; struct snd_interval ires[9]; u32 rmask,cmask,info,msbits,rate_num,rate_den; snd_pcm_uframes_t fifo_size; u8 reserved[64]; };
    struct snd_pcm_sw_params { int tstamp_mode; unsigned int period_step,sleep_min; snd_pcm_uframes_t avail_min,xfer_align,start_threshold,stop_threshold,silence_threshold,silence_size,boundary; u32 proto,tstamp_type; u8 reserved[56]; }; 
    struct snd_pcm_mmap_status { int state,pad1; snd_pcm_uframes_t hw_ptr; struct timespec tstamp; int suspended_state; struct timespec audio_tstamp; };
    struct snd_pcm_mmap_control { snd_pcm_uframes_t appl_ptr; snd_pcm_uframes_t avail_min; };
    struct snd_pcm_sync_ptr { u32 flags; union { struct snd_pcm_mmap_status  status; u8 reserved[64]; } s; union { struct snd_pcm_mmap_control control; u8 reserved[64]; } c; };
    struct snd_pcm_status { int state; struct timespec trigger_tstamp; struct timespec tstamp; snd_pcm_uframes_t appl_ptr,hw_ptr; snd_pcm_sframes_t delay; snd_pcm_uframes_t avail,avail_max,overrange; int suspended_state; u32 audio_tstamp_data; struct timespec audio_tstamp; struct timespec driver_tstamp; u32 audio_tstamp_accuracy; u8 reserved[20]; };
    typedef struct snd_pcm_mmap_status  pcm_status_t; typedef struct snd_pcm_mmap_control pcm_control_t; typedef struct snd_pcm_hw_params pcm_hw_params_t; typedef struct snd_pcm_sw_params pcm_sw_params_t;
    struct pcm_params { pcm_hw_params_t hw_params; pcm_sw_params_t sw_params; }; typedef struct pcm_params pcm_params_t;
    typedef enum pcm_param {PCM_ACCESS=0,PCM_FORMAT=1,PCM_RATE=11,PCM_CHANNELS=10,PCM_PERIOD_SIZE=13,PCM_BUFFER_SIZE=17,PCM_PERIODS=15,PCM_INTERRUPT=20,PCM_TSTAMP_TYPE=21,PCM_AVAIL_MIN=22,PCM_START_THRESHOLD=23,PCM_XRUN_THRESHOLD=24,PCM_SILENCE_THRESHOLD=25,PCM_SILENCE_SIZE=26} pcm_param_t;
    static inline struct snd_mask* get_mask_struct(struct snd_pcm_hw_params *p, u32 parameter) { return &p->masks[parameter - 0]; }
    static inline struct snd_interval* get_interval_struct(struct snd_pcm_hw_params *p, u32 parameter) { return &p->intervals[parameter - 8]; }
    static void hw_params_set_mask(struct snd_pcm_hw_params *p, int parameter, u32 value) { struct snd_mask *m = get_mask_struct(p,parameter); if (m->bits[((value) / 32)] & (1 << ((value) % 32))) {MemSetToVForNBytes(m, 0x00, sizeof(*m));} m->bits[((value) / 32)] |= (1 << ((value) % 32)); }
    static void hw_params_set_interval(struct snd_pcm_hw_params *p, int parameter, u32 min, u32 max) { struct snd_interval *i = get_interval_struct(p,parameter); i->openmin = i->openmax = 0; i->integer = 1; i->min = min; i->max = max; }
    static void hw_params_set(struct snd_pcm_hw_params *p, int parameter, u32 value) { if ((parameter >= 0 && parameter <= 2)) hw_params_set_mask(p,parameter,value); else if ((parameter >= 8 && parameter <= 19)) hw_params_set_interval(p,parameter,value,value); }
    static u32 hw_params_get_mask(struct snd_pcm_hw_params *p, int parameter, u32 value) { struct snd_mask *m=get_mask_struct(p,parameter); return m->bits[((value) / 32)] & (1 << ((value) % 32)); }
    static void hw_params_get_interval(struct snd_pcm_hw_params *p, int parameter, u32 *min, u32 *max) { struct snd_interval *i = get_interval_struct(p,parameter); *min = i->min + i->openmin; *max = i->max - i->openmax; }
    static u32 hw_params_get(struct snd_pcm_hw_params *p, int parameter, u32 value) { u32 r, t; return (parameter >= 0 && parameter <= 2) ? hw_params_get_mask(p,parameter,value) : ((parameter >= 8 && parameter <= 19) ? (hw_params_get_interval(p,parameter,&r,&t),r) : 0); }
    static void hw_params_fill(struct snd_pcm_hw_params *p) { MemSetToVForNBytes(p,0,sizeof(*p)); MemSetToVForNBytes(p->masks,0xff,sizeof(p->masks)); p->rmask = p->info = 0xffffffffU; for (int i=0;i<=11;i++) { p->intervals[i].min = 0; p->intervals[i].max = 0xffffffffU; } }
    unsigned long pcm_gethw(pcm_params_t *p, pcm_param_t param, unsigned int val) { return hw_params_get(&p->hw_params,param,val); } 
    unsigned long pcm_getsw(pcm_params_t *p, pcm_param_t param) { pcm_sw_params_t *sw = &p->sw_params; return ((u64*)&sw->avail_min)[param - 22]; }
    int pcm_params_setup(int fd, pcm_params_t *p) {
        if (ioctl(fd,_IOWR('A',0x11,struct snd_pcm_hw_params),&p->hw_params) == -1) return -1;
        if (!pcm_getsw(p,22)) ((u64*)&p->sw_params.avail_min)[0] = pcm_gethw(p,13,0);
        if (!pcm_getsw(p,24)) ((u64*)&p->sw_params.avail_min)[2] = pcm_gethw(p,17,0);
        if (ioctl(fd,_IOW('A',0x03,int),&p->sw_params.tstamp_type) == -1) return -1; // Support ancient kernels
        if (ioctl(fd,_IOWR('A',0x13,struct snd_pcm_sw_params),&p->sw_params) == -1) return -1;
        return ioctl(fd,_IO('A',0x40));
    }

    int pcm_open(int card, int device, int flags) { char path[4096]; StringFormat(path,sizeof(path),"/dev/snd/pcmC%uD%u%c",card,device,(flags & 1) == 0 ? 'c' : 'p'); return OS_Open(path,00000002 | (flags & (1 << 1) ? 00004000 : 0),0); }
#endif
#define DRMP3_HDR_IS_MONO(h)             (((h[3]) & 0xC0) == 0xC0)
#define DRMP3_HDR_IS_MS_STEREO(h)        (((h[3]) & 0xE0) == 0x60)
#define DRMP3_HDR_IS_FREE_FORMAT(h)      (((h[2]) & 0xF0) == 0)
#define DRMP3_HDR_IS_CRC(h)              (!((h[1]) & 1))
#define DRMP3_HDR_TEST_PADDING(h)        ((h[2]) & 0x2)
#define DRMP3_HDR_TEST_MPEG1(h)          ((h[1]) & 0x8)
#define DRMP3_HDR_TEST_NOT_MPEG25(h)     ((h[1]) & 0x10)
#define DRMP3_HDR_TEST_I_STEREO(h)       ((h[3]) & 0x10)
#define DRMP3_HDR_TEST_MS_STEREO(h)      ((h[3]) & 0x20)
#define DRMP3_HDR_GET_STEREO_MODE(h)     (((h[3]) >> 6) & 3)
#define DRMP3_HDR_GET_STEREO_MODE_EXT(h) (((h[3]) >> 4) & 3)
#define DRMP3_HDR_GET_LAYER(h)           (((h[1]) >> 1) & 3)
#define DRMP3_HDR_GET_BITRATE(h)         ((h[2]) >> 4)
#define DRMP3_HDR_GET_SAMPLE_RATE(h)     (((h[2]) >> 2) & 3)
#define DRMP3_HDR_GET_MY_SAMPLE_RATE(h)  (DRMP3_HDR_GET_SAMPLE_RATE(h) + (((h[1]>>3)&1)+((h[1]>>4)&1))*3)
#define DRMP3_HDR_IS_FRAME_576(h)        ((h[1] & 14) == 2)
#define DRMP3_HDR_IS_LAYER_1(h)          ((h[1] & 6) == 6)
#define DRMP3_OFFSET_PTR(p,offset) ((void*)((u8*)(p)+(offset)))
typedef struct { int frame_bytes,channels,sample_rate,layer,bitrate_kbps; } drmp3dec_frame_info;
typedef struct { const u8 *buf; int pos,limit; } drmp3_bs;
typedef struct { const u8 *sfbtab; u16 part_23_length,big_values,scalefac_compress; u8 global_gain,block_type,mixed_block_flag,n_long_sfb,n_short_sfb,table_select[3],region_count[3],subblock_gain[3],preflag,scalefac_scale,count1_table,scfsi; } drmp3_L3_gr_info;
typedef struct { drmp3_bs bs; u8 maindata[511 + 2304]; drmp3_L3_gr_info gr_info[4]; float grbuf[2][576],scf[40],syn[18+15][2*32]; u8 ist_pos[2][39]; } drmp3dec_scratch;
typedef struct { float mdct_overlap[2][9*32], qmf_state[15*2*32]; int reserv,free_format_bytes; u8 header[4],reserv_buf[511]; drmp3dec_scratch scratch; } drmp3dec;
typedef struct { drmp3dec decoder; u32 channels,sampleRate,mp3FChan,mp3FrameSampleRate,pcmFConsInMP3F,pcmFRemInMP3F,delayInPCMFrames,paddingInPCMFrames; void *pUserData; u8 pcmFrames[sizeof(float) * (1152 * 2)]; u64 currentPCMFrame,streamCursor,streamLength,streamStartOffset,totalPCMFrameCount; bool atEnd; size_t dataSize,dataCapacity,dataConsumed; u8 *pData; } drmp3;
static u32 drmp3_bs_get_bits(drmp3_bs *bs, int n) { u32 next,cache=0, s=bs->pos&7; int shl=n+s; const u8 *p=bs->buf+(bs->pos>>3); if ((bs->pos+=n)>bs->limit) {return 0;} next=*p++&(255>>s); while ((shl-=8)>0) { cache|=next<<shl; next=*p++; } return cache|(next>>-shl); }
static int drmp3_hdr_valid(const u8 *h) { return h[0]==0xff && ((h[1]&0xF0)==0xf0||(h[1]&0xFE)==0xe2) && (DRMP3_HDR_GET_LAYER(h)!=0) && (DRMP3_HDR_GET_BITRATE(h)!=15) && (DRMP3_HDR_GET_SAMPLE_RATE(h)!=3); }
static int drmp3_hdr_compare(const u8 *h1, const u8 *h2) { return drmp3_hdr_valid(h2) && ((h1[1]^h2[1])&0xFE)==0 && ((h1[2]^h2[2])&0x0C)==0 && !(DRMP3_HDR_IS_FREE_FORMAT(h1)^DRMP3_HDR_IS_FREE_FORMAT(h2)); }
static u8 g_halfrate[2][3][15]={ {{0,4,8,12,16,20,24,28,32,40,48,56,64,72,80},{0,4,8,12,16,20,24,28,32,40,48,56,64,72,80},{0,16,24,28,32,40,48,56,64,72,80,88,96,112,128}},{{0,16,20,24,28,32,40,48,56,64,80,96,112,128,160},{0,16,24,28,32,40,48,56,64,80,96,112,128,160,192},{0,16,32,48,64,80,96,112,128,144,160,176,192,208,224}} };
static unsigned drmp3_hdr_bitrate_kbps(const u8 *h) { return 2*g_halfrate[!!DRMP3_HDR_TEST_MPEG1(h)][(((h[1]) >> 1) & 3)-1/*layer*/][((h[2]) >> 4)/*bitrate*/]; }
static unsigned drmp3_hdr_sample_rate_hz(const u8 *h) { static const unsigned g_hz[3]={44100,48000,32000}; return g_hz[DRMP3_HDR_GET_SAMPLE_RATE(h)]>>(int)!DRMP3_HDR_TEST_MPEG1(h)>>(int)!DRMP3_HDR_TEST_NOT_MPEG25(h); }
static unsigned drmp3_hdr_frame_samples(const u8 *h) { return ((h[1]&6) == 6) ? 384 : (1152>>(int)DRMP3_HDR_IS_FRAME_576(h)); }
static int drmp3_hdr_frame_bytes(const u8 *h, int free_format_size) { int fb=drmp3_hdr_frame_samples(h)*drmp3_hdr_bitrate_kbps(h)*125/drmp3_hdr_sample_rate_hz(h); if (DRMP3_HDR_IS_LAYER_1(h)) {fb&=~3;} return fb?fb:free_format_size; }
static int drmp3_hdr_padding(const u8 *h) { return DRMP3_HDR_TEST_PADDING(h)?(DRMP3_HDR_IS_LAYER_1(h)?4:1):0; }
static u8 g_scf_long[8][23]={{0},{12,12,12,12,12,12,16,20,24,28,32,40,48,56,64,76,90,2,2,2,2,2,0},{0},{6,6,6,6,6,6,8,10,12,14,16,18,22,26,32,38,46,54,62,70,76,36,0},{0},{4,4,4,4,4,4,6,6,8,8,10,12,16,20,24,28,34,42,50,54,76,158,0},{4,4,4,4,4,4,6,6,6,8,10,12,16,18,22,28,34,40,46,54,54,192,0},{4,4,4,4,4,4,6,6,8,10,12,16,20,24,30,38,46,56,68,84,102,26,0}};
static u8 g_scf_short[8][40]={{4,4,4,4,4,4,4,4,4,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,24,24,24,30,30,30,40,40,40,18,18,18,0},{8,8,8,8,8,8,8,8,8,12,12,12,16,16,16,20,20,20,24,24,24,28,28,28,36,36,36,2,2,2,2,2,2,2,2,2,26,26,26,0},{4,4,4,4,4,4,4,4,4,6,6,6,6,6,6,8,8,8,10,10,10,14,14,14,18,18,18,26,26,26,32,32,32,42,42,42,18,18,18,0 },{4,4,4,4,4,4,4,4,4,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,24,24,24,32,32,32,44,44,44,12,12,12,0 }, { 4,4,4,4,4,4,4,4,4,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,24,24,24,30,30,30,40,40,40,18,18,18,0 },{4,4,4,4,4,4,4,4,4,4,4,4,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,22,22,22,30,30,30,56,56,56,0},{4,4,4,4,4,4,4,4,4,4,4,4,6,6,6,6,6,6,10,10,10,12,12,12,14,14,14,16,16,16,20,20,20,26,26,26,66,66,66,0},{4,4,4,4,4,4,4,4,4,4,4,4,6,6,6,8,8,8,12,12,12,16,16,16,20,20,20,26,26,26,34,34,34,42,42,42,12,12,12,0}};
static u8 g_scf_mixed[8][40]={{6,6,6,6,6,6,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,24,24,24,30,30,30,40,40,40,18,18,18,0},{12,12,12,4,4,4,8,8,8,12,12,12,16,16,16,20,20,20,24,24,24,28,28,28,36,36,36,2,2,2,2,2,2,2,2,2,26,26,26,0},{6,6,6,6,6,6,6,6,6,6,6,6,8,8,8,10,10,10,14,14,14,18,18,18,26,26,26,32,32,32,42,42,42,18,18,18,0},{6,6,6,6,6,6,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,24,24,24,32,32,32,44,44,44,12,12,12,0},{6,6,6,6,6,6,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,24,24,24,30,30,30,40,40,40,18,18,18,0},{4,4,4,4,4,4,6,6,4,4,4,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,22,22,22,30,30,30,56,56,56,0},{4,4,4,4,4,4,6,6,4,4,4,6,6,6,6,6,6,10,10,10,12,12,12,14,14,14,16,16,16,20,20,20,26,26,26,66,66,66,0},{4,4,4,4,4,4,6,6,4,4,4,6,6,6,8,8,8,12,12,12,16,16,16,20,20,20,26,26,26,34,34,34,42,42,42,12,12,12,0}};
static const u8 g_sfc_long_024[23] = { 6,6,6,6,6,6,8,10,12,14,16,20,24,28,32,38,46,52,60,68,58,54,0 };
void InitSCFTables() { for (int i=0;i<23;++i) g_scf_long[0][i] = g_scf_long[1][i] = g_scf_long[2][i] = g_sfc_long_024[i]; }
static int drmp3_L3_read_side_info(drmp3_bs *bs, drmp3_L3_gr_info *gr, const u8 *hdr) {
    unsigned tables,scfsi=0; int main_data_begin,part_23_sum = 0, gr_count=DRMP3_HDR_IS_MONO(hdr) ? 1 : 2, sr_idx=DRMP3_HDR_GET_MY_SAMPLE_RATE(hdr); sr_idx-=(sr_idx!=0);
    if (DRMP3_HDR_TEST_MPEG1(hdr)) { gr_count*=2; main_data_begin=drmp3_bs_get_bits(bs,9); scfsi=drmp3_bs_get_bits(bs,7+gr_count); }
    else main_data_begin = drmp3_bs_get_bits(bs,8+gr_count)>>gr_count;
    do {
        if (DRMP3_HDR_IS_MONO(hdr)) scfsi<<=4;
        gr->part_23_length=(u16)drmp3_bs_get_bits(bs,12); part_23_sum+=gr->part_23_length; gr->big_values=(u16)drmp3_bs_get_bits(bs,9); if (gr->big_values>288) return -1;
        
        gr->global_gain=(u8)drmp3_bs_get_bits(bs,8); gr->scalefac_compress=(u16)drmp3_bs_get_bits(bs,DRMP3_HDR_TEST_MPEG1(hdr)?4:9); gr->sfbtab=g_scf_long[sr_idx]; gr->n_long_sfb=22; gr->n_short_sfb=0;
        if (drmp3_bs_get_bits(bs,1)) {
            gr->block_type = (u8)drmp3_bs_get_bits(bs,2); if (!gr->block_type) return -1;
            
            gr->mixed_block_flag = (u8)drmp3_bs_get_bits(bs,1); gr->region_count[0] = 7; gr->region_count[1] = 255;
            if (gr->block_type==2) {
                scfsi&=0x0F0F;
                if (!gr->mixed_block_flag) { gr->region_count[0] = 8; gr->sfbtab = g_scf_short[sr_idx]; gr->n_long_sfb = 0; gr->n_short_sfb = 39; }
                else                       { gr->sfbtab = g_scf_mixed[sr_idx]; gr->n_long_sfb=DRMP3_HDR_TEST_MPEG1(hdr) ? 8 : 6; gr->n_short_sfb = 30; }
            }
            tables=drmp3_bs_get_bits(bs,10)<<5;
            gr->subblock_gain[0]=(u8)drmp3_bs_get_bits(bs,3); gr->subblock_gain[1]=(u8)drmp3_bs_get_bits(bs,3); gr->subblock_gain[2]=(u8)drmp3_bs_get_bits(bs,3);
        } else {
            gr->block_type=0; gr->mixed_block_flag=0;
            tables=drmp3_bs_get_bits(bs,15);
            gr->region_count[0]=(u8)drmp3_bs_get_bits(bs,4); gr->region_count[1]=(u8)drmp3_bs_get_bits(bs,3); gr->region_count[2]=255;
        }
        gr->table_select[0]=(u8)(tables>>10); gr->table_select[1]=(u8)((tables>>5)&31); gr->table_select[2]=(u8)((tables)&31);
        gr->preflag=(u8)(DRMP3_HDR_TEST_MPEG1(hdr) ? drmp3_bs_get_bits(bs,1) : (gr->scalefac_compress>=500));
        gr->scalefac_scale=(u8)drmp3_bs_get_bits(bs,1); gr->count1_table=(u8)drmp3_bs_get_bits(bs,1); gr->scfsi=(u8)((scfsi>>12)&15);
        scfsi <<= 4; gr++;
    } while(--gr_count);
    if (part_23_sum+bs->pos > bs->limit+main_data_begin*8) return -1;
    return main_data_begin;
}

static void drmp3_L3_read_scalefactors(u8 *scf, u8 *ist_pos, const u8 *scf_size, const u8 *scf_count, drmp3_bs *bs, int scfsi) {
    for (int i=0; i<4&&scf_count[i]; i++,scfsi*=2) {
        int cnt = scf_count[i];
        if (scfsi & 8) MemCpyFromBtoAForNBytes(scf,ist_pos,cnt);
        else {
            int bits=scf_size[i];
            if (!bits) { MemSetToVForNBytes(scf,0,cnt); MemSetToVForNBytes(ist_pos,0,cnt); }
            else { int max_scf=(scfsi<0)?((1<<bits)-1):-1; for (int k=0;k<cnt;k++) {int s=drmp3_bs_get_bits(bs,bits); ist_pos[k]=(u8)(s==max_scf?-1:s); scf[k]=(u8)s;} }
        }
        
        ist_pos+=cnt; scf+=cnt;
    }
    
    scf[0]=scf[1]=scf[2]=0;
}

static float drmp3_L3_ldexp_q2(float y, int exp_q2) { static const float g_expfrac[4]={9.31322575e-10f,7.83145814e-10f,6.58544508e-10f,5.53767716e-10f}; int e; do { e=vmin(30*4,exp_q2); y*=g_expfrac[e&3]*(1<<30>>(e>>2)); } while ((exp_q2-=e)>0); return y; }
#define DRMP3_MAX_SCFI (((255+-1*4-210)+3)&~3)
static void drmp3_L3_decode_scalefactors(const u8 *hdr, u8 *ist_pos, drmp3_bs *bs, const drmp3_L3_gr_info *gr, float *scf, int ch) {
    static const u8 g_scf_partitions[3][28]={{6,5,5,5,6,5,5,5,6,5,7,3,11,10,0,0,7,7,7,0,6,6,6,3,8,8,5,0}, {8,9,6,12,6,9,9,9,6,9,12,6,15,18,0,0,6,15,12,0,6,12,9,6,6,18,9,0}, {9,9,6,12,9,9,9,9,9,9,12,6,18,18,0,0,12,12,12,0,12,9,9,6,15,12,9,0}};
    const u8 *scf_partition=g_scf_partitions[!!gr->n_short_sfb+!gr->n_long_sfb];
    u8 scf_size[4],iscf[40]; int i,scf_shift=gr->scalefac_scale+1,gain_exp,scfsi=gr->scfsi; float gain;
    if (DRMP3_HDR_TEST_MPEG1(hdr)) { static const u8 g_scfc_decode[16]={0,1,2,3,12,5,6,7,9,10,11,13,14,15,18,19}; int part=g_scfc_decode[gr->scalefac_compress]; scf_size[1]=scf_size[0]=(u8)(part>>2); scf_size[3]=scf_size[2]=(u8)(part&3); }
    else {
        static const u8 g_mod[6*4]={5,5,4,4,5,5,4,1,4,3,1,1,5,6,6,1,4,4,4,1,4,3,1,1};
        int k,modprod,sfc,ist=DRMP3_HDR_TEST_I_STEREO(hdr)&&ch;
        sfc=gr->scalefac_compress>>ist;
        for (k=ist*3*4; sfc>=0; sfc-=modprod,k+=4) { for (modprod=1,i=3;i>=0;i--) {scf_size[i]=(u8)(sfc/modprod%g_mod[k+i]); modprod*=g_mod[k+i];} }
        scf_partition+=k; scfsi=-16;
    }
    
    drmp3_L3_read_scalefactors(iscf,ist_pos,scf_size,scf_partition,bs,scfsi);
    if (gr->n_short_sfb) {
        int sh=3-scf_shift;
        for (i=0;i<gr->n_short_sfb;i+=3) {
            iscf[gr->n_long_sfb+i+0]=(u8)(iscf[gr->n_long_sfb+i+0]+(gr->subblock_gain[0]<<sh));
            iscf[gr->n_long_sfb+i+1]=(u8)(iscf[gr->n_long_sfb+i+1]+(gr->subblock_gain[1]<<sh));
            iscf[gr->n_long_sfb+i+2]=(u8)(iscf[gr->n_long_sfb+i+2]+(gr->subblock_gain[2]<<sh));
        }
    } else if (gr->preflag) { static const u8 g_preamp[10]={1,1,1,1,2,2,3,3,3,2}; for (i=0;i<10;i++) {iscf[11+i]=(u8)(iscf[11+i]+g_preamp[i]);} }
    
    gain_exp=gr->global_gain+-1*4-210-(DRMP3_HDR_IS_MS_STEREO(hdr)?2:0);
    gain=drmp3_L3_ldexp_q2(1<<(DRMP3_MAX_SCFI/4),DRMP3_MAX_SCFI-gain_exp);
    for (i=0;i<(int)(gr->n_long_sfb+gr->n_short_sfb);i++) scf[i]=drmp3_L3_ldexp_q2(gain,iscf[i]<<scf_shift);
}

static const float g_drmp3_pow43[129+16]={ 0,-1,-2.519842f,-4.326749f,-6.349604f,-8.549880f,-10.902724f,-13.390518f,-16.000000f,-18.720754f,-21.544347f,-24.463781f,-27.473142f,-30.567351f,-33.741992f,-36.993181f,
                                           0,1,2.519842f,4.326749f,6.349604f,8.549880f,10.902724f,13.390518f,16.000000f,18.720754f,21.544347f,24.463781f,27.473142f,30.567351f,33.741992f,36.993181f,40.317474f,43.711787f,47.173345f,50.699631f,54.288352f,57.937408f,61.644865f,65.408941f,69.227979f,73.100443f,77.024898f,81.000000f,85.024491f,89.097188f,93.216975f,97.382800f,101.593667f,105.848633f,110.146801f,114.487321f,118.869381f,123.292209f,127.755065f,132.257246f,136.798076f,141.376907f,145.993119f,150.646117f,155.335327f,160.060199f,164.820202f,169.614826f,174.443577f,179.305980f,184.201575f,189.129918f,194.090580f,199.083145f,204.107210f,209.162385f,214.248292f,219.364564f,224.510845f,229.686789f,234.892058f,240.126328f,245.389280f,250.680604f,256.000000f,261.347174f,266.721841f,272.123723f,277.552547f,283.008049f,288.489971f,293.998060f,299.532071f,305.091761f,310.676898f,316.287249f,321.922592f,327.582707f,333.267377f,338.976394f,344.709550f,350.466646f,356.247482f,362.051866f,367.879608f,373.730522f,379.604427f,385.501143f,391.420496f,397.362314f,403.326427f,409.312672f,415.320884f,421.350905f,427.402579f,433.475750f,439.570269f,445.685987f,451.822757f,457.980436f,464.158883f,470.357960f,476.577530f,482.817459f,489.077615f,495.357868f,501.658090f,507.978156f,514.317941f,520.677324f,527.056184f,533.454404f,539.871867f,546.308458f,552.764065f,559.238575f,565.731879f,572.243870f,578.774440f,585.323483f,591.890898f,598.476581f,605.080431f,611.702349f,618.342238f,625.000000f,631.675540f,638.368763f,645.079578f };
static float drmp3_L3_pow_43(int x) {
    if (x<129) return g_drmp3_pow43[16+x];
    
    int mult=256; if (x<1024) { mult=16; x<<=3; }
    int sign=2*x&64;
    float frac=(float)((x&63)-sign)/((x&~63)+sign);
    return g_drmp3_pow43[16+((x+sign)>>6)]*(1.f+frac*((4.f/3)+frac*(2.f/9)))*mult;
}

static void drmp3_L3_huffman(float *dst, drmp3_bs *bs, const drmp3_L3_gr_info *gr_info, const float *scf, int layer3gr_limit) {
    static const i16 tabs[]={ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,785,785,785,785,784,784,784,784,513,513,513,513,513,513,513,513,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,-255,1313,1298,1282,785,785,785,785,784,784,784,784,769,769,769,769,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,290,288,-255,1313,1298,1282,769,769,769,769,529,529,529,529,529,529,529,529,528,528,528,528,528,528,528,528,512,512,512,512,512,512,512,512,290,288,-253,-318,-351,-367,785,785,785,785,784,784,784,784,769,769,769,769,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,819,818,547,547,275,275,275,275,561,560,515,546,289,274,288,258,
        -254,-287,1329,1299,1314,1312,1057,1057,1042,1042,1026,1026,784,784,784,784,529,529,529,529,529,529,529,529,769,769,769,769,768,768,768,768,563,560,306,306,291,259,-252,-413,-477,-542,1298,-575,1041,1041,784,784,784,784,769,769,769,769,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,-383,-399,1107,1092,1106,1061,849,849,789,789,1104,1091,773,773,1076,1075,341,340,325,309,834,804,577,577,532,532,516,516,832,818,803,816,561,561,531,531,515,546,289,289,288,258,-252,-429,-493,-559,1057,1057,1042,1042,529,529,529,529,529,529,529,529,784,784,784,784,769,769,769,769,512,512,512,512,512,512,512,512,-382,1077,-415,1106,1061,1104,849,849,789,789,1091,1076,1029,1075,834,834,597,581,340,340,339,324,804,833,532,532,832,772,818,803,817,787,816,771,290,290,290,290,288,258,
        -253,-349,-414,-447,-463,1329,1299,-479,1314,1312,1057,1057,1042,1042,1026,1026,785,785,785,785,784,784,784,784,769,769,769,769,768,768,768,768,-319,851,821,-335,836,850,805,849,341,340,325,336,533,533,579,579,564,564,773,832,578,548,563,516,321,276,306,291,304,259,-251,-572,-733,-830,-863,-879,1041,1041,784,784,784,784,769,769,769,769,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,-511,-527,-543,1396,1351,1381,1366,1395,1335,1380,-559,1334,1138,1138,1063,1063,1350,1392,1031,1031,1062,1062,1364,1363,1120,1120,1333,1348,881,881,881,881,375,374,359,373,343,358,341,325,791,791,1123,1122,-703,1105,1045,-719,865,865,790,790,774,774,1104,1029,338,293,323,308,-799,-815,833,788,772,818,803,816,322,292,307,320,561,531,515,546,289,274,288,258,
        -251,-525,-605,-685,-765,-831,-846,1298,1057,1057,1312,1282,785,785,785,785,784,784,784,784,769,769,769,769,512,512,512,512,512,512,512,512,1399,1398,1383,1367,1382,1396,1351,-511,1381,1366,1139,1139,1079,1079,1124,1124,1364,1349,1363,1333,882,882,882,882,807,807,807,807,1094,1094,1136,1136,373,341,535,535,881,775,867,822,774,-591,324,338,-671,849,550,550,866,864,609,609,293,336,534,534,789,835,773,-751,834,804,308,307,833,788,832,772,562,562,547,547,305,275,560,515,290,290,-252,-397,-477,-557,-622,-653,-719,-735,-750,1329,1299,1314,1057,1057,1042,1042,1312,1282,1024,1024,785,785,785,785,784,784,784,784,769,769,769,769,-383,1127,1141,1111,1126,1140,1095,1110,869,869,883,883,1079,1109,882,882,375,374,807,868,838,881,791,-463,867,822,368,263,852,837,836,-543,610,610,550,550,352,336,534,534,865,774,851,821,850,805,593,533,579,564,773,832,578,578,548,548,577,577,307,276,306,291,516,560,259,259,
        -250,-2107,-2507,-2764,-2909,-2974,-3007,-3023,1041,1041,1040,1040,769,769,769,769,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,-767,-1052,-1213,-1277,-1358,-1405,-1469,-1535,-1550,-1582,-1614,-1647,-1662,-1694,-1726,-1759,-1774,-1807,-1822,-1854,-1886,1565,-1919,-1935,-1951,-1967,1731,1730,1580,1717,-1983,1729,1564,-1999,1548,-2015,-2031,1715,1595,-2047,1714,-2063,1610,-2079,1609,-2095,1323,1323,1457,1457,1307,1307,1712,1547,1641,1700,1699,1594,1685,1625,1442,1442,1322,1322,-780,-973,-910,1279,1278,1277,1262,1276,1261,1275,1215,1260,1229,-959,974,974,989,989,-943,735,478,478,495,463,506,414,-1039,1003,958,1017,927,942,987,957,431,476,1272,1167,1228,-1183,1256,-1199,895,895,941,941,1242,1227,1212,1135,1014,1014,490,489,503,487,910,1013,985,925,863,894,970,955,1012,847,-1343,831,755,755,984,909,428,366,754,559,-1391,752,486,457,924,997,698,698,983,893,740,740,908,877,739,739,667,667,953,938,497,287,271,271,683,606,590,712,726,574,302,302,738,736,481,286,526,725,605,711,636,724,696,651,589,681,666,710,364,467,573,695,466,466,301,465,379,379,709,604,665,679,316,316,634,633,436,436,464,269,424,394,452,332,438,363,347,408,393,448,331,422,362,407,392,421,346,406,391,376,375,359,1441,1306,-2367,1290,-2383,1337,-2399,-2415,1426,1321,-2431,1411,1336,-2447,-2463,-2479,1169,1169,1049,1049,1424,1289,1412,1352,1319,-2495,1154,1154,1064,1064,1153,1153,416,390,360,404,403,389,344,374,373,343,358,372,327,357,342,311,356,326,1395,1394,1137,1137,1047,1047,1365,1392,1287,1379,1334,1364,1349,1378,1318,1363,792,792,792,792,1152,1152,1032,1032,1121,1121,1046,1046,1120,1120,1030,1030,-2895,1106,1061,1104,849,849,789,789,1091,1076,1029,1090,1060,1075,833,833,309,324,532,532,832,772,818,803,561,561,531,560,515,546,289,274,288,258,
        -250,-1179,-1579,-1836,-1996,-2124,-2253,-2333,-2413,-2477,-2542,-2574,-2607,-2622,-2655,1314,1313,1298,1312,1282,785,785,785,785,1040,1040,1025,1025,768,768,768,768,-766,-798,-830,-862,-895,-911,-927,-943,-959,-975,-991,-1007,-1023,-1039,-1055,-1070,1724,1647,-1103,-1119,1631,1767,1662,1738,1708,1723,-1135,1780,1615,1779,1599,1677,1646,1778,1583,-1151,1777,1567,1737,1692,1765,1722,1707,1630,1751,1661,1764,1614,1736,1676,1763,1750,1645,1598,1721,1691,1762,1706,1582,1761,1566,-1167,1749,1629,767,766,751,765,494,494,735,764,719,749,734,763,447,447,748,718,477,506,431,491,446,476,461,505,415,430,475,445,504,399,460,489,414,503,383,474,429,459,502,502,746,752,488,398,501,473,413,472,486,271,480,270,-1439,-1455,1357,-1471,-1487,-1503,1341,1325,-1519,1489,1463,1403,1309,-1535,1372,1448,1418,1476,1356,1462,1387,-1551,1475,1340,1447,1402,1386,-1567,1068,1068,1474,1461,455,380,468,440,395,425,410,454,364,467,466,464,453,269,409,448,268,432,1371,1473,1432,1417,1308,1460,1355,1446,1459,1431,1083,1083,1401,1416,1458,1445,1067,1067,1370,1457,1051,1051,1291,1430,1385,1444,1354,1415,1400,1443,1082,1082,1173,1113,1186,1066,1185,1050,-1967,1158,1128,1172,1097,1171,1081,-1983,1157,1112,416,266,375,400,1170,1142,1127,1065,793,793,1169,1033,1156,1096,1141,1111,1155,1080,1126,1140,898,898,808,808,897,897,792,792,1095,1152,1032,1125,1110,1139,1079,1124,882,807,838,881,853,791,-2319,867,368,263,822,852,837,866,806,865,-2399,851,352,262,534,534,821,836,594,594,549,549,593,593,533,533,848,773,579,579,564,578,548,563,276,276,577,576,306,291,516,560,305,305,275,259,
        -251,-892,-2058,-2620,-2828,-2957,-3023,-3039,1041,1041,1040,1040,769,769,769,769,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,-511,-527,-543,-559,1530,-575,-591,1528,1527,1407,1526,1391,1023,1023,1023,1023,1525,1375,1268,1268,1103,1103,1087,1087,1039,1039,1523,-604,815,815,815,815,510,495,509,479,508,463,507,447,431,505,415,399,-734,-782,1262,-815,1259,1244,-831,1258,1228,-847,-863,1196,-879,1253,987,987,748,-767,493,493,462,477,414,414,686,669,478,446,461,445,474,429,487,458,412,471,1266,1264,1009,1009,799,799,-1019,-1276,-1452,-1581,-1677,-1757,-1821,-1886,-1933,-1997,1257,1257,1483,1468,1512,1422,1497,1406,1467,1496,1421,1510,1134,1134,1225,1225,1466,1451,1374,1405,1252,1252,1358,1480,1164,1164,1251,1251,1238,1238,1389,1465,-1407,1054,1101,-1423,1207,-1439,830,830,1248,1038,1237,1117,1223,1148,1236,1208,411,426,395,410,379,269,1193,1222,1132,1235,1221,1116,976,976,1192,1162,1177,1220,1131,1191,963,963,-1647,961,780,-1663,558,558,994,993,437,408,393,407,829,978,813,797,947,-1743,721,721,377,392,844,950,828,890,706,706,812,859,796,960,948,843,934,874,571,571,-1919,690,555,689,421,346,539,539,944,779,918,873,932,842,903,888,570,570,931,917,674,674,-2575,1562,-2591,1609,-2607,1654,1322,1322,1441,1441,1696,1546,1683,1593,1669,1624,1426,1426,1321,1321,1639,1680,1425,1425,1305,1305,1545,1668,1608,1623,1667,1592,1638,1666,1320,1320,1652,1607,1409,1409,1304,1304,1288,1288,1664,1637,1395,1395,1335,1335,1622,1636,1394,1394,1319,1319,1606,1621,1392,1392,1137,1137,1137,1137,345,390,360,375,404,373,1047,-2751,-2767,-2783,1062,1121,1046,-2799,1077,-2815,1106,1061,789,789,1105,1104,263,355,310,340,325,354,352,262,339,324,1091,1076,1029,1090,1060,1075,833,833,788,788,1088,1028,818,818,803,803,561,561,531,531,816,771,546,546,289,274,288,258,
        -253,-317,-381,-446,-478,-509,1279,1279,-811,-1179,-1451,-1756,-1900,-2028,-2189,-2253,-2333,-2414,-2445,-2511,-2526,1313,1298,-2559,1041,1041,1040,1040,1025,1025,1024,1024,1022,1007,1021,991,1020,975,1019,959,687,687,1018,1017,671,671,655,655,1016,1015,639,639,758,758,623,623,757,607,756,591,755,575,754,559,543,543,1009,783,-575,-621,-685,-749,496,-590,750,749,734,748,974,989,1003,958,988,973,1002,942,987,957,972,1001,926,986,941,971,956,1000,910,985,925,999,894,970,-1071,-1087,-1102,1390,-1135,1436,1509,1451,1374,-1151,1405,1358,1480,1420,-1167,1507,1494,1389,1342,1465,1435,1450,1326,1505,1310,1493,1373,1479,1404,1492,1464,1419,428,443,472,397,736,526,464,464,486,457,442,471,484,482,1357,1449,1434,1478,1388,1491,1341,1490,1325,1489,1463,1403,1309,1477,1372,1448,1418,1433,1476,1356,1462,1387,-1439,1475,1340,1447,1402,1474,1324,1461,1371,1473,269,448,1432,1417,1308,1460,-1711,1459,-1727,1441,1099,1099,1446,1386,1431,1401,-1743,1289,1083,1083,1160,1160,1458,1445,1067,1067,1370,1457,1307,1430,1129,1129,1098,1098,268,432,267,416,266,400,-1887,1144,1187,1082,1173,1113,1186,1066,1050,1158,1128,1143,1172,1097,1171,1081,420,391,1157,1112,1170,1142,1127,1065,1169,1049,1156,1096,1141,1111,1155,1080,1126,1154,1064,1153,1140,1095,1048,-2159,1125,1110,1137,-2175,823,823,1139,1138,807,807,384,264,368,263,868,838,853,791,867,822,852,837,866,806,865,790,-2319,851,821,836,352,262,850,805,849,-2399,533,533,835,820,336,261,578,548,563,577,532,532,832,772,562,562,547,547,305,275,560,515,290,290,288,258
    };
static const u8 tab32[]={130,162,193,209,44,28,76,140,9,9,9,9,9,9,9,9,190,254,222,238,126,94,157,157,109,61,173,205}; static const u8 tab33[]={252,236,220,204,188,172,156,140,124,108,92,76,60,44,28,12};
static const i16 tabindex[2*16]={0,32,64,98,0,132,180,218,292,364,426,538,648,746,0,1126,1460,1460,1460,1460,1460,1460,1460,1460,1842,1842,1842,1842,1842,1842,1842,1842}; static const u8 g_linbits[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,3,4,6,8,10,13,4,5,6,7,8,9,11,13};
#define DRMP3_FLUSH_BITS(n) { bs_cache<<=(n); bs_sh+=(n); }
#define DRMP3_CHECK_BITS    while(bs_sh>=0){bs_cache|=(u32)*bs_next_ptr++<<bs_sh;bs_sh-=8;}
    float one=0.0f;
    int ireg=0,big_val_cnt=gr_info->big_values;
    const u8 *sfb=gr_info->sfbtab;
    const u8 *bs_next_ptr=bs->buf+bs->pos/8;
    u32 bs_cache=(((bs_next_ptr[0]*256u+bs_next_ptr[1])*256u+bs_next_ptr[2])*256u+bs_next_ptr[3])<<(bs->pos&7);
    int pairs_to_decode,np,bs_sh=(bs->pos&7)-8;
    bs_next_ptr+=4;
    while (big_val_cnt>0) {
        int tab_num=gr_info->table_select[ireg], sfb_cnt=gr_info->region_count[ireg++];
        const i16 *codebook=tabs+tabindex[tab_num]; int linbits=g_linbits[tab_num];
        if (linbits) {
            do {
                np=*sfb++/2; pairs_to_decode=vmin(big_val_cnt,np); one=*scf++;
                do {
                    int j,w=5,leaf=codebook[(bs_cache>>(32-w))];
                    while (leaf<0){DRMP3_FLUSH_BITS(w);w=leaf&7;leaf=codebook[(bs_cache>>(32-w))-(leaf>>3)];}
                    DRMP3_FLUSH_BITS(leaf>>8);
                    for (j=0;j<2;j++,dst++,leaf>>=4){
                        int lsb=leaf&0x0F;
                        if (lsb==15) { lsb += (bs_cache>>(32-(linbits))); DRMP3_FLUSH_BITS(linbits); DRMP3_CHECK_BITS; *dst= one * drmp3_L3_pow_43(lsb) * ((i32)bs_cache < 0 ? -1 : 1); }
                        else *dst=g_drmp3_pow43[16+lsb-16*(bs_cache>>31)]*one;
                        DRMP3_FLUSH_BITS(lsb?1:0);
                    }

                    DRMP3_CHECK_BITS;
                } while(--pairs_to_decode);
            } while((big_val_cnt-=np)>0&&--sfb_cnt>=0);
        } else {
            do {
                np=*sfb++/2; pairs_to_decode=vmin(big_val_cnt,np); one=*scf++;
                do {
                    int j,w=5,leaf=codebook[(bs_cache>>(32-w))];
                    while (leaf<0){DRMP3_FLUSH_BITS(w);w=leaf&7;leaf=codebook[(bs_cache>>(32-w))-(leaf>>3)];}
                    DRMP3_FLUSH_BITS(leaf>>8);
                    for (j=0;j<2;j++,dst++,leaf>>=4) { int lsb=leaf&0x0F; *dst=g_drmp3_pow43[16+lsb-16*(bs_cache>>31)]*one; DRMP3_FLUSH_BITS(lsb?1:0); }
                    DRMP3_CHECK_BITS;
                } while(--pairs_to_decode);
            } while((big_val_cnt-=np)>0&&--sfb_cnt>=0);
        }
    }

    for (np=1-big_val_cnt;;dst+=4) {
        const u8 *codebook_count1=(gr_info->count1_table)?tab33:tab32;
        int leaf=codebook_count1[(bs_cache>>28)];
        if (!(leaf&8)) leaf=codebook_count1[(leaf>>3)+(bs_cache<<4>>(32-(leaf&3)))];
        DRMP3_FLUSH_BITS(leaf&7);
        if (((bs_next_ptr-bs->buf)*8-24+bs_sh)>layer3gr_limit) break;
        if(!--np) { np=*sfb++/2; if(!np) {break;} one=*scf++; }; if(leaf&(128>>0)){dst[0]=((i32)bs_cache<0)?-one:one;DRMP3_FLUSH_BITS(1)} if(leaf&(128>>1)){dst[1]=((i32)bs_cache<0)?-one:one;DRMP3_FLUSH_BITS(1)}
        if(!--np) { np=*sfb++/2; if(!np) {break;} one=*scf++; }; if(leaf&(128>>2)){dst[2]=((i32)bs_cache<0)?-one:one;DRMP3_FLUSH_BITS(1)} if(leaf&(128>>3)){dst[3]=((i32)bs_cache<0)?-one:one;DRMP3_FLUSH_BITS(1)}
        DRMP3_CHECK_BITS;
    }

    bs->pos=layer3gr_limit;
}

static void drmp3_L3_midside_stereo(float *left, int n) { int i=0; float *right=left+576; for (; i<n; i++) { float a=left[i],b=right[i]; left[i]=a+b; right[i]=a-b; } }
static void drmp3_L3_intensity_stereo_band(float *left, int n, float kl, float kr) { int i; for(i=0;i<n;i++){left[i+576]=left[i]*kr;left[i]=left[i]*kl;} }
static void drmp3_L3_stereo_top_band(const float *right, const u8 *sfb, int nbands, int max_band[3]) { int i,k; max_band[0]=max_band[1]=max_band[2]=-1; for (i=0;i<nbands;i++){for(k=0;k<sfb[i];k+=2){if(right[k]!=0||right[k+1]!=0){max_band[i%3]=i;break;}}right+=sfb[i];} }
static void drmp3_L3_stereo_process(float *left, const u8 *ist_pos, const u8 *sfb, const u8 *hdr, int max_band[3], int mpeg2_sh) {
    static const float g_pan[7*2]={0,1,0.21132487f,0.78867513f,0.36602540f,0.63397460f,0.5f,0.5f,0.63397460f,0.36602540f,0.78867513f,0.21132487f,1,0};
    unsigned i,max_pos=DRMP3_HDR_TEST_MPEG1(hdr)?7:64;
    for (i=0;sfb[i];i++){
        unsigned ipos=ist_pos[i];
        if ((int)i>max_band[i%3]&&ipos<max_pos){
            float kl,kr,s=DRMP3_HDR_TEST_MS_STEREO(hdr)?1.41421356f:1;
            if(DRMP3_HDR_TEST_MPEG1(hdr)){kl=g_pan[2*ipos];kr=g_pan[2*ipos+1];}
            else{kl=1;kr=drmp3_L3_ldexp_q2(1,(ipos+1)>>1<<mpeg2_sh);if(ipos&1){kl=kr;kr=1;}}
            drmp3_L3_intensity_stereo_band(left,sfb[i],kl*s,kr*s);
        } else if (DRMP3_HDR_TEST_MS_STEREO(hdr)) drmp3_L3_midside_stereo(left,sfb[i]);

        left+=sfb[i];
    }
}
static void drmp3_L3_intensity_stereo(float *left, u8 *ist_pos, const drmp3_L3_gr_info *gr, const u8 *hdr) {
    int max_band[3],n_sfb=gr->n_long_sfb+gr->n_short_sfb,i,max_blocks=gr->n_short_sfb?3:1;
    drmp3_L3_stereo_top_band(left+576,gr->sfbtab,n_sfb,max_band);
    if (gr->n_long_sfb) max_band[0]=max_band[1]=max_band[2]=vmax(vmax(max_band[0],max_band[1]),max_band[2]);
    for (i=0;i<max_blocks;i++){ int default_pos = DRMP3_HDR_TEST_MPEG1(hdr) ? 3 : 0, itop = n_sfb-max_blocks + i, prev = itop - max_blocks; ist_pos[itop] = (u8)(max_band[i] >= prev ? default_pos : ist_pos[prev]); }
    drmp3_L3_stereo_process(left,ist_pos,gr->sfbtab,hdr,max_band,gr[1].scalefac_compress&1);
}

static void drmp3_L3_reorder(float *grbuf, float *scratch, const u8 *sfb) {
    int i,len; float *src=grbuf,*dst=scratch;
    for(;0!=(len=*sfb);sfb+=3,src+=2*len){for(i=0;i<len;i++,src++){*dst++=src[0*len];*dst++=src[1*len];*dst++=src[2*len];}}
    MemCpyFromBtoAForNBytes(grbuf,scratch,(dst-scratch)*sizeof(float));
}

static void drmp3_L3_antialias(float *grbuf, int nbands) {
    static const float g_aa[2][8]={{0.85749293f,0.88174200f,0.94962865f,0.98331459f,0.99551782f,0.99916056f,0.99989920f,0.99999316f},{0.51449576f,0.47173197f,0.31337745f,0.18191320f,0.09457419f,0.04096558f,0.01419856f,0.00369997f}};
    for(;nbands>0;nbands--,grbuf+=18){
        int i=0;
        for(;i<8;i++){float u=grbuf[18+i],d=grbuf[17-i];grbuf[18+i]=u*g_aa[0][i]-d*g_aa[1][i];grbuf[17-i]=u*g_aa[1][i]+d*g_aa[0][i];}
    }
}

static void drmp3_L3_dct3_9(float *y) {
    float s1,s3,s5,s7,t0,t2,t4,s0=y[0],s2=y[2],s4=y[4],s6=y[6],s8=y[8];
    t0=s0+s6*0.5f; s0-=s6; t4=(s4+s2)*0.93969262f; t2=(s8+s2)*0.76604444f;
    s6=(s4-s8)*0.17364818f; s4+=s8-s2; s2=s0-s4*0.5f; y[4]=s4+s0;
    s8=t0-t2+s6; s0=t0-t4+t2; s4=t0+t4-s6;
    s1=y[1]; s3=y[3]; s5=y[5]; s7=y[7];
    s3*=0.86602540f; t0=(s5+s1)*0.98480775f; t4=(s5-s7)*0.34202014f; t2=(s1+s7)*0.64278761f;
    s1=(s1-s5-s7)*0.86602540f; s5=t0-s3-t2; s7=t4-s3-t0; s3=t4+s3-t2;
    y[0]=s4-s7; y[1]=s2+s1; y[2]=s0-s3; y[3]=s8+s5; y[5]=s8-s5; y[6]=s0+s3; y[7]=s2-s1; y[8]=s4+s7;
}

static void drmp3_L3_imdct36(float *grbuf, float *overlap, const float *win, int nbands) {
    int i,j;
    static const float g_twid9[18]={0.73727734f,0.79335334f,0.84339145f,0.88701083f,0.92387953f,0.95371695f,0.97629601f,0.99144486f,0.99904822f,0.67559021f,0.60876143f,0.53729961f,0.46174861f,0.38268343f,0.30070580f,0.21643961f,0.13052619f,0.04361938f};
    for (j=0;j<nbands;j++,grbuf+=18,overlap+=9){
        float co[9],si[9];
        co[0]=-grbuf[0]; si[0]=grbuf[17];
        for(i=0;i<4;i++){si[8-2*i]=grbuf[4*i+1]-grbuf[4*i+2];co[1+2*i]=grbuf[4*i+1]+grbuf[4*i+2];si[7-2*i]=grbuf[4*i+4]-grbuf[4*i+3];co[2+2*i]=-(grbuf[4*i+3]+grbuf[4*i+4]);}
        drmp3_L3_dct3_9(co); drmp3_L3_dct3_9(si);
        si[1]=-si[1];si[3]=-si[3];si[5]=-si[5];si[7]=-si[7];
        i=0;
        for(;i<9;i++){float ovl=overlap[i],sum=co[i]*g_twid9[9+i]+si[i]*g_twid9[i];overlap[i]=co[i]*g_twid9[i]-si[i]*g_twid9[9+i];grbuf[i]=ovl*win[i]-sum*win[9+i];grbuf[17-i]=ovl*win[9+i]+sum*win[i];}
    }
}

static void drmp3_L3_idct3(float x0,float x1,float x2,float *dst){float m1=x1*0.86602540f,a1=x0-x2*0.5f;dst[1]=x0+x2;dst[0]=a1+m1;dst[2]=a1-m1;}
static void imdct12(float *x,float *dst,float *overlap){
    static const float g_twid3[6]={0.79335334f,0.92387953f,0.99144486f,0.60876143f,0.38268343f,0.13052619f};
    float co[3],si[3]; int i; drmp3_L3_idct3(-x[0],x[6]+x[3],x[12]+x[9],co); drmp3_L3_idct3(x[15],x[12]-x[9],x[6]-x[3],si);
    si[1]=-si[1];
    for(i=0;i<3;i++){float ovl=overlap[i],sum=co[i]*g_twid3[3+i]+si[i]*g_twid3[i];overlap[i]=co[i]*g_twid3[i]-si[i]*g_twid3[3+i];dst[i]=ovl*g_twid3[2-i]-sum*g_twid3[5-i];dst[5-i]=ovl*g_twid3[5-i]+sum*g_twid3[2-i];}
}

static void drmp3_L3_imdct_short(float *grbuf,float *overlap,int nbands){ for(;nbands>0;nbands--,overlap+=9,grbuf+=18){float tmp[18]; MemCpyFromBtoAForNBytes(tmp,grbuf,sizeof(tmp)); MemCpyFromBtoAForNBytes(grbuf,overlap,6*sizeof(float)); imdct12(tmp,grbuf+6,overlap+6); imdct12(tmp+1,grbuf+12,overlap+6); imdct12(tmp+2,overlap,overlap+6);} }
static void drmp3_L3_change_sign(float *grbuf){int b,i;for(b=0,grbuf+=18;b<32;b+=2,grbuf+=36)for(i=1;i<18;i+=2)grbuf[i]=-grbuf[i];}
static const float g_mdct_window[2][18]={{0.99904822f,0.99144486f,0.97629601f,0.95371695f,0.92387953f,0.88701083f,0.84339145f,0.79335334f,0.73727734f,0.04361938f,0.13052619f,0.21643961f,0.30070580f,0.38268343f,0.46174861f,0.53729961f,0.60876143f,0.67559021f},{1,1,1,1,1,1,0.99144486f,0.92387953f,0.79335334f,0,0,0,0,0,0,0.13052619f,0.38268343f,0.60876143f}};
static void drmp3_L3_imdct_gr(float *grbuf,float *overlap,unsigned block_type,unsigned n_long_bands){
    if (n_long_bands){drmp3_L3_imdct36(grbuf,overlap,g_mdct_window[0],n_long_bands);grbuf+=18*n_long_bands;overlap+=9*n_long_bands;}
    if (block_type==2) {drmp3_L3_imdct_short(grbuf,overlap,32-n_long_bands);} else {drmp3_L3_imdct36(grbuf,overlap,g_mdct_window[block_type==3],32-n_long_bands);}
}

static void drmp3_L3_save_reservoir(drmp3dec *h, drmp3dec_scratch *s) { int pos=(s->bs.pos+7)/8u,remains=s->bs.limit/8u-pos; if (remains>511){pos+=remains-511;remains=511;} if (remains>0) {MoveMemoryFromBtoAForNBytes(h->reserv_buf,s->maindata+pos,remains);} h->reserv=remains; }
static int drmp3_L3_restore_reservoir(drmp3dec *h, drmp3_bs *bs, drmp3dec_scratch *s, int main_data_begin) { 
    int frame_bytes=(bs->limit-bs->pos)/8,bytes_have=vmin(h->reserv,main_data_begin);
    MemCpyFromBtoAForNBytes(s->maindata,h->reserv_buf+vmax(0,h->reserv-main_data_begin),vmin(h->reserv,main_data_begin));
    MemCpyFromBtoAForNBytes(s->maindata+bytes_have,bs->buf+bs->pos/8,frame_bytes);
    s->bs.buf=s->maindata; s->bs.pos=0; s->bs.limit=(bytes_have+frame_bytes) * 8;
    return h->reserv>=main_data_begin;
}

static void drmp3_L3_decode(drmp3dec *h, drmp3dec_scratch *s, drmp3_L3_gr_info *gr_info, int nch){
    int ch; for(ch=0;ch<nch;ch++) { int limit=s->bs.pos+gr_info[ch].part_23_length; drmp3_L3_decode_scalefactors(h->header,s->ist_pos[ch],&s->bs,gr_info+ch,s->scf,ch); drmp3_L3_huffman(s->grbuf[ch],&s->bs,gr_info+ch,s->scf,limit); }
    if (DRMP3_HDR_TEST_I_STEREO(h->header)) drmp3_L3_intensity_stereo(s->grbuf[0],s->ist_pos[1],gr_info,h->header);
    else if (DRMP3_HDR_IS_MS_STEREO(h->header)) drmp3_L3_midside_stereo(s->grbuf[0],576);
    for(ch=0;ch<nch;ch++,gr_info++){
        int aa_bands=31,n_long_bands=(gr_info->mixed_block_flag?2:0)<<(int)(DRMP3_HDR_GET_MY_SAMPLE_RATE(h->header)==2);
        if (gr_info->n_short_sfb){aa_bands=n_long_bands-1;drmp3_L3_reorder(s->grbuf[ch]+n_long_bands*18,s->syn[0],gr_info->sfbtab+gr_info->n_long_sfb);}
        drmp3_L3_antialias(s->grbuf[ch],aa_bands);
        drmp3_L3_imdct_gr(s->grbuf[ch],h->mdct_overlap[ch],gr_info->block_type,n_long_bands);
        drmp3_L3_change_sign(s->grbuf[ch]);
    }
}

static const float g_sec[24]={10.19000816f,0.50060302f,0.50241929f,3.40760851f,0.50547093f,0.52249861f,2.05778098f,0.51544732f,0.56694406f,1.48416460f,0.53104258f,0.64682180f,1.16943991f,0.55310392f,0.78815460f,0.97256821f,0.58293498f,1.06067765f,0.83934963f,0.62250412f,1.72244716f,0.74453628f,0.67480832f,5.10114861f};
static void drmp3d_DCT_II(float *grbuf, int n){
    int i,k=0;
    for(;k<n;k++){
        float t[4][8],*x,*y=grbuf+k;
        for(x=t[0],i=0;i<8;i++,x++){float x0=y[i*18],x1=y[(15-i)*18],x2=y[(16+i)*18],x3=y[(31-i)*18],t0=x0+x3,t1=x1+x2,t2=(x1-x2)*g_sec[3*i],t3=(x0-x3)*g_sec[3*i+1];x[0]=t0+t1;x[8]=(t0-t1)*g_sec[3*i+2];x[16]=t3+t2;x[24]=(t3-t2)*g_sec[3*i+2];}
        for(x=t[0],i=0;i<4;i++,x+=8){float x0=x[0],x1=x[1],x2=x[2],x3=x[3],x4=x[4],x5=x[5],x6=x[6],x7=x[7],xt;xt=x0-x7;x0+=x7;x7=x1-x6;x1+=x6;x6=x2-x5;x2+=x5;x5=x3-x4;x3+=x4;x4=x0-x3;x0+=x3;x3=x1-x2;x1+=x2;x[0]=x0+x1;x[4]=(x0-x1)*0.70710677f;x5+=x6;x6=(x6+x7)*0.70710677f;x7+=xt;x3=(x3+x4)*0.70710677f;x5-=x7*0.198912367f;x7+=x5*0.382683432f;x5-=x7*0.198912367f;x0=xt-x6;xt+=x6;x[1]=(xt+x7)*0.50979561f;x[2]=(x4+x3)*0.54119611f;x[3]=(x0-x5)*0.60134488f;x[5]=(x0+x5)*0.89997619f;x[6]=(x4-x3)*1.30656302f;x[7]=(xt-x7)*2.56291556f;}
        for(i=0;i<7;i++,y+=4*18){y[0]=t[0][i];y[18]=t[2][i]+t[3][i]+t[3][i+1];y[36]=t[1][i]+t[1][i+1];y[54]=t[2][i+1]+t[3][i]+t[3][i+1];}
        y[0]=t[0][7];y[18]=t[2][7]+t[3][7];y[36]=t[1][7];y[54]=t[3][7];
    }
}

static float drmp3d_scale_pcm(float sample) { return sample*(1.f/32768.f); }
typedef float drmp3d_sample_t;
static void drmp3d_synth_pair(drmp3d_sample_t *pcm, int nch, const float *z){
    float a;
    a =(z[14*64]-z[    0])*29; a+=(z[ 1*64]+z[13*64])*213; a+=(z[12*64]-z[ 2*64])*459;
    a+=(z[ 3*64]+z[11*64])*2037; a+=(z[10*64]-z[ 4*64])*5153; a+=(z[ 5*64]+z[ 9*64])*6574;
    a+=(z[ 8*64]-z[ 6*64])*37489; a+=z[7*64]*75038;
    pcm[0]=drmp3d_scale_pcm(a);
    z+=2;
    a =z[14*64]*104; a+=z[12*64]*1567; a+=z[10*64]*9727; a+=z[8*64]*64019;
    a+=z[6*64]*-9975; a+=z[4*64]*-45; a+=z[2*64]*146; a+=z[0*64]*-5;
    pcm[16*nch]=drmp3d_scale_pcm(a);
}

static const float g_win[]={ -1,26,-31,208,218,401,-519,2063,2000,4788,-5517,7134,5959,35640,-39336,74992,-1,24,-35,202,222,347,-581,2080,1952,4425,-5879,7640,5288,33791,-41176,74856,-1,21,-38,196,225,294,-645,2087,1893,4063,-6237,8092,4561,31947,-43006,74630,-1,19,-41,190,227,244,-711,2085,1822,3705,-6589,8492,3776,30112,-44821,74313,-1,17,-45,183,228,197,-779,2075,1739,3351,-6935,8840,2935,28289,-46617,73908,-1,16,-49,176,228,153,-848,2057,1644,3004,-7271,9139,2037,26482,-48390,73415,-2,14,-53,169,227,111,-919,2032,1535,2663,-7597,9389,1082,24694,-50137,72835,-2,13,-58,161,224,72,-991,2001,1414,2330,-7910,9592,70,22929,-51853,72169,-2,11,-63,154,221,36,-1064,1962,1280,2006,-8209,9750,-998,21189,-53534,71420,-2,10,-68,147,215,2,-1137,1919,1131,1692,-8491,9863,-2122,19478,-55178,70590,-3,9,-73,139,208,-29,-1210,1870,970,1388,-8755,9935,-3300,17799,-56778,69679,-3,8,-79,132,200,-57,-1283,1817,794,1095,-8998,9966,-4533,16155,-58333,68692,-4,7,-85,125,189,-83,-1356,1759,605,814,-9219,9959,-5818,14548,-59838,67629,-4,7,-91,117,177,-106,-1428,1698,402,545,-9416,9916,-7154,12980,-61289,66494,-5,6,-97,111,163,-127,-1498,1634,185,288,-9585,9838,-8540,11455,-62684,65290};
static void drmp3d_synth(float *xl, drmp3d_sample_t *dstl, int nch, float *lins){
    int i; float *xr=xl+576*(nch-1); drmp3d_sample_t *dstr=dstl+(nch-1);
    float *zlin=lins+15*64; const float *w=g_win;
    zlin[4*15]=xl[18*16];zlin[4*15+1]=xr[18*16];zlin[4*15+2]=xl[0];zlin[4*15+3]=xr[0];
    zlin[4*31]=xl[1+18*16];zlin[4*31+1]=xr[1+18*16];zlin[4*31+2]=xl[1];zlin[4*31+3]=xr[1];
    drmp3d_synth_pair(dstr,nch,lins+4*15+1);drmp3d_synth_pair(dstr+32*nch,nch,lins+4*15+64+1);
    drmp3d_synth_pair(dstl,nch,lins+4*15);drmp3d_synth_pair(dstl+32*nch,nch,lins+4*15+64);
    for(i=14;i>=0;i--){
#define DRMP3_LOAD(k) float w0=*w++;float w1=*w++;float *vz=&zlin[4*i-k*64];float *vy=&zlin[4*i-(15-k)*64];
#define DRMP3_S0(k) {int j;DRMP3_LOAD(k) for(j=0;j<4;j++)b[j]=vz[j]*w1+vy[j]*w0,a[j]=vz[j]*w0-vy[j]*w1;}
#define DRMP3_S1(k) {int j;DRMP3_LOAD(k) for(j=0;j<4;j++)b[j]+=vz[j]*w1+vy[j]*w0,a[j]+=vz[j]*w0-vy[j]*w1;}
#define DRMP3_S2(k) {int j;DRMP3_LOAD(k) for(j=0;j<4;j++)b[j]+=vz[j]*w1+vy[j]*w0,a[j]+=vy[j]*w1-vz[j]*w0;}
        float a[4],b[4];
        zlin[4*i]=xl[18*(31-i)];zlin[4*i+1]=xr[18*(31-i)];zlin[4*i+2]=xl[1+18*(31-i)];zlin[4*i+3]=xr[1+18*(31-i)];
        zlin[4*(i+16)]=xl[1+18*(1+i)];zlin[4*(i+16)+1]=xr[1+18*(1+i)];zlin[4*(i-16)+2]=xl[18*(1+i)];zlin[4*(i-16)+3]=xr[18*(1+i)];
        DRMP3_S0(0) DRMP3_S2(1) DRMP3_S1(2) DRMP3_S2(3) DRMP3_S1(4) DRMP3_S2(5) DRMP3_S1(6) DRMP3_S2(7)
        dstr[(15-i)*nch]=drmp3d_scale_pcm(a[1]); dstr[(17+i)*nch]=drmp3d_scale_pcm(b[1]);
        dstl[(15-i)*nch]=drmp3d_scale_pcm(a[0]); dstl[(17+i)*nch]=drmp3d_scale_pcm(b[0]);
        dstr[(47-i)*nch]=drmp3d_scale_pcm(a[3]); dstr[(49+i)*nch]=drmp3d_scale_pcm(b[3]);
        dstl[(47-i)*nch]=drmp3d_scale_pcm(a[2]); dstl[(49+i)*nch]=drmp3d_scale_pcm(b[2]);
    }
}

static void drmp3d_synth_granule(float *qmf_state, float *grbuf, int nbands, int nch, drmp3d_sample_t *pcm, float *lins){ for(int i=0;i<nch;i++) {drmp3d_DCT_II(grbuf+576*i,nbands);} MemCpyFromBtoAForNBytes(lins,qmf_state,sizeof(float)*15*64); for(int i=0;i<nbands;i+=2) {drmp3d_synth(grbuf+i,pcm+32*nch*i,nch,lins+i*64);} MemCpyFromBtoAForNBytes(qmf_state,lins+nbands*64,sizeof(float)*15*64); }
static int drmp3d_match_frame(const u8 *hdr, int mp3_bytes, int frame_bytes){ for(int i=0,nmatch=0;nmatch<10;nmatch++){ i+=drmp3_hdr_frame_bytes(hdr+i,frame_bytes)+drmp3_hdr_padding(hdr+i); if (i + 4 > mp3_bytes) {return nmatch>0;} if (!drmp3_hdr_compare(hdr,hdr+i)) {return 0;} } return 1; }
static int drmp3d_find_frame(const u8 *mp3, int mp3_bytes, int *free_format_bytes, int *ptr_frame_bytes){
    int i,k;
    for(i=0;i<mp3_bytes-4;i++,mp3++){
        if (drmp3_hdr_valid(mp3)){
            int frame_bytes=drmp3_hdr_frame_bytes(mp3,*free_format_bytes);
            int frame_and_padding=frame_bytes+drmp3_hdr_padding(mp3);
            for(k=4;!frame_bytes&&k<2304&&i+2*k<mp3_bytes - 4;k++){
                if (drmp3_hdr_compare(mp3,mp3+k)) { int fb=k-drmp3_hdr_padding(mp3),nextfb=fb+drmp3_hdr_padding(mp3+k); if (i + k + nextfb + 4 > mp3_bytes || !drmp3_hdr_compare(mp3,mp3+k+nextfb)) continue; frame_and_padding=k; frame_bytes=fb; *free_format_bytes=fb; }
            }

            if((frame_bytes&&i+frame_and_padding<=mp3_bytes&&drmp3d_match_frame(mp3,mp3_bytes-i,frame_bytes))||(!i&&frame_and_padding==mp3_bytes)){*ptr_frame_bytes=frame_and_padding;return i;}
            *free_format_bytes=0;
        }
    }
    *ptr_frame_bytes=0; return mp3_bytes;
}

static void drmp3dec_init(drmp3dec *dec) { dec->header[0]=0; }
static int drmp3dec_decode_frame(drmp3dec *dec, const u8 *mp3, int mp3_bytes, void *pcm, drmp3dec_frame_info *info){
    int i=0,igr,frame_size=0,success=1;
    const u8 *hdr; drmp3_bs bs_frame[1];
    if (mp3_bytes>4&&dec->header[0]==0xff&&drmp3_hdr_compare(dec->header,mp3)){
        frame_size=drmp3_hdr_frame_bytes(mp3,dec->free_format_bytes)+drmp3_hdr_padding(mp3);
        if (frame_size!=mp3_bytes&&(frame_size + 4>mp3_bytes||!drmp3_hdr_compare(mp3,mp3+frame_size))) frame_size=0;
    }
    if (!frame_size){ MemSetToVForNBytes(dec,0,sizeof(drmp3dec)); i=drmp3d_find_frame(mp3,mp3_bytes,&dec->free_format_bytes,&frame_size); if (!frame_size || i + frame_size > mp3_bytes) {info->frame_bytes=i;return 0;} }
    
    hdr=mp3+i;
    MemCpyFromBtoAForNBytes(dec->header,hdr,4);
    info->frame_bytes=i+frame_size;
    info->channels=DRMP3_HDR_IS_MONO(hdr) ? 1 : 2;
    info->sample_rate=drmp3_hdr_sample_rate_hz(hdr);
    info->layer = 4 - DRMP3_HDR_GET_LAYER(hdr);
    info->bitrate_kbps=drmp3_hdr_bitrate_kbps(hdr);
    bs_frame[0].buf=hdr + 4; bs_frame[0].pos=0; bs_frame[0].limit=(frame_size - 4) * 8;
    if(DRMP3_HDR_IS_CRC(hdr)) drmp3_bs_get_bits(bs_frame,16);
    if(info->layer!=3) return 0;  /* Layer 1/2 not supported */
        
    int main_data_begin=drmp3_L3_read_side_info(bs_frame,dec->scratch.gr_info,hdr);
    if(main_data_begin<0||bs_frame->pos>bs_frame->limit){drmp3dec_init(dec);return 0;}
    success=drmp3_L3_restore_reservoir(dec,bs_frame,&dec->scratch,main_data_begin);
    if(success&&pcm!=NULL){
        for(igr=0;igr<(DRMP3_HDR_TEST_MPEG1(hdr)?2:1);igr++,pcm=DRMP3_OFFSET_PTR(pcm,sizeof(drmp3d_sample_t)*576*info->channels)){ MemSetToVForNBytes(dec->scratch.grbuf[0],0,576 * 2 * sizeof(float)); drmp3_L3_decode(dec,&dec->scratch,dec->scratch.gr_info+igr*info->channels,info->channels); drmp3d_synth_granule(dec->qmf_state,dec->scratch.grbuf[0],18,info->channels,(drmp3d_sample_t*)pcm,dec->scratch.syn[0]); }
    }
    drmp3_L3_save_reservoir(dec,&dec->scratch);
    return success*drmp3_hdr_frame_samples(dec->header);
}

static size_t drmp3__on_read_os(void *ud, void *buf, size_t n) { FHandle f = (FHandle)(uintptr_t)ud; if (f == INVALID_FHANDLE) {return 0;} long result = OS_Read(f,buf,n); return (result > 0) ? (size_t)result : 0; }
static bool drmp3__on_seek_os(void *ud, int offset, u8 origin) { FHandle f = (FHandle)(uintptr_t)ud; if (f == INVALID_FHANDLE) {return false;} int whence = origin; return OS_Seek(f,(i64)offset,whence) >= 0; }
static size_t drmp3__on_read(drmp3 *p, void *buf, size_t n) { size_t r = drmp3__on_read_os(p->pUserData,buf,n); p->streamCursor += r; return r; }
static size_t drmp3__on_read_clamped(drmp3 *p, void *buf, size_t n) { if (p->streamLength == (((u64)0xFFFFFFFF << 32) | (u64)0xFFFFFFFF)) {return drmp3__on_read(p,buf,n);} u64 rem = p->streamLength - p->streamCursor; if (n > rem) n = (size_t)rem; return drmp3__on_read(p,buf,n); }
static bool drmp3__on_seek(drmp3 *p, int offset, u8 origin) { if (!drmp3__on_seek_os(p->pUserData,offset,origin)) {return false;} if (origin == 0) {p->streamCursor = (u64)offset;}else{p->streamCursor += (u64)offset;} return true; }
static u32 drmp3_decode_next_frame_ex(drmp3 *p, drmp3d_sample_t *pPCMFrames, drmp3dec_frame_info *pInfo) {
    u32 pcmFramesRead = 0; if (p->atEnd) return 0;

    for (;;) {
        drmp3dec_frame_info info;
        if (p->dataSize < 16384) {
            if (p->pData) MoveMemoryFromBtoAForNBytes(p->pData, p->pData + p->dataConsumed, p->dataSize);
            p->dataConsumed = 0;
            if (p->dataCapacity < (16384 * 4)) { u8 *nd = (u8*)OS_Realloc(p->pData,p->dataCapacity,16384 * 4); p->pData = nd; p->dataCapacity = 16384 * 4; }

            size_t bytesRead = drmp3__on_read_clamped(p, p->pData + p->dataSize, p->dataCapacity - p->dataSize);
            if (!bytesRead && p->dataSize == 0) { p->atEnd = 1; return 0; }
            p->dataSize += bytesRead;
        }

        if (p->dataSize > 2147483647) { p->atEnd = 1; return 0; }
        if (!p->pData) return 0;

        pcmFramesRead = drmp3dec_decode_frame(&p->decoder,p->pData + p->dataConsumed,(int)p->dataSize,pPCMFrames,&info);
        p->dataConsumed += (size_t)info.frame_bytes; p->dataSize -= (size_t)info.frame_bytes;
        if (pcmFramesRead > 0) {
            pcmFramesRead = drmp3_hdr_frame_samples(p->decoder.header);
            p->pcmFConsInMP3F = 0; p->pcmFRemInMP3F = pcmFramesRead; p->mp3FChan = info.channels; p->mp3FrameSampleRate = info.sample_rate; 
            if (pInfo) *pInfo = info;
            break;
        } else if (info.frame_bytes == 0) {
            MoveMemoryFromBtoAForNBytes(p->pData, p->pData + p->dataConsumed, p->dataSize);
            p->dataConsumed = 0;
            if (p->dataCapacity == p->dataSize) { size_t needed=p->dataCapacity + 16384*4; u8 *nd=(u8*)OS_Realloc(p->pData,p->dataCapacity,needed); p->pData=nd; p->dataCapacity=needed; }
            size_t bytesRead = drmp3__on_read_clamped(p,p->pData + p->dataSize,p->dataCapacity - p->dataSize);
            if (!bytesRead) { p->atEnd = 1; return 0; }
            p->dataSize += bytesRead;
        }
    }
    return pcmFramesRead;
}

static u32 drmp3_decode_next_frame(drmp3 *p) { return drmp3_decode_next_frame_ex(p,(drmp3d_sample_t*)p->pcmFrames,NULL); }
static void drmp3__skip_id3v2(drmp3 *p) {
    char h[10]; if (drmp3__on_read_os(p->pUserData, h, 10) != 10) return;

    if (h[0] == 'I' && h[1] == 'D' && h[2] == '3') {
        u32 sz = (((u32)h[6] & 0x7F) << 21) | (((u32)h[7] & 0x7F) << 14) | (((u32)h[8] & 0x7F) << 7) | ((u32)h[9] & 0x7F);
        if (h[5] & 0x10) sz += 10;
        drmp3__on_seek_os(p->pUserData,(int)sz,1); // SEEK_CUR
        p->streamStartOffset += 10 + sz;
        p->streamCursor = p->streamStartOffset;
    } else drmp3__on_seek_os(p->pUserData,0,0); // SEEK_SET
}

static bool drmp3_init_internal(drmp3 *p) {
    drmp3dec_init(&p->decoder);
    p->streamCursor = p->streamStartOffset=0; p->streamLength = (((u64)0xFFFFFFFF << 32) | (u64)0xFFFFFFFF); p->delayInPCMFrames = p->paddingInPCMFrames=0; p->totalPCMFrameCount = (((u64)0xFFFFFFFF << 32) | (u64)0xFFFFFFFF);
    if (drmp3__on_seek_os(p->pUserData, 0, 2)) { // SEEK_END
        i64 slen = OS_Tell((FHandle)(uintptr_t)p->pUserData);
        if (slen > 0) {
            if (slen > 128) {
                char tag[3]; drmp3__on_seek_os(p->pUserData,-128,2); if (drmp3__on_read(p, tag, 3) == 3 && tag[0]=='T' && tag[1]=='A' && tag[2]=='G') slen -= 128;
            }
            
            p->streamLength = (u64)slen;
        }
        
        drmp3__on_seek_os(p->pUserData,0,0); p->streamCursor = 0;
    }

    drmp3__skip_id3v2(p); drmp3dec_frame_info firstFrameInfo;
    u32 firstFramePCMFrameCount = drmp3_decode_next_frame_ex(p,(drmp3d_sample_t*)p->pcmFrames,&firstFrameInfo);
    if (firstFramePCMFrameCount == 0) { OS_DeallocateRAM(p->pData,p->dataCapacity); p->pData = NULL; p->dataCapacity = 0; return false; }

    p->channels=p->mp3FChan; p->sampleRate=p->mp3FrameSampleRate;
    return true;
}

static bool drmp3_init_file(drmp3 *pMP3, const char *pFilePath) {
    if (!pMP3 || !pFilePath) return false;
    MemSetToVForNBytes(pMP3,0,sizeof(drmp3)); FHandle f = OS_OpenReadonly(pFilePath); if (f == INVALID_FHANDLE) return false;

    pMP3->pUserData = (void*)(uintptr_t)f;
    bool result = drmp3_init_internal(pMP3);
    if (!result) { OS_Close(f); return false; }
    return true;
}

static void drmp3_uninit(drmp3 *pMP3) { if (!pMP3) {return;}   if (pMP3->pUserData) {OS_Close((FHandle)(uintptr_t)pMP3->pUserData); pMP3->pUserData=NULL;}   OS_DeallocateRAM(pMP3->pData, pMP3->dataCapacity); pMP3->pData = NULL; pMP3->dataCapacity = 0; }
static void drmp3_reset(drmp3 *p) { p->pcmFConsInMP3F=0; p->pcmFRemInMP3F=0; p->currentPCMFrame=0; p->dataSize=0; p->atEnd=0; drmp3dec_init(&p->decoder); }
static bool drmp3_seek_to_start_of_stream(drmp3 *p){u64 o=p->streamStartOffset;if(!drmp3__on_seek(p,o<=0x7FFFFFFF?(int)o:0x7FFFFFFF,0))return 0;if(o>0x7FFFFFFF){o-=0x7FFFFFFF;while(o>0){int c=(o<=0x7FFFFFFF)?(int)o:0x7FFFFFFF;if(!drmp3__on_seek(p,c,1))return 0;o-=c;}}drmp3_reset(p);return 1;}
static u64 drmp3_read_pcm_frames_raw(drmp3 *p, u64 framesToRead, void *pBufferOut){
    u64 totalFramesRead=0;
    while(framesToRead>0){
        u32 framesToConsume;
        if(p->currentPCMFrame<p->delayInPCMFrames){ u32 skip=(u32)vmin(p->pcmFRemInMP3F,p->delayInPCMFrames-p->currentPCMFrame); p->currentPCMFrame+=skip; p->pcmFConsInMP3F+=skip; p->pcmFRemInMP3F-=skip; }
        framesToConsume=(u32)vmin(p->pcmFRemInMP3F,framesToRead);
        if(p->totalPCMFrameCount != (((u64)0xFFFFFFFF << 32) | (u64)0xFFFFFFFF) && p->totalPCMFrameCount > p->paddingInPCMFrames){
            if(p->currentPCMFrame<(p->totalPCMFrameCount-p->paddingInPCMFrames)){ u64 rem=(p->totalPCMFrameCount-p->paddingInPCMFrames)-p->currentPCMFrame; if(framesToConsume>rem) framesToConsume=(u32)rem; } else break;
        }
        if(pBufferOut){
            float *out=(float*)DRMP3_OFFSET_PTR(pBufferOut,sizeof(float)*totalFramesRead*p->channels);
            float *in =(float*)DRMP3_OFFSET_PTR(&p->pcmFrames[0],sizeof(float)*p->pcmFConsInMP3F*p->mp3FChan);
            MemCpyFromBtoAForNBytes(out,in,sizeof(float)*framesToConsume*p->channels);
        }
        p->currentPCMFrame+=framesToConsume; p->pcmFConsInMP3F+=framesToConsume;
        p->pcmFRemInMP3F-=framesToConsume;
        totalFramesRead+=framesToConsume; framesToRead-=framesToConsume;
        if(framesToRead==0) break;
        if(p->totalPCMFrameCount != (((u64)0xFFFFFFFF << 32) | (u64)0xFFFFFFFF) && p->totalPCMFrameCount > p->paddingInPCMFrames && p->currentPCMFrame >= (p->totalPCMFrameCount - p->paddingInPCMFrames)) break;
        if(drmp3_decode_next_frame(p)==0) break;
    }
    return totalFramesRead;
}

static u64 drmp3_read_pcm_frames_f32(drmp3 *pMP3, u64 framesToRead, float *pBufferOut){ if(!pMP3) {return 0;} return drmp3_read_pcm_frames_raw(pMP3,framesToRead,pBufferOut); }
static bool drmp3_seek_to_pcm_frame(drmp3 *pMP3, u64 frameIndex){
    if(!pMP3) return 0;
    if(frameIndex==0) return drmp3_seek_to_start_of_stream(pMP3);
    if(frameIndex<pMP3->currentPCMFrame){ if(!drmp3_seek_to_start_of_stream(pMP3)) {return 0;} }
    
    u64 toSkip=frameIndex-pMP3->currentPCMFrame; u64 skipped=drmp3_read_pcm_frames_f32(pMP3,toSkip,NULL);
    return skipped==toSkip;
}

static u64 drmp3_get_pcm_frame_count(drmp3 *pMP3){    
    u64 total;
    if(pMP3->totalPCMFrameCount != (((u64)0xFFFFFFFF << 32) | (u64)0xFFFFFFFF)){
        total=pMP3->totalPCMFrameCount;
        if(total>=pMP3->delayInPCMFrames)   total-=pMP3->delayInPCMFrames;
        if(total>=pMP3->paddingInPCMFrames) total-=pMP3->paddingInPCMFrames;
        return total;
    }

    u64 savedFrame=pMP3->currentPCMFrame;
    if(!drmp3_seek_to_start_of_stream(pMP3)) return 0;
    total=0;
    for(;;){u32 n=drmp3_decode_next_frame_ex(pMP3,NULL,NULL);if(!n)break;total+=n;}
    drmp3_seek_to_start_of_stream(pMP3); drmp3_seek_to_pcm_frame(pMP3,savedFrame);
    return total;
}
// Wav parsing --------------
typedef struct { FHandle fp; u16 channels,bitsPerSample,fmtTag; u32 sampleRate; u64 totalPCMFrameCount,dataChunkDataPos,bytesRemaining; } WaveFile;
static u16 WavU16LE(const u8 *d) { return (u16)(d[0]|(d[1]<<8)); }
static u32 WavU32LE(const u8 *d) { return (u32)(d[0]|(d[1]<<8)|(d[2]<<16)|(d[3]<<24)); }
static bool WavInit(WaveFile *w, const char *path) {
    u8 buf[36]; MemSetToVForNBytes(w,0,sizeof(*w));
    w->fp = OS_OpenReadonly(path);
    if (w->fp == INVALID_FHANDLE) return false;
    if (OS_Read(w->fp, buf, 12) != 12) goto fail;
    if (CompareMemoryForNBytes(buf,"RIFF",4) != 0) goto fail;
    if (CompareMemoryForNBytes(buf+8,"WAVE",4) != 0) goto fail;
    bool got_fmt=false,got_data=false;
    for (;;) {
        u8 chunkId[4],szBuf[4]; if ((OS_Read(w->fp,chunkId,4) != 4) || (OS_Read(w->fp,szBuf,4) != 4)) break;
        
        u32 chunkSize = WavU32LE(szBuf);
        if (CompareMemoryForNBytes(chunkId, "fmt ", 4) == 0) {
            if (chunkSize < 16) goto fail;
            
            u8 fmt[18]; u32 toRead = chunkSize < 18 ? chunkSize : 18;
            if (OS_Read(w->fp,fmt,toRead) != (long)toRead) goto fail;
            if (chunkSize > toRead) OS_Seek(w->fp, (i64)(chunkSize - toRead),1);
            w->fmtTag = WavU16LE(fmt+0); w->channels = WavU16LE(fmt+2); w->sampleRate = WavU32LE(fmt+4); w->bitsPerSample = WavU16LE(fmt+14);
            if (w->fmtTag == 0xFFFE && toRead >= 18) {
                u16 cbSize = WavU16LE(fmt + 16);
                if (cbSize >= 22) {
                    u8 ext[22]; if (OS_Read(w->fp,ext,22) == 22) w->fmtTag = WavU16LE(ext + 6);
                }
            }
            if (w->fmtTag != 0x1) goto fail; // PCM format
            if (w->bitsPerSample != 8 && w->bitsPerSample != 16) goto fail;
            got_fmt = true;
        } else if (CompareMemoryForNBytes(chunkId,"data",4) == 0) {
            w->dataChunkDataPos = (u64)OS_Tell(w->fp);
            u32 bpf = (u32)w->channels * (w->bitsPerSample / 8);
            if (bpf == 0) goto fail;
            w->bytesRemaining = chunkSize - (chunkSize % bpf);
            w->totalPCMFrameCount = w->bytesRemaining / bpf;
            got_data = true;
            break; /* data chunk is last thing we need */
        } else OS_Seek(w->fp,(i64)(chunkSize + (chunkSize & 1)),1);
    }

    if (got_fmt && got_data) return true;
    fail:
    if (w->fp != INVALID_FHANDLE) { OS_Close(w->fp); w->fp = INVALID_FHANDLE; }
    return false;
}

static u64 WavReadPCMFrames(WaveFile *w, u64 framesToRead, float *out) {
    if (!w || !out || framesToRead == 0) return 0;
    u32 bps = w->bitsPerSample; u32 bpf = (u32)w->channels * (bps / 8); if (bpf == 0) return 0;

    u64 framesLeft = w->bytesRemaining / bpf; if (framesToRead > framesLeft) framesToRead = framesLeft;
    u64 totalRead = 0; u8  tmp[4096];
    while (framesToRead > 0) {
        u64 batchFrames=framesToRead; u64 batchBytes=batchFrames * bpf;
        if (batchBytes > sizeof(tmp)) { batchFrames = sizeof(tmp) / bpf; batchBytes  = batchFrames * bpf; }
        size_t got = OS_Read(w->fp,tmp,(size_t)batchBytes);
        u64 gotFrames = got / bpf;
        u64 samples   = gotFrames * w->channels;
        if (bps == 8) { for (u64 i = 0; i < samples; i++) {*out++ = (tmp[i] / 255.0f) * 2.0f - 1.0f;} }
        else { for (u64 i = 0; i < samples; i++) {i16 s; MemCpyFromBtoAForNBytes(&s,tmp + i*2,2); *out++ = s * (1.0f / 32768.0f);} } // 16bit LE

        w->bytesRemaining -= gotFrames * bpf; framesToRead -= gotFrames; totalRead += gotFrames; if (gotFrames < batchFrames) break;
    }
    return totalRead;
}

typedef struct { float *samples; u32 frame_count,frame_pos; float volume; bool looping,positional,playing; Vector3 pos; size_t allocSize; } wav_channel_t;
typedef struct { drmp3 dec; bool open; float fade_vol,fade_target,fade_step; u32 src_rate; u64 frames_decoded,total_frames; } mp3_channel_t;
static wav_channel_t wav_ch[MAX_CHANNELS],*ext_ch[MAX_CHANNELS]; static u32 wav_count,ext_count,mp3_slot,log_frame_count,log_frame_pos; 
static mp3_channel_t mp3_ch[2]; static float *log_samples; static size_t log_allocSize=0; static bool log_playing,mp3_paused = false;
static float *resample_stereo(float *src, size_t srcSize, u32 *frames, u32 src_rate, size_t* allocSize) {
    if (src_rate == AUDIO_RATE) return src;
    
    u32 sf = *frames, df = (u32)((u64)sf*AUDIO_RATE/src_rate);
    float *dst = (float*)OS_Alloc(df*2*sizeof(float)); *allocSize = df*2*sizeof(float);
    float ratio = (float)sf/(float)df;
    for (u32 i = 0; i < df; i++) { float pos = i*ratio; u32 a = (u32)pos, b = a+1<sf?a+1:a; float t = pos-(float)a; dst[i*2+0] = src[a*2+0]+t*(src[b*2+0]-src[a*2+0]); dst[i*2+1] = src[a*2+1]+t*(src[b*2+1]-src[a*2+1]); }
    OS_DeallocateRAM(src,srcSize); *frames = df; return dst;
}

static void WavUnInit(WaveFile *w) { if (w->fp != INVALID_FHANDLE) { OS_Close(w->fp); w->fp = INVALID_FHANDLE; } }
static float *load_wav(const char *path,u32 *out_frames, size_t* allocSize) {
    WaveFile wav; if (!WavInit(&wav,path)) return NULL;
    if (wav.channels > 2) { WavUnInit(&wav); return NULL; }
    
    u64 frames = wav.totalPCMFrameCount;
    float *buf = (float*)OS_Alloc(frames*AUDIO_CHANNELS*sizeof(float)); size_t bufSize = frames*AUDIO_CHANNELS*sizeof(float);
    u64 got = WavReadPCMFrames(&wav,frames,buf);
    if (wav.channels == 1) for (i64 i=(i64)got-1;i>=0;i--) { buf[i*2+1]=buf[i]; buf[i*2]=buf[i]; }
    u32 src_rate = wav.sampleRate;
    WavUnInit(&wav);
    *out_frames = (u32)got;
    return resample_stereo(buf,bufSize,out_frames,src_rate,allocSize); // Reallocates and returns new buffer, freeing the buf alloc'ed here
}

static void wave_mix(wav_channel_t* w, float* mix) {
    float vol = w->volume * (Sys_Settings.VolumeMaster/100.0f)*(Sys_Settings.VolumeEffects/100.0f); Vector3 pos = w->pos;
    Vector3 d = {pos.x-Sys_Global.instances[PLAYER1].position.x,pos.y-Sys_Global.instances[PLAYER1].position.y,pos.z-Sys_Global.instances[PLAYER1].position.z};
    float dist = vsqrtf(d.x*d.x+d.y*d.y+d.z*d.z);
    float spatial_atten = (dist >= 64.0f) ? 0.0f : ((dist <= 1.0f) ? 1.0f : 1.0f-(dist-1.0f)/63.0f);
    if (w->positional) vol *= spatial_atten;
    for (i32 f = 0; f < AUDIO_FRAMES; f++) {
        if (w->frame_pos >= w->frame_count) { if (w->looping) w->frame_pos=0; else { w->playing=false; break; } }
        mix[f*2+0] += w->samples[w->frame_pos*2+0]*vol; mix[f*2+1] += w->samples[w->frame_pos*2+1]*vol; w->frame_pos++;
    }
}

static void audio_mix_period(i16 *out) {
    float mix[AUDIO_FRAMES*AUDIO_CHANNELS];
    MemSetToVForNBytes(mix,0,sizeof(mix));
    for (u32 c=0;c<wav_count;c++) { wav_channel_t* w = &wav_ch[c]; if (w->playing && w->samples) {wave_mix(w,mix);} }
    for (u32 c=0;c<ext_count;c++) { wav_channel_t* w =  ext_ch[c]; if (w->playing && w->samples) {wave_mix(w,mix);} }
    if (log_playing && log_samples) {
        float vol = (Sys_Settings.VolumeMaster/100.0f)*(Sys_Settings.VolumeMessage/100.0f);
        for (i32 f = 0; f < AUDIO_FRAMES; f++) {
            if (log_frame_pos >= log_frame_count) { log_playing=false; break; }
            mix[f*2+0] += log_samples[log_frame_pos*2+0] * vol; mix[f*2+1] += log_samples[log_frame_pos*2+1] * vol; log_frame_pos++;
        }
    }

    if (!mp3_paused) {
        for (u32 s = 0; s < 2; s++) {
            mp3_channel_t *m = &mp3_ch[s];
            if (!m->open) continue;
            u32 src_rate = m->src_rate ? m->src_rate : AUDIO_RATE;
            u64 frames_to_read = (src_rate == AUDIO_RATE) ? AUDIO_FRAMES : (u64)((u64)AUDIO_FRAMES*src_rate/AUDIO_RATE)+2;
            float raw[AUDIO_FRAMES*4];
            u64 got = drmp3_read_pcm_frames_f32(&m->dec,frames_to_read,raw);
            if (got == 0) { drmp3_uninit(&m->dec); m->open=false; continue; }
            float vol = m->fade_vol * (Sys_Settings.VolumeMaster/100.0f)*(Sys_Settings.VolumeMusic/100.0f);
            m->frames_decoded += got; float ratio = (float)got/(float)AUDIO_FRAMES;
            for (i32 f = 0; f < AUDIO_FRAMES; f++) {
                float pos = f*ratio; u32 a=(u32)pos, b=(a+1<(u32)got)?a+1:a; float t=pos-(float)a;
                float l = raw[a*2+0]+t*(raw[b*2+0]-raw[a*2+0]), r = raw[a*2+1]+t*(raw[b*2+1]-raw[a*2+1]);
                mix[f*2+0] += l*vol; mix[f*2+1] += r*vol;
                if (m->fade_step != 0.0f) {
                    m->fade_vol += m->fade_step;
                    if (m->fade_step>0.0f && m->fade_vol>=m->fade_target) { m->fade_vol=m->fade_target; m->fade_step=0.0f; }
                    else if (m->fade_step<0.0f && m->fade_vol<=m->fade_target) {
                        m->fade_vol=m->fade_target; m->fade_step=0.0f; if (m->fade_target==0.0f) { drmp3_uninit(&m->dec); m->open=false; }
                    }
                }
            }
        }
    }

    for (u32 i = 0; i < AUDIO_FRAMES * AUDIO_CHANNELS; i++) { float s = mix[i]; s = s > 1.0f ? 1.0f : (s < -1.0f ? -1.0f : s); out[i] = (i16)(s * 32767.0f); }
}

ENGINE_TO_MOD void play_wav(const char *path,float volume,Vector3 pos,bool positional) {
    if (StringsEqual(path,"./Audio/misc/null.wav")) return;

    i32 slot = -1;
    for (u32 i = 0; i < wav_count; i++) if (!wav_ch[i].playing && wav_ch[i].samples) { OS_DeallocateRAM(wav_ch[i].samples,wav_ch[i].allocSize); wav_ch[i].samples=NULL; wav_ch[i].allocSize=0; slot=i; break; }
    if (slot==-1 && wav_count<MAX_CHANNELS) slot=wav_count++;
    if (slot==-1) { DualLog("WARNING: Max WAV channels (%d) reached\n",MAX_CHANNELS); return; }
    u32 frames; size_t allocSize=0; float *buf = load_wav(path,&frames,&allocSize);
    if (!buf) { DualLog("ERROR: Failed to load WAV %s\n",path); return; }
    wav_channel_t *w = &wav_ch[slot];
    w->samples=buf; w->allocSize = allocSize; w->frame_count=frames; w->frame_pos=0; w->volume=volume;
    w->looping=false; w->positional=positional; w->pos=pos; w->playing=true;
}

ENGINE_TO_MOD void play_message(const char *path) {
    if (log_playing && log_samples && log_allocSize > 0) { log_playing=false; OS_DeallocateRAM(log_samples,log_allocSize); log_samples=NULL; log_allocSize=0; }
    u32 frames; float *buf = load_wav(path,&frames,&log_allocSize);
    if (!buf) { DualLog("ERROR: Failed to load message WAV %s\n",path); return; }
    log_samples=buf; log_frame_count=frames; log_frame_pos=0; log_playing=true;
}

ENGINE_TO_MOD i32 SoundInit(const char *path,ma_sound *pSound) { wav_channel_t *w=(wav_channel_t*)pSound; u32 frames; size_t allocSize=0; float *buf=load_wav(path,&frames,&allocSize); if (!buf) return -1; w->samples=buf; w->allocSize=allocSize; w->frame_count=frames; w->frame_pos=0; w->volume=1.0f; w->looping=false; w->positional=false; w->playing=false; return 0; }
ENGINE_TO_MOD i32 SoundStart(ma_sound *pSound) { wav_channel_t *w = (wav_channel_t*)pSound; w->frame_pos = 0; w->playing = true; for (u32 i=0;i<ext_count;++i) if (ext_ch[i] == w) return 0; if (ext_count < MAX_CHANNELS) ext_ch[ext_count++] = w; return 0; }
ENGINE_TO_MOD i32 SoundStop(ma_sound *pSound) { ((wav_channel_t*)pSound)->playing=false; return 0; }
ENGINE_TO_MOD void SoundUninit(ma_sound *s) { wav_channel_t *w = (wav_channel_t*)s; if (w->samples) { OS_DeallocateRAM(w->samples,w->allocSize); w->samples = NULL; w->allocSize = 0; } w->playing = false; for (u32 i=0;i<ext_count;++i) if (ext_ch[i] == w) { ext_ch[i] = ext_ch[--ext_count]; break; } }
ENGINE_TO_MOD void SoundSetVolume(ma_sound *pSound, float volume) { ((wav_channel_t*)pSound)->volume=volume; }
ENGINE_TO_MOD void SoundSetLooping(ma_sound *pSound, i32 loop) { ((wav_channel_t*)pSound)->looping=(bool)loop; }
ENGINE_TO_MOD bool GetSoundIsPlaying(ma_sound *pSound)  { return ((wav_channel_t*)pSound)->playing; }
ENGINE_TO_MOD i32 SoundGetCurrentFrameCursor(const ma_sound *pSound,u64 *pCursor) { *pCursor=((wav_channel_t*)pSound)->frame_pos; return 0; }
ENGINE_TO_MOD float SoundGetLength(ma_sound *pSound) { wav_channel_t *w=(wav_channel_t*)pSound; return (w->samples&&AUDIO_RATE)?(float)w->frame_count/(float)AUDIO_RATE:0.0f; }
static void mp3_open_slot(i32 s, const char *path, float fade_from, float fade_to, i32 fade_ms) {
    mp3_channel_t *m = &mp3_ch[s]; if (m->open) { drmp3_uninit(&m->dec); m->open=false; } if (!drmp3_init_file(&m->dec,path)) { DualLog("ERROR: Failed to load MP3 %s\n",path); return; }
    m->src_rate = m->dec.sampleRate; m->total_frames = drmp3_get_pcm_frame_count(&m->dec); drmp3_seek_to_pcm_frame(&m->dec,0); m->frames_decoded = 0; m->open = true; m->fade_target = fade_to;
    m->fade_vol = (m->fade_step = (fade_ms > 0) ? (fade_to - fade_from) / (AUDIO_RATE * fade_ms / 1000.0f) : 0.0f) == 0.0f ? fade_to : fade_from;
}

void play_mp3(const char *path, i32 fade_ms) { i32 old = mp3_slot, next = mp3_slot ? 0 : 1; if (mp3_ch[old].open) { mp3_ch[old].fade_target = 0.0f; mp3_ch[old].fade_step = (fade_ms > 0) ? -mp3_ch[old].fade_vol / (AUDIO_RATE * fade_ms / 1000.0f) : -1.0f; } mp3_open_slot(mp3_slot = next,path,0.0f,1.0f,fade_ms); }
void mp3_clear() { for (i32 i=0;i<2;i++) if (mp3_ch[i].open) { drmp3_uninit(&mp3_ch[i].dec); mp3_ch[i].open=false; } mp3_slot=0; }
ENGINE_TO_MOD void MP3Pause() { mp3_paused = true; }
ENGINE_TO_MOD void MP3Resume() { mp3_paused = false; }
ENGINE_TO_MOD float GetMP3RemainingTime() { mp3_channel_t *m = &mp3_ch[mp3_slot]; return (!m->open || m->frames_decoded >= m->total_frames) ? 0.0f : (!m->total_frames ? 1.0f : (float)(m->total_frames - m->frames_decoded) / (m->src_rate ? m->src_rate : AUDIO_RATE)); }
static FHandle pcm_fds[8]; static i32 pcm_fd_count = 0;
pthread_t audThreadID; void* AudThread(void* arg); 
#ifdef WINDOWS
    void AudioUpdate() {
        if (pcm_fd_count==0) {return;} 
        i16 buf[AUDIO_FRAMES*AUDIO_CHANNELS]; pcm_sync_t sync; if (pcm_sync(pcm_fds[0],&sync) < 0) {return;}
        
        u32 avail = AUDBUF_SIZE - ((sync.control.appl_ptr - sync.status.hw_ptr > AUDBUF_SIZE) ? 0 : sync.control.appl_ptr - sync.status.hw_ptr);
        while (avail>=(u32)AUDIO_FRAMES) { audio_mix_period(buf); for (i32 i=0;i<pcm_fd_count;i++) { if (pcm_write(buf,AUDIO_FRAMES)<0) {pcm_prepare(pcm_fds[i]);} } avail-=AUDIO_FRAMES; }
    }
    
    void InitAudio() { FHandle first = pcm_open_all(AUDIO_RATE,AUDIO_CHANNELS,AUDIO_FRAMES,AUDIO_PERIODS); if (first == INVALID_FHANDLE) { DualLog("ERROR: No WASAPI audio device found\n"); return; } pcm_fds[0] = first; pcm_fd_count = 1; pthread_create(&audThreadID,NULL,AudThread,NULL); }
#else // Linux
    typedef void snd_pcm_t;
    typedef int (*pfnspo)(snd_pcm_t**,const char*,int,int); typedef int (*pfn_snd_pcm_close)(snd_pcm_t*);    typedef int (*pfnspw)(snd_pcm_t*,const void*,u32);
    typedef int (*pfnspr)(snd_pcm_t*,int,int);              typedef int (*pfnspp)(snd_pcm_t*);               typedef int (*pfnsphps)();
    typedef int (*pfnsphpa)(snd_pcm_t*,void*);              typedef int (*pfnsphpsa)(snd_pcm_t*,void*,u32);  typedef int (*pfnsphpsf)(snd_pcm_t*,void*,int);
    typedef int (*pfnsphp)(snd_pcm_t*,void*);               typedef int (*pfnsphpsc)(snd_pcm_t*, void*,u32); typedef int (*pfnsphpsrn)(snd_pcm_t*,void*,u32*,int*);
    typedef int (*pfnsphpspsn)(snd_pcm_t*,void*,u64*,int*); typedef int (*pfnsphpspn)(snd_pcm_t*,void*,u32*,int*); static snd_pcm_t *apcm; static pfnspw snd_pcm_writei; static pfnspr snd_pcm_recover;
    static bool alsa_try_open_default() {
        void *so = dlopen("libasound.so.2",2); if (!so) {so = dlopen("libasound.so",2);} if (!so) { DualLog("Audio: libasound not found\n"); return false; }

        pfnspo spo = dlsym(so,"snd_pcm_open");                              pfnsphpa sphpa = dlsym(so,"snd_pcm_hw_params_any");            pfnsphps sphps = dlsym(so,"snd_pcm_hw_params_sizeof");            pfnsphpsa sphpsa = dlsym(so,"snd_pcm_hw_params_set_access");
        pfnsphpsf sphpsf = dlsym(so,"snd_pcm_hw_params_set_format");        pfnsphpsc sphpsc = dlsym(so,"snd_pcm_hw_params_set_channels"); pfnsphpsrn sphpsrn = dlsym(so,"snd_pcm_hw_params_set_rate_near"); pfnsphpspsn sphpspsn = dlsym(so,"snd_pcm_hw_params_set_period_size_near");
        pfnsphpspn sphpspn= dlsym(so,"snd_pcm_hw_params_set_periods_near"); pfnsphp snd_pcm_hw_params = dlsym(so,"snd_pcm_hw_params");     snd_pcm_writei  = dlsym(so,"snd_pcm_writei");                     snd_pcm_recover = dlsym(so,"snd_pcm_recover"); 
        pfnspp spp = dlsym(so,"snd_pcm_prepare");
        if (!spo || !sphps || !sphpa || !sphpsa || !sphpsf || !sphpsc || !sphpsrn || !sphpspsn || !sphpspn || !snd_pcm_hw_params || !snd_pcm_writei || !snd_pcm_recover || !spp) { DualLogError("Audio: libasound missing required symbols\n"); return false; }

        int r = spo(&apcm,"default",0,0);  if (r < 0 ||                            !apcm) { DualLogError("Audio: snd_pcm_open('default') failed: %d\n",r); return false; }
        int sz = sphps(); u8 hwp_buf[640]; if (sz > (int)sizeof(hwp_buf)                ) { DualLogError("Audio: hw_params_t too large (%d)\n",sz); return false; }
        void *hwp = hwp_buf;               if ((r = sphpa(apcm,hwp))                 < 0) { DualLogError("Audio: hw_params_any failed\n"); return false; }
                                           if ((r = sphpsa(apcm,hwp,3))              < 0) { DualLogError("Audio: set_access failed\n"); return false; }
                                           if ((r = sphpsf(apcm,hwp,2)   )           < 0) { DualLogError("Audio: set_format S16_LE failed\n"); return false; }
                                           if ((r = sphpsc(apcm,hwp,AUDIO_CHANNELS)) < 0) { DualLogError("Audio: set_channels(%d) failed\n",AUDIO_CHANNELS); return false; }
        u32 rate   =AUDIO_RATE; int dir=0; if ((r = sphpsrn(apcm,hwp,&rate,&dir))    < 0) { DualLogError("Audio: set_rate(%u) failed\n", AUDIO_RATE); return false; }
        u64 period =AUDIO_FRAMES;   dir=0; if ((r = sphpspsn(apcm,hwp,&period,&dir)) < 0) { DualLogError("Audio: set_period_size(%d) failed\n", AUDIO_FRAMES); return false; }
        u32 periods=AUDIO_PERIODS;  dir=0; if ((r = sphpspn(apcm,hwp,&periods,&dir)) < 0) { DualLogError("Audio: set_periods(%d) failed\n", AUDIO_PERIODS); return false; }
                                           if ((r = snd_pcm_hw_params(apcm,hwp))     < 0) { DualLogError("Audio: hw_params apply failed\n"); return false; }
                                           if ((r = spp(apcm))                       < 0) { DualLogError("Audio: snd_pcm_prepare failed\n"); return false; }
        return true;
    }

    static void init_pcm_device(i32 card, i32 dev) {
        FHandle r = pcm_open(card,dev,1|(1<<1)); if (r == INVALID_FHANDLE) return;
        
        pcm_params_t p; hw_params_fill(&p.hw_params); pcm_sw_params_t *sw = &p.sw_params; MemSetToVForNBytes(sw,0,sizeof(*sw)); sw->start_threshold = 1; sw->period_step = 1;
        hw_params_set(&p.hw_params,0/*PCM_FORMAT*/,3);                  hw_params_set(&p.hw_params,11/*PCM_RATE*/,AUDIO_RATE); hw_params_set(&p.hw_params,10/*PCM_CHANNELS*/,AUDIO_CHANNELS);
        hw_params_set(&p.hw_params,13/*PCM_PERIOD_SIZE*/,AUDIO_FRAMES); hw_params_set(&p.hw_params,15,AUDIO_PERIODS);
        if (pcm_params_setup(r,&p) >= 0 && pcm_fd_count < 8) pcm_fds[pcm_fd_count++] = r;
        else { DualLogError("Audio: raw device card=%d dev=%d setup failed, closing\n",card,dev); OS_Close(r); }
    }

    void AudioUpdate() { if (!apcm) {return;} i16 buf[AUDIO_FRAMES*AUDIO_CHANNELS]; audio_mix_period(buf); int r = snd_pcm_writei(apcm,buf,(u32)AUDIO_FRAMES); if (r < 0 && snd_pcm_recover(apcm,r,0) >= 0) { snd_pcm_writei(apcm,buf,(u32)AUDIO_FRAMES); } }
    void InitAudio() { if (!alsa_try_open_default()) { for (i32 card = 0; card < 8; card++) { for (i32 dev = 0; dev < 8; dev++) init_pcm_device(card,dev); } if (pcm_fd_count == 0) {DualLogError("Audio: no output device found\n"); return; } } pthread_create(&audThreadID,NULL,AudThread,NULL); }
#endif

void* AudThread(void* arg) { (void)arg; while (1) { AudioUpdate(); OS_USleep(1000); } return NULL; }
// ============== Raycast System
float half_to_float(half h);
RaycastHit RayTriangle(Vector3 origin, Vector3 dir, Vector3 posA, Vector3 posB, Vector3 posC, Vector3 normA, Vector3 normB, Vector3 normC) {
    Vector3 edgeAB = V3_AsubB(posB,posA), edgeAC = V3_AsubB(posC,posA); Vector3 normalVector = V3_Cross(edgeAB,edgeAC);
    Vector3 ao = V3_AsubB(origin,posA); Vector3 dao = V3_Cross(ao,dir);
    float determinant = -V3_dot(dir,normalVector); float invDet = 1.0f / determinant; float dst = V3_dot(ao, normalVector) * invDet;
    float u = V3_dot(edgeAC,dao) * invDet, v = -V3_dot(edgeAB,dao) * invDet; float w = 1.0f - u - v;
    return (RaycastHit){.point=V3_AplusB(origin,V3_ScaleByF(dir,dst)), .normal=V3_Normalize(V3_AplusB(V3_AplusB(V3_ScaleByF(normA,w),V3_ScaleByF(normB,u)),V3_ScaleByF(normC,v))), .distance=dst, .hitInstanceIndex=INSTANCE_COUNT, .hit=vabs(determinant) >= 1E-8f && dst >= 0 && u >= 0 && v >= 0 && w >= 0};
}
 
ENGINE_TO_MOD RaycastHit Raycast(Vector3 origin, Vector3 dir, float maxDist, u32 layerMask) {
    RaycastHit result = { .hit = false, .distance = maxDist, .point = {0.0f, 0.0f, 0.0f}, .normal = {0.0f, 0.0f, 0.0f}, .hitInstanceIndex = INSTANCE_COUNT };
    for (u16 i = START_INDEX_LEVEL_INSTANCES; i < INSTANCE_COUNT; ++i) {
        if (!(layerMask & Sys_Global.instances[i].layer)) continue;
        u16 mindex = Sys_Global.instances[i].modelIndex; if (mindex >= loadedModelsMaxIndex) continue;
        
        Vector3 objPos = Sys_Global.instances[i].position;
        u16 instCellIdx = PosGetCellCoords(objPos.x,objPos.z);
        Vector3 delta = V3_AsubB(objPos,origin);
        float distSqrd = delta.x*delta.x + delta.y*delta.y + delta.z*delta.z, radBounds = vmax(modelBounds[mindex], 1.81f); 
        float maxDistToObj = vmax(maxDist - radBounds,maxDist); if (distSqrd >= (maxDistToObj * maxDistToObj)) continue;
        
        if (!ConstIndexIsPortalBlockingDoor(Sys_Global.instances[i].index)) {
            if (((gridCellStates[instCellIdx] & (CELL_VISIBLE | CELL_OPEN)) == CELL_OPEN) && (Sys_Global.instances[i].index != 754 || !SkyIsVisible())) continue;
        }
        
        u32 triCount = modelTriangleCounts[mindex];
        if (triCount < 1) continue;
        
        float M[16];
        MemCpyFromBtoAForNBytes(M,&modelMatrices[i * 16],16 * sizeof(float));
        float m00=M[0], m10=M[1], m20=M[2];
        float m01=M[4], m11=M[5], m21=M[6];
        float m02=M[8], m12=M[9], m22=M[10];
        float tx=M[12], ty=M[13], tz=M[14];
        float sclx = vsqrtf(m00*m00 + m10*m10 + m20*m20); float sclx2 = sclx * sclx;
        float scly = vsqrtf(m01*m01 + m11*m11 + m21*m21); float scly2 = scly * scly;
        float sclz = vsqrtf(m02*m02 + m12*m12 + m22*m22); float sclz2 = sclz * sclz;
        Vector3 rel = {origin.x - tx, origin.y - ty, origin.z - tz};
        Vector3 localOrigin = {(rel.x*m00 + rel.y*m10 + rel.z*m20) / sclx2, (rel.x*m01 + rel.y*m11 + rel.z*m21) / scly2, (rel.x*m02 + rel.y*m12 + rel.z*m22) / sclz2};
        Vector3 localDir =    {(dir.x*m00 + dir.y*m10 + dir.z*m20) / sclx2, (dir.x*m01 + dir.y*m11 + dir.z*m21) / scly2, (dir.x*m02 + dir.y*m12 + dir.z*m22) / sclz2};
        localDir = V3_Normalize(localDir);
        for (u32 j=0;j<triCount;++j) {
            u32 bA = (u32)modelTriangles[mindex][j*3 + 0] * VERTEX_ATTRIBUTES_SIZE, bB = (u32)modelTriangles[mindex][j*3 + 1] * VERTEX_ATTRIBUTES_SIZE, bC = (u32)modelTriangles[mindex][j*3 + 2] * VERTEX_ATTRIBUTES_SIZE;
            Vector3 posA = {half_to_float( *(half*)(modelVertices[mindex] + bA + 0) ), half_to_float( *(half*)(modelVertices[mindex] + bA + 2) ), half_to_float( *(half*)(modelVertices[mindex] + bA + 4) )};
            Vector3 posB = {half_to_float( *(half*)(modelVertices[mindex] + bB + 0) ), half_to_float( *(half*)(modelVertices[mindex] + bB + 2) ), half_to_float( *(half*)(modelVertices[mindex] + bB + 4) )};
            Vector3 posC = {half_to_float( *(half*)(modelVertices[mindex] + bC + 0) ), half_to_float( *(half*)(modelVertices[mindex] + bC + 2) ), half_to_float( *(half*)(modelVertices[mindex] + bC + 4) )};
            Vector3 normA ={half_to_float( *(half*)(modelVertices[mindex] + bA + 6) ), half_to_float( *(half*)(modelVertices[mindex] + bA + 8) ), half_to_float( *(half*)(modelVertices[mindex] + bA + 10) )};
            Vector3 normB ={half_to_float( *(half*)(modelVertices[mindex] + bB + 6) ), half_to_float( *(half*)(modelVertices[mindex] + bB + 8) ), half_to_float( *(half*)(modelVertices[mindex] + bB + 10) )};
            Vector3 normC ={half_to_float( *(half*)(modelVertices[mindex] + bC + 6) ), half_to_float( *(half*)(modelVertices[mindex] + bC + 8) ), half_to_float( *(half*)(modelVertices[mindex] + bC + 10) )};
            RaycastHit tryTri = RayTriangle(localOrigin,localDir,posA,posB,posC,normA,normB,normC);
            if (!tryTri.hit) continue;
            
            Vector3 worldPoint = { m00*tryTri.point.x + m01*tryTri.point.y + m02*tryTri.point.z + tx, m10*tryTri.point.x + m11*tryTri.point.y + m12*tryTri.point.z + ty, m20*tryTri.point.x + m21*tryTri.point.y + m22*tryTri.point.z + tz };
            Vector3 toHit = V3_AsubB(worldPoint, origin);
            float worldDist = vsqrtf(toHit.x*toHit.x + toHit.y*toHit.y + toHit.z*toHit.z);
            if (worldDist >= result.distance) continue;
            
            Vector3 worldNormal = { (m00/sclx)*tryTri.normal.x + (m01/scly)*tryTri.normal.y + (m02/sclz)*tryTri.normal.z, (m10/sclx)*tryTri.normal.x + (m11/scly)*tryTri.normal.y + (m12/sclz)*tryTri.normal.z, (m20/sclx)*tryTri.normal.x + (m21/scly)*tryTri.normal.y + (m22/sclz)*tryTri.normal.z };
            worldNormal = V3_Normalize(worldNormal);
            result.hit              = true;
            result.point            = worldPoint;
            result.normal           = V3_Normalize(worldNormal);
            result.distance         = worldDist;
            result.hitInstanceIndex = i;
        }
    }
    
    return result;
}
 
ENGINE_TO_MOD void RaycastAll(Vector3 origin, Vector3 dir, float distance, u32 layerMask, RaycastHit* hits, u16 maxCount) { for (int i=0;i<maxCount;++i) {hits[i].hit = false;} (void)origin; (void)dir; (void)distance; (void)layerMask; }
ENGINE_TO_MOD RaycastHit CapsuleCast(Vector3 start, Vector3 end, float capsuleRadius, float castDist, u32 layerMask, bool hitTriggers) { RaycastHit result = { .hit = false }; (void)start; (void)end; (void)capsuleRadius; (void)castDist; (void)layerMask; (void)hitTriggers; return result; }
// ================ Rendering System
#include "Shaders/shaders.h"
static inline __attribute__((always_inline)) void ShaderError(u32 s, const char* name) { char er[512]; glGetShaderInfoLog(s,512,NULL,er); DualLogError("%s Comp Fail: %s\n",name,er); OS_Exit(1); }
static inline __attribute__((always_inline)) u32 CompileShader(u32 type, const char* source, const char* name) { u32 s = glCreateShader(type); glShaderSource(s,1,&source,NULL); glCompileShader(s); i32 ok; glGetShaderiv(s,0x8B81/*GL_COMPILE_STATUS*/,&ok); if (!ok) ShaderError(s,name); return s; }
static inline __attribute__((always_inline)) u32 LinkProgram(u32* s, i32 num, const char* name) { u32 p = glCreateProgram(); for (i32 i=0;i<num;++i) { glAttachShader(p,s[i]); } glLinkProgram(p); i32 ok; glGetProgramiv(p,0x8B82/*GL_LINK_STATUS*/,&ok); if (!ok) ShaderError(p,name); return p; }
u32 CompileAnyShader(const char* v, const char* s, const char* name) { return (v) ? LinkProgram((u32[]){CompileShader(0x8B31/*GL_VERTEX_SHADER*/,v,name),CompileShader(0x8B30/*GL_FRAGMENT_SHADER*/,s,name)},2,name) : LinkProgram((u32[]){CompileShader(0x91B9/*GL_COMPUTE_SHADER*/,s,name)},1,name); }
void CompileShaders() {
    Sys_Render.depthPrepassShaderProgram= CompileAnyShader(depthPrepassVertSrc,depthPrepassFragSrc,"DPre");/*Depth Prepass*/ Sys_Render.chunkShaderProgram          = CompileAnyShader(vertSrc,fragSrc,"Main");
    Sys_Render.uiShaderProgram          = CompileAnyShader(vertUISrc,fragUISrc,"UI");                                        Sys_Render.debugUnlitShaderProgram     = CompileAnyShader(debugUnlitVertSrc,debugUnlitFragSrc,"Ln");/*Line Drawing Unlit*/
    Sys_Render.shadowmapsShaderProgram  = CompileAnyShader(shadowmapVertSrc,shadowmapFragSrc,"Shad");                        Sys_Render.textShaderProgram           = CompileAnyShader(textVertSrc,textFragSrc,"Txt");
    Sys_Render.imageBlitShaderProgram   = CompileAnyShader(quadVertSrc,quadFragSrc,"Comp"); /*Image Blit Composite Pass*/    Sys_Render.ssrShaderProgram            = CompileAnyShader(NULL,ssrComputeSrc,"SSR");
    Sys_Render.voxelUpdateShaderProgram = CompileAnyShader(NULL,voxelUpdateComputeSrc,"Vox"); /*Voxel Update*/               Sys_Render.shadowmapsClearShaderProgram= CompileAnyShader(NULL,shadowmapsClearComputeSrc,"ShadCl"); /*Shadowmaps Clear*/
}

u32 SetupSSBO(u32* id, u32 bindx, size_t sz, const void* d, u32 typ) { glGenBuffers(1,id); glBindBuffer(GL_SSBO,*id); glBufferData(GL_SSBO,sz,d,typ); glBindBufferBase(GL_SSBO,bindx,*id); return *id; }
void mat4_lookat_from(float* m, Quaternion* camRotation, Vector3 eye) { // Kept around for light views for shadowmap cubemap faces.
    float x = camRotation->x, y = camRotation->y, z = camRotation->z, w = camRotation->w;
    float x2 = x * x, y2 = y * y, z2 = z * z;
    float xy = x * y, xz = x * z, yz = y * z;
    float wx = w * x, wy = w * y, wz = w * z;
    Vector3 right   = { 1.0f - 2.0f * (y2 + z2),        2.0f * (xy + wz),        2.0f * (xz - wy) };  // X+ (right)
    Vector3 up      = {        2.0f * (xy - wz), 1.0f - 2.0f * (x2 + z2),        2.0f * (yz + wx) };  // Y+ (up)
    Vector3 forward = {        2.0f * (xz + wy),        2.0f * (yz - wx), 1.0f - 2.0f * (x2 + y2) };  // Z+ (forward)
    m[0]  = right.x;   m[1]  = up.x;   m[2]  = -forward.x;// m[3]  = 0.0f;
    m[4]  = right.y;   m[5]  = up.y;   m[6]  = -forward.y;// m[7]  = 0.0f;
    m[8]  = right.z;   m[9]  = up.z;   m[10] = -forward.z;// m[11] = 0.0f;
    m[12] = -V3_dot(right, eye); m[13] = -V3_dot(up, eye); m[14] = V3_dot(forward, eye); m[15] = 1.0f;
}

__attribute__((pure,always_inline)) bool SphereInFrustum(FrustumPlane* ps, Vector3 c, float radius) { for (int i=0;i<6;++i) { if ((V3_dot(ps[i].normal,c) + ps[i].d) < -radius) return false; } return true; }
void ExtractFrustumPlanes(float* m, FrustumPlane* ps) {
    ps[0].normal.x = m[3] + m[0]; ps[0].normal.y = m[7] + m[4]; ps[0].normal.z = m[11] + m[8];  ps[0].d = m[15] + m[12]; // Left
    ps[1].normal.x = m[3] - m[0]; ps[1].normal.y = m[7] - m[4]; ps[1].normal.z = m[11] - m[8];  ps[1].d = m[15] - m[12]; // Right
    ps[2].normal.x = m[3] + m[1]; ps[2].normal.y = m[7] + m[5]; ps[2].normal.z = m[11] + m[9];  ps[2].d = m[15] + m[13]; // Bottom
    ps[3].normal.x = m[3] - m[1]; ps[3].normal.y = m[7] - m[5]; ps[3].normal.z = m[11] - m[9];  ps[3].d = m[15] - m[13]; // Top
    ps[4].normal.x = m[3] + m[2]; ps[4].normal.y = m[7] + m[6]; ps[4].normal.z = m[11] + m[10]; ps[4].d = m[15] + m[14]; // Near
    ps[5].normal.x = m[3] - m[2]; ps[5].normal.y = m[7] - m[6]; ps[5].normal.z = m[11] - m[10]; ps[5].d = m[15] - m[14]; // Far
    for (int i=0;i<6;i++) {
        float len = V3_Mag(ps[i].normal); if (len > 1e-6f) { ps[i].normal.x /= len; ps[i].normal.y /= len; ps[i].normal.z /= len; ps[i].d /= len; } // Normalize (could use V3_Normalize but need len for d term of FrustumPlane).
    }
}

Vector3 lightsNewPosition[LIGHT_COUNT];
ENGINE_TO_MOD i32 AddLight(Light* lit, LightAnimation* lanim) {
    i32 i = loadedLights; loadedLights++; if (loadedLights >= LIGHT_COUNT) { DualLogError("Too many lights %u added in level %d!\n",i,Sys_Global.curLev); OS_Exit(1); }

    MemCpyFromBtoAForNBytes(&lights[i],lit,sizeof(Light)); MemCpyFromBtoAForNBytes(&lanims[i],lanim,sizeof(LightAnimation));
    lightsNewPosition[i] = lit->pos; flag_set(&lights[i].lflags,LDIRTY,true);
    return i;
}

ENGINE_TO_MOD void TurnLightOff(u16 litIdx) { if (litIdx < loadedLights) {flag_set(&lights[litIdx].lflags,LIGHTON,false);} }
bool alreadyReadLightOnOnce[LIGHT_COUNT] = {0};
ENGINE_TO_MOD void LoadFieldIntoLight(char* k, char* v, char* il, u32 ln, Light* lit, LightAnimation* lam, u16 lIdx) {
    char* br = StringFindFirstCharWithin(k,'[');
    if (br) {
        int i = parse_numberu32(br + 1,il,ln);
        if (i >= 0 && i < 32) { // "intervalSteps[" index 12 is 's', "intervalStepisLerping[" index 12 is 'i'
            if (k[12] == 's') lam->intervalSteps[i] = parse_float(v,il,ln);
            else              lam->stepIsLerping[i] = parse_float(v,il,ln);
        }
        return;
    }

    static const struct { const char* key; u16 offset; u8 type; } map[] = {
        {"currentStep",    __builtin_offsetof(LightAnimation,currentStep),1},{"lerpValue",      __builtin_offsetof(LightAnimation,lerpValue),0},{"intervalSteps.Length",__builtin_offsetof(LightAnimation,numIntervalSteps),1},{"intervalStepisLerping.Length",__builtin_offsetof(LightAnimation, numLerpSteps),1},
        {"localPosition.x",__builtin_offsetof(Light,pos.x),0},               {"localPosition.y",__builtin_offsetof(Light,pos.y),0},             {"localPosition.z",     __builtin_offsetof(Light,pos.z),0},                    {"localRotation.x",             __builtin_offsetof(Light,spotDir.x),0},
        {"localRotation.y",__builtin_offsetof(Light,spotDir.y),0},           {"localRotation.z",__builtin_offsetof(Light,spotDir.z),0},         {"localRotation.w",     __builtin_offsetof(Light,spotDir.w),0},                {"range",                       __builtin_offsetof(Light,range),0},
        {"spotAngle",      __builtin_offsetof(Light,spotAng),0},             {"minIntensity",   __builtin_offsetof(Light,minIntensity),0},      {"maxIntensity",        __builtin_offsetof(Light,maxIntensity),0},             {"color.r",__builtin_offsetof(Light,col.r),0},{"color.g",__builtin_offsetof(Light,col.g),0},{"color.b",__builtin_offsetof(Light,col.b),0}
    };

    for (int i = 0; i < (int)(sizeof(map)/sizeof(map[0])); i++) {
        if (StringsEqual(k, map[i].key)) { // Types: 0 = float, 1 = u8.  Check key prefix to decide if pointing at 'lit' or 'lam'
            void* dest = (k[0] == 'l' && k[1] == 'o') ? (void*)lit : (void*)lam;
            if (k[0] == 'r' || k[0] == 's' || k[0] == 'm' || k[0] == 'c') {
                if (k[1] != 'u') dest = (void*)lit; // range, spot, max, color (not currentStep)
            }
            
            char* ptr = (char*)dest + map[i].offset;
            if (map[i].type == 0) *(float*)ptr = parse_float(v,il,ln);
            else                  *(u8*)ptr = parse_numberu8(v,il,ln);
            return;
        }
    }

    if (StringsEqual(k,"intensity")) lit->intensity = lit->maxIntensity = parse_float(v,il,ln) * 0.35f;
    else if (StringsEqual(k,"type")) flag_set(&lit->lflags, (v[0] == 'S') ? LSPOT : LDIR, true);
    else if (StringsEqual(k,"lightOn") && !alreadyReadLightOnOnce[lIdx]) { alreadyReadLightOnOnce[lIdx] = true; flag_set(&lit->lflags,LIGHTON,parse_bool(v,il,ln)); }
    else if (StringsEqual(k,"lerpOn")) flag_set(&lit->lflags,LERPON,parse_bool(v,il,ln));
}

static inline __attribute__((always_inline)) void mul_mat4(float *out, const float *a, const float *b) { // out = a * b
    out[0] =  a[0] * b[0]  + a[4] * b[1]  + a[8]  * b[2] + a[12]  * b[3]; out[1] =  a[1] * b[0]  + a[5] * b[1]  + a[9]  * b[2] + a[13]  * b[3];
    out[2] =  a[2] * b[0]  + a[6] * b[1] + a[10]  * b[2] + a[14]  * b[3]; out[3] =  a[3] * b[0]  + a[7] * b[1] + a[11]  * b[2] + a[15]  * b[3];
    out[4] =  a[0] * b[4]  + a[4] * b[5]  + a[8]  * b[6] + a[12]  * b[7]; out[5] =  a[1] * b[4]  + a[5] * b[5]  + a[9]  * b[6] + a[13]  * b[7];
    out[6] =  a[2] * b[4]  + a[6] * b[5] + a[10]  * b[6] + a[14]  * b[7]; out[7] =  a[3] * b[4]  + a[7] * b[5] + a[11]  * b[6] + a[15]  * b[7];
    out[8] =  a[0] * b[8]  + a[4] * b[9]  + a[8] * b[10] + a[12] * b[11]; out[9] =  a[1] * b[8]  + a[5] * b[9]  + a[9] * b[10] + a[13] * b[11];
    out[10] = a[2] * b[8]  + a[6] * b[9] + a[10] * b[10] + a[14] * b[11]; out[11] = a[3] * b[8]  + a[7] * b[9] + a[11] * b[10] + a[15] * b[11];
    out[12] = a[0] * b[12] + a[4] * b[13] + a[8] * b[14] + a[12] * b[15]; out[13] = a[1] * b[12] + a[5] * b[13] + a[9] * b[14] + a[13] * b[15];
    out[14] = a[2] * b[12] + a[6] * b[13] + a[10]* b[14] + a[14] * b[15]; out[15] = a[3] * b[12] + a[7] * b[13] + a[11]* b[14] + a[15] * b[15];
}

Quaternion cubeQuats[6] = {{0.0f,ONE_OVER_SQRT2,0.0f,ONE_OVER_SQRT2}/*+X:Right*/,{0.0f,-ONE_OVER_SQRT2,0.0f,ONE_OVER_SQRT2}/*-X:Left*/,{-ONE_OVER_SQRT2,0.0f,0.0f,ONE_OVER_SQRT2}/*+Y:Up*/,{ONE_OVER_SQRT2,0.0f,0.0f,ONE_OVER_SQRT2}/*-Y:Down*/,{0.0f,0.0f,0.0f,1.0f}/*+Z:Forward*/,{0.0f,1.0f,0.0f,0.0f}/*-Z:Backward*/ };
void UpdateLights() {
    for (u16 lightIdx = 0; lightIdx < loadedLights; ++lightIdx) {
        Vector3 lightPos = lightsNewPosition[lightIdx];
        lights[lightIdx].pos = lightPos;
        if (lights[lightIdx].lflags & LDIRTY) { // Marked all as true at level load.
            flag_set(&lights[lightIdx].lflags,LDIRTY,false);
            #pragma GCC unroll 6
            for (int j=0;j<6;++j) { // Update to new position
                mat4_lookat_from((float*)lightView[lightIdx][j],&cubeQuats[j],lightPos);
                mul_mat4((float*)lightViewProj[lightIdx][j],shadowmapsPerspectiveProjection,(float*)lightView[lightIdx][j]);
                ExtractFrustumPlanes((float*)lightViewProj[lightIdx][j],lightFrustumPlanes[lightIdx][j]);
            }
        }
    }
    
    if (!Sys_Global.gamePaused && !Sys_Global.menuActive) {
        for (int i=0;i<loadedLights;++i) { // Just lerps/flickers in intensity
            if (lanims[i].numIntervalSteps < 1) continue;
            if (!(lights[i].lflags & LIGHTON)) { lights[i].intensity = 0.0f; continue; }

            if (lanims[i].lerpTime < (float)Sys_Global.pauseRelativeTime) {
                lights[i].intensity = lanims[i].lerpUp ? lights[i].maxIntensity : lights[i].minIntensity; // Pick target to lerp towards
                lanims[i].lerpUp = !lanims[i].lerpUp;
                lanims[i].currentStep++; if (lanims[i].currentStep >= lanims[i].numIntervalSteps) lanims[i].currentStep = 0; // Wrap and start over continuous looping
                lanims[i].lerpStepTime = lanims[i].intervalSteps[lanims[i].currentStep];
                lanims[i].lerpTime = (float)Sys_Global.pauseRelativeTime + lanims[i].lerpStepTime;
                lanims[i].lerpStartTime = (float)Sys_Global.pauseRelativeTime;
            } else if (lights[i].lflags & LERPON) {
                if (lanims[i].currentStep < lanims[i].numLerpSteps) {
                    if (lanims[i].stepIsLerping[lanims[i].currentStep]) {
                        lanims[i].lerpValue = ((float)Sys_Global.pauseRelativeTime - lanims[i].lerpStartTime)/(lanims[i].lerpTime - lanims[i].lerpStartTime); // percent towards goal time
                        float lerpVal = lanims[i].lerpUp ? lanims[i].lerpValue : (1.0f - lanims[i].lerpValue);
                        lanims[i].lerpValue = lights[i].minIntensity + ((lights[i].maxIntensity - lights[i].minIntensity) * lerpVal);
                        lights[i].intensity = lanims[i].lerpValue;
                    }
                }
            }
        }
    }

    glBindBuffer(GL_SSBO,Sys_Render.lightsID); glBufferData(GL_SSBO,loadedLights * sizeof(Light),lights,GL_DYNAMIC_DRAW);
    glUseProgram(Sys_Render.voxelUpdateShaderProgram);
    glUniform3f(5,Sys_Global.instances[PLAYER1].position.x,Sys_Global.instances[PLAYER1].position.y,Sys_Global.instances[PLAYER1].position.z);
    glDispatchCompute((VOXELS_X+15)/16,(VOXELS_Z+15)/16,1);
}

#define CHGD(a,b) (vabs((a) - (b)) > 0.0001f)
ENGINE_TO_MOD void UpdateLight(u16 i, Vector3 pos, Color3 col, float range, float intensity, float max, float min, float spotAng, Quaternion spotDir, bool on, bool shad) {
    bool changed = ((!!(lights[i].lflags & SHADON) - shad) || (!!(lights[i].lflags & LIGHTON) -  on) || CHGD(lights[i].range,range) || CHGD(lights[i].pos.x,pos.x) || CHGD(lights[i].pos.y,pos.y) || CHGD(lights[i].pos.z,pos.z));
    lights[i].intensity=intensity; lights[i].minIntensity=min; lights[i].maxIntensity=max; lights[i].spotAng=spotAng; lights[i].spotDir=spotDir; lights[i].col=col; lights[i].pos=lightsNewPosition[i]=pos; lights[i].range=range;
    flag_set(&lights[i].lflags,19,(lights[i].lflags&LDIRTY)|changed<<4|on|shad<<1);
}
#undef CHGD

void RenderUIImage(i16 x, i16 y, i16 width, i16 height, u32 texIndex) {
    glUseProgram(Sys_Render.uiShaderProgram);
    glDisable(GL_BLEND);
    glBindVertexArray(Sys_Render.textVAO);
    glUniform1ui(0,texIndex);
    glBindBuffer(GL_ARRAY_BUFFER,Sys_Render.textVBO);
    float x1=x + width, y1=y + height, z=0.0f;
    float vertices[30] = {x,y1,z,0.0f,0.0f,x1,y,z,1.0f,1.0f,x1,y1,z,1.0f,0.0f,x,y1,z,0.0f,0.0f,x,y,z,0.0f,1.0f,x1,y,z,1.0f,1.0f};
    glBufferData(GL_ARRAY_BUFFER,30 * sizeof(float),vertices,GL_DYNAMIC_DRAW);
    glDrawArrays(0x0004/*GL_TRIANGLES*/,0,6);
    drawCalls++; uiDrawCalls++; vertsRendered += 6;    
    glBindBuffer(GL_ARRAY_BUFFER,0);
}

void ClearAll() {
    glBindFramebuffer(GL_FRAMEBUFFER,Sys_Render.gBufferFBO); glClearColor(0.0f,0.0f,0.0f,1.0f); glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER,Sys_Render.uiFBO);      glClearColor(0.0f,0.0f,0.0f,0.0f); glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER,0);                     glClearColor(0.0f,0.0f,0.0f,1.0f); glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

void RenderLoadingProgress(i32 offset, const char * restrict text) { ClearAll(); glViewport(0,0,Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight); RenderFormattedText(683 - offset,379,TEXT_WHITE,FONT_NORMAL,1.0f,text); ((_GLFWwindow*)window)->context.swapBuffers(((_GLFWwindow*)window)); }
// ===================================== Configuration Options Settings System
char statusText[TEXT_BUFFER_SIZE];
void CenterStatusPrint(const char * restrict fmt, ...) { va_list args; __builtin_va_start(args, fmt); StringFormatV(statusText,TEXT_BUFFER_SIZE,fmt,args); __builtin_va_end(args); DualLog("%s\n",statusText); Sys_Global.statusTextDecayFinished = get_time() + 3.5;/*secs decay time before text dissappears.*/ }
typedef enum { SETTING_U8, SETTING_U16, SETTING_INPUT } SettingType; typedef struct { const char* name; void* ptr; SettingType type; } Setting;
#define S_U8(n, v)  { n, &Sys_Settings.v, SETTING_U8 }
#define S_U16(n, v) { n, &Sys_Settings.v, SETTING_U16 }
#define S_IN(n, i)  { n, &Sys_Settings.InputCodeSettings[i], SETTING_INPUT }
const Setting configTable[] = {
    S_U16("ResolutionWidth",ScreenWidth),S_U16("ResolutionHeight",ScreenHeight),S_U8("Fullscreen",Fullscreen),      S_U8("FOV",FOV),                     S_U8("Brightness",Brightness),
    S_U8("Gamma",Gamma),S_U8("AA",FXAA),  S_U8("Shadows",Shadows),              S_U8("SSR",Reflections),            S_U8("VSync",Vsync),                 S_U8("ModelDetail",ModelDetail),
    S_U8("GI",GI),                        S_U8("SpeakerMode",SpeakerMode),      S_U8("Reverb",Reverb),              S_U8("VolumeMaster",VolumeMaster),   S_U8("VolumeMusic",VolumeMusic),
    S_U8("VolumeMessage",VolumeMessage),  S_U8("VolumeEffects",VolumeEffects),  S_U8("Language",Language),          S_U8("DynamicMusic",DynamicMusic),   S_U8("Footsteps",Footsteps),
    S_U8("InvertLook",InvertLook),        S_U8("Monitor",CurrentMonitor),       
    S_U8("InvertCyberspaceLook",InvertCyberspaceLook),  S_U8("InvertInventoryCycling",InvertInventoryCycling),S_U8("QuickItemPickup",QuickItemPickup),
    S_U8("QuickReloadWeapons",QuickReloadWeapons),      S_U8("MouseSensitivity",MouseSensitivity),            S_U8("NoShootMode",NoShootMode),           S_U8("HeadBob",HeadBob),
    S_IN("Forward",0),    S_IN("Strafe Left",1),S_IN("Backpedal",2), S_IN("Strafe Right",3),S_IN("Jump",4),        S_IN("Crouch",5),    S_IN("Prone",6),       S_IN("Lean Left",7),
    S_IN("Lean Right",8), S_IN("Sprint",9),     S_IN("Turn Left",10),S_IN("Turn Right",11), S_IN("Look Up",12),    S_IN("Look Down",13),S_IN("Recent Log",14),
    S_IN("Biomonitor",15),S_IN("Sensaround",16),S_IN("Lantern",17),  S_IN("Shield",18),     S_IN("Infrared",19),   S_IN("Email",20),    S_IN("Booster",21),
    S_IN("Jumpjets",22),  S_IN("Attack",23),    S_IN("Use",24),      S_IN("Menu/Back",25),  S_IN("Toggle Mode",26),S_IN("Reload",27),
    S_IN("Weapon +",28),  S_IN("Weapon -",29),  S_IN("Grenade",30),  S_IN("Grenade +",31),  S_IN("Grenade -",32),  S_IN("Ammo Type",33),S_IN("Patch Use",34),
    S_IN("Patch +",35),   S_IN("Patch -",36),   S_IN("Full Map",37), S_IN("Swim Up",38),    S_IN("Swim Down",39),  S_IN("Screenshot",40)
};

const int configTableSize = sizeof(configTable) / sizeof(Setting);
static inline __attribute__((always_inline)) i32 GetGLFWIndirectionIndexForAnInput(const char* val) { for (int i=0;i<134;++i) {if (StringsEqual(val,inputElements[i].name)) return i;} return 148; }
void LoadConfig() {
    FHandle f = OS_OpenReadonly("./Data/Config.ini");
    char line[512];
    while (GetNextStringUpToNewlineOrEOF(line,sizeof(line),f)) {
        char* s = data_parser_trim(line); if (*s == 0 || (s[0] == '/' && s[1] == '/')) continue;
        char* eq = StringFindFirstCharWithin(s, '='); if (!eq) continue;
        
        *eq = 0; char *key = data_parser_trim(s), *val = data_parser_trim(eq + 1);
        for (int i = 0; i < configTableSize; i++) {
            if (StringsEqual(key,configTable[i].name)) {
                if (configTable[i].type == SETTING_U8)         *( u8*)configTable[i].ptr = (u8)StringToInt(val);
                else if (configTable[i].type == SETTING_U16)   *(u16*)configTable[i].ptr = (u16)StringToInt(val);
                else if (configTable[i].type == SETTING_INPUT) *(u16*)configTable[i].ptr = GetGLFWIndirectionIndexForAnInput(val);
                break;
            }
        }
    }

    Sys_Settings.ScreenWidth = vmax(Sys_Settings.ScreenWidth,320);
    Sys_Settings.ScreenHeight = vmax(Sys_Settings.ScreenHeight,200);
    OS_Close(f);
}

void FilePrintString(FHandle f, const char* fmt, ...) { va_list a; __builtin_va_start(a,fmt); char b[128]; va_list c; __builtin_va_copy(c,a); StringFormatV(b,sizeof(b),fmt,c); __builtin_va_end(c); OS_RawWrite(f,b,GetStringLength(b)); __builtin_va_end(a); }
void SaveConfig() {
    FHandle f = OS_OpenWriteonly("./Data/Config.ini");
    for (int i=0;i<configTableSize;++i) {
        if (configTable[i].type == SETTING_U8)         FilePrintString(f,"%s = %u\n",configTable[i].name,*(u8*)configTable[i].ptr);
        else if (configTable[i].type == SETTING_U16)   FilePrintString(f,"%s = %u\n",configTable[i].name,*(u16*)configTable[i].ptr);
        else if (configTable[i].type == SETTING_INPUT) FilePrintString(f,"%s = %s\n",configTable[i].name,inputElements[*(u16*)configTable[i].ptr].name);
    }

    OS_Close(f);
    DualLog("Saved settings to ./Data/Config.ini! framenum %u\n",Sys_Global.frame);
}
// ============== Credits System
char creditStats[4096]; const char** creditPages = NULL;
static inline __attribute__((always_inline)) float GetScore(float stupid, bool isFinal) {
    float victories = (float)(Sys_Global.kills + Sys_Global.cyberkills);
    if (isFinal) victories -= vmin(Sys_Global.ressurections * 10.0f, victories * 0.666f);
    float secs  = vfloor((float)Sys_Global.pauseRelativeTime / 3600.0f);
    float score = victories * 10000.0f;
    score -= vmin(score * 0.666f, secs * 100.0f);
    score *= (stupid + 1.0f) / 37.0f;
    if (stupid > 35.0f) score += 2222222.0f;
    return vfloor(score);
}

static inline __attribute__((always_inline)) void DecomposeTime(double t, u32* h, u32* m, double* s) { double tb = vfloor(t / 3600.0); *h = (u32)tb; t -= tb * 3600.0; tb = vfloor(t / 60.0); *m = (u32)tb; *s = t - tb * 60.0; }
static inline __attribute__((always_inline)) void CreditsStats() {
    size_t off = 0;
    off += StringFormat(creditStats + off, sizeof(creditStats),"============================================================================\nCITADEL\n============================================================================\nCONGRATULATIONS %s\n",Sys_Global.playerName);
    u32 h,m; double s;
    DecomposeTime(Sys_Global.pauseRelativeTime,&h,&m,&s);
    off += StringFormat(creditStats + off, sizeof(creditStats),"Straight Time: %uh %um %.3fs\n",h,m,s);
    DecomposeTime(Sys_Global.absoluteTime,&h,&m,&s);
    off += StringFormat(creditStats + off,sizeof(creditStats),"Total Time (with reload from deaths): %uh %um %.3fs\n",h,m,s);
    float stupid = ((float)(Sys_Global.difficultyCombat * Sys_Global.difficultyCombat)) + ((float)(Sys_Global.difficultyPuzzle * Sys_Global.difficultyPuzzle)) + ((float)(Sys_Global.difficultyMission * Sys_Global.difficultyMission)) + ((float)(Sys_Global.difficultyCyber * Sys_Global.difficultyCyber));
    u32 finalSubscore = GetScore(stupid,false), finalScore = (u32)GetScore(stupid,true);
    off += StringFormat(creditStats + off,sizeof(creditStats),"Kills: %u\nKills in Cyberspace: %u\nScoreSubtotal: %u\nDeaths: %u\nRessurections: %u\n",Sys_Global.kills,Sys_Global.cyberkills,(u32)finalSubscore,Sys_Global.deaths,Sys_Global.ressurections);
    off += StringFormat(creditStats + off,sizeof(creditStats),"Combat: %u | Puzzle: %u | Mission: %u | Cyber: %u\n",Sys_Global.difficultyCombat,Sys_Global.difficultyPuzzle,Sys_Global.difficultyMission,Sys_Global.difficultyCyber);
    off += StringFormat(creditStats + off,sizeof(creditStats),"Difficulty Index: %.2f\nFinal Score: %u\n\n",stupid,finalScore);
    off += StringFormat(creditStats + off,sizeof(creditStats),"Shots Fired: %u\nGrenades Thrown: %u\n",Sys_Global.shotsFired,Sys_Global.grenadesThrown);
    off += StringFormat(creditStats + off,sizeof(creditStats),"Damage Dealt: %f\nDamage Received: %f\nSaves Scummed: %u\n\nClick to continue...\n",Sys_Global.damageDealt,Sys_Global.damageReceived,Sys_Global.savesScummed);
}
// ================ Rendering System
static __attribute__((noinline)) void GenerateAndBindTexture(u32 *id, i32 internalFormat, i32 width, i32 height, u32 format, u32 type, i32 filt, unsigned char* bmp) {
    if (*id == 0) {glGenTextures(1,id);} glBindTexture(GL_TEXTURE_2D,*id);
    glTexImage2D(GL_TEXTURE_2D,0,internalFormat,width,height,0,format,type,bmp);
    glTexParameteri(GL_TEXTURE_2D,0x2801/*GL_TEXTURE_MIN_FILTER*/,filt); glTexParameteri(GL_TEXTURE_2D,0x2800/*GL_TEXTURE_MAG_FILTER*/,filt);
}
static void GenBTexture(u32 *id, i32 internalFormat, i32 width, i32 height, u32 format, u32 type, i32 filt) { GenerateAndBindTexture(id,internalFormat,width,height,format,type,filt,NULL); }
void UpdateScreenSize(i32 width, i32 height) {
    u16 w = Sys_Settings.ScreenWidth = vmax(vmin((u16)width,7680u),320u), h = Sys_Settings.ScreenHeight = vmax(vmin((u16)height,4320u),200u); // Cap at minimum Quake resolution and maximum 8k.
    float wf = (float)w, hf = (float)h; Sys_Settings.ScreenCenterX = wf * 0.5f; Sys_Settings.ScreenCenterY = hf * 0.5f;
    glViewport(0,0,w,h); RenderSystem* rs = &Sys_Render;
    glUseProgram(rs->imageBlitShaderProgram); glUniform1ui(2,w); glUniform1ui(3,h); glUniform1i(26,Sys_Settings.SSR_RES);
    glUseProgram(rs->chunkShaderProgram); glUniform1ui(6,w); glUniform1ui(7,h);
    glUseProgram(rs->ssrShaderProgram); glUniform1ui(0,w / Sys_Settings.SSR_RES); glUniform1ui(1,h / Sys_Settings.SSR_RES); glUniform1i(2,Sys_Settings.SSR_RES);
    GenBTexture(&rs->inputImageID,     GL_RGBA8,w,h,GL_RGBA,GL_UNSIGNED_BYTE,0x2600/*GL_NEAREST*/); // Lit Raster
    GenBTexture(&rs->inputSpecID,      GL_RGBA8,w,h,GL_RGBA,GL_UNSIGNED_BYTE,0x2600/*GL_NEAREST*/); // Specular Colors
    GenBTexture(&rs->inputNormalID,    GL_RG16F,w,h, GL_RGB,        GL_FLOAT,0x2600/*GL_NEAREST*/); // Normal XYZ
    GenBTexture(&rs->inputDepthID,0x81A7/*GL_DEPTH_COMPONENT32*/,w,h,0x1902/*GL_DEPTH_COMPONENT*/,GL_FLOAT,0x2600/*GL_NEAREST*/); // Raster Depth
    GenBTexture(&rs->outputImageID,GL_RGBA8,w / Sys_Settings.SSR_RES,h / Sys_Settings.SSR_RES,GL_RGBA,GL_UNSIGNED_BYTE,0x2601/*GL_LINEAR*/);
    glBindFramebuffer(GL_FRAMEBUFFER,rs->gBufferFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,rs->inputImageID,0);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT1,GL_TEXTURE_2D,rs->inputSpecID,0);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT2,GL_TEXTURE_2D,rs->inputNormalID,0);
    glFramebufferTexture2D(GL_FRAMEBUFFER,0x8D00/*GL_DEPTH_ATTACHMENT*/,GL_TEXTURE_2D,rs->inputDepthID,0);
    glBindImageTexture(0,rs->inputImageID,0,GL_FALSE,0,GL_READ_WRITE,GL_RGBA8);      // Main Rendered Color
    glBindImageTexture(2,rs->inputSpecID,0,GL_FALSE,0,GL_READ_WRITE,GL_RGBA8);       // Specular
    glBindImageTexture(4,rs->outputImageID,0,GL_FALSE,0,GL_READ_WRITE,GL_RGBA8);     // SSR result
    glBindImageTexture(5,rs->inputNormalID,0,GL_FALSE,0,GL_READ_WRITE,GL_RG16F);     // Normal XYZ
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D,rs->outputImageID);
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    Sys_Input.ignore_next_mouse_delta = true;
}

ENGINE_TO_MOD void AddCamView(Vector3 pos, Quaternion rot, u8 fov, u16 width, u16 height, float near, float far) {    
    camViews[camViewCount] = (CamView){pos,rot,fov,width,height,near,far,Sys_Global.pauseRelativeTime + (camViewCount * 0.05f) + 0.5f,false}; // Staggered starts so not all at once for performance.
    GenBTexture(&camViewTextures[camViewCount],GL_RGBA8,width,height,GL_RGBA,GL_UNSIGNED_BYTE,0x2600/*GL_NEAREST*/); camViewCount++;
}

void SetGI() { }// TODO: Set needed Voxel GI uniforms from Sys_Settings.GI
void SetLanguage() { LoadTextForLanguage(Sys_Settings.Language); LoadLogTextForLanguage(Sys_Settings.Language); }
void ApplySettings() { ChangeFullScreenWindowed(); SetSkyRotateSpeed(); SetVSync(); SetGI(); SetLanguage(); }
void OpenMainMenu() { PlayMenuMusic(); Sys_Global.menuActive = true; currentMenuPage = Mpg_FrontPage; }
bool MenuEnter() { return (Sys_Input.keyStates[KEY_KP_ENTER].pressed || Sys_Input.keyStates[KEY_ENTER].pressed); }
static inline __attribute__((always_inline,pure)) bool CursorIsOverBounds(float startX, float endX, float startY, float endY) {
    return    Sys_Global.cursorPosition_x >= startX && Sys_Global.cursorPosition_x <= endX  /* 0 == left */
           && Sys_Global.cursorPosition_y >= startY && Sys_Global.cursorPosition_y <= endY; /* 0 ==  top */
}

u8 UI_Interactable(i16 x, i16 y, float w, float h, bool* cursorOver, i8 this, bool sustained) {
    bool cursorIsOver = CursorIsOverBounds(x, x + w, (float)y - h, (float)y);
    if (cursorIsOver && mouseMovementThisFrame) { currentMenuItem = this; if (cursorOver != NULL) {*cursorOver = cursorIsOver;} }
    if ((sustained ? Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT ].down : Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT ].pressed) && cursorIsOver) return 1u;
    if ((sustained ? Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].down : Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].pressed) && cursorIsOver) return 2u;
    return 0u;
}

u8 UI_Button(i16 x, i16 y, float w, float h, bool* cursorOver, i8 this) { return UI_Interactable(x,y,w,h,cursorOver,this,false); }
bool AnyLeftRightMouseDown() { return (Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].down || Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].down); }
bool UI_Slider(i16 x, i16 y, i16 w, i16 h, i16 sliderPos, i16 xPosForLabel, u8 currentValue, u8* out, bool* sliderActive, u8 min, u8 max, u8 step, u8 mindex, u16 lingdex) {
    bool over=false,changed=false; *out = currentValue;
    RenderUIImage(x,y, w,h, 1079); // Slider background
    RenderUIImage(x + sliderPos,y, h,h,1078); // Slider handle
    if (UI_Interactable(xPosForLabel,y,xPosForLabel + w,h,&over,mindex,true)) *sliderActive = true;
    if (*sliderActive && Sys_Input.currentMouse_dx != 0) {
        i32 new = (i32)currentValue + vmin(vmax(Sys_Input.currentMouse_dx,-1),1); *out = (u8)vmin(vmax(new,min),max); if (*out != currentValue) {changed = true;}
    }
    
    if (!AnyLeftRightMouseDown()) { if (*sliderActive) { *sliderActive = false; SaveConfig(); } }
    if (MenuEnter() && currentMenuItem == mindex) {
        bool shiftHeld = Sys_Input.keyStates[KEY_LEFT_SHIFT].down || Sys_Input.keyStates[KEY_RIGHT_SHIFT].down;
        if (shiftHeld) *out = *out <=  ((min + step) - 1) ? max : *out - step;
        else           *out = *out >= ((max - step) + 1) ?  min : *out + step;
        changed = true;
    }
    
    over = over || currentMenuItem == mindex;
    RenderFormattedText(xPosForLabel,y,over ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,"%s %u",Sys_Text.stringTable[lingdex],*out);
    return changed;
}

u8 UI_MenuButton(i16 bX, i16 bY, u8 menuItem, i16 bW, i16 bH,  i16 tX, i16 tY, const char* text, i16 pX, i16 pY) {
    bool over = false; u8 retvalue = 0u;
    retvalue = UI_Button(bX,bY,bW,bH,&over,menuItem); if (!retvalue) retvalue = (MenuEnter() && currentMenuItem == menuItem);
    over = over || currentMenuItem == menuItem;
    RenderFormattedText(tX,tY,over ? TEXT_STOPD_RED : TEXT_RED_MENU,FONT_STOPD,1.5f,text);
    RenderUIImage(pX,pY, 40,40,over ? 1029 : 1028); // Menu pad
    return retvalue;
}

bool UI_Checkbox(i16 x, i16 y, i8 mitem, u16 textIdx, bool currentlyOn) {
    RenderUIImage(x,y,16,16,910); // Checkbox background
    bool over = false; bool changed = (UI_Button(x,y + 16,210,16,&over,mitem) || (MenuEnter() && currentMenuItem == mitem)); over = over || currentMenuItem == mitem;
    if (currentlyOn) RenderUIImage(x + 2,y + 2, 12,12, 912); // Checkbox check
    RenderFormattedText(x + 20,y,over ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,Sys_Text.stringTable[textIdx]);
    return changed;
}

void UI_HeaderText(i16 x, const char* text) { RenderFormattedText(x,50,TEXT_GREEN_MENU_SHADOW,FONT_STOPD,1.75f,text); RenderFormattedText(x,46,TEXT_GREEN_MENU_GLOW,FONT_STOPD,1.75f,text); RenderFormattedText(x,48,TEXT_GREEN_MENU,FONT_STOPD,1.75f,text); }
ENGINE_TO_MOD void MenuGoBack() {
    if (returnToPause) { returnToPause = false; Sys_Global.gamePaused = true; Sys_Global.menuActive = false; PlayGameMusic(); }
    if (currentMenuPage == Mpg_Singleplayer || currentMenuPage == Mpg_Multiplayer || currentMenuPage == Mpg_Options) currentMenuPage = Mpg_FrontPage;//News
    else if (currentMenuPage == Mpg_Load || currentMenuPage == Mpg_NewGame || currentMenuPage == Mpg_IntroVideo || currentMenuPage == Mpg_CreditsVideo) currentMenuPage = Mpg_Singleplayer;
}

void CreateShadowBuffers() {
    Sys_Render.shadowMapSSBO           = SetupSSBO(&Sys_Render.shadowMapSSBO,           5,(MAX_SHADOWMAPS * (SHADOW_MAP_SIZE * SHADOW_MAP_SIZE * 6U)) * sizeof(u32), NULL, GL_STATIC_DRAW);    
    Sys_Render.shadowMapsIndirectionID = SetupSSBO(&Sys_Render.shadowMapsIndirectionID, 6,LIGHT_COUNT * sizeof(u32),NULL,GL_STATIC_DRAW); shadowBuffersCreated = true;
}

void ChangeMenuPage(u8 pg) { currentMenuPage = pg; currentMenuItem = currentMenuTab = 0; }
void RenderMenu() {    
    if (currentMenuPage != Mpg_IntroVideo && currentMenuPage != Mpg_CreditsVideo && currentMenuPage != Mpg_Options) RenderUIImage(-417,-384, 2200,1536, 1026); // Menu background
    if (currentMenuPage == Mpg_IntroVideo || currentMenuPage == Mpg_CreditsVideo) RenderUIImage(-417,-384, 2200,1536, 0); // Video blackground
    if (currentMenuPage == Mpg_Options) RenderUIImage(-417,-384, 2200,1536, 1032); // Menu background
    if (currentMenuPage == Mpg_FrontPage) {
        menuItemCount = 4; menuTabCount = 1;
        RenderUIImage(282,46, 800,128, 1031); // Title CITADEL with strikethrough effect
        if (UI_MenuButton(408,340, 0, 574,84, 304,188,/*"SINGLEPLAYER"*/Sys_Text.stringTable[719],413,276)) ChangeMenuPage(Mpg_Singleplayer);
        if (UI_MenuButton(408,458, 1, 574,84, 304,268,/*"MULTIPLAYER"*/Sys_Text.stringTable[720], 413,396)) ChangeMenuPage(Mpg_Multiplayer);
        if (UI_MenuButton(408,582, 2, 574,84, 304,350,/*"OPTIONS"*/Sys_Text.stringTable[721],     413,520)) ChangeMenuPage(Mpg_Options);
        if (UI_MenuButton(408,702, 3, 574,84, 304,430,/*"QUIT"*/Sys_Text.stringTable[722],        413,638)) OS_Exit(0);
    } else if (currentMenuPage == Mpg_Singleplayer) {
        menuItemCount = 5; menuTabCount = 1;
        UI_HeaderText(250,/*"SINGLEPLAYER"*/Sys_Text.stringTable[719]);
        if (UI_MenuButton(408,340,0,574,84, 304,188,/*"CONTINUE"*/Sys_Text.stringTable[723],    413,276)) ChangeMenuPage(Mpg_Load);
        if (UI_MenuButton(408,458,1,574,84, 304,268,/*"NEW GAME"*/Sys_Text.stringTable[741],    413,396)) ChangeMenuPage(Mpg_NewGame);
        if (UI_MenuButton(408,582,2,574,84, 304,350,/*"PLAY INTRO"*/Sys_Text.stringTable[742],  413,520)) ChangeMenuPage(Mpg_IntroVideo);
        if (UI_MenuButton(408,702,3,574,84, 304,430,/*"PLAY CREDITS"*/Sys_Text.stringTable[743],413,638)) ChangeMenuPage(Mpg_CreditsVideo);
        RenderUIImage(1060,724, 84,36, 1252); // Back Button background
        bool overBack = false;        
        if (UI_Button(1060,758, 84,32, &overBack, 4) || (MenuEnter() && currentMenuItem == 4)) MenuGoBack();
        overBack = overBack || currentMenuItem == 4;
        RenderFormattedText(1076,732,overBack ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_RED_MENU,FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == Mpg_Multiplayer) {
        menuItemCount = 1; menuTabCount = 1;
        UI_HeaderText(266,/*"MULTIPLAYER"*/Sys_Text.stringTable[720]);
        RenderUIImage(1060,724, 84,36, 1252); // Back Button background
        bool overBack = false;
        if (UI_Button(1060,758, 84,32, &overBack, 0) || (MenuEnter() && currentMenuItem == 0)) MenuGoBack();
        overBack = overBack || currentMenuItem == 0;
        RenderFormattedText(1076,732,overBack ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_RED_MENU,FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == Mpg_Options) {
        menuTabCount = 3;
        UI_HeaderText(238,/*"CONFIGURATION"*/Sys_Text.stringTable[745]);
        if (currentMenuTab != 0) RenderUIImage(179,220, 1001,548, 1030); // Config background
        if (currentMenuTab == 0) RenderUIImage(179,220, 1001,548, 1033); // Config background graphics (empty alpha center)
        RenderUIImage(520,196, 160,30, currentMenuTab == 2 ? 920 : 921); // Config tab unhighlighted
        if (UI_Button(520,196+30, 160,30, NULL, 2)) currentMenuTab = 2;
        RenderFormattedText(530,202,currentMenuTab == 2 ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,/*"AUDIO / LANG"*/Sys_Text.stringTable[793]);
        RenderUIImage(354,196, 160,30, currentMenuTab == 1 ? 920 : 921); // Config tab unhighlighted
        if (UI_Button(354,196+30, 160,30, NULL, 1)) currentMenuTab = 1;
        RenderFormattedText(366,202,currentMenuTab == 1 ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,/*"INPUT"*/Sys_Text.stringTable[792]);
        RenderUIImage(190,196, 160,30, currentMenuTab == 0 ? 920 : 921); // Config tab highlighted
        if (UI_Button(190,196+30, 160,30, NULL, 0)) currentMenuTab = 0;
        RenderFormattedText(200,202,currentMenuTab == 0 ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,/*"GRAPHICS"*/Sys_Text.stringTable[791]);
        if (currentMenuTab == 0) {
            bool overRes = false, overFull = false, overChgM = false;
            menuItemCount = 11; // Graphics
            if (UI_Checkbox(200,500,0,Sys_Settings.ModelDetail ? /*High*/915 : /*No Detail Level Models*/914,Sys_Settings.ModelDetail)) { Sys_Settings.ModelDetail = Sys_Settings.ModelDetail ? 0u : 1u; SaveConfig(); }
            if (UI_Checkbox(200,530,1,/*"FXAA"*/780,Sys_Settings.FXAA)) { Sys_Settings.FXAA = Sys_Settings.FXAA ? 0u : 1u; SaveConfig(); }
            if (UI_Checkbox(200,560,2,Sys_Settings.Shadows ? /*Soft*/787 : /*No Shadows*/785,Sys_Settings.Shadows)) { Sys_Settings.Shadows = Sys_Settings.Shadows ? 0u : 1u; if (!shadowBuffersCreated) {CreateShadowBuffers();} SaveConfig(); }
            if (UI_Checkbox(200,590,3,/*SSR*/788,Sys_Settings.Reflections)) { Sys_Settings.Reflections = Sys_Settings.Reflections ? 0u : 1u; SaveConfig(); }
            if (UI_Checkbox(200,620,4,/*VSYNC*/1026,Sys_Settings.Vsync)) { Sys_Settings.Vsync = Sys_Settings.Vsync ? 0u : 1u; SetVSync(); SaveConfig(); }
            RenderFormattedText(310,620,TEXT_GREEN,FONT_NORMAL,1.0f,"(FPS: %d)", Sys_Global.framesPerLastSecond); // Helper to see vsync take effect.
            u8 newVal;
            if (UI_Slider(400,650,128,16,(((Sys_Settings.FOV - 45.0f) / 105.0f) * (128 - 16)),200,Sys_Settings.FOV,&newVal,&fovSliderActive,45,150,5,5,/*Field of View*/775)) { Sys_Settings.FOV = newVal; if (!AnyLeftRightMouseDown()) {SaveConfig();} }
            if (UI_Slider(400,680,128,16,((Sys_Settings.Brightness / 100.0f) * (128 - 16)),200,Sys_Settings.Brightness,&newVal,&gammaSliderActive,0,100,2,6,/*Gamma*/774)) { Sys_Settings.Brightness = newVal; if (!AnyLeftRightMouseDown()) {SaveConfig();} }
            
            // Resolution
            {
                // Header hit area - UI_Button subtracts h from y internally, so pass y+h as y
                if (UI_Button(190,726,328,16,&overRes,7) || (MenuEnter() && currentMenuItem == 7)) { resDropdownOpen = !resDropdownOpen; currentMenuItem = 7; }
                overRes = overRes || currentMenuItem == 7;
                char resBuf[32];
                if (resDropdownCount > 0) StringFormat(resBuf, sizeof(resBuf), "%ux%u",(u32)resModes[resSelectedIdx].w,(u32)resModes[resSelectedIdx].h);
                else StringFormat(resBuf, sizeof(resBuf), "%ux%u",Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight);

                RenderUIImage(476, 710, 16, 16, overRes ? 1119 : 1077);
                RenderFormattedText(200, 710, overRes ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL, 1.0f, "RESOLUTION %s", resBuf);
            }
    
            // Fullscreen checkbox
            RenderUIImage(200,740, 16,16, 910); // Checkbox background
            if (UI_Button(200,756, 210,16, &overFull, 8) || (MenuEnter() && currentMenuItem == 8)) { Sys_Settings.Fullscreen = Sys_Settings.Fullscreen == 1u ? 0u : 1u; ChangeFullScreenWindowed(); SaveConfig(); }
            overFull = overFull || currentMenuItem == 8;
            if (Sys_Settings.Fullscreen) RenderUIImage(202,742, 12,12, 912); // Checkbox check
            RenderFormattedText(220,740,overFull ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,/*"Fullscreen"*/Sys_Text.stringTable[773]);
            RenderUIImage(588,730, 210,30, 1079); // Toggle monitor button background
            if (UI_Button(588,760, 210,30, &overChgM, 9) || (MenuEnter() && currentMenuItem == 9)) { CycleToNextMonitor(); }
            overChgM = overChgM || currentMenuItem == 9;
            RenderFormattedText(602,735,overChgM ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,/*"CHANGE MONITOR"*/Sys_Text.stringTable[1025]);
        } else if (currentMenuTab == 1) {
            menuItemCount = 49; // Input
        } else {
            menuItemCount = 10; // Audio / Lang
            u8 newVal;
            if (UI_Slider(426,240,128,16,((Sys_Settings.VolumeMaster / 100.0f) * (128 - 16)),200,Sys_Settings.VolumeMaster,&newVal,&masterVolumeSliderActive,0,100,5,0,/*Master Volume*/802)) { Sys_Settings.VolumeMaster = newVal; if (!AnyLeftRightMouseDown()) {SaveConfig();} }
            if (UI_Slider(426,270,128,16,((Sys_Settings.VolumeMusic / 100.0f) * (128 - 16)),200,Sys_Settings.VolumeMusic,&newVal,&musicVolumeSliderActive,0,100,5,1,/*Music Volume*/803)) { Sys_Settings.VolumeMusic = newVal; if (!AnyLeftRightMouseDown()) {SaveConfig();} }
        }
        
        RenderUIImage(1087,723, 84,36, 1252); // Back Button background
        i8 lastItem = menuItemCount - 1;
        bool overBack = false;
        if (UI_Button(1087,757, 84,32, &overBack, lastItem) || (MenuEnter() && currentMenuItem == lastItem)) MenuGoBack();
        overBack = overBack || currentMenuItem == lastItem;
        RenderFormattedText(1103,731,overBack ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_RED_MENU,FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == Mpg_Load || currentMenuPage == Mpg_Save) {
        menuItemCount = 9; menuTabCount = 1;
        bool isSave = currentMenuPage == Mpg_Save;
        UI_HeaderText(isSave ? 284 : 340, isSave ? /*"SAVE GAME"*/Sys_Text.stringTable[769] : /*"LOAD"*/Sys_Text.stringTable[726]);
        RenderUIImage(400,214, 586,500, 1037); // Load/Save table background
        RenderUIImage(1060,724, 84,36, 1252); // Back Button background
        bool overBack = false;
        if (UI_Button(1060,758, 84,32, &overBack, 0) || (MenuEnter() && currentMenuItem == 0)) MenuGoBack();
        overBack = overBack || currentMenuItem == 0;
        RenderFormattedText(1076,732, overBack ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_RED_MENU, FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == Mpg_NewGame) {
        menuItemCount = 7; menuTabCount = (currentMenuItem > 0 && currentMenuItem <= 16) ? 2 : 1;
        UI_HeaderText(290,/*"NEW GAME"*/Sys_Text.stringTable[741]);
        RenderUIImage(136,196,1088,558,1048); // Newgame inset
        RenderUIImage(136,196,1088,558,1049); // Newgame background
        if (UI_MenuButton(276,270,0,795,74, 226,146,/*"NAME:"*/Sys_Text.stringTable[746],299,214)) { /* Just for highlight */ }
        enteringPlayerName = (currentMenuItem == 0);
        if (Sys_Global.playerName[0] == '\0') RenderFormattedText(642,232,TEXT_RED_MENU,FONT_STOPD,1.0f,/*"ENTER NAME..."*/Sys_Text.stringTable[748]);
        else                                  RenderFormattedText(518,232,enteringPlayerName ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.0f,Sys_Global.playerName);

        if (UI_MenuButton(174,377,1,496,95, 148,202,/*"COMBAT"*/Sys_Text.stringTable[748],185,299)) { Sys_Global.difficultyCombat = Sys_Global.difficultyCombat >= 3 ? 0 : Sys_Global.difficultyCombat + 1; }  if (UI_MenuButton(704,377,3,496,95, 510,202,/*"MISSION"*/Sys_Text.stringTable[749],726,299)) { Sys_Global.difficultyMission = Sys_Global.difficultyMission >= 3 ? 0 : Sys_Global.difficultyMission + 1; }
        RenderFormattedText(162,270,Sys_Global.difficultyCombat == 0 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"0");    RenderFormattedText(513,270,Sys_Global.difficultyMission == 0 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"0");
        RenderFormattedText(233,270,Sys_Global.difficultyCombat == 1 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"1");    RenderFormattedText(584,270,Sys_Global.difficultyMission == 1 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"1");
        RenderFormattedText(307,270,Sys_Global.difficultyCombat == 2 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"2");    RenderFormattedText(658,270,Sys_Global.difficultyMission == 2 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"2");
        RenderFormattedText(379,270,Sys_Global.difficultyCombat == 3 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"3");    RenderFormattedText(730,270,Sys_Global.difficultyMission == 3 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"3");
        if (UI_MenuButton(174,568,2,496,92, 149,330,/*"PUZZLE"*/Sys_Text.stringTable[751],185,490)) { Sys_Global.difficultyPuzzle = Sys_Global.difficultyPuzzle >= 3 ? 0 : Sys_Global.difficultyPuzzle + 1; }  if (UI_MenuButton(704,568,4,496,92, 509,330,/*"CYBERSPACE"*/Sys_Text.stringTable[750],726,490)) { Sys_Global.difficultyCyber = Sys_Global.difficultyCyber >= 3 ? 0 : Sys_Global.difficultyCyber + 1; }
        RenderFormattedText(162,399,Sys_Global.difficultyPuzzle == 0 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"0");    RenderFormattedText(513,399,Sys_Global.difficultyCyber == 0 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"0");
        RenderFormattedText(233,399,Sys_Global.difficultyPuzzle == 1 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"1");    RenderFormattedText(584,399,Sys_Global.difficultyCyber == 1 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"1");
        RenderFormattedText(307,399,Sys_Global.difficultyPuzzle == 2 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"2");    RenderFormattedText(658,399,Sys_Global.difficultyCyber == 2 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"2");
        RenderFormattedText(379,399,Sys_Global.difficultyPuzzle == 3 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"3");    RenderFormattedText(730,399,Sys_Global.difficultyCyber == 3 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"3");
        if (UI_Button(221,460,82,79,NULL,1)) {Sys_Global.difficultyCombat =0; currentMenuItem=1; } if (UI_Button(330,460,82,79,NULL,1)) {Sys_Global.difficultyCombat =1; currentMenuItem=1; } if (UI_Button(439,460,82,79,NULL,1)) {Sys_Global.difficultyCombat =2; currentMenuItem=1; } if (UI_Button( 547,460,82,79,NULL,1)) {Sys_Global.difficultyCombat =3; currentMenuItem=1; }
        if (UI_Button(221,651,82,79,NULL,2)) {Sys_Global.difficultyPuzzle =0; currentMenuItem=2; } if (UI_Button(330,651,82,79,NULL,2)) {Sys_Global.difficultyPuzzle =1; currentMenuItem=2; } if (UI_Button(439,651,82,79,NULL,2)) {Sys_Global.difficultyPuzzle =2; currentMenuItem=2; } if (UI_Button( 547,651,82,79,NULL,2)) {Sys_Global.difficultyPuzzle =3; currentMenuItem=2; }
        if (UI_Button(748,460,82,79,NULL,3)) {Sys_Global.difficultyMission=0; currentMenuItem=3; } if (UI_Button(857,460,82,79,NULL,3)) {Sys_Global.difficultyMission=1; currentMenuItem=3; } if (UI_Button(966,460,82,79,NULL,3)) {Sys_Global.difficultyMission=2; currentMenuItem=3; } if (UI_Button(1074,460,82,79,NULL,3)) {Sys_Global.difficultyMission=3; currentMenuItem=3; }
        if (UI_Button(748,651,82,79,NULL,4)) {Sys_Global.difficultyCyber  =0; currentMenuItem=4; } if (UI_Button(857,651,82,79,NULL,4)) {Sys_Global.difficultyCyber  =1; currentMenuItem=4; } if (UI_Button(966,651,82,79,NULL,4)) {Sys_Global.difficultyCyber  =2; currentMenuItem=4; } if (UI_Button(1074,651,82,79,NULL,4)) {Sys_Global.difficultyCyber  =3; currentMenuItem=4; }
        bool overBack = false, overStart = false;
        if (UI_Button(544,747, 282,68, &overStart, 5) || (MenuEnter() && currentMenuItem == 5)) GoIntoGame(); // TODO reload game.
        overStart = overStart || currentMenuItem == 5;
        RenderFormattedText(400,464,overStart ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,/*"START"*/Sys_Text.stringTable[886]);
        
        if (UI_Button(1060,758, 84,32, &overBack, 6) || (MenuEnter() && currentMenuItem == 6)) MenuGoBack();
        overBack = overBack || currentMenuItem == 6;
        RenderUIImage(1060,724,84,36,1252); // Back Button background
        RenderFormattedText(1076,732,overBack ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_RED_MENU,FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == Mpg_IntroVideo || currentMenuPage == Mpg_CreditsVideo) {
        menuItemCount = menuTabCount = 1;
        if (MenuEnter()) MenuGoBack();
    }
    
    if (menuTabCount <= currentMenuTab) currentMenuTab = 0;
    if (menuItemCount <= currentMenuItem) currentMenuItem = 0;
    static const i8 ngSwap[7] = {0,3,4,1,2,6,5};
    if (Sys_Input.keyStates[KEY_RIGHT].pressed || Sys_Input.keyStates[KEY_LEFT].pressed) {
        int dir = Sys_Input.keyStates[KEY_RIGHT].pressed ? 1 : -1;
        currentMenuTab = (currentMenuTab + menuTabCount + dir) % menuTabCount;
        if (currentMenuPage == Mpg_NewGame && currentMenuItem < 7) currentMenuItem = ngSwap[currentMenuItem];
    }
}

void RenderPausedUI() {
    menuItemCount = 6; menuTabCount = 1;
    bool overResume = false, overLoad /* ;) */ = false, overSave = false, overOptions = false, overQuitMenu = false, overQuit = false;
    RenderUIImage(519,276,328,300,1025); // Pause Menu background
    RenderUIImage(519,276,328,300,1080); // Pause Menu background outline
    RenderFormattedText(610,210,TEXT_STOPD_RED_PAUSETITLE,FONT_STOPD,1.0f,/*"PAUSED"*/Sys_Text.stringTable[724]);
    if (UI_Button(522,330, 322,52, &overResume, 0) || (MenuEnter() && currentMenuItem == 0)) Sys_Global.gamePaused = false;
    overResume = overResume || currentMenuItem == 0;
    RenderFormattedText(610,306,overResume ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.0f,/*"RESUME"*/Sys_Text.stringTable[725]);
    if (UI_Button(522,390, 322,52, &overLoad, 1) || (MenuEnter() && currentMenuItem == 1)) { currentMenuPage = Mpg_Load; PlayMenuMusic(); Sys_Global.menuActive = true; returnToPause = true; }
    overLoad = overLoad || currentMenuItem == 1;
    RenderFormattedText(630,364, overLoad ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.0f,/*"LOAD"*/Sys_Text.stringTable[726]);
    if (UI_Button(522,450, 322,60, &overSave, 2) || (MenuEnter() && currentMenuItem == 2)) { currentMenuPage = Mpg_Save; PlayMenuMusic(); Sys_Global.menuActive = true; returnToPause = true; }
    overSave = overSave || currentMenuItem == 2;
    RenderFormattedText(635,422,overSave ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.0f,/*"SAVE"*/Sys_Text.stringTable[727]);
    if (UI_Button(522,510, 322,60, &overOptions, 3) || (MenuEnter() && currentMenuItem == 3)) { currentMenuPage = Mpg_Options; PlayMenuMusic(); Sys_Global.menuActive = true; returnToPause = true; }
    overOptions = overOptions || currentMenuItem == 3;
    RenderFormattedText(599,480,overOptions ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.0f,/*"OPTIONS"*/Sys_Text.stringTable[721]);
    if (UI_Button(522,570, 322,60, &overQuitMenu, 4) || (MenuEnter() && currentMenuItem == 4)) OpenMainMenu();
    overQuitMenu = overQuitMenu || currentMenuItem == 4;
    RenderFormattedText(546,538,overQuitMenu ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.0f,/*"QUIT TO MENU"*/Sys_Text.stringTable[728]);
    RenderUIImage(519,672,328,42,1252); // Pause Quit Game background
    if (UI_Button(522,714, 322,42, &overQuit, 5) || (MenuEnter() && currentMenuItem == 5)) OS_Exit(0);
    overQuit = overQuit || currentMenuItem == 5;
    RenderFormattedText(572,690,overQuit ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.0f,/*"QUIT GAME"*/Sys_Text.stringTable[729]);
}

typedef struct { float x,y,z,r,g,b,a; } DebugLineVertex;
DebugLineVertex debugLineVerts[MAX_DEBUG_LINE_VERTS * 2];
static inline __attribute__((always_inline)) void DrawDebugLines(float* viewProj) {
    if (Sys_Cheats.noclip) return;
    if (Sys_Global.debugLineVertCount == 0) return;

    glBindBuffer(GL_ARRAY_BUFFER,Sys_Render.debugLinesVBO);
    glBufferSubData(GL_ARRAY_BUFFER,0,Sys_Global.debugLineVertCount * sizeof(DebugLineVertex),debugLineVerts);
    CHECK_GL_ERROR(); // 1282 here
    glUseProgram(Sys_Render.debugUnlitShaderProgram);
    glUniformMatrix4fv(0,1,GL_FALSE,viewProj);
    glLineWidth(1.0f);
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(Sys_Render.debugLinesVAO);
    glDrawArrays(0x0001/*GL_LINES*/,0,Sys_Global.debugLineVertCount);
    glEnable(GL_DEPTH_TEST);
    drawCalls++; vertsRendered += Sys_Global.debugLineVertCount;
    Sys_Global.debugLineVertCount = 0;
}

ENGINE_TO_MOD void AddDebugLine(Vector3 start, Vector3 end, Color col) {
    if (Sys_Global.debugLineVertCount >= MAX_DEBUG_LINE_VERTS - 2) return;

    int i = Sys_Global.debugLineVertCount;
    debugLineVerts[i].x = start.x; debugLineVerts[i].y = start.y; debugLineVerts[i].z = start.z;
    debugLineVerts[i].r = col.r; debugLineVerts[i].g = col.g; 
    debugLineVerts[i].b = col.b; debugLineVerts[i].a = col.a; i++;
    debugLineVerts[i].x = end.x; debugLineVerts[i].y = end.y; debugLineVerts[i].z = end.z;
    debugLineVerts[i].r = col.r; debugLineVerts[i].g = col.g; 
    debugLineVerts[i].b = col.b; debugLineVerts[i].a = col.a; i++;
    Sys_Global.debugLineVertCount = i;
}

u8 MFD_LefTab=0,MFD_CenterTab=0,MFD_RightTab=0;
static double RenderUI() {
    drawCallsNormal = drawCalls;
    if (Sys_Global.creditsActive) { // Render Credits
        if (Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed) {
            ++Sys_Global.creditsPageIndex;
            if (Sys_Global.creditsPageIndex > CREDITS_PAGES) {Sys_Global.creditsActive = false; return get_time(); } // Finished with Erthang!  That's it, go home.
        }

        if (Sys_Global.creditsPageIndex == 1) { CreditsStats(); RenderFormattedText(300,10,TEXT_WHITE,FONT_NORMAL,1.0f,(const char*)&creditStats); }
        else                                                    RenderFormattedText(300,10,TEXT_WHITE,FONT_NORMAL,1.0f,creditPages[Sys_Global.creditsPageIndex]);
        
        return get_time();
    }
    if (Sys_Global.menuActive) RenderMenu();
    else if (Sys_Global.gamePaused) RenderPausedUI();
    if ((Sys_Global.menuActive || Sys_Global.gamePaused)) {
        if (Sys_Input.keyStates[KEY_DOWN].pressed) currentMenuItem = (currentMenuItem + 1) >= menuItemCount ? 0 : (currentMenuItem + 1);
        else if (Sys_Input.keyStates[KEY_UP].pressed) currentMenuItem = (currentMenuItem - 1) < 0 ? (menuItemCount - 1) : (currentMenuItem - 1);
    } else { // Normal UI
                                                                                                                           if (!Sys_Cheats.noHUD) {RenderUIImage(672,0,22,22,1020);} /*Shoot mode button*/
                                                                                                                           if (Sys_Global.inventoryMode && Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed && CursorIsOverBounds(672,694,22,0)) ForceShootMode();
        /*Left Tab Buttons*/                                                                                                                                                                                                                                                                                                                                           /*Right Tab Buttons*/
        RenderUIImage(-16,552,32,40,MFD_LefTab == 0 ? 1024 : 1022);                                                                                                                                                                                                                                                           RenderUIImage(1350,552,32,40,MFD_RightTab == 0 ? 1024 : 1022);
        RenderUIImage(-16,600,32,40,MFD_LefTab == 1 ? 1024 : 1022);                                                                                                                                                                                                                                                           RenderUIImage(1350,600,32,40,MFD_RightTab == 1 ? 1024 : 1022);
        RenderUIImage(-16,648,32,40,MFD_LefTab == 2 ? 1024 : 1022);                                                                                                                                                                                                                                                           RenderUIImage(1350,648,32,40,MFD_RightTab == 2 ? 1024 : 1022);
        RenderUIImage(-16,696,32,40,MFD_LefTab == 3 ? 1024 : 1022);                                                                                                                                                                                                                                                           RenderUIImage(1350,696,32,40,MFD_RightTab == 3 ? 1024 : 1022);
                                                                                                                                                                                     /*Center Tab Buttons*/
                                                                   RenderUIImage(400,752,64,32,MFD_CenterTab == 0 ? 1024 : 1021); RenderUIImage(480,752,64,32,MFD_CenterTab == 1 ? 1024 : 1021); RenderUIImage(560,752,64,32,MFD_CenterTab == 2 ? 1024 : 1021); RenderUIImage(902,752,64,32,MFD_CenterTab == 3 ? 1024 : 1021);
    }
    
    i16 debugTextStartY = 48; // Diagnostics / Debugging
    if (Sys_Cheats.showLocation && !Sys_Global.menuActive) RenderFormattedText(16, debugTextStartY, TEXT_WHITE, FONT_NORMAL,1.0f, "x: %.4f, y: %.4f, z: %.4f, rx: %.4f, ry: %.4f, rz: %.4f, rw: %.4f",Sys_Global.instances[PLAYER1].position.x,Sys_Global.instances[PLAYER1].position.y,Sys_Global.instances[PLAYER1].position.z,Sys_Global.instances[PLAYER1].rotation.x,Sys_Global.instances[PLAYER1].rotation.y,Sys_Global.instances[PLAYER1].rotation.z,Sys_Global.instances[PLAYER1].rotation.w);
    i16 lineSpacing = 18;
    if (!Sys_Global.menuActive && !Sys_Cheats.noHUD) RenderFormattedText(16,debugTextStartY + (lineSpacing * 1),TEXT_WHITE,FONT_NORMAL,1.0f,"playerCellIdx: %u, Shadow cpu ms: %.3f",playerCellIdx,voxen_Shadow_System.shadowTime * 1000);
    if (!Sys_Global.menuActive && !Sys_Cheats.noHUD) RenderFormattedText(16,debugTextStartY + (lineSpacing * 2),TEXT_WHITE,FONT_NORMAL,1.0f,"Player velocity: %.2f, %.2f, %.2f, Grounded: %u",Sys_Global.instances[PLAYER1].velocity.x,Sys_Global.instances[PLAYER1].velocity.y,Sys_Global.instances[PLAYER1].velocity.z,Sys_Global.instances[PLAYER1].entflags & EF_GROUNDED);
    RenderFormattedText(16,debugTextStartY + (lineSpacing * 4),TEXT_WHITE,FONT_NORMAL,1.0f,"Cursor: %d, %d  dx:%d dy:%d",Sys_Global.cursorPosition_x,Sys_Global.cursorPosition_y,Sys_Input.currentMouse_dx,Sys_Input.currentMouse_dy);
    if (Sys_Cheats.consoleActive) RenderFormattedText(16,0,TEXT_WHITE,FONT_NORMAL,1.0f, "] %s",consoleEntryText);
    if (Sys_Global.statusTextDecayFinished > Sys_Global.current_time) RenderFormattedText(460,114,TEXT_WHITE,FONT_NORMAL,1.0f, "%s",statusText);
    double time_now = get_time();
    if (Sys_Cheats.showFPS) {
        Sys_Global.thisFrameTime = (time_now - Sys_Global.last_time) * 1000.0;
        Sys_Global.cpuFrameTime = Sys_Global.cpuTime * 1000.0;
        u8 timingColor = TEXT_WHITE;
        if (vabs(Sys_Global.thisFrameTime - Sys_Global.cpuFrameTime) < 0.451) timingColor = TEXT_GREEN;
        if (Sys_Global.thisFrameTime > 6.944444) timingColor = TEXT_RED;
        drawCalls += 2; // Add two more for this text render ;)
        RenderFormattedText(16, debugTextStartY - lineSpacing, timingColor, FONT_NORMAL,1.0f, "ms: %.2f, CPU %.2f", Sys_Global.thisFrameTime,Sys_Global.cpuFrameTime);
        RenderFormattedText(16 + 230.0f, debugTextStartY - lineSpacing, TEXT_WHITE, FONT_NORMAL,1.0f, "(FPS:%d, Worst:%d),Drwclls:%d [G:%d UI:%d Sh:%d] Vrt:%d E:%u|M:%u|P:%u",Sys_Global.framesPerLastSecond,Sys_Global.worstFPS,drawCalls,drawCallsNormal,uiDrawCalls,shadDrawCalls,vertsRendered,Sys_Cheats.editMode,Sys_Global.menuActive,Sys_Global.gamePaused);
    }
    
    return time_now;
}

#define SHADOW_NEARMESH_MAX 1024
typedef struct {float depth; u16 index; } DepthSort;
DepthSort shadows_nearMeshes[SHADOW_NEARMESH_MAX];
static inline __attribute__((always_inline)) bool EntNotVisible(u16 i, bool otherCondition) { Entity* e = &Sys_Global.instances[i]; return e->texIndex > loadedTexturesMaxIndex || !(e->entflags & EF_ACTIVE) || e->index >= MAX_ENTITIES || e->modelIndex >= MODEL_IDX_MAX || e->texIndex >= MAX_VALID_TEXTURE || otherCondition; }
static inline __attribute__((always_inline,hot)) u16 GetAndBindModel(u16 i, u16 currentModelType) {
    glUniform1ui(0,i);
    u16 modelType = (instanceIsLODArray[i] || Sys_Settings.ModelDetail < 1u) && Sys_Global.instances[i].lodIndex < loadedModelsMaxIndex ? Sys_Global.instances[i].lodIndex : Sys_Global.instances[i].modelIndex;
    if (currentModelType == modelType && currentModelType != 0) return currentModelType;
    
    glBindVertexBuffer(0,Sys_Render.vbos[modelType],0,VERTEX_ATTRIBUTES_SIZE);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,Sys_Render.tbos[modelType]);
    return modelType;
}

#define SC_MAX (SHADOW_NEARMESH_MAX * MAX_SHADOWMAPS)
u16 shadowCasterIndices[SC_MAX],candidates[MAX_SHADOWMAPS];
static const u32 groupX_shadClear = ((SHADOW_MAP_SIZE * SHADOW_MAP_SIZE) + 31) / 32;
static __attribute__((hot)) void RenderShadowmaps() {    
    double shadowStartTime = get_time();
    MemSetToVForNBytes(candidates,0,MAX_SHADOWMAPS*sizeof(u16));
    u16 numShadowsCouldRender = 0;
    Vector3 playerPos = Sys_Global.instances[PLAYER1].position, pf = Sys_Global.instances[PLAYER1].forward;
    for (u16 i = 0; i < loadedLights; ++i) { // Collect candidates: only lights that are enabled and in PVS
        if (unlikely(!(lights[i].lflags & SHADON) || !(lights[i].lflags & LIGHTON))) continue;

        Vector3 lightPos = lights[i].pos;
        float intensity = lights[i].maxIntensity; // Much more stable than actual intensity (from fade/flickers).  Since gated by on above, this is fine now.
        if (unlikely(intensity < 0.1f)) continue;
        
        float range =  lights[i].range;
        float luminosity = (intensity / (range * range));
        if (luminosity < 0.008f && (range < 8.0f || intensity < 0.5f)) continue;
        
        u16 cellX = PosGetCellCoordX(lightPos.x), cellZ = PosGetCellCoordZ(lightPos.z);
        int lightCellIdx = (cellZ * WORLDX) + cellX; u8 r = vceil(range * (1.0f / CELL_SIZE));
        bool inPVS = (gridCellStates[lightCellIdx] & CELL_VISIBLE);
        if (likely(!inPVS)) inPVS = NeighborhoodInPVS(cellX,cellZ,r);
        if (!inPVS) continue;
        
        float dx = lightPos.x - playerPos.x, dy = lightPos.y - playerPos.y, dz = lightPos.z - playerPos.z;
        float distSqrdToPlayer = dx*dx + dy*dy + dz*dz;
        float dotResult = (dx*pf.x + dy*pf.y + dz*pf.z);
        if (dotResult < 0.0f && distSqrdToPlayer > (range * range)) continue;

        candidates[numShadowsCouldRender] = i; numShadowsCouldRender++; if (numShadowsCouldRender >= MAX_SHADOWMAPS) break;
    }

    if (numShadowsCouldRender > 0) { // Added since there is now work between here and the for loop so this is beneficial to check.
        glUseProgram(Sys_Render.shadowmapsClearShaderProgram); // Clear shadowmaps.  One might think that this would be less performant than standard shadowmap FBO with gl clears and textures but in fact this is faster on all but the oldest hardware (e.g. 10yrs old is fine, 13yrs suffers a small hit).
        for (u32 c=0;c<numShadowsCouldRender;++c) {
            glUniform1ui(0,c); glDispatchCompute(groupX_shadClear,6,1);
        }

        shadDrawCalls = 0;
        glViewport(0,0,SHADOW_MAP_SIZE,SHADOW_MAP_SIZE);
        glUseProgram(Sys_Render.shadowmapsShaderProgram);
        u32 shadowmapOffsetHead = 0U;
        MemSetToVForNBytes(shadowCasterIndices,0,SC_MAX*sizeof(u16));
        u32 numShadowCasters = 0;
        for (int i=START_INDEX_LEVEL_INSTANCES;i<INSTANCE_COUNT;++i) {
            if (EntNotVisible(i,(Sys_Global.instances[i].entflags & EF_NO_SHADOWS))) continue;

            shadowCasterIndices[numShadowCasters] = i;
            numShadowCasters++;
            if (numShadowCasters >= (SC_MAX)) break; // Ran out of shadowcasters max for frame.
        }
        
        u16 shadowMapIdx=0,currentModelType=0,currentTexIndex=0; bool currentIsTransparent=0;
        bool useDetail = Sys_Settings.ModelDetail;
        for (u32 c = 0; c < numShadowsCouldRender; ++c, ++shadowMapIdx) { // Render top MAX_SHADOWMAPS candidates
            u16 lightIdx = candidates[c];
            float effectiveRadius = vmin(lights[lightIdx].range,15.36f);
            u16 nearbyMeshCount = 0;
            Vector3 lpos = lights[lightIdx].pos;
            float cellCenterX=vround(lpos.x / CELL_SIZE) * CELL_SIZE, cellCenterZ=vround(lpos.z / CELL_SIZE) * CELL_SIZE;
            Vector3 deltaCellCenter = V3_AsubB((Vector3){lpos.x,0.0f,lpos.z},(Vector3){cellCenterX,0.0f,cellCenterZ});
            float distToCenterSqrd = V3_dot(deltaCellCenter,deltaCellCenter);
            bool skipNPCs = (distToCenterSqrd < 0.4096f); // 0.64 * 0.64
            for (u16 shadowCasterInstanceIdx = 0; shadowCasterInstanceIdx < numShadowCasters; shadowCasterInstanceIdx++) {
                u16 j = shadowCasterIndices[shadowCasterInstanceIdx];
                Entity* e = &Sys_Global.instances[j];
                Vector3 d = V3_AsubB(e->position,lpos);
                float distToLightSqrd = V3_dot(d,d);
                float radSum = (effectiveRadius + e->radius);
                if (distToLightSqrd >= radSum * radSum) continue;
                if (skipNPCs && ConstIndexIsNPC(e->index)) continue;
                
                shadows_nearMeshes[nearbyMeshCount].index = j; shadows_nearMeshes[nearbyMeshCount].depth = distToLightSqrd; 
                nearbyMeshCount++; if (nearbyMeshCount >= SHADOW_NEARMESH_MAX) { DualLogWarn("Shadowmapping needs larger nearMeshes count than %u!  Skipping some renderables for light %u!\n", SHADOW_NEARMESH_MAX, lightIdx); break; }
            }

            if (unlikely(nearbyMeshCount < 1)) continue;

            glUniform3f(3,lpos.x,lpos.y,lpos.z);
            voxen_Shadow_System.shadowmapIndirectionList[lightIdx] = shadowMapIdx;
            #pragma GCC unroll 6
            for (u8 face = 0; face < 6; face++) {                                            
                glUniform1ui(2,face);
                glUniformMatrix4fv(1,1,GL_FALSE,(float*)lightViewProj[lightIdx][face]);
                glUniform1ui(7,shadowmapOffsetHead + (face * SHADOW_MAP_SIZE * SHADOW_MAP_SIZE));
                for (u16 j = 0; j < nearbyMeshCount; ++j) {
                    int i = shadows_nearMeshes[j].index;
                    Entity* e = &Sys_Global.instances[i];
                    if (!SphereInFrustum(lightFrustumPlanes[lightIdx][face],e->position,e->shadRadius)) continue;

                    glUniform1ui(0,i);
                    u16 modelType = (instanceIsLODArray[i] || useDetail < 1u) && e->lodIndex < loadedModelsMaxIndex ? e->lodIndex : e->modelIndex;
                    if (currentModelType != modelType || currentModelType == 0) { currentModelType = modelType; glBindVertexBuffer(0,Sys_Render.vbos[modelType],0,VERTEX_ATTRIBUTES_SIZE); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,Sys_Render.tbos[modelType]); }
                    if (currentTexIndex != e->texIndex) { currentTexIndex = e->texIndex; glUniform1ui(6,e->texIndex); }
                    bool texIsTransparent = transparentTexture[e->texIndex];
                    if (currentIsTransparent != texIsTransparent) { currentIsTransparent = texIsTransparent; glUniform1ui(8,currentIsTransparent ? 1u : 0u); }
                    glDrawElements(0x0004/*GL_TRIANGLES*/,modelTriangleCounts[currentModelType]*3,GL_UNSIGNED_SHORT,0); drawCalls++; shadDrawCalls++; vertsRendered += modelTriangleCounts[currentModelType] * 3;
                }
            }
            
            shadowmapOffsetHead += (SHADOW_MAP_SIZE * SHADOW_MAP_SIZE) * 6;
        }

        glViewport(0,0,Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight);
        glBindBuffer(GL_SSBO,Sys_Render.shadowMapsIndirectionID); glBufferData(GL_SSBO,loadedLights * sizeof(u32),voxen_Shadow_System.shadowmapIndirectionList,GL_DYNAMIC_DRAW);
    }

    voxen_Shadow_System.shadowTime = get_time() - shadowStartTime;
}

DepthSort visibleInstances[INSTANCE_COUNT];
static inline __attribute__((always_inline)) bool DetermineIfInstanceVisible(u16 i, bool otherCondition, bool skyVisible, Vector3 playerPos, float* distSqrd) {
    if (EntNotVisible(i,otherCondition)) return false; // must be transparent && transparents or neither
    
    Entity* e = &Sys_Global.instances[i];
    u16 instCellIdx = e->cellIndex; u16 entIdx = e->index;
    Vector3 delta = V3_AsubB(e->position,playerPos);
    *distSqrd = delta.x*delta.x + delta.y*delta.y + delta.z*delta.z;
    float radius = modelBounds[e->modelIndex] * 2.0f * vmax(vmax(e->scale.x,e->scale.y),e->scale.z);
    if (!SphereInFrustum(playerFrustumPlanes,e->position,radius) && (entIdx != 754 || !skyVisible) && i != editModeSelection) return false;
    
    if (ConstIndexIsPortalBlockingDoor(entIdx)) { // Extra checks only needed for opaque portal blocking doors.
        bool inPVS = (gridCellStates[instCellIdx] & CELL_VISIBLE);
        if (!inPVS) {inPVS = NeighborhoodInPVS(e->cellX,e->cellZ,2u);} if (!inPVS) return false;
    } else {
        if (((gridCellStates[instCellIdx] & (CELL_VISIBLE | CELL_OPEN)) == CELL_OPEN) && (entIdx != 754 || !skyVisible)) return false;
        if (!(gridCellStates[instCellIdx] & CELL_OPEN) && *distSqrd >= 943.7184f && (entIdx != 754 || !skyVisible)) return false; // 30.72 * 30.72, 12 cells
    }

    if (Sys_Global.instances[i].camView != 255) camViews[Sys_Global.instances[i].camView].visible = true;
    return true;
}

float GetPainStatic() { return 0.0f; } // TODO: Hook into pain/health management and shield impact effect
Color GetPainStaticColor() { return (Color){1.0f,0.0f,0.0f,1.0f}; } // TODO: Hook staticColor up to red or blue for pain or shield impact.
__attribute__((pure)) i32 dsort(const void* a, const void* b) { float da = ((const DepthSort*)a)->depth; float db = ((const DepthSort*)b)->depth; return (db > da) - (db < da); }
__attribute__((pure)) i32 dsortInv(const void* a, const void* b) { float da = ((const DepthSort*)a)->depth; float db = ((const DepthSort*)b)->depth; return (da > db) - (da < db); }
#define MAX_VISIBLE 4096
#define DRAW_ENTITY(curN,curT,curG,curS,curM) { \
        u16 glow=e->glowIndex,norm=e->normIndex,spec=e->specIndex; \
        if (Sys_Cheats.showPhys) {if (e->collider == COLTYPE_BOX) {DrawBoxCollider(e);} else if (e->collider == COLTYPE_SPH) {DrawSphereCollider(e);} else if (e->collider == COLTYPE_CVX) {DrawMeshCollider(e);} else if (e->collider == COLTYPE_MSH) {DrawMeshCollider(e);} else if (e->collider == COLTYPE_CAP) {DrawCapsuleCollider(e);}} \
        DrawAngularVelocity(e); \
        glUniform1ui(17,tex==316?1u:0u); glUniform1ui(25,constIndex); glUniform1f(27,e->volume); glUniform1ui(13,(tex==36||tex==887) ? 1u : 0u); \
        if (grayscaleEnabled) { float npcHeat = ConstIndexIsNPC(constIndex) ? ((constIndex==419 || constIndex==422 || constIndex==424 || constIndex==429 || constIndex==430 || constIndex==431||constIndex==433||constIndex==437||constIndex==438||constIndex==441) ? 1.5f : 4.0f) : 0.0f; glUniform1f(9,npcHeat); } \
        glUniform1ui(30,e->camView < camViewCount ? 1u : 0u); \
        if(e->camView < camViewCount) { glActiveTexture(GL_TEXTURE6); glBindTexture(GL_TEXTURE_2D,camViewTextures[e->camView]); glUniform2ui(28,camViews[e->camView].width,camViews[e->camView].height); glUniform1i(29,6); } \
        if((curN) != (norm) || norm==0) { curN=norm; glUniform1ui( 1,(u32)norm); } \
        if((curT) != ( tex) ||  tex==0) { curT= tex; glUniform1ui(18,(u32)tex ); } \
        if((curG) != (glow) || glow==0) { curG=glow; glUniform1ui(19,(u32)glow); } \
        if((curS) != (spec) || spec==0) { curS=spec; glUniform1ui(20,(u32)spec); } \
        curM=GetAndBindModel(i,curM); u32 vc=modelTriangleCounts[curM]*3; glDrawElements(0x0004,vc,GL_UNSIGNED_SHORT,0); drawCalls++; vertsRendered+=vc; \
     }

bool mat4_inverse(const float* m, float* out) {
    float inv[16],det;
    inv[0] =  m[5]*m[10]*m[15] - m[5]*m[14]*m[11] - m[9]*m[6]*m[15] + m[9]*m[14]*m[7] + m[13]*m[6]*m[11] - m[13]*m[10]*m[7];
    inv[4] = -m[4]*m[10]*m[15] + m[4]*m[14]*m[11] + m[8]*m[6]*m[15] - m[8]*m[14]*m[7] - m[12]*m[6]*m[11] + m[12]*m[10]*m[7];
    inv[8] =  m[4]*m[9]*m[15]  - m[4]*m[13]*m[11] - m[8]*m[5]*m[15] + m[8]*m[13]*m[7]  + m[12]*m[5]*m[11] - m[12]*m[9]*m[7];
    inv[12]= -m[4]*m[9]*m[14]  + m[4]*m[13]*m[10] + m[8]*m[5]*m[14] - m[8]*m[13]*m[6]  - m[12]*m[5]*m[10] + m[12]*m[9]*m[6];
    inv[1] = -m[1]*m[10]*m[15] + m[1]*m[14]*m[11] + m[9]*m[2]*m[15] - m[9]*m[14]*m[3] - m[13]*m[2]*m[11] + m[13]*m[10]*m[3];
    inv[5] =  m[0]*m[10]*m[15] - m[0]*m[14]*m[11] - m[8]*m[2]*m[15] + m[8]*m[14]*m[3]  + m[12]*m[2]*m[11] - m[12]*m[10]*m[3];
    inv[9] = -m[0]*m[9]*m[15]  + m[0]*m[13]*m[11] + m[8]*m[1]*m[15] - m[8]*m[13]*m[3]  - m[12]*m[1]*m[11] + m[12]*m[9]*m[3];
    inv[13]=  m[0]*m[9]*m[14]  - m[0]*m[13]*m[10] - m[8]*m[1]*m[14] + m[8]*m[13]*m[2]  + m[12]*m[1]*m[10] - m[12]*m[9]*m[2];
    inv[2] =  m[1]*m[6]*m[15] - m[1]*m[14]*m[7] - m[5]*m[2]*m[15] + m[5]*m[14]*m[3] + m[13]*m[2]*m[7] - m[13]*m[6]*m[3];
    inv[6] = -m[0]*m[6]*m[15] + m[0]*m[14]*m[7] + m[4]*m[2]*m[15] - m[4]*m[14]*m[3] - m[12]*m[2]*m[7] + m[12]*m[6]*m[3];
    inv[10]=  m[0]*m[5]*m[15] - m[0]*m[13]*m[7] - m[4]*m[1]*m[15] + m[4]*m[13]*m[3] + m[12]*m[1]*m[7] - m[12]*m[5]*m[3];
    inv[14]= -m[0]*m[5]*m[14] + m[0]*m[13]*m[6] + m[4]*m[1]*m[14] - m[4]*m[13]*m[2] - m[12]*m[1]*m[6] + m[12]*m[5]*m[2];
    inv[3] = -m[1]*m[6]*m[11] + m[1]*m[10]*m[7] + m[5]*m[2]*m[11] - m[5]*m[10]*m[3] - m[9]*m[2]*m[7]  + m[9]*m[6]*m[3];
    inv[7] =  m[0]*m[6]*m[11] - m[0]*m[10]*m[7] - m[4]*m[2]*m[11] + m[4]*m[10]*m[3] + m[8]*m[2]*m[7]  - m[8]*m[6]*m[3];
    inv[11]= -m[0]*m[5]*m[11] + m[0]*m[9]*m[7]  + m[4]*m[1]*m[11] - m[4]*m[9]*m[3]  - m[8]*m[1]*m[7]  + m[8]*m[5]*m[3];
    inv[15]=  m[0]*m[5]*m[10] - m[0]*m[9]*m[6]  - m[4]*m[1]*m[10] + m[4]*m[9]*m[2]  + m[8]*m[1]*m[6]  - m[8]*m[5]*m[2];
    det = m[0]*inv[0] + m[1]*inv[4] + m[2]*inv[8] + m[3]*inv[12];
    if (det == 0.0f) { for(int i=0;i<16;++i) {out[i] = (i%5==0) ? 1.0f : 0.0f;} return false; }

    det = 1.0f / det; for (int i=0;i<16;++i) out[i] = inv[i] * det;
    return true;
}

void GetProjections(float* view, float* viewProj, float* invViewRot, float* invViewProj, float sfov, float aspect3D, float snear, float sfar) {
    float f = vcot(sfov * PI / 360.0f);
    float* m = rasterPerspectiveProjection;
    m[0] = f / aspect3D; m[1] = 0.0f; m[2] = 0.0f; m[3] = 0.0f; m[4] = 0.0f; m[5] = f; m[6] = 0.0f; m[7] = 0.0f;
    m[8] = 0.0f; m[9] = 0.0f; m[10]= -(sfar + snear) / (sfar - snear); m[11]= -1.0f;
    m[12]= 0.0f; m[13]= 0.0f; m[14]= -2.0f * sfar * snear / (sfar - snear); m[15]= 0.0f;
    voxen_Shadow_System.shadDotThresh = 1.0f / vsqrtf(1.0f + vtan(sfov * PI / 360.0f) * (1.0f + aspect3D * aspect3D));
    mat4_lookat_from(view,&Sys_Global.instances[PLAYER1].rotation,Sys_Global.instances[PLAYER1].position);
    mul_mat4(viewProj,rasterPerspectiveProjection,view);
    invViewRot[0]=view[0]; invViewRot[1]=view[4]; invViewRot[2]=view[8]; invViewRot[3]=view[1]; invViewRot[4]=view[5]; invViewRot[5]=view[9]; invViewRot[6]=view[2]; invViewRot[7]=view[6]; invViewRot[8]=view[10];
    mat4_inverse(viewProj,invViewProj);
}

static __attribute__((hot)) void Render(bool camView, u8 camViewIdx) {
    u16 swidth = camView ? camViews[camViewIdx].width : Sys_Settings.ScreenWidth, sheight = camView ? camViews[camViewIdx].height : Sys_Settings.ScreenHeight;
    float sfov = camView ? (float)camViews[camViewIdx].fov : (float)Sys_Settings.FOV;
    float snear = camView ? camViews[camViewIdx].near : NEAR_PLANE; float sfar = camView ? camViews[camViewIdx].far : Sys_Global.farPlane[Sys_Global.curLev];
    Vector3 playerPos = Sys_Global.instances[PLAYER1].position;
    float px=playerPos.x, py=playerPos.y, pz=playerPos.z, aspect3D=(float)swidth / (float)sheight;
    float view[16],viewProj[16],invViewRot[9],invViewProj[16];
    GetProjections(view,viewProj,invViewRot,invViewProj,sfov,aspect3D,snear,sfar);
    ExtractFrustumPlanes(viewProj,playerFrustumPlanes);
    glBindVertexArray(Sys_Render.chunkVAO); // Common vao for RenderDynamicShadowmaps and Rasterized Geometry
    glEnable(GL_DEPTH_TEST);
    if (likely(Sys_Settings.Shadows > 0u)) RenderShadowmaps();
    UpdateLights(); // This is where the voxels get updated!
    for (int i=0;i<LIGHT_COUNT;++i) flag_set(&lights[i].lflags,LDIRTY,false);
    glViewport(0,0,swidth,sheight);
    ClearAll();
    glBindFramebuffer(GL_FRAMEBUFFER,Sys_Render.gBufferFBO);
    glEnable(GL_CULL_FACE); glDisable(GL_BLEND); // Opaques
    u16 visibleCount = 0, currentTexIndex = 0, currentNormIndex = 0, currentGlowIndex = 0, currentSpecIndex = 0, currentModelType = 0, opaqueCount = 0;
    bool skyVisible = (gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX); float distSqrd = sfar * sfar;
    DepthSort tmpTransparent[MAX_VISIBLE]; u16 tcnt = 0;
    for (u16 i = START_INDEX_LEVEL_INSTANCES; i < Sys_Global.loadedInstances; ++i) { // Determine base visibility
        if (!DetermineIfInstanceVisible(i,false,skyVisible,playerPos,&distSqrd)) continue;
       
        if (transparentTexture[Sys_Global.instances[i].texIndex]) { tmpTransparent[tcnt].index = i; tmpTransparent[tcnt].depth = distSqrd; tcnt++; }
        else { visibleInstances[opaqueCount].index = i; visibleInstances[opaqueCount].depth = distSqrd; opaqueCount++; }
    }

    MemCpyFromBtoAForNBytes(visibleInstances + opaqueCount,tmpTransparent,tcnt * sizeof(DepthSort));
    visibleCount = opaqueCount + tcnt;
    glUseProgram(Sys_Render.depthPrepassShaderProgram); // Depth Prepass - Eliminates some overdraw for ~6.1% performance improvement in spite of added draw calls
    glUniformMatrix4fv(2,1,0,viewProj);
    glEnable(GL_DEPTH_TEST); glColorMask(0,0,0,0); glDepthMask(1); glDepthFunc(0x0201/*GL_LESS*/); glDisable(GL_BLEND);
    if (opaqueCount > 1) qsort_new(visibleInstances,opaqueCount,sizeof(DepthSort),dsortInv);
    if (tcnt > 1) qsort_new(visibleInstances + opaqueCount,tcnt,sizeof(DepthSort),dsort);
    for (u16 visibleIndex = 0; visibleIndex < visibleCount; ++visibleIndex) {
        u16 i = visibleInstances[visibleIndex].index;
        Entity* e = &Sys_Global.instances[i]; u16 tex = e->texIndex;
        if (transparentTexture[tex]) { glEnable(GL_CULL_FACE); glEnable(GL_BLEND); } // Transparents (with sort)
        else if (doubleSidedTexture[tex] || e->scale.x < 0.0f || e->scale.y < 0.0f || e->scale.z < 0.0f) { glDisable(GL_CULL_FACE); glEnable(GL_BLEND); } // Doublesided
        else { glEnable(GL_CULL_FACE); glDisable(GL_BLEND); } // Opaque
       
        currentModelType = GetAndBindModel(i,currentModelType);
        glUniform1ui(3,(u32)tex);
        u32 vertCount = modelTriangleCounts[currentModelType] * 3;
        glDrawElements(0x0004/*GL_TRIANGLES*/,vertCount,GL_UNSIGNED_SHORT,0); drawCalls++; vertsRendered += vertCount;
    }

    glUseProgram(Sys_Render.chunkShaderProgram); /*Main Pass*/ glUniformMatrix4fv(2,1,0,viewProj); glUniform1ui(25,0u); // default constIndex
    bool grayscaleEnabled = ModRequestsGrayscale(), refOn = Sys_Settings.Reflections;              glUniform1ui(26,(u32)grayscaleEnabled);
    float fogActual = Sys_Global.fogColor[Sys_Global.curLev].a + (float)(Sys_Global.fogFac / 255u); // Alpha is base density for level.
    glUniform3f(12,Sys_Global.fogColor[Sys_Global.curLev].r * fogActual,Sys_Global.fogColor[Sys_Global.curLev].g * fogActual,Sys_Global.fogColor[Sys_Global.curLev].b * fogActual); // Fog Color(which is density)
    glUniform1ui(14,refOn); glUniform1ui(15,Sys_Settings.Shadows); glUniform2f(8,Sys_Global.worldMin_x[Sys_Global.curLev],Sys_Global.worldMin_z[Sys_Global.curLev]); 
    glUniform3f(10,playerPos.x,playerPos.y,playerPos.z);
    glColorMask(1,1,1,1);   glDepthMask(0);                        glDepthFunc(0x0203/*GL_LEQUAL*/); // Opaque Pass
    visibleCount = currentTexIndex = currentNormIndex = currentGlowIndex = currentSpecIndex = currentModelType = 0;
    glUniform1f(9,0.0f); // Reset heat for infrared vision
    for (u16 visibleIndex = 0; visibleIndex < opaqueCount; ++visibleIndex) { // Opaques (already front-to-back)
        u16 i = visibleInstances[visibleIndex].index;
        Entity* e = &Sys_Global.instances[i]; u16 tex = e->texIndex; u32 constIndex = e->index;
        if (transparentTexture[tex]) continue;
        else if (doubleSidedTexture[tex] || e->scale.x < 0.0f || e->scale.y < 0.0f || e->scale.z < 0.0f) { glDisable(GL_CULL_FACE); glEnable(GL_BLEND); } // Doublesided (either)
        else { glEnable(GL_CULL_FACE); glDisable(GL_BLEND); } // Opaque
        DRAW_ENTITY(currentNormIndex,currentTexIndex,currentGlowIndex,currentSpecIndex,currentModelType)
    }

    glDepthMask(1); currentTexIndex = currentNormIndex = currentGlowIndex = currentSpecIndex = currentModelType = 0; // Transparents Pass
    glUniform1f(9,0.0f); // Reset heat for infrared vision
    for (u16 visibleIndex = opaqueCount; visibleIndex < (opaqueCount + tcnt); ++visibleIndex) {
        u16 i = visibleInstances[visibleIndex].index;
        Entity* e = &Sys_Global.instances[i]; u16 tex = e->texIndex; u32 constIndex = e->index;
        if (transparentTexture[tex]) { glEnable(GL_CULL_FACE); glEnable(GL_BLEND); } // Transparents (with sort)
        else if (doubleSidedTexture[tex] || e->scale.x < 0.0f || e->scale.y < 0.0f || e->scale.z < 0.0f) { glDisable(GL_CULL_FACE); glEnable(GL_BLEND); } // Doublesided (either)
        else continue; // Opaque

        if ((constIndex >= 561 && constIndex <= 565) || (constIndex >= 568 && constIndex <= 573)) glDepthFunc(0x0202/*GL_EQUAL*/); // Cutouts
        else glDepthFunc(0x0203/*GL_LEQUAL*/); // Actual alphas
        DRAW_ENTITY(currentNormIndex,currentTexIndex,currentGlowIndex,currentSpecIndex,currentModelType)
    }
    
    if (camView) {
        glBindFramebuffer(0x8CA8/*GL_READ_FRAMEBUFFER*/,Sys_Render.gBufferFBO);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glBindTexture(GL_TEXTURE_2D,camViewTextures[camViewIdx]);
        glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0,0,0,swidth,sheight);
        glBindTexture(GL_TEXTURE_2D,0);
        return; // After copying render result, skip SSR and composite for camviews <<<<<<<<<<<<< CAM VIEW BARRIER
    }

    if (unlikely(Sys_Global.debugLineVertCount > 1)) DrawDebugLines(viewProj); // Draw Debug Lines
    glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D,Sys_Render.inputDepthID);
    if (likely(refOn > 0u)) { // Screen Space Reflections
        glUseProgram(Sys_Render.ssrShaderProgram); glUniform3f(3,playerPos.x,playerPos.y,playerPos.z); glUniform1i(5,3);
        glUniformMatrix4fv(6,1,0,invViewProj);     glUniformMatrix4fv(4,1,GL_FALSE,viewProj);
        u32 groupX_ssr = ((Sys_Settings.ScreenWidth / Sys_Settings.SSR_RES) + 31) / 32, groupY_ssr = ((Sys_Settings.ScreenHeight / Sys_Settings.SSR_RES) + 31) / 32;
        glDispatchCompute(groupX_ssr,groupY_ssr,1);
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER,Sys_Render.uiFBO); glViewport(0,0,1366,768);
    glDisable(GL_CULL_FACE);
    Sys_Global.last_time = RenderUI();
    if ((Sys_Global.inventoryMode && !Sys_Cheats.noHUD) || Sys_Global.menuActive || Sys_Global.gamePaused) RenderUIImage((i16)(Sys_Global.cursorPosition_x) - 20,(i16)(Sys_Global.cursorPosition_y) - 20,40,40,GetCursorTexture());
    else RenderUIImage(663,364,40,40,GetCursorTexture()); // Centered on UI baseline resolution 1366x768
    glBindFramebuffer(GL_FRAMEBUFFER,0); glViewport(0,0,swidth,sheight); // Restore normal output size for final composite blit

    glUseProgram(Sys_Render.imageBlitShaderProgram);
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D,Sys_Render.inputImageID);
    glUniform1i(4,4); // outputImage texture sampler2D, don't remember why when active texture is texture 0. meh.... oh maybe to not read and write same binding?
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D,Sys_Render.inputUIID);
    glUniform1i(31,1); glUniform1i(32,3); glUniformMatrix4fv(33,1,0,invViewProj);
    double berserkTimeRemainingNormalized = Sys_Global.invP1.berserkFinishedTime > 0.0001 ? (Sys_Global.invP1.berserkFinishedTime - Sys_Global.pauseRelativeTime) / BERSERK_TIME : 0.0;
    if (Sys_Global.invP1.berserkFinishedTime < Sys_Global.pauseRelativeTime && Sys_Global.invP1.berserkFinishedTime > 0.0001) Sys_Global.invP1.berserkFinishedTime = berserkTimeRemainingNormalized = 0.0;
    glUniform1ui(5,refOn);           glUniform1ui(6,Sys_Settings.FXAA);                          glUniform1f(14,Sys_Settings.FOV);
    glUniform1f(16,aspect3D);        glUniform1ui(22,Sys_Settings.Shadows);                      glUniform1f(9,(float)berserkTimeRemainingNormalized);
    glUniform1f(10,berserkSeedTime); glUniform1ui(11,Sys_Settings.Brightness);                   glUniform3f(12,deg2rad(cam_yaw),deg2rad(cam_pitch),deg2rad(cam_roll));
    glUniform3f(13,px,py,pz);        glUniform1f(15,(float)Sys_Global.pauseRelativeTime * 0.1f); 
    glUniform1ui(17,(gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX) || Sys_Global.curLev == LEVEL_CYBERSPACE);
    glUniform1ui(18,(gridCellStates[playerCellIdx] & CELL_SEES_SUN) && Sys_Global.curLev != LEVEL_CYBERSPACE);
    glUniform1ui(19,((Sys_Global.curLev >= 10 && Sys_Global.curLev < LEVEL_CYBERSPACE) ? 1u : 0u) && (gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX));
    u32 shieldOnType = 0u; // No shield green tint.
    if (Sys_Global.instances[WORLD].ioflags & QUESTBIT_SHIELD_ACTIVATED) {
        if (Sys_Global.curLev == 6 || Sys_Global.curLev == 7) shieldOnType = 2u; // Shielding only below player for lower levels.
        else if (Sys_Global.curLev <= 5) shieldOnType = 1u; // Shielding everywhere as levels fully within shield.
    }
   
    glUniform1ui(20,shieldOnType);
    Color painStaticColor = GetPainStaticColor(); glUniform3f(23,painStaticColor.r,painStaticColor.g,painStaticColor.b);
    glUniformMatrix4fv(24,1,0,viewProj);          glUniformMatrix3fv(25,1,0,invViewRot);        glUniform1i(27,0); // Texture 0 for the rendered geometry color buffer
    glUniform1f(28,GetPainStatic());              glUniform1ui(29,(u32)ModRequestsGrayscale()); // Grayscale
    glBindVertexArray(Sys_Render.quadVAO);
    glDisable(GL_DEPTH_TEST);
    glDrawArrays(0x0006/*GL_TRIANGLE_FAN*/,0,4);
    drawCalls++; vertsRendered += 4;
    if ((Sys_Global.last_time - Sys_Global.lastFrameSecCountTime) >= 1.00) { // Update Diagnostic Poll
        Sys_Global.lastFrameSecCountTime = Sys_Global.last_time;
        Sys_Global.framesPerLastSecond = Sys_Global.frame - Sys_Global.lastFrameSecCount;
        if (Sys_Global.framesPerLastSecond < Sys_Global.worstFPS && Sys_Global.frame > 2000) Sys_Global.worstFPS = Sys_Global.framesPerLastSecond; // After startup, keep track of worst framerate seen.
        Sys_Global.lastFrameSecCount = Sys_Global.frame;
    }
}

void RenderCameraViews() { // Render in-world camera views.  Pops player position to elsewhere, renders to tiny fbo, pops player back.
    if (unlikely(Sys_Global.gamePaused || camViewCount == 0)) return;
    
    Vector3 tempPlayerPos = Sys_Global.instances[PLAYER1].position; Quaternion tempPlayerRot = Sys_Global.instances[PLAYER1].rotation;
    for (int cm=0;cm<camViewCount;++cm) {
        if (camViews[cm].finished < Sys_Global.pauseRelativeTime && camViews[cm].visible) {
            camViews[cm].finished = Sys_Global.pauseRelativeTime + 0.5f;
            Sys_Global.instances[PLAYER1].position = camViews[cm].position; Sys_Global.instances[PLAYER1].rotation = camViews[cm].rotation;
            CullCore();
            Render(true/*camview*/,cm); // Ok culling and light clusters (in voxels) have been updated, now render the view.
        }
    }

    Sys_Global.instances[PLAYER1].position = tempPlayerPos; Sys_Global.instances[PLAYER1].rotation = tempPlayerRot; // Restore player for normal render.
}

void UpdateInstanceMatrix4x4s() {
    if (unlikely(Sys_Global.gamePaused || Sys_Global.menuActive)) return;
    
    i32 dirtyMin = -1, dirtyMax = -1;
    for (u32 i = START_INDEX_LEVEL_INSTANCES; i < Sys_Global.loadedInstances; i++) {        
        Entity *e = &Sys_Global.instances[i];
        float x=e->rotation.x, y=e->rotation.y, z=e->rotation.z, w=e->rotation.w;
        float x2=x*x, y2=y*y, z2=z*z, xy=x*y, xz=x*z, yz=y*z, wx=w*x, wy=w*y, wz=w*z;
        float sclx=e->scale.x, scly=e->scale.y, sclz=e->scale.z;
        u32 m = i*16;
        modelMatrices[m+0] =(1.0f-2.0f*(y2+z2))*sclx; modelMatrices[m+1] =      (2.0f*(xy+wz))*sclx; modelMatrices[m+2] =      (2.0f*(xz-wy))*sclx;
        modelMatrices[m+4] =      (2.0f*(xy-wz))*scly; modelMatrices[m+5] =(1.0f-2.0f*(x2+z2))*scly; modelMatrices[m+6] =      (2.0f*(yz+wx))*scly;
        modelMatrices[m+8] =      (2.0f*(xz+wy))*sclz; modelMatrices[m+9] =      (2.0f*(yz-wx))*sclz; modelMatrices[m+10]=(1.0f-2.0f*(x2+y2))*sclz;
        modelMatrices[m+12]=e->position.x; modelMatrices[m+13]=e->position.y; modelMatrices[m+14]=e->position.z; modelMatrices[m+15]=1.0f;
        if (dirtyMin < 0) dirtyMin = (i32)i;
        dirtyMax = (i32)i;
    }
    if (dirtyMin < 0) return;
    glBindBuffer(GL_SSBO,Sys_Render.matricesBufferID);
    u32 offsetFloats = (u32)dirtyMin * 16;
    u32 countFloats  = ((u32)dirtyMax - (u32)dirtyMin + 1) * 16;
    glBufferSubData(GL_SSBO, offsetFloats * sizeof(float), countFloats * sizeof(float), modelMatrices + offsetFloats);
}
// ================= Initialization && Main
ENGINE_TO_MOD void InitializeEntity(Entity* entry) { // Blank entity, no index yet, for initial list population or temporary Entity. memset here would be harmful as only a handful of fields are the same.
    MemSetToVForNBytes(entry,0,sizeof(Entity));
    entry->index = U16_MAX;               entry->entflags = EF_ACTIVE; entry->kinematic = true;
    entry->layer = Layer_Default;         entry->camView = 255;             entry->rotation.w = 1.0f;
    entry->tickTime = 0.35f;          entry->angularDrag = 0.05f;
    entry->modelIndex = entry->lodIndex  = entry->colliderMeshIndex            = MODEL_IDX_MAX;
    entry->texIndex   = entry->glowIndex = entry->specIndex = entry->normIndex = MAX_VALID_TEXTURE;
    entry->scale.x    = entry->scale.y   = entry->scale.z   = entry->mass      = entry->volume = 1.0f;
    entry->dynamicFriction = entry->staticFriction = 0.6f;
}

FHandle levelFileHandle;
ENGINE_TO_MOD void LoadLevel(u8 curlevel) {
    double start_time = get_time();
    DebugRAM("start of LoadLevel");
    Sys_Global.levelCurrentlyLoading = true; Sys_Global.gamePaused = false; Sys_Global.menuActive = false;
    RenderLoadingProgress(100,"Loading level...");
    MemSetToVForNBytes(lights,0,LIGHT_COUNT * sizeof(Light)); MemSetToVForNBytes(lanims,0,LIGHT_COUNT * sizeof(LightAnimation)); loadedLights = 0;
    MemSetToVForNBytes(alreadyReadLightOnOnce,0,sizeof(alreadyReadLightOnOnce));
    MemSetToVForNBytes(modelMatrices,0,INSTANCE_COUNT * 16 * sizeof(float)); // Matrix4x4 = 16
    MemSetToVForNBytes(camViews,0,64 * sizeof(CamView)); camViewCount = 0;
    MemSetToVForNBytes(Sys_Global.instances + 3,0,(INSTANCE_COUNT - 3) * sizeof(Entity)); // Initialize instances, the global entity array for the currently loaded level.
    char filename[20]; // Minimum size for 0 through 13.
    StringFormat(filename,sizeof(filename),"./Data/level%d.txt",curlevel);
    levelFileHandle = OS_OpenReadonly(filename);
    LoadLevelMod(curlevel);
    OS_Close(levelFileHandle);
    for (int i=0;i<loadedLights;++i) lightsNewPosition[i]=lights[i].pos;
    DualLog("Loaded %d entities, %u static lights for Level %d... took %f secs\n",Sys_Global.loadedInstances,loadedLights,curlevel,get_time() - start_time);
    RenderLoadingProgress(110,"Initialize entities...");
    for (int i=PLAYER1;i<Sys_Global.loadedInstances;++i) {
        Entity* e = &Sys_Global.instances[i];
        i32 cellIdx = PosGetCellCoords(e->position.x,e->position.z);
        e->cellIndex = cellIdx; e->cellX=PosGetCellCoordX(e->position.x); e->cellZ=PosGetCellCoordZ(e->position.z);
        e->radius = modelBounds[e->modelIndex]*vmax(vmax(e->scale.x,e->scale.y),e->scale.z);
        e->shadRadius = e->radius * 1.41;
        ComputeConvexMeshInertiaTensor(e);
        if (e->mass < 0.001f && e->collider != COLTYPE_NONE && e->collider != COLTYPE_MSH && (e->entflags & EF_RIGIDBODY)) e->mass = 0.2f; // At least something!
    }
    
    ModInitAfterLoad(); ResetLevelAudio(); ResetLevelMusic(); creditPages = GetCreditsText();
    RenderLoadingProgress(110,"Loading cull system...");
    CullInit(); // Must be after level! MUST BE AFTER SortInstances!!
    glUseProgram(Sys_Render.voxelUpdateShaderProgram);
    glUniform2f(0,Sys_Global.voxelMinCenterX[Sys_Global.curLev],Sys_Global.voxelMinCenterZ[Sys_Global.curLev]);
    glUniform1f(1,Sys_Global.farPlane[Sys_Global.curLev] * Sys_Global.farPlane[Sys_Global.curLev]);
    glUniform1ui(2,loadedLights);
    glUniform2f(3,Sys_Global.worldMin_x[Sys_Global.curLev],Sys_Global.worldMin_z[Sys_Global.curLev]); 
    RenderLoadingProgress(120,"Loading voxel lighting data...");
    for (u16 i = 0; i < loadedLights; i++) { lightsNewPosition[i] = lights[i].pos; }
    MemSetToVForNBytes(voxen_Shadow_System.shadowmapIndirectionList,MAX_SHADOWMAPS + 1,loadedLights * sizeof(u32)); // Set to invalid values for all
    Sys_Global.levelCurrentlyLoading = false;
    DebugRAM("end of LoadLevel");
}

__attribute__((cold)) void NewGame() { // Reset World States
    DualLog("Loading new game...\n");
    RenderLoadingProgress(100,"Loading new game...");
    Sys_Global.menuActive = Sys_Global.gamePaused = enteringPlayerName = fovSliderActive = gammaSliderActive = masterVolumeSliderActive = musicVolumeSliderActive = messageVolumeSliderActive = sfxVolumeSliderActive = returnToPause = false;
    SetGlobalsModData();
    currentMenuItem = currentMenuTab = 0; currentMenuPage = Mpg_FrontPage;
    Sys_Global.pauseRelativeTime = Sys_Global.last_physics_time = 0.0;
    Sys_Global.inventoryMode = false;
    MemSetToVForNBytes(Sys_Global.instances,0,2 * sizeof(Entity)); // Blank out player entities
    PlayerInit(PLAYER1); PlayerInit(PLAYER2);
    Sys_Global.instances[WORLD].ioflags = 0u;
    cam_yaw = 90.0f; cam_pitch = 0.0f; cam_roll = 0.0f;
    Sys_Global.inventoryMode = Sys_Settings.NoShootMode;
    Sys_Global.pauseRelativeTime =  Sys_Global.last_physics_time = 0.0;
    Sys_Global.last_topframe_time = Sys_Global.last_physics_time - 0.05;
    Sys_Global.deltaTime = 0.0166666666f;
    Sys_Global.gameFinished = Sys_Global.creditsActive = Sys_Global.decoyActive = false;
	Sys_Global.ressurections = Sys_Global.deaths = Sys_Global.kills = Sys_Global.cyberkills = 0u;
	Sys_Global.shotsFired = Sys_Global.grenadesThrown = Sys_Global.savesScummed = 0U;
    Sys_Global.damageDealt = Sys_Global.damageReceived = 0.0f;
	Sys_Global.creditsPageIndex = 0u;
    for (int i=0;i<14;++i) Sys_Global.levelSecurity[i] = 100u;
    InputClearRisingAndFallingEdges();
    Sys_Input.currentMouse_dx = Sys_Input.currentMouse_dy = 0;
    Sys_Input.last_mouse_x = Sys_Input.last_mouse_y = 0;
    Sys_Input.ignore_next_mouse_delta = true;
    Sys_Input.isCapsLockOn = false; // As far as we're concerned, don't worry about OS state.
    Sys_Input.lastUse = false;
    LoadLevel(Sys_Global.startLevel); // Must be after entities!
    ModNewGame();
}

void GoIntoGame() { NewGame(); PlayGameMusic(); DualLog("Player named \"%s\" started the game!\n", Sys_Global.playerName); }
void* mod_handle = NULL;
void InitalizeEnvironment() {
    double game_start_time = get_time();
    random_range_rng = (u32)game_start_time; // Seed global rand uniquely with time since system boot.
    console_log_file = OS_OpenWriteonly("./voxen.log"); // Initialize log system for all prints to go to both stdout and voxen.log file
    DebugRAM("program start");
    DualLog("Voxen, the Voxel Lit Open Source Game Engine by W. Josiah Jack, MIT-0 licensed\nEntity size: %u\n",sizeof(Entity));
    WindowInit();
    Sys_Global.frame=0,Sys_Global.menuActive=true,Sys_Global.screenshotTimeout=1.0,Sys_Global.creditsPageIndex=1,Sys_Global.difficultyCombat=Sys_Global.difficultyCyber=Sys_Global.difficultyPuzzle=Sys_Global.difficultyMission=2,Sys_Global.deaths=0,Sys_Global.worstFPS=0,Sys_Global.cursorPosition_x=680,Sys_Global.cursorPosition_y=384;
    DualLog("Loading game definition...");
    FHandle gmFP = OS_OpenReadonly("./Data/gamedata.txt");
    if (!gmFP) { DualLogError("\nCannot open ./Data/gamedata.txt\n"); OS_Exit(1);  }
    
    char gmLine[512],global_modname[256],global_dllname[256]; u32 lineNum = 0;
    while (GetNextStringUpToNewlineOrEOF(gmLine,sizeof(gmLine),gmFP)) {
        lineNum++;
        char* s = data_parser_trim(gmLine); if (*s == 0 || (s[0] == '/' && s[1] == '/')) continue;
        char* colon = StringFindFirstCharWithin(s, ':'); if (!colon) continue;
        *colon = '\0'; char* key = data_parser_trim(s); char* val = data_parser_trim(colon + 1); if (*key == 0 || *val == 0) continue;

             if (StringsEqual(key,   "modname")) {  StringCopyInto_A_From_B(global_modname,val,sizeof(global_modname)); }                      else if (StringsEqual(key,   "dllname")) { StringCopyInto_A_From_B(global_dllname,val,sizeof(global_dllname)); }
        else if (StringsEqual(key,"windowicon")) { StringCopyInto_A_From_B(Sys_Global.global_winicon,val,sizeof(Sys_Global.global_winicon)); } else if (StringsEqual(key,"levelcount")) { Sys_Global.numLevels = parse_numberu8(val,gmLine,lineNum); }
        else if (StringsEqual(key,"startlevel")) { Sys_Global.startLevel = parse_numberu8(val,gmLine,lineNum); }
    }
    
    OS_Close(gmFP); DualLog(" %s:: num levels: %d, start level: %d\n",global_modname,Sys_Global.numLevels,Sys_Global.startLevel);
    LoadConfig(); // Get settings before setting window size.
    window = VCreateWindow(Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight,&global_modname[0]);
    CenterWindowOnMonitor();
    SetGLContext_GetFunctionPointers();
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); ((_GLFWwindow*)window)->context.swapBuffers(((_GLFWwindow*)window)); // Black out the window as early as possible for better presentation.
    i32 major=0,minor=0; glGetIntegerv(0x821B/*GL_MAJOR_VERSION*/,&major); glGetIntegerv(0x821C/*GL_MINOR_VERSION*/,&minor);
    if (major < 4 || (major == 4 && minor < 3)) { DualLogError("Need OpenGL >= 4.3, got %d.%d\n",major,minor); OS_Exit(1); }
    
    glFrontFace(0x0901/*GL_CCW*/); // Set triangle winding order
    glBlendFuncSeparate(0x0302/*GL_SRC_ALPHA*/, 0x0303/*GL_ONE_MINUS_SRC_ALPHA*/, 1, 0x0303/*GL_ONE_MINUS_SRC_ALPHA*/);
    CompileShaders();
    u32 vaos[4],vbos[4]; glGenVertexArrays(4,vaos); glGenBuffers(4,vbos);
    Sys_Render.quadVAO = vaos[0]; Sys_Render.chunkVAO = vaos[1]; Sys_Render.textVAO = vaos[2]; Sys_Render.debugLinesVAO = vaos[3];
    Sys_Render.quadVBO = vbos[0]; Sys_Render.chunkVBO = vbos[1]; Sys_Render.textVBO = vbos[2]; Sys_Render.debugLinesVBO = vbos[3]; 
    float quadBlit_vertices[] = {1.0f,-1.0f,1.0f,0.0f, 1.0f,1.0f,1.0f,1.0f, -1.0f,1.0f,0.0f,1.0f, -1.0f,-1.0f,0.0f,0.0f}; // 4 verts, 4 floats each x,y,u,v
    glBindVertexArray(Sys_Render.quadVAO); glBindBuffer(GL_ARRAY_BUFFER,Sys_Render.quadVBO); glBufferData(GL_ARRAY_BUFFER,sizeof(quadBlit_vertices),quadBlit_vertices,GL_STATIC_DRAW);
    glVertexAttribFormat(0,2,GL_FLOAT,GL_FALSE,0);                 glVertexAttribBinding(0,0); glEnableVertexAttribArray(0); // pos xy float @ offset 0
    glVertexAttribFormat(1,2,GL_FLOAT,GL_FALSE,2 * sizeof(float)); glVertexAttribBinding(1,0); glEnableVertexAttribArray(1); // uv (s,t)
    glBindVertexBuffer(0,Sys_Render.quadVBO,0,4 * sizeof(float));
    glBindVertexArray(Sys_Render.chunkVAO);
    glVertexAttribFormat(0,3,0x140B/*GL_HALF_FLOAT*/,GL_FALSE,0);  glVertexAttribBinding(0,0); glEnableVertexAttribArray(0); // pos xyz half-float @ offset 0
    glVertexAttribFormat(1,3,0x140B/*GL_HALF_FLOAT*/,GL_FALSE,6);  glVertexAttribBinding(1,0); glEnableVertexAttribArray(1); // normal xyz float   @ offset 6  (after 3×2 bytes)
    glVertexAttribFormat(2,2,0x140B/*GL_HALF_FLOAT*/,GL_FALSE,12); glVertexAttribBinding(2,0); glEnableVertexAttribArray(2); // uv st float
    glBindVertexBuffer(0,Sys_Render.chunkVBO,0,14);
    glBindVertexArray(Sys_Render.textVAO);
    glVertexAttribFormat(0,3,GL_FLOAT,GL_FALSE,0);                 glVertexAttribBinding(0,0); glEnableVertexAttribArray(0); // pos (x,y,z) 4 floats per vertex, stride = 4*sizeof(float)
    glVertexAttribFormat(1,2,GL_FLOAT,GL_FALSE,3 * sizeof(float)); glVertexAttribBinding(1,0); glEnableVertexAttribArray(1); // uv (s,t)
    glBindVertexBuffer(0, Sys_Render.textVBO,0,5 * sizeof(float));
    glBindVertexArray(Sys_Render.debugLinesVAO); glBindBuffer(GL_ARRAY_BUFFER,Sys_Render.debugLinesVBO); glBufferData(GL_ARRAY_BUFFER,MAX_DEBUG_LINE_VERTS * 2 * sizeof(DebugLineVertex),NULL,GL_DYNAMIC_DRAW);
    glVertexAttribFormat(0,3,GL_FLOAT,GL_FALSE,__builtin_offsetof(DebugLineVertex,x)); glVertexAttribBinding(0,0); glEnableVertexAttribArray(0);
    glVertexAttribFormat(1,4,GL_FLOAT,GL_FALSE,__builtin_offsetof(DebugLineVertex,r)); glVertexAttribBinding(1,0); glEnableVertexAttribArray(1);
    glBindVertexBuffer(0,Sys_Render.debugLinesVBO,0,sizeof(DebugLineVertex));
    InitFontAtlasses();
    GenerateAndBindTexture(&Sys_Render.inputUIID,GL_RGBA8,1366,768,GL_RGBA,GL_UNSIGNED_BYTE,0x2600/*GL_NEAREST*/,NULL); // UI Fixed Size Raster
    glGenFramebuffers(1,&Sys_Render.uiFBO);
    glBindFramebuffer(GL_FRAMEBUFFER,Sys_Render.uiFBO);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D,Sys_Render.inputUIID); glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,Sys_Render.inputUIID,0);
    u32 drawBuffersUI[] = {GL_COLOR_ATTACHMENT0}; glDrawBuffers(1,drawBuffersUI);
    u32 uistatus = glCheckFramebufferStatus(GL_FRAMEBUFFER); if (uistatus != 0x8CD5/*GL_FRAMEBUFFER_COMPLETE*/) DualLogError("UI Framebuffer incomplete: Error code %d\n",uistatus);
    glBindImageTexture(0,Sys_Render.inputUIID,0,GL_FALSE,0,GL_READ_WRITE,GL_RGBA8);/* UI Rendered Color*/ glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,Sys_Render.inputUIID,0);
    RenderLoadingProgress(40,"Loading...");
    float* m = shadowmapsPerspectiveProjection; float lightRangeMax=15.36f; float viewRange=(lightRangeMax - NEAR_PLANE);
    m[0]=1.0f; m[1]=0.0f; m[2]=0.0f; m[3]=0.0f; m[4]=0.0f; m[5]=1.0f; m[6]=0.0f; m[7]=0.0f; m[8]=0.0f; m[9]=0.0f; m[10]=-(lightRangeMax + NEAR_PLANE) / viewRange; m[11]=-1.0f; m[12]=0.0f; m[13]=0.0f; m[14]=-2.0f * lightRangeMax * NEAR_PLANE / viewRange; m[15]=0.0f;
    InitSCFTables(); InitAudio();
    char mod_path[256]; StringCopyInto_A_From_B(mod_path,"./",256); StringConcatenate(mod_path,global_dllname,256); StringConcatenate(mod_path,MOD_EXTENSION,256);
    mod_handle = OS_DlOpen(mod_path); if (!mod_handle) { DualLogError("Failed to load mod at:%s",mod_path); OS_Exit(1); }
    
    #define X(ret, name, params) name = (ret (*) params)OS_DlSym(mod_handle,#name); if(!name) DualLogError("Failed to load mod function: %s",#name);
    MOD_FUNCTION_LIST(X)
    #undef X
    ModLink(&Sys_Global,&Sys_Cheats,&Sys_Settings,&Sys_Text,&Sys_UI); // Link engine to mod
    Sys_Global.GetKey=GetKey; Sys_Global.GetKeyPressed=GetKeyPressed; // Link mod to engine
    ModEntityDefinitionsInitAfterLoad();
    glGenFramebuffers(1,&Sys_Render.gBufferFBO);
    ApplySettings(); // After loading of text and game data.
    glBindFramebuffer(GL_FRAMEBUFFER,Sys_Render.gBufferFBO);
    u32 drawBuffers[] = {GL_COLOR_ATTACHMENT0,GL_COLOR_ATTACHMENT1,GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3,drawBuffers);
    u32 status = glCheckFramebufferStatus(GL_FRAMEBUFFER); if (status != 0x8CD5/*GL_FRAMEBUFFER_COMPLETE*/) DualLogError("Framebuffer incomplete: Error code %d\n",status);
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    float mat[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    MemCpyFromBtoAForNBytes(&modelMatrices[0],mat,16 * sizeof(float)); // Null instance matrix used for UI
    Sys_Render.matricesBufferID        = SetupSSBO(&Sys_Render.matricesBufferID,        1,INSTANCE_COUNT * 16 * sizeof(float),modelMatrices, GL_STATIC_DRAW);
    Sys_Render.voxelLightListCountsID  = SetupSSBO(&Sys_Render.voxelLightListCountsID,  2,VOXEL_COUNT * sizeof(u32),NULL,GL_STATIC_DRAW);
    Sys_Render.voxelLightListsID       = SetupSSBO(&Sys_Render.voxelLightListsID,       3,VOXEL_COUNT * MAX_LIGHTS_PER_VOXEL * sizeof(u32),NULL,GL_STATIC_DRAW);
    Sys_Render.lightsID                = SetupSSBO(&Sys_Render.lightsID,                4,LIGHT_COUNT * sizeof(Light),NULL,GL_STATIC_DRAW);
    if (Sys_Settings.Shadows) CreateShadowBuffers();                               // 5,6
    Sys_Render.cellVisibleDataID       = SetupSSBO(&Sys_Render.cellVisibleDataID,       7,ARRSIZE * sizeof(u32),NULL,GL_STATIC_DRAW);
    Sys_Render.texturePalettesID       = SetupSSBO(&Sys_Render.texturePalettesID,       8,MAX_UNIQUE_COLORS * sizeof(u32),NULL,GL_STATIC_DRAW);
    Sys_Render.texturePaletteOffsetsID = SetupSSBO(&Sys_Render.texturePaletteOffsetsID, 9,MAX_VALID_TEXTURE * sizeof(u32),NULL,GL_STATIC_DRAW);
    Sys_Render.colorBufferID           = SetupSSBO(&Sys_Render.colorBufferID,          12,MAX_TOTAL_PIXELS * sizeof(u8),NULL,GL_STATIC_DRAW);
    Sys_Render.textureOffsetsID        = SetupSSBO(&Sys_Render.textureOffsetsID,       14,MAX_VALID_TEXTURE * sizeof(u32),NULL,GL_STATIC_DRAW);
    Sys_Render.textureSizesID          = SetupSSBO(&Sys_Render.textureSizesID,         15,MAX_VALID_TEXTURE * 2 * sizeof(i32),NULL, GL_STATIC_DRAW);
    glUseProgram(Sys_Render.shadowmapsShaderProgram);  glUniform1ui( 9,SHADOW_MAP_SIZE);     glUseProgram(Sys_Render.shadowmapsClearShaderProgram); glUniform1ui(1,SHADOW_MAP_SIZE);
    glUseProgram(Sys_Render.chunkShaderProgram);       glUniform1ui(21,SHADOW_MAP_SIZE); glUniform1f(22,(float)SHADOW_MAP_SIZE); glUniform1ui(23,LIGHT_COUNT); glUniform1ui(24,(u32)MAX_LIGHTS_PER_VOXEL); glUniform1ui(11,SHADOW_MAP_SIZE*SHADOW_MAP_SIZE);
    glUseProgram(Sys_Render.voxelUpdateShaderProgram); glUniform1ui( 4,SHADOW_MAP_SIZE); glUniform1ui(6,(u32)MAX_LIGHTS_PER_VOXEL);
    RenderLoadingProgress(100,"Loading textures..."); LoadTextures();
    RenderLoadingProgress(92,"Loading models...");    LoadModels();
    if (Sys_Global.introNotPlayed) {} // TODO: Play intro
    Sys_Global.absoluteTime = Sys_Global.last_topframe_time = Sys_Global.current_time = get_time();
    Sys_Global.pauseRelativeTime = Sys_Global.last_physics_time = 0.0;
    NewGame();
    OpenMainMenu(); // Comment out for immediate testing
    DebugRAM("InitializeEnvironment end"); DualLog("Game Initialized in %f secs\n",get_time() - game_start_time);
}

i32 main() { // Write the Code That Just Does the Thing(TM)
    InitalizeEnvironment();
    while(1) {
        if (queuedLevelToLoad != 255u) { LoadLevel(queuedLevelToLoad); queuedLevelToLoad = 255u; continue; }

        Sys_Global.current_time  = get_time();              Sys_Global.deltaTime          = Sys_Global.current_time - Sys_Global.last_topframe_time;
        Sys_Global.absoluteTime += Sys_Global.deltaTime;    Sys_Global.last_topframe_time = Sys_Global.current_time;
        if (!Sys_Global.gamePaused && !Sys_Global.menuActive) Sys_Global.pauseRelativeTime += Sys_Global.deltaTime;
        InputProcessing(); // Before anims and physics to allow them to respond immediately.
        UpdateAnims();     // Before physics to allow model swap out to affect physics state immediately.  Before rendering to affect shadowmaps immediately.
        Physics();
        ModUpdate();       // After physics so mod/gamecode can modify velocities before next frame.
        UpdateAmbientSounds(); UpdateMusic();
        drawCalls = uiDrawCalls = shadDrawCalls = vertsRendered = 0;
        RenderCameraViews();
        CullCore(); UpdateInstanceMatrix4x4s();
        Render(false/*!camview*/,0u);
        CheckAndTakeScreenshot();
        Sys_Global.frame++;
        InputClearRisingAndFallingEdges();
        Sys_Global.cpuTime = get_time() - Sys_Global.current_time; // Measure time over everything this frame before GPU swap buffers for diagnostic text.
        ((_GLFWwindow*)window)->context.swapBuffers(((_GLFWwindow*)window)); // Present frame (almost always waiting for GPU since GPU bound).
        CHECK_GL_ERROR();  // Lone catch for issues, I sprinkle this around when troubleshooting introduced issues.
        #ifdef DEBUG_RAM_OUTPUT
        { static const u32 dbgFrm[] = {4,100,200,500,1000}; static const char* dbgLbl[] = {"frame 4","frame 100","frame 200","frame 500","frame 1000"}; for (int d=0;d<5;d++) if (Sys_Global.frame == dbgFrm[d]) {DebugRAM(dbgLbl[d]); break;} }
        #endif
    }
    return 0;
}
