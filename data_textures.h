#ifndef VOXEN_DATA_TEXTURES_H
#define VOXEN_DATA_TEXTURES_H

#include <GL/glew.h>

extern GLuint colorBufferID;
extern int32_t textureCount;

bool isDoubleSided(uint32_t texIndexToCheck);
int LoadTextures(void);
void CleanupLoad(bool isBad);

#endif // VOXEN_DATA_TEXTURES_H
