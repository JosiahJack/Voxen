#ifndef VOXEN_VOXEL_H
#define VOXEN_VOXEL_H

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <stdbool.h>
#include <stdatomic.h>
#include "lights.h"

#define WORLDCELL_X_MAX 64
#define WORLDCELL_Y_MAX 64
#define WORLDCELL_Z_MAX 18 // Level 8 is only 17.5 cells tall!!  Could be 16 if I make the ceiling same height in last room as in original.
#define TOTAL_WORLD_CELLS (64 * 64 * 18)
#define VOXELS_PER_CELL (8 * 8 * 8)
#define VOXELS_WORLD_X_MAX (WORLDCELL_X_MAX * 8)
#define VOXELS_WORLD_Y_MAX (WORLDCELL_Y_MAX * 8)
#define VOXELS_WORLD_Z_MAX (WORLDCELL_Z_MAX * 8)
#define VOXEL_WIDTH_F 0.32f
#define WORLDCELL_WIDTH_F 2.56f
#define LIGHT_RANGE_VOXEL_MANHATTAN_DIST (floorf(LIGHT_RANGE_MAX / VOXEL_WIDTH_F))

#define TOTAL_VOXELS (WORLDCELL_X_MAX * WORLDCELL_Y_MAX * WORLDCELL_Z_MAX * VOXELS_PER_CELL) // 37748736
#define TOTAL_VOXEL_BITS (TOTAL_VOXELS / 8)
#define INVALID_LIGHTSET (uint64_t)(TOTAL_VOXELS) + 1ULL
#define MAX_LIGHTS_VISIBLE_PER_CELL 64
uint64_t lightSubsetBitmasks[TOTAL_WORLD_CELLS]; // One per world cell xy, 4096 * 32 * 64bits = 1MB
bool cellOccupancy[TOTAL_WORLD_CELLS]; // 131072 * 4byte = 524kb
uint8_t voxels[TOTAL_VOXELS]; // Stores bitfields

typedef struct {
    float* lights;
    uint32_t lightCount;
} VXGIWorkerData;

extern VXGIBuffer vxgi_buffers[2];
extern int current_vxgi_upload_index;
extern SDL_Thread* vxgi_thread;
extern _Atomic bool vxgi_running;
extern GLuint vxgiID;
extern bool vxgiEnabled;

void VXGI_Init(void);
uint32_t PositionToVoxelIndex(float x, float y, float z);
uint32_t PositionToWorldCellIndex(float x, float y, float z);
uint64_t GetLightSubsetFromPosition(float x, float y, float z);
void InsertOccupiedVoxel(float x, float y, float z);
void InsertLightToVoxels(uint32_t lightIdx);
int VXGI_Worker(__attribute__((unused)) void * imathread);
void VXGI_Shutdown(void);

#endif // VOXEN_VOXEL_H

