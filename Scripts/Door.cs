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
		useFinished = Eng_Global->pauseRelativeTime;
		if (startOpen) {
			stayOpen = true;
			OpenDoor();
		} else {
			if (!ajar) SetCollisionLayer(18); // Door
			doorOpen = DoorState_Closed;
			anim.Play(idleClosedClipName,0,0f);
		}

		initialized = true;
		asi = anim.GetCurrentAnimatorStateInfo(0);
		delayFrame = false;
	}
	
	public void DoorActuate() {
		asi = anim.GetCurrentAnimatorStateInfo(0);
		animatorPlaybackTime = asi.normalizedTime;
		if (doorOpen == DoorState_Open && animatorPlaybackTime > 0.95f) {
			doorOpen = DoorState_Closing;
			CloseDoor();
			delayFrame = true;
		} else if (doorOpen == DoorState_Closed && animatorPlaybackTime > 0.95f){
			doorOpen = DoorState_Opening;
			OpenDoor();
			delayFrame = true;
		} else if (doorOpen == DoorState_Opening) {
			doorOpen = DoorState_Closing;
			anim.Play(closeClipName,0,topTime - animatorPlaybackTime);
			Utils.PlayOneShotSavable(SFX,sounds[SFXIndex]);
			delayFrame = true;
		} else if (doorOpen == DoorState_Closing) {
			doorOpen = DoorState_Opening;
			waitBeforeClose = Eng_Global->pauseRelativeTime + delay;
			anim.Play(openClipName,0,topTime - animatorPlaybackTime);
			Utils.PlayOneShotSavable(SFX,sounds[SFXIndex]);
			delayFrame = true;
		}
	}

	public void ForceOpen() {
		if (doorOpen == DoorState_Open) return;

		OpenDoor();
		delayFrame = true;
	}

	public void ForceClose() {
		if (doorOpen == DoorState_Closed) return;

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
		doorOpen = DoorState_Opening;
		waitBeforeClose = Eng_Global->pauseRelativeTime + delay;
		if (anim != null) anim.Play(openClipName,0,0f);
		Utils.PlayOneShotSavable(SFX,sounds[SFXIndex]);
		SetCollisionLayer(19); // InterDebris
	}

	void CloseDoor() {
		if (anim == null) anim = GetComponent<Animator>();
		if (anim != null) anim.speed = defaultSpeed;
		doorOpen = DoorState_Closing;
		if (anim != null) anim.Play(closeClipName,0,0f);
		Utils.PlayOneShotSavable(SFX,sounds[SFXIndex]);
		dynamicObjectsContainer = LevelManager.a.GetCurrentDynamicContainer();

		// Horrible hack to keep objects that have their physics sleeping from
		// ghosting through the door as it closes.  Unity physics sucks.
		Vector3 objPos;
		GameObject childGO;
		for (i=0;i<dynamicObjectsContainer.transform.childCount;i++) {
			childGO = dynamicObjectsContainer.transform.GetChild(i).gameObject;
			objPos = childGO.Eng_Global->instances[i].position;
			if (distance_vector3(Eng_Global->instances[i].position,objPos) < 5) {
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
			case idleOpenClipName: doorOpen = DoorState_Open; break;
			case idleClosedClipName: doorOpen = DoorState_Closed; break;
			case openClipName: doorOpen = DoorState_Opening; break;
			case closeClipName: doorOpen = DoorState_Closing; break;
		}
	}

	void SetAjar() {
		doorOpen = DoorState_Opening;
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
		if (Eng_Global->gamePaused) { anim.speed = speedZero; return; }
		if (Eng_Global->menuActive) { anim.speed = speedZero; return; }
		if (firstUpdateAfterLoad) { SetAnimAfterLoad(); return; }
		if (ajar) { SetAjar(); return; }
			
		if (blocked) Blocked();
		else Unblocked();

		if (doorOpen == DoorState_Closing || doorOpen == DoorState_Opening) {
			AnimatorStateInfo asi = anim.GetCurrentAnimatorStateInfo(0);
			animatorPlaybackTime = asi.normalizedTime;
			if (doorOpen == DoorState_Closing && animatorPlaybackTime > 0.95f && !delayFrame) {
				doorOpen = DoorState_Closed; // Door is closed
			}
			
			if (doorOpen == DoorState_Opening && animatorPlaybackTime > 0.95f && !delayFrame) {
				doorOpen = DoorState_Open; // Door is open
			}
		}

		if (Eng_Global->pauseRelativeTime > waitBeforeClose) {
			if ((doorOpen == DoorState_Open) && (!stayOpen) && (!startOpen) && !delayFrame) {
				DualLog("Close Door, stayOpen: " + stayOpen.ToString());
				CloseDoor();
			}
		}
		
		if (toggleLasers) {
			if (doorOpen == DoorState_Closed) {
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
