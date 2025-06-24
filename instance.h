#ifndef VOXEN_INSTANCE_H
#define VOXEN_INSTANCE_H

#include <stdint.h>

typedef struct {
    uint16_t modelIndex;
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
} Instance;

#endif // VOXEN_INSTANCE_H
