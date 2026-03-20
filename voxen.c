// voxen.c - A realtime OpenGL 4.3+ Game Engine for Citadel: The System Shock Fan Remake
// TODO: Multiview renders for sensaround
// TODO: Multiview renders for camera views
// TODO: Add camera view entities
// TODO: Proper physics
// TODO: Particle system
// TODO: Raycasts
// TODO: Voxel GI
// TODO: Save/Load system
// TODO: Directional lights for cyberspace
// TODO: Directional light for sunlight
// TODO: Directional light shadowmapping just for sunlight
// TODO: TARGET ID: Type-LevelNum(0#)EnemyNum(###),Example: Mutant-06003, EXCEPTIONS: Cyborg-00001 is Edward Diego
#include "os.h" // Operating System calls shim layer.
#include "gl.h"
#include "glfw3.h"
GLFWwindow* window;
#define MOD_INTEROP
#include "voxen.h"
#include "miniaudio.h"
#include "Shaders/shaders.h"
#include "credits.h"
GlobalContext Sys_Global = { .menuActive = true, .screenshotTimeout = 1.0, .creditsPageIndex = 1, .difficultyCombat = 2, .difficultyCyber = 2, .difficultyPuzzle = 2, .difficultyMission = 2, .deaths = 0, .worstFPS = UINT32_MAX, .cursorPosition_x = 680, .cursorPosition_y = 384, .aspect3D = 1.0f };
CheatsSystem Sys_Cheats = { .god = false, .noclip = true, .showLocation = true, .showFPS = true, .editMode = true };
RenderSystem Sys_Render; SystemUI Sys_UI;
OsFileHandle console_log_file = 0;
AutoSplitterData autoSplitter = { 0x1337133713371337, 0, false, 0 }; // Fore use with LiveSplit or other future speedrunner utilities for doing speedruns
uint8_t queuedLevelToLoad = 255u;
float modelMatrices[INSTANCE_COUNT * 16];
uint8_t dirtyInstances[INSTANCE_COUNT];
double berserkFinished;
float berserkSeedTime, cam_pitch, cam_yaw = 90.0f, cam_roll, fogColorR, fogColorG, fogColorB, fogBaseDensityForLevel;
float rasterPerspectiveProjection[16];
float shadowmapsPerspectiveProjection[16];
char uiTextBuffer[TEXT_BUFFER_SIZE];
float uiOrthoProjection[16];
float lights[LIGHT_COUNT * LIGHT_DATA_SIZE];
bool lightDirty[LIGHT_COUNT];
static float lightView[LIGHT_COUNT][6][4][4]; // Array of Array of 6 Arrays of 16 floats (matrix 4x4).  lightView[i][face][0 ... 15]
static float lightViewProj[LIGHT_COUNT][6][16]; // Array of Array of 6 Arrays of 16 floats (matrix 4x4).  lightViewProj[i][face][0 ... 15]
FrustumPlane lightFrustumPlanes[LIGHT_COUNT][6][6]; // Array of Array of 6 Arrays of FrustumPlane structs (four floats).  lightFrustumPlanes[i][face][.nx,.ny,, .nz, .d]
FrustumPlane playerFrustumPlanes[6];
uint16_t loadedLights, editModeSelection, editModeTestEntityDefinition = 0; // Test instance and its model index
float voxelMinCenterX, voxelMinCenterZ;
VoxenShadowSystem voxen_Shadow_System;
float lightMinIntensity[LIGHT_COUNT];
float lightMaxIntensity[LIGHT_COUNT];
bool lightOn[LIGHT_COUNT];
bool lightLerpOn[LIGHT_COUNT];
bool lightLerpUp[LIGHT_COUNT];
uint8_t lightCurrentStep[LIGHT_COUNT];
float lightLerpValue[LIGHT_COUNT];
float lightLerpTime[LIGHT_COUNT];
float lightLerpStepTime[LIGHT_COUNT];
float lightLerpStartTime[LIGHT_COUNT];
uint8_t lightIntervalStepsLength[LIGHT_COUNT];
float lightIntervalSteps[LIGHT_COUNT][30];
uint8_t lightIntervalStepIsLerpingLength[LIGHT_COUNT];
float intervalStepisLerping[LIGHT_COUNT][30];
bool lightCastsShadows[LIGHT_COUNT];
uint16_t headmountedLanternLight;
Vector3 lanternPos;
float lanternVersionBrightness[3] = { 0.875f, 1.4f, 1.75f };
uint16_t useableItemsFrobIcons[94];
uint16_t selfIdx;
uint16_t loadedTexturesMaxIndex;
bool doubleSidedTexture[MAX_VALID_TEXTURE];
bool transparentTexture[MAX_VALID_TEXTURE];
uint32_t drawCallsRenderedThisFrame;
uint32_t textDrawCallsRenderedThisFrame;
uint32_t uiImageDrawCallsRenderedThisFrame;
uint32_t shadowDrawCallsRenderedThisFrame;
uint32_t verticesRenderedThisFrame;
uint32_t drawCallsNormal;
#define MAX_CHANNELS 16
ma_sound wav_sounds[MAX_CHANNELS];
float wav_volumes[MAX_CHANNELS]; // Setting independent base sfx volume (e.g. dropped physics object hard or lightly volume, independent of position).
int32_t wav_count = 0;
ma_sound log_sound;
#define MENUPAD        1028
#define MENUPAD_HILITE 1029
MenuPages currentMenuPage = MenuPages_FrontPage;
bool returnToPause = false, fovSliderActive = false, gammaSliderActive = false, masterVolumeSliderActive = false, musicVolumeSliderActive = false, messageVolumeSliderActive = false, sfxVolumeSliderActive = false, enteringPlayerName = false;
uint8_t currentPlayerNameLength = 0;
int8_t currentMenuItem = 0, currentMenuTab = 0, menuItemCount = 4, menuTabCount = 1;
static bool resDropdownOpen = false;
static int resDropdownCount = 0;
typedef struct { int w, h, hz; } ResMode;
static ResMode resModes[8];
static int resSelectedIdx = 0;

typedef struct {
   unsigned short x0,y0,x1,y1; // coordinates of bbox in bitmap
   float xoff,yoff,xadvance;
   float xoff2,yoff2;
} stbtt_packedchar;

typedef struct {
   float x0,y0,s0,t0; // top-left
   float x1,y1,s1,t1; // bottom-right
} stbtt_aligned_quad;
void stbtt_GetPackedQuad(const stbtt_packedchar *chardata, int pw, int ph, int char_index, float *xpos, float *ypos, stbtt_aligned_quad *q, int align_to_integer);

// Logs both to log file and console, usage same as printf
static void DualLogMain(const char *prefix, const char *fmt, va_list args) {
    char buf[2048]; va_list copy;
    __builtin_va_copy(copy, args);
    StringFormatV(buf, sizeof(buf), fmt, copy);
    __builtin_va_end(copy);
    // Write to console (stdout / stderr)
    #ifdef WINDOWS
        OsFileHandle out = GetStdHandle((prefix && prefix[0] == '\033') ? (DWORD)-12 : (DWORD)-11);
        if (prefix) OS_RawWrite(out, prefix, GetStringLength(prefix));
        OS_RawWrite(out, buf, GetStringLength(buf));
    #else
        // Linux - write to stdout (fd 1) or stderr (fd 2)
        OsFileHandle out = (prefix && prefix[0] == '\033') ? 2 : 1;  // use stderr for colored warnings/errors
        if (prefix) { OS_RawWrite(out, prefix, GetStringLength(prefix)); OS_RawWrite(out,"\033[0m ", 5); }
        OS_RawWrite(out, buf, GetStringLength(buf));
    #endif
    // Write to console_log_file
    if (console_log_file != OS_INVALID_HANDLE) {
        if (prefix) { OS_Write(console_log_file, prefix, GetStringLength(prefix), "console.log"); OS_Write(console_log_file,"\033[0m ",5,"console.log"); }
        OS_Write(console_log_file, buf, GetStringLength(buf), "console.log");
    }
}

ENGINE_TO_MOD void DualLog(const char* fmt, ...) { va_list args; __builtin_va_start(args,fmt); DualLogMain(NULL,fmt,args); __builtin_va_end(args); }
ENGINE_TO_MOD void DualLogWarn(const char* fmt, ...) { va_list args; __builtin_va_start(args,fmt); DualLogMain("\033[1;38;5;208mWARN:",fmt,args); __builtin_va_end(args); }
ENGINE_TO_MOD void DualLogError(const char* fmt, ...) { va_list args; __builtin_va_start(args,fmt); DualLogMain("\033[1;31mERROR:",fmt,args); __builtin_va_end(args); }
void DualLogErrorWrapper(const char* fmt, ...) { va_list args; __builtin_va_start(args,fmt); DualLogError("%s",fmt,args); __builtin_va_end(args); }

static inline __attribute__((always_inline)) void LogShaderError(GLuint s, const char* name) { char er[512]; glGetShaderInfoLog(s, 512, NULL, er); DualLogError("%s Compilation Failed: %s\n", name, er); OS_Exit(1); }
static inline __attribute__((always_inline)) GLuint CompileShader(GLenum type, const char* source, const char* name) { GLuint s = glCreateShader(type); glShaderSource(s, 1, &source, NULL); glCompileShader(s); GLint ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok); if (!ok) LogShaderError(s, name); return s; }
static inline __attribute__((always_inline)) GLuint LinkProgram(GLuint* s, int32_t num, const char* name) { GLuint p = glCreateProgram(); for (int32_t i = 0; i < num; i++) { glAttachShader(p, s[i]); glDeleteShader(s[i]); } glLinkProgram(p); GLint ok; glGetProgramiv(p, GL_LINK_STATUS, &ok); if (!ok) LogShaderError(p, name); return p; }
GLuint CompileStandardShader(const char* vsrc, const char* fsrc, const char* name) { GLuint vertShader = CompileShader(GL_VERTEX_SHADER, vsrc, name); GLuint fragShader = CompileShader(GL_FRAGMENT_SHADER, fsrc, name); return LinkProgram((GLuint[]){vertShader, fragShader}, 2, name); }
GLuint CompileComputeShader(const char* src, const char* name) { GLuint computeShader = CompileShader(GL_COMPUTE_SHADER, src, name); return LinkProgram((GLuint[]){computeShader}, 1, name); }
void CompileShaders(void) {
    Sys_Render.depthPrepassShaderProgram       = CompileStandardShader(depthPrepassVertSrc, depthPrepassFragSrc, "Depth Prepass");
    Sys_Render.chunkShaderProgram       = CompileStandardShader(vertSrc, fragSrc, "Main");
    Sys_Render.debugUnlitShaderProgram  = CompileStandardShader(debugUnlitVertSrc, debugUnlitFragSrc, "Debug Unlit");
    Sys_Render.shadowmapsShaderProgram  = CompileStandardShader(shadowmapVertSrc, shadowmapFragSrc, "Shadowmaps");
    Sys_Render.textShaderProgram        = CompileStandardShader(textVertSrc, textFragSrc, "Text");
    Sys_Render.imageBlitShaderProgram   = CompileStandardShader(quadVertSrc, quadFragSrc, "Image Blit");
    Sys_Render.ssrShaderProgram         = CompileComputeShader(ssrComputeSrc, "SSR");
    Sys_Render.voxelUpdateShaderProgram     = CompileComputeShader(voxelUpdateComputeSrc, "Voxel Update");
    Sys_Render.shadowmapsClearShaderProgram = CompileComputeShader(shadowmapsClearComputeSrc, "Shadowmaps Clear");
}

GLuint SetupSSBO(GLuint* id, GLuint bindingIndex, GLsizeiptr size, const void* data, GLenum usage) {
    glGenBuffers(1, id); glBindBuffer(GL_SHADER_STORAGE_BUFFER, *id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, usage);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingIndex, *id); return *id;
}

// Generates View Matrix4x4 for Geometry Rasterizer Pass from camera world position + orientation
void mat4_lookat_from(float* m, Quaternion* camRotation, Vector3 eye) { // Kept around for light views for shadowmap cubemap faces.
    float x = camRotation->x, y = camRotation->y, z = camRotation->z, w = camRotation->w;
    float x2 = x * x, y2 = y * y, z2 = z * z;
    float xy = x * y, xz = x * z, yz = y * z;
    float wx = w * x, wy = w * y, wz = w * z;
    Vector3 right   = { 1.0f - 2.0f * (y2 + z2),        2.0f * (xy + wz),        2.0f * (xz - wy) };  // X+ (right)
    Vector3 up      = {        2.0f * (xy - wz), 1.0f - 2.0f * (x2 + z2),        2.0f * (yz + wx) };  // Y+ (up)
    Vector3 forward = {        2.0f * (xz + wy),        2.0f * (yz - wx), 1.0f - 2.0f * (x2 + y2) };  // Z+ (forward)
    m[0]  = right.x;   m[1]  = up.x;   m[2]  = -forward.x;// m[3]  = 0.0f;
    m[4]  = right.y;   m[5]  = up.y;   m[6]  = -forward.y;// m[7]  = 0.0f;
    m[8]  = right.z;   m[9]  = up.z;   m[10] = -forward.z;// m[11] = 0.0f;
    m[12] = -dot_vector3(right, eye); m[13] = -dot_vector3(up, eye); m[14] = dot_vector3(forward, eye); m[15] = 1.0f;
}

__attribute__((pure,always_inline)) bool SphereInFrustum(FrustumPlane* planes, Vector3 c, float radius) { for (int i=0;i<6;++i) { if ((dot_vector3(planes[i].normal,c) + planes[i].d) < -radius) return false; } return true; }

void ExtractFrustumPlanes(float* m, FrustumPlane* planes) {
    planes[0].normal.x = m[3]  + m[0];  planes[0].normal.y = m[7]  + m[4];  planes[0].normal.z = m[11] + m[8];  planes[0].d = m[15] + m[12]; // Left
    planes[1].normal.x = m[3]  - m[0];  planes[1].normal.y = m[7]  - m[4];  planes[1].normal.z = m[11] - m[8];  planes[1].d = m[15] - m[12]; // Right
    planes[2].normal.x = m[3]  + m[1];  planes[2].normal.y = m[7]  + m[5];  planes[2].normal.z = m[11] + m[9];  planes[2].d = m[15] + m[13]; // Bottom
    planes[3].normal.x = m[3]  - m[1];  planes[3].normal.y = m[7]  - m[5];  planes[3].normal.z = m[11] - m[9];  planes[3].d = m[15] - m[13]; // Top
    planes[4].normal.x = m[3]  + m[2];  planes[4].normal.y = m[7]  + m[6];  planes[4].normal.z = m[11] + m[10]; planes[4].d = m[15] + m[14]; // Near
    planes[5].normal.x = m[3]  - m[2];  planes[5].normal.y = m[7]  - m[6];  planes[5].normal.z = m[11] - m[10]; planes[5].d = m[15] - m[14]; // Far
    for (int i = 0; i < 6; i++) {
        float len = magnitude_vector3(planes[i].normal); if (len > 1e-6f) { planes[i].normal.x /= len; planes[i].normal.y /= len; planes[i].normal.z /= len; planes[i].d /= len; } // Normalize (could use normalize_vector3 but need len for d term of FrustumPlane).
    }
}

ENGINE_TO_MOD int32_t PosGetCellCoords(float pos_x, float pos_z) { return (PosGetCellCoordZ(pos_z) * WORLDX) + PosGetCellCoordX(pos_x); } // Clamped just above.

Quaternion cubemapOrientationQuaternion[6] = {
    {0.0f, 0.707106781f, 0.0f, 0.707106781f},  // +X: Right
    {0.0f, -0.707106781f, 0.0f, 0.707106781f}, // -X: Left
    {-0.707106781f, 0.0f, 0.0f, 0.707106781f}, // +Y: Up
    {0.707106781f, 0.0f, 0.0f, 0.707106781f},  // -Y: Down
    {0.0f, 0.0f, 0.0f, 1.0f},                  // +Z: Forward
    {0.0f, 1.0f, 0.0f, 0.0f}                   // -Z: Backward
};

bool lightInPVS[LIGHT_COUNT];
Vector3 lightsNewPosition[LIGHT_COUNT];
bool UpdateLights(bool* voxelsNeedUpdated) {
    // Update headmounted lantern
    int32_t lant = headmountedLanternLight * LIGHT_DATA_SIZE;
    bool infraredOn = /*(Sys_Global.inventoryPlayer1.hasHardware & HW_INF) && */(Sys_Global.inventoryPlayer1.hardwareIsActive & HW_INF);
    bool lanternOn = /*(Sys_Global.inventoryPlayer1.hasHardware & HW_LAN) && */(Sys_Global.inventoryPlayer1.hardwareIsActive & HW_LAN);
    if (lanternOn || infraredOn) {
        Vector3 lanternPosLast = lanternPos;
        lanternPos = (Vector3){Sys_Global.instances[PLAYER1].position.x + 0.04f, Sys_Global.instances[PLAYER1].position.y + 0.24f, Sys_Global.instances[PLAYER1].position.z + 0.04f};
        lightsNewPosition[headmountedLanternLight] = lanternPos;
        lights[lant + LIGHT_DATA_OFFSET_POSX] = lanternPos.x;
        lights[lant + LIGHT_DATA_OFFSET_POSY] = lanternPos.y;
        lights[lant + LIGHT_DATA_OFFSET_POSZ] = lanternPos.z;
        lights[lant + LIGHT_DATA_OFFSET_RANGE] = infraredOn ? 50.36f : 11.52f;
        lights[lant + LIGHT_DATA_OFFSET_INTENSITY] = lightMaxIntensity[headmountedLanternLight] = infraredOn ? 0.8f : lanternVersionBrightness[Sys_Global.inventoryPlayer1.hardwareVersionSetting[7]];
        lightCastsShadows[headmountedLanternLight] = !infraredOn;
        lightDirty[headmountedLanternLight] = !lightOn[headmountedLanternLight] || (vabs(lanternPosLast.x - lanternPos.x) + vabs(lanternPosLast.y - lanternPos.y) + vabs(lanternPosLast.z - lanternPos.z)) > 0.001f;
        lightOn[headmountedLanternLight] = true;
        lightCastsShadows[headmountedLanternLight] = true;
        lightInPVS[headmountedLanternLight] = true;
    } else { 
        lightOn[headmountedLanternLight] = false; 
        lightDirty[headmountedLanternLight] = false; 
        lights[lant + LIGHT_DATA_OFFSET_INTENSITY] = 0.0f;
    }
    
    for (uint16_t lightIdx = 0; lightIdx < loadedLights; ++lightIdx) { 
        uint32_t litIdx = lightIdx * LIGHT_DATA_SIZE;
        lights[litIdx + LIGHT_DATA_OFFSET_POSX] = lightsNewPosition[lightIdx].x;
        lights[litIdx + LIGHT_DATA_OFFSET_POSY] = lightsNewPosition[lightIdx].y;
        lights[litIdx + LIGHT_DATA_OFFSET_POSZ] = lightsNewPosition[lightIdx].z;
        Vector3 lightPos = (Vector3){ lights[litIdx + LIGHT_DATA_OFFSET_POSX], lights[litIdx + LIGHT_DATA_OFFSET_POSY], lights[litIdx + LIGHT_DATA_OFFSET_POSZ] };
        if (lightDirty[lightIdx]) { // Marked all as true at level load.
            *voxelsNeedUpdated = true;
            #pragma GCC unroll 6
            for (int j=0;j<6;++j) { // Update to new position
                mat4_lookat_from((float*)lightView[lightIdx][j], &cubemapOrientationQuaternion[j], lightPos);
                mul_mat4((float*)lightViewProj[lightIdx][j], shadowmapsPerspectiveProjection, (float*)lightView[lightIdx][j]);
                ExtractFrustumPlanes((float*)lightViewProj[lightIdx][j], lightFrustumPlanes[lightIdx][j]);
            }
        }
        
        uint16_t cellX = (uint16_t)clamp((int32_t)vfloor((lightPos.x - worldMin_x + CELLXHALF) / WORLDCELL_WIDTH_F), 0, WORLDX_0BASED);
        uint16_t cellZ = (uint16_t)clamp((int32_t)vfloor((lightPos.z - worldMin_z + CELLXHALF) / WORLDCELL_WIDTH_F), 0, WORLDX_0BASED);
        int lightCellIdx = (cellZ * WORLDX) + cellX;
        float range = lights[litIdx + LIGHT_DATA_OFFSET_RANGE];
        int r = vceil(range * (1.0f / WORLDCELL_WIDTH_F));
        lightInPVS[lightIdx] = (gridCellStates[lightCellIdx] & CELL_VISIBLE) || (lightIdx == headmountedLanternLight);
        if (!lightInPVS[lightIdx]) {
            for (int ix = cellX - r; ix <= (int)cellX + r; ++ix) {
                for (int iz = cellZ - r; iz <= (int)cellZ + r; ++iz) {
                    if (unlikely(!XZPairInBounds(ix, iz))) continue;
                    
                    int subIdx = iz * WORLDX + ix;
                    if (get_cull_bit(precomputedVisibleCellsFromHere, lightCellIdx * ARRSIZE + subIdx) && (gridCellStates[subIdx] & CELL_VISIBLE)) {
                        lightInPVS[lightIdx] = true;
                        break;
                    }
                }
            }
        }
    }
    
    if (!Sys_Global.gamePaused && !Sys_Global.menuActive) {
        for (int i=0;i<loadedLights;++i) { // Just lerps/flickers in intensity
            if (lightIntervalStepsLength[i] < 1) continue;
            
            int litIdx = i * LIGHT_DATA_SIZE;
            if (!lightOn[i]) { lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY] = lightMinIntensity[i]; continue; }

            if (lightLerpTime[i] < (float)Sys_Global.pauseRelativeTime) {
                lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY] = lightLerpUp[i] ? lightMaxIntensity[i] : lightMinIntensity[i]; // Pick target to lerp towards
                lightLerpUp[i] = !lightLerpUp[i];
                lightCurrentStep[i]++;
                if (lightCurrentStep[i] >= lightIntervalStepsLength[i]) lightCurrentStep[i] = 0; // Wrap and start over continuous looping
                lightLerpStepTime[i] = lightIntervalSteps[i][lightCurrentStep[i]];
                lightLerpTime[i] = (float)Sys_Global.pauseRelativeTime + lightLerpStepTime[i];
                lightLerpStartTime[i] = (float)Sys_Global.pauseRelativeTime;
            } else if (lightLerpOn[i]) {
                if (lightCurrentStep[i] < lightIntervalStepIsLerpingLength[i]) {
                    if (intervalStepisLerping[i][lightCurrentStep[i]]) {
                        lightLerpValue[i] = ((float)Sys_Global.pauseRelativeTime - lightLerpStartTime[i])/(lightLerpTime[i] - lightLerpStartTime[i]); // percent towards goal time
                        float lerpVal = lightLerpUp[i] ? (lightLerpValue[i]) : (1.0f - lightLerpValue[i]);
                        lightLerpValue[i] = lightMinIntensity[i] + ((lightMaxIntensity[i] - lightMinIntensity[i]) * lerpVal);
                        lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY] = lightLerpValue[i];
                    }
                }
            }
        }
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, Sys_Render.lightsID); glBufferData(GL_SHADER_STORAGE_BUFFER, loadedLights * LIGHT_DATA_SIZE * sizeof(float), lights, GL_DYNAMIC_DRAW);
    if (*voxelsNeedUpdated) {
        Vector3 p = Sys_Global.instances[PLAYER1].position;
        glUseProgram(Sys_Render.voxelUpdateShaderProgram);
        glUniform3f(5,p.x,p.y,p.z);
        glUniform1ui(6,(uint32_t)MAX_LIGHTS_PER_VOXEL);
        glDispatchCompute((512+31)/32,(512+31)/32,1);
    }
    
    return *voxelsNeedUpdated;
}

typedef struct {
    float depth;
    uint16_t index;
} DepthSort;

__attribute__((pure)) int32_t compareDepthSort(const void* a, const void* b) {
    float da = ((const DepthSort*)a)->depth;
    float db = ((const DepthSort*)b)->depth;
    return (db > da) - (db < da);
}

__attribute__((pure)) int32_t compareDepthSortInverted(const void* a, const void* b) {
    float da = ((const DepthSort*)a)->depth;
    float db = ((const DepthSort*)b)->depth;
    return (da > db) - (da < db);
}

// ============================================================================
// UI Rendering and Text
#define BASE_RES_X 1366.0f // Positions done in fixed int positions off base resolution, scaled against current resolution.
#define BASE_RES_Y 768.0f
float UIX(int16_t x) { return (float)x / BASE_RES_X; } // Pos or value as percent of 1366x768 resolution
float RelX(int16_t x) { return UIX(x) * (float)Sys_Settings.ScreenWidth; } // Pos or value in current resolution
float UIY(int16_t y) { return (float)y / BASE_RES_Y; }
float RelY(int16_t y) { return UIY(y) * (float)Sys_Settings.ScreenHeight; }

void RenderUIImage(int16_t x, int16_t y, int16_t width, int16_t height, uint32_t texIndex) {
    float xpos = RelX(x); float ypos = RelY(y);
    glEnable(GL_BLEND);
    glClear(GL_DEPTH_BUFFER_BIT); // Clear main FBO.  glClearBufferfv was actually SLOWER!  2nd Clear needed or UI dissappears/flickers!!
    glDisable(GL_CULL_FACE);
    glUseProgram(Sys_Render.chunkShaderProgram);
    glBindVertexArray(Sys_Render.textVAO);
    glUniform1ui(1,0);
    glUniform1ui(3,1u);  // isUI true
    glUniform1ui(17,1u); // unlit is true
    glUniform1ui(19,0);
    glUniform1ui(20,0);
    glUniformMatrix4fv(2,1,GL_FALSE,uiOrthoProjection);
    glBindBuffer(GL_ARRAY_BUFFER,Sys_Render.textVBO);
    float x1 = xpos + RelX(width);
    float y1 = ypos + RelY(height);
    float z = 0.0f;
    float vertices[30] = {xpos,y1,z,0.0f,0.0f,x1,ypos,z,1.0f,1.0f,x1,y1,z,1.0f,0.0f,xpos,y1,z,0.0f,0.0f,xpos,ypos,z,0.0f,1.0f,x1,ypos,z,1.0f,1.0f};
    glUniform1ui(18,texIndex);    
    glBufferData(GL_ARRAY_BUFFER,30 * sizeof(float),vertices,GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES,0,6);
    drawCallsRenderedThisFrame++; uiImageDrawCallsRenderedThisFrame++; verticesRenderedThisFrame += 6;    
    glBindBuffer(GL_ARRAY_BUFFER,0);
}

__attribute__((pure)) bool CursorIsOverBounds(float startX, float endX, float startY, float endY) {
    return   (Sys_Global.cursorPosition_x >= startX && Sys_Global.cursorPosition_x <= endX   /* 0 == left */
           && Sys_Global.cursorPosition_y >= endY && Sys_Global.cursorPosition_y <= startY); /* 0 == top */
}

Color textColors[] = {
    {         1.0f,         1.0f,          1.0f, 1.0f}, // 0 White                       TEXT_WHITE
    { 0.890196078f, 0.874509804f,          0.0f, 1.0f}, // 1 Yellow                      TEXT_YELLOW
    { 0.623529412f, 0.611764706f,          0.0f, 1.0f}, // 2 Dark Yellow (Yellow * 0.7f) TEXT_DARK_YELLOW
    { 0.372549020f, 0.654901961f,  0.168627451f, 1.0f}, // 3 Green                       TEXT_GREEN
    { 0.917647059f, 0.137254902f,  0.168627451f, 1.0f}, // 4 Red                         TEXT_RED
    {         1.0f, 0.498039216f,          0.0f, 1.0f}, // 5 Orange                      TEXT_ORANGE
    { 0.674509804f, 0.058823529f,  0.070588235f, 1.0f}, // 6 StopD Red                   TEXT_STOPD_RED
    { 0.941176471f, 0.282352941f,  0.298039216f, 1.0f}, // 7 StopD Red Highlight         TEXT_STOPD_RED_HIGHLIGHT
    { 0.909803922f, 0.203921569f,  0.219607843f, 1.0f}, // 8 StopD Red Pause Title       TEXT_STOPD_RED_PAUSETITLE
    { 0.470588235f, 0.721568627f,  0.172549020f, 1.0f}, // 9 Green Menu Title            TEXT_GREEN_MENU
    { 0.137254902f, 0.356862745f,  0.109803922f, 1.0f}, // 10 Green Menu Title Shadow    TEXT_GREEN_MENU_SHADOW
    { 0.239215686f, 0.466666667f,  0.129411765f, 1.0f}, // 11 Green Menu Title Glow      TEXT_GREEN_MENU_GLOW
    { 0.392156863f, 0.031372549f,  0.039215686f, 1.0f}  // 12 Red Menu Text Dark         TEXT_RED_MENU
};

float textVertexData[8192]; // Reusable buffer for text vertices.  Most text only needs ~3000
extern stbtt_packedchar fontPackedChar[MAX_GLYPHS];
extern stbtt_packedchar fontPackedCharStopD[MAX_GLYPHS];
__attribute__((pure)) int32_t CodepointToPackedIndex(int32_t codepoint, int fontID);
void RenderFormattedText(int16_t x, int16_t y, uint32_t color, uint8_t fontID, float scaleInput, const char * restrict format, ...) {
    float scale = scaleInput;// * UIY(Sys_Settings.ScreenHeight);
    va_list args;
    __builtin_va_start(args, format); StringFormatV(uiTextBuffer,TEXT_BUFFER_SIZE,format,args); __builtin_va_end(args);
    glUseProgram(Sys_Render.textShaderProgram);
    glUniformMatrix4fv(0, 1, GL_FALSE, uiOrthoProjection);
    glUniform4f(3, textColors[color].r, textColors[color].g, textColors[color].b, textColors[color].a);
    if (fontID == FONT_STOPD) glBindTextureUnit(6, fontAtlasTexStopD);
    else glBindTextureUnit(6, fontAtlasTex);
    
    glUniform2f(4, 1.0f / (float)FONT_ATLAS_SIZE, 1.0f / (float)FONT_ATLAS_SIZE);
    glUniform1ui(2, fontID);
    glUniform1i(1, 6); // textTexture sampler2D
    glBindVertexArray(Sys_Render.textVAO);
    size_t vertexCount = 0;
    const char* p = uiTextBuffer;
    float xpos = RelX(x), ypos = RelY(y) + (RelY(16) * scale);
    float lineSpacing = RelY(22) * scale;
    stbtt_aligned_quad q;
    int characterCount = 0;
    float paddingUV = (10.0f / (float)FONT_ATLAS_SIZE); // This is for the black outline around all text for readability. (2.0f makes an interesting bold effect)
    float borderWidthPixels = 2.0f;
    while (*p) {
        // Decode UTF8
        const unsigned char *s = (const unsigned char *)p;
        uint32_t codepoint = 0;
        if (*s < 0x80) {          // 1-byte ASCII
            codepoint = *s++;
        } else if ((*s & 0xE0) == 0xC0) { // 2-byte
            codepoint  = (*s & 0x1F) << 6;
            codepoint |= (s[1] & 0x3F);
            s += 2;
        } else if ((*s & 0xF0) == 0xE0) { // 3-byte
            codepoint  = (*s & 0x0F) << 12;
            codepoint |= (s[1] & 0x3F) << 6;
            codepoint |= (s[2] & 0x3F);
            s += 3;
        } else if ((*s & 0xF8) == 0xF0) { // 4-byte
            codepoint  = (*s & 0x07) << 18;
            codepoint |= (s[1] & 0x3F) << 12;
            codepoint |= (s[2] & 0x3F) << 6;
            codepoint |= (s[3] & 0x3F);
            s += 4;
        } else {
            s++; // invalid byte
        }
        p = (const char *)s;
        characterCount++;
        if (codepoint == '\n' || characterCount > 120) {
            xpos = x;
            ypos += lineSpacing;
            characterCount = 0;
            continue;
        }
        
        int idx = CodepointToPackedIndex(codepoint, fontID);
        if (fontID == FONT_STOPD) stbtt_GetPackedQuad(fontPackedCharStopD, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, idx, &xpos, &ypos, &q, 1);
        else stbtt_GetPackedQuad(fontPackedChar, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, idx, &xpos, &ypos, &q, 1);
        float vx0 = (q.x0 * scale) - (borderWidthPixels);
        float vy0 = (q.y0 * scale) - (borderWidthPixels);
        float vx1 = (q.x1 * scale) + (borderWidthPixels);
        float vy1 = (q.y1 * scale) + (borderWidthPixels);
        float s0 = (q.s0) - (paddingUV);
        float t0 = (q.t0) - (paddingUV);
        float s1 = (q.s1) + (paddingUV);
        float t1 = (q.t1) + (paddingUV);
        float z = 0.0f;
        float textVertices[30] = { vx0, vy0, z, s0, t0, vx1, vy1, z, s1, t1, vx1, vy0, z, s1, t0, vx0, vy0, z, s0, t0, vx0, vy1, z, s0, t1, vx1, vy1, z, s1, t1 };
        __builtin_memcpy(textVertexData + vertexCount * 30, textVertices, sizeof(textVertices));
        vertexCount++;
        if (codepoint >= '0' && codepoint <= '9') {
            if (fontID == FONT_STOPD) xpos = q.x0 + fixedNumberAdvanceWidthStopD;
            else xpos = q.x0 + fixedNumberAdvanceWidth;
        }
    }
    
    if (vertexCount > 0) {
        glNamedBufferData(Sys_Render.textVBO, vertexCount * 30 * sizeof(float), textVertexData, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount * 6);
        drawCallsRenderedThisFrame++; textDrawCallsRenderedThisFrame++; verticesRenderedThisFrame += vertexCount * 6;
    }
}

void RenderLoadingProgress(int32_t offset, const char * restrict text) { // Only adds 0.01secs to game startup time.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    RenderFormattedText(Sys_Settings.ScreenWidth / 2 - offset, Sys_Settings.ScreenHeight / 2 - 5, TEXT_WHITE, FONT_NORMAL,1.0f,text);
    glfwSwapBuffers(window);
}

char statusText[TEXT_BUFFER_SIZE];
void CenterStatusPrint(const char * restrict fmt, ...) {
    va_list args; __builtin_va_start(args, fmt); StringFormatV(statusText,TEXT_BUFFER_SIZE,fmt,args); __builtin_va_end(args);
    DualLog("%s\n",statusText);
    Sys_Global.statusTextDecayFinished = get_time() + 2.5; // 2.5 second decay time before text dissappears.
}

__attribute__((cold)) void NewGame(void) { // Reset World States
    DualLog("Loading new game...\n");
    RenderLoadingProgress(100,"Loading new game...");
    DualLog("Rendered screen saying \"Loading new game...\"\n");
    Sys_Global.instances[WORLD].ioflags = 0u;
    Sys_Global.instances[WORLD].lev1SecCode = random_range_u8(0u,9u); Sys_Global.instances[WORLD].lev2SecCode = random_range_u8(0u,9u);
    Sys_Global.instances[WORLD].lev3SecCode = random_range_u8(0u,9u); Sys_Global.instances[WORLD].lev4SecCode = random_range_u8(0u,9u);
    Sys_Global.instances[WORLD].lev5SecCode = random_range_u8(0u,9u); Sys_Global.instances[WORLD].lev6SecCode = random_range_u8(0u,9u); // Must do rand's repeatedly to prevent these all being the same number.
    __builtin_memset(Sys_Global.instances,0,INSTANCE_COUNT * sizeof(Entity)); // Initialize instances, the global entity array for the currently loaded level.
    DualLog("Calling PlayerInits...\n");
    PlayerInit(PLAYER1); PlayerInit(PLAYER2);
    cam_yaw = 90.0f; cam_pitch = 0.0f; cam_roll = 0.0f;
    Sys_Global.inventoryMode = Sys_Settings.NoShootMode;
    DualLog("Calling LoadLevel...\n");
    LoadLevel(Sys_Global.startLevel); // Must be after entities!
    Sys_Global.pauseRelativeTime =  Sys_Global.last_physics_time = 0.0;
    Sys_Global.last_topframe_time = Sys_Global.last_physics_time - 0.05;
    Sys_Global.timeSinceLastPhysicsTick = 0.0166666666f;
    Sys_Global.gameFinished = Sys_Global.creditsActive = Sys_Global.decoyActive = false;
	Sys_Global.ressurections = Sys_Global.deaths = Sys_Global.kills = Sys_Global.cyberkills = 0u;
	Sys_Global.shotsFired = Sys_Global.grenadesThrown = Sys_Global.savesScummed = 0U;
    Sys_Global.damageDealt = Sys_Global.damageReceived = 0.0f;
	Sys_Global.creditsPageIndex = 0u;
    for (int i=0;i<14;++i) Sys_Global.levelSecurity[i] = 100u;
    InputClearRisingAndFallingEdges();
    Sys_Input.currentMouse_dx = Sys_Input.currentMouse_dy = 0;
    Sys_Input.last_mouse_x = Sys_Input.last_mouse_y = 0;
    Sys_Input.ignore_next_mouse_delta = true;
    Sys_Input.isCapsLockOn = false; // As far as we're concerned, don't worry about OS state.
    Sys_Input.lastUse = false;
}

__attribute__((cold)) void LoadGameModDefinition(void) { // Unique set separate from savedata path and resource data to keep it focussed
    double start_time = get_time();
    DualLog("Loading game definition...");
    OsFileHandle fp    = OS_OpenReadonly("./Data/gamedata.txt");
    if (!fp) { DualLogError("\nCannot open ./Data/gamedata.txt\n"); DualLogError("Could not parse ./Data/gamedata.txt!\n"); OS_Exit(1); }
    
    int32_t gamedatSize = OS_FileSize(fp);
    char* fb = OS_AllocateFileBackedRAMReadonly(gamedatSize, fp, "./Data/gamedata.txt");
    if (!fb || gamedatSize < 1) { DualLogError("Could not open ./Data/gamedata.txt\n"); OS_Exit(1); }
    
    OS_Close(fp);
    uint32_t lineNum = 0; uint8_t lineLength = 0; char line[256]; char key[256]; char value[256]; bool is_comment = false, in_value = false;
    uint32_t keyLength = 0; uint32_t valueLength = 0;
    for (int32_t i=0;i<gamedatSize;++i) { // Less 1 to avoid comment check overflow
        if (lineLength>=255 || valueLength>=255 || keyLength>=255) continue; // Keep going until we hit a newline or EOF, no valid line is this long so must be a comment.
        if (fb[i] == ' ' || fb[i] == '\t' || fb[i] == '\v' || fb[i] == '\f' || fb[i] == '\r') continue; // Disregard blanks or Windows garbage (this ain't a typewriter, yeesh).
        if (fb[i] == '/' && fb[i + 1] == '/') is_comment = true;
        if (fb[i] == '\n') {
            if (keyLength == 0 || valueLength == 0) { is_comment = in_value = false; lineLength = keyLength = valueLength = 0; lineNum++; continue; }

            // Ok, process kv pair
            value[valueLength] = '\0'; line[lineLength] = '\0';
                 if (StringsAreEqual(key, "modname"))    StringCopyInto_A_From_B(Sys_Global.global_modname,value,sizeof(Sys_Global.global_modname));
            else if (StringsAreEqual(key, "levelcount")) Sys_Global.numLevels = parse_numberu8(value, line, lineNum);
            else if (StringsAreEqual(key, "startlevel")) Sys_Global.startLevel = parse_numberu8(value, line, lineNum);
            
            is_comment = in_value = false; lineLength = keyLength = valueLength = 0; lineNum++;
            continue;
        }
        
        if (is_comment) continue; // Keep walking characters to finish the commented line.
        if (fb[i] == ':') { in_value = true; key[keyLength] = '\0'; continue; } // Skip splitter, mark inside value for key:value pair.

        if (in_value) { value[valueLength] = fb[i]; valueLength++; }
        else          {   key[keyLength] = fb[i];   keyLength++; }
        
        line[lineLength] = fb[i]; lineLength++; // Keep filling up the current line.
    }

    DualLog(" %s:: num levels: %d, start level: %d... took %f secs\n",Sys_Global.global_modname, Sys_Global.numLevels, Sys_Global.startLevel, get_time() - start_time);
}

__attribute__((cold)) void LoadEntities(void) {
    double start_time = get_time();
    Sys_Global.entityCount = 0;
    DataParser entity_parser;
    if (!parse_data_file(&entity_parser, MAX_ENTITIES, "./Data/entities.txt")) { DualLogError("Could not parse ./Data/entities.txt!\n"); OS_Exit(1); }
    
    Sys_Global.entityCount = (uint16_t)entity_parser.count;
    DualLog("Loading  %d entities...", Sys_Global.entityCount);
    if (Sys_Global.entityCount > MAX_ENTITIES) { DualLogError("Too many entities in parser count %d, greater than %d!\n", Sys_Global.entityCount, MAX_ENTITIES); OS_Exit(1); }
    if (Sys_Global.entityCount == 0) { DualLogError("No entities found in entities.txt\n"); OS_Exit(1); }

    __builtin_memset(Sys_Global.entities,0,MAX_ENTITIES * sizeof(Entity));
    ModEntityDefinitionsInitAfterLoad(&entity_parser);
    OS_DeallocateRAM(entity_parser.entries,entity_parser.count * sizeof(Entity));
    DualLog(" took %f secs\n", get_time() - start_time);
    DebugRAM("after loading all entities");
}

void GenerateAndBindTexture(GLuint *id, GLint internalFormat, int32_t width, int32_t height, GLenum format, GLenum type, GLenum target) {
    if (*id == 0) glGenTextures(1, id);
    glBindTexture(target, *id);
    glTexImage2D(target, 0, internalFormat, width, height, 0, format, type, NULL);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void UpdateProjectionMatrices(void) {
    float* m;
    m = uiOrthoProjection;
    m[0] = 2.0f / (float)Sys_Settings.ScreenWidth; m[1] =                                       0.0f; m[2] =  0.0f; m[3] = 0.0f;
    m[4] =                                   0.0f; m[5] = -2.0f / ((float)Sys_Settings.ScreenHeight); m[6] =  0.0f; m[7] = 0.0f;
    m[8] =                                   0.0f; m[9] =                                       0.0f; m[10]= -1.0f; m[11]= 0.0f;
    m[12]=                                  -1.0f; m[13]=                                       1.0f; m[14]=  0.0f; m[15]= 1.0f;
    
    Sys_Global.aspect3D = (float)Sys_Settings.ScreenWidth / (float)Sys_Settings.ScreenHeight;
    float f = vcot((float)Sys_Settings.FOV * PI / 360.0f);
    m = rasterPerspectiveProjection;
    m[0] = f / Sys_Global.aspect3D; m[1] = 0.0f; m[2] =                                           0.0f; m[3] =  0.0f;
    m[4] =         0.0f; m[5] =    f; m[6] =                                                      0.0f; m[7] =  0.0f;
    m[8] =         0.0f; m[9] = 0.0f; m[10]=      -(FAR_PLANE + NEAR_PLANE) / (FAR_PLANE - NEAR_PLANE); m[11]= -1.0f;
    m[12]=         0.0f; m[13]= 0.0f; m[14]= -2.0f * FAR_PLANE * NEAR_PLANE / (FAR_PLANE - NEAR_PLANE); m[15]=  0.0f;
    voxen_Shadow_System.shadDotThresh = 1.0f / vsqrtf(1.0f + vtan((float)Sys_Settings.FOV * PI / 360.0f) * (1.0f + Sys_Global.aspect3D * Sys_Global.aspect3D));
}

void UpdateScreenSize(GLFWwindow* unused, int32_t width, int32_t height) {
    (void)unused; // Appease glfwSetFramebufferSizeCallback pointer type
    Sys_Settings.ScreenWidth = vmax(vmin((uint16_t)width,7680u),320u); Sys_Settings.ScreenHeight = vmax(vmin((uint16_t)height,4320u),200u); // Cap at minimum Quake 1 resolution and maximum 8k.
    Sys_Settings.ScreenCenterX = (float)Sys_Settings.ScreenWidth * 0.5f; Sys_Settings.ScreenCenterY = (float)Sys_Settings.ScreenHeight * 0.5f;
    glViewport(0,0,Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight);
    UpdateProjectionMatrices();
    glUseProgram(Sys_Render.imageBlitShaderProgram);
    glUniform1ui(2,Sys_Settings.ScreenWidth);
    glUniform1ui(3,Sys_Settings.ScreenHeight);
    glUniform1i(26,Sys_Settings.SSR_RES);
    glUseProgram(Sys_Render.chunkShaderProgram);
    glUniform1ui(6,Sys_Settings.ScreenWidth);
    glUniform1ui(7,Sys_Settings.ScreenHeight);
    glUseProgram(Sys_Render.ssrShaderProgram);
    glUniform1ui(0,Sys_Settings.ScreenWidth / Sys_Settings.SSR_RES);
    glUniform1ui(1,Sys_Settings.ScreenHeight / Sys_Settings.SSR_RES);       
    glUniform1i(2,Sys_Settings.SSR_RES);
    GenerateAndBindTexture(&Sys_Render.inputImageID,            GL_RGBA8,Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight,           GL_RGBA,GL_UNSIGNED_BYTE,GL_TEXTURE_2D); // Lit Raster
    GenerateAndBindTexture(&Sys_Render.inputWorldPosID,       GL_RGBA32F,Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight,           GL_RGBA,        GL_FLOAT,GL_TEXTURE_2D); // Raster World Positions
    GenerateAndBindTexture(&Sys_Render.inputDepthID,GL_DEPTH_COMPONENT32,Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight,GL_DEPTH_COMPONENT,        GL_FLOAT,GL_TEXTURE_2D); // Raster Depth
    GenerateAndBindTexture(&Sys_Render.inputSpecID,             GL_RGBA8,Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight,           GL_RGBA,GL_UNSIGNED_BYTE,GL_TEXTURE_2D); // Specular Colors
    GenerateAndBindTexture(&Sys_Render.inputNormalID,           GL_RG16F,Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight,            GL_RGB,        GL_FLOAT,GL_TEXTURE_2D); // Normal XYZ
    glGenTextures(1,&Sys_Render.outputImageID);
    glBindTexture(GL_TEXTURE_2D,Sys_Render.outputImageID);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,Sys_Settings.ScreenWidth / Sys_Settings.SSR_RES,Sys_Settings.ScreenHeight / Sys_Settings.SSR_RES,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D,0);
    glBindFramebuffer(GL_FRAMEBUFFER, Sys_Render.gBufferFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,Sys_Render.inputImageID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT1,GL_TEXTURE_2D,Sys_Render.inputWorldPosID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT2,GL_TEXTURE_2D,Sys_Render.inputSpecID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT3,GL_TEXTURE_2D,Sys_Render.inputNormalID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,Sys_Render.inputDepthID, 0);
    glBindImageTexture(0,Sys_Render.inputImageID,0,GL_FALSE,0,GL_READ_WRITE,GL_RGBA8); // Main Rendered Color
    glBindImageTexture(1,Sys_Render.inputWorldPosID,0,GL_FALSE,0,GL_READ_WRITE,GL_RGBA32F); // World Position XYZ
    glBindImageTexture(2,Sys_Render.inputSpecID,0,GL_FALSE,0,GL_READ_WRITE,GL_RGBA8); // Specular
    glBindImageTexture(4,Sys_Render.outputImageID,0,GL_FALSE,0,GL_READ_WRITE,GL_RGBA8); // SSR result
    glBindImageTexture(5,Sys_Render.inputNormalID,0,GL_FALSE,0,GL_READ_WRITE,GL_RG16F); // Normal XYZ
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D,Sys_Render.outputImageID);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// GLFW Callbacks
bool IsNonRepeatingKey(int32_t key) { return key == GLFW_KEY_KP_ENTER || key == GLFW_KEY_ENTER || key == GLFW_KEY_TAB || key == GLFW_KEY_ESCAPE; }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
static void key_callback(GLFWwindow* window, int32_t key, int32_t scancode, int32_t action, int32_t mods) {
    if (key == GLFW_KEY_F10 && action) OS_Exit(0);
    if (Sys_Global.menuActive && !returnToPause) {
        if ((key == GLFW_KEY_RIGHT_ALT || key == GLFW_KEY_LEFT_ALT) && action && Sys_Input.keyStates[GLFW_KEY_ENTER].down)                    GoIntoGame();
        if (key == GLFW_KEY_ENTER && action && (Sys_Input.keyStates[GLFW_KEY_LEFT_ALT].down || Sys_Input.keyStates[GLFW_KEY_RIGHT_ALT].down)) GoIntoGame();
    }

    if (action == GLFW_PRESS || (action == GLFW_REPEAT && !IsNonRepeatingKey(key))) Input_KeyDown(key);
    else if (action == GLFW_RELEASE)                                                Input_KeyUp(key);
}

void Input_PollJoysticks(void) {
    for (int jid = GLFW_JOYSTICK_1; jid <= GLFW_JOYSTICK_LAST; ++jid) {
        if (!glfwJoystickPresent(jid)) continue;

        int buttonCount;
        const unsigned char* buttons = glfwGetJoystickButtons(jid, &buttonCount);
        for (int i = 0; i < buttonCount && i < MAX_JOYSTICK_BUTTONS; ++i) {
            KeyState* k = &Sys_Input.joystickButtons[GLFW_JOYSTICK_1][i];
            bool down = buttons[i] == GLFW_PRESS;
            k->pressed  = down && !k->down;
            k->released = !down && k->down;
            k->down     = down;
        }

        int hatCount;
        const unsigned char* hats = glfwGetJoystickHats(jid, &hatCount);
        for (int i = 0; i < hatCount && i < MAX_JOYSTICK_HATS; ++i) Sys_Input.joystickHats[i].down = hats[i];
    }
}

void Input_PollGamepad(void) {
    GLFWgamepadstate s;
    if (!glfwGetGamepadState(GLFW_JOYSTICK_1, &s)) return;

    for (int i = 0; i < GLFW_GAMEPAD_BUTTON_LAST + 1; ++i) {
        KeyState* k = &Sys_Input.gamepadButtons[i];
        bool down = s.buttons[i] == GLFW_PRESS;
        k->pressed  = down && !k->down;
        k->released = !down && k->down;
        k->down     = down;
    }
}

static void joystick_callback(int32_t jid, int32_t event) {
    if (jid > GLFW_JOYSTICK_LAST) return;
    bool connected = event == GLFW_CONNECTED;
    Sys_Input.joystickPresent[jid] = connected;
    if (!connected) { __builtin_memset(Sys_Input.joystickButtons,0,sizeof(Sys_Input.joystickButtons)); __builtin_memset(Sys_Input.joystickHats,0,sizeof(Sys_Input.joystickHats)); } // Clear
}

static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    if (Sys_Input.window_has_focus) {
        Sys_Input.currentMouse_dx = (int32_t)(xpos - Sys_Input.last_mouse_x);
        Sys_Input.currentMouse_dy = (int32_t)(ypos - Sys_Input.last_mouse_y);
        Sys_Input.last_mouse_x = xpos;
        Sys_Input.last_mouse_y = ypos;
        if (Sys_Input.ignore_next_mouse_delta) { Sys_Input.ignore_next_mouse_delta = false; return; }
        
        if (Sys_Global.globalFrameNum > 1) Input_MouseMove(Sys_Input.currentMouse_dx,Sys_Input.currentMouse_dy);
    }
}

static void window_focus_callback(GLFWwindow* window, int32_t focused) {
    Sys_Input.window_has_focus = focused != 0;
    Sys_Input.ignore_next_mouse_delta = true;
    glfwSetInputMode(window, GLFW_CURSOR, Sys_Input.window_has_focus ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

static void mouse_button_callback(GLFWwindow* window, int32_t button, int32_t action, int32_t mods) {
    Sys_Input.mouseButtons[button].down = Sys_Input.mouseButtons[button].pressed = (action == GLFW_PRESS);
    Sys_Input.mouseButtons[button].released = (action == GLFW_RELEASE);
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) { Sys_Input.scrollDelta += yoffset; }
#pragma GCC diagnostic pop

double monitorSwitchTime;
int currentMonitorIndex = 1; // Start on primary after first cycle, puts it a 0.
void CycleToNextMonitor(void) {
    if (get_time() < monitorSwitchTime) return;
    
    monitorSwitchTime = get_time() + 0.5; // Prevent toggling rapidly on accident
    int monitorCount;
    GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
    if (!monitors || monitorCount < 2) return;

    currentMonitorIndex = (currentMonitorIndex + 1) % monitorCount;
    GLFWmonitor* next = monitors[currentMonitorIndex];

    int mx, my;
    glfwGetMonitorPos(next, &mx, &my);
    const GLFWvidmode* mode = glfwGetVideoMode(next);
    int xpos = mx + (mode->width - Sys_Settings.ScreenWidth) / 2;
    int ypos = my + (mode->height - Sys_Settings.ScreenHeight) / 2;
    glfwSetWindowPos(window, xpos, ypos);
    Sys_Input.ignore_next_mouse_delta = true;
    DualLog("Window moved to monitor %d: %s at x: %d, y: %d\n", currentMonitorIndex, glfwGetMonitorName(next), xpos, ypos);
}

GLFWmonitor* GetCurrentMonitor(void) {
    int wx, wy, ww, wh;
    glfwGetWindowPos(window, &wx, &wy);
    glfwGetWindowSize(window, &ww, &wh);
    int bestArea = 0;
    GLFWmonitor* bestMonitor = glfwGetPrimaryMonitor();
    int monitorCount;
    GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
    for (int i = 0; i < monitorCount; i++) {
        int mx, my;
        glfwGetMonitorPos(monitors[i], &mx, &my);
        const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
        int mw = mode->width;
        int mh = mode->height;
        int left   = vmax(wx, mx);
        int right  = vmin(wx + ww, mx + mw);
        int top    = vmax(wy, my);
        int bottom = vmin(wy + wh, my + mh);
        int area = (right > left && bottom > top) ? (right - left) * (bottom - top) : 0;
        if (area > bestArea) {
            bestArea = area;
            bestMonitor = monitors[i];
        }
    }
    return bestMonitor;
}

void GoIntoGame(void) {
    Sys_Global.menuActive = Sys_Global.gamePaused = enteringPlayerName = gammaSliderActive = fovSliderActive = masterVolumeSliderActive = musicVolumeSliderActive = messageVolumeSliderActive = sfxVolumeSliderActive = returnToPause = false;
    currentMenuItem = currentMenuTab = 0; currentMenuPage = MenuPages_FrontPage;
    Sys_Global.inventoryMode = false;
    NewGame();
    PlayGameMusic();
    DualLog("Player named \"%s\" started the game!\n", Sys_Global.playerName);
}

void GatherResolutionModes(void) {
    resDropdownCount = 0;
    GLFWmonitor* monitor = GetCurrentMonitor();    if (!monitor) monitor = glfwGetPrimaryMonitor();
    int modeCount; const GLFWvidmode* modes = glfwGetVideoModes(monitor, &modeCount);
    if (!modes || modeCount < 1) return;
    for (int i = modeCount - 1; i >= 0 && resDropdownCount < 8; --i) {
        int w = modes[i].width, h = modes[i].height, hz = modes[i].refreshRate;
        int j = 0;
        for (; j < resDropdownCount; ++j) {
            if (resModes[j].w == w && resModes[j].h == h) { if (hz > resModes[j].hz) resModes[j].hz = hz; break; }
        }
        
        if (j == resDropdownCount) { resModes[resDropdownCount++] = (ResMode){w, h, hz}; }
    }
    
    resSelectedIdx = 0;
    for (int i = 0; i < resDropdownCount; ++i) {
        if (resModes[i].w == (int)Sys_Settings.ScreenWidth && resModes[i].h == (int)Sys_Settings.ScreenHeight) { resSelectedIdx = i; break; }
    }
}

void ChangeResolution(void) {
    if (resDropdownCount < 1) return;
    resSelectedIdx = (resSelectedIdx + 1) % resDropdownCount;
    Sys_Settings.ScreenWidth  = (uint32_t)resModes[resSelectedIdx].w;
    Sys_Settings.ScreenHeight = (uint32_t)resModes[resSelectedIdx].h;
    GLFWmonitor* monitor = GetCurrentMonitor();
    if (!monitor) monitor = glfwGetPrimaryMonitor();
    int mx,my; glfwGetMonitorPos(monitor, &mx, &my);
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    int xpos = mx + (mode->width  - (int)Sys_Settings.ScreenWidth)  / 2;
    int ypos = my + (mode->height - (int)Sys_Settings.ScreenHeight) / 2;
    glfwSetWindowSize(window, (int)Sys_Settings.ScreenWidth, (int)Sys_Settings.ScreenHeight);
    glfwSetWindowPos(window, xpos, ypos);
    UpdateScreenSize(NULL, (int)Sys_Settings.ScreenWidth, (int)Sys_Settings.ScreenHeight);
    Sys_Input.ignore_next_mouse_delta = true;
    resDropdownOpen = false;
    SaveConfig();
}

void ChangeFullScreenWindowed(void) {
    int monitorCount;
    GLFWmonitor* next = glfwGetMonitors(&monitorCount)[currentMonitorIndex];
    if (Sys_Settings.Fullscreen) {
        int xpos, ypos, width, height;
        glfwGetMonitorWorkarea(next,&xpos,&ypos,&width,&height);
        Sys_Settings.ScreenWidth = width; Sys_Settings.ScreenHeight = height;
        glfwSetWindowSize(window,Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight);
        glfwSetWindowPos(window,xpos,ypos - 18);
    } else {
        int mx,my; glfwGetMonitorPos(next, &mx, &my);
        const GLFWvidmode* mode = glfwGetVideoMode(next);
        int xpos = mx + (mode->width-Sys_Settings.ScreenWidth) / 2;
        int ypos = my + (mode->height-Sys_Settings.ScreenHeight) / 2;
        glfwSetWindowPos(window,xpos,ypos);
    }
    
    UpdateScreenSize(NULL,Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight);
    Sys_Input.ignore_next_mouse_delta = true;
}

void SetSkyRotateSpeed(void) {
    static const float speeds[] = { 0.05f, 1.0f, 2.5f, 3.75f, 6.25f };
    float skyRotateSpeed = speeds[Sys_Cheats.dizzyLevel];
    glUseProgram(Sys_Render.imageBlitShaderProgram);
    glUniform1f(30, skyRotateSpeed);
}

void SetVSync(void) { glfwSwapInterval((int32_t)Sys_Settings.Vsync); }
void SetGI(void) { }// TODO: Set needed Voxel GI uniforms from Sys_Settings.GI
void SetSpeakerMode(void) {
    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    switch (Sys_Settings.SpeakerMode) {
        case 0: config.playback.channels = 1; break;  // Mono
        case 1: config.playback.channels = 2; break;  // Stereo
        case 2: config.playback.channels = 4; break;  // Quad (4.0)
        case 3: config.playback.channels = 4; break;  // Surround (often 4.0)
        case 4: config.playback.channels = 6; break;  // 5.1
        case 5: config.playback.channels = 8; break;  // 7.1
        case 6: config.playback.channels = 2; break;  // Prologic → usually handled as stereo + decoder
        default: config.playback.channels = 0; break; // Let device decide
    }
}

void SetLanguage(void) { LoadTextForLanguage(Sys_Settings.Language); LoadLogTextForLanguage(Sys_Settings.Language); }

void ApplySettings(void) {
    UpdateScreenSize(NULL, Sys_Settings.ScreenWidth, Sys_Settings.ScreenHeight);
    SetSkyRotateSpeed();
    SetVSync();
    SetGI();
    SetSpeakerMode();
    SetLanguage();
}

#if defined(_WIN32) || defined(__CYGWIN__)
    #include <windows.h>
    #define PLATFORM_DLOPEN(path)        LoadLibraryA(path)
    #define PLATFORM_DLSYM(handle, name) GetProcAddress((handle), (name))
    #define PLATFORM_DLCLOSE(handle)     FreeLibrary((handle))
    static char win_err_buf[512];
    static const char* PLATFORM_DLERROR(void) {
        DWORD err = GetLastError();
        if (err == 0) return NULL;
        FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,NULL,err,MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),win_err_buf,sizeof(win_err_buf),NULL);
        return win_err_buf;
    }
#else
    #include <dlfcn.h>
    #define PLATFORM_DLOPEN(path)        dlopen((path), RTLD_NOW)
    #define PLATFORM_DLSYM(handle, name) dlsym((handle), (name))
    #define PLATFORM_DLCLOSE(handle)     dlclose((handle))
    #define PLATFORM_DLERROR()           dlerror()
#endif

bool GetKey(int settingIndex);
bool GetKeyPressed(int settingIndex);
void* mod_handle = NULL;
void LoadModFunctions(void) {
    // Clear previous handle if reloading
    if (mod_handle) {
        PLATFORM_DLCLOSE(mod_handle);
        mod_handle = NULL;
    }

    DualLog("Reloading mod code...");
    char mod_path[256];
    StringCopyInto_A_From_B(mod_path, "./", 256);
    StringConcatenate(mod_path, Sys_Global.global_modname, 256);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
    #if defined(_WIN32) || defined(__CYGWIN__)
        StringConcatenate(mod_path,".dll",256); // e.g. Citadel.dll
    #else
        StringConcatenate(mod_path,".so",256); // e.g. Citadel.so
    #endif
#pragma GCC diagnostic pop

    DualLog("dlopen-ing...");
    mod_handle = PLATFORM_DLOPEN(mod_path);
    if (!mod_handle) {
        const char* err = PLATFORM_DLERROR();
        if (err && *err) {
            DualLogError("dlopen of %s failed: %s",mod_path,err);
        } else {
            DualLogError("dlopen of %s failed: no detailed error from dlerror() — common with unresolved symbols or format issues",mod_path);
        }
        OS_Exit(1);
    }
    
    #define X(ret, name, params) \
        name = (ret (*) params)PLATFORM_DLSYM(mod_handle, #name); \
        if(!name) DualLogError("Failed to load mod function: %s", #name);

    MOD_FUNCTION_LIST(X)
    #undef X
    ModLink(&Sys_Global,&Sys_Cheats,&Sys_Settings,&Sys_Text,&Sys_UI);
    Sys_Global.GetKey = GetKey;
    Sys_Global.GetKeyPressed = GetKeyPressed;
    DualLog("done!\n");
}

extern unsigned char *stbi_load_from_memory(const uint8_t* buffer, int32_t len, int32_t *x, int32_t *y);
extern int32_t stbi_arena_size;
extern uint8_t*  stbi__arena_base;
extern void stbi__arena_init(void);
#define STBI_ARENA_SIZE 16 * 1024 * 1024
__attribute__((cold)) void InitializeEnvironment(void) {
    double init_start_time = get_time();
    Sys_Global.globalFrameNum = 0;
    DebugRAM("InitializeEnvironment start");
    DualLog("Voxen, the Voxel Lit Open Source Game Engine by W. Josiah Jack, MIT-0 licensed\n");
    if (!glfwInit()) { DualLogError("GLFW initialization failed\n"); OS_Exit(1); }
    
    double initMarker2 = get_time();
    DualLog("GLFW init took %f secs\n",initMarker2 - init_start_time);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SRGB_CAPABLE, 0);
    glfwWindowHint(GLFW_RESIZABLE, 1);
    LoadConfig(); // Get settings before setting window size.
    window = glfwCreateWindow(Sys_Settings.ScreenWidth, Sys_Settings.ScreenHeight, "Voxen", NULL, NULL);
    glfwSetFramebufferSizeCallback(window, UpdateScreenSize);
    if (!window) { DualLogError("glfwCreateWindow failed\n"); OS_Exit(1); }

    glfwMakeContextCurrent(window);
    DualLog("Load Config.ini, glfw create window and GL context took %f secs\n",get_time() - initMarker2);
    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) { DualLogError("Failed to initialize GLAD\n"); OS_Exit(1); }
    
    GLint major = 0, minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    if (major < 4 || (major == 4 && minor < 3)) { DualLogError("Need OpenGL >= 4.3, got %d.%d\n", major, minor); OS_Exit(1); }
    double initMarker3 = get_time();
    CycleToNextMonitor();
    glfwSetKeyCallback(window, key_callback);
    glfwSetJoystickCallback(joystick_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetWindowFocusCallback(window, window_focus_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glFrontFace(GL_CCW); // Set triangle sorting order (GL_CW vs GL_CCW)
//     glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Globally same alpha blending
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
    CompileShaders();
    double initMarker4 = get_time();
    DualLog("Set monitor, Set GLFW callbacks, Compile shaders took %f secs\n",initMarker4 - initMarker3);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the corner where last shadowmap wrote into
    GLuint vaos[4]; GLuint vbos[4];
    glCreateVertexArrays(4, vaos);
    glCreateBuffers(3, vbos);
    Sys_Render.quadVAO = vaos[0]; Sys_Render.vao_chunk = vaos[1]; Sys_Render.textVAO = vaos[2]; Sys_Render.debugLinesVAO = vaos[3];
    Sys_Render.quadVBO = vbos[0];                                     Sys_Render.textVBO = vbos[1]; Sys_Render.debugLinesVBO = vbos[2];
    float quadBlit_vertices[] = { 1.0f, -1.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,    -1.0f,1.0f, 0.0f, 1.0f,   -1.0f, -1.0f, 0.0f, 0.0f }; // 4 verts, 4 floats each pos.xy, uv.xy
    glNamedBufferData(Sys_Render.quadVBO, sizeof(quadBlit_vertices), quadBlit_vertices, GL_STATIC_DRAW);

    glVertexArrayAttribFormat(Sys_Render.quadVAO, 0, 2, GL_FLOAT, GL_FALSE, 0); // DSA: Set position format
    glVertexArrayAttribFormat(Sys_Render.quadVAO, 1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float)); // DSA: Set texcoord format
    glVertexArrayVertexBuffer(Sys_Render.quadVAO, 0, Sys_Render.quadVBO, 0, 4 * sizeof(float)); // DSA: Link VBO to VAO
    for (uint8_t i = 0; i < 2; i++) { glVertexArrayAttribBinding(Sys_Render.quadVAO, i, 0); glEnableVertexArrayAttrib(Sys_Render.quadVAO, i); }
    
    glVertexArrayAttribFormat(Sys_Render.vao_chunk, 0, 3, GL_FLOAT, GL_FALSE, 0); // Position (vec3)
    glVertexArrayAttribFormat(Sys_Render.vao_chunk, 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float)); // Normal (vec3)
    glVertexArrayAttribFormat(Sys_Render.vao_chunk, 2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float)); // Tex Coord (vec2)
    for (uint8_t i = 0; i < 3; i++) { glVertexArrayAttribBinding(Sys_Render.vao_chunk, i, 0); glEnableVertexArrayAttrib(Sys_Render.vao_chunk, i); }
    
    glVertexArrayAttribFormat(Sys_Render.textVAO, 0, 3, GL_FLOAT, GL_FALSE, 0); // pos (x,y,z) 4 floats per vertex, stride = 4*sizeof(float)
    glVertexArrayAttribFormat(Sys_Render.textVAO, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float));  // uv (s,t)
    glVertexArrayVertexBuffer(Sys_Render.textVAO, 0, Sys_Render.textVBO, 0, 5 * sizeof(float));
    for (uint8_t i = 0; i < 2; i++) { glVertexArrayAttribBinding(Sys_Render.textVAO, i, 0); glEnableVertexArrayAttrib(Sys_Render.textVAO, i); }
    
    glNamedBufferStorage(Sys_Render.debugLinesVBO, MAX_DEBUG_LINE_VERTS * 3 * sizeof(float), NULL, GL_DYNAMIC_STORAGE_BIT);  // persistent, client-writable
    glVertexArrayAttribFormat(Sys_Render.debugLinesVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glEnableVertexArrayAttrib(Sys_Render.debugLinesVAO, 0);
    glVertexArrayAttribBinding(Sys_Render.debugLinesVAO, 0, 0);
    glVertexArrayVertexBuffer(Sys_Render.debugLinesVAO, 0, Sys_Render.debugLinesVBO, 0, 3 * sizeof(float));
    float* m = shadowmapsPerspectiveProjection;
    m[0] = 1.0f; m[1] = 0.0f; m[2] =                                                                  0.0f; m[3] =  0.0f;
    m[4] = 0.0f; m[5] = 1.0f; m[6] =                                                                  0.0f; m[7] =  0.0f;
    m[8] = 0.0f; m[9] = 0.0f; m[10]=      -(LIGHT_RANGE_MAX + NEAR_PLANE) / (LIGHT_RANGE_MAX - NEAR_PLANE); m[11]= -1.0f;
    m[12]= 0.0f; m[13]= 0.0f; m[14]= -2.0f * LIGHT_RANGE_MAX * NEAR_PLANE / (LIGHT_RANGE_MAX - NEAR_PLANE); m[15]=  0.0f;
    DualLog("GL buffer definitions took %f secs\n", get_time() - initMarker4);
    ma_result result;
    ma_engine_config engine_config = ma_engine_config_init();
    engine_config.channels = 2; // Stereo output, adjust if needed
    result = ma_engine_init(&engine_config, &Sys_Global.audio_engine); if (result != MA_SUCCESS) DualLog("ERROR: Failed to initialize miniaudio engine: %d\n", result);
    LoadGameModDefinition();
    LoadModFunctions();
    LoadEntities();
    InitFontAtlasses();
    double nextInitTimeSection = get_time();
    RenderLoadingProgress(80,"Loading...");
    glGenFramebuffers(1, &Sys_Render.gBufferFBO);
    ApplySettings(); // After loading of text and game data.
    glBindFramebuffer(GL_FRAMEBUFFER, Sys_Render.gBufferFBO);
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
    glDrawBuffers(4, drawBuffers);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) DualLogError("Framebuffer incomplete: Error code %d\n", status);
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Needed to render loading progress.
    glDepthMask(GL_TRUE); // Always true, set just once ever.
    glfwSetWindowTitle(window, Sys_Global.global_modname);
    OsFileHandle fp = OS_OpenReadonly("./Textures/UI/menudot1.png");
    if (fp) {
        int windowIconFileSize = OS_FileSize(fp);
        uint8_t* file_buffer = OS_AllocateFileBackedRAMReadonly(windowIconFileSize, fp, "./Textures/UI/menudot1.png");
        if (!file_buffer) { DualLogError("Could not open backed buffer for ./Textures/UI/menudot1.png\n"); OS_Exit(1); }
        
        OS_Close(fp);
        int w = 1, h = 1;
        stbi__arena_init();
        unsigned char* pixels = stbi_load_from_memory(file_buffer, windowIconFileSize, &w, &h);
        if (!pixels) { DualLogError("Failed to load icon: ./Textures/UI/menudot1.png\n"); OS_Exit(1); }
        
        GLFWimage image; image.width  = w; image.height = h; image.pixels = pixels;
        glfwSetWindowIcon(window, 1, &image);
        OS_DeallocateRAM(file_buffer, windowIconFileSize);
        OS_DeallocateRAM(stbi__arena_base, STBI_ARENA_SIZE); stbi__arena_base = NULL;
    }

    DebugRAM("after freeing window bar icon");
    float mat[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    __builtin_memcpy(&modelMatrices[0],mat,16 * sizeof(float)); // Null instance matrix used for UI
    Sys_Render.cellVisibleDataID       = SetupSSBO(&Sys_Render.cellVisibleDataID,        4, ARRSIZE * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
    Sys_Render.shadowMapSSBO           = SetupSSBO(&Sys_Render.shadowMapSSBO,            5, TOTAL_SHADOWMAP_PIXELS * sizeof(uint32_t), NULL, GL_STATIC_DRAW);    
    glUseProgram(Sys_Render.shadowmapsShaderProgram); glUniform1ui(9,         SHADOW_MAP_SIZE);
    glUseProgram(Sys_Render.chunkShaderProgram);      glUniform1ui(21,        SHADOW_MAP_SIZE);
                                                      glUniform1f (22, (float)SHADOW_MAP_SIZE); glUniform1ui(23, LIGHT_COUNT);
                                                      glUniform1ui(24, (uint32_t)MAX_LIGHTS_PER_VOXEL);
    Sys_Render.voxelLightListCountsID  = SetupSSBO(&Sys_Render.voxelLightListCountsID,   6, VOXEL_COUNT * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
    Sys_Render.shadowMapsIndirectionID = SetupSSBO(&Sys_Render.shadowMapsIndirectionID,  8, LIGHT_COUNT * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
    Sys_Render.matricesBufferID        = SetupSSBO(&Sys_Render.matricesBufferID,        11, INSTANCE_COUNT * 16 * sizeof(float), modelMatrices, GL_STATIC_DRAW);
    Sys_Render.colorBufferID           = SetupSSBO(&Sys_Render.colorBufferID,           12, MAX_TOTAL_PIXELS * sizeof(uint8_t), NULL, GL_STATIC_DRAW);
    Sys_Render.blueNoiseBuffer         = SetupSSBO(&Sys_Render.blueNoiseBuffer,         13, 12288 * sizeof(float), blueNoise, GL_STATIC_DRAW);
    Sys_Render.textureOffsetsID        = SetupSSBO(&Sys_Render.textureOffsetsID,        14, MAX_VALID_TEXTURE * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
    Sys_Render.textureSizesID          = SetupSSBO(&Sys_Render.textureSizesID,          15, MAX_VALID_TEXTURE * 2 * sizeof(int32_t), NULL, GL_STATIC_DRAW);
    Sys_Render.texturePalettesID       = SetupSSBO(&Sys_Render.texturePalettesID,       16, MAX_UNIQUE_COLORS * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
    Sys_Render.texturePaletteOffsetsID = SetupSSBO(&Sys_Render.texturePaletteOffsetsID, 17, MAX_VALID_TEXTURE * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
    Sys_Render.lightsID                = SetupSSBO(&Sys_Render.lightsID,                19, LIGHT_COUNT * LIGHT_DATA_SIZE * sizeof(float), NULL, GL_STATIC_DRAW);
    Sys_Render.uniqueLightListsID      = SetupSSBO(&Sys_Render.uniqueLightListsID,      27, VOXEL_COUNT * MAX_LIGHTS_PER_VOXEL * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
    DualLog("GL SSBOs and Settings Apply... took %f secs\n",get_time() - nextInitTimeSection);
    if (Sys_Global.introNotPlayed) {} // TODO: Play intro
    NewGame();
    play_mp3("./Audio/music/TITLOOP-00_menu.mp3",1500);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    DebugRAM("InitializeEnvironment end");
    DualLog("InitializeEnvironment completed\n");
}

ENGINE_TO_MOD bool GetSoundIsPlaying(ma_sound* sound) { return ma_sound_is_playing(sound); }
float GetSoundRemainingTime(ma_sound* pSound) {
    if (!pSound || !ma_sound_is_playing(pSound)) return 0.0f;

    ma_uint64 currentFrame = ma_sound_get_time_in_pcm_frames(pSound);
    ma_uint64 pcmFramesLength = 0;
    ma_sound_get_length_in_pcm_frames(pSound, &pcmFramesLength);
    if (currentFrame >= pcmFramesLength) return 0.0f;

    uint64_t deltaFrames = pcmFramesLength - currentFrame;
    uint32_t sampleRate = ma_engine_get_sample_rate(&Sys_Global.audio_engine);
    return (float)deltaFrames / (float)sampleRate;
}

void mp3_clear(void) {
    ma_sound_stop(&Sys_Global.mp3_sounds[0]);
    ma_sound_stop(&Sys_Global.mp3_sounds[1]);
    Sys_Global.mp3_slot = 0;
}

ENGINE_TO_MOD void SoundSetVolume(ma_sound* pSound, float volume) { ma_sound_set_volume(pSound,volume); }
float GetSFXVolume(float volume) { return ((float)Sys_Settings.VolumeMaster/100.0f) * ((float)Sys_Settings.VolumeEffects/100.0f) * volume; }
float GetMusicVolume(void) { return ((float)Sys_Settings.VolumeMaster/100.0f) * ((float)Sys_Settings.VolumeMusic/100.0f); }
float GetMessageVolume(void) { return ((float)Sys_Settings.VolumeMaster/100.0f) * ((float)Sys_Settings.VolumeMessage/100.0f); }
void set_music_volume(void) { for (int i=0;i<2;++i) { ma_sound_set_volume(&Sys_Global.mp3_sounds[i], GetMusicVolume()); } }
void set_sfx_volume(void) { for (int i=0;i<MAX_CHANNELS;++i) { ma_sound_set_volume(&wav_sounds[i], GetSFXVolume(wav_volumes[i])); } }
void set_message_volume(void) { ma_sound_set_volume(&log_sound, GetMessageVolume()); }
void set_master_volume(void) { set_sfx_volume(); set_music_volume(); set_message_volume(); }

void play_mp3(const char* path, int32_t fade_in_ms) {
    int32_t old_slot = Sys_Global.mp3_slot;
    int32_t next_slot = Sys_Global.mp3_slot ? 0 : 1;
    if (ma_sound_is_playing(&Sys_Global.mp3_sounds[old_slot])) ma_sound_set_fade_in_milliseconds(&Sys_Global.mp3_sounds[old_slot], GetMusicVolume(), 0.0f, fade_in_ms);
    ma_sound_uninit(&Sys_Global.mp3_sounds[next_slot]); 
    ma_result result = ma_sound_init_from_file(&Sys_Global.audio_engine, path, MA_SOUND_FLAG_STREAM, NULL, NULL, &Sys_Global.mp3_sounds[next_slot]);
    if (result != MA_SUCCESS) { DualLog("ERROR: Failed to load MP3 %s: %d\n", path, result); return; }

    ma_sound_set_fade_in_milliseconds(&Sys_Global.mp3_sounds[next_slot], 0.0f, GetMusicVolume(), fade_in_ms);
    ma_sound_start(&Sys_Global.mp3_sounds[next_slot]);
    Sys_Global.mp3_slot = next_slot;
}

ENGINE_TO_MOD void play_wav(const char* path, float volume, Vector3 pos, bool positional) {
    int32_t slot = -1;
    for (int32_t i = 0; i < wav_count; i++) { // Try to find a free slot (either unused or finished)
        if (!ma_sound_is_playing(&wav_sounds[i]) && ma_sound_at_end(&wav_sounds[i])) {
            ma_sound_uninit(&wav_sounds[i]);
            slot = i;
            break;
        }
    }

    if (slot == -1 && wav_count < MAX_CHANNELS) slot = wav_count++; // If no free slot, use a new one if available
    if (slot == -1) { DualLog("WARNING: Max effect WAV channels (%d) reached\n", MAX_CHANNELS); return; }

    ma_result result = ma_sound_init_from_file(&Sys_Global.audio_engine, path, 0, NULL, NULL, &wav_sounds[slot]);
    if (result != MA_SUCCESS) {
        DualLog("ERROR: Failed to load effect WAV %s: %d\n", path, result);
        if (slot == wav_count - 1) wav_count--; // Revert count if init fails
        return;
    }
    
    if (positional) ma_sound_set_position(&wav_sounds[slot], pos.x, pos.y, pos.z);
    ma_sound_set_spatialization_enabled(&wav_sounds[slot], (ma_bool32)positional);
    wav_volumes[slot] = volume;
    ma_sound_set_volume(&wav_sounds[slot], GetSFXVolume(wav_volumes[slot]));
    ma_sound_start(&wav_sounds[slot]);
}

void play_message(const char* path) {
    if (ma_sound_is_playing(&log_sound)) { ma_sound_stop(&log_sound); ma_sound_uninit(&log_sound); }
    ma_result result = ma_sound_init_from_file(&Sys_Global.audio_engine, path, 0, NULL, NULL, &log_sound);
    if (result != MA_SUCCESS) { DualLog("ERROR: Failed to load message WAV %s: %d\n", path, result); return; }
    
    ma_sound_set_spatialization_enabled(&log_sound, false);
    ma_sound_set_volume(&log_sound, GetMessageVolume());
    ma_sound_start(&log_sound);
}

ENGINE_TO_MOD void SoundUninit(ma_sound* snd) { ma_sound_uninit(snd); }
ENGINE_TO_MOD ma_result SoundInit(const char* path, ma_uint32 flags, ma_sound_group* pGroup, ma_fence* pDoneFence, ma_sound* pSound) { return ma_sound_init_from_file(&Sys_Global.audio_engine,path,flags,pGroup,pDoneFence,pSound); }
ENGINE_TO_MOD void SoundSetLooping(ma_sound* pSound, ma_bool32 isLooping) { ma_sound_set_looping(pSound,isLooping); }
ENGINE_TO_MOD ma_result SoundStart(ma_sound* pSound) { return ma_sound_start(pSound); }
ENGINE_TO_MOD ma_result SoundStop(ma_sound* pSound) { return ma_sound_stop(pSound); }

ENGINE_TO_MOD float SoundGetLength(ma_sound* pSound) {
    if (!pSound) return 0.0f;
    
    ma_uint64 frames;
    if (ma_sound_get_length_in_pcm_frames(pSound, &frames) != MA_SUCCESS) return 0.0f;
    
    ma_uint32 sr = ma_engine_get_sample_rate(ma_sound_get_engine(pSound));
    return (sr == 0) ? 0.0f : (float)frames / (float)sr;
}

ENGINE_TO_MOD ma_result SoundGetCurrentFrameCursor(const ma_sound* pSound, ma_uint64* pCursor) { return ma_sound_get_cursor_in_pcm_frames(pSound,pCursor); }

void TextEntry(int32_t k) {
    if (k == GLFW_KEY_U && Sys_Input.keyStates[GLFW_KEY_LEFT_CONTROL].down) { Sys_Global.playerName[0] = '\0'; currentPlayerNameLength = 0; return; }
    if (k == GLFW_KEY_ENTER || k == GLFW_KEY_KP_ENTER) { currentMenuItem++; return; }
    if (k == GLFW_KEY_BACKSPACE && currentPlayerNameLength > 0) { Sys_Global.playerName[--currentPlayerNameLength] = '\0'; return; }
    if (currentPlayerNameLength >= 26) return;
    char c = (k >= GLFW_KEY_A && k <= GLFW_KEY_Z) ? 'a' + (k - GLFW_KEY_A) :
             (k >= GLFW_KEY_1 && k <= GLFW_KEY_9) ? '1' + (k - GLFW_KEY_1) :
             (k == GLFW_KEY_0)                    ? '0' : (k == GLFW_KEY_SPACE) ? ' ' : 0;
    if (c) { Sys_Global.playerName[currentPlayerNameLength] = c; Sys_Global.playerName[++currentPlayerNameLength] = '\0'; }
}

uint8_t UI_Button(int16_t x, int16_t y, float w, float h, bool* cursorOver, int8_t this) {
    float width = RelX(w); float height = RelY(h);
    float xpos = RelX(x); float ypos = RelY(y) - height;
    bool cursorIsOver = CursorIsOverBounds(xpos, xpos + width, ypos + height, ypos);
    if (cursorIsOver && mouseMovementThisFrame && cursorOver != NULL) {
        currentMenuItem = this;
        *cursorOver = cursorIsOver;
    }
    
    if (Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_LEFT ].pressed && cursorIsOver) return 1u;
    if (Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_RIGHT].pressed && cursorIsOver) return 2u;
    return 0u;
}

void RenderUIImage(int16_t x, int16_t y, int16_t width, int16_t height, uint32_t texIndex);
bool MenuEnter(void) { return (Sys_Input.keyStates[GLFW_KEY_KP_ENTER].pressed || Sys_Input.keyStates[GLFW_KEY_ENTER].pressed || Sys_Input.gamepadButtons[GLFW_GAMEPAD_BUTTON_A].pressed); }
uint8_t UI_MenuButton(int16_t bX, int16_t bY, uint8_t menuItem, int16_t bW, int16_t bH,  int16_t tX, int16_t tY, const char* text, int16_t pX, int16_t pY) {
    bool over = false; uint8_t retvalue = 0u;
    retvalue = UI_Button(bX,bY, bW,bH, &over, menuItem);
    if (!retvalue) retvalue = (MenuEnter() && currentMenuItem == menuItem);
    over = over || currentMenuItem == menuItem;
    RenderFormattedText(tX,tY, over ? TEXT_STOPD_RED : TEXT_RED_MENU,FONT_STOPD,1.5f,text);
    RenderUIImage(pX,pY, 40,40, over ? MENUPAD_HILITE : MENUPAD); // Menu pad
    return retvalue;
}

uint8_t UI_Slider(int16_t x, int16_t y, float w, float h, bool* cursorOver, int8_t this) {
    float width = RelX(w); float height = RelY(h);
    float xpos = RelX(x); float ypos = RelY(y) - height;
    bool cursorIsOver = CursorIsOverBounds(xpos, xpos + width, ypos + height, ypos);
    if (cursorIsOver && mouseMovementThisFrame) {
        currentMenuItem = this;
        if (cursorOver != NULL) *cursorOver = cursorIsOver;
    }
    
    if (Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_LEFT ].down && cursorIsOver) return 1u;
    if (Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_RIGHT].down && cursorIsOver) return 2u;
    return 0u;
}

void UI_HeaderText(int16_t x, const char* text) {
    RenderFormattedText(x,50,TEXT_GREEN_MENU_SHADOW,FONT_STOPD,1.75f,text);
    RenderFormattedText(x,46,TEXT_GREEN_MENU_GLOW,FONT_STOPD,1.75f,text);
    RenderFormattedText(x,48,TEXT_GREEN_MENU,FONT_STOPD,1.75f,text);
}

ENGINE_TO_MOD void MenuGoBack(void) {
    if (returnToPause) { returnToPause = false; Sys_Global.gamePaused = true; Sys_Global.menuActive = false; PlayGameMusic(); }
    if (currentMenuPage == MenuPages_Singleplayer || currentMenuPage == MenuPages_Multiplayer || currentMenuPage == MenuPages_Options) currentMenuPage = MenuPages_FrontPage;//News
    else if (currentMenuPage == MenuPages_Load || currentMenuPage == MenuPages_NewGame || currentMenuPage == MenuPages_IntroVideo || currentMenuPage == MenuPages_CreditsVideo) currentMenuPage = MenuPages_Singleplayer;
}

void ChangeMenuPage(MenuPages pg) { currentMenuPage = pg; currentMenuItem = currentMenuTab = 0; }

void RenderMenu(void) {
    if (Sys_Input.gamepadButtons[GLFW_GAMEPAD_BUTTON_B].pressed && currentMenuPage != MenuPages_FrontPage) { MenuGoBack(); return; }
    
    if (currentMenuPage != MenuPages_IntroVideo && currentMenuPage != MenuPages_CreditsVideo && currentMenuPage != MenuPages_Options) RenderUIImage(-417,-384, 2200,1536, 1026); // Menu background
    if (currentMenuPage == MenuPages_IntroVideo || currentMenuPage == MenuPages_CreditsVideo) RenderUIImage(-417,-384, 2200,1536, 0); // Video blackground
    if (currentMenuPage == MenuPages_Options) RenderUIImage(-417,-384, 2200,1536, 1032); // Menu background
    bool shiftHeld = Sys_Input.keyStates[GLFW_KEY_LEFT_SHIFT].down || Sys_Input.keyStates[GLFW_KEY_RIGHT_SHIFT].down;
    if (currentMenuPage == MenuPages_FrontPage) {
        menuItemCount = 4; menuTabCount = 1;
        RenderUIImage(282,46, 800,128, 1031); // Title CITADEL with strikethrough effect
        if (UI_MenuButton(408,340,0,574,84, 304,188,/*"SINGLEPLAYER"*/Sys_Text.stringTable[719],413,276)) ChangeMenuPage(MenuPages_Singleplayer);
        if (UI_MenuButton(408,458,1,574,84, 304,268,/*"MULTIPLAYER"*/Sys_Text.stringTable[720], 413,396)) ChangeMenuPage(MenuPages_Multiplayer);
        if (UI_MenuButton(408,582,2,574,84, 304,350,/*"OPTIONS"*/Sys_Text.stringTable[721],     413,520)) ChangeMenuPage(MenuPages_Options);
        if (UI_MenuButton(408,702,3,574,84, 304,430,/*"QUIT"*/Sys_Text.stringTable[722],        413,638)) OS_Exit(0);
    } else if (currentMenuPage == MenuPages_Singleplayer) {
        menuItemCount = 5; menuTabCount = 1;
        UI_HeaderText(250,/*"SINGLEPLAYER"*/Sys_Text.stringTable[719]);
        if (UI_MenuButton(408,340,0,574,84, 304,188,/*"CONTINUE"*/Sys_Text.stringTable[723],    413,276)) ChangeMenuPage(MenuPages_Load);
        if (UI_MenuButton(408,458,1,574,84, 304,268,/*"NEW GAME"*/Sys_Text.stringTable[741],    413,396)) ChangeMenuPage(MenuPages_NewGame);
        if (UI_MenuButton(408,582,2,574,84, 304,350,/*"PLAY INTRO"*/Sys_Text.stringTable[742],  413,520)) ChangeMenuPage(MenuPages_IntroVideo);
        if (UI_MenuButton(408,702,3,574,84, 304,430,/*"PLAY CREDITS"*/Sys_Text.stringTable[743],413,638)) ChangeMenuPage(MenuPages_CreditsVideo);
        RenderUIImage(1060,724, 84,36, 1252); // Back Button background
        bool overBack = false;        
        if (UI_Button(1060,758, 84,32, &overBack, 4) || (MenuEnter() && currentMenuItem == 4)) MenuGoBack();
        overBack = overBack || currentMenuItem == 4;
        RenderFormattedText(1076,732,overBack ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_RED_MENU,FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == MenuPages_Multiplayer) {
        menuItemCount = 1; menuTabCount = 1;
        UI_HeaderText(266,/*"MULTIPLAYER"*/Sys_Text.stringTable[720]);
        RenderUIImage(1060,724, 84,36, 1252); // Back Button background
        bool overBack = false;
        if (UI_Button(1060,758, 84,32, &overBack, 0) || (MenuEnter() && currentMenuItem == 0)) MenuGoBack();
        overBack = overBack || currentMenuItem == 0;
        RenderFormattedText(1076,732,overBack ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_RED_MENU,FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == MenuPages_Options) {
        menuTabCount = 3;
        UI_HeaderText(238,/*"CONFIGURATION"*/Sys_Text.stringTable[745]);
        if (currentMenuTab != 0) RenderUIImage(179,220, 1001,548, 1030); // Config background
        if (currentMenuTab == 0) RenderUIImage(179,220, 1001,548, 1033); // Config background graphics (empty alpha center)
        RenderUIImage(520,196, 160,30, currentMenuTab == 2 ? 920 : 921); // Config tab unhighlighted
        if (UI_Button(520,196+30, 160,30, NULL, 2)) currentMenuTab = 2;
        RenderFormattedText(530,202,currentMenuTab == 2 ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,"AUDIO / LANG");
        RenderUIImage(354,196, 160,30, currentMenuTab == 1 ? 920 : 921); // Config tab unhighlighted
        if (UI_Button(354,196+30, 160,30, NULL, 1)) currentMenuTab = 1;
        RenderFormattedText(366,202,currentMenuTab == 1 ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,"INPUT");
        RenderUIImage(190,196, 160,30, currentMenuTab == 0 ? 920 : 921); // Config tab highlighted
        if (UI_Button(190,196+30, 160,30, NULL, 0)) currentMenuTab = 0;
        RenderFormattedText(200,202,currentMenuTab == 0 ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,"GRAPHICS");
        if (currentMenuTab == 0) {
            bool overDetails = false, overAA = false, overShadows = false, overSSR = false, overVsync = false, overFOV = false, overBrightness = false, overRes = false, overFull = false, overChgM = false;
            menuItemCount = 11; // Graphics            
            RenderUIImage(200,500, 16,16, 910); // Checkbox background
            if (UI_Button(200,516, 210,16, &overDetails, 0) || (MenuEnter() && currentMenuItem == 0)) { Sys_Settings.ModelDetail = Sys_Settings.ModelDetail == 1u ? 0u : 1u; SaveConfig(); }
            overDetails = overDetails || currentMenuItem == 0;
            if (Sys_Settings.ModelDetail) RenderUIImage(202,502, 12,12, 912); // Checkbox check
            RenderFormattedText(220,500,overDetails ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,"DETAILED MODELS");
            
            RenderUIImage(200,530, 16,16, 910); // Checkbox background
            if (UI_Button(200,546, 210,16, &overAA, 1) || (MenuEnter() && currentMenuItem == 1)) { Sys_Settings.AntiAliasing = Sys_Settings.AntiAliasing == 1u ? 0u : 1u; SaveConfig(); }
            overAA = overAA || currentMenuItem == 1;
            if (Sys_Settings.AntiAliasing) RenderUIImage(202,532, 12,12, 912); // Checkbox check
            RenderFormattedText(220,530,overAA ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,"ANTIALIASING");
            
            RenderUIImage(200,560, 16,16, 910); // Checkbox background
            if (UI_Button(200,576, 210,16, &overShadows, 2) || (MenuEnter() && currentMenuItem == 2)) { Sys_Settings.Shadows = Sys_Settings.Shadows == 1u ? 0u : 1u; SaveConfig(); }
            overShadows = overShadows || currentMenuItem == 2;
            if (Sys_Settings.Shadows) RenderUIImage(202,562, 12,12, 912); // Checkbox check
            RenderFormattedText(220,560,overShadows ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,"SHADOWS");
            
            RenderUIImage(200,590, 16,16, 910); // Checkbox background
            if (UI_Button(200,606, 210,16, &overSSR, 3) || (MenuEnter() && currentMenuItem == 3)) { Sys_Settings.Reflections = Sys_Settings.Reflections == 1u ? 0u : 1u; SaveConfig(); }
            overSSR = overSSR || currentMenuItem == 3;
            if (Sys_Settings.Reflections) RenderUIImage(202,592, 12,12, 912); // Checkbox check
            RenderFormattedText(220,590,overSSR ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,"REFLECTIONS");
            
            RenderUIImage(200,620, 16,16, 910); // Checkbox background
            if (UI_Button(200,636, 210,16, &overVsync, 4) || (MenuEnter() && currentMenuItem == 4)) { Sys_Settings.Vsync = Sys_Settings.Vsync == 1u ? 0u : 1u; SetVSync(); SaveConfig(); }
            overVsync = overVsync || currentMenuItem == 4;
            if (Sys_Settings.Vsync) RenderUIImage(202,622, 12,12, 912); // Checkbox check
            RenderFormattedText(220,620,overVsync ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,"VSYNC (FPS: %d)", Sys_Global.framesPerLastSecond);
            
            // FOV Slider
            RenderUIImage(400,650, 128,16, 1079); // Slider background
            RenderUIImage(400 + (((Sys_Settings.FOV - 45.0f) / 105.0f) * 112),650, 16,16, 1078); // Slider handle [45, 150]
            if (UI_Slider(200,666, 328,16, &overFOV, 5)) fovSliderActive = true;
            if (fovSliderActive && Sys_Input.currentMouse_dx != 0) {
                int32_t new = (int32_t)Sys_Settings.FOV + vmin(vmax(Sys_Input.currentMouse_dx,-1),1); Sys_Settings.FOV = (uint8_t)vmin(vmax(new,45),150);
                UpdateProjectionMatrices();
            }
            
            if (!Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_LEFT].down && !Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_RIGHT].down) {
                if (fovSliderActive) SaveConfig();
                fovSliderActive = false;
            }
            
            if (MenuEnter() && currentMenuItem == 5) {
                if (shiftHeld) Sys_Settings.FOV = Sys_Settings.FOV <=  49 ? 150 : Sys_Settings.FOV - 5;
                else           Sys_Settings.FOV = Sys_Settings.FOV >= 146 ?  45 : Sys_Settings.FOV + 5;
                UpdateProjectionMatrices(); SaveConfig();
            }
            
            overFOV = overFOV || currentMenuItem == 5;
            RenderFormattedText(200,650,overFOV ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,"%s %u",/*Field of View*/Sys_Text.stringTable[775],Sys_Settings.FOV);
            
            // Brightness Slider
            RenderUIImage(400,680, 128,16, 1079); // Slider background
            RenderUIImage(400 + ((Sys_Settings.Brightness / 100.0f) * 112),680, 16,16, 1078); // Slider handle [45, 150]
            if (UI_Slider(200,696, 328,16, &overBrightness, 6)) gammaSliderActive = true;
            if (gammaSliderActive && Sys_Input.currentMouse_dx != 0) {
                int32_t new = (int32_t)Sys_Settings.Brightness + vmin(vmax(Sys_Input.currentMouse_dx,-1),1); Sys_Settings.Brightness = (uint8_t)vmin(vmax(new,0),100);
            }
            
            if (!Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_LEFT].down && !Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_RIGHT].down) {
                if (gammaSliderActive) SaveConfig();
                gammaSliderActive = false;
            }
            
            if (MenuEnter() && currentMenuItem == 6) {
                if (shiftHeld) Sys_Settings.Brightness = Sys_Settings.Brightness <=  1 ? 100 : Sys_Settings.Brightness - 2;
                else           Sys_Settings.Brightness = Sys_Settings.Brightness >= 99 ?   0 : Sys_Settings.Brightness + 2;
                SaveConfig();
            }
            
            overBrightness = overBrightness || currentMenuItem == 6;
            RenderFormattedText(200,680,overBrightness ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,"%s %u",/*Brightness*/Sys_Text.stringTable[771],Sys_Settings.Brightness);
            
            // Resolution
            {
                // Header hit area - UI_Button subtracts h from y internally, so pass y+h as y
                bool headerClick = (UI_Button(190, (int16_t)(710 + 16), 328, 16, &overRes, 7) != 0)
                                || (MenuEnter() && currentMenuItem == 7);
                overRes = overRes || currentMenuItem == 7;

                char resBuf[32];
                if (resDropdownCount > 0)
                    StringFormat(resBuf, sizeof(resBuf), "%ux%u %uHz",
                        Sys_Settings.ScreenWidth, Sys_Settings.ScreenHeight,
                        (uint32_t)resModes[resSelectedIdx].hz);
                else
                    StringFormat(resBuf, sizeof(resBuf), "%ux%u",
                        Sys_Settings.ScreenWidth, Sys_Settings.ScreenHeight);

                RenderUIImage(476, 710, 16, 16, overRes ? 1119 : 1077);
                RenderFormattedText(200, 710, overRes ? TEXT_YELLOW : TEXT_GREEN,
                                    FONT_NORMAL, 1.0f, "RESOLUTION %s", resBuf);

                if (headerClick) {
                    resDropdownOpen = !resDropdownOpen;
                    currentMenuItem = 7;
                }

                if (resDropdownOpen) {
                    // Items stack upward: item 0 (largest res) is furthest up, item N-1 nearest header
                    // Each item is 24px tall (matching texture 1120 which is 108x24)
                    // Item i draws at baseY = 710 - (resDropdownCount - i) * 24
                    // so item 0 is highest (smallest Y), last item is just above header
                    for (int i = 0; i < resDropdownCount; ++i) {
                        int16_t itemBaseY = (int16_t)(710 - (resDropdownCount - i) * 24);
                        bool overItem = false;
                        // UI_Button(x, y, w, h) — y is the BOTTOM of the hit zone in your coord system
                        // because it does: ypos = RelY(y) - RelY(h), then checks cursor >= ypos && <= ypos+h
                        // So pass itemBaseY + 24 as y so the hit zone covers [itemBaseY, itemBaseY+24]
                        bool clicked = (UI_Button(190, (int16_t)(itemBaseY + 24), 328, 24, &overItem, (int8_t)(10 + i)) != 0);
                        bool isEnterSelected = (MenuEnter() && currentMenuItem == (int8_t)(10 + i));

                        bool isSelected = (i == resSelectedIdx);
                        uint8_t color = isSelected   ? TEXT_YELLOW
                                    : overItem     ? TEXT_GREEN
                                                    : TEXT_WHITE;

                        // Background image for this item (108x24 → scale to 328x24 to match hit width)
                        RenderUIImage(190, (int16_t)(itemBaseY + 24), 328, 24, 1120);

                        char itemBuf[32];
                        StringFormat(itemBuf, sizeof(itemBuf), "%dx%d %dHz",
                            resModes[i].w, resModes[i].h, resModes[i].hz);
                        // Text sits 4px below itemBaseY for vertical centering within 24px item
                        RenderFormattedText(200, (int16_t)(itemBaseY + 4), color, FONT_NORMAL, 1.0f, "%s", itemBuf);

                        if (clicked || isEnterSelected) {
                            resSelectedIdx = i;
                            Sys_Settings.ScreenWidth  = (uint32_t)resModes[i].w;
                            Sys_Settings.ScreenHeight = (uint32_t)resModes[i].h;

                            GLFWmonitor* monitor = GetCurrentMonitor();
                            if (!monitor) monitor = glfwGetPrimaryMonitor();
                            int mx, my;
                            glfwGetMonitorPos(monitor, &mx, &my);
                            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
                            int xpos = mx + (mode->width  - (int)Sys_Settings.ScreenWidth)  / 2;
                            int ypos = my + (mode->height - (int)Sys_Settings.ScreenHeight) / 2;
                            glfwSetWindowSize(window, (int)Sys_Settings.ScreenWidth,
                                                    (int)Sys_Settings.ScreenHeight);
                            glfwSetWindowPos(window, xpos, ypos);
                            UpdateScreenSize(NULL, (int)Sys_Settings.ScreenWidth,
                                                (int)Sys_Settings.ScreenHeight);
                            Sys_Input.ignore_next_mouse_delta = true;
                            resDropdownOpen = false;
                            SaveConfig();
                            GatherResolutionModes();
                            break;
                        }
                    }

                    // Arrow key navigation within dropdown
                    if (Sys_Input.keyStates[GLFW_KEY_UP].pressed && currentMenuItem >= 10 && currentMenuItem > 10)
                        currentMenuItem--;
                    else if (Sys_Input.keyStates[GLFW_KEY_UP].pressed && currentMenuItem == 10)
                        currentMenuItem = (int8_t)(10 + resDropdownCount - 1);
                    if (Sys_Input.keyStates[GLFW_KEY_DOWN].pressed && currentMenuItem >= 10 && currentMenuItem < (int8_t)(10 + resDropdownCount - 1))
                        currentMenuItem++;
                    else if (Sys_Input.keyStates[GLFW_KEY_DOWN].pressed && currentMenuItem == (int8_t)(10 + resDropdownCount - 1))
                        currentMenuItem = 10;

                    // Escape or click outside closes
                    if (Sys_Input.keyStates[GLFW_KEY_ESCAPE].pressed)
                        resDropdownOpen = false;
                    if (Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_LEFT].pressed
                        && currentMenuItem < 10 && currentMenuItem != 7)
                        resDropdownOpen = false;
                }
            }
    
            // Fullscreen checkbox
            RenderUIImage(200,740, 16,16, 910); // Checkbox background
            if (UI_Button(200,756, 210,16, &overFull, 8) || (MenuEnter() && currentMenuItem == 8)) {
                Sys_Settings.Fullscreen = Sys_Settings.Fullscreen == 1u ? 0u : 1u;
                ChangeFullScreenWindowed();
                SaveConfig();
            }
            
            overFull = overFull || currentMenuItem == 8;
            if (Sys_Settings.Fullscreen) RenderUIImage(202,742, 12,12, 912); // Checkbox check
            RenderFormattedText(220,740,overFull ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,/*"Fullscreen"*/Sys_Text.stringTable[773]);
            
            RenderUIImage(588,730, 210,30, 1079); // Toggle monitor button background
            if (UI_Button(588,760, 210,30, &overChgM, 9) || (MenuEnter() && currentMenuItem == 9)) { CycleToNextMonitor(); }
            overChgM = overChgM || currentMenuItem == 9;
            RenderFormattedText(602,735,overChgM ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,/*"CHANGE MONITOR"*/Sys_Text.stringTable[1025]);
        } else if (currentMenuTab == 1) {
            menuItemCount = 49; // Input
        } else {
            bool overMasterVolume = false, overMusicSlider = false;
            menuItemCount = 10; // Audio / Lang
            // Master Volume Slider
            RenderUIImage(426,240, 128,16, 1079); // Slider background
            RenderUIImage(426 + ((Sys_Settings.VolumeMaster / 100.0f) * 112),240, 16,16, 1078); // Slider handle [45, 150]
            if (UI_Slider(200,256, 328,16, &overMasterVolume, 0)) masterVolumeSliderActive = true;
            if (masterVolumeSliderActive && Sys_Input.currentMouse_dx != 0) {
                int32_t new = (int32_t)Sys_Settings.VolumeMaster + vmin(vmax(Sys_Input.currentMouse_dx,-1),1); Sys_Settings.VolumeMaster = (uint8_t)vmin(vmax(new,0),100); set_master_volume();
            }
            
            if (!Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_LEFT].down && !Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_RIGHT].down) {
                if (masterVolumeSliderActive) SaveConfig();
                masterVolumeSliderActive = false;
            }
            
            if (MenuEnter() && currentMenuItem == 0) {
                if (shiftHeld) Sys_Settings.VolumeMaster = Sys_Settings.VolumeMaster <=  4 ? 100 : Sys_Settings.VolumeMaster - 5;
                else           Sys_Settings.VolumeMaster = Sys_Settings.VolumeMaster >= 96 ?   0 : Sys_Settings.VolumeMaster + 5;
                set_master_volume();
                SaveConfig();
            }
            
            overMasterVolume = overMasterVolume || currentMenuItem == 0;
            RenderFormattedText(200,240,overMasterVolume ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,"%s %u",/*Master Volume*/Sys_Text.stringTable[802],Sys_Settings.VolumeMaster);
            
            // Music Volume Slider
            RenderUIImage(426,270, 128,16, 1079); // Slider background
            RenderUIImage(426 + ((Sys_Settings.VolumeMusic / 100.0f) * 112),270, 16,16, 1078); // Slider handle [45, 150]
            if (UI_Slider(200,286, 328,16, &overMusicSlider, 1)) musicVolumeSliderActive = true;
            if (musicVolumeSliderActive && Sys_Input.currentMouse_dx != 0) {
                int32_t new = (int32_t)Sys_Settings.VolumeMusic + vmin(vmax(Sys_Input.currentMouse_dx,-1),1); Sys_Settings.VolumeMusic = (uint8_t)vmin(vmax(new,0),100); set_music_volume();
            }
            
            if (!Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_LEFT].down && !Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_RIGHT].down) {
                if (musicVolumeSliderActive) SaveConfig();
                musicVolumeSliderActive = false;
            }
            
            if (MenuEnter() && currentMenuItem == 1) {
                if (shiftHeld) Sys_Settings.VolumeMusic = Sys_Settings.VolumeMusic <=  4 ? 100 : Sys_Settings.VolumeMusic - 5;
                else           Sys_Settings.VolumeMusic = Sys_Settings.VolumeMusic >= 96 ?   0 : Sys_Settings.VolumeMusic + 5;
                set_music_volume();
                SaveConfig();
            }
            
            overMusicSlider = overMusicSlider || currentMenuItem == 1;
            RenderFormattedText(200,270,overMusicSlider ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,"%s %u",/*Music Volume*/Sys_Text.stringTable[803],Sys_Settings.VolumeMusic);
        }
        
        RenderUIImage(1087,723, 84,36, 1252); // Back Button background
        int8_t lastItem = menuItemCount - 1;
        bool overBack = false;
        if (UI_Button(1087,757, 84,32, &overBack, lastItem) || (MenuEnter() && currentMenuItem == lastItem)) MenuGoBack();
        overBack = overBack || currentMenuItem == lastItem;
        RenderFormattedText(1103,731,overBack ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_RED_MENU,FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == MenuPages_Load || currentMenuPage == MenuPages_Save) {
        menuItemCount = 9; menuTabCount = 1;
        bool isSave = currentMenuPage == MenuPages_Save;
        UI_HeaderText(isSave ? 284 : 340, isSave ? /*"SAVE GAME"*/Sys_Text.stringTable[769] : /*"LOAD"*/Sys_Text.stringTable[726]);
        RenderUIImage(400,214, 586,500, 1037); // Load/Save table background
        RenderUIImage(1060,724, 84,36, 1252); // Back Button background
        bool overBack = false;
        if (UI_Button(1060,758, 84,32, &overBack, 0) || (MenuEnter() && currentMenuItem == 0)) MenuGoBack();
        overBack = overBack || currentMenuItem == 0;
        RenderFormattedText(1076,732, overBack ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_RED_MENU, FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == MenuPages_NewGame) {
        menuItemCount = 7;
        menuTabCount = (currentMenuItem > 0 && currentMenuItem <= 16) ? 2 : 1;
        UI_HeaderText(290,/*"NEW GAME"*/Sys_Text.stringTable[741]);
        RenderUIImage(136,196,1088,558,1048); // Newgame inset
        RenderUIImage(136,196,1088,558,1049); // Newgame background
        if (UI_MenuButton(276,270,0,795,74, 226,146,/*"NAME:"*/Sys_Text.stringTable[746],299,214)) { /* Just for highlight */ }
        enteringPlayerName = (currentMenuItem == 0);
        if (Sys_Global.playerName[0] == '\0') RenderFormattedText(642,232,TEXT_RED_MENU,FONT_STOPD,1.0f,/*"ENTER NAME..."*/Sys_Text.stringTable[748]);
        else                                  RenderFormattedText(518,232,enteringPlayerName ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.0f,Sys_Global.playerName);

        if (UI_MenuButton(174,377,1,496,95, 148,202,/*"COMBAT"*/Sys_Text.stringTable[748],185,299)) { Sys_Global.difficultyCombat = Sys_Global.difficultyCombat >= 3 ? 0 : Sys_Global.difficultyCombat + 1; }  if (UI_MenuButton(704,377,3,496,95, 510,202,/*"MISSION"*/Sys_Text.stringTable[749],726,299)) { Sys_Global.difficultyMission = Sys_Global.difficultyMission >= 3 ? 0 : Sys_Global.difficultyMission + 1; }
        RenderFormattedText(162,270,Sys_Global.difficultyCombat == 0 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"0");    RenderFormattedText(513,270,Sys_Global.difficultyMission == 0 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"0");
        RenderFormattedText(233,270,Sys_Global.difficultyCombat == 1 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"1");    RenderFormattedText(584,270,Sys_Global.difficultyMission == 1 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"1");
        RenderFormattedText(307,270,Sys_Global.difficultyCombat == 2 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"2");    RenderFormattedText(658,270,Sys_Global.difficultyMission == 2 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"2");
        RenderFormattedText(379,270,Sys_Global.difficultyCombat == 3 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"3");    RenderFormattedText(730,270,Sys_Global.difficultyMission == 3 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"3");
        if (UI_MenuButton(174,568,2,496,92, 149,330,/*"PUZZLE"*/Sys_Text.stringTable[751],185,490)) { Sys_Global.difficultyPuzzle = Sys_Global.difficultyPuzzle >= 3 ? 0 : Sys_Global.difficultyPuzzle + 1; }  if (UI_MenuButton(704,568,4,496,92, 509,330,/*"CYBERSPACE"*/Sys_Text.stringTable[750],726,490)) { Sys_Global.difficultyCyber = Sys_Global.difficultyCyber >= 3 ? 0 : Sys_Global.difficultyCyber + 1; }
        RenderFormattedText(162,399,Sys_Global.difficultyPuzzle == 0 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"0");    RenderFormattedText(513,399,Sys_Global.difficultyCyber == 0 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"0");
        RenderFormattedText(233,399,Sys_Global.difficultyPuzzle == 1 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"1");    RenderFormattedText(584,399,Sys_Global.difficultyCyber == 1 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"1");
        RenderFormattedText(307,399,Sys_Global.difficultyPuzzle == 2 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"2");    RenderFormattedText(658,399,Sys_Global.difficultyCyber == 2 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"2");
        RenderFormattedText(379,399,Sys_Global.difficultyPuzzle == 3 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"3");    RenderFormattedText(730,399,Sys_Global.difficultyCyber == 3 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"3");
        if (UI_Button(221,460,82,79,NULL,1)) {Sys_Global.difficultyCombat =0; currentMenuItem=1; } if (UI_Button(330,460,82,79,NULL,1)) {Sys_Global.difficultyCombat =1; currentMenuItem=1; } if (UI_Button(439,460,82,79,NULL,1)) {Sys_Global.difficultyCombat =2; currentMenuItem=1; } if (UI_Button( 547,460,82,79,NULL,1)) {Sys_Global.difficultyCombat =3; currentMenuItem=1; }
        if (UI_Button(221,651,82,79,NULL,2)) {Sys_Global.difficultyPuzzle =0; currentMenuItem=2; } if (UI_Button(330,651,82,79,NULL,2)) {Sys_Global.difficultyPuzzle =1; currentMenuItem=2; } if (UI_Button(439,651,82,79,NULL,2)) {Sys_Global.difficultyPuzzle =2; currentMenuItem=2; } if (UI_Button( 547,651,82,79,NULL,2)) {Sys_Global.difficultyPuzzle =3; currentMenuItem=2; }
        if (UI_Button(748,460,82,79,NULL,3)) {Sys_Global.difficultyMission=0; currentMenuItem=3; } if (UI_Button(857,460,82,79,NULL,3)) {Sys_Global.difficultyMission=1; currentMenuItem=3; } if (UI_Button(966,460,82,79,NULL,3)) {Sys_Global.difficultyMission=2; currentMenuItem=3; } if (UI_Button(1074,460,82,79,NULL,3)) {Sys_Global.difficultyMission=3; currentMenuItem=3; }
        if (UI_Button(748,651,82,79,NULL,4)) {Sys_Global.difficultyCyber  =0; currentMenuItem=4; } if (UI_Button(857,651,82,79,NULL,4)) {Sys_Global.difficultyCyber  =1; currentMenuItem=4; } if (UI_Button(966,651,82,79,NULL,4)) {Sys_Global.difficultyCyber  =2; currentMenuItem=4; } if (UI_Button(1074,651,82,79,NULL,4)) {Sys_Global.difficultyCyber  =3; currentMenuItem=4; }
        bool overBack = false, overStart = false;
        if (UI_Button(544,747, 282,68, &overStart, 5) || (MenuEnter() && currentMenuItem == 5)) GoIntoGame(); // TODO reload game.
        overStart = overStart || currentMenuItem == 5;
        RenderFormattedText(400,464,overStart ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,/*"START"*/Sys_Text.stringTable[886]);
        
        if (UI_Button(1060,758, 84,32, &overBack, 6) || (MenuEnter() && currentMenuItem == 6)) MenuGoBack();
        overBack = overBack || currentMenuItem == 6;
        RenderUIImage(1060,724,84,36,1252); // Back Button background
        RenderFormattedText(1076,732,overBack ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_RED_MENU,FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == MenuPages_IntroVideo) {
        menuItemCount = 1; menuTabCount = 1;
        if (MenuEnter()) MenuGoBack();
    } else if (currentMenuPage == MenuPages_CreditsVideo) {
        menuItemCount = 1; menuTabCount = 1;
        if (MenuEnter()) MenuGoBack();
    }
    
    if (menuTabCount <= currentMenuTab) currentMenuTab = 0;
    if (menuItemCount <= currentMenuItem) currentMenuItem = 0;
    static const int8_t ngSwap[7] = {0, 3, 4, 1, 2, 6, 5};
    if (Sys_Input.keyStates[GLFW_KEY_RIGHT].pressed || Sys_Input.keyStates[GLFW_KEY_LEFT].pressed) {
        int dir = Sys_Input.keyStates[GLFW_KEY_RIGHT].pressed ? 1 : -1;
        currentMenuTab = (currentMenuTab + menuTabCount + dir) % menuTabCount;
        if (currentMenuPage == MenuPages_NewGame && currentMenuItem < 7) currentMenuItem = ngSwap[currentMenuItem];
    }
}

void RenderPausedUI(void) {
    menuItemCount = 6; menuTabCount = 1;
    bool overResume = false, overLoad /* ;) */ = false, overSave = false, overOptions = false, overQuitMenu = false, overQuit = false;
    RenderUIImage(519,276,328,300,1025); // Pause Menu background
    RenderUIImage(519,276,328,300,1080); // Pause Menu background outline
    RenderFormattedText(610,210,TEXT_STOPD_RED_PAUSETITLE,FONT_STOPD,1.0f,/*"PAUSED"*/Sys_Text.stringTable[724]);
    if (UI_Button(522,330, 322,52, &overResume, 0) || (MenuEnter() && currentMenuItem == 0)) Sys_Global.gamePaused = false;
    overResume = overResume || currentMenuItem == 0;
    RenderFormattedText(610,306,overResume ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.0f,/*"RESUME"*/Sys_Text.stringTable[725]);
    if (UI_Button(522,390, 322,52, &overLoad, 1) || (MenuEnter() && currentMenuItem == 1)) { currentMenuPage = MenuPages_Load; PlayMenuMusic(); Sys_Global.menuActive = true; returnToPause = true; }
    overLoad = overLoad || currentMenuItem == 1;
    RenderFormattedText(630,364, overLoad ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.0f,/*"LOAD"*/Sys_Text.stringTable[726]);
    if (UI_Button(522,450, 322,60, &overSave, 2) || (MenuEnter() && currentMenuItem == 2)) { currentMenuPage = MenuPages_Save; PlayMenuMusic(); Sys_Global.menuActive = true; returnToPause = true; }
    overSave = overSave || currentMenuItem == 2;
    RenderFormattedText(635,422,overSave ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.0f,/*"SAVE"*/Sys_Text.stringTable[727]);
    if (UI_Button(522,510, 322,60, &overOptions, 3) || (MenuEnter() && currentMenuItem == 3)) { currentMenuPage = MenuPages_Options; PlayMenuMusic(); Sys_Global.menuActive = true; returnToPause = true; }
    overOptions = overOptions || currentMenuItem == 3;
    RenderFormattedText(599,480,overOptions ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.0f,/*"OPTIONS"*/Sys_Text.stringTable[721]);
    if (UI_Button(522,570, 322,60, &overQuitMenu, 4) || (MenuEnter() && currentMenuItem == 4)) { PlayMenuMusic(); Sys_Global.menuActive = true; currentMenuPage = MenuPages_FrontPage; }
    overQuitMenu = overQuitMenu || currentMenuItem == 4;
    RenderFormattedText(546,538,overQuitMenu ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.0f,/*"QUIT TO MENU"*/Sys_Text.stringTable[728]);
    RenderUIImage(519,672,328,42,1252); // Pause Quit Game background
    if (UI_Button(522,714, 322,42, &overQuit, 5) || (MenuEnter() && currentMenuItem == 5)) OS_Exit(0);
    overQuit = overQuit || currentMenuItem == 5;
    RenderFormattedText(572,690,overQuit ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.0f,/*"QUIT GAME"*/Sys_Text.stringTable[729]);
}

float debugLineBuffer[MAX_DEBUG_LINE_VERTS * 3]; // xyz only
static inline __attribute__((always_inline)) void DrawDebugLines(float* viewProj) {    
    glNamedBufferSubData(Sys_Render.debugLinesVBO, 0, Sys_Global.debugLineVertCount * sizeof(float), debugLineBuffer);
    glUseProgram(Sys_Render.debugUnlitShaderProgram);
    glUniformMatrix4fv(0, 1, GL_FALSE, viewProj);
    glLineWidth(10.0f);
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(Sys_Render.debugLinesVAO);
    glDrawArrays(GL_LINES, 0, Sys_Global.debugLineVertCount / 3);
    glEnable(GL_DEPTH_TEST);
    drawCallsRenderedThisFrame++; verticesRenderedThisFrame += Sys_Global.debugLineVertCount / 3;
    Sys_Global.debugLineVertCount = 0;
}

ENGINE_TO_MOD void AddDebugLine(Vector3 start, Vector3 end) {
    int32_t i = Sys_Global.debugLineVertCount;
    debugLineBuffer[i++] = start.x; debugLineBuffer[i++] = start.y; debugLineBuffer[i++] = start.z;
    debugLineBuffer[i++] =   end.x; debugLineBuffer[i++] =   end.y; debugLineBuffer[i++] =   end.z;
    Sys_Global.debugLineVertCount = i;
}


char creditStats[4096];
static inline __attribute__((always_inline)) float GetScore(float stupid, bool isFinal) {
    float victories = (float)(Sys_Global.kills + Sys_Global.cyberkills);
    if (isFinal) victories -= vmin(Sys_Global.ressurections * 10.0f, victories * 0.666f);
    float secs  = vfloor((float)Sys_Global.pauseRelativeTime / 3600.0f);
    float score = victories * 10000.0f;
    score -= vmin(score * 0.666f, secs * 100.0f);
    score *= (stupid + 1.0f) / 37.0f;
    if (stupid > 35.0f) score += 2222222.0f;
    return vfloor(score);
}

static inline void DecomposeTime(double t, uint32_t* h, uint32_t* m, double* s) {
    double tb = vfloor(t / 3600.0); *h = (uint32_t)tb; t -= tb * 3600.0;
    tb = vfloor(t / 60.0);          *m = (uint32_t)tb; *s = t - tb * 60.0;
}

static inline __attribute__((always_inline)) void CreditsStats(void) {
    size_t off = 0;
    off += StringFormat(creditStats + off, sizeof(creditStats),"================================================================================\nCITADEL\n================================================================================\nCONGRATULATIONS %s\n",Sys_Global.playerName);
    uint32_t h,m; double s;
    DecomposeTime(Sys_Global.pauseRelativeTime,&h,&m,&s);
    off += StringFormat(creditStats + off, sizeof(creditStats),"Straight Time: %uh %um %.3fs\n",h,m,s);
    DecomposeTime(Sys_Global.absoluteTime,&h,&m,&s);
    off += StringFormat(creditStats + off,sizeof(creditStats),"Total Time (with reload from deaths): %uh %um %.3fs\n",h,m,s);
    float stupid = ((float)(Sys_Global.difficultyCombat * Sys_Global.difficultyCombat)) + ((float)(Sys_Global.difficultyPuzzle * Sys_Global.difficultyPuzzle)) + ((float)(Sys_Global.difficultyMission * Sys_Global.difficultyMission)) + ((float)(Sys_Global.difficultyCyber * Sys_Global.difficultyCyber));
    uint32_t finalSubscore = GetScore(stupid,false);
    off += StringFormat(creditStats + off,sizeof(creditStats),"Kills: %u\nKills in Cyberspace: %u\nScoreSubtotal: %u\nDeaths: %u\nRessurections: %u\n",Sys_Global.kills,Sys_Global.cyberkills,(uint32_t)finalSubscore,Sys_Global.deaths,Sys_Global.ressurections);
    off += StringFormat(creditStats + off,sizeof(creditStats),"Combat: %u | Puzzle: %u | Mission: %u | Cyber: %u\n",Sys_Global.difficultyCombat,Sys_Global.difficultyPuzzle,Sys_Global.difficultyMission,Sys_Global.difficultyCyber);
    uint32_t finalScore = (uint32_t)GetScore(stupid,true);
    off += StringFormat(creditStats + off,sizeof(creditStats),"Difficulty Index: %.2f\nFinal Score: %u\n\n",stupid,finalScore);
    off += StringFormat(creditStats + off,sizeof(creditStats),"Shots Fired: %u\nGrenades Thrown: %u\n",Sys_Global.shotsFired,Sys_Global.grenadesThrown);
    off += StringFormat(creditStats + off,sizeof(creditStats),"Damage Dealt: %f\nDamage Received: %f\nSaves Scummed: %u\n\nClick to continue...\n",Sys_Global.damageDealt,Sys_Global.damageReceived,Sys_Global.savesScummed);
}

void RenderCredits(void) {
    if (Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_LEFT].pressed) {
        ++Sys_Global.creditsPageIndex;
        if (Sys_Global.creditsPageIndex > CREDITS_PAGES) {Sys_Global.creditsActive = false; return; }
    }

    if (Sys_Global.creditsPageIndex == 1) {
        CreditsStats();
        RenderFormattedText(300,10,TEXT_WHITE,FONT_NORMAL,1.0f,(const char*)&creditStats);
    } else RenderFormattedText(300,10,TEXT_WHITE,FONT_NORMAL,1.0f,creditPages[Sys_Global.creditsPageIndex]);
}

void RenderMenu(void);
void RenderPausedUI(void);
extern float shadBiasMin;
static inline __attribute__((always_inline)) double RenderUI(void) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    drawCallsNormal = drawCallsRenderedThisFrame;
    if (Sys_Global.creditsActive) { RenderCredits(); return get_time(); }
    if (Sys_Global.menuActive) RenderMenu();
    else if (Sys_Global.gamePaused) RenderPausedUI();
    if ((Sys_Global.menuActive || Sys_Global.gamePaused) && Sys_Input.keyStates[GLFW_KEY_DOWN].pressed) currentMenuItem = (currentMenuItem + 1) >= menuItemCount ? 0 : (currentMenuItem + 1);
    if ((Sys_Global.menuActive || Sys_Global.gamePaused) && Sys_Input.keyStates[GLFW_KEY_UP].pressed) currentMenuItem = (currentMenuItem - 1) < 0 ? (menuItemCount - 1) : (currentMenuItem - 1);
    
    // Diagnostics / Debugging
    int16_t debugTextStartY = 58;
    if (Sys_Cheats.showLocation && !Sys_Global.menuActive) RenderFormattedText(16, debugTextStartY, TEXT_WHITE, FONT_NORMAL,1.0f, "x: %.4f, y: %.4f, z: %.4f, rx: %.4f, ry: %.4f, rz: %.4f, rw: %.4f",Sys_Global.instances[PLAYER1].position.x,Sys_Global.instances[PLAYER1].position.y,Sys_Global.instances[PLAYER1].position.z,Sys_Global.instances[PLAYER1].rotation.x,Sys_Global.instances[PLAYER1].rotation.y,Sys_Global.instances[PLAYER1].rotation.z,Sys_Global.instances[PLAYER1].rotation.w);
    int16_t lineSpacing = 18;
    if (!Sys_Global.menuActive && !Sys_Cheats.noHUD) RenderFormattedText(16,debugTextStartY + (lineSpacing * 1),TEXT_WHITE,FONT_NORMAL,1.0f,"timeSinceLastPhysicsTick: %.6f, numShadowsCouldRender: %u, playerCellIdx: %u, numCellsVisible: %u",Sys_Global.timeSinceLastPhysicsTick, voxen_Shadow_System.numShadowsCouldRender,playerCellIdx,numCellsVisible);
    if (!Sys_Global.menuActive && !Sys_Cheats.noHUD) RenderFormattedText(16,debugTextStartY + (lineSpacing * 2),TEXT_WHITE,FONT_NORMAL,1.0f,"Player velocity: %.2f, %.2f, %.2f",Sys_Global.instances[PLAYER1].velocity.x,Sys_Global.instances[PLAYER1].velocity.y,Sys_Global.instances[PLAYER1].velocity.z);
    if (!Sys_Global.menuActive && !Sys_Cheats.noHUD) RenderFormattedText(16,debugTextStartY + (lineSpacing * 3),TEXT_WHITE,FONT_NORMAL,1.0f,"Test Entity[%u] %s Index: %u, Shadow cpu ms: %.3f",editModeSelection,Sys_Global.entities[Sys_Global.instances[editModeSelection].index].path,editModeTestEntityDefinition,voxen_Shadow_System.shadowTime * 1000);
    if (!Sys_Global.menuActive && !Sys_Cheats.noHUD) RenderFormattedText(16,debugTextStartY + (lineSpacing * 4),TEXT_WHITE,FONT_NORMAL,1.0f,"Player cell: %u",Sys_Global.instances[PLAYER1].cellIndex);
    RenderFormattedText(16,debugTextStartY + (lineSpacing * 5),TEXT_WHITE,FONT_NORMAL,1.0f,"Cursor: %d, %d   dx: %d dy: %d",Sys_Global.cursorPosition_x,Sys_Global.cursorPosition_y,Sys_Input.currentMouse_dx,Sys_Input.currentMouse_dy);
    if (Sys_Cheats.consoleActive) RenderFormattedText(16, 0, TEXT_WHITE, FONT_NORMAL,1.0f, "] %s",consoleEntryText);
    if (Sys_Global.statusTextDecayFinished > Sys_Global.current_time) RenderFormattedText(479,114,TEXT_WHITE,FONT_NORMAL,1.0f, "%s",statusText);
    if (!Sys_Global.menuActive && !Sys_Global.gamePaused) {
        if (!Sys_Global.gamePaused && !Sys_Cheats.noHUD) RenderUIImage(672,0,22,22,1020); // Shoot mode button
        bool mouseReleased = Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_LEFT].pressed;
        if (Sys_Global.inventoryMode) {
            if (CursorIsOverBounds(672,694,22,0)) {
                if (mouseReleased) {
                    Sys_Global.inventoryMode = false;
                    Sys_Global.cursorPosition_x = Sys_Settings.ScreenWidth / 2;
                    Sys_Global.cursorPosition_y = Sys_Settings.ScreenHeight / 2;
                }
            }
        }
    }
    
    double time_now = get_time();
    if (Sys_Cheats.showFPS) {
        Sys_Global.thisFrameTime = (time_now - Sys_Global.last_time) * 1000.0;
        Sys_Global.cpuFrameTime = Sys_Global.cpuTime * 1000.0;
        uint8_t timingColor = TEXT_WHITE;
        if (vabs(Sys_Global.thisFrameTime - Sys_Global.cpuFrameTime) < 0.451) timingColor = TEXT_GREEN;
        if (Sys_Global.thisFrameTime > 6.944444) timingColor = TEXT_RED;
        drawCallsRenderedThisFrame += 2; textDrawCallsRenderedThisFrame += 2; // Add two more for this text render ;)
        RenderFormattedText(16, debugTextStartY - lineSpacing, timingColor, FONT_NORMAL,1.0f, "ms: %.2f, CPU %.2f", Sys_Global.thisFrameTime,Sys_Global.cpuFrameTime);
        RenderFormattedText(16 + 230.0f, debugTextStartY - lineSpacing, TEXT_WHITE, FONT_NORMAL,1.0f, "(FPS: %d, Worst: %d), Drwclls: %d [G %d UI %d Txt %d Shd %d] Vrts: %d E:%u|M:%u|P:%u|T:%.5f",Sys_Global.framesPerLastSecond,Sys_Global.worstFPS,drawCallsRenderedThisFrame,drawCallsNormal,uiImageDrawCallsRenderedThisFrame,textDrawCallsRenderedThisFrame,shadowDrawCallsRenderedThisFrame,verticesRenderedThisFrame,Sys_Cheats.editMode,Sys_Global.menuActive,Sys_Global.gamePaused,Sys_Global.pauseRelativeTime);
    }
    
    return time_now;
}

#define SHADOW_NEARMESH_MAX 768 // 350 was too low for light 712 on security atrium
DepthSort shadows_nearMeshes[SHADOW_NEARMESH_MAX]; // Found that this is typically around 172
float shadows_nearMeshRadii[SHADOW_NEARMESH_MAX];

typedef struct {
    uint16_t index; // Original index in lights array
    float distanceSquared; // Distance to camera squared
    float score; // Priority score (lower distance, higher intensity = higher priority)
    float radius;
    Vector3 position;
} LightCandidate;

static inline __attribute__((always_inline)) bool EntNotVisible(uint16_t i, bool otherCondition) { Entity* e = &Sys_Global.instances[i]; return e->texIndex > loadedTexturesMaxIndex || !(e->entflags & ENTFLAG_ACTIVE) || e->index >= MAX_ENTITIES || e->modelIndex >= MODEL_IDX_MAX || e->texIndex >= MAX_VALID_TEXTURE || otherCondition; }

static inline __attribute__((always_inline,hot)) uint16_t GetAndBindModel(uint16_t i, uint16_t currentModelType) {
    glUniform1ui(0,i);
    uint16_t modelType = (instanceIsLODArray[i] || Sys_Settings.ModelDetail < 1u) && Sys_Global.instances[i].lodIndex < loadedModelsMaxIndex ? Sys_Global.instances[i].lodIndex : Sys_Global.instances[i].modelIndex;
    if (currentModelType == modelType && currentModelType != 0) return currentModelType;
    
    glBindVertexBuffer(0,Sys_Render.vbos[modelType],0,VERTEX_ATTRIBUTES_COUNT * sizeof(float));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,Sys_Render.tbos[modelType]);
    return modelType;
}

static inline __attribute__((always_inline,hot)) void RenderShadowmaps(void) {    
    double shadowStartTime = get_time();
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, 4.0f);
    LightCandidate candidates[MAX_SHADOWMAPS];
    uint16_t numberFoundLightCandidatesForShadows = 0;
    float bestScores[MAX_SHADOWMAPS];
    voxen_Shadow_System.numShadowsCouldRender = 0;
    Vector3 playerPos = Sys_Global.instances[PLAYER1].position;
    float pfx = Sys_Global.instances[PLAYER1].forward.x;    float pfy = Sys_Global.instances[PLAYER1].forward.y;    float pfz = Sys_Global.instances[PLAYER1].forward.z;
    for (uint16_t i = 0; i < loadedLights; ++i) { // Collect candidates: only lights that are enabled, within FAR_PLANE, and in PVS
        if ((!lightCastsShadows[i])) continue;

        uint32_t litIdx = i * LIGHT_DATA_SIZE;
        Vector3 lightPos = (Vector3){ lights[litIdx], lights[litIdx + LIGHT_DATA_OFFSET_POSY], lights[litIdx + LIGHT_DATA_OFFSET_POSZ] };
        float intensity = lightMaxIntensity[i];
        if ((intensity < 0.1f)) continue;
        
        float range =  lights[litIdx + LIGHT_DATA_OFFSET_RANGE] * 0.99f; // Discard 1% more lights/meshes for performance.
        float luminosity = (intensity / (range * range));
        if (luminosity < 0.008f && (range < 8.0f || intensity < 0.5f) && i != headmountedLanternLight) continue;
        if (!lightInPVS[i] && i != headmountedLanternLight) continue;
        
        float dx = lightPos.x - playerPos.x; float dy = lightPos.y - playerPos.y; float dz = lightPos.z - playerPos.z;
        float distSqrdToPlayer = dx*dx + dy*dy + dz*dz;
        float dotResult = (dx*pfx + dy*pfy + dz*pfz);
        if (dotResult < 0.0f && distSqrdToPlayer > (range * range)) continue;
        
        float score = distSqrdToPlayer / vmax(intensity, 0.01f);
        if (numberFoundLightCandidatesForShadows < MAX_SHADOWMAPS) {
            candidates[numberFoundLightCandidatesForShadows] = (LightCandidate){ i, distSqrdToPlayer, score, range, lightPos };
            bestScores[numberFoundLightCandidatesForShadows] = score;
            numberFoundLightCandidatesForShadows++;
        } else {
            float currentWorst = bestScores[0];
            for (uint32_t j = 1; j < numberFoundLightCandidatesForShadows; j++) currentWorst = vmax(currentWorst, bestScores[j]);
            if (score < currentWorst) {  // Only compare against current worst
                int worstIdx = 0; // Find worst (highest score) and replace it
                for (uint32_t j = 1; j < numberFoundLightCandidatesForShadows; ++j) {
                    if (bestScores[j] > bestScores[worstIdx]) worstIdx = j;
                }
                candidates[worstIdx] = (LightCandidate){ i, distSqrdToPlayer, score, range, lightPos };
                bestScores[worstIdx] = score;
            }
        }

        voxen_Shadow_System.numShadowsCouldRender++;
    }

    uint32_t numLightsShadowmapsToRender = vmin(voxen_Shadow_System.numShadowsCouldRender, MAX_SHADOWMAPS);
    if (numLightsShadowmapsToRender > 0) { // Added since there is now work between here and the for loop so this is beneficial to check.
        glUseProgram(Sys_Render.shadowmapsClearShaderProgram); // Clear shadowmaps.  One might think that this would be less performant than standard shadowmap FBO with gl clears and textures but in fact this is faster on all but the oldest hardware (e.g. 10yrs old is fine, 13yrs suffers a small hit).
        glUniform1ui(1, SHADOW_MAP_SIZE);
        for (uint32_t c=0;c<numLightsShadowmapsToRender;++c) {
            glUniform1ui(0, c);
            GLuint groupX_shadClear = ((SHADOW_MAP_SIZE * SHADOW_MAP_SIZE) + 31) / 32;
            glDispatchCompute(groupX_shadClear,6,1);
        }

        shadowDrawCallsRenderedThisFrame = 0;
        __builtin_memset(voxen_Shadow_System.shadowmapIndirectionList, MAX_SHADOWMAPS + 1, loadedLights * sizeof(uint32_t)); // Set to invalid values for all
        glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
        glUseProgram(Sys_Render.shadowmapsShaderProgram);
        uint32_t shadowmapOffsetHead = 0U;
        uint16_t shadowCasterIndices[SHADOW_NEARMESH_MAX * MAX_SHADOWMAPS];
        uint32_t numShadowCasters = 0;
        for (int i=START_INDEX_LEVEL_INSTANCES;i<INSTANCE_COUNT;++i) {
            if (EntNotVisible(i,(Sys_Global.instances[i].entflags & ENTFLAG_NO_SHADOWS))) continue;
            if (ConstIndexIsNPC(Sys_Global.instances[i].index)) continue;

            shadowCasterIndices[numShadowCasters] = i;
            numShadowCasters++;
            if (numShadowCasters >= (SHADOW_NEARMESH_MAX * MAX_SHADOWMAPS)) break; // Ran out of shadowcasters max for frame.
        }
        
        uint16_t numShadowingLightsHandled = 0, currentModelType = 0, currentTexIndex = 0;
        bool currentIsTransparent = 0;
        for (uint32_t c = 0; c < numLightsShadowmapsToRender; ++c) { // Render top MAX_SHADOWMAPS candidates
            uint16_t lightIdx = candidates[c].index;
            float effectiveRadius = vmin(candidates[c].radius, 15.36f);
            Vector3 lightPos = candidates[c].position;
            uint16_t nearbyMeshCount = 0;
            for (uint16_t shadowCasterInstanceIdx = 0; shadowCasterInstanceIdx < numShadowCasters; shadowCasterInstanceIdx++) {
                uint16_t j = shadowCasterIndices[shadowCasterInstanceIdx];
                shadows_nearMeshRadii[nearbyMeshCount] = modelBounds[(Sys_Global.instances[j].modelIndex * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_RADIUS] * 0.99f;
                Vector3 d = Vector3_A_minus_B(Sys_Global.instances[j].position, lightPos);
                float distToLightSqrd = dot_vector3(d, d);
                float radSum = (effectiveRadius + shadows_nearMeshRadii[nearbyMeshCount]);
                if (distToLightSqrd >= radSum * radSum) continue;
                
                shadows_nearMeshes[nearbyMeshCount].index = j;
                shadows_nearMeshes[nearbyMeshCount].depth = distToLightSqrd; 
                nearbyMeshCount++;
                if (nearbyMeshCount >= SHADOW_NEARMESH_MAX) { DualLogWarn("Shadowmapping needs larger nearMeshes count than %u!  Skipping some renderables for light %u!\n", SHADOW_NEARMESH_MAX, lightIdx); break; }
            }

            if (nearbyMeshCount < 1) continue;

            glUniform3f(3, lightPos.x, lightPos.y, lightPos.z);
            voxen_Shadow_System.shadowmapIndirectionList[lightIdx] = numShadowingLightsHandled;
            #pragma GCC unroll 6
            for (uint8_t face = 0; face < 6; face++) {                                            
                glUniform1ui(2, face);
                glUniformMatrix4fv(1,1,GL_FALSE,(float*)lightViewProj[lightIdx][face]);
                glUniform1ui(7, shadowmapOffsetHead + (face * SHADOW_MAP_SIZE * SHADOW_MAP_SIZE));
                shadowDrawCallsRenderedThisFrame++;
                for (uint16_t j = 0; j < nearbyMeshCount; ++j) {
                    int i = shadows_nearMeshes[j].index;            
                    if (!SphereInFrustum(lightFrustumPlanes[lightIdx][face], Sys_Global.instances[i].position, shadows_nearMeshRadii[j] * 1.41f)) continue;

                    currentModelType = GetAndBindModel(i,currentModelType);
                    if (currentTexIndex != Sys_Global.instances[i].texIndex) { currentTexIndex = Sys_Global.instances[i].texIndex; glUniform1ui(6, Sys_Global.instances[i].texIndex); }
                    if (currentIsTransparent != transparentTexture[Sys_Global.instances[i].texIndex]) { currentIsTransparent = transparentTexture[Sys_Global.instances[i].texIndex]; glUniform1ui(8, transparentTexture[Sys_Global.instances[i].texIndex] ? 1u : 0u); }
                    glDrawElements(GL_TRIANGLES,modelTriangleCounts[currentModelType]*3,GL_UNSIGNED_INT,0); drawCallsRenderedThisFrame++; verticesRenderedThisFrame += modelTriangleCounts[currentModelType] * 3;
                }
            }
            
            shadowmapOffsetHead += (SHADOW_MAP_SIZE * SHADOW_MAP_SIZE) * 6;
            numShadowingLightsHandled++;
        }

        glViewport(0, 0, Sys_Settings.ScreenWidth, Sys_Settings.ScreenHeight);
        glNamedBufferData(Sys_Render.shadowMapsIndirectionID, loadedLights * sizeof(uint32_t), voxen_Shadow_System.shadowmapIndirectionList, GL_DYNAMIC_DRAW);
    }
    
    Sys_Render.shadowmapsNeedUpdated = false;
    glDisable(GL_POLYGON_OFFSET_FILL);
    voxen_Shadow_System.shadowTime = get_time() - shadowStartTime;
}

DepthSort visibleInstances[INSTANCE_COUNT];
static inline __attribute__((always_inline)) bool DetermineIfInstanceVisible(uint16_t i, bool otherCondition, bool skyVisible, Vector3 playerPos, float* distSqrd) {
    if (EntNotVisible(i,otherCondition)) return false; // must be transparent && transparents or neither
        
    Vector3 objPos = Sys_Global.instances[i].position;
    uint16_t instCellIdx = PosGetCellCoords(objPos.x, objPos.z);
    Vector3 delta = Vector3_A_minus_B(objPos, playerPos);
    *distSqrd = delta.x*delta.x + delta.y*delta.y + delta.z*delta.z;
    float radius = modelBounds[(Sys_Global.instances[i].modelIndex * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_RADIUS] * 2.0f;
    if (!SphereInFrustum(playerFrustumPlanes,objPos,radius) && (Sys_Global.instances[i].index != 754 || !skyVisible) && i != editModeSelection) return false;
    
    if (EntityIndexIsPortalBlockingDoor(Sys_Global.instances[i].index)) { // Extra checks only needed for opaque portal blocking doors.
        bool inPVS = (gridCellStates[instCellIdx] & CELL_VISIBLE);
        if (!inPVS) {
            uint16_t cellX = (uint16_t)clamp((int32_t)vfloor((objPos.x - worldMin_x + CELLXHALF) / WORLDCELL_WIDTH_F), 0, WORLDX_0BASED);
            uint16_t cellZ = (uint16_t)clamp((int32_t)vfloor((objPos.z - worldMin_z + CELLXHALF) / WORLDCELL_WIDTH_F), 0, WORLDX_0BASED);
            for (int ix = cellX - 2; ix <= (int)cellX + 2 && !inPVS; ++ix) {
                for (int iz = cellZ - 2; iz <= (int)cellZ + 2; ++iz) {
                    if (!XZPairInBounds(ix, iz)) return false;

                    int subIdx = iz * WORLDX + ix;
                    if (get_cull_bit(precomputedVisibleCellsFromHere, instCellIdx * ARRSIZE + subIdx) && (gridCellStates[subIdx] & CELL_VISIBLE)) { inPVS = true; break; }
                }
            }
        }
        if (!inPVS) return false;
    } else {
        if (!(Sys_Global.currentLevel == 1 && (Sys_Global.instances[i].index == 309 ||  Sys_Global.instances[i].index == 532))) { // Hack for beaker and beaker holder on level 1 shelf getting culled from door portals.
            if (((gridCellStates[instCellIdx] & (CELL_VISIBLE | CELL_OPEN)) == CELL_OPEN) && (Sys_Global.instances[i].index != 754 || !skyVisible)) return false; // For some shelves that are inset away from cells, need to still draw their items by checking && CELL_OPEN here, unfortunately this means they don't ever get culled :(
        }
        
        if (!(gridCellStates[instCellIdx] & CELL_OPEN) && *distSqrd >= 943.7184f && (Sys_Global.instances[i].index != 754 || !skyVisible)) return false; // 30.72 * 30.72, 12 cells
    }
    
    return true;
}

void qsort(void* base, size_t nmemb, size_t size, int (*cmp)(const void*, const void*));
static inline __attribute__((always_inline)) void RenderInstances(Vector3 playerPos, bool transparents) {
    uint16_t visibleCount = 0, currentTexIndex = 0, currentNormIndex = 0, currentGlowIndex = 0, currentSpecIndex = 0, currentModelType = 0;
    bool skyVisible = (gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX);
    for (uint16_t i = START_INDEX_LEVEL_INSTANCES; i < INSTANCE_COUNT; ++i) {
        float distSqrd = FAR_PLANE * FAR_PLANE;
        if (!DetermineIfInstanceVisible(i,(transparentTexture[Sys_Global.instances[i].texIndex] ^ transparents),skyVisible,playerPos,&distSqrd)) continue;

        visibleInstances[visibleCount].index = i;
        visibleInstances[visibleCount].depth = distSqrd;
        visibleCount++;
    }
    
    if (visibleCount > 1) qsort(visibleInstances, visibleCount, sizeof(DepthSort), transparents ? compareDepthSort : compareDepthSortInverted); // Sort by depth (ascending for front-to-back)
    for (uint16_t visibleIndex = 0; visibleIndex < visibleCount; ++visibleIndex) {
        uint16_t i = visibleInstances[visibleIndex].index;
        if (transparentTexture[Sys_Global.instances[i].texIndex] && transparents) { glEnable(GL_CULL_FACE); glEnable(GL_BLEND); } // Transparents (with sort)
        else if (doubleSidedTexture[Sys_Global.instances[i].texIndex] || Sys_Global.instances[i].scale.x < 0.0f || Sys_Global.instances[i].scale.y < 0.0f || Sys_Global.instances[i].scale.z < 0.0f) { glDisable(GL_CULL_FACE); glEnable(GL_BLEND); } // Doublesided
        else { glEnable(GL_CULL_FACE); glDisable(GL_BLEND); } // Opaque

        glUniform1ui(17, Sys_Global.instances[i].texIndex == 316 ? 1u : 0u);
        glUniform1ui(25,(uint32_t)Sys_Global.instances[i].index); // constIndex
        if (currentNormIndex != (uint32_t)Sys_Global.instances[i].normIndex || Sys_Global.instances[i].normIndex == 0) { currentNormIndex = (uint32_t)Sys_Global.instances[i].normIndex; glUniform1ui(1, currentNormIndex); }
        if (currentTexIndex  != (uint32_t)Sys_Global.instances[i].texIndex  || Sys_Global.instances[i].texIndex == 0)  { currentTexIndex  =  (uint32_t)Sys_Global.instances[i].texIndex; glUniform1ui(18, currentTexIndex); }
        if (currentGlowIndex != (uint32_t)Sys_Global.instances[i].glowIndex || Sys_Global.instances[i].glowIndex == 0) { currentGlowIndex = (uint32_t)Sys_Global.instances[i].glowIndex; glUniform1ui(19, currentGlowIndex); }
        if (currentSpecIndex != (uint32_t)Sys_Global.instances[i].specIndex || Sys_Global.instances[i].specIndex == 0) { currentSpecIndex = (uint32_t)Sys_Global.instances[i].specIndex; glUniform1ui(20, currentSpecIndex); }
        currentModelType = GetAndBindModel(i,currentModelType);
        uint32_t vertCount = modelTriangleCounts[currentModelType] * 3;
        glDrawElements(GL_TRIANGLES,vertCount,GL_UNSIGNED_INT,0); drawCallsRenderedThisFrame++; verticesRenderedThisFrame += vertCount;
    }
}

static inline __attribute__((always_inline)) void RenderInstancesDepthOnly(Vector3 playerPos) {
    uint16_t visibleCount = 0, currentModelType = 0;
    bool skyVisible = (gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX);
    for (uint16_t i = START_INDEX_LEVEL_INSTANCES; i < INSTANCE_COUNT; ++i) {
        float distSqrd = FAR_PLANE * FAR_PLANE;
        if (!DetermineIfInstanceVisible(i,transparentTexture[Sys_Global.instances[i].texIndex],skyVisible,playerPos,&distSqrd)) continue;
        
        visibleInstances[visibleCount].index = i;
        visibleInstances[visibleCount].depth = distSqrd;
        visibleCount++;
        if (visibleCount >= INSTANCE_COUNT) break; // Feels unnecessary given the loop bounds?
    }
    
    if (visibleCount > 1) qsort(visibleInstances, visibleCount, sizeof(DepthSort), compareDepthSortInverted); // Sort by depth (ascending for front-to-back)
    for (uint16_t visibleIndex = 0; visibleIndex < visibleCount; ++visibleIndex) {
        uint16_t i = visibleInstances[visibleIndex].index;
        if (doubleSidedTexture[Sys_Global.instances[i].texIndex] || Sys_Global.instances[i].scale.x < 0.0f || Sys_Global.instances[i].scale.y < 0.0f || Sys_Global.instances[i].scale.z < 0.0f) { glDisable(GL_CULL_FACE); glEnable(GL_BLEND); } // Doublesided
        else { glEnable(GL_CULL_FACE); glDisable(GL_BLEND); } // Opaque

        currentModelType = GetAndBindModel(i,currentModelType);
        uint32_t vertCount = modelTriangleCounts[currentModelType] * 3;
        glDrawElements(GL_TRIANGLES, vertCount, GL_UNSIGNED_INT, 0); drawCallsRenderedThisFrame++; verticesRenderedThisFrame += vertCount;
    }
}

float GetPainStatic(void) { return 0.0f; } // TODO: Hook into pain/health management and shield impact effect
Color GetPainStaticColor(void) { return (Color){1.0f,0.0f,0.0f,1.0f}; } // TODO: Hook staticColor up to red or blue for pain or shield impact.

static inline __attribute__((always_inline)) __attribute__((hot)) void Render(void) {
    drawCallsRenderedThisFrame = textDrawCallsRenderedThisFrame = uiImageDrawCallsRenderedThisFrame = shadowDrawCallsRenderedThisFrame = verticesRenderedThisFrame = 0; // Reset per frame
    
    // Frame prep, View Matrix, and Projection Matrix
    float view[16]; // Also known as view matrix
    Vector3 playerPos = Sys_Global.instances[PLAYER1].position;
    float px = playerPos.x, py = playerPos.y, pz = playerPos.z;
    {// mat4_lookat_from(view,&Sys_Global.instances[PLAYER1].rotation, playerPos); Manually inlined for performance
        float x = Sys_Global.instances[PLAYER1].rotation.x, y = Sys_Global.instances[PLAYER1].rotation.y, z = Sys_Global.instances[PLAYER1].rotation.z, w = Sys_Global.instances[PLAYER1].rotation.w;
        float x2 = x * x, y2 = y * y, z2 = z * z;
        float xy = x * y, xz = x * z, yz = y * z;
        float wx = w * x, wy = w * y, wz = w * z;
        Vector3 right   = { 1.0f - 2.0f * (y2 + z2),        2.0f * (xy + wz),        2.0f * (xz - wy) };  // X+ (right)
        Vector3 up      = {        2.0f * (xy - wz), 1.0f - 2.0f * (x2 + z2),        2.0f * (yz + wx) };  // Y+ (up)
        Vector3 forward = {        2.0f * (xz + wy),        2.0f * (yz - wx), 1.0f - 2.0f * (x2 + y2) };  // Z+ (forward)
        view[0]  = right.x; view[1]  = up.x; view[2]  = -forward.x; view[3]  = 0.0f;
        view[4]  = right.y; view[5]  = up.y; view[6]  = -forward.y; view[7]  = 0.0f;
        view[8]  = right.z; view[9]  = up.z; view[10] = -forward.z; view[11] = 0.0f;
        view[12] = -dot_vector3(right, playerPos); view[13] = -dot_vector3(up, playerPos); view[14] = dot_vector3(forward, playerPos); view[15] = 1.0f;
    }
    
    float viewProj[16]; // view-projection matrix
    mul_mat4(viewProj,rasterPerspectiveProjection,view);
    float invViewRot[9] = {view[0],view[4],view[8], view[1],view[5],view[9], view[2],view[6],view[10]};
    ExtractFrustumPlanes(viewProj,playerFrustumPlanes);
    glBindVertexArray(Sys_Render.vao_chunk); // Common vao for RenderShadowmaps and Rasterized Geometry
    glEnable(GL_DEPTH_TEST);
    if (likely(Sys_Settings.Shadows > 0u)) RenderShadowmaps();
    __builtin_memset(    lightDirty,0    ,LIGHT_COUNT * sizeof(bool)); // Clear dirty after shadowmaps for minimal shadowmap updating.
    __builtin_memset(dirtyInstances,0,Sys_Global.loadedInstances * sizeof(bool)); // Clear dirty after shadowmaps for minimal shadowmap updating.
    glBindFramebuffer(GL_FRAMEBUFFER, Sys_Render.gBufferFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the corner where last shadowmap wrote into
    glEnable(GL_CULL_FACE); glDisable(GL_BLEND); // Opaques
    
    // Depth Prepass - Eliminates some overdraw for ~6.1% performance improvement in spite of added draw calls since these are relatively cheap and avoid the heavy fragment work in main pass.
    glUseProgram(Sys_Render.depthPrepassShaderProgram);
    glUniformMatrix4fv(2,1,GL_FALSE,viewProj);
    glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    RenderInstancesDepthOnly(playerPos); // opaques only
    
    // Main Pass
    glUseProgram(Sys_Render.chunkShaderProgram);
    glUniformMatrix4fv(2,1,GL_FALSE,viewProj);
    glUniform1ui(25,0u); // default constIndex
    uint32_t grayscaleOn = (uint32_t)(/*(Sys_Global.inventoryPlayer1.hasHardware & HW_INF) && */(Sys_Global.inventoryPlayer1.hardwareIsActive & HW_INF));
    glUniform1ui(26,grayscaleOn);
    glUniform1ui(3,0u); // isUI false
    float fogActual = fogBaseDensityForLevel + (float)(Sys_Global.fogFac / 255u);
    glUniform3f(4,fogColorR * fogActual,fogColorG * fogActual,fogColorB * fogActual); // Fog Color(which is density)
    glUniform1ui(14,Sys_Settings.Reflections);   glUniform1ui(15,Sys_Settings.Shadows);
    glUniform1f(8,worldMin_x);   glUniform1f(9,worldMin_z);    glUniform3f(10,playerPos.x,playerPos.y,playerPos.z);
    glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);
    RenderInstances(playerPos,false);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    RenderInstances(playerPos,true); // opaque, then transparents
    glUniform1ui(25,0u); // reset constIndex

    // Draw Debug Lines
    if (unlikely(Sys_Global.debugLineVertCount > 1)) DrawDebugLines(viewProj);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if (likely(Sys_Settings.Reflections > 0u)) { // Screen Space Reflections
        glUseProgram(Sys_Render.ssrShaderProgram);
        glUniformMatrix4fv(4,1,GL_FALSE,viewProj);
        glUniform3f(3,playerPos.x,playerPos.y,playerPos.z);
        GLuint groupX_ssr = ((Sys_Settings.ScreenWidth  / Sys_Settings.SSR_RES) + 31) / 32;
        GLuint groupY_ssr = ((Sys_Settings.ScreenHeight / Sys_Settings.SSR_RES) + 31) / 32;
        glDispatchCompute(groupX_ssr,groupY_ssr,1);
    }

    glUseProgram(Sys_Render.imageBlitShaderProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,Sys_Render.inputImageID);
    glUniform1i(4,4); // outputImage texture sampler2D
    double berserkTimeRemainingNormalized = berserkFinished > 0.0001 ? (berserkFinished - Sys_Global.pauseRelativeTime) / BERSERK_TIME : 0.0;
    if (berserkFinished < Sys_Global.pauseRelativeTime && berserkFinished > 0.0001) berserkFinished = berserkTimeRemainingNormalized = 0.0;
    glUniform1ui(5,Sys_Settings.Reflections);
    glUniform1ui(6,Sys_Settings.AntiAliasing);
    glUniform1f(14,Sys_Settings.FOV);
    glUniform1f(16,(float)Sys_Settings.ScreenWidth / (float)Sys_Settings.ScreenHeight);
    glUniform1ui(22,Sys_Settings.Shadows);
    glUniform1f(9,(float)berserkTimeRemainingNormalized);
    glUniform1f(10,berserkSeedTime);
    glUniform1ui(11,Sys_Settings.Brightness);
    glUniform3f(12,deg2rad(cam_yaw), deg2rad(cam_pitch), deg2rad(cam_roll));
    glUniform3f(13,px, py, pz);
    glUniform1f(15,(float)Sys_Global.pauseRelativeTime * 0.1f);
    glUniform1ui(17,(gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX) || Sys_Global.currentLevel == LEVEL_CYBERSPACE);
    glUniform1ui(18,(gridCellStates[playerCellIdx] & CELL_SEES_SUN) && Sys_Global.currentLevel != LEVEL_CYBERSPACE);
    glUniform1ui(19,((Sys_Global.currentLevel >= 10 && Sys_Global.currentLevel < LEVEL_CYBERSPACE) ? 1u : 0u) && (gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX));
    uint32_t shieldOnType = 0u; // No shield green tint.
    if (Sys_Global.instances[WORLD].ioflags & QUESTBIT_SHIELD_ACTIVATED) {
        if (Sys_Global.currentLevel == 6 || Sys_Global.currentLevel == 7) shieldOnType = 2u; // Shielding only below player for lower levels.
        else if (Sys_Global.currentLevel <= 5) shieldOnType = 1u; // Shielding everywhere as levels fully within shield.
    }
    
    glUniform1ui(20, shieldOnType);
    Color painStaticColor = GetPainStaticColor();
    glUniform3f(23, painStaticColor.r, painStaticColor.g, painStaticColor.b);
    glUniformMatrix4fv(24, 1, GL_FALSE, viewProj);
    glUniformMatrix3fv(25, 1, GL_FALSE, invViewRot);
    glUniform1i(27, 0); // Texture 0 for the rendered geometry color buffer
    glUniform1f(28, GetPainStatic());
    glUniform1ui(29,grayscaleOn); // Grayscale
    glBindVertexArray(Sys_Render.quadVAO);
    glDisable(GL_DEPTH_TEST); // Reenabled later after all UI just up there before RenderShadowmaps call
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    drawCallsRenderedThisFrame++; verticesRenderedThisFrame += 4;

    // UI
    Sys_Global.last_time = RenderUI();

    // Cursor [ /// VERY LAST DRAWN OVER EVERYTHING ELSE! /// ]
    bool menuOrInventoryCursorStyle = (Sys_Global.gamePaused || Sys_Global.menuActive);
    uint16_t cursorTexture = menuOrInventoryCursorStyle ? 1261 : 1260;
    if ((Sys_Global.inventoryMode && !Sys_Cheats.noHUD) || Sys_Global.menuActive || Sys_Global.gamePaused) RenderUIImage((int16_t)(Sys_Global.cursorPosition_x) - 20, (int16_t)(Sys_Global.cursorPosition_y) - 20, 40,40, cursorTexture);
    else RenderUIImage(683-20,371, 40,40, cursorTexture);
    
    if ((Sys_Global.last_time - Sys_Global.lastFrameSecCountTime) >= 1.00) { // Update Diagnostic Poll
        Sys_Global.lastFrameSecCountTime = Sys_Global.last_time;
        Sys_Global.framesPerLastSecond = Sys_Global.globalFrameNum - Sys_Global.lastFrameSecCount;
        if (Sys_Global.framesPerLastSecond < Sys_Global.worstFPS && Sys_Global.globalFrameNum > 2000) Sys_Global.worstFPS = Sys_Global.framesPerLastSecond; // After startup, keep track of worst framerate seen.
        Sys_Global.lastFrameSecCount = Sys_Global.globalFrameNum;
    }
    
    Sys_Global.cpuTime = get_time() - Sys_Global.current_time; // Measure time over everything this frame before GPU swap buffers
    glfwSwapBuffers(window); // Present frame
    CHECK_GL_ERROR();
}

bool UpdatedPlayerCell(void);
void UpdateAnims(void);
bool CullCore(void);
int32_t main(void) {
    double game_start_time = get_time();
    random_range_rng = (uint32_t)game_start_time; // Seed global rand uniquely with time since system boot.
    console_log_file = OS_OpenWriteonly("./voxen.log"); // Initialize log system for all prints to go to both stdout and voxen.log file
    DebugRAM("program start");
    #ifdef WINDOWS
        SetDllDirectory("External\\Windows");
    #endif
    InitializeEnvironment();
    DebugRAM("prior to game loop");
    DualLog("Game Initialized in %f secs\n",get_time() - game_start_time);
    Sys_Global.absoluteTime = Sys_Global.last_topframe_time = Sys_Global.current_time = get_time();
    Sys_Global.pauseRelativeTime = Sys_Global.last_physics_time = 0.0;
    while(1) { // Main Loop
        if (glfwWindowShouldClose(window)) OS_Exit(0);
        if (queuedLevelToLoad != 255u) { LoadLevel(queuedLevelToLoad); queuedLevelToLoad = 255u; continue; }

        Sys_Global.current_time = get_time(); // Update Time
        Sys_Global.deltaTime = Sys_Global.current_time - Sys_Global.last_topframe_time;
        Sys_Global.absoluteTime += Sys_Global.deltaTime;
        Sys_Global.last_topframe_time = Sys_Global.current_time;
        if (!Sys_Global.gamePaused) Sys_Global.pauseRelativeTime += Sys_Global.deltaTime;
    
        // Update Events, calls Physics()
        mouseMovementThisFrame = false;
        glfwPollEvents();
        Input_PollJoysticks();
        Input_PollGamepad();
        if (Sys_Input.keyStates[GLFW_KEY_E].pressed) play_wav("./Audio/cyborgs/yourlevelsareterrible.wav",0.1f,(Vector3){},false);
        if (Sys_Input.window_has_focus) {
            if (Sys_Input.keyStates[GLFW_KEY_CAPS_LOCK].pressed) Sys_Input.isCapsLockOn = !Sys_Input.isCapsLockOn; // Change capslock state to match keyboard having toggled.  Must always happen regardless of paused/menu.
            ProcessInput(); // Calls ApplyPlayerMovements(), needs called without checking paused state for menus handling.
        }
        
        Sys_Global.timeSinceLastPhysicsTick = Sys_Global.pauseRelativeTime - Sys_Global.last_physics_time;
        if (likely(!Sys_Global.gamePaused || Sys_Global.menuActive)) UpdateAnims(); // Changes collision positions
        if (likely(!Sys_Global.gamePaused && !Sys_Global.menuActive)) { // Update Gameplay
            if (Sys_Global.timeSinceLastPhysicsTick > (1.0 / 144.0)) { Sys_Global.last_physics_time = Sys_Global.pauseRelativeTime; Physics(); }
            ModUpdate();
            UpdatePlayerFacingAngles();
            UpdateAmbientSounds();
            UpdateMusic();
        }

        if (likely(!Sys_Global.gamePaused || Sys_Global.menuActive)) {
            Sys_Render.shadowmapsNeedUpdated = UpdatedPlayerCell();
            Sys_Render.shadowmapsNeedUpdated = UpdateLights(&Sys_Render.shadowmapsNeedUpdated);
            CullCore();
            bool uploadInstances = false;
            for (uint32_t i = START_INDEX_LEVEL_INSTANCES; i < Sys_Global.loadedInstances; i++) {
                if (dirtyInstances[i]) {
                    if (Sys_Global.instances[i].modelIndex >= loadedModelsMaxIndex || modelVertexCounts[Sys_Global.instances[i].modelIndex] < 1) { dirtyInstances[i] = false; continue; } // No model or empty model

                    uploadInstances = true;    Sys_Render.shadowmapsNeedUpdated = true;
                    float x = Sys_Global.instances[i].rotation.x, y = Sys_Global.instances[i].rotation.y, z = Sys_Global.instances[i].rotation.z, w = Sys_Global.instances[i].rotation.w;
                    float x2 = x * x,   y2 = y * y,   z2 = z * z,   xy = x * y,   xz = x * z,   yz = y * z,   wx = w * x,   wy = w * y,   wz = w * z;
                    float sclx = Sys_Global.instances[i].scale.x; float scly = Sys_Global.instances[i].scale.y; float sclz = Sys_Global.instances[i].scale.z;
                    modelMatrices[(i * 16) + 0]  = (1.0f - 2.0f * (y2 + z2)) * sclx; // Right X, Necessary -x for blender right to left handed coordinate conversion.
                    modelMatrices[(i * 16) + 1]  =        (2.0f * (xy + wz)) * sclx; // Right Y
                    modelMatrices[(i * 16) + 2]  =        (2.0f * (xz - wy)) * sclx; // Right Z
                    modelMatrices[(i * 16) + 3]  = modelMatrices[(i * 16) + 7] = modelMatrices[(i * 16) + 11] = 0.0f;
                    modelMatrices[(i * 16) + 4]  =        (2.0f * (xy - wz)) * scly; // Up X
                    modelMatrices[(i * 16) + 5]  = (1.0f - 2.0f * (x2 + z2)) * scly; // Up Y
                    modelMatrices[(i * 16) + 6]  =        (2.0f * (yz + wx)) * scly; // Up Z
                    modelMatrices[(i * 16) + 8]  =        (2.0f * (xz + wy)) * sclz; // Forward X
                    modelMatrices[(i * 16) + 9]  =        (2.0f * (yz - wx)) * sclz; // Forward Y
                    modelMatrices[(i * 16) + 10] = (1.0f - 2.0f * (x2 + y2)) * sclz; // Forward Z
                    modelMatrices[(i * 16) + 12] = Sys_Global.instances[i].position.x;   modelMatrices[(i * 16) + 13] = Sys_Global.instances[i].position.y;   modelMatrices[(i * 16) + 14] = Sys_Global.instances[i].position.z;
                    modelMatrices[(i * 16) + 15] = 1.0f;
                }
            }
            if (uploadInstances) glNamedBufferData(Sys_Render.matricesBufferID, Sys_Global.loadedInstances * 16 * sizeof(float), modelMatrices, GL_DYNAMIC_DRAW);
        }
        
        Render();
        Sys_Global.globalFrameNum++;
        InputClearRisingAndFallingEdges();
        Sys_Input.currentMouse_dx = Sys_Input.currentMouse_dy = 0;
        #ifdef DEBUG_RAM_OUTPUT
            static const uint32_t dbgFrames[] = {4,100,200,500,1000};
            static const char*    dbgLabels[] = {"after 4 frames","after 100 frames","after 200 frames","after 500 frames","after 1000 frames"};
            for (int _d=0;_d<5;_d++) if (Sys_Global.globalFrameNum == dbgFrames[_d]) { DebugRAM(dbgLabels[_d]); break; }
        #endif
    }
    return 0;
}
