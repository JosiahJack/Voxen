// models.c - 3D Models Loading System
u8** modelVertices = NULL; u16** modelTriangles = NULL; u32 modelVertexCounts[MAX_MDLS] = {0}; u16 modelTriangleCounts[MAX_MDLS] = {0}; float modelBounds[MAX_MDLS] = {0}; u16 mdlsCnt = 0;
#define MAX_VERT_ELEMENT_SIZE 6964
#define MAX_OUTPUT_VERTS 20960
static float **thrd_pos = NULL, **thread_temp_nrm = NULL, **thrd_uv = NULL, **thrd_verts = NULL; static u16** thrd_tris = NULL; static u32** thrd_ht = NULL; static u32** thrd_ht_used = NULL; 
static u16** thrd_ft_scratch = NULL; static u32** thrd_remap_scratch = NULL; static u8** thrd_nv_scratch = NULL; static u8** thrd_cache_scratch = NULL;
typedef struct { const char* data; const char* name; int size; } RawOBJ; typedef struct { u16 index; bool animated; u8 animationNum; char path[128]; } ModelData; typedef struct { ModelData* entries; u32 count; u32 capacity; } ModelDataParser;
static u16 base_table[512]; static u8 shift_table[512];
void InitializeFloatToHalfLUT() {
    for (int i = 0; i < 256; ++i) {
        int e = i - 127;
             if (e < -24) { base_table[i] = 0; base_table[i | 0x100] = 0x8000; shift_table[i] = 24; shift_table[i | 0x100] = 24; } // Underflow to zero
        else if (e < -14) { base_table[i] = 0; base_table[i | 0x100] = 0x8000; shift_table[i] = (u8)(-14 - e + 13); shift_table[i | 0x100] = (u8)(-14 - e + 13); } // Denormal half float
        else if (e <= 15) { base_table[i] = (u16)((e + 15) << 10); base_table[i | 0x100] = (u16)(((e + 15) << 10) | 0x8000); shift_table[i] = 13; shift_table[i | 0x100] = 13; } // Regular normalized number
        else if (e < 128) { base_table[i] = 0x7C00; base_table[i | 0x100] = 0xFC00; shift_table[i] = 24; shift_table[i | 0x100] = 24; } // Overflow to infinity 
        else { base_table[i] = 0x7C00; base_table[i | 0x100] = 0xFC00; shift_table[i] = 13; shift_table[i | 0x100] = 13; } // Stay NaN / Infinity
    }
}

INLINE half float_to_half(float f) {
    u32 f_bits; mcpy(&f_bits,&f,4);
    u32 i = (f_bits >> 23) & 0x1FF;
    u8 sh = shift_table[i];
    u32 mant = (f_bits & 0x007FFFFF) + (1u << (sh - 1)); // round-to-nearest: bias by half the truncated range before the shift; carry ripples into base_table[i]'s exponent bits via the '+' below.  This fixes seams between modular wall "chunks"
    return (half)(base_table[i] + (mant >> sh));
}

INLINE float fast_atof(const char** p) { const char* c=*p; while (*c == ' ' || *c == '\t') {c++;} float s=1.0f; if(*c == '-'){s=-1.0f; c++;} float v=0.0f; while (*c >= '0' && *c <= '9') { v=v * 10.0f + (*c - '0'); c++; } if (*c == '.') { c++; float sub=0.1f; while (*c >= '0' && *c <= '9') { v += (*c - '0') * sub; sub*=0.1f; c++; } } *p=c; return s * v; }
INLINE i32 fast_atoi(const char** p) { const char* c = *p; while (*c == ' ' || *c == '\t') {c++;} i32 s=1; if(*c == '-'){s=-1; c++;} i32 v = 0; while (*c >= '0' && *c <= '9') { v = v * 10 + (*c - '0'); c++; } *p = c; return v * s; }
typedef struct { u32 idx,key; } TriSort;
int cmp(const void* a, const void* b) { u32 ka=((const TriSort*)a)->key, kb=((const TriSort*)b)->key; return (ka > kb) - (ka < kb); } // branchless 1 or -1
static void RadixSortTriangles(TriSort* src, TriSort* temp, u32 count) {
    if (count < 2) return;
    u32 b0[256] = {0};
    u32 b1[256] = {0};
    for (u32 i = 0; i < count; ++i) {u16 key = src[i].key; b0[key & 0xFF]++; b1[(key >> 8) & 0xFF]++;}
    u32 sum0 = 0, sum1 = 0;
    for (u32 i = 0; i < 256; ++i) { u32 t0 = b0[i]; u32 t1 = b1[i]; b0[i] = sum0; b1[i] = sum1; sum0 += t0; sum1 += t1; }
    for (u32 i = 0; i < count; ++i) {u32 radix0 = src[i].key & 0xFF; u32 dest = b0[radix0]++; temp[dest] = src[i];}
    for (u32 i = 0; i < count; ++i) { u32 radix1 = (temp[i].key >> 8) & 0xFF; u32 dest = b1[radix1]++; src[dest] = temp[i]; }
}

static void OptimizeVertexCache(u16* idx, u32 ic, u32 vc, u8* scratch) {
    if (ic < 3 || !vc) return;
    u32 tc = ic / 3;
    TriSort* t = (TriSort*)scratch;
    TriSort* t_tmp = (TriSort*)(scratch + (tc * sizeof(TriSort)));
    u16* n = (u16*)(scratch + (tc * sizeof(TriSort) * 2));
    for (u32 i = 0; i < tc; ++i) { u16* p = idx + i * 3; u32 m = p[0] < p[1] ? p[0] : p[1]; m = m < p[2] ? m : p[2]; t[i].idx = i; t[i].key = (u16)m; }
    RadixSortTriangles(t, t_tmp, tc);
    for (u32 i = 0; i < tc; ++i) { u16* s = idx + t[i].idx * 3; u16* d = n + i * 3; d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; }
    mcpy(idx, n, ic * sizeof(u16));
}

static u8* OptimizeVertexFetch(u8* v, u32* vc, u16* idx, u32 ic, size_t stride, u32* remap, u8* nv) {
    u32 oc = *vc; if (!oc || !ic) return v;
    mset(remap,0xFF,oc * sizeof(u32));
    u32 nc = 0;
    for (u32 i = 0; i < ic; ++i) { u32 id = idx[i]; if (id < oc && remap[id] == 0xFFFFFFFFU) { remap[id] = nc; ++nc; } }
    mset(remap,0xFF,oc * sizeof(u32));
    u32 write_ptr=0;
    for (u32 i=0;i<ic;++i) { u32 id = idx[i]; if (id < oc) { if (remap[id] == 0xFFFFFFFFU) { remap[id] = write_ptr; mcpy(nv + write_ptr * stride, v + id * stride, stride); write_ptr++; } idx[i]=(u16)remap[id]; } }
    *vc = nc;
    return nv;
}

#define HASH_SIZE 32768
static __attribute__((hot)) __attribute__((flatten)) bool ParseOBJ(u32 mindex, const char* __restrict d, int fs, float* __restrict tp, float* __restrict tn, float* __restrict tu, float* __restrict sv, u16* __restrict st, u32* __restrict ht, u32* __restrict ht_used, u16* __restrict ft, u32* __restrict remap_scr, u8* __restrict nv_scr, u8* __restrict cache_scr, u8** ov, u32* ovc, u16** ot, u16* otc, const char* name) {
    *ov = NULL; *ot = NULL; *ovc = *otc = 0;
    u32 pc=0,nc=0,uc=0,ec=0;
    float mx=1e9f,my=1e9f,mz=1e9f,Mx=-1e9f,My=-1e9f,Mz=-1e9f;
    const char *p = d, *e = d+fs;
    while (likely(p < e)) {
        while (p < e && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')) ++p;
        if (p >= e) break;
        if (*p == '#') { while (p < e && *p != '\n') ++p; continue; }
        if (*p == 'v') {
            ++p;
            if (*p == ' ') { if (unlikely(pc >= MAX_VERT_ELEMENT_SIZE)) {return false;} ++p; tp[pc*3] = fast_atof(&p); tp[pc*3+1] = fast_atof(&p); tp[pc*3+2] = fast_atof(&p); ++pc; }
            else if (*p == 'n' && p[1] == ' ') { p += 2; if (unlikely(nc >= MAX_VERT_ELEMENT_SIZE)) {return false;} tn[nc*3] = fast_atof(&p); tn[nc*3+1] = fast_atof(&p); tn[nc*3+2] = fast_atof(&p); ++nc; }
            else if (*p == 't' && p[1] == ' ') { p += 2; if (unlikely(uc >= MAX_VERT_ELEMENT_SIZE)) {return false;} tu[uc*2] = fast_atof(&p); tu[uc*2+1] = fast_atof(&p); ++uc; }
        } else if (*p == 'f' && p[1] == ' ') {
            p += 2;
            u32 vi[8]={0}, ti[8]={0}, ni[8]={0}; int nv = 0;
            while (nv < 8 && p < e && *p != '\n' && *p != '\r') {
                while (*p == ' ' || *p == '\t') ++p;
                if (*p == '\n' || *p == '\r' || *p == '#') break;
                long r = fast_atoi(&p);
                u32 v = (r>0) ? (u32)r : (r<0) ? (u32)((i32)pc + r) : 0; vi[nv] = v;
                if (*p == '/') {
                    ++p;
                    if (*p != '/') { r = fast_atoi(&p); u32 t = (r>0)?(u32)r:(r<0)?(u32)((i32)uc+r):0; ti[nv]=t; }
                    if (*p == '/') { ++p; r = fast_atoi(&p); u32 n = (r>0)?(u32)r:(r<0)?(u32)((i32)nc+r):0; ni[nv]=n; }
                }
                ++nv;
            }
            if (nv < 3) goto skip;
            for (int k=1; k<nv-1; ++k) {
                if (unlikely(ec + 3 > MAX_OUTPUT_VERTS)) {DualLogError("vert overflow %s!\n",name); return false;}
                u32 tri[3] = {0, (u32)k, (u32)(k+1)};
                for (int t=0; t<3; ++t) {
                    int ix = tri[t]; u32 v = vi[ix] ? vi[ix]-1 : 0; u32 tex = (ti[ix] && ti[ix] <= uc) ? ti[ix]-1 : 0; u32 nrm = (ni[ix] && ni[ix] <= nc) ? ni[ix]-1 : 0; float* dst = sv + (ec<<3);
                    dst[0]=-tp[v*3]; dst[1]=tp[v*3+1]; dst[2]=tp[v*3+2]; dst[3]=(nrm < nc) ? -tn[nrm*3] : 0; dst[4]=(nrm < nc) ? tn[nrm*3+1] : 0; dst[5]=(nrm < nc) ? tn[nrm*3+2] : 0; dst[6]=(tex < uc) ? tu[tex*2] : 0; dst[7]=(tex < uc) ? tu[tex*2+1] : 0;
                    float x=dst[0],y=dst[1],z=dst[2];
                    if (x < mx) mx = x; if (x > Mx) Mx = x;
                    if (y < my) my = y; if (y > My) My = y;
                    if (z < mz) mz = z; if (z > Mz) Mz = z;
                    st[ec] = (u16)ec; ++ec;
                }
            }
        skip:;
        } else while (p < e && *p != '\n') ++p;
    }
    if (unlikely(!ec)) return false;
    u32 used_slots_count = 0;
    u32* rem = (u32*)st; u32 ucnt = 0;
    for (u32 i=0; i<ec; ++i) {
        const float* v = sv + (i<<3);
        u64 h = *(u32*)(v+0) ^ *(u32*)(v+2) ^ *(u32*)(v+4) ^ *(u32*)(v+6); u32 s = (u32)(h ^ (h>>32)) & (HASH_SIZE-1);
        while (ht[s] != 0xFFFFFFFFU) { if (mcmp(sv+(ht[s]<<3), v, 32) == 0) { rem[i] = ht[s]; goto nxt; } s = (s+1) & (HASH_SIZE-1); }
        ht[s] = ucnt; rem[i] = ucnt;
        ht_used[used_slots_count++] = s;
        mcpy(sv+(ucnt<<3), v, 32);
        ++ucnt; nxt:;
    }
    for (u32 i=0;i<ec;++i) ft[i] = (u16)rem[i];
    OptimizeVertexCache(ft,ec,ucnt,cache_scr);
    u8* optv_src = OptimizeVertexFetch((u8*)sv,&ucnt,ft,ec,CPU_VRT_SZ,remap_scr,nv_scr);
    u8* final_v = OS_Alloc((size_t)ucnt * CPU_VRT_SZ); // Copy out precisely sized final persistent mesh chunks since buffers are recycled
    mcpy(final_v, optv_src, (size_t)ucnt * CPU_VRT_SZ);
    u16* final_t = OS_Alloc(ec * sizeof(u16));
    mcpy(final_t, ft, ec * sizeof(u16));
    *ov = final_v; *ovc = ucnt; *ot = final_t; *otc = ec/3; modelBounds[mindex] = vmax(vabs(mx),vmax(vabs(my),vmax(vabs(mz),vmax(Mx,vmax(My,Mz)))));
    for (u32 i = 0; i < used_slots_count; ++i) {ht[ht_used[i]] = 0xFFFFFFFFU;}
    return true;
}

typedef struct { u32 start, end; RawOBJ* raw; int tid; } ModelParseTask;
static void* ModelParsingWorker(void* arg) {
    ModelParseTask* t = arg;
    for (u32 i = t->start; i < t->end; ++i) {
        RawOBJ obj = t->raw[i]; if (unlikely(!obj.data || obj.size <= 0)) continue;
        if (!ParseOBJ(i,obj.data,obj.size,thrd_pos[t->tid],thread_temp_nrm[t->tid],thrd_uv[t->tid],thrd_verts[t->tid],thrd_tris[t->tid],thrd_ht[t->tid],thrd_ht_used[t->tid],thrd_ft_scratch[t->tid],thrd_remap_scratch[t->tid],thrd_nv_scratch[t->tid],thrd_cache_scratch[t->tid],&modelVertices[i],&modelVertexCounts[i],&modelTriangles[i],&modelTriangleCounts[i],obj.name)) continue;
    }
    return NULL;
}

bool ParseModelData(ModelDataParser *p, u16 maxSz, const char *fn) {
    FHandle fd; int sz; char* buf = OS_OpenAndAllocateFileBufferReadonly(fn, &fd, &sz);
    char *c = buf, *e = buf + sz; u32 maxidx = 0, ln = 0;
    while (c < e) { // first pass - find max index
        char* s = c; while (c < e && *c != '\n' && *c != '\r') ++c;
        if (c - s > 5) { while (cEmpty(*s)) {++s;} char* col = StringFindFirstCharWithin(s, ':'); if (col && sCompUpToLen(s, "index", col - s) == 0) { u32 idx = parse_numberu32(col+1, s, ln); if(idx > maxidx){maxidx=idx;} } }
        if (c < e && *c == '\r') ++c; if (c < e && *c == '\n') ++c; ++ln;
    }
    if (!maxidx) { DualLogWarn("No entries in %s\n", fn); OS_Free(buf,sz); return true; }
    if (maxidx >= maxSz) { DualLogWarn("Index too large in %s\n", fn); OS_Free(buf,sz); return true; }
    u32 cnt = maxidx + 1;
    ModelData* ents = OS_Alloc(cnt * sizeof(ModelData));
    p->entries = ents; p->capacity = p->count = cnt;
    for (u32 i=0; i<cnt; ++i) ents[i] = (ModelData){U16_MAX, false, 255, {0}};
    ModelData cur = {U16_MAX, false, 255, {0}};
    c = buf; e = buf+sz; ln = 0;
    while (c < e) {
        char* s = c; while (c < e && *c != '\n' && *c != '\r') ++c;
        size_t len = c - s; ++ln;
        if (len < 3) { if (c<e && (*c=='\r'||*c=='\n')) ++c; continue; }
        while (cEmpty(*s)) ++s;
        char* le = s + len - 1; while (le > s && cEmpty(*le)) --le;
        if (*s == '/' && s[1] == '/') goto next;
        if (*s == '#') {
            if (cur.path[0] && cur.index != U16_MAX && cur.index < cnt) ents[cur.index] = cur;
            cur = (ModelData){U16_MAX, false, 255, {0}};
            if (le > s) { size_t pl=le - s; if(pl >= sizeof(cur.path)){pl=sizeof(cur.path)-1;} mcpy(cur.path,s+1,pl); cur.path[pl] = 0; }
            goto next;
        }
        char* col = StringFindFirstCharWithin(s, ':');
        if (col) {
            char k[256]={0}, v[256]={0};
            sCpy2aSubFromb(k, col-s, s, 256);
            sCpy2aSubFromb(v, le-col, col+1, 256);
            if (sEqual(k,"index")) cur.index = parse_numberu16(v,s,ln);
            else if (sEqual(k,"animationNum")) cur.animationNum = parse_numberu16(v,s,ln);
            else if (sEqual(k,"animated")) cur.animated = parse_numberu8(v,s,ln);
        }
        next: if (c < e && *c == '\r') ++c; if (c < e && *c == '\n') ++c;
    }
    if (cur.path[0] && cur.index != U16_MAX && cur.index < cnt) ents[cur.index] = cur;
    OS_Free(buf, sz);
    return true;
}

void LoadModels() {
    double startModelTime = get_time();
    ModelDataParser mp = {0};
    if (!ParseModelData(&mp, MAX_MDLS,"./Data/models.txt")) { DualLogError("Failed models.txt\n"); OS_Exit(1); }
    DualLog("Loading models (%d) ...",mp.count);
    u32 maxid = 0; for (u32 i=0; i<mp.count; ++i) { if (mp.entries[i].index != U16_MAX && mp.entries[i].index > maxid) maxid = mp.entries[i].index; } mdlsCnt = (u16)maxid + 1;
    modelVertices = OS_Alloc(mdlsCnt * sizeof(u8*)); modelTriangles = OS_Alloc(mdlsCnt * sizeof(u16*));
    InitializeFloatToHalfLUT();
    size_t ft_sz = (size_t)MAX_OUTPUT_VERTS * sizeof(u16), remap_sz = (size_t)MAX_OUTPUT_VERTS * sizeof(u32), nv_sz = (size_t)MAX_OUTPUT_VERTS * CPU_VRT_SZ, cache_sz = ((MAX_OUTPUT_VERTS/3) * sizeof(TriSort)) * 2 + (MAX_OUTPUT_VERTS * sizeof(u16));
    size_t arena = mdlsCnt*sizeof(i32) + mdlsCnt*sizeof(RawOBJ) + 11*threadCnt*sizeof(void*) + (size_t)threadCnt * ((MAX_VERT_ELEMENT_SIZE*3 + MAX_VERT_ELEMENT_SIZE*3 + MAX_VERT_ELEMENT_SIZE*2)*sizeof(float) + MAX_OUTPUT_VERTS*8*sizeof(float) + MAX_OUTPUT_VERTS*sizeof(u32) + HASH_SIZE*sizeof(u32) + MAX_OUTPUT_VERTS*sizeof(u32) + ft_sz + remap_sz + nv_sz + cache_sz);
    void* arena_base = OS_Alloc(arena); char* p = arena_base;
    i32* idxmap = (i32*)p; p += mdlsCnt*sizeof(i32);
    mset(idxmap, -1, mdlsCnt*sizeof(i32));
    for (u32 i=0; i<mp.count; ++i) if (mp.entries[i].index != U16_MAX) idxmap[mp.entries[i].index] = (i32)i;
    RawOBJ* raw = (RawOBJ*)p; p += mdlsCnt*sizeof(RawOBJ);
    for (u32 i=0; i<mdlsCnt; ++i) { i32 pi = idxmap[i]; if(pi >= 0){ FHandle d; int sz=0; raw[i].data=(const char*)OS_OpenAndAllocateFileBufferReadonly(mp.entries[pi].path,&d,&sz); raw[i].size=sz; raw[i].name=mp.entries[pi].path;} }
    float **pos = (float**)p; p += threadCnt*sizeof(float*); float **nrm = (float**)p; p += threadCnt*sizeof(float*);
    float **uv = (float**)p; p += threadCnt*sizeof(float*); float **ov = (float**)p; p += threadCnt*sizeof(float*);
    u16 **ot = (u16**)p; p += threadCnt*sizeof(u16*);
    u32 **ht = (u32**)p; p += threadCnt*sizeof(u32*); u32 **ht_used = (u32**)p; p += threadCnt*sizeof(u32*);
    u16 **ft_scr = (u16**)p; p += threadCnt*sizeof(u16*);
    u32 **remap_scr = (u32**)p; p += threadCnt*sizeof(u32*); u8 **nv_scr = (u8**)p; p += threadCnt*sizeof(u8*);
    u8 **cache_scr = (u8**)p; p += threadCnt*sizeof(u8*);
    size_t psz = MAX_VERT_ELEMENT_SIZE*3*sizeof(float), usz = MAX_OUTPUT_VERTS*8*sizeof(float), tsz = MAX_OUTPUT_VERTS*sizeof(u32);
    for (int i=0; i<threadCnt; ++i) { 
        pos[i] = (float*)p; p+=psz; nrm[i] = (float*)p; p += psz; uv[i] = (float*)p; p+=MAX_VERT_ELEMENT_SIZE*2*sizeof(float); 
        ov[i] = (float*)p; p+=usz; ot[i] = (u16*)p; p+=tsz; 
        ht[i] = (u32*)p; p+=HASH_SIZE*sizeof(u32); ht_used[i] = (u32*)p; p+=MAX_OUTPUT_VERTS*sizeof(u32);
        ft_scr[i] = (u16*)p; p+=ft_sz;
        remap_scr[i] = (u32*)p; p+=remap_sz; nv_scr[i] = (u8*)p; p+=nv_sz;
        cache_scr[i] = (u8*)p; p+=cache_sz;
        mset(ht[i], 0xFF, HASH_SIZE * sizeof(u32));
    }
    thrd_pos = pos; thread_temp_nrm = nrm; thrd_uv = uv; thrd_verts = ov; thrd_tris = ot;
    thrd_ht = ht; thrd_ht_used = ht_used;
    thrd_ft_scratch = ft_scr; thrd_remap_scratch = remap_scr; thrd_nv_scratch = nv_scr; thrd_cache_scratch = cache_scr;
    ModelParseTask tasks[32]; u32 chunk = (mdlsCnt + threadCnt - 1) / threadCnt; OS_Thread th[32];
    for (int i=0;i<threadCnt;++i) tasks[i] = (ModelParseTask){i*chunk,(i+1)*chunk > mdlsCnt ? mdlsCnt : (i+1)*chunk,raw,i};
    if (threadCnt > 1) {
        for (int i=0;i<threadCnt;++i) OS_ThreadCreate(&th[i],ModelParsingWorker,&tasks[i]);
        for (int i=0;i<threadCnt;++i) OS_ThreadJoin(&th[i]);
    } else { for (int t=0;t<threadCnt;++t) ModelParsingWorker(&tasks[t]); }
    glGenBuffers(mdlsCnt,vbos); glGenBuffers(mdlsCnt,tbos); u32 tv=0,tt=0;
    for (int i=0; i<mdlsCnt; ++i) {
        if (!modelVertexCounts[i]) continue;
        tv += modelVertexCounts[i]; tt += modelTriangleCounts[i]; size_t vcz = (size_t)modelVertexCounts[i] * VRT_ATT_SZ, tcz = (size_t)modelTriangleCounts[i] * 3 * sizeof(u16);
        glBindBuffer(GL_ARRAY_BUFFER,vbos[i]); glBufferData(GL_ARRAY_BUFFER,vcz,NULL,GL_STATIC_DRAW);
        half* mpv = (half*)glMapBufferRange(GL_ARRAY_BUFFER,0,vcz,0x0002/*GL_MAP_WRITE_BIT*/|0x0008/*GL_MAP_INVALIDATE_BUFFER_BIT*/);
        const float* csrc = (const float*)modelVertices[i]; u32 elems = modelVertexCounts[i] * 8;
        for (u32 k=0;k<elems;++k) mpv[k] = float_to_half(csrc[k]);
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,tbos[i]); glBufferData(GL_ELEMENT_ARRAY_BUFFER,tcz,NULL,GL_STATIC_DRAW); void* mpt = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER,0,tcz,0x0002/*GL_MAP_WRITE_BIT*/|0x0008/*GL_MAP_INVALIDATE_BUFFER_BIT*/); mcpy(mpt,modelTriangles[i],tcz); glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
        if (raw[i].data) OS_Free((void*)raw[i].data,raw[i].size);
    }
    glBindBuffer(GL_ARRAY_BUFFER,0); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0); glFlush(); glFinish();
    OS_Free(arena_base,arena); OS_Free(mp.entries,mp.count * sizeof(ModelData));
    DualLog(" vertices: %u, tris: %u, %f secs\n",tv,tt,get_time() - startModelTime);
    DebugRAM("After LoadModels");
}
