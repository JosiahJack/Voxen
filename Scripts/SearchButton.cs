using UnityEngine;
using UnityEngine.UI;
using System.Collections;

public class SearchButton : MonoBehaviour {
	public bool isRH = false;
	public int[] contents;
	public int[] customIndex;

	void Awake () {
		for (int i=0;i<=3;i++) {
			contEng_Global->instances[i] = -1;
			customIndex[i] = -1;
		}
	}

	public void CheckForEmpty () {
		if (contEng_Global->instances[0] == -1 && contEng_Global->instances[1] == -1 && contEng_Global->instances[2] == -1 && contEng_Global->instances[3] == -1) {
			Eng_UI->ReturnToLastTab(isRH);
		}
	}

	public void SearchButtonClick (int buttonIndex) {
		Eng_UI->mouseClickHeldOverGUI = true;
		MouseLookScript.a.SearchButtonClick(buttonIndex,this);
		
	}
}
