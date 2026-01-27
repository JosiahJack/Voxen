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
    instances[WORLD].ioflags |= instances[i].ioflags;
    
    if (instances[i].ioflags & QUESTBIT_ROBOT_SPAWN_DEACTIVATED) DualLog("QUESTBIT_ROBOT_SPAWN_DEACTIVATED: 1");
    if (instances[i].ioflags & QUESTBIT_ISOTOPE_INSTALLED) DualLog("QUESTBIT_ISOTOPE_INSTALLED: 1");
    if (instances[i].ioflags & QUESTBIT_SHIELD_ACTIVATED) {
        DualLog("QUESTBIT_SHIELD_ACTIVATED: 1");
        QuestLogNotesManager.a.notes[8].SetActive(true);
        QuestLogNotesManager.a.checkBoxes[8].isOn = Const.a.questData.ShieldActivated;
        QuestLogNotesManager.a.labels[8].text = Const.a.stringTable[560];
    }
    
    if (instances[i].ioflags & QUESTBIT_LASER_SAFETY_OVERRIDEN) {
        DualLog("QUESTBIT_LASER_SAFETY_OVERRIDEN: 1");
        QuestLogNotesManager.a.notes[7].SetActive(true);
        QuestLogNotesManager.a.checkBoxes[7].isOn = Const.a.questData.LaserSafetyOverriden;
        QuestLogNotesManager.a.labels[7].text = Const.a.stringTable[559];
    }
    
    if (instances[i].ioflags & QUESTBIT_LASER_DESTROYED) {
        DualLog("QUESTBIT_LASER_DESTROYED: 1");
        if (AutoSplitterData.missionSplitID == 1) AutoSplitterData.missionSplitID++;
        QuestLogNotesManager.a.notes[9].SetActive(true);
        QuestLogNotesManager.a.checkBoxes[9].isOn = Const.a.questData.LaserDestroyed;
        QuestLogNotesManager.a.labels[9].text = Const.a.stringTable[561];
    }
    
    if (instances[i].ioflags & QUESTBIT_BETA_GROVE_CYBER_UNLOCKED) {
        DualLog("QUESTBIT_BETA_GROVE_CYBER_UNLOCKED: 1");
        QuestLogNotesManager.a.notes[12].SetActive(true);
    }
    
    if (instances[i].ioflags & QUESTBIT_GROVE_ALPHA_JETTISON_ENABLED) {
        DualLog("QUESTBIT_GROVE_ALPHA_JETTISON_ENABLED: 1");
        QuestLogNotesManager.a.notes[12].SetActive(true);
    }
    
    if (instances[i].ioflags & QUESTBIT_GROVE_BETA_JETTISON_ENABLED) {
        DualLog("QUESTBIT_GROVE_BETA_JETTISON_ENABLED: 1");
        QuestLogNotesManager.a.notes[12].SetActive(true);
    }
    
    if (instances[i].ioflags & QUESTBIT_GROVE_DELTA_JETTISON_ENABLED) {
        DualLog("QUESTBIT_GROVE_DELTA_JETTISON_ENABLED: 1");
        QuestLogNotesManager.a.notes[12].SetActive(true);
    }
    
    if (instances[i].ioflags & QUESTBIT_MASTER_JETTISON_BROKEN) {
        DualLog("QUESTBIT_MASTER_JETTISON_BROKEN: 1");
        if (AutoSplitterData.missionSplitID == 2) AutoSplitterData.missionSplitID++;
        QuestLogNotesManager.a.notes[12].SetActive(true);
        QuestLogNotesManager.a.notes[11].SetActive(true);
        QuestLogNotesManager.a.labels[11].text = Const.a.stringTable[563]; // Set:Diagnose and repair broken relay
    }
    
    if (instances[i].ioflags & QUESTBIT_RELAY_428_FIXED) {
        DualLog("QUESTBIT_RELAY_428_FIXED: 1");
        QuestLogNotesManager.a.notes[11].SetActive(true);
        QuestLogNotesManager.a.checkBoxes[11].isOn = Const.a.questData.Relay428Fixed;
        QuestLogNotesManager.a.labels[11].text = Const.a.stringTable[563]; // Set:Diagnose and repair broken relay
        QuestLogNotesManager.a.labels[11].text += Const.a.stringTable[564]; // Add:: 428.
    }
    
    if (instances[i].ioflags & QUESTBIT_MASTER_JETTISON_ENABLED) {
        DualLog("QUESTBIT_MASTER_JETTISON_ENABLED: 1");
        if (AutoSplitterData.missionSplitID == 3) AutoSplitterData.missionSplitID++;
        QuestLogNotesManager.a.notes[10].SetActive(true);
        QuestLogNotesManager.a.checkBoxes[10].isOn = Const.a.questData.MasterJettisonEnabled;
        QuestLogNotesManager.a.labels[10].text = Const.a.stringTable[562];
    }
    
    if (instances[i].ioflags & QUESTBIT_BETA_GROVE_JETTISONED) {
        DualLog("QUESTBIT_BETA_GROVE_JETTISONED: 1");
        if (AutoSplitterData.missionSplitID == 4) AutoSplitterData.missionSplitID++;
        QuestLogNotesManager.a.notes[12].SetActive(true);
        QuestLogNotesManager.a.checkBoxes[12].isOn = Const.a.questData.BetaGroveJettisoned;
        QuestLogNotesManager.a.labels[12].text = Const.a.stringTable[565];
        QuestLogNotesManager.a.notes[13].SetActive(true);
        QuestLogNotesManager.a.labels[13].text = Const.a.stringTable[566];
    }
    
    if (instances[i].ioflags & QUESTBIT_ANTENNA_NORTH_DESTROYED) {
        DualLog("QUESTBIT_ANTENNA_NORTH_DESTROYED: 1");
        QuestLogNotesManager.a.notes[13].SetActive(true);
    }
    
    if (instances[i].ioflags & QUESTBIT_ANTENNA_SOUTH_DESTROYED) {
        DualLog("QUESTBIT_ANTENNA_SOUTH_DESTROYED: 1");
        QuestLogNotesManager.a.notes[13].SetActive(true);
    }
    
    if (instances[i].ioflags & QUESTBIT_ANTENNA_EAST_DESTROYED) {
        DualLog("QUESTBIT_ANTENNA_EAST_DESTROYED: 1");
        QuestLogNotesManager.a.notes[13].SetActive(true);
    }
    
    if (instances[i].ioflags & QUESTBIT_ANTENNA_WEST_DESTROYED) {
        DualLog("QUESTBIT_ANTENNA_WEST_DESTROYED: 1");
        QuestLogNotesManager.a.notes[13].SetActive(true);
    }
    
    if (instances[i].ioflags & QUESTBIT_SELF_DESTRUCT_ACTIVATED) {
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
        QuestLogNotesManager.a.labels[14].text = Const.a.stringTable[567]; // Set:Engage reactor self-destruct.
        QuestLogNotesManager.a.labels[15].text = Const.a.stringTable[568]; // Set:Escape on escape pod.
    }
    
    if (instances[i].ioflags & QUESTBIT_BRIDGE_SEPARATED) {
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
        QuestLogNotesManager.a.labels[14].text = Const.a.stringTable[567]; // Set:Engage reactor self-destruct.
        QuestLogNotesManager.a.notes[16].SetActive(true);
        QuestLogNotesManager.a.notes[17].SetActive(true);
        QuestLogNotesManager.a.checkBoxes[16].isOn = true;
        QuestLogNotesManager.a.labels[16].text = Const.a.stringTable[569]; // Set:Access the bridge.
        QuestLogNotesManager.a.labels[17].text = Const.a.stringTable[570]; // Set:Destroy SHODAN.
    }
    
    if (instances[i].ioflags & QUESTBIT_ISOLINEAR_CHIPSET_INSTALLED) DualLog("QUESTBIT_ISOLINEAR_CHIPSET_INSTALLED: 1");
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
            QuestLogNotesManager.a.labels[8].text = Const.a.stringTable[560];
        }
    }
    if (LaserSafetyOverriden) {
        Const.a.questData.LaserSafetyOverriden = !Const.a.questData.LaserSafetyOverriden;
        QuestLogNotesManager.a.checkBoxes[7].isOn = Const.a.questData.LaserSafetyOverriden;
        if (Const.a.questData.LaserSafetyOverriden) {
            QuestLogNotesManager.a.notes[7].SetActive(true);
            QuestLogNotesManager.a.labels[7].text = Const.a.stringTable[559];
        }
    }
    if (LaserDestroyed) {
        Const.a.questData.LaserDestroyed = !Const.a.questData.LaserDestroyed;
        if (AutoSplitterData.missionSplitID == 1) { AutoSplitterData.missionSplitID++; }
        QuestLogNotesManager.a.checkBoxes[9].isOn = Const.a.questData.LaserDestroyed;
        if (Const.a.questData.LaserDestroyed) {
            QuestLogNotesManager.a.notes[9].SetActive(true);
            QuestLogNotesManager.a.labels[9].text = Const.a.stringTable[561];
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
            QuestLogNotesManager.a.labels[11].text = Const.a.stringTable[563];// Set:Diagnose and repair broken relay
        }
    }
    if (Relay428Fixed) {
        Const.a.questData.Relay428Fixed = !Const.a.questData.Relay428Fixed;
        QuestLogNotesManager.a.checkBoxes[11].isOn = Const.a.questData.Relay428Fixed;
        if (Const.a.questData.Relay428Fixed) {
            QuestLogNotesManager.a.notes[11].SetActive(true);
            QuestLogNotesManager.a.labels[11].text = Const.a.stringTable[563]; // Set:Diagnose and repair broken relay
            QuestLogNotesManager.a.labels[11].text += Const.a.stringTable[564]; // Add:: 428.
        }
    }
    if (MasterJettisonEnabled) {
        Const.a.questData.MasterJettisonEnabled = !Const.a.questData.MasterJettisonEnabled;
        QuestLogNotesManager.a.checkBoxes[10].isOn = Const.a.questData.MasterJettisonEnabled;
        if (Const.a.questData.MasterJettisonEnabled) {
            QuestLogNotesManager.a.notes[10].SetActive(true);
            QuestLogNotesManager.a.labels[10].text = Const.a.stringTable[562];
        }
    }
    if (BetaGroveJettisoned) {
        Const.a.questData.BetaGroveJettisoned = !Const.a.questData.BetaGroveJettisoned;
        QuestLogNotesManager.a.checkBoxes[12].isOn = Const.a.questData.BetaGroveJettisoned;
        if (Const.a.questData.BetaGroveJettisoned ) {
            QuestLogNotesManager.a.notes[12].SetActive(true);
            QuestLogNotesManager.a.labels[12].text = Const.a.stringTable[565];
            QuestLogNotesManager.a.notes[13].SetActive(true);
            QuestLogNotesManager.a.labels[13].text = Const.a.stringTable[566];
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
            QuestLogNotesManager.a.labels[14].text = Const.a.stringTable[567];// Set:Engage reactor self-destruct.
            QuestLogNotesManager.a.labels[15].text = Const.a.stringTable[568];// Set:Escape on escape pod.
        }
    }
    if (BridgeSeparated) {
        Const.a.questData.BridgeSeparated = !Const.a.questData.BridgeSeparated;
        if (Const.a.questData.BridgeSeparated) {
            QuestLogNotesManager.a.notes[16].SetActive(true);
            QuestLogNotesManager.a.notes[17].SetActive(true);
            QuestLogNotesManager.a.checkBoxes[16].isOn = true;
            QuestLogNotesManager.a.labels[16].text = Const.a.stringTable[569]; // Set:Access the bridge.
            QuestLogNotesManager.a.labels[17].text = Const.a.stringTable[570]; // Set:Destroy SHODAN.
        }
    }
    if (IsolinearChipsetInstalled) Const.a.questData.IsolinearChipsetInstalled = !Const.a.questData.IsolinearChipsetInstalled;
}

void TestBits(bool testIfTrue, UseData ud, TargetIO tio) {
    if (RobotSpawnDeactivated && (!data_parser_isspace(target) || !data_parser_isspace(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.RobotSpawnDeactivated, testIfTrue, ud, tio, target, targetIfFalse);
    if (IsotopeInstalled && (!data_parser_isspace(target) || !data_parser_isspace(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.IsotopeInstalled, testIfTrue, ud, tio, target, targetIfFalse);
    if (ShieldActivated && (!data_parser_isspace(target) || !data_parser_isspace(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.ShieldActivated, testIfTrue, ud, tio, target, targetIfFalse);
    if (LaserSafetyOverriden && (!data_parser_isspace(target) || !data_parser_isspace(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.LaserSafetyOverriden, testIfTrue, ud, tio, target, targetIfFalse);
    if (LaserDestroyed && (!data_parser_isspace(target) || !data_parser_isspace(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.LaserDestroyed, testIfTrue, ud, tio, target, targetIfFalse);
    if (BetaGroveCyberUnlocked && (!data_parser_isspace(target) || !data_parser_isspace(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.BetaGroveCyberUnlocked, testIfTrue, ud, tio, target, targetIfFalse);
    if (GroveAlphaJettisonEnabled && (!data_parser_isspace(target) || !data_parser_isspace(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.GroveAlphaJettisonEnabled, testIfTrue, ud, tio, target, targetIfFalse);
    if (GroveBetaJettisonEnabled && (!data_parser_isspace(target) || !data_parser_isspace(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.GroveBetaJettisonEnabled, testIfTrue, ud, tio, target, targetIfFalse);
    if (GroveDeltaJettisonEnabled && (!data_parser_isspace(target) || !data_parser_isspace(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.GroveDeltaJettisonEnabled, testIfTrue, ud, tio, target, targetIfFalse);
    if (MasterJettisonBroken && (!data_parser_isspace(target) || !data_parser_isspace(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.MasterJettisonBroken, testIfTrue, ud, tio, target, targetIfFalse);
    if (Relay428Fixed && (!data_parser_isspace(target) || !data_parser_isspace(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.Relay428Fixed, testIfTrue, ud, tio, target, targetIfFalse);
    if (MasterJettisonEnabled && (!data_parser_isspace(target) || !data_parser_isspace(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.MasterJettisonEnabled, testIfTrue, ud, tio, target, targetIfFalse);
    if (BetaGroveJettisoned && (!data_parser_isspace(target) || !data_parser_isspace(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.BetaGroveJettisoned, testIfTrue, ud, tio, target, targetIfFalse);
    if (AntennaNorthDestroyed && (!data_parser_isspace(target) || !data_parser_isspace(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.AntennaNorthDestroyed, testIfTrue, ud, tio, target, targetIfFalse);
    if (AntennaSouthDestroyed && (!data_parser_isspace(target) || !data_parser_isspace(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.AntennaSouthDestroyed, testIfTrue, ud, tio, target, targetIfFalse);
    if (AntennaEastDestroyed && (!data_parser_isspace(target) || !data_parser_isspace(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.AntennaEastDestroyed, testIfTrue, ud, tio, target, targetIfFalse);
    if (AntennaWestDestroyed && (!data_parser_isspace(target) || !data_parser_isspace(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.AntennaWestDestroyed, testIfTrue, ud, tio, target, targetIfFalse);
    if (SelfDestructActivated && (!data_parser_isspace(target) || !data_parser_isspace(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.SelfDestructActivated, testIfTrue, ud, tio, target, targetIfFalse);
    if (BridgeSeparated && (!data_parser_isspace(target) || !data_parser_isspace(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.BridgeSeparated, testIfTrue, ud, tio, target, targetIfFalse);
    if (IsolinearChipsetInstalled && (!data_parser_isspace(target) || !data_parser_isspace(targetIfFalse))) Const.a.questData.TargetOnGatePassed(Const.a.questData.IsolinearChipsetInstalled, testIfTrue, ud, tio, target, targetIfFalse);
}

// 	float justSavedTimeStamp;
// 	float savedReminderTime = 7f; // human short-term memory length
// 	bool startingNewGame = false;
// 	bool introNotPlayed = false;
// 	const float frobDistance = 4.9f;
// 	const float elevatorPadUseDistance = 2f;
// 	int creditsLength;
// 	Transform player1TargettingPos;
// 	GameObject player1Capsule;
// 	PlayerMovement player1PlayerMovementScript;
// 	PlayerHealth player1PlayerHealthScript;
// 	GameObject player1CapsuleMainCameragGO;
// 	List<PauseRigidbody> prb;
// 	List<PauseParticleSystem> psys;
// 	List<PauseAnimation> panimsList;

// 	public bool RaycastBudgetExceeded() {
// 		return (numberOfRaycastsThisFrame > maxRaycastsPerFrame);
// 	}
// 
//     public void LoadTextForLanguage(int lang) {
// // 		UnityEngine.DualLog("Loading language: " + lang.ToString());
//         string readline; // variable to hold each string read in from the file
//         int currentline = 0;
//         string tF = "text_english.txt";
//         switch (lang) {
//             case 0: tF = "text_english.txt"; break;
// 			case 1: tF = "text_espanol.txt"; break; // UPKEEP: Other languages
// 			case 2: tF = "text_deutsch.txt"; break; // German
// 			case 3: tF = "text_francais.txt"; break; // French
// 			case 4: tF = "text_nihongo.txt"; break; // Japanese
// 			case 5: tF = "text_russkiy.txt"; break; // Russian
// 			case 6: tF = "text_italiano.txt"; break; // Italian
// 			case 7: tF = "text_portugues.txt"; break; // Portugese
//         }
// 
//         StreamReader dataReader = Utils.ReadStreamingAsset(tF);
// 		if (stringTable.Length < 1025) stringTable = new string[1025];
//         using (dataReader) {
//             do {
//                 // Read the next line
//                 readline = dataReader.ReadLine();
//                 if (currentline < stringTable.Length) {
//                     stringTable[currentline] = readline;
// 				} else {
// 					UnityEngine.DualLog("WARNING: Ran out of slots in "
// 										  + "stringTable at "
// 										  + currentline.ToString());
// 					dataReader.Close();
// 					return;
// 				}
//                 currentline++;
//             } while (!dataReader.EndOfStream);
//             dataReader.Close();
// 			stringTableLoaded = true;
//             return;
//         }
//     }
// 
// 	void Start() {
// 		Config.LoadConfig();
// 		layerMaskNPCSight = LayerMask.GetMask("Default","Geometry",
// 											  "Door","InterDebris",
// 											  "PhysObjects","Player","Player2",
// 											  "Player3","Player4");
// 		layerMaskNPCAttack = LayerMask.GetMask("Default","Geometry","NPC",
// 											   "Door","InterDebris",
// 											   "PhysObjects","Player","Player2",
// 											   "Player3","Player4");
// 
// 		// Not including "Bullets" as this is merely used for spawning, not
// 		// setting level-wide NPC collisions.
// 		layerMaskNPCCollision = LayerMask.GetMask("Default","TransparentFX",
// 												  "IgnoreRaycast","Geometry",
// 												  "NPC","Door","InterDebris",
// 												  "Player","Clip","NPCClip",
// 												  "PhysObjects");
// 
// 		// Water is a hidden layer that prevents the player frobbing through
// 		// gratings, X-doors, etc.  Oh and also water...if that were a thing.
// 		layerMaskPlayerFrob = LayerMask.GetMask("Default","Geometry","Water",
// 												"Door","InterDebris",
// 												"PhysObjects","Player2",
// 												"Player3","Player4",
// 												"CorpseSearchable");
// 
// 		// Must have the geometry and default layers to prevent locking onto
// 		// NPCs through walls.
// 		layerMaskPlayerTargetIDFrob = LayerMask.GetMask("Default","Geometry",
// 														"Door",
// 														"Player2","Player3",
// 														"Player4","NPC",
// 														"CorpseSearchable");
// 
// 		layerMaskPlayerAttack = LayerMask.GetMask("Default","Geometry","NPC",
// 												  "Bullets","Door",
// 												  "InterDebris","PhysObjects",
// 												  "Player2","Player3","Player4",
// 												  "CorpseSearchable");
// 			
// 		layerMaskExplosion = LayerMask.GetMask("Default","Geometry","NPC",
// 												  "Bullets","Door",
// 												  "InterDebris","PhysObjects",
// 												  "Player2","Player3","Player4",
// 											  	  "Player","CorpseSearchable");
// 			
// 
// 		layerMaskPlayerFeet = LayerMask.GetMask("Default","Geometry");
// 
// 		LoadCreditsData();
// 		StartCoroutine(InitializeEventSystem());
// 		questData = new QuestBits ();
// // 		if (mainFont1 != null) { // Ensure text is crisp and readable.
// 			mainFont1.material.mainTexture.filterMode = FilterMode.Point;
// // 		}
// 
// // 		if (mainFont2 != null) { // Ensure text is crisp and readable.
// 			mainFont2.material.mainTexture.filterMode = FilterMode.Point;
// // 		}
// 
// 		ResetPauseLists();
// 		GameObject newGameIndicator = GameObject.Find("NewGameIndicator");
// 		GameObject loadGameIndicator = GameObject.Find("LoadGameIndicator");
// 		if (loadGameIndicator != null) {
// 			// OK OK ok ok, so we aren't actually using this now and are just wiping out
// 			// dynamic object containers and relying on loading all static object data to
// 			// all static objects with manually generated indices now instead of Unity's
// 			// guid since the guid IS DIFFERENT when you reload the scene because the guids
// 			// are recreated as both scenes need loaded at once due to Unity's horrible no
// 			// good uncontrollable way of doing scene reloads (which could be round-about
// 			// worked around by having an empty dummy scene but poses other problems with
// 			// DontDestroyOnLoad stuff.  Regardless the guids are wiped and thus breaks the
// 			// old method of correlating saved objects in savefile to objects in the scene.
// 			//
// 			// Hopefully I've successfully marked and do save every static object to restore
// 			// it to its former glory as-is when saved.  Hopefully.
// 			MainMenuHandler.a.IntroVideo.SetActive(false);
// 			MainMenuHandler.a.IntroVideoContainer.SetActive(false);
// 			PauseScript.a.mainMenu.SetActive(false);
// 			SceneTransitionHandler sth = loadGameIndicator.GetComponent<SceneTransitionHandler>();
// 			sth.Load();
// 		} else if (newGameIndicator != null || Application.platform == RuntimePlatform.Android) {
// 			UnityEngine.DualLog("newGameIndicator.name: " + newGameIndicator.name);
// 			Utils.SafeDestroy(newGameIndicator);
// 			GoIntoGame();				  // Start of the game!!
// 		}
// 	}
// 
// 	private void CheckIfNewGame () {
// 		string readline; // variable to hold each string read in from the file
// 		int currentline = 0;
// 		string dr;
// 		string fileName = "ng.dat";
// 		if (Application.platform == RuntimePlatform.Android) {
// 		    a.introNotPlayed = false;
// 			return;
// 		} else {
// 			string basePath = Utils.GetAppropriateDataPath();
// 			Utils.ConfirmExistsMakeIfNot(basePath,fileName);
// 			dr = Utils.SafePathCombine(basePath,fileName);
// 		}
// 
// 		if (!File.Exists(dr)) {
// 			UnityEngine.DualLog(fileName + " not found nor recreated");
// 			return;
// 		}
// 
// 		StreamReader dataReader = new StreamReader(dr,Encoding.ASCII);
// 		using (dataReader) {
// 			do {
// 				readline = dataReader.ReadLine(); // Read the next line
// 				if (currentline == 1) a.introNotPlayed = readline.Equals("1");
// 				currentline++;
// 			} while (!dataReader.EndOfStream);
// 
// 			dataReader.Close();
// 			return;
// 		}
// 	}
// 
// void WriteDatForIntroPlayed(bool setIntroNotPlayed) {
//     // Write bit to file
//     // No need to confirm it exists as StreamWriter will make it if not.
//     string basePath = Utils.GetAppropriateDataPath();
//     string dr = Utils.SafePathCombine(basePath,"ng.dat");
//     StreamWriter sw = new StreamWriter(dr,false,Encoding.ASCII);
//     if (sw != null) {
//         using (sw) {
//             sw.WriteLine(Utils.BoolToStringConfig(setIntroNotPlayed));
//             sw.Close();
//         }
//     }
// 
//     a.introNotPlayed = setIntroNotPlayed;
// }
//
//     public GameObject GetImpactType(HealthManager hm) {
//         if (hm == null) return GetObjectFromPool(PoolType.SparksSmall);
//         switch (hm.bloodType) {
//             case BloodType_None: return GetObjectFromPool(PoolType.SparksSmall);
//             case BloodType_Red: return GetObjectFromPool(PoolType.BloodSpurtSmall);
//             case BloodType_Yellow: return GetObjectFromPool(PoolType.BloodSpurtSmallYellow);
//             case BloodType_Green: return GetObjectFromPool(PoolType.BloodSpurtSmallGreen);
//             case BloodType_Robot: return GetObjectFromPool(PoolType.SparksSmallBlue);
// 			case BloodType_Leaf: return GetObjectFromPool(PoolType.LeafBurst);
// 			case BloodType_Mutation: return GetObjectFromPool(PoolType.MutationBurst);
// 			case BloodType_GrayMutation: return GetObjectFromPool(PoolType.GraytationBurst);
//         }
// 
//         return GetObjectFromPool(PoolType.SparksSmall);
// 	}
// 
// 	// Wrapper function to enable Save to be a coroutine so we can display
// 	// progress.  We don't though, currently we just haul off and get it with
// 	// top speed, no pausing momentarily to draw any progress bar since it is
// 	// plenty fast enough.
// 	public void StartSave(int index, string savename) {
// 		if (PlayerHealth.a.hm.health < 1.0f) return; // Can't save while dead!
//         if (Application.platform == RuntimePlatform.Android) return;
// 	    
// 		StartCoroutine(SaveRoutine(index,savename));
// 	}
// 
// 	// Going into the game removes the helper GameObjects for these reasons:
// 	// - GameNotYetStarted, Game is now started, mark it as such.  This happens
// 	//                      only on game entry at beginning of session (first
// 	//                      time after launching the game).
// 	// - NewGameIndicator,  Game is no longer a new game, because it's started.
// 	// - LoadGameIndicator, Game should have been loaded prior to entry.
// 	public void GoIntoGame(Stopwatch loadTimer) {
// 		GameObject freshGame = GameObject.Find("GameNotYetStarted");
// 		if (freshGame != null) Utils.SafeDestroy(freshGame);
// 		GameObject saveIndicator = GameObject.Find("NewGameIndicator");
// 		if (saveIndicator != null) {
// 			SceneTransitionHandler sth = saveIndicator.GetComponent<SceneTransitionHandler>();
// 			UnityEngine.DualLog("Acquiring sth data");
// 			if (sth != null) {
// 				if (sth.setActiveAtNext) {
// 					Sys_Global.difficultyCombat = sth.diffCombatCarryover;
// 					SSys_Global.difficultyMission = sth.diffMissionCarryover;
// 					SSys_Global.difficultyPuzzle = sth.diffPuzzleCarryover;
// 					SSys_Global.difficultyCyber = sth.diffCyberCarryover;
// 				}
// 			}
// 			Utils.SafeDestroy(saveIndicator);
// 		}
// 		
// 		GameObject loadIndicator = GameObject.Find("LoadGameIndicator");
// 		if (loadIndicator != null) {
// 			SceneTransitionHandler sth = loadIndicator.GetComponent<SceneTransitionHandler>();
// 			UnityEngine.DualLog("Acquiring sth data");
// 			if (sth != null) {
// 				if (sth.setActiveAtNext) {
// 					Sys_Global.difficultyCombat = sth.diffCombatCarryover;
// 					SSys_Global.difficultyMission = sth.diffMissionCarryover;
// 					SSys_Global.difficultyPuzzle = sth.diffPuzzleCarryover;
// 					SSys_Global.difficultyCyber = sth.diffCyberCarryover;
// 				}
// 			}
// 			Utils.SafeDestroy(loadIndicator);
// 		}
// 		
// 		Cursor.visible = true;
// 		Utils.Deactivate(loadingScreen);
// 		Utils.Deactivate(MainMenuHandler.a.IntroVideo);
// 		Utils.Deactivate(MainMenuHandler.a.IntroVideoContainer);
// 		Automap.a.ActivateAutomapUI();
// 		Automap.a.DeactivateAutomapUI();
// 		Utils.Deactivate(PauseScript.a.mainMenu);
// 		PauseScript.a.PauseDisable();
// 		if (PlayerHealth.a != null) {
// 			if (PlayerHealth.a.hm != null) PlayerHealth.a.hm.ClearOverlays();
// 		}
// 
// 		if (Const.a.NoShootMode) MouseLookScript.a.ForceInventoryMode();
// 		Utils.Activate(player1Capsule);
// 		Utils.Activate(player1CapsuleMainCameragGO.transform.parent.gameObject);
// 		Utils.Activate(player1CapsuleMainCameragGO);
// 		Utils.EnableCamera(MouseLookScript.a.playerCamera);
// 		WriteDatForIntroPlayed(false);
// 		if (loadTimer == null) {
// 			sprint(stringTable[197]);
// 		} else {
// 			sprint(stringTable[197] + " (" + loadTimer.Elapsed.ToString() + ")"); // Loading...Done!
// 		}
// 		
// 		DynamicCulling.a.Cull(false);
// 	}
// 
// 	public void GoIntoGame() {
// 		GoIntoGame(null);
// 	}
// 
// 	public void ShowLoading() {
// 		Utils.DisableCamera(MouseLookScript.a.playerCamera); // Hide changes.
// 		PauseScript.a.mainMenu.SetActive(false); // Ensure that main menu is 
// 												 // off if came from Load page.
//  		PauseScript.a.PauseEnable(); // Enable pause to make sure that nothing 
// 									 // goes on during couroutine as it happens
// 									 // over multiple frames.
// 		PauseScript.a.DisablePauseUI(); // Enable loading texts and unlock cursor.
// 		sprint(stringTable[196]); // Loading...
// 		if (PlayerHealth.a != null) PlayerHealth.a.hm.ClearOverlays();
// 		Cursor.lockState = CursorLockMode.None;
// 		Cursor.visible = true;
// 		loadPercentText.text = "(1) --.--";
// 
// 		// Clear the HUD
// 		MFDManager.a.TabReset(true);
// 		MFDManager.a.TabReset(false);
// 		MFDManager.a.DisableAllCenterTabs();
// 		loadingScreen.SetActive(true);
// 		AutoSplitterData.isLoading = true;
// 	}
// 
// 	public void ReloadScene(SceneTransitionHandler sth) {
// 		int index = SceneManager.GetActiveScene().buildIndex; // CitadelScene
// 		ObjectContainmentSystem.ClearLists();
//         SceneManager.CreateScene("LoadScene");
// 		Scene loadScene = SceneManager.GetSceneByName("LoadScene");
//         SceneManager.SetActiveScene(loadScene);
// 		AsyncOperation aso = SceneManager.UnloadSceneAsync(index,
// 							  UnloadSceneOptions.UnloadAllEmbeddedSceneObjects);
// 		sth.Reload(index, ref aso);
// 	}
// 
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
// 		startingNewGame = false;
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
// 			if (Inventory.a.hasHardware[1]) {
// 				// Go through all HealthManagers in the game and initialize the
// 				// linked overlays now for Automap.  Done after instantiation.
// 				List<GameObject> hmGOs = new List<GameObject>();
// 				
// 				// Find all HealthManager components.
// 				bool includeInactive = true;
// 				for (i=0;i<allParents.Count;i++) {
// 					Component[] compArray =
// 						allParents[i].GetComponentsInChildren(
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
// 			Component[] compArray = allParents[i].GetComponentsInChildren(typeof(TargetIO),true); // find all SaveObject components, including inactive (hence the true here at the end)
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
// 
// 	public void NPCAudioOcclusion() {
// 		// Raytraced Audio Occlusion ;)
// 		int hitCount = 0;
// 		float newVolume = 1.0f;
// 		RaycastHit[] results = new RaycastHit[6];
// 		AIController aic = null;
// 		for (int i=0;i<healthObjectsRegistration.Length;i++) {
// 			if (healthObjectsRegistration[i] == null) continue;
// 
// 			aic = healthObjectsRegistration[i].GetComponent<AIController>();
// 			if (aic == null) continue;
// 			if (aic.SFX == null) continue;
// 			if (aic.index < Const.a.sfxSightSoundForNPC.Length && aic.index >= 0) {
// 				if (Const.a.sfxSightSoundForNPC[aic.index] < Const.a.sounds.Length && Const.a.sfxSightSoundForNPC[aic.index] >= 0) {
// 					if (aic.SFX.clip == Const.a.sounds[Const.a.sfxSightSoundForNPC[aic.index]]) {
// 						aic.SFX.volume = aic.normalVolume;
// 						continue;
// 					}
// 				}
// 			}
// 
// 			hitCount = RaycastNonAlloc(
// 						MouseLookScript.a.transform.position,
// 						aic.transform.position
// 						  - MouseLookScript.a.transform.position,
// 						results,32f,Const.a.layerMaskPlayerFrob,
// 						QueryTriggerInteraction.UseGlobal);
// 
// 			aic.SFX.volume = aic.normalVolume;
// 			if (hitCount > 0) {
// 				if (hitCount > 5) {
// 					newVolume = aic.normalVolume * 0.65f;
// 				} else if (hitCount == 5) {
// 					newVolume = aic.normalVolume * 0.70f;
// 				} else if (hitCount == 4) {
// 					newVolume = aic.normalVolume * 0.75f;
// 				} else if (hitCount == 3) {
// 					newVolume = aic.normalVolume * 0.85f;
// 				} else if (hitCount == 2) {
// 					newVolume = aic.normalVolume * 0.90f;
// 				} else {
// 					newVolume = aic.normalVolume * 0.95f;
// 				}
// 
// 				aic.SFX.volume = newVolume;
// 			}
// 		}
// 	}
// 
// 	public void RegisterObjectWithHealth(HealthManager hm) {
// 		if (hm == null) return;
// 
// 		for (int i=0;i<healthObjectsRegistration.Length;i++) {
// 			if (healthObjectsRegistration[i] != null) {
// 				if (healthObjectsRegistration[i] == hm) {
// 					return; // already in the list
// 				}
// 			}
// 		}
// 
// 		int len = healthObjectsRegistration.Length;
// 		for (int i=0;i<len;i++) {
// 			if (healthObjectsRegistration[i] == null) {
// 				healthObjectsRegistration[i] = hm;
// 				return;
// 			}
// 
// 			if (i == (len - 1)) {
// 				string msg = "WARNING: Could not register object with health. "
// 							 + " Hit limit of ";
// 
// 				UnityEngine.DualLog(msg + len.ToString());
// 			}
// 		}
// 	}

void UseTargets(UseData ud, uint16_t i, string targetname) {
// 		// Next check if targetname is valid.  This is fine if not, some
// 		// triggers we just want to play the trigger's SFX and do nothing else.
// 		if (data_parser_isspace(targetname)) return;
// 
// 		UseData tempUD = new UseData();
// 		float numtargetsfound = 0;
// 		// Find each gameobject with matching targetname in the register, then
// 		// call Use for each.
// 		bool succeeded = false;
// 		for (int i=0;i<TargetRegister.Count;i++) {
// 			if (TargetnameRegister.Count < 1) {
// 				UnityEngine.DualLogWarning("NO TARGETNAMES IN "
// 										   + "TargetnameRegister!!!");
// 				return;
// 			}
// 
// 			if (TargetnameRegister[i] != targetname) continue;
// 
// 			if (TargetRegister[i] != null) {
// 				numtargetsfound++;
// 				tempUD = ud;
// 				DualLog("Running targets for %s\n", targetname);
// 
// 				// Added activeSelf bit to keep from spamming SetActive
// 				// when running targets through a trigger_multiple
// 				if (tempUD.GOSetActive && !TargetRegister[i].activeSelf) {
// 					//UnityEngine.DualLog("GOSetActive on " + targetname);
// 					TargetRegister[i].SetActive(true);
// 					succeeded = true;
// 				}
// 
// 				// Diddo for activeSelf to prevent spamming SetActive.
// 				if (tempUD.GOSetDeactive && TargetRegister[i].activeSelf) {
// 					//UnityEngine.DualLog("GOSetDeactive on " + targetname);
// 					TargetRegister[i].SetActive(false);
// 					succeeded = true;
// 				}
// 
// 				if (tempUD.GOToggleActive) {
// 					// If I abuse this with a trigger_multiple someone should
// 					// shoot me.
// 					TargetRegister[i].SetActive(!TargetRegister[i].activeSelf);
// 					succeeded = true;
// 				}
// 
// 				TargetIO tio = TargetRegister[i].GetComponent<TargetIO>();
// 				tio.Targetted(tempUD);
// 				succeeded = true;
// 			}
// 		}
// 
// 		if (!succeeded) {
// 			UnityEngine.DualLogWarning("Failed to find a matching targetname"
// 										 + " for " + targetname);
// 		}
}
// 
// 	// Should ONLY come from a TargetIO
// 	public void AddToTargetRegister(TargetIO tio, GameObject go) {
// 		string tn = tio.targetname;
// 	    for (int i=0;i<TargetRegister.Count; i++) {
// 	        if (TargetRegister[i] == null) continue;
// 	        if (TargetRegister[i] != go) continue; // Key check for whole loop.
// 			
// 	        // GameObject go is in registry already
//             if (TargetnameRegister[i] == tn) {
//                 return; // Already in register, name and object.
//             } else {
//                 TargetnameRegister[i] = tn; // Fix up partial registry.
//                 return; // Ok it's good now.
//             }
// 	    }
// 	    
// 	    // GameObject isn't in registry, add fresh.
// 	    TargetRegister.Add(go);
// 		TargetnameRegister.Add(tn);
// 		lastTargetRegistrySize = TargetnameRegister.Count;
// 	}
// 
// 	public void AddToTextLocalizationRegister(TextLocalization txtloc) {
// 		if (txtloc == null) return;
// 
// 		TextLocalizationRegister.Add(txtloc);
// 	}
// 
// 	public void ReverbOn() {
// 		for (int i=0;i<ReverbRegister.Length;i++) {
// 			if (ReverbRegister[i] != null) {
// 				AudioReverbZone arz = ReverbRegister[i].GetComponent<AudioReverbZone>();
// 				if (arz != null) arz.enabled = true;
// 			}
// 		}
// 	}
// 
// 	public void ReverbOff() {
// 		for (int i=0;i<ReverbRegister.Length;i++) {
// 			if (ReverbRegister[i] != null) {
// 				AudioReverbZone arz = ReverbRegister[i].GetComponent<AudioReverbZone>();
// 				if (arz != null) arz.enabled = false;
// 			}
// 		}
// 	}
// 
// 	public void AddToReverbRegister (GameObject go) {
// 		for (int i=0;i<ReverbRegister.Length;i++) {
// 			if (ReverbRegister[i] == null) {
// 				ReverbRegister[i] = go;
// 				return; // Ok, gameobject added to the register.
// 			}
// 		}
// 	}
// 
// 	void Shake(bool effectIsWorldwide, float distance, float force) {
// 		if (distance == -1) distance = globalShakeDistance;
// 		if (force == -1) force = globalShakeForce;
// 
// 		if (effectIsWorldwide) {
// 			// The whole station is a shakin' and a movin'!
// 			MouseLookScript.a.ScreenShake(force,1f);
// 		} else {
// 			// check if player is close enough and shake em' up!
// 			if (distance_vector3(transform.position,player1Capsule.transform.position) < distance) {
// 				MouseLookScript.a.ScreenShake(force,1f);
// 			}
// 		}
// 	}
// }
// 
// 	// Action bits.  What do we want our target to do, e.g. turn on a light or close a door or activate force bridge
// 	// Using multiple bools to allow for multiple actions to be attempted on all the targets
// 	public bool tripTrigger; // force activate a trigger
// 	public bool doorOpen; // force opens the door
// 	public bool doorOpenIfUnlocked; // open a door only if it isn't locked
// 	public bool doorClose; // force closes the door
// 	public bool doorLock; // locks door
// 	public bool doorUnlock; // unlocks door
// 	public bool switchTrigger; // force use a switch
// 	public bool chargeStationRecharge; // force recharge a charging station
// 	public bool enemyAlert; // alert an enemy and pass owner as the new enemy
// 	public bool forceBridgeActivate; // activate a force bridge
// 	public bool forceBridgeDeactivate; // deactivate a force bridge
// 	public bool forceBridgeToggle; // toggle a force bridge
// 	public bool gravityLiftToggle; // activate a gravity lift
// 	public bool textureChangeToggle; // toggle a texture on something
// 	public bool lightOn; // turn on the light
// 	public bool lightOff; // turn out that light!
// 	public bool lightToggle; // flip the switch
// 	public bool funcwallMove; // target a moving wall
// 	public bool missionBitOn; // turn a mission quest bit on
// 	public bool missionBitOff; // turn a mission quest bit off, wait why?? because shield deactivated is possible
// 	public bool missionBitToggle; // toggle mission bit
// 	public bool sendEmail; // send all players an email
// 	public bool switchLockToggle; // toggle locked state of a ButtonSwitch
// 	public bool lockCodeToScreenMaterialChanger; // set the code on a screen after CPUs are destroyed
// 	public bool spawnerActivate; // activate a SpawnManager
// 	public bool spawnerActivateAlerted; // activate a SpawnManager and notify all enemies of the player's location
// 	public bool cyborgConversionToggle; // toggle cyborg conversion so player can respawn on current level
// 	public bool GOSetActive; // turn a gameObject on
// 	public bool GOSetDeactive; // turn a gameObject off
// 	public bool GOToggleActive; // toggle gameObject on/off
// 	public bool toggleRadiationTrigger; // toggle radiation on/off for a radiation trigger
// 	public bool disableThisGOOnAwake = false; // disable this gameobject so that it can be enabled later
// 	public bool toggleRelayEnabled; // toggle logic relay enabled state
// 	public bool togglePuzzlePanelLocked; // toggle whether a puzzle panel is locked or not
// 	public bool testQuestBitIsOn; // run target if a certain quest bit is on
// 	public bool testQuestBitIsOff; // run target if a certain quest bit is off
// 	public bool playSoundOnce; // play a sound effect
// 	public bool stopSound; // play a sound effect
// 	public bool sendSprintMessage; // sprint to the status bar
// 	public bool radiationTreatment; // flash radiation treatment static on player's screen who used the treatment
// 	public bool startFlashingMaterials; // enable flashing of materials blink blink blink blink blink!
// 	public bool stopFlashingMaterials; // disable flashing
// 	public bool unlockElevatorPad; // unlock elevator keypad
// 	public bool unlockKeycodePad; // unlock elevator keypad
// 	public bool unlockPuzzlePad; // unlock puzzle pad, grid or wire
// 	public bool screenShake; // shake the screen/earthquake
// 	public bool awakeSleepingEnemy; // awaken a sleeping enemy, e.g. the sec-2 bots that are in repair sleep on level 8
// 	public bool branchFlip; // flip logic_branchs
// 	public bool branchFlipOnly; // only flip the branch, not flip and fire
// 	public bool doorAccessCardOverrideToggle; // set that access card has already been used
// 	public bool unlockSwitch; // unlock a ButtonSwitch
// 	public bool lockElevatorPad; // lock elevator keypad
// 	public bool alreadyDisabledThisGOOnceEver = false;
// 	public bool doorToggle;
// 
// 	private UseData tempUD;
// 	private bool startInitialized = false;
// 
// 	private void Start() {
// 		RemoteStart(this.gameObject,"self Start()");
// 	}
// 	
// 	public void RemoteStart(GameObject sender,string sourcefunc) {
// 		if (!string.IsNullOrEmpty(targetname)) Const.a.AddToTargetRegister(this,gameObject); // Always, since on load we need to refill register.
// 		Initialize();
// 	}
// 	
// 	public void Initialize() {
// 		if (startInitialized) return;
// 
// 		if (data_parser_isspace(instances[i].targetname[0])) DualLogError("instance[%u] marked as disable on first load but has no targetname: %s!\n",i, instances[i].targetname);
//         if (disableThisGOOnAwake && !alreadyDisabledThisGOOnceEver) flag_set(&instances[i].entflags, ENTFLAG_ACTIVE, false);
// 		startInitialized = true;
// 	}
// 
// 	// Comes from Const.a.UseTargets - already checked that target matched
// 	// targetname of this interaction.
void Targetted(UseData ud, uint16_t i) {
    if (instances[i].index == 699 && (instances[i].entflags & ENTFLAG_ENABLED))) { // Whatever else happens, if a LogicRelay, keep the messages going
// 		if (delay <=0) RunTargets(ud);
// 		else { StartCoroutine(DelayedTarget(ud)); }
    }

//     GameEnd gend = GetComponent<GameEnd>();
//     if (gend != null) gend.Targetted(ud);
// 
//     // or a LogicBranch since it also carries logic along
//     if (!ud.branchFlipOnly) {
//         LogicBranch lb = GetComponent<LogicBranch>();
//         if (lb != null && lb.relayEnabled) lb.Targetted(ud);
//     }
//     if (ud.branchFlip || ud.branchFlipOnly) {
//         // or a LogicBranch since it also carries logic along
//         LogicBranch lbr = GetComponent<LogicBranch>();
//         if (lbr != null) lbr.FlipTrackSwitch();
//     }
// 
//     if (ud.tripTrigger) {
//         Trigger trig = GetComponent<Trigger>();
//         if (trig != null) trig.Targetted(ud);
// 
//         TriggerCounter trigcnt = GetComponent<TriggerCounter>();
//         if (trigcnt != null) trigcnt.Targetted(ud);
//     }
// 
//     if (ud.doorUnlock) {
//         Door dr = GetComponent<Door>();
//         if (dr != null) {
//             dr.Unlock();
//             dr.accessCardUsedByPlayer = true;
//         }
//     } // Unlock before open or toggle
//     
//     if (ud.doorOpen) {
//         Door dr = GetComponent<Door>();
//         if (dr != null) {
//             dr.ForceOpen();
//         }
//     }
//     
//     if (ud.doorOpenIfUnlocked) {
//         Door dr = GetComponent<Door>();
//         if (dr != null) {
//             if (!dr.locked
//                 && (dr.requiredAccessCard == AccessCardType.None
//                     || dr.accessCardUsedByPlayer
//                     || Inventory.a.HasAccessCard(dr.requiredAccessCard))) {
//                 
//                 dr.ForceOpen();
//             }
//         }
//     }
//     
//     if (ud.doorToggle) {
// // 			UnityEngine.DualLog("Attempting to toggle door's open/closed state on " + gameObject.name);
//         Door dr = GetComponent<Door>();
//         if (dr != null) {
//             DoorState drprev = dr.doorOpen;
//             dr.DoorActuate();
// // 				if (dr.doorOpen != drprev) UnityEngine.DualLog("Successfully to toggled door's open/closed state on " + gameObject.name);
//         }
//     }
//     
//     if (ud.doorClose) {
//         Door dr = GetComponent<Door>();
//         if (dr != null) dr.ForceClose();
//     }
//     
//     if (ud.doorLock) { // Lock after forcing door into a position.
//         Door dr = GetComponent<Door>();
//         if (dr != null) dr.Lock();
//     }
//     
// 
// 
//     if (ud.doorAccessCardOverrideToggle) {
//         Door dr = GetComponent<Door>();
//         if (dr != null) {
//             dr.accessCardUsedByPlayer = !dr.accessCardUsedByPlayer;
//         }
//     }
// 
//     if (ud.switchTrigger) {
//         ButtonSwitch bs = GetComponent<ButtonSwitch>();
//         if (bs != null) bs.Targetted(ud);
//     }
// 
//     if (ud.chargeStationRecharge) {
//         ChargeStation chst = GetComponent<ChargeStation>();
//         if (chst != null) chst.ForceRecharge();
//     }
// 
//     if (ud.enemyAlert) {
//         AIController aic = GetComponent<AIController>();
//         if (aic != null) aic.Alert(ud);
//     }
// 
//     if (ud.forceBridgeActivate) {
//         ForceBridge fb = GetComponent<ForceBridge>();
//         //DualLog("Activating force bridge");
//         if (fb != null) fb.Activate(false);
//     }
// 
//     if (ud.forceBridgeDeactivate) {
//         ForceBridge fb = GetComponent<ForceBridge>();
//         //DualLog("Deactivating force bridge");
//         if (fb != null) fb.Deactivate(false);
//     }
// 
//     if (ud.forceBridgeToggle) {
//         //DualLog("Toggling force bridge");
//         ForceBridge fb = GetComponent<ForceBridge>();
//         if (fb != null) fb.Toggle();
//     }
// 
//     if (ud.gravityLiftToggle) {
//         GravityLift gl = GetComponent<GravityLift>();
//         if (gl != null) gl.Toggle();
//     }
// 
//     if (ud.textureChangeToggle) {
//         TextureChanger tch = GetComponent<TextureChanger>();
//         if (tch != null) tch.Toggle();
//     }
// 
//     if (ud.lightOn) {
//         LightAnimation lam = GetComponent<LightAnimation>();
//         if (lam != null) lam.TurnOn();
//     }
// 
//     if (ud.lightOff) {
//         LightAnimation lam = GetComponent<LightAnimation>();
//         if (lam != null) lam.TurnOff();
//     }
// 
//     if (ud.lightToggle) {
//         LightAnimation lam = GetComponent<LightAnimation>();
//         if (lam != null) lam.Toggle();
//     }
// 
//     if (ud.funcwallMove) {
//         //DualLog("FuncWall move activated!");
//         FuncWall fw = GetComponent<FuncWall>();
//         if (fw != null) fw.Targetted(ud);
//     }
// 
//     if (ud.missionBitOn) {
//         QuestBitRelay qbr = GetComponent<QuestBitRelay>();
//         if (qbr != null) qbr.EnableBits();
//     }
// 
//     if (ud.missionBitOff) {
//         QuestBitRelay qbr = GetComponent<QuestBitRelay>();
//         if (qbr != null) qbr.DisableBits();
//     }
// 
//     if (ud.missionBitToggle) {
//         QuestBitRelay qbr = GetComponent<QuestBitRelay>();
//         if (qbr != null) qbr.ToggleBits();
//     }
// 
//     if (ud.sendEmail) {
//         //DualLog("sendEmail was true for Targetted() with targetname: " + targetname);
//         Email msg = GetComponent<Email>();
//         if (msg != null) {
//             //DualLog("sendEmail was true and msg was found for Targetted() with targetname: " + targetname);
//             msg.Targetted();
//         }
//     }
// 
//     if (ud.switchLockToggle) {
//         ButtonSwitch btsw = GetComponent<ButtonSwitch>();
//         if (btsw != null) btsw.ToggleLocked();
//     }
// 
//     if (ud.unlockSwitch) {
//         ButtonSwitch btsw = GetComponent<ButtonSwitch>();
//         if (btsw != null) btsw.locked = false;
//     }
// 
//     if (ud.spawnerActivate) {
//         SpawnManager spwnmgr = GetComponent<SpawnManager>();
//         if (spwnmgr != null) spwnmgr.Activate(false);
//     }
// 
//     if (ud.spawnerActivateAlerted) {
//         SpawnManager spwnmgr = GetComponent<SpawnManager>();
//         if (spwnmgr != null) spwnmgr.Activate(true);
//     }
// 
//     if (ud.cyborgConversionToggle) {
//         LevelManager.a.CyborgConversionToggleForCurrentLevel();
//         CyborgConversionToggle cctog = GetComponent<CyborgConversionToggle>();
//         if (cctog != null) cctog.PlayVoxMessage();
//     }
// 
//     if (ud.toggleRadiationTrigger) {
//         Radiation rad = GetComponent<Radiation>();
//         if (rad != null) rad.enabled = !rad.enabled;
//     }
// 
//     if (ud.toggleRelayEnabled) {
//         LogicRelay logrel = GetComponent<LogicRelay>();
//         if (logrel != null) logrel.relayEnabled = !logrel.relayEnabled;
//     }
// 
//     if (ud.togglePuzzlePanelLocked) {
//         PuzzleGridPuzzle pgp = GetComponent<PuzzleGridPuzzle>();
//         if (pgp != null) pgp.locked = !pgp.locked;
// 
//         PuzzleWirePuzzle pwp = GetComponent<PuzzleWirePuzzle>();
//         if (pwp != null) pwp.locked = !pwp.locked;
//     }
// 
//     if (ud.testQuestBitIsOn) {
//         QuestBitRelay qbr = GetComponent<QuestBitRelay>();
//         if (qbr != null) qbr.TestBits(true,ud,this);
//     }
// 
//     if (ud.testQuestBitIsOff) {
//         QuestBitRelay qbr = GetComponent<QuestBitRelay>();
//         if (qbr != null) qbr.TestBits(false,ud,this);
//     }
// 
//     if (ud.playSoundOnce) {
//         PlaySoundTriggered pst = GetComponent<PlaySoundTriggered>();
//         if (pst != null) pst.PlaySoundEffect();
//     }
// 
//     if (ud.stopSound) {
//         PlaySoundTriggered pst = GetComponent<PlaySoundTriggered>();
//         if (pst != null) pst.StopSoundEffect();
//     }
// 
//     if (ud.sendSprintMessage) {
//         TriggeredSprintMessage tsm = GetComponent<TriggeredSprintMessage>();
//         if (tsm != null) Const.sprint(tsm.messageToDisplay);
//     }
// 
//     if (ud.radiationTreatment) {
//         if (PlayerReferenceManager.a != null) {
//             PlayerReferenceManager.a.playerRadiationTreatmentFlash.SetActive(true);
//             PlayerHealth.a.radiated = 0;
//         }
//     }
// 
//     if (ud.startFlashingMaterials) {
//         MaterialFlash mflash = GetComponent<MaterialFlash>();
//         if (mflash != null) mflash.StartFlashing();
//     }
// 
//     if (ud.stopFlashingMaterials) {
//         MaterialFlash mflash = GetComponent<MaterialFlash>();
//         if (mflash != null) mflash.StopFlashing();
//     }
// 
//     if (ud.unlockElevatorPad) {
//         KeypadElevator kelv = GetComponent<KeypadElevator>();
//         if (kelv != null) kelv.locked = false;
//     }
// 
//     if (ud.unlockKeycodePad) {
//         KeypadKeycode keyk = GetComponent<KeypadKeycode>();
//         if (keyk != null) keyk.locked = false;
//     }
// 
//     if (ud.unlockPuzzlePad) {
//         PuzzleGridPuzzle pgp = GetComponent<PuzzleGridPuzzle>();
//         if (pgp != null) pgp.locked = false;
// 
//         PuzzleWirePuzzle pwp = GetComponent<PuzzleWirePuzzle>();
//         if (pwp != null) pwp.locked = false;
//     }
// 
//     if (ud.screenShake) {
//         EffectScreenShake efsh = GetComponent<EffectScreenShake>();
//         if (efsh != null) efsh.Shake();
//     }
// 
//     if (ud.awakeSleepingEnemy) {
//         if (ConstIndexIsNPC(instances[i].index)) {
//             instances[i].asleep = false;
//             uint16_t cables = instances[i].sleepingCables;
//             if (cables != UINT16_MAX && cables != PLAYER1 && cables != PLAYER2) DeleteInstance(cables);
//     }
// 
//     if (ud.lockElevatorPad) {
//         KeypadElevator kelv = GetComponent<KeypadElevator>();
//         if (kelv != null) kelv.locked = true;
//     }
}
