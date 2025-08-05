#include <malloc.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <math.h>
#include "constants.h"
#include "data_models.h"
#include "debug.h"
#include "render.h"
#include "data_parser.h"
#include "voxel.h"
#include "event.h"

// #define DEBUG_MODEL_LOAD_DATA 1U

DataParser model_parser;
const char *valid_mdldata_keys[] = {"index"};
#define NUM_MODEL_KEYS 1
uint32_t modelVertexCounts[MODEL_COUNT];
uint32_t modelTriangleCounts[MODEL_COUNT];
uint32_t modelEdgeCounts[MODEL_COUNT];
GLuint vbos[MODEL_COUNT];
GLuint tbos[MODEL_COUNT]; // Triangle index buffers
GLuint tebos[MODEL_COUNT]; // Triangle's edge indices buffers
GLuint ebos[MODEL_COUNT]; // Edge index buffers
GLuint modelBoundsID;
float modelBounds[MODEL_COUNT * BOUNDS_ATTRIBUTES_COUNT];
uint32_t largestVertCount = 0;
uint32_t largestTriangleCount = 0;
uint32_t largestEdgeCount = 0;
GLuint vboMasterTable;
GLuint modelVertexOffsetsID;
GLuint modelVertexCountsID;
GLuint sphoxelsID;

float * tempVertices;
uint32_t * tempTriangles;
uint32_t * tempTriEdges;
Edge * tempEdges;
EdgeHashEntry * edgeHash;
float ** vertexDataArrays;
uint32_t ** triangleDataArrays;
uint32_t ** triEdgeDataArrays;
uint32_t ** edgeDataArrays;

#define HASH_SIZE 65536 // Power of 2 for fast modulo
#define HASH(v0, v1) (((v0 * 31 + v1) ^ (v1 * 17)) & (HASH_SIZE - 1))
#define MAX_VERT_COUNT 25000
#define MAX_TRI_COUNT 32767
#define MAX_EDGE_COUNT 100000

// Loads all 3D meshes
int LoadGeometry(void) {
    CHECK_GL_ERROR();
    double start_time = get_time();

    parser_init(&model_parser, valid_mdldata_keys, NUM_MODEL_KEYS, PARSER_DATA); // First parse ./Data/models.txt to see what to load to what indices
    if (!parse_data_file(&model_parser, "./Data/models.txt")) { DualLogError("Could not parse ./Data/models.txt!\n"); parser_free(&model_parser); return 1; }

    int maxIndex = -1;
    for (int k=0;k<model_parser.count;k++) {
        if (model_parser.entries[k].index > maxIndex && model_parser.entries[k].index != UINT16_MAX) maxIndex = model_parser.entries[k].index;
    }

    DualLog("Parsing %d models...\n",model_parser.count);
    int totalVertCount = 0;
    int totalBounds = 0;
    int totalTriCount = 0;
    int totalEdgeCount = 0;
    largestVertCount = 0;
    largestTriangleCount = 0;
    largestEdgeCount = 0;
    
    // Allocate persistent temporary buffers
    tempVertices = (float *)malloc(MAX_VERT_COUNT * VERTEX_ATTRIBUTES_COUNT * sizeof(float));
    if (!tempVertices) { DualLogError("Failed to allocate tempVertices buffer\n"); return 1; }
    DebugRAM("tempVertices buffer");
    
    tempTriangles = (uint32_t *)malloc(MAX_TRI_COUNT * 3 * sizeof(uint32_t));
    if (!tempTriangles) { DualLogError("Failed to allocate tempTriangles temporary buffer\n"); CleanupModelLoad(true); return 1; }
    DebugRAM("tempTriangles buffer");

    tempTriEdges = (uint32_t *)malloc(MAX_TRI_COUNT * 3 * sizeof(uint32_t));
    if (!tempTriEdges) { DualLogError("Failed to allocate tempTriEdges temporary buffer\n"); CleanupModelLoad(true); return 1; }
    DebugRAM("tempTriEdges buffer");
    
    tempEdges = (Edge *)calloc(MAX_EDGE_COUNT, sizeof(Edge));
    if (!tempEdges) { DualLogError("Failed to allocate tempEdges temporary buffer\n"); CleanupModelLoad(true); return 1; }
    DebugRAM("tempEdges buffer");
    
    edgeHash = (EdgeHashEntry *)calloc(HASH_SIZE, sizeof(EdgeHashEntry));
    if (!edgeHash) { DualLogError("Failed to allocate edgeHash temporary buffer\n"); CleanupModelLoad(true); return 1; }
    DebugRAM("edgeHash buffer");

    vertexDataArrays = (float **)calloc(MODEL_COUNT, sizeof(float *));
    if (!vertexDataArrays) { DualLogError("Failed to allocate vertexDataArrays\n"); CleanupModelLoad(true); return 1; }
    DebugRAM("vertexDataArrays buffer");

    triangleDataArrays = (uint32_t **)calloc(MODEL_COUNT, sizeof(uint32_t *));
    if (!triangleDataArrays) { DualLogError("Failed to allocate triangleDataArrays\n"); CleanupModelLoad(true); return 1; }
    DebugRAM("triangleDataArrays buffer");
    
    triEdgeDataArrays = (uint32_t **)calloc(MODEL_COUNT, sizeof(uint32_t *));
    if (!triEdgeDataArrays) { DualLogError("Failed to allocate triEdgeDataArrays\n"); CleanupModelLoad(true); return 1; }
    DebugRAM("triEdgeDataArrays buffer");
    
    edgeDataArrays = (uint32_t **)calloc(MODEL_COUNT, sizeof(uint32_t *));
    if (!edgeDataArrays) { DualLogError("Failed to allocate edgeDataArrays\n"); CleanupModelLoad(true); return 1; }
    DebugRAM("edgeDataArrays buffer");
    
    // Generate staging buffers
    GLuint stagingVBO, stagingTBO, stagingTEBO, stagingEBO;
    glGenBuffers(1, &stagingVBO);
    glGenBuffers(1, &stagingTBO);
    glGenBuffers(1, &stagingTEBO);
    glGenBuffers(1, &stagingEBO);
    glBindBuffer(GL_ARRAY_BUFFER, stagingVBO);
    glBufferData(GL_ARRAY_BUFFER, MAX_VERT_COUNT * VERTEX_ATTRIBUTES_COUNT * sizeof(float), NULL, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stagingTBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_TRI_COUNT * 3 * sizeof(uint32_t), NULL, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stagingTEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_TRI_COUNT * 3 * sizeof(uint32_t), NULL, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stagingEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_EDGE_COUNT * 4 * sizeof(uint32_t), NULL, GL_DYNAMIC_COPY);
    CHECK_GL_ERROR();
    DebugRAM("after staging buffers allocation");
    
    for (uint32_t i = 0; i < MODEL_COUNT; i++) {
        int matchedParserIdx = -1;
        for (int k=0;k<model_parser.count;k++) {
            if (model_parser.entries[k].index == i) {matchedParserIdx = k; break; }
        }

        if (matchedParserIdx < 0) continue;
        if (!model_parser.entries[matchedParserIdx].path || model_parser.entries[matchedParserIdx].path[0] == '\0') continue;

        struct aiPropertyStore* props = aiCreatePropertyStore(); // Disable non-essential FBX components
        aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_READ_ANIMATIONS, 0); // Disable animations
        aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_READ_MATERIALS, 0); // Disable materials
        aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_READ_TEXTURES, 0); // Disable textures
        aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_READ_LIGHTS, 0); // Disable lights
        aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_READ_CAMERAS, 0); // Disable cameras
        aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_OPTIMIZE_EMPTY_ANIMATION_CURVES, 1); // Drop empty animations
        aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_NO_SKELETON_MESHES, 1); // Disable skeleton meshes
        aiSetImportPropertyInteger(props, AI_CONFIG_PP_RVC_FLAGS, aiComponent_ANIMATIONS | aiComponent_BONEWEIGHTS | aiComponent_MATERIALS | aiComponent_TEXTURES | aiComponent_LIGHTS | aiComponent_CAMERAS); // Remove non-mesh components
        aiSetImportPropertyInteger(props, AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT); // Skip non-triangular primitives
        aiSetImportPropertyInteger(props, AI_CONFIG_PP_ICL_PTCACHE_SIZE, 12); // Optimize vertex cache
        aiSetImportPropertyInteger(props, AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4); // Limit bone weights
        aiSetImportPropertyInteger(props, AI_CONFIG_PP_FD_REMOVE, 1); // Remove degenerate primitives
        aiSetImportPropertyInteger(props, AI_CONFIG_PP_PTV_KEEP_HIERARCHY, 0); // Disable hierarchy preservation
        const struct aiScene *scene = aiImportFileExWithProperties(model_parser.entries[matchedParserIdx].path, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices, NULL, props);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) { DualLogError("Assimp failed to load %s: %s\n", model_parser.entries[matchedParserIdx].path, aiGetErrorString()); CleanupModelLoad(true); return 1; }

        // Count vertices, triangles, and estimate edges
        uint32_t vertexCount = 0;
        uint32_t triCount = 0;
        for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
            vertexCount += scene->mMeshes[m]->mNumVertices;
            triCount += scene->mMeshes[m]->mNumFaces;
        }
        
        if (vertexCount > MAX_VERT_COUNT || triCount > MAX_TRI_COUNT || triCount * 3 > MAX_EDGE_COUNT) { DualLogError("Model %s exceeds buffer limits: verts=%u (> %u), tris=%u (> %u), edges=%u (> %u)\n", model_parser.entries[matchedParserIdx].path, vertexCount, MAX_VERT_COUNT, triCount, MAX_TRI_COUNT, triCount * 3, MAX_EDGE_COUNT); CleanupModelLoad(true); return 1; }

        modelVertexCounts[i] = vertexCount;
        modelTriangleCounts[i] = triCount;
        uint32_t edgeCount = 0; // Will be updated after edge processing
        if (vertexCount > largestVertCount) largestVertCount = vertexCount;
        if (triCount > largestTriangleCount) largestTriangleCount = triCount;

#ifdef DEBUG_MODEL_LOAD_DATA
        if (triCount > 5000U) {
            DualLog("Model %s loaded with %d vertices, \033[1;33m%d\033[0;0m triangles\n", model_parser.entries[matchedParserIdx].path, vertexCount, triCount);
        } else {
            //DualLog("Model %s loaded with %d vertices, %d triangles\n", model_parser.entries[matchedParserIdx].path, vertexCount, triCount);
        }
#endif
        totalVertCount += vertexCount;
        totalTriCount += triCount;
        
        // Clear hash table
        memset(edgeHash, 0, HASH_SIZE * sizeof(EdgeHashEntry));

        // Extract vertex and triangle data, build edge list
        uint32_t vertexIndex = 0;
        float minx = 1E9f;
        float miny = 1E9f;
        float minz = 1E9f;
        float maxx = -1E9f;
        float maxy = -1E9f;
        float maxz = -1E9f;
        uint32_t triangleIndex = 0;
        uint32_t triEdgeIndex = 0;
        uint32_t globalVertexOffset = 0;

        for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
            struct aiMesh *mesh = scene->mMeshes[m];
            // Vertex data
            for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
                tempVertices[vertexIndex++] = mesh->mVertices[v].x; // Position
                tempVertices[vertexIndex++] = mesh->mVertices[v].y;
                tempVertices[vertexIndex++] = mesh->mVertices[v].z;
                tempVertices[vertexIndex++] = mesh->mNormals[v].x; // Normal (actually y, huh).
                tempVertices[vertexIndex++] = mesh->mNormals[v].y; // Actually X normal.  weird
                tempVertices[vertexIndex++] = mesh->mNormals[v].z;
                float tempU = mesh->mTextureCoords[0] ? mesh->mTextureCoords[0][v].x : 0.0f;
                float tempV = mesh->mTextureCoords[0] ? mesh->mTextureCoords[0][v].y : 0.0f;
                tempVertices[vertexIndex++] = tempU;
                tempVertices[vertexIndex++] = tempV;
                tempVertices[vertexIndex++] = 0; // Tex Index      Indices used later
                tempVertices[vertexIndex++] = 0; // Glow Index
                tempVertices[vertexIndex++] = 0; // Spec Index
                tempVertices[vertexIndex++] = 0; // Normal Index
                tempVertices[vertexIndex++] = 0; // Model Index
                tempVertices[vertexIndex++] = 0; // Instance Index
                if (mesh->mVertices[v].x < minx) minx = mesh->mVertices[v].x;
                if (mesh->mVertices[v].x > maxx) maxx = mesh->mVertices[v].x;
                if (mesh->mVertices[v].y < miny) miny = mesh->mVertices[v].y;
                if (mesh->mVertices[v].y > maxy) maxy = mesh->mVertices[v].y;
                if (mesh->mVertices[v].z < minz) minz = mesh->mVertices[v].z;
                if (mesh->mVertices[v].z > maxz) maxz = mesh->mVertices[v].z;
            }

            // Triangle and edge data
            for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
                struct aiFace *face = &mesh->mFaces[f];
                if (face->mNumIndices != 3) { DualLogError("Non-triangular face detected in %s, face %u\n", model_parser.entries[matchedParserIdx].path, f); CleanupModelLoad(true); return 1; }
                    
                uint32_t v[3] = {face->mIndices[0] + globalVertexOffset,
                                 face->mIndices[1] + globalVertexOffset,
                                 face->mIndices[2] + globalVertexOffset};
                uint32_t triangleIdx = triangleIndex / 3;
                uint32_t edgeIndices[3];

                // Validate vertex indices
                if (v[0] >= vertexCount || v[1] >= vertexCount || v[2] >= vertexCount) { DualLogError("Invalid vertex index in %s, face %u: v0=%u, v1=%u, v2=%u, vertexCount=%u\n", model_parser.entries[matchedParserIdx].path, f, v[0], v[1], v[2], vertexCount); CleanupModelLoad(true); return 1; }

                // Store vertex indices
                tempTriangles[triangleIndex++] = v[0];
                tempTriangles[triangleIndex++] = v[1];
                tempTriangles[triangleIndex++] = v[2];

                // Process edges with hash table
                for (int e = 0; e < 3; e++) {
                    uint32_t v0 = v[e];
                    uint32_t v1 = v[(e + 1) % 3];
                    if (v0 > v1) { uint32_t temp = v0; v0 = v1; v1 = temp; }

                    uint32_t hash = HASH(v0, v1);
                    while (edgeHash[hash].edgeIndex != 0 && (edgeHash[hash].v0 != v0 || edgeHash[hash].v1 != v1)) {
                        hash = (hash + 1) & (HASH_SIZE - 1); // Linear probing
                    }

                    int edgeFound = -1;
                    if (edgeHash[hash].edgeIndex != 0) {
                        edgeFound = edgeHash[hash].edgeIndex - 1;
//                         if (tempEdges[edgeFound].tri1 != UINT32_MAX) DualLogError("%s has >2 triangles for edge %d\n", model_parser.entries[matchedParserIdx].path, edgeFound);
                        tempEdges[edgeFound].tri1 = triangleIdx;
                    } else {
                        if (edgeCount < MAX_EDGE_COUNT) {
                            tempEdges[edgeCount].v0 = v0;
                            tempEdges[edgeCount].v1 = v1;
                            tempEdges[edgeCount].tri0 = triangleIdx;
                            tempEdges[edgeCount].tri1 = UINT32_MAX;
                            edgeHash[hash].v0 = v0;
                            edgeHash[hash].v1 = v1;
                            edgeHash[hash].edgeIndex = edgeCount + 1;
                            edgeFound = edgeCount++;
                        } else DualLogError("Edge count exceeds estimate for %s: %u >= %u\n", model_parser.entries[matchedParserIdx].path, edgeCount, MAX_EDGE_COUNT);
                    }

                    edgeIndices[e] = edgeFound;
                }

                // Store edge indices
                tempTriEdges[triEdgeIndex++] = edgeIndices[0];
                tempTriEdges[triEdgeIndex++] = edgeIndices[1];
                tempTriEdges[triEdgeIndex++] = edgeIndices[2];
            }
            globalVertexOffset += mesh->mNumVertices;
        }
        
        aiReleaseImport(scene);
        malloc_trim(0);
        
        modelEdgeCounts[i] = edgeCount;
        totalEdgeCount += edgeCount;
        if (edgeCount > largestEdgeCount) largestEdgeCount = edgeCount;
        
        // Copy to staging buffers
        if (vertexCount > 0) {
            glBindBuffer(GL_ARRAY_BUFFER, stagingVBO);
            void *mapped_buffer = glMapBufferRange(GL_ARRAY_BUFFER, 0, vertexCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
            if (!mapped_buffer) { DualLogError("Failed to map stagingVBO for model %d\n", i); CleanupModelLoad(true); return 1; }
            memcpy(mapped_buffer, tempVertices, vertexCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float));
            glUnmapBuffer(GL_ARRAY_BUFFER);
            glGenBuffers(1, &vbos[i]);
            glBindBuffer(GL_COPY_WRITE_BUFFER, vbos[i]);
            glBufferData(GL_COPY_WRITE_BUFFER, vertexCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float), NULL, GL_STATIC_DRAW);
            glCopyBufferSubData(GL_ARRAY_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, vertexCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float));
            CHECK_GL_ERROR();
        }

        if (triCount > 0) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stagingTBO);
            void *mapped_buffer = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, triCount * 3 * sizeof(uint32_t), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
            if (!mapped_buffer) { DualLogError("Failed to map stagingTBO for model %d\n", i); CleanupModelLoad(true); return 1; }
            memcpy(mapped_buffer, tempTriangles, triCount * 3 * sizeof(uint32_t));
            glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
            glGenBuffers(1, &tbos[i]);
            glBindBuffer(GL_COPY_WRITE_BUFFER, tbos[i]);
            glBufferData(GL_COPY_WRITE_BUFFER, triCount * 3 * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
            glCopyBufferSubData(GL_ELEMENT_ARRAY_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, triCount * 3 * sizeof(uint32_t));
            CHECK_GL_ERROR();

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stagingTEBO);
            mapped_buffer = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, triCount * 3 * sizeof(uint32_t), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
            if (!mapped_buffer) { DualLogError("Failed to map stagingTEBO for model %d\n", i); CleanupModelLoad(true); return 1; }
            memcpy(mapped_buffer, tempTriEdges, triCount * 3 * sizeof(uint32_t));
            glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
            glGenBuffers(1, &tebos[i]);
            glBindBuffer(GL_COPY_WRITE_BUFFER, tebos[i]);
            glBufferData(GL_COPY_WRITE_BUFFER, triCount * 3 * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
            glCopyBufferSubData(GL_ELEMENT_ARRAY_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, triCount * 3 * sizeof(uint32_t));
            CHECK_GL_ERROR();
        }

        if (edgeCount > 0) {
            uint32_t *tempEdgeData = (uint32_t *)malloc(edgeCount * 4 * sizeof(uint32_t));
            if (!tempEdgeData) { DualLogError("Failed to allocate edge buffer for %s\n", model_parser.entries[matchedParserIdx].path); CleanupModelLoad(true); return 1; }
            for (uint32_t j = 0; j < edgeCount; j++) {
                tempEdgeData[j * 4 + 0] = tempEdges[j].v0;
                tempEdgeData[j * 4 + 1] = tempEdges[j].v1;
                tempEdgeData[j * 4 + 2] = tempEdges[j].tri0;
                tempEdgeData[j * 4 + 3] = tempEdges[j].tri1;
            }
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stagingEBO);
            void *mapped_buffer = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, edgeCount * 4 * sizeof(uint32_t), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
            if (!mapped_buffer) { DualLogError("Failed to map stagingEBO for model %d\n", i); CleanupModelLoad(true); return 1; }
            memcpy(mapped_buffer, tempEdgeData, edgeCount * 4 * sizeof(uint32_t));
            glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
            glGenBuffers(1, &ebos[i]);
            glBindBuffer(GL_COPY_WRITE_BUFFER, ebos[i]);
            glBufferData(GL_COPY_WRITE_BUFFER, edgeCount * 4 * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
            glCopyBufferSubData(GL_ELEMENT_ARRAY_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, edgeCount * 4 * sizeof(uint32_t));
            CHECK_GL_ERROR();
            free(tempEdgeData);
        }

        float minx_pos = fabs(minx);
        float miny_pos = fabs(miny);
        float minz_pos = fabs(minz);
        float boundradius = minx_pos > miny_pos ? minx_pos : miny_pos;
        boundradius = boundradius > minz_pos ? boundradius : minz_pos;
        boundradius = boundradius > maxx ? boundradius : maxx;
        boundradius = boundradius > maxy ? boundradius : maxy;
        boundradius = boundradius > maxz ? boundradius : maxz;
        modelBounds[(i * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_MINX] = minx;
        modelBounds[(i * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_MINY] = miny;
        modelBounds[(i * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_MINZ] = minz;
        modelBounds[(i * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_MAXX] = maxx;
        modelBounds[(i * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_MAXY] = maxy;
        modelBounds[(i * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_MAXZ] = maxz;
        modelBounds[(i * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_RADIUS] = boundradius;
        totalBounds += BOUNDS_ATTRIBUTES_COUNT;

        malloc_trim(0);
        DebugRAM("post GPU upload for model %s", model_parser.entries[matchedParserIdx].path);
    }
    
    // Delete staging buffers
    glDeleteBuffers(1, &stagingVBO);
    glDeleteBuffers(1, &stagingTBO);
    glDeleteBuffers(1, &stagingTEBO);
    glDeleteBuffers(1, &stagingEBO);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    CHECK_GL_ERROR();
    DebugRAM("after staging buffers deleted");

#ifdef DEBUG_MODEL_LOAD_DATA
    DualLog("Largest vertex count: %d, triangle count: %d, edge count: %d\n", largestVertCount, largestTriangleCount, largestEdgeCount);
    DualLog("Total vertices: %d (", totalVertCount);
    print_bytes_no_newline(totalVertCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float));
    DualLog(")\nTotal triangles: %d (", totalTriCount);
    print_bytes_no_newline(totalTriCount * 3 * sizeof(uint32_t));
    DualLog(")\nTotal tri-edges: %d (", totalTriCount);
    print_bytes_no_newline(totalTriCount * 3 * sizeof(uint32_t));
    DualLog(")\nTotal edges: %d (", totalEdgeCount);
    print_bytes_no_newline(totalEdgeCount * 4 * sizeof(uint32_t));
    DualLog(")\nBounds (");
    print_bytes_no_newline(totalBounds * sizeof(float));
    DualLog(")\n");
#endif
    
    DebugRAM("post model load");

    // Clean up
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    CHECK_GL_ERROR();
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    DebugRAM("post model GPU data transfer");
    
    // Pass Model Type Bounds to GPU
    glGenBuffers(1, &modelBoundsID);
    CHECK_GL_ERROR();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelBoundsID);
    CHECK_GL_ERROR();
    glBufferData(GL_SHADER_STORAGE_BUFFER, MODEL_COUNT * BOUNDS_ATTRIBUTES_COUNT * sizeof(float), modelBounds, GL_STATIC_DRAW);
    CHECK_GL_ERROR();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, modelBoundsID);
    CHECK_GL_ERROR();
    malloc_trim(0);
    DebugRAM("post model model bounds data transfer");
    
    // Upload modelVertexOffsets
//     uint32_t modelVertexOffsets[MODEL_COUNT];
//     uint32_t offset = 0;
//     for (uint32_t i = 0; i < MODEL_COUNT; i++) {
//         modelVertexOffsets[i] = offset;
//         offset += modelVertexCounts[i];
//     }
//     
//     glGenBuffers(1, &modelVertexOffsetsID);
//     CHECK_GL_ERROR();
//     glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelVertexOffsetsID);
//     CHECK_GL_ERROR();
//     glBufferData(GL_SHADER_STORAGE_BUFFER, MODEL_COUNT * sizeof(uint32_t), modelVertexOffsets, GL_STATIC_DRAW);
//     CHECK_GL_ERROR();
//     glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, modelVertexOffsetsID);
//     CHECK_GL_ERROR();
    
    // Pass Model Vertex Counts to GPU
//     glGenBuffers(1, &modelVertexCountsID);
//     CHECK_GL_ERROR();
//     glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelVertexCountsID);
//     CHECK_GL_ERROR();
//     glBufferData(GL_SHADER_STORAGE_BUFFER, MODEL_COUNT * sizeof(uint32_t), modelVertexCounts, GL_STATIC_DRAW);
//     CHECK_GL_ERROR();
//     glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, modelVertexCountsID);
//     CHECK_GL_ERROR();
    
    // Create duplicate of all mesh data in one flat buffer in VRAM without using RAM
//     glGenBuffers(1, &vboMasterTable);
//     CHECK_GL_ERROR();
//     glBindBuffer(GL_SHADER_STORAGE_BUFFER, vboMasterTable);
//     CHECK_GL_ERROR();
//     glBufferData(GL_SHADER_STORAGE_BUFFER, totalVertCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float), NULL, GL_STATIC_DRAW);
//     CHECK_GL_ERROR();
//     for (int i = 0; i < MODEL_COUNT; ++i) {
//         glBindBuffer(GL_COPY_READ_BUFFER, vbos[i]);
//         CHECK_GL_ERROR();
//         glBindBuffer(GL_COPY_WRITE_BUFFER, vboMasterTable);
//         CHECK_GL_ERROR();
//         glCopyBufferSubData(GL_COPY_READ_BUFFER,GL_COPY_WRITE_BUFFER,0, modelVertexOffsets[i] * VERTEX_ATTRIBUTES_COUNT * sizeof(float), modelVertexCounts[i] * VERTEX_ATTRIBUTES_COUNT * sizeof(float));
//         CHECK_GL_ERROR();
//     }
// 
//     glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 20, vboMasterTable);
//     CHECK_GL_ERROR();
    
    // Create sphoxel buffer
//     glGenBuffers(1,&sphoxelsID);
//     CHECK_GL_ERROR();

    CleanupModelLoad(false);
    double end_time = get_time();
    DualLog("Load Models took %f seconds\n", end_time - start_time);
    DebugRAM("After full LoadModels completed");
    return 0;
}

void CleanupModelLoad(bool isBad) {
    parser_free(&model_parser);
    if (!isBad) {
        if (tempVertices) { free(tempVertices); tempVertices = NULL; }
        if (tempTriangles) { free(tempTriangles); tempTriangles = NULL; }
        if (tempTriEdges) { free(tempTriEdges); tempTriEdges = NULL; }
        if (tempEdges) { free(tempEdges); tempEdges = NULL; }
        if (edgeHash) { free(edgeHash); edgeHash = NULL; }
        if (vertexDataArrays) { free(vertexDataArrays); vertexDataArrays = NULL; }
        if (triangleDataArrays) { free(triangleDataArrays); triangleDataArrays = NULL; }
        if (triEdgeDataArrays) { free(triEdgeDataArrays); triEdgeDataArrays = NULL; }
        if (edgeDataArrays) { free(edgeDataArrays); edgeDataArrays = NULL; }
    } else {
        if (tempVertices) { free(tempVertices); tempVertices = NULL; }
        if (tempTriangles) { free(tempTriangles); tempTriangles = NULL; }
        if (tempTriEdges) { free(tempTriEdges); tempTriEdges = NULL; }
        if (tempEdges) { free(tempEdges); tempEdges = NULL; }
        if (edgeHash) { free(edgeHash); edgeHash = NULL; }
        if (vertexDataArrays) { free(vertexDataArrays); vertexDataArrays = NULL; }
        if (triangleDataArrays) { free(triangleDataArrays); triangleDataArrays = NULL; }
        if (triEdgeDataArrays) { free(triEdgeDataArrays); triEdgeDataArrays = NULL; }
        if (edgeDataArrays) { free(edgeDataArrays); edgeDataArrays = NULL; }
    }
    malloc_trim(0);
}
