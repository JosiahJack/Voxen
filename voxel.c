// voxel.c
// Description: Defines the voxel space invisible overlay data structure that
//              is positioned over the world at a regular interval and used by
//              both lighting and physics systems for all visibility and
//              collision checks.
//
// Voxels are in the following hierarchy:
// World cells 64x64x32
// Each cell contains 8x8x8 voxels.  World cells not currently utilized in
// VXGI, just raw voxels so 1024x1024x512.
//
// A voxel is 0.32f x 0.32f x 0.32f
// Each world cell is 2.56f x 2.56f x 2.56f
//
// Side note:
// In the physics engine, each cell is 2560 x 2560 x 2560 units wide (* 1000)
// and each voxel is 320 x 320 x 320 for 0.001f precision in world space
// Voxels do not contain voxel size 1x1x1 in physics due to RAM constraints.
//
// All voxels in voxel space = 64x64x32x8x8x8 = 67108864 voxels total (aka 2^26).

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <stdio.h>
#include "voxel.h"
#include "debug.h"
#include "lights.h"
#include "data_models.h"
#include "render.h"

void VXGI_Init(void) {
    DebugRAM("VXGI Init\n");
//     for (int i = 0; i < 2; ++i) {
//         SVO_Init(&vxgi_buffers[i].svo,i);
//         atomic_store(&vxgi_buffers[i].ready, false);
//     }
    memset(voxels,INVALID_LIGHTSET,TOTAL_VOXELS * sizeof(uint32_t));
    memset(cellOccupancy,0,TOTAL_WORLD_CELLS * sizeof(bool));
    memset(lightSubsetBitmasks,0,TOTAL_WORLD_CELLS * sizeof(uint64_t));
    
    DualLog("Spawning VXGI worker thread... ");
    VXGIWorkerData* workerData = malloc(sizeof(VXGIWorkerData));
    workerData->lights = lights;
    workerData->lightCount = LIGHT_COUNT;
    vxgi_thread = SDL_CreateThread(VXGI_Worker, "VXGIThread", workerData);
    
    glGenBuffers(1, &vxgiID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vxgiID);
    
    glBufferData(GL_SHADER_STORAGE_BUFFER, 64 * sizeof(uint32_t), NULL, GL_DYNAMIC_DRAW); // Light indices of current subset, 64 max.
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 19, vxgiID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glUniform1i(vxgiEnabledLoc_deferred, 0); // Mark as unusable until first vxgi thread iteration completes.
    glUniform1i(voxelCountLoc_deferred, 0);
    DualLog("DONE\n");
    DebugRAM("after initializing all voxels");
}

static inline uint32_t Flatten3DIndex(int x, int y, int z, int xMax, int yMax, int zMax) {
    int x_old = x;
    int y_old = y;
    int z_old = z;
    int xi = x < 0 ? 0 : (x >= xMax ? xMax - 1 : x);
    int yi = y < 0 ? 0 : (y >= yMax ? yMax - 1 : y);
    int zi = z < 0 ? 0 : (z >= zMax ? zMax - 1 : z);
    if (xi != x_old) DualLog("WARNING: Clamped x index from %d to %d from given position x: %f, y: %f, z: %f\n",x_old,xi,x,y,z);
    if (yi != y_old) DualLog("WARNING: Clamped y index from %d to %d from given position x: %f, y: %f, z: %f\n",y_old,yi,x,y,z);
    if (zi != z_old) DualLog("WARNING: Clamped z index from %d to %d from given position x: %f, y: %f, z: %f\n",z_old,zi,x,y,z);
    return (uint32_t)(x + (y * xMax) + (z * xMax * yMax));
}

uint32_t PositionToVoxelIndex(float x, float y, float z) {
    int xi = (int)floorf(x / VOXEL_WIDTH_F);
    int yi = (int)floorf(y / VOXEL_WIDTH_F);
    int zi = (int)floorf(z / VOXEL_WIDTH_F);
    return Flatten3DIndex(xi, yi, zi, VOXELS_WORLD_X_MAX, VOXELS_WORLD_Y_MAX,
                          VOXELS_WORLD_Z_MAX);
}

uint32_t VoxelIndexToPosition(uint32_t voxelIdx, float * x, float * y, float * z) {
    int zIdx = voxelIdx / (VOXELS_WORLD_X_MAX * VOXELS_WORLD_Y_MAX);
    int yzRemainder = voxelIdx % (VOXELS_WORLD_X_MAX * VOXELS_WORLD_Y_MAX);
    int yIdx = yzRemainder / VOXELS_WORLD_X_MAX;
    int xIdx = yzRemainder % VOXELS_WORLD_X_MAX;

    *x = xIdx * VOXEL_WIDTH_F + (VOXEL_WIDTH_F / 2.0f);
    *y = yIdx * VOXEL_WIDTH_F + (VOXEL_WIDTH_F / 2.0f);
    *z = zIdx * VOXEL_WIDTH_F + (VOXEL_WIDTH_F / 2.0f);
}

uint32_t PositionToWorldCellIndex(float x, float y, float z) {
    int xi = (int)floorf(x / WORLDCELL_WIDTH_F);
    int yi = (int)floorf(y / WORLDCELL_WIDTH_F);
    int zi = (int)floorf(z / WORLDCELL_WIDTH_F);
    return Flatten3DIndex(xi, yi, zi, WORLDCELL_X_MAX, WORLDCELL_Y_MAX, WORLDCELL_Z_MAX);
}

uint64_t GetLightSubsetFromPosition(float x, float y, float z) {
    uint32_t cellIdx = PositionToWorldCellIndex(x,y,z);
    uint32_t voxelIdx = PositionToVoxelIndex(x,y,z);
    if (!cellOccupancy[cellIdx]) return INVALID_LIGHTSET;
    if (voxels[voxelIdx] >= INVALID_LIGHTSET) return INVALID_LIGHTSET;
    
    return lightSubsetBitmasks[voxels[voxelIdx]];
}

void InsertOccupiedVoxel(float x, float y, float z) {
    uint32_t voxelIdx = PositionToVoxelIndex(x, y, z);
    uint32_t byteIdx = voxelIdx / 8;
    uint8_t bitIdx = voxelIdx % 8;
    voxelOccupancy[byteIdx] |= (1 << bitIdx);
    uint32_t cellIdx = PositionToWorldCellIndex(x, y, z);
    cellOccupancy[cellIdx] = true;
}

void InsertLightToVoxels(uint32_t lightIdx, uint32_t bitIndex) {
    float x,y,z;
    GetLightPos(lightIdx,x,y,z);
    uint32_t cellIdx = PositionToWorldCellIndex(x,y,z);
    if (!cellOccupancy[cellIdx]) InsertOccupiedVoxel(x,y,z);
    uint64_t lightSubset = GetLightSubsetFromPosition(x,y,z);
    lightSubset |= (1ULL << bitIndex);
}

float squareDistance3D(float x1, float y1, float z1, float x2, float y2, float z2) {
    float dx = (x2 - x1);
    float dy = (y2 - y1);
    float dz = (z2 - z1);
    return (dx * dx) + (dy * dy) + (dz * dz);
}

// Currently iterates over all world cells, updating each cell's lightSubset
// bitmask enabling up to 64 lights to be visible from that cell.
// This is it so far.  Just hinting to the deferred renderer to limit number of
// lights processed per pixel by indexing into the shader's buffer for light
// subsets using the pixel's worldPos and then checking against the current
// lights buffer of 64 lights based on player's current cell index.
int VXGI_Worker(__attribute__((unused)) void* data) {
    VXGIWorkerData* workerData = (VXGIWorkerData*)data;
    int write_index = 0;
    while (vxgi_running) {
        VXGIBuffer* buffer = &vxgi_buffers[write_index];
        if (!atomic_load(&buffer->ready)) {
            SDL_Delay(1);  // Avoid busy-wait
            continue;
        }

        uint32_t lightSubset;
        uint32_t litIdx;
        
        // Process voxels with lights
        for (uint32_t worldCellIdx = 0; worldCellIdx < TOTAL_WORLD_CELLS; ++worldCellIdx) {
            lightSubset = lightSubsetBitmasks[worldCellIdx];
            for (uint32_t voxelIdx = 0; voxelIdx < TOTAL_VOXELS; ++voxelIdx) { // Nested, terrible but this is in a separate thread, let 'im cook.
                float vox_x, vox_y, vox_z;
                VoxelIndexToPosition(voxelIdx,&vox_x,&vox_y,&vox_z);
                
                float xmin = vox_x - LIGHT_RANGE_MAX;
                float ymin = vox_y - LIGHT_RANGE_MAX;
                float zmin = vox_z - LIGHT_RANGE_MAX;
                float xmax = vox_x + LIGHT_RANGE_MAX;
                float ymax = vox_y + LIGHT_RANGE_MAX;
                float zmax = vox_z + LIGHT_RANGE_MAX;
                for (uint32_t lightIdx = 0; lightIdx < (workerData->lightCount * LIGHT_DATA_SIZE); lightIdx += LIGHT_DATA_SIZE) {
                    float lit_x,lit_y,lit_z;
                    GetLightPos(lightIdx,lit_x,lit_y,lit_z,&workerData->lights);
                    if (squareDistance3D(vox_x,vox_y,vox_z,lit_x,lit_y,lit_z) < LIGHT_RANGE_MAX_SQUARED) {
                        lightSubset |= true << litIdx; // Turn on the nth bit for this light for this world cell's lightSubset.
                        litIdx++;
                        if (litIdx >= 64) goto subsetComplete;// Don't hate me
                    }
                }
            }
            
            subsetComplete:
            break;
        }

        atomic_store(&buffer->ready, true);
        write_index = 1 - write_index;  // Swap buffers
    }
    free(data);  // Clean up workerData
    return 0;
}

void VXGI_Shutdown(void) {
    vxgi_running = false;
    SDL_WaitThread(vxgi_thread, NULL);

    for (int i = 0; i < 2; ++i) {
        free(vxgi_buffers[i].svo.nodes);
        free(vxgi_buffers[i].svo.voxels);
    }
}
