#ifndef VOXEN_RENDER_H
#define VOXEN_RENDER_H

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <stdbool.h>

#define DEBUG_OPENGL
#ifdef DEBUG_OPENGL
#define CHECK_GL_ERROR() do { GLenum err = glGetError(); if (err != GL_NO_ERROR) DualLogError("GL Error at %s:%d: %d\n", __FILE__, __LINE__, err); } while(0)
#else
#define CHECK_GL_ERROR() do {} while(0)
#endif
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

#define VOXEL_DATA_SIZE 1

extern int screen_width;
extern int screen_height;
extern uint32_t drawCallsRenderedThisFrame;
extern uint32_t verticesRenderedThisFrame;
extern GLuint matricesBuffer;
extern bool lightDirty[MAX_VISIBLE_LIGHTS];

void CacheUniformLocationsForShaders(void);
// void render_debug_text(float x, float y, const char *text, SDL_Color color);

#endif // VOXEN_RENDER_H
