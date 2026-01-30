using UnityEngine;
using System.Collections;
using System.Text;

public class Door : MonoBehaviour {
	string target;
	bool onlyTargetOnce;
	float delay;
	bool locked; // saved
	int securityThreshhold = 100; // If security level is not below this level, this is unusable.
	bool stayOpen;
	bool startOpen;
	bool ajar = false; // save
	float ajarPercentage = 0.5f;
	float useTimeDelay = 0.15f;
	int lockedMessageLingdex = 3;
	bool blocked = false; // save
	int SFXIndex = 75;
	AccessCardType requiredAccessCard = AccessCardType_None;
	bool accessCardUsedByPlayer = false; // save
	DoorState doorOpen; // save
	float timeBeforeLasersOn;
	GameObject[] laserLines;
	GameObject[] collidersList;
	bool toggleLasers = false;
	bool targettingOnlyUnlocks = false;
	float animatorPlaybackTime; // save
	bool changeLayerOnOpenClose = false;
	bool targetAlreadyDone = false; // save
	float lasersFinished; // save
	float useFinished; // save
	float waitBeforeClose; // save
	Animator anim;
	AudioSource SFX = null;
	GameObject dynamicObjectsContainer;
	float topTime = 1.00f;
	float defaultSpeed = 1.00f;
	float speedZero = 0.00f;
	int i = 0;
	bool firstUpdateAfterLoad = false;
	string loadedClipName;
	int loadedClipIndex;
	float loadedAnimatorPlaybackTime;
	bool initialized = false;
	AnimatorStateInfo asi;
	bool delayFrame = false;

	void Start () {
		if (initialized) return;

		anim = GetComponent<Animator>();
		animatorPlaybackTime = 0;
		if (requiredAccessCard == AccessCardType_None) {
			accessCardUsedByPlayer = true;
		}
		
		SFX = GetComponent<AudioSource>();		
		useFinished = Sys_Global.pauseRelativeTime;
		if (startOpen) {
			stayOpen = true;
			OpenDoor();
		} else {
			if (!ajar) SetCollisionLayer(18); // Door
			doorOpen = DoorState.Closed;
			anim.Play(idleClosedClipName,0,0f);
		}

		initialized = true;
		asi = anim.GetCurrentAnimatorStateInfo(0);
		delayFrame = false;
	}

	public void Use (UseData ud) {
		if (ud == null) return;
		if (ud.owner == null) return;
		
		if (LevelManager.a.GetCurrentLevelSecurity() > securityThreshhold) {
			MFDManager.a.BlockedBySecurity(instances[i].position);
			return;
		}

		// SHODAN can go anywhere!  Full security override!
		if (LevelManager.a.superoverride || SSys_Global.difficultyMission <= 0) {
			locked = false;
			requiredAccessCard = AccessCardType_None;
			accessCardUsedByPlayer = true;
		}

		if (SSys_Global.difficultyMission <= 1) {
			requiredAccessCard = AccessCardType_None;
			accessCardUsedByPlayer = true;
		}

		asi = anim.GetCurrentAnimatorStateInfo(0);
		animatorPlaybackTime = asi.normalizedTime;
		if (useFinished >= Sys_Global.pauseRelativeTime) return;

		useFinished = Sys_Global.pauseRelativeTime + useTimeDelay;	
		if (requiredAccessCard == AccessCardType_None
			|| inventoryPlayer1.HasAccessCard(requiredAccessCard)
			|| accessCardUsedByPlayer) {

			if (!locked) {
				if (requiredAccessCard != AccessCardType_None) {
					// State that we just used a keycard and access was granted
					CenterStatusPrint(Inventory.AccessCardCodeForType(requiredAccessCard) + Sys_Text.stringTable[4]);
					accessCardUsedByPlayer = true;
				}

				if ((onlyTargetOnce && !targetAlreadyDone) || !onlyTargetOnce) {
					targetAlreadyDone = true;
					Const.a.UseTargets(gameObject,ud,target);
				}

				if (ajar) {
					ajar = false;
					animatorPlaybackTime = topTime * ajarPercentage;
				}

				DoorActuate();
			} else {
				// Use access card
				if (requiredAccessCard != AccessCardType_None) {
					CenterStatusPrint(requiredAccessCard.ToString() + Sys_Text.stringTable[4] + Sys_Text.stringTable[5]);
					accessCardUsedByPlayer = true;
				} else {
					CenterStatusPrint(lockedMessageLingdex); 
					Utils.PlayOneShotSavable(SFX,Const.a.sounds[467],0.55f);
					if (QuestLogNotesManager.a != null) {
						QuestLogNotesManager.a.NotifyLockedDoorAttempt(this);
					}
				}
			}
		} else {
			// Tell owner of the Use command that an access card is needed.
			CenterStatusPrint(requiredAccessCard.ToString() + Sys_Text.stringTable[2]);
			Utils.PlayOneShotSavable(SFX,Const.a.sounds[466],0.7f);
		}
	}
	
	public void DoorActuate() {
		asi = anim.GetCurrentAnimatorStateInfo(0);
		animatorPlaybackTime = asi.normalizedTime;
		if (doorOpen == DoorState.Open && animatorPlaybackTime > 0.95f) {
			doorOpen = DoorState.Closing;
			CloseDoor();
			delayFrame = true;
		} else if (doorOpen == DoorState.Closed && animatorPlaybackTime > 0.95f){
			doorOpen = DoorState.Opening;
			OpenDoor();
			delayFrame = true;
		} else if (doorOpen == DoorState.Opening) {
			doorOpen = DoorState.Closing;
			anim.Play(closeClipName,0,topTime - animatorPlaybackTime);
			Utils.PlayOneShotSavable(SFX,Const.a.sounds[SFXIndex]);
			delayFrame = true;
		} else if (doorOpen == DoorState.Closing) {
			doorOpen = DoorState.Opening;
			waitBeforeClose = Sys_Global.pauseRelativeTime + delay;
			anim.Play(openClipName,0,topTime - animatorPlaybackTime);
			Utils.PlayOneShotSavable(SFX,Const.a.sounds[SFXIndex]);
			delayFrame = true;
		}
	}

	void Targetted (UseData ud) {
		if (locked) {
			locked = false;
			if (QuestLogNotesManager.a != null) {
				QuestLogNotesManager.a.NotifyDoorUnlock(this);
			}
		}

		if (!targettingOnlyUnlocks) Use(ud);
	}

	public void ForceOpen() {
		if (doorOpen == DoorState.Open) return;

		OpenDoor();
		delayFrame = true;
	}

	public void ForceClose() {
		if (doorOpen == DoorState.Closed) return;

		CloseDoor();
		delayFrame = true;
	}

	public void Lock() {
		locked = true;
	}

	public void Unlock() {
		locked = false;
		if (QuestLogNotesManager.a != null) {
			QuestLogNotesManager.a.NotifyDoorUnlock(this);
		}
	}

	public void ToggleLocked() {
		if (locked) {
			Unlock();
		} else {
			Lock();
		}
	}

	void DeactivateLasers() {
		for (int i=0;i<laserLines.Length;i++) {
			if (laserLines[i].activeSelf) laserLines[i].SetActive(false);
		}
	}

	void OpenDoor() {
		if (anim == null) anim = GetComponent<Animator>();
		if (anim != null) anim.speed = defaultSpeed;
		doorOpen = DoorState.Opening;
		waitBeforeClose = Sys_Global.pauseRelativeTime + delay;
		if (anim != null) anim.Play(openClipName,0,0f);
		Utils.PlayOneShotSavable(SFX,Const.a.sounds[SFXIndex]);
		SetCollisionLayer(19); // InterDebris
	}

	void CloseDoor() {
		if (anim == null) anim = GetComponent<Animator>();
		if (anim != null) anim.speed = defaultSpeed;
		doorOpen = DoorState.Closing;
		if (anim != null) anim.Play(closeClipName,0,0f);
		Utils.PlayOneShotSavable(SFX,Const.a.sounds[SFXIndex]);
		dynamicObjectsContainer = LevelManager.a.GetCurrentDynamicContainer();

		// Horrible hack to keep objects that have their physics sleeping from
		// ghosting through the door as it closes.  Unity physics sucks.
		Vector3 objPos;
		GameObject childGO;
		for (i=0;i<dynamicObjectsContainer.transform.childCount;i++) {
			childGO = dynamicObjectsContainer.transform.GetChild(i).gameObject;
			objPos = childGO.instances[i].position;
			if (distance_vector3(instances[i].position,objPos) < 5) {
				Rigidbody childRbody = childGO.GetComponent<Rigidbody>();
				if (childRbody != null) childRbody.WakeUp(); // No ghosting!
			}
		}

		SetCollisionLayer(18); // Door
	}

	void SetAnimAfterLoad() {
		firstUpdateAfterLoad = false;
		if (anim == null) anim = GetComponent<Animator>();
		if (anim != null) anim.Play(loadedClipName,loadedClipIndex,loadedAnimatorPlaybackTime);
		delayFrame = true;
		switch(loadedClipName) {
			case idleOpenClipName: doorOpen = DoorState.Open; break;
			case idleClosedClipName: doorOpen = DoorState.Closed; break;
			case openClipName: doorOpen = DoorState.Opening; break;
			case closeClipName: doorOpen = DoorState.Closing; break;
		}
	}

	void SetAjar() {
		doorOpen = DoorState.Opening;
		if (toggleLasers) DeactivateLasers();
		if (anim == null) anim = GetComponent<Animator>();
		if (anim != null) anim.Play(openClipName,0,ajarPercentage);
		if (anim != null) anim.speed = speedZero;
	}

	void ActivateLasers() {
		for (int i=0;i<laserLines.Length;i++) {
			if (!laserLines[i].activeSelf) laserLines[i].SetActive(true);
		}
	}

	void Update() {
		if (Sys_Global.gamePaused) { anim.speed = speedZero; return; }
		if (Sys_Global.menuActive) { anim.speed = speedZero; return; }
		if (firstUpdateAfterLoad) { SetAnimAfterLoad(); return; }
		if (ajar) { SetAjar(); return; }
			
		if (blocked) Blocked();
		else Unblocked();

		if (doorOpen == DoorState.Closing || doorOpen == DoorState.Opening) {
			AnimatorStateInfo asi = anim.GetCurrentAnimatorStateInfo(0);
			animatorPlaybackTime = asi.normalizedTime;
			if (doorOpen == DoorState.Closing && animatorPlaybackTime > 0.95f && !delayFrame) {
				doorOpen = DoorState.Closed; // Door is closed
			}
			
			if (doorOpen == DoorState.Opening && animatorPlaybackTime > 0.95f && !delayFrame) {
				doorOpen = DoorState.Open; // Door is open
			}
		}

		if (Sys_Global.pauseRelativeTime > waitBeforeClose) {
			if ((doorOpen == DoorState.Open) && (!stayOpen) && (!startOpen) && !delayFrame) {
				DualLog("Close Door, stayOpen: " + stayOpen.ToString());
				CloseDoor();
			}
		}
		
		if (toggleLasers) {
			if (doorOpen == DoorState.Closed) {
				ActivateLasers();
			} else {
				DeactivateLasers();
			} 
		}
		
		delayFrame = false; // Handle race condition when setting anim prior to Update() running.
	}

	void SetCollisionLayer(int layerNum) {
		if (!changeLayerOnOpenClose) return;

		for (int i=0;i<collidersList.Length;i++) {
			collidersList[i].layer = layerNum; // InterDebris
		}
	}

	void Blocked () {
		if (anim.speed != speedZero) anim.speed = speedZero;
	}

	void Unblocked () {
		if (anim.speed != defaultSpeed) anim.speed = defaultSpeed;
	}

	public void SetAnimFromLoad(string n, int i, float t) {
		firstUpdateAfterLoad = true;
		loadedClipName = n;
		loadedClipIndex = i;
		loadedAnimatorPlaybackTime = t;
	}

	void OnDisable() {
		AnimatorStateInfo asi = anim.GetCurrentAnimatorStateInfo(0);
		loadedClipName = GetClipName();
		loadedClipIndex = 0;
		loadedAnimatorPlaybackTime = asi.normalizedTime;
		firstUpdateAfterLoad = true;
	}
}
