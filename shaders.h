#ifndef VOXEN_SHADERS_H
#define VOXEN_SHADERS_H

#include <GL/glew.h>

extern GLuint chunkShaderProgram;
extern GLuint lightVolumeShaderProgram;
extern GLuint imageBlitShaderProgram;
extern GLuint screenSpaceShadowsComputeShader;
extern GLuint screenSpaceGIComputeShader;
extern GLuint textShaderProgram;

GLuint CompileShader(GLenum type, const char *source, const char *shaderName);
GLuint LinkProgram(GLuint *shaders, int count, const char *programName);
int CompileShaders(void);

#endif // VOXEN_SHADERS_H
