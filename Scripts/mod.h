// mod.h - Header only used by gamecode files
#include "common.h" // Types needed first
#define MOD_INTEROP_IMPLEMENTATION
#include "interop.h"
extern GlobalContext* Eng_Global; extern CheatsSystem* Eng_Cheats; extern SettingsSystem* Eng_Settings; extern TextSystem* Eng_Text; extern SystemUI* Eng_UI;
// For use with LiveSplit or other future speedrunner utilities for doing speedruns
typedef struct __attribute__((packed, aligned(8))) { uint64_t magicNumber; double thisRunTime; bool isLoading; int32_t missionSplitID; } AutoSplitterData;
extern AutoSplitterData autoSplitter;

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

uint8_t GetCurrentLevelSecurity(void);
uint16_t GetImpactType(uint16_t instanceIdx);
const char* GetPrefabNameFromIndex(int constIndex);
void TakeEnergy(float drain);
void GiveEnergy(float give, EnergyType type);
void BioMonitorInit(void);
void BioMonitorUpdate(uint16_t p);

extern bool vmailActive;
extern const AnimationClip modelAnimationClips[MAX_ANIMATED_MODELS][MAX_ANIMATION_CLIPS_PER_MODEL];
extern const char* sounds[SOUNDS_COUNT];

typedef struct {
    uint16_t owner;
    const char* argvalue;
} TargetArgs;

// Mod Inlines
static inline __attribute__((always_inline)) bool EntityLocked(const Entity* e) { return (e->entflags & ENTFLAG_LOCKED) != 0; }
static inline __attribute__((always_inline)) void EntitySetLocked(Entity* e, bool locked) { flag_set(&e->entflags,ENTFLAG_LOCKED,locked); }
static inline __attribute__((always_inline)) void UIBlockedBySecurity(Vector3 tetherPoint) { (void)tetherPoint; CenterStatusPrint("%s",Eng_Text->stringTable[25]); }
static inline __attribute__((always_inline)) void UICyberSprint(uint16_t textIndex) { CenterStatusPrint("%s",Eng_Text->stringTable[textIndex]); }
static inline __attribute__((always_inline)) void UIExitCyberspace(void) { CenterStatusPrint("%s",Eng_Text->stringTable[601]); }
static inline __attribute__((always_inline)) void HealthManagerHealingBed(uint16_t playerIdx, float amount, bool flashBed) { (void)flashBed; Entity* p = &Eng_Global->instances[playerIdx]; p->health = vmin(255.0f,p->health + amount); }
static inline __attribute__((always_inline)) void PlayerTakeDamage(uint16_t playerIdx, float damage) { Entity* p = &Eng_Global->instances[playerIdx]; p->health -= damage; if (p->health < 0.0f) p->health = 0.0f; }
static inline __attribute__((always_inline)) Entity* PE(uint16_t p) { return &Eng_Global->instances[p]; }
static inline __attribute__((always_inline)) float SfxVol(void) { return (float)Eng_Settings->VolumeEffects / 100.0f; }
static inline __attribute__((always_inline)) InventorySystem* Inv(uint16_t p) { return p == PLAYER1 ? &Eng_Global->inventoryPlayer1 : &Eng_Global->inventoryPlayer2; }

// Mod Shared Functions across TU's
void WeaponsUpdate(void);
void UseTargets(uint16_t activator, const char* argvalue, const char* targetname);
void Targetted(uint16_t activator, uint16_t self, const char* argvalue);
void ButtonSwitchTargetted(uint16_t self, uint16_t activator, const char* argvalue);
void ButtonSwitchUse(uint16_t self, uint16_t activator, const char* argvalue);
void DoorUse(uint16_t self, uint16_t activator, const char* argvalue);
void DoorTargetted(uint16_t self, uint16_t activator, const char* argvalue);
void DoorActuate(uint16_t self);
void DoorForceOpen(uint16_t self);
void DoorForceClose(uint16_t self);
void DoorLock(uint16_t self);
void DoorUnlock(uint16_t self);
void DoorToggleLocked(uint16_t self);
void DoorToggleAccessCardOverride(uint16_t self);
void TriggerTargetted(uint16_t self, uint16_t activator);
void TriggerCounterTargetted(uint16_t self, uint16_t activator, const char* argvalue);
void FuncWallTargetted(uint16_t self, uint16_t activator, const char* argvalue);
void ButtonSwitchInitAfterLoad(uint16_t self);
void DoorInitAfterLoad(uint16_t self);
void FuncWallInitAfterLoad(uint16_t self);
void ButtonSwitchToggleLocked(uint16_t self);
void ForceBridgeInitBeforeLoad(uint16_t self);
void ForceBridgeInitAfterLoad(uint16_t self);
void GravityLiftInitAfterLoad(uint16_t self);
void LogicTimerInitBeforeLoad(uint16_t self);
void TeleportTouchInitAfterLoad(uint16_t self);
void TextureChangerInitAfterLoad(uint16_t self);
void CyberItemInitBeforeLoad(uint16_t self);
void CyberMineInitBeforeLoad(uint16_t self);
void CyberTimerInitAfterLoad(uint16_t self);
void CyberSwitchInitAfterLoad(uint16_t self);
void ExplosionLifeInitAfterLoad(uint16_t self);
void ExplosionLifeUpdate(uint16_t self);
void EmailTargetted(uint16_t self, uint16_t activator, const char* argvalue);
void ForceBridgeActivate(uint16_t self, bool isSilent);
void ForceBridgeToggle(uint16_t self);
void GravityLiftToggle(uint16_t self);
void TextureChangerToggle(uint16_t self);
void ButtonSwitchUpdate(uint16_t self);
void DoorUpdate(uint16_t self);
void ForceBridgeUpdate(uint16_t self);
void FuncWallUpdate(uint16_t self);
void LogicTimerUpdate(uint16_t self);
void DelayedSpawnUpdate(uint16_t self);
void SearchFXResetUpdate(uint16_t self);
void CyberTimerUpdate(uint16_t self);
void GeneralInvApply(int buttonIdx,int customIdx);
bool InventoryAddSoftwareItem(uint16_t p,SoftwareType type,int vers);
int Get16WeaponIndexFromConstIndex(int index);
void UseGrenade(uint16_t playerIndex, int index);
bool AICheckPain(Entity* self);
void ResetHeldItem(uint16_t p);
void DropHeldItem(uint16_t p);
void AddItemToInventory(uint16_t p, int index, int customIndex);
void TextureSequenceUpdate(uint16_t self);
