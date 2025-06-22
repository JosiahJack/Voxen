#ifndef LIGHTS_H
#define LIGHTS_H

#include <GL/glew.h>

typedef struct {
    float position[3];
    float intensity;
    float radius;
    float spotAng; // In degrees, 0 for point lights
    float spotDir[3];
} LightData;

extern GLuint lightBufferID;
extern GLuint inputImageID, inputNormalsID, inputDepthID, outputImageID;

void InitializeLights(void);
void SetupGBuffer(void);

#endif // LIGHTS_H
