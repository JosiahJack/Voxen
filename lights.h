#ifndef LIGHTS_H
#define LIGHTS_H

#include <GL/glew.h>
                           //    0     1     2          3       4        5         6         7         8  9 10 11
#define LIGHT_DATA_SIZE 12 // posx, posy, posz, intensity, radius, spotAng, spotDirx, spotDiry, spotDirz, r, g, b
#define LIGHT_COUNT 256

extern float lights[LIGHT_COUNT * LIGHT_DATA_SIZE];
extern GLuint lightBufferID;
extern GLuint inputImageID, inputNormalsID, inputDepthID, outputImageID;

void InitializeLights(void);
void SetupGBuffer(void);

#endif // LIGHTS_H
