// types.h - The main data types used everywhere
#pragma once
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
