using UnityEngine;
using UnityEngine.UI;
using System.Collections;

public class LogTableContentsButtonsManager : MonoBehaviour {
	public GameObject[] LogButtons;

	void Update() {
		if (!Sys_Global.gamePaused && !Sys_Global.menuActive) {
			for (int i=0; i<10; i++) {
				// Only show category buttons for levels we have logs from
				if (inventoryPlayer1.numLogsFromLevel[i] > 0) {
					LogButtons[i].SetActive(true);
				} else {
					LogButtons[i].SetActive(false);
				}
			}
		}
	}
}
