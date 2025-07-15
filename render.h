#ifndef VOXEN_RENDER_H
#define VOXEN_RENDER_H

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <stdbool.h>

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
extern GLint textureCountLoc_chunk;

void CacheUniformLocationsForShaders(void);
// void render_debug_text(float x, float y, const char *text, SDL_Color color);

#endif // VOXEN_RENDER_H
