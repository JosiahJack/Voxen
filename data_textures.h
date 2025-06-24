#ifndef VOXEN_DATA_TEXTURES_H
#define VOXEN_DATA_TEXTURES_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>

#define TEXTURE_COUNT 3

extern SDL_Surface *textureSurfaces[];
// extern GLuint textureIDs[];
extern GLuint colorBufferID;
extern uint32_t textureOffsets[]; // Pixel offsets for each texture (e.g. if first texture is 4x4 then 2nd texture's offset starts at 16)
extern uint32_t totalPixels; // Total pixels across all textures
extern GLuint64 textureHandles[];
extern int textureSizes[];

int LoadTextures(void);

#endif // VOXEN_DATA_TEXTURES_H
