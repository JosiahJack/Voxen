// helpers.c - Helper Functions for various things, mostly libc avoidance
#include "os.h"
#include "voxen.h"
#define STBIW_UCHAR(x) (unsigned char)((x) & 0xff)
typedef void stbi_write_func(void *context, void *data, int size);

typedef struct {
   OsFileHandle context;
   const char *filePath;
   unsigned char buffer[64];
   int buf_used;
} stbi__write_context;

static void stbiw__writefv(stbi__write_context *s, const char *fmt, va_list v) {
   while (*fmt) {
      switch (*fmt++) {
         case ' ': break;
         case '1': { unsigned char x = STBIW_UCHAR(__builtin_va_arg(v, int));
                     OS_Write(s->context, &x, 1, s->filePath);
                     break; }
         case '2': { int x = __builtin_va_arg(v,int);
                     unsigned char b[2];
                     b[0] = STBIW_UCHAR(x);
                     b[1] = STBIW_UCHAR(x>>8);
                     OS_Write(s->context, b, 2, s->filePath);
                     break; }
         case '4': { uint32_t x = __builtin_va_arg(v,int);
                     unsigned char b[4];
                     b[0]=STBIW_UCHAR(x);
                     b[1]=STBIW_UCHAR(x>>8);
                     b[2]=STBIW_UCHAR(x>>16);
                     b[3]=STBIW_UCHAR(x>>24);
                     OS_Write(s->context, b, 4, s->filePath);
                     break; }
         default: return;
      }
   }
}

static void stbiw__write_flush(stbi__write_context *s) {
   if (s->buf_used) {
      OS_Write(s->context, &s->buffer, s->buf_used, s->filePath);
      s->buf_used = 0;
   }
}

static void stbiw__write1(stbi__write_context *s, unsigned char a) {
   if ((size_t)s->buf_used + 1 > sizeof(s->buffer)) stbiw__write_flush(s);
   s->buffer[s->buf_used++] = a;
}

static void stbiw__write3(stbi__write_context *s, unsigned char a, unsigned char b, unsigned char c) {
   int n;
   if ((size_t)s->buf_used + 3 > sizeof(s->buffer)) stbiw__write_flush(s);
   n = s->buf_used;
   s->buf_used = n+3;
   s->buffer[n+0] = a;
   s->buffer[n+1] = b;
   s->buffer[n+2] = c;
}

static void stbiw__write_pixel(stbi__write_context *s, int rgb_dir, int comp, int write_alpha, int expand_mono, unsigned char *d) {
   unsigned char bg[3] = { 255, 0, 255}, px[3];
   int k;
   if (write_alpha < 0) stbiw__write1(s, d[comp - 1]);
   switch (comp) {
      case 2: // 2 pixels = mono + alpha, alpha is written separately, so same as 1-channel case
      case 1:
         if (expand_mono) stbiw__write3(s, d[0], d[0], d[0]); // monochrome bmp
         else             stbiw__write1(s, d[0]);  // monochrome TGA
         break;
      case 4:
         if (!write_alpha) {
            // composite against pink background
            for (k = 0; k < 3; ++k) px[k] = bg[k] + ((d[k] - bg[k]) * d[3]) / 255;
            stbiw__write3(s, px[1 - rgb_dir], px[1], px[1 + rgb_dir]);
            break;
         }
         /* FALLTHROUGH */
      case 3:
         stbiw__write3(s, d[1 - rgb_dir], d[1], d[1 + rgb_dir]);
         break;
   }
   
   if (write_alpha > 0) stbiw__write1(s, d[comp - 1]);
}

static void stbiw__write_pixels(stbi__write_context *s, int rgb_dir, int vdir, int x, int y, int comp, void *data, int write_alpha, int scanline_pad, int expand_mono) {
   uint32_t zero = 0;
   int i,j, j_end;
   if (y <= 0) return;

   vdir *= -1;
   if (vdir < 0) {
      j_end = -1; j = y-1;
   } else {
      j_end =  y; j = 0;
   }

   for (; j != j_end; j += vdir) {
      for (i=0; i < x; ++i) {
         unsigned char *d = (unsigned char *) data + (j*x+i)*comp;
         stbiw__write_pixel(s, rgb_dir, comp, write_alpha, expand_mono, d);
      }
      stbiw__write_flush(s);
      OS_Write(s->context, &zero, scanline_pad, s->filePath);
   }
}

static int stbiw__outfile(stbi__write_context *s, int rgb_dir, int vdir, int x, int y, int comp, int expand_mono, void *data, int alpha, int pad, const char *fmt, ...) {
   if (y < 0 || x < 0) return 0;

   va_list v;
   __builtin_va_start(v, fmt);
   stbiw__writefv(s, fmt, v);
   __builtin_va_end(v);
   stbiw__write_pixels(s,rgb_dir,vdir,x,y,comp,data,alpha,pad, expand_mono);
   return 1;
}

static int stbi_write_bmp_core(stbi__write_context *s, int x, int y, int comp, const void *data) {
   if (comp != 4) {
      // write RGB bitmap
      int pad = (-x*3) & 3;
      return stbiw__outfile(s,-1,-1,x,y,comp,1,(void *) data,0,pad,
              "11 4 22 4" "4 44 22 444444",
              'B', 'M', 14+40+(x*3+pad)*y, 0,0, 14+40,  // file header
               40, x,y, 1,24, 0,0,0,0,0,0);             // bitmap header
   } else {
      // RGBA bitmaps need a v4 header
      // use BI_BITFIELDS mode with 32bpp and alpha mask
      // (straight BI_RGB with alpha mask doesn't work in most readers)
      return stbiw__outfile(s,-1,-1,x,y,comp,1,(void *)data,1,0,
         "11 4 22 4" "4 44 22 444444 4444 4 444 444 444 444",
         'B', 'M', 14+108+x*y*4, 0, 0, 14+108, // file header
         108, x,y, 1,32, 3,0,0,0,0,0, 0xff0000,0xff00,0xff,0xff000000u, 0, 0,0,0, 0,0,0, 0,0,0, 0,0,0); // bitmap V4 header
   }
}

int stbi_write_bmp(char const *filename, int x, int y, int comp, const void *data) {
    stbi__write_context s = { 0 };
    OsFileHandle f = OS_OpenWriteonly(filename);
//     FILE *f = fopen(filename, "wb");
    s.context = f;
    s.filePath = filename;
    int r = stbi_write_bmp_core(&s,x,y,comp,data);
    OS_Close(f);
//     fclose(f);
    return r;
}

double get_time(void) {
    #ifdef WINDOWS
        static LARGE_INTEGER frequency;
        static BOOL initialized = FALSE;
        if (!initialized) { QueryPerformanceFrequency(&frequency); initialized = TRUE; }
        LARGE_INTEGER counter;
        QueryPerformanceCounter(&counter);
        return (double)counter.QuadPart / frequency.QuadPart;
    #else
        struct { long tv_sec; long tv_nsec; } ts;
        long ret;
        __asm__ __volatile__("syscall" : "=a" (ret) : "a" (228), "D" (1), "S" (&ts) : "rcx", "r11", "memory"); // 1 == MONOTONIC
        if (ret != 0) { DualLogError("get_time failed\n"); return 0.0; }
        return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9; // Full time in seconds
    #endif
}

// Get USS aka the total RAM uniquely allocated for the process (btop shows RSS so pulls in shared libs and double counts shared RAM).
void DebugRAM(const char *context) {
#ifdef DEBUG_RAM_OUTPUT
    static void* heap_start = (void*)-1;
    if (heap_start == (void*)-1) heap_start = OS_Brk(NULL);
    void* current_brk = OS_Brk(NULL);
    size_t heap_bytes = (size_t)((char*)current_brk - (char*)heap_start);
    size_t uss_bytes = 0;
    long fd = OS_OpenReadonly("/proc/self/smaps_rollup");
    if (fd == OS_INVALID_HANDLE) { DualLogError("Failed to open /proc/self/smaps_rollup\n"); goto print_only_heap; }

    char buf[4096];
    long bytes_read = OS_Read(fd, buf, sizeof(buf)-1);
    if (bytes_read > 0) { buf[bytes_read] = '\0'; } else buf[0] = '\0';
    OS_Close(fd);
    char* p = buf;
    while (*p) {
        if (p[0]=='P' && p[1]=='r' && p[2]=='i' && p[3]=='v' && p[4]=='a' && p[5]=='t' && p[6]=='e' && p[7]=='_') {
            p += 8;
            size_t val = 0;
            if (p[0]=='C' && p[1]=='l' && p[2]=='e' && p[3]=='a' && p[4]=='n') { /* Clean */ }
            else if (p[0]=='D' && p[1]=='i' && p[2]=='r' && p[3]=='t' && p[4]=='y') { /* Dirty */ }
            else { p++; continue; }

            while (*p && *p != ':') p++;
            if (*p != ':') { p++; continue; }
            
            p++;
            while (*p == ' ' || *p == '\t') p++;
            while (*p >= '0' && *p <= '9') { val = val * 10 + (*p - '0'); p++; }
            uss_bytes += val * 1024;
        }
        
        p++;
    }

    print_only_heap:
    DualLog("Memory at %s: Heap %zu bytes (%zu KB | %.2f MB), USS %zu bytes (%zu KB | %.2f MB)\n",context,heap_bytes,heap_bytes / 1024,heap_bytes / 1024.0 / 1024.0,uss_bytes,uss_bytes / 1024,uss_bytes / 1024.0 / 1024.0);
#else
    (void)context;
#endif
}

void print_bytes_no_newline(int32_t count) { DualLog("%d bytes | %f kb | %f Mb",count,(double)count / 1000.0,(double)count / 1000000.0); }

typedef void(*PFNGLREADPIXELSPROC)(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t format, uint32_t type, void* pixels);
extern PFNGLREADPIXELSPROC glad_glReadPixels;
ENGINE_TO_MOD void Screenshot(void) {
    if (!TakeScreenshot() || Sys_Global.current_time <= Sys_Global.screenshotTimeout) return;
    
    Sys_Global.screenshotTimeout = Sys_Global.current_time + 1.0; // Prevent saving more than 1 per second for sanity purposes.
    OS_MakeFolder("Screenshots");
    unsigned char* pixels = OS_AllocateRAM(NULL, Sys_Settings.ScreenWidth * Sys_Settings.ScreenHeight * 4 * sizeof(char), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, OS_INVALID_HANDLE);//malloc(Sys_Settings.ScreenWidth * Sys_Settings.ScreenHeight * 4 * sizeof(char));
    glad_glReadPixels(0, 0, Sys_Settings.ScreenWidth, Sys_Settings.ScreenHeight, /*GL_RGBA*/ 0x1908, /*GL_UNSIGNED_BYTE*/ 0x1401, pixels);
    char filename[96]; StringFormat(filename, sizeof(filename), "Screenshots/%.2f_x%.1f_y%.1f_z%.1f.bmp", get_time(), (double)Sys_Global.instances[PLAYER1].position.x, (double)Sys_Global.instances[PLAYER1].position.y, (double)Sys_Global.instances[PLAYER1].position.z);
    if (!stbi_write_bmp(filename, Sys_Settings.ScreenWidth, Sys_Settings.ScreenHeight, 4, pixels)) DualLogError("Failed to save screenshot\n"); else DualLog("Saved screenshot %s\n", filename);
    OS_DeallocateRAM(pixels, Sys_Settings.ScreenWidth * Sys_Settings.ScreenHeight * 4 * sizeof(char));
}

uint32_t random_range_rng = 0x12345678u;
uint32_t xs32(void) { uint32_t x = random_range_rng; x ^= x << 13; x ^= x >> 17; x ^= x << 5; return random_range_rng = x ? x : 0xdeadbeefu; }
uint32_t random_u32(void) { return xs32(); }
uint8_t random_range_u8(uint8_t a, uint8_t b) {
    if (a == b) return a;
    if (a > b) { uint8_t temp = a; a = b; b = temp; }
    uint32_t r = (uint32_t)b - a; if (!r) return a; if (r == 256) return (uint8_t)xs32();
    uint32_t t = 256u - (256u % r), v; do v = (uint8_t)xs32(); while (v >= t); return a + (v % r);
}

uint32_t random_range_u32(uint32_t a, uint32_t b) {
    if (a == b) return a;
    if (a > b) { uint32_t temp = a; a = b; b = temp; }
    uint64_t r = (uint64_t)b - a; if (!r) return a;
    return a + (uint32_t)(((uint64_t)xs32() * r) >> 32);
}

int32_t random_range_i32(int32_t a, int32_t b) {
    if (a == b) return a;
    if (a > b) { int32_t temp = a; a = b; b = temp; }
    uint32_t r = (uint32_t)b - (uint32_t)a;
    return a + (int32_t)(((uint64_t)xs32() * r) >> 32);
}

float random_range(float a, float b) { return a + (b - a) * ((float)(xs32() >> 8) * (1.0f / (1U << 24))); }
double random_rangedub(double a, double b) { return a + (b - a) * ((double)(xs32() >> 8) * (1.0 / (1U << 24))); }

float lerp(float min, float max, float val) { return min + (max - min) * vclamp(val,0.0f,1.0f); }
float inverse_lerp(float min, float max, float val) { return (min == max) ? 0.0f : vclamp((val - min) / (max - min),0.0f,1.0f); }

float smooth_damp(float current, float target, float *current_velocity, float smooth_time) { 
    if (smooth_time < 0.0001f) smooth_time = 0.0001f;
    float omega = 2.0f / smooth_time;
    float x = omega * (float)Sys_Global.timeSinceLastPhysicsTick;
    float exp = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);
    float change = current - target;
    float original_to = target;
    target = current - change;
    float temp = (*current_velocity + omega * change) * (float)Sys_Global.timeSinceLastPhysicsTick;
    *current_velocity = (*current_velocity - omega * temp) * exp;
    float output = target + (change + temp) * exp;
    if ((original_to - current > 0.0f) == (output > original_to)) {
        output = original_to;
        *current_velocity = (output - original_to) / (float)Sys_Global.timeSinceLastPhysicsTick;
    }

    return output;
}

size_t GetStringLength(const char* s) {
    if (s == NULL) return 0;
    
    const char *p = s;
    while (*(p++));
    return (size_t)(p - s - 1);
}
// size_t strlen(const char* s) { return GetStringLength(s); }

char* data_parser_trim(char* s) {
    while (CharacterIsEmpty((unsigned char)*s)) s++;
    if (*s == 0) return s;

    char* e = s + GetStringLength(s) - 1;
    while (e > s && CharacterIsEmpty((unsigned char)*e)) e--;
    e[1] = 0;
    return s;
}

int32_t StringToInt(const char *str) { // atoi replacement
    while (CharacterIsEmpty(*str)) str++;
    int sign = 1;
         if (*str == '-') { sign = -1; str++; }
    else if (*str == '+') str++;

    if (*str < '0' || *str > '9') return 0;
    int64_t result = 0;
    while (*str >= '0' && *str <= '9') {
        int digit = *str - '0';
        if (result > (2147483647 - digit) / 10) return (sign == 1) ? 2147483647 : -2147483648;

        result = result * 10 + digit;
        str++;
    }

    return (int32_t)(sign * result);
}

bool CharacterIsEmpty(const char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r'; } // isspace replacement
bool StringIsEmpty(const char* a) { // C# String.IsNullOrWhiteSpace replacement
    size_t size = GetStringLength(a);
    for(size_t i=0;i < size;++i) {
        if (a[i] == '\0') break;
        if (!CharacterIsEmpty(a[i])) return false;
    }
    
    return true;
}

bool StringsAreEqual(const char* a, const char* b) { // !strcmp replacement (hated its inverted logic)
    size_t size  = GetStringLength(a);
    size_t size2 = GetStringLength(b);
    if (size != size2) return false;
    
    for (size_t i=0;i<size;++i) {
        if (a[i] != b[i]) return false;
        if (a[i] == '\0') break;
    }
    
    return true;
}
// int strcmp(const char *s1, const char *s2) { return !StringsAreEqual(s1,s2); }

bool StringsAreEqualLimitedBy(const char* a, const char* b, size_t limit) {
    if (limit == 0) return false;
    
    size_t size  = GetStringLength(a);
    size_t size2 = GetStringLength(b);
    if (size != size2) return false;
    
    for (size_t i=0;i<limit;++i) {
        if (a[i] != b[i]) return false;
        if (a[i] == '\0') break;
    }
    
    return true;
}

int StringCompareUpToLength(const char* s1, const char* s2, size_t n) { // !strncmp replacement
    if (n == 0) return 0;

    const unsigned char* p1 = (const unsigned char*)s1;
    const unsigned char* p2 = (const unsigned char*)s2;
    while (n-- > 0) {
        if (*p1 != *p2) {
            return (*p1 < *p2) ? -1 : 1;
        }
        if (*p1 == '\0') {
            break;
        }
        p1++;
        p2++;
    }
    return 0;
}

void StringCopyInto_A_From_B(char* a, const char* b, size_t bufferSize) { // strcpy replacement
    size_t size2=GetStringLength(b);
    if (size2>=bufferSize) { DualLogError("Error attempting string copy from B into A but B is bigger than buffer limit %u! A: ",bufferSize); DualLogError("%s, B: %s\n",a,b); OS_Exit(1); }
    
    for (size_t i=0;i<size2;++i) a[i] = b[i];
    a[size2] = '\0';
}

void StringCopyInto_A_SubstringFrom_B(char* a, size_t substringSize, const char* b, size_t bufferSize) { // strcpy replacement (hopefully my mnemonic "SubstringFrom_B" will help me remember substringSize comes before be in the args passed)
    if (substringSize >= bufferSize) { DualLogError("Substring too large for buffer! %u >= %u\n", substringSize, bufferSize);  OS_Exit(1); }
    
    for (size_t i=0;i<substringSize;++i) a[i] = b[i];
    a[substringSize] = '\0';
}

void StringConcatenate(char* a, const char* b, size_t bufferSize) { // strcat replacement
    size_t size  = GetStringLength(a);
    size_t size2 = GetStringLength(b);
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
            size_t h_idx = i;
            size_t n_idx = 0;
            while (haystack[h_idx] != '\0' && needle[n_idx] != '\0' && haystack[h_idx] == needle[n_idx]) {
                h_idx++;
                n_idx++;
            }

            if (needle[n_idx] == '\0') return (char*)&haystack[i];
        }
    }

    return NULL; // No match found
}

const char* StringFindLastChar(const char* str, const char c) { // strrchr replacement
    const char* lastSeen = NULL;
    do {
        if (*str == c) lastSeen = str;
    } while (*str++);
    return lastSeen;
}

char* StringFindFirstCharWithin(const char *s, char c) { // strchr replacement
    char* stringwalker = (char*)s;
    while (*stringwalker != c) {
        if (!*stringwalker) return NULL;
        stringwalker++;
    }
    return stringwalker;
}

char* StringReturnUpToDelimiterAndLopOffAndShiftOriginal(char* str, const char delim, char** saveptr) { // strtok_r replacement
    if (str) *saveptr = str;

    char* token = *saveptr;
    if (!token || *token == '\0') {
        *saveptr = NULL;
        return NULL;
    }

    char* p = token;
    while (*p != '\0' && *p != delim) {
        p++;
    }

    if (*p == delim) {
        *p = '\0';
        *saveptr = p + 1;
    } else {
        // End of string reached — no more tokens after this
        *saveptr = NULL;
    }

    return token;
}

void DoubleToStringFixed(char* dest, double value, int decimalPlaces, size_t bufferSize) {
    if (decimalPlaces < 0 || decimalPlaces > 9) { DualLogError("DoubleToStringFixed: decimalPlaces out of range\n"); OS_Exit(1); }

    if (value < 0.0) {
        if (bufferSize < 2) OS_Exit(1);
        
        *dest++ = '-';
        bufferSize--;
        value = -value;
    }

    uint64_t intPart = (uint64_t)value;
    char temp[32];
    size_t len = 0;
    if (intPart == 0) temp[len++] = '0';
    else {
        while (intPart > 0) {
            temp[len++] = '0' + (intPart % 10);
            intPart /= 10;
        }
    }

    if (len >= bufferSize) OS_Exit(1);
    for (size_t i = 0; i < len; ++i) dest[i] = temp[len - 1 - i];
    dest += len;
    bufferSize -= len;
    if (decimalPlaces == 0) { *dest = '\0'; return; }
    if (bufferSize < 1) OS_Exit(1);
    
    *dest++ = '.';
    bufferSize--;
    double frac = value - (uint64_t)value;
    double scale = 1.0;
    for (int i = 0; i < decimalPlaces; ++i) scale *= 10.0;
    uint64_t fracPart = (uint64_t)(frac * scale + 0.5);
    for (int i = decimalPlaces - 1; i >= 0; --i) {
        if (bufferSize < 1) OS_Exit(1);
        dest[i] = '0' + (fracPart % 10);
        fracPart /= 10;
    }

    dest[decimalPlaces] = '\0';
}

void StringAppendLiteral(char* dest, const char* literal, size_t bufferSize) {
    size_t curLen = GetStringLength(dest);
    size_t litLen = GetStringLength(literal);
    if (curLen + litLen >= bufferSize) { DualLogError("StringAppendLiteral overflow\n"); OS_Exit(1); }
    
    for (size_t i = 0; i < litLen; ++i) dest[curLen + i] = literal[i];
    dest[curLen + litLen] = '\0';
}

ENGINE_TO_MOD int StringFormatV(char* buffer, size_t bufferSize, const char* format, va_list args) { // vsnprintf replacement
    if (bufferSize == 0) return 0;

    size_t pos = 0;
    const char* f = format;
    while (*f && pos < bufferSize - 1) {
        if (*f != '%') { buffer[pos++] = *f++; continue; } else f++; // skip the '%'
        int decimals = 9;
        if (*f == '.') {
            f++;
            if (*f == '1') { decimals = 1; f++; }
            else if (*f == '2') { decimals = 2; f++; }
            else if (*f == '3') { decimals = 3; f++; }
            else if (*f == '4') { decimals = 4; f++; }
            else if (*f == '5') { decimals = 5; f++; }
            else if (*f == '6') { decimals = 6; f++; }
        }

        switch (*f) {
        case 's':
            {
                const char* s = __builtin_va_arg(args, const char*);
                size_t len = GetStringLength(s);
                if (pos + len >= bufferSize) len = bufferSize - pos - 1;
                for (size_t i = 0; i < len; ++i) buffer[pos++] = s[i];
            }
            break;
        case 'd':
        case 'i':
            {
                int val = __builtin_va_arg(args, int);
                if (val < 0) {
                    if (pos < bufferSize - 1) buffer[pos++] = '-';
                    val = -val;
                }
                char num[32];
                int i = 0;
                do { num[i++] = '0' + (val % 10); val /= 10; } while (val);
                while (i-- > 0 && pos < bufferSize - 1) buffer[pos++] = num[i];
            }
            break;
        case 'u':
            {
                unsigned int val = __builtin_va_arg(args, unsigned int);
                char num[32];
                int i = 0;
                do { num[i++] = '0' + (val % 10); val /= 10; } while (val);
                while (i-- > 0 && pos < bufferSize - 1) buffer[pos++] = num[i];
            }
            break;
        case 'f':
            {
                double val = __builtin_va_arg(args, double);
                char num[64];
                DoubleToStringFixed(num, val, decimals, sizeof(num));
                size_t len = GetStringLength(num);
                if (pos + len >= bufferSize) len = bufferSize - pos - 1;
                for (size_t i = 0; i < len; ++i) buffer[pos++] = num[i];
            }
            break;
        case '%':
            if (pos < bufferSize - 1) buffer[pos++] = '%';
            break;
        default:
            if (pos < bufferSize - 1) buffer[pos++] = '%';
            if (pos < bufferSize - 1) buffer[pos++] = *f;
            break;
        }
        f++;
    }

    buffer[pos] = '\0';
    return (int)pos;
}

ENGINE_TO_MOD int StringFormat(char* buffer, size_t bufferSize, const char* format, ...) { // snprintf replacement
    va_list args;
    __builtin_va_start(args, format);
    int ret = StringFormatV(buffer, bufferSize, format, args);
    __builtin_va_end(args);
    return ret;
}

ENGINE_TO_MOD bool PositionVisibleFromPlayerCell(float x, float z) {
    int32_t subIdx = PosGetCellCoords(x,z);
    int cellIdx = (playerCellIdx * ARRSIZE);
    int flat_idx = cellIdx + subIdx;
    return (get_cull_bit(precomputedVisibleCellsFromHere,flat_idx));
}

char* GetNextStringUpToNewlineOrEOF(char* buf, int size, OsFileHandle fd) { // fgets replacement, not thread safe but we don't do multithreading
    if (size <= 1 || buf == NULL) return NULL;

    char* p = buf;
    int remaining = size - 1;
    static int pos = 0;
    static int end = 0;
    static char buffer[4096];
    while (remaining > 0) {
        if (pos >= end) {
            long n = OS_Read(fd,buffer,sizeof(buffer));
            if (n <= 0 && p == buf) return NULL;

            pos = 0;
            end = (int)n;
        }

        while (remaining > 0 && pos < end) {
            char c = buffer[pos++];
            *p++ = c;
            remaining--;

            if (c == '\n') goto done;
        }
    }

    done:
    *p = '\0';
    return buf;
}

void FilePrintString(OsFileHandle f, const char* fmt, ...) {
    va_list args; __builtin_va_start(args,fmt);
    char buf[128]; va_list copy;
    __builtin_va_copy(copy,args);
    StringFormatV(buf,sizeof(buf),fmt,copy);
    __builtin_va_end(copy);
    OS_RawWrite(f,buf,GetStringLength(buf));
    __builtin_va_end(args);
}
