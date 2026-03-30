// common.h - Shared items between engine and gamecode (e.g. enums)
typedef __INT8_TYPE__     int8_t;
typedef __UINT8_TYPE__   uint8_t;
typedef __INT16_TYPE__   int16_t;
typedef __UINT16_TYPE__ uint16_t;
typedef __INT32_TYPE__   int32_t;
typedef __UINT32_TYPE__ uint32_t;
typedef __INT64_TYPE__   int64_t;
typedef __UINT64_TYPE__ uint64_t;
typedef uint64_t size_t;
typedef uint64_t size_t;
#ifndef UINT8_MAX
    #define UINT8_MAX 255
#endif
#ifndef UINT16_MAX
    #define UINT16_MAX 65535
#endif
#ifndef UINT32_MAX
    #define UINT32_MAX 4294967295
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
typedef uint8_t PhysCombineType;
typedef uint8_t ColliderType;
typedef uint16_t Text;
typedef struct { Vector3 point; Vector3 normal; float distance; uint16_t hitInstanceIndex; bool hit; } RaycastHit;
typedef struct { float speed; uint16_t frameStart; uint16_t frameEnd; uint16_t frameStartModelIndex; uint8_t framerate; } AnimationClip;
typedef struct { uint16_t x,z; } PortalCell;
typedef struct {
    PortalCell cellA;    // one side (usually the cell the door happened to just barely floating point rounding error start in)
    PortalCell cellB;    // tother side
    bool     portalNS; // true when the two cells share N or S edge, else they share E and W edges.
    bool     open;     // door is open
    bool     dirty;
} Portal;

// Make sure these match in chunk.glsl shader!
#define LIGHT_MAX_INTENSITY 8.0f
#define LIGHT_RANGE_MAX 15.36f
#define LIGHT_RANGE_MAX_SQUARED (LIGHT_RANGE_MAX * LIGHT_RANGE_MAX)
#define LIGHTON 1
#define SHADON  2
#define LIGHT_AND_SHADOW_ON 3
#define LSPOT   4
#define LDIR    8
#define LDIRTY 16
#define LERPON 32
typedef struct {
    Vector3 pos;        // 12
    float intensity;    // 4
    Color3 col;         // 12
    uint32_t lflags;    // 4 - light on 1b, shadows on 1b, type 2b, dirty 1b, lerp on 1b
    float range;        // 4
    float spotAng;      // 4
    float maxIntensity; // 4
    float minIntensity; // 4
    Quaternion spotDir; // 16
} Light; // 64bytes, one cache line, packed for GL transfer

typedef struct {
    float lerpValue;
    float lerpStepTime;
    float lerpStartTime;
    float lerpTime;
    float intervalSteps[32];
    bool stepIsLerping[32];
    bool lerpUp;
    uint8_t currentStep;
    uint8_t numIntervalSteps;
    uint8_t numLerpSteps;
} LightAnimation; // Separate from main lights buffer struct since it's not used very often

#define INSTANCE_COUNT 20480 // Max 5454 for Citadel level 7 geometry, Max 295 for Citadel level 1 dynamic objects, 1561 lights, extras for dynamically spawned objects/lights
#define LIGHT_COUNT 2048
#define MODEL_IDX_MAX 6805
#define MAX_VALID_TEXTURE 2048
#define MAX_TOTAL_PIXELS 67108864u
#define MAX_UNIQUE_COLORS 1048576u
#define MAX_ANIMATED_MODELS 64
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
#define FROB_DISTANCE 4.9f
#define ELEVATOR_PAD_TETHER_DIST 2.0f
#define PLAYER_CAPSULE_TOTAL_HEIGHT 2.0f
#define PLAYER_CAPSULE_RADIUS 0.48f
#define LEVEL_CYBERSPACE 13
#define MAX_ENTITIES 768 // Unique entity types, different than INSTANCE_COUNT which is the number of instances of any of these entities.
#define NULLENT 0u
#define WORLD   0u // Much like Quake, the world is entity 0.  Aand also like Quake, world is nullent and is 0.
#define PLAYER1 1u
#define PLAYER2 2u
#define MAX_CHILD_COUNT 2
#define START_INDEX_LEVEL_INSTANCES 3
#define CELL_SIZE 2.56f // Each cell is 2.56x2.56
#define VOXEL_SIZE 0.32f
#define VOXEL_HALF (VOXEL_SIZE * 0.5f)
#define ENG_ACTIVE               (1ull <<  0) // Instance renders and updates
#define ENG_GROUNDED             (1ull <<  1)
#define ENG_RIGIDBODY            (1ull <<  2)

#define ENTFLAG_ACTIVE               (1ull <<  0) // Instance renders and updates
#define ENTFLAG_CARDCHUNK            (1ull <<  1)
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

typedef uint32_t PhysicsLayer;
static const uint32_t PhysicsLayer_Default          = (1u << 0);
static const uint32_t PhysicsLayer_TransparentFX    = (1u << 1);
static const uint32_t PhysicsLayer_IgnoreRaycast    = (1u << 2);
//                                                   (1u << 3)  // unused
static const uint32_t PhysicsLayer_Water            = (1u << 4);
static const uint32_t PhysicsLayer_BlocksRaycast    = (1u << 4); // same as Water
static const uint32_t PhysicsLayer_UI               = (1u << 5);
//                                                   (1u << 6)  // unused
//                                                   (1u << 7)  // unused
static const uint32_t PhysicsLayer_GunViewModel     = (1u << 8);
static const uint32_t PhysicsLayer_Geometry         = (1u << 9);
static const uint32_t PhysicsLayer_NPC              = (1u << 10);
static const uint32_t PhysicsLayer_PlayerBullets    = (1u << 11);
static const uint32_t PhysicsLayer_Player           = (1u << 12);
static const uint32_t PhysicsLayer_Corpse           = (1u << 13);
static const uint32_t PhysicsLayer_PhysObjects      = (1u << 14);
static const uint32_t PhysicsLayer_Sky              = (1u << 15);
static const uint32_t PhysicsLayer_PlayerTriggerOnly= (1u << 16);
static const uint32_t PhysicsLayer_Trigger          = (1u << 17);
static const uint32_t PhysicsLayer_Door             = (1u << 18);
static const uint32_t PhysicsLayer_InterDebris      = (1u << 19);
static const uint32_t PhysicsLayer_Player2          = (1u << 20);
static const uint32_t PhysicsLayer_Player3          = (1u << 21);
static const uint32_t PhysicsLayer_Player4          = (1u << 22);
static const uint32_t PhysicsLayer_NPCTrigger       = (1u << 23);
static const uint32_t PhysicsLayer_NPCBullet        = (1u << 24);
static const uint32_t PhysicsLayer_NPCClip          = (1u << 25);
static const uint32_t PhysicsLayer_Clip             = (1u << 26);
static const uint32_t PhysicsLayer_Automap          = (1u << 27);
static const uint32_t PhysicsLayer_Culling          = (1u << 28);
static const uint32_t PhysicsLayer_CorpseSearchable = (1u << 29);
//                                                   (1u << 30) // unused
static const uint32_t PhysicsLayer_NULL             = (1u << 31);

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
    bool mouseClickHeldOverGUI;
    uint8_t elevButtonLevelIdx[8];
    uint16_t elevButtonSpawnIdx[8];
    bool buttonsEnabled[8];
    bool buttonsDarkened[8];
    uint8_t elevCurrentFloor;
} SystemUI;

typedef struct {	
	char stringTable[TEXT_STRING_COUNT][TEXT_LOCALIZATION_MAX_LENGTH]; // Hefty table for localization support.
	uint16_t audioLogImagesRefIndicesLH[TEXT_LOGS_COUNT];
	uint16_t audioLogImagesRefIndicesRH[TEXT_LOGS_COUNT];
	uint8_t audioLogType[TEXT_LOGS_COUNT];
	uint8_t audioLogLevelFound[TEXT_LOGS_COUNT];
	size_t file_size;
	size_t filelog_size;
	uint8_t* file_data;
	uint8_t* filelog_data;
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
    uint32_t accessCardOwned;
    uint8_t hasSoft;
    uint8_t softVersions[7];
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
    uint8_t numweapons;
    bool wepLoadedWithAlternate[7];
    uint8_t currentMagazineAmount[7];
    uint8_t currentMagazineAmount2[7];
    uint32_t wepAmmo[16];
    uint32_t wepAmmoSecondary[16];
    float weaponEnergySetting[16];
    bool justChangedWeap;
    int16_t weaponCurrentPending;
    int16_t weaponIndexPending;
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
    uint16_t heldObjectCustomIndex;
    uint16_t heldObjectAmmo;
    uint16_t heldObjectAmmo2;
    bool heldObjectLoadedAlternate;
    bool holdingObject;
    bool grenadeActive;
    double dropFinished;
    uint16_t weaponIndex;
    uint16_t currentSearchItem;
    float currentEnergyWeaponHeat[7];
    uint8_t grenAmmo[7];
    uint8_t grenConstIndex[7];
    uint8_t grenadeCurrent;
    uint8_t generalInvCurrent;
    uint16_t generalInvIndex;
    uint16_t generalInvCustomIndex[14];
    bool hasNewLogs;
    bool hasNewData;
    uint8_t patchCurrent;
    uint8_t patchCounts[7];
    uint8_t cyberItemIndex;
    float fatigue;
    float radiated;
    float resetAfterDeathTime;
    float energy;
    float maxEnergy;
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
    double painSoundFinished;
    double radSoundFinished;
    double radFXFinished;
    float radAdjust;
    float initialRadiation;
    bool playerDead;
    int16_t ladderState;
    bool staminupActive;
    bool hasLog[134];
    bool readLog[134];
} InventorySystem;

typedef struct {
    float     damage;
    float     penetration;
    float     offense;
    float     armorvalue;
    float     defense;
    float     impactVelocity;
    Vector3   attacknormal;
    Vector3   hitpoint;
    AttackType attackType;
    uint16_t  owner;      // instance index of shooter (player or NPC)
    uint16_t  hitIdx;     // instance index of hit entity
    bool      isOtherNPC;
    bool      berserkActive;
} DamageData;

typedef struct {
    uint16_t index;
    uint16_t modelIndex;
    uint16_t lodIndex;
    uint16_t pad0;
    uint32_t pad1;
    uint32_t pad2; // Pad out to 16byte alignment
    
    Quaternion rotation;
    
    Vector3 position;
    float volume;
    
    Vector3 scale;
    int32_t cellIndex;
    
    Vector3 velocity;
    uint32_t engflags;
    
    Vector3 forward;
    uint16_t texIndex;
    uint16_t glowIndex;
    
    Vector3 right;
    uint16_t specIndex;
    uint16_t normIndex;

} EngineEnt;

#define NUM_ENTITY_FIELDS 34
typedef /*FAT*/ struct  {
    uint64_t entflags;
    uint64_t ioflags;
    uint16_t index; // constIndex for entity type, used for indexing into arrays for resourec types when loading resources
    uint32_t layer;
    
    // Rendering
    uint16_t modelIndex;
    uint16_t texIndex;
    uint16_t altTexIndex;
    uint16_t glowIndex;
    uint16_t altGlowIndex;
    uint16_t specIndex;
    uint16_t normIndex;
    uint16_t lodIndex;
    bool cardchunk;
    bool kinematic;
    bool shadows;
    uint8_t camView;
    
    // Logic and I/O
    char targetname[TARGET_STRING_LENGTH];
    char target[TARGET_STRING_LENGTH];
    char target2[TARGET_STRING_LENGTH];
    char currenttarget[TARGET_STRING_LENGTH];
    char targetIfFalse[TARGET_STRING_LENGTH];
    char argvalue[TARGET_STRING_LENGTH];
    uint8_t securityThreshold;
    uint16_t messageIndex;
    int16_t textIndex;
    SoftwareType type;
    int16_t version;
    uint16_t teleportID;
    uint16_t targetDestinationID;
    int16_t SFXIndex;
    int16_t SFXLockedIndex;
    float delay;
    float damage;
    float itemLifeTime;
    float minutes;
    float seconds;
    float timeInterval;
    float randomMin;
    float randomMax;
    double intervalFinished;
    double delayFireFinished;
    double delayResetFinished;
    bool searchableInUse;
    bool generateContents;
    bool generationDone;
    bool dontReset;
    bool onlyOnce;
    bool ignoreSecondaryTriggers;
    bool allDone;
    bool currentTexture;
    bool useRandomTimes;
    bool active;
    bool touchEnabled;
    bool broken;
    bool stayOpen;
    bool startOpen;
    bool ajar;
    bool blocked;
    bool targetAlreadyDone;
    bool accessCardUsedByPlayer;
    bool toggleLasers;
    bool targettingOnlyUnlocks;
    bool changeLayerOnOpenClose;
    bool despawnInstead;
    bool doSelfAfterList;
    bool destroyAfterListInsteadOfDeactivate;
    bool iceActive;
    bool isDoor;
    bool forceFieldDirectionX;
    bool forceFieldDirectionY;
    bool forceFieldDirectionZ;
    float cyberTimer;
    int16_t numPlayers;
    uint16_t recentMostActivator;
    uint16_t countToTrigger;
    uint16_t counter;
    uint8_t maxRandomItems; // [0 4]
    uint16_t lookUpIndex; // For randomly generating items
    uint16_t contents[4];
    uint16_t customIndex[4];
    uint16_t useableItemIndex;
    uint16_t usableCustomIndex;
    uint16_t randomItem[4];
    uint16_t randomItemCustomIndex[4];
    float randomItemDropChance[4];
    float fireworkWaitMinMin;
    uint8_t lerpUp;
    AttackType attackType;
    int16_t ammo;
    int16_t ammo2;
    bool heldObjectLoadedAlternate;
    bool changeTexOnActive;
    bool blinkTexOnActive;

    uint16_t activateSFX;
    uint16_t lockedSFX;
    float health;
    float lastHealth;
    float cyberHealth;
    uint16_t messageLingdex;
    uint16_t lockedMessageLingdex;
    AccessCardType requiredAccessCard;
    double delayFinished;
    double tickFinished;
    double tickTime;
    double useFinished;
    double waitBeforeClose;
    double lasersFinished;
    float amount;
    float resetTime;
    float minSecurityLevel;
    float ajarPercentage;
    float useTimeDelay;
    float animatorPlaybackTime;
    float timeBeforeLasersOn;
    float force;
    float strength;
    float offStrengthFactor;
    float distancePaddingToTopPoint;
    double initialBurstFinished;
    double justUsed;
    double timerFinished;
    BloodType bloodType;
    DoorState doorOpen;
    ForceFieldColor fieldColor;
    bool lerping;
    TrackType trackType;
    MusicType musicType;
    bool onlyTargetOnce;
    bool autoPlayEmail;
    uint16_t emailIndex;
    bool inCyberTube;
    double noiseFinished;

    // Animation
    uint8_t clip;
    uint8_t numclips;
    uint16_t animationNum; // Global animation identifier into short table of AnimationClip's
    uint16_t frame; // 0 based index into the delta tables, 0 skips delta read and just uses raw modelIndex base pos verts with no delta applied.
    uint8_t  texAnimClip;
    uint16_t texFrame, texGlowFrame;
    bool textureAnimating;
    bool textureGlowAnimating;
    bool textureAnimationStopsAtDead;
    bool texAnimInReverse;
    bool texAnimRandom;
    uint16_t texAnimLight;
    uint16_t texAnimLight2;
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
    ColliderType collider;
    Vector3 colliderCenter; // Offset relative to .position's global worldspace xyz location
    Vector3 colliderSize; // x,y,z for Box, x for Sphere radius, else x, y, z for Capsule radius, height, and direction (0.0f = X-Axis, 1.0f = Y-Axis, 2.0f = Z-Axis respectively, default 1.0f)
    uint16_t colliderMeshIndex;
    Vector3 topPoint;
    Vector3 targetPosition;
    Vector3 startPosition;
    Vector3 activatedScale;
    Vector3 direction;
    float targetPositionY;
    float speed;
    float percentAjar;
    float percentMoved;
    FuncStates startState;
    FuncStates funcState;
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

    // Audio
    float volume;

    // Attachment
    uint16_t   child[MAX_CHILD_COUNT];
    Vector3    child_offset[MAX_CHILD_COUNT];
    Quaternion child_rotation[MAX_CHILD_COUNT];
    Vector3    child_scale[MAX_CHILD_COUNT];

    // NPC logic
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

typedef struct { Entity* entries; uint32_t count; uint32_t capacity; } DataParser;
typedef struct { uint8_t dataType; const char* fieldName; } EntityField;

#include "miniaudio.h"
typedef struct {
    uint32_t globalFrameNum;
    uint16_t loadedInstances; // Number of instances of entities loaded (always for just the current level)
	double cpuTime, thisFrameTime, cpuFrameTime, lastFrameSecCountTime;
	uint32_t lastFrameSecCount, framesPerLastSecond, worstFPS;
    int32_t cursorPosition_x, cursorPosition_y; // Separate internal cursor from system cursor.  This gets relatively pushed around by real cursor movement to give consistent platform behavior.
	Vector3 debugLine_start;
	Vector3 debugLine_end;
	double debugLineFinished;
	uint32_t debugLineVertCount;
	bool inventoryMode;
	double last_time, last_topframe_time, last_physics_time, deltaTime, current_time, timeSinceLastPhysicsTick;
	double screenshotTimeout, pauseRelativeTime, absoluteTime, statusTextDecayFinished, justSavedTimeStamp;
	bool levelCurrentlyLoading;
    double shakeFinished;
	char global_modname[256];
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
    bool boosterActive;
    int fogFac;
    bool uiIsBlocking;
   	bool mouseClickHeldOverGUI;
    bool (*GetKey)(int settingIndex);
    bool (*GetKeyPressed)(int settingIndex);
    uint16_t ressurectionActiveLevels;
    InventorySystem invP1;
    InventorySystem invP2;
    ma_engine audio_engine;
    ma_sound mp3_sounds[2]; // Two for crossfading
    int32_t mp3_slot;
    float timeScale;
    bool  geniusActive;
    Vector3 cyberspaceRecallPoint;
    Entity instances[INSTANCE_COUNT];
    uint8_t dirtyInstances[INSTANCE_COUNT];
    Color fogColor;
    char audiologNames[TEXT_LOGS_COUNT][TEXT_LOCALIZATION_MAX_LENGTH];
    char audiologSubjects[TEXT_LOGS_COUNT][TEXT_LOCALIZATION_MAX_LENGTH];
    char audiologSenders[TEXT_LOGS_COUNT][TEXT_LOCALIZATION_MAX_LENGTH];
    char audioLogSpeech2Text[TEXT_LOGS_COUNT][TEXT_LOCALIZATION_MAX_LENGTH];
    float worldMin_x, worldMin_z;
    float voxelMinCenterX, voxelMinCenterZ;
    uint16_t loadedLights;
} GlobalContext;

static inline __attribute__((always_inline)) void flag_setu32(uint32_t *flags, uint32_t bit, bool state) { *flags = (*flags & ~bit) | (-state & bit); }
static inline __attribute__((always_inline)) void flag_set(uint64_t *flags, uint64_t bit, bool state) { *flags = (*flags & ~bit) | (-state & bit); }
static inline __attribute__((always_inline)) bool EntityIndexIsPortalBlockingDoor(uint16_t entIdx) { return (entIdx >= 496 && entIdx <= 514 && entIdx != 502 && entIdx != 505 && entIdx != 506 && entIdx != 507); }// All doors except see-through doors.

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
static inline __attribute__((always_inline)) int32_t clamp(int32_t val, int32_t min, int32_t max) { return (val > max) ? max : ((val < min) ? min : val); }

// Math, Vectors, Quaternions
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
static inline __attribute__((always_inline)) uint8_t hardware14fromConstdex(uint16_t c) { return clamp(c - 21,0,14); }
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
static inline __attribute__((always_inline)) bool ConstIndexIsUsableObject(uint16_t c) { return ((c >= 307 && c <= 404) || c == 417); }
static inline __attribute__((always_inline)) bool ConstIndexIsAccessCard(uint16_t c) { return ((c >= 388 && c <= 398) || c == 417); }
static inline __attribute__((always_inline)) bool ConstIndexIsDynamicObject(uint16_t c) { return (c >= 307 && c <= 404) ||  c == 417 || (c >= 419 && c <= 428) || (c >= 430 && c <= 437) || (c >= 440 && c <= 442) || (c >= 458 && c <= 463) || (c >= 465 && c <= 476); }
static inline __attribute__((always_inline)) bool ConstIndexIsStaticObjectSaveable(int c) { return (c == 112 || c == 279 || (c >= 448 && c < 458) || c == 480 || c == 516 || (c >= 518 && c <= 526) || c == 530 || c == 531 || c == 546 || c == 555 || c == 594 || c == 596 || c == 598 || (c >= 600 && c < 603) || (c >= 604 && c < 616) || (c >= 688 && c < 693) || c == 694 || c == 695 || (c >= 699 && c < 704) || (c >= 741 && c < 746)); }
static inline __attribute__((always_inline)) bool ConstIndexIsStaticObjectImmutable(int c) {
	return ((c >= 527 && c < 530) || (c >= 532 && c < 546) || (c >= 547 && c < 553)
			|| c == 554 || (c >= 556 && c < 594) || c == 595 || c == 597 || c == 599
			|| c == 601 || c == 603 || (c >= 616 && c < 688) || c == 693 || c == 696 || c == 697
			|| c == 698 || (c >= 704 && c < 717) || c == 720 || (c >= 733 && c < 736)
			|| (c >= 737 && c < 739) || c == 746 || c == 747 || (c >= 750 && c <= 759 && c != 755));
}
