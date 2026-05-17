// data_models.c - Load 3D Models
void qsort(void* base, size_t nmemb, size_t size, int (*cmp)(const void*, const void*));
u8** modelVertices = NULL; u16** modelTriangles = NULL;
u32 modelVertexCounts[MODEL_IDX_MAX] = {0}; u16 modelTriangleCounts[MODEL_IDX_MAX] = {0};
float modelBounds[MODEL_IDX_MAX] = {0}; u16 loadedModelsMaxIndex = 0;
#define MAX_VERT_ELEMENT_SIZE 6964
#define MAX_OUTPUT_VERTS      36364
static float **thread_temp_pos = NULL, **thread_temp_nrm = NULL, **thread_temp_uv = NULL, **thread_out_verts = NULL; static u16** thread_out_tris = NULL;
typedef struct { const char* data; int size; } RawOBJ;
typedef struct { u16 index; bool animated; u8 animationNum; char path[128]; } ModelData;
typedef struct { ModelData* entries; u32 count; u32 capacity; } ModelDataParser;
typedef u16 half;
static Vector3 normalsTable[256];
static inline float NormDot(Vector3 a, Vector3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
void BuildNormalTable(void) {
    int n = 0;
    normalsTable[n++] = (Vector3){ 1, 0, 0}; // Axis-aligned
    normalsTable[n++] = (Vector3){-1, 0, 0};
    normalsTable[n++] = (Vector3){ 0, 1, 0};
    normalsTable[n++] = (Vector3){ 0,-1, 0};
    normalsTable[n++] = (Vector3){ 0, 0, 1};
    normalsTable[n++] = (Vector3){ 0, 0,-1};
    const float s2 = 0.70710678118f; // 1/sqrt(2)
    const float edge45[12][3] = { { s2, s2, 0}, { s2,-s2, 0}, {-s2, s2, 0}, {-s2,-s2, 0}, { s2, 0, s2}, { s2, 0,-s2}, {-s2, 0, s2}, {-s2, 0,-s2}, { 0, s2, s2}, { 0, s2,-s2}, { 0,-s2, s2}, { 0,-s2,-s2}, };
    for (int i = 0; i < 12; i++) normalsTable[n++] = (Vector3){edge45[i][0], edge45[i][1], edge45[i][2]}; // Edge 45° normals
    for (int sx = -1; sx <= 1; sx += 2) {
        for (int sy = -1; sy <= 1; sy += 2) {
            for (int sz = -1; sz <= 1; sz += 2) normalsTable[n++] = (Vector3){sx*0.57735026919f, sy*0.57735026919f, sz*0.57735026919f}; // Corner 45° normals, using 1/sqrt(3)
        }
    }

    for (int fi=0;n<256;fi++) { // Fill remaining slots with Fibonacci sphere
        float z = 1.0f - (2.0f * (fi + 0.5f)) / 256.0f;
        float r = vsqrtf(1.0f - z * z);
        float phi = 5.083203757f * (float)fi; // Golden angle in radians: PI * (1 + sqrt(5))
        Vector3 cand = { r * vcosf(phi), r * vsinf(phi), z };
        bool tooClose = false;
        for (int j = 0; j < n; j++) {
            if (NormDot(cand,normalsTable[j]) > 0.966f) { tooClose = true; break; }
        }
        if (!tooClose) normalsTable[n++] = cand;
    }
}

// Finds the index of the table entry whose direction best matches (nx,ny,nz).
// Brute-force linear scan — 256 dot products, trivially fast per-vertex.
static inline __attribute__((always_inline)) u8 QuantizeNormal(float nx, float ny, float nz) {
    float best = -2.0f;
    u8 idx  =  0;
    for (int i=0;i<256;++i) {
        float dot = nx * normalsTable[i].x + ny * normalsTable[i].y + nz * normalsTable[i].z;
        if (dot > best) { best = dot; idx = (u8)i; }
    }
    return idx;
}

static inline __attribute__((always_inline)) half float_to_half(float f) {
    u32 x; CopyMemoryFromBtoAForNBytes(&x,&f,4);
    u32 s = x>>31, ue = (x>>23)&0xff; i32 e = (i32)ue-127; u32 m = x&0x7fffff;
    if (ue == 0xff) return m ? (half)(0x7e00|(m>>13)|(s<<15)) : (half)((s<<15)|0x7c00);
    if (!ue && !m) return (half)(s<<15);
    if (e <= -24) return (half)(s<<15);
    if (e <= -14) { m = (m|0x800000) >> (-e-1); return (half)((s<<15)|(m>>13)); }
    if (e <= 15) { m += 0x1000; if (m >= 0x800000) { m = 0; e++; } return (half)((s<<15)|((e+15)<<10)|(m>>13)); }
    return (half)((s<<15)|0x7c00);
}

static inline __attribute__((always_inline)) float fast_atof(const char** p) {
    float v=0,s=1; while (**p == ' ' || **p == '\t') (*p)++;
    if (**p == '-') { s = -1; (*p)++; }
    while (**p >= '0' && **p <= '9') v = v*10 + (*(*p)++ - '0');
    if (**p == '.') { (*p)++; float sub = 0.1f; while (**p >= '0' && **p <= '9') { v += (*(*p)++ - '0')*sub; sub *= 0.1f; } }
    return s * v;
}

static inline __attribute__((always_inline)) i32 fast_atoi(const char** p) {
    i32 v = 0, s = 1; while (**p == ' ' || **p == '\t') (*p)++;
    if (**p == '-') { s = -1; (*p)++; }
    while (**p >= '0' && **p <= '9') v = v*10 + (*(*p)++ - '0');
    return v * s;
}

typedef struct { u32 idx,key; } TriSort;
int cmp(const void* a, const void* b) { u32 ka=((const TriSort*)a)->key, kb=((const TriSort*)b)->key; return (ka > kb) - (ka < kb); } // branchless 1 or -1
static void OptimizeVertexCache(u16* idx, u32 ic, u32 vc) {
    if (ic < 3 || !vc) return;
    
    u32 tc = ic / 3;
    TriSort* t = OS_Alloc(tc*sizeof(TriSort));
    for (u32 i=0; i<tc; ++i) {
        u16* p = idx+i*3; u32 m = p[0]<p[1]?p[0]:p[1]; m = m<p[2]?m:p[2];
        t[i].idx = i; t[i].key = m;
    }
    qsort(t, tc, sizeof(TriSort), cmp);
    u16* n = OS_Alloc(ic*sizeof(u16));
    for (u32 i=0; i<tc; ++i) { u16* s=idx+t[i].idx*3; u16* d=n+i*3; d[0]=s[0];d[1]=s[1];d[2]=s[2]; }
    CopyMemoryFromBtoAForNBytes(idx,n,ic*sizeof(u16));
    OS_DeallocateRAM(n, ic*sizeof(u16)); OS_DeallocateRAM(t, tc*sizeof(TriSort));
}

static u8* OptimizeVertexFetch(u8* v, u32* vc, u16* idx, u32 ic, size_t stride) {
    u32 oc = *vc; if (!oc || !ic) return v;
    u32 *remap = OS_Alloc(oc*sizeof(u32)), *first = OS_Alloc(oc*sizeof(u32));
    MemSetToValueForNBytes(remap,0xFF,oc*sizeof(u32));
    u32 nc = 0;
    for (u32 i=0; i<ic; ++i) {
        u32 id = idx[i];
        if (id < oc && remap[id] == 0xFFFFFFFFU) { remap[id] = nc; first[nc] = id; ++nc; }
    }
    u8* nv = OS_Alloc(nc*stride);
    for (u32 i=0; i<nc; ++i) CopyMemoryFromBtoAForNBytes(nv+i*stride,v+first[i]*stride,stride);
    for (u32 i=0; i<ic; ++i) if (idx[i] < oc) idx[i] = (u16)remap[idx[i]];
    *vc = nc;
    OS_DeallocateRAM(remap,oc*sizeof(u32)); OS_DeallocateRAM(first,oc*sizeof(u32));
    return nv;
}

static __attribute__((hot)) __attribute__((flatten)) bool ParseOBJ(u32 mindex, const char* __restrict d, int fs, float* __restrict tp, float* __restrict tn, float* __restrict tu, float* __restrict sv, u16* __restrict st, u8** ov, u32* ovc, u16** ot, u16* otc) {
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
            if (*p == ' ') { if (unlikely(pc >= MAX_VERT_ELEMENT_SIZE)) return false;
                ++p; tp[pc*3] = fast_atof(&p); tp[pc*3+1] = fast_atof(&p); tp[pc*3+2] = fast_atof(&p); ++pc;
            } else if (*p == 'n' && p[1] == ' ') { p += 2;
                if (unlikely(nc >= MAX_VERT_ELEMENT_SIZE)) return false;
                tn[nc*3] = fast_atof(&p); tn[nc*3+1] = fast_atof(&p); tn[nc*3+2] = fast_atof(&p); ++nc;
            } else if (*p == 't' && p[1] == ' ') { p += 2;
                if (unlikely(uc >= MAX_VERT_ELEMENT_SIZE)) return false;
                tu[uc*2] = fast_atof(&p); tu[uc*2+1] = fast_atof(&p); ++uc;
            }
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
                if (unlikely(ec + 3 > MAX_OUTPUT_VERTS)) {DualLogError("vert overflow!\n"); return false;}
                u32 tri[3] = {0, (u32)k, (u32)(k+1)};
                for (int t=0; t<3; ++t) {
                    int ix = tri[t];
                    u32 v = vi[ix] ? vi[ix]-1 : 0;
                    u32 tex = (ti[ix] && ti[ix] <= uc) ? ti[ix]-1 : 0;
                    u32 nrm = (ni[ix] && ni[ix] <= nc) ? ni[ix]-1 : 0;
                    float* dst = sv + (ec<<3);
                    dst[0] = -tp[v*3];   dst[1] =  tp[v*3+1]; dst[2] =  tp[v*3+2];
                    dst[3] = (nrm < nc) ? -tn[nrm*3]   : 0;
                    dst[4] = (nrm < nc) ?  tn[nrm*3+1] : 0;
                    dst[5] = (nrm < nc) ?  tn[nrm*3+2] : 0;
                    dst[6] = (tex < uc) ?  tu[tex*2]   : 0;
                    dst[7] = (tex < uc) ?  tu[tex*2+1] : 0;
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

    DualLog("[%u]Finished a main parse OBJ.\n",mindex);
    #define HASH_SIZE 65536
    //u32 ht[HASH_SIZE]; MemSetToValueForNBytes(ht, 0xFF, sizeof(ht));
    u32* ht = OS_Alloc(HASH_SIZE * sizeof(u32));
    MemSetToValueForNBytes(ht,0xFF,HASH_SIZE * sizeof(u32));
    u32* rem = (u32*)st; u32 ucnt = 0;
    for (u32 i=0; i<ec; ++i) {
        const float* v = sv + (i<<3);
        u64 h = *(u32*)(v+0) ^ *(u32*)(v+2) ^ *(u32*)(v+4) ^ *(u32*)(v+6);
        u32 s = (u32)(h ^ (h>>32)) & (HASH_SIZE-1);
        while (ht[s] != 0xFFFFFFFFU) {
            if (CompareMemoryForNBytes(sv+(ht[s]<<3), v, 32) == 0) { rem[i] = ht[s]; goto nxt; }
            s = (s+1) & (HASH_SIZE-1);
        }
        ht[s] = ucnt; rem[i] = ucnt;
        CopyMemoryFromBtoAForNBytes(sv+(ucnt<<3), v, 32);
        ++ucnt; nxt:;
    }

    DualLog("[%u]Finished a parse OBJ copy.\n",mindex);
    u8* fv = OS_Alloc((size_t)ucnt * VERTEX_ATTRIBUTES_SIZE);
    u8* dst = fv;
    for (u32 i=0; i<ucnt; ++i) {
        const float* src = sv + (i<<3);
        for (u32 j=0; j<8; ++j) { *(half*)dst = float_to_half(src[j]); dst += 2; }
    }

    DualLog("[%u]Finished a parse OBJ float to half (16bit) conversion.\n",mindex);
    u16* ft = OS_Alloc(ec * sizeof(u16));
    for (u32 i=0; i<ec; ++i) ft[i] = (u16)rem[i];
    OptimizeVertexCache(ft, ec, ucnt);
    DualLog("[%u]Finished a parse OBJ vertex cache optimization.\n",mindex);
    u32 oldc = ucnt;
    u8* optv = OptimizeVertexFetch(fv, &ucnt, ft, ec, VERTEX_ATTRIBUTES_SIZE);
    DualLog("[%u]Finished a parse OBJ vertex fetch optimization.\n",mindex);
    OS_DeallocateRAM(fv, oldc * VERTEX_ATTRIBUTES_SIZE);
    *ov = optv; *ovc = ucnt; *ot = ft; *otc = ec/3;
    float rad = vmax(vabs(mx),vmax(vabs(my),vmax(vabs(mz),vmax(Mx,vmax(My,Mz)))));
    modelBounds[mindex] = rad;
    OS_DeallocateRAM(ht,HASH_SIZE * sizeof(u32));
    return true;
}

typedef struct { u32 start, end; RawOBJ* raw; int tid; } ModelParseTask;
static void* ModelParsingWorker(void* arg) {
    ModelParseTask* t = arg;
    for (u32 i = t->start; i < t->end; ++i) {
        RawOBJ obj = t->raw[i];
        if (unlikely(!obj.data || obj.size <= 0)) continue;

        if (!ParseOBJ(i,obj.data,obj.size,thread_temp_pos[t->tid],thread_temp_nrm[t->tid],thread_temp_uv[t->tid],thread_out_verts[t->tid],thread_out_tris[t->tid],&modelVertices[i],&modelVertexCounts[i],&modelTriangles[i],&modelTriangleCounts[i])) continue;
    }
    return NULL;
}

bool ParseModelData(ModelDataParser *p, u16 maxSz, const char *fn) {
    OsFileHandle fd; int sz; char* buf = OS_OpenAndAllocateFileBufferReadonly(fn, &fd, &sz);
    char *c = buf, *e = buf + sz; u32 maxidx = 0, ln = 0;
    while (c < e) {  // first pass - find max index
        char* s = c; while (c < e && *c != '\n' && *c != '\r') ++c;
        if (c - s > 5) {
            while (CharacterIsEmpty(*s)) ++s;
            char* col = StringFindFirstCharWithin(s, ':');
            if (col && StringCompareUpToLength(s, "index", col - s) == 0) {
                u32 idx = parse_numberu32(col+1, s, ln);
                if (idx > maxidx) maxidx = idx;
            }
        }
        if (c < e && *c == '\r') ++c; if (c < e && *c == '\n') ++c; ++ln;
    }

    if (!maxidx) { DualLogWarn("No entries in %s\n", fn); OS_DeallocateRAM(buf,sz); return true; }
    if (maxidx >= maxSz) { DualLogWarn("Index too large in %s\n", fn); OS_DeallocateRAM(buf,sz); return true; }

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

        while (CharacterIsEmpty(*s)) ++s;
        char* le = s + len - 1; while (le > s && CharacterIsEmpty(*le)) --le;
        if (*s == '/' && s[1] == '/') goto next;
        if (*s == '#') {
            if (cur.path[0] && cur.index != U16_MAX && cur.index < cnt) ents[cur.index] = cur;
            cur = (ModelData){U16_MAX, false, 255, {0}};
            if (le > s) {
                size_t pl = le - s; if (pl >= sizeof(cur.path)) pl = sizeof(cur.path)-1;
                CopyMemoryFromBtoAForNBytes(cur.path,s+1,pl); cur.path[pl] = 0;
            }
            goto next;
        }

        char* col = StringFindFirstCharWithin(s, ':');
        if (col) {
            char k[256]={0}, v[256]={0};
            StringCopyInto_A_SubstringFrom_B(k, col-s, s, 256);
            StringCopyInto_A_SubstringFrom_B(v, le-col, col+1, 256);
            if (StringsEqual(k,"index"))             cur.index = parse_numberu16(v, s, ln);
            else if (StringsEqual(k,"animationNum")) cur.animationNum = parse_numberu16(v, s, ln);
            else if (StringsEqual(k,"animated"))     cur.animated = parse_numberu8(v, s, ln);
        }
        next:
        if (c < e && *c == '\r') ++c; if (c < e && *c == '\n') ++c;
    }
    if (cur.path[0] && cur.index != U16_MAX && cur.index < cnt) ents[cur.index] = cur;
    OS_DeallocateRAM(buf, sz);
    return true;
}

static void UploadMdlBuffer(u32 target, u32 buf, const void* data, size_t size) {
    glBindBuffer(target,buf); glBufferData(target,size,NULL,GL_STATIC_DRAW);
    void* mp = glMapBufferRange(target,0,size,0x0002/*GL_MAP_WRITE_BIT*/|0x0008/*GL_MAP_INVALIDATE_BUFFER_BIT*/);
    CopyMemoryFromBtoAForNBytes(mp,data,size); glUnmapBuffer(target);
}

void LoadModels(void) {
    double startModelTime = get_time();
    ModelDataParser mp = {0};
    if (!ParseModelData(&mp, MODEL_IDX_MAX,"./Data/models.txt")) { DualLogError("Failed models.txt\n"); OS_Exit(1); }

    DualLog("Loading models (%d) ...",mp.count);
    BuildNormalTable();
    u32 maxid = 0;
    for (u32 i=0; i<mp.count; ++i) { if (mp.entries[i].index != U16_MAX && mp.entries[i].index > maxid) maxid = mp.entries[i].index; }
    loadedModelsMaxIndex = (u16)maxid + 1;
    num_parse_threads = clamp(OS_GetNumThreads(), 1, 32);
    DualLog("Model loading using %u threads\n",num_parse_threads);
    modelVertices  = OS_Alloc(loadedModelsMaxIndex * sizeof(u8*));
    modelTriangles = OS_Alloc(loadedModelsMaxIndex * sizeof(u16*));
    size_t n = loadedModelsMaxIndex;
    size_t arena = n*sizeof(i32) + n*sizeof(RawOBJ) + 5*n*sizeof(float*) + (size_t)num_parse_threads * ((MAX_VERT_ELEMENT_SIZE*3 + MAX_VERT_ELEMENT_SIZE*3 + MAX_VERT_ELEMENT_SIZE*2)*sizeof(float) + MAX_OUTPUT_VERTS*8*sizeof(float) + MAX_OUTPUT_VERTS*sizeof(u32));
    void* arena_base = OS_Alloc(arena); char* p = arena_base;
    i32* idxmap = (i32*)p; p += n*sizeof(i32);
    MemSetToValueForNBytes(idxmap, -1, n*sizeof(i32));
    for (u32 i=0; i<mp.count; ++i) if (mp.entries[i].index != U16_MAX) idxmap[mp.entries[i].index] = (i32)i;
    RawOBJ* raw = (RawOBJ*)p; p += n*sizeof(RawOBJ);
    DualLog("file allocation loop start...\n");
    for (u32 i=0; i<n; ++i) {
        i32 pi = idxmap[i];
        if (pi >= 0) {
            const char* path = mp.entries[pi].path;
            OsFileHandle dummy; int sz=0;
            raw[i].data = (const char*)OS_OpenAndAllocateFileBufferReadonly(path,&dummy,&sz);
            raw[i].size = sz;
        }
    }

    DualLog("Prep threads...\n");
    float **pos = (float**)p; p += num_parse_threads*sizeof(float*);
    float **nrm = (float**)p; p += num_parse_threads*sizeof(float*);
    float **uv  = (float**)p; p += num_parse_threads*sizeof(float*);
    float **ov  = (float**)p; p += num_parse_threads*sizeof(float*);
    u16  **ot   =   (u16**)p; p += num_parse_threads*sizeof(u16*);
    size_t psz = MAX_VERT_ELEMENT_SIZE*3*sizeof(float);
    size_t usz = MAX_OUTPUT_VERTS*8*sizeof(float);
    size_t tsz = MAX_OUTPUT_VERTS*sizeof(u32);
    for (int i=0; i<num_parse_threads; ++i) {
        pos[i] = (float*)p; p += psz;
        nrm[i] = (float*)p; p += psz;
        uv[i]  = (float*)p; p += MAX_VERT_ELEMENT_SIZE*2*sizeof(float);
        ov[i]  = (float*)p; p += usz;
        ot[i]  = (u16*)p;   p += tsz;
    }

    thread_temp_pos = pos; thread_temp_nrm = nrm; thread_temp_uv = uv;
    thread_out_verts = ov; thread_out_tris = ot;
    ModelParseTask tasks[32];
    u32 chunk = (loadedModelsMaxIndex + num_parse_threads - 1) / num_parse_threads;
    DualLog("Executing model parse tasks...\n"); // last printed output
    for (int i=0;i<num_parse_threads;++i) tasks[i] = (ModelParseTask){i*chunk,(i+1)*chunk > loadedModelsMaxIndex ? loadedModelsMaxIndex : (i+1)*chunk,raw,i};
    pthread_t th[32];
    if (num_parse_threads > 3) {
        DualLog("Executing model parse thread creation...\n");
        for (int i=0;i<num_parse_threads;++i) pthread_create(&th[i],NULL,ModelParsingWorker,&tasks[i]);
        DualLog("Executing model parse thread joins...\n");
        for (int i=0;i<num_parse_threads;++i) pthread_join(th[i],NULL);
    } else { for (int t=0;t<num_parse_threads;++t) ModelParsingWorker(&tasks[t]); } // Single threaded fallback
    
    DualLog("Model glGenBuffers...\n");
    glGenBuffers(loadedModelsMaxIndex,Sys_Render.vbos); glGenBuffers(loadedModelsMaxIndex,Sys_Render.tbos);
    u32 tv=0,tt=0;
    for (int i=0; i<loadedModelsMaxIndex; ++i) {
        DualLog("GPU transfer model %u\n",i);
        if (!modelVertexCounts[i]) continue;

        tv += modelVertexCounts[i]; tt += modelTriangleCounts[i];
        UploadMdlBuffer(GL_ARRAY_BUFFER,Sys_Render.vbos[i],modelVertices[i],(size_t)modelVertexCounts[i] * VERTEX_ATTRIBUTES_SIZE);
        UploadMdlBuffer(GL_ELEMENT_ARRAY_BUFFER,Sys_Render.tbos[i],modelTriangles[i],(size_t)modelTriangleCounts[i] * 3 * sizeof(u16));
    }

    for (u32 i=0;i<loadedModelsMaxIndex;++i) { if (raw[i].data) OS_DeallocateRAM((void*)raw[i].data,raw[i].size); }
    OS_DeallocateRAM(arena_base, arena);
    DualLog("Model deallocs...\n");
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
    glFlush(); glFinish();
    OS_DeallocateRAM(mp.entries,mp.count * sizeof(ModelData));
    DualLog(" vertices: %u, tris: %u, %f secs\n",tv,tt,get_time() - startModelTime);
    DebugRAM("After LoadModels");
} // 391
