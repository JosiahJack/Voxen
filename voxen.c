// File: voxen.c
// Description: A realtime OpenGL 4.3+ Game Engine for Citadel: The System Shock Fan Remake
#define VERSION_STRING "v0.7.1"
#include <malloc.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <enet/enet.h>
#include "External/miniaudio.h"
// #include <fluidlite.h> // Temporarily disabled midi support until wav+mp3 is working.
// #include <libxmi.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "External/stb_image_write.h"
#define VOXEN_ENGINE_IMPLEMENTATION
#include "voxen.h"
#include "citadel.h"
#include "Shaders/text_vert.glsl.h" // Shaders are converted into string headers at build time.
#include "Shaders/text_frag.glsl.h"
#include "Shaders/chunk_vert.glsl.h"
#include "Shaders/chunk_frag.glsl.h"
#include "Shaders/shadowmap_vert.glsl.h"
#include "Shaders/shadowmap_frag.glsl.h"
#include "Shaders/composite_vert.glsl.h"
#include "Shaders/composite_frag.glsl.h"
#include "Shaders/ssr.compute.h"
// #include "Shaders/bluenoise64.cginc"
// ----------------------------------------------------------------------------
// Window
GLFWwindow *window;
bool inventoryMode = false;
uint16_t screen_width = 1366, screen_height = 768;
FILE* console_log_file = NULL;
// ----------------------------------------------------------------------------
// Settings
uint8_t settings_Reflections = 1u; // Default 1
uint8_t settings_Shadows = 2u; // Default 2 (1 is hard shadows, 2 enables Pseudo-Stochastic PCF sampling softening
uint8_t settings_AntiAliasing = 1u; // Default 1
uint8_t settings_Brightness = 100u; // Default 100 (for %)
float lodRangeSqrd = 38.4f * 38.4f;
// ----------------------------------------------------------------------------
// Instances
Entity instances[INSTANCE_COUNT];
float modelMatrices[INSTANCE_COUNT * 16];
uint8_t dirtyInstances[INSTANCE_COUNT];
GLuint instancesBuffer;
GLuint matricesBuffer;
// ----------------------------------------------------------------------------
// Game/Mod Definition
char global_modname[256];
bool global_modIsCitadel = false;
uint8_t numLevels = 2;
uint8_t startLevel = 3;
uint8_t currentLevel = 0;
bool gamePaused = false;
bool menuActive = false;
float pauseRelativeTime = 0.0f;
// ----------------------------------------------------------------------------
// Camera variables
// Start Actual: Puts player on Medical Level in actual game start position
float cam_x = -20.4f, cam_y = -43.79f + 0.84f, cam_z = 10.2f; // Added 0.84f for cam offset from center
float cam_yaw = 90.0f;
float cam_pitch = 0.0f;
float cam_roll = 0.0f;
Quaternion cam_rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
float cam_forwardx = 0.0f, cam_forwardy = 0.0f, cam_forwardz = 0.0f;
float cam_rightx = 0.0f, cam_righty = 0.0f, cam_rightz = 0.0f;
float cam_fov = 65.0f;
double last_mouse_x = 0.0, last_mouse_y = 0.0;
// ----------------------------------------------------------------------------
// OpenGL / Rendering
int32_t debugView = 0;
int32_t debugValue = 0;
float rasterPerspectiveProjection[16];
float shadowmapsPerspectiveProjection[16];
uint32_t drawCallsRenderedThisFrame = 0; // Total draw calls this frame
uint32_t verticesRenderedThisFrame = 0;
bool instanceIsLODArray[INSTANCE_COUNT];
GLuint inputImageID, inputNormalsID, inputDepthID, inputWorldPosID, gBufferFBO, outputImageID; // FBO
// ----------------------------------------------------------------------------
// Shaders
//    Chunk Geometery Unlit Raster Shader
GLuint chunkShaderProgram;
GLuint vao_chunk; // Vertex Array Object
GLint viewProjLoc_chunk = -1, matrixLoc_chunk = -1, texIndexLoc_chunk = -1, debugViewLoc_chunk = -1, debugValueLoc_chunk = -1, 
      glowSpecIndexLoc_chunk = -1, normInstanceIndexLoc_chunk = -1, screenWidthLoc_chunk = -1, screenHeightLoc_chunk = -1, 
      worldMin_xLoc_chunk = -1, worldMin_zLoc_chunk = -1, camPosLoc_chunk = -1, fogColorRLoc_chunk = -1, fogColorGLoc_chunk = -1,
      fogColorBLoc_chunk = -1;
GLuint blueNoiseBuffer;
float fogColorR = 0.04f, fogColorG = 0.04f, fogColorB = 0.09f;

//    Shadowmap Rastered Depth Shader
GLuint shadowCubeMap;
GLuint shadowFBO;
GLuint shadowmapsShaderProgram;
GLint modelMatrixLoc_shadowmaps = -1, viewProjMatrixLoc_shadowmaps = -1, texIndexLoc_shadowmaps = -1, glowSpecIndexLoc_shadowmaps = -1, normInstanceIndexLoc_shadowmaps = -1;
GLuint shadowMapSSBO; // SSBO for storing all shadow maps
bool shadowMapsRendered = false;
uint32_t lightIsDynamic[LIGHT_COUNT + 31 / 32] = {0};
uint16_t staticLightCount = 0;
uint16_t staticLightIndices[LIGHT_COUNT];

//    SSR (Screen Space Reflections)
#define SSR_RES 4 // 25% of render resolution.
GLuint ssrShaderProgram;
GLint screenWidthLoc_ssr = -1, screenHeightLoc_ssr = -1, viewProjectionLoc_ssr = -1, camPosLoc_ssr = -1;

//    Full Screen Quad Blit for rendering final output/image effect passes
GLuint imageBlitShaderProgram;
GLuint quadVAO, quadVBO;
GLint texLoc_quadblit = -1, debugViewLoc_quadblit = -1, debugValueLoc_quadblit = -1,
      screenWidthLoc_imageBlit = -1, screenHeightLoc_imageBlit = -1;
// ----------------------------------------------------------------------------
// Lights
// Could reduce spotAng to minimal bits.  I only have 6 spot lights and half are 151.7 and other half are 135.
GLuint lightsID;
float lights[LIGHT_COUNT * LIGHT_DATA_SIZE] = {0}; // 20800 floats
bool lightDirty[LIGHT_COUNT] = { [0 ... LIGHT_COUNT-1] = true };
// ----------------------------------------------------------------------------
// Event System states
int32_t maxEventCount_debug = 0;
double lastJournalWriteTime = 0;
uint32_t globalFrameNum = 0;
FILE* activeLogFile;
const char* manualLogName;
bool log_playback = false;
Event eventQueue[MAX_EVENTS_PER_FRAME]; // Queue for events to process this frame
Event eventJournal[EVENT_JOURNAL_BUFFER_SIZE]; // Journal buffer for event history to write into the log/demo file
int32_t eventJournalIndex;
int32_t eventIndex; // Event that made it to the counter.  Indices below this were
                // already executed and walked away from the counter.
int32_t eventQueueEnd; // End of the waiting line
const double time_step = 1.0 / 60.0; // 60fps
double last_time = 0.0;
double current_time = 0.0;
double cpuTime = 0.0;
bool journalFirstWrite = true;
// ----------------------------------------------------------------------------
// Audio
#define MAX_CHANNELS 64
// fluid_synth_t* midi_synth;
ma_engine audio_engine;
ma_sound mp3_sounds[2]; // For crossfading
ma_sound wav_sounds[MAX_CHANNELS];
int32_t wav_count = 0;

// Usage:
//play_mp3("./Audio/music/looped/track1.mp3",0.08f,0); // WORKED!
//play_wav("./Audio/cyborgs/yourlevelsareterrible.wav",0.1f); // WORKED!
// ----------------------------------------------------------------------------
// Networking
typedef enum {
    MODE_LISTEN_SERVER,    // Runs both server and client locally
    MODE_CLIENT            // Client only, connects to a server
} EngineMode;

EngineMode engine_mode = MODE_LISTEN_SERVER; // Default mode
char* server_address = "127.0.0.1"; // Default to localhost for listen server
int32_t server_port = 27015; // Default port

ENetHost* server_host = NULL;
ENetHost* client_host = NULL;
ENetPeer* server_peer = NULL; // Client's connection to server
// ----------------------------------------------------------------------------
// ============================================================================
// GLFW Callbacks
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F10 && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        if (log_playback) {
            log_playback = false;
            DualLog("Exited log playback manually.  Control returned\n");
        } else {
            EnqueueEvent_Simple(EV_QUIT);
        }
        return;
    }
    if (!log_playback) {
        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            EnqueueEvent_Int(EV_KEYDOWN, key);
        } else if (action == GLFW_RELEASE) {
            EnqueueEvent_Int(EV_KEYUP, key);
        }
    }
}

static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!log_playback && window_has_focus) {
        int32_t dx = (int32_t)(xpos - last_mouse_x);
        int32_t dy = (int32_t)(ypos - last_mouse_y);
        last_mouse_x = xpos;
        last_mouse_y = ypos;
        if (globalFrameNum > 1) EnqueueEvent_IntInt(EV_MOUSEMOVE, dx, dy);
    }
}
#pragma GCC diagnostic pop

static void window_focus_callback(GLFWwindow* window, int focused) {
    window_has_focus = focused != 0;
    if (window_has_focus) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}
// ============================================================================
// OpenGL / Rendering Helper Functions
void GenerateAndBindTexture(GLuint *id, GLenum internalFormat, int32_t width, int32_t height, GLenum format, GLenum type, GLenum target) {
    glGenTextures(1, id);
    glBindTexture(target, *id);
    glTexImage2D(target, 0, internalFormat, width, height, 0, format, type, NULL);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(target, 0);
    CHECK_GL_ERROR();
}

GLuint CompileShader(GLenum type, const char *source, const char *shaderName) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) { char infoLog[512]; glGetShaderInfoLog(shader, 512, NULL, infoLog); DualLogError("%s Compilation Failed: %s\n", shaderName, infoLog); exit(1); }
    return shader;
}

GLuint LinkProgram(GLuint *shaders, int32_t count, const char *programName) {
    GLuint program = glCreateProgram();
    for (int32_t i = 0; i < count; i++) glAttachShader(program, shaders[i]);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) { char infoLog[512]; glGetProgramInfoLog(program, 512, NULL, infoLog); DualLogError("%s Linking Failed: %s\n", programName, infoLog); exit(1); }

    for (int32_t i = 0; i < count; i++) glDeleteShader(shaders[i]);
    return program;
}

void CompileShaders(void) {
    GLuint vertShader, fragShader, computeShader;

    // Chunk Shader
    vertShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource, "Chunk Vertex Shader");
    fragShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderTraditional, "Chunk Fragment Shader");
    chunkShaderProgram = LinkProgram((GLuint[]){vertShader, fragShader}, 2, "Chunk Shader Program");
    
    // Shadowmaps Shader
    vertShader = CompileShader(GL_VERTEX_SHADER, shadowmapVertexShaderSource, "Shadowmaps Vertex Shader");
    fragShader = CompileShader(GL_FRAGMENT_SHADER, shadowmapFragmentShaderSource, "Shadowmaps Fragment Shader");
    shadowmapsShaderProgram = LinkProgram((GLuint[]){vertShader, fragShader}, 2, "Shadowmaps Shader Program");

    // Text Shader
    vertShader = CompileShader(GL_VERTEX_SHADER, textVertexShaderSource, "Text Vertex Shader");
    fragShader = CompileShader(GL_FRAGMENT_SHADER, textFragmentShaderSource, "Text Fragment Shader");
    textShaderProgram = LinkProgram((GLuint[]){vertShader, fragShader}, 2, "Text Shader Program");

    // Screen Space Reflections Compute Shader Program
    computeShader = CompileShader(GL_COMPUTE_SHADER, ssr_computeShader, "Screen Space Reflections Compute Shader");
    ssrShaderProgram = LinkProgram((GLuint[]){computeShader}, 1, "Screen Space Reflections Shader Program");

    // Image Blit Shader (For full screen image effects, rendering compute results, etc.)
    vertShader = CompileShader(GL_VERTEX_SHADER,   quadVertexShaderSource,   "Image Blit Vertex Shader");
    fragShader = CompileShader(GL_FRAGMENT_SHADER, quadFragmentShaderSource, "Image Blit Fragment Shader");
    imageBlitShaderProgram = LinkProgram((GLuint[]){vertShader, fragShader}, 2, "Image Blit Shader Program");

//     glGenBuffers(1, &blueNoiseBuffer);
//     glBindBuffer(GL_SHADER_STORAGE_BUFFER, blueNoiseBuffer);
//     glBufferData(GL_SHADER_STORAGE_BUFFER, 12288 * sizeof(float), blueNoise, GL_STATIC_DRAW);
//     glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, blueNoiseBuffer); // Use binding point 13
//     glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // Cache uniform locations after shader compile!
    viewProjLoc_chunk = glGetUniformLocation(chunkShaderProgram, "viewProjection");
    matrixLoc_chunk = glGetUniformLocation(chunkShaderProgram, "matrix");
    texIndexLoc_chunk = glGetUniformLocation(chunkShaderProgram, "texIndex");
    glowSpecIndexLoc_chunk = glGetUniformLocation(chunkShaderProgram, "glowSpecIndex");
    normInstanceIndexLoc_chunk = glGetUniformLocation(chunkShaderProgram, "normInstanceIndex");
    debugViewLoc_chunk = glGetUniformLocation(chunkShaderProgram, "debugView");
    debugValueLoc_chunk = glGetUniformLocation(chunkShaderProgram, "debugValue");
    screenWidthLoc_chunk = glGetUniformLocation(chunkShaderProgram, "screenWidth");
    screenHeightLoc_chunk = glGetUniformLocation(chunkShaderProgram, "screenHeight");
    worldMin_xLoc_chunk = glGetUniformLocation(chunkShaderProgram, "worldMin_x");
    worldMin_zLoc_chunk = glGetUniformLocation(chunkShaderProgram, "worldMin_z");
    camPosLoc_chunk = glGetUniformLocation(chunkShaderProgram, "camPos");
    fogColorRLoc_chunk = glGetUniformLocation(chunkShaderProgram, "fogColorR");
    fogColorGLoc_chunk = glGetUniformLocation(chunkShaderProgram, "fogColorG");
    fogColorBLoc_chunk = glGetUniformLocation(chunkShaderProgram, "fogColorB");
    
    modelMatrixLoc_shadowmaps = glGetUniformLocation(shadowmapsShaderProgram, "modelMatrix");
    viewProjMatrixLoc_shadowmaps = glGetUniformLocation(shadowmapsShaderProgram, "viewProjMatrix");
    texIndexLoc_shadowmaps = glGetUniformLocation(shadowmapsShaderProgram, "texIndex");
    glowSpecIndexLoc_shadowmaps = glGetUniformLocation(shadowmapsShaderProgram, "glowSpecIndex");
    normInstanceIndexLoc_shadowmaps = glGetUniformLocation(shadowmapsShaderProgram, "normInstanceIndex");

    screenWidthLoc_ssr = glGetUniformLocation(ssrShaderProgram, "screenWidth");
    screenHeightLoc_ssr = glGetUniformLocation(ssrShaderProgram, "screenHeight");
    viewProjectionLoc_ssr = glGetUniformLocation(ssrShaderProgram, "viewProjection");
    camPosLoc_ssr = glGetUniformLocation(ssrShaderProgram, "camPos");
    
    texLoc_quadblit = glGetUniformLocation(imageBlitShaderProgram, "tex");
    debugViewLoc_quadblit = glGetUniformLocation(imageBlitShaderProgram, "debugView");
    debugValueLoc_quadblit = glGetUniformLocation(imageBlitShaderProgram, "debugValue");
    screenWidthLoc_imageBlit = glGetUniformLocation(imageBlitShaderProgram, "screenWidth");
    screenHeightLoc_imageBlit = glGetUniformLocation(imageBlitShaderProgram, "screenHeight");
    
    projectionLoc_text = glGetUniformLocation(textShaderProgram, "projection");
    textColorLoc_text = glGetUniformLocation(textShaderProgram, "textColor");
    textTextureLoc_text = glGetUniformLocation(textShaderProgram, "textTexture");
    texelSizeLoc_text = glGetUniformLocation(textShaderProgram, "texelSize");
    CHECK_GL_ERROR();
}

void Screenshot() {
    struct stat st = {0};
    if (stat("Screenshots", &st) == -1) { // Check and make ./Screenshots/ folder if it doesn't exist yet.
        if (mkdir("Screenshots", 0755) != 0) { DualLogError("Failed to create Screenshots folder\n"); return; }
    }
    
    unsigned char* pixels = (unsigned char*)malloc(screen_width * screen_height * 4);
    glReadPixels(0, 0, screen_width, screen_height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    char timestamp[32];
    char filename[96];
    time_t now = time(NULL);
    struct tm *utc_time = localtime(&now);
    if (!utc_time) { DualLog("Failed to get current time for screenshot!\n"); free(pixels); return; }
    
    strftime(timestamp, sizeof(timestamp), "%d%b%Y_%H_%M_%S", utc_time);
    snprintf(filename, sizeof(filename), "Screenshots/%s_%s_x%.2f_y%.2f_z%.2f__time_%.1f.png", timestamp, VERSION_STRING, cam_x, cam_y, cam_z, get_time());
    int32_t success = stbi_write_png(filename, screen_width, screen_height, 4, pixels, screen_width * 4);
    if (!success) DualLog("Failed to save screenshot\n");
    else DualLog("Saved screenshot %s\n", filename);

    free(pixels);
}

// out = a * b
static inline void mul_mat4(float *out, const float *a, const float *b) {
    float result[16];
    for (int32_t col = 0; col < 4; ++col) {
        for (int32_t row = 0; row < 4; ++row) {
            result[col*4 + row] =
                a[0*4 + row] * b[col*4 + 0] +
                a[1*4 + row] * b[col*4 + 1] +
                a[2*4 + row] * b[col*4 + 2] +
                a[3*4 + row] * b[col*4 + 3];
        }
    }
    // copy back
    for (int32_t i = 0; i < 16; i++)
        out[i] = result[i];
}

// Invert an affine 4x4 matrix (last row = [0 0 0 1])
// out = inverse(m)
static inline void invertAffineMat4(float *out, const float *m) {
    // Extract rotation 3x3
    float r00 = m[0], r01 = m[1], r02 = m[2];
    float r10 = m[4], r11 = m[5], r12 = m[6];
    float r20 = m[8], r21 = m[9], r22 = m[10];

    // Transpose rotation
    out[0] = r00; out[1] = r10; out[2] = r20; out[3] = 0.0f;
    out[4] = r01; out[5] = r11; out[6] = r21; out[7] = 0.0f;
    out[8] = r02; out[9] = r12; out[10] = r22; out[11] = 0.0f;
    out[15] = 1.0f;

    // Invert translation
    float tx = m[12], ty = m[13], tz = m[14];
    out[12] = -(out[0]*tx + out[4]*ty + out[8]*tz);
    out[13] = -(out[1]*tx + out[5]*ty + out[9]*tz);
    out[14] = -(out[2]*tx + out[6]*ty + out[10]*tz);
}
// ============================================================================
void SetUpdatedMatrix(float *mat, float posx, float posy, float posz, Quaternion* quat, float sclx, float scly, float sclz) {
    float rot[16];
    quat_to_matrix(quat,rot);
    mat[0]  = rot[0] * -sclx; mat[1]  = rot[1] * -sclx; mat[2]  = rot[2] * -sclx; mat[3]  = 0.0f;
    mat[4]  = rot[4] * scly; mat[5]  = rot[5] * scly; mat[6]  = rot[6] * scly; mat[7]  = 0.0f;
    mat[8]  = rot[8] * sclz; mat[9]  = rot[9] * sclz; mat[10] = rot[10] * sclz; mat[11] = 0.0f;
    mat[12] = posx;          mat[13] = posy;          mat[14] = posz;          mat[15] = 1.0f;
}

void UpdateInstanceMatrix(int32_t i) {
    if (instances[i].modelIndex >= loadedModels) { dirtyInstances[i] = false; return; } // No model
    if (modelVertexCounts[instances[i].modelIndex] < 1) { dirtyInstances[i] = false; return; } // Empty model

    float mat[16]; // 4x4 matrix
    Quaternion quat = {instances[i].rotation.x, instances[i].rotation.y, instances[i].rotation.z, instances[i].rotation.w};
    SetUpdatedMatrix(mat, instances[i].position.x, instances[i].position.y, instances[i].position.z, &quat,instances[i].scale.x, instances[i].scale.y, instances[i].scale.z);
    memcpy(&modelMatrices[i * 16], mat, 16 * sizeof(float));
    dirtyInstances[i] = false;
}

void RenderLoadingProgress(int32_t offset, const char* format, ...) {
    glUseProgram(imageBlitShaderProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputImageID);
    glProgramUniform1i(imageBlitShaderProgram, texLoc_quadblit, 0);
    glBindVertexArray(quadVAO);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindTextureUnit(0, 0);
    glUseProgram(0);
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    RenderFormattedText(screen_width / 2 - offset, screen_height / 2 - 5, TEXT_WHITE, buffer);
    glEnable(GL_DEPTH_TEST);
    glfwSwapBuffers(window);
}
// ============================================================================
void UpdateScreenSize(void) {
    float* m;
    m = uiOrthoProjection;
    m[0] = 2.0f / (float)screen_width; m[1] =                           0.0f; m[2] =  0.0f; m[3] = 0.0f;
    m[4] =                       0.0f; m[5] = -2.0f / ((float)screen_height); m[6] =  0.0f; m[7] = 0.0f;
    m[8] =                       0.0f; m[9] =                           0.0f; m[10]= -1.0f; m[11]= 0.0f;
    m[12]=                      -1.0f; m[13]=                           1.0f; m[14]=  0.0f; m[15]= 1.0f;
    
    float aspect = (float)screen_width / (float)screen_height;
    float f = 1.0f / tan(cam_fov * M_PI / 360.0f);
    m = rasterPerspectiveProjection;
    m[0] = f / aspect; m[1] = 0.0f; m[2] =                                                  0.0f; m[3] =  0.0f;
    m[4] =       0.0f; m[5] =    f; m[6] =                                                  0.0f; m[7] =  0.0f;
    m[8] =       0.0f; m[9] = 0.0f; m[10]=      -(FAR_PLANE + NEAR_PLANE) / (FAR_PLANE - NEAR_PLANE); m[11]= -1.0f;
    m[12]=       0.0f; m[13]= 0.0f; m[14]= -2.0f * FAR_PLANE * NEAR_PLANE / (FAR_PLANE - NEAR_PLANE); m[15]=  0.0f;
    
    aspect = (float)SHADOW_MAP_SIZE / (float)SHADOW_MAP_SIZE;
    f = 1.0f / tan(SHADOWMAP_FOV * M_PI / 360.0f);
    m = shadowmapsPerspectiveProjection;
    m[0] = f / aspect; m[1] = 0.0f; m[2] =                                                  0.0f; m[3] =  0.0f;
    m[4] =       0.0f; m[5] =    f; m[6] =                                                  0.0f; m[7] =  0.0f;
    m[8] =       0.0f; m[9] = 0.0f; m[10]=      -(35.0 + NEAR_PLANE) / (35.0 - NEAR_PLANE); m[11]= -1.0f;
    m[12]=       0.0f; m[13]= 0.0f; m[14]= -2.0f * 35.0 * NEAR_PLANE / (35.0 - NEAR_PLANE); m[15]=  0.0f;
}

uint32_t* voxelLightListsRaw = NULL;
uint32_t* voxelLightListIndices = NULL;

typedef struct {
    uint16_t index; // Original index in lights array
    float distanceSquared; // Distance to camera squared
    float score; // Priority score (lower distance, higher intensity = higher priority)
} LightCandidate;

int32_t compareLightCandidates(const void* a, const void* b) {
    const LightCandidate* ca = (const LightCandidate*)a;
    const LightCandidate* cb = (const LightCandidate*)b;
    return (ca->score < cb->score) ? -1 : ((ca->score > cb->score) ? 1 : 0);
}

int32_t VoxelLists() {
    DualLog("Generating voxel lighting data...");
    double start_time = get_time();
    uint32_t* voxelLightListsRaw = malloc(VOXEL_COUNT * 4 * sizeof(uint32_t));
    uint32_t* voxelLightListIndices = malloc(VOXEL_COUNT * 2 * sizeof(uint32_t));
    memset(voxelLightListIndices, 0, VOXEL_COUNT * 2 * sizeof(uint32_t));
    const float startX = worldMin_x + (VOXEL_SIZE * 0.5f);
    const float startZ = worldMin_z + (VOXEL_SIZE * 0.5f);
    float rangeSquared[LIGHT_COUNT]; // Precompute light ranges
    for (int32_t i = 0; i < LIGHT_COUNT; ++i) {
        rangeSquared[i] = lights[(i * LIGHT_DATA_SIZE) + LIGHT_DATA_OFFSET_RANGE];
        rangeSquared[i] *= rangeSquared[i];
    }

    // Precompute total size
    uint32_t totalLightAssignments = 0;
    for (uint32_t lightIdx = 0; lightIdx < LIGHT_COUNT; ++lightIdx) {
        uint32_t litIdx = lightIdx * LIGHT_DATA_SIZE;
        float litX = lights[litIdx + LIGHT_DATA_OFFSET_POSX];
        float litZ = lights[litIdx + LIGHT_DATA_OFFSET_POSZ];
        float range = sqrtf(rangeSquared[lightIdx]);
        int32_t minCellX = (int32_t)floorf((litX - range - worldMin_x) / WORLDCELL_WIDTH_F);
        int32_t maxCellX = (int32_t)ceilf((litX + range - worldMin_x) / WORLDCELL_WIDTH_F);
        int32_t minCellZ = (int32_t)floorf((litZ - range - worldMin_z) / WORLDCELL_WIDTH_F);
        int32_t maxCellZ = (int32_t)ceilf((litZ + range - worldMin_z) / WORLDCELL_WIDTH_F);
        minCellX = minCellX > 0 ? minCellX : 0;
        maxCellX = 63 < maxCellX ? 63 : maxCellX;
        minCellZ = minCellZ > 0 ? minCellZ : 0;
        maxCellZ = 63 < maxCellZ ? 63 : maxCellZ;
        for (int32_t cellZ = minCellZ; cellZ <= maxCellZ; ++cellZ) {
            for (int32_t cellX = minCellX; cellX <= maxCellX; ++cellX) {
                uint32_t cellIndex = cellZ * 64 + cellX;
                for (uint32_t voxelZ = 0; voxelZ < 8; ++voxelZ) {
                    for (uint32_t voxelX = 0; voxelX < 8; ++voxelX) {
                        uint32_t voxelIndex = cellIndex * 64 + voxelZ * 8 + voxelX;
                        float posX = startX + (cellX * WORLDCELL_WIDTH_F) + (voxelX * VOXEL_SIZE);
                        float posZ = startZ + (cellZ * WORLDCELL_WIDTH_F) + (voxelZ * VOXEL_SIZE);
                        float distSqrd = squareDistance2D(posX, posZ, litX, litZ);
                        if (distSqrd < rangeSquared[lightIdx] && voxelLightListIndices[voxelIndex * 2 + 1] < MAX_LIGHTS_PER_VOXEL) {
                            voxelLightListIndices[voxelIndex * 2 + 1]++; // Increment light count
                            totalLightAssignments++;
                        }
                    }
                }
            }
        }
    }

    if (totalLightAssignments > VOXEL_COUNT * 4) { DualLogError("\nTotal light assignments (%u) exceed voxelLightListsRaw capacity (%u)\n", totalLightAssignments, VOXEL_COUNT * 4); return 1; }

    // Assign offsets and populate voxelLightListsRaw
    uint32_t head = 0;
    for (uint32_t idx = 0; idx < VOXEL_COUNT; ++idx) {
        if (voxelLightListIndices[idx * 2 + 1] > 0) {
            voxelLightListIndices[idx * 2] = head; // Set offset
            head += voxelLightListIndices[idx * 2 + 1]; // Advance head
        } else {
            voxelLightListIndices[idx * 2] = head; // Empty list points to current head
        }
    }

    // Assign light indices to voxelLightListsRaw
    uint32_t lightCounts[VOXEL_COUNT] = {0}; // Track current count for each voxel
    for (uint32_t lightIdx = 0; lightIdx < LIGHT_COUNT; ++lightIdx) {
        uint32_t litIdx = lightIdx * LIGHT_DATA_SIZE;
        float litX = lights[litIdx + LIGHT_DATA_OFFSET_POSX];
        float litZ = lights[litIdx + LIGHT_DATA_OFFSET_POSZ];
        float range = sqrtf(rangeSquared[lightIdx]);
        int32_t minCellX = (int32_t)floorf((litX - range - worldMin_x) / WORLDCELL_WIDTH_F);
        int32_t maxCellX = (int32_t)ceilf((litX + range - worldMin_x) / WORLDCELL_WIDTH_F);
        int32_t minCellZ = (int32_t)floorf((litZ - range - worldMin_z) / WORLDCELL_WIDTH_F);
        int32_t maxCellZ = (int32_t)ceilf((litZ + range - worldMin_z) / WORLDCELL_WIDTH_F);
        minCellX = minCellX > 0 ? minCellX : 0;
        maxCellX = 63 < maxCellX ? 63 : maxCellX;
        minCellZ = minCellZ > 0 ? minCellZ : 0;
        maxCellZ = 63 < maxCellZ ? 63 : maxCellZ;
        for (int32_t cellZ = minCellZ; cellZ <= maxCellZ; ++cellZ) {
            for (int32_t cellX = minCellX; cellX <= maxCellX; ++cellX) {
                uint32_t cellIndex = cellZ * 64 + cellX;
                for (uint32_t voxelZ = 0; voxelZ < 8; ++voxelZ) {
                    for (uint32_t voxelX = 0; voxelX < 8; ++voxelX) {
                        uint32_t voxelIndex = cellIndex * 64 + voxelZ * 8 + voxelX;
                        float posX = startX + (cellX * WORLDCELL_WIDTH_F) + (voxelX * VOXEL_SIZE);
                        float posZ = startZ + (cellZ * WORLDCELL_WIDTH_F) + (voxelZ * VOXEL_SIZE);
                        float distSqrd = squareDistance2D(posX, posZ, litX, litZ);

                        if (distSqrd < rangeSquared[lightIdx] && lightCounts[voxelIndex] < MAX_LIGHTS_PER_VOXEL) {
                            uint32_t offset = voxelLightListIndices[voxelIndex * 2];
                            voxelLightListsRaw[offset + lightCounts[voxelIndex]] = lightIdx;
                            lightCounts[voxelIndex]++;
                        }
                    }
                }
            }
        }
    }

    GLuint voxelLightListIndicesID;
    glGenBuffers(1, &voxelLightListIndicesID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, voxelLightListIndicesID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, VOXEL_COUNT * 2 * sizeof(uint32_t), voxelLightListIndices, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 26, voxelLightListIndicesID);
    free(voxelLightListIndices);

    GLuint voxelLightListsRawID;
    glGenBuffers(1, &voxelLightListsRawID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, voxelLightListsRawID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, head * sizeof(uint32_t), voxelLightListsRaw, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 27, voxelLightListsRawID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    free(voxelLightListsRaw);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightsID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, LIGHT_COUNT * LIGHT_DATA_SIZE * sizeof(float), lights, GL_STATIC_DRAW);
    
    for (uint16_t i = 0; i < loadedInstances; i++) UpdateInstanceMatrix(i);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, matricesBuffer);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, loadedInstances * 16 * sizeof(float), modelMatrices);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glFlush();
    CHECK_GL_ERROR();    
    malloc_trim(0);
    DualLog(" took %f seconds, total list size: %u\n", get_time() - start_time, head);
    return 0;
}

// Generates View Matrix4x4 for Geometry Rasterizer Pass from camera world position + orientation
void mat4_lookat_from(float* m, Quaternion* camRotation, float x, float y, float z) {
    float rotation[16];
    quat_to_matrix(camRotation, rotation);

    // Extract basis vectors (camera space axes)
    float right[3]   = { rotation[0], rotation[1], rotation[2] };   // X+ (right)
    float up[3]      = { rotation[4], rotation[5], rotation[6] };   // Y+ (up)
    float forward[3] = { rotation[8], rotation[9], rotation[10] };  // Z+ (forward)

    // View matrix: inverse rotation (transpose) and inverse translation
    m[0]  = right[0];   m[1]  = up[0];   m[2]  = -forward[0]; m[3]  = 0.0f;
    m[4]  = right[1];   m[5]  = up[1];   m[6]  = -forward[1]; m[7]  = 0.0f;
    m[8]  = right[2];   m[9]  = up[2];   m[10] = -forward[2]; m[11] = 0.0f;
    m[12] = -dot(right[0], right[1], right[2], x, y, z);   // -dot(right, eye)
    m[13] = -dot(up[0], up[1], up[2], x, y, z);      // -dot(up, eye)
    m[14] = dot(forward[0], forward[1], forward[2], x, y, z);  // dot(forward, eye)
    m[15] = 1.0f;
}

// Generates View Matrix4x4 for Geometry Rasterizer Pass from camera world position + orientation
void mat4_lookat(float* m) {
    mat4_lookat_from(m,&cam_rotation, cam_x, cam_y, cam_z);
}

Quaternion orientationQuaternion[6] = {
    {0.0f, 0.707106781f, 0.0f, 0.707106781f},  // +X: Right
    {0.0f, -0.707106781f, 0.0f, 0.707106781f}, // -X: Left
    {-0.707106781f, 0.0f, 0.0f, 0.707106781f}, // +Y: Up
    {0.707106781f, 0.0f, 0.0f, 0.707106781f},  // -Y: Down
    {0.0f, 0.0f, 0.0f, 1.0f},                  // +Z: Forward
    {0.0f, 1.0f, 0.0f, 0.0f}                   // -Z: Backward
};

void RenderShadowmap(uint16_t lightIdx) {
    uint32_t litIdx = lightIdx * LIGHT_DATA_SIZE;
    float lightPosX = lights[litIdx + LIGHT_DATA_OFFSET_POSX];
    float lightPosY = lights[litIdx + LIGHT_DATA_OFFSET_POSY];
    float lightPosZ = lights[litIdx + LIGHT_DATA_OFFSET_POSZ];
    float lightRadius = lights[litIdx + LIGHT_DATA_OFFSET_RANGE];
    float effectiveRadius = fmin(lightRadius, 15.36f);
    GLint lightPosLoc = glGetUniformLocation(shadowmapsShaderProgram, "lightPos");
    GLint ssbo_indexBaseLoc = glGetUniformLocation(shadowmapsShaderProgram, "ssbo_indexBase");
    uint16_t nearMeshes[loadedInstances];
    uint16_t nearbyMeshCount = 0;
    for (uint16_t j = 0; j < loadedInstances; j++) {
        if (instances[j].modelIndex >= loadedModels) continue;
        if (modelVertexCounts[instances[j].modelIndex] < 1) continue;
        if (IsDynamicObject(instances[j].index)) continue;
        
        float radius = modelBounds[(instances[j].modelIndex * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_RADIUS];
        float distToLightSqrd = squareDistance3D(instances[j].position.x, instances[j].position.y, instances[j].position.z, lightPosX, lightPosY, lightPosZ);
        if (distToLightSqrd > (effectiveRadius + radius) * (effectiveRadius + radius)) continue;
        
        nearMeshes[nearbyMeshCount] = j;
        nearbyMeshCount++;
    }

    for (uint8_t face = 0; face < 6; face++) {
        float lightView[16];
        float lightViewProj[16];
        mat4_lookat_from(lightView, &orientationQuaternion[face], lightPosX, lightPosY, lightPosZ);
        mul_mat4(lightViewProj, shadowmapsPerspectiveProjection, lightView);
        glUniform3f(lightPosLoc, lightPosX, lightPosY, lightPosZ);
        glUniform1i(ssbo_indexBaseLoc, lightIdx * 6 * SHADOW_MAP_SIZE * SHADOW_MAP_SIZE + face * SHADOW_MAP_SIZE * SHADOW_MAP_SIZE); // 128
        glUniformMatrix4fv(viewProjMatrixLoc_shadowmaps, 1, GL_FALSE, lightViewProj);
        for (uint16_t j = 0; j < nearbyMeshCount; ++j) {
            int i = nearMeshes[j];
            if (instances[i].modelIndex >= loadedModels) continue;
            if (modelVertexCounts[instances[i].modelIndex] < 1) continue; // Empty model

            if (i >= startOfDoubleSidedInstances) glDisable(GL_CULL_FACE);
            if (i >= startOfTransparentInstances) {
                glDisable(GL_CULL_FACE);
                glEnable(GL_BLEND); // Enable blending for transparent instances
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Additive blending: src * srcAlpha + dst
                glDepthMask(GL_FALSE); // Disable depth writes for transparent instances
            }
            int32_t modelType = instanceIsLODArray[i] && instances[i].lodIndex < loadedModels ? instances[i].lodIndex : instances[i].modelIndex;
            glUniformMatrix4fv(modelMatrixLoc_shadowmaps, 1, GL_FALSE, &modelMatrices[i * 16]);
            glBindVertexBuffer(0, vbos[modelType], 0, VERTEX_ATTRIBUTES_COUNT * sizeof(float));
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tbos[modelType]);
            glDrawElements(GL_TRIANGLES, modelTriangleCounts[modelType] * 3, GL_UNSIGNED_INT, 0);
            drawCallsRenderedThisFrame++;
            verticesRenderedThisFrame += modelTriangleCounts[modelType] * 3;
        }
    }

    glFlush();

    staticLightCount++;
}

// Renders all static shadowmaps at level load
void RenderShadowmaps(void) {
    if (settings_Shadows < 1u) return;

    double start_time = get_time();
    DualLog("Rendering shadowmaps...");
    DebugRAM("Start of RenderShadowmaps");
    glGenTextures(1, &shadowCubeMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, shadowCubeMap);
    for (int face = 0; face < 6; face++) glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_R8, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    glGenFramebuffers(1, &shadowFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, shadowCubeMap, 0);
    glDrawBuffer(0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) { GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER); DualLogError("\nShadow FBO incomplete, status: 0x%x\n", status); return; }
   
    // SSBO for shadow map data
    glGenBuffers(1, &shadowMapSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shadowMapSSBO);
    uint32_t shadowmapPixelCount = SHADOW_MAP_SIZE * SHADOW_MAP_SIZE * 6u;
    uint32_t depthMapBufferSize = (uint32_t)(loadedLights) * shadowmapPixelCount * sizeof(float);
    glBufferData(GL_SHADER_STORAGE_BUFFER, depthMapBufferSize, NULL, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, shadowMapSSBO);
    
    // Render static lights once
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
    glUseProgram(shadowmapsShaderProgram);
    glProgramUniform1i(shadowmapsShaderProgram, glGetUniformLocation(shadowmapsShaderProgram, "shadowmapSize"), (int32_t)(SHADOW_MAP_SIZE));
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glBindVertexArray(vao_chunk);
    for (uint16_t i = 0; i < loadedLights; ++i) RenderShadowmap(i);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, screen_width, screen_height);
    glEnable(GL_CULL_FACE);
    malloc_trim(0);
    shadowMapsRendered = true;
    DualLog(" took %f seconds to render %d static shadow maps\n", get_time() - start_time, staticLightCount);
    DebugRAM("After rendering all shadowmaps");
}

void RenderDynamicShadowmaps(void) {}
// ============================================================================
// Audio
// void InitializeAudio(const char* soundfont_path) {
int32_t InitializeAudio() {
    ma_result result;
    ma_engine_config engine_config = ma_engine_config_init();
    engine_config.channels = 2; // Stereo output, adjust if needed

    result = ma_engine_init(&engine_config, &audio_engine);
    if (result != MA_SUCCESS) {
        DualLog("ERROR: Failed to initialize miniaudio engine: %d\n", result);
        return 1;
    }
    
//     fluid_settings_t* settings = new_fluid_settings();
//     midi_synth = new_fluid_synth(settings);
//     fluid_synth_sfopen(midi_synth, soundfont_path); // e.g., "./SoundFonts/FluidR3_GM.sf2"
    return 0;
}

// Temporarily disabled midi support until wav+mp3 is working.
// void play_midi(const char* midi_path) {
//     // Convert XMI to MIDI if needed
//     void* midi_data; size_t midi_size;
//     if (strstr(midi_path, ".xmi")) {
//         xmi2midi(midi_path, &midi_data, &midi_size);
//     } else {
//         // Load MIDI directly
//         FILE* f = fopen(midi_path, "rb");
//         fseek(f, 0, SEEK_END);
//         midi_size = ftell(f);
//         rewind(f);
//         midi_data = malloc(midi_size);
//         fread(midi_data, 1, midi_size, f);
//         fclose(f);
//     }
//     // Render MIDI to PCM, feed to miniaudio
//     short pcm[44100 * 2]; // Stereo, 1 second
//     fluid_synth_write_s16(midi_synth, 44100, pcm, 0, 2, pcm, 1, 2);
//     ma_sound_init_from_data_source(&audio_engine, pcm, sizeof(pcm), NULL, NULL);
//     free(midi_data);
// }

void play_mp3(const char* path, float volume, int32_t fade_in_ms) {
    static int32_t current_sound = 0;
    ma_sound_uninit(&mp3_sounds[current_sound]);
    ma_result result = ma_sound_init_from_file(&audio_engine, path, MA_SOUND_FLAG_STREAM, NULL, NULL, &mp3_sounds[current_sound]);
    if (result != MA_SUCCESS) { DualLog("ERROR: Failed to load MP3 %s: %d\n", path, result);  return; }
    
    ma_sound_set_fade_in_milliseconds(&mp3_sounds[current_sound], 0.0f, volume, fade_in_ms);
    ma_sound_start(&mp3_sounds[current_sound]);
    current_sound = 1 - current_sound; // Toggle for crossfade
}

void play_wav(const char* path, float volume) {
    // Try to find a free slot (either unused or finished)
    int32_t slot = -1;
    for (int32_t i = 0; i < wav_count; i++) {
        if (!ma_sound_is_playing(&wav_sounds[i]) && ma_sound_at_end(&wav_sounds[i])) {
            ma_sound_uninit(&wav_sounds[i]);
            slot = i;
            break;
        }
    }
    
    // If no free slot, use a new one if available
    if (slot == -1 && wav_count < MAX_CHANNELS) slot = wav_count++;
    if (slot == -1) { DualLog("WARNING: Max WAV channels (%d) reached\n", MAX_CHANNELS); return; }

    ma_result result = ma_sound_init_from_file(&audio_engine, path, 0, NULL, NULL, &wav_sounds[slot]);
    if (result != MA_SUCCESS) {
        DualLog("ERROR: Failed to load WAV %s: %d\n", path, result);
        if (slot == wav_count - 1) wav_count--; // Revert count if init fails
        return;
    }
    
    ma_sound_set_volume(&wav_sounds[slot], volume);
    ma_sound_start(&wav_sounds[slot]);
}
// ============================================================================
void InitializeEnvironment(void) {
    double init_start_time = get_time();
    DebugRAM("InitializeEnvironment start");    
    if (!glfwInit()) { DualLogError("GLFW initialization failed\n"); exit(1); }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(screen_width, screen_height, "Voxen, the OpenGL Voxel Lit Engine", NULL, NULL);
    if (!window) { DualLogError("glfwCreateWindow failed\n"); glfwTerminate(); exit(1); }
    
    glfwMakeContextCurrent(window);
    UpdateScreenSize();
    DebugRAM("window init");
    int monitor_count = 0;
    /*GLFWmonitor** monitors = */glfwGetMonitors(&monitor_count);
    if (monitor_count > 0) {
        GLFWmonitor* target_monitor = glfwGetPrimaryMonitor();  // Use primary; or monitors[1] for second monitor, etc.
        // TODO: Make configurable, e.g., via argc/argv: int target_idx = 0; /* parse from args */; target_monitor = monitors[target_idx];
        if (target_monitor) {
            const GLFWvidmode* mode = glfwGetVideoMode(target_monitor);
            int mx, my;
            glfwGetMonitorPos(target_monitor, &mx, &my);
            // Center the window on the monitor (windowed mode)
            int xpos = mx + (mode->width - screen_width) / 2;
            int ypos = my + (mode->height - screen_height) / 2;
            glfwSetWindowPos(window, xpos, ypos);
            DualLog("Window positioned (windowed, centered) on monitor: %s (primary) at %d,%d\n", glfwGetMonitorName(target_monitor), xpos, ypos);
        }
    }
    
    stbi_flip_vertically_on_write(1);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glewExperimental = GL_TRUE; // Enable modern OpenGL support
    if (glewInit() != GLEW_OK) { DualLog("GLEW initialization failed\n"); exit(1); }

    const GLubyte* version = glGetString(GL_VERSION);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    DualLog("OpenGL Version: %s\n", version ? (const char*)version : "unknown");
    DualLog("Renderer: %s\n", renderer ? (const char*)renderer : "unknown");

    int32_t vsync_enable = 0;//1; // Set to 0 for false.
    glfwSwapInterval(vsync_enable);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetWindowFocusCallback(window, window_focus_callback);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glMinSampleShading(0.0f);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_CULL_FACE); // Enable backface culling
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW); // Set triangle sorting order (GL_CW vs GL_CCW)
    glViewport(0, 0, screen_width, screen_height);
    CompileShaders();
    glUseProgram(imageBlitShaderProgram);
    glUniform1ui(screenWidthLoc_imageBlit, screen_width);
    glUniform1ui(screenHeightLoc_imageBlit, screen_height);

    glUseProgram(chunkShaderProgram);
    glUniform1ui(screenWidthLoc_chunk, screen_width);
    glUniform1ui(screenHeightLoc_chunk, screen_height);
    glProgramUniform1f(chunkShaderProgram, glGetUniformLocation(chunkShaderProgram, "shadowmapSize"), (float)(SHADOW_MAP_SIZE));

    glUseProgram(ssrShaderProgram);
    glUniform1ui(screenWidthLoc_ssr, screen_width / SSR_RES);
    glUniform1ui(screenHeightLoc_ssr, screen_height / SSR_RES);
    glUseProgram(0);
        
    // Setup full screen quad for image blit for post processing effects like lighting.
    float vertices[] = {
         1.0f, -1.0f, 1.0f, 0.0f, // Bottom-right
         1.0f,  1.0f, 1.0f, 1.0f, // Top-right
        -1.0f,  1.0f, 0.0f, 1.0f, // Top-left
        -1.0f, -1.0f, 0.0f, 0.0f  // Bottom-left
    };
    
    glCreateBuffers(1, &quadVBO);
    glNamedBufferData(quadVBO, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glCreateVertexArrays(1, &quadVAO);
    glEnableVertexArrayAttrib(quadVAO, 0);
    glEnableVertexArrayAttrib(quadVAO, 1);
    glVertexArrayAttribFormat(quadVAO, 0, 2, GL_FLOAT, GL_FALSE, 0); // DSA: Set position format
    glVertexArrayAttribFormat(quadVAO, 1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float)); // DSA: Set texcoord format
    glVertexArrayVertexBuffer(quadVAO, 0, quadVBO, 0, 4 * sizeof(float)); // DSA: Link VBO to VAO
    glVertexArrayAttribBinding(quadVAO, 0, 0); // DSA: Bind position attribute to binding index 0
    glVertexArrayAttribBinding(quadVAO, 1, 0); // DSA: Bind texcoord attribute to binding index 0
    
    // VAO for Global Vertex Definition
    glGenVertexArrays(1, &vao_chunk);
    glBindVertexArray(vao_chunk);
    glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0); // Position (vec3)
    glVertexAttribFormat(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float)); // Normal (vec3)
    glVertexAttribFormat(2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float)); // Tex Coord (vec2)
    for (uint8_t i = 0; i < 3; i++) { glVertexAttribBinding(i, 0); glEnableVertexAttribArray(i); }
    glBindVertexArray(0);
    DebugRAM("after vao chunk bind");

    // Create Framebuffer
    // First pass gbuffer images
    GenerateAndBindTexture(&inputImageID,             GL_RGBA8, screen_width, screen_height,            GL_RGBA, GL_UNSIGNED_BYTE, GL_TEXTURE_2D); // Lit Raster
    GenerateAndBindTexture(&inputWorldPosID,        GL_RGBA32F, screen_width, screen_height,            GL_RGBA,         GL_FLOAT, GL_TEXTURE_2D); // Raster World Positions
    GenerateAndBindTexture(&inputDepthID, GL_DEPTH_COMPONENT24, screen_width, screen_height, GL_DEPTH_COMPONENT,         GL_FLOAT, GL_TEXTURE_2D); // Raster Depth
    
    glGenTextures(1, &outputImageID);
    glBindTexture(GL_TEXTURE_2D, outputImageID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,  screen_width / SSR_RES,  screen_height / SSR_RES, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) DualLogError("Failed to create texture SSR: OpenGL error %d\n", error);
    
    glGenFramebuffers(1, &gBufferFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, inputImageID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, inputWorldPosID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, inputDepthID, 0);
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, drawBuffers);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        switch (status) {
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: DualLogError("Framebuffer incomplete: Attachment issue\n"); break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: DualLogError("Framebuffer incomplete: Missing attachment\n"); break;
            case GL_FRAMEBUFFER_UNSUPPORTED: DualLogError("Framebuffer incomplete: Unsupported configuration\n"); break;
            default: DualLogError("Framebuffer incomplete: Error code %d\n", status);
        }
    }
    
    glBindImageTexture(0, inputImageID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8); // Main Rendered Color
    glBindImageTexture(1, inputWorldPosID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    //                 3 = depth
    glBindImageTexture(4, outputImageID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8); // SSR result
    glActiveTexture(GL_TEXTURE3); // Match binding = 3 in shader
    glBindTexture(GL_TEXTURE_2D, inputDepthID);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, outputImageID);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    DebugRAM("setup gbuffer end");
    
    // Text Initialization
    InitFontAtlasses();
    DebugRAM("stb TTF init");
    glCreateBuffers(1, &textVBO);
    glCreateVertexArrays(1, &textVAO);    
    glEnableVertexArrayAttrib(textVAO, 0);
    glEnableVertexArrayAttrib(textVAO, 1);
    glVertexArrayAttribFormat(textVAO, 0, 2, GL_FLOAT, GL_FALSE, 0);                  // pos (x,y) 4 floats per vertex, stride = 4*sizeof(float)
    glVertexArrayAttribFormat(textVAO, 1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float));  // uv (s,t)
    glVertexArrayVertexBuffer(textVAO, 0, textVBO, 0, 4 * sizeof(float));
    glVertexArrayAttribBinding(textVAO, 0, 0);
    glVertexArrayAttribBinding(textVAO, 1, 0);
    
    DebugRAM("text init");

    // Lights buffer
    glGenBuffers(1, &lightsID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightsID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, LIGHT_COUNT * LIGHT_DATA_SIZE * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 19, lightsID);

    // Input
    Input_MouselookApply();
    
    // Network
    if (enet_initialize() != 0) { DualLogError("ENet initialization failed\n"); exit(1); }
    ENetAddress address;
    enet_address_set_host(&address, server_address);
    address.port = server_port;
    client_host = enet_host_create(NULL, 1, 2, 0, 0);
    if (!client_host) { DualLogError("Failed to create ENet client host\n"); exit(1); }

    server_peer = enet_host_connect(client_host, &address, 2, 0);
    if (!server_peer) { DualLogError("Failed to connect to server\n"); exit(1); }
    
    // Audio
    InitializeAudio();
    DebugRAM("audio init");
    RenderLoadingProgress(50,"Loading...");

    // Load Game/Mod Definition
    const char* filename = "./Data/gamedata.txt";
    DualLog("Loading game definition from %s...",filename);    
    Entity entry;
    init_data_entry(&entry);
    FILE *gamedatfile = fopen(filename, "r");
    if (!gamedatfile) { DualLogError("\nCannot open %s\n", filename); DualLogError("Could not parse %s!\n", filename); exit(1); }
    
    uint32_t lineNum = 0;
    bool is_eof;
    while (!feof(gamedatfile)) {
        char token[1024];
        bool is_comment, is_newline;
        if (!read_token(gamedatfile, token, sizeof(token), ':', &is_comment, &is_eof, &is_newline, &lineNum)) {
            if (is_comment || is_newline) {
                if (is_newline) lineNum += 1;
                continue;
            }
        }
        
        char key[256];
        strncpy(key, token, sizeof(key) - 1);
        key[sizeof(key) - 1] = '\0';
        if (!read_token(gamedatfile, token, sizeof(token), '\n', &is_comment, &is_eof, &is_newline, &lineNum)) continue;
        
        process_key_value(&entry, key, token, key, lineNum);
        lineNum += 1;
    }
    
    fclose(gamedatfile);
    if (strcmp(global_modname, "Citadel") == 0) global_modIsCitadel = true;;
    currentLevel = startLevel;
    DualLog(" loaded Game Definition for %s:: num levels: %d, start level: %d\n",global_modname,numLevels,startLevel);
    RenderLoadingProgress(100,"Loading textures...");
    DualLog("Window and GL Init took %f seconds\n", get_time() - init_start_time);
    LoadTextures();
    RenderLoadingProgress(100,"Loading models...");
    LoadModels();
    RenderLoadingProgress(100,"Loading entities...");
    LoadEntities(); // Must be after models and textures else entity types can't be validated.
    RenderLoadingProgress(100,"Loading level data...");
    renderableCount = 0;
    loadedInstances = 3; // 0 == NULL, 1 == Player1, 2 == Player2
    LoadLevelGeometry(currentLevel); // Must be after entities!
    RenderLoadingProgress(110,"Loading lighting data...");
    LoadLevelLights(currentLevel);
    RenderLoadingProgress(120,"Loading dynamic object data...");
    LoadLevelDynamicObjects(currentLevel);
    SortInstances(); // All instances loaded, sort them for render order: opaques, doublesideds, transparents.  REORDERS instances[] INDICES!!  CAREFUL!!
    RenderLoadingProgress(110,"Loading cull system...");
    CullInit(); // Must be after level! MUST BE AFTER SortInstances!!
    RenderLoadingProgress(120,"Loading voxel lighting data...");
    glClearColor(0.0f, 0.0f, 0.0f, 0.2f);
    VoxelLists();
    DebugRAM("InitializeEnvironment end");
}

// All core engine operations run through the EventExecute as an Event processed
// by the unified event system in the order it was enqueued.
int32_t EventExecute(Event* event) {
    if (event->type == EV_NULL) return 0;

    switch(event->type) {
        case EV_KEYDOWN: return Input_KeyDown(event->payload1i);
        case EV_KEYUP: return Input_KeyUp(event->payload1i);
        case EV_MOUSEMOVE: return Input_MouseMove(event->payload1i,event->payload2i);
        case EV_PHYSICS_TICK: return Physics();
        case EV_PARTICLE_TICK: return ParticleSystemStep();
        case EV_QUIT: return 1; break;
    }

    DualLogError("Unknown event %d\n",event->type);
    return 99;
}

bool IsSphereInFOVCone(float inst_x, float inst_y, float inst_z) {
    // Vector from camera to instance
    float to_inst_x = inst_x - cam_x;
    float to_inst_y = inst_y - cam_y;
    float to_inst_z = inst_z - cam_z;
    float dist_sq = to_inst_x * to_inst_x + to_inst_y * to_inst_y + to_inst_z * to_inst_z;
    if (dist_sq < 13.107200002f) return true; // ((sqrt(2) * 2.56f)^2)^2

    // Precompute FOV constants (assuming cam_fov is constant per frame)
    static float cos_half_fov = 0.0f;
    static float last_cam_fov = -1.0f;
    if (cam_fov != last_cam_fov) {
        float fovAdjusted = cam_fov * 2.5f;
        float half_fov_rad = fovAdjusted * 0.5f * (M_PI / 180.0f); // deg2rad
        cos_half_fov = cosf(half_fov_rad);
        last_cam_fov = cam_fov;
    }

    // Compute dot product without normalization
    float dot = cam_forwardx * to_inst_x + cam_forwardy * to_inst_y + cam_forwardz * to_inst_z;
    float dist = sqrtf(dist_sq); // Only compute sqrt once
    float dot_normalized = dot / dist; // Normalize dot product
    if (dot_normalized >= cos_half_fov) return true; // Center is within FOV cone
    return false; // Outside FOV cone
}

int32_t EnqueueEvent(uint8_t type, int32_t payload1i, int32_t payload2i, float payload1f, float payload2f) {
    if (eventQueueEnd >= MAX_EVENTS_PER_FRAME) { DualLogError("Queue buffer filled!\n"); return 1; }

    //DualLog("Enqueued event type %d, at index %d\n",type,eventQueueEnd);
    eventQueue[eventQueueEnd].frameNum = globalFrameNum;
    eventQueue[eventQueueEnd].type = type;
    eventQueue[eventQueueEnd].timestamp = 0;
    eventQueue[eventQueueEnd].payload1i = payload1i;
    eventQueue[eventQueueEnd].payload2i = payload2i;
    eventQueue[eventQueueEnd].payload1f = payload1f;
    eventQueue[eventQueueEnd].payload2f = payload2f;
    eventQueueEnd++;
    return 0;
}

int32_t EnqueueEvent_IntInt(uint8_t type, int32_t payload1i, int32_t payload2i) {
    return EnqueueEvent(type,payload1i,payload2i,0.0f,0.0f);
}

int32_t EnqueueEvent_Int(uint8_t type, int32_t payload1i) {
    return EnqueueEvent(type,payload1i,0u,0.0f,0.0f);
}

int32_t EnqueueEvent_FloatFloat(uint8_t type, float payload1f, float payload2f) {
    return EnqueueEvent(type,0u,0u,payload1f,payload2f);
}

int32_t EnqueueEvent_Float(uint8_t type, float payload1f) {
    return EnqueueEvent(type,0u,0u,payload1f,0.0f);
}

// Enqueues an event with type only and no payload values.
int32_t EnqueueEvent_Simple(uint8_t type) {
    return EnqueueEvent(type,0u,0u,0.0f,0.0f);
}

// Intended to be called after each buffered write to the logfile in .dem
// format which is custom but similar concept to Quake 1 demos.
void clear_ev_journal(void) {
    //  Events will be buffer written until EV_NULL is seen so clear to EV_NULL.
    for (int32_t i=0;i<EVENT_JOURNAL_BUFFER_SIZE;i++) {
        eventJournal[i].type = EV_NULL;
        eventJournal[i].frameNum = 0;
        eventJournal[i].timestamp = 0.0;
        eventJournal[i].deltaTime_ns = 0.0;
    }

    eventJournalIndex = 0; // Restart at the beginning.
}

void JournalLog(void) {
    FILE* fp;
    if (journalFirstWrite) {
        fp = fopen("./voxen.dem", "wb"); // Overwrite for first write.
        journalFirstWrite = false;
    } else fp = fopen("./voxen.dem", "ab"); // Append

    if (!fp) {
        DualLogError("Failed to open voxen.dem for journal log\n");
        return;
    }

    // Write all valid events in eventJournal
    for (int32_t i = 0; i < eventJournalIndex; i++) {
        if (eventJournal[i].type != EV_NULL) {
            fwrite(&eventJournal[i], sizeof(Event), 1, fp);
        }
    }

    fflush(fp);
    fclose(fp);
}

bool IsPlayableEventType(uint8_t type) {
    if (type == EV_KEYDOWN || type == EV_KEYUP) return true;
    return type != EV_NULL;
}

// Makes use of global activeLogFile handle to read through log and enqueue events with matching frameNum to globalFrameNum
int32_t ReadActiveLog() {
    static bool eof_reached = false; // Track EOF across calls
    Event event;
    int32_t events_processed = 0;
    if (eof_reached) return 2; // Indicate EOF was previously reached

    DualLog("------ ReadActiveLog start for frame %d ------\n",globalFrameNum);
    while (events_processed < MAX_EVENTS_PER_FRAME) {
        size_t read_count = fread(&event, sizeof(Event), 1, activeLogFile);
        if (read_count != 1) {
            if (feof(activeLogFile)) {
                eof_reached = true;
                log_playback = false; // Finished enqueuing last frame, main will finish processing the queue and return input to user.
                return events_processed > 0 ? 0 : 2; // 0 if events were processed, 2 if EOF and no events
            }

            if (ferror(activeLogFile)) { DualLogError("Could not read log file\n"); return -1; }
        }

        if (!IsPlayableEventType(event.type)) continue; // Skip unplayable events

        if (event.frameNum == globalFrameNum) {
            // Enqueue events matching the current frame
            EnqueueEvent(event.type, event.payload1i, event.payload2i, event.payload1f, event.payload2f);
            events_processed++;
            DualLog("Enqueued event %d from log for frame %d\n",event.type,event.frameNum);
        } else if (event.frameNum > globalFrameNum) {
            // Event is for a future frame; seek back and stop processing
            fseek(activeLogFile, -(long)sizeof(Event), SEEK_CUR);
            DualLog("Readback of %d events for this frame %d from log\n",events_processed,globalFrameNum);
            return events_processed > 0 ? 0 : 1; // 0 if events processed, 1 if no matching events
        } // If event.frameNum < globalFrameNum, skip it (past event)
    }

    DualLog("End of log. Readback of %d events for this frame %d from log\n",events_processed,globalFrameNum);
    return events_processed > 0 ? 0 : 1; // 0 if events processed, 1 if limit reached with no matching events
}

// Convert the binary .dem file into human readable text
void JournalDump(const char* dem_file) {
    FILE* fpR = fopen(dem_file, "rb");
    if (!fpR) { DualLogError("Failed to open .dem file\n"); exit(1); }

    FILE* fpW = fopen("./log_dump.txt", "wb");
    if (!fpW) { DualLogError("Failed to open voxen.dem\n"); exit(1); }

    Event event;
    while (fread(&event, sizeof(Event), 1, fpR) == 1) {
        fprintf(fpW,"frameNum: %d, ",event.frameNum);
        fprintf(fpW,"event type: %d, ",event.type);
        fprintf(fpW,"timestamp: %f, ", event.timestamp);
        fprintf(fpW,"delta time: %f, ", event.deltaTime_ns);
        fprintf(fpW,"payload1i: %d, ", event.payload1i);
        fprintf(fpW,"payload2i: %d, ", event.payload2i);
        fprintf(fpW,"payload1f: %f, ", event.payload1f);
        fprintf(fpW,"payload2f: %f\n", event.payload2f); // \n flushes write to file
    }

    fclose(fpW);
    fclose(fpR);
}

// Queue was processed for the frame, clear it so next frame starts fresh.
void clear_ev_queue(void) {
    //  Events will be buffer written until EV_NULL is seen so clear to EV_NULL.
    for (int32_t i=0;i<MAX_EVENTS_PER_FRAME;i++) {
        eventQueue[i].type = EV_NULL;
        eventQueue[i].frameNum = 0;
        eventQueue[i].timestamp = 0.0;
        eventQueue[i].deltaTime_ns = 0.0;
    }

    eventIndex = 0;
    eventQueueEnd = 0;
}

double get_time(void) {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1) {
        DualLogError("clock_gettime failed\n");
        return 0.0;
    }

    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9; // Full time in seconds
}

// Process the entire event queue. Events might add more new events to the queue.
// Intended to be called once per loop iteration by the main loop.
int32_t EventQueueProcess(void) {
    int32_t status = 0;
    double timestamp = 0.0;
    int32_t eventCount = 0;
    for (int32_t i=0;i<MAX_EVENTS_PER_FRAME;i++) {
        if (eventQueue[i].type != EV_NULL) {
            eventCount++;
        }
    }

    if (eventCount > maxEventCount_debug) maxEventCount_debug = eventCount;
    eventIndex = 0;
    while (eventIndex < MAX_EVENTS_PER_FRAME) {
        if (eventQueue[eventIndex].type == EV_NULL) break; // End of queue

        eventQueue[eventIndex].frameNum = globalFrameNum;
        timestamp = current_time;
        eventQueue[eventIndex].timestamp = timestamp;
        eventQueue[eventIndex].deltaTime_ns = timestamp - eventJournal[eventJournalIndex].timestamp; // Twould be zero if eventJournalIndex == 0, no need to try to assign it as something else; avoiding branch.

        // Journal buffer entry of this event.  Still written to during playback for time deltas but never logged to .dem
        eventJournalIndex++; // Increment now to then write event into the journal.
        if (eventJournalIndex >= EVENT_JOURNAL_BUFFER_SIZE || (timestamp - lastJournalWriteTime) > 5.0) {
            if (!log_playback) {
                JournalLog();
                lastJournalWriteTime = get_time();
            }

            clear_ev_journal(); // Also sets eventJournalIndex to 0.
        }

        eventJournal[eventJournalIndex].frameNum = eventQueue[eventIndex].frameNum;
        eventJournal[eventJournalIndex].type = eventQueue[eventIndex].type;
        eventJournal[eventJournalIndex].timestamp = eventQueue[eventIndex].timestamp;
        eventJournal[eventJournalIndex].deltaTime_ns = eventQueue[eventIndex].deltaTime_ns;
        eventJournal[eventJournalIndex].payload1i = eventQueue[eventIndex].payload1i;
        eventJournal[eventJournalIndex].payload2i = eventQueue[eventIndex].payload2i;
        eventJournal[eventJournalIndex].payload1f = eventQueue[eventIndex].payload1f;
        eventJournal[eventJournalIndex].payload2f = eventQueue[eventIndex].payload2f;

        // Execute event after journal buffer entry such that we can dump the
        // journal buffer on error and last entry will be the problematic event.
        status = EventExecute(&eventQueue[eventIndex]);
        if (status) {
            if (status != 1) DualLog("EventExecute returned nonzero status: %d\n", status);
            return status;
        }

        eventIndex++;
    }

    clear_ev_queue();
    return 0;
}

int32_t EventInit(void) {
    journalFirstWrite = true;

    // Initialize the eventQueue as empty
    clear_ev_queue();
    clear_ev_journal(); // Initialize the event journal as empty.
    eventQueue[eventIndex].type = EV_NULL;
    eventQueue[eventIndex].timestamp = get_time();
    eventQueue[eventIndex].deltaTime_ns = 0.0;
    return 0;
}

typedef struct {
    uint16_t index;
    float depth;
} DepthSort;

int32_t compareDepthSort(const void* a, const void* b) {
    const DepthSort* da = (const DepthSort*)a;
    const DepthSort* db = (const DepthSort*)b;
    return da->depth > db->depth ? -1 : (da->depth < db->depth ? 1 : 0);
}

int32_t compareDepthSortInverted(const void* a, const void* b) {
    const DepthSort* da = (const DepthSort*)a;
    const DepthSort* db = (const DepthSort*)b;
    return da->depth > db->depth ? 1 : (da->depth < db->depth ? -1 : 0);
}

#define REND_OPAQUE      1u
#define REND_DOUBLESIDED 2u
#define REND_TRANSPARENT 3u
void RenderInstances(uint8_t type) {
    uint16_t* countsArray = NULL;
    uint16_t* offsetsArray = NULL;
    uint16_t startOfNextType = 0;
    switch(type) {
        case REND_OPAQUE:      countsArray  =  modelTypeCountsOpaque; // Cull face enabled after transparents rendered.  Might have 1 frame junk but that's fine to minimize gl calls.
                               offsetsArray = modelTypeOffsetsOpaque;
                               startOfNextType = startOfDoubleSidedInstances; break;
        case REND_DOUBLESIDED: glDisable(GL_CULL_FACE);
                               countsArray  =  modelTypeCountsDoubleSided;
                               offsetsArray = modelTypeOffsetsDoubleSided;
                               startOfNextType = startOfTransparentInstances; break;
        case REND_TRANSPARENT: glEnable(GL_BLEND);
                               glDepthMask(GL_TRUE);
                               glEnable(GL_CULL_FACE);
                               glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                               glDepthMask(GL_FALSE);
                               countsArray  =  modelTypeCountsTransparent;
                               offsetsArray = modelTypeOffsetsTransparent;
                               startOfNextType = loadedInstances - invalidModelIndexCount; break;
    }
    
    if (countsArray == NULL) { DualLogError("Invalid type %u passed to RenderInstances\n",type); return; }
    
    for (uint16_t modelIdx = 0; modelIdx < loadedModels; modelIdx++) {
        if (countsArray[modelIdx] == 0) continue;

        uint16_t start = offsetsArray[modelIdx];
        uint16_t count =  countsArray[modelIdx];
        DepthSort visibleInstances[start + count];
        uint16_t visibleCount = 0;
        for (uint16_t i = start; i < start + count && i < startOfNextType; i++) { // Filter visible instances
            uint16_t instCellIdx = (uint16_t)cellIndexForInstance[i];
            if (instCellIdx < ARRSIZE && !(gridCellStates[instCellIdx] & CELL_VISIBLE)) continue;
            
            float distSqrd = squareDistance3D(instances[i].position.x, instances[i].position.y, instances[i].position.z, cam_x, cam_y, cam_z);
            if (distSqrd >= FAR_PLANE_SQUARED) continue;
            
            if (!IsSphereInFOVCone(instances[i].position.x, instances[i].position.y, instances[i].position.z)) continue;
            
            visibleInstances[visibleCount].index = i;
            visibleInstances[visibleCount].depth = distSqrd;
            visibleCount++;
            instanceIsLODArray[i] = (distSqrd >= lodRangeSqrd);
        }
        
        if (visibleCount == 0) continue;
        
        if (type == REND_TRANSPARENT) qsort(visibleInstances, visibleCount, sizeof(DepthSort), compareDepthSort); // Sort by depth (descending for back-to-front)
        else qsort(visibleInstances, visibleCount, sizeof(DepthSort), compareDepthSortInverted); // Sort by depth (ascending for front-to-back)
        
        // Set texture-related uniforms once per model type
        uint16_t firstInstance = visibleInstances[0].index; // Safe since visibleCount > 0
        uint32_t texIndex = instances[firstInstance].texIndex;
        uint32_t glowdex = (uint32_t)instances[firstInstance].glowIndex;
        uint32_t specdex = (uint32_t)instances[firstInstance].specIndex;
        uint32_t glowSpecPack = (glowdex & 0xFFFFu) | ((specdex & 0xFFFFu) << 16);
        uint32_t normInstancePack = (uint32_t)instances[firstInstance].normIndex;
        glUniform1ui(texIndexLoc_chunk, texIndex);
        glUniform1ui(glowSpecIndexLoc_chunk, glowSpecPack);
        glUniform1ui(normInstanceIndexLoc_chunk, normInstancePack);
        for (uint16_t j = 0; j < visibleCount; j++) {
            uint16_t i = visibleInstances[j].index;
            int32_t modelType = instanceIsLODArray[i] && instances[i].lodIndex < loadedModels ? instances[i].lodIndex : instances[i].modelIndex;
            glUniformMatrix4fv(matrixLoc_chunk, 1, GL_FALSE, &modelMatrices[i * 16]);
            glBindVertexBuffer(0, vbos[modelType], 0, VERTEX_ATTRIBUTES_COUNT * sizeof(float));
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tbos[modelType]);
            glDrawElements(GL_TRIANGLES, modelTriangleCounts[modelType] * 3, GL_UNSIGNED_INT, 0);
            drawCallsRenderedThisFrame++;
            verticesRenderedThisFrame += modelTriangleCounts[modelType] * 3;
        }
    }
    
    if (type == REND_TRANSPARENT) {
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
        glEnable(GL_CULL_FACE);
    }
}

int32_t main(int32_t argc, char* argv[]) {
    double programStartTime = get_time();
    console_log_file = fopen("voxen.log", "w"); // Initialize log system for all prints to go to both stdout and voxen.log file
    if (!console_log_file) DualLogError("Failed to open log file voxen.log\n");
    if (argc >= 2 && (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0)) {
        printf("-----------------------------------------------------------\n");
        printf("Voxen "
               VERSION_STRING
               "10/20/2025\nthe OpenGL Voxel Lit Rendering Engine\n\nby W. Josiah Jack\nMIT-0 licensed\n\n\n");
        return 0;
    }

    if ((argc >= 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0))
        || (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) ) {
        printf("Voxen the OpenGL Voxel Lit Rendering Engine\n");
        printf("-----------------------------------------------------------\n");
        printf("        This is a rendering engine designed for optimized focused\n");
        printf("        usage of OpenGL making maximal use of GPU Driven rendering\n");
        printf("        techniques, a unified event system for debugging and log\n");
        printf("        playback, full mod support loading all data from external\n");
        printf("        files and using definition files for what to do with the\n");
        printf("        data.\n\n");
        printf("        This project aims to have minimal overhead, profiling,\n");
        printf("        traceability, robustness, and low level control.\n\n");
        printf("\n");
        printf("Valid arguments:\n");
        printf(" < none >\n    Runs the engine as normal, loading data from \n    neighbor directories (./Textures, ./Models, etc.)\n\n");
        printf("-v, --version\n    Prints version information\n\n");
        printf("play <file>\n    Plays back recorded log from current directory\n\n");
        printf("record <file>\n    Records all engine events to designated log\n    as a .dem file\n\n");
        printf("dump <file.dem>\n    Dumps the specified log into ./log_dump.txt\n    as human readable text.  You must provide full\n    file name with extension\n\n");
        printf("-h, --help\n    Provides this help text.  Neat!\n\n");
        printf("-----------------------------------------------------------\n");
        return 0;
    }

    if (argc == 3 && strcmp(argv[1], "dump") == 0) { DualLog("Converting log to plaintext: %s ...", argv[2]); JournalDump(argv[2]); DualLog("DONE!\n"); return 0; }

    globalFrameNum = 0;
    activeLogFile = 0;
    DebugRAM("prior to event system init");
    DualLog("Voxen "
            VERSION_STRING
            " by W. Josiah Jack, MIT-0 licensed\n");
    if (EventInit()) return 1;

    if (argc == 3 && strcmp(argv[1], "play") == 0) { // Log playback
        DualLog("Playing log: %s\n", argv[2]);
        activeLogFile = fopen(argv[2], "rb");
        if (!activeLogFile) {
            DualLogError("Failed to read log: %s\n", argv[2]);
        } else {
            log_playback = true; // Perform log playback.
        }
    } else if (argc == 3 && strcmp(argv[1], "record") == 0) { // Log record
        manualLogName = argv[2];
    }

    InitializeEnvironment();
    double last_physics_time = get_time();
    last_time = get_time();
    DebugRAM("prior to game loop");
    RenderShadowmaps();
    Input_MouselookApply();
    lastJournalWriteTime = get_time();
    DualLog("Game Initialized in %f secs\n",lastJournalWriteTime - programStartTime);
    while(1) {
        current_time = get_time();
        double frame_time = current_time - last_time;
        if (!gamePaused) pauseRelativeTime += (float)frame_time;

        // Enqueue input events
        glfwPollEvents();
        if (glfwWindowShouldClose(window)) EnqueueEvent_Simple(EV_QUIT);
        double timeSinceLastPhysicsTick = current_time - last_physics_time;
        if (timeSinceLastPhysicsTick > 0.006944444f) { // 144fps fixed tick rate
            last_physics_time = current_time;
            EnqueueEvent_Simple(EV_PHYSICS_TICK);
        }

        // Enqueue all logged events for the current frame.
        if (log_playback) {
            // Read the log file for current frame and enqueue events from log.
            int32_t read_status = ReadActiveLog();
            if (read_status == 2) { // EOF reached, no more events
                DualLog("Log playback completed.  Control returned.\n");
            } else if (read_status == -1) { // Read error
                DualLogError("Error reading log file, exiting playback\n");
                EnqueueEvent_Simple(EV_QUIT);
            }
        }

        // Server Actions
        // ====================================================================
        // Server Event Queue
        if (EventQueueProcess()) break; // Do everything
        
        // Client Actions
        // ====================================================================
        // Client Render
        drawCallsRenderedThisFrame = 0; // Reset per frame
        verticesRenderedThisFrame = 0;
        
        // 0. Clear Frame Buffers and Depth
        if (!gamePaused) glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear main FBO.  glClearBufferfv was actually SLOWER!

        if (!gamePaused) { // !PAUSED BLOCK -------------------------------------------------
            // 1. Culling
            Cull(); // Get world cell culling data into gridCellStates from precomputed data at init of what cells see what other cells.
            
            // 2. Pass instance data to GPU
            for (uint32_t i = 0; i < loadedInstances; i++) { if (dirtyInstances[i]) { UpdateInstanceMatrix(i); } }
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, matricesBuffer);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, loadedInstances * 16 * sizeof(float), modelMatrices); // * 16 because matrix4x4
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

            // 3. Dynamic Shadowmaps
            RenderDynamicShadowmaps();      
            
            // 3. Raterized Geometry
            //        Standard vertex + fragment rendering, but with special packing to minimize transfer data amounts
            glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
            glUseProgram(chunkShaderProgram);
            float view[16];
            mat4_lookat(view);
            float viewProj[16];
            float invViewProj[16];
            mul_mat4(viewProj, rasterPerspectiveProjection, view);
            invertAffineMat4(invViewProj, viewProj);
            glUniformMatrix4fv(viewProjLoc_chunk, 1, GL_FALSE, viewProj);
            glUniform1f(worldMin_xLoc_chunk, worldMin_x);
            glUniform1f(worldMin_zLoc_chunk, worldMin_z);
            glUniform3f(camPosLoc_chunk, cam_x, cam_y, cam_z);
            glUniform1f(fogColorRLoc_chunk, fogColorR);
            glUniform1f(fogColorGLoc_chunk, fogColorG);
            glUniform1f(fogColorBLoc_chunk, fogColorB);
            glProgramUniform1ui(chunkShaderProgram, glGetUniformLocation(chunkShaderProgram, "reflectionsEnabled"), settings_Reflections);
            glProgramUniform1ui(chunkShaderProgram, glGetUniformLocation(chunkShaderProgram, "shadowsEnabled"), settings_Shadows);
            glBindVertexArray(vao_chunk);
            memset(instanceIsLODArray,true,INSTANCE_COUNT * sizeof(bool)); // All using lower detail LOD mesh.
            RenderInstances(REND_OPAQUE);      // Opaque, e.g. most objects and level geometry chunks
            RenderInstances(REND_DOUBLESIDED); // Double Sided, e.g. cyber panels and foliage and negative scaled objects
            RenderInstances(REND_TRANSPARENT); // Transparents, e.g. windows and beakers
            glBindVertexArray(0);
            glUseProgram(0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            // ====================================================================
            // 6. SSR (Screen Space Reflections)
            if ((debugView == 0 || debugView == 7) && settings_Reflections > 0) {
                glUseProgram(ssrShaderProgram);
                glProgramUniform1i(ssrShaderProgram, glGetUniformLocation(ssrShaderProgram, "outputImage"), 4);
                glUniformMatrix4fv(viewProjectionLoc_ssr, 1, GL_FALSE, viewProj);
                glUniform3f(camPosLoc_ssr, cam_x, cam_y, cam_z);
                GLuint groupX_ssr = ((screen_width / SSR_RES) + 31) / 32;
                GLuint groupY_ssr = ((screen_height / SSR_RES) + 31) / 32;
                glDispatchCompute(groupX_ssr, groupY_ssr, 1);
                glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            }
        } else { // END !PAUSED BLOCK -------------------------------------------------
            glBindFramebuffer(GL_FRAMEBUFFER, 0); // Allow text to still render while paused
        }
        
        // 7. Render final meshes' results with full screen quad
        glUseProgram(imageBlitShaderProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, inputImageID);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, outputImageID);
        glProgramUniform1i(imageBlitShaderProgram, glGetUniformLocation(imageBlitShaderProgram, "outputImage"), 4);
        glProgramUniform1ui(imageBlitShaderProgram, glGetUniformLocation(imageBlitShaderProgram, "reflectionsEnabled"), settings_Reflections);
        glProgramUniform1ui(imageBlitShaderProgram, glGetUniformLocation(imageBlitShaderProgram, "aaEnabled"), settings_AntiAliasing);
        glProgramUniform1ui(imageBlitShaderProgram, glGetUniformLocation(imageBlitShaderProgram, "brightnessSetting"), settings_Brightness);
        glProgramUniform1f(imageBlitShaderProgram, glGetUniformLocation(imageBlitShaderProgram, "fov"), cam_fov);
        glProgramUniform1i(imageBlitShaderProgram, texLoc_quadblit, 0);
        glUniform3f(glGetUniformLocation(imageBlitShaderProgram, "camRot"), deg2rad(cam_yaw), deg2rad(cam_pitch), deg2rad(cam_roll));
        glProgramUniform1f(imageBlitShaderProgram, glGetUniformLocation(imageBlitShaderProgram, "timeVal"), pauseRelativeTime * 0.1);
        glBindVertexArray(quadVAO);
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glEnable(GL_DEPTH_TEST);
        glBindTextureUnit(0, 0);
        glUseProgram(0);
        RenderUI();
        cpuTime = get_time() - current_time;
        glfwSwapBuffers(window); // Present frame
        CHECK_GL_ERROR();
        globalFrameNum++;
        #ifdef DEBUG_RAM_OUTPUT
            if (globalFrameNum == 4) DebugRAM("after 4 frames of running");
            else if (globalFrameNum == 100) DebugRAM("after 100 frames of running");
            else if (globalFrameNum == 200) DebugRAM("after 200 frames of running");
        #endif
    }
    glfwTerminate();
    return 0;
}
