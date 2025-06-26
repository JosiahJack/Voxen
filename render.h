#ifndef VOXEN_RENDER_H
#define VOXEN_RENDER_H

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <stdbool.h>

extern int screen_width;
extern int screen_height;
extern uint32_t drawCallCount;
extern uint32_t vertexCount;
extern GLuint quadVAO, quadVBO;
extern GLuint chunkShaderProgram;
extern GLuint inputImageID, inputNormalsID, inputDepthID, inputWorldPosID, outputImageID, gBufferFBO;

void SetupGBuffer(void);
void SetupInstances(void);
void SetupQuad(void);
int ClearFrameBuffers(void);
void CacheUniformLocationsForChunkShader(void);
void RenderMeshInstances(GLint model_mat_loc, GLint texindex_loc, GLint modelindex_loc);
int RenderStaticMeshes(void);
void render_debug_text(float x, float y, const char *text, SDL_Color color);
int RenderUI(double deltaTime);
int ClientRender();

#endif // VOXEN_RENDER_H
