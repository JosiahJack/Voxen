using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;
using System.Collections;

public class MultiMediaLogTableButton : MonoBehaviour {
	public int logTableButtonIndex;

	void LogTableButtonClick() {
		inventoryPlayer1.hardwareIsActive[2] = true;
		Sys_UI.OpenEReaderInItemsTab();
		Sys_UI.mouseClickHeldOverGUI = true;
		Sys_UI.OpenLogsLevelFolder(logTableButtonIndex);
	}

	void Start() {
		GetComponent<Button>().onClick.AddListener(() => { LogTableButtonClick(); });
	}
}
