// voxen.c - A realtime OpenGL 4.3+ Game Engine for Citadel: The System Shock Fan Remake.  Main translation unit.  Core renderer.  OS Shim Layer.
#define GAME_TITLE "Citadel"
#define WIN_ICON "./Textures/UI/menudot1.png"
#include "common.h"
typedef __UINTPTR_TYPE__ uintptr_t; typedef __INTPTR_TYPE__ intptr_t;
#define likely(x)   __builtin_expect(!!(x),1)
#define unlikely(x) __builtin_expect(!!(x),0)
#define NULL ((void *)0)
#define assert(cond) do { if (!(cond)) { DualLogError("[%s:%d]:%s(): Assert fail:%s\n",__FILE__,__LINE__,__func__,#cond); *(volatile int*)0 = 0; } } while(0) // Force a crash for debug
char* sFindSub(const char* haystack, const char* needle);
#if defined(_WIN32)
    #define WINDOWS
    #define MOD_EXTENSION ".dll" // e.g. Citadel.dll
    #define OS_DlOpen(path)       LoadLibraryA(path)
    #define OS_DlSym(handle,name) GetProcAddress((handle),(name))
    #define DLL_IMP __declspec (dllimport)
    #define WINAPI __stdcall
    #define INVALID_FHANDLE ((void*) (i64)-1)
    typedef void* FHandle; typedef i64 (WINAPI *PROC)(); typedef i64 (WINAPI *FARPROC)(); typedef i64 (WINAPI *NEARPROC)();
    typedef struct { unsigned long Data1; u16 Data2,Data3; u8 Data4[8]; } GUID; typedef struct { int unused; } *HINSTANCE; typedef HINSTANCE HMODULE;  typedef struct { u32 nLength; void* lpSecurityDescriptor; i32 bInheritHandle; } *LPSECURITY_ATTRIBUTES;
    typedef struct { i64 QuadPart; } LARGE_INTEGER; typedef LARGE_INTEGER *PLARGE_INTEGER; typedef struct { u64 Internal,InternalHigh; union {struct {u32 Offset,OffsetHigh;} DUMMYSTRUCTNAME; void* Pointer;} DUMMYUNIONNAME; void* hEvent; } OVERLAPPED, *LPOVERLAPPED;
    typedef struct { union { u32 dwOemId; struct { u16 wProcessorArchitecture,wReserved; } DUMMYSTRUCTNAME; } DUMMYUNIONNAME; u32 dwPageSize; void* lpMinimumApplicationAddress,*lpMaximumApplicationAddress; u64 dwActiveProcessorMask; u32 dwNumberOfProcessors,dwProcessorType,dwAllocationGranularity; u16 wProcessorLevel,wProcessorRevision; } SYSTEM_INFO, *LPSYSTEM_INFO;
    DLL_IMP void* WINAPI CreateFileMappingA(void*,LPSECURITY_ATTRIBUTES,u32,u32,u32,const char*); DLL_IMP i32 WINAPI VirtualFree(void*,u64,u32);    DLL_IMP void* WINAPI VirtualAlloc(void*,u64,u32,u32);         DLL_IMP i32 WINAPI ReadFile(void*,void*,u32,u32*,LPOVERLAPPED);    DLL_IMP i32 WINAPI GetFileSizeEx(void*,PLARGE_INTEGER);          DLL_IMP i32 WINAPI UnmapViewOfFile(void*); DLL_IMP FARPROC WINAPI GetProcAddress(HINSTANCE,const char*);
    DLL_IMP void* WINAPI CreateFileA(const char*,u32,u32,LPSECURITY_ATTRIBUTES,u32,u32,void*);    DLL_IMP void* WINAPI GetStdHandle(u32);           DLL_IMP i32 WINAPI QueryPerformanceCounter(LARGE_INTEGER*);   DLL_IMP void* WINAPI MapViewOfFileEx(void*,u32,u32,u32,u64,void*); DLL_IMP i32 WINAPI WriteFile(void*,void*,u32,u32*,LPOVERLAPPED); DLL_IMP i32 WINAPI CloseHandle(void*);     DLL_IMP __declspec (noreturn) void WINAPI ExitProcess(u32);
    DLL_IMP i32 WINAPI SetFilePointerEx(void*,LARGE_INTEGER,PLARGE_INTEGER,u32);                  DLL_IMP void WINAPI GetSystemInfo(LPSYSTEM_INFO); DLL_IMP i32 WINAPI QueryPerformanceFrequency(LARGE_INTEGER*); DLL_IMP void* WINAPI MapViewOfFile(void*,u32,u32,u32,u64);         DLL_IMP HINSTANCE WINAPI LoadLibraryA(const char*);              DLL_IMP void* WINAPI CreateFileMappingW(void*,LPSECURITY_ATTRIBUTES,u32,u32,u32,u16*);
    struct timespec { i64 tv_sec; i32 tv_nsec; }; struct sched_param { int sched_priority; }; typedef uintptr_t pthread_t; typedef intptr_t pthread_mutex_t,pthread_cond_t; typedef int pthread_condattr_t; typedef u32 pthread_mutexattr_t; typedef struct pthread_attr_t { unsigned p_state; void *stack; size_t s_size; struct sched_param param; } pthread_attr_t;
    int pthread_create(pthread_t*,const pthread_attr_t*,void*(*func)(void*),void*); int pthread_join(pthread_t,void**); int __cdecl _mkdir(const char* dirname);
    INLINE __attribute__((noreturn)) void OS_Exit(i64 exitCode) { ExitProcess((u32)exitCode); __builtin_unreachable(); }
    INLINE void OS_Close(FHandle fd) { CloseHandle(fd); }
    INLINE void* OS_AllocateRAM(size_t l,i32 p,i32 f,FHandle fd) { (void)f; if (fd==(void*)-1) return VirtualAlloc(NULL,l,0x3000,(p&2)?4:2); void* m = CreateFileMappingW(fd,NULL,(p&2) ? 4 : 2,(u32)(l>>32),(u32)l,NULL); void* r=MapViewOfFileEx(m,(p&2)?2:4,0,0,l,NULL); return CloseHandle(m),r;}    
    #define OS_MakeFolder(path) _mkdir(path)
    INLINE long OS_Read(FHandle fd, void* buf, size_t count) { u32 bytesRead = 0; return (ReadFile((void*)fd,buf,(u32)count,&bytesRead,NULL)) ? (long)bytesRead : (long)-1; }
    INLINE FHandle OS_OpenReadonly(const char* path) { void* f = CreateFileA(path,0x80000000L,1,NULL,3,0x08000080,NULL); return f == (void*)-1 ? DualLogError("Could not open file %s for reading\n",path), (void*)-1 : f; }
    INLINE FHandle OS_OpenWriteonly(const char* path) { FHandle h = CreateFileA(path,0x40000000L,0,NULL,2,128,NULL); return h == (void*)-1 ? DualLogError("Failed to open %s for writing\n",path),(void*)-1 : h; }
    INLINE int OS_FileSize(FHandle f) { LARGE_INTEGER s; return (f==(FHandle)-1 || !GetFileSizeEx(f,&s)) ? -1 : (int)s.QuadPart; }
    INLINE void* OS_AllocateFileBackedRAMReadonly(size_t s,FHandle fd, char* path) { void* m; void* r; return(fd==(void*)-1||!s||!(m=CreateFileMappingA(fd,NULL,2,0,0,NULL))) ? DualLogError("CreateFileMappingA failed for %s\n",path),NULL : (r=MapViewOfFile(m,4,0,0,s)) ? (CloseHandle(m),r) : (DualLogError("Failed to allocate %s\n",path),CloseHandle(m),NULL);}
    INLINE i64 OS_Seek(FHandle fd, i64 ofs, int whence /*forth and forsooth pray tell*/) { LARGE_INTEGER l={.QuadPart=ofs},n; return SetFilePointerEx((void*)fd,l,&n,whence) ? n.QuadPart : -1; }
    INLINE i64 OS_Tell(FHandle fd) { LARGE_INTEGER l={0},n; return SetFilePointerEx((void*)fd,l,&n,1) ? n.QuadPart : -1; }
    INLINE int OS_GetNumThreads() { SYSTEM_INFO si; GetSystemInfo(&si); return (int)si.dwNumberOfProcessors; }
    INLINE void OS_Free(void* p, size_t s) { (void)s; if(!p) { DualLogError("Attempting to double free!\n"); OS_Exit(1); } if(!UnmapViewOfFile(p) && !VirtualFree(p,0,0x00008000)) DualLogError("VirtualFree failed\n"); }
    INLINE i64 OS_RawWrite(FHandle fd, const void* buf, size_t count) { u32 w; return WriteFile((void*)fd,(void*)buf,(u32)count,&w,NULL) ? (i64)w : -1; }
    #define THRSTACKSZ (8 * 1024 * 1024)
    typedef struct { void* handle; } OS_Thread;
    void* __stdcall GetProcessHeap(); void* __stdcall HeapAlloc(void* hHeap, u32 dwFlags, size_t dwBytes); i32 __stdcall HeapFree(void* hHeap, u32 dwFlags, void* lpMem); void __stdcall Sleep(u32 dwMilliseconds); u32 __stdcall WaitForSingleObject(void* hHandle, u32 dwMilliseconds);
    typedef u32 (__stdcall *LPTHREAD_START_ROUTINE)(void* lpParameter);
    void* __stdcall CreateThread(void* lpThreadAttributes, size_t dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, void* lpParameter, u32 dwCreationFlags, u32* lpThreadId);
    static u32 WINAPI thrtramp(void* a) { void** b=(void**)a; void*(*fn)(void*)=(void*(*)(void*))b[0]; void* arg=b[1]; HeapFree(GetProcessHeap(),0,b); fn(arg); return 0; }
    INLINE int OS_ThreadCreate(OS_Thread* out, void*(*fn)(void*), void* arg) { void** b=(void**)HeapAlloc(GetProcessHeap(),0,2 * sizeof(void*)); b[0]=(void*)fn; b[1]=arg; out->handle=CreateThread(NULL,THRSTACKSZ,thrtramp,b,0,NULL); if(!out->handle){HeapFree(GetProcessHeap(),0,b); return -1;} return 0; }
    INLINE void OS_ThreadJoin(OS_Thread* t) { WaitForSingleObject(t->handle,0xFFFFFFFFUL); CloseHandle(t->handle); t->handle = NULL; }
    INLINE void OS_USleep(u32 usec) { Sleep((usec + 999) / 1000); }
    double get_time() { static LARGE_INTEGER frequency,counter; static i32 init=0; if (!init) { QueryPerformanceFrequency(&frequency); init=1; } QueryPerformanceCounter(&counter); return (double)counter.QuadPart / frequency.QuadPart; }
#else
    #define LINUX
    #define MOD_EXTENSION ".so" // e.g. Citadel.so
    #define OS_DlOpen(path)       dlopen((path),2)
    #define OS_DlSym(handle,name) dlsym((handle),(name))
    #define INVALID_FHANDLE -1
    struct input_id { u16 bustype,vendor,product,version;}; struct input_absinfo {i32 value,minimum,maximum,fuzz,flat,resolution;}; struct input_event { struct { long tv_sec,tv_usec; } time; u16 type,code; i32 value; };
    typedef int FHandle; struct timespec {i64 tv_sec,tv_nsec;}; typedef u64 pthread_t; typedef u32 pthread_mutexattr_t; typedef struct {u8 _[40];} pthread_mutex_t; typedef struct {u8 _[48];} pthread_cond_t; typedef int pthread_condattr_t; typedef struct {u32 flags; void* stack;} pthread_attr_t;
    int pthread_create(pthread_t* restrict,const pthread_attr_t* restrict,void*(*start_routine)(void*),void* restrict); int pthread_join(pthread_t,void**); void *dlopen(const char*,int); void *dlsym(void*,const char *);
    INLINE int OS_IOControl(int fd, unsigned long req, void* arg) { long r = 16; __asm__ __volatile__("syscall":"+a"(r):"D"(fd),"S"(req),"d"(arg):"rcx","r11","memory"); return (int)r; }
    INLINE int OS_MakeFolder(const char* path) { long r = 83; __asm__ __volatile__("syscall":"+a"(r):"D"(path),"S"(0755LL):"rcx","r11","memory"); return (int)r; }
    INLINE long OS_Read(long f,void*b,size_t c) { long r = 0; __asm__ __volatile__("syscall":"+a"(r):"D"(f),"S"(b),"d"(c):"rcx","r11","memory"); return r; }
    INLINE __attribute__((noreturn)) void OS_Exit(i64 exitCode) { long r = 231; __asm__ __volatile__("syscall":"+a"(r):"D"(exitCode):"rcx","r11","memory"); __builtin_unreachable(); }
    INLINE void OS_Close(FHandle fd) { long r = 3; __asm__ __volatile__("syscall":"+a"(r):"D"(fd):"rcx","r11","memory"); }
    INLINE long OS_Open(const char* path, i32 flags, i32 mode) { long r = 2; __asm__ __volatile__("syscall":"+a"(r):"D"(path),"S"((long)flags),"d"((long)mode):"rcx","r11","memory"); return r; }
    INLINE void* OS_AllocateRAM(size_t len, i32 prot, i32 flags, FHandle fd){ long r=9; register int r10 __asm__("r10")=flags; register int r8 __asm__("r8")=fd; register long r9 __asm__("r9")=0; __asm__ __volatile__("syscall":"+a"(r):"D"(NULL),"S"(len),"d"(prot),"r"(r10),"r"(r8),"r"(r9):"rcx","r11","memory"); return (void*)r; }
    INLINE FHandle OS_OpenReadonly(const char* path) { FHandle f=OS_Open(path,0,0); return f < 0 ? DualLogError("Could not open file %s for reading\n",path), -1 : f; }
    INLINE FHandle OS_OpenWriteonly(const char* path) { FHandle f=OS_Open(path,1|00000100|00001000,0644); return f < 0 ? DualLogError("Failed to open %s for writing\n",path),-1 : f; }
    INLINE int OS_FileSize(FHandle f) { long r=5,s[18]; __asm__ __volatile__("syscall":"+a"(r):"D"(f),"S"(s):"rcx","r11","memory"); return (int)s[6]; }
    INLINE void* OS_AllocateFileBackedRAMReadonly(size_t s, FHandle fd, char* path) { void* r=OS_AllocateRAM(s,1,2,fd); return r==(void*)-1 ? DualLogError("Failed to allocate %s\n",path),NULL : r; }
    INLINE i64 OS_Seek(FHandle fd, i64 ofs, int whence /* forth and forsooth pray tell*/) { i64 r = 8; __asm__ __volatile__("syscall":"+a"(r):"D"(fd),"S"(ofs),"d"(whence):"rcx","r11","memory"); return r; }
    INLINE i64 OS_Tell(FHandle fd) { i64 r=8; __asm__ __volatile__("syscall":"+a"(r):"D"(fd),"S"(0LL),"d"(1):"rcx","r11","memory"); return r; }
    INLINE int OS_GetNumThreads() { unsigned long m[16]; long r=204; __asm__ __volatile__("syscall":"+a"(r):"D"(0LL),"S"(128LL),"d"(m):"rcx","r11","memory"); int c = 0; for(int i=0;i<(r/8);i++) {c+=__builtin_popcountll(m[i]);} return r < 0 ? 1 : c; }
    INLINE void OS_Free(void* p,size_t s){ long r=11; if(!p || p == (void*)-1) { DualLogError("Attempting to double free!\n"); OS_Exit(1); } __asm__ __volatile__("syscall":"+a"(r):"D"(p),"S"(s):"rcx","r11","memory"); if(r<0) DualLogError("munmap failed\n"); }
    INLINE i64 OS_RawWrite(FHandle fd, const void* buf, size_t cnt) { i64 r=1; __asm__ __volatile__("syscall":"+a"(r):"D"(fd),"S"(buf),"d"(cnt):"rcx","r11","memory"); return r; }
    #define SYSCALL1(n, a) syscall6(n,(long)(a),0,0,0,0,0)
    #define SYSCALL2(n, a, b) syscall6(n,(long)(a),(long)(b),0,0,0,0)
    #define SYSCALL3(n, a, b, c) syscall6(n,(long)(a),(long)(b),(long)(c),0,0,0)
    #define SYSCALL4(n, a, b, c, d) syscall6(n,(long)(a),(long)(b),(long)(c),(long)(d),0,0)
    #define THRSTACKSZ (8 * 1024 * 1024)
    INLINE long syscall6(long n, long a, long b, long c, long d, long e, long f) { register long r=n; register long r10 __asm__("r10") = d; register long r8  __asm__("r8")  = e; register long r9  __asm__("r9")  = f; __asm__ __volatile__("syscall":"+a"(r):"D"(a),"S"(b),"d"(c),"r"(r10),"r"(r8),"r"(r9):"rcx","r11","memory"); return r; }
    typedef struct __attribute__((aligned(16))) OS_ThreadHead { void(*trampoline)(struct OS_ThreadHead*),*(*fn)(void*),*arg; int join_futex,_pad; } OS_ThreadHead;
    typedef struct { struct OS_ThreadHead* head; void* stack_base; } OS_Thread;
    __attribute__((naked)) static long OS_CloneSyscall(struct OS_ThreadHead* stack) { __asm__ volatile ("mov %%rdi, %%rsi\nmov $0x50f00, %%edi\nmov $56, %%eax\nsyscall\nmov  %%rsp, %%rdi\nret\n":::"rax","rcx","rsi","rdi","r11","memory"); }
    __attribute__((noreturn)) static void thrtramp(struct OS_ThreadHead* head) { head->fn(head->arg); __atomic_store_n(&head->join_futex,1,__ATOMIC_SEQ_CST); SYSCALL3(202,&head->join_futex,1,0x7fffffff);/*futex wake*/  SYSCALL1(60,0); __builtin_unreachable(); }
    INLINE int OS_ThreadCreate(OS_Thread* out, void* (*fn)(void*), void* arg) { void* base = OS_AllocateRAM(THRSTACKSZ,0x1|0x2,0x02|0x20,INVALID_FHANDLE); if (!base || base == (void*)-1) return -1; struct OS_ThreadHead* head = (struct OS_ThreadHead*)((char*)base + THRSTACKSZ) - 1; head->trampoline = thrtramp; head->fn=fn; head->arg=arg; head->join_futex=0; head->_pad=0; long tid = OS_CloneSyscall(head); if(tid < 0){OS_Free(base,THRSTACKSZ); return (int)tid;} out->head=head; out->stack_base=base; return 0; } // Multithreading taken from https://github.com/skeeto/scratch/blob/master/misc/stack_head.c Ref: https://nullprogram.com/blog/2023/03/23/ This is free and unencumbered software released into the public domain.
    INLINE void OS_ThreadJoin(OS_Thread* t) { int v; while ((v = __atomic_load_n(&t->head->join_futex, __ATOMIC_SEQ_CST)) == 0) SYSCALL4(202, &t->head->join_futex, 0 /*FUTEX_WAIT*/, v, 0); OS_Free(t->stack_base,THRSTACKSZ); t->head = NULL; t->stack_base = NULL; }
    INLINE  void OS_USleep(u32 usec) { long ts[2] = {usec / 1000000,(usec % 1000000) * 1000L}; SYSCALL2(35,ts,ts); }
    double get_time() { struct {i64 s,ns;} ts; i64 ret; __asm__ __volatile__("syscall":"=a"(ret):"a"(228),"D"(1),"S"(&ts):"rcx","r11","memory"); if (ret != 0) {return 0.0;} return (double)ts.s + (double)ts.ns * 1e-9; } // Full time in seconds, 1 for MONOTONIC, Note that using clock_gettime wasn't any better for performance.
#endif
INLINE void* OS_Alloc(size_t amount) { return OS_AllocateRAM(amount,0x1|0x2,0x02|0x20,INVALID_FHANDLE); } 
INLINE void* OS_Calloc(size_t amount, size_t count) { return OS_Alloc(amount * count); }
INLINE void OS_Write(FHandle f,const void* buf, size_t s, const char* p) { size_t total=0; while(total < s) { i64 w=OS_RawWrite(f,(const char*)buf + total,s - total); if(w < 0) { DualLogError("Write error to %s: %s[%d]\n",p,w,(i32)-w); OS_Exit(1); } total += (size_t)w; } }
INLINE void* OS_OpenAndAllocateFileBufferReadonly(const char* p,FHandle* f,int* s){void* r;return((*f=OS_OpenReadonly(p))==(FHandle)-1)?*s=0,(void*)0:((*s=OS_FileSize(*f))<=0)?DualLogError("Skipping empty:%s\n",p),OS_Close(*f),OS_Exit(1),NULL:(r=OS_AllocateFileBackedRAMReadonly(*s,*f,(char*)p))?(OS_Close(*f),r):NULL;}
INLINE void* OS_Realloc(void* old, size_t olds, size_t news) { void* n; return !old ? OS_Alloc(news) : news <= olds ? old : (n=OS_Alloc(news)) ? (mcpy(n,old,olds),OS_Free(old,olds),n) : 0; }
enum {GL_ARRAY_BUFFER=0x8892,GL_DEPTH_BUFFER_BIT=0x00000100,GL_READ_WRITE=0x88BA,GL_SSBO=0x90D2,GL_CULL_FACE=0x0B44,GL_BLEND=0x0BE2,GL_DEPTH_TEST=0x0B71,GL_RGB=0x1907,GL_TEXTURE0=0x84C0,GL_TEXTURE5=0x84C5,GL_COLOR_ATTACHMENT0=0x8CE0,GL_RG16F=0x822F,GL_TEXTURE1=0x84C1,GL_TEXTURE6=0x84C6,GL_COLOR_ATTACHMENT1=0x8CE1,GL_ELEMENT_ARRAY_BUFFER=0x8893,GL_RGB16F=0x881B,GL_TEXTURE2=0x84C2,
      GL_TEXTURE_2D=0x0DE1,GL_COLOR_ATTACHMENT2=0x8CE2,GL_FALSE=0,GL_RGBA=0x1908,GL_TEXTURE3=0x84C3,GL_UNSIGNED_BYTE=0x1401,GL_COLOR_ATTACHMENT3=0x8CE3,GL_FLOAT=0x1406,GL_RGBA32F=0x8814,GL_TEXTURE4=0x84C4,GL_FRAMEBUFFER=0x8D40,GL_COLOR_ATTACHMENT4=0x8CE4,GL_UNSIGNED_SHORT=0x1403,GL_RGBA8=0x8058,GL_COLOR_BUFFER_BIT=0x00004000,GL_STATIC_DRAW=0x88E4,GL_DYNAMIC_DRAW=0x88E8};
typedef void(*FGL_AT)(u32),(*FGL_F)(),    (*FGL_FF)(u32),  (*FGL_AS)(u32,u32),  (*FGL_VAB)(u32,u32), (*FGL_GT)(i32,u32*),   (*FGL_DA)(u32,i32,i32),     (*FGL_CC)(float,float,float,float),(*FGL_BD)(u32,size_t,const void*,u32),   (*FGL_U4F)(i32,float,float,float,float),        (*FGL_BBB)(u32,u32,u32),  *(*FGL_MBR)(u32,intptr_t,size_t,u32);
typedef void(*FGL_C)(u32), (*FGL_FL)(),   (*FGL_EVAA)(u32),(*FGL_BB)(u32,u32),  (*FGL_BT)(u32,u32),  (*FGL_U1F)(i32,float), (*FGL_BFS)(u32,u32,u32,u32),(*FGL_DE)(u32,i32,u32,const void*),(*FGL_UM4FV)(i32,i32,bool,const float*), (*FGL_BSD)(u32,intptr_t,intptr_t,const void*),  (*FGL_DB)(i32,const u32*), (*FGL_CM)(bool,bool,bool,bool);
typedef void(*FGL_CS)(u32),(*FGL_RB)(u32),(*FGL_BVA)(u32), (*FGL_GVA)(i32,u32*),(*FGL_U1I)(i32,i32), (*FGL_DC)(u32,u32,u32),(*FGL_CPIV)(u32,u32,i32*),  (*FGL_BVB)(u32,u32,intptr_t,i32),  (*FGL_RP)(i32,i32,i32,i32,u32,u32,void*),(*FGL_SS)(u32,i32,const char*const*,const i32*),(*FGL_U2UI)(i32,u32,u32),  (*FGL_CTSI2D)(u32,i32,i32,i32,i32,i32,i32,i32);
typedef void(*FGL_E)(u32), (*FGL_DF)(u32),(*FGL_LP)(u32),  (*FGL_GB)(i32,u32*), (*FGL_BFB)(u32,u32), (*FGL_GFS)(i32,u32*),  (*FGL_TPI)(u32,u32,i32),    (*FGL_FBT2D)(u32,u32,u32,u32,i32), (*FGL_GSIL)(u32,i32,i32*,char*),         (*FGL_BIT)(u32,u32,i32,bool,i32,u32,u32),       (*FGL_VP)(i32,i32,i32,i32),(*FGL_T2D)(u32,i32,i32,i32,i32,i32,u32,u32,const void*);
typedef void(*FGL_UP)(u32),(*FGL_D)(u32), (*FGL_DM)(bool), (*FGL_LW)(float),    (*FGL_GIV)(u32,i32*),(*FGL_U1UI)(i32,u32),  (*FGL_GSIV)(u32,u32,i32*),  (*FGL_VAF)(u32,i32,u32,bool,u32),  (*FGL_UM3FV)(i32,i32,bool,const float*), (*FGL_U3F)(i32,float,float,float),              (*FGL_U2F)(i32,float,float);
typedef u32(*FGL_CFBS)(u32), (*FGL_CP)(), (*FGL_GERR)(), (*FGL_CBFV)(u32,i32,const float*), (*FGL_CRS)(u32); typedef bool(*FGL_UB)(u32);
FGL_FL glFlush; FGL_AT glActiveTexture; FGL_AS glAttachShader; FGL_CTSI2D glCopyTexSubImage2D;  FGL_BB glBindBuffer;  FGL_BBB glBindBufferBase;    FGL_CPIV glGetProgramiv;FGL_CC glClearColor;    FGL_U4F glUniform4f;        FGL_BFB glBindFramebuffer;FGL_VP glViewport;    FGL_BVA glBindVertexArray; FGL_EVAA glEnableVertexAttribArray; FGL_CFBS glCheckFramebufferStatus;            
FGL_F glFinish; FGL_UP glUseProgram;    FGL_DM glDepthMask;    FGL_VAB glVertexAttribBinding;   FGL_DF glDepthFunc;   FGL_DC glDispatchCompute;    FGL_DB glDrawBuffers;   FGL_GSIV glGetShaderiv; FGL_BVB glBindVertexBuffer; FGL_LW glLineWidth;       FGL_LP glLinkProgram; FGL_RB glReadBuffer;       FGL_U3F glUniform3f;
FGL_D glDisable;FGL_CM glColorMask;     FGL_CS glCompileShader;FGL_UM3FV glUniformMatrix3fv;    FGL_DA glDrawArrays;  FGL_VAF glVertexAttribFormat;FGL_CP glCreateProgram; FGL_CRS glCreateShader; FGL_BFS glBlendFuncSeparate;FGL_CBFV glClearBufferFv; FGL_UB glUnmapBuffer; FGL_UP glUseProgram;       FGL_BD glBufferData;    
FGL_C glClear;  FGL_DE glDrawElements;  FGL_U2UI glUniform2ui; FGL_UM4FV glUniformMatrix4fv;    FGL_GIV glGetIntegerv;FGL_GSIL glGetShaderInfoLog; FGL_U2F glUniform2f;    FGL_U1UI glUniform1ui;  FGL_GVA glGenVertexArrays;  FGL_RP glReadPixels;      FGL_SS glShaderSource;FGL_TPI glTexParameteri;   FGL_U1F glUniform1f;
FGL_E glEnable; FGL_FF glFrontFace;     FGL_GB glGenBuffers;   FGL_FBT2D glFramebufferTexture2D;FGL_GERR glGetError;  FGL_GFS glGenFramebuffers;   FGL_GT glGenTextures;   FGL_BSD glBufferSubData;FGL_MBR glMapBufferRange;   FGL_U1I glUniform1i;      FGL_T2D glTexImage2D; FGL_BIT glBindImageTexture;FGL_BT glBindTexture;   
typedef enum {JOYHAT_CENTERED=0,JOYHAT_UP=1,JOYHAT_RIGHT=2,JOYHAT_DOWN=4,JOYHAT_LEFT=8,JOYHAT_RIGHT_UP=(2|1),JOYHAT_RIGHT_DOWN=(2|4),JOYHAT_LEFT_UP=(8|1),JOYHAT_LEFT_DOWN=(8|4)} JoyHatId;
typedef enum {KEY_UNKNOWN=-1,KEY_SPACE=32,KEY_APOSTROPHE=39/* ' */,KEY_COMMA=44/* , */,KEY_MINUS=45/* - */,KEY_PERIOD=46/* . */,KEY_SLASH=47/* / */,KEY_0=48,KEY_1=49,KEY_2=50,KEY_3=51,KEY_4=52,KEY_5=53,KEY_6=54,KEY_7=55,KEY_8=56,KEY_9=57,
             KEY_SEMICOLON=59/* ; */,KEY_EQUAL=61/* = */,KEY_A=65,KEY_B=66,KEY_C=67,KEY_D=68,KEY_E=69,KEY_F=70,KEY_G=71,KEY_H=72,KEY_I=73,KEY_J=74,KEY_K=75,KEY_L=76,KEY_M=77,KEY_N=78,KEY_O=79,KEY_P=80,KEY_Q=81,KEY_R=82,KEY_S=83,KEY_T=84,KEY_U=85,KEY_V=86,KEY_W=87,KEY_X=88,KEY_Y=89,KEY_Z=90,
             KEY_LEFT_BRACKET=91/* [ */,KEY_BACKSLASH=92/* \ */,KEY_RIGHT_BRACKET=93/* ] */,KEY_GRAVE_ACCENT=96/* ` */,KEY_ESCAPE=256,KEY_ENTER=257,KEY_TAB=258,KEY_BACKSPACE=259,KEY_INSERT=260,KEY_DELETE=261,KEY_RIGHT=262,KEY_LEFT=263,KEY_DOWN=264,KEY_UP=265,KEY_PAGE_UP=266,KEY_PAGE_DOWN=267,
             KEY_HOME=268,KEY_END=269,KEY_CAPS_LOCK=280,KEY_SCROLL_LOCK=281,KEY_NUM_LOCK=282,KEY_PRINT_SCREEN=283,KEY_PAUSE=284,KEY_F1=290,KEY_F2=291,KEY_F3=292,KEY_F4=293,KEY_F5=294,KEY_F6=295,KEY_F7=296,KEY_F8=297,KEY_F9=298,KEY_F10=299,KEY_F11=300,KEY_F12=301,KEY_KP_0=320,
             KEY_KP_1=321,KEY_KP_2=322,KEY_KP_3=323,KEY_KP_4=324,KEY_KP_5=325,KEY_KP_6=326,KEY_KP_7=327,KEY_KP_8=328,KEY_KP_9=329,KEY_KP_DECIMAL=330,KEY_KP_DIVIDE=331,KEY_KP_MULTIPLY=332,KEY_KP_SUBTRACT=333,KEY_KP_ADD=334,KEY_KP_ENTER=335,KEY_KP_EQUAL=336,KEY_LEFT_SHIFT=340,
             KEY_LEFT_CONTROL=341,KEY_LEFT_ALT=342,KEY_LEFT_SUPER=343,KEY_RIGHT_SHIFT=344,KEY_RIGHT_CONTROL=345,KEY_RIGHT_ALT=346,KEY_RIGHT_SUPER=347,KEY_MENU=348} KeyId;
typedef enum {MOUSE_BUTTON_1=0,MOUSE_BUTTON_2=1,MOUSE_BUTTON_3=2,MOUSE_BUTTON_4=3,MOUSE_BUTTON_5=4,MOUSE_BUTTON_6=5,MOUSE_BUTTON_7=6,MOUSE_BUTTON_8=7,MOUSE_BUTTON_LEFT=0,MOUSE_BUTTON_RIGHT=1,MOUSE_BUTTON_MIDDLE=2} MouseButtonId;
typedef enum {JOYSTICK_1=0,JOYSTICK_2=1,JOYSTICK_3=2,JOYSTICK_4=3,JOYSTICK_5=4,JOYSTICK_6=5,JOYSTICK_7=6,JOYSTICK_8=7,JOYSTICK_9=8,JOYSTICK_10=9,JOYSTICK_11=10,JOYSTICK_12=11,JOYSTICK_13=12,JOYSTICK_14=13,JOYSTICK_15=14,JOYSTICK_16=15,JOYSTICK_LAST=15} JoystickId;
typedef struct { bool down,pressed,released; } KeyState; typedef struct { const char* name; int value; } InputElement; typedef struct { V3 normal; float d; } FrustumPlane; typedef struct PngArena { u8*base,*cursor,*end; } PngArena;
typedef struct { double scrollDelta; KeyState keyStates[MAX_KEYS],mouseButtons[MAX_MOUSE_BUTTONS],joystickButtons[16][16],joystickHats[5]; /* What can I say, I'm a man of many hats. ^^D*/ bool lastUse,isCapsLockOn,joystickPresent[16]; } InputSystem; double last_mouse_x,last_mouse_y;
u32 inputImageID,inputUIID,inputDepthID,inputWorldPosID,inputSpecID,inputNormalID,gBufferFBO,uiFBO,outputImageID,depthPrepassSP,chunkSP,chunkVAO,chunkVBO,uiSP,debugUnlitSP,shadowmapsSP,shadowmapsClearSP,shadowMapSSBO,shadowMapsIndirectionID,ssrSP,imageBlitSP,quadVAO,quadVBO,
    textSP,textVAO,textVBO,debugLinesVAO,debugLinesVBO,matricesBufferID,cellVisibleDataID,debugLineColors,colorBufferID,texPalID,texPalOfsID,textureOffsetsID,textureSizesID,lightsID,voxListCntsID,voxelLightListsID,voxelUpdateSP,vbos[MAX_MDLS],tbos[MAX_MDLS];
static float berserkSeedTime,rasterPerspectiveProjection[16],shadowmapsPerspectiveProjection[16],lightView[LIGHT_COUNT][6][4][4],lightViewProj[LIGHT_COUNT][6][16];
float modelMatrices[INSTANCE_COUNT*16]; u16** modelTriangles; u32 modelVertexCounts[MAX_MDLS] = {0}; u16 modelTriangleCounts[MAX_MDLS] = {0}; float modelBounds[MAX_MDLS]; u16 mdlsCnt;
float **vPos,**vNor,**vUV,**physPos; u16** physTris; u32* physVertCounts;
bool mouseMovementThisFrame,window_has_focus,ignore_next_mouse_delta,returnToPause=false,fovSliderActive=false,gammaSliderActive=false,masterVolumeSliderActive=false,musicVolumeSliderActive=false,messageVolumeSliderActive=false,sfxVolumeSliderActive=false,enteringPlayerName=false;
u8 currentPlayerNameLength=0; i8 currentMenuItem=0, currentMenuTab=0, menuItemCount=4, menuTabCount=1; static int threadCnt=0; static u32 globalframe=0,globalframesPerLastSecond;
#define CHECK_GL_ERROR() do { u32 err = glGetError(); if (err != 0) DualLogError("GL Error at %s:%d: %d\n", __FILE__, __LINE__, err); } while(0)
#define SHADOW_MAP_SIZE 128u
#define MAX_SHADOWMAPS 128u
#define NEAR_PLANE (0.02f)
#define ONE_OVER_SQRT2 0.70710678118f
InputSystem Sys_Input; CheatsSystem Cheats = {.god=false,.noclip=false,.showLocation=true,.showFPS=true,.editMode=false,.showPhys=false};
static bool shadowBuffersCreated = false;
typedef struct { V3 position; Quaternion rotation; u8 fov; u16 width,height; float near,far,finished; bool visible; } CamView; // Max is 8 cam views on level 8 + 3 sensaround views = 11.
CamView camViews[64],levelCamViews[14][64]; u8 camViewCount,levelCamViewCount[14]; u32 camViewTextures[64],levelCamViewTextures[14][64];
FrustumPlane lightFrustumPlanes[LIGHT_COUNT][6][6],playerFrustumPlanes[6];
u16 editModeSelection,editModeTestEntityDefinition=0; double shadowTime; double physTime; u32 shadowmapIndirectionList[LIGHT_COUNT]; u16 texCnt; bool doubleSidedTexture[MAX_TXRS],transparentTexture[MAX_TXRS]; u32 drawCalls,uiDrawCalls,shadDrawCalls,vertsRendered,drawCallsNormal;
static const u8 Mpg_FrontPage=0,Mpg_Singleplayer=1,Mpg_Multiplayer=2,Mpg_NewGame=3,Mpg_Load=4,Mpg_Options=5,Mpg_Save=6,Mpg_IntroVideo=7,Mpg_CreditsVideo=8; u8 currentMenuPage = Mpg_FrontPage; bool resDropdownOpen = false; int resDropdownCount=0,resSelectedIdx=0;
typedef struct {int w,h;} ResMode; ResMode resModes[8];
extern Entity EDefs[MAX_ENTITIES];
#include "lib.c" // LibC Replacements and Helpers
void TurnLightOff(u16 litIdx) { if (litIdx < World.loadedLights) {flag_set(&World.lights[litIdx].lflags,LIGHTON,false);} }
Color textColors[] = {{1.0f,1.0f,1.0f,1.0f},/* 0 White T_WHITE*/ {0.890196078f,0.874509804f,0.0f,1.0f},/* 1 Yellow T_YELLOW*/  {0.623529412f,0.611764706f,0.0f,1.0f},/* 2 Dark Yellow (Yellow * 0.7f) T_DARK_YELLOW*/ {0.372549020f,0.654901961f,0.168627451f,1.0f},/* 3 Green T_GREEN*/ {0.917647059f,0.137254902f,0.168627451f,1.0f},/* 4 Red T_RED*/
                      {1.0f,0.498039216f,0.0f,1.0f}, /* 5 Orange T_ORANGE*/ {0.674509804f,0.058823529f,0.070588235f,1.0f},/* 6 StopD Red T_STOPD_RED*/ {0.941176471f,0.282352941f,0.298039216f,1.0f},/* 7 StopD Red Highlight T_STOPD_RED_HIGHLIGHT*/ {0.909803922f,0.203921569f,0.219607843f,1.0f}, /* 8 StopD Red Pause Title T_STOPD_RED_PAUSETITLE*/
                      {0.470588235f,0.721568627f,0.172549020f,1.0f},/* 9 Green Menu Title T_GREEN_MENU*/ {0.137254902f,0.356862745f,0.109803922f,1.0f},/* 10 Green Menu Title Shadow T_GREEN_MENU_SHADOW*/ {0.239215686f,0.466666667f,0.129411765f,1.0f}, /* 11 Green Menu Title Glow T_GREEN_MENU_GLOW*/ {0.392156863f,0.031372549f,0.039215686f,1.0f} /* 12 Red Menu Text Dark T_RED_MENU*/ };
// Wireline Rendering
typedef struct { float x,y,z,r,g,b,a; } DebugLineVertex;
DebugLineVertex debugLineVerts[MAX_WIRELINE_VRTS * 2];
INLINE void DrawDebugLines(float* viewProj) {
    if (World.debugLineVertCount == 0) {return;}
    
    glBindBuffer(GL_ARRAY_BUFFER,debugLinesVBO); glBufferSubData(GL_ARRAY_BUFFER,0,World.debugLineVertCount * sizeof(DebugLineVertex),debugLineVerts); glUseProgram(debugUnlitSP); glUniformMatrix4fv(0,1,GL_FALSE,viewProj); glLineWidth(1.0f); glDisable(GL_DEPTH_TEST); glBindVertexArray(debugLinesVAO); 
    glDrawArrays(0x0001/*GL_LINES*/,0,World.debugLineVertCount); drawCalls++; vertsRendered += World.debugLineVertCount; glEnable(GL_DEPTH_TEST); World.debugLineVertCount = 0;
}

void AddWireLine(V3 start, V3 end, Color col) {
    if (World.debugLineVertCount >= MAX_WIRELINE_VRTS - 2) return;
    
    int i = World.debugLineVertCount;
    debugLineVerts[i].x = start.x; debugLineVerts[i].y = start.y; debugLineVerts[i].z = start.z;
    debugLineVerts[i].r = col.r; debugLineVerts[i].g = col.g; debugLineVerts[i].b = col.b; debugLineVerts[i].a = col.a; i++;
    debugLineVerts[i].x = end.x; debugLineVerts[i].y = end.y; debugLineVerts[i].z = end.z;
    debugLineVerts[i].r = col.r; debugLineVerts[i].g = col.g; debugLineVerts[i].b = col.b; debugLineVerts[i].a = col.a; i++;
    World.debugLineVertCount = i;
}

ShapeBox Entity_GetBox(u16 i); ShapeCapsule Entity_GetCap(u16 i); ShapeSphere Entity_GetSph(u16 i); void obb_axes(Quaternion q, V3 *ax, V3 *ay, V3 *az);
inline Color ColliderColor(u16 i) { return (!(World.instances[i].entflags & EF_RIGIDBODY)) ? textColors[T_GREEN_MENU_SHADOW] : ((World.colliding[i]) ? textColors[T_RED] : textColors[T_GREEN]); }
void DrawVelocityVector(u16 i) {
    if (!(World.instances[i].entflags & EF_RIGIDBODY)) {return;}
    V3 tip = V3_AplusB(World.position[i],V3_ScaleByF(World.velocity[i],0.25f)); AddWireLine(World.position[i],tip,textColors[T_ORANGE]); V3 perp = V3_Normalize(V3_Cross(World.velocity[i],(vabs(World.velocity[i].y/V3_Mag(World.velocity[i])) < 0.9f) ? (V3){0,1,0} : (V3){1,0,0}));
    AddWireLine(V3_AplusB(tip,V3_ScaleByF(perp,0.05f)),V3_AsubB(tip,V3_ScaleByF(perp,0.05f)),textColors[T_ORANGE]); // Small cross at tip so zero-length vecs are still visible when barely moving
}

void DrawBoxColliderColored(u16 i, Color col) {
    ShapeBox b = Entity_GetBox(i); V3 ax,ay,az,c[8],px,py,pz; obb_axes(b.rot,&ax,&ay,&az); px=V3_ScaleByF(ax,b.hExt.x); py=V3_ScaleByF(ay,b.hExt.y); pz=V3_ScaleByF(az,b.hExt.z);
    for (int s=0;s<8;s++) { float sx=(s&1)?1.f:-1.f,sy=(s&2)?1.f:-1.f,sz=(s&4)?1.f:-1.f; c[s]=V3_AplusB(b.ctr,V3_AplusB(V3_AplusB(V3_ScaleByF(px,sx),V3_ScaleByF(py,sy)),V3_ScaleByF(pz,sz))); }
    AddWireLine(c[0],c[1],col); AddWireLine(c[2],c[3],col); AddWireLine(c[4],c[5],col); AddWireLine(c[6],c[7],col); AddWireLine(c[0],c[2],col); AddWireLine(c[1],c[3],col); AddWireLine(c[4],c[6],col); AddWireLine(c[5],c[7],col); AddWireLine(c[0],c[4],col); AddWireLine(c[1],c[5],col); AddWireLine(c[2],c[6],col); AddWireLine(c[3],c[7],col);
    DrawVelocityVector(i);
}
void DrawBoxCollider(u16 i) { DrawBoxColliderColored(i,ColliderColor(i)); }

void DrawSphereWireframe(Color col, ShapeSphere s) { float step=6.28318530f/12; for (int seg=0;seg<12;seg++) { float a0=seg*step,a1=a0+step,c0=vcosf(a0),s0=vsinf(a0),c1=vcosf(a1),s1=vsinf(a1); AddWireLine(V3_AplusB(s.ctr,(V3){c0*s.rad,0,s0*s.rad}),V3_AplusB(s.ctr,(V3){c1*s.rad,0,s1*s.rad}),col); AddWireLine(V3_AplusB(s.ctr,(V3){c0*s.rad,s0*s.rad,0}),V3_AplusB(s.ctr,(V3){c1*s.rad,s1*s.rad,0}),col); AddWireLine(V3_AplusB(s.ctr,(V3){0,c0*s.rad,s0*s.rad}),V3_AplusB(s.ctr,(V3){0,c1*s.rad,s1*s.rad}),col); } }
void DrawSphereCollider(u16 i) { Color col = ColliderColor(i); ShapeSphere s = Entity_GetSph(i); DrawSphereWireframe(col,s); DrawVelocityVector(i); }
void DrawSphereContact(V3 pos, float rad) { if (Cheats.showPhys) {Color col = (Color){0.0f,0.0f,1.0f,1.0f}; ShapeSphere s = (ShapeSphere){pos,rad}; DrawSphereWireframe(col,s);} }
void DrawMeshCollider(u16 i) {
    Color col = ColliderColor(i); u16 mi = (World.collider[i] == COLTYPE_CVX) ? World.instances[i].colMeshIndex : World.instances[i].modelIndex; if (mi >= MAX_MDLS || mi >= mdlsCnt) return;
    u32 triCount = modelTriangleCounts[mi]; if (!triCount) return;
    float M[16]; mcpy(M, &modelMatrices[i*16], 64); float m00=M[0],m10=M[1],m20=M[2],m01=M[4],m11=M[5],m21=M[6],m02=M[8],m12=M[9],m22=M[10],tx=M[12],ty=M[13],tz=M[14];
    const float* pos = vPos[mi]; const u16* tris = modelTriangles[mi];
    for (u32 j=0; j<triCount; j++) {
        V3 w[3]; for (int k=0;k<3;++k) { u32 vi=tris[j*3 + k]; float x=pos[vi*3 + 0]; float y=pos[vi*3 + 1]; float z=pos[vi*3 + 2]; w[k]=(V3){m00*x + m01*y + m02*z + tx, m10*x + m11*y + m12*z + ty, m20*x + m21*y + m22*z + tz}; }
        AddWireLine(w[0],w[1],col); AddWireLine(w[1],w[2],col); AddWireLine(w[2],w[0],col);
    }
    DrawVelocityVector(i);
}

void DrawCapsuleCollider(u16 i) {
    Color col = ColliderColor(i); ShapeCapsule cap = Entity_GetCap(i);
    V3 axis=V3_Normalize(V3_AsubB(cap.tip,cap.base)); V3 ref=(vabs(axis.y)<0.9f)?(V3){0,1,0}:(V3){1,0,0}; V3 perp0=V3_Normalize(V3_Cross(axis,ref)),perp1=V3_Cross(axis,perp0);
    float step=6.28318530f/12,r=cap.rad;
    for (int seg=0;seg<12;seg++) {
        float a0=seg*step,a1=a0+step,c0=vcosf(a0),s0=vsinf(a0),c1=vcosf(a1),s1=vsinf(a1);
        V3 r0 = V3_AplusB(V3_ScaleByF(perp0,c0*r),V3_ScaleByF(perp1,s0*r)), r1=V3_AplusB(V3_ScaleByF(perp0,c1*r),V3_ScaleByF(perp1,s1*r));
        AddWireLine(V3_AplusB(cap.base,r0),V3_AplusB(cap.base,r1),col); AddWireLine(V3_AplusB(cap.tip,r0),V3_AplusB(cap.tip,r1),col);
    }
    for (int seg=0;seg<6;seg++) {
        float a0=seg*step,a1=a0+step,c0=vcosf(a0),s0=vsinf(a0),c1=vcosf(a1),s1=vsinf(a1);
        AddWireLine(V3_AplusB(cap.base,V3_AplusB(V3_ScaleByF(perp0,c0*r),V3_ScaleByF(axis,-s0*r))),V3_AplusB(cap.base,V3_AplusB(V3_ScaleByF(perp0,c1*r),V3_ScaleByF(axis,-s1*r))),col);
        AddWireLine(V3_AplusB(cap.base,V3_AplusB(V3_ScaleByF(perp1,c0*r),V3_ScaleByF(axis,-s0*r))),V3_AplusB(cap.base,V3_AplusB(V3_ScaleByF(perp1,c1*r),V3_ScaleByF(axis,-s1*r))),col);
        AddWireLine(V3_AplusB(cap.tip, V3_AplusB(V3_ScaleByF(perp0,c0*r),V3_ScaleByF(axis, s0*r))),V3_AplusB(cap.tip, V3_AplusB(V3_ScaleByF(perp0,c1*r),V3_ScaleByF(axis, s1*r))),col);
        AddWireLine(V3_AplusB(cap.tip, V3_AplusB(V3_ScaleByF(perp1,c0*r),V3_ScaleByF(axis, s0*r))),V3_AplusB(cap.tip, V3_AplusB(V3_ScaleByF(perp1,c1*r),V3_ScaleByF(axis, s1*r))),col);
    }
    for (int seg=0;seg<4;seg++) { float a=seg*(6.28318530f/4.f); V3 off=V3_AplusB(V3_ScaleByF(perp0,vcosf(a)*r),V3_ScaleByF(perp1,vsinf(a)*r)); AddWireLine(V3_AplusB(cap.base,off),V3_AplusB(cap.tip,off),col); }
    DrawVelocityVector(i);
}

void DrawAngularVelocity(u16 i) {
    if (!(World.instances[i].entflags & EF_RIGIDBODY) || V3_Mag(World.angularVelocity[i]) < 0.0001f) return; // skip near-zero
    Color purple = (Color){0.5f,0.0f,1.0f,1.0f}; V3 dir=V3_Normalize(World.angularVelocity[i]); V3 tip=V3_AplusB(World.position[i],V3_ScaleByF(World.angularVelocity[i],0.35f)); AddWireLine(World.position[i],tip,purple); // Arrow (line vector)
    V3 ref=(vabs(dir.y) < 0.9f) ? (V3){0,1,0} : (V3){1,0,0}; V3 perp=V3_Normalize(V3_Cross(dir,ref)); V3 perp2 = V3_Cross(dir,perp);
    AddWireLine(V3_AplusB(tip,V3_ScaleByF(perp, 0.05f)),V3_AplusB(tip,V3_ScaleByF(perp, -0.05f)), purple); // Small cross at tip so zero-length vectors are still visible
    AddWireLine(V3_AplusB(tip,V3_ScaleByF(perp2,0.05f)),V3_AplusB(tip,V3_ScaleByF(perp2,-0.05f)), purple);
    float rad=0.6f; /*Quarter circle arc (visualizes rotation plane + sense)*/ float step = 1.57079632679f / 8.0f; /*quarter circle divided into 8 segments*/
    V3 axis=dir; V3 p1=V3_Normalize(V3_Cross(axis,ref)); V3 p2=V3_Cross(axis,p1); V3 prev = V3_AplusB(World.position[i], V3_ScaleByF(p1,rad)); // Find two vectors perpendicular to angular axis
    for (int j=1;j<=8;++j) { float a = j * step; float c = vcosf(a); float s = vsinf(a); V3 cur = V3_AplusB(World.position[i],V3_AplusB(V3_ScaleByF(p1,c * rad),V3_ScaleByF(p2,s * rad))); AddWireLine(prev,cur,purple); prev = cur; }
}

#include "textures.c" // 2D Texture Load System
#include "models.c" // 3D Model Load System
#include "culling.c" // Culling System
#include "text.c" // Fonts and Text System
#include "winput.c" // Window + Input System
#include "trigger.c" // Trigger Volumes System
#include "physics.c" // Physics System
#include "particles.c" // Particles System
// Player Movement
float GetBasePlayerSpeed(u16 p,bool running){
    bool sprint=Sprint(); if(Cheats.noclip)return PLAYER_MAX_CYBER_SPEED*(sprint?2.5f:1.5f); if(World.curLev==LEVEL_CYBERSPACE)return PLAYER_MAX_CYBER_SPEED;
    BodyState b=World.instances[p].bodyState; float v=WALK_SPEED;
    switch(b){ case BodyState_CrouchingDown: case BodyState_Crouch:v=CROUCH_SPEED; break; case BodyState_Prone: case BodyState_ProningDown: case BodyState_ProningUp:v=PLAYER_MAX_PRONE_SPEED; break; default:break; }
    if ((sprint||World.boosterActive) && running) { v = World.invP1.fatigue > 80.0f && World.boosterActive ? SPRINT_SPEED_FATIGUED : SPRINT_SPEED;
    if (b==BodyState_Standing||b==BodyState_Crouch||b==BodyState_CrouchingDown)  v -= (WALK_SPEED-CROUCH_SPEED)*1.5f;
    else if(b==BodyState_Prone||b==BodyState_ProningDown||b==BodyState_ProningUp)v -= (WALK_SPEED-PLAYER_MAX_PRONE_SPEED)*2.f;}
    return v + (World.boosterActive ? PLAYER_BOOSTER_SPEED_BOOST : 0.0f);
}

inline float smooth_damp(float cur, float targ, float* vel, float tm, float dt) { float o=2.0f / vmax(tm,0.0001f); float x=o * dt; float exp=1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x); float d=cur - targ; float t=(*vel + o * d) * dt; *vel=(*vel - o * t) * exp; return targ + (d + t) * exp; }
bool CantStand(u16 playerIdx, float targetHeight) { // I can't stand it.
    float oldHeight = World.colliderSize[playerIdx].y; V3 oldPos = World.position[playerIdx];
    World.colliderSize[playerIdx].y = targetHeight; // Temporarily morph player into the standing capsule
    World.position[playerIdx].y += (targetHeight - oldHeight); 
    bool blocked = false;
    i32 cx = PosGetCellCoordX(World.position[playerIdx].x);
    i32 cz = PosGetCellCoordZ(World.position[playerIdx].z);
    u32 mask = GetCollisionMask(World.layer[playerIdx]);
    for (i32 dx = -1; dx <= 1 && !blocked; ++dx) {
        for (i32 dz = -1; dz <= 1 && !blocked; ++dz) {
            u32 cell = PosGetCellCoordsP(cx + dx, cz + dz);
            for (u16 k = 0; k < cellCounts[cell]; ++k) {
                u16 b = cellLists[cell][k]; if (b == playerIdx || !(mask & World.layer[b]) || World.collider[b] == COLTYPE_NONE) continue;
                if (World.collider[b] == COLTYPE_MSH) { Overlap r = CapMsh(Entity_GetCap(playerIdx),World.instances[b].modelIndex,&modelMatrices[b*16]); if (r.hit && r.pen > 0.08f) { blocked = true; break; } }
            }
        }
    }
    World.colliderSize[playerIdx].y = oldHeight;
    World.position[playerIdx] = oldPos;
    return blocked;
}

bool DoubleTapLeanLeft(void)  { if(!GetKeyPressed(7)){return false;} if (World.pauseRelativeTime < World.invP1.leanLeftTapFinished) { World.invP1.leanLeftTapFinished = 0.0; return true; } World.invP1.leanLeftTapFinished = World.pauseRelativeTime + 0.5; return false; }
bool DoubleTapLeanRight(void) { if(!GetKeyPressed(8)){return false;} if (World.pauseRelativeTime < World.invP1.leanRightTapFinished) { World.invP1.leanRightTapFinished = 0.0; return true; } World.invP1.leanRightTapFinished = World.pauseRelativeTime + 0.5; return false; }
void ApplyPlayerMovements(float dt) {
    Entity *p = &World.instances[PLAYER1]; Quaternion r = World.rotation[PLAYER1]; float leanSpeed = 70.0f, leanMaxAngle = 35.0f; float leanInput = (float)LeanLeft() - (float)LeanRight(); bool doubleTapLean = DoubleTapLeanLeft() || DoubleTapLeanRight();
    bool movingForward = Forward() > 0.1f, leanRight = leanInput < 0.0f, leanLeft = leanInput > 0.0f;
    if (doubleTapLean) { World.invP1.leanResetting = true; World.invP1.leanVelocity = 0.0f; KeyState *kL = GetCodeMapping(7), *kR = GetCodeMapping(8); kL->pressed = kR->pressed = false; } // Double-tap lean: initiate smooth reset to upright over 0.2 seconds
    if (World.invP1.leanResetting) { 
        World.invP1.leanTarget = smooth_damp(World.invP1.leanTarget,0.0f,&World.invP1.leanVelocity,0.2f,dt); 
        if(vabs(World.invP1.leanTarget) < 0.5f){World.invP1.leanTarget=World.invP1.leanVelocity=0.0f; World.invP1.leanResetting=false;} 
    } else {
        if (leanLeft || leanRight) { if(leanLeft){World.invP1.leanRightTapFinished =0;} if(leanRight){World.invP1.leanLeftTapFinished=0;} World.invP1.leanTarget=vclamp(World.invP1.leanTarget + (leanInput * leanSpeed * dt),-leanMaxAngle,leanMaxAngle); }
        else if (movingForward) { if (vabs(World.invP1.leanTarget) < 0.5f) { World.invP1.leanTarget = 0.0f; } else { World.invP1.leanTarget -= (World.invP1.leanTarget > 0.0f ? 1.0f : -1.0f) * leanSpeed * dt; } }
    }
    World.cam_roll = World.invP1.leanTarget;
    float targetRatio=1.0f, transitionSec=0.2f; float currentRatio=World.invP1.currentCrouchRatio;
    if (Crouch()) { // Crouch key always targets crouch ratio from any state
        if (p->bodyState == BodyState_Crouch) { if (!CantStand(PLAYER1,PLAYER_HEIGHT)){p->bodyState = BodyState_StandingUp;}} // Already at crouch → toggle up to standing
        else if (currentRatio > PLAYER_CROUCH_RATIO) { p->bodyState = BodyState_CrouchingDown;} // Above crouch → go down to crouch (handles "if standing up will go back to crouched")
        else {p->bodyState=BodyState_ProningUp;} // Below crouch → go up to crouch (handles "if proning down will go back to crouched")
    } else if (Prone()) {
        if (p->bodyState == BodyState_Standing) { p->bodyState = BodyState_ProningDown; } // Standing → go to prone
        else if (currentRatio > PLAYER_CROUCH_RATIO) { if (!CantStand(PLAYER1,PLAYER_HEIGHT)){p->bodyState=BodyState_StandingUp;}else{p->bodyState = BodyState_ProningDown;} } // Between crouch and standing → up to standing
        else if (p->bodyState == BodyState_Crouch) { p->bodyState = BodyState_ProningDown; } // Crouch → go to prone
        else { p->bodyState = BodyState_ProningUp; } // Between prone and crouch, or prone → up to crouch
    }
    switch (p->bodyState) {
        case BodyState_CrouchingDown:targetRatio=-0.01f; break;                       case BodyState_StandingUp:targetRatio=1.01f;  break;      case BodyState_ProningDown:targetRatio=-0.01f; break; 
        case BodyState_ProningUp:    targetRatio=1.01f; transitionSec+=0.1f; break; case BodyState_Crouch:    targetRatio=PLAYER_CROUCH_RATIO; break; case BodyState_Prone:      targetRatio=PLAYER_PRONE_RATIO; break; default: targetRatio=1.0f; break;
    }
    float lastRatio = World.invP1.currentCrouchRatio;
    World.invP1.currentCrouchRatio = smooth_damp(lastRatio,targetRatio,&World.invP1.crouchingVelocity,transitionSec,dt);
    if (World.invP1.currentCrouchRatio >= 1.0f) { World.invP1.currentCrouchRatio = 1.0f; if(p->bodyState == BodyState_StandingUp){p->bodyState=BodyState_Standing;} }
    else if (p->bodyState == BodyState_CrouchingDown && World.invP1.currentCrouchRatio <= PLAYER_CROUCH_RATIO) { World.invP1.currentCrouchRatio = PLAYER_CROUCH_RATIO; p->bodyState = BodyState_Crouch; }
    else if (p->bodyState == BodyState_ProningUp && World.invP1.currentCrouchRatio >= PLAYER_CROUCH_RATIO) { World.invP1.currentCrouchRatio = PLAYER_CROUCH_RATIO; p->bodyState = BodyState_Crouch; }
    else if (p->bodyState == BodyState_ProningDown && World.invP1.currentCrouchRatio <= PLAYER_PRONE_RATIO) { World.invP1.currentCrouchRatio = PLAYER_PRONE_RATIO; p->bodyState = BodyState_Prone; }
    World.colliderSize[PLAYER1].y = PLAYER_HEIGHT * World.invP1.currentCrouchRatio;
    float h=(float)Forward() - (float)Backpedal(), s=(float)StrafeRight() - (float)StrafeLeft(), vertInput=(float)SwimUp() - (float)SwimDn();
    float y2=r.y*r.y, xz=r.x*r.z, wy=r.w*r.y;
    p->forward=V3_Normalize((V3){ 2.0f*(xz + wy),2.0f*(r.y*r.z - r.w*r.x),1.0f - 2.0f*(r.x*r.x + y2) }); p->right=V3_Normalize((V3){ 1.0f - 2.0f*(y2 + r.z*r.z),2.0f*(r.x*r.y + r.w*r.z),2.0f*(xz - wy) });
    V3 inputDir={ p->forward.x*h + p->right.x*s,vertInput,p->forward.z*h + p->right.z*s}; 
    float inputLenSq = V3_dot(inputDir,inputDir); V3 w = (inputLenSq > PHY_EPSILON) ? V3_ScaleByF(inputDir, 1.0f / vsqrtf(inputLenSq)) : (V3){0, 0, 0};
    bool isRunning = (inputLenSq > 0.01f); float speed = GetBasePlayerSpeed(PLAYER1,isRunning) * 1.75f, accel=World.boosterActive ? 1.0f : 3.0f; V3 targetVel = V3_ScaleByF(w,speed); 
    if (World.invP1.ladderState > 0) {
        float climbSpeed = (Sprint() && isRunning) ? 1.2f : 0.4f;
        targetVel = (V3){p->right.x * s * speed * 0.3f, h * climbSpeed * 25.0f, p->right.z * s * speed * 0.3f};
        accel = 5.0f;
    } else { if (vabs(vertInput) < 0.001f) { targetVel.y = World.velocity[PLAYER1].y; } }
    V3 dv = V3_AsubB(targetVel, World.velocity[PLAYER1]); 
    dv = (V3){ vclamp(dv.x, -10.0f, 10.0f), vclamp(dv.y, -10.0f, 10.0f), vclamp(dv.z, -10.0f, 10.0f) };
    World.velocity[PLAYER1] = V3_AplusB(World.velocity[PLAYER1], V3_ScaleByF(dv, accel * vclamp(dt, 0.0005f, 0.1f)));
}
#include "console.c" // Console Sys - CHEATS!
#include "audio.c" // Audio Sys
#include "ray.c" // Raycast Sys
#include "credits.h"
#include "entity.c"
#include "ai.c"
#include "citadel.c"
#include "weapons.c"
#include "biomonitor.c" // End game specific code includes
// Credits Sys
char creditStats[4096];
INLINE float GetScore(float stupid, bool isFinal) {
    float victories = (float)(World.kills + World.cyberkills); if (isFinal) {victories -= vmin(World.ressurections * 10.0f, victories * 0.666f);}
    float secs  = vfloor((float)World.pauseRelativeTime / 3600.0f), score = victories * 10000.0f;
    score -= vmin(score * 0.666f, secs * 100.0f); score *= (stupid + 1.0f) / 37.0f; if (stupid > 35.0f) {score += 2222222.0f;}
    return vfloor(score);
}

INLINE void DecomposeTime(double t, u32* h, u32* m, double* s) { double tb = vfloor(t / 3600.0); *h = (u32)tb; t -= tb * 3600.0; tb = vfloor(t / 60.0); *m = (u32)tb; *s = t - tb * 60.0; }
INLINE void CreditsStats() {
    size_t off = 0; u32 h,m; double s;
    off += sFormat(creditStats + off, sizeof(creditStats),"============================================================================\nCITADEL\n============================================================================\nCONGRATULATIONS %s\n",World.playerName);
    DecomposeTime(World.pauseRelativeTime,&h,&m,&s); off += sFormat(creditStats + off, sizeof(creditStats),"Straight Time: %uh %um %.3fs\n",h,m,s);
    DecomposeTime(World.absoluteTime,&h,&m,&s);      off += sFormat(creditStats + off,sizeof(creditStats),"Total Time (with reload from deaths): %uh %um %.3fs\n",h,m,s);
    float stupid = ((float)(World.diffCbt * World.diffCbt)) + ((float)(World.diffPuz * World.diffPuz)) + ((float)(World.diffMis * World.diffMis)) + ((float)(World.diffCyb * World.diffCyb)); u32 finalSubscore = GetScore(stupid,false), finalScore = (u32)GetScore(stupid,true);
    off += sFormat(creditStats + off,sizeof(creditStats),"Kills: %u\nKills in Cyberspace: %u\nScoreSubtotal: %u\nDeaths: %u\nRessurections: %u\n",World.kills,World.cyberkills,(u32)finalSubscore,World.deaths,World.ressurections);
    off += sFormat(creditStats + off,sizeof(creditStats),"Combat: %u | Puzzle: %u | Mission: %u | Cyber: %u\n",World.diffCbt,World.diffPuz,World.diffMis,World.diffCyb);
    off += sFormat(creditStats + off,sizeof(creditStats),"Difficulty Index: %.2f\nFinal Score: %u\n\n",stupid,finalScore);
    off += sFormat(creditStats + off,sizeof(creditStats),"Shots Fired: %u\nGrenades Thrown: %u\n",World.shotsFired,World.grenadesThrown);
    off += sFormat(creditStats + off,sizeof(creditStats),"Damage Dealt: %f\nDamage Received: %f\nSaves Scummed: %u\n\nClick to continue...\n",World.damageDealt,World.damageReceived,World.savesScummed);
}
// Rendering Sys
#include "Shaders/shaders.h"
INLINE void ShaderError(u32 s, const char* name) { char er[512]; glGetShaderInfoLog(s,512,NULL,er); DualLogError("%s Comp Fail: %s\n",name,er); OS_Exit(1); }
INLINE u32 CompileShader(u32 type, const char* source, const char* name) { u32 s = glCreateShader(type); glShaderSource(s,1,&source,NULL); glCompileShader(s); i32 ok; glGetShaderiv(s,0x8B81/*GL_COMPILE_STATUS*/,&ok); if (!ok) ShaderError(s,name); return s; }
INLINE u32 LinkProgram(u32* s, i32 num, const char* name) { u32 p = glCreateProgram(); for (i32 i=0;i<num;++i) { glAttachShader(p,s[i]); } glLinkProgram(p); i32 ok; glGetProgramiv(p,0x8B82/*GL_LINK_STATUS*/,&ok); if (!ok) ShaderError(p,name); return p; }
u32 CompileAnyShader(const char* v, const char* s, const char* name) { return (v) ? LinkProgram((u32[]){CompileShader(0x8B31/*GL_VERTEX_SHADER*/,v,name),CompileShader(0x8B30/*GL_FRAGMENT_SHADER*/,s,name)},2,name) : LinkProgram((u32[]){CompileShader(0x91B9/*GL_COMPUTE_SHADER*/,s,name)},1,name); }
void CompileShaders() {
    depthPrepassSP=CompileAnyShader(depthPrepassVertSrc,depthPrepassFragSrc,"DPre"); chunkSP=CompileAnyShader(vertSrc,fragSrc,"Main"); uiSP=CompileAnyShader(vertUISrc,fragUISrc,"UI"); debugUnlitSP=CompileAnyShader(debugUnlitVertSrc,debugUnlitFragSrc,"Ln");
    shadowmapsSP=CompileAnyShader(shadowmapVertSrc,shadowmapFragSrc,"Shad"); textSP=CompileAnyShader(textVertSrc,textFragSrc,"Txt"); imageBlitSP=CompileAnyShader(quadVertSrc,quadFragSrc,"Comp"); ssrSP=CompileAnyShader(NULL,ssrCSSrc,"SSR"); voxelUpdateSP=CompileAnyShader(NULL,voxUpdCSSrc,"Vox"); 
    shadowmapsClearSP=CompileAnyShader(NULL,shadClearCSSrc,"ShadCl"); psysSp=CompileAnyShader(particleVertSrc,particleFragSrc,"Psys");
}

u32 MakeSSBO(u32* id, u32 bindx, size_t sz, const void* d, u32 typ) { glGenBuffers(1,id); glBindBuffer(GL_SSBO,*id); glBufferData(GL_SSBO,sz,d,typ); glBindBufferBase(GL_SSBO,bindx,*id); return *id; }
void mat4_lookat_from(float* m, Quaternion* camRotation, V3 eye) { // Kept around for light views for shadowmap cubemap faces.
    float x = camRotation->x, y = camRotation->y, z = camRotation->z, w = camRotation->w;
    float x2 = x * x, y2 = y * y, z2 = z * z; float xy = x * y, xz = x * z, yz = y * z; float wx = w * x, wy = w * y, wz = w * z;
    V3 right   = { 1.0f - 2.0f * (y2 + z2),        2.0f * (xy + wz),        2.0f * (xz - wy) };  // X+ (right)
    V3 up      = {        2.0f * (xy - wz), 1.0f - 2.0f * (x2 + z2),        2.0f * (yz + wx) };  // Y+ (up)
    V3 forward = {        2.0f * (xz + wy),        2.0f * (yz - wx), 1.0f - 2.0f * (x2 + y2) };  // Z+ (forward)
    m[0]  = right.x;   m[1]  = up.x;   m[2]  = -forward.x;// m[3]  = 0.0f;
    m[4]  = right.y;   m[5]  = up.y;   m[6]  = -forward.y;// m[7]  = 0.0f;
    m[8]  = right.z;   m[9]  = up.z;   m[10] = -forward.z;// m[11] = 0.0f;
    m[12] = -V3_dot(right, eye); m[13] = -V3_dot(up, eye); m[14] = V3_dot(forward, eye); m[15] = 1.0f;
}

__attribute__((pure,always_inline)) bool SphereInFrustum(FrustumPlane* ps, V3 c, float radius) { for (int i=0;i<6;++i) { if ((V3_dot(ps[i].normal,c) + ps[i].d) < -radius) return false; } return true; }
void ExtractFrustumPlanes(float* m, FrustumPlane* ps) {
    ps[0].normal.x = m[3] + m[0]; ps[0].normal.y = m[7] + m[4]; ps[0].normal.z = m[11] + m[8];  ps[0].d = m[15] + m[12]; // Left
    ps[1].normal.x = m[3] - m[0]; ps[1].normal.y = m[7] - m[4]; ps[1].normal.z = m[11] - m[8];  ps[1].d = m[15] - m[12]; // Right
    ps[2].normal.x = m[3] + m[1]; ps[2].normal.y = m[7] + m[5]; ps[2].normal.z = m[11] + m[9];  ps[2].d = m[15] + m[13]; // Bottom
    ps[3].normal.x = m[3] - m[1]; ps[3].normal.y = m[7] - m[5]; ps[3].normal.z = m[11] - m[9];  ps[3].d = m[15] - m[13]; // Top
    ps[4].normal.x = m[3] + m[2]; ps[4].normal.y = m[7] + m[6]; ps[4].normal.z = m[11] + m[10]; ps[4].d = m[15] + m[14]; // Near
    ps[5].normal.x = m[3] - m[2]; ps[5].normal.y = m[7] - m[6]; ps[5].normal.z = m[11] - m[10]; ps[5].d = m[15] - m[14]; // Far
    for (int i=0;i<6;i++) { float len = V3_Mag(ps[i].normal); if(len > 1e-6f){ps[i].normal.x /= len; ps[i].normal.y /= len; ps[i].normal.z /= len; ps[i].d /= len;} } //Normalize (could use V3_Normalize but need len for d term of FrustumPlane).
}

INLINE void mul_mat4(float *out, const float *a, const float *b) { // out = a * b
    out[0] =  a[0] * b[0]  + a[4] * b[1]  + a[8]  * b[2] + a[12]  * b[3]; out[1] =  a[1] * b[0]  + a[5] * b[1]  + a[9]  * b[2] + a[13]  * b[3];
    out[2] =  a[2] * b[0]  + a[6] * b[1] + a[10]  * b[2] + a[14]  * b[3]; out[3] =  a[3] * b[0]  + a[7] * b[1] + a[11]  * b[2] + a[15]  * b[3];
    out[4] =  a[0] * b[4]  + a[4] * b[5]  + a[8]  * b[6] + a[12]  * b[7]; out[5] =  a[1] * b[4]  + a[5] * b[5]  + a[9]  * b[6] + a[13]  * b[7];
    out[6] =  a[2] * b[4]  + a[6] * b[5] + a[10]  * b[6] + a[14]  * b[7]; out[7] =  a[3] * b[4]  + a[7] * b[5] + a[11]  * b[6] + a[15]  * b[7];
    out[8] =  a[0] * b[8]  + a[4] * b[9]  + a[8] * b[10] + a[12] * b[11]; out[9] =  a[1] * b[8]  + a[5] * b[9]  + a[9] * b[10] + a[13] * b[11];
    out[10] = a[2] * b[8]  + a[6] * b[9] + a[10] * b[10] + a[14] * b[11]; out[11] = a[3] * b[8]  + a[7] * b[9] + a[11] * b[10] + a[15] * b[11];
    out[12] = a[0] * b[12] + a[4] * b[13] + a[8] * b[14] + a[12] * b[15]; out[13] = a[1] * b[12] + a[5] * b[13] + a[9] * b[14] + a[13] * b[15];
    out[14] = a[2] * b[12] + a[6] * b[13] + a[10]* b[14] + a[14] * b[15]; out[15] = a[3] * b[12] + a[7] * b[13] + a[11]* b[14] + a[15] * b[15];
}

void RenderUIImage(i16 x, i16 y, i16 width, i16 height, u32 texIndex) {
    glUseProgram(uiSP); glDisable(GL_BLEND); glBindVertexArray(textVAO); glUniform1ui(0,texIndex); glBindBuffer(GL_ARRAY_BUFFER,textVBO);
    float x1=x + width, y1=y + height, z=0.0f; float vertices[30] = {x,y1,z,0.0f,0.0f,x1,y,z,1.0f,1.0f,x1,y1,z,1.0f,0.0f,x,y1,z,0.0f,0.0f,x,y,z,0.0f,1.0f,x1,y,z,1.0f,1.0f};
    glBufferData(GL_ARRAY_BUFFER,30 * sizeof(float),vertices,GL_DYNAMIC_DRAW); glDrawArrays(0x0004/*GL_TRIANGLES*/,0,6); drawCalls++; uiDrawCalls++; vertsRendered += 6; glBindBuffer(GL_ARRAY_BUFFER,0);
}

void ClearAll() { glBindFramebuffer(GL_FRAMEBUFFER,gBufferFBO); glClearColor(0,0,0,1); glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); glBindFramebuffer(GL_FRAMEBUFFER,uiFBO); glClearColor(0,0,0,0); glClear(GL_COLOR_BUFFER_BIT); glBindFramebuffer(GL_FRAMEBUFFER,0); glClearColor(0,0,0,1); glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); }
void RenderLoading(i32 offset, const char * restrict text) { ClearAll(); glViewport(0,0,Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight); RenderFormattedText(683 - offset,379,T_WHITE,FONT_NORMAL,1.0f,text); ((WinSyswindow*)window)->context.swapBuffers(((WinSyswindow*)window)); }
void GenerateAndBindTexture(u32 *id, i32 internalFormat, i32 width, i32 height, u32 format, u32 type, i32 filt, u8* bmp) { if (*id == 0) {glGenTextures(1,id);} glBindTexture(GL_TEXTURE_2D,*id); glTexImage2D(GL_TEXTURE_2D,0,internalFormat,width,height,0,format,type,bmp); glTexParameteri(GL_TEXTURE_2D,0x2801/*GL_TEXTURE_MIN_FILTER*/,filt); glTexParameteri(GL_TEXTURE_2D,0x2800/*GL_TEXTURE_MAG_FILTER*/,filt); }
static void GenBTexture(u32 *id, i32 internalFormat, i32 width, i32 height, u32 format, u32 type, i32 filt) { GenerateAndBindTexture(id,internalFormat,width,height,format,type,filt,NULL); }
void UpdateScreenSize(i32 width, i32 height) {
    u16 w = Sys_Settings.ScreenWidth = vmax(vmin((u16)width,7680u),320u), h = Sys_Settings.ScreenHeight = vmax(vmin((u16)height,4320u),200u); // Cap at minimum Quake resolution and maximum 8k.
    float wf = (float)w, hf = (float)h; Sys_Settings.ScreenCenterX = wf * 0.5f; Sys_Settings.ScreenCenterY = hf * 0.5f;
    glViewport(0,0,w,h);
    glUseProgram(imageBlitSP); glUniform1ui(2,w); glUniform1ui(3,h); glUniform1i(26,Sys_Settings.SSR_RES); glUseProgram(chunkSP); glUniform1ui(6,w); glUniform1ui(7,h); glUseProgram(ssrSP); glUniform1ui(0,w / Sys_Settings.SSR_RES); glUniform1ui(1,h / Sys_Settings.SSR_RES); glUniform1i(2,Sys_Settings.SSR_RES);
    GenBTexture(&inputImageID, GL_RGBA8,w,h,GL_RGBA,GL_UNSIGNED_BYTE,0x2600/*GL_NEAREST*/); // Lit Raster
    GenBTexture(&inputSpecID,  GL_RGBA8,w,h,GL_RGBA,GL_UNSIGNED_BYTE,0x2600/*GL_NEAREST*/); // Specular Colors
    GenBTexture(&inputNormalID,GL_RG16F,w,h, GL_RGB,        GL_FLOAT,0x2600/*GL_NEAREST*/); // Normal XYZ
    GenBTexture(&inputDepthID,0x81A7/*GL_DEPTH_COMPONENT32*/,w,h,0x1902/*GL_DEPTH_COMPONENT*/,GL_FLOAT,0x2600/*GL_NEAREST*/); // Raster Depth
    GenBTexture(&outputImageID,GL_RGBA8,w / Sys_Settings.SSR_RES,h / Sys_Settings.SSR_RES,GL_RGBA,GL_UNSIGNED_BYTE,0x2601/*GL_LINEAR*/);
    glBindFramebuffer(GL_FRAMEBUFFER,gBufferFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,inputImageID,0);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT1,GL_TEXTURE_2D,inputSpecID,0);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT2,GL_TEXTURE_2D,inputNormalID,0);
    glFramebufferTexture2D(GL_FRAMEBUFFER,0x8D00/*GL_DEPTH_ATTACHMENT*/,GL_TEXTURE_2D,inputDepthID,0);
    glBindImageTexture(0,inputImageID,0,GL_FALSE,0,GL_READ_WRITE,GL_RGBA8);      // Main Rendered Color
    glBindImageTexture(2,inputSpecID,0,GL_FALSE,0,GL_READ_WRITE,GL_RGBA8);       // Specular
    glBindImageTexture(4,outputImageID,0,GL_FALSE,0,GL_READ_WRITE,GL_RGBA8);     // SSR result
    glBindImageTexture(5,inputNormalID,0,GL_FALSE,0,GL_READ_WRITE,GL_RG16F);     // Normal XYZ
    glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D,outputImageID);
    glBindFramebuffer(GL_FRAMEBUFFER,0); ignore_next_mouse_delta = true;
}

bool MenuEnter() { return (Sys_Input.keyStates[KEY_KP_ENTER].pressed || Sys_Input.keyStates[KEY_ENTER].pressed); }
static inline __attribute__((always_inline,pure)) bool CursorIsOverBounds(float startX, float endX, float startY, float endY) { return World.cursorPosition_x >= startX && World.cursorPosition_x <= endX  /* 0 == left */ && World.cursorPosition_y >= startY && World.cursorPosition_y <= endY; /* 0 ==  top */ }
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
    if (*sliderActive && World.currentMouse_dx != 0) { i32 new = (i32)currentValue + vmin(vmax(World.currentMouse_dx,-1),1); *out = (u8)vmin(vmax(new,min),max); if (*out != currentValue) {changed = true;} }
    if (!AnyLeftRightMouseDown()) { if (*sliderActive) { *sliderActive = false; SaveConfig(); } }
    if (MenuEnter() && currentMenuItem == mindex) {
        bool shiftHeld = Sys_Input.keyStates[KEY_LEFT_SHIFT].down || Sys_Input.keyStates[KEY_RIGHT_SHIFT].down;
        if (shiftHeld) *out = *out <=  ((min + step) - 1) ? max : *out - step;
        else           *out = *out >= ((max - step) + 1) ?  min : *out + step;
        changed = true;
    }
    over = over || currentMenuItem == mindex;
    RenderFormattedText(xPosForLabel,y,over ? T_YELLOW : T_GREEN,FONT_NORMAL,1.0f,"%s %u",Sys_Text.stringTable[lingdex],*out);
    return changed;
}

u8 UI_MenuButton(i16 bX, i16 bY, u8 menuItem, i16 bW, i16 bH,  i16 tX, i16 tY, const char* text, i16 pX, i16 pY) {
    bool over = false; u8 retvalue = 0u;
    retvalue = UI_Button(bX,bY,bW,bH,&over,menuItem); if (!retvalue) retvalue = (MenuEnter() && currentMenuItem == menuItem);
    over = over || currentMenuItem == menuItem;
    RenderFormattedText(tX,tY,over ? T_STOPD_RED : T_RED_MENU,FONT_STOPD,1.5f,text); 
    RenderUIImage(pX,pY,40,40,over ? 1029 : 1028); // Menu pad
    return retvalue;
}

bool UI_Checkbox(i16 x, i16 y, i8 mitem, u16 textIdx, bool currentlyOn) {
    RenderUIImage(x,y,16,16,910); // Checkbox background
    bool over = false; bool changed = (UI_Button(x,y + 16,210,16,&over,mitem) || (MenuEnter() && currentMenuItem == mitem)); over = over || currentMenuItem == mitem;
    if (currentlyOn) RenderUIImage(x + 2,y + 2, 12,12, 912); // Checkbox check
    RenderFormattedText(x + 20,y,over ? T_YELLOW : T_GREEN,FONT_NORMAL,1.0f,Sys_Text.stringTable[textIdx]);
    return changed;
}

void UI_HeaderText(i16 x, const char* text) { RenderFormattedText(x,50,T_GREEN_MENU_SHADOW,FONT_STOPD,1.75f,text); RenderFormattedText(x,46,T_GREEN_MENU_GLOW,FONT_STOPD,1.75f,text); RenderFormattedText(x,48,T_GREEN_MENU,FONT_STOPD,1.75f,text); }
void MenuGoBack() {
    if (returnToPause) { returnToPause = false; World.paused = true; World.menuActive = false; PlayGameMusic(); }
    if (currentMenuPage == Mpg_Singleplayer || currentMenuPage == Mpg_Multiplayer || currentMenuPage == Mpg_Options) currentMenuPage = Mpg_FrontPage;//News
    else if (currentMenuPage == Mpg_Load || currentMenuPage == Mpg_NewGame || currentMenuPage == Mpg_IntroVideo || currentMenuPage == Mpg_CreditsVideo) currentMenuPage = Mpg_Singleplayer;
}

void CreateShadowBuffers() { shadowMapSSBO=MakeSSBO(&shadowMapSSBO,5,(MAX_SHADOWMAPS * (SHADOW_MAP_SIZE * SHADOW_MAP_SIZE * 6U)) * sizeof(u32),NULL,GL_STATIC_DRAW); shadowMapsIndirectionID=MakeSSBO(&shadowMapsIndirectionID,6,LIGHT_COUNT * sizeof(u32),NULL,GL_STATIC_DRAW); shadowBuffersCreated=true; }
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
        RenderFormattedText(1076,732,overBack ? T_STOPD_RED_HIGHLIGHT : T_RED_MENU,FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == Mpg_Multiplayer) {
        menuItemCount = 1; menuTabCount = 1;
        UI_HeaderText(266,/*"MULTIPLAYER"*/Sys_Text.stringTable[720]);
        RenderUIImage(1060,724, 84,36, 1252); // Back Button background
        bool overBack = false;
        if (UI_Button(1060,758, 84,32, &overBack, 0) || (MenuEnter() && currentMenuItem == 0)) MenuGoBack();
        overBack = overBack || currentMenuItem == 0;
        RenderFormattedText(1076,732,overBack ? T_STOPD_RED_HIGHLIGHT : T_RED_MENU,FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == Mpg_Options) {
        menuTabCount = 3;
        UI_HeaderText(238,/*"CONFIGURATION"*/Sys_Text.stringTable[745]);
        if (currentMenuTab != 0) RenderUIImage(179,220, 1001,548, 1030); // Config background
        if (currentMenuTab == 0) RenderUIImage(179,220, 1001,548, 1033); // Config background graphics (empty alpha center)
        RenderUIImage(520,196, 160,30, currentMenuTab == 2 ? 920 : 921); // Config tab unhighlighted
        if (UI_Button(520,196+30, 160,30, NULL, 2)) currentMenuTab = 2;
        RenderFormattedText(530,202,currentMenuTab == 2 ? T_YELLOW : T_GREEN,FONT_NORMAL,1.0f,/*"AUDIO / LANG"*/Sys_Text.stringTable[793]);
        RenderUIImage(354,196, 160,30, currentMenuTab == 1 ? 920 : 921); // Config tab unhighlighted
        if (UI_Button(354,196+30, 160,30, NULL, 1)) currentMenuTab = 1;
        RenderFormattedText(366,202,currentMenuTab == 1 ? T_YELLOW : T_GREEN,FONT_NORMAL,1.0f,/*"INPUT"*/Sys_Text.stringTable[792]);
        RenderUIImage(190,196, 160,30, currentMenuTab == 0 ? 920 : 921); // Config tab highlighted
        if (UI_Button(190,196+30, 160,30, NULL, 0)) currentMenuTab = 0;
        RenderFormattedText(200,202,currentMenuTab == 0 ? T_YELLOW : T_GREEN,FONT_NORMAL,1.0f,/*"GRAPHICS"*/Sys_Text.stringTable[791]);
        if (currentMenuTab == 0) {
            bool overRes = false, overFull = false, overChgM = false;
            menuItemCount = 11; // Graphics
            if (UI_Checkbox(200,500,0,Sys_Settings.ModelDetail ? /*High*/915 : /*No Detail Level Models*/914,Sys_Settings.ModelDetail)) { Sys_Settings.ModelDetail = Sys_Settings.ModelDetail ? 0u : 1u; SaveConfig(); }
            if (UI_Checkbox(200,530,1,/*"FXAA"*/780,Sys_Settings.FXAA)) { Sys_Settings.FXAA = Sys_Settings.FXAA ? 0u : 1u; SaveConfig(); }
            if (UI_Checkbox(200,560,2,Sys_Settings.Shadows ? /*Soft*/787 : /*No Shadows*/785,Sys_Settings.Shadows)) { Sys_Settings.Shadows = Sys_Settings.Shadows ? 0u : 1u; if (!shadowBuffersCreated) {CreateShadowBuffers();} SaveConfig(); }
            if (UI_Checkbox(200,590,3,/*SSR*/788,Sys_Settings.Reflections)) { Sys_Settings.Reflections = Sys_Settings.Reflections ? 0u : 1u; SaveConfig(); }
            if (UI_Checkbox(200,620,4,/*VSYNC*/1026,Sys_Settings.Vsync)) { Sys_Settings.Vsync = Sys_Settings.Vsync ? 0u : 1u; SetVSync(); SaveConfig(); }
            RenderFormattedText(310,620,T_GREEN,FONT_NORMAL,1.0f,"(FPS: %d)", globalframesPerLastSecond); // Helper to see vsync take effect.
            u8 newVal;
            if (UI_Slider(400,650,128,16,(((Sys_Settings.FOV - 45.0f) / 105.0f) * (128 - 16)),200,Sys_Settings.FOV,&newVal,&fovSliderActive,45,150,5,5,/*Field of View*/775)) { Sys_Settings.FOV = newVal; if (!AnyLeftRightMouseDown()) {SaveConfig();} }
            if (UI_Slider(400,680,128,16,((Sys_Settings.Brightness / 100.0f) * (128 - 16)),200,Sys_Settings.Brightness,&newVal,&gammaSliderActive,0,100,2,6,/*Gamma*/774)) { Sys_Settings.Brightness = newVal; if (!AnyLeftRightMouseDown()) {SaveConfig();} }
            
            // Resolution
            {
                // Header hit area - UI_Button subtracts h from y internally, so pass y+h as y
                if (UI_Button(190,726,328,16,&overRes,7) || (MenuEnter() && currentMenuItem == 7)) { resDropdownOpen = !resDropdownOpen; currentMenuItem = 7; }
                overRes = overRes || currentMenuItem == 7;
                char resBuf[32];
                if (resDropdownCount > 0) sFormat(resBuf, sizeof(resBuf), "%ux%u",(u32)resModes[resSelectedIdx].w,(u32)resModes[resSelectedIdx].h);
                else sFormat(resBuf, sizeof(resBuf), "%ux%u",Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight);

                RenderUIImage(476, 710, 16, 16, overRes ? 1119 : 1077);
                RenderFormattedText(200, 710, overRes ? T_YELLOW : T_GREEN,FONT_NORMAL, 1.0f, "RESOLUTION %s", resBuf);
            }
    
            // Fullscreen checkbox
            RenderUIImage(200,740, 16,16, 910); // Checkbox background
            if (UI_Button(200,756, 210,16, &overFull, 8) || (MenuEnter() && currentMenuItem == 8)) { Sys_Settings.Fullscreen = Sys_Settings.Fullscreen == 1u ? 0u : 1u; ChangeFullScreenWindowed(); SaveConfig(); }
            overFull = overFull || currentMenuItem == 8;
            if (Sys_Settings.Fullscreen) RenderUIImage(202,742, 12,12, 912); // Checkbox check
            RenderFormattedText(220,740,overFull ? T_YELLOW : T_GREEN,FONT_NORMAL,1.0f,/*"Fullscreen"*/Sys_Text.stringTable[773]);
            RenderUIImage(588,730, 210,30, 1079); // Toggle monitor button background
            if (UI_Button(588,760, 210,30, &overChgM, 9) || (MenuEnter() && currentMenuItem == 9)) { CycleToNextMonitor(); }
            overChgM = overChgM || currentMenuItem == 9;
            RenderFormattedText(602,735,overChgM ? T_YELLOW : T_GREEN,FONT_NORMAL,1.0f,/*"CHANGE MONITOR"*/Sys_Text.stringTable[1025]);
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
        RenderFormattedText(1103,731,overBack ? T_STOPD_RED_HIGHLIGHT : T_RED_MENU,FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == Mpg_Load || currentMenuPage == Mpg_Save) {
        menuItemCount = 9; menuTabCount = 1;
        bool isSave = currentMenuPage == Mpg_Save;
        UI_HeaderText(isSave ? 284 : 340, isSave ? /*"SAVE GAME"*/Sys_Text.stringTable[769] : /*"LOAD"*/Sys_Text.stringTable[726]);
        RenderUIImage(400,214, 586,500, 1037); // Load/Save table background
        RenderUIImage(1060,724, 84,36, 1252); // Back Button background
        bool overBack = false;
        if (UI_Button(1060,758, 84,32, &overBack, 0) || (MenuEnter() && currentMenuItem == 0)) MenuGoBack();
        overBack = overBack || currentMenuItem == 0;
        RenderFormattedText(1076,732, overBack ? T_STOPD_RED_HIGHLIGHT : T_RED_MENU, FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == Mpg_NewGame) {
        menuItemCount = 7; menuTabCount = (currentMenuItem > 0 && currentMenuItem <= 16) ? 2 : 1;
        UI_HeaderText(290,/*"NEW GAME"*/Sys_Text.stringTable[741]);
        RenderUIImage(136,196,1088,558,1048); // Newgame inset
        RenderUIImage(136,196,1088,558,1049); // Newgame background
        if (UI_MenuButton(276,270,0,795,74, 226,146,/*"NAME:"*/Sys_Text.stringTable[746],299,214)) { /* Just for highlight */ }
        enteringPlayerName = (currentMenuItem == 0);
        if (World.playerName[0] == '\0') RenderFormattedText(642,232,T_RED_MENU,FONT_STOPD,1.0f,/*"ENTER NAME..."*/Sys_Text.stringTable[748]);
        else                                  RenderFormattedText(518,232,enteringPlayerName ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.0f,World.playerName);
        if (UI_MenuButton(174,377,1,496,95, 148,202,/*"COMBAT"*/Sys_Text.stringTable[748],185,299)) { World.diffCbt = World.diffCbt >= 3 ? 0 : World.diffCbt + 1; }  if (UI_MenuButton(704,377,3,496,95, 510,202,/*"MISSION"*/Sys_Text.stringTable[749],726,299)) { World.diffMis = World.diffMis >= 3 ? 0 : World.diffMis + 1; }
        RenderFormattedText(162,270,World.diffCbt == 0 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"0");    RenderFormattedText(513,270,World.diffMis == 0 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"0");
        RenderFormattedText(233,270,World.diffCbt == 1 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"1");    RenderFormattedText(584,270,World.diffMis == 1 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"1");
        RenderFormattedText(307,270,World.diffCbt == 2 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"2");    RenderFormattedText(658,270,World.diffMis == 2 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"2");
        RenderFormattedText(379,270,World.diffCbt == 3 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"3");    RenderFormattedText(730,270,World.diffMis == 3 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"3");
        if (UI_MenuButton(174,568,2,496,92, 149,330,/*"PUZZLE"*/Sys_Text.stringTable[751],185,490)) { World.diffPuz = World.diffPuz >= 3 ? 0 : World.diffPuz + 1; }  if (UI_MenuButton(704,568,4,496,92, 509,330,/*"CYBERSPACE"*/Sys_Text.stringTable[750],726,490)) { World.diffCyb = World.diffCyb >= 3 ? 0 : World.diffCyb + 1; }
        RenderFormattedText(162,399,World.diffPuz == 0 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"0");    RenderFormattedText(513,399,World.diffCyb == 0 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"0");
        RenderFormattedText(233,399,World.diffPuz == 1 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"1");    RenderFormattedText(584,399,World.diffCyb == 1 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"1");
        RenderFormattedText(307,399,World.diffPuz == 2 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"2");    RenderFormattedText(658,399,World.diffCyb == 2 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"2");
        RenderFormattedText(379,399,World.diffPuz == 3 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"3");    RenderFormattedText(730,399,World.diffCyb == 3 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"3");
        if (UI_Button(221,460,82,79,NULL,1)) {World.diffCbt =0; currentMenuItem=1; } if (UI_Button(330,460,82,79,NULL,1)) {World.diffCbt =1; currentMenuItem=1; } if (UI_Button(439,460,82,79,NULL,1)) {World.diffCbt =2; currentMenuItem=1; } if (UI_Button( 547,460,82,79,NULL,1)) {World.diffCbt =3; currentMenuItem=1; }
        if (UI_Button(221,651,82,79,NULL,2)) {World.diffPuz =0; currentMenuItem=2; } if (UI_Button(330,651,82,79,NULL,2)) {World.diffPuz =1; currentMenuItem=2; } if (UI_Button(439,651,82,79,NULL,2)) {World.diffPuz =2; currentMenuItem=2; } if (UI_Button( 547,651,82,79,NULL,2)) {World.diffPuz =3; currentMenuItem=2; }
        if (UI_Button(748,460,82,79,NULL,3)) {World.diffMis=0; currentMenuItem=3; } if (UI_Button(857,460,82,79,NULL,3)) {World.diffMis=1; currentMenuItem=3; } if (UI_Button(966,460,82,79,NULL,3)) {World.diffMis=2; currentMenuItem=3; } if (UI_Button(1074,460,82,79,NULL,3)) {World.diffMis=3; currentMenuItem=3; }
        if (UI_Button(748,651,82,79,NULL,4)) {World.diffCyb  =0; currentMenuItem=4; } if (UI_Button(857,651,82,79,NULL,4)) {World.diffCyb  =1; currentMenuItem=4; } if (UI_Button(966,651,82,79,NULL,4)) {World.diffCyb  =2; currentMenuItem=4; } if (UI_Button(1074,651,82,79,NULL,4)) {World.diffCyb  =3; currentMenuItem=4; }
        bool overBack = false, overStart = false;
        if (UI_Button(544,747, 282,68, &overStart, 5) || (MenuEnter() && currentMenuItem == 5)) GoIntoGame(); // TODO reload game.
        overStart = overStart || currentMenuItem == 5;
        RenderFormattedText(400,464,overStart ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,/*"START"*/Sys_Text.stringTable[886]);
        if (UI_Button(1060,758, 84,32, &overBack, 6) || (MenuEnter() && currentMenuItem == 6)) MenuGoBack();
        overBack = overBack || currentMenuItem == 6;
        RenderUIImage(1060,724,84,36,1252); // Back Button background
        RenderFormattedText(1076,732,overBack ? T_STOPD_RED_HIGHLIGHT : T_RED_MENU,FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == Mpg_IntroVideo || currentMenuPage == Mpg_CreditsVideo) {
        menuItemCount = menuTabCount = 1;
        if (MenuEnter()) MenuGoBack();
    }
    if (menuTabCount <= currentMenuTab) currentMenuTab = 0;
    if (menuItemCount <= currentMenuItem) currentMenuItem = 0;
    static const i8 ngSwap[7] = {0,3,4,1,2,6,5};
    if (Sys_Input.keyStates[KEY_RIGHT].pressed || Sys_Input.keyStates[KEY_LEFT].pressed) { int dir = Sys_Input.keyStates[KEY_RIGHT].pressed ? 1 : -1; currentMenuTab = (currentMenuTab + menuTabCount + dir) % menuTabCount; if (currentMenuPage == Mpg_NewGame && currentMenuItem < 7) {currentMenuItem=ngSwap[currentMenuItem];} }
}

void RenderPausedUI() {
    menuItemCount = 6; menuTabCount = 1;
    bool overResume = false, overLoad /* ;) */ = false, overSave = false, overOptions = false, overQuitMenu = false, overQuit = false;
    RenderUIImage(519,276,328,300,1025); // Pause Menu background
    RenderUIImage(519,276,328,300,1080); // Pause Menu background outline
    RenderFormattedText(610,210,T_STOPD_RED_PAUSETITLE,FONT_STOPD,1.0f,/*"PAUSED"*/Sys_Text.stringTable[724]);
    if (UI_Button(522,330, 322,52, &overResume, 0) || (MenuEnter() && currentMenuItem == 0)) World.paused = false;
    overResume = overResume || currentMenuItem == 0;
    RenderFormattedText(610,306,overResume ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.0f,/*"RESUME"*/Sys_Text.stringTable[725]);
    if (UI_Button(522,390, 322,52, &overLoad, 1) || (MenuEnter() && currentMenuItem == 1)) { currentMenuPage = Mpg_Load; PlayMenuMusic(); World.menuActive = true; returnToPause = true; }
    overLoad = overLoad || currentMenuItem == 1;
    RenderFormattedText(630,364, overLoad ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.0f,/*"LOAD"*/Sys_Text.stringTable[726]);
    if (UI_Button(522,450, 322,60, &overSave, 2) || (MenuEnter() && currentMenuItem == 2)) { currentMenuPage = Mpg_Save; PlayMenuMusic(); World.menuActive = true; returnToPause = true; }
    overSave = overSave || currentMenuItem == 2;
    RenderFormattedText(635,422,overSave ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.0f,/*"SAVE"*/Sys_Text.stringTable[727]);
    if (UI_Button(522,510, 322,60, &overOptions, 3) || (MenuEnter() && currentMenuItem == 3)) { currentMenuPage = Mpg_Options; PlayMenuMusic(); World.menuActive = true; returnToPause = true; }
    overOptions = overOptions || currentMenuItem == 3;
    RenderFormattedText(599,480,overOptions ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.0f,/*"OPTIONS"*/Sys_Text.stringTable[721]);
    if (UI_Button(522,570, 322,60, &overQuitMenu, 4) || (MenuEnter() && currentMenuItem == 4)) { PlayMenuMusic(); World.menuActive = true; currentMenuPage = Mpg_FrontPage; }
    overQuitMenu = overQuitMenu || currentMenuItem == 4;
    RenderFormattedText(546,538,overQuitMenu ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.0f,/*"QUIT TO MENU"*/Sys_Text.stringTable[728]);
    RenderUIImage(519,672,328,42,1252); // Pause Quit Game background
    if (UI_Button(522,714, 322,42, &overQuit, 5) || (MenuEnter() && currentMenuItem == 5)) OS_Exit(0);
    overQuit = overQuit || currentMenuItem == 5;
    RenderFormattedText(572,690,overQuit ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.0f,/*"QUIT GAME"*/Sys_Text.stringTable[729]);
}

u8 MFD_LefTab=0,MFD_CenterTab=0,MFD_RightTab=0;
static double RenderUI() {
    drawCallsNormal = drawCalls;
    if (World.creditsActive) { // Render Credits
        if (Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed) { ++World.creditsPageIndex; if(World.creditsPageIndex > CREDITS_PAGES){World.creditsActive=false; return get_time();} /*Finished with Erthang!  That's it, go home.*/ }
        if (World.creditsPageIndex == 1) { CreditsStats(); RenderFormattedText(300,10,T_WHITE,FONT_NORMAL,1.0f,(const char*)&creditStats); }
        else                                                    RenderFormattedText(300,10,T_WHITE,FONT_NORMAL,1.0f,creditPages[World.creditsPageIndex]);
        return get_time();
    }
    if (World.menuActive) RenderMenu();
    else if (World.paused) RenderPausedUI();
    if ((World.menuActive || World.paused)) {
        if (Sys_Input.keyStates[KEY_DOWN].pressed) currentMenuItem = (currentMenuItem + 1) >= menuItemCount ? 0 : (currentMenuItem + 1);
        else if (Sys_Input.keyStates[KEY_UP].pressed) currentMenuItem = (currentMenuItem - 1) < 0 ? (menuItemCount - 1) : (currentMenuItem - 1);
    } else { /* Normal UI */
//         if (World.Sys_UI.showTeleportFX) { /*TeleportFX*/ }
//         if (World.Sys_UI.showRadiationFX) { /*RadiationFX*/ }
//         if (World.Sys_UI.showHealingFX) { /*HealingFX*/ }
//         if (World.Sys_UI.showShieldFX) { /*ShieldFX*/ }
//         if (World.Sys_UI.showShieldActivation) { /*waveup*/ /*wavedn*/ }
//         if (World.Sys_UI.showShieldDeactivation) { /*waveup*/ /*wavedn*/ }
//         if (World.Sys_UI.showDeathRessurectionFX) { /*spawndelaycontainers...*/ }
//         if (World.Sys_UI.showHardware) { /*ShieldButton*/ /*LanternButton*/ /*SensaroundButton*/ /*BioButton*/ /*NightVisionButton*/ /*EReaderButton*/ /*BoosterButton*/ /*JumpJetsButton*/ }
        if (!Cheats.noHUD) {RenderUIImage(672,0,22,22,1020);} /*ShootModeButton*/
        if (World.inventoryMode && Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed && CursorIsOverBounds(672,694,22,0)) ForceShootMode();
//         if (World.Sys_UI.showTextWarnings) { /*WarningTexts...*/ }
//         if (World.Sys_UI.showAutomapFull) { /*AutomapFullRawImage*/ /*PlayerIconFull*/ /*CloseFullmapButton*/ }
//         if (World.Sys_UI.showMissionTimer) { /*MissionTimerT*/ /*MissionTimer*/ }
        if (true/*World.Sys_UI.showLeftMFDPanel*/) {
            if (MFD_LefTab == 0) { /*WeaponTabLH: WepNameTextLH, WepIconLH, ClipBox, EnergyHeatTicks, ReloadButtons, EnergySlider*/ }
            else if (MFD_LefTab == 1) { /*ItemTabLH: ItemIcon, ItemText, Vaporize/Apply/Use Buttons, EReaderSections, AccessCardsList, Sliders*/ }
            else if (MFD_LefTab == 2) { /*AutomapTabLH: AutomapMask, Overlays, PlayerIcon, ZoomIn/Out/Full/Side Buttons*/ }
            else if (MFD_LefTab == 3) { /*TargetTabLH*/ }
            else if (MFD_LefTab == 4) { /*DataTabLH: SecurityLH, DataHeaders, ElevatorUIControl, KeycodeUIControl, SearchContents, AudioLogInfo, PuzzleGrid, PuzzleWire, SystemAnalyzer Display*/ }
//             if (World.Sys_UI.showSensaroundLH) { /*SensaroundLH Plane*/ }
            if (true/*World.Sys_UI.showTabButtonsPanelLH*/) {
                RenderUIImage(-16,552,32,40,MFD_LefTab == 0 ? 1024 : 1022); RenderUIImage(-16,600,32,40,MFD_LefTab == 1 ? 1024 : 1022); RenderUIImage(-16,648,32,40,MFD_LefTab == 2 ? 1024 : 1022); RenderUIImage(-16,696,32,40,MFD_LefTab == 3 ? 1024 : 1022); /*ButtonWeapon, ButtonItem, ButtonAutomap, ButtonTarget, ButtonData*/
            }
//             if (World.Sys_UI.showCyberTimer) { /*CyberTimerT*/ /*CyberTimer*/ }
        }
        if (true/*World.Sys_UI.showCenterMFDPanel*/) {
            if (MFD_CenterTab == 0) { /*MainTab: WeaponInventory, WeaponShotsInventory, GrenadeInventory, PatchInventory*/ }
            else if (MFD_CenterTab == 1) { /*HardwareTab: Label, HardwareInventory*/ }
            else if (MFD_CenterTab == 2) { /*GeneralTab: Label, GeneralInventory, AccessCards*/ }
            else if (MFD_CenterTab == 3) { /*SoftwareTab: Label, SoftwareInventory, ICEDrill, Pulser, Turbo, Decoy, Recall*/ }
            else if (MFD_CenterTab == 4) { /*MultiMediaDataReader: LogTableofContents, LogsLevelFolder, LogTextReader, EmailTab, DataTab, NotesTab*/ }
//             if (World.Sys_UI.showSensaroundCenter) { /*SensaroundCenter Plane*/ }
            if (true/*World.Sys_UI.showCenterTabButtons*/) {
                RenderUIImage(400,752,64,32,MFD_CenterTab == 0 ? 1024 : 1021); RenderUIImage(480,752,64,32,MFD_CenterTab == 1 ? 1024 : 1021); RenderUIImage(560,752,64,32,MFD_CenterTab == 2 ? 1024 : 1021); RenderUIImage(902,752,64,32,MFD_CenterTab == 3 ? 1024 : 1021); /*Main, Hardware, General, Software, AddToInventoryHelper*/
            }
        }
        if (true/*World.Sys_UI.showRightMFDPanel*/) {
            if (MFD_RightTab == 0) { /*WeaponTabRH: WepName, ClipBox, HeatTicks, Reload/Unload, EnergySlider*/ }
            else if (MFD_RightTab == 1) { /*ItemTabRH: Icons, Actions, EReaderSections, Sliders*/ }
            else if (MFD_RightTab == 2) { /*AutomapTabRH: AutomapMask, Zoom controls*/ }
            else if (MFD_RightTab == 3) { /*TargetTabRH*/ }
            else if (MFD_RightTab == 4) { /*DataTabRH: SecurityRH, Elevators, Keycodes, AudioLogs, Puzzles, SystemAnalyzer*/ }
//             if (World.Sys_UI.showSensaroundRH) { /*SensaroundRH Plane*/ }
            if (true/*World.Sys_UI.showTabButtonsPanelRH*/) {
                RenderUIImage(1350,552,32,40,MFD_RightTab == 0 ? 1024 : 1022); RenderUIImage(1350,600,32,40,MFD_RightTab == 1 ? 1024 : 1022); RenderUIImage(1350,648,32,40,MFD_RightTab == 2 ? 1024 : 1022); RenderUIImage(1350,696,32,40,MFD_RightTab == 3 ? 1024 : 1022); /*ButtonWeapon, ButtonItem, ButtonAutomap, ButtonTarget, ButtonData*/
            }
        }
//         if (World.Sys_UI.showBioMonitor) { /*Graph*/ /*Biomonitor texts, BPM, Patch, Fatigue*/ }
//         if (World.Sys_UI.showEnergyTickPanel) { /*EnergyTickPanel*/ }
//         if (World.Sys_UI.showHealthTickPanel) { /*HealthTickPanel*/ }
//         if (World.Sys_UI.showEnergyIndicator) { /*EnergyIndicator*/ /*EnergySurge*/ /*EnergyDrainText*/ /*EnergyJPMText*/ }
//         if (World.Sys_UI.showHealthIndicator) { /*HealthIndicator*/ /*HealthIndicatorCyber*/ }
//         if (World.Sys_UI.showVmailPlayer) { /*VmailPlayer GenStatus, BetaJet, etc.*/ }
//         if (World.Sys_UI.showSearchFX) { /*SearchFXLH*/ /*SearchFXRH*/ }
//         if (World.Sys_UI.showTouchables) { /*MainMenuTouch, Console, Left/RightTouchstick, TouchSpace/LMB/Swim*/ }
//         if (World.Sys_UI.showEMPStatic) { /*EMPStatic*/ }
//         if (World.Sys_UI.showPainStatic) { /*PainStatic*/ }
//         if (World.Sys_UI.showSightDimming) { /*SightDimming*/ }
//         if (World.Sys_UI.showDeathFX) { /*DeathFXContainer*/ }
    }
    i16 debugTextStartY = 48; /* Diagnostics / Debugging */
    if (Cheats.showLocation && !World.menuActive) RenderFormattedText(16, debugTextStartY, T_WHITE, FONT_NORMAL,1.0f, "x: %.4f, y: %.4f, z: %.4f, rx: %.4f, ry: %.4f, rz: %.4f, rw: %.4f",World.position[PLAYER1].x,World.position[PLAYER1].y,World.position[PLAYER1].z,World.rotation[PLAYER1].x,World.rotation[PLAYER1].y,World.rotation[PLAYER1].z,World.rotation[PLAYER1].w);
    i16 lineSpacing = 18;
    if (!World.menuActive && !Cheats.noHUD) RenderFormattedText(16,debugTextStartY + (lineSpacing * 1),T_WHITE,FONT_NORMAL,1.0f,"playerCellIdx: %u, Shadow cpu ms: %.3f, Physics cpu ms: %.3f",playerCellIdx,shadowTime * 1000,physTime * 1000);
    if (!World.menuActive && !Cheats.noHUD) RenderFormattedText(16,debugTextStartY + (lineSpacing * 2),T_WHITE,FONT_NORMAL,1.0f,"Player velocity: %.2f, %.2f, %.2f, Grounded: %u",World.velocity[PLAYER1].x,World.velocity[PLAYER1].y,World.velocity[PLAYER1].z,World.instances[PLAYER1].entflags & EF_GROUNDED);
    RenderFormattedText(16,debugTextStartY + (lineSpacing * 4),T_WHITE,FONT_NORMAL,1.0f,"Cursor: %d, %d  dx:%d dy:%d",World.cursorPosition_x,World.cursorPosition_y,World.currentMouse_dx,World.currentMouse_dy);
    if (Cheats.consoleActive) RenderFormattedText(16,0,T_WHITE,FONT_NORMAL,1.0f, "] %s",consoleEntryText);
    if (World.statusTextDecayFinished > World.current_time) RenderFormattedText(460,114,T_WHITE,FONT_NORMAL,1.0f, "%s",statusText);
    double time_now = get_time();
    if (Cheats.showFPS) {
        World.thisFrameTime = (time_now - World.last_time) * 1000.0;
        World.cpuFrameTime = World.cpuTime * 1000.0;
        u8 timingColor = T_WHITE;
        if (vabs(World.thisFrameTime - World.cpuFrameTime) < 0.451) timingColor = T_GREEN;
        if (World.thisFrameTime > 6.944444) timingColor = T_RED;
        drawCalls += 2; /* Add two more for this text render ;) */
        RenderFormattedText(16, debugTextStartY - lineSpacing, timingColor, FONT_NORMAL,1.0f, "ms: %.2f, CPU %.2f", World.thisFrameTime,World.cpuFrameTime);
        RenderFormattedText(16 + 230.0f, debugTextStartY - lineSpacing, T_WHITE, FONT_NORMAL,1.0f, "(FPS:%d),Drwclls:%d [G:%d UI:%d Sh:%d] Vrt:%d E:%u|M:%u|P:%u",globalframesPerLastSecond,drawCalls,drawCallsNormal,uiDrawCalls,shadDrawCalls,vertsRendered,Cheats.editMode,World.menuActive,World.paused);
    }
    return time_now;
}

#define SHADOW_NEARMESH_MAX 1024
typedef struct {float depth; u16 index; } DepthSort;
DepthSort shadows_nearMeshes[SHADOW_NEARMESH_MAX];
INLINE bool EntNotVisible(u16 i, bool otherCondition) { Entity* e = &World.instances[i]; return e->texIndex > texCnt || !(e->entflags & EF_ACTIVE) || e->index >= MAX_ENTITIES || e->modelIndex >= MAX_MDLS || e->texIndex >= MAX_TXRS || otherCondition; }
static inline __attribute__((always_inline,hot)) u16 GetAndBindModel(u16 i, u16 currentModelType) {
    glUniform1ui(0,i);
    u16 modelType = (instanceIsLODArray[i] || Sys_Settings.ModelDetail < 1u) && World.instances[i].lodIndex < mdlsCnt ? World.instances[i].lodIndex : World.instances[i].modelIndex;
    if (currentModelType == modelType && currentModelType != 0) return currentModelType;
    glBindVertexBuffer(0,vbos[modelType],0,VRT_ATT_SZ); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,tbos[modelType]);
    return modelType;
}

#define SC_MAX (SHADOW_NEARMESH_MAX * MAX_SHADOWMAPS)
u16 shadowCasterIndices[SC_MAX],candidates[MAX_SHADOWMAPS];
static const u32 groupX_shadClear = ((SHADOW_MAP_SIZE * SHADOW_MAP_SIZE) + 31) / 32;
static __attribute__((hot)) void RenderShadowmaps() {    
    double shadowStartTime = get_time();
    mset(candidates,0,MAX_SHADOWMAPS*sizeof(u16));
    u16 numShadowsCouldRender = 0;
    V3 playerPos = World.position[PLAYER1], pf = World.instances[PLAYER1].forward;
    for (u16 i = 0; i < World.loadedLights; ++i) { // Collect candidates: only lights that are enabled and in PVS
        if (unlikely(!(World.lights[i].lflags & SHADON) || !(World.lights[i].lflags & LIGHTON))) continue;
        V3 lightPos = World.lights[i].pos;
        float intensity = World.lights[i].maxIntensity; /*Much more stable than actual intensity (from fade/flickers).  Since gated by on above, this is fine now.*/ if (unlikely(intensity < 0.1f)) continue;
        float range =  World.lights[i].range; float luminosity = (intensity / (range * range)); if (luminosity < 0.008f && (range < 8.0f || intensity < 0.5f)) continue;
        u16 cellX = PosGetCellCoordX(lightPos.x), cellZ = PosGetCellCoordZ(lightPos.z);
        int lightCellIdx = (cellZ * WORLDX) + cellX; u8 r = vceil(range * (1.0f / CELL_SIZE));
        bool inPVS = (gridCellStates[lightCellIdx] & CELL_VISIBLE);
        if (likely(!inPVS)) inPVS = NeighborhoodInPVS(cellX,cellZ,r); if (!inPVS) continue;
        float dx = lightPos.x - playerPos.x, dy = lightPos.y - playerPos.y, dz = lightPos.z - playerPos.z;
        float distSqrdToPlayer = dx*dx + dy*dy + dz*dz;
        float dotResult = (dx*pf.x + dy*pf.y + dz*pf.z); if (dotResult < 0.0f && distSqrdToPlayer > (range * range)) continue;
        candidates[numShadowsCouldRender] = i; numShadowsCouldRender++; if (numShadowsCouldRender >= MAX_SHADOWMAPS) break;
    }
    if (numShadowsCouldRender > 0) { // Added since there is now work between here and the for loop so this is beneficial to check.
        glUseProgram(shadowmapsClearSP); // Clear shadowmaps.  One might think that this would be less performant than standard shadowmap FBO with gl clears and textures but in fact this is faster on all but the oldest hardware (e.g. 10yrs old is fine, 13yrs suffers a small hit).
        for (u32 c=0;c<numShadowsCouldRender;++c) { glUniform1ui(0,c); glDispatchCompute(groupX_shadClear,6,1); }
        shadDrawCalls = 0;
        glViewport(0,0,SHADOW_MAP_SIZE,SHADOW_MAP_SIZE);
        glUseProgram(shadowmapsSP);
        u32 shadowmapOffsetHead = 0U; mset(shadowCasterIndices,0,SC_MAX*sizeof(u16)); u32 numShadowCasters = 0;
        for (int i=INSTS_1ST_IDX;i<INSTANCE_COUNT;++i) {
            if(EntNotVisible(i,(World.instances[i].entflags & EF_NO_SHADOWS)) || IdxIsDynamicObject(World.instances[i].index)) {continue;}
            
            shadowCasterIndices[numShadowCasters] = i; numShadowCasters++; if(numShadowCasters >= (SC_MAX)){break;}/*Ran out of shadowcasters max for frame.*/
        }
        u16 shadowMapIdx=0,currentModelType=0,currentTexIndex=0; bool currentIsTransparent=0,useDetail=Sys_Settings.ModelDetail;
        for (u32 c = 0; c < numShadowsCouldRender; ++c, ++shadowMapIdx) { // Render top MAX_SHADOWMAPS candidates
            u16 lightIdx = candidates[c];
            float effectiveRadius = vmin(World.lights[lightIdx].range,15.36f); u16 nearbyMeshCount = 0; 
            V3 lpos = World.lights[lightIdx].pos;
            float cellCenterX=vround(lpos.x / CELL_SIZE) * CELL_SIZE, cellCenterZ=vround(lpos.z / CELL_SIZE) * CELL_SIZE;
            V3 deltaCellCenter = V3_AsubB((V3){lpos.x,0.0f,lpos.z},(V3){cellCenterX,0.0f,cellCenterZ});
            float distToCenterSqrd = V3_dot(deltaCellCenter,deltaCellCenter);
            bool skipNPCs = (distToCenterSqrd < 0.4096f); // 0.64 * 0.64
            for (u16 shadowCasterInstanceIdx = 0; shadowCasterInstanceIdx < numShadowCasters; shadowCasterInstanceIdx++) {
                u16 j = shadowCasterIndices[shadowCasterInstanceIdx];
                Entity* e = &World.instances[j];
                V3 d = V3_AsubB(World.position[j],lpos);
                float distToLightSqrd = V3_dot(d,d);
                float radSum = (effectiveRadius + World.radius[j]);
                if (distToLightSqrd >= radSum * radSum) continue;
                if (skipNPCs && IdxIsNPC(e->index)) continue;
                shadows_nearMeshes[nearbyMeshCount].index = j; shadows_nearMeshes[nearbyMeshCount].depth = distToLightSqrd; 
                nearbyMeshCount++; if (nearbyMeshCount >= SHADOW_NEARMESH_MAX) { DualLogWarn("Shadowmapping needs larger nearMeshes count than %u!  Skipping some renderables for light %u!\n", SHADOW_NEARMESH_MAX, lightIdx); break; }
            }
            if (unlikely(nearbyMeshCount < 1)) continue;
            glUniform3f(3,lpos.x,lpos.y,lpos.z);
            shadowmapIndirectionList[lightIdx] = shadowMapIdx;
            #pragma GCC unroll 6
            for (u8 face = 0; face < 6; face++) {                                            
                glUniform1ui(2,face);
                glUniformMatrix4fv(1,1,GL_FALSE,(float*)lightViewProj[lightIdx][face]);
                glUniform1ui(7,shadowmapOffsetHead + (face * SHADOW_MAP_SIZE * SHADOW_MAP_SIZE));
                for (u16 j = 0; j < nearbyMeshCount; ++j) {
                    int i = shadows_nearMeshes[j].index;
                    Entity* e = &World.instances[i];
                    if (!SphereInFrustum(lightFrustumPlanes[lightIdx][face],World.position[i],e->shadRadius)) continue;
                    glUniform1ui(0,i);
                    u16 modelType = (instanceIsLODArray[i] || useDetail < 1u) && e->lodIndex < mdlsCnt ? e->lodIndex : e->modelIndex;
                    if (currentModelType != modelType || currentModelType == 0) { currentModelType = modelType; glBindVertexBuffer(0,vbos[modelType],0,VRT_ATT_SZ); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,tbos[modelType]); }
                    if (currentTexIndex != e->texIndex) { currentTexIndex = e->texIndex; glUniform1ui(6,e->texIndex); }
                    bool texIsTransparent = transparentTexture[e->texIndex];
                    if (currentIsTransparent != texIsTransparent) { currentIsTransparent = texIsTransparent; glUniform1ui(8,(u32)currentIsTransparent); }
                    glDrawElements(0x0004/*GL_TRIANGLES*/,modelTriangleCounts[currentModelType]*3,GL_UNSIGNED_SHORT,0); drawCalls++; shadDrawCalls++; vertsRendered += modelTriangleCounts[currentModelType] * 3;
                }
            }
            shadowmapOffsetHead += (SHADOW_MAP_SIZE * SHADOW_MAP_SIZE) * 6;
        }
        glViewport(0,0,Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight); glBindBuffer(GL_SSBO,shadowMapsIndirectionID); glBufferData(GL_SSBO,World.loadedLights * sizeof(u32),shadowmapIndirectionList,GL_DYNAMIC_DRAW);
    }
    shadowTime = get_time() - shadowStartTime;
}

DepthSort visibleInstances[INSTANCE_COUNT];
INLINE bool DetermineIfInstanceVisible(u16 i, bool otherCondition, bool skyVisible, V3 playerPos, float* distSqrd) {
    if (EntNotVisible(i,otherCondition)) return false; // must be transparent && transparents or neither
    Entity* e = &World.instances[i]; u16 instCellIdx = e->cellIndex; u16 entIdx = e->index;
    V3 delta = V3_AsubB(World.position[i],playerPos); *distSqrd = V3_dot(delta,delta);
    float radius = modelBounds[e->modelIndex] * 2.0f * vmax(vmax(World.scale[i].x,World.scale[i].y),World.scale[i].z);
    if (!SphereInFrustum(playerFrustumPlanes,World.position[i],radius) && (entIdx != 754 || !skyVisible) && i != editModeSelection) return false;
    if (IdxIsPortalBlockingDoor(entIdx)) { // Extra checks only needed for opaque portal blocking doors.
        bool inPVS = (gridCellStates[instCellIdx] & CELL_VISIBLE);
        if (!inPVS) {inPVS = NeighborhoodInPVS(e->cellX,e->cellZ,2u);} if (!inPVS) return false;
    } else {
        if (((gridCellStates[instCellIdx] & (CELL_VISIBLE | CELL_OPEN)) == CELL_OPEN) && (entIdx != 754 || !skyVisible)) return false;
        if (!(gridCellStates[instCellIdx] & CELL_OPEN) && *distSqrd >= 943.7184f && (entIdx != 754 || !skyVisible)) return false; // 30.72 * 30.72, 12 cells
    }
    if (World.instances[i].camView != 255) camViews[World.instances[i].camView].visible = true;
    return true;
}

float GetPainStatic() { return 0.0f; } // TODO: Hook into pain/health management and shield impact effect
Color GetPainStaticColor() { return (Color){1.0f,0.0f,0.0f,1.0f}; } // TODO: Hook staticColor up to red or blue for pain or shield impact.
__attribute__((pure)) i32 dsort(const void* a, const void* b) { float da = ((const DepthSort*)a)->depth; float db = ((const DepthSort*)b)->depth; return (db > da) - (db < da); }
__attribute__((pure)) i32 dsortInv(const void* a, const void* b) { float da = ((const DepthSort*)a)->depth; float db = ((const DepthSort*)b)->depth; return (da > db) - (da < db); }
void DrawEntity(Entity* e, u16 i, u16 constIndex, u16 tex, u16 curN, u16 curT, u16 curG, u16 curS, u16 curM, bool grayscaleEnabled) {
    u16 glow=e->glowIndex,norm=e->normIndex,spec=e->specIndex;
    if (Cheats.showPhys) {if (World.collider[i] == COLTYPE_BOX) {DrawBoxCollider(i);} else if (World.collider[i] == COLTYPE_SPH) {DrawSphereCollider(i);} else if (World.collider[i] == COLTYPE_CVX) {DrawMeshCollider(i);} else if (World.collider[i] == COLTYPE_MSH) {DrawMeshCollider(i);} else if (World.collider[i] == COLTYPE_CAP) {DrawCapsuleCollider(i);} DrawAngularVelocity(i);}
    glUniform1ui(17,tex==316?1u:0u); glUniform1ui(25,constIndex); glUniform1f(27,e->volume); glUniform1ui(13,(tex==36||tex==887) ? 1u : 0u);
    if (grayscaleEnabled) { float npcHeat = IdxIsNPC(constIndex) ? ((constIndex==419 || constIndex==422 || constIndex==424 || constIndex==429 || constIndex==430 || constIndex==431||constIndex==433||constIndex==437||constIndex==438||constIndex==441) ? 1.5f : 4.0f) : 0.0f; glUniform1f(9,npcHeat); }
    glUniform1ui(30,e->camView < camViewCount ? 1u : 0u);
    if(e->camView < camViewCount) { glActiveTexture(GL_TEXTURE6); glBindTexture(GL_TEXTURE_2D,camViewTextures[e->camView]); glUniform2ui(28,camViews[e->camView].width,camViews[e->camView].height); glUniform1i(29,6); }
    if((curN) != (norm) || norm==0) { curN=norm; glUniform1ui( 1,(u32)norm); }
    if((curT) != ( tex) ||  tex==0) { curT= tex; glUniform1ui(18,(u32)tex ); }
    if((curG) != (glow) || glow==0) { curG=glow; glUniform1ui(19,(u32)glow); }
    if((curS) != (spec) || spec==0) { curS=spec; glUniform1ui(20,(u32)spec); }
    curM=GetAndBindModel(i,curM); u32 vc=modelTriangleCounts[curM]*3; glDrawElements(0x0004,vc,GL_UNSIGNED_SHORT,0); drawCalls++; vertsRendered+=vc;
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
    mat4_lookat_from(view,&World.rotation[PLAYER1],World.position[PLAYER1]);
    mul_mat4(viewProj,rasterPerspectiveProjection,view);
    invViewRot[0]=view[0]; invViewRot[1]=view[4]; invViewRot[2]=view[8]; invViewRot[3]=view[1]; invViewRot[4]=view[5]; invViewRot[5]=view[9]; invViewRot[6]=view[2]; invViewRot[7]=view[6]; invViewRot[8]=view[10];
    mat4_inverse(viewProj,invViewProj);
}

static __attribute__((hot)) void Render(bool camView, u8 camViewIdx) {
    u16 swidth = camView ? camViews[camViewIdx].width : Sys_Settings.ScreenWidth, sheight = camView ? camViews[camViewIdx].height : Sys_Settings.ScreenHeight;
    float sfov = camView ? (float)camViews[camViewIdx].fov : (float)Sys_Settings.FOV;
    float snear = camView ? camViews[camViewIdx].near : NEAR_PLANE; float sfar = camView ? camViews[camViewIdx].far : World.farPlane[World.curLev];
    V3 playerPos = World.position[PLAYER1];
    float px=playerPos.x, py=playerPos.y, pz=playerPos.z, aspect3D=(float)swidth / (float)sheight;
    float view[16],viewProj[16],invViewRot[9],invViewProj[16];
    GetProjections(view,viewProj,invViewRot,invViewProj,sfov,aspect3D,snear,sfar);
    ExtractFrustumPlanes(viewProj,playerFrustumPlanes);
    glBindVertexArray(chunkVAO); // Common vao for RenderDynamicShadowmaps and Rasterized Geometry
    glEnable(GL_DEPTH_TEST);
    if (likely(Sys_Settings.Shadows > 0u)) RenderShadowmaps();
    UpdateLights(); // This is where the voxels get updated!
    for (int i=0;i<LIGHT_COUNT;++i) flag_set(&World.lights[i].lflags,LDIRTY,false);
    glViewport(0,0,swidth,sheight);
    ClearAll();
    glBindFramebuffer(GL_FRAMEBUFFER,gBufferFBO);
    glEnable(GL_CULL_FACE); glDisable(GL_BLEND); // Opaques
    u16 visibleCount = 0, currentTexIndex = 0, currentNormIndex = 0, currentGlowIndex = 0, currentSpecIndex = 0, currentModelType = 0, opaqueCount = 0;
    bool skyVisible = (gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX); float distSqrd = sfar * sfar;
    DepthSort tmpTransparent[1024]; u16 tcnt = 0;
    for (u16 i = INSTS_1ST_IDX; i < World.instCount; ++i) { // Determine base visibility
        if (!DetermineIfInstanceVisible(i,false,skyVisible,playerPos,&distSqrd)) continue;
        if (transparentTexture[World.instances[i].texIndex]) { if(tcnt>1023){continue;} tmpTransparent[tcnt].index = i; tmpTransparent[tcnt].depth = distSqrd; tcnt++; }
        else { visibleInstances[opaqueCount].index = i; visibleInstances[opaqueCount].depth = distSqrd; opaqueCount++; }
    }
    mcpy(visibleInstances + opaqueCount,tmpTransparent,tcnt * sizeof(DepthSort));
    visibleCount = opaqueCount + tcnt;
    glUseProgram(depthPrepassSP); // Depth Prepass - Eliminates some overdraw for ~6.1% performance improvement in spite of added draw calls
    glUniformMatrix4fv(2,1,0,viewProj);
    glEnable(GL_DEPTH_TEST); glColorMask(0,0,0,0); glDepthMask(1); glDepthFunc(0x0201/*GL_LESS*/); glDisable(GL_BLEND);
    if (opaqueCount > 1) qsort_new(visibleInstances,opaqueCount,sizeof(DepthSort),dsortInv);
    if (tcnt > 1) qsort_new(visibleInstances + opaqueCount,tcnt,sizeof(DepthSort),dsort);
    for (u16 visibleIndex = 0; visibleIndex < visibleCount; ++visibleIndex) {
        u16 i = visibleInstances[visibleIndex].index;
        Entity* e = &World.instances[i]; u16 tex = e->texIndex;
        if (transparentTexture[tex]) { glEnable(GL_CULL_FACE); glEnable(GL_BLEND); } // Transparents (with sort)
        else if (doubleSidedTexture[tex] || World.scale[i].x < 0.0f || World.scale[i].y < 0.0f || World.scale[i].z < 0.0f) { glDisable(GL_CULL_FACE); glEnable(GL_BLEND); } // Doublesided
        else { glEnable(GL_CULL_FACE); glDisable(GL_BLEND); } // Opaque
        currentModelType = GetAndBindModel(i,currentModelType);
        glUniform1ui(3,(u32)tex);
        u32 vertCount = modelTriangleCounts[currentModelType] * 3;
        glDrawElements(0x0004/*GL_TRIANGLES*/,vertCount,GL_UNSIGNED_SHORT,0); drawCalls++; vertsRendered += vertCount;
    }
    glUseProgram(chunkSP); /*Main Pass*/ glUniformMatrix4fv(2,1,0,viewProj); glUniform1ui(25,0u); // default constIndex
    bool grayscaleEnabled = ModRequestsGrayscale(), refOn = Sys_Settings.Reflections;              glUniform1ui(26,(u32)grayscaleEnabled);
    float fogActual = World.fogColor[World.curLev].a + (float)(World.fogFac / 255u); // Alpha is base density for level.
    glUniform3f(12,World.fogColor[World.curLev].r * fogActual,World.fogColor[World.curLev].g * fogActual,World.fogColor[World.curLev].b * fogActual); // Fog Color(which is density)
    glUniform1ui(14,refOn); glUniform1ui(15,Sys_Settings.Shadows); glUniform2f(8,World.worldMin_x[World.curLev],World.worldMin_z[World.curLev]); 
    glUniform3f(10,playerPos.x,playerPos.y,playerPos.z);
    glColorMask(1,1,1,1);   glDepthMask(0);                        glDepthFunc(0x0203/*GL_LEQUAL*/); // Opaque Pass
    visibleCount = currentTexIndex = currentNormIndex = currentGlowIndex = currentSpecIndex = currentModelType = 0;
    glUniform1f(9,0.0f); // Reset heat for infrared vision
    for (u16 visibleIndex = 0; visibleIndex < opaqueCount; ++visibleIndex) { // Opaques (already front-to-back)
        u16 i = visibleInstances[visibleIndex].index;
        Entity* e = &World.instances[i]; u16 tex = e->texIndex; u32 constIndex = e->index;
        if (transparentTexture[tex]) continue;
        else if (doubleSidedTexture[tex] || World.scale[i].x < 0.0f || World.scale[i].y < 0.0f || World.scale[i].z < 0.0f) { glDisable(GL_CULL_FACE); glEnable(GL_BLEND); } // Doublesided (either)
        else { glEnable(GL_CULL_FACE); glDisable(GL_BLEND); } // Opaque
        DrawEntity(e,i,constIndex,tex,currentNormIndex,currentTexIndex,currentGlowIndex,currentSpecIndex,currentModelType,grayscaleEnabled);
    }
    glDepthMask(1); currentTexIndex = currentNormIndex = currentGlowIndex = currentSpecIndex = currentModelType = 0; // Transparents Pass
    for (u16 visibleIndex = opaqueCount; visibleIndex < (opaqueCount + tcnt); ++visibleIndex) {
        u16 i = visibleInstances[visibleIndex].index;
        Entity* e = &World.instances[i]; u16 tex = e->texIndex; u32 constIndex = e->index;
        if (transparentTexture[tex]) { glEnable(GL_CULL_FACE); glEnable(GL_BLEND); } // Transparents (with sort)
        else if (doubleSidedTexture[tex] || World.scale[i].x < 0.0f || World.scale[i].y < 0.0f || World.scale[i].z < 0.0f) { glDisable(GL_CULL_FACE); glEnable(GL_BLEND); } // Doublesided (either)
        else continue; // Opaque
        if ((constIndex >= 561 && constIndex <= 565) || (constIndex >= 568 && constIndex <= 573)) glDepthFunc(0x0202/*GL_EQUAL*/); // Cutouts
        else glDepthFunc(0x0203/*GL_LEQUAL*/); // Actual alphas
        DrawEntity(e,i,constIndex,tex,currentNormIndex,currentTexIndex,currentGlowIndex,currentSpecIndex,currentModelType,grayscaleEnabled);
    }
    V3 camUp = quat_rot_v3(World.rotation[PLAYER1],(V3){0,1,0});
    V3 camRight = quat_rot_v3(World.rotation[PLAYER1],(V3){1,0,0});
    PSys_Render(viewProj,playerPos,camUp,camRight,NEAR_PLANE,World.farPlane[World.curLev],inputDepthID); // Particles render
    if (camView) {
        glBindFramebuffer(0x8CA8/*GL_READ_FRAMEBUFFER*/,gBufferFBO);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glBindTexture(GL_TEXTURE_2D,camViewTextures[camViewIdx]);
        glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0,0,0,swidth,sheight);
        glBindTexture(GL_TEXTURE_2D,0);
        return; // After copying render result, skip SSR and composite for camviews <<<<<<<<<<<<< CAM VIEW BARRIER
    }
    if (unlikely(World.debugLineVertCount > 1)) DrawDebugLines(viewProj); // Draw Debug Lines
    glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D,inputDepthID);
    if (likely(refOn > 0u)) { // Screen Space Reflections
        glUseProgram(ssrSP); glUniform3f(3,playerPos.x,playerPos.y,playerPos.z); glUniform1i(5,3);
        glUniformMatrix4fv(6,1,0,invViewProj);     glUniformMatrix4fv(4,1,GL_FALSE,viewProj);
        u32 groupX_ssr = ((Sys_Settings.ScreenWidth / Sys_Settings.SSR_RES) + 31) / 32, groupY_ssr = ((Sys_Settings.ScreenHeight / Sys_Settings.SSR_RES) + 31) / 32;
        glDispatchCompute(groupX_ssr,groupY_ssr,1);
    }
    glBindFramebuffer(GL_FRAMEBUFFER,uiFBO); glViewport(0,0,1366,768);
    glDisable(GL_CULL_FACE);
    World.last_time = RenderUI();
    if ((World.inventoryMode && !Cheats.noHUD) || World.menuActive || World.paused) RenderUIImage((i16)(World.cursorPosition_x) - 20,(i16)(World.cursorPosition_y) - 20,40,40,GetCursorTexture());
    else if (!Cheats.noHUD) RenderUIImage(663,364,40,40,GetCursorTexture()); // Centered on UI fixed resolution 1366x768 FBO
    glBindFramebuffer(GL_FRAMEBUFFER,0); glViewport(0,0,swidth,sheight); // Restore normal output size for final composite blit
    glUseProgram(imageBlitSP); glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D,inputImageID); glUniform1i(4,4); // outputImage texture sampler2D, don't remember why when active texture is texture 0. meh.... oh maybe to not read and write same binding?
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D,inputUIID); glUniform1i(31,1); glUniform1i(32,3); glUniformMatrix4fv(33,1,0,invViewProj);
    double berserkTimeRemainingNormalized = World.invP1.berserkFinishedTime > 0.0001 ? (World.invP1.berserkFinishedTime - World.pauseRelativeTime) / BERSERK_TIME : 0.0;
    if (World.invP1.berserkFinishedTime < World.pauseRelativeTime && World.invP1.berserkFinishedTime > 0.0001) World.invP1.berserkFinishedTime = berserkTimeRemainingNormalized = 0.0;
    glUniform1ui(5,refOn); glUniform1ui(6,Sys_Settings.FXAA); glUniform1f(14,Sys_Settings.FOV); glUniform1f(16,aspect3D); glUniform1ui(22,Sys_Settings.Shadows); glUniform1f(9,(float)berserkTimeRemainingNormalized); glUniform1f(10,berserkSeedTime); glUniform1ui(11,Sys_Settings.Brightness);
    glUniform3f(12,deg2rad(World.cam_yaw),deg2rad(World.cam_pitch),deg2rad(World.cam_roll)); glUniform3f(13,px,py,pz); glUniform1f(15,(float)World.pauseRelativeTime * 0.1f); glUniform1ui(17,(gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX) || World.curLev == LEVEL_CYBERSPACE);
    glUniform1ui(18,(gridCellStates[playerCellIdx] & CELL_SEES_SUN) && World.curLev != LEVEL_CYBERSPACE); glUniform1ui(19,((World.curLev >= 10 && World.curLev < LEVEL_CYBERSPACE) ? 1u : 0u) && (gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX));
    u32 shieldOnType = 0u; // No shield green tint.
    if (World.instances[WORLD].ioflags & Q_SHIELD_ACTIVATED) { if (World.curLev == 6 || World.curLev == 7) {shieldOnType=2u;/*Shielding only below, levels 6+*/ } else if (World.curLev <= 5) {shieldOnType=1u;/*Shielding everywhere*/} }
    glUniform1ui(20,shieldOnType);
    Color painStaticColor = GetPainStaticColor(); glUniform3f(23,painStaticColor.r,painStaticColor.g,painStaticColor.b);
    glUniformMatrix4fv(24,1,0,viewProj);          glUniformMatrix3fv(25,1,0,invViewRot);        glUniform1i(27,0); // Texture 0 for the rendered geometry color buffer
    glUniform1f(28,GetPainStatic());              glUniform1ui(29,(u32)ModRequestsGrayscale()); glBindVertexArray(quadVAO); glDisable(GL_DEPTH_TEST);
    glDrawArrays(0x0006/*GL_TRIANGLE_FAN*/,0,4); drawCalls++; vertsRendered += 4;
    if ((World.last_time - World.lastFrameSecCountTime) >= 1.00) { // Update Diagnostic Poll
        World.lastFrameSecCountTime = World.last_time; globalframesPerLastSecond = globalframe - World.lastFrameSecCount;
        World.lastFrameSecCount = globalframe;
    }
}

void RenderCameraViews() { // Render in-world camera views.  Pops player position to elsewhere, renders to tiny fbo, pops player back.
    if (unlikely(World.paused || World.menuActive || camViewCount == 0 || World.curLev >= LEVEL_CYBERSPACE)) return;
    
    V3 tempPlayerPos = World.position[PLAYER1]; Quaternion tempPlayerRot = World.rotation[PLAYER1];
    for (int cm=0;cm<camViewCount;++cm) {
        if (camViews[cm].finished < World.pauseRelativeTime && camViews[cm].visible) { camViews[cm].finished = World.pauseRelativeTime + 0.5f; World.position[PLAYER1] = camViews[cm].position; World.rotation[PLAYER1] = camViews[cm].rotation; CullCore(); Render(true/*camview*/,cm); }
    }

    World.position[PLAYER1] = tempPlayerPos; World.rotation[PLAYER1] = tempPlayerRot; // Restore player for normal render.
}

void UpdateInstanceMatrix4x4s() {
    if (unlikely(World.paused || World.menuActive)) return;

    i32 dirtyMin = -1, dirtyMax = -1;
    for (u32 i = INSTS_1ST_IDX; i < World.instCount; i++) {        
        float x=World.rotation[i].x, y=World.rotation[i].y, z=World.rotation[i].z, w=World.rotation[i].w;
        float x2=x*x, y2=y*y, z2=z*z, xy=x*y, xz=x*z, yz=y*z, wx=w*x, wy=w*y, wz=w*z;
        float sclx=World.scale[i].x, scly=World.scale[i].y, sclz=World.scale[i].z; u32 m = i*16;
        modelMatrices[m+0]=(1.0f-2.0f*(y2+z2))*sclx; modelMatrices[m+1]=(2.0f*(xy+wz))*sclx;      modelMatrices[m+2]=(2.0f*(xz-wy))*sclx; // 3,7,11 == 0.0f, no need to set all the time.
        modelMatrices[m+4]=(2.0f*(xy-wz))*scly;      modelMatrices[m+5]=(1.0f-2.0f*(x2+z2))*scly; modelMatrices[m+6]=(2.0f*(yz+wx))*scly;
        modelMatrices[m+8]=(2.0f*(xz+wy))*sclz;      modelMatrices[m+9]=(2.0f*(yz-wx))*sclz;      modelMatrices[m+10]=(1.0f-2.0f*(x2+y2))*sclz;
        modelMatrices[m+12]=World.position[i].x;     modelMatrices[m+13]=World.position[i].y;     modelMatrices[m+14]=World.position[i].z;      modelMatrices[m+15]=1.0f;
        if (dirtyMin < 0) {dirtyMin = (i32)i;} dirtyMax = (i32)i;
    }
    
    if (dirtyMin < 0) return;
    
    glBindBuffer(GL_SSBO,matricesBufferID);
    u32 offsetFloats = (u32)dirtyMin * 16; u32 countFloats  = ((u32)dirtyMax - (u32)dirtyMin + 1) * 16;
    glBufferSubData(GL_SSBO,offsetFloats * sizeof(float),countFloats * sizeof(float),modelMatrices + offsetFloats);
}
// Init && Main
__attribute__((cold)) void NewGame() { // Reset World States
    DualLog("Loading new game...\n");
    RenderLoading(100,"Loading new game...");
    World.menuActive = World.paused = enteringPlayerName = fovSliderActive = gammaSliderActive = masterVolumeSliderActive = musicVolumeSliderActive = messageVolumeSliderActive = sfxVolumeSliderActive = returnToPause = false;
    for (int i=0;i<World.numLevels;++i) { World.worldMin_x[i] = levMins[i].x; World.worldMin_z[i] = levMins[i].y; World.voxMinCtrX[i] = World.worldMin_x[i] + VOXEL_HALF; World.voxMinCtrZ[i] = World.worldMin_z[i] + VOXEL_HALF; World.farPlane[i] = lFars[i]; World.fogColor[i] = fogLUT[i]; World.fogColor[i].a *= 3.8f; }
    SetLevelPointers(0);
    World.curLev = 0;
    World.mass[0] = 0.0f; World.dynamicFriction[0] = 0.4f; World.collider[0]=COLTYPE_NONE; // Static proxy just uses world.
    currentMenuItem = currentMenuTab = 0; currentMenuPage = Mpg_FrontPage;
    World.current_time = World.pauseRelativeTime = World.last_physics_time = World.pauseRelativeTime = World.last_physics_time=0.0; World.deltaTime=0.0166666666f;
    mset(World.instances,0,3 * sizeof(Entity)); // Blank out player entities
    World.instances[PLAYER1].index = 767;
    World.layer[PLAYER1] = L_Player;
    World.scale[PLAYER1] = (V3){1.0f,1.0f,1.0f};
    World.rotation[PLAYER1] = (Quaternion){0.0f,0.7071f,0.0f,0.7071f}; // 90deg rotation CW about Y axis as viewed from the top looking down onto player
    World.instances[PLAYER1].entflags = EF_ACTIVE|EF_RIGIDBODY;
    World.collider[PLAYER1] = COLTYPE_CAP; World.colliderCenter[PLAYER1].y = -PLAYER_CAM_OFFSET_Y; World.colliderSize[PLAYER1] = (V3){PLAYER_RADIUS,PLAYER_HEIGHT,COLLIDER_CAPSULE_DIRECTION_Y_F}; // Radius, Overall height including end radii (Unity convention, blech), Direction, 1.0 == Y-Axis
    World.mass[PLAYER1] = 1.0f; World.velocity[PLAYER1] = (V3){0.0f,0.0f,0.0f};
    World.cam_yaw = 90.0f; World.cam_pitch = World.cam_roll = World.invP1.leanTarget = World.invP1.leanShift = 0.0f; World.gravity[PLAYER1] = 1.0f; World.dynamicFriction[PLAYER1] = 0.6f; World.staticFriction[PLAYER1] = 0.8f; 
    World.instances[PLAYER1].health = 200.0f; World.instances[PLAYER1].noiseFinished = World.pauseRelativeTime;
    World.invP1.energy = 54.0f; World.invP1.energyDrainTickFinished = World.pauseRelativeTime + 0.1 + (double)random_range(0.5f, 1.0f);  World.invP1.maxEnergy = 255.0f;
    World.invP1.hardwareInvReferenceIndex[0]  = 21; World.invP1.hardwareInvReferenceIndex[1]  = 22; World.invP1.hardwareInvReferenceIndex[2]  = 23; World.invP1.hardwareInvReferenceIndex[3]  = 24; World.invP1.hardwareInvReferenceIndex[4]  = 25; World.invP1.hardwareInvReferenceIndex[5]  = 26;
    World.invP1.hardwareInvReferenceIndex[6]  = 27; World.invP1.hardwareInvReferenceIndex[7]  = 28; World.invP1.hardwareInvReferenceIndex[8]  = 29; World.invP1.hardwareInvReferenceIndex[9]  = 30; World.invP1.hardwareInvReferenceIndex[10] = 31; World.invP1.hardwareInvReferenceIndex[11] = 32;
    World.invP1.hardwareInvReferenceIndex[12] =  0; World.invP1.hardwareInvReferenceIndex[13] =  0; World.invP1.generalInventoryIndexRef[0] = 81; // Hardcoded lookup indices into the Const main table.
    for (int i=1;i<HW_COUNT;i++) World.invP1.generalInventoryIndexRef[i] = -1; // Skips 0th index on purpose as it always holds access cards "item".
    for (int i=0;i<HW_COUNT;++i) World.invP1.hardwareVersion[i] = World.invP1.hardwareVersionSetting[i] = 0;
    World.invP1.nitroTimeSetting = NITRO_DEFAULT_TIME;
    World.invP1.earthShakerTimeSetting = EARTH_SHAKER_DEFAULT_TIME;
    World.invP1.lastAddedIndex = World.invP1.currentCyberItem = World.invP1.globalLookupIndex = -1;
    World.invP1.hasNewEmail = World.invP1.hasNewNotes = true;
    World.invP1.isPulserNotDrill = true;
    for (int i=0;i<7;++i) World.invP1.weaponInventoryIndices[i] = World.invP1.weaponInventoryAmmoIndices[i] = -1;
    World.invP1.sparqSetting = 50.0f; World.invP1.ionSetting = 100.0f; World.invP1.blasterSetting = 15.0f; World.invP1.plasmaSetting = 40.0f; World.invP1.stungunSetting = 20.0f; World.invP1.justFired = (World.pauseRelativeTime - 31.0); // Set >30s before pauseRelativeTime to not immediately play action music.
    World.invP1.resetAfterDeathTime = 0.5; World.invP1.painSoundFinished = World.invP1.radSoundFinished = World.invP1.radFXFinished = World.pauseRelativeTime; World.Sys_UI.lastMultiMediaTabOpened = MM_EMAIL_TABLE;
    World.Sys_UI.logFinished = World.pauseRelativeTime; World.Sys_UI.tickFinished = World.Sys_UI.centerTabsTickFinished = World.current_time + 0.1 + (double)random_range(0.0f,1.0f); World.Sys_UI.blinkFinished = 1.0 + World.pauseRelativeTime; World.Sys_UI.beepFinished = 3.0 + World.pauseRelativeTime;
    World.invP1.mediFinishedTime = World.invP1.reflexFinishedTime = World.invP1.sightFinishedTime = -1.0; World.invP1.berserkIncrement = World.invP1.patchActive = 0; World.invP1.staminupActive = World.geniusActive = false; World.timeScale = DEFAULT_TIME_SCALE; 
    World.cam_yaw = 90.0f; World.cam_pitch = 0.0f; World.cam_roll = 0.0f; World.inventoryMode = Sys_Settings.NoShootMode;
    World.gameFinished = World.creditsActive = World.decoyActive = false; World.damageDealt = World.damageReceived = 0.0f;
    World.ressurections = World.deaths = World.kills = World.cyberkills = 0u; World.shotsFired = World.grenadesThrown = World.savesScummed = 0U; World.creditsPageIndex = 0u;
    for (int i=0;i<14;++i) World.levelSecurity[i] = 100u;
    ResetInput();
    World.currentMouse_dx = World.currentMouse_dy = 0; last_mouse_x = last_mouse_y = 0; ignore_next_mouse_delta = true;
    Sys_Input.lastUse = Sys_Input.isCapsLockOn = false; // As far as we're concerned, don't worry about OS capslock actual state.
    for (u8 lev = 1; lev < World.numLevels; ++lev) CopyPlayerState(0,lev);
    LoadAllLevels();
    LoadLevel(World.startLevel,(V3){10.52f,-43.792f + 0.84f,20.2908f}); // Must be after entities!  Fast pointer swap to startLevel. Start Actual: Puts player on Medical Level in actual game start position.
    GenerateConvexAdjacencyLists();
    World.lev1SecCode = random_range_u8(0u,9u); World.lev2SecCode = random_range_u8(0u,9u);
    World.lev3SecCode = random_range_u8(0u,9u); World.lev4SecCode = random_range_u8(0u,9u);
    World.lev5SecCode = random_range_u8(0u,9u); World.lev6SecCode = random_range_u8(0u,9u); // Must do rand's repeatedly to prevent these all being the same number.
    firstFrameMouselook = true; // Prevent jumps after cursor is centered once menu turned off.
}

// Init
void GoIntoGame() { NewGame(); PlayGameMusic(); DualLog("Player named \"%s\" started the game!\n", World.playerName); }
void InitalizeEnvironment() {
    double game_start_time = get_time(); random_range_rng = (u32)game_start_time; // Seed global rand uniquely with time since system boot.
    console_log_file = OS_OpenWriteonly("./voxen.log"); // Initialize log system for all prints to go to both stdout and voxen.log file
    DebugRAM("program start");
    DualLog("Voxen, the Voxel Lit Open Source Game Engine by W. Josiah Jack, MIT-0 licensed\nEntity size: %u\n",sizeof(Entity));
    SetLevelPointers(0);
    WindowInit(); threadCnt = clamp(OS_GetNumThreads(),1,32); globalframe=0,World.menuActive=true,World.screenshotTimeout=1.0,World.creditsPageIndex=1,World.diffCbt=World.diffCyb=World.diffPuz=World.diffMis=2,World.deaths=0,World.cursorPosition_x=680,World.cursorPosition_y=384;
    DualLog("Loading game definition...");
    World.numLevels = MAX_LEVELS; World.startLevel = 1/*medical*/; DualLog(" Citadel:: num levels: %d, start level: %d\n",World.numLevels,World.startLevel);
    LoadConfig(); // Get settings before setting window size.
    window = VCreateWindow(Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight);
    CenterWindowOnMonitor();
    SetGLContext_GetFunctionPointers();
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); ((WinSyswindow*)window)->context.swapBuffers(((WinSyswindow*)window)); // Black out the window as early as possible for better presentation.
    i32 major=0,minor=0; glGetIntegerv(0x821B/*GL_MAJOR_VERSION*/,&major); glGetIntegerv(0x821C/*GL_MINOR_VERSION*/,&minor); if (major < 4 || (major == 4 && minor < 3)) { DualLogError("Need OpenGL >= 4.3, got %d.%d\n",major,minor); OS_Exit(1); }
    glFrontFace(0x0901/*GL_CCW*/); // Set triangle winding order
    glBlendFuncSeparate(0x0302/*GL_SRC_ALPHA*/, 0x0303/*GL_ONE_MINUS_SRC_ALPHA*/, 1, 0x0303/*GL_ONE_MINUS_SRC_ALPHA*/);
    CompileShaders();
    PSys_Init();
    u32 tvaos[4],tvbos[4]; glGenVertexArrays(4,tvaos); glGenBuffers(4,tvbos);
    quadVAO = tvaos[0]; chunkVAO = tvaos[1]; textVAO = tvaos[2]; debugLinesVAO = tvaos[3];
    quadVBO = tvbos[0]; chunkVBO = tvbos[1]; textVBO = tvbos[2]; debugLinesVBO = tvbos[3]; 
    float quadBlit_vertices[] = {1.0f,-1.0f,1.0f,0.0f, 1.0f,1.0f,1.0f,1.0f, -1.0f,1.0f,0.0f,1.0f, -1.0f,-1.0f,0.0f,0.0f}; // 4 verts, 4 floats each x,y,u,v
    glBindVertexArray(quadVAO); glBindBuffer(GL_ARRAY_BUFFER,quadVBO); glBufferData(GL_ARRAY_BUFFER,sizeof(quadBlit_vertices),quadBlit_vertices,GL_STATIC_DRAW);
    glVertexAttribFormat(0,2,GL_FLOAT,GL_FALSE,0);                 glVertexAttribBinding(0,0); glEnableVertexAttribArray(0); // pos xy float @ offset 0
    glVertexAttribFormat(1,2,GL_FLOAT,GL_FALSE,2 * sizeof(float)); glVertexAttribBinding(1,0); glEnableVertexAttribArray(1); // uv (s,t)
    glBindVertexBuffer(0,quadVBO,0,4 * sizeof(float));
    glBindVertexArray(chunkVAO);
    glVertexAttribFormat(0,3,0x140B/*GL_HALF_FLOAT*/,GL_FALSE,0);  glVertexAttribBinding(0,0); glEnableVertexAttribArray(0); // pos xyz half-float @ offset 0
    glVertexAttribFormat(1,3,0x140B/*GL_HALF_FLOAT*/,GL_FALSE,6);  glVertexAttribBinding(1,0); glEnableVertexAttribArray(1); // normal xyz float   @ offset 6  (after 3×2 bytes)
    glVertexAttribFormat(2,2,0x140B/*GL_HALF_FLOAT*/,GL_FALSE,12); glVertexAttribBinding(2,0); glEnableVertexAttribArray(2); // uv st float
    glBindVertexArray(textVAO);
    glVertexAttribFormat(0,3,GL_FLOAT,GL_FALSE,0);                 glVertexAttribBinding(0,0); glEnableVertexAttribArray(0); // pos (x,y,z) 4 floats per vertex, stride = 4*sizeof(float)
    glVertexAttribFormat(1,2,GL_FLOAT,GL_FALSE,3 * sizeof(float)); glVertexAttribBinding(1,0); glEnableVertexAttribArray(1); // uv (s,t)
    glBindVertexBuffer(0, textVBO,0,5 * sizeof(float));
    glBindVertexArray(debugLinesVAO); glBindBuffer(GL_ARRAY_BUFFER,debugLinesVBO); glBufferData(GL_ARRAY_BUFFER,MAX_WIRELINE_VRTS * 2 * sizeof(DebugLineVertex),NULL,GL_DYNAMIC_DRAW);
    glVertexAttribFormat(0,3,GL_FLOAT,GL_FALSE,__builtin_offsetof(DebugLineVertex,x)); glVertexAttribBinding(0,0); glEnableVertexAttribArray(0);
    glVertexAttribFormat(1,4,GL_FLOAT,GL_FALSE,__builtin_offsetof(DebugLineVertex,r)); glVertexAttribBinding(1,0); glEnableVertexAttribArray(1);
    glBindVertexBuffer(0,debugLinesVBO,0,sizeof(DebugLineVertex));
    InitFontAtlasses();
    GenerateAndBindTexture(&inputUIID,GL_RGBA8,1366,768,GL_RGBA,GL_UNSIGNED_BYTE,0x2600/*GL_NEAREST*/,NULL); // UI Fixed Size Raster
    glGenFramebuffers(1,&uiFBO);
    glBindFramebuffer(GL_FRAMEBUFFER,uiFBO);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D,inputUIID); glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,inputUIID,0);
    u32 drawBuffersUI[] = {GL_COLOR_ATTACHMENT0}; glDrawBuffers(1,drawBuffersUI);
    u32 uistatus = glCheckFramebufferStatus(GL_FRAMEBUFFER); if (uistatus != 0x8CD5/*GL_FRAMEBUFFER_COMPLETE*/) DualLogError("UI Framebuffer incomplete: Error code %d\n",uistatus);
    glBindImageTexture(0,inputUIID,0,GL_FALSE,0,GL_READ_WRITE,GL_RGBA8);/* UI Rendered Color*/ glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,inputUIID,0);
    RenderLoading(40,"Loading...");
    float* m = shadowmapsPerspectiveProjection; float lightRangeMax=15.36f; float viewRange=(lightRangeMax - NEAR_PLANE);
    m[0]=1.0f; m[1]=0.0f; m[2]=0.0f; m[3]=0.0f; m[4]=0.0f; m[5]=1.0f; m[6]=0.0f; m[7]=0.0f; m[8]=0.0f; m[9]=0.0f; m[10]=-(lightRangeMax + NEAR_PLANE) / viewRange; m[11]=-1.0f; m[12]=0.0f; m[13]=0.0f; m[14]=-2.0f * lightRangeMax * NEAR_PLANE / viewRange; m[15]=0.0f;
    InitSCFTables();
    InitAudio();
    ModEDefsInitAfterLoad(); // Set the values for all 768 entity definitions, a doozy of a function.
    glGenFramebuffers(1,&gBufferFBO);
    ChangeFullScreenWindowed(); SetSkyRotateSpeed(); SetVSync(); LoadTextForLanguage(Sys_Settings.Language); LoadLogTextForLanguage(Sys_Settings.Language);
    glBindFramebuffer(GL_FRAMEBUFFER,gBufferFBO); u32 drawBuffers[] = {GL_COLOR_ATTACHMENT0,GL_COLOR_ATTACHMENT1,GL_COLOR_ATTACHMENT2}; glDrawBuffers(3,drawBuffers);
    u32 status = glCheckFramebufferStatus(GL_FRAMEBUFFER); if (status != 0x8CD5/*GL_FRAMEBUFFER_COMPLETE*/) DualLogError("Framebuffer incomplete: Error code %d\n",status);
    float mat[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1}; u32 MAX_LIGHTS_PER_VOXEL = 32;
    mcpy(&modelMatrices[0],mat,16 * sizeof(float)); // Null instance matrix used for UI
    matricesBufferID = MakeSSBO(&matricesBufferID, 1,INSTANCE_COUNT * 16 * sizeof(float),modelMatrices,GL_STATIC_DRAW);     cellVisibleDataID= MakeSSBO(&cellVisibleDataID,7,ARRSIZE * sizeof(u32),NULL,GL_STATIC_DRAW);
    voxListCntsID    = MakeSSBO(&voxListCntsID,    2,VOXEL_COUNT * sizeof(u32),NULL,GL_STATIC_DRAW);                        texPalID         = MakeSSBO(&texPalID,         8,MAX_UNIQUE_COLORS * sizeof(u32),NULL,GL_STATIC_DRAW);
    voxelLightListsID= MakeSSBO(&voxelLightListsID,3,VOXEL_COUNT * MAX_LIGHTS_PER_VOXEL * sizeof(u32),NULL,GL_STATIC_DRAW); texPalOfsID      = MakeSSBO(&texPalOfsID,      9,MAX_TXRS * sizeof(u32),NULL,GL_STATIC_DRAW);
    lightsID         = MakeSSBO(&lightsID,         4,LIGHT_COUNT * sizeof(Light),NULL,GL_STATIC_DRAW);                      colorBufferID    = MakeSSBO(&colorBufferID,   12,MAX_TOTAL_PIXELS * sizeof(u8),NULL,GL_STATIC_DRAW);
    if (Sys_Settings.Shadows) CreateShadowBuffers();                    /*5,6*/                                             textureOffsetsID = MakeSSBO(&textureOffsetsID,14,MAX_TXRS * sizeof(u32),NULL,GL_STATIC_DRAW);
                                                                                                                            textureSizesID   = MakeSSBO(&textureSizesID,  15,MAX_TXRS * 2 * sizeof(i32),NULL, GL_STATIC_DRAW);
    glUseProgram(shadowmapsSP);  glUniform1ui( 9,SHADOW_MAP_SIZE); glUseProgram(shadowmapsClearSP);           glUniform1ui(1,SHADOW_MAP_SIZE);
    glUseProgram(chunkSP);       glUniform1ui(21,SHADOW_MAP_SIZE); glUniform1f(22,(float)SHADOW_MAP_SIZE);    glUniform1ui(23,LIGHT_COUNT); glUniform1ui(24,(u32)MAX_LIGHTS_PER_VOXEL); glUniform1ui(11,SHADOW_MAP_SIZE*SHADOW_MAP_SIZE);
    glUseProgram(voxelUpdateSP); glUniform1ui( 4,SHADOW_MAP_SIZE); glUniform1ui(6,(u32)MAX_LIGHTS_PER_VOXEL);
    RenderLoading(100,"Loading textures..."); LoadTextures();
    RenderLoading(92,"Loading models...");    LoadModels();
    if (World.introNotPlayed) {} // TODO: Play intro
    World.absoluteTime = World.current_time = get_time();
    World.pauseRelativeTime = World.last_physics_time = 0.0;
    NewGame();
    PlayMenuMusic(); World.menuActive = true; currentMenuPage = Mpg_FrontPage; // Comment out for immediate testing
    DebugRAM("InitializeEnvironment end"); DualLog("Game Initialized in %f secs\n",get_time() - game_start_time);
}

i32 main() {
    InitalizeEnvironment();
    while(1) {
        if (queuedLevelToLoad != 255u) { LoadLevel(queuedLevelToLoad,queuedLevelPos); queuedLevelToLoad = 255u; continue; }
        double curtime = get_time(); World.deltaTime=World.current_time < 0.001f ? 0.000f : vmax(curtime - World.current_time,0.0); World.absoluteTime+=World.deltaTime; World.current_time=curtime;
        if (!World.paused && !World.menuActive) { if (World.pauseRelativeTime < 0.001f) {World.pauseRelativeTime = World.last_physics_time = curtime;} World.pauseRelativeTime += World.deltaTime; }
        InputProcessing(); // Before anims and physics to allow them to respond immediately.
        UpdateAnims();     // Before physics to allow model swap out to affect physics state immediately.  Before rendering to affect shadowmaps immediately.
        PSys_Update();     // Before physics to allow for particles to interact with static world.
        if (!World.paused && !World.menuActive) {
            double physStart = get_time(); float dt = vclamp((float)(World.pauseRelativeTime - World.last_physics_time),0.0005f,0.1f); World.last_physics_time = World.pauseRelativeTime; World.dt = dt;
            Physics(dt);
            physTime = get_time() - physStart;
        } else physTime = 0.0;
        ModUpdate(); // After physics so mod/gamecode can modify velocities before next frame.
        if (!World.paused && !World.menuActive) {MixAmbs();}
        UpdateMusic();
        drawCalls = uiDrawCalls = shadDrawCalls = vertsRendered = 0; RenderCameraViews();
        if (likely(!World.paused && !World.menuActive)) CullCore();
        UpdateInstanceMatrix4x4s();
        Render(false/*!camview*/,0u);
        if (ScrshotPressed() && World.current_time > World.screenshotTimeout) Screenshot();
        ResetInput(); globalframe++; World.cpuTime = get_time() - World.current_time; // Measure time over everything this frame before GPU swap buffers for diagnostic text.
        ((WinSyswindow*)window)->context.swapBuffers(((WinSyswindow*)window)); // Present frame (almost always waiting for GPU since GPU bound).
        CHECK_GL_ERROR(); // Lone catch for inadvertent issues.
        { static const u32 dbgFrm[] = {4,100,200,500,1000}; static const char* dbgLbl[] = {"frame 4","frame 100","frame 200","frame 500","frame 1000"}; for (int d=0;d<5;d++) if (globalframe == dbgFrm[d]) {DebugRAM(dbgLbl[d]); break;} }
    }
    return 0;
}
