using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Email : MonoBehaviour {
	public int emailIndex;
	public bool autoPlayEmail = false;

    public void Targetted() {
		// Give email.
		if (Eng_Global->inventoryPlayer1.hasLog[emailIndex]) return; // Already have it.

		Eng_Global->inventoryPlayer1.hasLog[emailIndex] = true;
		Eng_Global->inventoryPlayer1.hasNewEmail = true;
		Eng_Global->inventoryPlayer1.lastAddedIndex = emailIndex;
		if (Const.a.audioLogType[emailIndex] == AudioLogType.Email) {
			Eng_Global->inventoryPlayer1.beepDone = true;
		}

		if (autoPlayEmail) Eng_Global->inventoryPlayer1.PlayLastAddedLog(emailIndex);
	}
}
