using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Email : MonoBehaviour {
	public int emailIndex;
	public bool autoPlayEmail = false;

    public void Targetted() {
		// Give email.
		if (inventoryPlayer1.hasLog[emailIndex]) return; // Already have it.

		inventoryPlayer1.hasLog[emailIndex] = true;
		inventoryPlayer1.hasNewEmail = true;
		inventoryPlayer1.lastAddedIndex = emailIndex;
		if (Const.a.audioLogType[emailIndex] == AudioLogType.Email) {
			inventoryPlayer1.beepDone = true;
		}

		if (autoPlayEmail) inventoryPlayer1.PlayLastAddedLog(emailIndex);
	}
}
