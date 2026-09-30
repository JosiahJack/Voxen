// citadel.c - Game logic.
#include "common.h"
__attribute__((used)) AutoSplitterData autoSplitter = {0x1337133713371337,0,false,0}; static const u16 patchMsg[7] = {340,341,342,343,344,345,346}; /*Patch names in the +326 item-name table (Citadel: useableItemIndex+326, useableItemIndex=14+slot); Citadel composes "<name>" + stringTable[589]*/
void PatchDisableAll(),BiomonitorEnergyPulse(float),BioMonitorClearGraphs(),TextureSequenceInit(u16,char*),GrenadeActivate(u16); bool RecentLog(); extern double lerpStartTime; extern V3 queuedLevelPos; extern u8 queuedLevelToLoad; extern u16 editModeSelection;
/*Cyber decoy lifecycle + AI plumbing.  CyberDecoyRetargetAll lives in ai.c: it is the single choke point where a
  cyber NPC's enemy pointer is forced from the player onto the live decoy instance (or handed back to the player
  once the decoy expires), so no LOS check has to remember to consult the decoy itself.*/
void CyberDecoySetTarget(u16); void CyberDecoyRetargetAll(void); void CyberDecoySpawned(u16); void CyberDecoyExpired(u16); bool CyberDecoyIsLive(u16*); u16 CyberDecoyTarget(void);
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
    } bool depleted = World.invP1.patchCounts[patchSlot] <= 0; if (depleted) { for (int i = 0; i < 7; i++) { if (World.invP1.patchCounts[i] > 0) { World.invP1.patchCur = (i8)i; break; } } } if (depleted) CenterStatusPrint("%s%s%s",Sys_Text.stringTable[590],Sys_Text.stringTable[patchMsg[patchSlot]],Sys_Text.stringTable[589]); else CenterStatusPrint("%s%s",Sys_Text.stringTable[patchMsg[patchSlot]],Sys_Text.stringTable[589]); play_wav(sounds[89],AppliedFXVol(1.0f),(V3){0.0f,0.0f,0.0f},false);
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
void PlayLog(int logIndex) {if(logIndex<0||logIndex>=LOGCNT||!(World.invP1.hasHardware&HW_ERD)){return;} play_message(AudioLogPath(logIndex)); World.invP1.readLog[logIndex]=true; if(Sys_Text.audioLogType[logIndex] == AudioLogType_Vmail){World.Sys_UI.vmailActive=true;} CenterStatusPrint("%s%s",Sys_Text.stringTable[1020],World.audiologNames[logIndex]); MFD_OpenAudioLog(logIndex);/*Unity PlayLog ends with MFDManager.SendAudioLogToDataTab(logIndex)*/}
void PlayLastAddedLog(int logIndex) { if(logIndex < 0){return;} PlayLog(logIndex); World.invP1.lastAddedIndex = -1; }
void AddAudioLogToInventory(int index) {
    if (index < 0) { DualLog("BUG: Audio log picked up has no assigned index (-1)"); return; } if (index == 128) { CenterStatusPrint("%s",Sys_Text.stringTable[309]); return; }/*Trioptimum Funpack*/ World.invP1.hasLog[index]  = true; World.invP1.lastAddedIndex = index; World.invP1.numLogsFromLevel[Sys_Text.audioLogLevelFound[index]]++;
    if(Sys_Text.audioLogType[index] == AudioLogType_Email)World.invP1.hasNewEmail=true;else if(Sys_Text.audioLogType[index]==AudioLogType_Normal)World.invP1.hasNewLogs=true;
    if (World.invP1.hasHardware & HW_ERD) { char keyStr[8]; sFormat(keyStr,sizeof(keyStr),"%s", Sys_Settings.InputCodeSettings[20] ? "U" : "?"); CenterStatusPrint("%s%s%s %s",Sys_Text.stringTable[36],World.audiologNames[index],Sys_Text.stringTable[38],keyStr); } else { CenterStatusPrint("%s%s%s",Sys_Text.stringTable[36],World.audiologNames[index],Sys_Text.stringTable[310]); }
}

static inline void ItemAdd(u8 *cur, u8 *counts, int idx, int uIdx, int sysIdx) { if (!counts[*cur]) {*cur=(i8)idx;} counts[idx]++; CenterStatusPrint("%s%s", Sys_Text.stringTable[ItemStringIdx(uIdx)], Sys_Text.stringTable[sysIdx]); }
void AddGrenadeToInventory(int i, int u) { if (i >= 0){if (!World.Sys_UI.firstMain){World.Sys_UI.firstMain=true; World.Sys_UI.MFD_CenterTab=1;} World.invP1.grenConstIndex[i]=(i16)u; ItemAdd(&World.invP1.grenCur,World.invP1.grenAmmo,i,u,34);} }
void   AddPatchToInventory(int i, int u) { if (i >= 0){if (!World.Sys_UI.firstMain){World.Sys_UI.firstMain=true; World.Sys_UI.MFD_CenterTab=1;} ItemAdd(&World.invP1.patchCur,World.invP1.patchCounts,i,u,35);} }
static inline void GrenadeCycle(int step){int cur= World.invP1.grenCur, next=cur; for(int i=0;i<7;++i){next=(next+step+7)%7; if(   World.invP1.grenAmmo[next]>0){ if (next==cur) return; World.invP1.grenCur =(i8)next; play_wav(sounds[80],AppliedFXVol(1.0f),(V3){0,0,0},false);/*changeweapon*/ CenterStatusPrint("%s",Sys_Text.stringTable[579+next]); return;}}}
static inline void   PatchCycle(int step){int cur=World.invP1.patchCur, next=cur; for(int i=0;i<7;++i){next=(next+step+7)%7; if(World.invP1.patchCounts[next]>0){World.invP1.patchCur=(i8)next; play_wav(sounds[80],AppliedFXVol(1.0f),(V3){0,0,0},false);/*changeweapon*/ CenterStatusPrint("%s",Sys_Text.stringTable[patchMsg[next]]); return;}}}
void RemoveGrenade(int i) { if(World.invP1.grenAmmo[i] > 0){World.invP1.grenAmmo[i]--;} if(!World.invP1.grenAmmo[i]){GrenadeCycle(-1);} }
static i8 GetExistingCyberItemIndex() { if (World.invP1.softVersions[SW_TURBO]  > 0) {return 0;} if (World.invP1.softVersions[SW_DECOY]  > 0) {return 1;} if (World.invP1.softVersions[SW_RECALL] > 0) {return 2;} return -1; }
static void UseTurbo() {if(World.invP1.softVersions[SW_TURBO]<=0){World.invP1.hasSoft&=(u8)~(1u << SW_TURBO); return;} if(--World.invP1.softVersions[SW_TURBO]==0)World.invP1.hasSoft&=(u8)~(1u << SW_TURBO); if(World.invP1.turboFinished > World.pauseRelativeTime){World.invP1.turboFinished+=World.invP1.turboCyberTime;}else{World.invP1.turboFinished=World.invP1.turboCyberTime+World.pauseRelativeTime;}}
/* Cyber decoy.  Unity (Inventory.SpawnDecoy) instantiates prop_cyber_decoy (constIndex 553) at the player and
   DelayedSpawn (delay 15, destroyOnExpire) tears it down after 15s; the prefab has no HealthManager, so nothing else
   can remove it.  Voxen was spawning 417 instead -- item_cyber_decoy, a usable-object pickup -- which dropped a card on
   the floor, and World.decoyActive was then left claiming a decoy existed for the rest of the level.
   Collider radius 0.48 (prefabs SphereCollider 0.96 under the root's 0.5 local scale) comes from EDefs[553], and the
   decoy keeps EF_RIGIDBODY off because the prefab has no Rigidbody -- it holds position instead of falling.
   Reusing the existing DelayedSpawnUpdate arming (active + timerFinished + doSelfAfterList + despawnInstead) is the
   same 15s teardown Unity's DelayedSpawn performs, so the expiry goes through CyberDecoyExpired. */
#define CYBER_DECOY_LIFETIME 15.0
static void UseDecoy() {if (World.decoyActive) { CenterStatusPrint("%s",Sys_Text.stringTable[537]); return; } if (World.invP1.softVersions[SW_DECOY] <= 0) { World.invP1.hasSoft &= (u8)~(1u << SW_DECOY); return; } if (--World.invP1.softVersions[SW_DECOY] == 0) World.invP1.hasSoft &= (u8)~(1u << SW_DECOY); u16 decoyIdx = SpawnDynamicObject(CYBER_DECOY_CONST,true);
    if(decoyIdx != U16_MAX){World.position[decoyIdx]=World.position[PLAYER1]; World.velocity[decoyIdx]=(V3){0,0,0}; flag_set(&World.instances[decoyIdx].entflags,EF_RIGIDBODY,false);
        World.instances[decoyIdx].active = true; World.instances[decoyIdx].doSelfAfterList = true; World.instances[decoyIdx].despawnInstead = true; World.instances[decoyIdx].timerFinished = World.pauseRelativeTime + CYBER_DECOY_LIFETIME;
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
    if (slot < 0 || slot >= 7 || World.invP1.weaponInventoryIndices[slot] < 0) return;
    if (World.invP1.holdingObject) return; /* hands full, same refusal as GeneralInvTake */
    /* The weapon leaves the 7-slot list and becomes the held object, carrying its loaded magazines: AddItemToInventory -> AddWeaponToInventory reads heldAmmo/heldAmmo2/heldObjectLoadedAlternate to put them back (ADD TO INVENTORY, or a quick pickup). heldObjectIndex is the EDD, and a weapon's EDD is 343..358 - the very value weaponInventoryIndices already holds - so store it verbatim. Subtracting 307 was wrong: IdxIsWeapon, GetItemFrobTexture and ItemStringIdx all key off 343..358, and the -307 consumers (RelayPanelUse, HeldItemIsFrobUser) want the EDD too. */
    World.invP1.heldAmmo = World.invP1.currentMagazineAmount[slot]; World.invP1.heldAmmo2 = World.invP1.currentMagazineAmount2[slot]; World.invP1.heldObjectLoadedAlternate = World.invP1.wepLoadedWithAlternate[slot];
    World.invP1.heldObjectCustIdx = U16_MAX; World.invP1.heldObjectIndex = (u16)World.invP1.weaponInventoryIndices[slot]; World.invP1.grenActive = false; World.invP1.holdingObject = true;
    /* Unity's WeaponCurrent holds a List and RemoveWeapon drops the entry, so the rows below close up. Every per-slot array has to slide with it or the MFD renders a blank row (it draws row generalRowY[slot] for a populated slot, so a -1 hole leaves a gap) and the magazines/heat/energy setting desync from their weapon. */
    for (i32 i=slot;i<6;++i) { World.invP1.weaponInventoryIndices[i]=World.invP1.weaponInventoryIndices[i+1]; World.invP1.weaponInventoryAmmoIndices[i]=World.invP1.weaponInventoryAmmoIndices[i+1]; World.invP1.currentMagazineAmount[i]=World.invP1.currentMagazineAmount[i+1]; World.invP1.currentMagazineAmount2[i]=World.invP1.currentMagazineAmount2[i+1]; World.invP1.wepLoadedWithAlternate[i]=World.invP1.wepLoadedWithAlternate[i+1]; World.invP1.currentEnergyWeaponHeat[i]=World.invP1.currentEnergyWeaponHeat[i+1]; World.invP1.weaponEnergySetting[i]=World.invP1.weaponEnergySetting[i+1]; }
    World.invP1.weaponInventoryIndices[6]=World.invP1.weaponInventoryAmmoIndices[6]=-1; World.invP1.currentMagazineAmount[6]=World.invP1.currentMagazineAmount2[6]=0; World.invP1.wepLoadedWithAlternate[6]=false; World.invP1.currentEnergyWeaponHeat[6]=0.0f; World.invP1.weaponEnergySetting[6]=0.0f;
    /* Everything at or above the hole moved down one, so the held row does too; if the held weapon was the one removed this lands on the weapon above it (Unity: WeaponCurrent.RemoveWeapon). */
    if (slot <= World.invP1.weaponCurrent && World.invP1.weaponCurrent > 0) World.invP1.weaponCurrent--;
    if (World.invP1.weaponCurrent < 0) World.invP1.weaponCurrent = 0;
    World.invP1.weaponCurrentPending = World.invP1.weaponCurrent;
    World.invP1.weaponIndexPending = (World.invP1.weaponCurrent >= 0 && World.invP1.weaponInventoryIndices[World.invP1.weaponCurrent] >= 0) ? (i16)World.invP1.weaponInventoryIndices[World.invP1.weaponCurrent] : -1;
    u8 numweapons = 0; for (int i = 0; i < 7; i++) if (World.invP1.weaponInventoryIndices[i] >= 0) numweapons++;
    World.invP1.numweapons = numweapons;
    if (!numweapons) { for (int i = 0; i < 7; i++) World.invP1.currentMagazineAmount[i] = World.invP1.currentMagazineAmount2[i] = 0; }
    if (World.invP1.weaponIndexPending < 0) { World.instances[World.weaponVModelIndex].modelIndex = MAX_MDLS; World.invP1.weaponCurrentPending = World.invP1.weaponIndexPending = -1; ForceInventoryMode(); return; } /* no valid weapon selected: hide the view model */
    CompleteWeaponChange(); /* updates weapon state, view model, and HUD */
    ForceInventoryMode(); /* so the held weapon and its ADD TO INVENTORY button are on screen */
}
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
void CyberTimerUpdate(u16 self) { if(World.curLev != LEVEL_CYBERSPACE){return;} Entity* e=&World.instances[self]; if(e->cyberTimer <= 0.0f){UIExitCyberspace(); return;} if(e->timerFinished >= World.pauseRelativeTime){return;} e->cyberTimer-=1.0f; e->minutes=vfloor(e->cyberTimer / 60.0f); e->seconds=e->cyberTimer - (e->minutes * 60.0f); e->timerFinished=World.pauseRelativeTime + 1.0; }
void CyberWallInitAfterLoad(u16 self) { Entity* e=&World.instances[self]; e->tickFinished=World.pauseRelativeTime + 2.0; e->animSwapFinished=0.0; } // alpha pushed via glUniform1f(27, ...) in voxen.c
void CyberWallUpdate(u16 self) { Entity* e = &World.instances[self]; if (World.pauseRelativeTime < e->tickFinished) {return;} e->tickFinished = World.pauseRelativeTime + 0.05; }
void ExitCyberspace(void); /*forward decl*/
/*Cyber pickups (448-451,454-457): spherical distance check vs player; pickup deletes the instance.*/
void CyberItemUpdate(u16 self) { Entity* e=&World.instances[self]; if(!(e->entflags&EF_ACTIVE))return; if(V3_Dist(World.position[self],World.position[PLAYER1]) < 1.5f){ DeleteInstance(self); } }
/*Cyber exit (554): spherical distance check vs player; exits cyberspace.*/
void CyberExitUpdate(u16 self) { Entity* e=&World.instances[self]; if(!(e->entflags&EF_ACTIVE))return; if(World.curLev != LEVEL_CYBERSPACE)return; if(V3_Dist(World.position[self],World.position[PLAYER1]) < 2.0f){ ExitCyberspace(); } }
/*Cyber switch (555): spherical distance check vs player; one-shot activate (off->on frame), fires targets.*/
void CyberSwitchUpdate(u16 self) { Entity* e=&World.instances[self]; if(!(e->entflags&EF_ACTIVE))return; if(e->active)return; if(V3_Dist(World.position[self],World.position[PLAYER1]) < 1.5f){ e->active=true; ChangeAnim(e,A_ACTIVATED); if(e->textIndex>0) CenterStatusPrint("%s",Sys_Text.stringTable[e->textIndex]); UseTargets(self,e->targetIdx); } }
/*Cyber data fragment (552): spherical distance check vs player; shows message.*/
void CyberDataFragUpdate(u16 self) { Entity* e=&World.instances[self]; if(!(e->entflags&EF_ACTIVE))return; if(e->allDone)return; if(V3_Dist(World.position[self],World.position[PLAYER1]) < 1.5f){ e->allDone=true; if(e->textIndex>0) CenterStatusPrint("%s",Sys_Text.stringTable[e->textIndex]); } }
void SearchFXEnable(int side) {
    side=side==1; World.Sys_UI.searchFXActive[side]=true; World.Sys_UI.searchFXStartTime[side]=World.pauseRelativeTime;
    World.Sys_UI.searchFXCursorX[side]=(float)World.cursorPos_x; World.Sys_UI.searchFXCursorY[side]=(float)World.cursorPos_y;
}
void SearchFXResetEnable(u16 self) { Entity* e = &World.instances[self]; if (e->itemLifeTime <= 0.0f) {e->itemLifeTime = 3.0f;} e->delayFinished = World.pauseRelativeTime + e->itemLifeTime; }
void SearchFXResetUpdate(u16 self) { Entity* e = &World.instances[self]; if (e->delayFinished >= World.pauseRelativeTime) {return;} flag_set(&e->entflags,EF_ACTIVE,false); }
void DelayedSpawnEnable(u16 self) { Entity* e = &World.instances[self]; e->timerFinished = World.pauseRelativeTime + e->delay; e->active = true; }
void CyberDecoySpawned(u16 decoy) { World.decoyInstance = decoy; CyberDecoySetTarget(decoy);/*cyber NPCs already engaged with the player switch to the decoy the moment it exists (AIController.cs:1630,1708)*/ }
void CyberDecoyExpired(u16 decoy) { if (decoy >= World.instCount || World.instances[decoy].index != CYBER_DECOY_CONST) return; if (World.decoyActive) { World.decoyActive = false; World.decoyInstance = U16_MAX; CyberDecoySetTarget(U16_MAX); } }
/*Whether the recorded decoy instance is still the live one.  World.decoyActive alone is not enough: the instance
  table is refilled on every level load and DeleteInstance compacts it, so a stale index can point at a wall.  This
  validates the cached World.decoyInstance and refreshes it if the decoy moved slots, so the AI hot paths can compare
  an enemy index against World.decoyInstance instead of rescanning the table.*/
bool CyberDecoyIsLive(u16* out) { if (!World.decoyActive) { World.decoyInstance = U16_MAX; return false; } u16 d = World.decoyInstance;
    if (d >= INSTS_1ST_IDX && d < World.instCount && World.instances[d].index == CYBER_DECOY_CONST) { if (out) *out = d; return true; }
    for (u16 i = INSTS_1ST_IDX; i < World.instCount; ++i) if (World.instances[i].index == CYBER_DECOY_CONST) { World.decoyInstance = i; if (out) *out = i; return true; }
    World.decoyInstance = U16_MAX; return false; }
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
    if(!e->active){ e->modelIndex=MAX_MDLS; World.col[self]=COLTYPE_NONE;} switch (e->fieldColor) { case ForceFieldColor_Red:e->texIndex=38; break; case ForceFieldColor_Green:e->texIndex=40; break; case ForceFieldColor_Blue:e->texIndex=39; break; case ForceFieldColor_Purple:e->texIndex=41; break; case ForceFieldColor_RedFaint:e->texIndex=198; break; }
}

void ForceBridgeActivate(u16 s, bool silent){Entity* e=&World.instances[s]; if(e->active){return;} if(!silent){play_wav(sounds[102], AppliedFXVol(1.0f), World.position[s], true);} flag_set(&e->entflags,EF_ACTIVE,true); e->modelIndex=78; World.col[s]=COLTYPE_BOX; e->active=e->lerping=true; World.scale[s]=(V3){ e->forceFieldDirectionX ? 0.1f : e->activatedScale.x,e->forceFieldDirectionY ? 0.1f : e->activatedScale.y,e->forceFieldDirectionZ ? 0.1f : e->activatedScale.z };}
void ForceBridgeDeactivate(u16 self, bool silent) { Entity* e = &World.instances[self]; if (!e->active) {return;} if (!silent) {play_wav(sounds[102], AppliedFXVol(1.0f), World.position[self], true);} e->active = false; e->lerping = true; }
void ForceBridgeToggle(u16 self) { if (World.instances[self].active) {ForceBridgeDeactivate(self,false); } else {ForceBridgeActivate(self,false);} }
void ForceBridgeUpdate(u16 self) {
    Entity* e = &World.instances[self]; if(e->tickFinished >= World.pauseRelativeTime){return;} e->tickFinished = World.pauseRelativeTime + 0.05f;
    if (e->active) {
        if (!e->lerping) return; float sx=e->forceFieldDirectionX ? lerp(World.scale[self].x,e->activatedScale.x,0.1f) : World.scale[self].x, sy=e->forceFieldDirectionY ? lerp(World.scale[self].y,e->activatedScale.y,0.1f) : World.scale[self].y, sz=e->forceFieldDirectionZ ? lerp(World.scale[self].z,e->activatedScale.z,0.1f) : World.scale[self].z; 
        World.scale[self]=(V3){sx,sy,sz}; if(vabs(e->activatedScale.x - sx) < 0.08f && vabs(e->activatedScale.y - sy) < 0.08f && vabs(e->activatedScale.z - sz) < 0.08f){World.scale[self]=e->activatedScale; e->lerping=false;}
    } else if (e->lerping) {float sx=e->forceFieldDirectionX ? lerp(World.scale[self].x,0.0f,0.1f) : World.scale[self].x, sy=e->forceFieldDirectionY ? lerp(World.scale[self].y,0.0f,0.1f) : World.scale[self].y, sz=e->forceFieldDirectionZ ? lerp(World.scale[self].z,0.0f,0.1f) : World.scale[self].z; World.scale[self]=(V3){sx,sy,sz}; if (sx < 0.08f || sy < 0.08f || sz < 0.08f) { e->modelIndex = MAX_MDLS; World.col[self] = COLTYPE_NONE; e->lerping=false;}}
}

void TriggerCounterTarget(u16 self, u16 activator) { UseTargets(activator,World.instances[self].targetIdx); }
void TriggerCounterDelayedTarget(u16 self, u16 act) { Entity* e=&World.instances[self]; e->recentMostActivator = act; e->delayFinished = World.pauseRelativeTime + e->delay; e->deferredIoflags = World.targetIOActive ? World.targetIOActivatorIoflags : (act<World.instCount?World.instances[act].ioflags:0u); e->deferredIoflagsHi = World.targetIOActive ? World.targetIOActivatorIoflagsHi : (act<World.instCount?World.instances[act].ioflagsHi:0u); }/*Capture the activating bits now, while the activator is still resolvable.*//*Unity StartCoroutine(DelayedTarget); the target fires from TriggerCounterUpdate once the delay elapses. This used to also call the target immediately, so the delay was never honored.*/
void TriggerCounterUpdate(u16 self) { Entity* e=&World.instances[self]; if (e->delayFinished <= 0.0 || e->delayFinished >= World.pauseRelativeTime) {return;} e->delayFinished = 0.0; u32 sf=World.targetIOActivatorIoflags,sfh=World.targetIOActivatorIoflagsHi; bool wa=World.targetIOActive; World.targetIOActive=true; World.targetIOActivatorIoflags=e->deferredIoflags; World.targetIOActivatorIoflagsHi=e->deferredIoflagsHi; UseTargets(e->recentMostActivator,e->targetIdx); World.targetIOActivatorIoflags=sf; World.targetIOActivatorIoflagsHi=sfh; World.targetIOActive=wa; }/*Marking the chain active stops UseTargets re-seeding from World.instances[activator], which by now points at the player's level.*/
void LogicRelayDelayTarget(u16 self, u16 act) { Entity* e=&World.instances[self]; e->recentMostActivator = act; e->delayFinished = World.pauseRelativeTime + e->delay; }/*LogicRelay.cs:19,22 defers to StartCoroutine(DelayedTarget) when delay > 0. The relay's own ioflags ride the deferred hop, so they are re-read at fire time rather than captured.*/
void LogicRelayUpdate(u16 self) { Entity* e=&World.instances[self]; if (e->delayFinished <= 0.0 || e->delayFinished >= World.pauseRelativeTime) {return;} e->delayFinished = 0.0; u32 savedFlags = World.targetIOActivatorIoflags, savedFlagsHi = World.targetIOActivatorIoflagsHi; World.targetIOActivatorIoflags = e->ioflags; World.targetIOActivatorIoflagsHi = e->ioflagsHi; UseTargets(e->recentMostActivator, e->targetIdx); World.targetIOActivatorIoflags = savedFlags; World.targetIOActivatorIoflagsHi = savedFlagsHi; }
void TriggerCounterTargetted(u16 self, u16 act) { Entity* e=&World.instances[self]; e->counter++; if (e->counter != e->countToTrigger) {return;} if (e->delay <= 0.0f){TriggerCounterTarget(self,act);}else{TriggerCounterDelayedTarget(self,act);} if (!e->dontReset){e->counter=0;} }

// SpawnManager (constIndex 702).  Spawn points are the info_spawnpoint (715) entities on this level: Unity's
// spawnLocations array is an unordered Random.Range pick and each spawner's list is exactly that set, so the list
// does not need storing.  Two Unity checks have no Voxen equivalent and are noted inline.
void SpawnManagerActivate(u16 self, bool alerted) { Entity* e=&World.instances[self]; e->alertEnemiesOnAwake=alerted; e->active=true; e->delayFinished=World.pauseRelativeTime; }
// AreaHidden + AreaClear for a spawn point (SpawnManager.GetRandomLocation).  Unity rejects a point that is
// AreaHidden (not currently in the player's PVS) or not AreaClear (overlapping an area), then re-rolls.  AreaHidden
// maps onto the culled PVS directly; the spawn point is static, so its entity cellIndex is the cell its position
// resolves to.  AreaClear reuses CantStand's collision path via AreaHasClearance with span 0, i.e. a single
// strict 0.48-radius sphere test at the exact point.
// DIVERGENCE (intended): a bare sphere is a tighter volume than Unity's area overlap, so this is stricter than the
// reference game -- a cramped-but-legal spawn point can be rejected where Unity would accept it.  Pass
// AREA_SWEEP_SPAN instead of 0 to sweep like CantStand does, which makes it far more permissive.
static bool SpawnPointIsClear(u16 spot) {
    V3 p=World.position[spot]; if(World.curLev>=LEVEL_CYBERSPACE||PositionVisibleFromPlayerCell(p.x,p.z)) {/*cyberspace never builds PVS bits (culling.c:151), so treat it as all-visible, matching TargetIDInPlayerPVS. No spawner sits on 13 today.*/
        ShapeCapsule ball={.tip=p,.base=p,.rad=0.48f};/* tip==base is a sphere */
        return AreaHasClearance(ball,ball,0.0f,LMASK_NPC_COLLIDESWITH,0xFFFFu);
    }
    return false;
}
void SpawnManagerUpdate(u16 self) {
    Entity* e=&World.instances[self]; if (World.paused || World.menuActive || !e->active) {return;}/*SpawnManager.Update:40-42*/
    u16 levelNpcs=0,count=0; for (u16 i=INSTS_1ST_IDX;i<World.instCount;++i){ Entity* n=&World.instances[i]; if(!IdxIsNPC(n->index)) {continue;} ++levelNpcs; if(!(n->entflags&EF_ACTIVE) || (n->health <= 0.0f && n->cyberHealth <= 0.0f)) {continue;} if(e->countOnlySameIndex && n->index!=e->spawnIndex) {continue;} ++count;}
    if (levelNpcs > 300) {return;}/*SpawnManager.Update:48 bails on a crowded level*/
    if (e->numberActive != count) {e->numberActive = count;}
    if (e->numberActive >= e->numberToSpawn) {return;}
    if (e->delayFinished >= World.pauseRelativeTime) {return;}
    e->delayFinished = World.pauseRelativeTime + (double)random_range(e->minDelayBetweenSpawns, e->maxDelayBetweenSpawns);
    if (World.diffCbt == 0) {return;}/*SpawnManager.Spawn():101 - combat 0 spawns nothing*/
    u16 spots=0; for (u16 i=INSTS_1ST_IDX;i<World.instCount;++i) {if(World.instances[i].index==SPAWNPOINT_CONST) {++spots;}}
    if (!spots) {return;}
    /* GetRandomLocation():99-116 takes 10 shots, returns the first clear point once more than 8 have been clear,
       otherwise keeps the last clear one and gives up entirely if none were.  Random.Range(0, Length-1) there is
       inclusive at both ends and so never yields the LAST point; kept for parity, and random_range_u32 is also
       inclusive so the two agree. */
    u16 spot=0,valid=0; for (u8 shot=0;shot<10;++shot){ u16 want=(u16)random_range_u32(0,spots-1),cand=0; for (u16 i=INSTS_1ST_IDX;i<World.instCount;++i){ if(World.instances[i].index!=SPAWNPOINT_CONST) {continue;} if(want--==0){cand=i;break;} } if (!cand||!SpawnPointIsClear(cand)) {continue;} spot=cand; if (++valid>8) break; }
    if (!valid) {return;}/*GetRandomLocation returns null; Spawn():122 logs and bails*/
    u16 npc=SpawnDynamicObject(e->spawnIndex,false); if (!EntIdxIsValid(npc)) {return;}/*SpawnManager.Spawn():119-122 logs and bails*/
    World.position[npc]=World.position[spot]; World.scale[npc]=(V3){1.0f,1.0f,1.0f};
    if (!e->alertEnemiesOnAwake) {flag_set(&World.instances[npc].entflags, EF_WANDERING, true); return;}/*aic.wandering = true, except for index 14 which SpawnManager.Spawn():132 skips; 14 is not an NPC constIndex in Voxen's table, so there is no such exception*/
    AIAlert(npc);/*SetEnemy(player1)*/
    if (count + 1 >= e->numberToSpawn) {e->delayFinished = World.pauseRelativeTime + (double)e->allSpawnedResetDelay;}/*SpawnManager.Update:73-74 holds the wave back once it is full*/
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
/* Unity ButtonSwitch.SetActive plays "Activating"/"Deactivating" on every toggle when animateModel is set
  (ButtonSwitch.cs:117), and each transient clip settles on the matching held pose. Only func_switch4, 5 and 7
  serialize animateModel: 1; of those only switch4.glb/switch5.glb carry an animationNum in Data/models.txt, so
  the clip length comes from the same frames/(framerate*speed) the animator steps by rather than a fixed guess. */
void ButtonSwitchAnimate(Entity* e){
    if (e->animationNum >= MAX_ANIMS) return;
    u8 move = e->active ? A_ACTIVATE : A_DEACTIVATE;
    const AnimationClip* c = &modelAnimationClips[e->animationNum][move];
    if (c->framerate <= 0 || c->speed <= 0 || c->frameEnd < c->frameStart) { ButtonSwitchHoldPose(e); return; }
    ChangeAnim(e, move); e->switchAnimFinished = World.pauseRelativeTime + (double)(c->frameEnd - c->frameStart + 1) / ((double)c->framerate * c->speed);
}
void ButtonSwitchInitAfterLoad(u16 self) { Entity* e=&World.instances[self]; e->delayFinished=0.0f; ButtonSwitchHoldPose(e); if(e->active){e->tickFinished=World.pauseRelativeTime + 1.5 + (double)random_range(0.0f,1.0f);} }
void ButtonSwitchUseTargets(u16 self) { Entity* e=&World.instances[self]; UseTargets(self,e->targetIdx); e->active=!e->active; ButtonSwitchAnimate(e); if(e->index == 689 || e->index == 690 || e->index == 695) { TextureChangerToggle(self); if(e->index == 689 && e->active){e->tickFinished=World.pauseRelativeTime + 1.5f;} } }
static __attribute__((noinline)) void UIBlockedBySecurity(V3 tetherPoint) { (void)tetherPoint; play_wav(sounds[468], AppliedFXVol(0.85f), (V3){0,0,0}, false);/*blocked_by_security*/ MFD_OpenData(World.Sys_UI.lastDataSideRH,8);/*MFDManager.BlockedBySecurity() opens tab 4 on the last-used data side and raises the blocked view there*/ CenterStatusPrint("%s",Sys_Text.stringTable[25]); }
static __attribute__((noinline)) void EntitySetLocked(Entity* e, bool locked) { flag_set(&e->entflags,EF_LOCKED,locked); }
/*Button-switch click SFX.  The success sound is a prefab constant, not level data: no func_switch* prefab serializes
  SFXIndex and no Data/level*.txt line carries one, so every switch in the game fell through to e->SFXIndex == 0 and
  played sounds[0] on every press.  Values read straight off the prefab SoundOnUse components
  (Assets/Resources/Prefabs/func_switch{1,2,3,4,5,7,8}.prefab -> m_Sound.clip guid -> Const.a.sounds).  func_switch6
  (693) is unused by any level.  Same table shape as doorSFXIndex in LoadLevelData, one row per constIndex. */
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
void HealingBedUse(u16 self, u16 owner) { Entity* e=&World.instances[self]; if (GetCurrentLevelSecurity() <= UsableOrDef(e->minSecurityLevel,100.0f)) { if(!e->broken){HealthManagerHealingBed(PLAYER1,UsableOrDef(e->amount,170.0f),true); World.instances[PLAYER1].radiation=0.0f; World.invP1.radiationArea=false; CenterStatusPrint("%s",Sys_Text.stringTable[23],owner); play_wav(sounds[103], AppliedFXVol(1.0f), World.position[self], false);} else {CenterStatusPrint("%s",Sys_Text.stringTable[24],owner);} } else UIBlockedBySecurity(World.position[self]); }
int GeneralInvItem(int slot);
bool GeneralInvCanVaporize(int slot);
void GeneralInvRemove(int slot);
void VaporizeClick() {int slot=World.invP1.generalInvCurrent; if(!GeneralInvCanVaporize(slot))return; GeneralInvRemove(slot); play_wav(sounds[89],AppliedFXVol(1.0f),(V3){0},false);}
typedef struct { i8 norm,alt; } AmmoIconEntry;
static const AmmoIconEntry ammoIconTable[51]={[36-36]={7,8}/*Magnesium/Penetrator*/,[37-36]={-2,-2}/*Energy*/,[38-36]={0,1}/*Needle/Tranq*/,[39-36]={9,10}/*Hornette/Splinter*/,[40-36]={-2,-2}/*Energy*/,[41-36]={-1,-1}/*Rapier, no ammo*/,[42-36]={-1,-1}/*Pipe, no ammo*/,[43-36]={5,6}/*Hollow/Slug*/,[44-36]={11,-1}/*Magcart*/,[45-36]={2,3 }/*Standard/Teflon*/,[46-36]={-2,-2}/*Energy*/,[47-36]={14,-1}/*Rail Rounds*/,[48-36]={4,-1}/*Rubber Slugs*/,[49-36]={12,13}/*Slag/Large Slag*/,[50-36]={-2,-2}/*Energy*/,[51-36]={-2,-2}/*Energy*/};
i8 AmmoIconGet(int index,bool alt) { if (index < 343 || index > 358) {return -1;} const AmmoIconEntry* e = &ammoIconTable[index - 343]; return alt ? e->alt : e->norm; }
/*Menu video text overlay playback (intro/credits) lives in ui.c RenderVideoPage; the old CreditsScroll phase driver below was dead code and is removed.*/

void CyborgConversionToggleTargetted() {bool active=(World.ressurectionActiveLevels>>World.curLev)&1u; flag_setu16(&World.ressurectionActiveLevels,(1u<<World.curLev),!active); if(World.curLev==6)flag_setu16(&World.ressurectionActiveLevels,(1u<<10|1u<<11|1u<<12),!active);/*Set groves 10,11,12 when 6 toggled, shared*/ play_wav(sounds[active ? 183 : 184], AppliedFXVol(1.0f), (V3){0.0f,0.0f,0.0f}, false);/*"vox_cybconvcancelled" : "vox_cybconvenabled"*/ CenterStatusPrint("%s",Sys_Text.stringTable[active ? 591 : 592]);}
bool PlayerInElevatorCell(void); bool FindElevatorKeypadPos(u8 level,V3* outPos);
void ElevatorButtonClick(u16 self) {
    Entity* e = &World.instances[self]; if (World.Sys_UI.linkedElevatorDoor == U16_MAX) { CenterStatusPrint("%s",Sys_Text.stringTable[6]); /*Too far away from that.*/ return; } Entity* door = &World.instances[World.Sys_UI.linkedElevatorDoor]; bool doorClosed = door->doorOpen == DoorState_Closed;
    if (!PlayerInElevatorCell()) { CenterStatusPrint("%s",Sys_Text.stringTable[6]); /*Too far away from that.*/ return; } if (!doorClosed) { CenterStatusPrint("%s",Sys_Text.stringTable[7]); /*Door not closed.*/ return; } if (!(e->entflags & EF_ACTIVE)) { CenterStatusPrint("%s",Sys_Text.stringTable[8]); /*Floor not accessible.*/ return; }
    /*Preserve the player's relative offset from the keypad across the level load.*/
    V3 srcKeypad=World.Sys_UI.objectInUsePos; V3 offset=V3_AsubB(World.position[PLAYER1],srcKeypad);
    u8 destLevel=(u8)e->teleportID; V3 destKeypad; if (FindElevatorKeypadPos(destLevel,&destKeypad)) queuedLevelPos=V3_AplusB(destKeypad,offset); else queuedLevelPos=(e->targetDestinationID != U16_MAX && e->targetDestinationID < World.instCount) ? World.position[e->targetDestinationID] : (V3){0.0f,0.0f,0.0f}; queuedLevelToLoad=destLevel;
}

void EmailTargetted(u16 self) { Entity* e=&World.instances[self]; u16 idx=e->emailIndex; if(idx>=LOGCNT){return;} if(World.invP1.hasLog[idx]){return;} World.invP1.hasLog[idx]=World.invP1.hasNewEmail=true; World.invP1.lastAddedIndex=idx; if(Sys_Text.audioLogType[idx] == AudioLogType_Email){World.invP1.beepDone=true;} if(e->autoPlayEmail){PlayLastAddedLog(idx);} }
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
/*One definition of "this NPC is in the player's PVS", shared by the TargetID acquire paths below and the
  render loop in ui.c so sensing and drawing can never disagree. Plain cell visibility, no neighborhood
  fallback: the marker and its label are an unlit HUD overlay with no depth test, so without this an NPC in a
  room the player can't see still paints a marker over whatever wall is in front of it.*/
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
/*Per 0.1s tick, not per frame. Unity's PlayerEnergy.cs figures are per-frame at 60Hz, so matching its actual per-second rate means x6. [9] is the exception: hand-calibrated so v2 drains the 0..255 bar in 120s.*/ static const float  hwDrain[12][4] = {[3]={0.0921f,0.20478f,0.15354f,0.0f},[5]={0.24576f,0.61434f,0.28333f,0.30714f},[6]={0.010236f,0.0f,0.0f,0.0f},[7]={0.15354f,0.25596f,0.30714f,0.0f},[9]={0.0f,0.28333f,0.2125f,0.0f},[11]={0.51198f,0.51198f,0.51198f,0.51198f},};/*nightsight has no version switch in PlayerEnergy.cs, and hwBtns[].eng==0 means it always drains, so every version is charged*/
static const u16 hwDrainJPM[12][4] = {[3]={9,20,15,0},[5]={24,60,105,30},[6]={1,0,0,0},[7]={15,25,30,0},[9]={0,16,12,0},[11]={50,50,50,50},};/*PlayerEnergy.cs adds 50 with no version switch for nightsight, matching the version-agnostic drain above*/
bool ModRequestsGrayscale() { return ((World.invP1.hasHardware & HW_INF) && (World.invP1.hardwareIsActive & HW_INF) > 0); }
static void DeactivateHardwareOnEnergyDepleted() { World.invP1.hardwareIsActive = 0; }
void TakeEnergy(float take) { if (World.invP1.energy <= 0.0f || Cheats.redbull) {return;} World.invP1.energy -= take; if (World.invP1.energy <= 0.0f) { World.invP1.energy = 0.0f; play_wav(sounds[84],AppliedFXVol(1.0f),(V3){0.0f,0.0f,0.0f},false);/*energy_gone*/ CenterStatusPrint("%s",Sys_Text.stringTable[314]); /*Power supply exhausted.*/ DeactivateHardwareOnEnergyDepleted(); } }
void GiveEnergy(float give,EnergyType type) { World.invP1.energy += give; if (World.invP1.energy > 255.0f) {World.invP1.energy = 255.0f;} if (type == EnergyType_Battery){play_wav(sounds[79],AppliedFXVol(1.0f),(V3){0.0f,0.0f,0.0f},false);/*batteryuse*/} else if (type == EnergyType_ChargeStation){play_wav(sounds[100],AppliedFXVol(1.0f),(V3){0.0f,0.0f,0.0f},false);/*chargingstation*/} }
/*Shared by the booster drain and the footstep/rustle noise checks so "player is moving" means one thing*/
bool PlayerIsMoving() { return V3_dot(World.velocity[PLAYER1],World.velocity[PLAYER1]) > 0.1f; }
void PlayerEnergyInit() { World.invP1.energy = 54.0f; World.invP1.energyDrainTickFinished = World.pauseRelativeTime + 0.1 + random_range(0.0f,1.0f); World.invP1.drainJPM = 0; }
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
    /* Unity's general inventory is a list, so removing an item closes the rows up rather than leaving a hole: the MFD draws generalRowY[slot%7] for a populated slot, so a gap at 3 blanks left-column row 3 and a gap at 10 blanks right-column row 3. Slot 0 is the reserved access-card reader row and is never removable, so the shift starts at 1. */
    for (int i=slot;i<13;++i) { World.invP1.generalInventoryIndexRef[i]=World.invP1.generalInventoryIndexRef[i+1]; World.invP1.generalInvCustIdx[i]=World.invP1.generalInvCustIdx[i+1]; }
    World.invP1.generalInventoryIndexRef[13]=-1; World.invP1.generalInvCustIdx[13]=U16_MAX; MFD_GeneralChanged();
    /* The cursor indexes a slot, so it slides with its item. When the selected row itself goes, whatever slid into it takes the cursor; if nothing slid in, fall back to the last valid item (Unity: VaporizeButton scans 13 down to 0). */
    if (World.invP1.generalInvCurrent==slot) { if (GeneralInvItem(slot)<0) { World.invP1.generalInvCurrent=0; for (int i=13;i>=0;--i) { if (GeneralInvItem(i)>=0) { World.invP1.generalInvCurrent=(u8)i; break; } } } }
    else if (World.invP1.generalInvCurrent>slot) World.invP1.generalInvCurrent--;
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

/*Unity ObjectImpact.cs parity: impact sound with volume modulated by impact velocity.
  Threshold vel>2 (minVolumeSpeed), vol=(vel/10)*0.3 (maxVolumeSpeed), sound 523 (physics/impact_lightweight).
  Pitch is a +- semitone shift (Unity: random 0.8-1.2x multiplier), applied dynamically during mixing.*/
void ApplyImpactForceWithSound(u16 target, float vel, V3 normal, V3 pt) {
    ApplyImpactForce(target, vel, normal, pt);
    if (vel > 2.0f) play_wav_ext(sounds[523], AppliedFXVol((vel / 10.0f) * 0.3f), pt, true, random_range(-3.8631f,3.1564f));/*Unity ObjectImpact pitch Random.Range(0.8,1.2) playback ratio = 12*log2(r) semitones*/
}

/* Utils.ApplyImpactForceSphere (Utils.cs:1675-1734). The C# tests the colliders a Physics.OverlapSphere returns,
   so the player (slot 1) is in the list and takes half damage; the loop here used to start at INSTS_1ST_IDX (2) and
   so the player never took splash damage at all. Damage falloff is linear over the radius, but floored at 33% of the
   *un-halved* incoming damage (Utils.cs:1704-1713) -- the halve is applied before the floor and the floor is
   measured from the original, so the player floor is still 33%, not 16.5%. Past 4 units the C# requires line of
   sight through layerMaskExplosion before the blast connects (Utils.cs:1692-1698); inside 4 units it always hits.
   Blast impulse is baseVel scaled by that same falloff exactly once, and is not derived from the scaled splash
   damage, so bodies with and without a HealthManager get the same push. The C# also halves the impulse on Unity
   layer 10, which is NPC (Utils.cs:1721), and caps the velocity change at (0.5+5*mass)*min(damage/100,2) (1727-1729). */
void ApplyImpactForceSphere(DamageData* dd, V3 center, float radius, float baseVel) { 
    if (radius <= 0.0f || baseVel <= 0.0f) return; float r2 = radius * radius; float origDamage = dd ? dd->damage : 0.0f;
    float damageScale = vmin(origDamage / 100.0f, 2.0f);/*measured off the original damage, so it is the same for every body in the blast*/
    for (u16 i = PLAYER1; i < World.instCount; i++) {
        Entity* e = &World.instances[i]; if (!(e->entflags & EF_ACTIVE) || (e->entflags & EF_DEAD)) { if (!IdxIsNPC(e->index) || !(World.layer[i] & L_Corpse)) continue;/*Corpses are normally skipped here, but a dead NPC on the corpse layer stays in the blast so grenades can vaporize it (Death() re-entry).*/ } if (!(e->entflags & EF_RIGIDBODY) && !IdxIsNPC(e->index) && i != PLAYER1 && !IsDamageable(e)) continue;/*IsDamageable() widens the blast to static damageable props (sec_cpunode 478/479, consoles, cameras) which have health but no EF_RIGIDBODY, so grenades used to skip them entirely.  ApplyImpactForce below still no-ops for them, so only the damage applies.*/ float sqd = V3_SqDist(World.position[i], center); if (sqd > r2) continue; float dist = vsqrtf(sqd);
        if (dist >= 4.0f) { RaycastHit sight = Raycast(center, V3_ScaleByF(V3_AsubB(World.position[i],center), 1.0f / dist), radius + 0.02f, LMASK_EXPLOSION); if (!(sight.hit && sight.hitInstanceIndex == i)) continue; }
        float distPenalty = (radius - dist) / radius; if (distPenalty < 0.0f) distPenalty = 0.0f;
        V3 normal; if(dist > 0.0001f){normal=V3_ScaleByF(V3_AsubB(World.position[i],center), 1.0f / dist);}else{normal = (V3){0.0f,1.0f,0.0f};}
        float impactVel = baseVel * distPenalty;
        if (dd && origDamage > 0.0f && i != dd->owner) { DamageData splash=*dd; if (i == PLAYER1) splash.damage *= 0.5f; /*halve for player*/ splash.damage *= distPenalty; float saturation = origDamage * 0.33f; if (splash.damage < saturation) splash.damage = saturation;
            splash.impactVelocity = impactVel; splash.hitIdx = i; splash.hitpoint=World.position[i]; splash.attacknormal=normal; TakeDamage(i,splash); }
        if (World.layer[i] & L_NPC) impactVel *= 0.5f;
        ApplyImpactForce(i,vmin(impactVel,(0.5f + 5.0f * World.mass[i]) * damageScale),normal,World.position[i]);
    }
}

static void SpawnExplosionPreset(u16 type,V3 pos) { const PSysDef* preset=PSysTypeGet(type); if(!preset)return; PSysDef def=*preset; def.pos=pos; PSysAdd(&def); }
void SpawnExplosionEffect(V3 pos, int explosionType) {
    /* Unity's GrenadeActivate pulls the grenade's explosionType pool object and enables it in place, so each
       type plays the emitters of that pool: frag/concussion/earth/nitro/mine share GrenadeFragExplosions
       (ef_fragexplosion), EMP uses GrenadeEMPExplosions (ef_empexplosion) and gas uses GasExplosions. */
    switch (explosionType) {
        case 3: SpawnExplosionPreset(PSYS_gasExplosions,pos); SpawnExplosionPreset(PSYS_gasSmoke,pos); break;/*GasExplosions*/
        case 4: SpawnExplosionPreset(PSYS_ef_empexplosion,pos); SpawnExplosionPreset(PSYS_sprinkles,pos); SpawnExplosionPreset(PSYS_sparkles,pos); break;/*GrenadeEMPExplosions*/
        default: SpawnExplosionPreset(PSYS_CentralFireball,pos); SpawnExplosionPreset(PSYS_FireSpits,pos); break;/*GrenadeFragExplosions*/
    }
}
void GrenadeExplode(u16 self) {
    if(self>=World.instCount)return; Entity* e = &World.instances[self]; if(!(e->entflags&EF_ACTIVE))return; flag_set(&e->entflags,EF_ACTIVE,false);
    DamageData dd={.damage=e->damage,.penetration=e->strength,.offense=e->speed,.armorvalue=0.0f,.defense=0.0f,.impactVelocity=e->damage*1.5f,.attacknormal=(V3){0.0f,1.0f,0.0f},.hitpoint=World.position[self],.attackType=e->attackType,.owner=e->recentMostActivator,.hitIdx=WORLD,.isOtherNPC=false,.berserkActive=(World.invP1.patchActive & PATCH_BERSERK) != 0};
    i16 idx=GrenadeTypeFromConst(e->index); float radius=(idx>=7&&idx<=13) ? grenadeRadius[idx-7] : (e->strength>0.0f ? e->strength : 4.0f); ApplyImpactForceSphere(&dd,World.position[self],radius,e->damage * 0.1f); /*GrenadeActivate.cs:106 passes impactScale 1.0f, and Utils.cs:1683 makes impactVelocity damage*impactScale*0.1, so the blast impulse is damage*0.1 before falloff*/ if (!GrenadeIsNPCMine(self)) { World.invP1.makingNoise = true; World.invP1.noiseFinished = World.pauseRelativeTime + 2.0; } int soundIndex=60,explosionType=2;
    switch (idx) {case 7: case 11: soundIndex = 64; World.fogFac += 5; explosionType = 1; break;/*frag, mine*/ case 8: case 10: soundIndex = 60; World.fogFac += 7; explosionType = 2; break;/*conc, earth*/ case 9:  soundIndex = 67; explosionType = 4; break;/*emp*/ case 12: soundIndex = 60; World.fogFac += 6;  explosionType = 2; break;/*nitro*/ case 13: soundIndex = 63; World.fogFac += 10; explosionType = 3; break;/*gas*/}
    play_wav(SoundPath(soundIndex), AppliedFXVol(1.0f), World.position[self], true); SpawnExplosionEffect(World.position[self],explosionType); Shake(-1.0f); DeleteInstance(self);
}

void GrenadeActivate(u16 self) {
    if(self>=World.instCount)return; Entity* e=&World.instances[self]; i16 idx=GrenadeTypeFromConst(e->index);
    /* Called from DropHeldItem after the throw impulse is assigned, never when picked up. */
    if(idx==10){World.invP1.earthShakerTimeSetting=vclamp((float)World.invP1.earthShakerTimeSetting,4.0f,60.0f);e->timerFinished=World.pauseRelativeTime+World.invP1.earthShakerTimeSetting;}
    else if(idx==12){World.invP1.nitroTimeSetting=vclamp((float)World.invP1.nitroTimeSetting,2.0f,60.0f);e->timerFinished=World.pauseRelativeTime+World.invP1.nitroTimeSetting;}
}
void GrenadeUpdate(u16 self) { Entity* e = &World.instances[self]; i16 idx=GrenadeTypeFromConst(e->index); if(idx == 14){GrenadeExplode(self); return;} /*Plastique*/ if((idx == 10 || idx == 12) && e->timerFinished <= World.pauseRelativeTime) { GrenadeExplode(self); return; } if (idx == 11) { V3 origin = World.position[self]; float pr=1.451f; /*weapon_grenademine_live ProxCollision m_Radius, not blast radius.  Unity arms on OnTriggerEnter, so the sensing body's own collider radius adds to the trigger sphere (GrenadeProximity.cs); colliderSize.x is that radius for COLTYPE_CAP/COLTYPE_SPW, and mesh-collider bodies leave it at 0 so they test at the bare trigger radius.*/ bool npcMine = GrenadeIsNPCMine(self); /*Deliberate divergence from Unity GrenadeProximity, which prox-senses Player and NPC alike.  An NPC mine arms on the player; a player's own mine arms on NPCs.*/ for (u16 i = PLAYER1; i < World.instCount; i++) { Entity* o = &World.instances[i]; if (i == self || !(o->entflags & EF_ACTIVE) || (o->entflags & EF_DEAD)) continue; if (npcMine ? (i != PLAYER1) : (i == PLAYER1 || !IdxIsNPC(o->index))) continue; float rr = pr + ((World.col[i] == COLTYPE_CAP || World.col[i] == COLTYPE_SPH) ? World.colliderSize[i].x : 0.0f); if (V3_SqDist(World.position[i], origin) < (rr * rr)) { GrenadeExplode(self); return; } } } }
void GrenadeOnCollision(u16 self) { i16 idx=GrenadeTypeFromConst(World.instances[self].index); if ((idx >= 7 && idx <= 9) || idx == 13) GrenadeExplode(self); }
float GetDamageTakeAmount(DamageData* dd) { if (!dd) return 0.0f; float take = dd->damage; if (take <= 0.0f) return 0.0f; if (dd->berserkActive) take *= BERSERK_DAMAGE_MULTIPLIER; if (dd->defense > 0.0f && dd->offense < dd->defense) { float r = (dd->defense - dd->offense) / dd->defense; if (r > 0.85f) r = 0.85f; take *= (1.0f - r); } if (dd->armorvalue > 0.0f && dd->penetration < dd->armorvalue) { float a = (dd->armorvalue - dd->penetration) / dd->armorvalue; if (a > 0.85f) a = 0.85f; take *= (1.0f - a); } if (take < 0.0f) take = 0.0f; return take; }
void SpawnImpactEffect(u16 impactType, V3 pos) { if (impactType == 0 || impactType == U16_MAX) return; u16 fx = SpawnDynamicObject(impactType, false); if (!EntIdxIsValid(fx)) return; World.position[fx] = pos; Entity* e = &World.instances[fx]; flag_set(&e->entflags, EF_ACTIVE, true); if (e->itemLifeTime <= 0.0f) e->itemLifeTime = 1.0f; e->delayFinished = World.pauseRelativeTime + e->itemLifeTime; }
void ExitCyberspace(void) { UIExitCyberspace(); if (World.curLev != LEVEL_CYBERSPACE) return; if (World.instances[PLAYER1].cyberHealth <= 0.0f) World.instances[PLAYER1].cyberHealth = 1.0f; LoadLevel(World.startLevel < World.numLevels ? World.startLevel : 0, (V3){0.0f,0.0f,0.0f}); }
void ReduceCurrentLevelSecurity(SecurityType stype) { // cam 4, small node 10, large node 27 -- a typical 4-node/20-camera level then puts cameras near 2% and nodes near 10-13%
    static const u32 score[3]={4u,10u,27u}; u8 lev = World.curLev; if (lev >= MAX_LEVELS || stype == SecurityType_None) return;
    u8* tot[3]={&World.levelCameraCount[lev],&World.levelSmallNodeCount[lev],&World.levelLargeNodeCount[lev]};
    u8* dcd[3]={&World.levCamDestroyedCnt[lev],&World.levSmNodeDestroyedCnt[lev],&World.levNodeDestroyedCnt[lev]};
    u32 t=(u32)stype-1u; if (*dcd[t] < 255u) (*dcd[t])++;
    u32 total=0u, dead=0u; for (u32 i=0u;i<3u;++i) { total += (u32)*tot[i]*score[i]; dead += (u32)*dcd[i]*score[i]; }
    if (!total) return;
    u8 before = World.levelSecurity[lev], after;
    //  ceiling, so any surviving weight keeps the level above 0 and only killing every security object reaches it
    after = dead >= total ? 0u : (u8)(((total-dead)*100u + total-1u) / total); if (after > 100u) after = 100u;
    if (after >= before && before) after = before > 1u ? (u8)(before-1u) : before; // a kill must never read as 58 -> 58
    World.levelSecurity[lev] = after;
    CenterStatusPrint("%s%d%s", Sys_Text.stringTable[306], (int)World.levelSecurity[lev], Sys_Text.stringTable[307]);
}
/*Security-code displays.  Unity CodeScreen.Update re-assigns Const.a.screenCodes[matIndex] every 0.3s, and
  Texture slots 768..777 are Textures/screencode0..9.png, the same range EPerms[551].texIndex (768) already sits in.
  A code screen starts unlocked and flickers a random digit: Unity CodeScreen.Update re-picks
  Const.a.screenCodes[Random.Range(0,10)] every 0.3s while Const.a.questData.levNSecCodeLocked is false, and
  this level file flags the screens (codeScreen:1, Entity.codeScreen) so entity.c can put them on the
  ScreenCodeRandom clip with texAnimRandom.  This function is the lock: Unity's Const.LockCPUScreenCode, driven by
  TargetIO.lockCodeToScreenMaterialChanger off the CPU node's UseTargets, freezes levNSecCode and pins the material
  to the real digit once that level's last node falls.  It is called from ObjectDeath only when
  CPUNodesRemainOnLevel() is false, so the code stays guessable until the level is actually cleared.  It walks every
  loaded level, not just the current one, because a display is addressed by instance index and UseTargets already
  treats cross-level targets as ordinary.*/
static void CodeScreensSetForLevel(u8 lev) {
    if (lev < 1 || lev > 6) return;
    /*The level's code is drawn here, the moment its last node dies -- Unity's Const.LockCPUScreenCode, which
      re-rolls once and marks the code locked.  Drawing it at lock rather than at NewGame is what makes the
      self-destruct pads' 289/290 guard reachable: no nodes cleared, no code.*/
    i8 *code = &World.lev1SecCode; if (code[lev-1] < 0) { code[lev-1] = (i8)random_range_u8(0u,9u); }
    u8 digit = (u8)code[lev-1];
    u8 entryLevel = World.currentLevel;
    for (u8 l = 0; l < World.numLevels; ++l) {
        if (l != World.currentLevel) SetLevelPointers(l);
        for (u16 i = INSTS_1ST_IDX; i < World.instCount; ++i) { Entity* s = &World.instances[i]; if (s->index != 551 || !s->codeScreen) continue; CodeScreenShowDigit(i, digit); }
    }
    if (World.currentLevel != entryLevel) SetLevelPointers(entryLevel);
}
/* Any sec_cpunode/sec_cpunode_small still standing on the level that World.currentLevel points at.  ObjectDeath
   sets EF_DEAD_CHECKS_DONE before it calls this, so the node that just fell never counts itself.  A plain scan of
   the current level needs no SetLevelPointers dance and no per-level node tally, and it stays correct for runtime
   spawned and deleted nodes alike. */
static bool CPUNodesRemainOnLevel(void) {
    for (u16 i = INSTS_1ST_IDX; i < World.instCount; ++i) { const Entity* n = &World.instances[i]; if ((n->index == 478 || n->index == 479) && !(n->entflags & EF_DEAD_CHECKS_DONE)) return true; }
    return false;
}
void ProjectileEffectImpactOnCollision(u16 self,u16 hitIdx, V3 hitPos,V3 hitNormal) {
    if(self>=World.instCount||hitIdx>=World.instCount)return; Entity* e = &World.instances[self]; if (hitIdx == e->recentMostActivator) return;/*other.gameObject == host*/ e->counter++;/*numHits++*/
    DamageData dd={.damage=e->damage,.penetration=e->strength,.offense=e->speed,.armorvalue=0.,.defense=0,.impactVelocity=e->damage*1.5f,.attacknormal=hitNormal,.hitpoint=hitPos,.attackType=e->attackType,.owner=e->recentMostActivator,.hitIdx=hitIdx,.isOtherNPC=IdxIsNPC(World.instances[hitIdx].index),.berserkActive=(e->recentMostActivator==PLAYER1&&(World.invP1.patchActive & PATCH_BERSERK)!=0)};
    dd.damage = GetDamageTakeAmount(&dd);
    Entity* hit = &World.instances[hitIdx]; if (IdxIsNPC(hit->index)) { NPCTable* nt = &npcTable[hit->index - 419]; dd.armorvalue = nt->armorvalue; dd.defense = nt->defense; } if (e->lookUpIndex == 5) { ApplyImpactForceSphere(&dd, World.position[self], 3.2f, 1.0f); World.fogFac += 4; }/*Railgun sphere impact*/
    /* Impact effect on every contact, ricochets included.  Unity gates the first spawn on
       Utils.GetMainHealthManager(hitGO) != null and only plays the pooled effect unconditionally once
       numHits >= hitCountBeforeRemoval, so against bare level geometry -- which carries no HealthManager -- a
       plasma bolt (hitCountBeforeRemoval 5) ricocheted four times in silence and only sparked on the fifth and
       final contact.  Deciding this per contact rather than per collider is what makes the bolt leave a fresh
       plasmahit/maghit sprite at every wall and floor bounce.  Damage stays gated separately, below. */
    SpawnProjectileImpactParticles(e->index,hitPos,hitNormal);
    bool hostIsNPC=e->recentMostActivator<World.instCount&&IdxIsNPC(World.instances[e->recentMostActivator].index);
    if (hit->health > 0.0f || hit->cyberHealth > 0.0f) {
        if (e->counter < e->countToTrigger) dd.damage *= 0.85f;/*per-hit falloff*/ dd.impactVelocity = dd.damage * 1.5f; if (e->counter > 0) dd.impactVelocity /= 3.0f; float dmgFinal = TakeDamage(hitIdx,dd); float tranq=-1.0f;
        if (dd.isOtherNPC) { if(!(hit->entflags & EF_ASLEEP)){World.Sys_Music.inCombat=true;} if(dd.attackType == Att_Trnq){float stunAmount=vclamp(3.0f+(World.invP1.stungunSetting/100.0f)*7.0f,3.0f,10.0f); tranq=Tranquilize(hitIdx,stunAmount,true);} } if (dmgFinal < 0.0f) {dmgFinal = 0.0f;} CreateTargetIDInstance(dmgFinal,hitIdx,tranq);
    }
    if (World.curLev != LEVEL_CYBERSPACE && !hostIsNPC) { ApplyImpactForceWithSound(hitIdx,dd.impactVelocity,dd.attacknormal,hitPos); }/*impact force+sound for any dynamic object (Unity: Utils.ApplyImpactForce + ObjectImpact)*/
    if(e->countToTrigger<1)e->countToTrigger=1; if (e->counter >= e->countToTrigger) { if (e->despawnInstead){DeleteInstance(self);}else{flag_set(&e->entflags,EF_ACTIVE,false);} }
}
void ProjectileEffectImpactInitAfterLoad(u16 self) { if(self>=World.instCount)return;Entity* e=&World.instances[self]; e->counter=0;e->currentTargetIdx=0;e->lookUpIndex=(e->index==484||e->index==491)?5:0;if(e->countToTrigger<1){e->countToTrigger=e->index==485?5:(e->index==495?2:1);} }
 // None  Melee  MelEn  EnBm   Mag    Proj   Needle ProjEB ProjLn Gas    Tranq  Drill
static const float attackTypeMult[7][12]={[NPCType_Mutant]={1,1,1,1,0,1,2,1,1,2,1,1},[NPCType_Supermutant]={1,1,1,1,0,1,1,1,1,1.5,1,1},[NPCType_Robot]={1,1,1,1,4,1,0,1,1,0,1,1},[NPCType_Cyborg]={1,1,1,1,2,1,1,1,1,1,1,1},[NPCType_Supercyborg]={1,1,1,1,2,1,0,1,1,0,1,1},[NPCType_MutantCyborg]={1,1,1,1,0.5,1,2,1,1,2,1.5,1},[NPCType_Cyber]={1,1,1,1,1,1,1,1,1,1,1,0}}; // Attack type damage multiplier table [NPCType][AttType], 1.0f = no change, 0.0f = immune, other = multiplier
static const i16 objectDeathSound[] = {[458]=63,[459]=66,[460]=66,[464]=62,[465]=532,[466]=532,[467]=532,[468]=532,[469]=532,[470]=532,[471]=532,[472]=62,[473]=62,[474]=62,[475]=62,[476]=62,[477]=61,[478]=65,[479]=69,[525]=68,[526]=68,};
static bool IsCyberEntity(u16 self) { if (World.curLev == LEVEL_CYBERSPACE){return true;} Entity* e=&World.instances[self]; if (self != PLAYER1 && e->cyberHealth > 0.0f){return true;} return (IdxIsNPC(e->index) && (e->index - 419) > 23);/*24-28 are cyber enemies*/}
static float ApplyAttTypeAdjustments(u16 self,float take,AttType at) { if (!IdxIsNPC(World.instances[self].index) || World.instances[self].health <= 0.0f){return take;} NPCType t = npcTable[World.instances[self].index - 419].type; if (at >= 12){return take;} return take * attackTypeMult[t][at]; }
static void UseDeathTargets(u16 self) { if(self == PLAYER1){return;}/*Unity HealthManager.UseDeathTargets: 'if (isPlayer) return.  Player death does nothing.'*/ if (World.instances[self].targetOnDeathIdx != IO_NONE) UseTargets(self,World.instances[self].targetOnDeathIdx); }
static void TeleportAway(u16 self) { 
    if (World.instances[self].entflags & EF_TELEPORT_ON_DEATH) {return;} flag_set(&World.instances[self].entflags,EF_TELEPORT_ON_DEATH,true);
    /*Unity HealthManager.TeleportAway (HealthManager.cs:536) does Utils.Activate(teleportEffect) and only
     * afterwards disables collision and deactivates visibleMeshEntity.  Order matters here for a second
     * reason: the burst has to be created while modelIndex is still live, because PSysAddEx snapshots the
     * model so the emitter can go on spawning from the last visible frame after the body is hidden.*/
    const PSysDef* dgo = PSysTypeGet(PSYS_ef_diego_teleport); if (dgo) { PSysDef d = *dgo; d.pos = World.position[self]; d.rotation = World.rotation[self]; PSysAddEx(&d, self); }
    World.col[self] = COLTYPE_NONE; World.gravity[self] = 0.0f; World.velocity[self] = (V3){0,0,0}; World.angularVelocity[self] = (V3){0,0,0}; World.instances[self].modelIndex = U16_MAX; 
    play_wav(sounds[106]/*misc/teleport*/, AppliedFXVol(1.0f), World.position[self], false);
}

static void DropSearchables(u16 self) {for(int i=0;i<4;i++){if(World.instances[self].contents[i]<=-1){continue;} u16 spawned=SpawnDynamicObject(World.instances[self].contents[i]+307,true); if(EntIdxIsValid(spawned)){World.position[spawned]=World.position[self]; World.instances[spawned].custIdx[0]=World.instances[self].custIdx[i];}else{CenterStatusPrint("BUG: Failed to make search obj.");} World.instances[self].contents[i]=World.instances[self].custIdx[i]=-1;}}
static void CreateDeathEffects(u16 self,u16 fxPoolType) { if (fxPoolType == 0) {return; /*PoolType_None*/} if (!IdxInBounds((int)fxPoolType) || IdxIsGeometry((int)fxPoolType)) {return; /*PoolType is an ordinal enum (None..LeafBurst, 0..29), not a const index.  Feeding one to SpawnDynamicObject makes it try to spawn a level chunk, which is where "Indices 0 to 306 (level chunks) not possible when not on edit mode!" came from.  Refuse here so no caller can reach that.*/} V3 pos = World.position[self]; if (World.col[self] != COLTYPE_NONE) { pos = V3_AplusB(pos,World.colliderCenter[self]); } SpawnImpactEffect(fxPoolType, pos); }
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
    if (World.instances[self].entflags & EF_DEATH_BURST_DONE) { CreateDeathEffects(self,World.instances[self].deathBurst); DropSearchables(self); if (World.instances[self].index != 279){World.col[self]=COLTYPE_NONE;} HideSelf(self); } else { World.col[self] = COLTYPE_NONE; DropSearchables(self); CreateDeathEffects(self,World.instances[self].deathBurst); }
    flag_set(&World.instances[self].entflags,EF_DEAD_CHECKS_DONE,true); World.instances[self].automapHidden = true;
    { SecurityType stype = SecurityType_None; if(World.instances[self].index == 477){stype=SecurityType_Camera;}else if(World.instances[self].index == 479){stype=SecurityType_NodeSmall;} else if(World.instances[self].index == 478){stype=SecurityType_NodeLarge;} if(stype != SecurityType_None){ReduceCurrentLevelSecurity(stype);} }
    if ((World.instances[self].index == 478 || World.instances[self].index == 479) && !CPUNodesRemainOnLevel()) CodeScreensSetForLevel(World.curLev);/*sec_cpunode/sec_cpunode_small: the last node on the level reveals its security code on the displays and stops the flicker.  Independent of the level security percentage above -- revealing the code is a node-behaviour rule, not a percentage one.*/
    u16 idx = World.instances[self].index; SpawnSecCpuNodeGibs(self); play_wav(SoundPath((idx < 527 && objectDeathSound[idx] != 0) ? objectDeathSound[idx] : 62/*crate_break*/), AppliedFXVol(1.0f), World.position[self], true); if(e->deathBurst != 0){HideSelf(self);}
}

static void ScreenDeath(u16 self) { Entity* e=&World.instances[self]; if(e->entflags & EF_DEAD_CHECKS_DONE){return;} flag_set(&e->entflags,EF_DEAD_CHECKS_DONE,true); play_wav(sounds[69], AppliedFXVol(1.0f), World.position[self], true);/*screen_destroy*/ if (e->entflags & EF_DEATH_BURST_DONE) ObjectDeath(self);/*gib path*/ }
static inline bool IsGrenade(u16 i) { return ((i >= 314 && i <= 320) || i == 370 || i == 372 || i == 387 || i == 389 || (i >= 402 && i <= 404)); }
/* HealthManager.cs:492 -- if (vaporizeCorpse && !isSecCamera && !isGrenade) VaporizeCorpse(energyVaporized).  On the 29 NPC prefabs
   that flag is 1 on exactly the 15 that declare a searchCollider (all of them) and 0 on the other 14, so AUTOBOMB, the 8 gib types
   and the 5 cyber types are all refused.  Non-NPCs keep the HealthManager field default (true), which is the behaviour the Death()
   dynamic-object path already had. */
static bool CanVaporize(u16 self) { Entity* e=&World.instances[self]; if (e->index == 477/*sec_camera*/ || IsGrenade(e->index)) {return false;} if (IdxIsNPC(e->index)) {return ai_has_search_collider((u16)(e->index - 419));} return true; }
static void SpawnDeathEffectAt(u16 type, V3 pos) { const PSysDef* preset = PSysTypeGet(type); if (!preset) return; PSysDef def = *preset; def.pos = pos; PSysAdd(&def); }
static void VaporizeCorpse(u16 self,bool energyVaporized) { Entity* e=&World.instances[self]; if (!CanVaporize(self)) {return;}
    /* Always drop first, whatever happens to the visual: Unity's VaporizeCorpse calls searchObject.SpawnContents() before it tears
       the body down, and a corpse whose contents were already taken is a no-op rather than an error. */
    DropSearchables(self);
    /* Unity HealthManager.VaporizeCorpse:508-509 -- deathFX defaults to PoolType.CorpseHit, overridden to PoolType.Vaporize when
       energyVaporized.  An authored deathFX (Voxen's deathBurst, a real const index such as 725 for sec_camera) still wins, matching
       "if deathFX == None -> CorpseHit".  Spawn before DeleteInstance so the effect reads a live instance. */
    V3 pos = World.position[self]; if (World.col[self] != COLTYPE_NONE) { pos = V3_AplusB(pos,World.colliderCenter[self]); }
    if (e->deathBurst != 0 && IdxInBounds((int)e->deathBurst) && !IdxIsGeometry((int)e->deathBurst)) { CreateDeathEffects(self,e->deathBurst); }
    else { SpawnDeathEffectAt(energyVaporized ? PSYS_ef_vaporize_puff : PSYS_ef_corpsehit_puff, pos); }
    e->modelIndex=MAX_MDLS; if (IdxIsNPC(e->index) || IdxIsSearchable(e->index)) DeleteInstance(self); }
static void Death(u16 self,bool energyVaporized) {
    Entity* e = &World.instances[self];
    if (e->entflags & EF_DEAD_CHECKS_DONE) {
        /* Unity guards Death() with deathDone, so a second hit on a body is inert there.  Voxen wants corpses destructible, so a
           repeat hit landing on an NPC corpse that still exists vaporizes it -- Death() below must not replay, or UseDeathTargets
           and the gib spawn would fire twice.  Energy-beam shots (and grenade splash) go through here. */
        if (!IdxIsNPC(e->index) || !(World.layer[self] & L_Corpse) || self == PLAYER1 || IsGrenade(e->index) || (e->entflags & (EF_TELEPORT_ON_DEATH | EF_ACT_AS_CORPSE_ONLY))) return;
        VaporizeCorpse(self, energyVaporized);
        return;
    }
    UseDeathTargets(self); bool isNPC = IdxIsNPC(e->index); bool isObj = IdxIsDynamicObject(e->index); if (e->entflags & EF_ACT_AS_CORPSE_ONLY) { e->entflags |= EF_DEAD_CHECKS_DONE; return; }
    if (e->index == 477) { /* sec_camera has no dynamic-object path; deathFX 1 is CameraExplosions. */
        e->deathBurst = 725; ObjectDeath(self); DeleteInstance(self); return;
    }
    if (e->index == 478 || e->index == 479) { /* sec_cpunode(_small): ObjectDeath spawns gibs 840..853, drops the level security and latches the code screen, but the
        prefab serializes no deathFX, so deathBurst stays 0 and ObjectDeath's trailing HideSelf never runs -- the node would hang in the air as an
        unpickable shell.  Unity destroys the object outright.  DeleteInstance also stops a dead node still counting in CPUNodesRemainOnLevel. */
        ObjectDeath(self); DeleteInstance(self); return;
    }
    /* NPCs retain their entity mesh for AI death animation. Only non-NPC corpses vaporize. */
    bool vaporize=IdxIsCorpse(e->index); bool isGrenade=IsGrenade(e->index), doTeleport=(e->entflags & EF_TELEPORT_ON_DEATH) != 0; if (e->iceActive) World.col[self] = COLTYPE_NONE;
    if (vaporize && e->index != 477/*sec_camera*/ && !isGrenade) VaporizeCorpse(self,energyVaporized); else if (isObj && !isNPC) ObjectDeath(self); else if (e->index == 279/*screen*/) ScreenDeath(self); else if (doTeleport) TeleportAway(self); else if (isGrenade) GrenadeExplode(self);
    if (isNPC && !doTeleport) NPCDeath(self); else if (self == PLAYER1) { if (!RessurectPlayer()) World.deaths++; } flag_set(&e->entflags,EF_DEAD_CHECKS_DONE,true);
}

float TakeDamage(u16 self,DamageData dd) {
    if (Cheats.god && self == PLAYER1) return 0.0f;
    if (self < World.instCount && World.instances[self].index == CYBER_DECOY_CONST) return 0.0f;/*the decoy projection absorbs nothing: its prefab carries no HealthManager, and without this the health-0 fallthrough below would run Death() on the very first cyber shot and delete the decoy*/
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
    else { if(selfIdx == 477/*Camera constIndex 477 gets one-shot by tranq*/ && dd.attackType == Att_Trnq){take=World.instances[self].health + 1.0f;} take=ApplyAttTypeAdjustments(self,take,dd.attackType); if(!isPlayer)DualLog("TakeDamage inst %u ci %u hp %.3f->%.3f take %.3f att %d owner %u\n",(u32)self,(u32)selfIdx,World.instances[self].health,World.instances[self].health-take,take,(int)dd.attackType,(u32)dd.owner); World.instances[self].health-=take; if (isPlayer) { World.damageReceived+=take; World.Sys_Music.inCombat=true; } if (dd.owner == PLAYER1){World.damageDealt+=take;} }
    if (isNPC && (World.instances[self].health > 0.0f || (isCyber && World.instances[self].cyberHealth > 0.0f))) { if (npcTable[selfIdx - 419].timeBetweenPain > 0.0f) flag_set(&World.instances[self].entflags,EF_GO_INTO_PAIN,true); World.instances[self].recentMostActivator = dd.owner; TargetIDSendDamageReceive(self,take,dd.attackType); AICheckPain(self); }
    /* EnergyBeam and ProjectileEnergyBeam both reach HealthManager.Death as energyVaporized and take the corpse out at once, so they
       must not be gated on the corpse's remaining 50 health.  Checked before the <= 0 dispatch below because a beam landing on a
       barely-hurt corpse leaves it comfortably positive and would otherwise only chip it.  Everything else still wittles the corpse
       down hit by hit, which is the intended divergence from Unity's inert deathDone guard. */
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
static void DoorOpen(u16 self) { Entity* e = &World.instances[self]; ChangeAnim(e,A_OPENING); e->doorOpen = e->doorState = DoorState_Opening; e->waitBeforeClose = World.pauseRelativeTime + e->delay; if (e->SFXIndex > 0 && e->SFXIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXIndex], AppliedFXVol(1.0f), World.position[self], true); }
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
        /* Unity keeps normalizedTime on the transient clip, so reversing mid-swing resumes the opposite clip at
           1 - t. Voxen keeps the raw frame, so recover t from the source clip and map it onto the destination's
           range, which is a different length on most doors (e.g. doorD opens over 42 frames and closes over 50).
           Clamp into the source range first: UpdateAnims advances with a modulo, so a long tick can leave frame at
           or past frameEnd, and a frame left over from the other clip would saturate the ratio to 1.0 and snap
           the door to the start of its new clip. With the clamp, 0% is only reachable at a real endpoint. */
        u8 src = op ? A_OPENING : A_CLOSING, dst = op ? A_CLOSING : A_OPENING;
        AnimationClip srcClip = DoorGetClip(e,src), dstClip = DoorGetClip(e,dst);
        u32 f = e->frame; if (f < srcClip.frameStart) f = srcClip.frameStart; if (f > srcClip.frameEnd) f = srcClip.frameEnd;
        float p = (srcClip.frameEnd > srcClip.frameStart) ? (float)(f - srcClip.frameStart) / (float)(srcClip.frameEnd - srcClip.frameStart) : 1.0f;
        u16 newFrm = DoorFrameFromProgress(dstClip,1.0f - p); // Direct frame assignment (mid-anim reversal): clip + frame + matching model.
        e->clip = dst; e->frame = newFrm; e->currentFrameFinished = 0.0; e->modelIndex = dstClip.frameStartModelIndex + (u16)(newFrm - dstClip.frameStart); e->doorOpen = e->doorState = op ? DoorState_Closing : DoorState_Opening;
        if (!op) e->waitBeforeClose = World.pauseRelativeTime + e->delay; if (e->SFXIndex >= 0 && e->SFXIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXIndex], AppliedFXVol(1.0f), World.position[self], true);
    }
}

void DoorUse(u16 self, u16 activator) {
    if (activator == WORLD) return; Entity* e = &World.instances[self]; if (GetCurrentLevelSecurity() > UsableOrDef((float)e->securityThreshold,100.0f)) { UIBlockedBySecurity(World.position[self]); return; } if (Cheats.superoverride || World.diffMis <= 0) { EntitySetLocked(e,false); e->requiredAccessCard = ACC_None; }
    if (World.diffMis <= 1) { e->requiredAccessCard = ACC_None; } if (e->useFinished >= World.pauseRelativeTime) return; e->useFinished = World.pauseRelativeTime + 0.15f;
    if (e->requiredAccessCard != ACC_None) { if (!DoorInventoryHasAccessCard(e->requiredAccessCard)) {CenterStatusPrint("%s%s",AccessCardCodeForType(e->requiredAccessCard),Sys_Text.stringTable[2]); play_wav(sounds[467], AppliedFXVol(0.7f), World.position[self], true); return;} else {e->requiredAccessCard = ACC_None;}}
    if ((e->entflags & EF_LOCKED) != 0) { CenterStatusPrint("%s",Sys_Text.stringTable[e->lockedMessageLingdex]); play_wav(sounds[467], AppliedFXVol(0.55f), World.position[self], true); return; }  if ((e->onlyTargetOnce && !e->targetAlreadyDone) || !e->onlyTargetOnce) { e->targetAlreadyDone = true; UseTargets(self,e->targetIdx); } if (e->ajar) e->ajar = false; DoorActuate(self);
}

void DoorTargetted(u16 self, u16 activator) { if ((World.instances[self].entflags & EF_LOCKED) != 0) EntitySetLocked(&World.instances[self],false); if (!World.instances[self].targettingOnlyUnlocks) DoorUse(self,activator); }
void DoorUpdate(u16 self) {
    Entity* e = &World.instances[self]; if(e->ajar){return;} AnimationClip opening=DoorGetClip(e,A_OPENING), closing=DoorGetClip(e,A_CLOSING);
    if (e->doorOpen == DoorState_Opening && e->clip == A_OPENING && e->frame >= opening.frameEnd) { e->doorOpen = e->doorState = DoorState_Open; ChangeAnim(e,A_IDLE_OPEN); } else if (e->doorOpen == DoorState_Closing && e->clip == A_CLOSING && e->frame >= closing.frameEnd) { e->doorOpen = e->doorState = DoorState_Closed; ChangeAnim(e,A_IDLE_CLOSED); } if (World.pauseRelativeTime > e->waitBeforeClose && e->doorOpen == DoorState_Open && !e->stayOpen && !e->startOpen) DoorClose(self);
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
    if (e->index == 710 && e->questBitID != QB_None) { // info_mission/QuestBitRelay.  EnableBits/DisableBits act on the *activating* object's bits (Unity TargetIO.Targetted reads tempUD, never the receiver's own line), so no fallback to e->ioflags here.  Unity keeps evaluating the remaining TargetIO actions, hence no early out.
        if (aioflags & TARG_IOFLAGS_MISSION_BIT_ON) QuestBitSet(e->questBitID);
        if (aioflags & TARG_IOFLAGS_MISSION_BIT_OFF) QuestBitClear(e->questBitID);
        if (aioflags & TARG_IOFLAGS_MISSION_BIT_TOGGLE) QuestBitToggle(e->questBitID);
    }
    /*TestBits is separate from the set path because Unity looks the relay up per-action: QuestBitRelay only ships on
      info_mission, so a testQuestBitIsOn object without one is a no-op in the reference game (level8's lev8firstdoor
      gate is exactly that).  DIVERGENCE (intended): a test is honoured on any entity that names a bit and a test mode,
      so those gates actually fire.  Naming a bit is what opts in -- questBitID defaults to QB_None, matching Unity's
      "if (<boolean> && ...)", so the untagged gates stay inert rather than all testing bit 0 at once.  A fired test
      hands its own line down instead: Unity QuestBits.RunTargets does ud.SetBits(tio) + UseTargets(null,ud,target).*/
    if (e->questBitID != QB_None) {
        u8 tm = e->questTestMode; if (!tm && activator != WORLD && activator < World.instCount) tm = World.instances[activator].questTestMode;/*1==testQuestBitIsOn, 2==testQuestBitIsOff*/
        if (tm) { bool bitOn = QuestBitIsSet(e->questBitID); u32 savedFlags = World.targetIOActivatorIoflags, savedFlagsHi = World.targetIOActivatorIoflagsHi; World.targetIOActivatorIoflags = e->ioflags; World.targetIOActivatorIoflagsHi = e->ioflagsHi; UseTargets(activator, (tm == 1) == bitOn ? e->targetIdx : e->targetIfFalseIdx); World.targetIOActivatorIoflags = savedFlags; World.targetIOActivatorIoflagsHi = savedFlagsHi; }
    }
    if (e->index == 709) { CenterStatusPrint("%s", Sys_Text.stringTable[e->messageLingdex]); return; }/*info_message*/   if (e->index == 708) { GameEndSequence(); return; }/*info_gameend: Unity GameEnd.cs Targetted() sets gameFinished, pauses, enables the main menu and plays the credits*/
    if (e->index == 707) { EmailTargetted(self); return; }/*info_email*/                                                 if (aioflags & TARG_IOFLAGS_TRIPTRIGGER) { if(e->index == 598 || e->index == 600){TriggerTargetted(self,activator);}else if(e->index == 594){TriggerCounterTargetted(self,activator);} }
    if (aioflags & TARG_IOFLAGS_UNLOCK) { EntitySetLocked(e, false); if (IdxIsDoor(e->index) && (aioflags & (TARG_IOFLAGS_DOOROPEN | TARG_IOFLAGS_DOOROPENIFUNLOCKED))) e->requiredAccessCard = ACC_None;/*TargetIO.cs:145 pairs dr.Unlock() with dr.accessCardUsedByPlayer = true -- that flag is what retires a card requirement on a scripted unlock, and doorOpenIfUnlocked tests it alongside Inventory.a.HasAccessCard(). Voxen has no such field; DoorUse tests requiredAccessCard directly, so drop it here. Scoped to the force-open combos, which are exactly the records that used to clear it inside DoorForceOpen. UNLOCK-without-open still leaves the card check in DoorUse intact. */ }                                                       if ((aioflags & TARG_IOFLAGS_LOCK) && IdxIsDoor(e->index)) EntitySetLocked(e, true);                                     if (IdxIsButtonSwitch(e->index)) ButtonSwitchUse(self,activator);
/* Unity's ForceOpen() calls OpenDoor(), which does anim.Play(openClip,0,0f) -- it restarts the swing at 0% even mid-stroke, and skips the security/access-card/locked-message checks. That is the right call for a bare doorOpen record (3 of 83), but a record that also unlocks the door is a keypad or button acting as a keycard: 62 of 83. Those want the ordinary use path, so a door that is already moving reverses from its current frame via DoorActuate instead of snapping to 0%, and the door's own threshold, access card and locked message all still apply. TARG_IOFLAGS_UNLOCK already ran above, so the door is unlocked by the time DoorUse checks it. */
    if ((aioflags & TARG_IOFLAGS_DOOROPEN) && IdxIsDoor(e->index) && !(aioflags & TARG_IOFLAGS_UNLOCK)) { DoorForceOpen(self); } else if ((aioflags & TARG_IOFLAGS_DOOROPENIFUNLOCKED) && IdxIsDoor(e->index) && !(aioflags & TARG_IOFLAGS_UNLOCK) && (e->entflags & EF_LOCKED) == 0 && (e->requiredAccessCard == ACC_None || (World.invP1.accessCardOwned & (1u << e->requiredAccessCard)))) { DoorForceOpen(self); } else if ((aioflags & TARG_IOFLAGS_DOORCLOSE) && IdxIsDoor(e->index)) { DoorForceClose(self); } else if (IdxIsDoor(e->index)) { DoorTargetted(self, activator); }
    if (aioflags & TARG_IOFLAGS_FBRIDGE_ACTIVATE) ForceBridgeActivate(self, false); else if (aioflags & TARG_IOFLAGS_FBRIDGE_DEACTIVATE) ForceBridgeDeactivate(self, false); else if (aioflags & TARG_IOFLAGS_FBRIDGE_TOGGLE) ForceBridgeToggle(self);
    if (aioflags & TARG_IOFLAGS_GRAVLIFT_TOGGLE) { World.instances[self].active=!World.instances[self].active; if (e->index == 596) GravityLiftSyncVisuals(self); } if (aioflags & TARG_IOFLAGS_TEXTURE_CHG_TOGGLE) TextureChangerToggle(self);
    if (aioflags & TARG_IOFLAGS_FUNCWALL_MOVE) FuncWallTargetted(self);                                                  if (aioflags & TARG_IOFLAGS_SWITCH_LOCK_TOGGLE) EntitySetLocked(e, (e->entflags & EF_LOCKED) == 0);
    if (aioflags & TARG_IOFLAGS_INST_ACTIVATE) flag_set(&e->entflags, EF_ACTIVE, true); else if (aioflags & TARG_IOFLAGS_INST_DEACTIVATE) { if (e->camView != 255) { e->camView = 255; TextureSequenceInit(self, "Static"); flag_set(&e->entflags, EF_ACTIVE, true); }/*camera destroyed: keep its screen, switch it to Static*/ else { flag_set(&e->entflags, EF_ACTIVE, false); } } else if (aioflags & TARG_IOFLAGS_INST_TOGGLE) flag_set(&e->entflags, EF_ACTIVE, !(e->entflags & EF_ACTIVE));
    if ((aioflagsHi & TARG_IOFLAGHI_AWAKE_SLEEPING) && IdxIsNPC(e->index)) AIAwakeFromSleep(self);/*TargetIO.cs:398-400.  Runs before the alert below: aiac_idle freezes on EF_ASLEEP, so alerting a still-dormant NPC would be swallowed.  No asleep test needed -- AwakeFromSleep is idempotent.*/
    if ((aioflags & TARG_IOFLAGS_ENEMY_ALERT) && IdxIsNPC(e->index)) AIAlert(self);/*TargetIO.cs:211 alerts the receiver, after the activate/deactivate block above so a dormant NPC is awake before it is alerted*/
}

extern char ioNames[MAX_IO_NAMES][TARG_STRLEN];
INLINE V3 ScreenPointToRayOffset(V3 f,V3 r,float dx,float dy); extern u16 ioNameCount;
// Unity reads the actions off the *activator* (TargetIO.Targetted consumes tempUD set by the activating relay), never off
// the target, so a light needs only a targetname to be findable plus this consumer.  TARG_IOFLAGS_LIGHT_ON/OFF/TOGGLE were
// parsed into entity ioflags but had no reader anywhere in the engine, so every lightOn/lightOff/lightToggle was inert.
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
    /* LIGHTON alone is not enough: the renderer scales by intensity, and level data can leave that baked at
       minIntensity (.01) from a stale save while maxIntensity still holds the authored value.  Toggling the bit
       without moving intensity to the matching lerp endpoint leaves a light that is on and still invisible, which is
       exactly what lev1wall1light did. */
    World.lights[lightIdx].intensity = on ? World.lights[lightIdx].maxIntensity : World.lights[lightIdx].minIntensity;
}
void UseTargets(u16 activator, u16 targetIdx) {
    if(targetIdx==IO_NONE){return;} bool wasActive=World.targetIOActive,succeeded=false; u8 entryLevel=World.currentLevel; if(!wasActive){World.targetIOActive=true; World.targetIOEntryLevel=entryLevel; World.targetIOActivatorIdx=activator; World.targetIOActivatorEntity=World.instances[activator]; World.targetIOActivatorIoflagsHi=World.targetIOActivatorEntity.ioflagsHi; World.targetIOActivatorIoflags=World.instances[activator].ioflags;} const char* targetname=(targetIdx<ioNameCount) ? ioNames[targetIdx] : "";
    for (u8 lev = 0; lev < World.numLevels; ++lev) { if (World.currentLevel != lev) SetLevelPointers(lev); for (u16 i = INSTS_1ST_IDX; i < World.instCount; ++i) { if (World.instances[i].targetnameIdx != targetIdx) {continue;} Targetted(activator,i); succeeded=true; } if (World.lightTargetnames) { for (u16 l = 0; l < World.loadedLights; ++l) { if (World.lightTargetnames[l] != targetIdx) {continue;} TargetLight(activator,l); succeeded=true; } } }
    if (World.currentLevel != entryLevel) {SetLevelPointers(entryLevel);} if (!succeeded) {DualLogWarn("No target found: %s\n",targetname);} if (!wasActive) {World.targetIOActive=false;}
}
// Frob/Use
#define FROB_DISTANCE 4.9f
void MFD_OpenSearch(bool isRH),MFD_CloseSearch(void),MFD_OpenData(bool isRH,u8 code),MFD_OpenPaperLog(int,V3);
/*us_paperlog (603).  Unity PaperLog.Use (PaperLog.cs) is four lines: center-tab to the log view, hand logIndex to
  SendPaperLogToDataTab, force inventory mode.  Voxen had no path for it at all -- 603 matched none of the frobbable
  classes, so a paper log printed its name and did nothing -- even though all eight level-placed copies carry a
  logIndex.*/
static void PaperLogUse(u16 self) { MFD_OpenPaperLog((int)World.instances[self].logIndex, World.position[self]); }
static bool IsPuzzleGridPanel(u16 index) { return index>=609&&index<=613; }
static bool IsPuzzleWirePanel(u16 index) { return index>=741&&index<=745; }
static bool IsElevatorPanel(u16 index) { return index>=604&&index<=607; }
static bool IsFrobUsableSpecial(u16 index) { return index==574||index==546||index==608||index==614||index==602||index==603/*us_paperlog*/||IsElevatorPanel(index)||IsPuzzleGridPanel(index)||IsPuzzleWirePanel(index); }/*546 prop_charge_station; 614 us_relaypanel, 602 us_isotopepanel: InteractablePanel; 603 us_paperlog: PaperLog*/
/*---- Wire puzzle data (Unity PuzzleWirePuzzle.cs) ----------------------------------------------------------------------------
  Split by provenance, because the two halves come from different places:
    per instance, from the level line  currentPositionsLeft[i] / currentPositionsRight[i] -> Entity.wireCurL/R
    per prefab, static below          solutionPositions*, wiresOn, rowsActive, wireColors
  Voxen used to invent both halves: the targets were left at whatever the previous panel left behind (a stale solved
  panel handed the next one the previous answer), and the current positions were ignored entirely, so every wire
  puzzle opened with the column arrays all -1.  Values below are read straight out of
  Assets/Resources/Prefabs/us_puz_panel_*_wire.prefab (741 blue, 742 brown, 743 gray, 744 red, 745 teal).
  Unity's YAML packs these as fixed-width fields, so the extraction is 8 chars per int and 2 per bool/HUDColor:
  currentPositionsLeft: 02000000 00000000 04000000 ... -> {2,0,4,...}; wiresOn: 01010100000000 -> {T,T,T,F,...}.
  wireColors are HUDColor (Enumerations.cs:73 White,Red,Orange,Yellow,Green,Blue,Purple,Gray) and the first three live
  wire slots of 741/743/744 are Blue(5) and 742's third is Purple(6), so this needs real blue and purple text colors --
  T_BLUE/T_PURPLE were appended to textColors[] from PuzzleWire's serialized actualColorBlue/actualColorPurple.*/
/* hudColorToText / hudColorToRGB now live in common.h: the wire puzzle needs the HUDColor itself (for the line tint)
   and its text-colour equivalent (for the Genius hint glyph) on both sides of the citadel.c / ui.c split. */
typedef struct { u16 constIndex; i8 tgtL[7],tgtR[7]; bool wireOn[7]; u8 wireColor[7]; u8 rowsActive[7]; } WirePuzzleDef;
static const WirePuzzleDef wirePuzzleDefs[] = {
    /*741 blue  */{741,{1,2,5,-1,-1,-1,-1},{1,3,4,-1,-1,-1,-1},{true,true,true,false,false,false,false},{0,5,4,0,0,0,0},{1,1,1,1,1,1,0}},
    /*742 brown */{742,{1,3,2,-1,-1,-1,-1},{2,4,3,-1,-1,-1,-1},{true,true,true,false,false,false,false},{1,5,6,2,2,2,2},{1,1,1,1,1,1,0}},
    /*743 gray  */{743,{1,2,5,-1,-1,-1,-1},{1,3,4,-1,-1,-1,-1},{true,true,true,false,false,false,false},{0,5,4,0,0,0,0},{1,1,1,1,1,1,0}},
    /*744 red   */{744,{2,1,-1,-1,-1,-1,-1},{2,1,-1,-1,-1,-1,-1},{true,true,false,false,false,false,false},{0,5,0,0,0,0,0},{1,1,1,1,0,0,0}},
    /*745 teal  */{745,{0,1,2,3,-1,-1,-1},{2,4,5,0,-1,-1,-1},{true,true,true,true,false,false,false},{4,4,4,4,4,4,4},{1,1,1,1,1,1,0}},
};
static const WirePuzzleDef* WirePuzzleDefFor(u16 constIndex) { for (u32 i=0;i<sizeof(wirePuzzleDefs)/sizeof(wirePuzzleDefs[0]);++i) if (wirePuzzleDefs[i].constIndex==constIndex) return &wirePuzzleDefs[i]; return NULL; }
/*Unity indexes currentPositions by wire ("wire 2 sits on row 3"); pw_curL/pw_curR are indexed by column ("row 3 holds
  wire 2"), which is what PWFindCol and the swap-on-click in ui.c expect.  Inverting the permutation is the whole
  conversion; the target arrays stay wire-indexed, matching PWEval.*/
static void WireLoadInstance(u16 self, const WirePuzzleDef* def) {
    Entity* e=&World.instances[self];
    for (u8 c=0;c<7;++c) { World.Sys_UI.pw_curL[c]=-1; World.Sys_UI.pw_curR[c]=-1; }
    for (u8 w=0;w<7;++w) { int l=(int)e->wireCurL[w], r=(int)e->wireCurR[w]; if (w<7 && l>=0 && l<7) World.Sys_UI.pw_curL[l]=(i8)w; if (w<7 && r>=0 && r<7) World.Sys_UI.pw_curR[r]=(i8)w; }/*All seven slots, not just the first three: 745 teal runs four wires and level5's teal instance saves currentPositionsRight[3..5], so a w<3 guard dropped its fourth wire and left that panel unsolvable.*/
    for (u8 w=0;w<7;++w) { World.Sys_UI.pw_tgtL[w]=def->tgtL[w]; World.Sys_UI.pw_tgtR[w]=def->tgtR[w]; World.Sys_UI.pw_wireOn[w]=def->wireOn[w]; World.Sys_UI.pw_rowActive[w]=def->rowsActive[w]!=0; World.Sys_UI.pw_wireColor[w]=def->wireColor[w];/*raw HUDColor, as PuzzleWire.wireColors holds it*/ }
}
/*---- Grid puzzle per-instance data (Unity PuzzleGridPuzzle) --------------------------------------------
  PuzzleGridPuzzle.Save writes only puzzleSolved, grid[0..34], fired and locked.  cellType, gridType, theme,
  sourceIndex, outputIndex, securityThreshhold and target are scene-authoring overrides on the prefab instances in
  Assets/Scenes/CitadelScene.unity and are absent from Data/level*.txt, so they have to be baked in here.

  Keyed by the level-local position the level file itself carries (lP, which is what World.position[] holds), NOT by
  the 35-bit saved board.  The board is mutable, so a board key breaks the moment a player flips a cell, and it is
  also not unique: lev5flightbay3iris and lev5flightbay23bulkhead ship byte-identical grid[] yet differ on cellType 7
  and 21 (Bypass+And vs Standard+Standard), so a board key silently gave bulkhead iris's layout.

  Every value below is read out of the scene's PrefabInstance modifications for the PuzzleGridPuzzle component
  (script guid 4cc3aebad1e319a47842939088a42748) of the five us_puz_panel_*_grid prefabs, merged over the prefab
  defaults.  All 21 prefab instances in CitadelScene.unity were enumerated; 11 carry a real layout (width 7,
  height 5, source 14, output 20) and are listed here, and the other 10 are the panel mesh reused on wire panels
  with no grid data at all (width/height 0, no grid/cellType arrays).

  gridType is PuzzleGridType {King,Queen,Knight,Rook,Bishop,Pawn} (Enumerations.cs:86) and is NOT uniformly King:
  the two Bishop(4) panels are levRmedbeddoor and levRrobotspawncontrol, lev4hiddenclosetforcedoor is Rook(3), and
  lev1wall1relay plus both lev7antennafield1 are Pawn(5).  It only picks which piece a Standard cell behaves like
  (PuzzleGrid.cs:117 click, :141 hover) and is overridden to King outright on Easy difficulty (PuzzleGrid.cs:114).
  theme is HUDColor, and PuzzleGrid.UpdateCellImages only branches on Gray/Green/Purple/Blue -- White, Red, Orange
  and Yellow all fall through to the default gray sprites, so theme is recorded for completeness but does not change
  the current art.  securityThreshhold is 90 on lev1xdoor1 and 100 everywhere else.  target is the TargetIO name and
  is what fires the door/relay on solve; all nine distinct names appear as targetname somewhere in Data/level*.txt.*/
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
/*Level-local lP is the level file's own precision, so match on a tolerance rather than exact equality.  0.05 is well
  under the smallest gap between any two placed panels on the same level (level 5's two at y 23.74/22.97 are 0.77
  apart, and the two lev7antennafield1 are 1.6 apart) while absorbing the decimal truncation in the level text.*/
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
    /*Intern the scene-authored target so UseTargets has something to fire.  The level line carries only grid/solved/
      fired, so e->targetIdx is IO_NONE until now; all nine distinct names appear as targetname somewhere in the level
      data, so the interned index matches the entity that actually listens for it.  levRrobotspawncontrol's own level
      line already carries target:levRrobotspawncontrol, and IOInternName returns that same index, so this is a no-op
      there rather than a second, different name.*/
    if (def) { if (def->target) e->targetIdx=IOInternName(def->target);
               /*securityThreshhold is a scene override too and is absent from every one of the 11 level lines, so the
                 entity reads 0 and PuzzlePanelUse's UsableOrDef would fall back to 100.  lev1xdoor1 is authored 90.
                 Safe to re-assert: no record in the level data writes a panel's securityThreshhold at runtime.*/
               e->securityThreshold=def->security; }
    /*NOT re-asserting def->locked here.  PuzzleGridPuzzle.locked is a save field that TargetIO's unlock record
      mutates at runtime (entity.c maps unlockPuzzlePad to TARG_IOFLAGS_UNLOCK, and EntityTargetted clears EF_LOCKED),
      and both locked panels are unlocked that way: level 7's gray lev7antennafield1 by func_logic_relay 699 at
      line 8009 (unlockPuzzlePad:1) and level 2's levRrobotspawncontrol by the keypad at line 6101.  Re-asserting the
      scene value on every open would re-lock the panel the moment the player walked back to it after a relay fired.
      The 2 panels whose level lines carry locked:1 are already seeded correctly by the loader, and the 9 that do not
      are all authored locked:0, so the scene value never has to be written back at all.*/
    /*Always load from this panel's own Entity.  There used to be an InventorySystem pg_*Cache consulted first, but it
      is a single global slot keyed only on 7x5 plus gridType, and gridType is shared by 10 of the 11 panels (5 King,
      2 Bishop, 3 Pawn).  Solving one King panel and walking to another therefore restored the first panel's cells, and
      worse, its cellType[] with them.  e->gridCells lives in levelInstances[lev] inside GlobalContext, so it already
      survives level transitions and save/load on its own; UI_PuzzleGridCell writes it back on every click.  The cache
      had no durability the Entity did not already have and only added a cross-panel contamination path.*/
    if (def) {
        World.Sys_UI.pg_gridType=(PuzzleGridType)def->gridType; World.Sys_UI.pg_theme=def->theme; mcpy(World.Sys_UI.pg_type,def->cellType,sizeof(def->cellType));
    } else {
        World.Sys_UI.pg_gridType=PuzzleGridType_King; World.Sys_UI.pg_theme=HUDColor_Gray; mset(World.Sys_UI.pg_type,PuzzleCellType_Standard,sizeof(World.Sys_UI.pg_type));
    }
    mcpy(World.Sys_UI.pg_cell,e->gridCells,sizeof(e->gridCells));
    World.Sys_UI.pg_solved=e->puzzleSolved; World.Sys_UI.pg_fired=e->puzzleFired;
    { PGEvalPuzzle(); }
}
static void PuzzlePanelUse(u16 i) {
    Entity* e=&World.instances[i];
    if(GetCurrentLevelSecurity()>UsableOrDef((float)e->securityThreshold,100.0f)){UIBlockedBySecurity(World.position[i]);return;}
    if(e->entflags&EF_LOCKED){CenterStatusPrint("%s",Sys_Text.stringTable[302]);return;}
    World.Sys_UI.objectInUsePos=World.position[i]; World.Sys_UI.usingObject=true; ForceInventoryMode();
    if(IsPuzzleGridPanel(e->index)){
        if(!e->panelOpen){
            e->panelOpen=true;
            ChangeAnim(e,A_OPENING);
            play_wav(sounds[91],AppliedFXVol(1.0f),World.position[i],true);
        }
        bool initialize=World.Sys_UI.tetheredPGP!=i||World.Sys_UI.pg_width==0||World.Sys_UI.pg_height==0;
        World.Sys_UI.tetheredPGP=i;
        if(initialize){
            GridLoadInstance(i);
        }
        /*Unity's MFDManager.SendGridPuzzleToDataTab raises the grid on whichever side the player last used the data
          tab on, same as BlockedBySecurity above; it was hardcoded to the left hand MFD here, so a player who had
          been reading their inventory on the right got the puzzle thrown onto the other screen.*/
        MFD_OpenData(World.Sys_UI.lastDataSideRH,3);
    }else{
        const WirePuzzleDef* def=WirePuzzleDefFor(e->index);
        World.Sys_UI.tetheredPWP=i; World.Sys_UI.pw_selectedWire=-1; World.Sys_UI.pw_solved=false;
        if(def) WireLoadInstance(i,def);
        else { for(u8 c=0;c<7;++c){World.Sys_UI.pw_curL[c]=(i8)c;World.Sys_UI.pw_curR[c]=(i8)c;World.Sys_UI.pw_tgtL[c]=(i8)c;World.Sys_UI.pw_tgtR[c]=(i8)c;World.Sys_UI.pw_wireOn[c]=true;World.Sys_UI.pw_rowActive[c]=true;World.Sys_UI.pw_wireColor[c]=HUDC_YELLOW;} }
        /*Unity sends rememberColors on every SendWirePuzzleData; on hard the display colors collapse to all-Yellow
          until Genius reveals them, which is why the colors are stored true here and the display rule is applied at
          the draw site in ui.c rather than baked into pw_wireColor.*/
        MFD_OpenData(World.Sys_UI.lastDataSideRH,4);
    }
    CenterStatusPrint("%s",Sys_Text.stringTable[190]);
}
static bool PanelUseAllowed(u16 i) {
    Entity* e=&World.instances[i];
    if(GetCurrentLevelSecurity()>UsableOrDef((float)e->securityThreshold,100.0f)){UIBlockedBySecurity(World.position[i]);return false;}
    if(e->entflags&EF_LOCKED){
        u16 msg=(u16)e->lockedMessageLingdex;if(msg<T_LOGSTR_CNT&&msg!=0)CenterStatusPrint("%s",Sys_Text.stringTable[msg]);else CenterStatusPrint("%s",Sys_Text.stringTable[302]);
        /*KeypadKeycode.cs:35-42 and KeypadElevator.cs:47-51 - a locked panel answers with a Vox message naming the
          reason instead of opening, by firing lockedTarget.  Only keypads and elevator panels carry the field, so
          the rest of the locked path is unchanged.*/
        if(e->lockedTargetIdx!=IO_NONE)UseTargets(i,e->lockedTargetIdx);
        return false;
    }
    return true;
}

/*---- InteractablePanel (Citadel us_relaypanel / us_isotopepanel / us_retinalscanner) -----------------------------------
  Unity InteractablePanel.Use: the first frob throws the cover open (open=true, "Open" anim, SFX, message). The next
  frob while holding the required item installs it: installed=true, the installationItem child goes active, the
  effects fire, the held item is consumed and UseTargets runs. Holding the wrong item gives the deny click (43) plus
  the wrongItem message; frobbing an installed panel with an empty hand gives the alreadyInstalled message, and with
  the right item only the alreadyInstalled SFX.
  Voxen has no child GameObjects, so the placed item is the panel model's own frame: anim 45 (puzzlepanel3, "frame: 0 18"
  in Data/models.txt) A_OPENING 1-17 opens the cover and A_INSTALLED (frame 18, model 5614) is the item-in-place look.
  The level dump carries none of Unity's per-instance requiredIndex/messages/SFX/target, so the panels that are
  actually usable are listed in relayPanelScripts[], keyed by the targetname added to their level-data line; panels with
  no entry (the decorative level-3 ones, all saved open+installed) just animate and take no item.*/
/*How the installed item shows up. puzzlepanel3's animation has one chipset baked into frame 18, so only the relay 428
  panel (whose installationItem IS a chipset mesh) may use it; the antenna and isolinear panels get a real item entity
  spawned at the installationItem transform, and the isotope panel is its own model with an interim A_INSTALL clip.*/
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
/*Cover animation finishing into the open pose, and the armed 15s fuse: Unity's DelayedSpawn on the panel's
  ExplosionTimer activates the Explosion child (ExplosionLife/GrenadeActivate + light + sound) and basedestroyed
  after the delay. Voxen plays the explosion in place and removes the panel itself (DeleteInstance, not a despawn).*/
static void PuzzlePanelUpdate(u16 self) {
    Entity* e=&World.instances[self];
    if (e->clip==A_IDLE_CLOSED && e->panelOpen) ChangeAnim(e,A_IDLE_OPEN);
    if (e->panelOpen && e->clip==A_OPENING) { AnimationClip c=DoorGetClip(e,A_OPENING); if (c.frameEnd<=c.frameStart || e->frame>=c.frameEnd) ChangeAnim(e,A_IDLE_OPEN); }
}
static void RelayPanelUpdate(u16 self) {
    Entity* e=&World.instances[self];
    /*Panels that load already open/installed (the level data carries their state) start on the matching frame
      instead of the closed cover: installed (frame 18) wins over open (frame 17).*/
    if (e->clip==A_IDLE_CLOSED) { if (e->panelInstalled) ChangeAnim(e,A_INSTALLED); else if (e->panelOpen) ChangeAnim(e,A_IDLE_OPEN); }
    if (e->panelOpen && e->clip==A_OPENING) { AnimationClip c=DoorGetClip(e,A_OPENING); if (c.frameEnd<=c.frameStart || e->frame>=c.frameEnd) ChangeAnim(e,A_IDLE_OPEN); }
    if (e->panelOpen && e->clip==A_INSTALL) { AnimationClip c=DoorGetClip(e,A_INSTALL); if (c.frameEnd<=c.frameStart || e->frame>=c.frameEnd) ChangeAnim(e,A_INSTALLED); }/*isotope: insertion clip hands over to the installed frame*/
    if (e->panelArmed && World.pauseRelativeTime>=e->panelTimer) {
        e->panelArmed=false; V3 p=World.position[self];
        play_wav(sounds[64]/*explosion1*/,AppliedFXVol(1.0f),p,true); SpawnExplosionEffect(p,1); Shake(-1.0f); World.fogFac += 5.0f;
        /*Unity activates the Explosion and basedestroyed children and leaves the panel (the installationItem stays
          active too). Voxen keeps the panel instance and swaps in the destroyed texture (Textures/pnl3_ded.png) on
          the same model, so the burnt panel is what is left behind.*/
        const RelayPanelScript* sc=RelayPanelScriptFor(e); if (sc && sc->wreckTex) e->texIndex=sc->wreckTex;
    }
}
/*Frob with the item in hand (Citadel MouseLookScript.FrobWithHeldObject): only these useables are "frob users", and
  they are consumed by whatever UseHandler answers the frob; anything else you carry is put away as usual.*/
static bool HeldItemIsFrobUser(i16 item) { return item==54||item==56||item==57||item==61||item==64||item==92||item==93||item==94; }
bool FrobHeldItemIntoPanel(V3 p, V3 f, V3 r) {
    if (!World.invP1.holdingObject) return false;
    i16 item=(i16)(World.invP1.heldObjectIndex-307); if (!HeldItemIsFrobUser(item)) return false;
    RaycastHit h=Raycast(p,ScreenPointToRayOffset(f,r,0,0),FROB_DISTANCE,LMASK_PLAYER_FROB);
    if (!h.hit || h.hitInstanceIndex>=World.instCount) return false;
    u16 idx=h.hitInstanceIndex; if (World.instances[idx].index!=614 && World.instances[idx].index!=602) return false;
    RelayPanelUse(idx); return true;/*the panel decides: it either installs the item (consuming it) or refuses it*/
}
/*Elevator floor button layouts, from Textures/UI/ElevatorCheetSheet.txt (Unity ElevatorKeypad buttonText/buttonsEnabled/buttonsDarkened ground truth).
  label: index into elevFloorLabels[] (R=0,1=1..9=9,G1=10,G2=11,G4=12), -1 = hidden (not drawn, not clickable).
  darkened: drawn dimmed, not clickable (Unity buttonsDarkened).*/
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
/*Panel (level, x, y, z) -> elevLayouts index. Matched against Unity scene KeypadElevator instances by position.*/
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
    V3 pp=World.position[PLAYER1];
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
    /*Link to nearest active door (Unity KeypadElevator.linkedDoor).*/
    { V3 kp=World.position[i]; float best=1e30f; for (u32 d=INSTS_1ST_IDX;d<World.instCount;++d) { Entity* de=&World.instances[d]; if(!IdxIsDoor(de->index)||!(de->entflags&EF_ACTIVE)) continue; V3 dd=V3_AsubB(World.position[d],kp); float dist2=V3_dot(dd,dd); if(dist2<best){best=dist2; World.Sys_UI.linkedElevatorDoor=(u16)d;} } }
    /*Drive floor buttons off the linked elevator's floor set (Unity ElevatorKeypad).*/
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
    /*KeypadKeycode.cs:45-80 - the two level-R self-destruct pads take no keycode of their own; the three digits are
      the level security codes the player has to have read off the CPU screens, huns/tens/ones from 1,2,3 on one pad
      and 4,5,6 on the other.  Both records ship keycode 666 as a placeholder, which this overwrites.  A code the
      player has not earned is -1 and refuses the pad with 289/290 plus the security MFD tab, same as
      MFDManager.BlockedBySecurity.  Unity draws all six at NewGame so that guard is dead there; here they are drawn
      at lock (CodeScreensSetForLevel) so it is live.*/
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
    play_wav(sounds[91], AppliedFXVol(0.75f), (V3){0,0,0}, false); ForceInventoryMode();
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
    if (IdxIsSearchable(ent->index) || (World.layer[i]&(L_Corpse|L_CorpseSearchable)) || (IdxIsGib(ent->index) && (World.layer[i]&L_Corpse))) { SearchObject(i); } else if (IdxIsDoor(ent->index)) DoorUse(i,PLAYER1); else if (IdxIsNPC(ent->index)) CenterStatusPrint("%s%s",Sys_Text.stringTable[29],npcTable[World.instances[i].index - 419].name); else if (IdxIsButtonSwitch(ent->index)) ButtonSwitchUse(i,PLAYER1);
    else if(ent->index==574) HealingBedUse(i,PLAYER1); else if(ent->index==546) ChargeStationUse(i,PLAYER1); else if(ent->index==614||ent->index==602) RelayPanelUse(i); else if(IsElevatorPanel(ent->index)) ElevatorPanelUse(i); else if(ent->index==608) KeycodePanelUse(i); else if(IsPuzzleGridPanel(ent->index)||IsPuzzleWirePanel(ent->index)) PuzzlePanelUse(i); else if(ent->index==603) PaperLogUse(i);
    else if (IdxIsGeometry(ent->index)) { int t = UseNameTableIndex(ent->index); CenterStatusPrint("%s%s",Sys_Text.stringTable[29],t >= 0 ? Sys_Text.stringTable[t] : ""); }
    else if (IdxIsUsableObject(ent->index)) {
        World.invP1.holdingObject = true; World.invP1.heldObjectIndex = ent->index; World.invP1.heldObjectCustIdx = ent->customIndex; World.invP1.heldAmmo = ent->ammo; World.invP1.heldAmmo2 = ent->ammo2; World.invP1.heldObjectLoadedAlternate = ent->heldObjectLoadedAlternate;
        if (Sys_Settings.QuickItemPickup) { AddItemToInventory(ent->index,ent->customIndex); ResetHeldItem(); } else { CenterStatusPrint("%s%s",Sys_Text.stringTable[World.invP1.heldObjectIndex - 307 + 326],Sys_Text.stringTable[319]); /* picked up.*/ ForceInventoryMode(); }/*Inventory mode is turned on when picking something up*/ DeleteInstance(i);
    } else { int t = UseNameTableIndex(ent->index); CenterStatusPrint("%s%s",Sys_Text.stringTable[29],t >= 0 ? Sys_Text.stringTable[t] : ""); }
}

INLINE V3 ScreenPointToRayOffset(V3 f,V3 r,float dx,float dy){float px=(World.inventoryMode?(float)World.cursorPos_x:(float)UI_W*0.5f)+dx,py=(World.inventoryMode?(float)World.cursorPos_y:(float)UI_H*0.5f)+dy,t=vtan((float)Sys_Settings.FOV*0.5f*PI/180.0f),aspect=(float)Sys_Settings.ScreenWidth/(float)Sys_Settings.ScreenHeight,nx=(px-(float)UI_W*0.5f)/((float)UI_W*0.5f),ny=((float)UI_H*0.5f-py)/((float)UI_H*0.5f);V3 v=V3_Normalize((V3){nx*aspect*t,ny*t,-1.0f}),ff=(V3){-f.x,-f.y,-f.z},up=V3_Normalize(V3_Cross(r,ff));return(V3){v.x*r.x+v.y*up.x+v.z*ff.x,v.x*r.y+v.y*up.y+v.z*ff.y,v.x*r.z+v.y*up.z+v.z*ff.z};}
/* Non-static wrapper so weapons.c can apply pixel drift before the ray is built (Unity: drift added to the screen point before ScreenPointToRay). */
V3 ScreenPointToRayPixels(V3 f,V3 r,float dx,float dy){return ScreenPointToRayOffset(f,r,dx,dy);}
INLINE bool FrobRayIsFrobable(RaycastHit h){if(!h.hit)return false;u16 i=h.hitInstanceIndex;if(i>=World.instCount)return false;u16 e=World.instances[i].index;if((World.layer[i]&(L_Corpse|L_CorpseSearchable)))return true;return IsFrobUsableSpecial(e)||IdxIsUsableObject(e)||IdxIsSearchable(e)||IdxIsDoor(e)||IdxIsButtonSwitch(e)||IdxIsNPC(e)||IdxIsGib(e);}
extern bool editFieldEditing;
static bool TargetIDFrob(V3 p,V3 f,V3 r){V3 dir=ScreenPointToRayOffset(f,r,0,0);RaycastHit h=Raycast(p,dir,TargetIDGetSensingRange(true),LMASK_PLAYER_TARGET_ID_FROB);if(!h.hit||h.hitInstanceIndex>=World.instCount||!IdxIsNPC(World.instances[h.hitInstanceIndex].index))return false;u16 i=h.hitInstanceIndex;Entity* e=&World.instances[i];if((e->entflags&(EF_DYING|EF_DEAD))||!HasHealth(i)){if(World.layer[i]&(L_Corpse|L_CorpseSearchable)){UseEntity(i);return true;}return false;}/*Death flags, not health: AIDead restores health to NPC_CORPSE_HEALTH on the 15 searchCollider types (mask bit 4 = cyborg drone) so corpses stay shootable, which made a fully dead drone test as alive here.*/if((World.invP1.hasHardware&HW_TID)&&World.invP1.hwVers[HW_TID_IDX]>1){if(targetIDAttached[i]&&targetIDAttachedFinished[i]<=World.pauseRelativeTime)targetIDAttached[i]=false;if(!targetIDAttached[i]){CreateTargetIDInstance(-1.0f,i,-1.0f);return true;}}CenterStatusPrint("%s%s",Sys_Text.stringTable[29],npcTable[e->index-419].name);return true;}
static void Frob(V3 p,V3 f,V3 r){
    if(World.uiIsBlocking||World.curLev==LEVEL_CYBERSPACE)return;
    if(Cheats.editMode){V3 d0=ScreenPointToRayOffset(f,r,0,0);RaycastHit fh=Raycast(p,d0,World.farPlane[World.curLev],LMASK_PLAYER_FROB);editModeSelection=(fh.hit&&fh.hitInstanceIndex>=INSTS_1ST_IDX&&fh.hitInstanceIndex<World.instCount)?fh.hitInstanceIndex:U16_MAX; if(editModeSelection<U16_MAX){editFieldEditing=false; CenterStatusPrint("Selected object %u (const index %u)",editModeSelection,World.instances[editModeSelection].index);}else{CenterStatusPrint("Object deselected");}return;/*No pickup/search/use while in edit mode; selection only.*/}
    if(World.Sys_UI.vmailActive){World.Sys_UI.vmailActive=0;return;}if(World.invP1.holdingObject){if(FrobHeldItemIntoPanel(p,f,r)){return;} DropHeldItem();return;}if(TargetIDFrob(p,f,r))return;float o=(float)UI_H*0.02f;RaycastHit fh={0},bh={0};bool ok=false;V3 d0=ScreenPointToRayOffset(f,r,0,0);fh=Raycast(p,d0,FROB_DISTANCE,LMASK_PLAYER_FROB);bh=fh;ok=FrobRayIsFrobable(fh);float ox[8]={0,0,o,-o,o,-o,-o,o},oy[8]={-o,o,0,0,o,-o,o,-o};for(int i=0;i<8&&!ok;++i){V3 d=ScreenPointToRayOffset(f,r,ox[i],oy[i]);RaycastHit th=Raycast(p,d,FROB_DISTANCE,LMASK_PLAYER_FROB);if(FrobRayIsFrobable(th)){bh=th;ok=true;}}if(!ok)bh=fh;if(Cheats.showPhys){World.debugLine_start=p;World.debugLineFinished=World.pauseRelativeTime+3.0;V3 dbg=ok?ScreenPointToRayOffset(f,r,0,0):d0;RaycastHit dh=ok?bh:fh;World.debugLine_end=dh.hit?dh.point:(V3){dbg.x*FROB_DISTANCE+p.x,dbg.y*FROB_DISTANCE+p.y,dbg.z*FROB_DISTANCE+p.z};}if(!ok){if(fh.hit){u16 idx=fh.hitInstanceIndex;if(idx<World.instCount){u16 ei=World.instances[idx].index;if(IdxIsGeometry(ei)||IdxIsDoor(ei)||World.instances[idx].index>=595){int t=UseNameTableIndex(ei);CenterStatusPrint("%s%s",Sys_Text.stringTable[29],t>=0?Sys_Text.stringTable[t]:"");return;}}}CenterStatusPrint("%s",Sys_Text.stringTable[30]);}else UseEntity(bh.hitInstanceIndex);}
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

/* sec_camera (477) SecurityCameraRotate.  Sweeps the camera back and forth between startYAngle and endYAngle,
   pausing waitTime at each end, and only advances while the mesh is on screen (Unity: mR.isVisible).  Unity calls
   transform.Rotate(0, degreesYPerSecond * tickTime, 0, Space.World) once per Update() and never uses tickTime as a
   timestep, so at 60Hz that is 4 * 0.1 = 0.4 deg per frame = 24 deg/sec; stepped by World.dt here so the sweep speed
   does not track the frame rate.  The yaw is composed onto the authored rotation rather than accumulated in world
   space, which is what the wrapper-GameObject note in SecurityCameraRotate.cs describes and keeps the prefab's droop
   from precessing.  Start(): waitingFinished = relativeTime, rotatePositive = true. */
#define CAM_SWEEP_DEG_PER_SEC 24.0f
#define CAM_SWEEP_EPSILON    1.0f
INLINE float CamYawFromQuat(Quaternion q) { /*standard ZYX yaw extraction, only used to seed the sweep, so Unity's
    ZXY eulerAngles.y ordering does not have to be reproduced exactly*/
    float d = __builtin_atan2f(2.0f*(q.x*q.z + q.y*q.w), 1.0f - 2.0f*(q.x*q.x + q.y*q.y)) * (180.0f / 3.14159265f); /*m02/m22 of the rotation matrix: Unity eulerAngles.y, i.e. the Y angle of the ZXY decomposition, which is what startYAngle/endYAngle were authored against.  The ZYX form (2(wy+xz) over 1-2(y^2+z^2)) agrees for an untilted camera but diverges once the prefab droop is present, which every level camera has.*/
    if (d < 0.0f) d += 360.0f; return d;
}
void SecurityCameraRotateUpdate(u16 self) {
    Entity* e = &World.instances[self]; if (!e->camRotateEnabled) return;/*Unity: sec_camera.prefab ships SecurityCameraRotate disabled, so Unity dispatches no Update() at all for the 127 enabled:0 records*/
    if (!(e->entflags & EF_ACTIVE)) return;
    /*Unity: if (mR == null || !mR.isVisible || !mR.enabled) return -- the sweep is frozen whenever the camera is
       off screen.  playerFrustumPlanes is rebuilt in Render(), so here it still holds last frame's planes, which is
       fine for a gate.  Unity mR.isVisible is true when any part of the renderer is in frustum, hence the radius. */
    if (!SphereInFrustum(playerFrustumPlanes, World.position[self], 1.28f)) return;
    if (!e->camSweepInit) { e->camSweepInit = true; e->camBaseRot = World.rotation[self]; e->camYaw0 = CamYawFromQuat(e->camBaseRot); e->camYaw = e->camYaw0; }/*seeded lazily so the authored lR.* has already been applied*/
    if (e->camWaitingFinished >= World.pauseRelativeTime) return;/*Unity: if (waitingFinished < relativeTime)*/
    float dt = World.dt;
    if (e->camRotatePositive) { if (vabs(e->camYaw - e->camEndYAngle) <= CAM_SWEEP_EPSILON) { e->camRotatePositive = false; e->camWaitingFinished = World.pauseRelativeTime + e->camWaitTime; } else e->camYaw += CAM_SWEEP_DEG_PER_SEC * dt; }
    else { if (vabs(e->camYaw - e->camStartYAngle) <= CAM_SWEEP_EPSILON) { e->camRotatePositive = true; e->camWaitingFinished = World.pauseRelativeTime + e->camWaitTime; } else e->camYaw -= CAM_SWEEP_DEG_PER_SEC * dt; }
    if (e->camYaw < 0.0f) e->camYaw += 360.0f; else if (e->camYaw >= 360.0f) e->camYaw -= 360.0f;
    /*Absolute, not cumulative: composed onto the captured authored rotation.  Pre-multiplying the (camYaw-camYaw0) delta onto the previous frame's result would sum 0.4+0.8+1.2+... instead of applying 0.4 each frame, and each world-space Y pre-multiply would precess the prefab droop, which is the skew. */
    float half = deg2rad(e->camYaw - e->camYaw0) * 0.5f; Quaternion yaw = {0.0f, vsinf(half), 0.0f, vcosf(half)};
    World.rotation[self] = quat_multiply(yaw, e->camBaseRot);
}

void ModUpdate() {
    if (World.paused || World.menuActive) return; UpdateSearchTether(); WeaponsUpdate(); InventoryUpdate(); PlayerEnergyUpdate(); PatchUpdate(); HardwareUpdate(); MissionTimerUpdate(); if (Use()) Frob(World.position[PLAYER1],World.instances[PLAYER1].forward,World.instances[PLAYER1].right); if (World.pauseRelativeTime < World.debugLineFinished && (World.debugLineVertCount + 6) < (MAX_WIRELINE_VRTS * 3)) DrawLine(World.debugLine_start,World.debugLine_end,(Color){0.3f,0.1f,0.6f,0.5f});
    for (u16 i=INSTS_1ST_IDX;i<World.instCount;++i) {
        Entity* e = &World.instances[i]; u16 constdex = e->index; if(IsLiveGrenade(constdex) && (e->entflags & EF_ACTIVE)) GrenadeUpdate(i); DelayedSpawnUpdate(i); if(constdex==CYBER_DECOY_CONST) CyberDecoyExpired(i);/*the decoy's own DelayedSpawn just deleted it; clear decoyActive so cyber NPCs stop aiming at a dead index*/ if(constdex==614) RelayPanelUpdate(i); if(IsPuzzleGridPanel(constdex)) PuzzlePanelUpdate(i); if (e->textureAnimating && e->tickFinished < World.pauseRelativeTime) TextureSequenceUpdate(i); if(IdxIsButtonSwitch(constdex)){ButtonSwitchUpdate(i);} if(IdxIsDoor(constdex)){DoorUpdate(i);}    if(constdex == 701){LogicTimerUpdate(i);} if(constdex == 594){TriggerCounterUpdate(i);} if(constdex == 699){LogicRelayUpdate(i);} if(constdex == 702){SpawnManagerUpdate(i);} if(constdex == 477){SecurityCameraRotateUpdate(i);} if(e->itemLifeTime > 0.0f){SearchFXResetUpdate(i);}
        if(e->cyberTimer > 0.0f){CyberTimerUpdate(i);}          if(constdex == 515){ForceBridgeUpdate(i);} if(constdex == 517){FuncWallUpdate(i);}   if(constdex == 21 || constdex == 22){CyberWallUpdate(i);} if(IdxIsNPC(constdex)) { DrawAIDebug(i); AIControllerUpdate(i); AIAnimationControllerUpdate(i); }
        if(constdex==552){CyberDataFragUpdate(i);} if(constdex==554){CyberExitUpdate(i);} if(constdex==555){CyberSwitchUpdate(i);} if((constdex>=448&&constdex<=451)||(constdex>=454&&constdex<=457)){CyberItemUpdate(i);}
    }
    if (World.invP1.painSoundFinished < World.pauseRelativeTime && World.instances[PLAYER1].radiation > 1.0f && !(World.invP1.radSoundFinished < World.pauseRelativeTime)) { World.invP1.painSoundFinished = World.pauseRelativeTime + (double)random_range(2.5f,4.0f); play_wav(sounds[140]/*player/playerpain1*/,AppliedFXVol(0.2f),(V3){0,0,0},false); }
    if (!Cheats.god && World.invP1.radBleedFinished < World.pauseRelativeTime && World.instances[PLAYER1].radiation > 1.0f) { World.invP1.radBleedFinished = World.pauseRelativeTime + 1.8; float take=World.instances[PLAYER1].radiation*0.2f; World.instances[PLAYER1].health-=take; World.painStaticAlpha = take > 15.0f ? 1.0f : take > 10.0f ? 0.8f : 0.3f; }
    if (World.invP1.radSoundFinished < World.pauseRelativeTime && World.instances[PLAYER1].radiation > 1.0f) { double minT = World.instances[PLAYER1].radiation > 50.0f ? 0.5 : 1.0; World.invP1.radSoundFinished = World.pauseRelativeTime + minT + (double)random_range(0.0f,2.0f); play_wav(sounds[90]/*hud/radiation*/,AppliedFXVol(0.18f),(V3){0,0,0},false); }
    /*Puzzle panel tether distance check: sever tether if player moves too far*/
    if (World.Sys_UI.tetheredPGP != U16_MAX) { u16 pg=World.Sys_UI.tetheredPGP; if (pg<World.instCount && (World.instances[pg].entflags&EF_ACTIVE)) { float d2=V3_SqDist(World.position[PLAYER1],World.position[pg]); if (d2 > 64.0f) { UI_PuzzleGridClose(false); } } }
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
