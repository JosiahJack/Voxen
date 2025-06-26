// voxel.c
// Description: Defines the voxel space invisible overlay data structure that
//              is positioned over the world at a regular interval and used by
//              both lighting and physics systems for all visibility and
//              collision checks.
//
// Voxels are in the following hierarchy:
// World cells 64x64x32
// Each cell contains 128x128x128 voxels
//
// A voxel is 0.02f x 0.02f x 0.02f
// Each world cell is 2.56f x 2.56f x 2.56f
// In the physics engine, each cell is 2560 x 2560 x 2560
// and each voxel is 20 x 20 x 20 for 0.001f precision in world space
// Voxels need not contain voxels of size 1x1x1 in physics space due to RAM constraints
// All voxels in voxel space = 64x64x32x128x128x128 = 274877906944 (aka 2^38, 1D indexing could use 38bit indexing).

#include <GL/glew.h>
#include <stdint.h>

typedef struct {
    uint8_t worldCellx; // 2^6 = 64
    uint8_t worldCelly; // 2^6 = 64
    uint8_t worldCellz; // 2^5 = 32
    uint32_t cellVoxelIndex; // 2^21 = 2097152 = 128x128x128
} Voxel;

