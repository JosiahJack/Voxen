#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <omp.h>
#include "voxen.h"
#define C_STRUCT struct // #include <assimp/defs.h>
#ifdef _WIN32 // #include <assimp/defs.h>
#  undef ASSIMP_API
#  ifdef ASSIMP_BUILD_DLL_EXPORT
#    define ASSIMP_API __declspec(dllexport)
#    define ASSIMP_API_WINONLY __declspec(dllexport)
#  elif (defined ASSIMP_DLL)
#    define ASSIMP_API __declspec(dllimport)
#    define ASSIMP_API_WINONLY __declspec(dllimport)
#  else
#    define ASSIMP_API
#    define ASSIMP_API_WINONLY
#  endif
#else
#  define ASSIMP_API __attribute__((visibility("default")))
#  define ASSIMP_API_WINONLY
#endif // _WIN32
struct aiFileIO; // #include <assimp/cimport.h>
ASSIMP_API C_STRUCT aiPropertyStore *aiCreatePropertyStore(void); // #include <assimp/cimport.h>
ASSIMP_API const C_STRUCT aiScene *aiImportFileExWithProperties(const char *pFile, unsigned int pFlags, C_STRUCT aiFileIO *pFS, const C_STRUCT aiPropertyStore *pProps); // #include <assimp/cimport.h>
ASSIMP_API void aiSetImportPropertyInteger(C_STRUCT aiPropertyStore *store, const char *szName, int value); // #include <assimp/cimport.h>
ASSIMP_API void aiReleasePropertyStore(C_STRUCT aiPropertyStore *p); // #include <assimp/cimport.h>
ASSIMP_API const char *aiGetErrorString(void); // #include <assimp/cimport.h>
ASSIMP_API void aiReleaseImport(const C_STRUCT aiScene *pScene); // #include <assimp/cimport.h>
#include <assimp/scene.h> // Only 514 lines and make use of most all of it so leaving unhoovered, unlike assimp/cimport.h.
int malloc_trim(size_t pad); // #include <malloc.h>
ssize_t write(int fd, const void *buf, size_t nbytes); // #include <unistd.h>
int close (int filedes); // #include <unistd.h>

DataParser model_parser;

// Models
uint32_t* modelVertexCounts = NULL;
uint32_t* modelTriangleCounts = NULL;
uint16_t* modelTypeCountsOpaque = NULL;
uint16_t* modelTypeCountsDoubleSided = NULL;
uint16_t* modelTypeCountsTransparent = NULL;
uint16_t invalidModelIndexCount;
uint16_t* modelTypeOffsetsOpaque = NULL;
uint16_t* modelTypeOffsetsDoubleSided = NULL;
uint16_t* modelTypeOffsetsTransparent = NULL;
uint16_t opaqueInstancesHead = 0;
float** modelVertices = NULL;
uint32_t** modelTriangles = NULL;
GLuint* vbos = NULL;
GLuint* tbos = NULL;
GLuint modelBoundsID;
float* modelBounds = NULL;
uint16_t renderableCount = 0;
uint16_t loadedInstances = 0;
uint16_t loadedModels = 0;
uint16_t loadedLights = 0;
uint16_t startOfDoubleSidedInstances = INSTANCE_COUNT - 1;
uint16_t startOfTransparentInstances = INSTANCE_COUNT - 1;
uint16_t doubleSidedInstancesHead = 0;
uint16_t transparentInstancesHead = 0;

//-----------------------------------------------------------------------------
// Loads all 3D meshes
static void make_vmdl_path(const char *fbx_path, char *out, size_t outsz) {
    // Find last slash
    const char *slash = strrchr(fbx_path, '/');
    const char *start = slash ? slash + 1 : fbx_path;

    // Find last dot in filename
    const char *dot = strrchr(start, '.');
    size_t name_len = dot ? (size_t)(dot - start) : (size_t)strlen(start);

    // Build path: directory + basename (without extension) + ".vmdl"
    size_t dir_len = start - fbx_path;
    if (dir_len + name_len + 6 >= outsz) {
        // Truncate safely
        memcpy(out, fbx_path, outsz - 6);
        out[outsz - 6] = '\0';
        char *last_slash = strrchr(out, '/');
        if (last_slash) {
            *++last_slash = '\0';
            strcat(out, "toolong.vmdl");
        } else {
            strcpy(out, "toolong.vmdl");
        }
        return;
    }

    // Copy directory + basename
    memcpy(out, fbx_path, dir_len + name_len);
    out[dir_len + name_len] = '\0';
    strcat(out, ".vmdl");
}

static bool load_vmdl(const char *vmdl_path, uint8_t expected_md5[16], float **out_verts, uint32_t *out_vcount, uint32_t **out_idx, uint32_t *out_icount, void** out_map, size_t* out_mapsz) {
    int fd = open(vmdl_path, O_RDONLY);
    if (fd < 0) return false;

    struct stat st;
    if (fstat(fd, &st) < 0) { close(fd); return false; }
    if (st.st_size < 16 + 4 + 4) { close(fd); return false; }
    uint8_t *map = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    if (map == MAP_FAILED) return false;

    if (memcmp(map, expected_md5, 16) != 0) { 
        munmap(map, st.st_size); 
        return false; 
    }

    const uint8_t *p = map + 16;
    uint32_t vcnt = *(uint32_t*)p; p += 4;
    *out_vcount = vcnt;
    uint32_t icnt = *(uint32_t*)p; p += 4;
    *out_icount = icnt;
    size_t vert_bytes = vcnt * VERTEX_ATTRIBUTES_COUNT * sizeof(float);
    size_t idx_bytes  = icnt * 3 * sizeof(uint32_t);
    size_t expected   = 16 + 4 + vert_bytes + 4 + idx_bytes;
    if (expected != (size_t)st.st_size) { DualLogError("vmdl corrupted: size %zu, expected %zu from vertex count %u and tri count %u\n", st.st_size, expected, vcnt, icnt); munmap(map, st.st_size); return false; }
    if (p + vert_bytes + idx_bytes > map + st.st_size) { DualLogError("vmdl data overflow\n"); munmap(map, st.st_size); return false; }

    *out_verts  = (float*)p;
    p += vert_bytes;
    *out_idx    = (uint32_t*)p;
    *out_map = map;
    *out_mapsz = st.st_size;
//     DualLog("vmdl loaded: v=%u, i=%u, vert_bytes=%zu, idx_bytes=%zu\n", vcnt, icnt, vert_bytes, idx_bytes);
    return true;
}

static void write_vmdl(const char *vmdl_path, const uint8_t md5[16], const float *verts, uint32_t vcnt, const uint32_t *triangleIndices, uint32_t triCount) {
    int fd = open(vmdl_path, O_WRONLY|O_CREAT|O_TRUNC, 0644);
    if (fd < 0) return;

    size_t total = 16 + 4 + vcnt*VERTEX_ATTRIBUTES_COUNT*sizeof(float) + 4 + triCount*3*sizeof(uint32_t);
    uint8_t *buf = malloc(total);
    if (!buf) { close(fd); return; }

    uint8_t *p = buf;
    memcpy(p, md5, 16); p += 16;
    *(uint32_t*)p = vcnt; p += 4;
    *(uint32_t*)p = triCount; p += 4;
    memcpy(p, verts, vcnt*VERTEX_ATTRIBUTES_COUNT*sizeof(float)); p += vcnt*VERTEX_ATTRIBUTES_COUNT*sizeof(float);
    memcpy(p, triangleIndices, triCount*3*sizeof(uint32_t));
    size_t written = write(fd, buf, total);
    if (written != (size_t)total) DualLogError("write_vmdl: partial write %zd/%zu\n", written, total);
    free(buf);
    close(fd);
}

typedef struct {
    void* ptr;
    size_t size;
} MMapEntry;

MMapEntry* mmap_cleanup = NULL;
int mmap_cleanup_count = 0;
int mmap_cleanup_capacity = 0;

void add_mmap_cleanup(void* ptr, size_t size) {
    #pragma omp critical(mmap_cleanup)
    {
        if (mmap_cleanup_count >= mmap_cleanup_capacity) {
            mmap_cleanup_capacity = mmap_cleanup_capacity ? mmap_cleanup_capacity * 2 : 256;
            mmap_cleanup = realloc(mmap_cleanup, mmap_cleanup_capacity * sizeof(MMapEntry));
            if (!mmap_cleanup) {
                DualLogError("realloc failed in add_mmap_cleanup\n");
                exit(1);
            }
        }
        mmap_cleanup[mmap_cleanup_count].ptr = ptr;
        mmap_cleanup[mmap_cleanup_count].size = size;
        mmap_cleanup_count++;
    }
}

void cleanup_all_mmaps(void) {
    for (int i = 0; i < mmap_cleanup_count; i++) {
        munmap(mmap_cleanup[i].ptr, mmap_cleanup[i].size);
    }
    free(mmap_cleanup);
    mmap_cleanup = NULL;
    mmap_cleanup_count = mmap_cleanup_capacity = 0;
}

void LoadModels(void) {
    double start_time = get_time();
    DebugRAM("start of LoadModels");
    loadedModels = 0;
    if (!parse_data_file(&model_parser, "./Data/models.txt")) { DualLogError("Could not parse ./Data/models.txt!\n"); exit(1); }

    int32_t maxIndex = -1;
    for (int32_t k = 0; k < model_parser.count; k++) {
        if (model_parser.entries[k].index > maxIndex && model_parser.entries[k].index != UINT16_MAX) maxIndex = model_parser.entries[k].index;
    }

    loadedModels = maxIndex + 1;
    DualLog("Loading   models( %d) with max index  %d ...", model_parser.count, maxIndex);
    int32_t totalVertCount = 0;
    int32_t totalTriCount = 0;
    modelVertexCounts   = calloc(loadedModels, sizeof(uint32_t));
    modelTriangleCounts = calloc(loadedModels, sizeof(uint32_t));
    modelVertices       = calloc(loadedModels, sizeof(float*));
    modelTriangles      = calloc(loadedModels, sizeof(uint32_t*));
    modelBounds         = calloc(loadedModels * BOUNDS_ATTRIBUTES_COUNT, sizeof(float));
    int32_t* indexToParser = calloc(loadedModels, sizeof(int32_t));
    for (int32_t k = 0; k < model_parser.count; k++) {
        if (model_parser.entries[k].index != UINT16_MAX) {
            indexToParser[model_parser.entries[k].index] = k;
        }
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
    #pragma omp parallel default(none) \
        shared(model_parser, indexToParser, loadedModels, \
            modelVertexCounts, modelTriangleCounts, \
            modelVertices, modelTriangles, modelBounds, \
            props, totalVertCount, totalTriCount)
    {
        #pragma omp for schedule(dynamic)
        for (uint32_t i = 0; i < loadedModels; ++i) {
            int32_t parserIdx = indexToParser[i];
            const char *fbx_path = model_parser.entries[parserIdx].path;
            if (!fbx_path || !fbx_path[0]) continue;

            /* ---------- 1. Build .vmdl name ---------- */
            char vmdl_path[512];
            make_vmdl_path(fbx_path, vmdl_path, sizeof(vmdl_path));
            if (!vmdl_path[0] || strcmp(vmdl_path, ".vmdl") == 0 || vmdl_path[0] == '.') { DualLogError("Invalid vmdl_path for %s: '%s'\n", fbx_path, vmdl_path); exit(1); }

            /* ---------- 2. Compute MD5 of the .fbx ---------- */
            uint8_t fbx_md5[16];
            {
                FILE *f = fopen(fbx_path, "rb");
                if (!f) { DualLogError("Cannot open %s for MD5\n", fbx_path); continue; }
                fseek(f, 0, SEEK_END);
                long sz = ftell(f); fseek(f, 0, SEEK_SET);
                uint8_t *buf = malloc(sz);
                size_t read = fread(buf, 1, sz, f);
                if (read != (size_t)sz) { DualLogError("Failed to read full FBX: %s\n", fbx_path); exit(1); }
                    
                fclose(f);
                md5(buf, sz, fbx_md5);
                free(buf);
            }

            /* ---------- 3. Try to load cached .vmdl ---------- */
            float  *cached_verts = NULL;
            uint32_t cached_vcnt = 0;
            uint32_t *cached_idx  = NULL;
            uint32_t cached_icnt = 0;
            void* mmap_map = NULL;
            size_t mmap_size = 0;
            bool cache_hit = load_vmdl(vmdl_path, fbx_md5, &cached_verts, &cached_vcnt, &cached_idx,  &cached_icnt, &mmap_map, &mmap_size);

            /* ---------- 4. If cache miss – run Assimp ---------- */
            if (!cache_hit) {
                DualLog("No vmdl found or .fbx model was updated so needs refresh from .fbx source, loading %s with Assimp...\n", fbx_path);
                const struct aiScene *scene = aiImportFileExWithProperties(fbx_path, /*aiProcess_GenNormals*/ 0x20 | 0x800/*aiProcess_ImproveCacheLocality*/, NULL, props); // aiProcess vars from https://github.com/assimp/assimp/blob/672594c230832252f94bc90c19ca9ee9917be563/include/assimp/postprocess.h#L170
                if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) { DualLogError("Assimp failed %s: %s\n", fbx_path, aiGetErrorString()); continue; }

                /* ---- count verts / tris ---- */
                uint32_t vertexCount = 0, triCount = 0;
                for (uint32_t m = 0; m < scene->mNumMeshes; ++m) {
                    vertexCount += scene->mMeshes[m]->mNumVertices;
                    triCount    += scene->mMeshes[m]->mNumFaces;
                }
                
                if (vertexCount > MAX_VERT_COUNT || triCount > MAX_TRI_COUNT) { DualLogError("Model %s exceeds limits\n", fbx_path); aiReleaseImport(scene); continue; }

                modelVertexCounts[i]   = vertexCount;
                modelTriangleCounts[i] = triCount;

                modelVertices[i]  = calloc(vertexCount * VERTEX_ATTRIBUTES_COUNT, sizeof(float));
                modelTriangles[i] = calloc(triCount * 3, sizeof(uint32_t));

                /* ---- fill vertex / index arrays (same code you already have) ---- */
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
                        float u = mesh->mTextureCoords[0] ? mesh->mTextureCoords[0][vert].x : 0.0f;
                        float v = mesh->mTextureCoords[0] ? mesh->mTextureCoords[0][vert].y : 0.0f;
                        modelVertices[i][vertexIndex++] = u;
                        modelVertices[i][vertexIndex++] = v;
                        minx = fminf(minx, mesh->mVertices[vert].x);
                        maxx = fmaxf(maxx, mesh->mVertices[vert].x);
                        miny = fminf(miny, mesh->mVertices[vert].y);
                        maxy = fmaxf(maxy, mesh->mVertices[vert].y);
                        minz = fminf(minz, mesh->mVertices[vert].z);
                        maxz = fmaxf(maxz, mesh->mVertices[vert].z);
                    }

                    for (uint32_t f = 0; f < mesh->mNumFaces; ++f) {
                        struct aiFace *face = &mesh->mFaces[f];
                        if (face->mNumIndices != 3) {
                            DualLogError("Non-tri face in %s\n", fbx_path);
                            aiReleaseImport(scene);
                            continue;
                        }
                        uint32_t a = face->mIndices[0] + globalVertexOffset;
                        uint32_t b = face->mIndices[1] + globalVertexOffset;
                        uint32_t c = face->mIndices[2] + globalVertexOffset;
                        modelTriangles[i][triangleIndex++] = a;
                        modelTriangles[i][triangleIndex++] = b;
                        modelTriangles[i][triangleIndex++] = c;
                    }
                    globalVertexOffset += mesh->mNumVertices;
                }

                /* ---- bounds ---- */
                uint32_t base = i * BOUNDS_ATTRIBUTES_COUNT;
                modelBounds[base + BOUNDS_DATA_OFFSET_MINX] = minx;
                modelBounds[base + BOUNDS_DATA_OFFSET_MINY] = miny;
                modelBounds[base + BOUNDS_DATA_OFFSET_MINZ] = minz;
                modelBounds[base + BOUNDS_DATA_OFFSET_MAXX] = maxx;
                modelBounds[base + BOUNDS_DATA_OFFSET_MAXY] = maxy;
                modelBounds[base + BOUNDS_DATA_OFFSET_MAXZ] = maxz;
                float r = 0.0f;
                r = fmaxf(r, fabsf(minx)); r = fmaxf(r, fabsf(miny)); r = fmaxf(r, fabsf(minz));
                r = fmaxf(r, maxx);        r = fmaxf(r, maxy);        r = fmaxf(r, maxz);
                modelBounds[base + BOUNDS_DATA_OFFSET_RADIUS] = r;
                write_vmdl(vmdl_path, fbx_md5, modelVertices[i], vertexCount, modelTriangles[i], triCount);
                aiReleaseImport(scene);
            } else {
                /* ---- CACHE HIT ---- */
                modelVertexCounts[i]   = cached_vcnt;
                modelTriangleCounts[i] = cached_icnt;
                modelVertices[i]  = malloc(cached_vcnt * VERTEX_ATTRIBUTES_COUNT * sizeof(float));
                modelTriangles[i] = malloc(cached_icnt * 3 * sizeof(uint32_t));
                memcpy(modelVertices[i],  cached_verts, cached_vcnt * VERTEX_ATTRIBUTES_COUNT * sizeof(float));
                memcpy(modelTriangles[i], cached_idx,  cached_icnt * 3 * sizeof(uint32_t));
                float minx = 1E9f, miny = 1E9f, minz = 1E9f;
                float maxx = -1E9f, maxy = -1E9f, maxz = -1E9f;
                for (uint32_t v = 0; v < cached_vcnt; ++v) {
                    float x = cached_verts[v*VERTEX_ATTRIBUTES_COUNT + 0];
                    float y = cached_verts[v*VERTEX_ATTRIBUTES_COUNT + 1];
                    float z = cached_verts[v*VERTEX_ATTRIBUTES_COUNT + 2];
                    minx = fminf(minx, x); maxx = fmaxf(maxx, x);
                    miny = fminf(miny, y); maxy = fmaxf(maxy, y);
                    minz = fminf(minz, z); maxz = fmaxf(maxz, z);
                }
                uint32_t base = i * BOUNDS_ATTRIBUTES_COUNT;
                modelBounds[base + BOUNDS_DATA_OFFSET_MINX] = minx;
                modelBounds[base + BOUNDS_DATA_OFFSET_MINY] = miny;
                modelBounds[base + BOUNDS_DATA_OFFSET_MINZ] = minz;
                modelBounds[base + BOUNDS_DATA_OFFSET_MAXX] = maxx;
                modelBounds[base + BOUNDS_DATA_OFFSET_MAXY] = maxy;
                modelBounds[base + BOUNDS_DATA_OFFSET_MAXZ] = maxz;
                float r = 0.0f;
                r = fmaxf(r, fabsf(minx)); r = fmaxf(r, fabsf(miny)); r = fmaxf(r, fabsf(minz));
                r = fmaxf(r, maxx);        r = fmaxf(r, maxy);        r = fmaxf(r, maxz);
                modelBounds[base + BOUNDS_DATA_OFFSET_RADIUS] = r;
                add_mmap_cleanup(mmap_map, mmap_size);  // defer munmap
            }

            /* ---- atomic totals (debug) ---- */
            #pragma omp critical
            {
                totalVertCount += modelVertexCounts[i];
                totalTriCount  += modelTriangleCounts[i];
            }
        }
        
        #pragma omp barrier
        #pragma omp master
        {
            cleanup_all_mmaps();
        }
    }

    aiReleasePropertyStore(props);
    malloc_trim(0);
    vbos = calloc(loadedModels, sizeof(GLuint));
    tbos = calloc(loadedModels, sizeof(GLuint));
    glGenBuffers(loadedModels, vbos);
    glGenBuffers(loadedModels, tbos);
    for (int i = 0; i < loadedModels; ++i) {
        if (modelVertexCounts[i] == 0) continue;

        size_t vertSize = modelVertexCounts[i] * VERTEX_ATTRIBUTES_COUNT * sizeof(float);
        size_t triSize  = modelTriangleCounts[i] * 3 * sizeof(uint32_t);
        glBindBuffer(GL_ARRAY_BUFFER, vbos[i]);
        glBufferData(GL_ARRAY_BUFFER, vertSize, NULL, GL_STATIC_DRAW);  // orphan
        glBufferData(GL_ARRAY_BUFFER, vertSize, NULL, GL_STATIC_DRAW);  // safe
        void* ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, vertSize, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        memcpy(ptr, modelVertices[i], vertSize);
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tbos[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, triSize, NULL, GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, triSize, NULL, GL_STATIC_DRAW);
        ptr = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, triSize, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        memcpy(ptr, modelTriangles[i], triSize);
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

#ifdef DEBUG_MODEL_LOAD_DATA
    DualLog("Total vertices: %d (", totalVertCount); print_bytes_no_newline(totalVertCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float)); DualLog(")\nTotal triangles: %d (", totalTriCount); print_bytes_no_newline(totalTriCount * 3 * sizeof(uint32_t)); DualLog(")\n");
#endif

    glGenBuffers(1, &modelBoundsID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelBoundsID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, loadedModels * BOUNDS_ATTRIBUTES_COUNT * sizeof(float), modelBounds, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, modelBoundsID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    DualLog(" took %f seconds\n", get_time() - start_time);
    DebugRAM("After Load Models");
    free(indexToParser);
    malloc_trim(0);
}
