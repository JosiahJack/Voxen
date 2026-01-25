// todo.c - TODO stuff.  Someday this file will be deleted.  Meanwhile any stubs will go here and anything stupid.
// TODO: Figure out how to handle info_ressurection_points that needed to live outside the levels:
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
//TARGET ID: Type-LevelNum(0#)EnemyNum(###),Example: Mutant-06003, EXCEPTIONS: Cyborg-00001 is Edward Diego
#include "voxen.h"

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
