// lib.h - Declarations for custom C library
#pragma once
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

