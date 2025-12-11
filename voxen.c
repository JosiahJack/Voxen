// voxen.c
// Description: A realtime OpenGL 4.3+ Game Engine for Citadel: The System Shock Fan Remake
#include "os.h" // Operating System calls shim layer.
#include "voxen.h"
#include "event.h"
#include "entity.h"
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
#include "Shaders/bluenoise64.cginc"
#include "todo.h"
#include "input.c"

Voxen_GlobalContext voxen_globalContext;

bool inventoryMode = false;
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
#define SSR_RES 2 // Ratio is (1 / SSR_RES) * render resolution.
VoxenSettings voxen_Settings = {
    .ScreenWidth = 1366u,
    .ScreenHeight = 768u,
    .Shadows = 1u,
    .AntiAliasing = 1u, // Default 1
    .Brightness = 70u, // Default 100 (for %)
    .VolumeMusic = 20u,
    .Language = 0, // English default
    .CullEnabled = 1,
    .FOV = 65.0f,
    .Reflections = 1u, // Default 1
    .Vsync = false
};

// Cheats
Voxen_Cheats voxen_Cheats = {
    .god = true,
    .noclip = true,
    .notarget = false,
    .bottomless = false,
    .superoverride = false,
    .fatigueCheat = false,
    .redbull = false,
    .consoleActive = false,
    .noHUD = false,
    .showLocation = true,
    .showFPS = true,
    .dizzyLevel = 0u,
    .editMode = true
};
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
double pauseRelativeTime = 0.0f;
QuestBits questData;
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
float fogColorR, fogColorG, fogColorB, fogBaseDensityForLevel;
Voxen_GL_Comms voxen_GL_Comms;
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
uint32_t voxelLightListsRaw[VOXEL_COUNT * 4];
uint32_t voxelLightListIndices[VOXEL_COUNT * 2];
uint32_t shadowmapIndirectionList[LIGHT_COUNT];
uint16_t numDynamicLights;
float lights[LIGHT_COUNT * LIGHT_DATA_SIZE];
float lightsRangeSquared[LIGHT_COUNT];
bool lightDirty[LIGHT_COUNT] = { [0 ... LIGHT_COUNT-1] = true };
static float lightView[LIGHT_COUNT][6][4][4]; // Array of Array of 6 Arrays of 16 floats (matrix 4x4).  lightView[i][face][0 ... 15]
static float lightViewProj[LIGHT_COUNT][6][4][4]; // Array of Array of 6 Arrays of 16 floats (matrix 4x4).  lightViewProj[i][face][0 ... 15]
FrustumPlane lightFrustumPlanes[LIGHT_COUNT][6][6]; // Array of Array of 6 Arrays of FrustumPlane structs (four floats).  lightFrustumPlanes[i][face][.nx,.ny,, .nz, .d]
// ----------------------------------------------------------------------------
// OpenGL / Rendering Helper Functions
void GenerateAndBindTexture(GLuint *id, GLint internalFormat, int32_t width, int32_t height, GLenum format, GLenum type, GLenum target) {
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
    if (!success) { char infoLog[512]; glGetShaderInfoLog(shader, 512, NULL, infoLog); DualLogError("%s Compilation Failed: %s\n", shaderName, infoLog); OS_Exit(1); }
    return shader;
}

GLuint LinkProgram(GLuint *shaders, int32_t count, const char *programName) {
    GLuint program = glCreateProgram();
    for (int32_t i = 0; i < count; i++) { glAttachShader(program, shaders[i]); glDeleteShader(shaders[i]); }
    glLinkProgram(program);
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) { char infoLog[512]; glGetProgramInfoLog(program, 512, NULL, infoLog); DualLogError("%s Linking Failed: %s\n", programName, infoLog); OS_Exit(1); }
    return program;
}

void CompileShaders(void) {
    GLuint vertShader, fragShader, computeShader;
    vertShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource, "Chunk Vertex Shader");
    fragShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderTraditional, "Chunk Fragment Shader");
    voxen_GL_Comms.chunkShaderProgram = LinkProgram((GLuint[]){vertShader, fragShader}, 2, "Chunk Shader Program");
    
    vertShader = CompileShader(GL_VERTEX_SHADER, shadowmapVertexShaderSource, "Shadowmaps Vertex Shader");
    fragShader = CompileShader(GL_FRAGMENT_SHADER, shadowmapFragmentShaderSource, "Shadowmaps Fragment Shader");
    voxen_GL_Comms.shadowmapsShaderProgram = LinkProgram((GLuint[]){vertShader, fragShader}, 2, "Shadowmaps Shader Program");

    vertShader = CompileShader(GL_VERTEX_SHADER, textVertexShaderSource, "Text Vertex Shader");
    fragShader = CompileShader(GL_FRAGMENT_SHADER, textFragmentShaderSource, "Text Fragment Shader");
    voxen_GL_Comms.textShaderProgram = LinkProgram((GLuint[]){vertShader, fragShader}, 2, "Text Shader Program");

    computeShader = CompileShader(GL_COMPUTE_SHADER, ssr_computeShader, "Screen Space Reflections Compute Shader");
    voxen_GL_Comms.ssrShaderProgram = LinkProgram((GLuint[]){computeShader}, 1, "Screen Space Reflections Shader Program");
    
    computeShader = CompileShader(GL_COMPUTE_SHADER, shadowmaps_clear_computeShader, "Shadowmaps Clear Compute Shader");
    voxen_GL_Comms.shadowmapsClearShaderProgram = LinkProgram((GLuint[]){computeShader}, 1, "Shadowmaps Clear Shader Program");

    vertShader = CompileShader(GL_VERTEX_SHADER,   quadVertexShaderSource,   "Image Blit Vertex Shader");
    fragShader = CompileShader(GL_FRAGMENT_SHADER, quadFragmentShaderSource, "Image Blit Fragment Shader");
    voxen_GL_Comms.imageBlitShaderProgram = LinkProgram((GLuint[]){vertShader, fragShader}, 2, "Image Blit Shader Program");
    CHECK_GL_ERROR();
    
    glGenBuffers(1, &voxen_GL_Comms.blueNoiseBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, voxen_GL_Comms.blueNoiseBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 12288 * sizeof(float), blueNoise, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, voxen_GL_Comms.blueNoiseBuffer); // Use binding point 13
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void SetSkyRotateSpeed(void) {
    float speeds[] = { 0.05f, 1.0f, 2.5f, 3.75f, 6.25f };
    float skyRotateSpeed = speeds[voxen_Cheats.dizzyLevel];
    glProgramUniform1f(voxen_GL_Comms.imageBlitShaderProgram, 30, skyRotateSpeed);
}

void UpdateScreenSize(void) {
    UpdateProjectionMatrices();
    glProgramUniform1ui(voxen_GL_Comms.imageBlitShaderProgram, 2, voxen_Settings.ScreenWidth);
    glProgramUniform1ui(voxen_GL_Comms.imageBlitShaderProgram, 3, voxen_Settings.ScreenHeight);
    glProgramUniform1f(voxen_GL_Comms.imageBlitShaderProgram, 23, (float)(SHADOW_MAP_SIZE));
    glProgramUniform1i(voxen_GL_Comms.imageBlitShaderProgram, 26, SSR_RES);
    glProgramUniform1ui(voxen_GL_Comms.chunkShaderProgram, 6, voxen_Settings.ScreenWidth);
    glProgramUniform1ui(voxen_GL_Comms.chunkShaderProgram, 7, voxen_Settings.ScreenHeight);
    glProgramUniform1f(voxen_GL_Comms.chunkShaderProgram, 16, (float)(SHADOW_MAP_SIZE));
    glProgramUniform1ui(voxen_GL_Comms.ssrShaderProgram, 0, voxen_Settings.ScreenWidth / SSR_RES);
    glProgramUniform1ui(voxen_GL_Comms.ssrShaderProgram, 1, voxen_Settings.ScreenHeight / SSR_RES);       
    glProgramUniform1i(voxen_GL_Comms.ssrShaderProgram, 2, SSR_RES);
    SetSkyRotateSpeed();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Globally same alpha blending
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

void UpdateProjectionMatrices(void) {
    float* m;
    m = uiOrthoProjection;
    m[0] = 2.0f / (float)voxen_Settings.ScreenWidth; m[1] =                                         0.0f; m[2] =  0.0f; m[3] = 0.0f;
    m[4] =                                     0.0f; m[5] = -2.0f / ((float)voxen_Settings.ScreenHeight); m[6] =  0.0f; m[7] = 0.0f;
    m[8] =                                     0.0f; m[9] =                                         0.0f; m[10]= -1.0f; m[11]= 0.0f;
    m[12]=                                    -1.0f; m[13]=                                         1.0f; m[14]=  0.0f; m[15]= 1.0f;
    
    aspect3D = (float)voxen_Settings.ScreenWidth / (float)voxen_Settings.ScreenHeight;
    float f = vcot(voxen_Settings.FOV * PI / 360.0f);
    m = rasterPerspectiveProjection;
    m[0] = f / aspect3D; m[1] = 0.0f; m[2] =                                                      0.0f; m[3] =  0.0f;
    m[4] =         0.0f; m[5] =    f; m[6] =                                                      0.0f; m[7] =  0.0f;
    m[8] =         0.0f; m[9] = 0.0f; m[10]=      -(FAR_PLANE + NEAR_PLANE) / (FAR_PLANE - NEAR_PLANE); m[11]= -1.0f;
    m[12]=         0.0f; m[13]= 0.0f; m[14]= -2.0f * FAR_PLANE * NEAR_PLANE / (FAR_PLANE - NEAR_PLANE); m[15]=  0.0f;
    
    float aspectShad = (float)SHADOW_MAP_SIZE / (float)SHADOW_MAP_SIZE;
    f = 1.0f / vtan(SHADOWMAP_FOV * PI / 360.0f); // vcot introduces skewness causing false "Peter-Panning" from bubble distortion of the shadowmap depths.  Just stick with recip tangent.
    m = shadowmapsPerspectiveProjection;
    m[0] = f / aspectShad; m[1] = 0.0f; m[2] =                                                                  0.0f; m[3] =  0.0f;
    m[4] =           0.0f; m[5] =    f; m[6] =                                                                  0.0f; m[7] =  0.0f;
    m[8] =           0.0f; m[9] = 0.0f; m[10]=      -(LIGHT_RANGE_MAX + NEAR_PLANE) / (LIGHT_RANGE_MAX - NEAR_PLANE); m[11]= -1.0f;
    m[12]=           0.0f; m[13]= 0.0f; m[14]= -2.0f * LIGHT_RANGE_MAX * NEAR_PLANE / (LIGHT_RANGE_MAX - NEAR_PLANE); m[15]=  0.0f;
}

__attribute__((pure)) bool SphereInFrustum(FrustumPlane* planes, float cx, float cy, float cz, float radius) {
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

void UpdateDynamicLights(void) {
    if (gamePaused || menuActive) return;
    
    for (int i=0;i<loadedLights;++i) {
        if (lightIntervalStepsLength[i] < 1) continue;
        
        int litIdx = i * LIGHT_DATA_SIZE;
        if (lightOn[i]) {
            if (lightIntervalStepsLength[i] > 0) {
                float differenceInIntensity = (lightMaxIntensity[i] - lightMinIntensity[i]);
                if (lightLerpUp[i]) { // Going from lightMinIntensity to lightMaxIntensity
                    if (lightLerpTime[i] < (float)pauseRelativeTime) {
                        lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY] = lightMaxIntensity[i];
                        lightLerpUp[i] = false;
                        lightCurrentStep[i]++;
                        if (lightCurrentStep[i] >= lightIntervalStepsLength[i]) lightCurrentStep[i] = 0;
                        lightLerpStepTime[i] = lightIntervalSteps[i][lightCurrentStep[i]];
                        lightLerpTime[i] = (float)pauseRelativeTime + lightLerpStepTime[i];
                        lightLerpStartTime[i] = (float)pauseRelativeTime;
                    } else {
                        if (lightLerpOn[i]) {
                            if (lightCurrentStep[i] < lightIntervalStepIsLerpingLength[i]) {
                                if (intervalStepisLerping[i][lightCurrentStep[i]]) {
                                    lightLerpValue[i] = ((float)pauseRelativeTime - lightLerpStartTime[i])/(lightLerpTime[i] - lightLerpStartTime[i]); // percent towards goal time
                                    lightLerpValue[i] = lightMinIntensity[i] + (differenceInIntensity * (lightLerpValue[i]));
                                    lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY] = lightLerpValue[i];
                                }
                            }
                        }
                    }
                } else { // Going from lightMaxIntensity to lightMinIntensity
                    if (lightLerpTime[i] < (float)pauseRelativeTime) {
                        lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY] = lightMinIntensity[i];
                        lightLerpUp[i] = true;
                        lightCurrentStep[i]++;
                        if (lightCurrentStep[i] >= lightIntervalStepsLength[i]) lightCurrentStep[i] = 0;
                        lightLerpStepTime[i] = lightIntervalSteps[i][lightCurrentStep[i]];
                        lightLerpTime[i] = (float)pauseRelativeTime + lightLerpStepTime[i];
                        lightLerpStartTime[i] = (float)pauseRelativeTime;
                    } else {
                        if (lightLerpOn[i]) {
                            if (lightCurrentStep[i] == lightIntervalStepsLength[i]) lightCurrentStep[i] = 0;
                            if (lightCurrentStep[i] < lightIntervalStepIsLerpingLength[i]) {
                                if (intervalStepisLerping[i][lightCurrentStep[i]]) {
                                    lightLerpValue[i] = ((float)pauseRelativeTime - lightLerpStartTime[i])/(lightLerpTime[i] - lightLerpStartTime[i]); // percent towards goal time
                                    lightLerpValue[i] = lightMinIntensity[i] + (differenceInIntensity * (1.0f - lightLerpValue[i]));
                                    lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY] = lightLerpValue[i];
                                }
                            }
                        }
                    }
                }

            } else { // Light is on but no steps so set to normal intensity setting
                lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY] = lightBaseIntensity[i];
            }
        } else { // Light is turned off.
            DualLog("Seting light to off\n");
            lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY] = lightMinIntensity[i];
        }
    }

    glNamedBufferData(lightsID,loadedLights * LIGHT_DATA_SIZE * sizeof(float), lights, GL_DYNAMIC_DRAW);
}

uint32_t lightCounts[VOXEL_COUNT] = {0}; // Track current count for each voxel
void UpdateVoxelLightLists(void) {
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
//                 if (!(gridCellStates[cellIndex] & CELL_OPEN)) continue;
                
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
    memset(lightCounts,0,VOXEL_COUNT * sizeof(uint32_t)); // Track current count for each voxel
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
//                 if (!(gridCellStates[cellIndex] & CELL_OPEN)) continue;

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
}

uint16_t largestNearbyMeshCount = 0;

void RenderShadowmap(uint16_t lightIdx) {
    uint32_t litIdx = lightIdx * LIGHT_DATA_SIZE;
    float lightPosX = lights[litIdx + LIGHT_DATA_OFFSET_POSX];
    float lightPosY = lights[litIdx + LIGHT_DATA_OFFSET_POSY];
    float lightPosZ = lights[litIdx + LIGHT_DATA_OFFSET_POSZ];
    float lightRadius = lights[litIdx + LIGHT_DATA_OFFSET_RANGE];
    float halfRadSqrd = lightRadius * lightRadius * 0.5f;
    float effectiveRadius = vmin(lightRadius, 15.36f);
    uint16_t nearMeshes[256]; // Found that this is typically around 172
    uint16_t nearbyMeshCount = 0;
    for (uint16_t j = 3; j < loadedInstances; j++) { // Skip player indices and start at 3
        if (instances[j].modelIndex >= loadedModels) continue;
        if (modelVertexCounts[instances[j].modelIndex] < 1) continue;

        uint16_t instCellIdx = (uint16_t)cellIndexForInstance[j];
        if (voxen_Settings.CullEnabled) {
            if (instCellIdx < ARRSIZE && !(gridCellStates[instCellIdx] & CELL_VISIBLE)) continue;
            
            float radius = modelBounds[(instances[j].modelIndex * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_RADIUS];
            float distToLightSqrd = squareDistance3D(instances[j].position.x, instances[j].position.y, instances[j].position.z, lightPosX, lightPosY, lightPosZ);
            float radSum = (effectiveRadius + radius);
            if (distToLightSqrd > radSum * radSum) continue;
            
            float distSqrd = squareDistance3D(      instances[j].position.x,       instances[j].position.y,       instances[j].position.z,
                                              instances[PLAYER1].position.x, instances[PLAYER1].position.y, instances[PLAYER1].position.z);
            if (distSqrd >= FAR_PLANE_SQUARED) continue;
            
            instanceIsLODArray[j] = (distToLightSqrd >= halfRadSqrd);
        }
        
        nearMeshes[nearbyMeshCount] = j;
        nearbyMeshCount++;
        if (nearbyMeshCount >= 256) { DualLogWarn("Shadowmapping needs larger nearMeshes count than 256!  Skipping some renderables for light %u!\n", lightIdx); break; }
    }

    if (nearbyMeshCount > largestNearbyMeshCount) largestNearbyMeshCount = nearbyMeshCount;
    glProgramUniform1ui(voxen_GL_Comms.shadowmapsShaderProgram, 3, lightIdx * (uint32_t)LIGHT_DATA_SIZE);
    for (uint8_t face = 0; face < 6; face++) {
        glProgramUniform1i(voxen_GL_Comms.shadowmapsShaderProgram, 2, (shadowmapIndirectionList[lightIdx] * (6 * SHADOW_MAP_SIZE_SQD)) + (face * SHADOW_MAP_SIZE_SQD));
        glProgramUniformMatrix4fv(voxen_GL_Comms.shadowmapsShaderProgram, 1, 1, GL_FALSE, (float*)lightViewProj[lightIdx][face]);
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

static inline void sift_up(LightCandidate* h, int idx) {
    while (idx > 0) {
        int p = (idx - 1) >> 1;
        if (h[p].score <= h[idx].score) break;
        LightCandidate t = h[p];
        h[p] = h[idx];
        h[idx] = t;
        idx = p;
    }
}

static inline void sift_down(LightCandidate* h, int size, int idx) {
    for (;;) {
        int l = (idx << 1) + 1;
        int r = l + 1;
        int s = idx;
        if (l < size && h[l].score < h[s].score) s = l;
        if (r < size && h[r].score < h[s].score) s = r;
        if (s == idx) break;
        LightCandidate t = h[idx];
        h[idx] = h[s];
        h[s] = t;
        idx = s;
    }
}

uint32_t numShadowsCouldRender = 0;

void RenderShadowmaps(void) {
    largestNearbyMeshCount = 0;
    glUseProgram(voxen_GL_Comms.shadowmapsClearShaderProgram);
    GLuint groupX_shadClear = (TOTAL_SHADOWMAP_PIXELS + 31) / 32;
    glDispatchCompute(groupX_shadClear,1, 1);
    shadowDrawCallsRenderedThisFrame = 0;
    memset(shadowmapIndirectionList, MAX_SHADOWMAPS + 1, loadedLights * sizeof(uint32_t));
    glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
    glUseProgram(voxen_GL_Comms.shadowmapsShaderProgram);
    glProgramUniform1i(voxen_GL_Comms.shadowmapsShaderProgram, 4, (int32_t)SHADOW_MAP_SIZE);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glBindVertexArray(voxen_GL_Comms.vao_chunk);
    LightCandidate candidates[MAX_SHADOWMAPS];
    uint8_t heap_size = 0;
    numShadowsCouldRender = 0;
    for (uint16_t i = 0; i < loadedLights; ++i) { // Collect candidates: only lights that are enabled, within FAR_PLANE, and in PVS
        uint32_t litIdx = i * LIGHT_DATA_SIZE;
        float intensity = lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY];
        float range =  lights[litIdx + LIGHT_DATA_OFFSET_RANGE];
        if (range > 10.0f) continue;
        if (intensity < 0.1f) continue;
        
        float thresh = 0.018f;
        float luminosity = (intensity / (range * range));
        if (luminosity < thresh) continue;
        
        float lightPosX = lights[litIdx + LIGHT_DATA_OFFSET_POSX];
        float lightPosY = lights[litIdx + LIGHT_DATA_OFFSET_POSY];
        float lightPosZ = lights[litIdx + LIGHT_DATA_OFFSET_POSZ];
        float dx = lightPosX - instances[PLAYER1].position.x;
        float dy = lightPosY - instances[PLAYER1].position.y;
        float dz = lightPosZ - instances[PLAYER1].position.z;
        float distSqrd = dx * dx + dy * dy + dz * dz;
        if (distSqrd >= 2500.0f) continue; // shadowDistance of 50.0f

        int lightCellIdx = cellIndexForLight[i];
        bool inPVS = (gridCellStates[lightCellIdx] & CELL_VISIBLE);
        if (!inPVS) {
            int x = cellIndexForLightX[i];
            int z = cellIndexForLightZ[i];
            int r = vfloor(range * (1.0f / WORLDCELL_WIDTH_F)); // 1 / 2.56f
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
        float dotResult = dot(dx, dy, dz, cam_forwardx, cam_forwardy, cam_forwardz);
        if (dotResult > 0.5f) score *= 8.0f; // Favor lights in player's view cone
        else if (dotResult > 0.0f) score *= 4.0f; // Favor lights in player's view cone
        
        LightCandidate c = { i, distSqrd, score };
        if (heap_size < MAX_SHADOWMAPS) {
            candidates[heap_size] = c;
            sift_up(candidates, heap_size);
            heap_size++;
        } else if (c.score < candidates[0].score) {
            candidates[0] = c;
            sift_down(candidates, MAX_SHADOWMAPS, 0);
        }
        numShadowsCouldRender++;
    }

    for (int a = 1; a < heap_size; a++) {
        LightCandidate x = candidates[a];
        int b = a - 1;
        while (b >= 0 && candidates[b].score > x.score) { candidates[b+1] = candidates[b]; b--; }
        candidates[b+1] = x;
    }
    
    uint32_t numToRender = vmin(numShadowsCouldRender, MAX_SHADOWMAPS);
    for (uint32_t c = 0; c < numToRender; ++c) { // Render top MAX_SHADOWMAPS candidates
        uint16_t lightIdx = candidates[c].index;
        uint32_t slot = shadowDrawCallsRenderedThisFrame;
        shadowmapIndirectionList[lightIdx] = slot;
        RenderShadowmap(lightIdx);
        shadowDrawCallsRenderedThisFrame++;
    }

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT);
    glViewport(0, 0, voxen_Settings.ScreenWidth, voxen_Settings.ScreenHeight);
    glNamedBufferData(shadowMapsIndirectionID, loadedLights * sizeof(uint32_t), shadowmapIndirectionList, GL_DYNAMIC_DRAW);
}

// ============================================================================
// UI Rendering and Text
__attribute__((pure)) float GetScreenRelativeX(float percentage) { return (float)voxen_Settings.ScreenWidth * percentage; }
__attribute__((pure)) float GetScreenRelativeY(float percentage) { return (float)voxen_Settings.ScreenHeight * percentage; }

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
void RenderUIImages(void) {
    if (uiImageCount == 0) return;

    glUseProgram(voxen_GL_Comms.chunkShaderProgram);
    glBindVertexArray(voxen_GL_Comms.textVAO);
    glProgramUniform1ui(voxen_GL_Comms.chunkShaderProgram, 3, 1u); // isUI true
    glProgramUniform1ui(voxen_GL_Comms.chunkShaderProgram, 17, 1u); // unlit is true
    glProgramUniformMatrix4fv(voxen_GL_Comms.chunkShaderProgram, 2, 1, GL_FALSE, uiOrthoProjection);
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
            glUniform1ui(1, 0);
            glUniform1ui(18, currentTex);
            glUniform1ui(19, 0);
            glUniform1ui(20, 0);
            glNamedBufferData(voxen_GL_Comms.textVBO, vertexCount * 30 * sizeof(float), uiImageVertexData, GL_DYNAMIC_DRAW);
            glDrawArrays(GL_TRIANGLES, 0, vertexCount * 6);
            drawCallsRenderedThisFrame++;
            uiImageDrawCallsRenderedThisFrame++;
            verticesRenderedThisFrame += vertexCount * 6;
        }

        start = end;
    }
}

__attribute__((pure)) bool CursorIsOverBounds(float startX, float endX, float startY, float endY) {
    return (   cursorPosition_x >= startX && cursorPosition_x <= endX     // 0 == left
            && cursorPosition_y >= endY   && cursorPosition_y <= startY); // 0 == top
}

float textVertexData[8192]; // Reusable buffer for text vertices.  Most text only needs ~3000
void RenderFormattedText(float x, float y, float z, uint32_t color, uint8_t fontID, const char* format, ...) {
    va_list args;
    va_start(args, format); vsnprintf(uiTextBuffer, TEXT_BUFFER_SIZE, format, args); va_end(args);
    glUseProgram(voxen_GL_Comms.textShaderProgram);
    glProgramUniformMatrix4fv(voxen_GL_Comms.textShaderProgram, 0, 1, GL_FALSE, uiOrthoProjection);
    glProgramUniform4f(voxen_GL_Comms.textShaderProgram, 3, textColors[color].r, textColors[color].g, textColors[color].b, textColors[color].a);
    if (fontID == FONT_STOPD) glBindTextureUnit(6, fontAtlasTexStopD);
    else glBindTextureUnit(6, fontAtlasTex);
    
    glProgramUniform2f(voxen_GL_Comms.textShaderProgram, 4, 1.0f / (float)FONT_ATLAS_SIZE, 1.0f / (float)FONT_ATLAS_SIZE);
    glProgramUniform1ui(voxen_GL_Comms.textShaderProgram, 2, fontID);
    glProgramUniform1i(voxen_GL_Comms.textShaderProgram, 1, 6); // textTexture sampler2D
    glBindVertexArray(voxen_GL_Comms.textVAO);
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
        glNamedBufferData(voxen_GL_Comms.textVBO, vertexCount * 30 * sizeof(float), textVertexData, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount * 6);
        drawCallsRenderedThisFrame++;
        textDrawCallsRenderedThisFrame++;
        verticesRenderedThisFrame += vertexCount * 6;
    }
}

void RenderLoadingProgress(int32_t offset, const char* format, ...) { // Only adds 0.01secs to game startup time.
    glUseProgram(voxen_GL_Comms.imageBlitShaderProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, voxen_GL_Comms.inputImageID);
    glProgramUniform1i(voxen_GL_Comms.imageBlitShaderProgram, 27, 0); // Texture 0 for the rendered geometry color buffer
    glBindVertexArray(voxen_GL_Comms.quadVAO);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    drawCallsRenderedThisFrame++;
    verticesRenderedThisFrame += 4;
    glBindTextureUnit(0, 0);
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    RenderFormattedText(voxen_Settings.ScreenWidth / 2 - offset, voxen_Settings.ScreenHeight / 2 - 5, UI_LAYER_5, TEXT_WHITE, FONT_NORMAL, buffer);
    glEnable(GL_DEPTH_TEST);
    glfwSwapBuffers(voxen_globalContext.window);
}

void CenterStatusPrint(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    statusTextLengthWithoutNullTerminator = vsnprintf(statusText, TEXT_BUFFER_SIZE, fmt, args);
    va_end(args);
    DualLog("%s\n",statusText);
    statusTextDecayFinished = get_time() + 2.5; // 2.5 second decay time before text dissappears.
}
// ============================================================================
void InitializePlayer(uint16_t playerIdx) { // Just setting the things that are nonzero
    instances[playerIdx].index = 767;
    instances[playerIdx].position.x = 10.52f; // Start Actual: Puts player on Medical Level in actual game start position
    instances[playerIdx].position.y = -43.792f + 0.84f; // Added 0.84f for cam offset from center
    instances[playerIdx].position.z = 20.2908f;
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
    pauseRelativeTime = 0.0;
}

void InitializeEnvironment(void) {
    double init_start_time = get_time();
    if (!glfwInit()) { DualLogError("GLFW initialization failed\n"); OS_Exit(1); }
    
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SRGB_CAPABLE, 0);
    glfwWindowHint(GLFW_RESIZABLE, 0);
    voxen_globalContext.window = glfwCreateWindow(voxen_Settings.ScreenWidth, voxen_Settings.ScreenHeight, "Voxen, the OpenGL Voxel Lit Engine", NULL, NULL);
    if (!voxen_globalContext.window) { DualLogError("glfwCreateWindow failed\n"); OS_Exit(1); }
        
    glfwMakeContextCurrent(voxen_globalContext.window);
    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) { DualLogError("Failed to initialize GLAD\n"); OS_Exit(1); }
    GLFWmonitor* target_monitor = glfwGetPrimaryMonitor();  // Use primary; or monitors[1] for second monitor, etc.
    if (target_monitor) {
        const GLFWvidmode* mode = glfwGetVideoMode(target_monitor);
        int mx, my;
        glfwGetMonitorPos(target_monitor, &mx, &my);
        int xpos = mx + (mode->width - voxen_Settings.ScreenWidth) / 2;
        int ypos = my + (mode->height - voxen_Settings.ScreenHeight) / 2;
        glfwSetWindowPos(voxen_globalContext.window, xpos, ypos);
        DualLog("Window positioned (windowed, centered) on monitor: %s (primary) at %d,%d\nUsing GLFW %s, ", glfwGetMonitorName(target_monitor), xpos, ypos,glfwGetVersionString());
    } else { DualLogError("GLFW Unable to obtain target monitor [primary]!\n"); OS_Exit(1); }
    
    glfwSetInputMode(voxen_globalContext.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    const GLubyte* version = glGetString(GL_VERSION);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    if (!version) { DualLogError("OpenGL support not found!\n"); OS_Exit(1);}
    
    DualLog("OpenGL Version: %s, ", (const char*)version);
    DualLog("GPU: %s", renderer ? (const char*)renderer : "unknown");
    OS_CPUInfo();
    DebugRAM("GL Buffer and shader setup");
    Input_Init(voxen_globalContext.window);
    glfwSwapInterval(voxen_Settings.Vsync ? 1 : 0);
    glFrontFace(GL_CCW); // Set triangle sorting order (GL_CW vs GL_CCW)
    CompileShaders();
    glCreateBuffers(1, &voxen_GL_Comms.quadVBO);
    float quadBlit_vertices[] = { 1.0f, -1.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,    -1.0f,1.0f, 0.0f, 1.0f,   -1.0f, -1.0f, 0.0f, 0.0f }; // 4 verts, 4 floats each pos.xy, uv.xy
    glNamedBufferData(voxen_GL_Comms.quadVBO, sizeof(quadBlit_vertices), quadBlit_vertices, GL_STATIC_DRAW);
    glCreateVertexArrays(1, &voxen_GL_Comms.quadVAO);
    glVertexArrayAttribFormat(voxen_GL_Comms.quadVAO, 0, 2, GL_FLOAT, GL_FALSE, 0); // DSA: Set position format
    glVertexArrayAttribFormat(voxen_GL_Comms.quadVAO, 1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float)); // DSA: Set texcoord format
    glVertexArrayVertexBuffer(voxen_GL_Comms.quadVAO, 0, voxen_GL_Comms.quadVBO, 0, 4 * sizeof(float)); // DSA: Link VBO to VAO
    for (uint8_t i = 0; i < 2; i++) { glVertexArrayAttribBinding(voxen_GL_Comms.quadVAO, i, 0); glEnableVertexArrayAttrib(voxen_GL_Comms.quadVAO, i); }

    glCreateVertexArrays(1, &voxen_GL_Comms.vao_chunk);
    glVertexArrayAttribFormat(voxen_GL_Comms.vao_chunk, 0, 3, GL_FLOAT, GL_FALSE, 0); // Position (vec3)
    glVertexArrayAttribFormat(voxen_GL_Comms.vao_chunk, 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float)); // Normal (vec3)
    glVertexArrayAttribFormat(voxen_GL_Comms.vao_chunk, 2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float)); // Tex Coord (vec2)
    for (uint8_t i = 0; i < 3; i++) { glVertexArrayAttribBinding(voxen_GL_Comms.vao_chunk, i, 0); glEnableVertexArrayAttrib(voxen_GL_Comms.vao_chunk, i); }
    
    glCreateBuffers(1, &voxen_GL_Comms.textVBO);
    glCreateVertexArrays(1, &voxen_GL_Comms.textVAO);    
    glVertexArrayAttribFormat(voxen_GL_Comms.textVAO, 0, 3, GL_FLOAT, GL_FALSE, 0); // pos (x,y,z) 4 floats per vertex, stride = 4*sizeof(float)
    glVertexArrayAttribFormat(voxen_GL_Comms.textVAO, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float));  // uv (s,t)
    glVertexArrayVertexBuffer(voxen_GL_Comms.textVAO, 0, voxen_GL_Comms.textVBO, 0, 5 * sizeof(float));
    for (uint8_t i = 0; i < 2; i++) { glVertexArrayAttribBinding(voxen_GL_Comms.textVAO, i, 0); glEnableVertexArrayAttrib(voxen_GL_Comms.textVAO, i); }

    GenerateAndBindTexture(&voxen_GL_Comms.inputImageID,             GL_RGBA8, voxen_Settings.ScreenWidth, voxen_Settings.ScreenHeight,            GL_RGBA, GL_UNSIGNED_BYTE, GL_TEXTURE_2D); // Lit Raster
    GenerateAndBindTexture(&voxen_GL_Comms.inputWorldPosID,        GL_RGBA32F, voxen_Settings.ScreenWidth, voxen_Settings.ScreenHeight,            GL_RGBA,         GL_FLOAT, GL_TEXTURE_2D); // Raster World Positions
    GenerateAndBindTexture(&voxen_GL_Comms.inputDepthID, GL_DEPTH_COMPONENT32, voxen_Settings.ScreenWidth, voxen_Settings.ScreenHeight, GL_DEPTH_COMPONENT,         GL_FLOAT, GL_TEXTURE_2D); // Raster Depth
    GenerateAndBindTexture(&voxen_GL_Comms.inputSpecID,              GL_RGBA8, voxen_Settings.ScreenWidth, voxen_Settings.ScreenHeight,            GL_RGBA, GL_UNSIGNED_BYTE, GL_TEXTURE_2D); // Specular Colors
    glGenTextures(1, &voxen_GL_Comms.outputImageID);
    glBindTexture(GL_TEXTURE_2D, voxen_GL_Comms.outputImageID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,  voxen_Settings.ScreenWidth / SSR_RES,  voxen_Settings.ScreenHeight / SSR_RES, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glGenFramebuffers(1, &voxen_GL_Comms.gBufferFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, voxen_GL_Comms.gBufferFBO);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, voxen_GL_Comms.inputImageID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, voxen_GL_Comms.inputWorldPosID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, voxen_GL_Comms.inputSpecID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, voxen_GL_Comms.inputDepthID, 0);
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, drawBuffers);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) DualLogError("Framebuffer incomplete: Error code %d\n", status);
    glBindImageTexture(0, voxen_GL_Comms.inputImageID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8); // Main Rendered Color
    glBindImageTexture(1, voxen_GL_Comms.inputWorldPosID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F); // World Position XYZ 32bit, Normal XYZ 8bit, 8bits empty
    glBindImageTexture(2, voxen_GL_Comms.inputSpecID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8); // Specular
    //                 3 = depth
    glBindImageTexture(4, voxen_GL_Comms.outputImageID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8); // SSR result
    
    glActiveTexture(GL_TEXTURE3); // Match binding = 3 in shader
    glBindTexture(GL_TEXTURE_2D, voxen_GL_Comms.inputDepthID);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, voxen_GL_Comms.outputImageID);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    DebugRAM("setup gbuffer end");
    
    UpdateScreenSize();
    InitFontAtlasses();
    DebugRAM("after InitFontAtlasses");
    
    Input_MouselookApply(); // Input
    InitializeAudio(); // Audio
    malloc_trim(0);
    DebugRAM("after InitializeAudio");
    
    LoadTextForLanguage(voxen_Settings.Language);
    DebugRAM("LoadTextForLanguage end");
    
    LoadLogTextForLanguage(voxen_Settings.Language);
    DebugRAM("LoadLogTextForLanguage end");
    
    ParseGameData();
    DebugRAM("ParseGameData end");
    
    glfwSetWindowTitle(voxen_globalContext.window,global_modname);
    int fp = OS_OpenReadonly("./Textures/UI/menudot1.png");
    int windowIconFileSize = OS_FileSize(fp);
    uint8_t* file_buffer = OS_AllocateFileBackedRAMReadonly(windowIconFileSize, fp, "./Textures/UI/menudot1.png");
    OS_Close(fp);
    int w = 1, h = 1;
    stbi__arena_init();
    unsigned char* pixels = stbi_load_from_memory(file_buffer, windowIconFileSize, &w, &h);
    if (!pixels) { DualLogError("Failed to load icon: ./Textures/UI/menudot1.png\n"); OS_Exit(1); }
    
    GLFWimage image;
    image.width  = w;
    image.height = h;
    image.pixels = pixels;
    glfwSetWindowIcon(voxen_globalContext.window, 1, &image);
    OS_MemoryAdviseDontNeed(pixels, w * h * 4);
    file_buffer = OS_DeallocateRAM(file_buffer, windowIconFileSize);
    stbi__arena_base = OS_DeallocateRAM(stbi__arena_base, STBI_ARENA_SIZE);
    DebugRAM("after freeing window bar icon");
    DualLog("GL buffers, FBO, fonts, audio, localization, and window init took %f secs\n", get_time() - init_start_time);
    LoadTextures(); // Sequential due to GPU transfers
    LoadModels(); // Sequential due to GPU transfers
    LoadEntities(); // Had a note to do this after textures and models, didn't seem necessary but giving it a thread didn't help init times.
    lightsID = SetupSSBO(lightsID, 19, LIGHT_COUNT * LIGHT_DATA_SIZE * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    voxelLightListIndicesID = SetupSSBO(voxelLightListIndicesID, 26, VOXEL_COUNT * 2 * sizeof(uint32_t), NULL, GL_DYNAMIC_DRAW);
    voxelLightListsRawID = SetupSSBO(voxelLightListsRawID, 27,  VOXEL_COUNT * 4 * sizeof(uint32_t), NULL, GL_DYNAMIC_DRAW);
    float mat[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    memcpy(&modelMatrices[0], mat, 16 * sizeof(float)); // Null instance matrix used for UI
    matricesBuffer = SetupSSBO(matricesBuffer, 11, INSTANCE_COUNT * 16 * sizeof(float), modelMatrices, GL_DYNAMIC_DRAW);
    shadowMapsIndirectionID = SetupSSBO(shadowMapsIndirectionID, 8, LIGHT_COUNT * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
//     play_mp3("./Audio/music/TITLOOP-00_menu.mp3",((float)voxen_Settings.VolumeMusic/100.0f) * 0.4f + 0.09f,1500);
    NewGame(); // TODO: Do this from menu not immediately lol
    DebugRAM("InitializeEnvironment end");
}

typedef struct {
    uint16_t index;
    float depth;
} DepthSort;

static inline void ds_swap(DepthSort* a, DepthSort* b) {
    DepthSort t = *a; *a = *b; *b = t;
}

/* dir = +1.0 => ascending (small -> large)
   dir = -1.0 => descending (large -> small) */
static inline int ds_cmp_sign(const DepthSort* a, const DepthSort* b, float dir) {
    float d = (a->depth - b->depth) * dir;
    if (d > 0.0f) return 1;
    if (d < 0.0f) return -1;
    return 0;
}

static void ds_insertion(DepthSort* arr, int n, float dir) {
    int i;
    for (i = 1; i < n; ++i) {
        DepthSort v = arr[i];
        int j = i - 1;
        while (j >= 0 && ds_cmp_sign(&v, &arr[j], dir) < 0) {
            arr[j + 1] = arr[j];
            --j;
        }
        arr[j + 1] = v;
    }
}

static void ds_heapify_one(DepthSort* arr, int n, int i, float dir) {
    for (;;) {
        int l = (i << 1) + 1;
        int r = l + 1;
        int best = i;
        if (l < n && ds_cmp_sign(&arr[l], &arr[best], dir) > 0) best = l;
        if (r < n && ds_cmp_sign(&arr[r], &arr[best], dir) > 0) best = r;
        if (best == i) return;
        ds_swap(&arr[i], &arr[best]);
        i = best;
    }
}

static void ds_heapsort(DepthSort* arr, int n, float dir) {
    int i;
    if (n <= 1) return;
    for (i = (n >> 1) - 1; i >= 0; --i) ds_heapify_one(arr, n, i, dir);
    for (i = n - 1; i > 0; --i) {
        ds_swap(&arr[0], &arr[i]);
        ds_heapify_one(arr, i, 0, dir);
    }
}

static void ds_introsort(DepthSort* arr, int n, float dir) {
    if (n <= 1) return;

    /* maxDepth = floor(log2(n)) * 2 is a common choice; we compute log2(n) */
    int maxDepth = 0;
    {
        int t = n;
        while (t > 1) { t >>= 1; maxDepth++; }
        maxDepth = maxDepth * 2;
    }

    /* manual stack frame */
    typedef struct { int lo, hi, depth; } Frame;
    Frame stk[64]; /* 64 is plenty for n ~= 2300 */
    int sp = 0;
    stk[sp++] = (Frame){ 0, n - 1, maxDepth };

    while (sp) {
        Frame f = stk[--sp];
        int lo = f.lo, hi = f.hi;
        int depth = f.depth;

        int count = hi - lo + 1;
        if (count <= 16) {
            ds_insertion(arr + lo, count, dir);
            continue;
        }

        if (depth <= 0) {
            ds_heapsort(arr + lo, count, dir);
            continue;
        }

        /* median-of-three to choose pivot */
        int mid = lo + ((hi - lo) >> 1);
        if (ds_cmp_sign(&arr[mid], &arr[lo], dir) < 0) ds_swap(&arr[mid], &arr[lo]);
        if (ds_cmp_sign(&arr[hi],  &arr[lo], dir) < 0) ds_swap(&arr[hi],  &arr[lo]);
        if (ds_cmp_sign(&arr[mid], &arr[hi], dir) < 0) ds_swap(&arr[mid], &arr[hi]);

        float pivot = arr[mid].depth;

        int i = lo, j = hi;
        while (i <= j) {
            while ((arr[i].depth - pivot) * dir < 0.0f) i++;
            while ((arr[j].depth - pivot) * dir > 0.0f) j--;
            if (i <= j) {
                ds_swap(&arr[i], &arr[j]);
                i++; j--;
            }
        }

        depth--;

        /* push larger partition first to keep stack small (tail recursion elimination) */
        if (i < hi) {
            stk[sp++] = (Frame){ i, hi, depth };
        }
        if (lo < j) {
            stk[sp++] = (Frame){ lo, j, depth };
        }
    }
}

#define REND_OPAQUE      1u
#define REND_DOUBLESIDED 2u
#define REND_TRANSPARENT 3u
DepthSort visibleInstances[INSTANCE_COUNT];
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
                               glDepthMask(GL_FALSE);
                               countsArray  =  modelTypeCountsTransparent;
                               offsetsArray = modelTypeOffsetsTransparent;
                               startOfNextType = loadedInstances - invalidModelIndexCount; break;
    }
    
    if (!countsArray || !offsetsArray) return;
    if (startOfNextType > loadedInstances) return;
    
    memset(visibleInstances,0,INSTANCE_COUNT * sizeof(DepthSort));
    for (uint16_t modelIdx = 0; modelIdx < loadedModels; modelIdx++) {
        if (countsArray[modelIdx] == 0) continue;

        uint16_t start = offsetsArray[modelIdx];
        if (start < 3) DualLogError("offsets for rendering wrong!\n");
        uint16_t count = countsArray[modelIdx];
        uint16_t visibleCount = 0;
        for (uint16_t i = start; i < start + count && i < startOfNextType; i++) { // Filter visible instances
            uint16_t instCellIdx = (uint16_t)cellIndexForInstance[i];
            float distSqrd = squareDistance3D(      instances[i].position.x,       instances[i].position.y,       instances[i].position.z,
                                              instances[PLAYER1].position.x, instances[PLAYER1].position.y, instances[PLAYER1].position.z);
            if (voxen_Settings.CullEnabled) {
                if (instCellIdx < ARRSIZE && !(gridCellStates[instCellIdx] & CELL_VISIBLE)) continue;
                if (distSqrd >= FAR_PLANE_SQUARED) continue;
            }
            
            visibleInstances[visibleCount].index = i;
            visibleInstances[visibleCount].depth = distSqrd;
            visibleCount++;
            instanceIsLODArray[i] = (distSqrd >= 1253.16f); // 35.4f * 35.4f
        }
        
        if (visibleCount == 0) continue;
        
        float dir = (type == REND_TRANSPARENT) ? -1.0f : +1.0f;
        ds_introsort(visibleInstances, visibleCount, dir);
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

void SetFog(void) {
    glProgramUniform1f(voxen_GL_Comms.chunkShaderProgram, 11, fogColorR * fogBaseDensityForLevel);
    glProgramUniform1f(voxen_GL_Comms.chunkShaderProgram, 12, fogColorG * fogBaseDensityForLevel);
    glProgramUniform1f(voxen_GL_Comms.chunkShaderProgram, 13, fogColorB * fogBaseDensityForLevel);
}

double timeSinceLastPhysicsTick = 0.0;
double last_topframe_time = 0.0;

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
    last_topframe_time = last_physics_time - 0.05;
    DebugRAM("prior to game loop");
    DualLog("Game Initialized in %f secs\n",get_time() - game_start_time);
    while(1) {
        current_time = get_time();
        double frame_time = current_time - last_topframe_time;
        last_topframe_time = current_time;
        if (!gamePaused) pauseRelativeTime += frame_time;
        
        // Handle Berserk Effect for Compositing Shader
        float berserkTimeRemainingNormalized = berserkFinished > 0.0001f ? (berserkFinished - (float)pauseRelativeTime) / PATCH_TIME_BERSERK : 0.0f;
        if (berserkFinished < (float)pauseRelativeTime && berserkFinished > 0.0001f) berserkFinished = berserkTimeRemainingNormalized = 0.0f;
        InputClearRisingAndFallingEdges();
        glfwPollEvents();
        if (glfwWindowShouldClose(voxen_globalContext.window)) EnqueueEvent(EV_QUIT,EV_INT_FIELD_UNUSED,EV_INT_FIELD_UNUSED,EV_FLOAT_FIELD_UNUSED,EV_FLOAT_FIELD_UNUSED);
        timeSinceLastPhysicsTick = pauseRelativeTime - last_physics_time;
        if (!log_playback && !gamePaused && !menuActive) {
            last_physics_time = pauseRelativeTime;
            EnqueueEvent(EV_PHYSICS_TICK,EV_INT_FIELD_UNUSED,EV_INT_FIELD_UNUSED,EV_FLOAT_FIELD_UNUSED,EV_FLOAT_FIELD_UNUSED);
        }

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
            glNamedBufferData(matricesBuffer, loadedInstances * 16 * sizeof(uint32_t), modelMatrices, GL_DYNAMIC_DRAW);

            // 3. Light Updates
            UpdateDynamicLights();
            for (int i = 0; i < loadedLights; ++i) { if (lightDirty[i]) { UpdateVoxelLightLists(); break; } }
            if (voxen_Settings.Shadows > 0u) RenderShadowmaps();
            
            // 4. Raterized Geometry, Standard vertex + fragment rendering, but with special packing to minimize transfer data amounts
            glBindFramebuffer(GL_FRAMEBUFFER, voxen_GL_Comms.gBufferFBO);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the corner where last shadowmap wrote into  
            glUseProgram(voxen_GL_Comms.chunkShaderProgram);
            glProgramUniformMatrix4fv(voxen_GL_Comms.chunkShaderProgram, 2, 1, GL_FALSE, viewProj);
            glProgramUniform1ui(voxen_GL_Comms.chunkShaderProgram, 3, 0u); // isUI false
            glProgramUniform1f(voxen_GL_Comms.chunkShaderProgram, 8, worldMin_x);
            glProgramUniform1f(voxen_GL_Comms.chunkShaderProgram, 9, worldMin_z);
            glProgramUniform3f(voxen_GL_Comms.chunkShaderProgram, 10, instances[PLAYER1].position.x, instances[PLAYER1].position.y, instances[PLAYER1].position.z);
            glProgramUniform1ui(voxen_GL_Comms.chunkShaderProgram, 14, voxen_Settings.Reflections);
            glProgramUniform1ui(voxen_GL_Comms.chunkShaderProgram, 15, voxen_Settings.Shadows);
            glProgramUniform1ui(voxen_GL_Comms.chunkShaderProgram, 17, 0u); // unlit false
            glBindVertexArray(voxen_GL_Comms.vao_chunk);
            memset(instanceIsLODArray,true,INSTANCE_COUNT * sizeof(bool)); // All using lower detail LOD mesh.
            RenderInstances(REND_OPAQUE);      // Opaque, e.g. most objects and level geometry chunks
            RenderInstances(REND_DOUBLESIDED); // Double Sided, e.g. cyber panels and foliage and negative scaled objects
            RenderInstances(REND_TRANSPARENT); // Transparents, e.g. windows and beakers
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // 5. SSR (Screen Space Reflections)
            if ((debugView == 0 || debugView == 4) && voxen_Settings.Reflections > 0) {
                glUseProgram(voxen_GL_Comms.ssrShaderProgram);
                glUniformMatrix4fv(4, 1, GL_FALSE, viewProj);
                glUniform3f(3, instances[PLAYER1].position.x, instances[PLAYER1].position.y, instances[PLAYER1].position.z);
                GLuint groupX_ssr = ((voxen_Settings.ScreenWidth  / SSR_RES) + 31) / 32;
                GLuint groupY_ssr = ((voxen_Settings.ScreenHeight / SSR_RES) + 31) / 32;
                glDispatchCompute(groupX_ssr, groupY_ssr, 1);
                glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            }
        } else { // END !PAUSED BLOCK -------------------------------------------------
            glBindFramebuffer(GL_FRAMEBUFFER, 0); // Allow text to still render while paused
        }
        
        // 6. Render final meshes' results with full screen quad
        glUseProgram(voxen_GL_Comms.imageBlitShaderProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, voxen_GL_Comms.inputImageID);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, voxen_GL_Comms.outputImageID);
        glProgramUniform1i(voxen_GL_Comms.imageBlitShaderProgram, 6, 4); // outputImage texture sampler2D
        glProgramUniform1ui(voxen_GL_Comms.imageBlitShaderProgram, 17, (gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX) || currentLevel == 13);
        glProgramUniform1ui(voxen_GL_Comms.imageBlitShaderProgram, 18, (gridCellStates[playerCellIdx] & CELL_SEES_SUN) && currentLevel != 13);
        glProgramUniform1ui(voxen_GL_Comms.imageBlitShaderProgram, 19, ((currentLevel >= 10 && currentLevel < 13) ? 1u : 0u) && (gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX));
        uint32_t shieldOnType = 0u; // No shield green tint.
        if (questData.ShieldActivated) {
            if (currentLevel == 6 || currentLevel == 7) shieldOnType = 2u; // Shielding only below player for lower levels.
            else if (currentLevel <= 5) shieldOnType = 1u; // Shielding everywhere as levels fully within shield.
        }
        
        glProgramUniform1f(voxen_GL_Comms.imageBlitShaderProgram, 4, worldMin_x);
        glProgramUniform1f(voxen_GL_Comms.imageBlitShaderProgram, 5, worldMin_z);
        glProgramUniform1ui(voxen_GL_Comms.imageBlitShaderProgram, 7, voxen_Settings.Reflections);
        glProgramUniform1ui(voxen_GL_Comms.imageBlitShaderProgram, 8, voxen_Settings.AntiAliasing);
        glProgramUniform1f(voxen_GL_Comms.imageBlitShaderProgram, 9, berserkTimeRemainingNormalized);
        glProgramUniform1f(voxen_GL_Comms.imageBlitShaderProgram, 10, berserkSeedTime);
        glProgramUniform1ui(voxen_GL_Comms.imageBlitShaderProgram, 11, voxen_Settings.Brightness);
        glUniform3f(12, deg2rad(cam_yaw), deg2rad(cam_pitch), deg2rad(cam_roll));
        glProgramUniform3f(voxen_GL_Comms.imageBlitShaderProgram, 13, instances[PLAYER1].position.x, instances[PLAYER1].position.y, instances[PLAYER1].position.z);
        glProgramUniform1f(voxen_GL_Comms.imageBlitShaderProgram, 14, voxen_Settings.FOV);
        glProgramUniform1f(voxen_GL_Comms.imageBlitShaderProgram, 15, (float)pauseRelativeTime * 0.1f);
        glProgramUniform1f(voxen_GL_Comms.imageBlitShaderProgram, 16, aspect3D);
        glProgramUniform1ui(voxen_GL_Comms.imageBlitShaderProgram, 20, shieldOnType);
        glProgramUniform1ui(voxen_GL_Comms.imageBlitShaderProgram, 22, voxen_Settings.Shadows);
        glUniformMatrix4fv(24, 1, GL_FALSE, viewProj);
        glUniformMatrix3fv(25, 1, GL_FALSE, invViewRot);
        glProgramUniform1i(voxen_GL_Comms.imageBlitShaderProgram, 27, 0); // Texture 0 for the rendered geometry color buffer
        glProgramUniform1f(voxen_GL_Comms.imageBlitShaderProgram, 28, GetPainStatic());
        Color painStaticColor = GetPainStaticColor();
        glProgramUniform3f(voxen_GL_Comms.imageBlitShaderProgram, 29, painStaticColor.r, painStaticColor.g, painStaticColor.b);
        glBindVertexArray(voxen_GL_Comms.quadVAO);
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        drawCallsRenderedThisFrame++;
        verticesRenderedThisFrame += 4;

        // 7. UI
        uint32_t drawCallsNormal = drawCallsRenderedThisFrame;
        float screenCenterX = (float)voxen_Settings.ScreenWidth / 2;
        float screenCenterY = (float)voxen_Settings.ScreenHeight / 2;
        float lineSpacing = GetScreenRelativeY(genericTextHeightFac);
        glEnable(GL_BLEND);
        glClear(GL_DEPTH_BUFFER_BIT); // Clear main FBO.  glClearBufferfv was actually SLOWER!  2nd Clear needed or UI dissappears/flickers!!
        glDepthMask(GL_TRUE); // GL_TRUE Fixes z sorting unless it has alpha, GL_FALSE Fixes alpha rendering of text, but makes the z sort not work for some reason.
        glDisable(GL_CULL_FACE);
        
        // Cursor
        uint16_t cursorTexture = 1260;
        if (gamePaused || menuActive) cursorTexture = 1261;
        float cursorSize = (float)voxen_Settings.ScreenWidth * CURSOR_SCREEN_PERCENTAGE;
        float cursorHalfSize = cursorSize * 0.5f;
        if (CursorVisible()) AddUIImage(cursorPosition_x - cursorHalfSize, cursorPosition_y - cursorHalfSize, UI_LAYER_TOP, cursorSize, cursorSize, cursorTexture);
        else AddUIImage(screenCenterX - cursorHalfSize, screenCenterY - cursorHalfSize, UI_LAYER_TOP, cursorSize, cursorSize, cursorTexture);
        
        float shootModeWidth = GetScreenRelativeX(0.01639f), shootModeHeight = GetScreenRelativeX(0.01639f);
        float shootModePos_x = GetScreenRelativeX(0.5f) - (shootModeWidth * 0.5f);
        float shootModePos_y = 0.0f;
        if (!gamePaused && !voxen_Cheats.noHUD) AddUIImage(shootModePos_x, shootModePos_y, UI_LAYER_0, shootModeWidth, shootModeHeight, 1020); // Shoot mode button
        if (inventoryMode) {
            if (CursorIsOverBounds(shootModePos_x, shootModePos_x + shootModeWidth, shootModePos_y + shootModeHeight, shootModePos_y)) {
                if (mouseButtons[GLFW_MOUSE_BUTTON_LEFT].released) {
                    inventoryMode = false;
                    cursorPosition_x = voxen_Settings.ScreenWidth / 2;
                    cursorPosition_y = voxen_Settings.ScreenHeight / 2;
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
        if (!voxen_Cheats.noHUD && voxen_Cheats.showLocation) RenderFormattedText(leftPad, debugTextStartY, UI_LAYER_1, TEXT_WHITE, FONT_NORMAL, "x: %.4f, y: %.4f, z: %.4f", (double)instances[PLAYER1].position.x, (double)instances[PLAYER1].position.y, (double)instances[PLAYER1].position.z);
        if (!voxen_Cheats.noHUD) RenderFormattedText(leftPad, debugTextStartY + (lineSpacing * 1), UI_LAYER_1, TEXT_WHITE, FONT_NORMAL, "timeSinceLastPhysicsTick: %.6f, numShadowsCouldRender: %u, playerCellIdx: %u, numCellsVisible: %u", timeSinceLastPhysicsTick, numShadowsCouldRender, playerCellIdx, numCellsVisible);
        if (!voxen_Cheats.noHUD) RenderFormattedText(leftPad, debugTextStartY + (lineSpacing * 2), UI_LAYER_1, TEXT_WHITE, FONT_NORMAL, "Player velocity: %.2f, %.2f, %.2f, accumulated force: %.2f, %.2f, %.2f, largestNearbyMeshCount: %u", (double)instances[PLAYER1].velocity.x, (double)instances[PLAYER1].velocity.y, (double)instances[PLAYER1].velocity.z, (double)instances[PLAYER1].accumulatedForce.x, (double)instances[PLAYER1].accumulatedForce.y, (double)instances[PLAYER1].accumulatedForce.z, largestNearbyMeshCount);
        if (voxen_Cheats.consoleActive) RenderFormattedText(leftPad, 0, UI_LAYER_1, TEXT_WHITE, FONT_NORMAL, "] %s",consoleEntryText);
        if (statusTextDecayFinished > (float)current_time) RenderFormattedText(leftPad + (voxen_Settings.ScreenWidth / 2) - 220, screenCenterY - GetScreenRelativeY(0.30f + (genericTextHeightFac * 2.0f)), UI_LAYER_1, TEXT_WHITE, FONT_NORMAL, "%s",statusText);

        glDepthMask(GL_TRUE);
        RenderUIImages();
        double time_now = get_time();
        if (voxen_Cheats.showFPS) {
            double thisFrameTime = (time_now - last_time) * 1000.0;
            double cpuFrameTime = cpuTime * 1000.0;
            uint8_t timingColor = TEXT_WHITE;
            if (vabs(thisFrameTime - cpuFrameTime) < 0.451) timingColor = TEXT_GREEN;
            if (thisFrameTime > 6.944444) timingColor = TEXT_RED;
            drawCallsRenderedThisFrame++; textDrawCallsRenderedThisFrame++; // Add two more for this text render ;)
            drawCallsRenderedThisFrame++; textDrawCallsRenderedThisFrame++;
            RenderFormattedText(leftPad, debugTextStartY - lineSpacing, UI_LAYER_5, timingColor, FONT_NORMAL, "ms: %.2f, CPU %.2f", thisFrameTime,cpuFrameTime);
            RenderFormattedText(leftPad + 230.0f, debugTextStartY - lineSpacing, UI_LAYER_5, TEXT_WHITE, FONT_NORMAL, "(FPS: %d, Worst: %d), Drwclls: %d [G %d UI %d Txt %d Shd %d] Vrts: %d Edit:%u",framesPerLastSecond,worstFPS,drawCallsRenderedThisFrame, drawCallsNormal, uiImageDrawCallsRenderedThisFrame, textDrawCallsRenderedThisFrame, shadowDrawCallsRenderedThisFrame, verticesRenderedThisFrame, voxen_Cheats.editMode);
        }

        last_time = time_now;
        if ((time_now - lastFrameSecCountTime) >= 1.00) {
            lastFrameSecCountTime = time_now;
            framesPerLastSecond = globalFrameNum - lastFrameSecCount;
            if (framesPerLastSecond < worstFPS && globalFrameNum > 2000) worstFPS = framesPerLastSecond; // After startup, keep track of worst framerate seen.
            lastFrameSecCount = globalFrameNum;
        }
        
        if (keyStates[GLFW_KEY_F12].pressed && time_now > screenshotTimeout) {
            Screenshot();
            screenshotTimeout = time_now + 1.0; // Prevent saving more than 1 per second for sanity purposes.
        }
        
        if (keyStates[GLFW_KEY_ESCAPE].pressed) gamePaused = !gamePaused;
        cpuTime = get_time() - current_time;
        glfwSwapBuffers(voxen_globalContext.window); // Present frame
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
