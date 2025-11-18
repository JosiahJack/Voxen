#ifndef VOXEN_HEADER_H
#define VOXEN_HEADER_H
#define VERSION_STRING "v0.7.4"
// #define DEBUG_RAM_OUTPUT // Debug and Compile Flags

// Generic Lib Includes
#include <time.h>
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

// ----------------------------------------------------------------------------
// Settings
extern uint8_t settings_CullEnabled;
// ----------------------------------------------------------------------------
// Audio
#define MAX_AMBIENT_NOISES 32
extern uint16_t loadedAmbients;
extern uint16_t ambientRegistry[MAX_AMBIENT_NOISES];
void play_mp3(const char* path, float volume, int32_t fade_in_ms);
void play_wav(const char* path, float volume);
void InitializeAudio();
void UpdateAmbientSounds(void);
// ----------------------------------------------------------------------------
// Textures
#define MAX_TEXTURE_DIMENSION 2048
#define MAX_PALETTE_SIZE 256
#define MATERIAL_IDX_MAX 2048 // Max value the bit packing bits allow
#define BLACK_TEXTURE_IDX 41 // TODO: Make settable from gamedata.txt
extern bool* doubleSidedTexture;
extern bool* transparentTexture;
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
extern float* modelBounds;

extern GLuint* vbos;
extern GLuint* tbos;
extern uint16_t loadedTextures;
extern uint16_t loadedModels;
extern uint16_t loadedLights;
extern uint16_t gameObjectCount;
extern uint32_t* modelVertexCounts;
extern uint32_t* modelTriangleCounts;
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
#define MAX_VISIBLE_LIGHTS 90
#define SHADOW_MAP_SIZE 64u
#define MAX_SHADOWMAPS 80u
#define SHADOWMAP_FOV 90.0f

extern float lights[LIGHT_COUNT * LIGHT_DATA_SIZE];
extern float lightsRangeSquared[LIGHT_COUNT];
extern bool lightIsDynamic[LIGHT_COUNT];

// Levels / Game Management
#define LEVEL_CYBERSPACE 13
extern char global_modname[256];
extern uint8_t startLevel;
extern uint8_t numLevels; // Can be set by gamedata.txt
extern uint8_t currentLevel;
extern bool gamePaused;
extern bool menuActive;
extern bool levelCurrentlyLoading;

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
#define MAX_SQUARE_DIST_INT 268435456 // (64 * 256)^2 = max square dist
#define MAX_LIGHTS_PER_VOXEL 24 // Cap to prevent overflow
#define CELL_VISIBLE       1
#define CELL_OPEN          2
#define CELL_CLOSEDNORTH   4
#define CELL_CLOSEDEAST    8
#define CELL_CLOSEDSOUTH  16
#define CELL_CLOSEDWEST   32
#define CELL_SEES_SUN     64
#define CELL_SEES_SKYBOX 128
extern uint16_t numberOfFOVConeChecks0; // 5049
extern uint16_t numberOfFOVConeChecks1; // 5030
extern uint16_t numberOfFOVConeChecks2; // 2687
extern uint16_t numberOfFOVConeChecks3; // 2687
extern uint16_t playerCellIdx, playerCellIdx_x, playerCellIdx_y, playerCellIdx_z;
extern uint16_t numCellsVisible;
extern uint8_t gridCellStates[ARRSIZE];
extern uint32_t precomputedVisibleCellsFromHere[524288];
extern float worldMin_x, worldMin_z, voxelMinCenterX, voxelMinCenterZ;
void CullInit(void);
void CullCore(void);
void Cull();
bool get_cull_bit(const uint32_t* arr, size_t idx);
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
#define COLLIDER_TYPE_NONE 0
#define COLLIDER_TYPE_BOX 1
#define COLLIDER_TYPE_SPHERE 2
#define COLLIDER_TYPE_CAPSULE 3
#define COLLIDER_TYPE_CONVEXMESH 4
#define COLLIDER_TYPE_MESH 5
#define COLLIDER_CAPSULE_DIRECTION_X_F 0.0f // X-Axis
#define COLLIDER_CAPSULE_DIRECTION_Y_F 1.0f // Y-Axis
#define COLLIDER_CAPSULE_DIRECTION_Z_F 2.0f // Z-Axis
// TODO: Ensure that npc corpses get dynamicFriction of 10.0f, staticFriction of 10.0f, bounciness of 0.0f, frictionCombine of 2, bounceCombine of 3
extern double time_PhysicsStep;
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
extern float move_speed;
int32_t Physics(void);
void UpdateInstanceMatrix(int32_t i);
// ----------------------------------------------------------------------------
// Input
#define NUM_KEYS 350
extern bool editMode;
extern GLFWwindow *window;
extern bool keys[NUM_KEYS];
extern bool window_has_focus;
extern double last_mouse_x, last_mouse_y;
extern bool ignore_next_mouse_delta;
void CycleToNextMonitor(GLFWwindow* window);
void Input_Init(GLFWwindow* window);
void Input_MouselookApply();
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
extern uint16_t screen_width;
extern uint16_t screen_height;
extern int32_t debugView;
extern int32_t debugValue;
extern float fogColorR, fogColorG, fogColorB, fogColorRUsed, fogColorGUsed, fogColorBUsed, fogBaseDensityForLevel;
void SetFog();
extern uint32_t drawCallsRenderedThisFrame;
extern uint32_t verticesRenderedThisFrame;
extern bool lightDirty[LIGHT_COUNT];
extern bool global_modIsCitadel;
extern bool inventoryMode;
#define CURSOR_SCREEN_PERCENTAGE 0.02f
extern int32_t cursorPosition_x, cursorPosition_y;
extern float cam_yaw, cam_pitch, cam_roll, cam_fov;
extern float cam_forwardx, cam_forwardy, cam_forwardz, cam_rightx, cam_righty, cam_rightz;
extern Quaternion cam_rotation;
extern GLuint chunkShaderProgram;
extern GLuint imageBlitShaderProgram;
extern GLint debugViewLoc_quadblit, debugValueLoc_quadblit;
extern GLint debugViewLoc_chunk, debugValueLoc_chunk;
void CacheUniformLocationsForShaders(void);
GLuint SetupSSBO(GLuint id, GLuint bindingIndex, GLsizeiptr size, const void* data, GLenum usage);
void Screenshot(void);
void ToggleConsole(void);
void ConsoleEmulator(int32_t keycode);
bool CursorVisible(void);
// ----------------------------------------------------------------------------
// Cheats
extern bool god;
extern bool noclip;
extern bool notarget;
extern bool bottomless;
extern bool superoverride;
extern bool fatigueCheat;
extern bool redbull;
extern bool consoleActive;
extern bool noHUD;
// ----------------------------------------------------------------------------
// Text
#define TEXT_BUFFER_SIZE 1024
#define FONT_ATLAS_SIZE 4096
#define MAX_GLYPHS 8192      // Rough estimate for all ranges
#define FONT_NORMAL 0
#define FONT_STOPD 1

extern char** stringTable;
extern uint16_t* audioLogImagesRefIndicesLH;
extern uint16_t* audioLogImagesRefIndicesRH;
extern char** audiologNames;
extern char** audiologSubjects;
extern char** audiologSenders;
extern char** audioLogSpeech2Text;
extern uint8_t* audioLogType;
extern uint16_t* audioLogLevelFound;

void LoadTextForLanguage(uint8_t lang);
void LoadLogTextForLanguage(uint8_t lang);
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
int32_t CodepointToPackedIndex(int32_t codepoint, int fontID);
float TextWidth(const char *utf8, int fontID);
uint32_t DecodeUTF8(const char **p);
void InitFontAtlasses();
// ----------------------------------------------------------------------------
// UI
#define BTN_SHOOT_MODE 10
#define UI_LAYER_TOP 1.0f
#define UI_LAYER_5 0.5f
#define UI_LAYER_4 0.4f
#define UI_LAYER_3 0.3f
#define UI_LAYER_2 0.2f
#define UI_LAYER_1 0.1f
#define UI_LAYER_0 0.0f
extern float uiOrthoProjection[16];
float GetScreenRelativeX(float percentage);
float GetScreenRelativeY(float percentage);
// ----------------------------------------------------------------------------
// GAME LOGIC

// Patches
#define PATCH_TIME_BERSERK 30.0f

// ----------------------------------------------------------------------------
// Logging / Debug Prints
void OpenConsoleLogFile();
void Screenshot();
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
void Screenshot();
bool CursorVisible(void);
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

inline float squareDistance2D(float x1, float z1, float x2, float z2) {
    float dx = x2 - x1;
    float dz = z2 - z1;
    return dx * dx + dz * dz;
}

inline float squareDistance3D(float x1, float y1, float z1, float x2, float y2, float z2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float dz = z2 - z1;
    return dx * dx + dy * dy + dz * dz;
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

static inline void sanitize_utf8_ascii(char *s) {
    char *dst = s;
    while (*s) {
        if (!memcmp(s, "\xE2\x80\x90", 3) || !memcmp(s, "\xE2\x80\x91", 3) ||
            !memcmp(s, "\xE2\x80\x92", 3) || !memcmp(s, "\xE2\x80\x93", 3) ||
            !memcmp(s, "\xE2\x80\x94", 3) || !memcmp(s, "\xE2\x80\x95", 3) ||  // Added: Horizontal bar
            !memcmp(s, "\xE2\x88\x92", 3)) {
            dst[0] = '-'; dst++; s += 3; continue;
        }
        if (!memcmp(s, "\xC2\xAD", 2)) { dst[0] = '-'; dst++; s += 2; continue; }
        if (!memcmp(s, "\xE2\x80\x9C", 3) || !memcmp(s, "\xE2\x80\x9D", 3)) { dst[0] = '"'; dst++; s += 3; continue; }
        if (!memcmp(s, "\xE2\x80\x98", 3) || !memcmp(s, "\xE2\x80\x99", 3)) { dst[0] = '\''; dst++; s += 3; continue; }
        if (!memcmp(s, "\xEF\xBC\x8B", 3)) { dst[0] = '+'; dst++; s += 3; continue; }
        if (!memcmp(s, "\xEF\xBC\x8F", 3)) { dst[0] = '/'; dst++; s += 3; continue; }
        if (!memcmp(s, "\xEF\xBC\x88", 3)) { dst[0] = '('; dst++; s += 3; continue; }
        if (!memcmp(s, "\xEF\xBC\x89", 3)) { dst[0] = ')'; dst++; s += 3; continue; }
        if (!memcmp(s, "\xEF\xBC\x9A", 3)) { dst[0] = ':'; dst++; s += 3; continue; }
        if (!memcmp(s, "\xEF\xBC\x9B", 3)) { dst[0] = ';'; dst++; s += 3; continue; }
        if (!memcmp(s, "\xEF\xBC\x8C", 3)) { dst[0] = ','; dst++; s += 3; continue; }
        if (!memcmp(s, "\xEF\xBC\x8E", 3)) { dst[0] = '.'; dst++; s += 3; continue; }
        if (!memcmp(s, "\xEF\xBC\x8D", 3)) { dst[0] = '-'; dst++; s += 3; continue; }
        dst[0] = *s; dst++; s++;
    }
    *dst = '\0';
}

// // // // //
#ifdef VOXEN_ENGINE_IMPLEMENTATION // -----------------------------<<<
// // // // // 
double get_time(void) {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1) {
        DualLogError("clock_gettime failed\n");
        return 0.0;
    }

    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9; // Full time in seconds
}

// Get USS aka the total RAM uniquely allocated for the process (btop shows RSS so pulls in shared libs and double counts shared RAM).
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void DebugRAM(const char *context) {
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
    } else DualLogError("Failed to open /proc/self/smaps_rollup\n");

    DualLog("Memory at %s: Heap usage %zu bytes (%zu KB | %.2f MB), USS %zu bytes (%zu KB | %.2f MB)\n",
            context, info.uordblks, info.uordblks / 1024, info.uordblks / 1024.0 / 1024.0,
            uss_bytes, uss_bytes / 1024, uss_bytes / 1024.0 / 1024.0);
#endif
}
#pragma GCC diagnostic pop

void print_bytes_no_newline(int32_t count) { DualLog("%d bytes | %f kb | %f Mb",count,(float)count / 1000.0f,(float)count / 1000000.0f); }

static inline float random() { return ((float)rand() / RAND_MAX); }
static inline float crandom() { return 2.0f * (random() - 0.5f); }
// ============================================================================
#endif // VOXEN_ENGINE_IMPLEMENTATION
// ----------------------------------------------------------------------------
#endif // VOXEN_HEADER_H
