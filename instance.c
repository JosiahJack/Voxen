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
#include "quaternion.h"
#include "constants.h"

Instance instances[INSTANCE_COUNT];
float modelMatrices[INSTANCE_COUNT * 16];
uint8_t dirtyInstances[INSTANCE_COUNT];
GLuint instancesBuffer;
GLuint instancesInPVSBuffer;
GLuint matricesBuffer;

int SetupInstances(void) {
    DualLog("Initializing instances\n");
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
        
        if (idx == 5455) { // Test light representative, not actually the light, moves with it
            instances[idx].modelIndex = 621; // Test Light Sphere
            instances[idx].texIndex = 881; // white light
            instances[idx].glowIndex = 881; // white light
            instances[idx].sclx = 0.4f;
            instances[idx].scly = 0.4f;
            instances[idx].sclz = 0.4f;
        }
        
        dirtyInstances[idx] = true;        
    }
    
    // Test orienteering capability of existing quaternion code
    for (int i = 0; i < 6; ++i) {
        instances[i].posx = -2.56f;
        instances[i].posy = -2.56f;
        instances[i].posz = 0.0f;
        instances[i].modelIndex = 178; // generic lod card chunk
    }
    
    // Barrel
    instances[458].posx = -2.2f;
    instances[458].posy = -2.0f;
    instances[458].posz = -1.28f;
    instances[458].rotx = 0.0f;
    instances[458].roty = 0.0f;
    instances[458].rotz = 0.357f;
    instances[458].rotw = 0.934f;
    instances[458].modelIndex = 12;
    instances[458].texIndex = 30;
    instances[458].specIndex = 32;
    
    // Crate
    instances[472].posx = -2.2f;
    instances[472].posy = -3.4f;
    instances[472].posz = -1.28f;
    instances[472].rotx = 0.0f;
    instances[472].roty = 0.0f;
    instances[472].rotz = 0.199f;
    instances[472].rotw = 0.980f;
    instances[472].modelIndex = 60;
    instances[472].texIndex = 145;
    instances[472].glowIndex = 65535;
    instances[472].sclx = 1.0f;
    instances[472].scly = 1.0f;
    instances[472].sclz = 1.0f;
    
    // Cornell Box
    instances[0].texIndex = 513; // North med2_1
    instances[0].specIndex = 1242;
    instances[1].texIndex = 515; // South med2_1d
    instances[1].specIndex = 1242;
    instances[2].texIndex = 483; // East maint3_1
    instances[3].texIndex = 482; // West maint3_1d 
    instances[4].texIndex = 499; // floor med1_9 bright teal light
    instances[4].specIndex = 1242;
    instances[5].texIndex = 507; // ceil med1_7 medical tile floor
    instances[5].specIndex = 1236;
    
    
    // 0: Identity rotation (cell North side Y+ from cell center)
    Quaternion q;
    quat_identity(&q);
    instances[0].rotx = q.x;
    instances[0].roty = q.y;
    instances[0].rotz = q.z;
    instances[0].rotw = q.w;
//     printf("North cell side Y+ from cell center quat:: x: %f, y: %f, z: %f, w: %f\n",q.x,q.y,q.z,q.w);
    // Equates to Unity localRotation:
    // Inspector rotation -90, 0, 180, quaternion x: 0 y: 0.70711 z:0.70711 w: 0

    // 1: 180° around Z (cell South side Y- from cell center)
    quat_from_axis_angle(&q, 0.0f, 0.0f, 1.0f, M_PI);
    instances[1].rotx = q.x;
    instances[1].roty = q.y;
    instances[1].rotz = q.z;
    instances[1].rotw = q.w;
//     printf("South cell side Y- from cell center quat:: x: %f, y: %f, z: %f, w: %f\n",q.x,q.y,q.z,q.w);
    // Equates to Unity localRotation:
    // Inspector rotation -90, 0, 0, quaternion x: -0.70711, y: 0, z: 0, w: 0.70711
    
    // 2: +90° around Z (cell East side X+ from cell center)
    quat_from_axis_angle(&q, 0.0f, 0.0f, 1.0f, M_PI / 2.0f);
    instances[2].rotx = q.x;
    instances[2].roty = q.y;
    instances[2].rotz = q.z;
    instances[2].rotw = q.w;
//     printf("East cell side X+ from cell center quat:: x: %f, y: %f, z: %f, w: %f\n",q.x,q.y,q.z,q.w);

    // 3: −90° around Z (cell West side X− from cell center)
    quat_from_axis_angle(&q, 0.0f, 0.0f, 1.0f, -M_PI / 2.0f);
    instances[3].rotx = q.x;
    instances[3].roty = q.y;
    instances[3].rotz = q.z;
    instances[3].rotw = q.w;
//     printf("West cell side X- from cell center quat:: x: %f, y: %f, z: %f, w: %f\n",q.x,q.y,q.z,q.w);

    // 4: +90° around X (cell Floor Z− from cell center)
    quat_from_axis_angle(&q, 1.0f, 0.0f, 0.0f, -M_PI / 2.0f);
    instances[4].rotx = q.x;
    instances[4].roty = q.y;
    instances[4].rotz = q.z;
    instances[4].rotw = q.w;
//     printf("Down Z- from cell center quat:: x: %f, y: %f, z: %f, w: %f\n",q.x,q.y,q.z,q.w);
    
    // 5: −90° around X (cell Ceil Z+ from cell center)
    quat_from_axis_angle(&q, 1.0f, 0.0f, 0.0f, M_PI / 2.0f);
    instances[5].rotx = q.x;
    instances[5].roty = q.y;
    instances[5].rotz = q.z;
    instances[5].rotw = q.w;
//     printf("Up Z+ from cell center quat:: x: %f, y: %f, z: %f, w: %f\n",q.x,q.y,q.z,q.w);
    // Equates to Unity localRotation:
    // Inspector rotation -180, 0, 0, quaternion x: 1, y: 0, z: 0, w: 0

// North cell side Y+ from cell center quat:: x: 0.000000, y: 0.000000, z: 0.000000, w: 1.000000  = Unity X+, backtick toward Y+ (0deg roll?)
// South cell side Y- from cell center quat:: x: 0.000000, y: 0.000000, z: 1.000000, w: -0.000000 = Unity X-, backtick toward Y- (0deg roll?)
// East cell side X+ from cell center quat:: x: 0.000000, y: 0.000000, z: 0.707107, w: 0.707107   = Unity
// West cell side X- from cell center quat:: x: -0.000000, y: -0.000000, z: -0.707107, w: 0.707107= Unity
// Down Z- from cell center quat:: x: -0.707107, y: -0.000000, z: -0.000000, w: 0.707107          = Unity
// Up Z+ from cell center quat:: x: 0.707107, y: 0.000000, z: 0.000000, w: 0.707107               = Unity X+, backtick toward Z+ (-90deg roll??)

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
    
    glGenBuffers(1, &instancesInPVSBuffer);
    CHECK_GL_ERROR();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, instancesInPVSBuffer);
    CHECK_GL_ERROR();
    glBufferData(GL_SHADER_STORAGE_BUFFER, INSTANCE_COUNT * sizeof(uint32_t), NULL, GL_DYNAMIC_DRAW);
    CHECK_GL_ERROR();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, instancesInPVSBuffer);
    CHECK_GL_ERROR();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    CHECK_GL_ERROR();
    
    memset(modelMatrices, 0, INSTANCE_COUNT * 16 * sizeof(float)); // Matrix4x4 = 16
    glGenBuffers(1, &matricesBuffer);
    CHECK_GL_ERROR();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, matricesBuffer);
    CHECK_GL_ERROR();
    glBufferData(GL_SHADER_STORAGE_BUFFER, INSTANCE_COUNT * 16 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    CHECK_GL_ERROR();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, matricesBuffer);
    CHECK_GL_ERROR();
    malloc_trim(0);
    return 0;
}
