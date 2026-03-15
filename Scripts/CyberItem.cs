using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CyberItem : MonoBehaviour {
	public SoftwareType type;
	public int version;
	
	private GameObject explosionEffect;

	void Start() {
		if (Eng_Global->difficultyMission == 0) {
			// Disable data objects when Mission difficulty is 0.
			if (type == SoftwareType.Data) this.flag_set(&Eng_Global->instances[i].entflags, ENTFLAG_ACTIVE, false);
		}
	}

	void OnTriggerEnter(Collider other) {
		if (other.gameObject.CompareTag("Player")) {
			PlayerMovement pm = other.gameObject.GetComponent<PlayerMovement>();
			if (pm == null) return;

			if (!Eng_Global->inventoryPlayer1.AddSoftwareItem(type,version)) return;

			explosionEffect = null;
			explosionEffect = Const.a.GetObjectFromPool(PoolType.CyberDissolve);
			if (explosionEffect != null) {
				explosionEffect.SetActive(true);

				// Put vaporization effect at raycast center.
				explosionEffect.Eng_Global->instances[i].position = Eng_Global->instances[i].position; 
			}

			// We've been picked up, quick hide like you were.
			this.flag_set(&Eng_Global->instances[i].entflags, ENTFLAG_ACTIVE, false);
		}
	}
}
