// textures.c - 2D Texture Loading System and Texture Animations
#include "common.h"

typedef struct { u16 index; bool transparent, doublesided; u8 blend; char path[128]; } TextureData;
typedef struct { TextureData* entries; u32 count, capacity; } TextureDataParser;
typedef struct { u8 r, g, b; } PngPalEntry;
#define VIDEO_RING 8
#define VIDEO_MAXFRAME (1280u*720u)
typedef struct { char path[96]; u32 texIdx, w, h; } VidFrameRec;
static VidFrameRec* g_vidFrames = NULL; static u32 g_vidFrameCnt = 0;
static u32 g_vidPalOffs[VIDEO_RING], g_vidIdxOffs[VIDEO_RING];
static u8* g_vidFrameFlag = NULL;
static u32* g_vidDims = NULL;
typedef struct { u32 state; u32 frameTexIdx, palSize, w, h; u8* idx; u32* pal; } VidSlot;
static VidSlot g_vidSlot[VIDEO_RING];
static void* g_vidRingBuf = NULL; static size_t g_vidRingSize, g_vidRingIdxSize, g_vidRingPalSize;
static u32 g_vidTexBase, g_vidPalBase; static i32* g_vidSizesCPU = NULL; static u16 g_vidSlotTex[VIDEO_RING];
static u32 g_vidReqGen = 0, g_vidReqFrame = 0xFFFFFFFFu, g_vidPreFrame = 0xFFFFFFFFu;
static u32 g_vidLastTex = 0xFFFFFFFFu; static u32 g_vidLastWant = 0xFFFFFFFFu; static bool g_vidUp = false;
static PngArena g_vidArena; static OS_Thread g_vidThr;
INLINE void vid_yield(void) { long r = 24; __asm__ __volatile__("syscall"::"a"(r):"rcx","r11","memory"); } /*sched_yield, the worker is idle most of the time and a bare spin would burn a core*/
static bool has_sub(const char* h,const char* n);
static bool is_vid_path(const char* p);
static VidFrameRec* vid_find(u32 texIdx) { for (u32 k=0;k<g_vidFrameCnt;++k) { if (g_vidFrames[k].texIdx==texIdx) return &g_vidFrames[k]; } return NULL; }
static const u8 bayer8[64] = { 0,32, 8,40, 2,34,10,42, 48,16,56,24,50,18,58,26, 12,44, 4,36,14,46, 6,38, 60,28,52,20,62,30,54,22, 3,35,11,43, 1,33, 9,41, 51,19,59,27,49,17,57,25, 15,47, 7,39,13,45, 5,37, 63,31,55,23,61,29,53,21};
typedef struct { u32 distinct, exactHits, snaps, errSum, errMax; } PalStats;
static void vid_decode(VidFrameRec* r, VidSlot* s) { /*Same palette extraction as the load worker, but straight into a ring slot.*/
    FHandle fd; int sz = 0; const char* d = (const char*)OS_OpenAndAllocateFileBufferReadonly(r->path, &fd, &sz); if (unlikely(!d || sz <= 0)) return;
    int w = 0, h = 0; u8* pix = PngLoad((const u8*)d, sz, &w, &h, &g_vidArena);
    if (likely(pix && w > 0 && h > 0 && (u32)w*h <= VIDEO_MAXFRAME)) {
        u32 nP = (u32)w*h, bs = 0, x = 0, y = 0; u32 eh[TEXHASH_SZ]; mset(eh, 0xFF, sizeof(eh)); u8 ei[TEXHASH_SZ]; u8 nearcache[32768]; mset(nearcache, 0xFF, sizeof(nearcache));
        for (u32 p=0;p<nP;++p, ++x) {
            if (x == (u32)w) { x = 0; y++; }
            u32 c = ((u32*)pix)[p]; u32 hv = (c*0x9E3779B9u); hv ^= hv>>16; u32 sl = hv & (TEXHASH_SZ-1);
            for (u32 pr=0;pr<TEXHASH_SZ;++pr) { if (eh[sl]==U32_MAX) break; if (eh[sl]==c) { s->idx[p]=ei[sl]; goto vidfound; } sl = (sl+1)&(TEXHASH_SZ-1); }
            if (bs < 256) { s->pal[bs]=c; s->idx[p]=(u8)bs; eh[sl]=c; ei[sl]=(u8)bs; bs++; goto vidfound; }
            /*Palette full: snap to nearest. Ordered-dithered*/
            { i32 dz = (i32)bayer8[(y & 7u)*8u + (x & 7u)] - 32; i32 rr = (i32)(c & 255) + (dz>>2), gg = (i32)((c>>8)&255) + (dz>>2), bb = (i32)((c>>16)&255) + (dz>>2);
              if (rr<0) rr=0; if (rr>255) rr=255; if (gg<0) gg=0; if (gg>255) gg=255; if (bb<0) bb=0; if (bb>255) bb=255;
              u32 dc = ((u32)rr) | ((u32)gg<<8) | ((u32)bb<<16) | (c & 0xFF000000u);
              u32 ck = ((dc & 0xF8) >> 3) | ((dc & 0xF800) >> 6) | ((dc & 0xF80000) >> 9);
              u8 hit = nearcache[ck];
              if (hit == 0xFF) { u32 bestI = 0, bestD = ~0u;
                  for (u32 k=0;k<256;++k) { u32 pc = s->pal[k]; i32 dr=(i32)(pc&255)-rr, dg=(i32)((pc>>8)&255)-gg, db=(i32)((pc>>16)&255)-bb;
                      u32 dist = (u32)(dr*dr+dg*dg+db*db); if (dist<bestD) { bestD=dist; bestI=k; } }
                  hit = (u8)bestI; nearcache[ck] = hit; }
              s->idx[p] = hit; }
            vidfound:;
        }
        s->palSize = bs; s->w = (u32)w; s->h = (u32)h; s->frameTexIdx = r->texIdx;
    }
    OS_Free((void*)d,(size_t)sz);
}
static void* vid_worker(void* arg) { /*Decode-ahead: never touches GL. Slot state carries the upload handshake: 0 claimed, 1 decoded-awaiting-upload, 2 uploaded*/
    (void)arg; u32 last = 0, lastPri = 0xFFFFFFFFu, lastPre = 0xFFFFFFFFu, slot = 0;
    for (;;) { u32 g = __atomic_load_n(&g_vidReqGen, __ATOMIC_ACQUIRE);
        if (g != last) { last = g;
            for (u32 pass=0;pass<2;++pass) { u32* lp = pass?&lastPre:&lastPri; u32 want = __atomic_load_n(pass?&g_vidPreFrame:&g_vidReqFrame, __ATOMIC_RELAXED);
                if (want == 0xFFFFFFFFu || want == *lp) continue; /*the 60fps caller bumps the generation ~120x/s; without this the worker re-decodes the same 720p frame over and over and falls behind*/
                *lp = want; VidFrameRec* r = vid_find(want); if (!r) continue;
                VidSlot* s = &g_vidSlot[slot]; slot = (slot + 1u) % VIDEO_RING;
                __atomic_store_n(&s->state, 0u, __ATOMIC_RELAXED); /*claim before decode, so the main thread cannot upload a half-written slot*/
                vid_decode(r, s);
                __atomic_store_n(&s->state, 1u, __ATOMIC_RELEASE); } }
        else { u32 spins = 0; while (spins++ < 4096u) { __asm__ __volatile__("pause"); } vid_yield(); } }
}
u32 VideoPeekFrame(u32 texIdx) { /*Next frame in the same clip, so the worker can stay ahead of playback*/
    if (!g_vidUp || texIdx == 0xFFFFFFFFu) return 0xFFFFFFFFu; for (u32 k=0;k<g_vidFrameCnt;++k) { if (g_vidFrames[k].texIdx==texIdx) { if (k+1u < g_vidFrameCnt) return g_vidFrames[k+1].texIdx; return 0xFFFFFFFFu; } } return 0xFFFFFFFFu; }
void VideoInit(void) { if (g_vidUp) return; PngArenaInit(&g_vidArena); g_vidUp = true; OS_ThreadCreate(&g_vidThr, vid_worker, NULL); }
void VideoRequestFrame(u32 texIdx) { /*want and the frame after it: the clip advances ~11fps while this runs at 60fps, so asking one step ahead is what keeps the ring fed*/
    if (!g_vidUp || texIdx == 0xFFFFFFFFu || !vid_find(texIdx)) return;
    if (__atomic_load_n(&g_vidReqFrame, __ATOMIC_RELAXED) == texIdx) return; /*already queued, the 60fps caller must not re-queue the same frame every render*/
    for (u32 r=0;r<VIDEO_RING;++r) { if (g_vidSlot[r].frameTexIdx == texIdx && __atomic_load_n(&g_vidSlot[r].state,__ATOMIC_ACQUIRE)) return; /*already decoded*/ }
    __atomic_store_n(&g_vidReqFrame, texIdx, __ATOMIC_RELAXED); __atomic_fetch_add(&g_vidReqGen, 1, __ATOMIC_RELEASE);
}
void VideoPrefetchFrame(u32 texIdx) { /*Next clip frame, decoded in the same worker pass so playback is never a frame behind*/
    if (!g_vidUp || texIdx == 0xFFFFFFFFu || texIdx == __atomic_load_n(&g_vidReqFrame, __ATOMIC_RELAXED)) return;
    for (u32 r=0;r<VIDEO_RING;++r) { if (g_vidSlot[r].frameTexIdx == texIdx && __atomic_load_n(&g_vidSlot[r].state,__ATOMIC_ACQUIRE)) return; }
    if (!vid_find(texIdx)) return; __atomic_store_n(&g_vidPreFrame, texIdx, __ATOMIC_RELAXED); __atomic_fetch_add(&g_vidReqGen, 1, __ATOMIC_RELEASE);
}
u32 VideoFrameTexIndex(u32 texIdx) { /*Exact match only. The old nearest-slot fallback could walk backwards, and worse, could match a frame from a different clip that happened to be numerically closer. Playback is monotonic, so the correct answer when the wanted frame is not resident is to hold the previous one*/
    if (!g_vidUp) return 0xFFFFFFFFu;
    u32 best = 0xFFFFFFFFu;
    for (u32 r=0;r<VIDEO_RING;++r) { VidSlot* s = &g_vidSlot[r];
        u32 st = __atomic_load_n(&s->state, __ATOMIC_ACQUIRE);
        if (s->frameTexIdx == 0xFFFFFFFFu || st == 0) continue;
        if (st == 1) {
            glBindBuffer(GL_SSBO, colorBufferID); glBufferSubData(GL_SSBO, g_vidIdxOffs[r], s->w*s->h, s->idx);
            glBindBuffer(GL_SSBO, texPalID); glBufferSubData(GL_SSBO, g_vidPalOffs[r]*sizeof(u32), s->palSize*sizeof(u32), s->pal);
            g_vidSizesCPU[r*2] = (i32)s->w; g_vidSizesCPU[r*2+1] = (i32)s->h;
            glBindBuffer(GL_SSBO, textureSizesID); glBufferSubData(GL_SSBO, (size_t)g_vidSlotTex[r]*2*sizeof(i32), 2*sizeof(i32), &g_vidSizesCPU[r*2]);
            glBindBuffer(GL_SSBO, 0); g_vidLastTex = s->frameTexIdx; __atomic_store_n(&s->state, 2u, __ATOMIC_RELEASE);
        }
        if (s->frameTexIdx == texIdx) best = g_vidSlotTex[r];
    }
    if (best == 0xFFFFFFFFu) return 0xFFFFFFFFu;
    g_vidLastWant = texIdx;
    return best;
}
typedef struct { u32 img_x, img_y; i32 img_n, img_out_n; u8 img_depth, img_color_type, img_error; u16 img_palette_count; PngPalEntry img_palette[256]; u8 img_trns[256], *img_buffer, *img_buffer_end; } PngContext;
typedef struct { u8* indices; u32* palette, palSize; i32 w, h; } TexResult;
typedef struct TextureParseTask { u32 texCnt; _Atomic u32* shared_idx; i32* parsIdx; const TextureDataParser* parser; TexResult* results; int tid; } TextureParseTask;
typedef struct { PngContext* s; u8* idata, *expanded, *out; } PngData;
typedef struct { u16 fast[1 << 9], firstcode[16], firstsymbol[16], value[288]; i32 maxcode[17]; u8 size[288]; } PngHuffman;
typedef struct { u8 *zbuffer, *zbuffer_end, *zout, *zout_start; i32 num_bits; u32 code_buffer; PngHuffman z_length, z_distance; } pngzbuf;

enum { PNGFmt_none = 0, PNGFmt_sub = 1, PNGFmt_up = 2, PNGFmt_avg = 3, PNGFmt_paeth = 4, PNGFmt_avg_first, PNGFmt_paeth_first };

PngArena png_arena_main;
static PngArena* thread_png_arenas = NULL;
u32 totalPixels, totalPaletteColors;

void PngArenaInit(PngArena* arena) {
    if (!arena->base) {
        arena->base = OS_Alloc(16777216);
        arena->cursor = arena->base;
        arena->end = arena->base + 16777216;
    }
}

void* PngArenaAlloc(PngArena* a, size_t s) {
    s=(s + 15) & ~(size_t)15;/*Keep every allocation 16-byte aligned*/
    if (!a->base || a->cursor + s > a->end) { DualLogError("PngArena overflow\n"); OS_Exit(1); }
    
    void* p = a->cursor;
    a->cursor += s;
    return p;
}

static u32 PngGet32be(PngContext* s) {
    if (s->img_buffer + 4 > s->img_buffer_end) { s->img_error = 1; return 0; }
    
    const u8* p = s->img_buffer;
    s->img_buffer += 4;
    return ((u32)p[0] << 24) | ((u32)p[1] << 16) | ((u32)p[2] << 8) | p[3];
}

static i32 BitReverse(i32 n, i32 b) {
    n = ((n & 0xAAAA) >> 1) | ((n & 0x5555) << 1);
    n = ((n & 0xCCCC) >> 2) | ((n & 0x3333) << 2);
    n = ((n & 0xF0F0) >> 4) | ((n & 0x0F0F) << 4);
    n = ((n & 0xFF00) >> 8) | ((n & 0x00FF) << 8);
    return n >> (16 - b);
}

static i32 PngHuf(PngHuffman* z, const u8* sl, i32 num) {
    i32 i, k=0, code=0, nc[16], sz[17] = {0};
    mset(z->fast, 0, sizeof(z->fast));
    if(num != 32) { for (i=0;i<num;++i) { ++sz[sl[i]]; } }
    sz[0] = 0;
    for (i=1;i<16;++i) {
        if (sz[i] > (1<<i)) { return 0; }
        
        nc[i] = code;
        z->firstcode[i] = (u16)code;
        z->firstsymbol[i] = (u16)k;
        code += sz[i];
        if (sz[i] && code - 1 >= (1 << i)) { return 0; }
        
        z->maxcode[i] = code << (16 - i);
        code <<= 1;
        k += sz[i];
    }
    
    z->maxcode[16] = 0x10000;
    for (i=0;i<num;++i) {
        int s = (num == 32) ? 5 : sl[i]; if (!s) { continue; }
        
        int c = nc[s] - z->firstcode[s] + z->firstsymbol[s];
        u16 fv = (u16)((s << 9) | i);
        z->size[c] = (u8)s;
        z->value[c] = (u16)i;
        if(s <= 9){
            int j=BitReverse(nc[s],s);
            while (j < (1 << 9)) { z->fast[j] = fv; j += (1 << s); }
        }
        
        ++nc[s];
    }
    
    return 1;
}

INLINE u32 PngZReceive(pngzbuf* z, int n) {
    if (z->num_bits < 16 && z->zbuffer < z->zbuffer_end) { do { z->code_buffer |= (u32)(*z->zbuffer++) << z->num_bits; z->num_bits += 8; } while (z->num_bits <= 24 && z->zbuffer < z->zbuffer_end); }
    u32 k=z->code_buffer&((1u<<n)-1);
    z->code_buffer>>=n;
    z->num_bits-=n;
    return k;
}

INLINE u32 PngHuffman_decode(pngzbuf* a, PngHuffman* z) {
    if (a->num_bits < 16 && a->zbuffer < a->zbuffer_end) { do { a->code_buffer |= (u32)(*a->zbuffer++) << a->num_bits; a->num_bits += 8; } while (a->num_bits <= 24 && a->zbuffer < a->zbuffer_end); }
    int b = z->fast[a->code_buffer&511], s;
    if (b) { s = b >> 9; a->code_buffer >>= s; a->num_bits -= s; return b & 511; }
    
    int k = BitReverse(a->code_buffer,16);
    for (s=10;s<16 && k >= z->maxcode[s];++s) { };
    b = (k >> (16 - s)) - z->firstcode[s] + z->firstsymbol[s];
    if (b < 0 || ((u32)b) >= 288) return 0;
    
    a->code_buffer>>=s;
    a->num_bits -= s;
    return z->value[b];
}

static u8 PngZDefLen(int i) { return (i < 144) ? 8 : ((i < 256) ? 9 : ((i<280) ? 7 : 8)); }
u8* PngDecode(const u8* buffer, i32 len, i32 initial_size, i32* outlen, PngArena* arena) {
    pngzbuf a={0}; u8 *p=(u8*)PngArenaAlloc(arena,initial_size),d_len[288],*zout_end; i32 f,t; a.zbuffer=(u8*)buffer; a.zbuffer_end=(u8*)buffer+len; a.zout_start=a.zout=p; a.zbuffer+=2; zout_end=p+initial_size;
    do {if(a.zbuffer>=a.zbuffer_end){return NULL;} f=PngZReceive(&a,1); t=PngZReceive(&a,2);
        if (t == 0) {/*Uncompressed block*/
            u8 header[4]; if (a.num_bits & 7) PngZReceive(&a,a.num_bits & 7); for (int k=0;k<4;++k) { header[k] = (a.num_bits > 0) ? (a.num_bits -= 8, (u8)(a.code_buffer >> (k * 8))) : *a.zbuffer++; } a.code_buffer >>= (a.num_bits > 0 ? 0 : 0);/*Reset or discard state if necessary*/ i32 lenh=(header[1] << 8) | header[0]; if(a.zbuffer + lenh > a.zbuffer_end || a.zout + lenh > zout_end){return NULL;} mcpy(a.zout,a.zbuffer,lenh); a.zbuffer+=lenh; a.zout+=lenh;
        } else {
            if (t == 1) { for(int i=0; i<288; ++i) d_len[i]=PngZDefLen(i); PngHuf(&a.z_length, d_len, 288); PngHuf(&a.z_distance, NULL, 32); }
            else {/*Huffman compressed block*/
                static const u8 dz[]={16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15}; u8 lc[286+32+137], cs[19]={0}; u32 hl=PngZReceive(&a,5)+257, hd=PngZReceive(&a,5)+1, hc=PngZReceive(&a,4)+4, nt=hl+hd, n=0; for(u32 i=0;i<hc;++i) cs[dz[i]]=(u8)PngZReceive(&a,3); PngHuf(&a.z_length,cs,19); bool skip=false;
                while(n<nt){u32 c=PngHuffman_decode(&a,&a.z_length); if(c<16){lc[n++]=(u8)c;} else { u8 fz=0; if(c==16){ if(n==0){skip=true; break;} c=PngZReceive(&a,2)+3; fz=lc[n-1];}else if(c==17){c=PngZReceive(&a,3)+3;}else if(c==18){c=PngZReceive(&a,7)+11;}else{skip=true; break;} if(n+c>(u32)(sizeof(lc)/sizeof(lc[0]))){skip=true; break;} mset(lc+n,fz,c); n+=c;}} if(skip || !(PngHuf(&a.z_length,lc,hl) && PngHuf(&a.z_distance,lc+hl,hd))){if(outlen)*outlen=0; return NULL;}
            }
            u8* o=a.zout;/*Parse huffman block*/
            static const int lb[]={3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227,258}, le[]={0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0}, db[]={1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577}, de[]={0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};
            for(;;){
                if (a.zbuffer >= a.zbuffer_end && a.num_bits <= 0) { if(outlen) *outlen=0; return NULL;} int z=PngHuffman_decode(&a,&a.z_length); 
                if(z<256){if(o >= zout_end){return NULL;} *o++=(u8)z;} else if(z==256){ a.zout=o; break;}else{z-=257; if (z > 28) { if (outlen) *outlen=0; return NULL; } int l=lb[z]+(le[z]?PngZReceive(&a,le[z]):0); z=PngHuffman_decode(&a,&a.z_distance); if (z > 29) { if (outlen) *outlen=0; return NULL; } int d=db[z]+(de[z]?PngZReceive(&a,de[z]):0); u8* ph=o-d; if(ph < a.zout_start || o+1 > zout_end){return NULL;} while(l--){*o++=*ph++;}}
            }
        }
    } while (!f);    if (outlen) *outlen = (i32)(a.zout - a.zout_start); return a.zout_start;
}
 
static u8 first_row_filter[5] = {PNGFmt_none, PNGFmt_sub, PNGFmt_none, PNGFmt_avg_first, PNGFmt_paeth_first};
INLINE i32 PngPaeth(i32 a, i32 b, i32 c) { i32 p = a+b-c, pa = vabs(p-a), pb = vabs(p-b), pc = vabs(p-c); return (pa <= pb && pa <= pc) ? a : (pb <= pc ? b : c); }
static i32 CreatePngImageArena(PngArena* arena, PngData* a, u8* raw, u32 raw_len, i32 out_n, u32 x, u32 y, i32 img_n) {
    u32 i, j, stride = x * out_n, w_bytes = (img_n * x * 8 + 7) >> 3; i32 k, f; if (raw_len < (w_bytes + 1) * y) return 0; a->out = (u8*)PngArenaAlloc(arena, (size_t)x * y * out_n);
    for (j = 0; j < y; ++j) {
        u8 *cur=a->out+stride*j; u8 *prior=(j > 0) ? cur - stride : a->out; if ((f = *raw++) > 4) return 0; if (j == 0)f=first_row_filter[f];
        switch (f) {/*First pixel (left neighbor is implicitly 0)*/case PNGFmt_none:case PNGFmt_sub:for(k=0;k<img_n;++k)cur[k]=raw[k]; break; case PNGFmt_up:for(k=0;k<img_n;++k)cur[k]=raw[k]+prior[k]; break; case PNGFmt_avg:for(k=0;k<img_n;++k)cur[k]=raw[k]+(prior[k]>>1); break; case PNGFmt_paeth:for(k=0;k<img_n;++k)cur[k]=raw[k]+PngPaeth(0,prior[k],0); break; case PNGFmt_avg_first:for (k=0;k<img_n;++k)cur[k]=raw[k]+(0>>1);/*left doesn't exist*/ break; default:return 0;}
        if (img_n != out_n) cur[img_n] = 255; raw += img_n; cur += out_n; prior += out_n; 
        switch (f) {/*Remaining pixels in the row*/
            case PNGFmt_none:for(i=x-1;i>=1;--i,cur[img_n]=(img_n!=out_n ? 255 : cur[img_n]),raw+=img_n,cur+=out_n,prior+=out_n){for(k=0;k<img_n;++k)cur[k]=raw[k];} break; case PNGFmt_sub:for (i = x - 1; i >= 1; --i, cur[img_n] = (img_n != out_n ? 255 : cur[img_n]), raw += img_n, cur += out_n, prior += out_n) { for (k = 0; k < img_n; ++k) cur[k] = raw[k] + cur[k - out_n]; } break;
            case PNGFmt_up:for(i=x-1;i>=1;--i,cur[img_n]=(img_n!=out_n ? 255 : cur[img_n]),raw+=img_n,cur+=out_n,prior+=out_n){for(k=0;k<img_n;++k)cur[k]=raw[k]+prior[k];} break; case PNGFmt_avg:for (i = x - 1; i >= 1; --i, cur[img_n] = (img_n != out_n ? 255 : cur[img_n]), raw += img_n, cur += out_n, prior += out_n) { for (k = 0; k < img_n; ++k) cur[k] = raw[k] + ((prior[k] + cur[k - out_n]) >> 1); } break;
            case PNGFmt_paeth:for(i=x-1;i>=1;--i,cur[img_n]=(img_n!=out_n ? 255 : cur[img_n]),raw+=img_n,cur+=out_n,prior+=out_n){for(k=0;k<img_n;++k)cur[k]=raw[k]+PngPaeth(cur[k-out_n],prior[k],prior[k-out_n]);} break; case PNGFmt_avg_first:for(i=x-1;i>=1;--i,cur[img_n]=(img_n != out_n ? 255 : cur[img_n]),raw+=img_n,cur+=out_n,prior+=out_n){for(k=0;k<img_n;++k)cur[k]=raw[k]+(cur[k-out_n]>>1);} break; default: return 0;
        }
    } return 1;
}

u8* PngLoad(const u8* buffer, int len, int* x, int* y, PngArena* arena) {
    if (!buffer || len < 8){return NULL;} if (arena->base){arena->cursor=arena->base;}
    PngContext s={0}; s.img_n=s.img_out_n=0; s.img_buffer=(u8*)buffer; s.img_buffer_end=(u8*)buffer+len; s.img_error=0; s.img_palette_count=0; s.img_depth=8; s.img_color_type=0; mset(s.img_trns,255,sizeof(s.img_trns)); mset(s.img_palette,0,sizeof(s.img_palette)); PngData z={0}; z.s=&s; u32 ioff=0; z.expanded=z.idata=z.out=NULL; s.img_buffer+=8; s.img_x=s.img_y=1;
    for (;;) {
        u32 length=PngGet32be(&s),type=PngGet32be(&s); if (s.img_error || (u32)(s.img_buffer_end - s.img_buffer) < length) { s.img_error = 1; break; }
        switch (type) {
            case 0x49484452:{/*IHDR*/if(length != 13 || s.img_buffer+13>s.img_buffer_end){s.img_error=1; break;} s.img_x=PngGet32be(&s); s.img_y=PngGet32be(&s); if(s.img_x>16384 || s.img_y>16384){s.img_error=1; break;} u8 depth=*s.img_buffer++; i32 color=(*s.img_buffer++); s.img_buffer+=3; s.img_depth=depth; s.img_color_type=(u8)color; s.img_n=(color == 3) ? 1 : (color & 2 ? 3 : 1)+(color & 4 ? 1 : 0); if(depth==0||depth==16)s.img_error=1;} break;
            case 0x504C5445:{ s.img_palette_count = (u16)(length / 3); for (u32 i = 0; i < s.img_palette_count && i < 256; ++i) { s.img_palette[i].r = s.img_buffer[i*3+0]; s.img_palette[i].g = s.img_buffer[i*3+1]; s.img_palette[i].b = s.img_buffer[i*3+2]; } s.img_buffer += length; } break; // PLTE 
            case 0x74524E53:{ for (u32 i = 0; i < length && i < 256; ++i) s.img_trns[i] = s.img_buffer[i]; s.img_buffer += length; } break;  // tRNS - per-palette-entry alpha, defaults to 255 (opaque) for entries not listed
            case 0x49444154:if (!z.idata) { z.idata = (u8*)PngArenaAlloc(arena, len + 16); ioff = 0; } mcpy(z.idata + ioff, s.img_buffer, length); s.img_buffer += length; ioff += length; break;
            case 0x49454E44:{/*IEND*/
                if (s.img_error){goto Label_parsesuccess;} u32 bpp = (u32)s.img_n * s.img_depth; u32 row_bytes = (s.img_x * bpp + 7) / 8; u32 rL = row_bytes * s.img_y + s.img_y; z.expanded = (u8*)PngDecode(z.idata,ioff,rL,(i32*)(&rL),arena); if (!z.expanded){z.out=NULL; PngGet32be(&s); goto Label_parsesuccess;} i32 ok;
                if (s.img_depth < 8) { ok = CreatePngImageArena(arena,&z,z.expanded,rL,1,row_bytes,s.img_y, 1); if(ok){u32 nP=s.img_x*s.img_y; u8* samples=(u8*)PngArenaAlloc(arena,nP); if(!samples){ok=0;}else{u8 maxv=(u8)((1u<<s.img_depth)-1); for(u32 row=0;row<s.img_y;++row){u8* rp=z.out+row*row_bytes; for(u32 col=0;col<s.img_x;++col){u32 bit=col*s.img_depth; samples[row*s.img_x+col]=(rp[bit>>3]>>(8-s.img_depth-(u8)(bit&7)))&maxv;}} z.out=samples;}}}
                else { s.img_out_n = (s.img_color_type == 3) ? 1 : ((s.img_n + 1 == 4) ? 4 : s.img_n); ok = CreatePngImageArena(arena,&z,z.expanded, rL,s.img_out_n,s.img_x,s.img_y,s.img_n); }
                if (ok) {
                    u32 nP = s.img_x * s.img_y; u8* rgba = (u8*)PngArenaAlloc(arena, (size_t)nP * 4);
                    if (rgba) {
                        if (s.img_color_type == 3) { for (u32 p = 0; p < nP; ++p) {u8 idx=z.out[p]; if (idx>=s.img_palette_count){idx=0;} PngPalEntry* e=&s.img_palette[idx]; rgba[p*4+0]=e->r; rgba[p*4+1]=e->g; rgba[p*4+2]=e->b; rgba[p*4+3]=s.img_trns[idx];} }  // indexed -> palette lookup (z.out is 1 index byte/pixel, any depth)
                        else if (s.img_color_type == 0) {/*grayscale -> replicate into RGB, alpha 255 (no tRNS gray-key support)*/ u8 scale=s.img_depth<8 ? (u8)(255 / ((1u << s.img_depth) - 1)) : 1; for (u32 p = 0; p < nP; ++p) { u8 g = (s.img_depth < 8) ? (u8)(z.out[p] * scale) : z.out[p]; rgba[p*4+0]=g; rgba[p*4+1]=g; rgba[p*4+2]=g; rgba[p*4+3]=255;}}
                        else if (s.img_color_type == 4) {/*grayscale+alpha -> replicate into RGB, keep alpha*/ for (u32 p=0; p<nP;++p){u8 g=z.out[p*2],a=z.out[p*2+1]; rgba[p*4+0]=g; rgba[p*4+1]=g; rgba[p*4+2]=g; rgba[p*4+3]=a;}}
                        else { mcpy(rgba, z.out, (size_t)nP * 4); } // color type 2 (RGB, already alpha-promoted by CreatePngImageArena) or 6 (RGBA)
                        z.out = rgba;
                    } else z.out = NULL;
                } else z.out = NULL; 
                PngGet32be(&s); goto Label_parsesuccess;
            } default: s.img_buffer += length; break;
        } PngGet32be(&s);
    } Label_parsesuccess: *x = z.s->img_x; *y = z.s->img_y; return z.out;
}
 
static void* TextureParsingWorker(void* arg) {
    TextureParseTask* t = (TextureParseTask*)arg;
    u32 i;
    while ((i = __atomic_fetch_add((u32*)t->shared_idx,1,0)) < t->texCnt) { /*Dynamic Work Stealing: Threads fetch next available index automatically*/
        i32 pIdx = t->parsIdx[i];
        if(unlikely(pIdx < 0 || pIdx >= (i32)t->parser->count)) { continue; }
        
          doubleSidedTexture[i] = t->parser->entries[pIdx].doublesided;
          transparentTexture[i] = t->parser->entries[pIdx].transparent;
        particleBlendTexture[i] = t->parser->entries[pIdx].blend;
        FHandle dummy_fd;
        int sz = 0;
        const char* d = (const char*)OS_OpenAndAllocateFileBufferReadonly(t->parser->entries[pIdx].path, &dummy_fd, &sz);
        if(unlikely(!d || sz <= 0)) continue;
        if (g_vidFrameFlag[i]) { /*Video flipbook frame: never decoded here, w/h come from IHDR and the page is filled at playback*/
            if (sz > 24) { g_vidDims[i] = (((u32)(u8)d[16]<<24)|((u32)(u8)d[17]<<16)|((u32)(u8)d[18]<<8)|(u32)(u8)d[19]) << 16 | (((u32)(u8)d[20]<<24)|((u32)(u8)d[21]<<16)|((u32)(u8)d[22]<<8)|(u32)(u8)d[23]); }
            OS_Free((void*)d,(size_t)sz); continue; }

        int w = 0, h = 0;
        u8 *pix=PngLoad((const u8*)d, sz, &w, &h, &thread_png_arenas[t->tid]);
        if (!pix || w < 1 || h < 1) { OS_Free((void*)d,(size_t)sz); continue; }
        
        u32 nP = (u32)w*h; u8 *idx = (u8*)OS_Alloc(nP); u32 *pal = (u32*)OS_Alloc(256*sizeof(u32));
        u32 pSz = 0; u32 exact_hash[TEXHASH_SZ];
        mset(exact_hash, 0xFF, sizeof(exact_hash)); /*Fast Exact Match Hash Map, 0xFFFFFFFF U32_MAX = empty*/
        u8 exact_idx[TEXHASH_SZ], nearest_cache[32768]; /*15-bit Color Space Cache for fast nearest-neighbor fallback*/
        bool nearclear = false;
        for (u32 p = 0; p < nP; ++p) {
            u32 c = ((u32*)pix)[p];
            u32 h_val = (c * 0x9E3779B9u);/*Murmur-style avalanche hash*/
            h_val ^= h_val >> 16;
            u32 slot = h_val & (TEXHASH_SZ - 1);
            for (u32 probe=0;probe<TEXHASH_SZ;++probe) {
                if (exact_hash[slot] == U32_MAX) { break; }
                if (exact_hash[slot] == c) { idx[p] = exact_idx[slot]; goto found; }
                
                slot = (slot + 1) & (TEXHASH_SZ - 1);
            }
            
            if (pSz < 256) {
                pal[pSz] = c;
                idx[p] = (u8)pSz;
                exact_hash[slot] = c;
                exact_idx[slot] = (u8)pSz;
                pSz++;
            } else { /*Fallback: Check Nearest Neighbor Cache first (R5 G5 B5)*/
                if (!nearclear) { mset(nearest_cache, 0xFF, sizeof(nearest_cache)); nearclear = true; }
                u32 cache_key = ((c & 0xF8) >> 3) | ((c & 0xF800) >> 6) | ((c & 0xF80000) >> 9);
                if (nearest_cache[cache_key] != 0xFF) { idx[p] = nearest_cache[cache_key]; goto found; }
                u32 best = 0, bestDist = ~0u; /*Cache Miss: Full Search*/
                i32 r1 = c & 255, g1 = (c>>8) & 255, b1 = (c>>16) & 255, a1 = c>>24; 
                for (u32 k = 0; k < 256; ++k) {
                    u32 pc = pal[k];
                    i32 dr = (pc&255)-r1, dg = ((pc>>8)&255)-g1, db = ((pc>>16)&255)-b1, da = (pc>>24)-a1;
                    u32 dist = dr*dr + dg*dg + db*db + da*da;
                    if (dist < bestDist) { bestDist = dist; best = k; }
                }
                
                idx[p] = (u8)best;
                nearest_cache[cache_key] = (u8)best; // Update Cache
            }
            
            found:;
        }
        
        t->results[i] = (TexResult){.indices = idx, .palette = pal, .palSize = pSz, .w = w, .h = h};
        OS_Free((void*)d,(size_t)sz);
    }
    
    return NULL;
}
 
static bool ParseTextureData(TextureDataParser *p, u16 maxS, const char *fn) {
    FHandle fd; int sz; char *data = OS_OpenAndAllocateFileBufferReadonly(fn, &fd, &sz), *cur = data, *end = data + sz; u32 line = 0, m_idx = 0;
    while (cur < end) {
        char *s = cur;
        while (cur < end && *cur != '\n' && *cur != '\r') { cur++; }
        size_t len = cur - s;
        line++;
        if (len <= 0) { cur++; continue; }
        
        while (cEmpty(*s)) { s++; }
        char *le = s + (cur - s) - 1;
        while (le > s && cEmpty(*le)) { le--; }
        if (*s == '\0' || (s[0] == '/' && s[1] == '/') || s[0] == '#') {
            if (cur < end && (*cur == '\r' || *cur == '\n')) { cur++; }
            continue;
        }
        
        char *col = StringFindFirstCharWithin(s, ':');
        if (col && sCompUpToLen(s, "index", col - s) == 0) {
            char *v = col + 1;
            while (cEmpty(*v)) { v++; }
            u32 idx = parse_numberu32(v, s, line);
            if (idx > m_idx) { m_idx = idx; }
        }
        
        if (cur < end && (*cur == '\r' || *cur == '\n')) { cur++; }
    }
    
    if (!m_idx || m_idx >= maxS) {
        if (!m_idx) { DualLogWarn("No entries in %s\n", fn); }
        else { DualLogWarn("Index %u too large in %s\n",m_idx,fn); OS_Free(data,sz); return false; }
    }

    p->entries = OS_AllocScratch((p->count = p->capacity = m_idx + 1) * sizeof(TextureData));
    for (u32 i = 0; i < p->count; ++i) { p->entries[i] = (TextureData){.index = U16_MAX}; }
    TextureData e = {.index = U16_MAX}; line = 0; cur = data;
    while (cur < end) {
        char *s = cur;
        while (cur < end && *cur != '\n' && *cur != '\r') { cur++; }
        size_t len = cur - s;
        line++;
        if (len < 3) { cur++; continue; }
        
        while (cEmpty(*s)) { s++; }
        char *le = s + (cur - s) - 1;
        while (le > s && cEmpty(*le)) { le--; }
        if (s[0] == '/' && s[1] == '/') { cur++; continue; }
        
        if (*s == '#') {
            if (e.path[0] && e.index < p->capacity) { p->entries[e.index] = e; }
            e = (TextureData){.index = U16_MAX};
            size_t aL = le - s;
            if (aL >= sizeof(e.path)) { aL = sizeof(e.path) - 1; }
            mcpy(e.path,s + 1,aL); e.path[aL] = 0;
        } else {
            char *col = StringFindFirstCharWithin(s, ':');
            if (col) {
                char *k = s,*v = col + 1, tk[256] = {0}, tv[256] = {0};
                while(cEmpty(*k) && k < col) { k++; }
                while(cEmpty(*v) && v <= le) { v++; }
                size_t kL = col - k, vL = (le >= v) ? (le - v + 1) : 0;
                if (kL&&vL) {
                    sCpy2aSubFromb(tk, kL, k, 256);
                    sCpy2aSubFromb(tv, vL, v, 256);
                    char *ke = tk + slen(tk) - 1, *ve = tv + slen(tv) - 1;
                    while(ke>tk&&cEmpty(*ke)) {*ke-- = 0; }
                    while(ve>tv&&cEmpty(*ve)) {*ve-- = 0; }
                    
                         if (sEqual(tk, "index"))       { e.index       = parse_numberu16(tv,s,line); }
                    else if (sEqual(tk, "transparent")) { e.transparent = parse_bool(tv,s,line); }
                    else if (sEqual(tk, "doublesided")) { e.doublesided = parse_bool(tv,s,line); }
                    else if (sEqual(tk, "blend"))       { e.blend       = parse_numberu8(tv,s,line); }
                }
            }
        }
        
        if (cur < end && (*cur == '\r' || *cur == '\n')) { cur++; }
    }
    
    if (e.path[0] && e.index < p->capacity) { p->entries[e.index] = e; }
    OS_Free(data, sz);
    return true;
}

void SetWindowIcon(WinSysIcon*);
static bool has_sub(const char* h,const char* n){ for(;*h;++h){ const char* a=h,*b=n; while(*a&&*b&&*a==*b){++a;++b;} if(!*b) return true; } return false; }
static bool is_vid_path(const char* p){ return has_sub(p,"AAIntro")||has_sub(p,"AAOutro")||has_sub(p,"VMail"); }
void LoadTextures() {
    double start_time = get_time();
    DebugRAM("start LoadTextures");
    texCnt = totalPixels = totalPaletteColors = 0u;
    TextureDataParser texture_parser;
    if (unlikely(!ParseTextureData(&texture_parser, MAX_TXRS, "./Data/textures.txt"))) { DualLogError("Could not parse ./Data/textures.txt!\n"); OS_Exit(1); }
    
    i32 maxIndex = -1;
    for (u32 k=0;k<texture_parser.count;++k) {
        if (texture_parser.entries[k].index > maxIndex && texture_parser.entries[k].index != U16_MAX) { maxIndex = texture_parser.entries[k].index; }
    }
    
    texCnt = (u16)(maxIndex + 1);
    g_vidFrameFlag = OS_Alloc(texCnt); mset(g_vidFrameFlag, 0, texCnt);
    g_vidDims = OS_Alloc((size_t)texCnt * sizeof(u32)); mset(g_vidDims, 0, (size_t)texCnt * sizeof(u32));
    { u32 vc = 0;
      for (u32 k=0;k<texture_parser.count;++k) { u32 ti = texture_parser.entries[k].index; if (ti < texCnt && is_vid_path(texture_parser.entries[k].path)) { g_vidFrameFlag[ti] = 1; vc++; } }
      g_vidFrames = OS_Alloc((vc ? vc : 1) * sizeof(VidFrameRec)); g_vidFrameCnt = 0;
      for (u32 k=0;k<texture_parser.count;++k) { u32 ti = texture_parser.entries[k].index; if (ti >= texCnt || !g_vidFrameFlag[ti]) continue; VidFrameRec* r = &g_vidFrames[g_vidFrameCnt++]; r->texIdx = ti; r->w = r->h = 0; size_t pl = slen(texture_parser.entries[k].path); if (pl > sizeof(r->path)-1) pl = sizeof(r->path)-1; mcpy(r->path, texture_parser.entries[k].path, pl); r->path[pl] = 0; }
      DualLog("Video frames paged: %u (%.2f MB excluded from palette)\n", g_vidFrameCnt, ((double)g_vidFrameCnt*0.0)); }
    i32* parsIdx = OS_AllocScratch(texCnt * sizeof(i32));
    mset(parsIdx, -1, texCnt * sizeof(i32)); 
    for (u32 k=0;k<texture_parser.count;++k) {
        if (texture_parser.entries[k].index < texCnt) { parsIdx[texture_parser.entries[k].index] = (i32)k; }
    }
    
    DualLog("Loading textures (%u) ... ", texture_parser.count);
    thread_png_arenas = (PngArena*)OS_AllocScratch((size_t)threadCnt*sizeof(PngArena));
    for (int t=0;t<(i32)threadCnt;++t) { thread_png_arenas[t].base = NULL; PngArenaInit(&thread_png_arenas[t]); }
    
    TexResult* texResults = OS_AllocScratch(texCnt * sizeof(TexResult));
    TextureParseTask tasks[32];
    OS_Thread workers[32]; _Atomic u32 shared_idx = 0;/*The shared thread counter*/
    for (int t=0;t<(i32)threadCnt;++t) {
        tasks[t] = (TextureParseTask){.texCnt = texCnt, .shared_idx = &shared_idx, .parsIdx = parsIdx, .parser = &texture_parser, .results = texResults, .tid = t};
        OS_ThreadCreate(&workers[t], TextureParsingWorker,&tasks[t]);
    }
    for (int t=0;t<(i32)threadCnt;++t) {OS_ThreadJoin(&workers[t]); }
    
    totalPixels = totalPaletteColors = 0u;
    for (u16 i=0;i<texCnt;++i) { if (texResults[i].indices) { totalPixels += (u32)texResults[i].w * texResults[i].h; totalPaletteColors += texResults[i].palSize; } }
    size_t ringpal_size = (size_t)VIDEO_RING * 256 * sizeof(u32), ringidx_size = (size_t)VIDEO_RING * VIDEO_MAXFRAME;
    size_t offsets_size = (texCnt + VIDEO_RING) * sizeof(u32), palettes_size = totalPaletteColors * sizeof(u32), indices_size = totalPixels;
    size_t arena_size = offsets_size + palettes_size + indices_size;
    void* arena = OS_AllocateRAM(arena_size, 0x1|0x2, 0x20|0x02|0x08000, INVALID_FHANDLE);
    u8* cur = (u8*)arena;
    u32* textureOffsets = (u32*)cur;
    cur += offsets_size;
    i32* textureSizes          = OS_AllocScratch((texCnt + VIDEO_RING) * 2 * sizeof(i32));
    u32* texturePaletteOffsets = OS_AllocScratch((texCnt + VIDEO_RING) * sizeof(u32));
    u32* texturePalettes = (u32*)cur;
    cur += palettes_size;
    u8* all_indices = cur;
    /*Ring lives outside the arena: this one is freed at the end of the load, the ring is refilled every played frame*/
    g_vidRingBuf = OS_Alloc(ringidx_size + ringpal_size);
    u8* ringIndices = (u8*)g_vidRingBuf; (void)ringIndices;
    u32* ringPalettes = (u32*)((u8*)g_vidRingBuf + ringidx_size);
    g_vidTexBase = (u32)ringidx_size; /*SSBO layout: [ring indices][ring palettes][texture indices], so freed-arena indices stay addressable*/
    for (u32 r=0;r<VIDEO_RING;++r) { u16 si = (u16)(texCnt + r); g_vidPalOffs[r] = totalPaletteColors + r*256; g_vidIdxOffs[r] = r * VIDEO_MAXFRAME; textureOffsets[si] = g_vidIdxOffs[r]; texturePaletteOffsets[si] = g_vidPalOffs[r]; textureSizes[si*2] = 0; textureSizes[si*2+1] = 0; g_vidSlotTex[r] = si; }
    g_vidRingSize = ringidx_size + ringpal_size; g_vidRingIdxSize = ringidx_size; g_vidRingPalSize = ringpal_size; g_vidPalBase = totalPaletteColors; g_vidSizesCPU = OS_Alloc(VIDEO_RING * 2 * sizeof(i32));
    for (u32 r=0;r<VIDEO_RING;++r) { g_vidSlot[r].idx = (u8*)g_vidRingBuf + (size_t)r*VIDEO_MAXFRAME; g_vidSlot[r].pal = ringPalettes + (size_t)r*256; }
    for (u32 k=0;k<g_vidFrameCnt;++k) { g_vidFrames[k].w = g_vidDims[g_vidFrames[k].texIdx] >> 16; g_vidFrames[k].h = g_vidDims[g_vidFrames[k].texIdx] & 0xFFFF; }
    { size_t vp=0; for (u32 k=0;k<g_vidFrameCnt;++k) vp += (size_t)g_vidFrames[k].w*g_vidFrames[k].h; DualLog("Video ring reserved: %u slots x %u B = %.2f MB (frames would have been %.2f MB)\n",VIDEO_RING,VIDEO_MAXFRAME,(double)ringidx_size/1048576.0,(double)vp/1048576.0); }
    u32 pixel_base = 0, color_base = 0;
    for (u16 i=0;i<texCnt;++i) {
        if (!texResults[i].indices) { continue; }
        
        u32 numP = (u32)texResults[i].w * texResults[i].h;
        u32 palS = texResults[i].palSize;
        textureOffsets[i]=g_vidTexBase + pixel_base;
        texturePaletteOffsets[i] = color_base;
        textureSizes[i * 2]     = texResults[i].w;
        textureSizes[i * 2 + 1] = texResults[i].h;
        mcpy(all_indices+pixel_base,texResults[i].indices,numP); 
        mcpy(texturePalettes + color_base, texResults[i].palette, palS * sizeof(u32));
        pixel_base += numP;
        color_base += palS;
        OS_Free(texResults[i].indices, numP);
        OS_Free(texResults[i].palette, palS * sizeof(u32));
    }
    
    DualLog("total palette colors: %u, total pixels: %u...", totalPaletteColors, totalPixels);
    i32 packed_size = ((i32)totalPixels + 3) / 4 * sizeof(u32);
    if (packed_size > (i32)MAX_TOTAL_PIXELS || indices_size + ringidx_size > MAX_TOTAL_PIXELS) { DualLogError("colorBufferID too small: need %zu B (textures %zu + video ring %zu), MAX_TOTAL_PIXELS is %u\n",packed_size,(size_t)indices_size,ringidx_size,MAX_TOTAL_PIXELS); OS_Exit(1); } /*colorBufferID is allocated at exactly MAX_TOTAL_PIXELS, an under-sized value makes glBufferSubData fail with GL_INVALID_VALUE and silently leaves every texture unwritten*/
    glBindBuffer(GL_SSBO, colorBufferID);
    glBufferSubData(GL_SSBO, g_vidTexBase, packed_size, all_indices);
    glBindBuffer(GL_SSBO, texPalID);
    glBufferData(GL_SSBO, (totalPaletteColors + VIDEO_RING*256) * sizeof(u32), texturePalettes, GL_STATIC_DRAW);
    glBufferSubData(GL_SSBO, totalPaletteColors * sizeof(u32), ringpal_size, ringPalettes);
    
    glBindBuffer(GL_SSBO, textureOffsetsID);
    glBufferData(GL_SSBO, (texCnt + VIDEO_RING) * sizeof(u32), textureOffsets, GL_STATIC_DRAW);
    
    glBindBuffer(GL_SSBO, textureSizesID);
    glBufferData(GL_SSBO, (texCnt + VIDEO_RING) * 2 * sizeof(i32), textureSizes, GL_STATIC_DRAW);
    
    glBindBuffer(GL_SSBO, texPalOfsID);
    glBufferData(GL_SSBO, (texCnt + VIDEO_RING) * sizeof(u32), texturePaletteOffsets, GL_STATIC_DRAW);
    
    glBindBuffer(GL_SSBO, 0);
    texCnt = (u16)(texCnt + VIDEO_RING);
    VideoInit();
    
    for (int t=0;t<(i32)threadCnt;++t) { OS_Free(thread_png_arenas[t].base, 16777216); }
    
    FHandle fp = OS_OpenReadonly(WIN_ICON);
    int windowIconFileSize = OS_FileSize(fp);
    u8* file_buffer = OS_AllocateFileBackedRAMReadonly(windowIconFileSize, fp, WIN_ICON);
    OS_Close(fp); PngArenaInit(&png_arena_main);
    int w = 1, h = 1;
    u8* pixels = PngLoad(file_buffer, windowIconFileSize, &w, &h, &png_arena_main);
    if (!pixels) { DualLogError("Failed to load icon: %s\n", WIN_ICON); OS_Exit(1); }
    
    WinSysIcon image = (WinSysIcon){w, h, pixels};
    SetWindowIcon(&image);
    OS_Free(file_buffer, windowIconFileSize);
    OS_Free(png_arena_main.base, 16777216);
    OS_Free(arena, arena_size);
    png_arena_main.base = NULL;
    OS_FreeInitPhase();
    DebugRAM("after textures load");
    DualLog(" took %.6f secs\n", get_time() - start_time);
}

typedef struct { const u16 *frames;  u8 length; bool hasGlow; const u16 *glowFrames; u8 glowLength; const char* name; } TextureAnimClip;
u16 sequenceTextures[]={
    1159,1160,881,1162,1163,1164,/*scr_exp 01 - 06*/
    1310,1311,1312,1313,/*bridg1_1 001 - 004*/
    1115,1116,/*broken_clock01_glow 01 - 02*/
    1117,1118,/*broken_clock 01 - 02*/
    1124,1125,1126,1127,1128,1129,1130,/*g_energmine 00 - 06*/
    1131,1132,1133,1134,1135,1136,1137,1138,/*g_energmine_glow 00 - 07 (yes different count, supported!)*/
    1314,1315,1316,1317,/*scr_cita2_ 0 - 3*/
    1318,1319,1320,1321,/*scr_cita3_ 0 - 3*/
    1322,1323,1324,1325,1326,1327,1328,1329,/*scr_cita_ 0 - 7*/
    1330,/*engscreen1_04 index 45*/
    0,1331,1332,1333,1334,1335,1336,1337,/*scr_static2 0 - 6, then scr_static2_a*/
    1338,1339,1340,1341,1342,1343,/*scr_static 0 - 5*/
    1344,1345,1346,1347,/*screen1 0 - 3*/
    1348,1349,1350,1351,1352,/*screen2 0 - 4*/
    1353,1354,1355,1356,/*screen3 0 - 3*/
    1357,1358,1359,1360,1361,1362,/*screen4 0 - 5*/
    1363,1364,1365,1366,/*screen5 0 - 3*/
    1367,1368,1369,1370,/*triop1 0 - 3*/
    1371,1372,1373,1374,1375,1376,1377,1378,1379,1380,/*triop2 0 - 9*/
    1381,1382,1383,1384,1385,1386,1387,1388,/*triop3 0 - 7*/
    1389,/*triop4_8 index 105*/
    1381,/*triop3_0 index 106*/
    1390,1391,1392,1393,1394,1395,1396,1397,/*dna 0 - 7*/
    1398,1399,1400,1401,/*edcolor 0 - 3*/
    1402,1403,1404,1405,/*edgray 0 - 3*/
    1406,1407,1408,1409,1410,1411,1412,1413,1414,1415,1416,/*ammo_magcart 00 - 10*/
    1417,1418,1419,1420,1421,1422,1423,1424,1425,1426,1427,/*ammo_magcart_glow 00 - 10*/
    1428,1429,1430,1431,1432,1433,1434,1435,1436,1437,/*medicalbed 00 - 9*/
    1438,1439,1440,1441,1442,/*rad1_1 00 - 04*/
    1443,1444,1445,1446,1447,1448,1449,1450,1451,1452,/*screencode 0 - 9*/
    1453,1454,1455,1456,1457,1458,1459,1460,1461,1462,1463,1464,1465,1466,1467,1468,1469,1470,1471,1472,1473,1474,1475,1476,1477,1478,1479,1480,1481,1482,1483,1484,1485,1486,1487,1488,1489,/*shodanstatic 00 - 36*/
    1166,1167,1168,1169,/*telepad 00 - 03*/ 1490,1491,1492,1493,/*telepad_00_glow*/
    0,/*black index 212*/
    0,/*black*/
    0,/*black*/
    0,/*black*/
    0,/*black*/
    0,/*black index 217*/
    1495,1496,1497,1498,1499,1500,1501,1502,1503,1504,1505,1506,/*medscreen13 00 - 11*/
    1507,1508,1509,1510,1511,1512,1513,1514,/*medscreen24 00 - 07*/
    1515,1516,1517,1518,1519,1520,1521,1522,/*medscreen16 00 - 07*/
    1523,1524,1525,1526,1527,1528,1529,1530,1531,1532,1533,1534,1535,1536,1537,1538,1539,1540,1541,1542,1543,1544,1545,1546,1547,1548,1549,1550,1551,1552,1553,1554,1555,1556,1557,1558,1559,1560,1561,1562,1563,1564,1565,1566,1567,1568,1569,1570,1571,1572,1573,1574,1575,/*zerog 00 - 52*/
    1576,1577,1578/*door_x1 01 - 03 ends at index 305*/
};

static const TextureAnimClip textureAnimClips[NUM_TEXTURE_CLIPS] = {
    /*0*/{(u16[]){6,7,8,9,9,8,7,6},8,false,NULL,0,"Bridge11"},
    /*1*/{(u16[]){10,11},2,true,(u16[]){12,13},2,"BrokenClock"},
    /*2*/{(u16[]){14,15,16,17,18,19,20},7,true,(u16[]){21,22,23,24,25,26,27,28},8,"EnergMine"},
    /*3*/{(u16[]){29,30,31,32,45,45,45,29,30,31,32,29,30,31,32,29,30,31,32,29,45,45,45,29,30,31,32,29,30,31,32,29,30,31,32,29,30,31,32,29,45,45,45},43,false,NULL,0,"EngScreen1"},
    /*4*/{(u16[]){52,51,50,49,49,50,51,52},8,false,NULL,0,"EngScreen2"},
    /*5*/{(u16[]){29,30,31,32,45,29,30,31,45,29,30,31,45},13,false,NULL,0,"ExecScreen1"},
    /*6*/{(u16[]){83,84,85,86,83,83,86,85,84,83,103,83,84,85,86,83,83,86,85,84,83},21,false,NULL,0,"ExecScreen2"},
    /*7*/{(u16[]){115,115,117},3,false,NULL,0,"ExecScreen3"},
    /*8*/{(u16[]){115,115,115,115,116,117,118},7,false,NULL,0,"ExecScreen4"},
    /*9*/{(u16[]){123,124,125,126,127,128,129,130,131,132,133},11,true,(u16[]){134,135,136,137,138,139,140,141,142,143,144},11,"MagCartridge"},
    /*10*/{(u16[]){29,30,31,32,37},5,false,NULL,0,"MaintScreen1"},
    /*11*/{(u16[]){33,34,35,36,32,29},6,false,NULL,0,"MaintScreen2"},
    /*12*/{(u16[]){145,146,147,148,149,150,151,152,153,154},10,false,NULL,0,"MedicalBed"},
    /*13*/{(u16[]){54,59,118,116,118,59},6,false,NULL,0,"MedScreen1"},
    /*14*/{(u16[]){79,80,81,82},4,false,NULL,0,"MedScreen2"},
    /*15*/{(u16[]){99,98,97,92},4,false,NULL,0,"MedScreen3"},
    /*16*/{(u16[]){29,30,31,36},4,false,NULL,0,"MedScreen4"},
    /*17*/{(u16[]){56,55,54,59,59,54,55,56},8,false,NULL,0,"MedScreen5"},
    /*18*/{(u16[]){61,61,62,62,61,61,215,216,217,218,219,220},12,false,NULL,0,"MedScreen6"},
    /*19*/{(u16[]){119,120,121,122},4,false,NULL,0,"MedScreen7"},
    /*20*/{(u16[]){59,54,55,56},4,false,NULL,0,"MedScreen8"},
    /*21*/{(u16[]){37,38,39,40,41,42,43,44},8,false,NULL,0,"MedScreen9"},
    /*22*/{(u16[]){83,84,85,86,83,83,86,85,84,83},10,false,NULL,0,"MedScreen10"},
    /*23*/{(u16[]){67,66,66,67,79,80,80,79},8,false,NULL,0,"MedScreen11"},
    /*24*/{(u16[]){221,222,223,224,225,226,227,228,229,230,231,232},12,false,NULL,0,"MedScreen13"},
    /*25*/{(u16[]){79,80,81,82,82,81,80,79},8,false,NULL,0,"MedScreen16"},
    /*26*/{(u16[]){73,74,75,76,77,78},6,false,NULL,0,"MedScreen18"},
    /*27*/{(u16[]){73,74,76,75,77,76,78,73},8,false,NULL,0,"MedScreen22"},
    /*28*/{(u16[]){29,30,31,32},4,false,NULL,0,"MedScreen23"},
    /*29*/{(u16[]){233,234,235,236,237,238,239,240},8,false,NULL,0,"MedScreen24"},
    /*30*/{(u16[]){92,93,94,95},4,false,NULL,0,"MedScreen25"},
    /*31*/{(u16[]){241,242,243,244,245,246,247,248},8,false,NULL,0,"MedScreen27"},
    /*32*/{(u16[]){64,65,66,67,68},5,false,NULL,0,"MedScreen29"},
    /*33*/{(u16[]){155,156,157,158,159},5,false,NULL,0,"Rad1_1"},
    /*34*/{(u16[]){61,61,62,62},4,false,NULL,0,"ReacScreen4"},
    /*35*/{(u16[]){62,61,60},3,false,NULL,0,"SciScreen1"},
    /*36*/{(u16[]){107,108,109,111,112,113,114},7,false,NULL,0,"SciScreen2"},
    /*37*/{(u16[]){33,34,35,36},4,false,NULL,0,"SciScreen3"},
    /*38*/{(u16[]){188,189,113,112},4,false,NULL,0,"SciScreen4"},
    /*39*/{(u16[]){79,80,80,79,73,74,76,77,75},9,false,NULL,0,"SciScreen5"},
    /*40*/{(u16[]){0,1,2,3,4,5},6,false,NULL,0,"ScreenDestroyed"},
    /*41*/{(u16[]){160,161,162,163,164,165,166,167,168,169},10,false,NULL,0,"ScreenCodeRandom"},
    /*42*/{(u16[]){29,30,31},3,false,NULL,0,"SecScreen4"},
    /*43*/{(u16[]){170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,203,204,205,206,205,203,204,206,205,204,203,206,205,203,204,205,206,203,206,205,203,204,203},60,false,NULL,0,"ShodanStatic"},
    /*44*/{(u16[]){203,204,205,206,203,206,205,203,204,205,206,203,206,205,203,204,203},17,false,NULL,0,"Static"},
    /*45*/{(u16[]){207,208,209,210},4,true,(u16[]){211,212,213,214},4,"Telepad"},
    /*46*/{(u16[]){303,304,305},3,false,NULL,0,"XDoor1"},
    /*47*/{(u16[]){249,250,251,252,253,254,255,256,257,258,259,260,261,262,263,264,265,266,267,268,269,270,271,272,273,274,275,276,277,278,279,280,281,282,283,284,285,286,287,288,289,290,291,292,293,294,295,296,297,298,299,300,301,302},54,false,NULL,0,"ZeroGMutant"},
    /*48*/{(u16[]){287,288,289,290,291,292,293,294,295,296,297,298,299,300,301,302},16,false,NULL,0,"ZeroGMutantDeath"},
};

INLINE u16 ScreenCodeTexture(u8 digit) { return sequenceTextures[textureAnimClips[41].frames[digit%10u]]; }
void CodeScreenShowDigit(u16 self, u8 digit) { Entity* e = &World.instances[self]; e->textureAnimating = false; e->texAnimRandom = false; e->texIndex = ScreenCodeTexture(digit); }
static float TextureClipPeriod(u16 clip) {
    return (clip == 5 || clip == 6 || clip == 1)
           ? 0.5f /*ExecScreen1, ExecScreen2, BrokenClock*/
           : (clip == 41 || clip == 43)
             ? 0.3f /*ScreenCodeRandom, ShodanStatic*/
             : (clip == 44)
               ? 0.2f /*Static*/
               : (clip == 9)
                 ? 0.17f /*MagCartridge*/
                 : (clip == 47 || clip == 48)
                   ? 0.04166f /*ZeroGMutant, ZeroGMutantDeath*/
                   : 0.35f;
}

void TextureSequenceInit(u16 self, char* trimmed_value) {
    Entity* e=&World.instances[self];
    if (trimmed_value[0] == '\0') { e->textureAnimating = false; e->modelIndex = EDefs[e->index].modelIndex; return;}
    
    e->textureAnimating = true;
    e->textureGlowAnimating = false;
    e->texAnimLight = e->texAnimLight2 = U16_MAX;
    e->texFrame = e->texGlowFrame = 0;
    if (sEqual(trimmed_value,"ScreenDestroyed")) { World.instances[self].texAnimClip = NUM_TEXTURE_CLIPS - 1; return; }
    
    {
        size_t vlen=slen(trimmed_value);
        size_t d=vlen;
        while(d>0&&trimmed_value[d-1]>='0'&&trimmed_value[d-1]<='9'){--d;}
        if(vlen - d >= 1 && vlen - d <= 2 && d >= 7 && sCompUpToLen(trimmed_value + d - 7,"CamView",7) == 0) {
            int cvn = 0;
            for (size_t k=d;k<vlen;++k) { cvn = cvn * 10 + (trimmed_value[k] - '0'); }
            if (cvn >= 1 && cvn <= 64) { e->textureAnimating = false; e->camView = (u8)(cvn - 1); return;} /*<prefix>CamView<N>, e.g. SecCamView3: show this level's camera N - 1*/
        }
    }
    
    for(int i=0;i<NUM_TEXTURE_CLIPS;++i){
        if (sEqual(trimmed_value,textureAnimClips[i].name)) { World.instances[self].texAnimClip = i; e->textureGlowAnimating = textureAnimClips[i].hasGlow; return; }
    }
    
    e->textureAnimating = false; // Couldn't find match, just don't animate.
}

void TextureSequenceStart(u16 self, u8 clipIndex) {
    Entity* e = &World.instances[self];
    if (clipIndex >= NUM_TEXTURE_CLIPS) return;
    
    const TextureAnimClip* clip = &textureAnimClips[clipIndex];
    e->texAnimClip = clipIndex;
    e->texFrame = e->texGlowFrame = 0;
    e->textureAnimating = e->texAnimStopsAtDie = true;
    e->textureGlowAnimating = clip->hasGlow;
    e->texAnimRandom = e->texAnimInReverse = false;
    e->texIndex=sequenceTextures[clip->frames[0]];
    if (clip->hasGlow && clip->glowFrames) e->glowIndex = sequenceTextures[clip->glowFrames[0]];
    e->tickFinished = World.pauseRelativeTime+TextureClipPeriod(clipIndex);
}

void TextureSequenceUpdate(u16 self) {
    Entity* e = &World.instances[self];
    u16 tClip = e->texAnimClip;
    if (tClip >= NUM_TEXTURE_CLIPS) {
        e->textureAnimating = false;
    } else {
        const TextureAnimClip* clip = &textureAnimClips[tClip];
        bool stopAtDie = e->texAnimStopsAtDie && e->health <= 0.0f;
        e->tickFinished = World.pauseRelativeTime+TextureClipPeriod(tClip); 
        if (e->texAnimRandom && (!stopAtDie)) {
            e->texFrame = random_range_u32(0,clip->length-1);
            if (clip->hasGlow) { e->texGlowFrame=random_range_u32(0,clip->glowLength-1); }
        } else if (e->texAnimInReverse) {
            if (stopAtDie) {
                if (e->texFrame > 0) e->texFrame--;
            } else e->texFrame = (e->texFrame == 0) ? clip->length-1 : e->texFrame-1;
            
            if (clip->hasGlow) { e->texGlowFrame = (e->texGlowFrame == 0) ? clip->glowLength - 1 : e->texGlowFrame - 1;}
        } else {
            if (stopAtDie) {
                if (e->texFrame < clip->length - 1) { e->texFrame++; }
            } else e->texFrame = (e->texFrame+1) % clip->length;
            
            if (clip->hasGlow) { e->texGlowFrame = (e->texGlowFrame + 1) % clip->glowLength;}
        }
        
        if (stopAtDie && e->texFrame >= clip->length - 1) {
            e->textureAnimating=false;
            if(e->texAnimLight  < World.loadedLights){flag_set(&World.lights[e->texAnimLight].lflags,LIGHTON,false);}
            if(e->texAnimLight2 < World.loadedLights){flag_set(&World.lights[e->texAnimLight2].lflags,LIGHTON,false);}
        }
        
        e->texIndex = sequenceTextures[clip->frames[e->texFrame]];
        if (clip->hasGlow && clip->glowFrames) { e->glowIndex = sequenceTextures[clip->glowFrames[e->texGlowFrame]]; }
        if (e->index == 279 && !clip->hasGlow) { e->glowIndex = e->texIndex; }
    }
}
