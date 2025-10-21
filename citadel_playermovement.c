// PlayerMovement
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "voxen.h"
#include "citadel.h"

PlayerMovement playerMovement;

void Awake() {
    playerMovement = (PlayerMovement){
        .SFXJump = 135,
        .SFXJumpLand = 136,
        .SFXLadder = 137,
        .playerSpeed = 0.0f,
        .playerSpeedActual = 0.0f,
        .playerSpeedHorizontalActual = 0.0f,
        .grounded = false,
        .feetRayLength = 5.0f,
        .FatigueCheat = false,
        .bodyState = BodyState_Standing,
        .walkDeacceleration = 0.1f,
        .walkDeaccelerationBooster = 0.5f,
        .deceleration = 0.0f,
        .walkAccelAirRatio = 0.75f,
        .maxWalkSpeed = 3.2f,
        .maxCyberSpeed = 5.0f,
        .maxCyberUltimateSpeed = 12.0f,
        .maxCrouchSpeed = 1.25f,
        .maxProneSpeed = 0.5f,
        .maxSprintSpeed = 8.8f,
        .maxSprintSpeedFatigued = 5.5f,
        .maxVerticalSpeed = 10.0f,
        .boosterSpeedBoost = 1.2f,
        .jumpImpulseTime = 4.0f,
        .jumpVelocityBoots = 0.45f,
        .jumpVelocity = 1.1f,
        .jumpVelocityFatigued = 0.6f,
        .crouchRatio = 0.6f,
        .proneRatio = 0.2f,
        .transitionToCrouchSec = 0.2f,
        .transitionToProneAdd = 0.1f,
        .currentCrouchRatio = 1.0f,
        .capsuleHeight = 2.0f,
        .capsuleRadius = 0.48f,
        .ladderSpeed = 0.4f,
        .fallDamage = 75.0f,
        .CheatWallSticky = false,
        .CheatNoclip = false,
        .staminupActive = false,
        .horizontalMovement = (Vector2){ .x = 0.0f, .y = 0.0f },
        .verticalMovement = 0.0f,
        .jumpTime = 0.0f,
        .crouchingVelocity = 1.0f,
        .lastCrouchRatio = 1.0f,
        .layerGeometry = 9,
        .fallDamageSpeed = 11.72f,
        .oldVelocity = (Vector3){ .x = 0.0f, .y = 0.0f, .z = 0.0f },
        .fatigue = 0.0f,
        .jumpFatigue = 6.5f,
        .fatigueWanePerTick = 1.0f,
        .fatigueWanePerTickCrouched = 2.0f,
        .fatigueWanePerTickProne = 3.5f,
        .fatigueWaneTickSecs = 0.3f,
        .fatiguePerWalkTick = 0.88f,
        .fatiguePerSprintTick = 2.85f,
        .justJumped = false,
        .fatigueFinished = pauseRelativeTime,
        .fatigueFinished2 = pauseRelativeTime,
        .running = false,
        .relForward = 0.0f,
        .relSideways = 0.0f,
        .cyberSetup = false,
        .cyberDesetup = false,
        .bonus = 0.0f,
        .walkDeaccelerationVolx = 0.0f,
        .walkDeaccelerationVoly = 0.0f,
        .walkDeaccelerationVolz = 0.0f,
        .leanTarget = 0.0f,
        .leanShift = 0.0f,
        .leanMaxAngle = 35.0f,
        .leanMaxShift = 0.8f,
        .jumpSFXFinished = pauseRelativeTime,
        .ladderSFXFinished = pauseRelativeTime,
        .ladderSFXIntervalTime = 1.0f,
        .jumpSFXIntervalTime = 1.0f,
        .jumpLandSoundFinished = pauseRelativeTime,
        .jumpJetEnergySuckTickFinished = pauseRelativeTime,
        .jumpJetEnergySuckTick = 1.0f,
        .leanSpeed = 70.0f,
        .Notarget = false,
        .fatigueWarned = false,
        .ressurectingFinished = pauseRelativeTime,
        .burstForce = 35.0f,
        .doubleJumpFinished = pauseRelativeTime,
        .playerHome = (Vector3){ .x = -20.4f, .y = -43.792f, .z = 10.2f},
        .turboFinished = pauseRelativeTime,
        .turboCyberTime = 15.0f,
        .inCyberTube = false,
        .stepFinished = pauseRelativeTime,
        .rustleFinished = pauseRelativeTime,
        .doubleJumpTicks = 0,
        .inputtingMovement = false,
        .accel = 0.0f,
        .slideAngle = 0.9f,
        .gravFinished = pauseRelativeTime,
        .bodyLerpGravityOffDelayFinished = pauseRelativeTime,
        .feetOffset = (Vector3){ .x = 0.0f, .y = -48.0f, .z = 0.0f },
    };
}

void CyberSetup() {
    if (currentLevel == LEVEL_CYBERSPACE && !playerMovement.cyberSetup) {
        playerMovement.cyberSetup = true;
        playerMovement.cyberDesetup = true;
    }
}

void CyberDestupOrNoclipMaintain() {
    if (currentLevel == LEVEL_CYBERSPACE) return;
    
    if (playerMovement.cyberDesetup || playerMovement.CheatNoclip) {
        playerMovement.cyberDesetup = false;
        playerMovement.cyberSetup = false;
        if (currentLevel == LEVEL_CYBERSPACE) DualLogError("Attempted to reset mouselook angles when exiting cyberspace but currentLevel not updated yet!\n");
        Input_MouselookApply();
    }
}

void Update() {
    if (gamePaused || (playerMovement.ressurectingFinished >= pauseRelativeTime)) return;

    CyberSetup();
    CyberDestupOrNoclipMaintain();
//     Crouch();
//     Prone();
//     EndCrouchProneTransition();
//     FatigueApply(); // Here fatigue me out, except in cyberspace
//     Automap.a.UpdateAutomap(transform.localPosition); // Update the map.
}
