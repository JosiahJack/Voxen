#include "image_effects.h"

// Image Effect Blit quad
GLuint imageBlitShaderProgram;
GLuint quadVAO, quadVBO;
void SetupQuad(void) {
    float vertices[] = {
        -1.0f, -1.0f, 0.0f, 0.0f, // Bottom-left
        -1.0f,  1.0f, 0.0f, 1.0f, // Top-left
         1.0f,  1.0f, 1.0f, 1.0f, // Top-right
         1.0f, -1.0f, 1.0f, 0.0f  // Bottom-right
    };
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
