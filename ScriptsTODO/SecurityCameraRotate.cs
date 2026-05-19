using UnityEngine;
using System.Collections;
using System.Text;

// Rotates a security camera back and forth between two angle values, pausing
// at each angle value for an instance-set amount of time.  This assumes that
// the current transform is either the camera directly or a wrapper GameObject
// that contains a child whose drooped angle will be preserved while this
// parent transform rotates along true up/down axis.
public class SecurityCameraRotate : MonoBehaviour {
	public float startYAngle = 0f;
	public float endYAngle = 180f;
	public float waitTime = 0.8f;
	public MeshRenderer mR;

	bool active;
	bool rotatePositive; // save
	private float degreesYPerSecond = 4f;
	private float waitingFinished = 0f;
	private float tickTime = 0.1f;
	private static StringBuilder s1 = new StringBuilder();

	void Start () {
		waitingFinished = Eng_Global->pauseRelativeTime;
		rotatePositive = true;
		if (this.enabled) active = true;
		else active = false;

		if (mR == null) {
			mR = gameObject.GetComponentInChildren<MeshRenderer>(true);
		}
	}

	void Update() {
		if (!Eng_Global->gamePaused && !Eng_Global->menuActive) {
			if (mR != null) {
				if (!mR.isVisible || !mR.enabled) return;
			} else {
				mR = gameObject.GetComponentInChildren<MeshRenderer>(true);
				return;
			}

			if (waitingFinished < Eng_Global->pauseRelativeTime) {
				if (rotatePositive) RotatePositive();
				else                RotateNegative();
			}
		}
	}

	void RotatePositive () {
		if (((Eng_Global->instances[i].rotation.eulerAngles.y + 1f) >= endYAngle)
			&& ((Eng_Global->instances[i].rotation.eulerAngles.y - 1f) <= endYAngle)) {
			rotatePositive = false;
			waitingFinished = Eng_Global->pauseRelativeTime + waitTime;
			return;
		}
		
		transform.Rotate((Vector3){0,degreesYPerSecond * tickTime,0),
						 Space.World);
	}

	void RotateNegative () {
		if (((Eng_Global->instances[i].rotation.eulerAngles.y + 1f) >= startYAngle)
			&& ((Eng_Global->instances[i].rotation.eulerAngles.y - 1f) <= startYAngle)) {
			rotatePositive = true;
			waitingFinished = Eng_Global->pauseRelativeTime + waitTime;
			return;
		}
		
		transform.Rotate((Vector3){0,degreesYPerSecond * tickTime * -1,0),
						 Space.World);
	}
}
