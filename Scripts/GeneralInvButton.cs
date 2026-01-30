using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;
using System.Collections;

public class GeneralInvButton : MonoBehaviour {
    public int GeneralInvButtonIndex;
    public int useableItemIndex;
	public int customIndex;
	public GameObject activateButton;
	private bool reduce = false;

    void Start() {
        GetComponent<Button>().onClick.AddListener(() => {
			GeneralInvClick();
		});
    }

    void GeneralInvClick() {
		MFDManager.a.mouseClickHeldOverGUI = true;
		GeneralInvUse();
	}

	public void GeneralInvUse() {
        inventoryPlayer1.generalInvCurrent = GeneralInvButtonIndex; //Set current
		useableItemIndex =
			inventoryPlayer1.generalInventoryIndexRef[GeneralInvButtonIndex];

		// Access Cards
		if (GeneralInvButtonIndex == 0) {
			MFDManager.a.SendInfoToItemTab(81);
			if (MFDManager.a.lastItemSideRH) {
				MFDManager.a.rightTC.SetCurrentAsLast();
			} else {
				MFDManager.a.leftTC.SetCurrentAsLast();
			}
		} else {
			MFDManager.a.SendInfoToItemTab(useableItemIndex,customIndex);
			if (MFDManager.a.lastItemSideRH) {
				MFDManager.a.rightTC.SetCurrentAsLast();
			} else {
				MFDManager.a.leftTC.SetCurrentAsLast();
			}
		}
    }

    public void DoubleClick() {
        inventoryPlayer1.generalInvCurrent = GeneralInvButtonIndex; //Set current
		MFDManager.a.mouseClickHeldOverGUI = true;
		GeneralInvApply();
	}

	void ApplyBattery() {
		if (PlayerEnergy.a.energy >= 255f) {
			CenterStatusPrint("%s", Sys_Text.stringTable[303]);
			reduce = false;
		}

		PlayerEnergy.a.GiveEnergy(83f,EnergyType.Battery);
		reduce = true;
	}

	void ApplyIcadBattery() {
		if (PlayerEnergy.a.energy >= 255f) {
			CenterStatusPrint("%s", Sys_Text.stringTable[303]);
			reduce = false;
			return;
		}

		PlayerEnergy.a.GiveEnergy(255f,EnergyType.Battery);
		reduce = true;
	}

	void ApplyHealthkit() {
		if (PlayerHealth.a.hm.health >= PlayerHealth.a.hm.maxhealth) {
			CenterStatusPrint("%s", Sys_Text.stringTable[304]);
			reduce = false;
			return;
		}

		PlayerHealth.a.hm.health = PlayerHealth.a.hm.maxhealth;
		MFDManager.a.DrawTicks(true);
		reduce = true;
	}

	public void GeneralInvApply() {
		// Access Cards button
		if (GeneralInvButtonIndex == 0) {
			MFDManager.a.SendInfoToItemTab(81);
			MFDManager.a.OpenTab(1,true,TabMSG.None, useableItemIndex,
								 Handedness.LH);
			return;
		}

        reduce = false;
		useableItemIndex =
			inventoryPlayer1.generalInventoryIndexRef[GeneralInvButtonIndex];
		switch (useableItemIndex) {
			case 52: ApplyBattery(); break;
			case 53: ApplyIcadBattery(); break;
			case 55: ApplyHealthkit(); break;
			default:
				MFDManager.a.SendInfoToItemTab(useableItemIndex,customIndex);
				MFDManager.a.OpenTab(1,true,TabMSG.None, useableItemIndex,
									 Handedness.LH);

				// Set current.
				inventoryPlayer1.generalInvCurrent = GeneralInvButtonIndex;
				break;
		}

		if (reduce)  {
			inventoryPlayer1.generalInventoryIndexRef[GeneralInvButtonIndex] = -1;
			GUIState.a.ClearOverButton();
		}
	}
}
