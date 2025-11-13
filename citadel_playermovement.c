// citadel_playermovement.c - PlayerMovement
#include "entity.h"
#include "voxen.h"

typedef struct {
    float feetRayLength;
    bool FatigueCheat;
    uint8_t bodyState;
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
PlayerMovement playerMovement;
/*
void Awake() {
    playerMovement = (PlayerMovement){
        .SFXJump = 135,
        .SFXJumpLand = 136,
        .SFXLadder = 137,
        .playerSpeed = 0.0f,
        .grounded = false,
        .useGravity = true,
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
}*/
/*
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

// Get input for Jump and set impulse time, removed
// "&& (ladderState == 0)" since I want to be able to jump off a ladder
void Jump() {
    if (CheatNoclip && !Inventory.a.JumpJetsActive()) return;

    if (doubleJumpFinished < PauseScript.a.relativeTime) {
        doubleJumpTicks--;
        if (doubleJumpTicks < 0) doubleJumpTicks = 0;
    }

    if ((!gravliftState && GetInput.a.Jump())
        || gravliftState && GetInput.a.JumpDown()) {

        if (!justJumped) {
            if (grounded || gravliftState || Inventory.a.JumpJetsActive()) {
                jumpTime = jumpImpulseTime;
                doubleJumpFinished = PauseScript.a.relativeTime + Const.doubleClickTime;
                doubleJumpTicks++;
                justJumped = true;
                if (!Inventory.a.JumpJetsActive() && !Inventory.a.BoosterActive()) {
                    fatigue += jumpFatigue;
                }
            } else {
                if (ladderState > 1) {
                    jumpTime = jumpImpulseTime;
                    justJumped = true;
                    if (!Inventory.a.JumpJetsActive() && !Inventory.a.BoosterActive()) {
                        fatigue += jumpFatigue;
                    }
                }
            }
        }

        if (Inventory.a.BoosterActive() && Inventory.a.BoosterSetToBoost()) {
            if (justJumped && doubleJumpTicks == 2) {
                // Booster thrust
                rbody.AddForce(new Vector3(transform.forward.x * burstForce,
                                            transform.forward.y * burstForce,
                                            transform.forward.z * burstForce),
                                            ForceMode.Impulse);
                
                PlayerHealth.a.makingNoise = true;
                PlayerHealth.a.noiseFinished = PauseScript.a.relativeTime + 0.5f;
                PlayerEnergy.a.TakeEnergy(22f);
                if (BiomonitorGraphSystem.a != null) {
                    BiomonitorGraphSystem.a.EnergyPulse(22f);
                }

                justJumped = false;
                jumpTime = 0;
                doubleJumpTicks = 0;

                // Make sure we can't do it again right away.
                doubleJumpFinished = PauseScript.a.relativeTime - 1f;
            }
        }
    }

    if (staminupActive || FatigueCheat) fatigue = 0;
    
    // Perform Jump
    float jumpVelocityApply = jumpVelocity * rbody.mass;
    Vector3 jumpVel = new Vector3 (0,jumpVelocityApply,0);
    float jumpTimeMod = jumpTime;
    if (isSprinting) jumpTimeMod *= 0.5f;
    while (jumpTimeMod > 0) { // Why is this a `while` instead of an `if`??
                            // Because otherwise it don't work, duh!
        jumpTimeMod -= Time.smoothDeltaTime;
        if (fatigue > 80 && !Inventory.a.JumpJetsActive()) {
            jumpVelocityApply = jumpVelocityFatigued * rbody.mass;
            jumpVel.y = jumpVelocityApply;
        }

        if (Inventory.a.JumpJetsActive()) {
            float energysuck = 25f;
            jumpVelocityApply = jumpVelocityBoots * rbody.mass;
            jumpVel.y = jumpVelocityApply;
            switch (Inventory.a.hardwareVersionSetting[10]) {
                case 0: energysuck = 11f; break;
                case 1: energysuck = 26f; break;
                case 2: energysuck = 22f; break;
            }

            if (PlayerEnergy.a.energy >= energysuck) {
                rbody.AddForce(jumpVel,ForceMode.Force);  // huhnh!
                if (jumpJetEnergySuckTickFinished < PauseScript.a.relativeTime) {
                    jumpJetEnergySuckTickFinished = PauseScript.a.relativeTime + jumpJetEnergySuckTick;
                    PlayerEnergy.a.TakeEnergy(energysuck);
                    if (BiomonitorGraphSystem.a != null) {
                        BiomonitorGraphSystem.a.EnergyPulse(energysuck);
                    }
                }
            } else {
                hwbJumpJets.JumpJetsOff();
            }
        } else {
            if (ladderState > 1) {
                // Jump off ladder in direction of player facing.
                jumpVel = transform.forward * jumpVelocityApply * rbody.mass;
            }

            rbody.AddForce(jumpVel,ForceMode.Force);  // huhnh!
        }
    }

    if (justJumped && !Inventory.a.JumpJetsActive()) {
        // Play jump sound
        if (jumpSFXFinished < PauseScript.a.relativeTime) {
            jumpSFXFinished = PauseScript.a.relativeTime + jumpSFXIntervalTime;
            SFX.pitch = 1f;
            float jumpSFXVolume = 1.0f;
            if (fatigue > 80) jumpSFXVolume = 0.5f; // Quietly, we tired.
            
            PlayerHealth.a.makingNoise = true;
            PlayerHealth.a.noiseFinished = PauseScript.a.relativeTime + 0.5f;
            Physics.Raycast(transform.position, Vector3.down,
                            out tempHit,feetRayLength,
                            Const.a.layerMaskPlayerFeet);
            
            if (tempHit.collider == null) { Utils.PlayOneShotSavable(SFX,SFXJump,jumpSFXVolume); return; }
            GameObject hitGO = tempHit.collider.transform.gameObject;
            PrefabIdentifier prefID = hitGO.GetComponent<PrefabIdentifier>();
            if (prefID == null) {
                if (hitGO.transform.parent != null) {
                    prefID = hitGO.transform.parent.gameObject.GetComponent<PrefabIdentifier>();
                }
            }
            if (prefID == null) { Utils.PlayOneShotSavable(SFX,SFXJump,jumpSFXVolume); return; }
            
            FootStepType fstep = GetFootstepTypeForPrefab(prefID.constIndex);
            AudioClip stcp = JumpSound(fstep);
            Utils.PlayTempAudio(transform.position - feetOffset,stcp,jumpSFXVolume);
        }
        justJumped = false;
    }
    
    if (jumpTimeMod <= 0) justJumped = false; // for jump jets to work 
    jumpTime = jumpTimeMod;
}

void LadderStates() {
    if (CheatNoclip) return;
    if (ladderState < 1) return;

    float sidForce = 0f;
    float forForce = 0f;
    float upForce = 0f;
    if (grounded || Inventory.a.JumpJetsActive()) {
        // Ladder climb, allow while grounded
        float bonus = 1f;
        if (Inventory.a.JumpJetsActive()) bonus = 2f;

        sidForce = relSideways * walkAcceleration * Time.deltaTime;
        forForce = relForward * walkAcceleration * Time.deltaTime;
        upForce = ladderSpeed * relForward * walkAcceleration
                        * Time.deltaTime * bonus;

        // Climbing when touching the ground
        rbody.AddRelativeForce(sidForce,upForce,forForce);
    } else {
        // Climbing off the ground
        if (ladderSFXFinished < PauseScript.a.relativeTime
            && rbody.velocity.y > ladderSpeed * 0.5f) {

            SFX.pitch = (UnityEngine.Random.Range(0.8f,1.2f));
            Utils.PlayOneShotSavable(SFX,SFXLadder,0.2f);
            ladderSFXFinished = PauseScript.a.relativeTime
                                + ladderSFXIntervalTime;
        }

        float ladderSpeedMod = ladderSpeed;
        if (isSprinting && running) ladderSpeedMod = 1.2f; // Climb fast!

        sidForce = relSideways * walkAcceleration * Time.deltaTime * walkAccelAirRatio * 0.3f;
        forForce = relForward * walkAcceleration * Time.deltaTime * walkAccelAirRatio * 0.2f;
        upForce = ladderSpeedMod * relForward * walkAcceleration
                        * Time.deltaTime;

        rbody.AddRelativeForce(sidForce,upForce,forForce);
    }

    if (Inventory.a.BoosterActive() && Inventory.a.BoosterSetToSkates()) {
        deceleration = walkDeaccelerationBooster;
    } else {
        deceleration = walkDeacceleration;
    }

    // Set vertical velocity towards 0 when climbing.
    RigidbodySetVelocityY(rbody,(Mathf.SmoothDamp(rbody.velocity.y,0,
                                                    ref walkDeaccelerationVoly,
                                                    deceleration)));
}

private float runTime;

void WalkRun() {
    if (CheatNoclip) return;
    if (ladderState > 0) return;

    float sidForce = relSideways * walkAcceleration * Time.deltaTime;
    float forForce = relForward * walkAcceleration * Time.deltaTime;
    float upForce = 0f;
// 		if (rbody.velocity.magnitude < playerSpeed) {
// 			upForce = (floorDot - 1.0f)/1.0f * playerSpeed;
// 			if (upForce != 0) Debug.Log("upForce is " + upForce.ToString());
// 		}

    if (isSprinting) {
        sidForce *= 1.75f;
        forForce *= 2.00f;
    }

    Vector3 movDir = rbody.velocity;
    movDir.y = 0;
    movDir = movDir.normalized;
    if (floorDot < 0.98f) {
        if (Vector3.Dot(movDir,floorAng) < 0f) {
            if (Inventory.a.BoosterActive()) forForce *= 2f;
        }
    }

    if (grounded || Inventory.a.JumpJetsActive()) {
        // Normal walking
        runTime += Time.deltaTime;
        if (relForward == 0 && relSideways == 0) runTime = 0;

        rbody.AddRelativeForce(sidForce,upForce,forForce);
        movDir = rbody.velocity; // Updated after force add.
        movDir.y = 0;
        if (floorDot > 0.9f) rbody.velocity = movDir;
        movDir = movDir.normalized;
        if (fatigueFinished2 < PauseScript.a.relativeTime
            && movDir.sqrMagnitude > 0f && grounded
            && (relForward != 0 || relSideways != 0)) {

            fatigueFinished2 = PauseScript.a.relativeTime
                                + fatigueWaneTickSecs;

            if (!Inventory.a.BoosterActive()) {
                if (isSprinting) fatigue += fatiguePerSprintTick;
                else fatigue += fatiguePerWalkTick;
            }
        }
    } else {

        // Sprinting in the air
        sidForce *= walkAccelAirRatio;
        forForce *= walkAccelAirRatio;
        upForce *= walkAccelAirRatio;

        // Walking in the air, we're floating in the moonlit sky, the
        // people far below are sleeping as we fly!
        rbody.AddRelativeForce(sidForce,upForce,forForce);
    }

    if (staminupActive || FatigueCheat) fatigue = 0;
}

void FallDamage() {
    if (CheatNoclip) return;
    if (ladderState > 0) return;

    // Handle fall damage (no impact damage in cyber space 5/5/18, JJ)
    float velChange = Mathf.Abs((oldVelocity.y - rbody.velocity.y));
    if (velChange >= fallDamageSpeed) {
        DamageData dd = new DamageData ();
        float falltake = fallDamage - UnityEngine.Random.Range(0,68f);
        if (falltake > hm.health && falltake - hm.health < 5f) falltake = hm.health - 1f; // some small saving grace
        dd.damage = falltake; // No need for GetDamageTakeAmount since this is strictly internal to Player
        dd.attackType = AttackType.None;
        dd.offense = 0f;
        dd.isOtherNPC = false;
        // No impact force from fall damage.
        hm.TakeDamage (dd);
        PlayerHealth.a.makingNoise = true;
    }
    
    if (velChange >= 3f) {
        Physics.Raycast(transform.position, Vector3.down,out tempHit,feetRayLength,Const.a.layerMaskPlayerFeet);
        if (tempHit.collider == null) return;
        
        GameObject hitGO = tempHit.collider.transform.gameObject;
        PrefabIdentifier prefID = hitGO.GetComponent<PrefabIdentifier>();
        if (prefID == null) {
            if (hitGO.transform.parent != null) {
                prefID = hitGO.transform.parent.gameObject.GetComponent<PrefabIdentifier>();
            }
        }
        if (prefID == null) return;
        
        FootStepType fstep = GetFootstepTypeForPrefab(prefID.constIndex);
        AudioClip stcp = JumpLandSound(fstep);
        float vol = Mathf.Max(Mathf.Min(1f - ((fallDamageSpeed - velChange) / fallDamageSpeed),1f),0.5f);
        Utils.PlayTempAudio(transform.position - feetOffset,stcp,vol);
    }
}

void CyberspaceMovement() {
    if (currentLevel != LEVEL_CYBERSPACE) return;
    if (CheatNoclip) return;

    leanTransform.localRotation = Quaternion.Euler(0, 0, 0);
    leanTransform.localPosition = new Vector3(0,0,0);
    if (rbody.velocity.magnitude > maxCyberUltimateSpeed) {
        // Limit movement speed in all axes x,y,z in cyberspace
        RigidbodySetVelocity(rbody, maxCyberUltimateSpeed);
    }

    inputtingMovement = false;

    if (GetInput.a.Forward()) {
        if (turboFinished > PauseScript.a.relativeTime) {
            if (Vector3.Project(rbody.velocity, (cameraObject.transform.forward)).magnitude < playerSpeed * 2f)
                rbody.AddForce(cameraObject.transform.forward * walkAcceleration * 1.3f * 2f * Time.deltaTime,ForceMode.Acceleration); // double speed with turbo on
        } else {
            if (Vector3.Project(rbody.velocity, cameraObject.transform.forward).magnitude < playerSpeed)
                rbody.AddForce(cameraObject.transform.forward * walkAcceleration * 1.3f * Time.deltaTime,ForceMode.Acceleration);
        }
        inputtingMovement = true;
    }

    if (GetInput.a.Backpedal()) {
        if (turboFinished > PauseScript.a.relativeTime) {
            if (Vector3.Project(rbody.velocity, (cameraObject.transform.forward * -1f)).magnitude < playerSpeed * 2f)
            rbody.AddForce(cameraObject.transform.forward * walkAcceleration * 1.3f * 2f * Time.deltaTime * -1f,ForceMode.Acceleration); // double speed with turbo on
        } else {
            if (Vector3.Project(rbody.velocity, cameraObject.transform.forward * -1f).magnitude < playerSpeed) 
            rbody.AddForce(cameraObject.transform.forward * walkAcceleration * 1.3f * Time.deltaTime * -1f,ForceMode.Acceleration);
        }
        inputtingMovement = true;
    }

    if (GetInput.a.StrafeLeft()) {
        if (turboFinished > PauseScript.a.relativeTime) {
            if (Vector3.Project(rbody.velocity, (cameraObject.transform.right * -1f)).magnitude < playerSpeed * 2f)
            rbody.AddForce(cameraObject.transform.right * walkAcceleration * 1.3f * 2f * Time.deltaTime * -1f,ForceMode.Acceleration); // double speed with turbo on
        } else {
            if (Vector3.Project(rbody.velocity, cameraObject.transform.right * -1f).magnitude < playerSpeed) 
            rbody.AddForce(cameraObject.transform.right * walkAcceleration * 1.3f * Time.deltaTime * -1f,ForceMode.Acceleration);
        }
        inputtingMovement = true;
    }

    if (GetInput.a.StrafeRight()) {
        if (turboFinished > PauseScript.a.relativeTime) {
            if (Vector3.Project(rbody.velocity, cameraObject.transform.right).magnitude < playerSpeed * 2f)
            rbody.AddForce(cameraObject.transform.right * walkAcceleration * 1.3f * 2f * Time.deltaTime,ForceMode.Acceleration); // double speed with turbo on
        } else {
            if (Vector3.Project(rbody.velocity, cameraObject.transform.right).magnitude < playerSpeed) 
            rbody.AddForce(cameraObject.transform.right * walkAcceleration * 1.3f * Time.deltaTime,ForceMode.Acceleration);
        }
        inputtingMovement = true;
    }

    if (Const.a.difficultyCyber > 1) {
        if (rbody.velocity.magnitude < walkAcceleration * 0.05f) {
            tempVec = MouseCursor.a.GetCursorScreenPointForRay();
            tempVec = MouseLookScript.a.playerCamera.ScreenPointToRay(tempVec).direction;
            rbody.AddForce(tempVec * walkAcceleration*0.05f * Time.deltaTime); // turbo doesn't affect detrimental forces :)
        }
    } else {
        if (!inputtingMovement && !inCyberTube) rbody.velocity = Const.a.vectorZero;
    }
}

void Noclip() {
    if (!CheatNoclip) return;

    rbody.AddRelativeForce(relSideways * 2f * walkAcceleration * Time.deltaTime, 0, relForward * 2f * walkAcceleration * Time.deltaTime);
    if (GetInput.a.SwimUp()) rbody.AddRelativeForce(0, 4f * walkAcceleration * Time.deltaTime, 0); // Noclip up and down
    if (GetInput.a.SwimDn()) rbody.AddRelativeForce(0, 4f * walkAcceleration * Time.deltaTime * -1, 0);
}

Vector2 GetClampedHorizontalMovement() {
    horizontalMovement = new Vector2(rbody.velocity.x, rbody.velocity.z);
    if (horizontalMovement.magnitude > playerSpeed) {
        horizontalMovement = horizontalMovement.normalized;
        horizontalMovement *= playerSpeed; // Cap velocity to current max.
    }
    return horizontalMovement;
}

float GetClampedVerticalMovement() {
    if (grounded && !isSprinting) return 0f; // Prevent inadvertent view
                                                // bob from floating.
    if (rbody.velocity.y >= maxVerticalSpeed) return maxVerticalSpeed;
    return rbody.velocity.y;
}

void FatigueApply() {
    if (fatigue > 100f) fatigue = 100f; // Clamp at 100% maximum
    if (fatigue < 0) fatigue = 0; // Clamp at 0% minimum.

    if (fatigue > 80f && !fatigueWarned && currentLevel != LEVEL_CYBERSPACE) {
        twm.SendWarning(Const.a.stringTable[868],0.1f,0,HUDColor.White,324);
        fatigueWarned = true;
    } else {
        fatigueWarned = false;
    }

    if (currentLevel == LEVEL_CYBERSPACE) return;
    if (CheatNoclip || FatigueCheat) { fatigue = 0; return; }
    if (fatigueFinished >= PauseScript.a.relativeTime) return;

    fatigueFinished = PauseScript.a.relativeTime + fatigueWaneTickSecs;
    switch (bodyState) {
        case BodyState.Standing:    fatigue -= fatigueWanePerTick; break;
        case BodyState.Crouch:      fatigue -= fatigueWanePerTickCrouched; break;
        case BodyState.StandingUp:  fatigue -= fatigueWanePerTickCrouched; break;
        case BodyState.ProningDown: fatigue -= fatigueWanePerTickCrouched; break;
        case BodyState.Prone:       fatigue -= fatigueWanePerTickProne; break;
        case BodyState.ProningUp:   fatigue -= fatigueWanePerTickProne; break;
        default: fatigue -= fatigueWanePerTick; break;
    }
    if (fatigue < 0) fatigue = 0; // Clamp at 0% minimum.
}

void EndCrouchProneTransition() {
    if (currentLevel == LEVEL_CYBERSPACE) return;

    if (currentCrouchRatio >= 1) {
        if (bodyState == BodyState.StandingUp // Should overshoot slightly.
            || bodyState == BodyState.Standing) { // Maintain it.
            currentCrouchRatio = 1; //Clamp it
            bodyState = BodyState.Standing;
        }
    } else if (currentCrouchRatio < crouchRatio) {
        if (bodyState == BodyState.CrouchingDown // Should undershoot slightly
            || bodyState == BodyState.Crouch) { // Maintain it.
            currentCrouchRatio = crouchRatio; //Clamp it
            bodyState = BodyState.Crouch;
        } else if (bodyState == BodyState.ProningDown // Should undershoot slightly
                    || bodyState == BodyState.Prone) { // Maintain it.
            if (currentCrouchRatio < proneRatio) {
                currentCrouchRatio = proneRatio; //Clamp it
                bodyState = BodyState.Prone;
            }
        }
    } else {
        if (bodyState == BodyState.ProningUp) { // Should overshoot slightly
            if (currentCrouchRatio > crouchRatio) {
                currentCrouchRatio = crouchRatio; //Clamp it
                bodyState = BodyState.Crouch;
            }
        }
    }
}

void Prone() {
    if (currentLevel == LEVEL_CYBERSPACE) return;
    if (CheatNoclip) return;
    if (consoleActivated) return;
    if (!GetInput.a.Prone()) return;

    if (bodyState != BodyState.Prone && bodyState != BodyState.ProningDown) {
        bodyState = BodyState.ProningDown;
    } else {
        if (bodyState == BodyState.Prone || bodyState == BodyState.ProningDown) {
            if (CantStand()) {
                if (CantCrouch()) {
                    Const.sprint(Const.a.stringTable[188]);
                    return; // Can't crouch here
                } else bodyState = BodyState.ProningUp; // Can't stand, but can crouch here

                return;
            }
            
            bodyState = BodyState.StandingUp;
        }
    }
}

bool CantStand() {
    // Capsule default height is 2f.
    // 0.02f cushion added to 0.16f dist to top of head from camera.
    // 0.18f = 0.02f + (capsuleHeight * 0.5f) - Const.a.playerCameraOffsetY
    // = 0.02f + (1 - 0.84f) = 0.02f + 0.16f
    //
    // Crouch/Prone add:
    // 1.6f = capsule height (2f) - (capsule height (2f) * prone ratio (0.2f)) = 2f - 0.4f.
    float ofsY = ((1f - Const.a.playerCameraOffsetY) + 0.02f
                    + ((1f - currentCrouchRatio) * 1.6f)); // Crouch/Prone add

    Vector3 ofs = new Vector3(0f,ofsY,0f);
    return Physics.CheckCapsule(cameraObject.transform.position,
                                cameraObject.transform.position + ofs,
                                capsuleRadius,layerMask);
}

bool CantCrouch() {
    return Physics.CheckCapsule(cameraObject.transform.position,
                                cameraObject.transform.position
                                + new Vector3(0f,0.2f,0f),
                                capsuleRadius,layerMask);
}

void Crouch() {
    if (currentLevel == LEVEL_CYBERSPACE) return;
    if (CheatNoclip) return;
    if (consoleActivated) return;
    if (!GetInput.a.Crouch()) return;

    if ((bodyState == BodyState.Crouch) || (bodyState == BodyState.CrouchingDown)) {
        if (CantStand()) Const.sprint(Const.a.stringTable[187]); // Can't stand here
        else bodyState = BodyState.StandingUp; // Start standing up
    } else {
        if ((bodyState == BodyState.Standing) || (bodyState == BodyState.StandingUp)) {
            bodyState = BodyState.CrouchingDown; // Start crouching down
        } else {
            if ((bodyState == BodyState.Prone) || (bodyState == BodyState.ProningDown)) {
                if ((CantCrouch())) { Const.sprint(Const.a.stringTable[188]); return; } // Can't crouch here
                
                bodyState = BodyState.ProningUp; // Start getting up to crouch
            }
        }
    }
}

bool GetSprintInputState() {
    if (consoleActivated) return false;

    bool conditions = (grounded || CheatNoclip || ladderState > 0
                        || gravliftState);

    if (GetInput.a.Sprint()) {
        if (conditions) return !(GetInput.a.CapsLockOn());
        return false;
    } else {
        if (conditions) return GetInput.a.CapsLockOn();
        return false; // Can't sprint in the air.
    }
}

void CyberSetup() {
    if (currentLevel == LEVEL_CYBERSPACE && !cyberSetup) {
        cyberCollider.enabled = true;
        capsuleCollider.enabled = false;
        oldBodyState = bodyState;
        bodyState = BodyState.Standing; // Put to "standing" to prevent speed anomolies
        cyberSetup = true;
        cyberDesetup = true;
    }
}

void CyberDestupOrNoclipMaintain() {
    if (cyberDesetup || CheatNoclip) {
        cyberDesetup = false;
        cyberSetup = false;
        cyberCollider.enabled = false; // Can't touch dis!
        Mathf.Clamp(MouseLookScript.a.xRotation, -90f, 90f); // Pre-clamp camera rotation.
        bodyState = oldBodyState; // Return to what we were doing in the "real world" (real lol)
        if (CheatNoclip) { // Flying cheat...also map editing mode!
            capsuleCollider.enabled = false; //na nana na, na na, can't touch dis
            leanCapsuleCollider.enabled = false;
        } else {
            capsuleCollider.enabled = true;
            leanCapsuleCollider.enabled = true;
        }
    }
}

void ApplyGroundFriction() {
    if (running) {
        if (!CheatNoclip) {
            if (isSprinting) return;
        } else {
            if (isSprinting && running) {
                if (GetInput.a.SwimUp()) return;
                if (GetInput.a.SwimDn()) return;
            }
        }
    }

    tempVecRbody = rbody.velocity;
    Vector3 movDir = rbody.velocity;
    movDir.y = 0;
    movDir = movDir.normalized;
    if (Vector3.Dot(movDir,floorAng) < 0f && running) return;

    deceleration = walkDeacceleration;
    if (!grounded && ladderState < 1 && !justJumped) deceleration *= 1.5f;
    if (CheatNoclip) {
        deceleration = 0.05f;
        // Prevent gravity from affecting and decelerate like a horizontal.
        tempVecRbody.y = Mathf.SmoothDamp(rbody.velocity.y,0,
                                            ref walkDeaccelerationVoly,
                                            deceleration);
        if (isSprinting && running) return;
    } else {
        if (Inventory.a.BoosterActive()) {
            deceleration = walkDeaccelerationBooster;
        }

        tempVecRbody.y = rbody.velocity.y; // Don't affect gravity and let 
                                            // gravity keep pulling down.
    }

    tempVecRbody.x = Mathf.SmoothDamp(rbody.velocity.x,0,
                                        ref walkDeaccelerationVolx,
                                        deceleration);

    tempVecRbody.z = Mathf.SmoothDamp(rbody.velocity.z,0,
                                        ref walkDeaccelerationVolz,
                                        deceleration);
    if (currentLevel == LEVEL_CYBERSPACE) {
        tempVecRbody.y = Mathf.SmoothDamp(rbody.velocity.y,0,
                                            ref walkDeaccelerationVolz,
                                            deceleration);
    }

    rbody.velocity = tempVecRbody;
}

void Lean() {
    if (currentLevel == LEVEL_CYBERSPACE) return; // 6dof handled in MouseLookScript for this.
    if (CheatNoclip) return;

    if (GetInput.a.LeanRight()) {
        float trigFrac = Input.GetAxisRaw("JoyAxis3"); // L2
        float spd = leanSpeed;
        if (trigFrac > 0) spd *= trigFrac;
        leanTarget -= (spd * Time.deltaTime);
        if (leanTarget < (leanMaxAngle * -1)) {
            leanTarget = (leanMaxAngle * -1);
        }

        leanShift = -1 * (leanMaxShift * (leanTarget/leanMaxAngle));
    }
    if (GetInput.a.LeanLeft()) {
        float trigFrac = Input.GetAxisRaw("JoyAxis6"); // R2
        float spd = leanSpeed;
        if (trigFrac > 0) spd *= trigFrac;
        leanTarget += (spd * Time.deltaTime);
        if (leanTarget > leanMaxAngle) leanTarget = leanMaxAngle;
        leanShift = leanMaxShift * (leanTarget/(leanMaxAngle * -1));
    }
    leanTransform.localRotation = Quaternion.Euler(0, 0, leanTarget);
    leanTransform.localPosition = new Vector3(leanShift,0,0);
}

bool GetGravity() {
    if (currentLevel == LEVEL_CYBERSPACE) return false;
    if (CheatNoclip) return false;
    if (ladderState > 0) return false;
    if (bodyState == BodyState.StandingUp
        || bodyState == BodyState.CrouchingDown
        || bodyState == BodyState.ProningDown
        || bodyState == BodyState.ProningUp) {
        bodyLerpGravityOffDelayFinished = 0;
        return true;
    } else {
        if (bodyLerpGravityOffDelayFinished == 0) {
            bodyLerpGravityOffDelayFinished = PauseScript.a.relativeTime + 0.25f;
        }

        if (bodyLerpGravityOffDelayFinished > PauseScript.a.relativeTime) {
            return true;
        }
    }
    if (isSprinting) return true;

    // Disables gravity when touching steep ground to prevent player
    // sliding down ramps...hacky?
    if (grounded && floorDot >= slideAngle) return false;
    return true;
}

// Reset grounded to false when player is mid-air
void OnCollisionExit (){
    if (!PauseScript.a.Paused() && !PauseScript.a.MenuActive()) {
        // Automatically set grounded to false to prevent ability to climb any wall (Cheat!)
        if (!CheatWallSticky) {
            grounded = false;
        }
    }
}

private Manifold contactManifold;

// Sets grounded based on normal angle of the impact point (NOTE: This is not the surface normal!)
void OnCollisionStay(Collision collision) {
    if (PauseScript.a.Paused() || currentLevel == LEVEL_CYBERSPACE) return;
    
    int contactCount = collision.contactCount;
    float maxSlope = 0.35f;
    if (Inventory.a.BoosterActive()) maxSlope = 0.7f;
    for(tempInt=0;tempInt<collision.contactCount;tempInt++) {
        contactManifold = GetContact(tempInt,&contactPoint);
        floorAng = contactManifold.normal;
        floorDot = Vector3.Dot(floorAng,Vector3.up);
        if (floorDot <= 1f && floorDot >= maxSlope) {
            if (!grounded) stepFinished = PauseScript.a.relativeTime;
            grounded = true;
            return;
        }
    }
}

public void EnableCheatArsenal(int lev) {
    GameObject arsenal;
    switch(lev) {
        case 0: arsenal = cheatLRarsenal; break;
        case 1: arsenal = cheatL1arsenal; break;
        case 2: arsenal = cheatL2arsenal; break;
        case 3: arsenal = cheatL3arsenal; break;
        case 4: arsenal = cheatL4arsenal; break;
        case 5: arsenal = cheatL5arsenal; break;
        case 6: arsenal = cheatL6arsenal; break;
        case 7: arsenal = cheatL7arsenal; break;
        case 8: arsenal = cheatL8arsenal; break;
        case 9: arsenal = cheatL9arsenal; break;
        case 10: arsenal = cheatL6arsenal; break;
        case 11: arsenal = cheatL6arsenal; break;
        case 12: arsenal = cheatL6arsenal; break;
        default: arsenal = cheatL1arsenal; break;
    }
    GameObject cheatArsenal = Instantiate(arsenal,transform.position,
                                Const.a.quaternionIdentity) as GameObject;
                                
    if (cheatArsenal == null) return; // Failed!

    Transform prt = LevelManager.a.GetCurrentDynamicContainer().transform;
    cheatArsenal.transform.SetParent(prt);
    int childCount = cheatArsenal.transform.childCount;
    for (int i=childCount - 1;i>= 0; i--) {
        cheatArsenal.transform.GetChild(i).SetParent(prt);
    }
}

// Parse surface below to allow for playing different footstep sets for
// different types of flooring.
void FeetRayChecks() {
    if (currentLevel == LEVEL_CYBERSPACE) return;

    // Using value of 1.06 = (player capsule height / 2) + 0.06 = 1 + 0.06;
    bool successfulRay = Physics.Raycast(transform.position, Vector3.down,
                                            out tempHit,1.1f,
                                            Const.a.layerMaskPlayerFeet);

    //Debug.Log("Feet ray 1 success: " + successfulRay.ToString());
    // Success here means hit a useable something.
    // If a ray hits a wall or other unusable something, that's not success
    // and print "Can't use <something>"
    if (!successfulRay || tempHit.collider == null) {
// 			Debug.Log("bad ray for feet checks");
        // Automatically set grounded false, prevents ability to climb any wall
        if (!CheatWallSticky || gravliftState) grounded = false;
        return;
    }

    GameObject hitGO = tempHit.collider.transform.gameObject;
    if (hitGO == null) {
        // Automatically set grounded false, prevents ability to climb any wall
        if (!CheatWallSticky || gravliftState) grounded = false;
        return;
    }

    if (!Const.a.Footsteps) {
        SFXClothes.Stop();
        SFXFootsteps.Stop();
        return;
    }

    if (rbody.velocity.sqrMagnitude <= 0.05f) {
        SFXClothes.Stop();
    }

    if ((Mathf.Abs(relForward) + Mathf.Abs(relSideways)) == 0) return;

    if (rustleFinished < PauseScript.a.relativeTime) {
        rustleFinished = isSprinting
                            ? PauseScript.a.relativeTime
                            + UnityEngine.Random.Range(0.4f,0.6f)
                            : PauseScript.a.relativeTime
                            + UnityEngine.Random.Range(0.8f,1.2f);

        AudioClip rustle =
            Const.a.sounds[UnityEngine.Random.Range(459,465 + 1)];

        Utils.PlayOneShotSavable(SFXClothes,rustle,
                                    UnityEngine.Random.Range(0.3f,0.5f));
    }

    if (!grounded) return;

    successfulRay = Physics.Raycast(transform.position, Vector3.down,
                                    out tempHit,feetRayLength,
                                    Const.a.layerMaskPlayerFeet);
    
    if (tempHit.collider == null) return;
// 		Debug.DrawRay(transform.position,tempHit.point,Color.green,1f,true);
    hitGO = tempHit.collider.transform.gameObject;
    PrefabIdentifier prefID = hitGO.GetComponent<PrefabIdentifier>();
    if (prefID == null) {
        if (hitGO.transform.parent != null) {
            prefID = hitGO.transform.parent.gameObject.GetComponent<PrefabIdentifier>();
        }
    }
    if (prefID == null) return;

    // Footsteps
    if (stepFinished < PauseScript.a.relativeTime) {
        stepFinished = isSprinting
                        ? PauseScript.a.relativeTime
                            + UnityEngine.Random.Range(0.2f,0.3f)
                        : PauseScript.a.relativeTime
                            + UnityEngine.Random.Range(0.35f,0.65f);

        FootStepType fstep = GetFootstepTypeForPrefab(prefID.constIndex);
        AudioClip stcp = FootStepSound(fstep);
        Utils.PlayOneShotSavable(SFXFootsteps,stcp,
                                    UnityEngine.Random.Range(0.4f,0.55f));
    }
}

float GetBasePlayerSpeed() {
    // Cheat speeds
    if (CheatNoclip && isSprinting) return maxCyberSpeed * 2.5f;
    if (CheatNoclip) return maxCyberSpeed * 1.5f;

    if (currentLevel == LEVEL_CYBERSPACE) return maxCyberSpeed; //Cyber space speed

    float retval = maxWalkSpeed;
    bonus = 0f;
    if (Inventory.a.BoosterActive()) bonus = boosterSpeedBoost;
    switch (bodyState) {
        case BodyState.Standing: 		retval = maxWalkSpeed;   break;
        case BodyState.Crouch: 			retval = maxCrouchSpeed; break;
        case BodyState.CrouchingDown: 	retval = maxCrouchSpeed; break;
        case BodyState.StandingUp: 		retval = maxWalkSpeed;   break;
        case BodyState.Prone: 			retval = maxProneSpeed;  break;
        case BodyState.ProningDown: 	retval = maxProneSpeed;  break;
        case BodyState.ProningUp: 		retval = maxProneSpeed;  break;
    }

    if ((isSprinting || Inventory.a.BoosterActive()) && running) {
        if (fatigue > 80f && !Inventory.a.BoosterActive()) {
            retval = maxSprintSpeedFatigued;
        } else {
            retval = maxSprintSpeed;
        }

        if (bodyState == BodyState.Standing
            || bodyState == BodyState.Crouch
            || bodyState == BodyState.CrouchingDown) {

            // Subtract off the difference in speed between walking and
            // crouching from the sprint speed
            retval -= ((maxWalkSpeed - maxCrouchSpeed)*1.5f);
        } else if (bodyState == BodyState.Prone
                    || bodyState == BodyState.ProningDown
                    || bodyState == BodyState.ProningUp) {

            // Subtract off the difference in speed between walking and
            // proning from the sprint speed.
            retval -= ((maxWalkSpeed - maxProneSpeed)*2f);
        }
    }

    return retval + bonus;
}

void ApplyBodyStateLerps() {
    switch (bodyState) {
    case BodyState.CrouchingDown:
        currentCrouchRatio = Mathf.SmoothDamp(currentCrouchRatio,-0.01f,
                                                ref crouchingVelocity,
                                                transitionToCrouchSec);
        break;
    case BodyState.StandingUp:
        lastCrouchRatio = currentCrouchRatio;
        currentCrouchRatio = Mathf.SmoothDamp(currentCrouchRatio,1.01f,
                                                ref crouchingVelocity,
                                                transitionToCrouchSec);

        LocalPositionSetY(transform,(((currentCrouchRatio - lastCrouchRatio)
                                        * capsuleHeight) / 2)
                                    + transform.position.y);
        break;
    case BodyState.ProningDown:
        currentCrouchRatio = Mathf.SmoothDamp(currentCrouchRatio,-0.01f,
                                                ref crouchingVelocity,
                                                transitionToCrouchSec);
        break;
    case BodyState.ProningUp: // Prone to crouch
        lastCrouchRatio = currentCrouchRatio;
        currentCrouchRatio = Mathf.SmoothDamp(currentCrouchRatio,1.01f,
                                                ref crouchingVelocity,
                                                (transitionToCrouchSec
                                                + transitionToProneAdd));

        LocalPositionSetY(transform,(((currentCrouchRatio - lastCrouchRatio)
                                        * capsuleHeight) / 2)
                                    + transform.position.y);
        break;
    }
}

void SetRunningRelForwardsAndSidewaysFlags() {
    relForward = GetInput.a.Backpedal() ? -1f : 0f;
    if (GetInput.a.Forward()) {
        relForward = 1f;
    }

    relSideways = GetInput.a.StrafeLeft() ? -1f : 0f;
    if (GetInput.a.StrafeRight()) relSideways = 1f;

    // Now check for thumbstick/joystick input
    Vector2 leftThumbstick = new Vector2(
        Input.GetAxisRaw("JoyAxis1"), // Horizontal Left < 0, Right > 0
        Input.GetAxisRaw("JoyAxis2") * -1f // Vertical Down > 0,
                                            //   Up < 0 Inverted
    );

    Vector2 leftTouchstick = GetInput.a.leftTS.Coordinate();
    relForward += leftThumbstick.y + leftTouchstick.y;
    relSideways += leftThumbstick.x + leftTouchstick.x;

    // We are mashing a run button down.
    running = ((relForward != 0) || (relSideways != 0));

    float leanReset = relForward != 0.0f ? 1f : 0f;

    if (leanReset == 0) return; // Don't affect lean moving non-forward.
    if (currentLevel == LEVEL_CYBERSPACE) return; // Don't affect lean transform in cyber.

    if (leanTarget > 0) {
        if (Mathf.Abs(leanTarget - 0) < 0.05f) {
            leanShift = 0;
            leanTarget = 0;
        } else {
            leanTarget -= (leanSpeed * Time.deltaTime * leanReset);
        }

        if (Mathf.Abs(leanShift - 0) < 0.05f) {
            leanShift = 0;
            leanTarget = 0;
        } else {
            leanShift = -1 * (leanMaxShift * (leanTarget/leanMaxAngle))
                        * leanReset;
        }
    } else {
        if (Mathf.Abs(leanTarget - 0) < 0.05f) {
            leanShift = 0;
            leanTarget = 0;
        } else {
            leanTarget += (leanSpeed * Time.deltaTime * leanReset);
        }

        if (Mathf.Abs(leanShift - 0) < 0.05f) {
            leanShift = 0;
            leanTarget = 0;
        } else {
            leanShift = leanMaxShift * (leanTarget/(leanMaxAngle * -1))
                        * leanReset;
        }
    }
}

void Update() {
    if (gamePaused || (playerMovement.ressurectingFinished >= pauseRelativeTime) || menuActive || consoleActive) return;

    CyberSetup();
    CyberDestupOrNoclipMaintain();
//     Crouch(); TODO
//     Prone(); TODO
//     EndCrouchProneTransition(); TODO
//     FatigueApply(); // Here fatigue me out, except in cyberspace TODO
//     Automap.a.UpdateAutomap(transform.localPosition); // Update the map. TODO
    if (capsuleCollider.height != (currentCrouchRatio * 2.0f)) capsuleCollider.height = currentCrouchRatio * 2.0f; // Crouch/Prone by shrinking the capsule height.
    if (leanCapsuleCollider.height != capsuleCollider.height) leanCapsuleCollider.height = capsuleCollider.height; // Lean capsule should always match stalk capsule.
    SetRunningRelForwardsAndSidewaysFlags();
    playerSpeed = GetBasePlayerSpeed();
    ApplyBodyStateLerps(); // Handle body lerping for smooth transitions.
    Noclip();
    ApplyGroundFriction();
    bool grav = GetGravity();
    if (playerMovement.useGravity != grav) playerMovement.useGravity = grav;
    if (currentLevel == LEVEL_CYBERSPACE) {
        if (rbody.velocity.magnitude > playerSpeed && !CheatNoclip) {
            rbody.velocity = rbody.velocity.normalized * playerSpeed;
        }

        CyberspaceMovement();
        return;
    }

    // Non-cyberspace Normal Movement
    // --------------------------------------------------------------------
    // Clamp horizontal movement speed.
    horizontalMovement = GetClampedHorizontalMovement();
    RigidbodySetVelocityX(rbody, horizontalMovement.x);
    RigidbodySetVelocityZ(rbody, horizontalMovement.y); // NOT A BUG:
                                                        // Already passed
                                                        // rbody.velocity.z
                                                        // into the .y of
                                                        // this Vector2.
    // Clamp vertical movement speed.
    verticalMovement = GetClampedVerticalMovement();
    RigidbodySetVelocityY(rbody, verticalMovement);
    Lean();
    WalkRun();
    LadderStates();			 
    Jump();
    FallDamage();
    FeetRayChecks();
    oldVelocity = rbody.velocity;
}*/
