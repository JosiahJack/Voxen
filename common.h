// common.h - Shared items between engine and gamecode (e.g. enums)
#define INLINE static inline __attribute__((always_inline))
typedef __INT8_TYPE__   i8; typedef  __UINT8_TYPE__  u8; //  8bit types
typedef __INT16_TYPE__ i16; typedef __UINT16_TYPE__ u16; typedef u16 half; // 16bit types
typedef __INT32_TYPE__ i32; typedef __UINT32_TYPE__ u32; // 32bit types
typedef __INT64_TYPE__ i64; typedef __UINT64_TYPE__ u64; typedef __SIZE_TYPE__ size_t; // 64bit types
#ifndef U8_MAX
    #define U8_MAX 255
#endif
#ifndef U16_MAX
    #define U16_MAX 65535
#endif
#define bool u8
#define true 1
#define false 0
typedef struct { float r,g,b; } Color3; typedef struct { float r,g,b,a; } Color;
typedef struct { float x,y; } V2;  typedef struct { float x,y,z; } V3; typedef struct { float x,y,z,w; } Quaternion;
#define QUAT_IDENTITY ((Quaternion){0.0f,0.0f,0.0f,1.0f})
typedef u8 ColliderType;
typedef u16 Text;
typedef struct {V3 point; V3 normal; float distance; u16 hitInstanceIndex; bool hit;} RaycastHit;
typedef struct {float speed; u16 frameStart,frameEnd,frameStartModelIndex; u8 framerate;} AnimationClip;
#define LIGHTON 1
#define SHADON  2
#define LIGHT_AND_SHADOW_ON 3
#define LSPOT   4
#define LDIR    8
#define LDIRTY 16
#define LERPON 32
typedef struct { V3 pos; float intensity; Color3 col; u32 lflags; float range,spotAng,maxIntensity,minIntensity; Quaternion spotDir; } Light; // 64bytes, one cache line, packed for GL transfer
typedef struct { float lerpValue,lerpStepTime,lerpStartTime,lerpTime,intervalSteps[32]; bool stepIsLerping[32],lerpUp; u8 currentStep,numIntervalSteps,numLerpSteps; } LightAnimation; // Separate from main lights buffer struct since it's not used very often
#define INSTANCE_COUNT 16384 // Max 5454 for Citadel level 7 geometry, Max 295 for Citadel level 1 dynamic objects, 1561 lights, extras for dynamically spawned objects/lights
#define MAX_LEVELS 14 // Total number of levels loaded into RAM simultaneously.  World.levelInstances[MAX_LEVELS][INSTANCE_COUNT] holds them all.
#define LIGHT_COUNT 2048
#define MAX_MDLS 6000
#define MAX_TXRS 2048
#define MAX_TOTAL_PIXELS 29900000u
#define MAX_UNIQUE_COLORS 1048576u
#define MAX_ANIMS 64
#define MAX_ANIMCLIPS 32
#define MAX_WIRELINE_VRTS 512000
#define MAX_PORTALS 64 // Max is 49 on Citadel level 7
#define MAX_KEYS 512
#define MAX_MOUSE_BUTTONS 8
#define MAX_CHANNELS 48 // Max concurrent sounds, must keep track of for volume setting
#define VRT_ATT_SZ 16
#define CPU_VRT_SZ 32
#define DOUBLE_CLICK_TIME 0.5f
#define PLAYER_MAX_WALK_SPEED 3.6f
#define PLAYER_MAX_SPRINT_SPEED 8.8f
#define PLAYER_MAX_CYBER_SPEED 5.0f
#define PLAYER_MAX_CYBER_ULTIMATE_SPEED 12.0f
#define PLAYER_MAX_SPRINT_SPEED_FATIGUED 5.5f
#define PLAYER_MAX_CROUCH_SPEED 1.25f
#define PLAYER_MAX_PRONE_SPEED 0.5f
#define PLAYER_BOOSTER_SPEED_BOOST 1.2f
#define PLAYER_CROUCH_RATIO 0.6f
#define PLAYER_PRONE_RATIO 0.2f
#define PLAYER_TRANSITION_TO_PRONE_ADD 0.10f
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
#define INSTS_1ST_IDX 3
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
#define EF_ACTIVE               (1ull <<  0) // Instance renders and updates
#define EF_ISGRENADE            (1ull <<  1)
#define EF_GROUNDED             (1ull <<  2)
#define EF_RIGIDBODY            (1ull <<  3)
#define EF_NO_SHADOWS           (1ull <<  4)
#define EF_ASLEEP               (1ull <<  5) // Check if enemy starts out asleep such as the sleeping sec-2 bots on level 8 in the maintenance and recharge bays.
#define EF_WALK_PATH_ON_START   (1ull <<  6)
#define EF_TOUCHING_HURTS       (1ull <<  7)
#define EF_ACT_AS_CORPSE_ONLY   (1ull <<  8)
#define EF_DYING                (1ull <<  9)
#define EF_DEATH_BURST_DONE     (1ull << 10)
#define EF_DEAD                 (1ull << 11)
#define EF_TELEPORT_ON_DEATH    (1ull << 12)
#define EF_GO_INTO_PAIN         (1ull << 13)
#define EF_WANDERING            (1ull << 14)
#define EF_ACT_AS_TURRET        (1ull << 15)
#define EF_TARGID_ATTACHED      (1ull << 16)
#define EF_ENEM_IN_SIGHT        (1ull << 17)
#define EF_ENEM_IN_FRONT        (1ull << 18)
#define EF_ENEM_IN_FOV          (1ull << 19)
#define EF_ENEM_IN_LOS          (1ull << 20)
#define EF_FIRST_SIGHTING       (1ull << 21)
#define EF_DYING_SETUP          (1ull << 22)
#define EF_HAD_ENEMY            (1ull << 23)
#define EF_SHOT_FIRED           (1ull << 24)
#define EF_DEAD_CHECKS_DONE     (1ull << 25)
#define EF_HOP_DONE             (1ull << 26)
#define EF_LOCKED               (1ull << 27)
#define EF_HAS_CAMERA_VIEW      (1ull << 28)
#define EF_DAMAGE_ON_USE        (1ull << 29)
#define Q_ROBOT_SPAWN_DEACTIVATED      (1ull <<  0)
#define Q_ISOTOPE_INSTALLED            (1ull <<  1)
#define Q_SHIELD_ACTIVATED             (1ull <<  2)
#define Q_LASER_SAFETY_OVERRIDEN       (1ull <<  3)
#define Q_LASER_DESTROYED              (1ull <<  4)
#define Q_BETA_GROVE_CYBER_UNLOCKED    (1ull <<  5)
#define Q_GROVE_ALPHA_JETTISON_ENABLED (1ull <<  6)
#define Q_GROVE_BETA_JETTISON_ENABLED  (1ull <<  7)
#define Q_GROVE_DELTA_JETTISON_ENABLED (1ull <<  8)
#define Q_MASTER_JETTISON_BROKEN       (1ull <<  9)
#define Q_RELAY_428_FIXED              (1ull << 10)
#define Q_MASTER_JETTISON_ENABLED      (1ull << 11)
#define Q_BETA_GROVE_JETTISONED        (1ull << 12)
#define Q_ANTENNA_NORTH_DESTROYED      (1ull << 13)
#define Q_ANTENNA_SOUTH_DESTROYED      (1ull << 14)
#define Q_ANTENNA_EAST_DESTROYED       (1ull << 15)
#define Q_ANTENNA_WEST_DESTROYED       (1ull << 16)
#define Q_SELF_DESTRUCT_ACTIVATED      (1ull << 17)
#define Q_BRIDGE_SEPARATED             (1ull << 18)
#define Q_ISOLINEAR_CHIPSET_INSTALLED  (1ull << 19)
#define Q_LEV1_CODE_LOCKED             (1ull << 20)
#define Q_LEV2_CODE_LOCKED             (1ull << 21)
#define Q_LEV3_CODE_LOCKED             (1ull << 22)
#define Q_LEV4_CODE_LOCKED             (1ull << 23)
#define Q_LEV5_CODE_LOCKED             (1ull << 24)
#define Q_LEV6_CODE_LOCKED             (1ull << 25)
#define TARG_IOFLAGS_TRIPTRIGGER        (1ull << 0) // Action bits.  What do we want our target to do, e.g. turn on a light or close a door or activate force bridge.  Using multiple bools to allow for multiple actions to be attempted on all the targets.
#define TARG_IOFLAGS_DOOROPEN           (1ull << 1)
#define TARG_IOFLAGS_DOOROPENIFUNLOCKED (1ull << 2)
#define TARG_IOFLAGS_DOORCLOSE          (1ull << 3)
#define TARG_IOFLAGS_LOCK               (1ull << 4)
#define TARG_IOFLAGS_UNLOCK             (1ull << 5)
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
#define TARG_IOFLAGS_SWITCH_LOCK_TOGGLE (1ull << 21)
#define TARG_IOFLAGS_INST_ACTIVATE      (1ull << 22)
#define TARG_IOFLAGS_INST_DEACTIVATE    (1ull << 23)
#define TARG_IOFLAGS_INST_TOGGLE        (1ull << 24)
#define TARG_IOFLAGS_PLAY_SOUND_ONCE    (1ull << 25)
#define TARG_IOFLAGS_STOP_SOUND         (1ull << 26)
#define TARG_IOFLAGS_START_FLASHING_TEX (1ull << 27)
#define TARG_IOFLAGS_STOP_FLASHING_TEX  (1ull << 28)
#define TARG_IOFLAGS_BRANCH_FLIP        (1ull << 29)
#define TARG_IOFLAGS_BRANCH_FLIPONLY    (1ull << 30)
#define TARG_IOFLAGS_DISABLE_ON_AWAKE   (1ull << 31)
#define TARGET_STRING_LENGTH 38
#define CURSOR_SCREEN_PERCENTAGE 0.02f
#define FONT_NORMAL 0
#define FONT_STOPD  1
#define T_WHITE                0
#define T_YELLOW               1
#define T_DARK_YELLOW          2
#define T_GREEN                3
#define T_RED                  4
#define T_ORANGE               5
#define T_STOPD_RED            6
#define T_STOPD_RED_HIGHLIGHT  7
#define T_STOPD_RED_PAUSETITLE 8
#define T_GREEN_MENU           9
#define T_GREEN_MENU_SHADOW   10
#define T_GREEN_MENU_GLOW     11
#define T_RED_MENU            12
#define SAVE_REMINDER_TIME 7.0f // 7secs ~is human short-term memory length
#define CREDITS_PAGES 22
#define TARGET_ID_LENGTH 32 // Max needed 22 + 5 for ID + 1 for space between them = 28
#define SOUNDS_COUNT 670
#define T_DATA_FILEBUFFER_SIZE 65536 // 16 pages
#define T_STRING_COUNT 1100
#define T_LOCALIZATION_MAX_LENGTH 1280
#define T_LOGS_COUNT 134
#define T_BUFFER_SIZE 1024
#define FONT_ATLAS_SIZE 4672
#define MAX_GLYPHS 4096
#define MULTI_MEDIA_TAB_EMAIL_TABLE 0
#define MULTI_MEDIA_TAB_LOG_TABLE   1
#define MULTI_MEDIA_TAB_DATA_TABLE  2
#define MULTI_MEDIA_TAB_NOTES       3
#define ANIM_LOOP_ALL 0
#define ANIM_IDLE_CLOSED 0
#define ANIM_OPENING     1
#define ANIM_IDLE_OPEN   2
#define ANIM_CLOSING     3
#define ANIM_INSTALL     4
#define ANIM_INSTALLED   5
#define ANIM_INACTIVE   0
#define ANIM_ACTIVATE   1
#define ANIM_ACTIVATED  2
#define ANIM_DEACTIVATE 3
#define ANIM_IDLE    0
#define ANIM_WALK    1
#define ANIM_RUN     2
#define ANIM_ATTACK1 3
#define ANIM_ATTACK2 4
#define ANIM_ATTACK3 5
#define ANIM_PAIN    6
#define ANIM_PAIN2   7
#define ANIM_PAIN3   8
#define ANIM_DYING   9
#define ANIM_ATTACK_MISS 1
#define ANIM_ATTACK_HIT  2
#define NUM_AI_TYPES 29
#define MAX_DYNAMIC_ENTITIES 512
#define TERMINAL_VELOCITY 10.0f
#define PHYS_FLOAT_TO_INT_SCALEF 100.0f
#define COLTYPE_NONE 0
#define COLTYPE_BOX 1
#define COLTYPE_SPH 2
#define COLTYPE_CAP 3
#define COLTYPE_CVX 4
#define COLTYPE_MSH 5
#define COLLIDER_CAPSULE_DIRECTION_X_F 0.0f // X-Axis
#define COLLIDER_CAPSULE_DIRECTION_Y_F 1.0f // Y-Axis
#define COLLIDER_CAPSULE_DIRECTION_Z_F 2.0f // Z-Axis
typedef u8 BodyState;        static const u8 BodyState_Standing=0,BodyState_Crouch=1,BodyState_CrouchingDown=2,BodyState_StandingUp=3,BodyState_Prone=4,BodyState_ProningDown=5,BodyState_ProningUp=6;
typedef u8 Handedness;       static const u8 Handedness_Center=0,Handedness_LH=1,Handedness_RH=2;
typedef u8 AttType;       static const u8 Att_None=0,Att_Melee=1,Att_MlEg=2,Att_Beam=3,Att_Magn=4,Att_HitS=5,Att_PjNd=6,Att_PjBm=7,Att_Ball=8,Att_Gas=9,Att_Trnq=10,Att_Drill=11;
typedef u8 NPCType;          static const u8 NPCType_Mutant=0,NPCType_Supermutant=1,NPCType_Robot=2,NPCType_Cyborg=3,NPCType_Supercyborg=4,NPCType_Cyber=5,NPCType_MutantCyborg=6;
typedef u8 PerceptionLevel;  static const u8 PerceptionLevel_Low=0,PerceptionLevel_Medium=1,PerceptionLevel_High=2,PerceptionLevel_Omniscient=3;
typedef u8 AIState;          static const u8 AIState_Idle=0,AIState_Walk=1,AIState_Run=2,AIState_Attack1=3,AIState_Attack2=4,AIState_Attack3=5,AIState_Pain=6,AIState_Dying=7,AIState_Dead=8,AIState_Inspect=9,AIState_Interacting=10;
typedef u8 AIMoveType;       static const u8 AIMoveType_Walk=0,AIMoveType_Fly=1,AIMoveType_Swim=2,AIMoveType_Cyber=3,AIMoveType_None=4;
typedef u8 DoorState;        static const u8 DoorState_Closed=0,DoorState_Open=1,DoorState_Closing=2,DoorState_Opening=3;
typedef u8 FuncStates;       static const u8 FuncStates_Start=0,FuncStates_Target=1,FuncStates_MovingStart=2,FuncStates_MovingTarget=3,FuncStates_AjarMovingStart=4,FuncStates_AjarMovingTarget=5;
typedef u8 AccessCardType;   static const u8 AccessCardType_None=0,AccessCardType_Standard=1,AccessCardType_Medical=2,AccessCardType_Science=3,AccessCardType_Admin=4,AccessCardType_Group1=5,AccessCardType_Group2=6,AccessCardType_Group3=7,AccessCardType_Group4=8,AccessCardType_GroupA=9,AccessCardType_GroupB=10,AccessCardType_Storage=11,AccessCardType_Engineering=12,AccessCardType_Maintenance=13,AccessCardType_Security=14,AccessCardType_Per1=15,AccessCardType_Per2=16,AccessCardType_Per3=17,AccessCardType_Per4=18,AccessCardType_Per5=19;
typedef u8 MusicType;        static const u8 MusicType_None=0,MusicType_Walking=1,MusicType_Combat=2,MusicType_Override=3;
typedef u8 TrackType;        static const u8 TrackType_None=0,TrackType_Walking=1,TrackType_Combat=2,TrackType_Revive=3,TrackType_Death=4,TrackType_Cybertube=5,TrackType_Elevator=6,TrackType_Distortion=7;
typedef u8 BloodType;        static const u8 BloodType_None=0,BloodType_Red=1,BloodType_Yellow=2,BloodType_Green=3,BloodType_Robot=4,BloodType_Leaf=5,BloodType_Mutation=6,BloodType_GrayMutation=7;
typedef u8 SecurityType;     static const u8 SecurityType_None=0,SecurityType_Camera=1,SecurityType_NodeSmall=2,SecurityType_NodeLarge=3;
typedef u8 AudioLogType;     static const u8 AudioLogType_TextOnly=0,AudioLogType_Normal=1,AudioLogType_Email=2,AudioLogType_Papers=3,AudioLogType_Vmail=4,AudioLogType_Game=5;
typedef u8 EnergyType;       static const u8 EnergyType_Battery=0,EnergyType_ChargeStation=1;
typedef u8 FootStepType;     static const u8 FootStepType_None=0,FootStepType_Carpet=1,FootStepType_Concrete=2,FootStepType_GrittyCrete=3,FootStepType_Grass=4,FootStepType_Gravel=5,FootStepType_Rock=6,FootStepType_Glass=7,FootStepType_Marble=8,FootStepType_Metal=9,FootStepType_Grate=10,FootStepType_Metal2=11,FootStepType_Metpanel=12,FootStepType_Panel=13,FootStepType_Plaster=14,FootStepType_Plastic=15,FootStepType_Plastic2=16,FootStepType_Rubber=17,FootStepType_Sand=18,FootStepType_Squish=19,FootStepType_Vent=20,FootStepType_Water=21,FootStepType_Wood=22,FootStepType_Wood2=23;
typedef u8 MusicResourceType;static const u8 MusicResourceType_Menu=0,MusicResourceType_Medical=1,MusicResourceType_Science=2,MusicResourceType_Reactor=3,MusicResourceType_Executive=4,MusicResourceType_Grove=5,MusicResourceType_Cyber=6,MusicResourceType_Security=7,MusicResourceType_Revive=8,MusicResourceType_Death=9,MusicResourceType_Elevator=10,MusicResourceType_Distortion=11,MusicResourceType_Looped=12,MusicResourceType_Level=13;
typedef u8 HUDColor;         static const u8 HUDColor_White=0,HUDColor_Red=1,HUDColor_Orange=2,HUDColor_Yellow=3,HUDColor_Green=4,HUDColor_Blue=5,HUDColor_Purple=6,HUDColor_Gray=7;
typedef u8 ForceFieldColor;  static const u8 ForceFieldColor_Red=0,ForceFieldColor_Green=1,ForceFieldColor_Blue=2,ForceFieldColor_Purple=3,ForceFieldColor_RedFaint=4;
typedef u8 ButtonType;       static const u8 ButtonType_Generic=0,ButtonType_GeneralInv=1,ButtonType_Patch=2,ButtonType_Grenade=3,ButtonType_Weapon=4,ButtonType_Search=5,ButtonType_None=6,ButtonType_PGrid=7,ButtonType_PWire=8,ButtonType_Vaporize=9,ButtonType_ShootMode=10,ButtonType_GrenadeTimerSlider=11;
typedef u8 TabMSG;           static const u8 TabMSG_None=0,TabMSG_Search=1,TabMSG_AudioLog=2,TabMSG_Keypad=3,TabMSG_Elevator=4,TabMSG_GridPuzzle=5,TabMSG_WirePuzzle=6,TabMSG_EReader=7,TabMSG_Weapon=8,TabMSG_SystemAnalyzer=9;
typedef u8 PuzzleCellType;   static const u8 PuzzleCellType_Off=0,PuzzleCellType_Standard=1,PuzzleCellType_And=2,PuzzleCellType_Bypass=3;
typedef u8 PuzzleGridType;   static const u8 PuzzleGridType_King=0,PuzzleGridType_Queen=1,PuzzleGridType_Knight=2,PuzzleGridType_Rook=3,PuzzleGridType_Bishop=4,PuzzleGridType_Pawn=5;
static const u32 L_Default          = 1U;
static const u32 L_TransparentFX    = 2U;
//                                    4U   // unused (formerly IgnoreRaycast)
//                                    8U   // unused
static const u32 L_Water            = 16U; static const u32 L_BlocksRaycast = 16U;  // same as Water
static const u32 L_UI               = 32U;
//                                    64U  // unused
//                                    128U // unused
static const u32 L_GunViewModel     = 256U;
static const u32 L_Geometry         = 512U;
static const u32 L_NPC              = 1024U;
static const u32 L_PlayerBullets    = 2048U;
static const u32 L_Player           = 4096U;
static const u32 L_Corpse           = 8192U;
static const u32 L_PhysObjects      = 16384U;
//                                    32768U // unused (formerly Sky)
static const u32 L_PlayerTriggerOnly= 65536U;
static const u32 L_Trigger          = 131072U;
static const u32 L_Door             = 262144U;
static const u32 L_InterDebris      = 524288U;
static const u32 L_Player2          = 1048576U;
//                                    2097152U // unused (formerly Player3)
//                                    4194304U // unused (formerly Player4)
static const u32 L_NPCTrigger       = 8388608U;
static const u32 L_NPCBullet        = 16777216U;
static const u32 L_NPCClip          = 33554432U;
static const u32 L_Clip             = 67108864U;
static const u32 L_Automap          = 134217728U;
static const u32 L_Culling          = 268435456U;
static const u32 L_CorpseSearchable = 536870912U;
//                                    1073741824U // unused
static const u32 L_NULL             = 2147483648U;
#define LMASK_PLAYER_COLLIDESWITH   (L_Clip|L_NPCBullet|L_Player2|L_Door|L_Trigger|L_PlayerTriggerOnly|L_Default|L_TransparentFX|L_Geometry|L_NPC)
#define LMASK_NPC_COLLIDESWITH      (L_Clip|L_NPCClip|L_PlayerBullets|L_Player2|L_Player|L_Door|L_Trigger|L_NPCTrigger|L_Default|L_TransparentFX|L_Geometry|L_NPC)
#define LMASK_NPC_SIGHT             (L_Default|L_Geometry|L_Door|L_InterDebris|L_PhysObjects|L_Player)
#define LMASK_NPC_ATTACK            (L_Default|L_Geometry|L_NPC|L_Door|L_InterDebris|L_PhysObjects|L_Player)
#define LMASK_NPC_COLLISION         (L_Default|L_TransparentFX|L_Geometry|L_NPC|L_Door|L_InterDebris|L_Player|L_Clip|L_NPCClip|L_PhysObjects)
#define LMASK_PLAYER_FROB           (L_Default|L_Geometry|L_Water|L_Door|L_InterDebris|L_PhysObjects|L_CorpseSearchable)
#define LMASK_PLAYER_TARGET_ID_FROB (L_Default|L_Geometry|L_Door|L_NPC|L_CorpseSearchable)
#define LMASK_PLAYER_ATTACK         (L_Default|L_Geometry|L_NPC|L_PlayerBullets|L_Door|L_InterDebris|L_PhysObjects|L_CorpseSearchable)
#define LMASK_EXPLOSION             (L_Default|L_Geometry|L_NPC|L_PlayerBullets|L_Door|L_InterDebris|L_PhysObjects|L_Player|L_Player2|L_CorpseSearchable)
#define LMASK_PLAYER_FEET           (L_Default|L_Geometry)
typedef struct { i32 InputCodeSettings[42]; u16 ScreenWidth,ScreenHeight; float ScreenCenterX,ScreenCenterY; bool Fullscreen; u8 FOV,Brightness,Gamma,FXAA,Shadows,Reflections,Vsync,ModelDetail,GI,SpeakerMode,Reverb,VolumeMaster,VolumeMusic,VolumeMessage,VolumeEffects,Language,DynamicMusic; u8 Footsteps,InvertLook,InvertInventoryCycling,InvertCyberspaceLook,QuickItemPickup,QuickReloadWeapons,MouseSensitivity,NoShootMode,HeadBob,SSR_RES,CurrentMonitor; } SettingsSystem;
SettingsSystem Sys_Settings = { // Potato defaults so initial state is good on first run for potatoes (e.g. won't crash for out of VRAM, or won't take 5min to init).
    .InputCodeSettings = {
        5, /*Forward=F*/     0,/*Strafe Left=A*/ 18,/*Backpedal=S*/ 3,/*Strafe Right=D*/ 100,/*Jump=SPACE*/ 2,/*Crouch=C*/   23,/*Prone=X*/    16,/*Lean Left=Q*/   4,/*Lean Right=E*/ 45,/*Sprint=LEFT SHIFT*/ 38,/*Turn Left=LF ARROW*/ 39,/*Turn Right=RT ARROW*/ 36,/*Look Up=UP ARROW*/     37,/*Look Down=DN ARROW*/   20,/*Recent Log=U*/    26,/*Biomonitor=1*/
        27,/*Sensaround=2*/ 28,/*Lantern=3*/     29,/*Shield=4*/   30,/*Infrared=5*/      31,/*Email=6*/   32,/*Booster=7*/  33,/*Jumpjets=8*/ 56,/*Attack=LMB*/   57,/*Use=RMB*/      99,/*Menu/Back=ESCAPE*/  97,/*Toggle Mode=TAB*/    17,/*Reload=R*/           128,/*Weapon += MWHEEL + */ 129,/* Weapon - = MWHEEL - */ 6,/* Grenade   = G */ 19,/* Grenade + = T  */
        131,/*Grenade-=*/   21,/*Ammo Type=V*/    9,/*Patch Use=J*/ 8,/*Patch+=I*/       132,/*Patch-=,*/  12,/*Full Map=M*/ 21,/*Swim Up= V*/  2,/*Swim Down=C*/ 102,/*Console=`*/   101/*Screenshot=F12*/},
    .ScreenWidth=800u,.ScreenHeight=600u,.Fullscreen=0u,.FOV=65u,.Brightness=50u,.Gamma=50u,.FXAA=0u,.Shadows=0u,.Reflections=0u,.Vsync=0u,.ModelDetail=0u,.CurrentMonitor=0u,
    .GI=0u,.SpeakerMode=1u,.Reverb=0u,.VolumeMaster=100u,.VolumeMusic=25u,.VolumeMessage=75u,.VolumeEffects=100u,.Language=0u,.DynamicMusic=1u,.Footsteps=1u,.InvertLook=0u,
    .InvertCyberspaceLook=0u,.QuickItemPickup=0u,.QuickReloadWeapons=0u,.MouseSensitivity=10u,.NoShootMode=0u,.HeadBob=1u,.SSR_RES=4u};/*Ratio is (1 / SSR_RES) * res*/

typedef struct { bool god,noclip,notarget,bottomless,superoverride,fatigueCheat,redbull,consoleActive,noHUD,showLocation,showFPS,showPhys,editMode; u8 dizzyLevel; } CheatsSystem;
typedef struct {
        double logFinished,blinkFinished,beepFinished,tickFinished,/*Visual only, Time.time controlled*/ centerTabsTickFinished;/*Visual only, Time.time controlled*/ 
        i32 lastMultiMediaTabOpened,applyButtonReferenceIndex,curCenterTab,wep16index,tempSpriteIndex,count;
        u16 linkedElevatorDoor,tetheredPGP,tetheredPWP,tetheredSearchable,tetheredKeypadElevator,tetheredKeypadKeycode,elevButtonSpawnIdx[8];
        u8 highlightTickCount[4],beepCount,elevButtonLevelIdx[8],elevCurrentFloor;
        bool lastWeaponSideRH,lastItemSideRH,lastAutomapSideRH,lastTargetSideRH,lastDataSideRH,lastSearchSideRH,lastLogSideRH,lastLogSecondarySideRH,lastMinigameSideRH,logActive,paperLogInUse,usingObject,isBlocking,isRH,centerTabNotified[4],highlightStatus[4],audPaused,mouseClickHeldOverGUI,buttonsEnabled[8],buttonsDarkened[8],vmailActive;;
        AudioLogType logType;
        V3 objectInUsePos;
} SystemUI;
typedef struct { char stringTable[T_STRING_COUNT][T_LOCALIZATION_MAX_LENGTH]; u16 audioLogImagesRefIndicesLH[T_LOGS_COUNT],audioLogImagesRefIndicesRH[T_LOGS_COUNT]; u8 audioLogType[T_LOGS_COUNT],audioLogLevelFound[T_LOGS_COUNT]; size_t file_size,filelog_size; u8* file_data,*filelog_data; } TextSystem; // Hefty table for localization support.
TextSystem Sys_Text;
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
// Hw referenceIndex,ref14Index::Sys 21,0 Nav 22,1 Ere 23,2 Sen 24,3 Trg 25,4 Shi 26,5 Bio 27,6 Lan 28,7 Env 29,8 Boo 30,9 Jum 31,10 Nig 32,11
typedef struct {
    double nitroTimeSetting,earthShakerTimeSetting,justFired,waitTilNextFire,reloadFinished,lerpStartTime,dropFinished,playerHealthTimer,berserkFinishedTime,berserkIncrementFinishedTime,detoxFinishedTime,geniusFinishedTime,mediFinishedTime,reflexFinishedTime,sightFinishedTime,
           sightSideEffectFinishedTime,staminupFinishedTime,turboCyberTime,turboFinished,energyDrainTickFinished,painSoundFinished,radSoundFinished,radFXFinished;
    float weaponEnergySetting[16],reloadLerpValue,sparqSetting,ionSetting,blasterSetting,plasmaSetting,stungunSetting,energySliderClickedTime,cyberWeaponAttackFinished,targetY,currentEnergyWeaponHeat[7],fatigue,radiated,resetAfterDeathTime,energy,maxEnergy,radAdjust,initialRadiation;
    u32 accessCardOwned,wepAmmo[16],wepAmmoSecondary[16];
    i32 lastAddedIndex,emailCurrent,emailIndex,globalLookupIndex,weaponInventoryIndices[7],weaponInventoryAmmoIndices[7],hardwareInvCurrent,/*Current slot in the general inventory (14 slots).*/hardwareInvIndex,/*Current index to the item look-up table.*/generalInventoryIndexRef[14],berserkIncrement;
    i16 ladderState,weaponCurrentPending,weaponIndexPending;
    u16 numLogsFromLevel[10],hasHardware,hardwareIsActive,hardwareInvReferenceIndex[HW_COUNT],heldObjectIndex,heldObjectCustomIndex,heldObjectAmmo,heldObjectAmmo2,weaponIndex,currentSearchItem,generalInvIndex,generalInvCustomIndex[14],patchActive,drainJPM;
    u8 lerpUp,hasSoft,softVersions[7],hasMinigame,numweapons,currentMagazineAmount[7],currentMagazineAmount2[7],hardwareVersion[HW_COUNT],hardwareVersionSetting[HW_COUNT],grenAmmo[7],grenConstIndex[7],grenadeCurrent,generalInvCurrent,patchCurrent,patchCounts[7],cyberItemIndex;
        bool playerDead,beepDone,logPaused,hasNewEmail,hasNewNotes,currentCyberItem,isPulserNotDrill,wepLoadedWithAlternate[7],staminupActive,hasLog[134],readLog[134],justChangedWeap,overloadEnabled,recoiling,heldObjectLoadedAlternate,holdingObject,grenActive,hasNewLogs,hasNewData;
} InventorySystem;
typedef struct { float damage,penetration,offense,armorvalue,defense,impactVelocity; V3 attacknormal,hitpoint; AttType attackType; u16 owner,hitIdx; bool isOtherNPC,berserkActive; } DamageData;
typedef struct __attribute__((packed, aligned(8))) { u64 magicNumber; double thisRunTime; bool isLoading; i32 missionSplitID; } AutoSplitterData; // For use with LiveSplit or other future speedrunner utilities for doing speedruns
extern AutoSplitterData autoSplitter;
typedef struct {
        const char* name;
        AttType attackType,attackType2,attackType3;
        float damage,damage2,damage3,range,range2,range3,health,healthForCyberNPC;
        PerceptionLevel perception;
        float disruptability,armorvalue,defense;
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
        bool preactivateMeleeColliders; // Unused
        double huntTime;
        float flightHeight;
        bool flightHeightIsPercentage;
        bool switchMaterialOnDeath;
        float hearingRange;
        float timeForTranquilization;
        bool hopsOnMove;
        NPCType type;
        int projectile1Prefab,projectile2Prefab,projectile3Prefab;
} NPCTable;
extern NPCTable npcTable[NUM_AI_TYPES];
typedef struct { double clipFinished,combatImpulseFinished; bool inCombat,inZone,twoPlaying,distortion,cyberTube,elevator,levelEntry; } MusicSystem;
extern const char* sounds[SOUNDS_COUNT];
typedef /*FAT*/ struct  {
    u32 entflags,ioflags;
    u16 modelIndex,index; // constIndex for entity type, used for indexing into arrays for resourec types when loading resources
    V3 forward,right,lastPosition/*used for NPC logic, not physics*/,topPoint,targetPosition,startPosition,activatedScale,direction;
    float shadRadius;
    u16 texIndex,glowIndex,specIndex,normIndex,lodIndex,colMeshIndex;
    i32 cellIndex; i16 cellX,cellZ;
    u8 portalIndex,clip,numclips,texAnimClip,camView; // If this is a door, index into portal array for toggling state.
    bool cardchunk,kinematic;
    FuncStates/*u8*/ startState,funcState;
    BodyState/*u8*/ bodyState;
    float health,cyberHealth,targetPositionY,speed,percentAjar,percentMoved,volume,timeForTranquilization,gracePeriodFinished,meleeDamageFinished,idleTime,attack1SoundTime,attack2SoundTime,attack3SoundTime,timeTillEnemyChangeFinished,timeTillDeadFinished,timeTillPainFinished,huntFinished,
          randomWaitForNextAttack1Finished,randomWaitForNextAttack2Finished,randomWaitForNextAttack3Finished,attackFinished,attack2Finished,attack3Finished,deathBurstFinished,tranquilizeFinished,wanderFinished,timeSinceMovedEnough,posCheckFinished,currentFrameFinished,animSwapFinished,delay,damage,itemLifeTime,minutes,seconds,randomMin,randomMax,timeInterval,
          cyberTimer,intervalFinished,delayFireFinished,delayResetFinished,delayFinished,tickFinished,tickTime,useFinished,waitBeforeClose,lasersFinished,amount,resetTime,minSecurityLevel,ajarPercentage,useTimeDelay,timeBeforeLasersOn,force,strength,offStrengthFactor,distancePaddingToTopPoint,initialBurstFinished,justUsed,timerFinished,randomItemDropChance[4];
    V3 accumulatedForce;    
    u8 securityThreshold,lerpUp;
    u16 enemy,altTexIndex,altGlowIndex,messageIndex,teleportID,targetDestinationID,recentMostActivator,countToTrigger,counter,activateSFX,lockedSFX,messageLingdex,lockedMessageLingdex,animationNum,frame,texFrame,texGlowFrame,texAnimLight,texAnimLight2,lookUpIndex,contents[4],customIndex[4],useableItemIndex,usableCustomIndex,randomItem[4],randomItemCustomIndex[4];
    i16 version,SFXIndex,SFXLockedIndex,textIndex,emailIndex,ammo,ammo2;
    bool searchableInUse,generateContents,dontReset,onlyOnce,ignoreSecondaryTriggers,allDone,currentTexture,useRandomTimes,active,touchEnabled,broken,stayOpen,startOpen,ajar,blocked,targetAlreadyDone,toggleLasers,targettingOnlyUnlocks,changeLayerOnOpenClose,despawnInstead,doSelfAfterList,
         destroyAfterListInsteadOfDeactivate,iceActive,forceFieldDirectionX,forceFieldDirectionY,forceFieldDirectionZ,heldObjectLoadedAlternate,changeTexOnActive,blinkTexOnActive,alternateOn,lerping,onlyTargetOnce,autoPlayEmail,inCyberTube,noiseFinished,textureAnimating,textureGlowAnimating,
         textureAnimationStopsAtDead,texAnimInReverse,texAnimRandom;
    u8 maxRandomItems; // [0 4]
    AttType attackType;
    AccessCardType requiredAccessCard;
    BloodType bloodType;
    DoorState doorOpen;
    ForceFieldColor fieldColor;
    TrackType trackType;
    MusicType musicType;
    DoorState doorState;
    u16 mainSwitchMaterial,deathBurst;
    AIState currentState; // NPC logic
    V3 currentDestination,lastKnownEnemyPos,targettingPosition,idealTransformForward,idealPos;
    char targetname[TARGET_STRING_LENGTH],target[TARGET_STRING_LENGTH],target2[TARGET_STRING_LENGTH],currenttarget[TARGET_STRING_LENGTH],targetIfFalse[TARGET_STRING_LENGTH],texAnimResourceFolder[TARGET_STRING_LENGTH];
} Entity; // phew what a porker of a struct, it's been a eatin!
typedef struct { Entity* entries; u32 count; u32 capacity; } DataParser;
typedef __builtin_va_list va_list;
typedef struct { char soundPath[128]; float *samples; u32 frame_count,frame_pos; float volume; bool looping,positional,playing; V3 pos; size_t allocSize; } wav_channel_t;
typedef struct {
    u32 lastFrameSecCount,debugLineVertCount,shotsFired,grenadesThrown,savesScummed;
    u16 ressurections,deaths,kills,cyberkills,ressurectionActiveLevels,instCount; // Numbers of instances of entities and lights loaded (always for just the current level)
    float farPlane[14],damageDealt,damageReceived,timeScale,worldMin_x[14],worldMin_z[14],voxMinCtrX[14],voxMinCtrZ[14];
    double cpuTime,thisFrameTime,cpuFrameTime,lastFrameSecCountTime,debugLineFinished,shakeFinished,last_time,last_physics_time,deltaTime,current_time,screenshotTimeout,pauseRelativeTime,absoluteTime,statusTextDecayFinished,justSavedTimeStamp;
    i32 fogFac,cursorPosition_x,cursorPosition_y; // Separate internal cursor from system cursor.  This gets relatively pushed around by real cursor movement to give consistent platform behavior.
    V3 debugLine_start,debugLine_end,cyberspaceRecallPoint;
    u8 levelSecurity[14],startLevel,numLevels,curLev,diffCbt,diffPuz,diffMis,diffCyb,creditsPageIndex;
    int lev1SecCode,lev2SecCode,lev3SecCode,lev4SecCode,lev5SecCode,lev6SecCode;
    u8 currentLevel; // Which level's per-level arrays the pointers (instances, position, etc.) currently point to.  Usually equals curLev, but diverges briefly during cross-level target I/O.
    bool inventoryMode,levelCurrentlyLoading,introNotPlayed,paused,menuActive,gameFinished,creditsActive,decoyActive,boosterActive,uiIsBlocking,mouseClickHeldOverGUI,geniusActive;
    InventorySystem invP1,invP2;
    SystemUI Sys_UI;
    MusicSystem Sys_Music;

    // ===== Per-level parallel arrays (SoA) for cache-hot physics/render loops =====
    // The active level is selected by pointers below; SetLevelPointers(lev) swaps them.
    Entity levelInstances[MAX_LEVELS][INSTANCE_COUNT];
    V3 levelPosition[MAX_LEVELS][INSTANCE_COUNT];
    V3 levelScale[MAX_LEVELS][INSTANCE_COUNT];
    V3 levelVelocity[MAX_LEVELS][INSTANCE_COUNT];
    V3 levelAngularVelocity[MAX_LEVELS][INSTANCE_COUNT];
    V3 levelColliderCenter[MAX_LEVELS][INSTANCE_COUNT]; // Offset relative to .position's global worldspace xyz location
    V3 levelColliderSize[MAX_LEVELS][INSTANCE_COUNT]; // x,y,z for Box, x for Sphere radius, else x, y, z for Capsule radius, height, and direction (0.0f = X-Axis, 1.0f = Y-Axis, 2.0f = Z-Axis respectively, default 1.0f)
    ColliderType/*u8*/ levelCollider[MAX_LEVELS][INSTANCE_COUNT];
    Quaternion levelRotation[MAX_LEVELS][INSTANCE_COUNT];
    u32 levelLayer[MAX_LEVELS][INSTANCE_COUNT];
    float levelMass[MAX_LEVELS][INSTANCE_COUNT];
    float levelRadius[MAX_LEVELS][INSTANCE_COUNT];
    float levelGravity[MAX_LEVELS][INSTANCE_COUNT];
    float levelInertiaTensor[MAX_LEVELS][INSTANCE_COUNT][6];
    float levelInvInertiaTensor[MAX_LEVELS][INSTANCE_COUNT][6];
    float levelAngularDrag[MAX_LEVELS][INSTANCE_COUNT];
    float levelDynamicFriction[MAX_LEVELS][INSTANCE_COUNT];
    float levelStaticFriction[MAX_LEVELS][INSTANCE_COUNT];
    float levelBounciness[MAX_LEVELS][INSTANCE_COUNT];
    bool levelInvTnsrValid[MAX_LEVELS][INSTANCE_COUNT];
    bool levelColliding[MAX_LEVELS][INSTANCE_COUNT];
    u16 levelInstCount[MAX_LEVELS]; // Per-level instance count (parallel to instCount which always mirrors the active level's count).
    Light levelLights[14][LIGHT_COUNT];
    LightAnimation levelLAnims[14][LIGHT_COUNT];
    V3 levelLightsNewPosition[14][LIGHT_COUNT];
    u16 levelLoadedLights[14];

    // ===== Active-level pointers (the "current instances[]") =====
    // All existing call sites use World.instances[i], World.position[i], etc. unchanged;
    // these pointers simply redirect to the active level's row in the levelXxx[] arrays.
    Entity* instances;
    V3* position;
    V3* scale;
    V3* velocity;
    V3* angularVelocity;
    V3* colliderCenter;
    V3* colliderSize;
    ColliderType* collider;
    Quaternion* rotation;
    u32* layer;
    float* mass;
    float* radius;
    float* gravity;
    float (*inertiaTensor)[6];
    float (*invInertiaTensor)[6];
    float* angularDrag;
    float* dynamicFriction;
    float* staticFriction;
    float* bounciness;
    bool* invTnsrValid;
    bool* colliding;
    Light* lights;
    LightAnimation* lanims;
    V3* lightsNewPosition;
    u16 loadedLights; // mirrors levelLoadedLights[currentLevel]
    // instCount remains a scalar u16 (declared above) and mirrors levelInstCount[currentLevel].
    Color fogColor[14];
    // ===== Cross-level Target I/O cache =====
    // When UseTargets iterates across all 14 levels looking for matching targetname(s), it swaps the
    // active-level pointers to each level in turn.  The `activator` entity, however, lives in the
    // entry level (the level that was active when UseTargets was called) and its index is NOT valid
    // in other levels' instances arrays.  These fields cache the activator entity + ioflags at the
    // outermost UseTargets call so Targetted() and the functions it calls can still read the
    // activator's ioflags regardless of which level the pointers currently point to.
    // targetIOActive is true while inside the outermost UseTargets call (used to nest recursive
    // UseTargets calls correctly — e.g. DoorUse → UseTargets — without re-caching or prematurely
    // clearing the cache).  targetIOEntryLevel saves the level that was active on entry so the
    // outermost call can restore the pointers when it finishes.
    Entity targetIOActivatorEntity;
    u32 targetIOActivatorIoflags;
    u16 targetIOActivatorIdx;
    u8 targetIOEntryLevel;
    float cam_pitch,cam_yaw,cam_roll;
    i32 currentMouse_dx,currentMouse_dy;
    bool targetIOActive;
        char global_dllname[256],global_winicon[256],playerName[27],audiologNames[T_LOGS_COUNT][T_LOCALIZATION_MAX_LENGTH],audiologSubjects[T_LOGS_COUNT][T_LOCALIZATION_MAX_LENGTH],audiologSenders[T_LOGS_COUNT][T_LOCALIZATION_MAX_LENGTH],audioLogSpeech2Text[T_LOGS_COUNT][T_LOCALIZATION_MAX_LENGTH];
} GlobalContext; // Savable complete game state data
GlobalContext World = {0};
#define PI 3.14159265f
#define TAU 6.2831853f
#define vabs(x) ((x) < 0 ? -(x) : (x))
#define vmin(a,b) ((a) < (b) ? (a) : (b))
#define vmax(a,b) ((a) > (b) ? (a) : (b))
#define vclamp(x,a,b) vmin(vmax(x,a),b)
#define vsqrtf(x) __builtin_sqrtf(x)
INLINE float vinvsqtf(float v) { long i; float x2,y; const float threehalfs = 1.5F; x2 = v * 0.5F; y = v; i = * (long*)&y; i = 0x5f3759df - ( i >> 1 ); y = * (float*)&i; y = y * (threehalfs - (x2 * y * y)); return y; }
INLINE float vfloor(float x) { int i = (int)x; return (float)(i > x ? i - 1 : i); }
INLINE float vceil(float x) { int i = (int)x; return (float)(x > 0 && x > (float)i ? i + 1 : i); }
INLINE float vsinf(float x) { x -= TAU * vfloor(x / TAU); if (x > PI) { x -= TAU; } float s = (4/PI)*x - (4/(PI*PI))*x*vabs(x); return 0.225f*(s*vabs(s) - s) + s; }
INLINE float vcosf(float x) { return vsinf(x + 1.57079632f); }
INLINE float vacosf(float x) { float negate = (x < 0.0f) ? 1.0f : 0.0f; x = vabs(x); float ret = (-0.0187293f * x + 0.0742610f) * x - 0.2121144f; ret = (ret * x + 1.5707288f) * vsqrtf(1.0f - x); ret = ret - 2.0f * negate * ret; return negate * PI + (1.0f - 2.0f * negate) * ret; }
INLINE float vtan(float x) { return vsinf(x) / vcosf(x); }
INLINE float vcot(float x) { float x2 = x * x; float t = x + (x2 * x) * 0.33333333f; return 1.0f / t; }
INLINE float deg2rad(float degrees) { return degrees * (PI / 180.0f); }
INLINE float vexp2f(float x) { float ip = vfloor(x); float fp = x - ip; float p = 1.0f + fp * (0.69314718f + fp * (0.24022651f + fp * 0.05550411f)); /*poly approximation for 2^fp on [0,1]*/ int ei = (int)ip + 127; u32 bits = (u32)(ei << 23); union { u32 i; float f; } u = { bits }; return u.f * p; }
INLINE float vexp(float x) { return vexp2f(x * 1.4426950409f); } // 1/ln(2)
INLINE i32 clamp(i32 val, i32 min, i32 max) { return (val > max) ? max : ((val < min) ? min : val); }
INLINE float vround(float val) { return (val >= 0.0f) ? (float)(int)(val + 0.5f) : (float)(int)(val - 0.5f); }
INLINE V3 V3_AplusB(V3 a, V3 b) { return (V3){a.x + b.x, a.y + b.y, a.z + b.z}; }
INLINE V3 V3_AsubB(V3 a, V3 b) { return (V3){a.x - b.x, a.y - b.y, a.z - b.z}; }
INLINE V3 V3_ScaleByF(V3 v, float s) { return (V3){v.x * s, v.y * s, v.z * s}; }
INLINE V3 mul_v3_v3_elementwise(V3 v, V3 w) { return (V3){v.x * w.x, v.y * w.y, v.z * w.z}; }
INLINE float dot(float x1, float y1, float z1, float x2, float y2, float z2) { return x1*x2 + y1*y2 + z1*z2; }
INLINE float V3_dot(V3 a, V3 b) { return dot(a.x,a.y,a.z, b.x,b.y,b.z); }
INLINE float quat_dot(Quaternion a, Quaternion b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }
INLINE float V3_Mag(const V3 v) { return vsqrtf(V3_dot(v,v)); }
INLINE float V3_SqDist(V3 a, V3 b) { V3 d = V3_AsubB(a,b); return V3_dot(d,d); }
INLINE float V3_Dist(V3 a, V3 b) { return V3_Mag(V3_AsubB(a,b)); }
INLINE V3 V3_Cross(V3 a, V3 b) { return (V3){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }
INLINE V3 V3_Normalize(V3 v) { float len = V3_Mag(v); return len > 0.000001f ? (V3){v.x / len, v.y / len, v.z / len} : v; }
INLINE Quaternion quat_multiply(Quaternion q1, Quaternion q2) { return (Quaternion){(q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y),(q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x),(q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w),(q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z)}; } // Hamilton product, rotates q1 by q2
INLINE V3 quat_rot_v3(Quaternion q, V3 v) { Quaternion r = quat_multiply((quat_multiply(q, (Quaternion){v.x,v.y,v.z,0.0f})),(Quaternion){-q.x,-q.y,-q.z,q.w}); return (V3){r.x,r.y,r.z}; } // Returns rotated input vector rotated by a quaternion.
INLINE u8 hardware14fromConstdex(u16 c) { return clamp(c - 21,0,14); }
INLINE bool IdxIsPortalBlockingDoor(u16 entIdx) { return (entIdx >= 496 && entIdx <= 514 && entIdx != 502 && entIdx != 505 && entIdx != 506 && entIdx != 507); }// All doors except see-through doors.
INLINE bool IdxInBounds(int c) { return (c >= 0 && c <= 760); }
INLINE bool IdxIsGeometry(int c) { return (c >= 0 && c <= 306 && c != 112 && c != 279) || c == 760; }
INLINE bool IdxIsDoor(int c) { return (c >= 496 && c < 515); }
INLINE bool IdxIsLightStaticSaveable(int c) { return c == 748; }
INLINE bool IdxIsGenericTransform(int c) { return c == 749; }
INLINE bool IdxIsNPC(int c) { return (c >= 419 && c < 448); }
INLINE bool IdxIsCorpse(int c) { return (c >= 465 && c < 472); }
INLINE bool IdxIsHardware(int c) { return (c >= 328) && (c <= 339); }
INLINE bool IdxIsAmbient(int c) { return (c >= 621 && c <= 655); }
INLINE bool IdxIsButtonSwitch(int c) { return ((c >= 688 && c <= 692) || c == 694 || c == 695); }
INLINE bool IdxIsSearchable(int c) { return ((c >= 464 && c <= 476) || c == 530 || c == 531); }
INLINE bool IdxIsUsableObject(u16 c) { return ((c >= 307 && c <= 404) || c == 417); }
INLINE bool IdxIsAccessCard(u16 c) { return ((c >= 388 && c <= 398) || c == 417); }
INLINE bool IdxIsDynamicObject(u16 c) { return (c >= 307 && c <= 404) ||  c == 417 || (c >= 419 && c <= 428) || (c >= 430 && c <= 437) || (c >= 440 && c <= 442) || (c >= 458 && c <= 463) || (c >= 465 && c <= 476); }
INLINE bool IdxIsStaticObjectSaveable(int c) { return (c == 112 || c == 279 || (c >= 448 && c < 458) || c == 480 || c == 516 || (c >= 518 && c <= 526) || c == 530 || c == 531 || c == 546 || c == 555 || c == 594 || c == 596 || c == 598 || (c >= 600 && c < 603) || (c >= 604 && c < 616) || (c >= 688 && c < 693) || c == 694 || c == 695 || (c >= 699 && c < 704) || (c >= 741 && c < 746)); }
INLINE bool IdxIsStaticObjectImmutable(int c) { return ((c >= 527 && c < 530) || (c >= 532 && c < 546) || (c >= 547 && c < 553) || c == 554 || (c >= 556 && c < 594) || c == 595 || c == 597 || c == 599 || c == 601 || c == 603 || (c >= 616 && c < 688) || c == 693 || c == 696 || c == 697 || c == 698 || (c >= 704 && c < 717) || c == 720 || (c >= 733 && c < 736) || (c >= 737 && c < 739) || c == 746 || c == 747 || (c >= 750 && c <= 759 && c != 755)); }

void DualLogError(const char* fmt, ...); void DualLogWarn(const char* s, ...); void DualLog(const char* fmt, ...); bool cEmpty(const char c);

INLINE  int  mcmp(const void *s1, const void *s2, size_t n) { const u8 *p1 = (const u8 *)s1; const u8 *p2 = (const u8 *)s2; while (n--) { if (*p1 != *p2) {return *p1 - *p2;} p1++; p2++; } return 0; } // memcmp replacement
INLINE void* mmov(void *dst, const void *src, size_t n) { u8 *d = (u8*)dst; const u8* s = (const u8*)src; if (d < s) { while (n--) { *d++ = *s++; } } else if (d > s) { d += n; s += n; while (n--) { *--d = *--s; } } return dst; } // memmove replacement
INLINE void flag_setu16(u16 *flags, u16 bit, bool state) { *flags = (*flags & ~bit) | (-state & bit); }
INLINE void flag_set(u32 *flags, u32 bit, bool state) { *flags = (*flags & ~bit) | (-state & bit); }
INLINE u32 parse_numberu32(const char* str, const char* line, u32 lineNum) {
    if (str == 0 || *str == '\0') { DualLogError("Invalid blank string from line[%d]: %s\n", lineNum+1, line); return 0; }
    while (cEmpty((char)*str)) str++;
    while (cEmpty(*str)) str++;
    if (*str == '+') str++;
    if (*str == '-') { DualLogError("Invalid input, negative not allowed (%s)\n      from line[%d]: %s\n", str, lineNum+1, line); return 0; }
    unsigned long result = 0;
    while (*str >= '0' && *str <= '9') { i32 digit = *str - '0'; result = result * 10uL + (unsigned long)digit; str++; }
    return (u32)result;
}

INLINE u16 parse_numberu16(const char* str, const char* line, u32 lineNum) { u32 retval = parse_numberu32(str, line, lineNum); if (retval > U16_MAX) { DualLogError("Value %u out of range for u16 from line[%d]: %s\n", retval, lineNum+1, line); return 0; } return (u16)retval; }
INLINE u8 parse_numberu8(const char* str, const char* line, u32 lineNum) { u32 retval = parse_numberu32(str, line, lineNum); if (retval > U8_MAX) { DualLogError("Value %u out of range for u8 from line[%d]: %s\n", retval, lineNum+1, line); return 0; } return (u8)retval; }
INLINE bool parse_bool(const char* str, const char* line, u32 lineNum) { u32 parseval = parse_numberu32(str, line, lineNum); if (parseval > 1) {DualLogWarn("Loaded %u but expected boolean from line[%u]: %s\n",parseval, lineNum+1, line);} return parseval > 0 ? true : false; }
INLINE i32 parse_numberi32(const char* str, const char* line, u32 lineNum) {
    if (str == 0 || *str == '\0') { DualLogError("Invalid blank string from line[%d]: %s\n", lineNum+1, line); return 0; }
    while (cEmpty((char)*str)) str++;
    bool negative = false;
    if (*str == '+') str++;
    else if (*str == '-') { negative = true; str++; }
    long result = 0;
    while (*str >= '0' && *str <= '9') { result = result * 10L + (*str - '0'); str++; }
    return (i32)(negative ? -result : result);
}
INLINE i16 parse_numberi16(const char* str, const char* line, u32 lineNum) { i32 retval = parse_numberi32(str, line, lineNum); if (retval < -32768 || retval > 32767) { DualLogError("Value %d out of range for i16 from line[%d]: %s\n", retval, lineNum+1, line); return 0; } return (i16)retval; }
INLINE i8 parse_numberi8(const char* str, const char* line, u32 lineNum) { i32 retval = parse_numberi32(str, line, lineNum); if (retval < -128 || retval > 127) { DualLogError("Value %d out of range for i8 from line[%d]: %s\n", retval, lineNum+1, line); return 0; } return (i8)retval; }
INLINE float parse_float(const char* str, const char* line, u32 lineNum) {
    if (str == 0 || *str == '\0') { DualLogError("Invalid blank string from line[%d]: %s\n", lineNum+1, line); return 0.0f; }
    while (cEmpty(*str)) str++;
    bool negative = false;
    if (*str == '-') { negative = true; str++; }
    else if (*str == '+') { str++; }
    double value = 0.0;
    bool has_digit = false;
    while (*str >= '0' && *str <= '9') { value = value * 10.0 + (*str - '0'); str++; has_digit = true; } // Integer part
    if (*str == '.') { // Decimal part
        str++;
        double frac = 0.0;
        double place = 0.1;
        while (*str >= '0' && *str <= '9') { frac += (*str - '0') * place; place *= 0.1; str++; has_digit = true; }
        value += frac;
    }
    if (!has_digit) return 0.0f;
    if (negative) value = -value;
    return (float)value;
}

double get_time(); int sFormatV(char* buf, size_t bufsz, const char* f, va_list args);
char statusText[T_BUFFER_SIZE];
void CenterStatusPrint(const char * restrict fmt, ...) { va_list args; __builtin_va_start(args, fmt); sFormatV(statusText,T_BUFFER_SIZE,fmt,args); __builtin_va_end(args); DualLog("%s\n",statusText); World.statusTextDecayFinished = get_time() + 3.5;/*secs decay time before text dissappears.*/ }
void* mcpy(void *dst, const void *src, size_t n) { u8 *d=(u8 *)dst; const u8 *s=(const u8 *)src; while (n--) {*d++=*s++;} return dst; } // memcpy replacement
void* mset(void *dst, int c, size_t n) { u8 *p=(u8 *)dst; u8 v=(u8)c; while (n--) {*p++=v;} return dst; } // memset replacement
INLINE void SetLevelPointers(u8 lev) {
    if (lev >= MAX_LEVELS) return;
    World.currentLevel = lev;
    World.instances        = World.levelInstances[lev];
    World.position         = World.levelPosition[lev];
    World.scale            = World.levelScale[lev];
    World.velocity         = World.levelVelocity[lev];
    World.angularVelocity  = World.levelAngularVelocity[lev];
    World.colliderCenter   = World.levelColliderCenter[lev];
    World.colliderSize     = World.levelColliderSize[lev];
    World.collider         = World.levelCollider[lev];
    World.rotation         = World.levelRotation[lev];
    World.layer            = World.levelLayer[lev];
    World.mass             = World.levelMass[lev];
    World.radius           = World.levelRadius[lev];
    World.gravity          = World.levelGravity[lev];
    World.inertiaTensor    = World.levelInertiaTensor[lev];
    World.invInertiaTensor = World.levelInvInertiaTensor[lev];
    World.angularDrag      = World.levelAngularDrag[lev];
    World.dynamicFriction  = World.levelDynamicFriction[lev];
    World.staticFriction   = World.levelStaticFriction[lev];
    World.bounciness       = World.levelBounciness[lev];
    World.invTnsrValid     = World.levelInvTnsrValid[lev];
    World.colliding        = World.levelColliding[lev];
    World.instCount        = World.levelInstCount[lev];
    World.lights            = World.levelLights[lev];
    World.lanims            = World.levelLAnims[lev];
    World.lightsNewPosition = World.levelLightsNewPosition[lev];
    World.loadedLights      = World.levelLoadedLights[lev];
}
// CopyPlayerState: Copies the player-entity slots (PLAYER1=1, PLAYER2=2) — including the Entity
// struct and all parallel SoA arrays — from srcLevel to dstLevel.  Used by LoadLevel() to carry
// the player's state (position, health, ioflags, inventory-derived flags, etc.) across level
// switches so the player doesn't "reset" to the NewGame state when entering a new level.
// Note: slot 0 (WORLD/NULL) is per-level static geometry and is NOT copied.
INLINE void CopyPlayerState(u8 srcLevel, u8 dstLevel) {
    if (srcLevel >= MAX_LEVELS || dstLevel >= MAX_LEVELS || srcLevel == dstLevel) return;
    for (u16 s = PLAYER1; s <= PLAYER2; ++s) { // s = 1, 2
        World.levelInstances[dstLevel][s]            = World.levelInstances[srcLevel][s];
        World.levelPosition[dstLevel][s]             = World.levelPosition[srcLevel][s];
        World.levelScale[dstLevel][s]                = World.levelScale[srcLevel][s];
        World.levelVelocity[dstLevel][s]             = World.levelVelocity[srcLevel][s];
        World.levelAngularVelocity[dstLevel][s]      = World.levelAngularVelocity[srcLevel][s];
        World.levelColliderCenter[dstLevel][s]       = World.levelColliderCenter[srcLevel][s];
        World.levelColliderSize[dstLevel][s]         = World.levelColliderSize[srcLevel][s];
        World.levelCollider[dstLevel][s]             = World.levelCollider[srcLevel][s];
        World.levelRotation[dstLevel][s]             = World.levelRotation[srcLevel][s];
        World.levelLayer[dstLevel][s]                = World.levelLayer[srcLevel][s];
        World.levelMass[dstLevel][s]                 = World.levelMass[srcLevel][s];
        World.levelRadius[dstLevel][s]               = World.levelRadius[srcLevel][s];
        World.levelGravity[dstLevel][s]              = World.levelGravity[srcLevel][s];
        mcpy(World.levelInertiaTensor[dstLevel][s],    World.levelInertiaTensor[srcLevel][s],    6 * sizeof(float));
        mcpy(World.levelInvInertiaTensor[dstLevel][s], World.levelInvInertiaTensor[srcLevel][s], 6 * sizeof(float));
        World.levelAngularDrag[dstLevel][s]          = World.levelAngularDrag[srcLevel][s];
        World.levelDynamicFriction[dstLevel][s]      = World.levelDynamicFriction[srcLevel][s];
        World.levelStaticFriction[dstLevel][s]       = World.levelStaticFriction[srcLevel][s];
        World.levelBounciness[dstLevel][s]           = World.levelBounciness[srcLevel][s];
        World.levelInvTnsrValid[dstLevel][s]         = World.levelInvTnsrValid[srcLevel][s];
        World.levelColliding[dstLevel][s]            = World.levelColliding[srcLevel][s];
    }
}
INLINE void EntitySetLocked(Entity* e, bool locked) { DualLog("Unlocking entity with index %u\n",(u16)(e - World.instances)); flag_set(&e->entflags,EF_LOCKED,locked); }
INLINE void UIBlockedBySecurity(V3 tetherPoint) { (void)tetherPoint; CenterStatusPrint("%s",Sys_Text.stringTable[25]); }
INLINE void UICyberSprint(u16 textIndex) { CenterStatusPrint("%s",Sys_Text.stringTable[textIndex]); }
INLINE void UIExitCyberspace() { CenterStatusPrint("%s",Sys_Text.stringTable[601]); }
INLINE void HealthManagerHealingBed(u16 playerIdx, float amount, bool flashBed) { (void)flashBed; Entity* p = &World.instances[playerIdx]; p->health = vmin(255.0f,p->health + amount); }
INLINE void PlayerTakeDamage(u16 playerIdx, float damage) { Entity* p = &World.instances[playerIdx]; p->health -= damage; if (p->health < 0.0f) p->health = 0.0f; }
INLINE float SfxVol() { return (float)Sys_Settings.VolumeEffects / 100.0f; }
INLINE InventorySystem* Inv(u16 p) { return p == PLAYER1 ? &World.invP1 : &World.invP2; }
