// voxen.c
// Description: A realtime OpenGL 4.3+ Game Engine for Citadel: The System Shock Fan Remake
// TODO: Figure out how to handle info_ressurection_points that needed to live outside the levels:
// Level R -27.386 -55.488 26.5941
// Level 1 40.903 -42.372 -30.78
// Level 2 30.67407 -25.832 10.21412
// Level 3 38.26813 -15.498 20.37825
// Level 4 -19.48 -7.928 22.954
// Level 5 -24.358 12.5956 31.8497
// Level 6 -22.3568 33.7845 -30.728
// Level 7 2.228084 50.95243 7.532025
// Level 9.1_resdest 2.303 106.77 -38.554 (I don't remember what this is for, cheat spawn from `load 9`??)
// TODO: Animated lights
// TODO: Multiview renders for sensaround
// TODO: Proper physics
// TODO: Particle system
// TODO: Raycasts
// TODO: Voxel GI
// TODO: Scripting engine for gameplay
// TODO: Save/Load system
#define _GNU_SOURCE
#include <malloc.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include "event.h"
#define VOXEN_ENGINE_IMPLEMENTATION
#include "entity.h"
#include "voxen.h"
#include "vmath.h"
#include "matvecquat.h"
#include "External/stb_image.h"
#include "Shaders/text_vert.glsl.h" // Shaders are converted into string headers at build time.
#include "Shaders/text_frag.glsl.h"
#include "Shaders/chunk_vert.glsl.h"
#include "Shaders/chunk_frag.glsl.h"
#include "Shaders/shadowmap_vert.glsl.h"
#include "Shaders/shadowmap_frag.glsl.h"
#include "Shaders/composite_vert.glsl.h"
#include "Shaders/composite_frag.glsl.h"
#include "Shaders/ssr.compute.h"
#include "Shaders/shadowmaps_clear.compute.h"
#include "input.c"

typedef struct {
    float nx, ny, nz, d;
} FrustumPlane;

// ----------------------------------------------------------------------------
// Window
GLFWwindow *window;
double monitorSwitchTime;
bool inventoryMode = false;
uint16_t screen_width = 1366, screen_height = 768;
bool editMode = true;
// ----------------------------------------------------------------------------
// Diagnostics
double game_start_time = 0.00;
uint32_t globalFrameNum = 0;
double last_time = 0.0;
double current_time = 0.0;
double cpuTime = 0.0;
double lastFrameSecCountTime = 0.00;
uint32_t lastFrameSecCount = 0;
uint32_t framesPerLastSecond = 0;
uint32_t worstFPS = UINT32_MAX;
double screenshotTimeout = 0.0;
double time_PhysicsStep = 0.0;
char statusText[TEXT_BUFFER_SIZE];
// ----------------------------------------------------------------------------
// Settings
uint8_t settings_Shadows = 1u; // Default 2 (1 is hard shadows, 2 enables Pseudo-Stochastic PCF sampling softening
uint8_t settings_AntiAliasing = 1u; // Default 1
uint8_t settings_Brightness = 100u; // Default 100 (for %)
uint8_t settings_VolumeMusic = 20u;
uint8_t settings_Language = 0; // English default
uint8_t settings_CullEnabled = 1;
float settings_FOV = 65.0f;
#define SSR_RES 4 // Ratio is (1 / SSR_RES) * render resolution.
uint8_t settings_Reflections = 1u; // Default 1
float settings_SSRStepSize = 0.55f;
uint16_t settings_SSRStepCount = 48;
float settings_SSRSampleWeight = 2.15f;
bool settings_Vsync = false;
float lodRangeSqrd = 35.4f * 35.4f;
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
bool levelCurrentlyLoading = false;
float pauseRelativeTime = 0.0f;
QuestBits questData;
bool bottomless = false;
bool superoverride;
// ----------------------------------------------------------------------------
// Camera variables
float cam_yaw = 90.0f;
float cam_pitch = 0.0f;
float cam_roll = 0.0f;
Quaternion cam_rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
float cam_forwardx = 0.0f, cam_forwardy = 0.0f, cam_forwardz = 0.0f;
float cam_rightx = 0.0f, cam_righty = 0.0f, cam_rightz = 0.0f;
float berserkFinished = 0.0f;
float berserkSeedTime = 0.0f;
// ----------------------------------------------------------------------------
// OpenGL / Rendering
int32_t debugView = 0;
int32_t debugValue = 0;
float aspect3D = 1.0f;
float rasterPerspectiveProjection[16];
float shadowmapsPerspectiveProjection[16];
uint32_t drawCallsRenderedThisFrame = 0; // Total draw calls this frame
uint32_t textDrawCallsRenderedThisFrame = 0;
uint32_t uiImageDrawCallsRenderedThisFrame = 0;
uint32_t shadowDrawCallsRenderedThisFrame = 0;
uint32_t verticesRenderedThisFrame = 0;
bool instanceIsLODArray[INSTANCE_COUNT];
float fogColorR, fogColorG, fogColorB, fogColorRUsed, fogColorGUsed, fogColorBUsed, fogBaseDensityForLevel;
GLuint inputImageID, inputDepthID, inputWorldPosID, inputSpecID, gBufferFBO, outputImageID; // FBO

GLuint chunkShaderProgram; // Generic lit and unlit raster shader forward+
GLuint vao_chunk; // Vertex Array Object

GLuint shadowCubeMap;
GLuint shadowFBO;
GLuint shadowmapsShaderProgram;
GLuint shadowmapsClearShaderProgram;
GLuint shadowMapSSBO;
uint32_t totalShadowmapPixels = 0;

GLuint ssrShaderProgram; // SSR (Screen Space Reflections)

GLuint imageBlitShaderProgram; // Full Screen Quad Blit for rendering final compositing output/image effect passes
GLuint quadVAO, quadVBO;

GLuint textShaderProgram;
GLuint textVAO, textVBO;
// ----------------------------------------------------------------------------
// UI Cursor
bool cursorVisible = false;
int32_t cursorPosition_x = 680, cursorPosition_y = 384;
// ----------------------------------------------------------------------------
// UI
//    Text
#define TEXT_WHITE 0
#define TEXT_YELLOW 1
#define TEXT_DARK_YELLOW 2
#define TEXT_GREEN 3
#define TEXT_RED 4
#define TEXT_ORANGE 5
#define TEXT_STOPD_RED 6
#define TEXT_STOPD_RED_HIGHLIGHT 7
#define TEXT_STOPD_RED_PAUSETITLE 8
#define TEXT_COLOR_COUNT 9

//    Images
#define MAX_UI_IMAGES 64
bool noHUD = false;

typedef struct {
    float x, y, z;        // Top-left corner in screen space (pixels)
    float width, height; // Size in screen space (pixels)
    uint32_t texIndex; // Index into textureOffsets for palettized texture
    bool visible;      // Whether to render this image
} UIImage;

UIImage uiImages[MAX_UI_IMAGES];
uint32_t uiImageCount = 0;
char uiTextBuffer[TEXT_BUFFER_SIZE];
float uiOrthoProjection[16];
Color textColors[TEXT_COLOR_COUNT] = {
    {         1.0f,         1.0f,          1.0f, 1.0f}, // 0 White
    { 0.890196078f, 0.874509804f,          0.0f, 1.0f}, // 1 Yellow
    { 0.623529412f, 0.611764706f,          0.0f, 1.0f}, // 2 Dark Yellow 0.8902f * 0.7f, 0.8745f * 0.7f, 0f
    { 0.372549020f, 0.654901961f,  0.168627451f, 1.0f}, // 3 Green
    { 0.917647059f, 0.137254902f,  0.168627451f, 1.0f}, // 4 Red
    {         1.0f, 0.498039216f,          0.0f, 1.0f}, // 5 Orange
    { 0.674509804f, 0.058823529f,  0.070588235f, 1.0f}, // 6 StopD Red
    { 0.941176471f, 0.282352941f,  0.298039216f, 1.0f}, // 7 StopD Red Highlight
    { 0.909803922f, 0.203921569f,  0.219607843f, 1.0f}  // 8 StopD Red Pause Title
};

//      Center Status Print
int statusTextLengthWithoutNullTerminator = 6;
float statusTextDecayFinished = 0.0f;
// ----------------------------------------------------------------------------
// Lights
GLuint lightsID, voxelLightListIndicesID, voxelLightListsRawID, shadowMapsIndirectionID;
uint32_t* voxelLightListsRaw = NULL;
uint32_t* voxelLightListIndices = NULL;
uint32_t* shadowmapIndirectionList = NULL;
uint16_t numDynamicLights;
float lights[LIGHT_COUNT * LIGHT_DATA_SIZE] = {0};
float lightsRangeSquared[LIGHT_COUNT] = {0.0f};
bool lightDirty[LIGHT_COUNT] = { [0 ... LIGHT_COUNT-1] = true };
float (*lightView)[6][4][4] = NULL; // Array of Array of 6 Arrays of 16 floats (matrix 4x4).  lightView[i][face][0 ... 15]
float (*lightViewProj)[6][4][4] = NULL; // Array of Array of 6 Arrays of 16 floats (matrix 4x4).  lightViewProj[i][face][0 ... 15]
FrustumPlane (*lightFrustumPlanes)[6][6] = NULL; // Array of Array of 6 Arrays of FrustumPlane structs (four floats).  lightFrustumPlanes[i][face][.nx,.ny,, .nz, .d]
// ----------------------------------------------------------------------------
// OpenGL / Rendering Helper Functions
void GenerateAndBindTexture(GLuint *id, GLenum internalFormat, int32_t width, int32_t height, GLenum format, GLenum type, GLenum target) {
    glGenTextures(1, id);
    glBindTexture(target, *id);
    glTexImage2D(target, 0, internalFormat, width, height, 0, format, type, NULL);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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
    for (int32_t i = 0; i < count; i++) { glAttachShader(program, shaders[i]); glDeleteShader(shaders[i]); }
    glLinkProgram(program);
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) { char infoLog[512]; glGetProgramInfoLog(program, 512, NULL, infoLog); DualLogError("%s Linking Failed: %s\n", programName, infoLog); exit(1); }
    return program;
}

void CompileShaders(void) {
    GLuint vertShader, fragShader, computeShader;
    vertShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource, "Chunk Vertex Shader");
    fragShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderTraditional, "Chunk Fragment Shader");
    chunkShaderProgram = LinkProgram((GLuint[]){vertShader, fragShader}, 2, "Chunk Shader Program");
    
    vertShader = CompileShader(GL_VERTEX_SHADER, shadowmapVertexShaderSource, "Shadowmaps Vertex Shader");
    fragShader = CompileShader(GL_FRAGMENT_SHADER, shadowmapFragmentShaderSource, "Shadowmaps Fragment Shader");
    shadowmapsShaderProgram = LinkProgram((GLuint[]){vertShader, fragShader}, 2, "Shadowmaps Shader Program");

    vertShader = CompileShader(GL_VERTEX_SHADER, textVertexShaderSource, "Text Vertex Shader");
    fragShader = CompileShader(GL_FRAGMENT_SHADER, textFragmentShaderSource, "Text Fragment Shader");
    textShaderProgram = LinkProgram((GLuint[]){vertShader, fragShader}, 2, "Text Shader Program");

    computeShader = CompileShader(GL_COMPUTE_SHADER, ssr_computeShader, "Screen Space Reflections Compute Shader");
    ssrShaderProgram = LinkProgram((GLuint[]){computeShader}, 1, "Screen Space Reflections Shader Program");
    
    computeShader = CompileShader(GL_COMPUTE_SHADER, shadowmaps_clear_computeShader, "Shadowmaps Clear Compute Shader");
    shadowmapsClearShaderProgram = LinkProgram((GLuint[]){computeShader}, 1, "Shadowmaps Clear Shader Program");

    vertShader = CompileShader(GL_VERTEX_SHADER,   quadVertexShaderSource,   "Image Blit Vertex Shader");
    fragShader = CompileShader(GL_FRAGMENT_SHADER, quadFragmentShaderSource, "Image Blit Fragment Shader");
    imageBlitShaderProgram = LinkProgram((GLuint[]){vertShader, fragShader}, 2, "Image Blit Shader Program");
    CHECK_GL_ERROR();
}

static inline void mul_mat4(float *out, const float *a, const float *b) { // out = a * b
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
   
    for (int32_t i = 0; i < 16; i++) out[i] = result[i]; // copy back
}

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

void UpdateScreenSize(void) {
    float* m;
    m = uiOrthoProjection;
    m[0] = 2.0f / (float)screen_width; m[1] =                           0.0f; m[2] =  0.0f; m[3] = 0.0f;
    m[4] =                       0.0f; m[5] = -2.0f / ((float)screen_height); m[6] =  0.0f; m[7] = 0.0f;
    m[8] =                       0.0f; m[9] =                           0.0f; m[10]= -1.0f; m[11]= 0.0f;
    m[12]=                      -1.0f; m[13]=                           1.0f; m[14]=  0.0f; m[15]= 1.0f;
    
    aspect3D = (float)screen_width / (float)screen_height;
    float f = vcot(settings_FOV * PI / 360.0f);
    m = rasterPerspectiveProjection;
    m[0] = f / aspect3D; m[1] = 0.0f; m[2] =                                                      0.0f; m[3] =  0.0f;
    m[4] =         0.0f; m[5] =    f; m[6] =                                                      0.0f; m[7] =  0.0f;
    m[8] =         0.0f; m[9] = 0.0f; m[10]=      -(FAR_PLANE + NEAR_PLANE) / (FAR_PLANE - NEAR_PLANE); m[11]= -1.0f;
    m[12]=         0.0f; m[13]= 0.0f; m[14]= -2.0f * FAR_PLANE * NEAR_PLANE / (FAR_PLANE - NEAR_PLANE); m[15]=  0.0f;
    
    float aspectShad = (float)SHADOW_MAP_SIZE / (float)SHADOW_MAP_SIZE;
    f = 1.0f / vtan(SHADOWMAP_FOV * PI / 360.0f); // vcot introduces skewness causing false "Peter-Panning" from bubble distortion of the shadowmap depths.  Just stick with recip tangent.
    m = shadowmapsPerspectiveProjection;
    m[0] = f / aspectShad; m[1] = 0.0f; m[2] =                                            0.0f; m[3] =  0.0f;
    m[4] =           0.0f; m[5] =    f; m[6] =                                            0.0f; m[7] =  0.0f;
    m[8] =           0.0f; m[9] = 0.0f; m[10]=      -(35.0 + NEAR_PLANE) / (35.0 - NEAR_PLANE); m[11]= -1.0f;
    m[12]=           0.0f; m[13]= 0.0f; m[14]= -2.0f * 35.0 * NEAR_PLANE / (35.0 - NEAR_PLANE); m[15]=  0.0f;
    
    glProgramUniform1ui(imageBlitShaderProgram, 2, screen_width);
    glProgramUniform1ui(imageBlitShaderProgram, 3, screen_height);
    glProgramUniform1f(imageBlitShaderProgram, 23, (float)(SHADOW_MAP_SIZE));
    glProgramUniform1i(imageBlitShaderProgram, 26, SSR_RES);
    glProgramUniform1ui(chunkShaderProgram, 6, screen_width);
    glProgramUniform1ui(chunkShaderProgram, 7, screen_height);
    glProgramUniform1f(chunkShaderProgram, 16, (float)(SHADOW_MAP_SIZE));
    glProgramUniform1ui(ssrShaderProgram, 0, screen_width / SSR_RES);
    glProgramUniform1ui(ssrShaderProgram, 1, screen_height / SSR_RES);       
    glProgramUniform1i(ssrShaderProgram, 7, SSR_RES);
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

bool SphereInFrustum(FrustumPlane* planes, float cx, float cy, float cz, float radius) {
    for (int i = 0; i < 6; i++) {
        float dist = planes[i].nx * cx + planes[i].ny * cy + planes[i].nz * cz + planes[i].d;
        if (dist < -radius) return false;
    }
    return true;
}

void ExtractFrustumPlanes(float* m, FrustumPlane* planes) {
    planes[0].nx = m[3]  + m[0];  planes[0].ny = m[7]  + m[4];  planes[0].nz = m[11] + m[8];  planes[0].d = m[15] + m[12]; // Left
    planes[1].nx = m[3]  - m[0];  planes[1].ny = m[7]  - m[4];  planes[1].nz = m[11] - m[8];  planes[1].d = m[15] - m[12]; // Right
    planes[2].nx = m[3]  + m[1];  planes[2].ny = m[7]  + m[5];  planes[2].nz = m[11] + m[9];  planes[2].d = m[15] + m[13]; // Bottom
    planes[3].nx = m[3]  - m[1];  planes[3].ny = m[7]  - m[5];  planes[3].nz = m[11] - m[9];  planes[3].d = m[15] - m[13]; // Top
    planes[4].nx = m[3]  + m[2];  planes[4].ny = m[7]  + m[6];  planes[4].nz = m[11] + m[10]; planes[4].d = m[15] + m[14]; // Near
    planes[5].nx = m[3]  - m[2];  planes[5].ny = m[7]  - m[6];  planes[5].nz = m[11] - m[10]; planes[5].d = m[15] - m[14]; // Far
    for (int i = 0; i < 6; i++) {
        float len = vsqrtf(planes[i].nx*planes[i].nx + planes[i].ny*planes[i].ny + planes[i].nz*planes[i].nz);
        if (len > 0.0f) {
            planes[i].nx /= len; planes[i].ny /= len; planes[i].nz /= len; planes[i].d /= len; // Normalize
        }
    }
}

Quaternion cubemapOrientationQuaternion[6] = {
    {0.0f, 0.707106781f, 0.0f, 0.707106781f},  // +X: Right
    {0.0f, -0.707106781f, 0.0f, 0.707106781f}, // -X: Left
    {-0.707106781f, 0.0f, 0.0f, 0.707106781f}, // +Y: Up
    {0.707106781f, 0.0f, 0.0f, 0.707106781f},  // -Y: Down
    {0.0f, 0.0f, 0.0f, 1.0f},                  // +Z: Forward
    {0.0f, 1.0f, 0.0f, 0.0f}                   // -Z: Backward
};

void UpdateVoxelLightLists() {
    memset(voxelLightListsRaw, 0, VOXEL_COUNT * 4 * sizeof(uint32_t));
    memset(voxelLightListIndices, 0, VOXEL_COUNT * 2 * sizeof(uint32_t));
    uint32_t totalLightAssignments = 0;
    float cellWidthRecip = 1.0f / WORLDCELL_WIDTH_F;
    for (uint32_t lightIdx = 0; lightIdx < loadedLights; ++lightIdx) {
        uint32_t litIdx = lightIdx * LIGHT_DATA_SIZE;
        float litX = lights[litIdx + LIGHT_DATA_OFFSET_POSX];
        float litZ = lights[litIdx + LIGHT_DATA_OFFSET_POSZ];
        float range = lights[litIdx + LIGHT_DATA_OFFSET_RANGE]; // Can't early out here for range to player as it breaks shadows!
        int32_t minCellX = (int32_t)((litX - range - worldMin_x) * cellWidthRecip);
        int32_t maxCellX = (int32_t)vceil((litX + range - worldMin_x) * cellWidthRecip);
        int32_t minCellZ = (int32_t)((litZ - range - worldMin_z) * cellWidthRecip);
        int32_t maxCellZ = (int32_t)vceil((litZ + range - worldMin_z) * cellWidthRecip);
        minCellX = minCellX > 0 ? minCellX : 0;
        maxCellX = maxCellX > (WORLDX - 1) ? (WORLDX - 1) : maxCellX;
        minCellZ = minCellZ > 0 ? minCellZ : 0;
        maxCellZ = maxCellZ > (WORLDZ - 1) ? (WORLDZ - 1) : maxCellZ;
        for (int32_t cellZ = minCellZ; cellZ <= maxCellZ; ++cellZ) {
            for (int32_t cellX = minCellX; cellX <= maxCellX; ++cellX) {
                uint32_t cellIndex = cellZ * WORLDX + cellX;
                
                #pragma GCC unroll 8
                for (uint32_t voxelZ = 0; voxelZ < 8; ++voxelZ) {
                    #pragma GCC unroll 8
                    for (uint32_t voxelX = 0; voxelX < 8; ++voxelX) {
                        uint32_t voxelIndex = cellIndex * WORLDX + voxelZ * 8 + voxelX;
                        float posX = voxelMinCenterX + (cellX * WORLDCELL_WIDTH_F) + (voxelX * VOXEL_SIZE);
                        float posZ = voxelMinCenterZ + (cellZ * WORLDCELL_WIDTH_F) + (voxelZ * VOXEL_SIZE);
                        float distSqrd = squareDistance2D(posX, posZ, litX, litZ);
                        if (distSqrd < lightsRangeSquared[lightIdx] && voxelLightListIndices[voxelIndex * 2 + 1] < MAX_LIGHTS_PER_VOXEL) {
                            voxelLightListIndices[voxelIndex * 2 + 1]++; // Increment light count
                            totalLightAssignments++;
                        }
                    }
                }
            }
        }
    }

    if (totalLightAssignments > VOXEL_COUNT * 4) { DualLogError("\nTotal light assignments (%u) exceed voxelLightListsRaw capacity (%u)\n", totalLightAssignments, VOXEL_COUNT * 4); return; }

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
        float range = lights[litIdx + LIGHT_DATA_OFFSET_RANGE];
        float distSq = squareDistance2D(instances[PLAYER1].position.x, instances[PLAYER1].position.z, litX, litZ);
        if(distSq > FAR_PLANE_SQUARED) continue;
        
        int lightCellIdx = cellIndexForLight[lightIdx];
        bool inPVS = (gridCellStates[lightCellIdx] & CELL_VISIBLE);
        if(!inPVS) {
            int x = cellIndexForLightX[lightIdx];
            int z = cellIndexForLightZ[lightIdx];
            int r = vfloor(range * 0.390625f); // 6 max
            for(int ix=x-r; ix<=x+r && !inPVS; ix++){
                for(int iz=z-r; iz<=z+r; iz++){
                    if(!XZPairInBounds(ix,iz)) continue;
                    int subIdx = iz*WORLDX + ix;
                    if((gridCellStates[subIdx] & CELL_VISIBLE) &&
                        get_cull_bit(precomputedVisibleCellsFromHere, lightCellIdx*ARRSIZE + subIdx)) {
                        inPVS = true;
                        break;
                    }
                }
            }
        }
        if(!inPVS) continue; // Only include lights that the voxel can actually see
        
        int32_t minCellX = (int32_t)((litX - range - worldMin_x) * cellWidthRecip); // cast to int truncates, no floorf
        int32_t maxCellX = (int32_t)vceil((litX + range - worldMin_x) * cellWidthRecip);
        int32_t minCellZ = (int32_t)((litZ - range - worldMin_z) * cellWidthRecip); // cast to int truncates, no floorf
        int32_t maxCellZ = (int32_t)vceil((litZ + range - worldMin_z) * cellWidthRecip);
        minCellX = minCellX > 0 ? minCellX : 0;
        maxCellX = maxCellX > (WORLDX - 1) ? (WORLDX - 1) : maxCellX;
        minCellZ = minCellZ > 0 ? minCellZ : 0;
        maxCellZ = maxCellZ > (WORLDZ - 1) ? (WORLDZ - 1) : maxCellZ;
        for (int32_t cellZ = minCellZ; cellZ <= maxCellZ; ++cellZ) {
            for (int32_t cellX = minCellX; cellX <= maxCellX; ++cellX) {
                uint32_t cellIndex = cellZ * WORLDX + cellX;

                #pragma GCC unroll 8
                for (uint32_t voxelZ = 0; voxelZ < 8; ++voxelZ) {
                    #pragma GCC unroll 8
                    for (uint32_t voxelX = 0; voxelX < 8; ++voxelX) {
                        uint32_t voxelIndex = cellIndex * WORLDX + voxelZ * 8 + voxelX;
                        float posX = voxelMinCenterX + (cellX * WORLDCELL_WIDTH_F) + (voxelX * VOXEL_SIZE);
                        float posZ = voxelMinCenterZ + (cellZ * WORLDCELL_WIDTH_F) + (voxelZ * VOXEL_SIZE);
                        float distSqrd = squareDistance2D(posX, posZ, litX, litZ);
                        if (distSqrd < lightsRangeSquared[lightIdx] && lightCounts[voxelIndex] < MAX_LIGHTS_PER_VOXEL) {
                            uint32_t offset = voxelLightListIndices[voxelIndex * 2];
                            voxelLightListsRaw[offset + lightCounts[voxelIndex]] = lightIdx;
                            lightCounts[voxelIndex]++;
                        }
                    }
                }
            }
        }
    }

    for (int i=0;i<loadedLights;++i) {
        uint32_t litIdx = i * LIGHT_DATA_SIZE;
        float litX = lights[litIdx + LIGHT_DATA_OFFSET_POSX];
        float litY = lights[litIdx + LIGHT_DATA_OFFSET_POSY];
        float litZ = lights[litIdx + LIGHT_DATA_OFFSET_POSZ];
        #pragma GCC unroll 6
        for (int j=0;j<6;++j) {
            mat4_lookat_from((float*)lightView[i][j], &cubemapOrientationQuaternion[j], litX, litY, litZ);
            mul_mat4((float*)lightViewProj[i][j], shadowmapsPerspectiveProjection, (float*)lightView[i][j]);
            ExtractFrustumPlanes((float*)lightViewProj[i][j], lightFrustumPlanes[i][j]);
        }
    }
    
    glNamedBufferData(voxelLightListIndicesID, VOXEL_COUNT * 2 * sizeof(uint32_t), voxelLightListIndices, GL_DYNAMIC_DRAW);
    glNamedBufferData(voxelLightListsRawID, head * sizeof(uint32_t), voxelLightListsRaw, GL_DYNAMIC_DRAW);
    glNamedBufferData(lightsID,loadedLights * LIGHT_DATA_SIZE * sizeof(float), lights, GL_DYNAMIC_DRAW);
}

void VoxelLists() {
    DebugRAM("start of VoxelLists");
    voxelLightListsRaw = malloc(VOXEL_COUNT * 4 * sizeof(uint32_t));
    voxelLightListIndices = malloc(VOXEL_COUNT * 2 * sizeof(uint32_t));
    voxelLightListIndicesID = SetupSSBO(voxelLightListIndicesID, 26, VOXEL_COUNT * 2 * sizeof(uint32_t), NULL, GL_DYNAMIC_DRAW);
    voxelLightListsRawID = SetupSSBO(voxelLightListsRawID, 27,  1008105 * sizeof(uint32_t), NULL, GL_DYNAMIC_DRAW);
    shadowmapIndirectionList = malloc(loadedLights * sizeof(uint32_t));
    lightsID = SetupSSBO(lightsID, 19, loadedLights * LIGHT_DATA_SIZE * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    lightView      = calloc(loadedLights * 6 * 4 * 4, sizeof(float));
    lightViewProj  = calloc(loadedLights * 6 * 4 * 4, sizeof(float));
    lightFrustumPlanes   = calloc(loadedLights * 6 * 6, sizeof(FrustumPlane));
    DebugRAM("prior to UpdateVoxelLightLists");
    UpdateVoxelLightLists();
    float mat[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    memcpy(&modelMatrices[0], mat, 16 * sizeof(float)); // Null instance matrix used for UI
    for (uint16_t i = 3; i < INSTANCE_COUNT; i++) UpdateInstanceMatrix(i); // Skip player indices and start at 3
    matricesBuffer = SetupSSBO(matricesBuffer, 11, INSTANCE_COUNT * 16 * sizeof(float), modelMatrices, GL_DYNAMIC_DRAW);
    shadowMapsIndirectionID = SetupSSBO(shadowMapsIndirectionID, 8, loadedLights * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
    DebugRAM("end of VoxelLists");
}

void RenderShadowmap(uint16_t lightIdx) {
    uint32_t litIdx = lightIdx * LIGHT_DATA_SIZE;
    float lightPosX = lights[litIdx + LIGHT_DATA_OFFSET_POSX];
    float lightPosY = lights[litIdx + LIGHT_DATA_OFFSET_POSY];
    float lightPosZ = lights[litIdx + LIGHT_DATA_OFFSET_POSZ];
    float lightRadius = lights[litIdx + LIGHT_DATA_OFFSET_RANGE];
    float effectiveRadius = vmin(lightRadius, 15.36f);
    uint16_t nearMeshes[loadedInstances];
    uint16_t nearbyMeshCount = 0;
    for (uint16_t j = 3; j < loadedInstances; j++) { // Skip player indices and start at 3
        if (instances[j].modelIndex >= loadedModels) continue;
        if (modelVertexCounts[instances[j].modelIndex] < 1) continue;
        
        float radius = modelBounds[(instances[j].modelIndex * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_RADIUS];
        float distToLightSqrd = squareDistance3D(instances[j].position.x, instances[j].position.y, instances[j].position.z, lightPosX, lightPosY, lightPosZ);
        float radSum = (effectiveRadius + radius);
        if (distToLightSqrd > radSum * radSum) continue;
        
        nearMeshes[nearbyMeshCount] = j;
        nearbyMeshCount++;
    }

    glUniform1ui(3, lightIdx);
    for (uint8_t face = 0; face < 6; face++) {
        glUniform1i(2, (shadowmapIndirectionList[lightIdx] * (6 * SHADOW_MAP_SIZE_SQD)) + (face * SHADOW_MAP_SIZE_SQD));
        glUniformMatrix4fv(1, 1, GL_FALSE, (float*)lightViewProj[lightIdx][face]);
        for (uint16_t j = 0; j < nearbyMeshCount; ++j) {
            int i = nearMeshes[j];
            if (instances[i].modelIndex >= loadedModels) continue;
            if (modelVertexCounts[instances[i].modelIndex] < 1) continue; // Empty model
            
            float radius = modelBounds[(instances[i].modelIndex * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_RADIUS] * 2.56f; // Could use 1.42f for diagonal length of unit square, but this is fine.
            if (!SphereInFrustum(lightFrustumPlanes[lightIdx][face], instances[i].position.x, instances[i].position.y, instances[i].position.z, radius)) continue;

            int32_t modelType = instanceIsLODArray[i] && instances[i].lodIndex < loadedModels ? instances[i].lodIndex : instances[i].modelIndex;
            glUniform1ui(0, i);
            glBindVertexBuffer(0, vbos[modelType], 0, VERTEX_ATTRIBUTES_COUNT * sizeof(float));
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tbos[modelType]);
            glDrawElements(GL_TRIANGLES, modelTriangleCounts[modelType] * 3, GL_UNSIGNED_INT, 0);
            drawCallsRenderedThisFrame++;
            verticesRenderedThisFrame += modelTriangleCounts[modelType] * 3;
        }
    }
}

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

void RenderShadowmaps(void) {
    DebugRAM("start of RenderShadowmaps");
    if (settings_Shadows < 1u) return;

    glUseProgram(shadowmapsClearShaderProgram);
    GLuint groupX_shadClear = (totalShadowmapPixels + 31) / 32;
    glDispatchCompute(groupX_shadClear,1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    shadowDrawCallsRenderedThisFrame = 0;
    memset(shadowmapIndirectionList, MAX_SHADOWMAPS + 1, loadedLights * sizeof(uint32_t));
    glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
    glUseProgram(shadowmapsShaderProgram);
    glProgramUniform1i(shadowmapsShaderProgram, 4, (int32_t)SHADOW_MAP_SIZE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    glBindVertexArray(vao_chunk);
    LightCandidate candidates[loadedLights];
    uint32_t candidateCount = 0;
    for (uint16_t i = 0; i < loadedLights; ++i) { // Collect candidates: only lights that are enabled, within FAR_PLANE, and in PVS
        uint32_t litIdx = i * LIGHT_DATA_SIZE;
        float intensity = lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY];
        if (intensity < 0.1f) continue;
        
        float lightPosX = lights[litIdx + LIGHT_DATA_OFFSET_POSX];
        float lightPosY = lights[litIdx + LIGHT_DATA_OFFSET_POSY];
        float lightPosZ = lights[litIdx + LIGHT_DATA_OFFSET_POSZ];
        float distSqrd = squareDistance3D(instances[PLAYER1].position.x, instances[PLAYER1].position.y, instances[PLAYER1].position.z, lightPosX, lightPosY, lightPosZ);
        if (distSqrd >= FAR_PLANE_SQUARED) continue;
        
        int lightCellIdx = cellIndexForLight[i];
        bool inPVS = (gridCellStates[lightCellIdx] & CELL_VISIBLE);
        if (!inPVS) {
            int x = cellIndexForLightX[i];
            int z = cellIndexForLightZ[i];
            float range = lights[litIdx + LIGHT_DATA_OFFSET_RANGE];
            int r = vfloor(range * 0.390625f);
            for (int ix = x - r; ix <= x + r && !inPVS; ix++) {
                for (int iz = z - r; iz <= z + r; iz++) {
                    if (!XZPairInBounds(ix, iz)) continue;
                    int subIdx = iz * WORLDX + ix;
                    if ((gridCellStates[subIdx] & CELL_VISIBLE) &&
                        get_cull_bit(precomputedVisibleCellsFromHere, lightCellIdx * ARRSIZE + subIdx)) {
                        inPVS = true;
                        break;
                    }
                }
            }
        }
        if (!inPVS) continue;
        
        float score = distSqrd / vmax(intensity, 0.01f);
        candidates[candidateCount++] = (LightCandidate){ .index = i, .distanceSquared = distSqrd, .score = score };
    }

    qsort(candidates, candidateCount, sizeof(LightCandidate), compareLightCandidates);
    uint32_t numToRender = vmin(candidateCount, MAX_SHADOWMAPS);
    for (uint32_t c = 0; c < numToRender; ++c) { // Render top MAX_SHADOWMAPS candidates
        uint16_t lightIdx = candidates[c].index;
        uint32_t slot = shadowDrawCallsRenderedThisFrame;
        shadowmapIndirectionList[lightIdx] = slot;
        RenderShadowmap(lightIdx);
        shadowDrawCallsRenderedThisFrame++;
    }

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT);
    glViewport(0, 0, screen_width, screen_height);
    glEnable(GL_CULL_FACE);
    glNamedBufferData(shadowMapsIndirectionID, loadedLights * sizeof(uint32_t), shadowmapIndirectionList, GL_DYNAMIC_DRAW);
    malloc_trim(0);
    DebugRAM("end of RenderShadowmaps");
}

// ============================================================================
// UI Rendering and Text
float GetScreenRelativeX(float percentage) { return (float)screen_width * percentage; }
float GetScreenRelativeY(float percentage) { return (float)screen_height * percentage; }

uint32_t AddUIImage(float x, float y, float z, float width, float height, uint32_t texIndex) {
    if (uiImageCount >= MAX_UI_IMAGES) { DualLogError("Max UI images reached!\n"); return 0; }
    
    uiImages[uiImageCount].x = x;
    uiImages[uiImageCount].y = y;
    uiImages[uiImageCount].z = z;
    uiImages[uiImageCount].width = width;
    uiImages[uiImageCount].height = height;
    uiImages[uiImageCount].texIndex = texIndex;
    uiImages[uiImageCount].visible = true;
    uiImageCount++;
    return uiImageCount - 1; // Return index of this just now created image for use on making buttons.
}

float uiImageVertexData[31768];
void RenderUIImages() {
    if (uiImageCount == 0) return;

    glUseProgram(chunkShaderProgram);
    glBindVertexArray(textVAO);
    glProgramUniform1ui(chunkShaderProgram, 3, 1u); // isUI true
    glProgramUniform1ui(chunkShaderProgram, 17, 1u); // unlit is true
    glProgramUniformMatrix4fv(chunkShaderProgram, 2, 1, GL_FALSE, uiOrthoProjection);
    for (uint32_t i = 0; i < uiImageCount; i++) { // Sort images by texIndex to minimize state changes.  Simple bubble sort for small N
        for (uint32_t j = i + 1; j < uiImageCount; j++) {
            if (uiImages[j].texIndex < uiImages[i].texIndex) {
                UIImage temp = uiImages[i];
                uiImages[i] = uiImages[j];
                uiImages[j] = temp;
            }
        }
    }

    uint32_t start = 0;
    while (start < uiImageCount) {
        uint32_t currentTex = uiImages[start].texIndex;
        uint32_t end = start;
        while (end < uiImageCount && uiImages[end].texIndex == currentTex && uiImages[end].visible) end++; // Find range with same texIndex
        size_t vertexCount = 0;
        for (uint32_t i = start; i < end; i++) {  // Build vertex buffer for this batch
            if (!uiImages[i].visible) continue;

            float x0 = uiImages[i].x;
            float y0 = uiImages[i].y;
            float z0 = uiImages[i].z;
            float x1 = x0 + uiImages[i].width;
            float y1 = y0 + uiImages[i].height;
            float vertices[30] = { x0, y1, z0, 0.0f, 0.0f,
                                   x1, y0, z0, 1.0f, 1.0f,
                                   x1, y1, z0, 1.0f, 0.0f,
                                   x0, y1, z0, 0.0f, 0.0f,
                                   x0, y0, z0, 0.0f, 1.0f,
                                   x1, y0, z0, 1.0f, 1.0f };

            memcpy(uiImageVertexData + vertexCount * 30, vertices, sizeof(vertices));
            vertexCount++;
        }

        if (vertexCount > 0) {
            glUniform1ui(1, BLACK_TEXTURE_IDX);
            glUniform1ui(18, currentTex);
            glUniform1ui(19, BLACK_TEXTURE_IDX);
            glUniform1ui(20, BLACK_TEXTURE_IDX);
            glNamedBufferData(textVBO, vertexCount * 30 * sizeof(float), uiImageVertexData, GL_DYNAMIC_DRAW);
            glDrawArrays(GL_TRIANGLES, 0, vertexCount * 6);
            drawCallsRenderedThisFrame++;
            uiImageDrawCallsRenderedThisFrame++;
            verticesRenderedThisFrame += vertexCount * 6;
        }

        start = end;
    }

    glBindVertexArray(0);
    glUseProgram(0);
}

bool CursorIsOverBounds(float startX, float endX, float startY, float endY) {
    return (   cursorPosition_x >= startX && cursorPosition_x <= endX     // 0 == left
            && cursorPosition_y >= endY   && cursorPosition_y <= startY); // 0 == top
}

float textVertexData[8192]; // Reusable buffer for text vertices.  Most text only needs ~3000
void RenderFormattedText(float x, float y, float z, uint32_t color, uint8_t fontID, const char* format, ...) {
    va_list args;
    va_start(args, format); vsnprintf(uiTextBuffer, TEXT_BUFFER_SIZE, format, args); va_end(args);
    glUseProgram(textShaderProgram);
    glProgramUniformMatrix4fv(textShaderProgram, 0, 1, GL_FALSE, uiOrthoProjection);
    glProgramUniform4f(textShaderProgram, 3, textColors[color].r, textColors[color].g, textColors[color].b, textColors[color].a);
    if (fontID == FONT_STOPD) glBindTextureUnit(6, fontAtlasTexStopD);
    else glBindTextureUnit(6, fontAtlasTex);
    
    glProgramUniform2f(textShaderProgram, 4, 1.0f / (float)FONT_ATLAS_SIZE, 1.0f / (float)FONT_ATLAS_SIZE);
    glProgramUniform1ui(textShaderProgram, 2, fontID);
    glProgramUniform1i(textShaderProgram, 1, 6); // textTexture sampler2D
    glBindVertexArray(textVAO);
    size_t vertexCount = 0;
    const char* p = uiTextBuffer;
    float xpos = x, ypos = y + GetScreenRelativeY(0.0211f);
    float lineSpacing = GetScreenRelativeY(0.03f); // Match RenderUI
    stbtt_aligned_quad q;
    int characterCount = 0;
    float paddingUV = 12.0f / (float)FONT_ATLAS_SIZE; // This is for the black outline around all text for readability.
    float borderWidthPixels = 2.0f;
    while (*p) {
        uint32_t codepoint = DecodeUTF8(&p);
        characterCount++;
        if (codepoint == '\n' || characterCount > 120) {
            xpos = x;
            ypos += lineSpacing;
            characterCount = 0;
            continue;
        }

        int idx = CodepointToPackedIndex(codepoint, fontID);
        if (idx < 0) continue;

        if (fontID == FONT_STOPD) stbtt_GetPackedQuad(fontPackedCharStopD, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, idx, &xpos, &ypos, &q, 1);
        else stbtt_GetPackedQuad(fontPackedChar, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, idx, &xpos, &ypos, &q, 1);
        float vx0 = q.x0 - borderWidthPixels;
        float vy0 = q.y0 - borderWidthPixels;
        float vx1 = q.x1 + borderWidthPixels;
        float vy1 = q.y1 + borderWidthPixels;
        float s0 = q.s0 - paddingUV;
        float t0 = q.t0 - paddingUV;
        float s1 = q.s1 + paddingUV;
        float t1 = q.t1 + paddingUV;
        float textVertices[30] = { vx0, vy0, z, s0, t0, // Triangle 1
                                   vx1, vy1, z, s1, t1,
                                   vx1, vy0, z, s1, t0,
                                   vx0, vy0, z, s0, t0, // Triangle 2
                                   vx0, vy1, z, s0, t1,
                                   vx1, vy1, z, s1, t1 };

        memcpy(textVertexData + vertexCount * 30, textVertices, sizeof(textVertices));
        vertexCount++;
        if (codepoint >= '0' && codepoint <= '9') {
            if (fontID == FONT_STOPD) xpos = q.x0 + fixedNumberAdvanceWidthStopD;
            else xpos = q.x0 + fixedNumberAdvanceWidth;
        }
    }
    if (vertexCount > 0) {
        glNamedBufferData(textVBO, vertexCount * 30 * sizeof(float), textVertexData, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount * 6);
        drawCallsRenderedThisFrame++;
        textDrawCallsRenderedThisFrame++;
        verticesRenderedThisFrame += vertexCount * 6;
    }
    
    glBindVertexArray(0);
}

void RenderLoadingProgress(int32_t offset, const char* format, ...) { // Only adds 0.01secs to game startup time.
    glUseProgram(imageBlitShaderProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputImageID);
    glProgramUniform1i(imageBlitShaderProgram, 27, 0); // Texture 0 for the rendered geometry color buffer
    glBindVertexArray(quadVAO);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    drawCallsRenderedThisFrame++;
    verticesRenderedThisFrame += 4;
    glBindTextureUnit(0, 0);
    glUseProgram(0);
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    RenderFormattedText(screen_width / 2 - offset, screen_height / 2 - 5, UI_LAYER_5, TEXT_WHITE, FONT_NORMAL, buffer);
    glEnable(GL_DEPTH_TEST);
    glfwSwapBuffers(window);
}

void CenterStatusPrint(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    statusTextLengthWithoutNullTerminator = vsnprintf(statusText, TEXT_BUFFER_SIZE, fmt, args);
    va_end(args);
    DualLog("%s\n",statusText);
    statusTextDecayFinished = get_time() + 2.5f; // 2.5 second decay time before text dissappears.
}
// ============================================================================
void InitializePlayer(uint16_t playerIdx) { // Just setting the things that are nonzero
    instances[playerIdx].index = 767;
    instances[playerIdx].position.x = 10.2f; // Start Actual: Puts player on Medical Level in actual game start position
    instances[playerIdx].position.y = -43.792f + 0.84f; // Added 0.84f for cam offset from center
    instances[playerIdx].position.z = 20.40001f;
    instances[playerIdx].velocity.x = instances[playerIdx].velocity.y = instances[playerIdx].velocity.z = 0.0f;
    instances[playerIdx].scale.x = instances[playerIdx].scale.y = instances[playerIdx].scale.z = 1.0f;
    instances[playerIdx].rotation.x = instances[playerIdx].rotation.y = instances[playerIdx].rotation.z = 0.0f; instances[playerIdx].rotation.w = 1.0f;
    flag_enable(&instances[playerIdx].entflags, ENTFLAG_ACTIVE);
    flag_enable(&instances[playerIdx].entflags, ENTFLAG_USEGRAVITY);
    flag_enable(&instances[playerIdx].entflags, ENTFLAG_RIGIDBODY);
    instances[playerIdx].collider = COLLIDER_TYPE_CAPSULE;
    instances[playerIdx].colliderCenter.y = 0.84f;
    instances[playerIdx].colliderSize.x = 0.48f; // Radius
    instances[playerIdx].colliderSize.y = 2.0f;  // Overall height including end radii (Unity convention, blech)
    instances[playerIdx].colliderSize.z = COLLIDER_CAPSULE_DIRECTION_Y_F; // Direction, 1.0 == Y-Axis
    instances[playerIdx].mass = 1.0f;
    instances[playerIdx].linearDrag = 8.0f;
    instances[playerIdx].dynamicFriction = 0.6f;
    instances[playerIdx].staticFriction = 0.8f;
    instances[playerIdx].frictionCombine = PHYS_COMBINE_MUL;
}

void NewGame(void) {
    RenderLoadingProgress(100,"Loading new game...");
    memset(&ambientRegistry, 0, sizeof(uint16_t));
    memset(&questData, 0, sizeof(QuestBits));
    questData.lev1SecCode = random_range_u8(0u,9u); // Must do rand's repeatedly to prevent
    questData.lev2SecCode = random_range_u8(0u,9u); // these all being the same number.
    questData.lev3SecCode = random_range_u8(0u,9u);
    questData.lev4SecCode = random_range_u8(0u,9u);
    questData.lev5SecCode = random_range_u8(0u,9u);
    questData.lev6SecCode = random_range_u8(0u,9u);
    memset(instances,0,3 * sizeof(Entity)); // Initialize instances, the global entity array for the currently loaded level.
    InitializePlayer(PLAYER1); InitializePlayer(PLAYER2);
    levelCurrentlyLoading = true;
    LoadLevel(startLevel); // Must be after entities!
    pauseRelativeTime = 0.0f;
}

int currentMonitorIndex = 0;
bool ignore_next_mouse_delta = false;
void CycleToNextMonitor(GLFWwindow* window) {
    if (get_time() < monitorSwitchTime) return;
    
    monitorSwitchTime = get_time() + 1.5f; // Dumb hack to prevent toggling every frame from keypress illogic
    int monitorCount;
    GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
    if (!monitors || monitorCount < 2) return;

    currentMonitorIndex = (currentMonitorIndex + 1) % monitorCount;
    GLFWmonitor* next = monitors[currentMonitorIndex];

    int mx, my;
    glfwGetMonitorPos(next, &mx, &my);
    const GLFWvidmode* mode = glfwGetVideoMode(next);
    int xpos = mx + (mode->width - screen_width) / 2;
    int ypos = my + (mode->height - screen_height) / 2;
    glfwSetWindowPos(window, xpos, ypos);
    ignore_next_mouse_delta = true;
    DualLog("Window moved to monitor %d: %s at x: %d, y: %d\n", currentMonitorIndex, glfwGetMonitorName(next), xpos, ypos);
}

void InitializeEnvironment(void) {
    double init_start_time = get_time();
    if (!glfwInit()) { DualLogError("GLFW initialization failed\n"); exit(1); }
    
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SRGB_CAPABLE, 0);
    glfwWindowHint(GLFW_RESIZABLE, 0);
    window = glfwCreateWindow(screen_width, screen_height, "Voxen, the OpenGL Voxel Lit Engine", NULL, NULL);
    if (!window) { DualLogError("glfwCreateWindow failed\n"); exit(1); }
        
    glfwMakeContextCurrent(window);
    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) { DualLogError("Failed to initialize GLAD\n"); exit(1); }
    GLFWmonitor* target_monitor = glfwGetPrimaryMonitor();  // Use primary; or monitors[1] for second monitor, etc.
    if (target_monitor) { // TODO: Let user switch monitors from settings, especially in fullscreen.
        const GLFWvidmode* mode = glfwGetVideoMode(target_monitor);
        int mx, my;
        glfwGetMonitorPos(target_monitor, &mx, &my);
        int xpos = mx + (mode->width - screen_width) / 2;
        int ypos = my + (mode->height - screen_height) / 2;
        glfwSetWindowPos(window, xpos, ypos);
        DualLog("Window positioned (windowed, centered) on monitor: %s (primary) at %d,%d\nUsing GLFW %s, ", glfwGetMonitorName(target_monitor), xpos, ypos,glfwGetVersionString());
    } else { DualLogError("GLFW Unable to obtain target monitor [primary]!\n"); exit(1); }
    
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    const GLubyte* version = glGetString(GL_VERSION);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    if (!version) { DualLogError("OpenGL support not found!\n"); exit(1);}
    
    DualLog("OpenGL Version: %s, ", (const char*)version);
    DualLog("GPU: %s", renderer ? (const char*)renderer : "unknown");
    char cpu_brand[256] = "Unknown CPU";
    int logical_cores = 1;
    FILE* f = fopen("/proc/cpuinfo", "r");
    if (f) {
        char line[512];
        while (fgets(line, sizeof(line), f)) {
            if (!strncmp(line, "model name", 10)) {
                char* colon = strchr(line, ':');
                if (colon) {
                    strncpy(cpu_brand, colon + 2, sizeof(cpu_brand) - 1);
                    char* nl = strchr(cpu_brand, '\n');
                    if (nl) *nl = '\0';
                }
                break;
            }
        }
        fclose(f);
    }
    logical_cores = (int)sysconf(_SC_NPROCESSORS_ONLN);
    if (logical_cores <= 0) logical_cores = 1;
    DualLog("CPU: %s | Logical cores: %d\n", cpu_brand, logical_cores);
    DebugRAM("GL Buffer and shader setup");
    Input_Init(window);
    glfwSwapInterval(settings_Vsync ? 1 : 0);
    glFrontFace(GL_CCW); // Set triangle sorting order (GL_CW vs GL_CCW)
    CompileShaders();
    glCreateBuffers(1, &quadVBO);
    float quadBlit_vertices[] = { 1.0f, -1.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,    -1.0f,1.0f, 0.0f, 1.0f,   -1.0f, -1.0f, 0.0f, 0.0f }; // 4 verts, 4 floats each pos.xy, uv.xy
    glNamedBufferData(quadVBO, sizeof(quadBlit_vertices), quadBlit_vertices, GL_STATIC_DRAW);
    glCreateVertexArrays(1, &quadVAO);
    glEnableVertexArrayAttrib(quadVAO, 0);
    glEnableVertexArrayAttrib(quadVAO, 1);
    glVertexArrayAttribFormat(quadVAO, 0, 2, GL_FLOAT, GL_FALSE, 0); // DSA: Set position format
    glVertexArrayAttribFormat(quadVAO, 1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float)); // DSA: Set texcoord format
    glVertexArrayVertexBuffer(quadVAO, 0, quadVBO, 0, 4 * sizeof(float)); // DSA: Link VBO to VAO
    glVertexArrayAttribBinding(quadVAO, 0, 0); // DSA: Bind position attribute to binding index 0
    glVertexArrayAttribBinding(quadVAO, 1, 0); // DSA: Bind texcoord attribute to binding index 0
    
    glGenVertexArrays(1, &vao_chunk);
    glBindVertexArray(vao_chunk);
    glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0); // Position (vec3)
    glVertexAttribFormat(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float)); // Normal (vec3)
    glVertexAttribFormat(2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float)); // Tex Coord (vec2)
    for (uint8_t i = 0; i < 3; i++) { glVertexAttribBinding(i, 0); glEnableVertexAttribArray(i); }
    glBindVertexArray(0);
    
    glCreateBuffers(1, &textVBO);
    glCreateVertexArrays(1, &textVAO);    
    glEnableVertexArrayAttrib(textVAO, 0);
    glEnableVertexArrayAttrib(textVAO, 1);
    glVertexArrayAttribFormat(textVAO, 0, 3, GL_FLOAT, GL_FALSE, 0); // pos (x,y,z) 4 floats per vertex, stride = 4*sizeof(float)
    glVertexArrayAttribFormat(textVAO, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float));  // uv (s,t)
    glVertexArrayVertexBuffer(textVAO, 0, textVBO, 0, 5 * sizeof(float));
    glVertexArrayAttribBinding(textVAO, 0, 0);
    glVertexArrayAttribBinding(textVAO, 1, 0);
    DebugRAM("after vao chunk bind");

    GenerateAndBindTexture(&inputImageID,             GL_RGBA8, screen_width, screen_height,            GL_RGBA, GL_UNSIGNED_BYTE, GL_TEXTURE_2D); // Lit Raster
    GenerateAndBindTexture(&inputWorldPosID,        GL_RGBA32F, screen_width, screen_height,            GL_RGBA,         GL_FLOAT, GL_TEXTURE_2D); // Raster World Positions
    GenerateAndBindTexture(&inputDepthID, GL_DEPTH_COMPONENT32, screen_width, screen_height, GL_DEPTH_COMPONENT,         GL_FLOAT, GL_TEXTURE_2D); // Raster Depth
    GenerateAndBindTexture(&inputSpecID,              GL_RGBA8, screen_width, screen_height,            GL_RGBA, GL_UNSIGNED_BYTE, GL_TEXTURE_2D); // Specular Colors
    glGenTextures(1, &outputImageID);
    glBindTexture(GL_TEXTURE_2D, outputImageID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,  screen_width / SSR_RES,  screen_height / SSR_RES, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glGenFramebuffers(1, &gBufferFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, inputImageID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, inputWorldPosID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, inputSpecID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, inputDepthID, 0);
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, drawBuffers);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) DualLogError("Framebuffer incomplete: Error code %d\n", status);
    glBindImageTexture(0, inputImageID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8); // Main Rendered Color
    glBindImageTexture(1, inputWorldPosID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F); // World Position XYZ 32bit, Normal XYZ 8bit, 8bits empty
    glBindImageTexture(2, inputSpecID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8); // Specular
    //                 3 = depth
    glBindImageTexture(4, outputImageID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8); // SSR result
    glActiveTexture(GL_TEXTURE3); // Match binding = 3 in shader
    glBindTexture(GL_TEXTURE_2D, inputDepthID);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, outputImageID);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    DebugRAM("setup gbuffer end");
    UpdateScreenSize();
    InitFontAtlasses();
    DebugRAM("after InitFontAtlasses");
    Input_MouselookApply(); // Input
    InitializeAudio(); // Audio
    DebugRAM("after InitializeAudio");
    malloc_trim(0);
    DebugRAM("GL inits end");
    LoadTextForLanguage(settings_Language);
    DebugRAM("LoadTextForLanguage end");
    LoadLogTextForLanguage(settings_Language);
    DebugRAM("LoadLogTextForLanguage end");
    ParseGameData();
    DebugRAM("ParseGameData end");
    glfwSetWindowTitle(window,global_modname);
    int fp = open("./Textures/UI/menudot1.png", O_RDONLY);
    if (!fp) { DualLogError("Failed to open ./Textures/UI/menudot1.png: %s\n", strerror(errno)); exit(1); }
 
    DebugRAM("after setting window title");
//     struct stat file_stat;
//     fstat(fp, &file_stat);
//     size_t file_size = file_stat.st_size;            
//     uint8_t* file_buffer = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fp, 0);
//     close(fp);
//     if (file_buffer == MAP_FAILED) { DualLogError("Failed to mmap ./Textures/UI/menudot1.png\n"); exit(1); }
                
//     int w = 1, h = 1;
//     unsigned char* pixels = stbi_load_from_memory(file_buffer, file_size, &w, &h);
//     if (!pixels) { DualLogError("Failed to load icon: ./Textures/UI/menudot1.png\n"); exit(1); }
// 
//     DebugRAM("after loading window bar icon");
//     GLFWimage image;
//     image.width  = w;
//     image.height = h;
//     image.pixels = pixels;
//     glfwSetWindowIcon(window, 1, &image);
//     free(pixels);
//     munmap(file_buffer,file_size);
//     madvise(pixels, w * h * 4, MADV_DONTNEED);
    malloc_trim(0);
    DebugRAM("after freeing window bar icon");
    DualLog("GL buffers, FBO, fonts, audio, localization, and window init took %f secs\n", get_time() - init_start_time);
    LoadTextures(); // Sequential due to GPU transfers
    LoadModels(); // Sequential due to GPU transfers
    LoadEntities(); // Had a note to do this after textures and models, didn't seem necessary but giving it a thread didn't help init times.
//     play_mp3("./Audio/music/TITLOOP-00_menu.mp3",((float)settings_VolumeMusic/100.0f) * 0.4f + 0.09f,1500);
    NewGame(); // TODO: Do this from menu not immediately lol
    DebugRAM("InitializeEnvironment end");
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
                               startOfNextType = startOfDoubleSidedInstances;
                               glDisable(GL_BLEND);
                               glDepthMask(GL_TRUE);
                               glEnable(GL_DEPTH_TEST);
                               glEnable(GL_CULL_FACE); break;
        case REND_DOUBLESIDED: glDisable(GL_CULL_FACE);
                               countsArray  =  modelTypeCountsDoubleSided;
                               offsetsArray = modelTypeOffsetsDoubleSided;
                               startOfNextType = startOfTransparentInstances; break;
        case REND_TRANSPARENT: glEnable(GL_BLEND);
                               glEnable(GL_CULL_FACE);
                               glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                               glDepthMask(GL_FALSE);
                               countsArray  =  modelTypeCountsTransparent;
                               offsetsArray = modelTypeOffsetsTransparent;
                               startOfNextType = loadedInstances - invalidModelIndexCount; break;
    }
    
    for (uint16_t modelIdx = 0; modelIdx < loadedModels; modelIdx++) {
        if (countsArray[modelIdx] == 0) continue;

        uint16_t start = offsetsArray[modelIdx];
        if (start < 3) DualLogError("offsets for rendering wrong!\n");
        uint16_t count =  countsArray[modelIdx];
        DepthSort visibleInstances[start + count];
        uint16_t visibleCount = 0;
        for (uint16_t i = start; i < start + count && i < startOfNextType; i++) { // Filter visible instances
            uint16_t instCellIdx = (uint16_t)cellIndexForInstance[i];
            float distSqrd = squareDistance3D(      instances[i].position.x,       instances[i].position.y,       instances[i].position.z,
                                              instances[PLAYER1].position.x, instances[PLAYER1].position.y, instances[PLAYER1].position.z);
            if (settings_CullEnabled) {
                if (instCellIdx < ARRSIZE && !(gridCellStates[instCellIdx] & CELL_VISIBLE)) continue;
                if (distSqrd >= FAR_PLANE_SQUARED) continue;
            }
            
            visibleInstances[visibleCount].index = i;
            visibleInstances[visibleCount].depth = distSqrd;
            visibleCount++;
            instanceIsLODArray[i] = (distSqrd >= lodRangeSqrd);
        }
        
        if (visibleCount == 0) continue;
        
        if (type == REND_TRANSPARENT) qsort(visibleInstances, visibleCount, sizeof(DepthSort), compareDepthSort); // Sort by depth (descending for back-to-front)
        else                          qsort(visibleInstances, visibleCount, sizeof(DepthSort), compareDepthSortInverted); // Sort by depth (ascending for front-to-back)
        
        for (uint16_t j = 0; j < visibleCount; j++) {
            uint16_t i = visibleInstances[j].index;
            uint32_t texIndex = instances[i].texIndex;
            glUniform1ui(0, i);
            glUniform1ui(1, (uint32_t)instances[i].normIndex);
            glUniform1ui(18, texIndex);
            glUniform1ui(19, (uint32_t)instances[i].glowIndex);
            glUniform1ui(20, (uint32_t)instances[i].specIndex);
            int32_t modelType = instanceIsLODArray[i] && instances[i].lodIndex < loadedModels ? instances[i].lodIndex : instances[i].modelIndex;
            uint32_t vertCount = modelTriangleCounts[modelType] * 3;
            glBindVertexBuffer(0, vbos[modelType], 0, VERTEX_ATTRIBUTES_COUNT * sizeof(float));
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tbos[modelType]);
            glDrawElements(GL_TRIANGLES, vertCount, GL_UNSIGNED_INT, 0);
            drawCallsRenderedThisFrame++;
            verticesRenderedThisFrame += vertCount;
        }
    }
}

void SetFog() {
    fogColorRUsed = fogColorR * fogBaseDensityForLevel;
    fogColorGUsed = fogColorG * fogBaseDensityForLevel;
    fogColorBUsed = fogColorB * fogBaseDensityForLevel;
    glProgramUniform1f(chunkShaderProgram, 11, fogColorRUsed);
    glProgramUniform1f(chunkShaderProgram, 12, fogColorGUsed);
    glProgramUniform1f(chunkShaderProgram, 13, fogColorBUsed);
}

double timeSinceLastPhysicsTick = 0.0;

int32_t main(int32_t argc, char* argv[]) {
    game_start_time = get_time();
    random_range_rng = (uint32_t)game_start_time; // Seed global rand uniquely with time since system boot.
    OpenConsoleLogFile();
    DebugRAM("program start");
    if (argc >= 2 && (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0)) { DualLog("-----------------------------------------------------------\nVoxen " VERSION_STRING "\nthe Voxel Lit Open Source Game Engine\nby W. Josiah Jack\nMIT-0 licensed\n"); return 0; }
    if ((argc >= 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0))
        || (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) ) {
        DualLog("Voxen the Voxel Lit Open Source Game Engine\n");
        DualLog("-------------------------------------------------------------\n");
        DualLog("   This is a game engine designed for optimized focused usage\n");
        DualLog("   of OpenGL, making heavy use of GPU Driven rendering\n");
        DualLog("   techniques, a unified event system for debugging and log\n");
        DualLog("   playback, full mod support loading all data from external\n");
        DualLog("   files and using definition files for what to do with the\n");
        DualLog("   data.\n\n");
        DualLog("   This project aims to have minimal overhead, profiling,\n");
        DualLog("   traceability, robustness, and low level control.\n\n");
        DualLog("\n");
        DualLog("Valid arguments:\n");
        DualLog(" < none >\n    Runs the engine as normal, loading data from \n    neighbor directories (./Textures, ./Models, etc.)\n\n");
        DualLog("-v, --version\n    Prints version information\n\n");
        DualLog("play <file>\n    Plays back recorded log from current directory\n\n");
        DualLog("record <file>\n    Records all engine events to designated log\n    as a .dem file\n\n");
        DualLog("dump <file.dem>\n    Dumps the specified log into ./log_dump.txt\n    as human readable text.  You must provide full\n    file name with extension\n\n");
        DualLog("-h, --help\n    Provides this help text.  Neat!\n");
        DualLog("-----------------------------------------------------------\n");
        return 0;
    }

    if (argc == 3 && strcmp(argv[1], "dump") == 0) { DualLog("Converting log to plaintext: %s ...", argv[2]); JournalDump(argv[2]); DualLog("DONE!\n"); return 0; }

    globalFrameNum = 0;
    DebugRAM("prior to event system init");
    DualLog("Voxen " VERSION_STRING " by W. Josiah Jack, MIT-0 licensed\n");
    EventSystemInit(argc,argv[1],argv[2]);
    InitializeEnvironment();
    playerCellIdx_x = 0u; playerCellIdx_y = 0u; playerCellIdx_z = 0u; // Force a cull
    double last_physics_time = get_time();
    DebugRAM("prior to game loop");
    DualLog("Game Initialized in %f secs\n",get_time() - game_start_time);
    while(1) {
        current_time = get_time();
        double frame_time = current_time - last_time;
        if (!gamePaused) pauseRelativeTime += (float)frame_time;
        
        // Handle Berserk Effect for Compositing Shader
        float berserkTimeRemainingNormalized = berserkFinished > 0.0001f ? (berserkFinished - pauseRelativeTime) / PATCH_TIME_BERSERK : 0.0f;
        if (berserkFinished < pauseRelativeTime && berserkFinished > 0.0001f) berserkFinished = berserkTimeRemainingNormalized = 0.0f;
        InputClearRisingAndFallingEdges();
        glfwPollEvents();
        if (glfwWindowShouldClose(window)) EnqueueEvent(EV_QUIT,EV_INT_FIELD_UNUSED,EV_INT_FIELD_UNUSED,EV_FLOAT_FIELD_UNUSED,EV_FLOAT_FIELD_UNUSED);
        timeSinceLastPhysicsTick = pauseRelativeTime - last_physics_time;
//         if (timeSinceLastPhysicsTick > 0.006944444f && !gamePaused && !menuActive) { // 144fps fixed tick rate
            last_physics_time = pauseRelativeTime;
            EnqueueEvent(EV_PHYSICS_TICK,EV_INT_FIELD_UNUSED,EV_INT_FIELD_UNUSED,EV_FLOAT_FIELD_UNUSED,EV_FLOAT_FIELD_UNUSED);
//         }

        if (log_playback) { // Enqueue all logged events for the current frame.
            int32_t read_status = ReadActiveLog();
            if (read_status == 2) { // EOF reached, no more events
                DualLog("Log playback completed.  Control returned.\n");
            } else if (read_status == -1) { // Read error
                DualLogError("Error reading log file, exiting playback\n");
                EnqueueEvent(EV_QUIT,EV_INT_FIELD_UNUSED,EV_INT_FIELD_UNUSED,EV_FLOAT_FIELD_UNUSED,EV_FLOAT_FIELD_UNUSED);
            }
        }

        if (EventQueueProcess()) break; // Do everything
        
        drawCallsRenderedThisFrame = 0; // Reset per frame
        textDrawCallsRenderedThisFrame = 0;
        uiImageDrawCallsRenderedThisFrame = 0;
        shadowDrawCallsRenderedThisFrame = 0;
        verticesRenderedThisFrame = 0;
        uiImageCount = 0;
        memset(lightDirty,0,LIGHT_COUNT * sizeof(bool));
        
        // 0. View Matrix, and Projection Matrix
        float view[16]; // Also known as view matrix
        mat4_lookat_from(view,&cam_rotation, instances[PLAYER1].position.x, instances[PLAYER1].position.y, instances[PLAYER1].position.z);
        float viewProj[16]; // view-projection matrix
        mul_mat4(viewProj, rasterPerspectiveProjection, view);
        float invViewRot[9];
        invViewRot[0] = view[0]; invViewRot[3] = view[1]; invViewRot[6] = view[2];
        invViewRot[1] = view[4]; invViewRot[4] = view[5]; invViewRot[7] = view[6];
        invViewRot[2] = view[8]; invViewRot[5] = view[9]; invViewRot[8] = view[10];
        if (!gamePaused && !menuActive) { // !PAUSED BLOCK -------------------------------------------------
            UpdateAmbientSounds();
            
            // 1. Culling
            Cull(); // Get world cell culling data into gridCellStates from precomputed data at init of what cells see what other cells.
            
            // 2. Pass instance data to GPU
            for (uint32_t i = 3; i < loadedInstances; i++) { if (dirtyInstances[i]) { UpdateInstanceMatrix(i); } } // Skip player indices and start at 3
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, matricesBuffer);
            glBufferData(GL_SHADER_STORAGE_BUFFER, loadedInstances * 16 * sizeof(float), modelMatrices, GL_DYNAMIC_DRAW);
            
            // 3. Dynamic Shadowmaps
            for (int i = 0; i < loadedLights; ++i) {
                if (lightDirty[i]) {
                    UpdateVoxelLightLists(); // Takes 12ms of total frametime!!
                    if (settings_Shadows > 0u) RenderShadowmaps();
                    else {
                        memset(shadowmapIndirectionList, MAX_SHADOWMAPS + 1, loadedLights * sizeof(uint32_t));
                        glNamedBufferData(shadowMapsIndirectionID, loadedLights * sizeof(uint32_t), shadowmapIndirectionList, GL_DYNAMIC_DRAW);
                    }
                    
                    break;
                }
            }
            

            // 4. Raterized Geometry, Standard vertex + fragment rendering, but with special packing to minimize transfer data amounts
            glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the corner where last shadowmap wrote into  
            glUseProgram(chunkShaderProgram);
            glProgramUniformMatrix4fv(chunkShaderProgram, 2, 1, GL_FALSE, viewProj);
            glProgramUniform1ui(chunkShaderProgram, 3, 0u); // isUI false
            glProgramUniform1f(chunkShaderProgram, 8, worldMin_x);
            glProgramUniform1f(chunkShaderProgram, 9, worldMin_z);
            glProgramUniform3f(chunkShaderProgram, 10, instances[PLAYER1].position.x, instances[PLAYER1].position.y, instances[PLAYER1].position.z);
            glProgramUniform1ui(chunkShaderProgram, 14, settings_Reflections);
            glProgramUniform1ui(chunkShaderProgram, 15, settings_Shadows);
            glProgramUniform1ui(chunkShaderProgram, 17, 0u); // unlit false
            glBindVertexArray(vao_chunk);
            memset(instanceIsLODArray,true,INSTANCE_COUNT * sizeof(bool)); // All using lower detail LOD mesh.
            RenderInstances(REND_OPAQUE);      // Opaque, e.g. most objects and level geometry chunks
            RenderInstances(REND_DOUBLESIDED); // Double Sided, e.g. cyber panels and foliage and negative scaled objects
            RenderInstances(REND_TRANSPARENT); // Transparents, e.g. windows and beakers
            glBindVertexArray(0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // 5. SSR (Screen Space Reflections)
            if ((debugView == 0 || debugView == 4) && settings_Reflections > 0) {
                glUseProgram(ssrShaderProgram);
                glUniform1f(2, settings_SSRStepSize);
                glUniform1f(3, settings_SSRSampleWeight);
                glUniform1ui(4, settings_SSRStepCount);
                glUniformMatrix4fv(5, 1, GL_FALSE, viewProj);
                glUniform3f(6, instances[PLAYER1].position.x, instances[PLAYER1].position.y, instances[PLAYER1].position.z);
                GLuint groupX_ssr = ((screen_width / SSR_RES) + 31) / 32;
                GLuint groupY_ssr = ((screen_height / SSR_RES) + 31) / 32;
                glDispatchCompute(groupX_ssr, groupY_ssr, 1);
                glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            }
        } else { // END !PAUSED BLOCK -------------------------------------------------
            glBindFramebuffer(GL_FRAMEBUFFER, 0); // Allow text to still render while paused
        }
        
        // 6. Render final meshes' results with full screen quad
        glUseProgram(imageBlitShaderProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, inputImageID);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, outputImageID);
        glProgramUniform1i(imageBlitShaderProgram, 6, 4); // outputImage texture sampler2D
        glProgramUniform1ui(imageBlitShaderProgram, 17, (gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX) || currentLevel == 13);
        glProgramUniform1ui(imageBlitShaderProgram, 18, (gridCellStates[playerCellIdx] & CELL_SEES_SUN) && currentLevel != 13);
        glProgramUniform1ui(imageBlitShaderProgram, 19, ((currentLevel >= 10 && currentLevel < 13) ? 1u : 0u) && (gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX));
        uint32_t shieldOnType = 0u; // No shield green tint.
        if (questData.ShieldActivated) {
            if (currentLevel == 6 || currentLevel == 7) shieldOnType = 2u; // Shielding only below player for lower levels.
            else if (currentLevel <= 5) shieldOnType = 1u; // Shielding everywhere as levels fully within shield.
        }
        glProgramUniform1f(imageBlitShaderProgram, 4, worldMin_x);
        glProgramUniform1f(imageBlitShaderProgram, 5, worldMin_z);
        glProgramUniform1ui(imageBlitShaderProgram, 7, settings_Reflections);
        glProgramUniform1ui(imageBlitShaderProgram, 8, settings_AntiAliasing);
        glProgramUniform1f(imageBlitShaderProgram, 9, berserkTimeRemainingNormalized);
        glProgramUniform1f(imageBlitShaderProgram, 10, berserkSeedTime);
        glProgramUniform1ui(imageBlitShaderProgram, 11, settings_Brightness);
        glUniform3f(12, deg2rad(cam_yaw), deg2rad(cam_pitch), deg2rad(cam_roll));
        glProgramUniform3f(imageBlitShaderProgram, 13, instances[PLAYER1].position.x, instances[PLAYER1].position.y, instances[PLAYER1].position.z);
        glProgramUniform1f(imageBlitShaderProgram, 14, settings_FOV);
        glProgramUniform1f(imageBlitShaderProgram, 15, pauseRelativeTime * 0.1);
        glProgramUniform1f(imageBlitShaderProgram, 16, aspect3D);
        glProgramUniform1ui(imageBlitShaderProgram, 20, shieldOnType);
        glProgramUniform1ui(imageBlitShaderProgram, 22, settings_Shadows);
        glUniformMatrix4fv(24, 1, GL_FALSE, viewProj);
        glUniformMatrix3fv(25, 1, GL_FALSE, invViewRot);
        glProgramUniform1i(imageBlitShaderProgram, 27, 0); // Texture 0 for the rendered geometry color buffer
        float coloredStatic = 0.0f; // TODO: Hook into pain/health management and shield impact effect
        glProgramUniform1f(imageBlitShaderProgram, 28, coloredStatic);
        glProgramUniform3f(imageBlitShaderProgram, 29, 1.0f, 0.0f, 0.0f); // TODO: Hook staticColor up to red or blue for pain or shield impact.
        glBindVertexArray(quadVAO);
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        drawCallsRenderedThisFrame++;
        verticesRenderedThisFrame += 4;
        glBindTextureUnit(0, 0);

        // HUD
        // UI Common GL traits
        uint32_t drawCallsNormal = drawCallsRenderedThisFrame;

        // UI Common References
        float screenCenterX = (float)screen_width / 2;
        float screenCenterY = (float)screen_height / 2;
        float lineSpacing = GetScreenRelativeY(genericTextHeightFac);
        
        // 7. UI
        glEnable(GL_BLEND);
        glClear(GL_DEPTH_BUFFER_BIT); // Clear main FBO.  glClearBufferfv was actually SLOWER!  2nd Clear needed or UI dissappears/flickers!!
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE); // Fixes alpha rendering of text, but makes the z sort not work for some reason.
        glDisable(GL_CULL_FACE);
        
        //    Cursor
        uint16_t cursorTexture = 1260;
        if (gamePaused || menuActive) cursorTexture = 1261;
        float cursorSize = (float)screen_width * CURSOR_SCREEN_PERCENTAGE;
        float cursorHalfSize = cursorSize * 0.5f;
        if (CursorVisible()) AddUIImage(cursorPosition_x - cursorHalfSize, cursorPosition_y - cursorHalfSize, UI_LAYER_TOP, cursorSize, cursorSize, cursorTexture);
        else AddUIImage(screenCenterX - cursorHalfSize, screenCenterY - cursorHalfSize, UI_LAYER_TOP, cursorSize, cursorSize, cursorTexture);
        
        float shootModeWidth = GetScreenRelativeX(0.01639f), shootModeHeight = GetScreenRelativeX(0.01639f);
        float shootModePos_x = GetScreenRelativeX(0.5f) - (shootModeWidth * 0.5f);
        float shootModePos_y = 0.0f;
        if (!gamePaused) AddUIImage(shootModePos_x, shootModePos_y, UI_LAYER_0, shootModeWidth, shootModeHeight, 1020); // Shoot mode button
        if (inventoryMode) {
            if (CursorIsOverBounds(shootModePos_x, shootModePos_x + shootModeWidth, shootModePos_y + shootModeHeight, shootModePos_y)) {
                if (mouseButtons[GLFW_MOUSE_BUTTON_LEFT].released) {
                    inventoryMode = false;
                    cursorPosition_x = screen_width / 2;
                    cursorPosition_y = screen_height / 2;
                }
            }
        }
        
        if (gamePaused) {
            RenderFormattedText(screenCenterX - GetScreenRelativeX(genericTextWidthFacStopD * 3.0f), screenCenterY - GetScreenRelativeY(0.3f), UI_LAYER_5, TEXT_STOPD_RED_PAUSETITLE, FONT_STOPD, "PAUSED");
            char* pauseButton_ResumeText = "RESUME";
            float pauseButton_ResumeWidth = (TextWidth(pauseButton_ResumeText,FONT_STOPD) * 0.5f);
            float pauseButton_ResumeHeight = GetScreenRelativeY(genericTextHeightFacStopD);
            float pauseButton_ResumeX = screenCenterX - pauseButton_ResumeWidth;
            float pauseButton_ResumeY = screenCenterY - GetScreenRelativeY(0.08f);
            uint8_t pauseButton_ResumeColor = TEXT_STOPD_RED;
            bool pauseButton_CursorIsAbove = CursorIsOverBounds(pauseButton_ResumeX - GetScreenRelativeX(genericTextWidthFacStopD), pauseButton_ResumeX + pauseButton_ResumeWidth,
                                                                pauseButton_ResumeY + (pauseButton_ResumeHeight * 0.5f), pauseButton_ResumeY - (pauseButton_ResumeHeight * 0.5f));
            
            if (pauseButton_CursorIsAbove) pauseButton_ResumeColor = TEXT_STOPD_RED_HIGHLIGHT;
            RenderFormattedText(pauseButton_ResumeX, pauseButton_ResumeY, UI_LAYER_5, pauseButton_ResumeColor, FONT_STOPD, "RESUME");
            
            RenderFormattedText(screenCenterX - GetScreenRelativeX(genericTextWidthFacStopD * 2.0f), screenCenterY + GetScreenRelativeY(0.00f), UI_LAYER_5, TEXT_STOPD_RED, FONT_STOPD, "LOAD");
            RenderFormattedText(screenCenterX - GetScreenRelativeX(genericTextWidthFacStopD * 2.0f), screenCenterY + GetScreenRelativeY(0.08f), UI_LAYER_5, TEXT_STOPD_RED, FONT_STOPD, "SAVE");
            RenderFormattedText(screenCenterX - GetScreenRelativeX(genericTextWidthFacStopD * 3.5f), screenCenterY + GetScreenRelativeY(0.16f), UI_LAYER_5, TEXT_STOPD_RED, FONT_STOPD, "OPTIONS");
            RenderFormattedText(screenCenterX - GetScreenRelativeX(genericTextWidthFacStopD * 6.0f), screenCenterY + GetScreenRelativeY(0.24f), UI_LAYER_5, TEXT_STOPD_RED, FONT_STOPD, "QUIT TO MENU");
            RenderFormattedText(screenCenterX - GetScreenRelativeX(genericTextWidthFacStopD * 4.5f), screenCenterY + GetScreenRelativeY(0.40f), UI_LAYER_5, TEXT_STOPD_RED, FONT_STOPD, "QUIT GAME");
            float pauseBGWidth = GetScreenRelativeX(0.24f), pauseBGHeight = GetScreenRelativeY(0.39f);
            float pauseBGX = screenCenterX - (pauseBGWidth * 0.5f);
            float pauseBGY = screenCenterY - (pauseBGHeight * 0.5f) + GetScreenRelativeY(0.08f);
            AddUIImage(pauseBGX, pauseBGY, UI_LAYER_0, pauseBGWidth, pauseBGHeight, 1025); // Pause Menu background
            AddUIImage(pauseBGX, pauseBGY, UI_LAYER_1, pauseBGWidth, pauseBGHeight, 1080); // Pause Menu background
            float quitGame_Height = GetScreenRelativeY(0.05f);
            AddUIImage(pauseBGX, screenCenterY + GetScreenRelativeY(0.40f) - (quitGame_Height * 0.5f), UI_LAYER_0, pauseBGWidth, quitGame_Height, 950); // Pause Quit Game background
        }
        
        // Diagnostics / Debugging
        float debugTextStartY = GetScreenRelativeY(0.075f);
        float leftPad = GetScreenRelativeX(0.0125f);
        RenderFormattedText(leftPad, debugTextStartY, UI_LAYER_1, TEXT_WHITE, FONT_NORMAL, "x: %.4f, y: %.4f, z: %.4f", instances[PLAYER1].position.x, instances[PLAYER1].position.y, instances[PLAYER1].position.z);
        RenderFormattedText(leftPad, debugTextStartY + (lineSpacing * 1), UI_LAYER_1, TEXT_WHITE, FONT_NORMAL, "SSR step size: %.2f, step count: %u, sample weight: %.2f", settings_SSRStepSize, settings_SSRStepCount, settings_SSRSampleWeight);
        RenderFormattedText(leftPad, debugTextStartY + (lineSpacing * 2), UI_LAYER_1, TEXT_WHITE, FONT_NORMAL, "Player velocity: %.2f, %.2f, %.2f, accumulated force: %.2f, %.2f, %.2f", instances[PLAYER1].velocity.x, instances[PLAYER1].velocity.y, instances[PLAYER1].velocity.z, instances[PLAYER1].accumulatedForce.x, instances[PLAYER1].accumulatedForce.y, instances[PLAYER1].accumulatedForce.z);
        if (consoleActive) RenderFormattedText(leftPad, 0, UI_LAYER_1, TEXT_WHITE, FONT_NORMAL, "] %s",consoleEntryText);
        if (statusTextDecayFinished > current_time) RenderFormattedText(screenCenterX - (TextWidth(statusText,FONT_NORMAL) * 0.5f), screenCenterY - GetScreenRelativeY(0.30f + (genericTextHeightFac * 2.0f)), UI_LAYER_1, TEXT_WHITE, FONT_NORMAL, "%s",statusText);

        glDepthMask(GL_TRUE);
        RenderUIImages();    
        double time_now = get_time();
        drawCallsRenderedThisFrame++; textDrawCallsRenderedThisFrame++; // Add one more for this text render ;)
        drawCallsRenderedThisFrame++; textDrawCallsRenderedThisFrame++; // Add one more for this text render ;)
        double thisFrameTime = (time_now - last_time) * 1000.0f;
        double cpuFrameTime = cpuTime * 1000.0f;
        uint8_t timingColor = TEXT_WHITE;
        if (vabs(thisFrameTime - cpuFrameTime) < 0.451) timingColor = TEXT_ORANGE;
        if (thisFrameTime > 6.944444) timingColor = TEXT_RED;
        RenderFormattedText(leftPad, debugTextStartY - lineSpacing, UI_LAYER_5, timingColor, FONT_NORMAL, "ms: %.2f, CPU %.2f", thisFrameTime,cpuFrameTime);
        RenderFormattedText(leftPad + 230.0f, debugTextStartY - lineSpacing, UI_LAYER_5, TEXT_WHITE, FONT_NORMAL, "(FPS: %d, Worst: %d), Drwclls: %d [G %d UI %d Txt %d Shd %d] Vrts: %d",framesPerLastSecond,worstFPS,drawCallsRenderedThisFrame, drawCallsNormal, uiImageDrawCallsRenderedThisFrame, textDrawCallsRenderedThisFrame, shadowDrawCallsRenderedThisFrame, verticesRenderedThisFrame);
        last_time = time_now;
        if ((time_now - lastFrameSecCountTime) >= 1.00) {
            lastFrameSecCountTime = time_now;
            framesPerLastSecond = globalFrameNum - lastFrameSecCount;
            if (framesPerLastSecond < worstFPS && globalFrameNum > 2000) worstFPS = framesPerLastSecond; // After startup, keep track of worst framerate seen.
            lastFrameSecCount = globalFrameNum;
        }
        
        if (keyStates[GLFW_KEY_F12].pressed) {
            if (time_now > screenshotTimeout) {
                Screenshot();
                screenshotTimeout = time_now + 1.0; // Prevent saving more than 1 per second for sanity purposes.
            }
        }
        
        cpuTime = get_time() - current_time;
        glfwSwapBuffers(window); // Present frame
        CHECK_GL_ERROR();
        globalFrameNum++;
        #ifdef DEBUG_RAM_OUTPUT
            if (globalFrameNum == 4) { DebugRAM("after 4 frames of running"); malloc_trim(0); }
            else if (globalFrameNum == 100) { DebugRAM("after 100 frames of running"); }
            else if (globalFrameNum == 200) DebugRAM("after 200 frames of running");
            else if (globalFrameNum == 500) DebugRAM("after 500 frames of running");
            else if (globalFrameNum == 1000) DebugRAM("after 1000 frames of running");
        #endif
    }
    
    return 0;
}
