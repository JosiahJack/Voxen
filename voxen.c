// File: voxen.c
// Description: A realtime OpenGL based application for experimenting with voxel lighting techniques to derive new methods of high speed accurate lighting in resource constrained environements (e.g. embedded).
#define VERSION_STRING "v0.3.0"

#include <malloc.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include "event.h"
#include "constants.h"
#include "input.h"
#include "data_textures.h"
#include "data_models.h"
#include "render.h"
#include "cli_args.h"
#include "network.h"
#include "text.h"
#include "shaders.h"
#include "audio.h"
#include "instance.h"
#include "voxel.h"
#include "data_parser.h"
#include "data_entities.h"
#include "player.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// #define DEBUG_RAM_OUTPUT

// Window
SDL_Window *window;
int screen_width = 1920, screen_height = 1080;
bool window_has_focus = false;
static FILE *console_log_file = NULL;

// Game/Mod Definition
DataParser gamedata_parser;
uint8_t numLevels = 2;
uint8_t startLevel = 0;
const char *valid_gamedata_keys[] = {"levelcount","startlevel"};
#define NUM_GAMDAT_KEYS 2

// Event System states
int maxEventCount_debug = 0;
double lastJournalWriteTime = 0;
uint32_t globalFrameNum = 0;
FILE* activeLogFile;
const char* manualLogName;
bool log_playback = false;
Event eventQueue[MAX_EVENTS_PER_FRAME]; // Queue for events to process this frame
Event eventJournal[EVENT_JOURNAL_BUFFER_SIZE]; // Journal buffer for event history to write into the log/demo file
int eventJournalIndex;
int eventIndex; // Event that made it to the counter.  Indices below this were
                // already executed and walked away from the counter.
int eventQueueEnd; // End of the waiting line
const double time_step = 1.0 / 60.0; // 60fps
double last_time = 0.0;
double current_time = 0.0;
double start_frame_time = 0.0;
double screenshotTimeout = 0.0;

// ----------------------------------------------------------------------------
// OpenGL / Rendering
SDL_GLContext gl_context;

double lastFrameSecCountTime = 0.00;
uint32_t lastFrameSecCount = 0;
uint32_t framesPerLastSecond = 0;
uint32_t worstFPS = UINT32_MAX;
uint32_t drawCallsRenderedThisFrame = 0; // Total draw calls this frame
uint32_t shadowDrawCallsRenderedThisFrame = 0;
uint32_t verticesRenderedThisFrame = 0;
bool shadowsEnabled = true;
uint32_t playerCellIdx = 80000;
uint32_t playerCellIdx_x = 20000;
uint32_t playerCellIdx_y = 10000;
uint32_t playerCellIdx_z = 451;
uint8_t numLightsFound = 0;
float sightRangeSquared = 71.68f * 71.68f; // Max player view, level 6 crawlway 28 cells

GLuint vao_chunk; // Vertex Array Object

// Shaders
GLuint chunkShaderProgram;
GLint viewLoc_chunk = -1, projectionLoc_chunk = -1, matrixLoc_chunk = -1, texIndexLoc_chunk = -1,
      instanceIndexLoc_chunk = -1, modelIndexLoc_chunk = -1, debugViewLoc_chunk = -1, glowIndexLoc_chunk = -1,
      specIndexLoc_chunk = -1, instancesInPVSCount_chunk = -1, shadowsEnabledLoc_chunk = -1,
      overrideGlowRLoc_chunk = -1, overrideGlowGLoc_chunk = -1, overrideGlowBLoc_chunk = -1; // uniform locations
      
GLuint screenSpaceShadowsComputeShader;

//    Full Screen Quad Blit for rendering final output/image effect passes
GLuint imageBlitShaderProgram;
GLuint quadVAO, quadVBO;
GLint texLoc_quadblit = -1, debugViewLoc_quadblit = -1, debugValueLoc_quadblit = -1; // uniform locations

//    FBO
GLuint inputImageID, inputDepthID, inputWorldPosID, inputShadowStencilID, gBufferFBO; // FBO inputs

//    Cubemap
bool generatedCubemapProbes = false;
GLuint inputCubemapImageID, inputCubemapDepthID, inputCubemapWorldPosID, gBufferCubemapFBO;
int cubemapTexSize = 128;
GLuint cubemapTextures[NUM_CUBEMAPS] = {0};
float cubemapPositions[NUM_CUBEMAPS * 3];

// Lights
// Could reduce spotAng to minimal bits.  I only have 6 spot lights and half are 151.7 and other half are 135.
float lights[LIGHT_COUNT * LIGHT_DATA_SIZE];
bool lightDirty[MAX_VISIBLE_LIGHTS] = { [0 ... MAX_VISIBLE_LIGHTS-1] = true };
float lightsRangeSquared[LIGHT_COUNT];
float lightsInProximity[MAX_VISIBLE_LIGHTS * LIGHT_DATA_SIZE];
bool firstLightGen = true;

// Text
GLuint textShaderProgram;
TTF_Font* font = NULL;
GLuint textVAO, textVBO;
SDL_Color textColWhite = {255, 255, 255, 255};
SDL_Color textColRed = {255, 0, 0, 255};
SDL_Color textColGreen = {20, 255, 30, 255};
SDL_Color textColors[6] = {
    {255, 255, 255, 255}, // 0 White 1.0f, 1.0f, 1.0f
    {227, 223,   0, 255}, // 1 Yellow 0.8902f, 0.8745f, 0f
    {159, 156,   0, 255}, // 2 Dark Yellow 0.8902f * 0.7f, 0.8745f * 0.7f, 0f
    { 95, 167,  43, 255}, // 3 Green 0.3725f, 0.6549f, 0.1686f
    {234,  35,  43, 255}, // 4 Red 0.9176f, 0.1373f, 0.1686f
    {255, 127,   0, 255}  // 5 Orange 1f, 0.498f, 0f
};

float textQuadVertices[] = { // 2 triangles, text is applied as an image from SDL TTF
    // Positions   // Tex Coords
    0.0f, 0.0f,    0.0f, 0.0f, // Bottom-left
    1.0f, 0.0f,    1.0f, 0.0f, // Bottom-right
    1.0f, 1.0f,    1.0f, 1.0f, // Top-right
    0.0f, 1.0f,    0.0f, 1.0f  // Top-left
};

char uiTextBuffer[TEXT_BUFFER_SIZE];
GLint projectionLoc_text = -1, textColorLoc_text = -1, textTextureLoc_text = -1, texelSizeLoc_text = -1; // uniform locations
// ----------------------------------------------------------------------------

void DualLog(const char *fmt, ...) {
    va_list args1, args2; va_start(args1, fmt); va_copy(args2, args1);
    vfprintf(stdout, fmt, args1); // Print to console/terminal.
    va_end(args1);
    if (console_log_file) { vfprintf(console_log_file, fmt, args2); fflush(console_log_file); } // Print to log file.
    va_end(args2);
}

void DualLogError(const char *fmt, ...) {
    va_list args1, args2; va_start(args1, fmt); va_copy(args2, args1);
    fprintf(stderr, "\033[1;31mERROR: \033[0;31m"); vfprintf(stderr, fmt, args1); fprintf(stderr,"\033[0;0m");
    fflush(stderr); // Print to console/terminal.
    va_end(args1);
    if (console_log_file) {
        fprintf(console_log_file, "ERROR: "); vfprintf(console_log_file, fmt, args2);
        fflush(console_log_file); // Print to log file.
    }
    
    va_end(args2);
}

// Get RSS aka the total RAM reported by btop or other utilities that's allocated virtual ram for the process.
size_t get_rss_bytes(void) {
    FILE *fp = fopen("/proc/self/stat", "r");
    if (!fp) { DualLogError("Failed to open /proc/self/stat\n"); return 0; }

    char buffer[1024];
    if (!fgets(buffer, sizeof(buffer), fp)) { DualLogError("Failed to read /proc/self/stat\n"); fclose(fp); return 0; }
    
    fclose(fp);
    char *token = strtok(buffer, " ");
    int field = 0;
    size_t rss_pages = 0;
    while (token && field < 23) { // Parse the 24th field (RSS in pages)
        token = strtok(NULL, " ");
        field++;
    }
    if (token) { rss_pages = atol(token);
    } else { DualLogError("Failed to parse RSS from /proc/self/stat\n"); return 0; }

    return rss_pages * sysconf(_SC_PAGESIZE);
}

void DebugRAM(const char *context, ...) {
    // Buffer to hold formatted context string
    char formatted_context[1024];

    // Process variable arguments
    va_list args;
    va_start(args, context);
    vsnprintf(formatted_context, sizeof(formatted_context), context, args);
    va_end(args);

#ifdef DEBUG_RAM_OUTPUT
    struct mallinfo2 info = mallinfo2();
    size_t rss_bytes = get_rss_bytes();
    // Log heap and RSS usage with formatted context
    DualLog("Memory at %s: Heap usage %zu bytes (%zu KB | %.2f MB), RSS %zu bytes (%zu KB | %.2f MB)\n",
            formatted_context,
            info.uordblks, info.uordblks / 1024, info.uordblks / 1024.0 / 1024.0,
            rss_bytes, rss_bytes / 1024, rss_bytes / 1024.0 / 1024.0);
#endif
}

void print_bytes_no_newline(int count) {
    DualLog("%d bytes | %f kb | %f Mb",count,(float)count / 1000.0f,(float)count / 1000000.0f);
}

// ============================================================================
// OpenGL / Rendering Helper Functions

void mat4_identity(float* m) {
    for (int i = 0; i < 16; i++) m[i] = (i % 5 == 0) ? 1.0f : 0.0f;
}

// Generates Projection Matrix4x4 for Geometry Rasterizer Pass.
void mat4_perspective(float* m, float fov, float aspect, float near, float far) {
    float f = 1.0f / tan(fov * M_PI / 360.0f);
    mat4_identity(m);
    m[0] = f / aspect;
    m[5] = f;
    m[10] = -(far + near) / (far - near); // Fixed: Negative for correct depth
    m[11] = -1.0f;
    m[14] = -2.0f * far * near / (far - near);
    m[15] = 0.0f;
}

// Generates View Matrix4x4 for Geometry Rasterizer Pass
void mat4_lookat(float* m, float eyeX, float eyeY, float eyeZ, Quaternion* orientation) {
    // Convert quaternion to rotation matrix
    float rotation[16];
    quat_to_matrix(orientation, rotation);

    // Extract forward, right, and up vectors from rotation matrix
    float right[3] = { -rotation[0], -rotation[1], -rotation[2] };
    float up[3] = { rotation[4], rotation[5], rotation[6] };
    float forward[3] = { rotation[8], rotation[9], rotation[10] };

    // Build view matrix
    mat4_identity(m);
    m[0] = right[0];  m[4] = right[1];  m[8]  = right[2];  m[12] = -(right[0] * eyeX + right[1] * eyeY + right[2] * eyeZ);
    m[1] = up[0];     m[5] = up[1];     m[9]  = up[2];     m[13] = -(up[0] * eyeX + up[1] * eyeY + up[2] * eyeZ);
    m[2] = -forward[0]; m[6] = -forward[1]; m[10] = -forward[2]; m[14] = -(-forward[0] * eyeX + -forward[1] * eyeY + -forward[2] * eyeZ);
    m[3] = 0.0f;      m[7] = 0.0f;      m[11] = 0.0f;      m[15] = 1.0f;
}

// Matrix helper for 2D orthographic projection for text/UI
void mat4_ortho(float* m, float left, float right, float bottom, float top, float near, float far) {
    mat4_identity(m);
    m[0] = 2.0f / (right - left);
    m[5] = 2.0f / (top - bottom);
    m[10] = -2.0f / (far - near);
    m[12] = -(right + left) / (right - left);
    m[13] = -(top + bottom) / (top - bottom);
    m[14] = -(far + near) / (far - near);
    m[15] = 1.0f;
}

// Convert quaternion to a 4x4 matrix
void quat_to_matrix(Quaternion* q, float* m) {
    float xx = q->x * q->x;
    float xy = q->x * q->y;
    float xz = q->x * q->z;
    float xw = q->x * q->w;
    float yy = q->y * q->y;
    float yz = q->y * q->z;
    float yw = q->y * q->w;
    float zz = q->z * q->z;
    float zw = q->z * q->w;

    mat4_identity(m);
    m[0] = 1.0f - 2.0f * (yy + zz);  // Right X
    m[1] = 2.0f * (xy + zw);          // Right Y
    m[2] = 2.0f * (xz - yw);          // Right Z
    m[4] = 2.0f * (xy - zw);          // Up X
    m[5] = 1.0f - 2.0f * (xx + zz);  // Up Y
    m[6] = 2.0f * (yz + xw);          // Up Z
    m[8] = 2.0f * (xz + yw);          // Forward X
    m[9] = 2.0f * (yz - xw);          // Forward Y
    m[10] = 1.0f - 2.0f * (xx + yy); // Forward Z
    m[3] = 0.0f; m[7] = 0.0f; m[11] = 0.0f; m[15] = 1.0f;
}

void GenerateAndBindTexture(GLuint *id, GLenum internalFormat, int width, int height, GLenum format, GLenum type, GLenum target, const char *name) {
    glGenTextures(1, id);
    glBindTexture(target, *id);
    glTexImage2D(target, 0, internalFormat, width, height, 0, format, type, NULL);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    if (target == GL_TEXTURE_CUBE_MAP) {
        glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        CHECK_GL_ERROR();
        glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        CHECK_GL_ERROR();
        glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        CHECK_GL_ERROR();
        glTexParameteri(target, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        CHECK_GL_ERROR();
    }
    glBindTexture(target, 0);
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) DualLogError("Failed to create texture %s: OpenGL error %d\n", name, error);
}

void CacheUniformLocationsForShaders(void) {
    // Called after shader compilation in InitializeEnvironment
    viewLoc_chunk = glGetUniformLocation(chunkShaderProgram, "view");
    projectionLoc_chunk = glGetUniformLocation(chunkShaderProgram, "projection");
    matrixLoc_chunk = glGetUniformLocation(chunkShaderProgram, "matrix");
    texIndexLoc_chunk = glGetUniformLocation(chunkShaderProgram, "texIndex");
    glowIndexLoc_chunk = glGetUniformLocation(chunkShaderProgram, "glowIndex");
    specIndexLoc_chunk = glGetUniformLocation(chunkShaderProgram, "specIndex");
    instanceIndexLoc_chunk = glGetUniformLocation(chunkShaderProgram, "instanceIndex");
    modelIndexLoc_chunk = glGetUniformLocation(chunkShaderProgram, "modelIndex");
    debugViewLoc_chunk = glGetUniformLocation(chunkShaderProgram, "debugView");
    instancesInPVSCount_chunk = glGetUniformLocation(chunkShaderProgram, "instancesInPVSCount");
    shadowsEnabledLoc_chunk = glGetUniformLocation(chunkShaderProgram, "shadowsEnabled");
    overrideGlowRLoc_chunk = glGetUniformLocation(chunkShaderProgram, "overrideGlowR");
    overrideGlowGLoc_chunk = glGetUniformLocation(chunkShaderProgram, "overrideGlowG");
    overrideGlowBLoc_chunk = glGetUniformLocation(chunkShaderProgram, "overrideGlowB");
    
    texLoc_quadblit = glGetUniformLocation(imageBlitShaderProgram, "tex");
    debugViewLoc_quadblit = glGetUniformLocation(imageBlitShaderProgram, "debugView");
    debugValueLoc_quadblit = glGetUniformLocation(imageBlitShaderProgram, "debugValue");
    
    projectionLoc_text = glGetUniformLocation(textShaderProgram, "projection");
    textColorLoc_text = glGetUniformLocation(textShaderProgram, "textColor");
    textTextureLoc_text = glGetUniformLocation(textShaderProgram, "textTexture");
    texelSizeLoc_text = glGetUniformLocation(textShaderProgram, "texelSize");
}

void SetUpdatedMatrix(float * mat, float posx, float posy, float posz, float rotx, float roty, float rotz, float rotw, float sclx, float scly, float sclz) {
    float x = rotx, y = roty, z = rotz, w = rotw;
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
    mat[0]  = rot[0] * sclx; mat[1]  = rot[1] * scly; mat[2]  = rot[2]  * sclz; mat[3]  = 0.0f;
    mat[4]  = rot[4] * sclx; mat[5]  = rot[5] * scly; mat[6]  = rot[6]  * sclz; mat[7]  = 0.0f;
    mat[8]  = rot[8] * sclx; mat[9]  = rot[9] * scly; mat[10] = rot[10] * sclz; mat[11] = 0.0f;
    mat[12] =          posx; mat[13] =          posy; mat[14] =           posz; mat[15] = 1.0f;
}

void UpdateInstanceMatrix(int i) {
    if (instances[i].modelIndex >= MODEL_COUNT) { dirtyInstances[i] = false; return; }
    if (modelVertexCounts[instances[i].modelIndex] < 1) { dirtyInstances[i] = false; return; } // Empty model
    if (instances[i].modelIndex < 0) return; // Culled
    
    float mat[16]; // 4x4 matrix
    SetUpdatedMatrix(mat, instances[i].posx, instances[i].posy, instances[i].posz, instances[i].rotx, instances[i].roty, instances[i].rotz, instances[i].rotw, instances[i].sclx, instances[i].scly, instances[i].sclz);
    memcpy(&modelMatrices[i * 16], mat, 16 * sizeof(float));
    dirtyInstances[i] = false;
}

// Renders text at x,y coordinates specified using pointer to the string array.
void RenderText(float x, float y, const char *text, int colorIdx) {
    glDisable(GL_CULL_FACE); // Disable backface culling
    CHECK_GL_ERROR();
    if (!font) { DualLogError("Font is NULL\n"); return; }
    if (!text) { DualLogError("Text is NULL\n"); return; }
    
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, textColors[colorIdx]);
    if (!surface) { DualLogError("TTF_RenderText_Solid failed: %s\n", TTF_GetError()); return; }
    
    SDL_Surface *rgba_surface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(surface);
    if (!rgba_surface) { DualLogError("SDL_ConvertSurfaceFormat failed: %s\n", SDL_GetError()); return; }

    GLuint texture;
    glCreateTextures(GL_TEXTURE_2D, 1, &texture);
    CHECK_GL_ERROR();
    glTextureStorage2D(texture, 1, GL_RGBA8, rgba_surface->w, rgba_surface->h);
    CHECK_GL_ERROR();
    glTextureSubImage2D(texture, 0, 0, 0, rgba_surface->w, rgba_surface->h, GL_RGBA, GL_UNSIGNED_BYTE, rgba_surface->pixels);
    CHECK_GL_ERROR();
    glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    CHECK_GL_ERROR();
    glUseProgram(textShaderProgram);
    CHECK_GL_ERROR();
    float projection[16];
    mat4_ortho(projection, 0.0f, (float)screen_width, (float)screen_height, 0.0f, -1.0f, 1.0f);
    glProgramUniformMatrix4fv(textShaderProgram, projectionLoc_text, 1, GL_FALSE, projection);
    float r = textColors[colorIdx].r / 255.0f;
    float g = textColors[colorIdx].g / 255.0f;
    float b = textColors[colorIdx].b / 255.0f;
    float a = textColors[colorIdx].a / 255.0f;
    glProgramUniform4f(textShaderProgram, textColorLoc_text, r, g, b, a);
    CHECK_GL_ERROR();
    glBindTextureUnit(0,texture);
    CHECK_GL_ERROR();
    glProgramUniform1i(textShaderProgram, textTextureLoc_text, 0);
    CHECK_GL_ERROR();
    float scaleX = (float)rgba_surface->w;
    float scaleY = (float)rgba_surface->h;
    glProgramUniform2f(textShaderProgram, texelSizeLoc_text, 1.0f / scaleX, 1.0f / scaleY);
    CHECK_GL_ERROR();
    glEnable(GL_BLEND); // Enable blending for text transparency
    CHECK_GL_ERROR();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    CHECK_GL_ERROR();
    glDisable(GL_DEPTH_TEST); // Disable depth test for 2D overlay
    CHECK_GL_ERROR();
    glBindVertexArray(textVAO);
    CHECK_GL_ERROR();
    float vertices[] = {
        x,          y,          0.0f, 0.0f, // Bottom-left
        x + scaleX, y,          1.0f, 0.0f, // Bottom-right
        x + scaleX, y + scaleY, 1.0f, 1.0f, // Top-right
        x,          y + scaleY, 0.0f, 1.0f  // Top-left
    };
    
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    CHECK_GL_ERROR();
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    CHECK_GL_ERROR();
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4); // Render quad (two triangles)
    CHECK_GL_ERROR();
    drawCallsRenderedThisFrame++;
    verticesRenderedThisFrame+=6;

    // Cleanup
    glBindVertexArray(0);
    CHECK_GL_ERROR();
    glBindTextureUnit(0, 0);
    CHECK_GL_ERROR();
    glDisable(GL_BLEND);
    CHECK_GL_ERROR();
    glEnable(GL_DEPTH_TEST); // Re-enable depth test for 3D rendering
    CHECK_GL_ERROR();
    glUseProgram(0);
    CHECK_GL_ERROR();
    glDeleteTextures(1, &texture);
    CHECK_GL_ERROR();
    SDL_FreeSurface(rgba_surface);
    CHECK_GL_ERROR();
    glEnable(GL_CULL_FACE); // Reenable backface culling
    CHECK_GL_ERROR();
}

void RenderFormattedText(int x, int y, uint32_t color, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(uiTextBuffer, TEXT_BUFFER_SIZE, format, args);
    va_end(args);
    RenderText(x, y, uiTextBuffer, color);
}

void Screenshot() {
    struct stat st = {0};
    if (stat("Screenshots", &st) == -1) { // Check and make ./Screenshots/ folder if it doesn't exist yet.
        if (mkdir("Screenshots", 0755) != 0) { DualLog("Failed to create Screenshots folder: %s\n", strerror(errno)); return; }
    }
    
    unsigned char* pixels = (unsigned char*)malloc(screen_width * screen_height * 4);
    if (!pixels) { DualLog("Failed to allocate memory for screenshot pixels\n"); return; }

    glReadPixels(0, 0, screen_width, screen_height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    CHECK_GL_ERROR();
    char timestamp[32];
    char filename[96];
    time_t now = time(NULL);
    struct tm *utc_time = localtime(&now);
    if (!utc_time) { DualLog("Failed to get current time for screenshot!\n"); free(pixels); return; }
    
    strftime(timestamp, sizeof(timestamp), "%d%b%Y_%H_%M_%S", utc_time);
    snprintf(filename, sizeof(filename), "Screenshots/%s_%s.png", timestamp, VERSION_STRING);
    int success = stbi_write_png(filename, screen_width, screen_height, 4, pixels, screen_width * 4);
    if (!success) DualLog("Failed to save screenshot\n");
    else DualLog("Saved screenshot %s\n", filename);

    free(pixels);
}

void ScreenshotCubemapFace(int face) {
    // Write to file
    // Allocate buffer for pixel data (RGBA, 8 bits per channel)
    unsigned char* pixels = (unsigned char*)malloc(cubemapTexSize * cubemapTexSize * 4);
    if (!pixels) { DualLog("Failed to allocate memory for cubemap pixels\n"); return; }

    // Read pixels from the current framebuffer
    glReadPixels(0, 0, cubemapTexSize, cubemapTexSize, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    CHECK_GL_ERROR();

    // Generate filename (e.g., probe0_face0.png, probe0_face1.png, etc.)
    char filename[64];
    switch(face) {
        case 0: snprintf(filename, sizeof(filename), "cubemap_%d_ft.png", face); break;
        case 1: snprintf(filename, sizeof(filename), "cubemap_%d_bk.png", face); break;
        case 2: snprintf(filename, sizeof(filename), "cubemap_%d_rt.png", face); break;
        case 3: snprintf(filename, sizeof(filename), "cubemap_%d_lf.png", face); break;
        case 4: snprintf(filename, sizeof(filename), "cubemap_%d_up.png", face); break;
        case 5: snprintf(filename, sizeof(filename), "cubemap_%d_dn.png", face); break;
    }

    // Save to PNG using stb_image_write
    int success = stbi_write_png(filename, cubemapTexSize, cubemapTexSize, 4, pixels, cubemapTexSize * 4);
    if (!success) {
        DualLog("Failed to save cubemap face %d to %s\n", face, filename);
    } else {
        DualLog("Saved cubemap face %d to %s\n", face, filename);
    }

    // Free pixel buffer
    free(pixels);
}
// ============================================================================

typedef enum {
    SYS_SDL = 0,
    SYS_TTF,
    SYS_WIN,
    SYS_CTX,
    SYS_NET,
    SYS_AUD,
    SYS_COUNT // Number of subsystems
} SystemType;

bool systemInitialized[SYS_COUNT] = { [0 ... SYS_COUNT - 1] = false };

int InitializeEnvironment(void) {
    DebugRAM("InitializeEnvironment start");
    if (SDL_Init(SDL_INIT_VIDEO) < 0) { DualLogError("SDL_Init failed: %s\n", SDL_GetError()); return SYS_SDL + 1; }
    systemInitialized[SYS_SDL] = true;
    DebugRAM("SDL init");
    malloc_trim(0);

    if (TTF_Init() < 0) { DualLogError("TTF_Init failed: %s\n", TTF_GetError()); return SYS_TTF + 1; }
    systemInitialized[SYS_TTF] = true;
    DebugRAM("TTF init");
    malloc_trim(0);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    window = SDL_CreateWindow("Voxen, the OpenGL Voxel Lit Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, SDL_WINDOW_OPENGL);
    if (!window) { DualLogError("SDL_CreateWindow failed: %s\n", SDL_GetError()); return SYS_WIN + 1; }
    systemInitialized[SYS_WIN] = true;
    DebugRAM("window init");
    malloc_trim(0);

    gl_context = SDL_GL_CreateContext(window);
    if (!gl_context) { DualLogError("SDL_GL_CreateContext failed: %s\n", SDL_GetError()); return SYS_CTX + 1; }    
    systemInitialized[SYS_CTX] = true;
    DebugRAM("GL init");
    malloc_trim(0);
    
    stbi_flip_vertically_on_write(1);

    SDL_GL_MakeCurrent(window, gl_context);
    glewExperimental = GL_TRUE; // Enable modern OpenGL support
    if (glewInit() != GLEW_OK) { DualLog("GLEW initialization failed\n"); return SYS_CTX + 1; }

    // Diagnostic: Print OpenGL version and renderer
    const GLubyte* version = glGetString(GL_VERSION);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    DualLog("OpenGL Version: %s\n", version ? (const char*)version : "unknown");
    DualLog("Renderer: %s\n", renderer ? (const char*)renderer : "unknown");
    malloc_trim(0);

    int vsync_enable = 0;//1; // Set to 0 for false.
    SDL_GL_SetSwapInterval(vsync_enable);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_CULL_FACE); // Enable backface culling
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW); // Flip triangle sorting order
    glViewport(0, 0, screen_width, screen_height);
    CHECK_GL_ERROR();

    if (CompileShaders()) return SYS_COUNT + 1;
    DebugRAM("compile shaders");
    malloc_trim(0);
    
    // Setup full screen quad for image blit for post processing effects like lighting.
    float vertices[] = {
         1.0f, -1.0f, 1.0f, 0.0f, // Bottom-right
         1.0f,  1.0f, 1.0f, 1.0f, // Top-right
        -1.0f,  1.0f, 0.0f, 1.0f, // Top-left
        -1.0f, -1.0f, 0.0f, 0.0f  // Bottom-left
    };
    
    glCreateBuffers(1, &quadVBO);
    CHECK_GL_ERROR();
    glNamedBufferData(quadVBO, sizeof(vertices), vertices, GL_STATIC_DRAW);
    CHECK_GL_ERROR();
    glCreateVertexArrays(1, &quadVAO);
    CHECK_GL_ERROR();
    glEnableVertexArrayAttrib(quadVAO, 0);
    CHECK_GL_ERROR();
    glEnableVertexArrayAttrib(quadVAO, 1);
    CHECK_GL_ERROR();
    glVertexArrayAttribFormat(quadVAO, 0, 2, GL_FLOAT, GL_FALSE, 0); // DSA: Set position format
    CHECK_GL_ERROR();
    glVertexArrayAttribFormat(quadVAO, 1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float)); // DSA: Set texcoord format
    CHECK_GL_ERROR();
    glVertexArrayVertexBuffer(quadVAO, 0, quadVBO, 0, 4 * sizeof(float)); // DSA: Link VBO to VAO
    CHECK_GL_ERROR();
    glVertexArrayAttribBinding(quadVAO, 0, 0); // DSA: Bind position attribute to binding index 0
    CHECK_GL_ERROR();
    glVertexArrayAttribBinding(quadVAO, 1, 0); // DSA: Bind texcoord attribute to binding index 0
    CHECK_GL_ERROR();
    
    // VAO for Global Vertex Definition
    glGenVertexArrays(1, &vao_chunk);
    CHECK_GL_ERROR();
    glBindVertexArray(vao_chunk);
    CHECK_GL_ERROR();
    
    glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0); // Position (vec3)
    glVertexAttribFormat(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float)); // Normal (vec3)
    glVertexAttribFormat(2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float)); // Tex Coord (vec2)
    glVertexAttribFormat(3, 1, GL_FLOAT, GL_FALSE, 8 * sizeof(float)); // Tex Index (int)
    glVertexAttribFormat(4, 1, GL_FLOAT, GL_FALSE, 9 * sizeof(float)); // Glow Index (int)
    glVertexAttribFormat(5, 1, GL_FLOAT, GL_FALSE, 10 * sizeof(float)); // Spec Index (int)
    glVertexAttribFormat(6, 1, GL_FLOAT, GL_FALSE, 11 * sizeof(float)); // Normal Index (int)
    glVertexAttribFormat(7, 1, GL_FLOAT, GL_FALSE, 12 * sizeof(float)); // Model Index (int)
    glVertexAttribFormat(8, 1, GL_FLOAT, GL_FALSE, 13 * sizeof(float)); // Instance Index (int)
    for (uint8_t i = 0; i < 9; i++) {
        glVertexAttribBinding(i, 0);
        glEnableVertexAttribArray(i);
    }

    glBindVertexArray(0);
    DebugRAM("after vao chunk bind");
    
    // Initialize Lights
    for (uint16_t i = 0; i < LIGHT_COUNT; i++) {
        uint16_t base = i * LIGHT_DATA_SIZE; // Step by 12
        lights[base + 0] = base * 0.64f; // posx
        lights[base + 1] = base * 0.64f; // posy
        lights[base + 2] = 0.0f; // posz
        lights[base + 3] = 5.0f; // intensity
        lights[base + 4] = 5.24f; // radius
        lightsRangeSquared[i] = 5.24f * 5.24f;
        lights[base + 5] = 0.0f; // spotAng
        lights[base + 6] = 0.0f; // spotDirx
        lights[base + 7] = 0.0f; // spotDiry
        lights[base + 8] = -1.0f; // spotDirz
        lights[base + 9] = 1.0f; // r
        lights[base + 10] = 1.0f; // g
        lights[base + 11] = 1.0f; // b
    }
    
    lights[0] = 0.0f;
    lights[1] = -1.28f;
    lights[2] = 0.0f; // Fixed Z height
    lights[3] = 2.0f; // Default intensity
    lights[4] = 10.0f; // Default radius
    lightsRangeSquared[0] = 10.0f * 10.0f;
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
    lightsRangeSquared[1] = 10.0f * 10.0f;
    lights[6 + 12] = 0.0f;
    lights[7 + 12] = 0.0f;
    lights[8 + 12] = -1.0f;
    lights[9 + 12] = 1.0f;
    lights[10 + 12] = 0.0f;
    lights[11 + 12] = 0.0f;
    DebugRAM("init lights");

    // Create Framebuffer
    // First pass gbuffer images
    GenerateAndBindTexture(&inputImageID,             GL_RGBA8, screen_width, screen_height,            GL_RGBA,           GL_UNSIGNED_BYTE, GL_TEXTURE_2D, "Primary Lit Raster");
    CHECK_GL_ERROR();
    GenerateAndBindTexture(&inputDepthID, GL_DEPTH_COMPONENT24, screen_width, screen_height, GL_DEPTH_COMPONENT,            GL_UNSIGNED_INT, GL_TEXTURE_2D, "Primary Raster Depth");
    CHECK_GL_ERROR();
    GenerateAndBindTexture(&inputWorldPosID,        GL_RGBA32F, screen_width, screen_height,            GL_RGBA,                   GL_FLOAT, GL_TEXTURE_2D, "GBuffer World Position At Each Pixel");
    CHECK_GL_ERROR();
    GenerateAndBindTexture(&inputShadowStencilID,        GL_R8, screen_width, screen_height,            GL_RGBA,                   GL_FLOAT, GL_TEXTURE_2D, "GBuffer Bitmask for What Lights See Pixel");
    CHECK_GL_ERROR();

    glGenFramebuffers(1, &gBufferFBO);
    CHECK_GL_ERROR();
    glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
    CHECK_GL_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, inputImageID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, inputWorldPosID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, inputDepthID, 0);
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
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
    
    glBindImageTexture(0, inputImageID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
    CHECK_GL_ERROR();
    glBindImageTexture(1, inputWorldPosID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    CHECK_GL_ERROR();
    glBindImageTexture(2, inputShadowStencilID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R8);
    CHECK_GL_ERROR();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CHECK_GL_ERROR();
    malloc_trim(0);
    
    // Cubemap FBO duplicate
    GenerateAndBindTexture(&inputCubemapImageID,             GL_RGBA8, cubemapTexSize, cubemapTexSize,            GL_RGBA,           GL_UNSIGNED_BYTE, GL_TEXTURE_2D, "Cubemap Lit Raster");
    CHECK_GL_ERROR();
    GenerateAndBindTexture(&inputCubemapDepthID, GL_DEPTH_COMPONENT24, cubemapTexSize, cubemapTexSize, GL_DEPTH_COMPONENT,            GL_UNSIGNED_INT, GL_TEXTURE_2D, "Cubemap Raster Depth");
    CHECK_GL_ERROR();
    GenerateAndBindTexture(&inputCubemapWorldPosID,        GL_RGBA32F, cubemapTexSize, cubemapTexSize,            GL_RGBA,                   GL_FLOAT, GL_TEXTURE_2D, "Cubemap World Position At Each Pixel");
    CHECK_GL_ERROR();
    
    glGenFramebuffers(1, &gBufferCubemapFBO);
    CHECK_GL_ERROR();
    glBindFramebuffer(GL_FRAMEBUFFER, gBufferCubemapFBO);
    CHECK_GL_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, inputCubemapImageID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, inputCubemapWorldPosID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, inputCubemapDepthID, 0);
    GLenum drawCubemapBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, drawCubemapBuffers);
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        switch (status) {
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: DualLogError("Framebuffer incomplete: Attachment issue\n"); break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: DualLogError("Framebuffer incomplete: Missing attachment\n"); break;
            case GL_FRAMEBUFFER_UNSUPPORTED: DualLogError("Framebuffer incomplete: Unsupported configuration\n"); break;
            default: DualLogError("Framebuffer incomplete: Error code %d\n", status);
        }
    }
    
    // Don't bother binding image textures for cubemap here, done in render pass.
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CHECK_GL_ERROR();
    malloc_trim(0);
    DebugRAM("setup gbuffer end");
    
    // Initialize Cubemap Textures
    glGenTextures(NUM_CUBEMAPS, cubemapTextures);
    CHECK_GL_ERROR();
    for (int i = 0; i < NUM_CUBEMAPS; ++i) {
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextures[i]);
        CHECK_GL_ERROR();
        for (int face = 0; face < 6; ++face) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGBA8, cubemapTexSize, cubemapTexSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            CHECK_GL_ERROR();
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        CHECK_GL_ERROR();
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        CHECK_GL_ERROR();
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        CHECK_GL_ERROR();
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        CHECK_GL_ERROR();
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        CHECK_GL_ERROR();
    }
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    CHECK_GL_ERROR();
    DebugRAM("cubemap textures init");
    
    // Text Initialization
    glCreateBuffers(1, &textVBO);
    CHECK_GL_ERROR();
    glCreateVertexArrays(1, &textVAO);    
    CHECK_GL_ERROR();
    glNamedBufferData(textVBO, sizeof(textQuadVertices), textQuadVertices, GL_STATIC_DRAW);
    CHECK_GL_ERROR();
    glEnableVertexArrayAttrib(textVAO, 0); // DSA: Enable position attribute
    CHECK_GL_ERROR();
    glEnableVertexArrayAttrib(textVAO, 1); // DSA: Enable texture coordinate attribute
    CHECK_GL_ERROR();
    glVertexArrayAttribFormat(textVAO, 0, 2, GL_FLOAT, GL_FALSE, 0); // DSA: Position (x, y)
    CHECK_GL_ERROR();
    glVertexArrayAttribFormat(textVAO, 1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float)); // DSA: Texcoord (u, v)
    CHECK_GL_ERROR();
    glVertexArrayVertexBuffer(textVAO, 0, textVBO, 0, 4 * sizeof(float)); // DSA: Link VBO to VAO
    CHECK_GL_ERROR();
    glVertexArrayAttribBinding(textVAO, 0, 0); // DSA: Bind position to binding index 0
    CHECK_GL_ERROR();
    glVertexArrayAttribBinding(textVAO, 1, 0); // DSA: Bind texcoord to binding index 0
    CHECK_GL_ERROR();
    font = TTF_OpenFont("./Fonts/SystemShockText.ttf", 12);
    if (!font) { DualLogError("TTF_OpenFont failed: %s\n", TTF_GetError()); return SYS_TTF + 1; }
    DebugRAM("text init");
    
    // Input
    Input_Init();
    systemInitialized[SYS_NET] = InitializeNetworking() == 0;
    
    // Audio
    InitializeAudio();
    DebugRAM("audio init");
    systemInitialized[SYS_AUD] = true;
    
    // Load Game/Mod Definitio
    DualLog("Loading game definition from ./Data/gamedata.txt...\n");
    parser_init(&gamedata_parser, valid_gamedata_keys, NUM_GAMDAT_KEYS, PARSER_DATA);
    if (!parse_data_file(&gamedata_parser, "./Data/gamedata.txt")) { DualLogError("Could not parse ./Data/gamedata.txt!\n"); parser_free(&gamedata_parser); return 1; }
    printf("Made it to here after parse 1\n");
    numLevels = gamedata_parser.entries[0].levelCount;
    printf("Made it to here after parse 2\n");
    startLevel = gamedata_parser.entries[0].startLevel;
    printf("Made it to here after parse 3\n");
    DualLog("Game Definition:: num levels: %d, start level=%d",numLevels,startLevel);
    parser_free(&gamedata_parser);
    
    DebugRAM("InitializeEnvironment end");
    return 0;
}

int ExitCleanup(int status) { // Ifs allow deinit from anywhere, only as needed.
    if (systemInitialized[SYS_AUD]) CleanupAudio();
    if (systemInitialized[SYS_NET]) CleanupNetworking();
    if (activeLogFile) fclose(activeLogFile); // Close log playback file.
    if (colorBufferID) glDeleteBuffers(1, &colorBufferID);
    for (uint32_t i=0;i<MODEL_COUNT;i++) {        
        if (vbos[i]) glDeleteBuffers(1, &vbos[i]);
        if (tbos[i]) glDeleteBuffers(1, &tbos[i]);
        if (ebos[i]) glDeleteBuffers(1, &ebos[i]);
        vbos[i] = 0;
        tbos[i] = 0;
        ebos[i] = 0;
    }

    if (vao_chunk) glDeleteVertexArrays(1, &vao_chunk);
    if (chunkShaderProgram) glDeleteProgram(chunkShaderProgram);
    if (textVAO) glDeleteVertexArrays(1, &textVAO);
    if (textVBO) glDeleteBuffers(1, &textVBO);
    if (textShaderProgram) glDeleteProgram(textShaderProgram);
    if (quadVAO) glDeleteVertexArrays(1, &quadVAO);
    if (quadVBO) glDeleteBuffers(1, &quadVBO);
    if (imageBlitShaderProgram) glDeleteProgram(imageBlitShaderProgram);
    if (screenSpaceShadowsComputeShader) glDeleteProgram(screenSpaceShadowsComputeShader);
    if (inputImageID) glDeleteTextures(1,&inputImageID);
    if (inputDepthID) glDeleteTextures(1,&inputDepthID);
    if (inputWorldPosID) glDeleteTextures(1,&inputWorldPosID);
    if (gBufferFBO) glDeleteFramebuffers(1, &gBufferFBO);
    if (sphoxelsID) glDeleteBuffers(1, &sphoxelsID);
    if (visibleLightsID) glDeleteBuffers(1, &visibleLightsID);
    if (instancesBuffer) glDeleteBuffers(1, &instancesBuffer);
    if (instancesInPVSBuffer) glDeleteBuffers(1, &instancesInPVSBuffer);
    if (matricesBuffer) glDeleteBuffers(1, &matricesBuffer);
    if (vboMasterTable) glDeleteBuffers(1, &vboMasterTable);
    if (modelVertexOffsetsID) glDeleteBuffers(1, &modelVertexOffsetsID);
    if (modelVertexCountsID) glDeleteBuffers(1, &modelVertexCountsID);
    if (modelBoundsID) glDeleteBuffers(1, &modelBoundsID);
    if (systemInitialized[SYS_CTX]) SDL_GL_DeleteContext(gl_context); // Delete context after deleting buffers relevant to that context.
    if (systemInitialized[SYS_WIN]) SDL_DestroyWindow(window);
    if (font && systemInitialized[SYS_TTF]) TTF_CloseFont(font);
    if (systemInitialized[SYS_TTF]) TTF_Quit();
    if (systemInitialized[SYS_SDL]) SDL_Quit();
    if (console_log_file) { fclose(console_log_file); console_log_file = NULL; }
    return status;
}

// All core engine operations run through the EventExecute as an Event processed
// by the unified event system in the order it was enqueued.
int EventExecute(Event* event) {
    switch(event->type) {
        case EV_INIT: return InitializeEnvironment(); // Init called prior to Loading Data
        case EV_LOAD_TEXTURES: return LoadTextures();
        case EV_LOAD_MODELS: return LoadGeometry();
        case EV_LOAD_ENTITIES: return LoadEntities();
        case EV_LOAD_LEVELS: /*LoadLevels(); TODO*/ return 0;
        case EV_LOAD_VOXELS: VXGI_Init(); return 0;
        case EV_LOAD_INSTANCES: return SetupInstances();
        case EV_KEYDOWN: return Input_KeyDown(event->payload1u);
        case EV_KEYUP: return Input_KeyUp(event->payload1u);
        case EV_MOUSEMOVE: return Input_MouseMove(event->payload1f,event->payload2f);
        case EV_QUIT: return 1; break;
    }

    return 99; // Something went wrong
}

int main(int argc, char* argv[]) {
    console_log_file = fopen("voxen.log", "w"); // Initialize log system for all prints to go to both stdout and voxen.log file
    if (!console_log_file) DualLogError("Failed to open log file voxen.log\n");
    
    if (argc >= 2 && (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0)) { cli_args_print_version(); return 0; }
    if (argc >= 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) { cli_args_print_help(); return 0; }
    if (argc == 3 && strcmp(argv[1], "dump") == 0) { DualLog("Converting log to plaintext: %s ...", argv[2]); JournalDump(argv[2]); DualLog("DONE!\n"); return 0; }
    if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        cli_args_print_help();
        return 0;
    } else if (argc == 3 && strcmp(argv[1], "dump") == 0) { // Log dump as text
        DualLog("Converting log: %s ...", argv[2]);
        JournalDump(argv[2]);
        DualLog("DONE!\n");
        return 0;
    }


    int exitCode = 0;
    globalFrameNum = 0;
    activeLogFile = 0;
    DebugRAM("prior to event system init");
    exitCode = EventInit();
    if (exitCode) return ExitCleanup(exitCode);

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
    
    DebugRAM("prior init events queued");

    // Queue order for loading is IMPORTANT!
    EnqueueEvent_Simple(EV_INIT);
    EnqueueEvent_Simple(EV_LOAD_TEXTURES);
    EnqueueEvent_Simple(EV_LOAD_MODELS);
    EnqueueEvent_Simple(EV_LOAD_ENTITIES); // Must be after models and textures
                                           // else entity types can't be validated.
    EnqueueEvent_Simple(EV_LOAD_LEVELS); // Must be after entities or else 
                                         // instances can't know what to do.
    EnqueueEvent_Simple(EV_LOAD_INSTANCES);
    EnqueueEvent_Simple(EV_LOAD_VOXELS); // Must be after models! Needed for 
                                         // generating voxels), entities (lets 
                                         // instances know what models), and
                                         // levels (populates instances.
    double accumulator = 0.0;
    last_time = get_time();
    lastJournalWriteTime = get_time();
    DebugRAM("prior to game loop");
    while(1) {
        start_frame_time = get_time();

        // Enqueue input events
        SDL_Event event;
        float mouse_xrel = 0.0f, mouse_yrel = 0.0f;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) EnqueueEvent_Simple(EV_QUIT); // [x] button
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    if (log_playback) {
                        log_playback = false;
                        DualLog("Exited log playback manually.  Control returned\n");                           
                    } else EnqueueEvent_Simple(EV_QUIT); // <<< THAT"S ALL FOLKS!
                } else {
                    if (!log_playback) {
                        EnqueueEvent_Uint(EV_KEYDOWN,(uint32_t)event.key.keysym.scancode);
                    } else {
                        // Handle pause, rewind, fastforward of logs here
                    }
                }
            } else if (event.type == SDL_KEYUP && !log_playback) {
                EnqueueEvent_Uint(EV_KEYUP,(uint32_t)event.key.keysym.scancode);
            } else if (event.type == SDL_MOUSEMOTION && window_has_focus && !log_playback) {
                mouse_xrel += event.motion.xrel;
                mouse_yrel += event.motion.yrel;

            // These aren't really events so just handle them here.
            } else if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
                    window_has_focus = true;
                    SDL_SetRelativeMouseMode(SDL_TRUE);
                } else if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                    window_has_focus = false;
                    SDL_SetRelativeMouseMode(SDL_FALSE);
                }
            }
        }
        
        // After polling, enqueue a single mouse motion event if there was movement
        if (!log_playback && (mouse_xrel != 0.0f || mouse_yrel != 0.0f)) {
            EnqueueEvent_FloatFloat(EV_MOUSEMOVE, mouse_xrel, mouse_yrel);
        }
        
        current_time = get_time();
        double frame_time = current_time - last_time;
        accumulator += frame_time;
        while (accumulator >= time_step) {
            if (window_has_focus) ProcessInput();
            accumulator -= time_step;
        }

        // Enqueue all logged events for the current frame.
        if (log_playback) {
            // Read the log file for current frame and enqueue events from log.
            int read_status = ReadActiveLog();
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
        exitCode = EventQueueProcess(); // Do everything
        if (exitCode) break;
        
        // Client Actions
        // ====================================================================
        // Client Render
        bool debugRenderSegfaults = false;//true;
        drawCallsRenderedThisFrame = 0; // Reset per frame
        shadowDrawCallsRenderedThisFrame = 0;
        verticesRenderedThisFrame = 0;
        
        // 0. Clear Frame Buffers and Depth
        if (debugRenderSegfaults) DualLog("0. Clear Frame Buffers and Depth\n");
        glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
        CHECK_GL_ERROR();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear main FBO
        CHECK_GL_ERROR();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        CHECK_GL_ERROR();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear screen
        CHECK_GL_ERROR();
        
        // Test Stuff DELETE ME LATER, Update the test light to be "attached" to the testLight point moved by j,k,u,i,n,m
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
        instances[39].posx = testLight_x;
        instances[39].posy = testLight_y;
        instances[39].posz = testLight_z;
        instances[39].sclx = testLight_range * 0.04f;
        instances[39].scly = testLight_range * 0.04f;
        instances[39].sclz = testLight_range * 0.04f;
        dirtyInstances[39] = true;
        
        // 1. Light Culling to limit of MAX_VISIBLE_LIGHTS
        if (debugRenderSegfaults) DualLog("1. Light Culling to limit of MAX_VISIBLE_LIGHTS\n");
        playerCellIdx = PositionToWorldCellIndex(cam_x, cam_y, cam_z);
        playerCellIdx_x = PositionToWorldCellIndexX(cam_x);
        playerCellIdx_y = PositionToWorldCellIndexY(cam_y);
        playerCellIdx_z = PositionToWorldCellIndexZ(cam_z);
        numLightsFound = 0;
        for (uint16_t i=0;i<LIGHT_COUNT;++i) {
            uint16_t litIdx = (i * LIGHT_DATA_SIZE);
            float litIntensity = lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY];
            if (litIntensity < 0.015f) continue; // Off
            
            float litx = lights[litIdx + LIGHT_DATA_OFFSET_POSX];
            float lity = lights[litIdx + LIGHT_DATA_OFFSET_POSY];
            float litz = lights[litIdx + LIGHT_DATA_OFFSET_POSZ];
            if (squareDistance3D(cam_x, cam_y, cam_z, litx, lity, litz) < sightRangeSquared) {
                
                uint16_t idx = numLightsFound * LIGHT_DATA_SIZE;
                lightsInProximity[idx + LIGHT_DATA_OFFSET_POSX] = litx;
                lightsInProximity[idx + LIGHT_DATA_OFFSET_POSY] = lity;
                lightsInProximity[idx + LIGHT_DATA_OFFSET_POSZ] = litz;
                lightsInProximity[idx + LIGHT_DATA_OFFSET_INTENSITY] = litIntensity;
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
        
        for (uint8_t i=numLightsFound;i<MAX_VISIBLE_LIGHTS;++i) {
            lightsInProximity[(i * LIGHT_DATA_SIZE) + LIGHT_DATA_OFFSET_INTENSITY] = 0.0f;
        }
        
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, visibleLightsID);
        CHECK_GL_ERROR();
        glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_VISIBLE_LIGHTS * LIGHT_DATA_SIZE * sizeof(float), lightsInProximity, GL_DYNAMIC_DRAW);
        CHECK_GL_ERROR();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 19, visibleLightsID);
        CHECK_GL_ERROR();
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        CHECK_GL_ERROR();
        
        if (firstLightGen && globalFrameNum > 2) {
            for (uint8_t i=0;i<MAX_VISIBLE_LIGHTS;++i) lightDirty[i] = true;
            firstLightGen = false;
        }
                
        // 2. Instance Culling to only those in range of player
        if (debugRenderSegfaults) DualLog("2. Instance Culling to only those in range of player\n");
        bool instanceIsCulledArray[INSTANCE_COUNT];
        memset(instanceIsCulledArray,true,INSTANCE_COUNT * sizeof(bool)); // All culled.
        float distSqrd = 0.0f;
        for (uint16_t i=0;i<INSTANCE_COUNT;++i) {
            if (!instanceIsCulledArray[i]) continue; // Already marked as visible.
            
            distSqrd = squareDistance3D(instances[i].posx,instances[i].posy,instances[i].posz,cam_x, cam_y, cam_z);
            if (distSqrd < sightRangeSquared) instanceIsCulledArray[i] = false;
        }
        
        uint16_t instancesInPVSCount = 0;
        for (uint16_t i=0;i<INSTANCE_COUNT;++i) {
            if (!instanceIsCulledArray[i]) instancesInPVSCount++;
            if (instancesInPVSCount >= INSTANCE_COUNT - 1) break;
        }
        
        uint32_t instancesInPVS[instancesInPVSCount];
        uint32_t curIdx = 0;
        for (uint16_t i=0;i<INSTANCE_COUNT;++i) {
            if (!instanceIsCulledArray[i]) {
                instancesInPVS[curIdx] = i;
                curIdx++;
            }
        }

        // 3. Pass all instance matrices to GPU
        if (debugRenderSegfaults) DualLog("3. Pass all instance matrices to GPU\n");
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, matricesBuffer);
        CHECK_GL_ERROR();
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, INSTANCE_COUNT * 16 * sizeof(float), modelMatrices); // * 16 because matrix4x4
        CHECK_GL_ERROR();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, matricesBuffer);
        CHECK_GL_ERROR();
        
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, instancesInPVSBuffer);
        CHECK_GL_ERROR();
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, instancesInPVSCount * sizeof(uint32_t), instancesInPVS); // * 16 because matrix4x4
        CHECK_GL_ERROR();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, instancesInPVSBuffer);
        CHECK_GL_ERROR();
        
        glEnable(GL_BLEND);
        CHECK_GL_ERROR();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        CHECK_GL_ERROR();
    
        // 4. Generate Shadow Data - Render all meshes as black + glow, render all lights as spheres with their color and scaled for intensity
        if (globalFrameNum >= 2 && !generatedCubemapProbes) {
            Quaternion orientations[6];
            
            quat_identity(&orientations[0]);
            quat_identity(&orientations[1]);
            quat_identity(&orientations[2]);
            quat_identity(&orientations[3]);
            quat_identity(&orientations[4]);
            quat_identity(&orientations[5]);
            quat_from_yaw_pitch(&orientations[0], 180.0f,  90.0f); // Front Z+ (actually Y+) As looking along axis specified, value gets more this if going into screen.
            quat_from_yaw_pitch(&orientations[1],   0.0f,  90.0f); //  Back Z- (actually Y-)
            quat_from_yaw_pitch(&orientations[2],  90.0f,  90.0f); // Right X+ (actually X+)
            quat_from_yaw_pitch(&orientations[3], 270.0f,  90.0f); //  Left X- (actually X-)
            quat_from_yaw_pitch(&orientations[4], 180.0f,   0.0f); //    Up Y+ (actually Z+)
            quat_from_yaw_pitch(&orientations[5], 180.0f, 180.0f); //  Down Y- (actually Z-)
            glBindFramebuffer(GL_FRAMEBUFFER, gBufferCubemapFBO);
            CHECK_GL_ERROR();
            glViewport(0, 0, cubemapTexSize, cubemapTexSize);
            CHECK_GL_ERROR();
            glUseProgram(chunkShaderProgram);
            CHECK_GL_ERROR();
            glEnable(GL_DEPTH_TEST);
            CHECK_GL_ERROR();
            glBindImageTexture(0, inputCubemapImageID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
            CHECK_GL_ERROR();
            glBindImageTexture(1, inputCubemapWorldPosID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
            CHECK_GL_ERROR();
            for (int i=0; i < 6; ++i) {
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear color and depth
                CHECK_GL_ERROR();
                float view[16], projection[16]; // Set up view and projection matrices
                float fov = 65.0f;
                mat4_perspective(projection, fov, (float)cubemapTexSize / cubemapTexSize, 0.02f, 15.36f);
                mat4_lookat(view, 0.0f, 0.0f, 2.0f, &orientations[i]);
                glUniformMatrix4fv(viewLoc_chunk,       1, GL_FALSE,       view);
                CHECK_GL_ERROR();
                glUniformMatrix4fv(projectionLoc_chunk, 1, GL_FALSE, projection);
                CHECK_GL_ERROR();
                glUniform1i(debugViewLoc_chunk, 6); // Force all to render as black + glow
                CHECK_GL_ERROR();
                glUniform1i(instancesInPVSCount_chunk, instancesInPVSCount);
                CHECK_GL_ERROR();
                glUniform1i(shadowsEnabledLoc_chunk, true);
                CHECK_GL_ERROR();
                glBindVertexArray(vao_chunk);
                CHECK_GL_ERROR();
                glUniform1f(overrideGlowRLoc_chunk, 0.0f);
                CHECK_GL_ERROR();
                glUniform1f(overrideGlowGLoc_chunk, 0.0f);
                CHECK_GL_ERROR();
                glUniform1f(overrideGlowBLoc_chunk, 0.0f);
                CHECK_GL_ERROR();
                for (uint16_t i=0;i<INSTANCE_COUNT;i++) {
                    if (instanceIsCulledArray[i]) continue;
                    if (instances[i].modelIndex >= MODEL_COUNT) continue;
                    if (modelVertexCounts[instances[i].modelIndex] < 1) continue; // Empty model
                    if (instances[i].modelIndex < 0) continue; // Culled
                    if (i == 39) continue; // TODO delete me
                    
                    if (dirtyInstances[i]) UpdateInstanceMatrix(i);            
                    glUniform1i(texIndexLoc_chunk, instances[i].texIndex);
                    CHECK_GL_ERROR();
                    glUniform1i(glowIndexLoc_chunk, instances[i].glowIndex);
                    CHECK_GL_ERROR();
                    glUniform1i(specIndexLoc_chunk, instances[i].specIndex);
                    CHECK_GL_ERROR();
                    glUniform1i(instanceIndexLoc_chunk, i);
                    CHECK_GL_ERROR();
                    int modelType = instances[i].modelIndex;
                    glUniform1i(modelIndexLoc_chunk, modelType);
                    CHECK_GL_ERROR();
                    glUniformMatrix4fv(matrixLoc_chunk, 1, GL_FALSE, &modelMatrices[i * 16]);
                    CHECK_GL_ERROR();
                    glBindVertexBuffer(0, vbos[modelType], 0, VERTEX_ATTRIBUTES_COUNT * sizeof(float));
                    CHECK_GL_ERROR();
                    if (isDoubleSided(instances[i].texIndex)) glDisable(GL_CULL_FACE); // Disable backface culling
                    glDrawArrays(GL_TRIANGLES, 0, modelVertexCounts[modelType]);
                    CHECK_GL_ERROR();
                    if (isDoubleSided(instances[i].texIndex)) glEnable(GL_CULL_FACE); // Reenable backface culling
                    drawCallsRenderedThisFrame++;
                    verticesRenderedThisFrame += modelVertexCounts[modelType];
                }
                
                glEnable(GL_CULL_FACE); // Reenable backface culling
                glEnable(GL_DEPTH_TEST);
                for (uint16_t i=1;i<numLightsFound;++i) { // skip self, start at 1 TODO: Use cubemap array index
                    float mat[16]; // 4x4 matrix
                    uint16_t idx = i * LIGHT_DATA_SIZE;
                    float sphoxelSize = lightsInProximity[idx + LIGHT_DATA_OFFSET_RANGE] * 0.04f; // Const.segiVoxelSize from Citadel main
                    if (sphoxelSize > 8.0f) sphoxelSize = 8.0f;
                    SetUpdatedMatrix(mat, lightsInProximity[idx + LIGHT_DATA_OFFSET_POSX], lightsInProximity[idx + LIGHT_DATA_OFFSET_POSY], lightsInProximity[idx + LIGHT_DATA_OFFSET_POSZ], 0.0f, 0.0f, 0.0f, 1.0f, sphoxelSize, sphoxelSize, sphoxelSize);
                    glUniform1f(overrideGlowRLoc_chunk, lightsInProximity[idx + LIGHT_DATA_OFFSET_R] * lightsInProximity[idx + LIGHT_DATA_OFFSET_INTENSITY]);
                    CHECK_GL_ERROR();
                    glUniform1f(overrideGlowGLoc_chunk, lightsInProximity[idx + LIGHT_DATA_OFFSET_G] * lightsInProximity[idx + LIGHT_DATA_OFFSET_INTENSITY]);
                    CHECK_GL_ERROR();
                    glUniform1f(overrideGlowBLoc_chunk, lightsInProximity[idx + LIGHT_DATA_OFFSET_B] * lightsInProximity[idx + LIGHT_DATA_OFFSET_INTENSITY]);
                    CHECK_GL_ERROR();
                    glUniform1i(texIndexLoc_chunk, 41);
                    CHECK_GL_ERROR();
                    glUniform1i(glowIndexLoc_chunk, 41);
                    CHECK_GL_ERROR();
                    glUniform1i(specIndexLoc_chunk, 41);
                    CHECK_GL_ERROR();
                    glUniform1i(instanceIndexLoc_chunk, i);
                    CHECK_GL_ERROR();
                    int modelType = 621; // Test light icosphere
                    glUniform1i(modelIndexLoc_chunk, modelType);
                    CHECK_GL_ERROR();
                    glUniformMatrix4fv(matrixLoc_chunk, 1, GL_FALSE, mat);
                    CHECK_GL_ERROR();
                    glBindVertexBuffer(0, vbos[modelType], 0, VERTEX_ATTRIBUTES_COUNT * sizeof(float));
                    CHECK_GL_ERROR();
                    glDrawArrays(GL_TRIANGLES, 0, modelVertexCounts[modelType]);
                    CHECK_GL_ERROR();
                    drawCallsRenderedThisFrame++;
                    verticesRenderedThisFrame += modelVertexCounts[modelType];
                }
                
                ScreenshotCubemapFace(i);
            }
            
            generatedCubemapProbes = true;

            // Restore state
            glUniform1i(debugViewLoc_chunk, debugView);
            CHECK_GL_ERROR();
            glViewport(0, 0, screen_width, screen_height);
            CHECK_GL_ERROR();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear main FBO
            CHECK_GL_ERROR();
            glBindImageTexture(0, inputImageID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8); // Put normal textures back
            CHECK_GL_ERROR();
            glBindImageTexture(1, inputWorldPosID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
            CHECK_GL_ERROR();
            glBindImageTexture(2, inputShadowStencilID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R8);
            CHECK_GL_ERROR();
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            CHECK_GL_ERROR();
            malloc_trim(0);
        }
        
        // 5. Unlit Raterized Geometry
        //        Standard vertex + fragment rendering, but with special packing to minimize transfer data amounts
        if (debugRenderSegfaults) DualLog("5. Unlit Raterized Geometry\n");
        glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
        CHECK_GL_ERROR();
        glUseProgram(chunkShaderProgram);
        CHECK_GL_ERROR();
        glEnable(GL_DEPTH_TEST);
        CHECK_GL_ERROR();
        glEnable(GL_BLEND);
        CHECK_GL_ERROR();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        CHECK_GL_ERROR();
        float view[16], projection[16]; // Set up view and projection matrices
        float fov = 65.0f;
        mat4_perspective(projection, fov, (float)screen_width / screen_height, 0.02f, 100.0f);
        mat4_lookat(view, cam_x, cam_y, cam_z, &cam_rotation);
        glUniformMatrix4fv(viewLoc_chunk,       1, GL_FALSE,       view);
        CHECK_GL_ERROR();
        glUniformMatrix4fv(projectionLoc_chunk, 1, GL_FALSE, projection);
        CHECK_GL_ERROR();
        glUniform1i(debugViewLoc_chunk, debugView);
        CHECK_GL_ERROR();
        glUniform1i(instancesInPVSCount_chunk, instancesInPVSCount);
        CHECK_GL_ERROR();
        glUniform1i(shadowsEnabledLoc_chunk, shadowsEnabled);
        CHECK_GL_ERROR();
        glBindVertexArray(vao_chunk);
        CHECK_GL_ERROR();
        glUniform1f(overrideGlowRLoc_chunk, 0.0f);
        CHECK_GL_ERROR();
        glUniform1f(overrideGlowGLoc_chunk, 0.0f);
        CHECK_GL_ERROR();
        glUniform1f(overrideGlowBLoc_chunk, 0.0f);
        CHECK_GL_ERROR();
        for (uint16_t i=0;i<INSTANCE_COUNT;i++) {
            if (instanceIsCulledArray[i]) continue;
            if (instances[i].modelIndex >= MODEL_COUNT) continue;
            if (modelVertexCounts[instances[i].modelIndex] < 1) continue; // Empty model
            if (instances[i].modelIndex < 0) continue; // Culled
            if (debugView == 6 && i == 39) continue; // TODO: Delete me
            
            if (dirtyInstances[i]) UpdateInstanceMatrix(i);            
            glUniform1i(texIndexLoc_chunk, instances[i].texIndex);
            CHECK_GL_ERROR();
            glUniform1i(glowIndexLoc_chunk, instances[i].glowIndex);
            CHECK_GL_ERROR();
            glUniform1i(specIndexLoc_chunk, instances[i].specIndex);
            CHECK_GL_ERROR();
            glUniform1i(instanceIndexLoc_chunk, i);
            CHECK_GL_ERROR();
            int modelType = instances[i].modelIndex;
            glUniform1i(modelIndexLoc_chunk, modelType);
            CHECK_GL_ERROR();
            glUniformMatrix4fv(matrixLoc_chunk, 1, GL_FALSE, &modelMatrices[i * 16]);
            CHECK_GL_ERROR();
            glBindVertexBuffer(0, vbos[modelType], 0, VERTEX_ATTRIBUTES_COUNT * sizeof(float));
            CHECK_GL_ERROR();
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tbos[modelType]);
            CHECK_GL_ERROR();
            if (isDoubleSided(instances[i].texIndex)) glDisable(GL_CULL_FACE); // Disable backface culling
//             glDrawArrays(GL_TRIANGLES, 0, modelVertexCounts[modelType]);
            glDrawElements(GL_TRIANGLES, modelTriangleCounts[modelType] * 3, GL_UNSIGNED_INT, 0);
            CHECK_GL_ERROR();
            if (isDoubleSided(instances[i].texIndex)) glEnable(GL_CULL_FACE); // Reenable backface culling
            drawCallsRenderedThisFrame++;
            verticesRenderedThisFrame += modelTriangleCounts[modelType] * 3;
        }
        
        glEnable(GL_CULL_FACE); // Reenable backface culling
        glEnable(GL_DEPTH_TEST);
        if (debugView == 6) { // Render Light Spheres
           for (uint16_t i=0;i<numLightsFound;++i) {
                float mat[16]; // 4x4 matrix
                uint16_t idx = i * LIGHT_DATA_SIZE;
                float sphoxelSize = lightsInProximity[idx + LIGHT_DATA_OFFSET_RANGE] * 0.04f; // Const.segiVoxelSize from Citadel main
                if (sphoxelSize > 8.0f) sphoxelSize = 8.0f;
                SetUpdatedMatrix(mat, lightsInProximity[idx + LIGHT_DATA_OFFSET_POSX], lightsInProximity[idx + LIGHT_DATA_OFFSET_POSY], lightsInProximity[idx + LIGHT_DATA_OFFSET_POSZ], 0.0f, 0.0f, 0.0f, 1.0f, sphoxelSize, sphoxelSize, sphoxelSize);
                glUniform1f(overrideGlowRLoc_chunk, lightsInProximity[idx + LIGHT_DATA_OFFSET_R] * lightsInProximity[idx + LIGHT_DATA_OFFSET_INTENSITY]);
                CHECK_GL_ERROR();
                glUniform1f(overrideGlowGLoc_chunk, lightsInProximity[idx + LIGHT_DATA_OFFSET_G] * lightsInProximity[idx + LIGHT_DATA_OFFSET_INTENSITY]);
                CHECK_GL_ERROR();
                glUniform1f(overrideGlowBLoc_chunk, lightsInProximity[idx + LIGHT_DATA_OFFSET_B] * lightsInProximity[idx + LIGHT_DATA_OFFSET_INTENSITY]);
                CHECK_GL_ERROR();
                glUniform1i(texIndexLoc_chunk, 41);
                CHECK_GL_ERROR();
                glUniform1i(glowIndexLoc_chunk, 41);
                CHECK_GL_ERROR();
                glUniform1i(specIndexLoc_chunk, 41);
                CHECK_GL_ERROR();
                glUniform1i(instanceIndexLoc_chunk, i);
                CHECK_GL_ERROR();
                int modelType = 621; // Test light icosphere
                glUniform1i(modelIndexLoc_chunk, modelType);
                CHECK_GL_ERROR();
                glUniformMatrix4fv(matrixLoc_chunk, 1, GL_FALSE, mat);
                CHECK_GL_ERROR();
                glBindVertexBuffer(0, vbos[modelType], 0, VERTEX_ATTRIBUTES_COUNT * sizeof(float));
                CHECK_GL_ERROR();
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tbos[modelType]);
                CHECK_GL_ERROR();
//                 glDrawArrays(GL_TRIANGLES, 0, modelVertexCounts[modelType]);
                glDrawElements(GL_TRIANGLES, modelTriangleCounts[modelType] * 3, GL_UNSIGNED_INT, 0);
                CHECK_GL_ERROR();
                drawCallsRenderedThisFrame++;
                verticesRenderedThisFrame += modelTriangleCounts[modelType] * 3;
           }
        }
        
        glDisable(GL_BLEND);
        CHECK_GL_ERROR();
        
        // ====================================================================
        // Ok, turn off temporary framebuffer so we can draw to screen now.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        CHECK_GL_ERROR();
        // ====================================================================
        
        // 6. Screenspace Shadows (SSS) Compute Shader
//         if (debugRenderSegfaults) DualLog("7. Screenspace Shadows (SSS) Compute Shader\n");
//         glBindTexture(GL_TEXTURE_2D, inputShadowStencilID);
//         glClearTexImage(inputShadowStencilID, 0, GL_RED, GL_FLOAT, (float[]){0.0f});
//         glBindTexture(GL_TEXTURE_2D, 0);
// 
//         glUseProgram(screenSpaceShadowsComputeShader);
//         CHECK_GL_ERROR();
//         glUniform2f(glGetUniformLocation(screenSpaceShadowsComputeShader, "uScreenSize"), (float)screen_width, (float)screen_height);
//         CHECK_GL_ERROR();
//         float viewProj[16];
//         mat4_identity(viewProj);
//         glUniformMatrix4fv(glGetUniformLocation(screenSpaceShadowsComputeShader, "projection"), 1, GL_FALSE, projection);
//         CHECK_GL_ERROR();
//         glUniformMatrix4fv(glGetUniformLocation(screenSpaceShadowsComputeShader, "matrix"), 1, GL_FALSE, viewProj);
//         CHECK_GL_ERROR();
//         glUniformMatrix4fv(glGetUniformLocation(screenSpaceShadowsComputeShader, "view"), 1, GL_FALSE, view);
//         CHECK_GL_ERROR();
//         glUniform1f(glGetUniformLocation(screenSpaceShadowsComputeShader, "playerPosX"), 0.1f);
//         CHECK_GL_ERROR();
//         glUniform1f(glGetUniformLocation(screenSpaceShadowsComputeShader, "playerPosY"), 0.1f);
//         CHECK_GL_ERROR();
//         glUniform1f(glGetUniformLocation(screenSpaceShadowsComputeShader, "playerPosZ"), 0.1f);
//         CHECK_GL_ERROR();
//         GLuint groupX = (screen_width + 31) / 32;
//         GLuint groupY = (screen_height + 31) / 32;
//         glDispatchCompute(groupX, groupY, 1); // Dispatch compute shader
//         CHECK_GL_ERROR();
//         glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT); // Runs slightly faster 0.1ms without this, but may need if more shaders added in between
//         CHECK_GL_ERROR();
//         glUseProgram(0);
//         CHECK_GL_ERROR();
        
        // 6. Render final meshes' results with full screen quad
        if (debugRenderSegfaults) DualLog("7. Render final meshes' results with full screen quad\n");
        glUseProgram(imageBlitShaderProgram);
        CHECK_GL_ERROR();
        glActiveTexture(GL_TEXTURE0);
        CHECK_GL_ERROR();
        glBindTextureUnit(0, inputImageID); // Normal
        CHECK_GL_ERROR();
        glProgramUniform1i(imageBlitShaderProgram, texLoc_quadblit, 0);
        CHECK_GL_ERROR();
        glProgramUniform1i(imageBlitShaderProgram, debugViewLoc_quadblit, debugView);
        CHECK_GL_ERROR();
        glProgramUniform1i(imageBlitShaderProgram, debugValueLoc_quadblit, debugValue);
        CHECK_GL_ERROR();
        glBindVertexArray(quadVAO);
        CHECK_GL_ERROR();
        glDisable(GL_BLEND);
        CHECK_GL_ERROR();
        glDisable(GL_DEPTH_TEST);
        CHECK_GL_ERROR();
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        CHECK_GL_ERROR();
        glEnable(GL_DEPTH_TEST);
        CHECK_GL_ERROR();
        glBindTextureUnit(0, 0);
        CHECK_GL_ERROR();
        glUseProgram(0);
        CHECK_GL_ERROR();
        uint32_t drawCallsNormal = drawCallsRenderedThisFrame;
        
        // 7. Render UI Images
        if (debugRenderSegfaults) DualLog("8. Render UI Images\n");
 
        // 8. Render UI Text
        if (debugRenderSegfaults) DualLog("9. Render UI Text\n");
        glEnable(GL_STENCIL_TEST);
        CHECK_GL_ERROR();
        glStencilMask(0xFF); // Enable stencil writes
        CHECK_GL_ERROR();
        glStencilFunc(GL_ALWAYS, 1, 0xFF); // Always write 1
        CHECK_GL_ERROR();
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); // Replace stencil value
        CHECK_GL_ERROR();
        glClear(GL_STENCIL_BUFFER_BIT); // Clear stencil each frame TODO: Ok to do in ClearFrameBuffers()??
        CHECK_GL_ERROR();

        float cam_quat_yaw = 0.0f;
        float cam_quat_pitch = 0.0f;
        float cam_quat_roll = 0.0f;
        quat_to_euler(&cam_rotation,&cam_quat_yaw,&cam_quat_pitch,&cam_quat_roll);
        
        RenderFormattedText(10, 25, TEXT_WHITE, "x: %.2f, y: %.2f, z: %.2f", cam_x, cam_y, cam_z);
        CHECK_GL_ERROR();
        RenderFormattedText(10, 40, TEXT_WHITE, "cam yaw: %.2f, cam pitch: %.2f, cam roll: %.2f", cam_yaw, cam_pitch, cam_roll);
        CHECK_GL_ERROR();
        RenderFormattedText(10, 55, TEXT_WHITE, "cam quat yaw: %.2f, cam quat pitch: %.2f, cam quat roll: %.2f", cam_quat_yaw, cam_quat_pitch, cam_quat_roll);
        CHECK_GL_ERROR();
        RenderFormattedText(10, 70, TEXT_WHITE, "Peak frame queue count: %d", maxEventCount_debug);
        CHECK_GL_ERROR();
        RenderFormattedText(10, 85, TEXT_WHITE, "testLight intensity: %.4f, range: %.4f, spotAng: %.4f, x: %.3f, y: %.3f, z: %.3f", testLight_intensity,testLight_range,testLight_spotAng,testLight_x,testLight_y,testLight_z);
        CHECK_GL_ERROR();
        RenderFormattedText(10, 100, TEXT_WHITE, "DebugView: %d (%s), DebugValue: %d, Instances in PVS: %d", debugView, debugView == 1 ? "unlit"
                                                                                                : debugView == 2 ? "surface normals"
                                                                                                : debugView == 3 ? "depth"
                                                                                                : debugView == 4 ? "indices"
                                                                                                :(debugView == 5 ? "shadows" : "standard render"), debugValue, instancesInPVSCount);
        CHECK_GL_ERROR();
        RenderFormattedText(10, 115, TEXT_WHITE, "Num lights: %d, Player cell:: x: %d, y: %d, z: %d", numLightsFound, playerCellIdx_x, playerCellIdx_y, playerCellIdx_z);
        CHECK_GL_ERROR();
        
        // Frame stats
        double time_now = get_time();
        drawCallsRenderedThisFrame++; // Add one more for this text render ;)
        RenderFormattedText(10, 10, TEXT_WHITE, "Frame time: %.6f (FPS: %d), Draw calls: %d [Geo %d, Shdw %d, UI %d], Verts: %d, Worst FPS: %d",
                            (time_now - last_time) * 1000.0,framesPerLastSecond,drawCallsRenderedThisFrame,drawCallsNormal - shadowDrawCallsRenderedThisFrame,shadowDrawCallsRenderedThisFrame, drawCallsRenderedThisFrame - drawCallsNormal,verticesRenderedThisFrame,worstFPS);
        last_time = time_now;
        CHECK_GL_ERROR();
        if ((time_now - lastFrameSecCountTime) >= 1.00) {
            lastFrameSecCountTime = time_now;
            framesPerLastSecond = globalFrameNum - lastFrameSecCount;
            if (framesPerLastSecond < worstFPS && globalFrameNum > 10) worstFPS = framesPerLastSecond; // After startup, keep track of worst framerate seen.
            lastFrameSecCount = globalFrameNum;
        }
        
        if (keys[SDL_SCANCODE_F12]) {
            if (time_now > screenshotTimeout) {
                Screenshot();
                screenshotTimeout = time_now + 1.0; // Prevent saving more than 1 per second for sanity purposes.
            }
        }

        glDisable(GL_STENCIL_TEST);
        CHECK_GL_ERROR();
        glStencilMask(0x00); // Disable stencil writes
        CHECK_GL_ERROR();
        SDL_GL_SwapWindow(window); // Present frame
        CHECK_GL_ERROR();
        globalFrameNum++;
    }

    // Cleanup
    return ExitCleanup(exitCode);
}
