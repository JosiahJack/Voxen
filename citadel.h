// Types and Defines for Citadel: The System Shock Fan Remake
#include <stdbool.h>
#include "voxen.h"
#include "citadel_enumerations.h"

#define PLAYER_RADIUS 0.48f
#define PLAYER_HEIGHT 2.00f
#define PLAYER_CAM_OFFSET_Y 0.84f // Split capsule shape in the middle, camera is thus 0.16 away from top of the capsule ((2 / 2 = 1) - 0.84)

#define LEVEL_CYBERSPACE 13

typedef struct {
    int SFXJump;
    int SFXJumpLand;
    int SFXLadder;
    float playerSpeed;
    float playerSpeedActual;
    float playerSpeedHorizontalActual;
    bool isSprinting;
    bool grounded;
    float feetRayLength;
    bool FatigueCheat;
    BodyState bodyState;
    int ladderState;
    bool gravliftState;
    float walkAcceleration;
    int SFXIndex;
    float walkDeacceleration;
	float walkDeaccelerationBooster;
	float deceleration;
	float walkAccelAirRatio;
	float maxWalkSpeed;
	float maxCyberSpeed;
	float maxCyberUltimateSpeed;
	float maxCrouchSpeed;
	float maxProneSpeed;
	float maxSprintSpeed;
	float maxSprintSpeedFatigued;
	float maxVerticalSpeed;
	float boosterSpeedBoost; // ammount to boost by when booster is active
	float jumpImpulseTime;
	float jumpVelocityBoots;
	float jumpVelocity;
    float jumpVelocityFatigued;
	float crouchRatio;
	float proneRatio;
	float transitionToCrouchSec;
	float transitionToProneAdd;
	float currentCrouchRatio;
	float capsuleHeight;
	float capsuleRadius;
	float ladderSpeed;
	float fallDamage;
    bool CheatWallSticky;
    bool CheatNoclip;
    bool staminupActive;
    Vector2 horizontalMovement;
    float verticalMovement;
    float jumpTime;
    float crouchingVelocity;
    float lastCrouchRatio;
    int layerGeometry;
	int layerMask;
	float fallDamageSpeed;
	Vector3 oldVelocity;
	float fatigue;
	float jumpFatigue;
	float fatigueWanePerTick;
	float fatigueWanePerTickCrouched;
	float fatigueWanePerTickProne;
	float fatigueWaneTickSecs;
	float fatiguePerWalkTick;
	float fatiguePerSprintTick;
	bool justJumped;
	float fatigueFinished;
	float fatigueFinished2;
	bool running;
	float relForward;
	float relSideways;
	bool cyberSetup;
	bool cyberDesetup;
	float bonus;
    float walkDeaccelerationVolx;
    float walkDeaccelerationVoly;
    float walkDeaccelerationVolz;
	bool consoleActivated;
	float leanTarget;
	float leanShift;
	float leanMaxAngle;
	float leanMaxShift;
	float jumpSFXFinished;
	float ladderSFXFinished;
	float ladderSFXIntervalTime;
	float jumpSFXIntervalTime;
	float jumpLandSoundFinished;
	float jumpJetEnergySuckTickFinished;
	float jumpJetEnergySuckTick;
	float leanSpeed;
	bool Notarget; // for cheat to disable enemy sight checks against this player
    bool fatigueWarned;
    float ressurectingFinished;
	float burstForce;
	float doubleJumpFinished;
	Vector3 playerHome;
	float turboFinished;
	float turboCyberTime;
	bool inCyberTube;
	float stepFinished;
	float rustleFinished;
	int doubleJumpTicks;
	Vector3 tempVecRbody;
	bool inputtingMovement;
	float accel;
	float floorDot;
	Vector3 floorAng;
	float slideAngle;
	float gravFinished;
	float bodyLerpGravityOffDelayFinished;
	Vector3 feetOffset;
} PlayerMovement;

extern PlayerMovement playerMovement;
