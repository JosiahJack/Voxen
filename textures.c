// textures.c - 2D Texture Loading System
u32 totalPixels,totalPaletteColors;
typedef struct { u16 index; bool transparent; bool doublesided; char path[128]; } TextureData; typedef struct { TextureData* entries; u32 count; u32 capacity; } TextureDataParser; typedef struct { const char* data; int size; } RawTexture;
typedef struct { u32 img_x, img_y; i32 img_n, img_out_n; u8* img_buffer, *img_buffer_end; } PngContext;
typedef struct { u8* indices; u32* palette,palSize; i32 w, h; } TexResult;
typedef struct TextureParseTask { u32 texCnt; _Atomic u32* shared_idx; RawTexture* raw_textures; i32* parsIdx; const TextureDataParser* parser; TexResult* results;int tid; } TextureParseTask;
typedef struct { PngContext* s; u8* idata, *expanded, *out; } PngData; typedef struct { u16 fast[1<<9], firstcode[16], firstsymbol[16], value[288]; i32 maxcode[17]; u8 size[288]; } PngHuffman; typedef struct { u8 *zbuffer, *zbuffer_end, *zout, *zout_start; i32 num_bits; u32 code_buffer; PngHuffman z_length, z_distance; } pngzbuf;
enum { PNGFmt_none=0, PNGFmt_sub=1, PNGFmt_up=2, PNGFmt_avg=3, PNGFmt_paeth=4, PNGFmt_avg_first, PNGFmt_paeth_first };
PngArena png_arena_main; static PngArena* thread_png_arenas = NULL; static u8** textureIndexBuffers = NULL; static u32** texturePaletteBuffers = NULL; static u32* texturePaletteSizes = NULL; static i32* textureWidths = NULL; static i32* textureHeights = NULL;
void PngArenaInit(PngArena* arena) { if (!arena->base) { arena->base = OS_Alloc(16777216); arena->cursor = arena->base; arena->end = arena->base + 16777216; } }
void* PngArenaAlloc(PngArena* a, size_t s) { if(!a->base||a->cursor+s>a->end)return NULL; void* p=a->cursor; a->cursor+=s; return p; }
static u32 PngGet32be(PngContext* s) { const u8* p = s->img_buffer; s->img_buffer += 4; return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]; }
static i32 BitReverse(i32 n, i32 b) { n=((n&0xAAAA)>>1)|((n&0x5555)<<1); n=((n&0xCCCC)>>2)|((n&0x3333)<<2); n=((n&0xF0F0)>>4)|((n&0x0F0F)<<4); n=((n&0xFF00)>>8)|((n&0x00FF)<<8); return n>>(16-b); }
static i32 PngHuf(PngHuffman* z, const u8* sl, i32 num) {
    i32 i,k=0,code=0,nc[16],sz[17]={0}; mset(z->fast,0,sizeof(z->fast)); if(num != 32) { for(i=0;i<num;++i)++sz[sl[i]]; } sz[0]=0;
    for(i=1;i<16;++i){ if(sz[i]>(1<<i)){return 0;} nc[i]=code; z->firstcode[i]=(u16)code; z->firstsymbol[i]=(u16)k; code+=sz[i]; if(sz[i]&&code-1>=(1<<i)){return 0;} z->maxcode[i]=code<<(16-i); code<<=1; k+=sz[i]; }
    z->maxcode[16]=0x10000;
    for(i=0;i<num;++i){ int s=(num==32)?5:sl[i]; if(!s){continue;} int c=nc[s]-z->firstcode[s]+z->firstsymbol[s]; u16 fv=(u16)((s<<9)|i); z->size[c]=(u8)s; z->value[c]=(u16)i; if(s<=9){ int j=BitReverse(nc[s],s); while(j<(1<<9)){z->fast[j]=fv; j+=(1<<s);} } ++nc[s]; }
    return 1;
}

#define REFILL(z) if(z->num_bits<16){do{z->code_buffer|=(u32)(*z->zbuffer++)<<z->num_bits;z->num_bits+=8;}while(z->num_bits<=24);}
static u32 PngZReceive(pngzbuf* z, int n) { REFILL(z); u32 k=z->code_buffer&((1u<<n)-1); z->code_buffer>>=n; z->num_bits-=n; return k; }
static u32 PngHuffman_decode(pngzbuf* a, PngHuffman* z) { REFILL(a); int b=z->fast[a->code_buffer&511], s; if(b){ s=b>>9; a->code_buffer>>=s; a->num_bits-=s; return b&511; } int k=BitReverse(a->code_buffer,16); for(s=10; k>=z->maxcode[s]; ++s); b=(k>>(16-s))-z->firstcode[s]+z->firstsymbol[s]; a->code_buffer>>=s; a->num_bits-=s; return z->value[b]; }
static int PngParseHuffmanBlock(pngzbuf* a) {
    u8* o=a->zout;
    static const int lb[]={3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227,258}, le[]={0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0}, db[]={1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577}, de[]={0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};
    for(;;){ int z=PngHuffman_decode(a,&a->z_length); if(z<256){*o++=(u8)z;} else if(z==256){ a->zout=o; return 1; } else { z-=257; int l=lb[z]+(le[z]?PngZReceive(a,le[z]):0); z=PngHuffman_decode(a,&a->z_distance); int d=db[z]+(de[z]?PngZReceive(a,de[z]):0); u8* p=o-d; while(l--){*o++=*p++;} } }
}

static int PngComputeHuffmans(pngzbuf* a) {
    static const u8 dz[]={16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15}; u8 lc[286+32+137], cs[19]={0};
    u32 hl=PngZReceive(a,5)+257, hd=PngZReceive(a,5)+1, hc=PngZReceive(a,4)+4, nt=hl+hd, n=0;
    for(u32 i=0;i<hc;++i) cs[dz[i]]=(u8)PngZReceive(a,3); PngHuf(&a->z_length,cs,19);
    while(n<nt){
        u32 c=PngHuffman_decode(a,&a->z_length);
        if(c<16)lc[n++]=(u8)c;
        else { u8 f=0;
            if(c==16){c=PngZReceive(a,2)+3; f=lc[n-1];}
            else if(c==17)c=PngZReceive(a,3)+3;
            else if(c==18)c=PngZReceive(a,7)+11;
            else return 0; mset(lc+n,f,c); n+=c;
        }
    } return PngHuf(&a->z_length,lc,hl) && PngHuf(&a->z_distance,lc+hl,hd);
}

static int PngParseUncompressedBlock(pngzbuf* a) {
    u8 header[4]; i32 k = 0; if (a->num_bits & 7) PngZReceive(a, a->num_bits & 7);
    while (a->num_bits > 0) { header[k] = (u8)(a->code_buffer & 255); a->code_buffer >>= 8; a->num_bits -= 8; ++k; }
    if (k <= 0) header[0] = *a->zbuffer++;
    if (k <= 1) header[1] = *a->zbuffer++;
    if (k <= 2) header[2] = *a->zbuffer++;
    if (k <= 3) header[3] = *a->zbuffer++;
    i32 len = header[1] * 256 + header[0];
    mcpy(a->zout,a->zbuffer,len); a->zbuffer += len; a->zout += len;
    return 1;
}

static u8 PngZDefLen(int i) { return (i<144)?8:(i<256)?9:(i<280)?7:8; }
u8* PngDecode(const u8* buffer, i32 len, i32 initial_size, i32* outlen, PngArena* arena) {
    pngzbuf a = {0}; u8* p = (u8*)PngArenaAlloc(arena, initial_size), d_len[288]; i32 f, t;
    a.zbuffer = (u8*)buffer; a.zbuffer_end = (u8*)buffer+len; a.zout_start = a.zout = p; a.zbuffer += 2;
    do {
        f = PngZReceive(&a, 1); t = PngZReceive(&a, 2);
        if (t == 0) PngParseUncompressedBlock(&a);
        else {
            if (t == 1) { for(int i=0; i<288; ++i) d_len[i]=PngZDefLen(i); PngHuf(&a.z_length, d_len, 288); PngHuf(&a.z_distance, NULL, 32); }
            else PngComputeHuffmans(&a);
            PngParseHuffmanBlock(&a);
        }
    } while (!f);
    if (outlen) *outlen = (i32)(a.zout - a.zout_start); return a.zout_start;
}

static u8 first_row_filter[5] = {PNGFmt_none, PNGFmt_sub, PNGFmt_none, PNGFmt_avg_first, PNGFmt_paeth_first};
inline static i32 PngPaeth(i32 a, i32 b, i32 c) { i32 p = a+b-c, pa = vabs(p-a), pb = vabs(p-b), pc = vabs(p-c); return (pa <= pb && pa <= pc) ? a : (pb <= pc ? b : c); }
static i32 CreatePngImageArena(PngArena* arena, PngData* a, u8* raw, u32 raw_len, i32 out_n, u32 x, u32 y, i32 img_n) {
    u32 i, j, stride = x * out_n, w_bytes = (img_n * x * 8 + 7) >> 3; i32 k, f;
    if (raw_len < (w_bytes + 1) * y) return 0;
    a->out = (u8*)PngArenaAlloc(arena, (size_t)x * y * out_n);
    for (j = 0; j < y; ++j) {
        u8 *cur = a->out + stride * j;
        u8 *prior = (j > 0) ? cur - stride : a->out;
        if ((f = *raw++) > 4) return 0;
        if (j == 0) f = first_row_filter[f];
        switch (f) { /* First pixel (left neighbor is implicitly 0) */
            case PNGFmt_none:
            case PNGFmt_sub: for (k = 0; k < img_n; ++k) cur[k] = raw[k]; break;
            case PNGFmt_up: for (k = 0; k < img_n; ++k) cur[k] = raw[k] + prior[k]; break;
            case PNGFmt_avg: for (k = 0; k < img_n; ++k) cur[k] = raw[k] + (prior[k] >> 1); break;
            case PNGFmt_paeth: for (k = 0; k < img_n; ++k) cur[k] = raw[k] + PngPaeth(0, prior[k], 0); break;
            case PNGFmt_avg_first: for (k = 0; k < img_n; ++k) cur[k] = raw[k] + (0 >> 1);  /* left doesn't exist */ break;
            default: return 0;
        }
        if (img_n != out_n) cur[img_n] = 255;
        raw += img_n;
        cur += out_n;
        prior += out_n;
        switch (f) { /* Remaining pixels in the row */
            case PNGFmt_none: for (i = x - 1; i >= 1; --i, cur[img_n] = (img_n != out_n ? 255 : cur[img_n]), raw += img_n, cur += out_n, prior += out_n) { for (k = 0; k < img_n; ++k) cur[k] = raw[k]; } break;
            case PNGFmt_sub: for (i = x - 1; i >= 1; --i, cur[img_n] = (img_n != out_n ? 255 : cur[img_n]), raw += img_n, cur += out_n, prior += out_n) { for (k = 0; k < img_n; ++k) cur[k] = raw[k] + cur[k - out_n]; } break;
            case PNGFmt_up: for (i = x - 1; i >= 1; --i, cur[img_n] = (img_n != out_n ? 255 : cur[img_n]), raw += img_n, cur += out_n, prior += out_n) { for (k = 0; k < img_n; ++k) cur[k] = raw[k] + prior[k]; } break;
            case PNGFmt_avg: for (i = x - 1; i >= 1; --i, cur[img_n] = (img_n != out_n ? 255 : cur[img_n]), raw += img_n, cur += out_n, prior += out_n) { for (k = 0; k < img_n; ++k) cur[k] = raw[k] + ((prior[k] + cur[k - out_n]) >> 1); } break;
            case PNGFmt_paeth: for (i = x - 1; i >= 1; --i, cur[img_n] = (img_n != out_n ? 255 : cur[img_n]), raw += img_n, cur += out_n, prior += out_n) { for (k = 0; k < img_n; ++k) cur[k] = raw[k] + PngPaeth(cur[k - out_n], prior[k], prior[k - out_n]); } break;
            case PNGFmt_avg_first: for (i = x - 1; i >= 1; --i, cur[img_n] = (img_n != out_n ? 255 : cur[img_n]), raw += img_n, cur += out_n, prior += out_n) { for (k = 0; k < img_n; ++k) cur[k] = raw[k] + (cur[k - out_n] >> 1); } break;
            default: return 0;
        }
    }
    return 1;
}

u8* PngLoad(const u8* buffer, int len, int* x, int* y, PngArena* arena) {
    if (arena->base) arena->cursor = arena->base;
    PngContext s; s.img_n = s.img_out_n = 0; s.img_buffer = (u8*)buffer; s.img_buffer_end = (u8*)buffer + len;
    PngData z = {0}; z.s = &s; u32 ioff = 0; z.expanded = z.idata = z.out = NULL; s.img_buffer += 8; s.img_x = s.img_y = 1;
    for (;;) {
        u32 length = PngGet32be(&s), type = PngGet32be(&s);
        switch (type) {
            case 0x49484452: s.img_x = PngGet32be(&s); s.img_y = PngGet32be(&s); s.img_buffer++; { i32 color = (*s.img_buffer++); s.img_buffer += 3; s.img_n = (color & 2 ? 3 : 1) + (color & 4 ? 1 : 0); } break;
            case 0x49444154: if (!z.idata) { z.idata = (u8*)PngArenaAlloc(arena, len + 16); ioff = 0; } mcpy(z.idata + ioff, s.img_buffer, length); s.img_buffer += length; ioff += length; break;
            case 0x49454E44: { u32 rL = s.img_x * s.img_y * s.img_n + s.img_y; z.expanded = (u8*)PngDecode(z.idata, ioff, rL, (i32*)(&rL), arena); s.img_out_n = (s.img_n + 1 == 4) ? 4 : s.img_n; CreatePngImageArena(arena,&z,z.expanded, rL,s.img_out_n,s.img_x,s.img_y,s.img_n); PngGet32be(&s); goto Label_parsesuccess; }
            default: s.img_buffer += length; break;
        }
        PngGet32be(&s);
    }
    Label_parsesuccess: *x = z.s->img_x; *y = z.s->img_y; return z.out;
}

static void* TextureParsingWorker(void* arg) {
    TextureParseTask* t = (TextureParseTask*)arg; u32 i;
    while ((i = __atomic_fetch_add((u32*)t->shared_idx, 1, 5)) < t->texCnt) { // Dynamic Work Stealing: Threads fetch next available index automatically
        i32 pIdx = t->parsIdx[i]; 
        if (unlikely(pIdx < 0 || pIdx >= (i32)t->parser->count)) continue;
        doubleSidedTexture[i] = t->parser->entries[pIdx].doublesided; 
        transparentTexture[i] = t->parser->entries[pIdx].transparent;
        const char* d = t->raw_textures[i].data; 
        int sz = t->raw_textures[i].size; 
        if (unlikely(!d || sz <= 0)) continue;
        int w=0, h=0; u8 *pix = PngLoad((const u8*)d,sz,&w,&h,&thread_png_arenas[t->tid]); if (!pix || w < 1 || h < 1) { OS_Free((void*)d,(size_t)sz); continue; }
        u32 nP = (u32)w * h; u8 *idx = (u8*)OS_Alloc(nP); u32 *pal = (u32*)OS_Alloc(256 * sizeof(u32)); u32 pSz = 0; 
        u32 exact_hash[8192]; mset(exact_hash,0xFF,sizeof(exact_hash));// Fast Exact Match Hash Map (8192 slots), 0xFFFFFFFF = empty
        u8 exact_idx[8192]; 
        u8 nearest_cache[32768]; mset(nearest_cache,0xFF,sizeof(nearest_cache)); // 15-bit Color Space Cache for fast nearest-neighbor fallback
        for (u32 p = 0; p < nP; ++p) {
            u32 c = ((u32*)pix)[p];
            u32 h_val = (c * 0x9E3779B9u); // Murmur-style avalanche hash
            h_val ^= h_val >> 16;
            u32 slot = h_val & 8191;
            while (exact_hash[slot] != 0xFFFFFFFF) { if (exact_hash[slot] == c) { idx[p] = exact_idx[slot]; goto found; }  slot = (slot + 1) & 8191; }
            if (pSz < 256) { 
                pal[pSz] = c; 
                idx[p] = (u8)pSz; 
                exact_hash[slot] = c;
                exact_idx[slot] = (u8)pSz;
                pSz++; 
            } else { // Fallback: Check Nearest Neighbor Cache first (R5 G5 B5)
                u32 cache_key = ((c & 0xF8) << 7) | ((c & 0xF800) >> 1) | ((c & 0xF80000) >> 9);
                if (nearest_cache[cache_key] != 0xFF) { idx[p] = nearest_cache[cache_key]; goto found; }
                u32 best = 0, bestDist = ~0u; // Cache Miss: Full Search
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
    FHandle fd; int sz; char *data = OS_OpenAndAllocateFileBufferReadonly(fn, &fd, &sz), *cur = data, *end = data + sz;
    u32 line = 0, m_idx = 0;
    while (cur < end) {
        char *s = cur; while (cur < end && *cur != '\n' && *cur != '\r') cur++;
        size_t len = cur - s; line++; if (len <= 0) { cur++; continue; }
        while (cEmpty(*s)) s++; char *le = s + (cur - s) - 1;
        while (le > s && cEmpty(*le)) le--;
        if (*s == '\0' || (s[0] == '/' && s[1] == '/') || s[0] == '#') { if (cur < end && (*cur == '\r' || *cur == '\n')) cur++; continue; }
        char *col = StringFindFirstCharWithin(s, ':');
        if (col && sCompUpToLen(s, "index", col - s) == 0) {
            char *v = col + 1; while (cEmpty(*v)) v++;
            u32 idx = parse_numberu32(v, s, line); if (idx > m_idx) m_idx = idx;
        }
        if (cur < end && (*cur == '\r' || *cur == '\n')) cur++;
    }
    if (!m_idx || m_idx >= maxS) { if (!m_idx) DualLogWarn("No entries in %s\n", fn); else DualLogWarn("Index %u too large in %s\n", m_idx, fn); OS_Free(data, sz); return true; }
    p->entries = OS_Alloc((p->count = p->capacity = m_idx + 1) * sizeof(TextureData));
    for (u32 i = 0; i < p->count; ++i) p->entries[i] = (TextureData){.index = U16_MAX};
    TextureData e = {.index = U16_MAX}; line = 0; cur = data;
    while (cur < end) {
        char *s = cur; while (cur < end && *cur != '\n' && *cur != '\r') cur++;
        size_t len = cur - s; line++; if (len < 3) { cur++; continue; }
        while (cEmpty(*s)) s++; char *le = s + (cur - s) - 1; while (le > s && cEmpty(*le)) le--;
        if (s[0] == '/' && s[1] == '/') { cur++; continue; }
        if (*s == '#') {
            if (e.path[0] && e.index < p->capacity) p->entries[e.index] = e;
            e = (TextureData){.index = U16_MAX}; size_t aL = le - s;
            if (aL >= sizeof(e.path)) aL = sizeof(e.path) - 1; mcpy(e.path,s + 1,aL); e.path[aL] = 0;
        } else {
            char *col = StringFindFirstCharWithin(s, ':');
            if (col) {
                char *k = s, *v = col + 1, tk[256] = {0}, tv[256] = {0};
                while (cEmpty(*k) && k < col) k++; while (cEmpty(*v) && v <= le) v++;
                size_t kL = col - k, vL = (le >= v) ? (le - v + 1) : 0;
                if (kL && vL) {
                    sCpy2aSubFromb(tk, kL, k, 256); sCpy2aSubFromb(tv,vL,v,256);
                    char *ke = tk + slen(tk) - 1, *ve = tv + slen(tv) - 1;
                    while (ke > tk && cEmpty(*ke)) *ke-- = 0; while (ve > tv && cEmpty(*ve)) *ve-- = 0;
                         if (sEqual(tk,      "index")) e.index       = parse_numberu16(tv,s,line);
                    else if (sEqual(tk,"transparent")) e.transparent = parse_bool(tv,s,line);
                    else if (sEqual(tk,"doublesided")) e.doublesided = parse_bool(tv,s,line);
                }
            }
        }
        if (cur < end && (*cur == '\r' || *cur == '\n')) cur++;
    }
    if (e.path[0] && e.index < p->capacity) p->entries[e.index] = e;
    OS_Free(data, sz); return true;
}

typedef struct { int width,height; u8* pixels; } WinSysIcon;
void VSetWindowIcon(WinSysIcon*);
static __attribute__((noinline)) void LoadTextures() {
    double start_time = get_time();
    texCnt = totalPixels = totalPaletteColors = 0u;
    TextureDataParser texture_parser; 
    if (unlikely(!ParseTextureData(&texture_parser, MAX_TXRS, "./Data/textures.txt"))) { DualLogError("Could not parse ./Data/textures.txt!\n"); OS_Exit(1); }
    i32 maxIndex = -1;
    for (u32 k = 0; k < texture_parser.count; ++k) { if (texture_parser.entries[k].index > maxIndex && texture_parser.entries[k].index != U16_MAX) {maxIndex = texture_parser.entries[k].index;} }
    texCnt = (u16)(maxIndex + 1);
    i32* parsIdx = OS_Alloc(texCnt * sizeof(i32));
    mset(parsIdx, -1, texCnt * sizeof(i32));
    for (u32 k = 0; k < texture_parser.count; ++k) { if (texture_parser.entries[k].index < texCnt) {parsIdx[texture_parser.entries[k].index] = (i32)k;} }
    DualLog("Loading textures (%u) ... ", texture_parser.count);
    RawTexture* rawTextures = OS_Alloc(texCnt * sizeof(RawTexture));
    mset(rawTextures, 0, texCnt * sizeof(RawTexture));
    for (u32 i = 0; i < texCnt; ++i) {
        i32 p = parsIdx[i]; if (p < 0) continue;
        const char* path = texture_parser.entries[p].path;
        FHandle dummy_fd;
        int size = 0; 
        rawTextures[i].data = (const char*)OS_OpenAndAllocateFileBufferReadonly(path, &dummy_fd, &size);
        rawTextures[i].size = size;
    }
    thread_png_arenas = (PngArena*)OS_Alloc((size_t)threadCnt * sizeof(PngArena));
    for (int t = 0; t < threadCnt; ++t) { thread_png_arenas[t].base = NULL; PngArenaInit(&thread_png_arenas[t]); }
    TexResult* texResults = OS_Alloc(texCnt * sizeof(TexResult)); // Unified result struct allocation
    mset(texResults, 0, texCnt * sizeof(TexResult));
    TextureParseTask tasks[32]; 
    OS_Thread workers[32];
    _Atomic u32 shared_idx = 0; // The shared thread counter
    for (int t = 0; t < threadCnt; ++t) { tasks[t] = (TextureParseTask){.texCnt = texCnt, .shared_idx = &shared_idx, .raw_textures = rawTextures, .parsIdx = parsIdx, .parser = &texture_parser, .results = texResults,.tid = t}; OS_ThreadCreate(&workers[t], TextureParsingWorker, &tasks[t]); }
    for (int t = 0; t < threadCnt; ++t) OS_ThreadJoin(&workers[t]);
    totalPixels = totalPaletteColors = 0u;
    for (u16 i = 0; i < texCnt; ++i) { if (texResults[i].indices) { totalPixels += (u32)texResults[i].w * texResults[i].h; totalPaletteColors += texResults[i].palSize; } }
    size_t offsets_size = texCnt * sizeof(u32); size_t palettes_size = totalPaletteColors * sizeof(u32); size_t indices_size = totalPixels; size_t arena_size = offsets_size + palettes_size + indices_size;
    void* arena = OS_AllocateRAM(arena_size, 0x1|0x2, 0x20|0x02|0x08000, INVALID_FHANDLE);
    u8* cur = (u8*)arena;
    u32* textureOffsets = (u32*)cur; cur += offsets_size;
    i32* textureSizes = OS_Alloc(texCnt * 2 * sizeof(i32));
    u32* texturePaletteOffsets = OS_Alloc(texCnt * sizeof(u32));
    u32* texturePalettes = (u32*)cur; cur += palettes_size;
    u8* all_indices = cur; 
    u32 pixel_base = 0, color_base = 0;
    for (u16 i=0; i<texCnt; ++i) {
        if (!texResults[i].indices) continue;
        u32 numP = (u32)texResults[i].w * texResults[i].h; 
        u32 palS = texResults[i].palSize;
        textureOffsets[i] = pixel_base; 
        texturePaletteOffsets[i] = color_base;
        textureSizes[i*2]     = texResults[i].w; 
        textureSizes[i*2 + 1] = texResults[i].h;
        mcpy(all_indices + pixel_base, texResults[i].indices, numP);
        mcpy(texturePalettes + color_base, texResults[i].palette, palS * sizeof(u32));
        pixel_base += numP; 
        color_base += palS;
        OS_Free(texResults[i].indices, numP); 
        OS_Free(texResults[i].palette, palS * sizeof(u32));
    }
    DualLog("total palette colors: %u, total pixels: %u...", totalPaletteColors, totalPixels);
    i32 packed_size = ((i32)totalPixels + 3) / 4 * sizeof(u32);
    glBindBuffer(GL_SSBO, colorBufferID);
    void* dst = glMapBufferRange(GL_SSBO,0,packed_size,0x0002|0x0004);
    mcpy(dst,all_indices,packed_size); glUnmapBuffer(GL_SSBO);
    glBindBuffer(GL_SSBO, texPalID);         glBufferData(GL_SSBO, totalPaletteColors * sizeof(u32), texturePalettes, GL_STATIC_DRAW);
    glBindBuffer(GL_SSBO, textureOffsetsID); glBufferData(GL_SSBO, texCnt * sizeof(u32), textureOffsets, GL_STATIC_DRAW);
    glBindBuffer(GL_SSBO, textureSizesID);   glBufferData(GL_SSBO, texCnt * 2 * sizeof(i32), textureSizes, GL_STATIC_DRAW);
    glBindBuffer(GL_SSBO, texPalOfsID);      glBufferData(GL_SSBO, texCnt * sizeof(u32), texturePaletteOffsets, GL_STATIC_DRAW); glBindBuffer(GL_SSBO, 0);
    OS_Free(texture_parser.entries, texture_parser.count * sizeof(TextureData)); OS_Free(arena, arena_size);
    OS_Free(rawTextures, texCnt * sizeof(RawTexture));                           OS_Free(parsIdx, texCnt * sizeof(i32));
    OS_Free(texResults, texCnt * sizeof(TexResult));                             OS_Free(textureSizes, texCnt * 2 * sizeof(i32));
    OS_Free(texturePaletteOffsets, texCnt * sizeof(u32));        
    for (int t=0; t<threadCnt; ++t) OS_Free(thread_png_arenas[t].base, 16777216);
    OS_Free(thread_png_arenas, (size_t)threadCnt * sizeof(PngArena));
    FHandle fp = OS_OpenReadonly(World.global_winicon);
    int windowIconFileSize = OS_FileSize(fp);
    u8* file_buffer = OS_AllocateFileBackedRAMReadonly(windowIconFileSize, fp, World.global_winicon);    
    OS_Close(fp); PngArenaInit(&png_arena_main);
    int w=1, h=1; 
    u8* pixels = PngLoad(file_buffer, windowIconFileSize, &w, &h, &png_arena_main);
    if (!pixels) { DualLogError("Failed to load icon: %s\n", World.global_winicon); OS_Exit(1); }
    WinSysIcon image = (WinSysIcon){w,h,pixels}; 
    VSetWindowIcon(&image);
    OS_Free(file_buffer, windowIconFileSize); 
    OS_Free(png_arena_main.base, 16777216); 
    png_arena_main.base = NULL;
    DualLog(" took %.6f secs\n", get_time() - start_time);
    DebugRAM("After LoadTextures and after deallocation");
}
