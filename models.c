// models.c - 3D Models Loading System
#include "common.h"
#include "lib.h"
float **vPos, **thrd_pos, **thread_temp_nrm, **thrd_uv, **thrd_verts; u32 **thrd_ht, **thrd_ht_used, **thrd_remap_scratch; u8** thrd_cache_scratch;
typedef struct { const char* data; const char* name; int size; } RawOBJ; typedef struct { u16 index; bool animated; u8 animationNum; char path[128]; } ModelData; typedef struct { ModelData* entries; u32 count; u32 capacity; } ModelDataParser;
typedef struct { u32 start,end; int tid; } PhysGeomTask;
BvhNode** modelBVHNodes; u16** modelBVHTriOrder; u32 modelBVHNodeCounts[MAX_MDLS],modelBVHTriOrderCounts[MAX_MDLS];
typedef struct { BvhNode* nodes; u8* triOctants; u16 *triOrder, *triScratch,*initialTris; u32 nodeCount,triCount; } BvhBuildCtx;
BvhBuildCtx thrd_bvh_ctx[32];
u16 uniqueCvxMeshIndices[MAX_UNIQUE_CVX_MESHES]; u32 uniqueCvxMeshCount=0;
u32** cvxAdjOffsets = NULL; u16** cvxAdjLists = NULL; // CSR format adjacency data: cvxAdjOffsets[m] has vCount + 1 entries pointing into cvxAdjLists[m]
u16  cvxAdjStart[MAX_UNIQUE_CVX_MESHES];
u16  lastCvxSupport[MAX_UNIQUE_CVX_MESHES] = {0}; // Persistent hill-climbing state per mesh
int EdgeCompare(const void* a, const void* b) { u32 ea = *(const u32*)a, eb = *(const u32*)b; return (ea > eb) - (ea < eb); }
INLINE float fast_atof(const char** p) { const char* c=*p; while (*c == ' ' || *c == '\t') {c++;} float s=1.0f; if(*c == '-'){s=-1.0f; c++;} float v=0.0f; while (*c >= '0' && *c <= '9') { v=v * 10.0f + (*c - '0'); c++; } if (*c == '.') { c++; float sub=0.1f; while (*c >= '0' && *c <= '9') { v += (*c - '0') * sub; sub*=0.1f; c++; } } *p=c; return s * v; }
INLINE i32 fast_atoi(const char** p) { const char* c = *p; while (*c == ' ' || *c == '\t') {c++;} i32 s=1; if(*c == '-'){s=-1; c++;} i32 v = 0; while (*c >= '0' && *c <= '9') { v = v * 10 + (*c - '0'); c++; } *p = c; return v * s; }
typedef struct {u32 idx,key;} TriSort;
int cmp(const void* a, const void* b) { u32 ka=((const TriSort*)a)->key, kb=((const TriSort*)b)->key; return (ka > kb) - (ka < kb); } // branchless 1 or -1
void OptimizeVertexCache(u16* idx, u32 ic, u32 vc, u8* scratch) {
    if (ic < 3 || !vc) return;
    u32 tc = ic / 3;
    TriSort* t = (TriSort*)scratch;
    TriSort* t_tmp = (TriSort*)(scratch + (tc * sizeof(TriSort)));
    u16* n = (u16*)(scratch + (tc * sizeof(TriSort) * 2));
    for (u32 i = 0; i < tc; ++i) { u16* p = idx + i * 3; u32 m = p[0] < p[1] ? p[0] : p[1]; m = m < p[2] ? m : p[2]; t[i].idx = i; t[i].key = (u16)m; }
    if (tc >= 2) {
        //RadixSortTriangles(t,t_tmp,tc);
        u32 b0[256]={0}, b1[256]={0};
        for (u32 i = 0; i < tc; ++i) {u16 key = t[i].key; b0[key & 0xFF]++; b1[(key >> 8) & 0xFF]++;}
        u32 sum0=0, sum1=0;
        for (u32 i = 0; i < 256; ++i) { u32 t0 = b0[i]; u32 t1 = b1[i]; b0[i] = sum0; b1[i] = sum1; sum0 += t0; sum1 += t1; }
        for (u32 i = 0; i < tc; ++i) {u32 radix0 = t[i].key & 0xFF; u32 dest = b0[radix0]++; t_tmp[dest] = t[i];}
        for (u32 i = 0; i < tc; ++i) { u32 radix1 = (t_tmp[i].key >> 8) & 0xFF; u32 dest = b1[radix1]++; t[dest] = t_tmp[i]; }
    }
    for (u32 i = 0; i < tc; ++i) { u16* s = idx + t[i].idx * 3; u16* d = n + i * 3; d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; }
    mcpy(idx, n, ic * sizeof(u16));
}

u8* OptimizeVertexFetch(u8* v, u32* vc, u16* idx, u32 ic, size_t stride, u32* remap, u8* nv) {
    u32 oc = *vc; if (!oc || !ic) return v;
    mset(remap,0xFF,oc * sizeof(u32)); u32 nc = 0;      for(u32 i=0;i<ic;++i) { u32 id = idx[i]; if (id < oc && remap[id] == 0xFFFFFFFFU) { remap[id]=nc; ++nc; } }
    mset(remap,0xFF,oc * sizeof(u32)); u32 write_ptr=0; for(u32 i=0;i<ic;++i) { u32 id = idx[i]; if (id < oc) { if (remap[id] == 0xFFFFFFFFU) { remap[id]=write_ptr; mcpy(nv + write_ptr * stride, v + id * stride, stride); write_ptr++; } idx[i]=(u16)remap[id]; } }
    *vc = nc;
    return nv;
}

#define _mm_min_ps(A, B) ((__m128)__builtin_ia32_minps((__v4sf)(A), (__v4sf)(B)))
#define _mm_max_ps(A, B) ((__m128)__builtin_ia32_maxps((__v4sf)(A), (__v4sf)(B)))
static __attribute__((hot)) __attribute__((flatten)) bool ParseOBJ(u32 mindex, const char* __restrict d, int fs, float* __restrict tp, float* __restrict tn, float* __restrict tu, float* __restrict sv, u32* __restrict ht, u32* __restrict ht_used, u32* __restrict remap_scr, u8* __restrict cache_scr, float** __restrict ov_pos, u32* ovc, u16** ot, u16* otc) {
    *ov_pos=NULL; *ot=NULL; *ovc=*otc=0; u32 pc=0,nc=0,uc=0,ec=0; __m128 mn_v=_mm_set1_ps(1e9f), mx_v=_mm_set1_ps(-1e9f); const char *p=d, *e=d+fs;
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
                if (unlikely(ec + 3 > MAX_OUTPUT_VERTS)) {DualLogError("vert overflow!\n"); return false;}
                u32 tri[3] = {0, (u32)k, (u32)(k+1)};
                for (int t=0; t<3; ++t) {
                    int ix = tri[t]; u32 v = vi[ix] ? vi[ix]-1 : 0; u32 tex = (ti[ix] && ti[ix] <= uc) ? ti[ix]-1 : 0; u32 nrm = (ni[ix] && ni[ix] <= nc) ? ni[ix]-1 : 0; float* dst = sv + (ec<<3);
                    dst[0]=-tp[v*3]; dst[1]=tp[v*3+1]; dst[2]=tp[v*3+2]; dst[3]=(nrm < nc) ? -tn[nrm*3] : 0; dst[4]=(nrm < nc) ? tn[nrm*3+1] : 0; dst[5]=(nrm < nc) ? tn[nrm*3+2] : 0; dst[6]=(tex < uc) ? tu[tex*2] : 0; dst[7]=(tex < uc) ? tu[tex*2+1] : 0;
                    __m128 pos_v=_mm_loadu_ps(dst); mn_v=_mm_min_ps(mn_v,pos_v); mx_v=_mm_max_ps(mx_v,pos_v); ++ec;
                }
            }
        skip:;
        } else while (p < e && *p != '\n') ++p;
    }
    if (unlikely(!ec)) return false;
    u16* final_t = OS_Alloc(ec * sizeof(u16)); // Allocate final_t early so we can use it instead of ft_scratch
    u32 used_slots_count = 0; 
    u32* rem = (u32*)remap_scr; // Reuse remap_scr for the 'rem' array!
    u32 ucnt = 0;
    for (u32 i=0; i<ec; ++i) {
        const float* v = sv + (i<<3);
        const u32* uv = (const u32*)v; u32 h0 = uv[0] ^ uv[1] ^ uv[2] ^ uv[3]; u32 h1 = uv[4] ^ uv[5] ^ uv[6] ^ uv[7]; u32 s = (h0 ^ h1) & (WELD_HASH_SIZE-1);
        while (ht[s] != 0xFFFFFFFFU) { if (mcmp(sv+(ht[s]<<3), v, 32) == 0) { rem[i] = ht[s]; goto nxt; } s = (s+1) & (WELD_HASH_SIZE-1); }
        ht[s] = ucnt; rem[i] = ucnt; ht_used[used_slots_count++] = s; mcpy(sv+(ucnt<<3), v, 32); ++ucnt; nxt:;
    }
    for (u32 i=0;i<ec;++i) final_t[i] = (u16)rem[i];
    OptimizeVertexCache(final_t,ec,ucnt,cache_scr);
    float* final_verts = (float*)OS_Alloc((size_t)ucnt * CPU_VRT_SZ);
    OptimizeVertexFetch((u8*)sv,&ucnt,final_t,ec,CPU_VRT_SZ,remap_scr,(u8*)final_verts);
    *ov_pos = final_verts; *ovc = ucnt; *ot = final_t; *otc = ec/3; 
    float mn_arr[4], mx_arr[4]; _mm_storeu_ps(mn_arr,mn_v); _mm_storeu_ps(mx_arr,mx_v);
    modelBounds[mindex] = vmax(vabs(mn_arr[0]),vmax(vabs(mn_arr[1]),vmax(vabs(mn_arr[2]),vmax(mx_arr[0],vmax(mx_arr[1],mx_arr[2])))));
    for (u32 i = 0; i < used_slots_count; ++i) {ht[ht_used[i]] = 0xFFFFFFFFU;}
    return true;
}

// Recursive centroid-based, each tri goes into exactly one octant containing its centroid, no tri dupes. The node AABB is the union of its tri AABBs (NOT the octant AABB) — guarantees any query that overlaps a tri also overlaps its ancestor nodes, so traversal never misses a tri. triIdxArray is modified in-place: on return it is partitioned by octant so that each child's triangles are contiguous (matches the leaf ranges written to ctx->triOrder).
static i32 BvhBuildOctree(BvhBuildCtx* __restrict ctx, u16 m, const float* __restrict pos, const u16* __restrict tris, u16* triIdxArray, u32 triCount, u32 depth) {
    if (triCount == 0) return -1;
    if (ctx->nodeCount >= BVH_MAX_NODES_PER_MDL) depth = BVH_MAX_DEPTH;
    i32 nodeIdx = ctx->nodeCount++; BvhNode* node = &ctx->nodes[nodeIdx]; node->triStart = 0; node->triCount = 0;
    for(int i = 0; i < 8; i++){node->children[i]=-1;}
    __m128 mn_v=_mm_set1_ps(1e9f); __m128 mx_v=_mm_set1_ps(-1e9f);
    for (u32 i = 0; i < triCount; i++) {
        u32 triIdx = triIdxArray[i]; 
        u32 i0=tris[triIdx*3+0], i1=tris[triIdx*3+1], i2=tris[triIdx*3+2];
        __m128 v0 = _mm_loadu_ps(pos + (size_t)i0*3);
        __m128 v1 = _mm_loadu_ps(pos + (size_t)i1*3);
        __m128 v2 = _mm_loadu_ps(pos + (size_t)i2*3);
        mn_v = _mm_min_ps(mn_v, _mm_min_ps(_mm_min_ps(v0, v1), v2));
        mx_v = _mm_max_ps(mx_v, _mm_max_ps(_mm_max_ps(v0, v1), v2));
    }
    float mn_arr[4], mx_arr[4]; _mm_storeu_ps(mn_arr, mn_v); _mm_storeu_ps(mx_arr, mx_v); node->mn = (V3){mn_arr[0], mn_arr[1], mn_arr[2]}; node->mx = (V3){mx_arr[0], mx_arr[1], mx_arr[2]};
    if (depth >= 3 || triCount <= BVH_LEAF_MAX_TRIS || ctx->nodeCount + 8 > BVH_MAX_NODES_PER_MDL) {
        u32 startIdx = ctx->triCount;
        for (u32 i = 0; i < triCount && ctx->triCount < BVH_MAX_TRIS_PER_MDL; i++) { ctx->triOrder[ctx->triCount++] = triIdxArray[i]; }
        node->triStart = startIdx; node->triCount = (u16)triCount; return nodeIdx;
    }
    V3 center = V3_ScaleByF(V3_AplusB(node->mn, node->mx), 0.5f); __m128 center_v = _mm_setr_ps(center.x, center.y, center.z, 0.0f); __m128 third = _mm_set1_ps(1.0f/3.0f);
    u32 octantCounts[8] = {0};
    for (u32 i=0;i<triCount;++i) {
        u32 triIdx = triIdxArray[i];
        u32 i0 = tris[triIdx*3+0], i1 = tris[triIdx*3+1], i2 = tris[triIdx*3+2];
        __m128 v0 = _mm_loadu_ps(pos + (size_t)i0*3);
        __m128 v1 = _mm_loadu_ps(pos + (size_t)i1*3);
        __m128 v2 = _mm_loadu_ps(pos + (size_t)i2*3);
        __m128 centroid = _mm_mul_ps(_mm_add_ps(_mm_add_ps(v0, v1), v2), third);
        __v4si ge = (__v4si)(centroid >= center_v); // Compare centroid >= center. Returns a vector mask (all 1s if true, all 0s if false)
        u8 oct = (u8)((ge[0] & 1) | ((ge[1] & 1) << 1) | ((ge[2] & 1) << 2)); // Extract bits 0, 1, 2 from the mask
        ctx->triOctants[i] = oct; 
        octantCounts[oct]++;
    }
    u32 octantStarts[8];
    u32 total = 0;
    for (int o = 0; o < 8; o++) { octantStarts[o] = total; total += octantCounts[o]; }
    u32 octantFill[8] = {0};
    for (u32 i = 0; i < triCount; i++) { u8 o = ctx->triOctants[i]; ctx->triScratch[octantStarts[o] + octantFill[o]++] = triIdxArray[i]; }
    for (u32 i = 0; i < triCount; i++) { triIdxArray[i] = ctx->triScratch[i]; }
    for (int o = 0; o < 8; o++) {
        if (octantCounts[o] == 0) continue;
        i32 childIdx = BvhBuildOctree(ctx,m,pos,tris,triIdxArray + octantStarts[o],octantCounts[o],depth + 1); // <-- was missing pos,tris
        if (childIdx >= 0) node->children[o] = (i16)childIdx;
    }
    return nodeIdx;
}

static void BuildModelBVH(BvhBuildCtx* ctx, u16 m) {
    if (m >= mdlsCnt || m >= MAX_MDLS) return;
    modelBVHNodes[m] = NULL;
    modelBVHTriOrder[m] = NULL;
    modelBVHNodeCounts[m] = 0;
    modelBVHTriOrderCounts[m] = 0;
    u32 triCount = modelTriangleCounts[m];
    if (triCount == 0 || triCount > BVH_MAX_TRIS_PER_MDL) { DualLogError("Too many verts on model %u!  Could not build a BVH!\n",m); return;}
    if (!physPos[m] || !physTris[m]) return;
    ctx->nodeCount = ctx->triCount = 0;
    u16* initialTris = ctx->initialTris;
    for (u32 i = 0; i < triCount; i++) initialTris[i] = (u16)i;
    i32 rootIdx = BvhBuildOctree(ctx,m,physPos[m],physTris[m],initialTris,triCount,0);
    if (rootIdx < 0 || ctx->nodeCount == 0) return;
    modelBVHNodes[m] = (BvhNode*)OS_Alloc(ctx->nodeCount * sizeof(BvhNode));
    mcpy(modelBVHNodes[m], ctx->nodes, ctx->nodeCount * sizeof(BvhNode));
    modelBVHNodeCounts[m] = ctx->nodeCount;
    if (ctx->triCount > 0) {
        modelBVHTriOrder[m] = (u16*)OS_Alloc(ctx->triCount * sizeof(u16));
        if (modelBVHTriOrder[m]) { mcpy(modelBVHTriOrder[m],ctx->triOrder,ctx->triCount * sizeof(u16)); modelBVHTriOrderCounts[m] = ctx->triCount; }
    }
}

typedef struct { u32 start, end; RawOBJ* raw; int tid; } ModelParseTask;
static void* ModelParsingWorker(void* arg) {
    ModelParseTask* t = arg;
    for (u32 i = t->start; i < t->end; ++i) {
        RawOBJ obj = t->raw[i]; if (unlikely(!obj.data || obj.size <= 0)) continue;
        if (!ParseOBJ(i,obj.data,obj.size,thrd_pos[t->tid],thread_temp_nrm[t->tid],thrd_uv[t->tid],thrd_verts[t->tid],thrd_ht[t->tid],thrd_ht_used[t->tid],thrd_remap_scratch[t->tid],thrd_cache_scratch[t->tid],&vPos[i],&modelVertexCounts[i],&modelTriangles[i],&modelTriangleCounts[i])) continue;
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

float BvhRayAABBHit(V3 origin, V3 dir, V3 mn, V3 mx, float maxDist) { // Ray-vs-AABB slab test. Returns entry t (>=0) if the ray hits the AABB within [0, maxDist], or -1.0f if no hit. Handles axis-aligned rays (zero direction component) correctly.
    float tmin = 0.0f, tmax = maxDist;
    if (vabs(dir.x) < 1e-8f) { if (origin.x < mn.x || origin.x > mx.x) return -1.0f; } // X slab
    else {
        float inv = 1.0f / dir.x;
        float t1 = (mn.x - origin.x) * inv, t2 = (mx.x - origin.x) * inv;
        if (t1 > t2) { float t = t1; t1 = t2; t2 = t; }
        if (t1 > tmin) tmin = t1;
        if (t2 < tmax) tmax = t2;
        if (tmin > tmax) return -1.0f;
    }
    if (vabs(dir.y) < 1e-8f) { if (origin.y < mn.y || origin.y > mx.y) return -1.0f; } // Y slab
    else {
        float inv = 1.0f / dir.y;
        float t1 = (mn.y - origin.y) * inv, t2 = (mx.y - origin.y) * inv;
        if (t1 > t2) { float t = t1; t1 = t2; t2 = t; }
        if (t1 > tmin) tmin = t1;
        if (t2 < tmax) tmax = t2;
        if (tmin > tmax) return -1.0f;
    }
    if (vabs(dir.z) < 1e-8f) { if (origin.z < mn.z || origin.z > mx.z) return -1.0f; } // Z slab
    else {
        float inv = 1.0f / dir.z;
        float t1 = (mn.z - origin.z) * inv, t2 = (mx.z - origin.z) * inv;
        if (t1 > t2) { float t = t1; t1 = t2; t2 = t; }
        if (t1 > tmin) tmin = t1;
        if (t2 < tmax) tmax = t2;
        if (tmin > tmax) return -1.0f;
    }
    return tmin;
}

INLINE u32 WeldHash(i32 x, i32 y, i32 z) { u32 h = ((u32)x * 0x8DA6B343u) ^ ((u32)y * 0xD8163841u) ^ ((u32)z * 0xCB1AB31Fu); return h & (WELD_HASH_SIZE - 1); }
static void WeldModelPositions(u16 m, u32* weldHt, u32* weldHtUsed, u16* remap) {
    u32 vc = modelVertexCounts[m], tc = modelTriangleCounts[m];
    if (!vc || !tc) { physPos[m] = NULL; physTris[m] = NULL; physVertCounts[m] = 0; return; }
    const float* src = vPos[m]; mset(weldHt,0xFF,WELD_HASH_SIZE * sizeof(u32)); u32 usedSlots=0, weldedCount=0;
    float* weldedPos = (float*)OS_Alloc((size_t)vc * 3 * sizeof(float)); // worst case: no duplicates at all
    for (u32 i = 0; i < vc; ++i) {
        float x = src[i*8+0], y = src[i*8+1], z = src[i*8+2];
        i32 cx = (i32)vfloor(x * 10000), cy = (i32)vfloor(y * 10000), cz = (i32)vfloor(z * 10000);
        u32 found = 0xFFFFFFFFU;
        for (i32 dz = -1; dz <= 1 && found == 0xFFFFFFFFU; ++dz)
        for (i32 dy = -1; dy <= 1 && found == 0xFFFFFFFFU; ++dy)
        for (i32 dx = -1; dx <= 1 && found == 0xFFFFFFFFU; ++dx) {
            u32 slot = WeldHash(cx+dx, cy+dy, cz+dz);
            while (weldHt[slot] != 0xFFFFFFFFU) { u32 cand = weldHt[slot]; float ddx = weldedPos[cand*3+0]-x, ddy = weldedPos[cand*3+1]-y, ddz = weldedPos[cand*3+2]-z; if (ddx*ddx + ddy*ddy + ddz*ddz <= 0.0001f * 0.0001f) { found = cand; break; } slot = (slot + 1) & (WELD_HASH_SIZE - 1); }
        }
        if (found == 0xFFFFFFFFU) {
            found = weldedCount; weldedPos[weldedCount*3+0]=x; weldedPos[weldedCount*3+1]=y; weldedPos[weldedCount*3+2]=z; ++weldedCount; u32 slot = WeldHash(cx, cy, cz); 
            while (weldHt[slot] != 0xFFFFFFFFU) slot = (slot + 1) & (WELD_HASH_SIZE - 1);
            weldHt[slot] = found; weldHtUsed[usedSlots++] = slot;
        }
        remap[i] = (u16)found;
    }
    for (u32 i = 0; i < usedSlots; ++i) weldHt[weldHtUsed[i]] = 0xFFFFFFFFU; // reset shared scratch for the next model on this thread
    u16* weldedTris = (u16*)OS_Alloc((size_t)tc * 3 * sizeof(u16));
    const u16* srcTris = modelTriangles[m];
    for (u32 i = 0; i < tc * 3; ++i) weldedTris[i] = remap[srcTris[i]];
    physPos[m] = weldedPos; physTris[m] = weldedTris; physVertCounts[m] = weldedCount;
}

static void* PhysGeomWorker(void* a) { PhysGeomTask* t=a; BvhBuildCtx* bvhCtx=&thrd_bvh_ctx[t->tid]; u32* ht = thrd_ht[t->tid]; u32* u=thrd_ht_used[t->tid]; u16* sc=(u16*)thrd_remap_scratch[t->tid]; for (u32 m = t->start; m < t->end; ++m) { if(m >= mdlsCnt || !modelVertexCounts[m] || !modelTriangleCounts[m]){physPos[m]=NULL; physTris[m]=NULL; physVertCounts[m]=0; continue;}  WeldModelPositions((u16)m,ht,u,sc); BuildModelBVH(bvhCtx,(u16)m); }  return NULL; }
#define _mm256_cvtps_ph(A, imm) ((__m128i)__builtin_ia32_vcvtps2ph256((__v8sf)(__m256)(A), (int)(imm)))
void LoadModels() {
    double startModelTime = get_time();
    ModelDataParser mp = {0};
    if (!ParseModelData(&mp, MAX_MDLS,"./Data/models.txt")) { DualLogError("Failed models.txt\n"); OS_Exit(1); }
    DualLog("Loading   models (%d) ...",mp.count);
    u32 maxid = 0; for (u32 i=0; i<mp.count; ++i) { if (mp.entries[i].index != U16_MAX && mp.entries[i].index > maxid) maxid = mp.entries[i].index; } mdlsCnt = (u16)maxid + 1;
    vPos = OS_Alloc(mdlsCnt * sizeof(float*)); modelTriangles = OS_Alloc(mdlsCnt * sizeof(u16*));
    modelBVHNodes = (BvhNode**)OS_Alloc(mdlsCnt * sizeof(BvhNode*)); modelBVHTriOrder = (u16**)OS_Alloc(mdlsCnt * sizeof(u16*));
    size_t remap_sz = (size_t)MAX_OUTPUT_VERTS * sizeof(u32), cache_sz = ((MAX_OUTPUT_VERTS/3) * sizeof(TriSort)) * 2 + (MAX_OUTPUT_VERTS * sizeof(u16)); size_t bvh_nodes_sz = (size_t)BVH_MAX_NODES_PER_MDL * sizeof(BvhNode); size_t bvh_u8_sz = (size_t)BVH_MAX_TRIS_PER_MDL * sizeof(u8); size_t bvh_u16_sz = (size_t)BVH_MAX_TRIS_PER_MDL * sizeof(u16);
    size_t arena = mdlsCnt*sizeof(i32) + mdlsCnt*sizeof(RawOBJ) + 16*threadCnt*sizeof(void*) + (size_t)threadCnt * ((MAX_VERT_ELEMENT_SIZE*3 + MAX_VERT_ELEMENT_SIZE*3 + MAX_VERT_ELEMENT_SIZE*2)*sizeof(float) + MAX_OUTPUT_VERTS*8*sizeof(float) + WELD_HASH_SIZE*sizeof(u32) + MAX_OUTPUT_VERTS*sizeof(u32) + remap_sz + cache_sz + bvh_nodes_sz + bvh_u8_sz + 3*bvh_u16_sz);
    void* arena_base = OS_Alloc(arena); char* p = arena_base;
    i32* idxmap = (i32*)p; p += mdlsCnt*sizeof(i32);
    mset(idxmap, -1, mdlsCnt*sizeof(i32));
    for (u32 i=0; i<mp.count; ++i) if (mp.entries[i].index != U16_MAX) idxmap[mp.entries[i].index] = (i32)i;
    RawOBJ* raw = (RawOBJ*)p; p += mdlsCnt*sizeof(RawOBJ);
    for (u32 i=0; i<mdlsCnt; ++i) { i32 pi = idxmap[i]; if(pi >= 0){ FHandle d; int sz=0; raw[i].data=(const char*)OS_OpenAndAllocateFileBufferReadonly(mp.entries[pi].path,&d,&sz); raw[i].size=sz; raw[i].name=mp.entries[pi].path;} }
    float **pos = (float**)p; p += threadCnt*sizeof(float*); 
    float **nrm = (float**)p; p += threadCnt*sizeof(float*);
    float **uv = (float**)p; p += threadCnt*sizeof(float*); 
    float **ov = (float**)p; p += threadCnt*sizeof(float*);
    u32 **ht = (u32**)p; p += threadCnt*sizeof(u32*); 
    u32 **ht_used = (u32**)p; p += threadCnt*sizeof(u32*);
    u32 **remap_scr = (u32**)p; p += threadCnt*sizeof(u32*); 
    u8 **cache_scr = (u8**)p; p += threadCnt*sizeof(u8*);
    BvhNode **bvh_nodes_p = (BvhNode**)p; p += threadCnt*sizeof(BvhNode*);
    u8 **bvh_oct_p = (u8**)p; p += threadCnt*sizeof(u8*);
    u16 **bvh_order_p = (u16**)p; p += threadCnt*sizeof(u16*);
    u16 **bvh_scr_p = (u16**)p; p += threadCnt*sizeof(u16*);
    u16 **bvh_init_p = (u16**)p; p += threadCnt*sizeof(u16*);
    size_t psz = MAX_VERT_ELEMENT_SIZE*3*sizeof(float), usz = MAX_OUTPUT_VERTS*8*sizeof(float);
    for (int i=0; i<threadCnt; ++i) { 
        pos[i]=(float*)p; p+=psz; nrm[i] = (float*)p; p += psz; uv[i] = (float*)p; p+=MAX_VERT_ELEMENT_SIZE*2*sizeof(float); 
        ov[i]=(float*)p; p+=usz; 
        ht[i]=(u32*)p; p+=WELD_HASH_SIZE*sizeof(u32); ht_used[i] = (u32*)p; p+=MAX_OUTPUT_VERTS*sizeof(u32);
        remap_scr[i]=(u32*)p; p+=remap_sz; cache_scr[i]=(u8*)p; p+=cache_sz; bvh_nodes_p[i]=(BvhNode*)p; p += bvh_nodes_sz; bvh_oct_p[i]=(u8*)p; p += bvh_u8_sz; bvh_order_p[i]=(u16*)p; p += bvh_u16_sz; bvh_scr_p[i]=(u16*)p; p += bvh_u16_sz; bvh_init_p[i]=(u16*)p; p += bvh_u16_sz;
        mset(ht[i],0xFF,WELD_HASH_SIZE * sizeof(u32));
        thrd_bvh_ctx[i]=(BvhBuildCtx){.nodes=bvh_nodes_p[i], .triOctants=bvh_oct_p[i], .triOrder=bvh_order_p[i], .triScratch=bvh_scr_p[i], .initialTris=bvh_init_p[i], .nodeCount=0, .triCount=0};
    }
    thrd_pos = pos; thread_temp_nrm = nrm; thrd_uv = uv; thrd_verts = ov;
    thrd_ht = ht; thrd_ht_used = ht_used;
    thrd_remap_scratch = remap_scr; thrd_cache_scratch = cache_scr;
    ModelParseTask tasks[32]; u32 chunk = (mdlsCnt + threadCnt - 1) / threadCnt; OS_Thread th[32];
    for (int i=0;i<threadCnt;++i) tasks[i] = (ModelParseTask){i*chunk,(i+1)*chunk > mdlsCnt ? mdlsCnt : (i+1)*chunk,raw,i};
    if (threadCnt > 1) { // Each worker now parses its model range AND builds each model's BVH right after that model is parsed, all within the same thread -- overlapping BVH build cost with other threads' OBJ parsing instead of a separate serial post-pass.
        for (int i=0;i<threadCnt;++i) OS_ThreadCreate(&th[i],ModelParsingWorker,&tasks[i]);
        for (int i=0;i<threadCnt;++i) OS_ThreadJoin(&th[i]);
    } else { for (int t=0;t<threadCnt;++t) ModelParsingWorker(&tasks[t]); /*Single threaded fallback*/ }
    physPos = (float**)OS_Alloc(mdlsCnt * sizeof(float*));
    physTris = (u16**)OS_Alloc(mdlsCnt * sizeof(u16*));
    physVertCounts = (u32*)OS_Alloc(mdlsCnt * sizeof(u32));
    PhysGeomTask ptasks[32]; OS_Thread pth[32];
    for (int i=0;i<threadCnt;++i) ptasks[i] = (PhysGeomTask){i*chunk,(i+1)*chunk > mdlsCnt ? mdlsCnt : (i+1)*chunk,i};
    if (threadCnt > 1) {     for (int i=0;i<threadCnt;++i) OS_ThreadCreate(&pth[i],PhysGeomWorker,&ptasks[i]); } // Sneak the physics deduplication passes underneath the GPU upload ;)
    glGenBuffers(mdlsCnt,vbos); glGenBuffers(mdlsCnt,tbos); u32 tv=0,tt=0;
    for (int i=0; i<mdlsCnt; ++i) {
        if (!modelVertexCounts[i]) continue;
        tv += modelVertexCounts[i]; tt += modelTriangleCounts[i]; size_t vcz = (size_t)modelVertexCounts[i] * VRT_ATT_SZ, tcz = (size_t)modelTriangleCounts[i] * 3 * sizeof(u16);
        glBindBuffer(GL_ARRAY_BUFFER,vbos[i]); glBufferData(GL_ARRAY_BUFFER,vcz,NULL,GL_STATIC_DRAW);
        half* mpv = (half*)glMapBufferRange(GL_ARRAY_BUFFER,0,vcz,0x0002/*GL_MAP_WRITE_BIT*/|0x0008/*GL_MAP_INVALIDATE_BUFFER_BIT*/);
        u32 vc = modelVertexCounts[i]; const float *verts=vPos[i];
        for (u32 k = 0; k < vc; ++k) { __m256 v_in=(*(__m256_u const *)(&verts[k*8])); __m128i v_half=_mm256_cvtps_ph(v_in,0x00/*_MM_FROUND_TO_NEAREST_INT*/|0x08/*_MM_FROUND_NO_EXC*/); _mm_storeu_si128((__m128i*)&mpv[k*8],v_half); }
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,tbos[i]); glBufferData(GL_ELEMENT_ARRAY_BUFFER,tcz,NULL,GL_STATIC_DRAW); void* mpt = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER,0,tcz,0x0002/*GL_MAP_WRITE_BIT*/|0x0008/*GL_MAP_INVALIDATE_BUFFER_BIT*/); mcpy(mpt,modelTriangles[i],tcz); glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
        if (raw[i].data) OS_Free((void*)raw[i].data,raw[i].size);
    }
    glBindBuffer(GL_ARRAY_BUFFER,0); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
    if (threadCnt > 1) { for(int i=0;i<threadCnt;++i){OS_ThreadJoin(&pth[i]);}    } else { for(int t=0;t<threadCnt;++t){PhysGeomWorker(&ptasks[t]);} /*Single threaded fallback*/} // Regroup the physics deduplication passes after GPU upload, this does save about 0.18secs!
    for (u32 m = 0; m < mdlsCnt; ++m) {
        if (vPos[m]) { OS_Free(vPos[m],(size_t)modelVertexCounts[m] * CPU_VRT_SZ); OS_Free(modelTriangles[m],(size_t)modelTriangleCounts[m] * 3 * sizeof(u16)); vPos[m]=physPos[m]; modelTriangles[m]=physTris[m]; modelVertexCounts[m]=physVertCounts[m]; }
    }
    OS_Free(arena_base,arena); OS_Free(mp.entries,mp.count * sizeof(ModelData));
    DualLog(" vertices: %u, tris: %u, %f secs\n",tv,tt,get_time() - startModelTime);
    DebugRAM("After LoadModels");
}

const AnimationClip modelAnimationClips[MAX_ANIMS][MAX_ANIMCLIPS] = { // speed, frameStart, frameEnd, frameStartModelIndex, framerate
    [0]={[ANIM_IDLE_CLOSED]={1.0f,2,2,699,24},[ANIM_OPENING]={1.0f,2,11,699,24},[ANIM_IDLE_OPEN]={1.0f,11,11,708,24},[ANIM_CLOSING]={1.0f,12,21,709,24}}, // doorB (door2)
    [1]={[ANIM_IDLE_CLOSED]={1.0f,2,2,719,24},[ANIM_OPENING]={1.0f,2,12,719,24},[ANIM_IDLE_OPEN]={1.0f,12,12,729,24},[ANIM_CLOSING]={1.0f,14,24,731,24}}, // doorA (door1)
    [2]={[ANIM_IDLE]={1.0f,0,37,742,30},[ANIM_WALK]={1.0f,50,99,780,30},[ANIM_RUN]={1.1f,50,99,792,30},[ANIM_ATTACK1]={0.75f,111,136,830,30},[ANIM_PAIN]={0.5f,138,150,856,30},[ANIM_DYING]={0.75f,153,176,869,30}}, // npc_humanoid_mutant
    [3]={[ANIM_IDLE]={1.0f,1,207,893,24},[ANIM_ATTACK1]={1.0f,219,239,1100,24},[ANIM_WALK]={1.0f,252,308,1121,24},[ANIM_RUN]={1.0f,252,308,1121,24},[ANIM_PAIN]={1.0f,321,330,1177,24},[ANIM_PAIN2]={1.0f,331,344,1187,24},[ANIM_DYING]={1.0f,345,369,1201,24}}, // npc_cyborg_drone 
    [4]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1234,24},[ANIM_OPENING]={1.5f,2,44,1234,24},[ANIM_IDLE_OPEN]={1.0f,44,44,1276,24},[ANIM_CLOSING]={1.75f,46,96,1277,24}}, // doorD (door4, bulkhead 1)
    [5]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1328,24},[ANIM_OPENING]={1.0f,2,25,1328,24},[ANIM_IDLE_OPEN]={1.0f,25,25,1351,24},[ANIM_CLOSING]={1.0f,27,44,1352,24}}, // doorC (door3)
    [6]={[ANIM_IDLE_CLOSED]={1.0f,1,1,1370,24},[ANIM_OPENING]={1.2f,1,30,1370,24},[ANIM_IDLE_OPEN]={1.0f,30,30,1399,24},[ANIM_CLOSING]={1.2f,32,66,1400,24}}, // doorJ (xdoor1)
    [7]={[ANIM_IDLE_CLOSED]={1.0f,3,3,1435,24},[ANIM_OPENING]={1.2f,3,24,1435,24},[ANIM_IDLE_OPEN]={1.0f,26,26,1457,24},[ANIM_CLOSING]={1.2f,27,49,1458,24}}, // doorK (xdoor2)
    [8]={[ANIM_IDLE_CLOSED]={1.0f,3,3,1481,24},[ANIM_OPENING]={1.2f,3,27,1481,24},[ANIM_IDLE_OPEN]={1.0f,27,27,1505,24},[ANIM_CLOSING]={1.2f,30,51,1506,24}}, // doorL (door10)
    [9]={[ANIM_IDLE_CLOSED]={1.0f,3,3,1528,24},[ANIM_OPENING]={1.0f,3,15,1528,24},[ANIM_IDLE_OPEN]={1.0f,28,28,1541,24},[ANIM_CLOSING]={1.0f,28,39,1541,24}}, // doorE (door5)
    [10]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1553,24},[ANIM_OPENING]={1.0f,2,23,1553,24},[ANIM_IDLE_OPEN]={1.0f,23,23,1574,24},[ANIM_CLOSING]={1.0f,27,45,1541,24}}, // doorF (door6)
    [11]={[ANIM_IDLE_CLOSED]={1.0f,3,3,1594,24},[ANIM_OPENING]={1.0f,3,22,1594,24},[ANIM_IDLE_OPEN]={1.0f,22,22,1613,24},[ANIM_CLOSING]={1.0f,25,42,1614,24}}, // doorG (door7)
    [12]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1632,24},[ANIM_OPENING]={1.0f,2,25,1632,24},[ANIM_IDLE_OPEN]={1.0f,25,25,1655,24},[ANIM_CLOSING]={1.0f,27,49,1656,24}}, // doorH (door8)
    [13]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1679,24},[ANIM_OPENING]={1.0f,2,24,1679,24},[ANIM_IDLE_OPEN]={1.0f,24,24,1691,24},[ANIM_CLOSING]={1.0f,26,52,1692,24}}, // doorI (door9)
    [14]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1719,24},[ANIM_OPENING]={1.0f,2,20,1719,24},[ANIM_IDLE_OPEN]={1.0f,20,20,1737,24},[ANIM_CLOSING]={1.0f,22,41,1738,24}}, // door_elevator1
    [15]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1758,24},[ANIM_OPENING]={1.5f,2,21,1758,24},[ANIM_IDLE_OPEN]={1.0f,21,21,1777,24},[ANIM_CLOSING]={1.5f,23,41,1778,24}}, // door_elevator2
    [16]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1797,24},[ANIM_OPENING]={1.0f,2,22,1797,24},[ANIM_IDLE_OPEN]={1.0f,22,22,1817,24},[ANIM_CLOSING]={1.0f,24,43,1818,24}}, // door_elevator3
    [17]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1838,24},[ANIM_OPENING]={2.0f,2,32,1838,24},[ANIM_IDLE_OPEN]={1.0f,32,32,1868,24},[ANIM_CLOSING]={2.0f,34,62,1869,24}}, // door_elevator4
    [18]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1898,24},[ANIM_OPENING]={1.0f,2,21,1898,24},[ANIM_IDLE_OPEN]={1.0f,21,21,1917,24},[ANIM_CLOSING]={1.0f,23,41,1918,24}}, // door_secret2 (door_wall1)
    [19]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1937,24},[ANIM_OPENING]={1.0f,2,21,1937,24},[ANIM_IDLE_OPEN]={1.0f,21,21,1956,24},[ANIM_CLOSING]={1.0f,23,41,1957,24}}, // door_secret1 (door_wall2)
    [20]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1976,24},[ANIM_OPENING]={1.0f,2,17,1976,24},[ANIM_IDLE_OPEN]={1.0f,17,17,1991,24},[ANIM_CLOSING]={1.0f,19,33,1992,24}}, // door_secret3 (door_wall3)
    [21]={[ANIM_LOOP_ALL]={1.0f,1,47,2007,24}}, // chunk_eng2_6 (eng_wallpump)
    [22]={[ANIM_LOOP_ALL]={1.0f,1,50,2054,24}}, // flight_fanwall
    [23]={[ANIM_IDLE]={1.0f,3,3,2104,24},[ANIM_WALK]={1.0f,3,36,2104,24},[ANIM_ATTACK1]={1.0f,38,56,2138,24},[ANIM_ATTACK2]={1.0f,58,81,2156,24},[ANIM_ATTACK3]={1.0f,58,81,2156,24},[ANIM_RUN]={1.0f,3,36,2104,24},[ANIM_PAIN]={1.0f,84,96,2180,24},[ANIM_DYING]={1.0f,99,106,2192,24}}, // npc_bot_cortex_reaver
    [24]={[ANIM_IDLE]={1.0f,1,60,2200,24},[ANIM_ATTACK2]={1.0f,62,83,2260,24},[ANIM_ATTACK3]={1.0f,86,122,2282,24},[ANIM_RUN]={1.0f,143,182,2319,24},[ANIM_WALK]={1.0f,143,182,2319,24},[ANIM_PAIN]={1.0f,204,214,2359,24},[ANIM_PAIN2]={1.0f,216,227,2370,24},[ANIM_DYING]={1.0f,229,268,2382,24}}, // npc_cyborgassassin
    [25]={[ANIM_IDLE]={1.0f,1,155,2422,30},[ANIM_RUN]={1.0f,190,243,2577,30},[ANIM_WALK]={1.0f,190,243,2577,30},[ANIM_ATTACK2]={1.0f,265,289,2631,30},[ANIM_ATTACK1]={1.0f,291,332,2656,30},[ANIM_DYING]={1.0f,334,417,2698,30}}, // npc_cyborg_diego
    [26]={[ANIM_IDLE]={1.0f,1,68,2782,30},[ANIM_WALK]={1.0f,90,173,2850,30},[ANIM_RUN]={1.0f,90,173,2850,30},[ANIM_ATTACK2]={1.0f,194,214,2934,30},[ANIM_PAIN]={1.0f,216,233,2955,24},[ANIM_PAIN2]={1.0f,235,244,2973,24},[ANIM_DYING]={1.0f,319,386,2983,30},[ANIM_ATTACK1]={1.0f,401,422,3051,30},[ANIM_ATTACK3]={1.0f,424,450,3073,24}}, // npc_cyborg_elite
    [27]={[ANIM_IDLE]={1.0f,1,219,3100,24},[ANIM_WALK]={1.0f,240,286,3319,24},[ANIM_RUN]={1.0f,240,286,3319,24},[ANIM_PAIN]={1.0f,306,327,3366,24},[ANIM_ATTACK1]={1.0f,329,351,3388,24},[ANIM_ATTACK2]={1.0f,353,377,3411,24},[ANIM_DYING]={1.0f,380,402,3436,24},[ANIM_ATTACK3]={1.0f,416,438,3459,24}}, // npc_cyborg_enforcer
    [28]={[ANIM_IDLE]={1.0f,1,66,3482,24},[ANIM_ATTACK3]={1.0f,68,80,3548,24},[ANIM_ATTACK2]={1.0f,82,101,3561,24},[ANIM_PAIN]={1.0f,103,114,3581,24},[ANIM_WALK]={1.0f,122,157,3593,24},[ANIM_RUN]={1.0f,122,157,3593,24},[ANIM_DYING]={1.0f,169,217,3629,24}}, // npc_cyborgwarrior
    [29]={[ANIM_IDLE]={1.0f,3,3,3678,24},[ANIM_WALK]={1.0f,15,68,3679,24},[ANIM_RUN]={1.0f,15,68,3679,24},[ANIM_PAIN]={1.0f,82,92,3732,24},[ANIM_ATTACK2]={1.0f,94,117,3743,24},[ANIM_DYING]={1.0f,119,127,3767,24}}, // npc_execbot
    [30]={[ANIM_IDLE]={1.0f,1,39,3776,24},[ANIM_WALK]={2.0f,1,39,3776,24},[ANIM_RUN]={2.0f,1,39,3776,24},[ANIM_PAIN]={1.0f,41,73,3815,24},[ANIM_PAIN2]={0.5384f,75,95,3848,24},[ANIM_ATTACK2]={1.0f,97,121,3869,24},[ANIM_ATTACK3]={1.0f,106,121,3878,24}}, // npc_flierbot
    [31]={[ANIM_IDLE]={1.0f,1,73,3894,24},[ANIM_WALK]={1.0f,88,130,3967,24},[ANIM_RUN]={1.0f,88,130,3967,24},[ANIM_PAIN]={1.0f,144,159,4010,24},[ANIM_ATTACK1]={1.0f,162,183,4026,24},[ANIM_ATTACK2]={1.0f,186,209,4048,24},[ANIM_DYING]={1.0f,212,237,4072,24}}, // npc_gortiger
    [32]={[ANIM_IDLE]={1.0f,1,47,4098,24},[ANIM_WALK]={1.0f,49,87,4145,24},[ANIM_RUN]={1.0f,49,87,4145,24},[ANIM_PAIN]={1.0f,88,107,4184,24},[ANIM_PAIN2]={1.0f,109,125,4204,24},[ANIM_PAIN3]={1.0f,127,144,4221,24},[ANIM_ATTACK2]={1.0f,145,157,4239,24},[ANIM_DYING]={1.0f,160,239,4252,24}}, // npc_hopper
    [33]={[ANIM_IDLE]={1.0f,1,30,4332,24},[ANIM_WALK]={1.0f,1,30,4332,24},[ANIM_RUN]={1.0f,1,30,4332,24},[ANIM_PAIN]={1.0f,35,51,4362,24},[ANIM_ATTACK2]={1.0f,52,72,4379,24},[ANIM_DYING]={1.0f,79,103,4400,24}}, // npc_invisomut
    [34]={[ANIM_IDLE]={1.0f,2,2,4425,24},[ANIM_ATTACK1]={2.0f,2,71,4425,24},[ANIM_WALK]={2.0f,80,107,4495,24},[ANIM_RUN]={2.0f,80,107,4495,24},[ANIM_DYING]={1.0f,117,150,4523,24}}, // npc_maintenancebot
    [35]={[ANIM_IDLE]={2.5f,1,59,4557,24},[ANIM_WALK]={2.5f,1,59,4557,24},[ANIM_RUN]={2.5f,1,59,4557,24},[ANIM_ATTACK1]={1.0f,61,79,4616,24},[ANIM_PAIN]={1.0f,81,93,4635,24},[ANIM_DYING]={1.0f,94,119,4648,24}}, // npc_mutant_avian
    [36]={[ANIM_IDLE]={1.0f,1,78,4674,24},[ANIM_WALK]={1.0f,90,129,4752,24},[ANIM_RUN]={1.0f,90,129,4752,24},[ANIM_ATTACK2]={1.0f,142,185,4792,24},[ANIM_DYING]={1.0f,188,225,4836,24},[ANIM_PAIN]={1.0f,227,235,4874,24}}, // npc_plantmutant
    [37]={[ANIM_IDLE]={1.0f,1,42,4883,24},[ANIM_WALK]={2.0f,58,85,4925,24},[ANIM_RUN]={2.0f,58,85,4925,24},[ANIM_ATTACK1]={1.0f,102,123,4953,24},[ANIM_ATTACK2]={1.0f,126,148,4975,24}}, // npc_repairbot
    [38]={[ANIM_IDLE]={1.0f,1,54,4998,24},[ANIM_WALK]={1.0f,1,54,4998,24},[ANIM_RUN]={1.0f,58,95,5052,24}}, // npc_sec1bot
    [39]={[ANIM_IDLE]={0.333f,1,17,5090,24},[ANIM_WALK]={0.333f,19,38,5107,24},[ANIM_RUN]={0.333f,19,38,5107,24},[ANIM_ATTACK2]={0.25f,39,48,5127,24},[ANIM_ATTACK3]={1.0f,49,56,5137,24},[ANIM_PAIN]={1.0f,58,63,5145,24},[ANIM_DYING]={0.2f,65,66,5151,24}}, // npc_sec2bot
    [40]={[ANIM_IDLE]={0.18f,1,9,5153,24},[ANIM_WALK]={0.333f,1,9,5153,24},[ANIM_RUN]={0.333f,1,9,5153,24},[ANIM_ATTACK1]={0.5f,18,28,5162,24},[ANIM_PAIN]={0.333f,54,63,5173,24},[ANIM_DYING]={0.333f,77,85,5183,24}}, // npc_servbot
    [41]={[ANIM_IDLE]={1.0f,1,66,5192,24},[ANIM_WALK]={2.0f,79,132,5258,24},[ANIM_RUN]={2.5f,79,132,5258,24},[ANIM_PAIN]={1.0f,145,157,5312,24},[ANIM_ATTACK2]={1.0f,159,181,5325,24},[ANIM_DYING]={1.0f,183,221,5348,24}}, // npc_virusmutant
    [42]={[ANIM_IDLE]={1.0f,1,121,5387,24},[ANIM_DYING]={1.0f,121,157,5507,24}}, // npc_zerogmut
    [43]={[ANIM_IDLE_CLOSED]={1.0f,1,1,5544,24},[ANIM_OPENING]={1.2f,2,21,5545,24},[ANIM_IDLE_OPEN]={1.0f,21,21,5564,24}}, // puzzlepanel1
    [44]={[ANIM_IDLE_CLOSED]={1.0f,0,0,5565,24},[ANIM_OPENING]={1.2f,1,17,5566,24},[ANIM_IDLE_OPEN]={1.0f,17,17,5582,24},[ANIM_INSTALL]={1.0f,19,30,5584,24},[ANIM_INSTALLED]={1.0f,18,18,5583,24}}, // puzzlepanel2
    [45]={[ANIM_IDLE_CLOSED]={1.0f,0,0,5596,24},[ANIM_OPENING]={1.2f,1,17,5597,24},[ANIM_IDLE_OPEN]={1.0f,17,17,5613,24},[ANIM_INSTALLED]={1.0f,18,18,5614,24}}, // puzzlepanel3
    [46]={[ANIM_LOOP_ALL]={1.0f,1,100,5615,24}}, // sparkingwire
    [47]={[ANIM_INACTIVE]={1.0f,2,2,5715,24},[ANIM_ACTIVATE]={1.2f,2,4,5715,24},[ANIM_ACTIVATED]={1.0f,4,4,5717,24},[ANIM_DEACTIVATE]={1.0f,5,6,5718,24}}, // switch4
    [48]={[ANIM_INACTIVE]={1.0f,2,2,5720,24},[ANIM_ACTIVATE]={1.2f,2,6,5720,24},[ANIM_ACTIVATED]={1.0f,6,6,5724,24},[ANIM_DEACTIVATE]={1.0f,8,10,5725,24}}, // switch5
    [49]={[ANIM_IDLE]={1.0f,1,1,5728,24},[ANIM_ATTACK_MISS]={1.0f,1,13,5728,24},[ANIM_ATTACK_HIT]={1.0f,18,24,5741,24}}, // v_pipe
    [50]={[ANIM_IDLE]={1.0f,1,1,5748,24},[ANIM_ATTACK_MISS]={0.5f,4,22,5749,24},[ANIM_ATTACK_HIT]={1.0f,4,22,5749,24}}, // v_rapier
    [51]={[ANIM_IDLE]={1.0f,1,65,5768,24},[ANIM_WALK]={1.0f,75,98,5833,24},[ANIM_RUN]={1.0f,75,98,5833,24},[ANIM_ATTACK2]={1.0f,109,126,5857,24},[ANIM_ATTACK1]={1.0f,128,142,5875,24},[ANIM_PAIN]={1.0f,144,159,5890,24},[ANIM_PAIN2]={1.0f,161,174,5906,24},[ANIM_DYING]={1.0f,176,243,5920,24}}, // npc_mutant_cyborg
};

void PortalCulling(); bool ToggleDoorPortal(u8,u16,u16);
void UpdateAnims(void) {
    if (World.paused || World.menuActive) return;
    static double lastPauseTime = 0.0; if (lastPauseTime == 0.0) lastPauseTime = World.pauseRelativeTime;
    double animDT = World.pauseRelativeTime - lastPauseTime; lastPauseTime = World.pauseRelativeTime;
    if (animDT > 0.1) animDT = 0.1; if (animDT <= 0.0) return;
    bool portalsNeedUpdated = false;
    for (u16 i = INSTS_1ST_IDX; i < INSTANCE_COUNT; ++i) {
        Entity* e = &World.instances[i];
        if (e->modelIndex >= MAX_MDLS || !(e->entflags & EF_ACTIVE) || e->animationNum >= MAX_ANIMS || e->numclips == 0 || e->clip >= e->numclips) continue;
        AnimationClip* clip = (AnimationClip*)&modelAnimationClips[e->animationNum][e->clip]; if (clip->framerate <= 0 || clip->speed <= 0) continue;
        e->currentFrameFinished += animDT * clip->speed;
        double timePerFrame = 1.0 / (double)clip->framerate;
        if (e->currentFrameFinished >= timePerFrame) {
            u32 framesToAdvance = (u32)(e->currentFrameFinished / timePerFrame), frameCount = clip->frameEnd - clip->frameStart + 1;
            e->currentFrameFinished -= (double)framesToAdvance * timePerFrame;
            e->frame = (frameCount <= 1) ? clip->frameStart : clip->frameStart + ((e->frame - clip->frameStart + framesToAdvance) % frameCount);
            e->modelIndex = clip->frameStartModelIndex + (e->frame - clip->frameStart);
            if (IdxIsPortalBlockingDoor(e->index) && ToggleDoorPortal(e->portalIndex, i, modelAnimationClips[e->animationNum][ANIM_IDLE_CLOSED].frameStartModelIndex)) portalsNeedUpdated = true;
        }
    }
    if (portalsNeedUpdated) PortalCulling();
}

void ChangeAnim(Entity* e, u8 clip) { e->clip = clip; e->currentFrameFinished = 0.0; AnimationClip* c = (AnimationClip*)&modelAnimationClips[e->animationNum][e->clip]; e->frame = c->frameStart; } // TODO actually use this!}
void GenerateConvexAdjacencyLists() { // Hill Climb Racer Adjacency List
    double start_time = get_time();
    cvxAdjOffsets=OS_Alloc(MAX_UNIQUE_CVX_MESHES * sizeof(u32*)); cvxAdjLists=OS_Alloc(MAX_UNIQUE_CVX_MESHES * sizeof(u16*));
    for (u32 lev = 0; lev < MAX_LEVELS; ++lev) { // 1. Find unique convex mesh indices across all levels
        for (u32 i = 0; i < INSTANCE_COUNT; ++i) {
            if (World.levelCollider[lev][i] == COLTYPE_CVX) { //DualLog("Checking level %u, instance %u with constindex %u for convex mesh uniques, colMeshIndex: %u, current uniqueCvxMeshCount: %u\n",lev,i,World.levelInstances[lev][i].index,World.levelInstances[lev][i].colMeshIndex,uniqueCvxMeshCount);
                u16 colMeshIdx = World.levelInstances[lev][i].colMeshIndex;// if (colMeshIdx == U16_MAX) continue;
                if (colMeshIdx > MAX_MDLS) DualLogWarn("Improper convex mesh colMeshIndex on level %u, instance %u with constindex %u for convex mesh uniques, colMeshIndex: %u\n",lev,i,World.levelInstances[lev][i].index,World.levelInstances[lev][i].colMeshIndex);
                bool isUnique=true; u32 foundIdx=U16_MAX;
                for (u32 u = 0; u < uniqueCvxMeshCount; ++u) { if (uniqueCvxMeshIndices[u] == colMeshIdx) { isUnique = false; foundIdx = u; break; } }
                if (isUnique) { //DualLog("Processing convex mesh colMeshIndex %u on object %u[%u]\n",colMeshIdx,lev,i);
                    if (uniqueCvxMeshCount >= MAX_UNIQUE_CVX_MESHES) { DualLogWarn("Warning: Exceeded MAX_UNIQUE_CVX_MESHES! Some convex meshes will use slow linear support.\n"); World.levelInstances[lev][i].adjacencyIdx = U16_MAX; continue; }
                    uniqueCvxMeshIndices[uniqueCvxMeshCount] = colMeshIdx; World.levelInstances[lev][i].adjacencyIdx = (u16)uniqueCvxMeshCount; uniqueCvxMeshCount++; //DualLog("Incremented uniqueCvxMeshCount to %u\n",uniqueCvxMeshCount);
                } else { World.levelInstances[lev][i].adjacencyIdx = (u16)foundIdx; }
            } else { World.levelInstances[lev][i].adjacencyIdx = U16_MAX; }
        }
    }
    for (u32 u = 0; u < uniqueCvxMeshCount; ++u) { // 2. Generate edge adjacency list for each unique mesh
        u16 m = uniqueCvxMeshIndices[u]; if (m >= MAX_MDLS) { continue;}
        u32 vCount = physVertCounts[m], tCount = modelTriangleCounts[m];
        if (!vCount || !tCount || !physPos[m] || !physTris[m]) continue;
        u32 edgeCount = 0; u32* tempEdges = OS_Alloc(tCount * 3 * sizeof(u32));
        for (u32 t = 0; t < tCount; ++t) {
            u16 i0=physTris[m][t*3+0], i1=physTris[m][t*3+1], i2=physTris[m][t*3+2];
            tempEdges[edgeCount++]=((u32)vmin(i0,i1) << 16) | vmax(i0,i1);
            tempEdges[edgeCount++]=((u32)vmin(i1,i2) << 16) | vmax(i1,i2);
            tempEdges[edgeCount++]=((u32)vmin(i2,i0) << 16) | vmax(i2,i0);
        }
        qsort_new(tempEdges,edgeCount,sizeof(u32),EdgeCompare);
        u32 uniqueEdgeCount=0; 
        u32* degree=OS_Alloc(vCount * sizeof(u32)); 
        for (u32 i = 0; i < edgeCount; ++i) { if (i == 0 || tempEdges[i] != tempEdges[i-1]) { tempEdges[uniqueEdgeCount++]=tempEdges[i]; u16 a=(u16)(tempEdges[i] >> 16); u16 b=(u16)(tempEdges[i] & 0xFFFF); degree[a]++; degree[b]++; } }
        u32* offsets=OS_Alloc((vCount + 1) * sizeof(u32)); offsets[0]=0; for(u32 i=0;i<vCount;++i){offsets[i+1]=offsets[i] + degree[i];}
        u16* adjList = OS_Alloc(uniqueEdgeCount * 2 * sizeof(u16));
        u32* writePos = OS_Alloc(vCount * sizeof(u32));
        mcpy(writePos, offsets, vCount * sizeof(u32));
        for (u32 i=0;i<uniqueEdgeCount;++i) { u16 a=(u16)(tempEdges[i] >> 16); u16 b=(u16)(tempEdges[i] & 0xFFFF); adjList[writePos[a]++]=b; adjList[writePos[b]++]=a; }
        cvxAdjOffsets[u]=offsets; cvxAdjLists[u]=adjList; lastCvxSupport[u]=0;
        OS_Free(tempEdges,tCount * 3 * sizeof(u32)); OS_Free(degree,vCount * sizeof(u32)); OS_Free(writePos,vCount * sizeof(u32));
    }
    DualLog("Generating edge adjacency lists for %u convex meshes...took %f secs\n",uniqueCvxMeshCount,get_time() - start_time);
}
