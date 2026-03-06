#pragma once
// #define DEBUG_RAM_OUTPUT // Debug and Compile Flags
#include "./External/glad/gl.h"
#include "./External/glfw3.h"
typedef __INT16_TYPE__ int16_t;
typedef __INT32_TYPE__ int32_t;
typedef __UINT8_TYPE__ uint8_t;
typedef __UINT16_TYPE__ uint16_t;
typedef __UINT32_TYPE__ uint32_t;
#define bool _Bool
#define true 1
#define false 0
#define INSTANCE_COUNT 10240 // Max 5454 for Citadel level 7 geometry, Max 295 for Citadel level 1 dynamic objects, 1561 lights, extras for dynamically spawned objects/lights

typedef uint64_t size_t;
typedef struct { float r,g,b,a; } Color;
typedef struct { float x,y; } Vector2;
typedef struct { float x,y,z; } Vector3;
typedef struct { float x,y,z,w; } Quaternion;
typedef uint8_t PhysCombineType;
typedef uint8_t ColliderType;
typedef uint16_t Text;

#define SAVE_REMINDER_TIME 7.0f // 7secs ~is human short-term memory length
#define CREDITS_PAGES 22
typedef struct {
    GLFWwindow* window;
	bool inventoryMode;
	double last_time;
	double last_topframe_time;
	double last_physics_time;
    double deltaTime;
	double current_time;
	double timeSinceLastPhysicsTick;
	double screenshotTimeout;
	double pauseRelativeTime;
	double absoluteTime;
	double statusTextDecayFinished;
    double justSavedTimeStamp;
	bool levelCurrentlyLoading;
    double shakeFinished;
	char global_modname[256];
	bool global_modIsCitadel;
    bool introNotPlayed;
    uint8_t levelSecurity[14];
	uint8_t startLevel;
	uint8_t numLevels; // Can be set by gamedata.txt
	uint8_t currentLevel;
	uint8_t difficultyCombat;
	uint8_t difficultyPuzzle;
	uint8_t difficultyMission;
	uint8_t difficultyCyber;
	bool gamePaused;
	bool menuActive;
    bool gameFinished;
	uint16_t ressurections;
	uint16_t deaths;
	uint16_t kills;
	uint16_t cyberkills;
	uint32_t shotsFired;
	uint32_t grenadesThrown;
	float damageDealt;
	float damageReceived;
	uint32_t savesScummed;
	uint8_t creditsPageIndex;
	bool creditsActive;
    bool decoyActive;
	char playerName[27];
} GlobalContext;
extern GlobalContext Sys_Global;

typedef struct {
	int32_t InputCodeSettings[42];
	uint16_t ScreenWidth;
	uint16_t ScreenHeight;
    float ScreenCenterX;
    float ScreenCenterY;
	bool Fullscreen;
	uint8_t FOV;
	uint8_t Brightness;
	uint8_t Gamma;
	uint8_t AntiAliasing;
	uint8_t Shadows;
	uint8_t Reflections;
	uint8_t Vsync;
	uint8_t ModelDetail;
	uint8_t GI;
	uint8_t SpeakerMode;
	uint8_t Reverb;
	uint8_t VolumeMaster;
	uint8_t VolumeMusic;
	uint8_t VolumeMessage;
	uint8_t VolumeEffects;
	uint8_t Language;
	uint8_t DynamicMusic;
	uint8_t Footsteps;
	uint8_t InvertLook;
	uint8_t InvertInventoryCycling;
	uint8_t InvertCyberspaceLook;
	uint8_t QuickItemPickup;
	uint8_t QuickReloadWeapons;
	uint8_t MouseSensitivity;
	uint8_t NoShootMode;
	uint8_t HeadBob;
	uint8_t SSR_RES;
} SettingsSystem;
extern SettingsSystem Sys_Settings;

typedef struct {
    bool inCombat;
    bool inZone;
    bool twoPlaying;
    double clipFinished;
    double combatImpulseFinished;
    bool distortion;
    bool cyberTube;
    bool elevator;
    bool levelEntry;
} MusicSystem;
extern MusicSystem Sys_Music;

typedef struct { bool down; bool pressed; bool released; } KeyState;

#define MAX_KEYS 512
#define MAX_MOUSE_BUTTONS 8
#define MAX_JOYSTICK_BUTTONS 16
#define MAX_JOYSTICK_HATS 5
#define MAX_GAMEPAD_BUTTONS 20
typedef struct {
	KeyState keyStates[MAX_KEYS];
	KeyState mouseButtons[MAX_MOUSE_BUTTONS];
	KeyState gamepadButtons[MAX_GAMEPAD_BUTTONS];
	bool joystickPresent[GLFW_JOYSTICK_LAST + 1];
	KeyState joystickButtons[GLFW_JOYSTICK_LAST + 1][MAX_JOYSTICK_BUTTONS];
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

#define LIGHT_COUNT 1600 // MAX CITADEL LIGHT COUNT is 1561 for Level 7, leaves room for dynamic lights from projectiles
#define MAX_SHADOWMAPS 80u
#define SHADOW_MAP_SIZE 192u
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
} CheatsSystem;
extern CheatsSystem Sys_Cheats;

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

// BodyState
typedef uint8_t BodyState;
static const uint8_t BodyState_Standing = 0;
static const uint8_t BodyState_Crouch = 1;
static const uint8_t BodyState_CrouchingDown = 2;
static const uint8_t BodyState_StandingUp = 3;
static const uint8_t BodyState_Prone = 4;
static const uint8_t BodyState_ProningDown = 5;
static const uint8_t BodyState_ProningUp = 6;

// Handedness
typedef uint8_t Handedness;
static const uint8_t Handedness_Center = 0;
static const uint8_t Handedness_LH = 1;
static const uint8_t Handedness_RH = 2;

// AttackType
typedef uint8_t AttackType;
static const uint8_t AttackType_None = 0;
static const uint8_t AttackType_Melee = 1;
static const uint8_t AttackType_MeleeEnergy = 2;
static const uint8_t AttackType_EnergyBeam = 3;
static const uint8_t AttackType_Magnetic = 4;
static const uint8_t AttackType_Projectile = 5;
static const uint8_t AttackType_ProjectileNeedle = 6;
static const uint8_t AttackType_ProjectileEnergyBeam = 7;
static const uint8_t AttackType_ProjectileLaunched = 8;
static const uint8_t AttackType_Gas = 9;
static const uint8_t AttackType_Tranq = 10;
static const uint8_t AttackType_Drill = 11;

// NPCType
typedef uint8_t NPCType;
static const uint8_t NPCType_Mutant = 0;
static const uint8_t NPCType_Supermutant = 1;
static const uint8_t NPCType_Robot = 2;
static const uint8_t NPCType_Cyborg = 3;
static const uint8_t NPCType_Supercyborg = 4;
static const uint8_t NPCType_Cyber = 5;
static const uint8_t NPCType_MutantCyborg = 6;

// PerceptionLevel
typedef uint8_t PerceptionLevel;
static const uint8_t PerceptionLevel_Low = 0;
static const uint8_t PerceptionLevel_Medium = 1;
static const uint8_t PerceptionLevel_High = 2;
static const uint8_t PerceptionLevel_Omniscient = 3;

// AIState
typedef uint8_t AIState;
static const uint8_t AIState_Idle = 0;
static const uint8_t AIState_Walk = 1;
static const uint8_t AIState_Run = 2;
static const uint8_t AIState_Attack1 = 3;
static const uint8_t AIState_Attack2 = 4;
static const uint8_t AIState_Attack3 = 5;
static const uint8_t AIState_Pain = 6;
static const uint8_t AIState_Dying = 7;
static const uint8_t AIState_Dead = 8;
static const uint8_t AIState_Inspect = 9;
static const uint8_t AIState_Interacting = 10;

// AIMoveType
typedef uint8_t AIMoveType;
static const uint8_t AIMoveType_Walk = 0;
static const uint8_t AIMoveType_Fly = 1;
static const uint8_t AIMoveType_Swim = 2;
static const uint8_t AIMoveType_Cyber = 3;
static const uint8_t AIMoveType_None = 4;

// DoorState
typedef uint8_t DoorState;
static const uint8_t DoorState_Closed = 0;
static const uint8_t DoorState_Open = 1;
static const uint8_t DoorState_Closing = 2;
static const uint8_t DoorState_Opening = 3;

// FuncStates
typedef uint8_t FuncStates;
static const uint8_t FuncStates_Start = 0;
static const uint8_t FuncStates_Target = 1;
static const uint8_t FuncStates_MovingStart = 2;
static const uint8_t FuncStates_MovingTarget = 3;
static const uint8_t FuncStates_AjarMovingStart = 4;
static const uint8_t FuncStates_AjarMovingTarget = 5;

// SoftwareType
typedef uint8_t SoftwareType;
static const uint8_t SoftwareType_None = 0;
static const uint8_t SoftwareType_Drill = 1;
static const uint8_t SoftwareType_Pulser = 2;
static const uint8_t SoftwareType_CShield = 3;
static const uint8_t SoftwareType_Decoy = 4;
static const uint8_t SoftwareType_Recall = 5;
static const uint8_t SoftwareType_Turbo = 6;
static const uint8_t SoftwareType_Game = 7;
static const uint8_t SoftwareType_Data = 8;
static const uint8_t SoftwareType_Integrity = 9;
static const uint8_t SoftwareType_Keycard = 10;

// AccessCardType
typedef uint8_t AccessCardType;
static const uint8_t AccessCardType_None = 0;
static const uint8_t AccessCardType_Standard = 1;
static const uint8_t AccessCardType_Medical = 2;
static const uint8_t AccessCardType_Science = 3;
static const uint8_t AccessCardType_Admin = 4;
static const uint8_t AccessCardType_Group1 = 5;
static const uint8_t AccessCardType_Group2 = 6;
static const uint8_t AccessCardType_Group3 = 7;
static const uint8_t AccessCardType_Group4 = 8;
static const uint8_t AccessCardType_GroupA = 9;
static const uint8_t AccessCardType_GroupB = 10;
static const uint8_t AccessCardType_Storage = 11;
static const uint8_t AccessCardType_Engineering = 12;
static const uint8_t AccessCardType_Maintenance = 13;
static const uint8_t AccessCardType_Security = 14;
static const uint8_t AccessCardType_Per1 = 15;
static const uint8_t AccessCardType_Per2 = 16;
static const uint8_t AccessCardType_Per3 = 17;
static const uint8_t AccessCardType_Per4 = 18;
static const uint8_t AccessCardType_Per5 = 19;

// MusicType
typedef uint8_t MusicType;
static const uint8_t MusicType_None     = 0;
static const uint8_t MusicType_Walking  = 1;
static const uint8_t MusicType_Combat   = 2;
static const uint8_t MusicType_Override = 3;

// TrackType
typedef uint8_t TrackType;
static const uint8_t TrackType_None       = 0;
static const uint8_t TrackType_Walking    = 1;
static const uint8_t TrackType_Combat     = 2;
static const uint8_t TrackType_Revive     = 3;
static const uint8_t TrackType_Death      = 4;
static const uint8_t TrackType_Cybertube  = 5;
static const uint8_t TrackType_Elevator   = 6;
static const uint8_t TrackType_Distortion = 7;

// BloodType
typedef uint8_t BloodType;
static const uint8_t BloodType_None = 0;
static const uint8_t BloodType_Red = 1;
static const uint8_t BloodType_Yellow = 2;
static const uint8_t BloodType_Green = 3;
static const uint8_t BloodType_Robot = 4;
static const uint8_t BloodType_Leaf = 5;
static const uint8_t BloodType_Mutation = 6;
static const uint8_t BloodType_GrayMutation = 7;

// SecurityType
typedef uint8_t SecurityType;
static const uint8_t SecurityType_None = 0;
static const uint8_t SecurityType_Camera = 1;
static const uint8_t SecurityType_NodeSmall = 2;
static const uint8_t SecurityType_NodeLarge = 3;

// AudioLogType
typedef uint8_t AudioLogType;
static const uint8_t AudioLogType_TextOnly = 0;
static const uint8_t AudioLogType_Normal = 1;
static const uint8_t AudioLogType_Email = 2;
static const uint8_t AudioLogType_Papers = 3;
static const uint8_t AudioLogType_Vmail = 4;
static const uint8_t AudioLogType_Game = 5;

// EnergyType
typedef uint8_t EnergyType;
static const uint8_t EnergyType_Battery = 0;
static const uint8_t EnergyType_ChargeStation = 1;

// FootStepType
typedef uint8_t FootStepType;
static const uint8_t FootStepType_None = 0;
static const uint8_t FootStepType_Carpet = 1;
static const uint8_t FootStepType_Concrete = 2;
static const uint8_t FootStepType_GrittyCrete = 3;
static const uint8_t FootStepType_Grass = 4;
static const uint8_t FootStepType_Gravel = 5;
static const uint8_t FootStepType_Rock = 6;
static const uint8_t FootStepType_Glass = 7;
static const uint8_t FootStepType_Marble = 8;
static const uint8_t FootStepType_Metal = 9;
static const uint8_t FootStepType_Grate = 10;
static const uint8_t FootStepType_Metal2 = 11;
static const uint8_t FootStepType_Metpanel = 12;
static const uint8_t FootStepType_Panel = 13;
static const uint8_t FootStepType_Plaster = 14;
static const uint8_t FootStepType_Plastic = 15;
static const uint8_t FootStepType_Plastic2 = 16;
static const uint8_t FootStepType_Rubber = 17;
static const uint8_t FootStepType_Sand = 18;
static const uint8_t FootStepType_Squish = 19;
static const uint8_t FootStepType_Vent = 20;
static const uint8_t FootStepType_Water = 21;
static const uint8_t FootStepType_Wood = 22;
static const uint8_t FootStepType_Wood2 = 23;

// MusicResourceType
typedef uint8_t MusicResourceType;
static const uint8_t MusicResourceType_Menu = 0;
static const uint8_t MusicResourceType_Medical = 1;
static const uint8_t MusicResourceType_Science = 2;
static const uint8_t MusicResourceType_Reactor = 3;
static const uint8_t MusicResourceType_Executive = 4;
static const uint8_t MusicResourceType_Grove = 5;
static const uint8_t MusicResourceType_Cyber = 6;
static const uint8_t MusicResourceType_Security = 7;
static const uint8_t MusicResourceType_Revive = 8;
static const uint8_t MusicResourceType_Death = 9;
static const uint8_t MusicResourceType_Elevator = 10;
static const uint8_t MusicResourceType_Distortion = 11;
static const uint8_t MusicResourceType_Looped = 12;
static const uint8_t MusicResourceType_Level = 13;

// HUDColor
typedef uint8_t HUDColor;
static const uint8_t HUDColor_White = 0;
static const uint8_t HUDColor_Red = 1;
static const uint8_t HUDColor_Orange = 2;
static const uint8_t HUDColor_Yellow = 3;
static const uint8_t HUDColor_Green = 4;
static const uint8_t HUDColor_Blue = 5;
static const uint8_t HUDColor_Purple = 6;
static const uint8_t HUDColor_Gray = 7;

// ForceFieldColor
typedef uint8_t ForceFieldColor;
static const uint8_t ForceFieldColor_Red = 0;
static const uint8_t ForceFieldColor_Green = 1;
static const uint8_t ForceFieldColor_Blue = 2;
static const uint8_t ForceFieldColor_Purple = 3;
static const uint8_t ForceFieldColor_RedFaint = 4;

// ButtonType
typedef uint8_t ButtonType;
static const uint8_t ButtonType_Generic = 0;
static const uint8_t ButtonType_GeneralInv = 1;
static const uint8_t ButtonType_Patch = 2;
static const uint8_t ButtonType_Grenade = 3;
static const uint8_t ButtonType_Weapon = 4;
static const uint8_t ButtonType_Search = 5;
static const uint8_t ButtonType_None = 6;
static const uint8_t ButtonType_PGrid = 7;
static const uint8_t ButtonType_PWire = 8;
static const uint8_t ButtonType_Vaporize = 9;
static const uint8_t ButtonType_ShootMode = 10;
static const uint8_t ButtonType_GrenadeTimerSlider = 11;

// TabMSG
typedef uint8_t TabMSG;
static const uint8_t TabMSG_None = 0;
static const uint8_t TabMSG_Search = 1;
static const uint8_t TabMSG_AudioLog = 2;
static const uint8_t TabMSG_Keypad = 3;
static const uint8_t TabMSG_Elevator = 4;
static const uint8_t TabMSG_GridPuzzle = 5;
static const uint8_t TabMSG_WirePuzzle = 6;
static const uint8_t TabMSG_EReader = 7;
static const uint8_t TabMSG_Weapon = 8;
static const uint8_t TabMSG_SystemAnalyzer = 9;

// PuzzleCellType
typedef uint8_t PuzzleCellType;
static const uint8_t PuzzleCellType_Off = 0;
static const uint8_t PuzzleCellType_Standard = 1;
static const uint8_t PuzzleCellType_And = 2;
static const uint8_t PuzzleCellType_Bypass = 3;

// PuzzleGridType
typedef uint8_t PuzzleGridType;
static const uint8_t PuzzleGridType_King = 0;
static const uint8_t PuzzleGridType_Queen = 1;
static const uint8_t PuzzleGridType_Knight = 2;
static const uint8_t PuzzleGridType_Rook = 3;
static const uint8_t PuzzleGridType_Bishop = 4;
static const uint8_t PuzzleGridType_Pawn = 5;

typedef uint8_t PhysicsLayer;
static const uint8_t PhysicsLayer_Default          = 0;
static const uint8_t PhysicsLayer_TransparentFX    = 1;
static const uint8_t PhysicsLayer_IgnoreRaycast    = 2;
// static const uint8_t PhysicsLayer_              = 3;
static const uint8_t PhysicsLayer_Water            = 4;
static const uint8_t PhysicsLayer_BlocksRaycast    = 4;
static const uint8_t PhysicsLayer_UI               = 5;
//static const uint8_t PhysicsLayer_               = 6;
//static const uint8_t PhysicsLayer_               = 7;
static const uint8_t PhysicsLayer_GunViewModel     = 8;
static const uint8_t PhysicsLayer_Geometry         = 9;
static const uint8_t PhysicsLayer_NPC              = 10;
static const uint8_t PhysicsLayer_PlayerBullets    = 11;
static const uint8_t PhysicsLayer_Player           = 12;
static const uint8_t PhysicsLayer_Corpse           = 13;
static const uint8_t PhysicsLayer_PhysObjects      = 14;
static const uint8_t PhysicsLayer_Sky              = 15;
static const uint8_t PhysicsLayer_PlayerTriggerOnly= 16;
static const uint8_t PhysicsLayer_Trigger          = 17;
static const uint8_t PhysicsLayer_Door             = 18;
static const uint8_t PhysicsLayer_InterDebris      = 19;
static const uint8_t PhysicsLayer_Player2          = 20;
static const uint8_t PhysicsLayer_Player3          = 21;
static const uint8_t PhysicsLayer_Player4          = 22;
static const uint8_t PhysicsLayer_NPCTrigger       = 23;
static const uint8_t PhysicsLayer_NPCBullet        = 24;
static const uint8_t PhysicsLayer_NPCClip          = 25;
static const uint8_t PhysicsLayer_Clip             = 26;
static const uint8_t PhysicsLayer_Automap          = 27;
static const uint8_t PhysicsLayer_Culling          = 28;
static const uint8_t PhysicsLayer_CorpseSearchable = 29;
//static const uint8_t PhysicsLayer_               = 30;
static const uint8_t PhysicsLayer_NULL             = 31;

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
extern SystemUI Sys_UI;

#define MODEL_IDX_MAX 6805
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

extern uint16_t useableItemsFrobIcons[94];

#define NUM_AI_TYPES 29
typedef struct {
	const char* name;
	AttackType attackType;
	AttackType attackType2;
	AttackType attackType3;
	float damage;
	float damage2;
	float damage3;
	float range;
	float range2;
	float range3;
	float health;
	float healthForCyberNPC;
	PerceptionLevel perception;
	float disruptability;
	float armorvalue;
	float defense;
	AIMoveType moveType;
	float yawSpeed;
	float fov;
	float fovAttack;
	float fovStartMovement;
	float distToSeeBehind;
	float sightRange;
	float walkSpeed;
	float runSpeed;
	float attack1Speed;
	float attack2Speed;
	float attack3Speed;
	float attack3Force;
	float attack3Radius;
	float timeToPain;
	float timeBetweenPain;
	float timeTillDead;
	float timeToActualAttack1;
	float timeToActualAttack2;
	float timeToActualAttack3;
	float timeBetweenAttack1;
	float timeBetweenAttack2;
	float timeBetweenAttack3;
	float timeToChangeEnemy;
	float timeIdleSFXMin;
	float timeIdleSFXMax;
	float timeAttack1WaitMin;
	float timeAttack1WaitMax;
	float timeAttack1WaitChance;
	float timeAttack2WaitMin;
	float timeAttack2WaitMax;
	float timeAttack2WaitChance;
	float timeAttack3WaitMin;
	float timeAttack3WaitMax;
	float timeAttack3WaitChance;
	int attack1ProjectileLaunchedType; // Unused
	int attack2ProjectileLaunchedType; // Unused
	int attack3ProjectileLaunchedType; // Unused
	float projectileSpeedAttack1;
	float projectileSpeedAttack2;
	float projectileSpeedAttack3;
	bool hasLaserOnAttack1;
	bool hasLaserOnAttack2;
	bool hasLaserOnAttack3;
	bool explodeOnAttack3;
	bool preactivateMeleeColliders;
	double huntTime;
	float flightHeight;
	bool flightHeightIsPercentage;
	bool switchMaterialOnDeath;
	float hearingRange;
	float timeForTranquilization;
	bool hopsOnMove;
	NPCType type;
	int projectile1Prefab;
	int projectile2Prefab;
	int projectile3Prefab;
} NPCTable;
extern NPCTable npcTable[NUM_AI_TYPES];

#define TARGET_ID_LENGTH 32 // Max needed 22 + 5 for ID + 1 for space between them = 28
#define MAX_WAYPOINTS 32
#define MAX_CHILD_COUNT 4
#define MAX_ENTITIES 768 // Unique entity types, different than INSTANCE_COUNT which is the number of instances of any of these entities.
#define NULLENT 0u
#define WORLD   0u // Much like Quake, the world is entity 0.  Aand also like Quake, world is nullent and is 0.
#define PLAYER1 1u
#define PLAYER2 2u
#define START_INDEX_LEVEL_INSTANCES 3
#define ENTFLAG_ACTIVE               (1ull <<  0) // Instance renders and updates
#define ENTFLAG_CARDCHUNK            (1ull <<  1)
#define ENTFLAG_GROUNDED             (1ull <<  2)
#define ENTFLAG_USEGRAVITY           (1ull <<  3)
#define ENTFLAG_KINEMATIC            (1ull <<  4)
#define ENTFLAG_RIGIDBODY            (1ull <<  5)
#define ENTFLAG_DOUBLESIDED          (1ull <<  6)
#define ENTFLAG_TRANSPARENT          (1ull <<  7)
#define ENTFLAG_CHANGE_TEX_ON_ACTIVE (1ull <<  8)
#define ENTFLAG_BLINK_TEX_ON_ACTIVE  (1ull <<  9)
#define ENTFLAG_NO_SHADOWS           (1ull << 10)
#define ENTFLAG_ANIMATED             (1ull << 11)
#define ENTFLAG_ASLEEP               (1ull << 12) // Check if enemy starts out asleep such as the sleeping sec-2 bots on level 8 in the maintenance and recharge bays.
#define ENTFLAG_WALK_PATH_ON_START   (1ull << 13)
#define ENTFLAG_TEST_PERSISTENT      (1ull << 14)
#define ENTFLAG_TEST_OVERRIDE_TEST   (1ull << 15)
#define ENTFLAG_TOUCHING_HURTS       (1ull << 16)
#define ENTFLAG_ACT_AS_CORPSE_ONLY   (1ull << 17)
#define ENTFLAG_DYING                (1ull << 18)
#define ENTFLAG_DEATH_BURST_DONE     (1ull << 19)
#define ENTFLAG_DEAD                 (1ull << 20)
#define ENTFLAG_TELEPORT_ON_DEATH    (1ull << 21)
#define ENTFLAG_GO_INTO_PAIN         (1ull << 22)
#define ENTFLAG_DONT_LOOP_WAYPTS     (1ull << 23)
#define ENTFLAG_VISIT_WAYPTS_RND     (1ull << 24)
#define ENTFLAG_WANDERING            (1ull << 25)
#define ENTFLAG_ACT_AS_TURRET        (1ull << 26)
#define ENTFLAG_TARGID_ATTACHED      (1ull << 27)
#define ENTFLAG_ENEM_IN_SIGHT        (1ull << 28)
#define ENTFLAG_ENEM_IN_FRONT        (1ull << 29)
#define ENTFLAG_ENEM_IN_FOV          (1ull << 30)
#define ENTFLAG_ENEM_IN_LOS          (1ull << 31)
#define ENTFLAG_FIRST_SIGHTING       (1ull << 32)
#define ENTFLAG_DYING_SETUP          (1ull << 33)
#define ENTFLAG_HAD_ENEMY            (1ull << 34)
#define ENTFLAG_SHOT_FIRED           (1ull << 35)
#define ENTFLAG_DEAD_CHECKS_DONE     (1ull << 36)
#define ENTFLAG_HOP_DONE             (1ull << 37)
#define ENTFLAG_LOCKED               (1ull << 38)
#define ENTFLAG_HAS_CAMERA_VIEW      (1ull << 39)
#define ENTFLAG_REQUIRE_RESET        (1ull << 40)
#define ENTFLAG_GRAV_LIFT_STATE      (1ull << 41)
#define ENTFLAG_STOPSOUND_PLAYED     (1ull << 42)
#define ENTFLAG_DAMAGE_ON_USE        (1ull << 43)
#define ENTFLAG_MAKING_NOISE         (1ull << 44)
#define ENTFLAG_ENABLED              (1ull << 45) // Instance updates
#define ENTFLAG_ACTIVATED            (1ull << 46) // E.g. forcebridge visible, switch is flipped
#define ENTFLAG_VISIBLE              (1ull << 47) // Renders
#define QUESTBIT_ROBOT_SPAWN_DEACTIVATED      (1ull <<  0)
#define QUESTBIT_ISOTOPE_INSTALLED            (1ull <<  1)
#define QUESTBIT_SHIELD_ACTIVATED             (1ull <<  2)
#define QUESTBIT_LASER_SAFETY_OVERRIDEN       (1ull <<  3)
#define QUESTBIT_LASER_DESTROYED              (1ull <<  4)
#define QUESTBIT_BETA_GROVE_CYBER_UNLOCKED    (1ull <<  5)
#define QUESTBIT_GROVE_ALPHA_JETTISON_ENABLED (1ull <<  6)
#define QUESTBIT_GROVE_BETA_JETTISON_ENABLED  (1ull <<  7)
#define QUESTBIT_GROVE_DELTA_JETTISON_ENABLED (1ull <<  8)
#define QUESTBIT_MASTER_JETTISON_BROKEN       (1ull <<  9)
#define QUESTBIT_RELAY_428_FIXED              (1ull << 10)
#define QUESTBIT_MASTER_JETTISON_ENABLED      (1ull << 11)
#define QUESTBIT_BETA_GROVE_JETTISONED        (1ull << 12)
#define QUESTBIT_ANTENNA_NORTH_DESTROYED      (1ull << 13)
#define QUESTBIT_ANTENNA_SOUTH_DESTROYED      (1ull << 14)
#define QUESTBIT_ANTENNA_EAST_DESTROYED       (1ull << 15)
#define QUESTBIT_ANTENNA_WEST_DESTROYED       (1ull << 16)
#define QUESTBIT_SELF_DESTRUCT_ACTIVATED      (1ull << 17)
#define QUESTBIT_BRIDGE_SEPARATED             (1ull << 18)
#define QUESTBIT_ISOLINEAR_CHIPSET_INSTALLED  (1ull << 19)
#define QUESTBIT_LEV1_CODE_LOCKED             (1ull << 20)
#define QUESTBIT_LEV2_CODE_LOCKED             (1ull << 21)
#define QUESTBIT_LEV3_CODE_LOCKED             (1ull << 22)
#define QUESTBIT_LEV4_CODE_LOCKED             (1ull << 23)
#define QUESTBIT_LEV5_CODE_LOCKED             (1ull << 24)
#define QUESTBIT_LEV6_CODE_LOCKED             (1ull << 25)
#define TARG_IOFLAGS_TRIPTRIGGER        (1ull << 0) // Action bits.  What do we want our target to do, e.g. turn on a light or close a door or activate force bridge.  Using multiple bools to allow for multiple actions to be attempted on all the targets.
#define TARG_IOFLAGS_DOOROPEN           (1ull << 1)
#define TARG_IOFLAGS_DOOROPENIFUNLOCKED (1ull << 2)
#define TARG_IOFLAGS_DOORCLOSE          (1ull << 3)
#define TARG_IOFLAGS_DOORLOCK           (1ull << 4)
#define TARG_IOFLAGS_DOORUNLOCK         (1ull << 5)
#define TARG_IOFLAGS_SWITCHTRIGGER      (1ull << 6)
#define TARG_IOFLAGS_CHGSTAT_RECHARGE   (1ull << 7)
#define TARG_IOFLAGS_ENEMY_ALERT        (1ull << 8)
#define TARG_IOFLAGS_FBRIDGE_ACTIVATE   (1ull << 9)
#define TARG_IOFLAGS_FBRIDGE_DEACTIVATE (1ull << 10)
#define TARG_IOFLAGS_FBRIDGE_TOGGLE     (1ull << 11)
#define TARG_IOFLAGS_GRAVLIFT_TOGGLE    (1ull << 12)
#define TARG_IOFLAGS_TEXTURE_CHG_TOGGLE (1ull << 13)
#define TARG_IOFLAGS_LIGHT_ON           (1ull << 14)
#define TARG_IOFLAGS_LIGHT_OFF          (1ull << 15)
#define TARG_IOFLAGS_LIGHT_TOGGLE       (1ull << 16)
#define TARG_IOFLAGS_FUNCWALL_MOVE      (1ull << 17)
#define TARG_IOFLAGS_MISSION_BIT_ON     (1ull << 18)
#define TARG_IOFLAGS_MISSION_BIT_OFF    (1ull << 19)
#define TARG_IOFLAGS_MISSION_BIT_TOGGLE (1ull << 20)
#define TARG_IOFLAGS_TXFER2LOGIC_RELAY  (1ull << 21)
#define TARG_IOFLAGS_SEND_EMAIL         (1ull << 22)
#define TARG_IOFLAGS_SWITCH_LOCK_TOGGLE (1ull << 23)
#define TARG_IOFLAGS_LOCK_CODE_SCREEN   (1ull << 24)
#define TARG_IOFLAGS_SPAWNER_ACTIVATE   (1ull << 25)
#define TARG_IOFLAGS_SPAWNER_ACTALERTED (1ull << 26)
#define TARG_IOFLAGS_CYBORG_CONV_TOGGLE (1ull << 27)
#define TARG_IOFLAGS_INST_ACTIVATE      (1ull << 28)
#define TARG_IOFLAGS_INST_DEACTIVATE    (1ull << 29)
#define TARG_IOFLAGS_INST_TOGGLE        (1ull << 30)
#define TARG_IOFLAGS_TOGGLE_RADIATION   (1ull << 31)
#define TARG_IOFLAGS_TOGGLE_PUZPNL_LOCK (1ull << 32)
#define TARG_IOFLAGS_TEST_QUESTBIT_ON   (1ull << 33)
#define TARG_IOFLAGS_TEST_QUESTBIT_OFF  (1ull << 34)
#define TARG_IOFLAGS_PLAY_SOUND_ONCE    (1ull << 35)
#define TARG_IOFLAGS_STOP_SOUND         (1ull << 36)
#define TARG_IOFLAGS_SEND_CENTERPRINT   (1ull << 37)
#define TARG_IOFLAGS_RADIATION_TREATMNT (1ull << 38)
#define TARG_IOFLAGS_START_FLASHING_TEX (1ull << 39)
#define TARG_IOFLAGS_STOP_FLASHING_TEX  (1ull << 40)
#define TARG_IOFLAGS_UNLOCK_ELEVATORPAD (1ull << 41)
#define TARG_IOFLAGS_UNLOCK_KEYPAD      (1ull << 42)
#define TARG_IOFLAGS_UNLOCK_PUZPAD      (1ull << 43)
#define TARG_IOFLAGS_SCREENSHAKE        (1ull << 44)
#define TARG_IOFLAGS_AWAKE_SLEEPING_NPC (1ull << 45)
#define TARG_IOFLAGS_BRANCH_FLIP        (1ull << 46)
#define TARG_IOFLAGS_BRANCH_FLIPONLY    (1ull << 47)
#define TARG_IOFLAGS_TOG_DORACESOVERIDE (1ull << 48)
#define TARG_IOFLAGS_UNLOCK_SWITCH      (1ull << 49)
#define TARG_IOFLAGS_LOCK_ELEVATORPAD   (1ull << 50)
#define TARG_IOFLAGS_DOOR_TOGGLE        (1ull << 51)
#define TARG_IOFLAGS_ONCE_EVER          (1ull << 52)
#define TARG_IOFLAGS_ALREADY_DONE       (1ull << 53)
#define TARG_IOFLAGS_START_ON_SECOND    (1ull << 54)
#define TARG_IOFLAGS_ON_SECOND          (1ull << 55) // No he's on third
#define TARG_IOFLAGS_AUTOFLIP_ON_TARGET (1ull << 56)
#define TARG_IOFLAGS_DISABLE_ON_AWAKE   (1ull << 57)
#define TARG_IOFLAGS_DISABLD_ONCE_4EVER (1ull << 58)
#define TARGET_STRING_LENGTH 40
static inline __attribute__((always_inline)) void flag_set(uint64_t *flags, uint64_t bit, bool state) { *flags = (*flags & ~bit) | (-state & bit); }
typedef /*FAT*/ struct {
    uint64_t entflags;
    uint64_t ioflags;
    uint16_t index; // constIndex for entity type, used for indexing into arrays for resourec types when loading resources
    
    // Logic and I/O
    char targetname[TARGET_STRING_LENGTH];
    char target[TARGET_STRING_LENGTH];
    char target2[TARGET_STRING_LENGTH];
    char currenttarget[TARGET_STRING_LENGTH];
    char targetIfFalse[TARGET_STRING_LENGTH];
    uint8_t securityThreshold;
    uint16_t messageIndex;
    float delay;
    
    uint16_t activateSFX;
    uint16_t lockedSFX;
    int lev1SecCode;
    int lev2SecCode;
    int lev3SecCode;
    int lev4SecCode;
    int lev5SecCode;
    int lev6SecCode;
    float health;
    float lastHealth;
    float cyberHealth;
    float energy;
    float maxEnergy;
    uint16_t lockedMessageLingdex;
    double delayFinished;
    double tickFinished;
    double tickTime;
    float amount;
    float resetTime;
    float minSecurityLevel;
    BloodType bloodType;
    DoorState doorOpen;
    ForceFieldColor fieldColor;
    bool lerping;
    TrackType trackType;
    MusicType musicType;
    
    // Player
    float radiated;
    float resetAfterDeathTime;
    float playerHealthTimer;
    uint16_t patchActive;
    uint16_t drainJPM;
    double berserkFinishedTime;
    double berserkIncrementFinishedTime;
    double detoxFinishedTime;
    double geniusFinishedTime;
    double mediFinishedTime;
    double reflexFinishedTime;
    double sightFinishedTime;
    double sightSideEffectFinishedTime;
    double staminupFinishedTime;
    int berserkIncrement;
    double turboCyberTime;
    double turboFinished;
    double energyDrainTickFinished;
    double noiseFinished;
    double painSoundFinished;
    double radSoundFinished;
    double radFXFinished;
    float radAdjust;
    float initialRadiation;
    int32_t heldObjectIndex;
    bool playerDead;

    // Rendering
    uint16_t modelIndex;
    uint16_t texIndex;
    uint16_t altTexIndex;
    uint16_t glowIndex;
    uint16_t altGlowIndex;
    uint16_t specIndex;
    uint16_t normIndex;
    uint16_t lodIndex;

    // Animation
    uint8_t clip;
    uint8_t numclips;
    uint16_t animationNum; // Global animation identifier into short table of AnimationClip's
    uint16_t frame; // 0 based index into the delta tables, 0 skips delta read and just uses raw modelIndex base pos verts with no delta applied.
    int32_t cellIndex;
    uint8_t portalIndex; // If this is a door, index into portal array for toggling state.
    DoorState doorState;
    double currentFrameFinished;
    double currentFrameStartTime;
    double animSwapFinished;
    bool alternateOn;
    uint16_t mainSwitchMaterial;
    uint16_t alternateSwitchMaterial;

    // Physics
    Vector3 position;
    Vector3 lastPosition;
    Quaternion rotation;
    Vector3 scale;
    Vector3 forward;
    Vector3 right;
    Vector3 velocity;
    Vector3 angularVelocity;
    float gravity;
    BodyState bodyState;
    uint32_t layer;
    ColliderType collider;
    Vector3 colliderCenter; // Offset relative to .position's global worldspace xyz location
    Vector3 colliderSize; // x,y,z for Box, x for Sphere radius, else x, y, z for Capsule radius, height, and direction (0.0f = X-Axis, 1.0f = Y-Axis, 2.0f = Z-Axis respectively, default 1.0f)
    uint16_t colliderMeshIndex;
    Vector3 activatedScale;
    float mass;
    float linearDrag;
    float angularDrag;
    float inertia;
    float fatigue;
    Vector3 accumulatedForce;
    Vector3 accumulatedTorque;
    float dynamicFriction;
    float staticFriction;
    float bounciness;
    PhysCombineType frictionCombine;
    PhysCombineType bounceCombine;

    // Audio
    float volume;

    // Attachment
    uint16_t   child[MAX_CHILD_COUNT];
    Vector3    child_offset[MAX_CHILD_COUNT];
    Quaternion child_rotation[MAX_CHILD_COUNT];
    Vector3    child_scale[MAX_CHILD_COUNT];

    // NPC logic
    uint8_t npcIndex;
    AIState currentState;
    double timeForTranquilization;
    Vector3 sightPointOffset;
    Vector3 gunPointOffset;
    Vector3 gunPointOffset2;
    uint16_t muzzleBurst;
    uint16_t muzzleBurst2;
    uint16_t enemey;
    double gracePeriodFinished;
    double meleeDamageFinished;
    uint8_t walkWaypointsLength;
    Vector3 walkWaypoints[MAX_WAYPOINTS];
    uint16_t dyingTexture;
    uint16_t deathTexture;
    uint16_t deathBurst;
    float rangeToEnemy;
    int currentWaypoint;
    Vector3 currentDestination;
    double idleTime;
    double attack1SoundTime;
    double attack2SoundTime;
    double attack3SoundTime;
    double timeTillEnemyChangeFinished;
    double timeTillDeadFinished;
    double timeTillPainFinished;
    double huntFinished;
    Vector3 lastKnownEnemyPos;
    double randomWaitForNextAttack1Finished;
    double randomWaitForNextAttack2Finished;
    double randomWaitForNextAttack3Finished;
    Vector3 idealTransformForward;
    Vector3 idealPos;
    double attackFinished;
    double attack2Finished;
    double attack3Finished;
    Vector3 targettingPosition;
    double deathBurstFinished;
    double tranquilizeFinished;
    double wanderFinished;
    double timeSinceMovedEnough;
    double posCheckFinished;
    char targetID[TARGET_ID_LENGTH];

    // Misc
    char path[128];
    // phew what a porker of a struct, it's been a eatin!
} Entity;
extern Entity entities[MAX_ENTITIES]; // Global array of entity definitions (e.g. prefabs)
extern Entity instances[INSTANCE_COUNT];
extern bool instanceIsLODArray[INSTANCE_COUNT];
extern float modelMatrices[INSTANCE_COUNT * 16];
extern uint8_t dirtyInstances[INSTANCE_COUNT];

typedef struct {
    Entity* entries;
    uint32_t count;
    uint32_t capacity;
} DataParser;

// Math
#define vabs(x) ((x) < 0 ? -(x) : (x))
#define vmin(a,b) ((a) < (b) ? (a) : (b))
#define vmax(a,b) ((a) > (b) ? (a) : (b))
#define PI 3.14159265f
#define TAU 6.2831853f
static inline __attribute__((always_inline)) float vfloor(float x) { int i = (int)x; return (float)(i > x ? i - 1 : i); }
static inline __attribute__((always_inline)) float vceil(float x) { int i = (int)x; return (float)(x > 0 && x > (float)i ? i + 1 : i); }
#define vclamp(x,a,b) __builtin_fminf(__builtin_fmaxf(x, a), b)
#define vsqrtf(x) __builtin_sqrtf(x)
static inline __attribute__((always_inline)) float vsign(float x) { return x < 0.0f ? -1.0f : 1.0f; } // Follow Unity Sign convention where 0 = 1.0f sign.
static inline __attribute__((always_inline)) float vsinf(float x) { x -= TAU * vfloor(x / TAU); if (x > PI) { x -= TAU; } float s = (4/PI)*x - (4/(PI*PI))*x*vabs(x); return 0.225f*(s*vabs(s) - s) + s; }
static inline __attribute__((always_inline)) float vcosf(float x) { return vsinf(x + 1.57079632f); }
static inline __attribute__((always_inline)) float vacosf(float x) {
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
static inline __attribute__((always_inline)) float vtan(float x) { return vsinf(x) / vcosf(x); }
static inline __attribute__((always_inline)) float vcot(float x) { float x2 = x * x; float t = x + (x2 * x) * 0.33333333f; return 1.0f / t; }
static inline __attribute__((always_inline)) float deg2rad(float degrees) { return degrees * (PI / 180.0f); }
static inline __attribute__((always_inline)) float vlog2f(float x) {
    union { float f; unsigned int i; } v = { x };
    int e = (int)((v.i >> 23) & 255) - 127;
    v.i = (v.i & 0x7FFFFF) | 0x3F800000;   // normalize mantissa to [1,2)
    float m = v.f;
    float p = m - 1.0f;
    float log2m = p * (1.3465558f + p * (-0.33942322f + p * 0.028794660f)); // polynomial approximation of log2(m)
    return (float)e + log2m;
}

static inline __attribute__((always_inline)) float vlog(float x) { return vlog2f(x) * 0.69314718f; }
static inline __attribute__((always_inline)) float vexp2f(float x) {
    float ip = vfloor(x);
    float fp = x - ip;
    float p = 1.0f + fp * (0.69314718f + fp * (0.24022651f + fp * 0.05550411f)); // poly approximation for 2^fp on [0,1]
    int ei = (int)ip + 127;
    unsigned int bits = (unsigned int)(ei << 23);
    union { unsigned int i; float f; } u = { bits };
    return u.f * p;
}

static inline __attribute__((always_inline)) float vexp(float x) { return vexp2f(x * 1.4426950409f); } // 1/ln(2)
static inline __attribute__((always_inline)) float vpow(float a, float b) { return vexp(b * vlog(a)); }
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
#define MAX_TOTAL_PIXELS 26800000u
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
void UpdateScreenSize(GLFWwindow* window, int32_t width, int32_t height);

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
#define FROB_DISTANCE 4.9f
#define ELEVATOR_PAD_TETHER_DIST 2.0f
#define PLAYER_CAPSULE_TOTAL_HEIGHT 2.0f
#define PLAYER_CAPSULE_RADIUS 0.48f
#define PLAYER_CROUCH_RATIO 0.6f
#define PLAYER_PRONE_RATIO 0.2f
#define PLAYER_TRANSITION_TO_PRONE_ADD 0.1f
#define PLAYER_CAMERA_OFFSET_Y 0.84f
extern uint16_t testPointInSolid;
extern bool boosterActive;

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

void AddForce(uint16_t idx, Vector3 force, bool isImpulse);
int32_t Physics(void); // Main event tick
RaycastHit Raycast(Vector3 origin, Vector3 dir, float distance, uint32_t layerMask);
void RaycastAll(Vector3 origin, Vector3 dir, float distance, uint32_t layerMask, RaycastHit* hits, uint16_t maxCount);
bool CheckCapsule(Vector3 start, Vector3 end, float capsuleRadius, float capsuleHeight, uint32_t layerMask);
RaycastHit CapsuleCast(Vector3 start, Vector3 end, float capsuleRadius, float castDist, uint32_t layerMask, bool hitTriggers);
void ApplyPlayerMovements(void);
// ----------------------------------------------------------------------------
// Input
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
	#define CHECK_GL_ERROR() do { GLenum err = glGetError(); if (err != GL_NO_ERROR) DualLogError("GL Error at %s:%d: %d\n", __FILE__, __LINE__, err); } while(0)
#else
	#define CHECK_GL_ERROR() do {} while(0)
#endif
    
#define FAR_PLANE (71.68f) // Max player view, level 6 crawlway 28 cells
#define NEAR_PLANE (0.02f)
#define FAR_PLANE_SQUARED (FAR_PLANE * FAR_PLANE)
#define MAX_DEBUG_LINE_VERTS 8
extern float fogColorR, fogColorG, fogColorB, fogBaseDensityForLevel;
void SetFog(void);
extern bool lightDirty[LIGHT_COUNT];
#define VOXEL_COUNT 262144 // 64 * 64 * 8 * 8
#define VOXEL_LIGHT_IDX_CLEAR_VALUE 0xFFFFFFFFu
#define CURSOR_SCREEN_PERCENTAGE 0.02f
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
#define FONT_ATLAS_SIZE 3072
#define MAX_GLYPHS 639
#define FONT_NORMAL 0
#define FONT_STOPD  1
#define TEXT_WHITE                0
#define TEXT_YELLOW               1
#define TEXT_DARK_YELLOW          2
#define TEXT_GREEN                3
#define TEXT_RED                  4
#define TEXT_ORANGE               5
#define TEXT_STOPD_RED            6
#define TEXT_STOPD_RED_HIGHLIGHT  7
#define TEXT_STOPD_RED_PAUSETITLE 8
#define TEXT_GREEN_MENU           9
#define TEXT_GREEN_MENU_SHADOW   10
#define TEXT_GREEN_MENU_GLOW     11
#define TEXT_RED_MENU            12
extern bool returnToPause;
extern char** audiologNames;
extern char** audiologSubjects;
extern char** audiologSenders;
extern char** audioLogSpeech2Text;
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
