// citadel.c - Game logic.
#include "common.h"
__attribute__((used)) AutoSplitterData autoSplitter = {0x1337133713371337,0,false,0}; static const u16 patchMsg[7] = {340,341,342,343,344,345,346}; /*Patch names in the +326 item-name table (Citadel: useableItemIndex+326, useableItemIndex=14+slot); Citadel composes "<name>" + stringTable[589]*/
void PatchDisableAll(),BiomonitorEnergyPulse(float),BioMonitorClearGraphs(),TextureSequenceInit(u16,char*),GrenadeActivate(u16); bool RecentLog(); extern double lerpStartTime; extern V3 queuedLevelPos; extern u8 queuedLevelToLoad; extern u16 editModeSelection;
V3 ScreenPointToRay(V3 fwd, V3 rt) {
    float px=World.inventoryMode?(float)World.cursorPos_x:(float)UI_W*0.5f, py=World.inventoryMode?(float)World.cursorPos_y:(float)UI_H*0.5f;
    float tanFov=vtan((float)Sys_Settings.FOV*0.5f*PI/180.0f),aspect=(float)Sys_Settings.ScreenWidth/(float)Sys_Settings.ScreenHeight;
    float ndcX=(px-(float)UI_W*0.5f)/((float)UI_W*0.5f),ndcY=((float)UI_H*0.5f-py)/((float)UI_H*0.5f);
    V3 view=V3_Normalize((V3){ndcX*aspect*tanFov,ndcY*tanFov,-1.0f}),flipForward=(V3){-fwd.x,-fwd.y,-fwd.z}; V3 up=V3_Normalize(V3_Cross(rt,flipForward));
    return (V3){view.x*rt.x+view.y*up.x+view.z*flipForward.x,view.x*rt.y+view.y*up.y+view.z*flipForward.y,view.x*rt.z+view.y*up.z+view.z*flipForward.z};
}

static i16 GrenadeTypeFromConst(u16 idx) { switch(idx) { case 370:return 7; case 372:return 8; case 387:return 9; case 389:return 10; case 402:return 11; case 403:return 12; case 404:return 13; default:return -1; } }
bool IsLiveGrenade(u16 idx) { return GrenadeTypeFromConst(idx) >= 7; }
static const float grenadeDamage[7]={150,325,80,375,230,200,150},grenadePenetration[7]={20,35,100,50,35,25,100},grenadeOffense[7]={3,6,3,6,5,3,3},grenadeRadius[7]={4,7,6,7.5f,5.1f,5.12f,4}; static const AttType grenadeAttackType[7]={Att_HitS,Att_HitS,Att_Magn,Att_HitS,Att_HitS,Att_HitS,Att_Gas};
void GrenadeInit(u16 self) { i16 idx=GrenadeTypeFromConst(World.instances[self].index)-7; if(idx<0||idx>=7)return; Entity* e=&World.instances[self]; if(e->damage<=0.0f)e->damage=grenadeDamage[idx]; if(e->strength<=0.0f)e->strength=grenadePenetration[idx]; if(e->speed<=0.0f)e->speed=grenadeOffense[idx]; if(e->attackType==Att_None)e->attackType=grenadeAttackType[idx]; }
void ResetHeldItem() { World.invP1.heldObjectIndex=World.invP1.heldObjectCustIdx=U16_MAX; World.invP1.heldAmmo=World.invP1.heldAmmo2=0; World.invP1.heldObjectLoadedAlternate=World.invP1.holdingObject=World.invP1.grenActive=false; }
void DropHeldItem() {
    if (World.invP1.heldObjectIndex >= World.instCount) { ResetHeldItem(); return; }    if (World.invP1.dropFinished > World.pauseRelativeTime) {return;} World.invP1.dropFinished = World.pauseRelativeTime + 0.2;/*Prevent immediate re-grab at high fps*/ u16 n = AddInstance(World.invP1.heldObjectIndex,World.position[PLAYER1]);
    Entity* e = &World.instances[n]; e->customIndex = World.invP1.heldObjectCustIdx; e->ammo = World.invP1.heldAmmo; e->ammo2 = World.invP1.heldAmmo2; e->heldObjectLoadedAlternate = World.invP1.heldObjectLoadedAlternate;
    flag_set(&e->entflags,EF_RIGIDBODY,true); bool liveGrenade=IsLiveGrenade(e->index); if(liveGrenade){World.layer[n]=L_PlayerBullets; e->recentMostActivator=PLAYER1; GrenadeInit(n);} V3 tossDir = ScreenPointToRay(World.instances[PLAYER1].forward,World.instances[PLAYER1].right); World.position[n] = V3_AplusB(World.position[PLAYER1],V3_ScaleByF(tossDir,0.48f)); World.velocity[n] = V3_ScaleByF(tossDir,10.0f); if(liveGrenade)GrenadeActivate(n); ResetHeldItem();
}

void PatchUse(int patchSlot) {
    if (patchSlot < 0 || patchSlot > 6) return; if (World.invP1.patchCounts[patchSlot] <= 0) { CenterStatusPrint("%s", Sys_Text.stringTable[324]); return; } if (patchSlot == 3 && World.instances[PLAYER1].health >= 255.0f) { CenterStatusPrint("%s", Sys_Text.stringTable[304]); return; }
    World.invP1.patchCounts[patchSlot]--; World.invP1.patchActive |= (u16)(1u << patchSlot);
    switch (patchSlot) {
        case 0: if(World.invP1.berserkFinished > World.pauseRelativeTime){World.invP1.berserkFinished += 20.0;} else{World.invP1.berserkFinished = World.pauseRelativeTime + 20.0; World.invP1.berserkIncTime = World.pauseRelativeTime + 4.0; World.invP1.berserkIncrement = 0; berserkSeedTime = random_range(0.0f,1000.0f);} break;
        case 1: PatchDisableAll(); World.invP1.patchActive |= PATCH_DETOX; World.invP1.detoxFinished = World.pauseRelativeTime + 60.0; break;/*Detox wipes all other patches on use; other patches may still be applied afterwards*/
        case 2: if(World.invP1.geniusFinished > World.pauseRelativeTime) World.invP1.geniusFinished += 180.0; else World.invP1.geniusFinished = World.pauseRelativeTime + 180.0; World.geniusActive = true; break;
        case 3: if(World.invP1.mediFinished > World.pauseRelativeTime) World.invP1.mediFinished += 35.0; else { World.invP1.mediFinished = World.pauseRelativeTime + 35.0; World.invP1.mediPatchPulseFinished = 0.0; } World.invP1.mediPatchPulseCount = 0; break;
        case 4: { double reflexDur = 155.0 * 0.25;/*155 real seconds of 0.25x slow-mo*/ if(World.invP1.reflexFinishedTime > World.pauseRelativeTime) World.invP1.reflexFinishedTime += reflexDur; else World.invP1.reflexFinishedTime = World.pauseRelativeTime + reflexDur; World.timeScale = 0.25; } break;
        case 5: if(World.invP1.sightFinishedTime > World.pauseRelativeTime) World.invP1.sightFinishedTime += 40.0; else { World.invP1.sightFinishedTime = World.pauseRelativeTime + 40.0; World.invP1.sightSideEffectFinishedTime = -1.0; } break;
        case 6: if(World.invP1.staminupFinishedTime > World.pauseRelativeTime) World.invP1.staminupFinishedTime += 60.0; else World.invP1.staminupFinishedTime = World.pauseRelativeTime + 60.0; World.invP1.staminupActive = true; World.invP1.fatigue = 0.0f; break;
    }
    bool depleted = World.invP1.patchCounts[patchSlot] <= 0;
    if (depleted) { for (int i = 0; i < 7; i++) { if (World.invP1.patchCounts[i] > 0) { World.invP1.patchCur = (i8)i; break; } } }
    if (depleted) CenterStatusPrint("%s%s%s",Sys_Text.stringTable[590],Sys_Text.stringTable[patchMsg[patchSlot]],Sys_Text.stringTable[589]);
    else CenterStatusPrint("%s%s",Sys_Text.stringTable[patchMsg[patchSlot]],Sys_Text.stringTable[589]);
    play_wav(sounds[89],AppliedFXVol(1.0f),(V3){0.0f,0.0f,0.0f},false);
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
    static const u8 textIdx[12] = {21,22,23,24,25,26,27,28,29,30,31,32}; World.invP1.hasHardware |= (u16)(1u << index); World.invP1.hwVers[index] = (u8)hwversion; World.invP1.hwVersSetting[index]= hwversion > 0 ? (u8)(hwversion - 1) : 0; CenterStatusPrint("%s v%d",Sys_Text.stringTable[textIdx[index] + 326],hwversion);
}

bool AddGeneralObjectToInventory(int index, int custIdx){
    if (!World.Sys_UI.firstGeneral){World.Sys_UI.firstGeneral=true; World.Sys_UI.MFD_CenterTab=3;}
    for(i8 i=1;i<14;++i){if(World.invP1.genInvIdxRef[i]==-1){if(!InventoryHasAnyAccessCards()&&World.invP1.generalInvCurrent==0){World.invP1.generalInvCurrent=i;} World.invP1.genInvIdxRef[i]=index; World.invP1.generalInvCustIdx[i]=(i16)custIdx; World.Sys_UI.generalClickSlot = -1; CenterStatusPrint("%s%s",Sys_Text.stringTable[ItemStringIdx(index)],Sys_Text.stringTable[31]); return true;}} return false;
}

void CheckForUnreadLogs() { int e=0,l=0; for (int i=0;i<LOGCNT;++i) if (World.invP1.hasLog[i] && !World.invP1.readLog[i]) *(Sys_Text.audioLogType[i] == AudioLogType_Email ? &e : &l)=1; World.invP1.hasNewEmail=e; World.invP1.hasNewLogs=l; }
static int FindNextUnreadLog() { for (int i = LOGCNT-1; i >= 0; i--) { if(World.invP1.hasLog[i] && !World.invP1.readLog[i]){return i;} } return -1; }
void PlayLog(int logIndex) {if(logIndex<0||logIndex>=LOGCNT||!(World.invP1.hasHardware&HW_ERD)){return;} play_message(AudioLogPath(logIndex)); World.invP1.readLog[logIndex]=true; if(Sys_Text.audioLogType[logIndex] == AudioLogType_Vmail){World.Sys_UI.vmailActive=true;} CenterStatusPrint("%s%s",Sys_Text.stringTable[1020],World.audiologNames[logIndex]); MFD_OpenAudioLog(logIndex); }
void PlayLastAddedLog(int logIndex) { if(logIndex < 0){return;} PlayLog(logIndex); World.invP1.lastAddedIndex = -1; }
void AddAudioLogToInventory(int index) {
    if (index < 0 || index >= LOGCNT) { DualLogError("BUG: Audio log index %d out of range (%d)\n",index,LOGCNT); return; }
    if (index == 128) { CenterStatusPrint("%s",Sys_Text.stringTable[309]); return; }/*Trioptimum Funpack*/
        
    World.invP1.hasLog[index]  = true; World.invP1.lastAddedIndex = index; World.invP1.numLogsFromLevel[Sys_Text.audioLogLevelFound[index]]++;
         if(Sys_Text.audioLogType[index] == AudioLogType_Email)World.invP1.hasNewEmail=true;
    else if(Sys_Text.audioLogType[index] == AudioLogType_Normal)World.invP1.hasNewLogs=true;
    if (World.invP1.hasHardware & HW_ERD) {
        char keyStr[8];
        sFormat(keyStr,sizeof(keyStr),"%s", Sys_Settings.InputCodeSettings[20] ? "U" : "?");
        CenterStatusPrint("%s%s%s %s",Sys_Text.stringTable[36],World.audiologNames[index],Sys_Text.stringTable[38],keyStr); }
    else { CenterStatusPrint("%s%s%s",Sys_Text.stringTable[36],World.audiologNames[index],Sys_Text.stringTable[310]); }
}

static inline void ItemAdd(u8 *cur, u8 *counts, int idx, int uIdx, int sysIdx) { if (!counts[*cur]) {*cur=(i8)idx;} counts[idx]++; CenterStatusPrint("%s%s", Sys_Text.stringTable[ItemStringIdx(uIdx)], Sys_Text.stringTable[sysIdx]); }
void AddGrenadeToInventory(int i, int u) { if (i >= 0){if (!World.Sys_UI.firstMain){World.Sys_UI.firstMain=true; World.Sys_UI.MFD_CenterTab=1;} World.invP1.grenConstIndex[i]=(i16)u; ItemAdd(&World.invP1.grenCur,World.invP1.grenAmmo,i,u,34);} }
void   AddPatchToInventory(int i, int u) { if (i >= 0){if (!World.Sys_UI.firstMain){World.Sys_UI.firstMain=true; World.Sys_UI.MFD_CenterTab=1;} ItemAdd(&World.invP1.patchCur,World.invP1.patchCounts,i,u,35);} }
static inline void GrenadeCycle(int step){int cur= World.invP1.grenCur, next=cur; for(int i=0;i<7;++i){next=(next+step+7)%7; if(   World.invP1.grenAmmo[next]>0){ if (next==cur) return; World.invP1.grenCur =(i8)next; play_wav(sounds[80],AppliedFXVol(1.0f),(V3){0},false);/*changeweapon*/ CenterStatusPrint("%s",Sys_Text.stringTable[579+next]); return;}}}
static inline void   PatchCycle(int step){int cur=World.invP1.patchCur, next=cur; for(int i=0;i<7;++i){next=(next+step+7)%7; if(World.invP1.patchCounts[next]>0){World.invP1.patchCur=(i8)next; play_wav(sounds[80],AppliedFXVol(1.0f),(V3){0},false);/*changeweapon*/ CenterStatusPrint("%s",Sys_Text.stringTable[patchMsg[next]]); return;}}}
void RemoveGrenade(int i) { if(World.invP1.grenAmmo[i] > 0){World.invP1.grenAmmo[i]--;} if(!World.invP1.grenAmmo[i]){GrenadeCycle(-1);} }
static i8 GetExistingCyberItemIndex() { if (World.invP1.softVersions[SW_TURBO]  > 0) {return 0;} if (World.invP1.softVersions[SW_DECOY]  > 0) {return 1;} if (World.invP1.softVersions[SW_RECALL] > 0) {return 2;} return -1; }
static void UseTurbo() {if(World.invP1.softVersions[SW_TURBO]<=0){World.invP1.hasSoft&=(u8)~(1u << SW_TURBO); return;} if(--World.invP1.softVersions[SW_TURBO]==0)World.invP1.hasSoft&=(u8)~(1u << SW_TURBO); if(World.invP1.turboFinished > World.pauseRelativeTime){World.invP1.turboFinished+=World.invP1.turboCyberTime;}else{World.invP1.turboFinished=World.invP1.turboCyberTime+World.pauseRelativeTime;}}
void CyberDecoySpawned(u16);
static void UseDecoy() {if (World.decoyActive) { CenterStatusPrint("%s",Sys_Text.stringTable[537]); return; } if (World.invP1.softVersions[SW_DECOY] <= 0) { World.invP1.hasSoft &= (u8)~(1u << SW_DECOY); return; } if (--World.invP1.softVersions[SW_DECOY] == 0) World.invP1.hasSoft &= (u8)~(1u << SW_DECOY); u16 decoyIdx = SpawnDynamicObject(CYBER_DECOY_CONST,true);
    if(decoyIdx != U16_MAX){World.position[decoyIdx]=World.position[PLAYER1]; World.velocity[decoyIdx]=(V3){0}; flag_set(&World.instances[decoyIdx].entflags,EF_RIGIDBODY,false);
        World.instances[decoyIdx].active = true; World.instances[decoyIdx].doSelfAfterList = true; World.instances[decoyIdx].despawnInstead = true; World.instances[decoyIdx].timerFinished = World.pauseRelativeTime + 15.0f;
        World.decoyActive=true; CyberDecoySpawned(decoyIdx);}}
static void UseRecall() { if (World.invP1.softVersions[SW_RECALL] <= 0) {return;} if (--World.invP1.softVersions[SW_RECALL] == 0) {World.invP1.hasSoft &= (u8)~(1u << SW_RECALL);} World.position[PLAYER1] = World.cyberspaceRecallPoint; }
void UseCyberspaceItem() {
    if (World.invP1.cyberItemIndex <= 0) { World.invP1.cyberItemIndex = GetExistingCyberItemIndex(); if (World.invP1.cyberItemIndex < 0) { CenterStatusPrint("%s",Sys_Text.stringTable[473]); return; } }
    switch(World.invP1.cyberItemIndex) {case 0: if (!World.invP1.softVersions[SW_TURBO])  { World.invP1.cyberItemIndex = GetExistingCyberItemIndex(); return; } UseTurbo();  break; case 1: if (!World.invP1.softVersions[SW_DECOY])  { World.invP1.cyberItemIndex = GetExistingCyberItemIndex(); return; } UseDecoy();  break; case 2: if (!World.invP1.softVersions[SW_RECALL]) { World.invP1.cyberItemIndex = GetExistingCyberItemIndex(); return; } UseRecall(); break;}
}

void UseCyberspaceItemByIndex(int idx) { World.invP1.cyberItemIndex = (i8)idx; UseCyberspaceItem(); }/*MFD software tab rows 3..5 map to the 0..2 turbo/decoy/recall selector*/
void CycleCyberSpaceItemUp() { int next = World.invP1.cyberItemIndex + 1; if (next > 2){next=0;} for (int c = 0; c <= 7; c++) { if (World.invP1.hasSoft & (1u << (SW_TURBO+next))) { World.invP1.cyberItemIndex = (i8)next; return; } if (c == 7) { World.invP1.cyberItemIndex = -1; return; } if (++next > 2) {next = 0;} } }
void CycleCyberSpaceItemDn() { int next = World.invP1.cyberItemIndex - 1; if (next < 0){next=2;} for (int c = 0; c <= 7; c++) { if (World.invP1.hasSoft & (1u << (SW_TURBO+next))) { World.invP1.cyberItemIndex = (i8)next; return; } if (c == 7) { World.invP1.cyberItemIndex = -1; return; } if (--next < 0) {next = 2;} } }
void CompleteWeaponChange(void); /* defined in weapons.c: applies pending weapon change to state, view model, and HUD */
void RemoveWeapon(i32 slot) {
    if (slot < 0 || slot >= 7 || World.invP1.wepInvIndices[slot] < 0) return;
    if (World.invP1.holdingObject) return; /* hands full, same refusal as GeneralInvTake */
    /* The weapon leaves the 7-slot list and becomes the held object, carrying its loaded magazines: AddItemToInventory -> AddWeaponToInventory reads heldAmmo/heldAmmo2/heldObjectLoadedAlternate to put them back (ADD TO INVENTORY, or a quick pickup). heldObjectIndex is the EDD, and a weapon's EDD is 343..358 - the very value wepInvIndices already holds - so store it verbatim. Subtracting 307 was wrong: IdxIsWeapon, GetItemFrobTexture and ItemStringIdx all key off 343..358, and the -307 consumers (RelayPanelUse, HeldItemIsFrobUser) want the EDD too. */
    World.invP1.heldAmmo = World.invP1.currentMagazineAmount[slot]; World.invP1.heldAmmo2 = World.invP1.currentMagazineAmount2[slot]; World.invP1.heldObjectLoadedAlternate = World.invP1.wepLoadedWithAlternate[slot];
    World.invP1.heldObjectCustIdx = U16_MAX; World.invP1.heldObjectIndex = (u16)World.invP1.wepInvIndices[slot]; World.invP1.grenActive = false; World.invP1.holdingObject = true;
    for (i32 i=slot;i<6;++i) { World.invP1.wepInvIndices[i]=World.invP1.wepInvIndices[i+1]; World.invP1.wepInvAmIndices[i]=World.invP1.wepInvAmIndices[i+1]; World.invP1.currentMagazineAmount[i]=World.invP1.currentMagazineAmount[i+1]; World.invP1.currentMagazineAmount2[i]=World.invP1.currentMagazineAmount2[i+1]; World.invP1.wepLoadedWithAlternate[i]=World.invP1.wepLoadedWithAlternate[i+1]; World.invP1.curWepHeat[i]=World.invP1.curWepHeat[i+1]; }
    World.invP1.wepInvIndices[6]=World.invP1.wepInvAmIndices[6]=-1; World.invP1.currentMagazineAmount[6]=World.invP1.currentMagazineAmount2[6]=0; World.invP1.wepLoadedWithAlternate[6]=false; World.invP1.curWepHeat[6]=0.0f;
    if (slot <= World.invP1.wepCur && World.invP1.wepCur > 0) World.invP1.wepCur--;
    if (World.invP1.wepCur < 0) World.invP1.wepCur = 0;
    World.invP1.wepCurPending = World.invP1.wepCur;
    World.invP1.weaponIndexPending = (World.invP1.wepCur >= 0 && World.invP1.wepInvIndices[World.invP1.wepCur] >= 0) ? (i16)World.invP1.wepInvIndices[World.invP1.wepCur] : -1;
    u8 numweapons = 0; for (int i = 0; i < 7; i++) if (World.invP1.wepInvIndices[i] >= 0) numweapons++;
    World.invP1.numweapons = numweapons;
    if (!numweapons) { for (int i = 0; i < 7; i++) World.invP1.currentMagazineAmount[i] = World.invP1.currentMagazineAmount2[i] = 0; }
    if (World.invP1.weaponIndexPending < 0) { World.instances[World.weaponVModelIndex].modelIndex = MAX_MDLS; World.invP1.wepCurPending = World.invP1.weaponIndexPending = -1; ForceInventoryMode(); return; } /* no valid weapon selected: hide the view model */
    CompleteWeaponChange(); /* updates weapon state, view model, and HUD */
    ForceInventoryMode(); /* so the held weapon and its ADD TO INVENTORY button are on screen */
}
static float DefaultEnergySettingForWeapon(int wep16Index) { return (wep16Index == 4) ? 5.0f : (wep16Index == 10) ? 13.0f : (wep16Index == 14) ? 2.0f : 3.0f; }
__attribute__((noinline)) void AddAmmoToInventory(int index,int constIndex,int amount,bool isSecondary) { if(index < 0){return;} if(isSecondary){World.invP1.wepAmmoSecondary[index]+=(u16)amount;} else {World.invP1.wepAmmo[index]+=(u16)amount;} CenterStatusPrint("%s%s",Sys_Text.stringTable[ItemStringIdx(constIndex)],Sys_Text.stringTable[630]); }
bool AddWeaponToInventory(int index,int ammo1,int ammo2,bool loadedAlt) {
    if (index < 0) return false; if (!World.Sys_UI.firstMain){World.Sys_UI.firstMain=true; World.Sys_UI.MFD_CenterTab=1;} if(!World.Sys_UI.firstWeapon){World.Sys_UI.firstWeapon=true; World.Sys_UI.MFD_LefTab=1;}
    for (i32 i = 0; i < 7; i++) {
        if(World.invP1.wepInvIndices[i] >= 0){continue;} World.invP1.wepInvIndices[i] = index; i32 index16 = Get16WeaponIndexFromConstIndex(index); World.invP1.weaponEnergySetting[index16] = DefaultEnergySettingForWeapon(index16); if (i == 0) { World.invP1.wepCurPending=i; World.invP1.weaponIndexPending=(u16)index; World.invP1.justChangedWeap=true; WeaponFireStartWeaponDip(0.5f); WeaponFireCompleteWeaponChange(); }
        if (loadedAlt && ammo2 > 0){World.invP1.currentMagazineAmount2[i]=(u8)ammo2; if (ammo1 > 0) World.invP1.wepAmmo[index16]+=(u16)ammo1; World.invP1.wepLoadedWithAlternate[i]=true;}else{World.invP1.currentMagazineAmount[i]=(u8)ammo1; if (ammo2 > 0) World.invP1.wepAmmoSecondary[index16]+=(u16)ammo2; World.invP1.wepLoadedWithAlternate[i]=false;}
        CenterStatusPrint("%s%s",Sys_Text.stringTable[ItemStringIdx(index)],Sys_Text.stringTable[33]); World.invP1.numweapons=0; for (i32 j=0;j<7;j++) { if(World.invP1.wepInvIndices[j] >= 0){World.invP1.numweapons++;} } return true;
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
    if (GrenadeCycUp())  { if (World.curLev == LEVEL_CYBERSPACE) CycleCyberSpaceItemUp(); else GrenadeCycle(Sys_Settings.InvertInventoryCycling ? -1 : 1); } if (GrenadeCycDown()){ if (World.curLev == LEVEL_CYBERSPACE) CycleCyberSpaceItemDn(); else GrenadeCycle(Sys_Settings.InvertInventoryCycling ? 1 : -1); }
    if (RecentLog() && (World.invP1.hasHardware & HW_ERD)) {
        if(World.invP1.lastAddedIndex>=0){int temp=World.invP1.lastAddedIndex; PlayLog(temp); World.invP1.lastAddedIndex=FindNextUnreadLog(); if(World.invP1.lastAddedIndex==temp)World.invP1.lastAddedIndex=-1; CheckForUnreadLogs(); }else{int temp=World.invP1.lastAddedIndex; World.invP1.lastAddedIndex=FindNextUnreadLog(); if(World.invP1.lastAddedIndex==temp){World.invP1.lastAddedIndex=-1;} CheckForUnreadLogs(); CenterStatusPrint("%s",Sys_Text.stringTable[1019]);/*Log playback stopped.*/}
    } if (Patch()) { if (World.invP1.patchCur >= 0 && World.invP1.patchCur < 7 && World.invP1.patchCounts[World.invP1.patchCur] > 0){PatchUse(World.invP1.patchCur);} else {CenterStatusPrint("%s",Sys_Text.stringTable[324]); /*Out of patches.*/} } if (PatchCycUp()){PatchCycle(Sys_Settings.InvertInventoryCycling ? -1 : 1);} else if (PatchCycDown()){PatchCycle(Sys_Settings.InvertInventoryCycling ? 1 : -1);}
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
            case 367: AddAmmoToInventory(12,index,magazinePitchCountForWeapon[12],false); break; /*DC rubber slugs*/     case 373: AddAmmoToInventory(2,index,magazinePitchCountForWeapon[2],false); break; /*SV needle darts*/ 
            case 374: AddAmmoToInventory(2,index,magazinePitchCountForWeapon2[2],true); break; /*SV tranq darts*/  case 375: AddAmmoToInventory(9,index,magazinePitchCountForWeapon[9],false); break; /*ML standard rounds*/         case 376: AddAmmoToInventory(9,index,magazinePitchCountForWeapon2[9],true); break; /*ML teflon coated rounds*/
            case 377: AddAmmoToInventory(7,index,magazinePitchCountForWeapon[7],false); break; /*hollow-tip 2100 clip*/ case 378: AddAmmoToInventory(7,index,magazinePitchCountForWeapon2[7],true); break; /*heavy slug 2100 clip*/              case 379: AddAmmoToInventory(0,index,magazinePitchCountForWeapon[0],false); break; /*MARK3 magnesium-t*/
            case 380: AddAmmoToInventory(0,index,magazinePitchCountForWeapon2[0],true); break; /*MARK3 penetrator*/    case 381: AddAmmoToInventory(3,index,magazinePitchCountForWeapon[3],false); break; /*AM hornet clip*/              case 382: AddAmmoToInventory(3,index,magazinePitchCountForWeapon2[3],true); break; /*AM splinter clip*/
            case 383: AddAmmoToInventory(11,index,magazinePitchCountForWeapon[11],false); break; /*MM rail clip*/       case 384: AddAmmoToInventory(13,index,magazinePitchCountForWeapon[13],false); break; /*RF slag clip*/          case 385: AddAmmoToInventory(13,index,magazinePitchCountForWeapon2[13],true); break; /*RF large slag clip*/ 
            case 386: AddAmmoToInventory(8,index,magazinePitchCountForWeapon[8],false); break; /*SB magpulse cart*/ default: return;
        }
    } play_wav(sounds[87], AppliedFXVol(1.0f), (V3){0}, false);
}

void CyberDoorOnCollisionEnter(u16 self, u16 other) { if(other != PLAYER1){return;} CenterStatusPrint("%s  %s",Sys_Text.stringTable[World.instances[self].messageIndex],Sys_Text.stringTable[601]); }
void CyberTimerInitAfterLoad(u16 self) { Entity* e = &World.instances[self]; e->cyberTimer = 600.0f; e->timerFinished = World.pauseRelativeTime + 1.0; }
void CyberTimerReset(u16 self, int diff) { Entity* e = &World.instances[self]; switch (diff) { case 0: e->cyberTimer = 600.0f; break; case 1: e->cyberTimer = 300.0f; break; case 2: e->cyberTimer = 240.0f; break; case 3: e->cyberTimer = 180.0f; break; } }
void ExitCyberspace(void);
void CyberTimerUpdate(u16 self) { if(World.curLev != LEVEL_CYBERSPACE){return;} Entity* e=&World.instances[self]; if(e->timerFinished >= World.pauseRelativeTime){return;} e->cyberTimer-=1.0f; e->minutes=vfloor(e->cyberTimer / 60.0f); e->seconds=e->cyberTimer - (e->minutes * 60.0f); e->timerFinished=World.pauseRelativeTime + 1.0; if(e->cyberTimer <= 0.0f){ExitCyberspace(); return;} }
void CyberWallInitAfterLoad(u16 self) { Entity* e=&World.instances[self]; e->tickFinished=World.pauseRelativeTime + 2.0; e->animSwapFinished=0.0; }
void CyberWallUpdate(u16 self) { Entity* e = &World.instances[self]; if (World.pauseRelativeTime < e->tickFinished) {return;} e->tickFinished = World.pauseRelativeTime + 0.05; }
void ExitCyberspace(void);
void CyberItemUpdate(u16 self) { Entity* e=&World.instances[self]; if(!(e->entflags&EF_ACTIVE))return; if(V3_Dist(World.position[self],World.position[PLAYER1]) < 1.5f){ DeleteInstance(self); } }
void CyberExitUpdate(u16 self) { Entity* e=&World.instances[self]; if(!(e->entflags&EF_ACTIVE))return; if(World.curLev != LEVEL_CYBERSPACE)return; if(V3_Dist(World.position[self],World.position[PLAYER1]) < 2.0f){ ExitCyberspace(); } }
void CyberSwitchUpdate(u16 self) { Entity* e=&World.instances[self]; if(!(e->entflags&EF_ACTIVE))return; if(e->active)return; if(V3_Dist(World.position[self],World.position[PLAYER1]) < 1.5f){ e->active=true; ChangeAnim(e,A_ACTIVATED); if(e->textIndex>0) CenterStatusPrint("%s",Sys_Text.stringTable[e->textIndex]); UseTargets(self,e->targetIdx); } }
void CyberDataFragUpdate(u16 self) { Entity* e=&World.instances[self]; if(!(e->entflags&EF_ACTIVE))return; if(e->allDone)return; if(V3_Dist(World.position[self],World.position[PLAYER1]) < 1.5f){ e->allDone=true; if(e->textIndex>0) CenterStatusPrint("%s",Sys_Text.stringTable[e->textIndex]); } }
void SearchFXEnable(int side) {
    side=side==1; World.Sys_UI.searchFXActive[side]=true; World.Sys_UI.searchFXStartTime[side]=World.pauseRelativeTime;
    World.Sys_UI.searchFXCursorX[side]=(float)World.cursorPos_x; World.Sys_UI.searchFXCursorY[side]=(float)World.cursorPos_y;
}

void SearchFXResetUpdate(u16 self) { Entity* e = &World.instances[self]; if (e->delayFinished >= World.pauseRelativeTime) {return;} flag_set(&e->entflags,EF_ACTIVE,false); }
void DelayedSpawnEnable(u16 self) { Entity* e = &World.instances[self]; e->timerFinished = World.pauseRelativeTime + e->delay; e->active = true; }

void CyberDecoySetTarget(u16), CyberDecoyExpired(u16);
bool CyberDecoyIsLive(u16*);
u16 CyberDecoyTarget(void);
void CyberDecoySpawned(u16 decoy) { World.decoyInstance = decoy; CyberDecoySetTarget(decoy); }
void CyberDecoyExpired(u16 decoy) { if (decoy >= World.instCount || World.instances[decoy].index != CYBER_DECOY_CONST) return; if (World.decoyActive) { World.decoyActive = false; World.decoyInstance = U16_MAX; CyberDecoySetTarget(U16_MAX); } }
bool CyberDecoyIsLive(u16* out) { if (!World.decoyActive) { World.decoyInstance = U16_MAX; return false; } u16 d = World.decoyInstance;
    if (d >= INSTS_1ST_IDX && d < World.instCount && World.instances[d].index == CYBER_DECOY_CONST) { if(out){*out = d;} return true; }
    for (u16 i = INSTS_1ST_IDX; i < World.instCount; ++i) {  if (World.instances[i].index == CYBER_DECOY_CONST) { World.decoyInstance = i; if (out) *out = i; return true; }  }
    
    World.decoyInstance = U16_MAX;
    return false;
}
    
void DelayedSpawnUpdate(u16 s) { Entity* e=&World.instances[s]; if(!e->active||e->timerFinished<=0.0||e->timerFinished>World.pauseRelativeTime){return;} e->active=false; if(!e->doSelfAfterList){return;} if(e->despawnInstead){DeleteInstance(s);/*despawn means gone, not deactivated*/}else flag_set(&e->entflags,EF_ACTIVE,true);}
void FuncWallShiftChildren(u16 self, V3 delta) { if (vabs(delta.x)+vabs(delta.y)+vabs(delta.z) < 0.00001f) {return;} for (u16 i=PLAYER1;i<World.instCount;++i) { if (fwParentOf[i]==self) { World.position[i]=V3_AplusB(World.position[i],delta); } } }
void FuncWallInitAfterLoad(u16 self) {
    Entity* e=&World.instances[self]; V3 prev=World.position[self]; float distTotal=V3_Dist(e->startPosition,e->targetPosition); float f=0; if((u8)e->funcState>FStat_AjarMovingTarget)f=e->ajarPercentage; else if(e->funcState==FStat_AjarMovingTarget) f=e->ajarPercentage;
    else if(e->funcState ==FStat_AjarMovingStart){f=1.0f-e->ajarPercentage;} if (f < 0.0f) f = 0.0f; if (f > 1.0f) f = 1.0f; V3 np=(distTotal > 0.0001f) ? V3_AplusB(e->startPosition,V3_ScaleByF(V3_Normalize(V3_AsubB(e->targetPosition,e->startPosition)),distTotal*f)) : e->startPosition; World.position[self]=np; if ((u8)e->funcState <= FStat_MovingTarget) { e->funcState = FStat_Start; e->percentMoved = 0.0f; } FuncWallShiftChildren(self,V3_AsubB(np,prev));
}

void FuncWallMoveStart(u16 self) { World.instances[self].funcState = FStat_MovingStart; World.instances[self].tickFinished = World.pauseRelativeTime + 10.0f; }
void FuncWallMoveTarget(u16 self) { World.instances[self].funcState = FStat_MovingTarget; World.instances[self].tickFinished = World.pauseRelativeTime + 10.0f; }
void FuncWallTargetted(u16 self) { Entity* e = &World.instances[self]; u8 st = (u8)e->funcState; bool toTarget = st == FStat_Start || st == FStat_MovingStart || st == FStat_AjarMovingTarget || (st > FStat_AjarMovingTarget && e->ajarPercentage > 0.0f); if (toTarget){FuncWallMoveTarget(self);} else{FuncWallMoveStart(self);} play_wav(sounds[76], AppliedFXVol(1.0f), World.position[self], true); }
void FuncWallUpdateInner(u16 self) {
    Entity* e = &World.instances[self]; if (e->funcState != FStat_MovingStart && e->funcState != FStat_MovingTarget) return; V3 goal = e->funcState == FStat_MovingStart ? e->startPosition : e->targetPosition; FuncStates doneState = e->funcState == FStat_MovingStart ? FStat_Start : FStat_Target; V3 delta = V3_AsubB(goal,World.position[self]);
    float distanceLeft = V3_Mag(delta), total = V3_Dist(e->startPosition,e->targetPosition), dist = e->speed * (float)World.deltaTime * World.timeScale; if (distanceLeft <= dist || e->tickFinished < World.pauseRelativeTime) { World.position[self]=goal; e->funcState=doneState; e->percentMoved=doneState == FStat_Target ? 1.0f : 0.0f; return; }
    if (distanceLeft > 0.0001f) World.position[self]=V3_AplusB(World.position[self],V3_ScaleByF(V3_Normalize(delta),dist)); if (total > 0.0001f) e->percentMoved = V3_Dist(e->startPosition,World.position[self]) / total;
}

void FuncWallUpdate(u16 self) { V3 prev = World.position[self]; FuncWallUpdateInner(self); FuncWallShiftChildren(self,V3_AsubB(World.position[self],prev)); }
void func_forcebridge(u16 self) {
    Entity* e = &World.instances[self]; e->tickFinished = World.pauseRelativeTime + 0.05f + (double)random_range(0.0f,1.0f); e->lerping = true; if(e->activatedScale.x <= 0.02f){e->activatedScale.x = 2.56f;} if(e->activatedScale.y <= 0.02f){e->activatedScale.y = 0.08f;} if(e->activatedScale.z <= 0.02f){e->activatedScale.z = 2.56f;}
    if(!e->active){ e->modelIndex=MAX_MDLS; World.col[self]=CT_NONE;} switch (e->fieldColor) { case ForceFieldColor_Red:e->texIndex=38; break; case ForceFieldColor_Green:e->texIndex=40; break; case ForceFieldColor_Blue:e->texIndex=39; break; case ForceFieldColor_Purple:e->texIndex=41; break; case ForceFieldColor_RedFaint:e->texIndex=198; break; }
}

void ForceBridgeActivate(u16 s, bool silent){Entity* e=&World.instances[s]; if(e->active){return;} if(!silent){play_wav(sounds[102], AppliedFXVol(1.0f), World.position[s], true);} flag_set(&e->entflags,EF_ACTIVE,true); e->modelIndex=78; World.col[s]=CT_BOX; e->active=e->lerping=true; World.scale[s]=(V3){ e->forceFieldDirectionX ? 0.1f : e->activatedScale.x,e->forceFieldDirectionY ? 0.1f : e->activatedScale.y,e->forceFieldDirectionZ ? 0.1f : e->activatedScale.z };}
void ForceBridgeDeactivate(u16 self, bool silent) { Entity* e = &World.instances[self]; if (!e->active) {return;} if (!silent) {play_wav(sounds[102], AppliedFXVol(1.0f), World.position[self], true);} e->active = false; e->lerping = true; }
void ForceBridgeToggle(u16 self) { if (World.instances[self].active) {ForceBridgeDeactivate(self,false); } else {ForceBridgeActivate(self,false);} }
void ForceBridgeUpdate(u16 self) {
    Entity* e = &World.instances[self]; if(e->tickFinished >= World.pauseRelativeTime){return;} e->tickFinished = World.pauseRelativeTime + 0.05f;
    if (e->active) {
        if (!e->lerping) return; float sx=e->forceFieldDirectionX ? lerp(World.scale[self].x,e->activatedScale.x,0.1f) : World.scale[self].x, sy=e->forceFieldDirectionY ? lerp(World.scale[self].y,e->activatedScale.y,0.1f) : World.scale[self].y, sz=e->forceFieldDirectionZ ? lerp(World.scale[self].z,e->activatedScale.z,0.1f) : World.scale[self].z; 
        World.scale[self]=(V3){sx,sy,sz}; if(vabs(e->activatedScale.x - sx) < 0.08f && vabs(e->activatedScale.y - sy) < 0.08f && vabs(e->activatedScale.z - sz) < 0.08f){World.scale[self]=e->activatedScale; e->lerping=false;}
    } else if (e->lerping) {float sx=e->forceFieldDirectionX ? lerp(World.scale[self].x,0.0f,0.1f) : World.scale[self].x, sy=e->forceFieldDirectionY ? lerp(World.scale[self].y,0.0f,0.1f) : World.scale[self].y, sz=e->forceFieldDirectionZ ? lerp(World.scale[self].z,0.0f,0.1f) : World.scale[self].z; World.scale[self]=(V3){sx,sy,sz}; if (sx < 0.08f || sy < 0.08f || sz < 0.08f) { e->modelIndex = MAX_MDLS; World.col[self] = CT_NONE; e->lerping=false;}}
}

void TriggerCounterTarget(u16 self, u16 activator) { UseTargets(activator,World.instances[self].targetIdx); }
void TriggerCounterDelayedTarget(u16 self, u16 act) { Entity* e=&World.instances[self]; e->recentMostActivator = act; e->delayFinished = World.pauseRelativeTime + e->delay; e->deferredIoflags = World.targetIOActive ? World.targetIOActivatorIoflags : (act<World.instCount?World.instances[act].ioflags:0u); e->deferredIoflagsHi = World.targetIOActive ? World.targetIOActivatorIoflagsHi : (act<World.instCount?World.instances[act].ioflagsHi:0u); }/*Capture the activating bits now, while the activator is still resolvable.*/
void TriggerCounterUpdate(u16 self) { Entity* e=&World.instances[self]; if (e->delayFinished <= 0.0 || e->delayFinished >= World.pauseRelativeTime) {return;} e->delayFinished = 0.0; u32 sf=World.targetIOActivatorIoflags,sfh=World.targetIOActivatorIoflagsHi; bool wa=World.targetIOActive; World.targetIOActive=true; World.targetIOActivatorIoflags=e->deferredIoflags; World.targetIOActivatorIoflagsHi=e->deferredIoflagsHi; UseTargets(e->recentMostActivator,e->targetIdx); World.targetIOActivatorIoflags=sf; World.targetIOActivatorIoflagsHi=sfh; World.targetIOActive=wa; }/*Marking the chain active stops UseTargets re-seeding from World.instances[activator], which by now points at the player's level.*/
void LogicRelayDelayTarget(u16 self, u16 act) { Entity* e=&World.instances[self]; e->recentMostActivator = act; e->delayFinished = World.pauseRelativeTime + e->delay; }
void LogicRelayUpdate(u16 self) { Entity* e=&World.instances[self]; if (e->delayFinished <= 0.0 || e->delayFinished >= World.pauseRelativeTime) {return;} e->delayFinished = 0.0; u32 savedFlags = World.targetIOActivatorIoflags, savedFlagsHi = World.targetIOActivatorIoflagsHi; World.targetIOActivatorIoflags = e->ioflags; World.targetIOActivatorIoflagsHi = e->ioflagsHi; UseTargets(e->recentMostActivator, e->targetIdx); World.targetIOActivatorIoflags = savedFlags; World.targetIOActivatorIoflagsHi = savedFlagsHi; }
void TriggerCounterTargetted(u16 self, u16 act) { Entity* e=&World.instances[self]; e->counter++; if (e->counter != e->countToTrigger) {return;} if (e->delay <= 0.0f){TriggerCounterTarget(self,act);}else{TriggerCounterDelayedTarget(self,act);} if (!e->dontReset){e->counter=0;} }
void SpawnManagerActivate(u16 self, bool alerted) { Entity* e=&World.instances[self]; e->alertEnemiesOnAwake=alerted; e->active=true; e->delayFinished=World.pauseRelativeTime; }
static bool SpawnPointIsClear(u16 spot) {
    V3 p=World.position[spot]; if(World.curLev>=LEVEL_CYBERSPACE||PositionVisibleFromPlayerCell(p.x,p.z)) {/*cyberspace never builds PVS bits (culling.c:151), so treat it as all-visible, matching TargetIDInPlayerPVS. No spawner sits on 13 today.*/
        ShapeCapsule ball={.tip=p,.base=p,.rad=0.48f};/* tip==base is a sphere */
        return AreaHasClearance(ball,ball,0.0f,LMASK_NPC_COLLIDESWITH,0xFFFFu);
    }
    return false;
}

void SpawnManagerUpdate(u16 self) {
    Entity* e=&World.instances[self]; if (World.paused || World.menuActive || !e->active) {return;}
    
    u16 levelNpcs = 0, count = 0;
    for (u16 i=INSTS_1ST_IDX;i<World.instCount;++i) {
        Entity* n = &World.instances[i]; if(!IdxIsNPC(n->index)){continue;}
        ++levelNpcs; if(!(n->entflags&EF_ACTIVE) || (n->health <= 0.0f && n->cyberHealth <= 0.0f)) {continue;}
        if(e->countOnlySameIndex && n->index!=e->spawnIndex) {continue;}
        
        ++count;
    }
    if (levelNpcs > 300) {return;}
    
    if (e->numberActive != count) {e->numberActive = count;}
    if (e->numberActive >= e->numberToSpawn) {return;}
    if (e->delayFinished >= World.pauseRelativeTime) {return;}
    
    e->delayFinished = World.pauseRelativeTime + (double)random_range(e->minDelayBetweenSpawns, e->maxDelayBetweenSpawns);
    if (World.diffCbt == 0) {return;}
    
    u16 spots = 0;
    for (u16 i=INSTS_1ST_IDX;i<World.instCount;++i) { if(World.instances[i].index == SPAWNPOINT_CONST){++spots;} }
    if (!spots) {return;}

    u16 spot = 0, valid = 0;
    for (u8 shot=0;shot<10;++shot) {
        u16 want = (u16)random_range_u32(0,spots - 1), cand = 0;
        for (u16 i=INSTS_1ST_IDX;i<World.instCount;++i) {
            if(World.instances[i].index != SPAWNPOINT_CONST) {continue;}
            
            if(want--==0){cand=i;break;}
        }
        if (!cand || !SpawnPointIsClear(cand)) {continue;}
        
        spot=cand;
        if (++valid > 8) break;
    }
    if (!valid) {return;}
    u16 npc = SpawnDynamicObject(e->spawnIndex,false); if (!EntIdxIsValid(npc)) {return;}
    
    World.position[npc]=World.position[spot]; World.scale[npc]=(V3){1.0f,1.0f,1.0f};
    if (!e->alertEnemiesOnAwake) { flag_set(&World.instances[npc].entflags, EF_WANDERING, true); return; } 
    
    AIAlert(npc);
    if (count + 1 >= e->numberToSpawn) { e->delayFinished = World.pauseRelativeTime + (double)e->allSpawnedResetDelay; }
}
void TextureChangerToggle(u16 self) {
    u16 alt = 0, glowAlt = 0;
    if (World.instances[self].index == 538) { alt = 1118; glowAlt = 1116; } else if (World.instances[self].index == 689) { alt = 841; glowAlt = 840; } else if (World.instances[self].index == 690) { alt = 844; glowAlt = 843; } else if (World.instances[self].index == 695) { alt = 858; glowAlt = 857; } else return;
    if (World.instances[self].curTex) { World.instances[self].texIndex = EDefs[World.instances[self].index].texIndex; World.instances[self].glowIndex = EDefs[World.instances[self].index].glowIndex; } else { World.instances[self].texIndex = alt; World.instances[self].glowIndex = glowAlt; } World.instances[self].curTex = !World.instances[self].curTex;
}

static u16 NearestGravityLiftForVisual(u16 visual) {
    u16 nearest=U16_MAX; float nearestDistSq=64.0f;
    for (u16 lift=INSTS_1ST_IDX;lift<World.instCount;++lift) {
        if (World.instances[lift].index != 596) continue;
        float dx=World.position[visual].x-World.position[lift].x;
        float dy=World.position[visual].y-World.position[lift].y;
        float dz=World.position[visual].z-World.position[lift].z;
        float distSq=dx*dx+dy*dy+dz*dz;
        if (distSq < nearestDistSq) { nearest=lift; nearestDistSq=distSq; }
    }
    return nearest;
}

void GravityLiftSyncVisuals(u16 lift) {
    if (lift < INSTS_1ST_IDX || lift >= World.instCount || World.instances[lift].index != 596) return;
    bool active=World.instances[lift].active; u16 texture=active ? 1246 : 1248, glow=active ? 1247 : 1249;
    for (u16 visual=INSTS_1ST_IDX;visual<World.instCount;++visual) {
        if (World.instances[visual].index == 112 && NearestGravityLiftForVisual(visual) == lift) {
            World.instances[visual].texIndex=texture; World.instances[visual].glowIndex=glow;
        }
    }
}

void GravityLiftSyncAllVisuals(void) {
    for (u16 visual=INSTS_1ST_IDX;visual<World.instCount;++visual) {
        if (World.instances[visual].index != 112) continue;
        if (World.instances[visual].texIndex == 1248) { World.instances[visual].glowIndex=1249; continue; } /* Explicit level material override (inactive). */
        u16 lift=NearestGravityLiftForVisual(visual);
        if (lift != U16_MAX) { bool active=World.instances[lift].active; World.instances[visual].texIndex=active ? 1246 : 1248; World.instances[visual].glowIndex=active ? 1247 : 1249; }
    }
}

void LogicTimerInitBeforeLoad(u16 self) { Entity* e=&World.instances[self]; if(e->timeInterval <= 0.0f){e->timeInterval=0.35f;} if(e->randomMin <= 0.0f){e->randomMin=5.0f;} if(e->randomMax <= 0.0f){e->randomMax=10.0f;} e->intervalFinished=World.pauseRelativeTime + (e->useRandomTimes ? (double)random_range(e->randomMin,e->randomMax) : (double)e->timeInterval); }
void LogicTimerUseTargets(u16 self) { UseTargets(self,World.instances[self].targetIdx); }
void LogicTimerUpdate(u16 self) { Entity* e=&World.instances[self]; if(!e->active || e->intervalFinished >= World.pauseRelativeTime){return;} e->intervalFinished=World.pauseRelativeTime + (e->useRandomTimes ? (double)random_range(e->randomMin,e->randomMax) : (double)e->timeInterval); LogicTimerUseTargets(self); }
void LogicTimerTargetted(u16 self, u16 activator) { (void)activator; World.instances[self].active = !World.instances[self].active; }
void ButtonSwitchHoldPose(Entity* e){ e->switchAnimFinished=0.0; ChangeAnim(e, e->active ? A_ACTIVATED : A_INACTIVE); }
void ButtonSwitchAnimate(Entity* e){
    if (e->animationNum >= MAXANIM) return;
    u8 move = e->active ? A_ACTIVATE : A_DEACTIVATE;
    const AnimationClip* c = &modelAnimationClips[e->animationNum][move];
    if (c->framerate <= 0 || c->speed <= 0 || c->frameEnd < c->frameStart) { ButtonSwitchHoldPose(e); return; }
    ChangeAnim(e, move); e->switchAnimFinished = World.pauseRelativeTime + (double)(c->frameEnd - c->frameStart + 1) / ((double)c->framerate * c->speed);
}
void ButtonSwitchInitAfterLoad(u16 self) { Entity* e=&World.instances[self]; e->delayFinished=0.0f; ButtonSwitchHoldPose(e); if(e->active){e->tickFinished=World.pauseRelativeTime + 1.5 + (double)random_range(0.0f,1.0f);} }
void ButtonSwitchUseTargets(u16 self) { Entity* e=&World.instances[self]; UseTargets(self,e->targetIdx); e->active=!e->active; ButtonSwitchAnimate(e); if(e->index == 689 || e->index == 690 || e->index == 695) { TextureChangerToggle(self); if(e->index == 689 && e->active){e->tickFinished=World.pauseRelativeTime + 1.5f;} } }
static __attribute__((noinline)) void UIBlockedBySecurity(V3 tetherPoint) { (void)tetherPoint; play_wav(sounds[468], AppliedFXVol(0.85f), (V3){0}, false);/*blocked_by_security*/ MFD_OpenData(World.Sys_UI.lastDataSideRH,8);/*MFDManager.BlockedBySecurity() opens tab 4 on the last-used data side and raises the blocked view there*/ CenterStatusPrint("%s",Sys_Text.stringTable[25]); }
static __attribute__((noinline)) void EntitySetLocked(Entity* e, bool locked) { flag_set(&e->entflags,EF_LOCKED,locked); }
typedef struct { u16 constIndex, sfxIndex; } ButtonSwitchSFXDef;
static const ButtonSwitchSFXDef buttonSwitchSFX[] = {{688,44},{689,40},{690,44},{691,41},{692,41},{694,45},{695,40}};
static void ButtonSwitchPlaySFX(u16 self) {
    const Entity* e=&World.instances[self]; for (size_t i=0;i<sizeof(buttonSwitchSFX)/sizeof(buttonSwitchSFX[0]);++i) if (buttonSwitchSFX[i].constIndex==e->index) { if (buttonSwitchSFX[i].sfxIndex < SOUNDS_COUNT) play_wav(sounds[buttonSwitchSFX[i].sfxIndex], AppliedFXVol(1.0f), World.position[self], true); return; }
}
void ButtonSwitchUse(u16 self, u16 activator) {
    Entity* e = &World.instances[self]; if(Cheats.superoverride || World.diffMis == 0){EntitySetLocked(e,false);} else if(GetCurrentLevelSecurity() > UsableOrDef((float)e->securityThreshold,100.0f)){UIBlockedBySecurity(World.position[self]); return;}
    if ((e->entflags & EF_LOCKED) != 0) { if (e->lockedMessageLingdex >= 0 && e->lockedMessageLingdex < T_LOGSTR_CNT) CenterStatusPrint("%s",Sys_Text.stringTable[e->lockedMessageLingdex]); if (e->SFXLockedIndex >= 0 && e->SFXLockedIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXLockedIndex], AppliedFXVol(1.0f), World.position[self], true); return; }
    ButtonSwitchPlaySFX(self);
    if (e->messageIndex >= 0 && e->messageIndex < T_LOGSTR_CNT) CenterStatusPrint("%s",Sys_Text.stringTable[e->messageIndex]); if (e->delay > 0.0f) { e->recentMostActivator = activator; e->delayFinished = World.pauseRelativeTime + e->delay; } else ButtonSwitchUseTargets(self);
}

void ButtonSwitchUpdate(u16 self) { double t=World.pauseRelativeTime; Entity* e=&World.instances[self]; if (e->switchAnimFinished > 0.0 && e->switchAnimFinished <= t){ ButtonSwitchHoldPose(e); } if (e->delayFinished > 0.0 && e->delayFinished < t){e->delayFinished=0.0; ButtonSwitchUseTargets(self);} if (e->index == 689 && e->active && e->tickFinished < t) { TextureChangerToggle(self); e->tickFinished=t+1.5f; } }
void HealingBedUse(u16 self, u16 owner) { Entity* e=&World.instances[self]; if (GetCurrentLevelSecurity() <= UsableOrDef(e->minSecurityLevel,100.0f)) { if(!e->broken){HealthManagerHealingBed(PLAYER1,UsableOrDef(e->amount,170.0f)); World.instances[PLAYER1].radiation=0.0f; World.invP1.radiationArea=false; CenterStatusPrint("%s",Sys_Text.stringTable[23],owner); play_wav(sounds[103], AppliedFXVol(1.0f), World.position[self], false);} else {CenterStatusPrint("%s",Sys_Text.stringTable[24],owner);} } else UIBlockedBySecurity(World.position[self]); }
int GeneralInvItem(int slot);
bool GeneralInvCanVaporize(int slot);
void GeneralInvRemove(int slot);
void VaporizeClick() {int slot=World.invP1.generalInvCurrent; if(!GeneralInvCanVaporize(slot))return; GeneralInvRemove(slot); play_wav(sounds[89],AppliedFXVol(1.0f),(V3){0},false);}
void CyborgConversionToggleTargetted() {bool active=(World.ressurectionActiveLevels>>World.curLev)&1u; flag_setu16(&World.ressurectionActiveLevels,(1u<<World.curLev),!active); if(World.curLev==6)flag_setu16(&World.ressurectionActiveLevels,(1u<<10|1u<<11|1u<<12),!active);/*Set groves 10,11,12 when 6 toggled, shared*/ play_wav(sounds[active ? 183 : 184], AppliedFXVol(1.0f), (V3){0.0f,0.0f,0.0f}, false);/*"vox_cybconvcancelled" : "vox_cybconvenabled"*/ CenterStatusPrint("%s",Sys_Text.stringTable[active ? 591 : 592]);}
bool PlayerInElevatorCell(void); bool FindElevatorKeypadPos(u8 level,V3* outPos);
bool ElevatorDefaultDestinationPos(u8 level,V3* outPos) { V3 panel; if (FindElevatorKeypadPos(level,&panel) && ElevatorDestinationPosNear(level,panel,outPos)) return true; return GetElevatorDestinationPos(level,0,outPos); }
bool ElevatorDestinationPosNear(u8 level,V3 fromPos,V3* outPos) {
    V3 best; float bestSq=0.0f; bool found=false;
    for (u8 n=0;n<MAX_LEVELS;++n) { V3 p; if(!GetElevatorDestinationPos(level,n,&p)) break; V3 d=V3_AsubB(p,fromPos); float sq=V3_dot(d,d); if(!found||sq<bestSq){best=p;bestSq=sq;found=true;} }
    if(found)*outPos=best; return found;
}

void EmailTargetted(u16 self) { Entity* e=&World.instances[self]; u16 idx=e->emailIndex; if(idx>=LOGCNT){return;} if(World.invP1.hasLog[idx]){return;} World.invP1.hasLog[idx]=World.invP1.hasNewEmail=true; World.invP1.lastAddedIndex=idx; if(Sys_Text.audioLogType[idx] == AudioLogType_Email){World.invP1.beepDone=true;/*TODO actually play beep sound 3 times when new unread email received*/} if(e->autoPlayEmail){PlayLastAddedLog(idx);} }
u8 OverloadButtonVisualState() { if (World.invP1.curWepHeat[World.invP1.wepCur] > 25.0f) {return 2;} if (World.invP1.overloadEnabled) {return 1;} return 0; }
void OverloadButtonAction() {
    static double overloadClickFinished = 0.0; if (overloadClickFinished >= World.pauseRelativeTime){return;} overloadClickFinished = World.pauseRelativeTime + 0.4; if (World.invP1.curWepHeat[World.invP1.wepCur] > 25.0f) { CenterStatusPrint("%s",Sys_Text.stringTable[12]);/*Weapon too hot*/ return; }
    if (World.invP1.overloadEnabled) { CenterStatusPrint("%s",Sys_Text.stringTable[13]);/*Overload disabled*/ World.invP1.overloadEnabled = false; } else { CenterStatusPrint("%s",Sys_Text.stringTable[17]);/*Overload enabled*/ World.invP1.overloadEnabled = true; }
}

static i16 targetIDText[INSTANCE_COUNT]; static double targetIDTextFinished[INSTANCE_COUNT],targetIDAttachedFinished[INSTANCE_COUNT]; static bool targetIDAttached[INSTANCE_COUNT];
void TargetIDReset() { mset(targetIDText,0,sizeof(targetIDText));mset(targetIDTextFinished,0,sizeof(targetIDTextFinished));mset(targetIDAttachedFinished,0,sizeof(targetIDAttachedFinished));mset(targetIDAttached,0,sizeof(targetIDAttached)); }
float TargetIDGetSensingRange(bool manual) { u8 ver=World.invP1.hwVers[HW_TID_IDX]; if(manual)return ver==0?12.0f:ver>=4?18.0f:13.0f; return ver==0?12.0f:ver<=2?0.0f:ver==3?13.0f:20.0f; }
float TargetIDGetTetherRange() { return World.invP1.hwVers[HW_TID_IDX]>=4?22.0f:15.0f; }
bool TargetIDInPlayerPVS(u16 npc) { if (unlikely(npc>=World.instCount))return false; if (unlikely(World.curLev >= LEVEL_CYBERSPACE)) return true;/*Culling is disabled in cyberspace (CullCore early-outs), so never gate there.*/ return gridCellStates[World.instances[npc].cellIndex] & CELL_VISIBLE; }
void TargetIDSendDamageReceive(u16 self,float damage,AttType attackType) {
    if(self>=World.instCount||!IdxIsNPC(World.instances[self].index))return; if(!TargetIDInPlayerPVS(self))return;/*out of PVS: never latch sensing state*/ Entity* npc=&World.instances[self];
    if(attackType==Att_Trnq){targetIDText[self]=536;targetIDTextFinished[self]=World.pauseRelativeTime;}
    else{float mh=npcTable[npc->index-419].health;targetIDText[self]=damage>mh*.75f?514:damage>mh*.50f?515:damage>mh*.25f?513:damage>0.0f?512:511;targetIDTextFinished[self]=World.pauseRelativeTime+(damage==0.0f?1.0:2.5);}
}

bool TargetIDShouldRender(u16 npc) {
    if(npc>=World.instCount||!IdxIsNPC(World.instances[npc].index))return false; Entity* e=&World.instances[npc];
    if(!(e->entflags&EF_ACTIVE)||(e->entflags&EF_DEAD)||e->health<=0.0f)return false;
    if(!TargetIDInPlayerPVS(npc))return false;/*NPC left the PVS after acquiring: stop sensing and stop drawing.*/
    i16 textIdx=TargetIDGetText(npc);
    bool hw=(World.invP1.hasHardware&HW_TID)!=0; u8 ver=World.invP1.hwVers[HW_TID_IDX]; bool sufficientHw=hw&&(ver==0||ver>=3);
    if(textIdx==511)return true;/*NO DAMAGE always displays, regardless of hardware ownership/version/distance.*/
    if(textIdx>=512&&textIdx<=515&&sufficientHw)return true;/*Damage status bypasses distance with sufficient Target Identifier hardware.*/
    if(V3_Dist(World.position[npc],World.position[PLAYER1])>10.0f)return false;
    if(targetIDAttached[npc]&&targetIDAttachedFinished[npc]<World.pauseRelativeTime)targetIDAttached[npc]=false;
    if(sufficientHw)return true;
    return targetIDAttached[npc];
}

i16 TargetIDGetText(u16 npc) { if(npc>=World.instCount)return -1; Entity* e=&World.instances[npc]; if(e->tranquilizeFinished>World.pauseRelativeTime)return 536; if(targetIDText[npc]&&targetIDTextFinished[npc]<=World.pauseRelativeTime)targetIDText[npc]=0; return targetIDText[npc]?targetIDText[npc]:-1; }
void CreateTargetIDInstance(float damage,u16 hitIdx,float tranq) {
    if(hitIdx==WORLD||hitIdx>=World.instCount)return; Entity* npc=&World.instances[hitIdx]; if(!(npc->entflags&EF_ACTIVE)||!IdxIsNPC(npc->index)||npc->health<=0.0f)return;
    if(!TargetIDInPlayerPVS(hitIdx))return;/*acquire is PVS-gated, not just rendering*/
    bool hw=(World.invP1.hasHardware&HW_TID)!=0; if(!hw&&tranq<=0.0f&&damage>0.0f)return; if(V3_Dist(World.position[hitIdx],World.position[PLAYER1])>TargetIDGetTetherRange())return;
    targetIDAttached[hitIdx]=true; targetIDAttachedFinished[hitIdx]=World.pauseRelativeTime+(hw?9999999.0:vmax(1.0f,tranq));
    if(tranq>0.0f){targetIDText[hitIdx]=536;targetIDTextFinished[hitIdx]=World.pauseRelativeTime+tranq;} else if(damage>=0.0f)TargetIDSendDamageReceive(hitIdx,damage,Att_None);
}
// PlayerEnergy
static const float  hwDrain[12][4] = {[3]={0.0921f,0.20478f,0.15354f,0.0f},[5]={0.24576f,0.61434f,0.28333f,0.30714f},[6]={0.010236f,0.0f,0.0f,0.0f},[7]={0.15354f,0.25596f,0.30714f,0.0f},[9]={0.0f,0.28333f,0.2125f,0.0f},[11]={0.51198f,0.51198f,0.51198f,0.51198f},};
static const u16 hwDrainJPM[12][4] = {[3]={9,20,15,0},[5]={24,60,105,30},[6]={1,0,0,0},[7]={15,25,30,0},[9]={0,16,12,0},[11]={50,50,50,50},};
static void DeactivateHardwareOnEnergyDepleted() { World.invP1.hardwareIsActive = 0; }
void TakeEnergy(float take) { if (World.invP1.energy <= 0.0f || Cheats.redbull) {return;} World.invP1.energy -= take; if (World.invP1.energy <= 0.0f) { World.invP1.energy = 0.0f; play_wav(sounds[84],AppliedFXVol(1.0f),(V3){0.0f,0.0f,0.0f},false);/*energy_gone*/ CenterStatusPrint("%s",Sys_Text.stringTable[314]); /*Power supply exhausted.*/ DeactivateHardwareOnEnergyDepleted(); } }
void GiveEnergy(float give,EnergyType type) { World.invP1.energy += give; if (World.invP1.energy > 255.0f) {World.invP1.energy = 255.0f;} if (type == EnergyType_Battery){play_wav(sounds[79],AppliedFXVol(1.0f),(V3){0.0f,0.0f,0.0f},false);/*batteryuse*/} else if (type == EnergyType_ChargeStation){play_wav(sounds[100],AppliedFXVol(1.0f),(V3){0.0f,0.0f,0.0f},false);/*chargingstation*/} }
bool PlayerIsMoving() { return V3_dot(World.velocity[PLAYER1],World.velocity[PLAYER1]) > 0.1f; }
void PlayerEnergyUpdate() {
    if (World.invP1.energyDrainTickFinished > World.pauseRelativeTime) return; World.invP1.energyDrainTickFinished = World.pauseRelativeTime + 0.1; bool anyDrain = false; u8 ver; World.invP1.drainJPM = 0;
    for (int hw=3;hw<=11;++hw) { u16 bit=(u16)(1u << hw); if (!(World.invP1.hardwareIsActive & bit) || hw == 4 || hw == 8 || hw == 10) continue;/*No energy usage*/ if (hw==9 && World.invP1.ladderState>0) continue;/*Booster boost is ignored on a ladder*/ if (hw==9 && !PlayerIsMoving()) continue;/*...and only burns energy while the player is actually moving*/ ver=World.invP1.hwVersSetting[hw]; float drain=hwDrain[hw][ver];  World.invP1.drainJPM += hwDrainJPM[hw][ver]; if (drain > 0.0f) { TakeEnergy(drain); anyDrain = true; } }
    if (anyDrain && World.invP1.energy <= 0.0f) { DeactivateHardwareOnEnergyDepleted(); World.invP1.drainJPM = 0; } // Depleted
}
void ChargeStationUse(u16 self, u16 owner) {
    Entity* e=&World.instances[self];
    if (GetCurrentLevelSecurity() > UsableOrDef(e->minSecurityLevel,100.0f)) { UIBlockedBySecurity(World.position[self]); return; }
    if (e->rechargeFinished < World.pauseRelativeTime) {
        if (World.invP1.energy >= 255.0f) { CenterStatusPrint("%s",Sys_Text.stringTable[303]); return; }
        GiveEnergy(UsableOrDef(e->amount,170.0f),EnergyType_ChargeStation);
        if (e->damage > 0.0f) { DamageData dd={0}; dd.damage = vmin(e->damage,World.instances[PLAYER1].health - 1.0f); if (dd.damage > 0.0f) TakeDamage(PLAYER1,dd); } /*zap, never lethal*/
        if (e->usedMsgLingdex < T_LOGSTR_CNT) CenterStatusPrint("%s",Sys_Text.stringTable[e->usedMsgLingdex]);
        if (e->requireRecharge) e->rechargeFinished = World.pauseRelativeTime + UsableOrDef(e->resetTime,150.0f);
        UseTargets(owner,e->targetIdx);
    } else if (e->rechargeMsgLingdex < T_LOGSTR_CNT) CenterStatusPrint("%s",Sys_Text.stringTable[e->rechargeMsgLingdex]);
}
// GeneralInventory
void MFD_ShowGeneralItem();
int GeneralInvItem(int slot) { if (slot<0 || slot>=14) return -1; if (!slot) return 81; int item=World.invP1.genInvIdxRef[slot]; if (item<0) return -1; item=UseableFromConst(item); return IdxIsGenericItem(item + 307) ? item : -1; }
bool GeneralInvCanUse(int slot) { int item=GeneralInvItem(slot); return slot>0 && (item==52 || item==53 || item==55); }
bool GeneralInvCanVaporize(int slot) { int item=GeneralInvItem(slot); return slot>0 && item>=0 && (item<6 || item==33 || item==35 || item==58 || item==62); }
void GeneralInvRemove(int slot) {
    if (slot<=0 || slot>=14) return;

    for (int i=slot;i<13;++i) { World.invP1.genInvIdxRef[i]=World.invP1.genInvIdxRef[i+1]; World.invP1.generalInvCustIdx[i]=World.invP1.generalInvCustIdx[i+1]; }
    World.invP1.genInvIdxRef[13] = -1;
    World.invP1.generalInvCustIdx[13] = U16_MAX;
    World.Sys_UI.generalClickSlot = -1;
    if (World.invP1.generalInvCurrent==slot) {
        if (GeneralInvItem(slot) < 0) {
            World.invP1.generalInvCurrent = 0;
            for (int i=13;i>=0;--i) {
                if (GeneralInvItem(i) >= 0) { World.invP1.generalInvCurrent = (u8)i; break; }
            }
        }
    } else if (World.invP1.generalInvCurrent>slot) World.invP1.generalInvCurrent--;
}

void GeneralInvClick(int buttonIdx) { int item = GeneralInvItem(buttonIdx); if (item < 0) return; World.invP1.generalInvCurrent = (u8)buttonIdx; World.Sys_UI.mouseClickHeldOverGUI = World.uiIsBlocking = true; MFD_ShowGeneralItem(); }
void GeneralInvApply(int buttonIdx) {
    if (!GeneralInvCanUse(buttonIdx) || World.invP1.holdingObject) return;
    
    int item = GeneralInvItem(buttonIdx);
    if ((item == 55 ? World.instances[PLAYER1].health : World.invP1.energy) >= 255.0f) { CenterStatusPrint("%s", Sys_Text.stringTable[303]); return; }
    if (item == 55) World.instances[PLAYER1].health = 255.0f;
    else            GiveEnergy(item == 52 ? 83.0f : 255.0f, EnergyType_Battery);
    GeneralInvRemove(buttonIdx);
}

bool GeneralInvTake(int slot) {
    int item=GeneralInvItem(slot); if (slot<=0 || item<0 || World.invP1.holdingObject) return false;
    
    ResetHeldItem();
    World.invP1.heldObjectIndex = (u16)(item+307);
    World.invP1.heldObjectCustIdx = World.invP1.generalInvCustIdx[slot];
    World.invP1.holdingObject = true;
    GeneralInvRemove(slot);
    ForceInventoryMode();
    CenterStatusPrint("%s%s",Sys_Text.stringTable[item+326],Sys_Text.stringTable[319]); return true;
}

static bool GrenadeIsNPCMine(u16 self) { return World.layer[self] != L_PlayerBullets; }
void ApplyImpactForce(u16 target, float vel, V3 normal, V3 pt) {
    if (target == WORLD || target >= World.instCount || vel <= 0.0f){return;}
    Entity* e = &World.instances[target]; if((e->entflags & EF_DEAD) || (!(e->entflags & EF_RIGIDBODY) && target != PLAYER1)){return;}
    
    V3 n = V3_Normalize(normal); if (V3_Mag(n) < 0.0001f) {n = (V3){0.0f,1.0f,0.0f};/*At least make it pop off the floor*/} AddForce(target,V3_ScaleByF(n,vel),true); V3 lever = V3_AsubB(pt, World.position[target]); World.angularVelocity[target].x += lever.y * vel * 0.05f; World.angularVelocity[target].z += -lever.x * vel * 0.05f; (void)pt; // torque applied relative to point lever arm
}

void ApplyImpactForceWithSound(u16 target, float vel, V3 normal, V3 pt) {
    ApplyImpactForce(target, vel, normal, pt);
    if (vel > 2.0f) play_wav_ext(sounds[523], AppliedFXVol((vel / 10.0f) * 0.3f), pt, true, random_range(-3.8631f,3.1564f));
}

void ApplyImpactForceSphere(DamageData* dd, V3 center, float radius, float baseVel) { 
    if (radius <= 0.0f || baseVel <= 0.0f) return; float r2 = radius * radius; float origDamage = dd ? dd->damage : 0.0f;
    float damageScale = vmin(origDamage / 100.0f, 2.0f);/*measured off the original damage, so it is the same for every body in the blast*/
    for (u16 i = PLAYER1; i < World.instCount; i++) {
        Entity* e = &World.instances[i]; if (!(e->entflags & EF_ACTIVE) || (e->entflags & EF_DEAD)) { if (!IdxIsNPC(e->index) || !(World.layer[i] & L_Corpse)) continue;/*Corpses are normally skipped here, but a dead NPC on the corpse layer stays in the blast so grenades can vaporize it (Death() re-entry).*/ } if (!(e->entflags & EF_RIGIDBODY) && !IdxIsNPC(e->index) && i != PLAYER1 && !IsDamageable(e)) continue;/*IsDamageable() widens the blast to static damageable props (sec_cpunode 478/479, consoles, cameras) which have health but no EF_RIGIDBODY, so grenades used to skip them entirely.  ApplyImpactForce below still no-ops for them, so only the damage applies.*/ float sqd = V3_SqDist(World.position[i], center); if (sqd > r2) continue; float dist = vsqrtf(sqd);
        if (dist >= 4.0f) { RaycastHit sight = Raycast(center, V3_ScaleByF(V3_AsubB(World.position[i],center), 1.0f / dist), radius + 0.02f, LMASK_EXPLOSION); if (!(sight.hit && sight.hitInstanceIndex == i)) continue; }
        
        float distPenalty = (radius - dist) / radius; if (distPenalty < 0.0f) distPenalty = 0.0f;
        V3 normal; if(dist > 0.0001f){normal=V3_ScaleByF(V3_AsubB(World.position[i],center), 1.0f / dist);}else{normal = (V3){0.0f,1.0f,0.0f};}
        float impactVel = baseVel * distPenalty;
        if (dd && origDamage > 0.0f && i != dd->owner) {
            DamageData splash=*dd;
            if (i == PLAYER1) splash.damage *= 0.5f; /*halve for player*/
            splash.damage *= distPenalty;
            float saturation = origDamage * 0.33f;
            if (splash.damage < saturation) splash.damage = saturation;
            splash.impactVelocity = impactVel;
            splash.hitpoint = World.position[i];
            splash.attacknormal = normal;
            TakeDamage(i,splash);
        }
        if (World.layer[i] & L_NPC) impactVel *= 0.5f;
        ApplyImpactForce(i,vmin(impactVel,(0.5f + 5.0f * World.mass[i]) * damageScale),normal,World.position[i]);
    }
}

static void SpawnExplosionPreset(u16 type,V3 pos) { const PSysDef* preset=PSysTypeGet(type); if(!preset)return; PSysDef def=*preset; def.pos=pos; PSysAdd(&def); }
void SpawnExplosionEffect(V3 pos, int explosionType) {
    switch (explosionType) {
        case 3:  SpawnExplosionPreset(165,pos); SpawnExplosionPreset(168,pos); break;/*GasExplosions*/
        case 4:  SpawnExplosionPreset(162,pos); SpawnExplosionPreset(163,pos); SpawnExplosionPreset(164,pos); break;/*GrenadeEMPExplosions*/
        default: SpawnExplosionPreset(5,pos);   SpawnExplosionPreset(6,pos); break;/*GrenadeFragExplosions*/
    }
}

void GrenadeExplode(u16 self) {
    if(self>=World.instCount)return; Entity* e = &World.instances[self]; if(!(e->entflags&EF_ACTIVE))return; flag_set(&e->entflags,EF_ACTIVE,false);
    DamageData dd={.damage=e->damage,.penetration=e->strength,.offense=e->speed,.armorvalue=0.0f,.defense=0.0f,.impactVelocity=e->damage*1.5f,.attacknormal=(V3){0.0f,1.0f,0.0f},.hitpoint=World.position[self],.attackType=e->attackType,.owner=e->recentMostActivator,.isOtherNPC=false,.berserkActive=(World.invP1.patchActive & PATCH_BERSERK) != 0};
    i16 idx=GrenadeTypeFromConst(e->index); float radius=(idx>=7&&idx<=13) ? grenadeRadius[idx-7] : (e->strength>0.0f ? e->strength : 4.0f); ApplyImpactForceSphere(&dd,World.position[self],radius,e->damage * 0.1f);  if (!GrenadeIsNPCMine(self)) { World.invP1.makingNoise = true; World.invP1.noiseFinished = World.pauseRelativeTime + 2.0; } int soundIndex=60,explosionType=2;
    switch (idx) {case 7: case 11: soundIndex = 64; World.fogFac += 5; explosionType = 1; break;/*frag, mine*/ case 8: case 10: soundIndex = 60; World.fogFac += 7; explosionType = 2; break;/*conc, earth*/ case 9:  soundIndex = 67; explosionType = 4; break;/*emp*/ case 12: soundIndex = 60; World.fogFac += 6;  explosionType = 2; break;/*nitro*/ case 13: soundIndex = 63; World.fogFac += 10; explosionType = 3; break;/*gas*/}
    play_wav(SoundPath(soundIndex), AppliedFXVol(1.0f), World.position[self], true); SpawnExplosionEffect(World.position[self],explosionType); Shake(-1.0f); DeleteInstance(self);
}

void GrenadeActivate(u16 self) {
    if (self >= World.instCount) return;
    
    Entity* e = &World.instances[self];
    i16 idx = GrenadeTypeFromConst(e->index);
         if (idx == 10) { World.invP1.earthShakerTimeSetting = vclamp((float)World.invP1.earthShakerTimeSetting,4.0f,60.0f); e->timerFinished = World.pauseRelativeTime + World.invP1.earthShakerTimeSetting;}
    else if (idx == 12) { World.invP1.nitroTimeSetting       = vclamp((float)World.invP1.nitroTimeSetting,      2.0f,60.0f); e->timerFinished = World.pauseRelativeTime + World.invP1.nitroTimeSetting;}
}
void GrenadeUpdate(u16 self) { Entity* e = &World.instances[self]; i16 idx=GrenadeTypeFromConst(e->index); if(idx == 14){GrenadeExplode(self); return;} /*Plastique*/ if((idx == 10 || idx == 12) && e->timerFinished <= World.pauseRelativeTime) { GrenadeExplode(self); return; } if (idx == 11) { V3 origin = World.position[self]; float pr=1.451f;  bool npcMine = GrenadeIsNPCMine(self);  for (u16 i = PLAYER1; i < World.instCount; i++) { Entity* o = &World.instances[i]; if (i == self || !(o->entflags & EF_ACTIVE) || (o->entflags & EF_DEAD)) continue; if (npcMine ? (i != PLAYER1) : (i == PLAYER1 || !IdxIsNPC(o->index))) continue; float rr = pr + ((World.col[i] == CT_CAP || World.col[i] == CT_SPH) ? World.colliderSize[i].x : 0.0f); if (V3_SqDist(World.position[i], origin) < (rr * rr)) { GrenadeExplode(self); return; } } } }
void GrenadeOnCollision(u16 self) { i16 idx=GrenadeTypeFromConst(World.instances[self].index); if ((idx >= 7 && idx <= 9) || idx == 13) GrenadeExplode(self); }
float GetDamageTakeAmount(DamageData* dd) { if (!dd) return 0.0f; float take = dd->damage; if (take <= 0.0f) return 0.0f; if (dd->berserkActive) take *= 4.0/*Quad Damage!*/; if (dd->defense > 0.0f && dd->offense < dd->defense) { float r = (dd->defense - dd->offense) / dd->defense; if (r > 0.85f) r = 0.85f; take *= (1.0f - r); } if (dd->armorvalue > 0.0f && dd->penetration < dd->armorvalue) { float a = (dd->armorvalue - dd->penetration) / dd->armorvalue; if (a > 0.85f) a = 0.85f; take *= (1.0f - a); } if (take < 0.0f) take = 0.0f; return take; }
void SpawnImpactEffect(u16 impactType, V3 pos) { if (impactType == 0 || impactType == U16_MAX) return; u16 fx = SpawnDynamicObject(impactType, false); if (!EntIdxIsValid(fx)) return; World.position[fx] = pos; Entity* e = &World.instances[fx]; flag_set(&e->entflags, EF_ACTIVE, true); if (e->itemLifeTime <= 0.0f) e->itemLifeTime = 1.0f; e->delayFinished = World.pauseRelativeTime + e->itemLifeTime; }
void ExitCyberspace(void) { UIExitCyberspace(); if (World.curLev != LEVEL_CYBERSPACE) return; if (World.instances[PLAYER1].cyberHealth <= 0.0f) World.instances[PLAYER1].cyberHealth = 1.0f; LoadLevel((World.cyberReturnLevel >= 0 && (u8)World.cyberReturnLevel < World.numLevels) ? (u8)World.cyberReturnLevel : (World.startLevel < World.numLevels ? World.startLevel : 0), World.cyberReturnPos); }
void ReduceCurrentLevelSecurity(SecurityType stype) {
    static const u32 score[3]={4u,10u,27u}; u8 lev = World.curLev; if (lev >= MAX_LEVELS || stype == SecurityType_None) return;
    
    u8* tot[3] = { &World.levelCameraCount[lev],   &World.levelSmallNodeCount[lev],   &World.levelLargeNodeCount[lev]};
    u8* dcd[3] = { &World.levCamDestroyedCnt[lev], &World.levSmNodeDestroyedCnt[lev], &World.levNodeDestroyedCnt[lev]};
    u32 t=(u32)stype-1u;
    if (*dcd[t] < 255u) (*dcd[t])++;
    u32 total=0u, dead=0u;
    for (u32 i=0u;i<3u;++i) { total += (u32)*tot[i]*score[i]; dead += (u32)*dcd[i]*score[i]; }
    if (!total) return;
    
    u8 before = World.levelSecurity[lev], after;
    //  ceiling, so any surviving weight keeps the level above 0 and only killing every security object reaches it
    after = dead >= total ? 0u : (u8)(((total-dead)*100u + total-1u) / total);
    if (after > 100u) after = 100u;
    if (after >= before && before) after = before > 1u ? (u8)(before-1u) : before; // a kill must never read as 58 -> 58
    World.levelSecurity[lev] = after;
    CenterStatusPrint("%s%d%s", Sys_Text.stringTable[306], (int)World.levelSecurity[lev], Sys_Text.stringTable[307]);
}

static void CodeScreensSetForLevel(u8 lev) {
    if (lev < 1 || lev > 6) return;

    i8 *code = &World.lev1SecCode; if (code[lev-1] < 0) { code[lev-1] = (i8)random_range_u8(0u,9u); }
    u8 digit = (u8)code[lev-1];
    u8 entryLevel = World.currentLevel;
    for (u8 l = 0; l < World.numLevels; ++l) {
        if (l != World.currentLevel) SetLevelPointers(l);
        for (u16 i = INSTS_1ST_IDX; i < World.instCount; ++i) { Entity* s = &World.instances[i]; if (s->index != 551 || !s->codeScreen) continue; CodeScreenShowDigit(i, digit); }
    }
    if (World.currentLevel != entryLevel) SetLevelPointers(entryLevel);
}

static bool CPUNodesRemainOnLevel(void) {
    for (u16 i = INSTS_1ST_IDX; i < World.instCount; ++i) { const Entity* n = &World.instances[i]; if ((n->index == 478 || n->index == 479) && !(n->entflags & EF_DEAD_CHECKS_DONE)) return true; }
    return false;
}

void ProjectileEffectImpactOnCollision(u16 self,u16 hitIdx, V3 hitPos,V3 hitNormal) {
    if(self>=World.instCount||hitIdx>=World.instCount)return; Entity* e = &World.instances[self]; if (hitIdx == e->recentMostActivator) return;/*other.gameObject == host*/ e->counter++;/*numHits++*/
    DamageData dd={.damage=e->damage,.penetration=e->strength,.offense=e->speed,.armorvalue=0.,.defense=0,.impactVelocity=e->damage*1.5f,.attacknormal=hitNormal,.hitpoint=hitPos,.attackType=e->attackType,.owner=e->recentMostActivator,.isOtherNPC=IdxIsNPC(World.instances[hitIdx].index),.berserkActive=(e->recentMostActivator==PLAYER1&&(World.invP1.patchActive & PATCH_BERSERK)!=0)};
    if (dd.berserkActive) { dd.damage *= 4.0f; dd.berserkActive = false; }
    Entity* hit = &World.instances[hitIdx]; if (IdxIsNPC(hit->index)) { NPCTable* nt = &npcTable[hit->index - 419]; dd.armorvalue = nt->armorvalue; dd.defense = nt->defense; } if (e->lookUpIndex == 5) { ApplyImpactForceSphere(&dd, World.position[self], 3.2f, 1.0f); World.fogFac += 4; }/*Railgun sphere impact*/
    dd.damage = GetDamageTakeAmount(&dd);
    SpawnProjectileImpactParticles(e->index,hitPos,hitNormal);
    bool hostIsNPC=e->recentMostActivator<World.instCount&&IdxIsNPC(World.instances[e->recentMostActivator].index);
    if (hit->health > 0.0f || hit->cyberHealth > 0.0f) {
        if (e->counter < e->countToTrigger) dd.damage *= 0.85f;/*per-hit falloff*/ dd.impactVelocity = dd.damage * 1.5f; if (e->counter > 0) dd.impactVelocity /= 3.0f; float dmgFinal = TakeDamage(hitIdx,dd); float tranq=-1.0f;
        if (dd.isOtherNPC) {
            if (!(hit->entflags & EF_ASLEEP)) { World.invP1.justFired = World.pauseRelativeTime; }
            if (dd.attackType == Att_Trnq) {
                float stunAmount = vclamp(3.0f + (World.invP1.weaponEnergySetting[15] / 100.0f) * 7.0f, 3.0f, 10.0f);
                tranq = Tranquilize(hitIdx, stunAmount, true);
            }
        }
        if (dmgFinal < 0.0f) { dmgFinal = 0.0f; }
        CreateTargetIDInstance(dmgFinal, hitIdx, tranq);
    }
    if (World.curLev != LEVEL_CYBERSPACE && !hostIsNPC) { ApplyImpactForceWithSound(hitIdx,dd.impactVelocity,dd.attacknormal,hitPos); }
    if(e->countToTrigger<1)e->countToTrigger=1; if (e->counter >= e->countToTrigger) { if (e->despawnInstead){DeleteInstance(self);}else{flag_set(&e->entflags,EF_ACTIVE,false);} }
}
void ProjectileEffectImpactInitAfterLoad(u16 self) { if(self>=World.instCount)return;Entity* e=&World.instances[self]; e->counter=0;e->currentTargetIdx=0;e->lookUpIndex=(e->index==484||e->index==491)?5:0;if(e->countToTrigger<1){e->countToTrigger=e->index==485?5:(e->index==495?2:1);} }
 // None  Melee  MelEn  EnBm   Mag    Proj   Needle ProjEB ProjLn Gas    Tranq  Drill
static const float attackTypeMult[7][12]={[NPCType_Mutant]={1,1,1,1,0,1,2,1,1,2,1,1},[NPCType_Supermutant]={1,1,1,1,0,1,1,1,1,1.5,1,1},[NPCType_Robot]={1,1,1,1,4,1,0,1,1,0,1,1},[NPCType_Cyborg]={1,1,1,1,2,1,1,1,1,1,1,1},[NPCType_Supercyborg]={1,1,1,1,2,1,0,1,1,0,1,1},[NPCType_MutantCyborg]={1,1,1,1,0.5,1,2,1,1,2,1.5,1},[NPCType_Cyber]={1,1,1,1,1,1,1,1,1,1,1,0}}; // Attack type damage multiplier table [NPCType][AttType], 1.0f = no change, 0.0f = immune, other = multiplier
static const i16 objectDeathSound[] = {[458]=63,[459]=66,[460]=66,[464]=62,[465]=532,[466]=532,[467]=532,[468]=532,[469]=532,[470]=532,[471]=532,[472]=62,[473]=62,[474]=62,[475]=62,[476]=62,[477]=61,[478]=65,[479]=69,[525]=68,[526]=68,};
static bool IsCyberEntity(u16 self) { if (World.curLev == LEVEL_CYBERSPACE){return true;} Entity* e=&World.instances[self]; if (self != PLAYER1 && e->cyberHealth > 0.0f){return true;} return (IdxIsNPC(e->index) && (e->index - 419) > 23);/*24-28 are cyber enemies*/}
static float ApplyAttTypeAdjustments(u16 self,float take,AttType at) { if (!IdxIsNPC(World.instances[self].index) || World.instances[self].health <= 0.0f){return take;} NPCType t = npcTable[World.instances[self].index - 419].type; if (at >= 12){return take;} return take * attackTypeMult[t][at]; }
static void UseDeathTargets(u16 self) { if(self == PLAYER1){return;} if (World.instances[self].targetOnDeathIdx != IO_NONE) UseTargets(self,World.instances[self].targetOnDeathIdx); }
static void TeleportAway(u16 self) { 
    if (World.instances[self].entflags & EF_TELEPORT_ON_DEATH) {return;}
    
    flag_set(&World.instances[self].entflags,EF_TELEPORT_ON_DEATH,true);
    const PSysDef* dgo = PSysTypeGet(189);
    if (dgo) { PSysDef d = *dgo; d.pos = World.position[self]; d.rotation = World.rotation[self]; PSysAddEx(&d, self); }
    World.col[self] = CT_NONE;
    World.gravity[self] = 0.0f;
    World.velocity[self] = World.angularVelocity[self] = (V3){0};
    World.instances[self].modelIndex = U16_MAX; 
    play_wav(sounds[106]/*misc/teleport*/,AppliedFXVol(1.0f),World.position[self],false);
}

static void DropSearchables(u16 self) {
    for (int i=0;i<4;i++) {
        if(World.instances[self].contents[i]<=-1){continue;}
        
        u16 spawned=SpawnDynamicObject(World.instances[self].contents[i]+307,true);
        if(EntIdxIsValid(spawned)){World.position[spawned] = World.position[self]; World.instances[spawned].custIdx[0] = World.instances[self].custIdx[i];}
        else{CenterStatusPrint("BUG: Failed to make search obj.");}
        
        World.instances[self].contents[i]=World.instances[self].custIdx[i] = -1;}
}

static void CreateDeathEffects(u16 self,u16 fxPoolType) { if (fxPoolType == 0) {return; /*PoolType_None*/} if (!IdxInBounds((int)fxPoolType) || IdxIsGeometry((int)fxPoolType)) {return; /*PoolType is an ordinal enum (None..LeafBurst, 0..29), not a const index.  Feeding one to SpawnDynamicObject makes it try to spawn a level chunk, which is where "Indices 0 to 306 (level chunks) not possible when not on edit mode!" came from.  Refuse here so no caller can reach that.*/} V3 pos = World.position[self]; if (World.col[self] != CT_NONE) { pos = V3_AplusB(pos,World.colliderCenter[self]); } SpawnImpactEffect(fxPoolType, pos); }
static void HideSelf(u16 self) { if (World.instances[self].index == 279) {return; /*tv screens keep mesh visible*/} World.instances[self].modelIndex = MAX_MDLS; World.gravity[self] = 0.0f; }
static void SpawnSecCpuNodeGibs(u16 self) {
    if (World.instances[self].index != 478) return;
    V3 pos = World.position[self]; Quaternion rot = World.rotation[self];
    for (u16 gibConst = 840; gibConst <= 853; ++gibConst) {
        u16 gib = SpawnDynamicObject(gibConst, false);
        if (!EntIdxIsValid(gib) || gib == self) continue;
        World.position[gib] = pos; World.rotation[gib] = rot;
        World.velocity[gib] = (V3){0.0f, World.velocity[self].y, 0.0f};
        World.gravity[gib] = 1.0f; World.layer[gib] = L_Corpse;
    }
}
static void NPCDeath(u16 self) { if (World.instances[self].entflags & EF_DEAD_CHECKS_DONE) {return;} flag_set(&World.instances[self].entflags,EF_DEAD_CHECKS_DONE,true); CreateDeathEffects(self,World.instances[self].deathBurst); if (World.instances[self].index == 419) play_wav(sounds[64], AppliedFXVol(1.0f), World.position[self], true);/*npc_autobomb: explosion1*/ if (npcTable[World.instances[self].index - 419].type == NPCType_Cyber) DeleteInstance(self); }
static void ObjectDeath(u16 self) {
    Entity* e = &World.instances[self]; if (World.instances[self].entflags & EF_DEAD_CHECKS_DONE) return;
    
    if (World.instances[self].entflags & EF_DEATH_BURST_DONE) { CreateDeathEffects(self,World.instances[self].deathBurst); DropSearchables(self); if (World.instances[self].index != 279){World.col[self]=CT_NONE;} HideSelf(self); } else { World.col[self] = CT_NONE; DropSearchables(self); CreateDeathEffects(self,World.instances[self].deathBurst); }
    flag_set(&World.instances[self].entflags,EF_DEAD_CHECKS_DONE,true);
    { SecurityType stype = SecurityType_None; if(World.instances[self].index == 477){stype=SecurityType_Camera;}else if(World.instances[self].index == 479){stype=SecurityType_NodeSmall;} else if(World.instances[self].index == 478){stype=SecurityType_NodeLarge;} if(stype != SecurityType_None){ReduceCurrentLevelSecurity(stype);} }
    if ((World.instances[self].index == 478 || World.instances[self].index == 479) && !CPUNodesRemainOnLevel()) CodeScreensSetForLevel(World.curLev);/*sec_cpunode/sec_cpunode_small: the last node on the level reveals its security code on the displays and stops the flicker.  Independent of the level security percentage above -- revealing the code is a node-behaviour rule, not a percentage one.*/
    u16 idx = World.instances[self].index; SpawnSecCpuNodeGibs(self); play_wav(SoundPath((idx < 527 && objectDeathSound[idx] != 0) ? objectDeathSound[idx] : 62/*crate_break*/), AppliedFXVol(1.0f), World.position[self], true); if(e->deathBurst != 0){HideSelf(self);}
}

static void ScreenDeath(u16 self) { Entity* e=&World.instances[self]; if(e->entflags & EF_DEAD_CHECKS_DONE){return;} flag_set(&e->entflags,EF_DEAD_CHECKS_DONE,true); play_wav(sounds[69], AppliedFXVol(1.0f), World.position[self], true);/*screen_destroy*/ if (e->entflags & EF_DEATH_BURST_DONE) ObjectDeath(self);/*gib path*/ }
static inline bool IsGrenade(u16 i) { return ((i >= 314 && i <= 320) || i == 370 || i == 372 || i == 387 || i == 389 || (i >= 402 && i <= 404)); }
static bool CanVaporize(u16 self) { Entity* e=&World.instances[self]; if (e->index == 477/*sec_camera*/ || IsGrenade(e->index)) {return false;} if (IdxIsNPC(e->index)) {return ai_has_search_collider((u16)(e->index - 419));} return true; }
static void SpawnDeathEffectAt(u16 type, V3 pos) { const PSysDef* preset = PSysTypeGet(type); if (!preset) return; PSysDef def = *preset; def.pos = pos; PSysAdd(&def); }
static void VaporizeCorpse(u16 self,bool energyVaporized) {
    Entity* e=&World.instances[self]; if (!CanVaporize(self)) {return;}
    
    DropSearchables(self);
    V3 pos = World.position[self];
    if (World.col[self] != CT_NONE) { pos = V3_AplusB(pos,World.colliderCenter[self]); }
    if (e->deathBurst != 0 && IdxInBounds((int)e->deathBurst) && !IdxIsGeometry((int)e->deathBurst)) { CreateDeathEffects(self,e->deathBurst); }
    else { SpawnDeathEffectAt(energyVaporized ? 188 : 187, pos); }
    e->modelIndex=MAX_MDLS;
    if (IdxIsNPC(e->index) || IdxIsSearchable(e->index)) DeleteInstance(self);
}

static void Death(u16 self,bool energyVaporized) {
    Entity* e = &World.instances[self];
    if (e->entflags & EF_DEAD_CHECKS_DONE) {
        if (!IdxIsNPC(e->index) || !(World.layer[self] & L_Corpse) || self == PLAYER1 || IsGrenade(e->index) || (e->entflags & (EF_TELEPORT_ON_DEATH | EF_ACT_AS_CORPSE_ONLY))) return;
        VaporizeCorpse(self, energyVaporized); return;
    }
    UseDeathTargets(self); bool isNPC = IdxIsNPC(e->index); bool isObj = IdxIsDynamicObject(e->index); if (e->entflags & EF_ACT_AS_CORPSE_ONLY) { e->entflags |= EF_DEAD_CHECKS_DONE; return; }
    if (e->index == 477) { e->deathBurst = 725; ObjectDeath(self); DeleteInstance(self); return; }
    if (e->index == 478 || e->index == 479) { ObjectDeath(self); DeleteInstance(self); return; }
    
    bool vaporize = IdxIsCorpse(e->index), isGrenade = IsGrenade(e->index), doTeleport = (e->entflags & EF_TELEPORT_ON_DEATH) != 0;
    if (e->iceActive) World.col[self] = CT_NONE;
         if (vaporize && e->index != 477/*sec_camera*/ && !isGrenade) VaporizeCorpse(self,energyVaporized);
    else if (isObj && !isNPC) ObjectDeath(self);
    else if (e->index == 279/*screen*/) ScreenDeath(self);
    else if (doTeleport) TeleportAway(self);
    else if (isGrenade) GrenadeExplode(self);
    
         if (isNPC && !doTeleport) NPCDeath(self);
    else if (self == PLAYER1) { if (!RessurectPlayer()) World.deaths++; }
    
    flag_set(&e->entflags,EF_DEAD_CHECKS_DONE,true);
}

float TakeDamage(u16 self,DamageData dd) {
    if (Cheats.god && self == PLAYER1) return 0.0f;
    if (self < World.instCount && World.instances[self].index == CYBER_DECOY_CONST) return 0.0f;
    
    u16 selfIdx = World.instances[self].index;
    bool isCyber = IsCyberEntity(self), isNPC = IdxIsNPC(selfIdx), isPlayer = (self == PLAYER1), isGrenade = IsGrenade(selfIdx);
    float* hp = isCyber ? &World.instances[self].cyberHealth : &World.instances[self].health;
    if (isCyber) { if (dd.attackType == Att_Drill && isNPC){return 0.0f;} if (dd.attackType != Att_Drill && World.instances[self].iceActive){return 0.0f;} }
    if (*hp <= 0.0f) { bool allowPost = (isNPC || World.instances[self].iceActive || isPlayer || isGrenade || selfIdx == 279/*chunk_screen*/ || selfIdx == 477/*sec_camera*/); if (!allowPost) return 0.0f; }
    
    float take = dd.damage;
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
    else { if(selfIdx == 477/*Camera constIndex 477 gets one-shot by tranq*/ && dd.attackType == Att_Trnq){take=World.instances[self].health + 1.0f;} take=ApplyAttTypeAdjustments(self,take,dd.attackType); if(!isPlayer)DualLog("TakeDamage inst %u ci %u hp %.3f->%.3f take %.3f att %d owner %u\n",(u32)self,(u32)selfIdx,World.instances[self].health,World.instances[self].health-take,take,(int)dd.attackType,(u32)dd.owner); World.instances[self].health-=take; if (isPlayer) { World.damageReceived+=take; World.invP1.justFired = World.pauseRelativeTime; } if (dd.owner == PLAYER1){World.damageDealt+=take;} }
    if (isNPC && (World.instances[self].health > 0.0f || (isCyber && World.instances[self].cyberHealth > 0.0f))) { if (npcTable[selfIdx - 419].timeBetweenPain > 0.0f) flag_set(&World.instances[self].entflags,EF_GO_INTO_PAIN,true); World.instances[self].recentMostActivator = dd.owner; TargetIDSendDamageReceive(self,take,dd.attackType); AICheckPain(self); }
    if (!isCyber && (World.instances[self].entflags & EF_DEAD_CHECKS_DONE) && (World.layer[self] & L_Corpse)
        && (dd.attackType == Att_Beam || dd.attackType == Att_PjBm)) { VaporizeCorpse(self,true); return take; }
    if (isCyber) { if (World.instances[self].cyberHealth <= 0.0f) { if (!World.instances[self].iceActive && isNPC && !(World.instances[self].entflags & EF_DEAD_CHECKS_DONE)) {World.cyberkills++;} Death(self,false); } } else { if (World.instances[self].health <= 0.0f) { if (isNPC && !(World.instances[self].entflags & EF_DEAD_CHECKS_DONE)) {World.kills++;} Death(self,dd.attackType == Att_Beam || dd.attackType == Att_PjBm); } }    return take;
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
void PatchDisableAll(void){
    World.invP1.berserkFinished = World.invP1.berserkIncTime = World.invP1.detoxFinished = World.invP1.geniusFinished = World.invP1.mediFinished = World.invP1.reflexFinishedTime = World.invP1.sightFinishedTime = World.invP1.sightSideEffectFinishedTime = World.invP1.staminupFinishedTime = -1.0;
    World.invP1.mediPatchPulseFinished = 0.0;
    World.invP1.mediPatchPulseCount = 0;
    World.invP1.staminupActive = World.geniusActive = false;
    World.invP1.fatigue = 0.0f;
    World.invP1.berserkIncrement = World.invP1.patchActive = 0;
    World.timeScale = 1.0;
}

void PatchUpdate() {
    if (World.invP1.patchActive & PATCH_DETOX) { if (World.invP1.detoxFinished < World.pauseRelativeTime) World.invP1.patchActive -= PATCH_DETOX; else World.instances[PLAYER1].radiation = 0.0f; } // Detox: other patches wiped on use only; radiation zeroed every frame while active
    if (World.invP1.patchActive & PATCH_MEDI) { // Medi
        if (World.invP1.mediPatchPulseFinished == 0.0) World.invP1.mediPatchPulseCount = 0;
        if (World.invP1.mediPatchPulseFinished < World.pauseRelativeTime) {
            World.instances[PLAYER1].health = vmin(255.0f, World.instances[PLAYER1].health + 8.0f);
            World.invP1.mediPatchPulseFinished = World.pauseRelativeTime + (0.5 + World.invP1.mediPatchPulseCount * 0.5);
            World.invP1.mediPatchPulseCount++;
        }
        if (World.invP1.mediFinished < World.pauseRelativeTime && World.invP1.mediFinished != -1.0) {
            World.invP1.patchActive -= PATCH_MEDI;
            World.invP1.mediFinished = -1.0;
            World.invP1.mediPatchPulseFinished = 0.0;
            World.invP1.mediPatchPulseCount = 0;
        }
    } else {
        World.invP1.mediPatchPulseFinished = 0.0;
        World.invP1.mediPatchPulseCount = 0;
    }
    if (World.invP1.patchActive & PATCH_REFLEX) {
        if (World.invP1.reflexFinishedTime < World.pauseRelativeTime && World.invP1.reflexFinishedTime != -1.0) {
            World.invP1.patchActive -= PATCH_REFLEX;
            World.invP1.reflexFinishedTime = -1.0;
            World.timeScale = 1.0;
        } else { World.timeScale = 0.25; }
    } else { if(World.timeScale != 1.0){World.timeScale = 1.0;}}//Reflex (tracked in game-time: pauses with the game and persists through save/load)
    if (World.invP1.patchActive & PATCH_BERSERK) { // Berserk
        if (World.invP1.berserkFinished < World.pauseRelativeTime) { World.invP1.berserkIncrement = 0; World.invP1.patchActive -= PATCH_BERSERK; }
        else if (World.invP1.berserkIncTime < World.pauseRelativeTime) { World.invP1.berserkIncrement++; if (World.invP1.berserkIncrement > 6) World.invP1.berserkIncrement = 6; World.invP1.berserkIncTime = World.pauseRelativeTime + 4.0; }
    }
    if (World.invP1.patchActive & PATCH_GENIUS) { if(World.invP1.geniusFinished < World.pauseRelativeTime){World.invP1.patchActive -= PATCH_GENIUS; World.geniusActive=false;}else{World.geniusActive=true;} } // Genius
    if (World.invP1.patchActive & PATCH_SIGHT) { // Sight
        if (World.invP1.sightFinishedTime < World.pauseRelativeTime && World.invP1.sightFinishedTime != -1.0) { World.invP1.sightFinishedTime=-1.0; World.invP1.sightSideEffectFinishedTime = World.pauseRelativeTime + 17.0; }
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
bool RessurectPlayer(void) {
    if (!((World.ressurectionActiveLevels >> World.curLev) & 1u)) { return false; }
    V3 spot=(V3){0.0f,0.0f,0.0f};
    
    if (World.curLev == 10 || World.curLev == 11 || World.curLev == 12) { spot=ressurectionLocations[6]; spot.y -= (1.0f - PLYR_CAM_OFS_Y); LoadLevel(6, spot); }
    else if (World.curLev < 13) { spot=ressurectionLocations[World.curLev]; spot.y -= (1.0f - PLYR_CAM_OFS_Y); World.position[PLAYER1] = spot; }
    
    PlayTrack(TT_Revive, MT_Override);
    World.invP1.ressurectingFinished = World.pauseRelativeTime + 3.0;
    CenterStatusPrint("BRAIN ACTIVITY SATISFACTORY..."); // TODO actual timed text appearances and then restore movement and unpause game.
    return true;
}

// Doors
static bool DoorInventoryHasAccessCard(AccCardType card) { return card == ACC_None || (World.invP1.accessCardOwned & (1u << card)); }
static void DoorOpen(u16 self) { Entity* e = &World.instances[self]; ChangeAnim(e,A_OPENING); e->doorOpen = e->doorState = DoorState_Opening; e->waitBefClose = World.pauseRelativeTime + e->delay; if (e->SFXIndex > 0 && e->SFXIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXIndex], AppliedFXVol(1.0f), World.position[self], true); }
static void DoorClose(u16 self) { Entity* e = &World.instances[self]; ChangeAnim(e,A_CLOSING); e->doorOpen = e->doorState = DoorState_Closing; if (e->SFXIndex > 0 && e->SFXIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXIndex], AppliedFXVol(1.0f), World.position[self], true); }
void DoorForceOpen(u16 self) { World.instances[self].requiredAccessCard = ACC_None; EntitySetLocked(&World.instances[self],false); DoorOpen(self); }
void DoorForceClose(u16 self) { if (World.instances[self].doorOpen == DoorState_Closed) {return;} DoorClose(self); }
void DoorActuate(u16 self) {
    Entity* e = &World.instances[self];
    /* Settle a finished stroke before reading it. Frob() runs ahead of DoorUpdate() inside ModUpdate(), so a
       button press landing on the frame the door reaches frameEnd still sees doorOpen == Opening with
       frame == frameEnd, which the inversion below turns into a full 0% jump. Reconciling first sends that
       case down the clean DoorClose/DoorOpen path, where frameStart genuinely is the right resume point. */
    { AnimationClip o=DoorGetClip(e,A_OPENING), c=DoorGetClip(e,A_CLOSING);
      if (e->doorOpen == DoorState_Opening && e->clip == A_OPENING && e->frame >= o.frameEnd) { e->doorOpen = e->doorState = DoorState_Open; ChangeAnim(e,A_IDLE_OPEN); }
      else if (e->doorOpen == DoorState_Closing && e->clip == A_CLOSING && e->frame >= c.frameEnd) { e->doorOpen = e->doorState = DoorState_Closed; ChangeAnim(e,A_IDLE_CLOSED); } }
    if (e->doorOpen == DoorState_Open) { DoorClose(self); return; } if (e->doorOpen == DoorState_Closed) { DoorOpen(self); return; } bool op = e->doorOpen == DoorState_Opening;
    if (op || e->doorOpen == DoorState_Closing) {
        u8 src = op ? A_OPENING : A_CLOSING, dst = op ? A_CLOSING : A_OPENING;
        AnimationClip srcClip = DoorGetClip(e,src), dstClip = DoorGetClip(e,dst);
        u32 f = e->frame; if (f < srcClip.frameStart) f = srcClip.frameStart; if (f > srcClip.frameEnd) f = srcClip.frameEnd;
        float p = (srcClip.frameEnd > srcClip.frameStart) ? (float)(f - srcClip.frameStart) / (float)(srcClip.frameEnd - srcClip.frameStart) : 1.0f;
        u16 newFrm = DoorFrameFromProgress(dstClip,1.0f - p); // Direct frame assignment (mid-anim reversal): clip + frame + matching model.
        e->clip = dst; e->frame = newFrm; e->currentFrameFinished = 0.0; e->modelIndex = dstClip.frameStartModelIndex + (u16)(newFrm - dstClip.frameStart); e->doorOpen = e->doorState = op ? DoorState_Closing : DoorState_Opening;
        if (!op) e->waitBefClose = World.pauseRelativeTime + e->delay; if (e->SFXIndex >= 0 && e->SFXIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXIndex], AppliedFXVol(1.0f), World.position[self], true);
    }
}

void DoorUse(u16 self, u16 activator) {
    if (activator == WORLD) return; Entity* e = &World.instances[self]; if (GetCurrentLevelSecurity() > UsableOrDef((float)e->securityThreshold,100.0f)) { UIBlockedBySecurity(World.position[self]); return; } if (Cheats.superoverride || World.diffMis <= 0) { EntitySetLocked(e,false); e->requiredAccessCard = ACC_None; }
    if (World.diffMis <= 1) { e->requiredAccessCard = ACC_None; } if (e->useFinished >= World.pauseRelativeTime) return; e->useFinished = World.pauseRelativeTime + 0.15f;
    if (e->requiredAccessCard != ACC_None) { if (!DoorInventoryHasAccessCard(e->requiredAccessCard)) {CenterStatusPrint("%s%s",AccessCardCodeForType(e->requiredAccessCard),Sys_Text.stringTable[2]); play_wav(sounds[467], AppliedFXVol(0.7f), World.position[self], true); return;} else {e->requiredAccessCard = ACC_None;}}
    if ((e->entflags & EF_LOCKED) != 0) { CenterStatusPrint("%s",Sys_Text.stringTable[e->lockedMessageLingdex]); play_wav(sounds[467], AppliedFXVol(0.55f), World.position[self], true); return; }  if ((e->onlyTargetOnce && !e->targetAlreadyDone) || !e->onlyTargetOnce) { e->targetAlreadyDone = true; UseTargets(self,e->targetIdx); } if (e->ajar) e->ajar = false; DoorActuate(self);
}

void DoorTargetted(u16 self, u16 activator) { if ((World.instances[self].entflags & EF_LOCKED) != 0) EntitySetLocked(&World.instances[self],false); if (!World.instances[self].targettingOnlyUnlocks) DoorUse(self,activator); }
static void DoorLayerUpdate(u16 self, Entity* e) {
    if (!IdxIsLayerDoor(e->index)) return;
    u32 want = (e->doorState == DoorState_Closed) ? L_Door : L_InterDebris;
    if (World.layer[self] != want) World.layer[self] = want;
}

static bool DoorLasersOn(const Entity* e, AnimationClip closing) {
    if (e->ajar) return false;
    if (e->doorState == DoorState_Closed) return true;
    if (e->doorState != DoorState_Closing || e->clip != A_CLOSING) return false;
    float p = (closing.frameEnd > closing.frameStart) ? (float)(e->frame - closing.frameStart) / (float)(closing.frameEnd - closing.frameStart) : 1.0f;
    return (p * 100.0f >= e->timeBeforeLasersOn);
}

void DoorLaserDraw(u16 self) {
    Entity* e = &World.instances[self];
    if (!DoorLasersOn(e, DoorGetClip(e, A_CLOSING))) return;
    if (V3_SqDist(World.position[self], World.position[PLAYER1]) > 2500.0f) return; /* ~50m */
        
    AnimationClip closing = DoorGetClip(e, A_CLOSING);
    u16 m = (u16)(closing.frameStartModelIndex + (u16)(closing.frameEnd - closing.frameStart));
    if (m >= MAX_MDLS) return;
    
    V3 mn = modelMin[m], mx = modelMax[m];
    V3 ctr = (V3){(mn.x+mx.x)*0.5f * World.scale[self].x, (mn.y+mx.y)*0.5f * World.scale[self].y, (mn.z+mx.z)*0.5f * World.scale[self].z};
    V3 he  = (V3){(mx.x-mn.x)*0.5f * World.scale[self].x, (mx.y-mn.y)*0.5f * World.scale[self].y, (mx.z-mn.z)*0.5f * World.scale[self].z};
    V3 c = V3_AplusB(World.position[self], quat_rot_v3(World.rotation[self], ctr));
    bool wideX = he.x >= he.z;
    V3 right = quat_rot_v3(World.rotation[self], wideX ? (V3){1.0f,0.0f,0.0f} : (V3){0.0f,0.0f,1.0f});
    V3 up = quat_rot_v3(World.rotation[self], (V3){0.0f,1.0f,0.0f});
    float hw = (wideX ? he.x : he.z) * 0.94f, hh = he.y; /* stop just inside the frame so the bars read as inset */
    if (hw <= 0.0f || hh <= 0.0f) return;
    
    V3 lo, hi, lf, rf;
    if (e->index == 506) {
        float halfX = vmin(hw, hh);
        if (halfX <= 0.0f) return;
        
        lo = V3_AplusB(c, V3_AplusB(V3_ScaleByF(right,-halfX), V3_ScaleByF(up,-halfX)));
        hi = V3_AplusB(c, V3_AplusB(V3_ScaleByF(right, halfX), V3_ScaleByF( up, halfX)));
        lf = V3_AplusB(c, V3_AplusB(V3_ScaleByF(right,-halfX), V3_ScaleByF( up, halfX)));
        rf = V3_AplusB(c, V3_AplusB(V3_ScaleByF(right, halfX), V3_ScaleByF(up,-halfX)));
    } else {
        lo = V3_AplusB(c, V3_AplusB(V3_ScaleByF(right,-hw), V3_ScaleByF(up,-hh)));
        hi = V3_AplusB(c, V3_AplusB(V3_ScaleByF(right, hw), V3_ScaleByF( up,hh)));
        lf = V3_AplusB(lo, V3_ScaleByF(up, 2.56f));
        rf = V3_AsubB(hi, V3_ScaleByF(up, 2.56f));
    }
    V3 face = V3_Normalize(V3_Cross(right,up)); /* door plane normal, used to keep the triplet spread in-plane */
    const Color band[3] = {{1.0f,0.82f,0.08f,1.0f},{1.0f,0.42f,0.04f,1.0f},{1.0f,0.82f,0.08f,1.0f}};
    const float spread[3] = {-0.0225f, 0.0f, 0.0225f};
    for (int pass = 0; pass < 2; ++pass) {
        V3 a = pass ? lf : lo, b = pass ? rf : hi;
        V3 dir = V3_Normalize(V3_AsubB(b,a));
        V3 perp = V3_Normalize(V3_Cross(dir, face));
        for (int k = 0; k < 3; ++k) {
            V3 o = V3_ScaleByF(perp, spread[k]);
            DrawCrossedLine(V3_AplusB(a,o), V3_AplusB(b,o), perp, 0.015f, band[k]);
        }
    }
}

void DoorUpdate(u16 self) {
    Entity* e = &World.instances[self]; if(e->ajar){return;} AnimationClip opening=DoorGetClip(e,A_OPENING), closing=DoorGetClip(e,A_CLOSING);
    if (e->doorOpen == DoorState_Opening && e->clip == A_OPENING && e->frame >= opening.frameEnd) { e->doorOpen = e->doorState = DoorState_Open; ChangeAnim(e,A_IDLE_OPEN); } else if (e->doorOpen == DoorState_Closing && e->clip == A_CLOSING && e->frame >= closing.frameEnd) { e->doorOpen = e->doorState = DoorState_Closed; ChangeAnim(e,A_IDLE_CLOSED); } if (World.pauseRelativeTime > e->waitBefClose && e->doorOpen == DoorState_Open && !e->stayOpen && !e->startOpen) DoorClose(self);
    DoorLayerUpdate(self, e);
}

u16 SpawnDynamicObject(int val, bool cheat) {
    if (!IdxInBounds(val)) { DualLogWarn("Const index out of bounds: %u", val); return WORLD; } if (IdxIsGeometry(val) && !Cheats.editMode) { CenterStatusPrint("Indices 0 to 306 (level chunks)\nnot possible when not on edit mode!"); return WORLD; } (void)cheat;
    if (World.instCount >= INSTANCE_COUNT) { DualLogWarn("Failed to spawn constIndex %u: instance table full (%u/%u)",val,World.instCount,INSTANCE_COUNT); return WORLD; } u16 entityIndexInInstanceTable = AddInstance((u16)val, (V3){0.0f,0.0f,0.0f}); return entityIndexInInstanceTable;
}

// TargetIO: Full game cross-level target handling.  Iterates all loaded levels, temporarily swaps active pointers via SetLevelPointers(), finds matching targetname(s), and calls Targetted().  Activator from cur level. Recursion is safe via targetIOActive flag.
void TriggerTargetted(u16 self, u16 activator) { UseTargets(activator, World.instances[self].targetIdx); }
bool QuestBitIsSet(u8 qb) { return (qb < QB_COUNT) && ((World.missionBits >> qb) & 1u); }
void QuestBitSet(u8 qb)    { if (qb < QB_COUNT && !QuestBitIsSet(qb)) { World.missionBits |=  (1u << qb); QuestBitNoteSideEffects(qb, true); } }
void QuestBitClear(u8 qb)  { if (qb < QB_COUNT &&  QuestBitIsSet(qb)) { World.missionBits &= ~(1u << qb); QuestBitNoteSideEffects(qb, false); } }
void QuestBitToggle(u8 qb) { if (qb < QB_COUNT) { World.missionBits ^=  (1u << qb); QuestBitNoteSideEffects(qb, QuestBitIsSet(qb)); } }
void Targetted(u16 activator, u16 self) {
    Entity* e = &World.instances[self]; u32 aioflags = World.targetIOActive ? World.targetIOActivatorIoflags : World.instances[activator].ioflags; u32 aioflagsHi = World.targetIOActive ? World.targetIOActivatorIoflagsHi : World.instances[activator].ioflagsHi;
    if (e->index == 699) { if (!e->relayEnabled) return; if (e->relayOnceEver) { if (e->relayAlreadyDone) return; e->relayAlreadyDone = true; } if (e->delay > 0.0f) { LogicRelayDelayTarget(self, activator); return; } u32 savedFlags = World.targetIOActivatorIoflags, savedFlagsHi = World.targetIOActivatorIoflagsHi; World.targetIOActivatorIoflags = e->ioflags; World.targetIOActivatorIoflagsHi = e->ioflagsHi; UseTargets(activator,e->targetIdx); World.targetIOActivatorIoflags = savedFlags; World.targetIOActivatorIoflagsHi = savedFlagsHi; return; }
    if (e->index == 702) { if (aioflagsHi & TARG_IOFLAGHI_SPAWNER_ACTIVATE_ALERTED) { SpawnManagerActivate(self, true); } else if (aioflagsHi & TARG_IOFLAGHI_SPAWNER_ACTIVATE) { SpawnManagerActivate(self, false); } return; }
    if (e->index == 700) {
        if (!(aioflags & TARG_IOFLAGS_BRANCH_FLIPONLY)) { if (e->relayEnabled && e->currentTargetIdx != IO_NONE) { u32 savedFlags = World.targetIOActivatorIoflags, savedFlagsHi = World.targetIOActivatorIoflagsHi; World.targetIOActivatorIoflags = e->ioflags; World.targetIOActivatorIoflagsHi = e->ioflagsHi; UseTargets(activator,e->currentTargetIdx); World.targetIOActivatorIoflags = savedFlags; World.targetIOActivatorIoflagsHi = savedFlagsHi; e->branchOnSecond = !e->branchOnSecond; e->currentTargetIdx = e->branchOnSecond ? e->target2Idx : e->targetIdx; } }
        if (aioflags & (TARG_IOFLAGS_BRANCH_FLIP | TARG_IOFLAGS_BRANCH_FLIPONLY)) { e->branchOnSecond = !e->branchOnSecond; e->currentTargetIdx = e->branchOnSecond ? e->target2Idx : e->targetIdx; }   return;
    }
    if (e->index == 710 && e->questBitID != QB_None) {
        if (aioflags & TARG_IOFLAGS_MISSION_BIT_ON) QuestBitSet(e->questBitID);
        if (aioflags & TARG_IOFLAGS_MISSION_BIT_OFF) QuestBitClear(e->questBitID);
        if (aioflags & TARG_IOFLAGS_MISSION_BIT_TOGGLE) QuestBitToggle(e->questBitID);
    }
    if (e->questBitID != QB_None) {
        u8 tm = e->questTestMode; if (!tm && activator != WORLD) tm = World.targetIOActivatorEntity.questTestMode;/*1==testQuestBitIsOn, 2==testQuestBitIsOff; snapshot, not World.instances[activator]: UseTargets iterates levels via SetLevelPointers, so the live array is the target's level, not the activator's*/
        if (tm) { bool bitOn = QuestBitIsSet(e->questBitID); u32 savedFlags = World.targetIOActivatorIoflags, savedFlagsHi = World.targetIOActivatorIoflagsHi; World.targetIOActivatorIoflags = e->ioflags; World.targetIOActivatorIoflagsHi = e->ioflagsHi; UseTargets(activator, (tm == 1) == bitOn ? e->targetIdx : e->targetIfFalseIdx); World.targetIOActivatorIoflags = savedFlags; World.targetIOActivatorIoflagsHi = savedFlagsHi; }
    }
    if (e->index == 709) { CenterStatusPrint("%s", Sys_Text.stringTable[e->messageLingdex]); return; }/*info_message*/   if (e->index == 708) { GameEndSequence(); return; }
    if (e->index == 712) { /*info_playsound: play sounds[SFXClip] (vox/shodan announcements)*/ if (e->SFXIndex >= 0 && e->SFXIndex < SOUNDS_COUNT && sounds[e->SFXIndex][0]) { if (e->playEverywhere) play_wav(sounds[e->SFXIndex], AppliedFXVol(1.0f), (V3){0}, false); else play_wav(sounds[e->SFXIndex], AppliedFXVol(1.0f), World.position[self], true); } return; }
    if (e->index == 707) { EmailTargetted(self); return; }/*info_email*/                                                 if (aioflags & TARG_IOFLAGS_TRIPTRIGGER) { if(e->index == 598 || e->index == 600){TriggerTargetted(self,activator);}else if(e->index == 594){TriggerCounterTargetted(self,activator);} }
    if (aioflags & TARG_IOFLAGS_UNLOCK) { EntitySetLocked(e, false); if (IdxIsDoor(e->index) && (aioflags & (TARG_IOFLAGS_DOOROPEN | TARG_IOFLAGS_DOOROPENIFUNLOCKED))) e->requiredAccessCard = ACC_None; }                                                       if ((aioflags & TARG_IOFLAGS_LOCK) && IdxIsDoor(e->index)) EntitySetLocked(e, true);                                     if (IdxIsButtonSwitch(e->index)) ButtonSwitchUse(self,activator);
    if ((aioflags & TARG_IOFLAGS_DOOROPEN) && IdxIsDoor(e->index) && !(aioflags & TARG_IOFLAGS_UNLOCK)) { DoorForceOpen(self); } else if ((aioflags & TARG_IOFLAGS_DOOROPENIFUNLOCKED) && IdxIsDoor(e->index) && !(aioflags & TARG_IOFLAGS_UNLOCK) && (e->entflags & EF_LOCKED) == 0 && (e->requiredAccessCard == ACC_None || (World.invP1.accessCardOwned & (1u << e->requiredAccessCard)))) { DoorForceOpen(self); } else if ((aioflags & TARG_IOFLAGS_DOORCLOSE) && IdxIsDoor(e->index)) { DoorForceClose(self); } else if (IdxIsDoor(e->index)) { DoorTargetted(self, activator); }
    if (aioflags & TARG_IOFLAGS_FBRIDGE_ACTIVATE) ForceBridgeActivate(self, false); else if (aioflags & TARG_IOFLAGS_FBRIDGE_DEACTIVATE) ForceBridgeDeactivate(self, false); else if (aioflags & TARG_IOFLAGS_FBRIDGE_TOGGLE) ForceBridgeToggle(self);
    if (aioflags & TARG_IOFLAGS_GRAVLIFT_TOGGLE) { World.instances[self].active=!World.instances[self].active; if (e->index == 596) GravityLiftSyncVisuals(self); } if (aioflags & TARG_IOFLAGS_TEXTURE_CHG_TOGGLE) TextureChangerToggle(self);
    if (aioflags & TARG_IOFLAGS_FUNCWALL_MOVE) FuncWallTargetted(self);                                                  if (aioflags & TARG_IOFLAGS_SWITCH_LOCK_TOGGLE) EntitySetLocked(e, (e->entflags & EF_LOCKED) == 0);
    if (aioflags & TARG_IOFLAGS_INST_ACTIVATE) flag_set(&e->entflags, EF_ACTIVE, true); else if (aioflags & TARG_IOFLAGS_INST_DEACTIVATE) { if (e->camView != 255) { e->camView = 255; TextureSequenceInit(self, "Static"); flag_set(&e->entflags, EF_ACTIVE, true); }/*camera destroyed: keep its screen, switch it to Static*/ else { flag_set(&e->entflags, EF_ACTIVE, false); } } else if (aioflags & TARG_IOFLAGS_INST_TOGGLE) flag_set(&e->entflags, EF_ACTIVE, !(e->entflags & EF_ACTIVE));
    if ((aioflagsHi & TARG_IOFLAGHI_AWAKE_SLEEPING) && IdxIsNPC(e->index)) AIAwakeFromSleep(self);
    if ((aioflags & TARG_IOFLAGS_ENEMY_ALERT) && IdxIsNPC(e->index)) AIAlert(self);
}

extern char ioNames[MAX_IO_NAMES][TARG_STRLEN];
INLINE V3 ScreenPointToRayOffset(V3 f,V3 r,float dx,float dy); extern u16 ioNameCount;
void TargetLight(u16 activator, u16 lightIdx) {
    if (!World.lightTargetnames || lightIdx >= World.loadedLights) return;
    u32 aioflags = World.targetIOActive ? World.targetIOActivatorIoflags : World.instances[activator].ioflags;
    u32 a = aioflags & (TARG_IOFLAGS_LIGHT_ON|TARG_IOFLAGS_LIGHT_OFF|TARG_IOFLAGS_LIGHT_TOGGLE);
    if (!a) return;
    bool on = (World.lights[lightIdx].lflags & LIGHTON) != 0;
    if      (a & TARG_IOFLAGS_LIGHT_ON)  on = true;
    else if (a & TARG_IOFLAGS_LIGHT_OFF) on = false;
    else                                 on = !on;
    flag_set(&World.lights[lightIdx].lflags, LIGHTON, on);
    World.lights[lightIdx].intensity = on ? World.lights[lightIdx].maxIntensity : World.lights[lightIdx].minIntensity;
}

void UseTargets(u16 activator, u16 targetIdx) {
    if(targetIdx==IO_NONE){return;} bool wasActive=World.targetIOActive,succeeded=false; u8 entryLevel=World.currentLevel; if(!wasActive){World.targetIOActive=true; World.targetIOActivatorEntity=World.instances[activator]; World.targetIOActivatorIoflagsHi=World.targetIOActivatorEntity.ioflagsHi; World.targetIOActivatorIoflags=World.instances[activator].ioflags;} const char* targetname=(targetIdx<ioNameCount) ? ioNames[targetIdx] : "";
    for (u8 lev = 0; lev < World.numLevels; ++lev) { if (World.currentLevel != lev) SetLevelPointers(lev); for (u16 i = INSTS_1ST_IDX; i < World.instCount; ++i) { if (World.instances[i].targetnameIdx != targetIdx) {continue;} Targetted(activator,i); succeeded=true; } if (World.lightTargetnames) { for (u16 l = 0; l < World.loadedLights; ++l) { if (World.lightTargetnames[l] != targetIdx) {continue;} TargetLight(activator,l); succeeded=true; } } }
    if (World.currentLevel != entryLevel) {SetLevelPointers(entryLevel);} if (!succeeded) {DualLogWarn("No target found: %s\n",targetname);} if (!wasActive) {World.targetIOActive=false;}
}

// Frob/Use
#define FROB_DISTANCE 4.9f
void MFD_OpenSearch(bool isRH),MFD_CloseSearch(void),MFD_OpenData(bool isRH,u8 code),MFD_OpenPaperLog(int,V3);
static void PaperLogUse(u16 self) { MFD_OpenPaperLog((int)World.instances[self].logIndex, World.position[self]); }
static bool IsPuzzleGridPanel(u16 index) { return index>=609&&index<=613; }
static bool IsPuzzleWirePanel(u16 index) { return index>=741&&index<=745; }
static bool IsElevatorPanel(u16 index) { return index>=604&&index<=607; }
static bool IsFrobUsableSpecial(u16 index) { return index==574||index==546||index==608||index==614||index==602||index==603/*us_paperlog*/||index==556/*prop_cyberport*/||IsElevatorPanel(index)||IsPuzzleGridPanel(index)||IsPuzzleWirePanel(index); }/*546 prop_charge_station; 614 us_relaypanel, 602 us_isotopepanel: InteractablePanel; 603 us_paperlog: PaperLog*/
typedef struct { u16 constIndex; i8 tgtL[7],tgtR[7]; bool wireOn[7]; u8 wireColor[7]; u8 rowsActive[7]; } WirePuzzleDef;
static const WirePuzzleDef wirePuzzleDefs[] = {
    /*741 blue  */{741,{1,2,5,-1,-1,-1,-1},{1,3,4,-1,-1,-1,-1},{true,true,true,false,false,false,false},{0,5,4,0,0,0,0},{1,1,1,1,1,1,0}},
    /*742 brown */{742,{1,3,2,-1,-1,-1,-1},{2,4,3,-1,-1,-1,-1},{true,true,true,false,false,false,false},{1,5,6,2,2,2,2},{1,1,1,1,1,1,0}},
    /*743 gray  */{743,{1,2,5,-1,-1,-1,-1},{1,3,4,-1,-1,-1,-1},{true,true,true,false,false,false,false},{0,5,4,0,0,0,0},{1,1,1,1,1,1,0}},
    /*744 red   */{744,{2,1,-1,-1,-1,-1,-1},{2,1,-1,-1,-1,-1,-1},{true,true,false,false,false,false,false},{0,5,0,0,0,0,0},{1,1,1,1,0,0,0}},
    /*745 teal  */{745,{0,1,2,3,-1,-1,-1},{2,4,5,0,-1,-1,-1},{true,true,true,true,false,false,false},{4,4,4,4,4,4,4},{1,1,1,1,1,1,0}},
};

static const WirePuzzleDef* WirePuzzleDefFor(u16 constIndex) { for (u32 i=0;i<sizeof(wirePuzzleDefs)/sizeof(wirePuzzleDefs[0]);++i) if (wirePuzzleDefs[i].constIndex==constIndex) return &wirePuzzleDefs[i]; return NULL; }
static void WireLoadInstance(u16 self, const WirePuzzleDef* def) {
    Entity* e=&World.instances[self];
    for (u8 c=0;c<7;++c) { World.Sys_UI.pw_curL[c]=-1; World.Sys_UI.pw_curR[c]=-1; }
    for (u8 w=0;w<7;++w) { int l=(int)e->wireCurL[w], r=(int)e->wireCurR[w]; if (w<7 && l>=0 && l<7) World.Sys_UI.pw_curL[l]=(i8)w; if (w<7 && r>=0 && r<7) World.Sys_UI.pw_curR[r]=(i8)w; }/*All seven slots, not just the first three: 745 teal runs four wires and level5's teal instance saves currentPositionsRight[3..5], so a w<3 guard dropped its fourth wire and left that panel unsolvable.*/
    for (u8 w=0;w<7;++w) { World.Sys_UI.pw_tgtL[w]=def->tgtL[w]; World.Sys_UI.pw_tgtR[w]=def->tgtR[w]; World.Sys_UI.pw_wireOn[w]=def->wireOn[w]; World.Sys_UI.pw_rowActive[w]=def->rowsActive[w]!=0; World.Sys_UI.pw_wireColor[w]=def->wireColor[w];/*raw HUDColor, as PuzzleWire.wireColors holds it*/ }
}

typedef struct { V3 pos; u8 gridType, theme, security; const char* target; u8 cellType[35]; } GridPuzzleDef;
static const GridPuzzleDef gridPuzzleDefs[] = {
    /*0 R.ReactorLevel       teal   levRmedbeddoor*/{{-4.4910f,-54.6630f,30.3431f},4,2,100,"levRmedbeddoor",{0,0,0,0,1,1,1,1,1,1,0,1,0,1,1,0,3,1,1,0,3,1,1,1,0,1,0,1,0,0,0,0,1,1,1}},
    /*1 1.MedicalLevel       blue   lev1wall1relay*/{{39.7100f,-44.4490f,32.5760f},5,0,100,"lev1wall1relay",{1,1,1,0,1,1,1,1,0,1,0,1,0,1,1,0,1,1,1,0,1,1,0,1,0,1,0,1,1,1,1,0,1,1,1}},
    /*1 1.MedicalLevel       teal   lev1xdoor1*/{{6.0450f,-42.7601f,1.1690f},0,1,90,"lev1xdoor1",{0,0,0,0,0,0,0,3,2,1,1,1,3,1,3,1,1,1,1,0,2,3,1,1,1,1,3,1,0,0,0,0,0,0,0}},
    /*2 2.ScienceLevel       red    levRrobotspawncontrol*/{{-24.3560f,-28.0130f,34.6520f},4,2,100,"levRrobotspawncontrol",{0,0,0,0,1,1,1,1,1,1,0,1,0,1,1,0,2,1,1,0,2,1,1,1,0,1,0,1,0,0,0,0,1,1,1}},
    /*2 2.ScienceLevel       red    lev2doorarmory*/{{39.6969f,-27.4656f,-7.7154f},0,1,100,"lev2doorarmory",{0,0,0,0,0,0,0,0,1,2,1,1,0,0,3,1,1,1,1,3,3,0,1,1,1,1,0,0,0,0,0,0,0,0,0}},
    /*4 4.StorageLevel       brown  lev4hiddenclosetforcedoor*/{{-10.1910f,-9.5700f,24.9638f},3,3,100,"lev4hiddenclosetforcedoor",{0,1,0,1,0,0,0,1,1,1,1,1,1,0,1,1,1,2,1,1,1,1,1,1,1,1,1,0,0,1,0,1,0,0,0}},
    /*5 5.FlightDeck         brown  lev5flightbay3iris*/{{-15.4832f,9.6899f,23.7354f},0,1,100,"lev5flightbay3iris",{0,0,0,0,0,0,0,3,2,1,1,1,3,1,3,1,1,1,1,0,2,3,1,1,1,1,3,1,0,0,0,0,0,0,0}},
    /*5 5.FlightDeck         brown  lev5flightbay3iris*/{{-14.0602f,12.2240f,22.9740f},0,1,100,"lev5flightbay3iris",{0,0,0,0,0,0,0,1,2,1,1,3,1,0,1,1,1,1,0,2,3,1,1,1,1,3,1,0,0,0,0,0,0,0,0}},
    /*5 5.FlightDeck         brown  lev5flightbay23bulkhead*/{{-14.0472f,13.9730f,2.4340f},0,1,100,"lev5flightbay23bulkhead",{0,0,0,0,0,0,0,1,2,1,1,1,3,1,3,1,1,1,1,0,2,1,1,1,1,1,3,1,0,0,0,0,0,0,0}},
    /*7 7.EngineeringLevel   brown  lev7antennafield1*/{{4.6211f,50.6000f,54.9166f},5,0,100,"lev7antennafield1",{0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,1,1,1,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    /*7 7.EngineeringLevel   gray   lev7antennafield1*/{{3.3831f,50.8220f,56.1656f},5,0,100,"lev7antennafield1",{1,1,1,0,3,1,2,0,0,1,0,3,0,1,1,1,1,0,1,3,3,1,0,0,0,1,1,1,1,1,1,1,1,1,1}},
};

static const GridPuzzleDef* GridPuzzleDefFor(V3 pos) {
    for (u32 i=0;i<sizeof(gridPuzzleDefs)/sizeof(gridPuzzleDefs[0]);++i) {
        const GridPuzzleDef* d=&gridPuzzleDefs[i];
        if (vabs(pos.x-d->pos.x)<0.05f && vabs(pos.y-d->pos.y)<0.05f && vabs(pos.z-d->pos.z)<0.05f) return d;
    }
    return NULL;
}

static void GridLoadInstance(u16 self) {
    Entity* e=&World.instances[self];
    const GridPuzzleDef* def=GridPuzzleDefFor(World.position[self]);
    World.Sys_UI.pg_width=7; World.Sys_UI.pg_height=5; World.Sys_UI.pg_source=14; World.Sys_UI.pg_output=20;
    if (def) { if (def->target) e->targetIdx=IOInternName(def->target); e->securityThreshold=def->security; }
    
    if (def) { World.Sys_UI.pg_gridType=(PuzzleGridType)def->gridType; World.Sys_UI.pg_theme=def->theme; mcpy(World.Sys_UI.pg_type,def->cellType,sizeof(def->cellType)); }
    else     { World.Sys_UI.pg_gridType=PuzzleGridType_King; World.Sys_UI.pg_theme=HUDColor_Gray; mset(World.Sys_UI.pg_type,PuzzleCellType_Standard,sizeof(World.Sys_UI.pg_type)); }
    
    mcpy(World.Sys_UI.pg_cell,e->gridCells,sizeof(e->gridCells));
    World.Sys_UI.pg_solved=e->puzzleSolved; World.Sys_UI.pg_fired=e->puzzleFired;
    PGEvalPuzzle();
}

static void PuzzlePanelUse(u16 i) {
    Entity* e=&World.instances[i];
    if(GetCurrentLevelSecurity()>UsableOrDef((float)e->securityThreshold,100.0f)){UIBlockedBySecurity(World.position[i]);return;}
    if(e->entflags&EF_LOCKED){CenterStatusPrint("%s",Sys_Text.stringTable[302]);return;}
    World.Sys_UI.objectInUsePos=World.position[i]; World.Sys_UI.usingObject=true; ForceInventoryMode();
    if (IsPuzzleGridPanel(e->index)) {
        if(!e->panelOpen){
            e->panelOpen=true;
            ChangeAnim(e,A_OPENING);
            play_wav(sounds[91],AppliedFXVol(1.0f),World.position[i],true);
        }
        bool initialize=World.Sys_UI.tetheredPGP!=i||World.Sys_UI.pg_width==0||World.Sys_UI.pg_height==0;
        World.Sys_UI.tetheredPGP=i;
        if(initialize){ GridLoadInstance(i); }
        MFD_OpenData(World.Sys_UI.lastDataSideRH,3);
    } else {
        const WirePuzzleDef* def=WirePuzzleDefFor(e->index);
        World.Sys_UI.tetheredPWP=i; World.Sys_UI.pw_selectedWire=-1; World.Sys_UI.pw_solved=false;
        if(def) WireLoadInstance(i,def);
        else { for(u8 c=0;c<7;++c){World.Sys_UI.pw_curL[c]=(i8)c;World.Sys_UI.pw_curR[c]=(i8)c;World.Sys_UI.pw_tgtL[c]=(i8)c;World.Sys_UI.pw_tgtR[c]=(i8)c;World.Sys_UI.pw_wireOn[c]=true;World.Sys_UI.pw_rowActive[c]=true;World.Sys_UI.pw_wireColor[c]=HUDC_YELLOW;} }

        MFD_OpenData(World.Sys_UI.lastDataSideRH,4);
    }
    CenterStatusPrint("%s",Sys_Text.stringTable[190]);
}

static bool PanelUseAllowed(u16 i) {
    Entity* e=&World.instances[i];
    if(GetCurrentLevelSecurity()>UsableOrDef((float)e->securityThreshold,100.0f)){UIBlockedBySecurity(World.position[i]);return false;}
    if(e->entflags&EF_LOCKED){
        u16 msg=(u16)e->lockedMessageLingdex;if(msg<T_LOGSTR_CNT&&msg!=0)CenterStatusPrint("%s",Sys_Text.stringTable[msg]);else CenterStatusPrint("%s",Sys_Text.stringTable[302]);
        if(e->lockedTargetIdx!=IO_NONE)UseTargets(i,e->lockedTargetIdx);
        return false;
    }
    return true;
}

typedef enum { PanelInstallFrame, PanelInstallEntity, PanelInstallAnim } PanelInstallKind;
typedef struct { const char* name; u8 item; u16 itemConst; PanelInstallKind kind; u16 msgOpen,msgInst,msgWrong,msgAlready; i16 sfxOpen,sfxInst,sfxAlready; const char* target; bool blowUp; u16 wreckTex; } RelayPanelScript;
static const RelayPanelScript relayPanelScripts[] = {
    /*name             item itemConst kind               open inst wrong already  sfxO sfxI sfxA target                     fuse wreck*/
    {"panelRelay428",   57,  364,      PanelInstallFrame,  262, 260, 259,  261,    225,  42,  226, "lev3fixtherelay",        false, 0     },/*level 3 maintenance "relay 428": interface demodulator, chipset baked into the frame*/
    {"panelAntenna1",   56,  363,      PanelInstallEntity, 152, 606, 604,  605,     91,  42,  166, "lev7antenna1",           true,  618   },/*level 7 engineering antennas: Z-44 plastique placed on the panel, 15s fuse*/
    {"panelAntenna2",   56,  363,      PanelInstallEntity, 152, 606, 604,  605,     91,  42,  166, "lev7antenna2",           true,  618   },
    {"panelAntenna3",   56,  363,      PanelInstallEntity, 152, 606, 604,  605,     91,  42,  166, "lev7antenna3",           true,  618   },
    {"panelAntenna4",   56,  363,      PanelInstallEntity, 152, 606, 604,  605,     91,  42,  166, "lev7antenna4",           true,  618   },
    {"panelIsolinear",  64,  371,      PanelInstallEntity, 151, 211, 210,  212,     91,  42,  166, "lev9isolinearactivated", false, 0     },/*level 9: isolinear chipset placed on the panel*/
    {"panelIsotope",    61,  0,        PanelInstallAnim,   285, 283, 282,  284,     91, 235,  235, "levRinstallisotope",     false, 0     },/*level 0 reactor "isotope panel": X-22 goes in via A_INSTALL then A_INSTALLED*/
};

extern char ioNames[MAX_IO_NAMES][TARG_STRLEN];
static const RelayPanelScript* RelayPanelScriptFor(const Entity* e) {
    if (!e->targetnameIdx || e->targetnameIdx>=MAX_IO_NAMES) return NULL;
    for (u32 i=0;i<sizeof(relayPanelScripts)/sizeof(relayPanelScripts[0]);++i) if (sEqual(ioNames[e->targetnameIdx],relayPanelScripts[i].name)) return &relayPanelScripts[i];
    return NULL;
}

static void RelayPanelMsg(u16 msg) { if (msg>0 && msg<T_LOGSTR_CNT) CenterStatusPrint("%s",Sys_Text.stringTable[msg]); }
static void RelayPanelSfx(i16 sfx, V3 pos) { if (sfx>0 && sfx<SOUNDS_COUNT) play_wav(sounds[sfx],AppliedFXVol(1.0f),pos,true); }
static void RelayPanelUse(u16 self) {
    Entity* e=&World.instances[self]; if(!PanelUseAllowed(self))return;
    const RelayPanelScript* sc = RelayPanelScriptFor(e);
    /*UseData.mainIndex equivalent: the useable item index of whatever is in hand (-1 == empty hand).*/
    i16 held = (World.invP1.holdingObject && World.invP1.heldObjectIndex>=INSTS_1ST_IDX) ? (i16)(World.invP1.heldObjectIndex-307) : -1;
    if (!e->panelOpen) { e->panelOpen=true; ChangeAnim(e,A_OPENING); RelayPanelSfx(sc?sc->sfxOpen:91,World.position[self]); RelayPanelMsg(sc?sc->msgOpen:0); return; }
    if (e->panelInstalled && held<0) { RelayPanelMsg(sc?sc->msgAlready:0); return; }/*already installed, empty hand*/
    if (sc && held==(i16)sc->item) {
        if (e->panelInstalled) { RelayPanelSfx(sc->sfxAlready,World.position[self]); return; }/*wrong hand is not the issue: right item, already in*/
        e->panelInstalled=true;
        if (sc->kind==PanelInstallFrame) ChangeAnim(e,A_INSTALLED);                                  /*item baked into the model frame*/
        else if (sc->kind==PanelInstallAnim) ChangeAnim(e,A_INSTALL);                                /*isotope: interim clip, then A_INSTALLED*/
        else if (sc->itemConst) { u16 it=AddInstance(sc->itemConst,e->panelItemPos); if (it) World.rotation[it]=e->panelItemRot; }/*real item entity on the panel*/
        RelayPanelSfx(sc->sfxInst?sc->sfxInst:42,World.position[self]); RelayPanelMsg(sc->msgInst);
        ResetHeldItem();/*the item goes into the panel, so it leaves the hand for good*/
        if (sc->target && *sc->target) UseTargets(self,IOInternName(sc->target));
        if (sc->blowUp) { e->panelArmed=true; e->panelTimer = World.pauseRelativeTime + (e->delay>0.0f?(double)e->delay:15.0); }/*prefab DelayedSpawn: ExplosionTimer arms 15s after the installationItem goes active*/
        return;
    }
    play_wav(sounds[43]/*button_deny, aaaahhh!! Try again*/,AppliedFXVol(1.0f),World.position[self],true); RelayPanelMsg(sc?sc->msgWrong:0);
}

static void PuzzlePanelUpdate(u16 self) {
    Entity* e=&World.instances[self];
    if (e->clip==A_IDLE_CLOSED && e->panelOpen) ChangeAnim(e,A_IDLE_OPEN);
    if (e->panelOpen && e->clip==A_OPENING) { AnimationClip c=DoorGetClip(e,A_OPENING); if (c.frameEnd<=c.frameStart || e->frame>=c.frameEnd) ChangeAnim(e,A_IDLE_OPEN); }
}

static void RelayPanelUpdate(u16 self) {
    Entity* e=&World.instances[self];
    if (e->clip==A_IDLE_CLOSED) { if (e->panelInstalled) ChangeAnim(e,A_INSTALLED); else if (e->panelOpen) ChangeAnim(e,A_IDLE_OPEN); }
    if (e->panelOpen && e->clip==A_OPENING) { AnimationClip c=DoorGetClip(e,A_OPENING); if (c.frameEnd<=c.frameStart || e->frame>=c.frameEnd) ChangeAnim(e,A_IDLE_OPEN); }
    if (e->panelOpen && e->clip==A_INSTALL) { AnimationClip c=DoorGetClip(e,A_INSTALL); if (c.frameEnd<=c.frameStart || e->frame>=c.frameEnd) ChangeAnim(e,A_INSTALLED); }/*isotope: insertion clip hands over to the installed frame*/
    if (e->panelArmed && World.pauseRelativeTime>=e->panelTimer) {
        e->panelArmed=false; V3 p=World.position[self];
        play_wav(sounds[64]/*explosion1*/,AppliedFXVol(1.0f),p,true); SpawnExplosionEffect(p,1); Shake(-1.0f); World.fogFac += 5.0f;
        const RelayPanelScript* sc=RelayPanelScriptFor(e); if (sc && sc->wreckTex) e->texIndex=sc->wreckTex;
    }
}

static bool HeldItemIsFrobUser(i16 item) { return item==54||item==56||item==57||item==61||item==64||item==92||item==93||item==94; }
bool FrobHeldItemIntoPanel(V3 p, V3 f, V3 r) {
    if (!World.invP1.holdingObject) return false;
    i16 item=(i16)(World.invP1.heldObjectIndex-307); if (!HeldItemIsFrobUser(item)) return false;
    RaycastHit h=Raycast(p,ScreenPointToRayOffset(f,r,0,0),FROB_DISTANCE,LMASK_PLAYER_FROB);
    if (!h.hit || h.hitInstanceIndex>=World.instCount) return false;
    u16 idx=h.hitInstanceIndex; if (World.instances[idx].index!=614 && World.instances[idx].index!=602) return false;
    RelayPanelUse(idx); return true;/*the panel decides: it either installs the item (consuming it) or refuses it*/
}

typedef struct { i8 label; bool darkened; } ElevBtnDef;
static const ElevBtnDef elevLayouts[12][8] = {
    {{1,0},{2,0},{3,1},{6,1},{7,1},{8,1},{-1,0},{-1,0}},/*0: 1 to 2*/
    {{0,0},{1,1},{2,0},{3,0},{6,1},{7,1},{8,1},{-1,0}},/*1: 2 to 3*/
    {{3,0},{4,0},{5,0},{-1,0},{-1,0},{-1,0},{-1,0},{-1,0}},/*2: 3 to 4*/
    {{1,1},{2,1},{3,0},{6,0},{7,1},{8,1},{-1,0},{-1,0}},/*3: 3 to 6*/
    {{5,0},{6,0},{-1,0},{-1,0},{-1,0},{-1,0},{-1,0},{-1,0}},/*4: 5 to 6*/
    {{6,0},{10,0},{-1,0},{-1,0},{-1,0},{-1,0},{-1,0},{-1,0}},/*5: 6 to G1*/
    {{6,0},{11,0},{-1,0},{-1,0},{-1,0},{-1,0},{-1,0},{-1,0}},/*6: 6 to G2*/
    {{-1,0},{-1,0},{-1,0},{-1,0},{-1,0},{-1,0},{-1,0},{-1,0}},/*7: 6 to G3 (all hidden)*/
    {{6,0},{12,0},{-1,0},{-1,0},{-1,0},{-1,0},{-1,0},{-1,0}},/*8: 6 to G4*/
    {{1,1},{2,1},{3,1},{6,0},{7,0},{8,1},{-1,0},{-1,0}},/*9: 6 to 7*/
    {{1,1},{2,1},{3,1},{6,1},{7,0},{8,0},{-1,0},{-1,0}},/*10: 7 to 8*/
    {{1,1},{2,1},{3,1},{6,1},{8,0},{9,0},{-1,0},{-1,0}},/*11: 8 to 9*/
};

static const struct { u8 level; float x,y,z; u8 layout; } elevPanelMap[] = {
    {0,9.97f,-55.08f,39.40f,1},{1,49.88f,-44.58f,-18.0f,0},
    {2,52.47f,-27.94f,-25.62f,1},{2,3.79f,-27.94f,33.30f,1},
    {3,15.27f,-14.91f,-11.50f,1},{3,1.17f,-15.09f,12.75f,2},{3,6.32f,-15.09f,-20.52f,3},
    {4,-1.33f,1.64f,3.88f,2},
    {5,-15.31f,14.26f,-39.68f,4},{5,8.94f,13.16f,-7.61f,2},
    {6,-0.62f,34.09f,-69.24f,8},{6,-58.26f,34.14f,-39.71f,5},{6,-5.81f,36.02f,43.52f,6},{6,85.10f,34.14f,-37.09f,7},{6,-0.63f,34.10f,-46.09f,4},{6,60.81f,32.22f,35.84f,9},{6,-13.43f,34.14f,-30.73f,3},
    {7,16.27f,51.38f,56.23f,10},{7,26.57f,48.82f,-10.34f,9},
    {8,11.33f,97.46f,-41.43f,11},{8,3.58f,59.06f,20.01f,10},
    {9,3.57f,107.16f,-38.31f,11},
    {10,42.46f,136.38f,-7.83f,5},{11,9.91f,168.94f,-23.22f,6},{12,19.09f,196.14f,18.14f,8},
};

/*Elevator floor label index -> destination level. Labels: 0=R,1-9,10=G1,11=G2,12=G4,13=C.*/
static u8 ElevLabelToLevel(i8 labelIdx) {
    if (labelIdx>=1 && labelIdx<=9) return (u8)labelIdx;
    if (labelIdx==10) return 10; if (labelIdx==11) return 11; if (labelIdx==12) return 12;
    if (labelIdx==0) return 0;/*R*/
    return 255;
}

/*True if the player is inside an elevator volume (entity 706), using the same box-overlap logic as the automap elevator cell fill.*/
bool PlayerInElevatorCell(void) {
    V3 pp=World.position[PLAYER1]; pp.y-=PLYR_CAM_OFS_Y;
    for (u32 i=INSTS_1ST_IDX;i<World.instCount;++i) {
        Entity* e=&World.instances[i]; if(e->index!=706||!(e->entflags&EF_ACTIVE)){continue;}
        V3 c=World.colliderCenter[i],s=World.colliderSize[i],p=World.position[i];
        V3 rc=quat_rot_v3(World.rotation[i],c);
        float x0=p.x+rc.x-s.x*0.5f,x1=p.x+rc.x+s.x*0.5f,z0=p.z+rc.z-s.z*0.5f,z1=p.z+rc.z+s.z*0.5f;
        float y0=p.y+rc.y-s.y*0.5f,y1=p.y+rc.y+s.y*0.5f;
        if (pp.x>=x0&&pp.x<=x1&&pp.y>=y0&&pp.y<=y1&&pp.z>=z0&&pp.z<=z1) return true;
    }
    return false;
}

static void ElevatorPanelUse(u16 i) {
    if(!PanelUseAllowed(i))return;
    World.Sys_UI.tetheredKeypadElevator=i; World.Sys_UI.linkedElevatorDoor=U16_MAX; World.Sys_UI.objectInUsePos=World.position[i]; World.Sys_UI.usingObject=true;
    { V3 kp=World.position[i]; float best=1000000000000000000000000000000.0f; for (u32 d=INSTS_1ST_IDX;d<World.instCount;++d) { Entity* de=&World.instances[d]; if(!IdxIsDoor(de->index)||!(de->entflags&EF_ACTIVE)) continue; V3 dd=V3_AsubB(World.position[d],kp); float dist2=V3_dot(dd,dd); if(dist2<best){best=dist2; World.Sys_UI.linkedElevatorDoor=(u16)d;} } }
    int layout=-1; V3 pp=World.position[i];
    for (u32 m=0;m<sizeof(elevPanelMap)/sizeof(elevPanelMap[0]);++m) {
        if (elevPanelMap[m].level==World.currentLevel) { float dx=elevPanelMap[m].x-pp.x,dz=elevPanelMap[m].z-pp.z; if (dx*dx+dz*dz<4.0f) { layout=elevPanelMap[m].layout; break; } }
    }
    if (layout<0) layout=1;/*fallback: 2 to 3 strip*/
    for (int b=0;b<8;++b) { World.Sys_UI.elevButtonLabelIdx[b]=elevLayouts[layout][b].label; World.Sys_UI.buttonsDarkened[b]=elevLayouts[layout][b].darkened; World.Sys_UI.buttonsEnabled[b]=elevLayouts[layout][b].label>=0; World.Sys_UI.elevButtonLevelIdx[b]=ElevLabelToLevel(elevLayouts[layout][b].label); World.Sys_UI.elevButtonSpawnIdx[b]=U16_MAX; }
    ForceInventoryMode(); play_wav(sounds[91],AppliedFXVol(1.0f),(V3){0.0f,0.0f,0.0f},false); MFD_OpenData(false,1);
}

/*Find the keypad position on a given level (for elevator relative-positioning). Returns false if not found.*/
bool FindElevatorKeypadPos(u8 level,V3* outPos) {
    for (u32 m=0;m<sizeof(elevPanelMap)/sizeof(elevPanelMap[0]);++m) {
        if (elevPanelMap[m].level==level) { outPos->x=elevPanelMap[m].x; outPos->y=elevPanelMap[m].y; outPos->z=elevPanelMap[m].z; return true; }
    }
    return false;
}

static void KeycodePanelUse(u16 i) {
    if(!PanelUseAllowed(i))return;
    Entity* e=&World.instances[i];
    if(e->useQuestKeycode1||e->useQuestKeycode2){
        const i8 *c=&World.lev1SecCode; int off=e->useQuestKeycode1?0:3; u16 msg=e->useQuestKeycode1?289:290;
        if(c[off]<0||c[off+1]<0||c[off+2]<0){UIBlockedBySecurity(World.position[i]);CenterStatusPrint("%s",Sys_Text.stringTable[msg]);return;}
        e->keycode=(u16)(c[off]*100 + c[off+1]*10 + c[off+2]);
    }
    SystemUI* s=&World.Sys_UI; s->tetheredKeypadKeycode=i; s->keycodeValue=e->keycode; s->keycodeValid=true; s->keycodeSolved=false;
    s->keycodeHuns=s->keycodeTens=s->keycodeOnes=-1; s->keycodeEntry=-1; s->objectInUsePos=World.position[i]; s->usingObject=true;
    ForceInventoryMode(); play_wav(sounds[91],AppliedFXVol(1.0f),(V3){0.0f,0.0f,0.0f},false); MFD_OpenData(false,2);
}
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
    e->contents[slot]=e->custIdx[slot]=-1;
    if (Sys_Settings.QuickItemPickup) AddItemToInventory(World.invP1.heldObjectIndex,World.invP1.heldObjectCustIdx),ResetHeldItem(); else ForceInventoryMode(),CenterStatusPrint("%s%s",Sys_Text.stringTable[item+326],Sys_Text.stringTable[319]);/*Inventory mode is turned on when picking something up*/
    for (u8 i=0;i<4;++i) if (e->contents[i]>=0) return true;
    CloseSearch(); return true;
}

void SearchObject(int searchable) {
    UpdateSearchTether();
    if (searchable<INSTS_1ST_IDX || searchable>=World.instCount || !(World.instances[searchable].entflags&EF_ACTIVE)) return;
    Entity* e = &World.instances[searchable];
    if (World.Sys_UI.tetheredSearchable==searchable && e->srchInUse) {
        for (u8 slot=0;slot<4;++slot) {
            if (e->contents[slot]>=0) {
                SearchTakeSlot(slot);
                return;
            }
        }
        
        return;
    }
    CloseSearch();
    World.Sys_UI.tetheredSearchable = World.invP1.currentSearchItem = (u16)searchable;
    e->srchInUse = true;
    World.Sys_UI.objectInUsePos = World.position[searchable];
    World.Sys_UI.usingObject = true;
    MFD_OpenSearch(World.Sys_UI.lastSearchSideRH);
    SearchFXEnable(World.Sys_UI.lastSearchSideRH ? 1 : 0);
    play_wav(sounds[91], AppliedFXVol(0.75f), (V3){0}, false);
    ForceInventoryMode();
}

void MissionTimerInit(void) {
    World.misTimerT = 6000.0f;
    World.misTimerFinished = World.pauseRelativeTime + 1.0f;
    World.misTimerMission = 504;
    World.misTimerCurIdx = 0;
    World.misTimerLast = World.misTimerTimesUP = false;
}

void MissionTimerUpdateToNextMission(float newTimerAmount,int misTextIndex,int nextMissionIndex) {
    if (World.misTimerCurIdx == (u8)nextMissionIndex) return;
    if (World.diffMis < 3) return;
    
    World.misTimerT = newTimerAmount;
    World.misTimerCurIdx = (u8)nextMissionIndex;
    World.misTimerMission = (u16)misTextIndex;
    if (World.misTimerCurIdx == 4) { World.misTimerLast = true; /*No gameover for last timer.*/ }
}

void MissionTimerUpdate(void) {
    if (World.diffMis < 3) return;
    if (World.paused || World.menuActive) return;
    if (World.curLev == LEVEL_CYBERSPACE) return;
    
    if (World.misTimerTimesUP) {
        if (World.instances[PLAYER1].health > 0.0f) {
            World.invP1.radiationArea = true; World.instances[PLAYER1].radiation += 0.1f;
            return;
        }
    }
    
    if (World.misTimerT <= 0.0f) {
        if (World.misTimerLast) {
            World.misTimerMission = 509;
            World.misTimerTimesUP = true;
            return;
        }
        
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

static const char* UseSecretText(int index, u16 inst) { /*sshhhhhh, pretend you didn't see this ;)*/
    if (index < 860 || index > 862 || inst >= World.instCount) { return NULL; }
    u16 n = World.instances[inst].targetnameIdx;
    return (n && n < MAX_IO_NAMES) ? ioNames[n] : NULL;
}

static void CyberportUse(u16 i) { u8 src = World.currentLevel; int idx; if (src == 2) idx = (World.position[i].x < -26.0f) ? 2 : 3; else if (src == 5) idx = 4; else if (src == 6) idx = 5; else if (src == 8) idx = 6; else if (src == 9) idx = 7; else idx = (src < 8) ? src : 0; queuedLevelPos = cyberSpaceEntryLocations[idx]; queuedLevelToLoad = LEVEL_CYBERSPACE; }
void UseEntity(u16 i) {
    Entity* ent = &World.instances[i];
    if (   IdxIsSearchable(ent->index)
        || (World.layer[i]&(L_Corpse|L_CorpseSearchable))
        || (IdxIsGib(ent->index) && (World.layer[i] & L_Corpse)) ) {
        
        SearchObject(i);
    } else if (IdxIsDoor(ent->index)) { DoorUse(i,PLAYER1); }
    else if (IdxIsNPC(ent->index)) { CenterStatusPrint("%s%s",Sys_Text.stringTable[29],npcTable[World.instances[i].index - 419].name); }
    else if (IdxIsButtonSwitch(ent->index)) { ButtonSwitchUse(i,PLAYER1); }
    else if (ent->index==574) { HealingBedUse(i,PLAYER1); }
    else if (ent->index==546) { ChargeStationUse(i,PLAYER1); }
    else if (ent->index==556) { CyberportUse(i); }
    else if (ent->index==614||ent->index==602) { RelayPanelUse(i); }
    else if (IsElevatorPanel(ent->index)) { ElevatorPanelUse(i); }
    else if (ent->index==608) { KeycodePanelUse(i); }
    else if (IsPuzzleGridPanel(ent->index)||IsPuzzleWirePanel(ent->index)) { PuzzlePanelUse(i); }
    else if (ent->index==603) { PaperLogUse(i); }
    else if (IdxIsGeometry(ent->index)) {
        int t = UseNameTableIndex(ent->index);
        const char* sec = UseSecretText(ent->index,i);
        CenterStatusPrint("%s%s", Sys_Text.stringTable[29], sec ? sec : (t >= 0 ? Sys_Text.stringTable[t] : ""));
    } else if (IdxIsUsableObject(ent->index)) {
        World.invP1.holdingObject = true;
        World.invP1.heldObjectIndex = ent->index;
        World.invP1.heldObjectCustIdx = ent->customIndex;
        World.invP1.heldAmmo = ent->ammo;
        World.invP1.heldAmmo2 = ent->ammo2;
        World.invP1.heldObjectLoadedAlternate = ent->heldObjectLoadedAlternate;
        if (Sys_Settings.QuickItemPickup) { 
            AddItemToInventory(ent->index,ent->customIndex);
            ResetHeldItem();
        } else {
            CenterStatusPrint("%s%s", Sys_Text.stringTable[World.invP1.heldObjectIndex - 307 + 326], Sys_Text.stringTable[319]); /* picked up.*/
            ForceInventoryMode();
        }
        
        DeleteInstance(i);
    } else {
        int t = UseNameTableIndex(ent->index);
        const char* sec = UseSecretText(ent->index,i);
        CenterStatusPrint("%s%s",Sys_Text.stringTable[29],sec ? sec : (t >= 0 ? Sys_Text.stringTable[t] : ""));
    }
}

INLINE V3 ScreenPointToRayOffset(V3 f,V3 r,float dx,float dy){float px=(World.inventoryMode?(float)World.cursorPos_x:(float)UI_W*0.5f)+dx,py=(World.inventoryMode?(float)World.cursorPos_y:(float)UI_H*0.5f)+dy,t=vtan((float)Sys_Settings.FOV*0.5f*PI/180.0f),aspect=(float)Sys_Settings.ScreenWidth/(float)Sys_Settings.ScreenHeight,nx=(px-(float)UI_W*0.5f)/((float)UI_W*0.5f),ny=((float)UI_H*0.5f-py)/((float)UI_H*0.5f);V3 v=V3_Normalize((V3){nx*aspect*t,ny*t,-1.0f}),ff=(V3){-f.x,-f.y,-f.z},up=V3_Normalize(V3_Cross(r,ff));return(V3){v.x*r.x+v.y*up.x+v.z*ff.x,v.x*r.y+v.y*up.y+v.z*ff.y,v.x*r.z+v.y*up.z+v.z*ff.z};}
V3 ScreenPointToRayPixels(V3 f,V3 r,float dx,float dy){return ScreenPointToRayOffset(f,r,dx,dy);}
INLINE bool FrobRayIsFrobable(RaycastHit h){if(!h.hit)return false;u16 i=h.hitInstanceIndex;if(i>=World.instCount)return false;u16 e=World.instances[i].index;if((World.layer[i]&(L_Corpse|L_CorpseSearchable)))return true;return IsFrobUsableSpecial(e)||IdxIsUsableObject(e)||IdxIsSearchable(e)||IdxIsDoor(e)||IdxIsButtonSwitch(e)||IdxIsNPC(e)||IdxIsGib(e);}
extern bool editFieldEditing;
static bool TargetIDFrob(V3 p,V3 f,V3 r){V3 dir=ScreenPointToRayOffset(f,r,0,0);RaycastHit h=Raycast(p,dir,TargetIDGetSensingRange(true),LMASK_PLAYER_TARGET_ID_FROB);if(!h.hit||h.hitInstanceIndex>=World.instCount||!IdxIsNPC(World.instances[h.hitInstanceIndex].index))return false;u16 i=h.hitInstanceIndex;Entity* e=&World.instances[i];if((e->entflags&(EF_DYING|EF_DEAD))||!HasHealth(i)){if(World.layer[i]&(L_Corpse|L_CorpseSearchable)){UseEntity(i);return true;}return false;}/*Death flags, not health: AIDead restores health to NPC_CORPSE_HEALTH on the 15 searchCollider types (mask bit 4 = cyborg drone) so corpses stay shootable, which made a fully dead drone test as alive here.*/if((World.invP1.hasHardware&HW_TID)&&World.invP1.hwVers[HW_TID_IDX]>1){if(targetIDAttached[i]&&targetIDAttachedFinished[i]<=World.pauseRelativeTime)targetIDAttached[i]=false;if(!targetIDAttached[i]){CreateTargetIDInstance(-1.0f,i,-1.0f);return true;}}CenterStatusPrint("%s%s",Sys_Text.stringTable[29],npcTable[e->index-419].name);return true;}
static void Frob(V3 p,V3 f,V3 r){
    if(World.uiIsBlocking||World.curLev==LEVEL_CYBERSPACE)return;
    if(Cheats.editMode){V3 d0=ScreenPointToRayOffset(f,r,0,0);RaycastHit fh=Raycast(p,d0,World.farPlane[World.curLev],LMASK_PLAYER_FROB);editModeSelection=(fh.hit&&fh.hitInstanceIndex>=INSTS_1ST_IDX&&fh.hitInstanceIndex<World.instCount)?fh.hitInstanceIndex:U16_MAX; if(editModeSelection<U16_MAX){editFieldEditing=false; CenterStatusPrint("Selected object %u (const index %u)",editModeSelection,World.instances[editModeSelection].index);}else{CenterStatusPrint("Object deselected");}return;/*No pickup/search/use while in edit mode; selection only.*/}
    if(World.Sys_UI.vmailActive){World.Sys_UI.vmailActive=0;return;}if(World.invP1.holdingObject){if(FrobHeldItemIntoPanel(p,f,r)){return;} DropHeldItem();return;}if(TargetIDFrob(p,f,r))return;float o=(float)UI_H*0.02f;RaycastHit fh={0},bh={0};bool ok=false;V3 d0=ScreenPointToRayOffset(f,r,0,0);fh=Raycast(p,d0,FROB_DISTANCE,LMASK_PLAYER_FROB);bh=fh;ok=FrobRayIsFrobable(fh);float ox[8]={0,0,o,-o,o,-o,-o,o},oy[8]={-o,o,0,0,o,-o,o,-o};for(int i=0;i<8&&!ok;++i){V3 d=ScreenPointToRayOffset(f,r,ox[i],oy[i]);RaycastHit th=Raycast(p,d,FROB_DISTANCE,LMASK_PLAYER_FROB);if(FrobRayIsFrobable(th)){bh=th;ok=true;}}if(!ok)bh=fh;if(Cheats.showPhys){World.debugLine_start=p;World.debugLineFinished=World.pauseRelativeTime+3.0;V3 dbg=ok?ScreenPointToRayOffset(f,r,0,0):d0;RaycastHit dh=ok?bh:fh;World.debugLine_end=dh.hit?dh.point:(V3){dbg.x*FROB_DISTANCE+p.x,dbg.y*FROB_DISTANCE+p.y,dbg.z*FROB_DISTANCE+p.z};}if(!ok){if(fh.hit){u16 idx=fh.hitInstanceIndex;if(idx<World.instCount){u16 ei=World.instances[idx].index;if(IdxIsGeometry(ei)||IdxIsDoor(ei)||World.instances[idx].index>=595){int t=UseNameTableIndex(ei);const char* sec=UseSecretText(ei,idx);CenterStatusPrint("%s%s",Sys_Text.stringTable[29],sec?sec:(t>=0?Sys_Text.stringTable[t]:""));return;}}}CenterStatusPrint("%s",Sys_Text.stringTable[30]);}else UseEntity(bh.hitInstanceIndex);}
// Update
void WeaponsUpdate(); void TextureSequenceUpdate(u16 self); void AIAnimationControllerUpdate(u16 selfIdx); void AIControllerUpdate(u16 selfIdx);
extern const V3 sightPointOffsets[NUM_AI_TYPES];
void DrawAIDebug(u16 i) {
    if ((!IdxIsNPC(World.instances[i].index)) || !Cheats.showNPC) return; World.layer[i] = L_NPC; World.layer[PLAYER1] = L_Player; Quaternion r = World.rotation[i]; float x=r.x,y=r.y,z=r.z,w=r.w; V3 fwd = V3_Normalize((V3){2.0f*(x*z + w*y), 0.0f, 1.0f - 2.0f*(x*x + y*y)}); u16 npcIdx = World.instances[i].index - 419;
    V3 sightPt = V3_AplusB(World.position[i],quat_rot_v3(World.rotation[i],sightPointOffsets[npcIdx])); DrawLine(sightPt,V3_AplusB(sightPt,V3_ScaleByF(fwd,0.6f)),(Color){1.0f,1.0f,0.0f,1.0f}); V3 enemPt = World.position[PLAYER1]; enemPt.y -= 0.24f;
    RaycastHit hit = Raycast(sightPt,V3_AsubB(enemPt,sightPt),20.0f,LMASK_NPC_SIGHT); if (hit.hit && hit.hitInstanceIndex == PLAYER1) { DrawLine(sightPt,hit.point,(Color){1.0f,0.0f,0.0f,1.0f}); } else {DrawLine(sightPt,enemPt,(Color){0.0f,1.0f,1.0f,1.0f});} Entity* e = &World.instances[i]; Color dbgCol;
    if (e->currentState == AIState_Idle) dbgCol = (Color){0.0f,1.0f,0.0f,1.0f}; else if (e->currentState == AIState_Walk || e->currentState == AIState_Run) { if (e->entflags & EF_ENEM_IN_SIGHT) dbgCol = (Color){1.0f,0.0f,0.0f,1.0f}; else dbgCol = (Color){1.0f,1.0f,0.0f,1.0f}; }
    else if (e->currentState == AIState_Attack1 || e->currentState == AIState_Attack2 || e->currentState == AIState_Attack3) dbgCol = (Color){1.0f,0.0f,1.0f,1.0f}; else if (e->currentState == AIState_Pain) dbgCol = (Color){1.0f,0.0f,1.0f,1.0f}; else if (e->currentState == AIState_Dead) dbgCol = (Color){0.5f,0.5f,0.5f,1.0f}; else { dbgCol = (Color){1.0f,0.9f,0.8f,1.0f}; }
    DrawSphereWireframe(dbgCol, (ShapeSphere){sightPt, 0.32f});
}

INLINE float CamYawFromQuat(Quaternion q) {
    float d = __builtin_atan2f(2.0f * (q.x*q.z + q.y*q.w), 1.0f - 2.0f * (q.x*q.x + q.y*q.y)) * (180.0f / 3.14159265f);
    if (d < 0.0f) d += 360.0f; return d;
}

void SecurityCameraRotateUpdate(u16 self) { /*Sweeps the camera back and forth between startYAngle and endYAngle about the Y axis (yaw) only*/
    Entity* e = &World.instances[self];
    if (!e->camRotateEnabled) return;
    if (!(e->entflags & EF_ACTIVE)) return;
    if (!SphereInFrustum(playerFrustumPlanes, World.position[self], 1.28f)) return;
    
    if (!e->camSweepInit) { e->camSweepInit = true; e->camBaseRot = World.rotation[self]; e->camYaw0 = CamYawFromQuat(e->camBaseRot); e->camYaw = e->camYaw0; }
    if (e->camWaitingFinished >= World.pauseRelativeTime) return;
        
    float dt = World.dt;
    if (e->camRotatePositive) { // flip
        if (vabs(e->camYaw - e->camEndYAngle) <= 1.0f) {
            e->camRotatePositive = false;
            e->camWaitingFinished = World.pauseRelativeTime + e->camWaitTime;
        } else e->camYaw += 24.0f * dt;
    } else { // ney ney
        if (vabs(e->camYaw - e->camStartYAngle) <= 1.0f) {
            e->camRotatePositive = true;
            e->camWaitingFinished = World.pauseRelativeTime + e->camWaitTime;
        } else e->camYaw -= 24.0f * dt;
    }
    
    if (e->camYaw < 0.0f) { e->camYaw += 360.0f; }
    else if (e->camYaw >= 360.0f) { e->camYaw -= 360.0f; }
    
    float half = deg2rad(e->camYaw - e->camYaw0) * 0.5f;
    Quaternion yaw = {0.0f, vsinf(half), 0.0f, vcosf(half)};
    World.rotation[self] = quat_multiply(yaw, e->camBaseRot);
}

void ModUpdate() {
    if (World.paused || World.menuActive) return;
    
    UpdateSearchTether();
    WeaponsUpdate();
    InventoryUpdate();
    PlayerEnergyUpdate();
    PatchUpdate();
    HardwareUpdate();
    MissionTimerUpdate();
    if (Use()) { Frob(World.position[PLAYER1],World.instances[PLAYER1].forward,World.instances[PLAYER1].right); }
    if (World.pauseRelativeTime < World.debugLineFinished && (World.debugLineVertCount + 6) < MAX_WIRELINE_VRTS) { DrawLine(World.debugLine_start,World.debugLine_end,(Color){0.3f,0.1f,0.6f,0.5f}); }
    for (u16 i=INSTS_1ST_IDX;i<World.instCount;++i) {
        Entity* e = &World.instances[i];
        u16 constdex = e->index;
        if(IsLiveGrenade(constdex) && (e->entflags & EF_ACTIVE)) { GrenadeUpdate(i); }
        DelayedSpawnUpdate(i);
        if (constdex==CYBER_DECOY_CONST) { CyberDecoyExpired(i); }
        if (constdex==614) { RelayPanelUpdate(i); }
        if (IsPuzzleGridPanel(constdex)) { PuzzlePanelUpdate(i); }
        if (e->textureAnimating && e->tickFinished < World.pauseRelativeTime) { TextureSequenceUpdate(i); }
        if (IdxIsButtonSwitch(constdex)){ ButtonSwitchUpdate(i); }
        if (IdxIsDoor(constdex)) { DoorUpdate(i); }
        if (constdex == 701) { LogicTimerUpdate(i); }
        if (constdex == 594) { TriggerCounterUpdate(i); }
        if (constdex == 699) { LogicRelayUpdate(i); }
        if (constdex == 598 || constdex == 600) { TriggerTrippedUpdate(i); }
        if (constdex == 702) { SpawnManagerUpdate(i); }
        if (constdex == 477) { SecurityCameraRotateUpdate(i); }
        if (e->itemLifeTime > 0.0f) { SearchFXResetUpdate(i); }
        if (e->cyberTimer > 0.0f) { CyberTimerUpdate(i); }
        if (constdex == 515) { ForceBridgeUpdate(i); }
        if (constdex == 517) { FuncWallUpdate(i); }
        if (constdex == 21 || constdex == 22) { CyberWallUpdate(i); }
        if (IdxIsNPC(constdex)) {
            DrawAIDebug(i);
            AIControllerUpdate(i);
            AIAnimationControllerUpdate(i);
        }
        
        if (constdex == 552) { CyberDataFragUpdate(i); }
        if (constdex==554) { CyberExitUpdate(i); }
        if (constdex==555) { CyberSwitchUpdate(i); }
        if (constdex>=448 && constdex <= 457) { CyberItemUpdate(i); }
    }
    
    if (World.invP1.painSoundFinished < World.pauseRelativeTime && World.instances[PLAYER1].radiation > 1.0f && !(World.invP1.radSoundFinished < World.pauseRelativeTime)) {
        World.invP1.painSoundFinished = World.pauseRelativeTime + (double)random_range(2.5f,4.0f);
        play_wav(sounds[140]/*player/playerpain1*/,AppliedFXVol(0.2f),(V3){0},false);
    }
    
    if (!Cheats.god && World.invP1.radBleedFinished < World.pauseRelativeTime && World.instances[PLAYER1].radiation > 1.0f) { /*Radiation damage pulse*/
        World.invP1.radBleedFinished = World.pauseRelativeTime + 1.8;
        float take = World.instances[PLAYER1].radiation * 0.2f;
        World.instances[PLAYER1].health -= take;
        World.painStaticAlpha = take > 15.0f ? 1.0f : take > 10.0f ? 0.8f : 0.3f;
    }
    
    if (World.invP1.radSoundFinished < World.pauseRelativeTime && World.instances[PLAYER1].radiation > 1.0f) {
        double minT = World.instances[PLAYER1].radiation > 50.0f ? 0.5 : 1.0;
        World.invP1.radSoundFinished = World.pauseRelativeTime + minT + (double)random_range(0.0f, 2.0f);
        play_wav(sounds[90]/*hud/radiation*/, AppliedFXVol(0.18f), (V3){0.0f, 0.0f, 0.0f}, false);
    }
    
    if (World.Sys_UI.tetheredPGP != U16_MAX) { /*Panel tether sever if player moves too far*/
        u16 pg = World.Sys_UI.tetheredPGP;
        if (pg < World.instCount && (World.instances[pg].entflags & EF_ACTIVE)) {
            float d2 = V3_SqDist(World.position[PLAYER1], World.position[pg]);
            if (d2 > 64.0f) { UI_PuzzleGridClose(); }
        }
    }
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
    } return MAXTEX;
}

u16 GetCursorTexture() {
    if (World.paused || World.menuActive) { if (Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].down || Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].down){ return 2147; } return 1261; }
    if (World.invP1.holdingObject) { u16 tex = GetItemFrobTexture(World.invP1.heldObjectIndex); return tex < MAXTEX ? tex : 1250; }
    if (World.uiIsBlocking || World.Sys_UI.mouseClickHeldOverGUI) { if (Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].down || Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].down) { return 2147; } return 1261; }
    return GetCrosshairTexture();
}
