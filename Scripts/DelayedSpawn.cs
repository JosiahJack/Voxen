
public class DelayedSpawn : MonoBehaviour {
    float delay = 0.5f; // save
	GameObject[] objectsToSpawn;
	bool despawnInstead = false;
	bool doSelfAfterList = false;
	bool destroyAfterListInsteadOfDeactivate = false;
	float timerFinished;
	bool active;

	void OnEnable() {
		if (PauseScript.a != null) timerFinished = Sys_Global.pauseRelativeTime + delay;
        else timerFinished = delay;

		active = true;
    }

	void DelayedSpawnUpdate() {
		if (!active) return;
		if (timerFinished >= Sys_Global.pauseRelativeTime) return;

		active = false; // Once only, unless we do self after the list.
		for (int i=0;i<objectsToSpawn.Length;i++) {
			if (despawnInstead) {
				if (objectsToSpawn[i] != null) flag_set(&instances[objectsToSpawn[i]].entflags, ENTFLAG_ACTIVE, false);
			} else {
				if (objectsToSpawn[i] != null) flag_set(&instances[objectsToSpawn[i]].entflags, ENTFLAG_ACTIVE, true);
			}
		}

		if (doSelfAfterList) {
			if (despawnInstead) {
				if (destroyAfterListInsteadOfDeactivate) {
					DeleteInstance(selfIdx);
				} else {
                    flag_set(&SELF.entflags, ENTFLAG_ACTIVE, false);
					flag_set(&SELF.entflags, ENTFLAG_ACTIVE, false);
				}
			} else {
				gameObject.SetActive(true);
			}
		}
    }
}
