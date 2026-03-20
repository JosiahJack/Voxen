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
		Eng_UI->mouseClickHeldOverGUI = true;
		GeneralInvUse();
	}

	public void GeneralInvUse() {
        Eng_Global->inventoryPlayer1.generalInvCurrent = GeneralInvButtonIndex; //Set current
		useableItemIndex =
			Eng_Global->inventoryPlayer1.generalInventoryIndexRef[GeneralInvButtonIndex];

		// Access Cards
		if (GeneralInvButtonIndex == 0) {
			Eng_UI->SendInfoToItemTab(81);
			if (Eng_UI->lastItemSideRH) {
				Eng_UI->rightTC.SetCurrentAsLast();
			} else {
				Eng_UI->leftTC.SetCurrentAsLast();
			}
		} else {
			Eng_UI->SendInfoToItemTab(useableItemIndex,customIndex);
			if (Eng_UI->lastItemSideRH) {
				Eng_UI->rightTC.SetCurrentAsLast();
			} else {
				Eng_UI->leftTC.SetCurrentAsLast();
			}
		}
    }

    public void DoubleClick() {
        Eng_Global->inventoryPlayer1.generalInvCurrent = GeneralInvButtonIndex; //Set current
		Eng_UI->mouseClickHeldOverGUI = true;
		GeneralInvApply();
	}

	void ApplyBattery() {
		if (PlayerEnergy.a.energy >= 255f) {
			CenterStatusPrint("%s", Eng_Text->stringTable[303]);
			reduce = false;
		}

		GiveEnergy(83f,EnergyType_Battery);
		reduce = true;
	}

	void ApplyIcadBattery() {
		if (PlayerEnergy.a.energy >= 255f) {
			CenterStatusPrint("%s", Eng_Text->stringTable[303]);
			reduce = false;
			return;
		}

		GiveEnergy(255f,EnergyType_Battery);
		reduce = true;
	}

	void ApplyHealthkit() {
		if (Eng_Global->instances[PLAYER1].health >= Eng_Global->instances[PLAYER1].maxhealth) {
			CenterStatusPrint("%s", Eng_Text->stringTable[304]);
			reduce = false;
			return;
		}

		Eng_Global->instances[PLAYER1].health = Eng_Global->instances[PLAYER1].maxhealth;
		Eng_UI->DrawTicks(true);
		reduce = true;
	}

	public void GeneralInvApply() {
		// Access Cards button
		if (GeneralInvButtonIndex == 0) {
			Eng_UI->SendInfoToItemTab(81);
			Eng_UI->OpenTab(1,true,TabMSG.None, useableItemIndex,
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
				Eng_UI->SendInfoToItemTab(useableItemIndex,customIndex);
				Eng_UI->OpenTab(1,true,TabMSG.None, useableItemIndex,
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
