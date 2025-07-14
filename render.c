// render.c
// Handles setup of OpenGL rendering related objects, buffers.  Defines calls
// for all rendering tasks.
// Render order should be:
// 1. Clear
// 2. Render static geometry 
// 3. Render UI images (TODO)
// 4. Render UI text (debug only at the moment, e.g. frame stats)

#include <SDL2/SDL.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <GL/glew.h>
#include <stdio.h>
#include <string.h>
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
#include "instance.h"
#include "debug.h"
#include "voxel.h"

uint32_t drawCallCount = 0;
uint32_t vertexCount = 0;

GLuint inputImageID, inputNormalsID, inputDepthID, inputWorldPosID;
// GLuint outputImageID;
GLuint gBufferFBO;
GLuint inputTexMapsID;
float lightsInProximity[32 * LIGHT_DATA_SIZE];

void GenerateAndBindTexture(GLuint *id, GLenum internalFormat, int width, int height, GLenum format, GLenum type, const char *name) {
    glGenTextures(1, id);
    glBindTexture(GL_TEXTURE_2D, *id);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) DualLogError("Failed to create texture %s: OpenGL error %d\n", name, error);
}

// Create G-buffer textures
void SetupGBuffer(void) {
    DebugRAM("setup gbuffer start");
    // First pass gbuffer images
    GenerateAndBindTexture(&inputImageID,             GL_RGBA8, screen_width, screen_height,            GL_RGBA,           GL_UNSIGNED_BYTE, "Unlit Raster Albedo Colors");
    GenerateAndBindTexture(&inputNormalsID,         GL_RGBA16F, screen_width, screen_height,            GL_RGBA,              GL_HALF_FLOAT, "Unlit Raster Normals");
    GenerateAndBindTexture(&inputDepthID, GL_DEPTH_COMPONENT24, screen_width, screen_height, GL_DEPTH_COMPONENT,            GL_UNSIGNED_INT, "Unlit Raster Depth");
    GenerateAndBindTexture(&inputWorldPosID,        GL_RGBA32F, screen_width, screen_height,            GL_RGBA,                   GL_FLOAT, "Unlit Raster World Positions");
    GenerateAndBindTexture(&inputTexMapsID,         GL_RGBA32I, screen_width, screen_height,    GL_RGBA_INTEGER,                     GL_INT, "Unlit Raster Glow and Specular Map Indices");

    // Create framebuffer
    glGenFramebuffers(1, &gBufferFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, inputImageID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, inputNormalsID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, inputWorldPosID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, inputTexMapsID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, inputDepthID, 0);
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    glDrawBuffers(4, drawBuffers);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        switch (status) {
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: DualLogError("Framebuffer incomplete: Attachment issue\n"); break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: DualLogError("Framebuffer incomplete: Missing attachment\n"); break;
            case GL_FRAMEBUFFER_UNSUPPORTED: DualLogError("Framebuffer incomplete: Unsupported configuration\n"); break;
            default: DualLogError("Framebuffer incomplete: Error code %d\n", status);
        }
    }
    
    glBindImageTexture(0, inputImageID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
    glBindImageTexture(1, inputNormalsID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
    glBindImageTexture(2, inputDepthID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_DEPTH_COMPONENT24);
    glBindImageTexture(3, inputWorldPosID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(5, inputTexMapsID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32I);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);    
    DebugRAM("setup gbuffer end");
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
GLint viewLoc_chunk = -1, projectionLoc_chunk = -1, matrixLoc_chunk = -1;
GLint texIndexLoc_chunk = -1, textureCountLoc_chunk = -1;
GLint instanceIndexLoc_chunk = -1;
GLint modelIndexLoc_chunk = -1;
GLint debugViewLoc_chunk = -1;
GLint glowIndexLoc_chunk = -1;
GLint specIndexLoc_chunk = -1;

GLint screenWidthLoc_deferred = -1, screenHeightLoc_deferred = -1;
GLint shadowsEnabledLoc_deferred = -1;
GLint debugViewLoc_deferred = -1;

void CacheUniformLocationsForShaders(void) {
    // Called after shader compilation in InitializeEnvironment
    viewLoc_chunk = glGetUniformLocation(chunkShaderProgram, "view");
    projectionLoc_chunk = glGetUniformLocation(chunkShaderProgram, "projection");
    matrixLoc_chunk = glGetUniformLocation(chunkShaderProgram, "matrix");
    texIndexLoc_chunk = glGetUniformLocation(chunkShaderProgram, "texIndex");
    glowIndexLoc_chunk = glGetUniformLocation(chunkShaderProgram, "glowIndex");
    specIndexLoc_chunk = glGetUniformLocation(chunkShaderProgram, "specIndex");
    textureCountLoc_chunk = glGetUniformLocation(chunkShaderProgram, "textureCount");
    instanceIndexLoc_chunk = glGetUniformLocation(chunkShaderProgram, "instanceIndex");
    modelIndexLoc_chunk = glGetUniformLocation(chunkShaderProgram, "modelIndex");
    debugViewLoc_chunk = glGetUniformLocation(chunkShaderProgram, "debugView");
    
    screenWidthLoc_deferred = glGetUniformLocation(deferredLightingShaderProgram, "screenWidth");
    screenHeightLoc_deferred = glGetUniformLocation(deferredLightingShaderProgram, "screenHeight");
    shadowsEnabledLoc_deferred = glGetUniformLocation(deferredLightingShaderProgram, "shadowsEnabled");
    debugViewLoc_deferred = glGetUniformLocation(deferredLightingShaderProgram, "debugView");
}

void RenderMeshInstances(bool * instanceIsCulledArray) {    
    // Set up view and projection matrices
    float model[16]; // 4x4 matrix
    for (int i=0;i<INSTANCE_COUNT;i++) {
        if (instanceIsCulledArray[i]) continue;
        if (instances[i].modelIndex >= MODEL_COUNT) continue;
        if (instances[i].modelIndex < 0) continue; // Culled
        if (modelVertexCounts[instances[i].modelIndex] < 1) continue; // Empty model
        
        float x = instances[i].rotx, y = instances[i].roty, z = instances[i].rotz, w = instances[i].rotw;
        float len = sqrtf(x*x + y*y + z*z + w*w);
        if (len > 0.0f) {
            x /= len; y /= len; z /= len; w /= len;
        } else {
            x = 0.0f; y = 0.0f; z = 0.0f; w = 1.0f; // Default to identity rotation
        }
        
        float rot[16];
        mat4_identity(rot);
        rot[0] = 1.0f - 2.0f * (y*y + z*z);
        rot[1] = 2.0f * (x*y - w*z);
        rot[2] = 2.0f * (x*z + w*y);
        rot[4] = 2.0f * (x*y + w*z);
        rot[5] = 1.0f - 2.0f * (x*x + z*z);
        rot[6] = 2.0f * (y*z - w*x);
        rot[8] = 2.0f * (x*z - w*y);
        rot[9] = 2.0f * (y*z + w*x);
        rot[10] = 1.0f - 2.0f * (x*x + y*y);
        
        // Account for bad scale.  If instance is in the list, it should be visible!
        float sx = instances[i].sclx > 0.0f ? instances[i].sclx : 1.0f;
        float sy = instances[i].scly > 0.0f ? instances[i].scly : 1.0f;
        float sz = instances[i].sclz > 0.0f ? instances[i].sclz : 1.0f;
        
        model[0]  =       rot[0] * sx; model[1]  =       rot[1] * sy; model[2] =       rot[2]  * sz; model[3]  = 0.0f;
        model[4]  =       rot[4] * sx; model[5]  =       rot[5] * sy; model[6]  =      rot[6]  * sz; model[7]  = 0.0f;
        model[8]  =       rot[8] * sx; model[9]  =       rot[9] * sy; model[10] =      rot[10] * sz; model[11] = 0.0f;
        model[12] = instances[i].posx; model[13] = instances[i].posy; model[14] = instances[i].posz; model[15] = 1.0f;
        memcpy(&modelMatrices[i * 16], model, 16 * sizeof(float));
        glUniform1i(texIndexLoc_chunk, instances[i].texIndex);
        glUniform1i(glowIndexLoc_chunk, instances[i].glowIndex);
        glUniform1i(specIndexLoc_chunk, instances[i].specIndex);
        glUniform1i(instanceIndexLoc_chunk, i);
        int modelType = instances[i].modelIndex;
        glUniform1i(modelIndexLoc_chunk, modelType);
        glUniformMatrix4fv(matrixLoc_chunk, 1, GL_FALSE, model);
        glBindVertexBuffer(0, vbos[modelType], 0, VERTEX_ATTRIBUTES_COUNT * sizeof(float));
        
        if (isDoubleSided(instances[i].texIndex)) glDisable(GL_CULL_FACE); // Disable backface culling
        glDrawArrays(GL_TRIANGLES, 0, modelVertexCounts[modelType]);
        if (isDoubleSided(instances[i].texIndex)) glEnable(GL_CULL_FACE); // Reenable backface culling
        drawCallCount++;
        vertexCount += modelVertexCounts[modelType];

        // Uncomment for fun and mayhem, draw call 1, instance 0, error 501 for invalid input
//         GLenum err;
//         while ((err = glGetError()) != GL_NO_ERROR) DualLogError("GL Error for draw call %d: %x\n",i, err);
    }
}

bool shadowsEnabled = false;
uint32_t playerCellIdx = 80000;
uint32_t playerCellIdx_x = 20000;
uint32_t playerCellIdx_y = 10000;
uint32_t playerCellIdx_z = 451;
int numLightsFound = 0;

void TestStuffForRendering_DELETE_ME_LATER() {
    // Update the test light to be "attached" to the testLight point moved by j,k,u,i,n,m
    int lightBase = 0;
    lights[lightBase + 0] = testLight_x;
    lights[lightBase + 1] = testLight_y;
    lights[lightBase + 2] = testLight_z;
    lights[lightBase + 3] = testLight_intensity;
    lights[lightBase + 4] = testLight_range;
    lightsRangeSquared[lightBase] = testLight_range * testLight_range;
    lights[lightBase + 5] = testLight_spotAng;
    lights[lightBase + 9] = 1.0f; // r
    lights[lightBase + 10] = 1.0f; // g
    lights[lightBase + 11] = 1.0f; // b
    lightDirty[lightBase / 6] = true;
    instances[39].posx = testLight_x;
    instances[39].posy = testLight_y;
    instances[39].posz = testLight_z;
}

int RenderStaticMeshes(void) {
    TestStuffForRendering_DELETE_ME_LATER();

    drawCallCount = 0; // Reset per frame
    vertexCount = 0;
    
    // 1.Culling
    // 1a. Light Culling to max 32
    playerCellIdx = PositionToWorldCellIndex(cam_x, cam_y, cam_z);
    playerCellIdx_x = PositionToWorldCellIndexX(cam_x);
    playerCellIdx_y = PositionToWorldCellIndexY(cam_y);
    playerCellIdx_z = PositionToWorldCellIndexZ(cam_z);
    float sightRangeSquared = 71.68f * 71.68f; // Max player view, level 6 crawlway 28 cells
    numLightsFound = 0;
    for (int i=0;i<LIGHT_COUNT;++i) {
        if (lights[i + LIGHT_DATA_OFFSET_INTENSITY] < 0.015f) continue; // Off
        
        uint32_t litIdx = (i * LIGHT_DATA_SIZE);
        if (squareDistance3D(cam_x, cam_y, cam_z,
                             lights[litIdx + LIGHT_DATA_OFFSET_POSX],
                             lights[litIdx + LIGHT_DATA_OFFSET_POSY],
                             lights[litIdx + LIGHT_DATA_OFFSET_POSZ]) < sightRangeSquared) {
            
            int idx = numLightsFound * LIGHT_DATA_SIZE;
            lightsInProximity[idx + LIGHT_DATA_OFFSET_POSX] = lights[litIdx + LIGHT_DATA_OFFSET_POSX];
            lightsInProximity[idx + LIGHT_DATA_OFFSET_POSY] = lights[litIdx + LIGHT_DATA_OFFSET_POSY];
            lightsInProximity[idx + LIGHT_DATA_OFFSET_POSZ] = lights[litIdx + LIGHT_DATA_OFFSET_POSZ];
            lightsInProximity[idx + LIGHT_DATA_OFFSET_INTENSITY] = lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY];
            lightsInProximity[idx + LIGHT_DATA_OFFSET_RANGE] = lights[litIdx + LIGHT_DATA_OFFSET_RANGE];
            lightsInProximity[idx + LIGHT_DATA_OFFSET_SPOTANG] = lights[litIdx + LIGHT_DATA_OFFSET_SPOTANG];
            lightsInProximity[idx + LIGHT_DATA_OFFSET_SPOTDIRX] = lights[litIdx + LIGHT_DATA_OFFSET_SPOTDIRX];
            lightsInProximity[idx + LIGHT_DATA_OFFSET_SPOTDIRY] = lights[litIdx + LIGHT_DATA_OFFSET_SPOTDIRY];
            lightsInProximity[idx + LIGHT_DATA_OFFSET_SPOTDIRZ] = lights[litIdx + LIGHT_DATA_OFFSET_SPOTDIRZ];
            lightsInProximity[idx + LIGHT_DATA_OFFSET_R] = lights[litIdx + LIGHT_DATA_OFFSET_R];
            lightsInProximity[idx + LIGHT_DATA_OFFSET_G] = lights[litIdx + LIGHT_DATA_OFFSET_G];
            lightsInProximity[idx + LIGHT_DATA_OFFSET_B] = lights[litIdx + LIGHT_DATA_OFFSET_B];
            numLightsFound++;
            if (numLightsFound >= MAX_VISIBLE_LIGHTS) break; // Ok found 32 lights, cap it there.
        }
    }
    
    for (int i=numLightsFound;i<MAX_VISIBLE_LIGHTS;++i) {
        lightsInProximity[(i * LIGHT_DATA_SIZE) + LIGHT_DATA_OFFSET_INTENSITY] = 0.0f;
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vxgiID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_VISIBLE_LIGHTS * LIGHT_DATA_SIZE * sizeof(float), lightsInProximity, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 19, vxgiID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    
    // 1b. Instance Culling to only those in range of lights and player
    bool instanceIsCulledArray[INSTANCE_COUNT];
    memset(instanceIsCulledArray,true,INSTANCE_COUNT * sizeof(bool)); // All culled.
    float distSqrd = 0.0f;
    
    // Range to player culling of instances
    for (int i=0;i<INSTANCE_COUNT;++i) {
        distSqrd = squareDistance3D(instances[i].posx,instances[i].posy,instances[i].posz,cam_x, cam_y, cam_z);
        if (distSqrd < sightRangeSquared) instanceIsCulledArray[i] = false;
    }
    
    // Range to lights culling of instances
    // Well this is horribly slow for zero benefit (negative actually, adds 5 draw calls in test scene.
//     for (int j=0;j<LIGHT_COUNT;++j) {
//         for (int i=0;i<INSTANCE_COUNT;++i) {
//             if (!instanceIsCulledArray[i]) continue;
//             distSqrd = squareDistance3D(instances[i].posx,instances[i].posy,instances[i].posz,
//                                         lights[j + LIGHT_DATA_OFFSET_POSX], lights[j + LIGHT_DATA_OFFSET_POSY], lights[j + LIGHT_DATA_OFFSET_POSZ]);
//             
//             float boundsRadiusSquared = modelBounds[(i * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_RADIUS];
//             boundsRadiusSquared *= boundsRadiusSquared;
//             if ((distSqrd - (lightsRangeSquared[i] / 2.0f) - (boundsRadiusSquared / 2.0f)) < 0.0f) {
//                 instanceIsCulledArray[i] = false; // 1 - 0.4 - 0.3 = 0.3, circles not touching; 1 - 0.6 - 0.5 = -0.1, circles touching
//             }
//         }
//     }
    
    // 1. Unlit Raterized Geometry
    //        Standard vertex + fragment rendering, but
    //        with special packing to minimize transfer data amounts
    glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
    glUseProgram(chunkShaderProgram);
    glEnable(GL_DEPTH_TEST);
    float view[16], projection[16]; // Set up view and projection matrices
    float fov = 65.0f;
    mat4_perspective(projection, fov, (float)screen_width / screen_height, 0.02f, 100.0f);
    mat4_lookat(view, cam_x, cam_y, cam_z, &cam_rotation);
    glUniformMatrix4fv(viewLoc_chunk,       1, GL_FALSE,       view);
    glUniformMatrix4fv(projectionLoc_chunk, 1, GL_FALSE, projection);
    glUniform1i(debugViewLoc_chunk, debugView);
    glBindVertexArray(vao);
    
    // These should be static but cause issues if not...
    glUniform1i(textureCountLoc_chunk, textureCount); // Needed or else the texture index for test light stops rendering as unlit by deferred shader
    
    RenderMeshInstances(instanceIsCulledArray); // Render each model type's instances
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 3. Deferred Lighting + Shadow Calculations
    //        Apply deferred lighting with compute shader.  All lights are
    //        dynamic and can be updated at any time (flicker, light switches,
    //        move, change color, get marked as "culled" so shader can skip it,
    //        etc.).
    glUseProgram(deferredLightingShaderProgram);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // These should be static but cause issues if not...
    glUniform1ui(screenWidthLoc_deferred, screen_width); // Makes screen all black if not sent every frame.
    glUniform1ui(screenHeightLoc_deferred, screen_height); // Makes screen all black if not sent every frame.
    glUniform1i(debugViewLoc_deferred, debugView);

    glUniform1i(shadowsEnabledLoc_deferred, shadowsEnabled);
    float viewInv[16];
    mat4_inverse(viewInv,view);
    float projInv[16];
    mat4_inverse(projInv,projection);

    // Dispatch compute shader
    GLuint groupX = (screen_width + 7) / 8;
    GLuint groupY = (screen_height + 7) / 8;
    glDispatchCompute(groupX, groupY, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT); // Runs slightly faster 0.1ms without this, but may need if more shaders added in between
    
    // 4. Render final results with full screen quad.
    glUseProgram(imageBlitShaderProgram);
    glActiveTexture(GL_TEXTURE0);
    if (debugView == 0) {
        glBindTexture(GL_TEXTURE_2D, inputImageID); // Normal
    } else if (debugView == 1) {
        glBindTexture(GL_TEXTURE_2D, inputImageID); // Unlit
    } else if (debugView == 2) {
        glBindTexture(GL_TEXTURE_2D, inputNormalsID); // Triangle Normals 
    } else if (debugView == 3) {
        glBindTexture(GL_TEXTURE_2D, inputImageID); // Depth.  Values must be decoded in shader
    } else if (debugView == 4) {
        glBindTexture(GL_TEXTURE_2D, inputImageID); // Instance, Model, Texture indices as rgb. Values must be decoded in shader divided by counts.
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
uint32_t worstFPS = UINT32_MAX;

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
    RenderFormattedText(10, 85, TEXT_WHITE, "testLight intensity: %.4f, range: %.4f, spotAng: %.4f, x: %.3f, y: %.3f, z: %.3f", testLight_intensity,testLight_range,testLight_spotAng,testLight_x,testLight_y,testLight_z);
    RenderFormattedText(10, 100, TEXT_WHITE, "DebugView: %d, %s", debugView, debugView == 1 ? "unlit" : "normal");
    RenderFormattedText(10, 115, TEXT_WHITE, "Num lights: %d   Player cell:: x: %d, y: %d, z: %d", numLightsFound, playerCellIdx_x, playerCellIdx_y, playerCellIdx_z);
    
    // Frame stats
    drawCallCount++; // Add one more for this text render ;)
    RenderFormattedText(10, 10, TEXT_WHITE, "Frame time: %.6f (FPS: %d), Draw calls: %d, Vertices: %d, Worst FPS: %d",
                        deltaTime * 1000.0,framesPerLastSecond,drawCallCount,vertexCount,worstFPS);
    
    double time_now = get_time();
    if ((time_now - lastFrameSecCountTime) >= 1.00) {
        lastFrameSecCountTime = time_now;
        framesPerLastSecond = globalFrameNum - lastFrameSecCount;
        if (framesPerLastSecond < worstFPS && globalFrameNum > 10) worstFPS = framesPerLastSecond; // After startup, keep track of worst framerate seen.
        lastFrameSecCount = globalFrameNum;
    }

    glDisable(GL_STENCIL_TEST);
    glStencilMask(0x00); // Disable stencil writes
    return 0;
}
