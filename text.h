#ifndef VOXEN_TEXT_H
#define VOXEN_TEXT_H

#include <SDL2/SDL_ttf.h>
#include <GL/glew.h>

extern GLuint textShaderProgram;
extern TTF_Font* font;
extern GLuint textVAO, textVBO;

#define TEXT_WHITE 0
#define TEXT_YELLOW 1
#define TEXT_DARK_YELLOW 2
#define TEXT_GREEN 3
#define TEXT_RED 4
#define TEXT_ORANGE 5

#define TEXT_BUFFER_SIZE 128

int InitializeTextAndFonts(void);
void SetupTextQuad(void);
void RenderText(float x, float y, const char *text, int colorIdx);
void RenderFormattedText(int x, int y, uint32_t color, const char* format, ...);

#endif // VOXEN_TEXT_H
