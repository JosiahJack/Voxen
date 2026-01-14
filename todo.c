// todo.c - TODO stuff.  Someday this file will be deleted.  Meanwhile any stubs will go here and anything stupid.
// TODO: Figure out how to handle info_ressurection_points that needed to live outside the levels:
// Level R -27.386 -55.488 26.5941
// Level 1 40.903 -42.372 -30.78
// Level 2 30.67407 -25.832 10.21412
// Level 3 38.26813 -15.498 20.37825
// Level 4 -19.48 -7.928 22.954
// Level 5 -24.358 12.5956 31.8497
// Level 6 -22.3568 33.7845 -30.728
// Level 7 2.228084 50.95243 7.532025
// Level 9.1_resdest 2.303 106.77 -38.554 (I don't remember what this is for, cheat spawn from `load 9`??)
// TODO: Level loading
// TODO: Keybinds
// TODO: Multiview renders for sensaround
// TODO: Proper physics
// TODO: Particle system
// TODO: Raycasts
// TODO: Voxel GI
// TODO: Scripting engine for gameplay
// TODO: Save/Load system
// TODO: Directional lights for cyberspace
// TODO: Directional light for sunlight
// TODO: Directional light shadowmapping just for sunlight
// TODO: Let user switch monitors from settings, especially in fullscreen.
#include "voxen.h"

float correctionX, correctionY, correctionZ;
float correctionNPCX, correctionNPCY, correctionNPCZ;
float correctionDoorsX, correctionDoorsY, correctionDoorsZ;
float correctionDynamicsX, correctionDynamicsY, correctionDynamicsZ;
float correctionLightsSaveableX, correctionLightsSaveableY, correctionLightsSaveableZ;
float correctionStaticImmutableX, correctionStaticImmutableY, correctionStaticImmutableZ;
float correctionStaticSaveableX, correctionStaticSaveableY, correctionStaticSaveableZ;
float correctionLightX, correctionLightY, correctionLightZ;

void GetLevel_Transform_Offsets(int32_t curlevel, float* ofsx, float* ofsy, float* ofsz) {
    if (!Sys_Global.global_modIsCitadel) { *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f;  return; } // TODO: Resave levels with the offsets applied.
    
    switch(curlevel) { // Match the parent transforms #.NAMELevel, e.g. 1.MedicalLevel
        case 0:  *ofsx = 3.6f; *ofsy = -4.10195f; *ofsz = 1.0f; break;
        case 1:  *ofsx = 25.56f; *ofsy = -48.64f; *ofsz = -5.2f; break;
        case 2:  *ofsx = -2.6f; *ofsy = 0.0f; *ofsz = -7.7f; break;
        case 3:  *ofsx = -45.12f; *ofsy = -0.700374f; *ofsz = -16.32f; break;
        case 4:  *ofsx = -20.4f; *ofsy = 0.0f; *ofsz = 11.48f; break;
        case 5:  *ofsx = -10.14f; *ofsy = 0.065f; *ofsz = -0.0383f; break;
        case 6:  *ofsx = -0.6728f; *ofsy = 0.1725f; *ofsz = 3.76f; break;
        case 7: *ofsx = -6.7f; *ofsy = 0.24443f; *ofsz = 1.16f; break;
        case 8:  *ofsx = 1.08f; *ofsy = -0.935f; *ofsz = 0.8f; break;
        case 9:  *ofsx = 3.6f; *ofsy = 0.0f; *ofsz = -1.28f; break;
        case 10: *ofsx = 107.37f; *ofsy = 101.2f; *ofsz = 35.48f; break;
        case 11: *ofsx = 15.05f; *ofsy = 129.9f; *ofsz = -77.94f; break;
        case 12:  *ofsx = 19.04f; *ofsy = 162.2f; *ofsz = 95.8f; break;
        case LEVEL_CYBERSPACE: *ofsx = 164.7f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

void GetLevel_Dynamic_ContainerOffsets(int32_t curlevel, float* ofsx, float* ofsy, float* ofsz) {
    if (!Sys_Global.global_modIsCitadel) { *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f;  return; }

    switch(curlevel) { // Match the parent transforms #.NAMELevel, e.g. 1.DynamicObjectsSaveableInstantiated
        case 0:  *ofsx = -1.2417f; *ofsy = -0.26194f; *ofsz = -1.0883f; break;
        case 1:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 2:  *ofsx = -0.98611f; *ofsy = 0.84f; *ofsz = 1.1906f; break;
        case 3:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 4:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 5:  *ofsx = 0.0f; *ofsy = 0.07f; *ofsz = 0.0f; break;
        case 6:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 7:  *ofsx = 0.0f; *ofsy = 0.04f; *ofsz = 0.0f; break;
        case 8:  *ofsx = 0.0f; *ofsy = 0.16f; *ofsz = 0.0f; break;
        case 9:  *ofsx = 0.0f; *ofsy = 0.08f; *ofsz = 0.0f; break;
        case 10: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 11: *ofsx = 0.0f; *ofsy = 0.32f; *ofsz = 0.0f; break;
        case 12: *ofsx = 0.0f; *ofsy = 0.2f; *ofsz = 0.0f; break;
        case LEVEL_CYBERSPACE: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

void GetLevel_LightsStaticSaveable_ContainerOffsets(int32_t curlevel, float* ofsx, float* ofsy, float* ofsz) {
    if (!Sys_Global.global_modIsCitadel) { *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f;  return; }

    switch(curlevel) { // Match the parent transforms #.NAMELevel, e.g. 1.LightsStaticSaveable
        case 0:  *ofsx = -1.2417f; *ofsy = -0.26194f; *ofsz = -1.0883f; break;
        case 1:  *ofsx = 0.589f; *ofsy = -0.554f; *ofsz = -0.907f; break;
        case 2:  *ofsx = -0.98611f; *ofsy = 0.82105f; *ofsz = 1.1906f; break;
        case 3:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 4:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 5:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 6:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 7:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 8:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 9:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 10: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 11: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 12: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case LEVEL_CYBERSPACE: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

void GetLevel_LightsStaticImmutable_ContainerOffsets(int32_t curlevel, float* ofsx, float* ofsy, float* ofsz) {
    if (!Sys_Global.global_modIsCitadel) { *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f;  return; }

    switch(curlevel) { // Match the parent transforms #.NAMELevel, e.g. 1.LightsStaticImmutable
        case 0:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 1:  *ofsx = 0.589f; *ofsy = -0.554f; *ofsz = -0.907f; break;
        case 2:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 3:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 4:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 5:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 6:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 7:  *ofsx = -14.528f; *ofsy = 48.269f; *ofsz = -26.836f; break;
        case 8:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 9:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 10: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 11: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 12: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case LEVEL_CYBERSPACE: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

void GetLevel_DoorsStaticSaveable_ContainerOffsets(int32_t curlevel, float* ofsx, float* ofsy, float* ofsz) {
    if (!Sys_Global.global_modIsCitadel) { *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f;  return; }

    switch(curlevel) { // Match the parent transforms #.NAMELevel, e.g. 1.DoorsStaticSaveable
        case 0:  *ofsx = -1.2417f; *ofsy = -0.26194f; *ofsz = -1.0883f; break;
        case 1:  *ofsx = 0.589f; *ofsy = -0.554f; *ofsz = -0.907f; break;
        case 2:  *ofsx = -0.98611f; *ofsy = 0.82105f; *ofsz = 1.1906f; break;
        case 3:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 4:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 5:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 6:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 7:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 8:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 9:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 10: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 11: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 12: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case LEVEL_CYBERSPACE: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

void GetLevel_StaticObjectsSaveable_ContainerOffsets(int32_t curlevel, float* ofsx, float* ofsy, float* ofsz) {
    if (!Sys_Global.global_modIsCitadel) { *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f;  return; }

    switch(curlevel) { // Match the parent transforms #.NAMELevel, e.g. 1.StaticObjectsSaveable
        case 0:  *ofsx = -1.2417f; *ofsy = -0.26194f; *ofsz = -1.0883f; break;
        case 1:  *ofsx = 0.589f; *ofsy = -0.554f; *ofsz = -0.907f; break;
        case 2:  *ofsx = -0.98611f; *ofsy = 0.82105f; *ofsz = 1.1906f; break;
        case 3:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 4:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 5:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 6:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 7:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 8:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 9:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 10: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 11: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 12: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case LEVEL_CYBERSPACE: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

void GetLevel_StaticObjectsImmutable_ContainerOffsets(int32_t curlevel, float* ofsx, float* ofsy, float* ofsz) {
    if (!Sys_Global.global_modIsCitadel) { *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f;  return; }

    switch(curlevel) { // Match the parent transforms #.NAMELevel, e.g. 1.StaticObjectsImmutable
        case 0:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 1:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 2:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 3:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 4:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 5:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 6:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 7:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 8:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 9:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 10: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 11: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 12: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case LEVEL_CYBERSPACE: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

void GetLevel_NPCsSaveableInstantiated_ContainerOffsets(int32_t curlevel, float* ofsx, float* ofsy, float* ofsz) {
    if (!Sys_Global.global_modIsCitadel) { *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f;  return; }

    switch(curlevel) { // Match the parent transforms #.NAMELevel, e.g. 1.NPCsSaveableInstantiated
        case 0:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 1:  *ofsx = -33.28f; *ofsy = 48.64f; *ofsz = 7.679996f; break;
        case 2:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 3:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 4:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 5:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 6:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 7:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 8:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 9:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 10: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 11: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 12: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case LEVEL_CYBERSPACE: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

void SetUnityHierarchyOffsets(uint8_t curlevel) {
    GetLevel_Transform_Offsets(curlevel, &correctionX, &correctionY, &correctionZ);
    GetLevel_Dynamic_ContainerOffsets(curlevel, &correctionDynamicsX, &correctionDynamicsY, &correctionDynamicsZ);
    GetLevel_LightsStaticSaveable_ContainerOffsets(curlevel, &correctionLightsSaveableX, &correctionLightsSaveableY, &correctionLightsSaveableZ);
    GetLevel_StaticObjectsSaveable_ContainerOffsets(curlevel, &correctionStaticSaveableX, &correctionStaticSaveableY, &correctionStaticSaveableZ);
    GetLevel_StaticObjectsImmutable_ContainerOffsets(curlevel, &correctionStaticImmutableX, &correctionStaticImmutableY, &correctionStaticImmutableZ);
    GetLevel_LightsStaticImmutable_ContainerOffsets(curlevel, &correctionLightX, &correctionLightY, &correctionLightZ);
    GetLevel_DoorsStaticSaveable_ContainerOffsets(curlevel, &correctionDoorsX, &correctionDoorsY, &correctionDoorsZ);
    GetLevel_NPCsSaveableInstantiated_ContainerOffsets(curlevel, &correctionNPCX, &correctionNPCY, &correctionNPCZ);
}

// Apply the Unity hierarchy nonsense, TODO: Save out level#.txt from the engine just once and then delete all this.
void ApplyUnityHierarchyCorrectionAtLevelLoad(uint16_t instanceIdx, uint16_t entIdx) {
        if (Sys_Global.levelCurrentlyLoading && entIdx != 755 && entIdx != 590) { // Adjusted for in the level data directly, no correction.
        instances[instanceIdx].position.x += correctionX;   
        instances[instanceIdx].position.y += correctionY;
        instances[instanceIdx].position.z += correctionZ;
        if (ConstIndexIsDoor(entIdx)) {
            instances[instanceIdx].position.x += correctionDoorsX;
            instances[instanceIdx].position.y += correctionDoorsY;
            instances[instanceIdx].position.z += correctionDoorsZ;
        } else if (ConstIndexIsNPC(entIdx)) {
            instances[instanceIdx].position.x += correctionNPCX;
            instances[instanceIdx].position.y += correctionNPCY - 1.0f; // Offset to center them up in their capsule
            instances[instanceIdx].position.z += correctionNPCZ;
            Vector3 axis = (Vector3){0.0f, 0.0f, 1.0f}; // X-axis
            instances[instanceIdx].rotation = axis_angle_quaternion(axis, deg2rad(-90.0f));
        } else if (ConstIndexIsStaticObjectSaveable(entIdx)) {
            instances[instanceIdx].position.x += correctionStaticSaveableX;
            instances[instanceIdx].position.y += correctionStaticSaveableY;
            instances[instanceIdx].position.z += correctionStaticSaveableZ;
        } else if (ConstIndexIsStaticObjectImmutable(entIdx)) {
            instances[instanceIdx].position.x += correctionStaticImmutableX;
            instances[instanceIdx].position.y += correctionStaticImmutableY;
            instances[instanceIdx].position.z += correctionStaticImmutableZ;
        } else if (ConstIndexIsDynamicObject(entIdx)) { // MUST BE LAST AS IT OVERLAPS WITH NPC AND LIGHTS SAVEABLE!
            instances[instanceIdx].position.x += correctionDynamicsX;
            instances[instanceIdx].position.y += correctionDynamicsY;
            instances[instanceIdx].position.z += correctionDynamicsZ;
        } 
    }
}

void EnableCheatArsenal(uint8_t level) {
    switch(level) {
        default: break;
    }
}

uint16_t SpawnDynamicObject(int val, bool cheat) {
    if (!ConstIndexInBounds(val)) { DualLogError("Const index out of bounds: %u", val); return NULLENT; } // Checked in cmd_summon but used elsewhere so guard here too.
    
    if (cheat) DualLog("Cheat spawn constIndex %u, level: %u, from cheat: %u, name: ", val, Sys_Global.currentLevel, cheat);
//     Vector3 spawnPos = (Vector3){0.0,0.0,0.0};
//     if (cheat) spawnPos = (Vector3){instances[PLAYER1].position.x,instances[PLAYER1].position.y,instances[PLAYER1].position.z};
    if (ConstIndexIsGeometry(val)/* && !Sys_Cheats.editMode*/) { CenterStatusPrint("Indices 0 through 306 (level geometry chunks) not possible when not on edit mode!"); return NULLENT; }
    
    uint16_t entityIndexInInstanceTable = NULLENT;//MonoBehaviour.Instantiate(Const.a.GetPrefab(val),spawnPos, Const.a.quaternionIdentity) as Entity;
    if (cheat && ConstIndexIsHardware(val)) { // Hardware
//         UseableObjectUse uo = go.GetComponent<UseableObjectUse>();
//         int dex14 = Inventory.a.hardware14fromConstdex(uo.useableItemIndex);
//         if (Inventory.a.hasHardware[dex14]) uo.customIndex = (Inventory.a.hardwareVersion[dex14] + 1);
    }

    return entityIndexInInstanceTable;
}

void cmd_kill(void) {
    CenterStatusPrint("%s", voxen_Text.stringTable[1011]); // "Player decides to become a cyborg."
    // TakeDamage(...)
}

void cmd_undo(void) {
    if (Sys_Cheats.editMode) {
        // Utils.SafeDestroy(lastSpawnedGO); lastSpawnedGO = NULL;
        CenterStatusPrint("Last spawned object removed");
    } else {
        CenterStatusPrint("Cannot undo when not in Edit Mode");
    }
}

void cmd_shake(void) {
    // Const.a.Shake(true, -1, -1);
    CenterStatusPrint("SHAKE IT!");
}

void ApplyCorpseFriction(uint16_t instanceIdx) {
    instances[instanceIdx].dynamicFriction = 10.0f;
    instances[instanceIdx].staticFriction = 10.0f;
    instances[instanceIdx].bounciness = 0.0f;
    instances[instanceIdx].frictionCombine = PHYS_COMBINE_MUL;
    instances[instanceIdx].bounceCombine = PHYS_COMBINE_MAX;
}

float GetPainStatic(void) { // TODO: Hook into pain/health management and shield impact effect
    return 0.0f;
}

Color GetPainStaticColor(void) { // TODO: Hook staticColor up to red or blue for pain or shield impact.
    return (Color){1.0f,0.0f,0.0f,1.0f};
}

double monitorSwitchTime;
int currentMonitorIndex = 1; // Start on primary after first cycle, puts it a 0.
void CycleToNextMonitor(GLFWwindow* window) {
    if (get_time() < monitorSwitchTime) return;
    
    monitorSwitchTime = get_time() + 1.5; // Dumb hack to prevent toggling every frame from keypress illogic
    int monitorCount;
    GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
    if (!monitors || monitorCount < 2) return;

    currentMonitorIndex = (currentMonitorIndex + 1) % monitorCount;
    GLFWmonitor* next = monitors[currentMonitorIndex];

    int mx, my;
    glfwGetMonitorPos(next, &mx, &my);
    const GLFWvidmode* mode = glfwGetVideoMode(next);
    int xpos = mx + (mode->width - Sys_Settings.ScreenWidth) / 2;
    int ypos = my + (mode->height - Sys_Settings.ScreenHeight) / 2;
    glfwSetWindowPos(window, xpos, ypos);
    Sys_Input.ignore_next_mouse_delta = true;
    DualLog("Window moved to monitor %d: %s at x: %d, y: %d\n", currentMonitorIndex, glfwGetMonitorName(next), xpos, ypos);
}
