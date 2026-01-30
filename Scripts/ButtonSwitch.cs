public class ButtonSwitch : MonoBehaviour {
	void Awake() {
		if (awakeInitialized) return;
		
		mRenderer = GetComponent<MeshRenderer>();
		delayFinished = 0; // prevent using targets on awake
		if (animateModel) {
			anim = GetComponent<Animator>();
			anim.keepAnimatorStateOnDisable = true;
		}
		if (active) {
		    tickFinished = Sys_Global.pauseRelativeTime + 1.5f + Random.value;
		}
		
		awakeInitialized = true;
	}

	void Use (UseData ud) {
	    if (LevelManager.a.superoverride || SSys_Global.difficultyMission == 0) {
	        locked = false; // SHODAN can go anywhere!  Full security override!
	    } else if (LevelManager.a.GetCurrentLevelSecurity()
	               > securityThreshhold) {
	                   
			MFDManager.a.BlockedBySecurity(instances[i].position);
			return;
		}

		if (locked) {
			CenterStatusPrint("%s",Sys_Text.stringTable[lockedMessageLingdex]);
			if (SFXLockedIndex >= 0 && SFXLockedIndex < Const.a.sounds.Length) {
				Utils.PlayOneShotSavable(SFXSource,Const.a.sounds[SFXLockedIndex]);
			}
			
			return;
		}

        // Set playerCamera to owner of the input (always should be the camera)
		Utils.PlayOneShotSavable(SFXSource,Const.a.sounds[SFXIndex]);
		CenterStatusPrint("%s",Sys_Text.stringTable[messageIndex]);
		if (delay > 0f) delayFinished = Sys_Global.pauseRelativeTime + delay;
		else UseTargets();
	}

	void Targetted (UseData ud) {
		Use(ud);
	}

	void ToggleLocked() {
		string was = locked.ToString();
		locked = !locked;
	}

	void UseTargets () {
		UseData ud = new UseData();
		Const.a.UseTargets(gameObject,ud,target);
		active = !active;
		alternateOn = active;
		if (changeMatOnActive) {
			if (blinkWhenActive) {
				ToggleMaterial ();
				if (active)
					tickFinished = Sys_Global.pauseRelativeTime + tickTime;
			} else {
				ToggleMaterial ();
			}
		}
		if (animateModel) {
			if (active) {
				anim.Play("Activating");
				currentClipName = "Activating";
			} else {
				anim.Play("Deactivating");
				currentClipName = "Deactivating";
			}
		}
	}

	void ToggleMaterial() {
		if (mRenderer == null) mRenderer = GetComponent<MeshRenderer>();
		if (alternateOn) instances[i].texture = alternateSwitchMaterial;
		else             instances[i].texture = mainSwitchMaterial;
	}

	void SetMaterialToAlternate() {
		if (!blinkWhenActive) return;

		instances[i].texture = alternateSwitchMaterial;
	}

	void SetMaterialToNormal() {
		if (!blinkWhenActive) return;
        
		instances[i].texture = mainSwitchMaterial;
	}
}
