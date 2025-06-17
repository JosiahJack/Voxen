#include <stdio.h>
#include <stdbool.h>
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

uint32_t modelVertexCounts[MODEL_COUNT];
int32_t vbo_offsets[MODEL_COUNT];
uint32_t totalVertexCount = 0;

int LoadModels(float **vertexData, uint32_t *vertexCount) {
    totalVertexCount = 0;
    uint32_t currentVertexOffset = 0;
    vbo_offsets[0] = 0; // First model starts at 0
    float *tempVertices; // Temporary storage for all vertex data
    bool tempVerticesAlloced = false;
    for (int i = 0; i < MODEL_COUNT; i++) {
        const struct aiScene *scene = aiImportFile(modelPaths[i], // Load FBX file with Assimp
                                                   aiProcess_FindInvalidData        // Removes invalid data such as micro polys, invalid normals (recalc'ed later)
                                                   | aiProcess_Triangulate          // Triangulates any quads or ngons if tweren't already.
                                                   | aiProcess_GenNormals           // Generates normals for verts that don't already have it.
                                                   | aiProcess_ImproveCacheLocality // Fine tunes cache optimization for huge models.  Can help with single draw call
                                                   | aiProcess_FindDegenerates);    // Removes degenerate triangles such as ones with only verts or edges
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            fprintf(stderr, "Assimp failed to load %s: %s\n", modelPaths[i], aiGetErrorString());
            if (tempVerticesAlloced) free(tempVertices);
            return 1;
        }

        // Count total vertices in the model
        uint32_t vertexCount = 0;
        for (unsigned int m = 0; m < scene->mNumMeshes; m++) vertexCount += scene->mMeshes[m]->mNumVertices;
        modelVertexCounts[i] = vertexCount;

        // Allocate space for new vertices
        if (!tempVerticesAlloced) tempVertices = (float *)malloc((totalVertexCount + vertexCount) * 9 * sizeof(float));
        else tempVertices = (float *)realloc(tempVertices, (totalVertexCount + vertexCount) * 9 * sizeof(float));
        
        if (!tempVertices) {
            fprintf(stderr, "Failed to reallocate vertex buffer for %s\n", modelPaths[i]);
            aiReleaseImport(scene);
            return 1;
        }
        
        tempVerticesAlloced = true;

        // Extract vertex data
        uint32_t vertexIndex = totalVertexCount * 9;
        float tempU = 0.0f;
        float tempV = 0.0f;
        for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
            struct aiMesh *mesh = scene->mMeshes[m];
            for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
                tempVertices[vertexIndex++] = mesh->mVertices[v].x; // Position
                tempVertices[vertexIndex++] = mesh->mVertices[v].y;
                tempVertices[vertexIndex++] = mesh->mVertices[v].z;
                tempVertices[vertexIndex++] = mesh->mNormals[v].x; // Normal
                tempVertices[vertexIndex++] = mesh->mNormals[v].y;
                tempVertices[vertexIndex++] = mesh->mNormals[v].z;
                tempU = mesh->mTextureCoords[0] ? mesh->mTextureCoords[0][v].x : 0.0f; // Tex Coord
                tempV = mesh->mTextureCoords[0] ? mesh->mTextureCoords[0][v].y : 0.0f;
                tempVertices[vertexIndex++] = tempU;
                tempVertices[vertexIndex++] = tempV;
                tempVertices[vertexIndex++] = (float)(i); // Texture index matches model index TODO: Use separate texture index from constIndex
            }
            
        }

        totalVertexCount += vertexCount;
        if (i < MODEL_COUNT - 1) vbo_offsets[i] = currentVertexOffset;
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
    if (LoadModels(&vertexData, &vertexCount)) { fprintf(stderr, "Failed to load models!\n"); return 1; }

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * 9 * sizeof(float), vertexData, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0); // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float))); // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float))); // Tex Coord
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(8 * sizeof(float))); // Tex Index
    glEnableVertexAttribArray(3);
    glBindVertexArray(0);
    free(vertexData);
    return 0;
}
