#ifndef DATA_MODELS_H
#define DATA_MODELS_H

#include <GL/glew.h>

extern uint32_t modelVertexCounts[];
extern int32_t vbo_offsets[];
extern uint32_t totalVertexCount;
extern float cubeVertices[];
extern GLuint vao;
extern GLuint vbo;

void SetupCube(void);
int LoadModels(float **vertexData, uint32_t *vertexCount);
int SetupGeometry(void);

#endif // DATA_MODELS_H
