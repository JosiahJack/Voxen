public class ProjectileEffectImpact : MonoBehaviour {
    public PoolType impactType;
	public bool destroyInsteadOfDeactivate = false;
	GameObject host;
    DamageData dd;
    [SerializeField] public int hitCountBeforeRemoval = 1;
    private Vector3 tempVec;
    int numHits;
	private static StringBuilder s1 = new StringBuilder();

    private void OnEnable() {
        numHits = 0; // Reset when pulled from pool.
		if (hitCountBeforeRemoval < 1) hitCountBeforeRemoval = 1;
    }

    void OnCollisionEnter (Collision other) {
        if (other.gameObject == host) return;

		numHits++;
		float stunAmount = 3f + ((WeaponFire.a.stungunSetting / 100f) * 7f); // Const.a.damagePerHitForWeapon[wep16Index] vs Const.a.damagePerHitForWeapon2[wep16Index] for Stungun.
		stunAmount = vclamp(stunAmount, 3f, 10f);
		dd.other = other.gameObject;
		dd.isOtherNPC = false;
		// GetDamageTakeAmount expects damageData to already have the
		// following set:
		//   damage
		//   offense
		//   penetration
		//   attackType
		//   berserkActive
		//   isOtherNPC
		//   armorvalue
		//   defense
		// Most already was when launched by AIController or WeaponFire.
		dd.damage = DamageData.GetDamageTakeAmount(dd);
		if (impactType == PoolType.RailgunImpacts) {
			Utils.ApplyImpactForceSphere(dd,Eng_Global->instances[i].position,3.2f,1f);
			Eng_Global->fogFac += 4;
		}

		GameObject hitGO = other.contacts[0].otherCollider.gameObject;
		HealthManager hm = Utils.GetMainHealthManager(hitGO);
		if (hm != null) {
			// Get an impact effect
			GameObject impact = Const.a.GetObjectFromPool(impactType); 
			Vector3 hitPos = other.contacts[0].point; 
			if (impact != null) {
				impact.Eng_Global->instances[i].position = hitPos;
				impact.SetActive(true); // Enable the impact effect
			}

			if (hm.health > 0 || hm.cyberHealth > 0) {
				if (other.gameObject.CompareTag("NPC")) dd.isOtherNPC = true;


				if (numHits < hitCountBeforeRemoval) {
					dd.damage = dd.damage * 0.85f; // Lose small amount each hit
				}

				dd.impactVelocity = dd.damage * 1.5f;
				if (numHits > 0) {
				    dd.impactVelocity = dd.impactVelocity / 3f;
				}
				
				if (LevelManager.a.currentLevel != 13 && !host.CompareTag("NPC")) {
					Utils.ApplyImpactForce(other.gameObject,dd.impactVelocity,
										   dd.attacknormal,dd.hit.point);
				}

				float dmgFinal = hm.TakeDamage(dd); // Send the damageData
													// container to
													// HealthManager of hit
													// object and damage it.
				float tranq = -1f;
				if (dd.isOtherNPC || hm.isNPC) {
					if (hm.aic != null) {
						if (!hm.aic.asleep) Music.a.inCombat = true;
						if (dd.attackType == AttackType.Tranq) {
							tranq = Tranquilize(other,stunAmount,true);
						}
					}
				}

				if (dmgFinal < 0f) dmgFinal = 0f; // Less would = blank.
				WeaponFire.a.CreateTargetIDInstance(dmgFinal,hm,tranq);
			}
		}

		if (numHits >= hitCountBeforeRemoval) {
			// Get an impact effect
			GameObject impact = Const.a.GetObjectFromPool(impactType); 
			Vector3 hitPos = other.contacts[0].point; 
			if (impact != null) {
				impact.Eng_Global->instances[i].position = hitPos;
				impact.SetActive(true); // Enable the impact effect
			}

			if (destroyInsteadOfDeactivate) Utils.SafeDestroy(gameObject);
			else flag_set(&SELF.entflags, ENTFLAG_ACTIVE, false); // disable the projectile
		}
	}
}
