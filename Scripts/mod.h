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

extern const char* sounds[SOUNDS_COUNT];
