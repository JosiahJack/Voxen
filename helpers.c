// helpers.c - Helper Functions for various things, mostly libc avoidance
void* MemSetToVForNBytes(void *dst, int c, size_t n) { unsigned char *p=(unsigned char *)dst; unsigned char v=(unsigned char)c; while (n--) {*p++=v;} return dst; } // memset replacement
void* CopyMemoryFromBtoAForNBytes(void *dst, const void *src, size_t n) { unsigned char *d=(unsigned char *)dst; const unsigned char *s=(const unsigned char *)src; while (n--) {*d++=*s++;} return dst; } // memcpy replacement
void stbi_write_bmp(char const *filename, int x, int y, const void *data) {
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
double get_time(void) { static LARGE_INTEGER frequency,counter; static i32 init=0; if (!init) { QueryPerformanceFrequency(&frequency); init=1; } QueryPerformanceCounter(&counter); return (double)counter.QuadPart / frequency.QuadPart; }
#else
double get_time(void) { struct {i64 s,ns;} ts; i64 ret; __asm__ __volatile__("syscall":"=a"(ret):"a"(228),"D"(1),"S"(&ts):"rcx","r11","memory"); if (ret != 0) {return 0.0;} return (double)ts.s + (double)ts.ns * 1e-9; } // Full time in seconds, 1 for MONOTONIC, Note that using clock_gettime wasn't any better for performance.
#endif

// Get USS aka the total RAM uniquely allocated for the process (btop shows RSS so pulls in shared libs and double counts shared RAM).
void DebugRAM(const char *context) {
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
    
    const char *p = s; while (*(p++));
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

bool StringsEqual(const char* a, const char* b) { // !strcmp replacement (hated its inverted logic)
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
char* GetNextStringUpToNewlineOrEOF(char* buf, int size, FHandle fd) { // fgets replacement, not thread safe but we don't do multithreading
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

extern FHandle levelFileHandle;
ENGINE_TO_MOD char* GetLevelFileNextStringUpToNewlineOrEOF(char* buf, int size) { return GetNextStringUpToNewlineOrEOF(buf,size,levelFileHandle); }
ENGINE_TO_MOD Vector3 GetEntityLocalSpawnPointFromUnrotatedOffsetVector(Entity* originator, Vector3 offsetFromOriginator) {
    Vector3 scaledOfs = mul_v3_v3_elementwise(offsetFromOriginator, originator->scale);
    Vector3 rotatedOfs = quat_rotate_vector(originator->rotation, scaledOfs);
    Vector3 result = Vector3_A_plus_B(originator->position, rotatedOfs);
    return result;
}

static inline int pntz(size_t p[2]) {
    if (p[0] != 1) return __builtin_ctzll(p[0] - 1);
    if (p[1])      return 8*sizeof(size_t) + __builtin_ctzll(p[1]);
    return 0;
}
static inline void shl(size_t p[2], int n) {
    if (n >= 8*(int)sizeof(size_t)) { p[1]=p[0]; p[0]=0; n-=8*sizeof(size_t); if(!n)return; }
    p[1]=(p[1]<<n)|(p[0]>>(8*sizeof(size_t)-n)); p[0]<<=n;
}
static inline void shr(size_t p[2], int n) {
    if (n >= 8*(int)sizeof(size_t)) { p[0]=p[1]; p[1]=0; n-=8*sizeof(size_t); if(!n)return; }
    p[0]=(p[0]>>n)|(p[1]<<(8*sizeof(size_t)-n)); p[1]>>=n;
}
static void cycle(size_t w, unsigned char* ar[], int n) {
    unsigned char tmp[256]; size_t l;
    if (n<2) return;
    ar[n]=tmp;
    while (w) { l=w<256?w:256; CopyMemoryFromBtoAForNBytes(ar[n],ar[0],l); for(int i=0;i<n;i++){CopyMemoryFromBtoAForNBytes(ar[i],ar[i+1],l);ar[i]+=l;} w-=l; }
}
#define AL (16*sizeof(size_t))
static void sift(unsigned char* head, size_t w, cmpfun_r cmp, void* arg, int ps, size_t lp[]) {
    unsigned char* ar[AL]; int i=1; ar[0]=head;
    while (ps>1) {
        unsigned char* rt=head-w, *lf=rt-lp[ps-2];
        if (cmp(ar[0],lf,arg)>=0 && cmp(ar[0],rt,arg)>=0) break;
        if (cmp(lf,rt,arg)>=0) { ar[i++&(AL-1)]=lf; head=lf; ps--; } else { ar[i++&(AL-1)]=rt; head=rt; ps-=2; }
    }
    cycle(w,ar,i&(AL-1));
}
static void trinkle(unsigned char* head, size_t w, cmpfun_r cmp, void* arg, size_t pp[2], int ps, int trusty, size_t lp[]) {
    unsigned char* ar[AL]; int i=1; ar[0]=head;
    size_t p[2]={pp[0],pp[1]};
    while (p[0]!=1||p[1]!=0) {
        unsigned char* ss=head-lp[ps];
        if (cmp(ss,ar[0],arg)<=0) break;
        if (!trusty&&ps>1) { unsigned char* rt=head-w,*lf=rt-lp[ps-2]; if(cmp(rt,ss,arg)>=0||cmp(lf,ss,arg)>=0) break; }
        ar[i++&(AL-1)]=ss; head=ss; int t=pntz(p); shr(p,t); ps+=t; trusty=0;
    }
    if (!trusty) { cycle(w,ar,i&(AL-1)); sift(head,w,cmp,arg,ps,lp); }
}
void qsort_new(void* base, size_t nel, size_t w, cmpfun cmp) {
    // wrap 2-arg cmp into 3-arg inline — no separate wrapper_cmp needed
    // by casting: cmpfun and cmpfun_r differ only in the void* arg which we pass as NULL
    // and a well-behaved cmp never touches it, so this cast is safe in practice
    size_t lp[12*sizeof(size_t)], p[2]={1,0}, size=w*nel; int ps=1,trail;
    if (!size) return;
    unsigned char* head=base, *high=head+size-w;
    for (size_t i=2; lp[0]=lp[1]=w, (lp[i]=lp[i-2]+lp[i-1]+w)<size; i++);
    cmpfun_r cmp_r=(cmpfun_r)(void*)cmp; void* arg=NULL;
    while (head<high) {
        if ((p[0]&3)==3)                      { sift(head,w,cmp_r,arg,ps,lp); shr(p,2); ps+=2; }
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

typedef u16 half;
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
    float f; CopyMemoryFromBtoAForNBytes(&f,&out,4);
    return f;
}
