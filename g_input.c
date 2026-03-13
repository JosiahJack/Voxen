#include "mod.h"
// From Engine
GlobalContext* Eng_Global = 0; CheatsSystem* Eng_Cheats;
MOD_TO_ENGINE void ModInit(GlobalContext* globals, CheatsSystem* cheats) { Eng_Global = globals; Eng_Cheats = cheats; }
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
