#ifndef VOXEN_VOXEL_H
#define VOXEN_VOXEL_H

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <stdbool.h>
#include <stdatomic.h>
#include "lights.h"

#define SVO_MAX_DEPTH 6  // Adjust for 1024x1024x512 resolution
#define SVO_NODE_CHILDREN 8
#define TOTAL_VOXELS (64 * 64 * 32 * 8 * 8 * 8) // 67108864
#define SVO_INITIAL_NODE_CAPACITY 20000
#define SVO_INITIAL_VOXEL_CAPACITY 80000

typedef struct {
    float* lights;
    uint32_t lightCount;
} VXGIWorkerData;

typedef struct {
    uint8_t childrenMask;  // 8-bit mask: 1 bit per child (0 = empty, 1 = has child/voxel)
    uint32_t children[SVO_NODE_CHILDREN];  // Indices to children or voxel data
} SVONode;

typedef struct {
    float direct_light[3];  // RGB direct light result
    float indirect_light[3];  // RGB bounce light accumulation
    float occupancy;      // optional, for occlusion
} VXGIVoxel;

typedef struct {
    SVONode* nodes;  // Dynamic array of nodes
    uint32_t nodeCount;
    uint32_t nodeCapacity;
    VXGIVoxel* voxels;  // Dynamic array of voxel data
    uint32_t voxelCount;
    uint32_t voxelCapacity;
} SVO;

typedef struct {
    SVO svo;
    _Atomic bool ready;
} VXGIBuffer;

extern VXGIBuffer vxgi_buffers[2];
extern int current_vxgi_upload_index;
extern SDL_Thread* vxgi_thread;
extern _Atomic bool vxgi_running;
extern GLuint vxgiID;
extern bool vxgiEnabled;

void SVO_Init(SVO* svo, int index);
void VXGI_Init(void);
void VXGI_UpdateGL(void);
void WorldToVoxel(float x, float y, float z, int* vx, int* vy, int* vz);
void SVO_InsertVoxel(SVO* svo, int vx, int vy, int vz, VXGIVoxel voxel);
void InsertModelInstanceVoxels(int modelIdx, int texIdx, float posX, float posY, float posZ);
int VXGI_Worker(__attribute__((unused)) void * imathread);
void VXGI_Shutdown(void);

#endif // VOXEN_VOXEL_H

