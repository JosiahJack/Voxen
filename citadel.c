// citadel.c - Gamelogic.  Most functionality is trivial so put it here.
// TODO: Add camera view entities for other levels than just medical
// TODO: Particle system
// TODO: Voxel GI
// TODO: Save/Load system
// TODO: Directional lights for cyberspace
// TODO: Directional light for sunlight
// TODO: Directional light shadowmapping just for sunlight
// TODO: TARGET ID: Type-LevelNum(0#)EnemyNum(###),Example: Mutant-06003, EXCEPTIONS: Cyborg-00001 is Edward Diego
__attribute__((used)) AutoSplitterData autoSplitter = {0x1337133713371337,0,false,0}; // Fore use with LiveSplit or other future speedrunner utilities for doing speedruns
//=============================================================================
// Initialization
int lev1SecCode,lev2SecCode,lev3SecCode,lev4SecCode,lev5SecCode,lev6SecCode;
#ifdef WINDOWS
i32 __stdcall DllMain(void* hinstDLL, unsigned long fdwReason, void* lpReserved) { (void)hinstDLL; (void)lpReserved; switch (fdwReason) {} return 1; }
void* memset(void* dst, int c, size_t n) { return mset(dst,c,n); }
#endif

void ModNewGame(void) {
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

V3 ScreenPointToRay(V3 fwd, V3 rt) {
    u16 swidth = Sys_Settings.ScreenWidth, sheight = Sys_Settings.ScreenHeight;
    float offsetX = World.cursorPosition_x - ((float)swidth * 0.5f);
    float offsetY = World.cursorPosition_y - ((float)sheight * 0.5f);
    float ndcX = offsetX / ((float)swidth * 0.5f);
    float ndcY = -offsetY / ((float)sheight * 0.5f);
    float tanFov = vtan((float)Sys_Settings.FOV * 0.5f * PI / 180.0f);
    float aspect3D = (float)swidth / (float)sheight;
    V3 view = (V3){ndcX * tanFov * aspect3D,ndcY * tanFov,-1.0f};
    view = V3_Normalize(view);
    V3 flipForward = (V3){-fwd.x,-fwd.y,-fwd.z};
    V3 up = V3_Normalize(V3_Cross(rt,flipForward));
    return (V3){view.x * rt.x + view.y * up.x + view.z * flipForward.x,view.x * rt.y + view.y * up.y + view.z * flipForward.y,view.x * rt.z + view.y * up.z + view.z * flipForward.z};
}

void DropHeldItem(u16 p) {
    InventorySystem* inv = Inv(p);
    Entity* ply = &World.instances[p];
    if (inv->heldObjectIndex >= World.instCount) { ResetHeldItem(p); return; }
    if (inv->dropFinished > World.pauseRelativeTime) return;
    
    inv->dropFinished = World.pauseRelativeTime + 0.2; // Prevent immediate regrab at high fps
    u16 newent = AddInstance(inv->heldObjectIndex,World.position[p]);
    Entity* tossObject = &World.instances[newent];
    tossObject->usableCustomIndex = inv->heldObjectCustomIndex;
    tossObject->ammo = inv->heldObjectAmmo;
    tossObject->ammo2 = inv->heldObjectAmmo2;
    tossObject->heldObjectLoadedAlternate = inv->heldObjectLoadedAlternate;
    World.position[newent] = World.position[p];
    flag_set(&tossObject->entflags,EF_RIGIDBODY,true);
    V3 tossDir = V3_Normalize(ScreenPointToRay(ply->forward,ply->right));
    World.position[newent] = V3_AplusB(World.position[p],V3_ScaleByF(tossDir,0.48f));
    World.velocity[newent] = V3_ScaleByF(tossDir,10.0f);
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
            CenterStatusPrint("%s%s",Sys_Text.stringTable[44],AccessCardCodeForType(card));
            return;
        }
        Inv(p)->accessCardOwned |= (1u<<AccessCardType_Storage)|(1u<<AccessCardType_Security)|(1u<<AccessCardType_Maintenance);
        CenterStatusPrint("%s%s, %s, %s",Sys_Text.stringTable[45],AccessCardCodeForType(AccessCardType_Storage),AccessCardCodeForType(AccessCardType_Security),AccessCardCodeForType(AccessCardType_Maintenance));
        return;
    }
    if (InventoryHasAccessCard(p,card)) { CenterStatusPrint("%s%s",Sys_Text.stringTable[44],AccessCardCodeForType(card)); return; }
    Inv(p)->accessCardOwned |= (1u << card);
    CenterStatusPrint("%s%s",Sys_Text.stringTable[45],AccessCardCodeForType(card));
}

void AddHardwareToInventory(u16 p,int index,int constIndex,int hwversion,bool overt) {
    (void)constIndex;
    if (index < 0) return;
    InventorySystem* inv = Inv(p);
    if (hwversion < 0) { CenterStatusPrint("BUG: Hardware added with version < 0, using 0."); hwversion = 0; }
    if (hwversion > 0 && hwversion <= (int)inv->hardwareVersion[index]) {
        if (overt) CenterStatusPrint("%s",Sys_Text.stringTable[46]); // THAT WARE IS OBSOLETE. DISCARDED.
        return;
    }
    static const u8 textIdx[12] = {21,22,23,24,25,26,27,28,29,30,31,32};
    inv->hardwareInvIndex             = index;
    inv->hasHardware                 |= (u16)(1u << index);
    inv->hardwareVersion[index]       = (u8)hwversion;
    inv->hardwareVersionSetting[index]= hwversion > 0 ? (u8)(hwversion - 1) : 0;
    // TODO: engine enables HUD hardware buttons from hasHardware bitmask on render
    // TODO: nav unit (index 1): compass/automap HUD visibility from hasHardware & HW_NAV + version
    if (overt) CenterStatusPrint("%s v%d",Sys_Text.stringTable[textIdx[index] + 326],hwversion);
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
        CenterStatusPrint("%s%s",Sys_Text.stringTable[index + 326],Sys_Text.stringTable[31]);
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
    CenterStatusPrint("%s",Sys_Text.stringTable[msg[next]]);
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
    CenterStatusPrint("%s",Sys_Text.stringTable[msg[next]]);
}

void AddGrenadeToInventory(u16 p,int index,int useableIndex) {
    if (index < 0) return;
    InventorySystem* inv = Inv(p);
    bool anyGren = false;
    for (int i = 0; i < 7; i++) if (inv->grenAmmo[i]) { anyGren = true; break; }
    if (!anyGren) inv->grenadeCurrent = (i8)index;
    inv->grenAmmo[index]++;
    inv->grenConstIndex[index] = (i16)useableIndex;
    CenterStatusPrint("%s%s",Sys_Text.stringTable[useableIndex + 326],Sys_Text.stringTable[34]);
}

void RemoveGrenade(u16 p,int index) {
    InventorySystem* inv = Inv(p);
    if (inv->grenAmmo[index] > 0) inv->grenAmmo[index]--;
    if (!inv->grenAmmo[index]) GrenadeCycleDown(p);
}

void CheckForUnreadLogs(u16 p) {
    InventorySystem* inv = Inv(p);
    int em = 0, lg = 0;
    for (int i = T_LOGS_COUNT-1; i >= 0; i--) {
        if (inv->hasLog[i] && !inv->readLog[i]) {
            if (Sys_Text.audioLogType[i] == AudioLogType_Email) em++; else lg++;
        }
    }
    if (!em) inv->hasNewEmail = false;
    if (!lg) inv->hasNewLogs  = false;
}

static int FindNextUnreadLog(u16 p) {
    InventorySystem* inv = Inv(p);
    for (int i = T_LOGS_COUNT-1; i >= 0; i--) {
        if (inv->hasLog[i] && !inv->readLog[i]) return i;
    }
    return -1;
}

static void PlayLog(u16 p,int logIndex) {
    if (logIndex < 0) return;
    InventorySystem* inv = Inv(p);
    if (!(inv->hasHardware & HW_ERD)) return;
//     if (inv->logSndInited) { SndStop(&inv->logSound); SndUninit(&inv->logSound); inv->logSndInited = false; }
//     if (!SndInit(sounds[Sys_Text.audioLogSoundIndex[logIndex]],0,NULL,NULL,&inv->logSound)) {
//         SndSetVolume(&inv->logSound,(float)Sys_Settings.VolumeMessage / 100.0f);
//         SndStart(&inv->logSound);
//         inv->logSndInited = true;
//     }
//     inv->readLog[logIndex] = true;
//     if (Sys_Text.audioLogType[logIndex] == AudioLogType_Vmail) {
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
    CenterStatusPrint("%s%s",Sys_Text.stringTable[1020],World.audiologNames[logIndex]); // "Playing <name>"
    // TODO: SendAudioLogToDataTab(logIndex) — engine-side data tab notification
}

void PlayLastAddedLog(u16 p,int logIndex) {
    if (logIndex < 0) return;
    PlayLog(p,logIndex);
    Inv(p)->lastAddedIndex = -1;
}

void AddAudioLogToInventory(u16 p,int index) {
    if (index < 0) { DualLog("BUG: Audio log picked up has no assigned index (-1)"); return; }
    if (index == 128) { CenterStatusPrint("%s",Sys_Text.stringTable[309]); return; } // Trioptimum Funpack
    InventorySystem* inv = Inv(p);
    inv->hasLog[index]  = true;
    inv->lastAddedIndex = index;
    inv->numLogsFromLevel[Sys_Text.audioLogLevelFound[index]]++;
    if      (Sys_Text.audioLogType[index] == AudioLogType_Email)  inv->hasNewEmail = true;
    else if (Sys_Text.audioLogType[index] == AudioLogType_Normal) inv->hasNewLogs  = true;
    if (inv->hasHardware & HW_ERD) {
        // "Audio log <name> picked up. Press <key> to play." — TODO: key binding name interp
        CenterStatusPrint("%s%s%s",Sys_Text.stringTable[36],World.audiologNames[index],Sys_Text.stringTable[38]);
    } else {
        CenterStatusPrint("%s%s%s",Sys_Text.stringTable[36],World.audiologNames[index],Sys_Text.stringTable[310]);
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
    CenterStatusPrint("%s%s",Sys_Text.stringTable[constIndex + 326],Sys_Text.stringTable[35]);
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
    if (inv->turboFinished > World.pauseRelativeTime) inv->turboFinished += inv->turboCyberTime;
    else                                                    inv->turboFinished = inv->turboCyberTime + World.pauseRelativeTime;
}

static void UseDecoy(u16 p) {
    InventorySystem* inv = Inv(p);
    if (World.decoyActive) { CenterStatusPrint("%s",Sys_Text.stringTable[537]); return; }
    if (inv->softVersions[SW_DECOY] <= 0) { inv->hasSoft &= (u8)~(1u << SW_DECOY); return; }
    if (--inv->softVersions[SW_DECOY] == 0) inv->hasSoft &= (u8)~(1u << SW_DECOY);
    u16 decoyIdx = SpawnDynamicObject(417,true); // 417 = CyberDecoy constIndex
    if (decoyIdx != U16_MAX) World.position[decoyIdx] = World.position[p];
}

static void UseRecall(u16 p) {
    InventorySystem* inv = Inv(p);
    if (inv->softVersions[SW_RECALL] <= 0) return;
    if (--inv->softVersions[SW_RECALL] == 0) inv->hasSoft &= (u8)~(1u << SW_RECALL);
    World.position[p] = World.cyberspaceRecallPoint;
}

void UseCyberspaceItem(u16 p) {
    InventorySystem* inv = Inv(p);
    if (inv->cyberItemIndex <= 0) {
        inv->cyberItemIndex = GetExistingCyberItemIndex(p);
        if (inv->cyberItemIndex < 0) { CenterStatusPrint("%s",Sys_Text.stringTable[473]); return; }
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

bool AddSoftwareItem(u16 p, u16 index, int vers) {
    InventorySystem* inv = Inv(p);
    Entity* player = &World.instances[p];
    float sfxVol = (float)Sys_Settings.VolumeEffects / 100.0f;
    switch(index) {
        case 450/*item_cyber_drill*/:
            if (inv->isPulserNotDrill && !(inv->hasSoft & (1u << SW_PULSER))) inv->isPulserNotDrill = false;
            if (vers > inv->softVersions[SW_DRILL]) inv->softVersions[SW_DRILL] = (u8)vers;
            else CenterStatusPrint("%s",Sys_Text.stringTable[46]);
            inv->hasSoft |= (1u << SW_DRILL);
            play_wav(sounds[86],sfxVol,(V3){},false);
            CenterStatusPrint("%s%d%s",Sys_Text.stringTable[444],inv->softVersions[SW_DRILL],Sys_Text.stringTable[458]);
            return true;
        case 454/*item_cyber_pulser*/:
            if (!inv->isPulserNotDrill && !(inv->hasSoft & (1u << SW_PULSER))) inv->isPulserNotDrill = true;
            if (vers > inv->softVersions[SW_PULSER]) inv->softVersions[SW_PULSER] = (u8)vers;
            else CenterStatusPrint("%s",Sys_Text.stringTable[46]);
            inv->hasSoft |= (1u << SW_PULSER);
            play_wav(sounds[86],sfxVol,(V3){},false);
            CenterStatusPrint("%s%d%s",Sys_Text.stringTable[445],inv->softVersions[SW_PULSER],Sys_Text.stringTable[458]);
            return true;
        case 456/*item_cyber_shield*/:
            if (vers > inv->softVersions[SW_SHIELD]) inv->softVersions[SW_SHIELD] = (u8)vers;
            else CenterStatusPrint("%s",Sys_Text.stringTable[46]);
            inv->hasSoft |= (1u << SW_SHIELD);
            play_wav(sounds[86],sfxVol,(V3){},false);
            CenterStatusPrint("%s%d%s",Sys_Text.stringTable[446],inv->softVersions[SW_SHIELD],Sys_Text.stringTable[458]);
            return true;
        case 457/*item_cyber_turbo*/:
            if (inv->cyberItemIndex < 0) inv->cyberItemIndex = 0;
            inv->softVersions[SW_TURBO]++;
            inv->hasSoft |= (1u << SW_TURBO);
            play_wav(sounds[86],sfxVol,(V3){},false);
            CenterStatusPrint("%s",Sys_Text.stringTable[447]);
            return true;
        case 449/*item_cyber_decoy*/:
            if (inv->cyberItemIndex < 0) inv->cyberItemIndex = 1;
            inv->softVersions[SW_DECOY]++;
            inv->hasSoft |= (1u << SW_DECOY);
            play_wav(sounds[86],sfxVol,(V3){},false);
            CenterStatusPrint("%s",Sys_Text.stringTable[448]);
            return true;
        case 455/*item_cyber_recall*/:
            if (inv->cyberItemIndex < 0) inv->cyberItemIndex = 2;
            inv->softVersions[SW_RECALL]++;
            inv->hasSoft |= (1u << SW_RECALL);
            play_wav(sounds[86],sfxVol,(V3){},false);
            CenterStatusPrint("%s",Sys_Text.stringTable[449]);
            return true;
        case 451/* ;) item_cyber_game*/: {
            if (vers < 0 || vers >= 7) return false;
            inv->hasNewData  = true;
            inv->hasMinigame |= (u8)(1u << vers);
            static const u16 gameMsg[7] = {450,451,452,453,454,455,456};
            play_wav(sounds[86],sfxVol,(V3){},false);
            CenterStatusPrint("%s",Sys_Text.stringTable[gameMsg[vers]]);
            return true;
        }
        case 448/*item_cyber_data*/:
            inv->hasNewData = true;
            if (vers >= 0 && vers < T_LOGS_COUNT) inv->hasLog[vers] = true;
            play_wav(sounds[87],sfxVol,(V3){},false);
            CenterStatusPrint("%s",Sys_Text.stringTable[457]);
            return true;
        case 452/*item_cyber_integrity*/:
            if (player->cyberHealth >= 255.0f) return false;
            play_wav(sounds[86],sfxVol,(V3){},false);
            player->cyberHealth += 77.0f;
            if (player->cyberHealth > 255.0f) player->cyberHealth = 255.0f;
            // TODO: DrawTicks(true) — HUD cyber health tick refresh
            CenterStatusPrint("%s",Sys_Text.stringTable[459]);
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
            if (alt) sFormat(buf,bufSize,"%upn | %umg, %upn",mag,inv->wepAmmo[0],inv->wepAmmoSecondary[0]);
            else     sFormat(buf,bufSize,"%umg | %umg, %upn",mag,inv->wepAmmo[0],inv->wepAmmoSecondary[0]);
            break;
        case 37: case 40: case 46: case 50: case 51: // Energy weapons
            scpy_to_a_from_b(buf,heat > 80.0f ? Sys_Text.stringTable[14] : Sys_Text.stringTable[15],bufSize);
            break;
        case 38: // SV-23 Dartgun
            if (alt) sFormat(buf,bufSize,"%utq | %und, %utq",mag,inv->wepAmmo[2],inv->wepAmmoSecondary[2]);
            else     sFormat(buf,bufSize,"%und | %und, %utq",mag,inv->wepAmmo[2],inv->wepAmmoSecondary[2]);
            break;
        case 39: // AM-27 Flechette
            if (alt) sFormat(buf,bufSize,"%usp | %uhn, %usp",mag,inv->wepAmmo[3],inv->wepAmmoSecondary[3]);
            else     sFormat(buf,bufSize,"%uhn | %uhn, %usp",mag,inv->wepAmmo[3],inv->wepAmmoSecondary[3]);
            break;
        case 41: case 42: break; // Laser Rapier / Lead Pipe: no ammo
        case 43: // Magnum 2100
            if (alt) sFormat(buf,bufSize,"%usg | %uhw, %usg",mag,inv->wepAmmo[7],inv->wepAmmoSecondary[7]);
            else     sFormat(buf,bufSize,"%uhw | %uhw, %usg",mag,inv->wepAmmo[7],inv->wepAmmoSecondary[7]);
            break;
        case 44: // SB-20 Magpulse
            if (alt) sFormat(buf,bufSize,"%usu | %ucr, %usu",mag,inv->wepAmmo[8],inv->wepAmmoSecondary[8]);
            else     sFormat(buf,bufSize,"%ucr | %ucr, %usu",mag,inv->wepAmmo[8],inv->wepAmmoSecondary[8]);
            break;
        case 45: // ML-41 Pistol
            if (alt) sFormat(buf,bufSize,"%utf | %ust, %utf",mag,inv->wepAmmo[9],inv->wepAmmoSecondary[9]);
            else     sFormat(buf,bufSize,"%ust | %ust, %utf",mag,inv->wepAmmo[9],inv->wepAmmoSecondary[9]);
            break;
        case 47: sFormat(buf,bufSize,"%url | %url",inv->currentMagazineAmount[slot],inv->wepAmmo[11]); break; // MM-76 Railgun
        case 48: sFormat(buf,bufSize,"%urb | %urb",inv->currentMagazineAmount[slot],inv->wepAmmo[12]); break; // DC-05 Riotgun
        case 49: // RF-07 Skorpion
            if (alt) sFormat(buf,bufSize,"%ulg | %usm, %ulg",mag,inv->wepAmmo[13],inv->wepAmmoSecondary[13]);
            else     sFormat(buf,bufSize,"%usm | %usm, %ulg",mag,inv->wepAmmo[13],inv->wepAmmoSecondary[13]);
            break;
        default: break;
    }
}

void AddAmmoToInventory(u16 p,int index,int constIndex,int amount,bool isSecondary) {
    if (index < 0) return;
    InventorySystem* inv = Inv(p);
    if (isSecondary) inv->wepAmmoSecondary[index] += (u16)amount;
    else             inv->wepAmmo[index]          += (u16)amount;
    CenterStatusPrint("%s%s",Sys_Text.stringTable[constIndex + 326],Sys_Text.stringTable[630]);
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
        CenterStatusPrint("%s%s",Sys_Text.stringTable[index + 326],Sys_Text.stringTable[33]);
        UpdateAmmoCount(p);
        return true;
    }
    return false;
}

void InventoryUpdate(u16 p) {
    InventorySystem* inv = Inv(p);
    if (Grenade()) {
        if (World.instances[p].inCyberTube) UseCyberspaceItem(p);
        else if (inv->grenadeCurrent >= 0 && inv->grenadeCurrent < 7 && inv->grenAmmo[inv->grenadeCurrent] > 0) UseGrenade(p,inv->grenConstIndex[inv->grenadeCurrent]);
        else CenterStatusPrint("%s",Sys_Text.stringTable[322]); // Out of grenades.
    }
    
    if (GrenadeCycUp())  { if (World.instances[p].inCyberTube) CycleCyberSpaceItemUp(p); else GrenadeCycleUp(p); }
    if (GrenadeCycDown()){ if (World.instances[p].inCyberTube) CycleCyberSpaceItemDn(p); else GrenadeCycleDown(p); }
    if (RecentLog() && (inv->hasHardware & HW_ERD)) {
        bool playing = false;//SndPlaying(&inv->logSound); TODO
        if (inv->lastAddedIndex >= 0 && !playing) {
            int temp = inv->lastAddedIndex;
            PlayLog(p,temp);
            inv->lastAddedIndex = FindNextUnreadLog(p);
            if (inv->lastAddedIndex == temp) inv->lastAddedIndex = -1;
            CheckForUnreadLogs(p);
        } else {
//             SndStop(&inv->logSound); TODO
            int temp = inv->lastAddedIndex;
            inv->lastAddedIndex = FindNextUnreadLog(p);
            if (inv->lastAddedIndex == temp) inv->lastAddedIndex = -1;
            CheckForUnreadLogs(p);
            CenterStatusPrint("%s",Sys_Text.stringTable[1019]); // Log playback stopped.
        }
    }

    if (Patch()) {
        if (inv->patchCurrent >= 0 && inv->patchCurrent < 7 && inv->patchCounts[inv->patchCurrent] > 0)
            PatchUse(p,inv->patchCurrent);
        else
            CenterStatusPrint("%s",Sys_Text.stringTable[324]); // Out of patches.
    }
    if (PatchCycUp()) PatchCycleUp(p,true);
    if (PatchCycDown()) PatchCycleDown(p,true);
}

extern u8 magazinePitchCountForWeapon[16];
extern u8 magazinePitchCountForWeapon2[16];
static bool firstTimePickup = true;
static bool firstTimeSearch = true;
// Expects usableItem index
void AddItemFail(u16 p, int index) { DropHeldItem(p); CenterStatusPrint("%s%s%s", Sys_Text.stringTable[32],Sys_Text.stringTable[index + 326],Sys_Text.stringTable[318]); } // Inventory full.
void AddItemToInventory(u16 p, int index, int customIndex) {
    InventorySystem* inv = Inv(p);
    Sys_UI.mouseClickHeldOverGUI = true; // Prevent gun shooting.
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
void CyberDecoyEnable(u16 self) { (void)self; World.decoyActive = true; }
void CyberDecoyDisable(u16 self) { (void)self; World.decoyActive = false; }
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
    Entity* e = &World.instances[self];
    if (other != PLAYER1) return;
    UICyberSprint((u16)e->textIndex);
}
//=============================================================================
// CyberItem
void CyberItemInitBeforeLoad(u16 self) {
    Entity* e = &World.instances[self];
    if (World.diffMis == 0 && e->index == 448) flag_set(&e->entflags,EF_ACTIVE,false); // item_cyber_data
}

void CyberItemOnTriggerEnter(u16 self, u16 other) {
    Entity* e = &World.instances[self];
    if (other != PLAYER1) return;
    if (!AddSoftwareItem(PLAYER1,e->index,e->version)) return;
    flag_set(&e->entflags,EF_ACTIVE,false);
}
//=============================================================================
// CyberIce
void CyberIceOnTriggerEnter(u16 self, u16 other) { (void)self; Entity* e = &World.instances[other]; if (!(e->entflags & EF_RIGIDBODY)) return; e->layer = 24; World.velocity[other] = V3_ScaleByF(World.velocity[other],-1.0f); }
//=============================================================================
// CyberMine
void CyberMineInitBeforeLoad(u16 self) {
    Entity* e = &World.instances[self];
    e->damage = 55.0f;
    if (World.diffCyb < 3) { if (random_range(0.0f,1.0f) < 0.2f) flag_set(&e->entflags,EF_ACTIVE,false); e->damage = 33.0f; }
    if (World.diffCyb < 2) { if (random_range(0.0f,1.0f) < 0.33f) flag_set(&e->entflags,EF_ACTIVE,false); e->damage = 22.0f; }
    if (World.diffCyb < 1) { if (random_range(0.0f,1.0f) < 0.50f) flag_set(&e->entflags,EF_ACTIVE,false); e->damage = 11.0f; }
}

void CyberMineOnTriggerEnter(u16 self, u16 other) { Entity* e = &World.instances[self]; if (other != PLAYER1) return; PlayerTakeDamage(PLAYER1,e->damage); play_wav(sounds[67],1.0f,World.position[self],false); flag_set(&e->entflags,EF_ACTIVE,false); }
//=============================================================================
// CyberPush
void CyberPushOnTriggerStay(u16 self, u16 other) {
    Entity* e = &World.instances[self];
    Entity* player = &World.instances[PLAYER1];
    if (World.diffCyb < 1 || other != PLAYER1) return;
    player->inCyberTube = true;
    AddForce(PLAYER1,V3_ScaleByF(e->direction,e->force * (float)World.deltaTime),false);
    Sys_Music.cyberTube = true;
}

void CyberPushOnTriggerExit(u16 self, u16 other) { (void)self; if (other != PLAYER1) {return;} World.instances[other].inCyberTube = false; Sys_Music.cyberTube = false; }
//=============================================================================
// CyberDoor
void CyberDoorOnCollisionEnter(u16 self, u16 other) {
    Entity* e = &World.instances[self];
    if (!IdxIsDoor(e->index) || (other != PLAYER1 && other != PLAYER2)) return;
    CenterStatusPrint("%s  %s",Sys_Text.stringTable[e->messageIndex],Sys_Text.stringTable[601]);
}
//=============================================================================
// CyberSwitch
void CyberSwitchInitAfterLoad(u16 self) {
    Entity* e = &World.instances[self];
    if (e->iceActive) flag_set(&e->entflags,EF_ACTIVE,true); // TODO Visual subobject parity removed with hierarchy removal.
}

void CyberSwitchOnTriggerEnter(u16 self, u16 other) {
    Entity* e = &World.instances[self];
    if (e->active || other != PLAYER1) return;
    UICyberSprint((u16)e->textIndex);
    e->active = true;
    UseTargets(other,e->target);
}
//=============================================================================
// CyberTimer
void CyberTimerInitAfterLoad(u16 self) {
    Entity* e = &World.instances[self];
    e->cyberTimer = 600.0f;
    e->timerFinished = World.pauseRelativeTime + 1.0;
}

void CyberTimerReset(u16 self, int diff) {
    Entity* e = &World.instances[self];
    switch (diff) {
        case 0: e->cyberTimer = 600.0f; break;
        case 1: e->cyberTimer = 300.0f; break;
        case 2: e->cyberTimer = 240.0f; break;
        case 3: e->cyberTimer = 180.0f; break;
    }
}

void CyberTimerUpdate(u16 self) {
    Entity* e = &World.instances[self];
    if (e->cyberTimer <= 0.0f) { UIExitCyberspace(); return; }
    if (e->timerFinished >= World.pauseRelativeTime) return;
    
    e->cyberTimer -= 1.0f;
    e->minutes = vfloor(e->cyberTimer / 60.0f);
    e->seconds = e->cyberTimer - (e->minutes * 60.0f);
    e->timerFinished = World.pauseRelativeTime + 1.0;
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
    Entity* e = &World.instances[self];
    if (e->itemLifeTime <= 0.0f) e->itemLifeTime = 3.0f;
    e->delayFinished = World.pauseRelativeTime + e->itemLifeTime;
}

void SearchFXResetUpdate(u16 self) {
    Entity* e = &World.instances[self];
    if (e->delayFinished >= World.pauseRelativeTime) return;
    flag_set(&e->entflags,EF_ACTIVE,false);
}
//=============================================================================
// ExplosionLife
void ExplosionLifeInitAfterLoad(u16 self) {
    Entity* e = &World.instances[self];
    if (e->tickTime <= 0.0f) e->tickTime = 0.05f;
    if (e->delay <= 0.0f) e->delay = 0.8f;
    e->delayFinished = World.pauseRelativeTime + e->delay;
}

void ExplosionLifeUpdate(u16 self) {
    Entity* e = &World.instances[self];
    if (!(e->entflags & EF_ACTIVE) || e->delayFinished >= World.pauseRelativeTime) return;
    if (e->dontReset) flag_set(&e->entflags,EF_ACTIVE,false);
    else DeleteInstance(self);
}
//=============================================================================
// DelayedSpawn
void DelayedSpawnEnable(u16 self) {
    Entity* e = &World.instances[self];
    e->timerFinished = World.pauseRelativeTime + e->delay;
    e->active = true;
}

void DelayedSpawnUpdate(u16 self) {
    Entity* e = &World.instances[self];
    if (!e->active || e->timerFinished >= World.pauseRelativeTime) return;
    
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
    Entity* e = &World.instances[self];
    V3 tempVec = V3_AsubB(World.position[self],e->targetPosition);
    float distTotal = V3_Dist(e->startPosition,e->targetPosition);
    tempVec = V3_ScaleByF(V3_Normalize(tempVec),-1.0f);
    if (e->funcState == FuncStates_AjarMovingTarget) tempVec = V3_ScaleByF(tempVec,distTotal * e->percentAjar);
    else if (e->funcState == FuncStates_AjarMovingStart) tempVec = V3_ScaleByF(tempVec,distTotal * (1.0f - e->percentAjar));
    else if (e->funcState == FuncStates_MovingStart) tempVec = V3_ScaleByF(tempVec,distTotal * (1.0f - e->percentMoved));
    else tempVec = V3_ScaleByF(tempVec,distTotal * e->percentMoved);
    SetPosition(self,V3_AplusB(World.position[self],tempVec),true); // Force it like a teleport
}

void FuncWallMoveStart(u16 self) { World.instances[self].funcState = FuncStates_MovingStart; World.instances[self].tickFinished = World.pauseRelativeTime + 10.0f; }
void FuncWallMoveTarget(u16 self) { World.instances[self].funcState = FuncStates_MovingTarget; World.instances[self].tickFinished = World.pauseRelativeTime + 10.0f; }
void FuncWallTargetted(u16 self, u16 activator) {
    (void)activator;
    Entity* e = &World.instances[self];
    if (e->funcState == FuncStates_Start || e->funcState == FuncStates_MovingStart || e->funcState == FuncStates_AjarMovingTarget) FuncWallMoveTarget(self);
    else FuncWallMoveStart(self);
    play_wav(sounds[76],1.0f,World.position[self],true);
}

void FuncWallUpdate(u16 self) {
    Entity* e = &World.instances[self];
    V3 goal = e->funcState == FuncStates_MovingStart ? e->startPosition : e->targetPosition;
    FuncStates doneState = e->funcState == FuncStates_MovingStart ? FuncStates_Start : FuncStates_Target;
    if (e->funcState == FuncStates_Start) { SetPosition(self,e->startPosition,true); World.velocity[self] = (V3){0.0f,0.0f,0.0f}; e->percentMoved = 0.0f; return; }
    if (e->funcState == FuncStates_Target) { SetPosition(self,e->targetPosition,true); World.velocity[self] = (V3){0.0f,0.0f,0.0f}; e->percentMoved = 1.0f; return; }
    if (e->funcState != FuncStates_MovingStart && e->funcState != FuncStates_MovingTarget) return;
    V3 delta = V3_AsubB(goal,World.position[self]);
    float distanceLeft = V3_Mag(delta);
    float total = V3_Dist(e->startPosition,e->targetPosition);
    float dist = e->speed * (float)World.deltaTime;
    if (distanceLeft <= dist || e->tickFinished < World.pauseRelativeTime) {
        SetPosition(self,goal,true);
        e->funcState = doneState;
        e->percentMoved = doneState == FuncStates_Target ? 1.0f : 0.0f;
        World.velocity[self] = (V3){0.0f,0.0f,0.0f};
        return;
    }
    if (distanceLeft > 0.0001f) SetPosition(self,V3_AplusB(World.position[self],V3_ScaleByF(V3_Normalize(delta),dist)),true);
    if (total > 0.0001f) e->percentMoved = V3_Dist(e->startPosition,World.position[self]) / total;
}
//=============================================================================
// ForceBridge
void func_forcebridge(u16 self) {
    Entity* e = &World.instances[self];
    e->tickTime = 0.05f;
    e->tickFinished = World.pauseRelativeTime + e->tickTime + (double)random_range(0.0f,1.0f);
    e->lerping = true;
    if (e->activatedScale.x <= 0.02f) e->activatedScale.x = 2.56f;
    if (e->activatedScale.y <= 0.02f) e->activatedScale.y = 0.08f;
    if (e->activatedScale.z <= 0.02f) e->activatedScale.z = 2.56f;
    if (!e->active) { e->modelIndex = MAX_MDLS; e->collider = COLTYPE_NONE; }
    switch (e->fieldColor) {
        case ForceFieldColor_Red:      e->texIndex = 38; break;
        case ForceFieldColor_Green:    e->texIndex = 40; break;
        case ForceFieldColor_Blue:     e->texIndex = 39; break;
        case ForceFieldColor_Purple:   e->texIndex = 41; break;
        case ForceFieldColor_RedFaint: e->texIndex = 198; break;
    }
}

void ForceBridgeActivate(u16 self, bool isSilent) {
    Entity* e = &World.instances[self]; if (e->active) {return;}
    if (!isSilent) play_wav(sounds[102],1.0f,World.position[self],true);
    e->modelIndex = 78; e->collider = COLTYPE_BOX; e->active = e->lerping = true; World.scale[self] = (V3){ e->forceFieldDirectionX ? 0.1f : e->activatedScale.x, e->forceFieldDirectionY ? 0.1f : e->activatedScale.y, e->forceFieldDirectionZ ? 0.1f : e->activatedScale.z };
}

void ForceBridgeDeactivate(u16 self, bool isSilent) {
    Entity* e = &World.instances[self]; if (!e->active) {return;}
    if (!isSilent) {play_wav(sounds[102],1.0f,World.position[self],true);}
    e->active = false; e->lerping = true; e->modelIndex = MAX_MDLS; e->collider = COLTYPE_NONE;
}

void ForceBridgeToggle(u16 self) { if (World.instances[self].active) {ForceBridgeDeactivate(self,false); } else {ForceBridgeActivate(self,false);} }
void ForceBridgeUpdate(u16 self) {
    Entity* e = &World.instances[self];
    if (e->tickFinished >= World.pauseRelativeTime) return;
    e->tickFinished = World.pauseRelativeTime + e->tickTime;
    if (e->active) {
        if (!e->lerping) return;
        float sx = e->forceFieldDirectionX ? lerp(World.scale[self].x,e->activatedScale.x,e->tickTime * 2.0f) : World.scale[self].x;
        float sy = e->forceFieldDirectionY ? lerp(World.scale[self].y,e->activatedScale.y,e->tickTime * 2.0f) : World.scale[self].y;
        float sz = e->forceFieldDirectionZ ? lerp(World.scale[self].z,e->activatedScale.z,e->tickTime * 2.0f) : World.scale[self].z;
        World.scale[self] = (V3){sx,sy,sz};
        if (vabs(e->activatedScale.x - sx) < 0.08f && vabs(e->activatedScale.y - sy) < 0.08f && vabs(e->activatedScale.z - sz) < 0.08f) { World.scale[self] = e->activatedScale; e->lerping = false; }
    } else if (e->lerping) {
        float sx = e->forceFieldDirectionX ? lerp(World.scale[self].x,0.0f,e->tickTime * 2.0f) : World.scale[self].x;
        float sy = e->forceFieldDirectionY ? lerp(World.scale[self].y,0.0f,e->tickTime * 2.0f) : World.scale[self].y;
        float sz = e->forceFieldDirectionZ ? lerp(World.scale[self].z,0.0f,e->tickTime * 2.0f) : World.scale[self].z;
        World.scale[self] = (V3){sx,sy,sz};
        if (sx < 0.08f || sy < 0.08f || sz < 0.08f) { flag_set(&e->entflags,EF_ACTIVE,false); e->collider = COLTYPE_NONE; e->lerping = false; }
    }
}

//=============================================================================
// TeleportTouch
static u16 TeleportTouch_allTeleportTouches[8];
static bool TeleportTouch_initialized;
void TeleportTouchInitAfterLoad(u16 self) {
    Entity* e = &World.instances[self];
    if (!TeleportTouch_initialized) { for (u8 i = 0; i < 8; i++) TeleportTouch_allTeleportTouches[i] = U16_MAX; TeleportTouch_initialized = true; }
    if (e->teleportID >= 8) { DeleteInstance(self); return; }
    TeleportTouch_allTeleportTouches[e->teleportID] = self;
}

void TeleportTouchOnTriggerEnter(u16 self, u16 other) {
    Entity* e = &World.instances[self];
    Entity* player = &World.instances[PLAYER1];
    if (!e->touchEnabled || other != PLAYER1) return;
    if (player->health <= 0.0f || e->justUsed >= World.pauseRelativeTime) return;
    u16 dest = e->targetDestinationID < 8 ? TeleportTouch_allTeleportTouches[e->targetDestinationID] : U16_MAX;
    if (dest == U16_MAX) return;
    World.position[PLAYER1] = World.position[dest];
    World.instances[dest].justUsed = World.pauseRelativeTime + 1.0;
    play_wav(sounds[106],1.0f,World.position[dest],false);
}

//=============================================================================
// Trigger
void TriggerUseTargets(u16 self, u16 activator) { UseTargets(activator,World.instances[self].target); }
void TriggerDelayedTarget(u16 self, u16 activator) { World.instances[self].delayFireFinished = World.pauseRelativeTime + World.instances[self].delay; TriggerUseTargets(self,activator); }
void TriggerTriggerTripped(u16 self, u16 other) {
    Entity* e = &World.instances[self];
    if (other != PLAYER1 && other != PLAYER2) return;
    if (e->recentMostActivator && e->ignoreSecondaryTriggers) return;
    e->recentMostActivator = other;
    if (e->onlyOnce) e->allDone = true;
    if (e->delay <= 0.0f) TriggerUseTargets(self,other); else TriggerDelayedTarget(self,other);
}

void TriggerOnTriggerEnter(u16 self, u16 other) { if (!World.instances[self].allDone) TriggerTriggerTripped(self,other); }
void TriggerOnTriggerStay(u16 self, u16 other) { if (!World.instances[self].allDone) TriggerTriggerTripped(self,other); }
void TriggerTargetted(u16 self, u16 activator) { if (World.instances[self].ignoreSecondaryTriggers) World.instances[self].recentMostActivator = activator; }
//=============================================================================
// TriggerCounter
void TriggerCounterTarget(u16 self, u16 activator) { UseTargets(activator,World.instances[self].target); }
void TriggerCounterDelayedTarget(u16 self, u16 activator) { World.instances[self].delayFinished = World.pauseRelativeTime + World.instances[self].delay; TriggerCounterTarget(self,activator); }
void TriggerCounterTargetted(u16 self, u16 activator) {
    Entity* e = &World.instances[self];
    e->counter++;
    if (e->counter != e->countToTrigger) return;
    if (e->delay <= 0.0f) TriggerCounterTarget(self,activator); else TriggerCounterDelayedTarget(self,activator);
    if (!e->dontReset) e->counter = 0;
}
//=============================================================================
// TextureChanger
void TextureChangerInitAfterLoad(u16 self) {
    if (!World.instances[self].currentTexture) return;
    World.instances[self].texIndex = World.instances[self].altTexIndex;
    if (World.instances[self].altGlowIndex < MAX_TXRS) World.instances[self].glowIndex = World.instances[self].altGlowIndex;
}

void TextureChangerToggle(u16 self) {
    if (World.instances[self].currentTexture) {
        World.instances[self].texIndex = EDefs[World.instances[self].index].texIndex;
        World.instances[self].glowIndex = EDefs[World.instances[self].index].glowIndex;
    } else {
        World.instances[self].texIndex = World.instances[self].altTexIndex;
        if (World.instances[self].altGlowIndex < MAX_TXRS) World.instances[self].glowIndex = World.instances[self].altGlowIndex;
    }
    World.instances[self].currentTexture = !World.instances[self].currentTexture;
}
// GravityLift
void GravityLiftInitAfterLoad(u16 self) {
    if (World.instances[self].strength <= 0.0f) World.instances[self].strength = 12.0f;
    if (World.instances[self].offStrengthFactor <= 0.0f) World.instances[self].offStrengthFactor = 0.3f;
    if (World.instances[self].distancePaddingToTopPoint <= 0.0f) World.instances[self].distancePaddingToTopPoint = 0.32f;
    World.instances[self].topPoint = (V3){ 0.0f,World.position[self].y + (World.colliderSize[self].y * 0.5f), 0.0f };
}

// TODO just poll bounds and apply in trigger loop, yeesh
void GravityLiftOnTriggerExit(u16 other) { if (other == PLAYER1) World.instances[PLAYER1].gravity = 1.0f; }
// void GravityLiftOnForce(u16 self, u16 other, bool initial) {
//     if (other == PLAYER1) flag_set(&World.instances[PLAYER1].entflags,EF_GRAV_LIFT_STATE,true);
//     float topY = World.position[self].y + (World.colliderSize[self].y * 0.5f);
//     float dist = topY - World.position[other].y + 0.48f;
//     float velY = World.velocity[other].y < 0.0f ? 0.0f : World.velocity[other].y;
//     if (dist < World.instances[self].distancePaddingToTopPoint) AddForce(other,(V3){0.0f,9.81f - velY,0.0f},false); // TODO accel-vs-force parity
//     else if (World.velocity[other].y < (World.instances[self].strength * World.mass[other])) {
//         float yForce = (World.instances[self].strength * World.mass[other]) - World.velocity[other].y;
//         if (initial || World.instances[self].initialBurstFinished > World.pauseRelativeTime) yForce *= 2.0f;
//         AddForce(other,(V3){0.0f,yForce,0.0f},false);
//     }
// }
// 
// void GravityLiftOffForce(u16 self, u16 other, bool initial) {
//     if (other == PLAYER1) flag_set(&World.instances[PLAYER1].entflags,EF_GRAV_LIFT_STATE,true);
//     if (World.velocity[other].y < World.instances[self].offStrengthFactor) {
//         float yForce = World.instances[self].offStrengthFactor - World.velocity[other].y;
//         if (initial || World.instances[self].initialBurstFinished > World.pauseRelativeTime) yForce *= 2.0f;
//         AddForce(other,(V3){0.0f,yForce,0.0f},false);
//     }
// }
// 
// void GravityLiftOnTriggerEnter(u16 self, u16 other) {
//     World.instances[self].initialBurstFinished = World.pauseRelativeTime + 1.0f;
//     if (World.instances[self].active) GravityLiftOnForce(self,other,true);
//     else GravityLiftOffForce(self,other,true);
// }
// 
// void GravityLiftOnTriggerStay(u16 self, u16 other) {
//     if (World.instances[self].active) GravityLiftOnForce(self,other,false);
//     else GravityLiftOffForce(self,other,false);
// }

void GravityLiftToggle(u16 self) { World.instances[self].active = !World.instances[self].active; }
// LogicTimer
void LogicTimerInitBeforeLoad(u16 self) {
    Entity* e = &World.instances[self];
    if (e->timeInterval <= 0.0f) e->timeInterval = 0.35f;
    if (e->randomMin <= 0.0f) e->randomMin = 5.0f;
    if (e->randomMax <= 0.0f) e->randomMax = 10.0f;
    e->intervalFinished = World.pauseRelativeTime + (e->useRandomTimes ? (double)random_range(e->randomMin,e->randomMax) : (double)e->timeInterval);
}

void LogicTimerUseTargets(u16 self) { UseTargets(self,World.instances[self].target); }
void LogicTimerUpdate(u16 self) {
    return; // TODO for testing!  Was getting annoyed by target i/o troubleshooting messages from lev1 broken door firing constantly.
    Entity* e = &World.instances[self];
    if (!e->active || e->intervalFinished >= World.pauseRelativeTime) return;
    e->intervalFinished = World.pauseRelativeTime + (e->useRandomTimes ? (double)random_range(e->randomMin,e->randomMax) : (double)e->timeInterval);
    LogicTimerUseTargets(self);
}

void LogicTimerTargetted(u16 self, u16 activator) { (void)activator; World.instances[self].active = !World.instances[self].active; }
//=============================================================================
// ButtonSwitch
void ButtonSwitchInitAfterLoad(u16 self) {
    Entity* e = &World.instances[self];
    e->delayFinished = 0.0f;
    if (e->active) e->tickFinished = World.pauseRelativeTime + 1.5 + (double)random_range(0.0f,1.0f);
}

void ButtonSwitchUseTargets(u16 self, u16 activator) {
    Entity* e = &World.instances[self];
    DualLog("ButtonSwitchUseTargets, targeting:%s,ioflags:%u\n",e->target,e->ioflags);
    UseTargets(activator,e->target);
    e->active = !e->active;
    e->alternateOn = e->active;
    if (e->changeTexOnActive) {
        e->texIndex = e->alternateOn ? e->altTexIndex : e->mainSwitchMaterial;
        if (e->blinkTexOnActive && e->active) e->tickFinished = World.pauseRelativeTime + 1.5f;
    }
}

void ButtonSwitchUse(u16 self, u16 activator) {
    Entity* e = &World.instances[self];
    if (Cheats.superoverride || World.diffMis == 0) EntitySetLocked(e,false);
    else if (GetCurrentLevelSecurity() > e->securityThreshold) { UIBlockedBySecurity(World.position[self]); return; }
    if ((e->entflags & EF_LOCKED) != 0) {
        CenterStatusPrint("%s",Sys_Text.stringTable[e->lockedMessageLingdex]);
        if (e->SFXLockedIndex >= 0 && e->SFXLockedIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXLockedIndex],1.0f,World.position[self],true);
        return;
    }
    
    if (e->SFXIndex >= 0 && e->SFXIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXIndex],1.0f,World.position[self],true);
    CenterStatusPrint("%s",Sys_Text.stringTable[e->messageIndex]);
    if (e->delay > 0.0f) { e->recentMostActivator = activator; e->delayFinished = World.pauseRelativeTime + e->delay; }
    else ButtonSwitchUseTargets(self,activator);
}

void ButtonSwitchUpdate(u16 self) {
    Entity* e = &World.instances[self];
    if (e->delayFinished > 0.0 && e->delayFinished < World.pauseRelativeTime) { e->delayFinished = 0.0; ButtonSwitchUseTargets(self,e->recentMostActivator); }
    if (e->blinkTexOnActive && e->active && e->tickFinished < World.pauseRelativeTime) {
        e->alternateOn = !e->alternateOn;
        e->texIndex = e->alternateOn ? e->altTexIndex : e->mainSwitchMaterial;
        e->tickFinished = World.pauseRelativeTime + e->tickTime;
    }
}

void ButtonSwitchTargetted(u16 self, u16 activator) { ButtonSwitchUse(self,activator); }
//=============================================================================
// HealingBed
void HealingBedUse(u16 self, u16 owner) {
    Entity* e = &World.instances[self];
    if (GetCurrentLevelSecurity() <= (u8)e->minSecurityLevel) {
        if (!e->broken) {
            HealthManagerHealingBed(PLAYER1,e->amount,true);
            CenterStatusPrint("%s",Sys_Text.stringTable[23],owner);
            play_wav(sounds[103],1.0f,World.position[self],false);
        } else CenterStatusPrint("%s",Sys_Text.stringTable[24],owner);
    } else UIBlockedBySecurity(World.position[self]);
}
//=============================================================================
// TargetIO
void UseTargets(u16 activator, const char* targetname) {
    if (sEmpty(targetname)) return;
    
    bool succeeded = false;
    for (u16 i = INSTS_1ST_IDX; i < World.instCount; i++) {
        if (!sEqual(World.instances[i].targetname,targetname)) continue;
        
        DualLog("Successfully found matching targetname %s for entity %u and activator ioflags:%u\n",targetname,i,World.instances[activator].ioflags);
        Targetted(activator,i);
        succeeded = true;
    }
    if (!succeeded) DualLogWarn("Failed to find a matching targetname for %s\n",targetname);
}

void Targetted(u16 activator, u16 self) {
    Entity* e = &World.instances[self];
    Entity* a = &World.instances[activator];
    DualLog("Targetted running with a->ioflags:%u, e->index:%u, door conditions:%u\n",a->ioflags,e->index,((a->ioflags & TARG_IOFLAGS_DOOROPEN) && IdxIsDoor(e->index)));
    if (e->index == 709) { CenterStatusPrint("%s",Sys_Text.stringTable[e->messageLingdex]); return; } // info_message
    if (e->index == 708) { World.gameFinished = true; return; }
    
    if (e->index == 707 /*info_email*/) EmailTargetted(self,activator);
    if (a->ioflags & TARG_IOFLAGS_TRIPTRIGGER) {
        if (e->index == 598 || e->index == 600) TriggerTargetted(self,activator);
        else if (e->index == 594) TriggerCounterTargetted(self,activator);
    }
    
    if (a->ioflags & TARG_IOFLAGS_UNLOCK) EntitySetLocked(e,false);
    if ((a->ioflags & TARG_IOFLAGS_LOCK) && IdxIsDoor(e->index)) EntitySetLocked(e,true);
    
    if (IdxIsButtonSwitch(e->index)) ButtonSwitchTargetted(self,activator);
    if ((a->ioflags & TARG_IOFLAGS_DOOROPEN) && IdxIsDoor(e->index)) { DualLog("Running DoorForceOpen from ioflag DOOROPEN on entity %u\n",self); DoorForceOpen(self); }
    else if ((a->ioflags & TARG_IOFLAGS_DOOROPENIFUNLOCKED) && IdxIsDoor(e->index) && ((e->entflags & EF_LOCKED) == 0) && (e->requiredAccessCard == AccessCardType_None || (World.invP1.accessCardOwned & (1u << e->requiredAccessCard)))) DoorForceOpen(self);
    else if ((a->ioflags & TARG_IOFLAGS_DOORCLOSE) && IdxIsDoor(e->index)) DoorForceClose(self);
    else if (IdxIsDoor(e->index)) DoorTargetted(self,activator);
    
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
//     Sys_UI.mouseClickHeldOverGUI = true;
//     if (World.invP1.generalInvCurrent == 0) return; // Access Cards index.
// 
//     int cur = World.invP1.generalInvCurrent;
//     World.invP1.generalInventoryIndexRef[cur] = -1; // Remove item
//     World.invP1.generalInvCurrent -= 1;
//     if (World.invP1.generalInvCurrent < 0) {
//         World.invP1.generalInvCurrent = 0; // Bound to lowest, but only
//     }									   // since it is Access Cards.
// 
// 
//     cur = World.invP1.generalInvCurrent;
//     if (World.invP1.generalInventoryIndexRef[cur] < 0) {
//         for (int i=13; i >= 0; i--) {
//             if (World.invP1.generalInventoryIndexRef[i] >= 0) {
//                 World.invP1.generalInvCurrent = i;
//                 break; // Found last item in inventory.
//             }
//         }
//     }
// 
//     cur = World.invP1.generalInvCurrent;
//     int indexRef = World.invP1.generalInventoryIndexRef[cur];
//     if (World.invP1.generalInvCurrent == 0) {
//         if (World.invP1.HasAnyAccessCards()) {
//             Sys_UI.SendInfoToItemTab(indexRef);
//         } else {
//             // If no access cards, reset item tab to show nothing.
//             Sys_UI.SendInfoToItemTab(-1);
//             PtrExit();
//         }
//     } else {
//         GeneralInvButton genbut = World.invP1.genButtons[cur].GetComponent<GeneralInvButton>();
//         Sys_UI.SendInfoToItemTab(indexRef,genbut.customIndex);
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
    World.creditsActive    = true;
    World.creditsPageIndex = 0;
    creditsVidStartTime          = World.absoluteTime;
    creditsVidFinished           = World.absoluteTime + 37.2;
    creditsVidPhase              = 0;
    // TODO: start outro.webm video playback via engine video player
    // TODO: render Sys_Text.stringTable[610] as endVideoText1 (phase 0)
    // TODO: render Sys_Text.stringTable[611] as endVideoText2 (phase 1)
    // TODO: render Sys_Text.stringTable[612] as endVideoText3 (phase 2)
}

void CreditsUpdate(void) {
    if (!World.creditsActive) return;
    double elapsed = World.absoluteTime - creditsVidStartTime;

    // Drive video text phase transitions
    if (creditsVidFinished > 0.0) {
        if (elapsed >  7.0 && creditsVidPhase == 0) creditsVidPhase = 1; // TODO: swap text1->text2 visibility
        if (elapsed > 11.0 && creditsVidPhase == 1) creditsVidPhase = 2; // TODO: swap text2->text3 visibility
        if (elapsed > 14.0 && creditsVidPhase == 2) creditsVidPhase = 3; // TODO: hide text3
        if (World.absoluteTime >= creditsVidFinished) {
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
        if (!(World.creditsPageIndex >= CREDITS_PAGES)) {
            ++World.creditsPageIndex;
            if (!World.gameFinished && World.creditsPageIndex == 1) ++World.creditsPageIndex; // skip stats page when not finishing game
            if (World.creditsPageIndex >= CREDITS_PAGES) World.creditsPageIndex = CREDITS_PAGES; // bottom
        } else {
            World.creditsActive = false;
            MenuGoBack();
        }
        return;
    }

    if (ToggleMode()) { // right click — go back a page
        if (World.creditsPageIndex > 0) --World.creditsPageIndex;
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
    Entity* e = &World.instances[self];
    e->volume        = CYBERWALL_ALPHA_MIN;
    e->tickFinished  = World.pauseRelativeTime + CYBERWALL_INIT_DELAY;
    e->animSwapFinished = 0.0;
    // TODO: push e->volume to chunk_frag.glsl as _CenterAlpha uniform or
    // per-instance draw param for this geometry instance's material slot
}

void CyberWallUpdate(u16 self) {
    Entity* e = &World.instances[self];
    if (World.pauseRelativeTime < e->tickFinished) return;
    if (e->volume > CYBERWALL_ALPHA_MIN) {
        e->volume -= CYBERWALL_ALPHA_STEP;
        if (e->volume < CYBERWALL_ALPHA_MIN) e->volume = CYBERWALL_ALPHA_MIN;
    }
    e->tickFinished = World.pauseRelativeTime + CYBERWALL_TICK;
}

void CyberWallHit(u16 self) { // Called by projectile hit, collision, or ConwaySignal propagation from adjacent wall
    Entity* e = &World.instances[self];
    e->volume = CYBERWALL_ALPHA_MAX;
    // TODO: push e->volume to renderer as _CenterAlpha for this instance
}

void CyberWallConwaySignal(u16 self) { // Called when a conway propagation signal arrives from a neighbour TODO Conway's game of life propagation on world x,z plane
    World.instances[self].animSwapFinished = World.pauseRelativeTime + CYBERWALL_CONWAY_TIME;
}
//=============================================================================
// CyborgConversionToggle
void CyborgConversionToggleTargetted(void) {
    bool active = (World.ressurectionActiveLevels >> World.curLev) & 1u;
    flag_setu16(&World.ressurectionActiveLevels,(1u << World.curLev),!active);
    if (World.curLev == 6) flag_setu16(&World.ressurectionActiveLevels, (1u<<10|1u<<11|1u<<12),!active); // Set groves 10,11,12 when 6 gets toggled as they don't have their own switch
    play_wav(sounds[active ? 183 : 184],Sys_Settings.VolumeMessage,(V3){},false); // "vox_cybconvcancelled" : "vox_cybconvenabled"
    CenterStatusPrint("%s",Sys_Text.stringTable[active ? 591 : 592]);
}
//=============================================================================
// ElevatorButton
// static const char* elevFloorLabels[14] = {"R","1","2","3","4","5","6","7","8","9","G1","G2","G4","C"};
void ElevatorButtonClick(u16 self) {
    Entity* e = &World.instances[self];
    Sys_UI.mouseClickHeldOverGUI = true;
    if (Sys_UI.linkedElevatorDoor == U16_MAX) {
        CenterStatusPrint("%s",Sys_Text.stringTable[6]); // Too far away from that.
        return;
    }
    Entity* door = &World.instances[Sys_UI.linkedElevatorDoor];
    bool doorClosed = door->doorOpen == DoorState_Closed;
    float dist = V3_Dist(Sys_UI.objectInUsePos,World.position[PLAYER1]);
    if (dist > ELEVATOR_PAD_TETHER_DIST && !doorClosed) {
        CenterStatusPrint("%s",Sys_Text.stringTable[6]); // Too far away from that.
        return;
    }
    if (!doorClosed) {
        CenterStatusPrint("%s",Sys_Text.stringTable[7]); // Door not closed.
        return;
    }
    if (!(e->entflags & EF_ACTIVE)) {
        CenterStatusPrint("%s",Sys_Text.stringTable[8]); // Floor not accessible.
        return;
    }
    // TODO: call engine LoadLevel(e->teleportID, spawnPos) — destination spawn
    // position comes from position[e->targetDestinationID] if set
    // (targetDestinationID != U16_MAX), else V3 zero.
//     if (floorAccessible) { // floorAccessible set from the us_puz_elevatorkeypad, us_puz_elevatorkeypad2, us_puz_elevatorkeypad3, or us_puz_elevatorkeypad4 entity
//         LoadLevel(levelIndex,targetDestination.World.position[i]);
//     } else {
//         CenterStatusPrint("%s", Sys_Text.stringTable[8]);
//     }
}
//=============================================================================
// Email
void EmailTargetted(u16 self, u16 activator) {
    (void)activator; Entity* e = &World.instances[self]; u16 idx = e->emailIndex;
    InventorySystem* inv = &World.invP1;
    if (inv->hasLog[idx]) return;
    
    inv->hasLog[idx] = inv->hasNewEmail = true; inv->lastAddedIndex = idx;
    if (Sys_Text.audioLogType[idx] == AudioLogType_Email) inv->beepDone = true;
    if (e->autoPlayEmail) (void)0; // TODO: PlayLastAddedLog(idx) — trigger auto-play of log audio
}
//=============================================================================
// EnergyOverloadButton
#define OVERLOAD_CLICK_DEBOUNCE 0.4
#define OVERLOAD_HEAT_THRESHOLD 25.0f
static double overloadClickFinished = 0.0;

void OverloadButtonAction(void) {
    if (overloadClickFinished >= World.pauseRelativeTime) return;
    overloadClickFinished = World.pauseRelativeTime + OVERLOAD_CLICK_DEBOUNCE;
    InventorySystem* inv = &World.invP1;
    if (inv->currentEnergyWeaponHeat[inv->weaponIndex] > OVERLOAD_HEAT_THRESHOLD) {
        CenterStatusPrint("%s",Sys_Text.stringTable[12]); // Weapon too hot
        return;
    }
    if (inv->overloadEnabled) {
        CenterStatusPrint("%s",Sys_Text.stringTable[13]); // Overload disabled
        inv->overloadEnabled = false;
        // Render-time: normalButtonSprite, textClickableColor, stringTable[16]
    } else {
        CenterStatusPrint("%s",Sys_Text.stringTable[17]); // Overload enabled
        inv->overloadEnabled = true;
        // Render-time: overloadButtonSprite, textOverloadColor, stringTable[18]
    }
}

void OverloadEnergyClick(void) {
    Sys_UI.mouseClickHeldOverGUI = true;
    OverloadButtonAction();
}

// Called by weapon fire system after overload shot discharges
void OverloadFired(void) {
    World.invP1.overloadEnabled = false;
    // Render-time: normalButtonSprite, textDisabledColor
}

// Called from weapon pane render — returns visual state for renderer to act on
// 0 = normal+clickable, 1 = overloaded, 2 = disabled (post-fire/too hot)
u8 OverloadButtonVisualState(void) {
    InventorySystem* inv = &World.invP1;
    if (inv->currentEnergyWeaponHeat[inv->weaponIndex] > OVERLOAD_HEAT_THRESHOLD) return 2;
    if (inv->overloadEnabled) return 1;
    return 0;
}
//=============================================================================
// GeneralInventory
static void ApplyBattery(void) {
    InventorySystem* inv = Inv(PLAYER1);
    if (inv->energy >= 255.0f) { CenterStatusPrint("%s",Sys_Text.stringTable[303]); return; } // Energy full
    
    GiveEnergy(83.0f,EnergyType_Battery);
    inv->generalInventoryIndexRef[inv->hardwareInvCurrent] = -1;
}

static void ApplyIcadBattery(void) {
    InventorySystem* inv = Inv(PLAYER1);
    if (inv->energy >= 255.0f) { CenterStatusPrint("%s",Sys_Text.stringTable[303]); return; } // Energy full
    
    GiveEnergy(255.0f,EnergyType_Battery);
    inv->generalInventoryIndexRef[inv->hardwareInvCurrent] = -1;
}

static void ApplyHealthkit(void) {
    InventorySystem* inv = Inv(PLAYER1);
    if (inv->energy >= 255.0f) { CenterStatusPrint("%s",Sys_Text.stringTable[303]); return; } // Energy full
    
    World.instances[PLAYER1].health = 255.0f;
    // TODO: Sys_UI.DrawTicks(true) — HUD health tick refresh, MFDManager
    inv->generalInventoryIndexRef[inv->hardwareInvCurrent] = -1;
}

void GeneralInvUse(int buttonIdx,int customIdx) {
    InventorySystem* inv = Inv(PLAYER1);
    inv->hardwareInvCurrent = buttonIdx;
    int itemIdx = inv->generalInventoryIndexRef[buttonIdx];
    if (buttonIdx == 0) {
        // TODO: Sys_UI.SendInfoToItemTab(81) — access cards display, MFDManager
        // TODO: SetCurrentAsLast for active side panel, MFDManager
        return;
    }
    // TODO: Sys_UI.SendInfoToItemTab(itemIdx,customIdx) — MFDManager
    // TODO: SetCurrentAsLast for active side panel, MFDManager
    (void)customIdx;
    (void)itemIdx;
}

void GeneralInvApply(int buttonIdx,int customIdx) {
    if (buttonIdx == 0) {
        // TODO: Sys_UI.SendInfoToItemTab(81), OpenTab access cards — MFDManager
        return;
    }
    World.invP1.hardwareInvCurrent = buttonIdx;
    int itemIdx = World.invP1.generalInventoryIndexRef[buttonIdx];
    switch (itemIdx) {
        case 52: ApplyBattery();     break;
        case 53: ApplyIcadBattery(); break;
        case 55: ApplyHealthkit();   break;
        default:
            World.invP1.hardwareInvCurrent = buttonIdx;
            // TODO: Sys_UI.SendInfoToItemTab(itemIdx,customIdx), OpenTab — MFDManager
            (void)customIdx;
            break;
    }
}

void GeneralInvClick(int buttonIdx,int customIdx) { Sys_UI.mouseClickHeldOverGUI = true; GeneralInvUse(buttonIdx,customIdx); }
void GeneralInvDoubleClick(int buttonIdx,int customIdx) { Sys_UI.mouseClickHeldOverGUI = true; GeneralInvApply(buttonIdx,customIdx); }
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
    u8 ver = World.invP1.hardwareVersion[HW_TID_IDX];
    if (manual) return (ver >= 4) ? 18.0f : 13.0f;
    return (ver <= 2) ? 0.0f : ((ver == 3) ? 13.0f : 20.0f);
}

float TargetIDGetTetherRange(void) { return (World.invP1.hardwareVersion[HW_TID_IDX] >= 4) ? 22.0f : 15.0f; }
static void TargetIDDeactivate(u16 self) {
    Entity* e = &World.instances[self];
    if (e->enemy != WORLD) {
        Entity* npc = &World.instances[e->enemy];
        flag_set(&npc->entflags,EF_TARGID_ATTACHED,false);
        e->enemy = NULLENT;
    }
    e->textIndex  = -1;
    flag_set(&e->entflags,EF_ACTIVE,false);
}

void TargetIDSendDamageReceive(u16 self,float damage,AttackType attackType) {
    Entity* e   = &World.instances[self];
    if (e->enemy == NULLENT) return;
    Entity* npc = &World.instances[e->enemy];
    if (attackType == AttackType_Tranq) {
        e->textIndex         = 536; // STUNNED
        e->animSwapFinished  = World.pauseRelativeTime - 1.0; // expire damage text
    } else {
        float mh = npcTable[npc->index - 419].health;
        if      (damage > mh * 0.75f) e->textIndex = 514; // SEVERE DAMAGE
        else if (damage > mh * 0.50f) e->textIndex = 515; // MAJOR DAMAGE
        else if (damage > mh * 0.25f) e->textIndex = 513; // NORMAL DAMAGE
        else if (damage > 0.0f)       e->textIndex = 512; // MINOR DAMAGE
        else                          e->textIndex = 511; // NO DAMAGE
        e->animSwapFinished = World.pauseRelativeTime
                              + ((damage == 0.0f) ? TARGETID_DAMAGE_TIME_MISS
                                                  : TARGETID_DAMAGE_TIME_HIT);
    }
}

void TargetIDUpdate(u16 self) {
    Entity* e = &World.instances[self];
    if (!(e->entflags & EF_ACTIVE)) return;

    // Deactivation checks
    if (e->enemy == NULLENT) { TargetIDDeactivate(self); return; }
    Entity* npc = &World.instances[e->enemy];
    if (npc->health <= 0.0f) { TargetIDDeactivate(self); return; }
    if (V3_Dist(World.position[self],World.position[PLAYER1]) > TARGETID_LINK_DIST) { TargetIDDeactivate(self); return; }
    if (e->tickFinished < World.pauseRelativeTime) { TargetIDDeactivate(self); return; }

    SetPosition(self,World.position[e->enemy],true); // Track parent NPC position
    bool stunned = npc->tranquilizeFinished > World.pauseRelativeTime;
    flag_set(&e->entflags,EF_ASLEEP,stunned);
    if (e->textIndex >= 0) {
        if (stunned && e->animSwapFinished < World.pauseRelativeTime) e->textIndex = 536; // STUNNED
        else if (e->animSwapFinished < World.pauseRelativeTime) {
            e->textIndex = -1;
            if (!(World.invP1.hasHardware & HW_TID)) { TargetIDDeactivate(self); return; }
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
    Entity* e        = &World.instances[self];
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
    for (u16 i = INSTS_1ST_IDX; i < World.instCount; i++) {
        Entity* e = &World.instances[i];
        if (!(e->entflags & EF_ACTIVE))         continue;
        if (!IdxIsNPC(e->index))              continue;
        if (e->entflags & EF_DEAD)              continue;
        if (e->entflags & EF_TARGID_ATTACHED)   continue;
        if (V3_Dist(World.position[i],World.position[PLAYER1]) > TargetIDGetSensingRange(false))        continue;
        // TODO: CreateTargetIDInstance — weapon/targetting system
    }
}

bool ModRequestsGrayscale(void) {return (/*(World.invP1.hasHardware & HW_INF) && */(World.invP1.hardwareIsActive & HW_INF) > 0); }
static void DeactivateHardwareOnEnergyDepleted(void) {
    InventorySystem* inv = &World.invP1;
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
    if (Cheats.redbull)        return;
    inv->energy -= take;
    if (inv->energy <= 0.0f) {
        inv->energy = 0.0f;
        play_wav(sounds[84],Sys_Settings.VolumeEffects,(V3){},false); // energy_gone
        CenterStatusPrint("%s",Sys_Text.stringTable[314]); // Power supply exhausted.
        DeactivateHardwareOnEnergyDepleted();
    }
}

void GiveEnergy(float give,EnergyType type) {
    InventorySystem* inv = Inv(PLAYER1);
    inv->energy += give;
    if (inv->energy > 255.0f) inv->energy = 255.0f;
    if (type == EnergyType_Battery)       play_wav(sounds[79], Sys_Settings.VolumeEffects,(V3){},false); // batteryuse
    if (type == EnergyType_ChargeStation) play_wav(sounds[100],Sys_Settings.VolumeEffects,(V3){},false); // chargingstation
}

void PlayerEnergyInit(void) {
    InventorySystem* inv = Inv(PLAYER1);
    inv->energy = 54.0f;
    inv->energyDrainTickFinished = World.pauseRelativeTime + ENERGY_TICK + random_range(0.0f,1.0f);
    inv->drainJPM = 0;
}

void PlayerEnergyUpdate(void) {
    InventorySystem* inv = &World.invP1;
    if (inv->hasHardware & HW_TID) TargetIdentifierSenseTargets();
    if (inv->energyDrainTickFinished > World.pauseRelativeTime) return;
    
    inv->energyDrainTickFinished = World.pauseRelativeTime + ENERGY_TICK;
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
static bool GrenadeIsNPCMine(u16 self) { return World.instances[self].layer != L_PlayerBullets; }
void GrenadeExplode(u16 self) {
    Entity* e = &World.instances[self];
    // TODO: DamageData + ApplyImpactForceSphere(damage,attackType,penetration,offense,damage*1.5f,World.position[self],e->strength,1.0f)
    if (!GrenadeIsNPCMine(self)) { Entity* p = &World.instances[PLAYER1]; p->noiseFinished = World.pauseRelativeTime + 2.0; }
    i16 idx = (i16)e->index;
    int soundIndex = 60;
    switch (idx) {
        case 7: case 11: soundIndex = 64; World.fogFac += 5;  break; // frag, mine
        case 8: case 10: soundIndex = 60; World.fogFac += 7;  break; // conc, earth
        case 9:  soundIndex = 67;                           break; // emp
        case 12: soundIndex = 60; World.fogFac += 6;  break; // nitro
        case 13: soundIndex = 63; World.fogFac += 10; break; // gas
    }
    
    play_wav(sounds[soundIndex],1.0f,World.position[self],true);
    // TODO: SpawnExplosionEffect(World.position[self], explosionType)
    // TODO: Shake(-1,-1) — screen shake system
    DeleteInstance(self);
}

void GrenadeActivate(u16 self) {
    Entity* e = &World.instances[self];
    i16 idx = (i16)e->index;
    switch (idx) {
//         case 7: case 8: case 9: flag_set(&e->ioflags,GREN_FLAG_EXPLODE_CONTACT,true); break;
//         case 10: e->timerFinished = World.pauseRelativeTime + World.invP1.earthShakerTimeSetting; flag_set(&e->ioflags,GREN_FLAG_USE_TIMER,true); break;
//         case 11: flag_set(&e->ioflags,GREN_FLAG_USE_PROX,true); flag_set(&e->ioflags,GREN_FLAG_EXPLODE_CONTACT,false);                                        break;
//         case 12: e->timerFinished = World.pauseRelativeTime + World.invP1.nitroTimeSetting; flag_set(&e->ioflags,GREN_FLAG_USE_TIMER,true);       break;
//         case 13: flag_set(&e->ioflags,GREN_FLAG_EXPLODE_CONTACT,true); break;
        default: return;
    }
}

void GrenadeUpdate(u16 self) {
    Entity* e = &World.instances[self];
    if ((i16)e->index == 14) { GrenadeExplode(self); return; } // Plastique
    
//     if ((e->ioflags & GREN_FLAG_USE_TIMER) && e->timerFinished < World.pauseRelativeTime) GrenadeExplode(self); TODO
}

// Called by physics collision callback when grenade touches anything
void GrenadeOnCollision(u16 self) { (void)self;
//     if (World.instances[self].ioflags & GREN_FLAG_EXPLODE_CONTACT) GrenadeExplode(self); TODO
}
//=============================================================================
// ProjectileEffectImpact
// TODO: GetDamageTakeAmount(DamageData* dd) — weapon/armor calculation system
// TODO: TakeDamage(u16 target, DamageData* dd) -> float — health manager
// TODO: Tranquilize(u16 target, float amount, bool fromProjectile) -> float
// TODO: ApplyImpactForceSphere — needs OverlapSphere from physics, engine-side
// TODO: ApplyImpactForce(u16 target, float vel, V3 normal, V3 pt)
// TODO: SpawnImpactEffect(u16 impactType, V3 pos) — object pool
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static void ProjectileEffectImpactOnCollision(u16 self,u16 hitIdx, V3 hitPos,V3 hitNormal) {
    Entity* e   = &World.instances[self];
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
        .isOtherNPC    = IdxIsNPC(World.instances[hitIdx].index),
        .berserkActive = (World.invP1.patchActive & PATCH_BERSERK) != 0,
    };

    // Railgun sphere impact
    if (e->lookUpIndex == 5) { // TODO: replace magic with named PoolType enum
        // TODO: ApplyImpactForceSphere(dd, World.position[self], 3.2f, 1.0f)
        World.fogFac += 4;
    }

    Entity* hit = &World.instances[hitIdx];
    if (hit->health > 0.0f || hit->cyberHealth > 0.0f) {
        // TODO: dd.damage = GetDamageTakeAmount(&dd)
        if (e->counter < e->countToTrigger) dd.damage *= 0.85f; // per-hit falloff
        dd.impactVelocity = dd.damage * 1.5f;
        if (e->counter > 0) dd.impactVelocity /= 3.0f;
        if (World.curLev != LEVEL_CYBERSPACE
            && e->recentMostActivator == PLAYER1) {
            // TODO: ApplyImpactForce(hitIdx, dd.impactVelocity, dd.attacknormal, hitPos)
        }
        // TODO: float dmgFinal = TakeDamage(hitIdx, &dd)
        float dmgFinal  = 0.0f; // placeholder until TakeDamage implemented
//         float tranq     = -1.0f;
        if (dd.isOtherNPC) {
            if (!(hit->entflags & EF_ASLEEP)) Sys_Music.inCombat = true;
            if (dd.attackType == AttackType_Tranq) {
//                 float stunAmount = vclamp(3.0f + (World.invP1.stungunSetting
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
    Entity* e       = &World.instances[self];
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
    if (World.curLev == LEVEL_CYBERSPACE) return true;
    Entity* e = &World.instances[self];
    if (self != PLAYER1 && e->cyberHealth > 0.0f) return true;
    return (IdxIsNPC(e->index) && (e->index - 419) > 23); // 24-28 are cyber enemies
}

static float ApplyAttackTypeAdjustments(u16 self,float take,AttackType at) {
    Entity* e = &World.instances[self];
    if (!IdxIsNPC(e->index) || e->health <= 0.0f) return take;
    NPCType t = npcTable[e->index - 419].type;
    if (at >= 12) return take;
    return take * attackTypeMult[t][at];
}

static void UseDeathTargets(u16 self) {
    if (self == PLAYER1 || self == PLAYER2) return;
    Entity* e = &World.instances[self];
    if (!sEmpty(e->target)) UseTargets(self,e->target);
}

static void TeleportAway(u16 self) { 
    if (World.instances[self].entflags & EF_TELEPORT_ON_DEATH) {return;}
    flag_set(&World.instances[self].entflags,EF_TELEPORT_ON_DEATH,true);
    World.collider[self] = COLTYPE_NONE;
    World.gravity[self] = 0.0f;
    World.velocity[self] = (V3){0,0,0};
    World.angularVelocity[self] = (V3){0,0,0};
    World.instances[self].modelIndex = U16_MAX;
    /*TODO: activate teleport effect particle instance at self position*/
}

static void DropSearchables(u16 self) {
    // TODO: NotifySearchThatSearchableWasDestroyed();
    for (int i = 0; i < 4; i++) {
        if (World.instances[self].contents[i] == U16_MAX) continue;
        u16 spawned = SpawnDynamicObject(World.instances[self].contents[i] + 307,true);
        if (spawned != U16_MAX) {
            World.position[spawned] = World.position[self];
            World.instances[spawned].customIndex[0] = World.instances[self].customIndex[i];
        } else CenterStatusPrint("BUG: Failed to instantiate object being dropped on gib.");
        World.instances[self].contents[i] = World.instances[self].customIndex[i] = U16_MAX;
    }
}

static void CreateDeathEffects(u16 self,u16 fxPoolType) { if (fxPoolType == 0) {return; /*PoolType_None*/} V3 pos = World.position[self]; if (World.collider[self] != COLTYPE_NONE) { pos = V3_AplusB(pos,World.colliderCenter[self]); } /*TODO: SpawnEffectFromPool(fxPoolType, pos);*/ }
static void HideSelf(u16 self) { if (World.instances[self].index == 279) {return; /*tv screens keep mesh visible*/} World.instances[self].modelIndex = MAX_MDLS; World.instances[self].gravity = 0.0f; }
static void NPCDeath(u16 self) {
    if (World.instances[self].entflags & EF_DEAD_CHECKS_DONE) {return;}
    flag_set(&World.instances[self].entflags,EF_DEAD_CHECKS_DONE,true);
    CreateDeathEffects(self,World.instances[self].deathBurst);
    if (World.instances[self].index == 419) play_wav(sounds[64],1.0f,World.position[self],true); // npc_autobomb: explosion1
    if (npcTable[World.instances[self].index - 419].type == NPCType_Cyber) DeleteInstance(self);
    // else: keep collider alive to prevent falling through floor (Unity physics note preserved)
}

static void ObjectDeath(u16 self) {
    Entity* e = &World.instances[self];
    if (World.instances[self].entflags & EF_DEAD_CHECKS_DONE) return;
    if (World.instances[self].entflags & EF_DEATH_BURST_DONE) { // gibOnDeath reuses DEATH_BURST_DONE
        // Gib path
        CreateDeathEffects(self,World.instances[self].deathBurst);
        DropSearchables(self);
        if (World.instances[self].index != 279) World.collider[self] = COLTYPE_NONE;
        HideSelf(self);
    } else {
        World.collider[self] = COLTYPE_NONE;
        DropSearchables(self);
        CreateDeathEffects(self,World.instances[self].deathBurst);
    }
    flag_set(&World.instances[self].entflags,EF_DEAD_CHECKS_DONE,true);
    // TODO: disable automap overlay for this instance
    if (World.instances[self].securityThreshold > 0) {
        // TODO: ReduceCurrentLevelSecurity(e->securityThreshold) — security system
    }
    u16 idx = World.instances[self].index;
    i16 soundex = 62; // default: crate_break
    if (idx < 527 && objectDeathSound[idx] != 0) soundex = objectDeathSound[idx];
    play_wav(sounds[soundex],1.0f,World.position[self],true);
    if (e->deathBurst != 0) HideSelf(self);
}

static void ScreenDeath(u16 self) {
    Entity* e = &World.instances[self];
    if (e->entflags & EF_DEAD_CHECKS_DONE) return;
    flag_set(&e->entflags,EF_DEAD_CHECKS_DONE,true);
    play_wav(sounds[69],1.0f,World.position[self],true); // screen_destroy
    // TODO: stop ImageSequenceTextureArray animation for this instance
    if (e->entflags & EF_DEATH_BURST_DONE) ObjectDeath(self); // gib path
}

static void VaporizeCorpse(u16 self,bool energyVaporized) {
    Entity* e = &World.instances[self];
    flag_set(&e->entflags,EF_DEAD_CHECKS_DONE,true);
    DropSearchables(self);
    u16 fx = e->deathBurst;
    if (fx == 0) fx = 1; // PoolType_CorpseHit fallback
    if (energyVaporized) fx = 2; // PoolType_Vaporize
    e->modelIndex = MAX_MDLS;
    bool isNPC = IdxIsNPC(e->index);
    bool isSearchable = IdxIsSearchable(e->index);
    if (isNPC || isSearchable) DeleteInstance(self);
    CreateDeathEffects(self,fx);
}

static void Death(u16 self,bool energyVaporized) {
    Entity* e = &World.instances[self];
    if (e->entflags & EF_DEAD_CHECKS_DONE) return;
    UseDeathTargets(self);
    bool isNPC = IdxIsNPC(e->index);
    bool isObj = IdxIsDynamicObject(e->index);
    if (e->entflags & EF_ACT_AS_CORPSE_ONLY) { e->entflags |= EF_DEAD_CHECKS_DONE; return; }
//     bool gib        = (e->entflags & EF_DEATH_BURST_DONE) != 0;
    bool vaporize   = (IdxIsNPC(e->index) && e->health <= 0.0f) || IdxIsCorpse(e->index); // vaporizeCorpse maps to VISIBLE being set
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
    else if (self == PLAYER1 || self == PLAYER2) World.deaths++;
    flag_set(&e->entflags,EF_DEAD_CHECKS_DONE,true);
}

float TakeDamage(u16 self,DamageData dd) {
    if (Cheats.god && (self == PLAYER1 || self == PLAYER2)) return 0.0f;
    bool isCyber = IsCyberEntity(self);
    float* hp = isCyber ? &World.instances[self].cyberHealth : &World.instances[self].health;
    bool isNPC = IdxIsNPC(World.instances[self].index);
    bool isPlayer = (self == PLAYER1 || self == PLAYER2);
//     bool isObj = IdxIsDynamicObject(World.instances[self].index); // TODO
    bool isGrenade = (World.instances[self].entflags & EF_ISGRENADE) != 0;
//     bool isScreen  = (World.instances[self].index == 279); // TODO
    bool isCam = (World.instances[self].index == 477);
    if (isCyber) {
        if (dd.attackType == AttackType_Drill && isNPC) return 0.0f;
        if (dd.attackType != AttackType_Drill && World.instances[self].iceActive) return 0.0f;
    }
    // Dead exceptions — still allow damage to gibs, ice, player, grenades, screens, cameras, teleporters
    if (*hp <= 0.0f) {
        bool allowPost = (isNPC || World.instances[self].iceActive || isPlayer || isGrenade || World.instances[self].index == 279 || isCam);
        if (!allowPost) return 0.0f;
    }

    float take = dd.damage;
    if (isPlayer) {
        float absorb = 0.0f;
        if (isCyber) {
            // Cyber C-Shield software absorption
            if (World.invP1.hasSoft & (1 << SW_SHIELD)) {
                u8 sv = World.invP1.softVersions[SW_SHIELD];
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
            InventorySystem* inv = &World.invP1;
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
                    play_wav(sounds[94],Sys_Settings.VolumeEffects,(V3){},false); // shield absorb
                    int abs = (int)(absorb * 100.0f);
                    CenterStatusPrint("%s%d%s",Sys_Text.stringTable[208],abs,Sys_Text.stringTable[209]);
                    // TODO: shield screen flash effect
                }
            }
            if (take > 0.0f && (absorb < 0.4f || random_range(0.0f,1.0f) < 0.5f)) {
                play_wav(sounds[140],Sys_Settings.VolumeEffects,(V3){},false); // player pain
                // TODO: pstatic.Flash(take>15?2:take>10?1:0) — pain flash FX
            }
            if (dd.owner != NULLENT && IdxIsNPC(World.instances[dd.owner].index))
                World.instances[self].noiseFinished = World.pauseRelativeTime; // justHurtByEnemy for music system
        }
    }

    if (isCyber) {
        World.instances[self].cyberHealth -= take;
        if (isPlayer) {
            World.damageReceived += take;
            // TODO: DrawTicks(true)
            if (World.instances[self].cyberHealth <= 0.0f) {
                // TODO: ExitCyberspace()
                return 0.0f;
            }
        }
        if (dd.owner == PLAYER1 || dd.owner == PLAYER2) World.damageDealt += take;
    } else {
        // Camera constIndex 477 gets one-shot by tranq
        if (World.instances[self].index == 477 && dd.attackType == AttackType_Tranq) take = World.instances[self].health + 1.0f;
        take = ApplyAttackTypeAdjustments(self,take,dd.attackType);
        World.instances[self].health -= take;
        if (isPlayer) {
            World.damageReceived += take;
            Sys_Music.inCombat = true;
            // TODO: DrawTicks(true)
        }
        if (dd.owner == PLAYER1 || dd.owner == PLAYER2) World.damageDealt += take;
    }

    if (isNPC && (World.instances[self].health > 0.0f || (isCyber && World.instances[self].cyberHealth > 0.0f))) {
        if (npcTable[World.instances[self].index - 419].timeBetweenPain > 0.0f) flag_set(&World.instances[self].entflags,EF_GO_INTO_PAIN,true);
        World.instances[self].recentMostActivator = dd.owner; // Pass attacker to NPC
        TargetIDSendDamageReceive(self,take,dd.attackType);
        AICheckPain(self); // setup enemy with NPC
    }

    if (isCyber) {
        if (World.instances[self].cyberHealth <= 0.0f) { if (!World.instances[self].iceActive && isNPC) {World.cyberkills++;} Death(self,false); }
    } else {
        if (World.instances[self].health <= 0.0f) { if (isNPC) {World.kills++;} Death(self,dd.attackType == AttackType_EnergyBeam); }
    }
    return take;
}

void HealthManagerInitAfterLoad(u16 self) {
    bool isPlayer = (self == PLAYER1 || self == PLAYER2);
    bool isNPC    = IdxIsNPC(World.instances[self].index);
    if (isPlayer) {
        World.instances[self].health      = 211.0f;
        World.instances[self].cyberHealth = 255.0f;
        World.instances[self].noiseFinished = World.pauseRelativeTime - 31.0; // guarantee no combat music on start
        return;
    }
    if (isNPC) {
        if (IsCyberEntity(self)) {
            if (World.instances[self].cyberHealth < 0.0f) World.instances[self].cyberHealth = npcTable[World.instances[self].index - 419].healthForCyberNPC;
        } else {
            if (World.instances[self].health < 0.0f) World.instances[self].health = npcTable[World.instances[self].index - 419].health;
        }
        if (World.diffCbt == 0) { World.instances[self].health = 1.0f; }
        if (World.instances[self].entflags & EF_ACT_AS_CORPSE_ONLY) {
            World.instances[self].health = 0.0f; World.instances[self].cyberHealth = 0.0f;
            UseDeathTargets(self);
            if (World.instances[self].entflags & EF_TELEPORT_ON_DEATH) TeleportAway(self);
            else NPCDeath(self);
        }
    }
}
// Lights
V3 lightsNewPosition[LIGHT_COUNT];
i32 AddLight(Light* lit, LightAnimation* lanim) {
    i32 i = loadedLights; loadedLights++; if (loadedLights >= LIGHT_COUNT) { DualLogError("Too many lights %u added in level %d!\n",i,World.curLev); OS_Exit(1); }
    mcpy(&lights[i],lit,sizeof(Light)); mcpy(&lanims[i],lanim,sizeof(LightAnimation));
    lightsNewPosition[i] = lit->pos; flag_set(&lights[i].lflags,LDIRTY,true);
    return i;
}

bool alreadyReadLightOnOnce[LIGHT_COUNT] = {0};
void LoadFieldIntoLight(char* k, char* v, char* il, u32 ln, Light* lit, LightAnimation* lam, u16 lIdx) {
    char* br = StringFindFirstCharWithin(k,'[');
    if (br) {
        int i = parse_numberu32(br + 1,il,ln);
        if (i >= 0 && i < 32) { if(k[12] == 's'){lam->intervalSteps[i] = parse_float(v,il,ln);}else{lam->stepIsLerping[i] = parse_float(v,il,ln);} } /*"intervalSteps[" index 12 is 's', "intervalStepisLerping[" index 12 is 'i'*/
        return;
    }
    static const struct { const char* key; u16 offset; u8 type; } map[] = {
        {"currentStep",    __builtin_offsetof(LightAnimation,currentStep),1},{"lerpValue",      __builtin_offsetof(LightAnimation,lerpValue),0},{"intervalSteps.Length",__builtin_offsetof(LightAnimation,numIntervalSteps),1},{"intervalStepisLerping.Length",__builtin_offsetof(LightAnimation, numLerpSteps),1},
        {"localPosition.x",__builtin_offsetof(Light,pos.x),0},               {"localPosition.y",__builtin_offsetof(Light,pos.y),0},             {"localPosition.z",     __builtin_offsetof(Light,pos.z),0},                    {"localRotation.x",             __builtin_offsetof(Light,spotDir.x),0},
        {"localRotation.y",__builtin_offsetof(Light,spotDir.y),0},           {"localRotation.z",__builtin_offsetof(Light,spotDir.z),0},         {"localRotation.w",     __builtin_offsetof(Light,spotDir.w),0},                {"range",                       __builtin_offsetof(Light,range),0},
        {"spotAngle",      __builtin_offsetof(Light,spotAng),0},             {"minIntensity",   __builtin_offsetof(Light,minIntensity),0},      {"maxIntensity",        __builtin_offsetof(Light,maxIntensity),0},             {"color.r",__builtin_offsetof(Light,col.r),0},{"color.g",__builtin_offsetof(Light,col.g),0},{"color.b",__builtin_offsetof(Light,col.b),0}
    };
    for (int i = 0; i < (int)(sizeof(map)/sizeof(map[0])); i++) {
        if (sEqual(k, map[i].key)) { // Types: 0 = float, 1 = u8.  Check key prefix to decide if pointing at 'lit' or 'lam'
            void* dest = (k[0] == 'l' && k[1] == 'o') ? (void*)lit : (void*)lam;
            if (k[0] == 'r' || k[0] == 's' || k[0] == 'm' || k[0] == 'c') {
                if (k[1] != 'u') dest = (void*)lit; // range, spot, max, color (not currentStep)
            }
            char* ptr = (char*)dest + map[i].offset;
            if (map[i].type == 0) *(float*)ptr = parse_float(v,il,ln);
            else                  *(u8*)ptr = parse_numberu8(v,il,ln);
            return;
        }
    }
    if (sEqual(k,"intensity")) lit->intensity = lit->maxIntensity = parse_float(v,il,ln) * 0.35f;
    else if (sEqual(k,"type")) flag_set(&lit->lflags, (v[0] == 'S') ? LSPOT : LDIR, true);
    else if (sEqual(k,"lightOn") && !alreadyReadLightOnOnce[lIdx]) { alreadyReadLightOnOnce[lIdx] = true; flag_set(&lit->lflags,LIGHTON,parse_bool(v,il,ln)); }
    else if (sEqual(k,"lerpOn")) flag_set(&lit->lflags,LERPON,parse_bool(v,il,ln));
}

#define INFRARED_RANGE 50.35f
#define LANTERN_RANGE 11.52f
Color3 lantCol = (Color3){1.0f,1.0f,1.0f};
u16 headmountedLanternLight;
V3 lanternPos;
float lanternVersionBrightness[3] = {0.875f,1.4f,1.75f};
#define CHGD(a,b) (vabs((a) - (b)) > 0.0001f)
void UpdateLight(u16 i, V3 pos, Color3 col, float range, float intensity, float max, float min, float spotAng, Quaternion spotDir, bool on, bool shad) {
    bool changed = ((!!(lights[i].lflags & SHADON) - shad) || (!!(lights[i].lflags & LIGHTON) -  on) || CHGD(lights[i].range,range) || CHGD(lights[i].pos.x,pos.x) || CHGD(lights[i].pos.y,pos.y) || CHGD(lights[i].pos.z,pos.z));
    lights[i].intensity=intensity; lights[i].minIntensity=min; lights[i].maxIntensity=max; lights[i].spotAng=spotAng; lights[i].spotDir=spotDir; lights[i].col=col; lights[i].pos=lightsNewPosition[i]=pos; lights[i].range=range;
    flag_set(&lights[i].lflags,19,(lights[i].lflags&LDIRTY)|changed<<4|on|shad<<1);
}
#undef CHGD

void mat4_lookat_from(float*,Quaternion*,V3); INLINE void mul_mat4(float*,const float*,const float*); void ExtractFrustumPlanes(float*,FrustumPlane*);
Quaternion cubeQuats[6] = {{0.0f,ONE_OVER_SQRT2,0.0f,ONE_OVER_SQRT2}/*+X:Right*/,{0.0f,-ONE_OVER_SQRT2,0.0f,ONE_OVER_SQRT2}/*-X:Left*/,{-ONE_OVER_SQRT2,0.0f,0.0f,ONE_OVER_SQRT2}/*+Y:Up*/,{ONE_OVER_SQRT2,0.0f,0.0f,ONE_OVER_SQRT2}/*-Y:Down*/,{0.0f,0.0f,0.0f,1.0f}/*+Z:Forward*/,{0.0f,1.0f,0.0f,0.0f}/*-Z:Backward*/ };
void UpdateLights() {
    for (u16 lightIdx = 0; lightIdx < loadedLights; ++lightIdx) {
        V3 lightPos = lightsNewPosition[lightIdx];
        lights[lightIdx].pos = lightPos;
        if (lights[lightIdx].lflags & LDIRTY) { // Marked all as true at level load.
            flag_set(&lights[lightIdx].lflags,LDIRTY,false);
            #pragma GCC unroll 6
            for (int j=0;j<6;++j) { // Update to new position
                mat4_lookat_from((float*)lightView[lightIdx][j],&cubeQuats[j],lightPos);
                mul_mat4((float*)lightViewProj[lightIdx][j],shadowmapsPerspectiveProjection,(float*)lightView[lightIdx][j]);
                ExtractFrustumPlanes((float*)lightViewProj[lightIdx][j],lightFrustumPlanes[lightIdx][j]);
            }
        }
    }
    if (!World.paused && !World.menuActive) {
        for (int i=0;i<loadedLights;++i) { // Just lerps/flickers in intensity
            if (lanims[i].numIntervalSteps < 1) continue;
            if (!(lights[i].lflags & LIGHTON)) { lights[i].intensity = 0.0f; continue; }
            if (lanims[i].lerpTime < (float)World.pauseRelativeTime) {
                lights[i].intensity = lanims[i].lerpUp ? lights[i].maxIntensity : lights[i].minIntensity; // Pick target to lerp towards
                lanims[i].lerpUp = !lanims[i].lerpUp;
                lanims[i].currentStep++; if (lanims[i].currentStep >= lanims[i].numIntervalSteps) lanims[i].currentStep = 0; // Wrap and start over continuous looping
                lanims[i].lerpStepTime = lanims[i].intervalSteps[lanims[i].currentStep];
                lanims[i].lerpTime = (float)World.pauseRelativeTime + lanims[i].lerpStepTime;
                lanims[i].lerpStartTime = (float)World.pauseRelativeTime;
            } else if (lights[i].lflags & LERPON) {
                if (lanims[i].currentStep < lanims[i].numLerpSteps) {
                    if (lanims[i].stepIsLerping[lanims[i].currentStep]) {
                        lanims[i].lerpValue = ((float)World.pauseRelativeTime - lanims[i].lerpStartTime)/(lanims[i].lerpTime - lanims[i].lerpStartTime); // percent towards goal time
                        float lerpVal = lanims[i].lerpUp ? lanims[i].lerpValue : (1.0f - lanims[i].lerpValue);
                        lanims[i].lerpValue = lights[i].minIntensity + ((lights[i].maxIntensity - lights[i].minIntensity) * lerpVal);
                        lights[i].intensity = lanims[i].lerpValue;
                    }
                }
            }
        }
    }
    glBindBuffer(GL_SSBO,lightsID); glBufferData(GL_SSBO,loadedLights * sizeof(Light),lights,GL_DYNAMIC_DRAW);
    glUseProgram(voxelUpdateSP); glUniform3f(5,World.position[PLAYER1].x,World.position[PLAYER1].y,World.position[PLAYER1].z);
    glDispatchCompute((VOXELS_X+15)/16,(VOXELS_Z+15)/16,1);
}
// Hardware
void HardwareBioOff(void) {
    World.invP1.hardwareIsActive &= ~HW_BIO;
    if (Cheats.showFPS) return;
    // TODO: BiomonitorClearGraphs() — engine-side graph reset
    // TODO: deactivate bioMonitorContainer — engine reads BioMonitorActive()
}
void HardwareBioOn(void) {
    World.invP1.hardwareIsActive |= HW_BIO;
    // TODO: activate bioMonitorContainer — engine reads BioMonitorActive()
}
void HardwareBioAction(void) {
    InventorySystem* inv = Inv(PLAYER1);
    if (World.invP1.hardwareVersionSetting[HW_BIO_IDX] == 0 && inv->energy <= 0.0f) { CenterStatusPrint("%s",Sys_Text.stringTable[314]); return; }
    
    play_wav(sounds[78],SfxVol(),(V3){},false);
    if (BioMonitorActive(PLAYER1)) HardwareBioOff(); else HardwareBioOn();
}
void HardwareBioClick(void) { Sys_UI.mouseClickHeldOverGUI = true; HardwareBioAction(); }

void HardwareSensaroundOn(void) {
    World.invP1.hardwareIsActive |= HW_SNS;
    // TODO: activate sensaround cameras/overlays — engine reads hardwareIsActive & HW_SNS + version
}
void HardwareSensaroundOff(void) {
    World.invP1.hardwareIsActive &= ~HW_SNS;
    // TODO: deactivate sensaround cameras, restore tabs — engine reads hardwareIsActive & HW_SNS
}
void HardwareSensaroundAction(void) {
    InventorySystem* inv = Inv(PLAYER1);
    if (inv->energy <= 0.0f) { CenterStatusPrint("%s",Sys_Text.stringTable[314]); return; }
    
    if (inv->hardwareIsActive & HW_SNS) {
        play_wav(sounds[82],SfxVol(),(V3){},false); HardwareSensaroundOff();
    } else {
        play_wav(sounds[93],SfxVol(),(V3){},false); HardwareSensaroundOn();
    }
}

void HardwareSensaroundClick(void) { Sys_UI.mouseClickHeldOverGUI = true; HardwareSensaroundAction(); }

void HardwareShieldOn(void) {
    World.invP1.hardwareIsActive |= HW_SHD;
    // TODO: ShieldActivateFX — engine reads hardwareIsActive & HW_SHD
}

void HardwareShieldOff(void) { World.invP1.hardwareIsActive &= ~HW_SHD; }
void HardwareShieldOffWithEffects(void) {
    HardwareShieldOff();
    // TODO: ShieldDeactivateFX — engine reads hardwareIsActive & HW_SHD
}

void HardwareShieldAction(void) {
    InventorySystem* inv = Inv(PLAYER1);
    if (inv->energy <= 0.0f) { CenterStatusPrint("%s",Sys_Text.stringTable[314]); return; }
    
    if (World.invP1.hardwareIsActive & HW_SHD) {
        play_wav(sounds[95],SfxVol(),(V3){},false); HardwareShieldOffWithEffects();
    } else {
        play_wav(sounds[96],SfxVol(),(V3){},false); HardwareShieldOn();
    }
}

void HardwareShieldClick(void) { Sys_UI.mouseClickHeldOverGUI = true; HardwareShieldAction(); }

void HardwareLanternOn(void) {
    World.invP1.hardwareIsActive |= HW_LAN;
    // TODO: enable headlight at lanternBrightness[hardwareVersionSetting[HW_LAN_IDX]] — engine reads bitmask + version
}
void HardwareLanternOff(void) {
    World.invP1.hardwareIsActive &= ~HW_LAN;
    // TODO: disable headlight — engine reads hardwareIsActive & HW_LAN
}

void HardwareLanternAction(void) {
    InventorySystem* inv = Inv(PLAYER1);
    if (inv->energy <= 0.0f) { CenterStatusPrint("%s",Sys_Text.stringTable[314]); return; }
    
    play_wav(sounds[78],SfxVol(),(V3){},false);
    if (World.invP1.hardwareIsActive & HW_LAN) HardwareLanternOff(); else HardwareLanternOn();
}

void HardwareLanternClick(void) { Sys_UI.mouseClickHeldOverGUI = true; HardwareLanternAction(); }

void HardwareInfraredOn(void) {
    World.invP1.hardwareIsActive |= HW_INF;
    // TODO: enable infrared light + grayscale on player/sensaround cameras — engine reads bitmask
}

void HardwareInfraredOff(void) {
    World.invP1.hardwareIsActive &= ~HW_INF;
    // TODO: disable infrared light + grayscale — engine reads bitmask
}
void HardwareInfraredAction(void) {
    InventorySystem* inv = Inv(PLAYER1);
    if (inv->energy <= 0.0f) { CenterStatusPrint("%s",Sys_Text.stringTable[314]); return; }
    
    bool wasOn = (World.invP1.hardwareIsActive & HW_INF) != 0;
    play_wav(wasOn ? sounds[82] : sounds[98],SfxVol(),(V3){},false);
    if (wasOn) HardwareInfraredOff(); else HardwareInfraredOn();
}

void HardwareInfraredClick(void) { Sys_UI.mouseClickHeldOverGUI = true; HardwareInfraredAction(); }

void HardwareEReaderAction(void) {
    play_wav(sounds[97],SfxVol(),(V3){},false);
    World.invP1.hardwareIsActive |= HW_ERD;
    // TODO: OpenEReaderInItemsTab() — engine-side tab open
}

void HardwareEReaderClick(void) { Sys_UI.mouseClickHeldOverGUI = true; HardwareEReaderAction(); }

void HardwareBoosterOn(void)  { World.invP1.hardwareIsActive |=  HW_BST; }
void HardwareBoosterOff(void) { World.invP1.hardwareIsActive &= ~HW_BST; }
void HardwareBoosterAction(void) {
    InventorySystem* inv = Inv(PLAYER1);
    if (BoosterSetToBoost(PLAYER1) && inv->energy <= 0.0f) { CenterStatusPrint("%s",Sys_Text.stringTable[314]); return; }
    
    play_wav(sounds[78],SfxVol(),(V3){},false);
    if (World.invP1.hardwareIsActive & HW_BST) HardwareBoosterOff(); else HardwareBoosterOn();
}

void HardwareBoosterClick(void) { Sys_UI.mouseClickHeldOverGUI = true; HardwareBoosterAction(); }

void HardwareJumpJetsOn(void)  { World.invP1.hardwareIsActive |=  HW_JET; }
void HardwareJumpJetsOff(void) { World.invP1.hardwareIsActive &= ~HW_JET; }
void HardwareJumpJetsAction(u16 p) {
    InventorySystem* inv = Inv(p);
    if (inv->energy <= 0.0f) { CenterStatusPrint("%s",Sys_Text.stringTable[314]); return; }
    
    play_wav(sounds[78],SfxVol(),(V3){},false);
    JumpJetsToggle(PLAYER1);
    if (JumpJetsActive(PLAYER1)) HardwareJumpJetsOn(); else HardwareJumpJetsOff();
}

void HardwareJumpJetsClick(u16 p) { Sys_UI.mouseClickHeldOverGUI = true; HardwareJumpJetsAction(p); }
void HardwareUpdate(u16 p) {
    InventorySystem* inv = Inv(p);
    bool infraredOn = /*(inv->hasHardware & HW_INF) && */(inv->hardwareIsActive & HW_INF) > 0;
    bool lanternOn = /*(inv->hasHardware & HW_LAN) && */(inv->hardwareIsActive & HW_LAN) > 0;
    if (lanternOn || infraredOn) { // Update headmounted lantern/infrared's light (infrared overrides lantern brightness/range)
        V3 ppos = World.position[p];
        lanternPos = (V3){ppos.x + 0.04f,ppos.y + 0.24f,ppos.z + 0.04f};
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
    World.timeScale   = DEFAULT_TIME_SCALE;
    World.geniusActive = false;
    // TODO: sightLight disabled, sightDimming disabled — engine reads patchActive & PATCH_SIGHT + sightFinishedTime
    // TODO: BerserkFX disabled — engine reads patchActive & PATCH_BERSERK + berserkIncrement
}

void PatchUpdate(u16 playerIdx) {
    InventorySystem* inv = Inv(playerIdx);
    if (inv->patchActive & PATCH_DETOX) { // Detox
        if (inv->detoxFinishedTime < World.pauseRelativeTime) inv->patchActive -= PATCH_DETOX;
        // effect: engine reads PATCH_DETOX bit to ameliorate radiation — no gamecode action needed
    }

    if (inv->patchActive & PATCH_MEDI) { // Medi
        if (inv->mediFinishedTime < World.pauseRelativeTime && inv->mediFinishedTime != -1.0)
            { inv->patchActive -= PATCH_MEDI; inv->mediFinishedTime = -1.0; }
    }

    // Reflex — uses absoluteTime (wall clock) so timescale doesn't affect own expiry
    if (inv->patchActive & PATCH_REFLEX) {
        if (inv->reflexFinishedTime < World.absoluteTime && inv->reflexFinishedTime != -1.0) {
            inv->patchActive       -= PATCH_REFLEX;
            inv->reflexFinishedTime = -1.0;
            World.timeScale = DEFAULT_TIME_SCALE;
        } else {
            World.timeScale = REFLEX_TIME_SCALE;
        }
    } else {
        if (World.timeScale != DEFAULT_TIME_SCALE) World.timeScale = DEFAULT_TIME_SCALE;
    }
   
    if (inv->patchActive & PATCH_BERSERK) { // Berserk
        if (inv->berserkFinishedTime < World.pauseRelativeTime) {
            inv->berserkIncrement = 0;
            inv->patchActive -= PATCH_BERSERK;
            // TODO: BerserkFX disable + reset — engine reads patchActive & PATCH_BERSERK
        } else {
            // TODO: BerserkFX enable — engine reads patchActive & PATCH_BERSERK
            if (inv->berserkIncrementFinishedTime < World.pauseRelativeTime) {
                inv->berserkIncrement++;
                if (inv->berserkIncrement > 6) inv->berserkIncrement = 6;
                inv->berserkIncrementFinishedTime = World.pauseRelativeTime + (BERSERK_TIME / 5.0f);
                // TODO: engine reads berserkIncrement for texture swap + strength increment
            }
        }
    }

    if (inv->patchActive & PATCH_GENIUS) { // Genius
        if (inv->geniusFinishedTime < World.pauseRelativeTime) {
            inv->patchActive          -= PATCH_GENIUS;
            World.geniusActive = false;
        } else {
            World.geniusActive = true;
        }
    }

    if (inv->patchActive & PATCH_SIGHT) { // Sight
        if (inv->sightFinishedTime < World.pauseRelativeTime && inv->sightFinishedTime != -1.0) {
            inv->sightFinishedTime          = -1.0;
            inv->sightSideEffectFinishedTime = World.pauseRelativeTime + SIGHT_SIDE_EFFECT_TIME;
            // TODO: sightLight disable, sightDimming enable — engine reads sightFinishedTime == -1 + side effect active
        }
        if (inv->sightSideEffectFinishedTime < World.pauseRelativeTime && inv->sightSideEffectFinishedTime != -1.0) {
            inv->sightSideEffectFinishedTime = -1.0;
            inv->sightFinishedTime           = -1.0;
            inv->patchActive                -= PATCH_SIGHT;
            // TODO: sightDimming disable, sightLight disable — engine reads patchActive & PATCH_SIGHT
        }
    }

    if (inv->patchActive & PATCH_STAMINUP) { // Staminup
        if (inv->staminupFinishedTime < World.pauseRelativeTime) {
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
    World.timeScale           = DEFAULT_TIME_SCALE;
    World.geniusActive        = false;
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
	V3[] ressurectionLocation;
	bool[] ressurectionActive;
	u16[] ressurectionBayDoor;
	V3[] elevatorTargetDestinations;
    
	bool RessurectPlayer() {
		if (!ressurectionActive[World.curLev]) return false;

		if (World.curLev == 10 || World.curLev == 11 || World.curLev == 12) {
			LoadLevel(6,ressurectionLocation[currentLevel].position);
			ressurectionBayDoor[6].ForceClose();
		} else {
			if (World.curLev >= 0 || World.curLev < 13) World.instances[PLAYER1].position = ressurectionLocation[World.curLev];
		}

		// Activate death screen and readouts for "BRAIN ACTIVITY SATISFACTORY..." ya debatable right etc. etc.
// 		PlayerReferenceManager.a.playerDeathRessurectEffect.SetActive(true); // TODO
		PlayTrack(TrackType_Revive,MusicType_Override);
		World.instances[PLAYER1].ressurectingFinished = World.pauseRelativeTime + 3f;
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
		CenterStatusPrint("%s", Sys_Text.stringTable[306] + levelSecurity[currentLevel].ToString() + Sys_Text.stringTable[307]);

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
    if (World.invP1.holdingObject) { CenterStatusPrint("%s",Sys_Text.stringTable[311]); return; } // Can't use grenade, hands full

    ForceInventoryMode();  // Inventory mode is turned on when picking something up.
    ResetHeldItem(playerIndex);
    World.invP1.grenadeActive = true;
    CenterStatusPrint("%s%s",Sys_Text.stringTable[index + 326],Sys_Text.stringTable[320]); // activated, grenade is LIVE!
//     switch(index) { // Subtract one from the correct grenade inventory TODO
//         case 7:  World.invP1.heldObject = Const.a.GetPrefab(370); RemoveGrenade(0); break; // Frag
//         case 8:  World.invP1.heldObject = Const.a.GetPrefab(372); RemoveGrenade(3); break; // Concussion
//         case 9:  World.invP1.heldObject = Const.a.GetPrefab(387); RemoveGrenade(1); break; // EMP
//         case 10: World.invP1.heldObject = Const.a.GetPrefab(389); RemoveGrenade(6); break; // Earth Shaker
//         case 11: World.invP1.heldObject = Const.a.GetPrefab(402); RemoveGrenade(4); break; // Land Mine
//         case 12: World.invP1.heldObject = Const.a.GetPrefab(403); RemoveGrenade(5); break; // Nitropak
//         case 13: World.invP1.heldObject = Const.a.GetPrefab(404); RemoveGrenade(2); break; // Gas
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
//     World.instances[WORLD].ioflags |= World.instances[i].ioflags;
//     
//     if (World.instances[i].ioflags & Q_ROBOT_SPAWN_DEACTIVATED) DualLog("Q_ROBOT_SPAWN_DEACTIVATED: 1");
//     if (World.instances[i].ioflags & Q_ISOTOPE_INSTALLED) DualLog("Q_ISOTOPE_INSTALLED: 1");
//     if (World.instances[i].ioflags & Q_SHIELD_ACTIVATED) {
//         DualLog("Q_SHIELD_ACTIVATED: 1");
//         QuestLogNotesManager.a.notes[8].SetActive(true);
//         QuestLogNotesManager.a.checkBoxes[8].isOn = Const.a.questData.ShieldActivated;
//         QuestLogNotesManager.a.labels[8].text = Sys_Text.stringTable[560];
//     }
//     
//     if (World.instances[i].ioflags & Q_LASER_SAFETY_OVERRIDEN) {
//         DualLog("Q_LASER_SAFETY_OVERRIDEN: 1");
//         QuestLogNotesManager.a.notes[7].SetActive(true);
//         QuestLogNotesManager.a.checkBoxes[7].isOn = Const.a.questData.LaserSafetyOverriden;
//         QuestLogNotesManager.a.labels[7].text = Sys_Text.stringTable[559];
//     }
//     
//     if (World.instances[i].ioflags & Q_LASER_DESTROYED) {
//         DualLog("Q_LASER_DESTROYED: 1");
//         if (AutoSplitterData.missionSplitID == 1) AutoSplitterData.missionSplitID++;
//         QuestLogNotesManager.a.notes[9].SetActive(true);
//         QuestLogNotesManager.a.checkBoxes[9].isOn = Const.a.questData.LaserDestroyed;
//         QuestLogNotesManager.a.labels[9].text = Sys_Text.stringTable[561];
//     }
//     
//     if (World.instances[i].ioflags & Q_BETA_GROVE_CYBER_UNLOCKED) {
//         DualLog("Q_BETA_GROVE_CYBER_UNLOCKED: 1");
//         QuestLogNotesManager.a.notes[12].SetActive(true);
//     }
//     
//     if (World.instances[i].ioflags & Q_GROVE_ALPHA_JETTISON_ENABLED) {
//         DualLog("Q_GROVE_ALPHA_JETTISON_ENABLED: 1");
//         QuestLogNotesManager.a.notes[12].SetActive(true);
//     }
//     
//     if (World.instances[i].ioflags & Q_GROVE_BETA_JETTISON_ENABLED) {
//         DualLog("Q_GROVE_BETA_JETTISON_ENABLED: 1");
//         QuestLogNotesManager.a.notes[12].SetActive(true);
//     }
//     
//     if (World.instances[i].ioflags & Q_GROVE_DELTA_JETTISON_ENABLED) {
//         DualLog("Q_GROVE_DELTA_JETTISON_ENABLED: 1");
//         QuestLogNotesManager.a.notes[12].SetActive(true);
//     }
//     
//     if (World.instances[i].ioflags & Q_MASTER_JETTISON_BROKEN) {
//         DualLog("Q_MASTER_JETTISON_BROKEN: 1");
//         if (AutoSplitterData.missionSplitID == 2) AutoSplitterData.missionSplitID++;
//         QuestLogNotesManager.a.notes[12].SetActive(true);
//         QuestLogNotesManager.a.notes[11].SetActive(true);
//         QuestLogNotesManager.a.labels[11].text = Sys_Text.stringTable[563]; // Set:Diagnose and repair broken relay
//     }
//     
//     if (World.instances[i].ioflags & Q_RELAY_428_FIXED) {
//         DualLog("Q_RELAY_428_FIXED: 1");
//         QuestLogNotesManager.a.notes[11].SetActive(true);
//         QuestLogNotesManager.a.checkBoxes[11].isOn = Const.a.questData.Relay428Fixed;
//         QuestLogNotesManager.a.labels[11].text = Sys_Text.stringTable[563]; // Set:Diagnose and repair broken relay
//         QuestLogNotesManager.a.labels[11].text += Sys_Text.stringTable[564]; // Add:: 428.
//     }
//     
//     if (World.instances[i].ioflags & Q_MASTER_JETTISON_ENABLED) {
//         DualLog("Q_MASTER_JETTISON_ENABLED: 1");
//         if (AutoSplitterData.missionSplitID == 3) AutoSplitterData.missionSplitID++;
//         QuestLogNotesManager.a.notes[10].SetActive(true);
//         QuestLogNotesManager.a.checkBoxes[10].isOn = Const.a.questData.MasterJettisonEnabled;
//         QuestLogNotesManager.a.labels[10].text = Sys_Text.stringTable[562];
//     }
//     
//     if (World.instances[i].ioflags & Q_BETA_GROVE_JETTISONED) {
//         DualLog("Q_BETA_GROVE_JETTISONED: 1");
//         if (AutoSplitterData.missionSplitID == 4) AutoSplitterData.missionSplitID++;
//         QuestLogNotesManager.a.notes[12].SetActive(true);
//         QuestLogNotesManager.a.checkBoxes[12].isOn = Const.a.questData.BetaGroveJettisoned;
//         QuestLogNotesManager.a.labels[12].text = Sys_Text.stringTable[565];
//         QuestLogNotesManager.a.notes[13].SetActive(true);
//         QuestLogNotesManager.a.labels[13].text = Sys_Text.stringTable[566];
//     }
//     
//     if (World.instances[i].ioflags & Q_ANTENNA_NORTH_DESTROYED) {
//         DualLog("Q_ANTENNA_NORTH_DESTROYED: 1");
//         QuestLogNotesManager.a.notes[13].SetActive(true);
//     }
//     
//     if (World.instances[i].ioflags & Q_ANTENNA_SOUTH_DESTROYED) {
//         DualLog("Q_ANTENNA_SOUTH_DESTROYED: 1");
//         QuestLogNotesManager.a.notes[13].SetActive(true);
//     }
//     
//     if (World.instances[i].ioflags & Q_ANTENNA_EAST_DESTROYED) {
//         DualLog("Q_ANTENNA_EAST_DESTROYED: 1");
//         QuestLogNotesManager.a.notes[13].SetActive(true);
//     }
//     
//     if (World.instances[i].ioflags & Q_ANTENNA_WEST_DESTROYED) {
//         DualLog("Q_ANTENNA_WEST_DESTROYED: 1");
//         QuestLogNotesManager.a.notes[13].SetActive(true);
//     }
//     
//     if (World.instances[i].ioflags & Q_SELF_DESTRUCT_ACTIVATED) {
//         DualLog("Q_SELF_DESTRUCT_ACTIVATED: 1");
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
//         QuestLogNotesManager.a.labels[14].text = Sys_Text.stringTable[567]; // Set:Engage reactor self-destruct.
//         QuestLogNotesManager.a.labels[15].text = Sys_Text.stringTable[568]; // Set:Escape on escape pod.
//     }
//     
//     if (World.instances[i].ioflags & Q_BRIDGE_SEPARATED) {
//         DualLog("Q_BRIDGE_SEPARATED: 1");
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
//         QuestLogNotesManager.a.labels[14].text = Sys_Text.stringTable[567]; // Set:Engage reactor self-destruct.
//         QuestLogNotesManager.a.notes[16].SetActive(true);
//         QuestLogNotesManager.a.notes[17].SetActive(true);
//         QuestLogNotesManager.a.checkBoxes[16].isOn = true;
//         QuestLogNotesManager.a.labels[16].text = Sys_Text.stringTable[569]; // Set:Access the bridge.
//         QuestLogNotesManager.a.labels[17].text = Sys_Text.stringTable[570]; // Set:Destroy SHODAN.
//     }
//     
//     if (World.instances[i].ioflags & Q_ISOLINEAR_CHIPSET_INSTALLED) DualLog("Q_ISOLINEAR_CHIPSET_INSTALLED: 1");
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
//             QuestLogNotesManager.a.labels[8].text = Sys_Text.stringTable[560];
//         }
//     }
//     if (LaserSafetyOverriden) {
//         Const.a.questData.LaserSafetyOverriden = !Const.a.questData.LaserSafetyOverriden;
//         QuestLogNotesManager.a.checkBoxes[7].isOn = Const.a.questData.LaserSafetyOverriden;
//         if (Const.a.questData.LaserSafetyOverriden) {
//             QuestLogNotesManager.a.notes[7].SetActive(true);
//             QuestLogNotesManager.a.labels[7].text = Sys_Text.stringTable[559];
//         }
//     }
//     if (LaserDestroyed) {
//         Const.a.questData.LaserDestroyed = !Const.a.questData.LaserDestroyed;
//         if (AutoSplitterData.missionSplitID == 1) { AutoSplitterData.missionSplitID++; }
//         QuestLogNotesManager.a.checkBoxes[9].isOn = Const.a.questData.LaserDestroyed;
//         if (Const.a.questData.LaserDestroyed) {
//             QuestLogNotesManager.a.notes[9].SetActive(true);
//             QuestLogNotesManager.a.labels[9].text = Sys_Text.stringTable[561];
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
//             QuestLogNotesManager.a.labels[11].text = Sys_Text.stringTable[563];// Set:Diagnose and repair broken relay
//         }
//     }
//     if (Relay428Fixed) {
//         Const.a.questData.Relay428Fixed = !Const.a.questData.Relay428Fixed;
//         QuestLogNotesManager.a.checkBoxes[11].isOn = Const.a.questData.Relay428Fixed;
//         if (Const.a.questData.Relay428Fixed) {
//             QuestLogNotesManager.a.notes[11].SetActive(true);
//             QuestLogNotesManager.a.labels[11].text = Sys_Text.stringTable[563]; // Set:Diagnose and repair broken relay
//             QuestLogNotesManager.a.labels[11].text += Sys_Text.stringTable[564]; // Add:: 428.
//         }
//     }
//     if (MasterJettisonEnabled) {
//         Const.a.questData.MasterJettisonEnabled = !Const.a.questData.MasterJettisonEnabled;
//         QuestLogNotesManager.a.checkBoxes[10].isOn = Const.a.questData.MasterJettisonEnabled;
//         if (Const.a.questData.MasterJettisonEnabled) {
//             QuestLogNotesManager.a.notes[10].SetActive(true);
//             QuestLogNotesManager.a.labels[10].text = Sys_Text.stringTable[562];
//         }
//     }
//     if (BetaGroveJettisoned) {
//         Const.a.questData.BetaGroveJettisoned = !Const.a.questData.BetaGroveJettisoned;
//         QuestLogNotesManager.a.checkBoxes[12].isOn = Const.a.questData.BetaGroveJettisoned;
//         if (Const.a.questData.BetaGroveJettisoned ) {
//             QuestLogNotesManager.a.notes[12].SetActive(true);
//             QuestLogNotesManager.a.labels[12].text = Sys_Text.stringTable[565];
//             QuestLogNotesManager.a.notes[13].SetActive(true);
//             QuestLogNotesManager.a.labels[13].text = Sys_Text.stringTable[566];
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
//             QuestLogNotesManager.a.labels[14].text = Sys_Text.stringTable[567];// Set:Engage reactor self-destruct.
//             QuestLogNotesManager.a.labels[15].text = Sys_Text.stringTable[568];// Set:Escape on escape pod.
//         }
//     }
//     if (BridgeSeparated) {
//         Const.a.questData.BridgeSeparated = !Const.a.questData.BridgeSeparated;
//         if (Const.a.questData.BridgeSeparated) {
//             QuestLogNotesManager.a.notes[16].SetActive(true);
//             QuestLogNotesManager.a.notes[17].SetActive(true);
//             QuestLogNotesManager.a.checkBoxes[16].isOn = true;
//             QuestLogNotesManager.a.labels[16].text = Sys_Text.stringTable[569]; // Set:Access the bridge.
//             QuestLogNotesManager.a.labels[17].text = Sys_Text.stringTable[570]; // Set:Destroy SHODAN.
//         }
//     }
//     if (IsolinearChipsetInstalled) Const.a.questData.IsolinearChipsetInstalled = !Const.a.questData.IsolinearChipsetInstalled;
// }
// 
// void TestBits(bool testIfTrue, UseData ud, TargetIO tio) {
//     if (RobotSpawnDeactivated && (!sEmpty(target) || !sEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.RobotSpawnDeactivated, testIfTrue, ud, tio, target, targetIfFalse);
//     if (IsotopeInstalled && (!sEmpty(target) || !sEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.IsotopeInstalled, testIfTrue, ud, tio, target, targetIfFalse);
//     if (ShieldActivated && (!sEmpty(target) || !sEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.ShieldActivated, testIfTrue, ud, tio, target, targetIfFalse);
//     if (LaserSafetyOverriden && (!sEmpty(target) || !sEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.LaserSafetyOverriden, testIfTrue, ud, tio, target, targetIfFalse);
//     if (LaserDestroyed && (!sEmpty(target) || !sEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.LaserDestroyed, testIfTrue, ud, tio, target, targetIfFalse);
//     if (BetaGroveCyberUnlocked && (!sEmpty(target) || !sEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.BetaGroveCyberUnlocked, testIfTrue, ud, tio, target, targetIfFalse);
//     if (GroveAlphaJettisonEnabled && (!sEmpty(target) || !sEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.GroveAlphaJettisonEnabled, testIfTrue, ud, tio, target, targetIfFalse);
//     if (GroveBetaJettisonEnabled && (!sEmpty(target) || !sEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.GroveBetaJettisonEnabled, testIfTrue, ud, tio, target, targetIfFalse);
//     if (GroveDeltaJettisonEnabled && (!sEmpty(target) || !sEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.GroveDeltaJettisonEnabled, testIfTrue, ud, tio, target, targetIfFalse);
//     if (MasterJettisonBroken && (!sEmpty(target) || !sEmpty(targetIfFalse)))TargetOnGatePassed(Const.a.questData.MasterJettisonBroken, testIfTrue, ud, tio, target, targetIfFalse);
//     if (Relay428Fixed && (!sEmpty(target) || !sEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.Relay428Fixed, testIfTrue, ud, tio, target, targetIfFalse);
//     if (MasterJettisonEnabled && (!sEmpty(target) || !sEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.MasterJettisonEnabled, testIfTrue, ud, tio, target, targetIfFalse);
//     if (BetaGroveJettisoned && (!sEmpty(target) || !sEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.BetaGroveJettisoned, testIfTrue, ud, tio, target, targetIfFalse);
//     if (AntennaNorthDestroyed && (!sEmpty(target) || !sEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.AntennaNorthDestroyed, testIfTrue, ud, tio, target, targetIfFalse);
//     if (AntennaSouthDestroyed && (!sEmpty(target) || !sEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.AntennaSouthDestroyed, testIfTrue, ud, tio, target, targetIfFalse);
//     if (AntennaEastDestroyed && (!sEmpty(target) || !sEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.AntennaEastDestroyed, testIfTrue, ud, tio, target, targetIfFalse);
//     if (AntennaWestDestroyed && (!sEmpty(target) || !sEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.AntennaWestDestroyed, testIfTrue, ud, tio, target, targetIfFalse);
//     if (SelfDestructActivated && (!sEmpty(target) || !sEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.SelfDestructActivated, testIfTrue, ud, tio, target, targetIfFalse);
//     if (BridgeSeparated && (!sEmpty(target) || !sEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.BridgeSeparated, testIfTrue, ud, tio, target, targetIfFalse);
//     if (IsolinearChipsetInstalled && (!sEmpty(target) || !sEmpty(targetIfFalse))) TargetOnGatePassed(Const.a.questData.IsolinearChipsetInstalled, testIfTrue, ud, tio, target, targetIfFalse);
// }
//================================================================================
// Doors
enum { DOOR_CLIP_IDLE_CLOSED = 0, DOOR_CLIP_OPENING = 1, DOOR_CLIP_IDLE_OPEN = 2, DOOR_CLIP_CLOSING = 3 };
void ChangeAnim(Entity* e, u8 clip);
static AnimationClip DoorGetClip(const Entity* e, u8 clip) { return modelAnimationClips[e->animationNum][clip]; }
static float DoorClamp01(float v) { if (v < 0.0f) return 0.0f; if (v > 1.0f) return 1.0f; return v; }
static bool DoorInventoryHasAccessCard(AccessCardType card) { return card == AccessCardType_None || (World.invP1.accessCardOwned & (1u << card)); }
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

static void DoorSetClipFrame(u16 self, u8 clip, u16 frame) { ChangeAnim(&World.instances[self],clip); (void)frame; }
static void DoorSyncLayer(u16 self) {
    Entity* e = &World.instances[self];
    if (!e->changeLayerOnOpenClose) return;
    e->layer = DoorIsAjar(e) ? L_InterDebris : L_Door;
}

static void DoorOpen(u16 self) {
    DualLog("opening door %u\n",self);
    Entity* e = &World.instances[self];
    DoorSetClipFrame(self,DOOR_CLIP_OPENING,DoorGetClip(e,DOOR_CLIP_OPENING).frameStart);
    e->doorOpen = e->doorState = DoorState_Opening;
    e->waitBeforeClose = World.pauseRelativeTime + e->delay;
    DoorSyncLayer(self);
    if (e->SFXIndex > 0 && e->SFXIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXIndex],1.0f,World.position[self],true);
}

static void DoorClose(u16 self) {
    DualLog("closing door %u\n",self);
    Entity* e = &World.instances[self];
    DoorSetClipFrame(self,DOOR_CLIP_CLOSING,DoorGetClip(e,DOOR_CLIP_CLOSING).frameStart);
    e->doorOpen = e->doorState = DoorState_Closing;
    DoorSyncLayer(self);
    if (e->SFXIndex > 0 && e->SFXIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXIndex],1.0f,World.position[self],true);
}

void DoorForceOpen(u16 self) { World.instances[self].requiredAccessCard = AccessCardType_None; EntitySetLocked(&World.instances[self],false); DoorOpen(self); }
void DoorForceClose(u16 self) { if (World.instances[self].doorOpen == DoorState_Closed) {return;} DoorClose(self); }
void DoorActuate(u16 self) {
    Entity* e = &World.instances[self];
    if (e->doorOpen == DoorState_Open) { DoorClose(self); return; }
    if (e->doorOpen == DoorState_Closed) { DoorOpen(self); return; }
    
    if (e->doorOpen == DoorState_Opening) {
        float t = DoorGetProgress(e,DOOR_CLIP_OPENING);
        AnimationClip c = DoorGetClip(e,DOOR_CLIP_CLOSING);
        DoorSetClipFrame(self,DOOR_CLIP_CLOSING,DoorFrameFromProgress(c,1.0f - t));
        e->doorOpen = e->doorState = DoorState_Closing;
        DoorSyncLayer(self);
        if (e->SFXIndex >= 0 && e->SFXIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXIndex],1.0f,World.position[self],true);
        return;
    }
    
    if (e->doorOpen == DoorState_Closing) {
        float t = DoorGetProgress(e,DOOR_CLIP_CLOSING);
        AnimationClip c = DoorGetClip(e,DOOR_CLIP_OPENING);
        DoorSetClipFrame(self,DOOR_CLIP_OPENING,DoorFrameFromProgress(c,1.0f - t));
        e->doorOpen = e->doorState = DoorState_Opening;
        e->waitBeforeClose = World.pauseRelativeTime + e->delay;
        DoorSyncLayer(self);
        if (e->SFXIndex >= 0 && e->SFXIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXIndex],1.0f,World.position[self],true);
    }
}

void DoorInitAfterLoad(u16 self) {
    Entity* e = &World.instances[self];
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
    Entity* e = &World.instances[self];
    if (activator == NULLENT) return;
    if (GetCurrentLevelSecurity() > e->securityThreshold) { UIBlockedBySecurity(World.position[self]); return; }
    
    if (Cheats.superoverride || World.diffMis <= 0) { EntitySetLocked(e,false); e->requiredAccessCard = AccessCardType_None; }
    if (World.diffMis <= 1) { e->requiredAccessCard = AccessCardType_None; }
    if (e->useFinished >= World.pauseRelativeTime) return;
    
    e->useFinished = World.pauseRelativeTime + e->useTimeDelay;
    if (e->requiredAccessCard != AccessCardType_None) {
        if (!DoorInventoryHasAccessCard(e->requiredAccessCard)) {
            CenterStatusPrint("%s",Sys_Text.stringTable[2]); // TODO Access-card-specific status text.
            if (e->SFXLockedIndex >= 0 && e->SFXLockedIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXLockedIndex],0.7f,World.position[self],true);
            return;
        } else e->requiredAccessCard = AccessCardType_None; // TODO Access-card granted status text.
    }
    
    if ((e->entflags & EF_LOCKED) != 0) {        
        CenterStatusPrint("%s",Sys_Text.stringTable[e->lockedMessageLingdex]);
        if (e->SFXLockedIndex >= 0 && e->SFXLockedIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXLockedIndex],0.55f,World.position[self],true);
        return;
    }

    if ((e->onlyTargetOnce && !e->targetAlreadyDone) || !e->onlyTargetOnce) { e->targetAlreadyDone = true; UseTargets(activator,e->target); }
    if (e->ajar) e->ajar = false;
    DoorActuate(self);
}

void DoorTargetted(u16 self, u16 activator) { if ((World.instances[self].entflags & EF_LOCKED) != 0) EntitySetLocked(&World.instances[self],false); if (!World.instances[self].targettingOnlyUnlocks) DoorUse(self,activator); }
void DoorUpdate(u16 self) {
    Entity* e = &World.instances[self];
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
    if (World.pauseRelativeTime > e->waitBeforeClose && e->doorOpen == DoorState_Open && !e->stayOpen && !e->startOpen) DoorClose(self);
}
//================================================================================
// Misc
u16 SpawnDynamicObject(int val, bool cheat) {
    if (!IdxInBounds(val)) { DualLogError("Const index out of bounds: %u", val); return NULLENT; }
    if (cheat) DualLog("Cheat spawn constIndex %u, level: %u, from cheat: %u, name: ", val, World.curLev, cheat);
    if (IdxIsGeometry(val) && !Cheats.editMode) { CenterStatusPrint("Indices 0 through 306 (level geometry chunks) not possible when not on edit mode!"); return NULLENT; }
    u16 entityIndexInInstanceTable = NULLENT;
    return entityIndexInInstanceTable;
}

void DeactivateVMail(void) { } // TODO
// Frob/Use
void SearchObject(int searchable, bool first) {
    if (first) {
        // TODO highlight Item tab in mfd
        firstTimeSearch = false;
    }
    if (World.instances[searchable].searchableInUse) {
        for (int i=0;i<4;i++) {
            if (World.instances[searchable].contents[i] >= 0) break;
        }
    } else play_wav(sounds[91],0.75f,(V3){},false);
}

void UseEntity(u16 p, u16 i) {
    InventorySystem* inv = Inv(p);
    Entity* ent = &World.instances[i];
    if (IdxIsSearchable(ent->index)) { inv->currentSearchItem = i; SearchObject(i,firstTimeSearch); DualLog("Search\n"); }
    else if (IdxIsDoor(ent->index)) DoorUse(i,PLAYER1);
    else if (IdxIsNPC(ent->index)) DualLog("Can't use NPC\n");
    else if (IdxIsButtonSwitch(ent->index)) ButtonSwitchUse(i,PLAYER1);
    else if (IdxIsGeometry(ent->index)) DualLog("Can't use modular geometry\n");
    else if (IdxIsUsableObject(ent->index)) {
        inv->holdingObject = true;
        inv->heldObjectIndex = ent->index;
        inv->heldObjectCustomIndex = ent->usableCustomIndex;
        inv->heldObjectAmmo = ent->ammo;
        inv->heldObjectAmmo2 = ent->ammo2;
        inv->heldObjectLoadedAlternate = ent->heldObjectLoadedAlternate;
        if (Sys_Settings.QuickItemPickup) { AddItemToInventory(p,ent->index,ent->usableCustomIndex); ResetHeldItem(p); }
		else { CenterStatusPrint("%s%s",Sys_Text.stringTable[inv->heldObjectIndex - 307 + 326],Sys_Text.stringTable[319]); /* picked up.*/ ForceInventoryMode(); } // Inventory mode is turned on when picking something up

		DeleteInstance(i);
    } else CenterStatusPrint("%s%s",Sys_Text.stringTable[29],"name");
}

#define FROB_DISTANCE 4.9f
static void Frob(V3 pos, V3 forward, V3 right) {
    if (World.curLev == LEVEL_CYBERSPACE) return;
    if (vmailActive) { DeactivateVMail(); vmailActive = false; return; }
    if (World.uiIsBlocking) return;
    
    InventorySystem* inv = Inv(PLAYER1);
    if (inv->holdingObject) { DropHeldItem(PLAYER1); return; }

    V3 dir = ScreenPointToRay(forward,right);
    World.debugLine_start = pos;
    World.debugLine_end = (V3){dir.x * FROB_DISTANCE + pos.x,dir.y * FROB_DISTANCE + pos.y,dir.z * FROB_DISTANCE + pos.z};
    RaycastHit tempHit = Raycast(pos,dir,FROB_DISTANCE,LMASK_PLAYER_FROB);
    World.debugLineFinished = World.pauseRelativeTime + 3.0;
    if (!tempHit.hit) { CenterStatusPrint("%s",Sys_Text.stringTable[30]); return; }
    World.debugLine_end = tempHit.point;
    DualLog("Raycast hit!  Hit object %u of entity type %u at hit point %f %f %f\n",tempHit.hitInstanceIndex,World.instances[tempHit.hitInstanceIndex].index,tempHit.point.x,tempHit.point.y,tempHit.point.z);
    UseEntity(PLAYER1,tempHit.hitInstanceIndex);
}

bool FrobWithHeldObject(void) {
    return false;
    return true;
}
//================================================================================
// Update
void ModUpdate(void) {
    if (World.paused || World.menuActive) return;
    
    WeaponsUpdate();
    PatchUpdate(PLAYER1);
    HardwareUpdate(PLAYER1);
    if (Use()) Frob(World.position[PLAYER1],World.instances[PLAYER1].forward,World.instances[PLAYER1].right);
    if (World.pauseRelativeTime < World.debugLineFinished && (World.debugLineVertCount + 6) < (MAX_WIRELINE_VRTS * 3)) AddWireLine(World.debugLine_start,World.debugLine_end,(Color){0.3f,0.1f,0.6f,0.5f});
    for (u16 i = INSTS_1ST_IDX; i < World.instCount; ++i) {
        Entity* e = &World.instances[i];
        TextureSequenceUpdate(i);
        u16 constdex = e->index;
        if (constdex == 718) ExplosionLifeUpdate(i);
        if (IdxIsButtonSwitch(constdex)) ButtonSwitchUpdate(i);
        if (IdxIsDoor(constdex)) DoorUpdate(i);
        if (constdex == 701) LogicTimerUpdate(i);
        if (e->doSelfAfterList || e->despawnInstead || e->destroyAfterListInsteadOfDeactivate) DelayedSpawnUpdate(i);
        if (e->itemLifeTime > 0.0f) SearchFXResetUpdate(i);
        if (World.curLev == LEVEL_CYBERSPACE && e->cyberTimer > 0.0f) CyberTimerUpdate(i);
        if (constdex == 515) ForceBridgeUpdate(i);
        if (constdex == 517) FuncWallUpdate(i);
        if (constdex == 21 || constdex == 22) CyberWallUpdate(i);
        if (constdex == 736) TargetIDUpdate(i);
//         if (constdex == 596) { GravityLiftOnTriggerStay(i,PLAYER1); } // TODO: Must hook into trigger system
    }
}
// Entity Init
void SearchableInit(u16 i) {
    int numRandomGeneratedItems = 0;
    if (World.instances[i].generateContents) {
        for(int j=0;j<4;j++) {
            if (World.instances[i].randomItemDropChance[j] <= 0.0f) continue;
            u8 tempInt = random_range_u8(0,100);
            if (((float)tempInt / 100.0f) <= World.instances[i].randomItemDropChance[j]) {
                World.instances[i].contents[numRandomGeneratedItems] = World.instances[i].randomItem[j];
                numRandomGeneratedItems++;
                if (numRandomGeneratedItems > World.instances[i].maxRandomItems) break;
            }
        }
    }
}

u8 GetCurrentLevelSecurity(void) { return (World.diffMis < 1 || Cheats.superoverride) ? 0u : World.levelSecurity[World.curLev]; }
u16 GetImpactType(u16 instanceIdx) {
    switch (World.instances[instanceIdx].bloodType) {
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
    Entity* e = &World.instances[i];
    if (World.diffPuz == 3 && e->index == 361 && random_range(0.0f,1.0f) < 0.33f) DeleteInstance(i); // 33% chance of not spawning logic probes on Puzzle difficulty of 3
    if (World.diffMis <= 1 && IdxIsAccessCard(e->index)) DeleteInstance(i); // Remove access cards on Mission difficulty 1 or 0
    if (World.diffMis == 0 && e->index == 313) DeleteInstance(i); // Remove audiologs on Mission difficulty 0
}

void MFDInit(SystemUI* ui) {
    ui->lastMultiMediaTabOpened = MULTI_MEDIA_TAB_EMAIL_TABLE;
    ui->logFinished = World.pauseRelativeTime;
    ui->tickFinished = ui->centerTabsTickFinished = World.current_time + 0.1 + (double)random_range(0.0f,1.0f);
    ui->blinkFinished = 1.0 + World.pauseRelativeTime;
    ui->beepFinished = 3.0 + World.pauseRelativeTime;
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
    inv->justFired = (World.pauseRelativeTime - 31.0); // Set >30s before pauseRelativeTime to not immediately play action music.
    inv->energyDrainTickFinished = World.pauseRelativeTime + 0.1 + (double)random_range(0.5f, 1.0f);
    inv->energy = 54.0f;
    inv->maxEnergy = 255.0f;
    inv->resetAfterDeathTime = 0.5;
    inv->painSoundFinished = World.pauseRelativeTime;
    inv->radSoundFinished = World.pauseRelativeTime;
    inv->radFXFinished = World.pauseRelativeTime;
}

void PlayerInit(u16 i) {
    World.instances[i].index = 767;
    World.layer[i] = L_Player;
    World.position[i] = (V3){10.52f,-43.792f + 0.84f,20.2908f}; // Start Actual: Puts player on Medical Level in actual game start position.  Added 0.84f
    World.scale[i] = (V3){1.0f,1.0f,1.0f};
    World.rotation[i] = (Quaternion){0.0f,0.7071f,0.0f,0.7071f}; // 90deg rotation CW about Y axis as viewed from the top looking down onto player
    World.instances[i].entflags = EF_ACTIVE|EF_RIGIDBODY;
    World.collider[i] = COLTYPE_CAP;
    World.colliderCenter[i].y = -0.84f;
    World.colliderSize[i] = (V3){0.48f,2.0f,1.0f}; // Radius, Overall height including end radii (Unity convention, blech), Direction, 1.0 == Y-Axis
    World.mass[i] = 1.0f;
    World.velocity[i] = (V3){0.0f,0.0f,0.0f};
    World.gravity[i] = 1.0f;
    World.dynamicFriction[i] = 0.6f; World.staticFriction[i] = 0.8f;
    World.instances[i].health = 200.0f;
    World.instances[i].noiseFinished = World.pauseRelativeTime;
    if (i == PLAYER1) InventoryInit(&World.invP1);
    else if (i == PLAYER2) InventoryInit(&World.invP2);
}

#include "credits.h"
const char** GetCreditsText(void) { return creditPages; }
void ModInitAfterLoad(void) {
    for (int i=PLAYER1;i<World.instCount;++i) {
        u16 constIndex = World.instances[i].index;
        if (i == PLAYER1 || i == PLAYER2 || IdxIsDynamicObject(constIndex) || (IdxIsNPC(constIndex) && constIndex < 443/*not cyber*/)) World.gravity[i] = 1.0f;
        else World.gravity[i] = 0.0f;
        if (IdxIsGeometry(constIndex)) World.layer[i] = L_Geometry;
        else if (IdxIsDoor(constIndex)) World.layer[i] = L_Door;
        else if (IdxIsUsableObject(constIndex)) UsableInit(i);
        else if (IdxIsDoor(World.instances[i].index)) DoorInitAfterLoad(i);
        else if (IdxIsNPC(constIndex)) { World.layer[i] = L_NPC; /* TODO AIInit funcion */ }
        else if (IdxIsSearchable(constIndex)) SearchableInit(i);
        else if (constIndex == 515) func_forcebridge(i); // func_forcebridge
        else if (constIndex == 517) FuncWallInitAfterLoad(i);
        else if (constIndex == 596) GravityLiftInitAfterLoad(i);
        else if (constIndex == 701) LogicTimerInitBeforeLoad(i);
        else if (constIndex == 556) TeleportTouchInitAfterLoad(i); // prop_cyberport
        else if (constIndex == 555) { } // prop_cyber_switch CyberSwitchInitAfterLoad(i);
        else if (constIndex == 21 || constIndex == 22) CyberWallInitAfterLoad(i); // chunk_cyberpanel or chunk_cyberpanel_slice45
        else if (constIndex == 736) TargetIDInitAfterLoad(i);
        else if (IdxIsButtonSwitch(World.instances[i].index)) ButtonSwitchInitAfterLoad(i);
        else if (constIndex >= 448 && constIndex <= 457) CyberItemInitBeforeLoad(i);
        else if (constIndex == 480) CyberMineInitBeforeLoad(i);
        if (!sEmpty(World.instances[i].targetname) && (World.instances[i].ioflags & TARG_IOFLAGS_DISABLE_ON_AWAKE)) flag_set(&World.instances[i].entflags,EF_ACTIVE,false);
    }
}

u16 GetCrosshairTexture(void) {
    switch(World.invP1.weaponIndex) {
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
    if (World.paused || World.menuActive) return 1261; // Red standard cursor
    if (!World.invP1.holdingObject) return GetCrosshairTexture();
    switch(World.invP1.heldObjectIndex) {
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
