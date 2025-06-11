#ifndef SHADERS_GLSL_H
#define SHADERS_GLSL_H

#include <stdbool.h>

extern GLuint shaderProgram;
extern GLuint textShaderProgram;
extern bool use_bindless_textures;

int CompileShaders(void);

#endif // SHADERS_GLSL_H
