// weapons.c - Weapon System
float delayBetweenShotsForWeapon[16]={1.0f,0.6f,0.5f,0.1f,0.8f,1.6f,0.65f,0.8f,0.6f,0.5f,1.2f,0.9f,0.5f,0.08f,1.1f,0.75f};
float delayBetweenShotsForWeapon2[16]={1.0f,4.5f,0.5f,0.15f,4.0f,1.6f,0.75f,0.8f,0.6f,0.5f,1.2f,0.9f,0.5f,0.08f,5.0f,0.75f};
float damagePerHitForWeapon[16]={75.0f,12.0f,15.0f,10.0f,18.0f,150.0f,15.0f,60.0f,45.0f,22.0f,50.0f,185.0f,6.0f,35.0f,6.0f,2.0f};
float damagePerHitForWeapon2[16]={160.0f,70.0f,5.0f,22.0f,108.0f,0.0f,0.0f,85.0f,80.0f,33.0f,350.0f,0.0f,0.0f,35.0f,36.0f,15.0f};
float damageOverloadForWeapon[16]={0.0f,115.0f,0.0f,0.0f,180.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,60.0f,0.0f};
float energyDrainLowForWeapon[16]={0.0f,3.0f,0.0f,0.0f,5.0f,0.0f,0.0f,0.0f,0.0f,0.0f,13.0f,0.0f,0.0f,0.0f,2.0f,3.0f};
float energyDrainHiForWeapon[16]={0.0f,15.0f,0.0f,0.0f,30.0f,0.0f,0.0f,0.0f,0.0f,0.0f,130.0f,0.0f,0.0f,0.0f,8.0f,30.0f};
float energyDrainOverloadForWeapon[16]={0.0f,50.0f,0.0f,0.0f,100.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,24.0f,0.0f};
float penetrationForWeapon[16]={50.0f,25.0f,6.0f,35.0f,35.0f,80.0f,40.0f,30.0f,100.0f,20.0f,0.0f,35.0f,0.0f,40.0f,25.0f,0.0f};
float penetrationForWeapon2[16]={70.0f,0.0f,0.0f,32.0f,0.0f,0.0f,0.0f,25.0f,120.0f,30.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f};
float offenseForWeapon[16]={4.0f,4.0f,2.0f,2.0f,6.0f,5.0f,3.0f,4.0f,4.0f,2.0f,3.0f,6.0f,2.0f,2.0f,3.0f,3.0f};
float offenseForWeapon2[16]={5.0f,0.0f,3.0f,3.0f,0.0f,0.0f,0.0f,5.0f,5.0f,3.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f};
uint8_t magazinePitchCountForWeapon[16]={10,0,15,60,0,0,0,12,25,20,0,12,20,50,0,0};
uint8_t magazinePitchCountForWeapon2[16]={8,0,15,60,0,0,0,12,10,20,0,0,0,100,0,0};
float recoilForWeapon[16]={1.3f,0.0f,0.1f,0.2f,0.0f,0.0f,0.0f,1.2f,0.8f,0.5f,1.5f,1.0f,0.9f,0.7f,0.0f,0.1f};
AttackType attackTypeForWeapon[16]={
    AttackType_Projectile, AttackType_EnergyBeam,  AttackType_ProjectileNeedle,     AttackType_Projectile,
    AttackType_EnergyBeam, AttackType_MeleeEnergy, AttackType_Projectile,           AttackType_Projectile,
    AttackType_Magnetic,   AttackType_Projectile,  AttackType_ProjectileEnergyBeam, AttackType_ProjectileLaunched,
    AttackType_Projectile, AttackType_Projectile,  AttackType_EnergyBeam,           AttackType_Tranq
};

float reloadTime[16]={1.0f,0.8f,1.0f,1.2f,0.8f,0.8f,0.8f,1.3f,1.5f,0.8f,0.8f,1.0f,1.5f,2.0f,0.8f,0.8f};

// public class WeaponFire : MonoBehaviour {
// 	// External references, required
//     public GameObject impactEffect;
// 	public GameObject noDamageIndicator;
//     public Camera playerCamera; // assign in the editor
//     public GameObject playerCapsule;
//     public EnergyOverloadButton energoverButton;
//     public EnergyHeatTickManager energheatMgr;
//     public Animator anim; // assign in the editor
// 	public Animator rapieranim; // assign in the editor
// 	public GameObject muzFlashMK3;
// 	public GameObject muzSmokeMK3;
// 	public GameObject muzFlashBlaster;
// 	public GameObject muzFlashDartgun;
// 	public GameObject muzFlashFlechette;
// 	public GameObject muzSmokeFlechette;
// 	public GameObject muzFlashIonBeam;
// 	public GameObject muzSmokeMagnum;
// 	public GameObject muzFlashMagnum;
// 	public GameObject muzSmokePistol;
// 	public GameObject muzFlashPistol;
// 	public GameObject muzFlashMagpulse;
// 	public GameObject muzFlashPlasma;
// 	public GameObject muzSmokeRailgun;
// 	public GameObject muzFlashRailgun;
// 	public GameObject muzSmokeRiotgun;
// 	public GameObject muzFlashRiotgun;
// 	public GameObject muzSmokeSkorpion;
// 	public GameObject muzFlashSkorpion;
// 	public GameObject muzFlashSparq;
// 	public GameObject muzFlashStungun;
// 	public float[] fogBaseDensityForLevel;
// 	public Color[] fogColorForLevel;
// 	public SSMS.SSMSGlobalFog ssmsGlobalFog;
// 	public Transform reloadContainer; // Recoil the weapon view models
//     public bool overloadEnabled; // save
// 	public float reloadFinished; // save
// 	public float lerpStartTime; // save
// 	public float reloadLerpValue; // save
// 	public int fogFac;
// 
//     DamageData damageData;
//     float waitTilNextFire = 0f; // save
//     float sparqSetting = 50f; // save
//     float ionSetting = 100f; // save
//     float blasterSetting = 15f; // save
//     float plasmaSetting = 40f; // save
//     float stungunSetting = 20f;  // save
// 	Vector3 reloadContainerHome;
// 	bool recoiling; // save
// 	int lerpUp = 0; // 0 = not lerping, 1 = up, 2 = dn
// 	float justFired; // save
// 	float energySliderClickedTime; // save
// 	float cyberWeaponAttackFinished; // save
// 	float reloadContainerDropAmount = 0.66f;
// 	float targetY; // save
// 
// 	// Internal references
//     private float hitOffset = 0f;
//     private float verticalOffset = -0.2f; // For laser beams
//     private float fireDistance = 200f;
//     private float hitscanDistance = 200f;
//     private float meleescanDistance = 3.2f;
// 	private float overheatedPercent = 80f;
//     private float magpulseShotForce = 2.2f;
//     private float stungunShotForce = 2.2f;
//     private float railgunShotForce = 5f;
//     private float plasmaShotForce = 1.5f;
// 	private float inventoryModeViewRotateMax = 48f;
//     private float clipEnd;
//     private RaycastHit tempHit;
//     private Vector3 tempVec;
//     private HealthManager tempHM;
//     private float retval;
//     private float heatTickFinished;
//     private float heatTickTime = 0.50f;
// 	private Rigidbody playercapRbody;
// 	private float wepYRot;
// 	private static StringBuilder s1 = new StringBuilder();
// 
// 	// Not needed on Const as this only exists in one unique place on player.
// 	private float[] driftForWeapon = new float[16]{5f,0f,15f,50f,0f,0f,0f,8f,
// 												   3f,3f,3f,12f,10f,30f,0f,3f};
// 
// 	// Singleton instance
// 	public static WeaponFire a;
// 
// 	void Awake() {
// 		a = this;
// 	}
// 
//     void Start() {
//         damageData = new DamageData();
//         tempHit = new RaycastHit();
//         tempVec = (Vector3){0f, 0f, 0f);
//         heatTickFinished = Sys_Global.pauseRelativeTime + heatTickTime;
// 		reloadContainerHome = reloadContainer.localPosition;
// 
// 		// Set less than 30s before Sys_Global.pauseRelativeTime to guarantee we
// 		// don't immediately play action music.
// 		justFired = (Sys_Global.pauseRelativeTime - 31f);
// 
// 		energySliderClickedTime = Sys_Global.pauseRelativeTime;
// 		playercapRbody = playerCapsule.GetComponent<Rigidbody>();
// 		cyberWeaponAttackFinished = Sys_Global.pauseRelativeTime;
// 		wepYRot = 0f;
// 		sparqSetting = 50f;
// 		ionSetting = 100f;
// 		blasterSetting = 15f;
// 		plasmaSetting = 40f;
// 		stungunSetting = 20f;
// 		reloadLerpValue = 0;
// 		reloadFinished = Sys_Global.pauseRelativeTime;
// 		lerpStartTime = Sys_Global.pauseRelativeTime;
// 		fogFac = 0;
//     }
// 
//     void GetWeaponData(int index) {
//         if (index < 0) return;
// 		if (WeaponCurrent.a.weaponCurrent < 0) return;
// 
//         if (inventoryPlayer1.wepLoadedWithAlternate[WeaponCurrent.a.weaponCurrent]) {
// 			// Alternate (2)
//             damageData.damage = Const.a.damagePerHitForWeapon2[index];
//             damageData.delayBetweenShots = 
// 				Const.a.delayBetweenShotsForWeapon2[index];
// 
//             damageData.penetration = Const.a.penetrationForWeapon2[index];
//             damageData.offense = Const.a.offenseForWeapon2[index];
//         } else {
// 			// Normal
//             damageData.damage = Const.a.damagePerHitForWeapon[index];
//             damageData.delayBetweenShots =
// 				Const.a.delayBetweenShotsForWeapon[index];
// 
//             damageData.penetration = Const.a.penetrationForWeapon[index];
//             damageData.offense = Const.a.offenseForWeapon[index];
//         }
// 
//         damageData.damageOverload = Const.a.damageOverloadForWeapon[index];
//         damageData.energyDrainLow = Const.a.energyDrainLowForWeapon[index];
//         damageData.energyDrainHi = Const.a.energyDrainHiForWeapon[index];
//         damageData.energyDrainOver = Const.a.energyDrainOverloadForWeapon[index];
//         damageData.attackType = Const.a.attackTypeForWeapon[index];
//         damageData.berserkActive = (Utils.CheckFlags(instances[PLAYER1].patchActive,PlayerPatch.PATCH_BERSERK));
//     }
// 
//     public static int Get16WeaponIndexFromConstIndex(int index) {
//         switch (index) {
//             case 36: return 0; // Mark3 Assault Rifle
//             case 37: return 1; // ER-90 Blaster
//             case 38: return 2; // SV-23 Dartgun
//             case 39: return 3; // AM-27 Flechette
//             case 40: return 4; // RW-45 Ion Beam
//             case 41: return 5; // TS-04 Laser Rapier
//             case 42: return 6; // Lead Pipe
//             case 43: return 7; // Magnum 2100
//             case 44: return 8; // SB-20 Magpulse
//             case 45: return 9; // ML-41 Pistol
//             case 46: return 10;// LG-XX Plasma Rifle
//             case 47: return 11;// MM-76 Railgun
//             case 48: return 12;// DC-05 Riotgun
//             case 49: return 13;// RF-07 Skorpion
//             case 50: return 14;// Sparq Beam
//             case 51: return 15;// DH-07 Stungun
//         }
//         return -1;
//     }
// 
//     bool CurrentWeaponUsesEnergy () {
//         if (WeaponCurrent.a.weaponIndex == 37 || WeaponCurrent.a.weaponIndex == 40 ||
// 			WeaponCurrent.a.weaponIndex == 46 || WeaponCurrent.a.weaponIndex == 50 ||
// 			WeaponCurrent.a.weaponIndex == 51)
// 			return true;
//         return false;
//     }
// 
//     bool WeaponsHaveAnyHeat() {
// 		if (WeaponCurrent.a.redbull) return false;
// 		if (inventoryPlayer1.currentEnergyWeaponHeat[0] > 0f) return true;
// 		if (inventoryPlayer1.currentEnergyWeaponHeat[1] > 0f) return true;
// 		if (inventoryPlayer1.currentEnergyWeaponHeat[2] > 0f) return true;
// 		if (inventoryPlayer1.currentEnergyWeaponHeat[3] > 0f) return true;
// 		if (inventoryPlayer1.currentEnergyWeaponHeat[4] > 0f) return true;
// 		if (inventoryPlayer1.currentEnergyWeaponHeat[5] > 0f) return true;
// 		if (inventoryPlayer1.currentEnergyWeaponHeat[6] > 0f) return true;
//         return false;
//     }
// 
//     void HeatBleedOff() {
//         if (heatTickFinished < Sys_Global.pauseRelativeTime) {
// 			fogFac--;
// 			if (fogFac < 0.0f) fogFac = 0.0f;
// 			if (WeaponsHaveAnyHeat() || CurrentWeaponUsesEnergy()) {
// 				inventoryPlayer1.currentEnergyWeaponHeat[0] -= 10f; if (inventoryPlayer1.currentEnergyWeaponHeat[0] <= 0.0f) inventoryPlayer1.currentEnergyWeaponHeat[0] = 0.0f;
// 				inventoryPlayer1.currentEnergyWeaponHeat[1] -= 10f; if (inventoryPlayer1.currentEnergyWeaponHeat[1] <= 0.0f) inventoryPlayer1.currentEnergyWeaponHeat[1] = 0.0f;
// 				inventoryPlayer1.currentEnergyWeaponHeat[2] -= 10f; if (inventoryPlayer1.currentEnergyWeaponHeat[2] <= 0.0f) inventoryPlayer1.currentEnergyWeaponHeat[2] = 0.0f;
// 				inventoryPlayer1.currentEnergyWeaponHeat[3] -= 10f; if (inventoryPlayer1.currentEnergyWeaponHeat[3] <= 0.0f) inventoryPlayer1.currentEnergyWeaponHeat[3] = 0.0f;
// 				inventoryPlayer1.currentEnergyWeaponHeat[4] -= 10f; if (inventoryPlayer1.currentEnergyWeaponHeat[4] <= 0.0f) inventoryPlayer1.currentEnergyWeaponHeat[4] = 0.0f;
// 				inventoryPlayer1.currentEnergyWeaponHeat[5] -= 10f; if (inventoryPlayer1.currentEnergyWeaponHeat[5] <= 0.0f) inventoryPlayer1.currentEnergyWeaponHeat[5] = 0.0f;
// 				inventoryPlayer1.currentEnergyWeaponHeat[6] -= 10f; if (inventoryPlayer1.currentEnergyWeaponHeat[6] <= 0.0f) inventoryPlayer1.currentEnergyWeaponHeat[6] = 0.0f;
// 				if (CurrentWeaponUsesEnergy()) energheatMgr.HeatBleed(inventoryPlayer1.currentEnergyWeaponHeat[WeaponCurrent.a.weaponCurrent]); // update hud heat ticks if current weapon uses energy
// 			}
// 			
//             heatTickFinished = Sys_Global.pauseRelativeTime + heatTickTime;
//         }
//     }
// 
// 	public void Recoil (int i) {
// 		float strength = Const.a.recoilForWeapon[i];
// 		//DualLog("Recoil from gun index: "+i.ToString()+" with strength of " +strength.ToString());
// 		if (strength <= 0f) return;
// 		if (instances[PLAYER1].fatigue > 80.0f) strength = strength * 2f;
// 		strength = strength * 0.25f;
// 		Vector3 wepJoltPosition = (Vector3){reloadContainer.localPosition.x - (strength * 0.5f * random_range(-1.0f,1.0f)), reloadContainer.localPosition.y, (reloadContainerHome.z - strength));
// 		if (wepJoltPosition.x > 999f) wepJoltPosition.x = 0;
// 		if (wepJoltPosition.y > 999f) wepJoltPosition.y = 0;
// 		if (wepJoltPosition.z > 999f) wepJoltPosition.z = 0;
// 		reloadContainer.localPosition = wepJoltPosition;
// 		recoiling = true;
// 	}
// 
// 	void WeaponLerpGetTargetUp() {
// 		// Percentage of this half of the trip.
// 		reloadLerpValue = (0.5f - (1 - reloadLerpValue))/0.5f;
// 		targetY = (-1 * reloadContainerDropAmount * (1 - reloadLerpValue));
// 		if (targetY > reloadContainerHome.y) targetY = reloadContainerHome.y;
// 	}
// 
// 	void WeaponLerpGetTargetDown() {
// 		// Percentage of this half of the trip.
// 		reloadLerpValue = reloadLerpValue/0.5f;
// 		targetY = reloadContainerHome.y - reloadContainerDropAmount;
// 		targetY *= reloadLerpValue;
// 	}
// 
// 	void Recoiling() {
// 		if (!recoiling) return;
// 
// 		float x = reloadContainer.localPosition.x; // side to side
// 		float z = reloadContainer.localPosition.z; // forward and back
// 		z = lerp(z,reloadContainerHome.z,Time.deltaTime);
// 		x = lerp(x,reloadContainerHome.x,Time.deltaTime);
// 		reloadContainer.localPosition = 
// 			(Vector3){x,reloadContainer.localPosition.y,z);
// 	}
// 
// 	void UpdateWeaponReloadDip() {
// 		// Move weapon transform up/down for reload "animation" & weapon swap.
// 		int i = Get16WeaponIndexFromConstIndex(WeaponCurrent.a.weaponIndex);
// 		if (i < 0 || i > 15) i = 0;
// 		if (reloadFinished > Sys_Global.pauseRelativeTime) {
// 			float elapsed = (Sys_Global.pauseRelativeTime - lerpStartTime);
// 
// 			// Percent towards goal time total (both halves of the action).
// 			reloadLerpValue = (elapsed/(reloadFinished-lerpStartTime));//Const.a.reloadTime[i]);
// 			if (reloadLerpValue >= 0.5f) { // Flip back to lerp up.
// 				lerpUp = 1;
// 				WeaponLerpGetTargetUp();
// 				CompleteWeaponChange();
// 			} else {
// 				lerpUp = 2;
// 				WeaponLerpGetTargetDown();
// 			}
// 
// 			vclamp(targetY, -100f, 100f);
// 			Vector3 pos = (Vector3){reloadContainer.localPosition.x,
// 									  targetY,
// 									  reloadContainer.localPosition.z);
// 
// 			reloadContainer.localPosition = pos;
// 		} else {
// 			lerpUp = 0;
// 			Vector3 pos = (Vector3){reloadContainer.localPosition.x,
// 									  reloadContainerHome.y,
// 									  reloadContainer.localPosition.z);
// 
// 			reloadContainer.localPosition = pos;
// 		}
// 	}
// 
//     void Update() {
// 		if (Sys_Global.gamePaused) return;
// 		if (Sys_Global.menuActive) return;
// 
// 		// Slowly cool off any weapons that have been heated from firing
// 		HeatBleedOff();
// 		if (fogFac > 255) fogFac = 255;
// 		ssmsGlobalFog.fogDensity = 0.451f * (fogBaseDensityForLevel[LevelManager.a.currentLevel] + ((((float)fogFac)/255f) * fogBaseDensityForLevel[LevelManager.a.currentLevel]));
// 		ssmsGlobalFog.fogColor = ssmsGlobalFog.fogTint = fogColorForLevel[LevelManager.a.currentLevel];
// 		UpdateWeaponReloadDip();
// 		RotateViewWeapon();
// 		Recoiling();
// 		CheckAttackInput();
// 		CheckReloadInput();
// 		CheckAmmoChangeInput();
//     }
// 
// 	public void CompleteWeaponChange() {
// 		if (WeaponCurrent.a.weaponCurrentPending == -1) return;
// 
// 		// Set current weapon 7 slot
// 		WeaponCurrent.a.weaponCurrent = WeaponCurrent.a.weaponCurrentPending;
//         if (CurrentWeaponUsesEnergy()) {
// 			// Update hud heat ticks if current weapon uses energy
// 			int iC = WeaponCurrent.a.weaponCurrent;
// 			energheatMgr.HeatBleed(inventoryPlayer1.currentEnergyWeaponHeat[iC]);
// 		}
// 
// 		// Set current weapon inventory lookup index
// 		WeaponCurrent.a.weaponIndex = WeaponCurrent.a.weaponIndexPending;
// 
// 		// Reset pending indices now that transition is done
// 		WeaponCurrent.a.weaponCurrentPending = -1;
// 		WeaponCurrent.a.weaponIndexPending = -1;
// 
// 		// Update the ammo icons.
// 		int ind = WeaponCurrent.a.weaponIndex;
// 		bool alt = false;
// 		if (ind >= 0 && ind < 16) alt = inventoryPlayer1.wepLoadedWithAlternate[ind];
// 		Sys_UI.SetAmmoIcons(ind,alt);
// 		Sys_UI.SetWepInfo(WeaponCurrent.a.weaponIndex);
// 		WeaponCurrent.a.UpdateWeaponViewModels();
// 	}
// 
// 	public void StartWeaponDip(float delay) {
// 		if (delay < 0) delay = 0;
// 		reloadFinished = Sys_Global.pauseRelativeTime + delay;
// 		lerpStartTime = Sys_Global.pauseRelativeTime;
// 	}
// 
// 	void RotateViewWeapon() {
// 		if (MouseLookScript.a.inventoryMode) {
// 			float screenHalf = (Screen.width/2f);
// 			float cursorX = MouseCursor.a.drawTexture.center.x;
// 			float distFromCenter = (cursorX - screenHalf);
// 			float percentRotated = (distFromCenter / screenHalf);
// 			wepYRot = percentRotated * inventoryModeViewRotateMax;
// 			reloadContainer.localRotation = Quaternion.Euler(0f,wepYRot,0f);
// 		} else {
// 			reloadContainer.localRotation = Quaternion.Euler(0f,0f,0f);
// 		}
// 	}
// 
// 	void CheckAttackInput() {
// 		// Check for other things that must capture and override clicks
// 		if (GetInput.a.Attack()) {
// 			if (MouseLookScript.a.vmailActive) {
// 				inventoryPlayer1.DeactivateVMail();
// 				MouseLookScript.a.vmailActive = false;
// 				waitTilNextFire = Sys_Global.pauseRelativeTime + 0.8f;
// 				return;
// 			}
// 
// 			if (MouseLookScript.a.inCyberSpace) {
// 				FireCyberWeapon();
// 				return;
// 			}
// 
// 			if (MouseLookScript.a.holdingObject
// 				&& !Sys_UI.mouseClickHeldOverGUI) { // !Just clicked
// 				if (!GUIState.a.isBlocking) {
// 					// Drop it
// 					MouseLookScript.a.DropHeldItem ();
// 					return;
// 				} else {
// 					MouseLookScript.a.AddItemToInventory(MouseLookScript.a.heldObjectIndex,MouseLookScript.a.heldObjectCustomIndex);
// 					MouseLookScript.a.ResetHeldItem();
// 					return;
// 				}
// 			}
// 		}
// 
// 		int wepdex = Get16WeaponIndexFromConstIndex(WeaponCurrent.a.weaponIndex);
// 		if (wepdex == -1) return; // No weapon.
// 		if (GUIState.a.isBlocking) return;
// 		if (MouseLookScript.a.holdingObject) return;
// 		if (Sys_UI.mouseClickHeldOverGUI) return;
// 
// 		StartNormalAttack(wepdex);
// 	}
// 
// 	public void StartNormalAttack(int wep16Index) {
// 		if (wep16Index < 0 || wep16Index > 15) return;
// 
// 		GetWeaponData(wep16Index);
// 		if (GetInput.a.Attack()
// 			&& waitTilNextFire < Sys_Global.pauseRelativeTime
// 			&& (Sys_Global.pauseRelativeTime - energySliderClickedTime) > 0.1f
// 			&& reloadFinished < Sys_Global.pauseRelativeTime) {
// 
// 			StartCoroutine(CheckUIStateAndAttack(wep16Index));
// 		}
// 	}
// 
// 	IEnumerator CheckUIStateAndAttack(int wepdex) {
// 		yield return null; // Ensure next frame
// 
// 		if (GUIState.a.isBlocking) yield break;
// 		if (MouseLookScript.a.holdingObject) yield break;
// 		if (Sys_UI.mouseClickHeldOverGUI) yield break;
// 		if (reloadFinished >= Sys_Global.pauseRelativeTime) yield break;
// 		if (waitTilNextFire >= Sys_Global.pauseRelativeTime) yield break;
// 		if (wepdex < 0 || wepdex > 15) yield break;
// 		if (Automap.a.inFullMap) yield break;
// 
// 		justFired = Sys_Global.pauseRelativeTime; // set justFired so that Music.cs can see it and play corresponding music in a little bit from now or keep playing action music
// 		// Check weapon type and check ammo before firing
// 		switch (wepdex) {
// 			case 1: goto case 15;
// 			case 4: goto case 15;
// 			case 5: goto case 6;
// 			case 6:
// 				// Pipe or Laser Rapier, attack without prejudice.
// 				// isSilent == false here so play normal SFX.
// 				FireWeapon(wepdex, false); 
// 				break;
// 			case 10: goto case 15;
// 			case 14: goto case 15;
// 			case 15: 
// 				// Energy weapons so check energy level
// 				// Even if we have only 1 energy, we still fire with all we've got up to the energy level setting of course
// 				if (PlayerEnergy.a.energy > 0
// 					|| WeaponCurrent.a.bottomless
// 					|| WeaponCurrent.a.redbull) {
// 					if (inventoryPlayer1.currentEnergyWeaponHeat[WeaponCurrent.a.weaponCurrent] > overheatedPercent
// 						&& !WeaponCurrent.a.bottomless
// 						&& !WeaponCurrent.a.redbull) {
// 						Utils.PlayUIOneShotSavable(238); // noammo
// 						waitTilNextFire = Sys_Global.pauseRelativeTime + 0.8f;
// 						CenterStatusPrint(11);
// 					} else {
// 						FireWeapon(wepdex, false); // weapon index, isSilent == false so play normal SFX
// 					}
// 				} else {
// 					CenterStatusPrint(207); // Not enough energy to fire weapon.
// 				}
// 				break;
// 			default:
// 				// Uses normal ammo, check versus alternate or normal to see if we have ammo then fire
// 				if (inventoryPlayer1.wepLoadedWithAlternate[WeaponCurrent.a.weaponCurrent]) {
// 					if (WeaponCurrent.a.currentMagazineAmount2[WeaponCurrent.a.weaponCurrent] > 0
// 						|| WeaponCurrent.a.bottomless) {
// 						FireWeapon(wepdex, false); // weapon index, isSilent == false so play normal SFX
// 					} else {
// 						Utils.PlayUIOneShotSavable(238); // noammo
// 						waitTilNextFire = Sys_Global.pauseRelativeTime + 0.8f;
// 					}
// 				} else {
// 					if (WeaponCurrent.a.currentMagazineAmount[WeaponCurrent.a.weaponCurrent] > 0
// 						|| WeaponCurrent.a.bottomless) {
// 						FireWeapon(wepdex, false); // weapon index, isSilent == false so play normal SFX
// 					} else {
// 						Utils.PlayUIOneShotSavable(238); // noammo
// 						waitTilNextFire = Sys_Global.pauseRelativeTime + 0.8f;
// 					}
// 				}
// 				break;
// 		}
// 	}
// 
// 	void CheckReloadInput() {
// 		if (reloadFinished >= Sys_Global.pauseRelativeTime) return;
// 		if (!GetInput.a.Reload()) return;
// 
// 		if (Const.a.InputQuickReloadWeapons) {
// 			// Press reload once, to do both unload then reload
// 			WeaponCurrent.a.Reload();
// 			return;
// 		}
// 
// 		if (WeaponCurrent.a.weaponCurrent < 0) return;
// 
// 		// First press reload to unload, then press again to load
// 		int wep16index = WeaponFire.Get16WeaponIndexFromConstIndex(WeaponCurrent.a.weaponIndex);
// 		if (wep16index < 0) return;
// 
// 		if (inventoryPlayer1.wepLoadedWithAlternate[WeaponCurrent.a.weaponCurrent]) {
// 			if (WeaponCurrent.a.currentMagazineAmount2[WeaponCurrent.a.weaponCurrent] <= 0
// 				|| inventoryPlayer1.wepAmmoSecondary[wep16index] <= 0) { // True for no wepAmmoSecondary causes Reload to run and display no ammo message.
// 				WeaponCurrent.a.Reload();
// 			} else {
// 				WeaponCurrent.a.Unload(false);
// 			}
// 		} else {
// 			if (WeaponCurrent.a.currentMagazineAmount[WeaponCurrent.a.weaponCurrent] <= 0
// 				|| inventoryPlayer1.wepAmmo[wep16index] <= 0) { // True for no wepAmmo causes Reload to run and display no ammo message.
// 				WeaponCurrent.a.Reload();
// 			} else {
// 				WeaponCurrent.a.Unload(false);
// 			}
// 		}
// 	}
// 
// 	void CheckAmmoChangeInput() {
// 		if (reloadFinished >= Sys_Global.pauseRelativeTime) return;
// 		if (!GetInput.a.ChangeAmmoType()) return;
// 
// 		WeaponCurrent.a.ChangeAmmoType();
// 	}
// 
// 	public void FireCyberWeapon() {
// 		if (cyberWeaponAttackFinished < Sys_Global.pauseRelativeTime) {
// 			if (inventoryPlayer1.isPulserNotDrill) {
// 				if (inventoryPlayer1.hasSoft[1]) {
// 					// Fire pulser
// 					Const.a.shotsFired++;
// 					if (inventoryPlayer1.hasSoft[1]) FireCyberBeachball(true,railgunShotForce,492);
// 					Utils.PlayUIOneShotSavable(258); // wpulser
// 					cyberWeaponAttackFinished = Sys_Global.pauseRelativeTime + 0.08f;
// 				}
// 			} else {
// 				if (inventoryPlayer1.hasSoft[0]) {
// 					// Fire I.C.E. drill
// 					Const.a.shotsFired++;
// 					if (inventoryPlayer1.hasSoft[0]) FireCyberBeachball(false,plasmaShotForce,495);
// 					Utils.PlayUIOneShotSavable(241); // wdrill baby drill
// 					cyberWeaponAttackFinished = Sys_Global.pauseRelativeTime + 0.5f;
// 				}
// 			}
// 		}
// 	}
// 
// 	void FireCyberBeachball(bool isPulser, float shoveForce, int prefabID) {
//         // Create and hurl a beachball-like object.  On the developer commentary they said that the projectiles act
//         // like a beachball for collisions with enemies, but act like a baseball for walls/floor to prevent hitting corners
//         GameObject beachball = ConsoleEmulator.SpawnDynamicObject(prefabID,-1);
//         if (beachball != null) {
// 			damageData.damage = 10f * inventoryPlayer1.softVersions[0];
// 			if (isPulser) {
// 				// Cyberspace enemies don't have much health.
// 				damageData.damage = 1f + (0.25f * inventoryPlayer1.softVersions[1]);
// 			}
// 
//             damageData.owner = playerCapsule;
//             damageData.attackType = AttackType.ProjectileLaunched;
// 			if (!isPulser) damageData.attackType = AttackType.Drill;
//             beachball.GetComponent<ProjectileEffectImpact>().dd = damageData;
//             beachball.GetComponent<ProjectileEffectImpact>().host = playerCapsule;
//             beachball.instances[i].position = playerCamera.instances[i].position;
// 			MouseLookScript.a.SetCameraFocusPoint();
//             tempVec = MouseLookScript.a.cameraFocusPoint - playerCamera.instances[i].position;
//             beachball.transform.forward = tempVec.normalized;
//             beachball.SetActive(true);
//             Vector3 shove = beachball.transform.forward * shoveForce;
//             beachball.GetComponent<Rigidbody>().velocity = Const.a.vectorZero; // prevent random variation from the last shot's velocity
//             beachball.GetComponent<Rigidbody>().AddForce(shove, ForceMode.Impulse);
//         }
// 	}
// 
//     // index is used to get recoil down at the bottom and pass along ref for damageData, otherwise the cases use WeaponCurrent.a.weaponIndex
//     void FireWeapon(int index, bool isSilent) {
// 		PlayerHealth.a.makingNoise = true;
// 		PlayerHealth.a.noiseFinished = Sys_Global.pauseRelativeTime + 0.5f;
// 		GameObject smoke = null;
//         switch (WeaponCurrent.a.weaponIndex) {
//             case 36:
//                 //Mark3 Assault Rifle
//                 if (!isSilent) Utils.PlayUIOneShotSavable(251); // wmarksman
//                 if (DidRayHit(index)) HitScanFire(index);
// 				muzFlashMK3.SetActive(true);
// 				smoke = Instantiate(muzSmokeMK3,muzFlashMK3.instances[i].position,Const.a.quaternionIdentity) as GameObject;
// 				smoke.transform.parent = reloadContainer;
// 				smoke.SetActive(true);
// 				fogFac += 2;
//                 break;
//             case 37:
//                 //ER-90 Blaster
// 				blasterSetting = WeaponCurrent.a.weaponEnergySetting[WeaponCurrent.a.weaponCurrent];
// 				//DualLog("Blaster fired with energy setting of " + blasterSetting.ToString());
// 				if (!isSilent) Utils.PlayUIOneShotSavable(239); // wblaster
// 				if (DidRayHit(index)) HitScanFire(index);
// 				muzFlashBlaster.SetActive(true);
//                 if (overloadEnabled) {
//                     inventoryPlayer1.currentEnergyWeaponHeat[WeaponCurrent.a.weaponCurrent] = 100f;
//                 } else {
//                     inventoryPlayer1.currentEnergyWeaponHeat[WeaponCurrent.a.weaponCurrent] += blasterSetting;
// 					if (inventoryPlayer1.currentEnergyWeaponHeat[WeaponCurrent.a.weaponCurrent] > 100f) inventoryPlayer1.currentEnergyWeaponHeat[WeaponCurrent.a.weaponCurrent] = 100f; // cap it
//                 }
//                 break;
//             case 38:
//                 //SV-23 Dartgun
//                 if (!isSilent) Utils.PlayUIOneShotSavable(240); // wdartgun
//                 if (DidRayHit(index)) HitScanFire(index);
// 				muzFlashDartgun.SetActive(true);
//                 break;
//             case 39:
//                 //AM-27 Flechette
//                 if (!isSilent) Utils.PlayUIOneShotSavable(243); // wflechette
//                 if (DidRayHit(index)) HitScanFire(index);
// 				muzFlashFlechette.SetActive(true);
// 				smoke = Instantiate(muzSmokeFlechette,muzFlashFlechette.instances[i].position,Const.a.quaternionIdentity) as GameObject;
// 				smoke.transform.parent = reloadContainer;
// 				smoke.SetActive(true);
// 				fogFac += 1;
// 				break;
//             case 40:
//                 //RW-45 Ion Beam
// 				ionSetting = WeaponCurrent.a.weaponEnergySetting[WeaponCurrent.a.weaponCurrent];
// 				//DualLog("Ion rifle fired with energy setting of " + ionSetting.ToString());
//                 if (!isSilent) Utils.PlayUIOneShotSavable(245); // wion
//                 if (DidRayHit(index)) HitScanFire(index);
// 				muzFlashIonBeam.SetActive(true);
//                 if (overloadEnabled) {
//                     inventoryPlayer1.currentEnergyWeaponHeat[WeaponCurrent.a.weaponCurrent] = 100f;
//                 } else {
//                     inventoryPlayer1.currentEnergyWeaponHeat[WeaponCurrent.a.weaponCurrent] += ionSetting;
// 					if (inventoryPlayer1.currentEnergyWeaponHeat[WeaponCurrent.a.weaponCurrent] > 100f) inventoryPlayer1.currentEnergyWeaponHeat[WeaponCurrent.a.weaponCurrent] = 100f; // cap it
//                 }
//                 break;
//             case 41:
//                 //TS-04 Laser Rapier
//                 FireRapier(index, isSilent);
//                 break;
//             case 42:
//                 //Lead Pipe
//                 FirePipe(index, isSilent);
//                 break;
//             case 43:
//                 //Magnum 2100
//                 if (!isSilent) Utils.PlayUIOneShotSavable(249); // wmagnum
//                 if (DidRayHit(index)) HitScanFire(index);
// 				muzFlashMagnum.SetActive(true);
// 				smoke = Instantiate(muzSmokeMagnum,muzFlashMagnum.instances[i].position,Const.a.quaternionIdentity) as GameObject;
// 				smoke.transform.parent = reloadContainer;
// 				smoke.SetActive(true);
// 				fogFac += 3;
//                 break;
//             case 44:
//                 //SB-20 Magpulse
//                 if (!isSilent) Utils.PlayUIOneShotSavable(250); // wmagpulse
//                 FireMagpulse(index);
// 				muzFlashMagpulse.SetActive(true);
//                 break;
//             case 45:
//                 //ML-41 Pistol
//                 if (!isSilent) Utils.PlayUIOneShotSavable(255); // wpistol
//                 if (DidRayHit(index)) HitScanFire(index);
// 				muzFlashPistol.SetActive(true);
// 				smoke = Instantiate(muzSmokePistol,muzFlashPistol.instances[i].position,Const.a.quaternionIdentity) as GameObject;
// 				smoke.transform.parent = reloadContainer;
// 				smoke.SetActive(true);
// 				fogFac += 1;
//                 break;
//             case 46:
//                 //LG-XX Plasma Rifle
// 				plasmaSetting = WeaponCurrent.a.weaponEnergySetting[WeaponCurrent.a.weaponCurrent];
// 				//DualLog("Plasma rifle fired with energy setting of " + plasmaSetting.ToString());
//                 if (!isSilent) Utils.PlayUIOneShotSavable(257); // wplasma
//                 FirePlasma(index);
// 				muzFlashPlasma.SetActive(true);
//                 if (overloadEnabled) {
//                     inventoryPlayer1.currentEnergyWeaponHeat[WeaponCurrent.a.weaponCurrent] = 100f;
//                 } else {
//                     inventoryPlayer1.currentEnergyWeaponHeat[WeaponCurrent.a.weaponCurrent] += plasmaSetting;
// 					if (inventoryPlayer1.currentEnergyWeaponHeat[WeaponCurrent.a.weaponCurrent] > 100f) inventoryPlayer1.currentEnergyWeaponHeat[WeaponCurrent.a.weaponCurrent] = 100f; // cap it
//                 }
//                 break;
//             case 47:
//                 //MM-76 Railgun
//                 if (!isSilent) Utils.PlayUIOneShotSavable(259); // wrailgun
//                 FireRailgun(index);
// 				muzFlashRailgun.SetActive(true);
// 				smoke = Instantiate(muzSmokeRailgun,muzFlashRailgun.instances[i].position,Const.a.quaternionIdentity) as GameObject;
// 				smoke.transform.parent = reloadContainer;
// 				smoke.SetActive(true);
// 				fogFac += 2;
//                 break;
//             case 48:
//                 //DC-05 Riotgun
//                 if (!isSilent) Utils.PlayUIOneShotSavable(262); // wriotgun
//                 if (DidRayHit(index)) HitScanFire(index);
// 				muzFlashRiotgun.SetActive(true);
// 				smoke = Instantiate(muzSmokeRiotgun,muzFlashRiotgun.instances[i].position,Const.a.quaternionIdentity) as GameObject;
// 				smoke.transform.parent = reloadContainer;
// 				smoke.SetActive(true);
// 				fogFac += 4;
//                 break;
//             case 49:
//                 //RF-07 Skorpion
//                 if (!isSilent) Utils.PlayUIOneShotSavable(263); // wskorpion
//                 if (DidRayHit(index)) HitScanFire(index);
// 				muzFlashSkorpion.SetActive(true);
// 				smoke = Instantiate(muzSmokeSkorpion,muzFlashSkorpion.instances[i].position,Const.a.quaternionIdentity) as GameObject;
// 				smoke.transform.parent = reloadContainer;
// 				smoke.SetActive(true);
// 				fogFac += 2;
//                 break;
//             case 50:
//                 //Sparq Beam
// 				sparqSetting = WeaponCurrent.a.weaponEnergySetting[WeaponCurrent.a.weaponCurrent];
//                 if (!isSilent) Utils.PlayUIOneShotSavable(264); // wsparq
//                 if (DidRayHit(index)) HitScanFire(index);
// 				muzFlashSparq.SetActive(true);
//                 if (overloadEnabled) {
//                     inventoryPlayer1.currentEnergyWeaponHeat[WeaponCurrent.a.weaponCurrent] = 100f;
//                 } else {
//                     inventoryPlayer1.currentEnergyWeaponHeat[WeaponCurrent.a.weaponCurrent] += sparqSetting;
// 					if (inventoryPlayer1.currentEnergyWeaponHeat[WeaponCurrent.a.weaponCurrent] > 100f) inventoryPlayer1.currentEnergyWeaponHeat[WeaponCurrent.a.weaponCurrent] = 100f; // cap it
//                 }
//                 break;
//             case 51:
//                 //DH-07 Stungun
// 				stungunSetting = WeaponCurrent.a.weaponEnergySetting[WeaponCurrent.a.weaponCurrent];
//                 if (!isSilent) Utils.PlayUIOneShotSavable(265); // wstungun
//                 FireStungun(index);
// 				muzFlashStungun.SetActive(true);
//                 if (overloadEnabled) {
//                     inventoryPlayer1.currentEnergyWeaponHeat[WeaponCurrent.a.weaponCurrent] = 100f;
//                 } else {
//                     inventoryPlayer1.currentEnergyWeaponHeat[WeaponCurrent.a.weaponCurrent] += stungunSetting;
// 					if (inventoryPlayer1.currentEnergyWeaponHeat[WeaponCurrent.a.weaponCurrent] > 100f) inventoryPlayer1.currentEnergyWeaponHeat[WeaponCurrent.a.weaponCurrent] = 100f; // cap it
//                 }
//                 break;
//         }
// 
//         // TAKE AMMO
//         // no weapons subtract more than 1 at a time in a shot except for energy weapons, subtracting 1
//         // Check weapon type before subtracting ammo or energy
//         if (index == 5 || index == 6) {
//             // Melee don't count towards Const.a.shotsFired
//             // Pipe or Laser Rapier
//             // ammo is already 0, do nothing.  This is here to prevent subtracting ammo on the first slot of .wepAmmo[index] on the last else clause below
//         } else {
//             // Energy weapons so check energy level
//             if (index == 1 || index == 4 || index == 10 || index == 14 || index == 15) {
//                 if (overloadEnabled) {
//                     energoverButton.OverloadFired();
//                     if (!WeaponCurrent.a.bottomless && !WeaponCurrent.a.redbull) {
// 						TakeEnergy(Const.a.energyDrainOverloadForWeapon[index]); //take large amount
// 						if (BiomonitorGraphSystem.a != null) {
// 							BiomonitorEnergyPulse(Const.a.energyDrainOverloadForWeapon[index]);
// 						}
// 					}
//                 } else {
//                     float takeEnerg = (WeaponCurrent.a.weaponEnergySetting[WeaponCurrent.a.weaponCurrent] / 100f) * (Const.a.energyDrainHiForWeapon[index] - Const.a.energyDrainLowForWeapon[index]);
//                     if (!WeaponCurrent.a.bottomless && !WeaponCurrent.a.redbull) {
// 						TakeEnergy(takeEnerg);
// 						if (BiomonitorGraphSystem.a != null) {
// 							BiomonitorEnergyPulse(takeEnerg);
// 						}
// 					}
//                 }
//             } else {
//                 if (inventoryPlayer1.wepLoadedWithAlternate[WeaponCurrent.a.weaponCurrent]) {
//                     if (!WeaponCurrent.a.bottomless) WeaponCurrent.a.currentMagazineAmount2[WeaponCurrent.a.weaponCurrent]--; // Take ammo away
//                 } else {
//                     if (!WeaponCurrent.a.bottomless) WeaponCurrent.a.currentMagazineAmount[WeaponCurrent.a.weaponCurrent]--; // Take ammo away
//                 }
//             }
//             
//             Const.a.shotsFired++;
//         }
// 
// 		Recoil(index);
//         if (inventoryPlayer1.wepLoadedWithAlternate[WeaponCurrent.a.weaponCurrent]
// 			|| overloadEnabled) {
// 
//             overloadEnabled = false;
//             waitTilNextFire = Sys_Global.pauseRelativeTime
// 							  + Const.a.delayBetweenShotsForWeapon2[index];
//         } else {
//             waitTilNextFire = Sys_Global.pauseRelativeTime
// 							  + Const.a.delayBetweenShotsForWeapon[index];
//         }
// 
// 		inventoryPlayer1.UpdateAmmoText();
//     }
// 
//     bool DidRayHit(int wep16Index) {
// 		tempHM = null;
//         tempHit = new RaycastHit();
// 		tempVec = MouseCursor.a.GetCursorScreenPointForRay();
// 		tempVec.x += random_range(-driftForWeapon[wep16Index],
// 											  driftForWeapon[wep16Index]);
// 
// 		tempVec.y += random_range(-driftForWeapon[wep16Index],
// 											  driftForWeapon[wep16Index]);
// 
//         if (Raycast(playerCamera.ScreenPointToRay(tempVec),out tempHit,
// 							fireDistance,Const.a.layerMaskPlayerAttack)) {
// 
// 			tempHM = Utils.GetMainHealthManager(tempHit);
//             return true;
//         }
// 
//         return false;
//     }
// 
// 	void CreateStandardImpactMarks(int wep16index) {
// 		// Don't create bullet holes on objects that move
// 		if (tempHit.collider == null) return;
// 		if (tempHit.collider.transform.gameObject == null) return;
// 
// 		GameObject hitGO = tempHit.collider.transform.gameObject;
// 		if (hitGO.GetComponent<Rigidbody>() != null) return;
// 		if (hitGO.GetComponent<HealthManager>() != null) return; // don't create bullet holes on objects that die
// 		if (hitGO.GetComponent<Animator>() != null) return; // don't create bullet holes on objects that animate
// 		if (hitGO.GetComponent<Animation>() != null) return; // don't create bullet holes on objects that animate
// 		if (hitGO.GetComponent<Door>() != null) return; // don't create bullet holes on doors, makes them ghost and flicker through walls
// 
// 		// Add bullethole
// 		tempVec = tempHit.normal * 0.16f;
// 		GameObject holetype = Const.a.GetPrefab(522);
// 		switch(wep16index) {
// 			case 0:  holetype = Const.a.GetPrefab(518); break;
// 			case 1:  holetype = Const.a.GetPrefab(520); break;
// 			case 2:  holetype = Const.a.GetPrefab(522); break;
// 			case 3:  holetype = Const.a.GetPrefab(521); break;
// 			case 4:  holetype = Const.a.GetPrefab(519); break;
// 			case 5:  holetype = Const.a.GetPrefab(520); break;
// 			case 6:  holetype = Const.a.GetPrefab(522); break;
// 			case 7:  holetype = Const.a.GetPrefab(518); break;
// 			case 8:  holetype = Const.a.GetPrefab(519); break;
// 			case 9:  holetype = Const.a.GetPrefab(521); break;
// 			case 10: holetype = Const.a.GetPrefab(519); break;
// 			case 11: holetype = Const.a.GetPrefab(519); break;
// 			case 12: holetype = Const.a.GetPrefab(523); break;
// 			case 13: holetype = Const.a.GetPrefab(518); break;
// 			case 14: holetype = Const.a.GetPrefab(520); break;
// 			case 15: holetype = Const.a.GetPrefab(520); break;
// 		}
// 
// 		GameObject impactMark = (GameObject)Instantiate(holetype,
// 			(tempHit.point + tempVec),
// 			Quaternion.LookRotation(tempHit.normal*-1,Vector3.up),
// 			hitGO.transform);
// 
// 		Quaternion roll = impactMark.instances[i].rotation;
// 		roll *= Quaternion.Euler(0f,0f,random_range(0,3) * 90f);
// 		impactMark.instances[i].rotation = roll;
// 		GameObject dynamicObjectsContainer = LevelManager.a.GetCurrentDynamicContainer();
// 		impactMark.transform.parent = dynamicObjectsContainer.transform;
// 	}
// 
//     void CreateStandardImpactEffects() {
//         // Determine blood type of hit target and spawn corresponding blood particle effect from the Const.Pool
//         if (tempHM != null) {
//             GameObject impact = Const.a.GetImpactType(tempHM);
//             if (impact != null) {
//                 tempVec = tempHit.normal * hitOffset;
// 				impact.transform.SetPositionAndRotation(tempHit.point + tempVec,Quaternion.FromToRotation(Vector3.up, tempHit.normal));
//                 impact.SetActive(true);
//             }
//         } else {
//             // Allow for skipping adding sparks after special override impact effects per attack functions below
// 			GameObject impact = Const.a.GetObjectFromPool(PoolType.SparksSmall); //Didn't hit an object with a HealthManager script, use sparks
// 			if (impact != null) {
// 				tempVec = tempHit.normal * hitOffset;
// 				impact.transform.SetPositionAndRotation(tempHit.point + tempVec,Quaternion.FromToRotation(Vector3.up, tempHit.normal));
// 				impact.SetActive(true);
// 			}
//         }
//     }
// 
//     void CreateBeamImpactEffects(int wep16index) {
// 		int impactConstdex = 731; // Cyan for sparqbeam
// 		if (wep16index == 1) {
// 			impactConstdex = 739;  //Red laser for blaster
//         } else if (wep16index == 4) {
// 			impactConstdex = 740; // Yellow laser for ion
//         }
// 
//         GameObject impact = ConsoleEmulator.SpawnDynamicObject(impactConstdex);
// 		impact.transform.SetPositionAndRotation(tempHit.point,Quaternion.FromToRotation(Vector3.up, tempHit.normal));
// 		impact.SetActive(true);
//     }
// 
//     void CreateBeamEffects(int wep16index) {
//         int laserIndex = 405; // Turquoise/Pale-Teal for sparq
//         if (wep16index == 1) laserIndex = 406;  //Red laser for blaster
//         else  if (wep16index == 4) laserIndex = 407; // Yellow laser for ion
// 
// 		GameObject dynamicObjectsContainer = LevelManager.a.GetCurrentDynamicContainer();
// 		GameObject lasertracer = Instantiate(Const.a.GetPrefab(laserIndex),instances[i].position,Const.a.quaternionIdentity) as GameObject;
// 
// 		// Temporary object only, no need to save or mark as instantiated.
// 		if (lasertracer != null) {
// 			lasertracer.transform.SetParent(dynamicObjectsContainer.transform,true);
// 			tempVec = instances[i].position;
// 			tempVec.y += verticalOffset;
// 			lasertracer.GetComponent<LaserDrawing>().startPoint = tempVec;
// 			lasertracer.GetComponent<LaserDrawing>().endPoint = tempHit.point;
// 			lasertracer.SetActive(true);
// 		}
//     }
// 
// 	// dmg_min is Const.a.damagePerHitForWeapon[wep16Index], dmg_max is Const.a.damagePerHitForWeapon2[wep16Index]
// 	float DamageForPower(int wep16Index) {
// 	    float retval, dmg_min, dmg_max, ener_min, ener_max;
// 
//         // overload overrides current setting and uses overload damage
//         if (overloadEnabled) {
//             retval = Const.a.damageOverloadForWeapon[wep16Index];
//             return retval;
//         }
// 
// 		dmg_min = Const.a.damagePerHitForWeapon[wep16Index];
// 		dmg_max = Const.a.damagePerHitForWeapon2[wep16Index];
//         ener_min = Const.a.energyDrainLowForWeapon[wep16Index];
//         ener_max = Const.a.energyDrainHiForWeapon[wep16Index];
// 		// Calculates damage based on min and max values and applies a curve of the slopes based on the linear plotting of the slope from min at min to max at max...that makes sense right?
// 		// Right then, the beautifully ugly formula:
// 		retval = ((WeaponCurrent.a.weaponEnergySetting[WeaponCurrent.a.weaponCurrent]/100f)*((dmg_max/ener_max)-(dmg_min/ener_min)) + 3f) * (((WeaponCurrent.a.weaponEnergySetting[WeaponCurrent.a.weaponCurrent])/100f)*(ener_max-ener_min) + ener_min);
// 		//DualLog("returning DamageForPower of " + retval.ToString() + ", for wep16Index of " + wep16Index.ToString());
// 		return retval;
// 		// You gotta love maths!  There is a spreadsheet for this (.ods LibreOffice file format, found with src code) that shows the calculations to make this dmg curve. 
// 	}
// 
// 	// TargetID Instance
	void CreateTargetIDInstance(float dmgFinal, uint16_t npcIdx, float tranq) {
		 if (!ConstIndexIsNPC(instances[npcIdx].index) || instances[npcIdx].health <= 0f) return;
         if (!(inventoryPlayer1.hasHardware & HW_TID) || tranq > 0.0f) return;
		if (instances[npcIdx].linkedTargetID) return; // Let SendDamageReceive handle updates
/*
		float linkDistForTargID = TargetID.GetTargetIDTetherRange();
		bool showHealth = inventoryPlayer1.hasHardware[4] && inventoryPlayer1.hardwareVersion[4] > 2;
		bool showRange = inventoryPlayer1.hasHardware[4];
		bool showAttitude = inventoryPlayer1.hasHardware[4] && inventoryPlayer1.hardwareVersion[4] > 1;
		bool showName = inventoryPlayer1.hasHardware[4] && inventoryPlayer1.hardwareVersion[4] > 1;

		GameObject idFrame = Instantiate(Const.a.GetPrefab(736), instances[npcIdx].position, Const.a.quaternionIdentity) as GameObject;
		if (idFrame == null) return;

		TargetID tid = idFrame.GetComponent<TargetID>();
		if (tid == null) return;

		tid.parent = hm.transform;
		tid.linkedHM = hm;
		hm.linkedTargetID = tid;

		if (!inventoryPlayer1.hasHardware[4] || tranq > 0f || dmgFinal == 0f) {
			tid.currentText = tranq > 0f ? Sys_Text.stringTable[536] : (dmgFinal == 0f ? Sys_Text.stringTable[511] : "");
			tid.lifetime += tranq;
			tid.damageTimeFinished = vmax(Sys_Global.pauseRelativeTime + tranq,tid.damageTimeFinished + tranq);
			tid.lifetimeFinished = Sys_Global.pauseRelativeTime + tid.lifetime;
		} else {
			tid.currentText = ""; // Set by SendDamageReceive
			tid.lifetime = 9999999f;
			tid.lifetimeFinished = Sys_Global.pauseRelativeTime + tid.lifetime;
			tid.damageTime = 2.5f;
			if (tranq > 2.5f) tid.damageTime = tranq;
			tid.damageTimeFinished = Sys_Global.pauseRelativeTime + tid.damageTime;
		}

		// Center on what we just shot
		float yOfs = 0f;
		float xSize = 1.2f;
		float ySize = 2f;
		float textname_Ofs = 1.28f; // e.g. HOPPER5
		if (hm.aic != null) {
			switch(hm.aic.index) {
				case 0: yOfs = 0.5f; ySize = 1.0f; break; // Autobomb
				case 1: /* GOOD */ break; // Cyborg Assassin
				case 2: yOfs = -0.1f; ySize = 1.4f; xSize = 1.4f; textname_Ofs = 0.76f; break; // Avian Mutant
				case 3: /* GOOD */ break; // Exec-Bot
				case 4: /* GOOD */ break; // Cyborg Drone
				case 5: yOfs = 0.15f; ySize = 3f; xSize = 2.8f; textname_Ofs = 2.14f; break; // Cortex Reaver
				case 6: /* GOOD */ break; // Cyborg Warrior
				case 7: /* GOOD */ break; // Cyborg Enforcer
				case 8: yOfs = 0.05f; ySize = 2.3f; textname_Ofs = 1.48f;  break; // Cyborg Elite Guard
				case 9: ySize = 2.3f; textname_Ofs = 1.36f; break; // Cyborg of Edward Diego
				case 10: yOfs = -0.1f; ySize = 1.8f; xSize = 1.4f; textname_Ofs = 0.92f; break; // Sec-1 Bot
				case 11: yOfs = 0.04f; ySize = 2.4f; xSize = 2.4f; textname_Ofs = 1.51f; break; // Sec-2 Bot
				case 12: yOfs = -0.2f; ySize = 1.6f; xSize = 1.6f; textname_Ofs = 0.68f; break; // Maintenance Robot
				case 13: yOfs = 0.05f; ySize = 2.5f; xSize = 1.6f; textname_Ofs = 1.58f; break; // Mutant Cyborg
				case 14: yOfs = 0.5f; ySize = 2.3f; textname_Ofs = 2.5f; break; // Hopper
				case 15: /* GOOD */ break; // Humanoid Mutant
				case 16: yOfs = 0.12f; ySize = 0.8f; xSize = 1.8f; textname_Ofs = 0.7f; break; // Invisible Mutant
				case 17: /* GOOD */ break; // Virus Mutant
				case 18: yOfs = -0.22f; ySize = 1.5f; textname_Ofs = 0.63f; break; // Servbot
				case 19: yOfs = 0.2f; ySize = 1f; xSize = 1.55f; textname_Ofs = 0.92f;  break; // Flier Bot
				case 20: ySize = 1.1f;  xSize = 1.1f; textname_Ofs = 0.74f; break; // Zero-G Mutant
				case 21: yOfs = -0.25f; ySize = 1.5f; textname_Ofs = 0.53f; xSize = 2f; break; // Gorilla Tiger Mutant
				case 22: yOfs = -0.7f; ySize = 1f; textname_Ofs = 0f; break; // Repair Bot
				case 23: yOfs = -0.24f; ySize = 1.4f; textname_Ofs = 0.58f; break; // Plant Mutant
			}
		}

		// Set after setting textname_Ofs so all 3 can adapt to size.
		float textdmg_Ofs = textname_Ofs - 0.18f; // def: 1.1f, e.g. MINOR
		float textnum_Ofs = textname_Ofs - 0.09f; // def: 1.19f, e.g. 3.8M Idle

		idFrame.instances[i].position = hm.instances[i].position;
		idFrame.SetActive(true);
		tid.linkedHM = hm;
		hm.linkedTargetID = tid;
		tid.partSys.Play();

		//tid.partSys.
		ParticleSystemRenderer rd =
			tid.partSys.GetComponent<ParticleSystemRenderer>();

		rd.pivot = (Vector3){0f,yOfs,0f);

		ParticleSystem.MainModule pm = tid.partSys.main;
		pm.startSizeX = xSize;
		pm.startSizeY = ySize;
		pm.startSizeZ = xSize;

		RectTransform rt = tid.nameText.GetComponent<RectTransform>();
		rt.anchoredPosition = new Vector2(0f,textname_Ofs);

		RectTransform rtnums = tid.secondaryText.GetComponent<RectTransform>();
		rtnums.anchoredPosition = new Vector2(0f,textnum_Ofs);

		RectTransform rtdmg = tid.text.GetComponent<RectTransform>();
		rtdmg.anchoredPosition = new Vector2(0f,textdmg_Ofs);

		tid.playerCapsuleTransform = playerCapsule.transform;
		tid.playerLinkDistance = linkDistForTargID;
		tid.displayRange = showRange;
		tid.displayHealth = showHealth;
		tid.displayAttitude = showAttitude;
		tid.displayName = showName;
		tid.linkedHM.aic.hasTargetIDAttached = true;*/
	}
// 
//     // WEAPON FIRING CODE:
//     // ==============================================================================================================================
//     // Hitscan Weapons
//     //----------------------------------------------------------------------------------------------------------
//     // Guns and laser beams, used by most weapons
//     void HitScanFire(int wep16Index) {
//         damageData.other = tempHit.transform.gameObject;
// 		tempHM = Utils.GetMainHealthManager(tempHit);
// 		if (tempHM != null) {
// 			if (damageData.other != tempHM.gameObject) {
// 				damageData.other = tempHM.gameObject;
// 			}
// 		}
// 
//         if (wep16Index == 1 || wep16Index == 4 || wep16Index == 14) {
//             CreateBeamImpactEffects(wep16Index); // laser burst effect overrides standard blood spurts/robot sparks
//         } else {
//             CreateStandardImpactEffects(); // standard blood spurts/robot sparks
// 
// 			// the only exception
// 			if (wep16Index == 2 && inventoryPlayer1.wepLoadedWithAlternate[WeaponCurrent.a.weaponCurrent]) {
// 				damageData.attackType = AttackType.Tranq; // tranquilize the untranquil....yes
// 			}
//         }
// 
//         // Fill the damageData container
// 		// -------------------------------
// 		// Using tempHit.transform instead of tempHit.collider.transform to ensure we get overall NPC parent instead of its children.
// 		float tranq = -1f;
//         if (damageData.other.CompareTag("NPC")) {
//             damageData.isOtherNPC = true;
// 			if (damageData.attackType == AttackType.Tranq) {
// 				// Using tempHit.transform instead of tempHit.collider.transform to ensure we get overall NPC parent instead of its children.
// 				AIController taic = damageData.other.GetComponent<AIController>();
// 				if (taic !=null) {
// 					tranq = taic.Tranquilize(3f + random_range(0f,4f),false);
// 				}
// 			}
//         } else {
//             damageData.isOtherNPC = false;
// 			if (damageData.other.CompareTag("Geometry")) {
// 				CreateStandardImpactMarks(wep16Index);
// 			}
//         }
//         damageData.hit = tempHit;
// 		damageData.attacknormal = MouseCursor.a.GetCursorScreenPointForRay();
//         damageData.attacknormal = playerCamera.ScreenPointToRay(damageData.attacknormal).direction;
//         if (inventoryPlayer1.wepLoadedWithAlternate[WeaponCurrent.a.weaponCurrent]) {
//             damageData.damage = Const.a.damagePerHitForWeapon2[wep16Index];
// 			damageData.offense = Const.a.offenseForWeapon2[wep16Index];
// 			damageData.penetration = Const.a.penetrationForWeapon2[wep16Index];
//         } else {
// 			if (CurrentWeaponUsesEnergy()) {
//                 damageData.damage = DamageForPower(wep16Index);
// 			} else {
// 				damageData.damage = Const.a.damagePerHitForWeapon[wep16Index];
// 			}
// 			damageData.offense = Const.a.offenseForWeapon[wep16Index];
// 			damageData.penetration = Const.a.penetrationForWeapon[wep16Index];
//         }
//         
// 		if (damageData.attackType != AttackType.Tranq) damageData.attackType = Const.a.attackTypeForWeapon[wep16Index]; // If check to handle exception setting it above
//         damageData.damage = DamageData.GetDamageTakeAmount(damageData);
//         damageData.owner = playerCapsule;
// 		damageData.impactVelocity = 80f;
// 		if (wep16Index == 12) {
// 			damageData.impactVelocity = 120f;
// 			if (tempHM != null) { // babamm boxes be like, u ded
// 				if (tempHM.isObject) damageData.damage *= 10f;
// 			}
// 		}
// 
// 		float dmgFinal = 0f;
// 		GameObject hitGO = tempHit.collider.transform.gameObject;
//         if (tempHM != null && tempHM.health > 0) {
// 			damageData.damage *= 0.8f; // Bit of heavy handed rebalancing lol.
// 			dmgFinal = tempHM.TakeDamage(damageData); // send the damageData container to HealthManager of hit object and apply damage
// 			damageData.impactVelocity += damageData.damage;
// 			if (!damageData.isOtherNPC || wep16Index == 12) {
// 				Utils.ApplyImpactForce(hitGO,damageData.impactVelocity,
// 									   damageData.attacknormal,
// 									   damageData.hit.point);
// 			}
// 			if (tempHM.isNPC && !tempHM.aic.asleep) Music.a.inCombat = true;
// 		}
// 
// 		if (dmgFinal < 0f) dmgFinal = 0f; // Less would = blank.
// 		CreateTargetIDInstance(dmgFinal,tempHM,tranq);
// 
// 		UseableObjectUse uou = hitGO.GetComponent<UseableObjectUse>();
// 		if (uou != null) uou.HitForce(damageData); // knock objects around
// 
//         // Draw a laser beam for beam weapons
//         if (wep16Index == 1 || wep16Index == 4 || wep16Index == 14) {
// 			CreateBeamEffects(wep16Index);
// 		}
//     }
// 
//     // Melee weapons
//     //-------------------------------------------------------------------------
//     // Rapier and pipe.  Need extra code to handle anims for view model and
// 	// sound for swing-and-a-miss! vs. hit
// 	IEnumerator ApplyMeleeHit(int index16, GameObject targ, bool isRapier, 
// 							  bool silent,AudioClip hit, AudioClip miss,
// 							  AudioClip hitflesh) {
// 		if (targ.layer == gameObject.layer) yield break;
// 
// 		if (isRapier) yield return new WaitForSeconds(0.28f);
// 		else yield return new WaitForSeconds(0.15f);
// 
// 		damageData.other = targ;
// 		damageData.isOtherNPC = false;
// 		if (targ.CompareTag("NPC")) damageData.isOtherNPC = true;
// 		damageData.attacknormal = MouseCursor.a.GetCursorScreenPointForRay();
// 		damageData.attacknormal =
// 			playerCamera.ScreenPointToRay(damageData.attacknormal).direction;
// 		damageData.damage = Const.a.damagePerHitForWeapon[index16]; 
// 		damageData.damage = DamageData.GetDamageTakeAmount(damageData);
// 		damageData.offense = Const.a.offenseForWeapon[index16];
// 		damageData.penetration = Const.a.penetrationForWeapon[index16];
// 		damageData.owner = playerCapsule;
// 		if (isRapier) {
// 			damageData.attackType = AttackType.MeleeEnergy;
// 			if (PlayerEnergy.a.energy < 4f) {
// 				// Half pipe
// 				damageData.damage = Const.a.damagePerHitForWeapon[6] / 2f;
// 			}
// 		} else {
// 			damageData.attackType = AttackType.Melee;
// 		}
// 
// 		UseableObjectUse uou = targ.GetComponent<UseableObjectUse>();
// 		if (uou != null) uou.HitForce(damageData); // knock objects around
// 		tempHM = Utils.GetMainHealthManager(targ);
// 		if (tempHM != null) {
// 			if (damageData.other != tempHM.gameObject) {
// 				damageData.other = tempHM.gameObject;
// 			}
// 		}
// 
// 		CreateStandardImpactEffects();
// 		if (damageData.other.CompareTag("Geometry")) {
// 			CreateStandardImpactMarks(index16);
// 		}
// 
// 		if (tempHM == null) {
// 			if (!silent) {
// 				PrefabIdentifier prefID = targ.GetComponent<PrefabIdentifier>();
// 				if (prefID == null) {
// 					if (targ.transform.parent != null) {
// 						prefID = targ.transform.parent.gameObject.GetComponent<PrefabIdentifier>();
// 					}
// 				}
// 				
// 				if (prefID != null && !isRapier) {
// 					FootStepType fstep = PlayerMovement.a.GetFootstepTypeForPrefab(prefID.constIndex);
// 					AudioClip stcp = PlayerMovement.a.JumpLandSound(fstep);
// 					play_wav(instances[i].position,stcp,1f);
// 					play_wav(instances[i].position,hit,0.65f);	
// 				} else {
// 					play_wav(instances[i].position,hit,1f);	
// 				}
// 
// 				PlayerHealth.a.makingNoise = true;
// 				PlayerHealth.a.noiseFinished = Sys_Global.pauseRelativeTime+0.5f;
// 			}
// 			yield break;
// 		}
// 
// 		damageData.impactVelocity = 80f + damageData.damage;
// 		if (!damageData.isOtherNPC || index16 == 12) {
// 			if (!isRapier || (isRapier && PlayerEnergy.a.energy >= 4f)) {
// 				Utils.ApplyImpactForce(targ, damageData.impactVelocity,
// 					damageData.attacknormal,damageData.hit.point);
// 			}
// 		}
// 
// 		float dmgFinal = tempHM.TakeDamage(damageData);
// 		if (dmgFinal < 0f) dmgFinal = 0f; // Less would = blank.
// 		CreateTargetIDInstance(dmgFinal,tempHM,-1f);
// 		if (tempHM.isNPC && !tempHM.aic.asleep) Music.a.inCombat = true;
// 		if (!silent) {
// 			PlayerHealth.a.makingNoise = true;
// 			PlayerHealth.a.noiseFinished = Sys_Global.pauseRelativeTime + 0.5f;
// 			if ((tempHM.bloodType == BloodType_Red)
// 				|| (tempHM.bloodType == BloodType_Yellow)
// 				|| (tempHM.bloodType == BloodType_Green)) {
// 				Utils.PlayUIOneShotSavable(hitflesh);
// 			} else if (isRapier && PlayerEnergy.a.energy < 4f) {
// 				Utils.PlayUIOneShotSavable(67);
// 			} else {
// 				Utils.PlayUIOneShotSavable(hit);
// 			}
// 		}
// 
// 		if (isRapier) {
// 			TakeEnergy(3.666f); // 3 hits per tick.
// 			if (BiomonitorGraphSystem.a != null) {
// 				BiomonitorEnergyPulse(3.666f);
// 			}
// 		}
// 	}
// 
// 	// These are a bit silly.
//     void FireRapier(int i16, bool sil) {
// 		FireMelee(i16,true,sil,sounds[246],sounds[247],sounds[246],true); // wlaserrapier_hit, wlaserrapier_swing, wlaserrapier_hit
// 	}
// 
//     void FirePipe(int i16, bool sil) {
// 		FireMelee(i16,false,sil,sounds[253],sounds[254],sounds[252],false); // wpipe_hit, wpipe_swing, wpipe_dmg
// 	}
// 
// 	void FireMelee(int index16, bool isRapier, bool silent, AudioClip hit,
// 				   AudioClip miss,AudioClip hitflesh, bool rapier) {
// 		// Do normal straightline raytrace at center first.
// 		fireDistance = meleescanDistance;
// 		if (DidRayHit(index16)) {
// 			fireDistance = hitscanDistance; // Reset before any returns.
// 			if (rapier) {
// 				if (rapieranim != null) {
// 					rapieranim.Play("Attack2");
// 					//rapieranim.Play("Attack2",-1,float.NegativeInfinity);
// 				}
// 			} else {
// 				if (anim != null) {
// 					anim.Play("Attack2");
// 					//anim.Play("Attack2",-1,float.NegativeInfinity);
// 				}
// 			}
// 
// 			GameObject hitGO = tempHit.collider.transform.gameObject;
// 			StartCoroutine(ApplyMeleeHit(index16,hitGO,isRapier,silent,hit,
// 										 miss,hitflesh));
// 			return;
// 		}
// 
// 		fireDistance = hitscanDistance; // Reset since raycast failed.
// 
// 		// Check all objects we can hurt have HealthManager, that they are in
// 		// meleescanDistance range, that they are within player facing angle by
// 		// 60° (±30°)	
// 		for (int i=0;i<Const.a.healthObjectsRegistration.Length;i++) {
// 			if (Const.a.healthObjectsRegistration[i] == null) continue;
// 
// 			HealthManager hm = Const.a.healthObjectsRegistration[i];
// 			// Don't hurt deactive objects, like you know, corpse on
// 			// living entities...at least don't do it again please.
// 			if (hm == null) continue;
// 			if (!hm.gameObject.activeInHierarchy) continue;
// 
// 			if (distance_vector3(hm.instances[i].position,
// 								 playerCapsule.instances[i].position)
// 				>= meleescanDistance) {
// 
// 				continue;
// 			}
// 
// 			MouseLookScript.a.SetCameraFocusPoint();
// 			tempVec = MouseLookScript.a.cameraFocusPoint
// 						- playerCamera.instances[i].position;
// 
// 			tempVec = tempVec.normalized;
// 			Vector3 ang = hm.instances[i].position
// 							- playerCamera.instances[i].position;
// 
// 			ang = ang.normalized;
// 			float dot = Vector3.Dot(tempVec,ang);
// 			if (dot <= 0.666f) continue;
// 
// 			if (rapier) {
// 				if (rapieranim != null) rapieranim.Play("Attack2");
// 			} else {
// 				if (anim != null) anim.Play("Attack2");
// 			}
// 
// 			StartCoroutine(ApplyMeleeHit(index16,hm.gameObject,isRapier,silent,
// 										 hit,miss,hitflesh));
// 			return;
// 		}
// 
// 		// Swing and a miss, steeeerike!!
// 		if (!silent) Utils.PlayUIOneShotSavable(miss);
// 		if (rapier) {
// 			if (rapieranim != null) rapieranim.Play("Attack2");
// 		} else {
// 			if (anim != null) anim.Play("Attack1");
// 		}
// 	}
// 
//     // Projectile weapons
//     //-------------------------------------------------------------------------
//     void FirePlasma(int index16) { FireBeachball(index16,plasmaShotForce,485); }
//     void FireRailgun(int index16) { FireBeachball(index16,railgunShotForce,484); }
//     void FireMagpulse(int index16) { FireBeachball(index16,magpulseShotForce,482); }
//     void FireStungun(int index16) { FireBeachball(index16,stungunShotForce,483); }
// 
// 	void FireBeachball(int index16, float shoveForce, int prefabID) {
//         // Create and hurl a beachball-like object.  On the developer
// 		// commentary they said that the projectiles act like a beachball for
// 		// collisions with enemies, but act like a baseball for walls/floor to
// 		// prevent hitting corners.
//         GameObject beachball = ConsoleEmulator.SpawnDynamicObject(prefabID,1);
//         if (beachball != null) {
// 			if (CurrentWeaponUsesEnergy()) {
//                 damageData.damage = DamageForPower(index16);
// 			} else {
// 				damageData.damage = Const.a.damagePerHitForWeapon[index16];
// 			}
//             damageData.owner = playerCapsule;
//             damageData.attackType = Const.a.attackTypeForWeapon[index16];
// 			damageData.offense = Const.a.offenseForWeapon[index16];
// 			damageData.penetration = Const.a.penetrationForWeapon[index16];
//             beachball.GetComponent<ProjectileEffectImpact>().dd = damageData;
//             beachball.GetComponent<ProjectileEffectImpact>().host = playerCapsule;
//             beachball.instances[i].position = playerCamera.instances[i].position;
// 			MouseLookScript.a.SetCameraFocusPoint();
//             tempVec = MouseLookScript.a.cameraFocusPoint - playerCamera.instances[i].position;
//             beachball.transform.forward = tempVec.normalized;
//             beachball.SetActive(true);
//             Vector3 shove = beachball.transform.forward * shoveForce;
// 
// 			// Force starting with zero pior to adding impulse force.
//             beachball.GetComponent<Rigidbody>().velocity = Const.a.vectorZero;
//             beachball.GetComponent<Rigidbody>().AddForce(shove,ForceMode.Impulse);
//         }
// 	}
// 
//     public Vector3 ScreenPointToDirectionVector() {
//         Vector3 retval = Const.a.vectorZero;
//         retval = playerCamera.transform.forward;
//         return retval;
//     }
// }

// public class WeaponMagazineCounter : MonoBehaviour {
// 	public Sprite[] indicatorSprites;
// 	public Image onesIndicator;
// 	public Image tensIndicator;
// 	public Image hunsIndicator;
// 	private int tempi;
// 	private int checkcount;
// 	private int[] tempis = new int[] {0,0,0};
// 
// 	public void UpdateDigits (int newamount) {
// 		tempi = newamount;
// 		tempis[0] = 10;
// 		tempis[1] = 10;
// 		tempis[2] = 10;
// 
// 		//if (tempi > 99) {
// 			tempis[2] = tempi % 10; // ones
// 		if (newamount > 9) {
// 			tempi /= 10;
// 			tempis[1] = tempi % 10; // tens
// 		}
// 		if (newamount > 99) {
// 			tempi /= 10;
// 			tempis[0] = tempi % 10; // huns
// 		}
// 
// 		onesIndicator.overrideSprite = indicatorSprites[tempis[2]];
// 		tensIndicator.overrideSprite = indicatorSprites[tempis[1]];
// 		hunsIndicator.overrideSprite = indicatorSprites[tempis[0]];
// 	}
// 
// 	void Update() {
// 		if (Sys_Global.gamePaused || Sys_Global.menuActive) return;
// 		
// 		int index = WeaponCurrent.a.weaponCurrent; // 0 to 6, 7 slots
// 		// Changed from this:
// 		// Get16WeaponIndexFromConstIndex(WeaponCurrent.a.weaponIndex); 0 to 15
// 		if (index < 0) return;
// 
// 		if (WeaponCurrent.a.weaponIndex == -1
// 		    || WeaponCurrent.a.weaponIndex == 41
// 			|| WeaponCurrent.a.weaponIndex == 42
// 			|| MouseLookScript.a.inCyberSpace
// 			|| WeaponCurrent.a.weaponCurrentPending >= 0) {
// 				tempis[0] = 10; // blank
// 				tempis[1] = 10; // blank
// 				tempis[2] = 10; // blank
// 				onesIndicator.overrideSprite = indicatorSprites[tempis[2]];
// 				tensIndicator.overrideSprite = indicatorSprites[tempis[1]];
// 				hunsIndicator.overrideSprite = indicatorSprites[tempis[0]];
// 				return;
// 		}
// 
// 		if (inventoryPlayer1.wepLoadedWithAlternate[WeaponCurrent.a.weaponCurrent]) {
// 			UpdateDigits(WeaponCurrent.a.currentMagazineAmount2[index]);
// 		} else {
// 			UpdateDigits(WeaponCurrent.a.currentMagazineAmount[index]);
// 		}
// 	}
// }

// public class WeaponCurrent : MonoBehaviour {
// 	public GameObject ViewModelAssault;
// 	public GameObject ViewModelBlaster;
// 	public GameObject ViewModelDartgun;
// 	public GameObject ViewModelFlechette;
// 	public GameObject ViewModelIon;
// 	public GameObject ViewModelRapier;
// 	public GameObject ViewModelPipe;
// 	public GameObject ViewModelMagnum;
// 	public GameObject ViewModelMagpulse;
// 	public GameObject ViewModelPistol;
// 	public GameObject ViewModelPlasma;
// 	public GameObject ViewModelRailgun;
// 	public GameObject ViewModelRiotgun;
// 	public GameObject ViewModelSkorpion;
// 	public GameObject ViewModelSparq;
// 	public GameObject ViewModelStungun;
// 	public GameObject owner;
// 
// 	public int weaponCurrent = new int(); // save
// 	public int weaponIndex = new int(); // save
// 
// 	bool justChangedWeap = true; // save
// 	float[] weaponEnergySetting; // save
// 	int[] currentMagazineAmount; // save
// 	int[] currentMagazineAmount2; // save
// 	int lastIndex = 0; // save
// 	bool bottomless = false; // Don't use any ammo and
// 													  // energy weapons no
// 													  // energy, save
// 	bool redbull = false; // No energy usage, save
// 	private static StringBuilder s1 = new StringBuilder();
// 	
// 	public int weaponCurrentPending; // save
// 	public int weaponIndexPending; // save
// 
// 	public static WeaponCurrent a;
// 
// 	void Start() {
// 		a = this;
// 		a.weaponCurrent = 0; // Current slot in the weapon inventory (7 slots)
// 		a.weaponIndex = -1; // Current index to the weapon look-up tables
// 
// 		// Put energy settings to lowest energy level as default
// 		for (int j=0;j<7;j++) {
// 			a.weaponEnergySetting[j] = 0f;
// 			a.currentMagazineAmount[j] = 0;
// 			a.currentMagazineAmount2[j] = 0;
// 		}
// 		weaponCurrentPending = -1;
// 		weaponIndexPending = -1;
// 	}
// 
// 	public void SetAllViewModelsDeactive() {
// 		Utils.Deactivate(ViewModelAssault);
// 		Utils.Deactivate(ViewModelBlaster);
// 		Utils.Deactivate(ViewModelDartgun);
// 		Utils.Deactivate(ViewModelFlechette);
// 		Utils.Deactivate(ViewModelIon);
// 		Utils.Deactivate(ViewModelRapier);
// 		Utils.Deactivate(ViewModelPipe);
// 		Utils.Deactivate(ViewModelMagnum);
// 		Utils.Deactivate(ViewModelMagpulse);
// 		Utils.Deactivate(ViewModelPistol);
// 		Utils.Deactivate(ViewModelPlasma);
// 		Utils.Deactivate(ViewModelRailgun);
// 		Utils.Deactivate(ViewModelRiotgun);
// 		Utils.Deactivate(ViewModelSkorpion);
// 		Utils.Deactivate(ViewModelSparq);
// 		Utils.Deactivate(ViewModelStungun);
// 		Utils.Deactivate(Sys_UI.energySliderLH);
// 		Utils.Deactivate(Sys_UI.energyHeatTicksLH);
// 		Utils.Deactivate(Sys_UI.overloadButtonLH);
// 		Utils.Deactivate(Sys_UI.unloadButtonLH);
// 		Utils.Deactivate(Sys_UI.loadNormalAmmoButtonLH);
// 		Utils.Deactivate(Sys_UI.loadAlternateAmmoButtonLH);
// 
// 		Utils.Deactivate(Sys_UI.energySliderRH);
// 		Utils.Deactivate(Sys_UI.energyHeatTicksRH);
// 		Utils.Deactivate(Sys_UI.overloadButtonRH);
// 		Utils.Deactivate(Sys_UI.unloadButtonRH);
// 		Utils.Deactivate(Sys_UI.loadNormalAmmoButtonRH);
// 		Utils.Deactivate(Sys_UI.loadAlternateAmmoButtonRH);
// 	}
// 
// 	public void RemoveWeapon(int weaponButton7Index) {
// 		WeaponButtonsManager wepbutMan = Sys_UI.wepbutMan;
// 		WeaponButton wepbut = wepbutMan.wepButtonsScripts[0];
// 		if (weaponButton7Index != weaponCurrent) {
// 			if (weaponButton7Index > weaponCurrent) return; // No list shift.
// 
// 			weaponCurrent--;
// 			return; // Don't continue down and change the weapon, keep current.
// 		}
// 
// 		SetAllViewModelsDeactive();
// 		WeaponFire.a.reloadFinished = 0;
// 		int initialIndex = WeaponCurrent.a.weaponCurrent;
// 		if (initialIndex < 0) initialIndex = 0;
// 		if (initialIndex > 6) initialIndex = 0;
// 		int nextIndex = initialIndex - 1; // add 1 to get slot above this
// 		if (nextIndex < 0) nextIndex = 6; // wraparound to top
// 		int countCheck = 0;
// 		bool buttonNotValid = (inventoryPlayer1.weaponInventoryIndices[nextIndex] == -1);
// 		while (buttonNotValid) {
// 			countCheck++;
// 			if (countCheck > 13) return; // no weapons!  don't runaway loop
// 
// 			nextIndex--;
// 			if (nextIndex < 0) nextIndex = 6;
// 			buttonNotValid = (inventoryPlayer1.weaponInventoryIndices[nextIndex] == -1);
// 		}
// 
// 		wepbut = wepbutMan.wepButtonsScripts[nextIndex];
// 		if (!wepbut.gameObject.activeSelf) {
// 			wepbut = wepbutMan.wepButtonsScripts[0];
// 		}
// 
// 		if (wepbut.gameObject.activeSelf && nextIndex != initialIndex) {
// 			weaponIndexPending = wepbut.useableItemIndex;
// 			weaponCurrentPending = wepbut.WepButtonIndex;
// 		} else {
// 			weaponCurrentPending = 0;
// 			weaponIndexPending = -1;
// 		}
// 
// 		lastIndex = weaponCurrent;
// 		justChangedWeap = true;
// 		weaponCurrent = -1;
// 		weaponIndex = -1;
// 		WeaponFire.a.StartWeaponDip(0);
// 		currentMagazineAmount[weaponButton7Index] = 0; // Zero out ammo
// 		currentMagazineAmount2[weaponButton7Index] = 0;
// 		Sys_UI.UpdateHUDAmmoCountsEither();
// 		Sys_UI.SetWepInfo(-1);
// 		Sys_UI.OpenTab(0, true, TabMSG.Weapon, 0,Handedness.LH);
// 	}
// 
// 	public void WeaponChange(int useableItemIndex, int buttonIndex) {
// 		if (WeaponFire.a.reloadFinished > Sys_Global.pauseRelativeTime) return;
// 		if (useableItemIndex == -1 || buttonIndex > 6 || buttonIndex < 0) {
// 			Sys_UI.SetAmmoIcons(-1,false); // Clear the ammo icons.
// 			//DualLog("Early exit on WeaponChange() in WeaponCurrent.cs!");
// 			return;
// 		}
// 
// 		Utils.PlayUIOneShotSavable(80); // changeweapon
// 		if (buttonIndex == weaponCurrent) return; // Already there!
// 
// 		int wep16index =  // Get index into the list of 16 weapons
// 			  WeaponFire.Get16WeaponIndexFromConstIndex(useableItemIndex);
// 
// 		WeaponFire.a.StartWeaponDip(Const.a.reloadTime[wep16index]);
// 		weaponCurrentPending = buttonIndex;
// 		weaponIndexPending = useableItemIndex;
// 		Sys_UI.SetWepInfo(-1);
// 		Sys_UI.UpdateHUDAmmoCountsEither();
// 	}
// 
// 	void Update() {
// 		if (Sys_Global.gamePaused) return;
// 		if (Sys_Global.menuActive) return;
// 
// 		if (justChangedWeap) {
// 			justChangedWeap = false;
// 			Sys_UI.SetAmmoIcons(-1,false); // Clear it.
// 			UpdateWeaponViewModels();
// 		}
// 
// 		// Compare weaponCurrent since we might have more of the same type.
// 		if (lastIndex != weaponCurrent) lastIndex = weaponCurrent;
// 	}
// 
// 	public void UpdateWeaponViewModels() {
// 		if (Sys_Global.gamePaused) return;
// 		if (Sys_Global.menuActive) return;
// 
// 		int useableIndex = weaponIndex;
// 		int setWep = weaponIndex;
// 		if (weaponIndexPending >= 0) {
// 			setWep = weaponIndexPending;
// 			useableIndex = -1;
// 		}
// 
// 		Sys_UI.HideAmmoAndEnergyItems();
// 		SetAllViewModelsDeactive();
// 		switch (setWep) {
// 			case 36: // "LOAD MAGNESIUM", "LOAD PENETRATOR"
// 				Sys_UI.ShowAmmoItems(539,540);
// 				Utils.Activate(ViewModelAssault);
// 				break;
// 			case 37:
// 				Sys_UI.ShowEnergyItems();
// 				Utils.Activate(ViewModelBlaster);
// 				break;
// 			case 38: // "LOAD NEEDLE", "LOAD TRANQ"
// 				Sys_UI.ShowAmmoItems(541,542);
// 				Utils.Activate(ViewModelDartgun);
// 				break;
// 			case 39: // "LOAD HORNET", "LOAD SPLINTER"
// 				Sys_UI.ShowAmmoItems(543,544);
// 				Utils.Activate(ViewModelFlechette);
// 				break;
// 			case 40:
// 				Sys_UI.ShowEnergyItems();
// 				Utils.Activate(ViewModelIon);
// 				break;
// 			case 41:
// 				Utils.Activate(ViewModelRapier);
// 				break;
// 			case 42:
// 				Utils.Activate(ViewModelPipe);
// 				break;
// 			case 43: // "LOAD HOLLOW TIP", "LOAD HEAVY SLUG"
// 				Sys_UI.ShowAmmoItems(545,546);
// 				Utils.Activate(ViewModelMagnum);
// 				break;
// 			case 44: // "LOAD CARTRIDGE"
// 				Sys_UI.ShowAmmoItems(547,-1);
// 				Utils.Activate(ViewModelMagpulse);
// 				Sys_UI.HideAlternateAmmoButton();
// 				break;
// 			case 45: // "LOAD STANDARD", "LOAD TEFLON"
// 				Sys_UI.ShowAmmoItems(548,549);
// 				Utils.Activate(ViewModelPistol);
// 				break;
// 			case 46:
// 				Sys_UI.ShowEnergyItems();
// 				Utils.Activate(ViewModelPlasma);
// 				break;
// 			case 47: // "LOAD RAIL CLIP"
// 				Sys_UI.ShowAmmoItems(550,-1);
// 				Utils.Activate(ViewModelRailgun);
// 				Sys_UI.HideAlternateAmmoButton();
// 				break;
// 			case 48:  // "LOAD RUBBER SLUG"
// 				Sys_UI.ShowAmmoItems(551,-1);
// 				Utils.Activate(ViewModelRiotgun);
// 				Sys_UI.HideAlternateAmmoButton();
// 				break;
// 			case 49: // "LOAD SLAG", "LOAD LARGE SLAG"
// 				Sys_UI.ShowAmmoItems(552,553);
// 				Utils.Activate(ViewModelSkorpion);
// 				break;
// 			case 50:
// 				Sys_UI.ShowEnergyItems();
// 				Utils.Activate(ViewModelSparq);
// 				break;
// 			case 51:
// 				Sys_UI.ShowEnergyItems();
// 				Utils.Activate(ViewModelStungun);
// 				break;
// 		}
// 	}
// 
// 	public void ChangeAmmoType() {
// 		int wep16index = WeaponFire.Get16WeaponIndexFromConstIndex(weaponIndex);
// 		if (wep16index < 0) return;
// 		if (wep16index == 5 || wep16index == 6) {
// 			CenterStatusPrint(315);
// 			return; // Do nothing for pipe or rapier.
// 		}
// 
// 		if (wep16index == 1 || wep16index == 4 || wep16index == 10 || wep16index == 14 || wep16index == 15) {
// 			if (Sys_UI.overloadButtonLH.activeInHierarchy) {
// 				Sys_UI.overloadButtonLH.GetComponent<EnergyOverloadButton>().OverloadButtonAction();
// 			}
// 
// 			if (Sys_UI.overloadButtonRH.activeInHierarchy) {
// 				Sys_UI.overloadButtonRH.GetComponent<EnergyOverloadButton>().OverloadButtonAction();
// 			}
// 		} else {
// 			if (inventoryPlayer1.wepLoadedWithAlternate[weaponCurrent]) {
// 				if (inventoryPlayer1.wepAmmo[wep16index] > 0) {
// 				inventoryPlayer1.wepLoadedWithAlternate[weaponCurrent] = false;
// 				// Take bullets out of the clip, put them back into the ammo stockpile, then zero out the clip amount, did I say clip?  I mean magazine but whatever
// 				inventoryPlayer1.wepAmmoSecondary[wep16index] += currentMagazineAmount2[weaponCurrent];
// 				currentMagazineAmount2[weaponCurrent] = 0;
// 				LoadPrimaryAmmoType(false);
// 				} else {
// 					CenterStatusPrint(535); //No more of ammo type to load.
// 				}
// 			} else {
// 				if (inventoryPlayer1.wepAmmoSecondary[wep16index] > 0) {
// 					inventoryPlayer1.wepLoadedWithAlternate[weaponCurrent] = true;
// 					inventoryPlayer1.wepAmmo[wep16index] += currentMagazineAmount[weaponCurrent];
// 					currentMagazineAmount[weaponCurrent] = 0;
// 					LoadSecondaryAmmoType(false);
// 				} else {
// 					CenterStatusPrint(535); //No more of ammo type to load.
// 				}
// 			}
// 		}
// 	}
// 
// 	public void LoadPrimaryAmmoType(bool isSilent) {
// 		int wep16index = WeaponFire.Get16WeaponIndexFromConstIndex(weaponIndex);
// 		if (!inventoryPlayer1.wepLoadedWithAlternate[weaponCurrent]) { // Already loaded with normal.
// 			if (currentMagazineAmount[weaponCurrent] == Const.a.magazinePitchCountForWeapon[wep16index]) {
// 				CenterStatusPrint(191); //Current weapon magazine already full.
// 				return;
// 			}
// 			
// 			if (currentMagazineAmount[weaponCurrent] == inventoryPlayer1.wepAmmo[wep16index]) {
// 				CenterStatusPrint(535); // No more of ammo type to load
// 				return;
// 			}
// 		}
// 
// 		Unload(true);
// 		inventoryPlayer1.wepLoadedWithAlternate[weaponCurrent] = false;
// 
// 		// Put bullets into the magazine
// 		if (inventoryPlayer1.wepAmmo[wep16index] >= Const.a.magazinePitchCountForWeapon[wep16index]) {
// 			currentMagazineAmount[weaponCurrent] = Const.a.magazinePitchCountForWeapon[wep16index];
// 		} else {
// 			currentMagazineAmount[weaponCurrent] = inventoryPlayer1.wepAmmo[wep16index];
// 		}
// 
// 		// Take bullets out of the ammo stockpile
// 		inventoryPlayer1.wepAmmo[wep16index] -= currentMagazineAmount[weaponCurrent];
// 
// 		if (!isSilent) {
// 			if (wep16index == 0 || wep16index == 3) {
// 				Utils.PlayUIOneShotSavable(248); // wlocknload
// 			} else {
// 				Utils.PlayUIOneShotSavable(260); // wreload
// 			}
// 		}
// 
// 		// Update the counter on the HUD
// 		Sys_UI.UpdateHUDAmmoCounts(currentMagazineAmount[weaponCurrent]);
// 		WeaponFire.a.StartWeaponDip(Const.a.reloadTime[wep16index]);
// 
// 		// Pop it back to start to be sure
// 		WeaponFire.a.reloadContainer.localPosition =
// 			WeaponFire.a.reloadContainerHome;
// 	}
// 
// 	public void LoadSecondaryAmmoType(bool isSilent) {
// 		int wep16index = WeaponFire.Get16WeaponIndexFromConstIndex(weaponIndex);
// 		if (inventoryPlayer1.wepLoadedWithAlternate[weaponCurrent]) { // Already loaded with alternate
// 			if (currentMagazineAmount2[weaponCurrent] == Const.a.magazinePitchCountForWeapon2[wep16index]) {
// 				CenterStatusPrint(191); //Current weapon magazine already full.
// 				return;
// 			}
// 			
// 			if (currentMagazineAmount2[weaponCurrent] == inventoryPlayer1.wepAmmoSecondary[wep16index]) {
// 				CenterStatusPrint(535); // No more of ammo type to load
// 				return;
// 			}
// 		}
// 
// 		Unload(true);
// 		inventoryPlayer1.wepLoadedWithAlternate[weaponCurrent] = true;
// 
// 		// Put bullets into the magazine
// 		if (inventoryPlayer1.wepAmmoSecondary[wep16index] >= Const.a.magazinePitchCountForWeapon2[wep16index]) {
// 			currentMagazineAmount2[weaponCurrent] = Const.a.magazinePitchCountForWeapon2[wep16index];
// 		} else {
// 			currentMagazineAmount2[weaponCurrent] = inventoryPlayer1.wepAmmoSecondary[wep16index];
// 		}
// 
// 		// Take bullets out of the ammo stockpile
// 		inventoryPlayer1.wepAmmoSecondary[wep16index] -= currentMagazineAmount2[weaponCurrent];
// 
// 		if (!isSilent) {
// 			if (wep16index == 0 || wep16index == 3) {
// 				Utils.PlayUIOneShotSavable(248); // wlocknload
// 			} else {
// 				Utils.PlayUIOneShotSavable(260); // wreload
// 			}
// 		}
// 
// 		// Update the counter on the HUD
// 		Sys_UI.UpdateHUDAmmoCounts(currentMagazineAmount2[weaponCurrent]);
// 		WeaponFire.a.StartWeaponDip(Const.a.reloadTime[wep16index]);
// 
// 		// Pop it back to start to be sure
// 		WeaponFire.a.reloadContainer.localPosition =
// 			WeaponFire.a.reloadContainerHome;
// 	}
// 
// 	public void Unload(bool isSilent) {
// 		if (weaponIndex < 0) return;
// 		int wep16index = WeaponFire.Get16WeaponIndexFromConstIndex (weaponIndex);
// 		if (wep16index == 5 || wep16index == 6) {
// 			return; // do nothing for pipe or rapier
// 		}
// 
// 		if (wep16index == -1) return; // we don't have a weapon at all right now :)
// 
// 		// Take bullets out of the clip, put them back into the ammo stockpile, then zero out the clip amount, did I say clip?  I mean magazine but whatever
// 		if (inventoryPlayer1.wepLoadedWithAlternate[weaponCurrent]) {
// 			inventoryPlayer1.wepAmmoSecondary[wep16index] += currentMagazineAmount2[weaponCurrent];
// 			currentMagazineAmount2[weaponCurrent] = 0;
// 
// 			// Update the counter on the HUD
// 			Sys_UI.UpdateHUDAmmoCounts(currentMagazineAmount2[weaponCurrent]);
// 		} else {
// 			inventoryPlayer1.wepAmmo[wep16index] += currentMagazineAmount[weaponCurrent];
// 			currentMagazineAmount[weaponCurrent] = 0;
// 
// 			// Update the counter on the HUD
// 			Sys_UI.UpdateHUDAmmoCounts(currentMagazineAmount[weaponCurrent]);
// 		}
// 		if (!isSilent) Utils.PlayUIOneShotSavable(260); // wreload
// 	}
// 
// 	public void ReloadSecret(bool isSilent) {
// 		int wep16index = WeaponFire.Get16WeaponIndexFromConstIndex(weaponIndex);
// 		if (wep16index < 0) return;
// 
// 		if (wep16index == 5 || wep16index == 6) {
// 			CenterStatusPrint(315); // Weapon does not use ammo.
// 			return; // do nothing for pipe or rapier
// 		}
// 
// 		if (wep16index == 1 || wep16index == 4 || wep16index == 10 || wep16index == 14 || wep16index == 15) {
// 			CenterStatusPrint(538); // Weapon does not need reloaded.
// 			return; // do nothing for energy weapons
// 		}
// 
// 		if (weaponCurrent < 0) return;
// 
// 		if (inventoryPlayer1.wepLoadedWithAlternate[weaponCurrent]) {
// 			if (currentMagazineAmount2[weaponCurrent] == Const.a.magazinePitchCountForWeapon2[wep16index]) {
// 				CenterStatusPrint(191); //Current weapon magazine already full.
// 				return;
// 			}
// 
// 			if (inventoryPlayer1.wepAmmoSecondary[wep16index] <= 0) {
// 				if (inventoryPlayer1.wepAmmo[wep16index] <= 0) {
// 					CenterStatusPrint(305); //No more of any ammo type to load.
// 					return;
// 				} else {
// 					CenterStatusPrint(192); //No more of current ammo type to load, loading with alternate.
// 					LoadPrimaryAmmoType(isSilent);
// 					return;
// 				}
// 			}
// 			LoadSecondaryAmmoType(isSilent);
// 		} else {
// 			if (currentMagazineAmount[weaponCurrent] == Const.a.magazinePitchCountForWeapon[wep16index]) {
// 				CenterStatusPrint(191); //Current weapon magazine already full.
// 				return;
// 			}
// 
// 			if (inventoryPlayer1.wepAmmo[wep16index] <= 0) {
// 				if (inventoryPlayer1.wepAmmoSecondary[wep16index] <= 0) {
// 					CenterStatusPrint(305); //No more of any ammo type to load.
// 					return;
// 				} else {
// 					CenterStatusPrint(192); //No more of current ammo type to load, loading with alternate.
// 					LoadSecondaryAmmoType(isSilent);
// 					return;
// 				}
// 			}
// 			LoadPrimaryAmmoType(isSilent);
// 		}
// 	}
// 
// 	public void Reload() {
// 		ReloadSecret(false);
// 	}
// }

void WeaponCycleUp() {
    if (MouseLookScript.a.inCyberSpace) {
        inventoryPlayer1.isPulserNotDrill = !inventoryPlayer1.isPulserNotDrill; // There's only two cyberspace weapons, up is down.
        Utils.PlayUIOneShotSavable(80); // changeweapon
        if (inventoryPlayer1.isPulserNotDrill) {
            inventoryPlayer1.pulserButtonText.Select(true);
            inventoryPlayer1.drillButtonText.Select(false);
        } else {
            inventoryPlayer1.pulserButtonText.Select(false);
            inventoryPlayer1.drillButtonText.Select(true);
        }
    } else {
        if (Const.a.InputInvertInventoryCycling) wepbutMan.WeaponCycleDown();
        else {
            if (WeaponFire.a.reloadFinished > Sys_Global.pauseRelativeTime) return;

            int initialIndex = WeaponCurrent.a.weaponCurrent;
            if (initialIndex < 0) initialIndex = 0;
            if (initialIndex > 6) initialIndex = 0;
            int nextIndex = initialIndex + 1; // add 1 to get slot above this
            if (nextIndex > 6) nextIndex = 0; // wraparound to bottom
            int countCheck = 0;
            bool buttonNotValid = (inventoryPlayer1.weaponInventoryIndices[nextIndex] == -1);
            while (buttonNotValid) {
                countCheck++;
                if (countCheck > 13) {
                    return; // no weapons!  don't runaway loop
                }
                nextIndex++;
                if (nextIndex > 6) nextIndex = 0;
                buttonNotValid = (inventoryPlayer1.weaponInventoryIndices[nextIndex] == -1);
            }

            if (wepButtonsScripts[nextIndex].gameObject.activeSelf
                && nextIndex != initialIndex) {
                WeaponCurrent.a.WeaponChange(wepButtonsScripts[nextIndex].useableItemIndex, wepButtonsScripts[nextIndex].WepButtonIndex);
            }
        }
    }
}

void WeaponCycleDown() {
    if (MouseLookScript.a.inCyberSpace) {
        // There's only two cyberspace weapons, up is down.
        inventoryPlayer1.isPulserNotDrill = !inventoryPlayer1.isPulserNotDrill;
        Utils.PlayUIOneShotSavable(80); // changeweapon
        if (inventoryPlayer1.isPulserNotDrill) {
            inventoryPlayer1.pulserButtonText.Select(true);
            inventoryPlayer1.drillButtonText.Select(false);
        } else {
            inventoryPlayer1.pulserButtonText.Select(false);
            inventoryPlayer1.drillButtonText.Select(true);
        }
    } else {
        if (Const.a.InputInvertInventoryCycling) wepbutMan.WeaponCycleUp();
        else {
            if (WeaponFire.a.reloadFinished > Sys_Global.pauseRelativeTime) return;

            int initialIndex = WeaponCurrent.a.weaponCurrent;
            if (initialIndex < 0) initialIndex = 0;
            if (initialIndex > 6) initialIndex = 0;
            int nextIndex = initialIndex - 1; // add 1 to get slot above this
            if (nextIndex < 0) nextIndex = 6; // wraparound to top
            int countCheck = 0;
            bool buttonNotValid = (inventoryPlayer1.weaponInventoryIndices[nextIndex] == -1);
            while (buttonNotValid) {
                countCheck++;
                if (countCheck > 13) {
                    return; // no weapons!  don't runaway loop
                }
                nextIndex--;
                if (nextIndex < 0) nextIndex = 6;
                buttonNotValid = (inventoryPlayer1.weaponInventoryIndices[nextIndex] == -1);
            }

            if (wepButtonsScripts[nextIndex].gameObject.activeSelf
                && nextIndex != initialIndex) {
                WeaponCurrent.a.WeaponChange(wepButtonsScripts[nextIndex].useableItemIndex,
                                                wepButtonsScripts[nextIndex].WepButtonIndex);
            }
        }
    }
}
