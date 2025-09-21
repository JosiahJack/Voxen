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
#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdbool.h>
#include <GL/glew.h>
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
#define MAX_PATH 256
#define ENT_NAME_MAXLEN_NO_NULL_TERMINATOR 31

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
    char name[ENT_NAME_MAXLEN_NO_NULL_TERMINATOR + 1]; // 31 characters max, plus 1 for null terminator, results in nice even multiple of 4 bytes
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
#define INSTANCE_COUNT 5800 // Max 5454 for Citadel level 7 geometry, Max 295 for Citadel level 1 dynamic objects
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
#define SHADOW_ANGLE_DEG_BINS 360

extern float lights[LIGHT_COUNT * LIGHT_DATA_SIZE];

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

#define LUXEL_SIZE 0.16f
#define VOXEL_COUNT 262144 // 64 * 64 * 8 * 8
#define VOXEL_SIZE 0.32f
#define CELL_SIZE 2.56f // Each cell is 2.56x2.56
#define MAX_LIGHTS_PER_VOXEL 24 // Cap to prevent overflow
#define CELL_VISIBLE       1
#define CELL_OPEN          2
#define CELL_CLOSEDNORTH   4
#define CELL_CLOSEDEAST    8
#define CELL_CLOSEDSOUTH  16
#define CELL_CLOSEDWEST   32
#define CELL_SEES_SUN     64
#define CELL_SEES_SKYBOX 128
extern uint16_t playerCellIdx;
extern uint16_t playerCellIdx_x;
extern uint16_t playerCellIdx_y;
extern uint16_t playerCellIdx_z;
extern float cam_x;
extern float cam_y;
extern float cam_z;
extern uint16_t numCellsVisible;
extern uint8_t gridCellStates[ARRSIZE];
extern float gridCellFloorHeight[ARRSIZE];
extern float gridCellCeilingHeight[ARRSIZE];
extern uint32_t precomputedVisibleCellsFromHere[524288];
extern uint32_t cellIndexForInstance[INSTANCE_COUNT];
extern uint16_t cellIndexForLight[LIGHT_COUNT];
extern float worldMin_x;
extern float worldMin_z;
bool XZPairInBounds(int32_t x, int32_t z);
int32_t Cull_Init(void);
void CullCore(void);
void Cull();
// ----------------------------------------------------------------------------
// Physics
#define MAX_DYNAMIC_ENTITIES 256
#define TERMINAL_VELOCITY 10.0f
#define PLAYER_RADIUS 0.48f
#define PLAYER_HEIGHT 2.00f
#define PLAYER_CAM_OFFSET_Y 0.84f // Split capsule shape in the middle, camera is thus 0.16 away from top of the capsule ((2 / 2 = 1) - 0.84)
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

extern uint16_t screen_width;
extern uint16_t screen_height;
extern uint32_t drawCallsRenderedThisFrame;
extern uint32_t verticesRenderedThisFrame;
extern GLuint precomputedVisibleCellsFromHereID;
extern GLuint cellIndexForInstanceID;
extern bool lightDirty[MAX_VISIBLE_LIGHTS];
extern uint16_t doubleSidedInstancesHead;
extern uint16_t transparentInstancesHead;
extern bool global_modIsCitadel;

float quat_angle_deg(Quaternion a, Quaternion b);
void CacheUniformLocationsForShaders(void);
void Screenshot(void);
// ----------------------------------------------------------------------------
// Logging / Debug Prints
extern FILE *console_log_file;
void DualLog(const char *fmt, ...); // Logs both to log file and console, usage same as printf
void DualLogWarn(const char *fmt, ...);
void DualLogError(const char *fmt, ...);
void print_bytes_no_newline(int32_t count);
void DebugRAM(const char *context, ...);
void RenderLoadingProgress(int32_t offset, const char* format, ...);
// ----------------------------------------------------------------------------
#endif // VOXEN_HEADER_H
