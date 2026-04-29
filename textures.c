#include "os.h"
#include "gl.h"
#include "voxen.h"
extern u16 loadedTexturesMaxIndex;
u32 totalPixels;
u32 totalPaletteColors;
#define STBI_ARENA_SIZE 16*1024*1024
typedef struct { u16 index; bool transparent; bool doublesided; char path[128]; } TextureData;
typedef struct { TextureData* entries; u32 count; u32 capacity; } TextureDataParser;
typedef struct { const char* data; int size; } RawTexture;
typedef struct TextureParseTask { u32 start_tex; u32 end_tex; RawTexture* raw_textures; i32* parsIdx; const TextureDataParser* parser; int tid; } TextureParseTask;
typedef struct { u32 img_x, img_y; i32 img_n, img_out_n; u8* img_buffer, *img_buffer_end; } stbi__context;
typedef struct { stbi__context* s; u8* idata, *expanded, *out; } stbi__png;
enum { STBI__F_none = 0, STBI__F_sub = 1, STBI__F_up = 2, STBI__F_avg = 3, STBI__F_paeth = 4, STBI__F_avg_first, STBI__F_paeth_first };
typedef struct { u16 fast[1<<9], firstcode[16], firstsymbol[16], value[288]; i32 maxcode[17]; u8 size[288]; } stbi__zhuffman;
typedef struct { u8 *zbuffer, *zbuffer_end, *zout, *zout_start; i32 num_bits; u32 code_buffer; stbi__zhuffman z_length, z_distance; } stbi__zbuf;
static int num_parse_threads = 0;
u8* stbi_load_from_memory(const u8* buffer, i32 len, i32* x, i32* y);
StbiArena stbi_arena_main;
static StbiArena* thread_stbi_arenas = NULL;
static u8** textureIndexBuffers = NULL; static u32** texturePaletteBuffers = NULL; static u32* texturePaletteSizes = NULL;
static i32* textureWidths = NULL; static i32* textureHeights = NULL;

void stbi__arena_init_thread(StbiArena* arena) {if (!arena->base) { arena->base = OS_Alloc(STBI_ARENA_SIZE); arena->cursor = arena->base; arena->end = arena->base + STBI_ARENA_SIZE; } }
void* stbi__arena_alloc_thread(StbiArena* a, size_t s) { if(!a->base||a->cursor+s>a->end)return NULL; void* p=a->cursor; a->cursor+=s; return p; }
void* stbi__arena_alloc(size_t s) { return stbi__arena_alloc_thread(&stbi_arena_main, s); }
static u32 stbi__get32be(stbi__context* s) { const u8* p = s->img_buffer; s->img_buffer += 4; return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]; }
static i32 stbi__bit_reverse(i32 n, i32 b) {
    n=((n&0xAAAA)>>1)|((n&0x5555)<<1); n=((n&0xCCCC)>>2)|((n&0x3333)<<2);
    n=((n&0xF0F0)>>4)|((n&0x0F0F)<<4); n=((n&0xFF00)>>8)|((n&0x00FF)<<8);
    return n>>(16-b);
}

static i32 stbi__zbuild_huffman(stbi__zhuffman* z, const u8* sl, i32 num) {
    i32 i, k=0, code=0, nc[16], sz[17]={0}; __builtin_memset(z->fast, 0, sizeof(z->fast));
    if(num!=32){ for(i=0;i<num;++i)++sz[sl[i]]; } sz[0]=0;
    for(i=1;i<16;++i){
        if(sz[i]>(1<<i))return 0;
        
        nc[i]=code; z->firstcode[i]=(u16)code; z->firstsymbol[i]=(u16)k; code+=sz[i];
        if(sz[i]&&code-1>=(1<<i))return 0;
        
        z->maxcode[i]=code<<(16-i); code<<=1; k+=sz[i];
    }
    z->maxcode[16]=0x10000;
    for(i=0;i<num;++i){ 
        int s=(num==32)?5:sl[i]; if(!s)continue;
        int c=nc[s]-z->firstcode[s]+z->firstsymbol[s]; u16 fv=(u16)((s<<9)|i); z->size[c]=(u8)s; z->value[c]=(u16)i;
        if(s<=9){ int j=stbi__bit_reverse(nc[s],s); while(j<(1<<9)){z->fast[j]=fv; j+=(1<<s);} } ++nc[s];
    } return 1;
}

#define REFILL(z) if(z->num_bits<16){do{z->code_buffer|=(unsigned int)(*z->zbuffer++)<<z->num_bits;z->num_bits+=8;}while(z->num_bits<=24);}
static u32 stbi__zreceive(stbi__zbuf* z, int n) { REFILL(z); u32 k=z->code_buffer&((1u<<n)-1); z->code_buffer>>=n; z->num_bits-=n; return k; }
static u32 stbi__zhuffman_decode(stbi__zbuf* a, stbi__zhuffman* z) {
    REFILL(a); int b=z->fast[a->code_buffer&511], s;
    if(b){ s=b>>9; a->code_buffer>>=s; a->num_bits-=s; return b&511; }
    int k=stbi__bit_reverse(a->code_buffer,16); for(s=10; k>=z->maxcode[s]; ++s);
    b=(k>>(16-s))-z->firstcode[s]+z->firstsymbol[s]; a->code_buffer>>=s; a->num_bits-=s; return z->value[b];
}

static int stbi__parse_huffman_block(stbi__zbuf* a) {
    u8* o=a->zout;
    static const int lb[]={3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227,258},
                     le[]={0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0},
                     db[]={1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577},
                     de[]={0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};
    for(;;){
        int z=stbi__zhuffman_decode(a,&a->z_length); if(z<256)*o++=(u8)z; else if(z==256){ a->zout=o; return 1; }
        else { z-=257; int l=lb[z]+(le[z]?stbi__zreceive(a,le[z]):0); z=stbi__zhuffman_decode(a,&a->z_distance); int d=db[z]+(de[z]?stbi__zreceive(a,de[z]):0); u8* p=o-d; while(l--)*o++=*p++; }
    }
}

static int stbi__compute_huffman_codes(stbi__zbuf* a) {
    static const u8 dz[]={16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15}; u8 lc[286+32+137], cs[19]={0};
    u32 hl=stbi__zreceive(a,5)+257, hd=stbi__zreceive(a,5)+1, hc=stbi__zreceive(a,4)+4, nt=hl+hd, n=0;
    for(u32 i=0;i<hc;++i) cs[dz[i]]=(u8)stbi__zreceive(a,3); stbi__zbuild_huffman(&a->z_length,cs,19);
    while(n<nt){
        u32 c=stbi__zhuffman_decode(a,&a->z_length);
        if(c<16)lc[n++]=(u8)c;
        else { u8 f=0;
            if(c==16){c=stbi__zreceive(a,2)+3; f=lc[n-1];}
            else if(c==17)c=stbi__zreceive(a,3)+3;
            else if(c==18)c=stbi__zreceive(a,7)+11;
            else return 0; __builtin_memset(lc+n,f,c); n+=c;
        }
    } return stbi__zbuild_huffman(&a->z_length,lc,hl) && stbi__zbuild_huffman(&a->z_distance,lc+hl,hd);
}

static int stbi__parse_uncompressed_block(stbi__zbuf* a) {
    u8 header[4]; i32 k = 0;
    if (a->num_bits & 7) stbi__zreceive(a, a->num_bits & 7);
    while (a->num_bits > 0) { header[k] = (u8)(a->code_buffer & 255); a->code_buffer >>= 8; a->num_bits -= 8; ++k; }
    if (k <= 0) header[0] = *a->zbuffer++;
    if (k <= 1) header[1] = *a->zbuffer++;
    if (k <= 2) header[2] = *a->zbuffer++;
    if (k <= 3) header[3] = *a->zbuffer++;
    i32 len = header[1] * 256 + header[0];
    __builtin_memcpy(a->zout, a->zbuffer, len);
    a->zbuffer += len; a->zout += len;
    return 1;
}

static u8 stbi__zdef_len(int i) { return (i<144)?8:(i<256)?9:(i<280)?7:8; }
u8* stbi_zlib_decode_malloc_guesssize_headerflag(const u8* buffer, i32 len, i32 initial_size, i32* outlen) {
    stbi__zbuf a = {0}; u8* p = (u8*)stbi__arena_alloc(initial_size), d_len[288]; i32 f, t;
    a.zbuffer = (u8*)buffer; a.zbuffer_end = (u8*)buffer+len; a.zout_start = a.zout = p; a.zbuffer += 2;
    do {
        f = stbi__zreceive(&a, 1); t = stbi__zreceive(&a, 2);
        if (t == 0) stbi__parse_uncompressed_block(&a);
        else {
            if (t == 1) { for(int i=0; i<288; ++i) d_len[i]=stbi__zdef_len(i); stbi__zbuild_huffman(&a.z_length, d_len, 288); stbi__zbuild_huffman(&a.z_distance, NULL, 32); }
            else stbi__compute_huffman_codes(&a);
            stbi__parse_huffman_block(&a);
        }
    } while (!f);
    if (outlen) *outlen = (i32)(a.zout - a.zout_start); return a.zout_start;
}

u8* stbi_zlib_decode_malloc_guesssize_headerflag_arena(const u8* buffer, i32 len, i32 initial_size, i32* outlen, StbiArena* arena) {
    stbi__zbuf a = {0}; u8* p = (u8*)stbi__arena_alloc_thread(arena, initial_size), d_len[288]; i32 f, t;
    a.zbuffer = (u8*)buffer; a.zbuffer_end = (u8*)buffer+len; a.zout_start = a.zout = p; a.zbuffer += 2;
    do {
        f = stbi__zreceive(&a, 1); t = stbi__zreceive(&a, 2);
        if (t == 0) stbi__parse_uncompressed_block(&a);
        else {
            if (t == 1) { for(int i=0; i<288; ++i) d_len[i]=stbi__zdef_len(i); stbi__zbuild_huffman(&a.z_length, d_len, 288); stbi__zbuild_huffman(&a.z_distance, NULL, 32); }
            else stbi__compute_huffman_codes(&a);
            stbi__parse_huffman_block(&a);
        }
    } while (!f);
    if (outlen) *outlen = (i32)(a.zout - a.zout_start); return a.zout_start;
}

static u8 first_row_filter[5] = {STBI__F_none, STBI__F_sub, STBI__F_none, STBI__F_avg_first, STBI__F_paeth_first};
inline static i32 stbi__paeth(i32 a, i32 b, i32 c) { i32 p = a+b-c, pa = vabs(p-a), pb = vabs(p-b), pc = vabs(p-c); return (pa <= pb && pa <= pc) ? a : (pb <= pc ? b : c); }
static i32 stbi__create_png_image_raw_arena(StbiArena* arena, stbi__png* a, u8* raw, u32 raw_len, i32 out_n, u32 x, u32 y, i32 img_n) {
    u32 i, j, stride = x * out_n, w_bytes = (img_n * x * 8 + 7) >> 3; i32 k, f;
    if (raw_len < (w_bytes + 1) * y) return 0; a->out = (u8*)stbi__arena_alloc_thread(arena, (size_t)x * y * out_n);
    for (j = 0; j < y; ++j) {
        u8 *cur = a->out + stride * j, *prior = (j > 0) ? cur - stride : a->out;
        if ((f = *raw++) > 4) return 0; if (j == 0) f = first_row_filter[f];
        for (k = 0; k < img_n; ++k) {
            if (f == STBI__F_up) cur[k] = raw[k] + prior[k];
            else if (f == STBI__F_avg) cur[k] = raw[k] + (prior[k] >> 1);
            else if (f == STBI__F_paeth) cur[k] = raw[k] + stbi__paeth(0, prior[k], 0);
            else cur[k] = raw[k];
        }
        if (img_n != out_n) cur[img_n] = 255; raw += img_n; cur += out_n; prior += out_n;
        for (i = x - 1; i >= 1; --i, cur[img_n] = (img_n != out_n ? 255 : cur[img_n]), raw += img_n, cur += out_n, prior += out_n)
            for (k = 0; k < img_n; ++k) {
                if (f == STBI__F_none) cur[k] = raw[k];
                else if (f == STBI__F_sub) cur[k] = raw[k] + cur[k - out_n];
                else if (f == STBI__F_up) cur[k] = raw[k] + prior[k];
                else if (f == STBI__F_avg) cur[k] = raw[k] + ((prior[k] + cur[k - out_n]) >> 1);
                else if (f == STBI__F_paeth) cur[k] = raw[k] + stbi__paeth(cur[k - out_n], prior[k], prior[k - out_n]);
                else if (f == STBI__F_avg_first) cur[k] = raw[k] + (cur[k - out_n] >> 1);
            }
    }
    return 1;
}

static i32 stbi__create_png_image_raw(stbi__png* a, u8* raw, u32 raw_len, i32 out_n, u32 x, u32 y, i32 img_n) {
    u32 i, j, stride = x * out_n, w_bytes = (img_n * x * 8 + 7) >> 3; i32 k, f;
    if (raw_len < (w_bytes + 1) * y) return 0; a->out = (u8*)stbi__arena_alloc((size_t)x * y * out_n);
    for (j = 0; j < y; ++j) {
        u8 *cur = a->out + stride * j, *prior = (j > 0) ? cur - stride : a->out;
        if ((f = *raw++) > 4) return 0; if (j == 0) f = first_row_filter[f];
        for (k = 0; k < img_n; ++k) {
            if (f == STBI__F_up) cur[k] = raw[k] + prior[k];
            else if (f == STBI__F_avg) cur[k] = raw[k] + (prior[k] >> 1);
            else if (f == STBI__F_paeth) cur[k] = raw[k] + stbi__paeth(0, prior[k], 0);
            else cur[k] = raw[k];
        }
        if (img_n != out_n) cur[img_n] = 255; raw += img_n; cur += out_n; prior += out_n;
        for (i = x - 1; i >= 1; --i, cur[img_n] = (img_n != out_n ? 255 : cur[img_n]), raw += img_n, cur += out_n, prior += out_n)
            for (k = 0; k < img_n; ++k) {
                if (f == STBI__F_none) cur[k] = raw[k];
                else if (f == STBI__F_sub) cur[k] = raw[k] + cur[k - out_n];
                else if (f == STBI__F_up) cur[k] = raw[k] + prior[k];
                else if (f == STBI__F_avg) cur[k] = raw[k] + ((prior[k] + cur[k - out_n]) >> 1);
                else if (f == STBI__F_paeth) cur[k] = raw[k] + stbi__paeth(cur[k - out_n], prior[k], prior[k - out_n]);
                else if (f == STBI__F_avg_first) cur[k] = raw[k] + (cur[k - out_n] >> 1);
            }
    }
    return 1;
}

u8* stbi_load_from_memory_arena(const u8* buffer, int len, int* x, int* y, StbiArena* arena) {
    if (arena->base) arena->cursor = arena->base;
    stbi__context s; s.img_n = s.img_out_n = 0; s.img_buffer = (u8*)buffer; s.img_buffer_end = (u8*)buffer + len;
    stbi__png z = {0}; z.s = &s; u32 ioff = 0; z.expanded = z.idata = z.out = NULL;
    s.img_buffer += 8; s.img_x = s.img_y = 1;
    for (;;) {
        u32 length = stbi__get32be(&s), type = stbi__get32be(&s);
        switch (type) {
            case 0x49484452: s.img_x = stbi__get32be(&s); s.img_y = stbi__get32be(&s); s.img_buffer++; { i32 color = (*s.img_buffer++); s.img_buffer += 3; s.img_n = (color & 2 ? 3 : 1) + (color & 4 ? 1 : 0); } break;
            case 0x49444154: if (!z.idata) { z.idata = (u8*)stbi__arena_alloc_thread(arena, len + 16); ioff = 0; } __builtin_memcpy(z.idata + ioff, s.img_buffer, length); s.img_buffer += length; ioff += length; break;
            case 0x49454E44: { u32 rL = s.img_x * s.img_y * s.img_n + s.img_y; z.expanded = (u8*)stbi_zlib_decode_malloc_guesssize_headerflag_arena(z.idata, ioff, rL, (i32*)(&rL), arena);
                s.img_out_n = (s.img_n + 1 == 4) ? 4 : s.img_n; stbi__create_png_image_raw_arena(arena, &z, z.expanded, rL, s.img_out_n, s.img_x, s.img_y, s.img_n); stbi__get32be(&s); goto Label_parsesuccess; }
            default: s.img_buffer += length; break;
        }
        stbi__get32be(&s);
    }
    Label_parsesuccess: *x = z.s->img_x; *y = z.s->img_y; return z.out;
}

extern u8* stbi_load_from_memory(const u8* buffer, int len, int* x, int* y) {
    if (stbi_arena_main.base) stbi_arena_main.cursor = stbi_arena_main.base;
    stbi__context s; s.img_n = s.img_out_n = 0; s.img_buffer = (u8*)buffer; s.img_buffer_end = (u8*)buffer + len;
    stbi__png z = {0}; z.s = &s; u32 ioff = 0; z.expanded = z.idata = z.out = NULL;
    s.img_buffer += 8; s.img_x = s.img_y = 1;
    for (;;) {
        u32 length = stbi__get32be(&s), type = stbi__get32be(&s);
        switch (type) {
            case 0x49484452: s.img_x = stbi__get32be(&s); s.img_y = stbi__get32be(&s); s.img_buffer++; { i32 color = (*s.img_buffer++); s.img_buffer += 3; s.img_n = (color & 2 ? 3 : 1) + (color & 4 ? 1 : 0); } break;
            case 0x49444154: if (!z.idata) { z.idata = (u8*)stbi__arena_alloc(len + 16); ioff = 0; } __builtin_memcpy(z.idata + ioff, s.img_buffer, length); s.img_buffer += length; ioff += length; break;
            case 0x49454E44: { u32 rL = s.img_x * s.img_y * s.img_n + s.img_y; z.expanded = (u8*)stbi_zlib_decode_malloc_guesssize_headerflag(z.idata, ioff, rL, (i32*)(&rL));
                s.img_out_n = (s.img_n + 1 == 4) ? 4 : s.img_n; stbi__create_png_image_raw(&z, z.expanded, rL, s.img_out_n, s.img_x, s.img_y, s.img_n); stbi__get32be(&s); goto Label_parsesuccess; }
            default: s.img_buffer += length; break;
        }
        stbi__get32be(&s);
    }
    Label_parsesuccess: *x = z.s->img_x; *y = z.s->img_y; return z.out;
}

extern bool doubleSidedTexture[MAX_VALID_TEXTURE], transparentTexture[MAX_VALID_TEXTURE];
static void* TextureParsingWorker(void* arg) {
    TextureParseTask* t = (TextureParseTask*)arg;
    for (u32 i = t->start_tex; i < t->end_tex; ++i) {
        i32 pIdx = t->parsIdx[i]; if (unlikely(pIdx < 0 || pIdx >= (i32)t->parser->count)) continue;
        doubleSidedTexture[i] = t->parser->entries[pIdx].doublesided; transparentTexture[i] = t->parser->entries[pIdx].transparent;
        const char* d = t->raw_textures[i].data; int sz = t->raw_textures[i].size; if (unlikely(!d || sz <= 0)) continue;
        int w=0, h=0; u8 *pix = stbi_load_from_memory_arena((const u8*)d, sz, &w, &h, &thread_stbi_arenas[t->tid]);
        if (!pix || w < 1 || h < 1) { OS_DeallocateRAM((void*)d, (size_t)sz); continue; }
        u32 nP = (u32)w * h, pSz = 0, *pal = (u32*)OS_Alloc(1024); u8 *idx = (u8*)OS_Alloc(nP), hash[1024] = {0};
        for (u32 p = 0; p < nP; ++p) {
            u32 c = ((u32*)pix)[p], s = (c * 0x9e3779b9u) & 1023;
            while (hash[s]) { if (pal[hash[s]-1] == c) { idx[p] = hash[s]-1; goto found; } s = (s+1) & 1023; }
            if (pSz >= 256) {
                u32 bIdx = 0, bDist = -1; u8 r1=c, g1=c>>8, b1=c>>16, a1=c>>24;
                for (u32 k=0; k<pSz; k++) {
                    i32 dr=(pal[k]&255)-r1, dg=((pal[k]>>8)&255)-g1, db=((pal[k]>>16)&255)-b1, da=(pal[k]>>24)-a1;
                    u32 dst = dr*dr + dg*dg + db*db + da*da; if (dst < bDist) { bDist = dst; bIdx = k; }
                }
                idx[p] = bIdx; continue;
            }
            pal[pSz] = c; idx[p] = pSz; hash[s] = (u8)(++pSz); found:;
        }
        textureIndexBuffers[i] = idx; texturePaletteBuffers[i] = pal; texturePaletteSizes[i] = pSz; textureWidths[i] = w; textureHeights[i] = h;
        OS_DeallocateRAM((void*)d, (size_t)sz);
    }
    return NULL;
}

static bool ParseTextureData(TextureDataParser *p, u16 maxS, const char *fn) {
    OsFileHandle fd; int sz; char *data = OS_OpenAndAllocateFileBufferReadonly(fn, &fd, &sz), *cur = data, *end = data + sz;
    u32 line = 0, m_idx = 0;
    while (cur < end) {
        char *s = cur; while (cur < end && *cur != '\n' && *cur != '\r') cur++;
        size_t len = cur - s; line++; if (len <= 0) { cur++; continue; }
        while (CharacterIsEmpty(*s)) s++; char *le = s + (cur - s) - 1;
        while (le > s && CharacterIsEmpty(*le)) le--;
        if (*s == '\0' || (s[0] == '/' && s[1] == '/') || s[0] == '#') { if (cur < end && (*cur == '\r' || *cur == '\n')) cur++; continue; }
        char *col = StringFindFirstCharWithin(s, ':');
        if (col && StringCompareUpToLength(s, "index", col - s) == 0) {
            char *v = col + 1; while (CharacterIsEmpty(*v)) v++;
            u32 idx = parse_numberu32(v, s, line); if (idx > m_idx) m_idx = idx;
        }
        if (cur < end && (*cur == '\r' || *cur == '\n')) cur++;
    }
    if (!m_idx || m_idx >= maxS) { if (!m_idx) DualLogWarn("No entries in %s\n", fn); else DualLogWarn("Index %u too large in %s\n", m_idx, fn); OS_DeallocateRAM(data, sz); return true; }
    p->entries = OS_Alloc((p->count = p->capacity = m_idx + 1) * sizeof(TextureData));
    for (u32 i = 0; i < p->count; ++i) p->entries[i] = (TextureData){.index = U16_MAX};
    TextureData e = {.index = U16_MAX}; line = 0; cur = data;
    while (cur < end) {
        char *s = cur; while (cur < end && *cur != '\n' && *cur != '\r') cur++;
        size_t len = cur - s; line++; if (len < 3) { cur++; continue; }
        while (CharacterIsEmpty(*s)) s++; char *le = s + (cur - s) - 1; while (le > s && CharacterIsEmpty(*le)) le--;
        if (s[0] == '/' && s[1] == '/') { cur++; continue; }
        if (*s == '#') {
            if (e.path[0] && e.index < p->capacity) p->entries[e.index] = e;
            e = (TextureData){.index = U16_MAX}; size_t aL = le - s;
            if (aL >= sizeof(e.path)) aL = sizeof(e.path) - 1; __builtin_memcpy(e.path,s + 1,aL); e.path[aL] = 0;
        } else {
            char *col = StringFindFirstCharWithin(s, ':');
            if (col) {
                char *k = s, *v = col + 1, tk[256] = {0}, tv[256] = {0};
                while (CharacterIsEmpty(*k) && k < col) k++; while (CharacterIsEmpty(*v) && v <= le) v++;
                size_t kL = col - k, vL = (le >= v) ? (le - v + 1) : 0;
                if (kL && vL) {
                    StringCopyInto_A_SubstringFrom_B(tk, kL, k, 256); StringCopyInto_A_SubstringFrom_B(tv,vL,v,256);
                    char *ke = tk + GetStringLength(tk) - 1, *ve = tv + GetStringLength(tv) - 1;
                    while (ke > tk && CharacterIsEmpty(*ke)) *ke-- = 0; while (ve > tv && CharacterIsEmpty(*ve)) *ve-- = 0;
                         if (StringsEqual(tk,       "index")) e.index       = parse_numberu16(tv,s,line);
                    else if (StringsEqual(tk, "transparent")) e.transparent = parse_bool(tv,s,line);
                    else if (StringsEqual(tk, "doublesided")) e.doublesided = parse_bool(tv,s,line);
                }
            }
        }
        if (cur < end && (*cur == '\r' || *cur == '\n')) cur++;
    }
    if (e.path[0] && e.index < p->capacity) p->entries[e.index] = e;
    OS_DeallocateRAM(data, sz); return true;
}

void glfwSetWindowIcon(GLFWwindow* handle, const GLFWimage* images); extern GLFWwindow* window;
void LoadTextures(void) {
    double start_time = get_time();
    loadedTexturesMaxIndex = totalPixels = totalPaletteColors = 0u;
    TextureDataParser texture_parser;
    if (unlikely(!ParseTextureData(&texture_parser, MAX_VALID_TEXTURE, "./Data/textures.txt"))) { DualLogError("Could not parse ./Data/textures.txt!\n"); OS_Exit(1); }

    i32 maxIndex = -1;
    for (u32 k = 0; k < texture_parser.count; ++k) {
        if (texture_parser.entries[k].index > maxIndex && texture_parser.entries[k].index != U16_MAX) maxIndex = texture_parser.entries[k].index;
    }
    loadedTexturesMaxIndex = (u16)(maxIndex + 1);
    if (loadedTexturesMaxIndex == 0) { DualLogError("No textures found in textures.txt\n"); OS_Exit(1); }

    i32* parsIdx = OS_Alloc(loadedTexturesMaxIndex * sizeof(i32));
    __builtin_memset(parsIdx, -1, loadedTexturesMaxIndex * sizeof(i32));
    for (u32 k = 0; k < texture_parser.count; ++k) {
        if (texture_parser.entries[k].index < loadedTexturesMaxIndex) parsIdx[texture_parser.entries[k].index] = (i32)k;
    }

    DualLog("Loading textures (%u) ... ", texture_parser.count);
    RawTexture* rawTextures = OS_Alloc(loadedTexturesMaxIndex * sizeof(RawTexture));
    __builtin_memset(rawTextures,0,loadedTexturesMaxIndex * sizeof(RawTexture));
    for (u32 i = 0; i < loadedTexturesMaxIndex; ++i) {
        i32 p = parsIdx[i]; if (p < 0) continue;
        const char* path = texture_parser.entries[p].path;
        OsFileHandle dummy_fd;
        int size = 0; rawTextures[i].data = (const char*)OS_OpenAndAllocateFileBufferReadonly(path,&dummy_fd,&size);
        rawTextures[i].size = size;
    }

    num_parse_threads = OS_GetNumThreads();
    if (num_parse_threads < 1) num_parse_threads = 1;
    if (num_parse_threads > 32) num_parse_threads = 32;
    thread_stbi_arenas = (StbiArena*)OS_Alloc((size_t)num_parse_threads * sizeof(StbiArena));
    for (int t = 0; t < num_parse_threads; ++t) { thread_stbi_arenas[t].base = NULL; stbi__arena_init_thread(&thread_stbi_arenas[t]); }
    textureIndexBuffers = OS_Alloc(loadedTexturesMaxIndex * sizeof(u8*));
    texturePaletteBuffers = OS_Alloc(loadedTexturesMaxIndex * sizeof(u32*));
    texturePaletteSizes = OS_Alloc(loadedTexturesMaxIndex * sizeof(u32));
    textureWidths = OS_Alloc(loadedTexturesMaxIndex * sizeof(i32));
    textureHeights = OS_Alloc(loadedTexturesMaxIndex * sizeof(i32));
    TextureParseTask tasks[32]; u32 chunk = (loadedTexturesMaxIndex + (u32)num_parse_threads - 1U) / (u32)num_parse_threads;
    for (int t = 0; t < num_parse_threads; ++t) {
        u32 start = ((u32)t * chunk);
        tasks[t] = (TextureParseTask){.start_tex=start,.end_tex=clamp(start+chunk,0,loadedTexturesMaxIndex),.raw_textures=rawTextures,.parsIdx=parsIdx,.parser=&texture_parser,.tid=t};
    }

    pthread_t workers[32];
    for (int t = 0; t < num_parse_threads; ++t) pthread_create(&workers[t], NULL, TextureParsingWorker, &tasks[t]);
    for (int t = 0; t < num_parse_threads; ++t) pthread_join(workers[t], NULL);
    totalPixels = totalPaletteColors = 0u;
    for (u16 i = 0; i < loadedTexturesMaxIndex; ++i) {
        if (textureIndexBuffers[i]) { totalPixels += (u32)textureWidths[i] * textureHeights[i]; totalPaletteColors += texturePaletteSizes[i]; }
    }

    size_t offsets_size = loadedTexturesMaxIndex * sizeof(u32);
    size_t palettes_size = totalPaletteColors * sizeof(u32);
    size_t indices_size = totalPixels;
    size_t arena_size = offsets_size + palettes_size + indices_size;
    void* arena = OS_AllocateRAM(NULL,arena_size,PROT_READ|PROT_WRITE,MAP_ANONYMOUS|MAP_PRIVATE|MAP_POPULATE,OS_INVALID_HANDLE);
    u8* cur = (u8*)arena;
    u32* textureOffsets = (u32*)cur; cur += offsets_size;
    i32* textureSizes = OS_AllocateRAM(NULL,loadedTexturesMaxIndex * 2 * sizeof(i32),PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,OS_INVALID_HANDLE);
    u32* texturePaletteOffsets = OS_AllocateRAM(NULL,loadedTexturesMaxIndex * sizeof(u32),PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,OS_INVALID_HANDLE);
    u32* texturePalettes = (u32*)cur; cur += palettes_size;
    u8* all_indices = cur;
    u32 pixel_base = 0, color_base = 0;
    for (u16 i=0;i<loadedTexturesMaxIndex;++i) {
        if (!textureIndexBuffers[i]) continue;
        u32 numP = (u32)textureWidths[i] * textureHeights[i];
        u32 palS = texturePaletteSizes[i];
        textureOffsets[i] = pixel_base;
        texturePaletteOffsets[i] = color_base;
        textureSizes[i*2]     = textureWidths[i];
        textureSizes[i*2 + 1] = textureHeights[i];
        __builtin_memcpy(all_indices + pixel_base,textureIndexBuffers[i],numP);
        __builtin_memcpy(texturePalettes + color_base,texturePaletteBuffers[i],palS * sizeof(u32));
        pixel_base += numP;
        color_base += palS;
        OS_DeallocateRAM(textureIndexBuffers[i],numP);
        OS_DeallocateRAM(texturePaletteBuffers[i],palS * sizeof(u32));
    }

    DualLog("total palette colors: %u, total pixels: %u...", totalPaletteColors,totalPixels);
    i32 packed_size = ((i32)totalPixels + 3) / 4 * sizeof(u32);
    glBindBuffer(GL_SSBO,Sys_Render.colorBufferID);
    void* dst = glMapBufferRange(GL_SSBO,0,packed_size,0x0002/*GL_MAP_WRITE_BIT*/|0x0004/*GL_MAP_INVALIDATE_RANGE_BIT*/);
    __builtin_memcpy(dst,all_indices,packed_size);
    glUnmapBuffer(GL_SSBO);
    glBindBuffer(GL_SSBO,Sys_Render.texturePalettesID);
    glBufferData(GL_SSBO,totalPaletteColors * sizeof(u32),texturePalettes,GL_STATIC_DRAW);
    glBindBuffer(GL_SSBO,Sys_Render.textureOffsetsID);
    glBufferData(GL_SSBO,loadedTexturesMaxIndex * sizeof(u32),textureOffsets,GL_STATIC_DRAW);
    glBindBuffer(GL_SSBO,Sys_Render.textureSizesID);
    glBufferData(GL_SSBO,loadedTexturesMaxIndex * 2 * sizeof(i32),textureSizes,GL_STATIC_DRAW);
    glBindBuffer(GL_SSBO,Sys_Render.texturePaletteOffsetsID);
    glBufferData(GL_SSBO,loadedTexturesMaxIndex * sizeof(u32),texturePaletteOffsets,GL_STATIC_DRAW);
    glBindBuffer(GL_SSBO,0);
    OS_DeallocateRAM(texture_parser.entries,texture_parser.count * sizeof(TextureData));
    OS_DeallocateRAM(arena,arena_size);
    OS_DeallocateRAM(rawTextures,loadedTexturesMaxIndex * sizeof(RawTexture));
    OS_DeallocateRAM(parsIdx,loadedTexturesMaxIndex * sizeof(i32));
    OS_DeallocateRAM(textureIndexBuffers,loadedTexturesMaxIndex * sizeof(u8*));
    OS_DeallocateRAM(textureSizes,loadedTexturesMaxIndex * 2 * sizeof(i32));
    OS_DeallocateRAM(texturePaletteBuffers,loadedTexturesMaxIndex * sizeof(u32*));
    OS_DeallocateRAM(texturePaletteSizes,loadedTexturesMaxIndex * sizeof(u32));
    OS_DeallocateRAM(texturePaletteOffsets,loadedTexturesMaxIndex * sizeof(u32));
    OS_DeallocateRAM(textureWidths,loadedTexturesMaxIndex * sizeof(i32));
    OS_DeallocateRAM(textureHeights,loadedTexturesMaxIndex * sizeof(i32));
    for (int t=0;t<num_parse_threads;++t) OS_DeallocateRAM(thread_stbi_arenas[t].base,STBI_ARENA_SIZE);
    OS_DeallocateRAM(thread_stbi_arenas,(size_t)num_parse_threads * sizeof(StbiArena));
    OsFileHandle fp = OS_OpenReadonly(Sys_Global.global_winicon); // Load window icon
    int windowIconFileSize = OS_FileSize(fp);
    u8* file_buffer = OS_AllocateFileBackedRAMReadonly(windowIconFileSize,fp,Sys_Global.global_winicon);    
    OS_Close(fp); stbi__arena_init_thread(&stbi_arena_main);
    int w=1,h=1; unsigned char* pixels = stbi_load_from_memory(file_buffer,windowIconFileSize,&w,&h);
    if (!pixels) { DualLogError("Failed to load icon: %s\n",Sys_Global.global_winicon); OS_Exit(1); }
    
    GLFWimage image = (GLFWimage){w,h,pixels};
    glfwSetWindowIcon(window,&image);
    OS_DeallocateRAM(file_buffer,windowIconFileSize);
    OS_DeallocateRAM(stbi_arena_main.base,STBI_ARENA_SIZE); stbi_arena_main.base = NULL;
    DualLog(" took %.6f secs\n",get_time() - start_time);
    DebugRAM("After LoadTextures and after deallocation");
}
