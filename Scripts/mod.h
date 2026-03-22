// mod.h - Header only used by gamecode files
#include "common.h" // Types needed first
#define MOD_INTEROP_IMPLEMENTATION
#include "interop.h"
extern GlobalContext* Eng_Global; extern CheatsSystem* Eng_Cheats; extern SettingsSystem* Eng_Settings; extern TextSystem* Eng_Text; extern SystemUI* Eng_UI;
#define MULTI_MEDIA_TAB_EMAIL_TABLE 0
#define MULTI_MEDIA_TAB_LOG_TABLE   1
#define MULTI_MEDIA_TAB_DATA_TABLE  2
#define MULTI_MEDIA_TAB_NOTES       3
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

extern uint16_t useableItemsFrobIcons[94];

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

uint8_t GetCurrentLevelSecurity(void);
uint16_t GetImpactType(uint16_t instanceIdx);
const char* GetPrefabNameFromIndex(int constIndex);
void TakeEnergy(float drain);
void GiveEnergy(float give, EnergyType type);
void BioMonitorInit(void);
void BioMonitorUpdate(void);
extern bool vmailActive;
extern const AnimationClip modelAnimationClips[MAX_ANIMATED_MODELS][MAX_ANIMATION_CLIPS_PER_MODEL];
extern const char* sounds[SOUNDS_COUNT];

typedef struct {
    uint16_t owner;
    const char* argvalue;
} TargetArgs;

static inline bool EntityLocked(const Entity* e) { return (e->entflags & ENTFLAG_LOCKED) != 0; }
static inline void EntitySetLocked(Entity* e, bool locked) { flag_set(&e->entflags,ENTFLAG_LOCKED,locked); }
static inline const char* EntityDefName(uint16_t self) { return Eng_Global->entities[Eng_Global->instances[self].index].path; }
static inline bool EntityDefIs(uint16_t self, const char* name) { return StringsAreEqual(EntityDefName(self),name); }
static inline void UIBlockedBySecurity(Vector3 tetherPoint) { (void)tetherPoint; CenterStatusPrint("%s",Eng_Text->stringTable[25]); }
static inline void UICyberSprint(uint16_t textIndex) { CenterStatusPrint("%s",Eng_Text->stringTable[textIndex]); }
static inline void UIExitCyberspace(void) { CenterStatusPrint("%s",Eng_Text->stringTable[601]); }
static inline void HealthManagerHealingBed(uint16_t playerIdx, float amount, bool flashBed) { (void)flashBed; Entity* p = &Eng_Global->instances[playerIdx]; p->health = vmin(255.0f,p->health + amount); }
static inline void PlayerTakeDamage(uint16_t playerIdx, float damage) { Entity* p = &Eng_Global->instances[playerIdx]; p->health -= damage; if (p->health < 0.0f) p->health = 0.0f; }

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
void ForceBridgeDeactivate(uint16_t self, bool isSilent);
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
bool InventoryBioMonitorActive(uint16_t p);
bool InventoryBoosterSetToBoost(uint16_t p);
void InventoryJumpJetsToggle(uint16_t p);
bool InventoryJumpJetsActive(uint16_t p);
