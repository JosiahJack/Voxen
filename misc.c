// misc.h - ugh I dunno, reorganizing
#include "common.h"
#include "lib.h"
i32 PosGetCellCoordX(float x) { return (u16)clamp((i32)vfloor((x - World.worldMin_x[World.curLev] + CELLXHALF) / CELLSZ),0,(WORLDX - 1)); }
i32 PosGetCellCoordZ(float z) { return (u16)clamp((i32)vfloor((z - World.worldMin_z[World.curLev] + CELLXHALF) / CELLSZ),0,(WORLDX - 1)); }
i32 PosGetCellCoords(float x, float z) { return (PosGetCellCoordZ(z) * WORLDX) + PosGetCellCoordX(x); }
u32 PosGetCellCoordsP(i32 cx, i32 cz) { cx=clamp(cx,0,(WORLDX - 1)); cz=clamp(cz,0,(WORLDX - 1)); return (u32)cz * WORLDX + (u32)cx; }
char statusText[T_BUFFER_SIZE];
void CenterStatusPrint(const char * restrict fmt, ...) { va_list args; __builtin_va_start(args, fmt); sFormatV(statusText,T_BUFFER_SIZE,fmt,args); __builtin_va_end(args); DualLog("%s\n",statusText); World.statusTextDecayFinished = get_time() + 3.5;/*secs decay time before text dissappears.*/ }
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
//     static void* heap_start = (void*)-1; if(heap_start == (void*)-1){ long r = 12; __asm__ __volatile__("syscall":"+a"(r):"D"(NULL):"rcx","r11","memory"); heap_start = (void*)r; }
//     long r = 12; __asm__ __volatile__("syscall":"+a"(r):"D"(NULL):"rcx","r11","memory"); void* current_brk = (void*)r;
//     size_t heap_bytes = (size_t)((char*)current_brk - (char*)heap_start); size_t uss_bytes = 0;
//     long fd = OS_OpenReadonly("/proc/self/smaps_rollup"); if (fd == INVALID_FHANDLE) { DualLogError("Failed to open /proc/self/smaps_rollup\n"); return; }
//     char buf[4096]; long bytes_read = OS_Read(fd,buf,sizeof(buf)-1); if (bytes_read > 0) { buf[bytes_read] = '\0'; } else buf[0] = '\0'; OS_Close(fd); char* p = buf;
//     while (*p) {
//         if (mcmp(p,"Private_",8) == 0) {
//             p += 8; size_t val = 0; if (mcmp(p,"Clean",5) !=0 && mcmp(p,"Dirty",5) != 0) { p++; continue; }
//             while (*p && *p != ':') p++; if (*p != ':') { p++; continue; }
//             p++; while(*p == ' ' || *p == '\t'){p++;} while(*p >= '0' && *p <= '9'){val=val * 10 + (*p - '0'); p++;} uss_bytes += val * 1024;
//         }
//         p++;
//     }
//     DualLog("Mem at %s: Heap %ub(%uKB|%.2fMB), USS %ub(%uKB|%.2fMB)\n",context,heap_bytes,heap_bytes / 1024,heap_bytes / 1024.0 / 1024.0,uss_bytes,uss_bytes / 1024,uss_bytes / 1024.0 / 1024.0);
}

int OS_MakeFolder(const char* path);
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
static u32 xs32() { u32 x = random_range_rng; x ^= x << 13; x ^= x >> 17; x ^= x << 5; return random_range_rng = x ? x : 0xdeadbeefu; }
u8 random_range_u8(u8 a, u8 b) { if (a > b) { u8 temp = a; a = b; b = temp; } if (a == b) {return a;} u32 r = (u32)b - a + 1u; u32 v,limit = 256u - (256u % r); do { v = xs32() & 0xFFu; } while (v >= limit); return (u8)(a + (v % r)); }
u32 random_range_u32(u32 a, u32 b) { if (a > b) { u32 temp = a; a = b; b = temp; } if (a == b) {return a;} u64 range = (u64)b - a + 1u; return a + (u32)(((u64)xs32() * range) >> 32);  }
i32 random_range_i32(i32 a, i32 b) { if (a > b) { i32 temp = a; a = b; b = temp; } if (a == b) {return a;} u64 range = (u64)((i64)b - a + 1); return a + (i32)(((u64)xs32() * range) >> 32); }
float random_range(float a, float b) { float factor = ((float)(xs32() >> 8)) * (1.0f / 16777216.0f); return a + (b - a) * factor; }
u32 rand() { return xs32() & 0xFFFFu; }
float lerp(float min, float max, float val) { return min + (max - min) * vclamp(val,0.0f,1.0f); }
float inverse_lerp(float min, float max, float val) { return (min == max) ? 0.0f : vclamp((val - min) / (max - min),0.0f,1.0f); }
FHandle levelFileHandle;
char* sLevelFileUpToEndLine(char* buf, int size) { return sUpToEndLine(buf,size,levelFileHandle); }
