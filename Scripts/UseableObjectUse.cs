using UnityEngine;
using System.Collections;
using System.Text;

public class UseableObjectUse : MonoBehaviour {
	public int useableItemIndex;
	public int customIndex = -1;
	public int ammo = 0;
	public int ammo2 = 0;
	public bool heldObjectLoadedAlternate = false;
	private static StringBuilder s1 = new StringBuilder();

	void Awake() {
		// 33% chance of not spawning logic probes on Puzzle difficulty of 3
		if (Eng_Global->difficultyPuzzle == 3) {
			if (useableItemIndex == 54) {
				if (random_range(0,1f) < 0.33f) {
					Utils.SafeDestroy(gameObject);
				}
			}
		}

		// Remove access cards on Mission difficulty 1 or 0
		if (Eng_Global->difficultyMission <= 1) {
			if (useableItemIndex >= 81 && useableItemIndex <= 91) {
				Utils.SafeDestroy(gameObject);
			}
		}

		// Remove audiologs on Mission difficulty 0
		if (Eng_Global->difficultyMission == 0) {
			if (useableItemIndex == 6) Utils.SafeDestroy(gameObject);
		}
	}

	// Was GameObject owner as arguments, now UseData to hold more info.
	public void Use (UseData ud) {
	    if (MouseLookScript.a.holdingObject) {
	        MouseLookScript.a.DropHeldItem();
	        return;
	    }
	    
		if (useableItemIndex < 0) DualLog("BUG: Useable index less than 0!");
		MouseLookScript.a.holdingObject = true;
		Eng_Global->inventoryPlayer1.holdingObjectIndex = useableItemIndex;
		MouseLookScript.a.heldObjectCustomIndex = customIndex;
		MouseLookScript.a.heldObjectAmmo = ammo;
		MouseLookScript.a.heldObjectAmmo2 = ammo2;
		MouseLookScript.a.heldObjectLoadedAlternate = heldObjectLoadedAlternate;
		if (Const.a.InputQuickItemPickup) {
			MouseLookScript.a.AddItemToInventory(useableItemIndex,customIndex);
			MouseLookScript.a.ResetHeldItem();
		} else {
			MouseLookScript.a.ForceInventoryMode();  // Inventory mode is turned on when picking something up
			CenterStatusPrint("%s", Eng_Text->stringTable[useableItemIndex + 326] // <item>
						 + Eng_Text->stringTable[319]); // picked up.
		}
		
		Destroy(gameObject);
	}

	public void HitForce (DamageData dd) {
		Rigidbody rbody = GetComponent<Rigidbody>();
		if (rbody != null) {
			rbody.AddForceAtPosition((dd.attacknormal*(dd.damage + 80f)),
									 dd.hit.point); // knock me around will you
		}
	}
}
