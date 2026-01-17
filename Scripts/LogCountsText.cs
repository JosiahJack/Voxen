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
		if (!Sys_Global.gamePaused && !Sys_Global.menuActive) {
			tempint = Inventory.a.numLogsFromLevel[countsSlotnum];
			if (tempint > 0)text.text = Inventory.a.numLogsFromLevel[countsSlotnum].ToString();
			else 			text.text = " ";  // Blank out the text
		}
	}
}
