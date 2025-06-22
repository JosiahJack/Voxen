#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "lights.h"

#define M_PI 3.141592653f
#define M_PI_2 1.57079632679489661923f
#define TEXTURE_COUNT 3
#define MODEL_COUNT 3
#define LIGHT_COUNT 16

extern LightData lights[LIGHT_COUNT];
extern GLuint lightBufferID;

#endif // CONSTANTS_H
