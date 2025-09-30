#ifndef VOXEN_HEADER_H
#define VOXEN_HEADER_H

// Debug and Compile Flags
// #define DEBUG_RAM_OUTPUT
// #define DEBUG_TEXTURE_LOAD_DATA 1
// #define DEBUG_MODEL_LOAD_DATA 1U

// Generic Constants
#define M_PI 3.141592653f
#define M_PI_2 1.57079632679489661923f
#define MATH_EPSILON 0.00001f

// Global Types
typedef struct { float x,y; } Vector2;
typedef struct { float x,y,z; } Vector3;
typedef struct { float x,y,z,w; } Quaternion;
typedef struct { float r,g,b,a; } Color;

// Generic Lib Includes TODO REDUCE AS MUCH AS POSSIBLE!!
#include <stdint.h>
#include <stdbool.h>
#include <GL/glew.h>
#include <SDL2/SDL.h>
// #include <fluidlite.h> TODO Add midi support
// #include <libxmi.h>

// ----------------------------------------------------------------------------
// Audio
#include "./External/miniaudio.h"
#define MAX_CHANNELS 64
extern ma_engine audio_engine;
// extern fluid_synth_t* midi_synth; TODO Add midi support
extern ma_sound mp3_sounds[2]; // For crossfading
extern ma_sound wav_sounds[MAX_CHANNELS];
extern int32_t wav_count;
// int32_t InitializeAudio(const char* soundfont_path); TODO Add midi support
int32_t InitializeAudio();
// void play_midi(const char* midi_path); TODO Add midi support
void play_mp3(const char* path, float volume, int32_t fade_in_ms);
void play_wav(const char* path, float volume);
void CleanupAudio();
// ----------------------------------------------------------------------------
// Data Parsing
#define MAX_ENTRIES 6000
#define MAX_PATH 128

// Ordered with name last since it is accessed infrequently so doesn't need to hit cache much.
typedef struct {
    Vector3 position;
    Quaternion rotation;
    Vector3 scale;
    Vector3 velocity;
    Vector3 angularVelocity;
    Color color;
    float intensity;
    float range;
    float spotAngle;
    uint16_t modelIndex;
    uint16_t texIndex;
    uint16_t glowIndex;
    uint16_t specIndex;
    uint16_t normIndex;
    uint16_t lodIndex;
    uint16_t index;
    bool active;
    bool solid;
    bool cardchunk;
    bool doublesided; // Parsing only, TODO Remove
    bool transparent; // Parsing only, TODO Remove
    uint8_t type; // Parsing only, TODO Remove
    uint8_t saveableType; // Parsing only, TODO Remove
    uint8_t metadata; // padding, TODO fix!
    char path[MAX_PATH]; // Parsing only, TODO Remove
} Entity;
// Includes subfields for parsing from text files, e.g. x,y,z need to parse "position.x" one "field":
#define ENTITY_FIELD_COUNT 41

typedef struct {
    Vector3 mins;
    Vector3 maxs;
    uint8_t type;
} Trigger;

typedef struct {
    Entity* entries;
    int32_t count;        // Added to track valid entries
    int32_t capacity;
} DataParser;

void init_data_entry(Entity *entry);
bool read_token(FILE *file, char *token, size_t max_len, char delimiter, bool *is_comment, bool *is_eof, bool *is_newline, uint32_t *lineNum);
bool process_key_value(Entity *entry, const char *key, const char *value, const char *line, uint32_t lineNum);
void parser_init(DataParser *parser);
bool parse_data_file(DataParser *parser, const char *filename, int type);

// Textures
#define MAX_TEXTURE_COUNT 2048
#define MAX_TEXTURE_DIMENSION 2048
#define MAX_PALETTE_SIZE 9000
#define MATERIAL_IDX_MAX 2048 // Max value the bit packing bits allow
extern GLuint colorBufferID;
extern uint16_t textureCount;
bool isDoubleSided(uint32_t texIndexToCheck);
bool isTransparent(uint32_t texIndexToCheck);
int32_t LoadTextures(void);

// Models
#define MODEL_COUNT 680
#define MODEL_IDX_MAX 1024 // Max value the bit packing bits allow
#define MAX_VERT_COUNT 40000
#define MAX_TRI_COUNT 32768
#define VERTEX_ATTRIBUTES_COUNT 8 // x,y,z,nx,ny,nz,u,v
extern uint32_t modelVertexCounts[MODEL_COUNT];
extern uint32_t modelTriangleCounts[MODEL_COUNT];
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
extern GLuint modelBoundsID;
extern float modelBounds[MODEL_COUNT * BOUNDS_ATTRIBUTES_COUNT];

extern GLuint vbos[MODEL_COUNT];
extern GLuint tbos[MODEL_COUNT];
extern uint32_t renderableCount;
extern uint32_t loadedInstances;
extern int32_t gameObjectCount;
int32_t LoadModels(void);

// Entities
#define MAX_ENTITIES 768 // Unique entity types, different than INSTANCE_COUNT which is the number of instances of any of these entities.
#define INSTANCE_COUNT 7400 // Max 5454 for Citadel level 7 geometry, Max 295 for Citadel level 1 dynamic objects, 1561 lights, extras for dynamically spawned objects/lights
extern Entity entities[MAX_ENTITIES];
extern Entity instances[INSTANCE_COUNT];
extern float modelMatrices[INSTANCE_COUNT * 16];
extern uint8_t dirtyInstances[INSTANCE_COUNT];
extern GLuint instancesBuffer;
extern GLuint matricesBuffer;
extern int32_t startOfDoubleSidedInstances;
extern int32_t startOfTransparentInstances;
int32_t SetupInstances(void);
int32_t LoadEntities(void);

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
#define LIGHT_MAX_INTENSITY 8.0f
#define LIGHT_RANGE_MAX 15.36f
#define LIGHT_RANGE_MAX_SQUARED (LIGHT_RANGE_MAX * LIGHT_RANGE_MAX)
#define MAX_VISIBLE_LIGHTS 90
#define SHADOW_MAP_SIZE 256u
#define SHADOWMAP_FOV 90.0f

extern float lights[LIGHT_COUNT * LIGHT_DATA_SIZE];
extern uint32_t loadedLights;

// Levels / Game Management
extern char global_modname[256];
extern uint8_t startLevel;
extern uint8_t numLevels; // Can be set by gamedata.txt
extern uint8_t currentLevel;
extern bool gamePaused;
extern bool menuActive;
int32_t LoadLevels();
int32_t LoadLevelGeometry(uint8_t curlevel);
int32_t LoadLevelLights(uint8_t curlevel);
int32_t LoadLevelDynamicObjects(uint8_t curlevel);
// ----------------------------------------------------------------------------
// Event System
#define EV_NULL 0u
#define EV_INIT 1u
#define EV_KEYDOWN 10u
#define EV_KEYUP 11u
#define EV_MOUSEMOVE 12u
#define EV_MOUSEDOWN 13u
#define EV_MOUSEUP 14u
#define EV_MOUSEWARP 15u
#define EV_PLAYAUDIO_CLIP 40u
#define EV_PLAYAUDIO_STREAM 41u
#define EV_PHYSICS_TICK 50u
#define EV_PARTICLE_TICK 60u
#define EV_PAUSE 254u
#define EV_QUIT 255u

// Event Journal Buffer
#define EVENT_JOURNAL_BUFFER_SIZE 30000

// Event Queue
#define MAX_EVENTS_PER_FRAME 100

// Event System variables
typedef struct {
    double timestamp;
    double deltaTime_ns;
    uint32_t frameNum;
    int32_t payload1i; // First one used for payloads less than or equal to 4 bytes
    int32_t payload2i; // Second one used for more values or for long ints by using bitpacking
    float payload1f; // First one used for float payloads
    float payload2f; // Second one used for a 2nd value or for double via bitpacking
    uint8_t type;
} Event;
extern Event eventQueue[MAX_EVENTS_PER_FRAME];
extern int32_t eventJournalIndex;
// Journal buffer for event history to write into the log/demo file
extern Event eventJournal[EVENT_JOURNAL_BUFFER_SIZE];
extern int32_t eventIndex; // Event that made it to the counter.  Indices below this were
                // already executed and walked away from the counter.
extern int32_t eventQueueEnd; // End of the waiting line
extern FILE* activeLogFile;
extern bool log_playback;
extern double lastJournalWriteTime;
extern const char* manualLogName;
extern int32_t maxEventCount_debug;
extern uint32_t globalFrameNum;
extern double last_time;
extern double current_time;
extern float pauseRelativeTime;
int32_t EventExecute(Event* event);
int32_t EventInit(void);
int32_t EnqueueEvent(uint8_t type, int32_t payload1i, int32_t payload2i, float payload1f, float payload2f);
int32_t EnqueueEvent_IntInt(uint8_t type, int32_t payload1i, int32_t payload2i);
int32_t EnqueueEvent_Int(uint8_t type, int32_t payload1i);
int32_t EnqueueEvent_FloatFloat(uint8_t type, float payload1f, float payload2f);
int32_t EnqueueEvent_Float(uint8_t type, float payload1f);
int32_t EnqueueEvent_Simple(uint8_t type);
void clear_ev_journal(void);
void JournalLog(void);
int32_t ReadActiveLog();
int32_t JournalDump(const char* dem_file);
void clear_ev_queue(void);
double get_time(void);
int32_t EventQueueProcess(void);
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
extern uint16_t playerCellIdx, playerCellIdx_x, playerCellIdx_y, playerCellIdx_z;
extern uint16_t numCellsVisible;
extern uint8_t gridCellStates[ARRSIZE];
// extern float gridCellFloorHeight[ARRSIZE];
// extern float gridCellCeilingHeight[ARRSIZE];
extern uint32_t precomputedVisibleCellsFromHere[524288];
extern uint32_t cellIndexForInstance[INSTANCE_COUNT];
extern uint16_t cellIndexForLight[LIGHT_COUNT];
extern float worldMin_x, worldMin_z;
int32_t Cull_Init(void);
void CullCore(void);
void Cull();
// ----------------------------------------------------------------------------
// Helper Functions
static inline float deg2rad(float degrees) { return degrees * (M_PI / 180.0f); }
static inline float rad2deg(float radians) { return radians * (180.0f / M_PI); }
static inline void CellCoordsToPos(uint16_t x, uint16_t z, float* pos_x, float* pos_z) {
    *pos_x = worldMin_x + (x * WORLDCELL_WIDTH_F);
    *pos_z = worldMin_z + (z * WORLDCELL_WIDTH_F);
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
// ----------------------------------------------------------------------------
// Physics
#define MAX_DYNAMIC_ENTITIES 256
#define TERMINAL_VELOCITY 10.0f
#define PHYS_FLOAT_TO_INT_SCALEF 100.0f
extern double time_PhysicsStep;
extern Entity physObjects[MAX_DYNAMIC_ENTITIES];
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
int32_t ParticleSystemStep(void);
void ProcessInput(void);
int32_t Physics(void);
void UpdateInstanceMatrix(int32_t i);

// Construct rotation matrix (column-major, Unity: X+ right, Y+ up, Z+ forward)
inline void quat_to_matrix(Quaternion* q, float* m) {
    float x = q->x, y = q->y, z = q->z, w = q->w;
    float x2 = x * x, y2 = y * y, z2 = z * z;
    float xy = x * y, xz = x * z, yz = y * z;
    float wx = w * x, wy = w * y, wz = w * z;

    // Column-major rotation matrix for Unity (Y+ up, Z+ forward, X+ right)
    m[0]  = 1.0f - 2.0f * (y2 + z2); // Right X
    m[1]  = 2.0f * (xy + wz);        // Right Y
    m[2]  = 2.0f * (xz - wy);        // Right Z
    m[3]  = 0.0f;
    m[4]  = 2.0f * (xy - wz);        // Up X
    m[5]  = 1.0f - 2.0f * (x2 + z2); // Up Y
    m[6]  = 2.0f * (yz + wx);        // Up Z
    m[7]  = 0.0f;
    m[8]  = 2.0f * (xz + wy);        // Forward X
    m[9]  = 2.0f * (yz - wx);        // Forward Y
    m[10] = 1.0f - 2.0f * (x2 + y2); // Forward Z
    m[11] = 0.0f;
    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = 0.0f;
    m[15] = 1.0f;
}
// ----------------------------------------------------------------------------
// Input
extern bool keys[SDL_NUM_SCANCODES];
extern bool window_has_focus;
void Input_MouselookApply();
int32_t Input_KeyDown(int32_t scancode);
int32_t Input_KeyUp(int32_t scancode);
int32_t Input_MouseMove(int32_t xrel, int32_t yrel);
// ----------------------------------------------------------------------------
// Rendering
#define DEBUG_OPENGL
#ifdef DEBUG_OPENGL
#define CHECK_GL_ERROR() do { GLenum err = glGetError(); if (err != GL_NO_ERROR) DualLogError("GL Error at %s:%d: %d\n", __FILE__, __LINE__, err); } while(0)
#define CHECK_GL_ERROR_HERE(msg) \
    do { \
        GLenum err = glGetError(); \
        if (err != GL_NO_ERROR) \
            DualLogError("GL Error at %s:%d (%s): %d\n", __FILE__, __LINE__, msg, err); \
    } while(0)
#else
#define CHECK_GL_ERROR() do {} while(0)
#define CHECK_GL_ERROR_HERE() do {} while(0)
#endif
    
#define FAR_PLANE (71.68f) // Max player view, level 6 crawlway 28 cells
#define NEAR_PLANE (0.02f)
#define FAR_PLANE_SQUARED (FAR_PLANE * FAR_PLANE)
#define TEXT_WHITE 0
#define TEXT_YELLOW 1
#define TEXT_DARK_YELLOW 2
#define TEXT_GREEN 3
#define TEXT_RED 4
#define TEXT_ORANGE 5
#define TEXT_BUFFER_SIZE 1024
extern uint16_t screen_width;
extern uint16_t screen_height;
extern int32_t debugView;
extern int32_t debugValue;
extern GLint debugViewLoc_chunk, debugViewLoc_deferred, debugValueLoc_deferred, debugViewLoc_quadblit, debugValueLoc_quadblit;
extern float fogColorR, fogColorG, fogColorB;
extern uint32_t drawCallsRenderedThisFrame;
extern uint32_t verticesRenderedThisFrame;
extern GLuint precomputedVisibleCellsFromHereID;
extern GLuint cellIndexForInstanceID;
extern bool lightDirty[LIGHT_COUNT];
extern uint16_t doubleSidedInstancesHead;
extern uint16_t transparentInstancesHead;
extern bool global_modIsCitadel;
extern bool inventoryMode;
extern bool noclip;
extern bool consoleActive;
extern int32_t cursorPosition_x, cursorPosition_y;
extern float cam_x, cam_y, cam_z;
extern float cam_yaw, cam_pitch, cam_roll, cam_fov;
extern float cam_forwardx, cam_forwardy, cam_forwardz, cam_rightx, cam_righty, cam_rightz;
extern Quaternion cam_rotation;
extern float genericTextHeightFac;
extern GLuint chunkShaderProgram;
extern GLuint deferredLightingShaderProgram;
extern GLuint textShaderProgram;
extern GLuint imageBlitShaderProgram;
extern GLuint textVAO, textVBO;
extern GLint projectionLoc_text;
extern GLint textColorLoc_text;
extern GLint textTextureLoc_text;
extern GLint texelSizeLoc_text;
void InitFontAtlasses();
float quat_angle_deg(Quaternion a, Quaternion b);
void CacheUniformLocationsForShaders(void);
void Screenshot(void);
void ToggleConsole(void);
bool CursorVisible(void);
int32_t GetScreenRelativeX(float percentage);
int32_t GetScreenRelativeY(float percentage);
int32_t GetTextHCenter(int32_t pointToCenterOn, int32_t numCharactersNoNullTerminator);
extern float uiOrthoProjection[16];
float dot(float x1, float y1, float z1, float x2, float y2, float z2);
void RenderUI(void);
void RenderLoadingProgress(int32_t offset, const char* format, ...);
void ConsoleEmulator(int32_t scancode);
void RenderFormattedText(int32_t x, int32_t y, uint32_t color, const char* format, ...);
void CenterStatusPrint(const char* fmt, ...);
// ----------------------------------------------------------------------------
// Logging / Debug Prints
extern FILE *console_log_file;
void DualLog(const char* fmt, ...);
void DualLogWarn(const char* fmt, ...);
void DualLogError(const char* fmt, ...);
void DebugRAM(const char *context, ...);
#ifdef VOXEN_ENGINE_IMPLEMENTATION
//     Logs both to log file and console, usage same as printf
static void DualLogMain(FILE *stream, const char *prefix, const char *fmt, va_list args) {
    va_list copy; va_copy(copy, args);
    if (prefix) fprintf(stream, "%s\033[0m", prefix);
    vfprintf(stream, fmt, args);
    fprintf(stream, "\033[0m"); fflush(stream);
    if (console_log_file) {
        if (prefix) fprintf(console_log_file, "%s ", prefix);
        vfprintf(console_log_file, fmt, copy);
        fflush(console_log_file);
    }
    va_end(copy);
}

void DualLog(const char* fmt, ...) { va_list args; va_start(args, fmt); DualLogMain(stdout, NULL, fmt, args); va_end(args); }
void DualLogWarn(const char* fmt, ...) { va_list args; va_start(args, fmt); DualLogMain(stdout, "\033[1;38;5;208mWARN:", fmt, args); va_end(args); }
void DualLogError(const char* fmt, ...) { va_list args; va_start(args, fmt); DualLogMain(stderr, "\033[1;31mERROR:", fmt, args); va_end(args); }

// Get USS aka the total RAM uniquely allocated for the process (btop shows RSS so pulls in shared libs and double counts shared RAM).
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void DebugRAM(const char *context, ...) {
#ifdef DEBUG_RAM_OUTPUT
    char formatted_context[TEXT_BUFFER_SIZE];
    va_list args;
    va_start(args, context);
    vsnprintf(formatted_context, sizeof(formatted_context), context, args);
    va_end(args);
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
            formatted_context, info.uordblks, info.uordblks / 1024, info.uordblks / 1024.0 / 1024.0,
            uss_bytes, uss_bytes / 1024, uss_bytes / 1024.0 / 1024.0);
#endif
}
#pragma GCC diagnostic pop

void print_bytes_no_newline(int32_t count) { DualLog("%d bytes | %f kb | %f Mb",count,(float)count / 1000.0f,(float)count / 1000000.0f); }
#endif // VOXEN_ENGINE_IMPLEMENTATION
// ----------------------------------------------------------------------------
#endif // VOXEN_HEADER_H
