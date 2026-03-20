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
		Eng_UI->mouseClickHeldOverGUI = true;
		remainder = logTextOutput.GetComponent<Text>().text;
		if (remainder.Length>568) {
			// MORE BUTTON
			remainder = remainder.Remove(0,568);
			logTextOutput.GetComponent<Text>().text = remainder;
		} else {
			// CLOSE BUTTON
			Eng_UI->ResetMultiMediaTabs();
			Eng_UI->ClearDataTab(true);
			Eng_UI->ClearDataTab(false);
			Eng_UI->leftTC.ReturnToLastTab();
			Eng_UI->rightTC.ReturnToLastTab();
			Eng_UI->CenterTabButtonClickSilent(0,true);
			GetComponent<UIButtonMask>().PtrExit(); // Force mouse cursor out of UI.
		}
	}

	void Start() {
		GetComponent<Button>().onClick.AddListener(() => { LogMoreButtonClick(); });
	}
}
