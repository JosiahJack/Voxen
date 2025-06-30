#ifndef VOXEN_IMAGE_EFFECTS_H
#define VOXEN_IMAGE_EFFECTS_H

#include <GL/glew.h>

// Image Effect Blit quad
extern GLuint imageBlitShaderProgram;
extern GLuint quadVAO, quadVBO;
void SetupQuad(void);

#endif // VOXEN_IMAGE_EFFECTS_H
