using UnityEngine;
using System.Collections;

public class HealingBed : MonoBehaviour {
	public float amount = 170;  //default to 2/3 of 255, the total player can have
	public int minSecurityLevel = 0;
	public bool broken = false;
	private float give;

	private AudioSource SFXSource;

	void Awake () {
		SFXSource = GetComponent<AudioSource>();
	}

	public void Use (UseData ud) {
		if (GetCurrentLevelSecurity() <= minSecurityLevel) {
			if (!broken) {
				PlayerReferenceManager.a.playerCapsule.GetComponent<HealthManager>().HealingBed(amount,true);
				PlayerHealth.a.radiationArea = false;
				PlayerHealth.a.radiated = 0f;
				CenterStatusPrint("%s", Sys_Text.stringTable[23],ud.owner);
				Utils.PlayOneShotSavable(SFXSource,sounds[103]);
			} else {
				CenterStatusPrint("%s", Sys_Text.stringTable[24],ud.owner);
			}
		} else {
			Sys_UI.BlockedBySecurity(instances[i].position);
		}
	}
}
