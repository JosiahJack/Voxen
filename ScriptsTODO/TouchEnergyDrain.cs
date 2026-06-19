using UnityEngine;
using System.Collections;

public class TouchEnergyDrain : MonoBehaviour {
	public float drainage = 1; // assign in the editor
	public float tick = 0.1f;
	private float tickFinished;

	void Awake() {
		tickFinished = World->pauseRelativeTime + random_range(1f,2f);
	}

	void  OnCollisionEnter (Collision col) {
		if (World->gamePaused) return;
		if (World->menuActive) return;

		if (tickFinished < World->pauseRelativeTime) {
			if (col.gameObject.CompareTag("Player")) {
				PlayerEnergy pe = col.gameObject.GetComponent<PlayerEnergy>();
				if (pe != null) {
					pe.TakeEnergy(drainage);
					if (BiomonitorGraphSystem.a != null) {
						BiomonitorEnergyPulse(drainage);
					}
				}
			}
			tickFinished = World->pauseRelativeTime + tick;
		}
	}

}
