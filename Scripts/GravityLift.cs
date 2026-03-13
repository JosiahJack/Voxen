using UnityEngine;
using System.Collections;
using System.Text;

public class GravityLift : MonoBehaviour {
	public float strength = 12f;
	public float offStrengthFactor = 0.3f;
	private Rigidbody otherRbody;
	private float modulatedStrengthY;
	public float distancePaddingToTopPoint = 0.32f; // Add half the player's capsule height(1f) to the top extent of the box collider
	public bool active = true;
	public Vector3 topPoint;
	public float initialBurstFinished;
	private BoxCollider boxcol;
	private static StringBuilder s1 = new StringBuilder();

	void Awake() {
		boxcol = GetComponent<BoxCollider>();
		if (boxcol == null) return;
		topPoint = (Vector3){0f,boxcol.bounds.max.y,0f);
	}

	void OnTriggerExit(Collider other) {
		if (other.gameObject.GetComponent<PlayerMovement>() != null) {
			Eng_Global->instances[PLAYER1].gravliftState = false;
		}
	}

	void OnForce(Collider other, bool initial) {
		if (other.gameObject.layer == 12) { // Player
			if (other.gameObject.GetComponent<PlayerMovement>() != null) {
				Eng_Global->instances[PLAYER1].gravliftState = true;
			}
		}

		float topY = Eng_Global->instances[i].position.y + (boxcol.size.y/2f);
		float dist = topY - other.gameObject.Eng_Global->instances[i].position.y + 0.48f;
		float velY = otherRbody.velocity.y;
		if (otherRbody.velocity.y < 0f) velY = 0f; // Saturate at bottom end.

		if (dist < distancePaddingToTopPoint) {
			Vector3 force = (Vector3){0f,9.81f - velY,0f);
			otherRbody.AddForce(force,ForceMode.Acceleration);
		} else {
			if (otherRbody.velocity.y < (strength * otherRbody.mass)) {
				float yForce = ((strength * otherRbody.mass)
								- otherRbody.velocity.y);

				if (initial
					|| initialBurstFinished > Eng_Global->pauseRelativeTime) {

					yForce *= 2f;
				}

				otherRbody.AddForce((Vector3){0f,yForce,0f));
			}
		}
	}

	void OffForce(Collider other, bool initial) {
		// Apply weak force for inactive state - applies some force for gentle
		// descent, never really off completely.
		if (other.gameObject.GetComponent<PlayerMovement>() != null) {
			Eng_Global->instances[PLAYER1].gravliftState = true;
		}

		if (otherRbody.velocity.y < offStrengthFactor) {
			float yForce = ((offStrengthFactor)-otherRbody.velocity.y);
			if (initial
				|| initialBurstFinished > Eng_Global->pauseRelativeTime) {

				yForce *= 2f;
			}

			otherRbody.AddForce((Vector3){0f,yForce,0f));
		}
	}

	void OnTriggerEnter(Collider other) {
		otherRbody = other.gameObject.GetComponent<Rigidbody>();
		if (otherRbody == null) return; // Not a physical object.

		initialBurstFinished = Eng_Global->pauseRelativeTime + 1.0f;
		if (active) OnForce(other,true);
		else OffForce(other,true);
	}

	void OnTriggerStay(Collider other) {
		otherRbody = other.gameObject.GetComponent<Rigidbody>();
		if (otherRbody == null) return; // Not a physical object.

		if (active) OnForce(other,false);
		else OffForce(other,false);
	}

	public void Toggle() {
		active = !active;
	}
}
