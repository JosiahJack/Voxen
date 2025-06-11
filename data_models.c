#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "constants.h"
#include "data_models.h"
#include "blender2fbx.h"

const char *modelPaths[MODEL_COUNT] = {
    "./Models/med1_1.fbx",
    "./Models/med1_7.fbx",
    "./Models/med1_9.fbx"
};

// Vertex counts and offsets for each model
uint32_t modelVertexCounts[MODEL_COUNT];
int32_t vbo_offsets[MODEL_COUNT];

// Total vertex buffer size
uint32_t totalVertexCount = 0;

// Cube Geometry (positions, normals, tex coords, texture index)
// Basic cube of width 2.0f
float cubeVertices[] = {
    // Positions          // Normals           // Tex Coords  // Tex Index
    -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,  2.0f, // Top (med1_9.png)
     1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,  2.0f,
     1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,  2.0f,
    -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,  2.0f,

    -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,  1.0f, // Bottom (med1_7.png)
    -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,  0.0f, // Side (med1_1.png)
    -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,  0.0f,
     1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,  0.0f,
     1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,  0.0f,

    -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,  0.0f, // Side (med1_1.png)
     1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,  0.0f,
     1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,  0.0f,
    -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,  0.0f,

     1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,  0.0f, // Side (med1_1.png)
     1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,  0.0f,
     1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,  0.0f,
     1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,  0.0f,

    -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,  0.0f, // Side (med1_1.png)
    -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,  0.0f,
    -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,  0.0f,
    -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,  0.0f
};

void SetupCube(void) {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Tex Coord
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // Tex Index
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glBindVertexArray(0);
}

int LoadModels(float **vertexData, uint32_t *vertexCount) {
    totalVertexCount = sizeof(cubeVertices) / (9 * sizeof(float)); // Start with cube vertices
    vbo_offsets[0] = 0; // First model starts at 0

    // Temporary storage for all vertex data (cube + models)
    float *tempVertices = (float *)malloc(totalVertexCount * 9 * sizeof(float));
    if (!tempVertices) {
        fprintf(stderr, "Failed to allocate temporary vertex buffer\n");
        return 1;
    }
    memcpy(tempVertices, cubeVertices, sizeof(cubeVertices)); // Copy cube vertices

    uint32_t currentVertexOffset = totalVertexCount;

    for (int i = 0; i < MODEL_COUNT; i++) {
        // Load FBX file with Assimp
        const struct aiScene *scene = aiImportFile(
            modelPaths[i],
            aiProcess_Triangulate | aiProcess_GenNormals// | aiProcess_FlipUVs
        );
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            fprintf(stderr, "Assimp failed to load %s: %s\n", modelPaths[i], aiGetErrorString());
            free(tempVertices);
            return 1;
        }

        // Count total vertices in the model
        uint32_t vertexCount = 0;
        for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
            vertexCount += scene->mMeshes[m]->mNumVertices;
        }
        modelVertexCounts[i] = vertexCount;

        // Allocate space for new vertices
        tempVertices = (float *)realloc(tempVertices, (totalVertexCount + vertexCount) * 9 * sizeof(float));
        if (!tempVertices) {
            fprintf(stderr, "Failed to reallocate vertex buffer for %s\n", modelPaths[i]);
            aiReleaseImport(scene);
            return 1;
        }

        // Extract vertex data
        uint32_t vertexIndex = totalVertexCount * 9;
        for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
            struct aiMesh *mesh = scene->mMeshes[m];
            for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
                // Position
                tempVertices[vertexIndex++] = mesh->mVertices[v].x;
                tempVertices[vertexIndex++] = mesh->mVertices[v].y;
                tempVertices[vertexIndex++] = mesh->mVertices[v].z;
                // Normal
                tempVertices[vertexIndex++] = mesh->mNormals[v].x;
                tempVertices[vertexIndex++] = mesh->mNormals[v].y;
                tempVertices[vertexIndex++] = mesh->mNormals[v].z;
                // Tex Coord
                if (mesh->mTextureCoords[0]) {
                    tempVertices[vertexIndex++] = mesh->mTextureCoords[0][v].x;
                    tempVertices[vertexIndex++] = mesh->mTextureCoords[0][v].y;
                } else {
                    tempVertices[vertexIndex++] = 0.0f;
                    tempVertices[vertexIndex++] = 0.0f;
                }
                // Texture Index
                tempVertices[vertexIndex++] = (float)(i); // Texture index matches model index
            }
        }

        totalVertexCount += vertexCount;
        if (i < MODEL_COUNT - 1) {
            vbo_offsets[i + 1] = currentVertexOffset;
        }
        currentVertexOffset += vertexCount;

        aiReleaseImport(scene);
    }

    *vertexData = tempVertices;
    *vertexCount = totalVertexCount;
    return 0;
}

int SetupGeometry(void) {
    float *vertexData = NULL;
    uint32_t vertexCount = 0;
    if (LoadModels(&vertexData, &vertexCount)) {
        fprintf(stderr, "Failed to load models, falling back to cube\n");
        vertexData = cubeVertices;
        vertexCount = sizeof(cubeVertices) / (9 * sizeof(float));
        return 0;
    }

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * 9 * sizeof(float), vertexData, GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Tex Coord
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // Tex Index
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);

    if (vertexData != cubeVertices) {
        free(vertexData); // Free temporary buffer if not using cubeVertices
    }

    return 0;
}
