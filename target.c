// target.c - Targetted Functions for I/O System activations
void ButtonSwitchTargetted(UseData ud) {
    BUttonSwitchUse(ud);
}

void DoorTargetted (uint16_t doorIdx, UseData ud) {
    if (Eng_Global->instances[doorIdx].locked) {
        Eng_Global->instances[doorIdx].locked = false;
//         NotifyDoorUnlock(this); // TODO
    }

    if (!Eng_Global->instances[doorIdx].targettingOnlyUnlocks) DoorUse(ud);
}

void FlipTrackSwitch(uint16_t i) { // Swap targets
    Eng_Global->instances[i].currenttarget = Eng_Global->instances[i].onSecond) ? Eng_Global->instances[i].target : Eng_Global->instances[i].target2;
    Eng_Global->instances[i].onSecond = !Eng_Global->instances[i].onSecond;
}

void LogicBranchRunTargets(UseData ud, uint16_t i) {
//     TargetIO tio = GetComponent<TargetIO>();
//     ud.SetBits(tio);
    UseTargets(ud,currenttarget);
    if (autoFlipOnTarget) FlipTrackSwitch();
}

void LogicBranchTargetted (UseData ud, uint16_t i) {
    if (!(Eng_Global->instances[i].entflags & ENTFLAG_ENABLED)) return;

    if (Eng_Global->instances[i].delay <=0.0f) {
        LogicBranchRunTargets(ud);
    } else {
        StartCoroutine(DelayedTarget(ud));
    }
}

void GameEnd() {
    DualLog("Game finished!");
    Eng_Global->gameFinished = true; // YAY WE DID IT!!!!
/*    PauseEnable(); // Pauses game, no more to do TODO
    NoSavePauseQuit(); // quit to and enable main menu (exits the game to menu)
    PlayCredits(); // */Play credits and set page in menu handler
}

void TriggerTargetted (UseData ud, uint16_t i) {
    if (Eng_Global->instances[i].ignoreSecondaryTriggers) recentMostActivator = ud.owner;
    //TriggerTripped (Collider col, bool initialEntry);
}

void LogicRelayRunTargets(UseData ud) {
    if (onceEver && alreadyDone) return;

    TargetIO tio = GetComponent<TargetIO>();
    ud.SetBits(tio);
    UseTargets(null,ud,target);
    if (onceEver) alreadyDone = true;
}

void LogicUpdate() {
    if (SELF.tickFinished >= Eng_Global->pauseRelativeTime) return;
    
    LogicRelayRunTargets(ud);
}

void TriggerCounterTargetted (uint16_t activator, uint16_t i) {
    counter++;
    if (counter == countToTrigger) {
        if (delay <=0) {
            Target (ud);
        } else {
            Eng_Global->instances[i].tickFinished = Eng_Global->pauseRelativeTime + Eng_Global->instances[i].delay;
            Eng_Global->instances[i].activator = activator;
        }

        //!dontReset == reset, bleh double negatives why'd I do that
        if (!dontReset) counter = 0;
    }
}

// Already checked that target matched targetname of this interaction.
void Targetted(uint16_t activator, uint16_t other) {
    activatorIdx = activator; // Thing doing the targetting.
    selfIdx = other; // Thing being targetted.
    if (SELF.index == 699) { // Whatever else happens, if a LogicRelay, keep the messages going
		if (SELF.delay <=0.1f) LogicRelayRunTargets(ud);
		else TriggerRelayDelayedTarget(ud);
    }

    if (Eng_Global->instances[i].index == 708) GameEnd(); // info_gameend
    if (!ud.branchFlipOnly && Eng_Global->instances[i].index == 700 && (Eng_Global->instances[i].entflags & ENTFLAG_ENABLED)) LogicBranchTargetted(ud);
    if (ud.branchFlip || ud.branchFlipOnly && Eng_Global->instances[i].index == 700) FlipTrackSwitch();
    if ((ACTIVATOR.ioflags & TARG_IOFLAGS_TRIPTRIGGER) && (Eng_Global->instances[i].index == 598 || Eng_Global->instances[i].index == 600)) TriggerTargetted(ud);
    if ((ACTIVATOR.ioflags & TARG_IOFLAGS_TRIPTRIGGER) && Eng_Global->instances[i].index == 594) TriggerCounterTargetted(ud);
    if (ud.lockCodeToScreenMaterialChanger && Eng_Global->currentLevel == 1) flag_set(&Eng_Global->instances[WORLD].ioflags,QUESTBIT_LEV1_CODE_LOCKED,true);
    if (ud.lockCodeToScreenMaterialChanger && Eng_Global->currentLevel == 2) flag_set(&Eng_Global->instances[WORLD].ioflags,QUESTBIT_LEV1_CODE_LOCKED,true);
    if (ud.lockCodeToScreenMaterialChanger && Eng_Global->currentLevel == 3) flag_set(&Eng_Global->instances[WORLD].ioflags,QUESTBIT_LEV1_CODE_LOCKED,true);
    if (ud.lockCodeToScreenMaterialChanger && Eng_Global->currentLevel == 4) flag_set(&Eng_Global->instances[WORLD].ioflags,QUESTBIT_LEV1_CODE_LOCKED,true);
    if (ud.lockCodeToScreenMaterialChanger && Eng_Global->currentLevel == 5) flag_set(&Eng_Global->instances[WORLD].ioflags,QUESTBIT_LEV1_CODE_LOCKED,true);
    if (ud.lockCodeToScreenMaterialChanger && Eng_Global->currentLevel == 6) flag_set(&Eng_Global->instances[WORLD].ioflags,QUESTBIT_LEV1_CODE_LOCKED,true);

    if ((ACTIVATOR.ioflags & TARG_IOFLAGS_DOORUNLOCK)) {
        Door dr = GetComponent<Door>();
        if (dr != null) {
            dr.Unlock();
            dr.accessCardUsedByPlayer = true;
        }
    } // Unlock before open or toggle
    
    if (ACTIVATOR.ioflags & TARG_IOFLAGS_DOOROPEN) {
        Door dr = GetComponent<Door>();
        if (dr != null) {
            dr.ForceOpen();
        }
    }
    
    if (ACTIVATOR.ioflags & TARG_IOFLAGS_DOOROPENIFUNLOCKED) {
        Door dr = GetComponent<Door>();
        if (dr != null) {
            if (!dr.locked
                && (dr.requiredAccessCard == AccessCardType_None
                    || dr.accessCardUsedByPlayer
                    || Eng_Global->inventoryPlayer1.HasAccessCard(dr.requiredAccessCard))) {
                
                dr.ForceOpen();
            }
        }
    }
    
    if (ACTIVATOR.ioflags & TARG_IOFLAGS_DOOR_TOGGLE) {
// 			UnityEngine.DualLog("Attempting to toggle door's open/closed state on " + gameObject.name);
        Door dr = GetComponent<Door>();
        if (dr != null) {
            DoorState drprev = dr.doorOpen;
            dr.DoorActuate();
// 				if (dr.doorOpen != drprev) UnityEngine.DualLog("Successfully to toggled door's open/closed state on " + gameObject.name);
        }
    }
    
    if ((ACTIVATOR.ioflags & TARG_IOFLAGS_DOORCLOSE)) {
        Door dr = GetComponent<Door>();
        if (dr != null) dr.ForceClose();
    }
    
    if ((ACTIVATOR.ioflags & TARG_IOFLAGS_DOORLOCK)) { // Lock after forcing door into a position.
        Door dr = GetComponent<Door>();
        if (dr != null) dr.Lock();
    }
    


    if (ud.doorAccessCardOverrideToggle) {
        Door dr = GetComponent<Door>();
        if (dr != null) {
            dr.accessCardUsedByPlayer = !dr.accessCardUsedByPlayer;
        }
    }

    if ((ACTIVATOR.ioflags & TARG_IOFLAGS_SWITCHTRIGGER)) {
        ButtonSwitch bs = GetComponent<ButtonSwitch>();
        if (bs != null) bs.Targetted(ud);
    }

    if (SELF.index == 546 && (ACTIVATOR.ioflags & TARG_IOFLAGS_CHGSTAT_RECHARGE)) SELF.tickTime = 0.0f;

    if ((ACTIVATOR.ioflags & TARG_IOFLAGS_ENEMY_ALERT)) {
        AIController aic = GetComponent<AIController>();
        if (aic != null) aic.Alert(ud);
    }

    if ((ACTIVATOR.ioflags & TARG_IOFLAGS_FBRIDGE_ACTIVATE)) ForceBridgeActivate(false);
    if ((ACTIVATOR.ioflags & TARG_IOFLAGS_FBRIDGE_DEACTIVATE)) ForceBridgeDeactivate(false);
    if ((ACTIVATOR.ioflags & TARG_IOFLAGS_FBRIDGE_TOGGLE)) ForceBridgeToggle();

    if ((ACTIVATOR.ioflags & TARG_IOFLAGS_GRAVLIFT_TOGGLE)) {
        GravityLift gl = GetComponent<GravityLift>();
        if (gl != null) gl.Toggle();
    }

    if ((ACTIVATOR.ioflags & TARG_IOFLAGS_TEXTURE_CHG_TOGGLE)) {
        TextureChanger tch = GetComponent<TextureChanger>();
        if (tch != null) tch.Toggle();
    }

    if ((ACTIVATOR.ioflags & TARG_IOFLAGS_LIGHT_ON)) {
        LightAnimation lam = GetComponent<LightAnimation>();
        if (lam != null) lam.TurnOn();
    }

    if ((ACTIVATOR.ioflags & TARG_IOFLAGS_LIGHT_OFF)) {
        LightAnimation lam = GetComponent<LightAnimation>();
        if (lam != null) lam.TurnOff();
    }

    if ((ACTIVATOR.ioflags & TARG_IOFLAGS_LIGHT_TOGGLE)) {
        LightAnimation lam = GetComponent<LightAnimation>();
        if (lam != null) lam.Toggle();
    }

    if ((ACTIVATOR.ioflags & TARG_IOFLAGS_FUNCWALL_MOVE)) {
        //DualLog("FuncWall move activated!");
        FuncWall fw = GetComponent<FuncWall>();
        if (fw != null) fw.Targetted(ud);
    }

    if ((ACTIVATOR.ioflags & TARG_IOFLAGS_MISSION_BIT_ON)) {
        QuestBitRelay qbr = GetComponent<QuestBitRelay>();
        if (qbr != null) qbr.EnableBits();
    }

    if (ud.missionBitOff) {
        QuestBitRelay qbr = GetComponent<QuestBitRelay>();
        if (qbr != null) qbr.DisableBits();
    }

    if (ud.missionBitToggle) {
        QuestBitRelay qbr = GetComponent<QuestBitRelay>();
        if (qbr != null) qbr.ToggleBits();
    }

    if (ud.sendEmail) {
        //DualLog("sendEmail was true for Targetted() with targetname: " + targetname);
        Email msg = GetComponent<Email>();
        if (msg != null) {
            //DualLog("sendEmail was true and msg was found for Targetted() with targetname: " + targetname);
            msg.Targetted();
        }
    }

    if (ud.switchLockToggle) {
        ButtonSwitch btsw = GetComponent<ButtonSwitch>();
        if (btsw != null) btsw.ToggleLocked();
    }

    if (ud.unlockSwitch) {
        ButtonSwitch btsw = GetComponent<ButtonSwitch>();
        if (btsw != null) btsw.locked = false;
    }

    if (ud.spawnerActivate) {
        SpawnManager spwnmgr = GetComponent<SpawnManager>();
        if (spwnmgr != null) spwnmgr.Activate(false);
    }

    if (ud.spawnerActivateAlerted) {
        SpawnManager spwnmgr = GetComponent<SpawnManager>();
        if (spwnmgr != null) spwnmgr.Activate(true);
    }

    if (ud.cyborgConversionToggle) {
        LevelManager.a.CyborgConversionToggleForCurrentLevel();
        CyborgConversionToggle cctog = GetComponent<CyborgConversionToggle>();
        if (cctog != null) cctog.PlayVoxMessage();
    }

    if (ud.toggleRadiationTrigger) {
        Radiation rad = GetComponent<Radiation>();
        if (rad != null) rad.enabled = !rad.enabled;
    }

    if (ud.toggleRelayEnabled) {
        LogicRelay logrel = GetComponent<LogicRelay>();
        if (logrel != null) logrel.relayEnabled = !logrel.relayEnabled;
    }

    if (ud.togglePuzzlePanelLocked) {
        PuzzleGridPuzzle pgp = GetComponent<PuzzleGridPuzzle>();
        if (pgp != null) pgp.locked = !pgp.locked;

        PuzzleWirePuzzle pwp = GetComponent<PuzzleWirePuzzle>();
        if (pwp != null) pwp.locked = !pwp.locked;
    }

    if (ud.testQuestBitIsOn) {
        QuestBitRelay qbr = GetComponent<QuestBitRelay>();
        if (qbr != null) qbr.TestBits(true,ud,this);
    }

    if (ud.testQuestBitIsOff) {
        QuestBitRelay qbr = GetComponent<QuestBitRelay>();
        if (qbr != null) qbr.TestBits(false,ud,this);
    }

    if (ud.playSoundOnce) {
        PlaySoundTriggered pst = GetComponent<PlaySoundTriggered>();
        if (pst != null) pst.PlaySoundEffect();
    }

    if (ud.stopSound) {
        PlaySoundTriggered pst = GetComponent<PlaySoundTriggered>();
        if (pst != null) pst.StopSoundEffect();
    }

    if (ud.sendSprintMessage) {
        TriggeredSprintMessage tsm = GetComponent<TriggeredSprintMessage>();
        if (tsm != null) CenterStatusPrint(tsm.messageToDisplay);
    }

    if (ud.radiationTreatment) {
        if (PlayerReferenceManager.a != null) {
            PlayerReferenceManager.a.playerRadiationTreatmentFlash.SetActive(true);
            PlayerHealth.a.radiated = 0;
        }
    }

    if (ud.startFlashingMaterials) {
        MaterialFlash mflash = GetComponent<MaterialFlash>();
        if (mflash != null) mflash.StartFlashing();
    }

    if (ud.stopFlashingMaterials) {
        MaterialFlash mflash = GetComponent<MaterialFlash>();
        if (mflash != null) mflash.StopFlashing();
    }

    if (ud.unlockElevatorPad) {
        KeypadElevator kelv = GetComponent<KeypadElevator>();
        if (kelv != null) kelv.locked = false;
    }

    if (ud.unlockKeycodePad) {
        KeypadKeycode keyk = GetComponent<KeypadKeycode>();
        if (keyk != null) keyk.locked = false;
    }

    if (ud.unlockPuzzlePad) {
        PuzzleGridPuzzle pgp = GetComponent<PuzzleGridPuzzle>();
        if (pgp != null) pgp.locked = false;

        PuzzleWirePuzzle pwp = GetComponent<PuzzleWirePuzzle>();
        if (pwp != null) pwp.locked = false;
    }

    if (ud.screenShake) {
        EffectScreenShake efsh = GetComponent<EffectScreenShake>();
        if (efsh != null) efsh.Shake();
    }

    if (ud.awakeSleepingEnemy) {
        if (ConstIndexIsNPC(Eng_Global->instances[i].index)) {
            Eng_Global->instances[i].asleep = false;
            uint16_t cables = Eng_Global->instances[i].sleepingCables;
            if (cables != UINT16_MAX && cables != PLAYER1 && cables != PLAYER2) DeleteInstance(cables);
    }

    if (ud.lockElevatorPad) {
        KeypadElevator kelv = GetComponent<KeypadElevator>();
        if (kelv != null) kelv.locked = true;
    }
}

void UseTargets(UseData ud, const char* targetname) {
    if (StringIsEmpty(targetname)) return; // This is fine, some triggers we just want to play the trigger's SFX and do nothing else.

    float numtargetsfound = 0;
    bool succeeded = false;
    for (int i=START_INDEX_LEVEL_INSTANCES;i<loadedInstances;i++) { // Find each gameobject with matching targetname in the register, then call Use for each.
        if (!StringEquals(Eng_Global->instances[i].targetname,targetname)) continue;

        numtargetsfound++;
        DualLog("Running targets for %s (found %u so far)\n", targetname, numtargetsfound);
        if (ud.GOSetActive && !(Eng_Global->instances[i].entflags & ENTFLAG_ACTIVE)) flag_set(&Eng_Global->instances[i].entflags,ENTFLAG_ACTIVE,true); // Added activeSelf bit to keep from spamming SetActive when running targets through a trigger_multiple
        if (ud.GOSetDeactive && (Eng_Global->instances[i].entflags & ENTFLAG_ACTIVE)) flag_set(&Eng_Global->instances[i].entflags,ENTFLAG_ACTIVE,false); // Diddo for activeSelf to prevent spamming SetActive.
        if (ud.GOToggleActive) flag_set(&Eng_Global->instances[i].entflags,ENTFLAG_ACTIVE,!(Eng_Global->instances[i].entflags & ENTFLAG_ACTIVE)); // If I abuse this with a trigger_multiple someone should shoot me.
        Targetted(ud);
    }

    if (!numtargetsfound) DualLogWarning("Failed to find a matching targetname for %s", targetname);
}
