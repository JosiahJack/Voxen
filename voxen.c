// File: voxen.c
// Description: A realtime OpenGL based application for experimenting with
// voxel lighting techniques to derive new methods of high speed accurate
// lighting in resource constrained environements (e.g. embedded).
#define VERSION_STRING "v0.4.0"
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
#include <time.h>
#include <enet/enet.h>
#include "event.h"
#include "data_parser.h"
#include "render.h"
#include "text.glsl"
#include "chunk.glsl"
#include "imageblit.glsl"
#include "lightmap.compute"
#include "deferred_lighting.compute"
#include "ssr.compute"
#include "bluenoise64.cginc"
#include "audio.h"
#include "instance.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "External/stb_image_write.h"
#include "constants.h"
#include "dynamic_culling.h"

// #define DEBUG_RAM_OUTPUT

// Window
SDL_Window *window;
int screen_width = 800, screen_height = 600;
bool window_has_focus = false;
FILE* console_log_file = NULL;

// Instances
Instance instances[INSTANCE_COUNT];
float modelMatrices[INSTANCE_COUNT * 16];
uint8_t dirtyInstances[INSTANCE_COUNT];
uint16_t doubleSidedInstances[INSTANCE_COUNT]; // Needs to be large for cyberspace.
uint16_t doubleSidedInstancesHead = 0;
uint16_t transparentInstances[INSTANCE_COUNT]; // Could probably be like 16, ah well.
uint16_t transparentInstancesHead = 0;
GLuint instancesBuffer;
GLuint instancesInPVSBuffer;
GLuint matricesBuffer;

// Game/Mod Definition
const char* global_modname;
bool global_modIsCitadel = false;
uint8_t numLevels = 2;
uint8_t startLevel = 3;
uint8_t currentLevel = 0;
bool gamePaused = false;
bool menuActive = false;
const char* valid_gamedata_keys[] = {"modname","levelcount","startlevel"};
#define NUM_GAMDAT_KEYS 3

// Camera variables
// Start Actual: Puts player on Medical Level in actual game start position
float cam_x = -20.4f, cam_y = -43.792f + 0.84f, cam_z = 10.2f; // Camera position Cornell box, added 0.84f for cam
float cam_yaw = 90.0f;                                         // offset from player capsule center.
float cam_pitch = 0.0f;
float cam_roll = 0.0f;
Quaternion cam_rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
float cam_forwardx = 0.0f, cam_forwardy = 0.0f, cam_forwardz = 0.0f;
float cam_rightx = 0.0f, cam_righty = 0.0f, cam_rightz = 0.0f;
float cam_fov = 65.0f;

float deg2rad(float degrees) { return degrees * (M_PI / 180.0f); }
float rad2deg(float radians) { return radians * (180.0f / M_PI); }

float move_speed = 0.15f;
float mouse_sensitivity = 0.1f;
bool in_cyberspace = true;
float sprinting = 0.0f;
bool noclip = true;

// Input
bool keys[SDL_NUM_SCANCODES] = {0}; // SDL_NUM_SCANCODES 512b, covers all keys
int mouse_x = 0, mouse_y = 0; // Mouse position
int debugView = 0;
int debugValue = 0;

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
double screenshotTimeout = 0.0;
bool journalFirstWrite = true;

// Networking
typedef enum {
    MODE_LISTEN_SERVER,    // Runs both server and client locally
    //MODE_DEDICATED_SERVER, // Server only, no rendering (headless) Currently only using Listen for coop
    MODE_CLIENT            // Client only, connects to a server
} EngineMode;

EngineMode engine_mode = MODE_LISTEN_SERVER; // Default mode
char* server_address = "127.0.0.1"; // Default to localhost for listen server
int server_port = 27015; // Default port

ENetHost* server_host = NULL;
ENetHost* client_host = NULL;
ENetPeer* server_peer = NULL; // Client's connection to server

// ----------------------------------------------------------------------------
// OpenGL / Rendering
SDL_GLContext gl_context;
float uiOrthoProjection[16];
float rasterPerspectiveProjection[16];
float fov = 65.0f;
float nearPlane = 0.02f;
float farPlane = 71.68f;
float sightRangeSquared = 71.68f * 71.68f; // Max player view, level 6 crawlway 28 cells
double lastFrameSecCountTime = 0.00;
uint32_t lastFrameSecCount = 0;
uint32_t framesPerLastSecond = 0;
uint32_t worstFPS = UINT32_MAX;
uint32_t drawCallsRenderedThisFrame = 0; // Total draw calls this frame
uint32_t verticesRenderedThisFrame = 0;
uint8_t numLightsFound = 0;

// Shaders
GLuint chunkShaderProgram;
GLuint vao_chunk; // Vertex Array Object
GLint viewLoc_chunk = -1, projectionLoc_chunk = -1, matrixLoc_chunk = -1, texIndexLoc_chunk = -1,
      instanceIndexLoc_chunk = -1, modelIndexLoc_chunk = -1, debugViewLoc_chunk = -1, glowIndexLoc_chunk = -1,
      specIndexLoc_chunk = -1, instancesInPVSCount_chunk = -1, overrideGlowRLoc_chunk = -1, overrideGlowGLoc_chunk = -1,
      overrideGlowBLoc_chunk = -1;

//    GPU Lightmapper Compute Shader
GLuint lightmapShaderProgram;
GLuint totalLuxelCountLoc_lightmap = -1, instanceCountLoc_lightmap = -1, modelCountLoc_lightmap = -1,
       worldMin_xLoc_lightmap = -1, worldMin_zLoc_lightmap = -1, lightCountLoc_lightmap = -1;

//    Deferred Lighting Compute Shader
GLuint deferredLightingShaderProgram;
GLuint inputImageID, inputNormalsID, inputDepthID, inputWorldPosID, gBufferFBO, outputImageID; // FBO
GLuint precomputedVisibleCellsFromHereID, cellIndexForInstanceID, cellIndexForLightID, masterIndexForLightsInPVSID;
GLint screenWidthLoc_deferred = -1, screenHeightLoc_deferred = -1, debugViewLoc_deferred = -1,
      worldMin_xLoc_deferred = -1, worldMin_zLoc_deferred = -1, cam_xLoc_deferred = -1, cam_yLoc_deferred = -1, cam_zLoc_deferred = -1,
      fogColorRLoc_deferred = -1, fogColorGLoc_deferred = -1, fogColorBLoc_deferred = -1,
      viewProjectionLoc_deferred = -1, modelCountLoc_deferred = -1, totalLuxelCountLoc_deferred = -1;

int ssr_StepCount = 128;
float ssr_MaxDist = 71.68f;
float ssr_StepSize = 0.185f;
float fogColorR = 0.04f;
float fogColorG = 0.04f;
float fogColorB = 0.09f;

//    SSR (Screen Space Reflections)
#define SSR_RES 4 // 25% of render resolution.
GLuint ssrShaderProgram;
GLint screenWidthLoc_ssr = -1, screenHeightLoc_ssr = -1, viewProjectionLoc_ssr = -1, maxDistLoc_ssr = -1, stepSizeLoc_ssr = -1, 
      stepCountLoc_ssr = -1, cam_xLoc_ssr = -1, cam_yLoc_ssr = -1, cam_zLoc_ssr = -1;

//    Full Screen Quad Blit for rendering final output/image effect passes
GLuint imageBlitShaderProgram;
GLuint quadVAO, quadVBO;
GLint texLoc_quadblit = -1, debugViewLoc_quadblit = -1, debugValueLoc_quadblit = -1,
      screenWidthLoc_imageBlit = -1, screenHeightLoc_imageBlit = -1;

// Lights
// Could reduce spotAng to minimal bits.  I only have 6 spot lights and half are 151.7 and other half are 135.
float lights[LIGHT_COUNT * LIGHT_DATA_SIZE] = {0};
float lightsRangeSquared[LIGHT_COUNT] = {0};
bool lightDirty[MAX_VISIBLE_LIGHTS] = { [0 ... MAX_VISIBLE_LIGHTS-1] = true };
GLuint lightVBOs[MAX_VISIBLE_LIGHTS];
GLuint lightIBOs[MAX_VISIBLE_LIGHTS];
uint32_t lightVertexCounts[MAX_VISIBLE_LIGHTS];
uint32_t lightIndexCounts[MAX_VISIBLE_LIGHTS];
float lightsInProximity[MAX_VISIBLE_LIGHTS * LIGHT_DATA_SIZE];
GLuint lightsID;

// Text
#define TEXT_WHITE 0
#define TEXT_YELLOW 1
#define TEXT_DARK_YELLOW 2
#define TEXT_GREEN 3
#define TEXT_RED 4
#define TEXT_ORANGE 5
#define TEXT_BUFFER_SIZE 128
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

static void DualLogMain(FILE *stream, const char *prefix, const char *fmt, va_list args) {
    va_list copy; va_copy(copy, args);
    if (prefix) fprintf(stream, "%s\033[0m", prefix);
    vfprintf(stream, fmt, args);
    fprintf(stream, "\033[0m"); fflush(stream);
    if (console_log_file) {
        if (prefix) fprintf(console_log_file, "%s ", prefix);
        vfprintf(console_log_file, fmt, copy);
        fflush(console_log_file);
    }
    va_end(copy);
}

void DualLog(const char *fmt, ...) { va_list args; va_start(args, fmt); DualLogMain(stdout, NULL, fmt, args); va_end(args); }
void DualLogWarn(const char *fmt, ...) { va_list args; va_start(args, fmt); DualLogMain(stdout, "\033[1;38;5;208mWARN:", fmt, args); va_end(args); }
void DualLogError(const char *fmt, ...) { va_list args; va_start(args, fmt); DualLogMain(stderr, "\033[1;31mERROR:", fmt, args); va_end(args); }


// Get USS aka the total RAM uniquely allocated for the process (btop shows RSS so pulls in shared libs and double counts shared RAM).
void DebugRAM(const char *context, ...) {
    char formatted_context[1024];
    va_list args;
    va_start(args, context);
    vsnprintf(formatted_context, sizeof(formatted_context), context, args);
    va_end(args);

#ifdef DEBUG_RAM_OUTPUT
    struct mallinfo2 info = mallinfo2();
    size_t uss_bytes = 0;
    FILE *fp = fopen("/proc/self/smaps_rollup", "r");
    if (fp) {
        char line[256];
        size_t val;
        while (fgets(line, sizeof(line), fp)) {
            if (sscanf(line, "Private_Clean: %zu kB", &val) == 1)      uss_bytes += val * 1024;
            else if (sscanf(line, "Private_Dirty: %zu kB", &val) == 1) uss_bytes += val * 1024;
        }
        fclose(fp);
    } else {
        DualLogError("Failed to open /proc/self/smaps_rollup\n");
    }

    DualLog("Memory at %s: Heap usage %zu bytes (%zu KB | %.2f MB), USS %zu bytes (%zu KB | %.2f MB)\n",
            formatted_context,
            info.uordblks, info.uordblks / 1024, info.uordblks / 1024.0 / 1024.0,
            uss_bytes, uss_bytes / 1024, uss_bytes / 1024.0 / 1024.0);
#endif
}


void print_bytes_no_newline(int count) {
    DualLog("%d bytes | %f kb | %f Mb",count,(float)count / 1000.0f,(float)count / 1000000.0f);
}

// ============================================================================
// OpenGL / Rendering Helper Functions
void GenerateAndBindTexture(GLuint *id, GLenum internalFormat, int width, int height, GLenum format, GLenum type, GLenum target, const char *name) {
    glGenTextures(1, id);
    glBindTexture(target, *id);
    glTexImage2D(target, 0, internalFormat, width, height, 0, format, type, NULL);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    if (target == GL_TEXTURE_CUBE_MAP) {
        glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(target, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    }
    glBindTexture(target, 0);
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) DualLogError("Failed to create texture %s: OpenGL error %d\n", name, error);
}

GLuint blueNoiseBuffer;

GLuint CompileShader(GLenum type, const char *source, const char *shaderName) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        DualLogError("%s Compilation Failed: %s\n", shaderName, infoLog);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint LinkProgram(GLuint *shaders, int count, const char *programName) {
    GLuint program = glCreateProgram();
    for (int i = 0; i < count; i++) glAttachShader(program, shaders[i]);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        DualLogError("%s Linking Failed: %s\n", programName, infoLog);
        glDeleteProgram(program);
        return 0;
    }

    for (int i = 0; i < count; i++) glDeleteShader(shaders[i]);
    return program;
}

int CompileShaders(void) {
    GLuint vertShader, fragShader, computeShader;

    // Chunk Shader
    vertShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource, "Chunk Vertex Shader");            if (!vertShader) { return 1; }
    fragShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderTraditional, "Chunk Fragment Shader"); if (!fragShader) { glDeleteShader(vertShader); return 1; }
    chunkShaderProgram = LinkProgram((GLuint[]){vertShader, fragShader}, 2, "Chunk Shader Program");    if (!chunkShaderProgram) { return 1; }

    // Text Shader
    vertShader = CompileShader(GL_VERTEX_SHADER, textVertexShaderSource, "Text Vertex Shader");       if (!vertShader) { return 1; }
    fragShader = CompileShader(GL_FRAGMENT_SHADER, textFragmentShaderSource, "Text Fragment Shader"); if (!fragShader) { glDeleteShader(vertShader); return 1; }
    textShaderProgram = LinkProgram((GLuint[]){vertShader, fragShader}, 2, "Text Shader Program");    if (!textShaderProgram) { return 1; }

    // Lightmap Baking Compute Shader Program
    computeShader = CompileShader(GL_COMPUTE_SHADER, lightmap_compute_shader, "Lightmap Baking Compute Shader"); if (!computeShader) { return 1; }
    lightmapShaderProgram = LinkProgram((GLuint[]){computeShader}, 1, "Lightmap Baking Shader Program");        if (!lightmapShaderProgram) { return 1; }
    
    // Deferred Lighting Compute Shader Program
    computeShader = CompileShader(GL_COMPUTE_SHADER, deferredLighting_computeShader, "Deferred Lighting Compute Shader"); if (!computeShader) { return 1; }
    deferredLightingShaderProgram = LinkProgram((GLuint[]){computeShader}, 1, "Deferred Lighting Shader Program");        if (!deferredLightingShaderProgram) { return 1; }

    // Screen Space Reflections Compute Shader Program
    computeShader = CompileShader(GL_COMPUTE_SHADER, ssr_computeShader, "Screen Space Reflections Compute Shader"); if (!computeShader) { return 1; }
    ssrShaderProgram = LinkProgram((GLuint[]){computeShader}, 1, "Screen Space Reflections Shader Program");        if (!ssrShaderProgram) { return 1; }

    // Image Blit Shader (For full screen image effects, rendering compute results, etc.)
    vertShader = CompileShader(GL_VERTEX_SHADER,   quadVertexShaderSource,   "Image Blit Vertex Shader");     if (!vertShader) { return 1; }
    fragShader = CompileShader(GL_FRAGMENT_SHADER, quadFragmentShaderSource, "Image Blit Fragment Shader");   if (!fragShader) { glDeleteShader(vertShader); return 1; }
    imageBlitShaderProgram = LinkProgram((GLuint[]){vertShader, fragShader}, 2, "Image Blit Shader Program"); if (!imageBlitShaderProgram) { return 1; }

    glGenBuffers(1, &blueNoiseBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, blueNoiseBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 12288 * sizeof(float), blueNoise, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, blueNoiseBuffer); // Use binding point 13
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // Cache uniform locations after shader compile!
    viewLoc_chunk = glGetUniformLocation(chunkShaderProgram, "view");
    projectionLoc_chunk = glGetUniformLocation(chunkShaderProgram, "projection");
    matrixLoc_chunk = glGetUniformLocation(chunkShaderProgram, "matrix");
    texIndexLoc_chunk = glGetUniformLocation(chunkShaderProgram, "texIndex");
    glowIndexLoc_chunk = glGetUniformLocation(chunkShaderProgram, "glowIndex");
    specIndexLoc_chunk = glGetUniformLocation(chunkShaderProgram, "specIndex");
    instanceIndexLoc_chunk = glGetUniformLocation(chunkShaderProgram, "instanceIndex");
    modelIndexLoc_chunk = glGetUniformLocation(chunkShaderProgram, "modelIndex");
    debugViewLoc_chunk = glGetUniformLocation(chunkShaderProgram, "debugView");
    overrideGlowRLoc_chunk = glGetUniformLocation(chunkShaderProgram, "overrideGlowR");
    overrideGlowGLoc_chunk = glGetUniformLocation(chunkShaderProgram, "overrideGlowG");
    overrideGlowBLoc_chunk = glGetUniformLocation(chunkShaderProgram, "overrideGlowB");

    totalLuxelCountLoc_lightmap = glGetUniformLocation(lightmapShaderProgram, "totalLuxelCount");
    instanceCountLoc_lightmap = glGetUniformLocation(lightmapShaderProgram, "instanceCount");
    modelCountLoc_lightmap = glGetUniformLocation(lightmapShaderProgram, "modelCount");
    lightCountLoc_lightmap = glGetUniformLocation(lightmapShaderProgram, "lightCount");
    worldMin_xLoc_lightmap = glGetUniformLocation(lightmapShaderProgram, "worldMin_x");
    worldMin_zLoc_lightmap = glGetUniformLocation(lightmapShaderProgram, "worldMin_z");
    
    screenWidthLoc_deferred = glGetUniformLocation(deferredLightingShaderProgram, "screenWidth");
    screenHeightLoc_deferred = glGetUniformLocation(deferredLightingShaderProgram, "screenHeight");
    debugViewLoc_deferred = glGetUniformLocation(deferredLightingShaderProgram, "debugView");
    worldMin_xLoc_deferred = glGetUniformLocation(deferredLightingShaderProgram, "worldMin_x");
    worldMin_zLoc_deferred = glGetUniformLocation(deferredLightingShaderProgram, "worldMin_z");
    cam_xLoc_deferred = glGetUniformLocation(deferredLightingShaderProgram, "cam_x");
    cam_yLoc_deferred = glGetUniformLocation(deferredLightingShaderProgram, "cam_y");
    cam_zLoc_deferred = glGetUniformLocation(deferredLightingShaderProgram, "cam_z");
    fogColorRLoc_deferred = glGetUniformLocation(deferredLightingShaderProgram, "fogColorR");
    fogColorGLoc_deferred = glGetUniformLocation(deferredLightingShaderProgram, "fogColorG");
    fogColorBLoc_deferred = glGetUniformLocation(deferredLightingShaderProgram, "fogColorB");
    viewProjectionLoc_deferred = glGetUniformLocation(deferredLightingShaderProgram, "viewProjection");
    modelCountLoc_deferred = glGetUniformLocation(deferredLightingShaderProgram, "modelCount");
    totalLuxelCountLoc_deferred = glGetUniformLocation(deferredLightingShaderProgram, "totalLuxelCount");

    screenWidthLoc_ssr = glGetUniformLocation(ssrShaderProgram, "screenWidth");
    screenHeightLoc_ssr = glGetUniformLocation(ssrShaderProgram, "screenHeight");
    viewProjectionLoc_ssr = glGetUniformLocation(ssrShaderProgram, "viewProjection");
    cam_xLoc_ssr = glGetUniformLocation(ssrShaderProgram, "cam_x");
    cam_yLoc_ssr = glGetUniformLocation(ssrShaderProgram, "cam_y");
    cam_zLoc_ssr = glGetUniformLocation(ssrShaderProgram, "cam_z");
    stepCountLoc_ssr = glGetUniformLocation(ssrShaderProgram, "maxSteps");
    stepSizeLoc_ssr = glGetUniformLocation(ssrShaderProgram, "stepSize");
    maxDistLoc_ssr = glGetUniformLocation(ssrShaderProgram, "maxDistance");
    
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
    return 0;
}

// Renders text at x,y coordinates specified using pointer to the string array.
void RenderText(float x, float y, const char *text, int colorIdx) {
    glDisable(GL_CULL_FACE); // Disable backface culling
    if (!font) { DualLogError("Font is NULL\n"); return; }
    if (!text) { DualLogError("Text is NULL\n"); return; }
    
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, textColors[colorIdx]);
    if (!surface) { DualLogError("TTF_RenderText_Solid failed: %s\n", TTF_GetError()); return; }
    
    SDL_Surface *rgba_surface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(surface);
    if (!rgba_surface) { DualLogError("SDL_ConvertSurfaceFormat failed: %s\n", SDL_GetError()); return; }

    GLuint texture;
    glCreateTextures(GL_TEXTURE_2D, 1, &texture);
    glTextureStorage2D(texture, 1, GL_RGBA8, rgba_surface->w, rgba_surface->h);
    glTextureSubImage2D(texture, 0, 0, 0, rgba_surface->w, rgba_surface->h, GL_RGBA, GL_UNSIGNED_BYTE, rgba_surface->pixels);
    glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glUseProgram(textShaderProgram);
    glProgramUniformMatrix4fv(textShaderProgram, projectionLoc_text, 1, GL_FALSE, uiOrthoProjection);
    float r = textColors[colorIdx].r / 255.0f;
    float g = textColors[colorIdx].g / 255.0f;
    float b = textColors[colorIdx].b / 255.0f;
    float a = textColors[colorIdx].a / 255.0f;
    glProgramUniform4f(textShaderProgram, textColorLoc_text, r, g, b, a);
    glBindTextureUnit(0,texture);
    glProgramUniform1i(textShaderProgram, textTextureLoc_text, 0);
    float scaleX = (float)rgba_surface->w;
    float scaleY = (float)rgba_surface->h;
    glProgramUniform2f(textShaderProgram, texelSizeLoc_text, 1.0f / scaleX, 1.0f / scaleY);
    glEnable(GL_BLEND); // Enable blending for text transparency
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST); // Disable depth test for 2D overlay
    glBindVertexArray(textVAO);
    float vertices[] = {
        x,          y,          0.0f, 0.0f, // Bottom-left
        x + scaleX, y,          1.0f, 0.0f, // Bottom-right
        x + scaleX, y + scaleY, 1.0f, 1.0f, // Top-right
        x,          y + scaleY, 0.0f, 1.0f  // Top-left
    };
    
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4); // Render quad (two triangles)
    drawCallsRenderedThisFrame++;
    verticesRenderedThisFrame+=6;

    // Cleanup
    glBindVertexArray(0);
    glBindTextureUnit(0, 0);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST); // Re-enable depth test for 3D rendering
    glUseProgram(0);
    glDeleteTextures(1, &texture);
    SDL_FreeSurface(rgba_surface);
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
    glReadPixels(0, 0, screen_width, screen_height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
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

// out = a * b
static inline void mul_mat4(float *out, const float *a, const float *b) {
    float result[16];
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            result[col*4 + row] =
                a[0*4 + row] * b[col*4 + 0] +
                a[1*4 + row] * b[col*4 + 1] +
                a[2*4 + row] * b[col*4 + 2] +
                a[3*4 + row] * b[col*4 + 3];
        }
    }
    // copy back
    for (int i = 0; i < 16; i++)
        out[i] = result[i];
}


// ================================= Input ==================================
// Create a quaternion from yaw (around Y), pitch (around X), and roll (around Z) in degrees
void quat_from_yaw_pitch_roll(Quaternion* q, float yaw_deg, float pitch_deg, float roll_deg) {
    float yaw = deg2rad(yaw_deg);   // Around Y (up)
    float pitch = deg2rad(pitch_deg); // Around X (right)
    float roll = deg2rad(roll_deg);  // Around Z (forward)
    float cy = cosf(yaw * 0.5f);
    float sy = sinf(yaw * 0.5f);
    float cp = cosf(pitch * 0.5f);
    float sp = sinf(pitch * 0.5f);
    float cr = cosf(roll * 0.5f);
    float sr = sinf(roll * 0.5f);
    q->w = cy * cp * cr + sy * sp * sr;
    q->x = cy * sp * cr + sy * cp * sr; // X-axis (pitch)
    q->y = sy * cp * cr - cy * sp * sr; // Y-axis (yaw)
    q->z = cy * cp * sr - sy * sp * cr; // Z-axis (roll)
    
    // Normalize quaterrnion
    float len = sqrt(q->w * q->w + q->x * q->x + q->y * q->y + q->z * q->z);
    if (len > 1e-6f) { q->x /= len; q->y /= len; q->z /= len; q->w /= len; }
    else { q->x = 0.0f; q->y = 0.0f; q->z = 0.0f; q->w = 1.0f; }
}

void Input_MouselookApply() {
    if (in_cyberspace) quat_from_yaw_pitch_roll(&cam_rotation,cam_yaw,cam_pitch,cam_roll);
    else               quat_from_yaw_pitch_roll(&cam_rotation,cam_yaw,cam_pitch,    0.0f);
}

int Input_KeyDown(uint32_t scancode) {
    keys[scancode] = true;
    if (scancode == SDL_SCANCODE_TAB) {
        in_cyberspace = !in_cyberspace;
        cam_roll = 0.0f; // Reset roll for sanity
        Input_MouselookApply();
    }

    if (keys[SDL_SCANCODE_R]) {
        debugView++;
        glProgramUniform1i(deferredLightingShaderProgram, debugViewLoc_deferred, debugView);
        glProgramUniform1i(chunkShaderProgram, debugViewLoc_chunk, debugView);
        glProgramUniform1i(imageBlitShaderProgram, debugViewLoc_quadblit, debugView);
        if (debugView > 8) debugView = 0;
    }

    if (keys[SDL_SCANCODE_Y]) {
        debugValue++;
        glProgramUniform1i(imageBlitShaderProgram, debugValueLoc_quadblit, debugValue);
        if (debugValue > 6) debugValue = 0;
    }

    if (keys[SDL_SCANCODE_E]) {
        play_wav("./Audio/weapons/wpistol.wav",0.5f);
    }
    
    if (keys[SDL_SCANCODE_O]) {
        ssr_StepCount++;
    } else if (keys[SDL_SCANCODE_P]) {
        ssr_StepCount--;
    }
    
    if (keys[SDL_SCANCODE_K]) {
        ssr_MaxDist += 0.32f;
    } else if (keys[SDL_SCANCODE_L]) {
        ssr_MaxDist -= 0.32f;
    }
    
    if (keys[SDL_SCANCODE_N]) {
        ssr_StepSize += 0.01f;
    } else if (keys[SDL_SCANCODE_M]) {
        ssr_StepSize -= 0.01f;
    }
    

    if (keys[SDL_SCANCODE_1]) {
        fogColorR += 0.01f;
    } else if (keys[SDL_SCANCODE_2]) {
        fogColorR -= 0.01f;
    }
    
    if (keys[SDL_SCANCODE_3]) {
        fogColorG += 0.01f;
    } else if (keys[SDL_SCANCODE_4]) {
        fogColorG -= 0.01f;
    }
    
    if (keys[SDL_SCANCODE_5]) {
        fogColorB += 0.01f;
    } else if (keys[SDL_SCANCODE_6]) {
        fogColorB -= 0.01f;
    }

    return 0;
}

int Input_KeyUp(uint32_t scancode) {
    keys[scancode] = false;
    return 0;
}

int Input_MouseMove(float xrel, float yrel) {
    cam_yaw -= xrel * -mouse_sensitivity;
    cam_pitch += yrel * mouse_sensitivity;
    if (cam_pitch > 89.0f) cam_pitch = 89.0f; // Avoid gimbal lock at pure 90deg
    if (cam_pitch < -89.0f) cam_pitch = -89.0f;
    Input_MouselookApply();
    return 0;
}

void normalize_vector(float* x, float* y, float* z) {
    float len = sqrtf(*x * *x + *y * *y + *z * *z);
    if (len > 1e-6f) { *x /= len; *y /= len; *z /= len; } // Length check to avoid division by zero.
}

// Converts normalized quaternion to a 4x4 column-major matrix (left-handed, Y-up, Z-forward)
void quat_to_matrix(Quaternion* q, float* m) {
    float x = q->x, y = q->y, z = q->z, w = q->w;
    float x2 = x * x, y2 = y * y, z2 = z * z;
    float xy = x * y, xz = x * z, yz = y * z;
    float wx = w * x, wy = w * y, wz = w * z;

    // Column-major rotation matrix for Unity (Y+ up, Z+ forward, X+ right)
    m[0]  = 1.0f - 2.0f * (y2 + z2); // Right X
    m[1]  = 2.0f * (xy + wz);        // Right Y
    m[2]  = 2.0f * (xz - wy);        // Right Z
    m[3]  = 0.0f;
    m[4]  = 2.0f * (xy - wz);        // Up X
    m[5]  = 1.0f - 2.0f * (x2 + z2); // Up Y
    m[6]  = 2.0f * (yz + wx);        // Up Z
    m[7]  = 0.0f;
    m[8]  = 2.0f * (xz + wy);        // Forward X
    m[9]  = 2.0f * (yz - wx);        // Forward Y
    m[10] = 1.0f - 2.0f * (x2 + y2); // Forward Z
    m[11] = 0.0f;
    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = 0.0f;
    m[15] = 1.0f;
}

void UpdatePlayerFacingAngles() {
    float rotation[16]; // Extract forward and right vectors from quaternion
    quat_to_matrix(&cam_rotation, rotation);
    cam_forwardx = rotation[8];  // Forward X
    cam_forwardy = rotation[9];  // Forward Y
    cam_forwardz = rotation[10]; // Forward Z
    cam_rightx = rotation[0];  // Right X
    cam_righty = rotation[1];  // Right Y
    cam_rightz = rotation[2];  // Right Z
    normalize_vector(&cam_forwardx, &cam_forwardy, &cam_forwardz); // Normalize forward
    normalize_vector(&cam_rightx, &cam_righty, &cam_rightz); // Normalize strafe
}

// Update camera position based on input
void ProcessInput(void) {
    if (keys[SDL_SCANCODE_LSHIFT]) sprinting = 2.0f;
    else sprinting = 0.0f;

    float finalMoveSpeed = (move_speed + (sprinting * move_speed));
    if (keys[SDL_SCANCODE_F]) {
        cam_x += finalMoveSpeed * cam_forwardx; // Move forward
        cam_y += finalMoveSpeed * cam_forwardy;
        cam_z += finalMoveSpeed * cam_forwardz;
    } else if (keys[SDL_SCANCODE_S]) {
        cam_x -= finalMoveSpeed * cam_forwardx; // Move backward
        cam_y -= finalMoveSpeed * cam_forwardy;
        cam_z -= finalMoveSpeed * cam_forwardz;
    }

    if (keys[SDL_SCANCODE_D]) {
        cam_x += finalMoveSpeed * cam_rightx; // Strafe right
        cam_y += finalMoveSpeed * cam_righty;
        cam_z += finalMoveSpeed * cam_rightz;
    } else if (keys[SDL_SCANCODE_A]) {
        cam_x -= finalMoveSpeed * cam_rightx; // Strafe left
        cam_y -= finalMoveSpeed * cam_righty;
        cam_z -= finalMoveSpeed * cam_rightz;
    }

    if (noclip) {
        if (keys[SDL_SCANCODE_V]) cam_y += finalMoveSpeed; // Move up
        else if (keys[SDL_SCANCODE_C]) cam_y -= finalMoveSpeed; // Move down
    }

    if (keys[SDL_SCANCODE_Q]) {
        cam_roll += move_speed * 5.0f; // Move up
        Input_MouselookApply();
    } else if (keys[SDL_SCANCODE_T]) {
        cam_roll -= move_speed * 5.0f; // Move down
        Input_MouselookApply();
    }
}

// ============================================================================
float squareDistance3D(float x1, float y1, float z1, float x2, float y2, float z2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float dz = z2 - z1;
    return dx * dx + dy * dy + dz * dz;
}
// ============================================================================
void SetUpdatedMatrix(float *mat, float posx, float posy, float posz, float rotx, float roty, float rotz, float rotw, float sclx, float scly, float sclz) {
    float x = rotx, y = roty, z = rotz, w = rotw;
    float x2 = x * x, y2 = y * y, z2 = z * z;
    float xy = x * y, xz = x * z, yz = y * z;
    float wx = w * x, wy = w * y, wz = w * z;

    // Construct rotation matrix (column-major, Unity: X+ right, Y+ up, Z+ forward)
    float rot[16];
    rot[0]  = 1.0f - 2.0f * (y2 + z2); // Right X
    rot[1]  = 2.0f * (xy + wz);        // Right Y
    rot[2]  = 2.0f * (xz - wy);        // Right Z
    rot[3]  = 0.0f;
    rot[4]  = 2.0f * (xy - wz);        // Up X
    rot[5]  = 1.0f - 2.0f * (x2 + z2); // Up Y
    rot[6]  = 2.0f * (yz + wx);        // Up Z
    rot[7]  = 0.0f;
    rot[8]  = 2.0f * (xz + wy);        // Forward X
    rot[9]  = 2.0f * (yz - wx);        // Forward Y
    rot[10] = 1.0f - 2.0f * (x2 + y2); // Forward Z
    rot[11] = 0.0f;
    rot[12] = 0.0f;
    rot[13] = 0.0f;
    rot[14] = 0.0f;
    rot[15] = 1.0f;

    // Apply uniform scaling to rotation matrix
    mat[0]  = rot[0] * -sclx; mat[1]  = rot[1] * -sclx; mat[2]  = rot[2] * -sclx; mat[3]  = 0.0f;
    mat[4]  = rot[4] * scly; mat[5]  = rot[5] * scly; mat[6]  = rot[6] * scly; mat[7]  = 0.0f;
    mat[8]  = rot[8] * sclz; mat[9]  = rot[9] * sclz; mat[10] = rot[10] * sclz; mat[11] = 0.0f;
    mat[12] = posx;          mat[13] = posy;          mat[14] = posz;          mat[15] = 1.0f;
}

void UpdateInstanceMatrix(int i) {
    if (instances[i].modelIndex >= MODEL_COUNT) { dirtyInstances[i] = false; return; } // No model
    if (modelVertexCounts[instances[i].modelIndex] < 1) { dirtyInstances[i] = false; return; } // Empty model
    if (instances[i].modelIndex < 0) return; // Invalid or culled

    float mat[16]; // 4x4 matrix
    SetUpdatedMatrix(mat, instances[i].posx, instances[i].posy, instances[i].posz, instances[i].rotx, instances[i].roty, instances[i].rotz, instances[i].rotw, instances[i].sclx, instances[i].scly, instances[i].sclz);
    memcpy(&modelMatrices[i * 16], mat, 16 * sizeof(float));
    dirtyInstances[i] = false;
}

int SetupInstances(void) {
    DualLog("Initializing instances...\n");
//     int x,z;
//     x = 0; z = 0;
    int idx;
    for (idx = 0;idx<INSTANCE_COUNT;idx++) {
//         int entIdx = idx < MAX_ENTITIES ? idx : 0;
        instances[idx].modelIndex = UINT16_MAX;//entities[entIdx].modelIndex;
        instances[idx].texIndex = UINT16_MAX;//entities[entIdx].texIndex;
        instances[idx].glowIndex = UINT16_MAX;//entities[entIdx].glowIndex;
        instances[idx].specIndex = UINT16_MAX;//entities[entIdx].specIndex;
        instances[idx].normIndex = UINT16_MAX;//entities[entIdx].normIndex;
        instances[idx].lodIndex = UINT16_MAX;//entities[entIdx].lodIndex;
        instances[idx].posx = 0.0f;//((float)x * 2.56f);
        instances[idx].posy = 0.0f;
        instances[idx].posz = 0.0f;//((float)z * 5.12f);
        instances[idx].sclx = instances[idx].scly = instances[idx].sclz = 1.0f; // Default scale
        instances[idx].rotx = instances[idx].roty = instances[idx].rotz = 0.0f;
        instances[idx].rotw = 1.0f; // Quaternion identity
//         x++;
//         if (idx == 100 || idx == 200 || idx == 300 || idx == 400 || idx == 500
//             || idx == 600 || idx == 700 || idx == 800 || idx == 900) {
//
//             x = 0;
//             z++;
//         }

        dirtyInstances[idx] = true;
    }
    
//     idx = 5455;  // Test light representative, not actually the light, moves with it
//     instances[idx].modelIndex = 621; // Test Light Sphere
//     instances[idx].texIndex = 881; // white light
//     instances[idx].glowIndex = 881; // white light
//     instances[idx].sclx = 0.4f;
//     instances[idx].scly = 0.4f;
//     instances[idx].sclz = 0.4f;

    // Generate and size instance related buffers
    glGenBuffers(1, &instancesBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, instancesBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, INSTANCE_COUNT * sizeof(Instance), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, instancesBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    
    glGenBuffers(1, &instancesInPVSBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, instancesInPVSBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, INSTANCE_COUNT * sizeof(uint32_t), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, instancesInPVSBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    memset(modelMatrices, 0, INSTANCE_COUNT * 16 * sizeof(float)); // Matrix4x4 = 16
    
    glGenBuffers(1, &matricesBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, matricesBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, INSTANCE_COUNT * 16 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, matricesBuffer);
    CHECK_GL_ERROR();
    malloc_trim(0);
    DebugRAM("end of SetupInstances");
    return 0;
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

void UpdateScreenSize(void) {
    float* m;
    m = uiOrthoProjection;
    m[0] = 2.0f / (float)screen_width; m[1] =                           0.0f; m[2] =  0.0f; m[3] = 0.0f;
    m[4] =                       0.0f; m[5] = 2.0f / -((float)screen_height); m[6] =  0.0f; m[7] = 0.0f;
    m[8] =                       0.0f; m[9] =                           0.0f; m[10]= -1.0f; m[11]= 0.0f;
    m[12]=                      -1.0f; m[13]=                           1.0f; m[14]=  0.0f; m[15]= 1.0f;
    
    float aspect = (float)screen_width / (float)screen_height;
    float f = 1.0f / tan(fov * M_PI / 360.0f);
    m = rasterPerspectiveProjection;
    m[0] = f / aspect; m[1] = 0.0f; m[2] =                                                  0.0f; m[3] =  0.0f;
    m[4] =       0.0f; m[5] =    f; m[6] =                                                  0.0f; m[7] =  0.0f;
    m[8] =       0.0f; m[9] = 0.0f; m[10]=      -(farPlane + nearPlane) / (farPlane - nearPlane); m[11]= -1.0f;
    m[12]=       0.0f; m[13]= 0.0f; m[14]= -2.0f * farPlane * nearPlane / (farPlane - nearPlane); m[15]=  0.0f;
}

int LightmapBake() {
    double start_time = get_time();
    if (renderableCount < 1) { DualLogError("No renderables to bake lightmaps for!\n"); return 1; }

    uint32_t totalLuxelCount = 64u * 64u * renderableCount;
    DualLog("Starting GPU Lightmapper bake for %d luxels and %d instances!  This could take a bit...\n", totalLuxelCount, renderableCount);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightsID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, LIGHT_COUNT * LIGHT_DATA_SIZE * sizeof(float), lights, GL_DYNAMIC_DRAW); // Send all lights for level to lightmapper to bake er'thang.  This is limited down during main loop to culled lights
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 19, lightsID);
    for (uint16_t i=0;i<INSTANCE_COUNT;i++) UpdateInstanceMatrix(i); // Update every instance mat4x4 in modelMatrices array
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, matricesBuffer);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, INSTANCE_COUNT * 16 * sizeof(float), modelMatrices); // * 16 because matrix4x4
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, matricesBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glUseProgram(lightmapShaderProgram);
    glUniform1ui(totalLuxelCountLoc_lightmap, totalLuxelCount);
    glUniform1ui(instanceCountLoc_lightmap, renderableCount);
    glUniform1ui(modelCountLoc_lightmap, MODEL_COUNT);
    glUniform1ui(lightCountLoc_lightmap, LIGHT_COUNT);
    glUniform1f(worldMin_xLoc_lightmap, worldMin_x);
    glUniform1f(worldMin_zLoc_lightmap, worldMin_z);
    GLuint groupsX = (800 + 31) / 32;
    GLuint groupsY = (600 + 31) / 32;
    glDispatchCompute(groupsX, groupsY, 1);
    CHECK_GL_ERROR();
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
    double end_time = get_time();
    DualLog("Lightmap Bake took %f seconds\n", end_time - start_time);
    return 0;
}

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
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    window = SDL_CreateWindow("Voxen, the OpenGL Voxel Lit Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, SDL_WINDOW_OPENGL);
    if (!window) { DualLogError("SDL_CreateWindow failed: %s\n", SDL_GetError()); return SYS_WIN + 1; }
    systemInitialized[SYS_WIN] = true;
    UpdateScreenSize();
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
    glFrontFace(GL_CCW); // Set triangle sorting order (GL_CW vs GL_CCW)
    glViewport(0, 0, screen_width, screen_height);
    CHECK_GL_ERROR();

    if (CompileShaders()) return SYS_COUNT + 1;
    glUseProgram(chunkShaderProgram);
    glUniformMatrix4fv(projectionLoc_chunk, 1, GL_FALSE, rasterPerspectiveProjection);

    glUseProgram(chunkShaderProgram);
    glUniform1f(overrideGlowRLoc_chunk, 0.0f);
    glUniform1f(overrideGlowGLoc_chunk, 0.0f);
    glUniform1f(overrideGlowBLoc_chunk, 0.0f);

    glUseProgram(imageBlitShaderProgram);
    glUniform1ui(screenWidthLoc_imageBlit, screen_width);
    glUniform1ui(screenHeightLoc_imageBlit, screen_height);

    glUseProgram(deferredLightingShaderProgram);
    glUniform1ui(screenWidthLoc_deferred, screen_width);
    glUniform1ui(screenHeightLoc_deferred, screen_height);
    glUniform1ui(modelCountLoc_deferred, MODEL_COUNT);

    glUseProgram(ssrShaderProgram);
    glUniform1ui(screenWidthLoc_ssr, screen_width / SSR_RES);
    glUniform1ui(screenHeightLoc_ssr, screen_height / SSR_RES);
    glUniform1i(stepCountLoc_ssr, ssr_StepCount);
    glUniform1f(maxDistLoc_ssr, ssr_MaxDist);
    glUniform1f(stepSizeLoc_ssr, ssr_StepSize);
    glUseProgram(0);
    CHECK_GL_ERROR();
    malloc_trim(0);
    
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
    glVertexAttribFormat(3, 1, GL_FLOAT, GL_FALSE, 8 * sizeof(float)); // Tex Index (int)
    glVertexAttribFormat(4, 1, GL_FLOAT, GL_FALSE, 9 * sizeof(float)); // Glow Index (int)
    glVertexAttribFormat(5, 1, GL_FLOAT, GL_FALSE, 10 * sizeof(float)); // Spec Index (int)
    glVertexAttribFormat(6, 1, GL_FLOAT, GL_FALSE, 11 * sizeof(float)); // Normal Index (int)
    glVertexAttribFormat(7, 1, GL_FLOAT, GL_FALSE, 12 * sizeof(float)); // Model Index (int)
    glVertexAttribFormat(8, 1, GL_FLOAT, GL_FALSE, 13 * sizeof(float)); // Instance Index (int)
    glVertexAttribFormat(9, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float)); // Tex Coord Lightmap (vec2)
    for (uint8_t i = 0; i < 10; i++) {
        glVertexAttribBinding(i, 0);
        glEnableVertexAttribArray(i);
    }

    glBindVertexArray(0);
    DebugRAM("after vao chunk bind");

    // Create Framebuffer
    // First pass gbuffer images
    GenerateAndBindTexture(&inputImageID,             GL_RGBA8, screen_width, screen_height,            GL_RGBA,           GL_UNSIGNED_BYTE, GL_TEXTURE_2D, "Lit Raster");
    GenerateAndBindTexture(&inputWorldPosID,        GL_RGBA32F, screen_width, screen_height,            GL_RGBA,                   GL_FLOAT, GL_TEXTURE_2D, "Raster World Positions");
    GenerateAndBindTexture(&inputNormalsID,         GL_RGBA32F, screen_width, screen_height,            GL_RGBA,                   GL_FLOAT, GL_TEXTURE_2D, "Raster Normals");
    GenerateAndBindTexture(&inputDepthID, GL_DEPTH_COMPONENT24, screen_width, screen_height, GL_DEPTH_COMPONENT,            GL_UNSIGNED_INT, GL_TEXTURE_2D, "Raster Depth");
    GenerateAndBindTexture(&outputImageID,          GL_RGBA32F, screen_width / SSR_RES, screen_height / SSR_RES,    GL_RGBA,                   GL_FLOAT, GL_TEXTURE_2D, "Deferred Lighting Result Colors");
    glGenFramebuffers(1, &gBufferFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, inputImageID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, inputWorldPosID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, inputNormalsID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, inputDepthID, 0);
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, drawBuffers);
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
    glBindImageTexture(1, inputWorldPosID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(2, inputNormalsID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    //                 3 = depth
    glBindImageTexture(4, outputImageID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F); // Output
    glActiveTexture(GL_TEXTURE3); // Match binding = 3 in shader
    glBindTexture(GL_TEXTURE_2D, inputDepthID);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    malloc_trim(0);
    DebugRAM("setup gbuffer end");
    
    // Text Initialization
    glCreateBuffers(1, &textVBO);
    glCreateVertexArrays(1, &textVAO);    
    glNamedBufferData(textVBO, sizeof(textQuadVertices), textQuadVertices, GL_STATIC_DRAW);
    glEnableVertexArrayAttrib(textVAO, 0); // DSA: Enable position attribute
    glEnableVertexArrayAttrib(textVAO, 1); // DSA: Enable texture coordinate attribute
    glVertexArrayAttribFormat(textVAO, 0, 2, GL_FLOAT, GL_FALSE, 0); // DSA: Position (x, y)
    glVertexArrayAttribFormat(textVAO, 1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float)); // DSA: Texcoord (u, v)
    glVertexArrayVertexBuffer(textVAO, 0, textVBO, 0, 4 * sizeof(float)); // DSA: Link VBO to VAO
    glVertexArrayAttribBinding(textVAO, 0, 0); // DSA: Bind position to binding index 0
    glVertexArrayAttribBinding(textVAO, 1, 0); // DSA: Bind texcoord to binding index 0
    CHECK_GL_ERROR();
    font = TTF_OpenFont("./Fonts/SystemShockText.ttf", 12);
    if (!font) { DualLogError("TTF_OpenFont failed: %s\n", TTF_GetError()); return SYS_TTF + 1; }
    DebugRAM("text init");

    // Lights buffer
    glGenBuffers(1, &lightsID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightsID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_VISIBLE_LIGHTS * LIGHT_DATA_SIZE * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 19, lightsID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    DebugRAM("after Lights Buffer Init");
    
    // Input
    Input_MouselookApply();
    
    // Network
    if (enet_initialize() != 0) { DualLogError("ENet initialization failed\n"); return -1; }
    ENetAddress address;
    enet_address_set_host(&address, server_address);
    address.port = server_port;
    client_host = enet_host_create(NULL, 1, 2, 0, 0);
    if (!client_host) { DualLogError("Failed to create ENet client host\n"); return -1; }

    server_peer = enet_host_connect(client_host, &address, 2, 0);
    if (!server_peer) { DualLogError("Failed to connect to server\n"); return -1; }
    systemInitialized[SYS_NET] = true;
    
    // Audio
    InitializeAudio();
    DebugRAM("audio init");
    systemInitialized[SYS_AUD] = true;
    
    // Load Game/Mod Definition
    const char* filename = "./Data/gamedata.txt";
    DualLog("Loading game definition from %s...\n",filename);    
    DataParser gamedata_parser;
    DataEntry entry;
    init_data_entry(&entry);
    gamedata_parser.entries = &entry;
    gamedata_parser.valid_keys = valid_gamedata_keys;
    gamedata_parser.num_keys = NUM_GAMDAT_KEYS;
    FILE *gamedatfile = fopen(filename, "r");
    if (!gamedatfile) { DualLogError("Cannot open %s\n", filename); DualLogError("Could not parse %s!\n", filename); return 1; }
    
    uint32_t lineNum = 0;
    bool is_eof;
    while (!feof(gamedatfile)) {
        read_key_value(gamedatfile,&gamedata_parser, &entry, &lineNum, &is_eof);
    }
    
    fclose(gamedatfile);
    global_modname = entry.modname;
    if (strcmp(global_modname, "Citadel") == 0) global_modIsCitadel = true;;
    numLevels = entry.levelCount;
    startLevel = entry.startLevel;
    currentLevel = startLevel;
    DualLog("Game Definition for %s:: num levels: %d, start level: %d\n",global_modname,numLevels,startLevel);
    if (LoadTextures()) return 1;
    if (LoadGeometry()) return 1;
    if (LoadEntities()) return 1; // Must be after models and textures else entity types can't be validated.
    if (SetupInstances()) return 1;
    if (LoadLevelGeometry(currentLevel)) return 1; // Must be after entities!
    if (LoadLevelLights(currentLevel)) return 1;
    if (Cull_Init()) return 1; // Must be after level!
    if (LightmapBake()) return 1; // Must be after EVERYTHING ELSE!
    DebugRAM("InitializeEnvironment end");
    return 0;
}

float playerVelocity_y = 0.0f;
int Physics(void) {
    if (noclip) return 0;
    
//     DualLog("Physics tick, player at height: %f, playerVelocity_y: %f\n",cam_y, playerVelocity_y);
    float floorHeightForPlayer = INVALID_FLOOR_HEIGHT;
    for (int i=0;i<INSTANCE_COUNT;i++) {
        if ((uint16_t)cellIndexForInstance[i] == playerCellIdx) {
            if (instances[i].floorHeight > INVALID_FLOOR_HEIGHT) {
                floorHeightForPlayer = instances[i].floorHeight;
            }
        }
    }
    
    float stopHeight = floorHeightForPlayer + 0.84f;
    if (stopHeight < (INVALID_FLOOR_HEIGHT + 1.0f)) stopHeight = -45.5f;
    if (cam_y <= stopHeight) {
//         DualLog("Player hit floor at %f\n",stopHeight);
        cam_y = stopHeight;
        playerVelocity_y = 0.0f;
        return 0;
    }
    
    playerVelocity_y += 0.02f;
    if (playerVelocity_y > 1.0f) playerVelocity_y = 1.0f; // Terminal velocity
    cam_y -= (0.16f * playerVelocity_y);
//     DualLog("Player velocity_y at end of Physics tick: %f with cam_y %f relative to stopHeight %f\n",playerVelocity_y,cam_y,stopHeight);
    return 0;
}

// All core engine operations run through the EventExecute as an Event processed
// by the unified event system in the order it was enqueued.
int EventExecute(Event* event) {
    if (event->type == EV_NULL) return 0;

    switch(event->type) {
        case EV_KEYDOWN: return Input_KeyDown(event->payload1u);
        case EV_KEYUP: return Input_KeyUp(event->payload1u);
        case EV_MOUSEMOVE: return Input_MouseMove(event->payload1f,event->payload2f);
        case EV_PHYSICS_TICK: return Physics();
        case EV_QUIT: return 1; break;
    }

    DualLogError("Unknown event %d\n",event->type);
    return 99;
}

static const char* debugViewNames[] = {
    "standard render", // 0
    "unlit",           // 1
    "surface normals", // 2
    "depth",           // 3
    "indices",         // 4
    "worldpos",        // 5
    "lightview",       // 6
    "reflections",      // 7
    "lightmap"
};

float dot(float x1, float y1, float z1, float x2, float y2, float z2) {
    return x1 * x2 + y1 * y2 + z1 * z2;
}

// Generates View Matrix4x4 for Geometry Rasterizer Pass from camera world position + orientation
void mat4_lookat(float* m) {
    float rotation[16];
    quat_to_matrix(&cam_rotation, rotation);

    // Extract basis vectors (camera space axes)
    float right[3]   = { rotation[0], rotation[1], rotation[2] };   // X+ (right)
    float up[3]      = { rotation[4], rotation[5], rotation[6] };   // Y+ (up)
    float forward[3] = { rotation[8], rotation[9], rotation[10] };  // Z+ (forward)

    // View matrix: inverse rotation (transpose) and inverse translation
    m[0]  = right[0];   m[1]  = up[0];   m[2]  = -forward[0]; m[3]  = 0.0f;
    m[4]  = right[1];   m[5]  = up[1];   m[6]  = -forward[1]; m[7]  = 0.0f;
    m[8]  = right[2];   m[9]  = up[2];   m[10] = -forward[2]; m[11] = 0.0f;
    m[12] = -dot(right[0], right[1], right[2], cam_x, cam_y, cam_z);   // -dot(right, eye)
    m[13] = -dot(up[0], up[1], up[2], cam_x, cam_y, cam_z);      // -dot(up, eye)
    m[14] = dot(forward[0], forward[1], forward[2], cam_x, cam_y, cam_z);  // dot(forward, eye)
    m[15] = 1.0f;
}

typedef struct {
    uint16_t index; // Original index in lights array
    float distanceSquared; // Distance to camera squared
    float score; // Priority score (lower distance, higher intensity = higher priority)
} LightCandidate;

bool IsSphereInFOVCone(float inst_x, float inst_y, float inst_z, float radius) {
    float to_inst_x = inst_x - cam_x; // Vector from camera to instance
    float to_inst_y = inst_y - cam_y;
    float to_inst_z = inst_z - cam_z;
    float distance = sqrtf(to_inst_x * to_inst_x + to_inst_y * to_inst_y + to_inst_z * to_inst_z);
    if (distance < 3.62038672f) return true; // Avoid division by zero.  Closer than corner of a cell (sqrt(2) * 2.56f)

    to_inst_x /= distance; // Normalize direction to instance
    to_inst_y /= distance;
    to_inst_z /= distance;
    float dotFac = dot(cam_forwardx,cam_forwardy,cam_forwardz, to_inst_x,to_inst_y,to_inst_z);
    float fovAdjusted = cam_fov * 2.5f;
    float half_fov_rad = deg2rad(fovAdjusted * 0.5f); // Compare against cosine of half the FOV (cam_fov is in degrees, convert to radians)
    float cos_half_fov = cosf(half_fov_rad);
    if (dotFac >= cos_half_fov) return true; // Center is within FOV cone

    if (radius > 0.0f && distance > radius) {
        float adjusted_dot = dotFac - (radius / distance); // Approximate sphere extent
        if (adjusted_dot >= cos_half_fov) return true; // Part of sphere may be in view
    }

    return false; // Outside FOV cone
}

int EventInit(void) {
    journalFirstWrite = true;

    // Initialize the eventQueue as empty
    clear_ev_queue();
    clear_ev_journal(); // Initialize the event journal as empty.
    eventQueue[eventIndex].type = EV_NULL;
    eventQueue[eventIndex].timestamp = get_time();
    eventQueue[eventIndex].deltaTime_ns = 0.0;
    return 0;
}

int EnqueueEvent(uint8_t type, uint32_t payload1u, uint32_t payload2u, float payload1f, float payload2f) {
    if (eventQueueEnd >= MAX_EVENTS_PER_FRAME) { DualLogError("Queue buffer filled!\n"); return 1; }

    //DualLog("Enqueued event type %d, at index %d\n",type,eventQueueEnd);
    eventQueue[eventQueueEnd].frameNum = globalFrameNum;
    eventQueue[eventQueueEnd].type = type;
    eventQueue[eventQueueEnd].timestamp = 0;
    eventQueue[eventQueueEnd].payload1u = payload1u;
    eventQueue[eventQueueEnd].payload2u = payload2u;
    eventQueue[eventQueueEnd].payload1f = payload1f;
    eventQueue[eventQueueEnd].payload2f = payload2f;
    eventQueueEnd++;
    return 0;
}

int EnqueueEvent_UintUint(uint8_t type, uint32_t payload1u, uint32_t payload2u) {
    return EnqueueEvent(type,payload1u,payload2u,0.0f,0.0f);
}

int EnqueueEvent_Uint(uint8_t type, uint32_t payload1u) {
    return EnqueueEvent(type,payload1u,0u,0.0f,0.0f);
}

int EnqueueEvent_FloatFloat(uint8_t type, float payload1f, float payload2f) {
    return EnqueueEvent(type,0u,0u,payload1f,payload2f);
}

int EnqueueEvent_Float(uint8_t type, float payload1f) {
    return EnqueueEvent(type,0u,0u,payload1f,0.0f);
}

// Enqueues an event with type only and no payload values.
int EnqueueEvent_Simple(uint8_t type) {
    return EnqueueEvent(type,0u,0u,0.0f,0.0f);
}

// Intended to be called after each buffered write to the logfile in .dem
// format which is custom but similar concept to Quake 1 demos.
void clear_ev_journal(void) {
    //  Events will be buffer written until EV_NULL is seen so clear to EV_NULL.
    for (int i=0;i<EVENT_JOURNAL_BUFFER_SIZE;i++) {
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

        // TODO: Write player positions on first and 2nd line
    } else fp = fopen("./voxen.dem", "ab"); // Append

    if (!fp) {
        DualLogError("Failed to open voxen.dem for journal log\n");
        return;
    }

    // Write all valid events in eventJournal
    for (int i = 0; i < eventJournalIndex; i++) {
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
int ReadActiveLog() {
    static bool eof_reached = false; // Track EOF across calls
    Event event;
    int events_processed = 0;

    if (eof_reached) {
        return 2; // Indicate EOF was previously reached
    }

    DualLog("------ ReadActiveLog start for frame %d ------\n",globalFrameNum);
    while (events_processed < MAX_EVENTS_PER_FRAME) {
        size_t read_count = fread(&event, sizeof(Event), 1, activeLogFile);
        if (read_count != 1) {
            if (feof(activeLogFile)) {
                eof_reached = true;
                log_playback = false; // Finished enqueuing last frame, main will finish processing the queue and return input to user.
                return events_processed > 0 ? 0 : 2; // 0 if events were processed, 2 if EOF and no events
            }

            if (ferror(activeLogFile)) {
                DualLogError("Could not read log file\n");
                return -1; // Read error
            }
        }

        if (!IsPlayableEventType(event.type)) continue; // Skip unplayable events

        if (event.frameNum == globalFrameNum) {
            // Enqueue events matching the current frame
            EnqueueEvent(event.type, event.payload1u, event.payload2u, event.payload1f, event.payload2f);
            events_processed++;
            DualLog("Enqueued event %d from log for frame %d\n",event.type,event.frameNum);
        } else if (event.frameNum > globalFrameNum) {
            // Event is for a future frame; seek back and stop processing
            fseek(activeLogFile, -(long)sizeof(Event), SEEK_CUR);
            DualLog("Readback of %d events for this frame %d from log\n",events_processed,globalFrameNum);
            return events_processed > 0 ? 0 : 1; // 0 if events processed, 1 if no matching events
        }
        // If event.frameNum < globalFrameNum, skip it (past event)
    }

    DualLog("End of log. Readback of %d events for this frame %d from log\n",events_processed,globalFrameNum);
    return events_processed > 0 ? 0 : 1; // 0 if events processed, 1 if limit reached with no matching events
}

// Convert the binary .dem file into human readable text
int JournalDump(const char* dem_file) {
    FILE* fpR = fopen(dem_file, "rb");
    if (!fpR) {
        DualLogError("Failed to open .dem file\n");
        return -1;
    }

    FILE* fpW = fopen("./log_dump.txt", "wb");
    if (!fpW) {
        fclose(fpR); // Close .dem file that we were reading.
        DualLogError("Failed to open voxen.dem\n");
        return -1;
    }

    Event event;
    while (fread(&event, sizeof(Event), 1, fpR) == 1) {
        fprintf(fpW,"frameNum: %d, ",event.frameNum);
        fprintf(fpW,"event type: %d, ",event.type);
        fprintf(fpW,"timestamp: %f, ", event.timestamp);
        fprintf(fpW,"delta time: %f, ", event.deltaTime_ns);
        fprintf(fpW,"payload1u: %d, ", event.payload1u);
        fprintf(fpW,"payload2u: %d, ", event.payload2u);
        fprintf(fpW,"payload1f: %f, ", event.payload1f);
        fprintf(fpW,"payload2f: %f\n", event.payload2f); // \n flushes write to file
    }

    fclose(fpW);
    fclose(fpR);
    return 0;
}

// Queue was processed for the frame, clear it so next frame starts fresh.
void clear_ev_queue(void) {
    //  Events will be buffer written until EV_NULL is seen so clear to EV_NULL.
    for (int i=0;i<MAX_EVENTS_PER_FRAME;i++) {
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
int EventQueueProcess(void) {
    int status = 0;
    double timestamp = 0.0;
    int eventCount = 0;
    for (int i=0;i<MAX_EVENTS_PER_FRAME;i++) {
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
//                 DualLog("Event queue cleared after journal filled, log updated\n");
            } else {
//                 DualLog("Event queue cleared after journal filled, not writing to log during playback.\n");
            }

            clear_ev_journal(); // Also sets eventJournalIndex to 0.
        }

        eventJournal[eventJournalIndex].frameNum = eventQueue[eventIndex].frameNum;
        eventJournal[eventJournalIndex].type = eventQueue[eventIndex].type;
        eventJournal[eventJournalIndex].timestamp = eventQueue[eventIndex].timestamp;
        eventJournal[eventJournalIndex].deltaTime_ns = eventQueue[eventIndex].deltaTime_ns;
        eventJournal[eventJournalIndex].payload1u = eventQueue[eventIndex].payload1u;
        eventJournal[eventJournalIndex].payload2u = eventQueue[eventIndex].payload2u;
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

int main(int argc, char* argv[]) {
    console_log_file = fopen("voxen.log", "w"); // Initialize log system for all prints to go to both stdout and voxen.log file
    if (!console_log_file) DualLogError("Failed to open log file voxen.log\n");
    if (argc >= 2 && (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0)) {
        printf("-----------------------------------------------------------\n");
        printf("Voxen "
               VERSION_STRING
               "8/15/2025\nthe OpenGL Voxel Lit Rendering Engine\n\nby W. Josiah Jack\nMIT-0 licensed\n\n\n");
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
    
    DebugRAM("prior init events queued");
    if (InitializeEnvironment()) return 1;

    double accumulator = 0.0;
    double last_physics_time = get_time();
    last_time = get_time();
    lastJournalWriteTime = get_time();
    DebugRAM("prior to game loop");
    while(1) {
        current_time = get_time();
        double frame_time = current_time - last_time;
        
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
        
        accumulator += frame_time;
        UpdatePlayerFacingAngles();
        while (accumulator >= time_step) {
            if (window_has_focus) ProcessInput();
            accumulator -= time_step;
        }
        
        double timeSinceLastPhysicsTick = current_time - last_physics_time;
        if (timeSinceLastPhysicsTick > 0.016666666f) { // 60fps fixed tick rate
            last_physics_time = current_time;
            EnqueueEvent_Simple(EV_PHYSICS_TICK);
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
        if (EventQueueProcess()) break; // Do everything
        
        // Client Actions
        // ====================================================================
        // Client Render
        drawCallsRenderedThisFrame = 0; // Reset per frame
        verticesRenderedThisFrame = 0;
        
        // 0. Clear Frame Buffers and Depth
        glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear main FBO.  glClearBufferfv was actually SLOWER!

        Cull(); // Get world cell culling data into gridCellStates from precomputed data at init of what cells see what other cells.

        // 1. Light Culling to limit of MAX_VISIBLE_LIGHTS
        numLightsFound = 0;
        LightCandidate candidates[LIGHT_COUNT];
        uint16_t numCandidates = 0;

        // Step 1: Collect valid lights within sightRangeSquared
        for (uint16_t i = 0; i < LIGHT_COUNT; ++i) {
            uint16_t litIdx = (i * LIGHT_DATA_SIZE);
            float litIntensity = lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY];
            if (litIntensity < 0.015f) continue; // Skip if light is off

            float litx = lights[litIdx + LIGHT_DATA_OFFSET_POSX];
            float lity = lights[litIdx + LIGHT_DATA_OFFSET_POSY];
            float litz = lights[litIdx + LIGHT_DATA_OFFSET_POSZ];
            float distSqrd = squareDistance3D(cam_x, cam_y, cam_z, litx, lity, litz);
            int lightCellIdx = cellIndexForLight[i];
            int x = lightCellIdx % WORLDX;
            int y = lightCellIdx / WORLDX;
            int range = floor(lights[litIdx + LIGHT_DATA_OFFSET_RANGE] / 2.56f);
            int xMin = x - range; int xMax = x + range;
            int yMin = y - range; int yMax = y + range;
            bool inPVS = false;
            if ((gridCellStates[lightCellIdx] & CELL_VISIBLE)) {// || !(gridCellStates[lightCellIdx] & CELL_OPEN)) {
                inPVS = true; // Allow lights outside windows (and thus in non open cells) to still be applicable.
            } else { // Check cells that aren't visible but whose lights can light up cells that are visible.
                bool breakout = false;
                for (int ix = xMin;ix <= xMax; ix++) {
                    for (int iy = yMin;iy <= yMax; iy++) {
                        if (!XZPairInBounds(ix,iy)) continue;

                        int subIdx = (iy * WORLDX) + ix;
                        if ((gridCellStates[subIdx] & CELL_VISIBLE) // Player can see cell in light's range.
                            && precomputedVisibleCellsFromHere[(lightCellIdx * ARRSIZE) + subIdx]) { // Light's cell can see the cell in light's range.
                            
                            inPVS = true;
                            breakout = true;
                            break; // Avoid checking any more.  One is enough to count.
                        }
                    }
                    
                    if (breakout) break;
                }
            }
            
            if (distSqrd < sightRangeSquared && inPVS) {
                candidates[numCandidates].index = i;
                candidates[numCandidates].distanceSquared = distSqrd;

                // Adjust score to favor lights in view frustum if distance > 5.12
                float score = litIntensity / (distSqrd + 0.01f);
                if (distSqrd > 26.2144f) {
                    // Boost score for lights in the view frustum
                    float range = lights[litIdx + LIGHT_DATA_OFFSET_RANGE];
                    if (IsSphereInFOVCone(litx, lity, litz, range)) {
                        score *= 2.0f; // Increase score for frustum lights (tune this multiplier)
                    } else {
                        score *= 0.5f; // Reduce score for lights outside frustum
                    }
                }

                candidates[numCandidates].score = score;
                numCandidates++;
            }
        }

        // Step 2: Sort candidates by score (descending) to get top MAX_VISIBLE_LIGHTS
        for (uint16_t i = 0; i < numCandidates && i < MAX_VISIBLE_LIGHTS; ++i) {
            uint16_t bestIdx = i;
            float bestScore = candidates[i].score;
            for (uint16_t j = i + 1; j < numCandidates; ++j) {
                if (candidates[j].score > bestScore) {
                    bestScore = candidates[j].score;
                    bestIdx = j;
                }
            }
           
            if (bestIdx != i) { // Swap to put the best candidate at position i
                LightCandidate temp = candidates[i];
                candidates[i] = candidates[bestIdx];
                candidates[bestIdx] = temp;
            }
        }

        // Step 3: Copy the top MAX_VISIBLE_LIGHTS to lightsInProximity
        numLightsFound = numCandidates < MAX_VISIBLE_LIGHTS ? numCandidates : MAX_VISIBLE_LIGHTS;
        for (uint16_t i = 0; i < numLightsFound; ++i) {
            uint16_t litIdx = candidates[i].index * LIGHT_DATA_SIZE;
            uint16_t idx = i * LIGHT_DATA_SIZE;
            lightsInProximity[idx + LIGHT_DATA_OFFSET_POSX] = lights[litIdx + LIGHT_DATA_OFFSET_POSX];
            lightsInProximity[idx + LIGHT_DATA_OFFSET_POSY] = lights[litIdx + LIGHT_DATA_OFFSET_POSY];
            lightsInProximity[idx + LIGHT_DATA_OFFSET_POSZ] = lights[litIdx + LIGHT_DATA_OFFSET_POSZ];
            lightsInProximity[idx + LIGHT_DATA_OFFSET_INTENSITY] = lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY];
            lightsInProximity[idx + LIGHT_DATA_OFFSET_RANGE] = lights[litIdx + LIGHT_DATA_OFFSET_RANGE];
            lightsInProximity[idx + LIGHT_DATA_OFFSET_SPOTANG] = lights[litIdx + LIGHT_DATA_OFFSET_SPOTANG];
            lightsInProximity[idx + LIGHT_DATA_OFFSET_SPOTDIRX] = lights[litIdx + LIGHT_DATA_OFFSET_SPOTDIRX];
            lightsInProximity[idx + LIGHT_DATA_OFFSET_SPOTDIRY] = lights[litIdx + LIGHT_DATA_OFFSET_SPOTDIRY];
            lightsInProximity[idx + LIGHT_DATA_OFFSET_SPOTDIRZ] = lights[litIdx + LIGHT_DATA_OFFSET_SPOTDIRZ];
            lightsInProximity[idx + LIGHT_DATA_OFFSET_SPOTDIRW] = lights[litIdx + LIGHT_DATA_OFFSET_SPOTDIRW];
            lightsInProximity[idx + LIGHT_DATA_OFFSET_R] = lights[litIdx + LIGHT_DATA_OFFSET_R];
            lightsInProximity[idx + LIGHT_DATA_OFFSET_G] = lights[litIdx + LIGHT_DATA_OFFSET_G];
            lightsInProximity[idx + LIGHT_DATA_OFFSET_B] = lights[litIdx + LIGHT_DATA_OFFSET_B];
        }
        
        // Step 4: Clear remaining slots in lightsInProximity
        for (uint8_t i = numLightsFound; i < MAX_VISIBLE_LIGHTS; ++i) {
            lightsInProximity[(i * LIGHT_DATA_SIZE) + LIGHT_DATA_OFFSET_INTENSITY] = 0.0f;
        }
        
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightsID);
        glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_VISIBLE_LIGHTS * LIGHT_DATA_SIZE * sizeof(float), lightsInProximity, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 19, lightsID);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        // 2. Instance Culling to only those in range of player
        bool instanceIsCulledArray[INSTANCE_COUNT];
        bool instanceIsLODArray[INSTANCE_COUNT];
        memset(instanceIsCulledArray,true,INSTANCE_COUNT * sizeof(bool)); // All culled.
        memset(instanceIsLODArray,true,INSTANCE_COUNT * sizeof(bool)); // All using lower detail LOD mesh.
        float distSqrd = 0.0f;
        float lodRangeSqrd = 38.4f * 38.4f;
        for (uint16_t i=0;i<INSTANCE_COUNT;++i) {
            if (!instanceIsCulledArray[i]) continue; // Already marked as visible.
            
            distSqrd = squareDistance3D(instances[i].posx,instances[i].posy,instances[i].posz,cam_x, cam_y, cam_z);
            if (distSqrd < sightRangeSquared) instanceIsCulledArray[i] = false;
            if (distSqrd < lodRangeSqrd) instanceIsLODArray[i] = false; // Use full detail up close.
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
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, instancesInPVSBuffer);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, instancesInPVSCount * sizeof(uint32_t), instancesInPVS); // * 16 because matrix4x4
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, instancesInPVSBuffer);
        
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, matricesBuffer);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, INSTANCE_COUNT * 16 * sizeof(float), modelMatrices); // * 16 because matrix4x4
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, matricesBuffer);
        
        // 4. Raterized Geometry
        //        Standard vertex + fragment rendering, but with special packing to minimize transfer data amounts
        glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
        glUseProgram(chunkShaderProgram);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        float view[16]; // Set up view matrices
        mat4_lookat(view);
        glUniformMatrix4fv(viewLoc_chunk,       1, GL_FALSE,       view);
        glBindVertexArray(vao_chunk);
        for (uint16_t i=0;i<INSTANCE_COUNT;i++) {
            if (i == startOfDoubleSidedInstances) {
                if (debugValue > 0) DualLog("For frame %d, enabling doublesided\n",globalFrameNum);
                glDisable(GL_CULL_FACE);
            }

//             if (i == startOfTransparentInstances) {
//                 if (debugValue > 0) DualLog("For frame %d, enabling transparents\n",globalFrameNum);
//                 glEnable(GL_BLEND); // Enable blending for transparent instances
//                 glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending: src * srcAlpha + dst
//                 glDepthMask(GL_FALSE); // Disable depth writes for transparent instances
//             }
            if (instanceIsCulledArray[i]) continue; // Culled by distance
            if (instances[i].modelIndex >= MODEL_COUNT) continue;
            if (modelVertexCounts[instances[i].modelIndex] < 1) continue; // Empty model
            if (instances[i].modelIndex < 0) continue; // Invalid or hidden
            
            uint16_t instCellIdx = (uint16_t)cellIndexForInstance[i];
            if (instCellIdx < ARRSIZE) {
                if (!(gridCellStates[instCellIdx] & CELL_VISIBLE)) continue; // Culled by being in a cell outside player PVS
            }
            
            float radius = modelBounds[(instances[i].modelIndex * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_RADIUS];
            if (!IsSphereInFOVCone(instances[i].posx, instances[i].posy, instances[i].posz, radius)) continue; // Cone Frustum Culling
            
            if (dirtyInstances[i]) UpdateInstanceMatrix(i);
            glUniform1i(texIndexLoc_chunk, instances[i].texIndex);
            glUniform1i(glowIndexLoc_chunk, instances[i].glowIndex);
            glUniform1i(specIndexLoc_chunk, instances[i].specIndex);
            glUniform1i(instanceIndexLoc_chunk, i);
            int modelType = instanceIsLODArray[i] && instances[i].lodIndex < UINT16_MAX ? instances[i].lodIndex : instances[i].modelIndex;
            if (debugValue > 0) DualLog("For frame %d, rendering model type %d\n",globalFrameNum,instances[i].modelIndex);
            glUniform1i(modelIndexLoc_chunk, modelType);
            glUniformMatrix4fv(matrixLoc_chunk, 1, GL_FALSE, &modelMatrices[i * 16]);
            glBindVertexBuffer(0, vbos[modelType], 0, VERTEX_ATTRIBUTES_COUNT * sizeof(float));
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tbos[modelType]);
            glDrawElements(GL_TRIANGLES, modelTriangleCounts[modelType] * 3, GL_UNSIGNED_INT, 0);
            drawCallsRenderedThisFrame++;
            verticesRenderedThisFrame += modelTriangleCounts[modelType] * 3;
        }
        
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
        glEnable(GL_CULL_FACE); // Reenable backface culling
        glEnable(GL_DEPTH_TEST);
//         if (debugView == 6) { // Render Light Spheres
//            for (uint16_t i=0;i<numLightsFound;++i) {
//                 float mat[16]; // 4x4 matrix
//                 uint16_t idx = i * LIGHT_DATA_SIZE;
//                 float sphoxelSize = lightsInProximity[idx + LIGHT_DATA_OFFSET_RANGE] * 0.04f; // Const.segiVoxelSize from Citadel main
//                 if (sphoxelSize > 8.0f) sphoxelSize = 8.0f;
//                 SetUpdatedMatrix(mat, lightsInProximity[idx + LIGHT_DATA_OFFSET_POSX], lightsInProximity[idx + LIGHT_DATA_OFFSET_POSY], lightsInProximity[idx + LIGHT_DATA_OFFSET_POSZ],
//                                  0.0f, 0.0f, 0.0f, 1.0f, // Quaternion identity
//                                  sphoxelSize, sphoxelSize, sphoxelSize); // Uniform scale
//
//                 glUniform1f(overrideGlowRLoc_chunk, lightsInProximity[idx + LIGHT_DATA_OFFSET_R] * lightsInProximity[idx + LIGHT_DATA_OFFSET_INTENSITY]);
//                 glUniform1f(overrideGlowGLoc_chunk, lightsInProximity[idx + LIGHT_DATA_OFFSET_G] * lightsInProximity[idx + LIGHT_DATA_OFFSET_INTENSITY]);
//                 glUniform1f(overrideGlowBLoc_chunk, lightsInProximity[idx + LIGHT_DATA_OFFSET_B] * lightsInProximity[idx + LIGHT_DATA_OFFSET_INTENSITY]);
//                 glUniform1i(texIndexLoc_chunk, 41);
//                 glUniform1i(glowIndexLoc_chunk, 41);
//                 glUniform1i(specIndexLoc_chunk, 41);
//                 glUniform1i(instanceIndexLoc_chunk, i);
//                 int modelType = 621; // Test light icosphere
//                 glUniform1i(modelIndexLoc_chunk, modelType);
//                 glUniformMatrix4fv(matrixLoc_chunk, 1, GL_FALSE, mat);
//                 glBindVertexBuffer(0, vbos[modelType], 0, VERTEX_ATTRIBUTES_COUNT * sizeof(float));
//                 glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tbos[modelType]);
//                 glDrawElements(GL_TRIANGLES, modelTriangleCounts[modelType] * 3, GL_UNSIGNED_INT, 0);
//                 drawCallsRenderedThisFrame++;
//                 verticesRenderedThisFrame += modelTriangleCounts[modelType] * 3;
//            }
//         }

        // ====================================================================
        // Ok, turn off temporary framebuffer so we can draw to screen now.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // ====================================================================

        GLuint groupX = (screen_width + 31) / 32;
        GLuint groupY = (screen_height + 31) / 32;
        float viewProj[16];
        mul_mat4(viewProj, rasterPerspectiveProjection, view);
        if (debugView == 0 || debugView == 8) {
        // 5. Deferred Lighting
        //        Apply deferred lighting with compute shader.  All lights are
        //        dynamic and can be updated at any time (flicker, light switches,
        //        move, change color, get marked as "culled" so shader can skip it,
        //        etc.).
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellIndexForInstanceID);
            glBufferData(GL_SHADER_STORAGE_BUFFER, INSTANCE_COUNT * sizeof(uint32_t), cellIndexForInstance, GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 20, cellIndexForInstanceID);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

            glUseProgram(deferredLightingShaderProgram);
            glUniform1ui(totalLuxelCountLoc_deferred, 64u * 64u * renderableCount);
            glUniform1f(worldMin_xLoc_deferred, worldMin_x);
            glUniform1f(worldMin_zLoc_deferred, worldMin_z);
            glUniform1f(cam_xLoc_deferred, cam_x);
            glUniform1f(cam_yLoc_deferred, cam_y);
            glUniform1f(cam_zLoc_deferred, cam_z);
            glUniform1f(fogColorRLoc_deferred, fogColorR);
            glUniform1f(fogColorGLoc_deferred, fogColorG);
            glUniform1f(fogColorBLoc_deferred, fogColorB);
            glUniformMatrix4fv(viewProjectionLoc_deferred, 1, GL_FALSE, viewProj);

            // Dispatch compute shader
            glDispatchCompute(groupX, groupY, 1);
            CHECK_GL_ERROR();
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        }
        
        // 6. SSR (Screen Space Reflections)
        if (debugView == 0 || debugView == 7) {
            glUseProgram(ssrShaderProgram);
            glUniformMatrix4fv(viewProjectionLoc_ssr, 1, GL_FALSE, viewProj);
            glUniform1f(cam_xLoc_ssr, cam_x);
            glUniform1f(cam_yLoc_ssr, cam_y);
            glUniform1f(cam_zLoc_ssr, cam_z);
            GLuint groupX_ssr = ((screen_width / SSR_RES) + 31) / 32;
            GLuint groupY_ssr = ((screen_height / SSR_RES) + 31) / 32;
            glDispatchCompute(groupX_ssr, groupY_ssr, 1);
            CHECK_GL_ERROR();
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        }
        
        // 6. Render final meshes' results with full screen quad
        glUseProgram(imageBlitShaderProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, inputImageID);
        glProgramUniform1i(imageBlitShaderProgram, texLoc_quadblit, 0);
        glBindVertexArray(quadVAO);
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glEnable(GL_DEPTH_TEST);
        glBindTextureUnit(0, 0);
        glUseProgram(0);
        uint32_t drawCallsNormal = drawCallsRenderedThisFrame;
        
        // 7. Render UI Images
 
        // 8. Render UI Text;
        int textY = 25; int textVertOfset = 15;
        RenderFormattedText(10, textY, TEXT_WHITE, "x: %.2f, y: %.2f, z: %.2f", cam_x, cam_y, cam_z);
        RenderFormattedText(10, textY + (textVertOfset * 1), TEXT_WHITE, "cam yaw: %.2f, cam pitch: %.2f, cam roll: %.2f", cam_yaw, cam_pitch, cam_roll);
        RenderFormattedText(10, textY + (textVertOfset * 2), TEXT_WHITE, "Peak frame queue count: %d", maxEventCount_debug);
        RenderFormattedText(10, textY + (textVertOfset * 3), TEXT_WHITE, "DebugView: %d (%s), DebugValue: %d, Instances in PVS: %d", debugView, debugViewNames[debugView], debugValue, instancesInPVSCount);
        RenderFormattedText(10, textY + (textVertOfset * 4), TEXT_WHITE, "Num lights: %d, Num cells: %d, Player cell(%d):: x: %d, y: %d, z: %d", numLightsFound, numCellsVisible, playerCellIdx, playerCellIdx_x, playerCellIdx_y, playerCellIdx_z);
        RenderFormattedText(10, textY + (textVertOfset * 5), TEXT_WHITE, "SSR steps: %d, SSR step size: %f, SSR max dist: %f", ssr_StepCount, ssr_StepSize, ssr_MaxDist);
        RenderFormattedText(10, textY + (textVertOfset * 6), TEXT_WHITE, "Fog R: %f, G: %f, B: %f", fogColorR, fogColorG, fogColorB);
        
        // Frame stats
        double time_now = get_time();
        drawCallsRenderedThisFrame++; // Add one more for this text render ;)
        RenderFormattedText(10, textY - textVertOfset, TEXT_WHITE, "Frame time: %.6f (FPS: %d), Draw calls: %d [Geo %d, UI %d], Verts: %d, Worst FPS: %d",
                            (time_now - last_time) * 1000.0,framesPerLastSecond,drawCallsRenderedThisFrame,drawCallsNormal, drawCallsRenderedThisFrame - drawCallsNormal,verticesRenderedThisFrame,worstFPS);
        last_time = time_now;
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

        SDL_GL_SwapWindow(window); // Present frame
        CHECK_GL_ERROR();
        globalFrameNum++;
    }

    return 0;
}
