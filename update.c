// update.c - Entity Update logic for all entities thinking
#include "voxen.h"

void ButtonSwitchUpdate(void) {
    if ((SELF.delayFinished < Sys_Global.pauseRelativeTime) && SELF.delayFinished != 0) {
        SELF.delayFinished = 0;
//         UseTargets(); TODO
    }

    if (SELF.entflags & ENTFLAG_BLINK_TEX_ON_ACTIVE) {
        if (SELF.entflags & ENTFLAG_ACTIVE) {
            if (SELF.tickFinished < Sys_Global.pauseRelativeTime) {
//                 if (alternateOn) SetMaterialToAlternate(); TODO
//                 else SetMaterialToNormal();
                
                SELF.alternateOn = !SELF.alternateOn;
                SELF.tickFinished = Sys_Global.pauseRelativeTime + SELF.tickTime;
            }
        }
    }
}

void CameraViewUpdate() {
    uint16_t instCellIdx = PosGetCellCoords(SELF.position.x, SELF.position.z);
    if (!(gridCellStates[instCellIdx] & CELL_VISIBLE)) return;

    if (SELF.tickFinished < Sys_Global.current_time) {
        SELF.tickFinished = Sys_Global.current_time + SELF.tickTime;
//         cam.Render(); TODO
    }
}

void CodeScreenUpdate() {
    if (SELF.tickFinished > Sys_Global.pauseRelativeTime) return;
    
    SELF.tickFinished = Sys_Global.pauseRelativeTime + 0.3f;
    uint8_t matIndex = 0;
    switch (level) {
        case 1: matIndex = (Eng_Global->instances[WORLD].ioflags & QUESTBIT_LEV1_CODE_LOCKED) ? Eng_Global->instances[WORLD].lev1SecCode : (uint8_t)clamp(random_range(0.0f,10.0f),0.0f,9.0f); break;
        case 2: matIndex = (Eng_Global->instances[WORLD].ioflags & QUESTBIT_LEV2_CODE_LOCKED) ? Eng_Global->instances[WORLD].lev2SecCode : (uint8_t)clamp(random_range(0.0f,10.0f),0.0f,9.0f); break;
        case 3: matIndex = (Eng_Global->instances[WORLD].ioflags & QUESTBIT_LEV3_CODE_LOCKED) ? Eng_Global->instances[WORLD].lev3SecCode : (uint8_t)clamp(random_range(0.0f,10.0f),0.0f,9.0f); break;
        case 4: matIndex = (Eng_Global->instances[WORLD].ioflags & QUESTBIT_LEV4_CODE_LOCKED) ? Eng_Global->instances[WORLD].lev4SecCode : (uint8_t)clamp(random_range(0.0f,10.0f),0.0f,9.0f); break;
        case 5: matIndex = (Eng_Global->instances[WORLD].ioflags & QUESTBIT_LEV5_CODE_LOCKED) ? Eng_Global->instances[WORLD].lev5SecCode : (uint8_t)clamp(random_range(0.0f,10.0f),0.0f,9.0f); break;
        case 6: matIndex = (Eng_Global->instances[WORLD].ioflags & QUESTBIT_LEV6_CODE_LOCKED) ? Eng_Global->instances[WORLD].lev6SecCode : (uint8_t)clamp(random_range(0.0f,10.0f),0.0f,9.0f); break;
    }
    
    SELF.texture = min(768 + matIndex,777);
}

void FuncWallMoveToPosition(Vector3 goalPosition, FuncStates newState) {
//     rbody.WakeUp(); TODO
    float dist = speed * Sys_Global.deltaTime;
    tempVec = (Eng_Global->instances[i].position - goalPosition).normalized; // Relative
    tempVec = (tempVec * dist * -1) + Eng_Global->instances[i].position; // Absolute
    rbody.MovePosition(tempVec);
    float distanceLeft; = distance_vector3(Eng_Global->instances[i].position, goalPosition);
    float distTotal = vabs(targetPositionY);
    percentMoved = (distTotal - distanceLeft) / distTotal;
    if (float.IsNaN(percentMoved)) percentMoved = 0f;
    if (percentMoved > 1.0f) percentMoved = 1.0f;
    if (percentMoved < 0f) percentMoved = 0f;
    if (distanceLeft <= 0.04f || startTime < Sys_Global.pauseRelativeTime) {
        currentState = newState;
        if (SFXSource != null) {
            SFXSource.Stop ();
            SFXSource.loop = false;
            if (!(SELF.entflags & ENTFLAG_STOPSOUND_PLAYED) {
                Utils.PlayOneShotSavable(SFXSource,sounds[77]);
                SELF.entflags |= ENTFLAG_STOPSOUND_PLAYED;
            }
        }
    }
}

void FuncWallUpdate() {
    switch (currentState) {
        case FuncStates.Start:
            Eng_Global->instances[i].position = startPosition;
            if (Eng_Global->instances[i].velocity.sqrMagnitude > 0) Eng_Global->instances[i].velocity = (Vector3){0.0f,0.0f,0.0f};
            break;
        case FuncStates.Target:
            Eng_Global->instances[i].position = Eng_Global->instances[i].targetPosition;
            if (rbody.velocity.sqrMagnitude > 0) rbody.velocity = (Vector3){0.0f,0.0f,0.0f};
            break;
        case FuncStates.MovingStart:
            FuncWallMoveToPosition(startPosition, FuncStates.Start);
            break;
        case FuncStates.MovingTarget:
            FuncWallMoveToPosition(Eng_Global->instances[i].targetPosition, FuncStates.Target);
            break;
    }
}

void TargetIdentifierSenseTargets() {
    int lev = Sys_Global.currentLevel;    
    for (int i=0;i<loadedInstances;i++) {
        if (!ConstIndexIsNPC(Eng_Global->instances[i])) continue; // Only get NPCs.
        if (Eng_Global->instances[i].health <= 0.0f) continue; // It's dead, ignore.
        if (Eng_Global->instances[i].entflags & ENTFLAG_TARGID_ATTACHED) continue; // Already has a Target ID on it.

        float far = distance_vector3(Eng_Global->instances[i].position, Eng_Global->instances[i].position);
        if (far > TargetID.GetTargetIDSensingRange(false)) continue;
        
        CreateTargetIDInstance(-1.0f,i,-1.0f);
    }
}

void DeactivateHardwareOnEnergyDepleted() {
    flag_set(&inventoryPlayer1.hardwareIsActive, HW_SNS, false);
    flag_set(&inventoryPlayer1.hardwareIsActive, HW_BIO, false);
    flag_set(&inventoryPlayer1.hardwareIsActive, HW_SHD, false);
    flag_set(&inventoryPlayer1.hardwareIsActive, HW_LAN, false);
    flag_set(&inventoryPlayer1.hardwareIsActive, HW_BST, false);
    flag_set(&inventoryPlayer1.hardwareIsActive, HW_INF, false);
    flag_set(&inventoryPlayer1.hardwareIsActive, HW_TRC, false);
//     inventoryPlayer1.hardwareButtonManager.SensaroundOff(); //sensaround TODO
//     if (inventoryPlayer1.hardwareIsActive [HW_BIO] && inventoryPlayer1.hardwareVersionSetting[HW_BIO] == 0) inventoryPlayer1.hardwareButtonManager.BioOff(); // biomonitor, but only on v1, v2 doesn't use power
//     if (inventoryPlayer1.hardwareIsActive [HW_SHD]) inventoryPlayer1.hardwareButtonManager.ShieldOffWithEffects(); // shield
//     if (inventoryPlayer1.hardwareIsActive [HW_LAN]) inventoryPlayer1.hardwareButtonManager.LanternOff(); // lantern
//     if (inventoryPlayer1.hardwareIsActive [HW_BST]) inventoryPlayer1.hardwareButtonManager.BoosterOff(); // turbo motion booster
//     if (inventoryPlayer1.hardwareIsActive [HW_INF]) inventoryPlayer1.hardwareButtonManager.InfraredOff(); // infrared
}

void TakeEnergy(float drain) {
    float was = energy;
    if (energy <= 0.0f || drain <= 0.0f) return;
    if (inventoryPlayer1.redbull) return; // No energy drain!

    energy -= drain;
    if (energy <= 0.0f) {
        energy = 0.0f;
        Utils.PlayUIOneShotSavable(84); // energy_gone
        CenterStatusPrint(314); //Power supply exhausted.
        DeactivateHardwareOnEnergyDepleted();
    }
}

void GiveEnergy(float give, EnergyType type) {
    energy += give;
    if (energy > maxenergy) energy = maxenergy;
    if (type == EnergyType_Battery) Utils.PlayUIOneShotSavable(79); // batteryuse
    if (type == EnergyType_ChargeStation) Utils.PlayUIOneShotSavable(100); // chargingstation
}

void PlayerEnergyUpdate() {
    float drain = 1.0f;
    bool activeEnergyDrainers = false;
    if (Eng_Global->instances[PLAYER1].energyDrainTickFinished < Sys_Global.pauseRelativeTime) {
        uint16_t drainJPM = 0u;
        if (inventoryPlayer1.hardwareIsActive & HW_SNS) {
            switch (inventoryPlayer1.hardwareVersion[HW_SNS_IDX]) {
                case 0: drain = 0.01535f; drainJPM += 9; break; // takes about 300s to drain full energy
                case 1: drain = 0.03413f; drainJPM += 20; break; // takes about 300s to drain full energy
                case 2: drain = 0.02559f; drainJPM += 15; break; // takes about 240s to drain full energy
            }
            activeEnergyDrainers = true;
            TakeEnergy(drain);
        }

        if (inventoryPlayer1.hasHardware & HW_TID) TargetIdentifierSenseTargets();
        if (inventoryPlayer1.hardwareIsActive & HW_SHd) {
            switch (inventoryPlayer1.hardwareVersionSetting[HW_SHD_IDX]) {
                case 0: drain = 0.04096f; drainJPM += 24; break;
                case 1: drain = 0.10239f; drainJPM += 60; break;
                case 2: drain = 0.17919f; drainJPM += 105; break;
                case 3: drain = 0.05119f; drainJPM += 30; break;
            }
            activeEnergyDrainers = true;
            TakeEnergy(drain);
        }

       
        if (inventoryPlayer1.hardwareIsActive & HW_BIO) {
            switch (inventoryPlayer1.hardwareVersionSetting[HW_BIO]) {
                case 0: drain = 0.001706f; drainJPM += 1;  activeEnergyDrainers = true; break;
                case 1: drain = 0; break; // doesn't take energy
            }
            if (drain > 0) TakeEnergy(drain);
        }

        // 7 = Head Mounted Lantern
        if (inventoryPlayer1.hardwareIsActive & HW_LAN) {
            switch (inventoryPlayer1.hardwareVersionSetting[HW_LAN_IDX]) {
                case 0: tempF = 0.02559f; drainJPM += 15; break;// takes about 180s to drain full energy
                case 1: tempF = 0.04266f; drainJPM += 25; break; // takes about 120s to drain full energy
                case 2: tempF = 0.05119f; drainJPM += 30; break; // takes about 90s to drain full energy
            }
            activeEnergyDrainers = true;
            TakeEnergy(tempF);
        }

        // 8 Envirosuit - handled by HealthManager for radiation checks

        // 9 = Turbo Motion Booster - done in PlayerMovement since we only use energy on boost, no drain with skates
        if (inventoryPlayer1.hardwareIsActive & HW_BST) {
            switch (inventoryPlayer1.hardwareVersionSetting[HW_BST_IDX]) {
                case 0: tempF = 0f; break;
                case 1: tempF = 0.02f; drainJPM += 16; break; // takes about 120s to drain full energy
                case 2: tempF = 0.015f; drainJPM += 12; break; // takes about 90s to drain full energy
            }
            activeEnergyDrainers = true;
            if (tempF > 0) TakeEnergy(tempF);
        }

        // 10 Jump Jet Boots - done in PlayerMovement since we only drain while jumping

        // 11 Drain nightsight
        if (inventoryPlayer1.hardwareIsActive & HW_INF) {
            tempF = 0.08533f; drainJPM += 50; // takes about 120s to drain full energy
            activeEnergyDrainers = true;
            TakeEnergy(tempF);
        }
        
        Eng_Global->instances[PLAYER1].energyDrainTickFinished = Sys_Global.pauseRelativeTime + 0.1f;
    }
    
    // Turn everything off when we are out of energy
    if (activeEnergyDrainers && Eng_Global->instances[PLAYER1].energy <= 0.0f) {
        DeactivateHardwareOnEnergyDepleted();
        activeEnergyDrainers = false;
        Eng_Global->instances[PLAYER1].energy = 0.0f;
        Eng_Global->instances[PLAYER1].drainJPM = 0;
    } else {
        Eng_Global->instances[PLAYER1].drainJPM = drainJPM;
    }

    drainJPM = 50;
    if (drainJPM) RenderFormattedText(1202,138,TEXT_WHITE,FONT_NORMAL,1.0f,"%u J/min",drainJPM);
}

void HardwareButtonsUpdate() { // TODO
//     if (!hardwareButtonsContainer.activeInHierarchy) return; // In cyber space.
// 
//     hwb.ListenForHardwareHotkeys();
// 
//     // Check for and make the eReader button blink
//     if (hwb.buttons[5].gameObject.activeSelf) {
//         bool foundsome = false;
//         for (int i=0;i<hwb.ecbm.mmLBs.Length;i++) {
//             if (inventoryPlayer1.hasLog[hwb.ecbm.mmLBs[i].logReferenceIndex] && !inventoryPlayer1.readLog[hwb.ecbm.mmLBs[i].logReferenceIndex]) foundsome = true;
//         }
// 
//         if (foundsome) {
//             // You've got mail!
//             if (blinkFinished < Sys_Global.pauseRelativeTime) {
//                 blinkFinished = 1.0f + Sys_Global.pauseRelativeTime;
//                 inventoryPlayer1.hardwareIsActive[2] = !inventoryPlayer1.hardwareIsActive[2];
//                 if (inventoryPlayer1.hardwareIsActive[2]) {
//                     hwb.buttons[5].image.overrideSprite = hwb.buttonActive1[5];
//                 } else {
//                     hwb.buttons[5].image.overrideSprite = hwb.buttonDeactive[5];
//                 }
//             }
//             if (beepFinished < Sys_Global.pauseRelativeTime && inventoryPlayer1.beepDone) {
//                 beepFinished = 3.0f + Sys_Global.pauseRelativeTime;
//                 beepCount++;
//                 if (beepCount >= 3) { inventoryPlayer1.beepDone = false; beepCount = 0; } // Reset beeping, notification done.
//                 Utils.PlayOneShotSavable(hwb.SFX,sounds[83]); // emailalert, GO active handled by guard clause.
//             }
//         } else {
//             hwb.buttons[5].image.overrideSprite = hwb.buttonDeactive[5];
//         }
//     }
}

void LogReaderUpdate() {
    if (!Sys_UI.logActive) return;
    if (Sys_UI.logFinished >= Sys_Global.pauseRelativeTime) return;
    if (Sys_UI.logType == AudioLogType.Papers) return;
    if (Sys_UI.logType == AudioLogType.TextOnly) return;
    if (Sys_UI.logType == AudioLogType.Vmail) return;

    Sys_UI.logActive = false;
//     if (itemTabLH.eReaderSectionsContainer.activeInHierarchy) ReturnToLastTab(true); TODO
//     if (itemTabRH.eReaderSectionsContainer.activeInHierarchy) ReturnToLastTab(false);
//     if (DataReaderContentTab.activeInHierarchy) CenterTabButtonClickSilent(0,true);
}

void CenterTabBlink() {
    if (Sys_UI.centerTabsTickFinished >= Time.time) return;

    for (int i=0;i<4;i++) {
        if (Sys_UI.centerTabNotified[i]) ToggleHighlightOnCenterTabButton(i);
    }
    Sys_UI.centerTabsTickFinished = Time.time + 0.5f;
}

void MFDUpdate() {
    HardwareButtonsUpdate();
    LogReaderUpdate();
    CenterTabBlink();
    if (lastEnergy != PlayerEnergy.a.energy) DrawTicks(false);
    SELF.lastEnergy = SELF.energy;
    if (lastHealth != Eng_Global->instances[PLAYER1].health) DrawTicks(true);
    lastHealth = Eng_Global->instances[PLAYER1].health;
    WeaponButtonsManagerUpdate();
    UpdateAmmoAndLoadButtons();
    switch (inventoryPlayer1.weaponCurrent) {
        case 37: ShowEnergyItems(); break;
        case 40: ShowEnergyItems(); break;
        case 46: ShowEnergyItems(); break;
        case 50: ShowEnergyItems(); break;
        case 51: ShowEnergyItems(); break;
    }
    if (GetInput.a.WeaponCycUp()) WeaponCycleUp();
    if (GetInput.a.WeaponCycDown()) WeaponCycleDown();
    if (Input.GetKeyDown(KeyCode.F1)) leftTC.TabButtonAction(0);   // Weapon
    if (Input.GetKeyDown(KeyCode.F2)) leftTC.TabButtonAction(1);   // Item
    if (Input.GetKeyDown(KeyCode.F3)) leftTC.TabButtonAction(2);   // Automap
    // Target tab removed as unnecessary for Citadel.              // Target
    if (Input.GetKeyDown(KeyCode.F4)) leftTC.TabButtonAction(4);   // Data

    if (Input.GetKeyDown(KeyCode.F5)) rightTC.TabButtonAction(0);  // Weapon
    if (Input.GetKeyDown(KeyCode.F7)) rightTC.TabButtonAction(1);  // Item
    if (Input.GetKeyDown(KeyCode.F8)) rightTC.TabButtonAction(2);  // Automap
    // Target tab removed as unnecessary for Citadel.              // Target
    if (Input.GetKeyDown(KeyCode.F10)) rightTC.TabButtonAction(4); // Data

    if (Input.GetKeyDown(KeyCode.PageUp)) {
        if (DataReaderContentTab.activeInHierarchy) {
            ResetMultiMediaTabs();
            Utils.PlayUIOneShotSavable(97);
            CenterTabButtonClickSilent(0,true);
            if (inventoryPlayer1.hardwareIsActive[3]) {
                hwb.SensaroundOff();
                Utils.PlayUIOneShotSavable(82); // deactivate
            }
        } else {
            switch(curCenterTab) {
                case 0: CenterTabButtonAction(3); break;
                case 1: CenterTabButtonAction(0); break;
                case 2: CenterTabButtonAction(1); break;
                case 3: CenterTabButtonAction(2); break;
            }
        }
    }
    
    if (Input.GetKeyDown(KeyCode.PageDown)) {
        if (DataReaderContentTab.activeInHierarchy) {
            ResetMultiMediaTabs();
            Utils.PlayUIOneShotSavable(97);
            CenterTabButtonClickSilent(0,true);
            if (inventoryPlayer1.hardwareIsActive[3]) {
                hwb.SensaroundOff();
                Utils.PlayUIOneShotSavable(82); // deactivate
            }
        } else {
            switch(curCenterTab) {
                case 0: CenterTabButtonAction(1); break;
                case 1: CenterTabButtonAction(2); break;
                case 2: CenterTabButtonAction(3); break;
                case 3: CenterTabButtonAction(0); break;
            }
        }
    }

    // Handle severing connection with in use keypads, puzzles, etc. when player drifts too far away
    if (Sys_UI.usingObject) {
        if (distance_vector3(Eng_Global->instances[PLAYER1].position, objectInUsePos) > (Const.frobDistance + 0.16f)) {
            if (tetheredPGP != null) {
                ClosePuzzleGrid();
                tetheredPGP = null;
            }

            if (tetheredPWP != null) {
                ClosePuzzleWire();
                tetheredPWP = null;
            }

            if (tetheredKeypadElevator != null) {
                CloseElevatorPad();
                tetheredKeypadElevator = null;
            }

            if (tetheredKeypadKeycode != null) {
                CloseKeycodePad();
                tetheredKeypadKeycode = null;
            }

            if (tetheredSearchable != null) {
                CloseSearch();
                tetheredSearchable = null;
            }

            if (paperLogInUse) {
                ClosePaperLog();
                paperLogInUse = false;
            }
        }
    }

    // Update the weapon icon
//     wep16index = WeaponFire.Get16WeaponIndexFromConstIndex(inventoryPlayer1.weaponIndex); TODO
//     if (wep16index >=0 && wep16index < 16) {
//         if (leftTC.TabManager.WeaponTab.activeInHierarchy) {
//             iconLH.overrideSprite = wepIcons[wep16index];
//             if (inventoryPlayer1.numweapons <= 0 || inventoryPlayer1.weaponCurrentPending >= 0) {
//                 Utils.DisableImage(iconLH);
//             } else {
//                 Utils.EnableImage(iconLH);
//             }
//         }
// 
//         if (rightTC.TabManager.WeaponTab.activeInHierarchy) {
//             iconRH.overrideSprite = wepIcons[wep16index];
//             if (inventoryPlayer1.numweapons <= 0 || inventoryPlayer1.weaponCurrentPending >= 0) {
//                 Utils.DisableImage(iconRH);
//             } else {
//                 Utils.EnableImage(iconRH);
//             }
//         }
//     }
}

void PlayerRessurect() {
//     bool ressurected = LevelManager.a.RessurectPlayer(); TODO
//     if (!ressurected) DualLog("ERROR: failed to ressurect player!");
//     ressurections++;
//     hm.health = 211f;
//     Sys_UI.DrawTicks(true);
//     radiationArea = false;
//     radiated = 0;
//     playerDead = false;
//     PlayerPatch.a.DisableAllPatches();
//     Eng_Global->instances[PLAYER1].fatigue = 0f;
}

void PlayerDeathToMenu() {
    Sys_Global.mainMenu = true;
//     returnToPause = false;
//     MainMenuGoToFrontPage(); TODO
//     MainMenuPlayDeathVideo(); TODO
}

void PlayerDying() {
    SELF.playerHealthTimer += Time.deltaTime;
    SELF.radiationArea = false;
    SELF.radiated = 0.0f;
    flag_set(&SELF.entflags,ENTFLAG_MAKING_NOISE,false);
    if (playerHealthTimer >= resetAfterDeathTime) { SELF.health = 0.0f; SELF.playerDead = true; }
}

void PlayerDead() {
    if (SELF.heldObjectIndex != -1) { DropHeldItem(); ForceInventoryMode(); }	

//     hm.ClearOverlays(); TODO
//     if (LevelManager.a.ressurectionActive[Sys_Global.currentLevel]) PlayerRessurect(); // Ressurection TODO
//     else                                                            PlayerDeathToMenu(); // Game Over TODO
}

void PlayerHealthUpdate() {
    if (noiseFinished < Sys_Global.pauseRelativeTime) makingNoise = false;
    if (SELF.health <= 0.0f) {
        if (!SELF.playerDead) PlayerDying();
        else PlayerDead();
        return;
    }

    if (SELF.patchActive & PATCH_MEDI) {
        if (SELF.mediPatchPulseFinished == 0) SELF.mediPatchPulseCount = 0;
        if (SELF.mediPatchPulseFinished < Sys_Global.pauseRelativeTime) {
//             hm.HealingBed(8.0f,false); TODO
//             Sys_UI.DrawTicks(true); TODO
            SELF.mediPatchPulseFinished = Sys_Global.pauseRelativeTime + (0.5f + (mediPatchPulseCount * 0.5f));
            SELF.mediPatchPulseCount++;
        }
    } else {
        SELF.mediPatchPulseFinished = 0;
        SELF.mediPatchPulseCount = 0;
    }
    if (SELF.patchActive & PATCH_DETOX) radiated = 0.0f;
    if (radiated > 1.0f) {
        if (radiationArea) SELF.twm.SendWarning((Sys_Text.stringTable[184]),0.1f,-2,HUDColor.White,322); // radiationAreaWarningID
        if (!EnvirosuitApply()) SELF.twm.SendWarning((Sys_Text.stringTable[185] + radiated.ToString() +Sys_Text.stringTable[186]), 0.1f,-2,HUDColor.Red, 323); // radiationAmountWarningID Radiation poisoning ##LBP
        if (radFXFinished < Sys_Global.pauseRelativeTime) {
            radiationEffect.SetActive(true);
            float minT = 0.5f;
            if (radiated > 50.0f) minT = 0.25f;
            radFXFinished = Sys_Global.pauseRelativeTime + random_range(minT,1.0f);
        }
    } else {
        radiationArea = false;
        if (radiated < 0) radiated = 0;
    }

    if (radiationBleedOffFinished < Sys_Global.pauseRelativeTime) {
        if (!radiationArea) radiated -= 1.0f;  // Bleed off the radiation over time.
        if (radiated < 0) radiated = 0;
        radiationBleedOffFinished = Sys_Global.pauseRelativeTime + 1.8f;
        if (radiated > 0) {
            if (!hm.god) {
                hm.health -= radiated * 0.1f; // Apply health at rate of bleedoff time.
                Sys_UI.DrawTicks(true);
            }
            if (radSoundFinished < Sys_Global.pauseRelativeTime) {
                radSoundFinished = Sys_Global.pauseRelativeTime + random_range(1f,3f);
                Utils.PlayUIOneShotSavable(90);
            }
        }
    }
    if (lastHealth > hm.health) { // Did we lose health?
        if (painSoundFinished < Sys_Global.pauseRelativeTime && !(radSoundFinished < Sys_Global.pauseRelativeTime)) {
            painSoundFinished = Sys_Global.pauseRelativeTime + random_range(0.25f,3f); // Don't spam pain sounds
            Utils.PlayUIOneShotSavable(140);
            PlayerHealth.a.makingNoise = true;
        }
    }
    
    lastHealth = hm.health;
}

void ForceBridgeUpdate() {
    if (Sys_Global.gamePaused) return;
    if (Sys_Global.menuActive) return;
    if (SELF.tickFinished >= Sys_Global.pauseRelativeTime) return;

    SELF.tickFinished = Sys_Global.pauseRelativeTime + SELF.tickTime;
    if (SELF.entflags & ENTFLAG_ACTIVATED) {
        if (SELF.lerping) {
            float sx = SELF.scale.x;
            float sy = SELF.scale.y;
            float sz = SELF.scale.z;
            if (x) sx = lerp(SELF.scale.x,activatedScaleX,SELF.tickTime*2.0f);
            if (y) sy = lerp(SELF.scale.y,activatedScaleY,SELF.tickTime*2.0f);
            if (z) sz = lerp(SELF.scale.z,activatedScaleZ,SELF.tickTime*2.0f);
            SELF.scale = (Vector3){sx,sy,sz};
            if ((activatedScaleX - sx) < 0.08f && (activatedScaleY - sy) < 0.08f && (activatedScaleZ - sz) < 0.08f) {
                SELF.scale = (Vector3){activatedScaleX,activatedScaleY,activatedScaleZ);
                SELF.lerping = false;
            }
        } else {
            if (SELF.scale.x != activatedScaleX || SELF.scale.y != activatedScaleY || SELF.scale.z != activatedScaleZ) SELF.scale = (Vector3){activatedScaleX,activatedScaleY,activatedScaleZ};
        }
    } else {
        if (SELF.lerping) {
            // lerp scale down on deactivate
            float sx = SELF.scale.x;
            float sy = SELF.scale.y;
            float sz = SELF.scale.z;
            if (x) sx = lerp(SELF.scale.x,0.0f,SELF.tickTime*2.0f);
            if (y) sy = lerp(SELF.scale.y,0.0f,SELF.tickTime*2.0f);
            if (z) sz = lerp(SELF.scale.z,0.0f,SELF.tickTime*2.0f);
            SELF.scale = (Vector3){sx,sy,sz};
            if ((sx < 0.08f || sy < 0.08f || sz < 0.08f)) {
                flag_set(&SELF.entflags,ENTFLAG_ACTIVE,false);
                SELF.collider = COLLIDER_TYPE_NONE;
                SELF.lerping = false;
            }
        }
    }
}

// playerPizzazz on minigames - TODO
// void DriftUpUpdate() {
//     if (tickFinished >= Sys_Global.pauseRelativeTime) return;
// 
//     float delta = (1f / 60f);
//     tickFinished = Sys_Global.pauseRelativeTime + delta;
//     float drift = Eng_Global->instances[i].position.y;
//     drift += rate;
//     if (drift > endY) drift = endY;
//     Eng_Global->instances[i].position = (Vector3){Eng_Global->instances[i].position.x,drift,Eng_Global->instances[i].position.z};
//     drift = img.color.a;
//     drift -= fadeRate;
//     if (drift < endFade) drift = endFade;
//     img.color = new Color(img.color.r,img.color.g,img.color.b,drift);
// }

void UpdateWhileNotPaused(uint16_t i) {
    selfIdx = i;
    
    // Updates that occur regardless of active state
    
    if (!(SELF.entflags & ENTFLAG_ACTIVE)) return; // ACTIVE BARRIER =========================

    if (ConstIndexIsButtonSwitch(SELF.index)) ButtonSwitchUpdate();
    if (SELF.index == 755) CameraViewUpdate();
    if (SELF.index == 551) CodeScreenUpdate();
    if (SELF.index == 517) FuncWallUpdate();
    if (i == PLAYER1) {
        PlayerHealthUpdate();
        PlayerEnergyUpdate();
        BioMonitorUpdate();
        MFDUpdate();
    }
}
