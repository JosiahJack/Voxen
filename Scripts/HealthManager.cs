
	SecurityType securityAffected; // Not a reference, needs no optional flag, if using DTValidator that is.
	float health = -1f; // save, Current health, set in inspector for a different starting health than default on enemies.
	float cyberHealth = -1f; //save
	float maxhealth; // maximum health
	int index; // NPC Index
	bool isPlayer = false;
	bool isGrenade = false;
	bool gibOnDeath = false; // used for things like crates to "gib" and shatter
	bool dropItemsOnGib = false;
    bool vaporizeCorpse = true;
	bool isNPC = false;
	bool isObject = false;
	bool isIce = false;
	bool isSecCamera = false;
	bool teleportOnDeath = false;
	bool actAsCorpseOnly = false;
	bool gibsGetVelocity = false;
	Vector3 gibVelocityBoost;
	PoolType deathFX;
	BloodType bloodType;
	string targetOnDeath;
	bool inCyberSpace = false; // Externally modifiable

	// Internal references
	GameObject attacker;
	float justHurtByEnemy;
	bool deathDone = false;
	AIController aic;
	private PrefabIdentifier prefID;
    private float tempFloat;
	private float take;
	bool god = false; // is this entity invincible? used for player cheat
	bool teleportDone;
	TargetID linkedTargetID;
	bool awakeInitialized = false;
	bool startInitialized = false;

	public void Awake () {
		if (awakeInitialized) return;

        attacker = null;
		take = 0;
		if (isPlayer) {
			health = 211;
			cyberHealth = 255;
			maxhealth = 255;
			justHurtByEnemy = (Time.time - 31f); // set less than 30s below Time to guarantee we don't start playing action music right away, used by Music.cs
		}
		
		if (Const.a != null) Const.a.RegisterObjectWithHealth(this);
		awakeInitialized = true;
		if (isNPC && !gibOnDeath ) { // Set searchable item to CorpseSearchable layer.
			if (searchableItem != null) searchableItem.gameObject.layer = 29;
		}
	}

	// Put into Start instead of Awake to give Const time to populate from enemy_tables.csv
	public void Start () {
		if (startInitialized) return;

		if (isNPC) {
			aic = GetComponent<AIController>();
			index = aic.index;
			if (Const.a != null) {
				if (IsCyberEntity()) {
					if (cyberHealth == -1) cyberHealth = Const.a.healthForCyberNPC[index];
					if (maxhealth == -1) maxhealth = Const.a.healthForCyberNPC[index];
				} else {
					if (health == -1) health = Const.a.healthForNPC[index]; //leaves possibility of setting health lower than normal, for instance the cortex reaver on level 5
					if (maxhealth == -1) maxhealth = Const.a.healthForNPC[index]; // set maxhealth to default healthForNPC, possible to set higher, e.g. for cyborg assassins on level 9 whose health is 3 times normal
				}

				if (Eng_Global->difficultyCombat == 0) {
					maxhealth = 1;
					health = maxhealth;
				}
			}
			if (actAsCorpseOnly && isNPC) InitializeCorpseOnly();
        }
		if (maxhealth <= 0) maxhealth = health;
		startInitialized = true;
	}

	void UseDeathTargets() {
		#if UNITY_EDITOR
			if (!Application.isPlaying) return;
		#endif

		if (isPlayer) return; // Player death does nothing.

		UseData ud = new UseData();
		UseTargets(gameObject,ud,targetOnDeath);
	}

	void InitializeCorpseOnly() {
		health = 0;
		cyberHealth = 0;
		if (aic != null) { if (aic.SFX != null) aic.SFX.enabled = false; }
		if (!deathDone) {
			UseDeathTargets();
			if (teleportOnDeath) TeleportAway();
			else NPCDeath();
		}
	}

	float ApplyAttackTypeAdjustments(float take,DamageData dd) {
		if (isNPC && health > 0f) {
			if (npcTable[NPCID].type[index] == NPCType.Mutant) {
				switch(dd.attackType) {
					case AttackType.None: take *= 1f; break; // same
					case AttackType.Melee: take *= 1f; break; // same
					case AttackType.MeleeEnergy: take *= 1f; break; // same
					case AttackType.EnergyBeam: take *= 1f; break; // same
					case AttackType.Magnetic: take *= 0; break;
					case AttackType.Projectile: take *= 1f; break; // same
					case AttackType.ProjectileEnergyBeam: take *= 1f; break; // same
					case AttackType.ProjectileLaunched: take *= 1f; break; // same
					case AttackType.Gas: take *= 2f; break;
					case AttackType.ProjectileNeedle: take *= 2f; break; // same
					case AttackType.Tranq: take *= 1f; break; // same
				}
			}

			if (npcTable[NPCID].type[index] == NPCType.Supermutant) {
				switch(dd.attackType) {
					case AttackType.None: take *= 1f; break; // same
					case AttackType.Melee: take *= 1f; break; // same
					case AttackType.MeleeEnergy: take *= 1f; break; // same
					case AttackType.EnergyBeam: take *= 1f; break; // same
					case AttackType.Magnetic: take *= 0; break; // no damage
					case AttackType.Projectile: take *= 1f; break; // same
					case AttackType.ProjectileEnergyBeam: take *= 1f; break; // same
					case AttackType.ProjectileLaunched: take *= 1f; break; // same
					case AttackType.Gas: take *= 1.5f; break;
					case AttackType.ProjectileNeedle: take *= 1f; break; // same
					case AttackType.Tranq: take *= 1f; break; // same
				}
			}

			if (npcTable[NPCID].type[index] == NPCType.Robot) {
				switch(dd.attackType) {
					case AttackType.None: take *= 1f; break; // same
					case AttackType.Melee: take *= 1f; break; // same
					case AttackType.MeleeEnergy: take *= 1f; break; // same
					case AttackType.EnergyBeam: take *= 1f; break; // same
					case AttackType.Magnetic: take *= 4f; break; // same
					case AttackType.Projectile: take *= 1f; break; // same
					case AttackType.ProjectileEnergyBeam: take *= 1f; break; // same
					case AttackType.ProjectileLaunched: take *= 1f; break; // same
					case AttackType.Gas: take = 0; break; // no damage
					case AttackType.ProjectileNeedle: take = 0; break; // no damage
					case AttackType.Tranq: take *= 1f; break; // no damage
				}
			}

			if (npcTable[NPCID].type[index] == NPCType.Cyborg) {
				switch(dd.attackType) {
					case AttackType.None: take *= 1f; break; // same
					case AttackType.Melee: take *= 1f; break; // same
					case AttackType.MeleeEnergy: take *= 1f; break; // same
					case AttackType.EnergyBeam: take *= 1f; break; // same
					case AttackType.Magnetic: take *= 2f; break; // same
					case AttackType.Projectile: take *= 1f; break; // same
					case AttackType.ProjectileEnergyBeam: take *= 1f; break; // same
					case AttackType.ProjectileLaunched: take *= 1f; break; // same
					case AttackType.Gas: take *= 1f; break; // same
					case AttackType.ProjectileNeedle: take *= 1f; break; // same
					case AttackType.Tranq: take *= 1f; break; // same
				}
			}

			if (npcTable[NPCID].type[index] == NPCType.Supercyborg) {
				switch(dd.attackType) {
					case AttackType.None: take *= 1f; break; // same
					case AttackType.Melee: take *= 1f; break; // same
					case AttackType.MeleeEnergy: take *= 1f; break; // same
					case AttackType.EnergyBeam: take *= 1f; break; // same
					case AttackType.Magnetic: take *= 2f; break; // same
					case AttackType.Projectile: take *= 1f; break; // same
					case AttackType.ProjectileEnergyBeam: take *= 1f; break; // same
					case AttackType.ProjectileLaunched: take *= 1f; break; // same
					case AttackType.Gas: take = 0; break;
					case AttackType.ProjectileNeedle: take = 0; break;
					case AttackType.Tranq: take *= 1f; break;
				}
			}

			if (npcTable[NPCID].type[index] == NPCType.MutantCyborg) {
				switch(dd.attackType) {
					case AttackType.None: take *= 1f; break; // same
					case AttackType.Melee: take *= 1f; break; // same
					case AttackType.MeleeEnergy: take *= 1f; break; // same
					case AttackType.EnergyBeam: take *= 1f; break; // same
					case AttackType.Magnetic: take *= 0.5f; break; // same
					case AttackType.Projectile: take *= 1f; break; // same
					case AttackType.ProjectileEnergyBeam: take *= 1f; break; // same
					case AttackType.ProjectileLaunched: take *= 1f; break; // same
					case AttackType.Gas: take *= 2f; break; // same
					case AttackType.ProjectileNeedle: take *= 2f; break; // same
					case AttackType.Tranq: take *= 1.5f; break; // same
				}
			}

			if (npcTable[NPCID].type[index] == NPCType.Cyber) {
				switch(dd.attackType) {
					case AttackType.None: take *= 1f; break; // same
					case AttackType.Melee: take *= 1f; break; // same
					case AttackType.MeleeEnergy: take *= 1f; break; // same
					case AttackType.EnergyBeam: take *= 1f; break; // same
					case AttackType.Magnetic: take *= 1f; break; // same
					case AttackType.Projectile: take *= 1f; break; // same
					case AttackType.ProjectileEnergyBeam: take *= 1f; break; // same
					case AttackType.ProjectileLaunched: take *= 1f; break; // same
					case AttackType.Gas: take *= 1f; break; // same
					case AttackType.ProjectileNeedle: take *= 1f; break; // same
					case AttackType.Tranq: take *= 1f; break; // same
					case AttackType.Drill: take = 0f; break; // same
				}
			}
		}
		return take;
	}

	float TakeDamage(uint16_t this, DamageData dd) {
		if (this.god) return 0; // untouchable!

        tempFloat = health;
		if (IsCyberEntity()) {
			tempFloat = cyberHealth;
			if (dd.attackType == AttackType.Drill && isNPC) return 0; // Drill can't hurt NPC's
			if (dd.attackType != AttackType.Drill && isIce) return 0; // Pulser can't hurt Ice
		}

		// Object is dead exceptions.
		if (tempFloat <= 0) {
			if (gibOnDeath || isIce || isPlayer || isGrenade || Eng_Global->instances[selfIdx].index == 279 || isSecCamera || teleportOnDeath) return 0;
		}

		take = dd.damage;
		if (isPlayer) {
			float absorb = 0;
			if (inCyberSpace) {
				if (inventoryPlayer1.hasSoft[2]) {
					switch(inventoryPlayer1.softVersions[2]) {
						case 0: absorb = 0.00f; break;
						case 1: absorb = 0.10f; break;
						case 2: absorb = 0.15f; break;
						case 3: absorb = 0.20f; break;
						case 4: absorb = 0.25f; break;
						case 5: absorb = 0.30f; break;
						case 6: absorb = 0.35f; break;
						case 7: absorb = 0.40f; break;
						case 8: absorb = 0.45f; break;
						case 9: absorb = 0.50f; break;
					}

					take = (take * (1f - absorb)); // absorb percentage from above table
					if (take <= 0f) return 0f; // nothing to see here
				}
			} else {
				// Check if player shield is active
				if (dd.attackType == AttackType.Magnetic) {
					take = 0f; // don't get hurt by magnetic interactions
					empstatic.Flash(2);
					TakeEnergy(11f);
					if (BiomonitorGraphSystem.a != null) {
						BiomonitorEnergyPulse(11f);
					}
				}

				if (inventoryPlayer1.hardwareIsActive[5] && inventoryPlayer1.hasHardware[5]) {
					// Versions of shield protect against 20, 40, 75, 75%'s
					// Versions of shield thresholds are 0, 10, 15, 30...ooh what's this hang on now...Huh, turns out it absorbs all damage below the thresshold!  Cool!
					float thresh = 0;
					switch(inventoryPlayer1.hardwareVersion[5]) {
						case 0: absorb = 0.2f;   thresh = 0f; break;
						case 1: absorb = 0.4f;  thresh = 10f; break;
						case 2: absorb = 0.75f; thresh = 15f; break;
						case 3: absorb = 0.75f; thresh = 30f; break;
					}

					if (take < thresh) absorb = 1f; // ah yeah! absorb. it. all.
					if (absorb > 0) {
						if (absorb < 1f) absorb = absorb + random_range(-0.08f,0.08f); // +/- 8% variation - this was in the original I swear!  You could theoretically have 83% shielding max.
						if (absorb > 1f) absorb = 1f; // cap it at 100%....shouldn't really ever be here, nothing is 92% + 8%
						take *= (1f-absorb); // shield doing it's thing
						PlayerHealth.a.shieldEffect.SetActive(true); // Activate shield screen effect to indicate damage was absorbed, effect intensity determined by absorb amount
						Utils.PlayUIOneShotSavable(94); // Play shield absorb sound
						int abs = (int)(absorb * 100f); //  for int display of absorbption percent
						CenterStatusPrint("%s", Sys_Text.stringTable[208] + abs.ToString() + Sys_Text.stringTable[209],dd.other);  // Shield absorbs x% damage
					}
				}
				if (take > 0 && ((absorb <0.4f) || random_range(0.0f,1.0f) < 0.5f)) {
					Utils.PlayUIOneShotSavable(140); // Play player pain noise
					int intensityOfPainFlash = 0; // 0 = light
					if (take > 15f) {
						intensityOfPainFlash = 2; // 2 = heavy
					}

					if (take > 10f) {
						intensityOfPainFlash = 1; // 1 = med
					}

					pstatic.Flash(intensityOfPainFlash);
				}

				if (dd.ownerIsNPC) justHurtByEnemy = Eng_Global->pauseRelativeTime;
			}
		}

		// Do the damage, that's right do. your. worst!
		if (IsCyberEntity()) {
			float cybbefore = cyberHealth;
			cyberHealth -= take;
			if (isPlayer) {
			    Const.a.damageReceived += take;
				Sys_UI.DrawTicks(true);
				if (cyberHealth <= 0) {
					MouseLookScript.a.ExitCyberspace();
					return 0f;
				}
			}
			
			if (dd != null) {
				if (dd.owner != null) {
					if (dd.owner.CompareTag("Player")) {
						Const.a.damageDealt += take;
					}
				}
			}
		} else {
			float before = health;
			take = ApplyAttackTypeAdjustments(take,dd); // Apply critical based on AttackType

			// Was directly dd.damage but changed since we are check for extra
			// things in case GetDamageTakeAmount wasn't called on dd.damage
			// beforehand (e.g. player fall damage, internal to player only,
			// need to protect against shield, etc, JJ 9/5/19).
			Transform par = transform.parent;
			if (par != null) {
				PrefabIdentifier camPid = par.gameObject.GetComponent<PrefabIdentifier>();
				if (camPid != null) {
					if (camPid.constIndex == 477 && dd.attackType == AttackType.Tranq) take = health + 1; // Tranq darts kill cameras
				}
			}
			health -= take;
			if (isPlayer) {
			    Const.a.damageReceived += take;
				Sys_UI.DrawTicks(true);
				Music.a.inCombat = true;
			}
			
			if (dd.owner != null) {
				if (dd.owner.CompareTag("Player")) Const.a.damageDealt += take;
			}
		}

		attacker = dd.owner;
		if (isNPC && (health > 0f || (IsCyberEntity() && cyberHealth > 0f))) {
			AIController aic = GetComponent<AIController>();
			if (aic != null) {
				if (Const.a.timeBetweenPainForNPC[aic.index] > 0) {
					aic.goIntoPain = true;
				}

				aic.attacker = attacker;
				if (linkedTargetID != null) {
					linkedTargetID.SendDamageReceive(take,dd);
				}

				aic.CheckPain(); // setup enemy with NPC
			}
		}

		if (IsCyberEntity()) {
			if (cyberHealth <= 0f) {
			    if (!isIce && isNPC) Const.a.cyberkills++;
				Death(false); // False since you can't vaporize cyberspace corpses.
			}
		} else {
			if (health <= 0f) {
			    if (isNPC) Const.a.kills++;
				Death(dd.attackType == AttackType.EnergyBeam);
			}
		}

		return take;
	}

	void Death(bool energyVaporized) {
		if (!deathDone) {
			UseDeathTargets();
			if (isIce) Utils.DisableCollision(gameObject);
			if (vaporizeCorpse && !isSecCamera && !isGrenade) VaporizeCorpse(energyVaporized);
			else if (isObject) ObjectDeath();
			else if (Eng_Global->instances[selfIdx].index == 279) ScreenDeath();
			else if (teleportOnDeath) TeleportAway();
			else if (isGrenade) GrenadeDeath();

			if (isNPC && !teleportOnDeath) NPCDeath();
			else if (isPlayer) PlayerHealth.a.deaths++;

			deathDone = true;
		}
	}

	void VaporizeCorpse(bool energyVaporized) {
		deathDone = true;
		DropSearchables();
		if (deathFX == PoolType.None) deathFX = PoolType.CorpseHit;
		if (energyVaporized) deathFX = PoolType.Vaporize;
		MeshRenderer mr = GetComponent<MeshRenderer>();
		Utils.DisableMeshRenderer(mr);
		PrefabIdentifier pid = GetComponent<PrefabIdentifier>();
		if (pid == null) {
			GameObject par = transform.parent.gameObject;
			pid = par.GetComponent<PrefabIdentifier>();
			if (pid != null) {
				if ((pid.constIndex >= 465 && pid.constIndex < 472)
					|| ConsoleEmulator.ConstIndexIsNPC(pid.constIndex)) {
					
					Utils.SafeDestroy(par.gameObject); // All gone!
				}
			}
		}
		
		if (pid != null) {
			if ((pid.constIndex >= 465 && pid.constIndex < 472)
				|| ConsoleEmulator.ConstIndexIsNPC(pid.constIndex)) {
				
				Utils.SafeDestroy(gameObject); // All gone!
			}
		}

		CreateDeathEffects(deathFX);
	}

	void TeleportAway() {
		if (teleportDone) return;
        
        teleportDone = true;
        Utils.Activate(teleportEffect);
        Eng_Global->instances[i].collider = COLLIDER_TYPE_NONE; // Deactivate collisions.
        Eng_Global->instances[i].think = NULL;
        Eng_Global->instances[i].animationNum = MAX_ANIMATED_MODELS + 1;
        Eng_Global->instances[i].gravity = 0.0f;
        Eng_Global->instances[i].velocity = Eng_Global->instances[i].angularVelocity = (Vector3){ 0.0f, 0.0f, 0.0f };
        Eng_Global->instances[i].modelIdx = UINT16_MAX; // Removed from rendering.
	}

	void NPCDeath(uint16_t i) {
		if (deathDone) return; // We died the death, no 2nd deaths here.

		deathDone = true; // Mark it so we only die once.
		CreateDeathEffects(deathFX);
		if (NPCID == 0 && !Eng_Global->instances[i].actAsCorpseOnly) play_wav(sounds[64], 1.0f, Eng_Global->instances[i].position, true); // npc_autobomb: explosion1

		if (aic == null) {
			if (transform.parent != null) {
				aic = transform.parent.gameObject.GetComponent<AIController>();
			}
		}

		if (aic == null) return;

		if (npcTable[NPCID].type[aic.index] == NPCType.Cyber) {
			Utils.SafeDestroy(aic.gameObject);
		} else {
			// Ok.  We've been through this.  Must keep the parent collider on
			// in order to prevent NPC's randomly falling through the floor
			// when killed because Unity's physics are junk.
		}
	}
	
	public void ObjectDeath() {
		if (deathDone) return;

		if (gibOnDeath) {
			Gib();
		} else {
			Utils.DisableCollision(gameObject);
			DropSearchables();
			CreateDeathEffects(deathFX);
		}

		deathDone = true;
		if (linkedOverlay != null) {
			Utils.DisableImage(linkedOverlay); // Disable on automap
			Utils.Deactivate(linkedOverlay.gameObject);
		}

		if (securityAffected != SecurityType_None) {
			LevelManager.a.ReduceCurrentLevelSecurity(securityAffected);
		}

		int soundex = 62; // crate_break
		switch(prefID.constIndex) {
			case 458: soundex = 63; break; // prop_phys_barrel_chemical: explode_minor
			case 459: soundex = 66; break; // prop_phys_barrel_radiation: explosion3
			case 460: soundex = 66; break; // prop_phys_barrel_toxic: explosion3
			
			case 464: soundex = 62; break; // se_briefcase: crate_break
			case 465: soundex = 532; break; // se_corpse_blueshirt: impact_soft
			case 466: soundex = 532; break; // se_corpse_brownshirt: impact_soft
			case 467: soundex = 532; break; // se_corpse_eaten: impact_soft
			case 468: soundex = 532; break; // se_corpse_labcoat: impact_soft
			case 469: soundex = 532; break; // se_corpse_security: impact_soft
			case 470: soundex = 532; break; // se_corpse_tan: impact_soft
			case 471: soundex = 532; break; // se_corpse_torso: impact_soft
			case 472: soundex = 62; break; // se_crate1: crate_break
			case 473: soundex = 62; break; // se_crate2: crate_break
			case 474: soundex = 62; break; // se_crate3: crate_break
			case 475: soundex = 62; break; // se_crate4: crate_break
			case 476: soundex = 62; break; // se_crate5: crate_break
			case 477: soundex = 61; break; // sec_camera: camera_destroy
			case 478: soundex = 65; break; // sec_cpunode: explosion2
			case 479: soundex = 69; break; // sec_cpunode_small: screen_destroy
			
			case 525: soundex = 68; break; // prop_console01: hit3
			case 526: soundex = 68; break; // prop_console02: hit3
		}
		
		play_wav(sounds[soundex],1.0f,Eng_Global->instances[i].position,true);
		if (deathFX != PoolType.None) HideSelf();
	}

    public void Gib() {
		CreateDeathEffects(deathFX);
		if (gibObjects.Length > 0 ) {
			Rigidbody gibrbody = null;
			for (int i = 0; i < gibObjects.Length; i++) {
				Utils.Activate(gibObjects[i]);
				if (gibsGetVelocity) {
					gibrbody = gibObjects[i].GetComponent<Rigidbody>();
					if (gibrbody != null) {
						gibrbody.AddForce(gibVelocityBoost,ForceMode.Impulse);
					}
				}
			}
			
			gibrbody = null;

			for (int k=0;k<disableOnGib.Length;k++) {
				Utils.Deactivate(disableOnGib[k]);
			}
		}

		DropSearchables();
		if (Eng_Global->instances[selfIdx].index != 279) Utils.DisableCollision(gameObject);
		AIController aic = GetComponent<AIController>();
		if (aic != null) {
			if (aic.healthManager.gibOnDeath) { // We are a corpse here.
				// Turn off visible mesh entity from destroyed corpse.
				Utils.Deactivate(aic.visibleMeshEntity);
				aic.visibleMeshVisible = false;
			}
		}

		HideSelf(); // Can't deactivate parent as gibs are children!
    }

	void DropSearchables() {
		if (searchableItem == null) return;

		Sys_UI.NotifySearchThatSearchableWasDestroyed();
		GameObject levelDynamicContainer = LevelManager.a.GetCurrentDynamicContainer();
		for (int i=0;i<4;i++) {
			if (searchableItem.contEng_Global->instances[i] < 0) continue;

			GameObject tossObject =
				Instantiate(Const.a.GetPrefab(searchableItem.contEng_Global->instances[i] + 307),
							Eng_Global->instances[i].position,Const.a.quaternionIdentity)
								as GameObject;

			if (tossObject != null) {
				if (tossObject.activeSelf != true) tossObject.SetActive(true);
				tossObject.transform.SetParent(levelDynamicContainer.transform,true);
				tossObject.GetComponent<UseableObjectUse>().customIndex =
					searchableItem.customIndex[i];
			} else {
				CenterStatusPrint("BUG: Failed to instantiate object being dropped on gib.");
			}
			searchableItem.contEng_Global->instances[i] = -1;
			searchableItem.customIndex[i] = -1;
		}
	}

	public void GrenadeDeath() {
		GrenadeActivate ga = GetComponent<GrenadeActivate>();
		ga.Explode();
	}

	public void ScreenDeath() {
		if (deathDone) return;

		deathDone = true; // Screens maintain collisions, so not disabling here; also maintain visible mesh, don't turn it off
		play_wav(sounds[69],1.0f,Eng_Global->instances[i].position,true);
		ImageSequenceTextureArray ista = GetComponent<ImageSequenceTextureArray>();
		ista.Destroy(); // ista deada nowa
		if (gibOnDeath) Gib();
	}

	void CreateDeathEffects(PoolType fx) {
		if (fx == PoolType.None) return;

		GameObject explosionEffect = Const.a.GetObjectFromPool(fx);
		if (explosionEffect == null) return;

		Vector3 pos = Eng_Global->instances[i].position;
		BoxCollider boxCol = GetComponent<BoxCollider>();
		if (boxCol != null) pos = transform.TransformPoint(boxCol.center);

		// MeshCollider doesn't have a center, so don't check meshCol here.

		SphereCollider sphereCol = GetComponent<SphereCollider>();
		if (sphereCol != null) pos = transform.TransformPoint(sphereCol.center);

		CapsuleCollider capCol = GetComponent<CapsuleCollider>();
		if (capCol != null) pos = transform.TransformPoint(capCol.center);

 		// Enable death effects (e.g. explosion particle effect)
		explosionEffect.SetActive(true);
		explosionEffect.Eng_Global->instances[i].position = pos;
	}

	void HideSelf() {
		if (Eng_Global->instances[selfIdx].index == 279) return;

		if (isSecCamera) {
			PrefabIdentifier pid = transform.parent.gameObject.GetComponent<PrefabIdentifier>();
			if (pid != null) {
				if (pid.constIndex == 477) {
					Destroy(transform.parent.gameObject);
					return;
				}
			}
		}
		MeshRenderer mr = GetComponent<MeshRenderer>();
		Utils.DisableMeshRenderer(mr);
		Rigidbody rbody = GetComponent<Rigidbody>();
		if (rbody != null) rbody.useGravity = false;
		rbody = null;
	}

	public void HealingBed(float amount,bool flashBed) {
		health += amount;
		if (health > 255) health = 255;
		if (isPlayer) Sys_UI.DrawTicks(true);
		if (flashBed && healingFXFlash != null) healingFXFlash.SetActive(true);
	}

	public void AwakeFromLoad(int levID) {
		if (isNPC) return;
		
		if (!awakeInitialized) { Awake(); }
		if (!startInitialized) { Start(); }

		// Handle objects (includes corpses)
		if (isObject) {
			if (gibOnDeath) {
				int gibcnt = gibObjects.Length;
				if (gibcnt > 0) {
					if (gibObjects[0] != null) {
						for (int i=0;i<gibcnt;i++) {
							if (health > 0) {
								if (gibObjects[i].activeSelf) {
									gibObjects[i].SetActive(false);
								}
							} else {
								if (!gibObjects[i].activeSelf) {
									gibObjects[i].SetActive(true);
								}
							}
						}
					}
				}
			}

			UpdateLinkedOverlay();
			if ((health > 0 && !IsCyberEntity())
				|| (cyberHealth > 0 && IsCyberEntity())) {

				Utils.EnableCollision(gameObject);
				MeshRenderer mr = GetComponent<MeshRenderer>();
				if (mr != null) {
					mr.enabled = true;
				}
			} else {
				// No health
				// ------------------------------------------------------------
				Utils.DisableCollision(gameObject);
				if (linkedOverlay != null) {
					Utils.DisableImage(linkedOverlay); // Disable on automap.
					Utils.Deactivate(linkedOverlay.gameObject);
				}

				MeshRenderer mr = GetComponent<MeshRenderer>();
				if (mr != null) {
					mr.enabled = false;
				}
			}
		}

		// Handle screens
		if (Eng_Global->instances[selfIdx].index == 279) {
			if (health > 0) {
				ImageSequenceTextureArray ista = GetComponent<ImageSequenceTextureArray>();
				if (ista != null) {
					ista.AwakeFromLoad(health);
				}
			}
		}

		// Handle grenades
		if (isGrenade) {
			GrenadeActivate ga = GetComponent<GrenadeActivate>();
			if (ga != null) ga.AwakeFromLoad(health);
		}
	}

	public bool IsCyberEntity() { 
		if (inCyberSpace) return true;
		if (!isPlayer && cyberHealth > 0f) return true;
		return (index > 23 && isNPC); // 24, 25, 26, 27, 28 are Cyber enemies
	}
}
