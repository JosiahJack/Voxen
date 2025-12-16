// data_models.c - Load 3D Models from .vmdl caches or .fbx via Assimp if cache invalid
#include "os.h"
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "voxen.h"
#include "entity.h"
#include "vmath.h"
#define C_STRUCT struct // #include <assimp/defs.h>
#define ASSIMP_API
struct aiFileIO; // #include <assimp/cimport.h>
ASSIMP_API C_STRUCT aiPropertyStore *aiCreatePropertyStore(void); // #include <assimp/cimport.h>
ASSIMP_API const C_STRUCT aiScene *aiImportFileExWithProperties(const char *pFile, unsigned int pFlags, C_STRUCT aiFileIO *pFS, const C_STRUCT aiPropertyStore *pProps); // #include <assimp/cimport.h>
ASSIMP_API void aiSetImportPropertyInteger(C_STRUCT aiPropertyStore *store, const char *szName, int value); // #include <assimp/cimport.h>
ASSIMP_API void aiReleasePropertyStore(C_STRUCT aiPropertyStore *p); // #include <assimp/cimport.h>
ASSIMP_API const char *aiGetErrorString(void); // #include <assimp/cimport.h>
ASSIMP_API void aiReleaseImport(const C_STRUCT aiScene *pScene); // #include <assimp/cimport.h>
#include <assimp/scene.h> // Only 514 lines and make use of most all of it so leaving unhoovered, unlike assimp/cimport.h.
ssize_t write(int fd, const void *buf, size_t nbytes); // #include <unistd.h>
int close (int filedes); // #include <unistd.h>
ssize_t read(int fd, void *buf, size_t count);

DataParser model_parser;
float** modelVertices = NULL;
uint32_t modelVertexCounts[MODEL_IDX_MAX] = {0}; // 4kb
uint32_t modelTriangleCounts[MODEL_IDX_MAX] = {0}; // 4kb
uint8_t modelAnimationType[MODEL_IDX_MAX] = {0}; // 1kb
GLuint vbos[MODEL_IDX_MAX] = {0}; // 4kb
GLuint tbos[MODEL_IDX_MAX] = {0}; // 4kb
float modelBounds[MODEL_IDX_MAX * BOUNDS_ATTRIBUTES_COUNT] = {0}; // 1024 * 7 * 4 = 28.6kb
uint16_t loadedModelsMaxIndex = 0;

static void make_vmdl_path(const char *fbx_path, char *out, size_t outsz) {
    strncpy(out, fbx_path, outsz - 1);
    out[outsz - 1] = '\0';
    char *ext = strrchr(out, '.');
    if (ext && strcmp(ext, ".fbx") == 0) strncpy(ext, ".vmdl", outsz - (size_t)(ext - out) - 1);
    else if (strlen(out) + 5 < outsz) strcat(out, ".vmdl");
}

static bool load_vmdl(const char *vmdl_path, uint8_t expected_md5[16], float **out_verts, uint32_t *out_vcount, uint32_t **out_idx, uint32_t *out_icount, void** out_map, size_t* out_mapsz) {
    int fd = open(vmdl_path, O_RDONLY);
    if (fd < 0) return false;

    struct stat st;
    if (fstat(fd, &st) < 0) { close(fd); return false; }
    if (st.st_size < 16 + 4 + 4) { close(fd); return false; }
    
    uint8_t *map = mmap(NULL, (size_t)st.st_size, PROT_READ, MAP_PRIVATE, fd, 0); close(fd);
    if (map == MAP_FAILED) return false;
    if (memcmp(map, expected_md5, 16) != 0) { munmap(map, (size_t)st.st_size); return false; }

    const uint8_t *p = map + 16;
    uint32_t vcnt = *(uint32_t*)p; p += 4; *out_vcount = vcnt;
    uint32_t icnt = *(uint32_t*)p; p += 4; *out_icount = icnt;
    size_t vert_bytes = vcnt * VERTEX_ATTRIBUTES_COUNT * sizeof(float);
    size_t idx_bytes  = icnt * 3 * sizeof(uint32_t);
    size_t expected   = 16 + 4 + vert_bytes + 4 + idx_bytes;
    if (expected != (size_t)st.st_size) { DualLogError("vmdl corrupted: size %zu, expected %zu from vertex count %u and tri count %u\n", st.st_size, expected, vcnt, icnt); munmap(map, (size_t)st.st_size); return false; }
    if (p + vert_bytes + idx_bytes > map + (size_t)st.st_size) { DualLogError("vmdl data overflow\n"); munmap(map, (size_t)st.st_size); return false; }

    *out_verts  = (float*)p;
    p += vert_bytes;
    *out_idx    = (uint32_t*)p;
    *out_map = map;
    *out_mapsz = (size_t)st.st_size;
    return true;
}

static void write_vmdl(const char *vmdl_path, const uint8_t md5[16], const float *verts, uint32_t vcnt, const uint32_t *triangleIndices, uint32_t triCount) {
    int fd = open(vmdl_path, O_WRONLY|O_CREAT|O_TRUNC, 0644);
    if (fd < 0) return;

    size_t total = 16 + 4 + vcnt*VERTEX_ATTRIBUTES_COUNT*sizeof(float) + 4 + triCount*3*sizeof(uint32_t);
    uint8_t *buf = mmap(NULL, total, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);
    if (!buf) { close(fd); return; }

    uint8_t *p = buf;
    memcpy(p, md5, 16); p += 16;
    *(uint32_t*)p = vcnt; p += 4;
    *(uint32_t*)p = triCount; p += 4;
    memcpy(p, verts, vcnt*VERTEX_ATTRIBUTES_COUNT*sizeof(float)); p += vcnt*VERTEX_ATTRIBUTES_COUNT*sizeof(float);
    memcpy(p, triangleIndices, triCount*3*sizeof(uint32_t));
    size_t written = (size_t)write(fd, buf, total);
    if (written != (size_t)total) DualLogError("write_vmdl: partial write %zd/%zu\n", written, total);
    munmap(buf,total);
    close(fd);
}

typedef struct {
    void* ptr;
    size_t size;
} MMapEntry;

MMapEntry mmap_cleanup[MODEL_IDX_MAX];
int mmap_cleanup_count = 0;

void add_mmap_cleanup(void* ptr, size_t size) {
    mmap_cleanup[mmap_cleanup_count].ptr = ptr;
    mmap_cleanup[mmap_cleanup_count].size = size;
    mmap_cleanup_count++;
}

void cleanup_all_mmaps(void) {
    for (int i = 0; i < mmap_cleanup_count; i++) munmap(mmap_cleanup[i].ptr, mmap_cleanup[i].size);
}

// uint8_t modelFBX_FileBuffer[15360000]; // 14983372 found in practice
void LoadModels(void) {
    double start_time = get_time();
    loadedModelsMaxIndex = 0;
    uint16_t animatedModelCount = 0u;
    if (!parse_data_file(&model_parser, "./Data/models.txt")) { DualLogError("Could not parse ./Data/models.txt!\n"); OS_Exit(1); }

    int32_t maxIndex = -1;
    for (uint32_t k = 0; k < model_parser.count; k++) {
        if (model_parser.entries[k].index > maxIndex && model_parser.entries[k].index != UINT16_MAX) maxIndex = model_parser.entries[k].index;
    }

    uint16_t actualLoadedModels = 0u;
    for (int32_t i=0;i<MODEL_IDX_MAX;++i) actualLoadedModels += modelIndexUsedForCurrentLevel[i] ? 1u : 0u;
    loadedModelsMaxIndex = (uint16_t)maxIndex + 1U;
    DualLog("Loading   models( %d/%d) with max index  %d ...", actualLoadedModels, model_parser.count, maxIndex);
    modelVertices       = mmap(NULL, loadedModelsMaxIndex * sizeof(float*), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    uint32_t** modelTriangles      = mmap(NULL, loadedModelsMaxIndex * sizeof(uint32_t*), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    DebugRAM("after main mmap block");
    size_t indexToParser_size = loadedModelsMaxIndex * sizeof(int32_t);
    int32_t* indexToParser = mmap(NULL, indexToParser_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);
    for (uint32_t k = 0; k < model_parser.count; k++) {
        if (model_parser.entries[k].index != UINT16_MAX) indexToParser[model_parser.entries[k].index] = (int32_t)k;
    }
    
    struct aiPropertyStore* props = aiCreatePropertyStore();
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
    DebugRAM("prior to parallel model load loop");
    for (uint32_t i = 0; i < loadedModelsMaxIndex; ++i) {
        int32_t parserIdx = indexToParser[i];
        if (!modelIndexUsedForCurrentLevel[parserIdx]) continue;
        
        modelAnimationType[i] = model_parser.entries[parserIdx].animated;
        if (modelAnimationType[i] > 0u) animatedModelCount++;
        const char *fbx_path = model_parser.entries[parserIdx].path;
        if (!fbx_path || !fbx_path[0]) { DualLogError("No fbx path for model index %u\n", i); OS_Exit(1); }

        char vmdl_path[512];
        make_vmdl_path(fbx_path, vmdl_path, sizeof(vmdl_path));
        if (!vmdl_path[0] || strcmp(vmdl_path, ".vmdl") == 0 || vmdl_path[0] == '.') { DualLogError("Invalid vmdl_path for %s: '%s'\n", fbx_path, vmdl_path); OS_Exit(1); }

        uint8_t fbx_md5[16];
        int fbx_fp = open(fbx_path, O_RDONLY);
        if (!fbx_fp) { DualLogError("Failed to open %s\n", fbx_path); OS_Exit(1); }

        struct stat fbxstat;
        fstat(fbx_fp, &fbxstat);
        uint8_t* buf = mmap(NULL, (size_t)fbxstat.st_size, PROT_READ, MAP_PRIVATE, fbx_fp, 0);
        close(fbx_fp);
        if (buf == MAP_FAILED) { DualLogError("mmap failed for %s\n", fbx_path); OS_Exit(1); }

        size_t fbxread = (size_t)read(fbx_fp, buf, (size_t)fbxstat.st_size);
        if (fbxread == 0) DualLogError("Read failure for %s\n", fbx_path);
                                                     
        md5(buf, (size_t)fbxstat.st_size, fbx_md5);
        munmap(buf, (size_t)fbxstat.st_size);
        float  *cached_verts = NULL; uint32_t cached_vcnt = 0; uint32_t *cached_idx  = NULL; uint32_t cached_icnt = 0; void* mmap_map = NULL; size_t mmap_size = 0;
        bool cache_hit = load_vmdl(vmdl_path, fbx_md5, &cached_verts, &cached_vcnt, &cached_idx,  &cached_icnt, &mmap_map, &mmap_size);
        if (!cache_hit) {
            DualLog("No vmdl found or .fbx model was updated so needs refresh from .fbx source, loading %s with Assimp...\n", fbx_path);
            const struct aiScene *scene = aiImportFileExWithProperties(fbx_path, /*aiProcess_Triangulate*/ 0x8 | /*aiProcess_GenNormals*/ 0x20 | 0x800/*aiProcess_ImproveCacheLocality*/, NULL, props); // aiProcess vars from https://github.com/assimp/assimp/blob/672594c230832252f94bc90c19ca9ee9917be563/include/assimp/postprocess.h#L170
            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) { DualLogError("Assimp failed %s: %s\n", fbx_path, aiGetErrorString()); continue; }

            uint32_t vertexCount = 0, triCount = 0;
            for (uint32_t m = 0; m < scene->mNumMeshes; ++m) {
                vertexCount += scene->mMeshes[m]->mNumVertices;
                triCount    += scene->mMeshes[m]->mNumFaces;
            }
            
            if (vertexCount > 40000 || triCount > 32768) { DualLogError("Model %s exceeds limits\n", fbx_path); aiReleaseImport(scene); continue; }

            modelVertexCounts[i]   = vertexCount;
            modelTriangleCounts[i] = triCount;
            modelVertices[i]  = mmap(NULL, vertexCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            modelTriangles[i] =  mmap(NULL, triCount * 3 * sizeof(uint32_t), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            uint32_t vertexIndex = 0, triangleIndex = 0, globalVertexOffset = 0;
            float minx = 1E9f, miny = 1E9f, minz = 1E9f;
            float maxx = -1E9f, maxy = -1E9f, maxz = -1E9f;
            for (uint32_t m = 0; m < scene->mNumMeshes; ++m) {
                struct aiMesh *mesh = scene->mMeshes[m];
                for (uint32_t vert = 0; vert < mesh->mNumVertices; ++vert) {
                    modelVertices[i][vertexIndex++] = mesh->mVertices[vert].x;
                    modelVertices[i][vertexIndex++] = mesh->mVertices[vert].y;
                    modelVertices[i][vertexIndex++] = mesh->mVertices[vert].z;
                    modelVertices[i][vertexIndex++] = mesh->mNormals[vert].x;
                    modelVertices[i][vertexIndex++] = mesh->mNormals[vert].y;
                    modelVertices[i][vertexIndex++] = mesh->mNormals[vert].z;
                    float u = (mesh->mTextureCoords[0] && mesh->mNumUVComponents[0] > 0) ? mesh->mTextureCoords[0][vert].x : 0.0f;
                    float v = (mesh->mTextureCoords[0] && mesh->mNumUVComponents[0] > 0) ? mesh->mTextureCoords[0][vert].y : 0.0f;
                    modelVertices[i][vertexIndex++] = u;
                    modelVertices[i][vertexIndex++] = v;
                    minx = vmin(minx, mesh->mVertices[vert].x);
                    maxx = vmax(maxx, mesh->mVertices[vert].x);
                    miny = vmin(miny, mesh->mVertices[vert].y);
                    maxy = vmax(maxy, mesh->mVertices[vert].y);
                    minz = vmin(minz, mesh->mVertices[vert].z);
                    maxz = vmax(maxz, mesh->mVertices[vert].z);
                }

                for (uint32_t f = 0; f < mesh->mNumFaces; ++f) {
                    struct aiFace *face = &mesh->mFaces[f];
                    if (face->mNumIndices != 3) { DualLogError("Non-tri face in %s\n", fbx_path); aiReleaseImport(scene); continue; }
                    
                    uint32_t a = face->mIndices[0] + globalVertexOffset;
                    uint32_t b = face->mIndices[1] + globalVertexOffset;
                    uint32_t c = face->mIndices[2] + globalVertexOffset;
                    modelTriangles[i][triangleIndex++] = a;
                    modelTriangles[i][triangleIndex++] = b;
                    modelTriangles[i][triangleIndex++] = c;
                }
                
                globalVertexOffset += mesh->mNumVertices;
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
            r = vmax(r, maxx);        r = vmax(r, maxy);        r = vmax(r, maxz);
            modelBounds[base + BOUNDS_DATA_OFFSET_RADIUS] = r;
            write_vmdl(vmdl_path, fbx_md5, modelVertices[i], vertexCount, modelTriangles[i], triCount);
            aiReleaseImport(scene);
        } else { // Use existing .vmdl binary RAM blob (aka a cache hit was successful):
            modelVertexCounts[i]   = cached_vcnt;
            modelTriangleCounts[i] = cached_icnt;
            modelVertices[i]  = (float*)cached_verts;
            modelTriangles[i] = (uint32_t*)cached_idx;
            float minx = 1E9f, miny = 1E9f, minz = 1E9f;
            float maxx = -1E9f, maxy = -1E9f, maxz = -1E9f;
            for (uint32_t v = 0; v < cached_vcnt; ++v) {
                float x = cached_verts[v*VERTEX_ATTRIBUTES_COUNT + 0];
                float y = cached_verts[v*VERTEX_ATTRIBUTES_COUNT + 1];
                float z = cached_verts[v*VERTEX_ATTRIBUTES_COUNT + 2];
                minx = vmin(minx, x); maxx = vmax(maxx, x);
                miny = vmin(miny, y); maxy = vmax(maxy, y);
                minz = vmin(minz, z); maxz = vmax(maxz, z);
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
            r = vmax(r, maxx);        r = vmax(r, maxy);        r = vmax(r, maxz);
            modelBounds[base + BOUNDS_DATA_OFFSET_RADIUS] = r;
            add_mmap_cleanup(mmap_map, mmap_size);  // defer munmap
        }
    }

    DebugRAM("after to parallel model load loop");
    madvise(indexToParser, indexToParser_size, MADV_DONTNEED); munmap(indexToParser,indexToParser_size);
    aiReleasePropertyStore(props);
    glGenBuffers(loadedModelsMaxIndex, vbos);
    glGenBuffers(loadedModelsMaxIndex, tbos);
    uint32_t totalVertices = 0, totalTris = 0;
    for (int i = 0; i < loadedModelsMaxIndex; ++i) {
        if (modelVertexCounts[i] == 0) continue;

        size_t vertSize = modelVertexCounts[i] * VERTEX_ATTRIBUTES_COUNT * sizeof(float);
        totalVertices += modelVertexCounts[i];
        size_t triSize  = modelTriangleCounts[i] * 3 * sizeof(uint32_t);
        totalTris += (uint32_t)triSize;
        glBindBuffer(GL_ARRAY_BUFFER, vbos[i]);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)vertSize, NULL, GL_STATIC_DRAW);
        void* ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, (GLsizeiptr)vertSize, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        memcpy(ptr, modelVertices[i], vertSize);
        glUnmapBuffer(GL_ARRAY_BUFFER);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tbos[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)triSize, NULL, GL_STATIC_DRAW);
        ptr = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, (GLsizeiptr)triSize, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        memcpy(ptr, modelTriangles[i], triSize);
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
        glFlush(); // Surprisingly also causes the LoadTextures OpenGL driver in Linux to drop its CPU side RAM duplicates earlier
        glFinish();
    }
    
    DebugRAM("after to model to gpu transfer");
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    GLuint modelBoundsID = 0;
    modelBoundsID = SetupSSBO(modelBoundsID, 7, loadedModelsMaxIndex * BOUNDS_ATTRIBUTES_COUNT * sizeof(float), modelBounds, GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glFlush();
    glFinish();
    DualLog(" total vertices: %u, total tris: %u, animated models %u, took %f secs\n", totalVertices, totalTris, animatedModelCount, get_time() - start_time);
    for (int i = 0; i < loadedModelsMaxIndex; ++i) {
        if (modelVertexCounts[i] == 0) continue;
        
        madvise(modelTriangles[i], modelTriangleCounts[i] * 3 * sizeof(uint32_t), MADV_DONTNEED);
    }
    
    cleanup_all_mmaps();
    DebugRAM("After Load Models");
}
