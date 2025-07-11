// voxel.c
// Description: Defines the voxel space invisible overlay data structure that
//              is positioned over the world at a regular interval and used by
//              both lighting and physics systems for all visibility and
//              collision checks.
//
// Voxels are in the following hierarchy:
// World cells 64x64x32
// Each cell contains 8x8x8 voxels.  World cells not currently utilized in
// VXGI, just raw voxels so 512x512x144.
//
// A voxel is 0.32f x 0.32f x 0.32f
// Each world cell is 2.56f x 2.56f x 2.56f
//
// Side note:
// In the physics engine, each cell is 2560 x 2560 x 2560 units wide (* 1000)
// and each voxel is 320 x 320 x 320 for 0.001f precision in world space
// Voxels do not contain voxel size 1x1x1 in physics due to RAM constraints.
//
// All voxels in voxel space = 64x64x18x8x8x8 = 37748736 voxels (fits in 2^26).
// Currently only implementing world cells as a start.

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <stdbool.h>
#include <stdio.h>
#include "event.h"
#include "voxel.h"
#include "debug.h"
#include "lights.h"
#include "data_models.h"
#include "render.h"

// uint32_t cellOccupancy[TOTAL_WORLD_CELLS * MAX_LIGHTS_VISIBLE_PER_CELL]; // 131072 * 4byte = 524kb
GLuint vxgiID;

void VXGI_Init(void) {
    DebugRAM("VXGI Init\n");
    glGenBuffers(1, &vxgiID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vxgiID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 32 * LIGHT_DATA_SIZE * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 19, vxgiID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);    
    DebugRAM("after VXGI Init");
}

uint32_t Flatten3DIndex(int x, int y, int z, int xMax, int yMax) {
    return (uint32_t)(x + (y * xMax) + (z * xMax * yMax));
}

void WorldCellIndexToPosition(uint32_t worldIdx, float * x, float * y, float * z) {
    *x = (worldIdx % WORLDCELL_X_MAX) * WORLDCELL_WIDTH_F + WORLDCELL_WIDTH_F / 2.0f;
    *y = ((worldIdx / WORLDCELL_X_MAX) % WORLDCELL_Y_MAX) * WORLDCELL_WIDTH_F + WORLDCELL_WIDTH_F / 2.0f;
    *z = (worldIdx / (WORLDCELL_X_MAX * WORLDCELL_Y_MAX)) * WORLDCELL_WIDTH_F + WORLDCELL_WIDTH_F / 2.0f;   
}

uint32_t PositionToWorldCellIndexX(float x) {
    float cellHalf = WORLDCELL_WIDTH_F / 2.0f;
    int xi = (int)floorf((x + cellHalf) / WORLDCELL_WIDTH_F);
    return (xi < 0 ? 0 : (xi >= WORLDCELL_X_MAX ? WORLDCELL_X_MAX - 1 : xi));
}

uint32_t PositionToWorldCellIndexY(float y) {
    float cellHalf = WORLDCELL_WIDTH_F / 2.0f;
    int yi = (int)floorf((y + cellHalf) / WORLDCELL_WIDTH_F);
    return (yi < 0 ? 0 : (yi >= WORLDCELL_Y_MAX ? WORLDCELL_Y_MAX - 1 : yi));
}

uint32_t PositionToWorldCellIndexZ(float z) {
    float cellHalf = WORLDCELL_WIDTH_F / 2.0f;
    int zi = (int)floorf((z + cellHalf) / WORLDCELL_WIDTH_F);
    return (zi < 0 ? 0 : (zi >= WORLDCELL_Z_MAX ? WORLDCELL_Z_MAX - 1 : zi));
}

uint32_t PositionToWorldCellIndex(float x, float y, float z) {
    float cellHalf = WORLDCELL_WIDTH_F / 2.0f;
    int xi = (int)floorf((x + cellHalf) / WORLDCELL_WIDTH_F);
    int yi = (int)floorf((y + cellHalf) / WORLDCELL_WIDTH_F);
    int zi = (int)floorf((z + cellHalf) / WORLDCELL_WIDTH_F);
    xi = xi < 0 ? 0 : (xi >= WORLDCELL_X_MAX ? WORLDCELL_X_MAX - 1 : xi);
    yi = yi < 0 ? 0 : (yi >= WORLDCELL_Y_MAX ? WORLDCELL_Y_MAX - 1 : yi);
    zi = zi < 0 ? 0 : (zi >= WORLDCELL_Z_MAX ? WORLDCELL_Z_MAX - 1 : zi);
    return Flatten3DIndex(xi, yi, zi, WORLDCELL_X_MAX, WORLDCELL_Y_MAX);
}

float squareDistance3D(float x1, float y1, float z1, float x2, float y2, float z2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float dz = z2 - z1;
    return dx * dx + dy * dy + dz * dz;
}
