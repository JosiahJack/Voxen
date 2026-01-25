// data_models.c - Load 3D Models from .vmdl caches or .fbx via Assimp if cache invalid
#include <malloc.h>
#include "./External/assimp/cimport.h"
#include "./External/assimp/scene.h"
#include "os.h" // Operating System calls shim layer.
#include "voxen.h"
float** modelVertices = NULL;
uint32_t** modelTriangles = NULL;
uint32_t modelVertexCounts[MODEL_IDX_MAX] = {0}; // 4kb
uint32_t modelTriangleCounts[MODEL_IDX_MAX] = {0}; // 4kb
bool modelHasAnimation[MODEL_IDX_MAX] = {0}; // 1kb
float modelBounds[MODEL_IDX_MAX * BOUNDS_ATTRIBUTES_COUNT] = {0}; // 1024 * 7 * 4 = 28.6kb
uint16_t loadedModelsMaxIndex = 0;
const AnimationClip modelAnimationClips[MAX_ANIMATED_MODELS][MAX_ANIMATION_CLIPS_PER_MODEL] = { // speed, frameStart, frameEnd, frameStartModelIndex, framerate
    [0]={[ANIM_IDLE_CLOSED]={1.0f,2,2,699,24},[ANIM_OPENING]={1.0f,2,11,699,24},[ANIM_IDLE_OPEN]={1.0f,11,11,708,24},[ANIM_CLOSING]={1.0f,12,21,709,24}}, // doorB (door2)
    [1]={[ANIM_IDLE_CLOSED]={1.0f,2,2,719,24},[ANIM_OPENING]={1.0f,2,12,719,24},[ANIM_IDLE_OPEN]={1.0f,12,12,729,24},[ANIM_CLOSING]={1.0f,14,24,731,24}}, // doorA (door1)
    [2]={[ANIM_IDLE]={1.0f,0,37,742,30},[ANIM_WALK]={1.0f,50,99,780,30},[ANIM_RUN]={1.1f,50,99,792,30},[ANIM_ATTACK1]={0.75f,111,136,830,30},[ANIM_PAIN]={0.5f,138,150,856,30},[ANIM_DYING]={0.75f,153,176,869,30}}, // npc_humanoid_mutant
    [3]={[ANIM_IDLE]={1.0f,1,207,893,24},[ANIM_ATTACK1]={1.0f,219,239,1100,24},[ANIM_WALK]={1.0f,252,308,1121,24},[ANIM_RUN]={1.0f,252,308,1121,24},[ANIM_PAIN]={1.0f,321,330,1177,24},[ANIM_PAIN2]={1.0f,331,344,1187,24},[ANIM_DYING]={1.0f,345,369,1201,24}}, // npc_cyborg_drone 
    [4]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1234,24},[ANIM_OPENING]={1.5f,2,44,1234,24},[ANIM_IDLE_OPEN]={1.0f,44,44,1276,24},[ANIM_CLOSING]={1.75f,46,96,1277,24}}, // doorD (door4, bulkhead 1)
    [5]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1328,24},[ANIM_OPENING]={1.0f,2,25,1328,24},[ANIM_IDLE_OPEN]={1.0f,25,25,1351,24},[ANIM_CLOSING]={1.0f,27,44,1352,24}}, // doorC (door3)
    [6]={[ANIM_IDLE_CLOSED]={1.0f,1,1,1444,24},[ANIM_OPENING]={1.2f,1,30,1444,24},[ANIM_IDLE_OPEN]={1.0f,30,30,1399,24},[ANIM_CLOSING]={1.2f,32,66,1400,24}}, // doorK (xdoor1)
    [7]={[ANIM_IDLE_CLOSED]={1.0f,3,3,1435,24},[ANIM_OPENING]={1.2f,3,24,1435,24},[ANIM_IDLE_OPEN]={1.0f,26,26,1457,24},[ANIM_CLOSING]={1.2f,27,49,1458,24}}, // doorJ (xdoor2)
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
    [30] = {[ANIM_LOOP_ALL]    = { 1.0f, 0, 121, 4313, 24 }}, // npc_flierbot
    [31] = {[ANIM_LOOP_ALL]    = { 1.0f, 0, 179, 4435, 24 }}, // npc_gortiger
    [32] = {[ANIM_LOOP_ALL]    = { 1.0f, 0, 248, 4615, 24 }}, // npc_hopper
    [33] = {[ANIM_LOOP_ALL]    = { 1.0f, 0, 102, 4864, 24 }}, // npc_invisomut
    [34] = {[ANIM_LOOP_ALL]    = { 1.0f, 0, 169, 4967, 24 }}, // npc_maintenancebot
    [35] = {[ANIM_LOOP_ALL]    = { 1.0f, 0, 119, 5137, 15 }}, // npc_mutant_avian
    [36] = {[ANIM_LOOP_ALL]    = { 1.0f, 0, 239, 5257, 24 }}, // npc_plantmutant
    [37] = {[ANIM_LOOP_ALL]    = { 1.0f, 0, 148, 5497, 24 }}, // npc_repairbot
    [38] = {[ANIM_LOOP_ALL]    = { 1.0f, 0, 95,  5646, 24 }}, // npc_sec1bot
    [39] = {[ANIM_LOOP_ALL]    = { 1.0f, 0, 75,  5742, 24 }}, // npc_sec2bot
    [40] = {[ANIM_LOOP_ALL]    = { 1.0f, 0, 84,  5818, 24 }}, // npc_servbot
    [41] = {[ANIM_LOOP_ALL]    = { 1.0f, 0, 225, 5903, 24 }}, // npc_virusmutant
    [42] = {[ANIM_LOOP_ALL]    = { 1.0f, 0, 156, 6129, 24 }}, // npc_zerogmut
    [43] = {[ANIM_LOOP_ALL]    = { 1.0f, 1, 43,  6286, 24 }}, // puzzlepanel1
    [44] = {[ANIM_LOOP_ALL]    = { 1.0f, 0, 30,  6329, 24 }}, // puzzlepanel2 (starts at 000000)
    [45] = {[ANIM_LOOP_ALL]    = { 1.0f, 0, 18,  6360, 24 }}, // puzzlepanel3 (starts at 000000)
    [46] = {[ANIM_LOOP_ALL]    = { 1.0f, 1, 100, 6379, 24 }}, // sparkingwire
    [47] = {[ANIM_LOOP_ALL]    = { 1.0f, 1, 7,   6479, 24 }}, // switch4
    [48] = {[ANIM_LOOP_ALL]    = { 1.0f, 1, 12,  6486, 24 }}, // switch5
    [49] = {[ANIM_LOOP_ALL]    = { 1.0f, 1, 25,  6498, 24 }}, // v_pipe
    [50] = {[ANIM_LOOP_ALL]    = { 1.0f, 1, 23,  6523, 24 }}, // v_rapier
    [51] = {[ANIM_LOOP_ALL]    = { 1.0f, 1, 258, 6546, 24 }} // npc_mutant_cyborg
};

GLuint SetupSSBO(GLuint* id, GLuint bindingIndex, GLsizeiptr size, const void* data, GLenum usage);
struct aiPropertyStore* props;

static bool load_vmdl(const char *vmdl_path, uint64_t fbx_stamp, float **out_verts, uint32_t *out_vcount, uint32_t **out_idx, uint32_t *out_icount, void** out_map, size_t* out_mapsz) {
    OsFileHandle fd; int st_size; uint8_t* map = OS_OpenAndAllocateFileBufferReadonly(vmdl_path, &fd, &st_size);
    if ((size_t)st_size < sizeof(uint64_t) + 4 + 4) return false;
    
    uint64_t file_stamp_on_disk;
    memcpy(&file_stamp_on_disk, map, sizeof(uint64_t));
    if (file_stamp_on_disk != fbx_stamp) { OS_DeallocateRAM(map, (size_t)st_size); return false; }

    const uint8_t *p = map + sizeof(uint64_t);
    uint32_t vcnt = *(uint32_t*)p; p += 4; *out_vcount = vcnt;
    uint32_t icnt = *(uint32_t*)p; p += 4; *out_icount = icnt;
    size_t vert_bytes = vcnt * VERTEX_ATTRIBUTES_COUNT * sizeof(float);
    size_t idx_bytes  = icnt * 3 * sizeof(uint32_t);
    size_t expected   = sizeof(uint64_t) + 4 + vert_bytes + 4 + idx_bytes;
    if (expected != (size_t)st_size) { DualLogError("vmdl corrupted: size %zu, expected %zu from vertex count %u and tri count %u\n", st_size, expected, vcnt, icnt); OS_DeallocateRAM(map, (size_t)st_size); return false; }
    if (p + vert_bytes + idx_bytes > map + (size_t)st_size) { DualLogError("vmdl data overflow\n"); OS_DeallocateRAM(map, (size_t)st_size); return false; }

    *out_verts  = (float*)p;
    p += vert_bytes;
    *out_idx    = (uint32_t*)p;
    *out_map = map;
    *out_mapsz = (size_t)st_size;
    return true;
}

static void write_vmdl(const char *vmdl_path, const uint64_t fbx_stamp, const float *verts, uint32_t vcnt, const uint32_t *triangleIndices, uint32_t triCount) {
    OsFileHandle fd = OS_OpenWriteonly(vmdl_path);
    size_t total = sizeof(uint64_t) + 4 + vcnt*VERTEX_ATTRIBUTES_COUNT*sizeof(float) + 4 + triCount*3*sizeof(uint32_t);
    uint8_t *buf = OS_AllocateRAM(NULL, total, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, OS_INVALID_HANDLE);
    if (!buf) { OS_Close(fd); return; }

    uint8_t *p = buf;
    *(uint64_t *)p = fbx_stamp;
    p += sizeof(uint64_t);
    *(uint32_t*)p = vcnt; p += 4;
    *(uint32_t*)p = triCount; p += 4;
    memcpy(p, verts, vcnt*VERTEX_ATTRIBUTES_COUNT*sizeof(float)); p += vcnt*VERTEX_ATTRIBUTES_COUNT*sizeof(float);
    memcpy(p, triangleIndices, triCount*3*sizeof(uint32_t));
    OS_Write(fd, buf, total, vmdl_path);
    OS_DeallocateRAM(buf,total);
    OS_Close(fd);
}

typedef struct { void* ptr; size_t size; } MMapEntry;
MMapEntry mmap_cleanup[MODEL_IDX_MAX];
int mmap_cleanup_count = 0;
void cleanup_all_mmaps(void) { for (int i = 0; i < mmap_cleanup_count; i++) OS_DeallocateRAM(mmap_cleanup[i].ptr, mmap_cleanup[i].size); }

void LoadModel(bool fromCache, uint16_t i, const char* fbx_path, const char* vmdl_path, uint64_t fbx_stamp, float* cached_verts, uint32_t cached_vcnt, uint32_t* cached_idx, uint32_t cached_icnt, void* mmap_map, size_t mmap_size) {
    const struct aiScene* scene = NULL;
    if (!fromCache) {
        DualLog("No vmdl found or .fbx model was updated so needs refresh from .fbx source, loading %s with Assimp...\n", fbx_path);
        scene = aiImportFileExWithProperties(fbx_path, /*aiProcess_Triangulate*/ 0x8 | /*aiProcess_GenNormals*/ 0x20 | 0x800/*aiProcess_ImproveCacheLocality*/ | /*aiProcess_JoinIdenticalVertices*/ 0x2, NULL, props); // aiProcess vars from https://github.com/assimp/assimp/blob/672594c230832252f94bc90c19ca9ee9917be563/include/assimp/postprocess.h#L170
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) { DualLogError("Assimp failed %s: %s\n", fbx_path, aiGetErrorString()); return; }
    } // else use existing .vmdl binary RAM blob (aka a cache hit was successful)

    uint32_t vertexCount = 0, triCount = 0;
    if (fromCache) { vertexCount = cached_vcnt; triCount = cached_icnt;
    } else {  for (uint32_t m = 0; m < scene->mNumMeshes; ++m) { vertexCount += scene->mMeshes[m]->mNumVertices;  triCount += scene->mMeshes[m]->mNumFaces; }  }
    
    modelVertexCounts[i]   = vertexCount;
    modelTriangleCounts[i] = triCount;
    modelVertices[i]  = fromCache ? (float*)cached_verts : OS_AllocateRAM(NULL, vertexCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
    modelTriangles[i] =  fromCache ? (uint32_t*)cached_idx : OS_AllocateRAM(NULL, triCount * 3 * sizeof(uint32_t), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
    uint32_t vertexIndex = 0, triangleIndex = 0, globalVertexOffset = 0;
    float minx = 1E9f, miny = 1E9f, minz = 1E9f; float maxx = -1E9f, maxy = -1E9f, maxz = -1E9f;
    if (fromCache) {
        for (uint32_t vert = 0; vert < cached_vcnt; ++vert) {
            float x = cached_verts[(vert * VERTEX_ATTRIBUTES_COUNT) + 0];
            float y = cached_verts[(vert * VERTEX_ATTRIBUTES_COUNT) + 1];
            float z = cached_verts[(vert * VERTEX_ATTRIBUTES_COUNT) + 2];
            minx = vmin(minx, x); maxx = vmax(maxx, x);
            miny = vmin(miny, y); maxy = vmax(maxy, y);
            minz = vmin(minz, z); maxz = vmax(maxz, z);
        }
    } else {
        for (uint32_t m = 0; m < scene->mNumMeshes; ++m) {
            struct aiMesh *mesh = scene->mMeshes[m];
            for (uint32_t vert = 0; vert < mesh->mNumVertices; ++vert) {
                modelVertices[i][vertexIndex++] = mesh->mVertices[vert].x; modelVertices[i][vertexIndex++] = mesh->mVertices[vert].y; modelVertices[i][vertexIndex++] = mesh->mVertices[vert].z;
                modelVertices[i][vertexIndex++] = mesh->mNormals[vert].x; modelVertices[i][vertexIndex++] = mesh->mNormals[vert].y;  modelVertices[i][vertexIndex++] = mesh->mNormals[vert].z;
                float u = (mesh->mTextureCoords[0] && mesh->mNumUVComponents[0] > 0) ? mesh->mTextureCoords[0][vert].x : 0.0f;
                float v = (mesh->mTextureCoords[0] && mesh->mNumUVComponents[0] > 0) ? mesh->mTextureCoords[0][vert].y : 0.0f;
                modelVertices[i][vertexIndex++] = u; modelVertices[i][vertexIndex++] = v;
                minx = vmin(minx, mesh->mVertices[vert].x); maxx = vmax(maxx, mesh->mVertices[vert].x);
                miny = vmin(miny, mesh->mVertices[vert].y); maxy = vmax(maxy, mesh->mVertices[vert].y);
                minz = vmin(minz, mesh->mVertices[vert].z); maxz = vmax(maxz, mesh->mVertices[vert].z);
            }

            for (uint32_t f = 0; f < mesh->mNumFaces; ++f) {
                struct aiFace *face = &mesh->mFaces[f]; if (face->mNumIndices != 3) { DualLogError("Non-tri face in %s\n", fbx_path); continue; }
                
                uint32_t a = face->mIndices[0] + globalVertexOffset; uint32_t b = face->mIndices[1] + globalVertexOffset; uint32_t c = face->mIndices[2] + globalVertexOffset;
                modelTriangles[i][triangleIndex++] = a; modelTriangles[i][triangleIndex++] = b; modelTriangles[i][triangleIndex++] = c;
            }
            
            globalVertexOffset += mesh->mNumVertices;
        }
    }

    uint32_t base = i * BOUNDS_ATTRIBUTES_COUNT;
    modelBounds[base + BOUNDS_DATA_OFFSET_MINX] = minx;
    modelBounds[base + BOUNDS_DATA_OFFSET_MINY] = miny;
    modelBounds[base + BOUNDS_DATA_OFFSET_MINZ] = minz;
    modelBounds[base + BOUNDS_DATA_OFFSET_MAXX] = maxx;
    modelBounds[base + BOUNDS_DATA_OFFSET_MAXY] = maxy;
    modelBounds[base + BOUNDS_DATA_OFFSET_MAXZ] = maxz;
    float r = 0.0f;
    r = vmax(r, vabs(minx)); r = vmax(r, vabs(miny)); r = vmax(r, vabs(minz));
    r = vmax(r, maxx);       r = vmax(r, maxy);       r = vmax(r, maxz);
    modelBounds[base + BOUNDS_DATA_OFFSET_RADIUS] = r;
    if (fromCache) { mmap_cleanup[mmap_cleanup_count].ptr = mmap_map; mmap_cleanup[mmap_cleanup_count].size = mmap_size; mmap_cleanup_count++;
    } else {
        write_vmdl(vmdl_path, fbx_stamp, modelVertices[i], vertexCount, modelTriangles[i], triCount);
        aiReleaseImport(scene);
    }
}

void LoadModels(void) {
    if (loadedModelsMaxIndex > 0) return;

    DebugRAM("start of LoadModels");
    double start_time = get_time();        
    DataParser model_parser;
    if (!parse_data_file(&model_parser, MODEL_IDX_MAX, "./Data/models.txt")) { DualLogError("Could not parse ./Data/models.txt!\n"); OS_Exit(1); }

    int32_t maxIndex = -1;
    for (uint32_t k = 0; k < model_parser.count; k++) {
        if (model_parser.entries[k].index > maxIndex && model_parser.entries[k].index != UINT16_MAX) maxIndex = model_parser.entries[k].index;
    }

    loadedModelsMaxIndex = (uint16_t)maxIndex + 1U;
    DualLog("Loading   models( %d/%d) with max index  %d ...", loadedModelsMaxIndex, model_parser.count, maxIndex);
    modelVertices  = OS_AllocateRAM(NULL, loadedModelsMaxIndex * sizeof(float*),    PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
    modelTriangles = OS_AllocateRAM(NULL, loadedModelsMaxIndex * sizeof(uint32_t*), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
    DebugRAM("after main OS_AllocateRAM block");
    size_t indexToParser_size = loadedModelsMaxIndex * sizeof(int32_t);
    int32_t* indexToParser = OS_AllocateRAM(NULL, indexToParser_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, OS_INVALID_HANDLE);
    for (uint32_t k = 0; k < model_parser.count; k++) {
        if (model_parser.entries[k].index != UINT16_MAX) indexToParser[model_parser.entries[k].index] = (int32_t)k;
    }
    
    props = aiCreatePropertyStore();
    aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_READ_ANIMATIONS, 1);
    aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_READ_MATERIALS, 0);
    aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_READ_TEXTURES, 0);
    aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_READ_LIGHTS, 0);
    aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_READ_CAMERAS, 0);
    aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_OPTIMIZE_EMPTY_ANIMATION_CURVES, 1);
    aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_NO_SKELETON_MESHES, 0);
    aiSetImportPropertyInteger(props, AI_CONFIG_PP_RVC_FLAGS, aiComponent_ANIMATIONS | aiComponent_BONEWEIGHTS);
    aiSetImportPropertyInteger(props, AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
    aiSetImportPropertyInteger(props, AI_CONFIG_PP_ICL_PTCACHE_SIZE, 16);
    aiSetImportPropertyInteger(props, AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4);
    aiSetImportPropertyInteger(props, AI_CONFIG_PP_FD_REMOVE, 1);
    aiSetImportPropertyInteger(props, AI_CONFIG_PP_PTV_KEEP_HIERARCHY, 0);
    DebugRAM("prior to model load loop");
    for (uint32_t i = 0; i < loadedModelsMaxIndex; ++i) {
        int32_t parserIdx = indexToParser[i];
        modelHasAnimation[i] = (model_parser.entries[parserIdx].entflags & ENTFLAG_ANIMATED);
        const char *fbx_path = model_parser.entries[parserIdx].path;
        if (!fbx_path || !fbx_path[0]) { DualLogError("No fbx path for model index %u\n", i); OS_Exit(1); }

        char vmdl_path[256];
        size_t fbx_path_sz = strlen(fbx_path);
        strncpy(vmdl_path, fbx_path, fbx_path_sz - 3); // Chop off "fbx" or "obj" and then manually add "vmdl" terminating with \0 within the temp buffer.
        vmdl_path[fbx_path_sz - 3] = 'v'; vmdl_path[fbx_path_sz - 2] = 'm'; vmdl_path[fbx_path_sz - 1] = 'd'; vmdl_path[fbx_path_sz] = 'l'; vmdl_path[fbx_path_sz + 1] = '\0';
        if (!vmdl_path[0] || strcmp(vmdl_path, ".vmdl") == 0 || vmdl_path[0] == '.') { DualLogError("Invalid vmdl_path for %s: '%s'\n", fbx_path, vmdl_path); OS_Exit(1); }

        FileFingerprint fp;
        if (!OS_GetFileFingerprint(fbx_path, &fp)) { DualLogError("File change detection failed for %s\n", fbx_path); continue; }
        
        uint64_t fbx_stamp = OS_GetFilestamp(&fp);
        float  *cached_verts = NULL; uint32_t cached_vcnt = 0; uint32_t *cached_idx  = NULL; uint32_t cached_icnt = 0; void* mmap_map = NULL; size_t mmap_size = 0;
        bool cache_hit = load_vmdl(vmdl_path, fbx_stamp, &cached_verts, &cached_vcnt, &cached_idx,  &cached_icnt, &mmap_map, &mmap_size);
        LoadModel(cache_hit, i, fbx_path, vmdl_path, fbx_stamp, cached_verts, cached_vcnt, cached_idx, cached_icnt, mmap_map, mmap_size);        
    }

    DebugRAM("after model load loop");
    OS_DeallocateRAM(indexToParser,indexToParser_size);
    aiReleasePropertyStore(props);
    glGenBuffers(loadedModelsMaxIndex, Sys_Render.vbos);
    glGenBuffers(loadedModelsMaxIndex, Sys_Render.tbos);
    uint32_t totalVertices = 0, totalTris = 0;
    for (int i = 0; i < loadedModelsMaxIndex; ++i) {
        if (modelVertexCounts[i] == 0) continue;

        size_t vertSize = modelVertexCounts[i] * VERTEX_ATTRIBUTES_COUNT * sizeof(float);
        totalVertices += modelVertexCounts[i];
        size_t triSize  = modelTriangleCounts[i] * 3 * sizeof(uint32_t);
        totalTris += (uint32_t)triSize;
        glBindBuffer(GL_ARRAY_BUFFER, Sys_Render.vbos[i]);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)vertSize, NULL, GL_STATIC_DRAW);
        void* ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, (GLsizeiptr)vertSize, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        memcpy(ptr, modelVertices[i], vertSize);
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Sys_Render.tbos[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)triSize, NULL, GL_STATIC_DRAW);
        ptr = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, (GLsizeiptr)triSize, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        memcpy(ptr, modelTriangles[i], triSize);
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
        glFlush(); glFinish(); // Surprisingly also causes the LoadTextures OpenGL driver in Linux to drop its CPU side RAM duplicates earlier
    }
    
    DebugRAM("after to model to gpu transfer");
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glFlush(); glFinish();
    free(model_parser.entries);
    DualLog(" total vertices: %u, total tris: %u, took %f secs\n", totalVertices, totalTris, get_time() - start_time);
    cleanup_all_mmaps(); // Uggggh, can't without losing mesh collision support at the moment.
    #ifndef WINDOWS
        malloc_trim(0);
    #endif
    DebugRAM("After Load Models");
}
