#include <GL/glew.h>
#include <stdio.h>
#include <string.h>
#include "instance.h"
#include "data_models.h"
#include "debug.h"
#include "data_entities.h"

Instance instances[INSTANCE_COUNT];
float modelMatrices[INSTANCE_COUNT * 16];
GLuint instancesBuffer;
GLuint matricesBuffer;

int SetupInstances(void) {    
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
        instances[idx].sclx = 1.0f; // Default scale
        instances[idx].scly = 1.0f;
        instances[idx].sclz = 1.0f;
        instances[idx].rotx = 0.0f;
        instances[idx].roty = 0.0f;
        instances[idx].rotz = 0.0f;
        instances[idx].rotw = 1.0f;
        x++;
        if (idx == 100 || idx == 200 || idx == 300 || idx == 400 || idx == 500 || idx == 600 || idx == 700 || idx == 800 || idx == 900) {
            x = 0;
            y++;
        }
        
//         DualLog("Instance %d using entity %d:: mdx: %d, tex: %d, glw: %d, spc: %d, nrm: %d\n",
//                 idx,entIdx,instances[idx].modelIndex,instances[idx].texIndex,instances[idx].glowIndex,instances[idx].specIndex,instances[idx].normIndex);
    }
    
    instances[39].modelIndex = 620; // Test Light cube
    instances[39].texIndex = 881; // white light
    instances[39].sclx = 0.16f;
    instances[39].scly = 0.16f;
    instances[39].sclz = 0.16f;
    
//     instances[0].rotx = 0.707f;
//     instances[0].roty = 0.0f;
//     instances[0].rotz = 0.0f;
//     instances[0].rotw = 0.707f;
    
    glGenBuffers(1, &instancesBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, instancesBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, INSTANCE_COUNT * sizeof(Instance), instances, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, instancesBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    
    DualLog("Total instance size ");
    print_bytes_no_newline(INSTANCE_COUNT * 11 * 4);
    DualLog("\n");
    
    memset(modelMatrices, 0, INSTANCE_COUNT * 16 * sizeof(float)); // Matrix4x4 = 16
    glGenBuffers(1, &matricesBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, matricesBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, matricesBuffer);
    return 0;
}
