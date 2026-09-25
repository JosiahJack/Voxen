// citadel.c - Game logic.
#include "common.h"
__attribute__((used)) AutoSplitterData autoSplitter = {0x1337133713371337,0,false,0}; static const u16 patchMsg[7] = {325,326,327,328,329,330,331}; void PatchDisableAll(),BiomonitorEnergyPulse(float),BioMonitorClearGraphs(),TextureSequenceInit(u16,char*),GrenadeActivate(u16); bool RecentLog(); extern double lerpStartTime; extern V3 queuedLevelPos; extern u8 queuedLevelToLoad; extern u16 editModeSelection;
V3 ScreenPointToRay(V3 fwd, V3 rt) {
    float tanFov=vtan((float)Sys_Settings.FOV*0.5f*PI/180.0f),ndcX=((World.inventoryMode ? World.cursorPos_x : 683.0f) - 683.0f)/384.0f, ndcY=-((World.inventoryMode ? World.cursorPos_y : 384.0f)-384.0f)/384.0f; V3 view=V3_Normalize((V3){ndcX*tanFov,ndcY*tanFov,-1.0f}),flipForward=(V3){-fwd.x,-fwd.y,-fwd.z}; V3 up=V3_Normalize(V3_Cross(rt,flipForward)); return (V3){view.x*rt.x+view.y*up.x+view.z*flipForward.x,view.x*rt.y+view.y*up.y+view.z*flipForward.y,view.x*rt.z+view.y*up.z+view.z*flipForward.z};
}

static i16 GrenadeTypeFromConst(u16 idx) { switch(idx) { case 370:return 7; case 372:return 8; case 387:return 9; case 389:return 10; case 402:return 11; case 403:return 12; case 404:return 13; default:return -1; } }
static bool IsLiveGrenade(u16 idx) { return GrenadeTypeFromConst(idx) >= 7; }
static const float grenadeDamage[7]={150,325,80,375,230,200,150},grenadePenetration[7]={20,35,100,50,35,25,100},grenadeOffense[7]={3,6,3,6,5,3,3},grenadeRadius[7]={4,7,6,7.5f,5.1f,5.12f,4}; static const AttType grenadeAttackType[7]={Att_HitS,Att_HitS,Att_Magn,Att_HitS,Att_HitS,Att_HitS,Att_Gas};
static void GrenadeInit(u16 self) { i16 idx=GrenadeTypeFromConst(World.instances[self].index)-7; if(idx<0||idx>=7)return; Entity* e=&World.instances[self]; if(e->damage<=0.0f)e->damage=grenadeDamage[idx]; if(e->strength<=0.0f)e->strength=grenadePenetration[idx]; if(e->speed<=0.0f)e->speed=grenadeOffense[idx]; if(e->attackType==Att_None)e->attackType=grenadeAttackType[idx]; }
void ResetHeldItem() { World.invP1.heldObjectIndex=World.invP1.heldObjectCustIdx=U16_MAX; World.invP1.heldAmmo=World.invP1.heldAmmo2=0; World.invP1.heldObjectLoadedAlternate=World.invP1.holdingObject=World.invP1.grenActive=false; }
void DropHeldItem() {
    if (World.invP1.heldObjectIndex >= World.instCount) { ResetHeldItem(); return; }    if (World.invP1.dropFinished > World.pauseRelativeTime) {return;} World.invP1.dropFinished = World.pauseRelativeTime + 0.2;/*Prevent immediate re-grab at high fps*/ u16 n = AddInstance(World.invP1.heldObjectIndex,World.position[PLAYER1]);
    Entity* e = &World.instances[n]; e->customIndex = World.invP1.heldObjectCustIdx; e->ammo = World.invP1.heldAmmo; e->ammo2 = World.invP1.heldAmmo2; e->heldObjectLoadedAlternate = World.invP1.heldObjectLoadedAlternate;
    flag_set(&e->entflags,EF_RIGIDBODY,true); if(IsLiveGrenade(e->index)){World.layer[n]=L_PlayerBullets; e->recentMostActivator=PLAYER1; GrenadeInit(n); GrenadeActivate(n);} V3 tossDir = ScreenPointToRay(World.instances[PLAYER1].forward,World.instances[PLAYER1].right); World.position[n] = V3_AplusB(World.position[PLAYER1],V3_ScaleByF(tossDir,0.48f)); World.velocity[n] = V3_ScaleByF(tossDir,10.0f); ResetHeldItem();
}

void PatchUse(int patchSlot) {
    if (patchSlot < 0 || patchSlot > 6) return; if (World.invP1.patchCounts[patchSlot] <= 0) { CenterStatusPrint("%s", Sys_Text.stringTable[324]); return; } if (patchSlot == 3 && World.instances[PLAYER1].health >= 255.0f) { CenterStatusPrint("%s", Sys_Text.stringTable[304]); return; }/*Medi refused at full health, patch not consumed (Citadel PlayerPatch.cs)*/
    World.invP1.patchCounts[patchSlot]--; World.invP1.patchActive |= (u16)(1u << patchSlot);
    switch (patchSlot) {
        case 0: if(World.invP1.berserkFinished > World.pauseRelativeTime){World.invP1.berserkFinished += BERSERK_TIME;} else{World.invP1.berserkFinished = World.pauseRelativeTime + BERSERK_TIME; World.invP1.berserkIncTime = World.pauseRelativeTime + (BERSERK_TIME / 5.0); World.invP1.berserkIncrement = 0; berserkSeedTime = random_range(0.0f,1000.0f);} break;/*Voxen intentionally extends (rather than Citadel's reset of) the berserk timer on re-use, without resetting the visual effect*/
        case 1: PatchDisableAll(); World.invP1.patchActive |= PATCH_DETOX; World.invP1.detoxFinished = World.pauseRelativeTime + DETOX_TIME; break;/*Detox wipes all other patches on use; other patches may still be applied afterwards*/
        case 2: if(World.invP1.geniusFinished > World.pauseRelativeTime) World.invP1.geniusFinished += GENIUS_TIME; else World.invP1.geniusFinished = World.pauseRelativeTime + GENIUS_TIME; World.geniusActive = true; break;
        case 3: if(World.invP1.mediFinished > World.pauseRelativeTime) World.invP1.mediFinished += MEDI_TIME; else { World.invP1.mediFinished = World.pauseRelativeTime + MEDI_TIME; World.invP1.mediPatchPulseFinished = 0.0; } World.invP1.mediPatchPulseCount = 0; break;/*pulseFinished=0 heals immediately on fresh activation, matching Citadel*/
        case 4: { double reflexDur = REFLEX_TIME * REFLEX_TIME_SCALE;/*155 real seconds of 0.25x slow-mo, tracked in game-time so it pauses and saves*/ if(World.invP1.reflexFinishedTime > World.pauseRelativeTime) World.invP1.reflexFinishedTime += reflexDur; else World.invP1.reflexFinishedTime = World.pauseRelativeTime + reflexDur; World.timeScale = REFLEX_TIME_SCALE; } break;
        case 5: if(World.invP1.sightFinishedTime > World.pauseRelativeTime) World.invP1.sightFinishedTime += SIGHT_TIME; else { World.invP1.sightFinishedTime = World.pauseRelativeTime + SIGHT_TIME; World.invP1.sightSideEffectFinishedTime = -1.0; } break;
        case 6: if(World.invP1.staminupFinishedTime > World.pauseRelativeTime) World.invP1.staminupFinishedTime += STAMINUP_TIME; else World.invP1.staminupFinishedTime = World.pauseRelativeTime + STAMINUP_TIME; World.invP1.staminupActive = true; World.invP1.fatigue = 0.0f; break;
    } CenterStatusPrint("%s",Sys_Text.stringTable[patchMsg[patchSlot]]); if (World.invP1.patchCounts[World.invP1.patchCur] <= 0) { for (int i = 0; i < 7; i++) { if (World.invP1.patchCounts[i] > 0) { World.invP1.patchCur = (i8)i; break; } } } play_wav(sounds[89],AppliedFXVol(1.0f),(V3){0.0f,0.0f,0.0f},false);
}

void WeaponFireStartWeaponDip(float t) { if (t <= 0.0f) { World.invP1.reloadFinished = 0.0; return; } World.invP1.reloadFinished = World.pauseRelativeTime + (double)t; lerpStartTime = World.pauseRelativeTime; }
void WeaponFireCompleteWeaponChange() { World.invP1.justChangedWeap = false; World.invP1.recoiling = false; /* CompleteWeaponChange called by UpdateWeaponReloadDip when reloadLerpValue >= 0.5f after reload dip */ }
bool InventoryHasAccessCard(AccCardType card) { return (World.invP1.accessCardOwned & (1u << card)) != 0; }
bool InventoryHasAnyAccessCards() { return World.invP1.accessCardOwned != 0; }
const char* AccessCardCodeForType(AccCardType a) { // Called by ItemTabManager
    switch(a) {case ACC_Std: return "STD"; case ACC_Med: return "MED"; case ACC_Sci: return "SCI";  case ACC_Admin:return "ADM"; case ACC_Grp1: return "Group-1"; case ACC_Grp2:return "Group-2"; case ACC_Grp3:return "Group-3"; case ACC_Grp4:return "Group-4"; case ACC_GrpA:return "Group-A"; case ACC_GrpB:return "Group-B";
               case ACC_Stor:return "STO"; case ACC_Eng: return "ENG"; case ACC_Maint:return "MTN"; case ACC_Security:return "SEC"; case ACC_Per1:return "PER-1"; case ACC_Per2:return "PER-2";   case ACC_Per3:return "PER-3";   case ACC_Per4:return "PER-4";   case ACC_Per5:return "PER-5"; } return "Group-2";
}

void AddAccessCardToInventory(int index) {
    AccCardType card; if (!World.Sys_UI.firstGeneral){World.Sys_UI.firstGeneral=true; World.Sys_UI.MFD_CenterTab=3;}
    switch(index) {case 34:card=ACC_Admin; break; case 81:card=ACC_Std;  break; case 83:card=ACC_Grp1; break; case  84:card=ACC_Sci;  break; case 85:card=ACC_Eng; break; case 86:card=ACC_GrpB; break; case 87:card=ACC_Security; break; case 88:card=ACC_Per5; break; case 89:card=ACC_Med;   break; case 90:card=ACC_Grp3; break; case 91:card=ACC_Grp4; break; case 110:card=ACC_Per1; break;
                   default: CenterStatusPrint("BUG: Unmarked access card, defaulting to STD."); card = ACC_Std; break;}
    if (index == 87) { // Command card = STO + SEC + MTN
        if(InventoryHasAccessCard(ACC_Stor) && InventoryHasAccessCard(ACC_Security) && InventoryHasAccessCard(ACC_Maint)){CenterStatusPrint("%s%s",Sys_Text.stringTable[44],AccessCardCodeForType(card)); return;} World.invP1.accessCardOwned|=(1u<<ACC_Stor)|(1u<<ACC_Security)|(1u<<ACC_Maint); CenterStatusPrint("%s%s, %s, %s",Sys_Text.stringTable[45],AccessCardCodeForType(ACC_Stor),AccessCardCodeForType(ACC_Security),AccessCardCodeForType(ACC_Maint)); return;
    } if (InventoryHasAccessCard(card)) { CenterStatusPrint("%s%s",Sys_Text.stringTable[44],AccessCardCodeForType(card)); return; } World.invP1.accessCardOwned |= (1u << card); CenterStatusPrint("%s%s",Sys_Text.stringTable[45],AccessCardCodeForType(card));
}

void AddHardwareToInventory(int index,int hwversion) {
    if (!World.Sys_UI.firstHardware){World.Sys_UI.firstHardware=true; World.Sys_UI.MFD_CenterTab=2;} if((int)World.invP1.hwVers[index]==0 && index==1){World.Sys_UI.MFD_RightTab=3;} if (hwversion > 0 && hwversion <= (int)World.invP1.hwVers[index]) { CenterStatusPrint("%s",Sys_Text.stringTable[46]);/*THAT WARE IS OBSOLETE. DISCARDED.*/ return; } 
    static const u8 textIdx[12] = {21,22,23,24,25,26,27,28,29,30,31,32}; World.invP1.hardwareInvIndex = index; World.invP1.hasHardware |= (u16)(1u << index); World.invP1.hwVers[index] = (u8)hwversion; World.invP1.hwVersSetting[index]= hwversion > 0 ? (u8)(hwversion - 1) : 0; CenterStatusPrint("%s v%d",Sys_Text.stringTable[textIdx[index] + 326],hwversion);
}

void MFD_GeneralChanged();
bool AddGeneralObjectToInventory(int index, int custIdx){
    if (!World.Sys_UI.firstGeneral){World.Sys_UI.firstGeneral=true; World.Sys_UI.MFD_CenterTab=3;}
    for(i8 i=1;i<14;++i){if(World.invP1.generalInventoryIndexRef[i]==-1){if(!InventoryHasAnyAccessCards()&&World.invP1.generalInvCurrent==0){World.invP1.generalInvCurrent=i;} World.invP1.generalInventoryIndexRef[i]=index; World.invP1.generalInvCustIdx[i]=(i16)custIdx; MFD_GeneralChanged(); CenterStatusPrint("%s%s",Sys_Text.stringTable[ItemStringIdx(index)],Sys_Text.stringTable[31]); return true;}} return false;
}
void CheckForUnreadLogs() { int e=0,l=0; for (int i=0;i<LOGCNT;++i) if (World.invP1.hasLog[i] && !World.invP1.readLog[i]) *(Sys_Text.audioLogType[i] == AudioLogType_Email ? &e : &l)=1; World.invP1.hasNewEmail=e; World.invP1.hasNewLogs=l; }
static int FindNextUnreadLog() { for (int i = LOGCNT-1; i >= 0; i--) { if(World.invP1.hasLog[i] && !World.invP1.readLog[i]){return i;} } return -1; }
void PlayLog(int logIndex) {if(logIndex<0||logIndex>=LOGCNT||!(World.invP1.hasHardware&HW_ERD)){return;} play_message(AudioLogPath(logIndex)); World.invP1.readLog[logIndex]=true; if(Sys_Text.audioLogType[logIndex] == AudioLogType_Vmail){World.Sys_UI.vmailActive=true;} CenterStatusPrint("%s%s",Sys_Text.stringTable[1020],World.audiologNames[logIndex]);}
void PlayLastAddedLog(int logIndex) { if(logIndex < 0){return;} PlayLog(logIndex); World.invP1.lastAddedIndex = -1; }
void AddAudioLogToInventory(int index) {
    if (index < 0) { DualLog("BUG: Audio log picked up has no assigned index (-1)"); return; } if (index == 128) { CenterStatusPrint("%s",Sys_Text.stringTable[309]); return; }/*Trioptimum Funpack*/ World.invP1.hasLog[index]  = true; World.invP1.lastAddedIndex = index; World.invP1.numLogsFromLevel[Sys_Text.audioLogLevelFound[index]]++;
    if(Sys_Text.audioLogType[index] == AudioLogType_Email)World.invP1.hasNewEmail=true;else if(Sys_Text.audioLogType[index]==AudioLogType_Normal)World.invP1.hasNewLogs=true;
    if (World.invP1.hasHardware & HW_ERD) { char keyStr[8]; sFormat(keyStr,sizeof(keyStr),"%s", Sys_Settings.InputCodeSettings[20] ? "U" : "?"); CenterStatusPrint("%s%s%s %s",Sys_Text.stringTable[36],World.audiologNames[index],Sys_Text.stringTable[38],keyStr); } else { CenterStatusPrint("%s%s%s",Sys_Text.stringTable[36],World.audiologNames[index],Sys_Text.stringTable[310]); }
}

static inline void ItemAdd(u8 *cur, u8 *counts, int idx, int uIdx, int sysIdx) { if (!counts[*cur]) {*cur=(i8)idx;} counts[idx]++; CenterStatusPrint("%s%s", Sys_Text.stringTable[ItemStringIdx(uIdx)], Sys_Text.stringTable[sysIdx]); }
void AddGrenadeToInventory(int i, int u) { if (i >= 0){if (!World.Sys_UI.firstMain){World.Sys_UI.firstMain=true; World.Sys_UI.MFD_CenterTab=1;} World.invP1.grenConstIndex[i]=(i16)u; ItemAdd(&World.invP1.grenCur,World.invP1.grenAmmo,i,u,34);} }
void   AddPatchToInventory(int i, int u) { if (i >= 0){if (!World.Sys_UI.firstMain){World.Sys_UI.firstMain=true; World.Sys_UI.MFD_CenterTab=1;} ItemAdd(&World.invP1.patchCur,World.invP1.patchCounts,i,u,35);} }
static inline void GrenadeCycle(int step){int cur= World.invP1.grenCur, next=cur; for(int i=0;i<7;++i){next=(next+step+7)%7; if(   World.invP1.grenAmmo[next]>0){World.invP1.grenCur =(i8)next; CenterStatusPrint("%s",Sys_Text.stringTable[579+next]); return;}}}
static inline void   PatchCycle(int step){int cur=World.invP1.patchCur, next=cur; for(int i=0;i<7;++i){next=(next+step+7)%7; if(World.invP1.patchCounts[next]>0){World.invP1.patchCur=(i8)next; CenterStatusPrint("%s",Sys_Text.stringTable[579+next]); return;}}}
void RemoveGrenade(int i) { if(World.invP1.grenAmmo[i] > 0){World.invP1.grenAmmo[i]--;} if(!World.invP1.grenAmmo[i]){GrenadeCycle(-1);} }
static i8 GetExistingCyberItemIndex() { if (World.invP1.softVersions[SW_TURBO]  > 0) {return 0;} if (World.invP1.softVersions[SW_DECOY]  > 0) {return 1;} if (World.invP1.softVersions[SW_RECALL] > 0) {return 2;} return -1; }
static void UseTurbo() {if(World.invP1.softVersions[SW_TURBO]<=0){World.invP1.hasSoft&=(u8)~(1u << SW_TURBO); return;} if(--World.invP1.softVersions[SW_TURBO]==0)World.invP1.hasSoft&=(u8)~(1u << SW_TURBO); if(World.invP1.turboFinished > World.pauseRelativeTime){World.invP1.turboFinished+=World.invP1.turboCyberTime;}else{World.invP1.turboFinished=World.invP1.turboCyberTime+World.pauseRelativeTime;}}
static void UseDecoy() {if (World.decoyActive) { CenterStatusPrint("%s",Sys_Text.stringTable[537]); return; } if (World.invP1.softVersions[SW_DECOY] <= 0) { World.invP1.hasSoft &= (u8)~(1u << SW_DECOY); return; } if (--World.invP1.softVersions[SW_DECOY] == 0) World.invP1.hasSoft &= (u8)~(1u << SW_DECOY); u16 decoyIdx = SpawnDynamicObject(417,true);/*417 = CyberDecoy constIndex*/ if(decoyIdx != U16_MAX){World.position[decoyIdx]=World.position[PLAYER1];}}
static void UseRecall() { if (World.invP1.softVersions[SW_RECALL] <= 0) {return;} if (--World.invP1.softVersions[SW_RECALL] == 0) {World.invP1.hasSoft &= (u8)~(1u << SW_RECALL);} World.position[PLAYER1] = World.cyberspaceRecallPoint; }
void UseCyberspaceItem() {
    if (World.invP1.cyberItemIndex <= 0) { World.invP1.cyberItemIndex = GetExistingCyberItemIndex(); if (World.invP1.cyberItemIndex < 0) { CenterStatusPrint("%s",Sys_Text.stringTable[473]); return; } }
    switch(World.invP1.cyberItemIndex) {case 0: if (!World.invP1.softVersions[SW_TURBO])  { World.invP1.cyberItemIndex = GetExistingCyberItemIndex(); return; } UseTurbo();  break; case 1: if (!World.invP1.softVersions[SW_DECOY])  { World.invP1.cyberItemIndex = GetExistingCyberItemIndex(); return; } UseDecoy();  break; case 2: if (!World.invP1.softVersions[SW_RECALL]) { World.invP1.cyberItemIndex = GetExistingCyberItemIndex(); return; } UseRecall(); break;}
}

void CycleCyberSpaceItemUp() { int next = World.invP1.cyberItemIndex + 1; if (next > 2){next=0;} for (int c = 0; c <= 7; c++) { if (World.invP1.hasSoft & (1u << (SW_TURBO+next))) { World.invP1.cyberItemIndex = (i8)next; return; } if (c == 7) { World.invP1.cyberItemIndex = -1; return; } if (++next > 2) {next = 0;} } }
void CycleCyberSpaceItemDn() { int next = World.invP1.cyberItemIndex - 1; if (next < 0){next=2;} for (int c = 0; c <= 7; c++) { if (World.invP1.hasSoft & (1u << (SW_TURBO+next))) { World.invP1.cyberItemIndex = (i8)next; return; } if (c == 7) { World.invP1.cyberItemIndex = -1; return; } if (--next < 0) {next = 2;} } }
void RemoveWeapon(i32 slot) { World.invP1.weaponInventoryIndices[slot] = World.invP1.weaponInventoryAmmoIndices[slot] = -1; if (slot == World.invP1.weaponCurrent) { bool anyLeft = false; for (int i=0;i<7;i++) if (World.invP1.weaponInventoryIndices[i] >= 0) { anyLeft = true; break; } if (!anyLeft) World.instances[World.weaponVModelIndex].modelIndex = MAX_MDLS; } }
static float DefaultEnergySettingForWeapon(int wep16Index) { return (wep16Index == 4) ? 5.0f : (wep16Index == 10) ? 13.0f : (wep16Index == 14) ? 2.0f : 3.0f; }
__attribute__((noinline)) void AddAmmoToInventory(int index,int constIndex,int amount,bool isSecondary) { if(index < 0){return;} if(isSecondary){World.invP1.wepAmmoSecondary[index]+=(u16)amount;} else {World.invP1.wepAmmo[index]+=(u16)amount;} CenterStatusPrint("%s%s",Sys_Text.stringTable[ItemStringIdx(constIndex)],Sys_Text.stringTable[630]); }
bool AddWeaponToInventory(int index,int ammo1,int ammo2,bool loadedAlt) {
    if (index < 0) return false; if (!World.Sys_UI.firstMain){World.Sys_UI.firstMain=true; World.Sys_UI.MFD_CenterTab=1;} if(!World.Sys_UI.firstWeapon){World.Sys_UI.firstWeapon=true; World.Sys_UI.MFD_LefTab=1;}
    for (i32 i = 0; i < 7; i++) {
        if(World.invP1.weaponInventoryIndices[i] >= 0){continue;} World.invP1.weaponInventoryIndices[i] = index; i32 index16 = Get16WeaponIndexFromConstIndex(index); World.invP1.weaponEnergySetting[i] = DefaultEnergySettingForWeapon(index16); if (i == 0) { World.invP1.weaponCurrentPending=i; World.invP1.weaponIndexPending=(u16)index; World.invP1.justChangedWeap=true; WeaponFireStartWeaponDip(0.5f); WeaponFireCompleteWeaponChange(); }
        if (loadedAlt && ammo2 > 0){World.invP1.currentMagazineAmount2[i]=(u8)ammo2; if (ammo1 > 0) World.invP1.wepAmmo[index16]+=(u16)ammo1; World.invP1.wepLoadedWithAlternate[i]=true;}else{World.invP1.currentMagazineAmount[i]=(u8)ammo1; if (ammo2 > 0) World.invP1.wepAmmoSecondary[index16]+=(u16)ammo2; World.invP1.wepLoadedWithAlternate[i]=false;}
        CenterStatusPrint("%s%s",Sys_Text.stringTable[ItemStringIdx(index)],Sys_Text.stringTable[33]); World.invP1.numweapons=0; for (i32 j=0;j<7;j++) { if(World.invP1.weaponInventoryIndices[j] >= 0){World.invP1.numweapons++;} } return true;
    } return false;
}

void UseGrenade(int index) {
    static const u8 slots[7]={0,3,1,6,4,5,2};
    if (index<314 || index>320) return;
    if (!World.invP1.grenAmmo[slots[index-314]]) { CenterStatusPrint("%s",Sys_Text.stringTable[322]); return; }
    if (World.invP1.holdingObject) { CenterStatusPrint("%s",Sys_Text.stringTable[311]); return; }/*Can't use grenade, hands full*/ ForceInventoryMode(); ResetHeldItem(); World.invP1.grenActive=true; CenterStatusPrint("%s%s",Sys_Text.stringTable[ItemStringIdx(index)],Sys_Text.stringTable[320]); /*activated, grenade is LIVE!*/
    switch(index) {case 314:World.invP1.heldObjectIndex=370; RemoveGrenade(0); break; /*Frag*/         case 315:World.invP1.heldObjectIndex=372; RemoveGrenade(3); break; /*Concussion*/ case 316:World.invP1.heldObjectIndex=387; RemoveGrenade(1); break; /*EMP*/ case 317:World.invP1.heldObjectIndex=389; RemoveGrenade(6); break; /*Earth Shaker*/
                   case 318:World.invP1.heldObjectIndex=402; RemoveGrenade(4); break; /*Land Mine*/  case 319:World.invP1.heldObjectIndex=403; RemoveGrenade(5); break; /*Nitropak*/ case 320:World.invP1.heldObjectIndex=404; RemoveGrenade(2); break; /*Gas*/ default: return;}
    World.invP1.heldObjectCustIdx = U16_MAX; World.invP1.heldAmmo = 0; World.invP1.heldAmmo2 = 0; World.invP1.heldObjectLoadedAlternate = false; World.invP1.holdingObject = true;
}

void InventoryUpdate() {
    if (Grenade()) { if (World.curLev == LEVEL_CYBERSPACE){UseCyberspaceItem();} else if (World.invP1.grenCur >= 0 && World.invP1.grenCur < 7 && World.invP1.grenAmmo[World.invP1.grenCur] > 0){UseGrenade(World.invP1.grenConstIndex[World.invP1.grenCur]);} else {CenterStatusPrint("%s",Sys_Text.stringTable[322]);/*Out of grenades.*/} }
    if (GrenadeCycUp())  { if (World.curLev == LEVEL_CYBERSPACE) CycleCyberSpaceItemUp(); else GrenadeCycle( 1); } if (GrenadeCycDown()){ if (World.curLev == LEVEL_CYBERSPACE) CycleCyberSpaceItemDn(); else GrenadeCycle(-1); }
    if (RecentLog() && (World.invP1.hasHardware & HW_ERD)) {
        if(World.invP1.lastAddedIndex>=0){int temp=World.invP1.lastAddedIndex; PlayLog(temp); World.invP1.lastAddedIndex=FindNextUnreadLog(); if(World.invP1.lastAddedIndex==temp)World.invP1.lastAddedIndex=-1; CheckForUnreadLogs(); }else{int temp=World.invP1.lastAddedIndex; World.invP1.lastAddedIndex=FindNextUnreadLog(); if(World.invP1.lastAddedIndex==temp){World.invP1.lastAddedIndex=-1;} CheckForUnreadLogs(); CenterStatusPrint("%s",Sys_Text.stringTable[1019]);/*Log playback stopped.*/}
    } if (Patch()) { if (World.invP1.patchCur >= 0 && World.invP1.patchCur < 7 && World.invP1.patchCounts[World.invP1.patchCur] > 0){PatchUse(World.invP1.patchCur);} else {CenterStatusPrint("%s",Sys_Text.stringTable[324]); /*Out of patches.*/} } if (PatchCycUp()){PatchCycle( 1);} else if (PatchCycDown()){PatchCycle(-1);}
}

void AddItemFail(int index/*Expects usableItem index*/) { DropHeldItem(); CenterStatusPrint("%s%s%s", Sys_Text.stringTable[32],Sys_Text.stringTable[ItemStringIdx(index)],Sys_Text.stringTable[318]);/*Inventory full.*/ }
extern u8 magazinePitchCountForWeapon[16],magazinePitchCountForWeapon2[16];
void AddItemToInventory(int index, int custIdx) {
    if (IdxIsGenericItem(index)) { if(!AddGeneralObjectToInventory(index,custIdx)){AddItemFail(index);} } else if (IdxIsAudioLog(index)) { AddAudioLogToInventory(World.invP1.heldObjectCustIdx); } 
    else if (IdxIsWeapon(index)) { int constIndex = index + 307; if (constIndex < 343 || constIndex > 358) constIndex = index; if (!AddWeaponToInventory(constIndex,World.invP1.heldAmmo,World.invP1.heldAmmo2,World.invP1.heldObjectLoadedAlternate)) { AddItemFail(index); } } else if (IdxIsAccessCard(index)) AddAccessCardToInventory(UseableFromConst(index));
    else if (IdxIsHardware(index)) AddHardwareToInventory(index-328,custIdx);
    else {
        switch (index) {
            case 314: AddGrenadeToInventory(0,index); break; /*Frag*/ case 315: AddGrenadeToInventory(3,index); break; /*Concussion*/ case 316: AddGrenadeToInventory(1,index); break; /*EMP*/ case 317: AddGrenadeToInventory(6,index); break; /*Earth Shaker*/ case 318: AddGrenadeToInventory(4,index); break; /*Land Mine*/ case 319: AddGrenadeToInventory(5,index); break;/*Nitropak*/
            case 320: AddGrenadeToInventory(2,index); break; /*Gas*/
            case 321: case 322: case 323: case 324: case 325: case 326: case 327: AddPatchToInventory(index-321,index); break;
            case 21: AddHardwareToInventory(0,custIdx); break; case 22: AddHardwareToInventory(1,custIdx); break; case 23: AddHardwareToInventory(2,custIdx); break; case 24: AddHardwareToInventory(3,custIdx); break; case 25: AddHardwareToInventory(4,custIdx); break; case 26: AddHardwareToInventory(5,custIdx); break;
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
void SearchFXEnable(int side) {
    side=side==1; World.Sys_UI.searchFXActive[side]=true; World.Sys_UI.searchFXStartTime[side]=World.pauseRelativeTime;
    World.Sys_UI.searchFXCursorX[side]=(float)World.cursorPos_x; World.Sys_UI.searchFXCursorY[side]=(float)World.cursorPos_y;
}
void SearchFXResetEnable(u16 self) { Entity* e = &World.instances[self]; if (e->itemLifeTime <= 0.0f) {e->itemLifeTime = 3.0f;} e->delayFinished = World.pauseRelativeTime + e->itemLifeTime; }
void SearchFXResetUpdate(u16 self) { Entity* e = &World.instances[self]; if (e->delayFinished >= World.pauseRelativeTime) {return;} flag_set(&e->entflags,EF_ACTIVE,false); }
void DelayedSpawnEnable(u16 self) { Entity* e = &World.instances[self]; e->timerFinished = World.pauseRelativeTime + e->delay; e->active = true; }
void DelayedSpawnUpdate(u16 s) { Entity* e=&World.instances[s]; if(!e->active||e->timerFinished<=0.0||e->timerFinished>World.pauseRelativeTime){return;} e->active=false; if(!e->doSelfAfterList){return;} if(e->despawnInstead){if(e->destroyAfterListInsteadOfDeactivate){DeleteInstance(s);}else{flag_set(&e->entflags,EF_ACTIVE,false);}}else flag_set(&e->entflags,EF_ACTIVE,true);}
void FuncWallShiftChildren(u16 self, V3 delta) { if (vabs(delta.x)+vabs(delta.y)+vabs(delta.z) < 0.00001f) {return;} for (u16 i=PLAYER1;i<World.instCount;++i) { if (fwParentOf[i]==self) { World.position[i]=V3_AplusB(World.position[i],delta); } } }
void FuncWallInitAfterLoad(u16 self) {
    Entity* e=&World.instances[self]; V3 prev=World.position[self]; float distTotal=V3_Dist(e->startPosition,e->targetPosition); float f=0; if((u8)e->funcState>FStat_AjarMovingTarget)f=e->ajarPercentage; else if(e->funcState==FStat_AjarMovingTarget) f=e->ajarPercentage;
    else if(e->funcState ==FStat_AjarMovingStart){f=1.0f-e->ajarPercentage;} if (f < 0.0f) f = 0.0f; if (f > 1.0f) f = 1.0f; V3 np=(distTotal > 0.0001f) ? V3_AplusB(e->startPosition,V3_ScaleByF(V3_Normalize(V3_AsubB(e->targetPosition,e->startPosition)),distTotal*f)) : e->startPosition; World.position[self]=np; if ((u8)e->funcState <= FStat_MovingTarget) { e->funcState = FStat_Start; e->percentMoved = 0.0f; } FuncWallShiftChildren(self,V3_AsubB(np,prev));
}

void FuncWallMoveStart(u16 self) { World.instances[self].funcState = FStat_MovingStart; World.instances[self].tickFinished = World.pauseRelativeTime + 10.0f; }
void FuncWallMoveTarget(u16 self) { World.instances[self].funcState = FStat_MovingTarget; World.instances[self].tickFinished = World.pauseRelativeTime + 10.0f; }
void FuncWallTargetted(u16 self) { Entity* e = &World.instances[self]; u8 st = (u8)e->funcState; bool toTarget = st == FStat_Start || st == FStat_MovingStart || st == FStat_AjarMovingTarget || (st > FStat_AjarMovingTarget && e->ajarPercentage > 0.0f); if (toTarget){FuncWallMoveTarget(self);} else{FuncWallMoveStart(self);} play_wav(sounds[76],1.0f,World.position[self],true); }
void FuncWallUpdateInner(u16 self) {
    Entity* e = &World.instances[self]; if (e->funcState != FStat_MovingStart && e->funcState != FStat_MovingTarget) return; V3 goal = e->funcState == FStat_MovingStart ? e->startPosition : e->targetPosition; FuncStates doneState = e->funcState == FStat_MovingStart ? FStat_Start : FStat_Target; V3 delta = V3_AsubB(goal,World.position[self]);
    float distanceLeft = V3_Mag(delta), total = V3_Dist(e->startPosition,e->targetPosition), dist = e->speed * (float)World.deltaTime * World.timeScale; if (distanceLeft <= dist || e->tickFinished < World.pauseRelativeTime) { World.position[self]=goal; e->funcState=doneState; e->percentMoved=doneState == FStat_Target ? 1.0f : 0.0f; return; }
    if (distanceLeft > 0.0001f) World.position[self]=V3_AplusB(World.position[self],V3_ScaleByF(V3_Normalize(delta),dist)); if (total > 0.0001f) e->percentMoved = V3_Dist(e->startPosition,World.position[self]) / total;
}
void FuncWallUpdate(u16 self) { V3 prev = World.position[self]; FuncWallUpdateInner(self); FuncWallShiftChildren(self,V3_AsubB(World.position[self],prev)); }
void func_forcebridge(u16 self) {
    Entity* e = &World.instances[self]; e->tickFinished = World.pauseRelativeTime + 0.05f + (double)random_range(0.0f,1.0f); e->lerping = true; if(e->activatedScale.x <= 0.02f){e->activatedScale.x = 2.56f;} if(e->activatedScale.y <= 0.02f){e->activatedScale.y = 0.08f;} if(e->activatedScale.z <= 0.02f){e->activatedScale.z = 2.56f;}
    if(!e->active){ e->modelIndex=MAX_MDLS; World.col[self]=COLTYPE_NONE;} switch (e->fieldColor) { case ForceFieldColor_Red:e->texIndex=38; break; case ForceFieldColor_Green:e->texIndex=40; break; case ForceFieldColor_Blue:e->texIndex=39; break; case ForceFieldColor_Purple:e->texIndex=41; break; case ForceFieldColor_RedFaint:e->texIndex=198; break; }
}

void ForceBridgeActivate(u16 s, bool silent){Entity* e=&World.instances[s]; if(e->active){return;} if(!silent){play_wav(sounds[102],1.0f,World.position[s],true);} flag_set(&e->entflags,EF_ACTIVE,true); e->modelIndex=78; World.col[s]=COLTYPE_BOX; e->active=e->lerping=true; World.scale[s]=(V3){ e->forceFieldDirectionX ? 0.1f : e->activatedScale.x,e->forceFieldDirectionY ? 0.1f : e->activatedScale.y,e->forceFieldDirectionZ ? 0.1f : e->activatedScale.z };}
void ForceBridgeDeactivate(u16 self, bool silent) { Entity* e = &World.instances[self]; if (!e->active) {return;} if (!silent) {play_wav(sounds[102],1.0f,World.position[self],true);} e->active = false; e->lerping = true; }
void ForceBridgeToggle(u16 self) { if (World.instances[self].active) {ForceBridgeDeactivate(self,false); } else {ForceBridgeActivate(self,false);} }
void ForceBridgeUpdate(u16 self) {
    Entity* e = &World.instances[self]; if(e->tickFinished >= World.pauseRelativeTime){return;} e->tickFinished = World.pauseRelativeTime + 0.05f;
    if (e->active) {
        if (!e->lerping) return; float sx=e->forceFieldDirectionX ? lerp(World.scale[self].x,e->activatedScale.x,0.1f) : World.scale[self].x, sy=e->forceFieldDirectionY ? lerp(World.scale[self].y,e->activatedScale.y,0.1f) : World.scale[self].y, sz=e->forceFieldDirectionZ ? lerp(World.scale[self].z,e->activatedScale.z,0.1f) : World.scale[self].z; 
        World.scale[self]=(V3){sx,sy,sz}; if(vabs(e->activatedScale.x - sx) < 0.08f && vabs(e->activatedScale.y - sy) < 0.08f && vabs(e->activatedScale.z - sz) < 0.08f){World.scale[self]=e->activatedScale; e->lerping=false;}
    } else if (e->lerping) {float sx=e->forceFieldDirectionX ? lerp(World.scale[self].x,0.0f,0.1f) : World.scale[self].x, sy=e->forceFieldDirectionY ? lerp(World.scale[self].y,0.0f,0.1f) : World.scale[self].y, sz=e->forceFieldDirectionZ ? lerp(World.scale[self].z,0.0f,0.1f) : World.scale[self].z; World.scale[self]=(V3){sx,sy,sz}; if (sx < 0.08f || sy < 0.08f || sz < 0.08f) { e->modelIndex = MAX_MDLS; World.col[self] = COLTYPE_NONE; e->lerping=false;}}
}

void TriggerCounterTarget(u16 self, u16 activator) { UseTargets(activator,World.instances[self].targetIdx); }
void TriggerCounterDelayedTarget(u16 self, u16 act) { World.instances[self].delayFinished = World.pauseRelativeTime + World.instances[self].delay; TriggerCounterTarget(self,act); }
void TriggerCounterTargetted(u16 self, u16 act) { Entity* e=&World.instances[self]; e->counter++; if (e->counter != e->countToTrigger) {return;} if (e->delay <= 0.0f){TriggerCounterTarget(self,act);}else{TriggerCounterDelayedTarget(self,act);} if (!e->dontReset){e->counter=0;} }
void TextureChangerToggle(u16 self) {
    u16 alt = 0, glowAlt = 0;
    if (World.instances[self].index == 538) { alt = 1118; glowAlt = 1116; } else if (World.instances[self].index == 689) { alt = 841; glowAlt = 840; } else if (World.instances[self].index == 690) { alt = 844; glowAlt = 843; } else if (World.instances[self].index == 695) { alt = 858; glowAlt = 857; } else return;
    if (World.instances[self].curTex) { World.instances[self].texIndex = EDefs[World.instances[self].index].texIndex; World.instances[self].glowIndex = EDefs[World.instances[self].index].glowIndex; } else { World.instances[self].texIndex = alt; World.instances[self].glowIndex = glowAlt; } World.instances[self].curTex = !World.instances[self].curTex;
}

void LogicTimerInitBeforeLoad(u16 self) { Entity* e=&World.instances[self]; if(e->timeInterval <= 0.0f){e->timeInterval=0.35f;} if(e->randomMin <= 0.0f){e->randomMin=5.0f;} if(e->randomMax <= 0.0f){e->randomMax=10.0f;} e->intervalFinished=World.pauseRelativeTime + (e->useRandomTimes ? (double)random_range(e->randomMin,e->randomMax) : (double)e->timeInterval); }
void LogicTimerUseTargets(u16 self) { UseTargets(self,World.instances[self].targetIdx); }
void LogicTimerUpdate(u16 self) { Entity* e=&World.instances[self]; if(!e->active || e->intervalFinished >= World.pauseRelativeTime){return;} e->intervalFinished=World.pauseRelativeTime + (e->useRandomTimes ? (double)random_range(e->randomMin,e->randomMax) : (double)e->timeInterval); LogicTimerUseTargets(self); }
void LogicTimerTargetted(u16 self, u16 activator) { (void)activator; World.instances[self].active = !World.instances[self].active; }
void ButtonSwitchInitAfterLoad(u16 self) { Entity* e=&World.instances[self]; e->delayFinished=0.0f; if(e->active){e->tickFinished=World.pauseRelativeTime + 1.5 + (double)random_range(0.0f,1.0f);} }
void ButtonSwitchUseTargets(u16 self) { Entity* e=&World.instances[self]; UseTargets(self,e->targetIdx); e->active=!e->active; if(e->index == 689 || e->index == 690 || e->index == 695) { TextureChangerToggle(self); if(e->index == 689 && e->active){e->tickFinished=World.pauseRelativeTime + 1.5f;} } }
static __attribute__((noinline)) void UIBlockedBySecurity(V3 tetherPoint) { (void)tetherPoint; play_wav(sounds[468],0.85f,(V3){0,0,0},false);/*blocked_by_security*/ CenterStatusPrint("%s",Sys_Text.stringTable[25]); }
static __attribute__((noinline)) void EntitySetLocked(Entity* e, bool locked) { flag_set(&e->entflags,EF_LOCKED,locked); }
void ButtonSwitchUse(u16 self, u16 activator) {
    Entity* e = &World.instances[self]; if(Cheats.superoverride || World.diffMis == 0){EntitySetLocked(e,false);} else if(GetCurrentLevelSecurity() > e->securityThreshold){UIBlockedBySecurity(World.position[self]); return;}
    if ((e->entflags & EF_LOCKED) != 0) { CenterStatusPrint("%s",Sys_Text.stringTable[e->lockedMessageLingdex]); if (e->SFXLockedIndex >= 0 && e->SFXLockedIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXLockedIndex],1.0f,World.position[self],true); return; }
    if (e->SFXIndex >= 0 && e->SFXIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXIndex],1.0f,World.position[self],true);
    CenterStatusPrint("%s",Sys_Text.stringTable[e->messageIndex]); if (e->delay > 0.0f) { e->recentMostActivator = activator; e->delayFinished = World.pauseRelativeTime + e->delay; } else ButtonSwitchUseTargets(self);
}

void ButtonSwitchUpdate(u16 self) { double t=World.pauseRelativeTime; Entity* e=&World.instances[self]; if (e->delayFinished > 0.0 && e->delayFinished < t){e->delayFinished=0.0; ButtonSwitchUseTargets(self);} if (e->index == 689 && e->active && e->tickFinished < t) { TextureChangerToggle(self); e->tickFinished=t+1.5f; } }
void HealingBedUse(u16 self, u16 owner) { Entity* e=&World.instances[self]; if (GetCurrentLevelSecurity() <= (u8)e->minSecurityLevel) { if(!e->broken){HealthManagerHealingBed(PLAYER1,e->amount,true); CenterStatusPrint("%s",Sys_Text.stringTable[23],owner); play_wav(sounds[103],1.0f,World.position[self],false);} else {CenterStatusPrint("%s",Sys_Text.stringTable[24],owner);} } else UIBlockedBySecurity(World.position[self]); }
int GeneralInvItem(int slot);
bool GeneralInvCanVaporize(int slot);
void GeneralInvRemove(int slot);
void VaporizeClick() {int slot=World.invP1.generalInvCurrent; if(!GeneralInvCanVaporize(slot))return; GeneralInvRemove(slot); play_wav(sounds[89],AppliedFXVol(1.0f),(V3){0},false);}
typedef struct { i8 norm,alt; } AmmoIconEntry;
static const AmmoIconEntry ammoIconTable[51]={[36-36]={7,8}/*Magnesium/Penetrator*/,[37-36]={-2,-2}/*Energy*/,[38-36]={0,1}/*Needle/Tranq*/,[39-36]={9,10}/*Hornette/Splinter*/,[40-36]={-2,-2}/*Energy*/,[41-36]={-1,-1}/*Rapier, no ammo*/,[42-36]={-1,-1}/*Pipe, no ammo*/,[43-36]={5,6}/*Hollow/Slug*/,[44-36]={11,-1}/*Magcart*/,[45-36]={2,3 }/*Standard/Teflon*/,[46-36]={-2,-2}/*Energy*/,[47-36]={14,-1}/*Rail Rounds*/,[48-36]={4,-1}/*Rubber Slugs*/,[49-36]={12,13}/*Slag/Large Slag*/,[50-36]={-2,-2}/*Energy*/,[51-36]={-2,-2}/*Energy*/};
i8 AmmoIconGet(int index,bool alt) { if (index < 343 || index > 358) {return -1;} const AmmoIconEntry* e = &ammoIconTable[index - 343]; return alt ? e->alt : e->norm; }
static double creditsVidStartTime,creditsVidFinished; static u8 creditsVidPhase; // CreditsScroll, TODO video text phases: 0=text1 visible, 1=text2 visible, 2=text3 visible, 3=all hidden
void CreditsOnEnable() { World.creditsActive=true; World.creditsPageIndex=0; creditsVidStartTime=World.absoluteTime; creditsVidFinished=World.absoluteTime + 37.2; creditsVidPhase=0; }
void CreditsUpdate() {
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

void CyborgConversionToggleTargetted() {bool active=(World.ressurectionActiveLevels>>World.curLev)&1u; flag_setu16(&World.ressurectionActiveLevels,(1u<<World.curLev),!active); if(World.curLev==6)flag_setu16(&World.ressurectionActiveLevels,(1u<<10|1u<<11|1u<<12),!active);/*Set groves 10,11,12 when 6 toggled, shared*/ play_wav(sounds[active ? 183 : 184],Sys_Settings.VolumeMessage,(V3){0.0f,0.0f,0.0f},false);/*"vox_cybconvcancelled" : "vox_cybconvenabled"*/ CenterStatusPrint("%s",Sys_Text.stringTable[active ? 591 : 592]);}
void ElevatorButtonClick(u16 self) {
    Entity* e = &World.instances[self]; if (World.Sys_UI.linkedElevatorDoor == U16_MAX) { CenterStatusPrint("%s",Sys_Text.stringTable[6]); /*Too far away from that.*/ return; } Entity* door = &World.instances[World.Sys_UI.linkedElevatorDoor]; bool doorClosed = door->doorOpen == DoorState_Closed; float dist = V3_Dist(World.Sys_UI.objectInUsePos,World.position[PLAYER1]); 
    if (dist > 2.0f/*tether dist*/ && !doorClosed) { CenterStatusPrint("%s",Sys_Text.stringTable[6]); /*Too far away from that.*/ return; } if (!doorClosed) { CenterStatusPrint("%s",Sys_Text.stringTable[7]); /*Door not closed.*/ return; } if (!(e->entflags & EF_ACTIVE)) { CenterStatusPrint("%s",Sys_Text.stringTable[8]); /*Floor not accessible.*/ return; }
    queuedLevelPos=(e->targetDestinationID != U16_MAX && e->targetDestinationID < World.instCount) ? World.position[e->targetDestinationID] : (V3){0.0f,0.0f,0.0f}; queuedLevelToLoad=(u8)e->teleportID;
}

void EmailTargetted(u16 self) { Entity* e=&World.instances[self]; u16 idx=e->emailIndex; if(World.invP1.hasLog[idx]){return;} World.invP1.hasLog[idx]=World.invP1.hasNewEmail=true; World.invP1.lastAddedIndex=idx; if(Sys_Text.audioLogType[idx] == AudioLogType_Email){World.invP1.beepDone=true;} if(e->autoPlayEmail){PlayLastAddedLog(idx);} }
u8 OverloadButtonVisualState() { if (World.invP1.currentEnergyWeaponHeat[World.invP1.weaponCurrent] > 25.0f) {return 2;} if (World.invP1.overloadEnabled) {return 1;} return 0; }
void OverloadButtonAction() {
    static double overloadClickFinished = 0.0; if (overloadClickFinished >= World.pauseRelativeTime){return;} overloadClickFinished = World.pauseRelativeTime + 0.4; if (World.invP1.currentEnergyWeaponHeat[World.invP1.weaponCurrent] > 25.0f) { CenterStatusPrint("%s",Sys_Text.stringTable[12]);/*Weapon too hot*/ return; }
    if (World.invP1.overloadEnabled) { CenterStatusPrint("%s",Sys_Text.stringTable[13]);/*Overload disabled*/ World.invP1.overloadEnabled = false; } else { CenterStatusPrint("%s",Sys_Text.stringTable[17]);/*Overload enabled*/ World.invP1.overloadEnabled = true; }
}
// TargetID is render-only: state is kept by NPC index and no instance is spawned.
static i16 targetIDText[INSTANCE_COUNT]; static double targetIDTextFinished[INSTANCE_COUNT],targetIDAttachedFinished[INSTANCE_COUNT]; static bool targetIDAttached[INSTANCE_COUNT];
void TargetIDReset() { mset(targetIDText,0,sizeof(targetIDText));mset(targetIDTextFinished,0,sizeof(targetIDTextFinished));mset(targetIDAttachedFinished,0,sizeof(targetIDAttachedFinished));mset(targetIDAttached,0,sizeof(targetIDAttached)); }
float TargetIDGetSensingRange(bool manual) { u8 ver=World.invP1.hwVers[HW_TID_IDX]; if(manual)return ver==0?12.0f:ver>=4?18.0f:13.0f; return ver==0?12.0f:ver<=2?0.0f:ver==3?13.0f:20.0f; }
float TargetIDGetTetherRange() { return World.invP1.hwVers[HW_TID_IDX]>=4?22.0f:15.0f; }
void TargetIDSendDamageReceive(u16 self,float damage,AttType attackType) {
    if(self>=World.instCount||!IdxIsNPC(World.instances[self].index))return; Entity* npc=&World.instances[self];
    if(attackType==Att_Trnq){targetIDText[self]=536;targetIDTextFinished[self]=World.pauseRelativeTime;}
    else{float mh=npcTable[npc->index-419].health;targetIDText[self]=damage>mh*.75f?514:damage>mh*.50f?515:damage>mh*.25f?513:damage>0.0f?512:511;targetIDTextFinished[self]=World.pauseRelativeTime+(damage==0.0f?1.0:2.5);}
}
bool TargetIDShouldRender(u16 npc) {
    if(npc>=World.instCount||!IdxIsNPC(World.instances[npc].index))return false; Entity* e=&World.instances[npc];
    if(!(e->entflags&EF_ACTIVE)||(e->entflags&EF_DEAD)||e->health<=0.0f||V3_Dist(World.position[npc],World.position[PLAYER1])>10.0f)return false;
    if(targetIDAttached[npc]&&targetIDAttachedFinished[npc]<World.pauseRelativeTime)targetIDAttached[npc]=false;
    if((World.invP1.hasHardware&HW_TID)&&(World.invP1.hwVers[HW_TID_IDX]==0||World.invP1.hwVers[HW_TID_IDX]>=3))return true;
    return targetIDAttached[npc];
}
i16 TargetIDGetText(u16 npc) { if(npc>=World.instCount)return -1; Entity* e=&World.instances[npc]; if(e->tranquilizeFinished>World.pauseRelativeTime)return 536; if(targetIDText[npc]&&targetIDTextFinished[npc]<=World.pauseRelativeTime)targetIDText[npc]=0; return targetIDText[npc]?targetIDText[npc]:-1; }
void CreateTargetIDInstance(float damage,u16 hitIdx,float tranq) {
    if(hitIdx==WORLD||hitIdx>=World.instCount)return; Entity* npc=&World.instances[hitIdx]; if(!(npc->entflags&EF_ACTIVE)||!IdxIsNPC(npc->index)||npc->health<=0.0f)return;
    bool hw=(World.invP1.hasHardware&HW_TID)!=0; if(!hw&&tranq<=0.0f&&damage>0.0f)return; if(V3_Dist(World.position[hitIdx],World.position[PLAYER1])>TargetIDGetTetherRange())return;
    targetIDAttached[hitIdx]=true; targetIDAttachedFinished[hitIdx]=World.pauseRelativeTime+(hw?9999999.0:vmax(1.0f,tranq));
    if(tranq>0.0f){targetIDText[hitIdx]=536;targetIDTextFinished[hitIdx]=World.pauseRelativeTime+tranq;} else if(damage>=0.0f)TargetIDSendDamageReceive(hitIdx,damage,Att_None);
}
// PlayerEnergy
static const float  hwDrain[12][4] = {[3]={0.01535f,0.03413f,0.02559f,0.0f},[5]={0.04096f,0.10239f,0.17919f,0.05119f},[6]={0.001706f,0.0f,0.0f,0.0f},[7]={0.02559f,0.04266f,0.05119f,0.0f},[9]={0.0f,0.02f,0.015f,0.0f},[11]={0.08533f,0.0f,0.0f,0.0f},};
static const u16 hwDrainJPM[12][4] = {[3]={9,20,15,0},[5]={24,60,105,30},[6]={1,0,0,0},[7]={15,25,30,0},[9]={0,16,12,0},[11]={50,0,0,0},};
bool ModRequestsGrayscale() { return ((World.invP1.hasHardware & HW_INF) && (World.invP1.hardwareIsActive & HW_INF) > 0); }
static void DeactivateHardwareOnEnergyDepleted() { World.invP1.hardwareIsActive = 0; }
void TakeEnergy(float take) { if (World.invP1.energy <= 0.0f || Cheats.redbull) {return;} World.invP1.energy -= take; if (World.invP1.energy <= 0.0f) { World.invP1.energy = 0.0f; play_wav(sounds[84],AppliedFXVol(1.0f),(V3){0.0f,0.0f,0.0f},false);/*energy_gone*/ CenterStatusPrint("%s",Sys_Text.stringTable[314]); /*Power supply exhausted.*/ DeactivateHardwareOnEnergyDepleted(); } }
void GiveEnergy(float give,EnergyType type) { World.invP1.energy += give; if (World.invP1.energy > 255.0f) {World.invP1.energy = 255.0f;} if (type == EnergyType_Battery){play_wav(sounds[79],AppliedFXVol(1.0f),(V3){0.0f,0.0f,0.0f},false);/*batteryuse*/} else if (type == EnergyType_ChargeStation){play_wav(sounds[100],AppliedFXVol(1.0f),(V3){0.0f,0.0f,0.0f},false);/*chargingstation*/} }
void PlayerEnergyInit() { World.invP1.energy = 54.0f; World.invP1.energyDrainTickFinished = World.pauseRelativeTime + 0.1 + random_range(0.0f,1.0f); World.invP1.drainJPM = 0; }
void PlayerEnergyUpdate() {
    if (World.invP1.energyDrainTickFinished > World.pauseRelativeTime) return; World.invP1.energyDrainTickFinished = World.pauseRelativeTime + 0.1; bool anyDrain = false; u8 ver; World.invP1.drainJPM = 0;
    for (int hw=3;hw<=11;++hw) { u16 bit=(u16)(1u << hw); if (!(World.invP1.hardwareIsActive & bit) || hw == 4 || hw == 8 || hw == 10) continue;/*No energy usage*/ ver=World.invP1.hwVersSetting[hw]; float drain=hwDrain[hw][ver];  World.invP1.drainJPM += hwDrainJPM[hw][ver]; if (drain > 0.0f) { TakeEnergy(drain); anyDrain = true; } }
    if (anyDrain && World.invP1.energy <= 0.0f) { DeactivateHardwareOnEnergyDepleted(); World.invP1.drainJPM = 0; } // Depleted
}
// GeneralInventory
void MFD_ShowGeneralItem(),MFD_GeneralChanged();
int GeneralInvItem(int slot) {
    if (slot<0 || slot>=14) return -1;
    if (!slot) return 81;
    int item=World.invP1.generalInventoryIndexRef[slot]; if (item<0) return -1;
    item=UseableFromConst(item); return IdxIsGenericItem(item+307)?item:-1;
}
bool GeneralInvCanUse(int slot) { int item=GeneralInvItem(slot); return slot>0 && (item==52 || item==53 || item==55); }
bool GeneralInvCanVaporize(int slot) { int item=GeneralInvItem(slot); return slot>0 && item>=0 && (item<6 || item==33 || item==35 || item==58 || item==62); }
void GeneralInvRemove(int slot) {
    if (slot<=0 || slot>=14) return;
    World.invP1.generalInventoryIndexRef[slot]=-1; World.invP1.generalInvCustIdx[slot]=U16_MAX; MFD_GeneralChanged();
    if (World.invP1.generalInvCurrent!=slot) return;
    World.invP1.generalInvCurrent=0;
    for (int step=1;step<=14;++step) { int next=(slot-step+14)%14; if (GeneralInvItem(next)>=0) { World.invP1.generalInvCurrent=(u8)next; break; } }
}
void GeneralInvClick(int buttonIdx,int customIdx) {
    (void)customIdx; int item=GeneralInvItem(buttonIdx); if (item<0) return;
    World.invP1.generalInvCurrent=(u8)buttonIdx; World.invP1.generalInvIndex=(u16)item;
    World.mouseClickHeldOverGUI=World.Sys_UI.mouseClickHeldOverGUI=World.uiIsBlocking=true; MFD_ShowGeneralItem();
}
void GeneralInvApply(int buttonIdx,int customIdx) {
    (void)customIdx; if (!GeneralInvCanUse(buttonIdx) || World.invP1.holdingObject) return;
    int item=GeneralInvItem(buttonIdx);
    if ((item==55?World.instances[PLAYER1].health:World.invP1.energy)>=255.0f) { CenterStatusPrint("%s",Sys_Text.stringTable[303]); return; }
    if (item==55) World.instances[PLAYER1].health=255.0f; else GiveEnergy(item==52?83.0f:255.0f,EnergyType_Battery);
    GeneralInvRemove(buttonIdx);
}
void GeneralInvDoubleClick(int buttonIdx,int customIdx) { GeneralInvClick(buttonIdx,customIdx); GeneralInvApply(buttonIdx,customIdx); }
void GeneralInventoryActivate() { int slot=World.invP1.generalInvCurrent; if (slot<14) GeneralInvApply(slot,World.invP1.generalInvCustIdx[slot]); }
bool GeneralInvTake(int slot) {
    int item=GeneralInvItem(slot); if (slot<=0 || item<0 || World.invP1.holdingObject) return false;
    ResetHeldItem(); World.invP1.heldObjectIndex=(u16)(item+307); World.invP1.heldObjectCustIdx=World.invP1.generalInvCustIdx[slot]; World.invP1.holdingObject=true;
    GeneralInvRemove(slot); ForceInventoryMode(); CenterStatusPrint("%s%s",Sys_Text.stringTable[item+326],Sys_Text.stringTable[319]); return true;
}
static bool GrenadeIsNPCMine(u16 self) { return World.layer[self] != L_PlayerBullets; }
void ApplyImpactForce(u16 target, float vel, V3 normal, V3 pt) {
    if (target == WORLD || target >= World.instCount || vel <= 0.0f){return;} Entity* e = &World.instances[target]; if((e->entflags & EF_DEAD) || (!(e->entflags & EF_RIGIDBODY) && target != PLAYER1)){return;}
    V3 n = V3_Normalize(normal); if (V3_Mag(n) < 0.0001f) {n = (V3){0.0f,1.0f,0.0f};/*At least make it pop off the floor*/} AddForce(target,V3_ScaleByF(n,vel),true); V3 lever = V3_AsubB(pt, World.position[target]); World.angularVelocity[target].x += lever.y * vel * 0.05f; World.angularVelocity[target].z += -lever.x * vel * 0.05f; (void)pt; // torque applied relative to point lever arm
}

void ApplyImpactForceSphere(DamageData* dd, V3 center, float radius, float baseVel) { 
    if (radius <= 0.0f || baseVel <= 0.0f) return; float r2 = radius * radius;
    for (u16 i = INSTS_1ST_IDX; i < World.instCount; i++) {
        Entity* e = &World.instances[i]; if (!(e->entflags & EF_ACTIVE) || (e->entflags & EF_DEAD)) continue; if (!(e->entflags & EF_RIGIDBODY) && !IdxIsNPC(e->index) && i != PLAYER1) continue; float sqd = V3_SqDist(World.position[i], center); if (sqd > r2) continue; float dist = vsqrtf(sqd); float falloff = 1.0f - (dist / radius); if (falloff <= 0.0f) continue;
        V3 normal; if(dist > 0.0001f){normal=V3_ScaleByF(V3_AsubB(World.position[i],center), 1.0f / dist);}else{normal = (V3){0.0f,1.0f,0.0f};} ApplyImpactForce(i,baseVel * falloff,normal,World.position[i]); if (dd && dd->damage > 0.0f && i != dd->owner) { DamageData splash=*dd; splash.damage = dd->damage * falloff; splash.hitIdx = i; splash.hitpoint=World.position[i]; splash.attacknormal=normal; TakeDamage(i,splash); }
    }
}

void SpawnExplosionEffect(V3 pos, int explosionType) { static const u16 prefabs[6] = {729,730,731,732,733,734}; int idx = (explosionType >= 0 && explosionType < 6) ? explosionType : 2; u16 fx = SpawnDynamicObject(prefabs[idx], false); if (fx == WORLD || fx == U16_MAX) return; World.position[fx] = pos; Entity* e = &World.instances[fx]; flag_set(&e->entflags, EF_ACTIVE, true); if (e->delay <= 0.0f) e->delay = 0.8f; e->delayFinished = World.pauseRelativeTime + e->delay; }
void GrenadeExplode(u16 self) {
    Entity* e = &World.instances[self]; DamageData dd={.damage=e->damage,.penetration=e->strength,.offense=e->speed,.armorvalue=0.0f,.defense=0.0f,.impactVelocity=e->damage*1.5f,.attacknormal=(V3){0.0f,1.0f,0.0f},.hitpoint=World.position[self],.attackType=e->attackType,.owner=e->recentMostActivator,.hitIdx=WORLD,.isOtherNPC=false,.berserkActive=(World.invP1.patchActive & PATCH_BERSERK) != 0};
    i16 idx=GrenadeTypeFromConst(e->index); float radius=(idx>=7&&idx<=13) ? grenadeRadius[idx-7] : (e->strength>0.0f ? e->strength : 4.0f); ApplyImpactForceSphere(&dd,World.position[self],radius,e->damage * 1.5f); if (!GrenadeIsNPCMine(self)) { World.invP1.noiseFinished = World.pauseRelativeTime + 2.0; } int soundIndex=60,explosionType=2;
    switch (idx) {case 7: case 11: soundIndex = 64; World.fogFac += 5; explosionType = 1; break;/*frag, mine*/ case 8: case 10: soundIndex = 60; World.fogFac += 7; explosionType = 2; break;/*conc, earth*/ case 9:  soundIndex = 67; explosionType = 4; break;/*emp*/ case 12: soundIndex = 60; World.fogFac += 6;  explosionType = 2; break;/*nitro*/ case 13: soundIndex = 63; World.fogFac += 10; explosionType = 3; break;/*gas*/}
    play_wav(SoundPath(soundIndex),1.0f,World.position[self],true); SpawnExplosionEffect(World.position[self],explosionType); Shake(-1.0f); DeleteInstance(self);
}

void GrenadeActivate(u16 self) { i16 idx=GrenadeTypeFromConst(World.instances[self].index); if (idx == 10){World.instances[self].timerFinished=World.pauseRelativeTime + World.invP1.earthShakerTimeSetting;} if (idx == 12){World.instances[self].timerFinished=World.pauseRelativeTime + World.invP1.nitroTimeSetting;} }
void GrenadeUpdate(u16 self) { Entity* e = &World.instances[self]; i16 idx=GrenadeTypeFromConst(e->index); if(idx == 14){GrenadeExplode(self); return;} /*Plastique*/ if((idx == 10 || idx == 12) && e->timerFinished < World.pauseRelativeTime) { GrenadeExplode(self); return; } if (idx == 11) { V3 origin = World.position[self]; float pr=grenadeRadius[idx-7]; for (u16 i = PLAYER1; i < World.instCount; i++) { Entity* o = &World.instances[i]; if (i == self || !(o->entflags & EF_ACTIVE) || (o->entflags & EF_DEAD)) continue; if (i != PLAYER1 && !IdxIsNPC(o->index)) continue; if (V3_SqDist(World.position[i], origin) < (pr * pr)) { GrenadeExplode(self); return; } } } }
void GrenadeOnCollision(u16 self) { i16 idx=GrenadeTypeFromConst(World.instances[self].index); if ((idx >= 7 && idx <= 9) || idx == 13) GrenadeExplode(self); }
float GetDamageTakeAmount(DamageData* dd) { if (!dd) return 0.0f; float take = dd->damage; if (take <= 0.0f) return 0.0f; if (dd->berserkActive) take *= BERSERK_DAMAGE_MULTIPLIER; if (dd->defense > 0.0f && dd->offense < dd->defense) { float r = (dd->defense - dd->offense) / dd->defense; if (r > 0.85f) r = 0.85f; take *= (1.0f - r); } if (dd->armorvalue > 0.0f && dd->penetration < dd->armorvalue) { float a = (dd->armorvalue - dd->penetration) / dd->armorvalue; if (a > 0.85f) a = 0.85f; take *= (1.0f - a); } if (take < 0.0f) take = 0.0f; return take; }
void SpawnImpactEffect(u16 impactType, V3 pos) { if (impactType == 0 || impactType == U16_MAX) return; u16 fx = SpawnDynamicObject(impactType, false); if (fx == WORLD || fx == U16_MAX) return; World.position[fx] = pos; Entity* e = &World.instances[fx]; flag_set(&e->entflags, EF_ACTIVE, true); if (e->itemLifeTime <= 0.0f) e->itemLifeTime = 1.0f; e->delayFinished = World.pauseRelativeTime + e->itemLifeTime; }
void ExitCyberspace(void) { UIExitCyberspace(); if (World.curLev != LEVEL_CYBERSPACE) return; if (World.instances[PLAYER1].cyberHealth <= 0.0f) World.instances[PLAYER1].cyberHealth = 1.0f; LoadLevel(World.startLevel < World.numLevels ? World.startLevel : 0, (V3){0.0f,0.0f,0.0f}); }
void ReduceCurrentLevelSecurity(SecurityType stype) { // Typical level: 4 CPU nodes. 20 cameras, 100% = 4x + 20y.  Assuming that a good camera percentage is 2-3%, CPU % would be about 10-15 each
    u8 lev = World.curLev; if (lev >= 14 || stype == SecurityType_None) return; const float camScore=4.0f, nodeSmallScore=10.0f, nodeLargeScore=27.0f; float total = (World.levelCameraCount[lev]*camScore)+(World.levelSmallNodeCount[lev]*nodeSmallScore)+(World.levelLargeNodeCount[lev]*nodeLargeScore); if (total <= 0.0f) return; float drop = camScore;
    switch(stype){case SecurityType_Camera:drop=(camScore/total)*100.0f; if(World.levCamDestroyedCnt[lev]<255)World.levCamDestroyedCnt[lev]++; break; case SecurityType_NodeSmall:drop=(nodeSmallScore/total)*100.0f; if (World.levSmNodeDestroyedCnt[lev]<255)World.levSmNodeDestroyedCnt[lev]++; break; case SecurityType_NodeLarge:drop=(nodeLargeScore/total)*100.0f; if(World.levNodeDestroyedCnt[lev]<255) World.levNodeDestroyedCnt[lev]++; break; default:return;}
    int cur=(int)World.levelSecurity[lev]-(int)drop; if (cur<0) cur=0; World.levelSecurity[lev]=(u8)cur; if (World.levCamDestroyedCnt[lev]==World.levelCameraCount[lev] && World.levSmNodeDestroyedCnt[lev]==World.levelSmallNodeCount[lev] && World.levNodeDestroyedCnt[lev]==World.levelLargeNodeCount[lev]) World.levelSecurity[lev]=0; CenterStatusPrint("%s%d%s", Sys_Text.stringTable[306], (int)World.levelSecurity[lev], Sys_Text.stringTable[307]);
}
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static void ProjectileEffectImpactOnCollision(u16 self,u16 hitIdx, V3 hitPos,V3 hitNormal) {
    Entity* e = &World.instances[self]; if (hitIdx == e->recentMostActivator) return; e->counter++; DamageData dd={.damage=e->damage,.penetration=e->strength,.offense=e->speed,.armorvalue=0.,.defense=0,.impactVelocity=e->damage*1.5f,.attacknormal=hitNormal,.hitpoint=hitPos,.attackType=e->attackType,.owner=e->recentMostActivator,.hitIdx=hitIdx,.isOtherNPC=IdxIsNPC(World.instances[hitIdx].index),.berserkActive=(World.invP1.patchActive & PATCH_BERSERK)!=0};
    Entity* hit = &World.instances[hitIdx]; if (IdxIsNPC(hit->index)) { NPCTable* nt = &npcTable[hit->index - 419]; dd.armorvalue = nt->armorvalue; dd.defense = nt->defense; } if (e->lookUpIndex == 5) { ApplyImpactForceSphere(&dd, World.position[self], 3.2f, 1.0f); World.fogFac += 4; }/*Railgun sphere impact*/
    if (hit->health > 0.0f || hit->cyberHealth > 0.0f) {
        dd.damage = GetDamageTakeAmount(&dd); if (e->counter < e->countToTrigger) dd.damage *= 0.85f;/*per-hit falloff*/ dd.impactVelocity = dd.damage * 1.5f; if (e->counter > 0) dd.impactVelocity /= 3.0f; if (World.curLev != LEVEL_CYBERSPACE && e->recentMostActivator == PLAYER1) { ApplyImpactForce(hitIdx,dd.impactVelocity,dd.attacknormal,hitPos); } float dmgFinal = TakeDamage(hitIdx,dd); float tranq=-1.0f;
        if (dd.isOtherNPC) { if(!(hit->entflags & EF_ASLEEP)){World.Sys_Music.inCombat=true;} if(dd.attackType == Att_Trnq){float stunAmount=vclamp(3.0f+(World.invP1.stungunSetting/100.0f)*7.0f,3.0f,10.0f); tranq=Tranquilize(hitIdx,stunAmount,true);} } if (dmgFinal < 0.0f) {dmgFinal = 0.0f;} CreateTargetIDInstance(dmgFinal,hitIdx,tranq); SpawnImpactEffect(GetImpactType(hitIdx),hitPos);
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
    if (World.instances[self].entflags & EF_TELEPORT_ON_DEATH) {return;} flag_set(&World.instances[self].entflags,EF_TELEPORT_ON_DEATH,true); World.col[self] = COLTYPE_NONE; World.gravity[self] = 0.0f; World.velocity[self] = (V3){0,0,0}; World.angularVelocity[self] = (V3){0,0,0}; World.instances[self].modelIndex = U16_MAX; 
    V3 fxPos = World.position[self]; if(World.col[self] != COLTYPE_NONE){fxPos=V3_AplusB(fxPos,World.colliderCenter[self]);} SpawnImpactEffect(735,fxPos); play_wav(sounds[106],1.0f,fxPos,false);
}

static void DropSearchables(u16 self) {for(int i=0;i<4;i++){if(World.instances[self].contents[i]<=-1){continue;} u16 spawned=SpawnDynamicObject(World.instances[self].contents[i]+307,true); if(spawned!=U16_MAX){World.position[spawned]=World.position[self]; World.instances[spawned].custIdx[0]=World.instances[self].custIdx[i];}else{CenterStatusPrint("BUG: Failed to make search obj.");} World.instances[self].contents[i]=World.instances[self].custIdx[i]=-1;}}
static void CreateDeathEffects(u16 self,u16 fxPoolType) { if (fxPoolType == 0) {return; /*PoolType_None*/} V3 pos = World.position[self]; if (World.col[self] != COLTYPE_NONE) { pos = V3_AplusB(pos,World.colliderCenter[self]); } SpawnImpactEffect(fxPoolType, pos); }
static void HideSelf(u16 self) { if (World.instances[self].index == 279) {return; /*tv screens keep mesh visible*/} World.instances[self].modelIndex = MAX_MDLS; World.gravity[self] = 0.0f; }
static void SpawnSecCpuNodeGibs(u16 self) {
    if (World.instances[self].index != 478) return;
    V3 pos = World.position[self]; Quaternion rot = World.rotation[self];
    for (u16 gibConst = 840; gibConst <= 853; ++gibConst) {
        u16 gib = SpawnDynamicObject(gibConst, false);
        if (gib == U16_MAX || gib >= World.instCount || gib == self) continue;
        World.position[gib] = pos; World.rotation[gib] = rot;
        World.velocity[gib] = (V3){0.0f, World.velocity[self].y, 0.0f};
        World.gravity[gib] = 1.0f; World.layer[gib] = L_Corpse;
    }
}
static void NPCDeath(u16 self) { if (World.instances[self].entflags & EF_DEAD_CHECKS_DONE) {return;} flag_set(&World.instances[self].entflags,EF_DEAD_CHECKS_DONE,true); CreateDeathEffects(self,World.instances[self].deathBurst); if (World.instances[self].index == 419) play_wav(sounds[64],1.0f,World.position[self],true);/*npc_autobomb: explosion1*/ if (npcTable[World.instances[self].index - 419].type == NPCType_Cyber) DeleteInstance(self); }
static void ObjectDeath(u16 self) {
    Entity* e = &World.instances[self]; if (World.instances[self].entflags & EF_DEAD_CHECKS_DONE) return;
    if (World.instances[self].entflags & EF_DEATH_BURST_DONE) { CreateDeathEffects(self,World.instances[self].deathBurst); DropSearchables(self); if (World.instances[self].index != 279){World.col[self]=COLTYPE_NONE;} HideSelf(self); } else { World.col[self] = COLTYPE_NONE; DropSearchables(self); CreateDeathEffects(self,World.instances[self].deathBurst); }
    flag_set(&World.instances[self].entflags,EF_DEAD_CHECKS_DONE,true); World.instances[self].automapHidden = true;
    if (World.instances[self].securityThreshold > 0) { SecurityType stype = SecurityType_None; if(World.instances[self].index == 477){stype=SecurityType_Camera;}else if(World.instances[self].index == 479){stype=SecurityType_NodeSmall;} else if(World.instances[self].index == 478){stype=SecurityType_NodeLarge;} if(stype != SecurityType_None){ReduceCurrentLevelSecurity(stype);} }
    u16 idx = World.instances[self].index; SpawnSecCpuNodeGibs(self); play_wav(SoundPath((idx < 527 && objectDeathSound[idx] != 0) ? objectDeathSound[idx] : 62/*crate_break*/),1.0f,World.position[self],true); if(e->deathBurst != 0){HideSelf(self);}
}

static void ScreenDeath(u16 self) { Entity* e=&World.instances[self]; if(e->entflags & EF_DEAD_CHECKS_DONE){return;} flag_set(&e->entflags,EF_DEAD_CHECKS_DONE,true); play_wav(sounds[69],1.0f,World.position[self],true);/*screen_destroy*/ if (e->entflags & EF_DEATH_BURST_DONE) ObjectDeath(self);/*gib path*/ }
static void VaporizeCorpse(u16 self,bool energyVaporized) { Entity* e=&World.instances[self]; flag_set(&e->entflags,EF_DEAD_CHECKS_DONE,true); DropSearchables(self); e->modelIndex=MAX_MDLS; if (IdxIsNPC(e->index) || IdxIsSearchable(e->index)) DeleteInstance(self); CreateDeathEffects(self,energyVaporized ? 2 : ((e->deathBurst == 0) ? 1/*Corpse hit fallback*/ : e->deathBurst)); }
static inline bool IsGrenade(u16 i) { return ((i >= 314 && i <= 320) || i == 370 || i == 372 || i == 387 || i == 389 || (i >= 402 && i <= 404)); }
static void Death(u16 self,bool energyVaporized) {
    Entity* e = &World.instances[self]; if (e->entflags & EF_DEAD_CHECKS_DONE) return; UseDeathTargets(self); bool isNPC = IdxIsNPC(e->index); bool isObj = IdxIsDynamicObject(e->index); if (e->entflags & EF_ACT_AS_CORPSE_ONLY) { e->entflags |= EF_DEAD_CHECKS_DONE; return; }
    if (e->index == 477) { /* sec_camera has no dynamic-object path; deathFX 1 is CameraExplosions. */
        e->deathBurst = 725; ObjectDeath(self); DeleteInstance(self); return;
    }
    /* NPCs retain their entity mesh for AI death animation. Only non-NPC corpses vaporize. */
    bool vaporize=IdxIsCorpse(e->index); bool isGrenade=IsGrenade(e->index), doTeleport=(e->entflags & EF_TELEPORT_ON_DEATH) != 0; if (e->iceActive) World.col[self] = COLTYPE_NONE;
    if (vaporize && e->index != 477/*sec_camera*/ && !isGrenade) VaporizeCorpse(self,energyVaporized); else if (isObj && !isNPC) ObjectDeath(self); else if (e->index == 279/*screen*/) ScreenDeath(self); else if (doTeleport) TeleportAway(self); else if (isGrenade) GrenadeExplode(self);
    if (isNPC && !doTeleport) NPCDeath(self); else if (self == PLAYER1) { if (!RessurectPlayer()) World.deaths++; } flag_set(&e->entflags,EF_DEAD_CHECKS_DONE,true);
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
                if (take < thresh) absorb = 1.0f; if (absorb > 0.0f) { if (absorb < 1.0f) absorb = vclamp(absorb + random_range(-0.08f,0.08f),0.0f,1.0f); take *= (1.0f - absorb); play_wav(sounds[94],AppliedFXVol(1.0f),(V3){0.0f,0.0f,0.0f},false);/*shield absorb*/ int abs = (int)(absorb * 100.0f); CenterStatusPrint("%s%d%s",Sys_Text.stringTable[208],abs,Sys_Text.stringTable[209]); }
            } if (take > 0.0f && (absorb < 0.4f || random_range(0.0f,1.0f) < 0.5f)) { play_wav(sounds[140],AppliedFXVol(1.0f),(V3){0.0f,0.0f,0.0f},false); World.painStaticAlpha = take > 15.0f ? 1.0f : take > 10.0f ? 0.8f : 0.3f; }
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
static Color3 lantCol = (Color3){1.0f,1.0f,1.0f}; static float lanternVersionBrightness[3] = {0.875f,1.4f,1.75f}; static const float SIGHT_LIGHT_INTENSITY=0.36f,SIGHT_LIGHT_RANGE=75.8f;/*Citadel sightLight: white spotlight, intensity 0.36, range 75.7961*/
void HardwareUpdate() {
    bool infraredOn = (World.invP1.hasHardware & HW_INF) && (World.invP1.hardwareIsActive & HW_INF) > 0, lanternOn = (World.invP1.hasHardware & HW_LAN) && (World.invP1.hardwareIsActive & HW_LAN) > 0;
    bool sightOn = (World.invP1.patchActive & PATCH_SIGHT) && World.invP1.sightFinishedTime != -1.0;/*Sight patch main-effect window only, not the side-effect window*/
    if (lanternOn || infraredOn || sightOn) { // Headmounted light shared by lantern/infrared/sight: intensities stack, range takes the max of the active effects
        V3 ppos = World.position[PLAYER1]; lanternPos = (V3){ppos.x + 0.04f,ppos.y + 0.24f,ppos.z + 0.04f}; float intensity = 0.0f, range = 0.0f;
        if (infraredOn) { intensity += 0.8f; range = vmax(range,50.35f); }
        if (lanternOn) { intensity += lanternVersionBrightness[vclamp(World.invP1.hwVersSetting[7],0,2)]; range = vmax(range,11.52f); }
        if (sightOn) { intensity += SIGHT_LIGHT_INTENSITY; range = vmax(range,SIGHT_LIGHT_RANGE); }
        UpdateLight(headmountedLanternLight,lanternPos,lantCol,range,intensity,intensity,0.0f,0.0f,QUAT_IDENTITY,true,true);
    } else UpdateLight(headmountedLanternLight,lanternPos,lantCol,11.52f,0.0f,0.0f,0.0f,0.0f,QUAT_IDENTITY,false,false);
}
// Dermal Patches
void PatchDisableAll(void){World.invP1.berserkFinished=World.invP1.berserkIncTime=World.invP1.detoxFinished=World.invP1.geniusFinished=World.invP1.mediFinished=World.invP1.reflexFinishedTime=World.invP1.sightFinishedTime=World.invP1.sightSideEffectFinishedTime=World.invP1.staminupFinishedTime=-1.0; World.invP1.mediPatchPulseFinished=0.0; World.invP1.mediPatchPulseCount=0; World.invP1.staminupActive=World.geniusActive=false; World.invP1.fatigue=0.0f; World.invP1.berserkIncrement=World.invP1.patchActive=0; World.timeScale=DEFAULT_TIME_SCALE;}
void PatchUpdate() {
    if (World.invP1.patchActive & PATCH_DETOX) { if (World.invP1.detoxFinished < World.pauseRelativeTime) World.invP1.patchActive -= PATCH_DETOX; else World.instances[PLAYER1].radiation = 0.0f; } // Detox: other patches wiped on use only; radiation zeroed every frame while active
    if (World.invP1.patchActive & PATCH_MEDI) { // Medi
        if (World.invP1.mediPatchPulseFinished == 0.0) World.invP1.mediPatchPulseCount = 0;
        if (World.invP1.mediPatchPulseFinished < World.pauseRelativeTime) {
            World.instances[PLAYER1].health = vmin(255.0f, World.instances[PLAYER1].health + 8.0f);
            World.invP1.mediPatchPulseFinished = World.pauseRelativeTime + (0.5 + World.invP1.mediPatchPulseCount * 0.5);
            World.invP1.mediPatchPulseCount++;
        }
        if (World.invP1.mediFinished < World.pauseRelativeTime && World.invP1.mediFinished != -1.0) { World.invP1.patchActive -= PATCH_MEDI; World.invP1.mediFinished = -1.0; World.invP1.mediPatchPulseFinished = 0.0; World.invP1.mediPatchPulseCount = 0; }
    } else {
        World.invP1.mediPatchPulseFinished = 0.0; World.invP1.mediPatchPulseCount = 0;
    }
    if (World.invP1.patchActive & PATCH_REFLEX) { if (World.invP1.reflexFinishedTime < World.pauseRelativeTime && World.invP1.reflexFinishedTime != -1.0){ World.invP1.patchActive-=PATCH_REFLEX; World.invP1.reflexFinishedTime=-1.0; World.timeScale=DEFAULT_TIME_SCALE;}else{World.timeScale=REFLEX_TIME_SCALE;}}else{if(World.timeScale != DEFAULT_TIME_SCALE){World.timeScale=DEFAULT_TIME_SCALE;}}//Reflex (tracked in game-time: pauses with the game and persists through save/load)
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
    if (e->requiredAccessCard != ACC_None) { if (!DoorInventoryHasAccessCard(e->requiredAccessCard)) {CenterStatusPrint("%s%s",AccessCardCodeForType(e->requiredAccessCard),Sys_Text.stringTable[2]); play_wav(sounds[467],0.7f,World.position[self],true); return;} else {e->requiredAccessCard = ACC_None;}}
    if ((e->entflags & EF_LOCKED) != 0) { CenterStatusPrint("%s",Sys_Text.stringTable[e->lockedMessageLingdex]); play_wav(sounds[467],0.55f,World.position[self],true); return; }  if ((e->onlyTargetOnce && !e->targetAlreadyDone) || !e->onlyTargetOnce) { e->targetAlreadyDone = true; UseTargets(self,e->targetIdx); } if (e->ajar) e->ajar = false; DoorActuate(self);
}

void DoorTargetted(u16 self, u16 activator) { if ((World.instances[self].entflags & EF_LOCKED) != 0) EntitySetLocked(&World.instances[self],false); if (!World.instances[self].targettingOnlyUnlocks) DoorUse(self,activator); }
void DoorUpdate(u16 self) {
    Entity* e = &World.instances[self]; if(e->ajar){return;} AnimationClip opening=DoorGetClip(e,A_OPENING), closing=DoorGetClip(e,A_CLOSING);
    if (e->doorOpen == DoorState_Opening && e->clip == A_OPENING && e->frame >= opening.frameEnd) { e->doorOpen = e->doorState = DoorState_Open; ChangeAnim(e,A_IDLE_OPEN); } else if (e->doorOpen == DoorState_Closing && e->clip == A_CLOSING && e->frame >= closing.frameEnd) { e->doorOpen = e->doorState = DoorState_Closed; ChangeAnim(e,A_IDLE_CLOSED); } if (World.pauseRelativeTime > e->waitBeforeClose && e->doorOpen == DoorState_Open && !e->stayOpen && !e->startOpen) DoorClose(self);
}

void CloseFullmap() {}
u16 SpawnDynamicObject(int val, bool cheat) {
    if (!IdxInBounds(val)) { DualLogError("Const index out of bounds: %u", val); return 0xFFFF; } if (IdxIsGeometry(val) && !Cheats.editMode) { CenterStatusPrint("Indices 0 to 306 (level chunks)\nnot possible when not on edit mode!"); return 0xFFFF; } (void)cheat;
    if (World.instCount >= INSTANCE_COUNT) { DualLogError("Failed to spawn constIndex %u: instance table full (%u/%u)",val,World.instCount,INSTANCE_COUNT); return 0xFFFF; } u16 entityIndexInInstanceTable = AddInstance((u16)val, (V3){0.0f,0.0f,0.0f}); return entityIndexInInstanceTable;
}
// TargetIO: Full game cross-level target handling.  Iterates all loaded levels, temporarily swaps active pointers via SetLevelPointers(), finds matching targetname(s), and calls Targetted().  Activator from cur level. Recursion is safe via targetIOActive flag.
void TriggerTargetted(u16 self, u16 activator) { UseTargets(activator, World.instances[self].targetIdx); }
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
    if (aioflags & TARG_IOFLAGS_INST_ACTIVATE) flag_set(&e->entflags, EF_ACTIVE, true); else if (aioflags & TARG_IOFLAGS_INST_DEACTIVATE) { if (e->camView != 255) { e->camView = 255; TextureSequenceInit(self, "Static"); flag_set(&e->entflags, EF_ACTIVE, true); }/*camera destroyed: keep its screen, switch it to Static*/ else { flag_set(&e->entflags, EF_ACTIVE, false); } } else if (aioflags & TARG_IOFLAGS_INST_TOGGLE) flag_set(&e->entflags, EF_ACTIVE, !(e->entflags & EF_ACTIVE));
}

extern char ioNames[MAX_IO_NAMES][TARG_STRLEN]; extern u16 ioNameCount;
void UseTargets(u16 activator, u16 targetIdx) {
    if(targetIdx==IO_NONE){return;} bool wasActive=World.targetIOActive,succeeded=false; u8 entryLevel=World.currentLevel; if(!wasActive){World.targetIOActive=true; World.targetIOEntryLevel=entryLevel; World.targetIOActivatorIdx=activator; World.targetIOActivatorEntity=World.instances[activator]; World.targetIOActivatorIoflags=World.instances[activator].ioflags;} const char* targetname=(targetIdx<ioNameCount) ? ioNames[targetIdx] : "";
    for (u8 lev = 0; lev < World.numLevels; ++lev) { if (World.currentLevel != lev) SetLevelPointers(lev); for (u16 i = INSTS_1ST_IDX; i < World.instCount; ++i) { if (World.instances[i].targetnameIdx != targetIdx) {continue;} Targetted(activator,i); succeeded=true; } }
    if (World.currentLevel != entryLevel) {SetLevelPointers(entryLevel);} if (!succeeded) {DualLogWarn("No target found: %s\n",targetname);} if (!wasActive) {World.targetIOActive=false;}
}
// Frob/Use
#define FROB_DISTANCE 4.9f
void MFD_OpenSearch(bool isRH),MFD_CloseSearch(void);
void CloseSearch(void) {
    u16 s=World.Sys_UI.tetheredSearchable;
    if (s>=INSTS_1ST_IDX && s<World.instCount) World.instances[s].srchInUse=false;
    World.Sys_UI.tetheredSearchable=World.invP1.currentSearchItem=U16_MAX;
    if (s>=INSTS_1ST_IDX && s<World.instCount) World.Sys_UI.usingObject=false;
    World.Sys_UI.searchFXActive[0]=World.Sys_UI.searchFXActive[1]=false; MFD_CloseSearch();
}

void UpdateSearchTether(void) {
    u16 s=World.Sys_UI.tetheredSearchable; if (s==U16_MAX) return;
    if (s<INSTS_1ST_IDX || s>=World.instCount || !(World.instances[s].entflags&EF_ACTIVE) || !World.instances[s].srchInUse) { CloseSearch(); return; }
    V3 d=V3_AsubB(World.position[PLAYER1],World.position[s]);
    if (V3_dot(d,d)>(FROB_DISTANCE+0.16f)*(FROB_DISTANCE+0.16f)) CloseSearch();
}

bool SearchTakeSlot(u8 slot) {
    UpdateSearchTether(); u16 s=World.Sys_UI.tetheredSearchable;
    if (s==U16_MAX || slot>=4 || World.invP1.holdingObject) return false;
    Entity* e=&World.instances[s]; i16 item=e->contents[slot]; if (item<0 || item>110 || !IdxIsUsableObject((u16)(item+307))) return false;
    ResetHeldItem(); World.invP1.heldObjectIndex=(u16)(item+307); World.invP1.heldObjectCustIdx=(u16)e->custIdx[slot]; World.invP1.holdingObject=true;
    e->contents[slot]=e->custIdx[slot]=-1; ForceInventoryMode();
    CenterStatusPrint("%s%s",Sys_Text.stringTable[item+326],Sys_Text.stringTable[319]);
    for (u8 i=0;i<4;++i) if (e->contents[i]>=0) return true;
    CloseSearch(); return true;
}

void SearchObject(int searchable) {
    UpdateSearchTether();
    if (searchable<INSTS_1ST_IDX || searchable>=World.instCount || !(World.instances[searchable].entflags&EF_ACTIVE)) return;
    Entity* e=&World.instances[searchable];
    static bool loggedSearch=false;
    if (!loggedSearch) {
        DualLog("SearchObject: instance=%d constIndex=%u tether=%u srchInUse=%d contents=[%d,%d,%d,%d] custIdx=[%d,%d,%d,%d]\n",searchable,(u32)e->index,(u32)World.Sys_UI.tetheredSearchable,(int)e->srchInUse,(int)e->contents[0],(int)e->contents[1],(int)e->contents[2],(int)e->contents[3],(int)e->custIdx[0],(int)e->custIdx[1],(int)e->custIdx[2],(int)e->custIdx[3]);
        loggedSearch=true;
    }
    if (World.Sys_UI.tetheredSearchable==searchable && e->srchInUse) {
        for (u8 slot=0;slot<4;++slot) if (e->contents[slot]>=0) { SearchTakeSlot(slot); return; }
        return;
    }
    CloseSearch(); World.Sys_UI.tetheredSearchable=World.invP1.currentSearchItem=(u16)searchable; e->srchInUse=true;
    World.Sys_UI.objectInUsePos=World.position[searchable]; World.Sys_UI.usingObject=true;
    MFD_OpenSearch(World.Sys_UI.lastSearchSideRH); SearchFXEnable(World.Sys_UI.lastSearchSideRH?1:0);
    play_wav(sounds[91],0.75f,(V3){0,0,0},false); ForceInventoryMode();
}
// Mission timer. Port of MissionTimer.cs (ScriptsTODO/MissionTimer.cs): Awake/UpdateToNextMission/Update.
// Display (minutes/seconds countdown + mission label) is derived from misTimerT/misTimerMission at render time (ui.c MissionTimer/MissionTimerT, still placeholders); only logic lives here.
void MissionTimerInit(void) { // Port of MissionTimer.Awake. Called on new game (see NewGame in voxen.c).
    World.misTimerT = 6000.0f; World.misTimerFinished = World.pauseRelativeTime + 1.0f;
    World.misTimerMission = 504; World.misTimerCurIdx = 0; World.misTimerLast = World.misTimerTimesUP = false;
}
void MissionTimerUpdateToNextMission(float newTimerAmount,int misTextIndex,int nextMissionIndex) {
    if (World.misTimerCurIdx == (u8)nextMissionIndex) return;
    // Unity also notifies QuestLogNotesManager here; that script is not ported (see ui_todo.md D2), so only the timer itself advances.
    if (World.diffMis < 3) return; // Don't update timer on lower skill settings.
    World.misTimerT = newTimerAmount; World.misTimerCurIdx = (u8)nextMissionIndex; World.misTimerMission = (u16)misTextIndex;
    if (World.misTimerCurIdx == 4) World.misTimerLast = true; // No gameover for last timer.
}
void MissionTimerUpdate(void) { // Port of MissionTimer.Update. Called from ModUpdate below.
    if (World.diffMis < 3) return;
    if (World.paused || World.menuActive) return;
    if (World.curLev == LEVEL_CYBERSPACE) return; // Timer doesn't count down in cyberspace.
    if (World.misTimerTimesUP) {
        if (World.instances[PLAYER1].health > 0.0f) {
            World.invP1.radiationArea = true; World.instances[PLAYER1].radiation += 0.1f; // Port of GiveRadiation(0.1f) every frame; feeds the existing rad-bleed in ModUpdate.
            return;
        }
    }
    if (World.misTimerT <= 0.0f) {
        if (World.misTimerLast) {
            World.misTimerMission = 509; // Unity shows countdown text 869 + label 509 here; the 869 countdown is render-side, the label index is stored.
            World.misTimerTimesUP = true;
            return;
        }
        // Unity calls PlayerHealth.PlayerDeathToMenu (instant mission-fail death to menu). No equivalent in Voxen, so route through the normal death flow instead (resurrection still applies).
        World.instances[PLAYER1].health = 0.0f; Death(PLAYER1,false);
        return;
    }
    switch (World.misTimerCurIdx) {
        case 0: if (QuestBitIsSet(QB_LaserDestroyed)) MissionTimerUpdateToNextMission(10800.0f,505,1); break;
        case 1: if (QuestBitIsSet(QB_AntennaNorthDestroyed) && QuestBitIsSet(QB_AntennaSouthDestroyed) && QuestBitIsSet(QB_AntennaEastDestroyed) && QuestBitIsSet(QB_AntennaWestDestroyed)) MissionTimerUpdateToNextMission(2700.0f,506,2); break;
        case 2: if (QuestBitIsSet(QB_SelfDestructActivated)) MissionTimerUpdateToNextMission(3000.0f,507,3); break;
        case 3: if (QuestBitIsSet(QB_BridgeSeparated)) MissionTimerUpdateToNextMission(2700.0f,506,4); break;
    }
    if (World.misTimerFinished < World.pauseRelativeTime) { World.misTimerT -= 1.0f; World.misTimerFinished = World.pauseRelativeTime + 1.0; }
}
static int UseNameTableIndex(int index) {
    switch (index) {
        case 0:return 925; case 1:return 926; case 2:return 54; case 3:return 54; case 4: return 54; case 5: return 54; case 6: return 54; case 7: return 54; case 8: return 54; case 9: return 54; case 10: return 54; case 11: return 55; case 12: return 57; case 13: return 58; case 14: return 59; case 15: return 928; case 16: return 61; case 17: return 929; case 18: return 62; case 19: return 63; case 20: return 927; case 23: return 82; case 24: return 930; case 25: return 84; case 26: return 931;
        case 27: return 85; case 28: return 932; case 29: return 86; case 30: return 85; case 31: return 85; case 32: return 85; case 33: return 932; case 34: return 88; case 35: return 933; case 36: return 90; case 37: return 934; case 38: return 91; case 39: return 92; case 40: return 935; case 41: return 94; case 42: return 94; case 43: return 94; case 44: return 94; case 45: return 936; case 46: return 95; case 47: return 937; case 48: return 97; case 49: return 938; case 50: return 98;
        case 51: return 99; case 52: return 99; case 53: return 939; case 54: return 100; case 55: return 940; case 56: return 102; case 57: return 941; case 58: return 103; case 59: return 103; case 60: return 942; case 61: return 104; case 62: return 105; case 63: return 105; case 64: return 943; case 65: return 944; case 66: return 943; case 67: return 103; case 68: return 942; case 69: return 103; case 70: return 108; case 71: return 593; case 72: return 110; case 73: return 110; case 74: return 945;
        case 75: return 108; case 76: return 112; case 77: return 113; case 78: return 946; case 79: return 947; case 80: return 114; case 81: return 114; case 82: return 115; case 83: return 115; case 84: return 948; case 85: return 115; case 86: return 115; case 87: return 115; case 88: return 82; case 89: return 949; case 90: return 114; case 91: return 114; case 92: return 114; case 93: return 117; case 94: return 118; case 95: return 118; case 96: return 118; case 97: return 119; case 98: return 120;
        case 99: return 120; case 100: return 120; case 101: return 950; case 102: return 951; case 103: return 950; case 104: return 952; case 105: return 953; case 106: return 952; case 107: return 953; case 108: return 951; case 109: return 120; case 110: return 120; case 111: return 120; case 112: return 954; case 113: return 955; case 114: return 956; case 115: return 957; case 116: return 958; case 117: return 959; case 118: return 130; case 119: return 960; case 120: return 130;
        case 121: return 131; case 122: return 130; case 124: return 126; case 125: return 961; case 126: return 132; case 127: return 86; case 128: return 962; case 129: return 963; case 130: return 116; case 131: return 964; case 132: return 134; case 133: return 964; case 134: return 134; case 135: return 965; case 136: return 931; case 137: return 964; case 138: return 134; case 139: return 967; case 140: return 966; case 141: return 135; case 142: return 135; case 143: return 135;
        case 144: return 136; case 145: return 136; case 146: return 136; case 147: return 136; case 148: return 968; case 149: return 969; case 150: return 969; case 151: return 969; case 152: return 969; case 153: return 969; case 154: return 970; case 155: return 138; case 156: return 971; case 157: return 972; case 158: return 973; case 159: return 969; case 160: return 140; case 161: return 140; case 162: return 141; case 163: return 141; case 164: return 141; case 165: return 141;
        case 166: return 141; case 167: return 974; case 168: return 974; case 169: return 140; case 170: return 975; case 171: return 976; case 172: return 976; case 173: return 976; case 174: return 976; case 175: return 976; case 176: return 976; case 177: return 976; case 178: return 144; case 179: return 144; case 180: return 977; case 181: return 144; case 182: return 142; case 183: return 977; case 184: return 142; case 185: return 978; case 186: return 979; case 187: return 980;
        case 188: return 956; case 189: return 146; case 190: return 142; case 191: return 142; case 192: return 142; case 193: return 142; case 194: return 981; case 195: return 982; case 196: return 147; case 197: return 148; case 198: return 148; case 199: return 106; case 200: return 106; case 201: return 149; case 202: return 594; case 203: return 151; case 204: return 152; case 205: return 153; case 206: return 154; case 207: return 595; case 208: return 631; case 209: return 157;
        case 210: return 157; case 211: return 157; case 212: return 157; case 213: return 157; case 214: return 157; case 215: return 157; case 216: return 157; case 217: return 157; case 218: return 157; case 219: return 157; case 220: return 158; case 221: return 983; case 222: return 159; case 223: return 160; case 224: return 984; case 225: return 106; case 226: return 106; case 227: return 985; case 228: return 111; case 229: return 106; case 230: return 106; case 231: return 165;
        case 232: return 164; case 233: return 164; case 234: return 594; case 235: return 166; case 236: return 166; case 237: return 166; case 238: return 986; case 239: return 132; case 240: return 987; case 241: return 167; case 242: return 167; case 243: return 167; case 244: return 167; case 245: return 167; case 246: return 167; case 247: return 167; case 248: return 167; case 249: return 167; case 250: return 988; case 251: return 169; case 252: return 169; case 253: return 167;
        case 254: return 167; case 255: return 167; case 256: return 82; case 257: return 930; case 258: return 170; case 259: return 989; case 260: return 990; case 261: return 991; case 262: return 992; case 263: return 992; case 264: return 992; case 265: return 993; case 266: return 82; case 267: return 930; case 268: return 167; case 269: return 167; case 270: return 173; case 271: return 994; case 272: return 176; case 273: return 995; case 274: return 176; case 275: return 174;
        case 276: return 996; case 277: return 178; case 278: return 177; case 279: return 47; case 280: return 180; case 281: return 180; case 282: return 180; case 283: return 180; case 284: return 180; case 285: return 180; case 286: return 180; case 287: return 180; case 288: return 181; case 289: return 181; case 290: return 107; case 291: return 107; case 292: return 182; case 293: return 997; case 294: return 182; case 295: return 182; case 296: return 182; case 297: return 183;
        case 298: return 183; case 299: return 183; case 300: return 183; case 301: return 183; case 302: return 126; case 303: return 126; case 304: return 961; case 477: return 1027; case 478: return 1029; case 479: return 1028; case 656: return 1030; case 519: return 1044; case 520: return 1044; case 521: return 1044; case 522: return 1044; case 523: return 1044; case 657: return 1030; case 658: return 1030; case 659: return 1030; case 660: return 1030; case 661: return 1030; case 662: return 1030;
        case 663: return 1030; case 664: return 1030; case 665: return 1030; case 666: return 1030; case 667: return 1034; case 668: return 1035; case 669: return 1036; case 670: return 1037; case 671: return 1038; case 672: return 1039; case 673: return 1040; case 674: return 1041; case 675: return 1042; case 676: return 1043; case 677: return 1033; case 678: return 1033; case 679: return 1033; case 680: return 1032; case 681: return 1032; case 682: return 1031; case 683: return 1031; case 684: return 1031;
        case 685: return 1031; case 686: return 1031; case 687: return 1030; default: return -1; // No name available; caller will print just the prefix
    }
}

void UseEntity(u16 i) {
    Entity* ent = &World.instances[i];
    if (IdxIsSearchable(ent->index) || (World.layer[i]&L_CorpseSearchable) || (IdxIsGib(ent->index) && (World.layer[i]&L_Corpse))) { SearchObject(i); } else if (IdxIsDoor(ent->index)) DoorUse(i,PLAYER1); else if (IdxIsNPC(ent->index)) CenterStatusPrint("%s%s",Sys_Text.stringTable[29],npcTable[World.instances[i].index - 419].name); else if (IdxIsButtonSwitch(ent->index)) ButtonSwitchUse(i,PLAYER1);
    else if (IdxIsGeometry(ent->index)) { int t = UseNameTableIndex(ent->index); CenterStatusPrint("%s%s",Sys_Text.stringTable[29],t >= 0 ? Sys_Text.stringTable[t] : ""); }
    else if (IdxIsUsableObject(ent->index)) {
        World.invP1.holdingObject = true; World.invP1.heldObjectIndex = ent->index; World.invP1.heldObjectCustIdx = ent->customIndex; World.invP1.heldAmmo = ent->ammo; World.invP1.heldAmmo2 = ent->ammo2; World.invP1.heldObjectLoadedAlternate = ent->heldObjectLoadedAlternate;
        if (Sys_Settings.QuickItemPickup) { AddItemToInventory(ent->index,ent->customIndex); ResetHeldItem(); } else { CenterStatusPrint("%s%s",Sys_Text.stringTable[World.invP1.heldObjectIndex - 307 + 326],Sys_Text.stringTable[319]); /* picked up.*/ ForceInventoryMode(); }/*Inventory mode is turned on when picking something up*/ DeleteInstance(i);
    } else { int t = UseNameTableIndex(ent->index); CenterStatusPrint("%s%s",Sys_Text.stringTable[29],t >= 0 ? Sys_Text.stringTable[t] : ""); }
}

INLINE V3 ScreenPointToRayOffset(V3 f,V3 r,float dx,float dy){float bx=World.inventoryMode?(float)World.cursorPos_x:683.0f,by=World.inventoryMode?(float)World.cursorPos_y:384.0f,t=vtan((float)Sys_Settings.FOV*0.5f*PI/180.0f),nx=((bx+dx)-683.0f)/384.0f,ny=-((by+dy)-384.0f)/384.0f;V3 v=V3_Normalize((V3){nx*t,ny*t,-1.0f}),ff=(V3){-f.x,-f.y,-f.z},up=V3_Normalize(V3_Cross(r,ff));return(V3){v.x*r.x+v.y*up.x+v.z*ff.x,v.x*r.y+v.y*up.y+v.z*ff.y,v.x*r.z+v.y*up.z+v.z*ff.z};}
INLINE bool FrobRayIsFrobable(RaycastHit h){if(!h.hit)return false;u16 i=h.hitInstanceIndex;if(i>=World.instCount)return false;u16 e=World.instances[i].index;if((World.layer[i]&L_CorpseSearchable)) return true;return IdxIsUsableObject(e)||IdxIsSearchable(e)||IdxIsDoor(e)||IdxIsButtonSwitch(e)||IdxIsNPC(e)||IdxIsGib(e);}
extern bool editFieldEditing;
static bool TargetIDFrob(V3 p,V3 f,V3 r){V3 dir=ScreenPointToRayOffset(f,r,0,0);RaycastHit h=Raycast(p,dir,TargetIDGetSensingRange(true),LMASK_PLAYER_TARGET_ID_FROB);if(!h.hit||h.hitInstanceIndex>=World.instCount||!IdxIsNPC(World.instances[h.hitInstanceIndex].index))return false;u16 i=h.hitInstanceIndex;Entity* e=&World.instances[i];if(e->health<=0.0f){if(World.layer[i]&L_CorpseSearchable){UseEntity(i);return true;}return false;}if((World.invP1.hasHardware&HW_TID)&&World.invP1.hwVers[HW_TID_IDX]>1){if(targetIDAttached[i]&&targetIDAttachedFinished[i]<=World.pauseRelativeTime)targetIDAttached[i]=false;if(!targetIDAttached[i]){CreateTargetIDInstance(-1.0f,i,-1.0f);return true;}}CenterStatusPrint("%s%s",Sys_Text.stringTable[29],npcTable[e->index-419].name);return true;}
static void Frob(V3 p,V3 f,V3 r){
    if(World.uiIsBlocking||World.curLev==LEVEL_CYBERSPACE)return;
    if(Cheats.editMode){V3 d0=ScreenPointToRayOffset(f,r,0,0);RaycastHit fh=Raycast(p,d0,World.farPlane[World.curLev],LMASK_PLAYER_FROB);editModeSelection=(fh.hit&&fh.hitInstanceIndex>=INSTS_1ST_IDX&&fh.hitInstanceIndex<World.instCount)?fh.hitInstanceIndex:U16_MAX; if(editModeSelection<U16_MAX){editFieldEditing=false; CenterStatusPrint("Selected object %u (const index %u)",editModeSelection,World.instances[editModeSelection].index);}else{CenterStatusPrint("Object deselected");return;}}
    if(World.Sys_UI.vmailActive){World.Sys_UI.vmailActive=0;return;}if(World.invP1.holdingObject){DropHeldItem();return;}if(TargetIDFrob(p,f,r))return;float o=(float)Sys_Settings.ScreenHeight*0.02f;RaycastHit fh={0},bh={0};bool ok=false;V3 d0=ScreenPointToRayOffset(f,r,0,0);fh=Raycast(p,d0,FROB_DISTANCE,LMASK_PLAYER_FROB);bh=fh;ok=FrobRayIsFrobable(fh);float ox[8]={0,0,o,-o,o,-o,-o,o},oy[8]={-o,o,0,0,o,-o,o,-o};for(int i=0;i<8&&!ok;++i){V3 d=ScreenPointToRayOffset(f,r,ox[i],oy[i]);RaycastHit th=Raycast(p,d,FROB_DISTANCE,LMASK_PLAYER_FROB);if(FrobRayIsFrobable(th)){bh=th;ok=true;}}if(!ok)bh=fh;if(Cheats.showPhys){World.debugLine_start=p;World.debugLineFinished=World.pauseRelativeTime+3.0;V3 dbg=ok?ScreenPointToRayOffset(f,r,0,0):d0;RaycastHit dh=ok?bh:fh;World.debugLine_end=dh.hit?dh.point:(V3){dbg.x*FROB_DISTANCE+p.x,dbg.y*FROB_DISTANCE+p.y,dbg.z*FROB_DISTANCE+p.z};}if(!ok){if(fh.hit){u16 idx=fh.hitInstanceIndex;if(idx<World.instCount){u16 ei=World.instances[idx].index;if(IdxIsGeometry(ei)||IdxIsDoor(ei)||World.instances[idx].index>=595){int t=UseNameTableIndex(ei);CenterStatusPrint("%s%s",Sys_Text.stringTable[29],t>=0?Sys_Text.stringTable[t]:"");return;}}}CenterStatusPrint("%s",Sys_Text.stringTable[30]);}else UseEntity(bh.hitInstanceIndex);}
// Update
void WeaponsUpdate(); void TextureSequenceUpdate(u16 self); void AIAnimationControllerUpdate(u16 selfIdx); void AIControllerUpdate(u16 selfIdx);
extern float sightPointHeights[NUM_AI_TYPES];
void DrawAIDebug(u16 i) {
    if ((!IdxIsNPC(World.instances[i].index)) || !Cheats.showNPC) return; World.layer[i] = L_NPC; World.layer[PLAYER1] = L_Player; Quaternion r = World.rotation[i]; float x=r.x,y=r.y,z=r.z,w=r.w; V3 fwd = V3_Normalize((V3){2.0f*(x*z + w*y), 0.0f, 1.0f - 2.0f*(x*x + y*y)}); u16 npcIdx = World.instances[i].index - 419;
    V3 sightPt = V3_AplusB(World.position[i],(V3){0.0f,sightPointHeights[npcIdx],0.0f}); DrawLine(sightPt,V3_AplusB(sightPt,V3_ScaleByF(fwd,0.6f)),(Color){1.0f,1.0f,0.0f,1.0f}); V3 enemPt = World.position[PLAYER1]; enemPt.y -= 0.24f;
    RaycastHit hit = Raycast(sightPt,V3_AsubB(enemPt,sightPt),20.0f,LMASK_NPC_SIGHT); if (hit.hit && hit.hitInstanceIndex == PLAYER1) { DrawLine(sightPt,hit.point,(Color){1.0f,0.0f,0.0f,1.0f}); } else {DrawLine(sightPt,enemPt,(Color){0.0f,1.0f,1.0f,1.0f});} Entity* e = &World.instances[i]; Color dbgCol;
    if (e->currentState == AIState_Idle) dbgCol = (Color){0.0f,1.0f,0.0f,1.0f}; else if (e->currentState == AIState_Walk || e->currentState == AIState_Run) { if (e->entflags & EF_ENEM_IN_SIGHT) dbgCol = (Color){1.0f,0.0f,0.0f,1.0f}; else dbgCol = (Color){1.0f,1.0f,0.0f,1.0f}; }
    else if (e->currentState == AIState_Attack1 || e->currentState == AIState_Attack2 || e->currentState == AIState_Attack3) dbgCol = (Color){1.0f,0.0f,1.0f,1.0f}; else if (e->currentState == AIState_Pain) dbgCol = (Color){1.0f,0.0f,1.0f,1.0f}; else if (e->currentState == AIState_Dead) dbgCol = (Color){0.5f,0.5f,0.5f,1.0f}; else { dbgCol = (Color){1.0f,0.9f,0.8f,1.0f}; }
    DrawSphereWireframe(dbgCol, (ShapeSphere){sightPt, 0.32f});
}

void ModUpdate() {
    if (World.paused || World.menuActive) return; UpdateSearchTether(); WeaponsUpdate(); PatchUpdate(); HardwareUpdate(); MissionTimerUpdate(); if (Use()) Frob(World.position[PLAYER1],World.instances[PLAYER1].forward,World.instances[PLAYER1].right); if (World.pauseRelativeTime < World.debugLineFinished && (World.debugLineVertCount + 6) < (MAX_WIRELINE_VRTS * 3)) DrawLine(World.debugLine_start,World.debugLine_end,(Color){0.3f,0.1f,0.6f,0.5f});
    for (u16 i=INSTS_1ST_IDX;i<World.instCount;++i) {
        Entity* e = &World.instances[i]; u16 constdex = e->index; if(IsLiveGrenade(constdex) && (e->entflags & EF_ACTIVE)) GrenadeUpdate(i); DelayedSpawnUpdate(i); if (e->textureAnimating && e->tickFinished < World.pauseRelativeTime) TextureSequenceUpdate(i); if(IdxIsButtonSwitch(constdex)){ButtonSwitchUpdate(i);} if(IdxIsDoor(constdex)){DoorUpdate(i);}    if(constdex == 701){LogicTimerUpdate(i);} if(e->itemLifeTime > 0.0f){SearchFXResetUpdate(i);}
        if(e->cyberTimer > 0.0f){CyberTimerUpdate(i);}          if(constdex == 515){ForceBridgeUpdate(i);} if(constdex == 517){FuncWallUpdate(i);}   if(constdex == 21 || constdex == 22){CyberWallUpdate(i);} if(IdxIsNPC(constdex)) { DrawAIDebug(i); AIControllerUpdate(i); AIAnimationControllerUpdate(i); }
    }
    if (World.invP1.painSoundFinished < World.pauseRelativeTime && World.instances[PLAYER1].radiation > 1.0f && !(World.invP1.radSoundFinished < World.pauseRelativeTime)) { World.invP1.painSoundFinished = World.pauseRelativeTime + (double)random_range(2.5f,4.0f); play_wav(sounds[140]/*player/playerpain1*/,AppliedFXVol(0.2f),(V3){0,0,0},false); }
    if (World.invP1.radBleedFinished < World.pauseRelativeTime && World.instances[PLAYER1].radiation > 1.0f) { World.invP1.radBleedFinished = World.pauseRelativeTime + 1.8; float take=World.instances[PLAYER1].radiation*0.2f; World.instances[PLAYER1].health-=take; World.painStaticAlpha = take > 15.0f ? 1.0f : take > 10.0f ? 0.8f : 0.3f; }
    if (World.invP1.radSoundFinished < World.pauseRelativeTime && World.instances[PLAYER1].radiation > 1.0f) { double minT = World.instances[PLAYER1].radiation > 50.0f ? 0.5 : 1.0; World.invP1.radSoundFinished = World.pauseRelativeTime + minT + (double)random_range(0.0f,2.0f); play_wav(sounds[90]/*hud/radiation*/,AppliedFXVol(0.18f),(V3){0,0,0},false); }
}

u16 GetCrosshairTexture() { switch(World.invP1.weaponIndex) { case 343:case 345:case 350:case 352:case 355:return 1121;/*red*/case 344:case 347:case 357:return 1253;/*blue*/case 348:case 349:return 1066;/*orange*/case 351:case 354:return 1122;/*yellow*/ case 353:case 358:return 1161;/*teal*/default:return 1260;/*green*/ } }
u16 GetItemFrobTexture(u16 index) {
    switch(index){
        case 312: return 605;/*item_arm*/                 case 313: return 606;/*item_audiolog*/            case 364: return 969;/*item_chipset_interfacedemod*/ case 308: return 838;/*item_paper_wad*/            case 309: return 764;/*item_beaker*/            case 310: return 767;/*item_beverage*/            case 311: return 981;/*item_skull*/               case 314: return 853;/*weapon_grenadefrag*/          case 315: return 849;/*weapon_grenadeconc*/    case 316: return 851;/*weapon_grenadeemp*/ 
        case 317: return 850;/*weapon_grenadeearth*/      case 318: return 860;/*weapon_grenademine*/       case 319: return 861;/*weapon_grenadenitro*/         case 320: return 859;/*weapon_grenadegas*/         case 321: return 974;/*item_patch_berserk*/     case 322: return 975;/*item_patch_detox*/         case 323: return 976;/*item_patch_genius*/        case 324: return 977;/*item_patch_medi*/             case 325: return 978;/*tem_patch_reflex*/      case 326: return 979;/*item_patch_sight*/ 
        case 327: return 980;/*item_patch_staminup*/      case 328: return 882;/*item_hw_system*/           case 329: return 907;/*item_hw_navunit*/             case 330: return 902;/*item_hw_ereader*/           case 331: return 909;/*item_hw_sensaround*/     case 332: return 935;/*item_hw_targetid*/         case 333: return 911;/*item_hw_shield*/           case 334: return 900;/*item_hw_bio*/                 case 335: return 906;/*item_hw_lantern*/       case 336: return 903;/*item_hw_envirosuit*/
        case 337: return 901;/*item_hw_booster*/          case 338: return 905;/*item_hw_jumpjets*/         case 339: return 904;/*item_hw_infrared*/            case 340: return 966;/*item_fireextinguisher*/     case 341: return 626;/*item_access_card_admin*/ case 342: return 845;/*item_workerhelmet*/        case 343: return 988;/*weapon_mk3*/               case 344: return 982;/*weapon_blaster*/              case 345: return 983;/*weapon_dartgun*/        case 346: return 984;/*weapon_flechette*/
        case 347: return 985;/*weapon_ionrifle*/          case 348: return 1034;/*weapon_rapier*/           case 349: return 990;/*weapon_pipe*/                 case 350: return 986;/*weapon_magnum*/             case 351: return 987;/*weapon_magpulse*/        case 352: return 1010;/*weapon_pistol*/           case 353: return 1019;/*weapon_plasma*/           case 354: return 1027;/*weapon_railgun*/             case 355: return 1035;/*weapon_riotgun*/       case 356: return 1036;/*weapon_skorpion*/
        case 357: return 1052;/*weapon_sparqbeam*/        case 358: return 1065;/*weapon_stungun*/          case 359: return 965;/*item_battery*/                case 360: return 968;/*item_battery_icad*/         case 361: return 972;/*item_logic_probe*/       case 362: return 967;/*item_healthkit*/           case 363: return 973;/*item_plastique*/           case 365: return 766;/*item_flask*/                  case 366: return 969;/*item_chipset_bitflag*/  case 367: return 549;/*item_ammo_rubber*/
        case 368: return 971;/*item_isotopex22*/          case 369: return 765;/*it442em_testtube*/         case 370: return 853;/*weapon_grenadefrag_live*/     case 371: return 970;/*item_chipset_isolinear*/    case 372: return 849;/*weapon_grenadeconc_live*/case 373: return 420;/*item_ammo_needle*/         case 374: return 602;/*item_ammo_tranq*/          case 375: return 593;/*item_ammo_standard*/          case 376: return 597;/*item_ammo_teflon*/      case 377: return 411;/*item_ammo_hollow*/
        case 378: return 561;/*item_ammo_slug*/           case 379: return 419;/*item_ammo_magnesium*/      case 380: return 421;/*item_ammo_penetrator*/        case 381: return 417;/*item_ammo_hornet*/          case 382: return 577;/*item_ammo_splinter*/     case 383: return 422;/*item_ammo_rail*/           case 384: return 551;/*item_ammo_slag*/           case 385: return 552;/*item_ammo_slaglarge*/         case 386: return 418;/*item_ammo_magcart*/     case 387: return 851;/*weapon_grenadeemp_live*/
        case 388: return 762;/*item_access_card_std*/     case 389: return 850;/*weapon_grenadeearth_live*/ case 390: return 610;/*item_access_card_group1*/     case 391: return 621;/*item_access_card_science*/  case 392: return 609;/*item_access_card_eng*/   case 393: return 610;/*item_access_card_groupB*/  case 394: return 635;/*item_access_card_security*/case 395: return 761;/*item_access_card_per5diego*/  case 396: return 632;/*item_access_card_medi*/ case 397: return 610;/*item_access_card_group3*/
        case 398: return 624;/*item_access_card_purple*/  case 399: return 872;/*item_head_male*/           case 400: return 862;/*item_head_female*/            case 401: return 872;/*item_severedhead*/          case 402: return 860;/*weapon_grenademine_live*/case 403: return 861;/*weapon_grenadenitro_live*/ case 404: return 859;/*weapon_grenadegas_live*/   case 417: return 760;/*item_access_card_perdarcy*/
        case 307: return 1250;
    } return MAX_TXRS;
}

u16 GetCursorTexture() {
    if(World.paused || World.menuActive){if(Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].down || Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].down){return 2147;} return 1261;}
    if(World.invP1.holdingObject) {u16 tex=GetItemFrobTexture(World.invP1.heldObjectIndex); return tex<MAX_TXRS?tex:1250;}
    if(World.uiIsBlocking || World.mouseClickHeldOverGUI){if(Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].down || Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].down){return 2147;} return 1261;}
    return GetCrosshairTexture();
}
