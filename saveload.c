// saveload.c - Saving and Loading of entire game state
void WriteDatForIntroPlayed() {
    // TODO
}

void Save(uint8_t slot) {
    // TODO
}

void Load(uint8_t slot) {
    // TODO
}


// 	// Load the Game
// 	// ========================================================================
// 	// Sequence is as follows
// 	// 1. Player clicks on a button in load game menu or presses Quick Load.
// 	// 2. This function Load() is called with index -1 thru 7 and actual=false.
// 	// 3. Load then creates a DontDestroyOnLoad gameobject
// 	// 4. Current scene is unloaded.
// 	// 5. SceneTransitionHandler on DontDestroyOnLoad gameobject loads scene.
// 	// 6. Const Start() detects DontDestroyOnLoad object, uses Load actual=true
// 	// 7. Load then does actual load.
// 	//    a. Iterate over and destroy all dynamic objects in level containers.
// 	//    b. Find all remaining saveables to load static objects to.
// 	//    c. Load to static saveable objects.
// 	//    d. Iterate over dynamic object containers instantiating from save.
// 	public void Load(int saveFileIndex, bool actual) {
// 	    if (Application.platform == RuntimePlatform.Android) return;
// 		ShowLoading();
// 		GameObject freshGame = GameObject.Find("GameNotYetStarted");
// 		if (freshGame != null) Utils.SafeDestroy(freshGame);
// 		introNotPlayed = false;
// 		WriteDatForIntroPlayed(introNotPlayed); // reset
// 		StartCoroutine(Const.a.LoadRoutine(saveFileIndex,false));
// 	}
// 
// 	// LOAD 2. Called from Load menu or Quick Load.
// 	// LOAD 6. Called from Const.a.Start().
// 	public IEnumerator LoadRoutine(int saveFileIndex, bool actual) {
// 		Stopwatch loadTimer = new Stopwatch();
// 		Stopwatch loadUpdateTimer = new Stopwatch(); // For loading % indicator.
// 		loadTimer.Start();
// 		loading = true;
// 		UnityEngine.DualLog("Start of Load for index " + saveFileIndex.ToString());
// 		yield return null; // Update the view to show ShowLoading changes.
// 
// 		string readline; 					// Initialize temporary variables.
// 		int numSaveablesFromSavefile = 0;
// 		int i,j,k;
// 		GameObject currentGameObjectInScene = null;
// 		List<GameObject> saveableGameObjectsInScene = new List<GameObject>();
// 		loadPercentText.text = "Preparing...";
// 		yield return null; // Update progress text.
// 
// 		SaveObject.currentObjectInfo = "Start of Load...";
// 
// 		// Remove and clear out everything and reset any lists.
// 		ClearActiveAutomapOverlays();
// 		TargetRegister.Clear();
// 		TargetnameRegister.Clear();
// 		for (i=0;i<healthObjectsRegistration.Length;i++) {
// 			healthObjectsRegistration[i] = null;
// 		}
// 		
// 		LevelManager.a.ResetSaveStrings();
// 		for (i=0;i<14;i++) {
// 			LevelManager.a.UnloadLevelDynamicObjects(i,false); // Delete them all!
// 			LevelManager.a.UnloadLevelNPCs(i); // Delete them all!
// 			loadPercentText.text = "Preparing level " + i.ToString();
// 			yield return new WaitForSeconds(0.1f); // Update progress text.
// 		}
// 
// 		loadPercentText.text = "Open Save File         ";
// 		yield return null; // Update progress text.
// 
// 		List<string> readFileList = new List<string>();
// 		int index = 0; // Caching since it will be iterated over in a loop.
// 		string[] entries = new string[2048]; // Holds pipe | delimited strings
// 											 // on individual lines.
// 		string lName = "sav" + saveFileIndex.ToString() + ".txt";
// 		StreamReader sr = Utils.ReadStreamingAsset(lName);
// 		List<GameObject> allParents = SceneManager.GetActiveScene().GetRootGameObjects().ToList();
// 		if (sr != null) {
// 			// Read the file into a list, line by line
// 			using (sr) {
// 				do {
// 					readline = sr.ReadLine();
// 					if (readline != null) readFileList.Add(readline);
// 				} while (!sr.EndOfStream);
// 				sr.Close();
// 			}
// 
// 			loadPercentText.text = "Load Quest Data...     ";
// 			yield return null; // to update the sprint
// 			int numSaveFileLines = readFileList.Count;
// 			numSaveablesFromSavefile = numSaveFileLines - 3;
// 
// 			// readFileList[0] == saveName;  Not important, we are loading already now
// 			//index = 0; // Uncomment this if we pull in the saveName from this line for something.
// 
// 			// Read in global time, pause data, credit stats
// 			entries = readFileList[1].Split(Utils.splitCharChar);
// 			
// 			// The global time from which everything checks it's
// 			// somethingerotherFinished timer states.
// 			Sys_Global.pauseRelativeTime = Utils.GetFloatFromString(entries[index],"GameTime"); index++;
// 			PauseScript.a.absoluteTime = Utils.GetFloatFromString(entries[index],"TotalPlayTime"); index++;
// 			kills = Utils.GetIntFromString(entries[index],"kills"); index++;
// 			cyberkills = Utils.GetIntFromString(entries[index],"cyberkills"); index++;
// 			shotsFired = Utils.GetIntFromString(entries[index],"shotsFired"); index++;
// 			grenadesThrown = Utils.GetIntFromString(entries[index],"grenadesThrown"); index++;
// 			damageDealt = Utils.GetFloatFromString(entries[index],"damageDealt"); index++;
// 			damageReceived = Utils.GetFloatFromString(entries[index],"damageReceived"); index++;
// 			savesScummed = 1 + Utils.GetIntFromString(entries[index],"savesScummed"); // 1+, you're doin' it now!
// 			index = 0; // reset before starting next line
// 
// 			// Read in global states, difficulties, and quest mission bits.
// 			entries = readFileList[2].Split(Utils.splitCharChar);
// 			index = LevelManager.Load(LevelManager.a.gameObject,ref entries,index);
// 			index = questData.Load(ref entries,index);
// 			index = QuestLogNotesManager.a.Load(ref entries,index);
// 			difficultyCombat = Utils.GetIntFromString(entries[index],"difficultyCombat"); index++;
// 			difficultyMission = Utils.GetIntFromString(entries[index],"difficultyMission"); index++;
// 			difficultyPuzzle = Utils.GetIntFromString(entries[index],"difficultyPuzzle"); index++;
// 			difficultyCyber = Utils.GetIntFromString(entries[index],"difficultyCyber"); index++;
// 			loadPercentText.text = "Preprocess Save File...";
// 			yield return null;
// 
// 			// First pass to initialize tracking arrays:
// 			// - saveFile_Line_SaveID, This holds the full list of all unique IDs.
// 			// - saveableIsInstantiated, True if object is instantiated prefab.
// 			int[] saveFile_Line_SaveID = new int[numSaveFileLines];
// 			bool[] saveFile_Line_IsInstantiated = new bool[numSaveFileLines];
// 			bool[] alreadyLoadedLineFromSaveFile = new bool[numSaveFileLines];
// 			Utils.BlankBoolArray(ref alreadyLoadedLineFromSaveFile,false); // Fill with false.
// 			for (i = 3; i < numSaveFileLines; i++) {
// 				entries = readFileList[i].Split(Utils.splitCharChar);
// 				if (entries.Length < 1)  continue;
// 
// 				saveFile_Line_SaveID[i] = Utils.GetIntFromString(entries[2],"SaveID");
// 				saveFile_Line_IsInstantiated[i] = Utils.GetBoolFromString(entries[3],"instantiated");
// 			}
// 
// 			loadPercentText.text = "Preprocess Arrays...   ";
// 			yield return null;
// 			index = 3;
// 			SaveObject currentSaveObjectInScene;
// 
// 			// LOAD 7b. FIND ALL STATIC SAVEABLES
// 			// DO THIS AFTER BLANKING TO ENSURE WE HAVE UP-TO-DATE LIST!!
// 			// Find all gameobjects with SaveObject script attached.
// 			// This assumes every prefab and static GameObject has only one
// 			// SaveObject script attached at top parent for that object.
// 			// Exceptions:
// 			// - func_wall has its SaveObject on first child
// 			// - se_corpse_eaten has its SearchableItem on first child
// 			saveableGameObjectsInScene.Clear();
// 			FindAllSaveObjectsGOs(ref saveableGameObjectsInScene); // ref to avoid boxing.
// 			//UnityEngine.DualLog("Found " 
// 			//					  + saveableGameObjectsInScene.Count.ToString()
// 			//					  + " total static saveables remaining in "
// 			//					  + "scene after blanking out dynamic "
// 			//					  + "containers and NPC containers.");
// 
// 			bool[] alreadyCheckedThisSaveableGameObjectInScene = new bool[saveableGameObjectsInScene.Count];
// 			Utils.BlankBoolArray(ref alreadyCheckedThisSaveableGameObjectInScene,false); // Fill with false.
// 
// 			bool[] alreadyCheckedThisInstantiableGameObjectInScene = new bool[saveableGameObjectsInScene.Count];
// 			Utils.BlankBoolArray(ref alreadyCheckedThisInstantiableGameObjectInScene,false); // Fill with false.
// 
// 			// LOAD 7c. LOAD TO STATIC SAVEABLES
// 			// Ok, so we have a list of all saveableGameObjectsInScene and a list of
// 			//   all saveables from the savefile.
// 			// Main iteration loops through all lines in the savefile.
// 			// Second iteration loops through all saveableGameObjectsInScene to find a match.
// 			// The save file will always have more objects in it than in the
// 			//   level since we removed the instantiables.
// 			// When we come across an instantiated object in the saveable file,
// 			//   we need to skip it for later and instantiate them all.
// 			loadPercentText.text = "Loading Static Objects: 0.0% (    0 / "
// 								   + numSaveablesFromSavefile.ToString() + ")";
// 			yield return null;
// 			loadUpdateTimer.Start(); // For loading update
// 			float perc = 0f;
// 			for (i = 3; i < numSaveFileLines; i++) {
// 				if (saveFile_Line_IsInstantiated[i]) continue; // Skip instantiables.
// 
// 				alreadyLoadedLineFromSaveFile[i] = true;
// 				for (j=0;j<(saveableGameObjectsInScene.Count);j++) {
// 					if (alreadyCheckedThisSaveableGameObjectInScene[j]) continue; // skip checking this and doing GetComponent
// 					if (saveableGameObjectsInScene[j] == null) continue;
// 
// 					currentGameObjectInScene = saveableGameObjectsInScene[j];
// 					currentSaveObjectInScene = SaveLoad.GetPrefabSaveObject(currentGameObjectInScene);
// 					if (!currentSaveObjectInScene.instantiated) alreadyCheckedThisInstantiableGameObjectInScene[j] = true; // Huge time saver right here!
// 
// 					// Static Objects all have unique ID.
// // 					if (currentSaveObjectInScene.SaveID == 999999) UnityEngine.DualLog("Checking player during load");
// 					if (currentSaveObjectInScene.SaveID == saveFile_Line_SaveID[i]
// 						&& currentSaveObjectInScene.SaveID != 0) {
// 						
// // 						if (currentSaveObjectInScene.SaveID == 999999) UnityEngine.DualLog("Found player in savefile on line " + i.ToString() + " during load");
// 
// 						//if (!saveableGameObjectsInScene[j].isStatic // EDITOR ONLY!!!
// 						if (currentSaveObjectInScene.instantiated
// 							&& currentSaveObjectInScene.saveType != SaveableType.Light) {
// 							UnityEngine.DualLog("For some reason, attempting "
// 												  + "to load to dynamic object "
// 												  + saveableGameObjectsInScene[j].name);
// 						}
// 
// 						entries = readFileList[i].Split(Utils.splitCharChar);
// 						PrefabIdentifier prefID = SaveLoad.GetPrefabIdentifier(currentGameObjectInScene,true);
// 						SaveObject.Load(currentGameObjectInScene,ref entries,i,prefID);
// 						alreadyCheckedThisSaveableGameObjectInScene[j] = true; // Huge time saver right here!
// 						break;
// 					}
// 				}
// 
// 				perc = (float)i/(float)numSaveablesFromSavefile*100f;
// 				loadPercentText.text = "Loading Static Objects: "
// 									   + perc.ToString("0.0") + "% ("
// 									   + i.ToString() + " / "
// 									   + numSaveablesFromSavefile.ToString()
// 									   + ")";
// 									   
// 				if (loadUpdateTimer.ElapsedMilliseconds > 500) {
// 					loadUpdateTimer.Reset();
// 					loadUpdateTimer.Start();
// 					Cursor.lockState = CursorLockMode.None;
// 					Cursor.visible = true;
// 					yield return null;
// 				}
// 			}
// 			loadUpdateTimer.Stop();
// 
// 			// Check if we missed a static non-instantiable object to load to.
// 			int numberOfMissedObjects = 0;
// 			SaveObject sob;
// 			for (i=0;i<saveableGameObjectsInScene.Count;i++) {
// 				if (alreadyCheckedThisInstantiableGameObjectInScene[i]) {
// 					continue;
// 				}
// 
// 				sob = SaveLoad.GetPrefabSaveObject(saveableGameObjectsInScene[i]);
// 				if (sob != null) {
// 					if (!sob.instantiated) {
// 						UnityEngine.DualLog(saveableGameObjectsInScene[i].name
// 						+ " not loaded during Static Pass and is static");
// 					} else {
// 						UnityEngine.DualLog(saveableGameObjectsInScene[i].name
// 						+ " not loaded during Static Pass and is not static");
// 					}
// 				} else {
// 					UnityEngine.DualLog(saveableGameObjectsInScene[i].name
// 						+ " not loaded during Static Pass and is not static");
// 				}
// 				numberOfMissedObjects++;
// 			}
// 			if (numberOfMissedObjects > 0) {
// 				UnityEngine.DualLog("numberOfMissedObjects: "
// 									  + numberOfMissedObjects.ToString());
// 			}
// 
// 			// LOAD 7d. INSTANTIATE AND LOAD TO INSTANTIATED SAVEABLES
// 			// Now time to instantiate anything left that's supposed to be here
// 			loadUpdateTimer.Start(); // For loading update
// 			int constdex = -1; // To store the index of Master Index table.
// 			int levID = 1; // To store the level this was in.
// 			int savID = -1; // To store the SaveObject.SaveID.
// 			float percLoaded = 0f;
// 			GameObject instGO = null;
// 			GameObject contnr = null;
// // 			UnityEngine.DualLog("numSaveFileLines: " + numSaveFileLines.ToString());
// 			for (i = 3 ; i < numSaveFileLines; i++) {
// 				if (alreadyLoadedLineFromSaveFile[i]) continue;
// 
// 				entries = readFileList[i].Split(Utils.splitCharChar);
// 				if (entries.Length > 1) {
// 					constdex = Utils.GetIntFromString(entries[0],"constIndex");
// 					levID = Utils.GetIntFromString(entries[19],"levelID");
// 					if (!ConsoleEmulator.ConstIndexInBounds(constdex)) continue;
// 
// 					// Already did LevelManager.a.LoadLevel above, and since its
// 					// savestrings lists were empty, safe to spawn dynamics now.
// 					savID = Utils.GetIntFromString(entries[2],"SaveID");
// 					if (ConsoleEmulator.ConstIndexIsNPC(constdex)) {
// 						contnr = LevelManager.a.GetRequestedLevelNPCContainer(levID);
// 						instGO = ConsoleEmulator.SpawnDynamicObject(constdex,levID,false,contnr,savID);
// 						PrefabIdentifier prefID = SaveLoad.GetPrefabIdentifier(instGO,true);
// 						SaveObject.Load(instGO,ref entries,i,prefID); // Load NPC.
// 					} else if (ConsoleEmulator.ConstIndexIsDynamicObject(constdex)) {
// 						// For DynamicObjects, if current level, go ahead and Instantiate new Prefabs, else add string to LevelManager's list for other levels.
// 						if (levID == LevelManager.a.currentLevel) {
// 							contnr = LevelManager.a.GetRequestedLevelDynamicContainer(levID);
// 							instGO = ConsoleEmulator.SpawnDynamicObject(constdex,levID,false,contnr,savID);
// 							PrefabIdentifier prefID = SaveLoad.GetPrefabIdentifier(instGO,true);
// 							SaveObject.Load(instGO,ref entries,i,prefID); // Load NPC.
// 						} else {
// 							if (levID < LevelManager.a.DynamicObjectsSavestrings.Length && levID >= 0) { // levID < 14
// 								if (i < (readFileList.Count - 1) && readFileList.Count > 0 && i >= 0) {
// 									LevelManager.a.DynamicObjectsSavestrings[levID].Add(readFileList[i]);
// 								}
// 							}
// 						}
// 					}
// 
// 				}
// 
// 				percLoaded = ((float)i / (float)numSaveablesFromSavefile*100f);
// 				loadPercentText.text = "Loading Dynamic Objects: "
// 									   + percLoaded.ToString("0.0")
// 									   + "% (" + i.ToString() + " / "
// 									   + numSaveablesFromSavefile.ToString()
// 									   + ")";
// 				if (loadUpdateTimer.ElapsedMilliseconds > 50) {
// 					loadUpdateTimer.Reset();
// 					loadUpdateTimer.Start();
// 					Cursor.lockState = CursorLockMode.None;
// 					Cursor.visible = true;
// 					yield return null;
// 				}
// 			}
// 			
// 			// OK we read in all the dynamic objects above into the savestrings
// 			// list, now actaully instantiate them.
// 			LevelManager.a.LoadLevelDynamicObjects(LevelManager.a.currentLevel);
// 			loadUpdateTimer.Stop();
// 
// 			// LOAD 8.  Repopulate registries as needed that were on Awake.
// 			for (i = 0; i < LevelManager.a.npcsm.Length; i++ ) {
// 				LevelManager.a.npcsm[i].RepopulateChildList();
// 			}
// 			
// 			if (inventoryPlayer1.hasHardware[1]) {
// 				// Go through all HealthManagers in the game and initialize the
// 				// linked overlays now for Automap.  Done after instantiation.
// 				List<GameObject> hmGOs = new List<GameObject>();
// 				
// 				// Find all HealthManager components.
// 				bool includeInactive = true;
// 				for (i=0;i<allParents.Count;i++) {
// 					Component[] compArray =
// 						allParEng_Global->instances[i].GetComponentsInChildren(
// 							typeof(HealthManager),includeInactive);
// 
// 					// Add all gameObject with a HealthManager components.
// 					for (k=0;k<compArray.Length;k++) hmGOs.Add(compArray[k].gameObject);
// 				}
// 
// 				for (i=0;i<hmGOs.Count;i++) {
// 					if (hmGOs[i] == null) continue;
// 
// 					HealthManager hm = hmGOs[i].GetComponent<HealthManager>();
// 					if (hm == null) continue;
// 
// 					if ((hm.isNPC || hm.isSecCamera)) {
// 						hm.Awake(); // Set up slots.
// 						hm.Start(); // Setup overlay.
// 					}
// 				}
// 			}
// 		}
// 		
// 		loadPercentText.text = "Re-register targets...";
// 		yield return null;
// 		for (i=0;i<allParents.Count;i++) {
// 			Component[] compArray = allParEng_Global->instances[i].GetComponentsInChildren(typeof(TargetIO),true); // find all SaveObject components, including inactive (hence the true here at the end)
// 			for (k=0;k<compArray.Length;k++) {
// 				TargetIO tio = compArray[k].gameObject.GetComponent<TargetIO>();
// 				if (tio != null) {
// 					tio.RemoteStart(this.gameObject,"LoadRoutine()"); // Reregister
// 				}
// 			}
// 		}
// 		
// 		allParents.Clear();
// 		allParents = null; // Done with it.
// 		ResetPauseLists();
// 		loadPercentText.text = "Re-init cull systems...";
// 		yield return null;
// 		DynamicCulling.a.Cull_Init();
// 		DynamicCulling.a.CullCore();
// 		loadPercentText.text = "Cleaning Up...";
// 		yield return null;
// 
//  		System.GC.Collect(); // Collect it all!
// 		System.GC.WaitForPendingFinalizers();
// 		AutoSplitterData.isLoading = false;
// 		loadTimer.Stop();
// 		loading = false;
// 		loadPercentText.text = "";
// 		GoIntoGame(loadTimer);
// 	}
