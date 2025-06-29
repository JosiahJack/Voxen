#ifndef VOXEN_DATA_TEXTURES_H
#define VOXEN_DATA_TEXTURES_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>

extern SDL_Surface ** textureSurfaces;
extern GLuint colorBufferID;
extern uint32_t * textureOffsets; // Pixel offsets for each texture (e.g. if first texture is 4x4 then 2nd texture's offset starts at 16)
extern int * textureSizes;
extern int32_t textureCount;

int LoadTextures(void);

#endif // VOXEN_DATA_TEXTURES_H
