#ifndef VOXEN_INSTANCE_H
#define VOXEN_INSTANCE_H

#include <GL/glew.h>
#include <stdint.h>
#include <stdbool.h>

#define INSTANCE_COUNT 5750 // Max 5454 for Citadel level 7 geometry, Max 295 for Citadel level 1 dynamic objects

typedef struct { // Ensure chunk.glsl struct matches!
    int32_t modelIndex;
    int32_t texIndex;
    int32_t glowIndex;
    int32_t specIndex;
    int32_t normIndex;
    int32_t lodIndex;
    float posx;
    float posy;
    float posz;
    float sclx;
    float scly;
    float sclz;
    float rotx;
    float roty;
    float rotz;
    float rotw;
    float floorHeight;
} Instance;

extern bool instanceInPVS[INSTANCE_COUNT];
extern Instance instances[INSTANCE_COUNT];
extern float modelMatrices[INSTANCE_COUNT * 16];
extern uint8_t dirtyInstances[INSTANCE_COUNT];
extern GLuint instancesBuffer;
extern GLuint instancesInPVSBuffer;
extern GLuint matricesBuffer;
extern int startOfDoubleSidedInstances;
extern int startOfTransparentInstances;

int SetupInstances(void);

#endif // VOXEN_INSTANCE_H
