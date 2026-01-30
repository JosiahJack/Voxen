
	public GameObject playerCamera;
	public HealthManager hm;
	public Texture2D b1;
	public Texture2D b2;
	public Texture2D b3;
	public Texture2D b4;
	public Texture2D b5;
	public Texture2D b6;
	public Texture2D b7;
	public Light sightLight;
	public Image sightDimming;
	public PuzzleWire wirePuzzle;
	public BerserkEffect berserk;
	public BerserkEffect sensaroundCamCenterBerserk;
	public BerserkEffect sensaroundCamLeftBerserk;
	public BerserkEffect sensaroundCamRightBerserk;


	// Patches stack so multiple can be used at once
	// For instance, berserk + staminup + medi = 1 + 64 + 8 = 73
	// This is turning on bits in the int patchActive so above would be: 01001001,
	// meaning 3 patches are enabled out of the 7 types (short integer has 8 bits
	// but the 7th bit can be used for sign +/-)

	void Awake () {
		a = this;
		a.mediFinishedTime = -1f;
		a.reflexFinishedTime = -1f;
		a.sightFinishedTime = -1f;
		a.sightLight.enabled = false;
		a.BerserkDisable();
	}

	public void ActivatePatch(int index) { // Expects the usableItems index
		bool depleted = false;
		switch (index) {
		case 14:
			// Berserk Patch
			inventoryPlayer1.patchCounts[2]--;
			if (inventoryPlayer1.patchCounts[2] <= 0) { depleted = true; }
			if (!(instances[PLAYER1].patchActive & PATCH_BERSERK)) instances[PLAYER1].patchActive |= PATCH_BERSERK;
			berserkFinishedTime = Sys_Global.pauseRelativeTime + Const.berserkTime;
			float berserkIncrementTime = Const.berserkTime/5f;
			if (berserkIncrementFinishedTime > Sys_Global.pauseRelativeTime) berserkIncrementFinishedTime += berserkIncrementTime; // berserk effect stacks
			else                                                           berserkIncrementFinishedTime = Sys_Global.pauseRelativeTime + berserkIncrementTime;
			break;
		case 15:
			// Detox Patch
			inventoryPlayer1.patchCounts[6]--;
			if (inventoryPlayer1.patchCounts[6] <= 0) { depleted = true; }
			DisableAllPatches(); // remove all other effects, even medipatch
			instances[PLAYER1].patchActive = PATCH_DETOX; // overwrite all other active patches
			detoxFinishedTime = Sys_Global.pauseRelativeTime + Const.detoxTime; // detox doesn't stack, it cancels itself lol
			break;
		case 16:
			// Genius Patch
			inventoryPlayer1.patchCounts[5]--;
			if (inventoryPlayer1.patchCounts[5] <= 0) { depleted = true; }
			if (!(instances[PLAYER1].patchActive & PATCH_GENIUS)) instances[PLAYER1].patchActive |= PATCH_GENIUS;
			if (geniusFinishedTime > Sys_Global.pauseRelativeTime) {
				geniusFinishedTime += Const.geniusTime; // genius effect stacks
			} else {
				geniusFinishedTime = Sys_Global.pauseRelativeTime + Const.geniusTime;
			}
			break;
		case 17:
			// Medi Patch
			if (hm.health >=255) {
				CenterStatusPrint("%s", Sys_Text.stringTable[304],MouseLookScript.a.player);
				return;
			}
			inventoryPlayer1.patchCounts[3]--;
			if (inventoryPlayer1.patchCounts[3] <= 0) { depleted = true; }
			if (!(instances[PLAYER1].patchActive & PATCH_MEDI)) instances[PLAYER1].patchActive |= PATCH_MEDI;
			PlayerHealth.a.mediPatchPulseCount = 0;
			if (mediFinishedTime > Sys_Global.pauseRelativeTime) {
				mediFinishedTime += Const.mediTime; // medipatch effect stacks
			} else {
				mediFinishedTime = Sys_Global.pauseRelativeTime + Const.mediTime;
			}
			break;
		case 18:
			// Reflex Patch
			inventoryPlayer1.patchCounts[4]--;
			if (inventoryPlayer1.patchCounts[4] <= 0) { depleted = true; }
			Time.timeScale = Const.reflexTimeScale;
			if (!(instances[PLAYER1].patchActive & PATCH_REFLEX)) instances[PLAYER1].patchActive |= PATCH_REFLEX;
			if (reflexFinishedTime > Time.realtimeSinceStartup ) {
				reflexFinishedTime += Const.reflexTime; // reflex effect stacks
			} else {
				reflexFinishedTime = Time.realtimeSinceStartup + Const.reflexTime;
			}
			break;
		case 19:
			// Sight Patch
			inventoryPlayer1.patchCounts[1]--;
			if (inventoryPlayer1.patchCounts[1] <= 0) { depleted = true; }
			sightLight.enabled = true; // enable vision enhancement
			sightSideEffectFinishedTime = -1f;  // reset side effect timer from previous patch
			sightDimming.enabled = false; // deactivate side effect from previous patch
			if (!(instances[PLAYER1].patchActive & PATCH_SIGHT)) instances[PLAYER1].patchActive |= PATCH_SIGHT;
			if (sightFinishedTime > Sys_Global.pauseRelativeTime) sightFinishedTime += Const.sightTime; // sight effect stacks
			else                                                  sightFinishedTime = Sys_Global.pauseRelativeTime + Const.sightTime;
			break;
		case 20:
			// Staminup Patch
			inventoryPlayer1.patchCounts[0]--;
			if (inventoryPlayer1.patchCounts[0] <= 0) depleted = true;
			PlayerMovement.a.staminupActive = true;
			if (!(instances[PLAYER1].patchActive & PATCH_STAMINUP)) instances[PLAYER1].patchActive |= PATCH_STAMINUP;
			if (staminupFinishedTime > Sys_Global.pauseRelativeTime) {
				staminupFinishedTime += Const.staminupTime; // staminup effect stacks
			} else {
				staminupFinishedTime = Sys_Global.pauseRelativeTime + Const.staminupTime;
			}

			break;
		}

		if (depleted) {
			inventoryPlayer1.PatchCycleDown(false);
			CenterStatusPrint((Sys_Text.stringTable[590] + Sys_Text.stringTable[index + 326] + Sys_Text.stringTable[589]),MouseLookScript.a.player);
		} else {
			CenterStatusPrint((Sys_Text.stringTable[index + 326] + Sys_Text.stringTable[589]),MouseLookScript.a.player);
		}

		Utils.PlayUIOneShotSavable(89);
		GUIState.a.ClearOverButton();
	}

	void Update() {
		if (!Sys_Global.gamePaused && !Sys_Global.menuActive) {
			// ================================== DETOX PATCH =========================
			if (instances[PLAYER1].patchActive & PATCH_DETOX) {
				// ---Disable Patch---
				if (detoxFinishedTime < Sys_Global.pauseRelativeTime) {
					instances[PLAYER1].patchActive -= PATCH_DETOX; // Back to full force radiation effects, if present.  All normal.
				} else {
					// ***Patch Effect***
					instances[PLAYER1].patchActive = PATCH_DETOX; // Lets health script know to ameliorate the effects of radiation.
				}
			}

			// ================================== MEDI PATCH =========================
			if (instances[PLAYER1].patchActive & PATCH_MEDI) {
				// ---Disable Patch---
				if (mediFinishedTime < Sys_Global.pauseRelativeTime && mediFinishedTime != -1) {
					instances[PLAYER1].patchActive -= PATCH_MEDI;
					mediFinishedTime = -1;
				}
			}

			// ================================== REFLEX PATCH =======================
			if (instances[PLAYER1].patchActive & PATCH_REFLEX) {
				// ---Disable Patch---
				if (reflexFinishedTime < Time.realtimeSinceStartup && reflexFinishedTime != -1) {
					instances[PLAYER1].patchActive -= PATCH_REFLEX;
					Time.timeScale = Const.defaultTimeScale;
					reflexFinishedTime = -1;
				} else {
					// ***Patch Effect***
					if (Time.timeScale != Const.reflexTimeScale) Time.timeScale = Const.reflexTimeScale;
				}
			} else {
			    if (Time.timeScale != Const.defaultTimeScale) Time.timeScale = Const.defaultTimeScale;
			}

			// ================================== BERSERK PATCH =======================
			if (instances[PLAYER1].patchActive & PATCH_BERSERK) {
				// ---Disable Patch---
				if (berserkFinishedTime < Sys_Global.pauseRelativeTime) {
					berserkIncrement = 0;
					instances[PLAYER1].patchActive -= PATCH_BERSERK;
					BerserkDisable();
				} else {
					// ***Patch Effect***
					BerserkEnable();
					if (berserkIncrementFinishedTime < Sys_Global.pauseRelativeTime) {
						berserkIncrement++;
						switch (berserkIncrement) {
							case 0: berserk.swapTexture = b1; break;
							case 1: berserk.swapTexture = b2; berserk.IncrementStrength(); break;
							case 2: berserk.swapTexture = b3; break;
							case 3: berserk.swapTexture = b4; berserk.IncrementStats(); break;
							case 4: berserk.swapTexture = b5; break;
							case 5: berserk.swapTexture = b6; berserk.IncrementStats(); break;
							case 6: berserk.swapTexture = b7; berserk.IncrementStats(); break;
						}
						//gunCamBerserk.swapTexture = berserk.swapTexture;
						//gunCamBerserk.effectStrength = berserk.effectStrength;
						float berserkIncrementTime = Const.berserkTime/5f;
						berserkIncrementFinishedTime = Sys_Global.pauseRelativeTime + berserkIncrementTime;
					}
				}
			}

			// ================================== GENIUS PATCH ========================
			if (instances[PLAYER1].patchActive & PATCH_GENIUS) {
				// ---Disable Patch---
				if (geniusFinishedTime < Sys_Global.pauseRelativeTime) {
					MouseLookScript.a.geniusActive = false;
					instances[PLAYER1].patchActive -= PATCH_GENIUS;
					wirePuzzle.geniusActive = false;
				} else {
					// ***Patch Effect***
					MouseLookScript.a.geniusActive = true;  // so that LH/RH are swapped for mouse look
					wirePuzzle.geniusActive = true;
				}
			}

			// ================================== SIGHT PATCH =========================
			if (instances[PLAYER1].patchActive & PATCH_SIGHT) {
				// [[[Enable Side Effect]]]
				if (sightFinishedTime < Sys_Global.pauseRelativeTime && sightFinishedTime != -1f) {
					sightFinishedTime = -1f;
					sightSideEffectFinishedTime = Sys_Global.pauseRelativeTime + Const.sightSideEffectTime;
					sightLight.enabled = false;
					sightDimming.enabled = true;
				}

				// ---Disable Patch---
				if (sightSideEffectFinishedTime < Sys_Global.pauseRelativeTime && sightSideEffectFinishedTime != -1f) {
					sightSideEffectFinishedTime = -1f;
					sightFinishedTime = -1f;
					sightDimming.enabled = false;
					sightLight.enabled = false;
					instances[PLAYER1].patchActive -= PATCH_SIGHT;
				}
			}

			// ================================== STAMINUP PATCH ======================
			if (instances[PLAYER1].patchActive & PATCH_STAMINUP) {
				// ---Disable Patch---
				if (staminupFinishedTime < Sys_Global.pauseRelativeTime) {
					PlayerMovement.a.staminupActive = false;
					instances[PLAYER1].fatigue = 100f;  // side effect
					instances[PLAYER1].patchActive -= PATCH_STAMINUP;
				} else {
					// ***Patch Effect***
					instances[PLAYER1].fatigue = 0f;
					PlayerMovement.a.staminupActive = true;
				}
			}
		}
	}

	void BerserkEnable() {
		berserk.enabled = true;
		sensaroundCamCenterBerserk.enabled = true;
		sensaroundCamLeftBerserk.enabled = true;
		sensaroundCamRightBerserk.enabled = true;
	}

	void BerserkDisable() {
		berserk.Reset();
		berserk.enabled = false;
		sensaroundCamCenterBerserk.Reset();
		sensaroundCamCenterBerserk.enabled = false;
		sensaroundCamLeftBerserk.Reset();
		sensaroundCamLeftBerserk.enabled = false;
		sensaroundCamRightBerserk.Reset();
		sensaroundCamRightBerserk.enabled = false;
	}

	public void DisableAllPatches() {
		berserkFinishedTime = -1f;
		berserkIncrementFinishedTime =  -1f;
		berserkIncrement = 0;
		BerserkDisable();
		detoxFinishedTime =  -1f;
		geniusFinishedTime =  -1f;
		MouseLookScript.a.geniusActive = false;
		wirePuzzle.geniusActive = false;
		mediFinishedTime =  -1f;
		reflexFinishedTime =  -1f;
		Time.timeScale = Const.defaultTimeScale; // normal time speed
		sightFinishedTime =  -1f;
		sightSideEffectFinishedTime =  -1f;
		sightDimming.enabled = false;
		sightLight.enabled = false;
		staminupFinishedTime =  -1f;
		PlayerMovement.a.staminupActive = false;
		instances[PLAYER1].patchActive = 0;
	}
}
