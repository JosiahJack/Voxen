


	

	

	

	// Called by Automap.cs.  This handles the UI changes to make room.
	public void AutomapGoFull() {
		World.Sys_UI.mouseClickHeldOverGUI = true;
		
		ctbButtonMain.SetActive(false);
		ctbButtonHardware.SetActive(false);
		ctbButtonGeneral.SetActive(false);
		DisableAllCenterTabs();
		TabReset(true); // right
		TabReset(false); // left
		tabButtonsLHButtons.SetActive(false);
		tabButtonsRHButtons.SetActive(false);
		leftTC.TurnAllTabsOff();
		rightTC.TurnAllTabsOff();
		
	}

	// Handles returning UI back to how it was before clearing the board.
	public void CloseFullmap() {
		World.Sys_UI.mouseClickHeldOverGUI = true;
		if (!Const.a.noHUD) {
			Utils.Activate(ctbButtonMain);
			Utils.Activate(ctbButtonHardware);
			Utils.Activate(ctbButtonGeneral);
			//CenterTabButtonClickSilent(curCenterTab,true);
			Utils.Activate(tabButtonsLHButtons);
			Utils.Activate(tabButtonsRHButtons);
			TabReset(true); // right
			TabReset(false); // left
			ReturnToLastTab(true);
			ReturnToLastTab(false);
		}
		
		
	}

	// Called by MouseLookScript.cs
	public void EnterCyberspace() {
		World.Sys_UI.hwb.HideSensaround();
		TabReset(true); // right
		TabReset(false); // left
		rightTC.TurnAllTabsOff();
		leftTC.TurnAllTabsOff();
		ctbButtonMain.SetActive(false);
		ctbButtonHardware.SetActive(false);
		ctbButtonGeneral.SetActive(false);
		tabButtonsLHButtons.SetActive(false);
		tabButtonsRHButtons.SetActive(false);
		energyTickPanel.SetActive(false);
		energyIndicator.SetActive(false);
		healthIndicator.SetActive(false);
		bool segiLast = Const.a.GraphicsSEGI;
		Const.a.GraphicsSEGI = false;
		Config.SetSEGI();
		Const.a.GraphicsSEGI = segiLast; // Preserve setting for return from Cyberspace.
		if (!Const.a.noHUD) cyberHealthIndicator.SetActive(true);
		if (!Const.a.noHUD) cyberTimerT.SetActive(true);
		if (!Const.a.noHUD) cyberTimer.SetActive(true);
		hardwareButtonsContainer.SetActive(false);
		viewWeaponsContainer.SetActive(false);
		CyberTimer ct = cyberTimer.GetComponent<CyberTimer>();
		if (ct != null) ct.Reset(World.diffCyb);
		CenterTabButtonClickSilent(3,true);
		
	}

	// Called by MouseLookScript
	public void ExitCyberspace() {
		TabReset(true);
		TabReset(false);
		ReturnToLastTab(true);
		ReturnToLastTab(false);
		ctbButtonMain.SetActive(true);
		ctbButtonHardware.SetActive(true);
		ctbButtonGeneral.SetActive(true);
		if (World.invP1.hardwareIsActive[3]) World.Sys_UI.hwb.UnhideSensaround();
		tabButtonsLHButtons.SetActive(true);
		tabButtonsRHButtons.SetActive(true);
		Config.SetSEGI(); // Turn it back on if setting is on.
		if (!Const.a.noHUD) {
			energyTickPanel.SetActive(true);
			energyIndicator.SetActive(true);
			healthIndicator.SetActive(true);
			hardwareButtonsContainer.SetActive(true);
		}
		cyberHealthIndicator.SetActive(false);
		cyberSprintContainer.SetActive(false);
		cyberTimerT.SetActive(false);
		cyberTimer.SetActive(false);
		viewWeaponsContainer.SetActive(true);
		CenterTabButtonClickSilent(0,true);
		
	}

	public void CyberSprint (string message) {
		cyberSprintContainer.SetActive(true);
		cyberSprintText.text = message;
	}

	public void RevertDataTabState() {
		TabReset(true);
		TabReset(false);
		usingObject = false;
		logTable.SetActive(false);
		logLevelsFolder.SetActive(false);
		logReaderContainer.SetActive(false);
		ReturnToLastTab(true);
		ReturnToLastTab(false);
	}

	public void ClosePuzzleGrid() {
		World.Sys_UI.mouseClickHeldOverGUI = true;
		PuzzleGrid pg = puzzleGridLH.GetComponent<PuzzleGrid>();
		PuzzleGrid pgr = puzzleGridRH.GetComponent<PuzzleGrid>();
		tetheredPGP.SendDataBackToPanel(pg);
		
		pg.Reset();
		pgr.Reset();
		tetheredPGP = null;
		RevertDataTabState();
	}

	public void ClosePuzzleWire() {
		World.Sys_UI.mouseClickHeldOverGUI = true;
		PuzzleWire pw = puzzleWireLH.GetComponent<PuzzleWire>();
		PuzzleWire pwr = puzzleWireRH.GetComponent<PuzzleWire>();
		tetheredPWP.SendDataBackToPanel(pw,false);
		
		pw.Reset();
		pwr.Reset();
		tetheredPWP = null;
		RevertDataTabState();
	}

	public void CloseElevatorPad() {
		World.Sys_UI.mouseClickHeldOverGUI = true;
		tetheredKeypadElevator.SendDataBackToPanel();
		
		TurnOffElevatorPad();
		tetheredKeypadElevator = null;
		linkedElevatorDoor = null;
		RevertDataTabState();
	}

	public void CloseKeycodePad() {
		World.Sys_UI.mouseClickHeldOverGUI = true;
		TurnOffKeypad();
		
		tetheredKeypadKeycode = null;
		RevertDataTabState();
	}

	public void CloseSearch() {
		World.Sys_UI.mouseClickHeldOverGUI = true;
		
		if (tetheredSearchable != null) tetheredSearchable.ResetSearchable(false);
		tetheredSearchable = null;
		searchCloseButtonLH.SetActive(false);
		searchCloseButtonRH.SetActive(false);
		
		if (leftTC.TabManager.DataTab.activeSelf
			&& searchContainerLH.gameObject.activeSelf) {
			TabReset(false);
			logTable.SetActive(false);
			logLevelsFolder.SetActive(false);
			logReaderContainer.SetActive(false);
			ReturnToLastTab(false);
		}

		if (rightTC.TabManager.DataTab.activeSelf
			&& searchContainerRH.gameObject.activeSelf) {
			TabReset(true);
			ReturnToLastTab(true);
		}
		usingObject = false;
	}

	public void ClosePaperLog() {
		World.Sys_UI.mouseClickHeldOverGUI = true;
		
		CenterTabButtonClickSilent(curCenterTab,false);
	}

	public void DrawTicks(bool health) {
		tempSpriteIndex = -1;
		float checkVal = 0;
		if (health) {
			if (MouseLookScript.a.inCyberSpace) {
				checkVal = World.instances[PLAYER1].cyberHealth;
			} else {
				checkVal = World.instances[PLAYER1].health;
			}
		} else {
			checkVal = PlayerEnergy.a.energy;
		}

		// Always display ticks properly no matter what crazy value  we've been
		// hacked to have.
		if (checkVal > 255f) checkVal = 255f; 
		for (int i=1;i<24;i++) {
			if (checkVal < (11f * i)) tempSpriteIndex++;
		}

		tempSpriteIndex++;
		if (tempSpriteIndex >= 0 && tempSpriteIndex < 25) {
			if (health) {
				tickImageHealth.overrideSprite = tickImages[tempSpriteIndex];
			} else {
				tickImageEnergy.overrideSprite = tickImages[tempSpriteIndex];
			}
		} else {
			if (health) {
				tickImageHealth.overrideSprite = tickImages[24];
			} else {
				tickImageEnergy.overrideSprite = tickImages[24];
			}
		}
	}

	void WeaponButtonsManagerUpdate() {
		for (int i=0; i<7; i++) {
			WeaponButton wepbut = wepbutMan.wepButtonsScripts[i];
			GameObject buttonGO = wepbut.gameObject;
			if (World.invP1.weaponInventoryIndices[i] > 0) {
				if (!buttonGO.activeInHierarchy) buttonGO.SetActive(true);
				wepbut.useableItemIndex = World.invP1.weaponInventoryIndices[i];
				if (!wepbutMan.wepCountsText[i].activeInHierarchy) {
					wepbutMan.wepCountsText[i].SetActive(true);
				}
			} else {
				if (buttonGO.activeInHierarchy) buttonGO.SetActive(false);
				wepbut.useableItemIndex = -1;
				if (wepbutMan.wepCountsText[i].activeInHierarchy) {
					wepbutMan.wepCountsText[i].SetActive(false);
				}
			}
		}
	}

	public void OpenTab(int index, bool overrideToggling,TabMSG type,int intdata1, Handedness side) {
		if (side == Handedness.LH) {
			isRH = false;
		} else {
			isRH = true;
		}
		//switch (index) {
		//	case 0: isRH = lastWeaponSideRH; break;
		//	case 1: isRH = lastItemSideRH; break;
		//	case 2: isRH = lastAutomapSideRH; break;
		//	case 3: isRH = lastTargetSideRH; break;
		//	case 4: isRH = lastDataSideRH; break;
		//}
		if(!isRH) {
			// LH LEFT HAND MFD
			leftTC.TabButtonClickSilent(index,overrideToggling);
			if (type == TabMSG.Weapon) {
				TabReset(false);
			}

			if (type == TabMSG.AudioLog) {
				TabReset(false);
				audioLogContainerLH.SetActive(true);
			}

			if (type == TabMSG.Keypad) {
				TabReset(false);
				keycodeUIControlLH.SetActive(true);
				MouseLookScript.a.ForceInventoryMode();
			}

			if (type == TabMSG.Elevator) {
				TabReset(false);
				elevatorUIControlLH.SetActive(true);
				MouseLookScript.a.ForceInventoryMode();
			}

			if (type == TabMSG.GridPuzzle) {
				TabReset(false);
				puzzleGridLH.SetActive(true);
				MouseLookScript.a.ForceInventoryMode();
			}

			if (type == TabMSG.WirePuzzle) {
				TabReset(false);
				puzzleWireLH.SetActive(true);
				MouseLookScript.a.ForceInventoryMode();
			}

			if (type == TabMSG.EReader) {
				TabReset(false);
				itemTabLH.EReaderSectionSContainerOpen();
				MouseLookScript.a.ForceInventoryMode();
			}
			if (type == TabMSG.SystemAnalyzer) {
				TabReset(false);
				sysAnalyzerLH.SetActive(true);
			}
		} else {
			// RH RIGHT HAND MFD
			rightTC.TabButtonClickSilent(index,overrideToggling);
			if (type == TabMSG.AudioLog) {
				TabReset(true);
				audioLogContainerRH.SetActive(true);
			}

			if (type == TabMSG.Keypad) {
				TabReset(true);
				keycodeUIControlRH.SetActive(true);
			}

			if (type == TabMSG.Elevator) {
				TabReset(true);
				elevatorUIControlRH.SetActive(true);
			}

			if (type == TabMSG.GridPuzzle) {
				TabReset(true);
				puzzleGridRH.SetActive(true);
			}

			if (type == TabMSG.WirePuzzle) {
				TabReset(true);
				puzzleWireRH.SetActive(true);
				MouseLookScript.a.ForceInventoryMode();
			}

			if (type == TabMSG.EReader) {
				TabReset(true);
				itemTabRH.EReaderSectionSContainerOpen();
				MouseLookScript.a.ForceInventoryMode();
			}
			if (type == TabMSG.SystemAnalyzer) {
				TabReset(true);
				sysAnalyzerRH.SetActive(true);
			}
		}
	}

	public void ResetItemTab() {
		itemTabLH.Reset();
		itemTabRH.Reset();
	}

	public void SendInfoToItemTab(int index, int customIndex) {
		if (index < 0 || index > 110) { ResetItemTab(); return; }

		itemTabLH.SendItemDataToItemTab(index,customIndex);
		itemTabRH.SendItemDataToItemTab(index,customIndex);
	}

	public void SendInfoToItemTab(int index) {
		SendInfoToItemTab(index,-1);
	}

	// Clicking [Apply] button on left or right MFD's Item Tab to apply current patch or general inventory item.
	public void ApplyButtonClicked() {
		World.Sys_UI.mouseClickHeldOverGUI = true;
		itemTabLH.applyButton.SetActive(false);
		itemTabRH.applyButton.SetActive(false);
		if (applyButtonReferenceIndex < 0) return;

		if (applyButtonReferenceIndex == 55 // Health kit was applied
			|| applyButtonReferenceIndex == 52
			|| applyButtonReferenceIndex == 53) {
			// General Inventory
			// ----------------------------------------------------------------
			GameObject invbtn = 
				World.invP1.genButtons[World.invP1.generalInvCurrent];

			if (invbtn != null) {
				invbtn.GetComponent<GeneralInvButton>().DoubleClick();
			}

			int nextIndex = World.invP1.generalInvIndex - 1;
			if (nextIndex < 0) nextIndex = 0;
			World.invP1.generalInvIndex = nextIndex;

			// Set item tab to next general inv current.
			SendInfoToItemTab(World.invP1.generalInvIndex);
		} else {
			// Patches
			// ----------------------------------------------------------------
			World.invP1.patchButtonScripts[World.invP1.patchCurrent].DoubleClick();

			// Set item tab to next patch.
			SendInfoToItemTab(World.invP1.patchIndex);
		}
	}

	public void Search(bool isRH, string head, int numberFoundContents, int[] contents, int[] customIndex) {
		if (isRH) {
			headerTextRH.SetActive(true);
			headerText_textRH.enabled = true;
			headerText_textRH.text = head;
			if (numberFoundContents <= 0) {
				noItemsTextRH.SetActive(true);
				noItemsTextRH.GetComponent<Text>().enabled = true;
				return;
			}
			for (int i=0;i<4;i++) {
				if (contWorld.instances[i] > -1) {
					searchCloseButtonRH.SetActive(true);
					searchItemImagesRH[i].SetActive(true);
					searchItemImagesRH[i].GetComponent<Image>().overrideSprite = null;
					searchItemImagesRH[i].GetComponent<Image>().overrideSprite = Const.a.GetSpriteFromTexture(contWorld.instances[i]);
					searchContainerRH.contWorld.instances[i] = contWorld.instances[i];
					searchContainerRH.customIndex[i] = customIndex[i];
				}
			}
			searchCloseButtonRH.SetActive(true);
		} else {
			headerTextLH.SetActive(true);
			headerText_textLH.enabled = true;
			headerText_textLH.text = head;
			if (numberFoundContents <= 0) {
				noItemsTextLH.SetActive(true);
				noItemsTextLH.GetComponent<Text>().enabled = true;
				return;
			}
			for (int i=0;i<4;i++) {
				if (contWorld.instances[i] > -1) {
					searchCloseButtonLH.SetActive(true);
					searchItemImagesLH[i].SetActive(true);
					searchItemImagesLH[i].GetComponent<Image>().overrideSprite = null;
					searchItemImagesLH[i].GetComponent<Image>().overrideSprite = Const.a.GetSpriteFromTexture(contWorld.instances[i]);
					searchContainerLH.contWorld.instances[i] = contWorld.instances[i];
					searchContainerLH.customIndex[i] = customIndex[i];
				}
			}
			searchCloseButtonLH.SetActive(true);
		}
	}

	public void SendSearchToDataTab(string name, int contentCount, int[] resultContents, int[] resultsIndices, V3 searchPosition, SearchableItem si, bool useFX) {
		PrefabIdentifier pid = si.gameObject.GetComponent<PrefabIdentifier>();
		string headerName = name;
		if (pid != null) {
			switch(pid.constIndex) {
				case 464: headerName = Text->stringTable[895]; break;
				case 465: headerName = Text->stringTable[897]; break;
				case 530: headerName = Text->stringTable[898]; break;
				case 466: headerName = Text->stringTable[897]; break;
				case 467: headerName = Text->stringTable[897]; break;
				case 468: headerName = Text->stringTable[897]; break;
				case 469: headerName = Text->stringTable[897]; break;
				case 470: headerName = Text->stringTable[897]; break;
				case 471: headerName = Text->stringTable[897]; break;
				case 472: headerName = Text->stringTable[899]; break;
				case 473: headerName = Text->stringTable[899]; break;
				case 474: headerName = Text->stringTable[899]; break;
				case 475: headerName = Text->stringTable[899]; break;
				case 476: headerName = Text->stringTable[899]; break;
				case 531: headerName = Text->stringTable[896]; break;
			}
		}

		TabReset(lastSearchSideRH);
		
		// Still turn off opposite side Data Tab contents so they don't get multiple on at once.
		if (lastSearchSideRH) {
			noItemsTextLH.SetActive(false);
			blockedBySecurityLH.SetActive(false);
			elevatorUIControlLH.SetActive(false);
			keycodeUIControlLH.SetActive(false);
			puzzleGridLH.SetActive(false);
			puzzleWireLH.SetActive(false);
			audioLogContainerLH.SetActive(false);
			sysAnalyzerLH.SetActive(false);
			
			// Only on the left
			miniGamesContainer.SetActive(false);
			minigameSpace.SetActive(false);
			minigameButtonsContainer.SetActive(false);
			minigameViewContainer.SetActive(false);
			minigamePingSpaceContainer.SetActive(false);
			minigame15SpaceContainer.SetActive(false);
			minigameWing0SpaceContainer.SetActive(false);
			minigameBotbounceSpaceContainer.SetActive(false);
			minigameEelZapperSpaceContainer.SetActive(false);
			minigameRoadSpaceContainer.SetActive(false);
			minigameTriopToeSpaceContainer.SetActive(false);
			minigameCorpConqSpaceContainer.SetActive(false);
			minigameChessSpaceContainer.SetActive(false);
			minigameSpace.SetActive(false);
			minigameCamera.SetActive(false);
		} else {
			noItemsTextRH.SetActive(false);
			blockedBySecurityRH.SetActive(false);
			elevatorUIControlRH.SetActive(false);
			keycodeUIControlRH.SetActive(false);
			puzzleGridRH.SetActive(false);
			puzzleWireRH.SetActive(false);
			audioLogContainerRH.SetActive(false);
			sysAnalyzerRH.SetActive(false);
		}

		if (lastSearchSideRH) {
			OpenTab(4,true,TabMSG.Search,0,Handedness.RH);
			if (useFX) SearchFXRH.SetActive(true); // Enable search box scaling effect
		} else {
			OpenTab(4,true,TabMSG.Search,0,Handedness.LH);
			if (useFX) SearchFXLH.SetActive(true); // Enable search box scaling effect
		}
		
		Search(true,headerName,contentCount,resultContents,resultsIndices);
		Search(false,headerName,contentCount,resultContents,resultsIndices);
		if (tetheredSearchable != si) {
			if (tetheredSearchable != null) {
				tetheredSearchable.ResetSearchable(false);
				tetheredSearchable = null;
			}
		}
		tetheredSearchable = si;
		objectInUsePos = searchPosition;
		usingObject = true;
	}

	public void SendGridPuzzleToDataTab(bool[] states, PuzzleCellType[] types,
										 PuzzleGridType gtype, int start,
										 int end, int width, int height,
										 HUDColor colors, string t1, 
										 UseData ud, V3 tetherPoint,
										 PuzzleGridPuzzle pgp) {
		if (lastDataSideRH) {
			// Send to RH tab
			TabReset(true);
			puzzleGridRH.GetComponent<PuzzleGrid>().SendGrid(states,types,
															 gtype,start,end,
															 width,height,
															 colors,t1,ud,pgp);
			OpenTab(4,true,TabMSG.GridPuzzle,0,Handedness.RH);
			SearchFXRH.SetActive(true);
		} else {
			// Send to LH tab
			TabReset(false);
			puzzleGridLH.GetComponent<PuzzleGrid>().SendGrid(states,types,
															 gtype,start,end,
															 width,height,
															 colors,t1,ud,pgp);
			OpenTab(4,true,TabMSG.GridPuzzle,0,Handedness.LH);
			SearchFXLH.SetActive(true);
		}
		objectInUsePos = tetherPoint;
		tetheredPGP = pgp;
		usingObject = true;
	}

	public void SendWirePuzzleToDataTab(bool[] sentWiresOn, bool[] sentNodeRowsActive, int[] sentCurrentPositionsLeft, int[] sentCurrentPositionsRight, int[] sentTargetsLeft, int[] sentTargetsRight, HUDColor theme, HUDColor[] wireColors, string t1, UseData udSent,V3 tetherPoint, PuzzleWirePuzzle pwp) {
		TabReset(lastDataSideRH);
		if (lastDataSideRH) {
			// Send to RH tab
			puzzleWireRH.GetComponent<PuzzleWire>().SendWirePuzzleData(sentWiresOn,sentNodeRowsActive,sentCurrentPositionsLeft,sentCurrentPositionsRight,sentTargetsLeft,sentTargetsRight,theme,wireColors,t1,udSent,pwp);
			OpenTab(4,true,TabMSG.WirePuzzle,0,Handedness.RH);
			SearchFXRH.SetActive(true);
		} else {
			// Send to LH tab
			puzzleWireLH.GetComponent<PuzzleWire>().SendWirePuzzleData(sentWiresOn,sentNodeRowsActive,sentCurrentPositionsLeft,sentCurrentPositionsRight,sentTargetsLeft,sentTargetsRight,theme,wireColors,t1,udSent,pwp);
			OpenTab(4,true,TabMSG.WirePuzzle,0,Handedness.LH);
			SearchFXLH.SetActive(true);
		}
		objectInUsePos = tetherPoint;
		tetheredPWP = pwp;
		usingObject = true;
	}

	public void SendPaperLogToDataTab(int index,V3 tetherPoint) {
		if (Const.a.audioLogImagesRefIndicesLH[index] != 0) { // LH, but only
															  // if has image.
			TabReset(false);
			OpenTab(4,true,TabMSG.AudioLog,index,Handedness.LH);
		}
		if (Const.a.audioLogImagesRefIndicesRH[index] != 0) { // RH, but only
															  // if has image.
			TabReset(true);
			OpenTab(4,true,TabMSG.AudioLog,index,Handedness.RH);
		}
		
		logDataTabInfoLH.SendLogData(index,false); // false for LH
		logDataTabInfoRH.SendLogData(index,true);  // true for RH
		objectInUsePos = tetherPoint;
		paperLogInUse = true;
		usingObject = true;
		OpenLogTextReader();
		DataReaderContentTab.SetActive(true);
		logReaderContainer.SetActive(true);
		logReaderContainer.GetComponent<LogTextReaderManager>().SendTextToReader(index);
		logTable.SetActive(false);
		logLevelsFolder.SetActive(false);
	}

	public void SendAudioLogToDataTab(int index) {
		TabReset(false);
		OpenTab(4,true,TabMSG.AudioLog,index,Handedness.LH);  // LH
		if (Const.a.audioLogImagesRefIndicesRH[index] != 0) { // RH, but only
															  // if has image.
			DualLog("Activating 2nd image for logs");
			TabReset(true);
			OpenTab(4,true,TabMSG.AudioLog,index,Handedness.RH);
		}

		logDataTabInfoLH.SendLogData(index,false); // false for LH
		logDataTabInfoRH.SendLogData(index,true);  // true for RH
		CenterTabButtonClickSilent(4,true);
		if (tetheredSearchable != null) tetheredSearchable.searchableInUse = false;
		OpenLogTextReader();
		DataReaderContentTab.SetActive(true);
		logReaderContainer.SetActive(true);
		logReaderContainer.GetComponent<LogTextReaderManager>().SendTextToReader(index);
		logTable.SetActive(false);
		logLevelsFolder.SetActive(false);
		if (Const.a.audioLogs[index] != null) logFinished = World.pauseRelativeTime + Const.a.audioLogs[index].length + 0.1f; //add slight delay after log is finished playing to make sure we don't cut off audio in case there's a frame delay for audio start
		logActive = true;
		logType = Const.a.audioLogType[index];
	}

	public void OpenLastItemSide() {
		if (lastItemSideRH) {
			OpenTab(1,true,TabMSG.EReader,-1,Handedness.RH);
		} else {
			OpenTab(1,true,TabMSG.EReader,-1,Handedness.LH);
		}
	}

	public void OpenEReaderInItemsTab() {
		OpenTab(1,true,TabMSG.EReader,-1,Handedness.LH);
		CenterTabButtonClickSilent(4,false);
		if (tetheredSearchable != null) tetheredSearchable.searchableInUse = false;
		logTable.SetActive(false);
		logLevelsFolder.SetActive(false);
		logReaderContainer.SetActive(false);
		OpenLastMultiMediaTab();
	}

	public void ClearDataTab(bool isRH) {
		TabReset(isRH);
	}

	public void TurnOffKeypad() {
		if (lastDataSideRH) {
			keycodeUIControlRH.SetActive(false);
		} else {
			keycodeUIControlLH.SetActive(false);
		}
	}

	public void TurnOffElevatorPad() {
		if (lastDataSideRH) {
			elevatorUIControlRH.SetActive(false);
		} else {
			elevatorUIControlLH.SetActive(false);
		}
	}

	public bool GetElevatorControlActiveState() {
		if (lastDataSideRH) {
			return elevatorUIControlRH.activeInHierarchy;
		} else {
			return elevatorUIControlLH.activeInHierarchy;
		}
	}

	public void BlockedBySecurity(V3 tetherPoint) {
		TabReset(lastDataSideRH);
		if (lastDataSideRH) {
			OpenTab(4,true,TabMSG.None,0,Handedness.RH);
			blockedBySecurityRH.SetActive(true);
		} else {
			OpenTab(4,true,TabMSG.None,0,Handedness.LH);
			blockedBySecurityLH.SetActive(true);
		}

		CenterStatusPrint(25);
		Utils.PlayUIOneShotSavable(468,0.85f);
		objectInUsePos = tetherPoint;
		usingObject = true;
	}

	public void SendKeypadKeycodeToDataTab(int keycode, V3 tetherPoint,
										   KeypadKeycode keypad,
										   bool alreadySolved) {
		if (keycode < 0 || keypad == null) {
			KeypadKeycodeButtons kkbRH =
				keycodeUIControlRH.GetComponent<KeypadKeycodeButtons>();

			kkbRH.keycode = 0;
			kkbRH.keypad = null;
			kkbRH.ResetEntry();
			kkbRH.currentEntry = 0;


			KeypadKeycodeButtons kkbLH =
				keycodeUIControlLH.GetComponent<KeypadKeycodeButtons>();

			kkbLH.keycode = 0;
			kkbLH.keypad = null;
			kkbLH.ResetEntry();
			kkbLH.currentEntry = 0;
			return;
		}

		TabReset(lastDataSideRH);

		if (lastDataSideRH) {
			OpenTab(4,true,TabMSG.Keypad,0,Handedness.RH);
			keycodeUIControlRH.SetActive(true);
			KeypadKeycodeButtons kkb =
				keycodeUIControlRH.GetComponent<KeypadKeycodeButtons>();
			kkb.keycode = keycode;
			kkb.keypad = keypad;
			kkb.ResetEntry();
			if (World.diffMis <= 1) {
				kkb.currentEntry = keycode;
			}
		} else {
			OpenTab(4,true,TabMSG.Keypad,0,Handedness.LH);
			keycodeUIControlLH.SetActive(true);
			KeypadKeycodeButtons kkb =
					keycodeUIControlLH.GetComponent<KeypadKeycodeButtons>();
			kkb.keycode = keycode;
			kkb.keypad = keypad;
			kkb.ResetEntry();
			if (World.diffMis <= 1) {
				kkb.currentEntry = keycode;
			}
		}

		objectInUsePos = tetherPoint;
		tetheredKeypadKeycode = keypad;
		usingObject = true;
	}

	public void UpdateHUDAmmoCounts(int amount) {
		wepmagCounterLH.UpdateDigits(amount);
		wepmagCounterRH.UpdateDigits(amount);
	}

	public void DisableSearchItemImage(int index) {
		searchItemImagesLH[index].SetActive(false);
		searchItemImagesRH[index].SetActive(false);
	}

	public void ReturnTabsFromSearch() {
		if (leftTC.curTab == 4) leftTC.ReturnToLastTab();
		if (rightTC.curTab == 4) rightTC.ReturnToLastTab();
	}

	public void NotifySearchThatSearchableWasDestroyed() {
		if (tetheredSearchable != null) {
			tetheredSearchable.ResetSearchable(false); // reset the actual object
			// reset the HUD contents
			if (headerTextRH.activeSelf) {
				headerTextRH.SetActive(false);
				headerText_textRH.enabled = false;
				headerText_textRH.text = System.String.Empty;
				noItemsTextRH.SetActive(false);
				noItemsTextRH.GetComponent<Text>().enabled = false;
				searchCloseButtonRH.SetActive(false);
				for (int i=0;i<4;i++) {
					searchItemImagesRH[i].SetActive(false);
					searchItemImagesRH[i].GetComponent<Image>().overrideSprite = null;
					searchItemImagesRH[i].GetComponent<Image>().overrideSprite = Const.a.GetSpriteFromTexture(101);
					searchContainerRH.contWorld.instances[i] = -1;
					searchContainerRH.customIndex[i] = -1;
				}
			}

			if (headerTextLH.activeSelf) {
				headerTextLH.SetActive(false);
				headerText_textLH.enabled = false;
				headerText_textLH.text = System.String.Empty;
				noItemsTextLH.SetActive(false);
				noItemsTextLH.GetComponent<Text>().enabled = false;
				searchCloseButtonLH.SetActive(false);
				for (int i=0;i<4;i++) {
					searchItemImagesLH[i].SetActive(false);
					searchItemImagesLH[i].GetComponent<Image>().overrideSprite = null;
					searchItemImagesLH[i].GetComponent<Image>().overrideSprite = Const.a.GetSpriteFromTexture(101);
					searchContainerLH.contWorld.instances[i] = -1;
					searchContainerLH.customIndex[i] = -1;
				}
			}

			tetheredSearchable = null;
			ReturnTabsFromSearch();
		}
	}


	public void ShowAmmoItems(int normdex, int altdex) {
		Utils.Activate(ammoIndicatorHunsLH);
		Utils.Activate(ammoIndicatorTensLH);
		Utils.Activate(ammoIndicatorOnesLH);
		Utils.Activate(unloadButtonLH);
		Utils.Activate(loadNormalAmmoButtonLH);
		if (altdex >= 0) {
			Utils.Activate(loadAlternateAmmoButtonLH);
		} else {
			Utils.Deactivate(loadAlternateAmmoButtonLH);
		}
		
		Utils.Deactivate(energySliderLH);
		Utils.Deactivate(energyHeatTicksLH);
		Utils.Deactivate(overloadButtonLH);
		if (loadNormalAmmoButtonTextLH != null) {
			if (normdex > 0 && normdex < Text->stringTable.Length) {
				loadNormalAmmoButtonTextLH.text = Text->stringTable[normdex];
			} else {
				loadNormalAmmoButtonTextLH.text = "";
			}
		}

		if (loadAlternateAmmoButtonTextLH != null) {
			if (altdex > 0 && altdex < Text->stringTable.Length) {
				loadAlternateAmmoButtonTextLH.text = Text->stringTable[altdex];
			} else {
				loadAlternateAmmoButtonTextLH.text = "";
			}
		}

		Utils.Activate(ammoIndicatorHunsRH);
		Utils.Activate(ammoIndicatorTensRH);
		Utils.Activate(ammoIndicatorOnesRH);
		Utils.Activate(unloadButtonRH);
		Utils.Activate(loadNormalAmmoButtonRH);
		Utils.Activate(loadAlternateAmmoButtonRH);
		Utils.Deactivate(energySliderRH);
		Utils.Deactivate(energyHeatTicksRH);
		Utils.Deactivate(overloadButtonRH);
		if (loadNormalAmmoButtonTextRH != null) {
			if (normdex > 0 && normdex < Text->stringTable.Length) {
				loadNormalAmmoButtonTextRH.text = Text->stringTable[normdex];
			} else {
				loadNormalAmmoButtonTextRH.text = "";
			}
		}

		if (loadAlternateAmmoButtonTextRH != null) {
			if (altdex > 0 && altdex < Text->stringTable.Length) {
				loadAlternateAmmoButtonTextRH.text = Text->stringTable[altdex];
			} else {
				loadAlternateAmmoButtonTextRH.text = "";
			}
		}
	}

	public void ShowEnergyItems() {
		Utils.Activate(energySliderLH);
		Utils.Activate(energyHeatTicksLH);
		Utils.Activate(overloadButtonLH);
		Utils.Deactivate(ammoIndicatorHunsLH);
		Utils.Deactivate(ammoIndicatorTensLH);
		Utils.Deactivate(ammoIndicatorOnesLH);
		Utils.Deactivate(loadNormalAmmoButtonLH);
		Utils.Deactivate(loadAlternateAmmoButtonLH);
		Utils.Deactivate(unloadButtonLH);

		Utils.Activate(energySliderRH);
		Utils.Activate(energyHeatTicksRH);
		Utils.Activate(overloadButtonRH);
		Utils.Deactivate(ammoIndicatorHunsRH);
		Utils.Deactivate(ammoIndicatorTensRH);
		Utils.Deactivate(ammoIndicatorOnesRH);
		Utils.Deactivate(loadNormalAmmoButtonRH);
		Utils.Deactivate(loadAlternateAmmoButtonRH);
		Utils.Deactivate(unloadButtonRH);
	}

	public void HideAmmoAndEnergyItems() {
		Utils.Deactivate(ammoIndicatorHunsLH);
		Utils.Deactivate(ammoIndicatorTensLH);
		Utils.Deactivate(ammoIndicatorOnesLH);
		Utils.Deactivate(loadNormalAmmoButtonLH);
		Utils.Deactivate(loadAlternateAmmoButtonLH);
		Utils.Deactivate(energySliderLH);
		Utils.Deactivate(energyHeatTicksLH);
		Utils.Deactivate(overloadButtonLH);
		Utils.Deactivate(unloadButtonLH);

		Utils.Deactivate(ammoIndicatorHunsRH);
		Utils.Deactivate(ammoIndicatorTensRH);
		Utils.Deactivate(ammoIndicatorOnesRH);
		Utils.Deactivate(loadNormalAmmoButtonRH);
		Utils.Deactivate(loadAlternateAmmoButtonRH);
		Utils.Deactivate(energySliderRH);
		Utils.Deactivate(energyHeatTicksRH);
		Utils.Deactivate(overloadButtonRH);
		Utils.Deactivate(unloadButtonRH);
	}

	public void HideAlternateAmmoButton() {
		Utils.Deactivate(loadAlternateAmmoButtonRH);
		Utils.Deactivate(loadAlternateAmmoButtonRH);
	}

	public void SetAmmoIcons(int index, bool alt) {
		ammoIconManLH.SetAmmoIcon(index,alt);
		ammoIconManRH.SetAmmoIcon(index,alt);
	}

	void ChangeAmmoButtons(GameObject loadNormalAmmoButton, GameObject loadAlternateAmmoButton) {
		if (loadNormalAmmoButton == null || loadAlternateAmmoButton == null) return;

		int wep16index = Get16WeaponIndexFromConstIndex(World.invP1.weaponIndex);
		if (wep16index == 1 || wep16index == 4 || wep16index == 10 || wep16index == 14 || wep16index == 15) return; // Already hidden.

		Image norm = loadNormalAmmoButton.GetComponent<Image>();
		Image anorm = loadAlternateAmmoButton.GetComponent<Image>();
		if (World.invP1.wepLoadedWithAlternate[World.invP1.weaponCurrent]) {
			SetAmmoIcons(World.invP1.weaponIndex,true);
			norm.overrideSprite = ammoButtonDeHighlighted;
			if (World.invP1.currentMagazineAmount2[World.invP1.weaponCurrent] > 0) {
				anorm.overrideSprite = ammoButtonHighlighted;
			} else {
				anorm.overrideSprite = ammoButtonDeHighlighted;
			}
		} else {
			SetAmmoIcons(World.invP1.weaponIndex,false);
			anorm.overrideSprite = ammoButtonDeHighlighted;
			if (World.invP1.currentMagazineAmount[World.invP1.weaponCurrent] > 0) {
				norm.overrideSprite = ammoButtonHighlighted;
			} else {
				norm.overrideSprite = ammoButtonDeHighlighted;
			}
		}
	}

	public void UpdateHUDAmmoCountsEither() {
		if (World.invP1.weaponCurrent >= 0) {
			if (World.invP1.wepLoadedWithAlternate[World.invP1.weaponCurrent]) {
				UpdateHUDAmmoCounts(World.invP1.currentMagazineAmount2[World.invP1.weaponCurrent]);
			} else {
				UpdateHUDAmmoCounts(World.invP1.currentMagazineAmount[World.invP1.weaponCurrent]);
			}
		}
	}

	void UpdateAmmoAndLoadButtons() {
		if (World.invP1.weaponCurrent < 0
			|| World.invP1.weaponCurrentPending >= 0) {

			return;
		}

		UpdateHUDAmmoCountsEither();
		ChangeAmmoButtons(loadNormalAmmoButtonLH,loadAlternateAmmoButtonLH);
		ChangeAmmoButtons(loadNormalAmmoButtonRH,loadAlternateAmmoButtonRH);
	}

	public void SetWepInfo(int index) { // Expects usableItem index.
		if (index >= 0) {
			weptextRH.text = weptextLH.text = Text->stringTable[index + 326];
			iconRH.overrideSprite = iconLH.overrideSprite = Const.a.useableItemsIcons[index];
		} else {
			weptextRH.text = weptextLH.text = "";
			iconRH.overrideSprite = Const.a.useableItemsIcons[0]; // Nullsprite
			iconLH.overrideSprite = Const.a.useableItemsIcons[0]; // Nullsprite
		}
	}

	public void ReturnToLastTab(bool isRightHand) {
		usingObject = false;
		objectInUsePos = (V3){999f,999f,999f); // out of bounds
		if (isRightHand) {
			rightTC.ReturnToLastTab();
			if (rightTC.lastTab == 4) {
				if (tetheredPGP == null && tetheredPWP == null && tetheredKeypadElevator == null && tetheredKeypadKeycode == null && tetheredSearchable == null) {
					if (World.invP1.hasHardware[0]) sysAnalyzerRH.SetActive(true);
				}
			}
		} else {
			leftTC.ReturnToLastTab();
			if (leftTC.lastTab == 4) {
				if (tetheredPGP == null && tetheredPWP == null && tetheredKeypadElevator == null && tetheredKeypadKeycode == null && tetheredSearchable == null) {
					if (World.invP1.hasHardware[0]) sysAnalyzerLH.SetActive(true);
				}
			}
		}
	}


	// Center tabs
	public void DisableAllCenterTabs () {
		MainTab.SetActive(false);
		HardwareTab.SetActive(false);
		GeneralTab.SetActive(false);
		SoftwareTab.SetActive(false);
		DataReaderContentTab.SetActive(false);
	}

	void ToggleHighlightOnCenterTabButton (int buttonIndex) {
		Image buttonImage = null;
		switch (buttonIndex) {
			case 0: if (buttonImage != MainTabButton.image) buttonImage = MainTabButton.image; break;
			case 1: if (buttonImage != HardwareTabButton.image) buttonImage = HardwareTabButton.image; break;
			case 2: if (buttonImage != GeneralTabButton.image) buttonImage = GeneralTabButton.image; break;
			case 3: if (buttonImage != SoftwareTabButton.image) buttonImage = SoftwareTabButton.image; break;
		}

		if (buttonImage == null) return;
		if (highlightStatus[buttonIndex]) {
			if (buttonImage.overrideSprite != MFDSpriteNotification) buttonImage.overrideSprite = MFDSpriteNotification;
		} else {
			if (curCenterTab == buttonIndex) {
				if (buttonImage.overrideSprite != MFDSpriteSelected) buttonImage.overrideSprite = MFDSpriteSelected;
			} else {
				if (buttonImage.overrideSprite != MFDSprite) buttonImage.overrideSprite = MFDSprite;
			}
		}

		highlightTickCount[buttonIndex]++;
		highlightStatus[buttonIndex] = (!highlightStatus[buttonIndex]);

		if (highlightTickCount[buttonIndex] >= numTicks) {
			highlightStatus[buttonIndex] = false;
			highlightTickCount[buttonIndex] = 0;
			tabNotified[buttonIndex] = false; // stop blinking
			if (curCenterTab == buttonIndex) {
				if (buttonImage.overrideSprite != MFDSpriteSelected) buttonImage.overrideSprite = MFDSpriteSelected; // If we are on this tab, return to selected
			} else {
				if (buttonImage.overrideSprite != MFDSprite) buttonImage.overrideSprite = MFDSprite; // Return to normal
			}
		}
	}

	public void NotifyToCenterTab(int tabNum) {
		tabNotified[tabNum] = true;
		centerTabsTickFinished = World.pauseRelativeTime + centerTabsTickTime;
		ToggleHighlightOnCenterTabButton(tabNum);
	}

	public void CenterTabButtonClick(int tabNum) {
		World.Sys_UI.mouseClickHeldOverGUI = true;
		CenterTabButtonAction(tabNum);
	}

	public void CenterTabButtonAction(int tabNum) {
		if (PauseScript.a.mainMenu.activeInHierarchy) return;

		Utils.PlayUIOneShotSavable(97);
		CenterTabButtonClickSilent(tabNum,false);
		if (World.invP1.hardwareIsActive[3]) {
			hwb.SensaroundOff();
			Utils.PlayUIOneShotSavable(82); // deactivate
		}
	}

	public void CenterTabButtonClickSilent(int tabNum, bool forceOn) {
		bool wasActive = false;

		switch (tabNum) {
		case 0:
			wasActive = MainTab.activeInHierarchy;
			DisableAllCenterTabs();
			if (curCenterTab == 0) {
				if (wasActive && !forceOn) {
					break;
				} else {
					MainTab.SetActive(true);
					break;
				}
			}
			MainTabButton.image.overrideSprite = MFDSpriteSelected;
			DisableAllCenterTabs();
			MainTab.SetActive(true);
			HardwareTabButton.image.overrideSprite = MFDSprite;
			GeneralTabButton.image.overrideSprite = MFDSprite;
			SoftwareTabButton.image.overrideSprite = MFDSprite;
			curCenterTab = 0;
			break;
		case 1:
			wasActive = HardwareTab.activeInHierarchy;
			DisableAllCenterTabs();
			if (curCenterTab == 1) {
				if (wasActive && !forceOn) {
					break;
				} else {
					HardwareTab.SetActive(true);
					break;
				}
			}
			HardwareTabButton.image.overrideSprite = MFDSpriteSelected;
			DisableAllCenterTabs();
			HardwareTab.SetActive(true);
			MainTabButton.image.overrideSprite = MFDSprite;
			GeneralTabButton.image.overrideSprite = MFDSprite;
			SoftwareTabButton.image.overrideSprite = MFDSprite;
			curCenterTab = 1;
			break;
		case 2:
			wasActive = GeneralTab.activeInHierarchy;
			DisableAllCenterTabs();
			if (curCenterTab == 2) {
				if (wasActive && !forceOn) {
					break;
				} else {
					GeneralTab.SetActive(true);
					break;
				}
			}
			GeneralTabButton.image.overrideSprite = MFDSpriteSelected;
			DisableAllCenterTabs();
			GeneralTab.SetActive(true);
			MainTabButton.image.overrideSprite = MFDSprite;
			HardwareTabButton.image.overrideSprite = MFDSprite;
			SoftwareTabButton.image.overrideSprite = MFDSprite;
			curCenterTab = 2;
			break;
		case 3:
			wasActive = SoftwareTab.activeInHierarchy;
			DisableAllCenterTabs();
			if (curCenterTab == 3) {
				if (wasActive && !forceOn) {
					break;
				} else {
					SoftwareTab.SetActive(true);
					break;
				}
			}
			SoftwareTabButton.image.overrideSprite = MFDSpriteSelected;
			DisableAllCenterTabs();
			SoftwareTab.SetActive(true);
			MainTabButton.image.overrideSprite = MFDSprite;
			HardwareTabButton.image.overrideSprite = MFDSprite;
			GeneralTabButton.image.overrideSprite = MFDSprite;
			curCenterTab = 3;
			break;
		case 4:
			DisableAllCenterTabs();
			DataReaderContentTab.SetActive(true);
			OpenLogTableContents();
			MainTabButton.image.overrideSprite = MFDSprite;
			HardwareTabButton.image.overrideSprite = MFDSprite;
			GeneralTabButton.image.overrideSprite = MFDSprite;
			SoftwareTabButton.image.overrideSprite = MFDSprite;
			curCenterTab = 4;
			break;
		}
	}
	//--- End Center Tabs ---

	// Multi Media Tabs
	public void OpenLastMultiMediaTab() {
		switch (lastMultiMediaTabOpened) {
			case 0: OpenEmailTableContents(); break;
			case 1: OpenLogTableContents(); break;
			case 2: OpenDataTableContents(); break;
			case 3: OpenNotesTableContents(); break;
		}
	}

	public void ResetMultiMediaTabs() {
		startingSubTab.SetActive(false);
		secondaryTab1.SetActive(false);
		secondaryTab2.SetActive(false);
		emailTab.SetActive(false);
		ersbLH.SetEReaderSectionsButtonsHighlights(lastMultiMediaTabOpened);
		ersbRH.SetEReaderSectionsButtonsHighlights(lastMultiMediaTabOpened);
		dataTab.SetActive(false);
		notesTab.SetActive(false);
		multiMediaHeaderLabel.text = System.String.Empty;
	}

	public void OpenLogTableContents() {
		DisableAllCenterTabs();
		DataReaderContentTab.SetActive(true);
		World.Sys_UI.mouseClickHeldOverGUI = true;
		ResetMultiMediaTabs();
		startingSubTab.SetActive(true);
		multiMediaHeaderLabel.text = "LOGS";
		lastMultiMediaTabOpened = MM_LOG_TABLE;
		ersbLH.SetEReaderSectionsButtonsHighlights(1);
		ersbRH.SetEReaderSectionsButtonsHighlights(1);
	}

	public void OpenLogsLevelFolder(int curlevel) {
		DisableAllCenterTabs();
		DataReaderContentTab.SetActive(true);
		World.Sys_UI.mouseClickHeldOverGUI = true;
		ResetMultiMediaTabs();
		secondaryTab1.SetActive(true);
		multiMediaHeaderLabel.text = "Level " + curlevel.ToString() + " Logs";
		secondaryTab1.GetComponent<LogContentsButtonsManager>().currentLevelFolder = curlevel;
		secondaryTab1.GetComponent<LogContentsButtonsManager>().InitializeLogsFromLevelIntoFolder();
	}

	public void OpenLogTextReader() {
		DisableAllCenterTabs();
		DataReaderContentTab.SetActive(true);
		World.Sys_UI.mouseClickHeldOverGUI = true;
		ResetMultiMediaTabs();
		secondaryTab2.SetActive(true);
	}

	public void OpenEmailTableContents() {
		DisableAllCenterTabs();
		DataReaderContentTab.SetActive(true);
		World.Sys_UI.mouseClickHeldOverGUI = true;
		ResetMultiMediaTabs();
		emailTab.SetActive(true);
		multiMediaHeaderLabel.text = "EMAIL";
		lastMultiMediaTabOpened = MM_EMAIL_TABLE;
		ersbLH.SetEReaderSectionsButtonsHighlights(0);
		ersbRH.SetEReaderSectionsButtonsHighlights(0);
	}

	public void OpenDataTableContents() {
		DisableAllCenterTabs();
		DataReaderContentTab.SetActive(true);
		World.Sys_UI.mouseClickHeldOverGUI = true;
		ResetMultiMediaTabs();
		dataTab.SetActive(true);
		World.invP1.hasNewData = false;
		multiMediaHeaderLabel.text = "DATA";
		lastMultiMediaTabOpened = MM_DATA_TABLE;
		ersbLH.SetEReaderSectionsButtonsHighlights(2);
		ersbRH.SetEReaderSectionsButtonsHighlights(2);
	}

	public void OpenNotesTableContents() {
		DisableAllCenterTabs();
		DataReaderContentTab.SetActive(true);
		World.Sys_UI.mouseClickHeldOverGUI = true;
		ResetMultiMediaTabs();
		notesTab.SetActive(true);
		World.invP1.hasNewNotes = false;
		multiMediaHeaderLabel.text = "NOTES";
		lastMultiMediaTabOpened = MM_NOTES;
		ersbLH.SetEReaderSectionsButtonsHighlights(3);
		ersbRH.SetEReaderSectionsButtonsHighlights(3);
	}
	//--- End Multi Media Tabs ---

	// Minigames
	public void OpenMinigames() {
		TabReset(true);
		TabReset(false);
		OpenTab(4,true,TabMSG.None,0,Handedness.LH);
		CenterTabButtonClickSilent(3,false);
		miniGamesContainer.SetActive(true);
		minigameButtonsContainer.SetActive(true);
		minigameViewContainer.SetActive(false);

		minigamePingSpaceContainer.SetActive(false);
		minigame15SpaceContainer.SetActive(false);
		minigameWing0SpaceContainer.SetActive(false);
		minigameBotbounceSpaceContainer.SetActive(false);
		minigameEelZapperSpaceContainer.SetActive(false);
		minigameRoadSpaceContainer.SetActive(false);
		minigameTriopToeSpaceContainer.SetActive(false);
		minigameCorpConqSpaceContainer.SetActive(false);
		minigameChessSpaceContainer.SetActive(false);
		minigameSpace.SetActive(false);
		minigameCamera.SetActive(false);
	}

	public void MinigameStart_Ping() {
		CenterStatusPrint("%s", Text->stringTable[1021] + " PING");
		minigameButtonsContainer.SetActive(false);
		minigameViewContainer.SetActive(true);
		minigamePingSpaceContainer.SetActive(true);
		minigameSpace.SetActive(true);
		minigameCamera.SetActive(true);
	}

	public void MinigameStart_15() {
		CenterStatusPrint("%s", Text->stringTable[1021] + " 15");
		minigameButtonsContainer.SetActive(false);
		minigameViewContainer.SetActive(true);
		minigame15SpaceContainer.SetActive(true);
		minigameSpace.SetActive(true);
		minigameCamera.SetActive(true);
	}

	public void MinigameStart_Wing0() {
		CenterStatusPrint("%s", Text->stringTable[1021] + " WING-0");
		minigameButtonsContainer.SetActive(false);
		minigameViewContainer.SetActive(true);
		minigameWing0SpaceContainer.SetActive(true);
		minigameSpace.SetActive(true);
		minigameCamera.SetActive(true);
	}

	public void MinigameStart_Botbounce() {
		CenterStatusPrint("%s", Text->stringTable[1021] + " BOTBOUNCE");
		minigameButtonsContainer.SetActive(false);
		minigameViewContainer.SetActive(true);
		minigameBotbounceSpaceContainer.SetActive(true);
		minigameSpace.SetActive(true);
		minigameCamera.SetActive(true);
	}

	public void MinigameStart_EelZapper() {
		CenterStatusPrint("%s", Text->stringTable[1021] + " EEL ZAPPER");
		minigameButtonsContainer.SetActive(false);
		minigameViewContainer.SetActive(true);
		minigameEelZapperSpaceContainer.SetActive(true);
		minigameSpace.SetActive(true);
		minigameCamera.SetActive(true);
	}

	public void MinigameStart_Road() {
		CenterStatusPrint("%s", Text->stringTable[1021] + " ROAD");
		minigameButtonsContainer.SetActive(false);
		minigameViewContainer.SetActive(true);
		minigameRoadSpaceContainer.SetActive(true);
		minigameSpace.SetActive(true);
		minigameCamera.SetActive(true);
	}

	public void MinigameStart_TriopToe() {
		CenterStatusPrint("%s", Text->stringTable[1021] + " TRIOPTOE");
		minigameButtonsContainer.SetActive(false);
		minigameViewContainer.SetActive(true);
		minigameTriopToeSpaceContainer.SetActive(true);
		minigameSpace.SetActive(true);
		minigameCamera.SetActive(true);
	}

	// The original seemed to have planned for 9 minigames.  Maybe I'll make my
	// own new ones someday.
	public void MinigameStart_CorporateConquer() {
		CenterStatusPrint("%s", Text->stringTable[1021] + " CORP CONQ");
		minigameButtonsContainer.SetActive(false);
		minigameViewContainer.SetActive(true);
		minigameCorpConqSpaceContainer.SetActive(true);
		minigameSpace.SetActive(true);
		minigameCamera.SetActive(true);
	}

	public void MinigameStart_Chess() {
		CenterStatusPrint("%s", Text->stringTable[1021] + " Chess");
		minigameButtonsContainer.SetActive(false);
		minigameViewContainer.SetActive(true);
		minigameChessSpaceContainer.SetActive(true);
		minigameSpace.SetActive(true);
		minigameCamera.SetActive(true);
	}
}
