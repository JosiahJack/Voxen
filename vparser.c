// vparser.c - Voxen Parser Tool, use to regenerate -models, -text, or -all binary blob files for immediate load direct into memory with matching layout expected by Voxen engine
#include "types.h"
#include "parse.h"
#include "matvecquat.h"
#include "lib.h"
#include "os.h"
#include "stbtt.h"
i32 threadCnt=0;
int numPackedGlyphs=0,numPackedGlyphsStopD=0;
stbtt_packedchar fontPackedChar[MAX_GLYPHS],fontPackedCharStopD[MAX_GLYPHS];
float fixedNumberAdvanceWidth=0.0f,fixedNumberAdvanceWidthStopD=0.0f;
static const char* fallbackFontPaths[]={"./Fonts/FreeSerifBold.ttf","./Fonts/cambriab.ttf","./Fonts/NotoSansCJK-Bold.ttc"}, *fontPaths[]={"./Fonts/SystemShockText.ttf","./Fonts/StopD.ttf"};
static stbtt_fontinfo fontInfo[5]; static u8 *fontData[5];
typedef struct{char*path;u8*data;size_t size;stbtt_fontinfo info;}LoadedFont;
LoadedFont fallbackFonts[3];
GlyphRange fontRanges[]     ={{0x0020,0x7E - 0x20 + 1,0},{0x00A0,0xFF - 0xA0 + 1,95},{0x0400,0x04FF - 0x0400 + 1,95+96},{0x3040,0x30FF - 0x3040 + 1,95+96+256}};
GlyphRange fontRangesStopD[]={{0x0020,0x7E - 0x20 + 1,0},{0x00A0,0xFF - 0xA0 + 1,95},{0x0400,0x04FF - 0x0400 + 1,95+96},{0x3040,0x30FF - 0x3040 + 1,95+96+256}};
i32 numFontRanges=sizeof(fontRanges)/sizeof(fontRanges[0]);
i32 CodepointToPackedIndex(i32 cp,int fontID){ if(cp<32){cp=32;} if(cp>=447){cp=446;} const GlyphRange*ranges=(fontID==FONT_STOPD)?fontRangesStopD:fontRanges; i32 total=(fontID==FONT_STOPD)?numPackedGlyphsStopD:numPackedGlyphs; for(i32 i=0;i<numFontRanges;i++){if(cp>=ranges[i].first&&cp<ranges[i].first+ranges[i].count){i32 idx=ranges[i].startIndex+vmax((cp-ranges[i].first),0);if(idx<total){return idx;}}} return 0; }
LoadedFont LoadFallbackFont(const char*path,int fii,int ci){
    FHandle fd;int fsz;fontData[fii]=OS_OpenAndAllocateFileBufferReadonly(path,&fd,&fsz);
    int off=stbtt_GetFontOffsetForIndex(fontData[fii],ci);if(off<0){PrintLog("Invalid collection index %d for font %s\n",ci,path);OS_Exit(1);}
    if(!stbtt_InitFont_internal(&fontInfo[fii],fontData[fii],off)){PrintLog("Failed to init font at index %d in %s\n",ci,path);OS_Exit(1);}
    return (LoadedFont){(char*)path,fontData[fii],fsz,fontInfo[fii]};
}

int GetGlyphAndFont(u32 cp,stbtt_fontinfo**outFont,u8 fontID){ int g=stbtt_FindGlyphIndex(fontID==FONT_STOPD?&fontInfo[1]:&fontInfo[0],cp);if(g){*outFont=fontID==FONT_STOPD?&fontInfo[1]:&fontInfo[0];return g;} for(int i=0;i<3;i++){g=stbtt_FindGlyphIndex(&fallbackFonts[i].info,cp);if(g){*outFont=&fallbackFonts[i].info;return g;}} return 0; }
void GenerateTextBin() {
    double t0=get_time(); PrintLog("Generating text.bin...");
    ttAllocs = OS_Alloc(4674 * sizeof(TAlloc));
    FHandle fd1,fd2;int sz1,sz2;
    fontData[0]=OS_OpenAndAllocateFileBufferReadonly(fontPaths[0],&fd1,&sz1);
    fontData[1]=OS_OpenAndAllocateFileBufferReadonly(fontPaths[1],&fd2,&sz2);
    if(!stbtt_InitFont_internal(&fontInfo[0],fontData[0],0)){PrintLog("%s font init failed\n",fontPaths[0]);OS_Exit(1);}
    if(!stbtt_InitFont_internal(&fontInfo[1],fontData[1],0)){PrintLog("%s font init failed\n",fontPaths[1]);OS_Exit(1);}
    fallbackFonts[0]=LoadFallbackFont(fallbackFontPaths[0],2,0);
    fallbackFonts[1]=LoadFallbackFont(fallbackFontPaths[1],3,0);
    fallbackFonts[2]=LoadFallbackFont(fallbackFontPaths[2],4,2);
    u8*bmp=OS_Alloc(FONT_ATLAS_SIZE*FONT_ATLAS_SIZE); // Primary atlas
    stbtt_pack_context pc;stbtt_PackBegin(&pc,bmp,FONT_ATLAS_SIZE,FONT_ATLAS_SIZE,0,16,NULL);pc.h_oversample=3;pc.v_oversample=3;pc.skip_missing=1;numPackedGlyphs=0;
    for(int r=0;r<numFontRanges;++r){fontRanges[r].startIndex=numPackedGlyphs;
        for(int i=0;i<fontRanges[r].count;++i){if(numPackedGlyphs>=MAX_GLYPHS)break;u32 cp=fontRanges[r].first+i;stbtt_fontinfo*font=&fontInfo[0];u8*data=fontData[0];
            int g=stbtt_FindGlyphIndex(font,cp);if(!g){g=GetGlyphAndFont(cp,&font,FONT_NORMAL);if(!g)continue;data=(font==&fontInfo[0])?fontData[0]:((LoadedFont*)((char*)font-__builtin_offsetof(LoadedFont,info)))->data;}
            float h=20.0f;if(font!=&fontInfo[0])h*=1.2f;FPackRange range={h,cp,NULL,1,&fontPackedChar[numPackedGlyphs],0,0};stbtt_PackFontRanges(&pc,data,0,&range,1);
            int idx=numPackedGlyphs++;if(cp>='0'&&cp<='9')fixedNumberAdvanceWidth=vmax(fixedNumberAdvanceWidth,fontPackedChar[idx].xadvance);
        }
    }
    ttfree(pc.pack_info);
    u8*bmpStopD=OS_Alloc(FONT_ATLAS_SIZE*FONT_ATLAS_SIZE); // Secondary atlas
    stbtt_pack_context pc2;stbtt_PackBegin(&pc2,bmpStopD,FONT_ATLAS_SIZE,FONT_ATLAS_SIZE,0,16,NULL);pc2.h_oversample=3;pc2.v_oversample=3;pc2.skip_missing=1;numPackedGlyphsStopD=0;
    for(int r=0;r<numFontRanges;++r){fontRangesStopD[r].startIndex=numPackedGlyphsStopD;
        for(int i=0;i<fontRangesStopD[r].count;++i){if(numPackedGlyphsStopD>=MAX_GLYPHS)break;u32 cp=fontRangesStopD[r].first+i;stbtt_fontinfo*font=&fontInfo[1];u8*data=fontData[1];
            int g=stbtt_FindGlyphIndex(font,cp);if(!g){g=GetGlyphAndFont(cp,&font,FONT_STOPD);if(!g)continue;data=(font==&fontInfo[0])?fontData[0]:((LoadedFont*)((char*)font-__builtin_offsetof(LoadedFont,info)))->data;}
            float h=54.0f;if(font!=&fontInfo[1])h*=1.2f;FPackRange range={h,cp,NULL,1,&fontPackedCharStopD[numPackedGlyphsStopD],0,0};stbtt_PackFontRanges(&pc2,data,0,&range,1);
            int idx=numPackedGlyphsStopD++;if(cp>='0'&&cp<='9')fixedNumberAdvanceWidthStopD=vmax(fixedNumberAdvanceWidthStopD,fontPackedCharStopD[idx].xadvance);
        }
    }
    ttfree(pc2.pack_info);
    FHandle outFile = OS_OpenWriteonly("text.bin");
    if (outFile == (FHandle)-1) { PrintLog("Failed to open text.bin for writing.\n"); OS_Exit(1); }
    TextBinHeader header = {
        .magic = TEXT_BIN_MAGIC,
        .atlasSize = FONT_ATLAS_SIZE,
        .numPackedGlyphs = numPackedGlyphs,
        .numPackedGlyphsStopD = numPackedGlyphsStopD,
        .fixedNumberAdvanceWidth = fixedNumberAdvanceWidth,
        .fixedNumberAdvanceWidthStopD = fixedNumberAdvanceWidthStopD,
        .packedCharBytes = numPackedGlyphs * sizeof(stbtt_packedchar),
        .packedCharStopDBytes = numPackedGlyphsStopD * sizeof(stbtt_packedchar),
        .atlasBytes = FONT_ATLAS_SIZE * FONT_ATLAS_SIZE
    };
    OS_Write(outFile, &header, sizeof(TextBinHeader), "text.bin");
    OS_Write(outFile, fontPackedChar, header.packedCharBytes, "text.bin");
    OS_Write(outFile, fontPackedCharStopD, header.packedCharStopDBytes, "text.bin");
    OS_Write(outFile, bmp, header.atlasBytes, "text.bin");
    OS_Write(outFile, bmpStopD, header.atlasBytes, "text.bin");
    OS_Close(outFile);
    OS_Free(bmp, FONT_ATLAS_SIZE * FONT_ATLAS_SIZE);
    OS_Free(bmpStopD, FONT_ATLAS_SIZE * FONT_ATLAS_SIZE);
    OS_Free(fontData[0], sz1);
    OS_Free(fontData[1], sz2);
    OS_Free(fontData[2], fallbackFonts[0].size);
    OS_Free(fontData[3], fallbackFonts[1].size);
    OS_Free(fontData[4], fallbackFonts[2].size);
    OS_Free(ttAllocs, 4674 * sizeof(TAlloc));
    PrintLog(" finished in %f s\n", get_time() - t0);
}

u16 mdlsCnt;
static float **vPos; u16** modelTriangles; u32 modelVertexCounts[MAX_MDLS]; u16 modelTriangleCounts[MAX_MDLS]; float modelBounds[MAX_MDLS];
float** physPos; u16** physTris; u32* physVertCounts; u32** cvxAdjOffsets; u16** cvxAdjLists; u32 cvxAdjOffsetCounts[MAX_MDLS],cvxAdjListCounts[MAX_MDLS];
BvhNode** modelBVHNodes; u16** modelBVHTriOrder; u32 modelBVHNodeCounts[MAX_MDLS], modelBVHTriOrderCounts[MAX_MDLS];
static float **thrd_pos, **thread_temp_nrm, **thrd_uv, **thrd_verts; static u32 **thrd_ht, **thrd_ht_used, **thrd_remap_scratch; static u8** thrd_cache_scratch;
typedef struct { const char* data; const char* name; int size; } RawOBJ;
typedef struct { u16 index; bool animated; u8 animationNum; char path[128]; } ModelData;
typedef struct { ModelData* entries; u32 count; u32 capacity; } ModelDataParser;
typedef struct { u32 start,end; int tid; } PhysGeomTask;
typedef struct { BvhNode* nodes; u8* triOctants; u16 *triOrder, *triScratch,*initialTris; u32 nodeCount,triCount; } BvhBuildCtx;
static BvhBuildCtx thrd_bvh_ctx[32];
#define _mm_min_ps(A, B) ((__m128)__builtin_ia32_minps((__v4sf)(A), (__v4sf)(B)))
#define _mm_max_ps(A, B) ((__m128)__builtin_ia32_maxps((__v4sf)(A), (__v4sf)(B)))
#define _mm256_cvtps_ph(A, imm) ((__m128i)__builtin_ia32_vcvtps2ph256((__v8sf)(__m256)(A), (int)(imm)))
INLINE float fast_atof(const char** p) { const char* c=*p; while (*c == ' ' || *c == '\t') {c++;} float s=1.0f; if(*c == '-'){s=-1.0f; c++;} float v=0.0f; while (*c >= '0' && *c <= '9') { v=v * 10.0f + (*c - '0'); c++; } if (*c == '.') { c++; float sub=0.1f; while (*c >= '0' && *c <= '9') { v += (*c - '0') * sub; sub*=0.1f; c++; } } *p=c; return s * v; }
INLINE i32 fast_atoi(const char** p) { const char* c = *p; while (*c == ' ' || *c == '\t') {c++;} i32 s=1; if(*c == '-'){s=-1; c++;} i32 v = 0; while (*c >= '0' && *c <= '9') { v = v * 10 + (*c - '0'); c++; } *p = c; return v * s; }
typedef struct {u32 idx,key;} TriSort;
void OptimizeVertexCache(u16* idx, u32 ic, u32 vc, u8* scratch) {
    if (ic < 3 || !vc) return;
    u32 tc = ic / 3;
    TriSort* t = (TriSort*)scratch;
    TriSort* t_tmp = (TriSort*)(scratch + (tc * sizeof(TriSort)));
    u16* n = (u16*)(scratch + (tc * sizeof(TriSort) * 2));
    for (u32 i = 0; i < tc; ++i) { u16* p = idx + i * 3; u32 m = p[0] < p[1] ? p[0] : p[1]; m = m < p[2] ? m : p[2]; t[i].idx = i; t[i].key = (u16)m; }
    if (tc >= 2) {
        u32 b0[256]={0}, b1[256]={0};
        for (u32 i=0;i<tc;++i) {u16 key = t[i].key; b0[key & 0xFF]++; b1[(key >> 8) & 0xFF]++;}
        u32 sum0=0, sum1=0;
        for (u32 i=0;i<256;++i) { u32 t0 = b0[i]; u32 t1 = b1[i]; b0[i] = sum0; b1[i] = sum1; sum0 += t0; sum1 += t1; }
        for (u32 i=0;i<tc;++i) {u32 radix0 = t[i].key & 0xFF; u32 dest = b0[radix0]++; t_tmp[dest] = t[i];}
        for (u32 i=0;i<tc;++i) { u32 radix1 = (t_tmp[i].key >> 8) & 0xFF; u32 dest = b1[radix1]++; t[dest] = t_tmp[i]; }
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
                if (unlikely(ec + 3 > MAX_OUTPUT_VERTS)) {PrintLog("vert overflow!\n"); return false;}
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
        i32 childIdx = BvhBuildOctree(ctx,m,pos,tris,triIdxArray + octantStarts[o],octantCounts[o],depth + 1);
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
    if (triCount == 0 || triCount > BVH_MAX_TRIS_PER_MDL) { PrintLog("Too many verts on model %u!  Could not build a BVH!\n",m); return;}
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
    if (!maxidx) { PrintLog("No entries in %s\n", fn); OS_Free(buf,sz); return true; }
    if (maxidx >= maxSz) { PrintLog("Index too large in %s\n", fn); OS_Free(buf,sz); return true; }
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
    float* exactPos = (float*)OS_Alloc((size_t)weldedCount * 3 * sizeof(float));
    mcpy(exactPos,weldedPos, (size_t)weldedCount * 3 * sizeof(float));
    OS_Free(weldedPos,(size_t)vc * 3 * sizeof(float));
    u16* weldedTris = (u16*)OS_Alloc((size_t)tc * 3 * sizeof(u16));
    const u16* srcTris = modelTriangles[m];
    for (u32 i = 0; i < tc * 3; ++i) weldedTris[i] = remap[srcTris[i]];
    physPos[m] = exactPos; physTris[m] = weldedTris; physVertCounts[m] = weldedCount;
}

int EdgeCompare(const void* a, const void* b) { u32 ea = *(const u32*)a, eb = *(const u32*)b; return (ea > eb) - (ea < eb); }
static void GenerateModelAdjacency(u16 m) {
    cvxAdjOffsets[m] = NULL; cvxAdjLists[m] = NULL; cvxAdjOffsetCounts[m] = 0; cvxAdjListCounts[m] = 0;
    u32 vCount = physVertCounts[m], tCount = modelTriangleCounts[m]; if (!vCount || !tCount || !physPos[m] || !physTris[m]) return;
    u32 edgeCount = 0; u32* tempEdges = OS_Alloc(tCount * 3 * sizeof(u32));
    for (u32 t = 0; t < tCount; ++t) {
        u16 i0 = physTris[m][t*3+0], i1 = physTris[m][t*3+1], i2 = physTris[m][t*3+2];
        tempEdges[edgeCount++] = ((u32)vmin(i0,i1) << 16) | vmax(i0,i1);
        tempEdges[edgeCount++] = ((u32)vmin(i1,i2) << 16) | vmax(i1,i2);
        tempEdges[edgeCount++] = ((u32)vmin(i2,i0) << 16) | vmax(i2,i0);
    }
    qsort_new(tempEdges, edgeCount, sizeof(u32), EdgeCompare);
    u32 uniqueEdgeCount = 0;
    u32* degree = OS_Alloc(vCount * sizeof(u32));
    mset(degree, 0, vCount * sizeof(u32));
    for (u32 i = 0; i < edgeCount; ++i) {
        if (i == 0 || tempEdges[i] != tempEdges[i-1]) { tempEdges[uniqueEdgeCount++] = tempEdges[i]; u16 a = (u16)(tempEdges[i] >> 16); u16 b = (u16)(tempEdges[i] & 0xFFFF); degree[a]++; degree[b]++; }
    }
    u32* offsets = OS_Alloc((vCount + 1) * sizeof(u32)); offsets[0] = 0;
    for (u32 i = 0; i < vCount; ++i) offsets[i+1] = offsets[i] + degree[i];
    u16* adjList = OS_Alloc(uniqueEdgeCount * 2 * sizeof(u16));
    u32* writePos = OS_Alloc(vCount * sizeof(u32));
    mcpy(writePos, offsets, vCount * sizeof(u32));
    for (u32 i = 0; i < uniqueEdgeCount; ++i) { u16 a = (u16)(tempEdges[i] >> 16); u16 b = (u16)(tempEdges[i] & 0xFFFF); adjList[writePos[a]++] = b; adjList[writePos[b]++] = a; }
    cvxAdjOffsets[m] = offsets;  cvxAdjLists[m] = adjList;
    cvxAdjOffsetCounts[m] = vCount + 1;
    cvxAdjListCounts[m]  = uniqueEdgeCount * 2;
    OS_Free(tempEdges, tCount * 3 * sizeof(u32));
    OS_Free(degree, vCount * sizeof(u32));
    OS_Free(writePos, vCount * sizeof(u32));
}

static void* PhysGeomWorker(void* a) {
    PhysGeomTask* t=a; BvhBuildCtx* bvhCtx=&thrd_bvh_ctx[t->tid]; u32* ht = thrd_ht[t->tid]; u32* u=thrd_ht_used[t->tid]; u16* sc=(u16*)thrd_remap_scratch[t->tid];
    for (u32 m = t->start; m < t->end; ++m) {
        if(m >= mdlsCnt || !modelVertexCounts[m] || !modelTriangleCounts[m]){physPos[m]=NULL; physTris[m]=NULL; physVertCounts[m]=0; continue;}
        WeldModelPositions((u16)m,ht,u,sc); BuildModelBVH(bvhCtx,(u16)m); GenerateModelAdjacency((u16)m);
    }  return NULL;
}

static half* ConvertVertsToHalf(const float* verts, u32 vc) {
    half* out = (half*)OS_Alloc((size_t)vc * VRT_ATT_SZ);
    for (u32 k = 0; k < vc; ++k) { __m256 v_in = (*(__m256_u const *)(&verts[k*8])); __m128i v_half = _mm256_cvtps_ph(v_in,0x00/*_MM_FROUND_TO_NEAREST_INT*/|0x08/*_MM_FROUND_NO_EXC*/); _mm_storeu_si128((__m128i*)&out[k*8],v_half); }
    return out;
}

static u32 AlignUp4(u32 x) { return (x + 3u) & ~3u; }
static void WriteModelsBin(const char* outPath) {
    size_t payloadSize = 0;
    for (u32 m = 0; m < mdlsCnt; ++m) {
        if (!modelVertexCounts[m] || !modelTriangleCounts[m]) continue;
        payloadSize += AlignUp4(modelVertexCounts[m] * VRT_ATT_SZ);
        payloadSize += AlignUp4(modelTriangleCounts[m] * 3 * sizeof(u16));      // render tris
        payloadSize += AlignUp4(physVertCounts[m] * 3 * sizeof(float));
        payloadSize += AlignUp4(modelTriangleCounts[m] * 3 * sizeof(u16));      // welded phys tris - weld doesn't change tri count, only re-indexes
        payloadSize += AlignUp4(modelBVHNodeCounts[m] * sizeof(BvhNode));
        payloadSize += AlignUp4(modelBVHTriOrderCounts[m] * sizeof(u16));
        payloadSize += AlignUp4(cvxAdjOffsetCounts[m] * sizeof(u32));
        payloadSize += AlignUp4(cvxAdjListCounts[m]  * sizeof(u16));
    }
    size_t dirSize = (size_t)mdlsCnt * sizeof(ModelDirEntry);
    size_t totalSize = dirSize + payloadSize;
    u8* blob = (u8*)OS_Alloc(totalSize);
    ModelDirEntry* dir = (ModelDirEntry*)blob;
    mset(dir, 0, dirSize); // zeroed entry (vertCount==0) marks an unused model slot for the loader
    u32 cursor = (u32)dirSize;
    for (u32 m = 0; m < mdlsCnt; ++m) {
        ModelDirEntry* e = &dir[m];
        e->bound = modelBounds[m];
        if (!modelVertexCounts[m] || !modelTriangleCounts[m]) continue;
        half* halfVerts = ConvertVertsToHalf(vPos[m], modelVertexCounts[m]);
        size_t vRawSz = (size_t)modelVertexCounts[m] * VRT_ATT_SZ, vSz = AlignUp4((u32)vRawSz); e->vertOff = cursor; e->vertCount = modelVertexCounts[m]; mcpy(blob + cursor, halfVerts, vRawSz); cursor += (u32)vSz; 
        OS_Free(halfVerts, vRawSz);
        size_t tRawSz = (size_t)modelTriangleCounts[m] * 3 * sizeof(u16), tSz = AlignUp4((u32)tRawSz); e->triOff = cursor; e->triCount = modelTriangleCounts[m]; mcpy(blob + cursor, modelTriangles[m], tRawSz); cursor += (u32)tSz;
        size_t pRawSz = (size_t)physVertCounts[m] * 3 * sizeof(float), pSz = AlignUp4((u32)pRawSz); e->physPosOff = cursor; e->physVertCount = physVertCounts[m]; mcpy(blob + cursor, physPos[m], pRawSz); cursor += (u32)pSz;
        size_t ptRawSz = (size_t)modelTriangleCounts[m] * 3 * sizeof(u16), ptSz = AlignUp4((u32)ptRawSz); e->physTriOff = cursor; e->physTriCount = modelTriangleCounts[m]; mcpy(blob + cursor, physTris[m], ptRawSz); cursor += (u32)ptSz;
        if (modelBVHNodeCounts[m]) { size_t bnRawSz = (size_t)modelBVHNodeCounts[m] * sizeof(BvhNode), bnSz = AlignUp4((u32)bnRawSz); e->bvhNodeOff = cursor; e->bvhNodeCount = modelBVHNodeCounts[m]; mcpy(blob + cursor, modelBVHNodes[m], bnRawSz); cursor += (u32)bnSz; }
        if (modelBVHTriOrderCounts[m]) { size_t btRawSz = (size_t)modelBVHTriOrderCounts[m] * sizeof(u16), btSz = AlignUp4((u32)btRawSz); e->bvhTriOrderOff = cursor; e->bvhTriOrderCount = modelBVHTriOrderCounts[m]; mcpy(blob + cursor, modelBVHTriOrder[m], btRawSz); cursor += (u32)btSz; }
        if (cvxAdjOffsetCounts[m]) { size_t aoRawSz = (size_t)cvxAdjOffsetCounts[m] * sizeof(u32), aoSz = AlignUp4((u32)aoRawSz); e->cvxAdjOffOff = cursor; e->cvxAdjOffCount = cvxAdjOffsetCounts[m]; mcpy(blob + cursor, cvxAdjOffsets[m], aoRawSz); cursor += (u32)aoSz; }
        if (cvxAdjListCounts[m]) { size_t alRawSz = (size_t)cvxAdjListCounts[m] * sizeof(u16), alSz = AlignUp4((u32)alRawSz); e->cvxAdjListOff = cursor; e->cvxAdjListCount = cvxAdjListCounts[m]; mcpy(blob + cursor, cvxAdjLists[m], alRawSz); cursor += (u32)alSz; }
    }
    ModelsBinHeader header = { .magicNumber=MODELS_BIN_MAGIC, .version=MODELS_BIN_VERSION, .mdlsCnt=mdlsCnt, .size=(u32)totalSize, };
    FHandle fd = OS_OpenWriteonly(outPath);
    if (fd == (FHandle)-1) { PrintLog("Could not open %s for writing\n", outPath); OS_Exit(1); }
    OS_Write(fd, &header, sizeof(ModelsBinHeader), outPath);
    OS_Write(fd, blob, totalSize, outPath);
    OS_Close(fd);
    PrintLog("Wrote %s: %u models, %u bytes\n", outPath, mdlsCnt, (u32)totalSize);
    OS_Free(blob, totalSize);
}

void GenerateModelsBin() {
    const char* modelsListPath = "./Data/models.txt";
    const char* outPath        = "./models.bin";
    double startModelTime = get_time();
    ModelDataParser mp = {0};
    if (!ParseModelData(&mp, MAX_MDLS, modelsListPath)) { PrintLog("Failed %s\n", modelsListPath); OS_Exit(1); }
    PrintLog("Parsing models (%d) ...", mp.count);
    u32 maxid = 0; for (u32 i=0; i<mp.count; ++i) { if (mp.entries[i].index != U16_MAX && mp.entries[i].index > maxid) maxid = mp.entries[i].index; }
    mdlsCnt = (u16)maxid + 1;
    if (mdlsCnt >= MAX_MDLS) { PrintLog("mdlsCnt %u exceeds MAX_MDLS\n", mdlsCnt); OS_Exit(1); }
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
    if (threadCnt > 1) { for (int i=0;i<threadCnt;++i) OS_ThreadCreate(&th[i],ModelParsingWorker,&tasks[i]); for (int i=0;i<threadCnt;++i) OS_ThreadJoin(&th[i]); }
    else { for (int t=0;t<threadCnt;++t) ModelParsingWorker(&tasks[t]); }
    for (u32 i=0; i<mdlsCnt; ++i) if (raw[i].data) OS_Free((void*)raw[i].data,raw[i].size);

    physPos = (float**)OS_Alloc(mdlsCnt * sizeof(float*));
    physTris = (u16**)OS_Alloc(mdlsCnt * sizeof(u16*));
    physVertCounts = (u32*)OS_Alloc(mdlsCnt * sizeof(u32));
    cvxAdjOffsets = (u32**)OS_Alloc(mdlsCnt * sizeof(u32*));
    cvxAdjLists   = (u16**)OS_Alloc(mdlsCnt * sizeof(u16*));
    PhysGeomTask ptasks[32]; OS_Thread pth[32];
    for (int i=0;i<threadCnt;++i) ptasks[i] = (PhysGeomTask){i*chunk,(i+1)*chunk > mdlsCnt ? mdlsCnt : (i+1)*chunk,i};
    if (threadCnt > 1) { for (int i=0;i<threadCnt;++i) OS_ThreadCreate(&pth[i],PhysGeomWorker,&ptasks[i]); for(int i=0;i<threadCnt;++i){OS_ThreadJoin(&pth[i]);} }
    else { for(int t=0;t<threadCnt;++t){PhysGeomWorker(&ptasks[t]);} }
    WriteModelsBin(outPath);
    for (u32 i = 0; i < mdlsCnt; ++i) { if(cvxAdjOffsets[i]){OS_Free(cvxAdjOffsets[i],cvxAdjOffsetCounts[i] * sizeof(u32));} if(cvxAdjLists[i]){OS_Free(cvxAdjLists[i],cvxAdjListCounts[i] * sizeof(u16));} }
    OS_Free(cvxAdjOffsets,mdlsCnt * sizeof(u32*)); OS_Free(cvxAdjLists,mdlsCnt * sizeof(u16*)); OS_Free(arena_base,arena); OS_Free(mp.entries,mp.count * sizeof(ModelData));
    PrintLog(" finished in %f secs\n", get_time() - startModelTime);
}

int main(int argc, char** argv) {
    if (argc < 2) { PrintLog("Usage: vparser [options]\nOptions:\n  -text    Generate text.bin\n  -models  Generate models.bin\n  -all     Generate all asset binaries\n"); return 1; }
    threadCnt = clamp(OS_GetNumThreads(),1,32);
    int buildAll = (sEqual(argv[1],"-all"));
    if (buildAll || sEqual(argv[1],"-text")) { PrintLog("[vparser] Building text.bin...\n"); GenerateTextBin(); }
    if (buildAll || sEqual(argv[1],"-models")) { PrintLog("[vparser] Building models binaries...\n"); GenerateModelsBin(); }
    PrintLog("[vparser] Done.\n");
    return 0;
}
