using UnityEngine;
using System.Collections;

public class PooledItemDestroy : MonoBehaviour {
	public float itemLifeTime = 3.00f;
	public bool onlyOnce = false;
	private bool doneYet = false;
	private float timerFinished = 9999999f;

	void OnEnable () {
		timerFinished = Eng_Global->pauseRelativeTime + itemLifeTime;
	}

	void Update() {
		if (onlyOnce && doneYet) return;

		if (timerFinished < Eng_Global->pauseRelativeTime) {
			timerFinished = 9999999f;
			if (onlyOnce) doneYet = true;
			flag_set(&SELF.entflags, ENTFLAG_ACTIVE, false);
		}
	}
}
