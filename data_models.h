#ifndef DATA_MODELS_H
#define DATA_MODELS_H

#include <GL/glew.h>

#define VERTEX_ATTRIBUTES_COUNT 8

extern uint32_t modelVertexCounts[];
extern int32_t vbo_offsets[];
extern uint32_t totalVertexCount;
extern GLuint vao;
extern GLuint vbos[];

int LoadModels(float *vertexDataArrays[MODEL_COUNT], uint32_t vertexCounts[MODEL_COUNT]);
int SetupGeometry(void);

#endif // DATA_MODELS_H
