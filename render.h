#ifndef VOXEN_RENDER_H
#define VOXEN_RENDER_H

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <stdbool.h>

extern int screen_width;
extern int screen_height;
extern uint32_t drawCallCount;
extern uint32_t vertexCount;
extern float * modelMatrices;
extern GLuint matricesBuffer;
extern GLuint instancesBuffer;
extern GLuint quadVAO, quadVBO;
extern GLuint chunkShaderProgram;
extern GLuint inputImageID, inputNormalsID, inputDepthID, inputWorldPosID;
extern GLuint outputImageID, gBufferFBO, inputModelInstanceID;

// Static chunk shader locations (set on texture or model load)
extern GLint textureOffsetsLoc_chunk;
extern GLint textureSizesLoc_chunk;
extern GLint textureCountLoc_chunk;
extern GLint modelCountLoc_chunk;

// Static deferred shader locations (set on model load)
extern GLint screenWidthLoc_deferred;
extern GLint screenHeightLoc_deferred;
extern GLint textureOffsetsLoc_deferred;
extern GLint textureSizesLoc_deferred;

void SetupGBuffer(void);
void SetupInstances(void);
void SetupQuad(void);
int ClearFrameBuffers(void);
void CacheUniformLocationsForShaders(void);
void RenderMeshInstances();
int RenderStaticMeshes(void);
void render_debug_text(float x, float y, const char *text, SDL_Color color);
int RenderUI(double deltaTime);
int ClientRender();

#endif // VOXEN_RENDER_H
