#ifndef VOXEN_RENDER_H
#define VOXEN_RENDER_H

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <stdbool.h>

#define DEBUG_OPENGL
#ifdef DEBUG_OPENGL
#define CHECK_GL_ERROR() do { GLenum err = glGetError(); if (err != GL_NO_ERROR) DualLogError("GL Error at %s:%d: %d\n", __FILE__, __LINE__, err); } while(0)
#define CHECK_GL_ERROR_HERE(msg) \
    do { \
        GLenum err = glGetError(); \
        if (err != GL_NO_ERROR) \
            DualLogError("GL Error at %s:%d (%s): %d\n", __FILE__, __LINE__, msg, err); \
    } while(0)
#else
#define CHECK_GL_ERROR() do {} while(0)
#define CHECK_GL_ERROR_HERE() do {} while(0)
#endif
                           //    0     1     2          3       4        5         6         7         8         9 10 11 12
#define LIGHT_DATA_SIZE 13 // posx, posy, posz, intensity, radius, spotAng, spotDirx, spotDiry, spotDirz, spotDirw, r, g, b
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
#define LIGHT_DATA_OFFSET_SPOTDIRW 9
#define LIGHT_DATA_OFFSET_R 10
#define LIGHT_DATA_OFFSET_G 11
#define LIGHT_DATA_OFFSET_B 12
// Make sure these match in deferredLighting_computeShader!
                   
#define LIGHT_COUNT 1600 // MAX CITADEL LIGHT COUNT is 1561 for Level 7, leaves room for dynamic lights from projectiles
#define LIGHT_MAX_INTENSITY 8.0f
#define LIGHT_RANGE_MAX 15.36f
#define LIGHT_RANGE_MAX_SQUARED (LIGHT_RANGE_MAX * LIGHT_RANGE_MAX)
#define MAX_VISIBLE_LIGHTS 90

#define WORLDX 64
#define WORLDZ WORLDX
#define WORLDY 18 // Level 8 is only 17.5 cells tall!!  Could be 16 if I make the ceiling same height in last room as in original.
#define TOTAL_WORLD_CELLS (WORLDX * WORLDY * WORLDZ)
#define ARRSIZE (WORLDX * WORLDZ)
#define WORLDCELL_WIDTH_F 2.56f
#define CELLXHALF (WORLDCELL_WIDTH_F * 0.5f)
#define LIGHT_RANGE_VOXEL_MANHATTAN_DIST (floorf(LIGHT_RANGE_MAX / VOXEL_WIDTH_F))
#define INVALID_LIGHT_INDEX (LIGHT_COUNT + 1)
#define INVALID_FLOOR_HEIGHT -1300.0f
#define LUXEL_SIZE 0.16f

extern float lights[LIGHT_COUNT * LIGHT_DATA_SIZE];
extern float lightsRangeSquared[LIGHT_COUNT];

extern uint8_t numLevels;
extern uint8_t currentLevel;
extern bool gamePaused;
extern bool menuActive;
extern int screen_width;
extern int screen_height;
extern uint32_t drawCallsRenderedThisFrame;
extern uint32_t verticesRenderedThisFrame;
extern GLuint matricesBuffer;
extern GLuint precomputedVisibleCellsFromHereID;
extern GLuint cellIndexForInstanceID;
extern GLuint cellIndexForLightID;
extern GLuint lightmapID;
extern GLuint masterIndexForLightsInPVSID;
extern bool lightDirty[MAX_VISIBLE_LIGHTS];

typedef struct {
    float x, y, z, w;
} Quaternion;

void CacheUniformLocationsForShaders(void);
void Screenshot(void);

extern bool global_modIsCitadel;

#endif // VOXEN_RENDER_H
