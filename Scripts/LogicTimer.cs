using UnityEngine;
using System.Text;

public class LogicTimer : MonoBehaviour {
	float timeInterval = 0.35f;
	float randomMin = 5f;
	float randomMax = 10f;
	bool useRandomTimes = false;
	bool active = true;
	float intervalFinished;
	string target;

	void Start() {
		intervalFinished = Eng_Global->pauseRelativeTime + (useRandomTimes ? random_range(randomMin,randomMax) : timeInterval);
	}

	void Update() {
		if (!Eng_Global->gamePaused && !Eng_Global->menuActive && active) {
			if (intervalFinished < Eng_Global->pauseRelativeTime) {
				if (useRandomTimes) {
					intervalFinished = Eng_Global->pauseRelativeTime
									   + random_range(randomMin,randomMax);
				} else {
					intervalFinished = Eng_Global->pauseRelativeTime
									   + timeInterval;
				}
				UseTargets();
			}
		}
	}

	public void Targetted (UseData ud) {
		active = !active;
	}

	public void UseTargets () {
		UseData ud = new UseData();
		UseTargets(gameObject,ud,target);
	}
}
