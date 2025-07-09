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
extern GLuint quadVAO, quadVBO;
extern GLuint chunkShaderProgram;
extern GLuint inputImageID, inputNormalsID, inputDepthID, inputWorldPosID;
// extern GLuint outputImageID;
extern GLuint gBufferFBO;
extern GLuint inputTexMapsID;

// Static chunk shader locations (set on texture or model load)
extern GLint textureCountLoc_chunk;

// Static deferred shader locations (set on model load)
extern GLint screenWidthLoc_deferred;
extern GLint screenHeightLoc_deferred;
extern GLint vxgiEnabledLoc_deferred;
extern GLint voxelCountLoc_deferred;
extern GLint shadowsEnabledLoc_deferred;
extern bool shadowsEnabled;

void SetupGBuffer(void);
void SetupQuad(void);
int ClearFrameBuffers(void);
void CacheUniformLocationsForShaders(void);
void RenderMeshInstances();
int RenderStaticMeshes(void);
void render_debug_text(float x, float y, const char *text, SDL_Color color);
int RenderUI(double deltaTime);
int ClientRender();

#endif // VOXEN_RENDER_H
