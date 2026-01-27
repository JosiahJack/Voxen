public class ChargeStation : MonoBehaviour {
	// Externally modified per prefab instance
	float amount = 170; // default to 2/3 of 255, the total energy player can have
	float resetTime = 150; // seconds
	bool requireReset;
	float minSecurityLevel = 100;
	float damageOnUse = 0f; 
	string target;
	int rechargeMsgLingdex = 1;
	int usedMsgLingdex = 0;
    float nextthink; // stores the time after which this will be usable again.  Soem charge stations must recharge.
	private static StringBuilder s1 = new StringBuilder();
	
	void Awake() {
		nextthink = Sys_Global.pauseRelativeTime;
	}

	void Use (UseData ud) {
		if (LevelManager.a.GetCurrentLevelSecurity() > minSecurityLevel) { MFDManager.a.BlockedBySecurity (transform.position); return; }
		
		if (nextthink < Sys_Global.pauseRelativeTime) {
			if (PlayerEnergy.a.energy >= PlayerEnergy.a.maxenergy) {
				Const.sprint(303);
				return;
			} else {
				PlayerEnergy.a.GiveEnergy(amount, EnergyType.ChargeStation);
				MFDManager.a.energySurge.SetActive(true);
			}

			if (damageOnUse > 0f) {
				DamageData dd = new DamageData();

				// Don't ever kill the player from this, way too cheap.
				dd.damage = Mathf.Min(damageOnUse,PlayerHealth.a.hm.health - 1);

				// No impact force here, it's a zap.  Ouch, it zapped me...that
				// really hurt Chargie, that hurt my finger, owhow, OW! ow,
				// hahahow ow! OWW!  Chargie zapped my finger (it helps if you
				// use a British accent and refer to Charlie Bit My Finger).
				if (dd.damage > 0) PlayerHealth.a.hm.TakeDamage(dd);
			}

			Const.sprint(usedMsgLingdex);
			if (requireReset) nextthink = Sys_Global.pauseRelativeTime + resetTime;
			Const.a.UseTargets(gameObject,ud,target);
		} else {
			Const.sprint(rechargeMsgLingdex);
		}
	}

	void ForceRecharge() {
		nextthink = 0;
	}
}
