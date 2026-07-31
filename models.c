// models.c - 3D Models load from offline cache .bin
#include "common.h"
#include "lib.h"
static float **vPos;
BvhNode** modelBVHNodes; u16** modelBVHTriOrder; u32 modelBVHNodeCounts[MAX_MDLS],modelBVHTriOrderCounts[MAX_MDLS];
u16 uniqueCvxMeshIndices[MAX_UNIQUE_CVX_MESHES]; u32 uniqueCvxMeshCount=0;
u32** cvxAdjOffsets = NULL; u16** cvxAdjLists = NULL; // CSR format adjacency data: cvxAdjOffsets[m] has vCount + 1 entries pointing into cvxAdjLists[m]
u16  cvxAdjStart[MAX_UNIQUE_CVX_MESHES];
static u16 lastCvxSupport[MAX_UNIQUE_CVX_MESHES]={0}; // Persistent hill-climbing state per mesh
static void* modelsBinBlob = NULL; // decompressed [directory + payload]; lives for program lifetime, never freed
int EdgeCompare(const void* a, const void* b) { u32 ea = *(const u32*)a, eb = *(const u32*)b; return (ea > eb) - (ea < eb); }
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

void LoadModels() {
    double startModelTime = get_time();
    FHandle fd; int fsz = 0;
    char* raw = OS_OpenAndAllocateFileBufferReadonly("./models.bin", &fd, &fsz);
    if (!raw || fsz < (int)sizeof(ModelsBinHeader)) { DualLogError("Missing or truncated models.bin - run vparser first\n"); OS_Exit(1); }
    ModelsBinHeader header; mcpy(&header, raw, sizeof(ModelsBinHeader));
    if (header.magicNumber != MODELS_BIN_MAGIC) { DualLogError("models.bin: bad magic\n"); OS_Exit(1); }
    if (header.version != MODELS_BIN_VERSION) { DualLogError("models.bin: version mismatch (got %u, want %u) - rerun vparser\n", header.version, MODELS_BIN_VERSION); OS_Exit(1); }
    if (header.mdlsCnt >= MAX_MDLS) { DualLogError("models.bin: mdlsCnt %u exceeds MAX_MDLS\n", header.mdlsCnt); OS_Exit(1); }
    if ((size_t)fsz < sizeof(ModelsBinHeader) + header.compressedSize) { DualLogError("models.bin: file truncated relative to header\n"); OS_Exit(1); }
    u8* blob = (u8*)OS_Alloc(header.uncompressedSize);
    size_t got = BlowBubblesOfVoid((const u8*)raw + sizeof(ModelsBinHeader), header.compressedSize, blob, header.uncompressedSize);
    OS_Free(raw, fsz);
    if (got != header.uncompressedSize) { DualLogError("models.bin: decompression failed, expected %u got %u\n", header.uncompressedSize, (u32)got); OS_Exit(1); }
    modelsBinBlob = blob;
    mdlsCnt = (u16)header.mdlsCnt;
    const ModelDirEntry* dir = (const ModelDirEntry*)blob;
    vPos = (float**)OS_Alloc(mdlsCnt * sizeof(float*));
    modelTriangles = (u16**)OS_Alloc(mdlsCnt * sizeof(u16*));
    physPos = (float**)OS_Alloc(mdlsCnt * sizeof(float*));
    physTris = (u16**)OS_Alloc(mdlsCnt * sizeof(u16*));
    physVertCounts = (u32*)OS_Alloc(mdlsCnt * sizeof(u32));
    modelBVHNodes = (BvhNode**)OS_Alloc(mdlsCnt * sizeof(BvhNode*));
    modelBVHTriOrder = (u16**)OS_Alloc(mdlsCnt * sizeof(u16*));
    glGenBuffers(mdlsCnt,vbos); glGenBuffers(mdlsCnt,tbos); u32 tv=0,tt=0;
    for (u32 m = 0; m < mdlsCnt; ++m) {
        const ModelDirEntry* e = &dir[m];
        if (!e->vertCount) { // unused index slot - no source .obj at this index
            physPos[m]=NULL; physTris[m]=NULL; physVertCounts[m]=0;
            modelBVHNodes[m]=NULL; modelBVHTriOrder[m]=NULL; modelBVHNodeCounts[m]=0; modelBVHTriOrderCounts[m]=0;
            vPos[m]=NULL; modelTriangles[m]=NULL; modelVertexCounts[m]=0; modelTriangleCounts[m]=0; modelBounds[m]=0;
            continue;
        }
        tv += e->vertCount; tt += e->triCount; modelBounds[m] = e->bound;

        // Vertex data is already half-float and already vertex-cache/fetch optimized - straight upload, no CPU work.
        size_t vcz = (size_t)e->vertCount * VRT_ATT_SZ, tcz = (size_t)e->triCount * 3 * sizeof(u16);
        glBindBuffer(GL_ARRAY_BUFFER,vbos[m]); glBufferData(GL_ARRAY_BUFFER,vcz,blob + e->vertOff,GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,tbos[m]); glBufferData(GL_ELEMENT_ARRAY_BUFFER,tcz,blob + e->triOff,GL_STATIC_DRAW);

        // Physics + BVH data: pointers straight into the blob, zero copy.
        physPos[m] = (float*)(blob + e->physPosOff); physVertCounts[m] = e->physVertCount;
        physTris[m] = (u16*)(blob + e->physTriOff);
        modelBVHNodes[m] = e->bvhNodeCount ? (BvhNode*)(blob + e->bvhNodeOff) : NULL; modelBVHNodeCounts[m] = e->bvhNodeCount;
        modelBVHTriOrder[m] = e->bvhTriOrderCount ? (u16*)(blob + e->bvhTriOrderOff) : NULL; modelBVHTriOrderCounts[m] = e->bvhTriOrderCount;

        // Preserve the existing contract: after LoadModels(), vPos/modelTriangles/modelVertexCounts/
        // modelTriangleCounts alias the physics (welded, full-float) geometry rather than the render
        // geometry - physics hot paths read these directly as CPU-space positions.
        vPos[m] = physPos[m]; modelTriangles[m] = physTris[m];
        modelVertexCounts[m] = physVertCounts[m]; modelTriangleCounts[m] = e->physTriCount;
    }
    glBindBuffer(GL_ARRAY_BUFFER,0); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
    DualLog("Loaded models (%d): vertices: %u, tris: %u, %f secs\n",mdlsCnt,tv,tt,get_time() - startModelTime); DebugRAM("After LoadModels");
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
