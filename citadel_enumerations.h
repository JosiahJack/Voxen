// citadel_enumerations.h
#ifndef CITADEL_ENUMERATIONS_H
#define CITADEL_ENUMERATIONS_H

#include <stdint.h> // For uint8_t

// SaveableType
typedef uint8_t SaveableType;
static const uint8_t SaveableType_Transform = 0;
static const uint8_t SaveableType_Player = 1;
static const uint8_t SaveableType_Useable = 2;
static const uint8_t SaveableType_Grenade = 3;
static const uint8_t SaveableType_NPC = 4;
static const uint8_t SaveableType_Destructable = 5;
static const uint8_t SaveableType_SearchableStatic = 6;
static const uint8_t SaveableType_SearchableDestructable = 7;
static const uint8_t SaveableType_Door = 8;
static const uint8_t SaveableType_ForceBridge = 9;
static const uint8_t SaveableType_Switch = 10;
static const uint8_t SaveableType_FuncWall = 11;
static const uint8_t SaveableType_TeleDest = 12;
static const uint8_t SaveableType_LBranch = 13;
static const uint8_t SaveableType_LRelay = 14;
static const uint8_t SaveableType_LSpawner = 15;
static const uint8_t SaveableType_InteractablePanel = 16;
static const uint8_t SaveableType_ElevatorPanel = 17;
static const uint8_t SaveableType_Keypad = 18;
static const uint8_t SaveableType_PuzzleGrid = 19;
static const uint8_t SaveableType_PuzzleWire = 20;
static const uint8_t SaveableType_TCounter = 21;
static const uint8_t SaveableType_TGravity = 22;
static const uint8_t SaveableType_MChanger = 23;
static const uint8_t SaveableType_GravPad = 24;
static const uint8_t SaveableType_TransformParentless = 25;
static const uint8_t SaveableType_ChargeStation = 26;
static const uint8_t SaveableType_Light = 27;
static const uint8_t SaveableType_LTimer = 28;
static const uint8_t SaveableType_Camera = 29;
static const uint8_t SaveableType_DelayedSpawn = 30;
static const uint8_t SaveableType_SecurityCamera = 31;
static const uint8_t SaveableType_Trigger = 32;
static const uint8_t SaveableType_Projectile = 33;
static const uint8_t SaveableType_NormalScreen = 34;
static const uint8_t SaveableType_CyberSwitch = 35;
static const uint8_t SaveableType_CyberItem = 36;

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

#endif // CITADEL_ENUMERATIONS_H
