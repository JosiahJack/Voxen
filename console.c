// console.c - Console Emulator
#include "voxen.h"
#include "event.h"
#include <stdlib.h> // For atoi

#define MAX_HISTORY 7
int32_t currentEntryLength = 0;
bool consoleActive = false;
char consoleEntryText[TEXT_BUFFER_SIZE] = "Enter a command...";
char history[MAX_HISTORY][TEXT_BUFFER_SIZE] = {0};
int numHistory = 0;
int historyPos = 0;

void ToggleConsole(void) {
    static bool inventoryModeWasActivePriorToConsole = false;
    if (!consoleActive) inventoryModeWasActivePriorToConsole = inventoryMode;
    consoleActive = !consoleActive; // Tilde
    if (consoleActive) inventoryMode = true;
    else if (!inventoryModeWasActivePriorToConsole && inventoryMode) {
        inventoryMode = false;
        cursorPosition_x = (float)screen_width * 0.5f;
        cursorPosition_y = (float)screen_height * 0.5f;
    }
}

static void CheatLoadLevel(int lev) {
    CenterStatusPrint("Loading level %u", lev);
//     LevelManager.a.CheatLoadLevel(lev); TODO
}

static void AddToHistory(const char* entry) {
    if (strlen(entry) == 0) return;
    // Optional: avoid duplicates
    if (numHistory > 0 && strcmp(entry, history[numHistory - 1]) == 0) return;
    if (numHistory < MAX_HISTORY) {
        strcpy(history[numHistory], entry);
        numHistory++;
    } else {
        // Shift
        for (int i = 0; i < MAX_HISTORY - 1; i++) {
            strcpy(history[i], history[i + 1]);
        }
        strcpy(history[MAX_HISTORY - 1], entry);
    }
}

static void RecallHistory(int direction) { // direction 1 up (older), -1 down (newer)
    if (direction == 1) { // up
        if (historyPos > 0) {
            historyPos--;
            strcpy(consoleEntryText, history[historyPos]);
            currentEntryLength = strlen(consoleEntryText);
        }
    } else if (direction == -1) { // down
        if (historyPos < numHistory) {
            historyPos++;
            if (historyPos == numHistory) {
                consoleEntryText[0] = '\0';
                currentEntryLength = 0;
            } else {
                strcpy(consoleEntryText, history[historyPos]);
                currentEntryLength = strlen(consoleEntryText);
            }
        }
    }
}

static void EnterNoclip(void) {
    // PlayerMovement.a.CheatNoclip = true;
    // PlayerMovement.a.grounded = false;
    // PlayerMovement.a.rbody.useGravity = false;
    // Utils.DisableCapsuleCollider(PlayerMovement.a.capsuleCollider);
    // Utils.DisableCapsuleCollider(PlayerMovement.a.leanCapsuleCollider);
    // Utils.DisableSphereCollider(PlayerMovement.a.cyberCollider);
}

static void ExitNoclip(void) {
    // PlayerMovement.a.CheatNoclip = false;
    // PlayerMovement.a.grounded = false;
    // if (PlayerMovement.a.inCyberSpace) {
    //     Utils.EnableSphereCollider(PlayerMovement.a.cyberCollider);
    // } else {
    //     Utils.EnableCapsuleCollider(PlayerMovement.a.capsuleCollider);
    //     Utils.EnableCapsuleCollider(PlayerMovement.a.leanCapsuleCollider);
    // }
}

void ProcessConsoleCommand(const char* command) {
    if (command == NULL || strlen(command) == 0) {
        ToggleConsole();
        return;
    }

    char ts[TEXT_BUFFER_SIZE];
    strncpy(ts, command, TEXT_BUFFER_SIZE - 1);
    ts[TEXT_BUFFER_SIZE - 1] = '\0';

    // Add to history before processing
    AddToHistory(command);

    bool commandProcessed = false;

    if (strstr(ts, "noclip") || strstr(ts, "idclip") || strstr(ts, "no clip")) {
        noclip = !noclip;
        if (noclip) {
            EnterNoclip();
            CenterStatusPrint("noclip: %s", stringTable[1000]); // "ACTIVATED"
        } else {
            ExitNoclip();
            CenterStatusPrint("noclip: %s", stringTable[717]); // "DISABLED"
        }

        commandProcessed = true;
    } else if (strstr(ts, "editmode") || strstr(ts, "edit mode") || strstr(ts, "editor")) {
        editMode = !editMode;
        if (editMode) {
            CenterStatusPrint("edit mode: %s", stringTable[998]); // "Edit Mode activated! The current level can be shaped to your heart's content!"
            EnterNoclip();
            notarget = true;
        } else {
            CenterStatusPrint("%s", stringTable[999]); // "Edit Mode deactivated, normal play"
            ExitNoclip();
            notarget = false;
        }
        commandProcessed = true;
    } else if (strstr(ts, "cull")) {
        settings_CullEnabled = !settings_CullEnabled;
        CenterStatusPrint("Culling: %s", settings_CullEnabled ? stringTable[1000] : stringTable[717]);     
    } else if (strstr(ts, "notarget") || strstr(ts, "no target")) {
        notarget = !notarget;
        CenterStatusPrint("notarget: %s", notarget ? stringTable[1000] : stringTable[717]);
        commandProcessed = true;
    } else if (strstr(ts, "god") || (strstr(ts, "power") && strstr(ts, "overwhelming")) || strstr(ts, "whosyourdaddy") || strstr(ts, "iddqd")) {
        god = !god;
        CenterStatusPrint("god mode: %s", god ? stringTable[1000] : stringTable[717]);
        commandProcessed = true;
    } else if (strstr(ts, "load") && (strstr(ts, "0") || strstr(ts, "loadr") || strstr(ts, "load r")) &&
               !strstr(ts, "10") && !strstr(ts, "arsenal")) {
        if (menuActive) { CenterStatusPrint("%s", stringTable[1015]); } // "Cannot load levels via cheat while on the menu!"

        CheatLoadLevel(0);
        commandProcessed = true;
    } else if (strstr(ts, "load") && strstr(ts, "1") && !strstr(ts, "10") && !strstr(ts, "11") &&
               !strstr(ts, "12") && !strstr(ts, "13") && !strstr(ts, "g1") && !strstr(ts, "arsenal")) {
        CheatLoadLevel(1);
        commandProcessed = true;
    } else if ((strstr(ts, "load") && strstr(ts, "2") && !strstr(ts, "12") && !strstr(ts, "g2")) && !strstr(ts, "arsenal")) {
        CheatLoadLevel(2);
        commandProcessed = true;
    } else if ((strstr(ts, "load") && strstr(ts, "3") && !strstr(ts, "13") && !strstr(ts, "g3")) && !strstr(ts, "arsenal")) {
        CheatLoadLevel(3);
        commandProcessed = true;
    } else if (strstr(ts, "load") && strstr(ts, "4") && !strstr(ts, "g4") && !strstr(ts, "arsenal")) {
        CheatLoadLevel(4);
        commandProcessed = true;
    } else if (strstr(ts, "load") && strstr(ts, "5") && !strstr(ts, "arsenal")) {
        CheatLoadLevel(5);
        commandProcessed = true;
    } else if (strstr(ts, "load") && strstr(ts, "6") && !strstr(ts, "arsenal")) {
        CheatLoadLevel(6);
        commandProcessed = true;
    } else if (strstr(ts, "load") && strstr(ts, "7") && !strstr(ts, "arsenal")) {
        CheatLoadLevel(7);
        commandProcessed = true;
    } else if (strstr(ts, "load") && strstr(ts, "8") && !strstr(ts, "arsenal")) {
        CheatLoadLevel(8);
        commandProcessed = true;
    } else if (strstr(ts, "load") && strstr(ts, "9") && !strstr(ts, "arsenal")) {
        CheatLoadLevel(9);
        commandProcessed = true;
    } else if (((strstr(ts, "load") && strstr(ts, "g1")) || (strstr(ts, "load") && strstr(ts, "10"))) && !strstr(ts, "arsenal")) {
        CheatLoadLevel(10);
        commandProcessed = true;
    } else if (((strstr(ts, "load") && strstr(ts, "g2")) || (strstr(ts, "load") && strstr(ts, "11"))) && !strstr(ts, "arsenal")) {
        CheatLoadLevel(11);
        commandProcessed = true;
    } else if (((strstr(ts, "load") && strstr(ts, "g4")) || (strstr(ts, "load") && strstr(ts, "12"))) && !strstr(ts, "arsenal")) {
        CheatLoadLevel(12);
        commandProcessed = true;
    } else if (strstr(ts, "load") && strstr(ts, "g3") && !strstr(ts, "arsenal")) {
        CenterStatusPrint("%s", stringTable[1001]); // "Gamma grove already jettisoned! Those poor arrogant people."
        commandProcessed = true;
    } else if (strstr(ts, "load") && strstr(ts, "arsenal")) {
        int arsenalLev = -1;
        if (strstr(ts, "arsenalr") || strstr(ts, "arsenal r") || strstr(ts, "0")) arsenalLev = 0;
        else if (strstr(ts, "1")) arsenalLev = 1;
        else if (strstr(ts, "2")) arsenalLev = 2;
        else if (strstr(ts, "3")) arsenalLev = 3;
        else if (strstr(ts, "4")) arsenalLev = 4;
        else if (strstr(ts, "5")) arsenalLev = 5;
        else if (strstr(ts, "6")) arsenalLev = 6;
        else if (strstr(ts, "7")) arsenalLev = 7;
        else if (strstr(ts, "8")) arsenalLev = 8;
        else if (strstr(ts, "9")) arsenalLev = 9;
        else if (strstr(ts, "g1")) arsenalLev = 10;
        else if (strstr(ts, "g2")) arsenalLev = 11;
        else if (strstr(ts, "g4")) arsenalLev = 12;
        else if (strstr(ts, "g3")) {
            CenterStatusPrint("%s", stringTable[1001]); // "Gamma grove already jettisoned! Those poor arrogant people."
        }
        if (arsenalLev >= 0) {
            // PlayerMovement.a.EnableCheatArsenal(arsenalLev);
        }
        commandProcessed = true;
    } else if (strstr(ts, "bottomless") && strstr(ts, "clip")) {
        // WeaponCurrent.a.bottomless = !WeaponCurrent.a.bottomless;
        bottomless = !bottomless;
        if (bottomless) {
            CenterStatusPrint("bottomlessclip! %s", stringTable[1002]); // "Bring it!"
        } else {
            CenterStatusPrint("%s", stringTable[1003]); // "Hose disconnected from interdimensional wormhole. Normal ammo operation restored."
        }
        commandProcessed = true;
    } else if (strstr(ts, "nohud")) {
        // Const.a.noHUD = !Const.a.noHUD;
        noHUD = !noHUD;
        if (noHUD) {
            CenterStatusPrint("%s", stringTable[1004]); // "No HUD! Enjoy the cinematic screenshot experience!"
            // Deactivate all HUD elements
            // MouseLookScript.a.shootModeButton.SetActive(false);
            // MFDManager.a.overallLeftMFD.SetActive(false);
            // ... etc.
        } else {
            CenterStatusPrint("HUD %s", stringTable[1000]); // "ACTIVATED"
            // Activate all HUD elements
            // MouseLookScript.a.shootModeButton.SetActive(true);
            // MFDManager.a.overallLeftMFD.SetActive(true);
            // ... etc.
            // MFDManager.a.TabReset(true);
            // MFDManager.a.TabReset(false);
            // MFDManager.a.ReturnToLastTab(true);
            // MFDManager.a.ReturnToLastTab(false);
            // if (Inventory.a.hasHardware[1]) MouseLookScript.a.compassContainer.SetActive(true);
        }
        commandProcessed = true;
    } else if (strstr(ts, "ifeelthepower") || (strstr(ts, "i") && strstr(ts, "feel") && strstr(ts, "the") && strstr(ts, "power"))) {
        // WeaponCurrent.a.redbull = !WeaponCurrent.a.redbull;
        redbull = !redbull;
        if (redbull) {
            CenterStatusPrint("%s", stringTable[1006]); // "I feel the power! 0 energy consumption!"
        } else {
            CenterStatusPrint("%s", stringTable[1005]); // Energy usage normal
        }
        commandProcessed = true;
    } else if (strstr(ts, "show") && strstr(ts, "fps")) {
        CenterStatusPrint("%s", stringTable[1007]); // "Toggling FPS counter for framerate (bottom right corner)..."
        // PlayerMovement.a.fpsCounter.SetActive(!PlayerMovement.a.fpsCounter.activeInHierarchy);
        // Inventory.a.hardwareButtonManager.bioMonitorContainer.SetActive(true);
        commandProcessed = true;
    } else if (strstr(ts, "show") && strstr(ts, "location")) {
        CenterStatusPrint("%s", stringTable[1008]); // "Toggling locationIndicator (bottom left corner)..."
        // PlayerMovement.a.locationIndicator.SetActive(!PlayerMovement.a.locationIndicator.activeInHierarchy);
        commandProcessed = true;
    } else if (strstr(ts, "i") && strstr(ts, "am") && strstr(ts, "shodan")) {
        // LevelManager.a.superoverride = !LevelManager.a.superoverride;
        superoverride = !superoverride;
        if (superoverride) {
            CenterStatusPrint("%s", stringTable[1010]); // "Full security override enabled!"
        } else {
            CenterStatusPrint("%s", stringTable[1009]); // "SHODAN has regained control of security from you"
        }
        commandProcessed = true;
    } else if (strcmp(command, "dizzy") == 0) {
        // Cycle sky rotate speed
        // if (LevelManager.a.skyRotate.rotateSpeed < 0.9f) LevelManager.a.skyRotate.rotateSpeed = 1.f;
        // else if (LevelManager.a.skyRotate.rotateSpeed < 1.9f) LevelManager.a.skyRotate.rotateSpeed = 2.f;
        // else if (LevelManager.a.skyRotate.rotateSpeed < 4.9f) LevelManager.a.skyRotate.rotateSpeed = 5.f;
        // else if (LevelManager.a.skyRotate.rotateSpeed < 9.9f) LevelManager.a.skyRotate.rotateSpeed = 10.f;
        // else LevelManager.a.skyRotate.rotateSpeed = LevelManager.a.skyRotate.defaultSpeed;
        commandProcessed = true;
    } else if (strcmp(command, "mr. bean") == 0) {
        CenterStatusPrint("Nice try, there are no go carts to slow down here");
        commandProcessed = true;
    } else if (strcmp(command, "simon foster") == 0) {
        CenterStatusPrint("Nice try, nothing to paint here");
        commandProcessed = true;
    } else if (strcmp(command, "motherlode") == 0 || strcmp(command, "rosebud") == 0 ||
               strcmp(command, "kaching") == 0 || strcmp(command, "money") == 0) {
        CenterStatusPrint("Nice try, there's no money here.");
        commandProcessed = true;
    } else if (strcmp(command, "richard branson") == 0) {
        CenterStatusPrint("Nice try, there's no money here. You do realize this isn't Rollercoaster Tycoon right?");
        commandProcessed = true;
    } else if (strcmp(command, "john wardley") == 0) {
        CenterStatusPrint("WOW!");
        commandProcessed = true;
    } else if (strcmp(command, "john mace") == 0) {
        CenterStatusPrint("Nice try, there's nothing to pay double for here");
        commandProcessed = true;
    } else if (strcmp(command, "melanie warn") == 0) {
        CenterStatusPrint("I feel happy!!!");
        commandProcessed = true;
    } else if (strcmp(command, "damon hill") == 0) {
        CenterStatusPrint("Nice try, there are no go carts to speed up here");
        commandProcessed = true;
    } else if (strcmp(command, "michael schumacher") == 0) {
        CenterStatusPrint("Nice try, there are no go carts to give ludicrous speed here");
        commandProcessed = true;
    } else if (strcmp(command, "tony day") == 0) {
        CenterStatusPrint("Ok, now I want a hamburger");
        commandProcessed = true;
    } else if (strcmp(command, "katie brayshaw") == 0) {
        CenterStatusPrint("Hi there! Hello! Hey! Howdy!");
        commandProcessed = true;
    } else if (strstr(ts, "sudo") || strstr(ts, "admin")) {
        CenterStatusPrint("Super user access granted...ERROR: access restricted by SHODAN");
        commandProcessed = true;
    } else if (strstr(ts, "git")) {
        if (strstr(ts, "pull") || strstr(ts, "fetch")) {
            CenterStatusPrint("remote: Enumerating objects: 24601, done. Failed, could not connect with origin/triop.");
        } else if (strstr(ts, "status")) {
            CenterStatusPrint("Your branch is up to date with origin/triop. Working directory clean.");
        } else if (strstr(ts, "log")) {
            CenterStatusPrint("<Merge pull request #451 from SHODAN/NeuralLinkBugfix> 6 months ago...");
        } else if (strstr(ts, "reflog")) {
            CenterStatusPrint("dc51440 HEAD0 -> master: commit: Establish neural connection ... ERROR: invalid ID `2-4601`");
        } else if (strstr(ts, "merge")) {
            CenterStatusPrint("Failed, could not connect with origin/triop");
        } else if (strstr(ts, "push")) {
            CenterStatusPrint("Could not find Username for 'triopttp://192.168.1.451'");
        } else if (strstr(ts, "clone")) {
            CenterStatusPrint("Failed, connection blocked by SHODAN. Employee ID invalid.");
        } else if (strstr(ts, "branch") || strstr(ts, "-b")) {
            const char* last_space = strrchr(ts, ' ');
            char branch_name[256] = "unknown";
            if (last_space && last_space[1]) {
                strncpy(branch_name, last_space + 1, sizeof(branch_name) - 1);
                branch_name[sizeof(branch_name) - 1] = '\0';
            }
            CenterStatusPrint("Created new branch %s", branch_name);
        } else if (strstr(ts, "checkout")) {
            CenterStatusPrint("Branch name not recognized. Contact your TriopBucket representative.");
        } else {
            CenterStatusPrint("Branch name not recognized. Contact your TriopBucket representative.");
        }
        commandProcessed = true;
    } else if (strstr(ts, "restart")) {
        CenterStatusPrint("Yeah...better not");
        commandProcessed = true;
    } else if (strstr(ts, "quit") || strstr(ts, "exit")) {
        CenterStatusPrint("Use the Pause Menu by hitting Escape and using the QUIT option via mouse or arrow keys + ENTER");
        commandProcessed = true;
    } else if (strstr(ts, "cd") || strstr(ts, "./")) {
        CenterStatusPrint("Attempting to access directory... already at root");
        commandProcessed = true;
    } else if (strstr(ts, "kill") || strstr(ts, "kick") || strstr(ts, "ban") || strstr(ts, "destroy") ||
               strstr(ts, "attack") || strstr(ts, "suicide") || strstr(ts, "die")) {
        CenterStatusPrint("%s", stringTable[1011]); // "Player decides to become a cyborg."
        // DamageData dd = {0};
        // dd.damage = health + 1.0f;
        // dd.other = player_gameobject;
        // health_manager->TakeDamage(&dd);
        commandProcessed = true;
    } else if (strstr(ts, "justinbailey")) {
        CenterStatusPrint("Well, you don't have a suit already so...");
        commandProcessed = true;
    } else if (strstr(ts, "woodstock")) {
        CenterStatusPrint("How much wood could a woodchuck chuck...there's no wood in SPACE!");
        commandProcessed = true;
    } else if (strstr(ts, "quarry")) {
        CenterStatusPrint("There's obsidian on levels 6 and 8 if you want to feel decadant, otherwise we are lacking in the stone department.");
        commandProcessed = true;
    } else if (strstr(ts, "help")) {
        CenterStatusPrint("There's no one to save you now Hacker!");
        commandProcessed = true;
    } else if (strstr(ts, "zelda")) {
        CenterStatusPrint("Too late, already been to level 1");
        commandProcessed = true;
    } else if (strstr(ts, "allyourbasearebelongtous") || (strstr(ts, "all") && strstr(ts, "your") && strstr(ts, "base"))) {
        CenterStatusPrint("ERROR: SHODAN has overriden your command, remove SHODAN first.");
        commandProcessed = true;
    } else if (strstr(ts, "i") && strstr(ts, "am") && ((strstr(ts, "iron") && strstr(ts, "man")) ||
               strstr(ts, "amazing") || strstr(ts, "cool") || strstr(ts, "best"))) {
        CenterStatusPrint("That's nice dear.");
        commandProcessed = true;
    } else if ((strstr(ts, "impulse") && strstr(ts, "9")) || strstr(ts, "idkfa")) {
        CenterStatusPrint("I can only hold 7 weapons!! Nice try dearies!");
        commandProcessed = true;
    } else if (strstr(ts, "summon_obj")) {
        const char* last_arg = strrchr(ts, ' ');
        int val = (last_arg && last_arg[1]) ? atoi(last_arg + 1) : -1;
        if (val >= 0 && val < 438) {
            // SpawnDynamicObject(val, currentLevel, true, -1);
            // lastSpawnedGO = spawned_object;
        }
        commandProcessed = true;
    } else if (strstr(ts, "undo")) {
        if (/* lastSpawnedGO != NULL && */ editMode) {
            // Utils.SafeDestroy(lastSpawnedGO);
            // lastSpawnedGO = NULL;
        } else {
            CenterStatusPrint("Cannot undo when not in Edit Mode");
        }
        commandProcessed = true;
    } else if (strstr(ts, "settargetfps") || strstr(ts, "setfps")) {
        const char* last_arg = strrchr(ts, ' ');
        int val = (last_arg && last_arg[1]) ? atoi(last_arg + 1) : -1;
        if (val > 10 && val <= 200) {
            // Const.a.TARGET_FPS = val;
            // Config.SetVSync();
        }
        CenterStatusPrint("FPS] -> %d", val);
        commandProcessed = true;
    } else if (strstr(ts, "shake")) {
        // Const.a.Shake(true, -1, -1);
        commandProcessed = true;
    } else if (strstr(ts, "tired") || strstr(ts, "staminup")) {
        fatigueCheat = !fatigueCheat;
        if (fatigueCheat) {
            CenterStatusPrint("Stamin-Up! %s", stringTable[1013]); // "Fatigue no longer affects you!"
        } else {
            CenterStatusPrint("%s", stringTable[1012]); // "Fatigue returned to normal"
        }
        
        commandProcessed = true;
    }

    if (!commandProcessed) {
        CenterStatusPrint("%s%s", stringTable[1014], command); // "Unknown command or function: "
    }

    // Clear after processing
    consoleEntryText[0] = '\0';
    currentEntryLength = 0;
    historyPos = numHistory; // Position beyond newest for empty
    ToggleConsole();
}

void ConsoleEmulator(int32_t keycode) {
    if (keycode == GLFW_KEY_UP) {
        RecallHistory(1);
    } else if (keycode == GLFW_KEY_DOWN) {
        RecallHistory(-1);
    } 
    
    if (keycode == GLFW_KEY_U && keyStates[GLFW_KEY_LEFT_CONTROL].down) {
        consoleEntryText[0] = '\0'; // Clear the input
        currentEntryLength = 0;
        return;
    }
    
    if (keycode >= GLFW_KEY_A && keycode <= GLFW_KEY_Z) { // Handle alphabet keys
        if (currentEntryLength < (TEXT_BUFFER_SIZE - 1)) { // Ensure we don't overflow the buffer
            char c = 'a' + (keycode - GLFW_KEY_A); // Map keycode to lowercase character
            consoleEntryText[currentEntryLength] = c;
            consoleEntryText[currentEntryLength + 1] = '\0'; // Null-terminate
            currentEntryLength++;
        }
    } else if (keycode >= GLFW_KEY_1 && keycode <= GLFW_KEY_9) { // Handle number keys 1-9
        if (currentEntryLength < (TEXT_BUFFER_SIZE - 1)) {
            char c = '1' + (keycode - GLFW_KEY_1); // Map to '1'-'9'

            consoleEntryText[currentEntryLength] = c;
            consoleEntryText[currentEntryLength + 1] = '\0'; // Null-terminate
            currentEntryLength++;
        }
    } else if (keycode == GLFW_KEY_0) { // Handle '0'
        if (currentEntryLength < (TEXT_BUFFER_SIZE - 1)) {
            consoleEntryText[currentEntryLength] = '0';
            consoleEntryText[currentEntryLength + 1] = '\0'; // Null-terminate
            currentEntryLength++;
        }
    } else if (keycode == GLFW_KEY_BACKSPACE && currentEntryLength > 0) { // Handle backspace
        currentEntryLength--;
        consoleEntryText[currentEntryLength] = '\0'; // Null-terminate
    } else if (keycode == GLFW_KEY_SPACE) { // Handle space
        if (currentEntryLength < (TEXT_BUFFER_SIZE - 1)) {
            consoleEntryText[currentEntryLength] = ' ';
            consoleEntryText[currentEntryLength + 1] = '\0';
            currentEntryLength++;
        }
    } else if (keycode == GLFW_KEY_ENTER || keycode == GLFW_KEY_KP_ENTER) { // Handle enter (main and keypad)
        // Handle command execution or clear the console
        DualLog("Console command: %s\n", consoleEntryText);
        ProcessConsoleCommand(consoleEntryText);
    }
}
