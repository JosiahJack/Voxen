// common.h - Shared items between engine and gamecode (e.g. enums)
#pragma once
#define GAME_TITLE "Citadel"
#define WIN_ICON "./Textures/UI/menudot1.png"
#define INLINE static inline __attribute__((always_inline))
typedef __INT8_TYPE__   i8; typedef  __UINT8_TYPE__  u8;                               /*8bit types*/
typedef __INT16_TYPE__ i16; typedef __UINT16_TYPE__ u16; typedef u16 half;            /*16bit types*/
typedef __INT32_TYPE__ i32; typedef __UINT32_TYPE__ u32;                              /*32bit types*/
typedef __INT64_TYPE__ i64; typedef __UINT64_TYPE__ u64; typedef __SIZE_TYPE__ size_t;/*64bit types*/
typedef __UINTPTR_TYPE__ uintptr_t; typedef __INTPTR_TYPE__ intptr_t;
#define bool u8
#define true 1
#define false 0
#define likely(x)   __builtin_expect(!!(x),1)
#define unlikely(x) __builtin_expect(!!(x),0)
#define NULL ((void *)0)
enum{U16_MAX=65535};
typedef __builtin_va_list va_list;
typedef struct { float r,g,b; } Color3; typedef struct { float r,g,b,a; } Color;    typedef struct { float x,y; } V2;  typedef struct { float x,y,z; } V3; typedef struct { float x,y,z,w; } Quaternion;    typedef u8 ColliderType; typedef u16 Text;
typedef struct { bool hit; V3 point,normal; float pen; } Overlap;    typedef struct { V3 mn,mx; u32 triStart; u16 triCount; i16 children[8]; } BvhNode;
#if defined(_WIN32)
    typedef void* FHandle;
#else
    typedef int FHandle;
#endif
// SIMD
typedef float __v8sf __attribute__((__vector_size__(32))); typedef float __m256 __attribute__((__vector_size__(32))); typedef long long __m128i __attribute__((__vector_size__(16))); typedef __m128i __m128i_u __attribute__((__may_alias__, __aligned__(1))); typedef __m256 __m256_u __attribute__((__may_alias__, __aligned__(1)));
typedef float __v4sf __attribute__((__vector_size__(16))); typedef int __v4si __attribute__((__vector_size__(16)));   typedef float __m128 __attribute__((__vector_size__(16)));      typedef __m128 __m128_u __attribute__((__may_alias__, __aligned__(1)));
#define _mm_storeu_si128(P, V) (*(__m128i_u *)(P) = (V))
#define _mm_loadu_ps(P) (*(__m128_u const *)(P))
#define _mm_set1_ps(A) ((__m128){ (A), (A), (A), (A) })
#define _mm_setr_ps(e0,e1,e2,e3) ((__m128){ (e0), (e1), (e2), (e3) })
#define _mm_storeu_ps(P, A) (*(__m128_u *)(P) = (A))
#define _mm_add_ps(A, B) ((__m128)((__v4sf)(A) + (__v4sf)(B)))
#define _mm_mul_ps(A, B) ((__m128)((__v4sf)(A) * (__v4sf)(B)))
#define __m256i __m256i_t
#define _mm256_storeu_si256(P, V) (*(__m256i*)(P) = (V))
#define _mm256_loadu_si256(P) (*(__m256i*)(P))
#define _mm_loadu_si128(P) (*(__m128i_u*)(P))
typedef long long __m256i_t __attribute__((__vector_size__(32), __may_alias__, __aligned__(1)));
static inline __m256i _mm256_set1_epi8_fast(char c) { __m256i v; char *p = (char*)&v; for (int i = 0; i < 32; ++i) p[i] = c; return v; }
static inline __m128i _mm_set1_epi8_fast(char c) { __m128i v; char *p = (char*)&v; for (int i = 0; i < 16; ++i) p[i] = c; return v; }
typedef int (*cmpfun)(const void*,const void*);
typedef int (*cmpfun_r)(const void*,const void*,void*);
#define assert(cond) do { if (!(cond)) { DualLogError("[%s:%d]:%s(): Assert fail:%s\n",__FILE__,__LINE__,__func__,#cond); *(volatile int*)0 = 0; } } while(0) // Force a crash for debug
#define CHECK_GL_ERROR() do { u32 err = glGetError(); if (err != 0) DualLogError("GL Error at %s:%d: %d\n", __FILE__, __LINE__, err); } while(0)
void DualLogError(const char* s, ...); void DualLog(const char* s, ...); void DualLogWarn(const char* s, ...);
#define THRSTACKSZ (8 * 1024 * 1024)
#if defined(_WIN32)
    #define DLL_IMP __declspec (dllimport)
    #define WINAPI __stdcall
    #define INVALID_FHANDLE ((void*) (i64)-1)
    typedef struct { void* handle; } OS_Thread;
    struct timespec { i64 tv_sec; i32 tv_nsec; }; struct sched_param { int sched_priority; }; typedef uintptr_t pthread_t; typedef intptr_t pthread_mutex_t,pthread_cond_t; typedef int pthread_condattr_t; typedef u32 pthread_mutexattr_t;
    typedef struct pthread_attr_t { unsigned p_state; void *stack; size_t s_size; struct sched_param param; } pthread_attr_t; typedef struct { unsigned long Data1; u16 Data2,Data3; u8 Data4[8]; } GUID;
    int pthread_create(pthread_t*,const pthread_attr_t*,void*(*func)(void*),void*); int pthread_join(pthread_t,void**); DLL_IMP void* WINAPI GetStdHandle(u32);
#else
    #define INVALID_FHANDLE -1
    typedef struct __attribute__((aligned(16))) OS_ThreadHead { void(*trampoline)(struct OS_ThreadHead*),*(*fn)(void*),*arg; int join_futex,_pad; } OS_ThreadHead;
    typedef struct { struct OS_ThreadHead* head; void* stack_base; } OS_Thread;
    struct timespec {i64 tv_sec,tv_nsec;}; typedef u64 pthread_t; typedef u32 pthread_mutexattr_t; typedef struct {u8 _[40];} pthread_mutex_t; typedef struct {u8 _[48];} pthread_cond_t; typedef int pthread_condattr_t;
    typedef struct {u32 flags; void* stack;} pthread_attr_t;
    int pthread_create(pthread_t* restrict,const pthread_attr_t* restrict,void*(*start_routine)(void*),void* restrict); int pthread_join(pthread_t,void**); void *dlopen(const char*,int); void *dlsym(void*,const char*); INLINE long OS_Open(const char*,i32,i32);
#endif
void* mcpy(void *dst, const void *src, size_t n);
#if defined(_WIN32)
    typedef i64 (WINAPI *PROC)(); typedef i64 (WINAPI *FARPROC)(); typedef i64 (WINAPI *NEARPROC)();
    typedef struct { int unused; } *HINSTANCE; typedef HINSTANCE HMODULE;  typedef struct { u32 nLength; void* lpSecurityDescriptor; i32 bInheritHandle; } *LPSECURITY_ATTRIBUTES;
    typedef struct { i64 QuadPart; } LARGE_INTEGER; typedef LARGE_INTEGER *PLARGE_INTEGER; typedef struct { u64 Internal,InternalHigh; union {struct {u32 Offset,OffsetHigh;} DUMMYSTRUCTNAME; void* Pointer;} DUMMYUNIONNAME; void* hEvent; } OVERLAPPED, *LPOVERLAPPED;
    typedef struct { union { u32 dwOemId; struct { u16 wProcessorArchitecture,wReserved; } DUMMYSTRUCTNAME; } DUMMYUNIONNAME; u32 dwPageSize; void* lpMinimumApplicationAddress,*lpMaximumApplicationAddress; u64 dwActiveProcessorMask; u32 dwNumberOfProcessors,dwProcessorType,dwAllocationGranularity; u16 wProcessorLevel,wProcessorRevision; } SYSTEM_INFO, *LPSYSTEM_INFO;
    DLL_IMP void* WINAPI CreateFileMappingA(void*,LPSECURITY_ATTRIBUTES,u32,u32,u32,const char*); DLL_IMP i32 WINAPI VirtualFree(void*,u64,u32);    DLL_IMP void* WINAPI VirtualAlloc(void*,u64,u32,u32);         DLL_IMP i32 WINAPI ReadFile(void*,void*,u32,u32*,LPOVERLAPPED);    DLL_IMP i32 WINAPI GetFileSizeEx(void*,PLARGE_INTEGER);          DLL_IMP i32 WINAPI UnmapViewOfFile(void*); DLL_IMP FARPROC WINAPI GetProcAddress(HINSTANCE,const char*);
    DLL_IMP void* WINAPI CreateFileA(const char*,u32,u32,LPSECURITY_ATTRIBUTES,u32,u32,void*);                                                      DLL_IMP i32 WINAPI QueryPerformanceCounter(LARGE_INTEGER*);   DLL_IMP void* WINAPI MapViewOfFileEx(void*,u32,u32,u32,u64,void*); DLL_IMP i32 WINAPI WriteFile(void*,void*,u32,u32*,LPOVERLAPPED); DLL_IMP i32 WINAPI CloseHandle(void*);     DLL_IMP __declspec (noreturn) void WINAPI ExitProcess(u32);
    DLL_IMP i32 WINAPI SetFilePointerEx(void*,LARGE_INTEGER,PLARGE_INTEGER,u32);                  DLL_IMP void WINAPI GetSystemInfo(LPSYSTEM_INFO); DLL_IMP i32 WINAPI QueryPerformanceFrequency(LARGE_INTEGER*); DLL_IMP void* WINAPI MapViewOfFile(void*,u32,u32,u32,u64);         DLL_IMP HINSTANCE WINAPI LoadLibraryA(const char*);              DLL_IMP void* WINAPI CreateFileMappingW(void*,LPSECURITY_ATTRIBUTES,u32,u32,u32,u16*);
    INLINE __attribute__((noreturn)) void OS_Exit(i64 exitCode) { ExitProcess((u32)exitCode); __builtin_unreachable(); }
    INLINE void OS_Close(FHandle fd) { CloseHandle(fd); }
    INLINE void* OS_AllocateRAM(size_t l,i32 p,i32 f,FHandle fd) { (void)f; if (fd==(void*)-1) return VirtualAlloc(NULL,l,0x3000,(p&2)?4:2); void* m = CreateFileMappingW(fd,NULL,(p&2) ? 4 : 2,(u32)(l>>32),(u32)l,NULL); void* r=MapViewOfFileEx(m,(p&2)?2:4,0,0,l,NULL); return CloseHandle(m),r;}
    int __cdecl _mkdir(const char* dirname);
    INLINE int OS_MakeFolder(const char* path) { _mkdir(path); return 0; }
    INLINE long OS_Read(FHandle fd, void* buf, size_t count) { u32 bytesRead = 0; return (ReadFile((void*)fd,buf,(u32)count,&bytesRead,NULL)) ? (long)bytesRead : (long)-1; }
    INLINE FHandle OS_OpenReadonly(const char* path) { void* f = CreateFileA(path,0x80000000L,1,NULL,3,0x08000080,NULL); return f == (void*)-1 ? DualLogError("Could not open file %s for reading\n",path), (void*)-1 : f; }
    INLINE FHandle OS_OpenWriteonly(const char* path) { FHandle h = CreateFileA(path,0x40000000L,0,NULL,2,128,NULL); return h == (void*)-1 ? DualLogError("Failed to open %s for writing\n",path),(void*)-1 : h; }
    INLINE int OS_FileSize(FHandle f) { LARGE_INTEGER s; return (f==(FHandle)-1 || !GetFileSizeEx(f,&s)) ? -1 : (int)s.QuadPart; }
    INLINE void* OS_AllocateFileBackedRAMReadonly(size_t s,FHandle fd, char* path) { void* m; void* r; return(fd==(void*)-1||!s||!(m=CreateFileMappingA(fd,NULL,2,0,0,NULL))) ? DualLogError("CreateFileMappingA failed for %s\n",path),NULL : (r=MapViewOfFile(m,4,0,0,s)) ? (CloseHandle(m),r) : (DualLogError("Failed to allocate %s\n",path),CloseHandle(m),NULL);}
    INLINE long OS_Seek(FHandle fd, i64 ofs, int whence /*forth and forsooth pray tell*/) { LARGE_INTEGER l={.QuadPart=ofs},n; return SetFilePointerEx((void*)fd,l,&n,whence) ? n.QuadPart : -1; }
    INLINE long OS_Tell(FHandle fd) { LARGE_INTEGER l={0},n; return SetFilePointerEx((void*)fd,l,&n,1) ? n.QuadPart : -1; }
    INLINE int OS_GetNumThreads() { SYSTEM_INFO si; GetSystemInfo(&si); return (int)si.dwNumberOfProcessors; }
    INLINE void OS_Free(void* p, size_t s) { (void)s; if(!p) { DualLogError("Attempting to double free!\n"); OS_Exit(1); } if(!UnmapViewOfFile(p) && !VirtualFree(p,0,0x00008000)) DualLogError("VirtualFree failed\n"); }
    INLINE long OS_RawWrite(FHandle fd, const void* buf, size_t count) { u32 w; return WriteFile((void*)fd,(void*)buf,(u32)count,&w,NULL) ? (i64)w : -1; }
    void* __stdcall GetProcessHeap(); void* __stdcall HeapAlloc(void* hHeap, u32 dwFlags, size_t dwBytes); i32 __stdcall HeapFree(void* hHeap, u32 dwFlags, void* lpMem); void __stdcall Sleep(u32 dwMilliseconds); u32 __stdcall WaitForSingleObject(void* hHandle, u32 dwMilliseconds);
    typedef u32 (__stdcall *LPTHREAD_START_ROUTINE)(void* lpParameter);
    void* __stdcall CreateThread(void* lpThreadAttributes, size_t dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, void* lpParameter, u32 dwCreationFlags, u32* lpThreadId);
    INLINE u32 WINAPI thrtramp(void* a) { void** b=(void**)a; void*(*fn)(void*)=(void*(*)(void*))b[0]; void* arg=b[1]; HeapFree(GetProcessHeap(),0,b); fn(arg); return 0; }
    INLINE int OS_ThreadCreate(OS_Thread* out, void*(*fn)(void*), void* arg) { void** b=(void**)HeapAlloc(GetProcessHeap(),0,2 * sizeof(void*)); b[0]=(void*)fn; b[1]=arg; out->handle=CreateThread(NULL,THRSTACKSZ,thrtramp,b,0,NULL); if(!out->handle){HeapFree(GetProcessHeap(),0,b); return -1;} return 0; }
    INLINE void OS_ThreadJoin(OS_Thread* t) { WaitForSingleObject(t->handle,0xFFFFFFFFUL); CloseHandle(t->handle); t->handle = NULL; }
    INLINE void OS_USleep(u32 usec) { Sleep((usec + 999) / 1000); }
    INLINE double get_time() { static LARGE_INTEGER frequency,counter; static i32 init=0; if (!init) { QueryPerformanceFrequency(&frequency); init=1; } QueryPerformanceCounter(&counter); return (double)counter.QuadPart / frequency.QuadPart; }
#else
    struct input_id { u16 bustype,vendor,product,version;}; struct input_event { struct { long tv_sec,tv_usec; } time; u16 type,code; i32 value; };
    INLINE int OS_MakeFolder(const char* path) { long r = 83; __asm__ __volatile__("syscall":"+a"(r):"D"(path),"S"(0755LL):"rcx","r11","memory"); return (int)r; }
    INLINE long OS_Read(FHandle f,void*b,size_t c) { long r = 0; __asm__ __volatile__("syscall":"+a"(r):"D"(f),"S"(b),"d"(c):"rcx","r11","memory"); return r; }
    INLINE __attribute__((noreturn)) void OS_Exit(i64 exitCode) { long r = 231; __asm__ __volatile__("syscall":"+a"(r):"D"(exitCode):"rcx","r11","memory"); __builtin_unreachable(); }
    INLINE void OS_Close(FHandle fd) { long r = 3; __asm__ __volatile__("syscall":"+a"(r):"D"(fd):"rcx","r11","memory"); }
    INLINE long OS_Open(const char* path, i32 flags, i32 mode) { long r = 2; __asm__ __volatile__("syscall":"+a"(r):"D"(path),"S"((long)flags),"d"((long)mode):"rcx","r11","memory"); return r; }
    INLINE void* OS_AllocateRAM(size_t len, i32 prot, i32 flags, FHandle fd) { long r=9; register int r10 __asm__("r10")=flags; register int r8 __asm__("r8")=fd; register long r9 __asm__("r9")=0; __asm__ __volatile__("syscall":"+a"(r):"D"(NULL),"S"(len),"d"(prot),"r"(r10),"r"(r8),"r"(r9):"rcx","r11","memory"); return (void*)r; }
    INLINE FHandle OS_OpenReadonly(const char* path) { FHandle f=OS_Open(path,0,0); return f < 0 ? DualLogError("Could not open file %s for reading\n",path), -1 : f; }
    INLINE FHandle OS_OpenWriteonly(const char* path) { FHandle f=OS_Open(path,1|00000100|00001000,0644); return f < 0 ? DualLogError("Failed to open %s for writing\n",path),-1 : f; }
    INLINE int OS_FileSize(FHandle f) { long r=5,s[18]; __asm__ __volatile__("syscall":"+a"(r):"D"(f),"S"(s):"rcx","r11","memory"); return (int)s[6]; }
    INLINE void* OS_AllocateFileBackedRAMReadonly(size_t s, FHandle fd, char* path) { void* r=OS_AllocateRAM(s,1,2,fd); return r==(void*)-1 ? DualLogError("Failed to allocate %s\n",path),NULL : r; }
    INLINE long OS_Seek(FHandle fd, i64 ofs, int whence /* forth and forsooth pray tell*/) { i64 r = 8; __asm__ __volatile__("syscall":"+a"(r):"D"(fd),"S"(ofs),"d"(whence):"rcx","r11","memory"); return r; }
    INLINE long OS_Tell(FHandle fd) { i64 r=8; __asm__ __volatile__("syscall":"+a"(r):"D"(fd),"S"(0LL),"d"(1):"rcx","r11","memory"); return r; }
    INLINE int OS_GetNumThreads() { unsigned long m[16]; long r=204; __asm__ __volatile__("syscall":"+a"(r):"D"(0LL),"S"(128LL),"d"(m):"rcx","r11","memory"); int c = 0; for(int i=0;i<(r/8);i++) {c+=__builtin_popcountll(m[i]);} return r < 0 ? 1 : c; }
    INLINE void OS_Free(void* p, size_t s){ long r=11; if(!p || p == (void*)-1) { DualLogError("Attempting to double free!\n"); OS_Exit(1); } __asm__ __volatile__("syscall":"+a"(r):"D"(p),"S"(s):"rcx","r11","memory"); if(r<0) DualLogError("munmap failed\n"); }
    INLINE long OS_RawWrite(FHandle fd, const void* buf, size_t cnt) { i64 r=1; __asm__ __volatile__("syscall":"+a"(r):"D"(fd),"S"(buf),"d"(cnt):"rcx","r11","memory"); return r; }
    #define SYSCALL1(n, a) syscall6(n,(long)(a),0,0,0,0,0)
    #define SYSCALL2(n, a, b) syscall6(n,(long)(a),(long)(b),0,0,0,0)
    #define SYSCALL3(n, a, b, c) syscall6(n,(long)(a),(long)(b),(long)(c),0,0,0)
    #define SYSCALL4(n, a, b, c, d) syscall6(n,(long)(a),(long)(b),(long)(c),(long)(d),0,0)
    INLINE long syscall6(long n, long a, long b, long c, long d, long e, long f) { register long r=n; register long r10 __asm__("r10") = d; register long r8  __asm__("r8")  = e; register long r9  __asm__("r9")  = f; __asm__ __volatile__("syscall":"+a"(r):"D"(a),"S"(b),"d"(c),"r"(r10),"r"(r8),"r"(r9):"rcx","r11","memory"); return r; }
    __attribute__((naked)) static long OS_CloneSyscall(struct OS_ThreadHead* stack) { __asm__ volatile ("mov %%rdi, %%rsi\nmov $0x50f00, %%edi\nmov $56, %%eax\nsyscall\nmov  %%rsp, %%rdi\nret\n":::"rax","rcx","rsi","rdi","r11","memory"); }
    __attribute__((noreturn)) static void thrtramp(struct OS_ThreadHead* head) { head->fn(head->arg); __atomic_store_n(&head->join_futex,1,__ATOMIC_SEQ_CST); SYSCALL3(202,&head->join_futex,1,0x7fffffff);/*futex wake*/  SYSCALL1(60,0); __builtin_unreachable(); }
    INLINE int OS_ThreadCreate(OS_Thread* out, void*(*fn)(void*), void* arg) { void* base = OS_AllocateRAM(THRSTACKSZ,0x1|0x2,0x02|0x20,INVALID_FHANDLE); if (!base || base == (void*)-1) return -1; struct OS_ThreadHead* head = (struct OS_ThreadHead*)((char*)base + THRSTACKSZ) - 1; head->trampoline = thrtramp; head->fn=fn; head->arg=arg; head->join_futex=0; head->_pad=0; long tid = OS_CloneSyscall(head); if(tid < 0){OS_Free(base,THRSTACKSZ); return (int)tid;} out->head=head; out->stack_base=base; return 0; } // Multithreading taken from https://github.com/skeeto/scratch/blob/master/misc/stack_head.c Ref: https://nullprogram.com/blog/2023/03/23/ This is free and unencumbered software released into the public domain.
    INLINE void OS_ThreadJoin(OS_Thread* t) { int v; while ((v = __atomic_load_n(&t->head->join_futex, __ATOMIC_SEQ_CST)) == 0) SYSCALL4(202, &t->head->join_futex, 0 /*FUTEX_WAIT*/, v, 0); OS_Free(t->stack_base,THRSTACKSZ); t->head = NULL; t->stack_base = NULL; }
    INLINE void OS_USleep(u32 usec) { long ts[2] = {usec / 1000000,(usec % 1000000) * 1000L}; SYSCALL2(35,ts,ts); }
    INLINE double get_time() { struct {i64 s,ns;} ts; i64 ret; __asm__ __volatile__("syscall":"=a"(ret):"a"(228),"D"(1),"S"(&ts):"rcx","r11","memory"); if (ret != 0) {return 0.0;} return (double)ts.s + (double)ts.ns * 1e-9; } // Full time in seconds, 1 for MONOTONIC, Note that using clock_gettime wasn't any better for performance.
#endif
INLINE void* OS_Alloc(size_t amount) { return OS_AllocateRAM(amount,0x1|0x2,0x02|0x20,INVALID_FHANDLE); }
INLINE void OS_Write(FHandle f,const void* buf, size_t s, const char* p) { size_t total=0; while(total < s) { i64 w=OS_RawWrite(f,(const char*)buf + total,s - total); if(w < 0) { DualLogError("Write error to %s: %s[%d]\n",p,w,(i32)-w); OS_Exit(1); } total += (size_t)w; } }
INLINE void* OS_OpenAndAllocateFileBufferReadonly(const char* p,FHandle* f,int* s) {void* r;return((*f=OS_OpenReadonly(p))==(FHandle)-1)?*s=0,(void*)0:((*s=OS_FileSize(*f))<=0)?DualLogError("Skipping empty:%s\n",p),OS_Close(*f),OS_Exit(1),NULL:(r=OS_AllocateFileBackedRAMReadonly(*s,*f,(char*)p))?(OS_Close(*f),r):NULL;}
INLINE void* OS_Realloc(void* old, size_t olds, size_t news) { void* n; return !old ? OS_Alloc(news) : news <= olds ? old : (n=OS_Alloc(news)) ? (mcpy(n,old,olds),OS_Free(old,olds),n) : 0; }
static const Quaternion QUAT_IDENTITY=(Quaternion){0.0f,0.0f,0.0f,1.0f};
typedef struct { V3 point; V3 normal; float distance; u16 hitInstanceIndex; bool hit;} RaycastHit;
typedef struct { float speed; u16 frameStart,frameEnd,frameStartModelIndex; u8 framerate;} AnimationClip;
typedef struct { V3 pos; float intensity; Color3 col; u32 lflags; float range,spotAng,maxIntensity,minIntensity; Quaternion spotDir; } Light; // 64bytes, one cache line, packed for GL transfer
typedef struct { float lerpValue,lerpStepTime,lerpStartTime,lerpTime,intervalSteps[32]; bool stepIsLerping[32],lerpUp; u8 currentStep,numIntervalSteps,numLerpSteps; } LightAnimation; // Separate from main lights buffer struct since it's not used very often
enum {
    /*Culling*/ WORLDX = 64, WORLDZ = 64, WORLDY = 18, VOXELS_PER_CELL = 8, ARRSIZE = (WORLDX * WORLDZ), VOXELS_X = (WORLDX * VOXELS_PER_CELL), VOXELS_Z = (WORLDZ * VOXELS_PER_CELL), VOXEL_COUNT = (VOXELS_X * VOXELS_Z) /*64 * 64 * 8 * 8*/, 
                MAX_PORTALS = 56 /*Max 49 on lev 7*/, CELL_VISIBLE = 1, CELL_OPEN = 2, CELL_CLOSEDNORTH = 4, CELL_CLOSEDEAST = 8, CELL_CLOSEDSOUTH = 16, CELL_CLOSEDWEST = 32, CELL_SEES_SUN = 64, CELL_SEES_SKYBOX = 128,
    /*Entity Management*/ MAX_LEVELS = 14, LEVEL_CYBERSPACE = 13, CREDITS_PAGES = 22, AVG_CPU_TAPS = 2048, MAX_ENTITIES = 768, INSTANCE_COUNT = 9000, WORLD = 0, PLAYER1 = 1, INSTS_1ST_IDX = 2, NUM_AI_TYPES = 29,
    /*Lights*/ LIGHT_COUNT = 2200, MAX_LIGHTS_PER_VOXEL = 128, SHADOW_MAP_SIZE = 128, MAX_SHADOWMAPS = 128, LIGHTON = 1, SHADON = 2, LIGHT_AND_SHADOW_ON = 3, LSPOT = 4, LDIR = 8, LDIRTY = 16, LERPON = 32, 
    /*Models*/ MAX_MDLS=6400, WELD_HASH_SIZE=32768, MAX_VERT_ELEMENT_SIZE=6964, MAX_OUTPUT_VERTS=22960, VRT_ATT_SZ=16, CPU_VRT_SZ=32,
    /*Textures*/ MAX_TXRS = 2048, MAX_TOTAL_PIXELS = 38400780u, MAX_UNIQUE_COLORS = 120040u,
    /*Animations*/ MAX_ANIMCLIPS = 10, MAX_ANIMS = 53, ANIM_LOOP_ALL = 0, ANIM_IDLE_CLOSED = 0, ANIM_IDLE = 0, ANIM_INACTIVE = 0, ANIM_ATTACK_MISS = 1, ANIM_OPENING = 1, ANIM_WALK = 1, ANIM_ACTIVATE = 1, ANIM_ATTACK_HIT = 2, ANIM_ACTIVATED = 2,
                   ANIM_IDLE_OPEN = 2, ANIM_RUN = 2, ANIM_CLOSING = 3, ANIM_DEACTIVATE = 3, ANIM_ATTACK1 = 3, ANIM_ATTACK2 = 4, ANIM_INSTALL = 4, ANIM_ATTACK3 = 5, ANIM_INSTALLED = 5, ANIM_PAIN = 6, ANIM_PAIN2 = 7, ANIM_PAIN3 = 8, ANIM_DYING = 9,
    /*Physics*/ COLTYPE_NONE = 0, COLTYPE_BOX = 1, COLTYPE_SPH = 2, COLTYPE_CAP = 3, COLTYPE_CVX = 4, COLTYPE_MSH = 5, MAX_UNIQUE_CVX_MESHES = 5989, BVH_MAX_DEPTH=6, BVH_LEAF_MAX_TRIS=8, BVH_MAX_NODES_PER_MDL=586/*1 + 8 + 64 + 512 = 585*/, BVH_MAX_TRIS_PER_MDL=8000, MAX_WIRELINE_VRTS = 2024000,
    /*Input*/ MAX_KEYS = 512, MAX_MOUSE_BUTTONS = 8, INPUT_RELEASE = 0, INPUT_PRESS = 1, INPUT_REPEAT = 2,
    /*Audio*/ MAX_CHANNELS = 48, SOUNDS_COUNT = 670,
    /*Text*/ TARGET_STRING_LENGTH = 38, T_LOGSTR_CNT = 1100, T_LOGSTR_MAX = 1280, LOGCNT = 134, T_WHITE = 0, T_YELLOW = 1, T_DARK_YELLOW = 2, T_GREEN = 3, T_RED = 4, T_ORANGE = 5, T_STOPD_RED = 6, T_STOPD_RED_HIGHLIGHT = 7, T_STOPD_RED_PAUSETITLE = 8,
             T_GREEN_MENU = 9, T_GREEN_MENU_SHADOW = 10, T_GREEN_MENU_GLOW = 11, T_RED_MENU = 12, T_BUFFER_SIZE=1024, MAX_GLYPHS=4096, FONT_ATLAS_SIZE=4672, FONT_NORMAL=0, FONT_STOPD=1,
    /*Multimedia Tabs(UI)*/ MM_EMAIL_TABLE = 0, MM_LOG_TABLE = 1, MM_DATA_TABLE = 2, MM_NOTES = 3
};
bool cEmpty(const char c);
INLINE u32 parse_numberu32(const char* str, const char* line, u32 lineNum) {
    if(str == 0 || *str == '\0'){DualLogError("Invalid from line[%d]: %s\n",lineNum+1,line); return 0;}
    while(cEmpty((char)*str)){str++;} while(cEmpty(*str)){str++;} if(*str == '+'){str++;}
    if(*str == '-'){DualLogError("Invalid negative u32(%s) from line[%d]: %s\n",str,lineNum+1,line); return 0;}
    u64 result=0; while (*str >= '0' && *str <= '9') { i32 digit=*str-'0'; result=result*10 + (u64)digit; str++; } return (u32)result;
}

INLINE u16 parse_numberu16(const char* str, const char* line, u32 lineNum) { u32 retval = parse_numberu32(str, line, lineNum); if (retval > U16_MAX) { DualLogError("Value %u out of range for u16 from line[%d]: %s\n", retval, lineNum+1, line); return 0; } return (u16)retval; }
INLINE u8 parse_numberu8(const char* str, const char* line, u32 lineNum) { u32 retval = parse_numberu32(str, line, lineNum); if (retval > 255) { DualLogError("Value %u out of range for u8 from line[%d]: %s\n", retval, lineNum+1, line); return 0; } return (u8)retval; }
INLINE bool parse_bool(const char* str, const char* line, u32 lineNum) { u32 parseval = parse_numberu32(str, line, lineNum); if (parseval > 1) {DualLogWarn("Loaded %u but expected boolean from line[%u]: %s\n",parseval, lineNum+1, line);} return parseval > 0 ? true : false; }
static const float PLAYER_RADIUS=0.48f,PLAYER_HEIGHT=2.00f,PLAYER_CAM_OFFSET_Y=0.84f,CELLSZ=2.56f,CELLXHALF=(CELLSZ * 0.5f),VOXEL_SIZE=(CELLSZ/(float)VOXELS_PER_CELL),VOXEL_HALF=(VOXEL_SIZE * 0.5f),COLCAP_DIR_X_F=0.0f,COLCAP_DIR_Y_F=1.0f,COLCAP_DIR_Z_F=2.0f,
                   REFLEX_TIME_SCALE=0.25,DEFAULT_TIME_SCALE=1.0,BERSERK_DAMAGE_MULTIPLIER=4.0f/*Quad Damage!*/;
static const double BERSERK_TIME=20.0,DETOX_TIME=60.0,GENIUS_TIME=180.0,MEDI_TIME=35.0,REFLEX_TIME=155.0,SIGHT_TIME=40.0,STAMINUP_TIME=60.0,SIGHT_SIDE_EFFECT_TIME=17.0,NITRO_MIN_TIME=1.0,NITRO_MAX_TIME=60.0,NITRO_DEFAULT_TIME=7.0,EARTH_SHAKER_MIN_TIME=4.0,
                    EARTH_SHAKER_MAX_TIME=60.0,EARTH_SHAKER_DEFAULT_TIME=10.0;
enum{EF_ACTIVE=(1u<<0),                      EF_GROUNDED=(1u<<2),EF_RIGIDBODY=(1u<<3),EF_NO_SHADOWS=(1u<<4),EF_ASLEEP=(1u<<5),EF_WALK_PATH_ON_START=(1u<<6),EF_TOUCHING_HURTS=(1u<<7),EF_ACT_AS_CORPSE_ONLY=(1u<<8),EF_DYING=(1u<<9),EF_DEATH_BURST_DONE=(1u<<10),
     EF_DEAD=(1u<<11),EF_TELEPORT_ON_DEATH=(1u<<12),EF_GO_INTO_PAIN=(1u<<13),EF_WANDERING=(1u<<14),EF_ACT_AS_TURRET=(1u<<15),EF_TARGID_ATTACHED=(1u<<16),EF_ENEM_IN_SIGHT=(1u<<17),EF_ENEM_IN_FRONT=(1u<<18),EF_ENEM_IN_FOV=(1u<<19),EF_ENEM_IN_LOS=(1u<<20),
     EF_FIRST_SIGHTING=(1u<<21),EF_DYING_SETUP=(1u<<22),EF_HAD_ENEMY=(1u<<23),EF_SHOT_FIRED=(1u<<24),EF_DEAD_CHECKS_DONE=(1u<<25),EF_HOP_DONE=(1u<<26),EF_LOCKED=(1u<<27),EF_HAS_CAMERA_VIEW=(1u<<28),EF_DAMAGE_ON_USE=(1u<<29)};
enum{Q_ROBOT_SPAWN_DEACTIVATED=(1u<<0),Q_ISOTOPE_INSTALLED=(1u<<1),Q_SHIELD_ACTIVATED=(1u<<2),Q_LASER_SAFETY_OVERRIDEN=(1u<<3),Q_LASER_DESTROYED=(1u<<4),Q_BETA_GROVE_CYBER_UNLOCKED=(1u<<5),Q_GROVE_ALPHA_JETTISON_ENABLED=(1u<<6),
     Q_GROVE_BETA_JETTISON_ENABLED=(1u<<7),Q_GROVE_DELTA_JETTISON_ENABLED=(1u<<8),Q_MASTER_JETTISON_BROKEN=(1u<<9),Q_RELAY_428_FIXED=(1u<<10),Q_MASTER_JETTISON_ENABLED=(1u<<11),Q_BETA_GROVE_JETTISONED=(1u<<12),Q_ANTENNA_NORTH_DESTROYED=(1u<<13),
     Q_ANTENNA_SOUTH_DESTROYED=(1u<<14),Q_ANTENNA_EAST_DESTROYED=(1u<<15),Q_ANTENNA_WEST_DESTROYED=(1u<<16),Q_SELF_DESTRUCT_ACTIVATED=(1u<<17),Q_BRIDGE_SEPARATED=(1u<<18),Q_ISOLINEAR_CHIPSET_INSTALLED=(1u<<19),Q_LEV1_CODE_LOCKED=(1u<<20),
     Q_LEV2_CODE_LOCKED=(1u<<21),Q_LEV3_CODE_LOCKED=(1u<<22),Q_LEV4_CODE_LOCKED=(1u<<23),Q_LEV5_CODE_LOCKED=(1u<<24),Q_LEV6_CODE_LOCKED=(1u<<25)};
enum{TARG_IOFLAGS_TRIPTRIGGER=(1u<<0),TARG_IOFLAGS_DOOROPEN=(1u<<1),TARG_IOFLAGS_DOOROPENIFUNLOCKED=(1u<<2),TARG_IOFLAGS_DOORCLOSE=(1u<<3),TARG_IOFLAGS_LOCK=(1u<<4),TARG_IOFLAGS_UNLOCK=(1u<<5),TARG_IOFLAGS_SWITCHTRIGGER=(1u<<6),
     TARG_IOFLAGS_CHGSTAT_RECHARGE=(1u<<7),TARG_IOFLAGS_ENEMY_ALERT=(1u<<8),TARG_IOFLAGS_FBRIDGE_ACTIVATE=(1u<<9),TARG_IOFLAGS_FBRIDGE_DEACTIVATE=(1u<<10),TARG_IOFLAGS_FBRIDGE_TOGGLE=(1u<<11),TARG_IOFLAGS_GRAVLIFT_TOGGLE=(1u<<12),
     TARG_IOFLAGS_TEXTURE_CHG_TOGGLE=(1u<<13),TARG_IOFLAGS_LIGHT_ON=(1u<<14),TARG_IOFLAGS_LIGHT_OFF=(1u<<15),TARG_IOFLAGS_LIGHT_TOGGLE=(1u<<16),TARG_IOFLAGS_FUNCWALL_MOVE=(1u<<17),TARG_IOFLAGS_MISSION_BIT_ON=(1u<<18),TARG_IOFLAGS_MISSION_BIT_OFF=(1u<<19),
     TARG_IOFLAGS_MISSION_BIT_TOGGLE=(1u<<20),TARG_IOFLAGS_SWITCH_LOCK_TOGGLE=(1u<<21),TARG_IOFLAGS_INST_ACTIVATE=(1u<<22),TARG_IOFLAGS_INST_DEACTIVATE=(1u<<23),TARG_IOFLAGS_INST_TOGGLE=(1u<<24),TARG_IOFLAGS_PLAY_SOUND_ONCE=(1u<<25),
     TARG_IOFLAGS_STOP_SOUND=(1u<<26),TARG_IOFLAGS_START_FLASHING_TEX=(1u<<27),TARG_IOFLAGS_STOP_FLASHING_TEX=(1u<<28),TARG_IOFLAGS_BRANCH_FLIP=(1u<<29),TARG_IOFLAGS_BRANCH_FLIPONLY=(1u<<30),TARG_IOFLAGS_DISABLE_ON_AWAKE=(1u<<31)};
typedef enum {BodyState_Standing=0,BodyState_Crouch=1,BodyState_CrouchingDown=2,BodyState_StandingUp=3,BodyState_Prone=4,BodyState_ProningDown=5,BodyState_ProningUp=6} BodyState;
typedef enum {Att_None=0,Att_Melee=1,Att_MlEg=2,Att_Beam=3,Att_Magn=4,Att_HitS=5,Att_PjNd=6,Att_PjBm=7,Att_Ball=8,Att_Gas=9,Att_Trnq=10,Att_Drill=11} AttType;
typedef enum {NPCType_Mutant=0,NPCType_Supermutant=1,NPCType_Robot=2,NPCType_Cyborg=3,NPCType_Supercyborg=4,NPCType_Cyber=5,NPCType_MutantCyborg=6} NPCType;
typedef enum {PerceptionLevel_Low=0,PerceptionLevel_Medium=1,PerceptionLevel_High=2,PerceptionLevel_Omniscient=3} PerceptionLevel;
typedef enum {AIState_Idle=0,AIState_Walk=1,AIState_Run=2,AIState_Attack1=3,AIState_Attack2=4,AIState_Attack3=5,AIState_Pain=6,AIState_Dying=7,AIState_Dead=8,AIState_Inspect=9,AIState_Interacting=10} AIState;
typedef enum {AIMoveType_Walk=0,AIMoveType_Fly=1,AIMoveType_Swim=2,AIMoveType_Cyber=3,AIMoveType_None=4} AIMoveType;
typedef enum {SecurityType_None=0,SecurityType_Camera=1,SecurityType_NodeSmall=2,SecurityType_NodeLarge=3} SecurityType;
typedef enum {DoorState_Closed=0,DoorState_Open=1,DoorState_Closing=2,DoorState_Opening=3} DoorState;
typedef enum {FStat_Start=0,FStat_Target=1,FStat_MovingStart=2,FStat_MovingTarget=3,FStat_AjarMovingStart=4,FStat_AjarMovingTarget=5} FuncStates;
typedef enum {ACC_None=0,ACC_Std=1,ACC_Med=2,ACC_Sci=3,ACC_Admin=4,ACC_Grp1=5,ACC_Grp2=6,ACC_Grp3=7,ACC_Grp4=8,ACC_GrpA=9,ACC_GrpB=10,ACC_Stor=11,ACC_Eng=12,ACC_Maint=13,ACC_Security=14,ACC_Per1=15,ACC_Per2=16,ACC_Per3=17,ACC_Per4=18,ACC_Per5=19} AccCardType;
typedef enum {MT_None=0,MT_Walking=1,MT_Combat=2,MT_Override=3} MusicType;
typedef enum {TT_None=0,TT_Walking=1,TT_Combat=2,TT_Revive=3,TT_Death=4,TT_Cybertube=5,TT_Elevator=6,TT_Distortion=7} TrackType;
typedef enum {BloodType_None=0,BloodType_Red=1,BloodType_Yellow=2,BloodType_Green=3,BloodType_Robot=4,BloodType_Leaf=5,BloodType_Mutation=6,BloodType_GrayMutation=7 } BloodType;
typedef enum {AudioLogType_TextOnly=0,AudioLogType_Normal=1,AudioLogType_Email=2,AudioLogType_Papers=3,AudioLogType_Vmail=4,AudioLogType_Game=5} AudioLogType;
typedef enum {EnergyType_Battery=0,EnergyType_ChargeStation=1 } EnergyType;
typedef enum {Handedness_Center=0,Handedness_LH=1,Handedness_RH=2} Handedness;
typedef enum {FSTP_None=0,FSTP_Carpet=1,FSTP_Concrete=2,FSTP_GrittyCrete=3,FSTP_Grass=4,FSTP_Gravel=5,FSTP_Rock=6,FSTP_Glass=7,FSTP_Marble=8,FSTP_Metal=9,FSTP_Grate=10,FSTP_Metal2=11,FSTP_Metpanel=12,FSTP_Panel=13,FSTP_Plaster=14,FSTP_Plastic=15,
              FSTP_Plastic2=16,FSTP_Rubber=17,FSTP_Sand=18,FSTP_Squish=19,FSTP_Vent=20,FSTP_Water=21,FSTP_Wood=22,FSTP_Wood2=23} FootStepType;
typedef enum {HUDColor_White=0,HUDColor_Red=1,HUDColor_Orange=2,HUDColor_Yellow=3,HUDColor_Green=4,HUDColor_Blue=5,HUDColor_Purple=6,HUDColor_Gray=7} HUDColor;
typedef enum {ForceFieldColor_Red=0,ForceFieldColor_Green=1,ForceFieldColor_Blue=2,ForceFieldColor_Purple=3,ForceFieldColor_RedFaint=4} ForceFieldColor;
typedef enum {TabMSG_None=0,TabMSG_Search=1,TabMSG_AudioLog=2,TabMSG_Keypad=3,TabMSG_Elevator=4,TabMSG_GridPuzzle=5,TabMSG_WirePuzzle=6,TabMSG_EReader=7,TabMSG_Weapon=8,TabMSG_SystemAnalyzer=9} TabMSG;
typedef enum {PuzzleCellType_Off=0,PuzzleCellType_Standard=1,PuzzleCellType_And=2,PuzzleCellType_Bypass=3} PuzzleCellType;
typedef enum {PuzzleGridType_King=0,PuzzleGridType_Queen=1,PuzzleGridType_Knight=2,PuzzleGridType_Rook=3,PuzzleGridType_Bishop=4,PuzzleGridType_Pawn=5} PuzzleGridType;
typedef struct {V3 ctr,hExt; Quaternion rot;} ShapeBox;
typedef struct {V3 ctr; float rad;} ShapeSphere;
typedef struct {V3 tip,base; float rad;} ShapeCapsule;
enum{L_Default=(1u<<0),L_TransparentFX=(1u<<1),L_Water=(1u<<4),L_BlocksRaycast=(1u<<4),L_UI=(1u<<5),L_GunViewModel=(1u<<8),L_Geometry=(1u<<9),L_NPC=(1u<<10),L_PlayerBullets=(1u<<11),L_Player=(1u<<12),L_Corpse=(1u<<13),L_PhysObjects=(1u<<14),
     L_PlayerTriggerOnly=(1u<<16),L_Trigger=(1u<<17),L_Door=(1u<<18),L_InterDebris=(1u<<19),L_Player2=(1u<<20),L_NPCTrigger=(1u<<23),L_NPCBullet=(1u<<24),L_NPCClip=(1u<<25),L_Clip=(1u<<26),L_Automap=(1u<<27),L_Culling=(1u<<28),L_CorpseSearchable=(1u<<29)};
#define LMASK_PLAYER_COLLIDESWITH   (L_Clip|L_NPCBullet|L_Player2|L_Door|L_Trigger|L_PlayerTriggerOnly|L_Default|L_TransparentFX|L_Geometry|L_NPC)
#define LMASK_NPC_COLLIDESWITH      (L_Clip|L_NPCClip|L_PlayerBullets|L_Player2|L_Player|L_Door|L_Trigger|L_NPCTrigger|L_Default|L_TransparentFX|L_Geometry|L_NPC)
#define LMASK_NPC_SIGHT             (L_Default|L_Geometry|L_Door|L_InterDebris|L_PhysObjects|L_Player)
#define LMASK_NPC_ATTACK            (L_Default|L_Geometry|L_NPC|L_Door|L_InterDebris|L_PhysObjects|L_Player)
#define LMASK_NPC_COLLISION         (L_Default|L_TransparentFX|L_Geometry|L_NPC|L_Door|L_InterDebris|L_Player|L_Clip|L_NPCClip|L_PhysObjects)
#define LMASK_PLAYER_FROB           (L_Default|L_Geometry|L_Water|L_Door|L_InterDebris|L_PhysObjects|L_CorpseSearchable)
#define LMASK_PLAYER_TARGET_ID_FROB (L_Default|L_Geometry|L_Door|L_NPC|L_CorpseSearchable)
#define LMASK_PLAYER_ATTACK         (L_Default|L_Geometry|L_NPC|L_PlayerBullets|L_Door|L_InterDebris|L_PhysObjects|L_CorpseSearchable)
#define LMASK_EXPLOSION             (L_Default|L_Geometry|L_NPC|L_PlayerBullets|L_Door|L_InterDebris|L_PhysObjects|L_Player|L_Player2|L_CorpseSearchable)
#define LMASK_PLAYER_FEET           (L_Default|L_Geometry)
typedef struct {
    i32 InputCodeSettings[42]; u16 ScreenWidth,ScreenHeight; float ScreenCenterX,ScreenCenterY; bool Fullscreen;
    u8 FOV,Brightness,Gamma,FXAA,Shadows,Reflections,Vsync,ModelDetail,GI,SpeakerMode,Reverb,VolumeMaster,VolumeMusic,VolumeMessage,VolumeEffects,Language,DynamicMusic,Footsteps,InvertLook,InvertInventoryCycling,InvertCyberspaceLook,QuickItemPickup,
       QuickReloadWeapons,MouseSensitivity,NoShootMode,HeadBob,SSR_RES,CurrentMonitor;
} SettingsSystem;
extern SettingsSystem Sys_Settings;
typedef struct { bool god,noclip,notarget,bottomless,superoverride,fatigueCheat,redbull,consoleActive,noHUD,showLocation,showFPS,showPhys,animTest,editMode; u8 dizzyLevel; } CheatsSystem;
extern CheatsSystem Cheats;
typedef struct {
        double vmailFrameFinished,logFinished,blinkFinished,beepFinished,tickFinished,centerTabsTickFinished; i32 lastMultiMediaTabOpened,applyButtonReferenceIndex,curCenterTab,wep16index,tempSpriteIndex,count;
        u16 vmailFrame,linkedElevatorDoor,tetheredPGP,tetheredPWP,tetheredSearchable,tetheredKeypadElevator,tetheredKeypadKeycode,elevButtonSpawnIdx[8]; u8 highlightTickCount[4],beepCount,elevButtonLevelIdx[8],elevCurrentFloor;
        bool lastWeaponSideRH,lastItemSideRH,lastAutomapSideRH,lastTargetSideRH,lastDataSideRH,lastSearchSideRH,lastLogSideRH,lastLogSecondarySideRH,lastMinigameSideRH,logActive,paperLogInUse,usingObject,isBlocking,isRH,centerTabNotified[4],
             highlightStatus[4],audPaused,mouseClickHeldOverGUI,buttonsEnabled[8],buttonsDarkened[8];
        u8 vmailActive;
        AudioLogType logType; V3 objectInUsePos;
} SystemUI;
typedef struct { char stringTable[T_LOGSTR_CNT][T_LOGSTR_MAX]; u16 audioLogImagesRefIndicesLH[LOGCNT],audioLogImagesRefIndicesRH[LOGCNT]; u8 audioLogType[LOGCNT],audioLogLevelFound[LOGCNT],*file_data,*filelog_data; size_t file_size,filelog_size; } TextSystem;
extern TextSystem Sys_Text;
enum{PATCH_BERSERK=1, PATCH_DETOX=2, PATCH_GENIUS=4, PATCH_MEDI=8, PATCH_REFLEX=16, PATCH_SIGHT=32, PATCH_STAMINUP=64,
     HW_SYS=1/*System Analyzer*/,
     HW_NAV=2/*Navigation Unit*/,
     HW_ERD=4/*Datareader/EReader*/,
     HW_SNS=8/*Sensaround*/,
     HW_TID=16/*Target Identifier*/,
     HW_SHD=32/*Energy Shield*/,
     HW_BIO=64/*Biomonitor*/,
     HW_LAN=128/*Head Mounted Lantern*/,
     HW_ENV=256/*Envirosuit*/,
     HW_BST=512/*Turbo Motion Booster*/,
     HW_JET=1024/*Jump Jet Boots*/,
     HW_INF=2048/*Infrared Night Sight Enhancement*/,
     HW_COUNT=14,
     HW_SYS_IDX=0/*System Analyzer*/,
     HW_NAV_IDX=1/*Navigation Unit*/,
     HW_ERD_IDX=2/*Datareader/EReader*/,
     HW_SNS_IDX=3/*Sensaround*/,
     HW_TID_IDX=4/*Target Identifier*/,
     HW_SHD_IDX=5/*Energy Shield*/,
     HW_BIO_IDX=6/*Biomonitor*/,
     HW_LAN_IDX=7/*Head Mounted Lantern*/,
     HW_ENV_IDX=8/*Envirosuit*/,
     HW_BST_IDX=9/*Turbo Motion Booster*/,
     HW_JET_IDX=10/*Jump Jet Boots*/,
     HW_INF_IDX=11/*Infrared Night Sight Enhancement*/,
     SW_DRILL=0,SW_PULSER=1,SW_SHIELD=2,SW_TURBO=3,SW_DECOY=4,SW_RECALL=5,SW_GAMES=6,
     MINIGAME_PING=1,MINIGAME_15=2,MINIGAME_WING0=4,MINIGAME_BOTBOUNCE=8,MINIGAME_EEL_ZAPPER=16,MINIGAME_ROAD=32,MINIGAME_TRIOPTOE=64};
typedef struct { // Hw referenceIndex,ref14Index::Sys 21,0 Nav 22,1 Ere 23,2 Sen 24,3 Trg 25,4 Shi 26,5 Bio 27,6 Lan 28,7 Env 29,8 Boo 30,9 Jum 31,10 Nig 32,11
    double nitroTimeSetting,earthShakerTimeSetting,justFired,waitTilNextFire,reloadFinished,lerpStartTime,dropFinished,playerHealthTimer,berserkFinished,berserkIncTime,detoxFinished,geniusFinished,mediFinished,reflexFinishedTime,sightFinishedTime,
           leanLeftTapFinished,leanRightTapFinished,sightSideEffectFinishedTime,staminupFinishedTime,turboCyberTime,turboFinished,energyDrainTickFinished,painSoundFinished,radSoundFinished,radFXFinished,weaponDipFinished;
    float weaponEnergySetting[16],reloadLerpValue,sparqSetting,ionSetting,blasterSetting,plasmaSetting,stungunSetting,energySliderClickedTime,cyberWeaponAttackFinished,targetY,currentEnergyWeaponHeat[7],fatigue,radiated,resetAfterDeathTime,energy,
          radAdjust,initialRadiation,weaponDipLerp,currentCrouchRatio,leanTarget,leanShift,crouchingVelocity,leanVelocity;
    u32 accessCardOwned,wepAmmo[16],wepAmmoSecondary[16];
    i32 lastAddedIndex,emailCurrent,emailIndex,globalLookupIndex,weaponInventoryIndices[7],weaponInventoryAmmoIndices[7],hardwareInvCurrent/*Current slot in the general inventory (14 slots).*/,hardwareInvIndex/*Current index to the item look-up table.*/,
        generalInventoryIndexRef[14],berserkIncrement;
    i16 ladderState,weaponCurrentPending,weaponIndexPending,weaponCurrent;
    u16 hasHardware,hardwareIsActive,hardwareInvReferenceIndex[HW_COUNT],heldObjectIndex,heldObjectCustIdx,heldObjectAmmo,heldObjectAmmo2,weaponIndex,currentSearchItem,generalInvIndex,generalInvCustIdx[14],patchActive,drainJPM;
    u8 numLogsFromLevel[10],lerpUp,hasSoft,softVersions[7],hasMinigame,numweapons,currentMagazineAmount[7],currentMagazineAmount2[7],hardwareVersion[HW_COUNT],hardwareVersionSetting[HW_COUNT],grenAmmo[7],grenConstIndex[7],grenCur,generalInvCurrent,patchCur,
       patchCounts[7],cyberItemIndex;
    bool playerDead,beepDone,logPaused,hasNewEmail,hasNewNotes,isPulserNotDrill,wepLoadedWithAlternate[7],staminupActive,hasLog[134],readLog[134],justChangedWeap,overloadEnabled,recoiling,heldObjectLoadedAlternate,holdingObject,grenActive,hasNewLogs,hasNewData,radiationArea,leanResetting;
} InventorySystem;
typedef struct { float damage,penetration,offense,armorvalue,defense,impactVelocity; V3 attacknormal,hitpoint; AttType attackType; u16 owner,hitIdx; bool isOtherNPC,berserkActive; } DamageData;
typedef struct __attribute__((packed, aligned(8))) { u64 magicNumber; double thisRunTime; bool isLoading; i32 missionSplitID; } AutoSplitterData; // For use with LiveSplit or other future speedrunner utilities for doing speedruns
extern AutoSplitterData autoSplitter;
typedef struct { V3 position; Quaternion rotation; u8 fov; u16 width,height; float near,far,finished; bool visible; } CamView; // Max is 8 cam views on level 8 + 3 sensaround views = 11.
extern CamView camViews[64],levelCamViews[14][64]; extern u8 camViewCount,levelCamViewCount[14]; extern u32 camViewTextures[64],levelCamViewTextures[14][64];
typedef struct { // MUST PRESERVE ORDER TO MATCH TABLE!!
        const char* name; AttType attackType,attackType2,attackType3; float damage,damage2,damage3,range,range2,range3,health,healthForCyberNPC; PerceptionLevel perception; float disruptability,armorvalue,defense; AIMoveType moveType;
        float yawSpeed,fov,fovAttack,fovStartMovement,distToSeeBehind,sightRange,walkSpeed,runSpeed,attack1Speed,attack2Speed,attack3Speed,attack3Force,attack3Radius,timeToPain,timeBetweenPain,timeTillDead,timeToActualAttack1,timeToActualAttack2,
              timeToActualAttack3,timeBetweenAttack1,timeBetweenAttack2,timeBetweenAttack3,timeToChangeEnemy,timeIdleSFXMin,timeIdleSFXMax,timeAttack1WaitMin,timeAttack1WaitMax,timeAttack1WaitChance,timeAttack2WaitMin,timeAttack2WaitMax,timeAttack2WaitChance,
              timeAttack3WaitMin,timeAttack3WaitMax,timeAttack3WaitChance;
        int attack1ProjectileLaunchedType/*Unused*/,attack2ProjectileLaunchedType/*Unused*/,attack3ProjectileLaunchedType/*Unused*/; float projectileSpeedAttack1,projectileSpeedAttack2,projectileSpeedAttack3;
        bool hasLaserOnAttack1,hasLaserOnAttack2,hasLaserOnAttack3,explodeOnAttack3,preactivateMeleeColliders;/*Unused*/ double huntTime; float flightHeight; bool flightHeightIsPercentage,switchMaterialOnDeath;
        float hearingRange,timeForTranquilization; bool hopsOnMove; NPCType type; int projectile1Prefab,projectile2Prefab,projectile3Prefab;
} NPCTable;
extern NPCTable npcTable[NUM_AI_TYPES];
typedef struct { double clipFinished,combatImpulseFinished; bool inCombat,inZone,twoPlaying,distortion,cyberTube,elevator,levelEntry; } MusicSystem;
typedef /*FAT*/ struct  {
    u32 entflags,ioflags; u16 modelIndex,index; // constIndex for entity type, used for indexing into arrays for resource types when loading resources
    V3 forward,right,lastPosition/*used for NPC logic, not physics*/,topPoint,targetPosition,startPosition,activatedScale,direction;
    u16 texIndex,glowIndex,specIndex,normIndex,lodIndex,colMeshIndex;
    i32 cellIndex; i16 cellX,cellZ;
    u8 portalIndex,clip,numclips,texAnimClip,camView,securityThreshold,lerpUp,maxRandomItems/*[0 4] TODO*/,lastDmgType;
    FuncStates/*u8*/ funcState; BodyState/*u8*/ bodyState; 
    float shadRadius,health,cyberHealth,targetPositionY,radiation,speed,percentAjar,percentMoved,volume,timeForTranquilization,gracePeriodFinished,meleeDamageFinished,idleTime,attack1SoundTime,attack2SoundTime,attack3SoundTime,timeTillEnemyChangeFinished,
          timeTillDeadFinished,timeTillPainFinished,huntFinished,randWaitAtt1Finished,randWaitAtt2Finished,randWaitAtt3Finished,attackFinished,attack2Finished,attack3Finished,deathBurstFinished,tranquilizeFinished,wanderFinished,timeSinceMovedEnough,
          posCheckFinished,currentFrameFinished,animSwapFinished,delay,damage,itemLifeTime,minutes,seconds,randomMin,randomMax,timeInterval,cyberTimer,intervalFinished,delayFireFinished,delayResetFinished,delayFinished,tickFinished,tickTime,useFinished,
          waitBeforeClose,lasersFinished,amount,resetTime,minSecurityLevel,ajarPercentage,useTimeDelay,timeBeforeLasersOn,force,strength,offStrengthFactor,distancePaddingToTopPoint,initialBurstFinished,justUsed,timerFinished,randomItemDropChance[4];
    V3 accumulatedForce,currentDestination,lastKnownEnemyPos,targettingPosition,idealTransformForward,idealPos;    
    u16 enemy,altTexIndex,altGlowIndex,messageIndex,teleportID,targetDestinationID,recentMostActivator,countToTrigger,counter,activateSFX,lockedSFX,messageLingdex,lockedMessageLingdex,animationNum,frame,texFrame,texGlowFrame,texAnimLight,texAnimLight2,
        lookUpIndex,contents[4],custIdx[4],useableItemIndex,usableCustIdx,randomItem[4],randomItemCustIdx[4],mainSwitchMaterial,deathBurst,adjacencyIdx;
    i16 version,SFXIndex,SFXLockedIndex,textIndex,emailIndex,ammo,ammo2;
    bool searchableInUse,generateContents,dontReset,onlyOnce,ignoreSecondaryTriggers,allDone,currentTexture,useRandomTimes,active,touchEnabled,broken,stayOpen,startOpen,targetAlreadyDone,toggleLasers,targettingOnlyUnlocks,changeLayerOnOpenClose,
         despawnInstead,doSelfAfterList,destroyAfterListInsteadOfDeactivate,iceActive,forceFieldDirectionX,forceFieldDirectionY,forceFieldDirectionZ,heldObjectLoadedAlternate,changeTexOnActive,blinkTexOnActive,alternateOn,lerping,onlyTargetOnce,autoPlayEmail,
         noiseFinished,textureAnimating,textureGlowAnimating,textureAnimationStopsAtDead,texAnimInReverse,texAnimRandom,automapHidden,grenadeExplodeContact,grenadeUseTimer,grenadeUseProx,blocked,ajar;
    AttType attackType; AccCardType requiredAccessCard; BloodType bloodType; DoorState doorOpen; ForceFieldColor fieldColor; TrackType trackType; MusicType musicType; DoorState doorState; AIState currentState;
    char targetname[TARGET_STRING_LENGTH],target[TARGET_STRING_LENGTH],target2[TARGET_STRING_LENGTH],currenttarget[TARGET_STRING_LENGTH],targetIfFalse[TARGET_STRING_LENGTH],texAnimResourceFolder[TARGET_STRING_LENGTH];
} Entity; // phew what a porker of a struct, it's been a eatin!
typedef struct {
    u32 lastFrameSecCount,debugLineVertCount,shotsFired,grenadesThrown,savesScummed;
    u16 ressurections,deaths,kills,cyberkills,ressurectionActiveLevels,instCount/*Numbers of instances of entities and lights loaded (always for just the current level)*/,shd1,shd2,shd3,shd4/*ShieldGenerators on this level*/;
    float farPlane[MAX_LEVELS],damageDealt,damageReceived,timeScale,worldMin_x[MAX_LEVELS],worldMin_z[MAX_LEVELS],voxMinCtrX[MAX_LEVELS],voxMinCtrZ[MAX_LEVELS];
    double cpuTime,thisFrameTime,cpuFrameTime,lastFrameSecCountTime,debugLineFinished,shakeFinished,last_time,last_physics_time,deltaTime,current_time,screenshotTimeout,pauseRelativeTime,absoluteTime,statusTextDecayFinished,justSavedTimeStamp;
    i32 fogFac,cursorPos_x,cursorPos_y; // Separate internal cursor from system cursor.  This gets relatively pushed around by real cursor movement to give consistent platform behavior.
    V3 debugLine_start,debugLine_end,cyberspaceRecallPoint;
    u8 substeps,levelSecurity[MAX_LEVELS],startLevel,numLevels,curLev,diffCbt,diffPuz,diffMis,diffCyb,creditsPageIndex,levelCameraCount[MAX_LEVELS],levelSmallNodeCount[MAX_LEVELS],levelLargeNodeCount[MAX_LEVELS],levelCameraDestroyedCount[MAX_LEVELS],
       levelSmallNodeDestroyedCount[MAX_LEVELS],levelLargeNodeDestroyedCount[MAX_LEVELS];
    u8 lev1SecCode,lev2SecCode,lev3SecCode,lev4SecCode,lev5SecCode,lev6SecCode,currentLevel; // Which level's per-level arrays the pointers (instances, position, etc.) currently point to.  Usually curLev, but diverges briefly during cross-level target I/O.
    bool inventoryMode,levelCurrentlyLoading,introNotPlayed,paused,menuActive,gameFinished,creditsActive,decoyActive,boosterActive,uiIsBlocking,mouseClickHeldOverGUI,geniusActive;
    InventorySystem invP1; SystemUI Sys_UI; MusicSystem Sys_Music;
    Entity levelInstances[MAX_LEVELS][INSTANCE_COUNT];
    V3 levelPosition[MAX_LEVELS][INSTANCE_COUNT],levelScale[MAX_LEVELS][INSTANCE_COUNT],levelVelocity[MAX_LEVELS][INSTANCE_COUNT],levelAngularVelocity[MAX_LEVELS][INSTANCE_COUNT],
       levelColliderCenter[MAX_LEVELS][INSTANCE_COUNT]/*Offset relative to .position's global worldspace xyz location*/,levelColliderSize[MAX_LEVELS][INSTANCE_COUNT]/*x,y,z for Box, x for Sphere radius, else x,y,z for Capsule rad,height,dir(0=X,1=Y,2=Z)*/,
       levelLightsNewPosition[MAX_LEVELS][LIGHT_COUNT];
    ColliderType/*u8*/ levelCollider[MAX_LEVELS][INSTANCE_COUNT];
    Quaternion levelRotation[MAX_LEVELS][INSTANCE_COUNT];
    u32 levelLayer[MAX_LEVELS][INSTANCE_COUNT];
    float levelMass[MAX_LEVELS][INSTANCE_COUNT],levelRadius[MAX_LEVELS][INSTANCE_COUNT],levelGravity[MAX_LEVELS][INSTANCE_COUNT],levelInertiaTensor[MAX_LEVELS][INSTANCE_COUNT][6],levelInvInertiaTensor[MAX_LEVELS][INSTANCE_COUNT][6],
          levelAngularDrag[MAX_LEVELS][INSTANCE_COUNT],levelDynamicFriction[MAX_LEVELS][INSTANCE_COUNT],levelStaticFriction[MAX_LEVELS][INSTANCE_COUNT],levelBounciness[MAX_LEVELS][INSTANCE_COUNT];
    bool levelInvTnsrValid[MAX_LEVELS][INSTANCE_COUNT],levelColliding[MAX_LEVELS][INSTANCE_COUNT];
    u16 levelInstCount[MAX_LEVELS],levelLoadedLights[MAX_LEVELS];
    Light levelLights[MAX_LEVELS][LIGHT_COUNT];
    LightAnimation levelLAnims[MAX_LEVELS][LIGHT_COUNT];
    Entity* instances;
    V3* position,*scale,*velocity,*angularVelocity,*colliderCenter,*colliderSize;
    ColliderType* col;
    Quaternion* rotation;
    u32* layer,targetIOActivatorIoflags;
    float* mass,dt,*radius,*gravity,(*inertiaTensor)[6],(*invInertiaTensor)[6],*angularDrag,*dynamicFriction,*staticFriction,*bounciness,cam_pitch,cam_yaw,cam_roll;
    i32 currentMouse_dx,currentMouse_dy;
    bool *invTnsrValid,*colliding,targetIOActive;
    Light *lights; LightAnimation *lanims; V3 *lightsNewPosition; u16 loadedLights,targetIOActivatorIdx; Color fogColor[MAX_LEVELS]; Entity targetIOActivatorEntity; u8 targetIOEntryLevel;
    char playerName[27],audiologNames[LOGCNT][T_LOGSTR_MAX],audiologSubjects[LOGCNT][T_LOGSTR_MAX],audiologSenders[LOGCNT][T_LOGSTR_MAX],audioLogSpeech2Text[LOGCNT][T_LOGSTR_MAX];
} GlobalContext; // Savable complete game state data
extern GlobalContext World;
extern float modelMatrices[INSTANCE_COUNT*16]; extern u16** modelTriangles; extern u32 modelVertexCounts[MAX_MDLS]; extern u16 modelTriangleCounts[MAX_MDLS]; extern float modelBounds[MAX_MDLS]; extern u16 mdlsCnt; extern u32 globalframe;
extern float **physPos; extern u16** physTris; extern u32* physVertCounts; extern u16 uniqueCvxMeshIndices[MAX_UNIQUE_CVX_MESHES]; extern u32 uniqueCvxMeshCount;
extern u32** cvxAdjOffsets; extern u16** cvxAdjLists; extern u16 cvxAdjStart[MAX_UNIQUE_CVX_MESHES]; extern BvhNode** modelBVHNodes; extern u16** modelBVHTriOrder; extern u32 modelBVHNodeCounts[MAX_MDLS],modelBVHTriOrderCounts[MAX_MDLS];
extern u16 playerCellIdx,texCnt,cellLists[WORLDX*WORLDX][128],cellCounts[WORLDX*WORLDX]; extern const AnimationClip modelAnimationClips[MAX_ANIMS][MAX_ANIMCLIPS]; extern i32 threadCnt;
extern u32 vbos[MAX_MDLS],tbos[MAX_MDLS]; extern FHandle console_log_file; extern u32 drawCalls,vertsRendered,voxelUpdateSP,lightsID,psysSp,cellVisibleDataID,colorBufferID,texPalID,textureOffsetsID,textureSizesID,texPalOfsID;
extern u32 shadowmapIndirectionList[LIGHT_COUNT];
extern const char* sounds[SOUNDS_COUNT]; extern V3 lanternPos; extern u16 headmountedLanternLight; extern double last_mouse_x,last_mouse_y; extern Entity EDefs[MAX_ENTITIES];
extern V3 EDefscolliderCenter[MAX_ENTITIES]/*Offset relative to .position's global worldspace xyz location*/;
extern V3 EDefscolliderSize[MAX_ENTITIES]/*x,y,z for Box, x for Sphere radius, else x, y, z for Capsule radius, height, and direction (0.0f = X-Axis, 1.0f = Y-Axis, 2.0f = Z-Axis respectively, default 1.0f)*/;
extern ColliderType/*u8*/ EDefscol[MAX_ENTITIES];
extern const char* audioLogs[LOGCNT];
extern u32 EDefslayer[MAX_ENTITIES],gridCellStates[ARRSIZE];
extern float EDefsmass[MAX_ENTITIES],EDefsdynamicFriction[MAX_ENTITIES],EDefsstaticFriction[MAX_ENTITIES],EDefsbounciness[MAX_ENTITIES],EDefsangularDrag[MAX_ENTITIES],EDefsgravity[MAX_ENTITIES],berserkSeedTime,rasterPerspectiveProjection[16],
             shadowmapsPerspectiveProjection[16],lightView[LIGHT_COUNT][6][4][4],lightViewProj[LIGHT_COUNT][6][16];
typedef struct { V3 normal; float d; } FrustumPlane;
extern FrustumPlane lightFrustumPlanes[LIGHT_COUNT][6][6],playerFrustumPlanes[6];
typedef struct PngArena { u8*base,*cursor,*end; } PngArena;
extern PngArena png_arena_main;
extern bool instanceIsLODArray[INSTANCE_COUNT],doubleSidedTexture[MAX_TXRS],transparentTexture[MAX_TXRS],window_has_focus,ignore_next_mouse_delta,returnToPause,mouseMovementThisFrame,firstFrameMouselook;
extern u8 currentPlayerNameLength;
extern i8 currentMenuItem;
typedef struct { int width,height; u8* pixels; } WinSysIcon;
RaycastHit Raycast(V3,V3,float,u32); V3 ScreenPointToRay(V3,V3); u8 GetCurrentLevelSecurity(),*PngLoad(const u8*,int,int*,int*,PngArena*);
u16 AddInstance(u16,V3),SpawnDynamicObject(int,bool),GetCursorTexture(),DoorFrameFromProgress(AnimationClip,float);
double get_time(); float DoorClamp01(float),Tranquilize(u16,float,bool),TakeDamage(u16,DamageData);
void UseTargets(u16,const char*),AddForce(u16,V3,bool),CenterStatusPrint(const char * restrict fmt, ...),DebugRAM(const char*),
     play_wav(const char*,float,V3,bool),play_message(const char*),LoadLevel(u8,V3),SetLevelPointers(u8),CopyPlayerState(u8,u8),DeleteInstance(u16),MenuGoBack(),GoIntoGame(),Shake(float),TakeEnergy(float),ResetInput(),InputProcessing(),LoadAllLevels(),
     AddWireLine(V3,V3,Color),ForceInventoryMode(),ForceShootMode(),DoorSetClipFrame(u16,u8,u16),UpdateLight(u16,V3,Color3,float,float,float,float,float,Quaternion,bool,bool),UpdateLights(),ModUpdate(),InitFontAtlasses(),LoadLogTextForLanguage(u8),
     LoadTextForLanguage(u8),RenderFormattedText(i16,i16,u32,u8,float,const char* restrict,...),CullCore(),PngArenaInit(PngArena*),GenerateConvexAdjacencyLists();
char* StringFindFirstCharWithin(const char *s, char c);
AnimationClip DoorGetClip(const Entity*,u8);
bool Forward(),StrafeLeft(),Backpedal(),StrafeRight(),Jump(),JumpDown(),Crouch(),Prone(),LeanLeft(),DoubleTapLeanLeft(),LeanRight(),DoubleTapLeanRight(),Sprint(),TurnLeft(),TurnRight(),LookUp(),LookDown(),RecentLog(),Biomonitor(),Sensaround(),Lantern(),
     Shield(),Infrared(),Email(),Booster(),Jumpjets(),Attack(),Use(),Menu(),ToggleMode(),Reload(),WeaponCycUp(),WeaponCycDn(),Grenade(),GrenadeCycUp(),GrenadeCycDown(),ChangeAmmoType(),Patch(),PatchCycUp(),PatchCycDown(),/*Go*/Map()/*!*/,SwimUp(),SwimDn(),
     Console(),ScrshotPressed(),PositionVisibleFromPlayerCell(float,float),NeighborhoodInPVS(u16,u16,u8),AICheckPain(u16),ModRequestsGrayscale(),SkyIsVisible(),SkySunIsVisible();
// Synthesized Audio
typedef enum {SND_LASER_PISTOL=0,SND_LASER_RIFLE,SND_DOOR,SND_IMPACT_GLASS,SND_IMPACT_METAL,SND_EXPLOSION,SND_HISS,SND_PIPE,SND_SHIELD_HIT,SND_FOOTSTEP,SND_SAND_FOOTSTEP,SND_TAP_CASE,SND_PLASTIC_TAP,SND_SPARK_SMALL,SND_CRACKLE,SND_SINE,SND_CLINK,
              SND_BEAKER_CLINK,SND_BEAKER_THUD,SND_COUNT} SoundID;
typedef struct SynthVoice SynthVoice;
typedef float (*SynthFn)(SynthVoice*);
typedef struct SynthVoice { SynthFn fn; u32 frame,frames; float vol,pitch; V3 pos; bool positional,active; float p[4]/*preset params*/,s[8]/*generator state (extra slots vs original for richer sounds)*/; } SynthVoice;
void synth_set_room(float size, float wet);
void play_synth(SoundID id, float vol, float pitch);
// Math, Vectors, Quaternions
float sinf(float x); float tanf(float x); float cosf(float x);
#define PI 3.14159265f
#define TAU 6.2831853f
#define vabs(x) ((x) < 0 ? -(x) : (x))
#define vmin(a,b) ((a) < (b) ? (a) : (b))
#define vmax(a,b) ((a) > (b) ? (a) : (b))
#define vclamp(x,a,b) vmin(vmax(x,a),b)
#define vsqrtf(x) __builtin_sqrtf(x)
INLINE float vinvsqtf(float v) { __m128 x = (__m128){v,0.0f,0.0f,0.0f}; __m128 r=__builtin_ia32_rsqrtss(x); float y = r[0]; float x2 = v * 0.5f; return y * (1.5f - x2 * y * y); }
INLINE float vfloor(float x) { return __builtin_floorf(x); }
INLINE float vceil(float x) { return __builtin_ceilf(x); }
INLINE float vsinf(float x) { x -= TAU * vfloor(x * (1.0f / TAU)); if (x > PI) { x -= TAU; } float s = (4/PI)*x - (4/(PI*PI))*x*vabs(x); return 0.225f*(s*vabs(s) - s) + s; }
INLINE float vcosf(float x) { return vsinf(x + 1.57079632f); }
INLINE float vacosf(float x) { float negate = (x < 0.0f) ? 1.0f : 0.0f; x=vabs(x); float ret=(-0.0187293f*x + 0.0742610f)*x - 0.2121144f; ret=(ret*x + 1.5707288f)*vsqrtf(1.0f - x); ret=ret - 2.0f*negate*ret; return negate*PI + (1.0f - 2.0f*negate) * ret; }
INLINE float vtan(float x) { return tanf(x)/*vsinf(x) / vcosf(x)*/; }
INLINE float vcot(float x) { float x2 = x * x; float t = x + (x2 * x) * 0.33333333f; return 1.0f / t; }
INLINE float deg2rad(float degrees) { return degrees * (PI / 180.0f); }
INLINE float vexp2f(float x){float ip=vfloor(x); float fp=x - ip; float p=1.0f + fp*(0.69314718f + fp*(0.24022651f + fp*0.05550411f)); /*poly approx 2^fp on [0,1]*/ int ei=(int)ip + 127; u32 bits=(u32)(ei << 23); union{u32 i; float f;}u={bits}; return u.f*p;}
INLINE float vexp(float x) { return vexp2f(x * 1.4426950409f); } // 1/ln(2)
INLINE i32 clamp(i32 val, i32 min, i32 max) { return (val > max) ? max : ((val < min) ? min : val); }
INLINE float vround(float val) { return (val >= 0.0f) ? (float)(int)(val + 0.5f) : (float)(int)(val - 0.5f); }
INLINE V3 V3_AplusB(V3 a, V3 b) { return (V3){a.x + b.x, a.y + b.y, a.z + b.z}; }
INLINE V3 V3_AsubB(V3 a, V3 b) { return (V3){a.x - b.x, a.y - b.y, a.z - b.z}; }
INLINE V3 V3_ScaleByF(V3 v, float s) { return (V3){v.x * s, v.y * s, v.z * s}; }
INLINE V3 mul_v3_v3_elementwise(V3 v, V3 w) { return (V3){v.x * w.x, v.y * w.y, v.z * w.z}; }
INLINE float dot(float x1, float y1, float z1, float x2, float y2, float z2) { return x1*x2 + y1*y2 + z1*z2; }
INLINE float V3_dot(V3 a, V3 b) { return dot(a.x,a.y,a.z, b.x,b.y,b.z); }
INLINE float quat_dot(Quaternion a, Quaternion b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }
INLINE float V3_Mag(const V3 v) { return vsqrtf(V3_dot(v,v)); }
INLINE float V3_SqDist(V3 a, V3 b) { V3 d = V3_AsubB(a,b); return V3_dot(d,d); }
INLINE float V3_Dist(V3 a, V3 b) { return V3_Mag(V3_AsubB(a,b)); }
INLINE V3 V3_Cross(V3 a, V3 b) { return (V3){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }
INLINE V3 V3_Normalize(V3 v) { float len_sq = V3_dot(v,v); if (len_sq < 0.000001f){return v;} float inv_len = vinvsqtf(len_sq); return (V3){v.x * inv_len, v.y * inv_len, v.z * inv_len}; }
INLINE Quaternion quat_multiply(Quaternion q1, Quaternion q2){float aw=q1.w,ax=q1.x,ay=q1.y,az=q1.z,bw=q2.w,bx=q2.x,by=q2.y,bz=q2.z; return (Quaternion){aw*bx+ax*bw+ay*bz-az*by,aw*by-ax*bz+ay*bw+az*bx,aw*bz+ax*by-ay*bx+az*bw,aw*bw-ax*bx-ay*by-az*bz};}
INLINE V3 quat_rot_v3(Quaternion q, V3 v) {float x=q.x,y=q.y,z=q.z,w=q.w; float vx=v.x,vy=v.y,vz=v.z; float tx=2.0f*(y*vz-z*vy); float ty=2.0f*(z*vx-x*vz); float tz=2.0f*(x*vy-y*vx); return (V3){vx+w*tx+(y*tz-z*ty),vy+w*ty+(z*tx-x*tz),vz+w*tz+(x*ty-y*tx)};}
INLINE float quat_angle_deg(Quaternion a, Quaternion b) { float d = vclamp(vabs(quat_dot(a, b)), 0.0f, 1.0f); return 2.0f * vacosf(d) * (180.0f / PI); }
// Game Typechecks
INLINE u8 hardware14fromConstdex(u16 c) { return clamp(c - 21,0,14); }
INLINE bool IdxIsPortalBlockingDoor(u16 entIdx) { return (entIdx >= 496 && entIdx <= 514 && entIdx != 502 && entIdx != 505 && entIdx != 506 && entIdx != 507); }// All doors except see-through doors.
INLINE bool IdxInBounds(int c) { return (c >= 0 && c <= 760); }
INLINE bool IdxIsGeometry(int c) { return (c >= 0 && c <= 306 && c != 112 && c != 279) || c == 760; }
INLINE bool IdxIsDoor(int c) { return (c >= 496 && c < 515); }
INLINE bool IdxIsLightStaticSaveable(int c) { return c == 748; }
INLINE bool IdxIsGenericTransform(int c) { return c == 749; }
INLINE bool IdxIsNPC(int c) { return (c >= 419 && c <= 447); }
INLINE bool IdxIsCorpse(int c) { return (c >= 465 && c < 472); }
INLINE bool IdxIsHardware(int c) { return (c >= 328) && (c <= 339); }
INLINE bool IdxIsAmbient(int c) { return (c >= 621 && c <= 655); }
INLINE bool IdxIsButtonSwitch(int c) { return ((c >= 688 && c <= 692) || c == 694 || c == 695); }
INLINE bool IdxIsSearchable(int c) { return ((c >= 464 && c <= 476) || c == 530 || c == 531); }
INLINE bool IdxIsUsableObject(u16 c) { return ((c >= 307 && c <= 404) || c == 417); }
INLINE bool IdxIsAccessCard(u16 c) { return (c == 341 || c == 388 || (c >= 390 && c <= 398) || c == 417); }
INLINE bool IdxIsGenericItem(u16 c) { return (c >= 307 && c <= 312) || c == 340 || c == 342 || (c >= 359 && c <= 366) || c == 368 || c == 369 || c == 371 || (c >= 399 && c <= 401); }
INLINE bool IdxIsDynamicObject(u16 c) { return (c >= 307 && c <= 406) || c == 417 || (c >= 419 && c <= 447) || (c >= 458 && c <= 463) || (c >= 471 && c <= 476); }
INLINE bool IdxIsStaticObjectSaveable(int c) { return (c == 112 || c == 279 || (c >= 448 && c < 458) || c == 480 || c == 516 || (c >= 518 && c <= 526) || c == 530 || c == 531 || c == 546 || c == 555 || c == 594 || c == 596 || c == 598 || (c >= 600 && c < 603)  || (c >= 604 && c < 616) || (c >= 688 && c < 693) || c == 694 || c == 695 || (c >= 699 && c < 704) || (c >= 741 && c < 746)); }
INLINE bool IdxIsStaticObjectImmutable(int c) { return ((c >= 527 && c < 530) || (c >= 532 && c < 546) || (c >= 547 && c < 553) || c == 554 || (c >= 556 && c < 594) || c == 595 || c == 597 || c == 599 || c == 601 || c == 603 || (c >= 616 && c < 688) || c == 693 || c == 696 || c == 697 || c == 698 || (c >= 704 && c < 717) || c == 720 || (c >= 733 && c < 736) || (c >= 737 && c < 739) || c == 746 || c == 747 || (c >= 750 && c <= 759 && c != 755)); }
INLINE bool IdxIsWeapon(int c) { return (c >= 343 && c <= 358); }
INLINE bool IdxIsAudioLog(int c) { return c == 313; }
INLINE float UsableOrDef(float cur, float def) { u32 c = *(u32*)&cur, d = *(u32*)&def; u32 m = 0 - ((c >> 31) | ((c & 0x7FFFFFFF) == 0)); u32 r = (m & d) | (~m & c); return *(float*)&r; }
INLINE int Get16WeaponIndexFromConstIndex(int i) { return (i >= 36 && i <= 51) ? (i - 36) : -1; }
INLINE bool CurrentWeaponUsesEnergy(void) { int i = World.invP1.weaponIndex; return i==37 || i==40 || i==46 || i==50 || i==51; }
INLINE u16 GetImpactType(u16 instanceIdx){
    switch(World.instances[instanceIdx].bloodType){
        case BloodType_None:return 729; case BloodType_Red:return 724; case BloodType_Yellow:return 723; case BloodType_Green:return 722; case BloodType_Robot:return 730; case BloodType_Leaf:return 756; case BloodType_Mutation:return 757;
        case BloodType_GrayMutation:return 758;
    } return 729;
}
// Lib.c replacements and other inline helpers
INLINE void flag_setu16(u16 *flags, u16 bit, bool state) { *flags = (*flags & ~bit) | (-state & bit); }
INLINE void flag_set(u32 *flags, u32 bit, bool state) { *flags = (*flags & ~bit) | (-state & bit); }
INLINE bool BvhHasBVH(u16 m) { return (m < MAX_MDLS && modelBVHNodeCounts[m] && modelBVHNodes[m] != NULL); }
// bool cEmpty(const char c);

// Game logic inlines
INLINE void EntitySetLocked(Entity* e, bool locked) { DualLog("Unlocking entity with index %u\n",(u16)(e - World.instances)); flag_set(&e->entflags,EF_LOCKED,locked); }
INLINE void UIBlockedBySecurity(V3 tetherPoint) { (void)tetherPoint; CenterStatusPrint("%s",Sys_Text.stringTable[25]); }
INLINE void UICyberSprint(u16 textIndex) { CenterStatusPrint("%s",Sys_Text.stringTable[textIndex]); }
INLINE void UIExitCyberspace() { CenterStatusPrint("%s",Sys_Text.stringTable[601]); }
INLINE void HealthManagerHealingBed(u16 playerIdx, float amount, bool flashBed) { (void)flashBed; Entity* p = &World.instances[playerIdx]; p->health = vmin(255.0f,p->health + amount); }
INLINE void PlayerTakeDamage(u16 playerIdx, float damage) { Entity* p = &World.instances[playerIdx]; p->health -= damage; if (p->health < 0.0f) p->health = 0.0f; }
INLINE float SfxVol() { return (float)Sys_Settings.VolumeEffects / 100.0f; }
// GL
enum {GL_ARRAY_BUFFER=0x8892,GL_DEPTH_BUFFER_BIT=0x00000100,GL_READ_WRITE=0x88BA,GL_SSBO=0x90D2,GL_CULL_FACE=0x0B44,GL_BLEND=0x0BE2,GL_DEPTH_TEST=0x0B71,GL_RGB=0x1907,GL_TEXTURE0=0x84C0,GL_TEXTURE5=0x84C5,GL_COLOR_ATTACHMENT0=0x8CE0,GL_RG16F=0x822F,
      GL_TEXTURE1=0x84C1,GL_TEXTURE6=0x84C6,GL_COLOR_ATTACHMENT1=0x8CE1,GL_ELEMENT_ARRAY_BUFFER=0x8893,GL_RGB16F=0x881B,GL_TEXTURE2=0x84C2,GL_TEXTURE_2D=0x0DE1,GL_COLOR_ATTACHMENT2=0x8CE2,GL_FALSE=0,GL_RGBA=0x1908,GL_TEXTURE3=0x84C3,GL_UNSIGNED_BYTE=0x1401,
      GL_COLOR_ATTACHMENT3=0x8CE3,GL_FLOAT=0x1406,GL_RGBA32F=0x8814,GL_TEXTURE4=0x84C4,GL_FRAMEBUFFER=0x8D40,GL_COLOR_ATTACHMENT4=0x8CE4,GL_UNSIGNED_SHORT=0x1403,GL_RGBA8=0x8058,GL_COLOR_BUFFER_BIT=0x00004000,GL_STATIC_DRAW=0x88E4,GL_DYNAMIC_DRAW=0x88E8};
typedef void(*FGL_AT)(u32),(*FGL_F)(),    (*FGL_FF)(u32),  (*FGL_AS)(u32,u32),  (*FGL_VAB)(u32,u32), (*FGL_GT)(i32,u32*),   (*FGL_DA)(u32,i32,i32),     (*FGL_CC)(float,float,float,float),(*FGL_BD)(u32,size_t,const void*,u32),   (*FGL_U4F)(i32,float,float,float,float),        (*FGL_BBB)(u32,u32,u32),  *(*FGL_MBR)(u32,intptr_t,size_t,u32);
typedef void(*FGL_C)(u32), (*FGL_FL)(),   (*FGL_EVAA)(u32),(*FGL_BB)(u32,u32),  (*FGL_BT)(u32,u32),  (*FGL_U1F)(i32,float), (*FGL_BFS)(u32,u32,u32,u32),(*FGL_DE)(u32,i32,u32,const void*),(*FGL_UM4FV)(i32,i32,bool,const float*), (*FGL_BSD)(u32,intptr_t,intptr_t,const void*),  (*FGL_DB)(i32,const u32*), (*FGL_CM)(bool,bool,bool,bool);
typedef void(*FGL_CS)(u32),(*FGL_RB)(u32),(*FGL_BVA)(u32), (*FGL_GVA)(i32,u32*),(*FGL_U1I)(i32,i32), (*FGL_DC)(u32,u32,u32),(*FGL_CPIV)(u32,u32,i32*),  (*FGL_BVB)(u32,u32,intptr_t,i32),  (*FGL_RP)(i32,i32,i32,i32,u32,u32,void*),(*FGL_SS)(u32,i32,const char*const*,const i32*),(*FGL_U2UI)(i32,u32,u32),  (*FGL_CTSI2D)(u32,i32,i32,i32,i32,i32,i32,i32);
typedef void(*FGL_E)(u32), (*FGL_DF)(u32),(*FGL_LP)(u32),  (*FGL_GB)(i32,u32*), (*FGL_BFB)(u32,u32), (*FGL_GFS)(i32,u32*),  (*FGL_TPI)(u32,u32,i32),    (*FGL_FBT2D)(u32,u32,u32,u32,i32), (*FGL_GSIL)(u32,i32,i32*,char*),         (*FGL_BIT)(u32,u32,i32,bool,i32,u32,u32),       (*FGL_VP)(i32,i32,i32,i32),(*FGL_T2D)(u32,i32,i32,i32,i32,i32,u32,u32,const void*);
typedef void(*FGL_UP)(u32),(*FGL_D)(u32), (*FGL_DM)(bool), (*FGL_LW)(float),    (*FGL_GIV)(u32,i32*),(*FGL_U1UI)(i32,u32),  (*FGL_GSIV)(u32,u32,i32*),  (*FGL_VAF)(u32,i32,u32,bool,u32),  (*FGL_UM3FV)(i32,i32,bool,const float*), (*FGL_U3F)(i32,float,float,float),              (*FGL_U2F)(i32,float,float);
typedef u32(*FGL_CFBS)(u32), (*FGL_CP)(), (*FGL_GERR)(), (*FGL_CBFV)(u32,i32,const float*), (*FGL_CRS)(u32); typedef bool(*FGL_UB)(u32);
extern FGL_GB glGenBuffers; extern FGL_BB glBindBuffer; extern FGL_BD glBufferData; extern FGL_UB glUnmapBuffer; extern FGL_MBR glMapBufferRange; extern FGL_U2F glUniform2f; extern FGL_U1F glUniform1f; extern FGL_U1UI glUniform1ui; extern FGL_UP glUseProgram;
extern FGL_RP glReadPixels; extern FGL_U3F glUniform3f; extern FGL_DC glDispatchCompute; extern FGL_DA glDrawArrays; extern FGL_AT glActiveTexture; extern FGL_BVA glBindVertexArray; extern FGL_BVA glBindVertexArray; extern FGL_U1I glUniform1i;
extern FGL_E glEnable; extern FGL_U4F glUniform4f; extern FGL_BT glBindTexture; extern FGL_GERR glGetError; extern FGL_GVA glGenVertexArrays; extern FGL_VAF glVertexAttribFormat; extern FGL_VAB glVertexAttribBinding; extern FGL_EVAA glEnableVertexAttribArray;
extern FGL_BVB glBindVertexBuffer; extern FGL_BSD glBufferSubData; extern FGL_UM4FV glUniformMatrix4fv; extern FGL_DM glDepthMask; extern FGL_DF glDepthFunc; extern FGL_D glDisable; extern FGL_FL glFlush; extern FGL_F glFinish; extern FGL_LW glLineWidth;
// Input
typedef enum {JOYHAT_CENTERED=0,JOYHAT_UP=1,JOYHAT_RIGHT=2,JOYHAT_DOWN=4,JOYHAT_LEFT=8,JOYHAT_RIGHT_UP=(2|1),JOYHAT_RIGHT_DOWN=(2|4),JOYHAT_LEFT_UP=(8|1),JOYHAT_LEFT_DOWN=(8|4)} JoyHatId;
typedef enum {KEY_UNKNOWN=-1,KEY_SPACE=32,KEY_APOSTROPHE=39/* ' */,KEY_COMMA=44/* , */,KEY_MINUS=45/* - */,KEY_PERIOD=46/* . */,KEY_SLASH=47/* / */,KEY_0=48,KEY_1=49,KEY_2=50,KEY_3=51,KEY_4=52,KEY_5=53,KEY_6=54,KEY_7=55,KEY_8=56,KEY_9=57,
             KEY_SEMICOLON=59/* ; */,KEY_EQUAL=61/* = */,KEY_A=65,KEY_B=66,KEY_C=67,KEY_D=68,KEY_E=69,KEY_F=70,KEY_G=71,KEY_H=72,KEY_I=73,KEY_J=74,KEY_K=75,KEY_L=76,KEY_M=77,KEY_N=78,KEY_O=79,KEY_P=80,KEY_Q=81,KEY_R=82,KEY_S=83,KEY_T=84,KEY_U=85,KEY_V=86,
             KEY_W=87,KEY_X=88,KEY_Y=89,KEY_Z=90,KEY_LEFT_BRACKET=91/* [ */,KEY_BACKSLASH=92/* \ */,KEY_RIGHT_BRACKET=93/* ] */,KEY_GRAVE_ACCENT=96/* ` */,KEY_ESCAPE=256,KEY_ENTER=257,KEY_TAB=258,KEY_BACKSPACE=259,KEY_INSERT=260,KEY_DELETE=261,
             KEY_RIGHT=262,KEY_LEFT=263,KEY_DOWN=264,KEY_UP=265,KEY_PAGE_UP=266,KEY_PAGE_DOWN=267,KEY_HOME=268,KEY_END=269,KEY_CAPS_LOCK=280,KEY_SCROLL_LOCK=281,KEY_NUM_LOCK=282,KEY_PRINT_SCREEN=283,KEY_PAUSE=284,KEY_F1=290,KEY_F2=291,KEY_F3=292,KEY_F4=293,
             KEY_F5=294,KEY_F6=295,KEY_F7=296,KEY_F8=297,KEY_F9=298,KEY_F10=299,KEY_F11=300,KEY_F12=301,KEY_KP_0=320,KEY_KP_1=321,KEY_KP_2=322,KEY_KP_3=323,KEY_KP_4=324,KEY_KP_5=325,KEY_KP_6=326,KEY_KP_7=327,KEY_KP_8=328,KEY_KP_9=329,KEY_KP_DECIMAL=330,
             KEY_KP_DIVIDE=331,KEY_KP_MULTIPLY=332,KEY_KP_SUBTRACT=333,KEY_KP_ADD=334,KEY_KP_ENTER=335,KEY_KP_EQUAL=336,KEY_LEFT_SHIFT=340,KEY_LEFT_CONTROL=341,KEY_LEFT_ALT=342,KEY_LEFT_SUPER=343,KEY_RIGHT_SHIFT=344,KEY_RIGHT_CONTROL=345,KEY_RIGHT_ALT=346,
             KEY_RIGHT_SUPER=347,KEY_MENU=348} KeyId;
typedef enum {MOUSE_BUTTON_1=0,MOUSE_BUTTON_2=1,MOUSE_BUTTON_3=2,MOUSE_BUTTON_4=3,MOUSE_BUTTON_5=4,MOUSE_BUTTON_6=5,MOUSE_BUTTON_7=6,MOUSE_BUTTON_8=7,MOUSE_BUTTON_LEFT=0,MOUSE_BUTTON_RIGHT=1,MOUSE_BUTTON_MIDDLE=2} MouseButtonId;
typedef enum {JOYSTICK_1=0,JOYSTICK_2=1,JOYSTICK_3=2,JOYSTICK_4=3,JOYSTICK_5=4,JOYSTICK_6=5,JOYSTICK_7=6,JOYSTICK_8=7,JOYSTICK_9=8,JOYSTICK_10=9,JOYSTICK_11=10,JOYSTICK_12=11,JOYSTICK_13=12,JOYSTICK_14=13,JOYSTICK_15=14,JOYSTICK_16=15,JOYSTICK_LAST=15} JoystickId;
typedef struct { bool down,pressed,released; } KeyState; typedef struct { const char* name; int value; } InputElement;
typedef struct { double scrollDelta; KeyState keyStates[MAX_KEYS],mouseButtons[MAX_MOUSE_BUTTONS]; bool lastUse,isCapsLockOn; } InputSystem;
extern InputSystem Sys_Input;
// LibC Replacement
#define RAND_MAX 65535
void* mcpy(void *dst, const void *src, size_t n);
void* mset(void *dst, int c, size_t n);
i32 PosGetCellCoordX(float x);
i32 PosGetCellCoordZ(float z);
i32 PosGetCellCoords(float x, float z);
u32 PosGetCellCoordsP(i32 cx, i32 cz);
size_t slen(const char* s);
char* data_parser_trim(char* s);
i32 s2i32(const char *str);
bool cEmpty(const char c);
bool sEmpty(const char* a);
bool sEqual(const char* a, const char* b);
int sCompUpToLen(const char* s1, const char* s2, size_t n);
void scpy_to_a_from_b(char* a, const char* b, size_t bufsz);
void sCpy2aSubFromb(char* a, size_t subsz, const char* b, size_t bufsz);
void sCat(char* a, const char* b, size_t bufsz);
char c2Lower(const char c);
char* sFindSub(const char* s, const char* sub);
const char* StringFindLastChar(const char* str, const char c);
char* StringFindFirstCharWithin(const char *s, char c);
void double2str(char* dest, double value, int decs, size_t bufsz);
int sFormatV(char* buf, size_t bufsz, const char* f, va_list args);
int sFormat(char* buffer, size_t bufsz, const char* format, ...);
char* sUpToEndLine(char* buf, int sz, FHandle fd);
void PrintLog(const char* s, ...);
void DualLog(const char* s, ...);
void DualLogWarn(const char* s, ...);
void DualLogError(const char* s, ...);
void CenterStatusPrint(const char * restrict fmt, ...);
void BmpWrite(char const *filename, int x, int y, const void *data);
void DebugRAM(const char *context);
void Screenshot();
extern char statusText[T_BUFFER_SIZE];
extern u32 random_range_rng;
u8 random_range_u8(u8 a, u8 b);
u32 random_range_u32(u32 a, u32 b);
i32 random_range_i32(i32 a, i32 b);
float random_range(float a, float b);
u32 rand();
float lerp(float min, float max, float val);
float inverse_lerp(float min, float max, float val);
char* sLevelFileUpToEndLine(char* buf, int size);
void qsort_new(void* base, size_t nel, size_t w, cmpfun cmp);
size_t GetMaxCompressedSize(size_t srcSize);
size_t VoidSquasher(const u8* src, size_t srcSize, u8* dst, size_t dstCapacity);
size_t BlowBubblesOfVoid(const u8* src, size_t srcSize, u8* dst, size_t dstCapacity);
INLINE  int  mcmp(const void *s1, const void *s2, size_t n) { const u8 *p1 = (const u8 *)s1; const u8 *p2 = (const u8 *)s2; while (n--) { if (*p1 != *p2) {return *p1 - *p2;} p1++; p2++; } return 0; } // memcmp replacement
INLINE void* mmov(void *dst, const void *src, size_t n) { u8 *d = (u8*)dst; const u8* s = (const u8*)src; if (d < s) { while (n--) { *d++ = *s++; } } else if (d > s) { d += n; s += n; while (n--) { *--d = *--s; } } return dst; } // memmove replacement
