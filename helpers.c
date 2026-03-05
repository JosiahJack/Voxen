// helpers.c - Helper Functions for various things
#include "os.h"
#include "voxen.h"
#include <stdarg.h>
#define STBIW_UCHAR(x) (unsigned char)((x) & 0xff)

typedef void stbi_write_func(void *context, void *data, int size);

typedef struct {
   stbi_write_func *func;
   void *context;
   unsigned char buffer[64];
   int buf_used;
} stbi__write_context;

static void stbi__stdio_write(void *context, void *data, int size) {
   fwrite(data,1,size,(FILE*) context);
}

static void stbiw__writefv(stbi__write_context *s, const char *fmt, va_list v) {
   while (*fmt) {
      switch (*fmt++) {
         case ' ': break;
         case '1': { unsigned char x = STBIW_UCHAR(va_arg(v, int));
                     s->func(s->context,&x,1);
                     break; }
         case '2': { int x = va_arg(v,int);
                     unsigned char b[2];
                     b[0] = STBIW_UCHAR(x);
                     b[1] = STBIW_UCHAR(x>>8);
                     s->func(s->context,b,2);
                     break; }
         case '4': { uint32_t x = va_arg(v,int);
                     unsigned char b[4];
                     b[0]=STBIW_UCHAR(x);
                     b[1]=STBIW_UCHAR(x>>8);
                     b[2]=STBIW_UCHAR(x>>16);
                     b[3]=STBIW_UCHAR(x>>24);
                     s->func(s->context,b,4);
                     break; }
         default: return;
      }
   }
}

static void stbiw__write_flush(stbi__write_context *s) {
   if (s->buf_used) {
      s->func(s->context, &s->buffer, s->buf_used);
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
         if (expand_mono)
            stbiw__write3(s, d[0], d[0], d[0]); // monochrome bmp
         else
            stbiw__write1(s, d[0]);  // monochrome TGA
         break;
      case 4:
         if (!write_alpha) {
            // composite against pink background
            for (k = 0; k < 3; ++k)
               px[k] = bg[k] + ((d[k] - bg[k]) * d[3]) / 255;
            stbiw__write3(s, px[1 - rgb_dir], px[1], px[1 + rgb_dir]);
            break;
         }
         /* FALLTHROUGH */
      case 3:
         stbiw__write3(s, d[1 - rgb_dir], d[1], d[1 + rgb_dir]);
         break;
   }
   if (write_alpha > 0)
      stbiw__write1(s, d[comp - 1]);
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
      s->func(s->context, &zero, scanline_pad);
   }
}

static int stbiw__outfile(stbi__write_context *s, int rgb_dir, int vdir, int x, int y, int comp, int expand_mono, void *data, int alpha, int pad, const char *fmt, ...) {
   if (y < 0 || x < 0) return 0;

   va_list v;
   va_start(v, fmt);
   stbiw__writefv(s, fmt, v);
   va_end(v);
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

static int stbi_write_bmp(char const *filename, int x, int y, int comp, const void *data) {
    stbi__write_context s = { 0 };
//     OsFileHandle fd = OS_OpenWriteonly(filename);
    FILE *f = fopen(filename, "wb");
    s.func = stbi__stdio_write;
    s.context = (void*)f;
    int r = stbi_write_bmp_core(&s,x,y,comp,data);
//     OS_Close(fd);
    fclose(f);
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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void DebugRAM(const char *context) {
#ifdef DEBUG_RAM_OUTPUT
    struct mallinfo2 info = mallinfo2();
    size_t uss_bytes = 0;
    FILE *fp = fopen("/proc/self/smaps_rollup", "r");
    if (fp) {
        char line[256];
        size_t val;
        while (fgets(line, sizeof(line), fp)) {
            if (sscanf(line, "Private_Clean: %zu kB", &val) == 1)      uss_bytes += val * 1024;
            else if (sscanf(line, "Private_Dirty: %zu kB", &val) == 1) uss_bytes += val * 1024;
        }
        fclose(fp);
    } else DualLogError("Failed to open /proc/self/smaps_rollup\n");

    DualLog("Memory at %s: Heap usage %zu bytes (%zu KB | %.2f MB), USS %zu bytes (%zu KB | %.2f MB)\n",
            context, info.uordblks, info.uordblks / 1024, info.uordblks / 1024.0 / 1024.0,
            uss_bytes, uss_bytes / 1024, uss_bytes / 1024.0 / 1024.0);
#endif
}
#pragma GCC diagnostic pop

void print_bytes_no_newline(int32_t count) { DualLog("%d bytes | %f kb | %f Mb",count,(double)count / 1000.0,(double)count / 1000000.0); }

bool ConstIndexInBounds(int constdex) { return (constdex >= 0 && constdex <= 760); }
bool ConstIndexIsGeometry(int constdex) { return (constdex >= 0 && constdex <= 306 && constdex != 112 && constdex != 279) || constdex == 760; }
bool ConstIndexIsDoor(int constdex) { return (constdex >= 496 && constdex < 515); }
bool ConstIndexIsLightStaticSaveable(int constdex) { return constdex == 748; }
bool ConstIndexIsGenericTransform(int constdex) { return constdex == 749; }
bool ConstIndexIsNPC(int constdex) { return (constdex >= 419 && constdex < 448); }
bool ConstIndexIsHardware(int constdex) { return (constdex >= 328) && (constdex <= 339); }
bool ConstIndexIsAmbient(int constdex) { return (constdex >= 621 && constdex <= 655); }
bool ConstIndexIsButtonSwitch(int constdex) { return ((constdex >= 688 && constdex <= 692) || constdex == 694 || constdex == 695); }
bool ConstIndexIsDynamicObject(uint16_t constIndex) {
    return     (constIndex >= 307 && constIndex <= 404) ||  constIndex == 417 || (constIndex >= 419 && constIndex <= 428)
            || (constIndex >= 430 && constIndex <= 437) || (constIndex >= 440 && constIndex <= 442)
            || (constIndex >= 458 && constIndex <= 463) || (constIndex >= 465 && constIndex <= 476);
}

bool ConstIndexIsStaticObjectSaveable(int constdex) {
	return (constdex == 112 || constdex == 279 || (constdex >= 448 && constdex < 458) || constdex == 480 || constdex == 516
			|| (constdex >= 518 && constdex <= 526) || constdex == 530 || constdex == 531 || constdex == 546
			|| constdex == 555 || constdex == 594 || constdex == 596 || constdex == 598 || (constdex >= 600 && constdex < 603)
			|| (constdex >= 604 && constdex < 616) || (constdex >= 688 && constdex < 693) || constdex == 694 || constdex == 695
			|| (constdex >= 699 && constdex < 704) || (constdex >= 741 && constdex < 746));
}

bool ConstIndexIsStaticObjectImmutable(int constdex) {
	return ((constdex >= 527 && constdex < 530) || (constdex >= 532 && constdex < 546) || (constdex >= 547 && constdex < 553)
			|| constdex == 554 || (constdex >= 556 && constdex < 594) || constdex == 595 || constdex == 597 || constdex == 599
			|| constdex == 601 || constdex == 603 || (constdex >= 616 && constdex < 688) || constdex == 693 || constdex == 696 || constdex == 697
			|| constdex == 698 || (constdex >= 704 && constdex < 717) || constdex == 720 || (constdex >= 733 && constdex < 736)
			|| (constdex >= 737 && constdex < 739) || constdex == 746 || constdex == 747 || (constdex >= 750 && constdex <= 759 && constdex != 755));
}

bool TakeScreenshot(void);
void Screenshot(void) {
    if (!TakeScreenshot() || Sys_Global.current_time <= Sys_Global.screenshotTimeout) return;
    
    Sys_Global.screenshotTimeout = Sys_Global.current_time + 1.0; // Prevent saving more than 1 per second for sanity purposes.
    OS_MakeFolder("Screenshots");
    unsigned char* pixels = OS_AllocateRAM(NULL, Sys_Settings.ScreenWidth * Sys_Settings.ScreenHeight * 4 * sizeof(char), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, OS_INVALID_HANDLE);//malloc(Sys_Settings.ScreenWidth * Sys_Settings.ScreenHeight * 4 * sizeof(char));
    glReadPixels(0, 0, Sys_Settings.ScreenWidth, Sys_Settings.ScreenHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    char filename[96];
    snprintf(filename, sizeof(filename), "Screenshots/%.2f_x%.1f_y%.1f_z%.1f.bmp", get_time(), (double)instances[PLAYER1].position.x, (double)instances[PLAYER1].position.y, (double)instances[PLAYER1].position.z);
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

size_t GetStringLength(const char *s) {
    if (s == NULL) return 0;
    
    const char *p = s;
    while (*(p++));
    return (size_t)(p - s - 1);
}

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

uint8_t GetCurrentLevelSecurity() { return (Sys_Global.difficultyMission < 1 || Sys_Cheats.superoverride) ? 0u : Sys_Global.levelSecurity[Sys_Global.currentLevel]; }

uint16_t GetImpactType(uint16_t instanceIdx) {
    switch (instances[instanceIdx].bloodType) {
        case BloodType_None:         return 729; // SparksSmall
        case BloodType_Red:          return 724; // BloodSpurtSmall
        case BloodType_Yellow:       return 723; // BloodSpurtSmallYellow
        case BloodType_Green:        return 722; // BloodSpurtSmallGreen
        case BloodType_Robot:        return 730; // SparksSmallBlue
        case BloodType_Leaf:         return 756; // LeafBurst
        case BloodType_Mutation:     return 757; // MutationBurst
        case BloodType_GrayMutation: return 758; // GraytationBurst
    }

    return 729; // SparksSmall
}

int hardware14fromConstdex(int constdex) { return clamp(constdex - 21,0,14); }
