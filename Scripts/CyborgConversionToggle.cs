using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CyborgConversionToggle : MonoBehaviour {
	private AudioSource SFX;

    void Awake() {
		SFX = GetComponent<AudioSource>();
	}

	public void PlayVoxMessage() {
		SFX.Stop();
		int lindex = LevelManager.a.currentLevel != -1 ? LevelManager.a.currentLevel : 0;
		if (LevelManager.a.ressurectionActive[lindex]) {
			Utils.PlayOneShotSavable(SFX,sounds[183]); // "vox_cybconvcancelled"
			CenterStatusPrint("%s", Sys_Text.stringTable[591]); // "Cyborg conversion cancelled.  Healing normal."
		} else {
			Utils.PlayOneShotSavable(SFX,sounds[184]); // "vox_cybconvenabled"
			CenterStatusPrint("%s", Sys_Text.stringTable[592]); // "Cyborg conversion reactivated."
		}
	}
}
