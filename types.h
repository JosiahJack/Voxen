// types.h
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "./External/glad/gl.h"
#include "./External/glfw3.h"
#define INSTANCE_COUNT 10240 // Max 5454 for Citadel level 7 geometry, Max 295 for Citadel level 1 dynamic objects, 1561 lights, extras for dynamically spawned objects/lights

typedef struct { float r,g,b,a; } Color;
typedef struct { float x,y; } Vector2;
typedef struct { float x,y,z; } Vector3;
typedef struct { float x,y,z,w; } Quaternion;
typedef uint8_t PhysCombineType;
typedef uint8_t ColliderType;
typedef uint8_t DoorState;

typedef struct {
    GLFWwindow* window;
	bool inventoryMode;
	double last_time;
	double last_topframe_time;
	double last_physics_time;
	double current_time;
	double timeSinceLastPhysicsTick;
	double screenshotTimeout;
	double pauseRelativeTime;
	double statusTextDecayFinished;
	bool levelCurrentlyLoading;
	char global_modname[256];
	bool global_modIsCitadel;
	uint8_t startLevel;
	uint8_t numLevels; // Can be set by gamedata.txt
	uint8_t currentLevel;
	bool gamePaused;
	bool menuActive;
} GlobalContext;

typedef struct {
	int32_t InputCodeSettings[42];
	uint16_t ScreenWidth;
	uint16_t ScreenHeight;
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
	uint8_t InvertCyberspaceLook;
	uint8_t QuickItemPickup;
	uint8_t QuickReloadWeapons;
	uint8_t MouseSensitivity;
	uint8_t NoShootMode;
	uint8_t HeadBob;
	uint8_t SSR_RES;
} SettingsSystem;

typedef struct {
    bool down;
    bool pressed;
    bool released;
} KeyState;

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
	double scrollDelta;
	bool ignore_next_mouse_delta;
	bool lastUse;
	bool isCapsLockOn;
} InputSystem;

typedef struct {
	uint32_t globalFrameNum;
	double cpuTime;
	double lastFrameSecCountTime;
	uint32_t lastFrameSecCount;
	uint32_t framesPerLastSecond;
	uint32_t worstFPS;
	Vector3 debugLine_start;
	Vector3 debugLine_end;
	double debugLineFinished;
	uint32_t drawCallsRenderedThisFrame;
	uint32_t textDrawCallsRenderedThisFrame;
	uint32_t uiImageDrawCallsRenderedThisFrame;
	uint32_t shadowDrawCallsRenderedThisFrame;
	uint32_t verticesRenderedThisFrame;
	uint32_t drawCallsNormal;
	uint32_t debugLineVertCount;
} VoxenDiagnostics;

#define LIGHT_COUNT 1600 // MAX CITADEL LIGHT COUNT is 1561 for Level 7, leaves room for dynamic lights from projectiles
#define MAX_SHADOWMAPS 48u
#define SHADOW_MAP_SIZE 192u
#define TOTAL_SHADOWMAP_PIXELS (MAX_SHADOWMAPS * (SHADOW_MAP_SIZE * SHADOW_MAP_SIZE * 6U))
typedef struct {
    double shadowTime;
	uint32_t numShadowsCouldRender;
	uint32_t shadowmapSizes[MAX_SHADOWMAPS];
	uint32_t shadowmapOffsets[MAX_SHADOWMAPS];
    uint32_t shadowmapIndirectionList[LIGHT_COUNT];
    float shadDotThresh;
	bool useComputeClear;
} VoxenShadowSystem;

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

#define TEXT_DATA_FILEBUFFER_SIZE 65536 // 16 pages
#define TEXT_STRING_COUNT 1100
#define TEXT_LOCALIZATION_MAX_LENGTH 1207
#define TEXT_LOGS_COUNT 134
typedef struct {	
	uint8_t file_data[TEXT_DATA_FILEBUFFER_SIZE]; // Found that only 59430 were needed at one point, padded for safety and typo fixes
	char stringTable[TEXT_STRING_COUNT][TEXT_LOCALIZATION_MAX_LENGTH]; // Hefty table for localization support.
	uint16_t audioLogImagesRefIndicesLH[TEXT_LOGS_COUNT];
	uint16_t audioLogImagesRefIndicesRH[TEXT_LOGS_COUNT];
	uint8_t audioLogType[TEXT_LOGS_COUNT];
	uint8_t audioLogLevelFound[TEXT_LOGS_COUNT];
} Voxen_Text;

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

typedef struct {
    Vector3 mins;
    Vector3 maxs;
    uint8_t type;
} Trigger;

typedef struct {
	Vector3 normal;
    float d;
} FrustumPlane;

typedef struct {
	uint16_t x,z;
} PortalCell;

typedef struct {
    PortalCell cellA;    // one side (usually the cell the door happened to just barely floating point rounding error start in)
    PortalCell cellB;    // tother side
    bool     portalNS; // true when the two cells share N or S edge, else they share E and W edges.
    bool     open;     // door is open
    bool     dirty;
} Portal;

typedef struct {
	float speed;
	uint16_t frameStart;
	uint16_t frameEnd;
	uint16_t frameStartModelIndex;
	uint16_t frameEndModelIndex;
	uint8_t framerate;
} AnimationClip;

typedef struct {
    Vector3 point;
    Vector3 normal;
    float distance;
    uint16_t hitInstanceIndex;
    bool hit;
} RaycastHit;

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
static const uint8_t MusicType_None = 0;
static const uint8_t MusicType_Walking = 1;
static const uint8_t MusicType_Combat = 2;
static const uint8_t MusicType_Overlay = 3;
static const uint8_t MusicType_Override = 4;

// TrackType
typedef uint8_t TrackType;
static const uint8_t TrackType_None = 0;
static const uint8_t TrackType_Walking = 1;
static const uint8_t TrackType_Combat = 2;
static const uint8_t TrackType_MutantNear = 3;
static const uint8_t TrackType_CyborgNear = 4;
static const uint8_t TrackType_CyborgDroneNear = 5;
static const uint8_t TrackType_RobotNear = 6;
static const uint8_t TrackType_Transition = 7;
static const uint8_t TrackType_Revive = 8;
static const uint8_t TrackType_Death = 9;
static const uint8_t TrackType_Cybertube = 10;
static const uint8_t TrackType_Elevator = 11;
static const uint8_t TrackType_Distortion = 12;

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

// PoolType
typedef uint8_t PoolType;
static const uint8_t PoolType_None = 0;
static const uint8_t PoolType_CameraExplosions = 1;
static const uint8_t PoolType_SparksSmall = 2;
static const uint8_t PoolType_BloodSpurtSmall = 3;
static const uint8_t PoolType_BloodSpurtSmallYellow = 4;
static const uint8_t PoolType_BloodSpurtSmallGreen = 5;
static const uint8_t PoolType_SparksSmallBlue = 6;
static const uint8_t PoolType_HopperImpact = 7;
static const uint8_t PoolType_GrenadeFragExplosions = 8;
static const uint8_t PoolType_Vaporize = 9;
static const uint8_t PoolType_BlasterImpacts = 10;
static const uint8_t PoolType_IonImpacts = 11;
static const uint8_t PoolType_MagpulseImpacts = 12;
static const uint8_t PoolType_StungunImpacts = 13;
static const uint8_t PoolType_RailgunImpacts = 14;
static const uint8_t PoolType_PlasmaImpacts = 15;
static const uint8_t PoolType_ProjEnemShot6Impacts = 16;
static const uint8_t PoolType_ProjEnemShot2Impacts = 17;
static const uint8_t PoolType_ProjSeedPodsImpacts = 18;
static const uint8_t PoolType_TempAudioSources = 19;
static const uint8_t PoolType_GrenadeEMPExplosions = 20;
static const uint8_t PoolType_ProjEnemShot4Impacts = 21;
static const uint8_t PoolType_CrateExplosions = 22;
static const uint8_t PoolType_GrenadeFragLive = 23;
static const uint8_t PoolType_ConcussionLive = 24;
static const uint8_t PoolType_EMPLive = 25;
static const uint8_t PoolType_GasLive = 26;
static const uint8_t PoolType_GasExplosions = 27;
static const uint8_t PoolType_CorpseHit = 28;
static const uint8_t PoolType_LeafBurst = 29;
static const uint8_t PoolType_MutationBurst = 30;
static const uint8_t PoolType_GraytationBurst = 31;
static const uint8_t PoolType_BarrelExplosions = 32;
static const uint8_t PoolType_CyberDissolve = 33;
static const uint8_t PoolType_AutomapBotOverlays = 34;
static const uint8_t PoolType_AutomapCyborgOverlays = 35;
static const uint8_t PoolType_AutomapMutantOverlays = 36;
static const uint8_t PoolType_AutomapCameraOverlays = 37;

// ConfigToggleType
typedef uint8_t ConfigToggleType;
static const uint8_t ConfigToggleType_Fullscreen = 0;
static const uint8_t ConfigToggleType_SSAO = 1;
static const uint8_t ConfigToggleType_Bloom = 2;
static const uint8_t ConfigToggleType_SEGI = 3;
static const uint8_t ConfigToggleType_Reverb = 4;
static const uint8_t ConfigToggleType_Subtitles = 5;
static const uint8_t ConfigToggleType_InvertLook = 6;
static const uint8_t ConfigToggleType_InvertCyber = 7;
static const uint8_t ConfigToggleType_InvertInventoryCycling = 8;
static const uint8_t ConfigToggleType_QuickPickup = 9;
static const uint8_t ConfigToggleType_QuickReload = 10;
static const uint8_t ConfigToggleType_Reflections = 11;
static const uint8_t ConfigToggleType_Vsync = 12;
static const uint8_t ConfigToggleType_NoShootMode = 13;
static const uint8_t ConfigToggleType_DynamicMusic = 14;
static const uint8_t ConfigToggleType_HeadBob = 15;
static const uint8_t ConfigToggleType_Footsteps = 16;

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

#define MAX_CHILD_COUNT 4
typedef struct {
    uint16_t index; // constIndex for entity type, used for indexing into arrays for resourec types when loading resources
    uint32_t physics_handle;
    uint32_t entflags;
    uint16_t modelIndex;
    uint8_t animated;
    uint16_t texIndex;
    uint16_t altTexIndex;
    uint16_t glowIndex;
    uint16_t altGlowIndex;
    uint16_t specIndex;
    uint16_t normIndex;
    uint16_t lodIndex;
    uint8_t clip;
    uint8_t numclips;
    uint8_t animationNum; // Global animation identifier into short table of AnimationClip's
    uint16_t frame;
	int32_t cellIndex;
    uint8_t portalIndex; // If this is a door, index into portal array for toggling state.
    DoorState doorState;
    double currentFrameFinished;
	double currentFrameStartTime;
    Vector3 position;
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
    float mass;
    float linearDrag;
    float angularDrag;
    float inertia;
    Vector3 accumulatedForce;
    Vector3 accumulatedTorque;
    float dynamicFriction;
    float staticFriction;
    float bounciness;
    PhysCombineType frictionCombine;
    PhysCombineType bounceCombine;
    float volume;
    uint16_t   child[MAX_CHILD_COUNT];
    Vector3    child_offset[MAX_CHILD_COUNT];
    Quaternion child_rotation[MAX_CHILD_COUNT];
    Vector3    child_scale[MAX_CHILD_COUNT];
    bool persistent;
    bool overrideTest;
    char path[128];
} Entity;

typedef struct {
    Entity* entries;
    uint32_t count;
    uint32_t capacity;
} DataParser;

#define MODEL_IDX_MAX 7168
typedef struct {
	GLuint inputImageID;
	GLuint inputDepthID;
	GLuint inputWorldPosID;
	GLuint inputSpecID;
	GLuint inputNormalID;
	GLuint gBufferFBO;
	GLuint outputImageID;
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
	GLuint modelBoundsID;
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
	GLuint voxelLightListsID;
	GLuint vbos[MODEL_IDX_MAX];
	GLuint tbos[MODEL_IDX_MAX];
} Voxen_GL_Comms;
