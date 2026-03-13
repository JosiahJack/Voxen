using UnityEngine;
using System.Collections;

public class TouchEnergyDrain : MonoBehaviour {
	public float drainage = 1; // assign in the editor
	public float tick = 0.1f;
	private float tickFinished;

	void Awake() {
		tickFinished = Eng_Global->pauseRelativeTime + random_range(1f,2f);
	}

	void  OnCollisionEnter (Collision col) {
		if (Eng_Global->gamePaused) return;
		if (Eng_Global->menuActive) return;

		if (tickFinished < Eng_Global->pauseRelativeTime) {
			if (col.gameObject.CompareTag("Player")) {
				PlayerEnergy pe = col.gameObject.GetComponent<PlayerEnergy>();
				if (pe != null) {
					pe.TakeEnergy(drainage);
					if (BiomonitorGraphSystem.a != null) {
						BiomonitorEnergyPulse(drainage);
					}
				}
			}
			tickFinished = Eng_Global->pauseRelativeTime + tick;
		}
	}

}
