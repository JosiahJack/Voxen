// File: voxen.c
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
// TODO: Raycasts
// TODO: Voxel GI?
// TODO: Scripting engine for gameplay
// TODO: Save/Load system
#define VERSION_STRING "v0.7.2"
#include <malloc.h>
#include <string.h>
#include <sys/stat.h>
#include <math.h>
#include <stdlib.h>
#include "event.h"
#define VOXEN_ENGINE_IMPLEMENTATION
#include "voxen.h"
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
#include "citadel_playermovement.c"
#include "input.c"

typedef struct {
    float nx, ny, nz, d;
} FrustumPlane;

// ----------------------------------------------------------------------------
// Window
GLFWwindow *window;
bool inventoryMode = false;
uint16_t screen_width = 1366, screen_height = 768;
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
// ----------------------------------------------------------------------------
// Settings
uint8_t settings_Reflections = 1u; // Default 1
uint8_t settings_Shadows = 2u; // Default 2 (1 is hard shadows, 2 enables Pseudo-Stochastic PCF sampling softening
uint8_t settings_AntiAliasing = 1u; // Default 1
uint8_t settings_Brightness = 100u; // Default 100 (for %)
uint8_t settings_VolumeMusic = 20u;
bool settings_Vsync = false;
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
bool levelCurrentlyLoading = false;
float pauseRelativeTime = 0.0f;
QuestBits questData;
// ----------------------------------------------------------------------------
// Camera variables
// Start Actual: Puts player on Medical Level in actual game start position
float cam_yaw = 90.0f;
float cam_pitch = 0.0f;
float cam_roll = 0.0f;
Quaternion cam_rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
float cam_forwardx = 0.0f, cam_forwardy = 0.0f, cam_forwardz = 0.0f;
float cam_rightx = 0.0f, cam_righty = 0.0f, cam_rightz = 0.0f;
float cam_fov = 65.0f;
float berserkFinished = 0.0f;
float berserkSeedTime = 0.0f;
// ----------------------------------------------------------------------------
// OpenGL / Rendering
int32_t debugView = 0;
int32_t debugValue = 0;
float aspect3D = 1.0f;
float aspect2D = 1.0f;
float rasterPerspectiveProjection[16];
float shadowmapsPerspectiveProjection[16];
uint32_t drawCallsRenderedThisFrame = 0; // Total draw calls this frame
uint32_t textDrawCallsRenderedThisFrame = 0;
uint32_t uiImageDrawCallsRenderedThisFrame = 0;
uint32_t shadowDrawCallsRenderedThisFrame = 0;
uint32_t verticesRenderedThisFrame = 0;
bool instanceIsLODArray[INSTANCE_COUNT];
GLuint inputImageID, inputDepthID, inputWorldPosID, gBufferFBO, outputImageID; // FBO
// ----------------------------------------------------------------------------
// Shaders
//    Chunk Geometery Unlit Raster Shader
GLuint chunkShaderProgram;
GLuint vao_chunk; // Vertex Array Object
GLint viewProjLoc_chunk, matrixLoc_chunk, texIndexLoc_chunk, debugViewLoc_chunk, debugValueLoc_chunk, glowSpecIndexLoc_chunk, normInstanceIndexLoc_chunk, screenWidthLoc_chunk, screenHeightLoc_chunk, 
      worldMin_xLoc_chunk, worldMin_zLoc_chunk, camPosLoc_chunk, fogColorRLoc_chunk, fogColorGLoc_chunk, fogColorBLoc_chunk, shadowmapSizeLoc_chunk, reflectionsEnabledLoc_chunk, shadowsEnabledLoc_chunk,
      isUILoc_chunk, unlitLoc_chunk;
      
float fogColorR, fogColorG, fogColorB, fogColorRUsed, fogColorGUsed, fogColorBUsed, fogBaseDensityForLevel;

//    Shadowmap Rastered Depth Shader
GLuint shadowCubeMap;
GLuint shadowFBO;
GLuint shadowmapsShaderProgram;
GLint modelMatrixLoc_shadowmaps, viewProjMatrixLoc_shadowmaps, texIndexLoc_shadowmaps, glowSpecIndexLoc_shadowmaps, normInstanceIndexLoc_shadowmaps, lightPosLoc_shadowmaps, ssbo_indexBaseLoc_shadowmaps,
      shadowmapSizeLoc_shadowmaps, viewProjArrayLoc_shadowmaps;
GLuint shadowMapSSBO; // SSBO for storing all shadow maps
uint32_t totalShadowmapPixels = 0;
uint32_t shadSizeSquared = SHADOW_MAP_SIZE * SHADOW_MAP_SIZE;

//    SSR (Screen Space Reflections)
#define SSR_RES 4 // 25% of render resolution.
GLuint ssrShaderProgram;
GLint screenWidthLoc_ssr, screenHeightLoc_ssr, viewProjectionLoc_ssr, camPosLoc_ssr, outputImageLoc_ssr;

//    Shadowmaps Clear
GLuint shadowmapsClearShaderProgram;

//    Full Screen Quad Blit for rendering final output/image effect passes
GLuint imageBlitShaderProgram;
GLuint quadVAO, quadVBO;
GLint texLoc_quadblit, debugViewLoc_quadblit, debugValueLoc_quadblit, screenWidthLoc_imageBlit, screenHeightLoc_imageBlit, outputImageLoc_imageBlit, skyVisibleLoc_imageBlit, planetaryBodiesVisibleLoc_imageBlit,
      groveShieldVisibleLoc_imageBlit, stationShieldVisibleLoc_imageBlit, reflectionsEnabledLoc_imageBlit, aaEnabledLoc_imageBlit, brightnessSettingLoc_imageBlit, fovLoc_imageBlit, camRotLoc_imageBlit, timeValLoc_imageBlit,
      aspectLoc_imageBlit, shadowsSettingLoc_imageBlit, shadowmapSizeLoc_imageBlit, worldMin_xLoc_imageBlit, worldMin_zLoc_imageBlit, viewProjectionLoc_imageBlit, camPosLoc_imageBlit, invViewRotLoc_imageBlit,
      berserkTimeRemainingLoc_imageBlit, berserkSeedTimestampLoc_imageBlit;
      
//    Text Shader
GLuint textShaderProgram;
GLuint textVAO, textVBO;
GLint projectionLoc_text, textColorLoc_text, textTextureLoc_text, texelSizeLoc_text, fontTypeLoc_text;

// ----------------------------------------------------------------------------
// UI Cursor
bool cursorVisible = false;
int32_t cursorPosition_x = 680, cursorPosition_y = 384;
// ----------------------------------------------------------------------------
// UI
//    Images
#define MAX_UI_IMAGES 1024 // Adjust based on needs

typedef struct {
    float x, y, z;        // Top-left corner in screen space (pixels)
    float width, height; // Size in screen space (pixels)
    uint32_t texIndex; // Index into textureOffsets for palettized texture
    bool visible;      // Whether to render this image
} UIImage;

UIImage uiImages[MAX_UI_IMAGES];
uint32_t uiImageCount = 0;
GLuint uiImageVAO, uiImageVBO;

//    Text
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

//      Console Emulator
int32_t currentEntryLength = 0;
bool consoleActive = false;
char consoleEntryText[TEXT_BUFFER_SIZE] = "Enter a command...";
char statusText[TEXT_BUFFER_SIZE];

//      Center Status Print
int statusTextLengthWithoutNullTerminator = 6;
float statusTextDecayFinished = 0.0f;
// ----------------------------------------------------------------------------
// Lights
// Could reduce spotAng to minimal bits.  I only have 6 spot lights and half are 151.7 and other half are 135.
GLuint lightsID, voxelLightListIndicesID, voxelLightListsRawID, lightShadowsEnabledID, shadowMapsIndirectionID;
uint32_t* voxelLightListsRaw = NULL;
uint32_t* voxelLightListIndices = NULL;
uint32_t* shadowmapIndirectionList = NULL;
uint32_t numDynamicLights;
float lights[LIGHT_COUNT * LIGHT_DATA_SIZE] = {0};
float lightsRangeSquared[LIGHT_COUNT] = {0.0f};
bool lightDirty[LIGHT_COUNT] = { [0 ... LIGHT_COUNT-1] = true };
float*** lightViewProj = NULL; // Array of Array of 6 Arrays of 16 floats (matrix 4x4).  lightViewProj[i][face][0 ... 15]
float*** lightView = NULL; // Array of Array of 6 Arrays of 16 floats (matrix 4x4).  lightView[i][face][0 ... 15]
FrustumPlane*** lightFrustumPlanes = NULL; // Array of Array of 6 Arrays of FrustumPlane structs (four floats).  lightFrustumPlanes[i][face][.nx,.ny,, .nz, .d]
// ----------------------------------------------------------------------------
// ============================================================================
// OpenGL / Rendering Helper Functions
void GenerateAndBindTexture(GLuint *id, GLenum internalFormat, int32_t width, int32_t height, GLenum format, GLenum type, GLenum target) {
    glGenTextures(1, id);
    glBindTexture(target, *id);
    glTexImage2D(target, 0, internalFormat, width, height, 0, format, type, NULL);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(target, 0);
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
    
    // Shadowmaps Clear Compute Shader Program
    computeShader = CompileShader(GL_COMPUTE_SHADER, shadowmaps_clear_computeShader, "Shadowmaps Clear Compute Shader");
    shadowmapsClearShaderProgram = LinkProgram((GLuint[]){computeShader}, 1, "Shadowmaps Clear Shader Program");

    // Image Blit Shader (For full screen image effects, rendering compute results, etc.)
    vertShader = CompileShader(GL_VERTEX_SHADER,   quadVertexShaderSource,   "Image Blit Vertex Shader");
    fragShader = CompileShader(GL_FRAGMENT_SHADER, quadFragmentShaderSource, "Image Blit Fragment Shader");
    imageBlitShaderProgram = LinkProgram((GLuint[]){vertShader, fragShader}, 2, "Image Blit Shader Program");

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
    shadowmapSizeLoc_chunk = glGetUniformLocation(chunkShaderProgram, "shadowmapSize");
    reflectionsEnabledLoc_chunk = glGetUniformLocation(chunkShaderProgram, "reflectionsEnabled");
    shadowsEnabledLoc_chunk = glGetUniformLocation(chunkShaderProgram, "shadowsEnabled");
    isUILoc_chunk = glGetUniformLocation(chunkShaderProgram, "isUI");
    unlitLoc_chunk = glGetUniformLocation(chunkShaderProgram, "unlit");
    
    modelMatrixLoc_shadowmaps = glGetUniformLocation(shadowmapsShaderProgram, "modelMatrix");
    viewProjMatrixLoc_shadowmaps = glGetUniformLocation(shadowmapsShaderProgram, "viewProjMatrix");
    texIndexLoc_shadowmaps = glGetUniformLocation(shadowmapsShaderProgram, "texIndex");
    glowSpecIndexLoc_shadowmaps = glGetUniformLocation(shadowmapsShaderProgram, "glowSpecIndex");
    normInstanceIndexLoc_shadowmaps = glGetUniformLocation(shadowmapsShaderProgram, "normInstanceIndex");
    lightPosLoc_shadowmaps = glGetUniformLocation(shadowmapsShaderProgram, "lightPos");
    ssbo_indexBaseLoc_shadowmaps = glGetUniformLocation(shadowmapsShaderProgram, "ssbo_indexBase");
    shadowmapSizeLoc_shadowmaps = glGetUniformLocation(shadowmapsShaderProgram, "shadowmapSize");

    screenWidthLoc_ssr = glGetUniformLocation(ssrShaderProgram, "screenWidth");
    screenHeightLoc_ssr = glGetUniformLocation(ssrShaderProgram, "screenHeight");
    viewProjectionLoc_ssr = glGetUniformLocation(ssrShaderProgram, "viewProjection");
    camPosLoc_ssr = glGetUniformLocation(ssrShaderProgram, "camPos");
    outputImageLoc_ssr = glGetUniformLocation(ssrShaderProgram, "outputImage");
    
    texLoc_quadblit = glGetUniformLocation(imageBlitShaderProgram, "tex");
    debugViewLoc_quadblit = glGetUniformLocation(imageBlitShaderProgram, "debugView");
    debugValueLoc_quadblit = glGetUniformLocation(imageBlitShaderProgram, "debugValue");
    screenWidthLoc_imageBlit = glGetUniformLocation(imageBlitShaderProgram, "screenWidth");
    screenHeightLoc_imageBlit = glGetUniformLocation(imageBlitShaderProgram, "screenHeight");
    outputImageLoc_imageBlit = glGetUniformLocation(imageBlitShaderProgram, "outputImage");
    skyVisibleLoc_imageBlit = glGetUniformLocation(imageBlitShaderProgram, "skyVisible");
    planetaryBodiesVisibleLoc_imageBlit = glGetUniformLocation(imageBlitShaderProgram, "planetaryBodiesVisible");
    groveShieldVisibleLoc_imageBlit = glGetUniformLocation(imageBlitShaderProgram, "groveShieldVisible");
    stationShieldVisibleLoc_imageBlit = glGetUniformLocation(imageBlitShaderProgram, "stationShieldVisible");
    reflectionsEnabledLoc_imageBlit = glGetUniformLocation(imageBlitShaderProgram, "reflectionsEnabled");
    aaEnabledLoc_imageBlit = glGetUniformLocation(imageBlitShaderProgram, "aaEnabled");
    brightnessSettingLoc_imageBlit = glGetUniformLocation(imageBlitShaderProgram, "brightnessSetting");
    fovLoc_imageBlit = glGetUniformLocation(imageBlitShaderProgram, "fov");
    camRotLoc_imageBlit = glGetUniformLocation(imageBlitShaderProgram, "camRot");
    timeValLoc_imageBlit = glGetUniformLocation(imageBlitShaderProgram, "timeVal");
    aspectLoc_imageBlit = glGetUniformLocation(imageBlitShaderProgram, "aspect");
    shadowsSettingLoc_imageBlit = glGetUniformLocation(imageBlitShaderProgram, "shadowsEnabled");
    shadowmapSizeLoc_imageBlit = glGetUniformLocation(imageBlitShaderProgram, "shadowmapSize");
    worldMin_xLoc_imageBlit = glGetUniformLocation(imageBlitShaderProgram, "worldMin_x");
    worldMin_zLoc_imageBlit = glGetUniformLocation(imageBlitShaderProgram, "worldMin_z");
    viewProjectionLoc_imageBlit = glGetUniformLocation(imageBlitShaderProgram, "viewProjection");
    camPosLoc_imageBlit = glGetUniformLocation(imageBlitShaderProgram, "camPos");
    invViewRotLoc_imageBlit = glGetUniformLocation(imageBlitShaderProgram, "invViewRot");
    berserkTimeRemainingLoc_imageBlit = glGetUniformLocation(imageBlitShaderProgram, "berserkTimeRemaining");
    berserkSeedTimestampLoc_imageBlit = glGetUniformLocation(imageBlitShaderProgram, "berserkSeedTimestamp");
    
    projectionLoc_text = glGetUniformLocation(textShaderProgram, "projection");
    textColorLoc_text = glGetUniformLocation(textShaderProgram, "textColor");
    textTextureLoc_text = glGetUniformLocation(textShaderProgram, "textTexture");
    texelSizeLoc_text = glGetUniformLocation(textShaderProgram, "texelSize");
    fontTypeLoc_text = glGetUniformLocation(textShaderProgram, "fontType");
    CHECK_GL_ERROR();
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
// ============================================================================
void UpdateScreenSize(void) {
    float* m;
    m = uiOrthoProjection;
    m[0] = 2.0f / (float)screen_width; m[1] =                           0.0f; m[2] =  0.0f; m[3] = 0.0f;
    m[4] =                       0.0f; m[5] = -2.0f / ((float)screen_height); m[6] =  0.0f; m[7] = 0.0f;
    m[8] =                       0.0f; m[9] =                           0.0f; m[10]= -1.0f; m[11]= 0.0f;
    m[12]=                      -1.0f; m[13]=                           1.0f; m[14]=  0.0f; m[15]= 1.0f;
    
    aspect3D = (float)screen_width / (float)screen_height;
    float f = 1.0f / tan(cam_fov * M_PI / 360.0f);
    m = rasterPerspectiveProjection;
    m[0] = f / aspect3D; m[1] = 0.0f; m[2] =                                                      0.0f; m[3] =  0.0f;
    m[4] =         0.0f; m[5] =    f; m[6] =                                                      0.0f; m[7] =  0.0f;
    m[8] =         0.0f; m[9] = 0.0f; m[10]=      -(FAR_PLANE + NEAR_PLANE) / (FAR_PLANE - NEAR_PLANE); m[11]= -1.0f;
    m[12]=         0.0f; m[13]= 0.0f; m[14]= -2.0f * FAR_PLANE * NEAR_PLANE / (FAR_PLANE - NEAR_PLANE); m[15]=  0.0f;
    
    aspect2D = (float)SHADOW_MAP_SIZE / (float)SHADOW_MAP_SIZE;
    f = 1.0f / tan(SHADOWMAP_FOV * M_PI / 360.0f);
    m = shadowmapsPerspectiveProjection;
    m[0] = f / aspect2D; m[1] = 0.0f; m[2] =                                            0.0f; m[3] =  0.0f;
    m[4] =         0.0f; m[5] =    f; m[6] =                                            0.0f; m[7] =  0.0f;
    m[8] =         0.0f; m[9] = 0.0f; m[10]=      -(35.0 + NEAR_PLANE) / (35.0 - NEAR_PLANE); m[11]= -1.0f;
    m[12]=         0.0f; m[13]= 0.0f; m[14]= -2.0f * 35.0 * NEAR_PLANE / (35.0 - NEAR_PLANE); m[15]=  0.0f;
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

Quaternion cubemapOrientationQuaternion[6] = {
    {0.0f, 0.707106781f, 0.0f, 0.707106781f},  // +X: Right
    {0.0f, -0.707106781f, 0.0f, 0.707106781f}, // -X: Left
    {-0.707106781f, 0.0f, 0.0f, 0.707106781f}, // +Y: Up
    {0.707106781f, 0.0f, 0.0f, 0.707106781f},  // -Y: Down
    {0.0f, 0.0f, 0.0f, 1.0f},                  // +Z: Forward
    {0.0f, 1.0f, 0.0f, 0.0f}                   // -Z: Backward
};

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

bool IsSphereInFOVCone(float inst_x, float inst_y, float inst_z) {
    // Vector from camera to instance
    float to_inst_x = inst_x - instances[PLAYER1].position.x;
    float to_inst_y = inst_y - instances[PLAYER1].position.y;
    float to_inst_z = inst_z - instances[PLAYER1].position.z;
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
        float len = sqrtf(planes[i].nx*planes[i].nx + planes[i].ny*planes[i].ny + planes[i].nz*planes[i].nz);
        if (len > 0.0f) {
            planes[i].nx /= len; planes[i].ny /= len; planes[i].nz /= len; planes[i].d /= len; // Normalize
        }
    }
}

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
        int32_t maxCellX = (int32_t)ceilf((litX + range - worldMin_x) * cellWidthRecip);
        int32_t minCellZ = (int32_t)((litZ - range - worldMin_z) * cellWidthRecip);
        int32_t maxCellZ = (int32_t)ceilf((litZ + range - worldMin_z) * cellWidthRecip);
        minCellX = minCellX > 0 ? minCellX : 0;
        maxCellX = 63 < maxCellX ? 63 : maxCellX;
        minCellZ = minCellZ > 0 ? minCellZ : 0;
        maxCellZ = 63 < maxCellZ ? 63 : maxCellZ;
        for (int32_t cellZ = minCellZ; cellZ <= maxCellZ; ++cellZ) {
            for (int32_t cellX = minCellX; cellX <= maxCellX; ++cellX) {
                uint32_t cellIndex = cellZ * 64 + cellX;
//                 if (!(gridCellStates[cellIndex] & CELL_OPEN)) continue; // TODO should be able to do this somehow without it getting truncated and leaving half the cell black.
                
                for (uint32_t voxelZ = 0; voxelZ < 8; ++voxelZ) {
                    for (uint32_t voxelX = 0; voxelX < 8; ++voxelX) {
                        uint32_t voxelIndex = cellIndex * 64 + voxelZ * 8 + voxelX;
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
            int r = floor(range * 0.390625f);
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
        int32_t maxCellX = (int32_t)ceilf((litX + range - worldMin_x) * cellWidthRecip);
        int32_t minCellZ = (int32_t)((litZ - range - worldMin_z) * cellWidthRecip); // cast to int truncates, no floorf
        int32_t maxCellZ = (int32_t)ceilf((litZ + range - worldMin_z) * cellWidthRecip);
        minCellX = minCellX > 0 ? minCellX : 0;
        maxCellX = 63 < maxCellX ? 63 : maxCellX;
        minCellZ = minCellZ > 0 ? minCellZ : 0;
        maxCellZ = 63 < maxCellZ ? 63 : maxCellZ;
        for (int32_t cellZ = minCellZ; cellZ <= maxCellZ; ++cellZ) {
            for (int32_t cellX = minCellX; cellX <= maxCellX; ++cellX) {
                uint32_t cellIndex = cellZ * 64 + cellX;
//                 if (!(gridCellStates[cellIndex] & CELL_OPEN)) continue;

                for (uint32_t voxelZ = 0; voxelZ < 8; ++voxelZ) {
                    for (uint32_t voxelX = 0; voxelX < 8; ++voxelX) {
                        uint32_t voxelIndex = cellIndex * 64 + voxelZ * 8 + voxelX;
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
        for (int j=0;j<6;++j) {
            mat4_lookat_from(lightView[i][j], &cubemapOrientationQuaternion[j], litX, litY, litZ);
            mul_mat4(lightViewProj[i][j], shadowmapsPerspectiveProjection, lightView[i][j]);
            ExtractFrustumPlanes(lightViewProj[i][j], lightFrustumPlanes[i][j]);
        }
    }
    
    glNamedBufferData(voxelLightListIndicesID, VOXEL_COUNT * 2 * sizeof(uint32_t), voxelLightListIndices, GL_DYNAMIC_DRAW);
    glNamedBufferData(voxelLightListsRawID, head * sizeof(uint32_t), voxelLightListsRaw, GL_DYNAMIC_DRAW);
    glNamedBufferData(lightsID,loadedLights * LIGHT_DATA_SIZE * sizeof(float), lights, GL_DYNAMIC_DRAW);
}

uint32_t* lightShadowsEnabled = NULL;

void VoxelLists() {
    voxelLightListsRaw = malloc(VOXEL_COUNT * 4 * sizeof(uint32_t));
    voxelLightListIndices = malloc(VOXEL_COUNT * 2 * sizeof(uint32_t));
    voxelLightListIndicesID = SetupSSBO(voxelLightListIndicesID, 26, VOXEL_COUNT * 2 * sizeof(uint32_t), NULL, GL_DYNAMIC_DRAW);
    voxelLightListsRawID = SetupSSBO(voxelLightListsRawID, 27,  1008105 * sizeof(uint32_t), NULL, GL_DYNAMIC_DRAW);
    shadowmapIndirectionList = malloc(loadedLights * sizeof(uint32_t));
    lightsID = SetupSSBO(lightsID, 19, loadedLights * LIGHT_DATA_SIZE * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    lightView = malloc(loadedLights * sizeof(float**));
    lightViewProj = malloc(loadedLights * sizeof(float**));
    lightFrustumPlanes = malloc(loadedLights * sizeof(FrustumPlane**));
    for (int i=0;i<loadedLights;++i) {
        lightView[i] = malloc(6 * sizeof(float*));
        lightViewProj[i] = malloc(6 * sizeof(float*));
        lightFrustumPlanes[i] = malloc(6 * sizeof(FrustumPlane*));
        for (int j=0;j<6;++j) {
            lightView[i][j] = malloc(4 * 4 * sizeof(float)); // Matrix 4x4 for this cubemap face
            lightViewProj[i][j] = malloc(4 * 4 * sizeof(float)); // Matrix 4x4 for this cubemap face
            lightFrustumPlanes[i][j] = malloc(6 * sizeof(FrustumPlane)); // Frustum Planes for this cubemap face
        }
    }
    UpdateVoxelLightLists();
    for (uint16_t i = 3; i < loadedInstances; i++) UpdateInstanceMatrix(i); // Skip player indices and start at 3
    matricesBuffer = SetupSSBO(matricesBuffer, 11, loadedInstances * 16 * sizeof(float), modelMatrices, GL_DYNAMIC_DRAW);
    lightShadowsEnabled = malloc(loadedLights * sizeof(uint32_t));
    memset(lightShadowsEnabled,0u,loadedLights * sizeof(uint32_t));
    uint16_t numLightsWithShadows = 0;
    for (int i=0;i<loadedLights;++i) {
//         uint32_t litIdx = i * LIGHT_DATA_SIZE;
//         float lightRadius = lights[litIdx + LIGHT_DATA_OFFSET_RANGE];
//         float effectiveRadius = fmin(lightRadius, 15.36f);
//         float litIntensity = lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY];
//         float luminosity = (litIntensity / (effectiveRadius * effectiveRadius));
//         float thresh = 0.006f;//0.042f;
// //         if (currentLevel >= 10) thresh += 0.015f; // TODO retweak with Voxen
// //         if (currentLevel == 7 || currentLevel == 0 || currentLevel == 8) thresh += 0.0051f; // TODO retweak with Voxen
// //         if (currentLevel == 8) thresh += 0.005f; // TODO retweak with Voxen
//         if (luminosity < thresh) continue; // Skip if light is off
        
        lightShadowsEnabled[i] = 1u;
        numLightsWithShadows++;
    }
    
    lightShadowsEnabledID = SetupSSBO(lightShadowsEnabledID, 6, loadedLights * sizeof(uint32_t), lightShadowsEnabled, GL_STATIC_DRAW);
    shadowMapsIndirectionID = SetupSSBO(shadowMapsIndirectionID, 8, loadedLights * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
    DualLog("Number of lights with shadows: %u\n",numLightsWithShadows);
}

void RenderShadowmap(uint16_t lightIdx) {
    if (lightShadowsEnabled[lightIdx] == 0u) return;
    
    uint32_t litIdx = lightIdx * LIGHT_DATA_SIZE;
    float lightPosX = lights[litIdx + LIGHT_DATA_OFFSET_POSX];
    float lightPosY = lights[litIdx + LIGHT_DATA_OFFSET_POSY];
    float lightPosZ = lights[litIdx + LIGHT_DATA_OFFSET_POSZ];
    float lightRadius = lights[litIdx + LIGHT_DATA_OFFSET_RANGE];
    float effectiveRadius = fmin(lightRadius, 15.36f);
    float distSqrd = squareDistance3D(instances[PLAYER1].position.x, instances[PLAYER1].position.y, instances[PLAYER1].position.z, lightPosX, lightPosY, lightPosZ);
    if (distSqrd >= FAR_PLANE_SQUARED) return;

    int lightCellIdx = cellIndexForLight[lightIdx];
    bool inPVS = false;
    if ((gridCellStates[lightCellIdx] & CELL_VISIBLE)) {// || !(gridCellStates[lightCellIdx] & CELL_OPEN)) {
        inPVS = true; // Allow lights outside windows (and thus in non open cells) to still be applicable.
    } else { // Check cells that aren't visible but whose lights can light up cells that are visible.
        int x = cellIndexForLightX[lightIdx];
        int y = cellIndexForLightZ[lightIdx];
        int range = floor(lightRadius * 0.390625f); // 1 / 2.56f
        int xMin = x - range; int xMax = x + range;
        int yMin = y - range; int yMax = y + range;
        for (int ix = xMin;ix <= xMax; ix++) {
            for (int iy = yMin;iy <= yMax; iy++) {
                if (!XZPairInBounds(ix,iy)) continue;

                int subIdx = (iy * WORLDX) + ix;
                int cellIdx = (lightCellIdx * ARRSIZE);
                int flat_idx = cellIdx + subIdx;
                if ((gridCellStates[subIdx] & CELL_VISIBLE) // Player can see cell in light's range.
                    && get_cull_bit(precomputedVisibleCellsFromHere,flat_idx)) { // Light's cell can see the cell in light's range.
                    
                    inPVS = true;
                    goto Label_PVSCheck; // Avoid checking any more.  One is enough to count.
                }
            }
            
        }
    }
    
    Label_PVSCheck:    
    if (!inPVS) return;

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

    glUniform3f(lightPosLoc_shadowmaps, lightPosX, lightPosY, lightPosZ);
    for (uint8_t face = 0; face < 6; face++) {
        glUniform1i(ssbo_indexBaseLoc_shadowmaps, (shadowmapIndirectionList[lightIdx] * (6 * shadSizeSquared)) + (face * shadSizeSquared));
        glUniformMatrix4fv(viewProjMatrixLoc_shadowmaps, 1, GL_FALSE, lightViewProj[lightIdx][face]);
        for (uint16_t j = 0; j < nearbyMeshCount; ++j) {
            int i = nearMeshes[j];
            if (instances[i].modelIndex >= loadedModels) continue;
            if (modelVertexCounts[instances[i].modelIndex] < 1) continue; // Empty model
            
            float radius = modelBounds[(instances[i].modelIndex * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_RADIUS] * 1.42f;
            if (!SphereInFrustum(lightFrustumPlanes[lightIdx][face], instances[i].position.x, instances[i].position.y, instances[i].position.z, radius)) continue;

            int32_t modelType = instanceIsLODArray[i] && instances[i].lodIndex < loadedModels ? instances[i].lodIndex : instances[i].modelIndex;
            glUniformMatrix4fv(modelMatrixLoc_shadowmaps, 1, GL_FALSE, &modelMatrices[i * 16]);
            glBindVertexBuffer(0, vbos[modelType], 0, VERTEX_ATTRIBUTES_COUNT * sizeof(float));
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tbos[modelType]);
            glDrawElements(GL_TRIANGLES, modelTriangleCounts[modelType] * 3, GL_UNSIGNED_INT, 0);
            drawCallsRenderedThisFrame++;
            verticesRenderedThisFrame += modelTriangleCounts[modelType] * 3;
        }
    }
}

void RenderShadowmaps(void) {
    if (settings_Shadows < 1u) return;

    glUseProgram(shadowmapsClearShaderProgram);
    GLuint groupX_shadClear = (totalShadowmapPixels + 31) / 32;
    glDispatchCompute(groupX_shadClear,1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    
    shadowDrawCallsRenderedThisFrame = 0;
    memset(shadowmapIndirectionList, MAX_SHADOWMAPS + 1, loadedLights * sizeof(uint32_t));
    
    glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
    glUseProgram(shadowmapsShaderProgram);
    glProgramUniform1i(shadowmapsShaderProgram, shadowmapSizeLoc_shadowmaps, (int32_t)SHADOW_MAP_SIZE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    glBindVertexArray(vao_chunk);

    // Alternate form to find all lights without actual sorting:
    // first for over lights to get distance to player once
    // set short threshold of like 2.56 and use that first in one for loop.
    // another for loop, keeping head of the found lights but starting over at the beginning of the list and checking against a wider dist
    // another for loop, same as above but a little wider dist
    // another for loop, same as above but a little wider dist
    
    // This method is 0.027ms so not too worried about it at the moment.
//     double sortStart = get_time();
    // Collect candidates: only lights that are enabled, within FAR_PLANE, and in PVS
    LightCandidate candidates[loadedLights];
    uint32_t candidateCount = 0;
    for (uint16_t i = 0; i < loadedLights; ++i) {
        if (lightShadowsEnabled[i] == 0u) continue;

        uint32_t litIdx = i * LIGHT_DATA_SIZE;
        float lightPosX = lights[litIdx + LIGHT_DATA_OFFSET_POSX];
        float lightPosY = lights[litIdx + LIGHT_DATA_OFFSET_POSY];
        float lightPosZ = lights[litIdx + LIGHT_DATA_OFFSET_POSZ];
        float distSqrd = squareDistance3D(instances[PLAYER1].position.x, instances[PLAYER1].position.y, instances[PLAYER1].position.z, lightPosX, lightPosY, lightPosZ);
        if (distSqrd >= FAR_PLANE_SQUARED) continue;

        // Your inPVS check
        int lightCellIdx = cellIndexForLight[i];
        bool inPVS = (gridCellStates[lightCellIdx] & CELL_VISIBLE);
        if (!inPVS) {
            int x = cellIndexForLightX[i];
            int z = cellIndexForLightZ[i];
            float range = lights[litIdx + LIGHT_DATA_OFFSET_RANGE];
            int r = floor(range * 0.390625f);
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

        // Score: lower score = closer and brighter (higher priority)
        float intensity = lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY];
        float score = distSqrd / fmax(intensity, 0.01f);  // Avoid div by 0, favor bright lights

        candidates[candidateCount++] = (LightCandidate){ .index = i, .distanceSquared = distSqrd, .score = score };
    }

    // Sort candidates by score (ascending: best first)
    qsort(candidates, candidateCount, sizeof(LightCandidate), compareLightCandidates);

//     DualLog("Sorting time for lights: %f\n",get_time() - sortStart);
    // Render top MAX_SHADOWMAPS candidates
    uint32_t numToRender = fmin(candidateCount, MAX_SHADOWMAPS);
    for (uint32_t c = 0; c < numToRender; ++c) {
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

float uiImageVertexData[4096]; // Reusable buffer, adjust size as needed
void RenderUIImages() {
    if (uiImageCount == 0) return;

    glUseProgram(chunkShaderProgram);
    glBindVertexArray(uiImageVAO);
    glProgramUniform1ui(chunkShaderProgram, isUILoc_chunk, 1u);
    glProgramUniform1ui(chunkShaderProgram, unlitLoc_chunk, 1u);
    glProgramUniformMatrix4fv(chunkShaderProgram, viewProjLoc_chunk, 1, GL_FALSE, uiOrthoProjection);

    // Sort images by texIndex to minimize state changes.  Simple bubble sort for small N
    for (uint32_t i = 0; i < uiImageCount; i++) {
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
            float vertices[30] = {
                x0, y1, z0, 0.0f, 0.0f,
                x1, y0, z0, 1.0f, 1.0f,
                x1, y1, z0, 1.0f, 0.0f,
                x0, y1, z0, 0.0f, 0.0f,
                x0, y0, z0, 0.0f, 1.0f,
                x1, y0, z0, 1.0f, 1.0f
            };

            memcpy(uiImageVertexData + vertexCount * 30, vertices, sizeof(vertices));
            vertexCount++;
        }

        if (vertexCount > 0) {
            glUniform1ui(texIndexLoc_chunk, currentTex);
            glUniform1ui(glowSpecIndexLoc_chunk,     BLACK_TEXTURE_IDX);
            glUniform1ui(normInstanceIndexLoc_chunk, BLACK_TEXTURE_IDX);
            glUniformMatrix4fv(matrixLoc_chunk, 1, GL_FALSE, (float[16]){1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1});
            glNamedBufferData(uiImageVBO, vertexCount * 30 * sizeof(float), uiImageVertexData, GL_DYNAMIC_DRAW);
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

void ToggleConsole(void) {
    static bool inventoryModeWasActivePriorToConsole = false;
    if (!consoleActive) inventoryModeWasActivePriorToConsole = inventoryMode;
    consoleActive = !consoleActive; // Tilde
    if (consoleActive) inventoryMode = true;
    else if (!inventoryModeWasActivePriorToConsole && inventoryMode) {
        inventoryMode = false;
        cursorPosition_x = (float)screen_width * 0.5f;
        cursorPosition_y = (float)screen_height * 0.5f;
    }
}

void ProcessConsoleCommand(const char* command) {
    if (strcmp(command, "noclip") == 0) {
        noclip = !noclip;
        CenterStatusPrint("Noclip %s", noclip ? "enabled" : "disabled");
        ToggleConsole();
    }  else if (strcmp(command, "quit") == 0) {
        EnqueueEvent_Simple(EV_QUIT);
    } else {
        CenterStatusPrint("Unknown command: %s", command);
    }
    
    consoleEntryText[0] = '\0'; // Clear the input
    currentEntryLength = 0;
}

void ConsoleEmulator(int32_t keycode) {
    if (keycode == GLFW_KEY_U && keys[GLFW_KEY_LEFT_CONTROL]) {
        consoleEntryText[0] = '\0'; // Clear the input
        currentEntryLength = 0;
        return;
    }
    
    if (keycode >= GLFW_KEY_A && keycode <= GLFW_KEY_Z) { // Handle alphabet keys
        if (currentEntryLength < (TEXT_BUFFER_SIZE - 1)) { // Ensure we don't overflow the buffer
            char c = 'a' + (keycode - GLFW_KEY_A); // Map keycode to lowercase character
            consoleEntryText[currentEntryLength] = c;
            consoleEntryText[currentEntryLength + 1] = '\0'; // Null-terminate
            currentEntryLength++;
        }
    } else if (keycode >= GLFW_KEY_1 && keycode <= GLFW_KEY_9) { // Handle number keys 1-9
        if (currentEntryLength < (TEXT_BUFFER_SIZE - 1)) {
            char c = '1' + (keycode - GLFW_KEY_1); // Map to '1'-'9'

            consoleEntryText[currentEntryLength] = c;
            consoleEntryText[currentEntryLength + 1] = '\0'; // Null-terminate
            currentEntryLength++;
        }
    } else if (keycode == GLFW_KEY_0) { // Handle '0'
        if (currentEntryLength < (TEXT_BUFFER_SIZE - 1)) {
            consoleEntryText[currentEntryLength] = '0';
            consoleEntryText[currentEntryLength + 1] = '\0'; // Null-terminate
            currentEntryLength++;
        }
    } else if (keycode == GLFW_KEY_BACKSPACE && currentEntryLength > 0) { // Handle backspace
        currentEntryLength--;
        consoleEntryText[currentEntryLength] = '\0'; // Null-terminate
    } else if (keycode == GLFW_KEY_SPACE) { // Handle space
        if (currentEntryLength < (TEXT_BUFFER_SIZE - 1)) {
            consoleEntryText[currentEntryLength] = ' ';
            consoleEntryText[currentEntryLength + 1] = '\0';
            currentEntryLength++;
        }
    } else if (keycode == GLFW_KEY_ENTER || keycode == GLFW_KEY_KP_ENTER) { // Handle enter (main and keypad)
        // Handle command execution or clear the console
        DualLog("Console command: %s\n", consoleEntryText);
        ProcessConsoleCommand(consoleEntryText);
    }
}

float textVertexData[4096]; // Reusable buffer for text vertices.  Most text only needs ~3000

void RenderFormattedText(float x, float y, float z, uint32_t color, uint8_t fontID, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(uiTextBuffer, TEXT_BUFFER_SIZE, format, args);
    va_end(args);
    glUseProgram(textShaderProgram);
    glProgramUniformMatrix4fv(textShaderProgram, projectionLoc_text, 1, GL_FALSE, uiOrthoProjection);
    glProgramUniform4f(textShaderProgram, textColorLoc_text, textColors[color].r, textColors[color].g, textColors[color].b, textColors[color].a);
    if (fontID == FONT_STOPD) glBindTextureUnit(6, fontAtlasTexStopD);
    else glBindTextureUnit(6, fontAtlasTex);
    glProgramUniform2f(textShaderProgram, texelSizeLoc_text, 1.0f / (float)FONT_ATLAS_SIZE, 1.0f / (float)FONT_ATLAS_SIZE);
    glProgramUniform1ui(textShaderProgram, fontTypeLoc_text, fontID);
    glProgramUniform1i(textShaderProgram, textTextureLoc_text, 6);
    glBindVertexArray(textVAO);

    // Batch vertices for all glyphs
    size_t vertexCount = 0;
    const char* p = uiTextBuffer;
    float xpos = x, ypos = y + GetScreenRelativeY(0.0211f);
    float lineSpacing = GetScreenRelativeY(0.03f); // Match RenderUI
    stbtt_aligned_quad q;
    int characterCount = 0;
    // Define padding for SDF outline (in pixels)
    float paddingPixels = 12.0f;
    float paddingUV = paddingPixels / (float)FONT_ATLAS_SIZE;
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

        // Expand vertex quad
        float vx0 = q.x0 - borderWidthPixels;
        float vy0 = q.y0 - borderWidthPixels;
        float vx1 = q.x1 + borderWidthPixels;
        float vy1 = q.y1 + borderWidthPixels;

        // Expand UVs by padding
        float s0 = q.s0 - paddingUV;
        float t0 = q.t0 - paddingUV;
        float s1 = q.s1 + paddingUV;
        float t1 = q.t1 + paddingUV;

        float textVertices[30] = {
            // Triangle 1
            vx0, vy0, z, s0, t0,
            vx1, vy1, z, s1, t1,
            vx1, vy0, z, s1, t0,
            // Triangle 2
            vx0, vy0, z, s0, t0,
            vx0, vy1, z, s0, t1,
            vx1, vy1, z, s1, t1
        };

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

void RenderLoadingProgress(int32_t offset, const char* format, ...) {
    glUseProgram(imageBlitShaderProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputImageID);
    glProgramUniform1i(imageBlitShaderProgram, texLoc_quadblit, 0);
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

float GetTextHCenter(float pointToCenterOn, int32_t numCharactersNoNullTerminator) {
    float characterWidth = genericTextHeightFac * 0.75f * screen_height; // Measured some and found between 0.6 and 0.82 in Gimp for width to height ratio.
    return (pointToCenterOn - ((float)numCharactersNoNullTerminator * 0.5f) * characterWidth); // This could be mid character ;)
}

void CenterStatusPrint(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    statusTextLengthWithoutNullTerminator = vsnprintf(statusText, TEXT_BUFFER_SIZE, fmt, args);
    va_end(args);
    DualLog("%s\n",statusText);
    statusTextDecayFinished = get_time() + 2.0f; // 2 second decay time before text dissappears.
}
// ============================================================================
uint32_t random_range_rng = 0x12345678u; // Global seed
static inline uint32_t xs32(uint32_t *s){
    uint32_t x=*s; x^=x<<13; x^=x>>17; x^=x<<5;
    return *s = x ? x : 0xdeadbeefu;
}

static inline uint8_t random_range_u8(uint8_t a, uint8_t b){
    uint8_t n = (uint8_t)(b - a + 1u);
    if (!n) return a; // handle wrap if a>b (undefined otherwise)
    uint8_t v, t = (uint8_t)(256u % n);
    do v = (uint8_t)xs32(&random_range_rng); while (v >= 256u - t);
    return (uint8_t)(a + (v % n));
}

// ============================================================================
void InitializePlayer(uint16_t playerIdx) {
    instances[playerIdx].position.x = -20.4f;
    instances[playerIdx].position.y = -43.79f + 0.84f; // Added 0.84f for cam offset from center
    instances[playerIdx].position.z = 10.2f;
    instances[playerIdx].velocity.x = instances[playerIdx].velocity.y = instances[playerIdx].velocity.z = 0.0f;
    instances[playerIdx].scale.x = instances[playerIdx].scale.y = instances[playerIdx].scale.z = 1.0f;
    instances[playerIdx].rotation.x = instances[playerIdx].rotation.y = instances[playerIdx].rotation.z = 0.0f; instances[playerIdx].rotation.w = 1.0f;
    flag_enable(&instances[playerIdx].entflags, ENTFLAG_USEGRAVITY);
    instances[playerIdx].bodyState = BodyState_Standing;
}

void NewGame(void) {
    RenderLoadingProgress(100,"Loading new game...");
    levelCurrentlyLoading = true;
    memset(&ambientRegistry, 0, sizeof(uint16_t));
    loadedAmbients = 0;
    memset(&questData, 0, sizeof(QuestBits));
    questData.lev1SecCode = random_range_u8(0u,9u); // Must do rand's repeatedly to prevent
    questData.lev2SecCode = random_range_u8(0u,9u); // these all being the same number.
    questData.lev3SecCode = random_range_u8(0u,9u);
    questData.lev4SecCode = random_range_u8(0u,9u);
    questData.lev5SecCode = random_range_u8(0u,9u);
    questData.lev6SecCode = random_range_u8(0u,9u);
    renderableCount = 0;
    loadedInstances = 3; // 0 == NULL, 1 == Player1, 2 == Player2
    memset(instances,0,INSTANCE_COUNT * sizeof(Entity)); // Initialize instances, the global entity array for the currently loaded level.
    InitializePlayer(PLAYER1);
    InitializePlayer(PLAYER2);
    loadedLights = 0;
    LoadLevel(startLevel); // Must be after entities!
    SortInstances(); // All instances loaded, sort them for render order: opaques, doublesideds, transparents.  REORDERS instances[] INDICES!!  CAREFUL!!
    RenderLoadingProgress(110,"Loading cull system...");
    CullInit(); // Must be after level! MUST BE AFTER SortInstances!!
    RenderLoadingProgress(120,"Loading voxel lighting data...");
    glClearColor(0.0f, 0.0f, 0.0f, 0.2f); // Set after shadowmap rendering.
    //play_mp3("./Audio/music/THM1-19_medicalstart.mp3",((float)settings_VolumeMusic/100.0f) * 0.4f,100);
    VoxelLists();
    uint32_t shadowmapPixelCount = shadSizeSquared * 6u;
    totalShadowmapPixels = MAX_SHADOWMAPS * shadowmapPixelCount;
    shadowMapSSBO = SetupSSBO(shadowMapSSBO, 5, totalShadowmapPixels * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
    glUseProgram(shadowmapsClearShaderProgram);
    GLuint groupX_shadClear = (totalShadowmapPixels + 31) / 32;
    glDispatchCompute(groupX_shadClear,1, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    numDynamicLights = 0;
    for (int i=0;i<loadedLights;++i) { if (lightIsDynamic[i]) numDynamicLights++; }
    DualLog("%u dynamic lights in level %u\n", numDynamicLights, currentLevel);
    pauseRelativeTime = 0.0f;
    levelCurrentlyLoading = false;
}

static const float quadBlit_vertices[] = {
     1.0f, -1.0f, 1.0f, 0.0f, // Bottom-right
     1.0f,  1.0f, 1.0f, 1.0f, // Top-right
    -1.0f,  1.0f, 0.0f, 1.0f, // Top-left
    -1.0f, -1.0f, 0.0f, 0.0f  // Bottom-left
};

void InitializeEnvironment(void) {
    double init_start_time = get_time();
    DebugRAM("InitializeEnvironment start");   
    if (!glfwInit()) { DualLogError("GLFW initialization failed\n"); exit(1); }
    
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(screen_width, screen_height, "Voxen, the OpenGL Voxel Lit Engine", NULL, NULL);
    malloc_trim(0);
    if (!window) { DualLogError("glfwCreateWindow failed\n"); glfwTerminate(); exit(1); }
    
    glfwMakeContextCurrent(window);
    UpdateScreenSize();
    malloc_trim(0);
    DebugRAM("window init");
    GLFWmonitor* target_monitor = glfwGetPrimaryMonitor();  // Use primary; or monitors[1] for second monitor, etc.
    if (target_monitor) { // TODO: Let user switch monitors from settings, especially in fullscreen.
        const GLFWvidmode* mode = glfwGetVideoMode(target_monitor);
        int mx, my;
        glfwGetMonitorPos(target_monitor, &mx, &my);
        // Center the window on the monitor (windowed mode)
        int xpos = mx + (mode->width - screen_width) / 2;
        int ypos = my + (mode->height - screen_height) / 2;
        glfwSetWindowPos(window, xpos, ypos);
        DualLog("Window positioned (windowed, centered) on monitor: %s (primary) at %d,%d\n", glfwGetMonitorName(target_monitor), xpos, ypos);
    } else { DualLogError("GLFW Unable to obtain target monitor [primary]!\n"); exit(1); }
    
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glewExperimental = GL_TRUE; // Enable modern OpenGL support
    if (glewInit() != GLEW_OK) { DualLog("GLEW initialization failed\n"); exit(1); }

    malloc_trim(0);
    const GLubyte* version = glGetString(GL_VERSION);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    if (!version) { DualLogError("OpenGL support not found!\n"); exit(1);}
    
    DualLog("OpenGL Version: %s\n", (const char*)version);
    DualLog("GPU: %s\n", renderer ? (const char*)renderer : "unknown");
    glfwSwapInterval(settings_Vsync ? 1 : 0);
    Input_Init(window);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_MULTISAMPLE);
    glMinSampleShading(0.0f);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW); // Set triangle sorting order (GL_CW vs GL_CCW)
    glViewport(0, 0, screen_width, screen_height);
    malloc_trim(0);
    CompileShaders();
    malloc_trim(0);
    glProgramUniform1ui(imageBlitShaderProgram, screenWidthLoc_imageBlit, screen_width);
    glProgramUniform1ui(imageBlitShaderProgram, screenHeightLoc_imageBlit, screen_height);
    glProgramUniform1f( imageBlitShaderProgram, shadowmapSizeLoc_imageBlit, (float)(SHADOW_MAP_SIZE));
    glProgramUniform1ui(chunkShaderProgram, screenWidthLoc_chunk, screen_width);
    glProgramUniform1ui(chunkShaderProgram, screenHeightLoc_chunk, screen_height);
    glProgramUniform1f( chunkShaderProgram, shadowmapSizeLoc_chunk, (float)(SHADOW_MAP_SIZE));
    glProgramUniform1ui(ssrShaderProgram, screenWidthLoc_ssr, screen_width / SSR_RES);
    glProgramUniform1ui(ssrShaderProgram, screenHeightLoc_ssr, screen_height / SSR_RES);
    glProgramUniform1i( ssrShaderProgram, outputImageLoc_ssr, 4);
        
    glCreateBuffers(1, &quadVBO);
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
    DebugRAM("after vao chunk bind");
    
    glCreateBuffers(1, &uiImageVBO);
    glCreateVertexArrays(1, &uiImageVAO);
    glEnableVertexArrayAttrib(uiImageVAO, 0);
    glEnableVertexArrayAttrib(uiImageVAO, 1);
    glVertexArrayAttribFormat(uiImageVAO, 0, 3, GL_FLOAT, GL_FALSE, 0); // Position (vec3)
    glVertexArrayAttribFormat(uiImageVAO, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float)); // UV (vec2)
    glVertexArrayVertexBuffer(uiImageVAO, 0, uiImageVBO, 0, 5 * sizeof(float));
    glVertexArrayAttribBinding(uiImageVAO, 0, 0);
    glVertexArrayAttribBinding(uiImageVAO, 1, 0);
    DebugRAM("after ui image vao chunk bind");

    GenerateAndBindTexture(&inputImageID,             GL_RGBA8, screen_width, screen_height,            GL_RGBA, GL_UNSIGNED_BYTE, GL_TEXTURE_2D); // Lit Raster
    GenerateAndBindTexture(&inputWorldPosID,        GL_RGBA32F, screen_width, screen_height,            GL_RGBA,         GL_FLOAT, GL_TEXTURE_2D); // Raster World Positions
    GenerateAndBindTexture(&inputDepthID, GL_DEPTH_COMPONENT24, screen_width, screen_height, GL_DEPTH_COMPONENT,         GL_FLOAT, GL_TEXTURE_2D); // Raster Depth
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
    RenderLoadingProgress(100,"Loading..."); // Early load screen to immediately clear what's in the window

    InitFontAtlasses();
    glCreateBuffers(1, &textVBO);
    glCreateVertexArrays(1, &textVAO);    
    glEnableVertexArrayAttrib(textVAO, 0);
    glEnableVertexArrayAttrib(textVAO, 1);
    glVertexArrayAttribFormat(textVAO, 0, 3, GL_FLOAT, GL_FALSE, 0); // pos (x,y,z) 4 floats per vertex, stride = 4*sizeof(float)
    glVertexArrayAttribFormat(textVAO, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float));  // uv (s,t)
    glVertexArrayVertexBuffer(textVAO, 0, textVBO, 0, 5 * sizeof(float));
    glVertexArrayAttribBinding(textVAO, 0, 0);
    glVertexArrayAttribBinding(textVAO, 1, 0);

    Input_MouselookApply(); // Input
    InitializeAudio(); // Audio
    DebugRAM("audio init");
    RenderLoadingProgress(50,"Loading...");
    ParseGameData();
    RenderLoadingProgress(100,"Loading textures...");
    DualLog("Window and GL Init took %f seconds\n", get_time() - init_start_time);
    LoadTextures();
    RenderLoadingProgress(100,"Loading models...");
    LoadModels();
    RenderLoadingProgress(100,"Loading entities...");
    LoadEntities(); // Must be after models and textures else entity types can't be validated.
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
                               startOfNextType = startOfDoubleSidedInstances; break;
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
            if (instCellIdx < ARRSIZE && !(gridCellStates[instCellIdx] & CELL_VISIBLE)) continue;
            
            float distSqrd = squareDistance3D(      instances[i].position.x,       instances[i].position.y,       instances[i].position.z,
                                              instances[PLAYER1].position.x, instances[PLAYER1].position.y, instances[PLAYER1].position.z);
            
            if (distSqrd >= FAR_PLANE_SQUARED) continue;

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
            verticesRenderedThisFrame += modelVertexCounts[modelType];
        }
    }
    
    if (type == REND_TRANSPARENT) {
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
        glEnable(GL_CULL_FACE);
    }
}

// static const char* debugViewNames[] = {
//     "standard render", // 0
//     "unlit",           // 1
//     "surface normals", // 2
//     "depth",           // 3
//     "reflections"     // 4
// };

void SetFog() {
    fogColorRUsed = fogColorR * fogBaseDensityForLevel;
    fogColorGUsed = fogColorG * fogBaseDensityForLevel;
    fogColorBUsed = fogColorB * fogBaseDensityForLevel;
}

int32_t main(int32_t argc, char* argv[]) {
    game_start_time = get_time();
    DebugRAM("program start");
    random_range_rng = (uint32_t)game_start_time; // Seed global rand uniquely with time since system boot.
    OpenConsoleLogFile();
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
    ActiveLogFileInit();
    DebugRAM("prior to event system init");
    DualLog("Voxen "
            VERSION_STRING
            " by W. Josiah Jack, MIT-0 licensed\n");
    journalFirstWrite = true;
    clear_ev_queue();  // Initialize the eventQueue as empty
    clear_ev_journal(); // Initialize the event journal as empty.
    eventQueue[eventIndex].type = EV_NULL;
    eventQueue[eventIndex].timestamp = get_time();
    eventQueue[eventIndex].deltaTime_ns = 0.0;
    if (argc == 3 && strcmp(argv[1], "play") == 0) { // Log playback
        DualLog("Playing log: %s\n", argv[2]);
        OpenLogForPlayback(argv[2]);
    } else if (argc == 3 && strcmp(argv[1], "record") == 0) { // Log record
        manualLogName = argv[2]; // TODO: Add manual log naming support from cli arg.
    }

    InitializeEnvironment();
//     double last_physics_time = get_time();
    last_time = get_time();
    DebugRAM("prior to game loop");
    Input_MouselookApply();
    lastJournalWriteTime = get_time();
    RenderShadowmaps();
    DualLog("Game Initialized in %f secs\n",lastJournalWriteTime - game_start_time);
    while(1) {
        current_time = get_time();
        double frame_time = current_time - last_time;
        if (!gamePaused) pauseRelativeTime += (float)frame_time;
        
        // Handle Berserk Effect for Compositing Shader
        float berserkTimeRemainingNormalized = berserkFinished > 0.0001f ? (berserkFinished - pauseRelativeTime) / PATCH_TIME_BERSERK : 0.0f;
        if (berserkFinished < pauseRelativeTime && berserkFinished > 0.0001f) {
            berserkFinished = 0.0f;
            berserkTimeRemainingNormalized = 0.0f;
        }

        // Enqueue input events
        glfwPollEvents();
        if (glfwWindowShouldClose(window)) EnqueueEvent_Simple(EV_QUIT);
//         double timeSinceLastPhysicsTick = current_time - last_physics_time;
//         if (timeSinceLastPhysicsTick > 0.006944444f && !gamePaused && !menuActive) { // 144fps fixed tick rate
//             last_physics_time = current_time;
            EnqueueEvent_Simple(EV_PHYSICS_TICK);
//         }

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
        textDrawCallsRenderedThisFrame = 0;
        uiImageDrawCallsRenderedThisFrame = 0;
        shadowDrawCallsRenderedThisFrame = 0;
        verticesRenderedThisFrame = 0;
        uiImageCount = 0;
        memset(lightDirty,0,LIGHT_COUNT * sizeof(bool));
        
        // 0. Clear Frame Buffers and Depth
        if (!gamePaused) glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear main FBO.  glClearBufferfv was actually SLOWER!
    
        // 0.5 Set View and Projection Matrices
        float view[16]; // Also known as view matrix
        mat4_lookat_from(view,&cam_rotation, instances[PLAYER1].position.x, instances[PLAYER1].position.y, instances[PLAYER1].position.z);
        float viewProj[16]; // view-projection matrix
        float invViewProj[16]; // inverse view-projection matrix
        mul_mat4(viewProj, rasterPerspectiveProjection, view);
        invertAffineMat4(invViewProj, viewProj);
        float invViewRot[9];
        invViewRot[0] = view[0];
        invViewRot[1] = view[4];
        invViewRot[2] = view[8];
        invViewRot[3] = view[1];
        invViewRot[4] = view[5];
        invViewRot[5] = view[9];
        invViewRot[6] = view[2];
        invViewRot[7] = view[6];
        invViewRot[8] = view[10];
        if (!gamePaused && !menuActive) { // !PAUSED BLOCK -------------------------------------------------
            UpdateAmbientSounds();
            
            // 1. Culling
            Cull(); // Get world cell culling data into gridCellStates from precomputed data at init of what cells see what other cells.
            
            // 2. Pass instance data to GPU
            for (uint32_t i = 3; i < loadedInstances; i++) { if (dirtyInstances[i]) { UpdateInstanceMatrix(i); } } // Skip player indices and start at 3
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, matricesBuffer);
            glBufferData(GL_SHADER_STORAGE_BUFFER, loadedInstances * 16 * sizeof(float), modelMatrices, GL_DYNAMIC_DRAW);
            
            // 3. Dynamic Shadowmaps
            uint32_t lightBase = 817;
            uint32_t litIdx = lightBase * LIGHT_DATA_SIZE;
            if ((lights[litIdx + LIGHT_DATA_OFFSET_POSX]) != testLight_x) { lights[litIdx + LIGHT_DATA_OFFSET_POSX] = testLight_x; lightDirty[lightBase] = true; }
            if ((lights[litIdx + LIGHT_DATA_OFFSET_POSY]) != testLight_y) { lights[litIdx + LIGHT_DATA_OFFSET_POSY] = testLight_y; lightDirty[lightBase] = true; }
            if ((lights[litIdx + LIGHT_DATA_OFFSET_POSZ]) != testLight_z) { lights[litIdx + LIGHT_DATA_OFFSET_POSZ] = testLight_z; lightDirty[lightBase] = true; }
//             uint16_t numLightsFoundDirty = 0;
//             for (int i = 0; i < loadedLights; ++i) {
//                 if (lightDirty[i]) numLightsFoundDirty++;
//             }

//             if (numLightsFoundDirty > 0) {
                UpdateVoxelLightLists(); // Takes 1.4ms of total frametime!!
                if (settings_Shadows > 0u) RenderShadowmaps();
                else {
                    memset(shadowmapIndirectionList, MAX_SHADOWMAPS + 1, loadedLights * sizeof(uint32_t));
                    glNamedBufferData(shadowMapsIndirectionID, loadedLights * sizeof(uint32_t), shadowmapIndirectionList, GL_DYNAMIC_DRAW);
                }
//             }
            
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the corner where last shadowmap wrote into  

            // 4. Raterized Geometry
            //        Standard vertex + fragment rendering, but with special packing to minimize transfer data amounts
            glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
            glUseProgram(chunkShaderProgram);
            glUniformMatrix4fv(viewProjLoc_chunk, 1, GL_FALSE, viewProj);
            glUniform1f(worldMin_xLoc_chunk, worldMin_x);
            glUniform1f(worldMin_zLoc_chunk, worldMin_z);
            glUniform3f(camPosLoc_chunk, instances[PLAYER1].position.x, instances[PLAYER1].position.y, instances[PLAYER1].position.z);
            glUniform1f(fogColorRLoc_chunk, fogColorRUsed);
            glUniform1f(fogColorGLoc_chunk, fogColorGUsed);
            glUniform1f(fogColorBLoc_chunk, fogColorBUsed);
            glProgramUniform1ui(chunkShaderProgram, isUILoc_chunk, 0u);
            glProgramUniform1ui(chunkShaderProgram, unlitLoc_chunk,0u);
            glProgramUniform1ui(chunkShaderProgram, reflectionsEnabledLoc_chunk, settings_Reflections);
            glProgramUniform1ui(chunkShaderProgram, shadowsEnabledLoc_chunk, settings_Shadows);
            glBindVertexArray(vao_chunk);
            memset(instanceIsLODArray,true,INSTANCE_COUNT * sizeof(bool)); // All using lower detail LOD mesh.
            RenderInstances(REND_OPAQUE);      // Opaque, e.g. most objects and level geometry chunks
            RenderInstances(REND_DOUBLESIDED); // Double Sided, e.g. cyber panels and foliage and negative scaled objects
            RenderInstances(REND_TRANSPARENT); // Transparents, e.g. windows and beakers
            glBindVertexArray(0);
            glUseProgram(0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            // ====================================================================
            // 5. SSR (Screen Space Reflections)
            if ((debugView == 0 || debugView == 4) && settings_Reflections > 0) {
                glUseProgram(ssrShaderProgram);
                glUniformMatrix4fv(viewProjectionLoc_ssr, 1, GL_FALSE, viewProj);                
                glUniform3f(camPosLoc_ssr, instances[PLAYER1].position.x, instances[PLAYER1].position.y, instances[PLAYER1].position.z);
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
        glProgramUniform1i(imageBlitShaderProgram, outputImageLoc_imageBlit, 4);
        glProgramUniform1ui(imageBlitShaderProgram, skyVisibleLoc_imageBlit, (gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX) || currentLevel == 13);
        glProgramUniform1ui(imageBlitShaderProgram, planetaryBodiesVisibleLoc_imageBlit, (gridCellStates[playerCellIdx] & CELL_SEES_SUN) && currentLevel != 13);
        glProgramUniform1ui(imageBlitShaderProgram, groveShieldVisibleLoc_imageBlit, ((currentLevel >= 10 && currentLevel < 13) ? 1u : 0u) && (gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX));
        uint32_t shieldOnType = 0u; // No shield green tint.
        if (questData.ShieldActivated) {
            if (currentLevel == 6 || currentLevel == 7) shieldOnType = 2u; // Shielding only below player for lower levels.
            else if (currentLevel <= 5) shieldOnType = 1u; // Shielding everywhere as levels fully within shield.
        }
        glProgramUniform1ui(imageBlitShaderProgram, stationShieldVisibleLoc_imageBlit, shieldOnType);
        glProgramUniform1ui(imageBlitShaderProgram, reflectionsEnabledLoc_imageBlit, settings_Reflections);
        glProgramUniform1ui(imageBlitShaderProgram, aaEnabledLoc_imageBlit, settings_AntiAliasing);
        glProgramUniform1ui(imageBlitShaderProgram, brightnessSettingLoc_imageBlit, settings_Brightness);
        glProgramUniform1ui(imageBlitShaderProgram, shadowsSettingLoc_imageBlit, settings_Shadows);
        glProgramUniform1f(imageBlitShaderProgram, worldMin_xLoc_imageBlit, worldMin_x);
        glProgramUniform1f(imageBlitShaderProgram, worldMin_zLoc_imageBlit, worldMin_z);
        glProgramUniform3f(imageBlitShaderProgram, camPosLoc_imageBlit, instances[PLAYER1].position.x, instances[PLAYER1].position.y, instances[PLAYER1].position.z);
        glUniformMatrix4fv(viewProjectionLoc_imageBlit, 1, GL_FALSE, viewProj);
        glUniformMatrix3fv(invViewRotLoc_imageBlit, 1, GL_FALSE, invViewRot);
        glProgramUniform1f(imageBlitShaderProgram, berserkTimeRemainingLoc_imageBlit, berserkTimeRemainingNormalized);
        glProgramUniform1f(imageBlitShaderProgram, berserkSeedTimestampLoc_imageBlit, berserkSeedTime);
        glProgramUniform1f(imageBlitShaderProgram, fovLoc_imageBlit, cam_fov);
        glProgramUniform1i(imageBlitShaderProgram, texLoc_quadblit, 0);
        glUniform3f(camRotLoc_imageBlit, deg2rad(cam_yaw), deg2rad(cam_pitch), deg2rad(cam_roll));
        glProgramUniform1f(imageBlitShaderProgram, timeValLoc_imageBlit, pauseRelativeTime * 0.1);
        glProgramUniform1f(imageBlitShaderProgram, aspectLoc_imageBlit, aspect3D);
        glBindVertexArray(quadVAO);
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        drawCallsRenderedThisFrame++;
        verticesRenderedThisFrame += 4;
        glEnable(GL_DEPTH_TEST); // Turn on for UI Images
        glBindTextureUnit(0, 0);
        glUseProgram(0);
        // End world rendering
        // ------------------------------------
        // ====================================
        // HUD
        // UI Common GL traits
        uint32_t drawCallsNormal = drawCallsRenderedThisFrame;

        // UI Common References
        float screenCenterX = (float)screen_width / 2;
        float screenCenterY = (float)screen_height / 2;
        float lineSpacing = GetScreenRelativeY(genericTextHeightFac);
        
        // 7. UI
        glEnable(GL_BLEND);
        glClear(GL_DEPTH_BUFFER_BIT); // Clear main FBO.  glClearBufferfv was actually SLOWER!
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE); // Fixes alpha rendering of text, but makes the z sort not work for some reason.
//         glDepthMask(GL_TRUE); // Fixes z sorting unless it has alpha
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
                if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
                    DualLog("Clicked the Shoot Mode button %u\n", globalFrameNum);
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
//         RenderFormattedText(leftPad, debugTextStartY + (lineSpacing * 1), UI_LAYER_1, TEXT_WHITE, FONT_NORMAL, "cam yaw: %.2f, cam pitch: %.2f, cam roll: %.2f", cam_yaw, cam_pitch, cam_roll);
//         RenderFormattedText(leftPad, debugTextStartY + (lineSpacing * 2), UI_LAYER_4, TEXT_WHITE, "Peak frame queue count: %d", maxEventCount_debug);
//         RenderFormattedText(leftPad, debugTextStartY + (lineSpacing * 3), UI_LAYER_1, TEXT_WHITE, FONT_NORMAL, "DebugView: %d (%s), DebugValue: %d", debugView, debugViewNames[debugView], debugValue);
//         RenderFormattedText(leftPad, debugTextStartY + (lineSpacing * 4), UI_LAYER_1, TEXT_WHITE, "Num cells: %d, Player cell(%d):: x: %d, y: %d, z: %d", numCellsVisible, playerCellIdx, playerCellIdx_x, playerCellIdx_y, playerCellIdx_z);
        RenderFormattedText(leftPad, debugTextStartY + (lineSpacing * 5), UI_LAYER_1, TEXT_WHITE, FONT_NORMAL, "Character set test: abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.,;:'\"`~!@#...");
        RenderFormattedText(leftPad, debugTextStartY + (lineSpacing * 6), UI_LAYER_1, TEXT_WHITE, FONT_NORMAL, "  ...$%^&*()-=+\\/|<>äöüéóâêîôû123456789る。エレベーターでレベルを離れよБбвГгДдЁЖжзИиЙйкЛлмнПптФфЦцЧчШшЩщЪъЫыЬьЭэЮюЯя[{end test}]");
        if (consoleActive) RenderFormattedText(leftPad, 0, UI_LAYER_1, TEXT_WHITE, FONT_NORMAL, "] %s",consoleEntryText);
        if (statusTextDecayFinished > current_time) RenderFormattedText(GetTextHCenter(screenCenterX,statusTextLengthWithoutNullTerminator), screenCenterY - GetScreenRelativeY(0.30f + (genericTextHeightFac * 2.0f)), UI_LAYER_1, TEXT_WHITE, FONT_NORMAL, "%s",statusText);

        glDepthMask(GL_TRUE);
        RenderUIImages();
        
        // Frame stats (AFTER EVERYTHING ELSE)
        double time_now = get_time();
        drawCallsRenderedThisFrame++; textDrawCallsRenderedThisFrame++; // Add one more for this text render ;)
        RenderFormattedText(leftPad, debugTextStartY - lineSpacing, UI_LAYER_5, TEXT_WHITE, FONT_NORMAL, "ms: %.2f, CPU %.2f (FPS: %d, Worst: %d), Drwclls: %d [G %d UI %d Txt %d Shd %d] Vrts: %d", (time_now - last_time) * 1000.0f,cpuTime * 1000.0f,framesPerLastSecond,worstFPS,drawCallsRenderedThisFrame, drawCallsNormal, uiImageDrawCallsRenderedThisFrame, textDrawCallsRenderedThisFrame, shadowDrawCallsRenderedThisFrame, verticesRenderedThisFrame);
        // End ALL rendering
        // ------------------------------------
        // ====================================
        // Final Client Frame Actions
        last_time = time_now;
        if ((time_now - lastFrameSecCountTime) >= 1.00) {
            lastFrameSecCountTime = time_now;
            framesPerLastSecond = globalFrameNum - lastFrameSecCount;
            if (framesPerLastSecond < worstFPS && globalFrameNum > 2000) worstFPS = framesPerLastSecond; // After startup, keep track of worst framerate seen.
            lastFrameSecCount = globalFrameNum;
        }
        
        if (keys[GLFW_KEY_F12]) {
            if (time_now > screenshotTimeout) {
                Screenshot();
                screenshotTimeout = time_now + 1.0; // Prevent saving more than 1 per second for sanity purposes.
            }
        }
        
        glUseProgram(0);
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        cpuTime = get_time() - current_time;
        glfwSwapBuffers(window); // Present frame
        CHECK_GL_ERROR();
        globalFrameNum++;
        #ifdef DEBUG_RAM_OUTPUT
            if (globalFrameNum == 4) { DebugRAM("after 4 frames of running"); malloc_trim(0); }
            else if (globalFrameNum == 100) { DebugRAM("after 100 frames of running"); }
            else if (globalFrameNum == 200) DebugRAM("after 200 frames of running");
        #endif
    }
    glfwTerminate();
    return 0;
}
