using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;
using System.Collections;

public class PatchButton: MonoBehaviour {
	public int PatchButtonIndex;
	public int useableItemIndex;

	public void DoubleClick() {
		Sys_UI.mouseClickHeldOverGUI = true;
		PatchUse();
	}

	public void PatchUse() {
		PlayerPatch.a.ActivatePatch(useableItemIndex);
	}

	public void PatchInvClick (bool useSound) {
		Sys_UI.mouseClickHeldOverGUI = true;
		PatchSelect(useSound);
	}

	public void PatchSelect(bool useSound) {
		Sys_UI.SendInfoToItemTab(useableItemIndex);
		Eng_Global->inventoryPlayer1.patchCurrent = PatchButtonIndex; // Set current.
		for (int i = 0; i < 7; i++) {
			Eng_Global->inventoryPlayer1.patchCountTextObjects [i].color = Const.a.ssGreenText;
		}
		Eng_Global->inventoryPlayer1.patchCountTextObjects[PatchButtonIndex].color = Const.a.ssYellowText;
		if (useSound) Utils.PlayUIOneShotSavable(80); //changeweapon
	}

    void Start() {
        GetComponent<Button>().onClick.AddListener(() => { PatchInvClick(true); });
    }
}
