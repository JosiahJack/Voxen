
	int[] levelSecurity;
	int[] levelCameraCount;
	int[] levelLargeNodeCount;
	int[] levelSmallNodeCount;
	int[] levelCameraDestroyedCount;
	int[] levelSmallNodeDestroyedCount;
	int[] levelLargeNodeDestroyedCount;
	Vector3[] ressurectionLocation;
	bool[] ressurectionActive;
	uint16_t[] ressurectionBayDoor;
	Vector3[] elevatorTargetDestinations;

	void CyborgConversionToggleForCurrentLevel() {	    
		if (Eng_Global->currentLevel == 6) {
			if (ressurectionActive[Eng_Global->currentLevel]) {
				ressurectionActive[Eng_Global->currentLevel] = false;
				ressurectionActive[10] = false;
				ressurectionActive[11] = false;
				ressurectionActive[12] = false;
			} else {
				ressurectionActive[Eng_Global->currentLevel] = true;
				ressurectionActive[10] = true;
				ressurectionActive[11] = true;
				ressurectionActive[12] = true;
			}
		} else {
			ressurectionActive[Eng_Global->currentLevel] = !ressurectionActive[Eng_Global->currentLevel]; // Toggle current level.
		}
	}

	bool RessurectPlayer() {
		if (!ressurectionActive[Eng_Global->currentLevel]) return false;

		if (Eng_Global->currentLevel == 10 || Eng_Global->currentLevel == 11 || Eng_Global->currentLevel == 12) {
			LoadLevel(6,ressurectionLocation[currentLevel].position);
			ressurectionBayDoor[6].ForceClose();
		} else {
			if (Eng_Global->currentLevel >= 0 || Eng_Global->currentLevel < 13) Eng_Global->instances[PLAYER1].position = ressurectionLocation[Eng_Global->currentLevel];
		}

		// Activate death screen and readouts for "BRAIN ACTIVITY SATISFACTORY..." ya debatable right etc. etc.
// 		PlayerReferenceManager.a.playerDeathRessurectEffect.SetActive(true); // TODO
		PlayTrack(TrackType_Revive,MusicType_Override);
		Eng_Global->instances[PLAYER1].ressurectingFinished = Eng_Global->pauseRelativeTime + 3f;
		return true;
	}
	
	// Typical level
	// 4 CPU nodes
	// 20 cameras
	// 100% = 4x + 20y
	// Assuming that a good camera percentage is 2-3%, CPU % would be about 10-15 each
	void ReduceCurrentLevelSecurity(SecurityType stype) {
		float camScore = 4;
		float nodeSmallScore = 10;
		float nodeLargeScore = 27;
		float secscoreTotal = (levelCameraCount[currentLevel] * camScore) + (levelSmallNodeCount[currentLevel] * nodeSmallScore) + (levelLargeNodeCount[currentLevel] * nodeLargeScore);
		float secDrop = camScore; // default to camScore
		switch (stype) {
			case SecurityType_None: return;
			case SecurityType_Camera: secDrop = ((camScore/secscoreTotal) * 100); levelCameraDestroyedCount[currentLevel]++; break; // 1 camera divided by the total, so 2/ say (40+60) = 2/100 = 0.02, or 2% using the example numbers above
			case SecurityType_NodeSmall: secDrop = ((nodeSmallScore/secscoreTotal) * 100); levelSmallNodeDestroyedCount[currentLevel]++; break;
			case SecurityType_NodeLarge: secDrop = ((nodeLargeScore/secscoreTotal) * 100); levelLargeNodeDestroyedCount[currentLevel]++; break;
		}
		levelSecurity[currentLevel] -= (int)secDrop;
		if (levelSecurity [currentLevel] < 0) levelSecurity [currentLevel] = 0;
		if ((levelLargeNodeDestroyedCount[currentLevel] == levelLargeNodeCount[currentLevel]) && (levelSmallNodeDestroyedCount[currentLevel] == levelSmallNodeCount[currentLevel]) && (levelCameraDestroyedCount[currentLevel] == levelCameraCount[currentLevel])) {
			levelSecurity[currentLevel] = 0;
		}
		CenterStatusPrint("%s", Eng_Text->stringTable[306] + levelSecurity[currentLevel].ToString() + Eng_Text->stringTable[307]);

		// Notify quest log if all nodes were destroyed
		if (levelLargeNodeDestroyedCount[currentLevel] == levelLargeNodeCount[currentLevel]) {
			if (QuestLogNotesManager.a != null) QuestLogNotesManager.a.NodesDestroyed(currentLevel);
		}
	}
}
