using System.Collections;
using System.Collections.Generic;
using System.Text;
using UnityEngine;

public class Radiation : MonoBehaviour {
	public float radiationAmount = 11f;
	public float intervalTime = 1f;
	public float radFinished = 0f;
	private static StringBuilder s1 = new StringBuilder();

	void Start() {
		radFinished = Eng_Global->pauseRelativeTime + (intervalTime * 2);
	}

	void OnTriggerEnter (Collider col) {
		if (col.gameObject.CompareTag("Player")) {
			if (Eng_Global->instances[PLAYER1].health > 0f) {
				PlayerHealth.a.radiationArea = true;
				PlayerHealth.a.GiveRadiation(radiationAmount);
				radFinished = Eng_Global->pauseRelativeTime + (intervalTime*random_range(1f,1.5f));
			}
		}
	}

	void  OnTriggerStay (Collider col) {
		if (col.gameObject.CompareTag("Player")) {
			if (Eng_Global->instances[PLAYER1].health > 0f && (radFinished < Eng_Global->pauseRelativeTime)) {
				PlayerHealth.a.radiationArea = true;
				PlayerHealth.a.GiveRadiation(radiationAmount);
				radFinished = Eng_Global->pauseRelativeTime + (intervalTime*random_range(1f,1.5f));
			}
		}
	}

	void OnTriggerExit (Collider col) {
		if (col.gameObject.CompareTag("Player")) { 
			if (Eng_Global->instances[PLAYER1].health > 0f) {
				PlayerHealth.a.radiationArea = false;
				radFinished = Eng_Global->pauseRelativeTime;  // reset so re-triggering is instant
			}
		}
	}
	
	void OnDisable() {
		PlayerHealth.a.radiationArea = false;
	}
}
