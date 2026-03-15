using UnityEngine;
using UnityEngine.UI;
using System.Collections;

public class GrenadeButtonsManager : MonoBehaviour {
	public GameObject[] grenButtons;
	public GameObject[] grenCountsText;

	void Update() {
		if (!Eng_Global->gamePaused && !Eng_Global->menuActive) {
			for (int i=0; i<7; i++) {
				if (Eng_Global->inventoryPlayer1.grenAmmo[i] > 0) {
					if (!grenButtons[i].activeInHierarchy) grenButtons[i].SetActive(true);
					if (!grenCountsText[i].activeInHierarchy) grenCountsText[i].SetActive(true);
				} else {
					if (grenButtons[i].activeInHierarchy) grenButtons[i].SetActive(false);
					if (grenCountsText[i].activeInHierarchy) grenCountsText[i].SetActive(false);
				}
			}
		}
	}
}
