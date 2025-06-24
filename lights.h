#ifndef VOXEN_LIGHTS_H
#define VOXEN_LIGHTS_H

#include <GL/glew.h>
                           //    0     1     2          3       4        5         6         7         8  9 10 11
#define LIGHT_DATA_SIZE 12 // posx, posy, posz, intensity, radius, spotAng, spotDirx, spotDiry, spotDirz, r, g, b
      // Make sure this^^^^ matches in deferredLighting_computeShader!
                   
#define LIGHT_COUNT 256 // MAX CITADEL LIGHT COUNT is 1561 for Level 7
#define LIGHT_MAX_INTENSITY 8.0f
#define LIGHT_RANGE_MAX 15.36f

extern float spotAngTypes[];
extern float lights[LIGHT_COUNT * LIGHT_DATA_SIZE];
extern GLuint lightBufferID;
extern GLuint inputImageID, inputNormalsID, inputDepthID, outputImageID;

void InitializeLights(void);
void SetupGBuffer(void);

#endif // VOXEN_LIGHTS_H
