#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "constants.h"
#include "data_models.h"
#include "debug.h"
#include "render.h"

const char *modelPaths[MODEL_COUNT] = {
    "./Models/med1_1.fbx",
    "./Models/med1_7.fbx",
    "./Models/med1_9.fbx",
    "./Models/crate1.fbx",
    "./Models/test_cube.fbx",
    "./Models/test_light.fbx"
};

GLuint chunkShaderProgram;
uint32_t modelVertexCounts[MODEL_COUNT];
GLint modelTriangleCounts[MODEL_COUNT];
GLuint vao; // Vertex Array Object
GLuint vbos[MODEL_COUNT];
int32_t vbo_offsets[MODEL_COUNT];
uint32_t totalVertexCount = 0;
GLuint modelBoundsID;
float modelBounds[MODEL_COUNT * BOUNDS_ATTRIBUTES_COUNT];

int LoadModels(float *vertexDataArrays[MODEL_COUNT], uint32_t vertexCounts[MODEL_COUNT]) {
    int totalVertCount = 0;
    int totalBounds = 0;
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
        uint32_t triCount = 0;
        for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
            vertexCount += scene->mMeshes[m]->mNumVertices;
            triCount += scene->mMeshes[m]->mNumFaces;
        }

        modelVertexCounts[i] = vertexCount;
        modelTriangleCounts[i] = (GLint)triCount;//(vertexCount / 3);
        printf("Model %s loaded with %d vertices, %d tris\n", modelPaths[i], vertexCount, modelTriangleCounts[i]);
        totalVertCount += vertexCount;

        // Allocate vertex data for this model
        float *tempVertices = (float *)malloc(vertexCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float));
        if (!tempVertices) {
            fprintf(stderr, "Failed to reallocate vertex buffer for %s\n", modelPaths[i]);
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
                
                if (mesh->mVertices[v].x < minx) minx = mesh->mVertices[v].x;
                if (mesh->mVertices[v].x > maxx) maxx = mesh->mVertices[v].x;
                if (mesh->mVertices[v].y < miny) miny = mesh->mVertices[v].y;
                if (mesh->mVertices[v].y > maxy) maxy = mesh->mVertices[v].y;
                if (mesh->mVertices[v].z < minz) minz = mesh->mVertices[v].z;
                if (mesh->mVertices[v].z > maxz) maxz = mesh->mVertices[v].z;
            }
        }

        modelBounds[(i * BOUNDS_ATTRIBUTES_COUNT) + 0] = minx;
        modelBounds[(i * BOUNDS_ATTRIBUTES_COUNT) + 1] = miny;
        modelBounds[(i * BOUNDS_ATTRIBUTES_COUNT) + 2] = minz;
        modelBounds[(i * BOUNDS_ATTRIBUTES_COUNT) + 3] = maxx;
        modelBounds[(i * BOUNDS_ATTRIBUTES_COUNT) + 4] = maxy;
        modelBounds[(i * BOUNDS_ATTRIBUTES_COUNT) + 5] = maxz;
        totalBounds += 6;
        printf("Model index %d minx %f, miny %f, minz %f ;; maxx %f, maxy %f, maxz %f\n",i,minx,miny,minz,maxx,maxy,maxz);
        vertexDataArrays[i] = tempVertices;
        vertexCounts[i] = vertexCount;
        aiReleaseImport(scene);
    }

    // Log vertex counts
    printf("Total vertices in buffer %d (",totalVertCount);
    print_bytes_no_newline(totalVertCount * 8 * 4); // x,y,z,nx,ny,nz,u,v * 4
    printf(") (Bounds ");
    print_bytes_no_newline( totalBounds * 6 * 4); //minx,miny,minz,maxx,maxy,maxz * 4 
    printf(")\n");
    return 0;
}

// Loads all geometry, from 3D meshes or otherwise
int LoadGeometry(void) {
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
    
    glGenBuffers(1, &modelBoundsID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelBoundsID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, MODEL_COUNT * BOUNDS_ATTRIBUTES_COUNT * sizeof(float), modelBounds, GL_STATIC_DRAW);
    
    // Send static uniforms to chunk shader
    glUniform1ui(modelCountLoc_chunk, MODEL_COUNT);
    
    // Set static buffers once for Deferred Lighting shader
    glUniform1ui(screenWidthLoc_deferred, screen_width);
    glUniform1ui(screenHeightLoc_deferred, screen_height);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, vbos[0]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, vbos[1]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, vbos[2]);

    return 0;
}
