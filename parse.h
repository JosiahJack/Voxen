// parse.h - Common parsing declarations and types shared between Voxen and vparser binaries
// 3D Models
// Layout on disk: [ModelsBinHeader][VoidSquasher-compressed bytes]
// Layout once decompressed (the "blob"): [ModelDirEntry * mdlsCnt][payload...]
// Every offset in ModelDirEntry is a byte offset from the start of the blob (i.e. from the first
// ModelDirEntry itself) - a loaded model's data is just `(u8*)blob + entry->xOff`, no relocation
// pass, no pointers stored on disk. That's what makes the engine-side load trivial.
#pragma once
enum{MAX_MDLS=6000,T_BUFFER_SIZE=1024,MAX_GLYPHS=4096,FONT_ATLAS_SIZE=4672,FONT_NORMAL=0,FONT_STOPD=1,WELD_HASH_SIZE=32768,MAX_VERT_ELEMENT_SIZE=6964,MAX_OUTPUT_VERTS=20960,BVH_MAX_DEPTH=3,BVH_LEAF_MAX_TRIS=8,
     BVH_MAX_NODES_PER_MDL=586/*1 + 8 + 64 + 512 = 585 worst case, +safety*/,BVH_MAX_TRIS_PER_MDL=6986,VRT_ATT_SZ=16,CPU_VRT_SZ=32};
#define MODELS_BIN_MAGIC   0x534C444Du // 'MDLS'
#define MODELS_BIN_VERSION 1u
#pragma pack(push, 1)
typedef struct { u32 magicNumber; u32 version; u32 mdlsCnt; u32 uncompressedSize; /*size of the decompressed [directory + payload] blob*/ u32 compressedSize; } ModelsBinHeader;
// One entry per model slot, dense and index-addressable (mdlsCnt entries). An unused slot
// (no source .obj at that index) has vertCount == 0 and every other field zeroed.
typedef struct {
    u32 vertOff, vertCount;          // GPU-ready interleaved half verts: pos3,nrm3,uv2 (VRT_ATT_SZ/vert)
    u32 triOff, triCount;            // render index buffer, u16 * 3 per tri
    u32 physPosOff, physVertCount;   // welded fp32 positions, 3 floats/vert - CPU physics data
    u32 physTriOff, physTriCount;    // welded index buffer, u16 * 3 per tri (indexes physPos)
    u32 bvhNodeOff, bvhNodeCount;    // BvhNode[]
    u32 bvhTriOrderOff, bvhTriOrderCount; // u16[]
    float bound;                     // modelBounds[m] equivalent
} ModelDirEntry;
#pragma pack(pop)
// Text / Fonts
#define TEXT_BIN_MAGIC 0x54585442
#pragma pack(push, 1)
typedef struct { u32 magic; i32 atlasSize,numPackedGlyphs,numPackedGlyphsStopD; float fixedNumberAdvanceWidth,fixedNumberAdvanceWidthStopD; u32 packedCharBytes,packedCharStopDBytes,atlasBytes; } TextBinHeader;
#pragma pack(pop)
typedef struct{u16 x0,y0,x1,y1;float xoff,yoff,xadvance,xoff2,yoff2;}stbtt_packedchar;
typedef struct{float x0,y0,s0,t0,x1,y1,s1,t1;} aligned_quad;
typedef struct{i32 first,count,startIndex;}GlyphRange;
void DualLog(const char* s, ...);
void DualLogWarn(const char* s, ...);
void DualLogError(const char* s, ...);
bool cEmpty(const char c);
INLINE u32 parse_numberu32(const char* str, const char* line, u32 lineNum) {
    if(str == 0 || *str == '\0'){DualLogError("Invalid from line[%d]: %s\n",lineNum+1,line); return 0;}
    while(cEmpty((char)*str)){str++;} while(cEmpty(*str)){str++;} if(*str == '+'){str++;}
    if(*str == '-'){DualLogError("Invalid negative u32(%s) from line[%d]: %s\n",str,lineNum+1,line); return 0;}
    u64 result=0; while (*str >= '0' && *str <= '9') { i32 digit=*str-'0'; result=result*10 + (u64)digit; str++; } return (u32)result;
}

INLINE u16 parse_numberu16(const char* str, const char* line, u32 lineNum) { u32 retval = parse_numberu32(str, line, lineNum); if (retval > U16_MAX) { DualLogError("Value %u out of range for u16 from line[%d]: %s\n", retval, lineNum+1, line); return 0; } return (u16)retval; }
INLINE u8 parse_numberu8(const char* str, const char* line, u32 lineNum) { u32 retval = parse_numberu32(str, line, lineNum); if (retval > 255) { DualLogError("Value %u out of range for u8 from line[%d]: %s\n", retval, lineNum+1, line); return 0; } return (u8)retval; }
INLINE bool parse_bool(const char* str, const char* line, u32 lineNum) { u32 parseval = parse_numberu32(str, line, lineNum); if (parseval > 1) {DualLogWarn("Loaded %u but expected boolean from line[%u]: %s\n",parseval, lineNum+1, line);} return parseval > 0 ? true : false; }
