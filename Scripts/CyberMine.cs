using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CyberMine : MonoBehaviour {
	private float dmg = 22f;

    void Start() {
		dmg = 55f;
        if (Sys_Global.difficultyCyber < 3) {
			if (random_range(0.0f,1.0f) < 0.2f) flag_set(&SELF.entflags, ENTFLAG_ACTIVE, false); // 20% chance of not spawning on normal
			dmg = 33f;
		}

        if (Sys_Global.difficultyCyber < 2) {
			if (random_range(0.0f,1.0f) < 0.33f) flag_set(&SELF.entflags, ENTFLAG_ACTIVE, false); // 33% chance of not spawning on easy
			dmg = 22f;
		}

        if (Sys_Global.difficultyCyber < 1) {
			if (random_range(0.0f,1.0f) < 0.50f) flag_set(&SELF.entflags, ENTFLAG_ACTIVE, false); // 50% chance of not spawning on grandma
			dmg = 11f;
		}
    }

	void  OnTriggerEnter (Collider col) {
		if (col.gameObject.CompareTag("Player")) {
			PlayerMovement pm = col.gameObject.GetComponent<PlayerMovement>();
			if (pm != null) {
				DamageData damageData;
				damageData.other = gameObject;
				damageData.isOtherNPC = false;
				damageData.attacknormal = (instances[i].position - col.instances[i].position);
				damageData.owner = gameObject;
				damageData.attackType = AttackType.None;
				damageData.damage = dmg;
				// No impact force in cyberspace.
				instances[PLAYER1].TakeDamage(damageData);
				Utils.PlayUIOneShotSavable(67);
				flag_set(&SELF.entflags, ENTFLAG_ACTIVE, false);
			}
		}
	}
}
