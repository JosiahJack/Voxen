#ifndef VOXEN_HEADER_H
#define VOXEN_HEADER_H
#define VERSION_STRING "v0.7.2"
// #define DEBUG_RAM_OUTPUT // Debug and Compile Flags
// #define DEBUG_MODEL_LOAD_DATA 1U

// Generic Lib Includes
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "External/stb_truetype.h"
#include "citadel_enumerations.h"

// Generic Constants
#define M_PI 3.141592653f
#define MAX_PATH 128

// Global Types
typedef struct { float x,y; } Vector2;
typedef struct { float x,y,z; } Vector3;
typedef struct { float x,y,z,w; } Quaternion;
typedef struct { float r,g,b,a; } Color;

#define NULLENT 0
#define PLAYER1 1
#define PLAYER2 2
#define START_INDEX_LEVEL_INSTANCES 3
#define ENTFLAG_ACTIVE 1
#define ENTFLAG_CARDCHUNK 2
#define ENTFLAG_GROUNDED 4
#define ENTFLAG_USEGRAVITY 8

typedef struct {
    Vector3 position;
    Quaternion rotation;
    Vector3 scale;
    Vector3 velocity;
    uint32_t entflags;
    uint16_t modelIndex;
    uint16_t texIndex;
    uint16_t glowIndex;
    uint16_t specIndex;
    uint16_t normIndex;
    uint16_t lodIndex;
    uint16_t index; // constIndex for entity type, used for indexing into arrays for resourec types when loading resources
    BodyState bodyState;
    float volume;
    
    uint16_t   child0;
    Vector3    child0_offset;
    Quaternion child0_rotation;
    Vector3    child0_scale;
    
    uint16_t   child1;
    Vector3    child1_offset;
    Quaternion child1_rotation;
    Vector3    child1_scale;
} Entity;

typedef struct {
    Vector3 mins;
    Vector3 maxs;
    uint8_t type;
} Trigger;

typedef struct {
    uint16_t index;
    uint16_t modelIndex;
    uint16_t lodIndex;
    uint16_t texIndex;
    uint16_t glowIndex;
    uint16_t specIndex;
    uint16_t normIndex;
    bool doublesided;
    bool transparent;
    bool cardchunk;
    float volume;
    
    uint16_t   child0;
    Vector3    child0_offset;
    Quaternion child0_rotation;
    Vector3    child0_scale;
    
    uint16_t   child1;
    Vector3    child1_offset;
    Quaternion child1_rotation;
    Vector3    child1_scale;
    
    char path[MAX_PATH];
} ResourceEntry;

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
    ResourceEntry* entries;
    int32_t count;
    int32_t capacity;
} DataParser;

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
// Data Parsing
#define MAX_ENTRIES 6000
void ParseGameData();
bool parse_data_file(DataParser *parser, const char *filename);

// Textures
#define MAX_TEXTURE_DIMENSION 2048
#define MAX_PALETTE_SIZE 256
#define MATERIAL_IDX_MAX 2048 // Max value the bit packing bits allow
#define BLACK_TEXTURE_IDX 41
extern bool* doubleSidedTexture;
extern bool* transparentTexture;
bool isDoubleSided(uint32_t texIndexToCheck);
bool isTransparent(uint32_t texIndexToCheck);
void LoadTextures(void);

// Models
#define MODEL_COUNT 680
#define MODEL_IDX_MAX 1024 // Max value the bit packing bits allow
#define MAX_VERT_COUNT 40000
#define MAX_TRI_COUNT 32768
#define VERTEX_ATTRIBUTES_COUNT 8 // x,y,z,nx,ny,nz,u,v
extern uint32_t* modelVertexCounts;
extern uint32_t* modelTriangleCounts;
extern float** modelVertices;
extern uint32_t** modelTriangles;

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
extern uint16_t renderableCount;
extern uint16_t loadedInstances;
extern uint16_t loadedTextures;
extern uint16_t loadedModels;
extern uint16_t loadedLights;
extern uint16_t gameObjectCount;
void LoadModels(void);

// Entities
#define MAX_ENTITIES 768 // Unique entity types, different than INSTANCE_COUNT which is the number of instances of any of these entities.
#define INSTANCE_COUNT 10000 // Max 5454 for Citadel level 7 geometry, Max 295 for Citadel level 1 dynamic objects, 1561 lights, extras for dynamically spawned objects/lights
#define GEOMETRY_LOD_CARD_MODEL_IDX 178
extern Entity entities[MAX_ENTITIES];
extern Entity instances[INSTANCE_COUNT];
extern uint16_t* modelTypeCountsOpaque;
extern uint16_t* modelTypeCountsDoubleSided;
extern uint16_t* modelTypeCountsTransparent;
extern uint16_t invalidModelIndexCount;
extern uint16_t* modelTypeOffsetsOpaque;
extern uint16_t* modelTypeOffsetsDoubleSided;
extern uint16_t* modelTypeOffsetsTransparent;
extern uint16_t opaqueInstancesHead;
extern uint16_t doubleSidedInstancesHead;
extern uint16_t transparentInstancesHead;
extern float modelMatrices[INSTANCE_COUNT * 16];
extern uint8_t dirtyInstances[INSTANCE_COUNT];
extern uint16_t startOfDoubleSidedInstances;
extern uint16_t startOfTransparentInstances;
int32_t SetupInstances(void);
void LoadEntities(void);

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
#define SHADOW_MAP_SIZE 128u
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
void LoadLevel(uint8_t curlevel);
void SortInstances();
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
extern uint32_t cellIndexForInstance[INSTANCE_COUNT];
extern uint16_t cellIndexForLight[LIGHT_COUNT];
extern uint16_t cellIndexForLightX[LIGHT_COUNT];
extern uint16_t cellIndexForLightZ[LIGHT_COUNT];
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
extern double time_PhysicsStep;
extern uint16_t physHead;
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
int32_t ParticleSystemStep(void);
int32_t Physics(void);
void UpdateInstanceMatrix(int32_t i);
void quat_to_matrix(Quaternion* q, float* m);
// ----------------------------------------------------------------------------
// Input
#define NUM_KEYS 350
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
extern bool noclip;
extern bool consoleActive;
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
bool CursorVisible(void);
float dot(float x1, float y1, float z1, float x2, float y2, float z2);
// ----------------------------------------------------------------------------
// Text
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
#define TEXT_BUFFER_SIZE 1024
#define FONT_ATLAS_SIZE 4096
#define MAX_GLYPHS 8192      // Rough estimate for all ranges
#define FONT_NORMAL 0
#define FONT_STOPD 1
extern GLuint fontAtlasTex;
extern GLuint fontAtlasTexStopD;
extern float fixedNumberAdvanceWidth;
extern float fixedNumberAdvanceWidthStopD;
extern float genericTextHeightFacStopD;
extern float genericTextWidthFacStopD;
extern float genericTextHeightFac;
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
void ConsoleEmulator(int32_t scancode);
void CenterStatusPrint(const char* fmt, ...);
void JournalDump(const char* dem_file);
void DualLog(const char* fmt, ...);
void DualLogWarn(const char* fmt, ...);
void DualLogError(const char* fmt, ...);
void DebugRAM(const char *context);
void GetLevel_Transform_Offsets(int32_t curlevel, float* ofsx, float* ofsy, float* ofsz);
void GetLevel_LightsStaticImmutable_ContainerOffsets(int32_t curlevel, float* ofsx, float* ofsy, float* ofsz);
void DualLogEntity(uint16_t idx);
// ============================================================================
// ----------------------------------------------------------------------------
// Helper Functions
double get_time(void);
void md5(const uint8_t *data, size_t len, uint8_t out[16]);
bool ConstIndexInBounds(int constdex);
bool ConstIndexIsGeometry(int constdex);
bool ConstIndexIsDynamicObject(uint16_t constIndex);
bool ConstIndexIsDoor(int constdex);
bool ConstIndexIsLightStaticSaveable(int constdex);
bool ConstIndexIsGenericTransform(int constdex);
bool ConstIndexIsDynamicObject(uint16_t constIndex);
bool ConstIndexIsStaticObjectImmutable(int constdex);
bool ConstIndexIsNPC(int constdex);
bool ConstIndexIsHardware(int constdex);
bool ConstIndexIsAmbient(int constdex);
void Screenshot();
bool CursorVisible(void);
static inline float deg2rad(float degrees) { return degrees * (M_PI / 180.0f); }
static inline float rad2deg(float radians) { return radians * (180.0f / M_PI); }
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

float random() { return ((float)rand() / RAND_MAX); }
float crandom() { return 2.0f * (random() - 0.5f); }

void GetLevel_Transform_Offsets(int32_t curlevel, float* ofsx, float* ofsy, float* ofsz) {
    if (!global_modIsCitadel) { *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f;  return; } // TODO: Resave levels with the offsets applied.
    
    switch(curlevel) { // Match the parent transforms #.NAMELevel, e.g. 1.MedicalLevel
        case 0:  *ofsx = 3.6f; *ofsy = -4.10195f; *ofsz = 1.0f; break;
        case 1:  *ofsx = -5.12f; *ofsy = -48.64f; *ofsz = -15.36f; break;
        case 2:  *ofsx = -2.6f; *ofsy = 0.0f; *ofsz = -7.7f; break;
        case 3:  *ofsx = -45.12f; *ofsy = -0.700374f; *ofsz = -16.32f; break;
        case 4:  *ofsx = -20.4f; *ofsy = 0.0f; *ofsz = 11.48f; break;
        case 5:  *ofsx = -10.14f; *ofsy = 0.065f; *ofsz = -0.0383f; break;
        case 6:  *ofsx = -0.6728f; *ofsy = 0.1725f; *ofsz = 3.76f; break;
        case 7: *ofsx = -6.7f; *ofsy = 0.24443f; *ofsz = 1.16f; break;
        case 8:  *ofsx = 1.08f; *ofsy = -0.935f; *ofsz = 0.8f; break;
        case 9:  *ofsx = 3.6f; *ofsy = 0.0f; *ofsz = -1.28f; break;
        case 10: *ofsx = 107.37f; *ofsy = 101.2f; *ofsz = 35.48f; break;
        case 11: *ofsx = 15.05f; *ofsy = 129.9f; *ofsz = -77.94f; break;
        case 12:  *ofsx = 19.04f; *ofsy = 162.2f; *ofsz = 95.8f; break;
        case LEVEL_CYBERSPACE: *ofsx = 164.7f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

void GetLevel_LightsStaticSaveable_ContainerOffsets(int32_t curlevel, float* ofsx, float* ofsy, float* ofsz) {
    if (!global_modIsCitadel) { *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f;  return; }

    switch(curlevel) { // Match the parent transforms #.NAMELevel, e.g. 1.LightsStaticSaveable
        case 0:  *ofsx = -1.2417f; *ofsy = -0.26194f; *ofsz = -1.0883f; break;
        case 1:  *ofsx = 0.589f; *ofsy = -0.554f; *ofsz = -0.907f; break;
        case 2:  *ofsx = -0.98611f; *ofsy = 0.82105f; *ofsz = 1.1906f; break;
        case 3:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 4:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 5:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 6:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 7:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 8:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 9:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 10: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 11: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 12: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case LEVEL_CYBERSPACE: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

void GetLevel_LightsStaticImmutable_ContainerOffsets(int32_t curlevel, float* ofsx, float* ofsy, float* ofsz) {
    if (!global_modIsCitadel) { *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f;  return; }

    switch(curlevel) { // Match the parent transforms #.NAMELevel, e.g. 1.LightsStaticImmutable
        case 0:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 1:  *ofsx = -5.12f; *ofsy = -48.37571f; *ofsz = -15.391001f; break;
        case 2:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 3:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 4:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 5:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 6:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 7:  *ofsx = -14.528f; *ofsy = 48.269f; *ofsz = -26.836f; break;
        case 8:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 9:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 10: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 11: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 12: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case LEVEL_CYBERSPACE: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

void GetLevel_DoorsStaticSaveable_ContainerOffsets(int32_t curlevel, float* ofsx, float* ofsy, float* ofsz) {
    if (!global_modIsCitadel) { *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f;  return; }

    switch(curlevel) { // Match the parent transforms #.NAMELevel, e.g. 1.DoorsStaticSaveable
        case 0:  *ofsx = -1.2417f; *ofsy = -0.26194f; *ofsz = -1.0883f; break;
        case 1:  *ofsx = 0.589f; *ofsy = -0.554f; *ofsz = -0.907f; break;
        case 2:  *ofsx = -0.98611f; *ofsy = 0.82105f; *ofsz = 1.1906f; break;
        case 3:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 4:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 5:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 6:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 7:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 8:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 9:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 10: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 11: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 12: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case LEVEL_CYBERSPACE: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

void GetLevel_StaticObjectsSaveable_ContainerOffsets(int32_t curlevel, float* ofsx, float* ofsy, float* ofsz) {
    if (!global_modIsCitadel) { *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f;  return; }

    switch(curlevel) { // Match the parent transforms #.NAMELevel, e.g. 1.StaticObjectsSaveable
        case 0:  *ofsx = -1.2417f; *ofsy = -0.26194f; *ofsz = -1.0883f; break;
        case 1:  *ofsx = 0.589f; *ofsy = -0.554f; *ofsz = -0.907f; break;
        case 2:  *ofsx = -0.98611f; *ofsy = 0.82105f; *ofsz = 1.1906f; break;
        case 3:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 4:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 5:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 6:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 7:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 8:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 9:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 10: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 11: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 12: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case LEVEL_CYBERSPACE: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

void GetLevel_StaticObjectsImmutable_ContainerOffsets(int32_t curlevel, float* ofsx, float* ofsy, float* ofsz) {
    if (!global_modIsCitadel) { *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f;  return; }

    switch(curlevel) { // Match the parent transforms #.NAMELevel, e.g. 1.StaticObjectsImmutable
        case 0:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 1:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 2:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 3:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 4:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 5:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 6:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 7:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 8:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 9:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 10: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 11: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 12: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case LEVEL_CYBERSPACE: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

void GetLevel_NPCsSaveableInstantiated_ContainerOffsets(int32_t curlevel, float* ofsx, float* ofsy, float* ofsz) {
    if (!global_modIsCitadel) { *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f;  return; }

    switch(curlevel) { // Match the parent transforms #.NAMELevel, e.g. 1.NPCsSaveableInstantiated
        case 0:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 1:  *ofsx = -33.28f; *ofsy = 48.64f; *ofsz = 7.679996f; break;
        case 2:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 3:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 4:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 5:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 6:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 7:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 8:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 9:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 10: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 11: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 12: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case LEVEL_CYBERSPACE: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}
// ============================================================================
#endif // VOXEN_ENGINE_IMPLEMENTATION
// ----------------------------------------------------------------------------
#endif // VOXEN_HEADER_H
