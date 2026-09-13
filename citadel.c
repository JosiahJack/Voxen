// citadel.c - Game logic.
#include "common.h"
__attribute__((used)) AutoSplitterData autoSplitter = {0x1337133713371337,0,false,0}; static const u16 patchMsg[7] = {325,326,327,328,329,330,331}; void BiomonitorEnergyPulse(float),BioMonitorClearGraphs(void); bool RecentLog(); extern double lerpStartTime; extern V3 queuedLevelPos; extern u8 queuedLevelToLoad;
V3 ScreenPointToRay(V3 fwd, V3 rt) {
    float tanFov = vtan((float)Sys_Settings.FOV * 0.5f * PI / 180.0f), ndcX = ((World.inventoryMode ? World.cursorPos_x : 683.0f) - 683.0f) / 384.0f, ndcY = -((World.inventoryMode ? World.cursorPos_y : 384.0f) - 384.0f) / 384.0f;
    V3 view = V3_Normalize((V3){ndcX * tanFov,ndcY * tanFov,-1.0f}), flipForward = (V3){-fwd.x,-fwd.y,-fwd.z}; V3 up = V3_Normalize(V3_Cross(rt,flipForward));
    return (V3){view.x*rt.x + view.y*up.x + view.z*flipForward.x,view.x*rt.y + view.y*up.y + view.z*flipForward.y,view.x*rt.z + view.y*up.z + view.z*flipForward.z};
}

void ResetHeldItem() { World.invP1.heldObjectIndex=World.invP1.heldObjectCustIdx=U16_MAX; World.invP1.heldAmmo=World.invP1.heldAmmo2=0; World.invP1.heldObjectLoadedAlternate=World.invP1.holdingObject=World.invP1.grenActive=false; }
void DropHeldItem() {
    if (World.invP1.heldObjectIndex >= World.instCount) { ResetHeldItem(); return; }    if (World.invP1.dropFinished > World.pauseRelativeTime) {return;}
    World.invP1.dropFinished = World.pauseRelativeTime + 0.2;/*Prevent immediate re-grab at high fps*/ u16 n = AddInstance(World.invP1.heldObjectIndex,World.position[PLAYER1]);
    Entity* e = &World.instances[n]; e->usableCustIdx = World.invP1.heldObjectCustIdx; e->ammo = World.invP1.heldAmmo; e->ammo2 = World.invP1.heldAmmo2; e->heldObjectLoadedAlternate = World.invP1.heldObjectLoadedAlternate;
    flag_set(&e->entflags,EF_RIGIDBODY,true); V3 tossDir = ScreenPointToRay(World.instances[PLAYER1].forward,World.instances[PLAYER1].right); World.position[n] = V3_AplusB(World.position[PLAYER1],V3_ScaleByF(tossDir,0.48f)); World.velocity[n] = V3_ScaleByF(tossDir,10.0f); ResetHeldItem();
}

void PatchUse(int patchSlot) {
    if (patchSlot < 0 || patchSlot > 6) return; if (World.invP1.patchCounts[patchSlot] <= 0) { CenterStatusPrint("%s", Sys_Text.stringTable[324]); return; } World.invP1.patchCounts[patchSlot]--;
    World.invP1.patchActive |= (u16)(1u << patchSlot);
    switch (patchSlot) {
        case 0: if(World.invP1.berserkFinished > World.pauseRelativeTime){World.invP1.berserkFinished += BERSERK_TIME;} else{World.invP1.berserkFinished = World.pauseRelativeTime + BERSERK_TIME; World.invP1.berserkIncTime = World.pauseRelativeTime + (BERSERK_TIME / 5.0); World.invP1.berserkIncrement = 0;} break;
        case 1: World.invP1.detoxFinished        = World.pauseRelativeTime + DETOX_TIME; World.invP1.radiated = 0.0f; break;                     case 2: World.invP1.geniusFinished       = World.pauseRelativeTime + GENIUS_TIME; World.geniusActive = true; break;
        case 3: World.invP1.mediFinished         = World.pauseRelativeTime + MEDI_TIME; break;                                                   case 4: World.invP1.reflexFinishedTime   = World.absoluteTime + REFLEX_TIME; World.timeScale = REFLEX_TIME_SCALE; break;/*TODO Handle restoring offset from absolute time at loading savegame*/
        case 5: World.invP1.sightFinishedTime    = World.pauseRelativeTime + SIGHT_TIME; World.invP1.sightSideEffectFinishedTime = -1.0; break;  case 6: World.invP1.staminupFinishedTime = World.pauseRelativeTime + STAMINUP_TIME; World.invP1.staminupActive = true; World.invP1.fatigue = 0.0f; break;
    }
    CenterStatusPrint("%s",Sys_Text.stringTable[patchMsg[patchSlot]]); if (World.invP1.patchCounts[World.invP1.patchCur] <= 0) { for (int i = 0; i < 7; i++) { if (World.invP1.patchCounts[i] > 0) { World.invP1.patchCur = (i8)i; break; } } } play_wav(sounds[88],SfxVol(),(V3){0.0f,0.0f,0.0f},false);
}

void WeaponFireStartWeaponDip(float t) { if (t <= 0.0f) { World.invP1.reloadFinished = 0.0; return; } World.invP1.reloadFinished = World.pauseRelativeTime + (double)t; lerpStartTime = World.pauseRelativeTime; }
void CompleteWeaponChange(void); // Forward declaration from weapons.c
void WeaponFireCompleteWeaponChange(void) { World.invP1.justChangedWeap = false; World.invP1.recoiling = false; /* CompleteWeaponChange called by UpdateWeaponReloadDip when reloadLerpValue >= 0.5f after reload dip */ }
bool InventoryHasAccessCard(AccCardType card) { return (World.invP1.accessCardOwned & (1u << card)) != 0; }
bool InventoryHasAnyAccessCards() { return World.invP1.accessCardOwned != 0; }
const char* AccessCardCodeForType(AccCardType a) { // Called by ItemTabManager
    switch(a) {
        case ACC_Std: return "STD"; case ACC_Med: return "MED"; case ACC_Sci: return "SCI";  case ACC_Admin:return "ADM"; case ACC_Grp1: return "Group-1"; case ACC_Grp2:return "Group-2"; case ACC_Grp3:return "Group-3"; case ACC_Grp4:return "Group-4"; case ACC_GrpA:return "Group-A"; case ACC_GrpB:return "Group-B";
        case ACC_Stor:return "STO"; case ACC_Eng: return "ENG"; case ACC_Maint:return "MTN"; case ACC_Security:return "SEC"; case ACC_Per1:return "PER-1"; case ACC_Per2:return "PER-2";   case ACC_Per3:return "PER-3";   case ACC_Per4:return "PER-4";   case ACC_Per5:return "PER-5";
    } return "Group-2";
}

void AddAccessCardToInventory(int index) {
    AccCardType card;
    switch(index) {
        case 34:card=ACC_Admin; break; case 81:card=ACC_Std;  break; case 83:card=ACC_Grp1; break; case  84:card=ACC_Sci;  break; case 85:card=ACC_Eng; break; case 86:card=ACC_GrpB; break; case 87:card=ACC_Security; break; case 88:card=ACC_Per5; break; case 89:card=ACC_Med;   break; case 90:card=ACC_Grp3; break; case 91:card=ACC_Grp4; break; case 110:card=ACC_Per1; break;
        default: CenterStatusPrint("BUG: Unmarked access card, defaulting to STD."); card = ACC_Std; break;
    }
    if (index == 87) { // Command card = STO + SEC + MTN
        if (InventoryHasAccessCard(ACC_Stor) && InventoryHasAccessCard(ACC_Security) && InventoryHasAccessCard(ACC_Maint)) { CenterStatusPrint("%s%s",Sys_Text.stringTable[44],AccessCardCodeForType(card)); return; }
        World.invP1.accessCardOwned |= (1u<<ACC_Stor)|(1u<<ACC_Security)|(1u<<ACC_Maint); CenterStatusPrint("%s%s, %s, %s",Sys_Text.stringTable[45],AccessCardCodeForType(ACC_Stor),AccessCardCodeForType(ACC_Security),AccessCardCodeForType(ACC_Maint)); return;
    }
    if (InventoryHasAccessCard(card)) { CenterStatusPrint("%s%s",Sys_Text.stringTable[44],AccessCardCodeForType(card)); return; } World.invP1.accessCardOwned |= (1u << card); CenterStatusPrint("%s%s",Sys_Text.stringTable[45],AccessCardCodeForType(card));
}

void AddHardwareToInventory(int index,int hwversion) {
    if (hwversion > 0 && hwversion <= (int)World.invP1.hwVers[index]) { CenterStatusPrint("%s",Sys_Text.stringTable[46]);/*THAT WARE IS OBSOLETE. DISCARDED.*/ return; }
    static const u8 textIdx[12] = {21,22,23,24,25,26,27,28,29,30,31,32}; World.invP1.hardwareInvIndex = index; World.invP1.hasHardware |= (u16)(1u << index); World.invP1.hwVers[index] = (u8)hwversion; World.invP1.hwVersSetting[index]= hwversion > 0 ? (u8)(hwversion - 1) : 0; CenterStatusPrint("%s v%d",Sys_Text.stringTable[textIdx[index] + 326],hwversion);
}

bool AddGeneralObjectToInventory(int index, int custIdx) {
    for (i8 i=1;i<14;++i) {
        if (World.invP1.generalInventoryIndexRef[i] == -1) { if(!InventoryHasAnyAccessCards() && World.invP1.generalInvCurrent == 0){World.invP1.generalInvCurrent=i;} World.invP1.generalInventoryIndexRef[i]=index; World.invP1.generalInvCustIdx[i]=(i16)custIdx; CenterStatusPrint("%s%s",Sys_Text.stringTable[ItemStringIdx(index)],Sys_Text.stringTable[31]); return true; }
    } return false;
}

void CheckForUnreadLogs() { int e=0,l=0; for (int i=0;i<LOGCNT;++i) if (World.invP1.hasLog[i] && !World.invP1.readLog[i]) *(Sys_Text.audioLogType[i] == AudioLogType_Email ? &e : &l)=1; World.invP1.hasNewEmail=e; World.invP1.hasNewLogs=l; }
static int FindNextUnreadLog() { for (int i = LOGCNT-1; i >= 0; i--) { if(World.invP1.hasLog[i] && !World.invP1.readLog[i]){return i;} } return -1; }
static void PlayLog(int logIndex) {
    if(logIndex<0||logIndex>=LOGCNT||!(World.invP1.hasHardware&HW_ERD)){return;} play_message(AudioLogPath(logIndex)); World.invP1.readLog[logIndex]=true; if(Sys_Text.audioLogType[logIndex] == AudioLogType_Vmail){World.Sys_UI.vmailActive=true; /*World.invP1.vmailLogIndex=(i16)logIndex; TODO*/} CenterStatusPrint("%s%s",Sys_Text.stringTable[1020],World.audiologNames[logIndex]);
}

void PlayLastAddedLog(int logIndex) { if(logIndex < 0){return;} PlayLog(logIndex); World.invP1.lastAddedIndex = -1; }
void AddAudioLogToInventory(int index) {
    if (index < 0) { DualLog("BUG: Audio log picked up has no assigned index (-1)"); return; } if (index == 128) { CenterStatusPrint("%s",Sys_Text.stringTable[309]); return; }/*Trioptimum Funpack*/ World.invP1.hasLog[index]  = true; World.invP1.lastAddedIndex = index; World.invP1.numLogsFromLevel[Sys_Text.audioLogLevelFound[index]]++;
    if(Sys_Text.audioLogType[index] == AudioLogType_Email)World.invP1.hasNewEmail=true;else if(Sys_Text.audioLogType[index]==AudioLogType_Normal)World.invP1.hasNewLogs=true;
    if (World.invP1.hasHardware & HW_ERD) { char keyStr[8]; sFormat(keyStr,sizeof(keyStr),"%s", Sys_Settings.InputCodeSettings[20] ? "U" : "?"); CenterStatusPrint("%s%s%s %s",Sys_Text.stringTable[36],World.audiologNames[index],Sys_Text.stringTable[38],keyStr); } else { CenterStatusPrint("%s%s%s",Sys_Text.stringTable[36],World.audiologNames[index],Sys_Text.stringTable[310]); }
}

static inline void ItemAdd(u8 *cur, u8 *counts, int idx, int uIdx, int sysIdx) { if (!counts[*cur]) {*cur=(i8)idx;} counts[idx]++; CenterStatusPrint("%s%s", Sys_Text.stringTable[ItemStringIdx(uIdx)], Sys_Text.stringTable[sysIdx]); }
void AddGrenadeToInventory(int i, int u) { World.invP1.grenConstIndex[i]=(i16)u; ItemAdd(&World.invP1.grenCur,World.invP1.grenAmmo,i,u,34); }
void   AddPatchToInventory(int i, int u) { if (i >= 0) ItemAdd(&World.invP1.patchCur,World.invP1.patchCounts,i,u,35); }
static inline void GrenadeCycle(int step){int cur= World.invP1.grenCur, next=cur; for(int i=0;i<7;++i){next=(next+step+7)%7; if(   World.invP1.grenAmmo[next]>0){World.invP1.grenCur =(i8)next; CenterStatusPrint("%s",Sys_Text.stringTable[579+next]); return;}}}
static inline void   PatchCycle(int step){int cur=World.invP1.patchCur, next=cur; for(int i=0;i<7;++i){next=(next+step+7)%7; if(World.invP1.patchCounts[next]>0){World.invP1.patchCur=(i8)next; CenterStatusPrint("%s",Sys_Text.stringTable[579+next]); return;}}}
void RemoveGrenade(int i) { if(World.invP1.grenAmmo[i] > 0){World.invP1.grenAmmo[i]--;} if(!World.invP1.grenAmmo[i]){GrenadeCycle(-1);} }
static i8 GetExistingCyberItemIndex() { if (World.invP1.softVersions[SW_TURBO]  > 0) {return 0;} if (World.invP1.softVersions[SW_DECOY]  > 0) {return 1;} if (World.invP1.softVersions[SW_RECALL] > 0) {return 2;} return -1; }
static void UseTurbo() {
    if(World.invP1.softVersions[SW_TURBO]<=0){World.invP1.hasSoft&=(u8)~(1u << SW_TURBO); return;} if(--World.invP1.softVersions[SW_TURBO]==0)World.invP1.hasSoft&=(u8)~(1u << SW_TURBO); if(World.invP1.turboFinished > World.pauseRelativeTime){World.invP1.turboFinished+=World.invP1.turboCyberTime;}else{World.invP1.turboFinished=World.invP1.turboCyberTime+World.pauseRelativeTime;}
}

static void UseDecoy() {
    if (World.decoyActive) { CenterStatusPrint("%s",Sys_Text.stringTable[537]); return; } if (World.invP1.softVersions[SW_DECOY] <= 0) { World.invP1.hasSoft &= (u8)~(1u << SW_DECOY); return; }
    if (--World.invP1.softVersions[SW_DECOY] == 0) World.invP1.hasSoft &= (u8)~(1u << SW_DECOY); u16 decoyIdx = SpawnDynamicObject(417,true);/*417 = CyberDecoy constIndex*/ if (decoyIdx != U16_MAX) {World.position[decoyIdx] = World.position[PLAYER1];} 
}

static void UseRecall() { if (World.invP1.softVersions[SW_RECALL] <= 0) {return;} if (--World.invP1.softVersions[SW_RECALL] == 0) {World.invP1.hasSoft &= (u8)~(1u << SW_RECALL);} World.position[PLAYER1] = World.cyberspaceRecallPoint; }
void UseCyberspaceItem() {
    if (World.invP1.cyberItemIndex <= 0) { World.invP1.cyberItemIndex = GetExistingCyberItemIndex(); if (World.invP1.cyberItemIndex < 0) { CenterStatusPrint("%s",Sys_Text.stringTable[473]); return; } }
    switch(World.invP1.cyberItemIndex) {
        case 0: if (!World.invP1.softVersions[SW_TURBO])  { World.invP1.cyberItemIndex = GetExistingCyberItemIndex(); return; } UseTurbo();  break; case 1: if (!World.invP1.softVersions[SW_DECOY])  { World.invP1.cyberItemIndex = GetExistingCyberItemIndex(); return; } UseDecoy();  break; 
        case 2: if (!World.invP1.softVersions[SW_RECALL]) { World.invP1.cyberItemIndex = GetExistingCyberItemIndex(); return; } UseRecall(); break;
    }
}

void CycleCyberSpaceItemUp() { int next = World.invP1.cyberItemIndex + 1; if (next > 2){next=0;} for (int c = 0; c <= 7; c++) { if (World.invP1.hasSoft & (1u << (SW_TURBO+next))) { World.invP1.cyberItemIndex = (i8)next; return; } if (c == 7) { World.invP1.cyberItemIndex = -1; return; } if (++next > 2) {next = 0;} } }
void CycleCyberSpaceItemDn() { int next = World.invP1.cyberItemIndex - 1; if (next < 0){next=2;} for (int c = 0; c <= 7; c++) { if (World.invP1.hasSoft & (1u << (SW_TURBO+next))) { World.invP1.cyberItemIndex = (i8)next; return; } if (c == 7) { World.invP1.cyberItemIndex = -1; return; } if (--next < 0) {next = 2;} } }
void RemoveWeapon(i32 slot) { World.invP1.weaponInventoryIndices[slot] = World.invP1.weaponInventoryAmmoIndices[slot] = -1; if (slot == World.invP1.weaponCurrent) { bool anyLeft = false; for (int i=0;i<7;i++) if (World.invP1.weaponInventoryIndices[i] >= 0) { anyLeft = true; break; } if (!anyLeft) World.instances[World.weaponVModelIndex].modelIndex = MAX_MDLS; } }
static float DefaultEnergySettingForWeapon(int wep16Index) { return (wep16Index == 4) ? 5.0f : (wep16Index == 10) ? 13.0f : (wep16Index == 14) ? 2.0f : 3.0f; }
__attribute__((noinline)) void AddAmmoToInventory(int index,int constIndex,int amount,bool isSecondary) { if(index < 0){return;} if(isSecondary){World.invP1.wepAmmoSecondary[index]+=(u16)amount;} else {World.invP1.wepAmmo[index]+=(u16)amount;} CenterStatusPrint("%s%s",Sys_Text.stringTable[ItemStringIdx(constIndex)],Sys_Text.stringTable[630]); }
bool AddWeaponToInventory(int index,int ammo1,int ammo2,bool loadedAlt) {
    if (index < 0) return false;
    for (i32 i = 0; i < 7; i++) {
        if(World.invP1.weaponInventoryIndices[i] >= 0){continue;} World.invP1.weaponInventoryIndices[i] = index; i32 index16 = Get16WeaponIndexFromConstIndex(index); World.invP1.weaponEnergySetting[i] = DefaultEnergySettingForWeapon(index16);
        if (i == 0) { World.invP1.weaponCurrentPending=i; World.invP1.weaponIndexPending=(u16)index; World.invP1.justChangedWeap=true; WeaponFireStartWeaponDip(0.5f); WeaponFireCompleteWeaponChange(); }
        if (loadedAlt && ammo2 > 0){World.invP1.currentMagazineAmount2[i]=(u8)ammo2; if (ammo1 > 0) World.invP1.wepAmmo[index16]+=(u16)ammo1; World.invP1.wepLoadedWithAlternate[i]=true;}else{World.invP1.currentMagazineAmount[i]=(u8)ammo1; if (ammo2 > 0) World.invP1.wepAmmoSecondary[index16]+=(u16)ammo2; World.invP1.wepLoadedWithAlternate[i]=false;}
        CenterStatusPrint("%s%s",Sys_Text.stringTable[ItemStringIdx(index)],Sys_Text.stringTable[33]); World.invP1.numweapons=0; for (i32 j=0;j<7;j++) { if(World.invP1.weaponInventoryIndices[j] >= 0){World.invP1.numweapons++;} } return true;
    } return false;
}

void UseGrenade(int index) {
    if (World.invP1.holdingObject) { CenterStatusPrint("%s",Sys_Text.stringTable[311]); return; }/*Can't use grenade, hands full*/ ForceInventoryMode(); ResetHeldItem(); World.invP1.grenActive=true; CenterStatusPrint("%s%s",Sys_Text.stringTable[ItemStringIdx(index)],Sys_Text.stringTable[320]); /*activated, grenade is LIVE!*/
    switch(index) {
        case 314:World.invP1.heldObjectIndex=370; RemoveGrenade(0); break; /*Frag*/         case 315:World.invP1.heldObjectIndex=372; RemoveGrenade(3); break; /*Concussion*/ case 316:World.invP1.heldObjectIndex=387; RemoveGrenade(1); break; /*EMP*/ case 317:World.invP1.heldObjectIndex=389; RemoveGrenade(6); break; /*Earth Shaker*/
        case 318:World.invP1.heldObjectIndex=402; RemoveGrenade(4); break; /*Land Mine*/  case 319:World.invP1.heldObjectIndex=403; RemoveGrenade(5); break; /*Nitropak*/ case 320:World.invP1.heldObjectIndex=404; RemoveGrenade(2); break; /*Gas*/
        default: return;
    } World.invP1.heldObjectCustIdx = U16_MAX; World.invP1.heldAmmo = 0; World.invP1.heldAmmo2 = 0; World.invP1.heldObjectLoadedAlternate = false; World.invP1.holdingObject = true;
}

void InventoryUpdate() {
    if (Grenade()) { if (World.curLev == LEVEL_CYBERSPACE){UseCyberspaceItem();} else if (World.invP1.grenCur >= 0 && World.invP1.grenCur < 7 && World.invP1.grenAmmo[World.invP1.grenCur] > 0){UseGrenade(World.invP1.grenConstIndex[World.invP1.grenCur]);} else {CenterStatusPrint("%s",Sys_Text.stringTable[322]);/*Out of grenades.*/} }
    if (GrenadeCycUp())  { if (World.curLev == LEVEL_CYBERSPACE) CycleCyberSpaceItemUp(); else GrenadeCycle( 1); } if (GrenadeCycDown()){ if (World.curLev == LEVEL_CYBERSPACE) CycleCyberSpaceItemDn(); else GrenadeCycle(-1); }
    if (RecentLog() && (World.invP1.hasHardware & HW_ERD)) {
        if (World.invP1.lastAddedIndex >= 0) { int temp = World.invP1.lastAddedIndex; PlayLog(temp); World.invP1.lastAddedIndex = FindNextUnreadLog(); if (World.invP1.lastAddedIndex == temp) World.invP1.lastAddedIndex = -1; CheckForUnreadLogs(); }
        else { int temp = World.invP1.lastAddedIndex; World.invP1.lastAddedIndex = FindNextUnreadLog(); if (World.invP1.lastAddedIndex == temp) {World.invP1.lastAddedIndex = -1;} CheckForUnreadLogs(); CenterStatusPrint("%s",Sys_Text.stringTable[1019]); /*Log playback stopped.*/ }
    }
    if (Patch()) { if (World.invP1.patchCur >= 0 && World.invP1.patchCur < 7 && World.invP1.patchCounts[World.invP1.patchCur] > 0){PatchUse(World.invP1.patchCur);} else {CenterStatusPrint("%s",Sys_Text.stringTable[324]); /*Out of patches.*/} } if (PatchCycUp()){PatchCycle( 1);} else if (PatchCycDown()){PatchCycle(-1);}
}

void AddItemFail(int index/*Expects usableItem index*/) { DropHeldItem(); CenterStatusPrint("%s%s%s", Sys_Text.stringTable[32],Sys_Text.stringTable[ItemStringIdx(index)],Sys_Text.stringTable[318]);/*Inventory full.*/ }
extern u8 magazinePitchCountForWeapon[16],magazinePitchCountForWeapon2[16];
void AddItemToInventory(int index, int custIdx) {
    if (IdxIsGenericItem(index)) { if(!AddGeneralObjectToInventory(index,custIdx)){AddItemFail(index);} } else if (IdxIsAudioLog(index)) { AddAudioLogToInventory(World.invP1.heldObjectCustIdx); }
    else if (IdxIsWeapon(index)) { int constIndex = index + 307; if (constIndex < 343 || constIndex > 358) constIndex = index; if (!AddWeaponToInventory(constIndex,World.invP1.heldAmmo,World.invP1.heldAmmo2,World.invP1.heldObjectLoadedAlternate)) { AddItemFail(index); } } else if (IdxIsAccessCard(index)) AddAccessCardToInventory(index);
    else {
        switch (index) {
            case 314: AddGrenadeToInventory(0,index); break; /*Frag*/ case 315: AddGrenadeToInventory(3,index); break; /*Concussion*/ case 316: AddGrenadeToInventory(1,index); break; /*EMP*/ case 317: AddGrenadeToInventory(6,index); break; /*Earth Shaker*/ case 318: AddGrenadeToInventory(4,index); break; /*Land Mine*/ case 319: AddGrenadeToInventory(5,index); break;/*Nitropak*/
            case 320: AddGrenadeToInventory(2,index); break; /*Gas*/  case 14: AddPatchToInventory(2,index); break; case 15: AddPatchToInventory(6,index); break; case 16: AddPatchToInventory(5,index); break; case 17: AddPatchToInventory(3,index); break;    case 18: AddPatchToInventory(4,index); break; case 19: AddPatchToInventory(1,index); break;
            case 20: AddPatchToInventory(0,index); break; case 21: AddHardwareToInventory(0,custIdx); break; case 22: AddHardwareToInventory(1,custIdx); break; case 23: AddHardwareToInventory(2,custIdx); break; case 24: AddHardwareToInventory(3,custIdx); break; case 25: AddHardwareToInventory(4,custIdx); break; case 26: AddHardwareToInventory(5,custIdx); break;
            case 27: AddHardwareToInventory(6,custIdx); break; case 28: AddHardwareToInventory(7,custIdx); break; case 29: AddHardwareToInventory(8,custIdx); break; case 30: AddHardwareToInventory(9,custIdx); break; case 31: AddHardwareToInventory(10,custIdx);break; case 32: AddHardwareToInventory(11,custIdx); break;
            case 60: AddAmmoToInventory(12,index,magazinePitchCountForWeapon[12],false); break; /*rubber slugs*/      case 65: AddAmmoToInventory(8,index,magazinePitchCountForWeapon2[8],true); break; /*magpulse cartridge super*/ case 66: AddAmmoToInventory(2,index,magazinePitchCountForWeapon[2],false); break; /*needle darts*/ 
            case 67: AddAmmoToInventory(2,index,magazinePitchCountForWeapon2[2],true); break; /*tranquilizer darts*/  case 68: AddAmmoToInventory(9,index,magazinePitchCountForWeapon[9],false); break; /*standard bullets*/         case 69: AddAmmoToInventory(9,index,magazinePitchCountForWeapon2[9],true); break; /*teflon bullets*/
            case 70: AddAmmoToInventory(7,index,magazinePitchCountForWeapon[7],false); break; /*hollow point rounds*/ case 71: AddAmmoToInventory(7,index,magazinePitchCountForWeapon2[7],true); break; /*slug rounds*/              case 72: AddAmmoToInventory(0,index,magazinePitchCountForWeapon[0],false); break; /*magnesium tipped slugs*/
            case 73: AddAmmoToInventory(0,index,magazinePitchCountForWeapon2[0],true); break; /*penetrator slugs*/    case 74: AddAmmoToInventory(3,index,magazinePitchCountForWeapon[3],false); break; /*hornet clip*/              case 75: AddAmmoToInventory(3,index,magazinePitchCountForWeapon2[3],true); break; /*splinter clip*/
            case 76: AddAmmoToInventory(11,index,magazinePitchCountForWeapon[11],false); break; /*rail rounds*/       case 77: AddAmmoToInventory(13,index,magazinePitchCountForWeapon[13],false); break; /*slag magazine*/          case 78: AddAmmoToInventory(13,index,magazinePitchCountForWeapon2[13],true); break; /*large slag magazine*/ 
            case 79: AddAmmoToInventory(8,index,magazinePitchCountForWeapon[8],false); break; /*magpulse cartridges*/ case 80: AddAmmoToInventory(8,index,magazinePitchCountForWeapon2[8],false); break; /*small magpulse cartridges*/ default: return;
        }
    } play_wav(sounds[87],1.0f,(V3){0},false);
}

void CyberDoorOnCollisionEnter(u16 self, u16 other) { if(other != PLAYER1){return;} CenterStatusPrint("%s  %s",Sys_Text.stringTable[World.instances[self].messageIndex],Sys_Text.stringTable[601]); }
void CyberTimerInitAfterLoad(u16 self) { Entity* e = &World.instances[self]; e->cyberTimer = 600.0f; e->timerFinished = World.pauseRelativeTime + 1.0; }
void CyberTimerReset(u16 self, int diff) { Entity* e = &World.instances[self]; switch (diff) { case 0: e->cyberTimer = 600.0f; break; case 1: e->cyberTimer = 300.0f; break; case 2: e->cyberTimer = 240.0f; break; case 3: e->cyberTimer = 180.0f; break; } }
void CyberTimerUpdate(u16 self) { if(World.curLev != LEVEL_CYBERSPACE){return;} Entity* e=&World.instances[self]; if(e->cyberTimer <= 0.0f){UIExitCyberspace(); return;} if(e->timerFinished >= World.pauseRelativeTime){return;} e->cyberTimer-=1.0f; e->minutes=vfloor(e->cyberTimer / 60.0f); e->seconds=e->cyberTimer - (e->minutes * 60.0f); e->timerFinished=World.pauseRelativeTime + 1.0; }
void CyberWallInitAfterLoad(u16 self) { Entity* e=&World.instances[self]; e->tickFinished=World.pauseRelativeTime + 2.0; e->animSwapFinished=0.0; } // alpha pushed via glUniform1f(27, ...) in voxen.c
void CyberWallUpdate(u16 self) { Entity* e = &World.instances[self]; if (World.pauseRelativeTime < e->tickFinished) {return;} e->tickFinished = World.pauseRelativeTime + 0.05; }
void SearchFXResetEnable(u16 self) { Entity* e = &World.instances[self]; if (e->itemLifeTime <= 0.0f) {e->itemLifeTime = 3.0f;} e->delayFinished = World.pauseRelativeTime + e->itemLifeTime; }
void SearchFXResetUpdate(u16 self) { Entity* e = &World.instances[self]; if (e->delayFinished >= World.pauseRelativeTime) {return;} flag_set(&e->entflags,EF_ACTIVE,false); }
void DelayedSpawnEnable(u16 self) { Entity* e = &World.instances[self]; e->timerFinished = World.pauseRelativeTime + e->delay; e->active = true; }
void DelayedSpawnUpdate(u16 s) { Entity* e=&World.instances[s]; if(!e->active||e->timerFinished<=0.0||e->timerFinished>World.pauseRelativeTime){return;} e->active=false; if(!e->doSelfAfterList){return;} if(e->despawnInstead){if(e->destroyAfterListInsteadOfDeactivate){DeleteInstance(s);}else{flag_set(&e->entflags,EF_ACTIVE,false);}}else flag_set(&e->entflags,EF_ACTIVE,true);}
void FuncWallShiftChildren(u16 self, V3 delta) { if (vabs(delta.x)+vabs(delta.y)+vabs(delta.z) < 0.00001f) {return;} for (u16 i=PLAYER1;i<World.instCount;++i) { if (fwParentOf[i]==self) { World.position[i]=V3_AplusB(World.position[i],delta); } } }
void FuncWallInitAfterLoad(u16 self) {
    Entity* e=&World.instances[self]; V3 prev=World.position[self]; float distTotal=V3_Dist(e->startPosition,e->targetPosition); float f=0; if((u8)e->funcState>FStat_AjarMovingTarget)f=e->ajarPercentage; else if(e->funcState==FStat_AjarMovingTarget) f=e->ajarPercentage;
    else if(e->funcState ==FStat_AjarMovingStart){f=1.0f-e->ajarPercentage;} if (f < 0.0f) f = 0.0f; if (f > 1.0f) f = 1.0f; V3 np=(distTotal > 0.0001f) ? V3_AplusB(e->startPosition,V3_ScaleByF(V3_Normalize(V3_AsubB(e->targetPosition,e->startPosition)),distTotal*f)) : e->startPosition; World.position[self]=np;
    if ((u8)e->funcState <= FStat_MovingTarget) { e->funcState = FStat_Start; e->percentMoved = 0.0f; } FuncWallShiftChildren(self,V3_AsubB(np,prev));
}

void FuncWallMoveStart(u16 self) { World.instances[self].funcState = FStat_MovingStart; World.instances[self].tickFinished = World.pauseRelativeTime + 10.0f; }
void FuncWallMoveTarget(u16 self) { World.instances[self].funcState = FStat_MovingTarget; World.instances[self].tickFinished = World.pauseRelativeTime + 10.0f; }
void FuncWallTargetted(u16 self) { Entity* e = &World.instances[self]; u8 st = (u8)e->funcState; bool toTarget = st == FStat_Start || st == FStat_MovingStart || st == FStat_AjarMovingTarget || (st > FStat_AjarMovingTarget && e->ajarPercentage > 0.0f); if (toTarget){FuncWallMoveTarget(self);} else{FuncWallMoveStart(self);} play_wav(sounds[76],1.0f,World.position[self],true); }
void FuncWallUpdateInner(u16 self) {
    Entity* e = &World.instances[self]; if (e->funcState != FStat_MovingStart && e->funcState != FStat_MovingTarget) return; V3 goal = e->funcState == FStat_MovingStart ? e->startPosition : e->targetPosition; FuncStates doneState = e->funcState == FStat_MovingStart ? FStat_Start : FStat_Target; V3 delta = V3_AsubB(goal,World.position[self]);
    float distanceLeft = V3_Mag(delta), total = V3_Dist(e->startPosition,e->targetPosition), dist = e->speed * (float)World.deltaTime; if (distanceLeft <= dist || e->tickFinished < World.pauseRelativeTime) { World.position[self]=goal; e->funcState=doneState; e->percentMoved=doneState == FStat_Target ? 1.0f : 0.0f; return; }
    if (distanceLeft > 0.0001f) World.position[self]=V3_AplusB(World.position[self],V3_ScaleByF(V3_Normalize(delta),dist)); if (total > 0.0001f) e->percentMoved = V3_Dist(e->startPosition,World.position[self]) / total;
}
void FuncWallUpdate(u16 self) { V3 prev = World.position[self]; FuncWallUpdateInner(self); FuncWallShiftChildren(self,V3_AsubB(World.position[self],prev)); }
void func_forcebridge(u16 self) {
    Entity* e = &World.instances[self]; e->tickFinished = World.pauseRelativeTime + 0.05f + (double)random_range(0.0f,1.0f); e->lerping = true; if(e->activatedScale.x <= 0.02f){e->activatedScale.x = 2.56f;} if(e->activatedScale.y <= 0.02f){e->activatedScale.y = 0.08f;} if(e->activatedScale.z <= 0.02f){e->activatedScale.z = 2.56f;}
    if(!e->active){ e->modelIndex=MAX_MDLS; World.col[self]=COLTYPE_NONE;} switch (e->fieldColor) { case ForceFieldColor_Red:e->texIndex=38; break; case ForceFieldColor_Green:e->texIndex=40; break; case ForceFieldColor_Blue:e->texIndex=39; break; case ForceFieldColor_Purple:e->texIndex=41; break; case ForceFieldColor_RedFaint:e->texIndex=198; break; }
}

void ForceBridgeActivate(u16 self, bool isSilent) {
    Entity* e = &World.instances[self]; if (e->active) {return;}
    if(!isSilent){play_wav(sounds[102],1.0f,World.position[self],true);} flag_set(&e->entflags,EF_ACTIVE,true); e->modelIndex=78; World.col[self]=COLTYPE_BOX; e->active=e->lerping=true; World.scale[self]=(V3){ e->forceFieldDirectionX ? 0.1f : e->activatedScale.x,e->forceFieldDirectionY ? 0.1f : e->activatedScale.y,e->forceFieldDirectionZ ? 0.1f : e->activatedScale.z };
}

void ForceBridgeDeactivate(u16 self, bool isSilent) { Entity* e = &World.instances[self]; if (!e->active) {return;} if (!isSilent) {play_wav(sounds[102],1.0f,World.position[self],true);} e->active = false; e->lerping = true; }
void ForceBridgeToggle(u16 self) { if (World.instances[self].active) {ForceBridgeDeactivate(self,false); } else {ForceBridgeActivate(self,false);} }
void ForceBridgeUpdate(u16 self) {
    Entity* e = &World.instances[self]; if(e->tickFinished >= World.pauseRelativeTime){return;} e->tickFinished = World.pauseRelativeTime + 0.05f;
    if (e->active) {
        if (!e->lerping) return;
        float sx=e->forceFieldDirectionX ? lerp(World.scale[self].x,e->activatedScale.x,0.1f) : World.scale[self].x, sy=e->forceFieldDirectionY ? lerp(World.scale[self].y,e->activatedScale.y,0.1f) : World.scale[self].y, sz=e->forceFieldDirectionZ ? lerp(World.scale[self].z,e->activatedScale.z,0.1f) : World.scale[self].z;
        World.scale[self]=(V3){sx,sy,sz}; if(vabs(e->activatedScale.x - sx) < 0.08f && vabs(e->activatedScale.y - sy) < 0.08f && vabs(e->activatedScale.z - sz) < 0.08f){World.scale[self]=e->activatedScale; e->lerping=false;}
    } else if (e->lerping) { 
        float sx=e->forceFieldDirectionX ? lerp(World.scale[self].x,0.0f,0.1f) : World.scale[self].x, sy=e->forceFieldDirectionY ? lerp(World.scale[self].y,0.0f,0.1f) : World.scale[self].y, sz=e->forceFieldDirectionZ ? lerp(World.scale[self].z,0.0f,0.1f) : World.scale[self].z;
        World.scale[self]=(V3){sx,sy,sz}; if (sx < 0.08f || sy < 0.08f || sz < 0.08f) { e->modelIndex = MAX_MDLS; World.col[self] = COLTYPE_NONE; e->lerping = false; }
    }
}
// TriggerCounter
void TriggerCounterTarget(u16 self, u16 activator) { UseTargets(activator,World.instances[self].targetIdx); }
void TriggerCounterDelayedTarget(u16 self, u16 act) { World.instances[self].delayFinished = World.pauseRelativeTime + World.instances[self].delay; TriggerCounterTarget(self,act); }
void TriggerCounterTargetted(u16 self, u16 act) { Entity* e=&World.instances[self]; e->counter++; if (e->counter != e->countToTrigger) {return;} if (e->delay <= 0.0f){TriggerCounterTarget(self,act);}else{TriggerCounterDelayedTarget(self,act);} if (!e->dontReset){e->counter=0;} }
// TextureChanger
void TextureChangerToggle(u16 self) {
    u16 alt = 0, glowAlt = 0;
    if (World.instances[self].index == 538) { alt = 1118; glowAlt = 1116; } else if (World.instances[self].index == 689) { alt = 841; glowAlt = 840; } else if (World.instances[self].index == 690) { alt = 844; glowAlt = 843; } else if (World.instances[self].index == 695) { alt = 858; glowAlt = 857; } else return;
    if (World.instances[self].currentTexture) { World.instances[self].texIndex = EDefs[World.instances[self].index].texIndex; World.instances[self].glowIndex = EDefs[World.instances[self].index].glowIndex; } else { World.instances[self].texIndex = alt; World.instances[self].glowIndex = glowAlt; } World.instances[self].currentTexture = !World.instances[self].currentTexture;
}
// LogicTimer
void LogicTimerInitBeforeLoad(u16 self) { Entity* e=&World.instances[self]; if(e->timeInterval <= 0.0f){e->timeInterval=0.35f;} if(e->randomMin <= 0.0f){e->randomMin=5.0f;} if(e->randomMax <= 0.0f){e->randomMax=10.0f;} e->intervalFinished=World.pauseRelativeTime + (e->useRandomTimes ? (double)random_range(e->randomMin,e->randomMax) : (double)e->timeInterval); }
void LogicTimerUseTargets(u16 self) { UseTargets(self,World.instances[self].targetIdx); }
void LogicTimerUpdate(u16 self) { Entity* e=&World.instances[self]; if(!e->active || e->intervalFinished >= World.pauseRelativeTime){return;} e->intervalFinished=World.pauseRelativeTime + (e->useRandomTimes ? (double)random_range(e->randomMin,e->randomMax) : (double)e->timeInterval); LogicTimerUseTargets(self); }
void LogicTimerTargetted(u16 self, u16 activator) { (void)activator; World.instances[self].active = !World.instances[self].active; }
// ButtonSwitch
void ButtonSwitchInitAfterLoad(u16 self) { Entity* e=&World.instances[self]; e->delayFinished=0.0f; if(e->active){e->tickFinished=World.pauseRelativeTime + 1.5 + (double)random_range(0.0f,1.0f);} }
void ButtonSwitchUseTargets(u16 self) { Entity* e=&World.instances[self]; UseTargets(self,e->targetIdx); e->active=!e->active; if(e->index == 689 || e->index == 690 || e->index == 695) { TextureChangerToggle(self); if(e->index == 689 && e->active){e->tickFinished=World.pauseRelativeTime + 1.5f;} } }
static __attribute__((noinline)) void UIBlockedBySecurity(V3 tetherPoint) { (void)tetherPoint; CenterStatusPrint("%s",Sys_Text.stringTable[25]); }
static __attribute__((noinline)) void EntitySetLocked(Entity* e, bool locked) { flag_set(&e->entflags,EF_LOCKED,locked); }
void ButtonSwitchUse(u16 self, u16 activator) {
    Entity* e = &World.instances[self]; if(Cheats.superoverride || World.diffMis == 0){EntitySetLocked(e,false);} else if(GetCurrentLevelSecurity() > e->securityThreshold){UIBlockedBySecurity(World.position[self]); return;}
    if ((e->entflags & EF_LOCKED) != 0) { CenterStatusPrint("%s",Sys_Text.stringTable[e->lockedMessageLingdex]); if (e->SFXLockedIndex >= 0 && e->SFXLockedIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXLockedIndex],1.0f,World.position[self],true); return; }
    if (e->SFXIndex >= 0 && e->SFXIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXIndex],1.0f,World.position[self],true);
    CenterStatusPrint("%s",Sys_Text.stringTable[e->messageIndex]); if (e->delay > 0.0f) { e->recentMostActivator = activator; e->delayFinished = World.pauseRelativeTime + e->delay; } else ButtonSwitchUseTargets(self);
}

void ButtonSwitchUpdate(u16 self) { double t=World.pauseRelativeTime; Entity* e=&World.instances[self]; if (e->delayFinished > 0.0 && e->delayFinished < t){e->delayFinished=0.0; ButtonSwitchUseTargets(self);} if (e->index == 689 && e->active && e->tickFinished < t) { TextureChangerToggle(self); e->tickFinished=t+1.5f; } }
void HealingBedUse(u16 self, u16 owner) { Entity* e=&World.instances[self]; if (GetCurrentLevelSecurity() <= (u8)e->minSecurityLevel) { if(!e->broken){HealthManagerHealingBed(PLAYER1,e->amount,true); CenterStatusPrint("%s",Sys_Text.stringTable[23],owner); play_wav(sounds[103],1.0f,World.position[self],false);} else {CenterStatusPrint("%s",Sys_Text.stringTable[24],owner);} } else UIBlockedBySecurity(World.position[self]); }
// VaporizeButton
void VaporizeClick(void) {
    if (World.invP1.generalInvCurrent == 0) return;/*Access Cards index.*/ int cur = World.invP1.generalInvCurrent; World.invP1.generalInventoryIndexRef[cur] = -1;/*Remove item*/ World.invP1.generalInvCurrent -= 1; if (World.invP1.generalInvCurrent < 0) { World.invP1.generalInvCurrent = 0; }/*skip since 0 is Access Cards.*/
    cur = World.invP1.generalInvCurrent; if (World.invP1.generalInventoryIndexRef[cur] < 0) { for (int i=13; i >= 0; i--) { if (World.invP1.generalInventoryIndexRef[i] >= 0) { World.invP1.generalInvCurrent = (i8)i; break; } } } play_wav(sounds[89], SfxVol(), (V3){0.0f,0.0f,0.0f}, false); // vaporize sfx
}

typedef struct { i8 norm,alt; } AmmoIconEntry;
static const AmmoIconEntry ammoIconTable[51]={[36-36]={7,8}/*MK3 Magnesium/Penetrator*/,[37-36]={-2,-2}/*Energy*/,[38-36]={0,1}/*Dartgun Needle/Tranq*/,[39-36]={9,10}/*Flechette Hornette/Splinter*/,[40-36]={-2,-2}/*Energy*/,[41-36]={-1,-1}/*Rapier, no ammo*/,[42-36]={-1,-1}/*Pipe, no ammo*/,[43-36]={5,6}/*Magnum Hollow/Slug*/,[44-36]={11,-1}/*Magpulse Magcart*/,
                                              [45-36]={2,3 }/*Pistol Standard/Teflon*/,[46-36]={-2,-2}/*Energy*/,[47-36]={14,-1}/*Railgun Rail Rounds*/,[48-36]={4,-1}/*Riotgun Rubber Slugs*/,[49-36]={12,13}/*Skorpion Slag/Large Slag*/,[50-36]={-2,-2}/*Energy*/,[51-36]={-2,-2}/*Energy*/};
i8 AmmoIconGet(int index,bool alt) { if (index < 343 || index > 358) {return -1;} const AmmoIconEntry* e = &ammoIconTable[index - 343]; return alt ? e->alt : e->norm; }
static double creditsVidStartTime,creditsVidFinished; static u8 creditsVidPhase; // CreditsScroll, TODO video text phases: 0=text1 visible, 1=text2 visible, 2=text3 visible, 3=all hidden
void CreditsOnEnable(void) { World.creditsActive=true; World.creditsPageIndex=0; creditsVidStartTime=World.absoluteTime; creditsVidFinished=World.absoluteTime + 37.2; creditsVidPhase=0; }
void CreditsUpdate(void) {
    if (!World.creditsActive) return;
    double elapsed = World.absoluteTime - creditsVidStartTime;
    if (creditsVidFinished > 0.0) { // Drive video text phase transitions
        if (elapsed >  7.0 && creditsVidPhase == 0) { creditsVidPhase = 1; CenterStatusPrint("Credits phase: text2 visible"); } if (elapsed > 11.0 && creditsVidPhase == 1) { creditsVidPhase = 2; CenterStatusPrint("Credits phase: text3 visible"); } if (elapsed > 14.0 && creditsVidPhase == 2) { creditsVidPhase = 3; CenterStatusPrint("Credits phase: text hidden"); }
        if (World.absoluteTime >= creditsVidFinished) { creditsVidFinished=0.0; creditsVidPhase=3; CenterStatusPrint("Credits video finished"); }
    }
    if (Menu()) { if (creditsVidFinished > 0.0) { creditsVidFinished = 0.0; return; /*skip video*/} MenuGoBack(); return; } if (creditsVidFinished > 0.0) return; // absorb all click input while video playing
    if (Attack()) { // left click — advance
        if (!(World.creditsPageIndex >= CREDITS_PAGES)) { ++World.creditsPageIndex; if (!World.gameFinished && World.creditsPageIndex == 1) ++World.creditsPageIndex;/*skip stats page when not finishing game*/ if (World.creditsPageIndex >= CREDITS_PAGES) World.creditsPageIndex = CREDITS_PAGES;/*bottom*/ } else { World.creditsActive = false; MenuGoBack(); } return;
    } if (ToggleMode()) { if (World.creditsPageIndex > 0){--World.creditsPageIndex;} } // right click — go back a page
}
// CyborgConversionToggle
void CyborgConversionToggleTargetted(void) {
    bool active = (World.ressurectionActiveLevels >> World.curLev) & 1u; flag_setu16(&World.ressurectionActiveLevels,(1u << World.curLev),!active); if (World.curLev == 6) flag_setu16(&World.ressurectionActiveLevels, (1u<<10|1u<<11|1u<<12),!active); // Set groves 10,11,12 when 6 gets toggled as they don't have their own switch
    play_wav(sounds[active ? 183 : 184],Sys_Settings.VolumeMessage,(V3){0.0f,0.0f,0.0f},false);/*"vox_cybconvcancelled" : "vox_cybconvenabled"*/ CenterStatusPrint("%s",Sys_Text.stringTable[active ? 591 : 592]);
}
// ElevatorButton
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
float TargetIDGetSensingRange(bool manual) { u8 ver = World.invP1.hwVers[HW_TID_IDX]; if (manual) {return (ver >= 4) ? 18.0f : 13.0f;} return (ver <= 2) ? 0.0f : ((ver == 3) ? 13.0f : 20.0f); }
float TargetIDGetTetherRange() { return (World.invP1.hwVers[HW_TID_IDX] >= 4) ? 22.0f : 15.0f; }
static void TargetIDDeactivate(u16 self) { Entity* e=&World.instances[self]; if(e->enemy != WORLD){Entity* npc=&World.instances[e->enemy]; flag_set(&npc->entflags,EF_TARGID_ATTACHED,false); e->enemy=WORLD;} e->textIndex=-1; flag_set(&e->entflags,EF_ACTIVE,false); }
void TargetIDSendDamageReceive(u16 self,float damage,AttType attackType) {
    Entity* e=&World.instances[self]; if(e->enemy == WORLD){return;} Entity* npc=&World.instances[e->enemy];
    if (attackType == Att_Trnq) { e->textIndex=536;/*STUNNED*/ e->animSwapFinished=World.pauseRelativeTime - 1.0;/*expire damage text*/ }
    else {
        float mh = npcTable[npc->index - 419].health; if(damage > mh * 0.75f)e->textIndex = 514;/*SEVERE DAMAGE*/ else if(damage > mh * 0.50f)e->textIndex = 515;/*MAJOR DAMAGE*/ else if (damage > mh * 0.25f) e->textIndex = 513;/*NORMAL DAMAGE*/ else if (damage > 0.0f)e->textIndex = 512;/*MINOR DAMAGE*/ else e->textIndex = 511;/*NO DAMAGE*/
        e->animSwapFinished = World.pauseRelativeTime + ((damage == 0.0f) ? 1.0f : 2.5f);
    }
}

void TargetIDUpdate(u16 self) {
    if (!(World.instances[self].entflags & EF_ACTIVE)){return;} if (World.instances[self].enemy == WORLD) { TargetIDDeactivate(self); return; } Entity* npc = &World.instances[World.instances[self].enemy]; if (npc->health <= 0.0f) { TargetIDDeactivate(self); return; }
    if (V3_Dist(World.position[self],World.position[PLAYER1]) > 10.0f) { TargetIDDeactivate(self); return; } if (World.instances[self].tickFinished < World.pauseRelativeTime) { TargetIDDeactivate(self); return; } World.position[self]=World.position[World.instances[self].enemy]; // Track parent NPC position
    bool stunned = npc->tranquilizeFinished > World.pauseRelativeTime; flag_set(&World.instances[self].entflags,EF_ASLEEP,stunned);
    if (World.instances[self].textIndex >= 0) { if (stunned && World.instances[self].animSwapFinished < World.pauseRelativeTime) World.instances[self].textIndex = 536;/*STUNNED*/ else if (World.instances[self].animSwapFinished < World.pauseRelativeTime) { World.instances[self].textIndex = -1; if (!(World.invP1.hasHardware & HW_TID)) { TargetIDDeactivate(self); return; } } }
}
// PlayerEnergy
static const float  hwDrain[12][4] = {[3]={0.01535f,0.03413f,0.02559f,0.0f},[5]={0.04096f,0.10239f,0.17919f,0.05119f},[6]={0.001706f,0.0f,0.0f,0.0f},[7]={0.02559f,0.04266f,0.05119f,0.0f},[9]={0.0f,0.02f,0.015f,0.0f},[11]={0.08533f,0.0f,0.0f,0.0f},};
static const u16 hwDrainJPM[12][4] = {[3]={9,20,15,0},[5]={24,60,105,30},[6]={1,0,0,0},[7]={15,25,30,0},[9]={0,16,12,0},[11]={50,0,0,0},};
void CreateTargetIDInstance(float damage, u16 hitIdx, float tranq) { if (hitIdx == WORLD || hitIdx >= World.instCount) return; Entity* npc = &World.instances[hitIdx]; if (!(npc->entflags & EF_ACTIVE) || (npc->entflags & EF_TARGID_ATTACHED)) return; if (V3_Dist(World.position[hitIdx], World.position[PLAYER1]) > TargetIDGetTetherRange()) return; u16 tidIdx = SpawnDynamicObject(736, false); if (tidIdx == WORLD || tidIdx == U16_MAX) return; Entity* tid = &World.instances[tidIdx]; tid->enemy = hitIdx; tid->tickFinished = World.pauseRelativeTime + 4.0; tid->textIndex = (tranq >= 0.0f) ? 536 : -1; tid->animSwapFinished = World.pauseRelativeTime + (tranq >= 0.0f ? 2.5 : 0.0); World.position[tidIdx] = World.position[hitIdx]; flag_set(&tid->entflags, EF_ACTIVE, true); flag_set(&npc->entflags, EF_TARGID_ATTACHED, true); if (damage > 0.0f) TargetIDSendDamageReceive(tidIdx, damage, Att_None); }
void TargetIdentifierSenseTargets() { for (u16 i = INSTS_1ST_IDX; i < World.instCount; i++) { Entity* e = &World.instances[i]; if (!(e->entflags & EF_ACTIVE) || !IdxIsNPC(e->index) || (e->entflags & EF_DEAD) || (e->entflags & EF_TARGID_ATTACHED) || V3_Dist(World.position[i],World.position[PLAYER1]) > TargetIDGetSensingRange(false)){continue;} CreateTargetIDInstance(0.0f,i,-1.0f); } }
bool ModRequestsGrayscale() { return ((World.invP1.hasHardware & HW_INF) && (World.invP1.hardwareIsActive & HW_INF) > 0); }
static void DeactivateHardwareOnEnergyDepleted() { World.invP1.hardwareIsActive = 0; }
void TakeEnergy(float take) { if (World.invP1.energy <= 0.0f || Cheats.redbull) {return;} World.invP1.energy -= take; if (World.invP1.energy <= 0.0f) { World.invP1.energy = 0.0f; play_wav(sounds[84],Sys_Settings.VolumeEffects,(V3){0.0f,0.0f,0.0f},false);/*energy_gone*/ CenterStatusPrint("%s",Sys_Text.stringTable[314]); /*Power supply exhausted.*/ DeactivateHardwareOnEnergyDepleted(); } }
void GiveEnergy(float give,EnergyType type) { World.invP1.energy += give; if (World.invP1.energy > 255.0f) {World.invP1.energy = 255.0f;} if (type == EnergyType_Battery){play_wav(sounds[79],Sys_Settings.VolumeEffects,(V3){0.0f,0.0f,0.0f},false);/*batteryuse*/} else if (type == EnergyType_ChargeStation){play_wav(sounds[100],Sys_Settings.VolumeEffects,(V3){0.0f,0.0f,0.0f},false);/*chargingstation*/} }
void PlayerEnergyInit() { World.invP1.energy = 54.0f; World.invP1.energyDrainTickFinished = World.pauseRelativeTime + 0.1 + random_range(0.0f,1.0f); World.invP1.drainJPM = 0; }
void PlayerEnergyUpdate() {
    if (World.invP1.hasHardware & HW_TID) TargetIdentifierSenseTargets(); if (World.invP1.energyDrainTickFinished > World.pauseRelativeTime) return; World.invP1.energyDrainTickFinished = World.pauseRelativeTime + 0.1; bool anyDrain = false; u8 ver; World.invP1.drainJPM = 0;
    for (int hw=3;hw<=11;++hw) { u16 bit=(u16)(1u << hw); if (!(World.invP1.hardwareIsActive & bit) || hw == 4 || hw == 8 || hw == 10) continue;/*No energy usage*/ ver=World.invP1.hwVersSetting[hw]; float drain=hwDrain[hw][ver];  World.invP1.drainJPM += hwDrainJPM[hw][ver]; if (drain > 0.0f) { TakeEnergy(drain); anyDrain = true; } }
    if (anyDrain && World.invP1.energy <= 0.0f) { DeactivateHardwareOnEnergyDepleted(); World.invP1.drainJPM = 0; } // Depleted
}
// GeneralInventory
static void ApplyBattery(int btn) { if (World.invP1.energy >= 255.0f) { CenterStatusPrint("%s",Sys_Text.stringTable[303]); return; } GiveEnergy(83.0f,EnergyType_Battery); World.invP1.generalInventoryIndexRef[btn] = -1; }
static void ApplyIcadBattery(int btn) { if (World.invP1.energy >= 255.0f) { CenterStatusPrint("%s",Sys_Text.stringTable[303]); return; } GiveEnergy(255.0f,EnergyType_Battery); World.invP1.generalInventoryIndexRef[btn] = -1; }
static void ApplyHealthkit(int btn) { if (World.instances[PLAYER1].health >= 255.0f) { CenterStatusPrint("%s",Sys_Text.stringTable[303]); return; } World.instances[PLAYER1].health = 255.0f; World.invP1.generalInventoryIndexRef[btn] = -1; }
void GeneralInvClick(int buttonIdx,int customIdx) { World.Sys_UI.mouseClickHeldOverGUI = true; (void)customIdx; (void)buttonIdx;/*TODO actual actions int itemIdx = World.invP1.generalInventoryIndexRef[buttonIdx];*/ }
void GeneralInvApply(int buttonIdx,int customIdx) { if (buttonIdx == 0) { return; } int itemIdx = World.invP1.generalInventoryIndexRef[buttonIdx]; switch (itemIdx) { case 52:ApplyBattery(buttonIdx);break;  case 53:ApplyIcadBattery(buttonIdx);break;  case 55:ApplyHealthkit(buttonIdx);break;  default:(void)customIdx;break;} }
void GeneralInvDoubleClick(int buttonIdx,int customIdx) { World.Sys_UI.mouseClickHeldOverGUI = true; GeneralInvApply(buttonIdx,customIdx); }
void GeneralInventoryActivate() { int cur=World.invP1.generalInvCurrent; if(cur < 0 || cur >= 14){DualLog("BUG: generalInvCurrent out of range at %d",cur); return;} GeneralInvApply(cur,World.invP1.generalInvCustIdx[cur]); if(cur != 0)World.invP1.generalInventoryIndexRef[cur]=-1; }
static bool GrenadeIsNPCMine(u16 self) { return World.layer[self] != L_PlayerBullets; }
void ApplyImpactForce(u16 target, float vel, V3 normal, V3 pt) {
    if (target == WORLD || target >= World.instCount || vel <= 0.0f){return;} Entity* e = &World.instances[target]; if((e->entflags & EF_DEAD) || (!(e->entflags & EF_RIGIDBODY) && target != PLAYER1)){return;}
    V3 n = V3_Normalize(normal); if (V3_Mag(n) < 0.0001f) {n = (V3){0.0f,1.0f,0.0f};/*At least make it pop off the floor*/} AddForce(target,V3_ScaleByF(n,vel),true); V3 lever = V3_AsubB(pt, World.position[target]); World.angularVelocity[target].x += lever.y * vel * 0.05f; World.angularVelocity[target].z += -lever.x * vel * 0.05f; (void)pt; // torque applied relative to point lever arm
}

void ApplyImpactForceSphere(DamageData* dd, V3 center, float radius, float baseVel) { 
    if (radius <= 0.0f || baseVel <= 0.0f) return; float r2 = radius * radius;
    for (u16 i = INSTS_1ST_IDX; i < World.instCount; i++) {
        Entity* e = &World.instances[i]; if (!(e->entflags & EF_ACTIVE) || (e->entflags & EF_DEAD)) continue; if (!(e->entflags & EF_RIGIDBODY) && !IdxIsNPC(e->index) && i != PLAYER1) continue; float sqd = V3_SqDist(World.position[i], center); if (sqd > r2) continue; float dist = vsqrtf(sqd); float falloff = 1.0f - (dist / radius); if (falloff <= 0.0f) continue;
        V3 normal; if(dist > 0.0001f){normal=V3_ScaleByF(V3_AsubB(World.position[i],center), 1.0f / dist);}else{normal = (V3){0.0f,1.0f,0.0f}; ApplyImpactForce(i,baseVel * falloff,normal,World.position[i]);}
        if (dd && dd->damage > 0.0f && i != dd->owner) { DamageData splash=*dd; splash.damage = dd->damage * falloff; splash.hitIdx = i; splash.hitpoint=World.position[i]; splash.attacknormal=normal; TakeDamage(i,splash); }
    }
}

void SpawnExplosionEffect(V3 pos, int explosionType) { static const u16 prefabs[6] = {729,730,731,732,733,734}; int idx = (explosionType >= 0 && explosionType < 6) ? explosionType : 2; u16 fx = SpawnDynamicObject(prefabs[idx], false); if (fx == WORLD || fx == U16_MAX) return; World.position[fx] = pos; Entity* e = &World.instances[fx]; flag_set(&e->entflags, EF_ACTIVE, true); if (e->delay <= 0.0f) e->delay = 0.8f; e->delayFinished = World.pauseRelativeTime + e->delay; }
void GrenadeExplode(u16 self) {
    Entity* e = &World.instances[self];
    DamageData dd={.damage=e->damage,.penetration=e->strength,.offense=e->speed,.armorvalue=0.0f,.defense=0.0f,.impactVelocity=e->damage*1.5f,.attacknormal=(V3){0.0f,1.0f,0.0f},.hitpoint=World.position[self],.attackType=e->attackType,.owner=e->recentMostActivator,.hitIdx=WORLD,.isOtherNPC=false,.berserkActive=(World.invP1.patchActive & PATCH_BERSERK) != 0};
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
float GetDamageTakeAmount(DamageData* dd) { if (!dd) return 0.0f; float take = dd->damage; if (take <= 0.0f) return 0.0f; if (dd->berserkActive) take *= BERSERK_DAMAGE_MULTIPLIER; if (dd->defense > 0.0f && dd->offense < dd->defense) { float r = (dd->defense - dd->offense) / dd->defense; if (r > 0.85f) r = 0.85f; take *= (1.0f - r); } if (dd->armorvalue > 0.0f && dd->penetration < dd->armorvalue) { float a = (dd->armorvalue - dd->penetration) / dd->armorvalue; if (a > 0.85f) a = 0.85f; take *= (1.0f - a); } if (take < 0.0f) take = 0.0f; return take; }
void SpawnImpactEffect(u16 impactType, V3 pos) { if (impactType == 0 || impactType == U16_MAX) return; u16 fx = SpawnDynamicObject(impactType, false); if (fx == WORLD || fx == U16_MAX) return; World.position[fx] = pos; Entity* e = &World.instances[fx]; flag_set(&e->entflags, EF_ACTIVE, true); if (e->itemLifeTime <= 0.0f) e->itemLifeTime = 1.0f; e->delayFinished = World.pauseRelativeTime + e->itemLifeTime; }
void ExitCyberspace(void) { UIExitCyberspace(); if (World.curLev != LEVEL_CYBERSPACE) return; if (World.instances[PLAYER1].cyberHealth <= 0.0f) World.instances[PLAYER1].cyberHealth = 1.0f; LoadLevel(World.startLevel < World.numLevels ? World.startLevel : 0, (V3){0.0f,0.0f,0.0f}); }
void ReduceCurrentLevelSecurity(SecurityType stype) { // Typical level: 4 CPU nodes. 20 cameras, 100% = 4x + 20y.  Assuming that a good camera percentage is 2-3%, CPU % would be about 10-15 each
    u8 lev = World.curLev; if (lev >= 14 || stype == SecurityType_None) return; const float camScore=4.0f, nodeSmallScore=10.0f, nodeLargeScore=27.0f; float total = (World.levelCameraCount[lev]*camScore)+(World.levelSmallNodeCount[lev]*nodeSmallScore)+(World.levelLargeNodeCount[lev]*nodeLargeScore); if (total <= 0.0f) return; float drop = camScore;
    switch (stype) {
        case SecurityType_Camera: drop=(camScore/total)*100.0f; if (World.levelCameraDestroyedCount[lev]<255) World.levelCameraDestroyedCount[lev]++; break; case SecurityType_NodeSmall: drop=(nodeSmallScore/total)*100.0f; if (World.levelSmallNodeDestroyedCount[lev]<255) World.levelSmallNodeDestroyedCount[lev]++; break;
        case SecurityType_NodeLarge: drop=(nodeLargeScore/total)*100.0f; if (World.levelLargeNodeDestroyedCount[lev]<255) World.levelLargeNodeDestroyedCount[lev]++; break; default: return;
    }
    int cur=(int)World.levelSecurity[lev]-(int)drop; if (cur<0) cur=0; World.levelSecurity[lev]=(u8)cur; if (World.levelCameraDestroyedCount[lev]==World.levelCameraCount[lev] && World.levelSmallNodeDestroyedCount[lev]==World.levelSmallNodeCount[lev] && World.levelLargeNodeDestroyedCount[lev]==World.levelLargeNodeCount[lev]) World.levelSecurity[lev]=0;
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
 // None  Melee  MelEn  EnBm   Mag    Proj   Needle ProjEB ProjLn Gas    Tranq  Drill
static const float attackTypeMult[7][12]={[NPCType_Mutant]={1,1,1,1,0,1,2,1,1,2,1,1},[NPCType_Supermutant]={1,1,1,1,0,1,1,1,1,1.5,1,1},[NPCType_Robot]={1,1,1,1,4,1,0,1,1,0,1,1},[NPCType_Cyborg]={1,1,1,1,2,1,1,1,1,1,1,1},[NPCType_Supercyborg]={1,1,1,1,2,1,0,1,1,0,1,1},[NPCType_MutantCyborg]={1,1,1,1,0.5,1,2,1,1,2,1.5,1},[NPCType_Cyber]={1,1,1,1,1,1,1,1,1,1,1,0}}; // Attack type damage multiplier table [NPCType][AttType], 1.0f = no change, 0.0f = immune, other = multiplier
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
        if(spawned != U16_MAX){World.position[spawned]=World.position[self]; World.instances[spawned].custIdx[0]=World.instances[self].custIdx[i];}else{CenterStatusPrint("BUG: Failed to instantiate object being dropped on gib.");} World.instances[self].contents[i] = World.instances[self].custIdx[i]=-1;
    }
}

static void CreateDeathEffects(u16 self,u16 fxPoolType) { if (fxPoolType == 0) {return; /*PoolType_None*/} V3 pos = World.position[self]; if (World.col[self] != COLTYPE_NONE) { pos = V3_AplusB(pos,World.colliderCenter[self]); } SpawnImpactEffect(fxPoolType, pos); }
static void HideSelf(u16 self) { if (World.instances[self].index == 279) {return; /*tv screens keep mesh visible*/} World.instances[self].modelIndex = MAX_MDLS; World.gravity[self] = 0.0f; }
static void NPCDeath(u16 self) {
    if (World.instances[self].entflags & EF_DEAD_CHECKS_DONE) {return;}
    flag_set(&World.instances[self].entflags,EF_DEAD_CHECKS_DONE,true); CreateDeathEffects(self,World.instances[self].deathBurst); if (World.instances[self].index == 419) play_wav(sounds[64],1.0f,World.position[self],true);/*npc_autobomb: explosion1*/ if (npcTable[World.instances[self].index - 419].type == NPCType_Cyber) DeleteInstance(self);
}

static void ObjectDeath(u16 self) {
    Entity* e = &World.instances[self]; if (World.instances[self].entflags & EF_DEAD_CHECKS_DONE) return;
    if (World.instances[self].entflags & EF_DEATH_BURST_DONE) { CreateDeathEffects(self,World.instances[self].deathBurst); DropSearchables(self); if (World.instances[self].index != 279){World.col[self]=COLTYPE_NONE;} HideSelf(self); } else { World.col[self] = COLTYPE_NONE; DropSearchables(self); CreateDeathEffects(self,World.instances[self].deathBurst); }
    flag_set(&World.instances[self].entflags,EF_DEAD_CHECKS_DONE,true); World.instances[self].automapHidden = true;
    if (World.instances[self].securityThreshold > 0) { SecurityType stype = SecurityType_None; if(World.instances[self].index == 477){stype=SecurityType_Camera;}else if(World.instances[self].index == 479){stype=SecurityType_NodeSmall;} else if(World.instances[self].index == 478){stype=SecurityType_NodeLarge;} if(stype != SecurityType_None){ReduceCurrentLevelSecurity(stype);} }
    u16 idx = World.instances[self].index; play_wav(SoundPath((idx < 527 && objectDeathSound[idx] != 0) ? objectDeathSound[idx] : 62/*crate_break*/),1.0f,World.position[self],true); if(e->deathBurst != 0){HideSelf(self);}
}

static void ScreenDeath(u16 self) { Entity* e=&World.instances[self]; if(e->entflags & EF_DEAD_CHECKS_DONE){return;} flag_set(&e->entflags,EF_DEAD_CHECKS_DONE,true); play_wav(sounds[69],1.0f,World.position[self],true);/*screen_destroy*/ if (e->entflags & EF_DEATH_BURST_DONE) ObjectDeath(self);/*gib path*/ }
static void VaporizeCorpse(u16 self,bool energyVaporized) { Entity* e=&World.instances[self]; flag_set(&e->entflags,EF_DEAD_CHECKS_DONE,true); DropSearchables(self); e->modelIndex=MAX_MDLS; if (IdxIsNPC(e->index) || IdxIsSearchable(e->index)) DeleteInstance(self); CreateDeathEffects(self,energyVaporized ? 2 : ((e->deathBurst == 0) ? 1/*Corpse hit fallback*/ : e->deathBurst)); }
static inline bool IsGrenade(u16 i) { return ((i >= 314 && i <= 320) || i == 370 || i == 372 || i == 387 || i == 389 || (i >= 402 && i <= 404)); }
static void Death(u16 self,bool energyVaporized) {
    Entity* e = &World.instances[self]; if (e->entflags & EF_DEAD_CHECKS_DONE) return; UseDeathTargets(self); bool isNPC = IdxIsNPC(e->index); bool isObj = IdxIsDynamicObject(e->index); if (e->entflags & EF_ACT_AS_CORPSE_ONLY) { e->entflags |= EF_DEAD_CHECKS_DONE; return; }
    bool vaporize=(IdxIsNPC(e->index) && e->health <= 0.0f) || IdxIsCorpse(e->index); bool isGrenade=IsGrenade(e->index), doTeleport=(e->entflags & EF_TELEPORT_ON_DEATH) != 0; if (e->iceActive) World.col[self] = COLTYPE_NONE;
    if (vaporize && e->index != 477/*sec_camera*/ && !isGrenade) VaporizeCorpse(self,energyVaporized); else if (isObj) ObjectDeath(self); else if (e->index == 279/*screen*/) ScreenDeath(self); else if (doTeleport) TeleportAway(self); else if (isGrenade) GrenadeExplode(self);
    if (isNPC && !doTeleport) NPCDeath(self); else if (self == PLAYER1) { if (!RessurectPlayer()) World.deaths++; } flag_set(&e->entflags,EF_DEAD_CHECKS_DONE,true);
}

static const float AI_STOP_DIST=1.28f, AI_STOP_DIST_SQ=(AI_STOP_DIST * AI_STOP_DIST), AI_POS_CHECK_DELAY=2.0f, AI_WANDER_RANGE=79.0f, AI_TARGET_OFFSET_Y=0.24f; u16 npcCountInWorldPerType[NUM_AI_TYPES]; void DoorActuate(u16 self); void initGunOffsets(void); bool PositionVisibleFromPlayerCell(float,float);
// Name,AtkTyp1,2,3,Dmg1,2,3,Range1,2,3,Health,CybHealth,Percp,Disrp,Armr,Def,Movtyp,Yawspd,FOV,FOVAtk,FOVStartMov,DistToSeeBehind,SightRange,WalkSpd,RunSpd,AtkSpd1,2,3,AtkForce3,AtkRad3,TtPain,TbwPain,TtDead,TtActualAtk1,2,3,TbwAtk1,2,3,TEnemChg,TIdleSFXMin,TIdleSFXMax,TAtk1WaitMin,TAtk1WaitMax,TAtk1WaitChnc,TAtk2WaitMin,TAtk2WaitMax,TAtk2WaitChnc,TAtk3WaitMin,TAtk3WaitMax,TAtk3WaitChnc,ProjType1,2,3,ProjSpd1,2,3,HasLaser1,2,3,ExplodeOn3,PreActMeleCols,THunt,FlightHeight,FlightHeightIsPerc,SwitchMatOnDie,RangeHear,TTranq,Hops,NPCType,AtkProj1,2,3
NPCTable npcTable[NUM_AI_TYPES] = {
/* 0*/{"AUTOBOMB"              ,0,0,1,  0,  0,200,   0,    0,2.4,50,0,1,0.5,40,1,1,300,180,120,55,3.84,50,2.5,2.5,0,0,0,100,6,0,0,0.1,0,0,0,0,0,0,3,5,12,0.5,1,0.1,1,3,0.5,0,0,0,0,0,0,0,0,0,0,0,0,1,0,20,0,0,0,10,3,0,2,0,0,0 },
/* 1*/{"CYBORG ASSASSIN"       ,0,4,7, 30, 50, 35, 3.3,   10,20,65,0,2,0.6,5,4,1,180,180,80,15,3.2,50,2,2,0,0,0,0,0,0.45,5,2.083,0,0.25,0.2,0.91,0.91,1.58,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,0,3,0,0,0,0,0,60,0,0,0,10,3,0,3,0,0,489 },
/* 2*/{"AVIAN MUTANT"          ,1,0,0, 40, 40,  0, 3.3,   10,20,125,0,1,0.25,0,2,2,180,180,80,15,5.12,50,2,2,3.5,0,0,0,0,2,5,1,0.1,0,0,1,0,0,3,5,12,0.5,1,0.1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,60,0.65,1,0,10,3,0,1,0,0,0 },
/* 3*/{"EXEC-BOT"              ,0,4,0, 30, 35,  0, 3.3,   10,20,225,0,1,0.2,40,2,1,200,180,15,30,4.12,50,1.5,1.5,0,0,0,0,0,0.45,7,0.15,0,0.2,0,0,1.5,0,3,5,12,0,0,0,0.97,2,0.3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,180,0,0,0,10,3,0,2,0,0,0 },
/* 4*/{"CYBORG DRONE"          ,0,4,0, 20, 20, 20, 3.3,   25,50,60,0,1,0.3,0,2,1,65,180,80,15,3.2,50,1.6,2.2,0,0,0,0,0,0.542,15,0.958,0,0.1,0,0,1,0,3,20,45,0,0,0,1,2,0.5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,60,0,0,0,10,3,0,3,0,0,0 },
/* 5*/{"CORTEX REAVER"         ,0,4,7, 80,325,125, 3.3,   20,30,580,0,1,0.1,40,2,1,180,180,80,15,3.84,50,2,2,0,0,0,0,0,0.583,5,0.333,0,0.35244,0.324,0,1,1,3,15,30,0,0,0,0.2,1,0.5,8,15,1,0,0,0,0,0,10,0,0,0,0,0,600,0,0,0,10,3,0,2,0,0,372 },
/* 6*/{"CYBORG WARRIOR"        ,0,4,7, 35, 35,150, 3.3,   20,20,120,0,1,0.1,5,4,1,180,180,30,15,3.2,50,2.4,2.4,0,0,0,0,0,0.5,5,2.2,0,0.339,0.201,0,0.83,0.542,3,15,30,0,0,0,1,2,0.5,10,20,1,0,0,0,0,0,10,0,0,0,0,0,180,0,0,0,10,3,0,3,0,0,370 },
/* 7*/{"CYBORG ENFORCER"       ,1,4,7, 60, 60, 80, 3.3,   15,30,285,0,1,0.1,30,5,1,180,180,80,15,3.2,50,2.8,2.8,2.8,0,0.3,0,0,2,5,1.5,0.23471,0.393738,0.313266,0.958,0.958,0.958,5,15,30,0.1,0.3,0.1,0.1,0.5,0.5,10,25,1,0,0,0,0,0,10,0,0,0,0,0,600,0,0,0,10,3,0,4,0,0,387 },
/* 8*/{"CYBORG ELITE GUARD"    ,1,7,4, 70, 75,  0, 3.3,   10,50,380,0,1,0.05,50,6,1,180,180,80,15,3.2,50,3,3,1.5,0,0,0,0,0.4665,5,1.5,0.5,0.2653,0.117045,0.733,0.7,0.867,5,15,30,0.05,0.2,0.1,0.5,2,0.8,2,3,0.5,0,0,0,0,2,0,0,1,0,0,0,600,0,0,0,15,3,0,4,0,490,0 },
/* 9*/{"CYBORG OF EDWARD DIEGO",1,7,0, 80, 95,  0, 3.3,   40,50,900,0,2,0,55,6,1,180,180,80,15,3.2,50,2.8,2.8,0,0,0,0,0,0,0,0,0.28,0.363188,0.2,1.4,0.833,3,5,15,30,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,2.5,0,0,0,0,0,1,600,0,0,0,15,3,0,4,0,490,0 },
/*10*/{"SECURITY-1 ROBOT"      ,0,4,0, 35, 35,  0, 3.3,   10,20,170,0,1,0.15,40,4,2,180,180,80,15,4.12,50,2.5,2.5,1.5,0,0,0,0,2,5,0.05,0.5,0.1,0.2,1.2,1.5,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,0,0,0,0,0,0,0,600,1.28,0,0,10,3,0,2,0,0,0 },
/*11*/{"SECURITY-2 ROBOT"      ,0,4,4, 65, 65, 15, 3.3,    5,35,300,0,2,0.05,50,5,1,180,180,60,25,4.12,50,1.5,1.5,1.5,0,0,0,0,0.75,5,0.25,0.5,0.39,0.1,1.2,1,1.5,3,5,12,0.5,1,0.1,3,3.5,1,2.5,3.5,1,0,0,0,0,0,0,0,0,0,0,0,600,0,0,0,10,3,0,2,0,0,0 },
/*12*/{"MAINTENANCE ROBOT"     ,1,0,0, 25, 25,  0, 3.3,  3.3,20,75,0,1,0.3,40,3,1,180,180,80,15,3.84,50,2.2,2.6,0.02,0.02,0,0,0,0,0,1.6,3,0.7,0.2,2,1.3,3,3,5,12,0.5,1,0.1,1,2,0.3,1,2,0.5,0,0,0,0,0,0,0,0,0,0,0,180,0,0,0,10,3,0,2,0,0,0 },
/*13*/{"MUTANT CYBORG"         ,1,7,0, 35, 75, 50,   2,   30,49,340,0,1,0.2,15,6,1,180,180,60,15,3.2,50,1.5,1.5,0,0,0,0,0,0.583,3.5,3.41,0.265,0.285,0.2,0.625,0.75,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,2.8,0,0,0,0,0,0,180,0,0,0,10,3,0,5,0,491,0 },
/*14*/{"HOPPER"                ,0,4,0, 35, 35,  0,   0,17.92,17.92,150,0,1,0.25,35,4,1,180,160,80,15,3.84,50,7,7,0,0,0,0,0,0.708,5,0,0.5,0.1,0.5,0.5,0.5,0.5,3,5,12,0.5,1,0.1,0.5,1,0.5,1,2,0.5,0,0,0,0,0,0,0,1,0,0,0,180,0,0,0,10,3,1,2,0,0,0 },
/*15*/{"HUMANOID MUTANT"       ,1,0,0, 12, 12,  0, 3.3,   10,20,50,0,0,0.4,0,3,1,60,180,80,15,2.56,50,1.4,2,0.5,0,0,0,0,0.42,5,0.967,0.5,0.1,0.2,1.2,1.5,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,0,0,0,0,0,0,0,20,0,0,0,10,3,0,0,0,0,0 },
/*16*/{"INVISIBLE MUTANT"      ,0,7,0, 10, 35,  0, 3.3,   20,20,350,0,1,0.05,0,2,2,180,180,80,15,2.56,50,0.7,0.7,1.5,0.7,0.7,0,0,0.875,5,1.125,0.875,0.4,0.2,1.2,0.875,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,2,0,0,0,0,0,0,60,0.32,0,1,10,3,0,0,0,486,0 },
/*17*/{"VIRUS MUTANT"          ,0,7,0, 45, 30,  0, 3.3,   20,20,140,0,0,0.1,0,3,1,180,180,80,15,2.56,50,2.5,2.5,2.5,0.3,0,0,0,0.542,3,1.792,0.2874,0.2874,0.2874,0.958,0.958,0.958,3,5,12,0.5,1,0.1,0.5,0,0.5,1,2,0.5,0,0,0,0,1.75,0,0,0,0,0,0,20,0,0,0,10,3,0,1,0,481,0 },
/*18*/{"SERV-BOT"              ,1,0,0,  8,  0,  0, 3.3,   10,20,20,0,1,0.5,20,2,1,180,180,80,15,3.84,50,2,2,1.2,0,0,0,0,1.125,2,0.98,0.2,0.1,0.2,0.834,1.5,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,0,0,0,0,0,0,0,180,0,0,0,10,3,0,2,0,0,0 },
/*19*/{"FLIER BOT"             ,0,4,7, 30,150,  0, 3.3,   35,40,75,0,1,0.3,30,2,2,180,180,80,15,5.12,50,1.5,1.5,1.5,0,0,0,0,1.375,5,0.6,0.1,0.1,0.2,1,1.5,3,3,5,12,0.5,1,0.1,1,2,0.5,10,12,1,0,0,0,0,0,10,0,0,0,0,0,180,0.85,1,0,10,3,0,2,0,0,404 },
/*20*/{"ZERO-G MUTANT"         ,0,7,0, 20, 20,  0, 3.3,   20,20,90,0,1,0.5,0,2,2,180,180,80,15,2.56,50,0.8,1.4,0,0.8,0,0,0,0.1,0,0.1,0.5,0.05,0.2,1.2,1.5,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,2,0,0,0,0,0,0,60,1.96,0,0,10,3,0,0,0,488,0 },
/*21*/{"GORILLA TIGER MUTANT"  ,1,0,0, 60, 60,  0, 3.3, 3.84,20,200,0,1,0.1,0,3,1,180,180,80,15,2.56,50,3,3.5,1,2,0,0,0,0.667,5,1.625,0.5,0.1,0.2,0.958,1.042,3,3,15,30,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,0,0,0,0,0,0,0,60,0,0,0,10,3,0,1,0,0,0 },
/*22*/{"REPAIR BOT"            ,0,4,0, 12, 12,  0, 3.3,  3.3,20,65,0,1,0.4,25,3,1,180,180,80,15,3.84,50,2.25,3,0.5,0,0,0,0,0,0,0.05,0.2,0.1,0.2,1.25,1.5,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,0,0,0,0,0,0,0,180,0,0,0,10,3,0,2,0,0,0 },
/*23*/{"PLANT MUTANT"          ,0,7,0, 35, 25,  0, 3.3,   20,20,115,0,1,0.3,0,1,1,180,180,80,15,2.56,50,0.8,1.2,0.1,0,0,0,0,0.375,2,2.208,0.89,0.82,0.2,1.91,1.027,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,3.5,0,0,0,0,0,0,20,0,0,0,10,3,0,0,0,487,0 },
/*24*/{"CYBER DOG"             ,0,7,0,  0, 25,  0,   0,   20,0,0,20,1,0.5,0,1,4,250,240,50,15,20.48,25.6,2,2,0,0,0,0,0,0.1,0,0.5,0,0,0,0,0.3,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1.5,0,0,0,0,0,0,500,0.75,0,0,10,0,0,6,0,493,0 },
/*25*/{"CYBER GUARD"           ,0,7,0,  0, 25,  0,   0,   20,0,0,35,1,0.4,0,1,4,250,240,50,15,20.48,25.6,2,2,0,0,0,0,0,0.1,0,0.5,0,0,0,0,0.2,0,2,998,999,0,0,0,0,0,0,0,0,0,0,0,0,0,0.8,0,0,0,0,0,0,500,0.75,0,0,10,0,0,6,0,493,0 },
/*26*/{"CYBER RAM"             ,0,7,0,  0, 35,  0,   0,   20,0,0,40,1,0.25,0,1,4,80,240,50,15,20.48,25.6,4,4,0,0,0,0,0,0.1,0,0.5,0,0,0,0,0.2,0,2,998,999,0,0,0,0,0,0,0,0,0,0,0,0,0,1.2,0,0,0,0,0,0,500,0.75,0,0,10,0,0,6,0,494,0 },
/*27*/{"CYBER CORTEX REAVER"   ,0,7,0,  0, 45,  0,   0,   20,0,0,80,1,0.1,0,1,4,80,240,50,15,20.48,25.6,4,4,0,0,0,0,0,0.1,0,0.5,0,0,0,0,0.2,0,2,998,999,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,500,0.75,0,0,10,0,0,6,0,494,0 },
/*28*/{"SHODAN"                ,0,7,0,  0, 55,  0,   0,   20,0,0,500,2,0,0,1,4,360,280,280,15,20.48,25.6,0,0,0,0,0,0,0,0.1,0,0.5,0,0,0,0,0.05,0,2,998,999,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,500,0.75,0,0,10,0,0,6,0,494,0 }};
//                  NPC Sounds 0, 1,  2, 3, 4,  5,  6,  7,  8,  9,10, 11,12,13,14, 15, 16, 17, 18, 19, 20, 21,22, 23, 24, 25, 26, 27, 28                                      0,  1,  2,  3, 4,  5, 6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28
int sfxIdle[NUM_AI_TYPES]   ={-1,-1, -1,-1,58, -1, 59, -1, 59, 52,-1, -1,-1,-1,-1, -1,121, -1, -1, -1,121,118,-1, -1, -1, -1, -1, -1, -1}; int sfxSightSound[NUM_AI_TYPES] ={-1, -1,111,150,58,150,59,152,152, -1,150,150,151,152,150, -1,121, -1,151,150,121,119,151, -1, -1, -1, -1, -1, -1};
int sfxAttack1[NUM_AI_TYPES]={-1,-1,108,-1,-1,146, -1,146,252,247,-1, -1,-1,-1,-1,122, -1,108,146, -1, -1,118,-1,125,258,258,258,258,258}; int sfxAttack2[NUM_AI_TYPES] =   {-1,256, -1,148,50, 50,50, 50, 50,250, 50, 50,146,259,148, -1,121, -1, -1,147, -1, -1,146, -1,258,258,258,258,258};
int sfxAttack3[NUM_AI_TYPES]={-1,-1, -1,-1,-1,244,244,244,245, -1,-1,149,-1,-1,-1, -1, -1, -1, -1,244, -1, -1,-1, -1,258,258,258,258,258}; int sfxDeath[NUM_AI_TYPES] =     {-1, 48,110,143,48,145,48, 51, 47, 47,142,143,144, 47,162,123,120,134,144,144,120,117,144,124, -1, -1, -1, -1, -1};
float deathBurstTimer[NUM_AI_TYPES] = {0.0f,0.0f, 0.1f,0.0f,0.1f,0.1f,0.2f,0.1f,0.1f,0.1f,0.0f,0.45f,0.75f,0.1f,0.0f,0.0f,0.1f,0.224f,0.9f,0.0f,0.1f,0.1f,0.1f,0.2f,0.1f,0.1f,0.1f,0.1f,0.1f};
void InitNPC(u16 i) {
    static bool gunOffsetsInit = false; if (!gunOffsetsInit) { initGunOffsets(); gunOffsetsInit = true; }
    World.layer[i] = L_NPC; u16 npcID = World.instances[i].index - 419;
    World.instances[i].currentDestination = World.instances[i].lastPosition = World.instances[i].idealPos = World.position[i]; World.instances[i].idealTransformForward = World.instances[i].forward;
    World.instances[i].tickFinished = World.pauseRelativeTime + (double)random_range(0.0f, 1.0f); World.instances[i].idleTime = World.pauseRelativeTime + (double)random_range(npcTable[npcID].timeIdleSFXMin,npcTable[npcID].timeIdleSFXMax);
    World.instances[i].attack1SoundTime = World.instances[i].attack2SoundTime = World.instances[i].attack3SoundTime = World.pauseRelativeTime; World.instances[i].huntFinished = World.pauseRelativeTime; int diff = (npcTable[npcID].type == NPCType_Cyber) ? World.diffCyb : World.diffCbt;
    if (diff <= 1) { World.instances[i].huntFinished += vmax((npcTable[npcID].huntTime * 0.75),60.0); }/*More forgetful on easy.*/ else if (diff >= 3) { World.instances[i].huntFinished += vmax((npcTable[npcID].huntTime * 2.00),60.0); }/*Good memory on hard.*/ else { World.instances[i].huntFinished += vmax(npcTable[npcID].huntTime,60.0); }
    World.instances[i].attackFinished = World.pauseRelativeTime + 1.0; World.instances[i].attack2Finished = World.instances[i].attack3Finished = World.instances[i].timeTillPainFinished = World.instances[i].timeTillDeadFinished = World.instances[i].meleeDamageFinished = World.instances[i].gracePeriodFinished = World.pauseRelativeTime;
    World.instances[i].randWaitAtt1Finished = World.instances[i].randWaitAtt2Finished = World.instances[i].randWaitAtt3Finished = World.instances[i].tranquilizeFinished = World.instances[i].deathBurstFinished = World.instances[i].wanderFinished = World.instances[i].posCheckFinished = World.instances[i].timeTillEnemyChangeFinished = World.pauseRelativeTime;
    World.instances[i].timeSinceMovedEnough = 0.0; World.instances[i].currentState = AIState_Idle; u8 c=A_IDLE;
    if ((World.instances[i].entflags & EF_WANDERING) && (random_range(0.0f,1.0f) < 0.5f)){World.instances[i].currentState = AIState_Walk;} else {flag_set(&World.instances[i].entflags,EF_WANDERING,false);}
    if (World.instances[i].entflags & EF_ASLEEP) { World.instances[i].currentState=AIState_Idle; /*flag_set(&World.instances[e->sleepingCables].entflags, EF_ACTIVE, true);*//*deactivated sleeping cables in AIAwakeFromSleep*/ }
    switch (World.instances[i].currentState){case AIState_Walk:c=A_WALK; break; case AIState_Run:c=A_RUN; break; case AIState_Attack1:c=A_ATTACK1; break; case AIState_Attack2:c=A_ATTACK2; break; case AIState_Attack3:c=A_ATTACK3; break; case AIState_Pain:c=A_PAIN; break; case AIState_Dying: case AIState_Dead:c=A_DYING; break;}
    World.instances[i].clip = c; World.instances[i].frame = modelAnimationClips[World.instances[i].animationNum][c].frameStart; World.instances[i].currentFrameFinished = 0.0;
}

float Tranquilize(u16 i, float amount, bool energy) { u16 n=World.instances[i].index-419; if (npcTable[n].type == NPCType_Robot && !energy){return 0.0f;} float tranqSecs=(amount<3.0f) ? npcTable[n].timeForTranquilization : amount; World.instances[i].tranquilizeFinished=vmax(World.pauseRelativeTime+tranqSecs,World.instances[i].tranquilizeFinished+tranqSecs); return tranqSecs; }
bool IsCyberNPC(u16 i) { u16 npcID = World.instances[i].index - 419; return npcTable[npcID].type == NPCType_Cyber; }
bool HasHealth(u16 i) { if(IsCyberNPC(i)){return (World.instances[i].cyberHealth > 0.0f);} return (World.instances[i].health > 0.0f); }
INLINE bool ai_is_cyber(Entity* e)  { return npcTable[e->index - 419].type == NPCType_Cyber; }
INLINE bool ai_has_health(Entity* e){ return ai_is_cyber(e) ? e->cyberHealth > 0.0f : e->health > 0.0f; }
float sightPointHeights[NUM_AI_TYPES]={0.746f,0.824f,0.261f,0.89f,0.86f,0.875f,0.736f,0.923f,1.082f,1.014f,0.578f/*10*/,0.797f,0.185f,1.155f,1.759f,0.843f,0.133f,0.651f,0.323f,0.0f,0.0f/*20*/,0.0f,-0.662f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f/*28*/};
V3 gunOfs[NUM_AI_TYPES]; V3 gunOfs2[NUM_AI_TYPES];
void initGunOffsets(void) { for (int i=0;i<NUM_AI_TYPES;i++) { gunOfs[i]=(V3){0.0f,sightPointHeights[i]+0.3f,0.0f}; gunOfs2[i]=(V3){0.0f,sightPointHeights[i]+0.15f,0.0f}; } }
INLINE V3 ai_sight_pos(Entity* e) { u16 idx=(u16)(e - World.instances); return V3_AplusB(World.position[idx],(V3){0.0f,sightPointHeights[World.instances[idx].index - 419],0.0f}); }
INLINE V3 ai_gun_pos(Entity* e, int n) { u16 idx=(u16)(e - World.instances); u16 npcIdx=World.instances[idx].index - 419; V3 off = (n == 3) ? gunOfs[npcIdx] : gunOfs2[npcIdx]; if (n == 2 && off.x == 0.0f && off.y == 0.0f && off.z == 0.0f) off=gunOfs2[npcIdx]; return V3_AplusB(World.position[idx],off); }
Quaternion quat_look_rotation(V3 fwd, V3 up) {
    fwd=V3_Normalize(fwd); V3 r = V3_Normalize(V3_Cross(up,fwd)); up=V3_Cross(fwd,r); float m00=r.x, m01=r.y, m02=r.z, m10=up.x, m11=up.y, m12=up.z, m20=fwd.x, m21=fwd.y, m22=fwd.z; float tr = m00 + m11 + m22; Quaternion q;
    if (tr > 0.0f){float s=0.5f/vsqrtf(tr+1.0f); q.w=(0.25f/s); q.x=(m12-m21)*s; q.y=(m20-m02)*s; q.z=(m01-m10)*s;}else if(m00 > m11 && m00 > m22){float s=2.0f*vsqrtf(1.0f+m00-m11-m22); q.w=(m12-m21)/s; q.x=0.25f*s; q.y=(m01+m10)/s; q.z=(m20+m02)/s;}else if(m11 > m22){float s=2.0f*vsqrtf(1.0f+m11-m00-m22); q.w=(m20-m02)/s; q.x=(m01+m10)/s; q.y=0.25f*s; q.z=(m12+m21)/s; } else { float s = 2.0f * vsqrtf(1.0f + m22 - m00 - m11); q.w=(m01-m10)/s; q.x=(m20+m02)/s; q.y=(m12+m21)/s; q.z=0.25f*s;} return q;
}

void aiac_idle(Entity* self) { if ((self->entflags & EF_ASLEEP) || self->tranquilizeFinished >= World.current_time) {self->currentFrameFinished=World.current_time + 1e9; return;/*freeze*/} ChangeAnim(self,A_IDLE); }
void aiac_walk(Entity* self){if(self->entflags & EF_ACT_AS_TURRET){aiac_idle(self); return;} u16 idx=(u16)(self-World.instances); if((World.velocity[idx].x*World.velocity[idx].x+World.velocity[idx].z*World.velocity[idx].z)>(0.32f*0.32f)){ChangeAnim(self,A_WALK); return;} if (self->animSwapFinished<World.current_time){self->animSwapFinished=World.current_time+.5f; ChangeAnim(self,A_IDLE);}}
void aiac_dying(Entity* self) { flag_set(&self->entflags,EF_ASLEEP,false); AnimationClip cl=modelAnimationClips[self->animationNum][A_DYING]; if(cl.frameEnd == cl.frameStart){ChangeAnim(self,A_DYING); return;} ChangeAnim(self,A_DYING); /* animatorPlaybackTime deferred; playback transition handled by state machine */}
void AIAnimationControllerUpdate(u16 idx) {
    Entity* self = &World.instances[idx]; if((!(self->entflags & EF_ACTIVE)) || (self->animationNum >= MAX_ANIMS)){return;} if(self->currentState == AIState_Dying){aiac_dying(self); return;}
    if(self->currentState == AIState_Dead){AnimationClip cl=modelAnimationClips[self->animationNum][A_DYING]; self->clip=A_DYING; self->frame=cl.frameEnd; self->modelIndex=cl.frameStartModelIndex + (cl.frameEnd - cl.frameStart); self->currentFrameFinished=World.current_time + 1e9; return;/*freeze*/} if(self->entflags & EF_ASLEEP){aiac_idle(self); return;}
    if(self->currentState == AIState_Run && self->tranquilizeFinished >= World.current_time){aiac_idle(self); return;}
    switch (self->currentState) { case AIState_Walk:aiac_walk(self); break; case AIState_Run:if(self->entflags & EF_ACT_AS_TURRET){aiac_idle(self);}else{ChangeAnim(self,A_RUN);} break; case AIState_Attack1:ChangeAnim(self,A_ATTACK1); break; case AIState_Attack2:ChangeAnim(self,A_ATTACK2); break; case AIState_Attack3:ChangeAnim(self,A_ATTACK3); break; case AIState_Pain:ChangeAnim(self,A_PAIN); break; default:aiac_idle(self); break; }
}

bool AICheckIfEnemyInSight(u16 idx) {
    u16 eidx=World.instances[idx].enemy; if (!eidx || !ai_has_health(&World.instances[idx])) return false; bool enIsNPC = (World.layer[eidx] & L_NPC) != 0; int diff = ai_is_cyber(&World.instances[idx]) ? World.diffCyb : World.diffCbt; if (!ai_is_cyber(&World.instances[idx]) && !enIsNPC && !PositionVisibleFromPlayerCell(World.position[idx].x,World.position[idx].z)) return false;
    if (diff == 0 && (World.instances[idx].index - 419) != 28) return false; if (Cheats.notarget && !enIsNPC) { World.instances[idx].enemy = 0; World.instances[idx].posCheckFinished = World.pauseRelativeTime + AI_POS_CHECK_DELAY; World.instances[idx].lastPosition = World.position[idx]; flag_set(&World.instances[idx].entflags, EF_ENEM_IN_LOS, false); return false; }
    if (ai_is_cyber(&World.instances[idx]) && World.decoyActive) { flag_set(&World.instances[idx].entflags, EF_ENEM_IN_LOS, false); return false; } float dist = V3_Dist(World.position[eidx], ai_sight_pos(&World.instances[idx])); if (dist > npcTable[World.instances[idx].index - 419].sightRange) return false; if (ai_is_cyber(&World.instances[idx]) || enIsNPC) return true;
    V3 spos=ai_sight_pos(&World.instances[idx]); V3 lineN=V3_Normalize(V3_AsubB(World.position[eidx],spos));
    RaycastHit hit=Raycast(spos,lineN,npcTable[World.instances[idx].index-419].sightRange,LMASK_NPC_SIGHT);
    if (hit.hit) {
        if (hit.hitInstanceIndex == eidx) { flag_set(&World.instances[idx].entflags, EF_ENEM_IN_LOS, true); return true; } NPCType t = npcTable[World.instances[idx].index - 419].type;
        if (t != NPCType_Mutant && t != NPCType_Supermutant && t != NPCType_Cyber) {/*Smarte npcs attempt to open doors btw them and player.*/
            u16 hi=hit.hitInstanceIndex; if (hi&&V3_SqDist(hit.point,spos)<4.0f&&IdxIsDoor(World.instances[hi].index)){Entity* dr=&World.instances[hi]; if ((dr->doorOpen == DoorState_Closed || (dr->doorOpen == DoorState_Closing && World.diffCbt > 2)) && !(dr->entflags & EF_LOCKED) && GetCurrentLevelSecurity() <= dr->securityThreshold && (dr->requiredAccessCard == ACC_None)) DoorActuate(hi);}
        }
    } flag_set(&World.instances[idx].entflags, EF_ENEM_IN_LOS, false); return false;
}

void AISetHuntFinished(u16 idx) { World.instances[idx].huntFinished = World.pauseRelativeTime; int diff = ai_is_cyber(&World.instances[idx]) ? World.diffCyb : World.diffCbt; double ht = npcTable[World.instances[idx].index - 419].huntTime, mn=60.0; if(diff <= 1){World.instances[idx].huntFinished += (ht * 0.75 > mn ? ht * 0.75 : mn);}else if(diff >= 3){World.instances[idx].huntFinished += (ht * 2.0  > mn ? ht * 2.0 : mn);}else{World.instances[idx].huntFinished += (ht > mn ? ht : mn);} }
void AISetEnemy(u16 idx, u16 eidx) {
    if(!eidx){return;} World.instances[idx].enemy=eidx; World.instances[idx].posCheckFinished=World.pauseRelativeTime + AI_POS_CHECK_DELAY; flag_set(&World.instances[idx].entflags,EF_WANDERING,false); World.instances[idx].wanderFinished=World.pauseRelativeTime;
    World.instances[idx].lastPosition = World.position[idx]; World.instances[idx].lastKnownEnemyPos = World.position[eidx]; World.instances[idx].targettingPosition = (V3){World.position[eidx].x,World.position[eidx].y + AI_TARGET_OFFSET_Y,World.position[eidx].z}; AISetHuntFinished(idx);
}

void AIPlaySightSound(u16 idx) { if ((!(World.instances[idx].entflags&EF_FIRST_SIGHTING)) || (!ai_has_health(&World.instances[idx])) || (World.instances[idx].entflags&EF_ACT_AS_CORPSE_ONLY)){return;} flag_set(&World.instances[idx].entflags,EF_FIRST_SIGHTING,false); i16 sfx = sfxSightSound[World.instances[idx].index - 419]; if (sfx >= 39 && sfx < SOUNDS_COUNT){play_wav(sounds[sfx],SfxVol(),World.position[idx],true);} }
bool AICheckIfPlayerInSight(u16 idx) {
    int diff = ai_is_cyber(&World.instances[idx]) ? World.diffCyb : World.diffCbt; if ((!ai_is_cyber(&World.instances[idx]) && !PositionVisibleFromPlayerCell(World.position[idx].x,World.position[idx].z)) || (diff == 0 && (World.instances[idx].index - 419) != 28)) return false; if (World.instances[idx].enemy) return AICheckIfEnemyInSight(idx);
    flag_set(&World.instances[idx].entflags,EF_ENEM_IN_LOS,false); if ((ai_is_cyber(&World.instances[idx]) && World.decoyActive) || Cheats.notarget){return false;} V3 playerPos=World.position[PLAYER1], spos=ai_sight_pos(&World.instances[idx]); float dist=V3_Dist(playerPos,spos); NPCTable* npc = &npcTable[World.instances[idx].index - 419]; if (dist > npc->sightRange) return false;
    if (ai_is_cyber(&World.instances[idx])) { AISetEnemy(idx,PLAYER1); AIPlaySightSound(idx); return true; } V3 checkN = V3_Normalize(V3_AsubB(playerPos,spos)); float cosA = vclamp(V3_dot(checkN,World.instances[idx].forward), -1.0f, 1.0f); float angle = vacosf(cosA) * (180.0f / PI); bool makingNoise = World.invP1.noiseFinished > World.pauseRelativeTime;
    if (angle < npc->fov * 0.5f) { RaycastHit hit = Raycast(spos, checkN, dist + 0.1f, LMASK_NPC_SIGHT); if (hit.hit && hit.hitInstanceIndex == PLAYER1) { flag_set(&World.instances[idx].entflags, EF_ENEM_IN_LOS, true); AISetEnemy(idx,PLAYER1); AIPlaySightSound(idx); return true; } if (!hit.hit && makingNoise && dist < npc->hearingRange) { AISetEnemy(idx,PLAYER1); AIPlaySightSound(idx); return true; } }
    else { if (dist < npc->distToSeeBehind) { RaycastHit hit = Raycast(spos,checkN,dist + 0.1f,LMASK_NPC_SIGHT); if (hit.hit && hit.hitInstanceIndex == PLAYER1) { flag_set(&World.instances[idx].entflags, EF_ENEM_IN_LOS, true); AISetEnemy(idx,PLAYER1); AIPlaySightSound(idx); return true; } } if (makingNoise && dist < npc->hearingRange) { AISetEnemy(idx,PLAYER1); AIPlaySightSound(idx); return true; } }   return false;
}

static void AIEnemyInFrontChecks(Entity* e, u16 i) { if(!i){flag_set(&e->entflags,EF_ENEM_IN_FOV,false); flag_set(&e->entflags,EF_ENEM_IN_FRONT,false); return;} if(ai_is_cyber(e)){flag_set(&e->entflags,EF_ENEM_IN_FOV,true); flag_set(&e->entflags,EF_ENEM_IN_FRONT,true); return;} V3 spos=ai_sight_pos(e),epos=World.position[i]; V3 iv=V3_Normalize((V3){epos.x-spos.x,0.0f,epos.z-spos.z}); float d=V3_dot(iv,e->forward); flag_set(&e->entflags,EF_ENEM_IN_FOV,d>0.800f); flag_set(&e->entflags,EF_ENEM_IN_FRONT,d>0.300f); }
static Quaternion quat_slerp(Quaternion a, Quaternion b, float t) {
    float d = quat_dot(a, b); if (d < 0.0f) { b.x=-b.x; b.y=-b.y; b.z=-b.z; b.w=-b.w; d=-d; } if (d > 0.9995f) { Quaternion r = { a.x+t*(b.x-a.x), a.y+t*(b.y-a.y), a.z+t*(b.z-a.z), a.w+t*(b.w-a.w) }; float il = 1.0f / vsqrtf(r.x*r.x + r.y*r.y + r.z*r.z + r.w*r.w); r.x*=il; r.y*=il; r.z*=il; r.w*=il; return r; }
    d = vclamp(d, -1.0f, 1.0f); float th0 = vacosf(d), th = th0*t, sth0 = vsinf(th0); float s0 = vsinf(th0 - th) / sth0, s1 = vsinf(th) / sth0; return (Quaternion){ s0*a.x+s1*b.x, s0*a.y+s1*b.y, s0*a.z+s1*b.z, s0*a.w+s1*b.w };
}

static void AIFace(Entity* self, V3 goal) {
    u16 sidx=(u16)(self - World.instances); if (self->entflags & EF_ASLEEP) return; V3 fv = V3_AsubB(goal,World.position[sidx]); if (!ai_is_cyber(self)) fv.y = 0.0f; if (fv.x == 0.0f && fv.y == 0.0f && fv.z == 0.0f) return; u16 eidx = self->enemy; if (ai_is_cyber(self) && eidx) { World.rotation[sidx] = World.rotation[eidx]; return; }
    if (fv.x == 0.0f && fv.z == 0.0f) { if (eidx){fv=V3_AsubB(World.position[eidx],World.position[sidx]);} else{fv.x += 0.001f;} } Quaternion lr = quat_look_rotation(fv, (V3){0.0f,1.0f,0.0f}); float t = (float)(0.2f * npcTable[self->index - 419].yawSpeed * World.deltaTime); World.rotation[sidx] = quat_slerp(World.rotation[sidx],lr,t);
}

INLINE float quat_angle_deg(Quaternion a, Quaternion b) { float d = vclamp(vabs(quat_dot(a, b)), 0.0f, 1.0f); return 2.0f * vacosf(d) * (180.0f / PI); }
static bool AIWithinAngleToTarget(Entity* self) { if (ai_is_cyber(self)){return true;} if(V3_dot(self->idealTransformForward,self->idealTransformForward)<=1e-6f)return false; u16 sidx=(u16)(self - World.instances); Quaternion lr=quat_look_rotation(self->idealTransformForward,(V3){0,1,0}); float ang=quat_angle_deg(World.rotation[sidx],lr); float fovMov=npcTable[self->index - 419].fovStartMovement; if(ang<fovMov)return true; if(ang<fovMov*1.5f&&random_range(0.0f,1.0f)<0.5f){return true;} return false; }
bool AICheckPain(u16 self) {
    u16 ndx=World.instances[self].index - 419; if(ai_is_cyber(&World.instances[self]) || (World.instances[self].entflags & EF_ASLEEP) || (npcTable[ndx].timeBetweenPain <= 0.0f) || (!(World.instances[self].entflags & EF_GO_INTO_PAIN) || World.instances[self].timeTillPainFinished >= World.pauseRelativeTime)){return false;}
    World.instances[self].currentState = AIState_Pain; u16 atkIdx = World.instances[self].recentMostActivator;
    if (atkIdx && World.instances[self].timeTillEnemyChangeFinished < World.pauseRelativeTime) {
        World.instances[self].timeTillEnemyChangeFinished = World.pauseRelativeTime + npcTable[ndx].timeToChangeEnemy; bool atkIsPlayer = (World.layer[atkIdx] & L_Player) != 0;
        if (!atkIsPlayer && IdxIsNPC(World.instances[atkIdx].index)) {
            NPCType mt = npcTable[ndx].type, at = npcTable[World.instances[atkIdx].index - 419].type; bool canFight = World.instances[atkIdx].index != World.instances[self].index; if ((mt == NPCType_Robot && World.instances[self].enemy) || ((mt == NPCType_Cyborg || mt == NPCType_Supercyborg || mt == NPCType_Robot) && (at == NPCType_Cyborg || at == NPCType_Supercyborg || at == NPCType_Robot))) canFight = false; if (canFight) World.instances[self].enemy=atkIdx;
        } else World.instances[self].enemy=atkIdx;
        World.instances[self].posCheckFinished=World.pauseRelativeTime+AI_POS_CHECK_DELAY; flag_set(&World.instances[self].entflags,EF_WANDERING,false); World.instances[self].wanderFinished=World.pauseRelativeTime; World.instances[self].lastPosition=World.position[self]; if(World.instances[self].enemy){World.instances[self].lastKnownEnemyPos=World.instances[self].currentDestination=World.position[World.instances[self].enemy];}
    } flag_set(&World.instances[self].entflags, EF_GO_INTO_PAIN, false); World.instances[self].timeTillPainFinished = World.pauseRelativeTime + npcTable[ndx].timeToPain; return true;
}

static void AIIdle(u16 sidx) {
    if (World.instances[sidx].enemy && ai_has_health(&World.instances[sidx])) { World.instances[sidx].currentState = AIState_Run; return; } NPCTable* npc = &npcTable[World.instances[sidx].index - 419];
    if (World.instances[sidx].idleTime < World.pauseRelativeTime) { int sidle = sfxIdle[World.instances[sidx].index - 419]; if (random_range(0.0f, 1.0f) < 0.5f && sidle >= 0 && sidle < (i16)SOUNDS_COUNT) play_wav(sounds[sidle],SfxVol(),World.position[sidx],true); World.instances[sidx].idleTime = World.pauseRelativeTime + random_range(npc->timeIdleSFXMin, npc->timeIdleSFXMax); } AICheckPain(sidx);
}

static V3 AIGetWanderPoint(Entity* self) { u16 sidx=(u16)(self - World.instances); return (V3){World.position[sidx].x + random_range(-AI_WANDER_RANGE,AI_WANDER_RANGE),ai_is_cyber(self) ? World.position[sidx].y + random_range(-AI_WANDER_RANGE,AI_WANDER_RANGE) : 0.0f,World.position[sidx].z + random_range(-AI_WANDER_RANGE,AI_WANDER_RANGE)}; }
static V3 AIGetAStarPoint(Entity* self) {
    u16 sidx=(u16)(self - World.instances); V3 ep = self->enemy ? World.position[self->enemy] : World.position[sidx]; float px = World.position[sidx].x, py = World.position[sidx].y, pz = World.position[sidx].z; V3 cands[4] = {{px,py,pz + CELLSZ},{px,py,pz - CELLSZ},{px + CELLSZ,py,pz},{px - CELLSZ,py,pz}}; int best = -1; float bestD = 1e9f;
    for (int i = 0; i < 4; ++i) { if (!PositionVisibleFromPlayerCell(cands[i].x, cands[i].z)) continue; float d = V3_SqDist(ep, cands[i]); if (d < bestD) { bestD = d; best = i; } } return best >= 0 ? cands[best] : AIGetWanderPoint(self);
}

static V3 AIGetSearchPoint(Entity* self) { NPCType t = npcTable[self->index - 419].type; if (t == NPCType_Mutant || t == NPCType_Supermutant) {return AIGetWanderPoint(self);} return AIGetAStarPoint(self); }
static void AIHopMove(u16 self) { if (!(World.instances[self].entflags & EF_HOP_DONE)){flag_set(&World.instances[self].entflags,EF_HOP_DONE,true); AddForce(self,V3_ScaleByF(World.instances[self].forward,500.0f),true); AddForce(self,(V3){0,5.0f,0},true);} else {flag_set(&World.instances[self].entflags,EF_HOP_DONE,false);} }
static void AIWalk(u16 self) {
    if ((AICheckPain(self)) || (World.instances[self].entflags & EF_ASLEEP)){return;} if ((World.instances[self].entflags & EF_ENEM_IN_SIGHT) || World.instances[self].enemy){World.instances[self].currentState=AIState_Run; return;} if (World.instances[self].entflags & EF_ACT_AS_TURRET) {World.instances[self].currentState = AIState_Idle; return;}
    if ((npcTable[World.instances[self].index - 419].moveType == AIMoveType_None) || (World.instances[self].tranquilizeFinished >= World.pauseRelativeTime)){return;} u16 sidx = self; if (!PositionVisibleFromPlayerCell(World.position[sidx].x,World.position[sidx].z)){return;}
    float dist = V3_Dist(ai_sight_pos(&World.instances[self]),World.instances[self].currentDestination); if (World.instances[self].entflags & EF_WANDERING) { if (World.instances[self].wanderFinished < World.pauseRelativeTime || dist < AI_STOP_DIST * 0.5f) { World.instances[self].wanderFinished = World.pauseRelativeTime + random_range(3.0f, 8.0f); World.instances[self].currentDestination = AIGetWanderPoint(&World.instances[self]); } }
    if (dist > AI_STOP_DIST && AIWithinAngleToTarget(&World.instances[self])) {
        if (npcTable[World.instances[self].index - 419].hopsOnMove) { if (World.instances[self].modelIndex > 4148){AIHopMove(self);} }
        else {
            float ws  = npcTable[World.instances[self].index - 419].walkSpeed; V3 mv = { World.instances[self].forward.x*ws,World.instances[self].forward.y*ws,World.instances[self].forward.z*ws };
            if (npcTable[World.instances[self].index - 419].moveType != AIMoveType_Fly) { V3 spos = ai_sight_pos(&World.instances[self]); V3 cp = { spos.x + World.instances[self].forward.x*0.48f, spos.y, spos.z + World.instances[self].forward.z*0.48f }; RaycastHit gh = Raycast(cp,(V3){0,-1,0},CELLSZ,LMASK_NPC_COLLISION); if (!gh.hit) { mv.x = 0.0f; mv.z = 0.0f; } } mv.y = World.velocity[sidx].y; World.velocity[sidx] = mv;
        } return;
    } if (!(World.instances[self].entflags & EF_WANDERING)) World.instances[self].currentState = AIState_Idle;
}

static void AIRunMove(u16 self) { if (World.instances[self].entflags & EF_ACT_AS_TURRET){return;} float rs=npcTable[World.instances[self].index - 419].runSpeed; World.velocity[self]=(V3){World.instances[self].forward.x * rs,(vabs(World.gravity[self]) > 0.05f) ? World.velocity[self].y : World.instances[self].forward.y * rs,World.instances[self].forward.z * rs}; }
static void AIHunt(Entity* self) {
    u16 sidx=(u16)(self - World.instances); u16 eidx=self->enemy; if(!eidx){return;} self->currentDestination = ai_is_cyber(self) ? World.position[eidx] : AIGetSearchPoint(self); if ((npcTable[self->index - 419].moveType == AIMoveType_None) || (self->entflags & EF_ACT_AS_TURRET) || (npcTable[self->index - 419].runSpeed <= 0.0f) || (V3_SqDist(ai_sight_pos(self), self->currentDestination) <= AI_STOP_DIST_SQ) || (!AIWithinAngleToTarget(self))){return;}
    float rs=npcTable[self->index - 419].runSpeed; World.velocity[sidx]=(V3){self->forward.x*rs,World.velocity[sidx].y,self->forward.z*rs};
}

float DistToEnemy(u16 self, u16 enem) { if(self >= World.instCount){return 100000.0f;} if(enem >= World.instCount){return 100000.0f;} V3 selfPos = World.position[self], enemPos = World.position[enem]; V3 d = V3_AsubB(selfPos,enemPos); return V3_dot(d,d); }
static bool AICanAttack(u16 selfIdx, float dsq, u8 type, float* rangeToEnemy) {
    Entity* self = &World.instances[selfIdx]; *rangeToEnemy = DistToEnemy(selfIdx,self->enemy); if (*rangeToEnemy >= dsq) return false; AttType att = type == 3 ? npcTable[self->index - 419].attackType3 : (type == 2 ? npcTable[self->index - 419].attackType2 : npcTable[self->index - 419].attackType); if (att == Att_None) return false;
    if (type == 3) { if (*rangeToEnemy < 7.0f && att == Att_Ball) { int p = npcTable[self->index - 419].projectile3Prefab; if(p == 370 || p == 372 || p == 387 || p == 404){return false;} } } if (ai_is_cyber(self)/*Cyber enemies are dumb but aggressive.*/) return true;
    if ((!(self->entflags & EF_ENEM_IN_FRONT)) || (type >= 2 && !(self->entflags & EF_ENEM_IN_FOV))){return false;} float wait = type == 3 ? self->randWaitAtt3Finished : (type == 2 ? self->randWaitAtt2Finished : self->randWaitAtt1Finished); return wait < World.pauseRelativeTime;
}

static void AIBrakingMovement(Entity* self) { u16 sidx=(u16)(self - World.instances); u8 ni=self->index-419; if (ni == 1 || (ni >= 3 && ni <= 9) || (ni >= 11 && ni <= 13) || ni == 17 || ni == 23) { World.velocity[sidx].x *= 0.15f; World.velocity[sidx].z *= 0.15f; } }
static void AIStartAttack(Entity* self, int n) {
    AIBrakingMovement(self); NPCTable* npc = &npcTable[self->index - 419]; double between, toActual; switch (n) { case 1: between = npc->timeBetweenAttack1; toActual = npc->timeToActualAttack1; break; case 2: between = npc->timeBetweenAttack2; toActual = npc->timeToActualAttack2; break; default: between = npc->timeBetweenAttack3; toActual = npc->timeToActualAttack3; break; }
    self->attackFinished = World.pauseRelativeTime + between + toActual; self->gracePeriodFinished = World.pauseRelativeTime + toActual; self->currentState = (AIState)(AIState_Attack1 + (n - 1));
}

static void AIRun(u16 selfIdx) {
    Entity* self = &World.instances[selfIdx]; if ((AICheckPain(selfIdx)) || (self->entflags & EF_ASLEEP)){return;} if(!self->enemy){self->currentState = AIState_Idle; return;} if(self->tranquilizeFinished >= World.pauseRelativeTime && !ai_is_cyber(self)){return;}
    if (self->posCheckFinished <= World.pauseRelativeTime && !ai_is_cyber(self)) {
        self->posCheckFinished = World.pauseRelativeTime + AI_POS_CHECK_DELAY; float dToEn=V3_Dist(ai_sight_pos(self),World.position[self->enemy]); float dToLast=V3_Dist(World.position[selfIdx],self->lastPosition); self->lastPosition=World.position[selfIdx];
        if(dToLast < 0.48f && dToEn > AI_STOP_DIST && !(self->entflags & EF_WANDERING)){self->wanderFinished = World.pauseRelativeTime + 5.0f;/*Same search time as Quake 1*/ flag_set(&self->entflags,EF_WANDERING,true); self->currentDestination=AIGetSearchPoint(self);} else {flag_set(&self->entflags,EF_WANDERING,false);}
    }
    if (!(self->entflags & EF_ENEM_IN_SIGHT)) { if(self->huntFinished > World.pauseRelativeTime){AIHunt(self);} else {self->enemy=0; flag_set(&self->entflags,EF_WANDERING,true); self->wanderFinished=World.pauseRelativeTime + 1.0; self->currentState=AIState_Walk;} return; }
    if (self->enemy && !(self->entflags & EF_WANDERING)) { self->targettingPosition=(V3){World.position[self->enemy].x,World.position[self->enemy].y + AI_TARGET_OFFSET_Y,World.position[self->enemy].z}; self->currentDestination=self->targettingPosition; self->lastKnownEnemyPos=self->targettingPosition; }
    flag_set(&self->entflags,EF_SHOT_FIRED,false); AISetHuntFinished(selfIdx); NPCTable* ndat = &npcTable[self->index - 419]; float nr = ndat->range, near = nr * nr, mr = ndat->range2, mid  = mr * mr, fr = ndat->range3, far  = fr * fr, rangeToEnemy=100000.0f;
    if (AICanAttack(selfIdx,near,1,&rangeToEnemy)) { AIStartAttack(self,1); return; } if (AICanAttack(selfIdx, mid,2,&rangeToEnemy)) { AIStartAttack(self,2); return; } if (AICanAttack(selfIdx, far,3,&rangeToEnemy)) { AIStartAttack(self,3); return; }
    if (ndat->moveType != AIMoveType_None && rangeToEnemy > AI_STOP_DIST_SQ) { if (AIWithinAngleToTarget(self)) { if (ndat->hopsOnMove && !(World.instances[selfIdx].entflags & EF_ACT_AS_TURRET)){ if(World.instances[selfIdx].modelIndex > 4148){AIHopMove(selfIdx);}}else{AIRunMove(selfIdx);} } else if (World.diffCbt >= 2 && random_range(0.0f,1.0f) < 0.5f) { AIFace(self,self->currentDestination); } }
}

static void AIPain(Entity* self) { if (self->timeTillPainFinished < World.pauseRelativeTime) { self->currentState = AIState_Run; flag_set(&self->entflags, EF_GO_INTO_PAIN, false); self->timeTillPainFinished = World.pauseRelativeTime + npcTable[self->index - 419].timeBetweenPain; } }
static bool AIDeactivatesVisibleMeshWhileDying(Entity* self) { return self->index == 419 || self->index == 433 || self->index == 439 || (self->entflags & EF_TELEPORT_ON_DEATH); }
static void AIDying(u16 i) {
    if (!(World.instances[i].entflags & EF_DYING_SETUP)) {
        World.instances[i].enemy = 0; NPCTable* npc = &npcTable[World.instances[i].index - 419]; float dbt = deathBurstTimer[World.instances[i].index - 419]; if (dbt > 0.0f) { World.instances[i].deathBurstFinished = World.pauseRelativeTime + dbt; } else if (!(World.instances[i].entflags & EF_DEATH_BURST_DONE)) { if (World.instances[i].deathBurst > 0) { SpawnDynamicObject(World.instances[i].deathBurst, false); } flag_set(&World.instances[i].entflags, EF_DEATH_BURST_DONE, true); }
        u16 sidx = i; if (!(World.instances[i].entflags & EF_ACT_AS_CORPSE_ONLY) && !(World.instances[i].entflags & EF_TELEPORT_ON_DEATH)) { int sded=sfxDeath[World.instances[i].index - 419]; if (sded >= 0 && sded < (i16)SOUNDS_COUNT){play_wav(sounds[sded],SfxVol(),World.position[sidx],true);} } World.gravity[i] = ai_is_cyber(&World.instances[i]) ? 0.0f : 1.0f; // Physics for death
        flag_set(&World.instances[i].entflags,EF_ASLEEP,false); World.layer[i] = L_Corpse; flag_set(&World.instances[i].entflags,EF_FIRST_SIGHTING,true); World.instances[i].timeTillDeadFinished = World.pauseRelativeTime + npc->timeTillDead; //if (npc->switchMaterialOnDeath && World.instances[i].dyingTexture) World.instances[i].texIndex = World.instances[i].dyingTexture; // TODO Handle hopper and zerog texture changes
        if (World.instances[i].index == 428 || World.instances[i].index == 439) World.velocity[sidx] = (V3){0.0f,World.velocity[sidx].y,0.0f}; // Prevent gibs on Exec bot or fake melt on Zero-G mutant from having horizontal movement (looks nicer).
        if (World.instances[i].index == 433) World.layer[i] = L_Corpse; // Hopper: enable capsule collider (implicit in layer change)
        flag_set(&World.instances[i].entflags, EF_DYING_SETUP, true);
    }
    if (World.instances[i].timeTillDeadFinished < World.pauseRelativeTime) { flag_set(&World.instances[i].entflags,EF_DEAD,true); flag_set(&World.instances[i].entflags,EF_DYING,false); World.instances[i].currentState = AIState_Dead; } if (AIDeactivatesVisibleMeshWhileDying(&World.instances[i])) World.instances[i].modelIndex = MAX_MDLS; if (World.instances[i].index == 439) World.layer[i] = L_Corpse | L_CorpseSearchable; // Zero-G mutant enables search collider while still dying
}

static void AIDead(u16 idx) {
    Entity* self = &World.instances[idx]; flag_set(&World.instances[idx].entflags,EF_ASLEEP,false); flag_set(&World.instances[idx].entflags,EF_DEAD,true); flag_set(&World.instances[idx].entflags,EF_DYING,false); flag_set(&World.instances[idx].entflags,EF_DYING_SETUP,false); if (World.instances[idx].entflags & EF_DEAD_CHECKS_DONE){return;}
    if (AIDeactivatesVisibleMeshWhileDying(self)) World.instances[idx].modelIndex = MAX_MDLS; World.instances[idx].currentState = AIState_Dead; World.layer[idx] = L_Corpse; if (World.instances[idx].entflags & EF_TELEPORT_ON_DEATH) { World.gravity[idx] = 1.0f; World.instances[idx].modelIndex = MAX_MDLS; DeleteInstance(idx); /* TeleportAway not yet fully implemented; keep delete for now */}
    else if (ai_is_cyber(self)) { World.gravity[idx] = 0.0f; World.instances[idx].modelIndex = MAX_MDLS; DeleteInstance(idx); /* Gib effect: spawn basic debris using deathBurst index if defined */ } else { /*Enable search collider for non-gib corpses (Avian Mutant index 2 always searchable)*/ World.layer[idx] = L_Corpse | L_CorpseSearchable; World.velocity[idx].x = 0.0f; World.velocity[idx].z = 0.0f; if (World.instances[idx].index != 433) World.gravity[idx] = 1.0f;/*Hopper deactivates itself*/ }
    flag_set(&World.instances[idx].entflags, EF_DEAD_CHECKS_DONE, true);
}

static DamageData SetNPCData(Entity* self, int n){DamageData dd={0}; NPCTable* npc=&npcTable[self->index - 419]; dd.owner=(u16)(self - World.instances); switch(n){case 1:dd.damage=npc->damage; dd.attackType=npc->attackType; break; case 2:dd.damage=npc->damage2; dd.attackType=npc->attackType2; break; default:dd.damage=npc->damage3; dd.attackType=npc->attackType3; break;} dd.penetration=0; dd.defense=0; return dd;}
static void ai_apply_damage(DamageData dd, u16 hitIdx) { if(!hitIdx || hitIdx >= INSTANCE_COUNT){return;} dd.hitIdx=hitIdx; dd.damage*=(1.0f - (dd.defense / (dd.defense + dd.offense + 1.0f))); if(hitIdx == PLAYER1){PlayerTakeDamage(hitIdx, dd.damage);} else {Entity* t=&World.instances[hitIdx]; t->health-=dd.damage; if(t->health < 0.0f){t->health = 0.0f;} t->recentMostActivator=dd.owner; flag_set(&t->entflags,EF_GO_INTO_PAIN,true);} }
static void AIApplyAttackMovement(Entity* self, float speed) { u16 eidx=self->enemy; if(!eidx)return; if(self->entflags & EF_ACT_AS_TURRET){self->currentDestination=ai_sight_pos(self); return;} if(speed<=0||self->tranquilizeFinished>=World.pauseRelativeTime)return; self->currentDestination=World.position[eidx]; if(V3_SqDist(ai_sight_pos(self),self->currentDestination)<=AI_STOP_DIST_SQ)return; if(!AIWithinAngleToTarget(self))return; AddForce((u16)(self-World.instances),V3_ScaleByF(self->forward,speed),false); }
static void AITransitionAttackToRun(Entity* self, int n) {
    flag_set(&self->entflags,EF_GO_INTO_PAIN,false); self->currentState=AIState_Run; NPCTable* npc=&npcTable[self->index - 419]; float chance,wmin,wmax,*wait;
    switch (n) {
        case 1:  chance=npc->timeAttack1WaitChance; wmin=npc->timeAttack1WaitMin; wmax=npc->timeAttack1WaitMax; wait=&self->randWaitAtt1Finished; break; case 2:  chance=npc->timeAttack2WaitChance; wmin=npc->timeAttack2WaitMin; wmax=npc->timeAttack2WaitMax; wait=&self->randWaitAtt2Finished; break; default: chance=npc->timeAttack3WaitChance; wmin=npc->timeAttack3WaitMin; wmax=npc->timeAttack3WaitMax; wait=&self->randWaitAtt3Finished; break;
    } *wait = (random_range(0.0f, 1.0f) < chance) ? World.pauseRelativeTime + random_range(wmin, wmax) : World.pauseRelativeTime;
}

static void MuzzleBurst(Entity* self, int attackNum) { if (attackNum < 1 || attackNum > 3){attackNum=1;} static const int muzzleBurstIndices[4] = {370, 370, 370, 370}; int prefab = muzzleBurstIndices[attackNum]; if (prefab > 0) { u16 burst = SpawnDynamicObject(prefab, false); if (burst < INSTANCE_COUNT && burst != 0xFFFF) { World.position[burst] = ai_gun_pos(self,attackNum); } } }
static void ProjectileRaycast(Entity* self, int n) {
    if (n < 1 || n > 3){n = 1;} V3 spos = (n == 1) ? ai_sight_pos(self) : ai_gun_pos(self, n); u16 eidx = self->enemy; V3 targ = eidx ? self->targettingPosition : (V3){spos.x + self->forward.x*10.0f,spos.y,spos.z + self->forward.z*10.0f}; V3 dir=(n == 1) ? self->forward : V3_Normalize(V3_AsubB(targ,spos)); float range;
    switch (n) { case 1: range = npcTable[self->index - 419].range; break; case 2: range = npcTable[self->index - 419].range2; break; default: range = npcTable[self->index - 419].range3; break; }
    MuzzleBurst(self,n); RaycastHit hit = Raycast(spos, dir, range, LMASK_NPC_ATTACK); if(!hit.hit){return;} u16 hi = hit.hitInstanceIndex;
    if (n == 3 && self->index == 427 && eidx) DrawLine(ai_sight_pos(self), World.position[eidx],(Color){1.0f,0.15f,0.18f,0.85f}); // Targeting laser (Cyborg Elite, attack3)
    DamageData dd = SetNPCData(self,n); dd.hitpoint=hit.point; dd.attacknormal=dir; dd.impactVelocity=dd.damage; bool hitPlayer=(hi == PLAYER1); if(hitPlayer){dd.impactVelocity *= 0.5f;} dd.isOtherNPC=!hitPlayer && IdxIsNPC(World.instances[hi].index);
    if (hi){ai_apply_damage(dd,hi);} u16 impactCI = GetImpactType(hi); if(impactCI){u16 imp = SpawnDynamicObject(impactCI,true); if(imp && imp < INSTANCE_COUNT){World.position[imp]=hit.point;}}
}

static void ProjectileLaunched(Entity* self, int n) {
    u16 sidx=(u16)(self - World.instances); NPCTable* npc = &npcTable[self->index - 419]; int masterIdx; float launchSpd; switch (n) { case 1: masterIdx = npc->projectile1Prefab; launchSpd = npc->projectileSpeedAttack1; break; case 2: masterIdx = npc->projectile2Prefab; launchSpd = npc->projectileSpeedAttack2; break; default: masterIdx = npc->projectile3Prefab; launchSpd = npc->projectileSpeedAttack3; break; }
    V3 spos=ai_gun_pos(self,n); u16 eidx=self->enemy; V3 targ=eidx ? self->targettingPosition : (V3){spos.x + self->forward.x*20.0f,spos.y,spos.z + self->forward.z*20.0f}; V3 dir=V3_Normalize(V3_AsubB(targ,spos)); MuzzleBurst(self,n); u16 bb = SpawnDynamicObject((u16)masterIdx,false); if (!bb || bb >= INSTANCE_COUNT) bb = SpawnDynamicObject(370,false); // TODO validate in arg without double calling SpawnDynamicObject
    if (!bb || bb >= INSTANCE_COUNT) return; Entity* proj=&World.instances[bb]; World.layer[bb]=L_NPCBullet; World.position[bb]=spos; proj->forward=dir; // TODO: store damage data into projectile entity fields for deferred impact
    V3 shove = V3_ScaleByF(dir, launchSpd); if (vabs(World.gravity[sidx]) > 0.05f) { shove.x += World.velocity[sidx].x; shove.z += World.velocity[sidx].z; } World.velocity[bb] = (V3){0,0,0}; AddForce(bb,shove,true); flag_set(&proj->entflags,EF_ACTIVE | EF_RIGIDBODY,true);
}

static void AIExplodeAttack(Entity* self) {
    float radius = npcTable[self->index - 419].attack3Radius; float force=npcTable[self->index - 419].attack3Force; V3 epos = ai_sight_pos(self); DamageData dd = SetNPCData(self, 3);
    for (u16 i = INSTS_1ST_IDX; i < World.instCount; ++i) { Entity* t = &World.instances[i]; if (!(t->entflags & EF_ACTIVE)) continue; float dsq = V3_SqDist(epos,World.position[i]); if (dsq >= radius * radius) continue; float dist = vsqrtf(dsq), falloff = 1.0f - dist / radius; DamageData tdd = dd; tdd.damage *= falloff; ai_apply_damage(tdd, i); if (dist > 0.001f) AddForce(i,V3_ScaleByF(V3_Normalize(V3_AsubB(World.position[i],epos)),force * falloff),true); } self->health = 0.0f; // Self-destruct
}

static void AIMakeAttack(Entity* self, AttType att, int ind) { if (ind < 1 || ind > 3){ind=1;/*Melee hitscan by default.*/} switch (att) { case Att_Melee:ProjectileRaycast(self,ind); break; case Att_HitS:ProjectileRaycast(self,ind); World.fogFac += 1; break; case Att_Ball:ProjectileLaunched(self,ind); World.fogFac += 1; break; default: break; } }
void AIAttack(Entity* self, int slot) {
    u16 sidx = (u16)(self - World.instances); NPCTable* npc = &npcTable[self->index - 419]; if (slot == 3 && npc->explodeOnAttack3) { World.fogFac += 5; AIExplodeAttack(self); return; } AIApplyAttackMovement(self, slot == 1 ? npc->attack1Speed : slot == 2 ? npc->attack2Speed : npc->attack3Speed); int sat = slot == 1 ? sfxAttack1[self->index - 419] : slot == 2 ? sfxAttack2[self->index - 419] : sfxAttack3[self->index - 419];
    float* s_time = slot == 1 ? &self->attack1SoundTime : (slot == 2 ? &self->attack2SoundTime : &self->attack3SoundTime); u32 tb = slot == 1 ? npc->timeBetweenAttack1 : slot == 2 ? npc->timeBetweenAttack2 : npc->timeBetweenAttack3;
    (self->gracePeriodFinished < World.pauseRelativeTime && !(self->entflags & EF_SHOT_FIRED)) ? (flag_set(&self->entflags,EF_SHOT_FIRED,true),(*s_time < World.pauseRelativeTime && sat >= 0 && sat < (i16)SOUNDS_COUNT) ? (play_wav(sounds[sat],SfxVol(),World.position[sidx],true), *s_time=World.pauseRelativeTime + tb) : 0,AIMakeAttack(self,slot == 1 ? npc->attackType : slot == 2 ? npc->attackType2 : npc->attackType3,slot)) : 0;
    (slot == 3 && self->enemy) ? (self->index == 427 ? DrawLine(ai_sight_pos(self),World.position[self->enemy],(Color){1.0f, 0.15f, 0.18f, 0.85f}) : self->index == 433 ? DrawLine(ai_sight_pos(self),World.position[self->enemy],(Color){0.96f,1.0f,0.0f,0.88f}) : (void)0) : (void)0; if (self->attackFinished < World.pauseRelativeTime) AITransitionAttackToRun(self,slot);
}

static void AIFlierMoveToHoverHeight(Entity* self) {
    u16 sidx=(u16)(self - World.instances); NPCTable* npc = &npcTable[self->index - 419]; if (npc->runSpeed <= 0.0f) return; u16 eidx = self->enemy;
    if (eidx) { self->idealPos.y = World.position[eidx].y + AI_TARGET_OFFSET_Y; self->idealPos.x=World.position[sidx].x; self->idealPos.z=World.position[sidx].z; }
    else { V3 sp=ai_sight_pos(self); RaycastHit dn=Raycast(sp,(V3){0,-1,0},npc->sightRange,LMASK_NPC_SIGHT); RaycastHit up=Raycast(sp,(V3){0,1,0},npc->sightRange,LMASK_NPC_SIGHT); float dDn=dn.hit ? dn.distance : 0.0f, dUp=up.hit ? up.distance : 0.0f; float yH=npc->flightHeight * (npc->flightHeightIsPercentage ? dDn + dUp : 1.0f); V3 fp=dn.hit ? dn.point : World.position[sidx]; self->idealPos=(V3){fp.x,fp.y+yH,fp.z};}
    float dy = self->idealPos.y - World.position[sidx].y; if (vabs(dy) < 0.16f) return; float spd  = npc->runSpeed * (float)World.deltaTime; float step = vmin(vabs(dy), spd) * (dy < 0.0f ? -1.0f : 1.0f); World.position[sidx].y += step;
}

float AITranquilize(u16 idx, float amount, bool energy) { Entity* self = &World.instances[idx]; float secs = (amount < 3.0f) ? (float)npcTable[self->index - 419].timeForTranquilization : amount; if (npcTable[self->index - 419].type != NPCType_Robot || energy) { double a = World.pauseRelativeTime + secs, b = self->tranquilizeFinished + secs; self->tranquilizeFinished = a > b ? a : b; return secs; } return 0.0f; }
void AIAlert(u16 idx) { if (!World.diffCbt){return;} Entity* self = &World.instances[idx]; AISetEnemy(idx,PLAYER1); self->currentDestination = World.position[PLAYER1]; flag_set(&self->entflags, EF_ENEM_IN_SIGHT, false); }
void AIAwakeFromSleep(u16 idx) { flag_set(&World.instances[idx].entflags,EF_ASLEEP,false); AIAlert(idx);/*deactivate sleeping cables*/ for(u16 j=INSTS_1ST_IDX;j<World.instCount;++j){ if(j == idx){continue;} if(World.instances[j].entflags & EF_ASLEEP && World.instances[j].enemy == idx){flag_set(&World.instances[j].entflags,EF_ACTIVE,true);} }  }
static void AIThink(u16 idx) {
    Entity* self = &World.instances[idx]; if ((self->entflags & EF_DYING_SETUP) && self->deathBurstFinished < World.pauseRelativeTime && !(self->entflags & EF_DEATH_BURST_DONE)) { if (self->deathBurst > 0) { SpawnDynamicObject(self->deathBurst, false); } flag_set(&self->entflags,EF_DEATH_BURST_DONE,true); }
    if (!ai_has_health(self)) { if (!(self->entflags & EF_DYING) && !(self->entflags & EF_DEAD)){flag_set(&self->entflags,EF_DYING,true); self->currentState=AIState_Dying;}else if((self->entflags & EF_DEAD) && self->currentState != AIState_Dead){self->currentState=AIState_Dead;}else if((self->entflags & EF_DYING) && self->currentState != AIState_Dying){self->currentState=AIState_Dying;} }
    switch (self->currentState) { case AIState_Idle:AIIdle(idx); break; case AIState_Walk:AIWalk(idx); break; case AIState_Run:AIRun(idx); break; case AIState_Attack1:AIAttack(self,1); break; case AIState_Attack2:AIAttack(self,2); break; case AIState_Attack3:AIAttack(self,3); break; case AIState_Pain:AIPain(self); break; case AIState_Dying:AIDying(idx); break; case AIState_Dead:AIDead(idx); break; default:AIIdle(idx); break; }
    if (self->currentState == AIState_Dead || self->currentState == AIState_Dying) return;
}

void AIControllerUpdate(u16 idx) {
    if(!(World.instances[idx].entflags & EF_ACTIVE)){return;} u16 edx=World.instances[idx].index; if(!IdxIsNPC(edx)){return;} u16 ndx=edx-419; if(npcTable[ndx].type != NPCType_Cyber && npcTable[ndx].moveType != AIMoveType_Fly && World.instances[idx].currentState != AIState_Dead && World.instances[idx].currentState != AIState_Dying) World.gravity[idx] = 1.0f;
    flag_set(&World.instances[idx].entflags,EF_ENEM_IN_SIGHT,AICheckIfPlayerInSight(idx)); u16 eidx = World.instances[idx].enemy;
    if (eidx && ai_has_health(&World.instances[idx])) {
        bool enAlive = npcTable[ndx].type == NPCType_Cyber ? World.instances[eidx].cyberHealth > 0.0f : World.instances[eidx].health > 0.0f;
        if (!enAlive) { if (npcTable[ndx].type == NPCType_Cyber) { World.instances[idx].currentState = AIState_Idle; } else { flag_set(&World.instances[idx].entflags, EF_WANDERING, true); World.instances[idx].wanderFinished = World.pauseRelativeTime + random_range(3.0f, 8.0f); World.instances[idx].currentState = AIState_Walk; } World.instances[idx].enemy = 0; World.instances[idx].posCheckFinished = World.pauseRelativeTime; World.instances[idx].lastPosition = World.position[idx]; }
        else AIEnemyInFrontChecks(&World.instances[idx],eidx);
    }
    AIThink(idx);
    if (World.instances[idx].currentState != AIState_Dead && World.instances[idx].currentState != AIState_Idle) {
        if ((World.instances[idx].entflags & EF_ACT_AS_TURRET) && eidx) { World.instances[idx].currentDestination = (V3){World.position[eidx].x,World.position[eidx].y + AI_TARGET_OFFSET_Y,World.position[eidx].z}; } if (npcTable[ndx].type == NPCType_Cyber && eidx) World.instances[idx].currentDestination = World.position[eidx]; V3 toTarget = V3_AsubB(World.instances[idx].currentDestination,ai_sight_pos(&World.instances[idx]));
        if (npcTable[ndx].type != NPCType_Cyber) toTarget.y = 0.0f; World.instances[idx].idealTransformForward = V3_Normalize(toTarget); float sqmag = V3_dot(toTarget, toTarget); if (sqmag > 1e-6f || npcTable[ndx].type == NPCType_Cyber) AIFace(&World.instances[idx],World.instances[idx].currentDestination);
    } if (npcTable[ndx].moveType == AIMoveType_Fly && World.instances[idx].tranquilizeFinished < World.pauseRelativeTime) AIFlierMoveToHoverHeight(&World.instances[idx]);
}

float TakeDamage(u16 self,DamageData dd) {
    if (Cheats.god && self == PLAYER1) return 0.0f;
    bool isCyber = IsCyberEntity(self); float* hp = isCyber ? &World.instances[self].cyberHealth : &World.instances[self].health; u16 selfIdx = World.instances[self].index; bool isNPC = IdxIsNPC(selfIdx), isPlayer = (self == PLAYER1); bool isGrenade = IsGrenade(selfIdx);
    if (isCyber) { if (dd.attackType == Att_Drill && isNPC){return 0.0f;} if (dd.attackType != Att_Drill && World.instances[self].iceActive){return 0.0f;} } if (*hp <= 0.0f) { bool allowPost = (isNPC || World.instances[self].iceActive || isPlayer || isGrenade || selfIdx == 279/*chunk_screen*/ || selfIdx == 477/*sec_camera*/); if (!allowPost) return 0.0f; } float take = dd.damage;
    if (isPlayer) {
        float absorb = 0.0f;
        if (isCyber) { if (World.invP1.hasSoft & (1 << SW_SHIELD)) { u8 sv = World.invP1.softVersions[SW_SHIELD]; absorb = (sv <= 9) ? sv * 0.05f : 0.0f; take *= (1.0f - absorb); if (take <= 0.0f){return 0.0f;} } }// Cyber C-Shield software absorption
        else {
            if (dd.attackType == Att_Magn) {take = 0.0f; TakeEnergy(11.0f); World.empStaticAlpha=2.0f; BiomonitorEnergyPulse(11.0f);}
            if ((World.invP1.hardwareIsActive & HW_SHD) && (World.invP1.hasHardware & HW_SHD)) {
                float thresh = 0.0f; switch (World.invP1.hwVers[HW_SHD_IDX]) { case 0:absorb=0.20f; thresh=0.0f; break; case 1:absorb=0.40f; thresh=10.0f; break; case 2:absorb=0.75f; thresh=15.0f; break; case 3:absorb=0.75f; thresh=30.0f; break; }
                if (take < thresh) absorb = 1.0f; if (absorb > 0.0f) { if (absorb < 1.0f) absorb = vclamp(absorb + random_range(-0.08f,0.08f),0.0f,1.0f); take *= (1.0f - absorb); play_wav(sounds[94],Sys_Settings.VolumeEffects,(V3){0.0f,0.0f,0.0f},false);/*shield absorb*/ int abs = (int)(absorb * 100.0f); CenterStatusPrint("%s%d%s",Sys_Text.stringTable[208],abs,Sys_Text.stringTable[209]); }
            } if (take > 0.0f && (absorb < 0.4f || random_range(0.0f,1.0f) < 0.5f)) { play_wav(sounds[140],Sys_Settings.VolumeEffects,(V3){0.0f,0.0f,0.0f},false); World.painStaticAlpha = take > 15.0f ? 1.0f : take > 10.0f ? 0.8f : 0.3f; }
        }
    }
    if (isCyber) { World.instances[self].cyberHealth -= take; if (isPlayer) { World.damageReceived += take; if (World.instances[self].cyberHealth <= 0.0f) { ExitCyberspace(); return 0.0f; } } if (dd.owner == PLAYER1){World.damageDealt += take;} }
    else { if(selfIdx == 477/*Camera constIndex 477 gets one-shot by tranq*/ && dd.attackType == Att_Trnq){take=World.instances[self].health + 1.0f;} take=ApplyAttTypeAdjustments(self,take,dd.attackType); World.instances[self].health-=take; if (isPlayer) { World.damageReceived+=take; World.Sys_Music.inCombat=true; } if (dd.owner == PLAYER1){World.damageDealt+=take;} }
    if (isNPC && (World.instances[self].health > 0.0f || (isCyber && World.instances[self].cyberHealth > 0.0f))) { if (npcTable[selfIdx - 419].timeBetweenPain > 0.0f) flag_set(&World.instances[self].entflags,EF_GO_INTO_PAIN,true); World.instances[self].recentMostActivator = dd.owner; TargetIDSendDamageReceive(self,take,dd.attackType); AICheckPain(self); }
    if (isCyber) { if (World.instances[self].cyberHealth <= 0.0f) { if (!World.instances[self].iceActive && isNPC) {World.cyberkills++;} Death(self,false); } } else { if (World.instances[self].health <= 0.0f) { if (isNPC) {World.kills++;} Death(self,dd.attackType == Att_Beam); } }    return take;
}

void HealthManagerInitAfterLoad(u16 self) {
    if (self == PLAYER1) { World.instances[self].health=211.0f; World.instances[self].cyberHealth=255.0f; World.invP1.noiseFinished = World.pauseRelativeTime - 31.0;/*guarantee no combat music on start*/ return; }
    if (IdxIsNPC(World.instances[self].index)) {
        if (IsCyberEntity(self)) { if (World.instances[self].cyberHealth < 0.0f) World.instances[self].cyberHealth = npcTable[World.instances[self].index - 419].healthForCyberNPC; } else { if (World.instances[self].health < 0.0f) World.instances[self].health = npcTable[World.instances[self].index - 419].health; }
        if (World.diffCbt == 0) { World.instances[self].health = 1.0f; } if (World.instances[self].entflags & EF_ACT_AS_CORPSE_ONLY) { World.instances[self].health = 0.0f; World.instances[self].cyberHealth = 0.0f; UseDeathTargets(self); if (World.instances[self].entflags & EF_TELEPORT_ON_DEATH){TeleportAway(self);}else{NPCDeath(self);} }
    }
}
// Hardware
static Color3 lantCol = (Color3){1.0f,1.0f,1.0f}; static float lanternVersionBrightness[3] = {0.875f,1.4f,1.75f};
void HardwareUpdate() {
    bool infraredOn = (World.invP1.hasHardware & HW_INF) && (World.invP1.hardwareIsActive & HW_INF) > 0, lanternOn = (World.invP1.hasHardware & HW_LAN) && (World.invP1.hardwareIsActive & HW_LAN) > 0;
    if (lanternOn || infraredOn) { // Update headmounted lantern/infrared's light (infrared overrides lantern brightness/range)
        V3 ppos = World.position[PLAYER1]; lanternPos = (V3){ppos.x + 0.04f,ppos.y + 0.24f,ppos.z + 0.04f}; float intensity = infraredOn ? 0.8f : lanternVersionBrightness[vclamp(World.invP1.hwVersSetting[7],0,2)]; UpdateLight(headmountedLanternLight,lanternPos,lantCol,infraredOn ? 50.35f : 11.52f,intensity,intensity,0.0f,0.0f,QUAT_IDENTITY,true,true);
    } else UpdateLight(headmountedLanternLight,lanternPos,lantCol,11.52f,0.0f,0.0f,0.0f,0.0f,QUAT_IDENTITY,false,false);
}
// Dermal Patches
void PatchDisableAll(void){World.invP1.berserkFinished=World.invP1.berserkIncTime=World.invP1.detoxFinished=World.invP1.geniusFinished=World.invP1.mediFinished=World.invP1.reflexFinishedTime=World.invP1.sightFinishedTime=World.invP1.sightSideEffectFinishedTime=World.invP1.staminupFinishedTime=-1.0; World.invP1.staminupActive=World.geniusActive=false; World.invP1.fatigue=0.0f; World.invP1.berserkIncrement=World.invP1.patchActive=0; World.timeScale=DEFAULT_TIME_SCALE;}
void PatchUpdate() {
    if (World.invP1.patchActive & PATCH_DETOX) { if (World.invP1.detoxFinished < World.pauseRelativeTime) World.invP1.patchActive -= PATCH_DETOX; } // Detox
    if (World.invP1.patchActive & PATCH_MEDI) { if (World.invP1.mediFinished < World.pauseRelativeTime && World.invP1.mediFinished != -1.0) { World.invP1.patchActive -= PATCH_MEDI; World.invP1.mediFinished = -1.0; } } // Medi
    if (World.invP1.patchActive & PATCH_REFLEX) { if (World.invP1.reflexFinishedTime < World.absoluteTime && World.invP1.reflexFinishedTime != -1.0){ World.invP1.patchActive-=PATCH_REFLEX; World.invP1.reflexFinishedTime=-1.0; World.timeScale=DEFAULT_TIME_SCALE;}else{World.timeScale=REFLEX_TIME_SCALE;}}else{if(World.timeScale != DEFAULT_TIME_SCALE){World.timeScale=DEFAULT_TIME_SCALE;}}//Reflex
    if (World.invP1.patchActive & PATCH_BERSERK) { // Berserk
        if (World.invP1.berserkFinished < World.pauseRelativeTime) { World.invP1.berserkIncrement = 0; World.invP1.patchActive -= PATCH_BERSERK; }
        else if (World.invP1.berserkIncTime < World.pauseRelativeTime) { World.invP1.berserkIncrement++; if (World.invP1.berserkIncrement > 6) World.invP1.berserkIncrement = 6; World.invP1.berserkIncTime = World.pauseRelativeTime + (BERSERK_TIME / 5.0f); }
    }
    if (World.invP1.patchActive & PATCH_GENIUS) { if(World.invP1.geniusFinished < World.pauseRelativeTime){World.invP1.patchActive -= PATCH_GENIUS; World.geniusActive=false;}else{World.geniusActive=true;} } // Genius
    if (World.invP1.patchActive & PATCH_SIGHT) { // Sight
        if (World.invP1.sightFinishedTime < World.pauseRelativeTime && World.invP1.sightFinishedTime != -1.0) { World.invP1.sightFinishedTime=-1.0; World.invP1.sightSideEffectFinishedTime = World.pauseRelativeTime + SIGHT_SIDE_EFFECT_TIME; }
        if (World.invP1.sightSideEffectFinishedTime < World.pauseRelativeTime && World.invP1.sightSideEffectFinishedTime != -1.0) { World.invP1.sightSideEffectFinishedTime=World.invP1.sightFinishedTime=-1.0; World.invP1.patchActive -= PATCH_SIGHT; }
    }
    if (World.invP1.patchActive & PATCH_STAMINUP) { if (World.invP1.staminupFinishedTime < World.pauseRelativeTime) { World.invP1.staminupActive=false; World.invP1.fatigue=100.0f; World.invP1.patchActive -= PATCH_STAMINUP; } else { World.invP1.fatigue = 0.0f; World.invP1.staminupActive = true; } } // Staminup
}
// Quest Bits / Mission I/O — side effects on quest notes checklist when bits change
static void QuestBitNoteSideEffects(u8 qb, bool isOn) {
    if (isOn) {
        switch (qb) {
            case QB_ShieldActivated:       World.questNotesActive[8] = true;  World.questNotesChecked[8] = true;  break;                                                                       case QB_LaserSafetyOverriden:   World.questNotesActive[7] = true;  World.questNotesChecked[7] = true;  break;
            case QB_LaserDestroyed:         World.questNotesActive[9] = true;  World.questNotesChecked[9] = true;  if (autoSplitter.missionSplitID == 1) autoSplitter.missionSplitID++; break; case QB_BetaGroveCyberUnlocked: World.questNotesActive[12] = true; break;
            case QB_GroveAlphaJettisonEnabled: World.questNotesActive[12] = true; break;                                                                                                       case QB_GroveBetaJettisonEnabled:  World.questNotesActive[12] = true; break;
            case QB_GroveDeltaJettisonEnabled: World.questNotesActive[12] = true; break;                                                                                                       case QB_MasterJettisonBroken:   World.questNotesActive[12] = true; World.questNotesActive[11] = true; if (autoSplitter.missionSplitID == 2) autoSplitter.missionSplitID++; break;
            case QB_Relay428Fixed:          World.questNotesActive[11] = true; World.questNotesChecked[11] = true; break;                                                                      case QB_MasterJettisonEnabled:  World.questNotesActive[10] = true; World.questNotesChecked[10] = true; if (autoSplitter.missionSplitID == 3) autoSplitter.missionSplitID++; break;
            case QB_BetaGroveJettisoned:    World.questNotesActive[12] = true; World.questNotesChecked[12] = true; World.questNotesActive[13] = true; if (autoSplitter.missionSplitID == 4) autoSplitter.missionSplitID++; break;
            case QB_AntennaNorthDestroyed: case QB_AntennaSouthDestroyed: case QB_AntennaEastDestroyed: case QB_AntennaWestDestroyed:   World.questNotesActive[13] = true; break;              case QB_SelfDestructActivated:  for (int i=0;i<17;++i) World.questNotesActive[i] = true; World.questNotesChecked[14] = true; break;
            case QB_BridgeSeparated:        for (int i=0;i<17;++i) World.questNotesActive[i] = true; World.questNotesActive[17] = true; World.questNotesChecked[16] = true; break;             default: break;
        }
    } else {
        switch (qb) {
            case QB_ShieldActivated:       World.questNotesChecked[8] = false; break;   case QB_LaserSafetyOverriden:   World.questNotesChecked[7] = false; break;  case QB_LaserDestroyed:         World.questNotesChecked[9] = false; break;  case QB_Relay428Fixed:          World.questNotesChecked[11] = false; break;
            case QB_MasterJettisonEnabled:  World.questNotesChecked[10] = false; break; case QB_BetaGroveJettisoned:    World.questNotesChecked[12] = false; break; case QB_SelfDestructActivated:  World.questNotesChecked[14] = false; break; case QB_BridgeSeparated:        World.questNotesChecked[16] = false; break; default: break;
        }
    }
}
// Ressurection: when player dies on a level with resurrection active, teleport back to the ressurection point instead of counting a death.
bool RessurectPlayer(void) { if(!((World.ressurectionActiveLevels >> World.curLev) & 1u)){return false;} if (World.curLev == 10 || World.curLev == 11 || World.curLev == 12) LoadLevel(6, ressurectionLocations[6]); else if (World.curLev < 13) World.position[PLAYER1] = ressurectionLocations[World.curLev]; PlayTrack(TT_Revive, MT_Override); World.invP1.ressurectingFinished = World.pauseRelativeTime + 3.0; CenterStatusPrint("BRAIN ACTIVITY SATISFACTORY..."); return true; }
// Doors
static bool DoorInventoryHasAccessCard(AccCardType card) { return card == ACC_None || (World.invP1.accessCardOwned & (1u << card)); }
static float DoorGetProgress(const Entity* e, u8 clip) { AnimationClip c = DoorGetClip(e,clip); if(c.frameEnd <= c.frameStart){return 1.0f;} return DoorClamp01((float)(e->frame - c.frameStart) / (float)(c.frameEnd - c.frameStart)); } 
static void DoorOpen(u16 self) { Entity* e = &World.instances[self]; ChangeAnim(e,A_OPENING); e->doorOpen = e->doorState = DoorState_Opening; e->waitBeforeClose = World.pauseRelativeTime + e->delay; if (e->SFXIndex > 0 && e->SFXIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXIndex],1.0f,World.position[self],true); }
static void DoorClose(u16 self) { Entity* e = &World.instances[self]; ChangeAnim(e,A_CLOSING); e->doorOpen = e->doorState = DoorState_Closing; if (e->SFXIndex > 0 && e->SFXIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXIndex],1.0f,World.position[self],true); }
void DoorForceOpen(u16 self) { World.instances[self].requiredAccessCard = ACC_None; EntitySetLocked(&World.instances[self],false); DoorOpen(self); }
void DoorForceClose(u16 self) { if (World.instances[self].doorOpen == DoorState_Closed) {return;} DoorClose(self); }
void DoorActuate(u16 self) {
    Entity* e = &World.instances[self]; if (e->doorOpen == DoorState_Open) { DoorClose(self); return; } if (e->doorOpen == DoorState_Closed) { DoorOpen(self); return; } bool op = e->doorOpen == DoorState_Opening;
    if (op || e->doorOpen == DoorState_Closing) {
        int src = op ? A_OPENING : A_CLOSING, dst = op ? A_CLOSING : A_OPENING; AnimationClip dstClip = DoorGetClip(e,dst); u16 newFrm = DoorFrameFromProgress(dstClip,1.0f - DoorGetProgress(e,src)); // Direct frame assignment (mid-anim reversal): clip + frame + matching model.
        e->clip = dst; e->frame = newFrm; e->currentFrameFinished = 0.0; e->modelIndex = dstClip.frameStartModelIndex + (u16)(newFrm - dstClip.frameStart); e->doorOpen = e->doorState = op ? DoorState_Closing : DoorState_Opening;
        if (!op) e->waitBeforeClose = World.pauseRelativeTime + e->delay; if (e->SFXIndex >= 0 && e->SFXIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXIndex], 1.0f, World.position[self], true);
    }
}

void DoorUse(u16 self, u16 activator) {
    if (activator == WORLD) return; Entity* e = &World.instances[self]; if (GetCurrentLevelSecurity() > e->securityThreshold) { UIBlockedBySecurity(World.position[self]); return; } if (Cheats.superoverride || World.diffMis <= 0) { EntitySetLocked(e,false); e->requiredAccessCard = ACC_None; }
    if (World.diffMis <= 1) { e->requiredAccessCard = ACC_None; } if (e->useFinished >= World.pauseRelativeTime) return; e->useFinished = World.pauseRelativeTime + 0.15f;
    if (e->requiredAccessCard != ACC_None) { if (!DoorInventoryHasAccessCard(e->requiredAccessCard)) { CenterStatusPrint("%s",Sys_Text.stringTable[2]); if (e->SFXLockedIndex >= 0 && e->SFXLockedIndex < SOUNDS_COUNT) {play_wav(sounds[e->SFXLockedIndex],0.7f,World.position[self],true);} return; } else e->requiredAccessCard = ACC_None; }
    if ((e->entflags & EF_LOCKED) != 0) { CenterStatusPrint("%s",Sys_Text.stringTable[e->lockedMessageLingdex]); if (e->SFXLockedIndex >= 0 && e->SFXLockedIndex < SOUNDS_COUNT) {play_wav(sounds[e->SFXLockedIndex],0.55f,World.position[self],true);} return; }  if ((e->onlyTargetOnce && !e->targetAlreadyDone) || !e->onlyTargetOnce) { e->targetAlreadyDone = true; UseTargets(self,e->targetIdx); } if (e->ajar) e->ajar = false; DoorActuate(self);
}

void DoorTargetted(u16 self, u16 activator) { if ((World.instances[self].entflags & EF_LOCKED) != 0) EntitySetLocked(&World.instances[self],false); if (!World.instances[self].targettingOnlyUnlocks) DoorUse(self,activator); }
void DoorUpdate(u16 self) {
    Entity* e = &World.instances[self]; if(e->ajar){return;} AnimationClip opening=DoorGetClip(e,A_OPENING), closing=DoorGetClip(e,A_CLOSING);
    if (e->doorOpen == DoorState_Opening && e->clip == A_OPENING && e->frame >= opening.frameEnd) { e->doorOpen = e->doorState = DoorState_Open; ChangeAnim(e,A_IDLE_OPEN); } else if (e->doorOpen == DoorState_Closing && e->clip == A_CLOSING && e->frame >= closing.frameEnd) { e->doorOpen = e->doorState = DoorState_Closed; ChangeAnim(e,A_IDLE_CLOSED); } if (World.pauseRelativeTime > e->waitBeforeClose && e->doorOpen == DoorState_Open && !e->stayOpen && !e->startOpen) DoorClose(self);
}

void CloseFullmap() {}
u16 SpawnDynamicObject(int val, bool cheat) {
    if (!IdxInBounds(val)) { DualLogError("Const index out of bounds: %u", val); return 0xFFFF; } if (cheat) DualLog("Cheat spawn constIndex %u, level: %u, from cheat: %u, name: ",val,World.curLev,cheat); if (IdxIsGeometry(val) && !Cheats.editMode) { CenterStatusPrint("Indices 0 to 306 (level chunks)\nnot possible when not on edit mode!"); return 0xFFFF; }
    if (World.instCount >= INSTANCE_COUNT) { DualLogError("Failed to spawn constIndex %u: instance table full (%u/%u)",val,World.instCount,INSTANCE_COUNT); return 0xFFFF; } u16 entityIndexInInstanceTable = AddInstance((u16)val, (V3){0.0f,0.0f,0.0f}); return entityIndexInInstanceTable;
}
// TargetIO: Full game cross-level target handling.  Iterates all loaded levels, temporarily swaps active pointers via SetLevelPointers(), finds matching targetname(s), and calls Targetted().  Activator from cur level. Recursion is safe via targetIOActive flag.
void TriggerTargetted(u16 self, u16 activator) { (void)self; (void)activator;/*TODO run this trigger entity's targets*/ }
bool QuestBitIsSet(u8 qb) { return (qb < QB_COUNT) && ((World.missionBits >> qb) & 1u); }
void QuestBitSet(u8 qb)    { if (qb < QB_COUNT && !QuestBitIsSet(qb)) { World.missionBits |=  (1u << qb); QuestBitNoteSideEffects(qb, true); } }
void QuestBitClear(u8 qb)  { if (qb < QB_COUNT &&  QuestBitIsSet(qb)) { World.missionBits &= ~(1u << qb); QuestBitNoteSideEffects(qb, false); } }
void QuestBitToggle(u8 qb) { if (qb < QB_COUNT) { World.missionBits ^=  (1u << qb); QuestBitNoteSideEffects(qb, QuestBitIsSet(qb)); } }
void Targetted(u16 activator, u16 self) {
    Entity* e = &World.instances[self]; u32 aioflags = World.targetIOActive ? World.targetIOActivatorIoflags : World.instances[activator].ioflags;
    if (e->index == 699) { if (!e->relayEnabled) return; if (e->relayOnceEver) { if (e->relayAlreadyDone) return; e->relayAlreadyDone = true; } u32 savedFlags = World.targetIOActivatorIoflags; World.targetIOActivatorIoflags = e->ioflags; UseTargets(activator,e->targetIdx); World.targetIOActivatorIoflags = savedFlags; return; }
    if (e->index == 700) {
        if (!(aioflags & TARG_IOFLAGS_BRANCH_FLIPONLY)) { if (e->relayEnabled && e->currentTargetIdx != IO_NONE) { u32 savedFlags = World.targetIOActivatorIoflags; World.targetIOActivatorIoflags = e->ioflags; UseTargets(activator,e->currentTargetIdx); World.targetIOActivatorIoflags = savedFlags; e->branchOnSecond = !e->branchOnSecond; e->currentTargetIdx = e->branchOnSecond ? e->target2Idx : e->targetIdx; } }
        if (aioflags & (TARG_IOFLAGS_BRANCH_FLIP | TARG_IOFLAGS_BRANCH_FLIPONLY)) { e->branchOnSecond = !e->branchOnSecond; e->currentTargetIdx = e->branchOnSecond ? e->target2Idx : e->targetIdx; }   return;
    }
    if (e->index == 710) { // info_mission: quest bit set/clear/toggle, or test-and-branch via target/targetIfFalse.  Mode comes from the activating targetIO bits, falling back to the info_mission's own line.
        if (e->questBitID == QB_None) return; u32 modeFlags = aioflags ? aioflags : e->ioflags; if(modeFlags & TARG_IOFLAGS_MISSION_BIT_TOGGLE){QuestBitToggle(e->questBitID); DualLog("info_mission toggled bit %u -> %u\n",e->questBitID,(unsigned)QuestBitIsSet(e->questBitID)); return; }
        if (modeFlags & TARG_IOFLAGS_MISSION_BIT_OFF){QuestBitClear(e->questBitID); return;} if(modeFlags & TARG_IOFLAGS_MISSION_BIT_ON){QuestBitSet(e->questBitID); return;} u8 tm = e->questTestMode; if (!tm && activator != WORLD && activator < World.instCount) tm = World.instances[activator].questTestMode;
        if (tm) { bool bitOn = QuestBitIsSet(e->questBitID); bool pass = (tm == 1) ? bitOn : !bitOn;/*1==testQuestBitIsOn, 2==testQuestBitIsOff*/ UseTargets(activator, pass ? e->targetIdx : e->targetIfFalseIdx); }   return;
    }
    if (e->index == 709) { CenterStatusPrint("%s", Sys_Text.stringTable[e->messageLingdex]); return; }/*info_message*/   if (e->index == 708) { World.gameFinished = true; return; }
    if (e->index == 707) { EmailTargetted(self); return; }/*info_email*/                                                 if (aioflags & TARG_IOFLAGS_TRIPTRIGGER) { if(e->index == 598 || e->index == 600){TriggerTargetted(self,activator);}else if(e->index == 594){TriggerCounterTargetted(self,activator);} }
    if (aioflags & TARG_IOFLAGS_UNLOCK) EntitySetLocked(e, false);                                                       if ((aioflags & TARG_IOFLAGS_LOCK) && IdxIsDoor(e->index)) EntitySetLocked(e, true);                                     if (IdxIsButtonSwitch(e->index)) ButtonSwitchUse(self,activator);
    if ((aioflags & TARG_IOFLAGS_DOOROPEN) && IdxIsDoor(e->index)) { DoorForceOpen(self); } else if ((aioflags & TARG_IOFLAGS_DOOROPENIFUNLOCKED) && IdxIsDoor(e->index) && (e->entflags & EF_LOCKED) == 0 && (e->requiredAccessCard == ACC_None || (World.invP1.accessCardOwned & (1u << e->requiredAccessCard)))) { DoorForceOpen(self); } else if ((aioflags & TARG_IOFLAGS_DOORCLOSE) && IdxIsDoor(e->index)) { DoorForceClose(self); } else if (IdxIsDoor(e->index)) { DoorTargetted(self, activator); }
    if (aioflags & TARG_IOFLAGS_FBRIDGE_ACTIVATE) ForceBridgeActivate(self, false); else if (aioflags & TARG_IOFLAGS_FBRIDGE_DEACTIVATE) ForceBridgeDeactivate(self, false); else if (aioflags & TARG_IOFLAGS_FBRIDGE_TOGGLE) ForceBridgeToggle(self);
    if (aioflags & TARG_IOFLAGS_GRAVLIFT_TOGGLE) World.instances[self].active=!World.instances[self].active;             if (aioflags & TARG_IOFLAGS_TEXTURE_CHG_TOGGLE) TextureChangerToggle(self);
    if (aioflags & TARG_IOFLAGS_FUNCWALL_MOVE) FuncWallTargetted(self);                                                  if (aioflags & TARG_IOFLAGS_SWITCH_LOCK_TOGGLE) EntitySetLocked(e, (e->entflags & EF_LOCKED) == 0);
    if (aioflags & TARG_IOFLAGS_INST_ACTIVATE) flag_set(&e->entflags, EF_ACTIVE, true); else if (aioflags & TARG_IOFLAGS_INST_DEACTIVATE) flag_set(&e->entflags, EF_ACTIVE, false); else if (aioflags & TARG_IOFLAGS_INST_TOGGLE) flag_set(&e->entflags, EF_ACTIVE, !(e->entflags & EF_ACTIVE));
}

extern char ioNames[MAX_IO_NAMES][TARG_STRLEN]; extern u16 ioNameCount;
void UseTargets(u16 activator, u16 targetIdx) {
    if(targetIdx == IO_NONE){return;} bool wasActive=World.targetIOActive, succeeded=false; u8 entryLevel = World.currentLevel;
    if (!wasActive) { World.targetIOActive = true; World.targetIOEntryLevel = entryLevel; World.targetIOActivatorIdx = activator; World.targetIOActivatorEntity = World.instances[activator]; World.targetIOActivatorIoflags = World.instances[activator].ioflags; }
    const char* targetname = (targetIdx < ioNameCount) ? ioNames[targetIdx] : ""; // For logging only; matching is u16 compare against the interned table.
    for (u8 lev = 0; lev < World.numLevels; ++lev) { if (World.currentLevel != lev) SetLevelPointers(lev); for (u16 i = INSTS_1ST_IDX; i < World.instCount; ++i) { if (World.instances[i].targetnameIdx != targetIdx) {continue;} Targetted(activator,i); succeeded=true; } }
    if (World.currentLevel != entryLevel) {SetLevelPointers(entryLevel);} if (!succeeded) {DualLogWarn("No target found: %s\n",targetname);} if (!wasActive) {World.targetIOActive=false;}
}
// Frob/Use
void SearchObject(int searchable) { World.Sys_UI.highlightStatus[MM_NOTES]=true; World.Sys_UI.highlightTickCount[MM_NOTES]=3; World.Sys_UI.tickFinished=World.pauseRelativeTime; if (World.instances[searchable].searchableInUse) { for (int i=0;i<4;i++) { if (World.instances[searchable].contents[i] >= 0) break;/*TODO re-frob should pull first found item out*/ } } else play_wav(sounds[91],0.75f,(V3){0.0f,0.0f,0.0f},false); }
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
        World.invP1.holdingObject = true; World.invP1.heldObjectIndex = ent->index; World.invP1.heldObjectCustIdx = ent->usableCustIdx; World.invP1.heldAmmo = ent->ammo; World.invP1.heldAmmo2 = ent->ammo2; World.invP1.heldObjectLoadedAlternate = ent->heldObjectLoadedAlternate;
        if (Sys_Settings.QuickItemPickup) { AddItemToInventory(ent->index,ent->usableCustIdx); ResetHeldItem(); }
        else { CenterStatusPrint("%s%s",Sys_Text.stringTable[World.invP1.heldObjectIndex - 307 + 326],Sys_Text.stringTable[319]); /* picked up.*/ ForceInventoryMode(); } // Inventory mode is turned on when picking something up
        DeleteInstance(i);
    } else { int t = UseNameTableIndex(ent->index); CenterStatusPrint("%s%s",Sys_Text.stringTable[29],t >= 0 ? Sys_Text.stringTable[t] : ""); }
}

#define FROB_DISTANCE 4.9f
INLINE V3 ScreenPointToRayOffset(V3 f,V3 r,float dx,float dy){float bx=World.inventoryMode?(float)World.cursorPos_x:683.0f,by=World.inventoryMode?(float)World.cursorPos_y:384.0f,t=vtan((float)Sys_Settings.FOV*0.5f*PI/180.0f),nx=((bx+dx)-683.0f)/384.0f,ny=-((by+dy)-384.0f)/384.0f;V3 v=V3_Normalize((V3){nx*t,ny*t,-1.0f}),ff=(V3){-f.x,-f.y,-f.z},up=V3_Normalize(V3_Cross(r,ff));return(V3){v.x*r.x+v.y*up.x+v.z*ff.x,v.x*r.y+v.y*up.y+v.z*ff.y,v.x*r.z+v.y*up.z+v.z*ff.z};}
INLINE bool FrobRayIsFrobable(RaycastHit h){if(!h.hit)return false;u16 i=h.hitInstanceIndex;if(i>=World.instCount)return false;u16 e=World.instances[i].index;return IdxIsUsableObject(e)||IdxIsSearchable(e)||IdxIsDoor(e)||IdxIsButtonSwitch(e)||IdxIsNPC(e);}
static void Frob(V3 p,V3 f,V3 r){if(World.uiIsBlocking||World.curLev==LEVEL_CYBERSPACE)return;if(World.Sys_UI.vmailActive){World.Sys_UI.vmailActive=0;return;}if(World.invP1.holdingObject){DropHeldItem();return;}float o=(float)Sys_Settings.ScreenHeight*0.02f;RaycastHit fh={0},bh={0};bool ok=false;V3 d0=ScreenPointToRayOffset(f,r,0,0);fh=Raycast(p,d0,FROB_DISTANCE,LMASK_PLAYER_FROB);bh=fh;ok=FrobRayIsFrobable(fh);float ox[8]={0,0,o,-o,o,-o,-o,o},oy[8]={-o,o,0,0,o,-o,o,-o};for(int i=0;i<8&&!ok;++i){V3 d=ScreenPointToRayOffset(f,r,ox[i],oy[i]);RaycastHit th=Raycast(p,d,FROB_DISTANCE,LMASK_PLAYER_FROB);if(FrobRayIsFrobable(th)){bh=th;ok=true;}}if(!ok)bh=fh;if(Cheats.showPhys){World.debugLine_start=p;World.debugLineFinished=World.pauseRelativeTime+3.0;V3 dbg=ok?ScreenPointToRayOffset(f,r,0,0):d0;RaycastHit dh=ok?bh:fh;World.debugLine_end=dh.hit?dh.point:(V3){dbg.x*FROB_DISTANCE+p.x,dbg.y*FROB_DISTANCE+p.y,dbg.z*FROB_DISTANCE+p.z};}if(!ok){if(fh.hit){u16 idx=fh.hitInstanceIndex;if(idx<World.instCount){u16 ei=World.instances[idx].index;if(IdxIsGeometry(ei)||IdxIsDoor(ei)||World.instances[idx].index>=595){int t=UseNameTableIndex(ei);CenterStatusPrint("%s%s",Sys_Text.stringTable[29],t>=0?Sys_Text.stringTable[t]:"");return;}}}CenterStatusPrint("%s",Sys_Text.stringTable[30]);}else UseEntity(bh.hitInstanceIndex);}
// Update
void WeaponsUpdate(); void TextureSequenceUpdate(u16 self); void AIAnimationControllerUpdate(u16 selfIdx); void AIControllerUpdate(u16 selfIdx); void DrawSphereWireframe(Color col, ShapeSphere s);
extern float sightPointHeights[NUM_AI_TYPES];
void DrawAIDebug(u16 i) {
    if ((!IdxIsNPC(World.instances[i].index)) || !Cheats.showNPC) return;
    World.layer[i] = L_NPC; World.layer[PLAYER1] = L_Player; Quaternion r = World.rotation[i]; float x=r.x,y=r.y,z=r.z,w=r.w; V3 fwd = V3_Normalize((V3){2.0f*(x*z + w*y), 0.0f, 1.0f - 2.0f*(x*x + y*y)}); u16 npcIdx = World.instances[i].index - 419;
    V3 sightPt = V3_AplusB(World.position[i],(V3){0.0f,sightPointHeights[npcIdx],0.0f}); DrawLine(sightPt,V3_AplusB(sightPt,V3_ScaleByF(fwd,0.6f)),(Color){1.0f,1.0f,0.0f,1.0f}); V3 enemPt = World.position[PLAYER1]; enemPt.y -= 0.24f;
    RaycastHit hit = Raycast(sightPt,V3_AsubB(enemPt,sightPt),20.0f,LMASK_NPC_SIGHT); if (hit.hit && hit.hitInstanceIndex == PLAYER1) { DrawLine(sightPt,hit.point,(Color){1.0f,0.0f,0.0f,1.0f}); } else {DrawLine(sightPt,enemPt,(Color){0.0f,1.0f,1.0f,1.0f});} Entity* e = &World.instances[i]; Color dbgCol;
    if (e->currentState == AIState_Idle) dbgCol = (Color){0.0f,1.0f,0.0f,1.0f};
    else if (e->currentState == AIState_Walk || e->currentState == AIState_Run) { if (e->entflags & EF_ENEM_IN_SIGHT) dbgCol = (Color){1.0f,0.0f,0.0f,1.0f}; else dbgCol = (Color){1.0f,1.0f,0.0f,1.0f}; }
    else if (e->currentState == AIState_Attack1 || e->currentState == AIState_Attack2 || e->currentState == AIState_Attack3) dbgCol = (Color){1.0f,0.0f,1.0f,1.0f}; else if (e->currentState == AIState_Pain) dbgCol = (Color){1.0f,0.0f,1.0f,1.0f}; else if (e->currentState == AIState_Dead) dbgCol = (Color){0.5f,0.5f,0.5f,1.0f}; else { dbgCol = (Color){1.0f,0.9f,0.8f,1.0f}; }
    DrawSphereWireframe(dbgCol, (ShapeSphere){sightPt, 0.32f});
}

void ModUpdate() {
    if (World.paused || World.menuActive) return;
    WeaponsUpdate(); PatchUpdate(); HardwareUpdate();
    if (Use()) Frob(World.position[PLAYER1],World.instances[PLAYER1].forward,World.instances[PLAYER1].right);
    if (World.pauseRelativeTime < World.debugLineFinished && (World.debugLineVertCount + 6) < (MAX_WIRELINE_VRTS * 3)) DrawLine(World.debugLine_start,World.debugLine_end,(Color){0.3f,0.1f,0.6f,0.5f});
    for (u16 i=INSTS_1ST_IDX;i<World.instCount;++i) {
        Entity* e = &World.instances[i]; u16 constdex = e->index;
        DelayedSpawnUpdate(i);
        if (e->textureAnimating && e->tickFinished < World.pauseRelativeTime) TextureSequenceUpdate(i);
        if(IdxIsButtonSwitch(constdex)){ButtonSwitchUpdate(i);} if(IdxIsDoor(constdex)){DoorUpdate(i);}    if(constdex == 701){LogicTimerUpdate(i);} if(e->itemLifeTime > 0.0f){SearchFXResetUpdate(i);}
        if(e->cyberTimer > 0.0f){CyberTimerUpdate(i);}          if(constdex == 515){ForceBridgeUpdate(i);} if(constdex == 517){FuncWallUpdate(i);}   if(constdex == 21 || constdex == 22){CyberWallUpdate(i);}
        if(IdxIsNPC(constdex)) { DrawAIDebug(i); /*AIControllerUpdate(i); AIAnimationControllerUpdate(i);*/ }
    }

    if (World.invP1.painSoundFinished < World.pauseRelativeTime && World.instances[PLAYER1].radiation > 1.0f && !(World.invP1.radSoundFinished < World.pauseRelativeTime)) { World.invP1.painSoundFinished = World.pauseRelativeTime + (double)random_range(2.5f,4.0f); play_wav(sounds[140]/*player/playerpain1*/,SfxVol() * 0.2f,(V3){0,0,0},false); }
    if (World.invP1.radBleedFinished < World.pauseRelativeTime && World.instances[PLAYER1].radiation > 1.0f) { World.invP1.radBleedFinished = World.pauseRelativeTime + 1.8; float take=World.instances[PLAYER1].radiation*0.2f; World.instances[PLAYER1].health-=take; World.painStaticAlpha = take > 15.0f ? 1.0f : take > 10.0f ? 0.8f : 0.3f; }
    if (World.invP1.radSoundFinished < World.pauseRelativeTime && World.instances[PLAYER1].radiation > 1.0f) { double minT = World.instances[PLAYER1].radiation > 50.0f ? 0.5 : 1.0; World.invP1.radSoundFinished = World.pauseRelativeTime + minT + (double)random_range(0.0f,2.0f); play_wav(sounds[90]/*hud/radiation*/,SfxVol() * 0.18f,(V3){0,0,0},false); }
}

u16 GetCrosshairTexture() { switch(World.invP1.weaponIndex) { case 343:case 345:case 350:case 352:case 355:return 1121;/*red*/case 344:case 347:case 357:return 1253;/*blue*/case 348:case 349:return 1066;/*orange*/case 351:case 354:return 1122;/*yellow*/ case 353:case 358:return 1161;/*teal*/default:return 1260;/*green*/ } }
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
