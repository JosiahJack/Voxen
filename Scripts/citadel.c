// citadel.c - Gamelogic.  Most functionality is trivial so put it here.
// TODO: Add camera view entities for other levels than just medical
// TODO: Proper physics
// TODO: Particle system
// TODO: Voxel GI
// TODO: Save/Load system
// TODO: Directional lights for cyberspace
// TODO: Directional light for sunlight
// TODO: Directional light shadowmapping just for sunlight
// TODO: TARGET ID: Type-LevelNum(0#)EnemyNum(###),Example: Mutant-06003, EXCEPTIONS: Cyborg-00001 is Edward Diego
#include "mod.h"
__attribute__((used)) AutoSplitterData autoSplitter = {0x1337133713371337,0,false,0}; // Fore use with LiveSplit or other future speedrunner utilities for doing speedruns
//=============================================================================
// Initialization
GlobalContext* Eng_Global; CheatsSystem*  Eng_Cheats; SettingsSystem* Eng_Settings; TextSystem* Eng_Text; SystemUI* Eng_UI; // From Engine
MOD_TO_ENGINE void ModLink(GlobalContext* g,CheatsSystem* c,SettingsSystem* s,TextSystem* t,SystemUI* ui){Eng_Global=g;Eng_Cheats=c;Eng_Settings=s;Eng_Text=t;Eng_UI=ui;}
int lev1SecCode,lev2SecCode,lev3SecCode,lev4SecCode,lev5SecCode,lev6SecCode;
#ifdef WINDOWS
MOD_TO_ENGINE i32 __stdcall DllMain(void* hinstDLL, unsigned long fdwReason, void* lpReserved) { (void)hinstDLL; (void)lpReserved; switch (fdwReason) {} return 1; }
void* memset(void* dst, int c, size_t n) { return MemSetToVForNBytes(dst,c,n); }
#endif

MOD_TO_ENGINE void ModNewGame(void) {
    lev1SecCode = random_range_u8(0u,9u); lev2SecCode = random_range_u8(0u,9u);
    lev3SecCode = random_range_u8(0u,9u); lev4SecCode = random_range_u8(0u,9u);
    lev5SecCode = random_range_u8(0u,9u); lev6SecCode = random_range_u8(0u,9u); // Must do rand's repeatedly to prevent these all being the same number.
}
//=============================================================================
// Inventory
void ResetHeldItem(u16 p) {
    InventorySystem* inv = Inv(p);
    inv->heldObjectIndex = inv->heldObjectCustomIndex = U16_MAX;
    inv->heldObjectAmmo = inv->heldObjectAmmo2 = 0;
    inv->heldObjectLoadedAlternate = inv->holdingObject = inv->grenadeActive = false;
}

Vector3 ScreenPointToRay(Vector3 fwd, Vector3 rt) {
    u16 swidth = Eng_Settings->ScreenWidth, sheight = Eng_Settings->ScreenHeight;
    float offsetX = Eng_Global->cursorPosition_x - ((float)swidth * 0.5f);
    float offsetY = Eng_Global->cursorPosition_y - ((float)sheight * 0.5f);
    float ndcX = offsetX / ((float)swidth * 0.5f);
    float ndcY = -offsetY / ((float)sheight * 0.5f);
    float tanFov = vtan((float)Eng_Settings->FOV * 0.5f * PI / 180.0f);
    float aspect3D = (float)swidth / (float)sheight;
    Vector3 view = (Vector3){ndcX * tanFov * aspect3D,ndcY * tanFov,-1.0f};
    view = V3_Normalize(view);
    Vector3 flipForward = (Vector3){-fwd.x,-fwd.y,-fwd.z};
    Vector3 up = V3_Normalize(V3_Cross(rt,flipForward));
    return (Vector3){view.x * rt.x + view.y * up.x + view.z * flipForward.x,view.x * rt.y + view.y * up.y + view.z * flipForward.y,view.x * rt.z + view.y * up.z + view.z * flipForward.z};
}

void DropHeldItem(u16 p) {
    InventorySystem* inv = Inv(p);
    Entity* ply = &Eng_Global->instances[p];
    if (inv->heldObjectIndex >= Eng_Global->loadedInstances) { ResetHeldItem(p); return; }
    if (inv->dropFinished > Eng_Global->pauseRelativeTime) return;
    
    inv->dropFinished = Eng_Global->pauseRelativeTime + 0.2; // Prevent immediate regrab at high fps
    u16 newent = AddInstance(inv->heldObjectIndex,ply->position);
    Entity* tossObject = &Eng_Global->instances[newent];
    tossObject->usableCustomIndex = inv->heldObjectCustomIndex;
    tossObject->ammo = inv->heldObjectAmmo;
    tossObject->ammo2 = inv->heldObjectAmmo2;
    tossObject->heldObjectLoadedAlternate = inv->heldObjectLoadedAlternate;
    tossObject->position = ply->position;
    flag_set(&tossObject->entflags,EF_RIGIDBODY,true);
    Vector3 tossDir = V3_Normalize(ScreenPointToRay(ply->forward,ply->right));
    tossObject->position = V3_AplusB(ply->position,V3_ScaleByF(tossDir,0.48f));
    tossObject->velocity = V3_ScaleByF(tossDir,10.0f);
    DualLog("Dropping held object type %u at pos %f %f %f, with force %f %f %f\n",tossObject->index,tossObject->position.x,tossObject->position.y,tossObject->position.z,tossObject->velocity.x,tossObject->velocity.y,tossObject->velocity.z);
    ResetHeldItem(p);
}

void PatchUse(u16 playerIdx,int patchSlot) { (void)playerIdx; (void)patchSlot; } // TODO
void WeaponFireStartWeaponDip(float t) { (void)t; } // TODO
void WeaponFireCompleteWeaponChange(void) { } // TODO
bool InventoryHasAccessCard(u16 p,AccessCardType card) { return (Inv(p)->accessCardOwned & (1u << card)) != 0; }
bool InventoryHasAnyAccessCards(u16 p) { return Inv(p)->accessCardOwned != 0; }
const char* AccessCardCodeForType(AccessCardType a) { // Called by ItemTabManager
    switch(a) {
        case AccessCardType_Standard:    return "STD";
        case AccessCardType_Medical:     return "MED";
        case AccessCardType_Science:     return "SCI";
        case AccessCardType_Admin:       return "ADM";
        case AccessCardType_Group1:      return "Group-1";
        case AccessCardType_Group2:      return "Group-2";
        case AccessCardType_Group3:      return "Group-3";
        case AccessCardType_Group4:      return "Group-4";
        case AccessCardType_GroupA:      return "Group-A";
        case AccessCardType_GroupB:      return "Group-B";
        case AccessCardType_Storage:     return "STO";
        case AccessCardType_Engineering: return "ENG";
        case AccessCardType_Maintenance: return "MTN";
        case AccessCardType_Security:    return "SEC";
        case AccessCardType_Per1:        return "PER-1";
        case AccessCardType_Per2:        return "PER-2";
        case AccessCardType_Per3:        return "PER-3";
        case AccessCardType_Per4:        return "PER-4";
        case AccessCardType_Per5:        return "PER-5";
        default:                         return "Group-2";
    }
}

void AddAccessCardToInventory(u16 p,int index) {
    AccessCardType card;
    switch(index) {
        case  34: card = AccessCardType_Admin;       break;
        case  81: card = AccessCardType_Standard;    break;
        case  83: card = AccessCardType_Group1;      break;
        case  84: card = AccessCardType_Science;     break;
        case  85: card = AccessCardType_Engineering; break;
        case  86: card = AccessCardType_GroupB;      break;
        case  87: card = AccessCardType_Security;    break;
        case  88: card = AccessCardType_Per5;        break;
        case  89: card = AccessCardType_Medical;     break;
        case  90: card = AccessCardType_Group3;      break;
        case  91: card = AccessCardType_Group4;      break;
        case 110: card = AccessCardType_Per1;        break;
        default:
            CenterStatusPrint("BUG: Unmarked access card, defaulting to STD.");
            card = AccessCardType_Standard;
            break;
    }
    if (index == 87) { // Command card = STO + SEC + MTN
        if (InventoryHasAccessCard(p,AccessCardType_Storage) &&
            InventoryHasAccessCard(p,AccessCardType_Security) &&
            InventoryHasAccessCard(p,AccessCardType_Maintenance)) {
            CenterStatusPrint("%s%s",Eng_Text->stringTable[44],AccessCardCodeForType(card));
            return;
        }
        Inv(p)->accessCardOwned |= (1u<<AccessCardType_Storage)|(1u<<AccessCardType_Security)|(1u<<AccessCardType_Maintenance);
        CenterStatusPrint("%s%s, %s, %s",Eng_Text->stringTable[45],AccessCardCodeForType(AccessCardType_Storage),AccessCardCodeForType(AccessCardType_Security),AccessCardCodeForType(AccessCardType_Maintenance));
        return;
    }
    if (InventoryHasAccessCard(p,card)) { CenterStatusPrint("%s%s",Eng_Text->stringTable[44],AccessCardCodeForType(card)); return; }
    Inv(p)->accessCardOwned |= (1u << card);
    CenterStatusPrint("%s%s",Eng_Text->stringTable[45],AccessCardCodeForType(card));
}

void AddHardwareToInventory(u16 p,int index,int constIndex,int hwversion,bool overt) {
    (void)constIndex;
    if (index < 0) return;
    InventorySystem* inv = Inv(p);
    if (hwversion < 0) { CenterStatusPrint("BUG: Hardware added with version < 0, using 0."); hwversion = 0; }
    if (hwversion > 0 && hwversion <= (int)inv->hardwareVersion[index]) {
        if (overt) CenterStatusPrint("%s",Eng_Text->stringTable[46]); // THAT WARE IS OBSOLETE. DISCARDED.
        return;
    }
    static const u8 textIdx[12] = {21,22,23,24,25,26,27,28,29,30,31,32};
    inv->hardwareInvIndex             = index;
    inv->hasHardware                 |= (u16)(1u << index);
    inv->hardwareVersion[index]       = (u8)hwversion;
    inv->hardwareVersionSetting[index]= hwversion > 0 ? (u8)(hwversion - 1) : 0;
    // TODO: engine enables HUD hardware buttons from hasHardware bitmask on render
    // TODO: nav unit (index 1): compass/automap HUD visibility from hasHardware & HW_NAV + version
    if (overt) CenterStatusPrint("%s v%d",Eng_Text->stringTable[textIdx[index] + 326],hwversion);
}

int  NavUnitVersion(u16 p)     { return Inv(p)->hardwareVersion[HW_NAV_IDX]; }
int  BioMonitorVersion(u16 p)  { return Inv(p)->hardwareVersion[HW_BIO_IDX]; }
bool BioMonitorActive(u16 p)   { InventorySystem* i=Inv(p); return (i->hasHardware & HW_BIO) && (i->hardwareIsActive & HW_BIO); }
bool LanternActive(u16 p)      { InventorySystem* i=Inv(p); return (i->hasHardware & HW_LAN) && (i->hardwareIsActive & HW_LAN); }
int  EnvirosuitVersion(u16 p)  { return Inv(p)->hardwareVersion[HW_ENV_IDX]; }
bool BoosterSetToSkates(u16 p) { return Inv(p)->hardwareVersionSetting[HW_BST_IDX] == 0; }
bool BoosterSetToBoost(u16 p)  { return Inv(p)->hardwareVersionSetting[HW_BST_IDX] >= 1; }
bool BoosterActive(u16 p)      { InventorySystem* i=Inv(p); return (i->hasHardware & HW_BST) && (i->hardwareIsActive & HW_BST); }
void JumpJetsToggle(u16 p)     { Inv(p)->hardwareIsActive ^= HW_JET; }
int  JumpJetsVersion(u16 p)    { return Inv(p)->hardwareVersion[HW_JET_IDX]; }
bool JumpJetsActive(u16 p)     { InventorySystem* i=Inv(p); return (i->hasHardware & HW_JET) && (i->hardwareIsActive & HW_JET); }
// HideBioMonitor / UnHideBioMonitor: engine reads InventoryBioMonitorActive() for HUD visibility, no gamecode needed

bool AddGeneralObjectToInventory(u16 p,int index,int customIndex) {
    if (index < 0) return false;
    InventorySystem* inv = Inv(p);
    for (int i = 1; i < 14; i++) {
        if (inv->generalInventoryIndexRef[i] != -1) continue;
        if (!InventoryHasAnyAccessCards(p) && inv->generalInvCurrent == 0) inv->generalInvCurrent = (i8)i;
        inv->generalInventoryIndexRef[i] = index;
        inv->generalInvCustomIndex[i]    = (i16)customIndex;
        CenterStatusPrint("%s%s",Eng_Text->stringTable[index + 326],Eng_Text->stringTable[31]);
        return true;
    }
    return false;
}

void GeneralInventoryActivate(u16 p) {
    InventorySystem* inv = Inv(p);
    int cur = inv->generalInvCurrent;
    if (cur < 0 || cur >= 14) { DualLog("BUG: generalInvCurrent out of range at %d",cur); return; }
    GeneralInvApply(cur,inv->generalInvCustomIndex[cur]);
    if (cur != 0) inv->generalInventoryIndexRef[cur] = -1;
}

void GrenadeCycleDown(u16 p) {
    InventorySystem* inv = Inv(p);
    int last = inv->grenadeCurrent, next = last - 1;
    if (next < 0) next = 6;
    for (int c = 0; c <= 13; c++) {
        if (inv->grenAmmo[next] > 0) break;
        if (c == 13) return;
        if (--next < 0) next = 6;
    }
    if (last == next) return;
    inv->grenadeCurrent = (i8)next;
    static const u16 msg[7] = {579,580,581,582,583,584,585};
    CenterStatusPrint("%s",Eng_Text->stringTable[msg[next]]);
}

void GrenadeCycleUp(u16 p) {
    InventorySystem* inv = Inv(p);
    int last = inv->grenadeCurrent, next = last + 1;
    if (next > 6) next = 0;
    for (int c = 0; c <= 13; c++) {
        if (inv->grenAmmo[next] > 0) break;
        if (c == 13) return;
        if (++next > 6) next = 0;
    }
    if (last == next) return;
    inv->grenadeCurrent = (i8)next;
    static const u16 msg[7] = {579,580,581,582,583,584,585};
    CenterStatusPrint("%s",Eng_Text->stringTable[msg[next]]);
}

void AddGrenadeToInventory(u16 p,int index,int useableIndex) {
    if (index < 0) return;
    InventorySystem* inv = Inv(p);
    bool anyGren = false;
    for (int i = 0; i < 7; i++) if (inv->grenAmmo[i]) { anyGren = true; break; }
    if (!anyGren) inv->grenadeCurrent = (i8)index;
    inv->grenAmmo[index]++;
    inv->grenConstIndex[index] = (i16)useableIndex;
    CenterStatusPrint("%s%s",Eng_Text->stringTable[useableIndex + 326],Eng_Text->stringTable[34]);
}

void RemoveGrenade(u16 p,int index) {
    InventorySystem* inv = Inv(p);
    if (inv->grenAmmo[index] > 0) inv->grenAmmo[index]--;
    if (!inv->grenAmmo[index]) GrenadeCycleDown(p);
}

void CheckForUnreadLogs(u16 p) {
    InventorySystem* inv = Inv(p);
    int em = 0, lg = 0;
    for (int i = TEXT_LOGS_COUNT-1; i >= 0; i--) {
        if (inv->hasLog[i] && !inv->readLog[i]) {
            if (Eng_Text->audioLogType[i] == AudioLogType_Email) em++; else lg++;
        }
    }
    if (!em) inv->hasNewEmail = false;
    if (!lg) inv->hasNewLogs  = false;
}

static int FindNextUnreadLog(u16 p) {
    InventorySystem* inv = Inv(p);
    for (int i = TEXT_LOGS_COUNT-1; i >= 0; i--) {
        if (inv->hasLog[i] && !inv->readLog[i]) return i;
    }
    return -1;
}

static void PlayLog(u16 p,int logIndex) {
    if (logIndex < 0) return;
    InventorySystem* inv = Inv(p);
    if (!(inv->hasHardware & HW_ERD)) return;
//     if (inv->logSoundInited) { SoundStop(&inv->logSound); SoundUninit(&inv->logSound); inv->logSoundInited = false; }
//     if (!SoundInit(sounds[Eng_Text->audioLogSoundIndex[logIndex]],0,NULL,NULL,&inv->logSound)) {
//         SoundSetVolume(&inv->logSound,(float)Eng_Settings->VolumeMessage / 100.0f);
//         SoundStart(&inv->logSound);
//         inv->logSoundInited = true;
//     }
//     inv->readLog[logIndex] = true;
//     if (Eng_Text->audioLogType[logIndex] == AudioLogType_Vmail) {
//         vmailActive        = true;
//         inv->vmailLogIndex = (i16)logIndex; // engine reads to select which .webm to play
//         switch (logIndex) { // TODO
//             case 119:
//                 vmailbetajet.SetActive(true);
//                 fileName = "betajet.webm";
//                 Utils.ConfirmExistsMakeIfNot(basePath,fileName);
//                 urlPath = Utils.SafePathCombine(basePath,fileName);
//                 vmailbetajetVideo.url = urlPath;
//                 vmailbetajetVideo.Play();
//                 break;
//             case 116:
//                 vmailbridgesep.SetActive(true);
//                 fileName = "bridgesep.webm";
//                 Utils.ConfirmExistsMakeIfNot(basePath,fileName);
//                 urlPath = Utils.SafePathCombine(basePath,fileName);
//                 vmailbridgesepVideo.url = urlPath;
//                 vmailbridgesepVideo.Play();
//                 break;
//             case 117:
//                 vmailcitadestruct.SetActive(true);
//                 fileName = "citadestruct.webm";
//                 Utils.ConfirmExistsMakeIfNot(basePath,fileName);
//                 urlPath = Utils.SafePathCombine(basePath,fileName);
//                 vmailcitadestructVideo.url = urlPath;
//                 vmailcitadestructVideo.Play();
//                 break;
//             case 110:
//                 vmailgenstatus.SetActive(true);
//                 fileName = "genstatus.webm";
//                 Utils.ConfirmExistsMakeIfNot(basePath,fileName);
//                 urlPath = Utils.SafePathCombine(basePath,fileName);
//                 vmailgenstatusVideo.url = urlPath;
//                 vmailgenstatusVideo.Play();
//                 break;
//             case 114:
//                 vmaillaserdest.SetActive(true);
//                 fileName = "laserdest.webm";
//                 Utils.ConfirmExistsMakeIfNot(basePath,fileName);
//                 urlPath = Utils.SafePathCombine(basePath,fileName);
//                 vmaillaserdestVideo.url = urlPath;
//                 vmaillaserdestVideo.Play();
//                 break;
//             case 120:
//                 vmailshieldsup.SetActive(true);
//                 fileName = "shieldsup.webm";
//                 Utils.ConfirmExistsMakeIfNot(basePath,fileName);
//                 urlPath = Utils.SafePathCombine(basePath,fileName);
//                 vmailshieldsupVideo.url = urlPath;
//                 vmailshieldsupVideo.Play();
//                 break;
//         }
//    }
    CenterStatusPrint("%s%s",Eng_Text->stringTable[1020],Eng_Global->audiologNames[logIndex]); // "Playing <name>"
    // TODO: SendAudioLogToDataTab(logIndex) — engine-side data tab notification
}

void PlayLastAddedLog(u16 p,int logIndex) {
    if (logIndex < 0) return;
    PlayLog(p,logIndex);
    Inv(p)->lastAddedIndex = -1;
}

void AddAudioLogToInventory(u16 p,int index) {
    if (index < 0) { DualLog("BUG: Audio log picked up has no assigned index (-1)"); return; }
    if (index == 128) { CenterStatusPrint("%s",Eng_Text->stringTable[309]); return; } // Trioptimum Funpack
    InventorySystem* inv = Inv(p);
    inv->hasLog[index]  = true;
    inv->lastAddedIndex = index;
    inv->numLogsFromLevel[Eng_Text->audioLogLevelFound[index]]++;
    if      (Eng_Text->audioLogType[index] == AudioLogType_Email)  inv->hasNewEmail = true;
    else if (Eng_Text->audioLogType[index] == AudioLogType_Normal) inv->hasNewLogs  = true;
    if (inv->hasHardware & HW_ERD) {
        // "Audio log <name> picked up. Press <key> to play." — TODO: key binding name interp
        CenterStatusPrint("%s%s%s",Eng_Text->stringTable[36],Eng_Global->audiologNames[index],Eng_Text->stringTable[38]);
    } else {
        CenterStatusPrint("%s%s%s",Eng_Text->stringTable[36],Eng_Global->audiologNames[index],Eng_Text->stringTable[310]);
    }
}

void PatchCycleDown(u16 p,bool useSound) {
    (void)useSound; // engine plays patch select sound on patchCurrent change
    InventorySystem* inv = Inv(p);
    int next = inv->patchCurrent - 1;
    if (next < 0) next = 6;
    inv->patchCurrent = (i8)next;
    for (int c = 0; c <= 13; c++) {
        if (inv->patchCounts[next] > 0) break;
        if (c == 13) return;
        if (--next < 0) next = 6;
    }
    inv->patchCurrent = (i8)next;
}

void PatchCycleUp(u16 p,bool useSound) {
    (void)useSound;
    InventorySystem* inv = Inv(p);
    int next = inv->patchCurrent + 1;
    if (next > 6) next = 0;
    inv->patchCurrent = (i8)next;
    for (int c = 0; c <= 13; c++) {
        if (inv->patchCounts[next] > 0) break;
        if (c == 13) return;
        if (++next > 6) next = 0;
    }
    inv->patchCurrent = (i8)next;
}

void AddPatchToInventory(u16 p,int index,int constIndex) {
    if (index < 0) return;
    InventorySystem* inv = Inv(p);
    inv->patchCounts[index]++;
    if (!inv->patchCounts[inv->patchCurrent]) inv->patchCurrent = (i8)index;
    CenterStatusPrint("%s%s",Eng_Text->stringTable[constIndex + 326],Eng_Text->stringTable[35]);
}

static i8 GetExistingCyberItemIndex(u16 p) {
    InventorySystem* inv = Inv(p);
    if (inv->softVersions[SW_TURBO]  > 0) return 0;
    if (inv->softVersions[SW_DECOY]  > 0) return 1;
    if (inv->softVersions[SW_RECALL] > 0) return 2;
    return -1;
}

static void UseTurbo(u16 p) {
    InventorySystem* inv = Inv(p);
    if (inv->softVersions[SW_TURBO] <= 0) { inv->hasSoft &= (u8)~(1u << SW_TURBO); return; }
    if (--inv->softVersions[SW_TURBO] == 0) inv->hasSoft &= (u8)~(1u << SW_TURBO);
    if (inv->turboFinished > Eng_Global->pauseRelativeTime) inv->turboFinished += inv->turboCyberTime;
    else                                                    inv->turboFinished = inv->turboCyberTime + Eng_Global->pauseRelativeTime;
}

static void UseDecoy(u16 p) {
    InventorySystem* inv = Inv(p);
    if (Eng_Global->decoyActive) { CenterStatusPrint("%s",Eng_Text->stringTable[537]); return; }
    if (inv->softVersions[SW_DECOY] <= 0) { inv->hasSoft &= (u8)~(1u << SW_DECOY); return; }
    if (--inv->softVersions[SW_DECOY] == 0) inv->hasSoft &= (u8)~(1u << SW_DECOY);
    u16 decoyIdx = SpawnDynamicObject(417,true); // 417 = CyberDecoy constIndex
    if (decoyIdx != U16_MAX) Eng_Global->instances[decoyIdx].position = PE(p)->position;
}

static void UseRecall(u16 p) {
    InventorySystem* inv = Inv(p);
    if (inv->softVersions[SW_RECALL] <= 0) return;
    if (--inv->softVersions[SW_RECALL] == 0) inv->hasSoft &= (u8)~(1u << SW_RECALL);
    PE(p)->position = Eng_Global->cyberspaceRecallPoint;
}

void UseCyberspaceItem(u16 p) {
    InventorySystem* inv = Inv(p);
    if (inv->cyberItemIndex <= 0) {
        inv->cyberItemIndex = GetExistingCyberItemIndex(p);
        if (inv->cyberItemIndex < 0) { CenterStatusPrint("%s",Eng_Text->stringTable[473]); return; }
    }
    switch(inv->cyberItemIndex) {
        case 0: if (!inv->softVersions[SW_TURBO])  { inv->cyberItemIndex = GetExistingCyberItemIndex(p); return; } UseTurbo(p);  break;
        case 1: if (!inv->softVersions[SW_DECOY])  { inv->cyberItemIndex = GetExistingCyberItemIndex(p); return; } UseDecoy(p);  break;
        case 2: if (!inv->softVersions[SW_RECALL]) { inv->cyberItemIndex = GetExistingCyberItemIndex(p); return; } UseRecall(p); break;
    }
}

void CycleCyberSpaceItemUp(u16 p) {
    InventorySystem* inv = Inv(p);
    int next = inv->cyberItemIndex + 1;
    if (next > 2) next = 0;
    for (int c = 0; c <= 7; c++) {
        if (!(inv->hasSoft & (1u << next))) { inv->cyberItemIndex = (i8)next; return; }
        if (c == 7) { inv->cyberItemIndex = -1; return; }
        if (++next > 2) next = 0;
    }
}

void CycleCyberSpaceItemDn(u16 p) {
    InventorySystem* inv = Inv(p);
    int next = inv->cyberItemIndex - 1;
    if (next < 0) next = 2;
    for (int c = 0; c <= 7; c++) {
        if (inv->hasSoft & (1u << next)) { inv->cyberItemIndex = (i8)next; return; }
        if (c == 7) { inv->cyberItemIndex = -1; return; }
        if (--next < 0) next = 2;
    }
}

bool AddSoftwareItem(u16 p,u16 index,int vers) {
    InventorySystem* inv = Inv(p);
    Entity* player       = PE(p);
    float sfxVol         = (float)Eng_Settings->VolumeEffects / 100.0f;
    switch(index) {
        case 450/*item_cyber_drill*/:
            if (inv->isPulserNotDrill && !(inv->hasSoft & (1u << SW_PULSER))) inv->isPulserNotDrill = false;
            if (vers > inv->softVersions[SW_DRILL]) inv->softVersions[SW_DRILL] = (u8)vers;
            else CenterStatusPrint("%s",Eng_Text->stringTable[46]);
            inv->hasSoft |= (1u << SW_DRILL);
            play_wav(sounds[86],sfxVol,(Vector3){},false);
            CenterStatusPrint("%s%d%s",Eng_Text->stringTable[444],inv->softVersions[SW_DRILL],Eng_Text->stringTable[458]);
            return true;
        case 454/*item_cyber_pulser*/:
            if (!inv->isPulserNotDrill && !(inv->hasSoft & (1u << SW_PULSER))) inv->isPulserNotDrill = true;
            if (vers > inv->softVersions[SW_PULSER]) inv->softVersions[SW_PULSER] = (u8)vers;
            else CenterStatusPrint("%s",Eng_Text->stringTable[46]);
            inv->hasSoft |= (1u << SW_PULSER);
            play_wav(sounds[86],sfxVol,(Vector3){},false);
            CenterStatusPrint("%s%d%s",Eng_Text->stringTable[445],inv->softVersions[SW_PULSER],Eng_Text->stringTable[458]);
            return true;
        case 456/*item_cyber_shield*/:
            if (vers > inv->softVersions[SW_SHIELD]) inv->softVersions[SW_SHIELD] = (u8)vers;
            else CenterStatusPrint("%s",Eng_Text->stringTable[46]);
            inv->hasSoft |= (1u << SW_SHIELD);
            play_wav(sounds[86],sfxVol,(Vector3){},false);
            CenterStatusPrint("%s%d%s",Eng_Text->stringTable[446],inv->softVersions[SW_SHIELD],Eng_Text->stringTable[458]);
            return true;
        case 457/*item_cyber_turbo*/:
            if (inv->cyberItemIndex < 0) inv->cyberItemIndex = 0;
            inv->softVersions[SW_TURBO]++;
            inv->hasSoft |= (1u << SW_TURBO);
            play_wav(sounds[86],sfxVol,(Vector3){},false);
            CenterStatusPrint("%s",Eng_Text->stringTable[447]);
            return true;
        case 449/*item_cyber_decoy*/:
            if (inv->cyberItemIndex < 0) inv->cyberItemIndex = 1;
            inv->softVersions[SW_DECOY]++;
            inv->hasSoft |= (1u << SW_DECOY);
            play_wav(sounds[86],sfxVol,(Vector3){},false);
            CenterStatusPrint("%s",Eng_Text->stringTable[448]);
            return true;
        case 455/*item_cyber_recall*/:
            if (inv->cyberItemIndex < 0) inv->cyberItemIndex = 2;
            inv->softVersions[SW_RECALL]++;
            inv->hasSoft |= (1u << SW_RECALL);
            play_wav(sounds[86],sfxVol,(Vector3){},false);
            CenterStatusPrint("%s",Eng_Text->stringTable[449]);
            return true;
        case 451/* ;) item_cyber_game*/: {
            if (vers < 0 || vers >= 7) return false;
            inv->hasNewData  = true;
            inv->hasMinigame |= (u8)(1u << vers);
            static const u16 gameMsg[7] = {450,451,452,453,454,455,456};
            play_wav(sounds[86],sfxVol,(Vector3){},false);
            CenterStatusPrint("%s",Eng_Text->stringTable[gameMsg[vers]]);
            return true;
        }
        case 448/*item_cyber_data*/:
            inv->hasNewData = true;
            if (vers >= 0 && vers < TEXT_LOGS_COUNT) inv->hasLog[vers] = true;
            play_wav(sounds[87],sfxVol,(Vector3){},false);
            CenterStatusPrint("%s",Eng_Text->stringTable[457]);
            return true;
        case 452/*item_cyber_integrity*/:
            if (player->cyberHealth >= 255.0f) return false;
            play_wav(sounds[86],sfxVol,(Vector3){},false);
            player->cyberHealth += 77.0f;
            if (player->cyberHealth > 255.0f) player->cyberHealth = 255.0f;
            // TODO: DrawTicks(true) — HUD cyber health tick refresh
            CenterStatusPrint("%s",Eng_Text->stringTable[459]);
            return true;
        case 453/*item_cyber_keycard*/:
            inv->hasNewData = true;
            if (vers < 0 || vers > 110) vers = 81;
            AddAccessCardToInventory(p,vers);
            return true;
        default: break;
    }
    return false;
}

void RemoveWeapon(u16 p,int slot) { InventorySystem* inv =Inv(p); inv->weaponInventoryIndices[slot]=-1; inv->weaponInventoryAmmoIndices[slot]=-1; }
static float DefaultEnergySettingForWeapon(int wep16Index) { return (wep16Index == 4) ? 5.0f : (wep16Index == 10) ? 13.0f : (wep16Index == 14) ? 2.0f : 3.0f; }
void UpdateAmmoCount(u16 p) {
    InventorySystem* inv = Inv(p);
    inv->numweapons = 0;
    for (int i = 0; i < 7; i++) if (inv->weaponInventoryIndices[i] >= 0) inv->numweapons++;
}

// Returns ammo display string for slot. Writes to caller-provided buffer.
// Engine calls this per-frame for HUD weapon pane text.
void GetWeaponAmmoText(u16 p,int slot,char* buf,size_t bufSize) {
    InventorySystem* inv = Inv(p);
    buf[0] = '\0';
    int wepIdx = inv->weaponInventoryIndices[slot];
    bool alt   = inv->wepLoadedWithAlternate[slot];
    u8 mag  = alt ? inv->currentMagazineAmount2[slot] : inv->currentMagazineAmount[slot];
    float heat   = inv->currentEnergyWeaponHeat[slot];
    switch(wepIdx) {
        case 36: // MK3 Assault Rifle
            if (alt) StringFormat(buf,bufSize,"%upn | %umg, %upn",mag,inv->wepAmmo[0],inv->wepAmmoSecondary[0]);
            else     StringFormat(buf,bufSize,"%umg | %umg, %upn",mag,inv->wepAmmo[0],inv->wepAmmoSecondary[0]);
            break;
        case 37: case 40: case 46: case 50: case 51: // Energy weapons
            StringCopyInto_A_From_B(buf,heat > 80.0f ? Eng_Text->stringTable[14] : Eng_Text->stringTable[15],bufSize);
            break;
        case 38: // SV-23 Dartgun
            if (alt) StringFormat(buf,bufSize,"%utq | %und, %utq",mag,inv->wepAmmo[2],inv->wepAmmoSecondary[2]);
            else     StringFormat(buf,bufSize,"%und | %und, %utq",mag,inv->wepAmmo[2],inv->wepAmmoSecondary[2]);
            break;
        case 39: // AM-27 Flechette
            if (alt) StringFormat(buf,bufSize,"%usp | %uhn, %usp",mag,inv->wepAmmo[3],inv->wepAmmoSecondary[3]);
            else     StringFormat(buf,bufSize,"%uhn | %uhn, %usp",mag,inv->wepAmmo[3],inv->wepAmmoSecondary[3]);
            break;
        case 41: case 42: break; // Laser Rapier / Lead Pipe: no ammo
        case 43: // Magnum 2100
            if (alt) StringFormat(buf,bufSize,"%usg | %uhw, %usg",mag,inv->wepAmmo[7],inv->wepAmmoSecondary[7]);
            else     StringFormat(buf,bufSize,"%uhw | %uhw, %usg",mag,inv->wepAmmo[7],inv->wepAmmoSecondary[7]);
            break;
        case 44: // SB-20 Magpulse
            if (alt) StringFormat(buf,bufSize,"%usu | %ucr, %usu",mag,inv->wepAmmo[8],inv->wepAmmoSecondary[8]);
            else     StringFormat(buf,bufSize,"%ucr | %ucr, %usu",mag,inv->wepAmmo[8],inv->wepAmmoSecondary[8]);
            break;
        case 45: // ML-41 Pistol
            if (alt) StringFormat(buf,bufSize,"%utf | %ust, %utf",mag,inv->wepAmmo[9],inv->wepAmmoSecondary[9]);
            else     StringFormat(buf,bufSize,"%ust | %ust, %utf",mag,inv->wepAmmo[9],inv->wepAmmoSecondary[9]);
            break;
        case 47: StringFormat(buf,bufSize,"%url | %url",inv->currentMagazineAmount[slot],inv->wepAmmo[11]); break; // MM-76 Railgun
        case 48: StringFormat(buf,bufSize,"%urb | %urb",inv->currentMagazineAmount[slot],inv->wepAmmo[12]); break; // DC-05 Riotgun
        case 49: // RF-07 Skorpion
            if (alt) StringFormat(buf,bufSize,"%ulg | %usm, %ulg",mag,inv->wepAmmo[13],inv->wepAmmoSecondary[13]);
            else     StringFormat(buf,bufSize,"%usm | %usm, %ulg",mag,inv->wepAmmo[13],inv->wepAmmoSecondary[13]);
            break;
        default: break;
    }
}

void AddAmmoToInventory(u16 p,int index,int constIndex,int amount,bool isSecondary) {
    if (index < 0) return;
    InventorySystem* inv = Inv(p);
    if (isSecondary) inv->wepAmmoSecondary[index] += (u16)amount;
    else             inv->wepAmmo[index]          += (u16)amount;
    CenterStatusPrint("%s%s",Eng_Text->stringTable[constIndex + 326],Eng_Text->stringTable[630]);
}

bool AddWeaponToInventory(u16 p,int index,int ammo1,int ammo2,bool loadedAlt) {
    if (index < 0) return false;
    InventorySystem* inv = Inv(p);
    for (int i = 0; i < 7; i++) {
        if (inv->weaponInventoryIndices[i] >= 0) continue;
        inv->weaponInventoryIndices[i] = index;
        int index16 = (int)Get16WeaponIndexFromConstIndex(index);
        inv->weaponEnergySetting[i] = DefaultEnergySettingForWeapon(index16);
        if (i == 0) {
            inv->weaponCurrentPending = 0;
            inv->weaponIndexPending   = (u16)index;
            inv->justChangedWeap      = true;
            WeaponFireStartWeaponDip(0.5f);
            WeaponFireCompleteWeaponChange();
        }
        if (loadedAlt && ammo2 > 0) {
            inv->currentMagazineAmount2[i] = (u8)ammo2;
            if (ammo1 > 0) inv->wepAmmo[index16] += (u16)ammo1;
            inv->wepLoadedWithAlternate[i] = true;
        } else {
            inv->currentMagazineAmount[i] = (u8)ammo1;
            if (ammo2 > 0) inv->wepAmmoSecondary[index16] += (u16)ammo2;
            inv->wepLoadedWithAlternate[i] = false;
        }
        CenterStatusPrint("%s%s",Eng_Text->stringTable[index + 326],Eng_Text->stringTable[33]);
        UpdateAmmoCount(p);
        return true;
    }
    return false;
}

void InventoryUpdate(u16 p) {
    InventorySystem* inv = Inv(p);
    if (Grenade()) {
        if (PE(p)->inCyberTube) UseCyberspaceItem(p);
        else if (inv->grenadeCurrent >= 0 && inv->grenadeCurrent < 7 && inv->grenAmmo[inv->grenadeCurrent] > 0) UseGrenade(p,inv->grenConstIndex[inv->grenadeCurrent]);
        else CenterStatusPrint("%s",Eng_Text->stringTable[322]); // Out of grenades.
    }
    
    if (GrenadeCycUp())  { if (PE(p)->inCyberTube) CycleCyberSpaceItemUp(p); else GrenadeCycleUp(p); }
    if (GrenadeCycDown()){ if (PE(p)->inCyberTube) CycleCyberSpaceItemDn(p); else GrenadeCycleDown(p); }
    if (RecentLog() && (inv->hasHardware & HW_ERD)) {
        bool playing = false;//GetSoundIsPlaying(&inv->logSound); TODO
        if (inv->lastAddedIndex >= 0 && !playing) {
            int temp = inv->lastAddedIndex;
            PlayLog(p,temp);
            inv->lastAddedIndex = FindNextUnreadLog(p);
            if (inv->lastAddedIndex == temp) inv->lastAddedIndex = -1;
            CheckForUnreadLogs(p);
        } else {
//             SoundStop(&inv->logSound); TODO
            int temp = inv->lastAddedIndex;
            inv->lastAddedIndex = FindNextUnreadLog(p);
            if (inv->lastAddedIndex == temp) inv->lastAddedIndex = -1;
            CheckForUnreadLogs(p);
            CenterStatusPrint("%s",Eng_Text->stringTable[1019]); // Log playback stopped.
        }
    }

    if (Patch()) {
        if (inv->patchCurrent >= 0 && inv->patchCurrent < 7 && inv->patchCounts[inv->patchCurrent] > 0)
            PatchUse(p,inv->patchCurrent);
        else
            CenterStatusPrint("%s",Eng_Text->stringTable[324]); // Out of patches.
    }
    if (PatchCycUp()) PatchCycleUp(p,true);
    if (PatchCycDown()) PatchCycleDown(p,true);
}

extern u8 magazinePitchCountForWeapon[16];
extern u8 magazinePitchCountForWeapon2[16];
static bool firstTimePickup = true;
static bool firstTimeSearch = true;
// Expects usableItem index
void AddItemFail(u16 p, int index) { DropHeldItem(p); CenterStatusPrint("%s%s%s", Eng_Text->stringTable[32],Eng_Text->stringTable[index + 326],Eng_Text->stringTable[318]); } // Inventory full.
void AddItemToInventory(u16 p, int index, int customIndex) {
    InventorySystem* inv = Inv(p);
    Eng_UI->mouseClickHeldOverGUI = true; // Prevent gun shooting.
    if (index < 0) index = 0; // Good check on paper.
    if (index > 110) index = 94; // Way to get a head.
    if ((index >= 0 && index <= 5) || index == 33 || index == 35 || (index >= 52 && index < 59) || (index >= 61 && index <= 64) || (index >= 92 && index <= 101)) {
        if (!AddGeneralObjectToInventory(p,index,customIndex)) AddItemFail(p,index);
    } else if (index == 6) {
        AddAudioLogToInventory(p,inv->heldObjectCustomIndex);
    } else if (index >= 36 && index <= 51) {
        if (!AddWeaponToInventory(p,index,inv->heldObjectAmmo,inv->heldObjectAmmo2,inv->heldObjectLoadedAlternate)) AddItemFail(p,index);
    } else if (index == 34 || index == 81 || (index >= 83 && index <= 91) || index == 110) AddAccessCardToInventory(p,index);
    else {
        switch (index) {
            case 7:  AddGrenadeToInventory(p,0,index); break; // Frag
            case 8:  AddGrenadeToInventory(p,3,index); break; // Concussion
            case 9:  AddGrenadeToInventory(p,1,index); break; // EMP
            case 10: AddGrenadeToInventory(p,6,index); break; // Earth Shaker
            case 11: AddGrenadeToInventory(p,4,index); break; // Land Mine
            case 12: AddGrenadeToInventory(p,5,index); break; // Nitropak
            case 13: AddGrenadeToInventory(p,2,index); break; // Gas
            case 14: AddPatchToInventory(p,2,index); break;
            case 15: AddPatchToInventory(p,6,index); break;
            case 16: AddPatchToInventory(p,5,index); break;
            case 17: AddPatchToInventory(p,3,index); break;
            case 18: AddPatchToInventory(p,4,index); break;
            case 19: AddPatchToInventory(p,1,index); break;
            case 20: AddPatchToInventory(p,0,index); break;
            case 21: AddHardwareToInventory(p,0,index,customIndex,true); break;
            case 22: AddHardwareToInventory(p,1,index,customIndex,true); break;
            case 23: AddHardwareToInventory(p,2,index,customIndex,true); break;
            case 24: AddHardwareToInventory(p,3,index,customIndex,true); break;
            case 25: AddHardwareToInventory(p,4,index,customIndex,true); break;
            case 26: AddHardwareToInventory(p,5,index,customIndex,true); break;
            case 27: AddHardwareToInventory(p,6,index,customIndex,true); break;
            case 28: AddHardwareToInventory(p,7,index,customIndex,true); break;
            case 29: AddHardwareToInventory(p,8,index,customIndex,true); break;
            case 30: AddHardwareToInventory(p,9,index,customIndex,true); break;
            case 31: AddHardwareToInventory(p,10,index,customIndex,true); break;
            case 32: AddHardwareToInventory(p,11,index,customIndex,true); break;
            case 60: AddAmmoToInventory(p,12,index,magazinePitchCountForWeapon[12],false); break; // rubber slugs
            case 65: AddAmmoToInventory(p,8,index,magazinePitchCountForWeapon2[8],true); break; // magpulse cartridge super
            case 66: AddAmmoToInventory(p,2,index,magazinePitchCountForWeapon[2],false); break; // needle darts
            case 67: AddAmmoToInventory(p,2,index,magazinePitchCountForWeapon2[2],true); break; // tranquilizer darts
            case 68: AddAmmoToInventory(p,9,index,magazinePitchCountForWeapon[9],false); break; // standard bullets
            case 69: AddAmmoToInventory(p,9,index,magazinePitchCountForWeapon2[9],true); break; // teflon bullets
            case 70: AddAmmoToInventory(p,7,index,magazinePitchCountForWeapon[7],false); break; // hollow point rounds
            case 71: AddAmmoToInventory(p,7,index,magazinePitchCountForWeapon2[7],true); break; // slug rounds
            case 72: AddAmmoToInventory(p,0,index,magazinePitchCountForWeapon[0],false); break; // magnesium tipped slugs
            case 73: AddAmmoToInventory(p,0,index,magazinePitchCountForWeapon2[0],true); break; // penetrator slugs
            case 74: AddAmmoToInventory(p,3,index,magazinePitchCountForWeapon[3],false); break; // hornet clip
            case 75: AddAmmoToInventory(p,3,index,magazinePitchCountForWeapon2[3],true); break; // splinter clip
            case 76: AddAmmoToInventory(p,11,index,magazinePitchCountForWeapon[11],false); break; // rail rounds
            case 77: AddAmmoToInventory(p,13,index,magazinePitchCountForWeapon[13],false); break; // slag magazine
            case 78: AddAmmoToInventory(p,13,index,magazinePitchCountForWeapon2[13],true); break; // large slag magazine
            case 79: AddAmmoToInventory(p,8,index,magazinePitchCountForWeapon[8],false); break; // magpulse cartridges
            case 80: AddAmmoToInventory(p,8,index,magazinePitchCountForWeapon2[8],false); break; // small magpulse cartridges
        }
    }

//     Utils.PlayUIOneShotSavable(87); // frob_item    
    firstTimePickup = false;
}
//=============================================================================
// CyberDecoy
void CyberDecoyEnable(u16 self) { (void)self; Eng_Global->decoyActive = true; }
void CyberDecoyDisable(u16 self) { (void)self; Eng_Global->decoyActive = false; }
//=============================================================================
// CyberExit
void CyberExitOnTriggerEnter(u16 self, u16 other) {
    (void)self;
    if (other != PLAYER1) return;
    UIExitCyberspace();
}
//=============================================================================
// CyberDataFragment
void CyberDataFragmentOnTriggerEnter(u16 self, u16 other) {
    Entity* e = &Eng_Global->instances[self];
    if (other != PLAYER1) return;
    UICyberSprint((u16)e->textIndex);
}
//=============================================================================
// CyberItem
void CyberItemInitBeforeLoad(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    if (Eng_Global->difficultyMission == 0 && e->index == 448) flag_set(&e->entflags,EF_ACTIVE,false); // item_cyber_data
}

void CyberItemOnTriggerEnter(u16 self, u16 other) {
    Entity* e = &Eng_Global->instances[self];
    if (other != PLAYER1) return;
    if (!AddSoftwareItem(PLAYER1,e->index,e->version)) return;
    flag_set(&e->entflags,EF_ACTIVE,false);
}
//=============================================================================
// CyberIce
void CyberIceOnTriggerEnter(u16 self, u16 other) {
    (void)self;
    Entity* e = &Eng_Global->instances[other];
    if (!(e->entflags & EF_RIGIDBODY)) return;
    e->layer = 24;
    e->velocity = V3_ScaleByF(e->velocity,-1.0f);
}
//=============================================================================
// CyberMine
void CyberMineInitBeforeLoad(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    e->damage = 55.0f;
    if (Eng_Global->difficultyCyber < 3) { if (random_range(0.0f,1.0f) < 0.2f) flag_set(&e->entflags,EF_ACTIVE,false); e->damage = 33.0f; }
    if (Eng_Global->difficultyCyber < 2) { if (random_range(0.0f,1.0f) < 0.33f) flag_set(&e->entflags,EF_ACTIVE,false); e->damage = 22.0f; }
    if (Eng_Global->difficultyCyber < 1) { if (random_range(0.0f,1.0f) < 0.50f) flag_set(&e->entflags,EF_ACTIVE,false); e->damage = 11.0f; }
}

void CyberMineOnTriggerEnter(u16 self, u16 other) {
    Entity* e = &Eng_Global->instances[self];
    if (other != PLAYER1) return;
    PlayerTakeDamage(PLAYER1,e->damage);
    play_wav(sounds[67],1.0f,e->position,false);
    flag_set(&e->entflags,EF_ACTIVE,false);
}
//=============================================================================
// CyberPush
void CyberPushOnTriggerStay(u16 self, u16 other) {
    Entity* e = &Eng_Global->instances[self];
    Entity* player = &Eng_Global->instances[PLAYER1];
    if (Eng_Global->difficultyCyber < 1 || other != PLAYER1) return;
    player->inCyberTube = true;
    AddForce(PLAYER1,V3_ScaleByF(e->direction,e->force * (float)Eng_Global->deltaTime),false);
    Sys_Music.cyberTube = true;
}

void CyberPushOnTriggerExit(u16 self, u16 other) {
    (void)self;
    if (other != PLAYER1) return;
    
    Eng_Global->instances[other].inCyberTube = false;
    Sys_Music.cyberTube = false;
}
//=============================================================================
// CyberDoor
void CyberDoorOnCollisionEnter(u16 self, u16 other) {
    Entity* e = &Eng_Global->instances[self];
    if (!ConstIndexIsDoor(e->index) || (other != PLAYER1 && other != PLAYER2)) return;
    CenterStatusPrint("%s  %s",Eng_Text->stringTable[e->messageIndex],Eng_Text->stringTable[601]);
}
//=============================================================================
// CyberSwitch
void CyberSwitchInitAfterLoad(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    if (e->iceActive) flag_set(&e->entflags,EF_ACTIVE,true); // TODO Visual subobject parity removed with hierarchy removal.
}

void CyberSwitchOnTriggerEnter(u16 self, u16 other) {
    Entity* e = &Eng_Global->instances[self];
    if (e->active || other != PLAYER1) return;
    UICyberSprint((u16)e->textIndex);
    e->active = true;
    UseTargets(other,e->target);
}
//=============================================================================
// CyberTimer
void CyberTimerInitAfterLoad(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    e->cyberTimer = 600.0f;
    e->timerFinished = Eng_Global->pauseRelativeTime + 1.0;
}

void CyberTimerReset(u16 self, int diff) {
    Entity* e = &Eng_Global->instances[self];
    switch (diff) {
        case 0: e->cyberTimer = 600.0f; break;
        case 1: e->cyberTimer = 300.0f; break;
        case 2: e->cyberTimer = 240.0f; break;
        case 3: e->cyberTimer = 180.0f; break;
    }
}

void CyberTimerUpdate(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    if (e->cyberTimer <= 0.0f) { UIExitCyberspace(); return; }
    if (e->timerFinished >= Eng_Global->pauseRelativeTime) return;
    
    e->cyberTimer -= 1.0f;
    e->minutes = vfloor(e->cyberTimer / 60.0f);
    e->seconds = e->cyberTimer - (e->minutes * 60.0f);
    e->timerFinished = Eng_Global->pauseRelativeTime + 1.0;
}

//=============================================================================
// Ladder
void LadderOnTriggerEnter(u16 self, u16 other) {
    (void)self;
    if (other != PLAYER1) return;

    InventorySystem* inv = Inv(PLAYER1);
    inv->ladderState++;
    if (inv->ladderState < 1) inv->ladderState = 1;
}

void LadderOnTriggerExit(u16 self, u16 other) {
    (void)self;
    if (other != PLAYER1) return;
    
    InventorySystem* inv = Inv(PLAYER1);
    inv->ladderState--;
    if (inv->ladderState < 0) inv->ladderState = 0;
}
//=============================================================================
// SearchFX
void SearchFXResetEnable(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    if (e->itemLifeTime <= 0.0f) e->itemLifeTime = 3.0f;
    e->delayFinished = Eng_Global->pauseRelativeTime + e->itemLifeTime;
}

void SearchFXResetUpdate(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    if (e->delayFinished >= Eng_Global->pauseRelativeTime) return;
    flag_set(&e->entflags,EF_ACTIVE,false);
}
//=============================================================================
// ExplosionLife
void ExplosionLifeInitAfterLoad(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    if (e->tickTime <= 0.0f) e->tickTime = 0.05f;
    if (e->delay <= 0.0f) e->delay = 0.8f;
    e->delayFinished = Eng_Global->pauseRelativeTime + e->delay;
}

void ExplosionLifeUpdate(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    if (!(e->entflags & EF_ACTIVE) || e->delayFinished >= Eng_Global->pauseRelativeTime) return;
    if (e->dontReset) flag_set(&e->entflags,EF_ACTIVE,false);
    else DeleteInstance(self);
}
//=============================================================================
// DelayedSpawn
void DelayedSpawnEnable(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    e->timerFinished = Eng_Global->pauseRelativeTime + e->delay;
    e->active = true;
}

void DelayedSpawnUpdate(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    if (!e->active || e->timerFinished >= Eng_Global->pauseRelativeTime) return;
    
    e->active = false;
    if (!e->doSelfAfterList) return;
    
    if (e->despawnInstead) {
        if (e->destroyAfterListInsteadOfDeactivate) DeleteInstance(self);
        else flag_set(&e->entflags,EF_ACTIVE,false);
    } else flag_set(&e->entflags,EF_ACTIVE,true);
}
//=============================================================================
// FuncWall
void FuncWallInitAfterLoad(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    Vector3 tempVec = V3_AsubB(e->position,e->targetPosition);
    float distTotal = V3_Dist(e->startPosition,e->targetPosition);
    tempVec = V3_ScaleByF(V3_Normalize(tempVec),-1.0f);
    if (e->funcState == FuncStates_AjarMovingTarget) tempVec = V3_ScaleByF(tempVec,distTotal * e->percentAjar);
    else if (e->funcState == FuncStates_AjarMovingStart) tempVec = V3_ScaleByF(tempVec,distTotal * (1.0f - e->percentAjar));
    else if (e->funcState == FuncStates_MovingStart) tempVec = V3_ScaleByF(tempVec,distTotal * (1.0f - e->percentMoved));
    else tempVec = V3_ScaleByF(tempVec,distTotal * e->percentMoved);
    
    SetPosition(e,V3_AplusB(e->position,tempVec),true); // Force it like a teleport
}

void FuncWallMoveStart(u16 self) { Eng_Global->instances[self].funcState = FuncStates_MovingStart; Eng_Global->instances[self].tickFinished = Eng_Global->pauseRelativeTime + 10.0f; }
void FuncWallMoveTarget(u16 self) { Eng_Global->instances[self].funcState = FuncStates_MovingTarget; Eng_Global->instances[self].tickFinished = Eng_Global->pauseRelativeTime + 10.0f; }
void FuncWallTargetted(u16 self, u16 activator) {
    (void)activator;
    Entity* e = &Eng_Global->instances[self];
    if (e->funcState == FuncStates_Start || e->funcState == FuncStates_MovingStart || e->funcState == FuncStates_AjarMovingTarget) FuncWallMoveTarget(self);
    else FuncWallMoveStart(self);
    play_wav(sounds[76],1.0f,e->position,true);
}

void FuncWallUpdate(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    Vector3 goal = e->funcState == FuncStates_MovingStart ? e->startPosition : e->targetPosition;
    FuncStates doneState = e->funcState == FuncStates_MovingStart ? FuncStates_Start : FuncStates_Target;
    if (e->funcState == FuncStates_Start) { SetPosition(e,e->startPosition,true); e->velocity = (Vector3){0.0f,0.0f,0.0f}; e->percentMoved = 0.0f; return; }
    if (e->funcState == FuncStates_Target) { SetPosition(e,e->targetPosition,true); e->velocity = (Vector3){0.0f,0.0f,0.0f}; e->percentMoved = 1.0f; return; }
    if (e->funcState != FuncStates_MovingStart && e->funcState != FuncStates_MovingTarget) return;
    Vector3 delta = V3_AsubB(goal,e->position);
    float distanceLeft = V3_Mag(delta);
    float total = V3_Dist(e->startPosition,e->targetPosition);
    float dist = e->speed * (float)Eng_Global->deltaTime;
    if (distanceLeft <= dist || e->tickFinished < Eng_Global->pauseRelativeTime) {
        SetPosition(e,goal,true);
        e->funcState = doneState;
        e->percentMoved = doneState == FuncStates_Target ? 1.0f : 0.0f;
        e->velocity = (Vector3){0.0f,0.0f,0.0f};
        return;
    }
    if (distanceLeft > 0.0001f) SetPosition(e,V3_AplusB(e->position,V3_ScaleByF(V3_Normalize(delta),dist)),true);
    if (total > 0.0001f) e->percentMoved = V3_Dist(e->startPosition,e->position) / total;
}
//=============================================================================
// ForceBridge
void func_forcebridge(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    e->tickTime = 0.05f;
    e->tickFinished = Eng_Global->pauseRelativeTime + e->tickTime + (double)random_range(0.0f,1.0f);
    e->lerping = true;
    if (e->activatedScale.x <= 0.02f) e->activatedScale.x = 2.56f;
    if (e->activatedScale.y <= 0.02f) e->activatedScale.y = 0.08f;
    if (e->activatedScale.z <= 0.02f) e->activatedScale.z = 2.56f;
    if (!e->active) { e->modelIndex = MODEL_IDX_MAX; e->collider = COLTYPE_NONE; }
    switch (e->fieldColor) {
        case ForceFieldColor_Red:      e->texIndex = 38; break;
        case ForceFieldColor_Green:    e->texIndex = 40; break;
        case ForceFieldColor_Blue:     e->texIndex = 39; break;
        case ForceFieldColor_Purple:   e->texIndex = 41; break;
        case ForceFieldColor_RedFaint: e->texIndex = 198; break;
    }
}

void ForceBridgeActivate(u16 self, bool isSilent) {
    Entity* e = &Eng_Global->instances[self];
    if (e->active) return;
    
    if (!isSilent) play_wav(sounds[102],1.0f,e->position,true);
    e->modelIndex = 78; e->collider = COLTYPE_BOX;
    e->active = e->lerping = true;
    e->scale = (Vector3){ e->forceFieldDirectionX ? 0.1f : e->activatedScale.x, e->forceFieldDirectionY ? 0.1f : e->activatedScale.y, e->forceFieldDirectionZ ? 0.1f : e->activatedScale.z };
}

void ForceBridgeDeactivate(u16 self, bool isSilent) {
    Entity* e = &Eng_Global->instances[self];
    if (!e->active) return;
    
    if (!isSilent) play_wav(sounds[102],1.0f,e->position,true);
    e->active = false; e->lerping = true;
    e->modelIndex = MODEL_IDX_MAX; e->collider = COLTYPE_NONE;
}

void ForceBridgeToggle(u16 self) {
    if (Eng_Global->instances[self].active) ForceBridgeDeactivate(self,false);
    else ForceBridgeActivate(self,false);
}

void ForceBridgeUpdate(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    if (e->tickFinished >= Eng_Global->pauseRelativeTime) return;
    e->tickFinished = Eng_Global->pauseRelativeTime + e->tickTime;
    if (e->active) {
        if (!e->lerping) return;
        float sx = e->forceFieldDirectionX ? lerp(e->scale.x,e->activatedScale.x,e->tickTime * 2.0f) : e->scale.x;
        float sy = e->forceFieldDirectionY ? lerp(e->scale.y,e->activatedScale.y,e->tickTime * 2.0f) : e->scale.y;
        float sz = e->forceFieldDirectionZ ? lerp(e->scale.z,e->activatedScale.z,e->tickTime * 2.0f) : e->scale.z;
        e->scale = (Vector3){sx,sy,sz};
        if (vabs(e->activatedScale.x - sx) < 0.08f && vabs(e->activatedScale.y - sy) < 0.08f && vabs(e->activatedScale.z - sz) < 0.08f) { e->scale = e->activatedScale; e->lerping = false; }
    } else if (e->lerping) {
        float sx = e->forceFieldDirectionX ? lerp(e->scale.x,0.0f,e->tickTime * 2.0f) : e->scale.x;
        float sy = e->forceFieldDirectionY ? lerp(e->scale.y,0.0f,e->tickTime * 2.0f) : e->scale.y;
        float sz = e->forceFieldDirectionZ ? lerp(e->scale.z,0.0f,e->tickTime * 2.0f) : e->scale.z;
        e->scale = (Vector3){sx,sy,sz};
        if (sx < 0.08f || sy < 0.08f || sz < 0.08f) { flag_set(&e->entflags,EF_ACTIVE,false); e->collider = COLTYPE_NONE; e->lerping = false; }
    }
}

//=============================================================================
// TeleportTouch
static u16 TeleportTouch_allTeleportTouches[8];
static bool TeleportTouch_initialized;
void TeleportTouchInitAfterLoad(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    if (!TeleportTouch_initialized) { for (u8 i = 0; i < 8; i++) TeleportTouch_allTeleportTouches[i] = U16_MAX; TeleportTouch_initialized = true; }
    if (e->teleportID >= 8) { DeleteInstance(self); return; }
    TeleportTouch_allTeleportTouches[e->teleportID] = self;
}

void TeleportTouchOnTriggerEnter(u16 self, u16 other) {
    Entity* e = &Eng_Global->instances[self];
    Entity* player = &Eng_Global->instances[PLAYER1];
    if (!e->touchEnabled || other != PLAYER1) return;
    if (player->health <= 0.0f || e->justUsed >= Eng_Global->pauseRelativeTime) return;
    u16 dest = e->targetDestinationID < 8 ? TeleportTouch_allTeleportTouches[e->targetDestinationID] : U16_MAX;
    if (dest == U16_MAX) return;
    player->position = Eng_Global->instances[dest].position;
    Eng_Global->instances[dest].justUsed = Eng_Global->pauseRelativeTime + 1.0;
    play_wav(sounds[106],1.0f,Eng_Global->instances[dest].position,false);
}

//=============================================================================
// Trigger
void TriggerUseTargets(u16 self, u16 activator) { UseTargets(activator,Eng_Global->instances[self].target); }
void TriggerDelayedTarget(u16 self, u16 activator) { Eng_Global->instances[self].delayFireFinished = Eng_Global->pauseRelativeTime + Eng_Global->instances[self].delay; TriggerUseTargets(self,activator); }
void TriggerTriggerTripped(u16 self, u16 other) {
    Entity* e = &Eng_Global->instances[self];
    if (other != PLAYER1 && other != PLAYER2) return;
    if (e->recentMostActivator && e->ignoreSecondaryTriggers) return;
    e->recentMostActivator = other;
    if (e->onlyOnce) e->allDone = true;
    if (e->delay <= 0.0f) TriggerUseTargets(self,other); else TriggerDelayedTarget(self,other);
}

void TriggerOnTriggerEnter(u16 self, u16 other) { if (!Eng_Global->instances[self].allDone) TriggerTriggerTripped(self,other); }
void TriggerOnTriggerStay(u16 self, u16 other) { if (!Eng_Global->instances[self].allDone) TriggerTriggerTripped(self,other); }
void TriggerTargetted(u16 self, u16 activator) { if (Eng_Global->instances[self].ignoreSecondaryTriggers) Eng_Global->instances[self].recentMostActivator = activator; }
//=============================================================================
// TriggerCounter
void TriggerCounterTarget(u16 self, u16 activator) { UseTargets(activator,Eng_Global->instances[self].target); }
void TriggerCounterDelayedTarget(u16 self, u16 activator) { Eng_Global->instances[self].delayFinished = Eng_Global->pauseRelativeTime + Eng_Global->instances[self].delay; TriggerCounterTarget(self,activator); }
void TriggerCounterTargetted(u16 self, u16 activator) {
    Entity* e = &Eng_Global->instances[self];
    e->counter++;
    if (e->counter != e->countToTrigger) return;
    if (e->delay <= 0.0f) TriggerCounterTarget(self,activator); else TriggerCounterDelayedTarget(self,activator);
    if (!e->dontReset) e->counter = 0;
}
//=============================================================================
// TextureChanger
void TextureChangerInitAfterLoad(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    if (!e->currentTexture) return;
    e->texIndex = e->altTexIndex;
    if (e->altGlowIndex < MAX_VALID_TEXTURE) e->glowIndex = e->altGlowIndex;
}

void TextureChangerToggle(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    if (e->currentTexture) {
        e->texIndex = EDefs[e->index].texIndex;
        e->glowIndex = EDefs[e->index].glowIndex;
    } else {
        e->texIndex = e->altTexIndex;
        if (e->altGlowIndex < MAX_VALID_TEXTURE) e->glowIndex = e->altGlowIndex;
    }
    e->currentTexture = !e->currentTexture;
}

//=============================================================================
// GravityLift
void GravityLiftInitAfterLoad(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    if (e->strength <= 0.0f) e->strength = 12.0f;
    if (e->offStrengthFactor <= 0.0f) e->offStrengthFactor = 0.3f;
    if (e->distancePaddingToTopPoint <= 0.0f) e->distancePaddingToTopPoint = 0.32f;
    e->topPoint = (Vector3){ 0.0f, e->position.y + (e->colliderSize.y * 0.5f), 0.0f };
}

// TODO just poll bounds and apply in trigger loop, yeesh
// void GravityLiftOnTriggerExit(u16 self, u16 other) {
//     (void)self;
//     if (other == PLAYER1) Eng_Global->instances[PLAYER1].gravity = 1.0f;
// }
// 
// void GravityLiftOnForce(u16 self, u16 other, bool initial) {
//     Entity* e = &Eng_Global->instances[self];
//     Entity* o = &Eng_Global->instances[other];
//     if (other == PLAYER1) flag_set(&Eng_Global->instances[PLAYER1].entflags,EF_GRAV_LIFT_STATE,true);
//     float topY = e->position.y + (e->colliderSize.y * 0.5f);
//     float dist = topY - o->position.y + 0.48f;
//     float velY = o->velocity.y < 0.0f ? 0.0f : o->velocity.y;
//     if (dist < e->distancePaddingToTopPoint) AddForce(other,(Vector3){0.0f,9.81f - velY,0.0f},false); // TODO accel-vs-force parity
//     else if (o->velocity.y < (e->strength * o->mass)) {
//         float yForce = (e->strength * o->mass) - o->velocity.y;
//         if (initial || e->initialBurstFinished > Eng_Global->pauseRelativeTime) yForce *= 2.0f;
//         AddForce(other,(Vector3){0.0f,yForce,0.0f},false);
//     }
// }
// 
// void GravityLiftOffForce(u16 self, u16 other, bool initial) {
//     Entity* e = &Eng_Global->instances[self];
//     Entity* o = &Eng_Global->instances[other];
//     if (other == PLAYER1) flag_set(&Eng_Global->instances[PLAYER1].entflags,EF_GRAV_LIFT_STATE,true);
//     if (o->velocity.y < e->offStrengthFactor) {
//         float yForce = e->offStrengthFactor - o->velocity.y;
//         if (initial || e->initialBurstFinished > Eng_Global->pauseRelativeTime) yForce *= 2.0f;
//         AddForce(other,(Vector3){0.0f,yForce,0.0f},false);
//     }
// }
// 
// void GravityLiftOnTriggerEnter(u16 self, u16 other) {
//     Eng_Global->instances[self].initialBurstFinished = Eng_Global->pauseRelativeTime + 1.0f;
//     if (Eng_Global->instances[self].active) GravityLiftOnForce(self,other,true);
//     else GravityLiftOffForce(self,other,true);
// }
// 
// void GravityLiftOnTriggerStay(u16 self, u16 other) {
//     if (Eng_Global->instances[self].active) GravityLiftOnForce(self,other,false);
//     else GravityLiftOffForce(self,other,false);
// }

void GravityLiftToggle(u16 self) { Eng_Global->instances[self].active = !Eng_Global->instances[self].active; }

//=============================================================================
// LogicTimer
void LogicTimerInitBeforeLoad(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    if (e->timeInterval <= 0.0f) e->timeInterval = 0.35f;
    if (e->randomMin <= 0.0f) e->randomMin = 5.0f;
    if (e->randomMax <= 0.0f) e->randomMax = 10.0f;
    e->intervalFinished = Eng_Global->pauseRelativeTime + (e->useRandomTimes ? (double)random_range(e->randomMin,e->randomMax) : (double)e->timeInterval);
}

void LogicTimerUseTargets(u16 self) { UseTargets(self,Eng_Global->instances[self].target); }
void LogicTimerUpdate(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    if (!e->active || e->intervalFinished >= Eng_Global->pauseRelativeTime) return;
    e->intervalFinished = Eng_Global->pauseRelativeTime + (e->useRandomTimes ? (double)random_range(e->randomMin,e->randomMax) : (double)e->timeInterval);
    LogicTimerUseTargets(self);
}

void LogicTimerTargetted(u16 self, u16 activator) { (void)activator; Eng_Global->instances[self].active = !Eng_Global->instances[self].active; }
//=============================================================================
// ButtonSwitch
void ButtonSwitchInitAfterLoad(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    e->delayFinished = 0.0f;
    if (e->active) e->tickFinished = Eng_Global->pauseRelativeTime + 1.5 + (double)random_range(0.0f,1.0f);
}

void ButtonSwitchUseTargets(u16 self, u16 activator) {
    Entity* e = &Eng_Global->instances[self];
    DualLog("ButtonSwitchUseTargets, targeting:%s,ioflags:%u\n",e->target,e->ioflags);
    UseTargets(activator,e->target);
    e->active = !e->active;
    e->alternateOn = e->active;
    if (e->changeTexOnActive) {
        e->texIndex = e->alternateOn ? e->altTexIndex : e->mainSwitchMaterial;
        if (e->blinkTexOnActive && e->active) e->tickFinished = Eng_Global->pauseRelativeTime + 1.5f;
    }
}

void ButtonSwitchUse(u16 self, u16 activator) {
    Entity* e = &Eng_Global->instances[self];
    if (Eng_Cheats->superoverride || Eng_Global->difficultyMission == 0) EntitySetLocked(e,false);
    else if (GetCurrentLevelSecurity() > e->securityThreshold) { UIBlockedBySecurity(e->position); return; }
    if ((e->entflags & EF_LOCKED) != 0) {
        CenterStatusPrint("%s",Eng_Text->stringTable[e->lockedMessageLingdex]);
        if (e->SFXLockedIndex >= 0 && e->SFXLockedIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXLockedIndex],1.0f,e->position,true);
        return;
    }
    
    if (e->SFXIndex >= 0 && e->SFXIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXIndex],1.0f,e->position,true);
    CenterStatusPrint("%s",Eng_Text->stringTable[e->messageIndex]);
    if (e->delay > 0.0f) { e->recentMostActivator = activator; e->delayFinished = Eng_Global->pauseRelativeTime + e->delay; }
    else ButtonSwitchUseTargets(self,activator);
}

void ButtonSwitchUpdate(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    if (e->delayFinished > 0.0 && e->delayFinished < Eng_Global->pauseRelativeTime) { e->delayFinished = 0.0; ButtonSwitchUseTargets(self,e->recentMostActivator); }
    if (e->blinkTexOnActive && e->active && e->tickFinished < Eng_Global->pauseRelativeTime) {
        e->alternateOn = !e->alternateOn;
        e->texIndex = e->alternateOn ? e->altTexIndex : e->mainSwitchMaterial;
        e->tickFinished = Eng_Global->pauseRelativeTime + e->tickTime;
    }
}

void ButtonSwitchTargetted(u16 self, u16 activator) { ButtonSwitchUse(self,activator); }
//=============================================================================
// HealingBed
void HealingBedUse(u16 self, u16 owner) {
    Entity* e = &Eng_Global->instances[self];
    if (GetCurrentLevelSecurity() <= (u8)e->minSecurityLevel) {
        if (!e->broken) {
            HealthManagerHealingBed(PLAYER1,e->amount,true);
            CenterStatusPrint("%s",Eng_Text->stringTable[23],owner);
            play_wav(sounds[103],1.0f,e->position,false);
        } else CenterStatusPrint("%s",Eng_Text->stringTable[24],owner);
    } else UIBlockedBySecurity(e->position);
}
//=============================================================================
// TargetIO
void UseTargets(u16 activator, const char* targetname) {
    if (StringIsEmpty(targetname)) return;
    
    bool succeeded = false;
    for (u16 i = START_INDEX_LEVEL_INSTANCES; i < Eng_Global->loadedInstances; i++) {
        if (!StringsEqual(Eng_Global->instances[i].targetname,targetname)) continue;
        
        DualLog("Successfully found matching targetname %s for entity %u and activator ioflags:%u\n",targetname,i,Eng_Global->instances[activator].ioflags);
        Targetted(activator,i);
        succeeded = true;
    }
    if (!succeeded) DualLogWarn("Failed to find a matching targetname for %s\n",targetname);
}

void Targetted(u16 activator, u16 self) {
    Entity* e = &Eng_Global->instances[self];
    Entity* a = &Eng_Global->instances[activator];
    DualLog("Targetted running with a->ioflags:%u, e->index:%u, door conditions:%u\n",a->ioflags,e->index,((a->ioflags & TARG_IOFLAGS_DOOROPEN) && ConstIndexIsDoor(e->index)));
    if (e->index == 709) { CenterStatusPrint("%s",Eng_Text->stringTable[e->messageLingdex]); return; } // info_message
    if (e->index == 708) { Eng_Global->gameFinished = true; return; }
    
    if (e->index == 707 /*info_email*/) EmailTargetted(self,activator);
    if (a->ioflags & TARG_IOFLAGS_TRIPTRIGGER) {
        if (e->index == 598 || e->index == 600) TriggerTargetted(self,activator);
        else if (e->index == 594) TriggerCounterTargetted(self,activator);
    }
    
    if (a->ioflags & TARG_IOFLAGS_UNLOCK) EntitySetLocked(e,false);
    if ((a->ioflags & TARG_IOFLAGS_LOCK) && ConstIndexIsDoor(e->index)) EntitySetLocked(e,true);
    
    if (ConstIndexIsButtonSwitch(e->index)) ButtonSwitchTargetted(self,activator);
    if ((a->ioflags & TARG_IOFLAGS_DOOROPEN) && ConstIndexIsDoor(e->index)) { DualLog("Running DoorForceOpen from ioflag DOOROPEN on entity %u\n",self); DoorForceOpen(self); }
    else if ((a->ioflags & TARG_IOFLAGS_DOOROPENIFUNLOCKED) && ConstIndexIsDoor(e->index) && ((e->entflags & EF_LOCKED) == 0) && (e->requiredAccessCard == AccessCardType_None || (Eng_Global->invP1.accessCardOwned & (1u << e->requiredAccessCard)))) DoorForceOpen(self);
    else if ((a->ioflags & TARG_IOFLAGS_DOORCLOSE) && ConstIndexIsDoor(e->index)) DoorForceClose(self);
    else if (ConstIndexIsDoor(e->index)) DoorTargetted(self,activator);
    
    if (a->ioflags & TARG_IOFLAGS_FBRIDGE_ACTIVATE) ForceBridgeActivate(self,false);
    else if (a->ioflags & TARG_IOFLAGS_FBRIDGE_DEACTIVATE) ForceBridgeDeactivate(self,false);
    else if (a->ioflags & TARG_IOFLAGS_FBRIDGE_TOGGLE) ForceBridgeToggle(self);
    
    if (a->ioflags & TARG_IOFLAGS_GRAVLIFT_TOGGLE) GravityLiftToggle(self);
    if (a->ioflags & TARG_IOFLAGS_TEXTURE_CHG_TOGGLE) TextureChangerToggle(self);
    if (a->ioflags & TARG_IOFLAGS_FUNCWALL_MOVE) FuncWallTargetted(self,activator);
    if (a->ioflags & TARG_IOFLAGS_SWITCH_LOCK_TOGGLE) EntitySetLocked(e,(e->entflags & EF_LOCKED) == 0);
    if (a->ioflags & TARG_IOFLAGS_INST_ACTIVATE) flag_set(&e->entflags,EF_ACTIVE,true);
    else if (a->ioflags & TARG_IOFLAGS_INST_DEACTIVATE) flag_set(&e->entflags,EF_ACTIVE,false);
    else if (a->ioflags & TARG_IOFLAGS_INST_TOGGLE) flag_set(&e->entflags,EF_ACTIVE,!(e->entflags & EF_ACTIVE));
}
//=============================================================================
// VaporizeButton
void VaporizeClick(void) { // TODO
//     Eng_UI->mouseClickHeldOverGUI = true;
//     if (Eng_Global->invP1.generalInvCurrent == 0) return; // Access Cards index.
// 
//     int cur = Eng_Global->invP1.generalInvCurrent;
//     Eng_Global->invP1.generalInventoryIndexRef[cur] = -1; // Remove item
//     Eng_Global->invP1.generalInvCurrent -= 1;
//     if (Eng_Global->invP1.generalInvCurrent < 0) {
//         Eng_Global->invP1.generalInvCurrent = 0; // Bound to lowest, but only
//     }									   // since it is Access Cards.
// 
// 
//     cur = Eng_Global->invP1.generalInvCurrent;
//     if (Eng_Global->invP1.generalInventoryIndexRef[cur] < 0) {
//         for (int i=13; i >= 0; i--) {
//             if (Eng_Global->invP1.generalInventoryIndexRef[i] >= 0) {
//                 Eng_Global->invP1.generalInvCurrent = i;
//                 break; // Found last item in inventory.
//             }
//         }
//     }
// 
//     cur = Eng_Global->invP1.generalInvCurrent;
//     int indexRef = Eng_Global->invP1.generalInventoryIndexRef[cur];
//     if (Eng_Global->invP1.generalInvCurrent == 0) {
//         if (Eng_Global->invP1.HasAnyAccessCards()) {
//             Eng_UI->SendInfoToItemTab(indexRef);
//         } else {
//             // If no access cards, reset item tab to show nothing.
//             Eng_UI->SendInfoToItemTab(-1);
//             PtrExit();
//         }
//     } else {
//         GeneralInvButton genbut = Eng_Global->invP1.genButtons[cur].GetComponent<GeneralInvButton>();
//         Eng_UI->SendInfoToItemTab(indexRef,genbut.customIndex);
//     }
}
//=============================================================================
// AmmoIconManager
#define AMMO_ICON_NONE    -1
#define AMMO_ICON_ENERGY  -2

typedef struct { i8 norm,alt; } AmmoIconEntry;
static const AmmoIconEntry ammoIconTable[51] = {
    [36-36] = { 7,  8  }, // MK3 Magnesium / Penetrator
    [37-36] = {AMMO_ICON_ENERGY, AMMO_ICON_ENERGY},
    [38-36] = { 0,  1  }, // Dartgun Needle / Tranq
    [39-36] = { 9,  10 }, // Flechetter Hornette / Splinter
    [40-36] = {AMMO_ICON_ENERGY, AMMO_ICON_ENERGY},
    [41-36] = {AMMO_ICON_NONE, AMMO_ICON_NONE},  // Rapier, no ammo
    [42-36] = {AMMO_ICON_NONE, AMMO_ICON_NONE},  // Pipe, no ammo
    [43-36] = { 5,  6  }, // Magnum Hollow / Slug
    [44-36] = { 11, AMMO_ICON_NONE }, // Magpulse Magcart
    [45-36] = { 2,  3  }, // Pistol Standard / Teflon
    [46-36] = {AMMO_ICON_ENERGY, AMMO_ICON_ENERGY},
    [47-36] = { 14, AMMO_ICON_NONE }, // Railgun Rail Round
    [48-36] = { 4,  AMMO_ICON_NONE }, // Riotgun Rubber
    [49-36] = { 12, 13 }, // Skorpion Slag / Large Slag
    [50-36] = {AMMO_ICON_ENERGY, AMMO_ICON_ENERGY},
    [51-36] = {AMMO_ICON_ENERGY, AMMO_ICON_ENERGY},
};

// Returns the ammo icon sprite index for the current weapon slot.
// AMMO_ICON_ENERGY = render energy bar UI instead of icon/border.
// AMMO_ICON_NONE   = render nothing (no ammo type for this weapon).
// >= 0             = index into ammIcons sprite array for UI renderer.
i8 AmmoIconGet(int index,bool alt) {
    if (index < 36 || index > 51) return AMMO_ICON_NONE;
    const AmmoIconEntry* e = &ammoIconTable[index - 36];
    return alt ? e->alt : e->norm;
    // TODO: trigger immediate-mode UI redraw of weapon pane border,
    // icon visibility, energySlider, energyHeatTicks, energyOverloadButton
    // based on return value (engine-side UI rendering concern).
}
//=============================================================================
// CreditsScroll
// Video text phases: 0=text1 visible, 1=text2 visible, 2=text3 visible, 3=all hidden
static double creditsVidStartTime = 0.0;
static double creditsVidFinished  = 0.0;
static u8 creditsVidPhase    = 0;

void CreditsOnEnable(void) {
    Eng_Global->creditsActive    = true;
    Eng_Global->creditsPageIndex = 0;
    creditsVidStartTime          = Eng_Global->absoluteTime;
    creditsVidFinished           = Eng_Global->absoluteTime + 37.2;
    creditsVidPhase              = 0;
    // TODO: start outro.webm video playback via engine video player
    // TODO: render Eng_Text->stringTable[610] as endVideoText1 (phase 0)
    // TODO: render Eng_Text->stringTable[611] as endVideoText2 (phase 1)
    // TODO: render Eng_Text->stringTable[612] as endVideoText3 (phase 2)
}

void CreditsUpdate(void) {
    if (!Eng_Global->creditsActive) return;
    double elapsed = Eng_Global->absoluteTime - creditsVidStartTime;

    // Drive video text phase transitions
    if (creditsVidFinished > 0.0) {
        if (elapsed >  7.0 && creditsVidPhase == 0) creditsVidPhase = 1; // TODO: swap text1->text2 visibility
        if (elapsed > 11.0 && creditsVidPhase == 1) creditsVidPhase = 2; // TODO: swap text2->text3 visibility
        if (elapsed > 14.0 && creditsVidPhase == 2) creditsVidPhase = 3; // TODO: hide text3
        if (Eng_Global->absoluteTime >= creditsVidFinished) {
            creditsVidFinished = 0.0;
            creditsVidPhase    = 3;
            // TODO: deactivate exitVideo overlay and all text phases
        }
    }

    if (Menu()) { // Escape
        if (creditsVidFinished > 0.0) { creditsVidFinished = 0.0; return; } // skip video
        MenuGoBack();
        return;
    }

    if (creditsVidFinished > 0.0) return; // absorb all click input while video playing

    if (Attack()) { // left click — advance
        if (!(Eng_Global->creditsPageIndex >= CREDITS_PAGES)) {
            ++Eng_Global->creditsPageIndex;
            if (!Eng_Global->gameFinished && Eng_Global->creditsPageIndex == 1) ++Eng_Global->creditsPageIndex; // skip stats page when not finishing game
            if (Eng_Global->creditsPageIndex >= CREDITS_PAGES) Eng_Global->creditsPageIndex = CREDITS_PAGES; // bottom
        } else {
            Eng_Global->creditsActive = false;
            MenuGoBack();
        }
        return;
    }

    if (ToggleMode()) { // right click — go back a page
        if (Eng_Global->creditsPageIndex > 0) --Eng_Global->creditsPageIndex;
    }
}
//=============================================================================
// CyberWall
#define CYBERWALL_TICK          0.05
#define CYBERWALL_CONWAY_TIME   0.5
#define CYBERWALL_ALPHA_MIN     0.02f
#define CYBERWALL_ALPHA_MAX     1.0f
#define CYBERWALL_ALPHA_STEP    0.05f
#define CYBERWALL_INIT_DELAY    2.0
// volume  = centerAlphaCurrent
// tickFinished    = next alpha decay tick (already on Entity)
// animSwapFinished = conwayFinished (already on Entity)
void CyberWallInitAfterLoad(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    e->volume        = CYBERWALL_ALPHA_MIN;
    e->tickFinished  = Eng_Global->pauseRelativeTime + CYBERWALL_INIT_DELAY;
    e->animSwapFinished = 0.0;
    // TODO: push e->volume to chunk_frag.glsl as _CenterAlpha uniform or
    // per-instance draw param for this geometry instance's material slot
}

void CyberWallUpdate(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    if (Eng_Global->pauseRelativeTime < e->tickFinished) return;
    if (e->volume > CYBERWALL_ALPHA_MIN) {
        e->volume -= CYBERWALL_ALPHA_STEP;
        if (e->volume < CYBERWALL_ALPHA_MIN) e->volume = CYBERWALL_ALPHA_MIN;
    }
    e->tickFinished = Eng_Global->pauseRelativeTime + CYBERWALL_TICK;
}

void CyberWallHit(u16 self) { // Called by projectile hit, collision, or ConwaySignal propagation from adjacent wall
    Entity* e = &Eng_Global->instances[self];
    e->volume = CYBERWALL_ALPHA_MAX;
    // TODO: push e->volume to renderer as _CenterAlpha for this instance
}

void CyberWallConwaySignal(u16 self) { // Called when a conway propagation signal arrives from a neighbour TODO Conway's game of life propagation on world x,z plane
    Eng_Global->instances[self].animSwapFinished = Eng_Global->pauseRelativeTime + CYBERWALL_CONWAY_TIME;
}
//=============================================================================
// CyborgConversionToggle
void CyborgConversionToggleTargetted(void) {
    bool active = (Eng_Global->ressurectionActiveLevels >> Eng_Global->currentLevel) & 1u;
    flag_setu16(&Eng_Global->ressurectionActiveLevels,(1u << Eng_Global->currentLevel),!active);
    if (Eng_Global->currentLevel == 6) flag_setu16(&Eng_Global->ressurectionActiveLevels, (1u<<10|1u<<11|1u<<12),!active); // Set groves 10,11,12 when 6 gets toggled as they don't have their own switch
    play_wav(sounds[active ? 183 : 184],Eng_Settings->VolumeMessage,(Vector3){},false); // "vox_cybconvcancelled" : "vox_cybconvenabled"
    CenterStatusPrint("%s",Eng_Text->stringTable[active ? 591 : 592]);
}
//=============================================================================
// ElevatorButton
// static const char* elevFloorLabels[14] = {"R","1","2","3","4","5","6","7","8","9","G1","G2","G4","C"};
void ElevatorButtonClick(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    Eng_UI->mouseClickHeldOverGUI = true;
    if (Eng_UI->linkedElevatorDoor == U16_MAX) {
        CenterStatusPrint("%s",Eng_Text->stringTable[6]); // Too far away from that.
        return;
    }
    Entity* door = &Eng_Global->instances[Eng_UI->linkedElevatorDoor];
    bool doorClosed = door->doorOpen == DoorState_Closed;
    float dist = V3_Dist(Eng_UI->objectInUsePos,Eng_Global->instances[PLAYER1].position);
    if (dist > ELEVATOR_PAD_TETHER_DIST && !doorClosed) {
        CenterStatusPrint("%s",Eng_Text->stringTable[6]); // Too far away from that.
        return;
    }
    if (!doorClosed) {
        CenterStatusPrint("%s",Eng_Text->stringTable[7]); // Door not closed.
        return;
    }
    if (!(e->entflags & EF_ACTIVE)) {
        CenterStatusPrint("%s",Eng_Text->stringTable[8]); // Floor not accessible.
        return;
    }
    // TODO: call engine LoadLevel(e->teleportID, spawnPos) — destination spawn
    // position comes from instance[e->targetDestinationID].position if set
    // (targetDestinationID != U16_MAX), else Vector3 zero.
    // LoadLevel is engine-side level transition, not yet in interop.h.
//     if (floorAccessible) { // floorAccessible set from the us_puz_elevatorkeypad, us_puz_elevatorkeypad2, us_puz_elevatorkeypad3, or us_puz_elevatorkeypad4 entity
//         LoadLevel(levelIndex,targetDestination.Eng_Global->instances[i].position);
//     } else {
//         CenterStatusPrint("%s", Eng_Text->stringTable[8]);
//     }
}
//=============================================================================
// Email
void EmailTargetted(u16 self, u16 activator) {
    (void)activator; Entity* e = &Eng_Global->instances[self]; u16 idx = e->emailIndex;
    InventorySystem* inv = &Eng_Global->invP1;
    if (inv->hasLog[idx]) return;
    
    inv->hasLog[idx] = inv->hasNewEmail = true; inv->lastAddedIndex = idx;
    if (Eng_Text->audioLogType[idx] == AudioLogType_Email) inv->beepDone = true;
    if (e->autoPlayEmail) (void)0; // TODO: PlayLastAddedLog(idx) — trigger auto-play of log audio
}
//=============================================================================
// EnergyOverloadButton
#define OVERLOAD_CLICK_DEBOUNCE 0.4
#define OVERLOAD_HEAT_THRESHOLD 25.0f
static double overloadClickFinished = 0.0;

void OverloadButtonAction(void) {
    if (overloadClickFinished >= Eng_Global->pauseRelativeTime) return;
    overloadClickFinished = Eng_Global->pauseRelativeTime + OVERLOAD_CLICK_DEBOUNCE;
    InventorySystem* inv = &Eng_Global->invP1;
    if (inv->currentEnergyWeaponHeat[inv->weaponIndex] > OVERLOAD_HEAT_THRESHOLD) {
        CenterStatusPrint("%s",Eng_Text->stringTable[12]); // Weapon too hot
        return;
    }
    if (inv->overloadEnabled) {
        CenterStatusPrint("%s",Eng_Text->stringTable[13]); // Overload disabled
        inv->overloadEnabled = false;
        // Render-time: normalButtonSprite, textClickableColor, stringTable[16]
    } else {
        CenterStatusPrint("%s",Eng_Text->stringTable[17]); // Overload enabled
        inv->overloadEnabled = true;
        // Render-time: overloadButtonSprite, textOverloadColor, stringTable[18]
    }
}

void OverloadEnergyClick(void) {
    Eng_UI->mouseClickHeldOverGUI = true;
    OverloadButtonAction();
}

// Called by weapon fire system after overload shot discharges
void OverloadFired(void) {
    Eng_Global->invP1.overloadEnabled = false;
    // Render-time: normalButtonSprite, textDisabledColor
}

// Called from weapon pane render — returns visual state for renderer to act on
// 0 = normal+clickable, 1 = overloaded, 2 = disabled (post-fire/too hot)
u8 OverloadButtonVisualState(void) {
    InventorySystem* inv = &Eng_Global->invP1;
    if (inv->currentEnergyWeaponHeat[inv->weaponIndex] > OVERLOAD_HEAT_THRESHOLD) return 2;
    if (inv->overloadEnabled) return 1;
    return 0;
}
//=============================================================================
// GeneralInventory
static void ApplyBattery(void) {
    InventorySystem* inv = Inv(PLAYER1);
    if (inv->energy >= 255.0f) { CenterStatusPrint("%s",Eng_Text->stringTable[303]); return; } // Energy full
    
    GiveEnergy(83.0f,EnergyType_Battery);
    inv->generalInventoryIndexRef[inv->hardwareInvCurrent] = -1;
}

static void ApplyIcadBattery(void) {
    InventorySystem* inv = Inv(PLAYER1);
    if (inv->energy >= 255.0f) { CenterStatusPrint("%s",Eng_Text->stringTable[303]); return; } // Energy full
    
    GiveEnergy(255.0f,EnergyType_Battery);
    inv->generalInventoryIndexRef[inv->hardwareInvCurrent] = -1;
}

static void ApplyHealthkit(void) {
    InventorySystem* inv = Inv(PLAYER1);
    if (inv->energy >= 255.0f) { CenterStatusPrint("%s",Eng_Text->stringTable[303]); return; } // Energy full
    
    Eng_Global->instances[PLAYER1].health = 255.0f;
    // TODO: Eng_UI->DrawTicks(true) — HUD health tick refresh, MFDManager
    inv->generalInventoryIndexRef[inv->hardwareInvCurrent] = -1;
}

void GeneralInvUse(int buttonIdx,int customIdx) {
    InventorySystem* inv = Inv(PLAYER1);
    inv->hardwareInvCurrent = buttonIdx;
    int itemIdx = inv->generalInventoryIndexRef[buttonIdx];
    if (buttonIdx == 0) {
        // TODO: Eng_UI->SendInfoToItemTab(81) — access cards display, MFDManager
        // TODO: SetCurrentAsLast for active side panel, MFDManager
        return;
    }
    // TODO: Eng_UI->SendInfoToItemTab(itemIdx,customIdx) — MFDManager
    // TODO: SetCurrentAsLast for active side panel, MFDManager
    (void)customIdx;
    (void)itemIdx;
}

void GeneralInvApply(int buttonIdx,int customIdx) {
    if (buttonIdx == 0) {
        // TODO: Eng_UI->SendInfoToItemTab(81), OpenTab access cards — MFDManager
        return;
    }
    Eng_Global->invP1.hardwareInvCurrent = buttonIdx;
    int itemIdx = Eng_Global->invP1.generalInventoryIndexRef[buttonIdx];
    switch (itemIdx) {
        case 52: ApplyBattery();     break;
        case 53: ApplyIcadBattery(); break;
        case 55: ApplyHealthkit();   break;
        default:
            Eng_Global->invP1.hardwareInvCurrent = buttonIdx;
            // TODO: Eng_UI->SendInfoToItemTab(itemIdx,customIdx), OpenTab — MFDManager
            (void)customIdx;
            break;
    }
}

void GeneralInvClick(int buttonIdx,int customIdx) { Eng_UI->mouseClickHeldOverGUI = true; GeneralInvUse(buttonIdx,customIdx); }
void GeneralInvDoubleClick(int buttonIdx,int customIdx) { Eng_UI->mouseClickHeldOverGUI = true; GeneralInvApply(buttonIdx,customIdx); }
//=============================================================================
// TargetID
#define TARGETID_LINK_DIST       10.0f
#define TARGETID_DAMAGE_TIME_HIT  2.5f
#define TARGETID_DAMAGE_TIME_MISS 1.0f

// Display flags packed into ioflags (upper bits unused by I/O system on dynamic objs)
#define TARGID_DISPLAY_HEALTH  (1ull << 60)
#define TARGID_DISPLAY_RANGE   (1ull << 61)
#define TARGID_DISPLAY_ATTITUD (1ull << 62)
#define TARGID_DISPLAY_NAME    (1ull << 63)

float TargetIDGetSensingRange(bool manual) {
    u8 ver = Eng_Global->invP1.hardwareVersion[HW_TID_IDX];
    if (manual) return (ver >= 4) ? 18.0f : 13.0f;
    return (ver <= 2) ? 0.0f : ((ver == 3) ? 13.0f : 20.0f);
}

float TargetIDGetTetherRange(void) { return (Eng_Global->invP1.hardwareVersion[HW_TID_IDX] >= 4) ? 22.0f : 15.0f; }
static void TargetIDDeactivate(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    if (e->enemy != WORLD) {
        Entity* npc = &Eng_Global->instances[e->enemy];
        flag_set(&npc->entflags,EF_TARGID_ATTACHED,false);
        e->enemy = NULLENT;
    }
    e->textIndex  = -1;
    flag_set(&e->entflags,EF_ACTIVE,false);
}

void TargetIDSendDamageReceive(u16 self,float damage,AttackType attackType) {
    Entity* e   = &Eng_Global->instances[self];
    if (e->enemy == NULLENT) return;
    Entity* npc = &Eng_Global->instances[e->enemy];
    if (attackType == AttackType_Tranq) {
        e->textIndex         = 536; // STUNNED
        e->animSwapFinished  = Eng_Global->pauseRelativeTime - 1.0; // expire damage text
    } else {
        float mh = npcTable[npc->index - 419].health;
        if      (damage > mh * 0.75f) e->textIndex = 514; // SEVERE DAMAGE
        else if (damage > mh * 0.50f) e->textIndex = 515; // MAJOR DAMAGE
        else if (damage > mh * 0.25f) e->textIndex = 513; // NORMAL DAMAGE
        else if (damage > 0.0f)       e->textIndex = 512; // MINOR DAMAGE
        else                          e->textIndex = 511; // NO DAMAGE
        e->animSwapFinished = Eng_Global->pauseRelativeTime
                              + ((damage == 0.0f) ? TARGETID_DAMAGE_TIME_MISS
                                                  : TARGETID_DAMAGE_TIME_HIT);
    }
}

void TargetIDUpdate(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    if (!(e->entflags & EF_ACTIVE)) return;

    // Deactivation checks
    if (e->enemy == NULLENT) { TargetIDDeactivate(self); return; }
    Entity* npc = &Eng_Global->instances[e->enemy];
    if (npc->health <= 0.0f) { TargetIDDeactivate(self); return; }
    if (V3_Dist(e->position,Eng_Global->instances[PLAYER1].position) > TARGETID_LINK_DIST) { TargetIDDeactivate(self); return; }
    if (e->tickFinished < Eng_Global->pauseRelativeTime) { TargetIDDeactivate(self); return; }

    SetPosition(e,npc->position,true); // Track parent NPC position
    bool stunned = npc->tranquilizeFinished > Eng_Global->pauseRelativeTime;
    flag_set(&e->entflags,EF_ASLEEP,stunned);
    if (e->textIndex >= 0) {
        if (stunned && e->animSwapFinished < Eng_Global->pauseRelativeTime) e->textIndex = 536; // STUNNED
        else if (e->animSwapFinished < Eng_Global->pauseRelativeTime) {
            e->textIndex = -1;
            if (!(Eng_Global->invP1.hasHardware & HW_TID)) { TargetIDDeactivate(self); return; }
        }
    }

    // Secondary display string — built at render time from flags + npc state
    // TODO: render TargetID billboard text using e->textIndex, e->enemy,
    // e->ioflags TARGID_DISPLAY_* flags — pass to HUD/world-space text renderer:
    //   TARGID_DISPLAY_NAME    → npc->targetID string
    //   TARGID_DISPLAY_HEALTH  → vfloor(npc->health)
    //   TARGID_DISPLAY_RANGE   → V3_Dist(PLAYER1.pos, npc->pos)
    //   TARGID_DISPLAY_ATTITUD → map npc->currentState to stringTable indices:
    //     EF_ASLEEP                             → 519 Asleep
    //     Run/Attack1/Attack2/Attack3/Pain           → 518 Hostile
    //     Walk/Inspect/Interacting                   → 517 Cautious
    //     default                                    → 516 Idle
}

void TargetIDInitAfterLoad(u16 self) {
    Entity* e        = &Eng_Global->instances[self];
    e->textIndex     = -1;
    e->enemy        = NULLENT;
    e->tickFinished  = 0.0;
    e->animSwapFinished = 0.0;
    flag_set(&e->entflags,EF_ACTIVE,false); // starts pooled
}
//=============================================================================
// PlayerEnergy
#define ENERGY_TICK 0.1

static const float hwDrain[12][4] = {
    [3]  = { 0.01535f, 0.03413f, 0.02559f, 0.0f    },
    [5]  = { 0.04096f, 0.10239f, 0.17919f, 0.05119f},
    [6]  = { 0.001706f,0.0f,     0.0f,     0.0f    },
    [7]  = { 0.02559f, 0.04266f, 0.05119f, 0.0f    },
    [9]  = { 0.0f,     0.02f,    0.015f,   0.0f    },
    [11] = { 0.08533f, 0.0f,     0.0f,     0.0f    },
};
static const u16 hwDrainJPM[12][4] = {
    [3]  = {  9, 20, 15,   0},
    [5]  = { 24, 60,105,  30},
    [6]  = {  1,  0,  0,   0},
    [7]  = { 15, 25, 30,   0},
    [9]  = {  0, 16, 12,   0},
    [11] = { 50,  0,  0,   0},
};

static void TargetIdentifierSenseTargets(void) {
    for (u16 i = START_INDEX_LEVEL_INSTANCES; i < Eng_Global->loadedInstances; i++) {
        Entity* e = &Eng_Global->instances[i];
        if (!(e->entflags & EF_ACTIVE))         continue;
        if (!ConstIndexIsNPC(e->index))              continue;
        if (e->entflags & EF_DEAD)              continue;
        if (e->entflags & EF_TARGID_ATTACHED)   continue;
        if (V3_Dist(e->position,Eng_Global->instances[PLAYER1].position) > TargetIDGetSensingRange(false))        continue;
        // TODO: CreateTargetIDInstance — weapon/targetting system
    }
}

MOD_TO_ENGINE void SetModFatigue(float val) { Eng_Global->invP1.fatigue = val; }
MOD_TO_ENGINE bool ModRequestsGrayscale(void) {return (/*(Eng_Global->invP1.hasHardware & HW_INF) && */(Eng_Global->invP1.hardwareIsActive & HW_INF) > 0); }
static void DeactivateHardwareOnEnergyDepleted(void) {
    InventorySystem* inv = &Eng_Global->invP1;
    u16* active = &inv->hardwareIsActive;
    flag_set((u32*)active, HW_SNS, false);
    // TODO: SensaroundOff() — hardware button manager effects
    if ((*active & HW_BIO) && inv->hardwareVersionSetting[HW_BIO_IDX] == 0) {
        flag_set((u32*)active, HW_BIO, false);
        // TODO: BioOff()
    }
    if (*active & HW_SHD) {
        flag_set((u32*)active, HW_SHD, false);
        // TODO: ShieldOffWithEffects()
    }
    if (*active & HW_LAN) {
        flag_set((u32*)active, HW_LAN, false);
        // TODO: LanternOff()
    }
    if (*active & HW_BST) {
        flag_set((u32*)active, HW_BST, false);
        // TODO: BoosterOff()
    }
    if (*active & HW_INF) {
        flag_set((u32*)active, HW_INF, false);
        // TODO: InfraredOff()
    }
}

void TakeEnergy(float take) {
    InventorySystem* inv = Inv(PLAYER1);
    if (inv->energy <= 0.0f)          return;
    if (Eng_Cheats->redbull)        return;
    inv->energy -= take;
    if (inv->energy <= 0.0f) {
        inv->energy = 0.0f;
        play_wav(sounds[84],Eng_Settings->VolumeEffects,(Vector3){},false); // energy_gone
        CenterStatusPrint("%s",Eng_Text->stringTable[314]); // Power supply exhausted.
        DeactivateHardwareOnEnergyDepleted();
    }
}

void GiveEnergy(float give,EnergyType type) {
    InventorySystem* inv = Inv(PLAYER1);
    inv->energy += give;
    if (inv->energy > 255.0f) inv->energy = 255.0f;
    if (type == EnergyType_Battery)       play_wav(sounds[79], Eng_Settings->VolumeEffects,(Vector3){},false); // batteryuse
    if (type == EnergyType_ChargeStation) play_wav(sounds[100],Eng_Settings->VolumeEffects,(Vector3){},false); // chargingstation
}

void PlayerEnergyInit(void) {
    InventorySystem* inv = Inv(PLAYER1);
    inv->energy = 54.0f;
    inv->energyDrainTickFinished = Eng_Global->pauseRelativeTime + ENERGY_TICK + random_range(0.0f,1.0f);
    inv->drainJPM = 0;
}

void PlayerEnergyUpdate(void) {
    InventorySystem* inv = &Eng_Global->invP1;
    if (inv->hasHardware & HW_TID) TargetIdentifierSenseTargets();
    if (inv->energyDrainTickFinished > Eng_Global->pauseRelativeTime) return;
    
    inv->energyDrainTickFinished = Eng_Global->pauseRelativeTime + ENERGY_TICK;
    bool anyDrain = false; u8 ver; inv->drainJPM = 0;
    for (int hw = 3; hw <= 11; hw++) {
        u16 bit = (u16)(1u << hw);
        if (!(inv->hardwareIsActive & bit)) continue;
        if (hw == 4 || hw == 8 || hw == 10) continue; // No energy usage
        
        ver = inv->hardwareVersionSetting[hw];
        float drain = hwDrain[hw][ver];
        inv->drainJPM += hwDrainJPM[hw][ver];
        if (drain > 0.0f) { TakeEnergy(drain); anyDrain = true; }
    }
    
    if (anyDrain && inv->energy <= 0.0f) { DeactivateHardwareOnEnergyDepleted(); inv->drainJPM = 0; } // Depleted
}
//=============================================================================
// GrenadeActivate
static bool GrenadeIsNPCMine(u16 self) { return Eng_Global->instances[self].layer != Layer_PlayerBullets; }
void GrenadeExplode(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    // TODO: DamageData + ApplyImpactForceSphere(damage,attackType,penetration,offense,damage*1.5f,e->position,e->strength,1.0f)
    if (!GrenadeIsNPCMine(self)) { Entity* p = &Eng_Global->instances[PLAYER1]; p->noiseFinished = Eng_Global->pauseRelativeTime + 2.0; }
    i16 idx = (i16)e->index;
    int soundIndex = 60;
    switch (idx) {
        case 7: case 11: soundIndex = 64; Eng_Global->fogFac += 5;  break; // frag, mine
        case 8: case 10: soundIndex = 60; Eng_Global->fogFac += 7;  break; // conc, earth
        case 9:  soundIndex = 67;                           break; // emp
        case 12: soundIndex = 60; Eng_Global->fogFac += 6;  break; // nitro
        case 13: soundIndex = 63; Eng_Global->fogFac += 10; break; // gas
    }
    
    play_wav(sounds[soundIndex],1.0f,e->position,true);
    // TODO: SpawnExplosionEffect(e->position, explosionType)
    // TODO: Shake(-1,-1) — screen shake system
    DeleteInstance(self);
}

void GrenadeActivate(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    i16 idx = (i16)e->index;
    switch (idx) {
//         case 7: case 8: case 9: flag_set(&e->ioflags,GREN_FLAG_EXPLODE_CONTACT,true); break;
//         case 10: e->timerFinished = Eng_Global->pauseRelativeTime + Eng_Global->invP1.earthShakerTimeSetting; flag_set(&e->ioflags,GREN_FLAG_USE_TIMER,true); break;
//         case 11: flag_set(&e->ioflags,GREN_FLAG_USE_PROX,true); flag_set(&e->ioflags,GREN_FLAG_EXPLODE_CONTACT,false);                                        break;
//         case 12: e->timerFinished = Eng_Global->pauseRelativeTime + Eng_Global->invP1.nitroTimeSetting; flag_set(&e->ioflags,GREN_FLAG_USE_TIMER,true);       break;
//         case 13: flag_set(&e->ioflags,GREN_FLAG_EXPLODE_CONTACT,true); break;
        default: return;
    }
}

void GrenadeUpdate(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    if ((i16)e->index == 14) { GrenadeExplode(self); return; } // Plastique
    
//     if ((e->ioflags & GREN_FLAG_USE_TIMER) && e->timerFinished < Eng_Global->pauseRelativeTime) GrenadeExplode(self); TODO
}

// Called by physics collision callback when grenade touches anything
void GrenadeOnCollision(u16 self) { (void)self;
//     if (Eng_Global->instances[self].ioflags & GREN_FLAG_EXPLODE_CONTACT) GrenadeExplode(self); TODO
}
//=============================================================================
// ProjectileEffectImpact
// TODO: GetDamageTakeAmount(DamageData* dd) — weapon/armor calculation system
// TODO: TakeDamage(u16 target, DamageData* dd) -> float — health manager
// TODO: Tranquilize(u16 target, float amount, bool fromProjectile) -> float
// TODO: ApplyImpactForceSphere — needs OverlapSphere from physics, engine-side
// TODO: ApplyImpactForce(u16 target, float vel, Vector3 normal, Vector3 pt)
// TODO: SpawnImpactEffect(u16 impactType, Vector3 pos) — object pool
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static void ProjectileEffectImpactOnCollision(u16 self,u16 hitIdx, Vector3 hitPos,Vector3 hitNormal) {
    Entity* e   = &Eng_Global->instances[self];
    if (hitIdx == e->recentMostActivator) return; // hit own host, ignore
    e->counter++;
    DamageData dd = {
        .damage        = e->damage,
        .penetration   = e->strength,
        .offense       = e->speed,
        .armorvalue    = 0.0f,
        .defense       = 0.0f,
        .impactVelocity= e->damage * 1.5f,
        .attacknormal  = hitNormal,
        .hitpoint      = hitPos,
        .attackType    = e->attackType,
        .owner         = e->recentMostActivator,
        .hitIdx        = hitIdx,
        .isOtherNPC    = ConstIndexIsNPC(Eng_Global->instances[hitIdx].index),
        .berserkActive = (Eng_Global->invP1.patchActive & PATCH_BERSERK) != 0,
    };

    // Railgun sphere impact
    if (e->lookUpIndex == 5) { // TODO: replace magic with named PoolType enum
        // TODO: ApplyImpactForceSphere(dd, e->position, 3.2f, 1.0f)
        Eng_Global->fogFac += 4;
    }

    Entity* hit = &Eng_Global->instances[hitIdx];
    if (hit->health > 0.0f || hit->cyberHealth > 0.0f) {
        // TODO: dd.damage = GetDamageTakeAmount(&dd)
        if (e->counter < e->countToTrigger) dd.damage *= 0.85f; // per-hit falloff
        dd.impactVelocity = dd.damage * 1.5f;
        if (e->counter > 0) dd.impactVelocity /= 3.0f;
        if (Eng_Global->currentLevel != LEVEL_CYBERSPACE
            && e->recentMostActivator == PLAYER1) {
            // TODO: ApplyImpactForce(hitIdx, dd.impactVelocity, dd.attacknormal, hitPos)
        }
        // TODO: float dmgFinal = TakeDamage(hitIdx, &dd)
        float dmgFinal  = 0.0f; // placeholder until TakeDamage implemented
//         float tranq     = -1.0f;
        if (dd.isOtherNPC) {
            if (!(hit->entflags & EF_ASLEEP)) Sys_Music.inCombat = true;
            if (dd.attackType == AttackType_Tranq) {
//                 float stunAmount = vclamp(3.0f + (Eng_Global->invP1.stungunSetting
//                                           / 100.0f) * 7.0f, 3.0f, 10.0f);
                // TODO: tranq = Tranquilize(hitIdx, stunAmount, true)
            }
        }
        if (dmgFinal < 0.0f) dmgFinal = 0.0f;
        // TODO: CreateTargetIDInstance(dmgFinal, hitIdx, tranq)
        // TODO: SpawnImpactEffect(e->lookUpIndex, hitPos)
    }

    if (e->counter >= e->countToTrigger) {
        // TODO: SpawnImpactEffect(e->lookUpIndex, hitPos)
        if (e->despawnInstead) DeleteInstance(self);
        else flag_set(&e->entflags,EF_ACTIVE,false);
    }
}
#pragma GCC diagnostic pop

void ProjectileEffectImpactInitAfterLoad(u16 self) {
    Entity* e       = &Eng_Global->instances[self];
    e->counter      = 0;
    if (e->countToTrigger < 1) e->countToTrigger = 1;
}
//=============================================================================
// HealthManager

// Attack type damage multiplier table [NPCType][AttackType]
// 1.0f = no change, 0.0f = immune, other = multiplier
static const float attackTypeMult[7][12] = {
    // None  Melee  MelEn  EnBm   Mag    Proj   Needle ProjEB ProjLn Gas    Tranq  Drill
    [NPCType_Mutant]      = {1,1,1,1,0,  1,2,1,1,2,1,1},
    [NPCType_Supermutant] = {1,1,1,1,0,  1,1,1,1,1.5,1,1},
    [NPCType_Robot]       = {1,1,1,1,4,  1,0,1,1,0,1,1},
    [NPCType_Cyborg]      = {1,1,1,1,2,  1,1,1,1,1,1,1},
    [NPCType_Supercyborg] = {1,1,1,1,2,  1,0,1,1,0,1,1},
    [NPCType_MutantCyborg]= {1,1,1,1,0.5,1,2,1,1,2,1.5,1},
    [NPCType_Cyber]       = {1,1,1,1,1,  1,1,1,1,1,1,0},
};

// Object death sound table indexed by constIndex
static const i16 objectDeathSound[] = {
    [458]=63,[459]=66,[460]=66,
    [464]=62,[465]=532,[466]=532,[467]=532,[468]=532,[469]=532,[470]=532,[471]=532,
    [472]=62,[473]=62,[474]=62,[475]=62,[476]=62,
    [477]=61,[478]=65,[479]=69,
    [525]=68,[526]=68,
};

static bool IsCyberEntity(u16 self) {
    if (Eng_Global->currentLevel == LEVEL_CYBERSPACE) return true;
    Entity* e = &Eng_Global->instances[self];
    if (self != PLAYER1 && e->cyberHealth > 0.0f) return true;
    return (ConstIndexIsNPC(e->index) && (e->index - 419) > 23); // 24-28 are cyber enemies
}

static float ApplyAttackTypeAdjustments(u16 self,float take,AttackType at) {
    Entity* e = &Eng_Global->instances[self];
    if (!ConstIndexIsNPC(e->index) || e->health <= 0.0f) return take;
    NPCType t = npcTable[e->index - 419].type;
    if (at >= 12) return take;
    return take * attackTypeMult[t][at];
}

static void UseDeathTargets(u16 self) {
    if (self == PLAYER1 || self == PLAYER2) return;
    Entity* e = &Eng_Global->instances[self];
    if (!StringIsEmpty(e->target)) UseTargets(self,e->target);
}

static void TeleportAway(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    if (e->entflags & EF_TELEPORT_ON_DEATH) return; // already done, flag reused as teleportDone
    flag_set(&e->entflags,EF_TELEPORT_ON_DEATH,true);
    e->collider        = COLTYPE_NONE;
    e->gravity         = 0.0f;
    e->velocity        = (Vector3){0,0,0};
    e->angularVelocity = (Vector3){0,0,0};
    e->modelIndex      = U16_MAX; // remove from rendering
    // TODO: activate teleport effect particle instance at e->position
}

static void DropSearchables(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    // TODO: NotifySearchThatSearchableWasDestroyed()
    for (int i = 0; i < 4; i++) {
        if (e->contents[i] == U16_MAX) continue;
        u16 spawned = SpawnDynamicObject(e->contents[i] + 307,true);
        if (spawned != U16_MAX) {
            Eng_Global->instances[spawned].position   = e->position;
            Eng_Global->instances[spawned].customIndex[0] = e->customIndex[i];
        } else CenterStatusPrint("BUG: Failed to instantiate object being dropped on gib.");
        
        e->contents[i] = e->customIndex[i] = U16_MAX;
    }
}

static void CreateDeathEffects(u16 self,u16 fxPoolType) {
    if (fxPoolType == 0) return; // PoolType_None
    Entity* e = &Eng_Global->instances[self];
    Vector3 pos = e->position;
    // Use collider center offset if present
    if (e->collider != COLTYPE_NONE) {
        pos = V3_AplusB(pos,e->colliderCenter);
    }
    // TODO: SpawnEffectFromPool(fxPoolType, pos)
}

static void HideSelf(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    if (e->index == 279) return; // screens keep mesh visible
    
    e->modelIndex = MODEL_IDX_MAX;
    e->gravity = 0.0f;
}

static void NPCDeath(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    if (e->entflags & EF_DEAD_CHECKS_DONE) return;
    flag_set(&e->entflags,EF_DEAD_CHECKS_DONE,true);
    CreateDeathEffects(self,e->deathBurst);
    if (e->index == 419) play_wav(sounds[64],1.0f,e->position,true); // npc_autobomb: explosion1
    if (npcTable[e->index - 419].type == NPCType_Cyber) DeleteInstance(self);
    // else: keep collider alive to prevent falling through floor (Unity physics note preserved)
}

static void ObjectDeath(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    if (e->entflags & EF_DEAD_CHECKS_DONE) return;
    if (e->entflags & EF_DEATH_BURST_DONE) { // gibOnDeath reuses DEATH_BURST_DONE
        // Gib path
        CreateDeathEffects(self,e->deathBurst);
        DropSearchables(self);
        if (e->index != 279) e->collider = COLTYPE_NONE;
        HideSelf(self);
    } else {
        e->collider = COLTYPE_NONE;
        DropSearchables(self);
        CreateDeathEffects(self,e->deathBurst);
    }
    flag_set(&e->entflags,EF_DEAD_CHECKS_DONE,true);
    // TODO: disable automap overlay for this instance
    if (e->securityThreshold > 0) {
        // TODO: ReduceCurrentLevelSecurity(e->securityThreshold) — security system
    }
    u16 idx = e->index;
    i16 soundex = 62; // default: crate_break
    if (idx < 527 && objectDeathSound[idx] != 0) soundex = objectDeathSound[idx];
    play_wav(sounds[soundex],1.0f,e->position,true);
    if (e->deathBurst != 0) HideSelf(self);
}

static void ScreenDeath(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    if (e->entflags & EF_DEAD_CHECKS_DONE) return;
    flag_set(&e->entflags,EF_DEAD_CHECKS_DONE,true);
    play_wav(sounds[69],1.0f,e->position,true); // screen_destroy
    // TODO: stop ImageSequenceTextureArray animation for this instance
    if (e->entflags & EF_DEATH_BURST_DONE) ObjectDeath(self); // gib path
}

static void VaporizeCorpse(u16 self,bool energyVaporized) {
    Entity* e = &Eng_Global->instances[self];
    flag_set(&e->entflags,EF_DEAD_CHECKS_DONE,true);
    DropSearchables(self);
    u16 fx = e->deathBurst;
    if (fx == 0) fx = 1; // PoolType_CorpseHit fallback
    if (energyVaporized) fx = 2; // PoolType_Vaporize
    e->modelIndex = MODEL_IDX_MAX;
    bool isNPC = ConstIndexIsNPC(e->index);
    bool isSearchable = ConstIndexIsSearchable(e->index);
    if (isNPC || isSearchable) DeleteInstance(self);
    CreateDeathEffects(self,fx);
}

static void Death(u16 self,bool energyVaporized) {
    Entity* e = &Eng_Global->instances[self];
    if (e->entflags & EF_DEAD_CHECKS_DONE) return;
    UseDeathTargets(self);
    bool isNPC = ConstIndexIsNPC(e->index);
    bool isObj = ConstIndexIsDynamicObject(e->index);
    if (e->entflags & EF_ACT_AS_CORPSE_ONLY) { e->entflags |= EF_DEAD_CHECKS_DONE; return; }
//     bool gib        = (e->entflags & EF_DEATH_BURST_DONE) != 0;
    bool vaporize   = (ConstIndexIsNPC(e->index) && e->health <= 0.0f) || ConstIndexIsCorpse(e->index); // vaporizeCorpse maps to VISIBLE being set
    bool isScreen   = (e->index == 279);
    bool isGrenade  = (e->entflags & EF_ISGRENADE) != 0;
    bool isCam      = (e->index == 477);
    bool doTeleport = (e->entflags & EF_TELEPORT_ON_DEATH) != 0; // REQUIRE_RESET reused as teleportOnDeath
    if (e->iceActive) e->collider = COLTYPE_NONE;
    if (vaporize && !isCam && !isGrenade) VaporizeCorpse(self,energyVaporized);
    else if (isObj)    ObjectDeath(self);
    else if (isScreen) ScreenDeath(self);
    else if (doTeleport) TeleportAway(self);
    else if (isGrenade) GrenadeExplode(self);
    if (isNPC && !doTeleport) NPCDeath(self);
    else if (self == PLAYER1 || self == PLAYER2) Eng_Global->deaths++;
    flag_set(&e->entflags,EF_DEAD_CHECKS_DONE,true);
}

float TakeDamage(u16 self,DamageData dd) {
    Entity* e = &Eng_Global->instances[self];
    if (Eng_Cheats->god && (self == PLAYER1 || self == PLAYER2)) return 0.0f;
    bool isCyber = IsCyberEntity(self);
    float* hp    = isCyber ? &e->cyberHealth : &e->health;
    bool isNPC   = ConstIndexIsNPC(e->index);
    bool isPlayer = (self == PLAYER1 || self == PLAYER2);
//     bool isObj   = ConstIndexIsDynamicObject(e->index);
    bool isGrenade = (e->entflags & EF_ISGRENADE) != 0;
//     bool isScreen  = (e->index == 279);
    bool isCam     = (e->index == 477);

    if (isCyber) {
        if (dd.attackType == AttackType_Drill && isNPC) return 0.0f;
        if (dd.attackType != AttackType_Drill && e->iceActive) return 0.0f;
    }
    // Dead exceptions — still allow damage to gibs, ice, player, grenades, screens, cameras, teleporters
    if (*hp <= 0.0f) {
        bool allowPost = (isNPC || e->iceActive || isPlayer || isGrenade || e->index == 279 || isCam);
        if (!allowPost) return 0.0f;
    }

    float take = dd.damage;
    if (isPlayer) {
        float absorb = 0.0f;
        if (isCyber) {
            // Cyber C-Shield software absorption
            if (Eng_Global->invP1.hasSoft & (1 << SW_SHIELD)) {
                u8 sv = Eng_Global->invP1.softVersions[SW_SHIELD];
                absorb = (sv <= 9) ? sv * 0.05f : 0.0f;
                take *= (1.0f - absorb);
                if (take <= 0.0f) return 0.0f;
            }
        } else {
            if (dd.attackType == AttackType_Magnetic) {
                take = 0.0f;
                // TODO: empstatic.Flash(2), BiomonitorEnergyPulse(11f) — FX systems
                TakeEnergy(11.0f);
            }
            InventorySystem* inv = &Eng_Global->invP1;
            if ((inv->hardwareIsActive & HW_SHD) && (inv->hasHardware & HW_SHD)) {
                float thresh = 0.0f;
                switch (inv->hardwareVersion[HW_SHD_IDX]) {
                    case 0: absorb = 0.20f; thresh =  0.0f; break;
                    case 1: absorb = 0.40f; thresh = 10.0f; break;
                    case 2: absorb = 0.75f; thresh = 15.0f; break;
                    case 3: absorb = 0.75f; thresh = 30.0f; break;
                }
                if (take < thresh) absorb = 1.0f;
                if (absorb > 0.0f) {
                    if (absorb < 1.0f) absorb = vclamp(absorb + random_range(-0.08f,0.08f),0.0f,1.0f);
                    take *= (1.0f - absorb);
                    play_wav(sounds[94],Eng_Settings->VolumeEffects,(Vector3){},false); // shield absorb
                    int abs = (int)(absorb * 100.0f);
                    CenterStatusPrint("%s%d%s",Eng_Text->stringTable[208],abs,Eng_Text->stringTable[209]);
                    // TODO: shield screen flash effect
                }
            }
            if (take > 0.0f && (absorb < 0.4f || random_range(0.0f,1.0f) < 0.5f)) {
                play_wav(sounds[140],Eng_Settings->VolumeEffects,(Vector3){},false); // player pain
                // TODO: pstatic.Flash(take>15?2:take>10?1:0) — pain flash FX
            }
            if (dd.owner != NULLENT && ConstIndexIsNPC(Eng_Global->instances[dd.owner].index))
                e->noiseFinished = Eng_Global->pauseRelativeTime; // justHurtByEnemy for music system
        }
    }

    if (isCyber) {
        e->cyberHealth -= take;
        if (isPlayer) {
            Eng_Global->damageReceived += take;
            // TODO: DrawTicks(true)
            if (e->cyberHealth <= 0.0f) {
                // TODO: ExitCyberspace()
                return 0.0f;
            }
        }
        if (dd.owner == PLAYER1 || dd.owner == PLAYER2) Eng_Global->damageDealt += take;
    } else {
        // Camera constIndex 477 gets one-shot by tranq
        if (e->index == 477 && dd.attackType == AttackType_Tranq) take = e->health + 1.0f;
        take = ApplyAttackTypeAdjustments(self,take,dd.attackType);
        e->health -= take;
        if (isPlayer) {
            Eng_Global->damageReceived += take;
            Sys_Music.inCombat = true;
            // TODO: DrawTicks(true)
        }
        if (dd.owner == PLAYER1 || dd.owner == PLAYER2) Eng_Global->damageDealt += take;
    }

    if (isNPC && (e->health > 0.0f || (isCyber && e->cyberHealth > 0.0f))) {
        if (npcTable[e->index - 419].timeBetweenPain > 0.0f) flag_set(&e->entflags,EF_GO_INTO_PAIN,true);
        e->recentMostActivator = dd.owner; // Pass attacker to NPC
        TargetIDSendDamageReceive(self,take,dd.attackType);
        AICheckPain(e); // setup enemy with NPC
    }

    if (isCyber) {
        if (e->cyberHealth <= 0.0f) {
            if (!e->iceActive && isNPC) Eng_Global->cyberkills++;
            Death(self,false);
        }
    } else {
        if (e->health <= 0.0f) {
            if (isNPC) Eng_Global->kills++;
            Death(self,dd.attackType == AttackType_EnergyBeam);
        }
    }
    return take;
}

void HealthManagerInitAfterLoad(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    bool isPlayer = (self == PLAYER1 || self == PLAYER2);
    bool isNPC    = ConstIndexIsNPC(e->index);
    if (isPlayer) {
        e->health      = 211.0f;
        e->cyberHealth = 255.0f;
        e->noiseFinished = Eng_Global->pauseRelativeTime - 31.0; // guarantee no combat music on start
        return;
    }
    if (isNPC) {
        if (IsCyberEntity(self)) {
            if (e->cyberHealth < 0.0f) e->cyberHealth = npcTable[e->index - 419].healthForCyberNPC;
        } else {
            if (e->health < 0.0f) e->health = npcTable[e->index - 419].health;
        }
        if (Eng_Global->difficultyCombat == 0) { e->health = 1.0f; }
        if (e->entflags & EF_ACT_AS_CORPSE_ONLY) {
            e->health = 0.0f; e->cyberHealth = 0.0f;
            UseDeathTargets(self);
            if (e->entflags & EF_TELEPORT_ON_DEATH) TeleportAway(self);
            else NPCDeath(self);
        }
    }
}
//================================================================================
// Inventory Mode
MOD_TO_ENGINE void ForceShootMode(void) {
    if (Eng_Settings->NoShootMode) return; // We are being like the original now!

    Eng_UI->mouseClickHeldOverGUI = false;
//     CloseFullmap(); // TODO
    Eng_Global->inventoryMode = false; Eng_Global->cursorPosition_x = 663; Eng_Global->cursorPosition_y = 371; IgnoreNextMouseDelta(); // Centered on UI baseline resolution 1366x768
//     if (vmailActive) { Eng_Global->invP1.DeactivateVMail(); vmailActive = false; } // TODO
}

void ForceInventoryMode(void) { Eng_Global->inventoryMode = true; Eng_Global->cursorPosition_x = 663; Eng_Global->cursorPosition_y = 371; IgnoreNextMouseDelta(); } // Centered on UI baseline resolution 1366x768
void ToggleInventoryMode(void) { if (Eng_Global->inventoryMode) {ForceShootMode();} else {ForceInventoryMode();} }
//================================================================================
// Hardware
void HardwareBioOff(void) {
    Eng_Global->invP1.hardwareIsActive &= ~HW_BIO;
    if (Eng_Cheats->showFPS) return;
    // TODO: BiomonitorClearGraphs() — engine-side graph reset
    // TODO: deactivate bioMonitorContainer — engine reads BioMonitorActive()
}
void HardwareBioOn(void) {
    Eng_Global->invP1.hardwareIsActive |= HW_BIO;
    // TODO: activate bioMonitorContainer — engine reads BioMonitorActive()
}
void HardwareBioAction(void) {
    InventorySystem* inv = Inv(PLAYER1);
    if (Eng_Global->invP1.hardwareVersionSetting[HW_BIO_IDX] == 0 && inv->energy <= 0.0f) { CenterStatusPrint("%s",Eng_Text->stringTable[314]); return; }
    
    play_wav(sounds[78],SfxVol(),(Vector3){},false);
    if (BioMonitorActive(PLAYER1)) HardwareBioOff(); else HardwareBioOn();
}
void HardwareBioClick(void) { Eng_UI->mouseClickHeldOverGUI = true; HardwareBioAction(); }

void HardwareSensaroundOn(void) {
    Eng_Global->invP1.hardwareIsActive |= HW_SNS;
    // TODO: activate sensaround cameras/overlays — engine reads hardwareIsActive & HW_SNS + version
}
void HardwareSensaroundOff(void) {
    Eng_Global->invP1.hardwareIsActive &= ~HW_SNS;
    // TODO: deactivate sensaround cameras, restore tabs — engine reads hardwareIsActive & HW_SNS
}
void HardwareSensaroundAction(void) {
    InventorySystem* inv = Inv(PLAYER1);
    if (inv->energy <= 0.0f) { CenterStatusPrint("%s",Eng_Text->stringTable[314]); return; }
    
    if (inv->hardwareIsActive & HW_SNS) {
        play_wav(sounds[82],SfxVol(),(Vector3){},false); HardwareSensaroundOff();
    } else {
        play_wav(sounds[93],SfxVol(),(Vector3){},false); HardwareSensaroundOn();
    }
}

void HardwareSensaroundClick(void) { Eng_UI->mouseClickHeldOverGUI = true; HardwareSensaroundAction(); }

void HardwareShieldOn(void) {
    Eng_Global->invP1.hardwareIsActive |= HW_SHD;
    // TODO: ShieldActivateFX — engine reads hardwareIsActive & HW_SHD
}

void HardwareShieldOff(void) { Eng_Global->invP1.hardwareIsActive &= ~HW_SHD; }
void HardwareShieldOffWithEffects(void) {
    HardwareShieldOff();
    // TODO: ShieldDeactivateFX — engine reads hardwareIsActive & HW_SHD
}

void HardwareShieldAction(void) {
    InventorySystem* inv = Inv(PLAYER1);
    if (inv->energy <= 0.0f) { CenterStatusPrint("%s",Eng_Text->stringTable[314]); return; }
    
    if (Eng_Global->invP1.hardwareIsActive & HW_SHD) {
        play_wav(sounds[95],SfxVol(),(Vector3){},false); HardwareShieldOffWithEffects();
    } else {
        play_wav(sounds[96],SfxVol(),(Vector3){},false); HardwareShieldOn();
    }
}

void HardwareShieldClick(void) { Eng_UI->mouseClickHeldOverGUI = true; HardwareShieldAction(); }

void HardwareLanternOn(void) {
    Eng_Global->invP1.hardwareIsActive |= HW_LAN;
    // TODO: enable headlight at lanternBrightness[hardwareVersionSetting[HW_LAN_IDX]] — engine reads bitmask + version
}
void HardwareLanternOff(void) {
    Eng_Global->invP1.hardwareIsActive &= ~HW_LAN;
    // TODO: disable headlight — engine reads hardwareIsActive & HW_LAN
}

void HardwareLanternAction(void) {
    InventorySystem* inv = Inv(PLAYER1);
    if (inv->energy <= 0.0f) { CenterStatusPrint("%s",Eng_Text->stringTable[314]); return; }
    
    play_wav(sounds[78],SfxVol(),(Vector3){},false);
    if (Eng_Global->invP1.hardwareIsActive & HW_LAN) HardwareLanternOff(); else HardwareLanternOn();
}

void HardwareLanternClick(void) { Eng_UI->mouseClickHeldOverGUI = true; HardwareLanternAction(); }

void HardwareInfraredOn(void) {
    Eng_Global->invP1.hardwareIsActive |= HW_INF;
    // TODO: enable infrared light + grayscale on player/sensaround cameras — engine reads bitmask
}

void HardwareInfraredOff(void) {
    Eng_Global->invP1.hardwareIsActive &= ~HW_INF;
    // TODO: disable infrared light + grayscale — engine reads bitmask
}
void HardwareInfraredAction(void) {
    InventorySystem* inv = Inv(PLAYER1);
    if (inv->energy <= 0.0f) { CenterStatusPrint("%s",Eng_Text->stringTable[314]); return; }
    
    bool wasOn = (Eng_Global->invP1.hardwareIsActive & HW_INF) != 0;
    play_wav(wasOn ? sounds[82] : sounds[98],SfxVol(),(Vector3){},false);
    if (wasOn) HardwareInfraredOff(); else HardwareInfraredOn();
}

void HardwareInfraredClick(void) { Eng_UI->mouseClickHeldOverGUI = true; HardwareInfraredAction(); }

void HardwareEReaderAction(void) {
    play_wav(sounds[97],SfxVol(),(Vector3){},false);
    Eng_Global->invP1.hardwareIsActive |= HW_ERD;
    // TODO: OpenEReaderInItemsTab() — engine-side tab open
}

void HardwareEReaderClick(void) { Eng_UI->mouseClickHeldOverGUI = true; HardwareEReaderAction(); }

void HardwareBoosterOn(void)  { Eng_Global->invP1.hardwareIsActive |=  HW_BST; }
void HardwareBoosterOff(void) { Eng_Global->invP1.hardwareIsActive &= ~HW_BST; }
void HardwareBoosterAction(void) {
    InventorySystem* inv = Inv(PLAYER1);
    if (BoosterSetToBoost(PLAYER1) && inv->energy <= 0.0f) { CenterStatusPrint("%s",Eng_Text->stringTable[314]); return; }
    
    play_wav(sounds[78],SfxVol(),(Vector3){},false);
    if (Eng_Global->invP1.hardwareIsActive & HW_BST) HardwareBoosterOff(); else HardwareBoosterOn();
}

void HardwareBoosterClick(void) { Eng_UI->mouseClickHeldOverGUI = true; HardwareBoosterAction(); }

void HardwareJumpJetsOn(void)  { Eng_Global->invP1.hardwareIsActive |=  HW_JET; }
void HardwareJumpJetsOff(void) { Eng_Global->invP1.hardwareIsActive &= ~HW_JET; }
void HardwareJumpJetsAction(u16 p) {
    InventorySystem* inv = Inv(p);
    if (inv->energy <= 0.0f) { CenterStatusPrint("%s",Eng_Text->stringTable[314]); return; }
    
    play_wav(sounds[78],SfxVol(),(Vector3){},false);
    JumpJetsToggle(PLAYER1);
    if (JumpJetsActive(PLAYER1)) HardwareJumpJetsOn(); else HardwareJumpJetsOff();
}

void HardwareJumpJetsClick(u16 p) { Eng_UI->mouseClickHeldOverGUI = true; HardwareJumpJetsAction(p); }

#define INFRARED_RANGE 50.35f
#define LANTERN_RANGE 11.52f
Color3 lantCol = (Color3){1.0f,1.0f,1.0f};
u16 headmountedLanternLight;
Vector3 lanternPos;
float lanternVersionBrightness[3] = {0.875f,1.4f,1.75f};
void HardwareUpdate(u16 p) {
    InventorySystem* inv = Inv(p);
    bool infraredOn = /*(inv->hasHardware & HW_INF) && */(inv->hardwareIsActive & HW_INF) > 0;
    bool lanternOn = /*(inv->hasHardware & HW_LAN) && */(inv->hardwareIsActive & HW_LAN) > 0;
    if (lanternOn || infraredOn) { // Update headmounted lantern/infrared's light (infrared overrides lantern brightness/range)
        Vector3 ppos = Eng_Global->instances[p].position;
        lanternPos = (Vector3){ppos.x + 0.04f,ppos.y + 0.24f,ppos.z + 0.04f};
        float intensity = infraredOn ? 0.8f : lanternVersionBrightness[inv->hardwareVersionSetting[7]];
        UpdateLight(headmountedLanternLight,lanternPos,lantCol,infraredOn ? INFRARED_RANGE : LANTERN_RANGE,intensity,intensity,0.0f,0.0f,QUAT_IDENTITY,true,true);
    } else UpdateLight(headmountedLanternLight,lanternPos,lantCol,11.52f,0.0f,0.0f,0.0f,0.0f,QUAT_IDENTITY,false,false);
}
//================================================================================
// Patches
void PatchInit(u16 p) {
    InventorySystem* inv = Inv(p);
    inv->mediFinishedTime     = -1.0;
    inv->reflexFinishedTime   = -1.0;
    inv->sightFinishedTime    = -1.0;
    inv->berserkIncrement     = 0;
    inv->patchActive          = 0;
    inv->staminupActive       = false;
    Eng_Global->timeScale   = DEFAULT_TIME_SCALE;
    Eng_Global->geniusActive = false;
    // TODO: sightLight disabled, sightDimming disabled — engine reads patchActive & PATCH_SIGHT + sightFinishedTime
    // TODO: BerserkFX disabled — engine reads patchActive & PATCH_BERSERK + berserkIncrement
}

void PatchUpdate(u16 playerIdx) {
    InventorySystem* inv = Inv(playerIdx);
    if (inv->patchActive & PATCH_DETOX) { // Detox
        if (inv->detoxFinishedTime < Eng_Global->pauseRelativeTime) inv->patchActive -= PATCH_DETOX;
        // effect: engine reads PATCH_DETOX bit to ameliorate radiation — no gamecode action needed
    }

    if (inv->patchActive & PATCH_MEDI) { // Medi
        if (inv->mediFinishedTime < Eng_Global->pauseRelativeTime && inv->mediFinishedTime != -1.0)
            { inv->patchActive -= PATCH_MEDI; inv->mediFinishedTime = -1.0; }
    }

    // Reflex — uses absoluteTime (wall clock) so timescale doesn't affect own expiry
    if (inv->patchActive & PATCH_REFLEX) {
        if (inv->reflexFinishedTime < Eng_Global->absoluteTime && inv->reflexFinishedTime != -1.0) {
            inv->patchActive       -= PATCH_REFLEX;
            inv->reflexFinishedTime = -1.0;
            Eng_Global->timeScale = DEFAULT_TIME_SCALE;
        } else {
            Eng_Global->timeScale = REFLEX_TIME_SCALE;
        }
    } else {
        if (Eng_Global->timeScale != DEFAULT_TIME_SCALE) Eng_Global->timeScale = DEFAULT_TIME_SCALE;
    }
   
    if (inv->patchActive & PATCH_BERSERK) { // Berserk
        if (inv->berserkFinishedTime < Eng_Global->pauseRelativeTime) {
            inv->berserkIncrement = 0;
            inv->patchActive -= PATCH_BERSERK;
            // TODO: BerserkFX disable + reset — engine reads patchActive & PATCH_BERSERK
        } else {
            // TODO: BerserkFX enable — engine reads patchActive & PATCH_BERSERK
            if (inv->berserkIncrementFinishedTime < Eng_Global->pauseRelativeTime) {
                inv->berserkIncrement++;
                if (inv->berserkIncrement > 6) inv->berserkIncrement = 6;
                inv->berserkIncrementFinishedTime = Eng_Global->pauseRelativeTime + (BERSERK_TIME / 5.0f);
                // TODO: engine reads berserkIncrement for texture swap + strength increment
            }
        }
    }

    if (inv->patchActive & PATCH_GENIUS) { // Genius
        if (inv->geniusFinishedTime < Eng_Global->pauseRelativeTime) {
            inv->patchActive          -= PATCH_GENIUS;
            Eng_Global->geniusActive = false;
        } else {
            Eng_Global->geniusActive = true;
        }
    }

    if (inv->patchActive & PATCH_SIGHT) { // Sight
        if (inv->sightFinishedTime < Eng_Global->pauseRelativeTime && inv->sightFinishedTime != -1.0) {
            inv->sightFinishedTime          = -1.0;
            inv->sightSideEffectFinishedTime = Eng_Global->pauseRelativeTime + SIGHT_SIDE_EFFECT_TIME;
            // TODO: sightLight disable, sightDimming enable — engine reads sightFinishedTime == -1 + side effect active
        }
        if (inv->sightSideEffectFinishedTime < Eng_Global->pauseRelativeTime && inv->sightSideEffectFinishedTime != -1.0) {
            inv->sightSideEffectFinishedTime = -1.0;
            inv->sightFinishedTime           = -1.0;
            inv->patchActive                -= PATCH_SIGHT;
            // TODO: sightDimming disable, sightLight disable — engine reads patchActive & PATCH_SIGHT
        }
    }

    if (inv->patchActive & PATCH_STAMINUP) { // Staminup
        if (inv->staminupFinishedTime < Eng_Global->pauseRelativeTime) {
            inv->staminupActive  = false;
            inv->fatigue         = 100.0f; // side effect on expiry
            inv->patchActive    -= PATCH_STAMINUP;
        } else {
            inv->fatigue        = 0.0f;
            inv->staminupActive = true;
        }
    }
}

void PatchDisableAll(void) {
    InventorySystem* inv = Inv(PLAYER1);
    inv->berserkFinishedTime          = -1.0;
    inv->berserkIncrementFinishedTime = -1.0;
    inv->berserkIncrement             = 0;
    inv->detoxFinishedTime            = -1.0;
    inv->geniusFinishedTime           = -1.0;
    inv->mediFinishedTime             = -1.0;
    inv->reflexFinishedTime           = -1.0;
    inv->sightFinishedTime            = -1.0;
    inv->sightSideEffectFinishedTime  = -1.0;
    inv->staminupFinishedTime         = -1.0;
    inv->staminupActive               = false;
    inv->fatigue                      = 0.0f;
    inv->patchActive                  = 0;
    Eng_Global->timeScale           = DEFAULT_TIME_SCALE;
    Eng_Global->geniusActive        = false;
    // TODO: sightLight/sightDimming disable — engine reads patchActive == 0
    // TODO: BerserkFX disable + reset — engine reads patchActive & PATCH_BERSERK
}
//================================================================================
// TODO hopper death needs to tint red halfway through its death animation, then fade back to normal.
//================================================================================
// Security
/*
	int[] levelSecurity;
	int[] levelCameraCount;
	int[] levelLargeNodeCount;
	int[] levelSmallNodeCount;
	int[] levelCameraDestroyedCount;
	int[] levelSmallNodeDestroyedCount;
	int[] levelLargeNodeDestroyedCount;
	Vector3[] ressurectionLocation;
	bool[] ressurectionActive;
	u16[] ressurectionBayDoor;
	Vector3[] elevatorTargetDestinations;
    
	bool RessurectPlayer() {
		if (!ressurectionActive[Eng_Global->currentLevel]) return false;

		if (Eng_Global->currentLevel == 10 || Eng_Global->currentLevel == 11 || Eng_Global->currentLevel == 12) {
			LoadLevel(6,ressurectionLocation[currentLevel].position);
			ressurectionBayDoor[6].ForceClose();
		} else {
			if (Eng_Global->currentLevel >= 0 || Eng_Global->currentLevel < 13) Eng_Global->instances[PLAYER1].position = ressurectionLocation[Eng_Global->currentLevel];
		}

		// Activate death screen and readouts for "BRAIN ACTIVITY SATISFACTORY..." ya debatable right etc. etc.
// 		PlayerReferenceManager.a.playerDeathRessurectEffect.SetActive(true); // TODO
		PlayTrack(TrackType_Revive,MusicType_Override);
		Eng_Global->instances[PLAYER1].ressurectingFinished = Eng_Global->pauseRelativeTime + 3f;
		return true;
	}
	
	// Typical level
	// 4 CPU nodes
	// 20 cameras
	// 100% = 4x + 20y
	// Assuming that a good camera percentage is 2-3%, CPU % would be about 10-15 each
	void ReduceCurrentLevelSecurity(SecurityType stype) {
		float camScore = 4;
		float nodeSmallScore = 10;
		float nodeLargeScore = 27;
		float secscoreTotal = (levelCameraCount[currentLevel] * camScore) + (levelSmallNodeCount[currentLevel] * nodeSmallScore) + (levelLargeNodeCount[currentLevel] * nodeLargeScore);
		float secDrop = camScore; // default to camScore
		switch (stype) {
			case SecurityType_None: return;
			case SecurityType_Camera: secDrop = ((camScore/secscoreTotal) * 100); levelCameraDestroyedCount[currentLevel]++; break; // 1 camera divided by the total, so 2/ say (40+60) = 2/100 = 0.02, or 2% using the example numbers above
			case SecurityType_NodeSmall: secDrop = ((nodeSmallScore/secscoreTotal) * 100); levelSmallNodeDestroyedCount[currentLevel]++; break;
			case SecurityType_NodeLarge: secDrop = ((nodeLargeScore/secscoreTotal) * 100); levelLargeNodeDestroyedCount[currentLevel]++; break;
		}
		levelSecurity[currentLevel] -= (int)secDrop;
		if (levelSecurity [currentLevel] < 0) levelSecurity [currentLevel] = 0;
		if ((levelLargeNodeDestroyedCount[currentLevel] == levelLargeNodeCount[currentLevel]) && (levelSmallNodeDestroyedCount[currentLevel] == levelSmallNodeCount[currentLevel]) && (levelCameraDestroyedCount[currentLevel] == levelCameraCount[currentLevel])) {
			levelSecurity[currentLevel] = 0;
		}
		CenterStatusPrint("%s", Eng_Text->stringTable[306] + levelSecurity[currentLevel].ToString() + Eng_Text->stringTable[307]);

		// Notify quest log if all nodes were destroyed
		if (levelLargeNodeDestroyedCount[currentLevel] == levelLargeNodeCount[currentLevel]) {
			if (QuestLogNotesManager.a != null) QuestLogNotesManager.a.NodesDestroyed(currentLevel);
		}
	}
}*/
//=============================================================================
// Grenades
void UseGrenade(u16 playerIndex, int index) { // TODO
    (void)playerIndex; (void)index;
    if (Eng_Global->invP1.holdingObject) { CenterStatusPrint("%s",Eng_Text->stringTable[311]); return; } // Can't use grenade, hands full

    ForceInventoryMode();  // Inventory mode is turned on when picking something up.
    ResetHeldItem(playerIndex);
    Eng_Global->invP1.grenadeActive = true;
    CenterStatusPrint("%s%s",Eng_Text->stringTable[index + 326],Eng_Text->stringTable[320]); // activated, grenade is LIVE!
//     switch(index) { // Subtract one from the correct grenade inventory TODO
//         case 7:  Eng_Global->invP1.heldObject = Const.a.GetPrefab(370); RemoveGrenade(0); break; // Frag
//         case 8:  Eng_Global->invP1.heldObject = Const.a.GetPrefab(372); RemoveGrenade(3); break; // Concussion
//         case 9:  Eng_Global->invP1.heldObject = Const.a.GetPrefab(387); RemoveGrenade(1); break; // EMP
//         case 10: Eng_Global->invP1.heldObject = Const.a.GetPrefab(389); RemoveGrenade(6); break; // Earth Shaker
//         case 11: Eng_Global->invP1.heldObject = Const.a.GetPrefab(402); RemoveGrenade(4); break; // Land Mine
//         case 12: Eng_Global->invP1.heldObject = Const.a.GetPrefab(403); RemoveGrenade(5); break; // Nitropak
//         case 13: Eng_Global->invP1.heldObject = Const.a.GetPrefab(404); RemoveGrenade(2); break; // Gas
//     }
    
//     PutObjectInHand(index,-1,0,0,false,true);
}
//================================================================================
// Quest Bits / Mission I/O
// void TargetOnGatePassed(bool bitToCheck, bool passIfTrue, UseData ud, string targ, string targOnFalse) {
//     if (passIfTrue) {
//         if (!bitToCheck) { UseTargets(ud,tio,targ); return; }
//     } else {
//         if (bitToCheck) { UseTargets(ud,tio,targOnFalse); return; }
//     }
// 
//     UseTargets(targ);
// }
// 
// void EnableBits(u16 i) {
//     Eng_Global->instances[WORLD].ioflags |= Eng_Global->instances[i].ioflags;
//     
//     if (Eng_Global->instances[i].ioflags & QUESTBIT_ROBOT_SPAWN_DEACTIVATED) DualLog("QUESTBIT_ROBOT_SPAWN_DEACTIVATED: 1");
//     if (Eng_Global->instances[i].ioflags & QUESTBIT_ISOTOPE_INSTALLED) DualLog("QUESTBIT_ISOTOPE_INSTALLED: 1");
//     if (Eng_Global->instances[i].ioflags & QUESTBIT_SHIELD_ACTIVATED) {
//         DualLog("QUESTBIT_SHIELD_ACTIVATED: 1");
//         QuestLogNotesManager.a.notes[8].SetActive(true);
//         QuestLogNotesManager.a.checkBoxes[8].isOn = Const.a.questData.ShieldActivated;
//         QuestLogNotesManager.a.labels[8].text = Eng_Text->stringTable[560];
//     }
//     
//     if (Eng_Global->instances[i].ioflags & QUESTBIT_LASER_SAFETY_OVERRIDEN) {
//         DualLog("QUESTBIT_LASER_SAFETY_OVERRIDEN: 1");
//         QuestLogNotesManager.a.notes[7].SetActive(true);
//         QuestLogNotesManager.a.checkBoxes[7].isOn = Const.a.questData.LaserSafetyOverriden;
//         QuestLogNotesManager.a.labels[7].text = Eng_Text->stringTable[559];
//     }
//     
//     if (Eng_Global->instances[i].ioflags & QUESTBIT_LASER_DESTROYED) {
//         DualLog("QUESTBIT_LASER_DESTROYED: 1");
//         if (AutoSplitterData.missionSplitID == 1) AutoSplitterData.missionSplitID++;
//         QuestLogNotesManager.a.notes[9].SetActive(true);
//         QuestLogNotesManager.a.checkBoxes[9].isOn = Const.a.questData.LaserDestroyed;
//         QuestLogNotesManager.a.labels[9].text = Eng_Text->stringTable[561];
//     }
//     
//     if (Eng_Global->instances[i].ioflags & QUESTBIT_BETA_GROVE_CYBER_UNLOCKED) {
//         DualLog("QUESTBIT_BETA_GROVE_CYBER_UNLOCKED: 1");
//         QuestLogNotesManager.a.notes[12].SetActive(true);
//     }
//     
//     if (Eng_Global->instances[i].ioflags & QUESTBIT_GROVE_ALPHA_JETTISON_ENABLED) {
//         DualLog("QUESTBIT_GROVE_ALPHA_JETTISON_ENABLED: 1");
//         QuestLogNotesManager.a.notes[12].SetActive(true);
//     }
//     
//     if (Eng_Global->instances[i].ioflags & QUESTBIT_GROVE_BETA_JETTISON_ENABLED) {
//         DualLog("QUESTBIT_GROVE_BETA_JETTISON_ENABLED: 1");
//         QuestLogNotesManager.a.notes[12].SetActive(true);
//     }
//     
//     if (Eng_Global->instances[i].ioflags & QUESTBIT_GROVE_DELTA_JETTISON_ENABLED) {
//         DualLog("QUESTBIT_GROVE_DELTA_JETTISON_ENABLED: 1");
//         QuestLogNotesManager.a.notes[12].SetActive(true);
//     }
//     
//     if (Eng_Global->instances[i].ioflags & QUESTBIT_MASTER_JETTISON_BROKEN) {
//         DualLog("QUESTBIT_MASTER_JETTISON_BROKEN: 1");
//         if (AutoSplitterData.missionSplitID == 2) AutoSplitterData.missionSplitID++;
//         QuestLogNotesManager.a.notes[12].SetActive(true);
//         QuestLogNotesManager.a.notes[11].SetActive(true);
//         QuestLogNotesManager.a.labels[11].text = Eng_Text->stringTable[563]; // Set:Diagnose and repair broken relay
//     }
//     
//     if (Eng_Global->instances[i].ioflags & QUESTBIT_RELAY_428_FIXED) {
//         DualLog("QUESTBIT_RELAY_428_FIXED: 1");
//         QuestLogNotesManager.a.notes[11].SetActive(true);
//         QuestLogNotesManager.a.checkBoxes[11].isOn = Const.a.questData.Relay428Fixed;
//         QuestLogNotesManager.a.labels[11].text = Eng_Text->stringTable[563]; // Set:Diagnose and repair broken relay
//         QuestLogNotesManager.a.labels[11].text += Eng_Text->stringTable[564]; // Add:: 428.
//     }
//     
//     if (Eng_Global->instances[i].ioflags & QUESTBIT_MASTER_JETTISON_ENABLED) {
//         DualLog("QUESTBIT_MASTER_JETTISON_ENABLED: 1");
//         if (AutoSplitterData.missionSplitID == 3) AutoSplitterData.missionSplitID++;
//         QuestLogNotesManager.a.notes[10].SetActive(true);
//         QuestLogNotesManager.a.checkBoxes[10].isOn = Const.a.questData.MasterJettisonEnabled;
//         QuestLogNotesManager.a.labels[10].text = Eng_Text->stringTable[562];
//     }
//     
//     if (Eng_Global->instances[i].ioflags & QUESTBIT_BETA_GROVE_JETTISONED) {
//         DualLog("QUESTBIT_BETA_GROVE_JETTISONED: 1");
//         if (AutoSplitterData.missionSplitID == 4) AutoSplitterData.missionSplitID++;
//         QuestLogNotesManager.a.notes[12].SetActive(true);
//         QuestLogNotesManager.a.checkBoxes[12].isOn = Const.a.questData.BetaGroveJettisoned;
//         QuestLogNotesManager.a.labels[12].text = Eng_Text->stringTable[565];
//         QuestLogNotesManager.a.notes[13].SetActive(true);
//         QuestLogNotesManager.a.labels[13].text = Eng_Text->stringTable[566];
//     }
//     
//     if (Eng_Global->instances[i].ioflags & QUESTBIT_ANTENNA_NORTH_DESTROYED) {
//         DualLog("QUESTBIT_ANTENNA_NORTH_DESTROYED: 1");
//         QuestLogNotesManager.a.notes[13].SetActive(true);
//     }
//     
//     if (Eng_Global->instances[i].ioflags & QUESTBIT_ANTENNA_SOUTH_DESTROYED) {
//         DualLog("QUESTBIT_ANTENNA_SOUTH_DESTROYED: 1");
//         QuestLogNotesManager.a.notes[13].SetActive(true);
//     }
//     
//     if (Eng_Global->instances[i].ioflags & QUESTBIT_ANTENNA_EAST_DESTROYED) {
//         DualLog("QUESTBIT_ANTENNA_EAST_DESTROYED: 1");
//         QuestLogNotesManager.a.notes[13].SetActive(true);
//     }
//     
//     if (Eng_Global->instances[i].ioflags & QUESTBIT_ANTENNA_WEST_DESTROYED) {
//         DualLog("QUESTBIT_ANTENNA_WEST_DESTROYED: 1");
//         QuestLogNotesManager.a.notes[13].SetActive(true);
//     }
//     
//     if (Eng_Global->instances[i].ioflags & QUESTBIT_SELF_DESTRUCT_ACTIVATED) {
//         DualLog("QUESTBIT_SELF_DESTRUCT_ACTIVATED: 1");
//         QuestLogNotesManager.a.notes[0].SetActive(true);
//         QuestLogNotesManager.a.notes[1].SetActive(true);
//         QuestLogNotesManager.a.notes[2].SetActive(true);
//         QuestLogNotesManager.a.notes[3].SetActive(true);
//         QuestLogNotesManager.a.notes[4].SetActive(true);
//         QuestLogNotesManager.a.notes[5].SetActive(true);
//         QuestLogNotesManager.a.notes[6].SetActive(true);
//         QuestLogNotesManager.a.notes[7].SetActive(true);
//         QuestLogNotesManager.a.notes[8].SetActive(true);
//         QuestLogNotesManager.a.notes[9].SetActive(true);
//         QuestLogNotesManager.a.notes[10].SetActive(true);
//         QuestLogNotesManager.a.notes[11].SetActive(true);
//         QuestLogNotesManager.a.notes[12].SetActive(true);
//         QuestLogNotesManager.a.notes[13].SetActive(true);
//         QuestLogNotesManager.a.notes[14].SetActive(true); // Self destruct
//         QuestLogNotesManager.a.notes[15].SetActive(true); // Escape pod
//         QuestLogNotesManager.a.notes[16].SetActive(true); // Access the bridge
//         QuestLogNotesManager.a.checkBoxes[14].isOn = Const.a.questData.SelfDestructActivated;
//         QuestLogNotesManager.a.labels[14].text = Eng_Text->stringTable[567]; // Set:Engage reactor self-destruct.
//         QuestLogNotesManager.a.labels[15].text = Eng_Text->stringTable[568]; // Set:Escape on escape pod.
//     }
//     
//     if (Eng_Global->instances[i].ioflags & QUESTBIT_BRIDGE_SEPARATED) {
//         DualLog("QUESTBIT_BRIDGE_SEPARATED: 1");
//         QuestLogNotesManager.a.notes[0].SetActive(true);
//         QuestLogNotesManager.a.notes[1].SetActive(true);
//         QuestLogNotesManager.a.notes[2].SetActive(true);
//         QuestLogNotesManager.a.notes[3].SetActive(true);
//         QuestLogNotesManager.a.notes[4].SetActive(true);
//         QuestLogNotesManager.a.notes[5].SetActive(true);
//         QuestLogNotesManager.a.notes[6].SetActive(true);
//         QuestLogNotesManager.a.notes[7].SetActive(true);
//         QuestLogNotesManager.a.notes[8].SetActive(true);
//         QuestLogNotesManager.a.notes[9].SetActive(true);
//         QuestLogNotesManager.a.notes[10].SetActive(true);
//         QuestLogNotesManager.a.notes[11].SetActive(true);
//         QuestLogNotesManager.a.notes[12].SetActive(true);
//         QuestLogNotesManager.a.notes[13].SetActive(true);
//         QuestLogNotesManager.a.notes[14].SetActive(true); // Self destruct
//         QuestLogNotesManager.a.checkBoxes[14].isOn = Const.a.questData.SelfDestructActivated;
//         QuestLogNotesManager.a.labels[14].text = Eng_Text->stringTable[567]; // Set:Engage reactor self-destruct.
//         QuestLogNotesManager.a.notes[16].SetActive(true);
//         QuestLogNotesManager.a.notes[17].SetActive(true);
//         QuestLogNotesManager.a.checkBoxes[16].isOn = true;
//         QuestLogNotesManager.a.labels[16].text = Eng_Text->stringTable[569]; // Set:Access the bridge.
//         QuestLogNotesManager.a.labels[17].text = Eng_Text->stringTable[570]; // Set:Destroy SHODAN.
//     }
//     
//     if (Eng_Global->instances[i].ioflags & QUESTBIT_ISOLINEAR_CHIPSET_INSTALLED) DualLog("QUESTBIT_ISOLINEAR_CHIPSET_INSTALLED: 1");
// }
// 
// void DisableBits() {
//     if (RobotSpawnDeactivated) {
//         Const.a.questData.RobotSpawnDeactivated = false;
//     }
// 
//     if (IsotopeInstalled) Const.a.questData.IsotopeInstalled = false;
//     if (ShieldActivated) {
//         Const.a.questData.ShieldActivated = false;
//         DualLog("Bit unset ShieldActivated: "
//                     + Const.a.questData.ShieldActivated.ToString());
// 
//         QuestLogNotesManager.a.checkBoxes[8].isOn =
//             Const.a.questData.ShieldActivated;
//     }
//     if (LaserSafetyOverriden) {
//         Const.a.questData.LaserSafetyOverriden = false;
//         QuestLogNotesManager.a.checkBoxes[7].isOn = Const.a.questData.LaserSafetyOverriden;
//     }
//     if (LaserDestroyed) {
//         Const.a.questData.LaserDestroyed = false;
//         QuestLogNotesManager.a.checkBoxes[9].isOn = Const.a.questData.LaserDestroyed;
//     }
//     if (BetaGroveCyberUnlocked) Const.a.questData.BetaGroveCyberUnlocked = false;
//     if (GroveAlphaJettisonEnabled) Const.a.questData.GroveAlphaJettisonEnabled = false;
//     if (GroveBetaJettisonEnabled) Const.a.questData.GroveBetaJettisonEnabled = false;
//     if (GroveDeltaJettisonEnabled) Const.a.questData.GroveDeltaJettisonEnabled = false;
//     if (MasterJettisonBroken) Const.a.questData.MasterJettisonBroken = false;
//     if (Relay428Fixed) {
//         Const.a.questData.Relay428Fixed = false;
//         QuestLogNotesManager.a.checkBoxes[11].isOn = Const.a.questData.Relay428Fixed;
//     }
//     if (MasterJettisonEnabled) {
//         Const.a.questData.MasterJettisonEnabled = false;
//         QuestLogNotesManager.a.checkBoxes[10].isOn = Const.a.questData.MasterJettisonEnabled;
//     }
//     if (BetaGroveJettisoned) {
//         Const.a.questData.BetaGroveJettisoned = false;
//         QuestLogNotesManager.a.checkBoxes[12].isOn = Const.a.questData.BetaGroveJettisoned;
//     }
//     if (AntennaNorthDestroyed) Const.a.questData.AntennaNorthDestroyed = false;
//     if (AntennaSouthDestroyed) Const.a.questData.AntennaSouthDestroyed = false;
//     if (AntennaEastDestroyed) Const.a.questData.AntennaEastDestroyed = false;
//     if (AntennaWestDestroyed) Const.a.questData.AntennaWestDestroyed = false;
//     if (SelfDestructActivated) {
//         Const.a.questData.SelfDestructActivated = false;
//         QuestLogNotesManager.a.checkBoxes[14].isOn = Const.a.questData.SelfDestructActivated;
//     }
//     if (BridgeSeparated) Const.a.questData.BridgeSeparated = false;
//     if (IsolinearChipsetInstalled) Const.a.questData.IsolinearChipsetInstalled = false;
// }
// 
// void ToggleBits() {
//     if (RobotSpawnDeactivated) Const.a.questData.RobotSpawnDeactivated = !Const.a.questData.RobotSpawnDeactivated;
//     if (IsotopeInstalled) Const.a.questData.IsotopeInstalled = !Const.a.questData.IsotopeInstalled;
//     if (ShieldActivated) {
//         Const.a.questData.ShieldActivated = !Const.a.questData.ShieldActivated;
//         QuestLogNotesManager.a.checkBoxes[8].isOn = Const.a.questData.ShieldActivated;
//         if (Const.a.questData.ShieldActivated) {
//             QuestLogNotesManager.a.notes[8].SetActive(true);
//             QuestLogNotesManager.a.labels[8].text = Eng_Text->stringTable[560];
//         }
//     }
//     if (LaserSafetyOverriden) {
//         Const.a.questData.LaserSafetyOverriden = !Const.a.questData.LaserSafetyOverriden;
//         QuestLogNotesManager.a.checkBoxes[7].isOn = Const.a.questData.LaserSafetyOverriden;
//         if (Const.a.questData.LaserSafetyOverriden) {
//             QuestLogNotesManager.a.notes[7].SetActive(true);
//             QuestLogNotesManager.a.labels[7].text = Eng_Text->stringTable[559];
//         }
//     }
//     if (LaserDestroyed) {
//         Const.a.questData.LaserDestroyed = !Const.a.questData.LaserDestroyed;
//         if (AutoSplitterData.missionSplitID == 1) { AutoSplitterData.missionSplitID++; }
//         QuestLogNotesManager.a.checkBoxes[9].isOn = Const.a.questData.LaserDestroyed;
//         if (Const.a.questData.LaserDestroyed) {
//             QuestLogNotesManager.a.notes[9].SetActive(true);
//             QuestLogNotesManager.a.labels[9].text = Eng_Text->stringTable[561];
//         }
//     }
//     if (BetaGroveCyberUnlocked) Const.a.questData.BetaGroveCyberUnlocked = !Const.a.questData.BetaGroveCyberUnlocked;
//     if (GroveAlphaJettisonEnabled) Const.a.questData.GroveAlphaJettisonEnabled = !Const.a.questData.GroveAlphaJettisonEnabled;
//     if (GroveBetaJettisonEnabled) Const.a.questData.GroveBetaJettisonEnabled = !Const.a.questData.GroveBetaJettisonEnabled;
//     if (GroveDeltaJettisonEnabled) Const.a.questData.GroveDeltaJettisonEnabled = !Const.a.questData.GroveDeltaJettisonEnabled;
//     if (MasterJettisonBroken) {
//         Const.a.questData.MasterJettisonBroken = !Const.a.questData.MasterJettisonBroken;
//         if (Const.a.questData.MasterJettisonBroken) {
//             QuestLogNotesManager.a.notes[11].SetActive(true); // Diagnose and repair broken relay
//             QuestLogNotesManager.a.labels[11].text = Eng_Text->stringTable[563];// Set:Diagnose and repair broken relay
//         }
//     }
//     if (Relay428Fixed) {
//         Const.a.questData.Relay428Fixed = !Const.a.questData.Relay428Fixed;
//         QuestLogNotesManager.a.checkBoxes[11].isOn = Const.a.questData.Relay428Fixed;
//         if (Const.a.questData.Relay428Fixed) {
//             QuestLogNotesManager.a.notes[11].SetActive(true);
//             QuestLogNotesManager.a.labels[11].text = Eng_Text->stringTable[563]; // Set:Diagnose and repair broken relay
//             QuestLogNotesManager.a.labels[11].text += Eng_Text->stringTable[564]; // Add:: 428.
//         }
//     }
//     if (MasterJettisonEnabled) {
//         Const.a.questData.MasterJettisonEnabled = !Const.a.questData.MasterJettisonEnabled;
//         QuestLogNotesManager.a.checkBoxes[10].isOn = Const.a.questData.MasterJettisonEnabled;
//         if (Const.a.questData.MasterJettisonEnabled) {
//             QuestLogNotesManager.a.notes[10].SetActive(true);
//             QuestLogNotesManager.a.labels[10].text = Eng_Text->stringTable[562];
//         }
//     }
//     if (BetaGroveJettisoned) {
//         Const.a.questData.BetaGroveJettisoned = !Const.a.questData.BetaGroveJettisoned;
//         QuestLogNotesManager.a.checkBoxes[12].isOn = Const.a.questData.BetaGroveJettisoned;
//         if (Const.a.questData.BetaGroveJettisoned ) {
//             QuestLogNotesManager.a.notes[12].SetActive(true);
//             QuestLogNotesManager.a.labels[12].text = Eng_Text->stringTable[565];
//             QuestLogNotesManager.a.notes[13].SetActive(true);
//             QuestLogNotesManager.a.labels[13].text = Eng_Text->stringTable[566];
//         }
//     }
//     if (AntennaNorthDestroyed) Const.a.questData.AntennaNorthDestroyed = !Const.a.questData.AntennaNorthDestroyed;
//     if (AntennaSouthDestroyed) Const.a.questData.AntennaSouthDestroyed = !Const.a.questData.AntennaSouthDestroyed;
//     if (AntennaEastDestroyed) Const.a.questData.AntennaEastDestroyed = !Const.a.questData.AntennaEastDestroyed;
//     if (AntennaWestDestroyed) Const.a.questData.AntennaWestDestroyed = !Const.a.questData.AntennaWestDestroyed;
//     if (SelfDestructActivated) {
//         Const.a.questData.SelfDestructActivated = !Const.a.questData.SelfDestructActivated;
//         if (Const.a.questData.SelfDestructActivated) {
//             QuestLogNotesManager.a.notes[14].SetActive(true);
//             QuestLogNotesManager.a.notes[15].SetActive(true); // Escape pod
//             QuestLogNotesManager.a.labels[14].text = Eng_Text->stringTable[567];// Set:Engage reactor self-destruct.
//             QuestLogNotesManager.a.labels[15].text = Eng_Text->stringTable[568];// Set:Escape on escape pod.
//         }
//     }
//     if (BridgeSeparated) {
//         Const.a.questData.BridgeSeparated = !Const.a.questData.BridgeSeparated;
//         if (Const.a.questData.BridgeSeparated) {
//             QuestLogNotesManager.a.notes[16].SetActive(true);
//             QuestLogNotesManager.a.notes[17].SetActive(true);
//             QuestLogNotesManager.a.checkBoxes[16].isOn = true;
//             QuestLogNotesManager.a.labels[16].text = Eng_Text->stringTable[569]; // Set:Access the bridge.
//             QuestLogNotesManager.a.labels[17].text = Eng_Text->stringTable[570]; // Set:Destroy SHODAN.
//         }
//     }
//     if (IsolinearChipsetInstalled) Const.a.questData.IsolinearChipsetInstalled = !Const.a.questData.IsolinearChipsetInstalled;
// }
// 
// void TestBits(bool testIfTrue, UseData ud, TargetIO tio) {
//     if (RobotSpawnDeactivated && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.RobotSpawnDeactivated, testIfTrue, ud, tio, target, targetIfFalse);
//     if (IsotopeInstalled && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.IsotopeInstalled, testIfTrue, ud, tio, target, targetIfFalse);
//     if (ShieldActivated && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.ShieldActivated, testIfTrue, ud, tio, target, targetIfFalse);
//     if (LaserSafetyOverriden && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.LaserSafetyOverriden, testIfTrue, ud, tio, target, targetIfFalse);
//     if (LaserDestroyed && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.LaserDestroyed, testIfTrue, ud, tio, target, targetIfFalse);
//     if (BetaGroveCyberUnlocked && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.BetaGroveCyberUnlocked, testIfTrue, ud, tio, target, targetIfFalse);
//     if (GroveAlphaJettisonEnabled && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.GroveAlphaJettisonEnabled, testIfTrue, ud, tio, target, targetIfFalse);
//     if (GroveBetaJettisonEnabled && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.GroveBetaJettisonEnabled, testIfTrue, ud, tio, target, targetIfFalse);
//     if (GroveDeltaJettisonEnabled && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.GroveDeltaJettisonEnabled, testIfTrue, ud, tio, target, targetIfFalse);
//     if (MasterJettisonBroken && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse)))TargetOnGatePassed(Const.a.questData.MasterJettisonBroken, testIfTrue, ud, tio, target, targetIfFalse);
//     if (Relay428Fixed && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.Relay428Fixed, testIfTrue, ud, tio, target, targetIfFalse);
//     if (MasterJettisonEnabled && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.MasterJettisonEnabled, testIfTrue, ud, tio, target, targetIfFalse);
//     if (BetaGroveJettisoned && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.BetaGroveJettisoned, testIfTrue, ud, tio, target, targetIfFalse);
//     if (AntennaNorthDestroyed && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.AntennaNorthDestroyed, testIfTrue, ud, tio, target, targetIfFalse);
//     if (AntennaSouthDestroyed && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.AntennaSouthDestroyed, testIfTrue, ud, tio, target, targetIfFalse);
//     if (AntennaEastDestroyed && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.AntennaEastDestroyed, testIfTrue, ud, tio, target, targetIfFalse);
//     if (AntennaWestDestroyed && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.AntennaWestDestroyed, testIfTrue, ud, tio, target, targetIfFalse);
//     if (SelfDestructActivated && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.SelfDestructActivated, testIfTrue, ud, tio, target, targetIfFalse);
//     if (BridgeSeparated && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.BridgeSeparated, testIfTrue, ud, tio, target, targetIfFalse);
//     if (IsolinearChipsetInstalled && (!StringIsEmpty(target) || !StringIsEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.IsolinearChipsetInstalled, testIfTrue, ud, tio, target, targetIfFalse);
// }
//================================================================================
// Doors
enum { DOOR_CLIP_IDLE_CLOSED = 0, DOOR_CLIP_OPENING = 1, DOOR_CLIP_IDLE_OPEN = 2, DOOR_CLIP_CLOSING = 3 };
void ChangeAnim(Entity* e, u8 clip);
static AnimationClip DoorGetClip(const Entity* e, u8 clip) { return modelAnimationClips[e->animationNum][clip]; }
static float DoorClamp01(float v) { if (v < 0.0f) return 0.0f; if (v > 1.0f) return 1.0f; return v; }
static bool DoorInventoryHasAccessCard(AccessCardType card) { return card == AccessCardType_None || (Eng_Global->invP1.accessCardOwned & (1u << card)); }
static bool DoorIsAjar(const Entity* e) { return e->doorOpen == DoorState_Open || e->doorOpen == DoorState_Opening; }
static float DoorGetProgress(const Entity* e, u8 clip) {
    AnimationClip c = DoorGetClip(e,clip); if (c.frameEnd <= c.frameStart) return 1.0f;
    return DoorClamp01((float)(e->frame - c.frameStart) / (float)(c.frameEnd - c.frameStart));
}

static u16 DoorFrameFromProgress(AnimationClip c, float t) {
    if (c.frameEnd <= c.frameStart) return c.frameStart;
    u16 span = c.frameEnd - c.frameStart;
    return (u16)(c.frameStart + (u16)(DoorClamp01(t) * (float)span));
}

static void DoorSetClipFrame(u16 self, u8 clip, u16 frame) { ChangeAnim(&Eng_Global->instances[self],clip); (void)frame; }
static void DoorSyncLayer(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    if (!e->changeLayerOnOpenClose) return;
    e->layer = DoorIsAjar(e) ? Layer_InterDebris : Layer_Door;
}

static void DoorOpen(u16 self) {
    DualLog("opening door %u\n",self);
    Entity* e = &Eng_Global->instances[self];
    DoorSetClipFrame(self,DOOR_CLIP_OPENING,DoorGetClip(e,DOOR_CLIP_OPENING).frameStart);
    e->doorOpen = e->doorState = DoorState_Opening;
    e->waitBeforeClose = Eng_Global->pauseRelativeTime + e->delay;
    DoorSyncLayer(self);
    if (e->SFXIndex > 0 && e->SFXIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXIndex],1.0f,e->position,true);
}

static void DoorClose(u16 self) {
    DualLog("closing door %u\n",self);
    Entity* e = &Eng_Global->instances[self];
    DoorSetClipFrame(self,DOOR_CLIP_CLOSING,DoorGetClip(e,DOOR_CLIP_CLOSING).frameStart);
    e->doorOpen = e->doorState = DoorState_Closing;
    DoorSyncLayer(self);
    if (e->SFXIndex > 0 && e->SFXIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXIndex],1.0f,e->position,true);
}

void DoorForceOpen(u16 self) { Eng_Global->instances[self].requiredAccessCard = AccessCardType_None; EntitySetLocked(&Eng_Global->instances[self],false); DoorOpen(self); }
void DoorForceClose(u16 self) { if (Eng_Global->instances[self].doorOpen == DoorState_Closed) {return;} DoorClose(self); }
void DoorActuate(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    if (e->doorOpen == DoorState_Open) { DoorClose(self); return; }
    if (e->doorOpen == DoorState_Closed) { DoorOpen(self); return; }
    
    if (e->doorOpen == DoorState_Opening) {
        float t = DoorGetProgress(e,DOOR_CLIP_OPENING);
        AnimationClip c = DoorGetClip(e,DOOR_CLIP_CLOSING);
        DoorSetClipFrame(self,DOOR_CLIP_CLOSING,DoorFrameFromProgress(c,1.0f - t));
        e->doorOpen = e->doorState = DoorState_Closing;
        DoorSyncLayer(self);
        if (e->SFXIndex >= 0 && e->SFXIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXIndex],1.0f,e->position,true);
        return;
    }
    
    if (e->doorOpen == DoorState_Closing) {
        float t = DoorGetProgress(e,DOOR_CLIP_CLOSING);
        AnimationClip c = DoorGetClip(e,DOOR_CLIP_OPENING);
        DoorSetClipFrame(self,DOOR_CLIP_OPENING,DoorFrameFromProgress(c,1.0f - t));
        e->doorOpen = e->doorState = DoorState_Opening;
        e->waitBeforeClose = Eng_Global->pauseRelativeTime + e->delay;
        DoorSyncLayer(self);
        if (e->SFXIndex >= 0 && e->SFXIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXIndex],1.0f,e->position,true);
    }
}

void DoorInitAfterLoad(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    if (e->startOpen) e->stayOpen = true;
    if (e->useTimeDelay <= 0.0f) e->useTimeDelay = 0.15f;
    if (e->lockedMessageLingdex <= 0) e->lockedMessageLingdex = 3;
    if (e->SFXIndex < 0) e->SFXIndex = 75;
    if (e->doorOpen > DoorState_Opening) e->doorOpen = e->startOpen ? DoorState_Open : DoorState_Closed;
    e->doorState = e->doorOpen;
    if (e->ajar) {
        AnimationClip c = DoorGetClip(e,DOOR_CLIP_OPENING);
        DoorSetClipFrame(self,DOOR_CLIP_OPENING,DoorFrameFromProgress(c,e->ajarPercentage));
        e->doorOpen = e->doorState = DoorState_Opening;
        DoorSyncLayer(self);
        return;
    }
    
    switch (e->doorOpen) {
        case DoorState_Open:    DoorSetClipFrame(self,DOOR_CLIP_IDLE_OPEN,DoorGetClip(e,DOOR_CLIP_IDLE_OPEN).frameStart); break;
        case DoorState_Opening: DoorSetClipFrame(self,DOOR_CLIP_OPENING,DoorFrameFromProgress(DoorGetClip(e,DOOR_CLIP_OPENING),0.0f/*TODO percent of anim*/)); break;
        case DoorState_Closing: DoorSetClipFrame(self,DOOR_CLIP_CLOSING,DoorFrameFromProgress(DoorGetClip(e,DOOR_CLIP_CLOSING),0.0f/*TODO percent of anim*/)); break;
        default:                DoorSetClipFrame(self,DOOR_CLIP_IDLE_CLOSED,DoorGetClip(e,DOOR_CLIP_IDLE_CLOSED).frameStart); break;
    }
    
    DoorSyncLayer(self);
}

void DoorUse(u16 self, u16 activator) {
    DualLog("Door use called by activator %u\n",activator);
    Entity* e = &Eng_Global->instances[self];
    if (activator == NULLENT) return;
    if (GetCurrentLevelSecurity() > e->securityThreshold) { UIBlockedBySecurity(e->position); return; }
    
    if (Eng_Cheats->superoverride || Eng_Global->difficultyMission <= 0) { EntitySetLocked(e,false); e->requiredAccessCard = AccessCardType_None; }
    if (Eng_Global->difficultyMission <= 1) { e->requiredAccessCard = AccessCardType_None; }
    if (e->useFinished >= Eng_Global->pauseRelativeTime) return;
    
    e->useFinished = Eng_Global->pauseRelativeTime + e->useTimeDelay;
    if (e->requiredAccessCard != AccessCardType_None) {
        if (!DoorInventoryHasAccessCard(e->requiredAccessCard)) {
            CenterStatusPrint("%s",Eng_Text->stringTable[2]); // TODO Access-card-specific status text.
            if (e->SFXLockedIndex >= 0 && e->SFXLockedIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXLockedIndex],0.7f,e->position,true);
            return;
        } else e->requiredAccessCard = AccessCardType_None; // TODO Access-card granted status text.
    }
    
    if ((e->entflags & EF_LOCKED) != 0) {        
        CenterStatusPrint("%s",Eng_Text->stringTable[e->lockedMessageLingdex]);
        if (e->SFXLockedIndex >= 0 && e->SFXLockedIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXLockedIndex],0.55f,e->position,true);
        return;
    }

    if ((e->onlyTargetOnce && !e->targetAlreadyDone) || !e->onlyTargetOnce) { e->targetAlreadyDone = true; UseTargets(activator,e->target); }
    if (e->ajar) e->ajar = false;
    DoorActuate(self);
}

void DoorTargetted(u16 self, u16 activator) { if ((Eng_Global->instances[self].entflags & EF_LOCKED) != 0) EntitySetLocked(&Eng_Global->instances[self],false); if (!Eng_Global->instances[self].targettingOnlyUnlocks) DoorUse(self,activator); }
void DoorUpdate(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    if (e->blocked) return; // TODO frame-pause blocked doors instead of fully skipping.
    if (e->ajar) return;
    AnimationClip opening = DoorGetClip(e,DOOR_CLIP_OPENING);
    AnimationClip closing = DoorGetClip(e,DOOR_CLIP_CLOSING);
    if (e->doorOpen == DoorState_Opening && e->clip == DOOR_CLIP_OPENING && e->frame >= opening.frameEnd) {
        e->doorOpen = e->doorState = DoorState_Open;
        DoorSetClipFrame(self,DOOR_CLIP_IDLE_OPEN,DoorGetClip(e,DOOR_CLIP_IDLE_OPEN).frameStart);
        DoorSyncLayer(self);
    } else if (e->doorOpen == DoorState_Closing && e->clip == DOOR_CLIP_CLOSING && e->frame >= closing.frameEnd) {
        e->doorOpen = e->doorState = DoorState_Closed;
        DoorSetClipFrame(self,DOOR_CLIP_IDLE_CLOSED,DoorGetClip(e,DOOR_CLIP_IDLE_CLOSED).frameStart);
        DoorSyncLayer(self);
    }
    if (Eng_Global->pauseRelativeTime > e->waitBeforeClose && e->doorOpen == DoorState_Open && !e->stayOpen && !e->startOpen) DoorClose(self);
}
//================================================================================
// Misc
MOD_TO_ENGINE u16 SpawnDynamicObject(int val, bool cheat) {
    if (!ConstIndexInBounds(val)) { DualLogError("Const index out of bounds: %u", val); return NULLENT; }
    if (cheat) DualLog("Cheat spawn constIndex %u, level: %u, from cheat: %u, name: ", val, Eng_Global->currentLevel, cheat);
    if (ConstIndexIsGeometry(val) && !Eng_Cheats->editMode) { CenterStatusPrint("Indices 0 through 306 (level geometry chunks) not possible when not on edit mode!"); return NULLENT; }
    u16 entityIndexInInstanceTable = NULLENT;
    return entityIndexInInstanceTable;
}

void DeactivateVMail(void) { } // TODO
//================================================================================
// Physics
MOD_TO_ENGINE float GetBasePlayerSpeed(u16 p, bool running) {
    InventorySystem* inv = Inv(p);
    bool isSprinting = Sprint();
    if (Eng_Cheats->noclip && isSprinting) return PLAYER_MAX_CYBER_SPEED * 2.5f;
    if (Eng_Cheats->noclip) return PLAYER_MAX_CYBER_SPEED * 1.5f;
    if (Eng_Global->currentLevel == LEVEL_CYBERSPACE) return PLAYER_MAX_CYBER_SPEED; //Cyber space speed

    float retval = PLAYER_MAX_WALK_SPEED, bonus = 0.0f;
    if (Eng_Global->boosterActive) bonus = PLAYER_BOOSTER_SPEED_BOOST;
    BodyState bodyState = Eng_Global->instances[PLAYER1].bodyState;
    switch (bodyState) {
        case BodyState_StandingUp   : case BodyState_Standing:  retval = PLAYER_MAX_WALK_SPEED;   break;
        case BodyState_CrouchingDown: case BodyState_Crouch:    retval = PLAYER_MAX_CROUCH_SPEED; break;
        case BodyState_Prone:         case BodyState_ProningDown: case BodyState_ProningUp: retval = PLAYER_MAX_PRONE_SPEED; break;
    }

    if ((isSprinting || Eng_Global->boosterActive) && running) {
        if (inv->fatigue > 80.0f && Eng_Global->boosterActive) retval = PLAYER_MAX_SPRINT_SPEED_FATIGUED;
        else                                                   retval = PLAYER_MAX_SPRINT_SPEED;

        if (bodyState == BodyState_Standing || bodyState == BodyState_Crouch || bodyState == BodyState_CrouchingDown) {
            retval -= ((PLAYER_MAX_WALK_SPEED - PLAYER_MAX_CROUCH_SPEED) * 1.5f); // Subtract off the difference in speed between walking and crouching from the sprint speed
        } else if (bodyState == BodyState_Prone || bodyState == BodyState_ProningDown || bodyState == BodyState_ProningUp) {
            retval -= ((PLAYER_MAX_WALK_SPEED - PLAYER_MAX_PRONE_SPEED) * 2.0f); // Subtract off the difference in speed between walking and proning from the sprint speed.
        }
    }

    return retval + bonus;
}
//================================================================================
// Frob/Use
void SearchObject(int searchable, bool first) {
    if (first) {
        // TODO highlight Item tab in mfd
        firstTimeSearch = false;
    }
    if (Eng_Global->instances[searchable].searchableInUse) {
        for (int i=0;i<4;i++) {
            if (Eng_Global->instances[searchable].contents[i] >= 0) break;
        }
    } else play_wav(sounds[91],0.75f,(Vector3){},false);
}

void UseEntity(u16 p, u16 i) {
    InventorySystem* inv = Inv(p);
    Entity* ent = &Eng_Global->instances[i];
    if (ConstIndexIsSearchable(ent->index)) { inv->currentSearchItem = i; SearchObject(i,firstTimeSearch); DualLog("Search\n"); }
    else if (ConstIndexIsDoor(ent->index)) DoorUse(i,PLAYER1);
    else if (ConstIndexIsNPC(ent->index)) DualLog("Can't use NPC\n");
    else if (ConstIndexIsButtonSwitch(ent->index)) ButtonSwitchUse(i,PLAYER1);
    else if (ConstIndexIsGeometry(ent->index)) DualLog("Can't use modular geometry\n");
    else if (ConstIndexIsUsableObject(ent->index)) {
        inv->holdingObject = true;
        inv->heldObjectIndex = ent->index;
        inv->heldObjectCustomIndex = ent->usableCustomIndex;
        inv->heldObjectAmmo = ent->ammo;
        inv->heldObjectAmmo2 = ent->ammo2;
        inv->heldObjectLoadedAlternate = ent->heldObjectLoadedAlternate;
        if (Eng_Settings->QuickItemPickup) { AddItemToInventory(p,ent->index,ent->usableCustomIndex); ResetHeldItem(p); }
		else { CenterStatusPrint("%s%s",Eng_Text->stringTable[inv->heldObjectIndex - 307 + 326],Eng_Text->stringTable[319]); /* picked up.*/ ForceInventoryMode(); } // Inventory mode is turned on when picking something up

		DeleteInstance(i);
    } else CenterStatusPrint("%s%s",Eng_Text->stringTable[29],"name");
}

#define FROB_DISTANCE 4.9f
static inline __attribute__((always_inline)) void Frob(Vector3 pos, Vector3 forward, Vector3 right) {
    if (Eng_Global->currentLevel == LEVEL_CYBERSPACE) return;
    if (vmailActive) { DeactivateVMail(); vmailActive = false; return; }
    if (Eng_Global->uiIsBlocking) return;
    
    InventorySystem* inv = Inv(PLAYER1);
    if (inv->holdingObject) { DropHeldItem(PLAYER1); return; }

    Vector3 dir = ScreenPointToRay(forward,right);
    Eng_Global->debugLine_start = pos;
    Eng_Global->debugLine_end = (Vector3){dir.x * FROB_DISTANCE + pos.x,dir.y * FROB_DISTANCE + pos.y,dir.z * FROB_DISTANCE + pos.z};
    RaycastHit tempHit = Raycast(pos,dir,FROB_DISTANCE,LAYER_MASK_PLAYER_FROB);
    Eng_Global->debugLineFinished = Eng_Global->pauseRelativeTime + 3.0;
    if (!tempHit.hit) { CenterStatusPrint("%s",Eng_Text->stringTable[30]); return; }
    Eng_Global->debugLine_end = tempHit.point;
    DualLog("Raycast hit!  Hit object %u of entity type %u at hit point %f %f %f\n",tempHit.hitInstanceIndex,Eng_Global->instances[tempHit.hitInstanceIndex].index,tempHit.point.x,tempHit.point.y,tempHit.point.z);
    UseEntity(PLAYER1,tempHit.hitInstanceIndex);
}

bool FrobWithHeldObject(void) {
    return false;
    return true;
}
//================================================================================
// Update
MOD_TO_ENGINE void ModUpdate(void) {
    if (Eng_Global->gamePaused || Eng_Global->menuActive) return;
    
    WeaponsUpdate();
    PatchUpdate(PLAYER1);
    HardwareUpdate(PLAYER1);
    if (Use()) Frob(Eng_Global->instances[PLAYER1].position,Eng_Global->instances[PLAYER1].forward,Eng_Global->instances[PLAYER1].right);
    if (Eng_Global->pauseRelativeTime < Eng_Global->debugLineFinished && (Eng_Global->debugLineVertCount + 6) < (MAX_DEBUG_LINE_VERTS * 3)) AddDebugLine(Eng_Global->debugLine_start,Eng_Global->debugLine_end,(Color){0.3f,0.1f,0.6f,0.5f});
    for (u16 i = START_INDEX_LEVEL_INSTANCES; i < Eng_Global->loadedInstances; ++i) {
        Entity* e = &Eng_Global->instances[i];
        TextureSequenceUpdate(i);
        u16 constdex = e->index;
        if (constdex == 718) ExplosionLifeUpdate(i);
        if (ConstIndexIsButtonSwitch(constdex)) ButtonSwitchUpdate(i);
        if (ConstIndexIsDoor(constdex)) DoorUpdate(i);
        if (constdex == 701) LogicTimerUpdate(i);
        if (e->doSelfAfterList || e->despawnInstead || e->destroyAfterListInsteadOfDeactivate) DelayedSpawnUpdate(i);
        if (e->itemLifeTime > 0.0f) SearchFXResetUpdate(i);
        if (Eng_Global->currentLevel == LEVEL_CYBERSPACE && e->cyberTimer > 0.0f) CyberTimerUpdate(i);
        if (constdex == 515) ForceBridgeUpdate(i);
        if (constdex == 517) FuncWallUpdate(i);
        if (constdex == 21 || constdex == 22) CyberWallUpdate(i);
        if (constdex == 736) TargetIDUpdate(i);
//         if (constdex == 596) { GravityLiftOnTriggerStay(i,PLAYER1); } // TODO: Must hook into trigger system
    }
}
//================================================================================
// Input
MOD_TO_ENGINE bool Forward(void) { return Eng_Global->GetKey(0); }
MOD_TO_ENGINE bool StrafeLeft(void) { return Eng_Global->GetKey(1); }
MOD_TO_ENGINE bool Backpedal(void) { return Eng_Global->GetKey(2); }
MOD_TO_ENGINE bool StrafeRight(void) { return Eng_Global->GetKey(3); }
MOD_TO_ENGINE bool Jump(void) { return Eng_Global->GetKey(4); }
MOD_TO_ENGINE bool JumpDown(void) { return Eng_Global->GetKeyPressed(4); }
MOD_TO_ENGINE bool Crouch(void) { return Eng_Global->GetKeyPressed(5); }
MOD_TO_ENGINE bool Prone(void) { return Eng_Global->GetKeyPressed(6); }
MOD_TO_ENGINE bool LeanLeft(void) { return Eng_Global->GetKey(7); }
MOD_TO_ENGINE bool LeanRight(void) { return Eng_Global->GetKey(8); }
MOD_TO_ENGINE bool Sprint(void) { return Eng_Global->GetKey(9); }
MOD_TO_ENGINE bool TurnLeft(void) { return Eng_Global->GetKey(10); }
MOD_TO_ENGINE bool TurnRight(void) { return Eng_Global->GetKey(11); }
MOD_TO_ENGINE bool LookUp(void) { return Eng_Global->GetKey(12); }
MOD_TO_ENGINE bool LookDown(void) { return Eng_Global->GetKey(13); }
MOD_TO_ENGINE bool RecentLog(void) { return Eng_Global->GetKeyPressed(14); }
MOD_TO_ENGINE bool Biomonitor(void) { return Eng_Global->GetKeyPressed(15); }
MOD_TO_ENGINE bool Sensaround(void) { return Eng_Global->GetKeyPressed(16); }
MOD_TO_ENGINE bool Lantern(void) { return Eng_Global->GetKeyPressed(17); }
MOD_TO_ENGINE bool Shield(void) { return Eng_Global->GetKeyPressed(18); }
MOD_TO_ENGINE bool Infrared(void) { return Eng_Global->GetKeyPressed(19); }
MOD_TO_ENGINE bool Email(void) { return Eng_Global->GetKeyPressed(20); }
MOD_TO_ENGINE bool Booster(void) { return Eng_Global->GetKeyPressed(21); }
MOD_TO_ENGINE bool Jumpjets(void) { return Eng_Global->GetKeyPressed(22); }
MOD_TO_ENGINE bool Attack(void) { return Eng_Global->GetKeyPressed(23); }
MOD_TO_ENGINE bool Use(void) { return Eng_Global->GetKeyPressed(24); }
MOD_TO_ENGINE bool Menu(void) { return Eng_Global->GetKeyPressed(25); }
MOD_TO_ENGINE bool ToggleMode(void) { return Eng_Global->GetKeyPressed(26); }
MOD_TO_ENGINE bool Reload(void) { return Eng_Global->GetKeyPressed(27); }
MOD_TO_ENGINE bool WeaponCycUp(void) { return Eng_Global->GetKeyPressed(28); }
MOD_TO_ENGINE bool WeaponCycDown(void) { return Eng_Global->GetKeyPressed(29); }
MOD_TO_ENGINE bool Grenade(void) { return Eng_Global->GetKeyPressed(30); }
MOD_TO_ENGINE bool GrenadeCycUp(void) { return Eng_Global->GetKeyPressed(31); }
MOD_TO_ENGINE bool GrenadeCycDown(void) { return Eng_Global->GetKeyPressed(32); }
MOD_TO_ENGINE bool ChangeAmmoType(void) { return Eng_Global->GetKeyPressed(33); }
MOD_TO_ENGINE bool Patch(void) { return Eng_Global->GetKeyPressed(34); }
MOD_TO_ENGINE bool PatchCycUp(void) { return Eng_Global->GetKeyPressed(35); }
MOD_TO_ENGINE bool PatchCycDown(void) { return Eng_Global->GetKeyPressed(36); }
MOD_TO_ENGINE bool Map(void) { return Eng_Global->GetKeyPressed(37); }
MOD_TO_ENGINE bool SwimUp(void) { return Eng_Global->GetKey(38); }
MOD_TO_ENGINE bool SwimDn(void) { return Eng_Global->GetKey(39); }
MOD_TO_ENGINE bool Console(void) { return Eng_Global->GetKeyPressed(-1); }
MOD_TO_ENGINE bool TakeScreenshot(void) { return Eng_Global->GetKeyPressed(41); }

MOD_TO_ENGINE void ProcessInput(void) {
    if (Console()) ToggleConsole();
    if (Menu() && !Eng_Global->menuActive) { Eng_Global->gamePaused = !Eng_Global->gamePaused; return; }
    if (Menu() && Eng_Global->menuActive) { MenuGoBack(); return; }
    if (Eng_Global->gamePaused || Eng_Global->menuActive || Eng_Cheats->consoleActive) return; // Pause/Menu barrier <<<<<<<
    
    if (ToggleMode()) ToggleInventoryMode();
    if (Lantern()) Eng_Global->invP1.hardwareIsActive ^= HW_LAN;
    if (Infrared()) Eng_Global->invP1.hardwareIsActive ^= HW_INF;
    ApplyPlayerMovements();
}

MOD_TO_ENGINE void CheckAndTakeScreenshot(void) { if (TakeScreenshot() && Eng_Global->current_time > Eng_Global->screenshotTimeout) Screenshot(); }

void SearchableInit(u16 i) {
    int numRandomGeneratedItems = 0;
    if (Eng_Global->instances[i].generateContents) {
        for(int j=0;j<4;j++) {
            if (Eng_Global->instances[i].randomItemDropChance[j] <= 0.0f) continue;
            u8 tempInt = random_range_u8(0,100);
            if (((float)tempInt / 100.0f) <= Eng_Global->instances[i].randomItemDropChance[j]) {
                Eng_Global->instances[i].contents[numRandomGeneratedItems] = Eng_Global->instances[i].randomItem[j];
                numRandomGeneratedItems++;
                if (numRandomGeneratedItems > Eng_Global->instances[i].maxRandomItems) break;
            }
        }
    }
}
//================================================================================
// Entity Init
u8 GetCurrentLevelSecurity(void) { return (Eng_Global->difficultyMission < 1 || Eng_Cheats->superoverride) ? 0u : Eng_Global->levelSecurity[Eng_Global->currentLevel]; }
u16 GetImpactType(u16 instanceIdx) {
    switch (Eng_Global->instances[instanceIdx].bloodType) {
        case BloodType_None:         return 729; // SparksSmall
        case BloodType_Red:          return 724; // BloodSpurtSmall
        case BloodType_Yellow:       return 723; // BloodSpurtSmallYellow
        case BloodType_Green:        return 722; // BloodSpurtSmallGreen
        case BloodType_Robot:        return 730; // SparksSmallBlue
        case BloodType_Leaf:         return 756; // LeafBurst
        case BloodType_Mutation:     return 757; // MutationBurst
        case BloodType_GrayMutation: return 758; // GraytationBurst
    }
    return 729; // SparksSmall
}

void UsableInit(u16 i) {
    Entity* e = &Eng_Global->instances[i];
    if (Eng_Global->difficultyPuzzle == 3 && e->index == 361 && random_range(0.0f,1.0f) < 0.33f) DeleteInstance(i); // 33% chance of not spawning logic probes on Puzzle difficulty of 3
    if (Eng_Global->difficultyMission <= 1 && ConstIndexIsAccessCard(e->index)) DeleteInstance(i); // Remove access cards on Mission difficulty 1 or 0
    if (Eng_Global->difficultyMission == 0 && e->index == 313) DeleteInstance(i); // Remove audiologs on Mission difficulty 0
}

void MFDInit(SystemUI* ui) {
    ui->lastMultiMediaTabOpened = MULTI_MEDIA_TAB_EMAIL_TABLE;
    ui->logFinished = Eng_Global->pauseRelativeTime;
    ui->tickFinished = ui->centerTabsTickFinished = Eng_Global->current_time + 0.1 + (double)random_range(0.0f,1.0f);
    ui->blinkFinished = 1.0 + Eng_Global->pauseRelativeTime;
    ui->beepFinished = 3.0 + Eng_Global->pauseRelativeTime;
}

void InventoryInit(InventorySystem* inv) {
    inv->hardwareInvReferenceIndex[0]  = 21; // Hardcoded lookup indices into the Const main table.
    inv->hardwareInvReferenceIndex[1]  = 22;
    inv->hardwareInvReferenceIndex[2]  = 23;
    inv->hardwareInvReferenceIndex[3]  = 24;
    inv->hardwareInvReferenceIndex[4]  = 25;
    inv->hardwareInvReferenceIndex[5]  = 26;
    inv->hardwareInvReferenceIndex[6]  = 27;
    inv->hardwareInvReferenceIndex[7]  = 28;
    inv->hardwareInvReferenceIndex[8]  = 29;
    inv->hardwareInvReferenceIndex[9]  = 30;
    inv->hardwareInvReferenceIndex[10] = 31;
    inv->hardwareInvReferenceIndex[11] = 32;
    inv->hardwareInvReferenceIndex[12] =  0;
    inv->hardwareInvReferenceIndex[13] =  0;
    inv->generalInventoryIndexRef[0] = 81;
    for (int i=1;i<HW_COUNT;i++) inv->generalInventoryIndexRef[i] = -1; // Skips 0th index on purpose as it always holds access cards "item".
    for (int i=0;i<HW_COUNT;++i) inv->hardwareVersion[i] = inv->hardwareVersionSetting[i] = 0;
    inv->nitroTimeSetting = NITRO_DEFAULT_TIME;
    inv->earthShakerTimeSetting = EARTH_SHAKER_DEFAULT_TIME;
    inv->lastAddedIndex = -1;
    inv->hasNewEmail = true;
    inv->hasNewNotes = true;
    inv->currentCyberItem = -1;
    inv->isPulserNotDrill = true;
    for (int i=0;i<7;++i) inv->weaponInventoryIndices[i] = inv->weaponInventoryAmmoIndices[i] = -1;
    inv->globalLookupIndex = -1;
    inv->sparqSetting = 50.0f;
    inv->ionSetting = 100.0f;
    inv->blasterSetting = 15.0f;
    inv->plasmaSetting = 40.0f;
    inv->stungunSetting = 20.0f;
    inv->justFired = (Eng_Global->pauseRelativeTime - 31.0); // Set >30s before pauseRelativeTime to not immediately play action music.
    inv->energyDrainTickFinished = Eng_Global->pauseRelativeTime + 0.1 + (double)random_range(0.5f, 1.0f);
    inv->energy = 54.0f;
    inv->maxEnergy = 255.0f;
    inv->resetAfterDeathTime = 0.5;
    inv->painSoundFinished = Eng_Global->pauseRelativeTime;
    inv->radSoundFinished = Eng_Global->pauseRelativeTime;
    inv->radFXFinished = Eng_Global->pauseRelativeTime;
}

MOD_TO_ENGINE void PlayerInit(u16 i) {
    Eng_Global->instances[i].index = 767;
    Eng_Global->instances[i].layer = Layer_Player;
    Eng_Global->instances[i].position = (Vector3){10.52f,-43.792f + 0.84f,20.2908f}; // Start Actual: Puts player on Medical Level in actual game start position.  Added 0.84f
    Eng_Global->instances[i].scale = (Vector3){1.0f,1.0f,1.0f};
    Eng_Global->instances[i].rotation = (Quaternion){0.0f,0.7071f,0.0f,0.7071f}; // 90deg rotation CW about Y axis as viewed from the top looking down onto player
    Eng_Global->instances[i].entflags = EF_ACTIVE|EF_RIGIDBODY;
    Eng_Global->instances[i].collider = COLTYPE_CAP;
    Eng_Global->instances[i].colliderCenter.y = -0.84f;
    Eng_Global->instances[i].colliderSize = (Vector3){0.48f,2.0f,1.0f}; // Radius, Overall height including end radii (Unity convention, blech), Direction, 1.0 == Y-Axis
    Eng_Global->instances[i].mass = 1.0f;
    Eng_Global->instances[i].velocity = (Vector3){0.0f,0.0f,0.0f};
    Eng_Global->instances[i].gravity = 1.0f;
    Eng_Global->instances[i].dynamicFriction = 0.6f; Eng_Global->instances[i].staticFriction = 0.8f;
    Eng_Global->instances[i].health = 200.0f;
    Eng_Global->instances[i].noiseFinished = Eng_Global->pauseRelativeTime;
    if (i == PLAYER1) InventoryInit(&Eng_Global->invP1);
    else if (i == PLAYER2) InventoryInit(&Eng_Global->invP2);
}

#include "credits.h"
MOD_TO_ENGINE const char** GetCreditsText(void) { return creditPages; }
MOD_TO_ENGINE void ModInitAfterLoad(void) {
    for (int i=PLAYER1;i<Eng_Global->loadedInstances;++i) {
        Entity* e = &Eng_Global->instances[i]; u16 constIndex = e->index;
        if (i == PLAYER1 || i == PLAYER2 || ConstIndexIsDynamicObject(constIndex) || (ConstIndexIsNPC(constIndex) && constIndex < 443/*not cyber*/)) e->gravity = 1.0f;
        else e->gravity = 0.0f;
        
        if (ConstIndexIsGeometry(constIndex)) e->layer = Layer_Geometry;
        else if (ConstIndexIsDoor(constIndex)) e->layer = Layer_Door;
        else if (ConstIndexIsUsableObject(constIndex)) UsableInit(i);
        else if (ConstIndexIsDoor(e->index)) DoorInitAfterLoad(i);
        else if (ConstIndexIsNPC(constIndex)) { e->layer = Layer_NPC; /* TODO AIInit funcion */ }
        else if (ConstIndexIsSearchable(constIndex)) SearchableInit(i);
        else if (constIndex == 515) func_forcebridge(i); // func_forcebridge
        else if (constIndex == 517) FuncWallInitAfterLoad(i);
        else if (constIndex == 596) GravityLiftInitAfterLoad(i);
        else if (constIndex == 701) LogicTimerInitBeforeLoad(i);
        else if (constIndex == 556) TeleportTouchInitAfterLoad(i); // prop_cyberport
        else if (constIndex == 555) { } // prop_cyber_switch CyberSwitchInitAfterLoad(i);
        else if (constIndex == 21 || constIndex == 22) CyberWallInitAfterLoad(i); // chunk_cyberpanel or chunk_cyberpanel_slice45
        else if (constIndex == 736) TargetIDInitAfterLoad(i);
        else if (ConstIndexIsButtonSwitch(e->index)) ButtonSwitchInitAfterLoad(i);
        else if (constIndex >= 448 && constIndex <= 457) CyberItemInitBeforeLoad(i);
        else if (constIndex == 480) CyberMineInitBeforeLoad(i);
        if (!StringIsEmpty(e->targetname) && (e->ioflags & TARG_IOFLAGS_DISABLE_ON_AWAKE)) flag_set(&e->entflags,EF_ACTIVE,false);
    }
}

u16 GetCrosshairTexture(void) {
    switch(Eng_Global->invP1.weaponIndex) {
        case 36: case 38: case 43: case 45: case 48: return 1121; // red
        case 37: case 40: case 50: return 1253; // blue
        case 41: case 42: return 1166; // orange
        case 44: case 47: return 1122; // yellow
        case 46: case 51: return 1161; // teal
        default: return 1260; // green
    }
    
    return 1260;
}

u16 GetCursorTexture(void) {
    if (Eng_Global->gamePaused || Eng_Global->menuActive) return 1261; // Red standard cursor
    if (!Eng_Global->invP1.holdingObject) return GetCrosshairTexture();
    switch(Eng_Global->invP1.heldObjectIndex) {
        case 308: return 838; // item_paper_wad
        case 309: return 764; // item_beaker
        case 310: return 767; // item_beverage
        case 311: return 981; // item_skull
        case 312: case 313: return 605; // item_arm, item_audiolog
        case 314: return 853; // weapon_grenadefrag
        case 315: return 849; // weapon_grenadeconc
        case 316: return 851; // weapon_grenadeemp
        case 317: return 850; // weapon_grenadeearth
        case 318: return 860; // weapon_grenademine
        case 319: return 861; // weapon_grenadenitro
        case 320: return 859; // weapon_grenadegas
        case 321: return 974; // item_patch_berserk
        case 322: return 975; // item_patch_detox
        case 323: return 976; // item_patch_genius
        case 324: return 977; // item_patch_medi
        case 325: return 978; // tem_patch_reflex
        case 326: return 979; // item_patch_sight
        case 327: return 980; // item_patch_staminup
        case 328: return 882; // item_hw_system
        case 329: return 907; // item_hw_navunit
        case 330: return 902; // item_hw_ereader
        case 331: return 909; // item_hw_sensaround
        case 332: return 935; // item_hw_targetid
        case 333: return 911; // item_hw_shield
        case 334: return 900; // item_hw_bio
        case 335: return 906; // item_hw_lantern
        case 336: return 903; // item_hw_envirosuit
        case 337: return 901; // item_hw_booster
        case 338: return 905; // item_hw_jumpjets
        case 339: return 904; // item_hw_infrared
        case 340: return 966; // item_fireextinguisher
        case 341: return 626; // item_access_card_admin
        case 342: return 845; // item_workerhelmet
        case 343: return 988; // weapon_mk3
        case 344: return 982; // weapon_blaster
        case 345: return 983; // weapon_dartgun
        case 346: return 984; // weapon_flechette
        case 347: return 985; // weapon_ionrifle
        case 348: return 1034; // weapon_rapier
        case 349: return 990; // weapon_pipe
        case 350: return 986; // weapon_magnum
        case 351: return 987; // weapon_magpulse
        case 352: return 1010; // weapon_pistol
        case 353: return 1019; // weapon_plasma
        case 354: return 1027; // weapon_railgun
        case 355: return 1035; // weapon_riotgun
        case 356: return 1036; // weapon_skorpion
        case 357: return 1052; // weapon_sparqbeam
        case 358: return 1065; // weapon_stungun
        case 359: return 965; // item_battery
        case 360: return 968; // item_battery_icad
        case 361: return 972; // item_logic_probe
        case 362: return 967; // item_healthkit
        case 363: return 973; // item_plastique
        case 364: return 969; // item_chipset_interfacedemod
        case 365: return 766; // item_flask
        case 366: return 969; // item_chipset_bitflag
        case 367: return 549; // item_ammo_rubber
        case 368: return 971; // item_isotopex22
        case 369: return 765; // item_testtube
        case 370: return 853; // weapon_grenadefrag_live
        case 371: return 970; // item_chipset_isolinear
        case 372: return 849; // weapon_grenadeconc_live
        case 373: return 420; // item_ammo_needle
        case 374: return 602; // item_ammo_tranq
        case 375: return 593; // item_ammo_standard
        case 376: return 597; // item_ammo_teflon
        case 377: return 411; // item_ammo_hollow
        case 378: return 561; // item_ammo_slug
        case 379: return 419; // item_ammo_magnesium
        case 380: return 421; // item_ammo_penetrator
        case 381: return 417; // item_ammo_hornet
        case 382: return 577; // item_ammo_splinter
        case 383: return 422; // item_ammo_rail
        case 384: return 551; // item_ammo_slag
        case 385: return 552; // item_ammo_slaglarge
        case 386: return 418; // item_ammo_magcart
        case 387: return 851; // weapon_grenadeemp_live
        case 388: return 762; // item_access_card_std
        case 389: return 850; // weapon_grenadeearth_live
        case 390: return 610; // item_access_card_group1
        case 391: return 621; // item_access_card_science
        case 392: return 609; // item_access_card_eng
        case 393: return 610; // item_access_card_groupB
        case 394: return 635; // item_access_card_security
        case 395: return 761; // item_access_card_per5diego
        case 396: return 632; // item_access_card_medi
        case 397: return 610; // item_access_card_group3
        case 398: return 624; // item_access_card_purple
        case 399: return 872; // item_head_male
        case 400: return 862; // item_head_female
        case 401: return 872; // item_severedhead
        case 402: return 860; // weapon_grenademine_live
        case 403: return 861; // weapon_grenadenitro_live
        case 404: return 859; // weapon_grenadegas_live
        case 417: return 760; // item_access_card_perdarcy
    }
    
    return 1250; // paper wad fallback to make issue obvious
}//3903
