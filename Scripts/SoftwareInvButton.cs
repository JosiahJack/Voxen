using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class SoftwareInvButton : MonoBehaviour {
	public int index = 0;

	public void DoubleClick() {
		Sys_UI.mouseClickHeldOverGUI = true;
		SoftInvClick();
	}

    public void SoftInvClick() {
		Sys_UI.mouseClickHeldOverGUI = true;
		switch(index) {
			case 0:
					// Drill
					Eng_Global->inventoryPlayer1.pulserButtonText.Select(false);
					Eng_Global->inventoryPlayer1.drillButtonText.Select(true);
					Eng_Global->inventoryPlayer1.isPulserNotDrill = false;
					Utils.PlayUIOneShotSavable(80); // changeweapon
					break;
			case 1:
					// Pulser
					Eng_Global->inventoryPlayer1.pulserButtonText.Select(true);
					Eng_Global->inventoryPlayer1.drillButtonText.Select(false);
					Eng_Global->inventoryPlayer1.isPulserNotDrill = true;
					Utils.PlayUIOneShotSavable(80); // changeweapon
					break;
			case 2:
					// CyberShield
					if (MouseLookScript.a.inCyberSpace) {
						CenterStatusPrint("%s", Sys_Text.stringTable[461],Const.a.player1);
					} else {
						CenterStatusPrint("%s", Sys_Text.stringTable[460],Const.a.player1);
					}
					break;
			case 3:
					// Turbo
					if (MouseLookScript.a.inCyberSpace) {
						Eng_Global->inventoryPlayer1.UseTurbo();
						
					} else {
						CenterStatusPrint("%s", Sys_Text.stringTable[460],Const.a.player1);
					}
					break;
			case 4:
					// Decoy
					if (MouseLookScript.a.inCyberSpace) {
						Eng_Global->inventoryPlayer1.UseDecoy();
						
					} else {
						CenterStatusPrint("%s", Sys_Text.stringTable[460],Const.a.player1);
					}
					break;
			case 5:
					// Recall
					if (MouseLookScript.a.inCyberSpace) {
						Eng_Global->inventoryPlayer1.UseRecall();
						
					} else {
						CenterStatusPrint("%s", Sys_Text.stringTable[460],Const.a.player1);
					}
					break;
			case 6:
					// Games
					if (MouseLookScript.a.inCyberSpace) {
						CenterStatusPrint("%s", Sys_Text.stringTable[443],Const.a.player1);
					} else {
						Sys_UI.OpenMinigames();
						CenterStatusPrint("%s", Sys_Text.stringTable[309],Const.a.player1); // Trioptimum Funpack Module, don't play on company time!
					}
					break;
		}
	}
}
