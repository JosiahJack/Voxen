using UnityEngine;
using System.Collections;
using System.Text;

public class PlayerHealth : MonoBehaviour {
	// External references, required
	public GameObject radiationEffect;
	public GameObject shieldEffect;

	// Internal references
	float radiated = 0f; // save
	private float resetAfterDeathTime = 0.5f;
	float timer; // save
	bool playerDead = false; // save
	private float mediPatchPulseTime = 0.5f;
	private float mediPatchHealAmount = 8f;
	bool radiationArea = false; // save
	 float radiationBleedOffFinished = 0f;
	private float radiationBleedOffTime = 1.8f;
	private float radiationReductionAmount = 1f;
	private float radiationHealthDamageRatio = 0.1f;
	private int radiationAmountWarningID = 323;
	private int radiationAreaWarningID = 322;
	float mediPatchPulseFinished = 0f; // save
	int mediPatchPulseCount = 0; // save, Used to incrementally increase the time between health increases by 0.5s every n+0.5s. Saved so we don't use quick load to cheat health faster.
	public bool makingNoise = false; // save
	HealthManager hm;
	float lastHealth; // save
	float painSoundFinished; // save
	float radSoundFinished; // save
	float radFXFinished; // save
	private float radAdjust;
	private float initialRadiation;
	float noiseFinished;
	int deaths = 0;
	int ressurections = 0;
	private static StringBuilder s1 = new StringBuilder();
	
	public static PlayerHealth a;

	void Awake() {
		a = this;
	}

	void Start () {
		hm = GetComponent<HealthManager>();
		if (hm == null) {
			DualLogError("BUG: No HealthManager script found on player (sent"
					  	   + " from PlayerHealth.Awake)");
		}

		painSoundFinished = Sys_Global.pauseRelativeTime;
		radSoundFinished = Sys_Global.pauseRelativeTime;
		radFXFinished = Sys_Global.pauseRelativeTime;
		noiseFinished = Sys_Global.pauseRelativeTime;
		lastHealth = hm.health;
		radAdjust = 0f;
		initialRadiation = 0f;
	}

	void Update() {
		if (Sys_Global.gamePaused || Sys_Global.menuActive) return;

		if (noiseFinished < Sys_Global.pauseRelativeTime) makingNoise = false;
		if (hm.health <= 0f) {
			if (!playerDead) PlayerDying();
			else PlayerDead();
			return;
		}

		if (instances[PLAYER1].patchActive & PATCH_MEDI) {
			if (mediPatchPulseFinished == 0) mediPatchPulseCount = 0;
			if (mediPatchPulseFinished < Sys_Global.pauseRelativeTime) {
				hm.HealingBed(mediPatchHealAmount,false);
				MFDManager.a.DrawTicks(true);
				mediPatchPulseFinished = Sys_Global.pauseRelativeTime + (mediPatchPulseTime + (mediPatchPulseCount * 0.5f));
				mediPatchPulseCount++;
			}
		} else {
			mediPatchPulseFinished = 0;
			mediPatchPulseCount = 0;
		}
		if (instances[PLAYER1].patchActive & PATCH_DETOX) radiated = 0f;
		if (radiated > 1f) {
			if (radiationArea) {
				// Radiation area
				PlayerMovement.a.twm.SendWarning((Sys_Text.stringTable[184]),
												  0.1f,-2,HUDColor.White,
												  radiationAreaWarningID);
			}

			if (!EnvirosuitApply()) PlayerMovement.a.twm.SendWarning((Sys_Text.stringTable[185] + radiated.ToString() +Sys_Text.stringTable[186]), 0.1f,-2,HUDColor.Red, radiationAmountWarningID); // Radiation poisoning ##LBP
			if (radFXFinished < Sys_Global.pauseRelativeTime) {
				radiationEffect.SetActive(true);
				float minT = 0.5f;
				if (radiated > 50f) minT = 0.25f;
				radFXFinished = Sys_Global.pauseRelativeTime + random_range(minT,1f);
			}
		} else {
			radiationArea = false;
			if (radiated < 0) radiated = 0;
		}

		if (radiationBleedOffFinished < Sys_Global.pauseRelativeTime) {
			if (!radiationArea) radiated -= radiationReductionAmount;  // Bleed off the radiation over time.
			if (radiated < 0) radiated = 0;
			radiationBleedOffFinished = Sys_Global.pauseRelativeTime + radiationBleedOffTime;
			if (radiated > 0) {
				if (!hm.god) {
					hm.health -= radiated*radiationHealthDamageRatio; // Apply health at rate of bleedoff time.
					MFDManager.a.DrawTicks(true);
				}
				if (radSoundFinished < Sys_Global.pauseRelativeTime) {
					radSoundFinished = Sys_Global.pauseRelativeTime + random_range(1f,3f);
					Utils.PlayUIOneShotSavable(90);
				}
			}
		}
		if (lastHealth > hm.health) { // Did we lose health?
			if (painSoundFinished < Sys_Global.pauseRelativeTime && !(radSoundFinished < Sys_Global.pauseRelativeTime)) {
				painSoundFinished = Sys_Global.pauseRelativeTime + random_range(0.25f,3f); // Don't spam pain sounds
				Utils.PlayUIOneShotSavable(140);
				PlayerHealth.a.makingNoise = true;
			}
		}
		
		lastHealth = hm.health;
	}
	
	void PlayerDying() {
		timer += Time.deltaTime;
		radiationArea = false;
		radiated = 0;
		makingNoise = false;
		MFDManager.a.DrawTicks(true);
		if (timer >= resetAfterDeathTime) {
			hm.health = 0f;
			playerDead = true;
		}
	}
	
	void PlayerDead() {
		if (MouseLookScript.a.heldObjectIndex != -1) {
			MouseLookScript.a.DropHeldItem();
			MouseLookScript.a.ForceInventoryMode();
		}	
		int lindex = LevelManager.a.currentLevel != -1 ? LevelManager.a.currentLevel : 0;
		hm.ClearOverlays();
		if (LevelManager.a.ressurectionActive[lindex])
			PlayerRessurect(); // Ressurection
		else
			PlayerDeathToMenu(); // Game Over
	}

	public void PlayerRessurect() {
		bool ressurected = LevelManager.a.RessurectPlayer();
		if (!ressurected) DualLog("ERROR: failed to ressurect player!");
		ressurections++;
		hm.health = 211f;
		MFDManager.a.DrawTicks(true);
		radiationArea = false;
		radiated = 0;
		playerDead = false;
		PlayerPatch.a.DisableAllPatches();
		instances[PLAYER1].fatigue = 0f;
	}

	public void PlayerDeathToMenu() {
		Const.a.loadingScreen.SetActive(true);

		// Death to Main Menu
		if (MouseLookScript.a.inventoryMode == false) {
			MouseLookScript.a.ToggleInventoryMode();
			AudioListener.pause = false;
		}

		GameObject newGameIndicator = GameObject.Find("NewGameIndicator");
		GameObject loadGameIndicator = GameObject.Find("LoadGameIndicator");
		GameObject freshGame = GameObject.Find("GameNotYetStarted");
		if (newGameIndicator != null) Utils.SafeDestroy(newGameIndicator);
		if (loadGameIndicator != null) Utils.SafeDestroy(loadGameIndicator);
		if (freshGame != null) Utils.SafeDestroy(freshGame);
		PauseScript.a.mainMenu.SetActive(true);
		MainMenuHandler.a.InitialDisplay.SetActive(false);
		MainMenuHandler.a.returnToPause = false;
		MainMenuHandler.a.GoToFrontPage();
		MainMenuHandler.a.PlayDeathVideo();
		hm.health = 211f;
		MFDManager.a.DrawTicks(true);
		radiationArea = false;
		radiated = 0;
		playerDead = false;
		PlayerPatch.a.DisableAllPatches();
		instances[PLAYER1].fatigue = 0f;
	}

	// Check for envirosuit and apply reduction based on version
	bool EnvirosuitApply() {
		radAdjust = 0f;
		if (!inventoryPlayer1.hasHardware[8]) return false;
		if (PlayerEnergy.a.energy <= 0) return false;

		float enerTake = 0f;
		float frac = 0.12f;
		float energCost = 0.11f;
		switch (inventoryPlayer1.hardwareVersion[8]) {
			case 1: frac = 0.17f; energCost = 0.25f; break;
			case 2: frac = 0.15f; energCost = 0.16f; break;
			case 3: frac = 0.12f; energCost = 0.11f; break;
		}

		radAdjust = radiated * frac;
		float diff = radiated - radAdjust;
		radiated = radAdjust; // After calculating difference.
		if (radiated < 0) radiated = 0;
		if (diff < 0) diff = 0; // Prevent underflow.
		enerTake = (energCost * diff);
		if (enerTake < 0) enerTake = 0;
		radAdjust = initialRadiation - radiated;
		if (radAdjust < 0) radAdjust = 0;
		DualLog("Taking energy for envirosuit: " + enerTake.ToString());

		// Suit absorbs some radiation, say it.
		// Envirosuit absorbed ##LBP, Radiation poisoning ##LBP
		PlayerMovement.a.twm.SendWarning((Sys_Text.stringTable[280]
											+ radAdjust.ToString()
											+ Sys_Text.stringTable[281]
											+ Sys_Text.stringTable[185]
											+ radiated.ToString()
											+ Sys_Text.stringTable[186]),
											0.1f,-2,HUDColor.Red,
											radiationAmountWarningID);

		PlayerEnergy.a.TakeEnergy(enerTake);
		if (BiomonitorGraphSystem.a != null) {
			BiomonitorEnergyPulse(enerTake);
		}
		return true;
	}

	public void GiveRadiation(float rad) {
		if (playerDead) return;

		if (radiated < rad) radiated = rad;
		else return;

		EnvirosuitApply();
		initialRadiation = radiated;
	}
}
