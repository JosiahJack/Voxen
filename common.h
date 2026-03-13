// common.h - Shared includes between engine and gamecode (e.g. enums)
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
typedef struct { float r,g,b,a; } Color;
typedef struct { float x,y; } Vector2;
typedef struct { float x,y,z; } Vector3;
typedef struct { float x,y,z,w; } Quaternion;
typedef uint8_t PhysCombineType;
typedef uint8_t ColliderType;
typedef uint16_t Text;
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
#define INSTANCE_COUNT 10240 // Max 5454 for Citadel level 7 geometry, Max 295 for Citadel level 1 dynamic objects, 1561 lights, extras for dynamically spawned objects/lights
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
#define MAX_CHILD_COUNT 4
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
#define MAX_WAYPOINTS 32
#define TARGET_ID_LENGTH 32 // Max needed 22 + 5 for ID + 1 for space between them = 28

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

typedef struct {
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
    bool boosterActive;
    bool (*GetKey)(int settingIndex);
    bool (*GetKeyPressed)(int settingIndex);
    Entity instances[INSTANCE_COUNT];
} GlobalContext;

static inline __attribute__((always_inline)) void flag_set(uint64_t *flags, uint64_t bit, bool state) { *flags = (*flags & ~bit) | (-state & bit); }

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
