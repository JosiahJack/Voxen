#ifndef VOXEN_DATA_MODELS_H
#define VOXEN_DATA_MODELS_H

#include <GL/glew.h>

extern float modelRadius[];
extern GLuint indirectBuffer;
extern GLuint instanceIDBuffer;
extern uint32_t modelVertexCounts[];
extern GLuint vbo_offsets[];
extern uint32_t totalVertexCount;
extern GLuint vao;
extern GLuint vbo;
extern GLuint testVBO;
extern GLuint quadVAO;
extern GLuint quadVBO;

int LoadModels(float **vertexData, uint32_t *vertexCount);

#endif // VOXEN_DATA_MODELS_H
