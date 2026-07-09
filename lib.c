// lib.c - LibC replacement functions and other misc helpers.
static i32 PosGetCellCoordX(float x) { return (u16)clamp((i32)vfloor((x - World.worldMin_x[World.curLev] + CELLXHALF) / CELL_SIZE), 0, WORLDX_0BASED); }
static i32 PosGetCellCoordZ(float z) { return (u16)clamp((i32)vfloor((z - World.worldMin_z[World.curLev] + CELLXHALF) / CELL_SIZE), 0, WORLDX_0BASED); }
size_t slen(const char* s) { if (s == NULL) {return 0;} const char *p=s; while (*(p++)); return (size_t)(p - s - 1); } // strlen replacement
char* data_parser_trim(char* s) { while(cEmpty((u8)*s)){s++;} if (*s == 0){return s;} char* e=s + slen(s) - 1; while(e > s && cEmpty((u8)*e)){e--;} e[1]=0; return s; }
i32 s2i32(const char *str) { // atoi replacement, needed separately from fast_atoi for user console input
    while (cEmpty(*str)) {str++;} int sign = 1; if (*str == '-') { sign = -1; str++; } else if (*str == '+') {str++;} if (*str < '0' || *str > '9') return 0;
    i64 result = 0;
    while (*str >= '0' && *str <= '9') { int digit = *str - '0'; if (result > (2147483647 - digit) / 10) {return (sign == 1) ? 2147483647 : -2147483648;} result = result * 10 + digit; str++; }
    return (i32)(sign * result);
}

bool cEmpty(const char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r'; } // isspace replacement
bool sEmpty(const char* a) { if (!a || *a == '\0') {return true;} for (size_t i=0;a[i]!='\0';++i) { if(!cEmpty(a[i])){return false;} } return true; } // C# String.IsNullOrWhiteSpace replacement
bool sEqual(const char* a, const char* b) { size_t sz = slen(a); if(sz != slen(b)) {return false;} for(size_t i=0;i<sz;++i) { if(a[i] != b[i]){return false;} if(a[i] == '\0'){break;} } return true; } // !strcmp replacement (hated its inverted logic)
int sCompUpToLen(const char* s1, const char* s2, size_t n) { const u8 *p1 = (const u8*)s1, *p2 = (const u8*)s2; while (n-- > 0) { if (*p1 != *p2) {return (*p1 < *p2) ? -1 : 1;}  if (*p1 == '\0') {break;} p1++; p2++; } return 0; } // !strncmp replacement (yes inverted for sanity)
void scpy_to_a_from_b(char* a, const char* b, size_t bufsz) { size_t szb=slen(b); if (szb>=bufsz) { DualLogError("scpy_to_a_from_b: B bigger than buffer\n"); OS_Exit(1); } for(size_t i=0;i<szb;++i){a[i]=b[i];} a[szb] = '\0'; } // strcpy replacement
void sCpy2aSubFromb(char* a, size_t subsz, const char* b, size_t bufsz) { if (subsz >= bufsz) { DualLogError("sCpy2aSubFromb substring overflow!\n"); OS_Exit(1); } bool ended=0; for(size_t i= 0;i<subsz;++i){ if(!ended && b[i] == '\0'){ended=1;} if(ended){a[i]='\0';}else{a[i]=b[i];} } a[subsz]='\0'; } // strncpy replacement (hopefully my mnemonic "a_subfrom_b" will help)
void sCat(char* a, const char* b, size_t bufsz) { size_t sza=slen(a), szb=slen(b); if (sza + szb >= bufsz) { DualLogError("sCat overflow\n"); } char* dest = a + sza; for(size_t i=0;i<szb;++i){dest[i]=b[i];} dest[szb]='\0'; } // strcat replacement
char c2Lower(const char c) { return c + ((c >= 'A' && c <= 'Z') ? 32 : 0); } // If uppercase 'A'-'Z' (65-90), +32 into 'a'-'z' (97-122)
char* sFindSub(const char* s, const char* sub) { if (sub[0] == '\0') {return (char*)s;} for (size_t i = 0; s[i] != '\0'; ++i) { if (s[i] == sub[0]/*Only look if first matches*/) { size_t si=i, subi=0; while (s[si] != '\0' && sub[subi] != '\0' && s[si] == sub[subi]){si++; subi++;} if(sub[subi] == '\0'){return (char*)&s[i];} } } return NULL;/*No match*/ } // strstr replacement
const char* StringFindLastChar(const char* str, const char c) { const char* lastSeen = NULL; do { if (*str == c) lastSeen = str; } while (*str++); return lastSeen; } // strrchr replacement
char* StringFindFirstCharWithin(const char *s, char c) { char* stringwalker = (char*)s; while (*stringwalker != c) { if (!*stringwalker) {return NULL;} stringwalker++; } return stringwalker; } // strchr replacement
void double2str(char* dest, double value, int decs, size_t bufsz) {
    if (decs < 0 || decs > 9) { DualLogError("double2str: too many decimals\n"); OS_Exit(1); }
    if (value < 0.0) { if (bufsz < 2) {DualLogError("double2str: buffer too small A\n"); OS_Exit(1);} *dest++ = '-'; bufsz--; value = -value; }
    u64 whole = (u64)value; char temp[32]; size_t len = 0;
    if(whole == 0){temp[len++] = '0';}else{while (whole > 0) {temp[len++]='0' + (whole%10); whole/=10;} }
    if (len >= bufsz) {DualLogError("double2str: len larger than buffer\n"); OS_Exit(1);}
    for (size_t i = 0; i < len; ++i) {dest[i] = temp[len - 1 - i];}
    dest += len; bufsz -= len; if(decs == 0) {*dest = '\0'; return;} 
    if(bufsz < 1) {DualLogError("double2str: buffer too small B\n"); OS_Exit(1);}
    *dest++ = '.'; bufsz--; double frac = value - (u64)value, scale = 1.0;
    for (int i = 0; i < decs; ++i) {scale *= 10.0;}
    u64 fracs = (u64)(frac * scale + 0.5);
    for (int i = decs - 1; i >= 0; --i) { if (bufsz < 1) {DualLogError("double2str: buffer too small C\n"); OS_Exit(1);} dest[i]='0' + (fracs%10); fracs/=10; }
    dest[decs] = '\0';
}

int sFormatV(char* buf, size_t bufsz, const char* f, va_list args) {
    if (bufsz == 0) return 0;
    size_t pos = 0;
    while (*f && pos < bufsz - 1) {
        if (*f != '%') { buf[pos++] = *f++; continue; } 
        f++; // skip '%'
        int width = 0; char padChar = ' '; if (*f == '0') { padChar = '0'; f++; }
        while (*f >= '0' && *f <= '9') { width = width * 10 + (*f - '0'); f++; }
        int decimals = 9; if (*f == '.') { f++; if (*f >= '1' && *f <= '9') { decimals = *f - '0'; } f++; }
        switch (*f) {
            case 'x': { u32 val=__builtin_va_arg(args,u32); char num[32]; int i=0; const char* hexChars="0123456789abcdef"; do {num[i++]=hexChars[val % 16]; val/=16;}while(val); while(i < width && pos < bufsz - 1){buf[pos++]=padChar; width--;} while(i-- > 0 && pos < bufsz - 1){buf[pos++] = num[i];} } break;
            case 'u': { u32 val=__builtin_va_arg(args,u32); char num[32]; int i=0; do{num[i++]='0' + (val % 10); val/=10; }while(val); while(i < width && pos < bufsz - 1){buf[pos++]=padChar; width--;} while(i-- > 0 && pos < bufsz - 1){buf[pos++]=num[i];} } break;
            case 'c': { char c=(char)__builtin_va_arg(args,int); if(pos < bufsz - 1){buf[pos++]=c;} } break;
            case 'd': { case 'i': { int val=__builtin_va_arg(args,int); if(val < 0){ if(pos < bufsz - 1){buf[pos++] = '-';} val=-val; } char num[32]; int i=0; do{num[i++]='0' + (val % 10); val/=10;}while(val); while(i < width && pos < bufsz - 1){buf[pos++] = padChar; width--;} while(i-- > 0 && pos < bufsz - 1){buf[pos++] = num[i];} } } break;
            case 's': { const char* s=__builtin_va_arg(args,const char*); size_t len = slen(s); if(pos + len >= bufsz){len = bufsz - pos - 1;}; for(size_t i=0;i<len;++i){buf[pos++] = s[i];} } break;
            case 'f': { double val=__builtin_va_arg(args,double); char num[64]; double2str(num,val,decimals,sizeof(num)); size_t len=slen(num); if(pos + len >= bufsz){len=bufsz - pos - 1;} for (size_t i=0;i<len;++i){buf[pos++]=num[i];} } break;
            case '%': if (pos < bufsz - 1) buf[pos++] = '%'; break;
        }
        f++;
    }
    buf[pos] = '\0';
    return (int)pos;
}

int sFormat(char* buffer, size_t bufsz, const char* format, ...) { va_list args; __builtin_va_start(args,format); int ret = sFormatV(buffer,bufsz,format,args); __builtin_va_end(args); return ret; } // snprintf replacement
char* sUpToEndLine(char* buf, int sz, FHandle fd) {
    if (sz <= 1 || buf == NULL) {return NULL;}
    char* p=buf; int rem = sz - 1; static int pos=0, end=0; static char b[4096];
    while(rem > 0){ if(pos >= end){ long n=OS_Read(fd,b,sizeof(b)); if(n <= 0 && p == buf){return NULL;} pos=0; end=(int)n; } while(rem > 0 && pos < end){ char c=b[pos++]; *p++=c; rem--; if(c == '\n'){goto done;} } }
    done:
    *p = '\0'; return buf;
} // fgets replacement, not thread safe but no multithreading

FHandle console_log_file=0;
static void DualLogMain(const char *prefix, const char *fmt, va_list args) {
    char buf[4096]; va_list c; __builtin_va_copy(c,args); sFormatV(buf,sizeof(buf),fmt,c); __builtin_va_end(c); bool color = (prefix && prefix[0] == '\033');
    #ifdef WINDOWS
        FHandle out = GetStdHandle(color ? (u32)-12 : (u32)-11);
    #else
        FHandle out = color ? 2 : 1;
    #endif
    if (prefix) { OS_RawWrite(out,prefix,slen(prefix)); OS_RawWrite(out,"\033[0m ",5); } OS_RawWrite(out,buf,slen(buf));
    if (console_log_file != INVALID_FHANDLE) { if(prefix){OS_Write(console_log_file,prefix,slen(prefix),"console.log"); OS_Write(console_log_file,"\033[0m ",5,"console.log");} OS_Write(console_log_file, buf, slen(buf),"console.log"); }
}
// Misc Helpers
void DualLog(const char* s, ...) { va_list a; __builtin_va_start(a,s); DualLogMain(NULL,s,a); __builtin_va_end(a); }
void DualLogWarn(const char* s, ...) { va_list a; __builtin_va_start(a,s); DualLogMain("\033[1;38;5;208mWARN:",s,a); __builtin_va_end(a); }
void DualLogError(const char* s, ...) { va_list a; __builtin_va_start(a,s); DualLogMain("\033[1;31mERROR:",s,a); __builtin_va_end(a); }
void BmpWrite(char const *filename, int x, int y, const void *data) {
    FHandle f = OS_OpenWriteonly(filename);
    if (f == INVALID_FHANDLE) { DualLogError("Failed to open %s for writing\n", filename); return; }
    u32 fileSize = 14 + 108 + (u32)x * y * 4; // BMP file header (14 bytes)
    u8 fileHeader[14] = {'B','M',fileSize & 0xFF,(fileSize >> 8) & 0xFF,(fileSize >> 16) & 0xFF,(fileSize >> 24) & 0xFF,0,0,0,0,14 + 108,0,0,0};
    u8 infoHeader[108]={0}; *(u32*)(infoHeader+0)=108;/*size*/
    *(u32*)(infoHeader+4)=(u32)x;/*w*/ *(u32*)(infoHeader+8)=(u32)-y;/*h*/ *(u16*)(infoHeader+12)=1;/*planes*/ *(u16*)(infoHeader+14)=32;/*bit count*/ *(u32*)(infoHeader+16)=3;/*bit fields*/
    *(u32*)(infoHeader+40)=0x000000FF;/*Red*/ *(u32*)(infoHeader + 44) = 0x0000FF00;/*Green*/ *(u32*)(infoHeader + 48) = 0x00FF0000;/*Blue*/ *(u32*)(infoHeader + 52) = 0x00000000;/*Alpha*/
    OS_Write(f,fileHeader,14,filename); OS_Write(f,infoHeader,108,filename);
    const u8 *pixels = (const u8 *)data;
    for (int j=y-1;j>=0;--j) OS_Write(f,(void*)(pixels + j*x*4),(size_t)x*4,filename);
    OS_Close(f);
}

void DebugRAM(const char *context) { // Get USS aka the total RAM uniquely allocated for the process (btop shows RSS so pulls in shared libs and double counts shared RAM).
    (void)context;
    //static void* heap_start = (void*)-1; if(heap_start == (void*)-1){ long r = 12; __asm__ __volatile__("syscall":"+a"(r):"D"(NULL):"rcx","r11","memory"); heap_start = (void*)r; }
    //long r = 12; __asm__ __volatile__("syscall":"+a"(r):"D"(NULL):"rcx","r11","memory"); void* current_brk = (void*)r;
    //size_t heap_bytes = (size_t)((char*)current_brk - (char*)heap_start); size_t uss_bytes = 0;
    //long fd = OS_OpenReadonly("/proc/self/smaps_rollup"); if (fd == INVALID_FHANDLE) { DualLogError("Failed to open /proc/self/smaps_rollup\n"); return; }

    //char buf[4096]; long bytes_read = OS_Read(fd,buf,sizeof(buf)-1); if (bytes_read > 0) { buf[bytes_read] = '\0'; } else buf[0] = '\0'; OS_Close(fd); char* p = buf;
    //while (*p) {
        //if (mcmp(p,"Private_",8) == 0) {
            //p += 8; size_t val = 0; if (mcmp(p,"Clean",5) !=0 && mcmp(p,"Dirty",5) != 0) { p++; continue; }
            //while (*p && *p != ':') p++; if (*p != ':') { p++; continue; }
            
            //p++; while(*p == ' ' || *p == '\t'){p++;} while(*p >= '0' && *p <= '9'){val=val * 10 + (*p - '0'); p++;} uss_bytes += val * 1024;
        //}
        
        //p++;
    //}

    //DualLog("Mem at %s: Heap %ub(%uKB|%.2fMB), USS %ub(%uKB|%.2fMB)\n",context,heap_bytes,heap_bytes / 1024,heap_bytes / 1024.0 / 1024.0,uss_bytes,uss_bytes / 1024,uss_bytes / 1024.0 / 1024.0);
}

void Screenshot() {    
    World.screenshotTimeout = World.current_time + 1.0; // Prevent saving more than 1 per second for sanity purposes.
    OS_MakeFolder("Screenshots"); u16 w = Sys_Settings.ScreenWidth, h = Sys_Settings.ScreenHeight;
    u8* pixels = OS_Alloc(w * h * 4 * sizeof(char));
    glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,pixels);
    char filename[96]; sFormat(filename,sizeof(filename),"Screenshots/%.2f_x%.1f_y%.1f_z%.1f.bmp",get_time(),World.position[PLAYER1].x,World.position[PLAYER1].y,World.position[PLAYER1].z);
    BmpWrite(filename,w,h,pixels); DualLog("Saved screenshot %s\n",filename);
    OS_Free(pixels,w * h * 4 * sizeof(char));
}

u32 random_range_rng = 0x12345678u;
u32 xs32() { u32 x = random_range_rng; x ^= x << 13; x ^= x >> 17; x ^= x << 5; return random_range_rng = x ? x : 0xdeadbeefu; }
u8 random_range_u8(u8 a, u8 b) { if (a > b) { u8 temp = a; a = b; b = temp; } if (a == b) {return a;} u32 r = (u32)b - a + 1u; u32 v,limit = 256u - (256u % r); do { v = xs32() & 0xFFu; } while (v >= limit); return (u8)(a + (v % r)); }
u32 random_range_u32(u32 a, u32 b) { if (a > b) { u32 temp = a; a = b; b = temp; } if (a == b) {return a;} u64 range = (u64)b - a + 1u; return a + (u32)(((u64)xs32() * range) >> 32);  }
i32 random_range_i32(i32 a, i32 b) { if (a > b) { i32 temp = a; a = b; b = temp; } if (a == b) {return a;} u64 range = (u64)((i64)b - a + 1); return a + (i32)(((u64)xs32() * range) >> 32); }
float random_range(float a, float b) { float factor = ((float)(xs32() >> 8)) * (1.0f / 16777216.0f); return a + (b - a) * factor; }
u32 rand() { return xs32() & 0xFFFFu; }
#define RAND_MAX 65535
float lerp(float min, float max, float val) { return min + (max - min) * vclamp(val,0.0f,1.0f); }
float inverse_lerp(float min, float max, float val) { return (min == max) ? 0.0f : vclamp((val - min) / (max - min),0.0f,1.0f); }
float smooth_damp(float current, float target, float *current_velocity, float smooth_time) { 
    if (smooth_time < 0.0001f) smooth_time = 0.0001f;
    float omega = 2.0f / smooth_time;
    float x = omega * (float)World.deltaTime;
    float exp = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);
    float change = current - target, original_to = target;
    target = current - change;
    float temp = (*current_velocity + omega * change) * (float)World.deltaTime;
    *current_velocity = (*current_velocity - omega * temp) * exp;
    float output = target + (change + temp) * exp;
    if ((original_to - current > 0.0f) == (output > original_to)) { output=original_to; *current_velocity=(output - original_to) / (float)World.deltaTime; }
    return output;
}

FHandle levelFileHandle;
char* sLevelFileUpToEndLine(char* buf, int size) { return sUpToEndLine(buf,size,levelFileHandle); }
V3 GetLocalTransformedPos(Entity* originator, V3 offsetFromOriginator) { u16 idx=(u16)(originator - World.instances); V3 scaledOfs = mul_v3_v3_elementwise(offsetFromOriginator,World.scale[idx]); V3 rotatedOfs = quat_rot_v3(World.rotation[idx],scaledOfs); V3 result = V3_AplusB(World.position[idx],rotatedOfs); return result; }
static inline int pntz(size_t p[2]) { return (p[0] != 1) ? __builtin_ctzll(p[0] - 1) : (p[1] ? 8 * sizeof(size_t) + __builtin_ctzll(p[1]) : 0); }
static inline void shl(size_t p[2], int n) { if (n >= 8 * (int)sizeof(size_t)) { p[1] = p[0]; p[0] = 0; n -= 8 * sizeof(size_t); } if (n) { p[1] = (p[1] << n) | (p[0] >> (8 * sizeof(size_t) - n)); p[0] <<= n; } }
static inline void shr(size_t p[2], int n) { if (n >= 8 * (int)sizeof(size_t)) { p[0] = p[1]; p[1] = 0; n -= 8 * sizeof(size_t); } if (n) { p[0] = (p[0] >> n) | (p[1] << (8 * sizeof(size_t) - n)); p[1] >>= n; } }
static void scycle(size_t w, u8* ar[], int n) { u8 tmp[256]; size_t l; if (n<2) {return;} ar[n]=tmp; while(w){ l=w<256?w:256; mcpy(ar[n],ar[0],l); for(int i=0;i<n;i++){mcpy(ar[i],ar[i+1],l);ar[i]+=l;} w-=l; } }
typedef int (*cmpfun)(const void*,const void*); typedef int (*cmpfun_r)(const void*,const void*,void*);
static void sift(u8* hd, size_t w, cmpfun_r cmp, void* arg, int ps, size_t lp[]) { u8* ar[(16*sizeof(size_t))]; int i=1; ar[0]=hd; while (ps>1) { u8* rt=hd-w, *lf=rt-lp[ps-2]; if(cmp(ar[0],lf,arg)>=0 && cmp(ar[0],rt,arg)>=0){break;} if(cmp(lf,rt,arg)>=0){ar[i++&((16*sizeof(size_t))-1)]=lf; hd=lf; ps--;}else{ar[i++&((16*sizeof(size_t))-1)]=rt; hd=rt; ps-=2;} } scycle(w,ar,i&((16*sizeof(size_t))-1)); }
static void trinkle(u8* hd, size_t w, cmpfun_r cmp, void* arg, size_t pp[2], int ps, int trusty, size_t lp[]) {
    u8* ar[(16*sizeof(size_t))]; int i=1; ar[0]=hd; size_t p[2]={pp[0],pp[1]};
    while (p[0]!=1||p[1]!=0) { u8* ss=hd-lp[ps]; if(cmp(ss,ar[0],arg)<=0){break;} if(!trusty&&ps>1){ u8* rt=hd-w,*lf=rt-lp[ps-2]; if(cmp(rt,ss,arg)>=0||cmp(lf,ss,arg)>=0){break;} } ar[i++&((16*sizeof(size_t))-1)]=ss; hd=ss; int t=pntz(p); shr(p,t); ps+=t; trusty=0; }
    if (!trusty) { scycle(w,ar,i&((16*sizeof(size_t))-1)); sift(hd,w,cmp,arg,ps,lp); }
}

void qsort_new(void* base, size_t nel, size_t w, cmpfun cmp) {
    size_t lp[12*sizeof(size_t)], p[2]={1,0}, size=w*nel; int ps=1,trail; if (!size) return;
    u8* hd=base, *high=hd+size-w; for (size_t i=2;lp[0]=lp[1]=w,(lp[i]=lp[i-2]+lp[i-1]+w)<size;i++)/*empty body*/; cmpfun_r cmp_r=(cmpfun_r)(void*)cmp; void* arg=NULL;
    while (hd<high) {
        if ((p[0]&3)==3) { sift(hd,w,cmp_r,arg,ps,lp); shr(p,2); ps+=2; }
        else { if(lp[ps-1]>=((size_t)(high-hd))){trinkle(hd,w,cmp_r,arg,p,ps,0,lp);}else {sift(hd,w,cmp_r,arg,ps,lp);}  if(ps==1){shl(p,1); ps=0;}else{shl(p,ps-1); ps=1;} }
        p[0]|=1; hd+=w;
    }
    trinkle(hd,w,cmp_r,arg,p,ps,0,lp);
    while (ps!=1||p[0]!=1||p[1]!=0) { if(ps<=1) {trail=pntz(p); shr(p,trail); ps+=trail;}else{shl(p,2); ps-=2; p[0]^=7; shr(p,1); trinkle(hd-lp[ps]-w,w,cmp_r,arg,p,ps+1,1,lp); shl(p,1); p[0]|=1; trinkle(hd-w,w,cmp_r,arg,p,ps,1,lp);} hd-=w; }
}
