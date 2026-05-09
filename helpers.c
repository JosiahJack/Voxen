// helpers.c - Helper Functions for various things, mostly libc avoidance
static void* CopyMemoryFromBtoAForNBytes(void *dst, const void *src, size_t n) { unsigned char *d=(unsigned char *)dst; const unsigned char *s=(const unsigned char *)src; while (n--) {*d++=*s++;} return dst; } // memcpy replacement
void stbi_write_bmp(char const *filename, int x, int y, const void *data) {
    OsFileHandle f = OS_OpenWriteonly(filename);
    if (f == OS_INVALID_HANDLE) { DualLogError("Failed to open %s for writing\n", filename); return; }

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
    if (fd == OS_INVALID_HANDLE) { DualLogError("Failed to open /proc/self/smaps_rollup\n"); return; }

    char buf[4096]; long bytes_read = OS_Read(fd,buf,sizeof(buf)-1);
    if (bytes_read > 0) { buf[bytes_read] = '\0'; } else buf[0] = '\0';
    OS_Close(fd); char* p = buf;
    while (*p) {
        if (p[0]=='P'&&p[1]=='r'&&p[2]=='i'&&p[3]=='v'&&p[4]=='a'&&p[5]=='t'&&p[6]=='e'&&p[7]=='_') {
            p += 8;
            size_t val = 0;
            if (p[0]=='C'&&p[1]=='l'&&p[2]=='e'&&p[3]=='a'&&p[4]=='n') { /* Clean */ }
            else if (p[0]=='D'&&p[1]=='i'&&p[2]=='r'&&p[3]=='t'&&p[4]=='y') { /* Dirty */ }
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

    DualLog("Memory at %s: Heap %u bytes (%u KB | %.2f MB), USS %u bytes (%u KB | %.2f MB)\n",context,heap_bytes,heap_bytes / 1024,heap_bytes / 1024.0 / 1024.0,uss_bytes,uss_bytes / 1024,uss_bytes / 1024.0 / 1024.0);
#else
    (void)context;
#endif
}

ENGINE_TO_MOD void Screenshot(void) {
    if (!TakeScreenshot() || Sys_Global.current_time <= Sys_Global.screenshotTimeout) return;
    
    Sys_Global.screenshotTimeout = Sys_Global.current_time + 1.0; // Prevent saving more than 1 per second for sanity purposes.
    OS_MakeFolder("Screenshots"); u16 w = Sys_Settings.ScreenWidth, h = Sys_Settings.ScreenHeight;
    unsigned char* pixels = OS_Alloc(w * h * 4 * sizeof(char));
    glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,pixels);
    Vector3 p = Sys_Global.instances[PLAYER1].position;
    char filename[96]; StringFormat(filename,sizeof(filename),"Screenshots/%.2f_x%.1f_y%.1f_z%.1f.bmp",get_time(),p.x,p.y,p.z);
    stbi_write_bmp(filename,w,h,pixels); DualLog("Saved screenshot %s\n",filename);
    OS_DeallocateRAM(pixels,w * h * 4 * sizeof(char));
}

u32 random_range_rng = 0x12345678u;
u32 xs32(void) { u32 x = random_range_rng; x ^= x << 13; x ^= x >> 17; x ^= x << 5; return random_range_rng = x ? x : 0xdeadbeefu; }
u32 random_u32(void) { return xs32(); }
u8 random_range_u8(u8 a, u8 b) {
    if (a == b) return a;
    if (a > b) { u8 temp = a; a = b; b = temp; }
    u32 r = (u32)b - a; if (!r) return a; if (r == 256) return (u8)xs32();
    u32 t = 256u - (256u % r), v; do v = (u8)xs32(); while (v >= t); return a + (v % r);
}

u32 random_range_u32(u32 a, u32 b) {
    if (a == b) return a;
    if (a > b) { u32 temp = a; a = b; b = temp; }
    u64 r = (u64)b - a; if (!r) return a;
    return a + (u32)(((u64)xs32() * r) >> 32);
}

i32 random_range_i32(i32 a, i32 b) {
    if (a == b) return a;
    if (a > b) { i32 temp = a; a = b; b = temp; }
    u32 r = (u32)b - (u32)a;
    return a + (i32)(((u64)xs32() * r) >> 32);
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

ENGINE_TO_MOD size_t GetStringLength(const char* s) { // strlen replacement
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

i32 StringToInt(const char *str) { // atoi replacement
    while (CharacterIsEmpty(*str)) str++;
    int sign = 1;
         if (*str == '-') { sign = -1; str++; }
    else if (*str == '+') str++;

    if (*str < '0' || *str > '9') return 0;
    i64 result = 0;
    while (*str >= '0' && *str <= '9') {
        int digit = *str - '0';
        if (result > (2147483647 - digit) / 10) return (sign == 1) ? 2147483647 : -2147483648;

        result = result * 10 + digit;
        str++;
    }

    return (i32)(sign * result);
}

ENGINE_TO_MOD bool CharacterIsEmpty(const char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r'; } // isspace replacement
bool StringIsEmpty(const char* a) { // C# String.IsNullOrWhiteSpace replacement
    size_t size = GetStringLength(a);
    for(size_t i=0;i < size;++i) {
        if (a[i] == '\0') break;
        if (!CharacterIsEmpty(a[i])) return false;
    }
    
    return true;
}

bool __attribute__((noinline)) StringsEqual(const char* a, const char* b) { // !strcmp replacement (hated its inverted logic)
    size_t size = GetStringLength(a), size2 = GetStringLength(b); if (size != size2) return false;
    
    for (size_t i=0;i<size;++i) {
        if (a[i] != b[i]) return false;
        if (a[i] == '\0') break;
    }
    
    return true;
}

int StringCompareUpToLength(const char* s1, const char* s2, size_t n) { const unsigned char *p1 = (const unsigned char*)s1, *p2 = (const unsigned char*)s2; while (n-- > 0) { if (*p1 != *p2) {return (*p1 < *p2) ? -1 : 1;}  if (*p1 == '\0') {break;} p1++; p2++; } return 0; } // !strncmp replacement (yes inverted for sanity)
void StringCopyInto_A_From_B(char* a, const char* b, size_t bufferSize) { // strcpy replacement
    size_t size2=GetStringLength(b); if (size2>=bufferSize) { DualLogError("Error attempting string copy from B into A but B is bigger than buffer limit %u! A: ",bufferSize); DualLogError("%s, B: %s\n",a,b); OS_Exit(1); }
    
    for (size_t i=0;i<size2;++i) a[i] = b[i];
    a[size2] = '\0';
}

void StringCopyInto_A_SubstringFrom_B(char* a, size_t substringSize, const char* b, size_t bufferSize) { // strncpy replacement (hopefully my mnemonic "SubstringFrom_B" will help me remember substringSize comes before be in the args passed)
    if (substringSize >= bufferSize) { DualLogError("Substring too large for buffer! %u >= %u\n", substringSize, bufferSize);  OS_Exit(1); }
    
    bool reachedEndB = false;
    for (size_t i= 0;i<substringSize;++i) {
        if (!reachedEndB && b[i] == '\0') reachedEndB = true;
        if (reachedEndB) a[i] = '\0';
        else a[i] = b[i]; // Normal copy
    }
    
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

ENGINE_TO_MOD char* StringFindFirstCharWithin(const char *s, char c) { // strchr replacement
    char* stringwalker = (char*)s;
    while (*stringwalker != c) {
        if (!*stringwalker) return NULL;
        stringwalker++;
    }
    return stringwalker;
}

void DoubleToStringFixed(char* dest, double value, int decimalPlaces, size_t bufferSize) {
    if (decimalPlaces < 0 || decimalPlaces > 9) { DualLogError("DoubleToStringFixed: decimalPlaces out of range\n"); OS_Exit(1); }

    if (value < 0.0) {
        if (bufferSize < 2) OS_Exit(1);
        
        *dest++ = '-';
        bufferSize--;
        value = -value;
    }

    u64 intPart = (u64)value;
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
    double frac = value - (u64)value;
    double scale = 1.0;
    for (int i = 0; i < decimalPlaces; ++i) scale *= 10.0;
    u64 fracPart = (u64)(frac * scale + 0.5);
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
                const char* s = __builtin_va_arg(args, const char*);
                size_t len = GetStringLength(s);
                if (pos + len >= bufferSize) len = bufferSize - pos - 1;
                for (size_t i = 0; i < len; ++i) buffer[pos++] = s[i];
            } break;
            case 'f': {
                double val = __builtin_va_arg(args, double);
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
char* GetNextStringUpToNewlineOrEOF(char* buf, int size, OsFileHandle fd) { // fgets replacement, not thread safe but we don't do multithreading
    if (size <= 1 || buf == NULL) return NULL;

    char* p = buf;
    int remaining = size - 1;
    static int pos = 0;
    static int end = 0;
    static char buffer[4096];
    while (remaining > 0) {
        if (pos >= end) {
            long n = OS_Read(fd,buffer,sizeof(buffer)); if (n <= 0 && p == buf) return NULL;
            pos = 0; end = (int)n;
        }

        while (remaining > 0 && pos < end) {
            char c = buffer[pos++]; *p++ = c; remaining--; if (c == '\n') goto done;
        }
    }

    done:
    *p = '\0';
    return buf;
}

extern OsFileHandle levelFileHandle;
ENGINE_TO_MOD char* GetLevelFileNextStringUpToNewlineOrEOF(char* buf, int size) { return GetNextStringUpToNewlineOrEOF(buf,size,levelFileHandle); }
void FilePrintString(OsFileHandle f, const char* fmt, ...) {
    va_list args; __builtin_va_start(args,fmt);
    char buf[128]; va_list copy;
    __builtin_va_copy(copy,args);
    StringFormatV(buf,sizeof(buf),fmt,copy);
    __builtin_va_end(copy);
    OS_RawWrite(f,buf,GetStringLength(buf));
    __builtin_va_end(args);
}

ENGINE_TO_MOD Vector3 GetEntityLocalSpawnPointFromUnrotatedOffsetVector(Entity* originator, Vector3 offsetFromOriginator) {
    Vector3 scaledOfs = mul_v3_v3_elementwise(offsetFromOriginator, originator->scale);
    Vector3 rotatedOfs = quat_rotate_vector(originator->rotation, scaledOfs);
    Vector3 result = Vector3_A_plus_B(originator->position, rotatedOfs);
    return result;
}
