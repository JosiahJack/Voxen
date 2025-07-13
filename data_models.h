#ifndef VOXEN_DATA_MODELS_H
#define VOXEN_DATA_MODELS_H

#include <GL/glew.h>

#define MODEL_COUNT 1024
#define VERTEX_ATTRIBUTES_COUNT 8
#define BOUNDS_ATTRIBUTES_COUNT 7
#define BOUNDS_DATA_OFFSET_MINX 0
#define BOUNDS_DATA_OFFSET_MINY 1
#define BOUNDS_DATA_OFFSET_MINZ 2
#define BOUNDS_DATA_OFFSET_MAXX 3
#define BOUNDS_DATA_OFFSET_MAXY 4
#define BOUNDS_DATA_OFFSET_MAXZ 5
#define BOUNDS_DATA_OFFSET_RADIUS 6

extern uint32_t modelVertexCounts[MODEL_COUNT];
extern float modelBounds[MODEL_COUNT * BOUNDS_ATTRIBUTES_COUNT];
extern GLuint modelBoundsID;
extern uint32_t totalVertexCount;
extern GLuint vao;
extern GLuint vbos[MODEL_COUNT];
extern float * vertexDataArrays[MODEL_COUNT];

int LoadGeometry(void);
void CleanupModelLoadOnFail(void);

#endif // VOXEN_DATA_MODELS_H
