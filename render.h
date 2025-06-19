#ifndef VOXEN_RENDER_H
#define VOXEN_RENDER_H

#include <GL/glew.h>
#include <SDL2/SDL_ttf.h>

extern int screen_width;
extern int screen_height;

typedef struct {
    GLuint count;
    GLuint instanceCount;
    GLuint first;
    GLuint baseInstance;
} DrawArraysIndirectCommand;

extern TTF_Font* font;
extern GLuint textVAO, textVBO;
extern GLuint instanceSSBO;
extern GLuint modelBoundsBuffer;
extern GLuint drawCountBuffer;

void SetupTextQuad(void);
int SetupGeometry(void);
int ClearFrameBuffers(void);
int RenderStaticMeshes(void);
void render_debug_text(float x, float y, const char *text, SDL_Color color);
int RenderUI(double deltaTime);

#endif // VOXEN_RENDER_H
