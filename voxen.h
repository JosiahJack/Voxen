#pragma once
#if defined(LINUX)
//     #define DEBUG_RAM_OUTPUT // Debug and Compile Flags
#endif

#if defined(_WIN32) || defined(__CYGWIN__)
    #define ENGINE_TO_MOD __declspec(dllimport)
#elif defined(__APPLE__) || defined(__linux__) || defined(__ANDROID__)
    #define ENGINE_TO_MOD __attribute__((visibility("default")))
#else
    #define ENGINE_TO_MOD     // fallback — may need manual .def / version script
#endif

#include "common.h"
#define SetMemoryToValueForNBytes __builtin_memset
#define CopyMemoryFromBtoAForNBytes __builtin_memcpy
#define MAX_KEYS 512
#define MAX_MOUSE_BUTTONS 8
#define MAX_JOYSTICK_BUTTONS 16
#define MAX_JOYSTICK_HATS 5
#define MAX_GAMEPAD_BUTTONS 20
extern GlobalContext Sys_Global;
extern Entity entities[MAX_ENTITIES]; // Global array of entity definitions (e.g. prefabs)
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
	uint32_t globalFrameNum;
	double cpuTime;
    double thisFrameTime;
    double cpuFrameTime;
	double lastFrameSecCountTime;
	uint32_t lastFrameSecCount;
	uint32_t framesPerLastSecond;
	uint32_t worstFPS;
	Vector3 debugLine_start;
	Vector3 debugLine_end;
	double debugLineFinished;
	uint32_t debugLineVertCount;
} DiagnosticsSystem;
extern DiagnosticsSystem Sys_Dx;

#define LIGHT_COUNT 2048 // MAX CITADEL LIGHT COUNT is 1561 for Level 7, leaves room for dynamic lights from projectiles
#define MAX_SHADOWMAPS 128u
#define SHADOW_MAP_SIZE 128u
#define TOTAL_SHADOWMAP_PIXELS (MAX_SHADOWMAPS * (SHADOW_MAP_SIZE * SHADOW_MAP_SIZE * 6U))
typedef struct {
    double shadowTime;
    uint32_t numShadowsCouldRender;
    uint32_t shadowmapSizes[MAX_SHADOWMAPS];
    uint32_t shadowmapOffsets[MAX_SHADOWMAPS];
    uint32_t shadowmapIndirectionList[LIGHT_COUNT];
    float shadDotThresh;
} VoxenShadowSystem;
extern VoxenShadowSystem voxen_Shadow_System;

#define SOUNDS_COUNT 670
#define TEXT_DATA_FILEBUFFER_SIZE 65536 // 16 pages
#define TEXT_STRING_COUNT 1100
#define TEXT_LOCALIZATION_MAX_LENGTH 1280
#define TEXT_LOGS_COUNT 134
typedef struct {	
	uint8_t file_data[TEXT_DATA_FILEBUFFER_SIZE]; // Found that only 59430 were needed at one point, padded for safety and typo fixes
	char stringTable[TEXT_STRING_COUNT][TEXT_LOCALIZATION_MAX_LENGTH]; // Hefty table for localization support.
	uint16_t audioLogImagesRefIndicesLH[TEXT_LOGS_COUNT];
	uint16_t audioLogImagesRefIndicesRH[TEXT_LOGS_COUNT];
	uint8_t audioLogType[TEXT_LOGS_COUNT];
	uint8_t audioLogLevelFound[TEXT_LOGS_COUNT];
} Voxen_Text;
extern Voxen_Text Sys_Text;

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
typedef struct { uint16_t x,z; } PortalCell;

typedef struct {
    PortalCell cellA;    // one side (usually the cell the door happened to just barely floating point rounding error start in)
    PortalCell cellB;    // tother side
    bool     portalNS; // true when the two cells share N or S edge, else they share E and W edges.
    bool     open;     // door is open
    bool     dirty;
} Portal;

typedef struct { float speed; uint16_t frameStart; uint16_t frameEnd; uint16_t frameStartModelIndex; uint8_t framerate; } AnimationClip;

typedef struct {
    Vector3 point;
    Vector3 normal;
    float distance;
    uint16_t hitInstanceIndex;
    bool hit;
} RaycastHit;



#define MAX_SAVENAME_LENGTH 24
#define MENU_ITEMS_MAX 52 // Input page has 40 for the input codes, plus back button plus 10 other widgets
typedef uint8_t MenuPages;
static const uint8_t MenuPages_FrontPage = 0;
static const uint8_t MenuPages_Singleplayer = 1;
static const uint8_t MenuPages_Multiplayer = 2;
static const uint8_t MenuPages_NewGame = 3;
static const uint8_t MenuPages_Load = 4;
static const uint8_t MenuPages_Options = 5;
static const uint8_t MenuPages_Save = 6;
static const uint8_t MenuPages_IntroVideo = 7;
static const uint8_t MenuPages_CreditsVideo = 8;
void MenuGoBack(void);

#define PATCH_BERSERK   1
#define PATCH_DETOX     2
#define PATCH_GENIUS    4
#define PATCH_MEDI      8
#define PATCH_REFLEX   16
#define PATCH_SIGHT    32
#define PATCH_STAMINUP 64
#define BERSERK_TIME  20.0
#define DETOX_TIME    60.0
#define GENIUS_TIME  180.0
#define MEDI_TIME     35.0
#define REFLEX_TIME  155.0
#define SIGHT_TIME    40.0
#define STAMINUP_TIME 60.0
#define SIGHT_SIDE_EFFECT_TIME 17.0
#define REFLEX_TIME_SCALE 0.25
#define DEFAULT_TIME_SCALE 1.0
#define BERSERK_DAMAGE_MULTIPLIER 4.0f // Quad Damage!
#define NITRO_MIN_TIME     1.0
#define NITRO_MAX_TIME    60.0
#define NITRO_DEFAULT_TIME 7.0
#define EARTH_SHAKER_MIN_TIME      4.0
#define EARTH_SHAKER_MAX_TIME     60.0
#define EARTH_SHAKER_DEFAULT_TIME 10.0
#define GLOBAL_SHAKE_DISTANCE 0.3f
#define GLOBAL_SHAKE_FORCE    1.0f
#define HW_COUNT 14
#define HW_SYS    1 // System Analyzer
#define HW_NAV    2 // Navigation Unit
#define HW_ERD    4 // Datareader/EReader
#define HW_SNS    8 // Sensaround
#define HW_TID   16 // Target Identifier
#define HW_SHD   32 // Energy Shield
#define HW_BIO   64 // Biomonitor
#define HW_LAN  128 // Head Mounted Lantern
#define HW_ENV  256 // Envirosuit
#define HW_BST  512 // Turbo Motion Booster
#define HW_JET 1024 // Jump Jet Boots
#define HW_INF 2048 // Infrared Night Sight Enhancement

#define SW_DRILL  0
#define SW_PULSER 1
#define SW_SHIELD 2
#define SW_TURBO  3
#define SW_DECOY  4
#define SW_RECALL 5
#define SW_GAMES  6

#define MINIGAME_PING        1
#define MINIGAME_15          2
#define MINIGAME_WING0       4
#define MINIGAME_BOTBOUNCE   8
#define MINIGAME_EEL_ZAPPER 16
#define MINIGAME_ROAD       32
#define MINIGAME_TRIOPTOE   64

// Hw referenceIndex, ref14Index
// Sys 21,0
// Nav 22,1
// Ere 23,2
// Sen 24,3
// Trg 25,4
// Shi 26,5
// Bio 27,6
// Lan 28,7
// Env 29,8
// Boo 30,9
// Jum 31,10
// Nig 32,11
typedef struct {
    uint32_t accessCardOwned;
    uint8_t hasSoft;
    uint8_t softVersions[7];
    bool hasLog[134];
    bool readLog[134];
    uint16_t numLogsFromLevel[10];
    int lastAddedIndex;
	bool beepDone;
	bool logPaused;
    bool hasNewEmail;
    bool hasNewNotes;
	int emailCurrent;
	int emailIndex;
    uint8_t hasMinigame;
    uint16_t hasHardware;
    uint16_t hardwareIsActive;
    uint8_t hardwareVersion[HW_COUNT];
    uint8_t hardwareVersionSetting[HW_COUNT];
    uint16_t hardwareInvReferenceIndex[HW_COUNT];
    int hardwareInvCurrent; // Current slot in the general inventory (14 slots).
	int hardwareInvIndex; // Current index to the item look-up table.
	int generalInventoryIndexRef[14];
    double nitroTimeSetting;
    double earthShakerTimeSetting;
    bool currentCyberItem;
    bool isPulserNotDrill;
    int globalLookupIndex;
    int weaponInventoryIndices[7];
    int weaponInventoryAmmoIndices[7];
    double waitTilNextFire;
    bool overloadEnabled;
    double reloadFinished;
    double lerpStartTime;
    float reloadLerpValue;
    float sparqSetting;
    float ionSetting;
    float blasterSetting;
    float plasmaSetting;
    float stungunSetting;
    bool recoiling;
    uint8_t lerpUp;
    double justFired;
	float energySliderClickedTime;
	float cyberWeaponAttackFinished;
	float targetY;
    uint16_t heldObjectIndex;
    bool holdingObject;
    uint16_t weaponIndex;
} InventorySystem;
extern InventorySystem inventoryPlayer1;
extern InventorySystem inventoryPlayer2;

void PlayerInit(uint16_t i);
void TakeEnergy(float drain);
void GiveEnergy(float give, EnergyType type);
void BioMonitorInit(void);
void BioMonitorUpdate(void);

#define MULTI_MEDIA_TAB_EMAIL_TABLE 0
#define MULTI_MEDIA_TAB_LOG_TABLE   1
#define MULTI_MEDIA_TAB_DATA_TABLE  2
#define MULTI_MEDIA_TAB_NOTES       3
typedef struct {
	int lastMultiMediaTabOpened;
	bool lastWeaponSideRH;
	bool lastItemSideRH;
	bool lastAutomapSideRH;
	bool lastTargetSideRH;
	bool lastDataSideRH;
	bool lastSearchSideRH;
	bool lastLogSideRH;
	bool lastLogSecondarySideRH;
	bool lastMinigameSideRH;
	double logFinished;
	bool logActive;
	AudioLogType logType;
	uint16_t linkedElevatorDoor;
	Vector3 objectInUsePos;
	uint16_t tetheredPGP;
	uint16_t tetheredPWP;
	uint16_t tetheredSearchable;
	uint16_t tetheredKeypadElevator;
	uint16_t tetheredKeypadKeycode;
	bool paperLogInUse;
	bool usingObject;
	int applyButtonReferenceIndex;
	int curCenterTab;
    bool isBlocking;
	bool mouseClickHeldOverGUI;
	bool isRH;
	int wep16index;
	int tempSpriteIndex;
	float lastEnergy;
	float lastHealth;
	double tickFinished; // Visual only, Time.time controlled
	int count;
	bool centerTabNotified[4];
	double centerTabsTickFinished; // Visual only, Time.time controlled
	bool highlightStatus[4];
	uint8_t highlightTickCount[4];
	double blinkFinished;
	double beepFinished;
	uint8_t beepCount;
	bool audPaused;
} SystemUI;
extern SystemUI Sys_UIPlayer1;
extern SystemUI Sys_UIPlayer2;

#define MODEL_IDX_MAX 6805
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
extern uint8_t dirtyInstances[INSTANCE_COUNT];

typedef struct {
    Entity* entries;
    uint32_t count;
    uint32_t capacity;
} DataParser;

void DualLog(const char* fmt, ...);
void DualLogWarn(const char* fmt, ...);
void DualLogError(const char* fmt, ...);
extern const char* sounds[670];
extern const char* audioLogs[134];
void play_mp3(const char* path, int32_t fade_in_ms);
void play_wav(const char* path, float volume, Vector3 pos, bool positional);
#define MAX_VALID_TEXTURE 2048
#define MAX_TEXTURE_DIMENSION 2048
#define MAX_PALETTE_SIZE 256
#define MAX_TOTAL_PIXELS 27800000u
#define MAX_UNIQUE_COLORS 80000u
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
extern Vector3 lightsNewPosition[LIGHT_COUNT];
extern uint16_t gameObjectCount;
extern uint32_t modelVertexCounts[MODEL_IDX_MAX];
extern uint32_t modelTriangleCounts[MODEL_IDX_MAX];
extern bool modelHasAnimation[MODEL_IDX_MAX];
#define MAX_ANIMATED_MODELS 64
#define MAX_ANIMATION_CLIPS_PER_MODEL 32
extern const AnimationClip modelAnimationClips[MAX_ANIMATED_MODELS][MAX_ANIMATION_CLIPS_PER_MODEL];

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
void RenderLoadingProgress(int32_t offset, const char* text);

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
#define MAX_LIGHTS_PER_VOXEL 64 // Cap to prevent overflow
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
void AddCameraPosition(uint16_t i);
void RemoveCameraPosition(uint16_t i);
void CullInit(void);
bool CullCore(void);
bool get_cull_bit(const uint32_t* arr, int idx);
static inline __attribute__((always_inline)) bool EntityIndexIsPortalBlockingDoor(uint16_t entIdx) { return (entIdx >= 496 && entIdx <= 514 && entIdx != 502 && entIdx != 505 && entIdx != 506 && entIdx != 507); }// All doors except see-through doors.
// Credits
extern char creditStats[4096];
void CreditsScroll(void);
void RenderFormattedText(int16_t x, int16_t y, uint32_t color, uint8_t fontID, float scale, const char * restrict format, ...);
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
void ProcessInput(void);
bool MouseWheelBoundAndRolled(int setCode);
void UpdatePlayerFacingAngles(void);
void InputClearRisingAndFallingEdges(void);
void LoadConfig(void);
void SaveConfig(void);
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
#define MAX_DEBUG_LINE_VERTS 8
extern int fogFac;
extern float fogColorR, fogColorG, fogColorB, fogBaseDensityForLevel;
extern bool lightDirty[LIGHT_COUNT];
#define VOXEL_COUNT 262144 // 64 * 64 * 8 * 8
#define VOXEL_LIGHT_IDX_CLEAR_VALUE 0xFFFFFFFFu
extern int32_t cursorPosition_x, cursorPosition_y;
extern float cam_yaw, cam_pitch, cam_roll;
extern uint16_t loadedModelsMaxIndex;
extern bool enteringPlayerName;
void Screenshot(void);
void ToggleConsole(void);
void ConsoleEmulator(int32_t keycode);
void SetSkyRotateSpeed(void);
// ----------------------------------------------------------------------------
// UI
#define TEXT_BUFFER_SIZE 1024
#define FONT_ATLAS_SIZE 4672
#define MAX_GLYPHS 4096
extern bool returnToPause;
extern char audiologNames[TEXT_LOGS_COUNT][TEXT_LOCALIZATION_MAX_LENGTH];
extern char audiologSubjects[TEXT_LOGS_COUNT][TEXT_LOCALIZATION_MAX_LENGTH];
extern char audiologSenders[TEXT_LOGS_COUNT][TEXT_LOCALIZATION_MAX_LENGTH];
extern char audioLogSpeech2Text[TEXT_LOGS_COUNT][TEXT_LOCALIZATION_MAX_LENGTH];
extern GLuint fontAtlasTex;
extern GLuint fontAtlasTexStopD;
extern float fixedNumberAdvanceWidth;
extern float fixedNumberAdvanceWidthStopD;
extern bool mouseMovementThisFrame;
extern char consoleEntryText[TEXT_BUFFER_SIZE];
void LoadTextForLanguage(uint8_t lang);
void LoadLogTextForLanguage(uint8_t lang);
int32_t CodepointToPackedIndex(int32_t codepoint, int32_t fontID);
void InitFontAtlasses(void);
// ----------------------------------------------------------------------------
// Helper Functions
#define DOUBLE_CLICK_TIME 0.5f
double get_time(void);
void Screenshot(void);
void CenterStatusPrint(const char* fmt, ...);
void DebugRAM(const char *context);
extern uint32_t random_range_rng;
double get_time(void);
void AddInstance(uint16_t entIdx, uint16_t instanceIdx);
uint32_t xs32(void);
uint8_t random_range_u8(uint8_t a, uint8_t b);
uint32_t random_range_u32(uint32_t a, uint32_t b);
int32_t random_range_i32(int32_t a, int32_t b);
float random_range(float a, float b);
double random_rangedub(double a, double b);
float lerp(float min, float max, float val);
float inverse_lerp(float min, float max, float val);
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
bool ConstIndexIsButtonSwitch(int constdex);
uint8_t GetCurrentLevelSecurity(void);
uint16_t GetImpactType(uint16_t instanceIdx);
int hardware14fromConstdex(int constdex);
const char* GetPrefabNameFromIndex(int constIndex);
static inline __attribute__((always_inline)) void CellCoordsToPos(uint16_t x, uint16_t z, float* pos_x, float* pos_z) {
    *pos_x = worldMin_x + (x * WORLDCELL_WIDTH_F);
    *pos_z = worldMin_z + (z * WORLDCELL_WIDTH_F);
}

static inline __attribute__((always_inline)) int32_t clamp(int32_t val, int32_t min, int32_t max) { return (val > max) ? max : ((val < min) ? min : val); }
static inline __attribute__((always_inline)) int32_t PosGetCellCoordX(float pos_x) { return (uint16_t)clamp((int32_t)vfloor((pos_x - worldMin_x + CELLXHALF) / WORLDCELL_WIDTH_F), 0, WORLDX_0BASED); }
static inline __attribute__((always_inline)) int32_t PosGetCellCoordZ(float pos_z) { return (uint16_t)clamp((int32_t)vfloor((pos_z - worldMin_z + CELLXHALF) / WORLDCELL_WIDTH_F), 0, WORLDX_0BASED); }
static inline __attribute__((always_inline)) int32_t PosGetCellCoords(float pos_x, float pos_z) { return (PosGetCellCoordZ(pos_z) * WORLDX) + PosGetCellCoordX(pos_x); } // Clamped just above.
static inline __attribute__((always_inline)) bool XZPairInBounds(int32_t x, int32_t z) { return (x < WORLDX && z < WORLDZ && x >= 0 && z >= 0); }

extern RenderSystem Sys_Render; // Added last to make use of all defines for sizes.

// Math, Vectors, Quaternions
static inline __attribute__((always_inline)) Vector3 Vector3_A_plus_B(Vector3 a, Vector3 b) { return (Vector3){a.x + b.x, a.y + b.y, a.z + b.z}; }
static inline __attribute__((always_inline)) Vector3 Vector3_A_minus_B(Vector3 a, Vector3 b) { return (Vector3){a.x - b.x, a.y - b.y, a.z - b.z}; }
static inline __attribute__((always_inline)) Vector3 scale_vector3(Vector3 v, float s) { Vector3 res = {v.x * s, v.y * s, v.z * s}; return res; }
static inline __attribute__((always_inline)) float dot(float x1, float y1, float z1, float x2, float y2, float z2) { return x1*x2 + y1*y2 + z1*z2; }
static inline __attribute__((always_inline)) float dot_vector3(Vector3 a, Vector3 b) { return dot(a.x,a.y,a.z, b.x,b.y,b.z); }
static inline __attribute__((always_inline)) float quat_dot(Quaternion a, Quaternion b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }
static inline __attribute__((always_inline)) float magnitude_vector3(const Vector3 v) { return vsqrtf(dot_vector3(v, v)); }
static inline __attribute__((always_inline)) Vector3 min_vector3(Vector3 a, Vector3 b) { return (Vector3){ a.x<b.x ? a.x : b.x, a.y<b.y ? a.y : b.y, a.z<b.z ? a.z : b.z }; }
static inline __attribute__((always_inline)) Vector3 max_vector3(Vector3 a, Vector3 b) { return (Vector3){ a.x>b.x ? a.x : b.x, a.y>b.y ? a.y : b.y, a.z>b.z ? a.z : b.z }; }
static inline __attribute__((always_inline)) float dist_sq_vector3(Vector3 a, Vector3 b) { Vector3 d = Vector3_A_minus_B(a, b); return dot_vector3(d, d); }
static inline __attribute__((always_inline)) float distance_vector3(Vector3 a, Vector3 b) { return magnitude_vector3(Vector3_A_minus_B(a, b)); }
static inline __attribute__((always_inline)) Vector3 cross_vector3(Vector3 a, Vector3 b) { return (Vector3){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }
static inline __attribute__((always_inline)) void normalize_vector(float* x, float* y, float* z) { float len = vsqrtf(*x * *x + *y * *y + *z * *z); if (len > 1e-6f) { *x /= len; *y /= len; *z /= len; } }
static inline __attribute__((always_inline)) Vector3 normalize_vector3(Vector3 v) { float len = magnitude_vector3(v); return len > 0.000001f ? (Vector3){v.x / len, v.y / len, v.z / len} : v; }
static inline __attribute__((always_inline)) float squareDistance2D(float x1, float z1, float x2, float z2) { float dx = x2 - x1; float dz = z2 - z1; return dx * dx + dz * dz; }
static inline __attribute__((always_inline)) float squareDistance3D(float x1, float y1, float z1, float x2, float y2, float z2) { float dx = x2 - x1; float dy = y2 - y1; float dz = z2 - z1; return dx * dx + dy * dy + dz * dz; }
uint16_t PointInSolid(Vector3 point, uint32_t layerMask);
bool EntityIsAnimated(uint16_t entIdx);

static inline __attribute__((always_inline)) float quat_angle_deg(Quaternion a, Quaternion b) {
    float d = vabs(quat_dot(a, b));
    if (d > 1.0f) d = 1.0f;
    return vacosf(d) * 2.0f * (180.0f / PI);
}

static inline __attribute__((always_inline)) Quaternion mul_quaternion(Quaternion a, Quaternion b) {
    return (Quaternion){
        a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z,
        a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y,
        a.w*b.y - a.x*b.z + a.y*b.w + a.z*b.x,
        a.w*b.z + a.x*b.y - a.y*b.x + a.z*b.w
    };
}

static inline __attribute__((always_inline)) Vector3 rotate_quaternion(Quaternion rotation, Vector3 axis) {
    Vector3 qv = {rotation.x, rotation.y, rotation.z}; // Take only the xyz, not w
    Vector3 uv = cross_vector3(qv, axis);
    return Vector3_A_plus_B(axis,Vector3_A_plus_B(scale_vector3(uv, 2.0f * rotation.w), scale_vector3(cross_vector3(qv, uv), 2.0f)));
}

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

bool parse_data_file(DataParser *parser, uint16_t maxSize, const char *filename);

extern uint16_t invalidModelIndexCount;
extern uint16_t entityCount;
extern uint16_t loadedInstances;
extern uint16_t startOfDoubleSidedInstances;
extern uint16_t startOfTransparentInstances;
extern uint16_t endOfModels;
void InitializeEntity(Entity* entry);
void LoadEntities(void);
void LoadLevel(uint8_t curlevel);

#define GEOMETRY_LOD_CARD_MODEL_IDX 178 // Need to specify in gamedata.txt
void EnableCheatArsenal(uint8_t level);
uint16_t SpawnDynamicObject(int val, bool cheat);
void cmd_kill(void);
void cmd_undo(void);
void cmd_shake(void);
float GetPainStatic(void);
Color GetPainStaticColor(void);
extern int currentMonitorIndex;
typedef struct { uint64_t magicNumber; double thisRunTime; bool isLoading; int missionSplitID; } AutoSplitterData;
extern AutoSplitterData autoSplitter;
void UpdateWhileNotPaused(uint16_t i);
void ScreenShake (float force, double duration);
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
extern uint32_t totalPixels;
extern uint32_t totalPaletteColors;
extern uint16_t loadedTexturesMaxIndex;
extern bool doubleSidedTexture[MAX_VALID_TEXTURE];
extern bool transparentTexture[MAX_VALID_TEXTURE];
void UpdateMusic(void);
void PlayMenuMusic(void);
void PlayGameMusic(void);

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
char* GetNextStringUpToNewlineOrEOF(char* buf, int size, long fd);
void WeaponsUpdate(void);

extern bool vmailActive;

// Interop - From Mod
void (*ModInit)(GlobalContext*,CheatsSystem*);
bool (*Forward)(void);
bool (*StrafeLeft)(void);
bool (*Backpedal)(void);
bool (*StrafeRight)(void);
bool (*Jump)(void);
bool (*JumpDown)(void);
bool (*Crouch)(void);
bool (*Prone)(void);
bool (*LeanLeft)(void);
bool (*LeanRight)(void);
bool (*Sprint)(void);
bool (*TurnLeft)(void);
bool (*TurnRight)(void);
bool (*LookUp)(void);
bool (*LookDown)(void);
bool (*RecentLog)(void);
bool (*Biomonitor)(void);
bool (*Sensaround)(void);
bool (*Lantern)(void);
bool (*Shield)(void);
bool (*Infrared)(void);
bool (*Email)(void);
bool (*Booster)(void);
bool (*Jumpjets)(void);
bool (*Attack)(void);
bool (*Use)(void);
bool (*Menu)(void);
bool (*ToggleMode)(void);
bool (*Reload)(void);
bool (*WeaponCycUp)(void);
bool (*WeaponCycDown)(void);
bool (*Grenade)(void);
bool (*GrenadeCycUp)(void);
bool (*GrenadeCycDown)(void);
bool (*ChangeAmmoType)(void);
bool (*Patch)(void);
bool (*PatchCycUp)(void);
bool (*PatchCycDown)(void);
bool (*Map)(void);
bool (*SwimUp)(void);
bool (*SwimDn)(void);
bool (*ChangeAmmoType)(void);
bool (*Console)(void);
float (*GetBasePlayerSpeed)(bool running);
void (*InitializeAIAfterLoad)(uint16_t i);
bool (*TakeScreenshot)(void);
