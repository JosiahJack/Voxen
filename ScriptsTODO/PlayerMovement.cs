	int SFXJump = 135;
	int SFXJumpLand = 136;
	int SFXLadder = 137;
	private float feetRayLength = 5f;
	int ladderState = 0; // save
	public bool gravliftState = false; // save
	bool inCyberSpace = false; // save
	float walkAcceleration = 2000f;
	int SFXIndex = -1; // save
	private float walkDeacceleration = 0.1f; // was 0.30f
	private float walkDeaccelerationBooster = 0.5f; // was 2f, adjusted player physics material to reduce friction for moving up stairs
	private float deceleration;
	private float walkAccelAirRatio = 0.75f;
	private float maxWalkSpeed = 3.2f;
	private float maxCyberSpeed = 5f;
	private float maxCyberUltimateSpeed = 12f;
	private float maxCrouchSpeed = 1.25f; //1.75f
	private float maxProneSpeed = .5f; //1f
	private float maxSprintSpeed = 8.8f;
	private float maxSprintSpeedFatigued = 5.5f;
	private float maxVerticalSpeed = 10f;
	private float boosterSpeedBoost = 1.2f; // ammount to boost by when booster is active
	private float jumpImpulseTime = 4.0f;
	private float jumpVelocityBoots = 0.45f;
	private float jumpVelocity = 1.1f;
	private float jumpVelocityFatigued = 0.6f;
	public float crouchRatio = 0.6f;
	public float proneRatio = 0.2f;
	public float transitionToCrouchSec = 0.2f;
	public float transitionToProneAdd = 0.1f;
	public float currentCrouchRatio = 1f; // save
	public float capsuleHeight;
	private float capsuleRadius;
	private float ladderSpeed = 0.4f;
	private float fallDamage = 75f;
	bool CheatWallSticky; // save
    bool CheatNoclip; // save
    bool staminupActive = false;
	public V2 horizontalMovement;
	public float verticalMovement;
	float jumpTime; // save
	private float crouchingVelocity = 1f;
	private float lastCrouchRatio;
	private int layerGeometry = 9;
	private int layerMask;
	Rigidbody rbody;
	private float fallDamageSpeed = 11.72f;
	V3 oldVelocity; // save
	private float jumpFatigue = 6.5f;
	private float fatigueWanePerTick = 1f;
	private float fatigueWanePerTickCrouched = 2f;
	private float fatigueWanePerTickProne = 3.5f;
	private float fatigueWaneTickSecs = 0.3f;
	private float fatiguePerWalkTick = 0.88f;
	private float fatiguePerSprintTick = 2.85f;
	bool justJumped = false; // save
	float fatigueFinished; // save
	float fatigueFinished2; // save
	private int def1 = 1;
	public bool running = false;
	public float relForward = 0f;
	public float relSideways = 0f;
	bool cyberSetup = false; // save
	bool cyberDesetup = false; // save
	SphereCollider cyberCollider;
	CapsuleCollider capsuleCollider;
	BodyState oldBodyState; // save
	public float bonus;
    private float walkDeaccelerationVolx;
    private float walkDeaccelerationVoly;
    private float walkDeaccelerationVolz;
	bool consoleActivated; // save
	float leanTarget = 0f; // save
	float leanShift = 0f; // save
	private float leanMaxAngle = 35f;
	private float leanMaxShift = 0.8f;
	float jumpSFXFinished; // save
	float ladderSFXFinished;
	private float ladderSFXIntervalTime = 1f;
	private float jumpSFXIntervalTime = 1f;
	float jumpLandSoundFinished; // save
	float jumpJetEnergySuckTickFinished; // save
	private float jumpJetEnergySuckTick = 1f;
	private V3 tempVec;
	private V2 tempVec2;
	private int tempInt;
	private float leanSpeed = 70f;
	bool Notarget = false; // for cheat to disable enemy sight checks against this player
	bool fatigueWarned; // save
	private float burstForce = 35f;
	float doubleJumpFinished; // save
	private V3 playerHome;
	float turboFinished = 0f; // save
	float turboCyberTime = 15f;
	bool inCyberTube = false;
	float stepFinished;
	float rustleFinished;
	private int doubleJumpTicks = 0;
	private V3 tempVecRbody;
	private bool inputtingMovement;
	private float accel;
	private RaycastHit tempHit;
	public float floorDot;
	public V3 floorAng;
	private float slideAngle = 0.9f;
	private float gravFinished;
	private float bodyLerpGravityOffDelayFinished;
	private static V3 feetOffset = (V3){0f,-0.48f,0f);
	
	public static PlayerMovement a;

	void Awake() {
		a = this;
	}

    void Start() {
		currentCrouchRatio = def1;
		bodyState = BodyState_Standing;
		cyberDesetup = false;
		oldBodyState = bodyState;
		fatigueFinished = World.pauseRelativeTime;
		fatigueFinished2 = World.pauseRelativeTime;
		ladderSFXFinished = World.pauseRelativeTime;
		rbody = GetComponent<Rigidbody>();
		oldVelocity = rbody.velocity;
		capsuleCollider = GetComponent<CapsuleCollider>();
		capsuleHeight = capsuleCollider.height;
		capsuleRadius = capsuleCollider.radius;
		layerMask = def1 << layerGeometry;
		staminupActive = false;
		cyberCollider = GetComponent<SphereCollider>();
		consoleActivated = false;
		jumpLandSoundFinished = World.pauseRelativeTime;
		justJumped = false;
		jumpSFXFinished = World.pauseRelativeTime;
		fatigueWarned = false;
		jumpJetEnergySuckTickFinished = World.pauseRelativeTime;
		World.instances[PLAYER1].ressurectingFinished = World.pauseRelativeTime;
		tempInt = -1;
		doubleJumpFinished = World.pauseRelativeTime;
		doubleJumpTicks = 0;
		turboFinished = World.pauseRelativeTime;
		stepFinished = World.pauseRelativeTime;
		rustleFinished = World.pauseRelativeTime;
		bodyLerpGravityOffDelayFinished = 0;
    }

	void Update() {
		if (World.paused
			|| (ressurectingFinished >= World.pauseRelativeTime)) {
			return;
		}

		// Normal play when not paused...
		if (rbody.isKinematic) rbody.isKinematic = false; // Allow physics.
		rbody.WakeUp(); // Force player physics to never sleep.
		CyberSetup();
		if (!inCyberSpace) {
			CyberDestupOrNoclipMaintain();
		} else {
			PlayerHealth.a.makingNoise = true; // Cyber enemies more aware.
			PlayerHealth.a.noiseFinished = World.pauseRelativeTime + 0.5f;
		}

		isSprinting = GetSprintInputState();
		Crouch();
		Prone();
		EndCrouchProneTransition();
		FatigueApply(); // Here fatigue me out, except in cyberspace
		Automap.a.UpdateAutomap(World.instances[i].position); // Update the map.
	}

	void FixedUpdate() {
		// Readout for debugging in Inspector.
		playerSpeedActual = rbody.velocity.magnitude;
		
		V2 hz = new V2(rbody.velocity.x, rbody.velocity.z);
		playerSpeedHorizontalActual = hz.magnitude;

		if (World.paused || World.menuActive) return;
		if (ressurectingFinished > World.pauseRelativeTime) return;
		if (consoleActivated) return;

		// Crouch/Prone by shrinking the capsule height.
		if (capsuleCollider.height != (currentCrouchRatio * 2f)) {
			capsuleCollider.height = currentCrouchRatio * 2f;
		}

		// Lean capsule should always match stalk capsule.
		if (leanCapsuleCollider.height != capsuleCollider.height) {
			leanCapsuleCollider.height = capsuleCollider.height;
		}

		SetRunningRelForwardsAndSidewaysFlags();
		playerSpeed = GetBasePlayerSpeed();
		ApplyBodyStateLerps(); // Handle body lerping for smooth transitions.
		Noclip();
		ApplyGroundFriction();
		bool grav = GetGravity();

		// Avoid useless setting of the rbody.
		if (rbody.useGravity != grav) rbody.useGravity = grav;
		if (grav) ApplyGravity();

		if (inCyberSpace) {
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
															// this V2.
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
	}

	// Parse surface below to allow for playing different footstep sets for
	// different types of flooring.
	void FeetRayChecks() {
		if (inCyberSpace) return;

		// Using value of 1.06 = (player capsule height / 2) + 0.06 = 1 + 0.06;
		bool successfulRay = Raycast(World.instances[i].position, V3.down,
											 out tempHit,1.1f,
											 Const.a.layerMaskPlayerFeet);

		//DualLog("Feet ray 1 success: " + successfulRay.ToString());
		// Success here means hit a useable something.
		// If a ray hits a wall or other unusable something, that's not success
		// and print "Can't use <something>"
		if (!successfulRay || tempHit.collider == null) {
// 			DualLog("bad ray for feet checks");
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

		if (rbody.velocity.sqrMagnitude <= 0.05f) SFXClothes.Stop();
		if ((vabs(relForward) + vabs(relSideways)) == 0) return;

		if (rustleFinished < World.pauseRelativeTime) {
			rustleFinished = World.pauseRelativeTime + (isSprinting ? random_range(0.4f,0.6f) : random_range(0.8f,1.2f));
			AudioClip rustle = sounds[random_range_u32(459,465 + 1)];
			Utils.PlayOneShotSavable(SFXClothes,rustle,random_range(0.3f,0.5f));
		}

		if (!grounded) return;

		successfulRay = Raycast(World.instances[i].position, V3.down,out tempHit,feetRayLength,Const.a.layerMaskPlayerFeet);
		if (tempHit.collider == null) return;
		other = tempHit.collider.transform.gameObject;

		// Footsteps
		if (stepFinished < World.pauseRelativeTime) {
			stepFinished = World.pauseRelativeTime + (isSprinting ? random_range(0.2f,0.3f) : random_range(0.35f,0.65f));
			FootStepType fstep = GetFootstepTypeForPrefab(World.instances[other].constIndex);
			AudioClip stcp = FootStepSound(fstep);
			Utils.PlayOneShotSavable(SFXFootsteps,stcp,random_range(0.4f,0.55f));
		}
	}

	float GetBasePlayerSpeed() {
		// Cheat speeds
		if (CheatNoclip && isSprinting) return maxCyberSpeed * 2.5f;
		if (CheatNoclip) return maxCyberSpeed * 1.5f;
		if (inCyberSpace) return maxCyberSpeed; //Cyber space speed

		float retval = maxWalkSpeed;
		bonus = 0f;
		if (World.invP1.BoosterActive()) bonus = boosterSpeedBoost;
		switch (bodyState) {
			case BodyState_Standing: 		retval = maxWalkSpeed;   break;
			case BodyState_Crouch: 			retval = maxCrouchSpeed; break;
			case BodyState_CrouchingDown: 	retval = maxCrouchSpeed; break;
			case BodyState_StandingUp: 		retval = maxWalkSpeed;   break;
			case BodyState_Prone: 			retval = maxProneSpeed;  break;
			case BodyState_ProningDown: 	retval = maxProneSpeed;  break;
			case BodyState_ProningUp: 		retval = maxProneSpeed;  break;
		}

		if ((isSprinting || World.invP1.BoosterActive()) && running) {
			if (World.instances[PLAYER1].fatigue > 80f && !World.invP1.BoosterActive()) {
				retval = maxSprintSpeedFatigued;
			} else {
				retval = maxSprintSpeed;
			}

			if (bodyState == BodyState_Standing
				|| bodyState == BodyState_Crouch
				|| bodyState == BodyState_CrouchingDown) {

				// Subtract off the difference in speed between walking and
				// crouching from the sprint speed
				retval -= ((maxWalkSpeed - maxCrouchSpeed)*1.5f);
			} else if (bodyState == BodyState_Prone
					   || bodyState == BodyState_ProningDown
					   || bodyState == BodyState_ProningUp) {

				// Subtract off the difference in speed between walking and
				// proning from the sprint speed.
				retval -= ((maxWalkSpeed - maxProneSpeed)*2f);
			}
		}

		return retval + bonus;
	}

	void ApplyBodyStateLerps() {
		switch (bodyState) {
		case BodyState_CrouchingDown:
			currentCrouchRatio = smooth_damp(currentCrouchRatio,-0.01f, ref crouchingVelocity, transitionToCrouchSec);
			break;
		case BodyState_StandingUp:
			lastCrouchRatio = currentCrouchRatio;
			currentCrouchRatio = smooth_damp(currentCrouchRatio,1.01f, ref crouchingVelocity, transitionToCrouchSec);
			LocalPositionSetY(transform,(((currentCrouchRatio - lastCrouchRatio) * capsuleHeight) / 2) + World.instances[i].position.y);
			break;
		case BodyState_ProningDown:
			currentCrouchRatio = smooth_damp(currentCrouchRatio,-0.01f, ref crouchingVelocity, transitionToCrouchSec);
			break;
		case BodyState_ProningUp: // Prone to crouch
			lastCrouchRatio = currentCrouchRatio;
			currentCrouchRatio = smooth_damp(currentCrouchRatio,1.01f, ref crouchingVelocity, (transitionToCrouchSec + transitionToProneAdd));
			LocalPositionSetY(transform,(((currentCrouchRatio - lastCrouchRatio) * capsuleHeight) / 2) + World.instances[i].position.y);
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
		V2 leftThumbstick = new V2(
			Input.GetAxisRaw("JoyAxis1"), // Horizontal Left < 0, Right > 0
			Input.GetAxisRaw("JoyAxis2") * -1f // Vertical Down > 0,
											   //   Up < 0 Inverted
		);

		V2 leftTouchstick = GetInput.a.leftTS.Coordinate();
		relForward += leftThumbstick.y + leftTouchstick.y;
		relSideways += leftThumbstick.x + leftTouchstick.x;

		// We are mashing a run button down.
		running = ((relForward != 0) || (relSideways != 0));

		float leanReset = relForward != 0.0f ? 1f : 0f;

		if (leanReset == 0) return; // Don't affect lean moving non-forward.
		if (inCyberSpace) return; // Don't affect lean transform in cyber.

		if (leanTarget > 0) {
			if (vabs(leanTarget - 0) < 0.05f) {
				leanShift = 0;
				leanTarget = 0;
			} else {
				leanTarget -= (leanSpeed * Time.deltaTime * leanReset);
			}

			if (vabs(leanShift - 0) < 0.05f) {
				leanShift = 0;
				leanTarget = 0;
			} else {
				leanShift = -1 * (leanMaxShift * (leanTarget/leanMaxAngle))
							* leanReset;
			}
		} else {
			if (vabs(leanTarget - 0) < 0.05f) {
				leanShift = 0;
				leanTarget = 0;
			} else {
				leanTarget += (leanSpeed * Time.deltaTime * leanReset);
			}

			if (vabs(leanShift - 0) < 0.05f) {
				leanShift = 0;
				leanTarget = 0;
			} else {
				leanShift = leanMaxShift * (leanTarget/(leanMaxAngle * -1))
							* leanReset;
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
		V3 movDir = rbody.velocity;
		movDir.y = 0;
		movDir = movDir.normalized;
		if (V3.Dot(movDir,floorAng) < 0f && running) return;

		deceleration = walkDeacceleration;
		if (!grounded && ladderState < 1 && !justJumped) deceleration *= 1.5f;
		if (CheatNoclip) {
			deceleration = 0.05f;
			// Prevent gravity from affecting and decelerate like a horizontal.
			tempVecRbody.y = smooth_damp(rbody.velocity.y,0,
											  ref walkDeaccelerationVoly,
											  deceleration);
			if (isSprinting && running) return;
		} else {
			if (World.invP1.BoosterActive()) {
				deceleration = walkDeaccelerationBooster;
			}

			tempVecRbody.y = rbody.velocity.y; // Don't affect gravity and let 
											   // gravity keep pulling down.
		}

		tempVecRbody.x = smooth_damp(rbody.velocity.x,0,
										  ref walkDeaccelerationVolx,
										  deceleration);

		tempVecRbody.z = smooth_damp(rbody.velocity.z,0,
										  ref walkDeaccelerationVolz,
										  deceleration);
		if (inCyberSpace) {
			tempVecRbody.y = smooth_damp(rbody.velocity.y,0,
											  ref walkDeaccelerationVolz,
											  deceleration);
		}

		rbody.velocity = tempVecRbody;
	}

	void Lean() {
		if (inCyberSpace) return; // 6dof handled in MouseLookScript for this.
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
		leanTransform.localPosition = (V3){leanShift,0,0);
	}

	bool GetGravity() {
		if (inCyberSpace) return false;
		if (CheatNoclip) return false;
		if (ladderState > 0) return false;
		if (bodyState == BodyState_StandingUp
			|| bodyState == BodyState_CrouchingDown
			|| bodyState == BodyState_ProningDown
			|| bodyState == BodyState_ProningUp) {
			bodyLerpGravityOffDelayFinished = 0;
// 			DualLog("Crouching gravity! " + rbody.useGravity.ToString());
			return true;
		} else {
			if (bodyLerpGravityOffDelayFinished == 0) {
				bodyLerpGravityOffDelayFinished = World.pauseRelativeTime + 0.25f;
			}

			if (bodyLerpGravityOffDelayFinished > World.pauseRelativeTime) {
				return true;
			}
		}
		if (isSprinting) return true;

		// Disables gravity when touching steep ground to prevent player
		// sliding down ramps...hacky?
		if (grounded && floorDot >= slideAngle) return false;
		return true;
	}

	// Get input for Jump and set impulse time, removed
	// "&& (ladderState == 0)" since I want to be able to jump off a ladder
	void Jump() {
		if (CheatNoclip && !World.invP1.JumpJetsActive()) return;

		if (doubleJumpFinished < World.pauseRelativeTime) {
			doubleJumpTicks--;
			if (doubleJumpTicks < 0) doubleJumpTicks = 0;
		}

		if ((!gravliftState && GetInput.a.Jump()) || gravliftState && GetInput.a.JumpDown()) {

			if (!justJumped) {
				if (grounded || gravliftState || World.invP1.JumpJetsActive()) {
					jumpTime = jumpImpulseTime;
					doubleJumpFinished = World.pauseRelativeTime + Const.doubleClickTime;
					doubleJumpTicks++;
					justJumped = true;
					if (!World.invP1.JumpJetsActive() && !World.invP1.BoosterActive()) World.instances[PLAYER1].fatigue += jumpFatigue;
				} else {
					if (ladderState > 1) {
						jumpTime = jumpImpulseTime;
						justJumped = true;
						if (!World.invP1.JumpJetsActive() && !World.invP1.BoosterActive()) {
							World.instances[PLAYER1].fatigue += jumpFatigue;
						}
					}
				}
			}

			if (World.invP1.BoosterActive() && World.invP1.BoosterSetToBoost()) {
				if (justJumped && doubleJumpTicks == 2) {
					// Booster thrust
					rbody.AddForce((V3){transform.forward.x * burstForce, transform.forward.y * burstForce, transform.forward.z * burstForce), ForceMode.Impulse);
					PlayerHealth.a.makingNoise = true;
					PlayerHealth.a.noiseFinished = World.pauseRelativeTime + 0.5f;
					TakeEnergy(22f);
					if (BiomonitorGraphSystem.a != null) {
						BiomonitorEnergyPulse(22f);
					}

					justJumped = false;
					jumpTime = 0;
					doubleJumpTicks = 0;

					// Make sure we can't do it again right away.
					doubleJumpFinished = World.pauseRelativeTime - 1f;
				}
			}
		}

		if (staminupActive || FatigueCheat) World.instances[PLAYER1].fatigue = 0.0f;
		
		// Perform Jump
		float jumpVelocityApply = jumpVelocity * rbody.mass;
		V3 jumpVel = new V3 (0,jumpVelocityApply,0);
		float jumpTimeMod = jumpTime;
		if (isSprinting) jumpTimeMod *= 0.5f;
		while (jumpTimeMod > 0) { // Why is this a `while` instead of an `if`??
							   // Because otherwise it don't work, duh!
			jumpTimeMod -= Time.smoothDeltaTime;
			if (World.instances[PLAYER1].fatigue > 80.0f && !World.invP1.JumpJetsActive()) {
				jumpVelocityApply = jumpVelocityFatigued * rbody.mass;
				jumpVel.y = jumpVelocityApply;
			}

			if (World.invP1.JumpJetsActive()) {
				float energysuck = 25f;
				jumpVelocityApply = jumpVelocityBoots * rbody.mass;
				jumpVel.y = jumpVelocityApply;
				switch (World.invP1.hardwareVersionSetting[10]) {
					case 0: energysuck = 11f; break;
					case 1: energysuck = 26f; break;
					case 2: energysuck = 22f; break;
				}

				if (PlayerEnergy.a.energy >= energysuck) {
					rbody.AddForce(jumpVel,ForceMode.Force);  // huhnh!
					if (jumpJetEnergySuckTickFinished < World.pauseRelativeTime) {
						jumpJetEnergySuckTickFinished = World.pauseRelativeTime + jumpJetEnergySuckTick;
						TakeEnergy(energysuck);
						if (BiomonitorGraphSystem.a != null) {
							BiomonitorEnergyPulse(energysuck);
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

		if (justJumped && !World.invP1.JumpJetsActive()) {
			// Play jump sound
			if (jumpSFXFinished < World.pauseRelativeTime) {
				jumpSFXFinished = World.pauseRelativeTime + jumpSFXIntervalTime;
				SFX.pitch = 1f;
				float jumpSFXVolume = 1.0f;
				if (World.instances[PLAYER1].fatigue > 80.0f) jumpSFXVolume = 0.5f; // Quietly, we tired.
				
				PlayerHealth.a.makingNoise = true;
				PlayerHealth.a.noiseFinished = World.pauseRelativeTime + 0.5f;
				Raycast(World.instances[i].position, V3.down,
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
				play_wav(sounds[JumpSound(fstep)],jumpSFXVolume,V3_AsubB(World.instances[i].position,feetOffset),true);
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
		if (grounded || World.invP1.JumpJetsActive()) {
			// Ladder climb, allow while grounded
			float bonus = 1f;
			if (World.invP1.JumpJetsActive()) bonus = 2f;

			sidForce = relSideways * walkAcceleration * Time.deltaTime;
			forForce = relForward * walkAcceleration * Time.deltaTime;
			upForce = ladderSpeed * relForward * walkAcceleration
							* Time.deltaTime * bonus;

			// Climbing when touching the ground
			rbody.AddRelativeForce(sidForce,upForce,forForce);
		} else {
			// Climbing off the ground
			if (ladderSFXFinished < World.pauseRelativeTime
				&& rbody.velocity.y > ladderSpeed * 0.5f) {

				SFX.pitch = (random_range(0.8f,1.2f));
				Utils.PlayOneShotSavable(SFX,SFXLadder,0.2f);
				ladderSFXFinished = World.pauseRelativeTime
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

		if (World.invP1.BoosterActive() && World.invP1.BoosterSetToSkates()) {
			deceleration = walkDeaccelerationBooster;
		} else {
			deceleration = walkDeacceleration;
		}

		// Set vertical velocity towards 0 when climbing.
		RigidbodySetVelocityY(rbody,(smooth_damp(rbody.velocity.y,0,
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
// 			if (upForce != 0) DualLog("upForce is " + upForce.ToString());
// 		}

		if (isSprinting) {
			sidForce *= 1.75f;
			forForce *= 2.00f;
		}

		V3 movDir = rbody.velocity;
		movDir.y = 0;
		movDir = movDir.normalized;
		if (floorDot < 0.98f) {
			if (V3.Dot(movDir,floorAng) < 0f) {
				if (World.invP1.BoosterActive()) forForce *= 2f;
			}
		}

		if (grounded || World.invP1.JumpJetsActive()) {
			// Normal walking
			runTime += Time.deltaTime;
			if (relForward == 0 && relSideways == 0) runTime = 0;

			rbody.AddRelativeForce(sidForce,upForce,forForce);
			movDir = rbody.velocity; // Updated after force add.
			movDir.y = 0;
			if (floorDot > 0.9f) rbody.velocity = movDir;
			movDir = movDir.normalized;
			if (fatigueFinished2 < World.pauseRelativeTime && movDir.sqrMagnitude > 0f && grounded && (relForward != 0 || relSideways != 0)) {
				fatigueFinished2 = World.pauseRelativeTime + fatigueWaneTickSecs;
				if (!World.invP1.BoosterActive()) {
					if (isSprinting) World.instances[PLAYER1].fatigue += fatiguePerSprintTick;
					else World.instances[PLAYER1].fatigue += fatiguePerWalkTick;
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
		float velChange = vabs((oldVelocity.y - rbody.velocity.y));
		if (velChange >= fallDamageSpeed) {
			DamageData dd = new DamageData ();
			float falltake = fallDamage - random_range(0,68f);
			if (falltake > hm.health && falltake - hm.health < 5f) falltake = hm.health - 1f; // some small saving grace
			dd.damage = falltake; // No need for GetDamageTakeAmount since this is strictly internal to Player
			dd.attackType = AttType.None;
			dd.offense = 0f;
			dd.isOtherNPC = false;
			// No impact force from fall damage.
			hm.TakeDamage (dd);
			PlayerHealth.a.makingNoise = true;
		}
		
		if (velChange >= 3f) {
			Raycast(World.instances[i].position, V3.down,out tempHit,feetRayLength,Const.a.layerMaskPlayerFeet);
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
			float vol = vmax(vmin(1f - ((fallDamageSpeed - velChange) / fallDamageSpeed),1f),0.5f);
			play_wav(sounds[JumpLandSound(fstep)],vol,V3_AsubB(World.instances[i].position,feetOffset),true);
		}
	}

	void CyberspaceMovement() {
		if (!inCyberSpace) return;
		if (CheatNoclip) return;

		leanTransform.localRotation = Quaternion.Euler(0, 0, 0);
		leanTransform.localPosition = (V3){0,0,0);
		if (rbody.velocity.magnitude > maxCyberUltimateSpeed) {
			// Limit movement speed in all axes x,y,z in cyberspace
			RigidbodySetVelocity(rbody, maxCyberUltimateSpeed);
		}

		inputtingMovement = false;

		if (GetInput.a.Forward()) {
			if (turboFinished > World.pauseRelativeTime) {
				if (V3.Project(rbody.velocity, (cameraObject.transform.forward)).magnitude < playerSpeed * 2f)
					rbody.AddForce(cameraObject.transform.forward * walkAcceleration * 1.3f * 2f * Time.deltaTime,ForceMode.Acceleration); // double speed with turbo on
			} else {
				if (V3.Project(rbody.velocity, cameraObject.transform.forward).magnitude < playerSpeed)
					rbody.AddForce(cameraObject.transform.forward * walkAcceleration * 1.3f * Time.deltaTime,ForceMode.Acceleration);
			}
			inputtingMovement = true;
		}

		if (GetInput.a.Backpedal()) {
			if (turboFinished > World.pauseRelativeTime) {
				if (V3.Project(rbody.velocity, (cameraObject.transform.forward * -1f)).magnitude < playerSpeed * 2f)
				rbody.AddForce(cameraObject.transform.forward * walkAcceleration * 1.3f * 2f * Time.deltaTime * -1f,ForceMode.Acceleration); // double speed with turbo on
			} else {
				if (V3.Project(rbody.velocity, cameraObject.transform.forward * -1f).magnitude < playerSpeed) 
				rbody.AddForce(cameraObject.transform.forward * walkAcceleration * 1.3f * Time.deltaTime * -1f,ForceMode.Acceleration);
			}
			inputtingMovement = true;
		}

		if (GetInput.a.StrafeLeft()) {
			if (turboFinished > World.pauseRelativeTime) {
				if (V3.Project(rbody.velocity, (cameraObject.transform.right * -1f)).magnitude < playerSpeed * 2f)
				rbody.AddForce(cameraObject.transform.right * walkAcceleration * 1.3f * 2f * Time.deltaTime * -1f,ForceMode.Acceleration); // double speed with turbo on
			} else {
				if (V3.Project(rbody.velocity, cameraObject.transform.right * -1f).magnitude < playerSpeed) 
				rbody.AddForce(cameraObject.transform.right * walkAcceleration * 1.3f * Time.deltaTime * -1f,ForceMode.Acceleration);
			}
			inputtingMovement = true;
		}

		if (GetInput.a.StrafeRight()) {
			if (turboFinished > World.pauseRelativeTime) {
				if (V3.Project(rbody.velocity, cameraObject.transform.right).magnitude < playerSpeed * 2f)
				rbody.AddForce(cameraObject.transform.right * walkAcceleration * 1.3f * 2f * Time.deltaTime,ForceMode.Acceleration); // double speed with turbo on
			} else {
				if (V3.Project(rbody.velocity, cameraObject.transform.right).magnitude < playerSpeed) 
				rbody.AddForce(cameraObject.transform.right * walkAcceleration * 1.3f * Time.deltaTime,ForceMode.Acceleration);
			}
			inputtingMovement = true;
		}

		if (World.diffCyb > 1) {
			if (rbody.velocity.magnitude < walkAcceleration * 0.05f) {
				tempVec = ScreenPointToRay();
				tempVec = MouseLookScript.a.playerCamera.ScreenPointToRay(tempVec).direction;
				rbody.AddForce(tempVec * walkAcceleration*0.05f * Time.deltaTime); // turbo doesn't affect detrimental forces :)
			}
		} else {
			if (!inputtingMovement && !inCyberTube) rbody.velocity = (V3){0.0f,0.0f,0.0f};
		}
	}

	void Noclip() {
		if (!CheatNoclip) return;

		rbody.AddRelativeForce(relSideways * 2f * walkAcceleration * Time.deltaTime, 0, relForward * 2f * walkAcceleration * Time.deltaTime);
		if (GetInput.a.SwimUp()) rbody.AddRelativeForce(0, 4f * walkAcceleration * Time.deltaTime, 0); // Noclip up and down
		if (GetInput.a.SwimDn()) rbody.AddRelativeForce(0, 4f * walkAcceleration * Time.deltaTime * -1, 0);
	}

	V2 GetClampedHorizontalMovement() {
		horizontalMovement = new V2(rbody.velocity.x, rbody.velocity.z);
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
		if (World.instances[PLAYER1].fatigue > 100.0f) World.instances[PLAYER1].fatigue = 100.0f; // Clamp at 100% maximum
		if (World.instances[PLAYER1].fatigue <   0.0f) World.instances[PLAYER1].fatigue =   0.0f; // Clamp at   0% minimum.
		if (World.instances[PLAYER1].fatigue > 80.0f && !fatigueWarned && !inCyberSpace) {
			twm.SendWarning(Text->stringTable[868],0.1f,0,HUDColor.White,324);
			fatigueWarned = true;
		} else fatigueWarned = false;

		if (inCyberSpace) return;
		if (CheatNoclip || FatigueCheat) { World.instances[PLAYER1].fatigue = 0.0f; return; }
		if (fatigueFinished >= World.pauseRelativeTime) return;

		fatigueFinished = World.pauseRelativeTime + fatigueWaneTickSecs;
		switch (bodyState) {
			case BodyState_Standing:    World.instances[PLAYER1].fatigue -= fatigueWanePerTick; break;
			case BodyState_Crouch:      World.instances[PLAYER1].fatigue -= fatigueWanePerTickCrouched; break;
			case BodyState_StandingUp:  World.instances[PLAYER1].fatigue -= fatigueWanePerTickCrouched; break;
			case BodyState_ProningDown: World.instances[PLAYER1].fatigue -= fatigueWanePerTickCrouched; break;
			case BodyState_Prone:       World.instances[PLAYER1].fatigue -= fatigueWanePerTickProne; break;
			case BodyState_ProningUp:   World.instances[PLAYER1].fatigue -= fatigueWanePerTickProne; break;
			default:                    World.instances[PLAYER1].fatigue -= fatigueWanePerTick; break;
		}
		if (World.instances[PLAYER1].fatigue < 0) World.instances[PLAYER1].fatigue = 0; // Clamp at 0% minimum.
	}

	void EndCrouchProneTransition() {
		if (inCyberSpace) return;

		if (currentCrouchRatio >= 1) {
			if (bodyState == BodyState_StandingUp // Should overshoot slightly.
			    || bodyState == BodyState_Standing) { // Maintain it.
				currentCrouchRatio = 1; //Clamp it
				bodyState = BodyState_Standing;
			}
		} else if (currentCrouchRatio < crouchRatio) {
			if (bodyState == BodyState_CrouchingDown // Should undershoot slightly
				|| bodyState == BodyState_Crouch) { // Maintain it.
				currentCrouchRatio = crouchRatio; //Clamp it
				bodyState = BodyState_Crouch;
			} else if (bodyState == BodyState_ProningDown // Should undershoot slightly
					   || bodyState == BodyState_Prone) { // Maintain it.
				if (currentCrouchRatio < proneRatio) {
					currentCrouchRatio = proneRatio; //Clamp it
					bodyState = BodyState_Prone;
				}
			}
		} else {
			if (bodyState == BodyState_ProningUp) { // Should overshoot slightly
				if (currentCrouchRatio > crouchRatio) {
					currentCrouchRatio = crouchRatio; //Clamp it
					bodyState = BodyState_Crouch;
				}
			}
		}
	}

	void Prone() {
		if (inCyberSpace) return;
		if (CheatNoclip) return;
		if (consoleActivated) return;
		if (!GetInput.a.Prone()) return;

		if (bodyState != BodyState_Prone && bodyState != BodyState_ProningDown) {
			bodyState = BodyState_ProningDown;
		} else {
			if (bodyState == BodyState_Prone || bodyState == BodyState_ProningDown) {
				if (CantStand()) {
					if (CantCrouch()) {
						CenterStatusPrint("%s", Text->stringTable[188]);
						return; // Can't crouch here
					} else bodyState = BodyState_ProningUp; // Can't stand, but can crouch here

					return;
				}
				
				bodyState = BodyState_StandingUp;
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

		V3 ofs = (V3){0f,ofsY,0f);
		return Physics.CheckCapsule(cameraObject.World.instances[i].position,
									cameraObject.World.instances[i].position + ofs,
									capsuleRadius,layerMask);
	}

	bool CantCrouch() {
		return Physics.CheckCapsule(cameraObject.World.instances[i].position,
									cameraObject.World.instances[i].position
									+ (V3){0f,0.2f,0f),
									capsuleRadius,layerMask);
	}

	void Crouch() {
		if (inCyberSpace) return;
		if (CheatNoclip) return;
		if (consoleActivated) return;
		if (!GetInput.a.Crouch()) return;

		if ((bodyState == BodyState_Crouch) || (bodyState == BodyState_CrouchingDown)) {
			if (CantStand()) CenterStatusPrint("%s", Text->stringTable[187]); // Can't stand here
			else bodyState = BodyState_StandingUp; // Start standing up
		} else {
			if ((bodyState == BodyState_Standing) || (bodyState == BodyState_StandingUp)) {
				bodyState = BodyState_CrouchingDown; // Start crouching down
			} else {
				if ((bodyState == BodyState_Prone) || (bodyState == BodyState_ProningDown)) {
					if ((CantCrouch())) { CenterStatusPrint("%s", Text->stringTable[188]); return; } // Can't crouch here
					
					bodyState = BodyState_ProningUp; // Start getting up to crouch
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
		if (inCyberSpace && !cyberSetup) {
			cyberCollider.enabled = true;
			capsuleCollider.enabled = false;
			MouseLookScript.a.inCyberSpace = true; // Enable full camera rotation up/down by disabling clamp
			oldBodyState = bodyState;
			bodyState = BodyState_Standing; // Put to "standing" to prevent speed anomolies
			cyberSetup = true;
			cyberDesetup = true;
		}
	}

	void CyberDestupOrNoclipMaintain() {
		if (cyberDesetup || CheatNoclip) {
			cyberDesetup = false;
			cyberSetup = false;
			cyberCollider.enabled = false; // Can't touch dis!
			vclamp(MouseLookScript.a.xRotation, -90f, 90f); // Pre-clamp camera rotation.
			MouseLookScript.a.inCyberSpace = false; // Disable full camera rotation up/down by enabling auto clamp.
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

	// Reset grounded to false when player is mid-air
	void OnCollisionExit (){
		if (!World.paused && !World.menuActive) {
			// Automatically set grounded to false to prevent ability to climb any wall (Cheat!)
			if (!CheatWallSticky) {
				grounded = false;
			}
		}
	}
	
	private ContactPoint contactPoint;

	// Sets grounded based on normal angle of the impact point (NOTE: This is not the surface normal!)
	void OnCollisionStay(Collision collision) {
		if (World.paused || inCyberSpace) return;
		
		int contactCount = collision.contactCount;
		float maxSlope = 0.35f;
		if (World.invP1.BoosterActive()) maxSlope = 0.7f;
		for(tempInt=0;tempInt<collision.contactCount;tempInt++) {
			contactPoint = collision.GetContact(tempInt);;
			floorAng = contactPoint.normal;
			floorDot = V3.Dot(floorAng,V3.up);
			if (floorDot <= 1f && floorDot >= maxSlope) {
				if (!grounded) stepFinished = World.pauseRelativeTime;
				grounded = true;
				return;
			}
		}
	}
