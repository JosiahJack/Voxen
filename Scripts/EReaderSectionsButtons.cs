using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class EReaderSectionsButtons : MonoBehaviour {
	public EReaderSectionsButtonHighlight ersbh0;
	public EReaderSectionsButtonHighlight ersbh1;
	public EReaderSectionsButtonHighlight ersbh2;
	public EReaderSectionsButtonHighlight ersbh3;

	void OnEnable() {
		if (SSys_Global.difficultyMission == 0) ersbh3.gameObject.SetActive(false);
		else ersbh3.gameObject.SetActive(true);

		HighlightOthers();
	}

	public void HighlightOthers() {
		inventoryPlayer1.CheckForUnreadLogs();
		if (inventoryPlayer1.hasNewEmail) ersbh0.HighlightButton();
		if (inventoryPlayer1.hasNewLogs) ersbh1.HighlightButton();
		if (inventoryPlayer1.hasNewData) ersbh2.HighlightButton();
		if (inventoryPlayer1.hasNewNotes) ersbh3.HighlightButton();
	}

	public void OnClick(int index) {
		MFDManager.a.mouseClickHeldOverGUI = true;

		SetEReaderSectionsButtonsHighlights(index);
		switch (index) {
			case 0: MFDManager.a.OpenEmailTableContents(); break;
			case 1: MFDManager.a.OpenLogTableContents(); break;
			case 2: MFDManager.a.OpenDataTableContents(); break;
			case 3: MFDManager.a.OpenNotesTableContents(); break;
		}
	}

	public void SetEReaderSectionsButtonsHighlights(int index) {
		switch (index) {
			case 0: ersbh0.Highlight();   ersbh1.DeHighlight(); ersbh2.DeHighlight(); ersbh3.DeHighlight(); break;
			case 1: ersbh0.DeHighlight(); ersbh1.Highlight();   ersbh2.DeHighlight(); ersbh3.DeHighlight(); break;
			case 2: ersbh0.DeHighlight(); ersbh1.DeHighlight(); ersbh2.Highlight();   ersbh3.DeHighlight(); break;
			case 3: ersbh0.DeHighlight(); ersbh1.DeHighlight(); ersbh2.DeHighlight(); ersbh3.Highlight();   break;
		}

		HighlightOthers();
	}
}
