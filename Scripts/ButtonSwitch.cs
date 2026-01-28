public class ButtonSwitch : MonoBehaviour {
	bool locked = false; // save
	int lockedMessageLingdex = 193;
	bool active; // save
	Material mainSwitchMaterial;
	Material alternateSwitchMaterial;
	AudioSource SFXSource;
	MeshRenderer mRenderer;
	Animator anim;
	GameObject player; // Set on use, no need for initialization check.
	const float tickTime = 1.5f;
	bool awakeInitialized = false;
    float delayFinished;
	float tickFinished;
	bool alternateOn;
	string currentClipName;
	private static StringBuilder s1 = new StringBuilder();

	void Awake() {
		if (awakeInitialized) return;

		SFXSource = GetComponent<AudioSource>();
		if (SFXSource == null) {
		    DualLog("BUG: ButtonSwitch missing component for SFXSource");
		} else SFXSource.playOnAwake = false;
		
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
	                   
			MFDManager.a.BlockedBySecurity(transform.position);
			return;
		}

		if (locked) {
			Const.sprint(lockedMessageLingdex);
			if (SFXLockedIndex >= 0 && SFXLockedIndex < Const.a.sounds.Length) {
				Utils.PlayOneShotSavable(SFXSource,Const.a.sounds[SFXLockedIndex]);
			}
			
			return;
		}

        // Set playerCamera to owner of the input (always should be the camera)
		player = ud.owner;
		Utils.PlayOneShotSavable(SFXSource,Const.a.sounds[SFXIndex]);
		Const.sprint(messageIndex);
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
		ud.owner = player;
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
		if (alternateOn) mRenderer.material = alternateSwitchMaterial;
		else             mRenderer.material = mainSwitchMaterial;
	}

	void SetMaterialToAlternate() {
		if (!blinkWhenActive) return;

		if (mRenderer == null) mRenderer = GetComponent<MeshRenderer>();
		if (mRenderer.material != alternateSwitchMaterial) {
		    mRenderer.material = alternateSwitchMaterial;
		}
	}

	void SetMaterialToNormal() {
		if (!blinkWhenActive) return;

		if (mRenderer == null) mRenderer = GetComponent<MeshRenderer>();
		if (mRenderer.material != mainSwitchMaterial) {
		    mRenderer.material = mainSwitchMaterial;
		}
	}

	Update() {
		if (Sys_Global.gamePaused || Sys_Global.menuActive) return;

		if ((delayFinished < Sys_Global.pauseRelativeTime) && delayFinished != 0) {
			delayFinished = 0;
			UseTargets();
		}

		if (blinkWhenActive) {
			if (active) {
				if (tickFinished < Sys_Global.pauseRelativeTime) {
					if (mRenderer.isVisible) {
						if (alternateOn) SetMaterialToAlternate();
						else SetMaterialToNormal();
					}
					alternateOn = !alternateOn;
					tickFinished = Sys_Global.pauseRelativeTime + tickTime;
				}
			}
		}
	}
}
