using UnityEngine;
using System.Collections;
using System.Text;

public class KeypadKeycode : MonoBehaviour {
	public int securityThreshhold = 100; // If security level is not below this level, this is unusable.
	public int keycode; // the access code
	public bool locked = false; // save
	public string target;
	public string lockedTarget;
	public int successMessageLingdex = -1;
	public int lockedMessageLingdex = -1;
	public bool solved = false; // save
	public bool useQuestKeycode1 = false;
	public bool useQuestKeycode2 = false;
	
	bool padInUse = false; // save
	private GameObject playerCamera;
	private static StringBuilder s1 = new StringBuilder();

	void Start () {
		padInUse = false;
		playerCamera = PlayerReferenceManager.a.playerCapsuleMainCamera;
	}

	public void Use (UseData ud) {
	    if (Eng_Cheats->superoverride || World->diffMis == 0) {
	        locked = false; // SHODAN can go anywhere!  Full security override!
		} else if (GetCurrentLevelSecurity() > securityThreshhold) {
		    Eng_UI->BlockedBySecurity(World->instances[i].position);
		    return;
		}

		if (locked) {
			CenterStatusPrint(lockedMessageLingdex);

			// Target something because we are locked like a Vox message to
			// say we're locked, e.g. "Non emergency life pods disabled."
			UseTargets(gameObject,ud,lockedTarget); 
			return;
		}

		if (useQuestKeycode1) {
			if (Const.a.questData.lev1SecCode != -1) {
				if (Const.a.questData.lev2SecCode != -1) {
					if (Const.a.questData.lev3SecCode != -1) {
						int tempones = Const.a.questData.lev3SecCode;
						int temptens = Const.a.questData.lev2SecCode * 10;
						int temphuns = Const.a.questData.lev1SecCode * 100;
						
						// Decode digits into keycode from levels 1, 2, and 3
						// in order huns, tens, ones.
						keycode = temphuns + temptens + tempones;
					}
				}
			} else {
				CenterStatusPrint(289);
				return;
			}
		}

		if (useQuestKeycode2) {
			if (Const.a.questData.lev4SecCode != -1) {
				if (Const.a.questData.lev5SecCode != -1) {
					if (Const.a.questData.lev6SecCode != -1) {
						int tempones = Const.a.questData.lev6SecCode;
						int temptens = Const.a.questData.lev5SecCode * 10;
						int temphuns = Const.a.questData.lev4SecCode * 100;
						
						// Secode digits into keycode from levels 4, 5, and 
						// in order huns, tens, ones.
						keycode = temphuns + temptens + tempones;
					}
				}
			} else {
				CenterStatusPrint(290);
				return;
			}
		}

		padInUse = true;
		Utils.PlayUIOneShotSavable(91);
		MouseLookScript.a.ForceInventoryMode();
		Eng_UI->SendKeypadKeycodeToDataTab(keycode,World->instances[i].position,
		                                        this,solved);
	}

	public void UseTargets () {
		UseData ud = new UseData();
		ud.owner = playerCamera;
		UseTargets(gameObject,ud,target);
		CenterStatusPrint(successMessageLingdex);
	}
}
