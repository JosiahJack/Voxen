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
#include "data_parser.h"
#include "voxel.h"

DataParser model_parser;
const char *valid_mdldata_keys[] = {"index"};
GLuint chunkShaderProgram;
uint32_t modelVertexCounts[MODEL_COUNT];
GLuint vao; // Vertex Array Object
GLuint vbos[MODEL_COUNT];
uint32_t totalVertexCount = 0;
GLuint modelBoundsID;
float modelBounds[MODEL_COUNT * BOUNDS_ATTRIBUTES_COUNT];
float * vertexDataArrays[MODEL_COUNT];

typedef enum {
    MDL_PARSER = 0,
    MDL_COUNT // Number of subsystems
} ModelLoadDataType;

bool loadModelItemInitialized[MDL_COUNT] = { [0 ... MDL_COUNT - 1] = false };

// Loads all geometry, from 3D meshes or otherwise
int LoadGeometry(void) {
    // First parse ./Data/textures.txt to see what textures to load to what indices
    parser_init(&model_parser, valid_mdldata_keys, 1, PARSER_DATA);
    if (!parse_data_file(&model_parser, "./Data/models.txt")) {
        printf("ERROR: Could not parse ./Data/models.txt!\n");
        parser_free(&model_parser);
        return 1;
    }
    
    loadModelItemInitialized[MDL_PARSER] = true;
    int maxIndex = -1;
    for (int k=0;k<model_parser.count;k++) {
        if (model_parser.entries[k].index > maxIndex && model_parser.entries[k].index != UINT16_MAX) {maxIndex = model_parser.entries[k].index; }
    }
        
    printf("Parsing %d models out of max allowed %d...\n",model_parser.count,MODEL_COUNT);
    
    int totalVertCount = 0;
    int totalBounds = 0;
    for (uint32_t i = 0; i < MODEL_COUNT; i++) {
        int matchedParserIdx = -1;
        for (int k=0;k<model_parser.count;k++) {
            if (model_parser.entries[k].index == i) {matchedParserIdx = k; break; }
        }
        
        if (matchedParserIdx < 0) continue;
        if (!model_parser.entries[matchedParserIdx].path || model_parser.entries[matchedParserIdx].path[0] == '\0') continue;
        
        const struct aiScene *scene = aiImportFile(model_parser.entries[matchedParserIdx].path,
//                                                     aiProcess_FindInvalidData
//                                                     | aiProcess_Triangulate
//                                                     | aiProcess_GenNormals
                                                     aiProcess_ImproveCacheLocality
                                                    | aiProcess_FindDegenerates
                                                    | aiProcess_LimitBoneWeights);
        
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            fprintf(stderr, "Assimp failed to load %s: %s\n", model_parser.entries[matchedParserIdx].path, aiGetErrorString());
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
//         printf("Model %s loaded with %d vertices, %d tris\n", model_parser.entries[matchedParserIdx].path, vertexCount, triCount);
        totalVertCount += vertexCount;

        // Allocate vertex data for this model
        float *tempVertices = (float *)malloc(vertexCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float));
        if (!tempVertices) {
            fprintf(stderr, "Failed to allocate vertex buffer for %s\n", model_parser.entries[matchedParserIdx].path);
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
        vertexDataArrays[i] = tempVertices;
        aiReleaseImport(scene);
    }

    // Log vertex counts
    printf("Total vertices in buffer %d (",totalVertCount);
    print_bytes_no_newline(totalVertCount * 8 * 4); // x,y,z,nx,ny,nz,u,v * 4
    printf(") (Bounds ");
    print_bytes_no_newline(totalBounds * 4); //minx,miny,minz,maxx,maxy,maxz * 4 
    printf(")\n");

    // Generate and bind VAO
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    memset(vbos, 0, MODEL_COUNT * sizeof(uint32_t));
        
    // Generate and populate VBOs
    glGenBuffers(MODEL_COUNT, vbos);
    for (uint32_t i = 0; i < MODEL_COUNT; i++) {
        if (modelVertexCounts[i] == 0) continue; // No model specified with this index.
        if (vertexDataArrays[i] == NULL) {
            printf("WARNING: model %d has invalid vertex data (count=%u, ptr=%p)\n",
                i, modelVertexCounts[i], (void*)vertexDataArrays[i]);
            continue;
        }
        
        glBindBuffer(GL_ARRAY_BUFFER, vbos[i]);
        glBufferData(GL_ARRAY_BUFFER, modelVertexCounts[i] * VERTEX_ATTRIBUTES_COUNT * sizeof(float), vertexDataArrays[i], GL_STATIC_DRAW);
    }
    
    // Define vertex attribute formats
    glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0); // Position (vec3)
    glVertexAttribFormat(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float)); // Normal (vec3)
    glVertexAttribFormat(2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float)); // Tex Coord (vec2)
    glVertexAttribFormat(3, 1, GL_INT, GL_FALSE, 6 * sizeof(float)); // Tex Index (int)
    
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

    glBindVertexArray(0);
    
    glGenBuffers(1, &modelBoundsID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelBoundsID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, MODEL_COUNT * BOUNDS_ATTRIBUTES_COUNT * sizeof(float), modelBounds, GL_STATIC_DRAW);
    
    // Set static buffers once for Deferred Lighting shader
    glUniform1ui(screenWidthLoc_deferred, screen_width);
    glUniform1ui(screenHeightLoc_deferred, screen_height);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, vbos[0]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, vbos[1]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, vbos[2]);

    return 0;
}

void CleanupModelLoadOnFail(void) {
    if (loadModelItemInitialized[MDL_PARSER]) parser_free(&model_parser);
}
