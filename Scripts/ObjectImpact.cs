using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class ObjectImpact : MonoBehaviour {
	// External values set per prefab instance, optional.
	public float minVolumeSpeed = 2f;
	public float maxVolumeSpeed = 10f;
	public int impactSFXIndex = 523;

	// Internal references, required
	AudioSource SFXSource;
	Rigidbody rbody;
	Vector3 oldVelocity;

	void Start () {
		rbody = GetComponent<Rigidbody>();
		if (rbody == null) this.enabled = false;
		SFXSource = GetComponent<AudioSource>();
	}

	void OnCollisionEnter(Collision collision) {
		if (Sys_Global.gamePaused) return;
		if (Sys_Global.menuActive) return;
		if (collision == null) return;

		if (collision.relativeVelocity.sqrMagnitude > (minVolumeSpeed * minVolumeSpeed)) {
			if (SFXSource != null) {
				SFXSource.pitch = (random_range(0.8f,1.2f));
				float vol = (collision.relativeVelocity.magnitude/maxVolumeSpeed) * 0.3f;
				Utils.PlayOneShotSavable(SFXSource,sounds[impactSFXIndex],vol); // Play sound when object changes velocity significantly enough that it must have hit something
			}
		}
	}
}
