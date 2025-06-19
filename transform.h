#ifndef VOXEN_TRANSFORM_H
#define VOXEN_TRANSFORM_H

typedef struct {
    float position[3]; // vec3
    float rotation[4]; // vec4 quaternion (x, y, z, w)
    float scale[3];    // vec3
} Transform;

extern Transform transformsBuffer[];

#endif // VOXEN_TRANSFORM_H
