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
#include "event.h"
#include "data_parser.h"
#include "data_textures.h"
#include "data_models.h"
#include "data_entities.h"
#include "data_levels.h"
#include "render.h"
#include "text.glsl"
#include "chunk.glsl"
#include "imageblit.glsl"
#include "deferred_lighting.compute"
#include "bluenoise64.cginc"
#include "audio.h"
#include "instance.h"
#include "voxel.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "External/stb_image_write.h"
#include <enet/enet.h>
#include "constants.h"

// #define DEBUG_RAM_OUTPUT

// Window
SDL_Window *window;
int screen_width = 800, screen_height = 600;
bool window_has_focus = false;
FILE *console_log_file = NULL;

// Instances
Instance instances[INSTANCE_COUNT];
float modelMatrices[INSTANCE_COUNT * 16];
uint8_t dirtyInstances[INSTANCE_COUNT];
GLuint instancesBuffer;
GLuint instancesInPVSBuffer;
GLuint matricesBuffer;

// Game/Mod Definition
DataParser gamedata_parser;
uint8_t numLevels = 2;
uint8_t startLevel = 3;
const char *valid_gamedata_keys[] = {"levelcount","startlevel"};
#define NUM_GAMDAT_KEYS 2

// Camera variables
// Start Actual: Puts player on Medical Level in actual game start position
float cam_x = -6.44f, cam_y = 0.0f, cam_z = -2.56f; // Camera position Cornell box
// float cam_x = -20.40001f, cam_y = -43.52f, cam_z = 10.2f; // Camera position Unity
// float cam_x = -15.65f, cam_z = 25.54f, cam_y = 5.85f; // Camera position Voxen

typedef struct {
    float w, x, y, z;
} Quaternion;

Quaternion cam_rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
float cam_yaw = 0.0f;
float cam_pitch = 0.0f;
float cam_roll = 0.0f;
float cam_fov = 65.0f;

float deg2rad(float degrees) {
    return degrees * (M_PI / 180.0f);
}

float rad2deg(float radians) {
    return radians * (180.0f / M_PI);
}

float move_speed = 0.1f;
float mouse_sensitivity = 0.1f;
bool in_cyberspace = true;
float sprinting = 0.0f;

float testLight_x = -2.56f;
float testLight_z = -2.56f;
float testLight_y = 1.12f;
float testLight_intensity = 2.5f;
float testLight_range = 3.0f;
float testLight_spotAng = 0.0f;
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
double start_frame_time = 0.0;
double screenshotTimeout = 0.0;

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
uint32_t playerCellIdx = 80000;
uint32_t playerCellIdx_x = 20000;
uint32_t playerCellIdx_y = 10000;
uint32_t playerCellIdx_z = 451;
uint8_t numLightsFound = 0;

// Shaders
GLuint chunkShaderProgram;
GLuint vao_chunk; // Vertex Array Object
GLint viewLoc_chunk = -1, projectionLoc_chunk = -1, matrixLoc_chunk = -1, texIndexLoc_chunk = -1,
      instanceIndexLoc_chunk = -1, modelIndexLoc_chunk = -1, debugViewLoc_chunk = -1, glowIndexLoc_chunk = -1,
      specIndexLoc_chunk = -1, instancesInPVSCount_chunk = -1, overrideGlowRLoc_chunk = -1, overrideGlowGLoc_chunk = -1, overrideGlowBLoc_chunk = -1; // uniform locations

//    Deferred Lighting Compute Shader
GLuint deferredLightingShaderProgram;
GLuint inputImageID, inputNormalsID, inputDepthID, inputWorldPosID, gBufferFBO, outputImageID; // FBO
GLint screenWidthLoc_deferred = -1, screenHeightLoc_deferred = -1, debugViewLoc_deferred = -1, sphoxelCountLoc_deferred = -1; // uniform locations

//    Full Screen Quad Blit for rendering final output/image effect passes
GLuint imageBlitShaderProgram;
GLuint quadVAO, quadVBO;
GLint texLoc_quadblit = -1, debugViewLoc_quadblit = -1, debugValueLoc_quadblit = -1; // uniform locations

// Lights
// Could reduce spotAng to minimal bits.  I only have 6 spot lights and half are 151.7 and other half are 135.
float lights[LIGHT_COUNT * LIGHT_DATA_SIZE];
bool lightDirty[MAX_VISIBLE_LIGHTS] = { [0 ... MAX_VISIBLE_LIGHTS-1] = true };
float lightsRangeSquared[LIGHT_COUNT];
GLuint lightVBOs[MAX_VISIBLE_LIGHTS];
GLuint lightIBOs[MAX_VISIBLE_LIGHTS];
uint32_t lightVertexCounts[MAX_VISIBLE_LIGHTS];
uint32_t lightIndexCounts[MAX_VISIBLE_LIGHTS];
float lightsInProximity[MAX_VISIBLE_LIGHTS * LIGHT_DATA_SIZE];
GLuint visibleLightsID;

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

    // Deferred Lighting Compute Shader Program
    computeShader = CompileShader(GL_COMPUTE_SHADER, deferredLighting_computeShader, "Deferred Lighting Compute Shader"); if (!computeShader) { return 1; }
    deferredLightingShaderProgram = LinkProgram((GLuint[]){computeShader}, 1, "Deferred Lighting Shader Program");        if (!deferredLightingShaderProgram) { return 1; }

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

    screenWidthLoc_deferred = glGetUniformLocation(deferredLightingShaderProgram, "screenWidth");
    screenHeightLoc_deferred = glGetUniformLocation(deferredLightingShaderProgram, "screenHeight");
    debugViewLoc_deferred = glGetUniformLocation(deferredLightingShaderProgram, "debugView");
    sphoxelCountLoc_deferred = glGetUniformLocation(deferredLightingShaderProgram, "sphoxelCount");
    
    texLoc_quadblit = glGetUniformLocation(imageBlitShaderProgram, "tex");
    debugViewLoc_quadblit = glGetUniformLocation(imageBlitShaderProgram, "debugView");
    debugValueLoc_quadblit = glGetUniformLocation(imageBlitShaderProgram, "debugValue");
    
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
        if (debugView > 6) debugView = 0;
    }

    if (keys[SDL_SCANCODE_Y]) {
        debugValue++;
        if (debugValue > 5) debugValue = 0;
    }

    if (keys[SDL_SCANCODE_O]) {
        testLight_intensity += 8.0f / 256.0f;
        if (testLight_intensity > 8.0f) testLight_intensity = 8.0f;
    } else if (keys[SDL_SCANCODE_P]) {
        testLight_intensity -= 8.0f / 256.0f;
        if (testLight_intensity < 0.01f) testLight_intensity = 0.01f;
    }

    if (keys[SDL_SCANCODE_E]) {
        play_wav("./Audio/weapons/wpistol.wav",0.5f);
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

// Update camera position based on input
void ProcessInput(void) {
    if (keys[SDL_SCANCODE_LSHIFT]) sprinting = 1.0f;
    else sprinting = 0.0f;

    float rotation[16]; // Extract forward and right vectors from quaternion
    quat_to_matrix(&cam_rotation, rotation);
    float facing_x = rotation[8];  // Forward X
    float facing_y = rotation[9];  // Forward Y
    float facing_z = rotation[10]; // Forward Z
    float strafe_x = rotation[0];  // Right X
    float strafe_y = rotation[1];  // Right Y
    float strafe_z = rotation[2];  // Right Z
    normalize_vector(&facing_x, &facing_y, &facing_z); // Normalize forward
    normalize_vector(&strafe_x, &strafe_y, &strafe_z); // Normalize strafe
    float finalMoveSpeed = (move_speed + (sprinting * move_speed));
    if (keys[SDL_SCANCODE_F]) {
        cam_x += finalMoveSpeed * facing_x; // Move forward
        cam_y += finalMoveSpeed * facing_y;
        cam_z += finalMoveSpeed * facing_z;
    } else if (keys[SDL_SCANCODE_S]) {
        cam_x -= finalMoveSpeed * facing_x; // Move backward
        cam_y -= finalMoveSpeed * facing_y;
        cam_z -= finalMoveSpeed * facing_z;
    }

    if (keys[SDL_SCANCODE_D]) {
        cam_x += finalMoveSpeed * strafe_x; // Strafe right
        cam_y += finalMoveSpeed * strafe_y;
        cam_z += finalMoveSpeed * strafe_z;
    } else if (keys[SDL_SCANCODE_A]) {
        cam_x -= finalMoveSpeed * strafe_x; // Strafe left
        cam_y -= finalMoveSpeed * strafe_y;
        cam_z -= finalMoveSpeed * strafe_z;
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

    if (keys[SDL_SCANCODE_K]) {
        testLight_x += finalMoveSpeed;
        lightDirty[0] = true;
    } else if (keys[SDL_SCANCODE_J]) {
        testLight_x -= finalMoveSpeed;
        lightDirty[0] = true;
    }

    if (keys[SDL_SCANCODE_N]) {
        testLight_y += finalMoveSpeed;
        lightDirty[0] = true;
    } else if (keys[SDL_SCANCODE_M]) {
        testLight_y -= finalMoveSpeed;
        lightDirty[0] = true;
    }

    if (keys[SDL_SCANCODE_U]) {
        testLight_z += finalMoveSpeed;
        lightDirty[0] = true;
    } else if (keys[SDL_SCANCODE_I]) {
        testLight_z -= finalMoveSpeed;
        lightDirty[0] = true;
    }

    if (keys[SDL_SCANCODE_L]) {
        testLight_range += finalMoveSpeed;
        lightDirty[0] = true;
    } else if (keys[SDL_SCANCODE_SEMICOLON]) {
        testLight_range -= finalMoveSpeed;
        if (testLight_range < 0.0f) testLight_range = 0.0f;
        else lightDirty[0] = true;
    }

    if (keys[SDL_SCANCODE_B]) {
        testLight_spotAng += finalMoveSpeed * 2.0f;
        if (testLight_spotAng > 180.0f) testLight_spotAng = 180.0f;
    } else if (keys[SDL_SCANCODE_Z]) {
        testLight_spotAng -= finalMoveSpeed * 2.0f;
        if (testLight_spotAng < 0.0f) testLight_spotAng = 0.0f;
    }

    if (keys[SDL_SCANCODE_X]) {
        for (int i=0;i<MAX_VISIBLE_LIGHTS;++i) lightDirty[i] = true;
    }
}

// ============================================================================
uint32_t Flatten3DIndex(int x, int y, int z, int xMax, int yMax) {
    return (uint32_t)(x + (y * xMax) + (z * xMax * yMax));
}

void WorldCellIndexToPosition(uint32_t worldIdx, float * x, float * z, float * y) { // Swapped to reflect Unity coordinate system
    *x = (worldIdx % WORLDCELL_X_MAX) * WORLDCELL_WIDTH_F + WORLDCELL_WIDTH_F / 2.0f;
    *y = ((worldIdx / WORLDCELL_X_MAX) % WORLDCELL_Y_MAX) * WORLDCELL_WIDTH_F + WORLDCELL_WIDTH_F / 2.0f;
    *z = (worldIdx / (WORLDCELL_X_MAX * WORLDCELL_Y_MAX)) * WORLDCELL_WIDTH_F + WORLDCELL_WIDTH_F / 2.0f;
}

uint32_t PositionToWorldCellIndexX(float x) {
    float cellHalf = WORLDCELL_WIDTH_F / 2.0f;
    int xi = (int)floorf((x + cellHalf) / WORLDCELL_WIDTH_F);
    return (xi < 0 ? 0 : (xi >= WORLDCELL_X_MAX ? WORLDCELL_X_MAX - 1 : xi));
}

uint32_t PositionToWorldCellIndexY(float y) {
    float cellHalf = WORLDCELL_WIDTH_F / 2.0f;
    int yi = (int)floorf((y + cellHalf) / WORLDCELL_WIDTH_F);
    return (yi < 0 ? 0 : (yi >= WORLDCELL_Y_MAX ? WORLDCELL_Y_MAX - 1 : yi));
}

uint32_t PositionToWorldCellIndexZ(float z) {
    float cellHalf = WORLDCELL_WIDTH_F / 2.0f;
    int zi = (int)floorf((z + cellHalf) / WORLDCELL_WIDTH_F);
    return (zi < 0 ? 0 : (zi >= WORLDCELL_Z_MAX ? WORLDCELL_Z_MAX - 1 : zi));
}

uint32_t PositionToWorldCellIndex(float x, float y, float z) {
    float cellHalf = WORLDCELL_WIDTH_F / 2.0f;
    int xi = (int)floorf((x + cellHalf) / WORLDCELL_WIDTH_F);
    int yi = (int)floorf((y + cellHalf) / WORLDCELL_WIDTH_F);
    int zi = (int)floorf((z + cellHalf) / WORLDCELL_WIDTH_F);
    xi = xi < 0 ? 0 : (xi >= WORLDCELL_X_MAX ? WORLDCELL_X_MAX - 1 : xi);
    yi = yi < 0 ? 0 : (yi >= WORLDCELL_Y_MAX ? WORLDCELL_Y_MAX - 1 : yi);
    zi = zi < 0 ? 0 : (zi >= WORLDCELL_Z_MAX ? WORLDCELL_Z_MAX - 1 : zi);
    return Flatten3DIndex(xi, yi, zi, WORLDCELL_X_MAX, WORLDCELL_Y_MAX);
}

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
    if (instances[i].modelIndex >= MODEL_COUNT) { dirtyInstances[i] = false; return; }
    if (modelVertexCounts[instances[i].modelIndex] < 1) { dirtyInstances[i] = false; return; } // Empty model
    if (instances[i].modelIndex < 0) return; // Culled

    float mat[16]; // 4x4 matrix
    SetUpdatedMatrix(mat, instances[i].posx, instances[i].posy, instances[i].posz, instances[i].rotx, instances[i].roty, instances[i].rotz, instances[i].rotw, instances[i].sclx, instances[i].scly, instances[i].sclz);
    memcpy(&modelMatrices[i * 16], mat, 16 * sizeof(float));
    dirtyInstances[i] = false;
}

int SetupInstances(void) {
    DualLog("Initializing instances\n");
    CHECK_GL_ERROR();
    int x,z,idx;
    for (idx = 0, x = 0, z = 0;idx<INSTANCE_COUNT;idx++) {
        int entIdx = idx < MAX_ENTITIES ? idx : 0;
        instances[idx].modelIndex = entities[entIdx].modelIndex;
        instances[idx].texIndex = entities[entIdx].texIndex;
        instances[idx].glowIndex = entities[entIdx].glowIndex;
        instances[idx].specIndex = entities[entIdx].specIndex;
        instances[idx].normIndex = entities[entIdx].normIndex;
        instances[idx].lodIndex = entities[entIdx].lodIndex;
        instances[idx].posx = ((float)x * 2.56f);
        instances[idx].posy = 0.0f;
        instances[idx].posz = ((float)z * 5.12f);
        instances[idx].sclx = instances[idx].scly = instances[idx].sclz = 1.0f; // Default scale
        instances[idx].rotx = instances[idx].roty = instances[idx].rotz = 0.0f;
        instances[idx].rotw = 1.0f;
        x++;
        if (idx == 100 || idx == 200 || idx == 300 || idx == 400 || idx == 500
            || idx == 600 || idx == 700 || idx == 800 || idx == 900) {

            x = 0;
            z++;
        }

        dirtyInstances[idx] = true;
    }
    
    idx = 5455;  // Test light representative, not actually the light, moves with it
    instances[idx].modelIndex = 621; // Test Light Sphere
    instances[idx].texIndex = 881; // white light
    instances[idx].glowIndex = 881; // white light
    instances[idx].sclx = 0.4f;
    instances[idx].scly = 0.4f;
    instances[idx].sclz = 0.4f;

    glGenBuffers(1, &instancesBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, instancesBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, INSTANCE_COUNT * sizeof(Instance), instances, GL_DYNAMIC_DRAW);
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
        lights[base + 1] = 0.0f; // posy
        lights[base + 2] = base * 0.64f; // posz
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
    lights[2] = 0.0f;
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
    lights[1 + 12] = 0.0f; // Fixed Y height
    lights[2 + 12] = 0.0f;
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
    GenerateAndBindTexture(&inputImageID,             GL_RGBA8, screen_width, screen_height,            GL_RGBA,           GL_UNSIGNED_BYTE, GL_TEXTURE_2D, "Lit Raster");
    GenerateAndBindTexture(&inputWorldPosID,        GL_RGBA32F, screen_width, screen_height,            GL_RGBA,                   GL_FLOAT, GL_TEXTURE_2D, "Raster World Positions");
    GenerateAndBindTexture(&inputNormalsID,         GL_RGBA32F, screen_width, screen_height,            GL_RGBA,                   GL_FLOAT, GL_TEXTURE_2D, "Raster Normals");
    GenerateAndBindTexture(&inputDepthID, GL_DEPTH_COMPONENT24, screen_width, screen_height, GL_DEPTH_COMPONENT,            GL_UNSIGNED_INT, GL_TEXTURE_2D, "Raster Depth");
    GenerateAndBindTexture(&outputImageID,          GL_RGBA32F, screen_width, screen_height,            GL_RGBA,                   GL_FLOAT, GL_TEXTURE_2D, "Deferred Lighting Result Colors");
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
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CHECK_GL_ERROR();
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
    glGenBuffers(1, &visibleLightsID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, visibleLightsID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 32 * LIGHT_DATA_SIZE * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 19, visibleLightsID);
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
    
    // Load Game/Mod Definitio
    DualLog("Loading game definition from ./Data/gamedata.txt...\n");
    parser_init(&gamedata_parser, valid_gamedata_keys, NUM_GAMDAT_KEYS, PARSER_GAME);
    if (!parse_data_file(&gamedata_parser, "./Data/gamedata.txt")) { DualLogError("Could not parse ./Data/gamedata.txt!\n"); parser_free(&gamedata_parser); return 1; }
    numLevels = gamedata_parser.entries[0].levelCount;
    startLevel = gamedata_parser.entries[0].startLevel;
    DualLog("Game Definition:: num levels: %d, start level=%d\n",numLevels,startLevel);
    parser_free(&gamedata_parser);
    
    DebugRAM("InitializeEnvironment end");
    return 0;
}

// All core engine operations run through the EventExecute as an Event processed
// by the unified event system in the order it was enqueued.
int EventExecute(Event* event) {
    switch(event->type) {
        case EV_INIT: return InitializeEnvironment(); // Init called prior to Loading Data
        case EV_LOAD_TEXTURES: return LoadTextures();
        case EV_LOAD_MODELS: return LoadGeometry();
        case EV_LOAD_ENTITIES: return LoadEntities();
        case EV_LOAD_LEVELS: LoadLevels(); return 0;
        case EV_LOAD_INSTANCES: return SetupInstances();
        case EV_KEYDOWN: return Input_KeyDown(event->payload1u);
        case EV_KEYUP: return Input_KeyUp(event->payload1u);
        case EV_MOUSEMOVE: return Input_MouseMove(event->payload1f,event->payload2f);
        case EV_QUIT: return 1; break;
    }

    return 99; // Something went wrong
}

static const char* debugViewNames[] = {
    "standard render", // 0
    "unlit",           // 1
    "surface normals", // 2
    "depth",           // 3
    "indices",         // 4
    "worldpos",        // 5
    "lightview"        // 6
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

    // Queue order for loading is IMPORTANT!
    EnqueueEvent_Simple(EV_INIT);
    EnqueueEvent_Simple(EV_LOAD_TEXTURES);
    EnqueueEvent_Simple(EV_LOAD_MODELS);
    EnqueueEvent_Simple(EV_LOAD_ENTITIES); // Must be after models and textures else entity types can't be validated.
    EnqueueEvent_Simple(EV_LOAD_INSTANCES);
    EnqueueEvent_Simple(EV_LOAD_LEVELS); // Must be after entities!
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
        if (EventQueueProcess()) break; // Do everything
        
        // Client Actions
        // ====================================================================
        // Client Render
        drawCallsRenderedThisFrame = 0; // Reset per frame
        verticesRenderedThisFrame = 0;
        
        // 0. Clear Frame Buffers and Depth
        glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear main FBO
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear screen
        
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
        int testLightInstanceIdx = 5455;
        instances[testLightInstanceIdx].posx = testLight_x;
        instances[testLightInstanceIdx].posy = testLight_y;
        instances[testLightInstanceIdx].posz = testLight_z;
        instances[testLightInstanceIdx].sclx = testLight_range * 0.04f;
        instances[testLightInstanceIdx].scly = testLight_range * 0.04f;
        instances[testLightInstanceIdx].sclz = testLight_range * 0.04f;
        dirtyInstances[testLightInstanceIdx] = true;
        
        // 1. Light Culling to limit of MAX_VISIBLE_LIGHTS
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
        glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_VISIBLE_LIGHTS * LIGHT_DATA_SIZE * sizeof(float), lightsInProximity, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 19, visibleLightsID);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        // 2. Instance Culling to only those in range of player
        bool instanceIsCulledArray[INSTANCE_COUNT];
        bool instanceIsLODArray[INSTANCE_COUNT];
        memset(instanceIsCulledArray,true,INSTANCE_COUNT * sizeof(bool)); // All culled.
        memset(instanceIsLODArray,true,INSTANCE_COUNT * sizeof(bool)); // All using lower detail LOD mesh.
        float distSqrd = 0.0f;
        float lodRangeSqrd = 20.48f * 20.48f;
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
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, matricesBuffer);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, INSTANCE_COUNT * 16 * sizeof(float), modelMatrices); // * 16 because matrix4x4
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, matricesBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, instancesInPVSBuffer);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, instancesInPVSCount * sizeof(uint32_t), instancesInPVS); // * 16 because matrix4x4
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, instancesInPVSBuffer);
        
        // 4. Raterized Geometry
        //        Standard vertex + fragment rendering, but with special packing to minimize transfer data amounts
        glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
        glUseProgram(chunkShaderProgram);
        glEnable(GL_DEPTH_TEST);
        float view[16]; // Set up view matrices
        mat4_lookat(view);
        glUniformMatrix4fv(viewLoc_chunk,       1, GL_FALSE,       view);
        glUniformMatrix4fv(projectionLoc_chunk, 1, GL_FALSE, rasterPerspectiveProjection);
        glUniform1i(debugViewLoc_chunk, debugView);
        glBindVertexArray(vao_chunk);
        glUniform1f(overrideGlowRLoc_chunk, 0.0f);
        glUniform1f(overrideGlowGLoc_chunk, 0.0f);
        glUniform1f(overrideGlowBLoc_chunk, 0.0f);
        for (uint16_t i=0;i<INSTANCE_COUNT;i++) {
            if (instanceIsCulledArray[i]) continue;
            if (instances[i].modelIndex >= MODEL_COUNT) continue;
            if (modelVertexCounts[instances[i].modelIndex] < 1) continue; // Empty model
            if (instances[i].modelIndex < 0) continue; // Culled
            if (debugView == 6 && i == testLightInstanceIdx) continue; // TODO: Delete me
            
            if (dirtyInstances[i]) UpdateInstanceMatrix(i);            
            glUniform1i(texIndexLoc_chunk, instances[i].texIndex);
            glUniform1i(glowIndexLoc_chunk, instances[i].glowIndex);
            glUniform1i(specIndexLoc_chunk, instances[i].specIndex);
            glUniform1i(instanceIndexLoc_chunk, i);
            int modelType = instanceIsLODArray[i] && instances[i].lodIndex < UINT16_MAX ? instances[i].lodIndex : instances[i].modelIndex;
            glUniform1i(modelIndexLoc_chunk, modelType);
            glUniformMatrix4fv(matrixLoc_chunk, 1, GL_FALSE, &modelMatrices[i * 16]);
            glBindVertexBuffer(0, vbos[modelType], 0, VERTEX_ATTRIBUTES_COUNT * sizeof(float));
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tbos[modelType]);
            if (isDoubleSided(instances[i].texIndex)) glDisable(GL_CULL_FACE); // Disable backface culling
            glDrawElements(GL_TRIANGLES, modelTriangleCounts[modelType] * 3, GL_UNSIGNED_INT, 0);
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
                glUniform1f(overrideGlowGLoc_chunk, lightsInProximity[idx + LIGHT_DATA_OFFSET_G] * lightsInProximity[idx + LIGHT_DATA_OFFSET_INTENSITY]);
                glUniform1f(overrideGlowBLoc_chunk, lightsInProximity[idx + LIGHT_DATA_OFFSET_B] * lightsInProximity[idx + LIGHT_DATA_OFFSET_INTENSITY]);
                glUniform1i(texIndexLoc_chunk, 41);
                glUniform1i(glowIndexLoc_chunk, 41);
                glUniform1i(specIndexLoc_chunk, 41);
                glUniform1i(instanceIndexLoc_chunk, i);
                int modelType = 621; // Test light icosphere
                glUniform1i(modelIndexLoc_chunk, modelType);
                glUniformMatrix4fv(matrixLoc_chunk, 1, GL_FALSE, mat);
                glBindVertexBuffer(0, vbos[modelType], 0, VERTEX_ATTRIBUTES_COUNT * sizeof(float));
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tbos[modelType]);
                glDrawElements(GL_TRIANGLES, modelTriangleCounts[modelType] * 3, GL_UNSIGNED_INT, 0);
                drawCallsRenderedThisFrame++;
                verticesRenderedThisFrame += modelTriangleCounts[modelType] * 3;
           }
        }
        
        glDisable(GL_BLEND);

        // ====================================================================
        // Ok, turn off temporary framebuffer so we can draw to screen now.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // ====================================================================
        
        // 5. Deferred Lighting
        //        Apply deferred lighting with compute shader.  All lights are
        //        dynamic and can be updated at any time (flicker, light switches,
        //        move, change color, get marked as "culled" so shader can skip it,
        //        etc.).
        if (debugView != 2) {
            glUseProgram(deferredLightingShaderProgram);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

            // These should be static but cause issues if not...
            glUniform1ui(screenWidthLoc_deferred, screen_width); // Makes screen all black if not sent every frame.
            glUniform1ui(screenHeightLoc_deferred, screen_height); // Makes screen all black if not sent every frame.
            glUniform1i(debugViewLoc_deferred, debugView);

            // Dispatch compute shader
            GLuint groupX = (screen_width + 31) / 32;
            GLuint groupY = (screen_height + 31) / 32;
            glDispatchCompute(groupX, groupY, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT); // Runs slightly faster 0.1ms without this, but may need if more shaders added in between
        }
        
        // 6. Render final meshes' results with full screen quad
        glUseProgram(imageBlitShaderProgram);
        glActiveTexture(GL_TEXTURE0);
        if (debugView == 0) {
            glBindTexture(GL_TEXTURE_2D, outputImageID); // Forward + GI
        } else { // 1,2,3,4,5,6
            glBindTexture(GL_TEXTURE_2D, inputImageID); // Forward Pass Debug Views
        }

        glProgramUniform1i(imageBlitShaderProgram, texLoc_quadblit, 0);
        glProgramUniform1i(imageBlitShaderProgram, debugViewLoc_quadblit, debugView);
        glProgramUniform1i(imageBlitShaderProgram, debugValueLoc_quadblit, debugValue);
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
        RenderFormattedText(10, textY + (textVertOfset * 3), TEXT_WHITE, "testLight intensity: %.4f, range: %.4f, spotAng: %.4f, x: %.3f, y: %.3f, z: %.3f", testLight_intensity,testLight_range,testLight_spotAng,testLight_x,testLight_y,testLight_z);
        RenderFormattedText(10, textY + (textVertOfset * 4), TEXT_WHITE, "DebugView: %d (%s), DebugValue: %d, Instances in PVS: %d", debugView, debugViewNames[debugView], debugValue, instancesInPVSCount);
        RenderFormattedText(10, textY + (textVertOfset * 5), TEXT_WHITE, "Num lights: %d, Player cell:: x: %d, y: %d, z: %d", numLightsFound, playerCellIdx_x, playerCellIdx_y, playerCellIdx_z);
        
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
