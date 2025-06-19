#ifndef VOXEN_SHADERS_GLSL_H
#define VOXEN_SHADERS_GLSL_H

#include <GL/glew.h>

extern GLuint shaderProgram;
extern GLuint textShaderProgram;
extern GLuint cullShaderProgram;
extern GLuint rasterizeShaderProgram;
extern GLuint imageBlitShaderProgram;

int CompileShaders(void);

#endif // VOXEN_SHADERS_GLSL_H
