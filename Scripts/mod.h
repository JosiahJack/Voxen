// mod.h - Header only used by gamecode files
#include "common.h" // Types needed first
#define MOD_INTEROP_MOD
#include "interop.h"
extern GlobalContext* Eng_Global; extern CheatsSystem* Eng_Cheats; extern SettingsSystem* Eng_Settings; extern TextSystem* Eng_Text; extern SystemUI* Eng_UI;
// For use with LiveSplit or other future speedrunner utilities for doing speedruns
typedef struct __attribute__((packed, aligned(8))) { u64 magicNumber; double thisRunTime; bool isLoading; i32 missionSplitID; } AutoSplitterData;
extern AutoSplitterData autoSplitter;
#ifndef NULL
    #define NULL 0
#endif
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
	bool preactivateMeleeColliders; // Unused
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
extern int lev1SecCode;
extern int lev2SecCode;
extern int lev3SecCode;
extern int lev4SecCode;
extern int lev5SecCode;
extern int lev6SecCode;
extern Entity EDefs[MAX_ENTITIES];
extern bool vmailActive;
extern const AnimationClip modelAnimationClips[MAX_ANIMATED_MODELS][MAX_ANIMATION_CLIPS_PER_MODEL];
extern const char* sounds[SOUNDS_COUNT];

typedef struct {
    u16 owner;
    const char* argvalue;
} TargetArgs;

// Mod Inlines
static inline __attribute__((always_inline)) void EntitySetLocked(Entity* e, bool locked) { DualLog("Unlocking entity with index %u\n",(u16)(e - Eng_Global->instances)); flag_set(&e->entflags,EF_LOCKED,locked); }
static inline __attribute__((always_inline)) void UIBlockedBySecurity(Vector3 tetherPoint) { (void)tetherPoint; CenterStatusPrint("%s",Eng_Text->stringTable[25]); }
static inline __attribute__((always_inline)) void UICyberSprint(u16 textIndex) { CenterStatusPrint("%s",Eng_Text->stringTable[textIndex]); }
static inline __attribute__((always_inline)) void UIExitCyberspace(void) { CenterStatusPrint("%s",Eng_Text->stringTable[601]); }
static inline __attribute__((always_inline)) void HealthManagerHealingBed(u16 playerIdx, float amount, bool flashBed) { (void)flashBed; Entity* p = &Eng_Global->instances[playerIdx]; p->health = vmin(255.0f,p->health + amount); }
static inline __attribute__((always_inline)) void PlayerTakeDamage(u16 playerIdx, float damage) { Entity* p = &Eng_Global->instances[playerIdx]; p->health -= damage; if (p->health < 0.0f) p->health = 0.0f; }
static inline __attribute__((always_inline)) Entity* PE(u16 p) { return &Eng_Global->instances[p]; }
static inline __attribute__((always_inline)) float SfxVol(void) { return (float)Eng_Settings->VolumeEffects / 100.0f; }
static inline __attribute__((always_inline)) InventorySystem* Inv(u16 p) { return p == PLAYER1 ? &Eng_Global->invP1 : &Eng_Global->invP2; }

// Mod Shared Functions across TU's
void WeaponsUpdate(void);
void UseTargets(u16 activator, const char* argvalue, const char* targetname);
void Targetted(u16 activator, u16 self, const char* argvalue);
void ButtonSwitchTargetted(u16 self, u16 activator, const char* argvalue);
void ButtonSwitchUse(u16 self, u16 activator, const char* argvalue);
void DoorUse(u16 self, u16 activator, const char* argvalue);
void DoorTargetted(u16 self, u16 activator, const char* argvalue);
void DoorActuate(u16 self);
void DoorForceOpen(u16 self);
void DoorForceClose(u16 self);
void TriggerTargetted(u16 self, u16 activator);
void TriggerCounterTargetted(u16 self, u16 activator, const char* argvalue);
void FuncWallTargetted(u16 self, u16 activator, const char* argvalue);
void ButtonSwitchInitAfterLoad(u16 self);
void DoorInitAfterLoad(u16 self);
void FuncWallInitAfterLoad(u16 self);
void ForceBridgeInitBeforeLoad(u16 self);
void ForceBridgeInitAfterLoad(u16 self);
void GravityLiftInitAfterLoad(u16 self);
void LogicTimerInitBeforeLoad(u16 self);
void TeleportTouchInitAfterLoad(u16 self);
void TextureChangerInitAfterLoad(u16 self);
void CyberItemInitBeforeLoad(u16 self);
void CyberMineInitBeforeLoad(u16 self);
void CyberTimerInitAfterLoad(u16 self);
void CyberSwitchInitAfterLoad(u16 self);
void ExplosionLifeInitAfterLoad(u16 self);
void ExplosionLifeUpdate(u16 self);
void EmailTargetted(u16 self, u16 activator, const char* argvalue);
void ForceBridgeActivate(u16 self, bool isSilent);
void ForceBridgeToggle(u16 self);
void GravityLiftToggle(u16 self);
void TextureChangerToggle(u16 self);
void ButtonSwitchUpdate(u16 self);
void DoorUpdate(u16 self);
void ForceBridgeUpdate(u16 self);
void FuncWallUpdate(u16 self);
void LogicTimerUpdate(u16 self);
void DelayedSpawnUpdate(u16 self);
void SearchFXResetUpdate(u16 self);
void CyberTimerUpdate(u16 self);
void GeneralInvApply(int buttonIdx,int customIdx);
bool InventoryAddSoftwareItem(u16 p,SoftwareType type,int vers);
int Get16WeaponIndexFromConstIndex(int index);
void UseGrenade(u16 playerIndex, int index);
bool AICheckPain(Entity* self);
void ResetHeldItem(u16 p);
void DropHeldItem(u16 p);
void AddItemToInventory(u16 p, int index, int customIndex);
void TextureSequenceUpdate(u16 self);
u16 AddInstance(u16 entIdx, Vector3 pos);
void DeleteInstance(u16 i);
u8 GetCurrentLevelSecurity(void);
u16 GetImpactType(u16 instanceIdx);
const char* GetPrefabNameFromIndex(int constIndex);
void TakeEnergy(float drain);
void GiveEnergy(float give, EnergyType type);
void BioMonitorInit(void);
void BioMonitorUpdate(u16 p);
void* MemCpyFromBtoAForNBytes(void *dst, const void *src, size_t n); // memcpy replacement
