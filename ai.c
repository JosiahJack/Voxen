// ai.c - AI logic control for NPC's enemies in the game.

// 	// External manually assigned references, but ok if not assigned
//     public GameObject searchColliderGO;
// 	public LineRenderer laserLightning;
// 	public Transform[] walkWaypoints; // point(s) for NPC to walk to when
// 									  // roaming or patrolling
// 	public Material deathMaterial;
// 	public SkinnedMeshRenderer actualSMR;
// 	public GameObject deathBurst;
// 	public GameObject sightPoint;
// 	public AIState currentState; // save (referenced by int index 0 thru 10)
// 	public bool walkPathOnStart = false; // save
//     public bool dontLoopWaypoints = false; // save
// 	public bool visitWaypointsRandomly = false; // save
// 	public bool asleep = false; // save, Check if enemy starts out asleep such
// 								// as the sleeping sec-2 bots on level 8 in the
// 								// maintenance and recharge bays.
// 	public bool wandering; // save
// 	public bool actAsTurret = false; // save
// 	public bool withinPVS = false; // True when the enemy is in visible cell.
// 	public bool visibleMeshVisible; // save
// 	public GameObject sleepingCables;
// 	
// 	// Internal, keeping exposed in inspector for troubleshooting.
// 	public GameObject enemy; // save (referenced by int index enemIDRead)
// 
// 	// Internal
// 	[HideInInspector] public string targetID; // save
// 	[HideInInspector] public bool hasTargetIDAttached = false; // save
//     [HideInInspector] public float gracePeriodFinished; // save
//     [HideInInspector] public float meleeDamageFinished; // save
// 	public bool inSight = false; // save
//     [HideInInspector] public bool infront; // save
//     [HideInInspector] public bool inProjFOV; // save
//     [HideInInspector] public bool LOSpossible; // save
//     [HideInInspector] public bool goIntoPain = false; // save
// 	public float rangeToEnemy = 999f; // save
// 	[HideInInspector] public GameObject attacker;
// 	[HideInInspector] public bool firstSighting; // save
// 	[HideInInspector] public bool dyingSetup; // save
// 	public bool ai_dying; // save
// 	public bool ai_dead; // save
// 	[HideInInspector] public int currentWaypoint; // save
// 	public Vector3 currentDestination; // save
// 	[HideInInspector] public float idleTime; // save
// 	[HideInInspector] public float attack1SoundTime; // save
// 	[HideInInspector] public float attack2SoundTime; // save
// 	[HideInInspector] public float attack3SoundTime; // save
// 	[HideInInspector] public int SFXIndex = -1; // save
// 	[HideInInspector] public float timeTillEnemyChangeFinished; // save
// 	[HideInInspector] public float timeTillDeadFinished; // save
// 	[HideInInspector] public float timeTillPainFinished; // save
// 	[HideInInspector] public AudioSource SFX;
// 	[HideInInspector] public float normalVolume;
// 	public float huntFinished; // save
// 	[HideInInspector] public bool hadEnemy; // save
// 	[HideInInspector] public Vector3 lastKnownEnemyPos; // save
// 	[HideInInspector] public Vector3 tempVec; // save
// 	private Vector3 tempVec2; // Only ever used right away, nosave
// 	[HideInInspector] public bool shotFired = false; // save
//     private DamageData damageData;
//     private RaycastHit tempHit;
//     private HealthManager tempHM;
// 	[HideInInspector] public float randomWaitForNextAttack1Finished; // save
// 	[HideInInspector] public float randomWaitForNextAttack2Finished; // save
// 	[HideInInspector] public float randomWaitForNextAttack3Finished; // save
// 	[HideInInspector] public Vector3 idealTransformForward; // save
// 	public Vector3 idealPos; // used by flyers to establish
// 											   // correct height // save
// 	[HideInInspector] public float attackFinished; // save
// 	[HideInInspector] public float attack2Finished; // save
// 	[HideInInspector] public float attack3Finished; // save
// 	[HideInInspector] public Vector3 targettingPosition; // Used to give the
// 														 // player a chance to
// 														 // dodge attacks by
// 														 // moving after start
// 														 // of an attack, enemy
// 														 // attacks along same
// 														 // starting line, save
// 	[HideInInspector] public float deathBurstFinished; // save
// 	[HideInInspector] public bool deathBurstDone; // save
// 	[HideInInspector] public float tranquilizeFinished; // save
// 	[HideInInspector] public bool hopDone; // save
// 	public float wanderFinished; // save
// 	[HideInInspector] public float timeSinceMovedEnough;
// 	private float dotResult = -1f; // Only ever used right away, nosave
// 	private Vector3 infrontVec; // Only ever used right away, nosave
// 	[HideInInspector] public bool startInitialized = false; // nosave
// 	private HealthManager enemyHM;
// 	private const float stopDistance = 1.28f; // Constant
// 	private Vector3 faceVec; // Only ever used right away, nosave
// 	private Quaternion lookRot; // Only ever used right away, nosave
// 	private bool deadChecksDone = false;
// 	private float near;
// 	private float mid;
// 	private float far;
// 	public Vector3 lastPosition;
// 	public float distToLastPos;
// 	public float posCheckFinished;
// 	private const float positionCheckDelay = 2f;
// 	private const float searchTime = 5f;
// 	StringBuilder s1 = new StringBuilder();
// 	Vector3 targetOffset = new Vector3(0f,0.24f,0f);

float AI_Tranquilize(float amount, bool energy) {
    if (Sys_AI.type[index] == NPCType_Robot && !energy) return 0.0f;

    float tranqSecs = (tranqSecs < 3.0f) ? npcTable[NPCID].timeForTranquilization : amount;
    tranquilizeFinished = Mathf.Max(Sys_Global.pauseRelativeTime + tranqSecs, tranquilizeFinished + tranqSecs);
    return tranqSecs;
}

static inline bool IsCyberNPC() { return npcTable[NPCID].type == NPCType_Cyber; }

void InitializeAI(uint16_t i) {
    attack1SoundTime = Sys_Global.pauseRelativeTime;
    attack2SoundTime = Sys_Global.pauseRelativeTime;
    attack3SoundTime = Sys_Global.pauseRelativeTime;
    timeTillEnemyChangeFinished = Sys_Global.pauseRelativeTime;
    SetHuntFinished();
    attackFinished = Sys_Global.pauseRelativeTime;
    attack2Finished = Sys_Global.pauseRelativeTime;
    attack3Finished = Sys_Global.pauseRelativeTime;
    timeTillPainFinished = Sys_Global.pauseRelativeTime;
    timeTillDeadFinished = Sys_Global.pauseRelativeTime;
    meleeDamageFinished = Sys_Global.pauseRelativeTime;
    gracePeriodFinished = Sys_Global.pauseRelativeTime;
    randomWaitForNextAttack1Finished = Sys_Global.pauseRelativeTime;
    randomWaitForNextAttack2Finished = Sys_Global.pauseRelativeTime;
    randomWaitForNextAttack3Finished = Sys_Global.pauseRelativeTime;
    tranquilizeFinished = Sys_Global.pauseRelativeTime;
    deathBurstFinished = Sys_Global.pauseRelativeTime;
    wanderFinished = Sys_Global.pauseRelativeTime;
    posCheckFinished = Sys_Global.pauseRelativeTime;
    lastPosition = transform.position;
    timeSinceMovedEnough = 0f;
    damageData = new DamageData();
    damageData.ownerIsNPC = true;
    tempHit = new RaycastHit();
    tempVec = new Vector3(0f, 0f, 0f);
    SFX = GetComponent<AudioSource>();
    SFX.playOnAwake = false;
    if (SFX.volume == 0f) SFX.volume = 1.0f;
    normalVolume = SFX.volume;
    if (walkWaypoints.Length > 0 && walkWaypoints[currentWaypoint] != null
        && walkPathOnStart && !asleep) {
        
        currentDestination = walkWaypoints[currentWaypoint].transform.position;
        currentState = AIState.Walk; // If waypoints are set, start walking
    } else {
        currentState = AIState.Idle; // No waypoints, stay put
    }

    if (wandering && (UnityEngine.Random.Range(0,1f) < 0.5f)) {
        currentState = AIState.Walk;
    } else wandering = false;

    if (asleep) {
        currentState = AIState.Idle;
        Utils.Activate(sleepingCables);
    }

    raycastingTickFinished = tickFinished + Random.value; // Separate rand.
    attackFinished = Sys_Global.pauseRelativeTime + 1f;
    idealTransformForward = sightPoint.transform.forward;
    if (!IsCyberNPC()) targetID = Const.GetTargetID(index);
    else             targetID = Const.GetCyberTargetID(index);

    if (asleep) Utils.Activate(sleepingCables);
    startInitialized = true;
}

	void AI_Face(Vector3 goalLocation) {
		if (asleep) return;

		faceVec = goalLocation - transform.position;
		if (!IsCyberNPC()) faceVec.y = 0f;
		if (faceVec.x == 0f && faceVec.z == 0f && faceVec.y == 0f) return; // Avoid zero quat error.
		if (Vector3.Dot(faceVec,Vector3.up) > 0.99f && !IsCyberNPC()) return; // Up results in no Y rotation.

		// Rotate as fast as we can towards facing the goal location.
		Vector3 up = Vector3.up;
		if (IsCyberNPC() && enemy != null) {
			up = enemy.transform.up;
			transform.rotation = enemy.transform.rotation;
			return;
		}
		
		if (goalLocation == transform.position) {
			if (enemy != null) faceVec = enemy.transform.position - transform.position;
			else faceVec.x += 0.001f;
		}
		lookRot = Quaternion.LookRotation(faceVec,up);
		transform.rotation =
			Quaternion.Slerp(transform.rotation,lookRot,Const.aiTickTime
							 * npcTable[index].yawSpeed * Time.deltaTime); 
	}

	void LateUpdate() {
		Const.a.numberOfRaycastsThisFrame = 0;
	}

	public bool HasHealth(HealthManager hm) {
		if (hm == null) return false;
		if (IsCyberNPC()) return (hm.cyberHealth > 0);
		return (hm.health > 0);
	}

	void EnableAutomapOverlay() {
		if (healthManager.linkedOverlay == null) return;

		if (healthManager.health > 0 && gameObject.activeInHierarchy) {
			Utils.EnableImage(healthManager.linkedOverlay);
		} else {
			Utils.DisableImage(healthManager.linkedOverlay);
		}
	}

	void Update() {
		if (!startInitialized) return;

		if (Sys_Global.gamePaused || Sys_Global.menuActive) return;

		rbody.isKinematic = false;
		if (raycastingTickFinished >= Sys_Global.pauseRelativeTime) return;

		raycastingTickFinished = Sys_Global.pauseRelativeTime + Const.raycastTick;
		EnableAutomapOverlay();
		inSight = CheckIfPlayerInSight();
		if (enemy != null && HasHealth(healthManager)) {
			// Check if enemy health drops to 0
			if (enemyHM == null) enemyHM = Utils.GetMainHealthManager(enemy);
			if (enemyHM != null) {
				if (!HasHealth(enemyHM)) {
					Debug.Log("Enemy died, forgetting and wandering");
					if (IsCyberNPC()) {
						currentState = AIState.Idle;
					} else {
						// Enemy is dead, let's wander around aimlessly now
						wandering = true;
						wanderFinished = Sys_Global.pauseRelativeTime + UnityEngine.Random.Range(3f,8f);
						currentState = AIState.Walk;
					}
					
					enemy = null; // Forget the enemy.
					Debug.Log("enemy forgotten");
					enemyHM = null;
					posCheckFinished = Sys_Global.pauseRelativeTime;
					lastPosition = transform.position;
				}
			}

			// Enemy still has health
			if (enemy != null) {
				enemyInFrontChecks(enemy);
				rangeToEnemy = (enemy.transform.position
							- sightPoint.transform.position).sqrMagnitude;
			}
		} else {
			infront = false;
			inProjFOV = false;
			rangeToEnemy = Const.a.sightRange[index]
						   * Const.a.sightRange[index];
		}
	}

	void FixedUpdate(uint16_t i) {
		if (Sys_Global.gamePaused || Sys_Global.menuActive) return;

        Think();
        if (healthManager.linkedOverlay != null) {
            if (!IsCyberNPC()
                //&& healthManager.health > 0 // Only health, not cyber.
                && Inventory.a.hasHardware[1]
                && Inventory.a.NavUnitVersion() > 1) {

                healthManager.UpdateLinkedOverlay();
            } else {
                Utils.DisableImage(healthManager.linkedOverlay);
            }
        }


        // Rotation and Special movement that must be done every FixedUpdate
        if (currentState != AIState.Dead) {
            if (currentState != AIState.Idle) {
				if (actAsTurret && enemy != null) {
					currentDestination = enemy.transform.position;
					currentDestination.y = enemy.transform.position.y + 0.24f;
				}

				if (IsCyberNPC() && enemy != null) {
					currentDestination = enemy.transform.position;
				}

                idealTransformForward = currentDestination
										- sightPoint.transform.position;

                if (!IsCyberNPC()) idealTransformForward.y = 0;
				idealTransformForward = idealTransformForward.normalized;
				if (idealTransformForward.sqrMagnitude > Mathf.Epsilon
					|| IsCyberNPC()) {

					AI_Face(currentDestination);
				}
            }
        }
	}

	void Think() {
		if (!DynamicCulling.a.cullEnabled) withinPVS = true;
		if (dyingSetup && deathBurstFinished < Sys_Global.pauseRelativeTime
			&& !deathBurstDone) { // Activate any death effects
			
			if (deathBurst != null) deathBurst.SetActive(true);
			deathBurstDone = true;
		}

		if (!HasHealth(healthManager)) {
			// If we haven't gone into dying and we aren't dead, do dying.
			if (!ai_dying && !ai_dead) {
				ai_dying = true; // No going back!
				currentState = AIState.Dying; // Start to collapse in a heap,
											  // melt, explode, etc.
				
			} else if (ai_dead && currentState != AIState.Dead) {
				currentState = AIState.Dead;
			} else if (ai_dying && currentState != AIState.Dying) {
				currentState = AIState.Dying;
			}
		}

		switch (currentState) {
			case AIState.Idle: 	  Idle(); 	 break;
			case AIState.Walk:	  Walk(); 	 break;
			case AIState.Run: 	  Run(); 	 break;
			case AIState.Attack1: Attack1(); break;
			case AIState.Attack2: Attack2(); break;
			case AIState.Attack3: Attack3(); break;
			case AIState.Pain: 	  Pain();	 break;
			case AIState.Dying:   Dying(); 	 break;
			case AIState.Dead: 	  Dead(); 	 break;
			default: 			  Idle(); 	 break;
		}

		if (currentState == AIState.Dead || currentState == AIState.Dying) {
			return; // Don't do any checks, we're dead.
		}

		if (asleep) {
			Utils.Activate(sleepingCables);
			return; // Don't check for an enemy, we are sleeping! shh!!
		}

		if (npcTable[index].moveType[index] == AIMoveType.Fly
			&& tranquilizeFinished < Sys_Global.pauseRelativeTime) {
			FlierMoveToHoverHeight();
		}
	}

	void FlierMoveToHoverHeight() {
		if (Const.a.runSpeed[index] <= 0) return;

		float distUp = 0;
		float distDn = 0;
		Vector3 floorPoint = new Vector3();
		floorPoint = Const.a.vectorZero;
		if (enemy != null) {
		    idealPos = transform.position; // Where it's at
		    idealPos.y = enemy.transform.position.y + 0.24f; // Player eye height.
		} else if (!Const.a.RaycastBudgetExceeded()) {
			if (Physics.Raycast(sightPoint.transform.position,
								sightPoint.transform.up * -1,out tempHit,
								Const.a.sightRange[index],
								Const.a.layerMaskNPCSight)) {
				Const.a.numberOfRaycastsThisFrame++;
				distDn = Vector3.Distance(sightPoint.transform.position, tempHit.point);
				floorPoint = tempHit.point;
			}

			if (Physics.Raycast(sightPoint.transform.position, sightPoint.transform.up,out tempHit, Const.a.sightRange[index], Const.a.layerMaskNPCSight)) {
				Const.a.numberOfRaycastsThisFrame++;
				distUp = Vector3.Distance(sightPoint.transform.position, tempHit.point);
			}

			float distT = (distUp + distDn);
			float yHeight = Const.a.flightHeight[index];
			if (Const.a.flightHeightIsPercentage[index]) {
				yHeight *= distT;
			}

			idealPos = floorPoint + new Vector3(0,yHeight, 0);
		}

		float dist = Mathf.Abs(idealPos.y - transform.position.y);
		if (dist < 0.16f) return; // Close enuff

		float spd = Const.a.runSpeed[index] * Time.deltaTime;
		transform.position = Vector3.MoveTowards(transform.position,idealPos,spd);
	}

	public bool CheckPain() {
		if (IsCyberNPC()) return false;
		if (asleep) return false;
		if (npcTable[index].timeBetweenPain <= 0) return false;

		if (goIntoPain && timeTillPainFinished < Sys_Global.pauseRelativeTime) {
			currentState = AIState.Pain;
			if (attacker != null) {
				if (timeTillEnemyChangeFinished < Sys_Global.pauseRelativeTime) {
					timeTillEnemyChangeFinished = Sys_Global.pauseRelativeTime
						+ Const.a.timeToChangeEnemy[index];
						
					AIController attackerAIC = attacker.GetComponent<AIController>();
					if (attackerAIC != null && attacker.layer != 12) { // Attacker is an NPC and not the player.
						NPCType myType = Const.a.type[index];
						NPCType attackerType = Const.a.type[attackerAIC.index];
						bool canInfight = false;
						
						// Check infighting rules
						if (myType == NPCType_Robot && enemy != null) {
							// Robots only change to player, keep existing enemy otherwise
							canInfight = false;
						} else if (      (myType == NPCType_Cyborg ||       myType == NPCType_Supercyborg ||       myType == NPCType_Robot) && 
								   (attackerType == NPCType_Cyborg || attackerType == NPCType_Supercyborg || attackerType == NPCType_Robot)) {
							canInfight = false; // Cyborgs don't fight Cyborgs or Robots
						} else if (   (      myType == NPCType_Mutant ||       myType == NPCType_Supermutant)
								   && (attackerType == NPCType_Mutant || attackerType == NPCType_Supermutant)) {
							canInfight = (attackerAIC.index != index); // Mutants can infight if different index
						} else {
							canInfight = (attackerAIC.index != index); // All other combinations can infight
						}

						if (canInfight) enemy = attacker;
					} else {
						enemy = attacker; // Attacker is the player, set enemy to player.
					}
					posCheckFinished = Sys_Global.pauseRelativeTime + positionCheckDelay;
					wandering = false;
					wanderFinished = Sys_Global.pauseRelativeTime;
					lastPosition = transform.position;
					if (enemy != null) {
						enemyHM = Utils.GetMainHealthManager(enemy);
						lastKnownEnemyPos = enemy.transform.position;
						currentDestination = enemy.transform.position;
					}
				}
			}
			goIntoPain = false;
			timeTillPainFinished = Sys_Global.pauseRelativeTime
								   + Const.a.timeToPain[index];
			return true;
		}
		return false;
	}

	void Idle() {
		if (enemy != null && HasHealth(healthManager)) {
			currentState = AIState.Run;
			return;
		}

		if (idleTime < Sys_Global.pauseRelativeTime) {
			if (UnityEngine.Random.Range(0,1f) < 0.5f) { // 50% Chance of idle.
				SFXIndex = Const.a.sfxIdle[index];
				Utils.PlayOneShotSavable(SFX,SFXIndex);
			}
			idleTime = Sys_Global.pauseRelativeTime
					   + Random.Range(Const.a.timeIdleSFXMin[index],
									  Const.a.timeIdleSFXMax[index]);
		}

		if (asleep) {
			rbody.constraints =   RigidbodyConstraints.FreezePositionX
								| RigidbodyConstraints.FreezePositionY
								| RigidbodyConstraints.FreezePositionZ
								| RigidbodyConstraints.FreezeRotationX
								| RigidbodyConstraints.FreezeRotationY
								| RigidbodyConstraints.FreezeRotationZ;
		}

		CheckPain(); // Go into pain if just hurt, data sent by HealthManager.
	}

	Vector3 GetWanderPoint() {
		float newX = transform.position.x + UnityEngine.Random.Range(-79f,79f);
		float newZ = transform.position.z + UnityEngine.Random.Range(-79f,79f);
		float newY = 0f;
		if (IsCyberNPC()) newY = transform.position.y + UnityEngine.Random.Range(-79f,79f);
		return new Vector3(newX,newY,newZ);
	}

	void Walk() {
        if (CheckPain()) return; // Go into pain if just hurt
		if (asleep) return;
        if (inSight || enemy != null) { currentState = AIState.Run; return; }
        if (actAsTurret) { currentState = AIState.Idle; return; }
        if (npcTable[index].moveType[index] == AIMoveType.None) return;
		if (tranquilizeFinished >= Sys_Global.pauseRelativeTime) return;
		if (!withinPVS && DynamicCulling.a.cullEnabled) return;
		
		float dist = Vector3.Distance(sightPoint.transform.position,currentDestination);
		if (wandering) {
			if (wanderFinished < Sys_Global.pauseRelativeTime || (dist < (stopDistance * 0.5f))) {
				wanderFinished = Sys_Global.pauseRelativeTime + UnityEngine.Random.Range(3f,8f);
				currentDestination = GetWanderPoint();
			}
		}

		// Destination still far away and turned to within angle to move, move
		if (dist > stopDistance) {
			if (WithinAngleToTarget()) {
				if (Const.a.hopsOnMove[index]) {
					// Move it move it.
					float playbackTime = 1f;
					if (hopAnimator != null) {
						AnimatorStateInfo asi =
							hopAnimator.GetCurrentAnimatorStateInfo(0);

						playbackTime = asi.normalizedTime;
					}

					if (playbackTime > 0.1395f) { // Hopper only!
						if (!hopDone) {
							hopDone = true;
							Vector3 force = sightPoint.transform.forward * 500f;
							rbody.WakeUp();
							rbody.AddForce(force);
							rbody.AddForce(Vector3.up * 5f);
						}
					} else {
						hopDone = false;
					}
				} else {
					tempVec = (sightPoint.transform.forward
							   * Const.a.walkSpeed[index]);

					if (Const.a.numberOfRaycastsThisFrame <= Const.maxRaycastsPerFrame
						&& npcTable[index].moveType[index] != AIMoveType.Fly) {

						Vector3 checkPos = sightPoint.transform.position
										   + (tempVec.normalized * 0.48f);

						int mk = Const.a.layerMaskNPCCollision;
						if (!Physics.Raycast(checkPos,Vector3.down,2.56f,mk)) {
							Const.a.numberOfRaycastsThisFrame++;
							tempVec.x = 0f;
							tempVec.z = 0f;
						}
					}

					tempVec.y = rbody.velocity.y; // Carry across gravity.
					rbody.velocity = tempVec;
				}
			}
			return; // Still moving to same target destination.
		}

		if (walkWaypoints.Length < 1) {
			if (!wandering) {
				currentState = AIState.Idle; // No wandering, just go to Idle.
			}
			return; // No waypoint to visit, wait for wandering timer.
		}

		// Need new spot to move to
		if (visitWaypointsRandomly) {
			// Max is exclusive for the integer overload, no need to do
			// (walkWaypoints.Length - 1).
			currentWaypoint = Random.Range(0, walkWaypoints.Length);
		} else {
			currentWaypoint++;
		}

		if (currentWaypoint < 0) currentWaypoint = 0;
		if ((currentWaypoint >= (walkWaypoints.Length - 1))) {
			currentWaypoint = 0; // Wrap around.

			// Stop when reached end of list; out of waypoints.
			if (dontLoopWaypoints) {
				currentState = AIState.Idle;
				return;
			}
		}

		if (walkWaypoints.Length < 1) return;
		if (walkWaypoints[currentWaypoint] == null) return; // No gaps allowed.

		currentDestination = walkWaypoints[currentWaypoint].transform.position;
	}

	bool CanAttack1(float dist) {
    	if (rangeToEnemy >= dist) return false;
		if (npcTable[index].attackType[index] == AttackType.None) return false;
		if (IsCyberNPC()) return true;
		if (!infront) return false;
		if (randomWaitForNextAttack1Finished >= Sys_Global.pauseRelativeTime) {
			return false;
		}

		return true;
	}

	bool CanAttack2(float dist) {
    	if (rangeToEnemy >= dist) return false;
		if (npcTable[index].attackType2[index] == AttackType.None) return false;
		if (IsCyberNPC()) return true;
		if (!infront) return false;
		if (!inProjFOV) return false;
		if (randomWaitForNextAttack2Finished >= Sys_Global.pauseRelativeTime) {
			return false;
		}

		return true;
	}

	bool CanAttack3(float dist) {
    	if (rangeToEnemy >= dist) return false;
		if (rangeToEnemy < 7f && npcTable[index].attackType3[index] == AttackType.ProjectileLaunched
			&& (Const.a.projectile3Prefab[index] == 370 || Const.a.projectile3Prefab[index] == 372
			    || Const.a.projectile3Prefab[index] == 387 || Const.a.projectile3Prefab[index] == 404)) return false;
			
		if (npcTable[index].attackType3[index] == AttackType.None) return false;
		if (IsCyberNPC()) return true;
		if (!infront) return false;
		if (!inProjFOV) return false;
		if (randomWaitForNextAttack3Finished >= Sys_Global.pauseRelativeTime) {
			return false;
		}

		return true;
	}

	void BrakingMovement() {
		if (index == 1 || (index >= 3 && index <= 9)
			|| (index >= 11 && index <= 13) || index == 17 || index == 23) {

			rbody.velocity *= 0.15f; // Stop scoot to shoot.
		}
	}

	void StartAttack1() {
		BrakingMovement();
		attackFinished = Sys_Global.pauseRelativeTime + npcTable[index].timeBetweenAttack1 + npcTable[index].timeToActualAttack1;
		gracePeriodFinished = Sys_Global.pauseRelativeTime + npcTable[index].timeToActualAttack1;
		currentState = AIState.Attack1;
		if (npcTable[index].preactivateMeleeColliders) instances[i].touchingHurts = true;
	}

	void StartAttack2() {
		BrakingMovement();
		attackFinished = Sys_Global.pauseRelativeTime + npcTable[index].timeBetweenAttack2 + npcTable[index].timeToActualAttack2;
		gracePeriodFinished = Sys_Global.pauseRelativeTime + Const.a.timeToActualAttack2[index];
		currentState = AIState.Attack2;
	}

	void StartAttack3() {
		BrakingMovement();
		attackFinished = Sys_Global.pauseRelativeTime + npcTable[index].timeBetweenAttack3 + npcTable[index].timeToActualAttack3;
		gracePeriodFinished = Sys_Global.pauseRelativeTime + Const.a.timeToActualAttack3[index];
		currentState = AIState.Attack3;
	}

	void HopMove() {
		if (actAsTurret) return;

		// Move it move it.
		float playbackTime = 0.0f;
		if (hopAnimator != null) {
			AnimatorStateInfo asi = hopAnimator.GetCurrentAnimatorStateInfo(0);
			playbackTime = asi.normalizedTime;
		}

		if (playbackTime > 0.1395f) {
			if (!hopDone) {
				hopDone = true;
				rbody.AddForce(sightPoint.transform.forward * 500f); // Huh!
			}
		} else {
			hopDone = false;
		}
	}

	void RunMove() {
		if (actAsTurret) return;

		tempVec = sightPoint.transform.forward * Const.a.runSpeed[index];
		if (rbody.useGravity) tempVec.y = rbody.velocity.y; // Keep gravity.
		rbody.velocity = tempVec;
	}
	
	Vector3 GetAStarPoint() {
		if (DynamicCulling.a == null) return GetWanderPoint();
		
		Vector2Int currentCell = DynamicCulling.a.PosToCellCoords(transform.position);
		if (!DynamicCulling.a.XYPairInBounds(currentCell.x,currentCell.y)) return GetWanderPoint();
			
		bool clearNorth = false;
		bool clearSouth = false;
		bool clearEast = false;
		bool clearWest = false;
		Vector3 northPoint = transform.position + new Vector3(0f,0f,2.56f);
		Vector3 southPoint = transform.position + new Vector3(0f,0f,-2.56f);
		Vector3 eastPoint = transform.position + new Vector3(2.56f,0f,0f);
		Vector3 westPoint = transform.position + new Vector3(-2.56f,0f,0f);
		List<Vector3> availablePositions = new List<Vector3>();
		if (DynamicCulling.a.XYPairInBounds(currentCell.x,currentCell.y + 1)) {
			clearNorth = (DynamicCulling.a.gridCells[currentCell.x,currentCell.y + 1].open && !DynamicCulling.a.gridCells[currentCell.x,currentCell.y].closedNorth);
			if (clearNorth) availablePositions.Add(northPoint);
		}
		
		if (DynamicCulling.a.XYPairInBounds(currentCell.x,currentCell.y - 1)) {		
			clearSouth = (DynamicCulling.a.gridCells[currentCell.x,currentCell.y - 1].open && !DynamicCulling.a.gridCells[currentCell.x,currentCell.y].closedSouth);
			if (clearSouth) availablePositions.Add(southPoint);
		}
		
		if (DynamicCulling.a.XYPairInBounds(currentCell.x + 1,currentCell.y)) {		
			clearEast = (DynamicCulling.a.gridCells[currentCell.x + 1,currentCell.y].open && !DynamicCulling.a.gridCells[currentCell.x,currentCell.y].closedEast);
			if (clearEast) availablePositions.Add(eastPoint);
		}
		
		if (DynamicCulling.a.XYPairInBounds(currentCell.x - 1,currentCell.y)) {		
			clearWest = (DynamicCulling.a.gridCells[currentCell.x - 1,currentCell.y].open && !DynamicCulling.a.gridCells[currentCell.x,currentCell.y].closedWest);
			if (clearWest) availablePositions.Add(westPoint);
		}

		// Randomly select point but only from available choices
		int nearest = 0;		
		for (int i=0;i<availablePositions.Count;i++) {
			if (Vector3.Distance(enemy.transform.position,availablePositions[i]) < Vector3.Distance(enemy.transform.position,availablePositions[nearest])) nearest = i;
		}
		
		return availablePositions[nearest];
	}
	
	Vector3 GetSearchPoint(bool hunting) {
		//if (hunting) return lastKnownEnemyPos; // When we can't see the enemy, go to the last spot we saw them.
		
		switch(Const.a.type[index]) {
			case NPCType_Mutant: return GetWanderPoint();
			case NPCType_Supermutant: return GetWanderPoint();
			case NPCType_Robot: return GetAStarPoint();
			case NPCType_Cyborg: return GetAStarPoint();
			case NPCType_Supercyborg: return GetAStarPoint();
			case NPCType_MutantCyborg: return GetAStarPoint();
		}
		
		return GetWanderPoint();
	}
	
	private void SetHuntFinished() {
// 		Debug.Log("Set hunt finished time");
		huntFinished = Sys_Global.pauseRelativeTime;
		int diff = Const.a.difficultyCombat;
		if (IsCyberNPC()) diff = Const.a.difficultyCyber;
		if (diff <= 1) { // More forgetful on easy.
			huntFinished += Mathf.Max((Const.a.huntTime[index] * 0.75f),60f);
		} else if (diff >= 3) { // Good memory on hard.
			huntFinished += Mathf.Max((Const.a.huntTime[index] * 2.00f),60f); 
		} else {
		    huntFinished += Mathf.Max(Const.a.huntTime[index],60f);
		}
	}

	void Run() {
		if (CheckPain()) return; // Go into pain just hurt
		if (asleep) return;
		if (enemy == null) { currentState = AIState.Idle; return; }

		if (tranquilizeFinished >= Sys_Global.pauseRelativeTime
			&& !IsCyberNPC()) {
			return;
		}

		if (posCheckFinished <= Sys_Global.pauseRelativeTime && !IsCyberNPC()) {
			posCheckFinished = Sys_Global.pauseRelativeTime + positionCheckDelay;
			float distToEnem = Vector3.Distance(sightPoint.transform.position,enemy.transform.position);
			distToLastPos = Vector3.Distance(transform.position,lastPosition);
			lastPosition = transform.position;
			if (distToLastPos < 0.48f && distToEnem > stopDistance && !wandering) {
				wanderFinished = Sys_Global.pauseRelativeTime + searchTime;
				wandering = true;
				currentDestination = GetSearchPoint(false);
			} else {
				wandering = false;
			}
		}

        if (!inSight) {
            if (huntFinished > Sys_Global.pauseRelativeTime) {
                Hunt();
            } else {
				Debug.Log("enemy hunt ended");
                enemy = null;
				enemyHM = null;
				wandering = true; // Sometimes look like we are still searching
				wanderFinished = Sys_Global.pauseRelativeTime + 1f;
                currentState = AIState.Walk;
            }
            return;
        }
        
		if (enemy != null && !wandering) {
			targettingPosition = enemy.transform.position + targetOffset;
			currentDestination = targettingPosition;
			lastKnownEnemyPos = targettingPosition;
		}

		shotFired = false;
		SetHuntFinished();
		near = npcTable[index].range[index]  * npcTable[index].range[index];
		mid  = npcTable[index].range2[index] * npcTable[index].range2[index];
		far  = npcTable[index].range3[index] * npcTable[index].range3[index];
        if (CanAttack1(near)) {
			StartAttack1();
			return;
        } else if (CanAttack2(mid)) {
			StartAttack2();
			return;
		} else if (CanAttack3(far)) {
			StartAttack3();
			return;
		}

		// Enemy still far away and turned to within angle, then move
		if ((npcTable[index].moveType[index] != AIMoveType.None)
			&& (rangeToEnemy > (stopDistance * stopDistance))) {
			if (WithinAngleToTarget()) {
				if (Const.a.hopsOnMove[index]) HopMove();
				else                                 RunMove(); // <<<<<RUN
			} else {
				if (Const.a.difficultyCombat >= 2) {
					if (Random.Range(0f,1f) < 0.5f) AI_Face(currentDestination);
				}
			}
			
		}
	}

    void Hunt() {
		if (IsCyberNPC()) {
			currentDestination = enemy.transform.position; // See through walls
		} else {
			// UPDATE: A* Pathfinding with world grid.
			currentDestination = GetSearchPoint(true); //enemy.transform.position;//lastKnownEnemyPos;
		}

		// Destination is still far enough away and within angle, then move.
		if (npcTable[index].moveType[index] == AIMoveType.None) return;
		if (actAsTurret) return; // Enemy marked to not move (e.g. on pillar).
		if (Const.a.runSpeed[index] <= 0) return; // Enemy doesn't move.

		Transform eyeTr = sightPoint.transform;
		Vector3 eyePos = eyeTr.position;
		float sqrDist = (eyePos - currentDestination).sqrMagnitude;
		if (sqrDist <= (stopDistance * stopDistance)) return; // At stop point.
		if (!WithinAngleToTarget()) return;

		rbody.velocity = (eyeTr.forward * Const.a.runSpeed[index]);
    }

	// Commonized function to remove previous boilerplate code from all 3
	// attack functions below.  Applies movement towards the enemy while
	// attacking, assumes we were already facing enemy within attack angle.
	void ApplyAttackMovement(float speedToApply) {
		if (enemy == null) return;

		if (actAsTurret) {
			currentDestination = sightPoint.transform.position;
			return;
		}

		if (speedToApply <= 0) return;
		if (tranquilizeFinished >= Sys_Global.pauseRelativeTime) return;

		// Attack3 used targettingPosition but it is so rare I decided to use
		// the known working method from Attack1 and Attack2.
        currentDestination = enemy.transform.position;
		Vector3 eyePos = sightPoint.transform.position;
		float sqrDist = (eyePos - currentDestination).sqrMagnitude;
		if (sqrDist <= (stopDistance * stopDistance)) return; // At stop point.
		if (!WithinAngleToTarget()) return; // Still turning to face.

		rbody.AddForce(transform.forward * speedToApply);
	}

	// attackNum corresponds to attack used so right lookup tables can be used.
	// attackNum of 1 = Attack1, 2 = Attack2, 3 = Attack3
	void Transition_AttackToRun(int attackNum) {
		instances[i].touchingHurts = false;
		goIntoPain = false; // Prevent doing pain immediately after attack.
		currentState = AIState.Run; // Done with attack.
		if (attackNum < 1 || attackNum > 3) attackNum = 1;
		float now = Sys_Global.pauseRelativeTime;
		switch (attackNum) {
			case 1: // Attack1
				float perc1Chance = Const.a.timeAttack1WaitChance[index];
				if (Random.Range(0f,1f) < perc1Chance) {
					float min1 = Const.a.timeAttack1WaitMin[index];
					float max1 = Const.a.timeAttack1WaitMax[index];
					float wait1 = Random.Range(min1,max1);
					randomWaitForNextAttack1Finished = now + wait1;
				} else {
					randomWaitForNextAttack1Finished = now;
				}
				break;
			case 2: // Attack2
				float perc2Chance = Const.a.timeAttack2WaitChance[index];
				if (Random.Range(0f,1f) < perc2Chance) {
					float min2 = Const.a.timeAttack2WaitMin[index];
					float max2 = Const.a.timeAttack2WaitMax[index];
					float wait2 = Random.Range(min2,max2);
					randomWaitForNextAttack2Finished = now + wait2;
				} else {
					randomWaitForNextAttack2Finished = now;
				}
				break;
			case 3: // Attack3
				float perc3Chance = Const.a.timeAttack3WaitChance[index];
				if (Random.Range(0f,1f) < perc3Chance) {
					float min3 = Const.a.timeAttack3WaitMin[index];
					float max3 = Const.a.timeAttack3WaitMax[index];
					float wait3 = Random.Range(min3,max3);
					randomWaitForNextAttack3Finished = now + wait3;
				} else {
					randomWaitForNextAttack3Finished = now;
				}
				break;
		}
	}

    bool WithinAngleToTarget () {
		if (IsCyberNPC()) return true;
		if (idealTransformForward.sqrMagnitude <= Mathf.Epsilon) return false;

		Quaternion lookRot = Quaternion.LookRotation(idealTransformForward);
		float fovMov = Const.a.fovStartMovement[index];
		float ang = Quaternion.Angle(transform.rotation,lookRot);
		if (ang < fovMov) return true;
		if (ang < (fovMov * 1.5f)) {
			if (Random.Range(0f,1f) < 0.5f) return true;
		}
        return false;
    }

	Vector3 GetAttackStartPoint(int attackNum) {
		if (attackNum < 1 || attackNum > 3) attackNum = 1;
		Vector3 startPos = sightPoint.transform.position;
		switch (attackNum) {
			case 2:
				if (gunPoint != null) {
					startPos = gunPoint.transform.position;
				} else if (gunPoint2 != null) {
					startPos = gunPoint2.transform.position;
				}
				break;
			case 3:
				if (gunPoint2 != null) {
					startPos = gunPoint2.transform.position;
				} else if (gunPoint != null) {
					startPos = gunPoint.transform.position;
				}
				break;
		}

		return startPos;
	}

	// Returns unit vector pointing from starting point of attack towards enemy.
	Vector3 GetDirectionRayToEnemy(Vector3 targPos, int attackNum) {
		if (attackNum < 1 || attackNum > 3) attackNum = 1;
		switch (attackNum) {
			case 1:
				return sightPoint.transform.forward;
			case 2:
				return (targPos - GetAttackStartPoint(attackNum)).normalized;
			case 3:
				return (targPos - GetAttackStartPoint(attackNum)).normalized;
		}
		return Const.a.vectorZero;
	}

	float GetRangeForAttack(int attackNum) {
		if (attackNum < 1 || attackNum > 3) attackNum = 1;
		float range = npcTable[index].range[index];
		switch (attackNum) {
			case 1:
				range = npcTable[index].range[index];
				break;
			case 2:
				range = npcTable[index].range2[index];
				break;
			case 3:
				range = npcTable[index].range3[index];
				break;
		}
		return range;
	}

    void CreateStandardImpactEffects() {
        // Determine blood type of hit target and spawn corresponding blood
		// particle effect from the Const.Pool
		float offset = 0f;
		GameObject impact = null;
        if (tempHM != null) {
			offset = 0.08f;
            impact = Const.a.GetImpactType(tempHM); // Returns blood type.
        } else { // Didn't hit object with a HealthManager script, use sparks.
			impact = Const.a.GetObjectFromPool(PoolType.SparksSmall); 
		}

		if (impact == null) return;

		impact.transform.position = tempHit.point + (tempHit.normal * offset);
		impact.transform.rotation = Quaternion.FromToRotation(Vector3.up,
															  tempHit.normal);
		impact.SetActive(true);
    }

	// Activates a GameObject that has automatically playing particle effects,
	// lights, etc.  The muzzle bursts are all set up to deactivate on their
	// own; no need to check them later.  attackNum corresponds to the attack
	// used so correct lookup tables can be used.  attackNum of 1 = Attack1,
	// 2 = Attack2, 3 = Attack3
	void MuzzleBurst(int attackNum) {
		if (attackNum < 1 || attackNum > 3) attackNum = 1;
		if (index == 18) Utils.Activate(muzzleBurst); // Activate this one too.
		switch (attackNum) { // No muzzle burst for Attack1 melee.
			case 2:
				Utils.Activate(muzzleBurst);
				break;
			case 3:
				Utils.Activate(muzzleBurst2);
				break;
		}
	}

	// Does the raycast and sets tempHit for the hit data and tempHM for the
	// hit object's HealthManager.  Returns true if it actually hit something.
    bool DidRayHit(int attackNum) {
		tempHM = null;
		if (attackNum < 1 || attackNum > 3) attackNum = 1;
		tempVec = GetDirectionRayToEnemy(targettingPosition,attackNum);
		Vector3 pos = GetAttackStartPoint(attackNum);
		float range = GetRangeForAttack(attackNum);
		int mask = Const.a.layerMaskNPCAttack;
		if (!Physics.Raycast(pos,tempVec,out tempHit,range,mask)) return false;

		Const.a.numberOfRaycastsThisFrame++;
		tempHM = Utils.GetMainHealthManager(tempHit);
		return true;
    }

	void MakeLaserEffect(int attackNum) {
		bool hasLaser = false;
		switch(attackNum) {
			case 1: hasLaser = Const.a.hasLaserOnAttack1[index]; break;
			case 2: hasLaser = Const.a.hasLaserOnAttack2[index]; break;
			case 3: hasLaser = Const.a.hasLaserOnAttack3[index]; break;
		}

		if (!hasLaser) return;

		GameObject laz = Instantiate(Const.a.GetPrefab(408),transform.position,
									 Const.a.quaternionIdentity) as GameObject;

		if (laz == null) return; // No laser!

		GameObject dCont = LevelManager.a.GetCurrentDynamicContainer();
		laz.transform.SetParent(dCont.transform,true);
		LaserDrawing ldraw = laz.GetComponent<LaserDrawing>();
		ldraw.startPoint = sightPoint.transform.position;
		ldraw.endPoint = tempHit.point;
		Utils.Activate(laz);
	}

	void PositionTargettingLaser() {
		if (laserLightning == null) return;
		if (!laserLightning.enabled) return;

		Vector3[] pts = new Vector3[] {
			sightPoint.transform.position,
			enemy.transform.position
		};

		laserLightning.SetPositions(pts);
	}

	void MakeTargettingLaser() {
		if (index != 8) return; // Cyborg Elite only.
		if (laserLightning == null) return;

		laserLightning.startWidth = 0.1f;
		laserLightning.endWidth = 0.15f;
        laserLightning.enabled = true;
		PositionTargettingLaser();
	}

	// Used for attack type of AttackType.Projectile.
	// Does a raycast and then applies attack instantly.
	// Also turns on laser effect if used.
	// attackNum corresponds to attack used so right lookup tables can be used.
	// attackNum of 1 = Attack1, 2 = Attack2, 3 = Attack3
	// Attack1 is typically Melee, Attack2 is typically a gun from gunPoint,
	// Attack2 could be a gun or grenade from gunPoint2 (2 as in 2nd gun
	// attack, NOT Attack2).
	void ProjectileRaycast(int attackNum) {
		if (attackNum < 1 || attackNum > 3) attackNum = 1;
		MuzzleBurst(attackNum);
		if (DidRayHit(attackNum)) {
			MakeLaserEffect(attackNum);
			if (attackNum == 3) MakeTargettingLaser();
			if (tempHM != null) {
				// SetNPCData sets: owner, damage, penetration, offense
				damageData = DamageData.SetNPCData(index,attackNum,gameObject);

				// Using tempHit.transform instead of
				// tempHit.collider.transform to get overall parent of another
				// NPC or of the player.
				damageData.other = tempHit.transform.gameObject;
				if (tempHit.transform.gameObject.CompareTag("NPC")) {
					damageData.isOtherNPC = true;
				} else {
					damageData.isOtherNPC = false;
				}
				damageData.hit = tempHit;
				damageData.attacknormal = tempVec;
				damageData.attackType = AttackType.Projectile;

				// GetDamageTakeAmount expects damageData to already have the
				// following set: damage, offense, penetration, attackType,
				//   berserkActive, isOtherNPC, armorvalue, defense
				damageData.impactVelocity = damageData.damage;
				if (tempHM.isPlayer) damageData.impactVelocity *= 0.5f;
				damageData.damage = DamageData.GetDamageTakeAmount(damageData);
				tempHM.TakeDamage(damageData);
			}

			CreateStandardImpactEffects();
		}
	}

	// Used for attack type of AttackType.ProjectileLaunched.
	// Hurls a beachball-like object that will apply damage later if it hits
	// something that can be hurt.
	//   attackNum corresponds to the correct lookup table
	//   attackNum of 1 = Attack1, 2 = Attack2, 3 = Attack3
	void ProjectileLaunched(int attackNum) {
		if (attackNum < 1 || attackNum > 3) attackNum = 3;
		MuzzleBurst(attackNum);
		tempVec = GetDirectionRayToEnemy(targettingPosition, attackNum);
		Vector3 startPos = GetAttackStartPoint(attackNum);

		// SetNPCData sets: owner, damage, penetration, offense
		damageData = DamageData.SetNPCData(index,attackNum,gameObject);
		damageData.attacknormal = tempVec;
		damageData.attackType = AttackType.ProjectileLaunched;
		// Can't call DamageData.GetDamageTakeAmount here since we haven't hit
		// anyone yet so we don't know if isOtherNPC is true or not, called in
		// ProjectileEffectImpact.

		// Create and hurl a beachball-like object.  On the developer
		// commentary they said that the projectiles act like a beachball for
		// collisions with enemies, but act like a baseball for walls/floor to
		// prevent hitting corners.  Calling it a beachball for fun.
		GameObject beachball = null;
		float launchSpeed = 10f;
		int masterIndex = 370; // Default frag.
		switch (attackNum) {
			case 1:
				masterIndex = Const.a.projectile1Prefab[index];
				launchSpeed = Const.a.projectileSpeedAttack1[index];
				break;
			case 2:
				masterIndex = Const.a.projectile2Prefab[index];
				launchSpeed = Const.a.projectileSpeedAttack2[index];
				break;
			case 3:
				masterIndex = Const.a.projectile3Prefab[index];
				launchSpeed = Const.a.projectileSpeedAttack3[index];
				break;
		}

		beachball = ConsoleEmulator.SpawnDynamicObject(masterIndex,-1);
		if (beachball == null) beachball = Const.a.GetPrefab(370); // Frag
		beachball.tag = "NPC";
		beachball.layer = 24; // NPCBullet
		ProjectileEffectImpact pei = 
			beachball.GetComponent<ProjectileEffectImpact>();
		if (pei != null) {
			pei.dd = damageData;
			pei.host = gameObject;
		}
		beachball.transform.position = startPos;
		beachball.transform.forward = tempVec.normalized;
		Utils.Activate(beachball);
		GrenadeActivate ga = beachball.GetComponent<GrenadeActivate>();
		if (ga != null) ga.Activate();
		Vector3 shove = (beachball.transform.forward * launchSpeed);
		//if (IsCyberNPC()) {
		//	if (enemy != null) {
		//		Rigidbody rbodyEnemy = enemy.GetComponent<Rigidbody>();
		//		if (rbodyEnemy != null && UnityEngine.Random.Range(0f,1f) < 0.5f) {
		//			shove = shove + (rbodyEnemy.velocity * 0.2f);
		//		}
		//	}
		//}

		// Add in the enemy's velocity to the projectile (in case they are
		// riding on a moving platform or something - wait I don't have those!)
		if (!IsCyberNPC()) shove += rbody.velocity;

		// Ensure no velocity to start with.
		beachball.GetComponent<Rigidbody>().velocity = Const.a.vectorZero;
		beachball.GetComponent<Rigidbody>().AddForce(shove, ForceMode.Impulse);
	}

	// Die in a fiery explosion BOOM!
	//   attackNum of 1 = Attack1, 2 = Attack2, 3 = Attack3
	void ExplodeAttack(int attackNum) {
		if (attackNum < 1 || attackNum > 3) attackNum = 3;
		DamageData dd = DamageData.SetNPCData(index,attackNum,gameObject);
		if (dd == null) return;

		float take = DamageData.GetDamageTakeAmount(dd);
		dd.other = gameObject;
		dd.damage = take;
		Utils.ApplyImpactForceSphere(dd,sightPoint.transform.position,
									 Const.a.attack3Radius[index],1.5f);

		healthManager.TakeDamage(dd); // Self destruct.
	}

	// Attack type determines raycast or launched object.  Checked for None
	//   elsewhere.
	//
	// attackIndex for Attack 1, 2, or 3 lookup tables.
	void AIAttack(AttackType att_type, int ind) {
		if (ind < 1 || ind > 3) ind = 1; // Melee hitscan by default.

		switch (att_type) {
			case AttackType.Melee:				ProjectileRaycast(ind); break;
			case AttackType.Projectile:			ProjectileRaycast(ind); WeaponFire.a.fogFac += 1; break;
			case AttackType.ProjectileLaunched:	ProjectileLaunched(ind); WeaponFire.a.fogFac += 1; break;
		}
	}

	// Typically used for melee.
	void Attack1() {
		ApplyAttackMovement(Const.a.attack1Speed[index]);
		if (gracePeriodFinished < Sys_Global.pauseRelativeTime) {
			if (!shotFired) {
				shotFired = true;
				if (attack1SoundTime < Sys_Global.pauseRelativeTime) {
					SFXIndex = Const.a.sfxAttack1[index];
					Utils.PlayOneShotSavable(SFX,SFXIndex);
					attack1SoundTime = Sys_Global.pauseRelativeTime
						+ Const.a.timeBetweenAttack1[index];
				}
				AIAttack(npcTable[index].attackType[index],1);
			}
        }

        if (attackFinished < Sys_Global.pauseRelativeTime) {
			Transition_AttackToRun(1);  // Handle exiting this state.
		}
	}

	// Typically used for normal projectile attack
    void Attack2() {
		ApplyAttackMovement(Const.a.attack2Speed[index]);
        if (gracePeriodFinished < Sys_Global.pauseRelativeTime) {
            if (!shotFired) {
                shotFired = true; 
                if (attack2SoundTime < Sys_Global.pauseRelativeTime) {
					SFXIndex = Const.a.sfxAttack2[index];
					Utils.PlayOneShotSavable(SFX,SFXIndex);
                    attack2SoundTime = Sys_Global.pauseRelativeTime
						+ Const.a.timeBetweenAttack2[index];
                }
				AIAttack(npcTable[index].attackType2[index],2);
            }
        }

        if (attackFinished < Sys_Global.pauseRelativeTime) {
			Transition_AttackToRun(2); // Handle exiting this state.
		}
	}

	// Typically used for secondary projectile or grenade attack
	void Attack3() {
		if (Const.a.explodeOnAttack3[index]) {
			WeaponFire.a.fogFac += 5;
			ExplodeAttack(3);
			return;  // No time check, this is only done once without delay.
					 // We are dead now so exit on out.
		}

		ApplyAttackMovement(Const.a.attack3Speed[index]);
        if (gracePeriodFinished < Sys_Global.pauseRelativeTime) {
            if (!shotFired) {
                shotFired = true;
				if (attack3SoundTime < Sys_Global.pauseRelativeTime) {
					SFXIndex = Const.a.sfxAttack3[index];
					Utils.PlayOneShotSavable(SFX,SFXIndex);
					attack3SoundTime = Sys_Global.pauseRelativeTime
						+ Const.a.timeBetweenAttack3[index];
				}
				AIAttack(npcTable[index].attackType3[index],3);
            }
        }

		PositionTargettingLaser();

        if (attackFinished < Sys_Global.pauseRelativeTime) {
			Transition_AttackToRun(3); // Handle exiting this state.
		}
	}

	void Pain() {
		if (timeTillPainFinished < Sys_Global.pauseRelativeTime) {
			currentState = AIState.Run; // Go into run after we get hurt
			goIntoPain = false;
			timeTillPainFinished = Sys_Global.pauseRelativeTime
				+ Const.a.timeBetweenPain[index];
		}
	}

	void DyingSetup() {
// 		Debug.Log("NPC " + gameObject.name + " start of dying setup");
		enemy = null; // Reset for loading from saves

		if (Const.a.deathBurstTimer[index] > 0) {
			deathBurstFinished = Sys_Global.pauseRelativeTime
				+ Const.a.deathBurstTimer[index];
		} else {
			if (!deathBurstDone) {
				Utils.Activate(deathBurst); // Activate death effects
				deathBurstDone = true;
			}
		}

		instances[i].touchingHurts = false;
		if (healthManager != null) {
			if (!healthManager.actAsCorpseOnly && !healthManager.teleportOnDeath) {
				Utils.Deactivate(healthManager.linkedOverlay.gameObject);
				SFXIndex = Const.a.sfxDeath[index];
				Utils.PlayOneShotSavable(SFX,SFXIndex);
			}

			if (npcTable[index].moveType[index] == AIMoveType.Fly
				&& (!healthManager.gibOnDeath || index == 2)) { // Avian Mutant
				if (healthManager.gibOnDeath) rbody.useGravity = false;
				else rbody.useGravity = true; // Avian Mutant and Zero-G Mutant
			} else {
				if (healthManager.gibOnDeath) {
					rbody.useGravity = false;
				} else {
					rbody.useGravity = true;
					rbody.collisionDetectionMode = CollisionDetectionMode.ContinuousSpeculative;
					rbody.isKinematic = true;
				}
			}
		}

		if (IsCyberNPC()) rbody.useGravity = false;
		if (index == 14) {
			CapsuleCollider capcol = GetComponent<CapsuleCollider>();
			capcol.enabled = true;
		}
		asleep = false;
		rbody.constraints = RigidbodyConstraints.None;
		if (!rbody.freezeRotation) rbody.freezeRotation = true;
		gameObject.layer = 13; // Change to Corpse layer

		// Bump it up a hair to prevent corpse falling through the floor
		//transform.position = new Vector3(transform.position.x,
		//								 transform.position.y + 0.04f,
		//								 transform.position.z);

		firstSighting = true;

		// Timer for wait until death animation finishes before Dead().
		timeTillDeadFinished = Sys_Global.pauseRelativeTime;
		timeTillDeadFinished += Const.a.timeTillDead[index];
		if (Const.a.switchMaterialOnDeath[index]
			&& deathMaterial != null && actualSMR != null) {
			actualSMR.material = deathMaterial;
		}

		if (index == 9 || index == 20) {
			rbody.velocity = new Vector3(0f,rbody.velocity.z,0f);
		}

		dyingSetup = true;
// 		Debug.Log("NPC " + gameObject.name + " finished dying setup");
	}

	public bool DeactivatesVisibleMeshWhileDying() {
		return (index == 0 // Autobomb
				|| index == 14 // Hopper
				|| index == 20 // Zero-g mutant
				|| healthManager.teleportOnDeath);
	}

	void Dying() {
		if (!dyingSetup) DyingSetup();

		// Check if timer for dying animation is finished letting it play.
		if (timeTillDeadFinished < Sys_Global.pauseRelativeTime) {
			ai_dead = true;
// 			Debug.Log("NPC " + gameObject.name + " has now died");
			ai_dying = false;
			currentState = AIState.Dead;
		}

		if (DeactivatesVisibleMeshWhileDying() && visibleMeshEntity.activeSelf) {
			Utils.Deactivate(visibleMeshEntity);
			UnityEngine.Debug.Log("DeactivatesVisibleMeshWhileDying while dying deactivate visibleMeshEntity");
			if (visibleMeshVisible) {
				visibleMeshVisible = false;
// 				Debug.Log("NPC " + gameObject.name + " visibleMeshVisible now "
// 						  + "false due to dying");
			}
		}

		if (index == 20) searchColliderGO.SetActive(true);
	}

	void Dead() {
		asleep = false;
		ai_dead = true;
		ai_dying = false;
		dyingSetup = false;
		if (deadChecksDone) return;
		
		if (DeactivatesVisibleMeshWhileDying() && visibleMeshEntity.activeSelf) {
			UnityEngine.Debug.Log("DeactivatesVisibleMeshWhileDying while dead deactivate visibleMeshEntity");
			Utils.Deactivate(visibleMeshEntity);
			visibleMeshVisible = false;
// 			Debug.Log("NPC " + gameObject.name + " visibleMeshVisible now "
// 			+ "false due to dead");
		}

		currentState = AIState.Dead;
		gameObject.layer = 13; // Corpse layer
		if (searchColliderGO != null && (!healthManager.gibOnDeath
											|| index == 2)) { // Avian Mutant
			searchColliderGO.SetActive(true);
			rbody.constraints = RigidbodyConstraints.FreezePositionX
								| RigidbodyConstraints.FreezePositionZ;
		}

		Utils.DisableImage(healthManager.linkedOverlay);
		if (!rbody.freezeRotation) rbody.freezeRotation = true;
		if (healthManager.gibOnDeath || healthManager.teleportOnDeath
			|| IsCyberNPC()) {

			rbody.useGravity = false;

			// Normally just turn off the main model, then turn on lovely gibs.
			if (healthManager.gibOnDeath) healthManager.Gib();
			if (healthManager.teleportOnDeath && !healthManager.teleportDone) {
				healthManager.TeleportAway();
				rbody.useGravity = true;
			}

			Utils.Deactivate(visibleMeshEntity);
			visibleMeshVisible = false;
		} else {
			if (index != 14) { // Hopper turns itself off.
				rbody.useGravity = true;
			}
		}

		deadChecksDone = true;
	}

	bool CheckIfEnemyInSight() {
		if (!HasHealth(healthManager)) return false;
		
		bool enemyIsNPC = enemy.layer == 10;
	    int diff = Const.a.difficultyCombat;
		if (IsCyberNPC()) {
			diff = Const.a.difficultyCyber;
		} else {
			if ((!withinPVS && !enemyIsNPC) && DynamicCulling.a.cullEnabled) return false;
		}

        if (diff == 0 && index != 28) return false;

		if (PlayerMovement.a.Notarget && !enemyIsNPC) {
			enemy = null; // Force forget when using Notarget cheat.
			posCheckFinished = Sys_Global.pauseRelativeTime + positionCheckDelay;
			lastPosition = transform.position;
			LOSpossible = false;
			return false;
		}

		if (IsCyberNPC() && Const.a.decoyActive) {
			//Debug.Log("Decoy forget!");
			LOSpossible = false; // Silly decoy hack to prevent seeing player.
			return false;
		}

		// Get distance between enemy and found player
		float dist = Vector3.Distance(enemy.transform.position,
									  sightPoint.transform.position);

		if (dist > Const.a.sightRange[index]) {
			//Debug.Log("Distance to enemy is too far to see");
			return false;
		}

		if (IsCyberNPC() || enemyIsNPC) return true;

		if (Const.a.numberOfRaycastsThisFrame > Const.maxRaycastsPerFrame) {
			return inSight; // Zero order hold last until next actual update.
		}

		// Get vector line made from enemy to found player
		Vector3 line = enemy.transform.position - sightPoint.transform.position;
        if (Physics.Raycast(sightPoint.transform.position,line.normalized,
							out tempHit, Const.a.sightRange[index],
							Const.a.layerMaskNPCSight)) {
			Const.a.numberOfRaycastsThisFrame++;
			GameObject hitObj = tempHit.collider.gameObject;
            if (hitObj == enemy) {
				LOSpossible = true;
                return true;
			} else {
				// If we are a smart cookie, open doors if we see a door while trying to look at player.
				if (hitObj != null && (Vector3.Distance(tempHit.point,sightPoint.transform.position) < 2f)
					&& Const.a.type[index] != NPCType_Mutant && Const.a.type[index] != NPCType_Supermutant && Const.a.type[index] != NPCType_Cyber) {

					Door dr = hitObj.GetComponent<Door>();
					if (dr == null) {
						UseHandlerRelay uhr = hitObj.GetComponent<UseHandlerRelay>();
						if (uhr != null) {
							if (uhr.referenceUseHandler != null) {
								dr = uhr.referenceUseHandler.GetComponent<Door>();
							}
						}
					}

					if (dr != null) {
						if ((dr.doorOpen == DoorState.Closed || (dr.doorOpen == DoorState.Closing && Const.a.difficultyCombat > 2))
							&& !dr.locked && (LevelManager.a.GetCurrentLevelSecurity() <= dr.securityThreshhold)
							&& (dr.requiredAccessCard == AccessCardType.None || dr.accessCardUsedByPlayer || Inventory.a.HasAccessCard(dr.requiredAccessCard))) {
						
							dr.DoorActuate();
						}
					}
				}
			}
        }

        LOSpossible = false;
		//Debug.Log("Can't see current enemy after raycasting");

        return false;
	}

	bool CheckIfPlayerInSight() {
	    int diff = Const.a.difficultyCombat;
		if (IsCyberNPC()) {
			diff = Const.a.difficultyCyber;
		} else {
			if (!withinPVS && DynamicCulling.a.cullEnabled) return false;
		}

        if (diff == 0 && index != 28) return false;
		if (enemy != null) return CheckIfEnemyInSight();

		LOSpossible = false; // Reset line of sight value. Doing this after 
							 // CheckIfEnemyInSight so it doesn't break it.

		if (IsCyberNPC() && Const.a.decoyActive) {
			//Debug.Log("Decoy forget!");
			return false;
		}
		if (Const.a.player1Capsule == null) return false; // No found player

		// Can't see him, he's on notarget.
		if (PlayerMovement.a.Notarget) return false;

		tempVec = Const.a.player1Capsule.transform.position;

		// Get distance between enemy and found player
		float dist = Vector3.Distance(tempVec,sightPoint.transform.position);

		// Don't waste time raycasting if we won't be able to see them anyway.
		if (dist > Const.a.sightRange[index]) return false;
        
        if (IsCyberNPC()) {
			SetEnemy(Const.a.player1Capsule,Const.a.player1TargettingPos);
			PlaySightSound();
			return true;
		}

		// Get vector line made from enemy to found player
		Vector3 checkline = tempVec - sightPoint.transform.position;
		float angle = Vector3.Angle(checkline,sightPoint.transform.forward);
		if (angle < (Const.a.fov[index] * 0.5f)) {
			// Check for line of sight
			if (Const.a.RaycastBudgetExceeded()) return inSight; // No change.

			// Changed from using sight range to dist to minimize checkdistance.
			if (Physics.Raycast(sightPoint.transform.position,
								checkline.normalized,out tempHit,
								(dist + 0.1f),Const.a.layerMaskNPCSight)) {
				Const.a.numberOfRaycastsThisFrame++;
				if (tempHit.collider.gameObject == Const.a.player1Capsule) {
					LOSpossible = true;  // Clear path from enemy to found player
					SetEnemy(Const.a.player1Capsule,Const.a.player1TargettingPos);
					PlaySightSound();
					return true;
				}
			} else {
				if (PlayerHealth.a.makingNoise) {
					if (dist < Const.a.hearingRange[index]) {
						SetEnemy(Const.a.player1Capsule,Const.a.player1TargettingPos);
						PlaySightSound();
						return true;
					}
				}
			}
		} else {
			if (dist < Const.a.distToSeeBehind[index]) {
				// Still check for line of sight, some locations there could be
				// walls in the ways still due to the angles.  Changed from
				// using sight range to dist to minimize checkdistance; added
				// slight amount to it though to avoid quantization inaccuracies.

				if (Const.a.numberOfRaycastsThisFrame > Const.maxRaycastsPerFrame) {
					return inSight; // Don't change it, zero order hold.
				}

				if (Physics.Raycast(sightPoint.transform.position,
									checkline.normalized, out tempHit,
									(dist + 0.1f),Const.a.layerMaskNPCSight)) {
					Const.a.numberOfRaycastsThisFrame++;
					if (tempHit.collider.gameObject == Const.a.player1Capsule) {
						LOSpossible = true; // Clear path from enemy to player.
						SetEnemy(Const.a.player1Capsule,Const.a.player1TargettingPos);
						PlaySightSound();
						return true;
					}
				}
			}
			if (PlayerHealth.a.makingNoise) {
				if (dist < Const.a.hearingRange[index]) {
					SetEnemy(Const.a.player1Capsule,Const.a.player1TargettingPos);
					PlaySightSound();
					return true;
				}
			}
		}

		return false;
	}

	public void SetEnemy(GameObject enemSent,Transform targettingPosSent) {
		if (enemSent == null) return;

		enemy = enemSent;
		posCheckFinished = Sys_Global.pauseRelativeTime + positionCheckDelay;
		wandering = false;
		wanderFinished = Sys_Global.pauseRelativeTime;
		lastPosition = transform.position;
		enemyHM = Utils.GetMainHealthManager(enemSent);
		lastKnownEnemyPos = enemy.transform.position;
		targettingPosition = targettingPosSent.position;
		SetHuntFinished();
	}

	void PlaySightSound() {
		if (firstSighting && HasHealth(healthManager)) {
			firstSighting = false;
			if (!healthManager.actAsCorpseOnly) {
				SFXIndex = Const.a.sfxSightSound[index];
				Utils.PlayOneShotSavable(SFX,SFXIndex);	
			}
		}
	}
	
	void enemyInFrontChecks(GameObject target) {
		if (target == null) {
			infront = false;
			inProjFOV = false;
			return;
		}

	    if (IsCyberNPC()) {
	        infront = true;
	        inProjFOV = true;
	        return;
	    }

        infrontVec = target.transform.position;
		infrontVec.y = sightPoint.transform.position.y; // Ignore height delta.
		infrontVec = Vector3.Normalize(infrontVec - sightPoint.transform.position);
		inProjFOV = false;
		infront = false;
        dotResult = Vector3.Dot(infrontVec,sightPoint.transform.forward);
        if (dotResult > 0.800) {
			inProjFOV = true; // Enemy is within ±18° of forward facing vector.
		}

        if (dotResult > 0.300) {
			infront = true;   // Enemy is within ±63° of forward facing vector.
		}
    }

	public void Alert(UseData ud) {
		if (Const.a.difficultyCombat == 0) return;

		SetEnemy(Const.a.player1Capsule,Const.a.player1Capsule.transform);
		currentDestination = enemy.transform.position;
		inSight = false;
	}

	public void AwakeFromSleep(UseData ud) {
		asleep = false;
		Utils.Deactivate(sleepingCables);
		Alert(ud);
	}
}
