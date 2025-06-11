#ifndef DATA_TEXTURES_H
#define DATA_TEXTURES_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>

extern SDL_Surface *textureSurfaces[];
extern GLuint textureIDs[];
extern GLuint64 textureHandles[];
extern bool use_bindless_textures;

int LoadTextures(void);

#endif // DATA_TEXTURES_H
