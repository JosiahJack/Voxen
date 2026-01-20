// data_models.c - Load 3D Models from .vmdl caches or .fbx via Assimp if cache invalid
DataParser model_parser;
float** modelVertices = NULL;
uint32_t** modelTriangles = NULL;
uint32_t modelVertexCounts[MODEL_IDX_MAX] = {0}; // 4kb
uint32_t modelTriangleCounts[MODEL_IDX_MAX] = {0}; // 4kb
bool modelHasAnimation[MODEL_IDX_MAX] = {0}; // 1kb
float modelBounds[MODEL_IDX_MAX * BOUNDS_ATTRIBUTES_COUNT] = {0}; // 1024 * 7 * 4 = 28.6kb
float modelBoundingRadii[MODEL_IDX_MAX];
uint16_t loadedModelsMaxIndex = 0;
AnimationClip modelAnimationClips[MAX_ANIMATED_MODELS][MAX_ANIMATION_CLIPS_PER_MODEL];
GLuint SetupSSBO(GLuint* id, GLuint bindingIndex, GLsizeiptr size, const void* data, GLenum usage);

static void make_vmdl_path(const char *fbx_path, char *out, size_t outsz) {
    strncpy(out, fbx_path, outsz - 1);
    out[outsz - 1] = '\0';
    char *ext = strrchr(out, '.');
    if (ext && strcmp(ext, ".fbx") == 0) strncpy(ext, ".vmdl", outsz - (size_t)(ext - out) - 1);
    else if (strlen(out) + 5 < outsz) strcat(out, ".vmdl");
}

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
    for (int i = 0; i < mmap_cleanup_count; i++) OS_DeallocateRAM(mmap_cleanup[i].ptr, mmap_cleanup[i].size);
}

uint16_t GetHalfFloatAsU16(float base, float offsetted) { return (uint16_t)(vmax(vmin(((offsetted - base) * 1000.0f) + 32768.0f,65535.0f),0.0f) + 0.5f); } // Scale of 1000 gives ±32.767 units range

float deltaAnimationBasePositions[MAX_ANIMATED_MODELS][5000 * 3];
uint16_t deltaAnimationTables[MAX_ANIMATED_MODELS * 5000 * 3]; // Probably overkill, but maybe not, half are NPCs.
uint32_t deltaOffsets[MAX_ANIMATED_MODELS];
uint32_t currentAnimOffsetHead;
struct aiPropertyStore* props;
void LoadModel(uint16_t i, uint16_t animNum, uint16_t numFrames, uint16_t numFramesRemaining, const char* fbx_path, const char* vmdl_path, uint64_t fbx_stamp) {
    const struct aiScene *scene = aiImportFileExWithProperties(fbx_path, /*aiProcess_Triangulate*/ 0x8 | /*aiProcess_GenNormals*/ 0x20 | 0x800/*aiProcess_ImproveCacheLocality*/ | /*aiProcess_JoinIdenticalVertices*/ 0x2, NULL, props); // aiProcess vars from https://github.com/assimp/assimp/blob/672594c230832252f94bc90c19ca9ee9917be563/include/assimp/postprocess.h#L170
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) { DualLogError("Assimp failed %s: %s\n", fbx_path, aiGetErrorString()); return; }

    uint32_t vertexCount = 0, triCount = 0;
    for (uint32_t m = 0; m < scene->mNumMeshes; ++m) { vertexCount += scene->mMeshes[m]->mNumVertices;  triCount += scene->mMeshes[m]->mNumFaces; }
    if (numFramesRemaining > 0u && numFramesRemaining != numFrames && animNum != UINT16_MAX && false) { // Parse into animation deltas table for this model, don't treate like a normal model.
        uint32_t vertexIndex = 0;
        deltaOffsets[animNum] = currentAnimOffsetHead;
        for (uint32_t m = 0; m < scene->mNumMeshes; ++m) {
            struct aiMesh *mesh = scene->mMeshes[m];
            for (uint32_t vert = 0; vert < mesh->mNumVertices; ++vert) { // Identical order expected for all .obj files in animation sequence so this should be safe.
                deltaAnimationTables[currentAnimOffsetHead + vertexIndex++] = GetHalfFloatAsU16(deltaAnimationBasePositions[animNum][(vert * 3) + 0], mesh->mVertices[vert].x);
                deltaAnimationTables[currentAnimOffsetHead + vertexIndex++] = GetHalfFloatAsU16(deltaAnimationBasePositions[animNum][(vert * 3) + 1], mesh->mVertices[vert].y);
                deltaAnimationTables[currentAnimOffsetHead + vertexIndex++] = GetHalfFloatAsU16(deltaAnimationBasePositions[animNum][(vert * 3) + 2], mesh->mVertices[vert].z);
            }
            
            // Disregard bounds changes from animations, radii are padded in main render loop anyway and it's such a minor thing I'm not going to worry about it.
            currentAnimOffsetHead += mesh->mNumVertices;
        }
        
        aiReleaseImport(scene);
        return;
    }

    modelVertexCounts[i]   = vertexCount;
    modelTriangleCounts[i] = triCount;
    modelVertices[i]  = OS_AllocateRAM(NULL, vertexCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
    modelTriangles[i] =  OS_AllocateRAM(NULL, triCount * 3 * sizeof(uint32_t), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
    uint32_t vertexIndex = 0, triangleIndex = 0, globalVertexOffset = 0;
    float minx = 1E9f, miny = 1E9f, minz = 1E9f;
    float maxx = -1E9f, maxy = -1E9f, maxz = -1E9f;
    for (uint32_t m = 0; m < scene->mNumMeshes; ++m) {
        struct aiMesh *mesh = scene->mMeshes[m];
        for (uint32_t vert = 0; vert < mesh->mNumVertices; ++vert) {
            modelVertices[i][vertexIndex++] = mesh->mVertices[vert].x;
            modelVertices[i][vertexIndex++] = mesh->mVertices[vert].y;
            modelVertices[i][vertexIndex++] = mesh->mVertices[vert].z;
            if (numFramesRemaining > 0u && animNum != UINT16_MAX) { // First animation, get base pose
                if (vert > 5000) { DualLogWarn("Animated mesh %s has more than 5000 verts!\n", fbx_path); continue; }
                
                deltaAnimationBasePositions[animNum][(vert * 3) + 0] = mesh->mVertices[vert].x;
                deltaAnimationBasePositions[animNum][(vert * 3) + 1] = mesh->mVertices[vert].y;
                deltaAnimationBasePositions[animNum][(vert * 3) + 2] = mesh->mVertices[vert].z;
            }
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
            if (face->mNumIndices != 3) { DualLogError("Non-tri face in %s\n", fbx_path); continue; }
            
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
    modelBoundingRadii[i] = r;
    write_vmdl(vmdl_path, fbx_stamp, modelVertices[i], vertexCount, modelTriangles[i], triCount);
    aiReleaseImport(scene);
}

void LoadVMDL(uint16_t i, uint16_t animNum, uint16_t numFrames, uint16_t numFramesRemaining, float* cached_verts, uint32_t cached_vcnt, uint32_t* cached_idx, uint32_t cached_icnt, void* mmap_map, size_t mmap_size) {
    if (numFramesRemaining > 0u && numFramesRemaining != numFrames && animNum != UINT16_MAX && false) { // Parse into animation deltas table for this model, don't treate like a normal model.
        uint32_t vertexIndex = 0;
        deltaOffsets[animNum] = currentAnimOffsetHead;
        for (uint32_t v = 0; v < cached_vcnt; ++v) { // Identical order expected for all .obj files in animation sequence so this should be safe.
            float x = cached_verts[v*VERTEX_ATTRIBUTES_COUNT + 0];
            float y = cached_verts[v*VERTEX_ATTRIBUTES_COUNT + 1];
            float z = cached_verts[v*VERTEX_ATTRIBUTES_COUNT + 2];
            deltaAnimationTables[currentAnimOffsetHead + vertexIndex++] = GetHalfFloatAsU16(deltaAnimationBasePositions[animNum][(v * 3) + 0], x);
            deltaAnimationTables[currentAnimOffsetHead + vertexIndex++] = GetHalfFloatAsU16(deltaAnimationBasePositions[animNum][(v * 3) + 1], y);
            deltaAnimationTables[currentAnimOffsetHead + vertexIndex++] = GetHalfFloatAsU16(deltaAnimationBasePositions[animNum][(v * 3) + 2], z);
        }
         
        currentAnimOffsetHead+=cached_vcnt;
        // Disregard bounds changes from animations, radii are padded in main render loop anyway and it's such a minor thing I'm not going to worry about it.
        return;
    }

    modelVertexCounts[i]   = cached_vcnt;
    modelTriangleCounts[i] = cached_icnt;
    modelVertices[i]  = (float*)cached_verts;
    modelTriangles[i] = (uint32_t*)cached_idx;
    float minx = 1E9f, miny = 1E9f, minz = 1E9f;
    float maxx = -1E9f, maxy = -1E9f, maxz = -1E9f;
    for (uint32_t vert = 0; vert < cached_vcnt; ++vert) {
        float x = cached_verts[(vert * VERTEX_ATTRIBUTES_COUNT) + 0];
        float y = cached_verts[(vert * VERTEX_ATTRIBUTES_COUNT) + 1];
        float z = cached_verts[(vert * VERTEX_ATTRIBUTES_COUNT) + 2];
        minx = vmin(minx, x); maxx = vmax(maxx, x);
        miny = vmin(miny, y); maxy = vmax(maxy, y);
        minz = vmin(minz, z); maxz = vmax(maxz, z);
        if (numFramesRemaining > 0u && animNum != UINT16_MAX) { // First animation, get base pose
            if (vert > 5000) { DualLogWarn("Animated mesh %u [anim %u] has more than 5000 verts!\n", i, animNum); continue; }
            
            deltaAnimationBasePositions[animNum][(vert * 3) + 0] = x;
            deltaAnimationBasePositions[animNum][(vert * 3) + 1] = y;
            deltaAnimationBasePositions[animNum][(vert * 3) + 2] = z;
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
    r = vmax(r, maxx);        r = vmax(r, maxy);        r = vmax(r, maxz);
    modelBounds[base + BOUNDS_DATA_OFFSET_RADIUS] = r;
    add_mmap_cleanup(mmap_map, mmap_size);  // defer munmap
}

// uint8_t modelFBX_FileBuffer[15360000]; // 14983372 found in practice
void LoadModels(void) {
    double start_time = get_time();    
    if (loadedModelsMaxIndex > 0) {
        #ifdef ONLY_LOAD_LEVEL_NEEDS
            glDeleteBuffers(loadedModelsMaxIndex, Sys_Render.vbos);
            glDeleteBuffers(loadedModelsMaxIndex, Sys_Render.tbos);
            memset(modelVertexCounts, 0, MODEL_IDX_MAX * sizeof(uint32_t));
            memset(modelTriangleCounts, 0, MODEL_IDX_MAX * sizeof(uint32_t));
            memset(modelHasAnimation, 0, MODEL_IDX_MAX * sizeof(uint8_t));
            memset(modelBounds, 0, MODEL_IDX_MAX * BOUNDS_ATTRIBUTES_COUNT * sizeof(float));
            memset(modelBoundingRadii, 0, MODEL_IDX_MAX * sizeof(float));
            loadedModelsMaxIndex = 0;
        #else
            return;
        #endif
    }
    
    uint16_t animatedModelCount = 0u, numFramesRemaining = 0u, animNum = UINT16_MAX, numFrames = 0;
    if (!parse_data_file(&model_parser, "./Data/models.txt")) { DualLogError("Could not parse ./Data/models.txt!\n"); OS_Exit(1); }

    int32_t maxIndex = -1;
    for (uint32_t k = 0; k < model_parser.count; k++) {
        if (model_parser.entries[k].index > maxIndex && model_parser.entries[k].index != UINT16_MAX) maxIndex = model_parser.entries[k].index;
    }

    loadedModelsMaxIndex = (uint16_t)maxIndex + 1U;
    #ifdef ONLY_LOAD_LEVEL_NEEDS
        uint16_t actualLoadedModels = 0u;
        for (int32_t i=0;i<MODEL_IDX_MAX;++i) actualLoadedModels += modelIndexUsedForCurrentLevel[i] ? 1u : 0u;
        DualLog("Loading   models( %d/%d) with max index  %d ...", actualLoadedModels, model_parser.count, maxIndex);
    #else
        DualLog("Loading   models( %d/%d) with max index  %d ...", loadedModelsMaxIndex, model_parser.count, maxIndex);
    #endif
    
    modelVertices       = OS_AllocateRAM(NULL, loadedModelsMaxIndex * sizeof(float*), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
    modelTriangles      = OS_AllocateRAM(NULL, loadedModelsMaxIndex * sizeof(uint32_t*), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
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
        #ifdef ONLY_LOAD_LEVEL_NEEDS
            if (!modelIndexUsedForCurrentLevel[parserIdx] && numFramesRemaining <= 0u) continue;
        #endif
        
        modelHasAnimation[i] = (model_parser.entries[parserIdx].entflags & ENTFLAG_ANIMATED);
        numFrames = model_parser.entries[parserIdx].frames;
        const char *fbx_path = model_parser.entries[parserIdx].path;
        if (modelHasAnimation[i] && numFramesRemaining <= 0u) { 
            numFramesRemaining = numFrames;
            animNum = model_parser.entries[parserIdx].animationNum;
            animatedModelCount++;
            DualLog("Loading animated model %s with %u frames\n", fbx_path, numFramesRemaining);
        }
        if (!fbx_path || !fbx_path[0]) { DualLogError("No fbx path for model index %u\n", i); OS_Exit(1); }

        char vmdl_path[512];
        make_vmdl_path(fbx_path, vmdl_path, sizeof(vmdl_path));
        if (!vmdl_path[0] || strcmp(vmdl_path, ".vmdl") == 0 || vmdl_path[0] == '.') { DualLogError("Invalid vmdl_path for %s: '%s'\n", fbx_path, vmdl_path); OS_Exit(1); }

        FileFingerprint fp;
        if (!OS_GetFileFingerprint(fbx_path, &fp)) { DualLogError("File change detection failed for %s\n", fbx_path); continue; }
        
        uint64_t fbx_stamp = OS_GetFilestamp(&fp);
        float  *cached_verts = NULL; uint32_t cached_vcnt = 0; uint32_t *cached_idx  = NULL; uint32_t cached_icnt = 0; void* mmap_map = NULL; size_t mmap_size = 0;
        bool cache_hit = load_vmdl(vmdl_path, fbx_stamp, &cached_verts, &cached_vcnt, &cached_idx,  &cached_icnt, &mmap_map, &mmap_size);
        if (!cache_hit) { DualLog("No vmdl found or .fbx model was updated so needs refresh from .fbx source, loading %s with Assimp...\n", fbx_path); LoadModel(i, animNum, numFramesRemaining, numFrames, fbx_path, vmdl_path, fbx_stamp); }
        else LoadVMDL(i, animNum, numFramesRemaining, numFrames, cached_verts, cached_vcnt, cached_idx, cached_icnt, mmap_map, mmap_size); // Use existing .vmdl binary RAM blob (aka a cache hit was successful)
        
        if (numFramesRemaining > 0u) {
            --numFramesRemaining;
            if (numFramesRemaining <= 0u) animNum = UINT16_MAX;
        }
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
        glFlush(); // Surprisingly also causes the LoadTextures OpenGL driver in Linux to drop its CPU side RAM duplicates earlier
        glFinish();
    }
    
    DebugRAM("after to model to gpu transfer");
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glFlush();
    glFinish();
    DualLog(" total vertices: %u, total tris: %u, animated models %u, %u final anim offset, took %f secs\n", totalVertices, totalTris, animatedModelCount, currentAnimOffsetHead, get_time() - start_time);
    Sys_Render.modelAnimDeltasID       = SetupSSBO(&Sys_Render.modelAnimDeltasID,        2, currentAnimOffsetHead * 3 * sizeof(uint16_t), deltaAnimationTables, GL_STATIC_DRAW);
    Sys_Render.modelAnimDeltaOffsetsID = SetupSSBO(&Sys_Render.modelAnimDeltaOffsetsID,  3, MAX_ANIMATED_MODELS * sizeof(uint32_t), deltaOffsets, GL_STATIC_DRAW);
    cleanup_all_mmaps(); // Uggggh, can't without losing mesh collision support at the moment.
    DebugRAM("After Load Models");
}
