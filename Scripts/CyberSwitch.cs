using System.Collections;
using System.Collections.Generic;
using System.Text;
using UnityEngine;

public class CyberSwitch : MonoBehaviour {
	public int textIndex = 463;
	public bool active = false; // save
	public string target;
	public GameObject activeCenter;
	public GameObject deactiveCenter;
	public GameObject iceNode;
	bool iceActive;
	private static StringBuilder s1 = new StringBuilder();

	void Awake() {
		if (iceNode != null && iceNode.activeSelf) iceActive = true;
		Initialize(active,iceActive);
	}

	public void Initialize(bool startOn, bool turnIceOn) {
		if (active) {
			deactiveCenter.SetActive(false);
			activeCenter.SetActive(true);
		} else {
			deactiveCenter.SetActive(true);
			activeCenter.SetActive(false);
		}

		if (turnIceOn) {
			iceNode.SetActive(true);
		} else {
			iceNode.SetActive(false);
		}
	}

	void OnTriggerEnter(Collider other) {
		if (active) return;

		if (other.gameObject.CompareTag("Player")) {
			Eng_UI->CyberSprint(Eng_Text->stringTable[textIndex]);
			active = true;
			deactiveCenter.SetActive(false);
			activeCenter.SetActive(true);

			UseData ud = new UseData();
			ud.owner = other.gameObject;
			UseTargets(gameObject,ud,target);
		}
	}
}
