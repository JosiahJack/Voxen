#ifndef VOXEN_DATA_MODELS_H
#define VOXEN_DATA_MODELS_H

#include <GL/glew.h>

#define MODEL_COUNT 1024
#define VERTEX_ATTRIBUTES_COUNT 14 // x,y,z,nx,ny,nz,u,v,texIdx,glowIdx,specIdx,normIdx,modelIdx,instanceIdx
              // Make sure this^^^^ matches in createLightVolume_computeShader!

extern uint32_t modelVertexCounts[MODEL_COUNT];
extern uint32_t modelTriangleCounts[MODEL_COUNT];
extern GLuint modelBoundsID;
extern GLuint vbos[MODEL_COUNT];
extern GLuint tbos[MODEL_COUNT];
extern GLuint tebos[MODEL_COUNT];
extern GLuint ebos[MODEL_COUNT];
// extern GLuint vboMasterTable;
// extern GLuint modelVertexOffsetsID;
// extern GLuint modelVertexCountsID;
extern float ** vertexDataArrays;
extern uint32_t ** triangleDataArrays;
extern uint32_t ** triEdgeDataArrays;
extern uint32_t ** edgeDataArrays;

int LoadGeometry(void);

#endif // VOXEN_DATA_MODELS_H
