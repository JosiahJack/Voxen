// render.c
// Handles setup of OpenGL rendering related objects, buffers.  Defines calls
// for all rendering tasks.
// Render order should be:
// 1. Clear
// 2. Render static geometry 
// 3. Render UI images (TODO)
// 4. Render UI text (debug only at the moment, e.g. frame stats)

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <GL/glew.h>
#include <stdio.h>
#include <time.h>
#include "constants.h"
#include "text.glsl"
#include "data_textures.h"
#include "data_models.h"
#include "event.h"
#include "quaternion.h"
#include "matrix.h"
#include "player.h"
#include "render.h"
#include "input.h"
#include "lights.h"
#include "chunk.glsl"
#include "imageblit.glsl"
#include "deferred_lighting.compute"
#include "text.h"

uint32_t drawCallCount = 0;
uint32_t vertexCount = 0;

GLuint chunkShaderProgram;

int CompileShaders(void) {
    // ------------------------------
    // Chunk Shader
    
    // Vertex Subshader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        fprintf(stderr, "Vertex Shader Compilation Failed: %s\n", infoLog);
        return 1;
    }

    // Fragment Subshader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char *fragSource = fragmentShaderTraditional;
    glShaderSource(fragmentShader, 1, &fragSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        fprintf(stderr, "Fragment Shader Compilation Failed: %s\n", infoLog);
        return 1;
    }

    // Shader Program
    chunkShaderProgram = glCreateProgram();
    glAttachShader(chunkShaderProgram, vertexShader);
    glAttachShader(chunkShaderProgram, fragmentShader);
    glLinkProgram(chunkShaderProgram);
    glGetProgramiv(chunkShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(chunkShaderProgram, 512, NULL, infoLog);
        fprintf(stderr, "Chunk Shader Program Linking Failed: %s\n", infoLog);
        return 1;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // ------------------------------
    // Text Shader
    
    // Text Vertex Subshader
    GLuint textVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(textVertexShader, 1, &textVertexShaderSource, NULL);
    glCompileShader(textVertexShader);
    glGetShaderiv(textVertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(textVertexShader, 512, NULL, infoLog);
        fprintf(stderr, "Text Vertex Shader Compilation Failed: %s\n", infoLog);
        return 1;
    }

    // Text Fragment Subshader
    GLuint textFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(textFragmentShader, 1, &textFragmentShaderSource, NULL);
    glCompileShader(textFragmentShader);
    glGetShaderiv(textFragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(textFragmentShader, 512, NULL, infoLog);
        fprintf(stderr, "Text Fragment Shader Compilation Failed: %s\n", infoLog);
        return 1;
    }

    // Text Shader Program
    textShaderProgram = glCreateProgram();
    glAttachShader(textShaderProgram, textVertexShader);
    glAttachShader(textShaderProgram, textFragmentShader);
    glLinkProgram(textShaderProgram);
    glGetProgramiv(textShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(textShaderProgram, 512, NULL, infoLog);
        fprintf(stderr, "Text Shader Program Linking Failed: %s\n", infoLog);
        return 1;
    }

    glDeleteShader(textVertexShader);
    glDeleteShader(textFragmentShader);

    // ------------------------------  
    // Deferred Lighting Compute Shader Program
    GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(computeShader, 1, &deferredLighting_computeShader, NULL);
    glCompileShader(computeShader);
    glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(computeShader, 512, NULL, infoLog);
        fprintf(stderr, "Compute Shader Compilation Failed: %s\n", infoLog);
        return 1;
    }

    deferredLightingShaderProgram = glCreateProgram();
    glAttachShader(deferredLightingShaderProgram, computeShader);
    glLinkProgram(deferredLightingShaderProgram);
    glGetProgramiv(deferredLightingShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(deferredLightingShaderProgram, 512, NULL, infoLog);
        fprintf(stderr, "Compute Shader Program Linking Failed: %s\n", infoLog);
        return 1;
    }

    glDeleteShader(computeShader);
    
    // ------------------------------  
    // Image Blit Shader (For full screen image effects, rendering compute results, etc.)
    
    // Full Screen Quad Vertex Subshader
    GLuint quadVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(quadVertexShader, 1, &quadVertexShaderSource, NULL);
    glCompileShader(quadVertexShader);
    glGetShaderiv(quadVertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(quadVertexShader, 512, NULL, infoLog);
        fprintf(stderr, "Image Blit Vertex Shader Compilation Failed: %s\n", infoLog);
        return 1;
    }

    // Full Screen Quad Fragment Subshader
    GLuint quadFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(quadFragmentShader, 1, &quadFragmentShaderSource, NULL);
    glCompileShader(quadFragmentShader);
    glGetShaderiv(quadFragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(quadFragmentShader, 512, NULL, infoLog);
        fprintf(stderr, "Image Blit Fragment Shader Compilation Failed: %s\n", infoLog);
        return 1;
    }
    
    imageBlitShaderProgram = glCreateProgram();
    glAttachShader(imageBlitShaderProgram, quadVertexShader);
    glAttachShader(imageBlitShaderProgram, quadFragmentShader);
    glLinkProgram(imageBlitShaderProgram);
    glGetProgramiv(imageBlitShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(imageBlitShaderProgram, 512, NULL, infoLog);
        fprintf(stderr, "Image Blit Shader Program Linking Failed: %s\n", infoLog);
        return 1;
    }
    
    glDeleteShader(quadVertexShader);
    glDeleteShader(quadFragmentShader);

    // ------------------------------  

    return 0;
}

GLuint inputImageID, inputNormalsID, inputDepthID, inputWorldPosID, outputImageID, gBufferFBO;

void GenerateAndBindTexture(GLuint *id, GLenum internalFormat, int width, int height, GLenum format, GLenum type, const char *name) {
    glGenTextures(1, id);
    glBindTexture(GL_TEXTURE_2D, *id);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) fprintf(stderr, "Failed to create texture %s: OpenGL error %d\n", name, error);
}

// Create G-buffer textures
void SetupGBuffer(void) {
    // First pass gbuffer images
    GenerateAndBindTexture(&inputImageID,              GL_RGBA8, screen_width, screen_height,            GL_RGBA, GL_UNSIGNED_BYTE, "Unlit Raster Albedo Colors");
    GenerateAndBindTexture(&inputNormalsID,          GL_RGBA16F, screen_width, screen_height,            GL_RGBA, GL_UNSIGNED_BYTE, "Unlit Raster Normals");
    GenerateAndBindTexture(&inputDepthID, GL_DEPTH_COMPONENT32F, screen_width, screen_height, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, "Unlit Raster Depth");
    GenerateAndBindTexture(&inputWorldPosID,         GL_RGBA32F, screen_width, screen_height,            GL_RGBA, GL_UNSIGNED_BYTE, "Unlit Raster World Positions");
    
    // Second pass gbuffer images
    GenerateAndBindTexture(&outputImageID,             GL_RGBA8, screen_width, screen_height,            GL_RGBA, GL_UNSIGNED_BYTE, "Deferred Lighting Result Colors");

    // Create framebuffer
    glGenFramebuffers(1, &gBufferFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, inputImageID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, inputNormalsID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, inputWorldPosID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, inputDepthID, 0);
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, drawBuffers);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        switch (status) {
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: fprintf(stderr, "Framebuffer incomplete: Attachment issue\n"); break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: fprintf(stderr, "Framebuffer incomplete: Missing attachment\n"); break;
            case GL_FRAMEBUFFER_UNSUPPORTED: fprintf(stderr, "Framebuffer incomplete: Unsupported configuration\n"); break;
            default: fprintf(stderr, "Framebuffer incomplete: Error code %d\n", status);
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

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

int ClearFrameBuffers(void) {
    // Clear the G-buffer FBO
    glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set clear color (black, fully opaque)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Clear the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Ensure consistent clear color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//     glClear(GL_STENCIL_BUFFER_BIT); // Clear stencil each frame
    return 0;
}

// Global uniform locations (cached during init)
GLint viewLoc = -1, projectionLoc = -1, modelLoc = -1, texIndexLoc = -1;
GLint textureOffsetsLoc = -1, textureSizesLoc = -1;

void CacheUniformLocationsForChunkShader(void) {
    // Called after shader compilation in InitializeEnvironment
    viewLoc = glGetUniformLocation(chunkShaderProgram, "view");
    projectionLoc = glGetUniformLocation(chunkShaderProgram, "projection");
    modelLoc = glGetUniformLocation(chunkShaderProgram, "model");
    texIndexLoc = glGetUniformLocation(chunkShaderProgram, "texIndex");
    textureOffsetsLoc = glGetUniformLocation(chunkShaderProgram, "textureOffsets");
    textureSizesLoc = glGetUniformLocation(chunkShaderProgram, "textureSizes");
}

int RenderStaticMeshes(void) {
    // Update the test light to be "attached" to the player
    lights[0] = testLight_x;
    lights[1] = testLight_y;
    lights[2] = testLight_z;
    lights[3] = testLight_intensity;
    lights[4] = testLight_range;
    lights[5] = testLight_spotAng;
    
    drawCallCount = 0; // Reset per frame
    vertexCount = 0;
    glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
    glUseProgram(chunkShaderProgram);

    // Set up view and projection matrices
    float view[16], projection[16];
    float fov = 65.0f;
    mat4_perspective(projection, fov, (float)screen_width / screen_height, 0.02f, 1300.0f);
    mat4_lookat(view, cam_x, cam_y, cam_z, &cam_rotation);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, projection);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, colorBufferID);
    glUniform1uiv(textureOffsetsLoc, TEXTURE_COUNT, textureOffsets);
    glUniform2iv(textureSizesLoc, TEXTURE_COUNT, textureSizes);
    glBindVertexArray(vao);

    // Render each model, simple draw calls, no instancing yet
    float model[16];
    int drawCallLimit = 50;
    int currentModelType = 1;
    int loopIter = 0;
    for (int yarray=0;yarray<drawCallLimit;yarray++) {
        for (int xarray=0;xarray<drawCallLimit;xarray++) {
            mat4_identity(model);
            mat4_translate(model,(float)xarray * 2.56f, (float)yarray * 2.56f, 0.0f);
            glUniform1i(texIndexLoc, currentModelType);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model);
            glBindVertexBuffer(0, vbos[currentModelType], 0, VERTEX_ATTRIBUTES_COUNT * sizeof(float));
            glDrawArrays(GL_TRIANGLES, 0, modelVertexCounts[currentModelType]);
            drawCallCount++;
            vertexCount += modelVertexCounts[0];
            loopIter++;
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Apply deferred lighting with compute shader
    glUseProgram(deferredLightingShaderProgram);
    GLint screenWidthLoc = glGetUniformLocation(deferredLightingShaderProgram, "screenWidth");
    GLint screenHeightLoc = glGetUniformLocation(deferredLightingShaderProgram, "screenHeight");
    GLint lightDataSizeLoc = glGetUniformLocation(deferredLightingShaderProgram, "lightDataSize");
    glUniform1ui(screenWidthLoc, screen_width);
    glUniform1ui(screenHeightLoc, screen_height);
    glUniform1ui(lightDataSizeLoc, LIGHT_DATA_SIZE);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBufferID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, LIGHT_COUNT * LIGHT_DATA_SIZE * sizeof(float), lights, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, lightBufferID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glBindImageTexture(0, inputImageID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
    glBindImageTexture(1, inputNormalsID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
    glBindImageTexture(2, inputDepthID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_DEPTH_COMPONENT32F);
    glBindImageTexture(3, inputWorldPosID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(4, outputImageID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

    // Dispatch compute shader
    GLuint groupX = (screen_width + 7) / 8;
    GLuint groupY = (screen_height + 7) / 8;
    glDispatchCompute(groupX, groupY, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    
    // Render final gather lighting
    glUseProgram(imageBlitShaderProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, outputImageID);
    glUniform1i(glGetUniformLocation(imageBlitShaderProgram, "tex"), 0);
    glBindVertexArray(quadVAO);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glEnable(GL_DEPTH_TEST);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
    return 0;
}

// Renders text at x,y coordinates specified using pointer to the string array.
void render_debug_text(float x, float y, const char *text, SDL_Color color) {
    if (!font) { fprintf(stderr, "Font is NULL\n"); return; }
    if (!text) { fprintf(stderr, "Text is NULL\n"); return; }
    
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
    
    float scaleX = (float)rgba_surface->w;
    float scaleY = (float)rgba_surface->h;
    GLint texelSizeLoc = glGetUniformLocation(textShaderProgram, "texelSize");
    glUniform2f(texelSizeLoc, 1.0f / scaleX, 1.0f / scaleY);

    // Enable blending for text transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST); // Disable depth test for 2D overlay

    // Bind VAO and adjust quad position/size
    glBindVertexArray(textVAO);
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
    drawCallCount++;
    vertexCount+=4*128;

    // Cleanup
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST); // Re-enable depth test for 3D rendering
    glUseProgram(0);
    glDeleteTextures(1, &texture);
    SDL_FreeSurface(rgba_surface);
}

double lastFrameSecCountTime = 0.00;
uint32_t lastFrameSecCount = 0;
uint32_t framesPerLastSecond = 0;

int RenderUI(double deltaTime) {
    glEnable(GL_STENCIL_TEST);
    glStencilMask(0xFF); // Enable stencil writes
    glStencilFunc(GL_ALWAYS, 1, 0xFF); // Always write 1
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); // Replace stencil value
    glClear(GL_STENCIL_BUFFER_BIT); // Clear stencil each frame TODO: Ok to do in ClearFrameBuffers()??
    glDisable(GL_LIGHTING); // Disable lighting for text
    SDL_Color textCol = {255, 255, 255, 255}; // White
    SDL_Color textColRed = {255, 0, 0, 255};
    SDL_Color textColGreen = {20, 255, 30, 255};

    // Draw debug text
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
    render_debug_text(10, 70, text4, textColRed);
    
    char text5[128];
    snprintf(text5, sizeof(text5), "testLight_spotAng: %.4f", testLight_spotAng);
    render_debug_text(10, 95, text5, textCol);
    
    char text6[128];
    snprintf(text6, sizeof(text6), "testLight_intensity: %.4f", testLight_intensity);
    render_debug_text(10, 110, text6, textCol);
    
    // Frame stats
    drawCallCount++; // Add one more for this text render ;)
    char text0[256];
    snprintf(text0, sizeof(text0), "Frame time: %.6f (FPS: %d), Draw calls: %d, Vertices: %d", deltaTime * 1000.0,framesPerLastSecond,drawCallCount,vertexCount);
    render_debug_text(10, 10, text0, textColGreen); // Top-left corner (10, 10)
    double time_now = get_time();
    if ((time_now - lastFrameSecCountTime) >= 1.00) {
        lastFrameSecCountTime = time_now;
        framesPerLastSecond = globalFrameNum - lastFrameSecCount;
        lastFrameSecCount = globalFrameNum;
    }

    glDisable(GL_STENCIL_TEST);
    glStencilMask(0x00); // Disable stencil writes
    return 0;
}

// Main render call for entire graphics pipeline.
int ClientRender() {
    int exitCode = 0;
    exitCode = ClearFrameBuffers();
    if (exitCode) return exitCode;
    
    exitCode = RenderStaticMeshes(); // FIRST FOR RESETTING DRAW CALL COUNTER!
    if (exitCode) return exitCode;

    exitCode = RenderUI(get_time() - last_time);
    return exitCode;
}
