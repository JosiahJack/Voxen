using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class TriggerCounter : MonoBehaviour {
	public int countToTrigger;
	public int counter;
	public string target;
	public string argvalue;
	public float delay;
	public bool dontReset;

	void Target(UseData ud) {
		ud.argvalue = argvalue;
		UseTargets(gameObject,ud,target);
	}

    IEnumerator DelayedTarget(UseData ud) {
        yield return new WaitForSeconds(delay);
        Target(ud);
    }
}
