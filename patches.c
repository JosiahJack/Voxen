#define PATCH_BERSERK   1
#define PATCH_DETOX     2
#define PATCH_GENIUS    4
#define PATCH_MEDI      8
#define PATCH_REFLEX   16
#define PATCH_SIGHT    32
#define PATCH_STAMINUP 64

void ActivatePatch(int index) { // Expects the usableItems index
//     bool depleted = false;
    switch (index) {
        case 14:
            // Berserk Patch
    //         patchCounts[2]--;
    //         if (patchCounts[2] <= 0) { depleted = true; }
    //         if (!(patchActive & PATCH_BERSERK)) patchActive += PATCH_BERSERK;
            berserkFinished = voxen_globalContext.pauseRelativeTime + PATCH_TIME_BERSERK;
            berserkSeedTime = voxen_globalContext.current_time;
            break;
//         case 15:
//             // Detox Patch
//             Inventory.a.patchCounts[6]--;
//             if (Inventory.a.patchCounts[6] <= 0) { depleted = true; }
//             DisableAllPatches(); // remove all other effects, even medipatch
//             patchActive = PATCH_DETOX; // overwrite all other active patches
//             detoxFinishedTime = PauseScript.a.relativeTime + Const.detoxTime; // detox doesn't stack, it cancels itself lol
//             break;
//         case 16:
//             // Genius Patch
//             Inventory.a.patchCounts[5]--;
//             if (Inventory.a.patchCounts[5] <= 0) { depleted = true; }
//             if (!(Utils.CheckFlags(patchActive, PATCH_GENIUS))) patchActive += PATCH_GENIUS;
//             if (geniusFinishedTime > PauseScript.a.relativeTime) {
//                 geniusFinishedTime += Const.geniusTime; // genius effect stacks
//             } else {
//                 geniusFinishedTime = PauseScript.a.relativeTime + Const.geniusTime;
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
//             if (mediFinishedTime > PauseScript.a.relativeTime) {
//                 mediFinishedTime += Const.mediTime; // medipatch effect stacks
//             } else {
//                 mediFinishedTime = PauseScript.a.relativeTime + Const.mediTime;
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
//             if (sightFinishedTime > PauseScript.a.relativeTime) {
//                 sightFinishedTime += Const.sightTime; // sight effect stacks
//             } else {
//                 sightFinishedTime = PauseScript.a.relativeTime + Const.sightTime;
//             }
//             break;
//         case 20:
//             // Staminup Patch
//             Inventory.a.patchCounts[0]--;
//             if (Inventory.a.patchCounts[0] <= 0) depleted = true;
//             PlayerMovement.a.staminupActive = true;
//             if (!(Utils.CheckFlags(patchActive, PATCH_STAMINUP))) patchActive += PATCH_STAMINUP;
//             if (staminupFinishedTime > PauseScript.a.relativeTime) {
//                 staminupFinishedTime += Const.staminupTime; // staminup effect stacks
//             } else {
//                 staminupFinishedTime = PauseScript.a.relativeTime + Const.staminupTime;
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
