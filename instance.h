#ifndef VOXEN_INSTANCE_H
#define VOXEN_INSTANCE_H

#include <GL/glew.h>
#include <stdint.h>
#include <stdbool.h>

#define INSTANCE_COUNT 5500 // Max 5454 for Citadel level 7

typedef struct {
    int32_t modelIndex; // offset 0, size 4b
    int32_t texIndex;
    int32_t glowIndex;
    int32_t specIndex;
    int32_t normIndex;
    float posx; // offset 4, size 4b
    float posy; // offset 8, size 4b
    float posz; // offset 12, size 4b
    float sclx; // offset 16, size 4b
    float scly; // offset 20, size 4b
    float sclz; // offset 24, size 4b
    float rotx; // offset 28, size 4b
    float roty; // offset 32, size 4b
    float rotz; // offset 36, size 4b
    float rotw; // offset 40, size 4b
} Instance;

extern bool instanceInPVS[INSTANCE_COUNT];
extern Instance instances[INSTANCE_COUNT];
extern float modelMatrices[INSTANCE_COUNT * 16];
extern uint8_t dirtyInstances[INSTANCE_COUNT];
extern GLuint instancesBuffer;
extern GLuint matricesBuffer;

int SetupInstances(void);
void UpdateInstanceMatrix(int i);

#endif // VOXEN_INSTANCE_H
