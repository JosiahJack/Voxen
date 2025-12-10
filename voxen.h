#pragma once
#define VERSION_STRING "v0.7.4"
// #define DEBUG_RAM_OUTPUT // Debug and Compile Flags

// Generic Lib Includes
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include "./External/glad/gl.h"
#include "./External/glfw3.h"
#include "External/stb_truetype.h"
#include "citadel_enumerations.h"
#include "matvecquat.h"

// Generic Constants
#define MAX_PATH 128

// Global Types
typedef struct { float r,g,b,a; } Color;

extern double timeSinceLastPhysicsTick;
typedef uint8_t PhysCombineType;
typedef uint8_t ColliderType;

typedef struct {
    Vector3 mins;
    Vector3 maxs;
    uint8_t type;
} Trigger;

typedef struct {
    float nx, ny, nz, d;
} FrustumPlane;

typedef struct {
    GLFWwindow* window;
} Voxen_GlobalContext;
extern Voxen_GlobalContext voxen_globalContext;

typedef struct {
	GLuint inputImageID;
	GLuint inputDepthID;
	GLuint inputWorldPosID;
	GLuint inputSpecID;
	GLuint gBufferFBO;
	GLuint outputImageID;
	GLuint chunkShaderProgram; // Generic lit and unlit raster shader forward+
	GLuint vao_chunk; // Vertex Array Object
	GLuint shadowFBO;
	GLuint shadowmapsShaderProgram;
	GLuint shadowmapsClearShaderProgram;
	GLuint shadowMapSSBO;
	GLuint ssrShaderProgram; // SSR (Screen Space Reflections)
	GLuint imageBlitShaderProgram; // Full Screen Quad Blit for rendering final compositing output/image effect passes
	GLuint quadVAO, quadVBO;
	GLuint textShaderProgram;
	GLuint textVAO, textVBO;
	GLuint blueNoiseBuffer;
} Voxen_GL_Comms;
extern Voxen_GL_Comms voxen_GL_Comms;

typedef struct {
	int lev1SecCode;
	int lev2SecCode;
	int lev3SecCode;
	int lev4SecCode;
	int lev5SecCode;
	int lev6SecCode;
	bool lev1SecCodeLocked;
	bool lev2SecCodeLocked;
	bool lev3SecCodeLocked;
	bool lev4SecCodeLocked;
	bool lev5SecCodeLocked;
	bool lev6SecCodeLocked;
	bool RobotSpawnDeactivated;
	bool IsotopeInstalled;
	bool ShieldActivated;
	bool LaserSafetyOverriden;
	bool LaserDestroyed;
	bool BetaGroveCyberUnlocked;
	bool GroveAlphaJettisonEnabled;
	bool GroveBetaJettisonEnabled;
	bool GroveDeltaJettisonEnabled;
	bool MasterJettisonBroken;
	bool Relay428Fixed;
	bool MasterJettisonEnabled;
	bool BetaGroveJettisoned;
	bool AntennaNorthDestroyed;
	bool AntennaSouthDestroyed;
	bool AntennaEastDestroyed;
	bool AntennaWestDestroyed;
	bool SelfDestructActivated;
	bool BridgeSeparated;
	bool IsolinearChipsetInstalled;
} QuestBits;
extern QuestBits questData;

typedef struct {
	uint16_t ScreenWidth;
	uint16_t ScreenHeight;
	uint8_t Shadows;
	uint8_t AntiAliasing;
	uint8_t Brightness;
	uint8_t VolumeMusic;
	uint8_t Language;
	uint8_t CullEnabled;
	float FOV;
	uint8_t Reflections;
	bool Vsync;
} Settings;
extern Settings voxen_Settings;
// ----------------------------------------------------------------------------
// Audio
#define MAX_AMBIENT_NOISES 32
extern uint16_t loadedAmbients;
extern uint16_t ambientRegistry[MAX_AMBIENT_NOISES];
void play_mp3(const char* path, float volume, int32_t fade_in_ms);
void play_wav(const char* path, float volume);
void InitializeAudio(void);
void UpdateAmbientSounds(void);
// ----------------------------------------------------------------------------
// Textures
#define MAX_VALID_TEXTURE 2048
#define MAX_TEXTURE_DIMENSION 2048
#define MAX_PALETTE_SIZE 256
#define MATERIAL_IDX_MAX 2048 // Max value the bit packing bits allow
extern bool doubleSidedTexture[MAX_VALID_TEXTURE];
extern bool transparentTexture[MAX_VALID_TEXTURE];
bool isDoubleSided(uint32_t texIndexToCheck);
bool isTransparent(uint32_t texIndexToCheck);
void LoadTextures(void);
// ----------------------------------------------------------------------------
// Models
#define MODEL_COUNT 680
#define MODEL_IDX_MAX 1024 // Max value the bit packing bits allow
#define VERTEX_ATTRIBUTES_COUNT 8 // x,y,z,nx,ny,nz,u,v
#define BOUNDS_ATTRIBUTES_COUNT 7
#define BOUNDS_DATA_OFFSET_MINX 0
#define BOUNDS_DATA_OFFSET_MINY 1
#define BOUNDS_DATA_OFFSET_MINZ 2
#define BOUNDS_DATA_OFFSET_MAXX 3
#define BOUNDS_DATA_OFFSET_MAXY 4
#define BOUNDS_DATA_OFFSET_MAXZ 5
#define BOUNDS_DATA_OFFSET_RADIUS 6
extern float modelBounds[MODEL_IDX_MAX * BOUNDS_ATTRIBUTES_COUNT];
extern float** modelVertices;
extern GLuint vbos[MODEL_IDX_MAX];
extern GLuint tbos[MODEL_IDX_MAX];
extern uint16_t loadedTextures;
extern uint16_t loadedModels;
extern uint16_t loadedLights;
extern uint16_t numDynamicLights;
extern uint16_t gameObjectCount;
extern uint32_t modelVertexCounts[MODEL_IDX_MAX];
extern uint32_t modelTriangleCounts[MODEL_IDX_MAX];
void LoadModels(void);

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
                   
#define LIGHT_COUNT 1600 // MAX CITADEL LIGHT COUNT is 1561 for Level 7, leaves room for dynamic lights from projectiles
   // Make sure this ^^^ matches in shadowmaps_clear_selective compute shader!

#define LIGHT_MAX_INTENSITY 8.0f
#define LIGHT_RANGE_MAX 15.36f
#define LIGHT_RANGE_MAX_SQUARED (LIGHT_RANGE_MAX * LIGHT_RANGE_MAX)
#define SHADOW_MAP_SIZE 256u
#define SHADOW_MAP_SIZE_SQD (SHADOW_MAP_SIZE * SHADOW_MAP_SIZE)
#define MAX_SHADOWMAPS 62u
#define TOTAL_SHADOWMAP_PIXELS (MAX_SHADOWMAPS * (SHADOW_MAP_SIZE * SHADOW_MAP_SIZE * 6U))
#define SHADOWMAP_FOV 90.0f

extern float lights[LIGHT_COUNT * LIGHT_DATA_SIZE];
extern float lightsRangeSquared[LIGHT_COUNT];
extern float lightBaseIntensity[LIGHT_COUNT];
extern bool lightOn[LIGHT_COUNT];
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
void UpdateVoxelLightLists(void);
void RenderShadowmaps(void);
void RenderLoadingProgress(int32_t offset, const char* format, ...);

// Levels / Game Management
#define LEVEL_CYBERSPACE 13
extern char global_modname[256];
extern uint8_t startLevel;
extern uint8_t numLevels; // Can be set by gamedata.txt
extern uint8_t currentLevel;
extern bool gamePaused;
extern bool menuActive;
extern bool levelCurrentlyLoading;
extern GLuint matricesBuffer;

// ----------------------------------------------------------------------------
// Dynamic Culling
#define WORLDX 64
#define WORLDZ WORLDX
#define WORLDY 18 // Level 8 is only 17.5 cells tall!!  Could be 16 if I make the ceiling same height in last room as in original.
#define TOTAL_WORLD_CELLS (WORLDX * WORLDY * WORLDZ)
#define ARRSIZE (WORLDX * WORLDZ)
#define WORLDCELL_WIDTH_F 2.56f
#define CELLXHALF (WORLDCELL_WIDTH_F * 0.5f)
#define LIGHT_RANGE_VOXEL_MANHATTAN_DIST (floorf(LIGHT_RANGE_MAX / VOXEL_WIDTH_F))
#define INVALID_LIGHT_INDEX (LIGHT_COUNT + 1)
#define PRECOMPUTED_VISIBILITY_SIZE 524288 // 4096 * 4096 / 32
#define VOXEL_COUNT 262144 // 64 * 64 * 8 * 8
#define VOXEL_SIZE 0.32f
#define VOXEL_HALF (VOXEL_SIZE * 0.5f)
#define CELL_SIZE 2.56f // Each cell is 2.56x2.56
#define MAX_LIGHTS_PER_VOXEL 24 // Cap to prevent overflow
#define CELL_VISIBLE       1u
#define CELL_OPEN          2u
#define CELL_CLOSEDNORTH   4u
#define CELL_CLOSEDEAST    8u
#define CELL_CLOSEDSOUTH  16u
#define CELL_CLOSEDWEST   32u
#define CELL_SEES_SUN     64u
#define CELL_SEES_SKYBOX 128u
extern uint16_t playerCellIdx, playerCellIdx_x, playerCellIdx_y, playerCellIdx_z;
extern uint16_t numCellsVisible;
extern uint8_t gridCellStates[ARRSIZE];
extern uint32_t precomputedVisibleCellsFromHere[524288];
extern float worldMin_x, worldMin_z, voxelMinCenterX, voxelMinCenterZ;
void CullInit(void);
void CullCore(void);
void Cull(void);
bool get_cull_bit(const uint32_t* arr, int idx);
// ----------------------------------------------------------------------------
// Physics
#define MAX_DYNAMIC_ENTITIES 512
#define TERMINAL_VELOCITY 10.0f
#define PHYS_FLOAT_TO_INT_SCALEF 100.0f
#define PLAYER_RADIUS 0.48f
#define PLAYER_HEIGHT 2.00f
#define PLAYER_CAM_OFFSET_Y 0.84f // Split capsule shape in the middle, camera is thus 0.16 away from top of the capsule ((2 / 2 = 1) - 0.84)
#define PHYS_COMBINE_AVG 0 // All the same for both frictionCombine and bounceCombine
#define PHYS_COMBINE_MIN 1
#define PHYS_COMBINE_MUL 2
#define PHYS_COMBINE_MAX 3
#define FORCEMODE_ACCUMULATE 0
#define FORCEMODE_IMPULSE 1
#define COLLIDER_TYPE_NONE 0
#define COLLIDER_TYPE_BOX 1
#define COLLIDER_TYPE_SPHERE 2
#define COLLIDER_TYPE_CAPSULE 3
#define COLLIDER_TYPE_CONVEXMESH 4
#define COLLIDER_TYPE_MESH 5
#define COLLIDER_CAPSULE_DIRECTION_X_F 0.0f // X-Axis
#define COLLIDER_CAPSULE_DIRECTION_Y_F 1.0f // Y-Axis
#define COLLIDER_CAPSULE_DIRECTION_Z_F 2.0f // Z-Axis
extern double time_PhysicsStep;
extern uint8_t boosterActive;
typedef uint8_t PhysicsLayer;
static const uint8_t PhysicsLayer_Default          = 0;
static const uint8_t PhysicsLayer_TransparentFX    = 1;
static const uint8_t PhysicsLayer_IgnoreRaycast    = 2;
//static const uint8_t PhysicsLayer_               = 3; // Layers direct copy from Unity version of Citadel, [sic] and sick
static const uint8_t PhysicsLayer_BlocksRaycast    = 4;
static const uint8_t PhysicsLayer_UI               = 5;
//static const uint8_t PhysicsLayer_               = 6;
//static const uint8_t PhysicsLayer_               = 7;
//static const uint8_t PhysicsLayer_               = 8;
static const uint8_t PhysicsLayer_Geometry         = 9;
static const uint8_t PhysicsLayer_NPC              = 10;
static const uint8_t PhysicsLayer_PlayerBullets    = 11;
static const uint8_t PhysicsLayer_Player           = 12;
static const uint8_t PhysicsLayer_Corpse           = 13;
static const uint8_t PhysicsLayer_PhysObjects      = 14;
static const uint8_t PhysicsLayer_Sky              = 15;
//static const uint8_t PhysicsLayer_               = 16;
//static const uint8_t PhysicsLayer_               = 17;
static const uint8_t PhysicsLayer_Door             = 18;
static const uint8_t PhysicsLayer_InterDebris      = 19;
static const uint8_t PhysicsLayer_Player2          = 20;
//static const uint8_t PhysicsLayer_               = 21;
//static const uint8_t PhysicsLayer_               = 22;
//static const uint8_t PhysicsLayer_               = 23;
static const uint8_t PhysicsLayer_NPCBullet        = 24;
static const uint8_t PhysicsLayer_NPCClip          = 25;
static const uint8_t PhysicsLayer_Clip             = 26;
//static const uint8_t PhysicsLayer_               = 27;
//static const uint8_t PhysicsLayer_               = 28;
static const uint8_t PhysicsLayer_CorpseSearchable = 29;
int32_t Physics(void);
void UpdateInstanceMatrix(int32_t i);
void AddForce(uint16_t idx, Vector3 force, bool isImpulse);
// ----------------------------------------------------------------------------
// Input
#define MAX_KEYS 512
#define MAX_MOUSE_BUTTONS 8

typedef struct {
    bool down;
    bool pressed;
    bool released;
} KeyState;

extern KeyState keyStates[MAX_KEYS];
extern KeyState mouseButtons[MAX_MOUSE_BUTTONS];
extern bool window_has_focus;
extern double last_mouse_x, last_mouse_y;
extern bool ignore_next_mouse_delta;
void CycleToNextMonitor(GLFWwindow* window);
void Input_Init(GLFWwindow* window);
void Input_MouselookApply(void);
int32_t Input_KeyDown(int32_t scancode);
int32_t Input_KeyUp(int32_t scancode);
int32_t Input_MouseMove(int32_t xrel, int32_t yrel);
// ----------------------------------------------------------------------------
// Rendering
#define DEBUG_OPENGL
#ifdef DEBUG_OPENGL
#define CHECK_GL_ERROR() do { GLenum err = glGetError(); if (err != GL_NO_ERROR) DualLogError("GL Error at %s:%d: %d\n", __FILE__, __LINE__, err); } while(0)
#else
#define CHECK_GL_ERROR() do {} while(0)
#endif
    
#define FAR_PLANE (71.68f) // Max player view, level 6 crawlway 28 cells
#define NEAR_PLANE (0.02f)
#define FAR_PLANE_SQUARED (FAR_PLANE * FAR_PLANE)
extern float testLight_x, testLight_y, testLight_z;
extern int32_t debugView;
extern int32_t debugValue;
extern float fogColorR, fogColorG, fogColorB, fogColorRUsed, fogColorGUsed, fogColorBUsed, fogBaseDensityForLevel;
void SetFog(void);
extern uint32_t drawCallsRenderedThisFrame;
extern uint32_t verticesRenderedThisFrame;
extern bool lightDirty[LIGHT_COUNT];
extern bool global_modIsCitadel;
extern bool inventoryMode;
#define CURSOR_SCREEN_PERCENTAGE 0.02f
extern int32_t cursorPosition_x, cursorPosition_y;
extern float cam_yaw, cam_pitch, cam_roll;
extern float cam_forwardx, cam_forwardy, cam_forwardz, cam_rightx, cam_righty, cam_rightz;
extern Quaternion cam_rotation;
void CacheUniformLocationsForShaders(void);
GLuint SetupSSBO(GLuint id, GLuint bindingIndex, GLsizeiptr size, const void* data, GLenum usage);
void Screenshot(void);
void ToggleConsole(void);
void ConsoleEmulator(int32_t keycode);
bool CursorVisible(void);
// ----------------------------------------------------------------------------
// Cheats
typedef struct {
	bool god;
	bool noclip;
	bool notarget;
	bool bottomless;
	bool superoverride;
	bool fatigueCheat;
	bool redbull;
	bool consoleActive;
	bool noHUD;
	bool showLocation;
	bool showFPS;
	bool editMode;
	uint8_t dizzyLevel;
} Voxen_Cheats;
extern Voxen_Cheats voxen_Cheats;

void SetSkyRotateSpeed(void);
// ----------------------------------------------------------------------------
// Text
#define TEXT_BUFFER_SIZE 1024
#define FONT_ATLAS_SIZE 3072
#define MAX_GLYPHS 639
#define FONT_NORMAL 0
#define FONT_STOPD 1
#define TEXT_STRING_COUNT 1100
#define TEXT_LOCALIZATION_MAX_LENGTH 1207
#define TEXT_LOGS_COUNT 134
#define TEXT_DATA_FILEBUFFER_SIZE 65536 // 16 pages
typedef struct {	
	uint8_t file_data[TEXT_DATA_FILEBUFFER_SIZE]; // Found that only 59430 were needed at one point, padded for safety and typo fixes
	char stringTable[TEXT_STRING_COUNT][TEXT_LOCALIZATION_MAX_LENGTH]; // Hefty table for localization support.
	uint16_t audioLogImagesRefIndicesLH[TEXT_LOGS_COUNT];
	uint16_t audioLogImagesRefIndicesRH[TEXT_LOGS_COUNT];
	uint8_t audioLogType[TEXT_LOGS_COUNT];
	uint8_t audioLogLevelFound[TEXT_LOGS_COUNT];
} Voxen_Text;
extern Voxen_Text voxen_Text;

extern char** audiologNames;
extern char** audiologSubjects;
extern char** audiologSenders;
extern char** audioLogSpeech2Text;
extern GLuint fontAtlasTex;
extern GLuint fontAtlasTexStopD;
extern float fixedNumberAdvanceWidth;
extern float fixedNumberAdvanceWidthStopD;
extern float genericTextHeightFacStopD;
extern float genericTextWidthFacStopD;
extern float genericTextHeightFac;
extern char consoleEntryText[TEXT_BUFFER_SIZE];
extern stbtt_packedchar fontPackedChar[MAX_GLYPHS];
extern stbtt_packedchar fontPackedCharStopD[MAX_GLYPHS];

void LoadTextForLanguage(uint8_t lang);
void LoadLogTextForLanguage(uint8_t lang);
int32_t CodepointToPackedIndex(int32_t codepoint, int32_t fontID);
float TextWidth(const char *utf8, int32_t fontID);
uint32_t DecodeUTF8(const char **p);
void InitFontAtlasses(void);
// ----------------------------------------------------------------------------
// UI
#define UI_LAYER_TOP 1.0f
#define UI_LAYER_5 0.5f
#define UI_LAYER_4 0.4f
#define UI_LAYER_3 0.3f
#define UI_LAYER_2 0.2f
#define UI_LAYER_1 0.1f
#define UI_LAYER_0 0.0f
float GetScreenRelativeX(float percentage);
float GetScreenRelativeY(float percentage);
// ----------------------------------------------------------------------------
// GAME LOGIC

// Patches
#define PATCH_TIME_BERSERK 30.0f

// ----------------------------------------------------------------------------
// Logging / Debug Prints
void OpenConsoleLogFile(void);
void Screenshot(void);
void CenterStatusPrint(const char* fmt, ...);
void JournalDump(const char* dem_file);
void DualLog(const char* fmt, ...);
void DualLogWarn(const char* fmt, ...);
void DualLogError(const char* fmt, ...);
void DebugRAM(const char *context);
void GetLevel_Transform_Offsets(int32_t curlevel, float* ofsx, float* ofsy, float* ofsz);
void GetLevel_LightsStaticImmutable_ContainerOffsets(int32_t curlevel, float* ofsx, float* ofsy, float* ofsz);
// ============================================================================
// ----------------------------------------------------------------------------
// Helper Functions
extern uint32_t random_range_rng;
double get_time(void);
void md5(const uint8_t *data, size_t len, uint8_t out[16]);
float clampf(float x, float a, float b);
uint32_t xs32(uint32_t *s);
uint8_t random_range_u8(uint8_t a, uint8_t b);
int data_parser_isspace(char c);
uint32_t parse_numberu32(const char* str, const char* line, uint32_t lineNum);
uint16_t parse_numberu16(const char* str, const char* line, uint32_t lineNum);
uint8_t parse_numberu8(const char* str, const char* line, uint32_t lineNum);
bool parse_bool(const char* str, const char* line, uint32_t lineNum);
float parse_float(const char* str, const char* line, uint32_t lineNum);

bool ConstIndexInBounds(int constdex);
bool ConstIndexIsGeometry(int constdex);
bool ConstIndexIsDynamicObject(uint16_t constIndex);
bool ConstIndexIsDoor(int constdex);
bool ConstIndexIsLightStaticSaveable(int constdex);
bool ConstIndexIsGenericTransform(int constdex);
bool ConstIndexIsStaticObjectImmutable(int constdex);
bool ConstIndexIsStaticObjectSaveable(int constdex);
bool ConstIndexIsNPC(int constdex);
bool ConstIndexIsHardware(int constdex);
bool ConstIndexIsAmbient(int constdex);
bool CursorVisible(void);
float LoadRelativeTimeDifferential(char* trimmed_value, char* initialLine, uint32_t lineNum);
static inline void CellCoordsToPos(uint16_t x, uint16_t z, float* pos_x, float* pos_z) {
    *pos_x = worldMin_x + (x * WORLDCELL_WIDTH_F);
    *pos_z = worldMin_z + (z * WORLDCELL_WIDTH_F);
}

static inline int32_t clamp(int32_t val, int32_t min, int32_t max) {
    if (val > max) return max;
    if (val < min) return min;
    return val;
}

static inline void PosToCellCoords(float pos_x, float pos_z, uint16_t* x, uint16_t* z) {
    int32_t max = WORLDX - 1; // 63
    int32_t xval = (int32_t)((pos_x - worldMin_x + CELLXHALF) / WORLDCELL_WIDTH_F);
    if (xval > max) xval = max;
    if (xval < 0) xval = 0;
    *x = (uint16_t)xval;
    
    int32_t zval = (int32_t)((pos_z - worldMin_z + CELLXHALF) / WORLDCELL_WIDTH_F);
    if (zval > max) zval = max;
    if (zval < 0) zval = 0;
    *z = (uint16_t)zval;
}

static inline bool XZPairInBounds(int32_t x, int32_t z) {
    return (x < WORLDX && z < WORLDZ && x >= 0 && z >= 0);
}

static inline void flag_enable(uint32_t *flags, uint32_t bit) {
    *flags |= bit;
}

static inline void flag_disable(uint32_t *flags, uint32_t bit) {
    *flags &= ~bit;
}

static inline void flag_set(uint32_t *flags, uint32_t bit, bool state) {
    *flags = (*flags & ~bit) | (-state & bit);
}

// static inline void sanitize_utf8_ascii(char *s) {
//     char *dst = s;
//     while (*s) {
//         if (!memcmp(s, "\xE2\x80\x90", 3) || !memcmp(s, "\xE2\x80\x91", 3) ||
//             !memcmp(s, "\xE2\x80\x92", 3) || !memcmp(s, "\xE2\x80\x93", 3) ||
//             !memcmp(s, "\xE2\x80\x94", 3) || !memcmp(s, "\xE2\x80\x95", 3) ||  // Added: Horizontal bar
//             !memcmp(s, "\xE2\x88\x92", 3)) {
//             dst[0] = '-'; dst++; s += 3; continue;
//         }
//         if (!memcmp(s, "\xC2\xAD", 2)) { dst[0] = '-'; dst++; s += 2; continue; }
//         if (!memcmp(s, "\xE2\x80\x9C", 3) || !memcmp(s, "\xE2\x80\x9D", 3)) { dst[0] = '"'; dst++; s += 3; continue; }
//         if (!memcmp(s, "\xE2\x80\x98", 3) || !memcmp(s, "\xE2\x80\x99", 3)) { dst[0] = '\''; dst++; s += 3; continue; }
//         if (!memcmp(s, "\xEF\xBC\x8B", 3)) { dst[0] = '+'; dst++; s += 3; continue; }
//         if (!memcmp(s, "\xEF\xBC\x8F", 3)) { dst[0] = '/'; dst++; s += 3; continue; }
//         if (!memcmp(s, "\xEF\xBC\x88", 3)) { dst[0] = '('; dst++; s += 3; continue; }
//         if (!memcmp(s, "\xEF\xBC\x89", 3)) { dst[0] = ')'; dst++; s += 3; continue; }
//         if (!memcmp(s, "\xEF\xBC\x9A", 3)) { dst[0] = ':'; dst++; s += 3; continue; }
//         if (!memcmp(s, "\xEF\xBC\x9B", 3)) { dst[0] = ';'; dst++; s += 3; continue; }
//         if (!memcmp(s, "\xEF\xBC\x8C", 3)) { dst[0] = ','; dst++; s += 3; continue; }
//         if (!memcmp(s, "\xEF\xBC\x8E", 3)) { dst[0] = '.'; dst++; s += 3; continue; }
//         if (!memcmp(s, "\xEF\xBC\x8D", 3)) { dst[0] = '-'; dst++; s += 3; continue; }
//         dst[0] = *s; dst++; s++;
//     }
//     *dst = '\0';
// }
