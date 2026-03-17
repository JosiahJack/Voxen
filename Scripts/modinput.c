#include "mod.h"
void WeaponsUpdate(void);

#define FROB_DISTANCE 4.9f
static inline __attribute__((always_inline)) void Frob(Vector3 pos, Vector3 forward, Vector3 right) {
    float offsetX = Eng_Global->cursorPosition_x - (Eng_Settings->ScreenWidth * 0.5f);
    float offsetY = Eng_Global->cursorPosition_y - (Eng_Settings->ScreenHeight * 0.5f);
    float ndcX = offsetX / (Eng_Settings->ScreenWidth * 0.5f);
    float ndcY = -offsetY / (Eng_Settings->ScreenHeight * 0.5f);  // flip Y
    float tanFov = vtan((float)Eng_Settings->FOV * 0.5f * PI / 180.0f);
    Vector3 view = (Vector3){ ndcX * tanFov * Eng_Global->aspect3D, ndcY * tanFov, -1.0f };
    view = normalize_vector3(view);
    Vector3 flipForward = (Vector3){ -forward.x, -forward.y, -forward.z};
    Vector3 up = normalize_vector3( cross_vector3(right, flipForward) );
    Vector3 dir = (Vector3){ view.x * right.x + view.y * up.x + view.z * (flipForward.x), view.x * right.y + view.y * up.y + view.z * (flipForward.y), view.x * right.z + view.y * up.z + view.z * (flipForward.z) };
    Eng_Global->debugLine_start = pos;
    Eng_Global->debugLine_end   = (Vector3){ dir.x * FROB_DISTANCE + pos.x, dir.y * FROB_DISTANCE + pos.y, dir.z * FROB_DISTANCE + pos.z };
    RaycastHit tempHit = Raycast(pos, dir, FROB_DISTANCE, LAYER_MASK_PLAYER_FROB);
    if (tempHit.hit) {
        Eng_Global->debugLine_end = tempHit.point;
        DualLog("Raycast hit!  Hit object %u named of entity type %s(%u) at hit point %f %f %f\n",tempHit.hitInstanceIndex,Eng_Global->entities[Eng_Global->instances[tempHit.hitInstanceIndex].index].path,Eng_Global->instances[tempHit.hitInstanceIndex].index,tempHit.point.x,tempHit.point.y,tempHit.point.z);
    }
    
    Eng_Global->debugLineFinished = Eng_Global->pauseRelativeTime + 3.0;
}

MOD_TO_ENGINE void ModUpdate(void) {
    WeaponsUpdate();
    if (Use()) Frob(Eng_Global->instances[PLAYER1].position,Eng_Global->instances[PLAYER1].forward,Eng_Global->instances[PLAYER1].right);
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
