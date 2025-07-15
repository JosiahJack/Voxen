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

#define MAX_UNIQUE_VALUE 20000

extern int screen_width;
extern int screen_height;
extern uint32_t drawCallCount;
extern uint32_t vertexCount;
extern GLuint matricesBuffer;
extern GLuint chunkShaderProgram;
extern GLuint lightVolumeShaderProgram;
extern GLuint imageBlitShaderProgram;
extern GLuint deferredLightingShaderProgram;
extern GLuint lightVolumeMeshShaderProgram;

void CacheUniformLocationsForShaders(void);
// void render_debug_text(float x, float y, const char *text, SDL_Color color);

#endif // VOXEN_RENDER_H
