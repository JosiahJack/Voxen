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
		Sys_UI.mouseClickHeldOverGUI = true;
		GeneralInvUse();
	}

	public void GeneralInvUse() {
        Eng_Global->inventoryPlayer1.generalInvCurrent = GeneralInvButtonIndex; //Set current
		useableItemIndex =
			Eng_Global->inventoryPlayer1.generalInventoryIndexRef[GeneralInvButtonIndex];

		// Access Cards
		if (GeneralInvButtonIndex == 0) {
			Sys_UI.SendInfoToItemTab(81);
			if (Sys_UI.lastItemSideRH) {
				Sys_UI.rightTC.SetCurrentAsLast();
			} else {
				Sys_UI.leftTC.SetCurrentAsLast();
			}
		} else {
			Sys_UI.SendInfoToItemTab(useableItemIndex,customIndex);
			if (Sys_UI.lastItemSideRH) {
				Sys_UI.rightTC.SetCurrentAsLast();
			} else {
				Sys_UI.leftTC.SetCurrentAsLast();
			}
		}
    }

    public void DoubleClick() {
        Eng_Global->inventoryPlayer1.generalInvCurrent = GeneralInvButtonIndex; //Set current
		Sys_UI.mouseClickHeldOverGUI = true;
		GeneralInvApply();
	}

	void ApplyBattery() {
		if (PlayerEnergy.a.energy >= 255f) {
			CenterStatusPrint("%s", Sys_Text.stringTable[303]);
			reduce = false;
		}

		GiveEnergy(83f,EnergyType_Battery);
		reduce = true;
	}

	void ApplyIcadBattery() {
		if (PlayerEnergy.a.energy >= 255f) {
			CenterStatusPrint("%s", Sys_Text.stringTable[303]);
			reduce = false;
			return;
		}

		GiveEnergy(255f,EnergyType_Battery);
		reduce = true;
	}

	void ApplyHealthkit() {
		if (Eng_Global->instances[PLAYER1].health >= Eng_Global->instances[PLAYER1].maxhealth) {
			CenterStatusPrint("%s", Sys_Text.stringTable[304]);
			reduce = false;
			return;
		}

		Eng_Global->instances[PLAYER1].health = Eng_Global->instances[PLAYER1].maxhealth;
		Sys_UI.DrawTicks(true);
		reduce = true;
	}

	public void GeneralInvApply() {
		// Access Cards button
		if (GeneralInvButtonIndex == 0) {
			Sys_UI.SendInfoToItemTab(81);
			Sys_UI.OpenTab(1,true,TabMSG.None, useableItemIndex,
								 Handedness.LH);
			return;
		}

        reduce = false;
		useableItemIndex =
			Eng_Global->inventoryPlayer1.generalInventoryIndexRef[GeneralInvButtonIndex];
		switch (useableItemIndex) {
			case 52: ApplyBattery(); break;
			case 53: ApplyIcadBattery(); break;
			case 55: ApplyHealthkit(); break;
			default:
				Sys_UI.SendInfoToItemTab(useableItemIndex,customIndex);
				Sys_UI.OpenTab(1,true,TabMSG.None, useableItemIndex,
									 Handedness.LH);

				// Set current.
				Eng_Global->inventoryPlayer1.generalInvCurrent = GeneralInvButtonIndex;
				break;
		}

		if (reduce)  {
			Eng_Global->inventoryPlayer1.generalInventoryIndexRef[GeneralInvButtonIndex] = -1;
			
		}
	}
}
