#include <malloc.h>
#include <GL/glew.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "render.h"
#include "instance.h"
#include "data_models.h"
#include "debug.h"
#include "data_entities.h"
#include "voxel.h"
#include "matrix.h"

Instance instances[INSTANCE_COUNT];
float modelMatrices[INSTANCE_COUNT * 16];
uint8_t dirtyInstances[INSTANCE_COUNT];
GLuint instancesBuffer;
GLuint matricesBuffer;

int SetupInstances(void) {    
    CHECK_GL_ERROR();
    int x = 0;
    int y = 0;
    for (int idx=0;idx<INSTANCE_COUNT;idx++) {
        int entIdx = idx < MAX_ENTITIES ? idx : 0;
        instances[idx].modelIndex = entities[entIdx].modelIndex;
        instances[idx].texIndex = entities[entIdx].texIndex;
        instances[idx].glowIndex = entities[entIdx].glowIndex;
        instances[idx].specIndex = entities[entIdx].specIndex;
        instances[idx].normIndex = entities[entIdx].normIndex;
        instances[idx].posx = ((float)x * 2.56f); // Position in grid with gaps for shadow testing.
        instances[idx].posy = ((float)y * 5.12f);
        instances[idx].posz = 0.0f;
        instances[idx].sclx = instances[idx].scly = instances[idx].sclz = 1.0f; // Default scale
        instances[idx].rotx = instances[idx].roty = instances[idx].rotz = 0.0f;
        instances[idx].rotw = 1.0f;
        x++;
        if (idx == 100 || idx == 200 || idx == 300 || idx == 400 || idx == 500
            || idx == 600 || idx == 700 || idx == 800 || idx == 900) {
            
            x = 0;
            y++;
        }
        
        if (idx == 39) { // Test light representative, not actually the light, moves with it
            instances[idx].modelIndex = 620; // Test Light cube
            instances[idx].texIndex = 881; // white light
            instances[idx].glowIndex = 881; // white light
            instances[idx].sclx = 0.16f;
            instances[idx].scly = 0.16f;
            instances[idx].sclz = 0.16f;
        }
        
        dirtyInstances[idx] = true;        
    }
    
    glGenBuffers(1, &instancesBuffer);
    CHECK_GL_ERROR();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, instancesBuffer);
    CHECK_GL_ERROR();
    glBufferData(GL_SHADER_STORAGE_BUFFER, INSTANCE_COUNT * sizeof(Instance), instances, GL_DYNAMIC_DRAW);
    CHECK_GL_ERROR();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, instancesBuffer);
    CHECK_GL_ERROR();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    CHECK_GL_ERROR();
    memset(modelMatrices, 0, INSTANCE_COUNT * 16 * sizeof(float)); // Matrix4x4 = 16
    glGenBuffers(1, &matricesBuffer);
    CHECK_GL_ERROR();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, matricesBuffer);
    CHECK_GL_ERROR();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, matricesBuffer);
    CHECK_GL_ERROR();
    malloc_trim(0);
    return 0;
}
