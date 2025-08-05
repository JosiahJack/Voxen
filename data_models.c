#include <malloc.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "constants.h"
#include "data_models.h"
#include "debug.h"
#include "render.h"
#include "data_parser.h"
#include "voxel.h"
#include "event.h"

#define DEBUG_MODEL_LOAD_DATA

DataParser model_parser;
const char *valid_mdldata_keys[] = {"index"};
#define NUM_MODEL_KEYS 1
uint32_t modelVertexCounts[MODEL_COUNT];
uint32_t modelTriangleCounts[MODEL_COUNT];
uint32_t modelEdgeCounts[MODEL_COUNT];
GLuint vbos[MODEL_COUNT];
GLuint tbos[MODEL_COUNT]; // Triangle index buffers
GLuint ebos[MODEL_COUNT]; // Edge index buffers
uint32_t totalVertexCount = 0;
uint32_t totalTriangleCount = 0;
uint32_t totalEdgeCount = 0;
GLuint modelBoundsID;
float modelBounds[MODEL_COUNT * BOUNDS_ATTRIBUTES_COUNT];
uint32_t largestVertCount = 0;
uint32_t largestTriangleCount = 0;
uint32_t largestEdgeCount = 0;
GLuint vboMasterTable;
GLuint modelVertexOffsetsID;
GLuint modelVertexCountsID;
GLuint sphoxelsID;

// Structure to represent an edge for building edge list
typedef struct {
    uint32_t v0, v1; // Vertex indices (sorted: v0 < v1)
    uint32_t tri0, tri1; // Triangle indices (tri1 = UINT32_MAX if unshared)
} Edge;

// Loads all 3D meshes
int LoadGeometry(void) {
    CHECK_GL_ERROR();
    double start_time = get_time();
    DebugRAM("start of loading all models");

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

    float **vertexDataArrays = (float **)calloc(MODEL_COUNT, sizeof(float *));
    uint32_t **triangleDataArrays = (uint32_t **)calloc(MODEL_COUNT, sizeof(uint32_t *));
    uint32_t **edgeDataArrays = (uint32_t **)calloc(MODEL_COUNT, sizeof(uint32_t *));
    if (!vertexDataArrays || !triangleDataArrays || !edgeDataArrays) {
        DualLogError("Failed to allocate temporary arrays\n");
        parser_free(&model_parser);
        free(vertexDataArrays);
        free(triangleDataArrays);
        free(edgeDataArrays);
        return 1;
    }

    for (uint32_t i = 0; i < MODEL_COUNT; i++) {
        int matchedParserIdx = -1;
        for (int k=0;k<model_parser.count;k++) {
            if (model_parser.entries[k].index == i) {matchedParserIdx = k; break; }
        }

        if (matchedParserIdx < 0) continue;
        if (!model_parser.entries[matchedParserIdx].path || model_parser.entries[matchedParserIdx].path[0] == '\0') continue;

        const struct aiScene *scene = aiImportFile(model_parser.entries[matchedParserIdx].path,aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            DualLogError("Assimp failed to load %s: %s\n", model_parser.entries[matchedParserIdx].path, aiGetErrorString());
            free(vertexDataArrays);
            free(triangleDataArrays);
            free(edgeDataArrays);
            parser_free(&model_parser);
            return 1;
        }

        DebugRAM("after assimp scene made for %s",model_parser.entries[matchedParserIdx].path);

        // Count vertices, triangles, and estimate edges
        uint32_t vertexCount = 0;
        uint32_t triCount = 0;
        for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
            vertexCount += scene->mMeshes[m]->mNumVertices;
            triCount += scene->mMeshes[m]->mNumFaces;
        }

        uint32_t maxEdgeCount = triCount * 3; // Upper bound
        Edge *tempEdges = (Edge *)calloc(maxEdgeCount, sizeof(Edge));
        if (!tempEdges) {
            DualLogError("Failed to allocate edge array for %s\n", model_parser.entries[matchedParserIdx].path);
            aiReleaseImport(scene);
            free(vertexDataArrays);
            free(triangleDataArrays);
            free(edgeDataArrays);
            parser_free(&model_parser);
            return 1;
        }

        modelVertexCounts[i] = vertexCount;
        modelTriangleCounts[i] = triCount;
        uint32_t edgeCount = 0; // Will be updated after edge processing

        if (vertexCount > largestVertCount) largestVertCount = vertexCount;
        if (triCount > largestTriangleCount) largestTriangleCount = triCount;

#ifdef DEBUG_MODEL_LOAD_DATA
        if (vertexCount > 1000U) {
            DualLog("Model %s loaded with \033[1;33m%d\033[0;0m vertices, %d triangles\n", model_parser.entries[matchedParserIdx].path, vertexCount, triCount);
        } else {
            DualLog("Model %s loaded with %d vertices, %d triangles\n", model_parser.entries[matchedParserIdx].path, vertexCount, triCount);
        }
#endif
        totalVertCount += vertexCount;
        totalTriCount += triCount;

        // Allocate vertex and triangle data
        float *tempVertices = (float *)malloc(vertexCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float));
        uint32_t *tempTriangles = (uint32_t *)malloc(triCount * 6 * sizeof(uint32_t)); // 3 verts + 3 edges per triangle
        if (!tempVertices || !tempTriangles) {
            DualLogError("Failed to allocate buffers for %s\n", model_parser.entries[matchedParserIdx].path);
            free(tempEdges);
            aiReleaseImport(scene);
            free(vertexDataArrays);
            free(triangleDataArrays);
            free(edgeDataArrays);
            parser_free(&model_parser);
            return 1;
        }

        DebugRAM("after tempVertices and tempTriangles malloc for %s", model_parser.entries[matchedParserIdx].path);

        // Extract vertex and triangle data, build edge list
        uint32_t vertexIndex = 0;
        float minx = 1E9f;
        float miny = 1E9f;
        float minz = 1E9f;
        float maxx = -1E9f;
        float maxy = -1E9f;
        float maxz = -1E9f;
        uint32_t triangleIndex = 0;
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
                uint32_t v[3] = {face->mIndices[0], face->mIndices[1], face->mIndices[2]};
                uint32_t triangleIdx = triangleIndex / 6; // Current triangle index
                uint32_t edgeIndices[3];

                // Store vertex indices for the triangle
                tempTriangles[triangleIndex++] = v[0];
                tempTriangles[triangleIndex++] = v[1];
                tempTriangles[triangleIndex++] = v[2];

                // Process each edge (v0-v1, v1-v2, v2-v0)
                for (int e = 0; e < 3; e++) {
                    uint32_t v0 = v[e] + globalVertexOffset;
                    uint32_t v1 = v[(e + 1) % 3] + globalVertexOffset;
                    // Sort vertices for consistent edge identification
                    if (v0 > v1) { uint32_t temp = v0; v0 = v1; v1 = temp; }

                    // Check if edge exists
                    int edgeFound = -1;
                    for (uint32_t j = 0; j < edgeCount; j++) {
                        if (tempEdges[j].v0 == v0 && tempEdges[j].v1 == v1) {
                            edgeFound = j;
                            break;
                        }
                    }

                    if (edgeFound >= 0) {
                        // Edge exists, update tri1
                        if (tempEdges[edgeFound].tri1 == UINT32_MAX) {
                            tempEdges[edgeFound].tri1 = triangleIdx;
                        } else {
                            DualLogError("Non-manifold edge detected in %s\n", model_parser.entries[matchedParserIdx].path);
//                             free(tempEdges);
//                             free(tempVertices);
//                             free(tempTriangles);
//                             aiReleaseImport(scene);
//                             free(vertexDataArrays);
//                             free(triangleDataArrays);
//                             free(edgeDataArrays);
//                             parser_free(&model_parser);
//                             return 1;
                        }
                    } else {
                        // New edge
                        if (edgeCount >= maxEdgeCount) {
                            DualLogError("Edge count exceeds estimate for %s\n", model_parser.entries[matchedParserIdx].path);
                            free(tempEdges);
                            free(tempVertices);
                            free(tempTriangles);
                            aiReleaseImport(scene);
                            free(vertexDataArrays);
                            free(triangleDataArrays);
                            free(edgeDataArrays);
                            parser_free(&model_parser);
                            return 1;
                        }
                        tempEdges[edgeCount].v0 = v0;
                        tempEdges[edgeCount].v1 = v1;
                        tempEdges[edgeCount].tri0 = triangleIdx;
                        tempEdges[edgeCount].tri1 = UINT32_MAX;
                        edgeFound = edgeCount++;
                    }
                    edgeIndices[e] = edgeFound;
                }

                // Store edge indices for the triangle
                tempTriangles[triangleIndex++] = edgeIndices[0];
                tempTriangles[triangleIndex++] = edgeIndices[1];
                tempTriangles[triangleIndex++] = edgeIndices[2];
            }
            globalVertexOffset += mesh->mNumVertices;
        }

        modelEdgeCounts[i] = edgeCount;
        if (edgeCount > largestEdgeCount) largestEdgeCount = edgeCount;
        totalEdgeCount += edgeCount;

        // Allocate and populate edge buffer
        uint32_t *tempEdgeData = (uint32_t *)malloc(edgeCount * 4 * sizeof(uint32_t)); // 2 verts + 2 tris per edge
        if (!tempEdgeData) {
            DualLogError("Failed to allocate edge buffer for %s\n", model_parser.entries[matchedParserIdx].path);
            free(tempEdges);
            free(tempVertices);
            free(tempTriangles);
            aiReleaseImport(scene);
            free(vertexDataArrays);
            free(triangleDataArrays);
            free(edgeDataArrays);
            parser_free(&model_parser);
            return 1;
        }

        for (uint32_t j = 0; j < edgeCount; j++) {
            tempEdgeData[j * 4 + 0] = tempEdges[j].v0;
            tempEdgeData[j * 4 + 1] = tempEdges[j].v1;
            tempEdgeData[j * 4 + 2] = tempEdges[j].tri0;
            tempEdgeData[j * 4 + 3] = tempEdges[j].tri1;
        }

        free(tempEdges); // Free temporary edge array
        DebugRAM("after vertices, triangles, and edges for %s", model_parser.entries[matchedParserIdx].path);

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

        vertexDataArrays[i] = tempVertices;
        triangleDataArrays[i] = tempTriangles;
        edgeDataArrays[i] = tempEdgeData;
        aiReleaseImport(scene);
        malloc_trim(0);
        DebugRAM("after assimp release %s", model_parser.entries[matchedParserIdx].path);
    }

#ifdef DEBUG_MODEL_LOAD_DATA
    DualLog("Largest vertex count: %d, triangle count: %d, edge count: %d\n",
            largestVertCount, largestTriangleCount, largestEdgeCount);
#endif
    DebugRAM("after model load first pass");

#ifdef DEBUG_MODEL_LOAD_DATA
    DualLog("Total vertices: %d (", totalVertCount);
    print_bytes_no_newline(totalVertCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float));
    DualLog("), Total triangles: %d (", totalTriCount);
    print_bytes_no_newline(totalTriCount * 6 * sizeof(uint32_t));
    DualLog("), Total edges: %d (", totalEdgeCount);
    print_bytes_no_newline(totalEdgeCount * 4 * sizeof(uint32_t));
    DualLog("), Bounds (");
    print_bytes_no_newline(totalBounds * sizeof(float));
    DualLog(")\n");
#endif

    // Generate VBOs, TBOs, and EBOs
    memset(vbos, 0, MODEL_COUNT * sizeof(GLuint));
    memset(tbos, 0, MODEL_COUNT * sizeof(GLuint));
    memset(ebos, 0, MODEL_COUNT * sizeof(GLuint));
    glGenBuffers(MODEL_COUNT, vbos);
    glGenBuffers(MODEL_COUNT, tbos);
    glGenBuffers(MODEL_COUNT, ebos);
    CHECK_GL_ERROR();

    // Staging buffers
    GLuint stagingVertexBuffer, stagingTriangleBuffer, stagingEdgeBuffer;
    glGenBuffers(1, &stagingVertexBuffer);
    glGenBuffers(1, &stagingTriangleBuffer);
    glGenBuffers(1, &stagingEdgeBuffer);
    CHECK_GL_ERROR();

    glBindBuffer(GL_ARRAY_BUFFER, stagingVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, largestVertCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float), NULL, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stagingTriangleBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, largestTriangleCount * 6 * sizeof(uint32_t), NULL, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stagingEdgeBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, largestEdgeCount * 4 * sizeof(uint32_t), NULL, GL_DYNAMIC_COPY);
    CHECK_GL_ERROR();
    DebugRAM("after glGenBuffers vbos, tbos, and ebos");

    for (uint32_t i = 0; i < MODEL_COUNT; i++) {
        if (modelVertexCounts[i] == 0 || vertexDataArrays[i] == NULL ||
            triangleDataArrays[i] == NULL || edgeDataArrays[i] == NULL) {
            if (modelVertexCounts[i] != 0) {
                DualLog("\033[33mWARNING: model %d has invalid data (verts=%u, tris=%u, edges=%u, vert_ptr=%p, tri_ptr=%p, edge_ptr=%p)\033[0m\n",
                        i, modelVertexCounts[i], modelTriangleCounts[i], modelEdgeCounts[i],
                        (void*)vertexDataArrays[i], (void*)triangleDataArrays[i], (void*)edgeDataArrays[i]);
            }
            continue;
        }

        // Load vertices
        glBindBuffer(GL_ARRAY_BUFFER, stagingVertexBuffer);
        void *mappedVertexBuffer = glMapBufferRange(GL_ARRAY_BUFFER, 0,
                                                   modelVertexCounts[i] * VERTEX_ATTRIBUTES_COUNT * sizeof(float),
                                                   GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
        if (!mappedVertexBuffer) {
            DualLogError("Failed to map stagingVertexBuffer for model %d\n", i);
            continue;
        }
        memcpy(mappedVertexBuffer, vertexDataArrays[i], modelVertexCounts[i] * VERTEX_ATTRIBUTES_COUNT * sizeof(float));
        glUnmapBuffer(GL_ARRAY_BUFFER);

        glBindBuffer(GL_COPY_READ_BUFFER, stagingVertexBuffer);
        glBindBuffer(GL_COPY_WRITE_BUFFER, vbos[i]);
        glBufferData(GL_COPY_WRITE_BUFFER, modelVertexCounts[i] * VERTEX_ATTRIBUTES_COUNT * sizeof(float), NULL, GL_STATIC_DRAW);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0,
                            modelVertexCounts[i] * VERTEX_ATTRIBUTES_COUNT * sizeof(float));
        CHECK_GL_ERROR();

        // Load triangles
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stagingTriangleBuffer);
        void *mappedTriangleBuffer = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0,
                                                     modelTriangleCounts[i] * 6 * sizeof(uint32_t),
                                                     GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
        if (!mappedTriangleBuffer) {
            DualLogError("Failed to map stagingTriangleBuffer for model %d\n", i);
            continue;
        }
        memcpy(mappedTriangleBuffer, triangleDataArrays[i], modelTriangleCounts[i] * 6 * sizeof(uint32_t));
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

        glBindBuffer(GL_COPY_READ_BUFFER, stagingTriangleBuffer);
        glBindBuffer(GL_COPY_WRITE_BUFFER, tbos[i]);
        glBufferData(GL_COPY_WRITE_BUFFER, modelTriangleCounts[i] * 6 * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0,
                            modelTriangleCounts[i] * 6 * sizeof(uint32_t));
        CHECK_GL_ERROR();

        // Load edges
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stagingEdgeBuffer);
        void *mappedEdgeBuffer = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0,
                                                 modelEdgeCounts[i] * 4 * sizeof(uint32_t),
                                                 GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
        if (!mappedEdgeBuffer) {
            DualLogError("Failed to map stagingEdgeBuffer for model %d\n", i);
            continue;
        }
        memcpy(mappedEdgeBuffer, edgeDataArrays[i], modelEdgeCounts[i] * 4 * sizeof(uint32_t));
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

        glBindBuffer(GL_COPY_READ_BUFFER, stagingEdgeBuffer);
        glBindBuffer(GL_COPY_WRITE_BUFFER, ebos[i]);
        glBufferData(GL_COPY_WRITE_BUFFER, modelEdgeCounts[i] * 4 * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0,
                            modelEdgeCounts[i] * 4 * sizeof(uint32_t));
        CHECK_GL_ERROR();

        // Free temporary buffers
        free(vertexDataArrays[i]);
        free(triangleDataArrays[i]);
        free(edgeDataArrays[i]);
        vertexDataArrays[i] = NULL;
        triangleDataArrays[i] = NULL;
        edgeDataArrays[i] = NULL;
    }

    // Clean up
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &stagingVertexBuffer);
    glDeleteBuffers(1, &stagingTriangleBuffer);
    glDeleteBuffers(1, &stagingEdgeBuffer);
    free(vertexDataArrays);
    free(triangleDataArrays);
    free(edgeDataArrays);
    CHECK_GL_ERROR();
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    glFlush();
    glFinish();
    CHECK_GL_ERROR();
    DebugRAM("after model staging buffer clear");

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

    parser_free(&model_parser);
    malloc_trim(0);
    DebugRAM("after loading all models");
    double end_time = get_time();
    DualLog("Load Models took %f seconds\n", end_time - start_time);
    return 0;
}
