#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "transform.h"
#include "render.h"
#include "constants.h"
#include "data_models.h"
#include "blender2fbx.h"

float modelRadius[MODEL_COUNT];

const char *modelPaths[MODEL_COUNT] = {
    "./Models/med1_1.fbx",
    "./Models/med1_7.fbx",
    "./Models/med1_9.fbx"
};

uint32_t modelVertexCounts[MODEL_COUNT];
GLuint vbo_offsets[MODEL_COUNT];
uint32_t totalVertexCount = 0;

int LoadModels(float **vertexData, uint32_t *vertexCount) {
    totalVertexCount = 0;
    vbo_offsets[0] = 0; // First model starts at 0
    float *tempVertices = NULL;
    bool tempVerticesAlloced = false;
    for (int i = 0; i < MODEL_COUNT; i++) {
        const struct aiScene *scene = aiImportFile(modelPaths[i],
                                                   aiProcess_FindInvalidData
                                                   | aiProcess_Triangulate
                                                   | aiProcess_GenNormals
                                                   | aiProcess_ImproveCacheLocality
                                                   | aiProcess_FindDegenerates); // aiProcess_FlipWindingOrder if computer rasterizer renders nothing.
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            fprintf(stderr, "Assimp failed to load %s: %s\n", modelPaths[i], aiGetErrorString());
            if (tempVerticesAlloced) free(tempVertices);
            return 1;
        }

        // Count total vertices in the model
        uint32_t vertexCount = 0;
        for (unsigned int m = 0; m < scene->mNumMeshes; m++) vertexCount += scene->mMeshes[m]->mNumVertices;
        modelVertexCounts[i] = vertexCount;
        printf("Model %s loaded with %d vertices\n", modelPaths[i], vertexCount);

        
        // Allocate space for new vertices
        if (!tempVerticesAlloced) tempVertices = (float *)malloc(vertexCount * 9 * sizeof(float));
        else tempVertices = (float *)realloc(tempVertices, (totalVertexCount + vertexCount) * 9 * sizeof(float));
        
        if (!tempVertices) {
            fprintf(stderr, "Failed to reallocate vertex buffer for %s\n", modelPaths[i]);
            aiReleaseImport(scene);
            return 1;
        }
        tempVerticesAlloced = true;

        // Extract vertex data
        uint32_t vertexIndex = totalVertexCount * 9;
        float minx, miny, minz;
        float maxx, maxy, maxz;
        minx = miny = minz = 1e9;
        maxx = maxy = maxz = -1e9;
        for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
            struct aiMesh *mesh = scene->mMeshes[m];
            for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
                tempVertices[vertexIndex++] = mesh->mVertices[v].x; // Position
                if (mesh->mVertices[v].x < minx) minx = mesh->mVertices[v].x;
                if (mesh->mVertices[v].x > maxx) maxx = mesh->mVertices[v].x;
                tempVertices[vertexIndex++] = mesh->mVertices[v].y;
                if (mesh->mVertices[v].y < miny) miny = mesh->mVertices[v].y;
                if (mesh->mVertices[v].y > maxy) maxy = mesh->mVertices[v].y;
                tempVertices[vertexIndex++] = mesh->mVertices[v].z;
                if (mesh->mVertices[v].x < minz) minz = mesh->mVertices[v].z;
                if (mesh->mVertices[v].x > maxz) maxz = mesh->mVertices[v].z;
                tempVertices[vertexIndex++] = mesh->mNormals[v].x; // Normal
                tempVertices[vertexIndex++] = mesh->mNormals[v].y;
                tempVertices[vertexIndex++] = mesh->mNormals[v].z;
                float tempU = mesh->mTextureCoords[0] ? mesh->mTextureCoords[0][v].x : 0.0f;
                float tempV = mesh->mTextureCoords[0] ? mesh->mTextureCoords[0][v].y : 0.0f;
                tempVertices[vertexIndex++] = tempU;
                tempVertices[vertexIndex++] = tempV;
                tempVertices[vertexIndex++] = (float)i; // TexIndex
            }
        }
        
        float extentMinMost = fabs(minx) > fabs(miny) ? fabs(minx) : fabs(miny);
        extentMinMost = extentMinMost > fabs(minz) ? extentMinMost : fabs(minz);
        float extentMaxMost = maxx > maxy ? maxx : maxy;
        extentMaxMost = extentMaxMost > maxz ? extentMaxMost : maxz;
        modelRadius[i] = extentMaxMost > extentMinMost ? extentMaxMost : extentMinMost;
        modelVertexCounts[i] = vertexCount;
        printf("Model %d radius=%f\n",i,modelRadius[i]);
        vbo_offsets[i] = totalVertexCount;
        totalVertexCount += vertexCount;
        printf("Cumulative totalVertexCount %d\n",totalVertexCount);
        aiReleaseImport(scene);
    }

    
    *vertexData = tempVertices;
    *vertexCount = totalVertexCount;

    // Log offsets and counts
    printf("modelVertexCounts 0 %d 1 %d 2 %d\n",modelVertexCounts[0],modelVertexCounts[1],modelVertexCounts[2]);
    printf("vbo_offsets 0 %d 1 %d 2 %d\n",vbo_offsets[0],vbo_offsets[1],vbo_offsets[2]);
    return 0;
}
