#include <malloc.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
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

// #define DEBUG_MODEL_LOAD_DATA

DataParser model_parser;
const char *valid_mdldata_keys[] = {"index"};
#define NUM_MODEL_KEYS 1
uint32_t modelVertexCounts[MODEL_COUNT];
GLuint vbos[MODEL_COUNT];
uint32_t totalVertexCount = 0;
GLuint modelBoundsID;
float modelBounds[MODEL_COUNT * BOUNDS_ATTRIBUTES_COUNT];
float * vertexDataArrays[MODEL_COUNT];
uint32_t largestVertCount = 0;
GLuint vboMasterTable;
GLuint modelVertexOffsetsID;

// New: Edge-related structures and arrays
typedef struct {
    float x0, y0, z0; // Start vertex position
    float x1, y1, z1; // End vertex position
    float nx0, ny0, nz0; // Normal of first adjacent triangle
    float nx1, ny1, nz1; // Normal of second adjacent triangle
} Edge;

typedef struct {
    Edge *edges;
    uint32_t edgeCount;
} EdgeList;

EdgeList edgeDataArrays[MODEL_COUNT]; // Per-model edge lists

typedef enum {
    MDL_PARSER = 0,
    MDL_COUNT // Number of subsystems
} ModelLoadDataType;

bool loadModelItemInitialized[MDL_COUNT] = { [0 ... MDL_COUNT - 1] = false };

// Helper function: Compute triangle normal
static void computeTriangleNormal(const struct aiMesh *mesh, unsigned int faceIdx, float *nx, float *ny, float *nz) {
    struct aiFace *face = &mesh->mFaces[faceIdx];
    if (face->mNumIndices != 3) return; // Ensure triangle

    // Get vertex positions
    float v0[3] = {mesh->mVertices[face->mIndices[0]].x, mesh->mVertices[face->mIndices[0]].y, mesh->mVertices[face->mIndices[0]].z};
    float v1[3] = {mesh->mVertices[face->mIndices[1]].x, mesh->mVertices[face->mIndices[1]].y, mesh->mVertices[face->mIndices[1]].z};
    float v2[3] = {mesh->mVertices[face->mIndices[2]].x, mesh->mVertices[face->mIndices[2]].y, mesh->mVertices[face->mIndices[2]].z};

    // Compute edges
    float e1[3] = {v1[0] - v0[0], v1[1] - v0[1], v1[2] - v0[2]};
    float e2[3] = {v2[0] - v0[0], v2[1] - v0[1], v2[2] - v0[2]};

    // Cross product for normal
    *nx = e1[1] * e2[2] - e1[2] * e2[1];
    *ny = e1[2] * e2[0] - e1[0] * e2[2];
    *nz = e1[0] * e2[1] - e1[1] * e2[0];

    // Normalize
    float len = sqrtf(*nx * *nx + *ny * *ny + *nz * *nz);
    if (len > 0.0f) {
        *nx /= len;
        *ny /= len;
        *nz /= len;
    }
}

// Helper function: Compare two vertices for equality (with tolerance)
static bool verticesEqual(float x0, float y0, float z0, float x1, float y1, float z1, float tolerance) {
    return fabs(x0 - x1) < tolerance && fabs(y0 - y1) < tolerance && fabs(z0 - z1) < tolerance;
}

// Loads all geometry, from 3D meshes or otherwise
int LoadGeometry(void) {
    CHECK_GL_ERROR();
    double start_time = get_time();
    DebugRAM("start of loading all models");

    // Initialize edgeDataArrays
    memset(edgeDataArrays, 0, MODEL_COUNT * sizeof(EdgeList));

    // First parse ./Data/textures.txt to see what textures to load to what indices
    parser_init(&model_parser, valid_mdldata_keys, NUM_MODEL_KEYS, PARSER_DATA);
    if (!parse_data_file(&model_parser, "./Data/models.txt")) { DualLogError("Could not parse ./Data/models.txt!\n"); parser_free(&model_parser); return 1; }

    loadModelItemInitialized[MDL_PARSER] = true;
    int maxIndex = -1;
    for (int k=0;k<model_parser.count;k++) {
        if (model_parser.entries[k].index > maxIndex && model_parser.entries[k].index != UINT16_MAX) {maxIndex = model_parser.entries[k].index; }
    }

    DualLog("Parsing %d models...\n",model_parser.count);

    int totalVertCount = 0;
    int totalBounds = 0;
    largestVertCount = 0;
    for (uint32_t i = 0; i < MODEL_COUNT; i++) {
        int matchedParserIdx = -1;
        for (int k=0;k<model_parser.count;k++) {
            if (model_parser.entries[k].index == i) {matchedParserIdx = k; break; }
        }

        if (matchedParserIdx < 0) continue;
        if (!model_parser.entries[matchedParserIdx].path || model_parser.entries[matchedParserIdx].path[0] == '\0') continue;

        const struct aiScene *scene = aiImportFile(model_parser.entries[matchedParserIdx].path,aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            DualLogError("Assimp failed to load %s: %s\n", model_parser.entries[matchedParserIdx].path, aiGetErrorString());
            return 1;
        }

        DebugRAM("after assimp scene made for %s",model_parser.entries[matchedParserIdx].path);

        // Count total vertices and triangles in the model
        uint32_t vertexCount = 0;
        uint32_t triCount = 0;
        for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
            vertexCount += scene->mMeshes[m]->mNumVertices;
            triCount += scene->mMeshes[m]->mNumFaces;
        }

        modelVertexCounts[i] = vertexCount;
        if (vertexCount > largestVertCount) largestVertCount = vertexCount;
#ifdef DEBUG_MODEL_LOAD_DATA
        if (vertexCount > 5000U) {
            DualLog("Model %s loaded with \033[1;33m%d\033[0;0m vertices, %d tris\n", model_parser.entries[matchedParserIdx].path, vertexCount, triCount);
        } else {
            DualLog("Model %s loaded with %d vertices, %d tris\n", model_parser.entries[matchedParserIdx].path, vertexCount, triCount);
        }
#endif
        totalVertCount += vertexCount;

        // Allocate vertex data for this model
        float *tempVertices = (float *)malloc(vertexCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float));
        if (!tempVertices) {
            DualLogError("Failed to allocate vertex buffer for %s\n", model_parser.entries[matchedParserIdx].path);
            aiReleaseImport(scene);
            return 1;
        }

        DebugRAM("after tempVertices malloc for %s",model_parser.entries[matchedParserIdx].path);

        // Allocate temporary storage for edges
        uint32_t maxPossibleEdges = triCount * 3; // Each triangle has 3 edges
        Edge *tempEdges = (Edge *)malloc(maxPossibleEdges * sizeof(Edge));
        uint32_t edgeCount = 0;
        if (!tempEdges) {
            DualLogError("Failed to allocate edge buffer for %s\n", model_parser.entries[matchedParserIdx].path);
            free(tempVertices);
            aiReleaseImport(scene);
            return 1;
        }

        // Extract vertex data
        uint32_t vertexIndex = 0;
        float minx = 1E9f;
        float miny = 1E9f;
        float minz = 1E9f;
        float maxx = -1E9f;
        float maxy = -1E9f;
        float maxz = -1E9f;

        // Store triangle data for edge processing
        typedef struct {
            float v[3][3]; // Three vertices: x, y, z
            float normal[3]; // Triangle normal: nx, ny, nz
        } Triangle;
        Triangle *triangles = (Triangle *)malloc(triCount * sizeof(Triangle));
        if (!triangles) {
            DualLogError("Failed to allocate triangle buffer for %s\n", model_parser.entries[matchedParserIdx].path);
            free(tempVertices);
            free(tempEdges);
            aiReleaseImport(scene);
            return 1;
        }
        uint32_t triIndex = 0;

        for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
            struct aiMesh *mesh = scene->mMeshes[m];
            // Store triangles
            for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
                if (mesh->mFaces[f].mNumIndices != 3) continue; // Skip non-triangles
                struct aiFace *face = &mesh->mFaces[f];
                triangles[triIndex].v[0][0] = mesh->mVertices[face->mIndices[0]].x;
                triangles[triIndex].v[0][1] = mesh->mVertices[face->mIndices[0]].y;
                triangles[triIndex].v[0][2] = mesh->mVertices[face->mIndices[0]].z;
                triangles[triIndex].v[1][0] = mesh->mVertices[face->mIndices[1]].x;
                triangles[triIndex].v[1][1] = mesh->mVertices[face->mIndices[1]].y;
                triangles[triIndex].v[1][2] = mesh->mVertices[face->mIndices[1]].z;
                triangles[triIndex].v[2][0] = mesh->mVertices[face->mIndices[2]].x;
                triangles[triIndex].v[2][1] = mesh->mVertices[face->mIndices[2]].y;
                triangles[triIndex].v[2][2] = mesh->mVertices[face->mIndices[2]].z;
                computeTriangleNormal(mesh, f, &triangles[triIndex].normal[0], &triangles[triIndex].normal[1], &triangles[triIndex].normal[2]);
                triIndex++;
            }

            for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
                tempVertices[vertexIndex++] = -mesh->mVertices[v].x; // Position
                tempVertices[vertexIndex++] = mesh->mVertices[v].y;
                tempVertices[vertexIndex++] = mesh->mVertices[v].z;
                tempVertices[vertexIndex++] = -mesh->mNormals[v].x; // Normal (actually y, huh).
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
        }
        
        DebugRAM("after vertices for loop for %s",model_parser.entries[matchedParserIdx].path);

        // Generate edge list (non-indexed, compare vertex positions)
        const float tolerance = 0.0001f; // Tolerance for floating-point comparison
        for (uint32_t t1 = 0; t1 < triCount; t1++) {
            // Get edges of triangle t1: (v0,v1), (v1,v2), (v2,v0)
            float *v0 = triangles[t1].v[0];
            float *v1 = triangles[t1].v[1];
            float *v2 = triangles[t1].v[2];
            float *n1 = triangles[t1].normal;
            float *edges[3][2] = {{v0, v1}, {v1, v2}, {v2, v0}}; // Edge vertex pairs

            for (int e = 0; e < 3; e++) {
                float *e_v0 = edges[e][0];
                float *e_v1 = edges[e][1];
                bool found = false;

                // Check against other triangles
                for (uint32_t t2 = t1 + 1; t2 < triCount && !found; t2++) {
                    float *u0 = triangles[t2].v[0];
                    float *u1 = triangles[t2].v[1];
                    float *u2 = triangles[t2].v[2];
                    float *n2 = triangles[t2].normal;
                    float *other_edges[3][2] = {{u0, u1}, {u1, u2}, {u2, u0}};

                    for (int f = 0; f < 3; f++) {
                        // Check if edges match (v0->v1 == u1->u0 or v0->v1 == u0->u1)
                        if ((verticesEqual(e_v0[0], e_v0[1], e_v0[2], other_edges[f][1][0], other_edges[f][1][1], other_edges[f][1][2], tolerance) &&
                             verticesEqual(e_v1[0], e_v1[1], e_v1[2], other_edges[f][0][0], other_edges[f][0][1], other_edges[f][0][2], tolerance)) ||
                            (verticesEqual(e_v0[0], e_v0[1], e_v0[2], other_edges[f][0][0], other_edges[f][0][1], other_edges[f][0][2], tolerance) &&
                             verticesEqual(e_v1[0], e_v1[1], e_v1[2], other_edges[f][1][0], other_edges[f][1][1], other_edges[f][1][2], tolerance))) {
                            // Shared edge found
                            Edge edge;
                            edge.x0 = e_v0[0];
                            edge.y0 = e_v0[1];
                            edge.z0 = e_v0[2];
                            edge.x1 = e_v1[0];
                            edge.y1 = e_v1[1];
                            edge.z1 = e_v1[2];
                            edge.nx0 = n1[0];
                            edge.ny0 = n1[1];
                            edge.nz0 = n1[2];
                            edge.nx1 = n2[0];
                            edge.ny1 = n2[1];
                            edge.nz1 = n2[2];
                            if (edgeCount < maxPossibleEdges) {
                                tempEdges[edgeCount++] = edge;
                            }
                            found = true;
                            break;
                        }
                    }
                }
            }
        }

        // Store edge list
        edgeDataArrays[i].edges = (Edge *)malloc(edgeCount * sizeof(Edge));
        if (!edgeDataArrays[i].edges) {
            DualLogError("Failed to allocate edgeDataArrays for model %d\n", i);
            free(tempVertices);
            free(tempEdges);
            free(triangles);
            aiReleaseImport(scene);
            return 1;
        }
        memcpy(edgeDataArrays[i].edges, tempEdges, edgeCount * sizeof(Edge));
        edgeDataArrays[i].edgeCount = edgeCount;
#ifdef DEBUG_MODEL_LOAD_DATA
        DualLog("Model %s generated %u edges\n",model_parser.entries[matchedParserIdx].path, edgeCount);
#endif
        free(tempEdges);
        free(triangles);

        DebugRAM("after vertices and edges for %s",model_parser.entries[matchedParserIdx].path);

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
        aiReleaseImport(scene);
        malloc_trim(0);
        DebugRAM("after assimp release %s",model_parser.entries[matchedParserIdx].path);
    }

#ifdef DEBUG_MODEL_LOAD_DATA
    DualLog("Largest vertex count for a model: %d\n",largestVertCount);
#endif
    DebugRAM("after model load first pass");

#ifdef DEBUG_MODEL_LOAD_DATA
    // Log vertex counts
    DualLog("Total vertices in buffer %d (",totalVertCount);
    print_bytes_no_newline(totalVertCount * 8 * 4); // x,y,z,nx,ny,nz,u,v * 4
    DualLog(") (Bounds ");
    print_bytes_no_newline(totalBounds * 4); //minx,miny,minz,maxx,maxy,maxz * 4 
    DualLog(")\n");
#endif

    memset(vbos, 0, MODEL_COUNT * sizeof(uint32_t));

    // Generate and populate VBOs
    glGenBuffers(MODEL_COUNT, vbos);
    CHECK_GL_ERROR();

    GLuint stagingBuffer;
    glGenBuffers(1, &stagingBuffer);
    CHECK_GL_ERROR();
    glBindBuffer(GL_ARRAY_BUFFER, stagingBuffer);
    glBufferData(GL_ARRAY_BUFFER, largestVertCount * VERTEX_ATTRIBUTES_COUNT * sizeof(uint32_t), NULL, GL_DYNAMIC_COPY);
    CHECK_GL_ERROR();
    DebugRAM("after glGenBuffers vbos");
    for (uint32_t i = 0; i < MODEL_COUNT; i++) {
        if (modelVertexCounts[i] == 0) continue; // No model specified with this index.
        if (vertexDataArrays[i] == NULL) {
            DualLog("\033[33mWARNING: model %d has invalid vertex data (count=%u, ptr=%p)\033[0m\n",
                    i, modelVertexCounts[i], (void*)vertexDataArrays[i]);
            continue;
        }

        glBindBuffer(GL_ARRAY_BUFFER, stagingBuffer);
        void *mapped_buffer = glMapBufferRange(GL_ARRAY_BUFFER, 0, modelVertexCounts[i] * VERTEX_ATTRIBUTES_COUNT * sizeof(float), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
        if (!mapped_buffer) { DualLogError("Failed to map stagingBuffer for model %d\n", i); continue; }

        memcpy(mapped_buffer, vertexDataArrays[i], modelVertexCounts[i] * VERTEX_ATTRIBUTES_COUNT * sizeof(float));
        glUnmapBuffer(GL_ARRAY_BUFFER);

        // Copy to VBO
        glBindBuffer(GL_COPY_READ_BUFFER, stagingBuffer);
        CHECK_GL_ERROR();
        glBindBuffer(GL_COPY_WRITE_BUFFER, vbos[i]);
        CHECK_GL_ERROR();
        glBufferData(GL_COPY_WRITE_BUFFER, modelVertexCounts[i] * VERTEX_ATTRIBUTES_COUNT * sizeof(float), NULL, GL_STATIC_DRAW);
        CHECK_GL_ERROR();
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, modelVertexCounts[i] * VERTEX_ATTRIBUTES_COUNT * sizeof(float));
        glFlush();
        glFinish();
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    CHECK_GL_ERROR();
    glDeleteBuffers(1, &stagingBuffer);
    CHECK_GL_ERROR();
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    glFlush();
    glFinish();
    CHECK_GL_ERROR();
    DebugRAM("after model staging buffer clear");

    // Upload modelVertexOffsets
    uint32_t modelVertexOffsets[MODEL_COUNT];
    uint32_t offset = 0;
    for (uint32_t i = 0; i < MODEL_COUNT; i++) {
        modelVertexOffsets[i] = offset;
        offset += modelVertexCounts[i];
    }

    glGenBuffers(1, &modelVertexOffsetsID);
    CHECK_GL_ERROR();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelVertexOffsetsID);
    CHECK_GL_ERROR();
    glBufferData(GL_SHADER_STORAGE_BUFFER, MODEL_COUNT * sizeof(uint32_t), modelVertexOffsets, GL_STATIC_DRAW);
    CHECK_GL_ERROR();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, modelVertexOffsetsID);
    CHECK_GL_ERROR();
    
    // Create duplicate of all mesh data in one flat buffer in VRAM without using RAM
    glGenBuffers(1, &vboMasterTable);
    CHECK_GL_ERROR();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vboMasterTable);
    CHECK_GL_ERROR();
    glBufferData(GL_SHADER_STORAGE_BUFFER, totalVertCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float), NULL, GL_STATIC_DRAW);
    CHECK_GL_ERROR();
    for (int i = 0; i < MODEL_COUNT; ++i) {
        glBindBuffer(GL_COPY_READ_BUFFER, vbos[i]);
        CHECK_GL_ERROR();
        glBindBuffer(GL_COPY_WRITE_BUFFER, vboMasterTable);
        CHECK_GL_ERROR();
        glCopyBufferSubData(GL_COPY_READ_BUFFER,GL_COPY_WRITE_BUFFER,0, modelVertexOffsets[i] * VERTEX_ATTRIBUTES_COUNT * sizeof(float), modelVertexCounts[i] * VERTEX_ATTRIBUTES_COUNT * sizeof(float));
        CHECK_GL_ERROR();
    }

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 20, vboMasterTable);
    CHECK_GL_ERROR();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    CHECK_GL_ERROR();
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    DebugRAM("after copying to VBO Master Table");

    // Pass Model Type Bounds to GPU
    glGenBuffers(1, &modelBoundsID);
    CHECK_GL_ERROR();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelBoundsID);
    CHECK_GL_ERROR();
    glBufferData(GL_SHADER_STORAGE_BUFFER, MODEL_COUNT * BOUNDS_ATTRIBUTES_COUNT * sizeof(float), modelBounds, GL_STATIC_DRAW);
    CHECK_GL_ERROR();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, modelBoundsID);
    CHECK_GL_ERROR();
    malloc_trim(0);
    DebugRAM("after loading all models");
    double end_time = get_time();
    DualLog("Load Models took %f seconds\n", end_time - start_time);
    return 0;
}

void CleanupModelLoadOnFail(void) {
    if (loadModelItemInitialized[MDL_PARSER]) parser_free(&model_parser);
    // New: Free edge data arrays
    for (uint32_t i = 0; i < MODEL_COUNT; i++) {
        if (edgeDataArrays[i].edges) {
            free(edgeDataArrays[i].edges);
            edgeDataArrays[i].edges = NULL;
            edgeDataArrays[i].edgeCount = 0;
        }
    }
}
