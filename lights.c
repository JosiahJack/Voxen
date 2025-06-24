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
GLuint shadowMapShaderProgram;
GLuint shadowFBO;
GLuint shadowCubemaps[LIGHT_COUNT];

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
    }
    
    lights[0] = 10.24f;
    lights[1] = 0.0f;
    lights[2] = 0.0f; // Fixed Z height
    lights[3] = 2.0f; // Default intensity
    lights[4] = 10.0f; // Default radius
    lights[6] = 0.0f;
    lights[7] = 0.0f;
    lights[8] = -1.0f;
    lights[9] = 1.0f;
    lights[10] = 1.0f;
    lights[11] = 1.0f;
    
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

    // Create and bind SSBO
    glGenBuffers(1, &lightBufferID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBufferID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, LIGHT_COUNT * LIGHT_DATA_SIZE * sizeof(float), lights, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, lightBufferID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    
    SetupShadowMaps();
    CacheUniformLocationsForShadowmapShader();
}

// Create cubemaps for each light
void SetupShadowMaps(void) {
    glGenTextures(LIGHT_COUNT, shadowCubemaps);
    for (int i = 0; i < LIGHT_COUNT; i++) {
        glBindTexture(GL_TEXTURE_CUBE_MAP, shadowCubemaps[i]);
        for (int face = 0; face < 6; face++) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_DEPTH_COMPONENT32F,
                         SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            fprintf(stderr, "Failed to create cubemap %d: OpenGL error %d\n", i, error);
        }
    }
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    // Create shadow FBO
    glGenFramebuffers(1, &shadowFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowCubemaps[0], 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "Shadow FBO incomplete: Error code %d\n", status);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLint shadviewLoc = -1, shadprojectionLoc = -1, shadmodelLoc = -1;

void CacheUniformLocationsForShadowmapShader(void) {
    shadviewLoc = glGetUniformLocation(shadowMapShaderProgram, "view");
    shadprojectionLoc = glGetUniformLocation(shadowMapShaderProgram, "projection");
    shadmodelLoc = glGetUniformLocation(shadowMapShaderProgram, "model");
}

float shadowmap_fov = 90.0f;
float shadowmap_aspect = 1.0f; // Square cubemap faces

void RenderPointLightShadowMap(int lightIndex, float lightPosX, float lightPosY, float lightPosZ) {
    float projection[16];
    mat4_perspective(projection, 90.0f, 1.0f, 0.02f, SHADOW_MAP_FARPLANE);
    for (int face = 0; face < 6; face++) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, shadowCubemaps[lightIndex], 0);
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) { fprintf(stderr, "Shadow FBO incomplete for light %d, face %d: Error %d\n", lightIndex, face, status); continue; }

        glClear(GL_DEPTH_BUFFER_BIT);

        // Set up view and projection matrices
        float view[16], projection[16];
        mat4_perspective(projection, shadowmap_fov,shadowmap_aspect, 0.02f, 20.0f);
        Quaternion shadcam_rotation;
        float yaw = 0.0f;   // 0 == y-, 90 == x+, 180 == y+, 270 == x-
        float pitch = 0.0f; // 0.0 == up, 180 == down
        float roll = 0.0f; // positive rotate view clockwise (world goes CCW)
        if (face == 0) {
            yaw = 0.0f; pitch = 0.0f; roll = 0.0f; // +X
        } else if (face == 1) {
            yaw = 180.0f; pitch = 0.0f; roll = 00.0f; // -X           
        } else if (face == 2) {
            yaw = 90.0f; pitch = 00.0f; roll = 0.0f; // +Y      
        } else if (face == 3) {
            yaw = 270.0f; pitch = 00.0f; roll = 0.0f; // -Y      
        } else if (face == 4) {
            yaw = 0.0f; pitch = -90.0f; roll = 0.0f; // +Z      
        } else if (face == 5) {
            yaw = 0.0f; pitch = 90.0f; roll = 0.0f; // -Z      
        }
        
        quat_from_yaw_pitch_roll(&shadcam_rotation,yaw,pitch,roll);
        mat4_lookat(view, lightPosX, lightPosY, lightPosZ, &shadcam_rotation);
        glUniformMatrix4fv(shadviewLoc, 1, GL_FALSE, view);
        glUniformMatrix4fv(shadprojectionLoc, 1, GL_FALSE, projection);
        glBindVertexArray(vao);

        // Render each model instance, simple draw calls, no instancing yet
        RenderMeshInstances();
    }

    lightDirty[lightIndex] = false;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, screen_width, screen_height);
}

void RenderDirtyShadowMaps(void) {
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
    glDepthFunc(GL_LESS);
    glUseProgram(shadowMapShaderProgram);
    glEnable(GL_DEPTH_TEST);    
    int updatesThisFrame = 0;
    const int maxUpdatesPerFrame = 32;
    for (int i = 0; i < LIGHT_COUNT && updatesThisFrame < maxUpdatesPerFrame; i++) {
        if (lightDirty[i] && lights[i * LIGHT_DATA_SIZE + 3] > 0.0f) { // Dirty and active
            //                                                        x                                y                               z
            RenderPointLightShadowMap(i, lights[i * LIGHT_DATA_SIZE + 0], lights[i * LIGHT_DATA_SIZE + 1],lights[i * LIGHT_DATA_SIZE + 2]);
            updatesThisFrame++;
        }
    }
    
    glDisable(GL_DEPTH_TEST);
}
