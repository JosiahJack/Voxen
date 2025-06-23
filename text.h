#ifndef VOXEN_TEXT_H
#define VOXEN_TEXT_H

#include <SDL2/SDL_ttf.h>
#include <GL/glew.h>

extern GLuint textShaderProgram;
extern TTF_Font* font;
extern GLuint textVAO, textVBO;
extern GLuint fontAtlasTexture;
extern int fontAtlasWidth;
extern int fontAtlasHeight;

int InitializeTextAndFonts(void);
void SetupTextQuad(void);
void SetupFontAtlas(void);

#endif // VOXEN_TEXT_H
