using UnityEngine;
using UnityEngine.UI;
using System.Collections;

[System.Serializable]
public class LogCountsText : MonoBehaviour {
	Text text;
	int tempint;
	public int countsSlotnum = 0;
	
	void Start () {
		text = GetComponent<Text>();
	}

	void Update() {
		if (!Eng_Global->gamePaused && !Eng_Global->menuActive) {
			tempint = inventoryPlayer1.numLogsFromLevel[countsSlotnum];
			if (tempint > 0)text.text = inventoryPlayer1.numLogsFromLevel[countsSlotnum].ToString();
			else 			text.text = " ";  // Blank out the text
		}
	}
}
