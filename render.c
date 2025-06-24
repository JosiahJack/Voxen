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
#include "data_textures.h"
#include "data_models.h"
#include "event.h"
#include "quaternion.h"
#include "matrix.h"
#include "player.h"
#include "render.h"
#include "input.h"
#include "lights.h"
#include "text.h"
#include "image_effects.h"
#include "instance.h"

uint32_t drawCallCount = 0;
uint32_t vertexCount = 0;

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

int instanceCount = 8;
Instance instances[8] = { [0 ... 7] = 
    {0,
    0.0f,0.0f,0.0f,
    0.0f,0.0f,0.0f,
    0.0f,0.0f,0.0f,0.0f}
};

void SetupInstances(void) {
    int currentModelType = 0;
    float xpos = 0.0f;
    float ypos = 0.0f;
    for (int idx=0;idx<instanceCount;idx++) {
        instances[idx].modelIndex = currentModelType;
        instances[idx].posx = xpos + ((float)idx * 5.12f); // Position in grid with gaps for shadow testing.
        instances[idx].posy = ypos + ((float)idx * 2.56f);
        instances[idx].posz = 0.0f;
        instances[idx].sclx = 1.0f; // Default scale
        instances[idx].scly = 1.0f;
        instances[idx].sclz = 1.0f;
        instances[idx].rotx = 0.0f; // Quaternion identity
        instances[idx].roty = 0.0f;
        instances[idx].rotz = 0.0f;
        instances[idx].rotw = 1.0f;
    }
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

//     glClear(GL_STENCIL_BUFFER_BIT); // Clear stencil each frame TODO: can do this here instead of down below?
    return 0;
}

// Global uniform locations (cached during init)
GLint viewLoc = -1, projectionLoc = -1, modelLoc = -1, texIndexLoc = -1;
GLint textureOffsetsLoc = -1, textureSizesLoc = -1, modelIndexLoc = -1, debugViewLoc = -1;

void CacheUniformLocationsForChunkShader(void) {
    // Called after shader compilation in InitializeEnvironment
    viewLoc = glGetUniformLocation(chunkShaderProgram, "view");
    projectionLoc = glGetUniformLocation(chunkShaderProgram, "projection");
    modelLoc = glGetUniformLocation(chunkShaderProgram, "model");
    texIndexLoc = glGetUniformLocation(chunkShaderProgram, "texIndex");
    textureOffsetsLoc = glGetUniformLocation(chunkShaderProgram, "textureOffsets");
    textureSizesLoc = glGetUniformLocation(chunkShaderProgram, "textureSizes");
    modelIndexLoc = glGetUniformLocation(chunkShaderProgram, "modelIndex");
    debugViewLoc = glGetUniformLocation(chunkShaderProgram, "debugView");
}

void RenderMeshInstances(GLint model_mat_loc, GLint texindex_loc, GLint modelindex_loc) {
    float model[16]; // 4x4 matrix
    int drawCallLimit = 10;
    int currentModelType = 0;
    int loopIter = 0;
    int modelIndex = 0;
    for (int yarray=0;yarray<drawCallLimit;yarray+=2) {
        for (int xarray=0;xarray<drawCallLimit;xarray+=2) {
            mat4_identity(model);
            mat4_translate(model,(float)xarray * 2.56f, (float)yarray * 2.56f, 0.0f);
            if (texindex_loc >= 0) glUniform1i(texindex_loc, currentModelType);
            glUniform1i(modelindex_loc, modelIndex);
            glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, model);
            glBindVertexBuffer(0, vbos[currentModelType], 0, VERTEX_ATTRIBUTES_COUNT * sizeof(float));
            glDrawArrays(GL_TRIANGLES, 0, modelVertexCounts[currentModelType]);
            drawCallCount++;
            modelIndex++;
            vertexCount += modelVertexCounts[0];
            loopIter++;
            
            if (xarray % 4 == 0) {
                currentModelType++;
                if (currentModelType >= MODEL_COUNT) currentModelType = 0;
            }
        }
    }
    
    // TODO: Figure out why this doesn't work
//     for (int i=0;i<instanceCount;i++) {
//         mat4_identity(model);
//         mat4_translate(model,instances[i].posx,instances[i].posy,instances[i].posz);
//         if (texindex_loc >= 0) glUniform1i(texindex_loc, currentModelType);
//         glUniform1i(modelindex_loc, instances[i].modelIndex);
//         glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, model);
//         glBindVertexBuffer(0, vbos[instances[i].modelIndex], 0, VERTEX_ATTRIBUTES_COUNT * sizeof(float));
//         glDrawArrays(GL_TRIANGLES, 0, modelVertexCounts[instances[i].modelIndex]);
//         drawCallCount++;
//         vertexCount += modelVertexCounts[0];
//     }   
}

int RenderStaticMeshes(void) {
    // Update the test light to be "attached" to the player
    lights[0] = testLight_x;
    lights[1] = testLight_y;
    lights[2] = testLight_z;
    lights[3] = testLight_intensity;
    lights[4] = testLight_range;
    lights[5] = testLight_spotAng;
//     lightDirty[0] = true;
    
    drawCallCount = 0; // Reset per frame
    vertexCount = 0;
    
    // Render Shadowmaps (aka rerender whole scene for each light, YIKES!)
//     RenderDirtyShadowMaps(); // Uses separate FBO                        DIDN'T WORK!!
    
    glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
    glUseProgram(chunkShaderProgram);
    glEnable(GL_DEPTH_TEST);

    // Set up view and projection matrices
    float view[16], projection[16];
    float fov = 65.0f;
    mat4_perspective(projection, fov, (float)screen_width / screen_height, 0.02f, 100.0f);
    mat4_lookat(view, cam_x, cam_y, cam_z, &cam_rotation);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, projection);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, colorBufferID);
    glUniform1uiv(textureOffsetsLoc, TEXTURE_COUNT, textureOffsets);
    glUniform2iv(textureSizesLoc, TEXTURE_COUNT, textureSizes);
    glUniform1i(debugViewLoc, debugView);
    glBindVertexArray(vao);

    // Render each model instance, simple draw calls, no instancing yet
    RenderMeshInstances(modelLoc, texIndexLoc, modelIndexLoc);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Apply deferred lighting with compute shader
    glUseProgram(deferredLightingShaderProgram);
    GLint screenWidthLoc = glGetUniformLocation(deferredLightingShaderProgram, "screenWidth");
    GLint screenHeightLoc = glGetUniformLocation(deferredLightingShaderProgram, "screenHeight");
    GLint lightFarPlaneLoc = glGetUniformLocation(deferredLightingShaderProgram, "lightFarPlane");
    glUniform1ui(screenWidthLoc, screen_width);
    glUniform1ui(screenHeightLoc, screen_height);
    glUniform1f(lightFarPlaneLoc, 20.0f);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBufferID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, LIGHT_COUNT * LIGHT_DATA_SIZE * sizeof(float), lights, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, lightBufferID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    
    glBindImageTexture(0, inputImageID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
    glBindImageTexture(1, inputNormalsID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
    glBindImageTexture(2, inputDepthID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_DEPTH_COMPONENT32F);
    glBindImageTexture(3, inputWorldPosID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(4, outputImageID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
    
//     for (int i = 0; i < LIGHT_COUNT; i++) {
//         glActiveTexture(GL_TEXTURE0 + i);
//         glBindTexture(GL_TEXTURE_CUBE_MAP, shadowCubemaps[i]);
//         char uniformName[32];
//         snprintf(uniformName, sizeof(uniformName), "shadowCubemaps[%d]", i);
//         glUniform1i(glGetUniformLocation(deferredLightingShaderProgram, uniformName), i);
//     }

    // Dispatch compute shader
    GLuint groupX = (screen_width + 7) / 8;
    GLuint groupY = (screen_height + 7) / 8;
    glDispatchCompute(groupX, groupY, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    
    // Render final gather lighting
    glUseProgram(imageBlitShaderProgram);
    glActiveTexture(GL_TEXTURE0);
    if (debugView == 0) {
        glBindTexture(GL_TEXTURE_2D, outputImageID);
    } else if (debugView == 1) {
        glBindTexture(GL_TEXTURE_2D, inputImageID);        
    } else if (debugView == 2) {
        glBindTexture(GL_TEXTURE_2D, inputNormalsID);        
    } else if (debugView == 3) {
        glBindTexture(GL_TEXTURE_2D, inputImageID);        
    }
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

    float cam_quat_yaw = 0.0f;
    float cam_quat_pitch = 0.0f;
    float cam_quat_roll = 0.0f;
    quat_to_euler(&cam_rotation,&cam_quat_yaw,&cam_quat_pitch,&cam_quat_roll);
    
    RenderFormattedText(10, 25, TEXT_WHITE, "x: %.2f, y: %.2f, z: %.2f", cam_x, cam_y, cam_z);
    RenderFormattedText(10, 40, TEXT_WHITE, "cam yaw: %.2f, cam pitch: %.2f, cam roll: %.2f", cam_yaw, cam_pitch, cam_roll);
    RenderFormattedText(10, 55, TEXT_WHITE, "cam quat yaw: %.2f, cam quat pitch: %.2f, cam quat roll: %.2f", cam_quat_yaw, cam_quat_pitch, cam_quat_roll);
    RenderFormattedText(10, 70, TEXT_WHITE, "Peak frame queue count: %d", maxEventCount_debug);
    RenderFormattedText(10, 95, TEXT_WHITE, "testLight_spotAng: %.4f", testLight_spotAng);
    RenderFormattedText(10, 110, TEXT_WHITE, "testLight_intensity: %.4f", testLight_intensity);
    RenderFormattedText(10, 125, TEXT_WHITE, "DebugView: %d", debugView);
    
    // Frame stats
    drawCallCount++; // Add one more for this text render ;)
    RenderFormattedText(10, 10, TEXT_WHITE, "Frame time: %.6f (FPS: %d), Draw calls: %d, Vertices: %d",
                        deltaTime * 1000.0,framesPerLastSecond,drawCallCount,vertexCount);
    
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
