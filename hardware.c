// hardware.c - Hardware behavior for cybernetic enhancements in game
#include "voxen.h"
// Hw referenceIndex, ref14Index, button index
// Bio 27,6, 0
// Sen 24,3, 1
// Lan 28,7, 2
// Shi 26,5, 3
// Nig 32,11,4
// Ere 23,2, 5
// Boo 30,9, 6
// Jum 31,10,7

private float brightness = 0f;
private const float lanternVersion1Brightness = 2.5f;
private const float lanternVersion2Brightness = 4;
private const float lanternVersion3Brightness = 5;

void Awake () {
    SFX = GetComponent<AudioSource>();
    gsc = playerCamera.GetComponent<Grayscale>();
    gscSensaCenter = sensaroundCenterCamera.GetComponent<Grayscale>();
    gscSensaLH = sensaroundLHCamera.GetComponent<Grayscale>();
    gscSensaRH = sensaroundRHCamera.GetComponent<Grayscale>();
}

// 0 = bio, 1 = sen, 2 = lan, 3 = shi, 4 = nig, 5 = ere, 6 = boo, 7 = jum
// verz must come from inventoryPlayer1.hardwareVersionSetting[] as this value has already subtracted 1 since the version number on prefabs is 1 based but the one needed for images is 0 based.
public void SetVersionIconForButton(bool isOn, int verz, int button8Index) {
// 		DualLog("SetVersionIconForButton with version " + verz.ToString() + ", and button8Index of " + button8Index.ToString());
    if (button8Index < 0 || button8Index > 7) button8Index = 0;
    if (isOn) {
        switch (verz) {
        case 0:
            buttons[button8Index].image.overrideSprite = buttonActive1[button8Index];
            break;
        case 1:
            buttons[button8Index].image.overrideSprite = buttonActive2[button8Index];
            break;
        case 2:
            buttons[button8Index].image.overrideSprite = buttonActive3[button8Index];
            break;
        case 3:
            buttons[button8Index].image.overrideSprite = buttonActive4[button8Index];
            break;
        default:
            buttons[button8Index].image.overrideSprite = buttonActive4[button8Index];
            break;
        }
    } else {
        buttons[button8Index].image.overrideSprite = buttonDeactive[button8Index];
    }
}

public void BioClick() {
    Sys_UI.mouseClickHeldOverGUI = true;
    BioAction();
}

public void BioAction() {
    if (inventoryPlayer1.BioMonitorVersion() == 0 && PlayerEnergy.a.energy <= 0) {
        CenterStatusPrint("%s", Sys_Text.stringTable[314],inventoryPlayer1.owner);
        return;
    }

    Utils.PlayUIOneShotSavable(78);
    if (inventoryPlayer1.BioMonitorActive()) {
        BioOff();
    } else {
        BioOn();
    }
}

// Called by PlayerEnergy when exhausted energy to 0 so mustn't play sound.
public void BioOff() {
    inventoryPlayer1.hardwareIsActive[6] = false;
    SetVersionIconForButton(inventoryPlayer1.hardwareIsActive[6],inventoryPlayer1.hardwareVersionSetting[6],0);
    
    if (Sys_Cheats.showFPS) return;
    if (BiomonitorGraphSystem.a != null) {
        BiomonitorGraphSystem.a.ClearGraphs();
    }

    Utils.Deactivate(bioMonitorContainer);
}

public void BioOn() {
    inventoryPlayer1.hardwareIsActive[6] = true;
    SetVersionIconForButton(inventoryPlayer1.BioMonitorActive(),inventoryPlayer1.hardwareVersionSetting[6],0);
    Utils.Activate(bioMonitorContainer);
}

public void ActivateSensaroundCenter() {
    Sys_UI.DisableAllCenterTabs();
    Utils.Activate(sensaroundCenterCamera);
    Utils.Activate(sensaroundCenter);
}

public void ActivateSensaroundSides() {
    Sys_UI.TabReset(true); // right
    Sys_UI.TabReset(false); // left
    if (sensaroundLHCamera != null) sensaroundLHCamera.SetActive (true);
    if (sensaroundLH != null) sensaroundLH.SetActive (true);
    if (sensaroundRHCamera != null) sensaroundRHCamera.SetActive (true);
    if (sensaroundRH != null) sensaroundRH.SetActive (true);
}

public void HideSensaround() {
    if (sensaroundCenterCamera != null) sensaroundCenterCamera.SetActive(false);
    if (sensaroundCenter != null) sensaroundCenter.SetActive(false);
    if (sensaroundLHCamera != null) sensaroundLHCamera.SetActive(false);
    if (sensaroundLH != null) sensaroundLH.SetActive(false);
    if (sensaroundRHCamera != null) sensaroundRHCamera.SetActive(false);
    if (sensaroundRH != null) sensaroundRH.SetActive(false);
}

public void UnhideSensaround() {
    if (!inventoryPlayer1.hardwareIsActive[3]) return;
    
    if (inventoryPlayer1.hardwareVersion[3] == 1) {
        ActivateSensaroundCenter(); // Only center on version 1.
    } else {
        ActivateSensaroundCenter();
        ActivateSensaroundSides();
    }
}

public void DeactivateSensaroundCameras() {
    HideSensaround();
    Sys_UI.CenterTabButtonClickSilent(Sys_UI.curCenterTab,true);
    Sys_UI.TabReset(true); // right
    Sys_UI.TabReset(false); // left
    Sys_UI.ReturnToLastTab(true);
    Sys_UI.ReturnToLastTab(false);
}

public void SensaroundOn() {
    inventoryPlayer1.hardwareIsActive[3] = true;
    SetVersionIconForButton(inventoryPlayer1.hardwareIsActive[3], inventoryPlayer1.hardwareVersionSetting[3],1);
    UnhideSensaround();
}

public void SensaroundClick() {
    Sys_UI.mouseClickHeldOverGUI = true;
    SensaroundAction();
}

public void SensaroundAction() {
    if (PlayerEnergy.a.energy <=0) { CenterStatusPrint("%s", Sys_Text.stringTable[314],inventoryPlayer1.owner); return; }

    if (inventoryPlayer1.hardwareIsActive[3]) {
        Utils.PlayUIOneShotSavable(82);
        SensaroundOff();
    } else {
        Utils.PlayUIOneShotSavable(93);
        SensaroundOn();
    }
}

// called by PlayerEnergy when exhausted energy to 0
public void SensaroundOff() {
    inventoryPlayer1.hardwareIsActive[3] = false;
    SetVersionIconForButton(inventoryPlayer1.hardwareIsActive[3],inventoryPlayer1.hardwareVersionSetting[3],1);
    DeactivateSensaroundCameras();
}

public void ShieldClick() {
    Sys_UI.mouseClickHeldOverGUI = true;
    ShieldAction();
}

public void ShieldOff() {
    inventoryPlayer1.hardwareIsActive[5] = false;
    SetVersionIconForButton(inventoryPlayer1.hardwareIsActive[5],inventoryPlayer1.hardwareVersionSetting[5],3);
}

public void ShieldOn() {
    inventoryPlayer1.hardwareIsActive[5] = true;
    SetVersionIconForButton(inventoryPlayer1.hardwareIsActive[5],inventoryPlayer1.hardwareVersionSetting[5],3);
}

public void ShieldAction() {
    if (PlayerEnergy.a.energy <=0) { CenterStatusPrint("%s", Sys_Text.stringTable[314],inventoryPlayer1.owner); return; }
    if (inventoryPlayer1.hardwareIsActive[5]) {
        Utils.PlayUIOneShotSavable(95);
        ShieldOffWithEffects();
    } else {
        Utils.PlayUIOneShotSavable(96);
        ShieldDeactivateFX.SetActive(false);
        ShieldActivateFX.SetActive(true);
        ShieldOn();
        
    }
}

// Called by PlayerEnergy when exhausted energy to 0.
public void ShieldOffWithEffects() {
    ShieldOff();
    ShieldDeactivateFX.SetActive(true);
    ShieldActivateFX.SetActive(false);
}

void LanternClick() {
    Sys_UI.mouseClickHeldOverGUI = true;
    LanternAction();
}

void LanternAction() {
    if (PlayerEnergy.a.energy <=0) { CenterStatusPrint("%s", Sys_Text.stringTable[314],inventoryPlayer1.owner); return; }
    Utils.PlayUIOneShotSavable(78);
    if (inventoryPlayer1.hardwareIsActive[7]) {
        LanternOff();
    } else {
        LanternOn();
    }
}

void LanternOn() {
    inventoryPlayer1.hardwareIsActive[7] = true;
    SetVersionIconForButton(inventoryPlayer1.LanternActive(), inventoryPlayer1.hardwareVersionSetting[7],2);

    // Figure out which brightness setting to use depending on version.
    switch(inventoryPlayer1.hardwareVersionSetting[7]) {
        case 0: brightness = lanternVersion1Brightness; break;
        case 1: brightness = lanternVersion2Brightness; break;
        case 2: brightness = lanternVersion3Brightness; break;
        default: brightness = 0.0f; break;
    }

    Utils.EnableLight(headlight);
    headlight.intensity = brightness; // Set the light intensity per version.
}

// Called by PlayerEnergy when exhausted energy to 0.
void LanternOff() {
    inventoryPlayer1.hardwareIsActive[7] = false;
    SetVersionIconForButton(inventoryPlayer1.LanternActive(), inventoryPlayer1.hardwareVersionSetting[7],2);
    Utils.DisableLight(headlight);
    headlight.intensity = 0.0f; // Turn the light off.
}

void InfraredClick() {
    Sys_UI.mouseClickHeldOverGUI = true;
    InfraredAction();
}

void InfraredAction() {
    if (PlayerEnergy.a.energy <=0) { CenterStatusPrint("%s", Sys_Text.stringTable[314],inventoryPlayer1.owner); return; }
    if (inventoryPlayer1.hardwareIsActive[11]) {
        Utils.PlayUIOneShotSavable(82);
    } else {
        Utils.PlayUIOneShotSavable(98);
    }
    inventoryPlayer1.hardwareIsActive[11] = !inventoryPlayer1.hardwareIsActive[11];
    SetVersionIconForButton(inventoryPlayer1.hardwareIsActive[11], inventoryPlayer1.hardwareVersionSetting[11],4);
    if (inventoryPlayer1.hardwareIsActive[11]) {
        InfraredOn();
    } else {
        InfraredOff();
    }
}

void InfraredOn() {
    Utils.EnableLight(infraredLight);
    Utils.EnableGrayscale(gsc);
    Utils.EnableGrayscale(gscSensaCenter);
    Utils.EnableGrayscale(gscSensaLH);
    Utils.EnableGrayscale(gscSensaRH);
}

// called by PlayerMovement when exhausted energy to < 11f
void InfraredOff() {
    inventoryPlayer1.hardwareIsActive[11] = false;
    Utils.DisableLight(infraredLight);
    Utils.DisableGrayscale(gsc);
    Utils.DisableGrayscale(gscSensaCenter);
    Utils.DisableGrayscale(gscSensaLH);
    Utils.DisableGrayscale(gscSensaRH);
    SetVersionIconForButton(false,inventoryPlayer1.hardwareVersionSetting[11],4);
}

void EReaderClick () {
    Sys_UI.mouseClickHeldOverGUI = true;
    EReaderAction();
}

void EReaderOn() {
    inventoryPlayer1.hardwareIsActive[2] = true;
    Sys_UI.OpenEReaderInItemsTab();
}

void EReaderAction() {
    Utils.PlayUIOneShotSavable(97);
    EReaderOn();
}


void BoosterClick() {
    Sys_UI.mouseClickHeldOverGUI = true;
    BoosterAction();
}

void BoosterAction() {
    if (inventoryPlayer1.BoosterSetToBoost() && PlayerEnergy.a.energy <= 0) {
        CenterStatusPrint("%s", Sys_Text.stringTable[314],inventoryPlayer1.owner);
        return;
    }

    Utils.PlayUIOneShotSavable(78);
    if (inventoryPlayer1.hardwareIsActive[9]) {
        BoosterOff();
    } else {
        BoosterOn();
    }
}

void BoosterOn() {
    inventoryPlayer1.hardwareIsActive[9] = true;
    SetVersionIconForButton(inventoryPlayer1.hardwareIsActive[9],inventoryPlayer1.hardwareVersionSetting[9],6);
}

// called by PlayerMovement when exhausted energy to < 11f
void BoosterOff() {
    inventoryPlayer1.hardwareIsActive[9] = false;
    SetVersionIconForButton(inventoryPlayer1.hardwareIsActive[9],inventoryPlayer1.hardwareVersionSetting[9],6);
}

void JumpJetsClick() {
    Sys_UI.mouseClickHeldOverGUI = true;
    JumpJetsAction();
}

void JumpJetsAction() {
    if (PlayerEnergy.a.energy <= 0) {
        CenterStatusPrint("%s", Sys_Text.stringTable[314],inventoryPlayer1.owner);
        return;
    }

    Utils.PlayUIOneShotSavable(78);
    inventoryPlayer1.JumpJetsToggle();
    if (inventoryPlayer1.JumpJetsActive()) {
        JumpJetsOn();
    } else {
        JumpJetsOff();
    }
}

void JumpJetsOn() {
    inventoryPlayer1.hardwareIsActive[10] = true;
    SetVersionIconForButton(inventoryPlayer1.JumpJetsActive(),inventoryPlayer1.hardwareVersionSetting[10],7);
}

// called by PlayerMovement when exhausted energy to < 11f
void JumpJetsOff() {
    inventoryPlayer1.hardwareIsActive[10] = false;
    SetVersionIconForButton(inventoryPlayer1.JumpJetsActive(),inventoryPlayer1.hardwareVersionSetting[10],7);
}
