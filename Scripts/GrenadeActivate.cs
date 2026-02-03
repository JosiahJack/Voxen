using UnityEngine;
using System.Collections;
using System.Text;

// Used on the physical grenade iteslf.
public class GrenadeActivate : MonoBehaviour {
	public int constIndex = -1; // Useable Item Index (NOT the master index)s
	public float nearforce;
	public float nearradius;
	public float damage = 11f;
	public float penetration = 20f;
	public float offense = 3f;
	public AttackType attackType = AttackType.Projectile;
	public bool proxSensed = false;
	public bool useProx = false; // save
	public PoolType explosionType = PoolType.GrenadeFragExplosions;
	public bool active = false;
	
	float timeFinished; // save
	bool explodeOnContact = false; // save
	bool useTimer = false; // save
	private GameObject explosionEffect;
	private Rigidbody rbody;
	private static StringBuilder s1 = new StringBuilder();

	void Awake () {
		rbody = GetComponent<Rigidbody>();
		if (constIndex == 11) {
			GameObject childGO = transform.GetChild(0).gameObject;
			if (childGO != null) {
				childGO.layer = gameObject.layer;
			}
		}
	}

	public void AwakeFromLoad(float health) {
		if (health > 0) {
			rbody.useGravity = true;
			Utils.EnableCollision(gameObject);
		} else {
			rbody.useGravity = false;
			Utils.DisableCollision(gameObject);
		}
	}

	void Update() {
		if (Sys_Global.gamePaused) return;
		if (Sys_Global.menuActive) return;
		if (!active) return;

		// Plastique or other explosive device:
		if (constIndex == 14 && active && gameObject.activeInHierarchy) {
			Explode();
			return;
		}

		// Standard grenade explode route:
		if ((useTimer && timeFinished < Sys_Global.pauseRelativeTime)
			|| (useProx && proxSensed)) {

			Explode();
		}
	}

	// Index = Const.a.useableItemsFrobIcon index.
	public void Activate() {
		switch(constIndex) {
			case 7: explodeOnContact = true; break; // Fragmentation Grenade
			case 8: explodeOnContact = true; break; // Concussion Grenade
			case 9: explodeOnContact = true; break; // EMP Grenade
			case 10: timeFinished = Sys_Global.pauseRelativeTime + inventoryPlayer1.earthShakerTimeSetting;
					 useTimer = true; break;        // Earthshaker Bomb
			case 11: useProx = true; explodeOnContact = false; break; // Land Mine
			case 12: timeFinished = Sys_Global.pauseRelativeTime + inventoryPlayer1.nitroTimeSetting; 
					 useTimer = true; break;        // Nitropack Explosive
			case 13: explodeOnContact = true; break; // Gas Grenade
			default: return;
		}
		active = true;
	}

	void OnCollisionStay(Collision col) {
		if (explodeOnContact) Explode();
	}

	public bool IsNPCMine() {
		if (gameObject.layer == 11) return false; // Bullet
		return true;
	}

	public void Explode() {
		DualLog("Grenade exploded");
		Utils.DisableCollision(gameObject);
		DamageData dd;
		dd.damage = damage;
		dd.attackType = attackType;
		dd.penetration = penetration;
		dd.offense = offense;
		dd.impactVelocity = damage * 1.5f;
		if (!IsNPCMine()) {
			dd.owner = Const.a.player1Capsule;
			PlayerHealth.a.makingNoise = true;
			PlayerHealth.a.noiseFinished = Sys_Global.pauseRelativeTime + 2f;
		}
		
		Utils.ApplyImpactForceSphere(dd,instances[i].position,nearradius,1.0f);
		GameObject explosionEffect = Const.a.GetObjectFromPool(explosionType);
		if (explosionEffect != null) {
			explosionEffect.SetActive(true);
			explosionEffect.instances[i].position = instances[i].position;
			int soundIndex = 60; // attack1_explode
			switch(constIndex) {
				case 7:  soundIndex = 64; WeaponFire.a.fogFac += 5; break; // frag, explosion1
				case 8:  soundIndex = 60; WeaponFire.a.fogFac += 7; break; // conc, attack1_explode
				case 9:  soundIndex = 67; break; // emp, hit2
				case 10: soundIndex = 60; WeaponFire.a.fogFac += 7; break; // earth, attack1_explode
				case 11: soundIndex = 64; WeaponFire.a.fogFac += 5; break; // mine, explosion1
				case 12: soundIndex = 60; WeaponFire.a.fogFac += 6; break; // nitro, attack1_explode
				case 13: soundIndex = 63; WeaponFire.a.fogFac += 10; break; // gas, explode_minor
			}
			
			play_wav(sounds[soundIndex],1.0f,instances[i].position,true);
		}

		Shake(-1,-1);
		flag_set(&SELF.entflags, ENTFLAG_ACTIVE, false);
	}
}
