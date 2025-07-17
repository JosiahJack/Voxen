// File: voxen.c
// Description: A realtime OpenGL based application for experimenting with voxel lighting techniques to derive new methods of high speed accurate lighting in resource constrained environements (e.g. embedded).
#include <malloc.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
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
#include "data_entities.h"
#include "player.h"
#include "matrix.h"

// #define DEBUG_RAM_OUTPUT

// Window
SDL_Window *window;
int screen_width = 800, screen_height = 600;
bool window_has_focus = false;
static FILE *console_log_file = NULL;

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
         
// ----------------------------------------------------------------------------
// OpenGL / Rendering
SDL_GLContext gl_context;

double lastFrameSecCountTime = 0.00;
uint32_t lastFrameSecCount = 0;
uint32_t framesPerLastSecond = 0;
uint32_t worstFPS = UINT32_MAX;
uint32_t drawCallCount = 0;
uint32_t vertexCount = 0;
bool shadowsEnabled = false;
uint32_t playerCellIdx = 80000;
uint32_t playerCellIdx_x = 20000;
uint32_t playerCellIdx_y = 10000;
uint32_t playerCellIdx_z = 451;
int numLightsFound = 0;
float sightRangeSquared = 71.68f * 71.68f; // Max player view, level 6 crawlway 28 cells
uint32_t maxLightVolumeMeshVerts = 65535;

GLuint vao_chunk; // Vertex Array Object

// Shaders
GLuint chunkShaderProgram;
GLint viewLoc_chunk = -1, projectionLoc_chunk = -1, matrixLoc_chunk = -1, texIndexLoc_chunk = -1,
      instanceIndexLoc_chunk = -1, modelIndexLoc_chunk = -1, debugViewLoc_chunk = -1, glowIndexLoc_chunk = -1,
      specIndexLoc_chunk = -1; // uniform locations

GLuint imageBlitShaderProgram;
GLuint quadVAO, quadVBO;

GLuint deferredLightingShaderProgram;
GLuint inputImageID, inputNormalsID, inputDepthID, inputWorldPosID, gBufferFBO, inputTexMapsID; // FBO inputs
GLint screenWidthLoc_deferred = -1, screenHeightLoc_deferred = -1, shadowsEnabledLoc_deferred = -1,
      debugViewLoc_deferred = -1; // uniform locations

// Lights
// Could reduce spotAng to minimal bits.  I only have 6 spot lights and half are 151.7 and other half are 135.
float lights[LIGHT_COUNT * LIGHT_DATA_SIZE];
bool lightDirty[LIGHT_COUNT] = { [0 ... LIGHT_COUNT-1] = true };
float lightsRangeSquared[LIGHT_COUNT];
// GLuint lightVolumeMeshShaderProgram; // Compute shader to make the mesh
GLuint lightVolumeShaderProgram;     // vert + frag shader to render it
GLint lightPosXLoc_lightvol = -1, lightPosYLoc_lightvol = -1, lightPosZLoc_lightvol = -1, lightRangeLoc_lightvol = -1,
      matrix_lightvol = -1, view_lightvol = -1, projection_lightvol = -1, debugView_lightvol = -1; // uniform locations
GLuint lightVBOs[MAX_VISIBLE_LIGHTS];
uint32_t lightVertexCounts[MAX_VISIBLE_LIGHTS];
float lightsInProximity[32 * LIGHT_DATA_SIZE];
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
    fprintf(stderr, "\033[1;31mERROR: \033[0;31m"); vfprintf(stderr, fmt, args1); fprintf(stderr,"\033[0;0m"); fflush(stderr); // Print to console/terminal.
    va_end(args1);
    if (console_log_file) { fprintf(console_log_file, "ERROR: "); vfprintf(console_log_file, fmt, args2); fflush(console_log_file); } // Print to log file.
    va_end(args2);
}

// Get RSS aka the total RAM reported by btop or other utilities that's allocated virtual ram for the process.
size_t get_rss_bytes(void) {
    FILE *fp = fopen("/proc/self/stat", "r");
    if (!fp) {
        DualLogError("Failed to open /proc/self/stat\n");
        return 0;
    }

    char buffer[1024];
    if (!fgets(buffer, sizeof(buffer), fp)) {
        DualLogError("Failed to read /proc/self/stat\n");
        fclose(fp);
        return 0;
    }
    fclose(fp);

    // Parse the 24th field (RSS in pages)
    char *token = strtok(buffer, " ");
    int field = 0;
    size_t rss_pages = 0;
    while (token && field < 23) {
        token = strtok(NULL, " ");
        field++;
    }
    if (token) {
        rss_pages = atol(token);
    } else {
        DualLogError("Failed to parse RSS from /proc/self/stat\n");
        return 0;
    }

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
    
    screenWidthLoc_deferred = glGetUniformLocation(deferredLightingShaderProgram, "screenWidth");
    screenHeightLoc_deferred = glGetUniformLocation(deferredLightingShaderProgram, "screenHeight");
    shadowsEnabledLoc_deferred = glGetUniformLocation(deferredLightingShaderProgram, "shadowsEnabled");
    debugViewLoc_deferred = glGetUniformLocation(deferredLightingShaderProgram, "debugView");
    
//     lightPosXLoc_lightvol = glGetUniformLocation(lightVolumeMeshShaderProgram, "lightPosX");
//     lightPosYLoc_lightvol = glGetUniformLocation(lightVolumeMeshShaderProgram, "lightPosY");
//     lightPosZLoc_lightvol = glGetUniformLocation(lightVolumeMeshShaderProgram, "lightPosZ");
//     lightRangeLoc_lightvol = glGetUniformLocation(lightVolumeMeshShaderProgram, "lightRange");
    
    matrix_lightvol = glGetUniformLocation(lightVolumeShaderProgram, "matrix");
    view_lightvol = glGetUniformLocation(lightVolumeShaderProgram, "view");
    projection_lightvol = glGetUniformLocation(lightVolumeShaderProgram, "projection");
    debugView_lightvol = glGetUniformLocation(lightVolumeShaderProgram, "debugView");
}

void UpdateInstanceMatrix(int i) {
    if (instances[i].modelIndex >= MODEL_COUNT) { dirtyInstances[i] = false; return; }
    if (modelVertexCounts[instances[i].modelIndex] < 1) { dirtyInstances[i] = false; return; } // Empty model
    if (instances[i].modelIndex < 0) return; // Culled
    
    float x = instances[i].rotx, y = instances[i].roty, z = instances[i].rotz, w = instances[i].rotw;
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
    
    float mat[16]; // 4x4 matrix
    mat[0]  =       rot[0] * sx; mat[1]  =       rot[1] * sy; mat[2] =       rot[2]  * sz; mat[3]  = 0.0f;
    mat[4]  =       rot[4] * sx; mat[5]  =       rot[5] * sy; mat[6]  =      rot[6]  * sz; mat[7]  = 0.0f;
    mat[8]  =       rot[8] * sx; mat[9]  =       rot[9] * sy; mat[10] =      rot[10] * sz; mat[11] = 0.0f;
    mat[12] = instances[i].posx; mat[13] = instances[i].posy; mat[14] = instances[i].posz; mat[15] = 1.0f;
    memcpy(&modelMatrices[i * 16], mat, 16 * sizeof(float));
    dirtyInstances[i] = false;
}

void UpdateLightVolumes(void) {
    memset(lightVBOs, 0, MAX_VISIBLE_LIGHTS * sizeof(GLuint));
    glGenBuffers(MAX_VISIBLE_LIGHTS, lightVBOs);
    size_t sizeOfTempVertices = largestVertCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float);
    float *tempVerts = (float *)malloc(sizeOfTempVertices);
    if (!tempVerts) { DualLogError("Failed to allocate tempVerts buffer\n"); return; }
    
    memset(lightVertexCounts, 0, MAX_VISIBLE_LIGHTS * sizeof(uint32_t));
    double start_time = get_time();
    for (uint32_t lightIdx = 0; lightIdx < 1; ++lightIdx) {
        uint32_t litIdx = (lightIdx * LIGHT_DATA_SIZE);
        float lit_x = lights[litIdx + LIGHT_DATA_OFFSET_POSX];
        float lit_y = lights[litIdx + LIGHT_DATA_OFFSET_POSY];
        float lit_z = lights[litIdx + LIGHT_DATA_OFFSET_POSZ];
        float sqrRange = lights[litIdx + LIGHT_DATA_OFFSET_RANGE]; sqrRange *= sqrRange; // Square it
        uint32_t headVertIdx = 0;
        for (uint32_t instanceIdx = 0; instanceIdx < INSTANCE_COUNT; ++instanceIdx) {
            if (instanceIdx == 39) continue; // Skip test light. TODO: Remove once done with test light!
            
            int32_t modelIdx = instances[instanceIdx].modelIndex;
            if (modelIdx < 0 || modelIdx >= MODEL_COUNT) continue;

            uint32_t vertCount = modelVertexCounts[modelIdx];
            if (!vertexDataArrays[modelIdx]) { DualLogError("vertexDataArrays[%d] is NULL\n", modelIdx); continue; }

            // Match UpdateInstanceMatrix exactly
            float x = instances[instanceIdx].rotx, y = instances[instanceIdx].roty, z = instances[instanceIdx].rotz, w = instances[instanceIdx].rotw;
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
            float sx = instances[instanceIdx].sclx > 0.0f ? instances[instanceIdx].sclx : 1.0f;
            float sy = instances[instanceIdx].scly > 0.0f ? instances[instanceIdx].scly : 1.0f;
            float sz = instances[instanceIdx].sclz > 0.0f ? instances[instanceIdx].sclz : 1.0f;
            float transform[16];
            transform[0]  = rot[0] * sx; transform[1]  = rot[1] * sy; transform[2]  = rot[2] * sz; transform[3]  = 0.0f;
            transform[4]  = rot[4] * sx; transform[5]  = rot[5] * sy; transform[6]  = rot[6] * sz; transform[7]  = 0.0f;
            transform[8]  = rot[8] * sx; transform[9]  = rot[9] * sy; transform[10] = rot[10] * sz; transform[11] = 0.0f;
            transform[12] = instances[instanceIdx].posx;
            transform[13] = instances[instanceIdx].posy;
            transform[14] = instances[instanceIdx].posz;
            transform[15] = 1.0f;
            for (uint32_t vertIdx = 0; (vertIdx < vertCount) && (headVertIdx < maxLightVolumeMeshVerts - 2); vertIdx += 3) {
                uint32_t vertexIdx = (vertIdx * VERTEX_ATTRIBUTES_COUNT);
                float *model = vertexDataArrays[modelIdx];

                float world_pos[3][3], model_pos[3][3];
                bool anyInRange = false;
                float min_x = 1e9, max_x = -1e9, min_y = 1e9, max_y = -1e9, min_z = 1e9, max_z = -1e9;
                
                #pragma GCC unroll 3
                for (int i = 0; i < 3; ++i) {
                    model_pos[i][0] = model[vertexIdx + i * VERTEX_ATTRIBUTES_COUNT + 0];
                    model_pos[i][1] = model[vertexIdx + i * VERTEX_ATTRIBUTES_COUNT + 1];
                    model_pos[i][2] = model[vertexIdx + i * VERTEX_ATTRIBUTES_COUNT + 2];
                    mat4_transform_vec3(transform, model_pos[i][0], model_pos[i][1], model_pos[i][2], world_pos[i]);
                    if (world_pos[i][0] < min_x) min_x = world_pos[i][0];
                    if (world_pos[i][0] > max_x) max_x = world_pos[i][0];
                    if (world_pos[i][1] < min_y) min_y = world_pos[i][1];
                    if (world_pos[i][1] > max_y) max_y = world_pos[i][1];
                    if (world_pos[i][2] < min_z) min_z = world_pos[i][2];
                    if (world_pos[i][2] > max_z) max_z = world_pos[i][2];
                    float dist = squareDistance3D(lit_x, lit_y, lit_z, world_pos[i][0], world_pos[i][1], world_pos[i][2]);
                    if (dist < sqrRange) anyInRange = true; // Can't break early, breaks tris!  Need to transform all 3 first!
                }

                if (__builtin_expect(!!(!anyInRange), 0)) continue;
                
                if (headVertIdx + 3 > largestVertCount) { DualLogError("headVertIdx (%u) exceeds largestVertCount (%u)\n", headVertIdx + 3, largestVertCount); break; }
                
                #pragma GCC unroll 3
                for (int i = 0; i < 3; ++i) {
                    uint32_t workingIdx = (headVertIdx + i) * VERTEX_ATTRIBUTES_COUNT;
                    tempVerts[workingIdx + 0] = world_pos[i][0];
                    tempVerts[workingIdx + 1] = world_pos[i][1];
                    tempVerts[workingIdx + 2] = world_pos[i][2];
                    tempVerts[workingIdx + 3] = model[vertexIdx + i * VERTEX_ATTRIBUTES_COUNT + 3];
                    tempVerts[workingIdx + 4] = model[vertexIdx + i * VERTEX_ATTRIBUTES_COUNT + 4];
                    tempVerts[workingIdx + 5] = model[vertexIdx + i * VERTEX_ATTRIBUTES_COUNT + 5];
                    tempVerts[workingIdx + 6] = model[vertexIdx + i * VERTEX_ATTRIBUTES_COUNT + 6];
                    tempVerts[workingIdx + 7] = model[vertexIdx + i * VERTEX_ATTRIBUTES_COUNT + 7];
                    memcpy(&tempVerts[workingIdx + 8],  &instances[instanceIdx].texIndex, sizeof(float)); // Copy exact bits
                    memcpy(&tempVerts[workingIdx + 9],  &instances[instanceIdx].glowIndex, sizeof(float));
                    memcpy(&tempVerts[workingIdx + 10], &instances[instanceIdx].specIndex, sizeof(float));
                    memcpy(&tempVerts[workingIdx + 11], &instances[instanceIdx].normIndex, sizeof(float));
                    memcpy(&tempVerts[workingIdx + 12], &modelIdx, sizeof(float));
                    memcpy(&tempVerts[workingIdx + 13], &instanceIdx, sizeof(float));
                }

                headVertIdx += 3;
            }
            lightVertexCounts[lightIdx] = headVertIdx;
        }

        glBindBuffer(GL_ARRAY_BUFFER, lightVBOs[lightIdx]);
        size_t bufferSize = lightVertexCounts[lightIdx] * VERTEX_ATTRIBUTES_COUNT * sizeof(float);
        if (lightVertexCounts[lightIdx] == 0 || bufferSize > sizeOfTempVertices) { DualLogError("Invalid buffer size for lightVBOs[%d]: %zu (vertices=%u)\n", lightIdx, bufferSize, lightVertexCounts[lightIdx]); continue; }
        glBufferData(GL_ARRAY_BUFFER, bufferSize, tempVerts, GL_DYNAMIC_DRAW);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    free(tempVerts);
    double end_time = get_time();
    DualLog("Generating light volume meshes took %f seconds\n", end_time - start_time);
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
    
    glGenVertexArrays(1, &quadVAO);
    CHECK_GL_ERROR();
    glGenBuffers(1, &quadVBO);
    CHECK_GL_ERROR();
    glBindVertexArray(quadVAO);
    CHECK_GL_ERROR();
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    CHECK_GL_ERROR();
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    CHECK_GL_ERROR();
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    CHECK_GL_ERROR();
    glEnableVertexAttribArray(0);
    CHECK_GL_ERROR();
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    CHECK_GL_ERROR();
    glEnableVertexAttribArray(1);
    CHECK_GL_ERROR();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    CHECK_GL_ERROR();
    glBindVertexArray(0);
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
    for (int i = 0; i < 9; i++) {
        glVertexAttribBinding(i, 0);
        glEnableVertexAttribArray(i);
    }

    glBindVertexArray(0);
    DebugRAM("after vao chunk bind");
    
    int fontInit = InitializeTextAndFonts();
    if (fontInit) { DualLogError("TTF_OpenFont failed: %s\n", TTF_GetError()); return SYS_TTF + 1; }
    DebugRAM("setup full screen quad"); 
    
    // Initialize Lights
    for (int i = 0; i < LIGHT_COUNT; i++) {
        int base = i * LIGHT_DATA_SIZE; // Step by 12
        lights[base + 0] = base * 0.08f; // posx
        lights[base + 1] = base * 0.08f; // posy
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
        lightDirty[i] = true;
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
    lightDirty[0] = true;
    
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
    lightDirty[1] = true;
    DebugRAM("init lights"); 

    // First pass gbuffer images
    GenerateAndBindTexture(&inputImageID,             GL_RGBA8, screen_width, screen_height,            GL_RGBA,           GL_UNSIGNED_BYTE, "Unlit Raster Albedo Colors");
    CHECK_GL_ERROR();
    GenerateAndBindTexture(&inputNormalsID,         GL_RGBA16F, screen_width, screen_height,            GL_RGBA,              GL_HALF_FLOAT, "Unlit Raster Normals");
    CHECK_GL_ERROR();
    GenerateAndBindTexture(&inputDepthID, GL_DEPTH_COMPONENT24, screen_width, screen_height, GL_DEPTH_COMPONENT,            GL_UNSIGNED_INT, "Unlit Raster Depth");
    CHECK_GL_ERROR();
    GenerateAndBindTexture(&inputWorldPosID,        GL_RGBA32F, screen_width, screen_height,            GL_RGBA,                   GL_FLOAT, "Unlit Raster World Positions");
    CHECK_GL_ERROR();
    GenerateAndBindTexture(&inputTexMapsID,         GL_RGBA32I, screen_width, screen_height,    GL_RGBA_INTEGER,                     GL_INT, "Unlit Raster Glow and Specular Map Indices");
    CHECK_GL_ERROR();

    // Create framebuffer
    glGenFramebuffers(1, &gBufferFBO);
    CHECK_GL_ERROR();
    glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
    CHECK_GL_ERROR();
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
    CHECK_GL_ERROR();
    glBindImageTexture(1, inputNormalsID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
    CHECK_GL_ERROR();
    glBindImageTexture(3, inputWorldPosID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    CHECK_GL_ERROR();
    glBindImageTexture(5, inputTexMapsID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32I);
    CHECK_GL_ERROR();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);    
    CHECK_GL_ERROR();
    malloc_trim(0);
    DebugRAM("setup gbuffer end");
    Input_Init();
    systemInitialized[SYS_NET] = InitializeNetworking() == 0;
    InitializeAudio();
    DebugRAM("audio init"); 
    systemInitialized[SYS_AUD] = true;
    DebugRAM("InitializeEnvironment end");
    return 0;
}

int ExitCleanup(int status) { // Ifs allow deinit from anywhere, only as needed.
    if (systemInitialized[SYS_AUD]) CleanupAudio();
    if (systemInitialized[SYS_NET]) CleanupNetworking();
    if (activeLogFile) fclose(activeLogFile); // Close log playback file.
    if (colorBufferID) glDeleteBuffers(1, &colorBufferID);
    for (uint32_t i=0;i<MODEL_COUNT;i++) {
        if (vertexDataArrays[i]) free(vertexDataArrays[i]);
        if (vbos[i]) glDeleteBuffers(1, &vbos[i]);
        vbos[i] = 0;
    }
    
    for (uint32_t i=0;i<MAX_VISIBLE_LIGHTS;i++) {
        if (lightVBOs[i]) glDeleteBuffers(1, &lightVBOs[i]);
        lightVBOs[i] = 0;
    }
    
    if (vboMasterTable) glDeleteBuffers(1, &vboMasterTable);
    if (modelVertexOffsetsID) glDeleteBuffers(1, &modelVertexOffsetsID);
    if (vao_chunk) glDeleteVertexArrays(1, &vao_chunk);
    if (chunkShaderProgram) glDeleteProgram(chunkShaderProgram);
    if (textVAO) glDeleteVertexArrays(1, &textVAO);
    if (textVBO) glDeleteBuffers(1, &textVBO);
    if (textShaderProgram) glDeleteProgram(textShaderProgram);
    if (quadVAO) glDeleteVertexArrays(1, &quadVAO);
    if (quadVBO) glDeleteBuffers(1, &quadVBO);
    if (imageBlitShaderProgram) glDeleteProgram(imageBlitShaderProgram);
    if (inputImageID) glDeleteTextures(1,&inputImageID);
    if (inputNormalsID) glDeleteTextures(1,&inputNormalsID);
    if (inputDepthID) glDeleteTextures(1,&inputDepthID);
    if (inputWorldPosID) glDeleteTextures(1,&inputWorldPosID);
    if (inputTexMapsID) glDeleteTextures(1,&inputTexMapsID);
    if (gBufferFBO) glDeleteFramebuffers(1, &gBufferFBO);
    if (deferredLightingShaderProgram) glDeleteProgram(deferredLightingShaderProgram);
//     if (lightVolumeMeshShaderProgram) glDeleteProgram(lightVolumeMeshShaderProgram);
    if (modelBoundsID) glDeleteBuffers(1, &modelBoundsID);
    if (visibleLightsID) glDeleteBuffers(1, &visibleLightsID);
    if (instancesBuffer) glDeleteBuffers(1, &instancesBuffer);
    if (matricesBuffer) glDeleteBuffers(1, &matricesBuffer);
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
        case EV_LOAD_INSTANCES: int exitCode = SetupInstances(); UpdateLightVolumes(); return exitCode;
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
        drawCallCount = 0; // Reset per frame
        vertexCount = 0;
        
        // 0. Clear Frame Buffers and Depth
        if (debugRenderSegfaults) DualLog("0. Clear Frame Buffers and Depth\n");
        glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
        CHECK_GL_ERROR();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        CHECK_GL_ERROR();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        CHECK_GL_ERROR();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
        lightDirty[lightBase / 6] = true;
        instances[39].posx = testLight_x;
        instances[39].posy = testLight_y;
        instances[39].posz = testLight_z;
        dirtyInstances[39] = true;
        
        // 1. Light Culling to limit of MAX_VISIBLE_LIGHTS
        if (debugRenderSegfaults) DualLog("1. Light Culling to limit of MAX_VISIBLE_LIGHTS\n");
        playerCellIdx = PositionToWorldCellIndex(cam_x, cam_y, cam_z);
        playerCellIdx_x = PositionToWorldCellIndexX(cam_x);
        playerCellIdx_y = PositionToWorldCellIndexY(cam_y);
        playerCellIdx_z = PositionToWorldCellIndexZ(cam_z);
        numLightsFound = 0;
        for (int i=0;i<LIGHT_COUNT;++i) {
            uint32_t litIdx = (i * LIGHT_DATA_SIZE);
            float litIntensity = lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY];
            if (litIntensity < 0.015f) continue; // Off
            
            float litx = lights[litIdx + LIGHT_DATA_OFFSET_POSX];
            float lity = lights[litIdx + LIGHT_DATA_OFFSET_POSY];
            float litz = lights[litIdx + LIGHT_DATA_OFFSET_POSZ];
            if (squareDistance3D(cam_x, cam_y, cam_z, litx, lity, litz) < sightRangeSquared) {
                
                int idx = numLightsFound * LIGHT_DATA_SIZE;
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
        
        for (int i=numLightsFound;i<MAX_VISIBLE_LIGHTS;++i) {
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
        
        // 2. Instance Culling to only those in range of player
        if (debugRenderSegfaults) DualLog("2. Instance Culling to only those in range of player\n");
        bool instanceIsCulledArray[INSTANCE_COUNT];
        memset(instanceIsCulledArray,true,INSTANCE_COUNT * sizeof(bool)); // All culled.
        float distSqrd = 0.0f;
        for (int i=0;i<INSTANCE_COUNT;++i) {
            if (!instanceIsCulledArray[i]) continue; // Already marked as visible.
            
            distSqrd = squareDistance3D(instances[i].posx,instances[i].posy,instances[i].posz,cam_x, cam_y, cam_z);
            if (distSqrd < sightRangeSquared) instanceIsCulledArray[i] = false;
        }

        // 3. Pass all instance matrices to GPU
        if (debugRenderSegfaults) DualLog("3. Pass all instance matrices to GPU\n");
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, matricesBuffer);
        CHECK_GL_ERROR();
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, INSTANCE_COUNT * 16 * sizeof(float), modelMatrices); // * 16 because matrix4x4
        CHECK_GL_ERROR();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, matricesBuffer);
        CHECK_GL_ERROR();
        
        // 4. Unlit Raterized Geometry
        //        Standard vertex + fragment rendering, but with special packing to minimize transfer data amounts
        if (debugRenderSegfaults) DualLog("4. Unlit Raterized Geometry\n");
        glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
        CHECK_GL_ERROR();
        glUseProgram(chunkShaderProgram);
        CHECK_GL_ERROR();
        glEnable(GL_DEPTH_TEST);
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
        glBindVertexArray(vao_chunk);
        CHECK_GL_ERROR();
        for (int i=0;i<INSTANCE_COUNT;i++) {
            if (instanceIsCulledArray[i]) continue;
            if (instances[i].modelIndex >= MODEL_COUNT) continue;
            if (modelVertexCounts[instances[i].modelIndex] < 1) continue; // Empty model
            if (instances[i].modelIndex < 0) continue; // Culled

            if (dirtyInstances[i]) UpdateInstanceMatrix(i);
            if (i != 39) continue; // Skip everything except test light's white cube.
            
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
            drawCallCount++;
            vertexCount += modelVertexCounts[modelType];
        }
        
        // 5. Render Light Volume Meshes
        if (debugRenderSegfaults) DualLog("5. Render Light Volume Meshes\n");
        glUseProgram(lightVolumeShaderProgram);
        CHECK_GL_ERROR();
        float identity[16];
        mat4_identity(identity);
        glUniformMatrix4fv(matrix_lightvol, 1, GL_FALSE, identity);
        CHECK_GL_ERROR();
        glUniformMatrix4fv(view_lightvol, 1, GL_FALSE, view);
        CHECK_GL_ERROR();
        glUniformMatrix4fv(projection_lightvol, 1, GL_FALSE, projection);
        CHECK_GL_ERROR();
        glUniform1i(debugView_lightvol, debugView);
        CHECK_GL_ERROR();
//         glDisable(GL_CULL_FACE); // Disable backface culling
        CHECK_GL_ERROR();
    //     for (uint32_t lightIdx = 0; lightIdx < numLightsFound; lightIdx++) {
        for (uint32_t lightIdx = 0; lightIdx < 1; lightIdx++) {
            if (lightVertexCounts[lightIdx] == 0) continue;

            glBindVertexBuffer(0, lightVBOs[lightIdx], 0, VERTEX_ATTRIBUTES_COUNT * sizeof(float));
            CHECK_GL_ERROR();
            glDrawArrays(GL_TRIANGLES, 0, lightVertexCounts[lightIdx]);
            CHECK_GL_ERROR();
            drawCallCount++;
            vertexCount += lightVertexCounts[lightIdx];
        }
        
//         glEnable(GL_CULL_FACE); // Reenable backface culling
        CHECK_GL_ERROR();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        CHECK_GL_ERROR();

        // 6. Deferred Lighting + Shadow Calculations
        //        Apply deferred lighting with compute shader.  All lights are
        //        dynamic and can be updated at any time (flicker, light switches,
        //        move, change color, get marked as "culled" so shader can skip it,
        //        etc.).
        if (debugRenderSegfaults) DualLog("6. Deferred Lighting + Shadow Calculations\n");
        glUseProgram(deferredLightingShaderProgram);
        CHECK_GL_ERROR();
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        CHECK_GL_ERROR();

        // These should be static but cause issues if not...
        glUniform1ui(screenWidthLoc_deferred, screen_width); // Makes screen all black if not sent every frame.
        CHECK_GL_ERROR();
        glUniform1ui(screenHeightLoc_deferred, screen_height); // Makes screen all black if not sent every frame.
        CHECK_GL_ERROR();
        glUniform1i(debugViewLoc_deferred, debugView);
        CHECK_GL_ERROR();
        glUniform1i(shadowsEnabledLoc_deferred, shadowsEnabled);
        CHECK_GL_ERROR();
        float viewInv[16];
        mat4_inverse(viewInv,view);
        float projInv[16];
        mat4_inverse(projInv,projection);

        // Dispatch compute shader
        GLuint groupX = (screen_width + 7) / 8;
        GLuint groupY = (screen_height + 7) / 8;
        glDispatchCompute(groupX, groupY, 1);
        CHECK_GL_ERROR();
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT); // Runs slightly faster 0.1ms without this, but may need if more shaders added in between
        CHECK_GL_ERROR();
        
        // 7. Render final meshes' results with full screen quad
        if (debugRenderSegfaults) DualLog("7. Render final meshes' results with full screen quad\n");
        glUseProgram(imageBlitShaderProgram);
        CHECK_GL_ERROR();
        glActiveTexture(GL_TEXTURE0);
        CHECK_GL_ERROR();
        if (debugView == 0) {
            glBindTexture(GL_TEXTURE_2D, inputImageID); // Normal
            CHECK_GL_ERROR();
        } else if (debugView == 1) {
            glBindTexture(GL_TEXTURE_2D, inputImageID); // Unlit
            CHECK_GL_ERROR();
        } else if (debugView == 2) {
            glBindTexture(GL_TEXTURE_2D, inputNormalsID); // Triangle Normals 
            CHECK_GL_ERROR();
        } else if (debugView == 3) {
            glBindTexture(GL_TEXTURE_2D, inputImageID); // Depth.  Values must be decoded in shader
            CHECK_GL_ERROR();
        } else if (debugView == 4) {
            glBindTexture(GL_TEXTURE_2D, inputImageID); // Instance, Model, Texture indices as rgb. Values must be decoded in shader divided by counts.
            CHECK_GL_ERROR();
        }
        
        glUniform1i(glGetUniformLocation(imageBlitShaderProgram, "tex"), 0);
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
        glBindTexture(GL_TEXTURE_2D, 0);
        CHECK_GL_ERROR();
        glUseProgram(0);
        CHECK_GL_ERROR();
        
        // 8. Render UI Images
        if (debugRenderSegfaults) DualLog("8. Render UI Images\n");
 
        // 9. Render UI Text
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
        RenderFormattedText(10, 100, TEXT_WHITE, "DebugView: %d, %s", debugView, debugView == 1 ? "unlit" : "normal");
        CHECK_GL_ERROR();
        RenderFormattedText(10, 115, TEXT_WHITE, "Num lights: %d   Player cell:: x: %d, y: %d, z: %d", numLightsFound, playerCellIdx_x, playerCellIdx_y, playerCellIdx_z);
        CHECK_GL_ERROR();
        
        // Frame stats
        drawCallCount++; // Add one more for this text render ;)
        RenderFormattedText(10, 10, TEXT_WHITE, "Frame time: %.6f (FPS: %d), Draw calls: %d, Vertices: %d, Worst FPS: %d",
                            (get_time() - last_time) * 1000.0,framesPerLastSecond,drawCallCount,vertexCount,worstFPS);
        CHECK_GL_ERROR();
        
        double time_now = get_time();
        if ((time_now - lastFrameSecCountTime) >= 1.00) {
            lastFrameSecCountTime = time_now;
            framesPerLastSecond = globalFrameNum - lastFrameSecCount;
            if (framesPerLastSecond < worstFPS && globalFrameNum > 10) worstFPS = framesPerLastSecond; // After startup, keep track of worst framerate seen.
            lastFrameSecCount = globalFrameNum;
        }

        glDisable(GL_STENCIL_TEST);
        CHECK_GL_ERROR();
        glStencilMask(0x00); // Disable stencil writes
        CHECK_GL_ERROR();
        SDL_GL_SwapWindow(window); // Present frame
        CHECK_GL_ERROR();
        last_time = current_time;
        globalFrameNum++;
    }

    // Cleanup
    return ExitCleanup(exitCode);
}
