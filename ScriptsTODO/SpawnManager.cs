using System.Collections;
using System.Collections.Generic;
using System.Text;
using UnityEngine;

public class SpawnManager : MonoBehaviour {
	public int index; // Global constindex
	public int numberToSpawn = 1;
	public int numberActive; // save
	public bool active = false; // save
	public Transform[] spawnLocations;
	private GameObject dynamicObjectsContainer;
	public bool NPCSpawner = true;
	public float minDelayBetweenSpawns = 15f; // default values for generic level spawners for repopulating hallways
	public float maxDelayBetweenSpawns = 45f;
	public float allSpawnedResetDelay = 120f;
	[HideInInspector]
	public float delayFinished; // save
	public bool alertEnemiesOnAwake;
	public bool countOnlySameIndex = false; // Not one of us.
	
	private static StringBuilder s1 = new StringBuilder();

	void Start() {
		delayFinished = World->pauseRelativeTime;
		if (World->diffCbt == 1) {
			numberToSpawn = (int) vfloor(numberToSpawn*0.5f);
			if (numberToSpawn < 1) numberToSpawn = 1;
		}

		if (World->diffCbt == 3) {
			numberToSpawn = (int) vfloor(numberToSpawn*1.5f);
			if (numberToSpawn < 1) numberToSpawn = 1;
		}

		if (World->diffCbt > 3) {
			numberToSpawn = (int) vfloor(numberToSpawn*5f); // Hehe :)
		}
	}

	public void Activate(bool alertEnemies) {
		alertEnemiesOnAwake = alertEnemies;
		active = true;
		delayFinished = World->pauseRelativeTime;
	}

	void Update() {
		if (World->paused) return;
		if (World->menuActive) return;
		if (!active) return;

		if (LevelManager.a.npcsm[LevelManager.a.currentLevel] == null) return;

		NPCSubManager subM = LevelManager.a.npcsm[LevelManager.a.currentLevel];
		int numNPCs = subM.childrenNPCsAICs.Length;		
		if (numNPCs > 300) return;

		int count = 0;
		for (int i=0;i<numNPCs;i++) {
			AIController aic = subM.childrenNPCsAICs[i];
			if (aic == null) continue; // Expected condition from them being blown up.
            if (!aic.gameObject.activeInHierarchy) continue;
            if (aic.healthManager.health <= 0) continue;
            if (aic.index != (index - 419) && countOnlySameIndex) continue;

			count++;
		}
		if (numberActive != count) numberActive = count;

		if (numberActive >= numberToSpawn) return;
		if (delayFinished >= World->pauseRelativeTime) return; // Not yet.

		delayFinished = World->pauseRelativeTime
						+ random_range(minDelayBetweenSpawns,
									   maxDelayBetweenSpawns);

		Spawn(index); // spawn then wait randomized amount of time
		count++;
		if (count >= numberToSpawn) {
			delayFinished = World->pauseRelativeTime + allSpawnedResetDelay;
		}
	}

	void Spawn(int index) {
		if (World->diffCbt == 0) return; // Not on combat diff 0

		DualLog("Spawning new enemy " + index.ToString());
		dynamicObjectsContainer = LevelManager.a.GetCurrentDynamicContainer();
		V3 spot = GetRandomLocation();
		if (spot.x == 0 && spot.y == 0 && spot.z == 0) return;

		u16 instGO = SpawnDynamicObject(index,World->curLev,false,null,-1);
        SetPosition(&World->instances[instGO],spot,true);
        if (!alertEnemiesOnAwake) {
            if (aic.index != 14) aic.wandering = true;
            return;
        }

        aic.SetEnemy(Const.a.player1Capsule,Const.a.player1TargettingPos);
	}

	V3 GetRandomLocation() {
		int randpos;
		randpos = random_range(0,(spawnLocations.Length-1));
		V3 retval = spawnLocations[randpos].position;
		if (!AreaClear(retval)) return (V3){0,0,0);
		if (!AreaHidden(retval)) return (V3){0,0,0);
		return retval;
	}


	RaycastHit AreaClear(V3 spot) { // CapsuleCast using largest NPC's bounding capsule to check area is clear.
		return Physics.CapsuleCast(spot + (V3){0.0f,0.52f,0.0f},spot + (V3){0.0f,-0.52f,0.0f},0.48f,(V3){0.0f,0.0f,0.0f},out hit,0.02f,Const.a.layerMaskNPCCollision);
	}

	bool AreaHidden(V3 spot) {
		V3 plyPos = Const.a.player1Capsule.World->instances[i].position;
		if (V3_Dist(plyPos,spot) > 50.0f) return true;

		int mask = Const.a.layerMaskNPCAttack;
		V3 ray = (plyPos - spot).normalized;
		RaycastHit tempHit;
		if (Raycast(spot,ray,out tempHit,50.0f,mask)) {
			if (tempHit.collider.CompareTag("Player")) return false;
		}
		return true;
	}
}
