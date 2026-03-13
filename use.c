// use.c - Use Functions (e.g. door open, switch activate, charge station draw energy, etc.)
void ChargStationUse (UseData ud) {
    if (GetCurrentLevelSecurity() > minSecurityLevel) { Sys_UI.BlockedBySecurity(SELF.position); return; }
    
    if (SELF.tickTime < Sys_Global.pauseRelativeTime) {
        if (PlayerEnergy.a.energy >= PlayerEnergy.a.maxenergy) {
            CenterStatusPrint("%s",Sys_Text.stringTable[303]);
            return;
        } else {
            GiveEnergy(amount, EnergyType_ChargeStation);
//             Sys_UI.energySurge.SetActive(true); TODO
        }

        if (damageOnUse > 0f) {
            DamageData dd;
            dd.damage = vmin(damageOnUse,Eng_Global->instances[PLAYER1].health - 1);  // Don't ever kill the player from this, way too cheap.

            // No impact force here, it's a zap.  Ouch, it zapped me...that
            // really hurt Chargie, that hurt my finger, owhow, OW! ow,
            // hahahow ow! OWW!  Chargie zapped my finger (it helps if you
            // use a British accent and refer to Charlie Bit My Finger).
            if (dd.damage > 0) Eng_Global->instances[PLAYER1].TakeDamage(dd);
        }

        CenterStatusPrint("%s",Sys_Text.stringTable[0]);
        if (SELF.entflags & ENTFLAG_REQUIRE_RESET) SELF.tickTime = Sys_Global.pauseRelativeTime + resetTime;
        UseTargets(gameObject,ud,target);
    } else {
        CenterStatusPrint("%s",Sys_Text.stringTable[1]);
    }
}

void FuncWallUse (UseData ud) {
    if (SELF.currentState == FuncStates_Start || SELF.currentState == FuncStates_MovingStart || SELF.currentState == FuncStates_AjarMovingTarget) SELF.currentState = FuncStates_MovingTarget;
    else SELF.currentState = FuncStates_MovingStart;
    
    startTime = Sys_Global.pauseRelativeTime + 10.0f;
    play_wav(sounds[76]); // doorwall_move
    flag_set(&SELF.entflags,ENTFLAG_STOPSOUND_PLAYED,false);
}

void ActivatePatch(int index) { // Expects the usableItems index
    bool depleted = false;
    switch (index) {
    case 14:
        // Berserk Patch
        inventoryPlayer1.patchCounts[2]--;
        if (inventoryPlayer1.patchCounts[2] <= 0) depleted = true;
        if (!(Eng_Global->instances[PLAYER1].patchActive & PATCH_BERSERK)) Eng_Global->instances[PLAYER1].patchActive |= PATCH_BERSERK;
        berserkFinishedTime = Sys_Global.pauseRelativeTime + BERSERK_TIME;
        float berserkIncrementTime = BERSERK_TIME / 5.0f;
        if (berserkIncrementFinishedTime > Sys_Global.pauseRelativeTime) berserkIncrementFinishedTime += berserkIncrementTime; // berserk effect stacks
        else                                                             berserkIncrementFinishedTime = Sys_Global.pauseRelativeTime + berserkIncrementTime;
        break;
    case 15:
        // Detox Patch
        inventoryPlayer1.patchCounts[6]--;
        if (inventoryPlayer1.patchCounts[6] <= 0) depleted = true;
        DisableAllPatches(); // remove all other effects, even medipatch
        Eng_Global->instances[PLAYER1].patchActive = PATCH_DETOX; // overwrite all other active patches
        detoxFinishedTime = Sys_Global.pauseRelativeTime + DETOX_TIME; // detox doesn't stack, it cancels itself lol
        break;
    case 16:
        // Genius Patch
        inventoryPlayer1.patchCounts[5]--;
        if (inventoryPlayer1.patchCounts[5] <= 0) depleted = true;
        if (!(Eng_Global->instances[PLAYER1].patchActive & PATCH_GENIUS)) Eng_Global->instances[PLAYER1].patchActive |= PATCH_GENIUS;
        if (geniusFinishedTime > Sys_Global.pauseRelativeTime) {
            geniusFinishedTime += GENIUS_TIME; // genius effect stacks
        } else {
            geniusFinishedTime = Sys_Global.pauseRelativeTime + GENIUS_TIME;
        }
        break;
    case 17:
        // Medi Patch
        if (hm.health >=255) {
            CenterStatusPrint("%s", Sys_Text.stringTable[304],MouseLookScript.a.player);
            return;
        }
        inventoryPlayer1.patchCounts[3]--;
        if (inventoryPlayer1.patchCounts[3] <= 0) depleted = true;
        if (!(Eng_Global->instances[PLAYER1].patchActive & PATCH_MEDI)) Eng_Global->instances[PLAYER1].patchActive |= PATCH_MEDI;
        PlayerHealth.a.mediPatchPulseCount = 0;
        if (mediFinishedTime > Sys_Global.pauseRelativeTime) {
            mediFinishedTime += MEDI_TIME; // medipatch effect stacks
        } else {
            mediFinishedTime = Sys_Global.pauseRelativeTime + MEDI_TIME;
        }
        break;
    case 18:
        // Reflex Patch
        inventoryPlayer1.patchCounts[4]--;
        if (inventoryPlayer1.patchCounts[4] <= 0) depleted = true;
        Time.timeScale = REFLEX_TIME_SCALE;
        if (!(Eng_Global->instances[PLAYER1].patchActive & PATCH_REFLEX)) Eng_Global->instances[PLAYER1].patchActive |= PATCH_REFLEX;
        if (reflexFinishedTime > Time.realtimeSinceStartup ) {
            reflexFinishedTime += REFLEX_TIME; // reflex effect stacks
        } else {
            reflexFinishedTime = Time.realtimeSinceStartup + REFLEX_TIME;
        }
        break;
    case 19:
        // Sight Patch
        inventoryPlayer1.patchCounts[1]--;
        if (inventoryPlayer1.patchCounts[1] <= 0) depleted = true;
        sightLight.enabled = true; // enable vision enhancement
        sightSideEffectFinishedTime = -1.0f;  // reset side effect timer from previous patch
        sightDimming.enabled = false; // deactivate side effect from previous patch
        if (!(Eng_Global->instances[PLAYER1].patchActive & PATCH_SIGHT)) Eng_Global->instances[PLAYER1].patchActive |= PATCH_SIGHT;
        if (sightFinishedTime > Sys_Global.pauseRelativeTime) sightFinishedTime += SIGHT_TIME; // sight effect stacks
        else                                                  sightFinishedTime = Sys_Global.pauseRelativeTime + SIGHT_TIME;
        break;
    case 20:
        // Staminup Patch
        inventoryPlayer1.patchCounts[0]--;
        if (inventoryPlayer1.patchCounts[0] <= 0) depleted = true;
        Eng_Global->instances[PLAYER1].staminupActive = true;
        if (!(Eng_Global->instances[PLAYER1].patchActive & PATCH_STAMINUP)) Eng_Global->instances[PLAYER1].patchActive |= PATCH_STAMINUP;
        if (staminupFinishedTime > Sys_Global.pauseRelativeTime) {
            staminupFinishedTime += STAMINUP_TIME; // staminup effect stacks
        } else {
            staminupFinishedTime = Sys_Global.pauseRelativeTime + STAMINUP_TIME;
        }

        break;
    }

    if (depleted) {
        inventoryPlayer1.PatchCycleDown(false);
        CenterStatusPrint("%s%s%s",Sys_Text.stringTable[590],Sys_Text.stringTable[index + 326],Sys_Text.stringTable[589]);
    } else {
        CenterStatusPrint("%s%s",Sys_Text.stringTable[index + 326],Sys_Text.stringTable[589]);
    }

    Utils.PlayUIOneShotSavable(89);
}

void ButtonSwitchUseTargets () { // TODO
//     UseData ud = new UseData();
//     UseTargets(gameObject,ud,target);
       bool active = Eng_Global->instances[i].entflags & ENTFLAG_ACTIVE;
       active = !active;
       flag_set(&Eng_Global->instances[i].entflags,ENTFLAG_ACTIVE,active);
       Eng_Global->instances[i].alternateOn = active;
//     if (Eng_Global->instances[i].entflags & ENTFLAG_CHANGE_TEX_ON_ACTIVE) {
//         if (Eng_Global->instances[i].entflags & ENTFLAG_BLINK_TEX_ON_ACTIVE) {
//             ToggleMaterial();
//             if (Eng_Global->instances[i].entflags & ENTFLAG_ACTIVE)
//                 Eng_Global->instances[i].tickFinished = Sys_Global.pauseRelativeTime + tickTime;
//         } else {
//             ToggleMaterial(); TODO
//         }
//     }
//     if (Eng_Global->instances[i].entflags & ENTFLAG_ANIMATED) {
//         if (active) { // UH OH TODO TODO Fix the overlap here with gameObject.active
//             anim.Play("Activating");
//         } else {
//             anim.Play("Deactivating");
//         }
//     }
}

void ButtonSwitchUse(uint16_t i, UseData ud) {
    if (Eng_Cheats->superoverride || Sys_Global.difficultyMission == 0) {
        Eng_Global->instances[i].locked = false; // SHODAN can go anywhere!  Full security override!
    } else if (GetCurrentLevelSecurity() > Eng_Global->instances[i].securityThreshhold) {
//         BlockedBySecurity(Eng_Global->instances[i].position); TODO
        return;
    }

    if (Eng_Global->instances[i].locked) {
        CenterStatusPrint("%s",Sys_Text.stringTable[Eng_Global->instances[i].lockedMessageLingdex]);
        if (Eng_Global->instances[i].SFXLockedIndex >= 0 && Eng_Global->instances[i].SFXLockedIndex < SOUNDS_COUNT) play_wav(sounds[SELF.SFXLockedIndex],1.0f,SELF.position,true);
        return;
    }

    // Set playerCamera to owner of the input (always should be the camera)
    Utils.PlayOneShotSavable(SFXSource,sounds[SFXIndex]);
    CenterStatusPrint("%s",Sys_Text.stringTable[messageIndex]);
    if (Eng_Global->instances[i].delay > 0.0f) Eng_Global->instances[i].delayFinished = Sys_Global.pauseRelativeTime + Eng_Global->instances[i].delay;
    else ButtonSwitchUseTargets();
}

// Check for envirosuit and apply reduction based on version
bool EnvirosuitApply() {
    radAdjust = 0.0f;
    if (!inventoryPlayer1.hasHardware[8]) return false;
    if (PlayerEnergy.a.energy <= 0) return false;

    float enerTake = 0.0f;
    float frac = 0.12f;
    float energCost = 0.11f;
    switch (inventoryPlayer1.hardwareVersion[8]) {
        case 1: frac = 0.17f; energCost = 0.25f; break;
        case 2: frac = 0.15f; energCost = 0.16f; break;
        case 3: frac = 0.12f; energCost = 0.11f; break;
    }

    radAdjust = radiated * frac;
    float diff = radiated - radAdjust;
    radiated = radAdjust; // After calculating difference.
    if (radiated < 0) radiated = 0;
    if (diff < 0) diff = 0; // Prevent underflow.
    enerTake = (energCost * diff);
    if (enerTake < 0) enerTake = 0;
    radAdjust = initialRadiation - radiated;
    if (radAdjust < 0) radAdjust = 0;
    DualLog("Taking energy for envirosuit: %f",(double)enerTake);

    // Suit absorbs some radiation, say it.
    // Envirosuit absorbed ##LBP, Radiation poisoning ##LBP
    Eng_Global->instances[PLAYER1].twm.SendWarning((Sys_Text.stringTable[280]
                                        + radAdjust.ToString()
                                        + Sys_Text.stringTable[281]
                                        + Sys_Text.stringTable[185]
                                        + radiated.ToString()
                                        + Sys_Text.stringTable[186]),
                                        0.1f,-2,HUDColor.Red,
                                        radiationAmountWarningID);

    TakeEnergy(enerTake);
    BiomonitorEnergyPulse(enerTake);
    return true;
}

void GiveRadiation(float rad) {
    if (Eng_Global->instances[PLAYER1].health <= 0.0f) return;

    if (Eng_Global->instances[PLAYER1].radiated < rad) Eng_Global->instances[PLAYER1].radiated = rad;
    else return;

//     EnvirosuitApply(); TODO
    Eng_Global->instances[PLAYER1].initialRadiation = radiated;
}

void DoorUse (UseData ud) {
    if (ud == null) return;
    if (ud.owner == null) return;
    if (GetCurrentLevelSecurity() > securityThreshhold) { Sys_UI.BlockedBySecurity(Eng_Global->instances[i].position); return; }

    if (Eng_Cheats->superoverride || Sys_Global.difficultyMission <= 0) { // SHODAN can go anywhere!  Full security override!
        locked = false;
        requiredAccessCard = AccessCardType_None;
        accessCardUsedByPlayer = true;
    }

    if (Sys_Global.difficultyMission <= 1) { requiredAccessCard = AccessCardType_None; accessCardUsedByPlayer = true; }
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
                UseTargets(gameObject,ud,target);
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
                Utils.PlayOneShotSavable(SFX,sounds[467],0.55f);
                if (QuestLogNotesManager.a != null) {
                    QuestLogNotesManager.a.NotifyLockedDoorAttempt(this);
                }
            }
        }
    } else {
        // Tell owner of the Use command that an access card is needed.
        CenterStatusPrint(requiredAccessCard.ToString() + Sys_Text.stringTable[2]);
        Utils.PlayOneShotSavable(SFX,sounds[466],0.7f);
    }
}

// Entry Positions:
// ======================
// 	L2A 157.1608 -15.53 47.331
// 	L2B 256.0416 -0.716 62.48789
// 	L5 126.43 29.56733 34.24
// 	L6 177.612 3.294942 108.7725
// 	L8 244.735 41.99257 -19.695
// 	L9 185.161 84.502 -46.04246
void CyberAccessUse (uint16_t activator, uint16_t cybAcc) {
    selfIdx = cybAcc;
    activatorIdx = activator;
    UseTargets(gameObject,ud,Eng_Global->instances[i].target);
    CenterStatusPrint("%s", Sys_Text.stringTable[441]); // Entering Cyberspace!
    Vector3 entryPosition = (Vector3){ 195.42000f, -13.44000f,  33.28000f};
    switch(LevelManager.a.currentLevel) {
        case 0: entryPosition = (Vector3){ 210.68340f,   2.81200f, -24.37800f}; break;
        case 1: entryPosition = (Vector3){ 195.42000f, -13.44000f,  33.28000f}; break;
        case 2: 
            if (ACTIVATOR.position.x < -26.0f ) {
                // Keycard room port at localPosition -34.53611 -27.76395 2.5696
                entryPosition = (Vector3){ 157.16080f, -15.53000f,  47.33100f};
            } else {
                // Library port
                entryPosition = (Vector3){ 256.04160f,  -0.71600f,  62.48789f};
            }
            
            break;
        case 5: entryPosition = (Vector3){ 126.43000f,  29.56733f,  34.24000f}; break;
        case 6: entryPosition = (Vector3){ 177.61200f,   3.29494f, 108.77250f}; break;
        case 8: entryPosition = (Vector3){ 244.73500f,  41.99257f, -19.69500f}; break;
        case 9: entryPosition = (Vector3){ 185.16100f,  84.50200f, -46.04246f}; break;
    }
    MouseLookScript.a.EnterCyberspace(entryPosition);
}
