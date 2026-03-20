#include "mod.h"
void WeaponsUpdate(void);

void DeactivateVMail(void) { } // TODO

void SearchObject (int searchable){
//     bool useFX = true;
    if (Eng_Global->instances[searchable].searchableInUse) {
        for (int i=0;i<4;i++) {
            if (Eng_Global->instances[searchable].contents[i] >= 0) {
//                 MouseCursor.a.GetComponent<MouseCursor>().cursorImage = Const.a.useableItemsFrobIcons[contentsSearchable[i]];
//                 Eng_Global->inventoryPlayer1.heldObjectIndex = curSearchScript.contEng_Global->instances[i];
//                 heldObjectCustomIndex = curSearchScript.customIndex[i];
//                 curSearchScript.contEng_Global->instances[i] = -1;
//                 curSearchScript.customIndex[i] = -1;
//                 if (Eng_Global->inventoryPlayer1.heldObjectIndex != -1) Eng_Global->inventoryPlayer1.holdingObject = true;
//                 CenterStatusPrint("%s%s",Eng_Text->stringTable[Eng_Global->inventoryPlayer1.heldObjectIndex + 326],Eng_Text->stringTable[319]); // picked up
//                 Eng_UI->DisableSearchItemImage(i);
//                 useFX = false;
                break;
            }
        }
    } else play_wav(sounds[91],0.75f,(Vector3){},false); // searchsound

//     curSearchScript.searchableInUse = true;
//     int numberFoundContents = 0;
//     int[] resultContents = {-1,-1,-1,-1};  // create blanked container for search results
//     int[] resultCustomIndex = {-1,-1,-1,-1};  // create blanked container for search results custom indices
//     for (int i=3;i>=0;i--) { // Search through array to see if any items are in the container
//         resultContEng_Global->instances[i] = curSearchScript.contEng_Global->instances[i];
//         resultCustomIndex[i] = curSearchScript.customIndex[i];
//         // If something was found, add 1 to count.
//         if (resultContEng_Global->instances[i] > -1) numberFoundContents++;
//     }
// 
//     static const bool firstTimeSearch = true;
//     if (firstTimeSearch) {
//         firstTimeSearch = false;
//         Eng_UI->OpenTab(4,true,TabMSG.Search,-1,Handedness.LH);
//     }
//     
//     Eng_UI->SendSearchToDataTab(curSearchScript.objectName,numberFoundContents,resultContents,resultCustomIndex,currentSearchItem.Eng_Global->instances[i].position,curSearchScript, useFX);
//     ForceInventoryMode();
}

void UseEntity(uint16_t i) {
    Entity* ent = &Eng_Global->instances[i];
    
    if (ConstIndexIsSearchable(ent->index)) {
        Eng_Global->inventoryPlayer1.currentSearchItem = i;
        SearchObject(i);
        DualLog("Search\n");
    }
    else if (ConstIndexIsDoor(ent->index)) DualLog("Used door\n");
    else if (ConstIndexIsNPC(ent->index)) DualLog("Can't use NPC\n");
    else if (ConstIndexIsButtonSwitch(ent->index)) DualLog("Uses button switch\n");
    else if (ConstIndexIsGeometry(ent->index)) DualLog("Can't use modular geometry\n");
    else if (ConstIndexIsDynamicObject(ent->index)) DualLog("Using a dynamic object\n");
    else CenterStatusPrint("%s%s",Eng_Text->stringTable[29],"name"); // "Can't use "
}

#define FROB_DISTANCE 4.9f
static inline __attribute__((always_inline)) void Frob(Vector3 pos, Vector3 forward, Vector3 right) {
    if (vmailActive) { DeactivateVMail(); vmailActive = false; return; }
    if (Eng_Global->uiIsBlocking) return;

    float offsetX = Eng_Global->cursorPosition_x - (Eng_Settings->ScreenWidth * 0.5f);
    float offsetY = Eng_Global->cursorPosition_y - (Eng_Settings->ScreenHeight * 0.5f);
    float ndcX = offsetX / (Eng_Settings->ScreenWidth * 0.5f);
    float ndcY = -offsetY / (Eng_Settings->ScreenHeight * 0.5f);  // flip Y
    float tanFov = vtan((float)Eng_Settings->FOV * 0.5f * PI / 180.0f);
    Vector3 view = (Vector3){ ndcX * tanFov * Eng_Global->aspect3D, ndcY * tanFov, -1.0f };
    view = normalize_vector3(view);
    Vector3 flipForward = (Vector3){-forward.x,-forward.y,-forward.z};
    Vector3 up = normalize_vector3(cross_vector3(right,flipForward));
    Vector3 dir = (Vector3){ view.x * right.x + view.y * up.x + view.z * (flipForward.x), view.x * right.y + view.y * up.y + view.z * (flipForward.y), view.x * right.z + view.y * up.z + view.z * (flipForward.z) };
    Eng_Global->debugLine_start = pos;
    Eng_Global->debugLine_end   = (Vector3){ dir.x * FROB_DISTANCE + pos.x, dir.y * FROB_DISTANCE + pos.y, dir.z * FROB_DISTANCE + pos.z };
    RaycastHit tempHit = Raycast(pos,dir,FROB_DISTANCE,LAYER_MASK_PLAYER_FROB);
    Eng_Global->debugLineFinished = Eng_Global->pauseRelativeTime + 3.0;
    if (!tempHit.hit) { CenterStatusPrint("%s",Eng_Text->stringTable[30]); return; } // You are too far away from that
    
    Eng_Global->debugLine_end = tempHit.point;
    DualLog("Raycast hit!  Hit object %u named of entity type %s(%u) at hit point %f %f %f\n",tempHit.hitInstanceIndex,Eng_Global->entities[Eng_Global->instances[tempHit.hitInstanceIndex].index].path,Eng_Global->instances[tempHit.hitInstanceIndex].index,tempHit.point.x,tempHit.point.y,tempHit.point.z);
    UseEntity(tempHit.hitInstanceIndex);
}

bool FrobWithHeldObject(void) {
//     bool frobUser = (Eng_Global->inventoryPlayer1.heldObjectIndex == 54 || Eng_Global->inventoryPlayer1.heldObjectIndex == 56
//                   || Eng_Global->inventoryPlayer1.heldObjectIndex == 57 || Eng_Global->inventoryPlayer1.heldObjectIndex == 61
//                   || Eng_Global->inventoryPlayer1.heldObjectIndex == 64 || Eng_Global->inventoryPlayer1.heldObjectIndex == 92
//                   || Eng_Global->inventoryPlayer1.heldObjectIndex == 93 || Eng_Global->inventoryPlayer1.heldObjectIndex == 94);
//     if (!frobUser) return false;
// 
//     cursorPoint = MouseCursor.a.GetCursorScreenPointForRay();
//     RaycastHit ray = Raycast(playerCamera.ScreenPointToRay(cursorPoint), out tempHit, Const.frobDistance)) {
//     if (!ray.hit) return false; // Can't use it on something, go ahead and drop it.
// 
//     Utils.PlayUIOneShotSavable(91); // searchsound
//     UseEntity(ray.hitInstanceIndex);
    return false;
    return true; // Item can get absorbed, not dropped.
}

MOD_TO_ENGINE void ModUpdate(void) {
    WeaponsUpdate();
    if (Use()) {
        if (Eng_Global->uiIsBlocking) {
//             if (Eng_Global->inventoryPlayer1.holdingObject && Eng_Global->currentLevel != LEVEL_CYBERSPACE) {
//                 AddItemToInventory(Eng_Global->inventoryPlayer1.heldObjectIndex,heldObjectCustomIndex);
//                 MouseCursor.a.liveGrenade = false;
// 				ResetHeldItem();
//             }
        } else if (Eng_Global->currentLevel != LEVEL_CYBERSPACE) {
            if (Eng_Global->inventoryPlayer1.dropFinished < Eng_Global->pauseRelativeTime) {
				if (Eng_Global->inventoryPlayer1.holdingObject) {
// 					if (!FrobWithHeldObject()) DropHeldItem();
				} else Frob(Eng_Global->instances[PLAYER1].position,Eng_Global->instances[PLAYER1].forward,Eng_Global->instances[PLAYER1].right);
			}
        }
    }
    if (Eng_Global->pauseRelativeTime < Eng_Global->debugLineFinished && (Eng_Global->debugLineVertCount + 6) < (MAX_DEBUG_LINE_VERTS * 3)) AddDebugLine(Eng_Global->debugLine_start,Eng_Global->debugLine_end);
    // Main entity loop
    for (uint16_t i=START_INDEX_LEVEL_INSTANCES;i<Eng_Global->loadedInstances;++i) {
        
    }
}
MOD_TO_ENGINE bool Forward(void) {     return Eng_Global->GetKey(0); }
MOD_TO_ENGINE bool StrafeLeft(void) {  return Eng_Global->GetKey(1); }
MOD_TO_ENGINE bool Backpedal(void) {   return Eng_Global->GetKey(2); }
MOD_TO_ENGINE bool StrafeRight(void) { return Eng_Global->GetKey(3); }
MOD_TO_ENGINE bool Jump(void) {        return Eng_Global->GetKey(4); }
MOD_TO_ENGINE bool JumpDown(void) {    return Eng_Global->GetKeyPressed(4); }
MOD_TO_ENGINE bool Crouch(void) {      return Eng_Global->GetKeyPressed(5); }
MOD_TO_ENGINE bool Prone(void) {       return Eng_Global->GetKeyPressed(6); }
MOD_TO_ENGINE bool LeanLeft(void) {    return Eng_Global->GetKey(7); }
MOD_TO_ENGINE bool LeanRight(void) {   return Eng_Global->GetKey(8); }
MOD_TO_ENGINE bool Sprint(void) {      return Eng_Global->GetKey(9); } // Toggle Sprint unused
MOD_TO_ENGINE bool TurnLeft(void) {    return Eng_Global->GetKey(10); }
MOD_TO_ENGINE bool TurnRight(void) {   return Eng_Global->GetKey(11); }
MOD_TO_ENGINE bool LookUp(void) {      return Eng_Global->GetKey(12); }
MOD_TO_ENGINE bool LookDown(void) {    return Eng_Global->GetKey(13); }
MOD_TO_ENGINE bool RecentLog(void) {   return Eng_Global->GetKeyPressed(14); }
MOD_TO_ENGINE bool Biomonitor(void) {  return Eng_Global->GetKeyPressed(15); }
MOD_TO_ENGINE bool Sensaround(void) {  return Eng_Global->GetKeyPressed(16); }
MOD_TO_ENGINE bool Lantern(void) {     return Eng_Global->GetKeyPressed(17); }
MOD_TO_ENGINE bool Shield(void) {      return Eng_Global->GetKeyPressed(18); }
MOD_TO_ENGINE bool Infrared(void) {    return Eng_Global->GetKeyPressed(19); }
MOD_TO_ENGINE bool Email(void) {       return Eng_Global->GetKeyPressed(20); }
MOD_TO_ENGINE bool Booster(void) {     return Eng_Global->GetKeyPressed(21); }
MOD_TO_ENGINE bool Jumpjets(void) {    return Eng_Global->GetKeyPressed(22); }
MOD_TO_ENGINE bool Attack(void) {      return Eng_Global->GetKeyPressed(23); }
MOD_TO_ENGINE bool Use(void) {         return Eng_Global->GetKeyPressed(24); }
MOD_TO_ENGINE bool Menu(void) {        return Eng_Global->GetKeyPressed(25); }
MOD_TO_ENGINE bool ToggleMode(void) {  return Eng_Global->GetKeyPressed(26); }
MOD_TO_ENGINE bool Reload(void) {      return Eng_Global->GetKeyPressed(27); }
MOD_TO_ENGINE bool WeaponCycUp(void) { return Eng_Global->GetKeyPressed(28); }
MOD_TO_ENGINE bool WeaponCycDown(void){return Eng_Global->GetKeyPressed(29); }
MOD_TO_ENGINE bool Grenade(void) {     return Eng_Global->GetKeyPressed(30); }
MOD_TO_ENGINE bool GrenadeCycUp(void) {return Eng_Global->GetKeyPressed(31); }
MOD_TO_ENGINE bool GrenadeCycDown(void){return Eng_Global->GetKeyPressed(32); }
MOD_TO_ENGINE bool ChangeAmmoType(void){return Eng_Global->GetKeyPressed(33); }
MOD_TO_ENGINE bool Patch(void) {       return Eng_Global->GetKeyPressed(34); }
MOD_TO_ENGINE bool PatchCycUp(void) {  return Eng_Global->GetKeyPressed(35); }
MOD_TO_ENGINE bool PatchCycDown(void) {return Eng_Global->GetKeyPressed(36); }
MOD_TO_ENGINE bool Map(void) {         return Eng_Global->GetKeyPressed(37); }
MOD_TO_ENGINE bool SwimUp(void) {      return Eng_Global->GetKey(38); }
MOD_TO_ENGINE bool SwimDn(void) {      return Eng_Global->GetKey(39); }
MOD_TO_ENGINE bool Console(void) {     return Eng_Global->GetKeyPressed(-1); }
MOD_TO_ENGINE bool TakeScreenshot(void) {  return Eng_Global->GetKeyPressed(41); }

MOD_TO_ENGINE void ProcessInput(void) {
    if (Console()) ToggleConsole();
    if (TakeScreenshot() && Eng_Global->current_time > Eng_Global->screenshotTimeout) Screenshot();
    if (Menu() && !Eng_Global->menuActive) { Eng_Global->gamePaused = !Eng_Global->gamePaused; return; }
    if (Menu() && Eng_Global->menuActive) { MenuGoBack(); return; }
    if (Eng_Global->gamePaused || Eng_Global->menuActive || Eng_Cheats->consoleActive) return; // =========== PAUSE BARRIER ==================

    if (ToggleMode()) {
        IgnoreNextMouseDelta();
        Eng_Global->inventoryMode = !Eng_Global->inventoryMode;
        Eng_Global->cursorPosition_x = Eng_Settings->ScreenWidth / 2;
        Eng_Global->cursorPosition_y = Eng_Settings->ScreenHeight / 2;
    }
    
    // Hardware hotkeys TODO
    if (Lantern()) Eng_Global->inventoryPlayer1.hardwareIsActive ^= HW_LAN;
    if (Infrared()) Eng_Global->inventoryPlayer1.hardwareIsActive ^= HW_INF;
//     if ((Eng_Global->inventoryPlayer1.hasHardware & HW_ERD) && GetInput.a.Email())      EReaderAction();
//     if ((Eng_Global->inventoryPlayer1.hasHardware & HW_SNS) && GetInput.a.Sensaround()) SensaroundAction();
//     if ((Eng_Global->inventoryPlayer1.hasHardware & HW_SHD) && GetInput.a.Shield())     ShieldAction();
//     if ((Eng_Global->inventoryPlayer1.hasHardware & HW_BIO) && GetInput.a.Biomonitor()) BioAction();
//     if ((Eng_Global->inventoryPlayer1.hasHardware & HW_LAN) && GetInput.a.Lantern())    LanternAction();
//     if ((Eng_Global->inventoryPlayer1.hasHardware & HW_BST) && GetInput.a.Booster())    BoosterAction();
//     if ((Eng_Global->inventoryPlayer1.hasHardware & HW_JET) && GetInput.a.Jumpjets())   JumpJetsAction();
//     if ((Eng_Global->inventoryPlayer1.hasHardware & HW_INF) && GetInput.a.Infrared())   InfraredAction();
    ApplyPlayerMovements();
}

MOD_TO_ENGINE uint16_t SpawnDynamicObject(int val, bool cheat) {
    if (!ConstIndexInBounds(val)) { DualLogError("Const index out of bounds: %u", val); return NULLENT; } // Checked in cmd_summon but used elsewhere so guard here too.
    
    if (cheat) DualLog("Cheat spawn constIndex %u, level: %u, from cheat: %u, name: ", val, Eng_Global->currentLevel, cheat);
//     Vector3 spawnPos = (Vector3){0.0,0.0,0.0};
//     if (cheat) spawnPos = (Vector3){Eng_Global->instances[PLAYER1].position.x,Eng_Global->instances[PLAYER1].position.y,Eng_Global->instances[PLAYER1].position.z};
    if (ConstIndexIsGeometry(val) && !Eng_Cheats->editMode) { CenterStatusPrint("Indices 0 through 306 (level geometry chunks) not possible when not on edit mode!"); return NULLENT; }
    
    uint16_t entityIndexInInstanceTable = NULLENT;//MonoBehaviour.Instantiate(Const.a.GetPrefab(val),spawnPos, Const.a.quaternionIdentity) as Entity;
    if (cheat && ConstIndexIsHardware(val)) { // Hardware
//         UseableObjectUse uo = go.GetComponent<UseableObjectUse>();
//         int dex14 = hardware14fromConstdex(uo.useableItemIndex);
//         if (inventoryPlayer1.hasHardware[dex14]) uo.customIndex = (inventoryPlayer1.hardwareVersion[dex14] + 1);
    }

    return entityIndexInInstanceTable;
}
