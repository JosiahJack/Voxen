#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include "render.h"
#include "matrix.h"
#include "constants.h"
#include "lights.h"
#include "quaternion.h"

float spotAngTypes[8] = { 0.0f, 30.0f, 45.0f, 60.0f, 75.0f, 90.0f, 135.0f, 151.7f }; // What?  I only have 6 spot lights and half are 151.7 and other half are 135.

GLuint deferredLightingShaderProgram;
float lights[LIGHT_COUNT * LIGHT_DATA_SIZE];
GLuint lightBufferID;
bool lightDirty[LIGHT_COUNT] = { [0 ... LIGHT_COUNT-1] = true };

// Initialize lights with random positions
void InitializeLights(void) {
    srand(time(NULL)); // Seed random number generator
    for (int i = 0; i < LIGHT_COUNT; i++) {
        int base = i * LIGHT_DATA_SIZE; // Step by 12
        lights[base + 0] = base * 0.08f; // posx
        lights[base + 1] = base * 0.08f; // posy
        lights[base + 2] = 0.0f; // posz
        lights[base + 3] = base > 12 * 256 ? 0.0f : 2.0f; // intensity
        lights[base + 4] = 5.24f; // radius
        lights[base + 5] = 0.0f; // spotAng
        lights[base + 6] = 0.0f; // spotDirx
        lights[base + 7] = 0.0f; // spotDiry
        lights[base + 8] = -1.0f; // spotDirz
        lights[base + 9] = 1.0f; // r
        lights[base + 10] = 1.0f; // g
        lights[base + 11] = 1.0f; // b
        lightDirty[i] = true;
    }
    
    lights[0] = 0.0f;
    lights[1] = -1.28f;
    lights[2] = 0.0f; // Fixed Z height
    lights[3] = 2.0f; // Default intensity
    lights[4] = 10.0f; // Default radius
    lights[6] = 0.0f;
    lights[7] = 0.0f;
    lights[8] = -1.0f;
    lights[9] = 1.0f;
    lights[10] = 1.0f;
    lights[11] = 1.0f;
    lightDirty[0] = true;
    
    lights[0 + 12] = 10.24f;
    lights[1 + 12] = 0.0f;
    lights[2 + 12] = 0.0f; // Fixed Z height
    lights[3 + 12] = 2.0f; // Default intensity
    lights[4 + 12] = 10.0f; // Default radius
    lights[6 + 12] = 0.0f;
    lights[7 + 12] = 0.0f;
    lights[8 + 12] = -1.0f;
    lights[9 + 12] = 1.0f;
    lights[10 + 12] = 0.0f;
    lights[11 + 12] = 0.0f;
    lightDirty[1] = true;

    // Create and bind SSBO
    glGenBuffers(1, &lightBufferID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBufferID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, LIGHT_COUNT * LIGHT_DATA_SIZE * sizeof(float), lights, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, lightBufferID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}
