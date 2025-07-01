#ifndef VOXEN_INSTANCE_H
#define VOXEN_INSTANCE_H

#include <stdint.h>

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

#endif // VOXEN_INSTANCE_H
