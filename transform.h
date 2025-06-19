#ifndef VOXEN_TRANSFORM_H
#define VOXEN_TRANSFORM_H

typedef struct {
    float position[3]; // vec3
    float rotation[4]; // vec4 quaternion (x, y, z, w)
    float scale[3];    // vec3
} Transform;

typedef struct {
    float position[3];
    float rotation[4]; // x, y, z, w
    float scale[3];
    uint32_t modelIndex;
} InstanceData;

extern InstanceData instancesBuffer[];

void transform_vertices(float *verticesIn, float *verticesOut, unsigned int vertexCount,
                       InstanceData *instances, unsigned int instanceCount,
                       unsigned int *modelVertexCounts, unsigned int *vbo_offsets,
                       float cameraPos[3], float cameraYaw, float cameraPitch,
                       float fovV, float fovH,
                       unsigned int screenWidth, unsigned int screenHeight);

#endif // VOXEN_TRANSFORM_H
