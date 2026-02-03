using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;
using System.Collections;
using System.Collections.Generic;

public class LogMoreButton : MonoBehaviour {
	public GameObject logTextOutput;
	public GameObject multiMediaTab;
	private string remainder = System.String.Empty;

	void LogMoreButtonClick() {
		Sys_UI.mouseClickHeldOverGUI = true;
		remainder = logTextOutput.GetComponent<Text>().text;
		if (remainder.Length>568) {
			// MORE BUTTON
			remainder = remainder.Remove(0,568);
			logTextOutput.GetComponent<Text>().text = remainder;
		} else {
			// CLOSE BUTTON
			Sys_UI.ResetMultiMediaTabs();
			Sys_UI.ClearDataTab(true);
			Sys_UI.ClearDataTab(false);
			Sys_UI.leftTC.ReturnToLastTab();
			Sys_UI.rightTC.ReturnToLastTab();
			Sys_UI.CenterTabButtonClickSilent(0,true);
			GetComponent<UIButtonMask>().PtrExit(); // Force mouse cursor out of UI.
		}
	}

	void Start() {
		GetComponent<Button>().onClick.AddListener(() => { LogMoreButtonClick(); });
	}
}
