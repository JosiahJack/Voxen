#include "mod.h"
void WeaponsUpdate(void);

static void UpdateConvertedEntity(uint16_t i) {
    Entity* e = &Eng_Global->instances[i];
    if (EntityDefIs(i,"ef_fragexplosion")) ExplosionLifeUpdate(i);
    if (ConstIndexIsButtonSwitch(e->index)) ButtonSwitchUpdate(i);
    if (ConstIndexIsDoor(e->index)) DoorUpdate(i);
    if (EntityDefIs(i,"logic_timer")) LogicTimerUpdate(i);
    if (e->doSelfAfterList || e->despawnInstead || e->destroyAfterListInsteadOfDeactivate) DelayedSpawnUpdate(i);
    if (e->itemLifeTime > 0.0f) SearchFXResetUpdate(i);
    if (Eng_Global->currentLevel == LEVEL_CYBERSPACE && e->cyberTimer > 0.0f) CyberTimerUpdate(i);
    if (EntityDefIs(i,"func_forcebridge")) ForceBridgeUpdate(i);
    if (EntityDefIs(i,"func_wall")) FuncWallUpdate(i);
}

void DeactivateVMail(void) { }

void SearchObject(int searchable) {
    if (Eng_Global->instances[searchable].searchableInUse) {
        for (int i=0;i<4;i++) {
            if (Eng_Global->instances[searchable].contents[i] >= 0) break;
        }
    } else play_wav(sounds[91],0.75f,(Vector3){},false);
}

void UseEntity(uint16_t i) {
    Entity* ent = &Eng_Global->instances[i];
    if (ConstIndexIsSearchable(ent->index)) {
        Eng_Global->inventoryPlayer1.currentSearchItem = i;
        SearchObject(i);
        DualLog("Search\n");
    }
    else if (ConstIndexIsDoor(ent->index)) DoorUse(i,PLAYER1,ent->argvalue);
    else if (ConstIndexIsNPC(ent->index)) DualLog("Can't use NPC\n");
    else if (ConstIndexIsButtonSwitch(ent->index)) ButtonSwitchUse(i,PLAYER1,ent->argvalue);
    else if (ConstIndexIsGeometry(ent->index)) DualLog("Can't use modular geometry\n");
    else if (ConstIndexIsDynamicObject(ent->index)) DualLog("Using a dynamic object\n");
    else CenterStatusPrint("%s%s",Eng_Text->stringTable[29],"name");
}

#define FROB_DISTANCE 4.9f
static inline __attribute__((always_inline)) void Frob(Vector3 pos, Vector3 forward, Vector3 right) {
    if (vmailActive) { DeactivateVMail(); vmailActive = false; return; }
    if (Eng_Global->uiIsBlocking) return;
    float offsetX = Eng_Global->cursorPosition_x - (Eng_Settings->ScreenWidth * 0.5f);
    float offsetY = Eng_Global->cursorPosition_y - (Eng_Settings->ScreenHeight * 0.5f);
    float ndcX = offsetX / (Eng_Settings->ScreenWidth * 0.5f);
    float ndcY = -offsetY / (Eng_Settings->ScreenHeight * 0.5f);
    float tanFov = vtan((float)Eng_Settings->FOV * 0.5f * PI / 180.0f);
    Vector3 view = (Vector3){ndcX * tanFov * Eng_Global->aspect3D,ndcY * tanFov,-1.0f};
    view = normalize_vector3(view);
    Vector3 flipForward = (Vector3){-forward.x,-forward.y,-forward.z};
    Vector3 up = normalize_vector3(cross_vector3(right,flipForward));
    Vector3 dir = (Vector3){view.x * right.x + view.y * up.x + view.z * flipForward.x,view.x * right.y + view.y * up.y + view.z * flipForward.y,view.x * right.z + view.y * up.z + view.z * flipForward.z};
    Eng_Global->debugLine_start = pos;
    Eng_Global->debugLine_end = (Vector3){dir.x * FROB_DISTANCE + pos.x,dir.y * FROB_DISTANCE + pos.y,dir.z * FROB_DISTANCE + pos.z};
    RaycastHit tempHit = Raycast(pos,dir,FROB_DISTANCE,LAYER_MASK_PLAYER_FROB);
    Eng_Global->debugLineFinished = Eng_Global->pauseRelativeTime + 3.0;
    if (!tempHit.hit) { CenterStatusPrint("%s",Eng_Text->stringTable[30]); return; }
    Eng_Global->debugLine_end = tempHit.point;
    DualLog("Raycast hit!  Hit object %u named of entity type %s(%u) at hit point %f %f %f\n",tempHit.hitInstanceIndex,Eng_Global->entities[Eng_Global->instances[tempHit.hitInstanceIndex].index].path,Eng_Global->instances[tempHit.hitInstanceIndex].index,tempHit.point.x,tempHit.point.y,tempHit.point.z);
    UseEntity(tempHit.hitInstanceIndex);
}

bool FrobWithHeldObject(void) {
    return false;
    return true;
}

MOD_TO_ENGINE void ModUpdate(void) {
    WeaponsUpdate();
    if (Use()) {
        if (Eng_Global->uiIsBlocking) {
        } else if (Eng_Global->currentLevel != LEVEL_CYBERSPACE) {
            if (Eng_Global->inventoryPlayer1.dropFinished < Eng_Global->pauseRelativeTime) {
                if (Eng_Global->inventoryPlayer1.holdingObject) {
                } else Frob(Eng_Global->instances[PLAYER1].position,Eng_Global->instances[PLAYER1].forward,Eng_Global->instances[PLAYER1].right);
            }
        }
    }
    if (Eng_Global->pauseRelativeTime < Eng_Global->debugLineFinished && (Eng_Global->debugLineVertCount + 6) < (MAX_DEBUG_LINE_VERTS * 3)) AddDebugLine(Eng_Global->debugLine_start,Eng_Global->debugLine_end);
    for (uint16_t i = START_INDEX_LEVEL_INSTANCES; i < Eng_Global->loadedInstances; ++i) UpdateConvertedEntity(i);
}
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
    if (TakeScreenshot() && Eng_Global->current_time > Eng_Global->screenshotTimeout) Screenshot();
    if (Menu() && !Eng_Global->menuActive) { Eng_Global->gamePaused = !Eng_Global->gamePaused; return; }
    if (Menu() && Eng_Global->menuActive) { MenuGoBack(); return; }
    if (Eng_Global->gamePaused || Eng_Global->menuActive || Eng_Cheats->consoleActive) return;
    if (ToggleMode()) {
        IgnoreNextMouseDelta();
        Eng_Global->inventoryMode = !Eng_Global->inventoryMode;
        Eng_Global->cursorPosition_x = Eng_Settings->ScreenWidth / 2;
        Eng_Global->cursorPosition_y = Eng_Settings->ScreenHeight / 2;
    }
    if (Lantern()) Eng_Global->inventoryPlayer1.hardwareIsActive ^= HW_LAN;
    if (Infrared()) Eng_Global->inventoryPlayer1.hardwareIsActive ^= HW_INF;
    ApplyPlayerMovements();
}

MOD_TO_ENGINE uint16_t SpawnDynamicObject(int val, bool cheat) {
    if (!ConstIndexInBounds(val)) { DualLogError("Const index out of bounds: %u", val); return NULLENT; }
    if (cheat) DualLog("Cheat spawn constIndex %u, level: %u, from cheat: %u, name: ", val, Eng_Global->currentLevel, cheat);
    if (ConstIndexIsGeometry(val) && !Eng_Cheats->editMode) { CenterStatusPrint("Indices 0 through 306 (level geometry chunks) not possible when not on edit mode!"); return NULLENT; }
    uint16_t entityIndexInInstanceTable = NULLENT;
    return entityIndexInInstanceTable;
}
