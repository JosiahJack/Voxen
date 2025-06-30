#ifndef VOXEN_DATA_MODELS_H
#define VOXEN_DATA_MODELS_H

#include <GL/glew.h>

#define MODEL_COUNT 7 // Also need to set in deferred_lighting.compute!!
#define VERTEX_ATTRIBUTES_COUNT 8
#define BOUNDS_ATTRIBUTES_COUNT 6

extern uint32_t modelVertexCounts[];
extern GLint modelTriangleCounts[];
extern float modelBounds[];
extern GLuint modelBoundsID;
extern int32_t vbo_offsets[];
extern uint32_t totalVertexCount;
extern GLuint vao;
extern GLuint vbos[];

int LoadModels(float *vertexDataArrays[MODEL_COUNT], uint32_t vertexCounts[MODEL_COUNT]);
int LoadGeometry(void);

#endif // VOXEN_DATA_MODELS_H
