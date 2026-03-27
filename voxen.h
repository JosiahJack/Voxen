// voxen.h - Engine specific shared values
#pragma once
#if defined(LINUX)
//     #define DEBUG_RAM_OUTPUT // Debug and Compile Flags
#endif
#include "common.h" // Types needed first
#include "interop.h"
#define MAX_KEYS 512
#define MAX_MOUSE_BUTTONS 8
#define MAX_JOYSTICK_BUTTONS 16
#define MAX_JOYSTICK_HATS 5
#define MAX_GAMEPAD_BUTTONS 20
#define MAX_SHADOWMAPS 96u
#define SHADOW_MAP_SIZE 128u
#define TOTAL_SHADOWMAP_PIXELS (MAX_SHADOWMAPS * (SHADOW_MAP_SIZE * SHADOW_MAP_SIZE * 6U))
#define MAX_SAVENAME_LENGTH 24
#define MAX_PALETTE_SIZE 256
#define MAX_TEXTURE_DIMENSION 2048
#define VERTEX_ATTRIBUTES_COUNT 8 // x,y,z,nx,ny,nz,u,v
#define VERTEX_ATTRIBUTES_SIZE 16 // Was 32
#define BOUNDS_ATTRIBUTES_COUNT 7
#define BOUNDS_DATA_OFFSET_MINX 0
#define BOUNDS_DATA_OFFSET_MINY 1
#define BOUNDS_DATA_OFFSET_MINZ 2
#define BOUNDS_DATA_OFFSET_MAXX 3
#define BOUNDS_DATA_OFFSET_MAXY 4
#define BOUNDS_DATA_OFFSET_MAXZ 5
#define BOUNDS_DATA_OFFSET_RADIUS 6
#define WORLDX 64
#define WORLDZ WORLDX
#define WORLDY 18 // Level 8 is only 17.5 cells tall!!  Could be 16 if I make the ceiling same height in last room as in original.
#define WORLDX_0BASED (WORLDX - 1)
#define WORLDZ_0BASED (WORLDZ - 1)
#define TOTAL_WORLD_CELLS (WORLDX * WORLDY * WORLDZ)
#define ARRSIZE (WORLDX * WORLDZ)
#define WORLDCELL_WIDTH_F 2.56f
#define CELLXHALF (WORLDCELL_WIDTH_F * 0.5f)
#define PRECOMPUTED_VISIBILITY_SIZE 524288 // 4096 * 4096 / 32
#define VOXEL_COUNT 262144 // 64 * 64 * 8 * 8
#define MAX_LIGHTS_PER_VOXEL 32 // Cap to prevent overflow
#define CELL_VISIBLE       1u
#define CELL_OPEN          2u
#define CELL_CLOSEDNORTH   4u
#define CELL_CLOSEDEAST    8u
#define CELL_CLOSEDSOUTH  16u
#define CELL_CLOSEDWEST   32u
#define CELL_SEES_SUN     64u
#define CELL_SEES_SKYBOX 128u
// Lights
                           //    0     1     2          3       4        5         6         7         8         9 10 11 12
#define LIGHT_DATA_SIZE 13 // posx, posy, posz, intensity, radius, spotAng, spotDirx, spotDiry, spotDirz, spotDirw, r, g, b
      // Make sure this^^^^ matches in chunk.glsl shader!
#define LIGHT_DATA_OFFSET_POSX 0
#define LIGHT_DATA_OFFSET_POSY 1
#define LIGHT_DATA_OFFSET_POSZ 2
#define LIGHT_DATA_OFFSET_INTENSITY 3
#define LIGHT_DATA_OFFSET_RANGE 4
#define LIGHT_DATA_OFFSET_SPOTANG 5
#define LIGHT_DATA_OFFSET_SPOTDIRX 6
#define LIGHT_DATA_OFFSET_SPOTDIRY 7
#define LIGHT_DATA_OFFSET_SPOTDIRZ 8
#define LIGHT_DATA_OFFSET_SPOTDIRW 9
#define LIGHT_DATA_OFFSET_R 10
#define LIGHT_DATA_OFFSET_G 11
#define LIGHT_DATA_OFFSET_B 12
// Make sure these match in chunk.glsl shader!
#define LIGHT_MAX_INTENSITY 8.0f
#define LIGHT_RANGE_MAX 15.36f
#define LIGHT_RANGE_MAX_SQUARED (LIGHT_RANGE_MAX * LIGHT_RANGE_MAX)

#define DEBUG_OPENGL
#ifdef DEBUG_OPENGL
	#define CHECK_GL_ERROR() do { GLenum err = glGetError(); if (err != GL_NO_ERROR) DualLogError("GL Error at %s:%d: %d\n", __FILE__, __LINE__, err); } while(0)
#else
	#define CHECK_GL_ERROR() do {} while(0)
#endif
    
#define FAR_PLANE (71.68f) // Max player view, level 6 crawlway 28 cells
#define NEAR_PLANE (0.02f)
#define FAR_PLANE_SQUARED (FAR_PLANE * FAR_PLANE)
#define TEXT_BUFFER_SIZE 1024
#define FONT_ATLAS_SIZE 4672
#define MAX_GLYPHS 4096
extern GlobalContext Sys_Global;
extern SystemUI Sys_UI;
typedef struct { bool down; bool pressed; bool released; } KeyState;
typedef struct {
	KeyState keyStates[MAX_KEYS];
	KeyState mouseButtons[MAX_MOUSE_BUTTONS];
	KeyState gamepadButtons[MAX_GAMEPAD_BUTTONS];
	bool joystickPresent[16];
	KeyState joystickButtons[16][MAX_JOYSTICK_BUTTONS];
	KeyState joystickHats[MAX_JOYSTICK_HATS]; // What can I say, I'm a man of many hats. ^^D
	bool window_has_focus;
	double last_mouse_x, last_mouse_y;
    int32_t currentMouse_dx, currentMouse_dy;
	double scrollDelta;
	bool ignore_next_mouse_delta;
	bool lastUse;
	bool isCapsLockOn;
} InputSystem;
extern InputSystem Sys_Input;

typedef struct {
    double shadowTime;
    uint32_t numShadowsCouldRender;
    uint32_t shadowmapSizes[MAX_SHADOWMAPS];
    uint32_t shadowmapOffsets[MAX_SHADOWMAPS];
    uint32_t shadowmapIndirectionList[LIGHT_COUNT];
    float shadDotThresh;
} VoxenShadowSystem;
extern VoxenShadowSystem voxen_Shadow_System;

typedef struct {
    double timestamp;
    double deltaTime_ns;
    uint32_t frameNum; // Can't unionize the payloads as some need both.
    int32_t payload1i; // First one used for payloads less than or equal to 4 bytes
    int32_t payload2i; // Second one used for more values or for long ints by using bitpacking
    float payload1f;   // First one used for float payloads
    float payload2f;   // Second one used for a 2nd value or for double via bitpacking
    uint8_t type;
} Event;

typedef struct { Vector3 normal; float d; } FrustumPlane;

typedef struct StbiArena {
    uint8_t* base;
    uint8_t* cursor;
    uint8_t* end;
} StbiArena;

typedef uint32_t GLuint;
typedef struct {
	GLuint inputImageID;
	GLuint inputDepthID;
	GLuint inputWorldPosID;
	GLuint inputSpecID;
	GLuint inputNormalID;
	GLuint inputImageLastID;
	GLuint gBufferFBO;
	GLuint outputImageID;
    GLuint depthPrepassShaderProgram;
	GLuint chunkShaderProgram; // Generic lit and unlit raster shader forward+
	GLuint debugUnlitShaderProgram;
	GLuint vao_chunk; // Vertex Array Object
	GLuint shadowFBO;
	GLuint shadowmapsShaderProgram;
	GLuint shadowmapsClearShaderProgram;
	GLuint shadowMapSSBO;
	GLuint ssrShaderProgram; // SSR (Screen Space Reflections)
	GLuint imageBlitShaderProgram; // Full Screen Quad Blit for rendering final compositing output/image effect passes
	GLuint quadVAO;
	GLuint quadVBO;
	GLuint textShaderProgram;
	GLuint textVAO;
	GLuint textVBO;
	GLuint debugLinesVAO;
	GLuint debugLinesVBO;
	GLuint blueNoiseBuffer;
    GLuint modelAnimDeltasID;
    GLuint modelAnimDeltaOffsetsID;
	GLuint matricesBufferID;
	GLuint colorBufferID;
	GLuint texturePalettesID;
	GLuint textureOffsetsID;
	GLuint textureSizesID;
	GLuint texturePaletteOffsetsID;
	GLuint lightsID;
	GLuint shadowMapsIndirectionID;
	GLuint cellVisibleDataID;
	GLuint voxelUpdateShaderProgram;
	GLuint voxelLightListCountsID;
	GLuint uniqueLightListsID;
	GLuint vbos[MODEL_IDX_MAX];
	GLuint tbos[MODEL_IDX_MAX];
	bool shadowmapsNeedUpdated;
} RenderSystem;

extern bool instanceIsLODArray[INSTANCE_COUNT];
extern float modelMatrices[INSTANCE_COUNT * 16];
extern const char* sounds[670];
extern const char* audioLogs[134];
void play_mp3(const char* path, int32_t fade_in_ms);
void play_wav(const char* path, float volume, Vector3 pos, bool positional);
extern float modelBounds[MODEL_IDX_MAX * BOUNDS_ATTRIBUTES_COUNT];
extern uint8_t** modelVertices;
extern uint16_t** modelTriangles;
extern uint16_t loadedTexturesMaxIndex;
extern uint16_t loadedModelsMaxIndex;
extern uint16_t loadedLights;
extern Vector3 lightsNewPosition[LIGHT_COUNT];
extern uint32_t modelVertexCounts[MODEL_IDX_MAX];
extern uint16_t modelTriangleCounts[MODEL_IDX_MAX];
extern bool modelHasAnimation[MODEL_IDX_MAX];
extern float lights[LIGHT_COUNT * LIGHT_DATA_SIZE];
extern bool lightOn[LIGHT_COUNT];
extern bool lightInPVS[LIGHT_COUNT];
extern bool lightCastsShadows[LIGHT_COUNT];
extern bool lightLerpOn[LIGHT_COUNT];
extern bool lightLerpUp[LIGHT_COUNT];
extern uint8_t lightCurrentStep[LIGHT_COUNT];
extern float lightLerpValue[LIGHT_COUNT];
extern float lightLerpTime[LIGHT_COUNT];
extern float lightLerpStepTime[LIGHT_COUNT];
extern float lightLerpStartTime[LIGHT_COUNT];
extern uint8_t lightIntervalStepsLength[LIGHT_COUNT];
extern float lightIntervalSteps[LIGHT_COUNT][30];
extern uint8_t lightIntervalStepIsLerpingLength[LIGHT_COUNT];
extern float intervalStepisLerping[LIGHT_COUNT][30];
extern float lightMinIntensity[LIGHT_COUNT];
extern float lightMaxIntensity[LIGHT_COUNT];
extern uint8_t queuedLevelToLoad;
extern uint16_t playerCellIdx;
extern uint16_t numCellsVisible;
extern uint32_t gridCellStates[ARRSIZE];
extern uint32_t precomputedVisibleCellsFromHere[524288];
bool get_cull_bit(const uint32_t* arr, int idx);
void RenderFormattedText(int16_t x, int16_t y, uint32_t color, uint8_t fontID, float scale, const char * restrict format, ...);
// ----------------------------------------------------------------------------
// Physics
extern uint16_t testPointInSolid;
void AddForce(uint16_t idx, Vector3 force, bool isImpulse);
int32_t Physics(void); // Main event tick
RaycastHit Raycast(Vector3 origin, Vector3 dir, float distance, uint32_t layerMask);
void RaycastAll(Vector3 origin, Vector3 dir, float distance, uint32_t layerMask, RaycastHit* hits, uint16_t maxCount);
bool CheckCapsule(Vector3 start, Vector3 end, float capsuleRadius, float capsuleHeight, uint32_t layerMask);
RaycastHit CapsuleCast(Vector3 start, Vector3 end, float capsuleRadius, float castDist, uint32_t layerMask, bool hitTriggers);
void ApplyPlayerMovements(void);
// ----------------------------------------------------------------------------
// Input
void Input_MouselookApply(void);
int32_t Input_KeyDown(int32_t scancode);
int32_t Input_KeyUp(int32_t scancode);
int32_t Input_MouseMove(int32_t xrel, int32_t yrel);
bool MouseWheelBoundAndRolled(int setCode);
void UpdatePlayerFacingAngles(void);
void InputClearRisingAndFallingEdges(void);
void LoadConfig(void);
void SaveConfig(void);
// ----------------------------------------------------------------------------
// Rendering
extern bool lightDirty[LIGHT_COUNT];
extern float cam_yaw, cam_pitch, cam_roll;
extern uint16_t loadedModelsMaxIndex;
extern bool enteringPlayerName;
void ConsoleEmulator(int32_t keycode);
void SetSkyRotateSpeed(void);
// ----------------------------------------------------------------------------
// UI
extern bool returnToPause;
extern GLuint fontAtlasTex;
extern GLuint fontAtlasTexStopD;
extern float fixedNumberAdvanceWidth;
extern float fixedNumberAdvanceWidthStopD;
extern bool mouseMovementThisFrame;
extern char consoleEntryText[TEXT_BUFFER_SIZE];
void LoadTextForLanguage(uint8_t lang);
void LoadLogTextForLanguage(uint8_t lang);
void InitFontAtlasses(void);
// ----------------------------------------------------------------------------
// Helper Functions
void Screenshot(void);
void DebugRAM(const char *context);
extern uint32_t random_range_rng;
double get_time(void);
void AddInstance(uint16_t entIdx, uint16_t instanceIdx);
uint32_t xs32(void);
char* data_parser_trim(char* s);
int32_t StringToInt(const char *str);
size_t GetStringLength(const char *s);
bool CharacterIsEmpty(const char c);
bool StringIsEmpty(const char* c);
bool StringsAreEqual(const char* c, const char* c2);
bool StringsAreEqualLimitedBy(const char* a, const char* b, size_t limit);
void StringCopyInto_A_From_B(char* a, const char* b, size_t bufferSize);
void StringCopyInto_A_SubstringFrom_B(char* a, size_t substringSize, const char* b, size_t bufferSize);
void StringConcatenate(char* a, const char* b, size_t bufferSize);
extern RenderSystem Sys_Render; // Added last to make use of all defines for sizes.
bool EntityIsAnimated(uint16_t entIdx);

static inline __attribute__((always_inline)) void mul_mat4(float *out, const float *a, const float *b) { // out = a * b
	out[0] =  a[0] * b[0]  + a[4] * b[1]  + a[8]  * b[2] + a[12]  * b[3];
	out[1] =  a[1] * b[0]  + a[5] * b[1]  + a[9]  * b[2] + a[13]  * b[3];
	out[2] =  a[2] * b[0]  + a[6] * b[1] + a[10]  * b[2] + a[14]  * b[3];
	out[3] =  a[3] * b[0]  + a[7] * b[1] + a[11]  * b[2] + a[15]  * b[3];
	out[4] =  a[0] * b[4]  + a[4] * b[5]  + a[8]  * b[6] + a[12]  * b[7];
	out[5] =  a[1] * b[4]  + a[5] * b[5]  + a[9]  * b[6] + a[13]  * b[7];
	out[6] =  a[2] * b[4]  + a[6] * b[5] + a[10]  * b[6] + a[14]  * b[7];
	out[7] =  a[3] * b[4]  + a[7] * b[5] + a[11]  * b[6] + a[15]  * b[7];
	out[8] =  a[0] * b[8]  + a[4] * b[9]  + a[8] * b[10] + a[12] * b[11];
	out[9] =  a[1] * b[8]  + a[5] * b[9]  + a[9] * b[10] + a[13] * b[11];
	out[10] = a[2] * b[8]  + a[6] * b[9] + a[10] * b[10] + a[14] * b[11];
	out[11] = a[3] * b[8]  + a[7] * b[9] + a[11] * b[10] + a[15] * b[11];
	out[12] = a[0] * b[12] + a[4] * b[13] + a[8] * b[14] + a[12] * b[15];
	out[13] = a[1] * b[12] + a[5] * b[13] + a[9] * b[14] + a[13] * b[15];
	out[14] = a[2] * b[12] + a[6] * b[13] + a[10]* b[14] + a[14] * b[15];
	out[15] = a[3] * b[12] + a[7] * b[13] + a[11]* b[14] + a[15] * b[15];
}

extern uint16_t invalidModelIndexCount;
extern uint16_t startOfDoubleSidedInstances;
extern uint16_t startOfTransparentInstances;
extern uint16_t endOfModels;
void InitializeEntity(Entity* entry);
void LoadLevel(uint8_t curlevel);
void EnableCheatArsenal(uint8_t level);
float GetPainStatic(void);
Color GetPainStaticColor(void);
extern int currentMonitorIndex;
void ScreenShake(float force, double duration);
void Shake(float force);
void InitAfterLoad(void);
void SetVSync(void);
void UpdateProjectionMatrices(void);
void TextEntry(int32_t keycode);
void GoIntoGame(void);
void NewGame(void);
void mp3_clear(void);
void play_message(const char* path);
void set_music_volume(void);
void set_sfx_volume(void);
void set_message_volume(void);
void set_master_volume(void);
char CharToLower(const char c);
char* StringFindSubstring(const char* haystack, const char* needle);
const char* StringFindLastChar(const char* str, const char c);
char* StringFindFirstCharWithin(const char *s, char c);
char* StringReturnUpToDelimiterAndLopOffAndShiftOriginal(char* str, const char delim, char** saveptr);
int StringCompareUpToLength(const char* s1, const char* s2, size_t n);
extern uint16_t loadedTexturesMaxIndex;
extern bool doubleSidedTexture[MAX_VALID_TEXTURE];
extern bool transparentTexture[MAX_VALID_TEXTURE];

static inline __attribute__((always_inline)) uint32_t parse_numberu32(const char* str, const char* line, uint32_t lineNum) {
    if (str == 0 || *str == '\0') { DualLogError("Invalid blank string from line[%d]: %s\n", lineNum+1, line); return 0; }
    while (CharacterIsEmpty((char)*str)) str++;
    while (CharacterIsEmpty(*str)) str++;
    if (*str == '+') str++;
    if (*str == '-') { DualLogError("Invalid input, negative not allowed (%s)\n      from line[%d]: %s\n", str, lineNum+1, line); return 0; }
    unsigned long result = 0;
    while (*str >= '0' && *str <= '9') {
        int digit = *str - '0';
        result = result * 10uL + (unsigned long)digit;
        str++;
    }

    return (uint32_t)result;
}

static inline __attribute__((always_inline)) uint16_t parse_numberu16(const char* str, const char* line, uint32_t lineNum) {
    uint32_t retval = parse_numberu32(str, line, lineNum);
    if (retval > UINT16_MAX) { DualLogError("Value %u out of range for uint16_t from line[%d]: %s\n", retval, lineNum+1, line); return 0; }
    return (uint16_t)retval;
}

static inline __attribute__((always_inline)) uint8_t parse_numberu8(const char* str, const char* line, uint32_t lineNum) {
    uint32_t retval = parse_numberu32(str, line, lineNum);
    if (retval > UINT8_MAX) { DualLogError("Value %u out of range for uint8_t from line[%d]: %s\n", retval, lineNum+1, line); return 0; }
    return (uint8_t)retval;
}

static inline __attribute__((always_inline)) bool parse_bool(const char* str, const char* line, uint32_t lineNum) {
    uint32_t parseval = parse_numberu32(str, line, lineNum);
    if (parseval > 1) DualLogWarn("Loaded %u but expected boolean from line[%u]: %s\n",parseval, lineNum+1, line);
    return parseval > 0 ? true : false;
}

static inline __attribute__((always_inline)) float parse_float(const char* str, const char* line, uint32_t lineNum) {
    if (str == 0 || *str == '\0') { DualLogError("Invalid blank string from line[%d]: %s\n", lineNum+1, line); return 0.0f; }
    
    while (CharacterIsEmpty(*str)) str++;
    bool negative = false;
    if (*str == '-') { negative = true; str++; }
    else if (*str == '+') { str++; }

    double value = 0.0;
    bool has_digit = false;
    while (*str >= '0' && *str <= '9') { // Integer part
        value = value * 10.0 + (*str - '0');
        str++;
        has_digit = true;
    }

    if (*str == '.') { // Decimal part
        str++;
        double frac = 0.0;
        double place = 0.1;
        while (*str >= '0' && *str <= '9') {
            frac += (*str - '0') * place;
            place *= 0.1;
            str++;
            has_digit = true;
        }

        value += frac;
    }

    if (!has_digit) return 0.0f;

    if (negative) value = -value;
    return (float)value;
}

typedef __builtin_va_list va_list;
int StringFormatV(char* buffer, size_t bufferSize, const char* format, va_list args);
int StringFormat(char* buffer, size_t bufferSize, const char* format, ...);
static inline __attribute__((always_inline)) int32_t PosGetCellCoordX(float pos_x) { return (uint16_t)clamp((int32_t)vfloor((pos_x - Sys_Global.worldMin_x + CELLXHALF) / WORLDCELL_WIDTH_F), 0, WORLDX_0BASED); }
static inline __attribute__((always_inline)) int32_t PosGetCellCoordZ(float pos_z) { return (uint16_t)clamp((int32_t)vfloor((pos_z - Sys_Global.worldMin_z + CELLXHALF) / WORLDCELL_WIDTH_F), 0, WORLDX_0BASED); }
static inline __attribute__((always_inline)) bool XZPairInBounds(int32_t x, int32_t z) { return (x < WORLDX && z < WORLDZ && x >= 0 && z >= 0); }

extern uint32_t* texturePaletteOffsets;
extern int32_t* textureSizes;
