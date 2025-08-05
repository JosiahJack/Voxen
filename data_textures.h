#ifndef VOXEN_DATA_TEXTURES_H
#define VOXEN_DATA_TEXTURES_H

#include <GL/glew.h>
#include <stdint.h>
#include <stdbool.h>

extern GLuint colorBufferID;
extern uint16_t textureCount;

bool isDoubleSided(uint32_t texIndexToCheck);
int LoadTextures(void);
void CleanupLoad(bool isBad);

#endif // VOXEN_DATA_TEXTURES_H
