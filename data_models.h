#ifndef VOXEN_DATA_MODELS_H
#define VOXEN_DATA_MODELS_H

#include <GL/glew.h>

#define MODEL_COUNT 1024
#define VERTEX_ATTRIBUTES_COUNT 14 // x,y,z,nx,ny,nz,u,v,texIdx,glowIdx,specIdx,normIdx,modelIdx,instanceIdx
              // Make sure this^^^^ matches in createLightVolume_computeShader!

#define BOUNDS_ATTRIBUTES_COUNT 7
#define BOUNDS_DATA_OFFSET_MINX 0
#define BOUNDS_DATA_OFFSET_MINY 1
#define BOUNDS_DATA_OFFSET_MINZ 2
#define BOUNDS_DATA_OFFSET_MAXX 3
#define BOUNDS_DATA_OFFSET_MAXY 4
#define BOUNDS_DATA_OFFSET_MAXZ 5
#define BOUNDS_DATA_OFFSET_RADIUS 6

// Structure to represent an edge for building edge list
typedef struct {
    uint32_t v0, v1; // Vertex indices (sorted: v0 < v1)
    uint32_t tri0, tri1; // Triangle indices (tri1 = UINT32_MAX if unshared)
} Edge;

// Simple hash table for edge lookup
typedef struct {
    uint32_t v0, v1;
    uint32_t edgeIndex;
} EdgeHashEntry;

extern uint32_t modelVertexCounts[MODEL_COUNT];
extern uint32_t modelTriangleCounts[MODEL_COUNT];
extern uint32_t modelEdgeCounts[MODEL_COUNT];
extern GLuint modelVertexCountsID;
extern float modelBounds[MODEL_COUNT * BOUNDS_ATTRIBUTES_COUNT];
extern GLuint modelBoundsID;
extern GLuint vbos[MODEL_COUNT];
extern GLuint tbos[MODEL_COUNT];
extern GLuint tebos[MODEL_COUNT];
extern GLuint ebos[MODEL_COUNT];
extern GLuint vboMasterTable;
extern GLuint modelVertexOffsetsID;
extern uint32_t largestVertCount;
extern GLuint sphoxelsID;
extern float ** vertexDataArrays;
extern uint32_t ** triangleDataArrays;
extern uint32_t ** triEdgeDataArrays;
extern uint32_t ** edgeDataArrays;

int LoadGeometry(void);
void CleanupModelLoad(bool isBad);

#endif // VOXEN_DATA_MODELS_H
