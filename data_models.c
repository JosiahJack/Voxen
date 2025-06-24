#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "constants.h"
#include "data_models.h"

const char *modelPaths[MODEL_COUNT] = {
    "./Models/med1_1.fbx",
    "./Models/med1_7.fbx",
    "./Models/med1_9.fbx"
};

GLuint chunkShaderProgram;
uint32_t modelVertexCounts[MODEL_COUNT];
GLint modelTriangleCounts[MODEL_COUNT];
GLuint vao; // Vertex Array Object
GLuint vbos[MODEL_COUNT];
int32_t vbo_offsets[MODEL_COUNT];
uint32_t totalVertexCount = 0;

int LoadModels(float *vertexDataArrays[MODEL_COUNT], uint32_t vertexCounts[MODEL_COUNT]) {
    for (int i = 0; i < MODEL_COUNT; i++) {
        const struct aiScene *scene = aiImportFile(modelPaths[i],
                                                    aiProcess_FindInvalidData
                                                    | aiProcess_Triangulate
                                                    | aiProcess_GenNormals
                                                    | aiProcess_ImproveCacheLocality
                                                    | aiProcess_FindDegenerates);
        
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            fprintf(stderr, "Assimp failed to load %s: %s\n", modelPaths[i], aiGetErrorString());
            return 1;
        }

        // Count total vertices in the model
        uint32_t vertexCount = 0;
        for (unsigned int m = 0; m < scene->mNumMeshes; m++) vertexCount += scene->mMeshes[m]->mNumVertices;

        modelVertexCounts[i] = vertexCount;
        modelTriangleCounts[i] = (GLint)(vertexCount / 3);
        printf("Model %s loaded with %d vertices, %d tris\n", modelPaths[i], vertexCount, modelTriangleCounts[i]);

        // Allocate vertex data for this model
        float *tempVertices = (float *)malloc(vertexCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float));
        if (!tempVertices) {
            fprintf(stderr, "Failed to reallocate vertex buffer for %s\n", modelPaths[i]);
            aiReleaseImport(scene);
            return 1;
        }

        // Extract vertex data
        uint32_t vertexIndex = 0;
        for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
            struct aiMesh *mesh = scene->mMeshes[m];
            for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
                tempVertices[vertexIndex++] = mesh->mVertices[v].x; // Position
                tempVertices[vertexIndex++] = mesh->mVertices[v].y;
                tempVertices[vertexIndex++] = mesh->mVertices[v].z;
                tempVertices[vertexIndex++] = mesh->mNormals[v].x; // Normal
                tempVertices[vertexIndex++] = mesh->mNormals[v].y;
                tempVertices[vertexIndex++] = mesh->mNormals[v].z;
                float tempU = mesh->mTextureCoords[0] ? mesh->mTextureCoords[0][v].x : 0.0f;
                float tempV = mesh->mTextureCoords[0] ? mesh->mTextureCoords[0][v].y : 0.0f;
                tempVertices[vertexIndex++] = tempU;
                tempVertices[vertexIndex++] = tempV;
            }
        }

        vertexDataArrays[i] = tempVertices;
        vertexCounts[i] = vertexCount;
        aiReleaseImport(scene);
    }

    // Log vertex counts
    printf("modelVertexCounts 0 %d 1 %d 2 %d\n", modelVertexCounts[0], modelVertexCounts[1], modelVertexCounts[2]);
    return 0;
}

int SetupGeometry(void) {
    float *vertexDataArrays[MODEL_COUNT];
    uint32_t vertexCounts[MODEL_COUNT];
    if (LoadModels(vertexDataArrays, vertexCounts)) {
        fprintf(stderr, "Failed to load models!\n");
        return 1;
    }

    // Generate and bind VAO
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Generate and populate VBOs
    glGenBuffers(MODEL_COUNT, vbos);
    for (int i = 0; i < MODEL_COUNT; i++) {
        glBindBuffer(GL_ARRAY_BUFFER, vbos[i]);
        glBufferData(GL_ARRAY_BUFFER, vertexCounts[i] * VERTEX_ATTRIBUTES_COUNT * sizeof(float), vertexDataArrays[i], GL_STATIC_DRAW);
        free(vertexDataArrays[i]); // Free after uploading to GPU
    }

    // Define vertex attribute formats
    glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0); // Position (vec3)
    glVertexAttribFormat(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float)); // Normal (vec3)
    glVertexAttribFormat(2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float)); // Tex Coord (vec2)

    // Bind attributes to a single binding point (0)
    glVertexAttribBinding(0, 0); // Position
    glVertexAttribBinding(1, 0); // Normal
    glVertexAttribBinding(2, 0); // Tex Coord
    glVertexAttribBinding(3, 0); // Tex Index

    // Enable attributes
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    // Initial binding (optional, will be overridden in RenderStaticMeshes)
    glBindVertexBuffer(0, vbos[0], 0, VERTEX_ATTRIBUTES_COUNT * sizeof(float));

    glBindVertexArray(0);
    return 0;
}
