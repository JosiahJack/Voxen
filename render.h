#ifndef RENDER_H
#define RENDER_H

#include <GL/glew.h>
#include <stdbool.h>

extern int screen_width;
extern int screen_height;
extern TTF_Font* font;
extern GLuint textVAO, textVBO;
extern uint32_t drawCallCount;
extern uint32_t vertexCount;
extern GLuint quadVAO, quadVBO;
extern GLuint chunkShaderProgram;
extern GLuint inputImageID, inputNormalsID, inputDepthID, inputWorldPosID, outputImageID, gBufferFBO;
extern GLuint textShaderProgram;
extern GLuint deferredLightingShaderProgram;
extern GLuint imageBlitShaderProgram;

int CompileShaders(void);
void SetupQuad(void);
void SetupTextQuad(void);
int ClearFrameBuffers(void);
void CacheUniformLocationsForChunkShader(void);
int RenderStaticMeshes(void);
void render_debug_text(float x, float y, const char *text, SDL_Color color);
int RenderUI(double deltaTime);
int ClientRender();

#endif // RENDER_H
