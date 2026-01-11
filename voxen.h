#pragma once
// #define DEBUG_RAM_OUTPUT // Debug and Compile Flags
#define ONLY_LOAD_LEVEL_NEEDS

#include <string.h>
#include <stdarg.h>
#include "./External/glad/gl.h"
#include "./External/glfw3.h"
#include "External/stb_truetype.h"
#include "types.h"

// Math
#define vabs(x) ((x) < 0 ? -(x) : (x))
#define vmin(a,b) ((a) < (b) ? (a) : (b))
#define vmax(a,b) ((a) > (b) ? (a) : (b))
#define PI 3.14159265f
#define TAU 6.2831853f
static inline float vfloor(float x) { int i = (int)x; return (float)(i > x ? i - 1 : i); }
static inline float vceil(float x) { int i = (int)x; return (float)(i < x ? i + 1 : i); }
static inline float vclamp(float x, float a, float b) { return x < a ? a : (x > b ? b : x); }
static inline float vsqrtf(float x) { union { float f; unsigned int i; } u = { x }; u.i = 0x1fbd1df5 + (u.i >> 1); return 0.5f * (u.f + x / u.f); }
static inline float vsinf(float x) { x -= TAU * vfloor(x / TAU); if (x > PI) { x -= TAU; } float s = (4/PI)*x - (4/(PI*PI))*x*vabs(x); return 0.225f*(s*vabs(s) - s) + s; }
static inline float vcosf(float x) { return vsinf(x + 1.57079632f); }
static inline float vacosf(float x) {
    float negate = (x < 0.0f) ? 1.0f : 0.0f;
    x = vabs(x);
    float ret = -0.0187293f;
    ret = ret * x + 0.0742610f;
    ret = ret * x - 0.2121144f;
    ret = ret * x + 1.5707288f;
    ret = ret * vsqrtf(1.0f - x);
    ret = ret - 2.0f * negate * ret;
    return negate * PI + (1.0f - 2.0f * negate) * ret;
}
static inline float vtan(float x) { return vsinf(x) / vcosf(x); }
static inline float vcot(float x) { float x2 = x * x; float t = x + (x2 * x) * 0.33333333f; return 1.0f / t; }
static inline float deg2rad(float degrees) { return degrees * (PI / 180.0f); }
static inline float vlog2f(float x) {
    union { float f; unsigned int i; } v = { x };
    int e = (int)((v.i >> 23) & 255) - 127;
    v.i = (v.i & 0x7FFFFF) | 0x3F800000;   // normalize mantissa to [1,2)
    float m = v.f;
    float p = m - 1.0f;
    float log2m = p * (1.3465558f + p * (-0.33942322f + p * 0.028794660f)); // polynomial approximation of log2(m)
    return (float)e + log2m;
}

static inline float vlog(float x) { return vlog2f(x) * 0.69314718f; }
static inline float vexp2f(float x) {
    float ip = vfloor(x);
    float fp = x - ip;
    float p = 1.0f + fp * (0.69314718f + fp * (0.24022651f + fp * 0.05550411f)); // poly approximation for 2^fp on [0,1]
    int ei = (int)ip + 127;
    unsigned int bits = (unsigned int)(ei << 23);
    union { unsigned int i; float f; } u = { bits };
    return u.f * p;
}

static inline float vexp(float x) { return vexp2f(x * 1.4426950409f); } // 1/ln(2)
static inline float vpow(float a, float b) { return vexp(b * vlog(a)); }

void DualLog(const char* fmt, ...);
void DualLogWarn(const char* fmt, ...);
void DualLogError(const char* fmt, ...);

#define ANIM_LOOP_ALL    0

#define ANIM_IDLE_CLOSED 1
#define ANIM_OPENING     2
#define ANIM_IDLE_OPEN   3
#define ANIM_CLOSING     4

#define ANIM_IDLE    1
#define ANIM_WALK    2
#define ANIM_RUN     3
#define ANIM_ATTACK1 4
#define ANIM_ATTACK2 5
#define ANIM_ATTACK3 6
#define ANIM_PAIN    7
#define ANIM_DYING   8
#define ANIM_DEAD    9

extern GlobalContext Sys_Global;
extern QuestBits questData;
extern SettingsSystem Sys_Settings;
extern VoxenShadowSystem voxen_Shadow_System;
extern DiagnosticsSystem Sys_Dx;
extern CheatsSystem Sys_Cheats;
extern Voxen_Text voxen_Text;
// ----------------------------------------------------------------------------
// Audio
void play_mp3(const char* path, float volume, int32_t fade_in_ms);
void play_wav(const char* path, float volume);
void InitializeAudio(void);
void ResetLevelAudio(void);
void UpdateAmbientSounds(void);
// ----------------------------------------------------------------------------
// Textures
#define MAX_VALID_TEXTURE 2048
#define MAX_TEXTURE_DIMENSION 2048
#define MAX_PALETTE_SIZE 256
#define MAX_TOTAL_PIXELS 25000000u
#define MAX_UNIQUE_COLORS 1024000u
extern bool doubleSidedTexture[MAX_VALID_TEXTURE];
extern bool transparentTexture[MAX_VALID_TEXTURE];
bool isDoubleSided(uint32_t texIndexToCheck);
bool isTransparent(uint32_t texIndexToCheck);
void LoadTextures(void);
// ----------------------------------------------------------------------------
// Models
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
extern uint32_t** modelTriangles;
extern uint16_t loadedTexturesMaxIndex;
extern uint16_t loadedModelsMaxIndex;
extern uint16_t loadedLights;
extern uint16_t gameObjectCount;
extern uint32_t modelVertexCounts[MODEL_IDX_MAX];
extern uint32_t modelTriangleCounts[MODEL_IDX_MAX];
extern uint8_t modelAnimationType[MODEL_IDX_MAX];
#define MAX_ANIMATED_MODELS 64
#define MAX_ANIMATION_CLIPS_PER_MODEL 32
extern AnimationClip modelAnimationClips[MAX_ANIMATED_MODELS][MAX_ANIMATION_CLIPS_PER_MODEL];
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

#define LIGHT_MAX_INTENSITY 8.0f
#define LIGHT_RANGE_MAX 15.36f
#define LIGHT_RANGE_MAX_SQUARED (LIGHT_RANGE_MAX * LIGHT_RANGE_MAX)

extern float lights[LIGHT_COUNT * LIGHT_DATA_SIZE];
extern bool lightOn[LIGHT_COUNT];
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
void UpdateVoxelLightLists(void);
void RenderLoadingProgress(int32_t offset, const char* text);

#define LEVEL_CYBERSPACE 13
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
#define MAX_PORTALS 64 // Max is 49 on Citadel level 7
extern uint8_t queuedLevelToLoad;
extern uint16_t playerCellIdx;
extern uint16_t numCellsVisible;
extern uint32_t gridCellStates[ARRSIZE];
extern float gridCellFloorHeight[ARRSIZE];
extern float gridCellCeilingHeight[ARRSIZE];
extern Portal activePortals[MAX_PORTALS];
extern uint8_t numActivePortals;
extern uint32_t precomputedVisibleCellsFromHere[524288];
extern float worldMin_x, worldMin_z, voxelMinCenterX, voxelMinCenterZ;
void CullInit(void);
void CullCore(void);
bool get_cull_bit(const uint32_t* arr, int idx);
static inline bool EntityIndexIsPortalBlockingDoor(uint16_t entIdx) { return (entIdx >= 496 && entIdx <= 514 && entIdx != 502 && entIdx != 505 && entIdx != 506 && entIdx != 507); }// All doors except see-through doors.
// Credits
extern char creditStats[4096];
void CreditsStats(void);
void CreditsScroll(void);
void RenderFormattedText(float x, float y, uint32_t color, uint8_t fontID, const char * restrict format, ...);
// ----------------------------------------------------------------------------
// Physics
#define MAX_DYNAMIC_ENTITIES 512
#define TERMINAL_VELOCITY 10.0f
#define PHYS_FLOAT_TO_INT_SCALEF 100.0f
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
#define FROB_DISTANCE 4.9f
#define PLAYER_CAPSULE_TOTAL_HEIGHT 2.0f
#define PLAYER_CAPSULE_RADIUS 0.48f
#define PLAYER_CROUCH_RATIO 0.6f
#define PLAYER_PRONE_RATIO 0.2f
#define PLAYER_TRANSITION_TO_PRONE_ADD 0.1f
#define PLAYER_CAMERA_OFFSET_Y 0.84f
extern uint16_t testPointInSolid;
extern uint8_t boosterActive;

#define LAYER_MASK_PLAYER_COLLIDESWITH ((1u << PhysicsLayer_Clip) | (1u << PhysicsLayer_NPCBullet) | (1u << PhysicsLayer_Player2) | (1u << PhysicsLayer_Door) \
										| (1u << PhysicsLayer_Trigger) | (1u << PhysicsLayer_PlayerTriggerOnly) | (1u << PhysicsLayer_Default) | (1u << PhysicsLayer_TransparentFX) \
										| (1u << PhysicsLayer_IgnoreRaycast) | (1u << PhysicsLayer_Geometry) | (1u << PhysicsLayer_NPC))
#define LAYER_MASK_NPC_COLLIDESWITH ((1u << PhysicsLayer_Clip) | (1u << PhysicsLayer_NPCClip) | (1u << PhysicsLayer_PlayerBullets) | (1u << PhysicsLayer_Player2) | (1u << PhysicsLayer_Player) | (1u << PhysicsLayer_Door) \
										| (1u << PhysicsLayer_Trigger) | (1u << PhysicsLayer_NPCTrigger) | (1u << PhysicsLayer_Default) | (1u << PhysicsLayer_TransparentFX) \
										| (1u << PhysicsLayer_IgnoreRaycast) | (1u << PhysicsLayer_Geometry) | (1u << PhysicsLayer_NPC))
#define LAYER_MASK_NPC_SIGHT ((1u << PhysicsLayer_Default) | (1u << PhysicsLayer_Geometry) | (1u << PhysicsLayer_Door) | (1u << PhysicsLayer_InterDebris) \
	                          | (1u << PhysicsLayer_PhysObjects) | (1u << PhysicsLayer_Player))

#define LAYER_MASK_NPC_ATTACK ((1u << PhysicsLayer_Default) | (1u << PhysicsLayer_Geometry) | (1u << PhysicsLayer_NPC) | (1u << PhysicsLayer_Door) \
							   | (1u << PhysicsLayer_InterDebris) | (1u << PhysicsLayer_PhysObjects) | (1u << PhysicsLayer_Player))

#define LAYER_MASK_NPC_COLLISION ((1u << PhysicsLayer_Default) | (1u << PhysicsLayer_TransparentFX) | (1u << PhysicsLayer_IgnoreRaycast) | (1u << PhysicsLayer_Geometry) \
							      | (1u << PhysicsLayer_NPC) | (1u << PhysicsLayer_Door) | (1u << PhysicsLayer_InterDebris) | (1u << PhysicsLayer_Player) \
							      | (1u << PhysicsLayer_Clip) | (1u << PhysicsLayer_NPCClip) | (1u << PhysicsLayer_PhysObjects))
#define LAYER_MASK_PLAYER_FROB ((1u << PhysicsLayer_Default) | (1u << PhysicsLayer_Geometry) | (1u << PhysicsLayer_Water) | (1u << PhysicsLayer_Door) \
								| (1u << PhysicsLayer_InterDebris) | (1u << PhysicsLayer_PhysObjects) | (1u << PhysicsLayer_CorpseSearchable))

#define LAYER_MASK_PLAYER_TARGET_ID_FROB ((1u << PhysicsLayer_Default) | (1u << PhysicsLayer_Geometry) | (1u << PhysicsLayer_Door) | (1u << PhysicsLayer_NPC) | (1u << PhysicsLayer_CorpseSearchable))
#define LAYER_MASK_PLAYER_ATTACK ((1u << PhysicsLayer_Default) | (1u << PhysicsLayer_Geometry) | (1u << PhysicsLayer_NPC) | (1u << PhysicsLayer_PlayerBullets) \
								  | (1u << PhysicsLayer_Door) | (1u << PhysicsLayer_InterDebris) | (1u << PhysicsLayer_PhysObjects) | (1u << PhysicsLayer_CorpseSearchable))

#define LAYER_MASK_EXPLOSION ((1u << PhysicsLayer_Default) | (1u << PhysicsLayer_Geometry) | (1u << PhysicsLayer_NPC) | (1u << PhysicsLayer_PlayerBullets) | (1u << PhysicsLayer_Door) \
							  | (1u << PhysicsLayer_InterDebris) | (1u << PhysicsLayer_PhysObjects) | (1u << PhysicsLayer_Player) | (1u << PhysicsLayer_Player2) | (1u << PhysicsLayer_CorpseSearchable))

#define LAYER_MASK_PLAYER_FEET ((1u << PhysicsLayer_Default) | (1u << PhysicsLayer_Geometry))

void UpdateInstanceMatrix(int32_t i);
void AddForce(uint16_t idx, Vector3 force, bool isImpulse);
int32_t Physics(void); // Main event tick
RaycastHit Raycast(Vector3 origin, Vector3 dir, float distance, uint32_t layerMask);
void RaycastAll(Vector3 origin, Vector3 dir, float distance, uint32_t layerMask, RaycastHit* hits, uint16_t maxCount);
bool CheckCapsule(Vector3 start, Vector3 end, float capsuleRadius, float capsuleHeight, uint32_t layerMask);
RaycastHit CapsuleCast(Vector3 start, Vector3 end, float capsuleRadius, float castDist, uint32_t layerMask, bool hitTriggers);
void ApplyPlayerMovements(void);
// ----------------------------------------------------------------------------
// Input
extern InputSystem Sys_Input;
void CycleToNextMonitor(GLFWwindow* window);
void Input_Init(GLFWwindow* window);
void Input_MouselookApply(void);
int32_t Input_KeyDown(int32_t scancode);
int32_t Input_KeyUp(int32_t scancode);
int32_t Input_MouseMove(int32_t xrel, int32_t yrel);
void ProcessInput(void);
bool MouseWheelBoundAndRolled(int setCode);
void UpdatePlayerFacingAngles(void);
void InputClearRisingAndFallingEdges(void);
bool Forward(void);
bool StrafeLeft(void);
bool Backpedal(void);
bool StrafeRight(void);
bool Jump(void);
bool JumpDown(void);
bool Crouch(void);
bool Prone(void);
bool LeanLeft(void);
bool LeanRight(void);
bool Sprint(void);
bool TurnLeft(void);
bool TurnRight(void);
bool LookUp(void);
bool LookDown(void);
bool RecentLog(void);
bool Biomonitor(void);
bool Sensaround(void);
bool Lantern(void);
bool Shield(void);
bool Infrared(void);
bool Email(void);
bool Booster(void);
bool Jumpjets(void);
bool Attack(void);
bool Use(void);
bool Menu(void);
bool ToggleMode(void);
bool Reload(void);
bool WeaponCycUp(void);
bool WeaponCycDown(void);
bool Grenade(void);
bool GrenadeCycUp(void);
bool GrenadeCycDown(void);
bool ChangeAmmoType(void);
bool Patch(void);
bool PatchCycUp(void);
bool PatchCycDown(void);
bool Map(void);
bool SwimUp(void);
bool SwimDn(void);
bool SwapAmmoType(void);
bool Console(void);
bool MouseWheelUp(void);
bool MouseWheelDn(void);
void LoadConfig(void);
void SaveConfig(void);
void ApplySettings(void);
// ----------------------------------------------------------------------------
// Rendering
#define DEBUG_OPENGL
#ifdef DEBUG_OPENGL
	void glDebugMessageCallback(GLDEBUGPROC callback, const void* userParam);
	void glDebugMessageControl(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled);
	#define CHECK_GL_ERROR() do { GLenum err = glGetError(); if (err != GL_NO_ERROR) DualLogError("GL Error at %s:%d: %d\n", __FILE__, __LINE__, err); } while(0)
#else
	#define CHECK_GL_ERROR() do {} while(0)
#endif
    
#define FAR_PLANE (71.68f) // Max player view, level 6 crawlway 28 cells
#define NEAR_PLANE (0.02f)
#define FAR_PLANE_SQUARED (FAR_PLANE * FAR_PLANE)
extern float fogColorR, fogColorG, fogColorB, fogBaseDensityForLevel;
void SetFog(void);
extern bool lightDirty[LIGHT_COUNT];
#define CURSOR_SCREEN_PERCENTAGE 0.02f
extern int32_t cursorPosition_x, cursorPosition_y;
extern float cam_yaw, cam_pitch, cam_roll;
extern uint16_t loadedModelsMaxIndex;
extern float aspect3D;
extern float rasterPerspectiveProjection[16];
extern float shadowmapsPerspectiveProjection[16];
extern float uiOrthoProjection[16];
void Screenshot(void);
void ToggleConsole(void);
void ConsoleEmulator(int32_t keycode);
bool CursorVisible(void);
void SetSkyRotateSpeed(void);
// ----------------------------------------------------------------------------
// UI
#define TEXT_BUFFER_SIZE 1024
#define FONT_ATLAS_SIZE 3072
#define MAX_GLYPHS 639
#define FONT_NORMAL 0
#define FONT_STOPD 1
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
float GetScreenRelativeX(float percentage);
float GetScreenRelativeY(float percentage);
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
#define EV_INT_FIELD_UNUSED 0
#define EV_FLOAT_FIELD_UNUSED 0.0f
#define EVENT_JOURNAL_BUFFER_SIZE 1000
#define MAX_EVENTS_PER_FRAME 100
extern Event eventQueue[MAX_EVENTS_PER_FRAME];
extern int32_t eventJournalIndex;
extern bool journalFirstWrite;
extern Event eventJournal[EVENT_JOURNAL_BUFFER_SIZE]; // Journal buffer for event history to write into the log/demo file
extern int32_t eventIndex; // Event that made it to the counter.  Indices below this were already executed and walked away from the counter.
extern bool log_playback;
int32_t ReadActiveLog(void);
void EventSystemInit(int32_t argc, char* command, char* command_input1);
int32_t EnqueueEvent(uint8_t type, int32_t payload1i, int32_t payload2i, float payload1f, float payload2f);
double get_time(void);
int32_t EventQueueProcess(void);
// ----------------------------------------------------------------------------
// Patches
#define PATCH_TIME_BERSERK 30.0f
// ----------------------------------------------------------------------------
// Logging / Debug Prints
void OpenConsoleLogFile(void);
void Screenshot(void);
void CenterStatusPrint(const char* fmt, ...);
void JournalDump(const char* dem_file);
void DebugRAM(const char *context);
void GetLevel_Transform_Offsets(int32_t curlevel, float* ofsx, float* ofsy, float* ofsz);
void GetLevel_LightsStaticImmutable_ContainerOffsets(int32_t curlevel, float* ofsx, float* ofsy, float* ofsz);
// ============================================================================
// ----------------------------------------------------------------------------
// Helper Functions
extern uint32_t random_range_rng;
double get_time(void);
void AddInstance(uint16_t entIdx, uint16_t instanceIdx);
uint64_t file_stamp(const FileFingerprint *fp);
bool get_file_fingerprint(const char *path, FileFingerprint *fp);
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
const char* GetPrefabNameFromIndex(int constIndex);
static inline void CellCoordsToPos(uint16_t x, uint16_t z, float* pos_x, float* pos_z) {
    *pos_x = worldMin_x + (x * WORLDCELL_WIDTH_F);
    *pos_z = worldMin_z + (z * WORLDCELL_WIDTH_F);
}

static inline int32_t clamp(int32_t val, int32_t min, int32_t max) {
    if (val > max) return max;
    if (val < min) return min;
    return val;
}

static inline float clampf(float val, float min, float max) {
    if (val > max) return max;
    if (val < min) return min;
    return val;
}

static inline int32_t PosGetCellCoordX(float pos_x) { return (uint16_t)clamp((int32_t)vfloor((pos_x - worldMin_x + CELLXHALF) / WORLDCELL_WIDTH_F), 0, WORLDX_0BASED); }
static inline int32_t PosGetCellCoordZ(float pos_z) { return (uint16_t)clamp((int32_t)vfloor((pos_z - worldMin_z + CELLXHALF) / WORLDCELL_WIDTH_F), 0, WORLDX_0BASED); }
static inline int32_t PosGetCellCoords(float pos_x, float pos_z) { return (PosGetCellCoordZ(pos_z) * WORLDX) + PosGetCellCoordX(pos_x); } // Clamped just above.
static inline bool XZPairInBounds(int32_t x, int32_t z) { return (x < WORLDX && z < WORLDZ && x >= 0 && z >= 0); }
static inline void flag_enable(uint32_t *flags, uint32_t bit) { *flags |= bit; }
static inline void flag_disable(uint32_t *flags, uint32_t bit) { *flags &= ~bit; }
static inline void flag_set(uint32_t *flags, uint32_t bit, bool state) { *flags = (*flags & ~bit) | (-state & bit); }

extern RenderSystem Sys_Render; // Added last to make use of all defines for sizes.

// Math, Vectors, Quaternions
static inline Vector3 Vector3_A_plus_B(Vector3 a, Vector3 b) { return (Vector3){a.x + b.x, a.y + b.y, a.z + b.z}; }
static inline Vector3 Vector3_A_minus_B(Vector3 a, Vector3 b) { return (Vector3){a.x - b.x, a.y - b.y, a.z - b.z}; }
static inline Vector3 scale_vector3(Vector3 v, float s) { Vector3 res = {v.x * s, v.y * s, v.z * s}; return res; }
static inline float dot(float x1, float y1, float z1, float x2, float y2, float z2) { return x1*x2 + y1*y2 + z1*z2; }
static inline float dot_vector3(Vector3 a, Vector3 b) { return dot(a.x,a.y,a.z, b.x,b.y,b.z); }
static inline float quat_dot(Quaternion a, Quaternion b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }
static inline float magnitude_vector3(const Vector3 v) { return vsqrtf(dot_vector3(v, v)); }
static inline Vector3 min_vector3(Vector3 a, Vector3 b) { return (Vector3){ a.x<b.x ? a.x : b.x, a.y<b.y ? a.y : b.y, a.z<b.z ? a.z : b.z }; }
static inline Vector3 max_vector3(Vector3 a, Vector3 b) { return (Vector3){ a.x>b.x ? a.x : b.x, a.y>b.y ? a.y : b.y, a.z>b.z ? a.z : b.z }; }
static inline float dist_sq_vector3(Vector3 a, Vector3 b) { Vector3 d = Vector3_A_minus_B(a, b); return dot_vector3(d, d); }
static inline Vector3 cross_vector3(Vector3 a, Vector3 b) { return (Vector3){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }
static inline Vector3 normalize_vector3(Vector3 v) { float len = magnitude_vector3(v); return len > 0.000001f ? (Vector3){v.x / len, v.y / len, v.z / len} : v; }
static inline float squareDistance2D(float x1, float z1, float x2, float z2) { float dx = x2 - x1; float dz = z2 - z1; return dx * dx + dz * dz; }
static inline float squareDistance3D(float x1, float y1, float z1, float x2, float y2, float z2) { float dx = x2 - x1; float dy = y2 - y1; float dz = z2 - z1; return dx * dx + dy * dy + dz * dz; }
uint16_t PointInSolid(Vector3 point, uint32_t layerMask);
void normalize_vector(float* x, float* y, float* z);
__attribute__((pure)) Vector3 mul_mat4_vector3(const float* m, Vector3 v);
void quat_to_matrix(Quaternion* q, float* m);
Quaternion axis_angle_quaternion(const Vector3 axis, float angle);
void normalize_quaternion(Quaternion* q);
Vector3 quat_rotate(Quaternion q, Vector3 v);
void UpdateInstanceMatrix(int32_t i);
bool EntityIsAnimated(uint16_t entIdx);

static inline float quat_angle_deg(Quaternion a, Quaternion b) {
    float d = vabs(quat_dot(a, b));
    if (d > 1.0f) d = 1.0f;
    return vacosf(d) * 2.0f * (180.0f / PI);
}

static inline Quaternion mul_quaternion(Quaternion a, Quaternion b) {
    return (Quaternion){
        a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z,
        a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y,
        a.w*b.y - a.x*b.z + a.y*b.w + a.z*b.x,
        a.w*b.z + a.x*b.y - a.y*b.x + a.z*b.w
    };
}

static inline Vector3 rotate_quaternion(Quaternion rotation, Vector3 axis) {
    Vector3 qv = {rotation.x, rotation.y, rotation.z}; // Take only the xyz, not w
    Vector3 uv = cross_vector3(qv, axis);
    return Vector3_A_plus_B(axis,Vector3_A_plus_B(scale_vector3(uv, 2.0f * rotation.w), scale_vector3(cross_vector3(qv, uv), 2.0f)));
}

static inline void mul_mat4(float *out, const float *a, const float *b) { // out = a * b
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
   
    for (int32_t i = 0; i < 16; i++) out[i] = result[i]; // copy back
}

#define MAX_ENTITIES 768 // Unique entity types, different than INSTANCE_COUNT which is the number of instances of any of these entities.
#define NULLENT 0u
#define PLAYER1 1u
#define PLAYER2 2u
#define START_INDEX_LEVEL_INSTANCES 3
#define ENTFLAG_ACTIVE        1u
#define ENTFLAG_CARDCHUNK     2u
#define ENTFLAG_GROUNDED      4u
#define ENTFLAG_USEGRAVITY    8u
#define ENTFLAG_KINEMATIC    16u
#define ENTFLAG_RIGIDBODY    32u
#define ENTFLAG_DOUBLESIDED  64u
#define ENTFLAG_TRANSPARENT 128u
#define ENTFLAG_CHANGE_TEX_ON_ACTIVE 256u
#define ENTFLAG_BLINK_TEX_ON_ACTIVE  512U
#define ENTFLAG_NO_SHADOWS 1024u

extern Entity entities[MAX_ENTITIES]; // Global array of entity definitions
extern Entity instances[INSTANCE_COUNT];

extern bool instanceIsLODArray[INSTANCE_COUNT];
extern float modelMatrices[INSTANCE_COUNT * 16];
extern uint8_t dirtyInstances[INSTANCE_COUNT];

void ParseGameData(void);
bool parse_data_file(DataParser *parser, const char *filename);

extern uint16_t invalidModelIndexCount;
#ifdef ONLY_LOAD_LEVEL_NEEDS
    extern bool modelIndexUsedForCurrentLevel[MODEL_IDX_MAX];
    extern bool textureIndexUsedForCurrentLevel[MAX_VALID_TEXTURE];
#endif
extern uint16_t entityCount;
extern uint16_t loadedInstances;
extern uint16_t startOfDoubleSidedInstances;
extern uint16_t startOfTransparentInstances;
void InitializeEntity(Entity* entry);
void DualLogEntityInstance(uint16_t idx);
void DualLogEntity(Entity ent);
void LoadEntities(void);
void LoadLevel(uint8_t curlevel);
