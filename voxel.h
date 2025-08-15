#ifndef VOXEN_VOXEL_H
#define VOXEN_VOXEL_H

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <stdbool.h>
#include <stdatomic.h>
#include "render.h"

#define WORLDCELL_X_MAX 64
#define WORLDCELL_Z_MAX 64
#define WORLDCELL_Y_MAX 18 // Level 8 is only 17.5 cells tall!!  Could be 16 if I make the ceiling same height in last room as in original.
#define TOTAL_WORLD_CELLS (64 * 64 * 18)
// #define VOXELS_PER_CELL (8 * 8 * 8)
// #define VOXELS_WORLD_X_MAX (WORLDCELL_X_MAX * 8)
// #define VOXELS_WORLD_Y_MAX (WORLDCELL_Y_MAX * 8)
// #define VOXELS_WORLD_Z_MAX (WORLDCELL_Z_MAX * 8)
// #define VOXEL_WIDTH_F 0.32f
#define WORLDCELL_WIDTH_F 2.56f
#define LIGHT_RANGE_VOXEL_MANHATTAN_DIST (floorf(LIGHT_RANGE_MAX / VOXEL_WIDTH_F))
// #define TOTAL_VOXELS (WORLDCELL_X_MAX * WORLDCELL_Y_MAX * WORLDCELL_Z_MAX * VOXELS_PER_CELL) // 37748736
// #define TOTAL_VOXEL_BITS (TOTAL_VOXELS / 8)
#define INVALID_LIGHT_INDEX (LIGHT_COUNT + 1)
#define MAX_LIGHTS_VISIBLE_PER_CELL 32
#define NUM_CUBEMAPS 25

#endif // VOXEN_VOXEL_H

