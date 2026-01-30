using UnityEngine;
using System.Collections;

public class TouchEnergyDrain : MonoBehaviour {
	public float drainage = 1; // assign in the editor
	public float tick = 0.1f;
	private float tickFinished;

	void Awake() {
		tickFinished = Sys_Global.pauseRelativeTime + random_range(1f,2f);
	}

	void  OnCollisionEnter (Collision col) {
		if (Sys_Global.gamePaused) return;
		if (Sys_Global.menuActive) return;

		if (tickFinished < Sys_Global.pauseRelativeTime) {
			if (col.gameObject.CompareTag("Player")) {
				PlayerEnergy pe = col.gameObject.GetComponent<PlayerEnergy>();
				if (pe != null) {
					pe.TakeEnergy(drainage);
					if (BiomonitorGraphSystem.a != null) {
						BiomonitorEnergyPulse(drainage);
					}
				}
			}
			tickFinished = Sys_Global.pauseRelativeTime + tick;
		}
	}

}
