// entlogic.c - Handles entity update logic and I/O between them
#include "voxen.h"

void TargetOnGatePassed(bool bitToCheck, bool passIfTrue, UseData ud, string targ, string targOnFalse) {
    if (passIfTrue) {
        if (!bitToCheck) { UseTargets(ud,tio,targ); return; }
    } else {
        if (bitToCheck) { UseTargets(ud,tio,targOnFalse); return; }
    }

    UseTargets(targ);
}

void EnableBits(uint16_t i) {
    Eng_Global->instances[WORLD].ioflags |= Eng_Global->instances[i].ioflags;
    
    if (Eng_Global->instances[i].ioflags & QUESTBIT_ROBOT_SPAWN_DEACTIVATED) DualLog("QUESTBIT_ROBOT_SPAWN_DEACTIVATED: 1");
    if (Eng_Global->instances[i].ioflags & QUESTBIT_ISOTOPE_INSTALLED) DualLog("QUESTBIT_ISOTOPE_INSTALLED: 1");
    if (Eng_Global->instances[i].ioflags & QUESTBIT_SHIELD_ACTIVATED) {
        DualLog("QUESTBIT_SHIELD_ACTIVATED: 1");
        QuestLogNotesManager.a.notes[8].SetActive(true);
        QuestLogNotesManager.a.checkBoxes[8].isOn = Const.a.questData.ShieldActivated;
        QuestLogNotesManager.a.labels[8].text = Eng_Text->stringTable[560];
    }
    
    if (Eng_Global->instances[i].ioflags & QUESTBIT_LASER_SAFETY_OVERRIDEN) {
        DualLog("QUESTBIT_LASER_SAFETY_OVERRIDEN: 1");
        QuestLogNotesManager.a.notes[7].SetActive(true);
        QuestLogNotesManager.a.checkBoxes[7].isOn = Const.a.questData.LaserSafetyOverriden;
        QuestLogNotesManager.a.labels[7].text = Eng_Text->stringTable[559];
    }
    
    if (Eng_Global->instances[i].ioflags & QUESTBIT_LASER_DESTROYED) {
        DualLog("QUESTBIT_LASER_DESTROYED: 1");
        if (AutoSplitterData.missionSplitID == 1) AutoSplitterData.missionSplitID++;
        QuestLogNotesManager.a.notes[9].SetActive(true);
        QuestLogNotesManager.a.checkBoxes[9].isOn = Const.a.questData.LaserDestroyed;
        QuestLogNotesManager.a.labels[9].text = Eng_Text->stringTable[561];
    }
    
    if (Eng_Global->instances[i].ioflags & QUESTBIT_BETA_GROVE_CYBER_UNLOCKED) {
        DualLog("QUESTBIT_BETA_GROVE_CYBER_UNLOCKED: 1");
        QuestLogNotesManager.a.notes[12].SetActive(true);
    }
    
    if (Eng_Global->instances[i].ioflags & QUESTBIT_GROVE_ALPHA_JETTISON_ENABLED) {
        DualLog("QUESTBIT_GROVE_ALPHA_JETTISON_ENABLED: 1");
        QuestLogNotesManager.a.notes[12].SetActive(true);
    }
    
    if (Eng_Global->instances[i].ioflags & QUESTBIT_GROVE_BETA_JETTISON_ENABLED) {
        DualLog("QUESTBIT_GROVE_BETA_JETTISON_ENABLED: 1");
        QuestLogNotesManager.a.notes[12].SetActive(true);
    }
    
    if (Eng_Global->instances[i].ioflags & QUESTBIT_GROVE_DELTA_JETTISON_ENABLED) {
        DualLog("QUESTBIT_GROVE_DELTA_JETTISON_ENABLED: 1");
        QuestLogNotesManager.a.notes[12].SetActive(true);
    }
    
    if (Eng_Global->instances[i].ioflags & QUESTBIT_MASTER_JETTISON_BROKEN) {
        DualLog("QUESTBIT_MASTER_JETTISON_BROKEN: 1");
        if (AutoSplitterData.missionSplitID == 2) AutoSplitterData.missionSplitID++;
        QuestLogNotesManager.a.notes[12].SetActive(true);
        QuestLogNotesManager.a.notes[11].SetActive(true);
        QuestLogNotesManager.a.labels[11].text = Eng_Text->stringTable[563]; // Set:Diagnose and repair broken relay
    }
    
    if (Eng_Global->instances[i].ioflags & QUESTBIT_RELAY_428_FIXED) {
        DualLog("QUESTBIT_RELAY_428_FIXED: 1");
        QuestLogNotesManager.a.notes[11].SetActive(true);
        QuestLogNotesManager.a.checkBoxes[11].isOn = Const.a.questData.Relay428Fixed;
        QuestLogNotesManager.a.labels[11].text = Eng_Text->stringTable[563]; // Set:Diagnose and repair broken relay
        QuestLogNotesManager.a.labels[11].text += Eng_Text->stringTable[564]; // Add:: 428.
    }
    
    if (Eng_Global->instances[i].ioflags & QUESTBIT_MASTER_JETTISON_ENABLED) {
        DualLog("QUESTBIT_MASTER_JETTISON_ENABLED: 1");
        if (AutoSplitterData.missionSplitID == 3) AutoSplitterData.missionSplitID++;
        QuestLogNotesManager.a.notes[10].SetActive(true);
        QuestLogNotesManager.a.checkBoxes[10].isOn = Const.a.questData.MasterJettisonEnabled;
        QuestLogNotesManager.a.labels[10].text = Eng_Text->stringTable[562];
    }
    
    if (Eng_Global->instances[i].ioflags & QUESTBIT_BETA_GROVE_JETTISONED) {
        DualLog("QUESTBIT_BETA_GROVE_JETTISONED: 1");
        if (AutoSplitterData.missionSplitID == 4) AutoSplitterData.missionSplitID++;
        QuestLogNotesManager.a.notes[12].SetActive(true);
        QuestLogNotesManager.a.checkBoxes[12].isOn = Const.a.questData.BetaGroveJettisoned;
        QuestLogNotesManager.a.labels[12].text = Eng_Text->stringTable[565];
        QuestLogNotesManager.a.notes[13].SetActive(true);
        QuestLogNotesManager.a.labels[13].text = Eng_Text->stringTable[566];
    }
    
    if (Eng_Global->instances[i].ioflags & QUESTBIT_ANTENNA_NORTH_DESTROYED) {
        DualLog("QUESTBIT_ANTENNA_NORTH_DESTROYED: 1");
        QuestLogNotesManager.a.notes[13].SetActive(true);
    }
    
    if (Eng_Global->instances[i].ioflags & QUESTBIT_ANTENNA_SOUTH_DESTROYED) {
        DualLog("QUESTBIT_ANTENNA_SOUTH_DESTROYED: 1");
        QuestLogNotesManager.a.notes[13].SetActive(true);
    }
    
    if (Eng_Global->instances[i].ioflags & QUESTBIT_ANTENNA_EAST_DESTROYED) {
        DualLog("QUESTBIT_ANTENNA_EAST_DESTROYED: 1");
        QuestLogNotesManager.a.notes[13].SetActive(true);
    }
    
    if (Eng_Global->instances[i].ioflags & QUESTBIT_ANTENNA_WEST_DESTROYED) {
        DualLog("QUESTBIT_ANTENNA_WEST_DESTROYED: 1");
        QuestLogNotesManager.a.notes[13].SetActive(true);
    }
    
    if (Eng_Global->instances[i].ioflags & QUESTBIT_SELF_DESTRUCT_ACTIVATED) {
        DualLog("QUESTBIT_SELF_DESTRUCT_ACTIVATED: 1");
        QuestLogNotesManager.a.notes[0].SetActive(true);
        QuestLogNotesManager.a.notes[1].SetActive(true);
        QuestLogNotesManager.a.notes[2].SetActive(true);
        QuestLogNotesManager.a.notes[3].SetActive(true);
        QuestLogNotesManager.a.notes[4].SetActive(true);
        QuestLogNotesManager.a.notes[5].SetActive(true);
        QuestLogNotesManager.a.notes[6].SetActive(true);
        QuestLogNotesManager.a.notes[7].SetActive(true);
        QuestLogNotesManager.a.notes[8].SetActive(true);
        QuestLogNotesManager.a.notes[9].SetActive(true);
        QuestLogNotesManager.a.notes[10].SetActive(true);
        QuestLogNotesManager.a.notes[11].SetActive(true);
        QuestLogNotesManager.a.notes[12].SetActive(true);
        QuestLogNotesManager.a.notes[13].SetActive(true);
        QuestLogNotesManager.a.notes[14].SetActive(true); // Self destruct
        QuestLogNotesManager.a.notes[15].SetActive(true); // Escape pod
        QuestLogNotesManager.a.notes[16].SetActive(true); // Access the bridge
        QuestLogNotesManager.a.checkBoxes[14].isOn = Const.a.questData.SelfDestructActivated;
        QuestLogNotesManager.a.labels[14].text = Eng_Text->stringTable[567]; // Set:Engage reactor self-destruct.
        QuestLogNotesManager.a.labels[15].text = Eng_Text->stringTable[568]; // Set:Escape on escape pod.
    }
    
    if (Eng_Global->instances[i].ioflags & QUESTBIT_BRIDGE_SEPARATED) {
        DualLog("QUESTBIT_BRIDGE_SEPARATED: 1");
        QuestLogNotesManager.a.notes[0].SetActive(true);
        QuestLogNotesManager.a.notes[1].SetActive(true);
        QuestLogNotesManager.a.notes[2].SetActive(true);
        QuestLogNotesManager.a.notes[3].SetActive(true);
        QuestLogNotesManager.a.notes[4].SetActive(true);
        QuestLogNotesManager.a.notes[5].SetActive(true);
        QuestLogNotesManager.a.notes[6].SetActive(true);
        QuestLogNotesManager.a.notes[7].SetActive(true);
        QuestLogNotesManager.a.notes[8].SetActive(true);
        QuestLogNotesManager.a.notes[9].SetActive(true);
        QuestLogNotesManager.a.notes[10].SetActive(true);
        QuestLogNotesManager.a.notes[11].SetActive(true);
        QuestLogNotesManager.a.notes[12].SetActive(true);
        QuestLogNotesManager.a.notes[13].SetActive(true);
        QuestLogNotesManager.a.notes[14].SetActive(true); // Self destruct
        QuestLogNotesManager.a.checkBoxes[14].isOn = Const.a.questData.SelfDestructActivated;
        QuestLogNotesManager.a.labels[14].text = Eng_Text->stringTable[567]; // Set:Engage reactor self-destruct.
        QuestLogNotesManager.a.notes[16].SetActive(true);
        QuestLogNotesManager.a.notes[17].SetActive(true);
        QuestLogNotesManager.a.checkBoxes[16].isOn = true;
        QuestLogNotesManager.a.labels[16].text = Eng_Text->stringTable[569]; // Set:Access the bridge.
        QuestLogNotesManager.a.labels[17].text = Eng_Text->stringTable[570]; // Set:Destroy SHODAN.
    }
    
    if (Eng_Global->instances[i].ioflags & QUESTBIT_ISOLINEAR_CHIPSET_INSTALLED) DualLog("QUESTBIT_ISOLINEAR_CHIPSET_INSTALLED: 1");
}

void DisableBits() {
    if (RobotSpawnDeactivated) {
        Const.a.questData.RobotSpawnDeactivated = false;
    }

    if (IsotopeInstalled) Const.a.questData.IsotopeInstalled = false;
    if (ShieldActivated) {
        Const.a.questData.ShieldActivated = false;
        DualLog("Bit unset ShieldActivated: "
                    + Const.a.questData.ShieldActivated.ToString());

        QuestLogNotesManager.a.checkBoxes[8].isOn =
            Const.a.questData.ShieldActivated;
    }
    if (LaserSafetyOverriden) {
        Const.a.questData.LaserSafetyOverriden = false;
        QuestLogNotesManager.a.checkBoxes[7].isOn = Const.a.questData.LaserSafetyOverriden;
    }
    if (LaserDestroyed) {
        Const.a.questData.LaserDestroyed = false;
        QuestLogNotesManager.a.checkBoxes[9].isOn = Const.a.questData.LaserDestroyed;
    }
    if (BetaGroveCyberUnlocked) Const.a.questData.BetaGroveCyberUnlocked = false;
    if (GroveAlphaJettisonEnabled) Const.a.questData.GroveAlphaJettisonEnabled = false;
    if (GroveBetaJettisonEnabled) Const.a.questData.GroveBetaJettisonEnabled = false;
    if (GroveDeltaJettisonEnabled) Const.a.questData.GroveDeltaJettisonEnabled = false;
    if (MasterJettisonBroken) Const.a.questData.MasterJettisonBroken = false;
    if (Relay428Fixed) {
        Const.a.questData.Relay428Fixed = false;
        QuestLogNotesManager.a.checkBoxes[11].isOn = Const.a.questData.Relay428Fixed;
    }
    if (MasterJettisonEnabled) {
        Const.a.questData.MasterJettisonEnabled = false;
        QuestLogNotesManager.a.checkBoxes[10].isOn = Const.a.questData.MasterJettisonEnabled;
    }
    if (BetaGroveJettisoned) {
        Const.a.questData.BetaGroveJettisoned = false;
        QuestLogNotesManager.a.checkBoxes[12].isOn = Const.a.questData.BetaGroveJettisoned;
    }
    if (AntennaNorthDestroyed) Const.a.questData.AntennaNorthDestroyed = false;
    if (AntennaSouthDestroyed) Const.a.questData.AntennaSouthDestroyed = false;
    if (AntennaEastDestroyed) Const.a.questData.AntennaEastDestroyed = false;
    if (AntennaWestDestroyed) Const.a.questData.AntennaWestDestroyed = false;
    if (SelfDestructActivated) {
        Const.a.questData.SelfDestructActivated = false;
        QuestLogNotesManager.a.checkBoxes[14].isOn = Const.a.questData.SelfDestructActivated;
    }
    if (BridgeSeparated) Const.a.questData.BridgeSeparated = false;
    if (IsolinearChipsetInstalled) Const.a.questData.IsolinearChipsetInstalled = false;
}

void ToggleBits() {
    if (RobotSpawnDeactivated) Const.a.questData.RobotSpawnDeactivated = !Const.a.questData.RobotSpawnDeactivated;
    if (IsotopeInstalled) Const.a.questData.IsotopeInstalled = !Const.a.questData.IsotopeInstalled;
    if (ShieldActivated) {
        Const.a.questData.ShieldActivated = !Const.a.questData.ShieldActivated;
        QuestLogNotesManager.a.checkBoxes[8].isOn = Const.a.questData.ShieldActivated;
        if (Const.a.questData.ShieldActivated) {
            QuestLogNotesManager.a.notes[8].SetActive(true);
            QuestLogNotesManager.a.labels[8].text = Eng_Text->stringTable[560];
        }
    }
    if (LaserSafetyOverriden) {
        Const.a.questData.LaserSafetyOverriden = !Const.a.questData.LaserSafetyOverriden;
        QuestLogNotesManager.a.checkBoxes[7].isOn = Const.a.questData.LaserSafetyOverriden;
        if (Const.a.questData.LaserSafetyOverriden) {
            QuestLogNotesManager.a.notes[7].SetActive(true);
            QuestLogNotesManager.a.labels[7].text = Eng_Text->stringTable[559];
        }
    }
    if (LaserDestroyed) {
        Const.a.questData.LaserDestroyed = !Const.a.questData.LaserDestroyed;
        if (AutoSplitterData.missionSplitID == 1) { AutoSplitterData.missionSplitID++; }
        QuestLogNotesManager.a.checkBoxes[9].isOn = Const.a.questData.LaserDestroyed;
        if (Const.a.questData.LaserDestroyed) {
            QuestLogNotesManager.a.notes[9].SetActive(true);
            QuestLogNotesManager.a.labels[9].text = Eng_Text->stringTable[561];
        }
    }
    if (BetaGroveCyberUnlocked) Const.a.questData.BetaGroveCyberUnlocked = !Const.a.questData.BetaGroveCyberUnlocked;
    if (GroveAlphaJettisonEnabled) Const.a.questData.GroveAlphaJettisonEnabled = !Const.a.questData.GroveAlphaJettisonEnabled;
    if (GroveBetaJettisonEnabled) Const.a.questData.GroveBetaJettisonEnabled = !Const.a.questData.GroveBetaJettisonEnabled;
    if (GroveDeltaJettisonEnabled) Const.a.questData.GroveDeltaJettisonEnabled = !Const.a.questData.GroveDeltaJettisonEnabled;
    if (MasterJettisonBroken) {
        Const.a.questData.MasterJettisonBroken = !Const.a.questData.MasterJettisonBroken;
        if (Const.a.questData.MasterJettisonBroken) {
            QuestLogNotesManager.a.notes[11].SetActive(true); // Diagnose and repair broken relay
            QuestLogNotesManager.a.labels[11].text = Eng_Text->stringTable[563];// Set:Diagnose and repair broken relay
        }
    }
    if (Relay428Fixed) {
        Const.a.questData.Relay428Fixed = !Const.a.questData.Relay428Fixed;
        QuestLogNotesManager.a.checkBoxes[11].isOn = Const.a.questData.Relay428Fixed;
        if (Const.a.questData.Relay428Fixed) {
            QuestLogNotesManager.a.notes[11].SetActive(true);
            QuestLogNotesManager.a.labels[11].text = Eng_Text->stringTable[563]; // Set:Diagnose and repair broken relay
            QuestLogNotesManager.a.labels[11].text += Eng_Text->stringTable[564]; // Add:: 428.
        }
    }
    if (MasterJettisonEnabled) {
        Const.a.questData.MasterJettisonEnabled = !Const.a.questData.MasterJettisonEnabled;
        QuestLogNotesManager.a.checkBoxes[10].isOn = Const.a.questData.MasterJettisonEnabled;
        if (Const.a.questData.MasterJettisonEnabled) {
            QuestLogNotesManager.a.notes[10].SetActive(true);
            QuestLogNotesManager.a.labels[10].text = Eng_Text->stringTable[562];
        }
    }
    if (BetaGroveJettisoned) {
        Const.a.questData.BetaGroveJettisoned = !Const.a.questData.BetaGroveJettisoned;
        QuestLogNotesManager.a.checkBoxes[12].isOn = Const.a.questData.BetaGroveJettisoned;
        if (Const.a.questData.BetaGroveJettisoned ) {
            QuestLogNotesManager.a.notes[12].SetActive(true);
            QuestLogNotesManager.a.labels[12].text = Eng_Text->stringTable[565];
            QuestLogNotesManager.a.notes[13].SetActive(true);
            QuestLogNotesManager.a.labels[13].text = Eng_Text->stringTable[566];
        }
    }
    if (AntennaNorthDestroyed) Const.a.questData.AntennaNorthDestroyed = !Const.a.questData.AntennaNorthDestroyed;
    if (AntennaSouthDestroyed) Const.a.questData.AntennaSouthDestroyed = !Const.a.questData.AntennaSouthDestroyed;
    if (AntennaEastDestroyed) Const.a.questData.AntennaEastDestroyed = !Const.a.questData.AntennaEastDestroyed;
    if (AntennaWestDestroyed) Const.a.questData.AntennaWestDestroyed = !Const.a.questData.AntennaWestDestroyed;
    if (SelfDestructActivated) {
        Const.a.questData.SelfDestructActivated = !Const.a.questData.SelfDestructActivated;
        if (Const.a.questData.SelfDestructActivated) {
            QuestLogNotesManager.a.notes[14].SetActive(true);
            QuestLogNotesManager.a.notes[15].SetActive(true); // Escape pod
            QuestLogNotesManager.a.labels[14].text = Eng_Text->stringTable[567];// Set:Engage reactor self-destruct.
            QuestLogNotesManager.a.labels[15].text = Eng_Text->stringTable[568];// Set:Escape on escape pod.
        }
    }
    if (BridgeSeparated) {
        Const.a.questData.BridgeSeparated = !Const.a.questData.BridgeSeparated;
        if (Const.a.questData.BridgeSeparated) {
            QuestLogNotesManager.a.notes[16].SetActive(true);
            QuestLogNotesManager.a.notes[17].SetActive(true);
            QuestLogNotesManager.a.checkBoxes[16].isOn = true;
            QuestLogNotesManager.a.labels[16].text = Eng_Text->stringTable[569]; // Set:Access the bridge.
            QuestLogNotesManager.a.labels[17].text = Eng_Text->stringTable[570]; // Set:Destroy SHODAN.
        }
    }
    if (IsolinearChipsetInstalled) Const.a.questData.IsolinearChipsetInstalled = !Const.a.questData.IsolinearChipsetInstalled;
}

void TestBits(bool testIfTrue, UseData ud, TargetIO tio) {
    if (RobotSpawnDeactivated && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.RobotSpawnDeactivated, testIfTrue, ud, tio, target, targetIfFalse);
    if (IsotopeInstalled && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.IsotopeInstalled, testIfTrue, ud, tio, target, targetIfFalse);
    if (ShieldActivated && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.ShieldActivated, testIfTrue, ud, tio, target, targetIfFalse);
    if (LaserSafetyOverriden && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.LaserSafetyOverriden, testIfTrue, ud, tio, target, targetIfFalse);
    if (LaserDestroyed && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.LaserDestroyed, testIfTrue, ud, tio, target, targetIfFalse);
    if (BetaGroveCyberUnlocked && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.BetaGroveCyberUnlocked, testIfTrue, ud, tio, target, targetIfFalse);
    if (GroveAlphaJettisonEnabled && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.GroveAlphaJettisonEnabled, testIfTrue, ud, tio, target, targetIfFalse);
    if (GroveBetaJettisonEnabled && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.GroveBetaJettisonEnabled, testIfTrue, ud, tio, target, targetIfFalse);
    if (GroveDeltaJettisonEnabled && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.GroveDeltaJettisonEnabled, testIfTrue, ud, tio, target, targetIfFalse);
    if (MasterJettisonBroken && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.MasterJettisonBroken, testIfTrue, ud, tio, target, targetIfFalse);
    if (Relay428Fixed && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.Relay428Fixed, testIfTrue, ud, tio, target, targetIfFalse);
    if (MasterJettisonEnabled && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.MasterJettisonEnabled, testIfTrue, ud, tio, target, targetIfFalse);
    if (BetaGroveJettisoned && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.BetaGroveJettisoned, testIfTrue, ud, tio, target, targetIfFalse);
    if (AntennaNorthDestroyed && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.AntennaNorthDestroyed, testIfTrue, ud, tio, target, targetIfFalse);
    if (AntennaSouthDestroyed && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.AntennaSouthDestroyed, testIfTrue, ud, tio, target, targetIfFalse);
    if (AntennaEastDestroyed && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.AntennaEastDestroyed, testIfTrue, ud, tio, target, targetIfFalse);
    if (AntennaWestDestroyed && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.AntennaWestDestroyed, testIfTrue, ud, tio, target, targetIfFalse);
    if (SelfDestructActivated && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.SelfDestructActivated, testIfTrue, ud, tio, target, targetIfFalse);
    if (BridgeSeparated && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.BridgeSeparated, testIfTrue, ud, tio, target, targetIfFalse);
    if (IsolinearChipsetInstalled && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.IsolinearChipsetInstalled, testIfTrue, ud, tio, target, targetIfFalse);
}
