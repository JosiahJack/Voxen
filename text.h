#ifndef VOXEN_TEXT_H
#define VOXEN_TEXT_H

#include <SDL2/SDL_ttf.h>
#include <GL/glew.h>

extern GLuint textShaderProgram;
extern TTF_Font* font;
extern GLuint textVAO, textVBO;

int InitializeTextAndFonts(void);
void SetupTextQuad(void);

#endif // VOXEN_TEXT_H
