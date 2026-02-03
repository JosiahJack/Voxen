using System.Collections;
using System.Collections.Generic;
using System.Text;
using UnityEngine;

public class Trigger : MonoBehaviour {
	public float delay = 0;
	public bool onlyOnce = false;
	public bool ignoreSecondaryTriggers = false;
	public int numPlayers = 0;
	public string target;
	public string argvalue; // e.g. how much to set a counter to
	GameObject recentMostActivator;
	float delayFireFinished;
	float delayResetFinished;
	bool allDone = false;
	private static StringBuilder s1 = new StringBuilder();

    IEnumerator DelayedTarget(GameObject activator) {
        yield return new WaitForSeconds(delay);
        UseTargets(activator);
    }

	public void UseTargets (GameObject activator) {
		UseData ud = new UseData();
		ud.owner = activator;
		ud.argvalue = argvalue;
		UseTargets(gameObject,ud,target);
	}

	void TriggerTripped (Collider col, bool initialEntry) {
		if (col == null) DualLog("BUG: TriggerTripped was fed a null col!");

		if (col.gameObject.CompareTag("Player")) {
			HealthManager hm = Utils.GetMainHealthManager(col.gameObject);
			if (hm != null) {
				if (hm.health > 0f && hm.isPlayer) {
					if (recentMostActivator != null) {
						if (ignoreSecondaryTriggers) return;
					}
					recentMostActivator = col.gameObject;

					if (initialEntry && recentMostActivator.CompareTag("Player")) numPlayers++;
					if (onlyOnce) allDone = true;
					
					if (delay <=0) {
						UseTargets(recentMostActivator);
					} else {
						StartCoroutine(DelayedTarget(recentMostActivator));
					}
				}
			}
		}
	}

	void OnTriggerEnter (Collider col) {
		if (allDone) return;
		if (col == null) return;
		if (col.gameObject == null) return;
		if (col.gameObject.CompareTag("Player"))
			TriggerTripped(col,true);
	}

	void  OnTriggerStay (Collider col) {
		if (allDone) return;
		if (col == null) return;
		if (col.gameObject == null) return;
		if (col.gameObject.CompareTag("Player"))
			TriggerTripped (col, false);
	}

	void OnTriggerExit (Collider col) {
		if (allDone) return;
		if (col.gameObject.CompareTag("Player")) numPlayers--;
	}
