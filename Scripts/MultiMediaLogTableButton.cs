using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;
using System.Collections;

public class MultiMediaLogTableButton : MonoBehaviour {
	public int logTableButtonIndex;

	void LogTableButtonClick() {
		Eng_Global->inventoryPlayer1.hardwareIsActive[2] = true;
		Eng_UI->OpenEReaderInItemsTab();
		Eng_UI->mouseClickHeldOverGUI = true;
		Eng_UI->OpenLogsLevelFolder(logTableButtonIndex);
	}

	void Start() {
		GetComponent<Button>().onClick.AddListener(() => { LogTableButtonClick(); });
	}
}
