#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <GL/glew.h>
#include "constants.h"
#include "shaders.glsl.h"
#include "data_textures.h"
#include "data_models.h"
#include "event.h"
#include "transform.h"
#include "quaternion.h"
#include "matrix.h"
#include "player.h"
#include "render.h"

// Image Effect quad
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

Transform transformsBuffer[INSTANCE_COUNT] = {
    // Model 0: med1_1.fbx at (0, 1.28, 0), no rotation, scale 1
    {{ 0.00f, 1.28f, 0.00f}, {0.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
    
    // Model 1: med1_7.fbx at (2.56, 1.28, 0), no rotation, scale 1
    {{ 2.56f, 1.28f, 0.00f}, {0.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
    
    // Model 2: med1_9.fbx at (-2.56, 1.28, 0), no rotation, scale 1
    {{-2.56f, 1.28f, 0.00f}, {0.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}}
};

GLuint testVBO, outputTexture;

// Setup test vertex buffer for 4 triangles
void SetupTestVertexBuffer(void) {
float vertices[12 * 6] = {
    // Triangle 1 (top-left) - Reversed to CCW
    200.0f, 400.0f, 1.0f, 0.0f, 1.0f, 0.0f,  // (200, 400)
    300.0f, 500.0f, 1.0f, 1.0f, 0.0f, 0.0f,  // (300, 500)
    200.0f, 500.0f, 1.0f, 0.0f, 0.0f, 0.0f,  // (200, 500)
    // Triangle 2 (top-right) - Reversed to CCW
    200.0f, 400.0f, 1.0f, 0.0f, 1.0f, 0.0f,  // (200, 400)
    300.0f, 400.0f, 1.0f, 1.0f, 1.0f, 0.0f,  // (300, 400)
    300.0f, 500.0f, 1.0f, 1.0f, 0.0f, 0.0f,  // (300, 500)
    // Triangle 3 (bottom-left, different texture) - Reversed to CCW
    400.0f, 200.0f, 2.0f, 0.0f, 1.0f, 1.0f,  // (400, 200)
    500.0f, 300.0f, 2.0f, 1.0f, 0.0f, 1.0f,  // (500, 300)
    400.0f, 300.0f, 2.0f, 0.0f, 0.0f, 1.0f,  // (400, 300)
    // Triangle 4 (bottom-right) - Reversed to CCW
    400.0f, 200.0f, 2.0f, 0.0f, 1.0f, 1.0f,  // (400, 200)
    500.0f, 200.0f, 2.0f, 1.0f, 1.0f, 1.0f,  // (500, 200)
    500.0f, 300.0f, 2.0f, 1.0f, 0.0f, 1.0f   // (500, 300)
};
    
    for (int i=0;i<4;i++) {
        int baseIdx = i * 18;
        printf("v0: %f\n",vertices[baseIdx]);
        printf("v0: %f\n",vertices[baseIdx + 6]);
        printf("v0: %f\n",vertices[baseIdx + 12]);
    }

    glGenBuffers(1, &testVBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, testVBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 12 * 6 * sizeof(float), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

// Setup output texture for compute shader
void SetupOutputTexture(void) {
    glGenTextures(1, &outputTexture);
    glBindTexture(GL_TEXTURE_2D, outputTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, screen_width, screen_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// Quad for text (2 triangles, positions and tex coords)
float textQuadVertices[] = {
    // Positions   // Tex Coords
    0.0f, 0.0f,    0.0f, 0.0f, // Bottom-left
    1.0f, 0.0f,    1.0f, 0.0f, // Bottom-right
    1.0f, 1.0f,    1.0f, 1.0f, // Top-right
    0.0f, 1.0f,    0.0f, 1.0f  // Top-left
};

void SetupTextQuad(void) {
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(textQuadVertices), textQuadVertices, GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Tex Coord
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

int SetupGeometry(void) {
    SetupTestVertexBuffer();
    SetupOutputTexture();
    SetupQuad();

    float *vertexData = NULL;
    uint32_t vertexCount = 0;
    if (LoadModels(&vertexData, &vertexCount)) { fprintf(stderr, "Failed to load models!\n"); return 1; }

    // VBO and VAO setup
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * 9 * sizeof(float), vertexData, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // Instance SSBO
    glGenBuffers(1, &instanceSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, instanceSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, INSTANCE_COUNT * sizeof(Transform), transformsBuffer, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, instanceSSBO);

    // Model bounds SSBO
    glGenBuffers(1, &modelBoundsBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelBoundsBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, MODEL_COUNT * sizeof(float), modelRadius, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, modelBoundsBuffer);

    // Indirect draw buffer
    glGenBuffers(1, &indirectBuffer);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBuffer);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, INSTANCE_COUNT * sizeof(DrawArraysIndirectCommand), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, indirectBuffer);

    // Instance ID buffer
    glGenBuffers(1, &instanceIDBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, instanceIDBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, INSTANCE_COUNT * sizeof(uint32_t), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, instanceIDBuffer);

    // Draw count buffer
    glGenBuffers(1, &drawCountBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, drawCountBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(uint32_t), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, drawCountBuffer);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glBindVertexArray(0);
    free(vertexData);
    return 0;
}

int ClearFrameBuffers(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    return 0;
}

int RenderStaticMeshes(void) {
    // Compute shader: culling
    glUseProgram(cullShaderProgram);

    // Reset draw count
    uint32_t zero = 0;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, drawCountBuffer);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(uint32_t), &zero);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // Set uniforms
    glUniform3f(glGetUniformLocation(cullShaderProgram, "cameraPos"), cam_x, cam_y, cam_z);
    float fov = 65.0f * M_PI / 180.0f; // Convert to radians
    float aspect = (float)screen_width / screen_height;
    float yawFov = fov * aspect;
    float pitchFov = fov;
    glUniform1f(glGetUniformLocation(cullShaderProgram, "yawMin"), -yawFov / 2.0f);
    glUniform1f(glGetUniformLocation(cullShaderProgram, "yawMax"), yawFov / 2.0f);
    glUniform1f(glGetUniformLocation(cullShaderProgram, "pitchMin"), -pitchFov / 2.0f);
    glUniform1f(glGetUniformLocation(cullShaderProgram, "pitchMax"), pitchFov / 2.0f);
    glUniform1uiv(glGetUniformLocation(cullShaderProgram, "vbo_offsets"), MODEL_COUNT, vbo_offsets);
    glUniform1uiv(glGetUniformLocation(cullShaderProgram, "modelVertexCounts"), MODEL_COUNT, modelVertexCounts);

    // Dispatch compute shader
    glDispatchCompute((INSTANCE_COUNT + 63) / 64, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_COMMAND_BARRIER_BIT);
    
    // Rasterization compute shader (test with 4 triangles)
    glUseProgram(rasterizeShaderProgram);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, testVBO);                        // BINDING 0
    glBindImageTexture(1, outputTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8); // BINDING 1
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, colorBufferID);                  // BINDING 2
    glUniform1uiv(glGetUniformLocation(rasterizeShaderProgram, "textureOffsets"), TEXTURE_COUNT, textureOffsets);
    glUniform2iv(glGetUniformLocation(rasterizeShaderProgram, "textureSizes"), TEXTURE_COUNT, textureSizes);
    glUniform1ui(glGetUniformLocation(rasterizeShaderProgram, "screenWidth"), screen_width);
    glUniform1ui(glGetUniformLocation(rasterizeShaderProgram, "screenHeight"), screen_height);
    glDispatchCompute((GLuint)ceil(screen_width / 8.0), (GLuint)ceil(screen_height / 8.0), 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glUseProgram(imageBlitShaderProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, outputTexture);
    glUniform1i(glGetUniformLocation(imageBlitShaderProgram, "tex"), 0);
    glBindVertexArray(quadVAO);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glEnable(GL_DEPTH_TEST);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);

    // Graphics pipeline
//     glUseProgram(shaderProgram);
//     glUniform3f(glGetUniformLocation(shaderProgram, "cameraPos"), cam_x, cam_y, cam_z); // Set vertex shader uniforms
//     glUniform1f(glGetUniformLocation(shaderProgram, "yawMin"), -yawFov / 2.0f);
//     glUniform1f(glGetUniformLocation(shaderProgram, "yawMax"), yawFov / 2.0f);
//     glUniform1f(glGetUniformLocation(shaderProgram, "pitchMin"), -pitchFov / 2.0f);
//     glUniform1f(glGetUniformLocation(shaderProgram, "pitchMax"), pitchFov / 2.0f);
//     glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, colorBufferID);
//     glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, instanceSSBO);
//     glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, instanceIDBuffer);
//     glUniform1uiv(glGetUniformLocation(shaderProgram, "textureOffsets"), TEXTURE_COUNT, textureOffsets);
//     glUniform2iv(glGetUniformLocation(shaderProgram, "textureSizes"), TEXTURE_COUNT, textureSizes);
//     glBindVertexArray(vao);
//     glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBuffer);
//     glMultiDrawArraysIndirect(GL_TRIANGLES, 0, INSTANCE_COUNT, 0); // Draw visible instances
//     glBindVertexArray(0);
//     glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
//     glUseProgram(0);
    return 0;
}

// Renders text at x,y coordinates specified using pointer to the string array.
void render_debug_text(float x, float y, const char *text, SDL_Color color) {
    if (!font || !text) { fprintf(stderr, "Font or text is NULL\n"); return; }
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);
    if (!surface) { fprintf(stderr, "TTF_RenderText_Solid failed: %s\n", TTF_GetError()); return; }
    SDL_Surface *rgba_surface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(surface);
    if (!rgba_surface) { fprintf(stderr, "SDL_ConvertSurfaceFormat failed: %s\n", SDL_GetError()); return; }

    // Create and bind texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rgba_surface->w, rgba_surface->h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, rgba_surface->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Use text shader
    glUseProgram(textShaderProgram);

    // Set up orthographic projection
    float projection[16];
    mat4_ortho(projection, 0.0f, (float)screen_width, (float)screen_height, 0.0f, -1.0f, 1.0f);
    GLint projLoc = glGetUniformLocation(textShaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection);

    // Set text color (convert SDL_Color to 0-1 range)
    float r = color.r / 255.0f;
    float g = color.g / 255.0f;
    float b = color.b / 255.0f;
    float a = color.a / 255.0f;
    GLint colorLoc = glGetUniformLocation(textShaderProgram, "textColor");
    glUniform4f(colorLoc, r, g, b, a);

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    GLint texLoc = glGetUniformLocation(textShaderProgram, "textTexture");
    glUniform1i(texLoc, 0);

    // Enable blending for text transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST); // Disable depth test for 2D overlay

    // Bind VAO and adjust quad position/size
    glBindVertexArray(textVAO);
    float scaleX = (float)rgba_surface->w;
    float scaleY = (float)rgba_surface->h;
    float vertices[] = {
        x,          y,          0.0f, 0.0f, // Bottom-left
        x + scaleX, y,          1.0f, 0.0f, // Bottom-right
        x + scaleX, y + scaleY, 1.0f, 1.0f, // Top-right
        x,          y + scaleY, 0.0f, 1.0f  // Top-left
    };
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    // Render quad (two triangles)
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // Cleanup
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST); // Re-enable depth test for 3D rendering
    glUseProgram(0);
    glDeleteTextures(1, &texture);
    SDL_FreeSurface(rgba_surface);
}

int RenderUI(double deltaTime) {
    glDisable(GL_LIGHTING); // Disable lighting for text
    SDL_Color textCol = {255, 255, 255, 255}; // White

    // Draw debug text
    char text0[128];
    snprintf(text0, sizeof(text0), "Frame time: %.6f", deltaTime * 1000.0);
    render_debug_text(10, 10, text0, textCol); // Top-left corner (10, 10)

    char text1[128];
    snprintf(text1, sizeof(text1), "x: %.2f, y: %.2f, z: %.2f", cam_x, cam_y, cam_z);
    render_debug_text(10, 25, text1, textCol); // Top-left corner (10, 10)

    float cam_quat_yaw = 0.0f;
    float cam_quat_pitch = 0.0f;
    float cam_quat_roll = 0.0f;
    quat_to_euler(&cam_rotation,&cam_quat_yaw,&cam_quat_pitch,&cam_quat_roll);
    char text2[128];
    snprintf(text2, sizeof(text2), "cam yaw: %.2f, cam pitch: %.2f, cam roll: %.2f", cam_yaw, cam_pitch, cam_roll);
    render_debug_text(10, 40, text2, textCol);

    char text3[128];
    snprintf(text3, sizeof(text3), "cam quat yaw: %.2f, cam quat pitch: %.2f, cam quat roll: %.2f", cam_quat_yaw, cam_quat_pitch, cam_quat_roll);
    render_debug_text(10, 55, text3, textCol);

    char text4[128];
    snprintf(text4, sizeof(text4), "Peak frame queue count: %.2d", maxEventCount_debug);
    render_debug_text(10, 70, text4, textCol);
    return 0;
}
