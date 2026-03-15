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
#include "Shaders/shaders.h"
#include "credits.h"
GlobalContext Sys_Global = { .menuActive = true, .screenshotTimeout = 1.0, .creditsPageIndex = 1, .difficultyCombat = 2, .difficultyCyber = 2, .difficultyPuzzle = 2, .difficultyMission = 2, .deaths = 0, .worstFPS = UINT32_MAX };
CheatsSystem Sys_Cheats = { .god = false, .noclip = true, .showLocation = true, .showFPS = true, .editMode = true };
RenderSystem Sys_Render;
SystemUI Sys_UIPlayer1;
SystemUI Sys_UIPlayer2;
OsFileHandle console_log_file = 0;
AutoSplitterData autoSplitter = { 0x1337133713371337, 0, false, 0 }; // Fore use with LiveSplit or other future speedrunner utilities for doing speedruns
uint8_t queuedLevelToLoad = 255u;
Entity entities[MAX_ENTITIES]; // Global array of entity definitions
uint16_t entityCount; // Number of entities loaded
float modelMatrices[INSTANCE_COUNT * 16];
uint8_t dirtyInstances[INSTANCE_COUNT];
double berserkFinished;
float berserkSeedTime, aspect3D = 1.0f, cam_pitch, cam_yaw = 90.0f, cam_roll, fogColorR, fogColorG, fogColorB, fogBaseDensityForLevel;
float rasterPerspectiveProjection[16];
float shadowmapsPerspectiveProjection[16];
int32_t cursorPosition_x = 680, cursorPosition_y = 384; // Separate internal cursor from system cursor.  This gets relatively pushed around by real cursor movement to give consistent platform behavior.
char uiTextBuffer[TEXT_BUFFER_SIZE];
float uiOrthoProjection[16];
float lights[LIGHT_COUNT * LIGHT_DATA_SIZE];
bool lightDirty[LIGHT_COUNT];
static float lightView[LIGHT_COUNT][6][4][4]; // Array of Array of 6 Arrays of 16 floats (matrix 4x4).  lightView[i][face][0 ... 15]
static float lightViewProj[LIGHT_COUNT][6][16]; // Array of Array of 6 Arrays of 16 floats (matrix 4x4).  lightViewProj[i][face][0 ... 15]
FrustumPlane lightFrustumPlanes[LIGHT_COUNT][6][6]; // Array of Array of 6 Arrays of FrustumPlane structs (four floats).  lightFrustumPlanes[i][face][.nx,.ny,, .nz, .d]
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
uint32_t totalPixels;
uint32_t totalPaletteColors;
uint16_t loadedTexturesMaxIndex;
bool doubleSidedTexture[MAX_VALID_TEXTURE];
bool transparentTexture[MAX_VALID_TEXTURE];
uint32_t drawCallsRenderedThisFrame;
uint32_t textDrawCallsRenderedThisFrame;
uint32_t uiImageDrawCallsRenderedThisFrame;
uint32_t shadowDrawCallsRenderedThisFrame;
uint32_t verticesRenderedThisFrame;
uint32_t drawCallsNormal;

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
        OsFileHandle out = 1;
        if (prefix) OS_RawWrite(out,prefix,GetStringLength(prefix));
        OS_RawWrite(out, buf, GetStringLength(buf));
    #else
        // Linux/macOS/Android - write to stdout (fd 1) or stderr (fd 2)
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

void DualLog(const char* fmt, ...) { va_list args; __builtin_va_start(args, fmt); DualLogMain(NULL, fmt, args); __builtin_va_end(args); }
void DualLogWarn(const char* fmt, ...) { va_list args; __builtin_va_start(args, fmt); DualLogMain("\033[1;38;5;208mWARN:", fmt, args); __builtin_va_end(args); }
void DualLogError(const char* fmt, ...) { va_list args; __builtin_va_start(args, fmt); DualLogMain("\033[1;31mERROR:", fmt, args); __builtin_va_end(args); }

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

__attribute__((pure)) bool SphereInFrustum(FrustumPlane* planes, Vector3 c, float radius) {
    for (int i=0;i<6;++i) { if ((planes[i].normal.x * c.x + planes[i].normal.y * c.y + planes[i].normal.z * c.z + planes[i].d) < -radius) return false; }
    return true;
}

void ExtractFrustumPlanes(float* m, FrustumPlane* planes) {
    planes[0].normal.x = m[3]  + m[0];  planes[0].normal.y = m[7]  + m[4];  planes[0].normal.z = m[11] + m[8];  planes[0].d = m[15] + m[12]; // Left
    planes[1].normal.x = m[3]  - m[0];  planes[1].normal.y = m[7]  - m[4];  planes[1].normal.z = m[11] - m[8];  planes[1].d = m[15] - m[12]; // Right
    planes[2].normal.x = m[3]  + m[1];  planes[2].normal.y = m[7]  + m[5];  planes[2].normal.z = m[11] + m[9];  planes[2].d = m[15] + m[13]; // Bottom
    planes[3].normal.x = m[3]  - m[1];  planes[3].normal.y = m[7]  - m[5];  planes[3].normal.z = m[11] - m[9];  planes[3].d = m[15] - m[13]; // Top
    planes[4].normal.x = m[3]  + m[2];  planes[4].normal.y = m[7]  + m[6];  planes[4].normal.z = m[11] + m[10]; planes[4].d = m[15] + m[14]; // Near
    planes[5].normal.x = m[3]  - m[2];  planes[5].normal.y = m[7]  - m[6];  planes[5].normal.z = m[11] - m[10]; planes[5].d = m[15] - m[14]; // Far
    for (int i = 0; i < 6; i++) {
        float len = vsqrtf(planes[i].normal.x*planes[i].normal.x + planes[i].normal.y*planes[i].normal.y + planes[i].normal.z*planes[i].normal.z);
        if (len > 0.0f) {
            planes[i].normal.x /= len; planes[i].normal.y /= len; planes[i].normal.z /= len; planes[i].d /= len; // Normalize
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

bool lightInPVS[LIGHT_COUNT];
Vector3 lightsNewPosition[LIGHT_COUNT];
bool UpdateLights(bool* voxelsNeedUpdated) {
    // Update headmounted lantern
    int32_t lant = headmountedLanternLight * LIGHT_DATA_SIZE;
    if (/*(Sys_Global.inventoryPlayer1.hasHardware & HW_LAN) && */(Sys_Global.inventoryPlayer1.hardwareIsActive & HW_LAN)) {
        Vector3 lanternPosLast = lanternPos;
        lanternPos = Sys_Global.instances[PLAYER1].position;
        lanternPos.y -= 0.24f;
        lanternPos.x += 0.04f;
        lanternPos.z += 0.04f;
        lightsNewPosition[headmountedLanternLight] = lanternPos;
        lights[lant + LIGHT_DATA_OFFSET_POSX] = lanternPos.x;
        lights[lant + LIGHT_DATA_OFFSET_POSY] = lanternPos.y;
        lights[lant + LIGHT_DATA_OFFSET_POSZ] = lanternPos.z;
        lights[lant + LIGHT_DATA_OFFSET_INTENSITY] = lightMaxIntensity[headmountedLanternLight] = lanternVersionBrightness[Sys_Global.inventoryPlayer1.hardwareVersionSetting[7]];
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

            float differenceInIntensity = (lightMaxIntensity[i] - lightMinIntensity[i]);
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
                        lightLerpValue[i] = lightMinIntensity[i] + (differenceInIntensity * lerpVal);
                        lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY] = lightLerpValue[i];
                    }
                }
            }
        }
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, Sys_Render.lightsID); glBufferData(GL_SHADER_STORAGE_BUFFER, loadedLights * LIGHT_DATA_SIZE * sizeof(float), lights, GL_DYNAMIC_DRAW);
    if (*voxelsNeedUpdated) {
        float px = Sys_Global.instances[PLAYER1].position.x; float py = Sys_Global.instances[PLAYER1].position.y; float pz = Sys_Global.instances[PLAYER1].position.z;
        float fx = Sys_Global.instances[PLAYER1].forward.x;  float fy = Sys_Global.instances[PLAYER1].forward.y;  float fz = Sys_Global.instances[PLAYER1].forward.z;
        glUseProgram(Sys_Render.voxelUpdateShaderProgram);
        glUniform3f(5, px, py, pz);
        glUniform3f(6, fx, fy, fz);
        glUniform1ui(7, (uint32_t)MAX_LIGHTS_PER_VOXEL);
        GLuint groupX_voxels = (512 + 31) / 32;
        GLuint groupZ_voxels = (512 + 31) / 32; // Actually just a local size y, but for z axis voxels
        glDispatchCompute(groupX_voxels,groupZ_voxels, 1);
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
    return (cursorPosition_x >= startX && cursorPosition_x <= endX /* 0 == left */ && cursorPosition_y >= endY && cursorPosition_y <= startY); /* 0 == top */
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
    float paddingUV = (12.0f / (float)FONT_ATLAS_SIZE); // This is for the black outline around all text for readability.
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
        CopyMemoryFromBtoAForNBytes(textVertexData + vertexCount * 30, textVertices, sizeof(textVertices));
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
    va_list args;
    __builtin_va_start(args, fmt);
    StringFormat(statusText,TEXT_BUFFER_SIZE,fmt,args);
    __builtin_va_end(args);
    DualLog("%s\n",statusText);
    Sys_Global.statusTextDecayFinished = get_time() + 2.5; // 2.5 second decay time before text dissappears.
}

__attribute__((cold)) void NewGame(void) { // Reset World States
    RenderLoadingProgress(100,"Loading new game...");
    Sys_Global.instances[WORLD].ioflags = 0u;
    Sys_Global.instances[WORLD].lev1SecCode = random_range_u8(0u,9u); // Must do rand's repeatedly to prevent
    Sys_Global.instances[WORLD].lev2SecCode = random_range_u8(0u,9u); // these all being the same number.
    Sys_Global.instances[WORLD].lev3SecCode = random_range_u8(0u,9u);
    Sys_Global.instances[WORLD].lev4SecCode = random_range_u8(0u,9u);
    Sys_Global.instances[WORLD].lev5SecCode = random_range_u8(0u,9u);
    Sys_Global.instances[WORLD].lev6SecCode = random_range_u8(0u,9u);
    SetMemoryToValueForNBytes(Sys_Global.instances,0,INSTANCE_COUNT * sizeof(Entity)); // Initialize instances, the global entity array for the currently loaded level.
    PlayerInit(PLAYER1); PlayerInit(PLAYER2);
    cam_yaw = 90.0f; cam_pitch = 0.0f; cam_roll = 0.0f;
    Sys_Global.inventoryMode = Sys_Settings.NoShootMode;
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

    if (StringsAreEqual(Sys_Global.global_modname, "Citadel")) Sys_Global.global_modIsCitadel = true;
    DualLog(" %s:: num levels: %d, start level: %d... took %f secs\n",Sys_Global.global_modname, Sys_Global.numLevels, Sys_Global.startLevel, get_time() - start_time);
}

__attribute__((cold)) void LoadEntities(void) {
    double start_time = get_time();
    entityCount = 0;
    DataParser entity_parser;
    if (!parse_data_file(&entity_parser, MAX_ENTITIES, "./Data/entities.txt")) { DualLogError("Could not parse ./Data/entities.txt!\n"); OS_Exit(1); }
    
    entityCount = (uint16_t)entity_parser.count;
    DualLog("Loading  %d entities...", entityCount);
    if (entityCount > MAX_ENTITIES) { DualLogError("Too many entities in parser count %d, greater than %d!\n", entityCount, MAX_ENTITIES); OS_Exit(1); }
    if (entityCount == 0) { DualLogError("No entities found in entities.txt\n"); OS_Exit(1); }

    SetMemoryToValueForNBytes(entities,0,MAX_ENTITIES * sizeof(Entity));
//     #pragma omp parallel for
    for (int32_t i = 0; i < entityCount; i++) {
        if (entity_parser.entries[i].index == UINT16_MAX) continue;

        entities[i] = entity_parser.entries[i];
        flag_set(&entities[i].entflags, ENTFLAG_ACTIVE, true);
        flag_set(&entities[i].entflags, ENTFLAG_GROUNDED, false);
        flag_set(&entities[i].entflags, ENTFLAG_RIGIDBODY, ConstIndexIsDynamicObject(entities[i].index));
        if (entity_parser.entries[i].entflags & ENTFLAG_CARDCHUNK) {
            entities[i].lodIndex = GEOMETRY_LOD_CARD_MODEL_IDX; // Generic LOD card
            entities[i].collider = COLLIDER_TYPE_BOX;
            entities[i].colliderCenter = (Vector3){ .x = 0.0f, .y = 1.44f, .z = 0.0f };
            entities[i].colliderSize = (Vector3){ .x = 2.56f, .y = 0.32f, .z = 2.56f };
        }
        
        if (ConstIndexIsButtonSwitch(entities[i].index)) {
            entities[i].lockedMessageLingdex = 193; // ButtonSwitch
            entities[i].tickTime = 1.5;
        }
    }

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
    
    aspect3D = (float)Sys_Settings.ScreenWidth / (float)Sys_Settings.ScreenHeight;
    float f = vcot((float)Sys_Settings.FOV * PI / 360.0f);
    m = rasterPerspectiveProjection;
    m[0] = f / aspect3D; m[1] = 0.0f; m[2] =                                                      0.0f; m[3] =  0.0f;
    m[4] =         0.0f; m[5] =    f; m[6] =                                                      0.0f; m[7] =  0.0f;
    m[8] =         0.0f; m[9] = 0.0f; m[10]=      -(FAR_PLANE + NEAR_PLANE) / (FAR_PLANE - NEAR_PLANE); m[11]= -1.0f;
    m[12]=         0.0f; m[13]= 0.0f; m[14]= -2.0f * FAR_PLANE * NEAR_PLANE / (FAR_PLANE - NEAR_PLANE); m[15]=  0.0f;
    voxen_Shadow_System.shadDotThresh = 1.0f / vsqrtf(1.0f + vtan((float)Sys_Settings.FOV * PI / 360.0f) * (1.0f + aspect3D * aspect3D));
}

void UpdateScreenSize(GLFWwindow* unused, int32_t width, int32_t height) {
    (void)unused; // Appease glfwSetFramebufferSizeCallback pointer type
    Sys_Settings.ScreenWidth = vmax(vmin((uint16_t)width, 7680), 320u); Sys_Settings.ScreenHeight = vmax(vmin((uint16_t)height, 4320), 200u); // Cap at minimum Quake 1 resolution and maximum 8k.
    Sys_Settings.ScreenCenterX = (float)Sys_Settings.ScreenWidth * 0.5f; Sys_Settings.ScreenCenterY = (float)Sys_Settings.ScreenHeight * 0.5f;
    DualLog("Screen size updated to %u x %u from input values %d x %d\n", Sys_Settings.ScreenWidth, Sys_Settings.ScreenHeight, width, height);
    glViewport(0, 0, Sys_Settings.ScreenWidth, Sys_Settings.ScreenHeight);
    UpdateProjectionMatrices();
    glUseProgram(Sys_Render.imageBlitShaderProgram);
    glUniform1ui(2, Sys_Settings.ScreenWidth);
    glUniform1ui(3, Sys_Settings.ScreenHeight);
    glUniform1i(26, Sys_Settings.SSR_RES);
    glUseProgram(Sys_Render.chunkShaderProgram);
    glUniform1ui(6, Sys_Settings.ScreenWidth);
    glUniform1ui(7, Sys_Settings.ScreenHeight);
    glUseProgram(Sys_Render.ssrShaderProgram);
    glUniform1ui(0, Sys_Settings.ScreenWidth / Sys_Settings.SSR_RES);
    glUniform1ui(1, Sys_Settings.ScreenHeight / Sys_Settings.SSR_RES);       
    glUniform1i(2, Sys_Settings.SSR_RES);
    GenerateAndBindTexture(&Sys_Render.inputImageID,             GL_RGBA8, Sys_Settings.ScreenWidth, Sys_Settings.ScreenHeight,            GL_RGBA, GL_UNSIGNED_BYTE, GL_TEXTURE_2D); // Lit Raster
    GenerateAndBindTexture(&Sys_Render.inputWorldPosID,        GL_RGBA32F, Sys_Settings.ScreenWidth, Sys_Settings.ScreenHeight,            GL_RGBA,         GL_FLOAT, GL_TEXTURE_2D); // Raster World Positions
    GenerateAndBindTexture(&Sys_Render.inputDepthID, GL_DEPTH_COMPONENT32, Sys_Settings.ScreenWidth, Sys_Settings.ScreenHeight, GL_DEPTH_COMPONENT,         GL_FLOAT, GL_TEXTURE_2D); // Raster Depth
    GenerateAndBindTexture(&Sys_Render.inputSpecID,              GL_RGBA8, Sys_Settings.ScreenWidth, Sys_Settings.ScreenHeight,            GL_RGBA, GL_UNSIGNED_BYTE, GL_TEXTURE_2D); // Specular Colors
    GenerateAndBindTexture(&Sys_Render.inputNormalID,            GL_RG16F, Sys_Settings.ScreenWidth, Sys_Settings.ScreenHeight,              GL_RG,         GL_FLOAT, GL_TEXTURE_2D); // Normal XYZ
    glGenTextures(1, &Sys_Render.outputImageID);
    glBindTexture(GL_TEXTURE_2D, Sys_Render.outputImageID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,  Sys_Settings.ScreenWidth / Sys_Settings.SSR_RES,  Sys_Settings.ScreenHeight / Sys_Settings.SSR_RES, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, Sys_Render.gBufferFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Sys_Render.inputImageID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, Sys_Render.inputWorldPosID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, Sys_Render.inputSpecID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, Sys_Render.inputNormalID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, Sys_Render.inputDepthID, 0);
    glBindImageTexture(0, Sys_Render.inputImageID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8); // Main Rendered Color
    glBindImageTexture(1, Sys_Render.inputWorldPosID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F); // World Position XYZ
    glBindImageTexture(2, Sys_Render.inputSpecID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8); // Specular
    glBindImageTexture(4, Sys_Render.outputImageID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8); // SSR result
    glBindImageTexture(5, Sys_Render.inputNormalID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RG16F); // Normal XYZ
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, Sys_Render.outputImageID);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// GLFW Callbacks
void SetVSync(void) { glfwSwapInterval((int32_t)Sys_Settings.Vsync); }
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
    
    if (event == GLFW_CONNECTED) {
        SetMemoryToValueForNBytes(&Sys_Input.joystickPresent[jid], 1, sizeof(bool));
    } else if (event == GLFW_DISCONNECTED) {
        SetMemoryToValueForNBytes(&Sys_Input.joystickPresent[jid], 0, sizeof(bool));
        SetMemoryToValueForNBytes(Sys_Input.joystickButtons, 0, sizeof(Sys_Input.joystickButtons));
        SetMemoryToValueForNBytes(Sys_Input.joystickHats, 0, sizeof(Sys_Input.joystickHats));
    }
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
    if (action == GLFW_PRESS) {
        Sys_Input.mouseButtons[button].down = true;
        Sys_Input.mouseButtons[button].pressed = true;
    } else if (action == GLFW_RELEASE) {
        Sys_Input.mouseButtons[button].down = false;
        Sys_Input.mouseButtons[button].released = true;
    }
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

void ChangeResolution(void) {
//                 int monitorCount;
//                 GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
//                 GLFWmonitor* monitor = GetCurrentMonitor();
//                 int modeCount;
//                 const GLFWvidmode* modes = glfwGetVideoModes(monitor, &modeCount);
//                 int currentIdx = -1;
//                 int curw = 0, curh = 0;
//                 for (int i = 0; i < modeCount; i++) {
//                     if (curw == modes[i].width && curh == modes[i].height) continue;
//                     
//                     curw = modes[i].width; curh = modes[i].height;
//                     DualLog("glfw reported resolution: %u x %u\n", modes[i].width, modes[i].height);
//                     if (modes[i].width  == Sys_Settings.ScreenWidth && modes[i].height == Sys_Settings.ScreenHeight) {
//                         currentIdx = i;
//                         break;
//                     }
//                 }
// 
//                 int nextIdx = (currentIdx + 1);
//                 if (nextIdx >= modeCount) nextIdx = 0;
//                 Sys_Settings.ScreenWidth  = modes[nextIdx].width;
//                 Sys_Settings.ScreenHeight = modes[nextIdx].height;
//                 UpdateScreenSize(NULL, Sys_Settings.ScreenWidth, Sys_Settings.ScreenHeight);
//                 Sys_Input.ignore_next_mouse_delta = true;
}

void ChangeFullScreenWindowed(void) {
    int monitorCount;
    GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
    GLFWmonitor* next = monitors[currentMonitorIndex];
    if (Sys_Settings.Fullscreen) {
        int xpos, ypos, width, height;
        glfwGetMonitorWorkarea(next, &xpos, &ypos, &width, &height);
        Sys_Settings.ScreenWidth = width; Sys_Settings.ScreenHeight = height;
        glfwSetWindowSize(window, Sys_Settings.ScreenWidth, Sys_Settings.ScreenHeight);
        glfwSetWindowPos(window, xpos, ypos-18);
    } else {
        int monitorCount;
        GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
        GLFWmonitor* next = monitors[currentMonitorIndex];
        int mx, my;
        glfwGetMonitorPos(next, &mx, &my);
        const GLFWvidmode* mode = glfwGetVideoMode(next);
        int xpos = mx + (mode->width - Sys_Settings.ScreenWidth) / 2;
        int ypos = my + (mode->height - Sys_Settings.ScreenHeight) / 2;
        glfwSetWindowPos(window, xpos, ypos);
    }
    
    UpdateScreenSize(NULL, Sys_Settings.ScreenWidth, Sys_Settings.ScreenHeight);
    Sys_Input.ignore_next_mouse_delta = true;
}

void SetSkyRotateSpeed(void) {
    static const float speeds[] = { 0.05f, 1.0f, 2.5f, 3.75f, 6.25f };
    float skyRotateSpeed = speeds[Sys_Cheats.dizzyLevel];
    glUseProgram(Sys_Render.imageBlitShaderProgram);
    glUniform1f(30, skyRotateSpeed);
}


void SetVSync(void);

void SetGI(void) {
    if (Sys_Settings.GI) {
        // TODO: Set needed Voxel GI uniforms
    }
}

void SetSpeakerMode(void) {
    switch (Sys_Settings.SpeakerMode) {
        case 0: break;//targetMode = AudioSpeakerMode.Mono; break; // TODO
        case 1: break;//targetMode = AudioSpeakerMode.Stereo; break;
        case 2: break;//targetMode = AudioSpeakerMode.Quad; break;
        case 3: break;//targetMode = AudioSpeakerMode.Surround; break;
        case 4: break;//targetMode = AudioSpeakerMode.Mode5point1; break;
        case 5: break;//targetMode = AudioSpeakerMode.Mode7point1; break;
        case 6: break;//targetMode = AudioSpeakerMode.Prologic; break;
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
    #define PLATFORM_DLOPEN(path)    LoadLibraryA(path)
    #define PLATFORM_DLSYM(handle, name)  GetProcAddress((handle), (name))
    #define PLATFORM_DLCLOSE(handle) FreeLibrary((handle))
#else
    #include <dlfcn.h>
    #define PLATFORM_DLOPEN(path)    dlopen((path), RTLD_NOW)
    #define PLATFORM_DLSYM(handle, name)  dlsym((handle), (name))
    #define PLATFORM_DLCLOSE(handle) dlclose((handle))
#endif

void* mod_handle = NULL;
bool GetKey(int settingIndex);
bool GetKeyPressed(int settingIndex);
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
        const char* err = dlerror();
        if (err && *err) {
            DualLogError("dlopen of %s failed: %s",mod_path,err);
        } else {
            DualLogError("dlopen of %s failed: no detailed error from dlerror() — common with unresolved symbols or format issues",mod_path);
        }
        OS_Exit(1);
    }
    
    ModInit         = (void (*)(GlobalContext*,CheatsSystem*,SettingsSystem*)) PLATFORM_DLSYM(mod_handle,"ModInit"); if (!ModInit) { DualLogError("Failed to load ModInit function pointer from mod data\n"); OS_Exit(1); }
    ModInit(&Sys_Global,&Sys_Cheats,&Sys_Settings);
    Sys_Global.GetKey = GetKey;
    Sys_Global.GetKeyPressed = GetKeyPressed;
#define LINK_MOD_SYMBOL(name,ptr) PLATFORM_DLSYM(mod_handle,(name)); if (!(ptr)) { DualLogError("Failed to load %s function pointer from mod data\n", (name)); OS_Exit(1); }
    ModUpdate       = (void (*)(void))           LINK_MOD_SYMBOL("ModUpdate",ModUpdate);
    Forward         = (bool (*)(void))           LINK_MOD_SYMBOL("Forward",Forward);
    Backpedal       = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "Backpedal");      if (!Backpedal) { DualLogError("Failed to load Backpedal function pointer from mod data\n"); OS_Exit(1); }
    StrafeLeft      = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "StrafeLeft");     if (!StrafeLeft) { DualLogError("Failed to load StrafeLeft function pointer from mod data\n"); OS_Exit(1); }
    StrafeRight     = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "StrafeRight");    if (!StrafeRight) { DualLogError("Failed to load StrafeRight function pointer from mod data\n"); OS_Exit(1); }
    Jump            = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "Jump");           if (!Jump) { DualLogError("Failed to load Jump function pointer from mod data\n"); OS_Exit(1); }
    JumpDown        = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "JumpDown");       if (!JumpDown) { DualLogError("Failed to load JumpDown function pointer from mod data\n"); OS_Exit(1); }
    Crouch          = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "Crouch");         if (!Crouch) { DualLogError("Failed to load Crouch function pointer from mod data\n"); OS_Exit(1); }
    Prone           = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "Prone");          if (!Prone) { DualLogError("Failed to load Prone function pointer from mod data\n"); OS_Exit(1); }
    LeanLeft        = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "LeanLeft");       if (!LeanLeft) { DualLogError("Failed to load LeanLeft function pointer from mod data\n"); OS_Exit(1); }
    LeanRight       = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "LeanRight");      if (!LeanRight) { DualLogError("Failed to load LeanRight function pointer from mod data\n"); OS_Exit(1); }
    Sprint          = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "Sprint");         if (!Sprint) { DualLogError("Failed to load Sprint function pointer from mod data\n"); OS_Exit(1); }
    TurnLeft        = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "TurnLeft");       if (!TurnLeft) { DualLogError("Failed to load TurnLeft function pointer from mod data\n"); OS_Exit(1); }
    TurnRight       = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "TurnRight");      if (!TurnRight) { DualLogError("Failed to load TurnRight function pointer from mod data\n"); OS_Exit(1); }
    LookUp          = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "LookUp");         if (!LookUp) { DualLogError("Failed to load LookUp function pointer from mod data\n"); OS_Exit(1); }
    LookDown        = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "LookDown");       if (!LookDown) { DualLogError("Failed to load LookDown function pointer from mod data\n"); OS_Exit(1); }
    RecentLog       = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "RecentLog");      if (!RecentLog) { DualLogError("Failed to load RecentLog function pointer from mod data\n"); OS_Exit(1); }
    Biomonitor      = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "Biomonitor");     if (!Biomonitor) { DualLogError("Failed to load Biomonitor function pointer from mod data\n"); OS_Exit(1); }
    Sensaround      = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "Sensaround");     if (!StrafeLeft) { DualLogError("Failed to load StrafeLeft function pointer from mod data\n"); OS_Exit(1); }
    Lantern         = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "Lantern");        if (!Lantern) { DualLogError("Failed to load Lantern function pointer from mod data\n"); OS_Exit(1); }
    Shield          = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "Shield");         if (!Shield) { DualLogError("Failed to load Shield function pointer from mod data\n"); OS_Exit(1); }
    Infrared        = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "Infrared");       if (!Infrared) { DualLogError("Failed to load Infrared function pointer from mod data\n"); OS_Exit(1); }
    Email           = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "Email");          if (!Email) { DualLogError("Failed to load Email function pointer from mod data\n"); OS_Exit(1); }
    Booster         = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "Booster");        if (!Booster) { DualLogError("Failed to load Booster function pointer from mod data\n"); OS_Exit(1); }
    Jumpjets        = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "Jumpjets");       if (!Jumpjets) { DualLogError("Failed to load Jumpjets function pointer from mod data\n"); OS_Exit(1); }
    Attack          = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "Attack");         if (!Attack) { DualLogError("Failed to load Attack function pointer from mod data\n"); OS_Exit(1); }
    Use             = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "Use");            if (!Use) { DualLogError("Failed to load Use function pointer from mod data\n"); OS_Exit(1); }
    Menu            = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "Menu");           if (!Menu) { DualLogError("Failed to load Menu function pointer from mod data\n"); OS_Exit(1); }
    ToggleMode      = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "ToggleMode");     if (!ToggleMode) { DualLogError("Failed to load ToggleMode function pointer from mod data\n"); OS_Exit(1); }
    Reload          = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "Reload");         if (!Reload) { DualLogError("Failed to load Reload function pointer from mod data\n"); OS_Exit(1); }
    WeaponCycUp     = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "WeaponCycUp");    if (!WeaponCycUp) { DualLogError("Failed to load WeaponCycUp function pointer from mod data\n"); OS_Exit(1); }
    WeaponCycDown   = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "WeaponCycDown");  if (!WeaponCycDown) { DualLogError("Failed to load WeaponCycDown function pointer from mod data\n"); OS_Exit(1); }
    Grenade         = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "Grenade");        if (!Grenade) { DualLogError("Failed to load Grenade function pointer from mod data\n"); OS_Exit(1); }
    GrenadeCycUp    = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "GrenadeCycUp");   if (!GrenadeCycUp) { DualLogError("Failed to load GrenadeCycUp function pointer from mod data\n"); OS_Exit(1); }
    GrenadeCycDown  = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "GrenadeCycDown"); if (!GrenadeCycDown) { DualLogError("Failed to load GrenadeCycDown function pointer from mod data\n"); OS_Exit(1); }
    ChangeAmmoType  = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "ChangeAmmoType"); if (!ChangeAmmoType) { DualLogError("Failed to load ChangeAmmoType function pointer from mod data\n"); OS_Exit(1); }
    Patch           = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "Patch");          if (!Patch) { DualLogError("Failed to load Patch function pointer from mod data\n"); OS_Exit(1); }
    PatchCycUp      = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "PatchCycUp");     if (!PatchCycUp) { DualLogError("Failed to load PatchCycUp function pointer from mod data\n"); OS_Exit(1); }
    PatchCycDown    = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "PatchCycDown");   if (!PatchCycDown) { DualLogError("Failed to load PatchCycDown function pointer from mod data\n"); OS_Exit(1); }
    Map             = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "Map");            if (!Map) { DualLogError("Failed to load Map function pointer from mod data\n"); OS_Exit(1); }
    SwimUp          = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "SwimUp");         if (!SwimUp) { DualLogError("Failed to load SwimUp function pointer from mod data\n"); OS_Exit(1); }
    SwimDn          = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "SwimDn");         if (!SwimDn) { DualLogError("Failed to load SwimDn function pointer from mod data\n"); OS_Exit(1); }
    ChangeAmmoType    = (bool (*)(void))         PLATFORM_DLSYM(mod_handle, "ChangeAmmoType"); if (!ChangeAmmoType) { DualLogError("Failed to load ChangeAmmoType function pointer from mod data\n"); OS_Exit(1); }
    GetBasePlayerSpeed = (float (*)(bool))       PLATFORM_DLSYM(mod_handle, "GetBasePlayerSpeed");    if (!GetBasePlayerSpeed) { DualLogError("Failed to load GetBasePlayerSpeed function pointer from mod data\n"); OS_Exit(1); }
    InitializeAIAfterLoad = (void (*)(uint16_t)) PLATFORM_DLSYM(mod_handle, "InitializeAIAfterLoad"); if (!InitializeAIAfterLoad) { DualLogError("Failed to load InitializeAIAfterLoad function pointer from mod data\n"); OS_Exit(1); }
    TakeScreenshot  = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "TakeScreenshot"); if (!TakeScreenshot) { DualLogError("Failed to load TakeScreenshot function pointer from mod data\n"); OS_Exit(1); }
    Console         = (bool (*)(void))           PLATFORM_DLSYM(mod_handle, "Console");        if (!Console) { DualLogError("Failed to load Console function pointer from mod data\n"); OS_Exit(1); }
    UpdateMusic     = (void (*)(void))           LINK_MOD_SYMBOL("UpdateMusic",UpdateMusic);
    PlayMenuMusic   = (void (*)(void))           LINK_MOD_SYMBOL("PlayMenuMusic",PlayMenuMusic);
    PlayGameMusic   = (void (*)(void))           LINK_MOD_SYMBOL("PlayGameMusic",PlayGameMusic);
    ResetLevelMusic   = (void (*)(void))         LINK_MOD_SYMBOL("ResetLevelMusic",ResetLevelMusic);
    DualLog("done!\n");
}

void InitializeAudio(void);
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
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(window, key_callback);
    glfwSetJoystickCallback(joystick_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetWindowFocusCallback(window, window_focus_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glFrontFace(GL_CCW); // Set triangle sorting order (GL_CW vs GL_CCW)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Globally same alpha blending
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
    InitializeAudio(); // Audio
    LoadGameModDefinition();
    LoadModFunctions();
    LoadEntities();
    InitFontAtlasses();
    double nextInitTimeSection = get_time();
//     BioMonitorInit(); TODO
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
    CopyMemoryFromBtoAForNBytes(&modelMatrices[0],mat,16 * sizeof(float)); // Null instance matrix used for UI
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
    DebugRAM("InitializeEnvironment end");
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

static inline __attribute__((always_inline)) void AddDebugLine(Vector3 start, Vector3 end) {
    int32_t i = Sys_Global.debugLineVertCount;
    debugLineBuffer[i++] = start.x; debugLineBuffer[i++] = start.y; debugLineBuffer[i++] = start.z;
    debugLineBuffer[i++] =   end.x; debugLineBuffer[i++] =   end.y; debugLineBuffer[i++] =   end.z;
    Sys_Global.debugLineVertCount = i;
}

#define FROB_DISTANCE 4.9f
static inline __attribute__((always_inline)) void Frob(Vector3 pos, Vector3 forward, Vector3 right) {
    float offsetX = cursorPosition_x - (Sys_Settings.ScreenWidth * 0.5f);
    float offsetY = cursorPosition_y - (Sys_Settings.ScreenHeight * 0.5f);
    float ndcX = offsetX / (Sys_Settings.ScreenWidth * 0.5f);
    float ndcY = -offsetY / (Sys_Settings.ScreenHeight * 0.5f);  // flip Y
    float tanFov = vtan((float)Sys_Settings.FOV * 0.5f * PI / 180.0f);
    Vector3 view = (Vector3){ ndcX * tanFov * aspect3D, ndcY * tanFov, -1.0f };
    view = normalize_vector3(view);
    Vector3 flipForward = (Vector3){ -forward.x, -forward.y, -forward.z};
    Vector3 up = normalize_vector3( cross_vector3(right, flipForward) );
    Vector3 dir = (Vector3){ view.x * right.x + view.y * up.x + view.z * (flipForward.x),
                             view.x * right.y + view.y * up.y + view.z * (flipForward.y),
                             view.x * right.z + view.y * up.z + view.z * (flipForward.z) };
                             
    Sys_Global.debugLine_start = pos;
    Sys_Global.debugLine_end   = (Vector3){ dir.x * FROB_DISTANCE + pos.x, dir.y * FROB_DISTANCE + pos.y, dir.z * FROB_DISTANCE + pos.z };
    RaycastHit tempHit = Raycast(pos, dir, FROB_DISTANCE, LAYER_MASK_PLAYER_FROB);
    if (tempHit.hit) {
        Sys_Global.debugLine_end = tempHit.point;
        DualLog("Raycast hit!  Hit object %u named of entity type %s(%u) at hit point %f %f %f\n", tempHit.hitInstanceIndex, entities[Sys_Global.instances[tempHit.hitInstanceIndex].index].path, Sys_Global.instances[tempHit.hitInstanceIndex].index, (double)tempHit.point.x, (double)tempHit.point.y, (double)tempHit.point.z);
    }
    
    Sys_Global.debugLineFinished = Sys_Global.current_time + 3.0;
}

#define SHADOW_NEARMESH_MAX 512 // 350 was too low for light 712 on security atrium
DepthSort shadows_nearMeshes[SHADOW_NEARMESH_MAX]; // Found that this is typically around 172
float shadows_nearMeshRadii[SHADOW_NEARMESH_MAX];

typedef struct {
    uint16_t index; // Original index in lights array
    float distanceSquared; // Distance to camera squared
    float score; // Priority score (lower distance, higher intensity = higher priority)
    float radius;
    Vector3 position;
} LightCandidate;

bool EntNotVisible(uint16_t i, bool otherCondition) {
    if (!(Sys_Global.instances[i].entflags & ENTFLAG_ACTIVE)) return true;
    if (Sys_Global.instances[i].index >= MAX_ENTITIES || Sys_Global.instances[i].modelIndex >= MODEL_IDX_MAX || Sys_Global.instances[i].texIndex >= MAX_VALID_TEXTURE) return true;
    if (otherCondition) return true;
    return false;
}

static inline __attribute__((always_inline)) __attribute__((hot)) void RenderShadowmaps(void) {    
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
        } else if (score < bestScores[0]) {  // Only compare against current worst
            int worstIdx = 0; // Find worst (highest score) and replace it
            for (uint32_t j = 1; j < numberFoundLightCandidatesForShadows; ++j) {
                if (bestScores[j] > bestScores[worstIdx]) worstIdx = j;
            }
            candidates[worstIdx] = (LightCandidate){ i, distSqrdToPlayer, score, range, lightPos };
            bestScores[worstIdx] = score;
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
        SetMemoryToValueForNBytes(voxen_Shadow_System.shadowmapIndirectionList, MAX_SHADOWMAPS + 1, loadedLights * sizeof(uint32_t)); // Set to invalid values for all
        glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
        glUseProgram(Sys_Render.shadowmapsShaderProgram);
        uint32_t shadowmapOffsetHead = 0U;
        uint16_t shadowCasterIndices[SHADOW_NEARMESH_MAX * MAX_SHADOWMAPS];
        uint32_t numShadowCasters = 0;
        for (int i=START_INDEX_LEVEL_INSTANCES;i<INSTANCE_COUNT;++i) {
            if (EntNotVisible(i,(Sys_Global.instances[i].entflags & ENTFLAG_NO_SHADOWS))) continue;
//             if (ConstIndexIsNPC(Sys_Global.instances[i].index)) continue;

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
                glUniformMatrix4fv(1, 1, GL_FALSE, (float*)lightViewProj[lightIdx][face]);
                glUniform1ui(7, shadowmapOffsetHead + (face * SHADOW_MAP_SIZE * SHADOW_MAP_SIZE));
                shadowDrawCallsRenderedThisFrame++;
                for (uint16_t j = 0; j < nearbyMeshCount; ++j) {
                    int i = shadows_nearMeshes[j].index;            
                    if (!SphereInFrustum(lightFrustumPlanes[lightIdx][face], Sys_Global.instances[i].position, shadows_nearMeshRadii[j] * 1.41f)) continue;

                    int32_t modelType = (instanceIsLODArray[i] || Sys_Settings.ModelDetail < 1u) && Sys_Global.instances[i].lodIndex < loadedModelsMaxIndex ? Sys_Global.instances[i].lodIndex : Sys_Global.instances[i].modelIndex;
                    if (currentModelType != modelType) {
                        currentModelType = modelType;
                        glBindVertexBuffer(0, Sys_Render.vbos[currentModelType], 0, VERTEX_ATTRIBUTES_COUNT * sizeof(float));
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Sys_Render.tbos[currentModelType]);
                    }
                    
                    glUniform1ui(0, i);
                    if (currentTexIndex != Sys_Global.instances[i].texIndex) { currentTexIndex = Sys_Global.instances[i].texIndex; glUniform1ui(6, Sys_Global.instances[i].texIndex); }
                    if (currentIsTransparent != transparentTexture[Sys_Global.instances[i].texIndex]) { currentIsTransparent = transparentTexture[Sys_Global.instances[i].texIndex]; glUniform1ui(8, transparentTexture[Sys_Global.instances[i].texIndex] ? 1u : 0u); }
                    glDrawElements(GL_TRIANGLES, modelTriangleCounts[currentModelType] * 3, GL_UNSIGNED_INT, 0);
                    drawCallsRenderedThisFrame++; verticesRenderedThisFrame += modelTriangleCounts[currentModelType] * 3;
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

char creditStats[4096];
static inline __attribute__((always_inline)) float GetScore(float stupid, bool isFinal) {
    float score = 0.0f;
    float victories = (float)(Sys_Global.kills + Sys_Global.cyberkills);
    float secs = 0.0f;
    secs = vfloor((float)Sys_Global.pauseRelativeTime / 3600.0f);
    if (!isFinal) { // Report score if no deaths.
        score = victories * 10000.0f;
        score -= vmin(score * 0.666f,secs * 100.0f);
        score *= ((stupid + 1.0f) / 37.0f);
        if (stupid > 35.0f) score += 2222222.0f; // secret kevin bonus
        return vfloor(score);
    }

    // Death is 10 anti-kills, but you always keep at least a third of your kills.
    float deathPenalty = Sys_Global.ressurections * 10.0f;
    score = victories - vmin(deathPenalty,victories * 0.666f);
    score *= 10000.0f;
    score -= vmin(score * 0.666f,secs * 100.0f);
    score *= ((stupid + 1.0f) / 37.0f); // 9 * 4 + 1 is best difficulty factor
    if (stupid > 35.0f) score += 2222222.0f; // secret kevin bonus
    return vfloor(score);
}

static inline __attribute__((always_inline)) void CreditsStats(void) {
    size_t off = 0;
    off += StringFormat(creditStats + off, sizeof(creditStats), "================================================================================\nCITADEL\n");
    off += StringFormat(creditStats + off, sizeof(creditStats), "================================================================================\nCONGRATULATIONS %s\n", Sys_Global.playerName);
    uint32_t hours, minutes; double secs;
    double t = Sys_Global.pauseRelativeTime;
    double tb = (vfloor(t/3600.0));
    hours = (uint32_t)tb;
    t = t - (tb * 3600.0);
    tb = vfloor(t / 60.0);
    minutes = (uint32_t)tb;
    secs = t - (tb * 60.0);
    off += StringFormat(creditStats + off, sizeof(creditStats), "Straight Time: %uh %um %.3fs\n", hours, minutes, secs);
    t = Sys_Global.absoluteTime;
    tb = vfloor(t/3600.0);
    hours = (uint32_t)tb;
    t = t - (tb * 3600.0);
    tb = vfloor(t / 60.0);
    minutes = (uint32_t)tb;
    secs = t - (tb * 60.0);
    off += StringFormat(creditStats + off, sizeof(creditStats), "Total Time (with reload from deaths): %uh %um %.3fs\n", hours, minutes, secs);
    float stupid = 0.0f;
    stupid += (float)(Sys_Global.difficultyCombat * Sys_Global.difficultyCombat);
    stupid += (float)(Sys_Global.difficultyPuzzle * Sys_Global.difficultyPuzzle);
    stupid += (float)(Sys_Global.difficultyMission * Sys_Global.difficultyMission);
    stupid += (float)(Sys_Global.difficultyCyber * Sys_Global.difficultyCyber);
    uint32_t finalSubscore = GetScore(stupid, false);
    off += StringFormat(creditStats + off, sizeof(creditStats), "Kills: %u\nKills in Cyberspace: %u\nScoreSubtotal: %u\nDeaths: %u\nRessurections: %u\n", Sys_Global.kills, Sys_Global.cyberkills, (uint32_t)finalSubscore, Sys_Global.deaths, Sys_Global.ressurections);
    off += StringFormat(creditStats + off, sizeof(creditStats), "Combat: %u | Puzzle: %u | Mission: %u | Cyber: %u\n", Sys_Global.difficultyCombat, Sys_Global.difficultyPuzzle, Sys_Global.difficultyMission, Sys_Global.difficultyCyber);
    uint32_t finalScore = (uint32_t)GetScore(stupid, true);
    off += StringFormat(creditStats + off, sizeof(creditStats), "Difficulty Index: %.2f\nFinal Score: %u\n\n", (double)stupid, finalScore);
    off += StringFormat(creditStats + off, sizeof(creditStats), "Shots Fired: %u\nGrenades Thrown: %u\n", Sys_Global.shotsFired, Sys_Global.grenadesThrown);
    off += StringFormat(creditStats + off, sizeof(creditStats), "Damage Dealt: %f\nDamage Received: %f\nSaves Scummed: %u\n\nClick to continue...\n", (double)Sys_Global.damageDealt, (double)Sys_Global.damageReceived, Sys_Global.savesScummed);
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
    
    // Diagnostics / Debugging
    int16_t debugTextStartY = 58;
    if (Sys_Cheats.showLocation && !Sys_Global.menuActive) RenderFormattedText(16, debugTextStartY, TEXT_WHITE, FONT_NORMAL,1.0f, "x: %.4f, y: %.4f, z: %.4f, rx: %.4f, ry: %.4f, rz: %.4f, rw: %.4f",Sys_Global.instances[PLAYER1].position.x,Sys_Global.instances[PLAYER1].position.y,Sys_Global.instances[PLAYER1].position.z,Sys_Global.instances[PLAYER1].rotation.x,Sys_Global.instances[PLAYER1].rotation.y,Sys_Global.instances[PLAYER1].rotation.z,Sys_Global.instances[PLAYER1].rotation.w);
    int16_t lineSpacing = 18;
    if (!Sys_Global.menuActive && !Sys_Cheats.noHUD) RenderFormattedText(16,debugTextStartY + (lineSpacing * 1),TEXT_WHITE,FONT_NORMAL,1.0f,"timeSinceLastPhysicsTick: %.6f, numShadowsCouldRender: %u, playerCellIdx: %u, numCellsVisible: %u",Sys_Global.timeSinceLastPhysicsTick, voxen_Shadow_System.numShadowsCouldRender,playerCellIdx,numCellsVisible);
    if (!Sys_Global.menuActive && !Sys_Cheats.noHUD) RenderFormattedText(16,debugTextStartY + (lineSpacing * 2),TEXT_WHITE,FONT_NORMAL,1.0f,"Player velocity: %.2f, %.2f, %.2f",Sys_Global.instances[PLAYER1].velocity.x,Sys_Global.instances[PLAYER1].velocity.y,Sys_Global.instances[PLAYER1].velocity.z);
    if (!Sys_Global.menuActive && !Sys_Cheats.noHUD) RenderFormattedText(16,debugTextStartY + (lineSpacing * 3),TEXT_WHITE,FONT_NORMAL,1.0f,"Test Entity[%u] %s Index: %u, Shadow cpu ms: %.3f",editModeSelection,entities[Sys_Global.instances[editModeSelection].index].path,editModeTestEntityDefinition,voxen_Shadow_System.shadowTime * 1000);
    if (!Sys_Global.menuActive && !Sys_Cheats.noHUD) RenderFormattedText(16,debugTextStartY + (lineSpacing * 4),TEXT_WHITE,FONT_NORMAL,1.0f,"Player cell: %u, floor: %.3f, ceil: %.3f",Sys_Global.instances[PLAYER1].cellIndex,gridCellFloorHeight[Sys_Global.instances[PLAYER1].cellIndex],gridCellCeilingHeight[Sys_Global.instances[PLAYER1].cellIndex]);
    RenderFormattedText(16,debugTextStartY + (lineSpacing * 5),TEXT_WHITE,FONT_NORMAL,1.0f,"Cursor: %d, %d   dx: %d dy: %d",cursorPosition_x,cursorPosition_y,Sys_Input.currentMouse_dx,Sys_Input.currentMouse_dy);
    if (Sys_Cheats.consoleActive) RenderFormattedText(16, 0, TEXT_WHITE, FONT_NORMAL,1.0f, "] %s",consoleEntryText);
    if (Sys_Global.statusTextDecayFinished > Sys_Global.current_time) RenderFormattedText(479,114,TEXT_WHITE,FONT_NORMAL,1.0f, "%s",statusText);
    if (!Sys_Global.menuActive && !Sys_Global.gamePaused) {
        if (!Sys_Global.gamePaused && !Sys_Cheats.noHUD) RenderUIImage(672,0,22,22,1020); // Shoot mode button
        bool mouseReleased = Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_LEFT].pressed;
        if (Sys_Global.inventoryMode) {
            if (CursorIsOverBounds(672,694,22,0)) {
                if (mouseReleased) {
                    Sys_Global.inventoryMode = false;
                    cursorPosition_x = Sys_Settings.ScreenWidth / 2;
                    cursorPosition_y = Sys_Settings.ScreenHeight / 2;
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

DepthSort visibleInstances[INSTANCE_COUNT];
static inline void swap(char *a, char *b, size_t sz) { while (sz--) { char t=*a; *a++=*b; *b++=t; } }

static void heapsort(char *base, size_t n, size_t sz, int (*cmp)(const void*, const void*)) {
    if (n < 2) return;

    size_t i, j, k;
    char *a = base;  // just for readability

    // build heap
    for (i = n / 2; i-- > 0; ) {
        j = i;
        for (;;) {
            k = 2 * j + 1;
            if (k >= n) break;               // ← crucial
            if (k + 1 < n && cmp(a + (k + 1) * sz, a + k * sz) > 0)
                k++;
            if (cmp(a + j * sz, a + k * sz) >= 0) break;
            swap(a + j * sz, a + k * sz, sz);
            j = k;
        }
    }

    // extract
    while (n > 1) {
        swap(a, a + (n - 1) * sz, sz);
        n--;

        j = 0;
        for (;;) {
            k = 2 * j + 1;
            if (k >= n) break;               // ← crucial
            if (k + 1 < n && cmp(a + (k + 1) * sz, a + k * sz) > 0)
                k++;
            if (cmp(a + j * sz, a + k * sz) >= 0) break;
            swap(a + j * sz, a + k * sz, sz);
            j = k;
        }
    }
}

void qsort(void *base, size_t nmemb, size_t size, int (*cmp)(const void*, const void*)) {
    if (nmemb < 2) return;

    char *b = (char*)base;
    size_t stack[64], sp = 0, n = nmemb, depth = 0;

    while (1) {
        if (n > 32) {
            if (++depth > 64) {
                heapsort(b, n, size, cmp);
                goto pop;
            }

            // median-of-3 pivot selection (your original, but safer)
            char *lo = b;
            char *hi = b + (n-1)*size;
            char *p  = b + (n/2)*size;

            // simple median-of-3 swap to front
            if (cmp(lo, p) > 0) swap(lo, p, size);
            if (cmp(lo, hi) > 0) swap(lo, hi, size);
            if (cmp(p, hi) > 0) swap(p, hi, size);
            swap(lo, p, size);          // pivot now at b
            p = b;

            // Lomuto partition (your loop, unchanged)
            for (lo = b + size; lo <= hi; lo += size) {
                if (cmp(lo, p) < 0) {
                    p += size;
                    if (p != lo) swap(p, lo, size);
                }
            }
            swap(b, p, size);           // final pivot position

            // === FIXED stack push + recurse on smaller first ===
            size_t left  = (p - b) / size;        // elements < pivot
            size_t right = (b + n*size - p - size) / size;  // elements > pivot

            if (left > right) {                       // push larger first (smaller on top)
                stack[sp++] = right; stack[sp++] = (p - b)/size + 1;  // right subarray start offset
                n = left;
            } else {
                stack[sp++] = left;  stack[sp++] = 0;                 // left subarray
                n = right;
                b = p + size;
            }
            continue;
        }

        // insertion sort (unchanged, safe)
        for (char *lo = b + size; lo < b + n*size; lo += size) {
            for (char *p = lo; p > b && cmp(p - size, p) > 0; p -= size) {
                swap(p - size, p, size);
            }
        }

pop:
        if (sp == 0) return;
        n = stack[--sp];
        b = (char*)base + stack[--sp] * size;
    }
}

static inline __attribute__((always_inline)) void RenderInstances(Vector3 playerPos, bool transparents) {
    uint16_t visibleCount = 0, currentTexIndex = 0, currentNormIndex = 0, currentGlowIndex = 0, currentSpecIndex = 0, currentModelType = 0;
    bool skyVisible = (gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX);
    for (uint16_t i = START_INDEX_LEVEL_INSTANCES; i < INSTANCE_COUNT; ++i) {
        if (EntNotVisible(i,(transparentTexture[Sys_Global.instances[i].texIndex] ^ transparents))) continue; // must be transparent && transparents or neither
        
        Vector3 objPos = Sys_Global.instances[i].position;
        uint16_t instCellIdx = PosGetCellCoords(objPos.x, objPos.z);
        Vector3 delta = Vector3_A_minus_B(objPos, playerPos);
        float distSqrd = delta.x*delta.x + delta.y*delta.y + delta.z*delta.z;
        if (distSqrd >= FAR_PLANE_SQUARED && (Sys_Global.instances[i].index != 754 || !skyVisible) && i != editModeSelection) continue;

        if (EntityIndexIsPortalBlockingDoor(Sys_Global.instances[i].index)) { // Extra checks only needed for opaque portal blocking doors.
            bool inPVS = (gridCellStates[instCellIdx] & CELL_VISIBLE);
            if (!inPVS) {
                uint16_t cellX = (uint16_t)clamp((int32_t)vfloor((objPos.x - worldMin_x + CELLXHALF) / WORLDCELL_WIDTH_F), 0, WORLDX_0BASED);
                uint16_t cellZ = (uint16_t)clamp((int32_t)vfloor((objPos.z - worldMin_z + CELLXHALF) / WORLDCELL_WIDTH_F), 0, WORLDX_0BASED);
                int r = vfloor(5.12f * (1.0f / WORLDCELL_WIDTH_F));
                for (int ix = cellX - r; ix <= (int)cellX + r && !inPVS; ++ix) {
                    for (int iz = cellZ - r; iz <= (int)cellZ + r; ++iz) {
                        if (!XZPairInBounds(ix, iz)) continue;

                        int subIdx = iz * WORLDX + ix;
                        if (get_cull_bit(precomputedVisibleCellsFromHere, instCellIdx * ARRSIZE + subIdx) && (gridCellStates[subIdx] & CELL_VISIBLE)) {
                            inPVS = true;
                            break;
                        }
                    }
                }
            }
            if (!inPVS) continue;
        } else {
            if (!(Sys_Global.currentLevel == 1 && (Sys_Global.instances[i].index == 309 ||  Sys_Global.instances[i].index == 532))) { // Hack for beaker and beaker holder on level 1 shelf getting culled from door portals.
                if (((gridCellStates[instCellIdx] & (CELL_VISIBLE | CELL_OPEN)) == CELL_OPEN) && (Sys_Global.instances[i].index != 754 || !skyVisible)) continue; // For some shelves that are inset away from cells, need to still draw their items by checking && CELL_OPEN here, unfortunately this means they don't ever get culled :(
            }
            
            if (!(gridCellStates[instCellIdx] & CELL_OPEN) && distSqrd >= 943.7184f && (Sys_Global.instances[i].index != 754 || !skyVisible)) continue; // 30.72 * 30.72, 12 cells
        }

        float dotResult = dot_vector3(delta, Sys_Global.instances[PLAYER1].forward);
        float radius = modelBounds[(Sys_Global.instances[i].modelIndex * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_RADIUS] * 2.0f;
        if (dotResult < 0.0f && distSqrd > (radius * radius) && i != editModeSelection) continue;
        
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

        glUniform1ui(0, i);    glUniform1ui(17, Sys_Global.instances[i].texIndex == 316 ? 1u : 0u);
        if (currentNormIndex != (uint32_t)Sys_Global.instances[i].normIndex || Sys_Global.instances[i].normIndex == 0) { currentNormIndex = (uint32_t)Sys_Global.instances[i].normIndex; glUniform1ui(1, currentNormIndex); }
        if (currentTexIndex  != (uint32_t)Sys_Global.instances[i].texIndex)  { currentTexIndex  =  (uint32_t)Sys_Global.instances[i].texIndex; glUniform1ui(18, currentTexIndex); }
        if (currentGlowIndex != (uint32_t)Sys_Global.instances[i].glowIndex || Sys_Global.instances[i].glowIndex == 0) { currentGlowIndex = (uint32_t)Sys_Global.instances[i].glowIndex; glUniform1ui(19, currentGlowIndex); }
        if (currentSpecIndex != (uint32_t)Sys_Global.instances[i].specIndex || Sys_Global.instances[i].specIndex == 0) { currentSpecIndex = (uint32_t)Sys_Global.instances[i].specIndex; glUniform1ui(20, currentSpecIndex); }
        int32_t modelType = (instanceIsLODArray[i] || Sys_Settings.ModelDetail < 1u) && Sys_Global.instances[i].lodIndex < loadedModelsMaxIndex ? Sys_Global.instances[i].lodIndex : Sys_Global.instances[i].modelIndex;
        if (currentModelType != modelType) {
            currentModelType = modelType;
            glBindVertexBuffer(0, Sys_Render.vbos[currentModelType], 0, VERTEX_ATTRIBUTES_COUNT * sizeof(float));
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Sys_Render.tbos[currentModelType]);
        }
        
        uint32_t vertCount = modelTriangleCounts[currentModelType] * 3;
        glDrawElements(GL_TRIANGLES, vertCount, GL_UNSIGNED_INT, 0);
        drawCallsRenderedThisFrame++; verticesRenderedThisFrame += vertCount;
    }
}

static inline __attribute__((always_inline)) void RenderInstancesDepthOnly(Vector3 playerPos) {
    uint16_t visibleCount = 0, currentModelType = 0;
    bool skyVisible = (gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX);
    for (uint16_t i = START_INDEX_LEVEL_INSTANCES; i < INSTANCE_COUNT; ++i) {
        if (EntNotVisible(i,transparentTexture[Sys_Global.instances[i].texIndex])) continue; // must be transparent && transparents or neither
        
        Vector3 objPos = Sys_Global.instances[i].position;
        uint16_t instCellIdx = PosGetCellCoords(objPos.x, objPos.z);
        Vector3 delta = Vector3_A_minus_B(objPos, playerPos);
        float distSqrd = delta.x*delta.x + delta.y*delta.y + delta.z*delta.z;
        if (distSqrd >= FAR_PLANE_SQUARED && (Sys_Global.instances[i].index != 754 || !skyVisible)) continue;

        if (EntityIndexIsPortalBlockingDoor(Sys_Global.instances[i].index)) { // Extra checks only needed for opaque portal blocking doors.
            bool inPVS = (gridCellStates[instCellIdx] & CELL_VISIBLE);
            if (!inPVS) {
                uint16_t cellX = (uint16_t)clamp((int32_t)vfloor((objPos.x - worldMin_x + CELLXHALF) / WORLDCELL_WIDTH_F), 0, WORLDX_0BASED);
                uint16_t cellZ = (uint16_t)clamp((int32_t)vfloor((objPos.z - worldMin_z + CELLXHALF) / WORLDCELL_WIDTH_F), 0, WORLDX_0BASED);
                int r = vfloor(5.12f * (1.0f / WORLDCELL_WIDTH_F));
                for (int ix = cellX - r; ix <= (int)cellX + r && !inPVS; ++ix) {
                    for (int iz = cellZ - r; iz <= (int)cellZ + r; ++iz) {
                        if (!XZPairInBounds(ix, iz)) continue;

                        int subIdx = iz * WORLDX + ix;
                        if (get_cull_bit(precomputedVisibleCellsFromHere, instCellIdx * ARRSIZE + subIdx) && (gridCellStates[subIdx] & CELL_VISIBLE)) {
                            inPVS = true;
                            break;
                        }
                    }
                }
            }
            if (!inPVS) continue;
        } else {
            if (!(Sys_Global.currentLevel == 1 && (Sys_Global.instances[i].index == 309 ||  Sys_Global.instances[i].index == 532))) { // Hack for beaker and beaker holder on level 1 shelf getting culled from door portals.
                if (((gridCellStates[instCellIdx] & (CELL_VISIBLE | CELL_OPEN)) == CELL_OPEN) && (Sys_Global.instances[i].index != 754 || !skyVisible)) continue; // For some shelves that are inset away from cells, need to still draw their items by checking && CELL_OPEN here, unfortunately this means they don't ever get culled :(
            }
            
            if (!(gridCellStates[instCellIdx] & CELL_OPEN) && distSqrd >= 943.7184f && (Sys_Global.instances[i].index != 754 || !skyVisible)) continue; // 30.72 * 30.72, 12 cells
        }

        float dotResult = dot_vector3(delta, Sys_Global.instances[PLAYER1].forward);
        float radius = vmax(modelBounds[(Sys_Global.instances[i].modelIndex * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_RADIUS] * 2.0f,2.56f);
        if (dotResult < 0.0f && distSqrd > (radius * radius)) continue;
        
        visibleInstances[visibleCount].index = i;
        visibleInstances[visibleCount].depth = distSqrd;
        visibleCount++;
    }
    
    if (visibleCount > 1) qsort(visibleInstances, visibleCount, sizeof(DepthSort), compareDepthSortInverted); // Sort by depth (ascending for front-to-back)
    for (uint16_t visibleIndex = 0; visibleIndex < visibleCount; ++visibleIndex) {
        uint16_t i = visibleInstances[visibleIndex].index;
        if (doubleSidedTexture[Sys_Global.instances[i].texIndex] || Sys_Global.instances[i].scale.x < 0.0f || Sys_Global.instances[i].scale.y < 0.0f || Sys_Global.instances[i].scale.z < 0.0f) { glDisable(GL_CULL_FACE); glEnable(GL_BLEND); } // Doublesided
        else { glEnable(GL_CULL_FACE); glDisable(GL_BLEND); } // Opaque

        glUniform1ui(0, i);
        int32_t modelType = (instanceIsLODArray[i] || Sys_Settings.ModelDetail < 1u) && Sys_Global.instances[i].lodIndex < loadedModelsMaxIndex ? Sys_Global.instances[i].lodIndex : Sys_Global.instances[i].modelIndex;
        if (currentModelType != modelType) {
            currentModelType = modelType;
            glBindVertexBuffer(0, Sys_Render.vbos[currentModelType], 0, VERTEX_ATTRIBUTES_COUNT * sizeof(float));
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Sys_Render.tbos[currentModelType]);
        }
        
        uint32_t vertCount = modelTriangleCounts[currentModelType] * 3;
        glDrawElements(GL_TRIANGLES, vertCount, GL_UNSIGNED_INT, 0);
        drawCallsRenderedThisFrame++; verticesRenderedThisFrame += vertCount;
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
    mul_mat4(viewProj, rasterPerspectiveProjection, view);
    float invViewRot[9] = { view[0], view[4], view[8],    view[1], view[5], view[9],    view[2], view[6], view[10] };
    glBindVertexArray(Sys_Render.vao_chunk); // Common vao for RenderShadowmaps and Rasterized Geometry
    glEnable(GL_DEPTH_TEST);
    if (likely(Sys_Settings.Shadows > 0u)) RenderShadowmaps();
    SetMemoryToValueForNBytes(    lightDirty,0    ,LIGHT_COUNT * sizeof(bool)); // Clear dirty after shadowmaps for minimal shadowmap updating.
    SetMemoryToValueForNBytes(dirtyInstances,0,loadedInstances * sizeof(bool)); // Clear dirty after shadowmaps for minimal shadowmap updating.
    glBindFramebuffer(GL_FRAMEBUFFER, Sys_Render.gBufferFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the corner where last shadowmap wrote into
    glEnable(GL_CULL_FACE); glDisable(GL_BLEND); // Opaques
    
    // Depth Prepass - Eliminates some overdraw for ~6.1% performance improvement in spite of added draw calls since these are relatively cheap and avoid the heavy fragment work in main pass.
    glUseProgram(Sys_Render.depthPrepassShaderProgram);
    glUniformMatrix4fv(2, 1, GL_FALSE, viewProj);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    RenderInstancesDepthOnly(playerPos); // opaques only
    
    // Main Pass
    glUseProgram(Sys_Render.chunkShaderProgram);
    glUniformMatrix4fv(2, 1, GL_FALSE, viewProj);
    glUniform1ui(3, 0u); // isUI false
    float fogActual = fogBaseDensityForLevel + (float)(Sys_Global.fogFac / 255u);
    glUniform3f(4, fogColorR * fogActual, fogColorG * fogActual, fogColorB * fogActual); // Fog Color(which is density)
    glUniform1ui(14, Sys_Settings.Reflections);   glUniform1ui(15, Sys_Settings.Shadows);
    glUniform1f(8, worldMin_x);   glUniform1f(9, worldMin_z);    glUniform3f(10, playerPos.x, playerPos.y, playerPos.z);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_EQUAL);
    RenderInstances(playerPos, false);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    RenderInstances(playerPos, true); // opaque, then transparents
    
    // Draw Debug Lines
    if (unlikely(Sys_Global.debugLineVertCount > 1)) DrawDebugLines(viewProj);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if (likely(Sys_Settings.Reflections > 0u)) { // Screen Space Reflections
        glUseProgram(Sys_Render.ssrShaderProgram);
        glUniformMatrix4fv(4, 1, GL_FALSE, viewProj);
        glUniform3f(3, playerPos.x, playerPos.y, playerPos.z);
        GLuint groupX_ssr = ((Sys_Settings.ScreenWidth  / Sys_Settings.SSR_RES) + 31) / 32;
        GLuint groupY_ssr = ((Sys_Settings.ScreenHeight / Sys_Settings.SSR_RES) + 31) / 32;
        glDispatchCompute(groupX_ssr, groupY_ssr, 1);
    }

    glUseProgram(Sys_Render.imageBlitShaderProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Sys_Render.inputImageID);
    glUniform1i(4, 4); // outputImage texture sampler2D
    double berserkTimeRemainingNormalized = berserkFinished > 0.0001 ? (berserkFinished - Sys_Global.pauseRelativeTime) / BERSERK_TIME : 0.0;
    if (berserkFinished < Sys_Global.pauseRelativeTime && berserkFinished > 0.0001) berserkFinished = berserkTimeRemainingNormalized = 0.0;
    glUniform1ui(5, Sys_Settings.Reflections);
    glUniform1ui(6, Sys_Settings.AntiAliasing);
    glUniform1f(14, Sys_Settings.FOV);
    glUniform1f(16, (float)Sys_Settings.ScreenWidth / (float)Sys_Settings.ScreenHeight);
    glUniform1ui(22, Sys_Settings.Shadows);
    glUniform1f(9, (float)berserkTimeRemainingNormalized);
    glUniform1f(10, berserkSeedTime);
    glUniform1ui(11, Sys_Settings.Brightness);
    glUniform3f(12, deg2rad(cam_yaw), deg2rad(cam_pitch), deg2rad(cam_roll));
    glUniform3f(13, px, py, pz);
    glUniform1f(15, (float)Sys_Global.pauseRelativeTime * 0.1f);
    glUniform1ui(17, (gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX) || Sys_Global.currentLevel == LEVEL_CYBERSPACE);
    glUniform1ui(18, (gridCellStates[playerCellIdx] & CELL_SEES_SUN) && Sys_Global.currentLevel != LEVEL_CYBERSPACE);
    glUniform1ui(19, ((Sys_Global.currentLevel >= 10 && Sys_Global.currentLevel < LEVEL_CYBERSPACE) ? 1u : 0u) && (gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX));
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
    glBindVertexArray(Sys_Render.quadVAO);
    glDisable(GL_DEPTH_TEST); // Reenabled later after all UI just up there before RenderShadowmaps call
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    drawCallsRenderedThisFrame++; verticesRenderedThisFrame += 4;

    // UI
    Sys_Global.last_time = RenderUI();
    
    // Cursor [ /// VERY LAST DRAWN OVER EVERYTHING ELSE! /// ]
    bool menuOrInventoryCursorStyle = (Sys_Global.gamePaused || Sys_Global.menuActive);
    uint16_t cursorTexture = menuOrInventoryCursorStyle ? 1261 : 1260;
    if ((Sys_Global.inventoryMode && !Sys_Cheats.noHUD) || Sys_Global.menuActive || Sys_Global.gamePaused) RenderUIImage((int16_t)(cursorPosition_x) - 20, (int16_t)(cursorPosition_y) - 20, 40,40, cursorTexture);
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
void UpdateAnims(void); void UpdateAmbientSounds(void);
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
        ProcessInput(); // Calls ApplyPlayerMovements(), needs called without checking paused state for menus handling.
        if (likely(!Sys_Global.gamePaused && !Sys_Global.menuActive)) { // Update Gameplay
            ModUpdate();
            if (Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_RIGHT].released) Frob(Sys_Global.instances[PLAYER1].position, Sys_Global.instances[PLAYER1].forward, Sys_Global.instances[PLAYER1].right);
            if (Sys_Global.current_time < Sys_Global.debugLineFinished && (Sys_Global.debugLineVertCount + 6) < (MAX_DEBUG_LINE_VERTS * 3)) AddDebugLine(Sys_Global.debugLine_start, Sys_Global.debugLine_end);
//             for (uint16_t i=START_INDEX_LEVEL_INSTANCES;i<loadedInstances;++i) UpdateWhileNotPaused(i); // TODO Get new states prior to updating animations, physics event, or rendering
//             Sys_Global.instances[editModeSelection].index = editModeTestEntityDefinition;
//             Sys_Global.instances[editModeSelection].modelIndex = entities[editModeTestEntityDefinition].modelIndex;
//             Sys_Global.instances[editModeSelection].texIndex = entities[editModeTestEntityDefinition].modelIndex;
            UpdateAmbientSounds();
        }

        if (!Sys_Global.gamePaused && !Sys_Global.menuActive) UpdatePlayerFacingAngles();
        Sys_Global.timeSinceLastPhysicsTick = Sys_Global.pauseRelativeTime - Sys_Global.last_physics_time;
        if (!Sys_Global.gamePaused && !Sys_Global.menuActive && Sys_Global.timeSinceLastPhysicsTick > (1.0 / 144.0)) {
            Sys_Global.last_physics_time = Sys_Global.pauseRelativeTime;
            Physics();
        }

        if (likely(!Sys_Global.gamePaused && !Sys_Global.menuActive)) { UpdateAnims(); UpdateMusic(); }
        if (likely(!Sys_Global.gamePaused || Sys_Global.menuActive)) {
            Sys_Render.shadowmapsNeedUpdated = UpdatedPlayerCell();
            Sys_Render.shadowmapsNeedUpdated = UpdateLights(&Sys_Render.shadowmapsNeedUpdated);
            CullCore();
            bool uploadInstances = false;
            for (uint32_t i = START_INDEX_LEVEL_INSTANCES; i < loadedInstances; i++) {
                if (dirtyInstances[i]) {
                    if (Sys_Global.instances[i].modelIndex >= loadedModelsMaxIndex || modelVertexCounts[Sys_Global.instances[i].modelIndex] < 1) { dirtyInstances[i] = false; continue; } // No model or empty model

                    uploadInstances = true;    Sys_Render.shadowmapsNeedUpdated = true;
                    float x = Sys_Global.instances[i].rotation.x, y = Sys_Global.instances[i].rotation.y, z = Sys_Global.instances[i].rotation.z, w = Sys_Global.instances[i].rotation.w;
                    float x2 = x * x,   y2 = y * y,   z2 = z * z,   xy = x * y,   xz = x * z,   yz = y * z,   wx = w * x,   wy = w * y,   wz = w * z;
                    float sclx = Sys_Global.instances[i].scale.x; float scly = Sys_Global.instances[i].scale.y; float sclz = Sys_Global.instances[i].scale.z;
                    modelMatrices[(i * 16) + 0] = (1.0f - 2.0f * (y2 + z2)) * -sclx; // Right X, Necessary -x for blender right to left handed coordinate conversion.
                    modelMatrices[(i * 16) + 1]  = (2.0f * (xy + wz)) * -sclx; // Right Y
                    modelMatrices[(i * 16) + 2]  = (2.0f * (xz - wy)) * -sclx; // Right Z
                    modelMatrices[(i * 16) + 3] = modelMatrices[(i * 16) + 7] = modelMatrices[(i * 16) + 11] = 0.0f;
                    modelMatrices[(i * 16) + 4]  = (2.0f * (xy - wz)) * scly; // Up X
                    modelMatrices[(i * 16) + 5]  = (1.0f - 2.0f * (x2 + z2)) * scly; // Up Y
                    modelMatrices[(i * 16) + 6]  = (2.0f * (yz + wx)) * scly; // Up Z
                    modelMatrices[(i * 16) + 8]  = (2.0f * (xz + wy)) * sclz; // Forward X
                    modelMatrices[(i * 16) + 9]  = (2.0f * (yz - wx)) * sclz; // Forward Y
                    modelMatrices[(i * 16) + 10] = (1.0f - 2.0f * (x2 + y2)) * sclz; // Forward Z
                    modelMatrices[(i * 16) + 12] = Sys_Global.instances[i].position.x;   modelMatrices[(i * 16) + 13] = Sys_Global.instances[i].position.y;   modelMatrices[(i * 16) + 14] = Sys_Global.instances[i].position.z;
                    modelMatrices[(i * 16) + 15]= 1.0f;
                }
            }
            if (uploadInstances) glNamedBufferData(Sys_Render.matricesBufferID, loadedInstances * 16 * sizeof(float), modelMatrices, GL_DYNAMIC_DRAW);
        }
        
        Render();
        Sys_Global.globalFrameNum++;
        InputClearRisingAndFallingEdges();
        Sys_Input.currentMouse_dx = Sys_Input.currentMouse_dy = 0;
        #ifdef DEBUG_RAM_OUTPUT
            if (Sys_Global.globalFrameNum == 4) { DebugRAM("after 4 frames of running"); }
            else if (Sys_Global.globalFrameNum == 100) { DebugRAM("after 100 frames of running"); }
            else if (Sys_Global.globalFrameNum == 200) DebugRAM("after 200 frames of running");
            else if (Sys_Global.globalFrameNum == 500) DebugRAM("after 500 frames of running");
            else if (Sys_Global.globalFrameNum == 1000) DebugRAM("after 1000 frames of running");
        #endif
    }
    return 0;
}
