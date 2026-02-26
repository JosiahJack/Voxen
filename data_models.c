// data_models.c - Load 3D Models from .vmdl caches or .fbx via Assimp if cache invalid
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

GLuint SetupSSBO(GLuint* id, GLuint bindingIndex, GLsizeiptr size, const void* data, GLenum usage);
struct aiPropertyStore* props;

static bool LoadVMDL(const char *vmdl_path, uint64_t fbx_stamp, float **out_verts, uint32_t *out_vcount, uint32_t **out_idx, uint32_t *out_icount) {
    OsFileHandle fd; int st_size; uint8_t* map = OS_OpenAndAllocateFileBufferReadonly(vmdl_path, &fd, &st_size);
    if ((size_t)st_size < sizeof(uint64_t) + 4 + 4) return false;
    
    uint64_t file_stamp_on_disk;
    __builtin_memcpy(&file_stamp_on_disk, map, sizeof(uint64_t));
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
    return true;
}

void LoadModel(bool fromCache, uint16_t i, const char* fbx_path, const char* vmdl_path, uint64_t fbx_stamp, float* cached_verts, uint32_t cached_vcnt, uint32_t* cached_idx, uint32_t cached_icnt) {
    const struct aiScene* scene = NULL;
    if (!fromCache) {
        DualLog("No vmdl found or .fbx model was updated so needs refresh from .fbx source, loading %s with Assimp...\n", fbx_path);
        scene = aiImportFileExWithProperties(fbx_path, /*aiProcess_Triangulate*/ 0x8 | 0x800/*aiProcess_ImproveCacheLocality*/ | /*aiProcess_JoinIdenticalVertices*/ 0x2, NULL, props); // aiProcess vars from https://github.com/assimp/assimp/blob/672594c230832252f94bc90c19ca9ee9917be563/include/assimp/postprocess.h#L170
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
    if (!fromCache) {
        aiReleaseImport(scene);
        OsFileHandle fd = OS_OpenWriteonly(vmdl_path);
        size_t total = sizeof(uint64_t) + 4 + vertexCount*VERTEX_ATTRIBUTES_COUNT*sizeof(float) + 4 + triCount*3*sizeof(uint32_t);
        uint8_t *buf = OS_AllocateRAM(NULL, total, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, OS_INVALID_HANDLE);
        if (!buf) { OS_Close(fd); return; }

        uint8_t *p = buf;
        *(uint64_t *)p = fbx_stamp;
        p += sizeof(uint64_t);
        *(uint32_t*)p = vertexCount; p += 4;
        *(uint32_t*)p = triCount; p += 4;
        __builtin_memcpy(p, modelVertices[i], vertexCount*VERTEX_ATTRIBUTES_COUNT*sizeof(float)); p += vertexCount*VERTEX_ATTRIBUTES_COUNT*sizeof(float);
        __builtin_memcpy(p, modelTriangles[i], triCount*3*sizeof(uint32_t));
        OS_Write(fd, buf, total, vmdl_path);
        OS_DeallocateRAM(buf,total);
        OS_Close(fd);
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
    aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_READ_ANIMATIONS, 1); aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_READ_MATERIALS, 0);
    aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_READ_TEXTURES, 0);   aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_READ_LIGHTS, 0);
    aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_READ_CAMERAS, 0);    aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_OPTIMIZE_EMPTY_ANIMATION_CURVES, 1);
    aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_NO_SKELETON_MESHES, 0);  aiSetImportPropertyInteger(props, AI_CONFIG_PP_RVC_FLAGS, aiComponent_ANIMATIONS | aiComponent_BONEWEIGHTS);
    aiSetImportPropertyInteger(props, AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
    aiSetImportPropertyInteger(props, AI_CONFIG_PP_ICL_PTCACHE_SIZE, 16);       aiSetImportPropertyInteger(props, AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4);
    aiSetImportPropertyInteger(props, AI_CONFIG_PP_FD_REMOVE, 1);               aiSetImportPropertyInteger(props, AI_CONFIG_PP_PTV_KEEP_HIERARCHY, 0);
    DebugRAM("prior to model load loop");
    for (uint32_t i = 0; i < loadedModelsMaxIndex; ++i) {
        int32_t parserIdx = indexToParser[i];
        modelHasAnimation[i] = (model_parser.entries[parserIdx].entflags & ENTFLAG_ANIMATED);
        const char *fbx_path = model_parser.entries[parserIdx].path;
        if (!fbx_path || !fbx_path[0]) { DualLogError("No fbx path for model index %u\n", i); continue; }

        char vmdl_path[256];
        size_t fbx_path_sz = GetStringLength(fbx_path);
        StringCopyInto_A_SubstringFrom_B(vmdl_path, fbx_path_sz - 3, fbx_path, 256); // Chop off "fbx" or "obj" and then manually add "vmdl" terminating with \0 within the temp buffer.
        if (StringIsEmpty(vmdl_path)) { DualLogError("Invalid vmdl_path for %s: '%s'\n", fbx_path, vmdl_path); OS_Exit(1); }
        
        StringConcatenate(vmdl_path, "vmdl", 256); // Extension . separator was preserved above, so just add the letters part.
        FileFingerprint fp;
        if (!OS_GetFileFingerprint(fbx_path, &fp)) { DualLogError("File change detection failed for %s (%s)\n", fbx_path, vmdl_path); continue; }
        
        uint64_t fbx_stamp = OS_GetFilestamp(&fp);
        float *cached_verts = NULL; uint32_t cached_vcnt = 0, cached_icnt = 0; uint32_t *cached_idx = NULL; 
        bool cache_hit = LoadVMDL(vmdl_path, fbx_stamp, &cached_verts, &cached_vcnt, &cached_idx, &cached_icnt);
        LoadModel(cache_hit, i, fbx_path, vmdl_path, fbx_stamp, cached_verts, cached_vcnt, cached_idx, cached_icnt);        
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
        __builtin_memcpy(ptr, modelVertices[i], vertSize);
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Sys_Render.tbos[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)triSize, NULL, GL_STATIC_DRAW);
        ptr = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, (GLsizeiptr)triSize, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        __builtin_memcpy(ptr, modelTriangles[i], triSize);
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
        glFlush(); glFinish(); // Surprisingly also causes the LoadTextures OpenGL driver in Linux to drop its CPU side RAM duplicates earlier
    }
    
    DebugRAM("after to model to gpu transfer");
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glFlush(); glFinish();
    OS_DeallocateRAM(model_parser.entries,model_parser.count * sizeof(Entity));
    DualLog(" total vertices: %u, total tris: %u, took %f secs\n", totalVertices, totalTris, get_time() - start_time);
    DebugRAM("After Load Models");
}
