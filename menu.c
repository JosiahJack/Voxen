// menu.c - Main Menu
MainMenu Sys_Menu;

void SetIndex(int index) {
    Sys_Menu.currentIndex = index;
//     for (int i=0;i<Sys_Menu.currentMenuItemsLength;i++) { // TODO
//         menuItems[i].SendMessage("DeHighlight",SendMessageOptions.DontRequireReceiver);
//         menuItems[i].SendMessage("InputFieldCancelFocus",SendMessageOptions.DontRequireReceiver);
//     }
// 
//     if (menuItems[currentIndex] != null) menuItems[currentIndex].SendMessage("Highlight",SendMessageOptions.DontRequireReceiver);
//     if (menuItems[currentIndex] != null) menuItems[currentIndex].SendMessage("InputFieldFocus",SendMessageOptions.DontRequireReceiver);
//     if (menuSubItems[currentIndex] != null) menuSubItems[currentIndex].SendMessage("Highlight",SendMessageOptions.DontRequireReceiver);
}

void MenuInit() {
    Sys_Menu.currentIndex = 0;
    Sys_Menu.currentSaveSlot = -1;
    Sys_Menu.presetQuestionValue = -1;
    SetIndex(0);
    Sys_Menu.axisUp = Sys_Menu.axisDn = Sys_Menu.inCutscene = Sys_Menu.returnToPause = Sys_Menu.typingSaveGame = false;
    Config.SetVolume();
    GoToFrontPage();
    Sys_Global.introNotPlayed = true;
//     string basePath = Utils.GetAppropriateDataPath(); TODO
//     string indn = Utils.SafePathCombine(basePath,"introdone.dat");
//     if (System.IO.File.Exists(indn)) {
//         IntroVideo.SetActive(false);
//         ClearVideoRT();
//         IntroVideoContainer.SetActive(false);
//         BackGroundMusic.clip = Music.a.titleMusic;
//         BackGroundMusic.Play();
//     } else {
//         System.IO.File.Create(indn);
//         PlayIntro();
//     }
    Sys_Global.introNotPlayed = false; // TODO temp for testing
    if (Sys_Global.introNotPlayed) {
        inCutscene = false;
        vidFinished = Time.time + 117.5f;
        vidStartTime = Time.time;

        // Setup text.
        introVideoText1.text = Sys_Text.stringTable[613];
        introVideoText2.text = Sys_Text.stringTable[614];
        introVideoText3.text = Sys_Text.stringTable[615];
        introVideoText4.text = Sys_Text.stringTable[616];
        introVideoText5.text = Sys_Text.stringTable[617];
        introVideoText6.text = Sys_Text.stringTable[618];
        introVideoText7.text = Sys_Text.stringTable[619];
        introVideoText8.text = Sys_Text.stringTable[620];
        introVideoText9.text = Sys_Text.stringTable[621];
        introVideoText10.text = Sys_Text.stringTable[622];
        introVideoText11.text = Sys_Text.stringTable[623];
        introVideoText12.text = Sys_Text.stringTable[624];
        introVideoText13.text = Sys_Text.stringTable[625];
        introVideoText14.text = Sys_Text.stringTable[626];
        introVideoText15.text = Sys_Text.stringTable[627];
        Utils.Activate(introVideoTextGO1);
        Utils.Deactivate(introVideoTextGO2);
        Utils.Deactivate(introVideoTextGO3);
        Utils.Deactivate(introVideoTextGO4);
        Utils.Deactivate(introVideoTextGO5);
        Utils.Deactivate(introVideoTextGO6);
        Utils.Deactivate(introVideoTextGO7);
        Utils.Deactivate(introVideoTextGO8);
        Utils.Deactivate(introVideoTextGO9);
        Utils.Deactivate(introVideoTextGO10);
        Utils.Deactivate(introVideoTextGO11);
        Utils.Deactivate(introVideoTextGO12);
        Utils.Deactivate(introVideoTextGO13);
        Utils.Deactivate(introVideoTextGO14);
        Utils.Deactivate(introVideoTextGO15);
    }
}

void ShiftMenuItem (bool isDownKey) {
//     if (menuItems[currentIndex] != null) { TODO
//         menuItems[currentIndex].SendMessage("DeHighlight",SendMessageOptions.DontRequireReceiver);
//         menuItems[currentIndex].SendMessage("InputFieldCancelFocus",SendMessageOptions.DontRequireReceiver);
//     }
//     if (menuSubItems[currentIndex] != null) menuSubItems[currentIndex].SendMessage("DeHighlight",SendMessageOptions.DontRequireReceiver);

    if (isDownKey) {
        Sys_Menu.currentIndex++;
        if (Sys_Menu.currentIndex >= Sys_Menu.currentMenuItemsLength) Sys_Menu.currentIndex = 0; // Wrap around :)
    } else {
        Sys_Menu.currentIndex--;
        if (Sys_Menu.currentIndex < 0) Sys_Menu.currentIndex = (Sys_Menu.currentMenuItemsLength - 1); // Wrap around :)
    }

//     if (menuItems[currentIndex] != null) menuItems[currentIndex].SendMessage("Highlight",SendMessageOptions.DontRequireReceiver); TODO
//     if (menuItems[currentIndex] != null) menuItems[currentIndex].SendMessage("InputFieldFocus",SendMessageOptions.DontRequireReceiver);
//     if (menuSubItems[currentIndex] != null) menuSubItems[currentIndex].SendMessage("Highlight",SendMessageOptions.DontRequireReceiver);
}

void MenuInput() {
    if (Sys_Input.keyStates[GLFW_KEY_ENTER].pressed || Sys_Input.keyStates[GLFW_KEY_KP_ENTER].pressed || Sys_Input.keyStates[GLFW_JOYSTICK_1].pressed) {
//         if (menuItems[currentIndex].GetComponent<Button>() != null) { TODO
//             menuItems[currentIndex].GetComponent<Button>().onClick.Invoke();
//         } else {
//             if (menuItems[currentIndex].GetComponent<StartMenuNameAKeyEnter>() != null) menuItems[currentIndex].GetComponent<StartMenuNameAKeyEnter>().ClickViaKeyboard();
//             if (menuItems[currentIndex].GetComponent<StartMenuDifficultyController>() != null) menuItems[currentIndex].GetComponent<StartMenuDifficultyController>().ClickViaKeyboard();
//         }
        return;
    }

//     if (Sys_Input.keyStates[GLFW_GAMEPAD_AXIS_LEFT_Y] >= 0) axisUp = false; // TODO
    if (Input.GetKeyUp(KeyCode.UpArrow) || Input.GetKeyUp(KeyCode.LeftArrow) || ((Input.GetAxisRaw("JoyAxis8") < 0 || Input.GetAxisRaw("JoyAxis2") < 0) && !Sys_Menu.axisUp)) {
        Sys_Menu.axisUp = true;
        ShiftMenuItem(false);
    }

    if (Input.GetAxisRaw("JoyAxis8") <= 0) axisDn = false;
    if (Input.GetKeyUp(KeyCode.DownArrow) || Input.GetKeyUp(KeyCode.RightArrow) || ((Input.GetAxisRaw("JoyAxis8") > 0 || Input.GetAxisRaw("JoyAxis2") > 0) && !Sys_Menu.axisDn)) {
        Sys_Menu.axisDn = true;
        ShiftMenuItem(true);
    }
}

void LeaveDeathCutscene() {
    inCutscene = false;
    DeathVideo.SetActive(false);
    DeathVideoContainer.SetActive(false);
    ClearVideoRT();
    BackGroundMusic.clip = Music.a.titleMusic;
    BackGroundMusic.Play();
}

void LeaveIntroCutscene() {
    inCutscene = false;
    IntroVideo.SetActive(false);
    ClearVideoRT();
    IntroVideoContainer.SetActive(false);
    Const.a.WriteDatForIntroPlayed(false);
    BackGroundMusic.clip = Music.a.titleMusic;
    BackGroundMusic.Play();
}


void MenuButton_StartGame (bool isNew) {
    Sys_Global.difficultyCombat = combat.difficultySetting;
    Sys_Global.difficultyMission = mission.difficultySetting;
    Sys_Global.difficultyPuzzle = puzzle.difficultySetting;
    Sys_Global.difficultyCyber = cyber.difficultySetting;
    if (Sys_Global.difficultyMission < 3) {
        MissionTimer.a.text.text = System.String.Empty;
        MissionTimer.a.timerTypeText.text = System.String.Empty;
        Sys_UI.overallMissionTimerT.SetActive(false);
        Sys_UI.overallMissionTimer.SetActive(false);
    } else {
        Sys_UI.overallMissionTimerT.SetActive(true);
        Sys_UI.overallMissionTimer.SetActive(true);
    }
    
    if (isNew) {
        string pname = newgamePage.GetComponentInChildren<InputField>(true).text;
        if (StringIsEmpty(pname)) pname = "Hacker";
        Const.a.playerName = pname;
        Const.a.NewGame();
    }

    MouseCursor.a.mainCamera.enabled = true;
    this.flag_set(&SELF.entflags, ENTFLAG_ACTIVE, false);
}

void GoToOptionsSubmenu(bool accessedFromPause) {
    ResetPages();
    if (accessedFromPause) IntroVideoContainer.SetActive(false);
    optionsPage.SetActive(true);
    SetOptionsTabGraphics();
    currentPage = Pages.op;
    returnToPause = accessedFromPause;
    RenderConfigView();
    EventSystem.current.SetSelectedGameObject(null);
}

void SetOptionsTabGraphics() {
    GraphicsTab.SetActive(true);
    InputTab.SetActive(false);
    AudioTab.SetActive(false);
    GraphicsTabButtonImage.overrideSprite = OptionsTabHilited;
    InputTabButtonImage.overrideSprite = OptionsTabDehilited;
    AudioTabButtonImage.overrideSprite = OptionsTabDehilited;
    UpdateConfigTabTextColor();
}

void SetOptionsTabInput() {
    GraphicsTab.SetActive(false);
    InputTab.SetActive(true);
    AudioTab.SetActive(false);
    GraphicsTabButtonImage.overrideSprite = OptionsTabDehilited;
    InputTabButtonImage.overrideSprite = OptionsTabHilited;
    AudioTabButtonImage.overrideSprite = OptionsTabDehilited;
    UpdateConfigTabTextColor();
}

void SetOptionsTabAudio() {
    GraphicsTab.SetActive(false);
    InputTab.SetActive(false);
    AudioTab.SetActive(true);
    GraphicsTabButtonImage.overrideSprite = OptionsTabDehilited;
    InputTabButtonImage.overrideSprite = OptionsTabDehilited;
    AudioTabButtonImage.overrideSprite = OptionsTabHilited;
    UpdateConfigTabTextColor();
}

void UpdateConfigTabTextColor() {
    if (GraphicsTab.activeInHierarchy) {
        GraphicsTabButtonText.color = Const.a.ssYellowText;
        InputTabButtonText.color = Const.a.ssGreenText;
        AudioTabButtonText.color = Const.a.ssGreenText;
    } else if (InputTab.activeInHierarchy) {
        GraphicsTabButtonText.color = Const.a.ssGreenText;
        InputTabButtonText.color = Const.a.ssYellowText;
        AudioTabButtonText.color = Const.a.ssGreenText;
    } else if (AudioTab.activeInHierarchy) {
        GraphicsTabButtonText.color = Const.a.ssGreenText;
        InputTabButtonText.color = Const.a.ssGreenText;
        AudioTabButtonText.color = Const.a.ssYellowText;
    }
}

void RenderConfigView() {
//     DynamicCulling.a.CullCore(); TODO
//     configCamera.Render();
}

void GoToNewGameSubmenu() {
    ResetPages();
    newgamePage.SetActive(true);
    newgameInputText.ActivateInputField();
    currentPage = Pages.np;
}

char* GetSaveName(int index) {
    string savName = "sav" + index.ToString() + ".txt";
    string basePath = Utils.GetAppropriateDataPath();
    string sP = Utils.SafePathCombine(basePath,savName);
    string retval = "! unknown !";
    Utils.ConfirmExistsMakeIfNot(basePath,savName);
    StreamReader sf = new StreamReader(sP);
    if (sf == null) {
        DualLog("GetSaveName error! sf null");
        return retval;
    }

    using (sf) {
        retval = sf.ReadLine();
        if (retval == null) {
            DualLog("GetSaveName error! retval null");
            return "! unknown !"; // just in case
        }

        sf.Close();
    }

    DualLog("GetSaveName retval: %s",retval);
    return retval;
}

void GoToLoadGameSubmenu (bool accessedFromPause) {
    ResetPages();
    loadPage.SetActive(true);
    currentPage = Pages.lp;
    returnToPause = accessedFromPause;
    for (int i=0;i<8;i++) {
        loadButtonText[i].text = GetSaveName(i);
    }	
    EventSystem.current.SetSelectedGameObject(null);
}

void GoToSaveGameSubmenu (bool accessedFromPause) {
    ResetPages();
    savePage.SetActive(true);
    currentPage = Pages.sv;
    returnToPause = accessedFromPause;
    for (int i=0;i<8;i++) {
        saveButtonText[i].text = GetSaveName(i);
    }	
    EventSystem.current.SetSelectedGameObject(null);
}

void SaveGameEntry (int index) {
    currentSaveSlot = index;
    typingSaveGame = true;
    saveNameInput[index].SetActive(true);
    saveNamePlaceholder[index].SetActive(true);
    saveButtonText[index].text = System.String.Empty;
    tempSaveNameHolder = saveButtonText[index].text;
    saveButtonText[index].text = System.String.Empty;
    saveNameInputField[index].ActivateInputField();
}

void SaveQuickSaveButton () {
    SaveGame(7,"quicksave");
}

void SaveGame(int index,string savename) {
    Const.a.StartSave(index,savename);
    CenterStatusPrint("%s", Sys_Text.stringTable[28] + index.ToString() + "!",Const.a.player1);
    PauseScript.a.EnablePauseUI();
    MouseCursor.a.mainCamera.enabled = true;
    this.flag_set(&SELF.entflags, ENTFLAG_ACTIVE, false);
}

void LoadGame(int index) {
    if (loadButtonText[index].text == "- unused -"
        || loadButtonText[index].text == "- unused quicksave -") {
        CenterStatusPrint("%s", Sys_Text.stringTable[1022]); // "No data to load."
    } else Const.a.Load(index,false);
}

void GoBack () {
    EventSystem.current.SetSelectedGameObject(null);
    if (typingSaveGame) {
        saveNameInput[currentSaveSlot].SetActive(false);
        saveNamePlaceholder[currentSaveSlot].SetActive(false);
        typingSaveGame = false;
        loadButtonText[currentSaveSlot].text = tempSaveNameHolder;
        currentSaveSlot = -1;
        return;
    }

    if (returnToPause) {
        PauseScript.a.ExitSaveDialog();
        ResetPages();
        returnToPause = false;
        this.flag_set(&SELF.entflags, ENTFLAG_ACTIVE, false);
        return;
    }

    if (currentPage == Pages.sv) {
        GoToFrontPage();
        return;
    }

    // Go Back to front page
    if (currentPage == Pages.sp || currentPage == Pages.mp || currentPage == Pages.op) {
        GoToFrontPage();
        return;
    }

    // Go Back to singlepayer page
    if (currentPage == Pages.np || currentPage == Pages.lp || currentPage == Pages.cd) {
        if (currentPage == Pages.cd) {
            BackGroundMusic.clip = Music.a.titleMusic;
            BackGroundMusic.Play();
        }
        GoToSingleplayerSubmenu();
        return;
    }
}

void PlayDeathVideo() {
    DeathVideoContainer.SetActive(true);
    DeathVideo.SetActive(true);
    string basePath = Utils.GetAppropriateDataPath();
    string fileName = "death.webm";
    Utils.ConfirmExistsMakeIfNot(basePath,fileName);
    string urlPath = Utils.SafePathCombine(basePath,fileName);
    deathPlayer.url = urlPath;
    deathPlayer.Play();
    deathPlayer.SetDirectAudioMute(0,true);
    deathVideoText1.text = Sys_Text.stringTable[628];
    deathVideoText2.text = Sys_Text.stringTable[629];
    Utils.Activate(deathVideoTextGO1);
    Utils.Deactivate(deathVideoTextGO2);
    gameObject.SetActive(true);
    BackGroundMusic.clip = Music.a.levelMusicDeath;
    if (dataFound) BackGroundMusic.Play();
    vidFinished = Time.time + 16.8f;
    vidStartTime = Time.time;
}

void PlayIntro() {
    Const.a.WriteDatForIntroPlayed(false);
    IntroVideoContainer.SetActive(true);
    IntroVideo.SetActive(true);
    string basePath = Utils.GetAppropriateDataPath();
    string fileName = "intro.webm";
    Utils.ConfirmExistsMakeIfNot(basePath,fileName);
    string urlPath = Utils.SafePathCombine(basePath,fileName);
    introPlayer.url = urlPath;
    introPlayer.Play();
    if (!dataFound) introPlayer.SetDirectAudioMute(0,true);
    else introPlayer.SetDirectAudioMute(0,false);

    inCutscene = true;
    BackGroundMusic.Stop();
    vidFinished = Time.time + 117.5f;
    vidStartTime = Time.time;

    // Setup text.
    introVideoText1.text = Sys_Text.stringTable[613];
    introVideoText2.text = Sys_Text.stringTable[614];
    introVideoText3.text = Sys_Text.stringTable[615];
    introVideoText4.text = Sys_Text.stringTable[616];
    introVideoText5.text = Sys_Text.stringTable[617];
    introVideoText6.text = Sys_Text.stringTable[618];
    introVideoText7.text = Sys_Text.stringTable[619];
    introVideoText8.text = Sys_Text.stringTable[620];
    introVideoText9.text = Sys_Text.stringTable[621];
    introVideoText10.text = Sys_Text.stringTable[622];
    introVideoText11.text = Sys_Text.stringTable[623];
    introVideoText12.text = Sys_Text.stringTable[624];
    introVideoText13.text = Sys_Text.stringTable[625];
    introVideoText14.text = Sys_Text.stringTable[626];
    introVideoText15.text = Sys_Text.stringTable[627];
    Utils.Activate(introVideoTextGO1);
    Utils.Deactivate(introVideoTextGO2);
    Utils.Deactivate(introVideoTextGO3);
    Utils.Deactivate(introVideoTextGO4);
    Utils.Deactivate(introVideoTextGO5);
    Utils.Deactivate(introVideoTextGO6);
    Utils.Deactivate(introVideoTextGO7);
    Utils.Deactivate(introVideoTextGO8);
    Utils.Deactivate(introVideoTextGO9);
    Utils.Deactivate(introVideoTextGO10);
    Utils.Deactivate(introVideoTextGO11);
    Utils.Deactivate(introVideoTextGO12);
    Utils.Deactivate(introVideoTextGO13);
    Utils.Deactivate(introVideoTextGO14);
    Utils.Deactivate(introVideoTextGO15);
}

void PlayCredits () {
    ResetPages();
    creditsPage.SetActive(true);
    currentPage = Pages.cd;
    if (Const.a.DynamicMusic) {
        BackGroundMusic.clip = Music.a.creditsMusic;
    } else {
        BackGroundMusic.clip = Music.a.levelMusicLooped;
    }

    if (gameObject.activeSelf && dataFound) BackGroundMusic.Play();
}

void SetConfigPreset(int index) {
    presetQuestionValue = index;
    if (presetQuestionValue == 1)  presetQuestionText.text = Sys_Text.stringTable[924]; // CHANGE ALL KEYS TO LEGACY PRESET?
    else presetQuestionText.text = Sys_Text.stringTable[923]; // RESET ALL KEYS TO DEFAULT?

    PresetConfirmDialog.SetActive(true);
}

void CancelPresetSet() {
    presetQuestionValue = -1;
    PresetConfirmDialog.SetActive(false);
}

void ApplyPreset() {
    switch (presetQuestionValue) {
        case 0: // Default
            Const.a.InputCodeSettings[0] = 22; // Forward = w
            Const.a.InputCodeSettings[1] = 0; // Strafe Left = a
            Const.a.InputCodeSettings[2] = 18; // Backpedal = s
            Const.a.InputCodeSettings[3] = 3; // Strafe Right = d
            Const.a.InputCodeSettings[4] = 87; // Jump = space
            Const.a.InputCodeSettings[5] = 2; // Crouch = c
            Const.a.InputCodeSettings[6] = 23; // Prone = x
            Const.a.InputCodeSettings[7] = 16; // Lean Left = q
            Const.a.InputCodeSettings[8] = 4; // Lean Right = e
            Const.a.InputCodeSettings[9] = 46; // Sprint = left shift
            Const.a.InputCodeSettings[10] = 139; // Toggle Sprint = capslock
            Const.a.InputCodeSettings[11] = 38; // Turn Left = left
            Const.a.InputCodeSettings[12] = 39; // Turn Right = right
            Const.a.InputCodeSettings[13] = 36; // Look Up = up
            Const.a.InputCodeSettings[14] = 37; // Look Down = down
            Const.a.InputCodeSettings[15] = 20; // Recent Log = u
            Const.a.InputCodeSettings[16] = 26; // Biomonitor = 1
            Const.a.InputCodeSettings[17] = 27; // Sensaround = 2
            Const.a.InputCodeSettings[18] = 28; // Lantern = 3
            Const.a.InputCodeSettings[19] = 29; // Shield = 4
            Const.a.InputCodeSettings[20] = 30; // Infrared = 5
            Const.a.InputCodeSettings[21] = 31; // Email = 6
            Const.a.InputCodeSettings[22] = 32; // Booster = 7
            Const.a.InputCodeSettings[23] = 33; // Jumpjets = 8
            Const.a.InputCodeSettings[24] = 53; // Attack = mouse 0
            Const.a.InputCodeSettings[25] = 54; // Use = mouse 1
            Const.a.InputCodeSettings[26] = 86; // Menu/Back = escape
            Const.a.InputCodeSettings[27] = 84; // Toggle Mode = tab
            Const.a.InputCodeSettings[28] = 17; // Reload = r
            Const.a.InputCodeSettings[29] = 153; // Weapon + = mwheel up
            Const.a.InputCodeSettings[30] = 154; // Weapon - = mwheel dn
            Const.a.InputCodeSettings[31] = 6; // Grenade = g
            Const.a.InputCodeSettings[32] = 19; // Grenade + = t
            Const.a.InputCodeSettings[33] = 1; // Grenade - = b
            Const.a.InputCodeSettings[34] = 21; // Ammo Type = v
            Const.a.InputCodeSettings[35] = 109; // Unused
            Const.a.InputCodeSettings[36] = 9; // Patch Use = j
            Const.a.InputCodeSettings[37] = 8; // Patch + = i
            Const.a.InputCodeSettings[38] = 133; // Patch - = ,
            Const.a.InputCodeSettings[39] = 12; // Full Map = m
            Const.a.NoShootMode = false;
            Const.a.InputQuickReloadWeapons = true;
            Const.a.InputQuickItemPickup = false;
            break;
        case 1: // Legacy SS1
            Const.a.InputCodeSettings[0] = 18; // Forward = s
            Const.a.InputCodeSettings[1] = 25; // Strafe Left = z
            Const.a.InputCodeSettings[2] = 23; // Backpedal = x
            Const.a.InputCodeSettings[3] = 2; // Strafe Right = c
            Const.a.InputCodeSettings[4] = 87; // Jump = space
            Const.a.InputCodeSettings[5] = 6; // Crouch = g
            Const.a.InputCodeSettings[6] = 1; // Prone = b
            Const.a.InputCodeSettings[7] = 16; // Lean Left = q
            Const.a.InputCodeSettings[8] = 4; // Lean Right = e
            Const.a.InputCodeSettings[9] = 46; // Sprint = left shift
            Const.a.InputCodeSettings[10] = 139; // Toggle Sprint = capslock
            Const.a.InputCodeSettings[11] = 0; // Turn Left = a
            Const.a.InputCodeSettings[12] = 3; // Turn Right = d
            Const.a.InputCodeSettings[13] = 17; // Look Up = r
            Const.a.InputCodeSettings[14] = 21; // Look Down = v
            Const.a.InputCodeSettings[15] = 20; // Recent Log = p
            Const.a.InputCodeSettings[16] = 26; // Biomonitor = 1
            Const.a.InputCodeSettings[17] = 28; // Sensaround = 3
            Const.a.InputCodeSettings[18] = 29; // Lantern = 4
            Const.a.InputCodeSettings[19] = 30; // Shield = 5
            Const.a.InputCodeSettings[20] = 31; // Infrared = 6
            Const.a.InputCodeSettings[21] = 33; // Email = 8
            Const.a.InputCodeSettings[22] = 34; // Booster = 9
            Const.a.InputCodeSettings[23] = 35; // Jumpjets = 0
            Const.a.InputCodeSettings[24] = 54; // Use = mouse 1
            Const.a.InputCodeSettings[25] = 53; // Attack = mouse 0
            Const.a.InputCodeSettings[26] = 86; // Menu/Back = escape
            Const.a.InputCodeSettings[27] = 84; // Toggle Mode = tab
            Const.a.InputCodeSettings[28] = 19; // Reload = t
            Const.a.InputCodeSettings[29] = 153; // Weapon + = mwheel up
            Const.a.InputCodeSettings[30] = 154; // Weapon - = mwheel dn
            Const.a.InputCodeSettings[31] = 7; // Grenade = h
            Const.a.InputCodeSettings[32] = 24; // Grenade + = y
            Const.a.InputCodeSettings[33] = 13; // Grenade - = n
            Const.a.InputCodeSettings[34] = 10; // Ammo Type = k
            Const.a.InputCodeSettings[35] = 109; // Unused
            Const.a.InputCodeSettings[36] = 9; // Patch Use = j
            Const.a.InputCodeSettings[37] = 8; // Patch + = i
            Const.a.InputCodeSettings[38] = 133; // Patch - = ,
            Const.a.InputCodeSettings[39] = 12; // Full Map = m
            Const.a.NoShootMode = true;
            Const.a.InputQuickReloadWeapons = false;
            Const.a.InputQuickItemPickup = false;
            break;
    }
    presetQuestionValue = -1;	
    Config.WriteConfig(); // Save config.  Always set to autosave.
    for (int i=0;i<keybindButtons.Length;i++) {
        keybindButtons[i].UpdateText();
    }
    ctInvertUpDnLook.AlignWithConfigFile();
    ctInvertUpDnCyberLook.AlignWithConfigFile();
    ctInvertInventoryCyc.AlignWithConfigFile();
    ctQuickItemPickUp.AlignWithConfigFile();
    ctQuickReload.AlignWithConfigFile();
    ctNoShootMode.AlignWithConfigFile();
    CancelPresetSet();
}

void MenuUpdate() {
    if (Input.GetMouseButtonUp(0)
        || Input.GetMouseButtonUp(1)
        || Input.GetKeyDown(KeyCode.Escape)
        || Input.GetKeyDown(KeyCode.JoystickButton0)
        || Input.GetKeyDown(KeyCode.JoystickButton1)
        || Input.anyKey) {
        if ((inCutscene || IntroVideoContainer.activeSelf
            || DeathVideoContainer.activeSelf)
            && !CouldNotFindDialogue.activeSelf) {
            
            if (IntroVideoContainer.activeSelf && (Time.time - vidStartTime) > 1.5f) {
                LeaveIntroCutscene();
            } else if (DeathVideoContainer.activeSelf && (Time.time - vidStartTime) > 1.5f) {
                LeaveDeathCutscene();
            }

            return;
        }
    }

    if (Input.GetKeyDown(KeyCode.Escape)
        || Input.GetKeyDown(KeyCode.JoystickButton1)) { // Escape/back
                                                        // button listener
        if (savePage.activeInHierarchy && !newgamePage.activeInHierarchy) {
            if (currentSaveSlot > 0
                && currentSaveSlot < saveNameInputField.Length) {
                InputField infld = saveNameInputField[currentSaveSlot];
                if (infld != null) infld.DeactivateInputField();
            }
            currentSaveSlot = -1;
            typingSaveGame = false;
            returnToPause = true;
        }

        GoBack();
        return;
    }

    if (IntroVideoContainer.activeSelf) {
        if (vidFinished > 0 && (Time.time - vidStartTime) > 6.7f
            && introVideoTextGO1.activeSelf
            && !introVideoTextGO2.activeSelf) {

            Utils.Deactivate(introVideoTextGO1);
            Utils.Activate(introVideoTextGO2);
        }

        if (vidFinished > 0 && (Time.time - vidStartTime) > 9.9f
            && introVideoTextGO2.activeSelf
            && !introVideoTextGO3.activeSelf) {

            Utils.Deactivate(introVideoTextGO2);
            Utils.Activate(introVideoTextGO3);
        }

        if (vidFinished > 0 && (Time.time - vidStartTime) > 19.2f
            && introVideoTextGO3.activeSelf
            && !introVideoTextGO4.activeSelf) {

            Utils.Deactivate(introVideoTextGO3);
            Utils.Activate(introVideoTextGO4);
        }

        if (vidFinished > 0 && (Time.time - vidStartTime) > 30.7f
            && introVideoTextGO4.activeSelf
            && !introVideoTextGO5.activeSelf) {

            Utils.Deactivate(introVideoTextGO4);
            Utils.Activate(introVideoTextGO5);
        }

        if (vidFinished > 0 && (Time.time - vidStartTime) > 37.9f
            &&  introVideoTextGO5.activeSelf
            && !introVideoTextGO6.activeSelf) {

            Utils.Deactivate(introVideoTextGO5);
            Utils.Activate(  introVideoTextGO6);
        }

        if (vidFinished > 0 && (Time.time - vidStartTime) > 43.7f
            &&  introVideoTextGO6.activeSelf
            && !introVideoTextGO7.activeSelf) {

            Utils.Deactivate(introVideoTextGO6);
            Utils.Activate(  introVideoTextGO7);
        }

        if (vidFinished > 0 && (Time.time - vidStartTime) > 48.1f
            &&  introVideoTextGO7.activeSelf
            && !introVideoTextGO8.activeSelf) {

            Utils.Deactivate(introVideoTextGO7);
            Utils.Activate(  introVideoTextGO8);
        }

        if (vidFinished > 0 && (Time.time - vidStartTime) > 59.3f
            &&  introVideoTextGO8.activeSelf
            && !introVideoTextGO9.activeSelf) {

            Utils.Deactivate(introVideoTextGO8);
            Utils.Activate(  introVideoTextGO9);
        }

        if (vidFinished > 0 && (Time.time - vidStartTime) > 69.1f
            &&  introVideoTextGO9.activeSelf
            && !introVideoTextGO10.activeSelf) {

            Utils.Deactivate(introVideoTextGO9);
            Utils.Activate(  introVideoTextGO10);
        }

        if (vidFinished > 0 && (Time.time - vidStartTime) > 74.5f
            &&  introVideoTextGO10.activeSelf
            && !introVideoTextGO11.activeSelf) {

            Utils.Deactivate(introVideoTextGO10);
            Utils.Activate(  introVideoTextGO11);
        }

        if (vidFinished > 0 && (Time.time - vidStartTime) > 81.2f
            &&  introVideoTextGO11.activeSelf
            && !introVideoTextGO12.activeSelf) {

            Utils.Deactivate(introVideoTextGO11);
            Utils.Activate(  introVideoTextGO12);
        }

        if (vidFinished > 0 && (Time.time - vidStartTime) > 89.2f
            &&  introVideoTextGO12.activeSelf
            && !introVideoTextGO13.activeSelf) {

            Utils.Deactivate(introVideoTextGO12);
            Utils.Activate(  introVideoTextGO13);
        }

        if (vidFinished > 0 && (Time.time - vidStartTime) > 98.4f
            &&  introVideoTextGO13.activeSelf
            && !introVideoTextGO14.activeSelf) {

            Utils.Deactivate(introVideoTextGO13);
            Utils.Activate(  introVideoTextGO14);
        }

        if (vidFinished > 0 && (Time.time - vidStartTime) > 105.0f
            &&  introVideoTextGO14.activeSelf
            && !introVideoTextGO15.activeSelf) {

            Utils.Deactivate(introVideoTextGO14);
            Utils.Activate(  introVideoTextGO15);
        }


        if (vidFinished < Time.time && IntroVideoContainer.activeSelf
            && vidFinished > 0) {

            vidFinished = 0;
            Utils.Deactivate(IntroVideoContainer);
            Utils.Deactivate(introVideoTextGO1);
            Utils.Deactivate(introVideoTextGO2);
            Utils.Deactivate(introVideoTextGO3);
            Utils.Deactivate(introVideoTextGO4);
            Utils.Deactivate(introVideoTextGO5);
            Utils.Deactivate(introVideoTextGO6);
            Utils.Deactivate(introVideoTextGO7);
            Utils.Deactivate(introVideoTextGO8);
            Utils.Deactivate(introVideoTextGO9);
            Utils.Deactivate(introVideoTextGO10);
            Utils.Deactivate(introVideoTextGO11);
            Utils.Deactivate(introVideoTextGO12);
            Utils.Deactivate(introVideoTextGO13);
            Utils.Deactivate(introVideoTextGO14);
            Utils.Deactivate(introVideoTextGO15);
            ClearVideoRT();
        }
    } else if (DeathVideoContainer.activeSelf) {
        if (vidFinished > 0 && (Time.time - vidStartTime) > 5.53f
            && deathVideoTextGO1.activeSelf
            && !deathVideoTextGO2.activeSelf) {

            Utils.Deactivate(deathVideoTextGO1);
            Utils.Activate(deathVideoTextGO2);
            ClearVideoRT();
        }

        if (vidFinished < Time.time && DeathVideoContainer.activeSelf
            && vidFinished > 0) {

            vidFinished = 0;
            Utils.Deactivate(DeathVideoContainer);
            Utils.Deactivate(deathVideoTextGO1);
            Utils.Deactivate(deathVideoTextGO2);
            ClearVideoRT();
            BackGroundMusic.clip = Music.a.titleMusic;
            BackGroundMusic.Play();
        }
    } else {
        if (!BackGroundMusic.isPlaying
            && !saltTheFries.activeInHierarchy
            && gameObject.activeSelf) {
            BackGroundMusic.clip = Music.a.titleMusic;
            BackGroundMusic.Play();
        }
    }

    // Qmaster's cheat
    if ((   (Input.GetKey(KeyCode.LeftAlt) && Input.GetKeyDown(KeyCode.P))
            || (Input.GetKeyDown(KeyCode.LeftAlt) && Input.GetKey(KeyCode.P)))
        && !CouldNotFindDialogue.activeInHierarchy) {
        if (StringIsEmpty(Const.a.playerName)) {
            Const.a.playerName = "Qmaster";
        }

        StartGame(true);
        return;
    }

    if (typingSaveGame && (Input.GetKeyUp(KeyCode.Return)
                            || Input.GetKeyUp(KeyCode.KeypadEnter)
                            || Input.GetKeyDown(KeyCode.JoystickButton0))
        && savePage.activeInHierarchy
        && !newgamePage.activeInHierarchy) {
        if (currentSaveSlot < 0) return;

        InputField infldTemp = saveNameInputField[currentSaveSlot];
        string sname = infldTemp.text;
        if (!string.IsNullOrEmpty(sname)) {
            if (sname == "- unused -") sname = "Savegame: - unused - "
                                                + currentSaveSlot.ToString();
            SaveGame(currentSaveSlot,sname);
            saveNameInput[currentSaveSlot].SetActive(false);
            saveNamePlaceholder[currentSaveSlot].SetActive(false);
            saveButtonText[currentSaveSlot].text = sname;
            typingSaveGame = false;
            currentSaveSlot = -1;
        } else {
            GoBack();
            return;
        }
    }

    UpdateConfigTabTextColor();
}
