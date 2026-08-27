// citadel.c - Game logic.
#include "common.h"
extern void PainStaticFlash(float intensity); extern void EmpStaticFlash(float intensity); extern void BiomonitorEnergyPulse(float take); extern void BioMonitorClearGraphs(void);
__attribute__((used)) AutoSplitterData autoSplitter = {0x1337133713371337,0,false,0};
V3 ScreenPointToRay(V3 fwd, V3 rt) {
    float tanFov = vtan((float)Sys_Settings.FOV * 0.5f * PI / 180.0f), ndcX = ((World.inventoryMode ? World.cursorPos_x : 683.0f) - 683.0f) / 384.0f, ndcY = -((World.inventoryMode ? World.cursorPos_y : 384.0f) - 384.0f) / 384.0f;
    V3 view = V3_Normalize((V3){ndcX * tanFov,ndcY * tanFov,-1.0f}), flipForward = (V3){-fwd.x,-fwd.y,-fwd.z}; V3 up = V3_Normalize(V3_Cross(rt,flipForward));
    return (V3){view.x*rt.x + view.y*up.x + view.z*flipForward.x,view.x*rt.y + view.y*up.y + view.z*flipForward.y,view.x*rt.z + view.y*up.z + view.z*flipForward.z};
}

void ResetHeldItem() { World.invP1.heldObjectIndex=World.invP1.heldObjectCustIdx=U16_MAX; World.invP1.heldObjectAmmo=World.invP1.heldObjectAmmo2=0; World.invP1.heldObjectLoadedAlternate=World.invP1.holdingObject=World.invP1.grenActive=false; }
void DropHeldItem() {
    if (World.invP1.heldObjectIndex >= World.instCount) { ResetHeldItem(); return; }    if (World.invP1.dropFinished > World.pauseRelativeTime) {return;}
    World.invP1.dropFinished = World.pauseRelativeTime + 0.2; // Prevent immediate re-grab at high fps
    u16 n = AddInstance(World.invP1.heldObjectIndex,World.position[PLAYER1]);
    Entity* e = &World.instances[n]; e->usableCustIdx = World.invP1.heldObjectCustIdx; e->ammo = World.invP1.heldObjectAmmo; e->ammo2 = World.invP1.heldObjectAmmo2; e->heldObjectLoadedAlternate = World.invP1.heldObjectLoadedAlternate;
    flag_set(&e->entflags,EF_RIGIDBODY,true); 
    V3 tossDir = ScreenPointToRay(World.instances[PLAYER1].forward,World.instances[PLAYER1].right); World.position[n] = V3_AplusB(World.position[PLAYER1],V3_ScaleByF(tossDir,0.48f)); World.velocity[n] = V3_ScaleByF(tossDir,10.0f);
    ResetHeldItem();
}

static const u16 patchMsg[7] = {325,326,327,328,329,330,331};
void PatchUse(int patchSlot) {
    if (patchSlot < 0 || patchSlot > 6) return; if (World.invP1.patchCounts[patchSlot] <= 0) { CenterStatusPrint("%s", Sys_Text.stringTable[324]); return; } World.invP1.patchCounts[patchSlot]--;
    World.invP1.patchActive |= (u16)(1u << patchSlot);
    switch (patchSlot) {
        case 0: if(World.invP1.berserkFinished > World.pauseRelativeTime){World.invP1.berserkFinished += BERSERK_TIME;}
                else{World.invP1.berserkFinished = World.pauseRelativeTime + BERSERK_TIME; World.invP1.berserkIncTime = World.pauseRelativeTime + (BERSERK_TIME / 5.0); World.invP1.berserkIncrement = 0;} break;
        case 1: World.invP1.detoxFinished        = World.pauseRelativeTime + DETOX_TIME; World.invP1.radiated = 0.0f; break;
        case 2: World.invP1.geniusFinished       = World.pauseRelativeTime + GENIUS_TIME; World.geniusActive = true; break;
        case 3: World.invP1.mediFinished         = World.pauseRelativeTime + MEDI_TIME; break;
        case 4: World.invP1.reflexFinishedTime   = World.absoluteTime + REFLEX_TIME; World.timeScale = REFLEX_TIME_SCALE; break; // TODO Handle restoring offset from absolute time at loading savegame
        case 5: World.invP1.sightFinishedTime    = World.pauseRelativeTime + SIGHT_TIME; World.invP1.sightSideEffectFinishedTime = -1.0; break;
        case 6: World.invP1.staminupFinishedTime = World.pauseRelativeTime + STAMINUP_TIME; World.invP1.staminupActive = true; World.invP1.fatigue = 0.0f; break;
    }
    CenterStatusPrint("%s",Sys_Text.stringTable[patchMsg[patchSlot]]); if (World.invP1.patchCounts[World.invP1.patchCur] <= 0) { for (int i = 0; i < 7; i++) { if (World.invP1.patchCounts[i] > 0) { World.invP1.patchCur = (i8)i; break; } } }
    play_wav(sounds[88],SfxVol(),(V3){0.0f,0.0f,0.0f},false);
}

void WeaponFireStartWeaponDip(float t) { (void)t; if (t <= 0.0f) { World.invP1.weaponDipLerp = 0.0f; World.invP1.weaponDipFinished = 0.0; return; } World.invP1.weaponDipFinished = World.pauseRelativeTime + (double)t; World.invP1.weaponDipLerp = 1.0f; }
void WeaponFireCompleteWeaponChange(void) { World.invP1.justChangedWeap = false; World.invP1.weaponCurrentPending = -1; World.invP1.weaponIndexPending = -1; World.invP1.recoiling = false; }
bool InventoryHasAccessCard(AccCardType card) { return (World.invP1.accessCardOwned & (1u << card)) != 0; }
bool InventoryHasAnyAccessCards() { return World.invP1.accessCardOwned != 0; }
const char* AccessCardCodeForType(AccCardType a) { // Called by ItemTabManager
    switch(a) {
        case ACC_Std: return "STD";     case ACC_Med: return "MED";     case ACC_Sci: return "SCI"; case ACC_Admin:return "ADM"; case ACC_Grp1: return "Group-1"; case ACC_Grp2:return "Group-2"; case ACC_Grp3:return "Group-3"; case ACC_Grp4:return "Group-4";
        case ACC_GrpA:return "Group-A"; case ACC_GrpB:return "Group-B"; case ACC_Stor:return "STO"; case ACC_Eng:  return "ENG"; case ACC_Maint:return "MTN";     case ACC_Security:return "SEC"; case ACC_Per1:return "PER-1";   case ACC_Per2:return "PER-2";
        case ACC_Per3:return "PER-3";   case ACC_Per4:return "PER-4";   case ACC_Per5:return "PER-5";
    } return "Group-2";
}

void AddAccessCardToInventory(int index) {
    AccCardType card;
    switch(index) {
        case 34:card=ACC_Admin; break; case 81:card=ACC_Std;  break; case 83:card=ACC_Grp1; break; case  84:card=ACC_Sci;  break; case 85:card=ACC_Eng; break; case 86:card=ACC_GrpB; break; case 87:card=ACC_Security; break; case 88:card=ACC_Per5; break; 
        case 89:card=ACC_Med;   break; case 90:card=ACC_Grp3; break; case 91:card=ACC_Grp4; break; case 110:card=ACC_Per1; break;
        default: CenterStatusPrint("BUG: Unmarked access card, defaulting to STD."); card = ACC_Std; break;
    }
    if (index == 87) { // Command card = STO + SEC + MTN
        if (InventoryHasAccessCard(ACC_Stor) && InventoryHasAccessCard(ACC_Security) && InventoryHasAccessCard(ACC_Maint)) { CenterStatusPrint("%s%s",Sys_Text.stringTable[44],AccessCardCodeForType(card)); return; }
        World.invP1.accessCardOwned |= (1u<<ACC_Stor)|(1u<<ACC_Security)|(1u<<ACC_Maint); CenterStatusPrint("%s%s, %s, %s",Sys_Text.stringTable[45],AccessCardCodeForType(ACC_Stor),AccessCardCodeForType(ACC_Security),AccessCardCodeForType(ACC_Maint));
        return;
    }
    if (InventoryHasAccessCard(card)) { CenterStatusPrint("%s%s",Sys_Text.stringTable[44],AccessCardCodeForType(card)); return; }
    World.invP1.accessCardOwned |= (1u << card); CenterStatusPrint("%s%s",Sys_Text.stringTable[45],AccessCardCodeForType(card));
}

void AddHardwareToInventory(int index,int hwversion,bool overt) {
    if (hwversion > 0 && hwversion <= (int)World.invP1.hardwareVersion[index]) { if(overt){CenterStatusPrint("%s",Sys_Text.stringTable[46]);/*THAT WARE IS OBSOLETE. DISCARDED.*/} return; }
    static const u8 textIdx[12] = {21,22,23,24,25,26,27,28,29,30,31,32};
    World.invP1.hardwareInvIndex = index; World.invP1.hasHardware |= (u16)(1u << index); World.invP1.hardwareVersion[index] = (u8)hwversion; World.invP1.hardwareVersionSetting[index]= hwversion > 0 ? (u8)(hwversion - 1) : 0;
    if (overt) CenterStatusPrint("%s v%d",Sys_Text.stringTable[textIdx[index] + 326],hwversion);
}

bool AddGeneralObjectToInventory(int index, int custIdx) {
    for (i8 i=1;i<14;++i) {
        if (World.invP1.generalInventoryIndexRef[i] == -1) { 
            if(!InventoryHasAnyAccessCards() && World.invP1.generalInvCurrent == 0){World.invP1.generalInvCurrent=i;} World.invP1.generalInventoryIndexRef[i]=index; 
            World.invP1.generalInvCustIdx[i]=(i16)custIdx; CenterStatusPrint("%s%s",Sys_Text.stringTable[index + 326],Sys_Text.stringTable[31]);
            return true;
        }
    } return false;
}

void CheckForUnreadLogs() { int e=0,l=0; for (int i=0;i<LOGCNT;++i) if (World.invP1.hasLog[i] && !World.invP1.readLog[i]) *(Sys_Text.audioLogType[i] == AudioLogType_Email ? &e : &l)=1; World.invP1.hasNewEmail=e; World.invP1.hasNewLogs=l; }
static int FindNextUnreadLog() { for (int i = LOGCNT-1; i >= 0; i--) { if(World.invP1.hasLog[i] && !World.invP1.readLog[i]){return i;} } return -1; }
static void PlayLog(int logIndex) {
    if(logIndex < 0 || logIndex >= LOGCNT || !(World.invP1.hasHardware & HW_ERD)){return;} play_message(AudioLogPath(logIndex)); World.invP1.readLog[logIndex]=true;
    if (Sys_Text.audioLogType[logIndex] == AudioLogType_Vmail) {
        World.Sys_UI.vmailActive        = true;
//         World.invP1.vmailLogIndex = (i16)logIndex;
//         switch (logIndex) { // TODO
//             case 119: vmailbetajet.SetActive(true); fileName = "betajet.webm"; break;     case 116: vmailbridgesep.SetActive(true); fileName = "bridgesep.webm"; break; case 117: vmailcitadestruct.SetActive(true); fileName = "citadestruct.webm"; break;
//             case 110: vmailgenstatus.SetActive(true); fileName = "genstatus.webm"; break; case 114: vmaillaserdest.SetActive(true); fileName = "laserdest.webm"; break; case 120: vmailshieldsup.SetActive(true); fileName = "shieldsup.webm"; break;
//         }
    }
    CenterStatusPrint("%s%s",Sys_Text.stringTable[1020],World.audiologNames[logIndex]);
}

void PlayLastAddedLog(int logIndex) { if(logIndex < 0){return;} PlayLog(logIndex); World.invP1.lastAddedIndex = -1; }
void AddAudioLogToInventory(int index) {
    if (index < 0) { DualLog("BUG: Audio log picked up has no assigned index (-1)"); return; }
    if (index == 128) { CenterStatusPrint("%s",Sys_Text.stringTable[309]); return; } // Trioptimum Funpack
    World.invP1.hasLog[index]  = true;
    World.invP1.lastAddedIndex = index;
    World.invP1.numLogsFromLevel[Sys_Text.audioLogLevelFound[index]]++;
    if      (Sys_Text.audioLogType[index] == AudioLogType_Email)  World.invP1.hasNewEmail = true;
    else if (Sys_Text.audioLogType[index] == AudioLogType_Normal) World.invP1.hasNewLogs  = true;
    if (World.invP1.hasHardware & HW_ERD) { CenterStatusPrint("%s%s%s",Sys_Text.stringTable[36],World.audiologNames[index],Sys_Text.stringTable[38]); } // "Audio log <name> picked up. Press <key> to play." — TODO: key binding name interp
    else { CenterStatusPrint("%s%s%s",Sys_Text.stringTable[36],World.audiologNames[index],Sys_Text.stringTable[310]); }
}

static inline void ItemAdd(u8 *cur, u8 *counts, int idx, int uIdx, int sysIdx) { if (!counts[*cur]) {*cur=(i8)idx;} counts[idx]++; CenterStatusPrint("%s%s", Sys_Text.stringTable[uIdx + 326], Sys_Text.stringTable[sysIdx]); }
void AddGrenadeToInventory(int i, int u) { World.invP1.grenConstIndex[i]=(i16)u; ItemAdd(&World.invP1.grenCur,World.invP1.grenAmmo,i,u,34); }
void   AddPatchToInventory(int i, int u) { if (i >= 0) ItemAdd(&World.invP1.patchCur,World.invP1.patchCounts,i,u,35); }
static inline void GrenadeCycle(int step){int cur= World.invP1.grenCur, next=cur; for(int i=0;i<7;++i){next=(next+step+7)%7; if(   World.invP1.grenAmmo[next]>0){World.invP1.grenCur =(i8)next; CenterStatusPrint("%s",Sys_Text.stringTable[579+next]); return;}}}
static inline void   PatchCycle(int step){int cur=World.invP1.patchCur, next=cur; for(int i=0;i<7;++i){next=(next+step+7)%7; if(World.invP1.patchCounts[next]>0){World.invP1.patchCur=(i8)next; CenterStatusPrint("%s",Sys_Text.stringTable[579+next]); return;}}}
void RemoveGrenade(int i) { if(World.invP1.grenAmmo[i] > 0){World.invP1.grenAmmo[i]--;} if(!World.invP1.grenAmmo[i]){GrenadeCycle(-1);} }
static i8 GetExistingCyberItemIndex() { if (World.invP1.softVersions[SW_TURBO]  > 0) {return 0;} if (World.invP1.softVersions[SW_DECOY]  > 0) {return 1;} if (World.invP1.softVersions[SW_RECALL] > 0) {return 2;} return -1; }
static void UseTurbo() {
    if (World.invP1.softVersions[SW_TURBO] <= 0) { World.invP1.hasSoft &= (u8)~(1u << SW_TURBO); return; }
    if (--World.invP1.softVersions[SW_TURBO] == 0) World.invP1.hasSoft &= (u8)~(1u << SW_TURBO);
    if(World.invP1.turboFinished > World.pauseRelativeTime){World.invP1.turboFinished+=World.invP1.turboCyberTime;}else{World.invP1.turboFinished=World.invP1.turboCyberTime + World.pauseRelativeTime;}
}

static void UseDecoy() {
    if (World.decoyActive) { CenterStatusPrint("%s",Sys_Text.stringTable[537]); return; }
    if (World.invP1.softVersions[SW_DECOY] <= 0) { World.invP1.hasSoft &= (u8)~(1u << SW_DECOY); return; }
    if (--World.invP1.softVersions[SW_DECOY] == 0) World.invP1.hasSoft &= (u8)~(1u << SW_DECOY);
    u16 decoyIdx = SpawnDynamicObject(417,true); // 417 = CyberDecoy constIndex
    if (decoyIdx != U16_MAX) {World.position[decoyIdx] = World.position[PLAYER1];}
}

static void UseRecall() { if (World.invP1.softVersions[SW_RECALL] <= 0) {return;} if (--World.invP1.softVersions[SW_RECALL] == 0) {World.invP1.hasSoft &= (u8)~(1u << SW_RECALL);} World.position[PLAYER1] = World.cyberspaceRecallPoint; }
void UseCyberspaceItem() {
    if (World.invP1.cyberItemIndex <= 0) { World.invP1.cyberItemIndex = GetExistingCyberItemIndex(); if (World.invP1.cyberItemIndex < 0) { CenterStatusPrint("%s",Sys_Text.stringTable[473]); return; } }
    switch(World.invP1.cyberItemIndex) {
        case 0: if (!World.invP1.softVersions[SW_TURBO])  { World.invP1.cyberItemIndex = GetExistingCyberItemIndex(); return; } UseTurbo();  break;
        case 1: if (!World.invP1.softVersions[SW_DECOY])  { World.invP1.cyberItemIndex = GetExistingCyberItemIndex(); return; } UseDecoy();  break;
        case 2: if (!World.invP1.softVersions[SW_RECALL]) { World.invP1.cyberItemIndex = GetExistingCyberItemIndex(); return; } UseRecall(); break;
    }
}

void CycleCyberSpaceItemUp() { int next = World.invP1.cyberItemIndex + 1; if (next > 2){next=0;} for (int c = 0; c <= 7; c++) { if (!(World.invP1.hasSoft & (1u << next))) { World.invP1.cyberItemIndex = (i8)next; return; } if (c == 7) { World.invP1.cyberItemIndex = -1; return; } if (++next > 2) {next = 0;} } }
void CycleCyberSpaceItemDn() { int next = World.invP1.cyberItemIndex - 1; if (next < 0){next=2;} for (int c = 0; c <= 7; c++) { if (  World.invP1.hasSoft & (1u << next))  { World.invP1.cyberItemIndex = (i8)next; return; } if (c == 7) { World.invP1.cyberItemIndex = -1; return; } if (--next < 0) {next = 2;} } }
void RemoveWeapon(int slot) { World.invP1.weaponInventoryIndices[slot] = World.invP1.weaponInventoryAmmoIndices[slot] = -1; }
static float DefaultEnergySettingForWeapon(int wep16Index) { return (wep16Index == 4) ? 5.0f : (wep16Index == 10) ? 13.0f : (wep16Index == 14) ? 2.0f : 3.0f; }
void UpdateAmmoCount() { World.invP1.numweapons=0; for (int i=0;i<7;i++) { if(World.invP1.weaponInventoryIndices[i] >= 0){World.invP1.numweapons++;} } }
void GetWeaponAmmoText(int slot,char* buf,size_t bufSize) {
    buf[0] = '\0'; int wepIdx = World.invP1.weaponInventoryIndices[slot]; bool alt = World.invP1.wepLoadedWithAlternate[slot]; float heat = World.invP1.currentEnergyWeaponHeat[slot];
    u8 mag = alt ? World.invP1.currentMagazineAmount2[slot] : World.invP1.currentMagazineAmount[slot];
    switch(wepIdx) {
        case 36: if (alt){sFormat(buf,bufSize,"%upn | %umg, %upn",mag,World.invP1.wepAmmo[0],World.invP1.wepAmmoSecondary[0]);}else{sFormat(buf,bufSize,"%umg | %umg, %upn",mag,World.invP1.wepAmmo[0],World.invP1.wepAmmoSecondary[0]);} break; // MK3 Assault Rifle
        case 37: case 40: case 46: case 50: case 51: scpy_to_a_from_b(buf,heat > 80.0f ? Sys_Text.stringTable[14] : Sys_Text.stringTable[15],bufSize); break; // Energy weapons
        case 38: if (alt){sFormat(buf,bufSize,"%utq | %und, %utq",mag,World.invP1.wepAmmo[2],World.invP1.wepAmmoSecondary[2]);}else{sFormat(buf,bufSize,"%und | %und, %utq",mag,World.invP1.wepAmmo[2],World.invP1.wepAmmoSecondary[2]);} break; // SV-23 Dartgun
        case 39: if (alt){sFormat(buf,bufSize,"%usp | %uhn, %usp",mag,World.invP1.wepAmmo[3],World.invP1.wepAmmoSecondary[3]);}else{sFormat(buf,bufSize,"%uhn | %uhn, %usp",mag,World.invP1.wepAmmo[3],World.invP1.wepAmmoSecondary[3]);} break; // AM-27 Flechette
        case 41: case 42: break; // Laser Rapier / Lead Pipe: no ammo
        case 43: if (alt){sFormat(buf,bufSize,"%usg | %uhw, %usg",mag,World.invP1.wepAmmo[7],World.invP1.wepAmmoSecondary[7]);}else{sFormat(buf,bufSize,"%uhw | %uhw, %usg",mag,World.invP1.wepAmmo[7],World.invP1.wepAmmoSecondary[7]);} break; // Magnum 2100
        case 44: if (alt){sFormat(buf,bufSize,"%usu | %ucr, %usu",mag,World.invP1.wepAmmo[8],World.invP1.wepAmmoSecondary[8]);}else{sFormat(buf,bufSize,"%ucr | %ucr, %usu",mag,World.invP1.wepAmmo[8],World.invP1.wepAmmoSecondary[8]);} break; // SB-20 Magpulse
        case 45: if (alt){sFormat(buf,bufSize,"%utf | %ust, %utf",mag,World.invP1.wepAmmo[9],World.invP1.wepAmmoSecondary[9]);}else{sFormat(buf,bufSize,"%ust | %ust, %utf",mag,World.invP1.wepAmmo[9],World.invP1.wepAmmoSecondary[9]);} break; // ML-41 Pistol
        case 47: sFormat(buf,bufSize,"%url | %url",World.invP1.currentMagazineAmount[slot],World.invP1.wepAmmo[11]); break; // MM-76 Railgun
        case 48: sFormat(buf,bufSize,"%urb | %urb",World.invP1.currentMagazineAmount[slot],World.invP1.wepAmmo[12]); break; // DC-05 Riotgun
        case 49: if (alt){sFormat(buf,bufSize,"%ulg | %usm, %ulg",mag,World.invP1.wepAmmo[13],World.invP1.wepAmmoSecondary[13]);}else{sFormat(buf,bufSize,"%usm | %usm, %ulg",mag,World.invP1.wepAmmo[13],World.invP1.wepAmmoSecondary[13]);} break; // RF-07 Skorpion
        default: break;
    }
}

__attribute__((noinline)) void AddAmmoToInventory(int index,int constIndex,int amount,bool isSecondary) { if(index < 0){return;} if(isSecondary){World.invP1.wepAmmoSecondary[index]+=(u16)amount;} else {World.invP1.wepAmmo[index]+=(u16)amount;} CenterStatusPrint("%s%s",Sys_Text.stringTable[constIndex + 326],Sys_Text.stringTable[630]); }
bool AddWeaponToInventory(int index,int ammo1,int ammo2,bool loadedAlt) {
    if (index < 0) return false;
    for (int i = 0; i < 7; i++) {
        if (World.invP1.weaponInventoryIndices[i] >= 0) continue;
        World.invP1.weaponInventoryIndices[i] = index;
        int index16 = (int)Get16WeaponIndexFromConstIndex(index);
        World.invP1.weaponEnergySetting[i] = DefaultEnergySettingForWeapon(index16);
        if (i == 0) {
            World.invP1.weaponCurrentPending = 0;
            World.invP1.weaponIndexPending   = (u16)index;
            World.invP1.justChangedWeap      = true;
            WeaponFireStartWeaponDip(0.5f);
            WeaponFireCompleteWeaponChange();
        }
        if (loadedAlt && ammo2 > 0) {
            World.invP1.currentMagazineAmount2[i] = (u8)ammo2;
            if (ammo1 > 0) World.invP1.wepAmmo[index16] += (u16)ammo1;
            World.invP1.wepLoadedWithAlternate[i] = true;
        } else {
            World.invP1.currentMagazineAmount[i] = (u8)ammo1;
            if (ammo2 > 0) World.invP1.wepAmmoSecondary[index16] += (u16)ammo2;
            World.invP1.wepLoadedWithAlternate[i] = false;
        }
        CenterStatusPrint("%s%s",Sys_Text.stringTable[index + 326],Sys_Text.stringTable[33]);
        UpdateAmmoCount();
        return true;
    }
    return false;
}

void UseGrenade(int index) {
    (void)index;
    if (World.invP1.holdingObject) { CenterStatusPrint("%s",Sys_Text.stringTable[311]); return; } // Can't use grenade, hands full
    ForceInventoryMode();  // Inventory mode is turned on when picking something up.
    ResetHeldItem();
    World.invP1.grenActive = true;
    CenterStatusPrint("%s%s",Sys_Text.stringTable[index + 326],Sys_Text.stringTable[320]); // activated, grenade is LIVE!
    switch(index) {
        case 7:  World.invP1.heldObjectIndex = 370; RemoveGrenade(0); break; // Frag
        case 8:  World.invP1.heldObjectIndex = 372; RemoveGrenade(3); break; // Concussion
        case 9:  World.invP1.heldObjectIndex = 387; RemoveGrenade(1); break; // EMP
        case 10: World.invP1.heldObjectIndex = 389; RemoveGrenade(6); break; // Earth Shaker
        case 11: World.invP1.heldObjectIndex = 402; RemoveGrenade(4); break; // Land Mine
        case 12: World.invP1.heldObjectIndex = 403; RemoveGrenade(5); break; // Nitropak
        case 13: World.invP1.heldObjectIndex = 404; RemoveGrenade(2); break; // Gas
        default: return;
    }
    World.invP1.heldObjectCustIdx = U16_MAX; World.invP1.heldObjectAmmo = 0; World.invP1.heldObjectAmmo2 = 0; World.invP1.heldObjectLoadedAlternate = false; World.invP1.holdingObject = true;
}

void InventoryUpdate() {
    if (Grenade()) { if (World.curLev == LEVEL_CYBERSPACE){UseCyberspaceItem();} else if (World.invP1.grenCur >= 0 && World.invP1.grenCur < 7 && World.invP1.grenAmmo[World.invP1.grenCur] > 0){UseGrenade(World.invP1.grenConstIndex[World.invP1.grenCur]);} else {CenterStatusPrint("%s",Sys_Text.stringTable[322]);/*Out of grenades.*/} }
    if (GrenadeCycUp())  { if (World.curLev == LEVEL_CYBERSPACE) CycleCyberSpaceItemUp(); else GrenadeCycle( 1); }
    if (GrenadeCycDown()){ if (World.curLev == LEVEL_CYBERSPACE) CycleCyberSpaceItemDn(); else GrenadeCycle(-1); }
    if (RecentLog() && (World.invP1.hasHardware & HW_ERD)) {
        if (World.invP1.lastAddedIndex >= 0) { int temp = World.invP1.lastAddedIndex; PlayLog(temp); World.invP1.lastAddedIndex = FindNextUnreadLog(); if (World.invP1.lastAddedIndex == temp) World.invP1.lastAddedIndex = -1; CheckForUnreadLogs(); }
        else { int temp = World.invP1.lastAddedIndex; World.invP1.lastAddedIndex = FindNextUnreadLog(); if (World.invP1.lastAddedIndex == temp) {World.invP1.lastAddedIndex = -1;} CheckForUnreadLogs(); CenterStatusPrint("%s",Sys_Text.stringTable[1019]); /*Log playback stopped.*/ }
    }
    if (Patch()) { if (World.invP1.patchCur >= 0 && World.invP1.patchCur < 7 && World.invP1.patchCounts[World.invP1.patchCur] > 0){PatchUse(World.invP1.patchCur);} else {CenterStatusPrint("%s",Sys_Text.stringTable[324]); /*Out of patches.*/} }
    if (PatchCycUp()){PatchCycle( 1);} else if (PatchCycDown()){PatchCycle(-1);}
}

void AddItemFail(int index/*Expects usableItem index*/) { DropHeldItem(); CenterStatusPrint("%s%s%s", Sys_Text.stringTable[32],Sys_Text.stringTable[index + 326],Sys_Text.stringTable[318]);/*Inventory full.*/ }
extern u8 magazinePitchCountForWeapon[16],magazinePitchCountForWeapon2[16];
void AddItemToInventory(int index, int custIdx) {
    if (IdxIsGenericItem(index)) { if(!AddGeneralObjectToInventory(index,custIdx)){AddItemFail(index);} }
    else if (IdxIsAudioLog(index)) { AddAudioLogToInventory(World.invP1.heldObjectCustIdx); }
    else if (IdxIsWeapon(index)) { if (!AddWeaponToInventory(index,World.invP1.heldObjectAmmo,World.invP1.heldObjectAmmo2,World.invP1.heldObjectLoadedAlternate)) { AddItemFail(index); } }
    else if (IdxIsAccessCard(index)) AddAccessCardToInventory(index);
    else {
        switch (index) {
            case 314: AddGrenadeToInventory(0,index); break; /*Frag*/
            case 315: AddGrenadeToInventory(3,index); break; /*Concussion*/
            case 316: AddGrenadeToInventory(1,index); break; /*EMP*/
            case 317: AddGrenadeToInventory(6,index); break; /*Earth Shaker*/
            case 318: AddGrenadeToInventory(4,index); break; /*Land Mine*/
            case 319: AddGrenadeToInventory(5,index); break; /*Nitropak*/
            case 320: AddGrenadeToInventory(2,index); break; /*Gas*/
            case 14: AddPatchToInventory(2,index); break; case 15: AddPatchToInventory(6,index); break; case 16: AddPatchToInventory(5,index); break; case 17: AddPatchToInventory(3,index); break;
            case 18: AddPatchToInventory(4,index); break; case 19: AddPatchToInventory(1,index); break; case 20: AddPatchToInventory(0,index); break;
            case 21: AddHardwareToInventory(0,custIdx,true); break; case 22: AddHardwareToInventory(1,custIdx,true); break; case 23: AddHardwareToInventory(2,custIdx,true); break; case 24: AddHardwareToInventory(3,custIdx,true); break;
            case 25: AddHardwareToInventory(4,custIdx,true); break; case 26: AddHardwareToInventory(5,custIdx,true); break; case 27: AddHardwareToInventory(6,custIdx,true); break; case 28: AddHardwareToInventory(7,custIdx,true); break;
            case 29: AddHardwareToInventory(8,custIdx,true); break; case 30: AddHardwareToInventory(9,custIdx,true); break; case 31: AddHardwareToInventory(10,custIdx,true);break; case 32: AddHardwareToInventory(11,custIdx,true); break;
            case 60: AddAmmoToInventory(12,index,magazinePitchCountForWeapon[12],false); break; /*rubber slugs*/         case 65: AddAmmoToInventory(8,index,magazinePitchCountForWeapon2[8],true); break; /*magpulse cartridge super*/
            case 66: AddAmmoToInventory(2,index,magazinePitchCountForWeapon[2],false); break; /*needle darts*/           case 67: AddAmmoToInventory(2,index,magazinePitchCountForWeapon2[2],true); break; /*tranquilizer darts*/
            case 68: AddAmmoToInventory(9,index,magazinePitchCountForWeapon[9],false); break; /*standard bullets*/       case 69: AddAmmoToInventory(9,index,magazinePitchCountForWeapon2[9],true); break; /*teflon bullets*/
            case 70: AddAmmoToInventory(7,index,magazinePitchCountForWeapon[7],false); break; /*hollow point rounds*/    case 71: AddAmmoToInventory(7,index,magazinePitchCountForWeapon2[7],true); break; /*slug rounds*/
            case 72: AddAmmoToInventory(0,index,magazinePitchCountForWeapon[0],false); break; /*magnesium tipped slugs*/ case 73: AddAmmoToInventory(0,index,magazinePitchCountForWeapon2[0],true); break; /*penetrator slugs*/
            case 74: AddAmmoToInventory(3,index,magazinePitchCountForWeapon[3],false); break; /*hornet clip*/            case 75: AddAmmoToInventory(3,index,magazinePitchCountForWeapon2[3],true); break; /*splinter clip*/
            case 76: AddAmmoToInventory(11,index,magazinePitchCountForWeapon[11],false); break; /*rail rounds*/          case 77: AddAmmoToInventory(13,index,magazinePitchCountForWeapon[13],false); break; /*slag magazine*/
            case 78: AddAmmoToInventory(13,index,magazinePitchCountForWeapon2[13],true); break; /*large slag magazine*/  case 79: AddAmmoToInventory(8,index,magazinePitchCountForWeapon[8],false); break; /*magpulse cartridges*/
            case 80: AddAmmoToInventory(8,index,magazinePitchCountForWeapon2[8],false); break; /*small magpulse cartridges*/
            default: return;
        }
    }
    play_wav(sounds[87],1.0f,(V3){0},false);
}

void CyberDecoyEnable() { World.decoyActive = true; }
void CyberDecoyDisable() { World.decoyActive = false; }
void CyberDoorOnCollisionEnter(u16 self, u16 other) { if(other != PLAYER1){return;} CenterStatusPrint("%s  %s",Sys_Text.stringTable[World.instances[self].messageIndex],Sys_Text.stringTable[601]); }
void CyberTimerInitAfterLoad(u16 self) { Entity* e = &World.instances[self]; e->cyberTimer = 600.0f; e->timerFinished = World.pauseRelativeTime + 1.0; }
void CyberTimerReset(u16 self, int diff) { Entity* e = &World.instances[self]; switch (diff) { case 0: e->cyberTimer = 600.0f; break; case 1: e->cyberTimer = 300.0f; break; case 2: e->cyberTimer = 240.0f; break; case 3: e->cyberTimer = 180.0f; break; } }
void CyberTimerUpdate(u16 self) {
    if(World.curLev != LEVEL_CYBERSPACE){return;} Entity* e=&World.instances[self]; if(e->cyberTimer <= 0.0f){UIExitCyberspace(); return;} if(e->timerFinished >= World.pauseRelativeTime){return;}
    e->cyberTimer-=1.0f; e->minutes=vfloor(e->cyberTimer / 60.0f); e->seconds = e->cyberTimer - (e->minutes * 60.0f); e->timerFinished = World.pauseRelativeTime + 1.0;
}

void CyberWallInitAfterLoad(u16 self) { Entity* e=&World.instances[self]; e->tickFinished=World.pauseRelativeTime + 2.0; e->animSwapFinished=0.0; } // TODO: push e->volume to chunk_frag.glsl as _CenterAlpha uniform or per-instance draw param for this geometry instance's material slot
void CyberWallUpdate(u16 self) { Entity* e = &World.instances[self]; if (World.pauseRelativeTime < e->tickFinished) {return;} e->tickFinished = World.pauseRelativeTime + 0.05; }
void SearchFXResetEnable(u16 self) { Entity* e = &World.instances[self]; if (e->itemLifeTime <= 0.0f) {e->itemLifeTime = 3.0f;} e->delayFinished = World.pauseRelativeTime + e->itemLifeTime; }
void SearchFXResetUpdate(u16 self) { Entity* e = &World.instances[self]; if (e->delayFinished >= World.pauseRelativeTime) {return;} flag_set(&e->entflags,EF_ACTIVE,false); }
void DelayedSpawnEnable(u16 self) { Entity* e = &World.instances[self]; e->timerFinished = World.pauseRelativeTime + e->delay; e->active = true; }
void DelayedSpawnUpdate(u16 self) {
    Entity* e = &World.instances[self]; if(!e->active || e->timerFinished <= 0.0 || e->timerFinished > World.pauseRelativeTime){return;}
    e->active = false; if(!e->doSelfAfterList){return;}
    if (e->despawnInstead) { if(e->destroyAfterListInsteadOfDeactivate){DeleteInstance(self);}else{flag_set(&e->entflags,EF_ACTIVE,false);} }     else flag_set(&e->entflags,EF_ACTIVE,true);
}

void FuncWallShiftChildren(u16 self, V3 delta) {
    if (vabs(delta.x)+vabs(delta.y)+vabs(delta.z) < 0.00001f) {return;}
    for (u16 i=PLAYER1;i<World.instCount;++i) { if (fwParentOf[i]==self) { World.position[i]=V3_AplusB(World.position[i],delta); } }
}
void FuncWallInitAfterLoad(u16 self) {
    Entity* e = &World.instances[self]; V3 prev = World.position[self];
    float distTotal = V3_Dist(e->startPosition,e->targetPosition); float f = 0.0f;
    if ((u8)e->funcState > FStat_AjarMovingTarget) f = e->ajarPercentage; // legacy out-of-range states: park at the ajar fractional point
    else if (e->funcState == FStat_AjarMovingTarget) f = e->ajarPercentage;
    else if (e->funcState == FStat_AjarMovingStart) f = 1.0f - e->ajarPercentage;
    if (f < 0.0f) f = 0.0f; if (f > 1.0f) f = 1.0f;
    V3 np = (distTotal > 0.0001f) ? V3_AplusB(e->startPosition,V3_ScaleByF(V3_Normalize(V3_AsubB(e->targetPosition,e->startPosition)),distTotal*f)) : e->startPosition;
    World.position[self]=np;
    if ((u8)e->funcState <= FStat_MovingTarget) { e->funcState = FStat_Start; e->percentMoved = 0.0f; } // rendered closed, so first frob must open
    FuncWallShiftChildren(self,V3_AsubB(np,prev));
}

void FuncWallMoveStart(u16 self) { World.instances[self].funcState = FStat_MovingStart; World.instances[self].tickFinished = World.pauseRelativeTime + 10.0f; }
void FuncWallMoveTarget(u16 self) { World.instances[self].funcState = FStat_MovingTarget; World.instances[self].tickFinished = World.pauseRelativeTime + 10.0f; }
void FuncWallTargetted(u16 self) { Entity* e = &World.instances[self]; u8 st = (u8)e->funcState;
    bool toTarget = st == FStat_Start || st == FStat_MovingStart || st == FStat_AjarMovingTarget || (st > FStat_AjarMovingTarget && e->ajarPercentage > 0.0f);
    if (toTarget){FuncWallMoveTarget(self);} else{FuncWallMoveStart(self);} play_wav(sounds[76],1.0f,World.position[self],true); }
void FuncWallUpdateInner(u16 self) {
    Entity* e = &World.instances[self];
    if (e->funcState != FStat_MovingStart && e->funcState != FStat_MovingTarget) return;
    V3 goal = e->funcState == FStat_MovingStart ? e->startPosition : e->targetPosition;
    FuncStates doneState = e->funcState == FStat_MovingStart ? FStat_Start : FStat_Target;
    V3 delta = V3_AsubB(goal,World.position[self]);
    float distanceLeft = V3_Mag(delta), total = V3_Dist(e->startPosition,e->targetPosition), dist = e->speed * (float)World.deltaTime;
    if (distanceLeft <= dist || e->tickFinished < World.pauseRelativeTime) { World.position[self]=goal; e->funcState=doneState; e->percentMoved=doneState == FStat_Target ? 1.0f : 0.0f; return; }
    if (distanceLeft > 0.0001f) World.position[self]=V3_AplusB(World.position[self],V3_ScaleByF(V3_Normalize(delta),dist));
    if (total > 0.0001f) e->percentMoved = V3_Dist(e->startPosition,World.position[self]) / total;
}
void FuncWallUpdate(u16 self) {
    V3 prev = World.position[self];
    FuncWallUpdateInner(self);
    FuncWallShiftChildren(self,V3_AsubB(World.position[self],prev));
}
// ForceBridge
void func_forcebridge(u16 self) {
    Entity* e = &World.instances[self];
    e->tickFinished = World.pauseRelativeTime + 0.05f + (double)random_range(0.0f,1.0f); e->lerping = true;
    if(e->activatedScale.x <= 0.02f){e->activatedScale.x = 2.56f;} if(e->activatedScale.y <= 0.02f){e->activatedScale.y = 0.08f;} if(e->activatedScale.z <= 0.02f){e->activatedScale.z = 2.56f;}
    if(!e->active){ e->modelIndex=MAX_MDLS; World.col[self]=COLTYPE_NONE;}
    switch (e->fieldColor) {
        case ForceFieldColor_Red:e->texIndex=38; break; case ForceFieldColor_Green:e->texIndex=40; break; case ForceFieldColor_Blue:e->texIndex=39; break; case ForceFieldColor_Purple:e->texIndex=41; break; case ForceFieldColor_RedFaint:e->texIndex=198; break;
    }
}

void ForceBridgeActivate(u16 self, bool isSilent) {
    Entity* e = &World.instances[self]; if (e->active) {return;}
    if(!isSilent){play_wav(sounds[102],1.0f,World.position[self],true);}
    flag_set(&e->entflags,EF_ACTIVE,true);
    e->modelIndex=78; World.col[self]=COLTYPE_BOX; e->active=e->lerping=true; World.scale[self]=(V3){ e->forceFieldDirectionX ? 0.1f : e->activatedScale.x,e->forceFieldDirectionY ? 0.1f : e->activatedScale.y,e->forceFieldDirectionZ ? 0.1f : e->activatedScale.z };
}

void ForceBridgeDeactivate(u16 self, bool isSilent) { Entity* e = &World.instances[self]; if (!e->active) {return;} if (!isSilent) {play_wav(sounds[102],1.0f,World.position[self],true);} e->active = false; e->lerping = true; }
void ForceBridgeToggle(u16 self) { if (World.instances[self].active) {ForceBridgeDeactivate(self,false); } else {ForceBridgeActivate(self,false);} }
void ForceBridgeUpdate(u16 self) {
    Entity* e = &World.instances[self]; if (e->tickFinished >= World.pauseRelativeTime) return;
    e->tickFinished = World.pauseRelativeTime + 0.05f;
    if (e->active) {
        if (!e->lerping) return;
        float sx = e->forceFieldDirectionX ? lerp(World.scale[self].x,e->activatedScale.x,0.1f) : World.scale[self].x;
        float sy = e->forceFieldDirectionY ? lerp(World.scale[self].y,e->activatedScale.y,0.1f) : World.scale[self].y;
        float sz = e->forceFieldDirectionZ ? lerp(World.scale[self].z,e->activatedScale.z,0.1f) : World.scale[self].z;
        World.scale[self] = (V3){sx,sy,sz}; if (vabs(e->activatedScale.x - sx) < 0.08f && vabs(e->activatedScale.y - sy) < 0.08f && vabs(e->activatedScale.z - sz) < 0.08f) { World.scale[self] = e->activatedScale; e->lerping = false; }
    } else if (e->lerping) {
        float sx = e->forceFieldDirectionX ? lerp(World.scale[self].x,0.0f,0.1f) : World.scale[self].x;
        float sy = e->forceFieldDirectionY ? lerp(World.scale[self].y,0.0f,0.1f) : World.scale[self].y;
        float sz = e->forceFieldDirectionZ ? lerp(World.scale[self].z,0.0f,0.1f) : World.scale[self].z;
        World.scale[self] = (V3){sx,sy,sz}; if (sx < 0.08f || sy < 0.08f || sz < 0.08f) { e->modelIndex = MAX_MDLS; World.col[self] = COLTYPE_NONE; e->lerping = false; }
    }
}

// TriggerCounter
void TriggerCounterTarget(u16 self, u16 activator) { UseTargets(activator,World.instances[self].targetIdx); }
void TriggerCounterDelayedTarget(u16 self, u16 act) { World.instances[self].delayFinished = World.pauseRelativeTime + World.instances[self].delay; TriggerCounterTarget(self,act); }
void TriggerCounterTargetted(u16 self, u16 act) { Entity* e=&World.instances[self]; e->counter++; if (e->counter != e->countToTrigger) {return;} if (e->delay <= 0.0f){TriggerCounterTarget(self,act);}else{TriggerCounterDelayedTarget(self,act);} if (!e->dontReset){e->counter=0;} }
// TextureChanger
void TextureChangerToggle(u16 self) {
    u16 alt = 0, glowAlt = 0;
         if (World.instances[self].index == 538) { alt = 1118; glowAlt = 1116; }
    else if (World.instances[self].index == 689) { alt = 841; glowAlt = 840; }
    else if (World.instances[self].index == 690) { alt = 844; glowAlt = 843; }
    else if (World.instances[self].index == 695) { alt = 858; glowAlt = 857; }
    else return;
    if (World.instances[self].currentTexture) { World.instances[self].texIndex = EDefs[World.instances[self].index].texIndex; World.instances[self].glowIndex = EDefs[World.instances[self].index].glowIndex; }
    else { World.instances[self].texIndex = alt; World.instances[self].glowIndex = glowAlt; }
    World.instances[self].currentTexture = !World.instances[self].currentTexture;
}
// LogicTimer
void LogicTimerInitBeforeLoad(u16 self) { Entity* e=&World.instances[self]; if(e->timeInterval <= 0.0f){e->timeInterval=0.35f;} if(e->randomMin <= 0.0f){e->randomMin=5.0f;} if(e->randomMax <= 0.0f){e->randomMax=10.0f;} e->intervalFinished=World.pauseRelativeTime + (e->useRandomTimes ? (double)random_range(e->randomMin,e->randomMax) : (double)e->timeInterval); }
void LogicTimerUseTargets(u16 self) { UseTargets(self,World.instances[self].targetIdx); }
void LogicTimerUpdate(u16 self) { Entity* e=&World.instances[self]; if(!e->active || e->intervalFinished >= World.pauseRelativeTime){return;} e->intervalFinished=World.pauseRelativeTime + (e->useRandomTimes ? (double)random_range(e->randomMin,e->randomMax) : (double)e->timeInterval); LogicTimerUseTargets(self); }
void LogicTimerTargetted(u16 self, u16 activator) { (void)activator; World.instances[self].active = !World.instances[self].active; }
// ButtonSwitch
void ButtonSwitchInitAfterLoad(u16 self) { Entity* e=&World.instances[self]; e->delayFinished=0.0f; if(e->active){e->tickFinished=World.pauseRelativeTime + 1.5 + (double)random_range(0.0f,1.0f);} }
void ButtonSwitchUseTargets(u16 self, u16 activator) {
    Entity* e=&World.instances[self]; (void)activator;
    UseTargets(self,e->targetIdx); // Citadel semantics: the originator's own ioflags become the UseData carried to targets.
    e->active=!e->active;
    if(e->index == 689 || e->index == 690 || e->index == 695) { TextureChangerToggle(self); if(e->index == 689 && e->active){e->tickFinished=World.pauseRelativeTime + 1.5f;} }
}

static __attribute__((noinline)) void UIBlockedBySecurity(V3 tetherPoint) { (void)tetherPoint; CenterStatusPrint("%s",Sys_Text.stringTable[25]); }
static __attribute__((noinline)) void EntitySetLocked(Entity* e, bool locked) { DualLog("Unlocking entity with index %u\n",(u16)(e - World.instances)); flag_set(&e->entflags,EF_LOCKED,locked); }
void ButtonSwitchUse(u16 self, u16 activator) {
    Entity* e = &World.instances[self]; if(Cheats.superoverride || World.diffMis == 0){EntitySetLocked(e,false);} else if(GetCurrentLevelSecurity() > e->securityThreshold){UIBlockedBySecurity(World.position[self]); return;}
    if ((e->entflags & EF_LOCKED) != 0) { CenterStatusPrint("%s",Sys_Text.stringTable[e->lockedMessageLingdex]); if (e->SFXLockedIndex >= 0 && e->SFXLockedIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXLockedIndex],1.0f,World.position[self],true); return; }
    if (e->SFXIndex >= 0 && e->SFXIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXIndex],1.0f,World.position[self],true);
    CenterStatusPrint("%s",Sys_Text.stringTable[e->messageIndex]);
    if (e->delay > 0.0f) { e->recentMostActivator = activator; e->delayFinished = World.pauseRelativeTime + e->delay; } else ButtonSwitchUseTargets(self,activator);
}

void ButtonSwitchUpdate(u16 self) {
    Entity* e = &World.instances[self]; if (e->delayFinished > 0.0 && e->delayFinished < World.pauseRelativeTime) { e->delayFinished = 0.0; ButtonSwitchUseTargets(self,e->recentMostActivator); }
    if (e->index == 689 && e->active && e->tickFinished < World.pauseRelativeTime) { TextureChangerToggle(self); e->tickFinished = World.pauseRelativeTime + 1.5f; }
}

void ButtonSwitchTargetted(u16 self, u16 activator) { ButtonSwitchUse(self,activator); }
void HealingBedUse(u16 self, u16 owner) { Entity* e=&World.instances[self]; if (GetCurrentLevelSecurity() <= (u8)e->minSecurityLevel) { if(!e->broken){HealthManagerHealingBed(PLAYER1,e->amount,true); CenterStatusPrint("%s",Sys_Text.stringTable[23],owner); play_wav(sounds[103],1.0f,World.position[self],false);} else {CenterStatusPrint("%s",Sys_Text.stringTable[24],owner);} } else UIBlockedBySecurity(World.position[self]); }
// VaporizeButton
void VaporizeClick(void) {
    if (World.invP1.generalInvCurrent == 0) return; // Access Cards index.
    int cur = World.invP1.generalInvCurrent;
    World.invP1.generalInventoryIndexRef[cur] = -1; // Remove item
    World.invP1.generalInvCurrent -= 1;
    if (World.invP1.generalInvCurrent < 0) { World.invP1.generalInvCurrent = 0; } // since it is Access Cards.
    cur = World.invP1.generalInvCurrent;
    if (World.invP1.generalInventoryIndexRef[cur] < 0) { for (int i=13; i >= 0; i--) { if (World.invP1.generalInventoryIndexRef[i] >= 0) { World.invP1.generalInvCurrent = (i8)i; break; } } }
    play_wav(sounds[89], SfxVol(), (V3){0.0f,0.0f,0.0f}, false); // vaporize sfx
}

typedef struct { i8 norm,alt; } AmmoIconEntry;
static const AmmoIconEntry ammoIconTable[51]={[36-36]={7,8}/*MK3 Magnesium/Penetrator*/,[37-36]={-2,-2}/*Energy*/,[38-36]={0,1}/*Dartgun Needle/Tranq*/,[39-36]={9,10}/*Flechette Hornette/Splinter*/,[40-36]={-2,-2}/*Energy*/,[41-36]={-1,-1}/*Rapier, no ammo*/,
                                              [42-36]={-1,-1}/*Pipe, no ammo*/,[43-36]={5,6}/*Magnum Hollow/Slug*/,[44-36]={11,-1}/*Magpulse Magcart*/,[45-36]={2,3 }/*Pistol Standard/Teflon*/,[46-36]={-2,-2}/*Energy*/,[47-36]={14,-1}/*Railgun Rail Rounds*/,
                                              [48-36]={4,-1}/*Riotgun Rubber Slugs*/,[49-36]={12,13}/*Skorpion Slag/Large Slag*/,[50-36]={-2,-2}/*Energy*/,[51-36]={-2,-2}/*Energy*/};
i8 AmmoIconGet(int index,bool alt) { if (index < 36 || index > 51) {return -1;} const AmmoIconEntry* e = &ammoIconTable[index - 36]; return alt ? e->alt : e->norm; }
static double creditsVidStartTime,creditsVidFinished; static u8 creditsVidPhase; // CreditsScroll, TODO video text phases: 0=text1 visible, 1=text2 visible, 2=text3 visible, 3=all hidden
void CreditsOnEnable(void) { World.creditsActive=true; World.creditsPageIndex=0; creditsVidStartTime=World.absoluteTime; creditsVidFinished=World.absoluteTime + 37.2; creditsVidPhase=0; }
void CreditsUpdate(void) {
    if (!World.creditsActive) return;
    double elapsed = World.absoluteTime - creditsVidStartTime;
    if (creditsVidFinished > 0.0) { // Drive video text phase transitions
        if (elapsed >  7.0 && creditsVidPhase == 0) creditsVidPhase = 1; // TODO: swap text1->text2 visibility
        if (elapsed > 11.0 && creditsVidPhase == 1) creditsVidPhase = 2; // TODO: swap text2->text3 visibility
        if (elapsed > 14.0 && creditsVidPhase == 2) creditsVidPhase = 3; // TODO: hide text3
        if (World.absoluteTime >= creditsVidFinished) { creditsVidFinished=0.0; creditsVidPhase=3; } // TODO: deactivate exitVideo overlay and all text phases
    }
    if (Menu()) { if (creditsVidFinished > 0.0) { creditsVidFinished = 0.0; return; /*skip video*/} MenuGoBack(); return; }
    if (creditsVidFinished > 0.0) return; // absorb all click input while video playing
    if (Attack()) { // left click — advance
        if (!(World.creditsPageIndex >= CREDITS_PAGES)) {
            ++World.creditsPageIndex; if (!World.gameFinished && World.creditsPageIndex == 1) ++World.creditsPageIndex; // skip stats page when not finishing game
            if (World.creditsPageIndex >= CREDITS_PAGES) World.creditsPageIndex = CREDITS_PAGES; // bottom
        } else { World.creditsActive = false; MenuGoBack(); }
        return;
    }
    if (ToggleMode()) { if (World.creditsPageIndex > 0){--World.creditsPageIndex;} } // right click — go back a page
}
// CyborgConversionToggle
void CyborgConversionToggleTargetted(void) {
    bool active = (World.ressurectionActiveLevels >> World.curLev) & 1u; flag_setu16(&World.ressurectionActiveLevels,(1u << World.curLev),!active); if (World.curLev == 6) flag_setu16(&World.ressurectionActiveLevels, (1u<<10|1u<<11|1u<<12),!active); // Set groves 10,11,12 when 6 gets toggled as they don't have their own switch
    play_wav(sounds[active ? 183 : 184],Sys_Settings.VolumeMessage,(V3){0.0f,0.0f,0.0f},false);/*"vox_cybconvcancelled" : "vox_cybconvenabled"*/ CenterStatusPrint("%s",Sys_Text.stringTable[active ? 591 : 592]);
}
// ElevatorButton
// static const char* elevFloorLabels[14] = {"R","1","2","3","4","5","6","7","8","9","G1","G2","G4","C"}; TODO
extern V3 queuedLevelPos; extern u8 queuedLevelToLoad;
void ElevatorButtonClick(u16 self) {
    Entity* e = &World.instances[self]; if (World.Sys_UI.linkedElevatorDoor == U16_MAX) { CenterStatusPrint("%s",Sys_Text.stringTable[6]); /*Too far away from that.*/ return; }
    Entity* door = &World.instances[World.Sys_UI.linkedElevatorDoor]; bool doorClosed = door->doorOpen == DoorState_Closed; float dist = V3_Dist(World.Sys_UI.objectInUsePos,World.position[PLAYER1]);
    if (dist > 2.0f/*tether dist*/ && !doorClosed) { CenterStatusPrint("%s",Sys_Text.stringTable[6]); /*Too far away from that.*/ return; }
    if (!doorClosed) { CenterStatusPrint("%s",Sys_Text.stringTable[7]); /*Door not closed.*/ return; }
    if (!(e->entflags & EF_ACTIVE)) { CenterStatusPrint("%s",Sys_Text.stringTable[8]); /*Floor not accessible.*/ return; }
    queuedLevelPos=(e->targetDestinationID != U16_MAX && e->targetDestinationID < World.instCount) ? World.position[e->targetDestinationID] : (V3){0.0f,0.0f,0.0f}; queuedLevelToLoad=(u8)e->teleportID;
}

void EmailTargetted(u16 self) { Entity* e=&World.instances[self]; u16 idx=e->emailIndex; if(World.invP1.hasLog[idx]){return;} World.invP1.hasLog[idx]=World.invP1.hasNewEmail=true; World.invP1.lastAddedIndex=idx; if(Sys_Text.audioLogType[idx] == AudioLogType_Email){World.invP1.beepDone=true;} if(e->autoPlayEmail){PlayLastAddedLog(idx);} }
u8 OverloadButtonVisualState() { if (World.invP1.currentEnergyWeaponHeat[World.invP1.weaponCurrent] > 25.0f) {return 2;} if (World.invP1.overloadEnabled) {return 1;} return 0; }
void OverloadButtonAction() {
    static double overloadClickFinished = 0.0; if (overloadClickFinished >= World.pauseRelativeTime){return;} overloadClickFinished = World.pauseRelativeTime + 0.4; 
    if (World.invP1.currentEnergyWeaponHeat[World.invP1.weaponCurrent] > 25.0f) { CenterStatusPrint("%s",Sys_Text.stringTable[12]);/*Weapon too hot*/ return; }
    if (World.invP1.overloadEnabled) { CenterStatusPrint("%s",Sys_Text.stringTable[13]);/*Overload disabled*/ World.invP1.overloadEnabled = false; } else { CenterStatusPrint("%s",Sys_Text.stringTable[17]);/*Overload enabled*/ World.invP1.overloadEnabled = true; }
}

// TargetID
#define TARGETID_LINK_DIST       10.0f
#define TARGETID_DAMAGE_TIME_HIT  2.5f
#define TARGETID_DAMAGE_TIME_MISS 1.0f
float TargetIDGetSensingRange(bool manual) { u8 ver = World.invP1.hardwareVersion[HW_TID_IDX]; if (manual) {return (ver >= 4) ? 18.0f : 13.0f;} return (ver <= 2) ? 0.0f : ((ver == 3) ? 13.0f : 20.0f); }
float TargetIDGetTetherRange() { return (World.invP1.hardwareVersion[HW_TID_IDX] >= 4) ? 22.0f : 15.0f; }
static void TargetIDDeactivate(u16 self) { Entity* e=&World.instances[self]; if(e->enemy != WORLD){Entity* npc=&World.instances[e->enemy]; flag_set(&npc->entflags,EF_TARGID_ATTACHED,false); e->enemy=WORLD;} e->textIndex=-1; flag_set(&e->entflags,EF_ACTIVE,false); }
void TargetIDSendDamageReceive(u16 self,float damage,AttType attackType) {
    Entity* e=&World.instances[self]; if(e->enemy == WORLD){return;} Entity* npc=&World.instances[e->enemy];
    if (attackType == Att_Trnq) { e->textIndex=536;/*STUNNED*/ e->animSwapFinished=World.pauseRelativeTime - 1.0;/*expire damage text*/ }
    else {
        float mh = npcTable[npc->index - 419].health;
        if      (damage > mh * 0.75f) e->textIndex = 514; // SEVERE DAMAGE
        else if (damage > mh * 0.50f) e->textIndex = 515; // MAJOR DAMAGE
        else if (damage > mh * 0.25f) e->textIndex = 513; // NORMAL DAMAGE
        else if (damage > 0.0f)       e->textIndex = 512; // MINOR DAMAGE
        else                          e->textIndex = 511; // NO DAMAGE
        e->animSwapFinished = World.pauseRelativeTime + ((damage == 0.0f) ? TARGETID_DAMAGE_TIME_MISS : TARGETID_DAMAGE_TIME_HIT);
    }
}

void TargetIDUpdate(u16 self) {
    if (!(World.instances[self].entflags & EF_ACTIVE)) return;
    if (World.instances[self].enemy == WORLD) { TargetIDDeactivate(self); return; }
    Entity* npc = &World.instances[World.instances[self].enemy];
    if (npc->health <= 0.0f) { TargetIDDeactivate(self); return; }
    if (V3_Dist(World.position[self],World.position[PLAYER1]) > TARGETID_LINK_DIST) { TargetIDDeactivate(self); return; }
    if (World.instances[self].tickFinished < World.pauseRelativeTime) { TargetIDDeactivate(self); return; }
    World.position[self]=World.position[World.instances[self].enemy]; // Track parent NPC position
    bool stunned = npc->tranquilizeFinished > World.pauseRelativeTime;
    flag_set(&World.instances[self].entflags,EF_ASLEEP,stunned);
    if (World.instances[self].textIndex >= 0) {
        if (stunned && World.instances[self].animSwapFinished < World.pauseRelativeTime) World.instances[self].textIndex = 536; // STUNNED
        else if (World.instances[self].animSwapFinished < World.pauseRelativeTime) { World.instances[self].textIndex = -1; if (!(World.invP1.hasHardware & HW_TID)) { TargetIDDeactivate(self); return; } }
    }
}
// PlayerEnergy
static const float  hwDrain[12][4] = {[3]={0.01535f,0.03413f,0.02559f,0.0f},[5]={0.04096f,0.10239f,0.17919f,0.05119f},[6]={0.001706f,0.0f,0.0f,0.0f},[7]={0.02559f,0.04266f,0.05119f,0.0f},[9]={0.0f,0.02f,0.015f,0.0f},[11]={0.08533f,0.0f,0.0f,0.0f},};
static const u16 hwDrainJPM[12][4] = {[3]={9,20,15,0},[5]={24,60,105,30},[6]={1,0,0,0},[7]={15,25,30,0},[9]={0,16,12,0},[11]={50,0,0,0},};
void CreateTargetIDInstance(float damage, u16 hitIdx, float tranq) { if (hitIdx == WORLD || hitIdx >= World.instCount) return; Entity* npc = &World.instances[hitIdx]; if (!(npc->entflags & EF_ACTIVE) || (npc->entflags & EF_TARGID_ATTACHED)) return; if (V3_Dist(World.position[hitIdx], World.position[PLAYER1]) > TargetIDGetTetherRange()) return; u16 tidIdx = SpawnDynamicObject(736, false); if (tidIdx == WORLD || tidIdx == U16_MAX) return; Entity* tid = &World.instances[tidIdx]; tid->enemy = hitIdx; tid->tickFinished = World.pauseRelativeTime + 4.0; tid->textIndex = (tranq >= 0.0f) ? 536 : -1; tid->animSwapFinished = World.pauseRelativeTime + (tranq >= 0.0f ? 2.5 : 0.0); World.position[tidIdx] = World.position[hitIdx]; flag_set(&tid->entflags, EF_ACTIVE, true); flag_set(&npc->entflags, EF_TARGID_ATTACHED, true); if (damage > 0.0f) TargetIDSendDamageReceive(tidIdx, damage, Att_None); }
void TargetIdentifierSenseTargets() { for (u16 i = INSTS_1ST_IDX; i < World.instCount; i++) { Entity* e = &World.instances[i]; if (!(e->entflags & EF_ACTIVE) || !IdxIsNPC(e->index) || (e->entflags & EF_DEAD) || (e->entflags & EF_TARGID_ATTACHED) || V3_Dist(World.position[i],World.position[PLAYER1]) > TargetIDGetSensingRange(false)){continue;} CreateTargetIDInstance(0.0f,i,-1.0f); } }
bool ModRequestsGrayscale() { return ((World.invP1.hasHardware & HW_INF) && (World.invP1.hardwareIsActive & HW_INF) > 0); }
static void DeactivateHardwareOnEnergyDepleted() { World.invP1.hardwareIsActive = 0; }
void TakeEnergy(float take) { if (World.invP1.energy <= 0.0f || Cheats.redbull) {return;} World.invP1.energy -= take; if (World.invP1.energy <= 0.0f) { World.invP1.energy = 0.0f; play_wav(sounds[84],Sys_Settings.VolumeEffects,(V3){0.0f,0.0f,0.0f},false);/*energy_gone*/ CenterStatusPrint("%s",Sys_Text.stringTable[314]); /*Power supply exhausted.*/ DeactivateHardwareOnEnergyDepleted(); } }
void GiveEnergy(float give,EnergyType type) {
    World.invP1.energy += give; if (World.invP1.energy > 255.0f) {World.invP1.energy = 255.0f;}
    if (type == EnergyType_Battery){play_wav(sounds[79],Sys_Settings.VolumeEffects,(V3){0.0f,0.0f,0.0f},false);/*batteryuse*/} else if (type == EnergyType_ChargeStation){play_wav(sounds[100],Sys_Settings.VolumeEffects,(V3){0.0f,0.0f,0.0f},false);/*chargingstation*/}
}

void PlayerEnergyInit() { World.invP1.energy = 54.0f; World.invP1.energyDrainTickFinished = World.pauseRelativeTime + 0.1 + random_range(0.0f,1.0f); World.invP1.drainJPM = 0; }
void PlayerEnergyUpdate() {
    if (World.invP1.hasHardware & HW_TID) TargetIdentifierSenseTargets(); if (World.invP1.energyDrainTickFinished > World.pauseRelativeTime) return;
    World.invP1.energyDrainTickFinished = World.pauseRelativeTime + 0.1; bool anyDrain = false; u8 ver; World.invP1.drainJPM = 0;
    for (int hw=3;hw<=11;++hw) {
        u16 bit=(u16)(1u << hw); if (!(World.invP1.hardwareIsActive & bit) || hw == 4 || hw == 8 || hw == 10) continue; // No energy usage
        ver=World.invP1.hardwareVersionSetting[hw]; float drain=hwDrain[hw][ver];  World.invP1.drainJPM += hwDrainJPM[hw][ver]; if (drain > 0.0f) { TakeEnergy(drain); anyDrain = true; }
    }
    if (anyDrain && World.invP1.energy <= 0.0f) { DeactivateHardwareOnEnergyDepleted(); World.invP1.drainJPM = 0; } // Depleted
}
// GeneralInventory
static void ApplyBattery(void) { if (World.invP1.energy >= 255.0f) { CenterStatusPrint("%s",Sys_Text.stringTable[303]); return; }/*Energy full*/ GiveEnergy(83.0f,EnergyType_Battery); World.invP1.generalInventoryIndexRef[World.invP1.hardwareInvCurrent] = -1; }
static void ApplyIcadBattery(void) { if (World.invP1.energy >= 255.0f) { CenterStatusPrint("%s",Sys_Text.stringTable[303]); return; }/*Energy full*/ GiveEnergy(255.0f,EnergyType_Battery); World.invP1.generalInventoryIndexRef[World.invP1.hardwareInvCurrent] = -1; }
static void ApplyHealthkit(void) { if (World.instances[PLAYER1].health >= 255.0f) { CenterStatusPrint("%s",Sys_Text.stringTable[303]); return; }/*Energy full*/ World.instances[PLAYER1].health = 255.0f; World.invP1.generalInventoryIndexRef[World.invP1.hardwareInvCurrent] = -1; }
void GeneralInvUse(int buttonIdx,int customIdx) {
    World.invP1.hardwareInvCurrent = buttonIdx; int itemIdx = World.invP1.generalInventoryIndexRef[buttonIdx]; if (buttonIdx == 0) {return;}
    (void)customIdx; (void)itemIdx;
}

void GeneralInvClick(int buttonIdx,int customIdx) { World.Sys_UI.mouseClickHeldOverGUI = true; GeneralInvUse(buttonIdx,customIdx); }
void GeneralInvApply(int buttonIdx,int customIdx) {
    if (buttonIdx == 0) { return; }
    World.invP1.hardwareInvCurrent = buttonIdx; int itemIdx = World.invP1.generalInventoryIndexRef[buttonIdx];
    switch (itemIdx) { case 52:ApplyBattery();break;  case 53:ApplyIcadBattery();break;  case 55:ApplyHealthkit();break;  default:World.invP1.hardwareInvCurrent=buttonIdx;(void)customIdx;break;}
}
void GeneralInvDoubleClick(int buttonIdx,int customIdx) { World.Sys_UI.mouseClickHeldOverGUI = true; GeneralInvApply(buttonIdx,customIdx); }
void GeneralInventoryActivate() { int cur=World.invP1.generalInvCurrent; if(cur < 0 || cur >= 14){DualLog("BUG: generalInvCurrent out of range at %d",cur); return;} GeneralInvApply(cur,World.invP1.generalInvCustIdx[cur]); if(cur != 0)World.invP1.generalInventoryIndexRef[cur]=-1; }
static bool GrenadeIsNPCMine(u16 self) { return World.layer[self] != L_PlayerBullets; }
void ApplyImpactForce(u16 target, float vel, V3 normal, V3 pt) {
    if (target == WORLD || target >= World.instCount || vel <= 0.0f){return;} Entity* e = &World.instances[target]; if((e->entflags & EF_DEAD) || (!(e->entflags & EF_RIGIDBODY) && target != PLAYER1)){return;}
    V3 n = V3_Normalize(normal); if (V3_Mag(n) < 0.0001f) {n = (V3){0.0f,1.0f,0.0f};/*At least make it pop off the floor*/} AddForce(target,V3_ScaleByF(n,vel),true); (void)pt; // TODO, have torque applied relative to point lever arm.
}

void ApplyImpactForceSphere(DamageData* dd, V3 center, float radius, float baseVel) { 
    if (radius <= 0.0f || baseVel <= 0.0f) return;
    float r2 = radius * radius;
    for (u16 i = INSTS_1ST_IDX; i < World.instCount; i++) {
        Entity* e = &World.instances[i]; if (!(e->entflags & EF_ACTIVE) || (e->entflags & EF_DEAD)) continue;
        if (!(e->entflags & EF_RIGIDBODY) && !IdxIsNPC(e->index) && i != PLAYER1) continue;
        float sqd = V3_SqDist(World.position[i], center); if (sqd > r2) continue;
        float dist = vsqrtf(sqd); float falloff = 1.0f - (dist / radius); if (falloff <= 0.0f) continue;
        V3 normal; if(dist > 0.0001f){normal=V3_ScaleByF(V3_AsubB(World.position[i],center), 1.0f / dist);}else{normal = (V3){0.0f,1.0f,0.0f}; ApplyImpactForce(i,baseVel * falloff,normal,World.position[i]);}
        if (dd && dd->damage > 0.0f && i != dd->owner) { DamageData splash=*dd; splash.damage = dd->damage * falloff; splash.hitIdx = i; splash.hitpoint=World.position[i]; splash.attacknormal=normal; TakeDamage(i,splash); }
    }
}

void SpawnExplosionEffect(V3 pos, int explosionType) { static const u16 prefabs[6] = {729,730,731,732,733,734}; int idx = (explosionType >= 0 && explosionType < 6) ? explosionType : 2; u16 fx = SpawnDynamicObject(prefabs[idx], false); if (fx == WORLD || fx == U16_MAX) return; World.position[fx] = pos; Entity* e = &World.instances[fx]; flag_set(&e->entflags, EF_ACTIVE, true); if (e->delay <= 0.0f) e->delay = 0.8f; e->delayFinished = World.pauseRelativeTime + e->delay; }
void GrenadeExplode(u16 self) {
    Entity* e = &World.instances[self];
    DamageData dd={.damage=e->damage,.penetration=e->strength,.offense=e->speed,.armorvalue=0.0f,.defense=0.0f,.impactVelocity=e->damage*1.5f,.attacknormal=(V3){0.0f,1.0f,0.0f},.hitpoint=World.position[self],.attackType=e->attackType,
                   .owner=e->recentMostActivator,.hitIdx=WORLD,.isOtherNPC=false,.berserkActive=(World.invP1.patchActive & PATCH_BERSERK) != 0};
    float radius = (e->strength > 0.0f) ? e->strength : 4.0f;
    ApplyImpactForceSphere(&dd,World.position[self],radius,e->damage * 1.5f); if (!GrenadeIsNPCMine(self)) { World.invP1.noiseFinished = World.pauseRelativeTime + 2.0; } i16 idx=(i16)e->index; int soundIndex=60,explosionType=2;
    switch (idx) {
        case 7: case 11: soundIndex = 64; World.fogFac += 5; explosionType = 1; break;/*frag, mine*/ case 8: case 10: soundIndex = 60; World.fogFac += 7; explosionType = 2; break;/*conc, earth*/ case 9:  soundIndex = 67; explosionType = 4; break;/*emp*/
        case 12: soundIndex = 60; World.fogFac += 6;  explosionType = 2; break;/*nitro*/ case 13: soundIndex = 63; World.fogFac += 10; explosionType = 3; break;/*gas*/
    }
    play_wav(SoundPath(soundIndex),1.0f,World.position[self],true); SpawnExplosionEffect(World.position[self],explosionType); Shake(-1.0f); DeleteInstance(self);
}

void GrenadeActivate(u16 self) { u16 idx=World.instances[self].index; if (idx == 10){World.instances[self].timerFinished=World.pauseRelativeTime + World.invP1.earthShakerTimeSetting;} if (idx == 12){World.instances[self].timerFinished=World.pauseRelativeTime + World.invP1.nitroTimeSetting;} }
void GrenadeUpdate(u16 self) { Entity* e = &World.instances[self]; u16 idx=World.instances[self].index; if(idx == 14){GrenadeExplode(self); return;} /*Plastique*/ if((idx == 10 || idx == 12) && e->timerFinished < World.pauseRelativeTime) { GrenadeExplode(self); return; } if (idx == 11) { V3 origin = World.position[self]; float pr = (e->strength > 0.0f) ? e->strength : 1.5f; for (u16 i = PLAYER1; i < World.instCount; i++) { Entity* o = &World.instances[i]; if (i == self || !(o->entflags & EF_ACTIVE) || (o->entflags & EF_DEAD)) continue; if (i != PLAYER1 && !IdxIsNPC(o->index)) continue; if (V3_SqDist(World.position[i], origin) < (pr * pr)) { GrenadeExplode(self); return; } } } }
void GrenadeOnCollision(u16 self) { u16 idx=World.instances[self].index; if ((idx >= 7 && idx <= 9) || idx == 13) GrenadeExplode(self); }
// ProjectileEffectImpact
float GetDamageTakeAmount(DamageData* dd) { if (!dd) return 0.0f; float take = dd->damage; if (take <= 0.0f) return 0.0f; if (dd->berserkActive) take *= BERSERK_DAMAGE_MULTIPLIER; if (dd->defense > 0.0f && dd->offense < dd->defense) { float r = (dd->defense - dd->offense) / dd->defense; if (r > 0.85f) r = 0.85f; take *= (1.0f - r); } if (dd->armorvalue > 0.0f && dd->penetration < dd->armorvalue) { float a = (dd->armorvalue - dd->penetration) / dd->armorvalue; if (a > 0.85f) a = 0.85f; take *= (1.0f - a); } if (take < 0.0f) take = 0.0f; return take; }
void SpawnImpactEffect(u16 impactType, V3 pos) { if (impactType == 0 || impactType == U16_MAX) return; u16 fx = SpawnDynamicObject(impactType, false); if (fx == WORLD || fx == U16_MAX) return; World.position[fx] = pos; Entity* e = &World.instances[fx]; flag_set(&e->entflags, EF_ACTIVE, true); if (e->itemLifeTime <= 0.0f) e->itemLifeTime = 1.0f; e->delayFinished = World.pauseRelativeTime + e->itemLifeTime; }
void ExitCyberspace(void) { UIExitCyberspace(); if (World.curLev != LEVEL_CYBERSPACE) return; if (World.instances[PLAYER1].cyberHealth <= 0.0f) World.instances[PLAYER1].cyberHealth = 1.0f; LoadLevel(World.startLevel < World.numLevels ? World.startLevel : 0, (V3){0.0f,0.0f,0.0f}); }
void ReduceCurrentLevelSecurity(SecurityType stype) { // Typical level: 4 CPU nodes. 20 cameras, 100% = 4x + 20y.  Assuming that a good camera percentage is 2-3%, CPU % would be about 10-15 each
    u8 lev = World.curLev; if (lev >= 14 || stype == SecurityType_None) return;
    const float camScore=4.0f, nodeSmallScore=10.0f, nodeLargeScore=27.0f; float total = (World.levelCameraCount[lev]*camScore)+(World.levelSmallNodeCount[lev]*nodeSmallScore)+(World.levelLargeNodeCount[lev]*nodeLargeScore); if (total <= 0.0f) return;
    float drop = camScore;
    switch (stype) {
        case SecurityType_Camera: drop=(camScore/total)*100.0f; if (World.levelCameraDestroyedCount[lev]<255) World.levelCameraDestroyedCount[lev]++; break;
        case SecurityType_NodeSmall: drop=(nodeSmallScore/total)*100.0f; if (World.levelSmallNodeDestroyedCount[lev]<255) World.levelSmallNodeDestroyedCount[lev]++; break;
        case SecurityType_NodeLarge: drop=(nodeLargeScore/total)*100.0f; if (World.levelLargeNodeDestroyedCount[lev]<255) World.levelLargeNodeDestroyedCount[lev]++; break;
        default: return;
    }
    int cur=(int)World.levelSecurity[lev]-(int)drop; if (cur<0) cur=0; World.levelSecurity[lev]=(u8)cur;
    if (World.levelCameraDestroyedCount[lev]==World.levelCameraCount[lev] && World.levelSmallNodeDestroyedCount[lev]==World.levelSmallNodeCount[lev] && World.levelLargeNodeDestroyedCount[lev]==World.levelLargeNodeCount[lev]) World.levelSecurity[lev]=0;
    CenterStatusPrint("%s%d%s", Sys_Text.stringTable[306], (int)World.levelSecurity[lev], Sys_Text.stringTable[307]);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
float Tranquilize(u16 i, float amount, bool energy);
static void ProjectileEffectImpactOnCollision(u16 self,u16 hitIdx, V3 hitPos,V3 hitNormal) {
    Entity* e = &World.instances[self]; if (hitIdx == e->recentMostActivator) return; // hit own host, ignore
    e->counter++;
    DamageData dd = {.damage=e->damage,.penetration=e->strength,.offense=e->speed,.armorvalue=0.0f,.defense=0.0f,.impactVelocity= e->damage * 1.5f,.attacknormal=hitNormal,.hitpoint=hitPos,.attackType=e->attackType,.owner=e->recentMostActivator,.hitIdx=hitIdx,
                     .isOtherNPC=IdxIsNPC(World.instances[hitIdx].index),.berserkActive=(World.invP1.patchActive & PATCH_BERSERK) != 0};
    Entity* hit = &World.instances[hitIdx];
    if (IdxIsNPC(hit->index)) { NPCTable* nt = &npcTable[hit->index - 419]; dd.armorvalue = nt->armorvalue; dd.defense = nt->defense; }
    if (e->lookUpIndex == 5) { ApplyImpactForceSphere(&dd, World.position[self], 3.2f, 1.0f); World.fogFac += 4; } // Railgun sphere impact
    if (hit->health > 0.0f || hit->cyberHealth > 0.0f) {
        dd.damage = GetDamageTakeAmount(&dd);
        if (e->counter < e->countToTrigger) dd.damage *= 0.85f; // per-hit falloff
        dd.impactVelocity = dd.damage * 1.5f;
        if (e->counter > 0) dd.impactVelocity /= 3.0f;
        if (World.curLev != LEVEL_CYBERSPACE && e->recentMostActivator == PLAYER1) { ApplyImpactForce(hitIdx,dd.impactVelocity,dd.attacknormal,hitPos); }
        float dmgFinal = TakeDamage(hitIdx,dd); float tranq=-1.0f;
        if (dd.isOtherNPC) { if(!(hit->entflags & EF_ASLEEP)){World.Sys_Music.inCombat=true;} if(dd.attackType == Att_Trnq){float stunAmount=vclamp(3.0f+(World.invP1.stungunSetting/100.0f)*7.0f,3.0f,10.0f); tranq=Tranquilize(hitIdx,stunAmount,true);} }
        if (dmgFinal < 0.0f) {dmgFinal = 0.0f;} CreateTargetIDInstance(dmgFinal,hitIdx,tranq); SpawnImpactEffect(GetImpactType(hitIdx),hitPos);
    }
    if (e->counter >= e->countToTrigger) { SpawnImpactEffect(GetImpactType(hitIdx),hitPos); if (e->despawnInstead){DeleteInstance(self);}else{flag_set(&e->entflags,EF_ACTIVE,false);} }
}
#pragma GCC diagnostic pop

void ProjectileEffectImpactInitAfterLoad(u16 self) { Entity* e=&World.instances[self]; e->counter=0; if(e->countToTrigger < 1){e->countToTrigger=1;} }
// HealthManager

 // None  Melee  MelEn  EnBm   Mag    Proj   Needle ProjEB ProjLn Gas    Tranq  Drill
static const float attackTypeMult[7][12]={[NPCType_Mutant]={1,1,1,1,0,1,2,1,1,2,1,1},[NPCType_Supermutant]={1,1,1,1,0,1,1,1,1,1.5,1,1},[NPCType_Robot]={1,1,1,1,4,1,0,1,1,0,1,1},[NPCType_Cyborg]={1,1,1,1,2,1,1,1,1,1,1,1},[NPCType_Supercyborg]={1,1,1,1,2,1,0,1,1,0,1,1},
                                          [NPCType_MutantCyborg]={1,1,1,1,0.5,1,2,1,1,2,1.5,1},[NPCType_Cyber]={1,1,1,1,1,1,1,1,1,1,1,0}}; // Attack type damage multiplier table [NPCType][AttType], 1.0f = no change, 0.0f = immune, other = multiplier
static const i16 objectDeathSound[] = {[458]=63,[459]=66,[460]=66,[464]=62,[465]=532,[466]=532,[467]=532,[468]=532,[469]=532,[470]=532,[471]=532,[472]=62,[473]=62,[474]=62,[475]=62,[476]=62,[477]=61,[478]=65,[479]=69,[525]=68,[526]=68,};
static bool IsCyberEntity(u16 self) { if (World.curLev == LEVEL_CYBERSPACE){return true;} Entity* e=&World.instances[self]; if (self != PLAYER1 && e->cyberHealth > 0.0f){return true;} return (IdxIsNPC(e->index) && (e->index - 419) > 23);/*24-28 are cyber enemies*/}
static float ApplyAttTypeAdjustments(u16 self,float take,AttType at) { if (!IdxIsNPC(World.instances[self].index) || World.instances[self].health <= 0.0f){return take;} NPCType t = npcTable[World.instances[self].index - 419].type; if (at >= 12){return take;} return take * attackTypeMult[t][at]; }
static void UseDeathTargets(u16 self) { if(self == PLAYER1){return;} if (World.instances[self].targetIdx != IO_NONE) UseTargets(self,World.instances[self].targetIdx); }
static void TeleportAway(u16 self) { 
    if (World.instances[self].entflags & EF_TELEPORT_ON_DEATH) {return;} flag_set(&World.instances[self].entflags,EF_TELEPORT_ON_DEATH,true);
    World.col[self] = COLTYPE_NONE; World.gravity[self] = 0.0f; World.velocity[self] = (V3){0,0,0}; World.angularVelocity[self] = (V3){0,0,0}; World.instances[self].modelIndex = U16_MAX;
    V3 fxPos = World.position[self]; if(World.col[self] != COLTYPE_NONE){fxPos=V3_AplusB(fxPos,World.colliderCenter[self]);} SpawnImpactEffect(735,fxPos); play_wav(sounds[106],1.0f,fxPos,false);
}

static void DropSearchables(u16 self) {

    for (int i = 0; i < 4; i++) {
        if (World.instances[self].contents[i] <= -1) {continue;} u16 spawned = SpawnDynamicObject(World.instances[self].contents[i] + 307,true);
        if(spawned != U16_MAX){World.position[spawned]=World.position[self]; World.instances[spawned].custIdx[0]=World.instances[self].custIdx[i];}else{CenterStatusPrint("BUG: Failed to instantiate object being dropped on gib.");}
        World.instances[self].contents[i] = World.instances[self].custIdx[i]=-1;
    }
}

static void CreateDeathEffects(u16 self,u16 fxPoolType) { if (fxPoolType == 0) {return; /*PoolType_None*/} V3 pos = World.position[self]; if (World.col[self] != COLTYPE_NONE) { pos = V3_AplusB(pos,World.colliderCenter[self]); } SpawnImpactEffect(fxPoolType, pos); }
static void HideSelf(u16 self) { if (World.instances[self].index == 279) {return; /*tv screens keep mesh visible*/} World.instances[self].modelIndex = MAX_MDLS; World.gravity[self] = 0.0f; }
static void NPCDeath(u16 self) {
    if (World.instances[self].entflags & EF_DEAD_CHECKS_DONE) {return;}
    flag_set(&World.instances[self].entflags,EF_DEAD_CHECKS_DONE,true); CreateDeathEffects(self,World.instances[self].deathBurst); if (World.instances[self].index == 419) play_wav(sounds[64],1.0f,World.position[self],true); // npc_autobomb: explosion1
    if (npcTable[World.instances[self].index - 419].type == NPCType_Cyber) DeleteInstance(self);

}

static void ObjectDeath(u16 self) {
    Entity* e = &World.instances[self]; if (World.instances[self].entflags & EF_DEAD_CHECKS_DONE) return;
    if (World.instances[self].entflags & EF_DEATH_BURST_DONE) { CreateDeathEffects(self,World.instances[self].deathBurst); DropSearchables(self); if (World.instances[self].index != 279){World.col[self]=COLTYPE_NONE;} HideSelf(self); }
    else { World.col[self] = COLTYPE_NONE; DropSearchables(self); CreateDeathEffects(self,World.instances[self].deathBurst); }
    flag_set(&World.instances[self].entflags,EF_DEAD_CHECKS_DONE,true); World.instances[self].automapHidden = true;
    if (World.instances[self].securityThreshold > 0) {
        SecurityType stype = SecurityType_None; if(World.instances[self].index == 477){stype=SecurityType_Camera;}else if(World.instances[self].index == 479){stype=SecurityType_NodeSmall;} else if(World.instances[self].index == 478){stype=SecurityType_NodeLarge;}
        if(stype != SecurityType_None){ReduceCurrentLevelSecurity(stype);}
    }
    u16 idx = World.instances[self].index; play_wav(SoundPath((idx < 527 && objectDeathSound[idx] != 0) ? objectDeathSound[idx] : 62/*crate_break*/),1.0f,World.position[self],true); if(e->deathBurst != 0){HideSelf(self);}
}

static void ScreenDeath(u16 self) {
    Entity* e = &World.instances[self]; if (e->entflags & EF_DEAD_CHECKS_DONE) return;
    flag_set(&e->entflags,EF_DEAD_CHECKS_DONE,true); play_wav(sounds[69],1.0f,World.position[self],true); // screen_destroy

    if (e->entflags & EF_DEATH_BURST_DONE) ObjectDeath(self); // gib path
}

static void VaporizeCorpse(u16 self,bool energyVaporized) { Entity* e=&World.instances[self]; flag_set(&e->entflags,EF_DEAD_CHECKS_DONE,true); DropSearchables(self); e->modelIndex=MAX_MDLS; if (IdxIsNPC(e->index) || IdxIsSearchable(e->index)) DeleteInstance(self); CreateDeathEffects(self,energyVaporized ? 2 : ((e->deathBurst == 0) ? 1/*Corpse hit fallback*/ : e->deathBurst)); }
static inline bool IsGrenade(u16 i) { return ((i >= 314 && i <= 320) || i == 370 || i == 372 || i == 387 || i == 389 || (i >= 402 && i <= 404)); }
static void Death(u16 self,bool energyVaporized) {
    Entity* e = &World.instances[self];
    if (e->entflags & EF_DEAD_CHECKS_DONE) return;
    UseDeathTargets(self);
    bool isNPC = IdxIsNPC(e->index);
    bool isObj = IdxIsDynamicObject(e->index);
    if (e->entflags & EF_ACT_AS_CORPSE_ONLY) { e->entflags |= EF_DEAD_CHECKS_DONE; return; }
//     bool gib        = (e->entflags & EF_DEATH_BURST_DONE) != 0;
    bool vaporize=(IdxIsNPC(e->index) && e->health <= 0.0f) || IdxIsCorpse(e->index);
    bool isGrenade=IsGrenade(e->index), doTeleport=(e->entflags & EF_TELEPORT_ON_DEATH) != 0;
    if (e->iceActive) World.col[self] = COLTYPE_NONE;
    if (vaporize && e->index != 477/*sec_camera*/ && !isGrenade) VaporizeCorpse(self,energyVaporized);
    else if (isObj) ObjectDeath(self);
    else if (e->index == 279/*screen*/) ScreenDeath(self);
    else if (doTeleport) TeleportAway(self);
    else if (isGrenade) GrenadeExplode(self);
    if (isNPC && !doTeleport) NPCDeath(self); else if (self == PLAYER1) { if (!RessurectPlayer()) World.deaths++; }
    flag_set(&e->entflags,EF_DEAD_CHECKS_DONE,true);
}

float TakeDamage(u16 self,DamageData dd) {
    if (Cheats.god && self == PLAYER1) return 0.0f;
    bool isCyber = IsCyberEntity(self); float* hp = isCyber ? &World.instances[self].cyberHealth : &World.instances[self].health;
    u16 selfIdx = World.instances[self].index;
    bool isNPC = IdxIsNPC(selfIdx), isPlayer = (self == PLAYER1);

    bool isGrenade = IsGrenade(selfIdx);

    if (isCyber) { if (dd.attackType == Att_Drill && isNPC){return 0.0f;} if (dd.attackType != Att_Drill && World.instances[self].iceActive){return 0.0f;} }
   
    if (*hp <= 0.0f) {
        bool allowPost = (isNPC || World.instances[self].iceActive || isPlayer || isGrenade || selfIdx == 279/*chunk_screen*/ || selfIdx == 477/*sec_camera*/);
        if (!allowPost) return 0.0f;
    }
    float take = dd.damage;
    if (isPlayer) {
        float absorb = 0.0f;
        if (isCyber) { if (World.invP1.hasSoft & (1 << SW_SHIELD)) { u8 sv = World.invP1.softVersions[SW_SHIELD]; absorb = (sv <= 9) ? sv * 0.05f : 0.0f; take *= (1.0f - absorb); if (take <= 0.0f){return 0.0f;} } }// Cyber C-Shield software absorption
        else {
            if (dd.attackType == Att_Magn) { take = 0.0f; TakeEnergy(11.0f); EmpStaticFlash(2.0f); BiomonitorEnergyPulse(11.0f); }
            if ((World.invP1.hardwareIsActive & HW_SHD) && (World.invP1.hasHardware & HW_SHD)) {
                float thresh = 0.0f;
                switch (World.invP1.hardwareVersion[HW_SHD_IDX]) { case 0:absorb=0.20f; thresh=0.0f; break; case 1:absorb=0.40f; thresh=10.0f; break; case 2:absorb=0.75f; thresh=15.0f; break; case 3:absorb=0.75f; thresh=30.0f; break; }
                if (take < thresh) absorb = 1.0f;
                if (absorb > 0.0f) {
                    if (absorb < 1.0f) absorb = vclamp(absorb + random_range(-0.08f,0.08f),0.0f,1.0f);
                    take *= (1.0f - absorb); play_wav(sounds[94],Sys_Settings.VolumeEffects,(V3){0.0f,0.0f,0.0f},false); // shield absorb
                    int abs = (int)(absorb * 100.0f); CenterStatusPrint("%s%d%s",Sys_Text.stringTable[208],abs,Sys_Text.stringTable[209]);

                }
            }
            if (take > 0.0f && (absorb < 0.4f || random_range(0.0f,1.0f) < 0.5f)) { play_wav(sounds[140],Sys_Settings.VolumeEffects,(V3){0.0f,0.0f,0.0f},false); PainStaticFlash(take > 15.0f ? 2.0f : take > 10.0f ? 1.0f : 0.5f); }
        }
    }

    if (isCyber) {
        World.instances[self].cyberHealth -= take;
        if (isPlayer) { World.damageReceived += take; if (World.instances[self].cyberHealth <= 0.0f) { ExitCyberspace(); return 0.0f; } }
        if (dd.owner == PLAYER1){World.damageDealt += take;}
    } else {
        if (selfIdx == 477/*Camera constIndex 477 gets one-shot by tranq*/ && dd.attackType == Att_Trnq) take = World.instances[self].health + 1.0f;
        take = ApplyAttTypeAdjustments(self,take,dd.attackType); World.instances[self].health -= take; if (isPlayer) { World.damageReceived += take; World.Sys_Music.inCombat = true; }
        if (dd.owner == PLAYER1){World.damageDealt += take;}
    }
    if (isNPC && (World.instances[self].health > 0.0f || (isCyber && World.instances[self].cyberHealth > 0.0f))) {
        if (npcTable[selfIdx - 419].timeBetweenPain > 0.0f) flag_set(&World.instances[self].entflags,EF_GO_INTO_PAIN,true);
        World.instances[self].recentMostActivator = dd.owner; // Pass attacker to NPC
        TargetIDSendDamageReceive(self,take,dd.attackType);
        AICheckPain(self); // setup enemy with NPC
    }
    if (isCyber) { if (World.instances[self].cyberHealth <= 0.0f) { if (!World.instances[self].iceActive && isNPC) {World.cyberkills++;} Death(self,false); } }
    else { if (World.instances[self].health <= 0.0f) { if (isNPC) {World.kills++;} Death(self,dd.attackType == Att_Beam); } }
    return take;
}

void HealthManagerInitAfterLoad(u16 self) {
    if (self == PLAYER1) { World.instances[self].health=211.0f; World.instances[self].cyberHealth=255.0f; World.invP1.noiseFinished = World.pauseRelativeTime - 31.0;/*guarantee no combat music on start*/ return; }
    if (IdxIsNPC(World.instances[self].index)) {
        if (IsCyberEntity(self)) { if (World.instances[self].cyberHealth < 0.0f) World.instances[self].cyberHealth = npcTable[World.instances[self].index - 419].healthForCyberNPC; }
        else { if (World.instances[self].health < 0.0f) World.instances[self].health = npcTable[World.instances[self].index - 419].health; }
        if (World.diffCbt == 0) { World.instances[self].health = 1.0f; }
        if (World.instances[self].entflags & EF_ACT_AS_CORPSE_ONLY) { World.instances[self].health = 0.0f; World.instances[self].cyberHealth = 0.0f; UseDeathTargets(self); if (World.instances[self].entflags & EF_TELEPORT_ON_DEATH){TeleportAway(self);}else{NPCDeath(self);} }
    }
}
// Hardware
void HardwareBioOff(void) { World.invP1.hardwareIsActive &= ~HW_BIO; if (Cheats.showFPS) {return;} BioMonitorClearGraphs(); }
void HardwareBioOn(void) { World.invP1.hardwareIsActive |= HW_BIO; }
void HardwareBioAction(void) { if (World.invP1.hardwareVersionSetting[HW_BIO_IDX] == 0 && World.invP1.energy <= 0.0f) { CenterStatusPrint("%s",Sys_Text.stringTable[314]); return; } play_wav(sounds[78],SfxVol(),(V3){0.0f,0.0f,0.0f},false); if ((World.invP1.hasHardware & HW_BIO) && (World.invP1.hardwareIsActive & HW_BIO)) HardwareBioOff(); else HardwareBioOn(); }
void HardwareBioClick(void) { World.Sys_UI.mouseClickHeldOverGUI = true; HardwareBioAction(); }
void HardwareSensaroundOn(void) { World.invP1.hardwareIsActive |= HW_SNS; }
void HardwareSensaroundOff(void) { World.invP1.hardwareIsActive &= ~HW_SNS; }
void HardwareSensaroundAction(void) { if (World.invP1.energy <= 0.0f) { CenterStatusPrint("%s",Sys_Text.stringTable[314]); return; } if (World.invP1.hardwareIsActive & HW_SNS) { play_wav(sounds[82],SfxVol(),(V3){0.0f,0.0f,0.0f},false); HardwareSensaroundOff(); } else { play_wav(sounds[93],SfxVol(),(V3){0.0f,0.0f,0.0f},false); HardwareSensaroundOn(); } }
void HardwareSensaroundClick(void) { World.Sys_UI.mouseClickHeldOverGUI = true; HardwareSensaroundAction(); }
void HardwareShieldOn(void) { World.invP1.hardwareIsActive |= HW_SHD; }
void HardwareShieldOff(void) { World.invP1.hardwareIsActive &= ~HW_SHD; }
void HardwareShieldOffWithEffects(void) { HardwareShieldOff(); }
void HardwareShieldAction(void) { if (World.invP1.energy <= 0.0f) { CenterStatusPrint("%s",Sys_Text.stringTable[314]); return; } if (World.invP1.hardwareIsActive & HW_SHD) { play_wav(sounds[95],SfxVol(),(V3){0.0f,0.0f,0.0f},false); HardwareShieldOffWithEffects(); } else { play_wav(sounds[96],SfxVol(),(V3){0.0f,0.0f,0.0f},false); HardwareShieldOn(); } }
void HardwareShieldClick(void) { World.Sys_UI.mouseClickHeldOverGUI = true; HardwareShieldAction(); }
void HardwareLanternOn(void) { World.invP1.hardwareIsActive |= HW_LAN; }
void HardwareLanternOff(void) { World.invP1.hardwareIsActive &= ~HW_LAN; }
void HardwareLanternAction(void) { if (World.invP1.energy <= 0.0f) { CenterStatusPrint("%s",Sys_Text.stringTable[314]); return; } play_wav(sounds[78],SfxVol(),(V3){0.0f,0.0f,0.0f},false); if (World.invP1.hardwareIsActive & HW_LAN) HardwareLanternOff(); else HardwareLanternOn(); }
void HardwareLanternClick(void) { World.Sys_UI.mouseClickHeldOverGUI = true; HardwareLanternAction(); }
void HardwareInfraredOn(void) { World.invP1.hardwareIsActive |= HW_INF; }
void HardwareInfraredOff(void) { World.invP1.hardwareIsActive &= ~HW_INF; }
void HardwareInfraredAction(void) { if (World.invP1.energy <= 0.0f) { CenterStatusPrint("%s",Sys_Text.stringTable[314]); return; } bool wasOn = (World.invP1.hardwareIsActive & HW_INF) != 0; play_wav(wasOn ? sounds[82] : sounds[98],SfxVol(),(V3){0.0f,0.0f,0.0f},false); if (wasOn) HardwareInfraredOff(); else HardwareInfraredOn(); }
void HardwareInfraredClick(void) { World.Sys_UI.mouseClickHeldOverGUI = true; HardwareInfraredAction(); }
void HardwareEReaderAction(void) { play_wav(sounds[97],SfxVol(),(V3){0.0f,0.0f,0.0f},false); World.invP1.hardwareIsActive |= HW_ERD; }
void HardwareEReaderClick(void) { World.Sys_UI.mouseClickHeldOverGUI = true; HardwareEReaderAction(); }
void HardwareBoosterOn(void)  { World.invP1.hardwareIsActive |=  HW_BST; }
void HardwareBoosterOff(void) { World.invP1.hardwareIsActive &= ~HW_BST; }
void HardwareBoosterAction(void) { if (World.invP1.hardwareVersionSetting[HW_BST_IDX] >= 1/*Set to boost not skates*/ && World.invP1.energy <= 0.0f) { CenterStatusPrint("%s",Sys_Text.stringTable[314]); return; } play_wav(sounds[78],SfxVol(),(V3){0.0f,0.0f,0.0f},false); if (World.invP1.hardwareIsActive & HW_BST) HardwareBoosterOff(); else HardwareBoosterOn(); } 
void HardwareBoosterClick(void) { World.Sys_UI.mouseClickHeldOverGUI = true; HardwareBoosterAction(); }
void HardwareJumpJetsOn(void)  { World.invP1.hardwareIsActive |=  HW_JET; }
void HardwareJumpJetsOff(void) { World.invP1.hardwareIsActive &= ~HW_JET; }
void HardwareJumpJetsAction() { if (World.invP1.energy <= 0.0f) { CenterStatusPrint("%s",Sys_Text.stringTable[314]); return; } play_wav(sounds[78],SfxVol(),(V3){0.0f,0.0f,0.0f},false); World.invP1.hardwareIsActive ^= HW_JET; if ((World.invP1.hasHardware & HW_JET) && (World.invP1.hardwareIsActive & HW_JET)) HardwareJumpJetsOn(); else HardwareJumpJetsOff(); }
void HardwareJumpJetsClick() { World.Sys_UI.mouseClickHeldOverGUI = true; HardwareJumpJetsAction(); }
#define INFRARED_RANGE 50.35f
#define LANTERN_RANGE 11.52f
static Color3 lantCol = (Color3){1.0f,1.0f,1.0f};
static float lanternVersionBrightness[3] = {0.875f,1.4f,1.75f};
void HardwareUpdate() {
    bool infraredOn = (World.invP1.hasHardware & HW_INF) && (World.invP1.hardwareIsActive & HW_INF) > 0;
    bool lanternOn = (World.invP1.hasHardware & HW_LAN) && (World.invP1.hardwareIsActive & HW_LAN) > 0;
    if (lanternOn || infraredOn) { // Update headmounted lantern/infrared's light (infrared overrides lantern brightness/range)
        V3 ppos = World.position[PLAYER1]; lanternPos = (V3){ppos.x + 0.04f,ppos.y + 0.24f,ppos.z + 0.04f};
        float intensity = infraredOn ? 0.8f : lanternVersionBrightness[vclamp(World.invP1.hardwareVersionSetting[7],0,2)];
        UpdateLight(headmountedLanternLight,lanternPos,lantCol,infraredOn ? INFRARED_RANGE : LANTERN_RANGE,intensity,intensity,0.0f,0.0f,QUAT_IDENTITY,true,true);
    } else UpdateLight(headmountedLanternLight,lanternPos,lantCol,11.52f,0.0f,0.0f,0.0f,0.0f,QUAT_IDENTITY,false,false);
}
// Dermal Patches
void PatchUpdate() {
    if (World.invP1.patchActive & PATCH_DETOX) { if (World.invP1.detoxFinished < World.pauseRelativeTime) World.invP1.patchActive -= PATCH_DETOX; } // Detox
    if (World.invP1.patchActive & PATCH_MEDI) { if (World.invP1.mediFinished < World.pauseRelativeTime && World.invP1.mediFinished != -1.0) { World.invP1.patchActive -= PATCH_MEDI; World.invP1.mediFinished = -1.0; } } // Medi
    if (World.invP1.patchActive & PATCH_REFLEX) { if (World.invP1.reflexFinishedTime < World.absoluteTime && World.invP1.reflexFinishedTime != -1.0){ World.invP1.patchActive-=PATCH_REFLEX; World.invP1.reflexFinishedTime=-1.0; World.timeScale=DEFAULT_TIME_SCALE;}else{World.timeScale=REFLEX_TIME_SCALE;}}else{if(World.timeScale != DEFAULT_TIME_SCALE){World.timeScale=DEFAULT_TIME_SCALE;}}//Reflex
    if (World.invP1.patchActive & PATCH_BERSERK) { // Berserk
        if (World.invP1.berserkFinished < World.pauseRelativeTime) {
            World.invP1.berserkIncrement = 0;
            World.invP1.patchActive -= PATCH_BERSERK;

        } else {

            if (World.invP1.berserkIncTime < World.pauseRelativeTime) {
                World.invP1.berserkIncrement++;
                if (World.invP1.berserkIncrement > 6) World.invP1.berserkIncrement = 6;
                World.invP1.berserkIncTime = World.pauseRelativeTime + (BERSERK_TIME / 5.0f);

            }
        }
    }
    if (World.invP1.patchActive & PATCH_GENIUS) { if(World.invP1.geniusFinished < World.pauseRelativeTime){World.invP1.patchActive -= PATCH_GENIUS; World.geniusActive=false;}else{World.geniusActive=true;} } // Genius
    if (World.invP1.patchActive & PATCH_SIGHT) { // Sight
        if (World.invP1.sightFinishedTime < World.pauseRelativeTime && World.invP1.sightFinishedTime != -1.0) { World.invP1.sightFinishedTime=-1.0; World.invP1.sightSideEffectFinishedTime = World.pauseRelativeTime + SIGHT_SIDE_EFFECT_TIME; }
        if (World.invP1.sightSideEffectFinishedTime < World.pauseRelativeTime && World.invP1.sightSideEffectFinishedTime != -1.0) { World.invP1.sightSideEffectFinishedTime=World.invP1.sightFinishedTime=-1.0; World.invP1.patchActive -= PATCH_SIGHT; }
    }
    if (World.invP1.patchActive & PATCH_STAMINUP) { if (World.invP1.staminupFinishedTime < World.pauseRelativeTime) { World.invP1.staminupActive=false; World.invP1.fatigue=100.0f; World.invP1.patchActive -= PATCH_STAMINUP; } else { World.invP1.fatigue = 0.0f; World.invP1.staminupActive = true; } } // Staminup
}

void PatchDisableAll(void) {
    World.invP1.berserkFinished = World.invP1.berserkIncTime = World.invP1.detoxFinished = World.invP1.geniusFinished = World.invP1.mediFinished = World.invP1.reflexFinishedTime = World.invP1.sightFinishedTime = World.invP1.sightSideEffectFinishedTime = World.invP1.staminupFinishedTime = -1.0;
    World.invP1.staminupActive=false; World.invP1.fatigue =0.0f; World.invP1.berserkIncrement = World.invP1.patchActive = 0; World.timeScale  = DEFAULT_TIME_SCALE; World.geniusActive = false;


}
// Quest Bits / Mission I/O — side effects on quest notes checklist when bits change
static void QuestBitNoteSideEffects(u8 qb, bool isOn) {
    if (isOn) {
        switch (qb) {
            case QB_ShieldActivated:       World.questNotesActive[8] = true;  World.questNotesChecked[8] = true;  break;
            case QB_LaserSafetyOverriden:   World.questNotesActive[7] = true;  World.questNotesChecked[7] = true;  break;
            case QB_LaserDestroyed:         World.questNotesActive[9] = true;  World.questNotesChecked[9] = true;  if (autoSplitter.missionSplitID == 1) autoSplitter.missionSplitID++; break;
            case QB_BetaGroveCyberUnlocked: World.questNotesActive[12] = true; break;
            case QB_GroveAlphaJettisonEnabled: World.questNotesActive[12] = true; break;
            case QB_GroveBetaJettisonEnabled:  World.questNotesActive[12] = true; break;
            case QB_GroveDeltaJettisonEnabled: World.questNotesActive[12] = true; break;
            case QB_MasterJettisonBroken:   World.questNotesActive[12] = true; World.questNotesActive[11] = true; if (autoSplitter.missionSplitID == 2) autoSplitter.missionSplitID++; break;
            case QB_Relay428Fixed:          World.questNotesActive[11] = true; World.questNotesChecked[11] = true; break;
            case QB_MasterJettisonEnabled:  World.questNotesActive[10] = true; World.questNotesChecked[10] = true; if (autoSplitter.missionSplitID == 3) autoSplitter.missionSplitID++; break;
            case QB_BetaGroveJettisoned:    World.questNotesActive[12] = true; World.questNotesChecked[12] = true; World.questNotesActive[13] = true; if (autoSplitter.missionSplitID == 4) autoSplitter.missionSplitID++; break;
            case QB_AntennaNorthDestroyed:
            case QB_AntennaSouthDestroyed:
            case QB_AntennaEastDestroyed:
            case QB_AntennaWestDestroyed:   World.questNotesActive[13] = true; break;
            case QB_SelfDestructActivated:  for (int i=0;i<17;++i) World.questNotesActive[i] = true; World.questNotesChecked[14] = true; break;
            case QB_BridgeSeparated:        for (int i=0;i<17;++i) World.questNotesActive[i] = true; World.questNotesActive[17] = true; World.questNotesChecked[16] = true; break;
            default: break;
        }
    } else {
        switch (qb) {
            case QB_ShieldActivated:       World.questNotesChecked[8] = false; break;
            case QB_LaserSafetyOverriden:   World.questNotesChecked[7] = false; break;
            case QB_LaserDestroyed:         World.questNotesChecked[9] = false; break;
            case QB_Relay428Fixed:          World.questNotesChecked[11] = false; break;
            case QB_MasterJettisonEnabled:  World.questNotesChecked[10] = false; break;
            case QB_BetaGroveJettisoned:    World.questNotesChecked[12] = false; break;
            case QB_SelfDestructActivated:  World.questNotesChecked[14] = false; break;
            case QB_BridgeSeparated:        World.questNotesChecked[16] = false; break;
            default: break;
        }
    }
}
// Ressurection: when player dies on a level with resurrection active, teleport back to the ressurection point instead of counting a death.
bool RessurectPlayer(void) {
    if (!((World.ressurectionActiveLevels >> World.curLev) & 1u)) return false;
    if (World.curLev == 10 || World.curLev == 11 || World.curLev == 12) LoadLevel(6, ressurectionLocations[6]);
    else if (World.curLev < 13) World.position[PLAYER1] = ressurectionLocations[World.curLev];
    PlayTrack(TT_Revive, MT_Override); World.invP1.ressurectingFinished = World.pauseRelativeTime + 3.0;
    CenterStatusPrint("BRAIN ACTIVITY SATISFACTORY...");
    return true;
}
// Doors
static bool DoorInventoryHasAccessCard(AccCardType card) { return card == ACC_None || (World.invP1.accessCardOwned & (1u << card)); }
static float DoorGetProgress(const Entity* e, u8 clip) { AnimationClip c = DoorGetClip(e,clip); if(c.frameEnd <= c.frameStart){return 1.0f;} return DoorClamp01((float)(e->frame - c.frameStart) / (float)(c.frameEnd - c.frameStart)); } 
static void DoorOpen(u16 self) { Entity* e = &World.instances[self]; ChangeAnim(e,ANIM_OPENING); e->doorOpen = e->doorState = DoorState_Opening; e->waitBeforeClose = World.pauseRelativeTime + e->delay; if (e->SFXIndex > 0 && e->SFXIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXIndex],1.0f,World.position[self],true); }
static void DoorClose(u16 self) { Entity* e = &World.instances[self]; ChangeAnim(e,ANIM_CLOSING); e->doorOpen = e->doorState = DoorState_Closing; if (e->SFXIndex > 0 && e->SFXIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXIndex],1.0f,World.position[self],true); }
void DoorForceOpen(u16 self) { World.instances[self].requiredAccessCard = ACC_None; EntitySetLocked(&World.instances[self],false); DoorOpen(self); }
void DoorForceClose(u16 self) { if (World.instances[self].doorOpen == DoorState_Closed) {return;} DoorClose(self); }
void DoorActuate(u16 self) {
    Entity* e = &World.instances[self]; if (e->doorOpen == DoorState_Open) { DoorClose(self); return; } if (e->doorOpen == DoorState_Closed) { DoorOpen(self); return; }
    bool op = e->doorOpen == DoorState_Opening;
    if (op || e->doorOpen == DoorState_Closing) {
        int src = op ? ANIM_OPENING : ANIM_CLOSING, dst = op ? ANIM_CLOSING : ANIM_OPENING;
        AnimationClip dstClip = DoorGetClip(e,dst); u16 newFrm = DoorFrameFromProgress(dstClip,1.0f - DoorGetProgress(e,src)); // Direct frame assignment (mid-anim reversal): clip + frame + matching model.
        e->clip = dst; e->frame = newFrm; e->currentFrameFinished = 0.0; e->modelIndex = dstClip.frameStartModelIndex + (u16)(newFrm - dstClip.frameStart); e->doorOpen = e->doorState = op ? DoorState_Closing : DoorState_Opening;
        if (!op) e->waitBeforeClose = World.pauseRelativeTime + e->delay;
        if (e->SFXIndex >= 0 && e->SFXIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXIndex], 1.0f, World.position[self], true);
    }
}

void DoorUse(u16 self, u16 activator) {
    if (activator == WORLD) return;
    Entity* e = &World.instances[self];
    if (GetCurrentLevelSecurity() > e->securityThreshold) { UIBlockedBySecurity(World.position[self]); return; }
    if (Cheats.superoverride || World.diffMis <= 0) { EntitySetLocked(e,false); e->requiredAccessCard = ACC_None; }
    if (World.diffMis <= 1) { e->requiredAccessCard = ACC_None; }
    if (e->useFinished >= World.pauseRelativeTime) return;
    e->useFinished = World.pauseRelativeTime + 0.15f;
    if (e->requiredAccessCard != ACC_None) {
        if (!DoorInventoryHasAccessCard(e->requiredAccessCard)) { CenterStatusPrint("%s",Sys_Text.stringTable[2]); if (e->SFXLockedIndex >= 0 && e->SFXLockedIndex < SOUNDS_COUNT) {play_wav(sounds[e->SFXLockedIndex],0.7f,World.position[self],true);} return; }
        else e->requiredAccessCard = ACC_None;
    }
    if ((e->entflags & EF_LOCKED) != 0) { CenterStatusPrint("%s",Sys_Text.stringTable[e->lockedMessageLingdex]); if (e->SFXLockedIndex >= 0 && e->SFXLockedIndex < SOUNDS_COUNT) {play_wav(sounds[e->SFXLockedIndex],0.55f,World.position[self],true);} return; }
    if ((e->onlyTargetOnce && !e->targetAlreadyDone) || !e->onlyTargetOnce) { e->targetAlreadyDone = true; UseTargets(self,e->targetIdx); }
    if (e->ajar) e->ajar = false;
    DoorActuate(self);
}

void DoorTargetted(u16 self, u16 activator) { if ((World.instances[self].entflags & EF_LOCKED) != 0) EntitySetLocked(&World.instances[self],false); if (!World.instances[self].targettingOnlyUnlocks) DoorUse(self,activator); }
void DoorUpdate(u16 self) {
    Entity* e = &World.instances[self]; if(e->ajar){return;}
    AnimationClip opening=DoorGetClip(e,ANIM_OPENING), closing=DoorGetClip(e,ANIM_CLOSING);
    if (e->doorOpen == DoorState_Opening && e->clip == ANIM_OPENING && e->frame >= opening.frameEnd) { e->doorOpen = e->doorState = DoorState_Open; ChangeAnim(e,ANIM_IDLE_OPEN); }
    else if (e->doorOpen == DoorState_Closing && e->clip == ANIM_CLOSING && e->frame >= closing.frameEnd) { e->doorOpen = e->doorState = DoorState_Closed; ChangeAnim(e,ANIM_IDLE_CLOSED); }
    if (World.pauseRelativeTime > e->waitBeforeClose && e->doorOpen == DoorState_Open && !e->stayOpen && !e->startOpen) DoorClose(self);
}

void CloseFullmap() {}
u16 SpawnDynamicObject(int val, bool cheat) {
    if (!IdxInBounds(val)) { DualLogError("Const index out of bounds: %u", val); return 0xFFFF; }
    if (cheat) DualLog("Cheat spawn constIndex %u, level: %u, from cheat: %u, name: ",val,World.curLev,cheat);
    if (IdxIsGeometry(val) && !Cheats.editMode) { CenterStatusPrint("Indices 0 to 306 (level chunks)\nnot possible when not on edit mode!"); return 0xFFFF; }
    if (World.instCount >= INSTANCE_COUNT) { DualLogError("Failed to spawn constIndex %u: instance table full (%u/%u)",val,World.instCount,INSTANCE_COUNT); return 0xFFFF; }
    u16 entityIndexInInstanceTable = AddInstance((u16)val, (V3){0.0f,0.0f,0.0f});
    return entityIndexInInstanceTable;
}
// TargetIO: Full game cross-level target handling.  Iterates all loaded levels, temporarily swaps active pointers via SetLevelPointers(), finds matching targetname(s), and calls Targetted().  Activator from cur level. Recursion is safe via targetIOActive flag.
void TriggerTargetted(u16 self, u16 activator) { if (World.instances[self].ignoreSecondaryTriggers) World.instances[self].recentMostActivator = activator; }
bool QuestBitIsSet(u8 qb) { return (qb < QB_COUNT) && ((World.missionBits >> qb) & 1u); }
void QuestBitSet(u8 qb)    { if (qb < QB_COUNT && !QuestBitIsSet(qb)) { World.missionBits |=  (1u << qb); QuestBitNoteSideEffects(qb, true); } }
void QuestBitClear(u8 qb)  { if (qb < QB_COUNT &&  QuestBitIsSet(qb)) { World.missionBits &= ~(1u << qb); QuestBitNoteSideEffects(qb, false); } }
void QuestBitToggle(u8 qb) { if (qb < QB_COUNT) { World.missionBits ^=  (1u << qb); QuestBitNoteSideEffects(qb, QuestBitIsSet(qb)); } }

void Targetted(u16 activator, u16 self) {
    Entity* e = &World.instances[self]; u32 aioflags = World.targetIOActive ? World.targetIOActivatorIoflags : World.instances[activator].ioflags;
    if (e->index == 699) {
        if (!e->relayEnabled) return;
        if (e->relayOnceEver) { if (e->relayAlreadyDone) return; e->relayAlreadyDone = true; }
        u32 savedFlags = World.targetIOActivatorIoflags; World.targetIOActivatorIoflags = e->ioflags;
        UseTargets(activator,e->targetIdx);
        World.targetIOActivatorIoflags = savedFlags;
        return;
    }
    if (e->index == 700) {
        if (!(aioflags & TARG_IOFLAGS_BRANCH_FLIPONLY)) {
            if (e->relayEnabled && e->currentTargetIdx != IO_NONE) {
                u32 savedFlags = World.targetIOActivatorIoflags; World.targetIOActivatorIoflags = e->ioflags;
                UseTargets(activator,e->currentTargetIdx);
                World.targetIOActivatorIoflags = savedFlags;
                e->branchOnSecond = !e->branchOnSecond; e->currentTargetIdx = e->branchOnSecond ? e->target2Idx : e->targetIdx;
            }
        }
        if (aioflags & (TARG_IOFLAGS_BRANCH_FLIP | TARG_IOFLAGS_BRANCH_FLIPONLY)) {
            e->branchOnSecond = !e->branchOnSecond; e->currentTargetIdx = e->branchOnSecond ? e->target2Idx : e->targetIdx;
        }
        return;
    }
    if (e->index == 710) { // info_mission: quest bit set/clear/toggle, or test-and-branch via target/targetIfFalse.  Mode comes from the activating targetIO bits, falling back to the info_mission's own line.
        if (e->questBitID == QB_None) return;
        u32 modeFlags = aioflags ? aioflags : e->ioflags;
        if (modeFlags & TARG_IOFLAGS_MISSION_BIT_TOGGLE) { QuestBitToggle(e->questBitID); DualLog("info_mission toggled bit %u -> %u\n",e->questBitID,(unsigned)QuestBitIsSet(e->questBitID)); return; }
        if (modeFlags & TARG_IOFLAGS_MISSION_BIT_OFF)    { QuestBitClear(e->questBitID); return; }
        if (modeFlags & TARG_IOFLAGS_MISSION_BIT_ON)     { QuestBitSet(e->questBitID); return; }
        u8 tm = e->questTestMode;
        if (!tm && activator != WORLD && activator < World.instCount) tm = World.instances[activator].questTestMode;
        if (tm) {
            bool bitOn = QuestBitIsSet(e->questBitID);
            bool pass = (tm == 1) ? bitOn : !bitOn; // 1==testQuestBitIsOn, 2==testQuestBitIsOff
            UseTargets(activator, pass ? e->targetIdx : e->targetIfFalseIdx);
        }
        return;
    }
    DualLog("Targetted a->ioflags:%u e:%u doorcond:%u\n", aioflags, e->index, ((aioflags & TARG_IOFLAGS_DOOROPEN) && IdxIsDoor(e->index)));
    if (e->index == 709) { CenterStatusPrint("%s", Sys_Text.stringTable[e->messageLingdex]); return; } // info_message
    if (e->index == 708) { World.gameFinished = true; return; }
    if (e->index == 707) { EmailTargetted(self); return; } // info_email
    if (aioflags & TARG_IOFLAGS_TRIPTRIGGER) { if(e->index == 598 || e->index == 600){TriggerTargetted(self,activator);}else if(e->index == 594){TriggerCounterTargetted(self,activator);} }
    if (aioflags & TARG_IOFLAGS_UNLOCK) EntitySetLocked(e, false);
    if ((aioflags & TARG_IOFLAGS_LOCK) && IdxIsDoor(e->index)) EntitySetLocked(e, true);
    if (IdxIsButtonSwitch(e->index)) ButtonSwitchTargetted(self, activator);
    if ((aioflags & TARG_IOFLAGS_DOOROPEN) && IdxIsDoor(e->index)) { DoorForceOpen(self); } 
    else if ((aioflags & TARG_IOFLAGS_DOOROPENIFUNLOCKED) && IdxIsDoor(e->index) && (e->entflags & EF_LOCKED) == 0 && (e->requiredAccessCard == ACC_None || (World.invP1.accessCardOwned & (1u << e->requiredAccessCard)))) { DoorForceOpen(self); }
    else if ((aioflags & TARG_IOFLAGS_DOORCLOSE) && IdxIsDoor(e->index)) { DoorForceClose(self); }
    else if (IdxIsDoor(e->index)) { DoorTargetted(self, activator); }
    if (aioflags & TARG_IOFLAGS_FBRIDGE_ACTIVATE) ForceBridgeActivate(self, false);
    else if (aioflags & TARG_IOFLAGS_FBRIDGE_DEACTIVATE) ForceBridgeDeactivate(self, false);
    else if (aioflags & TARG_IOFLAGS_FBRIDGE_TOGGLE) ForceBridgeToggle(self);
    if (aioflags & TARG_IOFLAGS_GRAVLIFT_TOGGLE) World.instances[self].active=!World.instances[self].active;
    if (aioflags & TARG_IOFLAGS_TEXTURE_CHG_TOGGLE) TextureChangerToggle(self);
    if (aioflags & TARG_IOFLAGS_FUNCWALL_MOVE) FuncWallTargetted(self);
    if (aioflags & TARG_IOFLAGS_SWITCH_LOCK_TOGGLE) EntitySetLocked(e, (e->entflags & EF_LOCKED) == 0);
    if (aioflags & TARG_IOFLAGS_INST_ACTIVATE) flag_set(&e->entflags, EF_ACTIVE, true);
    else if (aioflags & TARG_IOFLAGS_INST_DEACTIVATE) flag_set(&e->entflags, EF_ACTIVE, false);
    else if (aioflags & TARG_IOFLAGS_INST_TOGGLE) flag_set(&e->entflags, EF_ACTIVE, !(e->entflags & EF_ACTIVE));
}

void UseTargets(u16 activator, u16 targetIdx) {
    if(targetIdx == IO_NONE){return;} bool wasActive=World.targetIOActive, succeeded=false; u8 entryLevel = World.currentLevel;
    if (!wasActive) { World.targetIOActive = true; World.targetIOEntryLevel = entryLevel; World.targetIOActivatorIdx = activator; World.targetIOActivatorEntity = World.instances[activator]; World.targetIOActivatorIoflags = World.instances[activator].ioflags; }
    const char* targetname = IOName(targetIdx); // For logging only; matching is u16 compare against the interned table.
    for (u8 lev = 0; lev < World.numLevels; ++lev) {
        if (World.currentLevel != lev) SetLevelPointers(lev);
        for (u16 i = INSTS_1ST_IDX; i < World.instCount; ++i) { if (World.instances[i].targetnameIdx != targetIdx) {continue;} DualLog("Target hit: %s on %u (lev %u), timestamp: %f\n",targetname,i,lev,World.pauseRelativeTime); Targetted(activator,i); succeeded=true; }
    }
    if (World.currentLevel != entryLevel) {SetLevelPointers(entryLevel);} if (!succeeded) {DualLogWarn("No target found: %s\n",targetname);} if (!wasActive) {World.targetIOActive=false;}
}
// Frob/Use
void SearchObject(int searchable) {
    World.Sys_UI.highlightStatus[MM_NOTES] = true; World.Sys_UI.highlightTickCount[MM_NOTES] = 3; World.Sys_UI.tickFinished = World.pauseRelativeTime;
    if (World.instances[searchable].searchableInUse) { for (int i=0;i<4;i++) { if (World.instances[searchable].contents[i] >= 0) break; } } else play_wav(sounds[91],0.75f,(V3){0.0f,0.0f,0.0f},false);
}

static int UseNameTableIndex(int index) {
    switch (index) {
        case 0:return 925; case 1:return 926; case 2:return 54; case 3:return 54; case 4: return 54; case 5: return 54; case 6: return 54; case 7: return 54; case 8: return 54; case 9: return 54; case 10: return 54; case 11: return 55; case 12: return 57;
        case 13: return 58; case 14: return 59; case 15: return 928; case 16: return 61; case 17: return 929; case 18: return 62; case 19: return 63; case 20: return 927; case 23: return 82; case 24: return 930; case 25: return 84; case 26: return 931;
        case 27: return 85; case 28: return 932; case 29: return 86; case 30: return 85; case 31: return 85; case 32: return 85; case 33: return 932; case 34: return 88; case 35: return 933; case 36: return 90; case 37: return 934; case 38: return 91;
        case 39: return 92; case 40: return 935; case 41: return 94; case 42: return 94; case 43: return 94; case 44: return 94; case 45: return 936; case 46: return 95; case 47: return 937; case 48: return 97; case 49: return 938; case 50: return 98;
        case 51: return 99; case 52: return 99; case 53: return 939; case 54: return 100; case 55: return 940; case 56: return 102; case 57: return 941; case 58: return 103; case 59: return 103; case 60: return 942; case 61: return 104; case 62: return 105;
        case 63: return 105; case 64: return 943; case 65: return 944; case 66: return 943; case 67: return 103; case 68: return 942; case 69: return 103; case 70: return 108; case 71: return 593; case 72: return 110; case 73: return 110; case 74: return 945;
        case 75: return 108; case 76: return 112; case 77: return 113; case 78: return 946; case 79: return 947; case 80: return 114; case 81: return 114; case 82: return 115; case 83: return 115; case 84: return 948; case 85: return 115; case 86: return 115;
        case 87: return 115; case 88: return 82; case 89: return 949; case 90: return 114; case 91: return 114; case 92: return 114; case 93: return 117; case 94: return 118; case 95: return 118; case 96: return 118; case 97: return 119; case 98: return 120;
        case 99: return 120; case 100: return 120; case 101: return 950; case 102: return 951; case 103: return 950; case 104: return 952; case 105: return 953; case 106: return 952; case 107: return 953; case 108: return 951; case 109: return 120;
        case 110: return 120; case 111: return 120; case 112: return 954; case 113: return 955; case 114: return 956; case 115: return 957; case 116: return 958; case 117: return 959; case 118: return 130; case 119: return 960; case 120: return 130;
        case 121: return 131; case 122: return 130; case 124: return 126; case 125: return 961; case 126: return 132; case 127: return 86; case 128: return 962; case 129: return 963; case 130: return 116; case 131: return 964; case 132: return 134;
        case 133: return 964; case 134: return 134; case 135: return 965; case 136: return 931; case 137: return 964; case 138: return 134; case 139: return 967; case 140: return 966; case 141: return 135; case 142: return 135; case 143: return 135;
        case 144: return 136; case 145: return 136; case 146: return 136; case 147: return 136; case 148: return 968; case 149: return 969; case 150: return 969; case 151: return 969; case 152: return 969; case 153: return 969; case 154: return 970;
        case 155: return 138; case 156: return 971; case 157: return 972; case 158: return 973; case 159: return 969; case 160: return 140; case 161: return 140; case 162: return 141; case 163: return 141; case 164: return 141; case 165: return 141;
        case 166: return 141; case 167: return 974; case 168: return 974; case 169: return 140; case 170: return 975; case 171: return 976; case 172: return 976; case 173: return 976; case 174: return 976; case 175: return 976; case 176: return 976;
        case 177: return 976; case 178: return 144; case 179: return 144; case 180: return 977; case 181: return 144; case 182: return 142; case 183: return 977; case 184: return 142; case 185: return 978; case 186: return 979; case 187: return 980;
        case 188: return 956; case 189: return 146; case 190: return 142; case 191: return 142; case 192: return 142; case 193: return 142; case 194: return 981; case 195: return 982; case 196: return 147; case 197: return 148; case 198: return 148;
        case 199: return 106; case 200: return 106; case 201: return 149; case 202: return 594; case 203: return 151; case 204: return 152; case 205: return 153; case 206: return 154; case 207: return 595; case 208: return 631; case 209: return 157;
        case 210: return 157; case 211: return 157; case 212: return 157; case 213: return 157; case 214: return 157; case 215: return 157; case 216: return 157; case 217: return 157; case 218: return 157; case 219: return 157; case 220: return 158;
        case 221: return 983; case 222: return 159; case 223: return 160; case 224: return 984; case 225: return 106; case 226: return 106; case 227: return 985; case 228: return 111; case 229: return 106; case 230: return 106; case 231: return 165;
        case 232: return 164; case 233: return 164; case 234: return 594; case 235: return 166; case 236: return 166; case 237: return 166; case 238: return 986; case 239: return 132; case 240: return 987; case 241: return 167; case 242: return 167;
        case 243: return 167; case 244: return 167; case 245: return 167; case 246: return 167; case 247: return 167; case 248: return 167; case 249: return 167; case 250: return 988; case 251: return 169; case 252: return 169; case 253: return 167;
        case 254: return 167; case 255: return 167; case 256: return 82; case 257: return 930; case 258: return 170; case 259: return 989; case 260: return 990; case 261: return 991; case 262: return 992; case 263: return 992; case 264: return 992;
        case 265: return 993; case 266: return 82; case 267: return 930; case 268: return 167; case 269: return 167; case 270: return 173; case 271: return 994; case 272: return 176; case 273: return 995; case 274: return 176; case 275: return 174;
        case 276: return 996; case 277: return 178; case 278: return 177; case 279: return 47; case 280: return 180; case 281: return 180; case 282: return 180; case 283: return 180; case 284: return 180; case 285: return 180; case 286: return 180;
        case 287: return 180; case 288: return 181; case 289: return 181; case 290: return 107; case 291: return 107; case 292: return 182; case 293: return 997; case 294: return 182; case 295: return 182; case 296: return 182; case 297: return 183;
        case 298: return 183; case 299: return 183; case 300: return 183; case 301: return 183; case 302: return 126; case 303: return 126; case 304: return 961; case 477: return 1027; case 478: return 1029; case 479: return 1028; case 656: return 1030;
        case 519: return 1044; case 520: return 1044; case 521: return 1044; case 522: return 1044; case 523: return 1044; case 657: return 1030; case 658: return 1030; case 659: return 1030; case 660: return 1030; case 661: return 1030; case 662: return 1030;
        case 663: return 1030; case 664: return 1030; case 665: return 1030; case 666: return 1030; case 667: return 1034; case 668: return 1035; case 669: return 1036; case 670: return 1037; case 671: return 1038; case 672: return 1039; case 673: return 1040;
        case 674: return 1041; case 675: return 1042; case 676: return 1043; case 677: return 1033; case 678: return 1033; case 679: return 1033; case 680: return 1032; case 681: return 1032; case 682: return 1031; case 683: return 1031; case 684: return 1031;
        case 685: return 1031; case 686: return 1031; case 687: return 1030; default: return -1; // No name available; caller will print just the prefix
    }
}

void UseEntity(u16 i) {
    Entity* ent = &World.instances[i];
    if (IdxIsSearchable(ent->index)) { World.invP1.currentSearchItem = i; SearchObject(i); CenterStatusPrint("Search\n"); }
    else if (IdxIsDoor(ent->index)) DoorUse(i,PLAYER1);
    else if (IdxIsNPC(ent->index)) CenterStatusPrint("%s%s",Sys_Text.stringTable[29],npcTable[World.instances[i].index - 419].name);
    else if (IdxIsButtonSwitch(ent->index)) ButtonSwitchUse(i,PLAYER1);
    else if (IdxIsGeometry(ent->index)) { int t = UseNameTableIndex(ent->index); CenterStatusPrint("%s%s",Sys_Text.stringTable[29],t >= 0 ? Sys_Text.stringTable[t] : ""); }
    else if (IdxIsUsableObject(ent->index)) {
        World.invP1.holdingObject = true; World.invP1.heldObjectIndex = ent->index; World.invP1.heldObjectCustIdx = ent->usableCustIdx; World.invP1.heldObjectAmmo = ent->ammo; World.invP1.heldObjectAmmo2 = ent->ammo2; World.invP1.heldObjectLoadedAlternate = ent->heldObjectLoadedAlternate;
        if (Sys_Settings.QuickItemPickup) { AddItemToInventory(ent->index,ent->usableCustIdx); ResetHeldItem(); }
        else { CenterStatusPrint("%s%s",Sys_Text.stringTable[World.invP1.heldObjectIndex - 307 + 326],Sys_Text.stringTable[319]); /* picked up.*/ ForceInventoryMode(); } // Inventory mode is turned on when picking something up
        DeleteInstance(i);
    } else { int t = UseNameTableIndex(ent->index); CenterStatusPrint("%s%s",Sys_Text.stringTable[29],t >= 0 ? Sys_Text.stringTable[t] : ""); }
}

#define FROB_DISTANCE 4.9f
static void Frob(V3 pos, V3 forward, V3 right) {
    if (World.uiIsBlocking || World.curLev == LEVEL_CYBERSPACE){return;}    if(World.Sys_UI.vmailActive){World.Sys_UI.vmailActive=0; return;}    if(World.invP1.holdingObject){DropHeldItem(); return;}
    V3 dir = ScreenPointToRay(forward, right); RaycastHit h = Raycast(pos,dir,FROB_DISTANCE,LMASK_PLAYER_FROB);
    if (Cheats.showPhys) { World.debugLine_start = pos; World.debugLineFinished = World.pauseRelativeTime + 3.0; World.debugLine_end = h.hit ? h.point : (V3){dir.x * FROB_DISTANCE + pos.x, dir.y * FROB_DISTANCE + pos.y, dir.z * FROB_DISTANCE + pos.z}; }
    if (!h.hit) { CenterStatusPrint("%s", Sys_Text.stringTable[30]); } else { UseEntity(h.hitInstanceIndex); }
}
// Update
void WeaponsUpdate(); void TextureSequenceUpdate(u16 self); void AIAnimationControllerUpdate(u16 selfIdx); void AIControllerUpdate(u16 selfIdx); void DrawSphereWireframe(Color col, ShapeSphere s);
extern float sightPointHeights[NUM_AI_TYPES];
void DrawAIDebug(u16 i) {
    if (!IdxIsNPC(World.instances[i].index)) return;
    if (!Cheats.showNPC) return;
    World.layer[i] = L_NPC; World.layer[PLAYER1] = L_Player;
    Quaternion r = World.rotation[i]; float x=r.x,y=r.y,z=r.z,w=r.w;
    V3 fwd = V3_Normalize((V3){2.0f*(x*z + w*y), 0.0f, 1.0f - 2.0f*(x*x + y*y)});
    u16 npcIdx = World.instances[i].index - 419;
    V3 sightPt = V3_AplusB(World.position[i],(V3){0.0f,sightPointHeights[npcIdx],0.0f});
    AddWireLine(sightPt,V3_AplusB(sightPt,V3_ScaleByF(fwd,0.6f)),(Color){1.0f,1.0f,0.0f,1.0f});
    V3 enemPt = World.position[PLAYER1]; enemPt.y -= 0.24f;
    RaycastHit hit = Raycast(sightPt,V3_AsubB(enemPt,sightPt),20.0f,LMASK_NPC_SIGHT);
    if (hit.hit && hit.hitInstanceIndex == PLAYER1) { AddWireLine(sightPt,hit.point,(Color){1.0f,0.0f,0.0f,1.0f}); } else {AddWireLine(sightPt,enemPt,(Color){0.0f,1.0f,1.0f,1.0f});}
    Entity* e = &World.instances[i];
    Color dbgCol;
    if (e->currentState == AIState_Idle) dbgCol = (Color){0.0f,1.0f,0.0f,1.0f};
    else if (e->currentState == AIState_Walk || e->currentState == AIState_Run) {
        if (e->entflags & EF_ENEM_IN_SIGHT) dbgCol = (Color){1.0f,0.0f,0.0f,1.0f};
        else dbgCol = (Color){1.0f,1.0f,0.0f,1.0f};
    } else if (e->currentState == AIState_Attack1 || e->currentState == AIState_Attack2 || e->currentState == AIState_Attack3) dbgCol = (Color){1.0f,0.0f,1.0f,1.0f};
    else if (e->currentState == AIState_Pain) dbgCol = (Color){1.0f,0.0f,1.0f,1.0f};
    else if (e->currentState == AIState_Dead) dbgCol = (Color){0.5f,0.5f,0.5f,1.0f};
    else { dbgCol = (Color){1.0f,0.9f,0.8f,1.0f}; }
    DrawSphereWireframe(dbgCol, (ShapeSphere){sightPt, 0.32f});
}

void ModUpdate() {
    if (World.paused || World.menuActive) return;
    WeaponsUpdate(); PatchUpdate(); HardwareUpdate();
    if (Use()) Frob(World.position[PLAYER1],World.instances[PLAYER1].forward,World.instances[PLAYER1].right);
    if (World.pauseRelativeTime < World.debugLineFinished && (World.debugLineVertCount + 6) < (MAX_WIRELINE_VRTS * 3)) AddWireLine(World.debugLine_start,World.debugLine_end,(Color){0.3f,0.1f,0.6f,0.5f});
    for (u16 i=INSTS_1ST_IDX;i<World.instCount;++i) {
        Entity* e = &World.instances[i]; u16 constdex = e->index;
        DelayedSpawnUpdate(i);
        if (e->textureAnimating && e->tickFinished < World.pauseRelativeTime) TextureSequenceUpdate(i);
        if(IdxIsButtonSwitch(constdex)){ButtonSwitchUpdate(i);} if(IdxIsDoor(constdex)){DoorUpdate(i);}    if(constdex == 701){LogicTimerUpdate(i);} if(e->itemLifeTime > 0.0f){SearchFXResetUpdate(i);}
        if(e->cyberTimer > 0.0f){CyberTimerUpdate(i);}          if(constdex == 515){ForceBridgeUpdate(i);} if(constdex == 517){FuncWallUpdate(i);}   if(constdex == 21 || constdex == 22){CyberWallUpdate(i);}
        //if(IdxIsNPC(constdex)) { AIControllerUpdate(i); AIAnimationControllerUpdate(i); }
        if(IdxIsNPC(constdex)) { DrawAIDebug(i); }
    }
}

u16 GetCrosshairTexture() { switch(World.invP1.weaponIndex) { case 36:case 38:case 43:case 45:case 48:return 1121;/*red*/case 37:case 40:case 50:return 1253;/*blue*/case 41:case 42:return 1166;/*orange*/case 44:case 47:return 1122;/*yellow*/ case 46:case 51:return 1161;/*teal*/default:return 1260;/*green*/ } }
u16 GetCursorTexture() {
    if(World.paused||World.menuActive)return 1261;/*Red standard cursor*/if(!World.invP1.holdingObject)return GetCrosshairTexture();
    switch(World.invP1.heldObjectIndex){
        case 312: return 605;/*item_arm*/                 case 313: return 606;/*item_audiolog*/            case 364: return 969;/*item_chipset_interfacedemod*/ case 308: return 838;/*item_paper_wad*/            case 309: return 764;/*item_beaker*/ 
        case 310: return 767;/*item_beverage*/            case 311: return 981;/*item_skull*/               case 314: return 853;/*weapon_grenadefrag*/          case 315: return 849;/*weapon_grenadeconc*/        case 316: return 851;/*weapon_grenadeemp*/ 
        case 317: return 850;/*weapon_grenadeearth*/      case 318: return 860;/*weapon_grenademine*/       case 319: return 861;/*weapon_grenadenitro*/         case 320: return 859;/*weapon_grenadegas*/         case 321: return 974;/*item_patch_berserk*/
        case 322: return 975;/*item_patch_detox*/         case 323: return 976;/*item_patch_genius*/        case 324: return 977;/*item_patch_medi*/             case 325: return 978;/*tem_patch_reflex*/          case 326: return 979;/*item_patch_sight*/ 
        case 327: return 980;/*item_patch_staminup*/      case 328: return 882;/*item_hw_system*/           case 329: return 907;/*item_hw_navunit*/             case 330: return 902;/*item_hw_ereader*/           case 331: return 909;/*item_hw_sensaround*/ 
        case 332: return 935;/*item_hw_targetid*/         case 333: return 911;/*item_hw_shield*/           case 334: return 900;/*item_hw_bio*/                 case 335: return 906;/*item_hw_lantern*/           case 336: return 903;/*item_hw_envirosuit*/
        case 337: return 901;/*item_hw_booster*/          case 338: return 905;/*item_hw_jumpjets*/         case 339: return 904;/*item_hw_infrared*/            case 340: return 966;/*item_fireextinguisher*/     case 341: return 626;/*item_access_card_admin*/
        case 342: return 845;/*item_workerhelmet*/        case 343: return 988;/*weapon_mk3*/               case 344: return 982;/*weapon_blaster*/              case 345: return 983;/*weapon_dartgun*/            case 346: return 984;/*weapon_flechette*/
        case 347: return 985;/*weapon_ionrifle*/          case 348: return 1034;/*weapon_rapier*/           case 349: return 990;/*weapon_pipe*/                 case 350: return 986;/*weapon_magnum*/             case 351: return 987;/*weapon_magpulse*/
        case 352: return 1010;/*weapon_pistol*/           case 353: return 1019;/*weapon_plasma*/           case 354: return 1027;/*weapon_railgun*/             case 355: return 1035;/*weapon_riotgun*/           case 356: return 1036;/*weapon_skorpion*/
        case 357: return 1052;/*weapon_sparqbeam*/        case 358: return 1065;/*weapon_stungun*/          case 359: return 965;/*item_battery*/                case 360: return 968;/*item_battery_icad*/         case 361: return 972;/*item_logic_probe*/
        case 362: return 967;/*item_healthkit*/           case 363: return 973;/*item_plastique*/           case 365: return 766;/*item_flask*/                  case 366: return 969;/*item_chipset_bitflag*/      case 367: return 549;/*item_ammo_rubber*/
        case 368: return 971;/*item_isotopex22*/          case 369: return 765;/*it442em_testtube*/         case 370: return 853;/*weapon_grenadefrag_live*/     case 371: return 970;/*item_chipset_isolinear*/    case 372: return 849;/*weapon_grenadeconc_live*/
        case 373: return 420;/*item_ammo_needle*/         case 374: return 602;/*item_ammo_tranq*/          case 375: return 593;/*item_ammo_standard*/          case 376: return 597;/*item_ammo_teflon*/          case 377: return 411;/*item_ammo_hollow*/
        case 378: return 561;/*item_ammo_slug*/           case 379: return 419;/*item_ammo_magnesium*/      case 380: return 421;/*item_ammo_penetrator*/        case 381: return 417;/*item_ammo_hornet*/          case 382: return 577;/*item_ammo_splinter*/
        case 383: return 422;/*item_ammo_rail*/           case 384: return 551;/*item_ammo_slag*/           case 385: return 552;/*item_ammo_slaglarge*/         case 386: return 418;/*item_ammo_magcart*/         case 387: return 851;/*weapon_grenadeemp_live*/
        case 388: return 762;/*item_access_card_std*/     case 389: return 850;/*weapon_grenadeearth_live*/ case 390: return 610;/*item_access_card_group1*/     case 391: return 621;/*item_access_card_science*/  case 392: return 609;/*item_access_card_eng*/
        case 393: return 610;/*item_access_card_groupB*/  case 394: return 635;/*item_access_card_security*/case 395: return 761;/*item_access_card_per5diego*/  case 396: return 632;/*item_access_card_medi*/     case 397: return 610;/*item_access_card_group3*/
        case 398: return 624;/*item_access_card_purple*/  case 399: return 872;/*item_head_male*/           case 400: return 862;/*item_head_female*/            case 401: return 872;/*item_severedhead*/          case 402: return 860;/*weapon_grenademine_live*/
        case 403: return 861;/*weapon_grenadenitro_live*/ case 404: return 859;/*weapon_grenadegas_live*/   case 417: return 760;/*item_access_card_perdarcy*/
    }
    return 1250;/*paper wad fallback*/
}
