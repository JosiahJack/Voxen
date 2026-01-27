#define PATCH_BERSERK   1
#define PATCH_DETOX     2
#define PATCH_GENIUS    4
#define PATCH_MEDI      8
#define PATCH_REFLEX   16
#define PATCH_SIGHT    32
#define PATCH_STAMINUP 64
#define BERSERK_TIME  20.0f
#define DETOX_TIME    60.0f
#define GENIUS_TIME  180.0f
#define MEDI_TIME     35.0f
#define REFLEX_TIME  155.0f
#define SIGHT_TIME    40.0f
#define STAMINUP_TIME 60.0f
#define SIGHT_SIDE_EFFECT_TIME 17.0f
#define REFLEX_TIME_SCALE 0.25f
#define DEFAULT_TIME_SCALE 1.0f
#define BERSERK_DAMAGE_MULTIPLIER 4.0f // Quad Damage!
#define NITRO_MIN_TIME     1.0f
#define NITRO_MAX_TIME    60.0f
#define NITRO_DEFAULT_TIME 7.0f
#define EARTH_SHAKER_MIN_TIME      4.0f
#define EARTH_SHAKER_MAX_TIME     60.0f
#define EARTH_SHAKER_DEFAULT_TIME 10.0f
#define GLOBAL_SHAKE_DISTANCE 0.3f
#define GLOBAL_SHAKE_FORCE    1.0f

void ActivatePatch(int index) { // Expects the usableItems index
//     bool depleted = false;
    switch (index) {
        case 14:
            // Berserk Patch
    //         patchCounts[2]--;
    //         if (patchCounts[2] <= 0) { depleted = true; }
    //         if (!(patchActive & PATCH_BERSERK)) patchActive += PATCH_BERSERK;
            berserkFinished = Sys_Global.pauseRelativeTime + PATCH_TIME_BERSERK;
            berserkSeedTime = Sys_Global.current_time;
            break;
//         case 15:
//             // Detox Patch
//             Inventory.a.patchCounts[6]--;
//             if (Inventory.a.patchCounts[6] <= 0) { depleted = true; }
//             DisableAllPatches(); // remove all other effects, even medipatch
//             patchActive = PATCH_DETOX; // overwrite all other active patches
//             detoxFinishedTime = Sys_Global.pauseRelativeTime + Const.detoxTime; // detox doesn't stack, it cancels itself lol
//             break;
//         case 16:
//             // Genius Patch
//             Inventory.a.patchCounts[5]--;
//             if (Inventory.a.patchCounts[5] <= 0) { depleted = true; }
//             if (!(Utils.CheckFlags(patchActive, PATCH_GENIUS))) patchActive += PATCH_GENIUS;
//             if (geniusFinishedTime > Sys_Global.pauseRelativeTime) {
//                 geniusFinishedTime += Const.geniusTime; // genius effect stacks
//             } else {
//                 geniusFinishedTime = Sys_Global.pauseRelativeTime + Const.geniusTime;
//             }
//             break;
//         case 17:
//             // Medi Patch
//             if (hm.health >=255) {
//                 Const.sprint(Const.a.stringTable[304],MouseLookScript.a.player);
//                 return;
//             }
//             Inventory.a.patchCounts[3]--;
//             if (Inventory.a.patchCounts[3] <= 0) { depleted = true; }
//             if (!(Utils.CheckFlags(patchActive, PATCH_MEDI))) patchActive += PATCH_MEDI;
//             PlayerHealth.a.mediPatchPulseCount = 0;
//             if (mediFinishedTime > Sys_Global.pauseRelativeTime) {
//                 mediFinishedTime += Const.mediTime; // medipatch effect stacks
//             } else {
//                 mediFinishedTime = Sys_Global.pauseRelativeTime + Const.mediTime;
//             }
//             break;
//         case 18:
//             // Reflex Patch
//             Inventory.a.patchCounts[4]--;
//             if (Inventory.a.patchCounts[4] <= 0) { depleted = true; }
//             Time.timeScale = Const.reflexTimeScale;
//             if (!(Utils.CheckFlags(patchActive, PATCH_REFLEX))) patchActive += PATCH_REFLEX;
//             if (reflexFinishedTime > Time.realtimeSinceStartup ) {
//                 reflexFinishedTime += Const.reflexTime; // reflex effect stacks
//             } else {
//                 reflexFinishedTime = Time.realtimeSinceStartup + Const.reflexTime;
//             }
//             break;
//         case 19:
//             // Sight Patch
//             Inventory.a.patchCounts[1]--;
//             if (Inventory.a.patchCounts[1] <= 0) { depleted = true; }
//             sightLight.enabled = true; // enable vision enhancement
//             sightSideEffectFinishedTime = -1f;  // reset side effect timer from previous patch
//             sightDimming.enabled = false; // deactivate side effect from previous patch
//             if (!(Utils.CheckFlags(patchActive, PATCH_SIGHT))) patchActive += PATCH_SIGHT;
//             if (sightFinishedTime > Sys_Global.pauseRelativeTime) {
//                 sightFinishedTime += Const.sightTime; // sight effect stacks
//             } else {
//                 sightFinishedTime = Sys_Global.pauseRelativeTime + Const.sightTime;
//             }
//             break;
//         case 20:
//             // Staminup Patch
//             Inventory.a.patchCounts[0]--;
//             if (Inventory.a.patchCounts[0] <= 0) depleted = true;
//             PlayerMovement.a.staminupActive = true;
//             if (!(Utils.CheckFlags(patchActive, PATCH_STAMINUP))) patchActive += PATCH_STAMINUP;
//             if (staminupFinishedTime > Sys_Global.pauseRelativeTime) {
//                 staminupFinishedTime += Const.staminupTime; // staminup effect stacks
//             } else {
//                 staminupFinishedTime = Sys_Global.pauseRelativeTime + Const.staminupTime;
//             }
// 
//             break;
    }

//     if (depleted) {
//         Inventory.a.PatchCycleDown(false);
//         Const.sprint((Const.a.stringTable[590]
//                         + Const.a.stringTable[index + 326]
//                         + Const.a.stringTable[589]),MouseLookScript.a.player);
//     } else {
//         Const.sprint((Const.a.stringTable[index + 326]
//                         + Const.a.stringTable[589]),MouseLookScript.a.player);
//     }
// 
//     Utils.PlayUIOneShotSavable(89);
//     GUIState.a.ClearOverButton();
}
