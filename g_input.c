#include "mod.h"
// From Engine
GlobalContext* Eng_Global = 0; CheatsSystem* Eng_Cheats;
MOD_TO_ENGINE void ModInit(GlobalContext* globals, CheatsSystem* cheats) { Eng_Global = globals; Eng_Cheats = cheats; }
bool (*GetKey)(int settingIndex);
bool (*GetKeyPressed)(int settingIndex);
MOD_TO_ENGINE bool Forward(void) {     return GetKey(0); }
MOD_TO_ENGINE bool StrafeLeft(void) {  return GetKey(1); }
MOD_TO_ENGINE bool Backpedal(void) {   return GetKey(2); }
MOD_TO_ENGINE bool StrafeRight(void) { return GetKey(3); }
MOD_TO_ENGINE bool Jump(void) {        return GetKey(4); }
MOD_TO_ENGINE bool JumpDown(void) {    return GetKeyPressed(4); }
MOD_TO_ENGINE bool Crouch(void) {      return GetKeyPressed(5); }
MOD_TO_ENGINE bool Prone(void) {       return GetKeyPressed(6); }
MOD_TO_ENGINE bool LeanLeft(void) {    return GetKey(7); }
MOD_TO_ENGINE bool LeanRight(void) {   return GetKey(8); }
MOD_TO_ENGINE bool Sprint(void) {      return GetKey(9); } // Toggle Sprint unused
MOD_TO_ENGINE bool TurnLeft(void) {    return GetKey(10); }
MOD_TO_ENGINE bool TurnRight(void) {   return GetKey(11); }
MOD_TO_ENGINE bool LookUp(void) {      return GetKey(12); }
MOD_TO_ENGINE bool LookDown(void) {    return GetKey(13); }
MOD_TO_ENGINE bool RecentLog(void) {   return GetKeyPressed(14); }
MOD_TO_ENGINE bool Biomonitor(void) {  return GetKeyPressed(15); }
MOD_TO_ENGINE bool Sensaround(void) {  return GetKeyPressed(16); }
MOD_TO_ENGINE bool Lantern(void) {     return GetKeyPressed(17); }
MOD_TO_ENGINE bool Shield(void) {      return GetKeyPressed(18); }
MOD_TO_ENGINE bool Infrared(void) {    return GetKeyPressed(19); }
MOD_TO_ENGINE bool Email(void) {       return GetKeyPressed(20); }
MOD_TO_ENGINE bool Booster(void) {     return GetKeyPressed(21); }
MOD_TO_ENGINE bool Jumpjets(void) {    return GetKeyPressed(22); }
MOD_TO_ENGINE bool Attack(void) {      return GetKeyPressed(23); }
MOD_TO_ENGINE bool Use(void) {         return GetKeyPressed(24); }
MOD_TO_ENGINE bool Menu(void) {        return GetKeyPressed(25); }
MOD_TO_ENGINE bool ToggleMode(void) {  return GetKeyPressed(26); }
MOD_TO_ENGINE bool Reload(void) {      return GetKeyPressed(27); }
MOD_TO_ENGINE bool WeaponCycUp(void) { return GetKeyPressed(28); }
MOD_TO_ENGINE bool WeaponCycDown(void){return GetKeyPressed(29); }
MOD_TO_ENGINE bool Grenade(void) {     return GetKeyPressed(30); }
MOD_TO_ENGINE bool GrenadeCycUp(void) {return GetKeyPressed(31); }
MOD_TO_ENGINE bool GrenadeCycDown(void){return GetKeyPressed(32); }
MOD_TO_ENGINE bool ChangeAmmoType(void){return GetKeyPressed(33); }
MOD_TO_ENGINE bool Patch(void) {       return GetKeyPressed(34); }
MOD_TO_ENGINE bool PatchCycUp(void) {  return GetKeyPressed(35); }
MOD_TO_ENGINE bool PatchCycDown(void) {return GetKeyPressed(36); }
MOD_TO_ENGINE bool Map(void) {         return GetKeyPressed(37); }
MOD_TO_ENGINE bool SwimUp(void) {      return GetKey(38); }
MOD_TO_ENGINE bool SwimDn(void) {      return GetKey(39); }
MOD_TO_ENGINE bool Console(void) {     return GetKeyPressed(-1); }
MOD_TO_ENGINE bool TakeScreenshot(void) {  return GetKeyPressed(41); }
