#ifndef VOXEN_LIGHTS_H
#define VOXEN_LIGHTS_H

#include <GL/glew.h>
#include <stdbool.h>
#include "data_models.h"
                           //    0     1     2          3       4        5         6         7         8  9 10 11
#define LIGHT_DATA_SIZE 12 // posx, posy, posz, intensity, radius, spotAng, spotDirx, spotDiry, spotDirz, r, g, b
      // Make sure this^^^^ matches in deferredLighting_computeShader!

#define LIGHT_DATA_OFFSET_POSX 0
#define LIGHT_DATA_OFFSET_POSY 1
#define LIGHT_DATA_OFFSET_POSZ 2
#define LIGHT_DATA_OFFSET_INTENSITY 3
#define LIGHT_DATA_OFFSET_RANGE 4
#define LIGHT_DATA_OFFSET_SPOTANG 5
#define LIGHT_DATA_OFFSET_SPOTDIRX 6
#define LIGHT_DATA_OFFSET_SPOTDIRY 7
#define LIGHT_DATA_OFFSET_SPOTDIRZ 8
#define LIGHT_DATA_OFFSET_R 9
#define LIGHT_DATA_OFFSET_G 10
#define LIGHT_DATA_OFFSET_B 11
                   
#define LIGHT_COUNT 1600 // MAX CITADEL LIGHT COUNT is 1561 for Level 7
#define LIGHT_MAX_INTENSITY 8.0f
#define LIGHT_RANGE_MAX 15.36f
#define LIGHT_RANGE_MAX_SQUARED (LIGHT_RANGE_MAX * LIGHT_RANGE_MAX)
#define MAX_VISIBLE_LIGHTS 32
#define MAX_VERTS_PER_LIGHT_VOLUME 65535

#define VOXEL_DATA_SIZE 1
#define SHADOW_MAP_SIZE 512
#define SHADOW_MAP_FARPLANE 20.0

#endif // VOXEN_LIGHTS_H
