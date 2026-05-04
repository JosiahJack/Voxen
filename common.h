// common.h - Shared items between engine and gamecode (e.g. enums)
typedef __INT8_TYPE__    i8;
typedef __UINT8_TYPE__   u8;
typedef __INT16_TYPE__  i16;
typedef __UINT16_TYPE__ u16;
typedef __INT32_TYPE__  i32;
typedef __UINT32_TYPE__ u32;
typedef __INT64_TYPE__  i64;
typedef __UINT64_TYPE__ u64;
typedef __SIZE_TYPE__ size_t;
#ifndef U8_MAX
    #define U8_MAX 255
#endif
#ifndef U16_MAX
    #define U16_MAX 65535
#endif
#define bool _Bool
#define true 1
#define false 0
typedef struct { float r,g,b; } Color3;
typedef struct { float r,g,b,a; } Color;
typedef struct { float x,y; } Vector2;
typedef struct { float x,y,z; } Vector3;
typedef struct { float x,y,z,w; } Quaternion;
#define QUAT_IDENTITY ((Quaternion){0.0f,0.0f,0.0f,1.0f})
typedef u8 PhysCombineType;
typedef u8 ColliderType;
typedef u16 Text;
typedef struct { Vector3 point; Vector3 normal; float distance; u16 hitInstanceIndex; bool hit; } RaycastHit;
typedef struct { float speed; u16 frameStart,frameEnd,frameStartModelIndex; u8 framerate; } AnimationClip;
#define LIGHTON 1
#define SHADON  2
#define LIGHT_AND_SHADOW_ON 3
#define LSPOT   4
#define LDIR    8
#define LDIRTY 16
#define LERPON 32
typedef struct { Vector3 pos; float intensity; Color3 col; u32 lflags; float range,spotAng,maxIntensity,minIntensity; Quaternion spotDir; } Light; // 64bytes, one cache line, packed for GL transfer
typedef struct { float lerpValue,lerpStepTime,lerpStartTime,lerpTime,intervalSteps[32]; bool stepIsLerping[32],lerpUp; u8 currentStep,numIntervalSteps,numLerpSteps; } LightAnimation; // Separate from main lights buffer struct since it's not used very often
#define INSTANCE_COUNT 7680 // Max 5454 for Citadel level 7 geometry, Max 295 for Citadel level 1 dynamic objects, 1561 lights, extras for dynamically spawned objects/lights
#define LIGHT_COUNT 2048
#define MODEL_IDX_MAX 6805
#define MAX_VALID_TEXTURE 2048
#define MAX_TOTAL_PIXELS 30000000u
#define MAX_UNIQUE_COLORS 1048576u
#define MAX_ANIMATED_MODELS 52
#define MAX_ANIMATION_CLIPS_PER_MODEL 32
#define MAX_DEBUG_LINE_VERTS 8
#define MAX_PORTALS 64 // Max is 49 on Citadel level 7
#define DOUBLE_CLICK_TIME 0.5f
#define PLAYER_MAX_WALK_SPEED 3.2f
#define PLAYER_MAX_SPRINT_SPEED 8.8f
#define PLAYER_MAX_CYBER_SPEED 5.0f
#define PLAYER_MAX_CYBER_ULTIMATE_SPEED 12.0f
#define PLAYER_MAX_SPRINT_SPEED_FATIGUED 5.5f
#define PLAYER_MAX_CROUCH_SPEED 1.25f
#define PLAYER_MAX_PRONE_SPEED 0.5f
#define PLAYER_BOOSTER_SPEED_BOOST 1.2f
#define PLAYER_CROUCH_RATIO 0.6f
#define PLAYER_PRONE_RATIO 0.2f
#define PLAYER_TRANSITION_TO_PRONE_ADD 0.1f
#define PLAYER_RADIUS 0.48f
#define PLAYER_HEIGHT 2.00f
#define PLAYER_CAM_OFFSET_Y 0.84f // Split capsule shape in the middle, camera is thus 0.16 away from top of the capsule ((2 / 2 = 1) - 0.84)
#define ELEVATOR_PAD_TETHER_DIST 2.0f
#define PLAYER_CAPSULE_TOTAL_HEIGHT 2.0f
#define PLAYER_CAPSULE_RADIUS 0.48f
#define LEVEL_CYBERSPACE 13
#define MAX_ENTITIES 768 // Unique entity types, different than INSTANCE_COUNT which is the number of instances of any of these entities.
#define NULLENT 0u
#define WORLD   0u // Much like Quake, the world is entity 0.  Aand also like Quake, world is nullent and is 0.
#define PLAYER1 1u
#define PLAYER2 2u
#define START_INDEX_LEVEL_INSTANCES 3
#define WORLDX 64
#define WORLDZ WORLDX
#define WORLDY 18 // Level 8 is only 17.5 cells tall!!  Could be 16 if I make the ceiling same height in last room as in original.
#define WORLDX_0BASED (WORLDX - 1)
#define WORLDZ_0BASED (WORLDZ - 1)
#define TOTAL_WORLD_CELLS (WORLDX * WORLDY * WORLDZ)
#define ARRSIZE (WORLDX * WORLDZ)
#define VOXELS_X (WORLDX * VOXELS_PER_CELL)
#define VOXELS_Z (WORLDZ * VOXELS_PER_CELL)
#define VOXEL_COUNT (VOXELS_X * VOXELS_Z) // 64 * 64 * 8 * 8
#define CELL_SIZE 2.56f // Each cell is 2.56x2.56
#define CELLXHALF (CELL_SIZE * 0.5f)
#define VOXELS_PER_CELL 8
#define VOXEL_SIZE (CELL_SIZE / (float)VOXELS_PER_CELL)
#define VOXEL_HALF (VOXEL_SIZE * 0.5f)
#define CELL_VISIBLE       1u
#define CELL_OPEN          2u
#define CELL_CLOSEDNORTH   4u
#define CELL_CLOSEDEAST    8u
#define CELL_CLOSEDSOUTH  16u
#define CELL_CLOSEDWEST   32u
#define CELL_SEES_SUN     64u
#define CELL_SEES_SKYBOX 128u
#define ENTFLAG_ACTIVE               (1ull <<  0) // Instance renders and updates
#define ENTFLAG_ISGRENADE            (1ull <<  1)
#define ENTFLAG_GROUNDED             (1ull <<  2)
#define ENTFLAG_KINEMATIC            (1ull <<  3)
#define ENTFLAG_RIGIDBODY            (1ull <<  4)
#define ENTFLAG_NO_SHADOWS           (1ull <<  5)
#define ENTFLAG_ASLEEP               (1ull <<  6) // Check if enemy starts out asleep such as the sleeping sec-2 bots on level 8 in the maintenance and recharge bays.
#define ENTFLAG_WALK_PATH_ON_START   (1ull <<  7)
#define ENTFLAG_TOUCHING_HURTS       (1ull <<  8)
#define ENTFLAG_ACT_AS_CORPSE_ONLY   (1ull <<  9)
#define ENTFLAG_DYING                (1ull << 10)
#define ENTFLAG_DEATH_BURST_DONE     (1ull << 11)
#define ENTFLAG_DEAD                 (1ull << 12)
#define ENTFLAG_TELEPORT_ON_DEATH    (1ull << 13)
#define ENTFLAG_GO_INTO_PAIN         (1ull << 14)
#define ENTFLAG_DONT_LOOP_WAYPTS     (1ull << 15)
#define ENTFLAG_VISIT_WAYPTS_RND     (1ull << 16)
#define ENTFLAG_WANDERING            (1ull << 17)
#define ENTFLAG_ACT_AS_TURRET        (1ull << 18)
#define ENTFLAG_TARGID_ATTACHED      (1ull << 19)
#define ENTFLAG_ENEM_IN_SIGHT        (1ull << 20)
#define ENTFLAG_ENEM_IN_FRONT        (1ull << 21)
#define ENTFLAG_ENEM_IN_FOV          (1ull << 22)
#define ENTFLAG_ENEM_IN_LOS          (1ull << 23)
#define ENTFLAG_FIRST_SIGHTING       (1ull << 24)
#define ENTFLAG_DYING_SETUP          (1ull << 25)
#define ENTFLAG_HAD_ENEMY            (1ull << 26)
#define ENTFLAG_SHOT_FIRED           (1ull << 27)
#define ENTFLAG_DEAD_CHECKS_DONE     (1ull << 28)
#define ENTFLAG_HOP_DONE             (1ull << 29)
#define ENTFLAG_LOCKED               (1ull << 30)
#define ENTFLAG_HAS_CAMERA_VIEW      (1ull << 31)
#define ENTFLAG_REQUIRE_RESET        (1ull << 32)
#define ENTFLAG_GRAV_LIFT_STATE      (1ull << 33)
#define ENTFLAG_STOPSOUND_PLAYED     (1ull << 34)
#define ENTFLAG_DAMAGE_ON_USE        (1ull << 35)
#define ENTFLAG_MAKING_NOISE         (1ull << 36)
#define ENTFLAG_VISIBLE              (1ull << 37) // Renders
#define ENTFLAG_ANIM_DEAD_DONE       (1ull << 38)
#define ENTFLAG_NO_DYING_ANIM        (1ull << 39)
#define ENTFLAG_NO_DEATH_FREEZE      (1ull << 40)
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
#define TARGET_STRING_LENGTH 38
#define CURSOR_SCREEN_PERCENTAGE 0.02f
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
#define SAVE_REMINDER_TIME 7.0f // 7secs ~is human short-term memory length
#define CREDITS_PAGES 22
#define MAX_WAYPOINTS 8
#define TARGET_ID_LENGTH 32 // Max needed 22 + 5 for ID + 1 for space between them = 28
#define SOUNDS_COUNT 670
#define TEXT_DATA_FILEBUFFER_SIZE 65536 // 16 pages
#define TEXT_STRING_COUNT 1100
#define TEXT_LOCALIZATION_MAX_LENGTH 1280
#define TEXT_LOGS_COUNT 134
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

// BodyState
typedef u8 BodyState;
static const u8 BodyState_Standing = 0;
static const u8 BodyState_Crouch = 1;
static const u8 BodyState_CrouchingDown = 2;
static const u8 BodyState_StandingUp = 3;
static const u8 BodyState_Prone = 4;
static const u8 BodyState_ProningDown = 5;
static const u8 BodyState_ProningUp = 6;

// Handedness
typedef u8 Handedness;
static const u8 Handedness_Center = 0;
static const u8 Handedness_LH = 1;
static const u8 Handedness_RH = 2;

// AttackType
typedef u8 AttackType;
static const u8 AttackType_None = 0;
static const u8 AttackType_Melee = 1;
static const u8 AttackType_MeleeEnergy = 2;
static const u8 AttackType_EnergyBeam = 3;
static const u8 AttackType_Magnetic = 4;
static const u8 AttackType_Projectile = 5;
static const u8 AttackType_ProjectileNeedle = 6;
static const u8 AttackType_ProjectileEnergyBeam = 7;
static const u8 AttackType_ProjectileLaunched = 8;
static const u8 AttackType_Gas = 9;
static const u8 AttackType_Tranq = 10;
static const u8 AttackType_Drill = 11;

// NPCType
typedef u8 NPCType;
static const u8 NPCType_Mutant = 0;
static const u8 NPCType_Supermutant = 1;
static const u8 NPCType_Robot = 2;
static const u8 NPCType_Cyborg = 3;
static const u8 NPCType_Supercyborg = 4;
static const u8 NPCType_Cyber = 5;
static const u8 NPCType_MutantCyborg = 6;

// PerceptionLevel
typedef u8 PerceptionLevel;
static const u8 PerceptionLevel_Low = 0;
static const u8 PerceptionLevel_Medium = 1;
static const u8 PerceptionLevel_High = 2;
static const u8 PerceptionLevel_Omniscient = 3;

// AIState
typedef u8 AIState;
static const u8 AIState_Idle = 0;
static const u8 AIState_Walk = 1;
static const u8 AIState_Run = 2;
static const u8 AIState_Attack1 = 3;
static const u8 AIState_Attack2 = 4;
static const u8 AIState_Attack3 = 5;
static const u8 AIState_Pain = 6;
static const u8 AIState_Dying = 7;
static const u8 AIState_Dead = 8;
static const u8 AIState_Inspect = 9;
static const u8 AIState_Interacting = 10;

// AIMoveType
typedef u8 AIMoveType;
static const u8 AIMoveType_Walk = 0;
static const u8 AIMoveType_Fly = 1;
static const u8 AIMoveType_Swim = 2;
static const u8 AIMoveType_Cyber = 3;
static const u8 AIMoveType_None = 4;

// DoorState
typedef u8 DoorState;
static const u8 DoorState_Closed = 0;
static const u8 DoorState_Open = 1;
static const u8 DoorState_Closing = 2;
static const u8 DoorState_Opening = 3;

// FuncStates
typedef u8 FuncStates;
static const u8 FuncStates_Start = 0;
static const u8 FuncStates_Target = 1;
static const u8 FuncStates_MovingStart = 2;
static const u8 FuncStates_MovingTarget = 3;
static const u8 FuncStates_AjarMovingStart = 4;
static const u8 FuncStates_AjarMovingTarget = 5;

// SoftwareType
typedef u8 SoftwareType;
static const u8 SoftwareType_None = 0;
static const u8 SoftwareType_Drill = 1;
static const u8 SoftwareType_Pulser = 2;
static const u8 SoftwareType_CShield = 3;
static const u8 SoftwareType_Decoy = 4;
static const u8 SoftwareType_Recall = 5;
static const u8 SoftwareType_Turbo = 6;
static const u8 SoftwareType_Game = 7;
static const u8 SoftwareType_Data = 8;
static const u8 SoftwareType_Integrity = 9;
static const u8 SoftwareType_Keycard = 10;

// AccessCardType
typedef u8 AccessCardType;
static const u8 AccessCardType_None = 0;
static const u8 AccessCardType_Standard = 1;
static const u8 AccessCardType_Medical = 2;
static const u8 AccessCardType_Science = 3;
static const u8 AccessCardType_Admin = 4;
static const u8 AccessCardType_Group1 = 5;
static const u8 AccessCardType_Group2 = 6;
static const u8 AccessCardType_Group3 = 7;
static const u8 AccessCardType_Group4 = 8;
static const u8 AccessCardType_GroupA = 9;
static const u8 AccessCardType_GroupB = 10;
static const u8 AccessCardType_Storage = 11;
static const u8 AccessCardType_Engineering = 12;
static const u8 AccessCardType_Maintenance = 13;
static const u8 AccessCardType_Security = 14;
static const u8 AccessCardType_Per1 = 15;
static const u8 AccessCardType_Per2 = 16;
static const u8 AccessCardType_Per3 = 17;
static const u8 AccessCardType_Per4 = 18;
static const u8 AccessCardType_Per5 = 19;

// MusicType
typedef u8 MusicType;
static const u8 MusicType_None     = 0;
static const u8 MusicType_Walking  = 1;
static const u8 MusicType_Combat   = 2;
static const u8 MusicType_Override = 3;

// TrackType
typedef u8 TrackType;
static const u8 TrackType_None       = 0;
static const u8 TrackType_Walking    = 1;
static const u8 TrackType_Combat     = 2;
static const u8 TrackType_Revive     = 3;
static const u8 TrackType_Death      = 4;
static const u8 TrackType_Cybertube  = 5;
static const u8 TrackType_Elevator   = 6;
static const u8 TrackType_Distortion = 7;

// BloodType
typedef u8 BloodType;
static const u8 BloodType_None = 0;
static const u8 BloodType_Red = 1;
static const u8 BloodType_Yellow = 2;
static const u8 BloodType_Green = 3;
static const u8 BloodType_Robot = 4;
static const u8 BloodType_Leaf = 5;
static const u8 BloodType_Mutation = 6;
static const u8 BloodType_GrayMutation = 7;

// SecurityType
typedef u8 SecurityType;
static const u8 SecurityType_None = 0;
static const u8 SecurityType_Camera = 1;
static const u8 SecurityType_NodeSmall = 2;
static const u8 SecurityType_NodeLarge = 3;

// AudioLogType
typedef u8 AudioLogType;
static const u8 AudioLogType_TextOnly = 0;
static const u8 AudioLogType_Normal = 1;
static const u8 AudioLogType_Email = 2;
static const u8 AudioLogType_Papers = 3;
static const u8 AudioLogType_Vmail = 4;
static const u8 AudioLogType_Game = 5;

// EnergyType
typedef u8 EnergyType;
static const u8 EnergyType_Battery = 0;
static const u8 EnergyType_ChargeStation = 1;

// FootStepType
typedef u8 FootStepType;
static const u8 FootStepType_None = 0;
static const u8 FootStepType_Carpet = 1;
static const u8 FootStepType_Concrete = 2;
static const u8 FootStepType_GrittyCrete = 3;
static const u8 FootStepType_Grass = 4;
static const u8 FootStepType_Gravel = 5;
static const u8 FootStepType_Rock = 6;
static const u8 FootStepType_Glass = 7;
static const u8 FootStepType_Marble = 8;
static const u8 FootStepType_Metal = 9;
static const u8 FootStepType_Grate = 10;
static const u8 FootStepType_Metal2 = 11;
static const u8 FootStepType_Metpanel = 12;
static const u8 FootStepType_Panel = 13;
static const u8 FootStepType_Plaster = 14;
static const u8 FootStepType_Plastic = 15;
static const u8 FootStepType_Plastic2 = 16;
static const u8 FootStepType_Rubber = 17;
static const u8 FootStepType_Sand = 18;
static const u8 FootStepType_Squish = 19;
static const u8 FootStepType_Vent = 20;
static const u8 FootStepType_Water = 21;
static const u8 FootStepType_Wood = 22;
static const u8 FootStepType_Wood2 = 23;

// MusicResourceType
typedef u8 MusicResourceType;
static const u8 MusicResourceType_Menu = 0;
static const u8 MusicResourceType_Medical = 1;
static const u8 MusicResourceType_Science = 2;
static const u8 MusicResourceType_Reactor = 3;
static const u8 MusicResourceType_Executive = 4;
static const u8 MusicResourceType_Grove = 5;
static const u8 MusicResourceType_Cyber = 6;
static const u8 MusicResourceType_Security = 7;
static const u8 MusicResourceType_Revive = 8;
static const u8 MusicResourceType_Death = 9;
static const u8 MusicResourceType_Elevator = 10;
static const u8 MusicResourceType_Distortion = 11;
static const u8 MusicResourceType_Looped = 12;
static const u8 MusicResourceType_Level = 13;

// HUDColor
typedef u8 HUDColor;
static const u8 HUDColor_White = 0;
static const u8 HUDColor_Red = 1;
static const u8 HUDColor_Orange = 2;
static const u8 HUDColor_Yellow = 3;
static const u8 HUDColor_Green = 4;
static const u8 HUDColor_Blue = 5;
static const u8 HUDColor_Purple = 6;
static const u8 HUDColor_Gray = 7;

// ForceFieldColor
typedef u8 ForceFieldColor;
static const u8 ForceFieldColor_Red = 0;
static const u8 ForceFieldColor_Green = 1;
static const u8 ForceFieldColor_Blue = 2;
static const u8 ForceFieldColor_Purple = 3;
static const u8 ForceFieldColor_RedFaint = 4;

// ButtonType
typedef u8 ButtonType;
static const u8 ButtonType_Generic = 0;
static const u8 ButtonType_GeneralInv = 1;
static const u8 ButtonType_Patch = 2;
static const u8 ButtonType_Grenade = 3;
static const u8 ButtonType_Weapon = 4;
static const u8 ButtonType_Search = 5;
static const u8 ButtonType_None = 6;
static const u8 ButtonType_PGrid = 7;
static const u8 ButtonType_PWire = 8;
static const u8 ButtonType_Vaporize = 9;
static const u8 ButtonType_ShootMode = 10;
static const u8 ButtonType_GrenadeTimerSlider = 11;

// TabMSG
typedef u8 TabMSG;
static const u8 TabMSG_None = 0;
static const u8 TabMSG_Search = 1;
static const u8 TabMSG_AudioLog = 2;
static const u8 TabMSG_Keypad = 3;
static const u8 TabMSG_Elevator = 4;
static const u8 TabMSG_GridPuzzle = 5;
static const u8 TabMSG_WirePuzzle = 6;
static const u8 TabMSG_EReader = 7;
static const u8 TabMSG_Weapon = 8;
static const u8 TabMSG_SystemAnalyzer = 9;

// PuzzleCellType
typedef u8 PuzzleCellType;
static const u8 PuzzleCellType_Off = 0;
static const u8 PuzzleCellType_Standard = 1;
static const u8 PuzzleCellType_And = 2;
static const u8 PuzzleCellType_Bypass = 3;

// PuzzleGridType
typedef u8 PuzzleGridType;
static const u8 PuzzleGridType_King = 0;
static const u8 PuzzleGridType_Queen = 1;
static const u8 PuzzleGridType_Knight = 2;
static const u8 PuzzleGridType_Rook = 3;
static const u8 PuzzleGridType_Bishop = 4;
static const u8 PuzzleGridType_Pawn = 5;

typedef u32 PhysicsLayer;
static const u32 PhysicsLayer_Default          = (1u << 0);
static const u32 PhysicsLayer_TransparentFX    = (1u << 1);
static const u32 PhysicsLayer_IgnoreRaycast    = (1u << 2);
//                                                   (1u << 3)  // unused
static const u32 PhysicsLayer_Water            = (1u << 4);
static const u32 PhysicsLayer_BlocksRaycast    = (1u << 4); // same as Water
static const u32 PhysicsLayer_UI               = (1u << 5);
//                                                   (1u << 6)  // unused
//                                                   (1u << 7)  // unused
static const u32 PhysicsLayer_GunViewModel     = (1u << 8);
static const u32 PhysicsLayer_Geometry         = (1u << 9);
static const u32 PhysicsLayer_NPC              = (1u << 10);
static const u32 PhysicsLayer_PlayerBullets    = (1u << 11);
static const u32 PhysicsLayer_Player           = (1u << 12);
static const u32 PhysicsLayer_Corpse           = (1u << 13);
static const u32 PhysicsLayer_PhysObjects      = (1u << 14);
static const u32 PhysicsLayer_Sky              = (1u << 15);
static const u32 PhysicsLayer_PlayerTriggerOnly= (1u << 16);
static const u32 PhysicsLayer_Trigger          = (1u << 17);
static const u32 PhysicsLayer_Door             = (1u << 18);
static const u32 PhysicsLayer_InterDebris      = (1u << 19);
static const u32 PhysicsLayer_Player2          = (1u << 20);
static const u32 PhysicsLayer_Player3          = (1u << 21);
static const u32 PhysicsLayer_Player4          = (1u << 22);
static const u32 PhysicsLayer_NPCTrigger       = (1u << 23);
static const u32 PhysicsLayer_NPCBullet        = (1u << 24);
static const u32 PhysicsLayer_NPCClip          = (1u << 25);
static const u32 PhysicsLayer_Clip             = (1u << 26);
static const u32 PhysicsLayer_Automap          = (1u << 27);
static const u32 PhysicsLayer_Culling          = (1u << 28);
static const u32 PhysicsLayer_CorpseSearchable = (1u << 29);
//                                                   (1u << 30) // unused
static const u32 PhysicsLayer_NULL             = (1u << 31);

#define LAYER_MASK_PLAYER_COLLIDESWITH (PhysicsLayer_Clip | PhysicsLayer_NPCBullet | PhysicsLayer_Player2 | PhysicsLayer_Door \
                                       | PhysicsLayer_Trigger | PhysicsLayer_PlayerTriggerOnly | PhysicsLayer_Default | PhysicsLayer_TransparentFX \
                                       | PhysicsLayer_IgnoreRaycast | PhysicsLayer_Geometry | PhysicsLayer_NPC)

#define LAYER_MASK_NPC_COLLIDESWITH (PhysicsLayer_Clip | PhysicsLayer_NPCClip | PhysicsLayer_PlayerBullets | PhysicsLayer_Player2 | PhysicsLayer_Player | PhysicsLayer_Door \
                                    | PhysicsLayer_Trigger | PhysicsLayer_NPCTrigger | PhysicsLayer_Default | PhysicsLayer_TransparentFX \
                                    | PhysicsLayer_IgnoreRaycast | PhysicsLayer_Geometry | PhysicsLayer_NPC)

#define LAYER_MASK_NPC_SIGHT (PhysicsLayer_Default | PhysicsLayer_Geometry | PhysicsLayer_Door | PhysicsLayer_InterDebris \
                             | PhysicsLayer_PhysObjects | PhysicsLayer_Player)

#define LAYER_MASK_NPC_ATTACK (PhysicsLayer_Default | PhysicsLayer_Geometry | PhysicsLayer_NPC | PhysicsLayer_Door \
                              | PhysicsLayer_InterDebris | PhysicsLayer_PhysObjects | PhysicsLayer_Player)

#define LAYER_MASK_NPC_COLLISION (PhysicsLayer_Default | PhysicsLayer_TransparentFX | PhysicsLayer_IgnoreRaycast | PhysicsLayer_Geometry \
                                 | PhysicsLayer_NPC | PhysicsLayer_Door | PhysicsLayer_InterDebris | PhysicsLayer_Player \
                                 | PhysicsLayer_Clip | PhysicsLayer_NPCClip | PhysicsLayer_PhysObjects)

#define LAYER_MASK_PLAYER_FROB (PhysicsLayer_Default | PhysicsLayer_Geometry | PhysicsLayer_Water | PhysicsLayer_Door \
                               | PhysicsLayer_InterDebris | PhysicsLayer_PhysObjects | PhysicsLayer_CorpseSearchable)

#define LAYER_MASK_PLAYER_TARGET_ID_FROB (PhysicsLayer_Default | PhysicsLayer_Geometry | PhysicsLayer_Door | PhysicsLayer_NPC | PhysicsLayer_CorpseSearchable)

#define LAYER_MASK_PLAYER_ATTACK (PhysicsLayer_Default | PhysicsLayer_Geometry | PhysicsLayer_NPC | PhysicsLayer_PlayerBullets \
                                 | PhysicsLayer_Door | PhysicsLayer_InterDebris | PhysicsLayer_PhysObjects | PhysicsLayer_CorpseSearchable)

#define LAYER_MASK_EXPLOSION (PhysicsLayer_Default | PhysicsLayer_Geometry | PhysicsLayer_NPC | PhysicsLayer_PlayerBullets | PhysicsLayer_Door \
                             | PhysicsLayer_InterDebris | PhysicsLayer_PhysObjects | PhysicsLayer_Player | PhysicsLayer_Player2 | PhysicsLayer_CorpseSearchable)

#define LAYER_MASK_PLAYER_FEET (PhysicsLayer_Default | PhysicsLayer_Geometry)

typedef struct {
	i32 InputCodeSettings[42];
	u16 ScreenWidth;
	u16 ScreenHeight;
    float ScreenCenterX;
    float ScreenCenterY;
	bool Fullscreen;
	u8 FOV;
	u8 Brightness;
	u8 Gamma;
	u8 FXAA;
	u8 Shadows;
	u8 Reflections;
	u8 Vsync;
	u8 ModelDetail;
	u8 GI;
	u8 SpeakerMode;
	u8 Reverb;
	u8 VolumeMaster;
	u8 VolumeMusic;
	u8 VolumeMessage;
	u8 VolumeEffects;
	u8 Language;
	u8 DynamicMusic;
	u8 Footsteps;
	u8 InvertLook;
	u8 InvertInventoryCycling;
	u8 InvertCyberspaceLook;
	u8 QuickItemPickup;
	u8 QuickReloadWeapons;
	u8 MouseSensitivity;
	u8 NoShootMode;
	u8 HeadBob;
	u8 SSR_RES;
    u8 CurrentMonitor;
} SettingsSystem;
extern SettingsSystem Sys_Settings;

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
	u8 dizzyLevel;
} CheatsSystem;
extern CheatsSystem Sys_Cheats;

typedef u8 MenuPages;
static const u8 MenuPages_FrontPage = 0;
static const u8 MenuPages_Singleplayer = 1;
static const u8 MenuPages_Multiplayer = 2;
static const u8 MenuPages_NewGame = 3;
static const u8 MenuPages_Load = 4;
static const u8 MenuPages_Options = 5;
static const u8 MenuPages_Save = 6;
static const u8 MenuPages_IntroVideo = 7;
static const u8 MenuPages_CreditsVideo = 8;
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
	u16 linkedElevatorDoor;
	Vector3 objectInUsePos;
	u16 tetheredPGP;
	u16 tetheredPWP;
	u16 tetheredSearchable;
	u16 tetheredKeypadElevator;
	u16 tetheredKeypadKeycode;
	bool paperLogInUse;
	bool usingObject;
	int applyButtonReferenceIndex;
	int curCenterTab;
    bool isBlocking;
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
	u8 highlightTickCount[4];
	double blinkFinished;
	double beepFinished;
	u8 beepCount;
	bool audPaused;
    bool mouseClickHeldOverGUI;
    u8 elevButtonLevelIdx[8];
    u16 elevButtonSpawnIdx[8];
    bool buttonsEnabled[8];
    bool buttonsDarkened[8];
    u8 elevCurrentFloor;
} SystemUI;

typedef struct {	
	char stringTable[TEXT_STRING_COUNT][TEXT_LOCALIZATION_MAX_LENGTH]; // Hefty table for localization support.
	u16 audioLogImagesRefIndicesLH[TEXT_LOGS_COUNT];
	u16 audioLogImagesRefIndicesRH[TEXT_LOGS_COUNT];
	u8 audioLogType[TEXT_LOGS_COUNT];
	u8 audioLogLevelFound[TEXT_LOGS_COUNT];
	size_t file_size,filelog_size;
	u8* file_data;
	u8* filelog_data;
} TextSystem;
extern TextSystem Sys_Text;

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
#define HW_SYS_IDX    0 // System Analyzer
#define HW_NAV_IDX    1 // Navigation Unit
#define HW_ERD_IDX    2 // Datareader/EReader
#define HW_SNS_IDX    3 // Sensaround
#define HW_TID_IDX    4 // Target Identifier
#define HW_SHD_IDX    5 // Energy Shield
#define HW_BIO_IDX    6 // Biomonitor
#define HW_LAN_IDX    7 // Head Mounted Lantern
#define HW_ENV_IDX    8 // Envirosuit
#define HW_BST_IDX    9 // Turbo Motion Booster
#define HW_JET_IDX   10 // Jump Jet Boots
#define HW_INF_IDX   11 // Infrared Night Sight Enhancement

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
    u32 accessCardOwned;
    u8 hasSoft,softVersions[7];
    u16 numLogsFromLevel[10];
    int lastAddedIndex;
	bool beepDone,logPaused,hasNewEmail,hasNewNotes;
	int emailCurrent,emailIndex;
    u8 hasMinigame;
    u16 hasHardware,hardwareIsActive;
    u8 hardwareVersion[HW_COUNT],hardwareVersionSetting[HW_COUNT];
    u16 hardwareInvReferenceIndex[HW_COUNT];
    int hardwareInvCurrent; // Current slot in the general inventory (14 slots).
	int hardwareInvIndex; // Current index to the item look-up table.
	int generalInventoryIndexRef[14];
    double nitroTimeSetting,earthShakerTimeSetting;
    bool currentCyberItem,isPulserNotDrill;
    int globalLookupIndex;
    int weaponInventoryIndices[7],weaponInventoryAmmoIndices[7];
    u8 numweapons;
    bool wepLoadedWithAlternate[7];
    u8 currentMagazineAmount[7],currentMagazineAmount2[7];
    u32 wepAmmo[16],wepAmmoSecondary[16];
    float weaponEnergySetting[16];
    bool justChangedWeap,overloadEnabled,recoiling;
    i16 weaponCurrentPending,weaponIndexPending;
    double justFired,waitTilNextFire,reloadFinished,lerpStartTime,dropFinished;
    float reloadLerpValue,sparqSetting,ionSetting,blasterSetting,plasmaSetting,stungunSetting;
    u8 lerpUp;
	float energySliderClickedTime,cyberWeaponAttackFinished,targetY;
    u16 heldObjectIndex,heldObjectCustomIndex,heldObjectAmmo,heldObjectAmmo2;
    bool heldObjectLoadedAlternate,holdingObject,grenadeActive;
    u16 weaponIndex,currentSearchItem;
    float currentEnergyWeaponHeat[7];
    u8 grenAmmo[7],grenConstIndex[7],grenadeCurrent,generalInvCurrent;
    u16 generalInvIndex,generalInvCustomIndex[14];
    bool hasNewLogs,hasNewData;
    u8 patchCurrent,patchCounts[7],cyberItemIndex;
    float fatigue,radiated,resetAfterDeathTime,energy,maxEnergy;
    u16 patchActive,drainJPM;
    double playerHealthTimer,berserkFinishedTime,berserkIncrementFinishedTime,detoxFinishedTime,geniusFinishedTime,mediFinishedTime,reflexFinishedTime,sightFinishedTime;
    double sightSideEffectFinishedTime,staminupFinishedTime,turboCyberTime,turboFinished,energyDrainTickFinished,painSoundFinished,radSoundFinished,radFXFinished;
    int berserkIncrement;
    float radAdjust;
    float initialRadiation;
    bool playerDead;
    i16 ladderState;
    bool staminupActive,hasLog[134],readLog[134];
} InventorySystem;

typedef struct { float damage,penetration,offense,armorvalue,defense,impactVelocity; Vector3 attacknormal,hitpoint; AttackType attackType; u16 owner,hitIdx; bool isOtherNPC,berserkActive; } DamageData;

typedef /*FAT*/ struct  {
    u64 entflags;
    u16 index; // constIndex for entity type, used for indexing into arrays for resourec types when loading resources
    Vector3 position;
    float radius,shadRadius;
    Vector3 scale,forward,right;
    Quaternion rotation;
    
    // Rendering
    u16 modelIndex,texIndex,glowIndex,specIndex,normIndex,lodIndex;
    bool cardchunk,kinematic,shadows;
    u8 camView;
    
    // Physics
    u32 layer;
    Vector3 velocity,angularVelocity,lastPosition;
    float gravity;
    BodyState bodyState;
    ColliderType collider;
    Vector3 colliderCenter; // Offset relative to .position's global worldspace xyz location
    Vector3 colliderSize; // x,y,z for Box, x for Sphere radius, else x, y, z for Capsule radius, height, and direction (0.0f = X-Axis, 1.0f = Y-Axis, 2.0f = Z-Axis respectively, default 1.0f)
    u16 colliderMeshIndex;
    Vector3 topPoint,targetPosition,startPosition,activatedScale,direction;
    float targetPositionY,speed,percentAjar,percentMoved;
    FuncStates startState,funcState;
    float mass,linearDrag,angularDrag,inertia;
    Vector3 accumulatedForce,accumulatedTorque;
    float dynamicFriction,staticFriction,bounciness;
    PhysCombineType frictionCombine,bounceCombine;
    float volume; // Audio
    
    // Logic and I/O
    u64 ioflags;
    float health,lastHealth,cyberHealth;
    u8 securityThreshold,lerpUp;
    char targetname[TARGET_STRING_LENGTH];
    char target[TARGET_STRING_LENGTH];
    char target2[TARGET_STRING_LENGTH];
    char currenttarget[TARGET_STRING_LENGTH];
    char targetIfFalse[TARGET_STRING_LENGTH];
    char argvalue[TARGET_STRING_LENGTH];
    u16 enemy,altTexIndex,altGlowIndex,messageIndex,teleportID,targetDestinationID;
    i16 version,SFXIndex,SFXLockedIndex,textIndex,emailIndex;
    SoftwareType type;
    float delay,damage,itemLifeTime,minutes,seconds,randomMin,randomMax;
    double timeInterval,cyberTimer,intervalFinished,delayFireFinished,delayResetFinished;
    bool searchableInUse,generateContents,generationDone,dontReset,onlyOnce,ignoreSecondaryTriggers,allDone,currentTexture,useRandomTimes,active; // Lawdy, we'll make these bitflags someday
    bool touchEnabled,broken,stayOpen,startOpen,ajar,blocked,targetAlreadyDone,accessCardUsedByPlayer,toggleLasers,targettingOnlyUnlocks;
    bool changeLayerOnOpenClose,despawnInstead,doSelfAfterList,destroyAfterListInsteadOfDeactivate,iceActive;
    bool forceFieldDirectionX,forceFieldDirectionY,forceFieldDirectionZ,heldObjectLoadedAlternate,changeTexOnActive,blinkTexOnActive;
    i16 numPlayers;
    u16 recentMostActivator;
    u16 countToTrigger;
    u16 counter;
    u8 maxRandomItems; // [0 4]
    u16 lookUpIndex,contents[4],customIndex[4],useableItemIndex,usableCustomIndex,randomItem[4],randomItemCustomIndex[4];
    float randomItemDropChance[4];
    float fireworkWaitMinMin;
    AttackType attackType;
    i16 ammo,ammo2;
    u16 activateSFX,lockedSFX,messageLingdex,lockedMessageLingdex;
    AccessCardType requiredAccessCard;
    double delayFinished,tickFinished,tickTime,useFinished,waitBeforeClose,lasersFinished;
    float amount,resetTime,minSecurityLevel,ajarPercentage,useTimeDelay,timeBeforeLasersOn,force,strength;
    float offStrengthFactor;
    float distancePaddingToTopPoint;
    double initialBurstFinished,justUsed,timerFinished;
    BloodType bloodType;
    DoorState doorOpen;
    ForceFieldColor fieldColor;
    bool lerping,onlyTargetOnce,autoPlayEmail,inCyberTube,noiseFinished;
    TrackType trackType;
    MusicType musicType;

    // Animation
    u8 clip,numclips,texAnimClip;
    u16 animationNum,frame,texFrame,texGlowFrame;
    bool textureAnimating,textureGlowAnimating,textureAnimationStopsAtDead,texAnimInReverse,texAnimRandom;
    u16 texAnimLight;
    u16 texAnimLight2;
    i32 cellIndex;
    i16 cellX,cellZ;
    u8 portalIndex; // If this is a door, index into portal array for toggling state.
    DoorState doorState;
    double currentFrameFinished;
    double currentFrameStartTime;
    double animSwapFinished;
    bool alternateOn;
    u16 mainSwitchMaterial;
    AIState currentState; // NPC logic
    u16 deathBurst;
    u8 walkWaypointsLength,currentWaypoint;
    double timeForTranquilization,gracePeriodFinished,meleeDamageFinished,idleTime,attack1SoundTime,attack2SoundTime,attack3SoundTime;
    double timeTillEnemyChangeFinished,timeTillDeadFinished,timeTillPainFinished,huntFinished;
    double randomWaitForNextAttack1Finished,randomWaitForNextAttack2Finished,randomWaitForNextAttack3Finished;
    double attackFinished,attack2Finished,attack3Finished,deathBurstFinished,tranquilizeFinished,wanderFinished,timeSinceMovedEnough,posCheckFinished;
    Vector3 currentDestination,lastKnownEnemyPos,targettingPosition,idealTransformForward,idealPos,walkWaypoints[MAX_WAYPOINTS];
    char targetID[TARGET_ID_LENGTH],texAnimResourceFolder[TARGET_STRING_LENGTH],path[TARGET_STRING_LENGTH];
    // phew what a porker of a struct, it's been a eatin!
} Entity;

typedef struct { Entity* entries; u32 count; u32 capacity; } DataParser;
typedef struct { u8 dataType; const char* fieldName; } EntityField;
typedef __builtin_va_list va_list;
typedef struct { char _opaque[128]; } ma_sound;
typedef int ma_bool32;
#define MA_TRUE  1
#define MA_FALSE 0
typedef struct {
    u32 globalFrameNum;
    u16 loadedInstances,loadedLights; // Numbers of instances of entities and lights loaded (always for just the current level)
    float farPlane;
	double cpuTime, thisFrameTime, cpuFrameTime, lastFrameSecCountTime;
	u32 lastFrameSecCount, framesPerLastSecond, worstFPS;
    i32 cursorPosition_x, cursorPosition_y; // Separate internal cursor from system cursor.  This gets relatively pushed around by real cursor movement to give consistent platform behavior.
	Vector3 debugLine_start;
	Vector3 debugLine_end;
	double debugLineFinished;
	u32 debugLineVertCount;
	bool inventoryMode;
	double last_time, last_topframe_time, last_physics_time, deltaTime, current_time, timeSinceLastPhysicsTick;
	double screenshotTimeout, pauseRelativeTime, absoluteTime, statusTextDecayFinished, justSavedTimeStamp;
	bool levelCurrentlyLoading;
    double shakeFinished;
	char global_modname[256];
	char global_dllname[256];
	char global_winicon[256];
    bool introNotPlayed;
    u8 levelSecurity[14];
	u8 startLevel;
	u8 numLevels; // Can be set by gamedata.txt
	u8 currentLevel;
	u8 difficultyCombat,difficultyPuzzle,difficultyMission,difficultyCyber;
	bool gamePaused,menuActive,gameFinished;
	u16 ressurections,deaths,kills,cyberkills,ressurectionActiveLevels;
	u32 shotsFired,grenadesThrown;
	float damageDealt,damageReceived;
	u32 savesScummed;
	u8 creditsPageIndex;
	bool creditsActive,decoyActive,boosterActive,uiIsBlocking,mouseClickHeldOverGUI;
	char playerName[27];
    int fogFac;
    bool (*GetKey)(int settingIndex);
    bool (*GetKeyPressed)(int settingIndex);
    InventorySystem invP1,invP2;
    float timeScale;
    bool  geniusActive;
    Vector3 cyberspaceRecallPoint;
    Entity instances[INSTANCE_COUNT];
    u8 dirtyInstances[INSTANCE_COUNT];
    Color fogColor;
    char audiologNames[TEXT_LOGS_COUNT][TEXT_LOCALIZATION_MAX_LENGTH],audiologSubjects[TEXT_LOGS_COUNT][TEXT_LOCALIZATION_MAX_LENGTH];
    char audiologSenders[TEXT_LOGS_COUNT][TEXT_LOCALIZATION_MAX_LENGTH],audioLogSpeech2Text[TEXT_LOGS_COUNT][TEXT_LOCALIZATION_MAX_LENGTH];
    float worldMin_x,worldMin_z,voxelMinCenterX,voxelMinCenterZ;
    u8 physicsDebug;
} GlobalContext;

static inline __attribute__((always_inline)) void flag_setu16(u16 *flags, u16 bit, bool state) { *flags = (*flags & ~bit) | (-state & bit); }
static inline __attribute__((always_inline)) void flag_setu32(u32 *flags, u32 bit, bool state) { *flags = (*flags & ~bit) | (-state & bit); }
static inline __attribute__((always_inline)) void flag_set(u64 *flags, u64 bit, bool state) { *flags = (*flags & ~bit) | (-state & bit); }

// Math, Vectors, Quaternions
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
    x = vabs(x); float ret = (-0.0187293f * x + 0.0742610f) * x - 0.2121144f; ret = (ret * x + 1.5707288f) * vsqrtf(1.0f - x); ret = ret - 2.0f * negate * ret;
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
static inline __attribute__((always_inline)) i32 clamp(i32 val, i32 min, i32 max) { return (val > max) ? max : ((val < min) ? min : val); }
static inline __attribute__((always_inline)) float vround(float val) { return (val >= 0.0f) ? (float)(int)(val + 0.5f) : (float)(int)(val - 0.5f); }
static inline __attribute__((always_inline)) Vector3 Vector3_A_plus_B(Vector3 a, Vector3 b) { return (Vector3){a.x + b.x, a.y + b.y, a.z + b.z}; }
static inline __attribute__((always_inline)) Vector3 Vector3_A_minus_B(Vector3 a, Vector3 b) { return (Vector3){a.x - b.x, a.y - b.y, a.z - b.z}; }
static inline __attribute__((always_inline)) Vector3 scale_vector3(Vector3 v, float s) { return (Vector3){v.x * s, v.y * s, v.z * s}; }
static inline __attribute__((always_inline)) Vector3 mul_v3_v3_elementwise(Vector3 v, Vector3 w) { return (Vector3){v.x * w.x, v.y * w.y, v.z * w.z}; }
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
static inline __attribute__((always_inline)) Quaternion quat_multiply(Quaternion q1, Quaternion q2) { return (Quaternion){(q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y),(q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x),(q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w),(q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z)}; } // Hamilton product, rotates q1 by q2
static inline __attribute__((always_inline)) Vector3 quat_rotate_vector(Quaternion q, Vector3 v) { Quaternion r = quat_multiply((quat_multiply(q, (Quaternion){v.x,v.y,v.z,0.0f})),(Quaternion){-q.x,-q.y,-q.z,q.w}); return (Vector3){r.x,r.y,r.z}; } // Returns rotated input vector rotated by a quaternion.
static inline __attribute__((always_inline)) u8 hardware14fromConstdex(u16 c) { return clamp(c - 21,0,14); }
static inline __attribute__((always_inline)) bool ConstIndexIsPortalBlockingDoor(u16 entIdx) { return (entIdx >= 496 && entIdx <= 514 && entIdx != 502 && entIdx != 505 && entIdx != 506 && entIdx != 507); }// All doors except see-through doors.
static inline __attribute__((always_inline)) bool ConstIndexInBounds(int c) { return (c >= 0 && c <= 760); }
static inline __attribute__((always_inline)) bool ConstIndexIsGeometry(int c) { return (c >= 0 && c <= 306 && c != 112 && c != 279) || c == 760; }
static inline __attribute__((always_inline)) bool ConstIndexIsDoor(int c) { return (c >= 496 && c < 515); }
static inline __attribute__((always_inline)) bool ConstIndexIsLightStaticSaveable(int c) { return c == 748; }
static inline __attribute__((always_inline)) bool ConstIndexIsGenericTransform(int c) { return c == 749; }
static inline __attribute__((always_inline)) bool ConstIndexIsNPC(int c) { return (c >= 419 && c < 448); }
static inline __attribute__((always_inline)) bool ConstIndexIsHardware(int c) { return (c >= 328) && (c <= 339); }
static inline __attribute__((always_inline)) bool ConstIndexIsAmbient(int c) { return (c >= 621 && c <= 655); }
static inline __attribute__((always_inline)) bool ConstIndexIsButtonSwitch(int c) { return ((c >= 688 && c <= 692) || c == 694 || c == 695); }
static inline __attribute__((always_inline)) bool ConstIndexIsSearchable(int c) { return ((c >= 464 && c <= 476) || c == 530 || c == 531); }
static inline __attribute__((always_inline)) bool ConstIndexIsUsableObject(u16 c) { return ((c >= 307 && c <= 404) || c == 417); }
static inline __attribute__((always_inline)) bool ConstIndexIsAccessCard(u16 c) { return ((c >= 388 && c <= 398) || c == 417); }
static inline __attribute__((always_inline)) bool ConstIndexIsDynamicObject(u16 c) { return (c >= 307 && c <= 404) ||  c == 417 || (c >= 419 && c <= 428) || (c >= 430 && c <= 437) || (c >= 440 && c <= 442) || (c >= 458 && c <= 463) || (c >= 465 && c <= 476); }
static inline __attribute__((always_inline)) bool ConstIndexIsStaticObjectSaveable(int c) { return (c == 112 || c == 279 || (c >= 448 && c < 458) || c == 480 || c == 516 || (c >= 518 && c <= 526) || c == 530 || c == 531 || c == 546 || c == 555 || c == 594 || c == 596 || c == 598 || (c >= 600 && c < 603) || (c >= 604 && c < 616) || (c >= 688 && c < 693) || c == 694 || c == 695 || (c >= 699 && c < 704) || (c >= 741 && c < 746)); }
static inline __attribute__((always_inline)) bool ConstIndexIsStaticObjectImmutable(int c) { return ((c >= 527 && c < 530) || (c >= 532 && c < 546) || (c >= 547 && c < 553) || c == 554 || (c >= 556 && c < 594) || c == 595 || c == 597 || c == 599 || c == 601 || c == 603 || (c >= 616 && c < 688) || c == 693 || c == 696 || c == 697 || c == 698 || (c >= 704 && c < 717) || c == 720 || (c >= 733 && c < 736) || (c >= 737 && c < 739) || c == 746 || c == 747 || (c >= 750 && c <= 759 && c != 755)); }
static inline __attribute__((always_inline)) void* MemSetToValueForNBytes(void *dst, int c, size_t n) { unsigned char *p=(unsigned char *)dst; unsigned char v=(unsigned char)c; while (n--) {*p++=v;} return dst; } // memset replacement
static inline __attribute__((always_inline)) void* CopyMemoryFromBtoAForNBytes(void *dst, const void *src, size_t n) { unsigned char *d=(unsigned char *)dst; const unsigned char *s=(const unsigned char *)src; while (n--) {*d++=*s++;} return dst; } // memcpy replacement
static inline __attribute__((always_inline)) int CompareMemoryForNBytes(const void *s1, const void *s2, size_t n) { const unsigned char *p1 = (const unsigned char *)s1; const unsigned char *p2 = (const unsigned char *)s2; while (n--) { if (*p1 != *p2) {return *p1 - *p2;} p1++; p2++; } return 0; } // memcmp replacement
static inline __attribute__((always_inline)) void* MoveMemoryFromBtoAForNBytes(void *dst, const void *src, size_t n) {
    unsigned char *d = (unsigned char *)dst; const unsigned char *s = (const unsigned char *)src;
    if (d < s) {
        while (n--) { *d++ = *s++; }
    } else if (d > s) { d += n; s += n; while (n--) { *--d = *--s; } }
    return dst;
} // memmove replacement

// static float vexp(float x) { return vexp2f(x * 1.4426950409f); } // 1/ln(2)
// static float vpow(float a, float b) { return vexp(b * vlog(a)); }
// static i32 clamp(i32 val, i32 min, i32 max) { return (val > max) ? max : ((val < min) ? min : val); }
// static float vround(float val) { return (val >= 0.0f) ? (float)(int)(val + 0.5f) : (float)(int)(val - 0.5f); }
// static Vector3 Vector3_A_plus_B(Vector3 a, Vector3 b) { return (Vector3){a.x + b.x, a.y + b.y, a.z + b.z}; }
// static Vector3 Vector3_A_minus_B(Vector3 a, Vector3 b) { return (Vector3){a.x - b.x, a.y - b.y, a.z - b.z}; }
// static Vector3 scale_vector3(Vector3 v, float s) { return (Vector3){v.x * s, v.y * s, v.z * s}; }
// static Vector3 mul_v3_v3_elementwise(Vector3 v, Vector3 w) { return (Vector3){v.x * w.x, v.y * w.y, v.z * w.z}; }
// static float dot(float x1, float y1, float z1, float x2, float y2, float z2) { return x1*x2 + y1*y2 + z1*z2; }
// static float dot_vector3(Vector3 a, Vector3 b) { return dot(a.x,a.y,a.z, b.x,b.y,b.z); }
// static float quat_dot(Quaternion a, Quaternion b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }
// static float magnitude_vector3(const Vector3 v) { return vsqrtf(dot_vector3(v, v)); }
// static Vector3 min_vector3(Vector3 a, Vector3 b) { return (Vector3){ a.x<b.x ? a.x : b.x, a.y<b.y ? a.y : b.y, a.z<b.z ? a.z : b.z }; }
// static Vector3 max_vector3(Vector3 a, Vector3 b) { return (Vector3){ a.x>b.x ? a.x : b.x, a.y>b.y ? a.y : b.y, a.z>b.z ? a.z : b.z }; }
// static float dist_sq_vector3(Vector3 a, Vector3 b) { Vector3 d = Vector3_A_minus_B(a, b); return dot_vector3(d, d); }
// static float distance_vector3(Vector3 a, Vector3 b) { return magnitude_vector3(Vector3_A_minus_B(a, b)); }
// static Vector3 cross_vector3(Vector3 a, Vector3 b) { return (Vector3){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }
// static void normalize_vector(float* x, float* y, float* z) { float len = vsqrtf(*x * *x + *y * *y + *z * *z); if (len > 1e-6f) { *x /= len; *y /= len; *z /= len; } }
// static Vector3 normalize_vector3(Vector3 v) { float len = magnitude_vector3(v); return len > 0.000001f ? (Vector3){v.x / len, v.y / len, v.z / len} : v; }
// static float squareDistance2D(float x1, float z1, float x2, float z2) { float dx = x2 - x1; float dz = z2 - z1; return dx * dx + dz * dz; }
// static float squareDistance3D(float x1, float y1, float z1, float x2, float y2, float z2) { float dx = x2 - x1; float dy = y2 - y1; float dz = z2 - z1; return dx * dx + dy * dy + dz * dz; }
// static Quaternion quat_multiply(Quaternion q1, Quaternion q2) { return (Quaternion){(q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y),(q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x),(q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w),(q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z)}; } // Hamilton product, rotates q1 by q2
// static Vector3 quat_rotate_vector(Quaternion q, Vector3 v) { Quaternion r = quat_multiply((quat_multiply(q, (Quaternion){v.x,v.y,v.z,0.0f})),(Quaternion){-q.x,-q.y,-q.z,q.w}); return (Vector3){r.x,r.y,r.z}; } // Returns rotated input vector rotated by a quaternion.
// static u8 hardware14fromConstdex(u16 c) { return clamp(c - 21,0,14); }
// static bool ConstIndexIsPortalBlockingDoor(u16 entIdx) { return (entIdx >= 496 && entIdx <= 514 && entIdx != 502 && entIdx != 505 && entIdx != 506 && entIdx != 507); }// All doors except see-through doors.
// static bool ConstIndexInBounds(int c) { return (c >= 0 && c <= 760); }
// static bool ConstIndexIsGeometry(int c) { return (c >= 0 && c <= 306 && c != 112 && c != 279) || c == 760; }
// static bool ConstIndexIsDoor(int c) { return (c >= 496 && c < 515); }
// static bool ConstIndexIsLightStaticSaveable(int c) { return c == 748; }
// static bool ConstIndexIsGenericTransform(int c) { return c == 749; }
// static bool ConstIndexIsNPC(int c) { return (c >= 419 && c < 448); }
// static bool ConstIndexIsHardware(int c) { return (c >= 328) && (c <= 339); }
// static bool ConstIndexIsAmbient(int c) { return (c >= 621 && c <= 655); }
// static bool ConstIndexIsButtonSwitch(int c) { return ((c >= 688 && c <= 692) || c == 694 || c == 695); }
// static bool ConstIndexIsSearchable(int c) { return ((c >= 464 && c <= 476) || c == 530 || c == 531); }
// static bool ConstIndexIsUsableObject(u16 c) { return ((c >= 307 && c <= 404) || c == 417); }
// static bool ConstIndexIsAccessCard(u16 c) { return ((c >= 388 && c <= 398) || c == 417); }
// static bool ConstIndexIsDynamicObject(u16 c) { return (c >= 307 && c <= 404) ||  c == 417 || (c >= 419 && c <= 428) || (c >= 430 && c <= 437) || (c >= 440 && c <= 442) || (c >= 458 && c <= 463) || (c >= 465 && c <= 476); }
// static bool ConstIndexIsStaticObjectSaveable(int c) { return (c == 112 || c == 279 || (c >= 448 && c < 458) || c == 480 || c == 516 || (c >= 518 && c <= 526) || c == 530 || c == 531 || c == 546 || c == 555 || c == 594 || c == 596 || c == 598 || (c >= 600 && c < 603) || (c >= 604 && c < 616) || (c >= 688 && c < 693) || c == 694 || c == 695 || (c >= 699 && c < 704) || (c >= 741 && c < 746)); }
// static bool ConstIndexIsStaticObjectImmutable(int c) { return ((c >= 527 && c < 530) || (c >= 532 && c < 546) || (c >= 547 && c < 553) || c == 554 || (c >= 556 && c < 594) || c == 595 || c == 597 || c == 599 || c == 601 || c == 603 || (c >= 616 && c < 688) || c == 693 || c == 696 || c == 697 || c == 698 || (c >= 704 && c < 717) || c == 720 || (c >= 733 && c < 736) || (c >= 737 && c < 739) || c == 746 || c == 747 || (c >= 750 && c <= 759 && c != 755)); }
// static void* MemSetToValueForNBytes(void *dst, int c, size_t n) { unsigned char *p=(unsigned char *)dst; unsigned char v=(unsigned char)c; while (n--) {*p++=v;} return dst; } // memset replacement
// static void* CopyMemoryFromBtoAForNBytes(void *dst, const void *src, size_t n) { unsigned char *d=(unsigned char *)dst; const unsigned char *s=(const unsigned char *)src; while (n--) {*d++=*s++;} return dst; } // memcpy replacement
// static int CompareMemoryForNBytes(const void *s1, const void *s2, size_t n) { const unsigned char *p1 = (const unsigned char *)s1; const unsigned char *p2 = (const unsigned char *)s2; while (n--) { if (*p1 != *p2) {return *p1 - *p2;} p1++; p2++; } return 0; } // memcmp replacement
// static void* MoveMemoryFromBtoAForNBytes(void *dst, const void *src, size_t n) {
//     unsigned char *d = (unsigned char *)dst; const unsigned char *s = (const unsigned char *)src;
//     if (d < s) {
//         while (n--) { *d++ = *s++; }
//     } else if (d > s) { d += n; s += n; while (n--) { *--d = *--s; } }
//     return dst;
// } // memmove replacement

