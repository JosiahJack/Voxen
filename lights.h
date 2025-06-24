#ifndef VOXEN_LIGHTS_H
#define VOXEN_LIGHTS_H

#include <GL/glew.h>
#include <stdbool.h>
#include "data_models.h"
                           //    0     1     2          3       4        5         6         7         8  9 10 11
#define LIGHT_DATA_SIZE 12 // posx, posy, posz, intensity, radius, spotAng, spotDirx, spotDiry, spotDirz, r, g, b
      // Make sure this^^^^ matches in deferredLighting_computeShader!
                   
#define LIGHT_COUNT 16 // MAX CITADEL LIGHT COUNT is 1561 for Level 7
#define LIGHT_MAX_INTENSITY 8.0f
#define LIGHT_RANGE_MAX 15.36f

#define VOXEL_DATA_SIZE 1
#define SHADOW_MAP_SIZE 512
#define SHADOW_MAP_FARPLANE 20.0

extern float spotAngTypes[];
extern float lights[LIGHT_COUNT * LIGHT_DATA_SIZE];
extern GLuint lightBufferID;
extern GLuint inputImageID, inputNormalsID, inputDepthID, outputImageID;

extern bool lightDirty[LIGHT_COUNT];
extern GLuint shadowCubemaps[LIGHT_COUNT]; // Array of cubemap texture IDs
extern GLuint shadowFBO; // Single FBO for rendering shadow maps
extern GLuint deferredLightingShaderProgram;
extern GLuint shadowMapShaderProgram;

void InitializeLights(void);
void SetupShadowMaps(void);
void CacheUniformLocationsForShadowmapShader(void);
void RenderPointLightShadowMap(int lightIndex, float lightPosX, float lightPosY, float lightPosZ);
void RenderDirtyShadowMaps(void);

#endif // VOXEN_LIGHTS_H
