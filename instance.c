#include <malloc.h>
#include <GL/glew.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
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
        
        if (idx == 39) { // Test light representative, not actually the light, moves with it
            instances[idx].modelIndex = 620; // Test Light cube
            instances[idx].texIndex = 881; // white light
            instances[idx].glowIndex = 881; // white light
            instances[idx].sclx = 0.16f;
            instances[idx].scly = 0.16f;
            instances[idx].sclz = 0.16f;
        }
        
        dirtyInstances[idx] = true;
//         UpdateInstanceMatrix(idx);
        
//         if (instances[idx].modelIndex < MODEL_COUNT && instances[idx].modelIndex >= 0) {
//             DualLog("Inserting occupied voxel for instance %d\n",idx);
//             InsertOccupiedVoxel(instances[idx].posx,instances[idx].posy,instances[idx].posz);
//         }
        
//         DualLog("Instance %d using entity %d:: mdx: %d, tex: %d, glw: %d, spc: %d, nrm: %d\n",
//                 idx,entIdx,instances[idx].modelIndex,instances[idx].texIndex,instances[idx].glowIndex,instances[idx].specIndex,instances[idx].normIndex);
    }
    
    glGenBuffers(1, &instancesBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, instancesBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, INSTANCE_COUNT * sizeof(Instance), instances, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, instancesBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    
//     DualLog("Total instance size ");
//     print_bytes_no_newline(INSTANCE_COUNT * 11 * 4);
//     DualLog("\n");
    
    memset(modelMatrices, 0, INSTANCE_COUNT * 16 * sizeof(float)); // Matrix4x4 = 16
    glGenBuffers(1, &matricesBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, matricesBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, matricesBuffer);
    
    malloc_trim(0);

    InitLightVolumes(); // Create Light Volume meshes
    return 0;
}

void UpdateInstanceMatrix(int i) {
    if (instances[i].modelIndex >= MODEL_COUNT) { dirtyInstances[i] = false; return; }
    if (modelVertexCounts[instances[i].modelIndex] < 1) { dirtyInstances[i] = false; return; } // Empty model
    if (instances[i].modelIndex < 0) return; // Culled
    
    float x = instances[i].rotx, y = instances[i].roty, z = instances[i].rotz, w = instances[i].rotw;
    float len = sqrtf(x*x + y*y + z*z + w*w);
    if (len > 0.0f) {
        x /= len; y /= len; z /= len; w /= len;
    } else {
        x = 0.0f; y = 0.0f; z = 0.0f; w = 1.0f; // Default to identity rotation
    }
    
    float rot[16];
    mat4_identity(rot);
    rot[0] = 1.0f - 2.0f * (y*y + z*z);
    rot[1] = 2.0f * (x*y - w*z);
    rot[2] = 2.0f * (x*z + w*y);
    rot[4] = 2.0f * (x*y + w*z);
    rot[5] = 1.0f - 2.0f * (x*x + z*z);
    rot[6] = 2.0f * (y*z - w*x);
    rot[8] = 2.0f * (x*z - w*y);
    rot[9] = 2.0f * (y*z + w*x);
    rot[10] = 1.0f - 2.0f * (x*x + y*y);
    
    // Account for bad scale.  If instance is in the list, it should be visible!
    float sx = instances[i].sclx > 0.0f ? instances[i].sclx : 1.0f;
    float sy = instances[i].scly > 0.0f ? instances[i].scly : 1.0f;
    float sz = instances[i].sclz > 0.0f ? instances[i].sclz : 1.0f;
    
    float mat[16]; // 4x4 matrix
    mat[0]  =       rot[0] * sx; mat[1]  =       rot[1] * sy; mat[2] =       rot[2]  * sz; mat[3]  = 0.0f;
    mat[4]  =       rot[4] * sx; mat[5]  =       rot[5] * sy; mat[6]  =      rot[6]  * sz; mat[7]  = 0.0f;
    mat[8]  =       rot[8] * sx; mat[9]  =       rot[9] * sy; mat[10] =      rot[10] * sz; mat[11] = 0.0f;
    mat[12] = instances[i].posx; mat[13] = instances[i].posy; mat[14] = instances[i].posz; mat[15] = 1.0f;
    memcpy(&modelMatrices[i * 16], mat, 16 * sizeof(float));
    dirtyInstances[i] = false;
}
