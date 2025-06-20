#ifndef RENDER_H
#define RENDER_H

extern int screen_width;
extern int screen_height;

extern TTF_Font* font;
extern GLuint textVAO, textVBO;
extern uint32_t drawCallCount;
extern uint32_t vertexCount;

void SetupTextQuad(void);
int ClearFrameBuffers(void);
void CacheUniformLocationsForChunkShader(void);
int RenderStaticMeshes(void);
void render_debug_text(float x, float y, const char *text, SDL_Color color);
int RenderUI(double deltaTime);

#endif // RENDER_H
