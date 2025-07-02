#ifndef VOXEN_DATA_MODELS_H
#define VOXEN_DATA_MODELS_H

#include <GL/glew.h>

#define VERTEX_ATTRIBUTES_COUNT 8
#define BOUNDS_ATTRIBUTES_COUNT 6

extern uint32_t modelCount;
extern uint32_t * modelVertexCounts;
extern float * modelBounds;
extern GLuint modelBoundsID;
extern uint32_t totalVertexCount;
extern GLuint vao;
extern GLuint * vbos;

int LoadGeometry(void);
void CleanupModelLoadOnFail(void);

#endif // VOXEN_DATA_MODELS_H
