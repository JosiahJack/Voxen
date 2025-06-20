#ifndef VOXEN_TRANSFORM_H
#define VOXEN_TRANSFORM_H

#include <GL/glew.h>

typedef struct {
    float position[3];
    float rotation[4]; // x, y, z, w
    float scale[3];
    uint32_t modelIndex;
} InstanceData;

extern InstanceData instancesBuffer[];

void transform_vertices(float *verticesIn, float *verticesOut, unsigned int vertexCount, InstanceData *instances, uint32_t *modelVertexCounts, GLuint *vbo_offsets);

#endif // VOXEN_TRANSFORM_H
