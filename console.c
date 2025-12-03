// console.c - Console Emulator
#include "voxen.h"
#include "event.h"
#include "entity.h"
#include <ctype.h> // For tolower
#include <stdlib.h> // For atoi
#include <stdio.h> // For snprintf

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

static void AddToHistory(const char* entry) {
    if (strlen(entry) == 0) return;
    if (numHistory > 0 && strcmp(entry, history[numHistory - 1]) == 0) return;
    
    if (numHistory < MAX_HISTORY) {
        strcpy(history[numHistory], entry);
        numHistory++;
    } else {
        for (int i = 0; i < MAX_HISTORY - 1; i++) strcpy(history[i], history[i + 1]);
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
    // PlayerMovement.a.CheatNoclip = true; // TODO
    // PlayerMovement.a.grounded = false;
    // PlayerMovement.a.rbody.useGravity = false;
    // Utils.DisableCapsuleCollider(PlayerMovement.a.capsuleCollider);
    // Utils.DisableCapsuleCollider(PlayerMovement.a.leanCapsuleCollider);
    // Utils.DisableSphereCollider(PlayerMovement.a.cyberCollider);
}

static void ExitNoclip(void) {
    // PlayerMovement.a.CheatNoclip = false; // TODO
    // PlayerMovement.a.grounded = false;
    // if (PlayerMovement.a.inCyberSpace) {
    //     Utils.EnableSphereCollider(PlayerMovement.a.cyberCollider);
    // } else {
    //     Utils.EnableCapsuleCollider(PlayerMovement.a.capsuleCollider);
    //     Utils.EnableCapsuleCollider(PlayerMovement.a.leanCapsuleCollider);
    // }
}

typedef void (*ConsoleCmdFuncNoArg)(void);
typedef void (*ConsoleCmdFuncInt)(int);
typedef void (*ConsoleCmdFuncStr)(const char*);

typedef struct {
    const char* name;
    union {
        ConsoleCmdFuncNoArg  noArg;
        ConsoleCmdFuncInt    withInt;
        ConsoleCmdFuncStr    withStr;
        void*                raw;
    } func;
    enum { CMD_NOARG, CMD_INT, CMD_STR } type;
} ConsoleCommand;

static int CommandMatch(const char* input, const char* cmd) {
    while (*cmd && *input) {
        char c1 = tolower((unsigned char)*input++);
        char c2 = tolower((unsigned char)*cmd++);
        if (c1 == ' ' || c1 == '_') c1 = ' ';
        if (c2 == ' ' || c2 == '_') c2 = ' ';
        if (c1 != c2) return 0;
    }

    return *cmd == '\0' && (*input == '\0' || isspace((unsigned char)*input) || *input == '_' || *input == '\0');
}

static void cmd_noclip(void) {
    noclip = !noclip;
    if (noclip) {
        EnterNoclip();
        CenterStatusPrint("noclip: %s", stringTable[1000]); // "ACTIVATED"
    } else {
        ExitNoclip();
        CenterStatusPrint("noclip: %s", stringTable[717]); // "DISABLED"
    }
}

static void cmd_edit(void) {
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
}

static void cmd_savegeometry(void) {
    if (!editMode) {
        CenterStatusPrint("savegeometry only works in edit mode!");
        return;
    }

    char filename[64];
    snprintf(filename, sizeof(filename), "./Data/level%u.txt", currentLevel);

    FILE* f = fopen(filename, "w");
    if (!f) {
        CenterStatusPrint("Failed to open %s for writing!", filename);
        return;
    }

    DualLog("Saving current level geometry to %s...", filename);

    // We'll save only geometry chunks for now (constIndex 0–306 except 112, and 760)
    int saved = 0;
    for (uint16_t i = START_INDEX_LEVEL_INSTANCES; i < loadedInstances; ++i) {
        Entity* ent = &instances[i];
        uint16_t idx = ent->index;

        if (!ConstIndexIsGeometry(idx)) continue;

        const char* name = "unknown";
        if (idx < entityCount && entities[idx].path[0]) {
            const char* slash = strrchr(entities[idx].path, '/');
            name = slash ? slash + 1 : entities[idx].path;
            const char* dot = strstr(name, ".fbx");
            if (dot) name = dot - 4; // strip .fbx
        }

        fprintf(f, "constIndex:%u|%s (%u)|", idx, name, i - START_INDEX_LEVEL_INSTANCES + 1);
        fprintf(f, "localPosition.x:%08.5f|", ent->position.x);
        fprintf(f, "localPosition.y:%08.5f|", ent->position.y);
        fprintf(f, "localPosition.z:%08.5f|", ent->position.z);
        fprintf(f, "localRotation.x:%08.5f|", ent->rotation.x);
        fprintf(f, "localRotation.y:%08.5f|", ent->rotation.y);
        fprintf(f, "localRotation.z:%08.5f|", ent->rotation.z);
        fprintf(f, "localRotation.w:%08.5f|", ent->rotation.w);
        fprintf(f, "localScale.x:%08.5f|",    ent->scale.x);
        fprintf(f, "localScale.y:%08.5f|",    ent->scale.y);
        fprintf(f, "localScale.z:%08.5f|",     ent->scale.z);
        fprintf(f, "go.activeSelf:%s|\n", (ent->entflags & ENTFLAG_ACTIVE) ? "True" : "False");

        saved++;
    }

    fclose(f);
    CenterStatusPrint("Saved %d geometry objects to %s", saved, filename);
    DualLog("Saved %d geometry objects.", saved);
}

static int ParseLevelArg(const char* arg) {
    if (!arg || !*arg) return -1;

    char clean[64] = {0};
    int j = 0;
    for (int i = 0; arg[i] && j < 60; i++) {
        if (arg[i] != ' ' && arg[i] != '_') clean[j++] = tolower((unsigned char)arg[i]);
    }
    
    clean[j] = '\0';

    // Special cases
    if (strcmp(clean, "r") == 0 || strstr(clean, "reactor")) return 0;
    if (strstr(clean, "g1") || strstr(clean, "10")) return 10;
    if (strstr(clean, "g2") || strstr(clean, "11")) return 11;
    if (strstr(clean, "g4") || strstr(clean, "12")) return 12;
    if (strstr(clean, "g3")) {
        CenterStatusPrint("%s", stringTable[1001]); // "Gamma grove already jettisoned! Those poor arrogant people."
        return -2; // Special code: do not load
    }

    int level = atoi(clean);
    if (level >= 0 && level < numLevels) return level;
    return -1; // Invalid
}

static void cmd_loadlevel(const char* arg) {
    if (menuActive) { CenterStatusPrint("%s", stringTable[1015]); return; } // "Cannot load levels via cheat while on the main menu!"

    int level = ParseLevelArg(arg);
    if (level >= 0) {
        if (level == -2) return; // Already printed g3 message
        if (level < 0) { CenterStatusPrint("Invalid level argument"); return; }
        
        CenterStatusPrint("Loading level %u", level);
        // LevelManager.a.LoadLevel(level); // TODO
    }
}

static void cmd_loadarsenal(const char* arg) {
    int level = ParseLevelArg(arg);
    if (level == -2) return; // g3 message already shown
    
    if (level >= 0) {
        // EnableCheatArsenal(level);
    }
}

static void cmd_summon(int itemConstIndex) {
    if (itemConstIndex >= 0 && itemConstIndex < 438) {
        // SpawnDynamicObject(itemConstIndex, currentLevel, true, -1);
        CenterStatusPrint("Summoned object ID %d", itemConstIndex);
    } else {
        CenterStatusPrint("Invalid object ID: %s", itemConstIndex);
    }
}

static void cmd_notarget(void) { notarget = !notarget; CenterStatusPrint("notarget: %s", notarget ? stringTable[1000] : stringTable[717]); }
static void cmd_cull(void) { settings_CullEnabled = !settings_CullEnabled; CenterStatusPrint("Culling: %s", settings_CullEnabled ? stringTable[1000] : stringTable[717]);  }
static void cmd_showfps(void) { showFPS = !showFPS; }
static void cmd_showlocation(void) { showLocation = !showLocation; }
static void cmd_help(void) { CenterStatusPrint("There's no one to save you now Hacker!"); }
static void cmd_nomoney(void) { CenterStatusPrint("Nice try, there's no money here."); }
static void cmd_god(void) { god = !god; CenterStatusPrint("god mode: %s", god ? stringTable[1000] : stringTable[717]); }
static void cmd_energy(void) {
    redbull = !redbull; 
    if (redbull) CenterStatusPrint("%s", stringTable[1006]); // "I feel the power! 0 energy consumption!"
    else CenterStatusPrint("%s", stringTable[1005]); // Energy usage normal
}

static void cmd_dizzy(void) {
    dizzyLevel++;
    if (dizzyLevel > 4) dizzyLevel = 0;
    float speeds[] = { 0.05f, 1.0f, 2.5f, 3.75f, 6.25f };
    skyRotateSpeed = speeds[dizzyLevel];
    glProgramUniform1f(imageBlitShaderProgram, 30, skyRotateSpeed);
}

static void cmd_bottomless(void) {
    bottomless = !bottomless;
    if (bottomless) CenterStatusPrint("bottomlessclip! %s", stringTable[1002]); // "Bring it!"
    else CenterStatusPrint("%s", stringTable[1003]); // "Hose disconnected from interdimensional wormhole. Normal ammo operation restored."
}

static void cmd_nohud(void) {
    noHUD = !noHUD;
    if (noHUD) CenterStatusPrint("%s", stringTable[1004]); // "No HUD! Enjoy the cinematic screenshot experience!"
    else CenterStatusPrint("HUD %s", stringTable[1000]); // "ACTIVATED"
}

static void cmd_iamshodan(void) {
    superoverride = !superoverride;
    if (superoverride) {
        CenterStatusPrint("%s", stringTable[1010]); // "Full security override enabled!"
    } else {
        CenterStatusPrint("%s", stringTable[1009]); // "SHODAN has regained control of security from you"
    }
}

static void cmd_mrbean(void)         { CenterStatusPrint("Nice try, there are no go carts to slow down here"); }
static void cmd_simonfoster(void)    { CenterStatusPrint("Nice try, nothing to paint here"); }
static void cmd_richardbranson(void) { CenterStatusPrint("Nice try, there's no money here. You do realize this isn't Rollercoaster Tycoon right?"); }
static void cmd_johnwardley(void)    { CenterStatusPrint("WOW!"); }
static void cmd_johnmace(void)       { CenterStatusPrint("Nice try, there's nothing to pay double for here"); }
static void cmd_melaniewarn(void)    { CenterStatusPrint("I feel happy!!!"); }
static void cmd_damonhill(void)      { CenterStatusPrint("Nice try, there are no go carts to speed up here"); }
static void cmd_michaelschumacher(void) { CenterStatusPrint("Nice try, there are no go carts to give ludicrous speed here"); }
static void cmd_tonyday(void)        { CenterStatusPrint("Ok, now I want a hamburger"); }
static void cmd_katiebrayshaw(void)  { CenterStatusPrint("Hi there! Hello! Hey! Howdy!"); }

static void cmd_sudo(void)           { CenterStatusPrint("Super user access granted...ERROR: access restricted by SHODAN"); }

static void cmd_git(const char* arg) {
    if (!arg) arg = "";
    if (strstr(arg, "pull") || strstr(arg, "fetch")) {
        CenterStatusPrint("remote: Enumerating objects: 24601, done. Failed, could not connect with origin/triop.");
    } else if (strstr(arg, "status")) {
        CenterStatusPrint("Your branch is up to date with origin/triop. Working directory clean.");
    } else if (strstr(arg, "log")) {
        CenterStatusPrint("<Merge pull request #451 from SHODAN/NeuralLinkBugfix> 6 months ago...");
    } else if (strstr(arg, "reflog")) {
        CenterStatusPrint("dc51440 HEAD0 -> master: commit: Establish neural connection ... ERROR: invalid ID `2-4601`");
    } else if (strstr(arg, "merge")) {
        CenterStatusPrint("Failed, could not connect with origin/triop");
    } else if (strstr(arg, "push")) {
        CenterStatusPrint("Could not find Username for 'triopttp://192.168.1.451'");
    } else if (strstr(arg, "clone")) {
        CenterStatusPrint("Failed, connection blocked by SHODAN. Employee ID invalid.");
    } else if (strstr(arg, "branch") || strstr(arg, "-b")) {
        const char* last = strrchr(arg, ' ');
        const char* name = last ? last + 1 : "unknown";
        CenterStatusPrint("Created new branch %s", name);
    } else if (strstr(arg, "checkout")) {
        CenterStatusPrint("Branch name not recognized. Contact your TriopBucket representative.");
    } else {
        CenterStatusPrint("Branch name not recognized. Contact your TriopBucket representative.");
    }
}

static void cmd_restart(void)        { CenterStatusPrint("Yeah...better not"); }
static void cmd_quit(void)           { CenterStatusPrint("Use the Pause Menu by hitting Escape and using the QUIT option via mouse or arrow keys + ENTER"); }
static void cmd_cd(void)             { CenterStatusPrint("Attempting to access directory... already at root"); }

static void cmd_kill(void) {
    CenterStatusPrint("%s", stringTable[1011]); // "Player decides to become a cyborg."
    // TODO: TakeDamage(...)
}

static void cmd_justinbailey(void)   { CenterStatusPrint("Well, you don't have a suit already so..."); }
static void cmd_woodstock(void)      { CenterStatusPrint("How much wood could a woodchuck chuck...there's no wood in SPACE!"); }
static void cmd_quarry(void)         { CenterStatusPrint("There's obsidian on levels 6 and 8 if you want to feel decadent, otherwise we are lacking in the stone department."); }
static void cmd_zelda(void)          { CenterStatusPrint("Too late, already been to level 1"); }
static void cmd_allyourbase(void)    { CenterStatusPrint("ERROR: SHODAN has overriden your command, remove SHODAN first."); }
static void cmd_iamironman(void)     { CenterStatusPrint("That's nice dear."); }
static void cmd_idkfa(void)          { CenterStatusPrint("I can only hold 7 weapons!! Nice try dearies!"); }
static void cmd_ai(void)             { CenterStatusPrint("Only AI allowed around here is SHODAN"); }
static void cmd_aireal(void)         { CenterStatusPrint("In my magnificence, I shape clay, crafting new lifeforms..."); }

static void cmd_undo(void) {
    if (editMode) {
        // Utils.SafeDestroy(lastSpawnedGO); lastSpawnedGO = NULL;
        CenterStatusPrint("Last spawned object removed");
    } else {
        CenterStatusPrint("Cannot undo when not in Edit Mode");
    }
}

static void cmd_shake(void) {
    // Const.a.Shake(true, -1, -1); // TODO
    CenterStatusPrint("SHAKE IT!");
}

static void cmd_staminup(void) {
    fatigueCheat = !fatigueCheat;
    if (fatigueCheat) {
        CenterStatusPrint("Stamin-Up! %s", stringTable[1013]);
    } else {
        CenterStatusPrint("%s", stringTable[1012]);
    }
}

static const ConsoleCommand g_ConsoleCommands[] = {
    { "noclip",  {.noArg = cmd_noclip}, CMD_NOARG},
    { "idclip",  {.noArg = cmd_noclip}, CMD_NOARG},
    { "no clip", {.noArg = cmd_noclip}, CMD_NOARG},
    { "god",           {.noArg = cmd_god}, CMD_NOARG},
    { "overwhelming",  {.noArg = cmd_god}, CMD_NOARG},
    { "whosyourdaddy", {.noArg = cmd_god}, CMD_NOARG},
    { "iddqd",         {.noArg = cmd_god}, CMD_NOARG},
    { "notarget", {.noArg = cmd_notarget}, CMD_NOARG},
    { "no target",{.noArg = cmd_notarget}, CMD_NOARG},
    { "editmode",  {.noArg = cmd_edit}, CMD_NOARG},
    { "edit",      {.noArg = cmd_edit}, CMD_NOARG},
    { "edit mode", {.noArg = cmd_edit}, CMD_NOARG},
    { "editor",    {.noArg = cmd_edit}, CMD_NOARG},
    { "cull", {.noArg = cmd_cull}, CMD_NOARG},
    { "showfps", {.noArg = cmd_showfps}, CMD_NOARG},
    { "show fps", {.noArg = cmd_showfps}, CMD_NOARG},
    { "showlocation", {.noArg = cmd_showlocation}, CMD_NOARG},
    { "show location", {.noArg = cmd_showlocation}, CMD_NOARG},
    { "nohud", {.noArg = cmd_nohud}, CMD_NOARG},
    { "no hud", {.noArg = cmd_nohud}, CMD_NOARG},
    { "bottomlessclip", {.noArg = cmd_bottomless}, CMD_NOARG},
    { "bottomless clip", {.noArg = cmd_bottomless}, CMD_NOARG},
    { "load", {.withStr = cmd_loadlevel}, CMD_STR},
    { "loadarsenal",  {.withStr = cmd_loadarsenal}, CMD_STR},
    { "load arsenal", {.withStr = cmd_loadarsenal}, CMD_STR},
    { "summon_obj", {.withInt = cmd_summon}, CMD_INT},
    { "summonobj", {.withInt = cmd_summon}, CMD_INT},
    { "motherlode", {.noArg = cmd_nomoney}, CMD_NOARG},
    { "rosebud",    {.noArg = cmd_nomoney}, CMD_NOARG},
    { "kaching",    {.noArg = cmd_nomoney}, CMD_NOARG},
    { "money",      {.noArg = cmd_nomoney}, CMD_NOARG},
    { "dizzy", {.noArg = cmd_dizzy}, CMD_NOARG},
    { "help", {.noArg = cmd_help}, CMD_NOARG},
    { "ifeelthepower", {.noArg = cmd_energy}, CMD_NOARG},
    { "power", {.noArg = cmd_energy}, CMD_NOARG},
    { "energy", {.noArg = cmd_energy}, CMD_NOARG},
    { "i feel the power", {.noArg = cmd_energy}, CMD_NOARG},
    { "i am shodan",     {.noArg = cmd_iamshodan},        CMD_NOARG },
    { "iamshodan",       {.noArg = cmd_iamshodan},        CMD_NOARG },
    { "mr. bean",        {.noArg = cmd_mrbean},           CMD_NOARG },
    { "simon foster",    {.noArg = cmd_simonfoster},      CMD_NOARG },
    { "richard branson", {.noArg = cmd_richardbranson},  CMD_NOARG },
    { "john wardley",    {.noArg = cmd_johnwardley},      CMD_NOARG },
    { "john mace",       {.noArg = cmd_johnmace},         CMD_NOARG },
    { "melanie warn",    {.noArg = cmd_melaniewarn},      CMD_NOARG },
    { "damon hill",      {.noArg = cmd_damonhill},        CMD_NOARG },
    { "michael schumacher", {.noArg = cmd_michaelschumacher}, CMD_NOARG },
    { "tony day",        {.noArg = cmd_tonyday},          CMD_NOARG },
    { "katie brayshaw",  {.noArg = cmd_katiebrayshaw},    CMD_NOARG },
    { "sudo",            {.noArg = cmd_sudo},             CMD_NOARG },
    { "admin",           {.noArg = cmd_sudo},             CMD_NOARG },
    { "git",             {.withStr = cmd_git},            CMD_STR },
    { "restart",         {.noArg = cmd_restart},          CMD_NOARG },
    { "quit",            {.noArg = cmd_quit},             CMD_NOARG },
    { "exit",            {.noArg = cmd_quit},             CMD_NOARG },
    { "cd",              {.noArg = cmd_cd},               CMD_NOARG },
    { "./",              {.noArg = cmd_cd},               CMD_NOARG },
    { "kill",            {.noArg = cmd_kill},             CMD_NOARG },
    { "suicide",         {.noArg = cmd_kill},             CMD_NOARG },
    { "die",             {.noArg = cmd_kill},             CMD_NOARG },
    { "justinbailey",    {.noArg = cmd_justinbailey},     CMD_NOARG },
    { "woodstock",       {.noArg = cmd_woodstock},        CMD_NOARG },
    { "quarry",          {.noArg = cmd_quarry},           CMD_NOARG },
    { "zelda",           {.noArg = cmd_zelda},            CMD_NOARG },
    { "allyourbasearebelongtous", {.noArg = cmd_allyourbase}, CMD_NOARG },
    { "all your base",   {.noArg = cmd_allyourbase},      CMD_NOARG },
    { "i am iron man",   {.noArg = cmd_iamironman},       CMD_NOARG },
    { "i am amazing",    {.noArg = cmd_iamironman},       CMD_NOARG },
    { "i am cool",       {.noArg = cmd_iamironman},       CMD_NOARG },
    { "i am best",       {.noArg = cmd_iamironman},       CMD_NOARG },
    { "idkfa",           {.noArg = cmd_idkfa},            CMD_NOARG },
    { "impulse 9",       {.noArg = cmd_idkfa},            CMD_NOARG },
    { "undo",            {.noArg = cmd_undo},             CMD_NOARG },
    { "shake",           {.noArg = cmd_shake},            CMD_NOARG },
    { "tired",           {.noArg = cmd_staminup},         CMD_NOARG },
    { "staminup",        {.noArg = cmd_staminup},         CMD_NOARG },
    { "grok",            {.noArg = cmd_ai},               CMD_NOARG },
    { "chatgpt",         {.noArg = cmd_ai},               CMD_NOARG },
    { "claude",          {.noArg = cmd_ai},               CMD_NOARG },
    { "gemini",          {.noArg = cmd_ai},               CMD_NOARG },
    { "shodan",          {.noArg = cmd_aireal},           CMD_NOARG },
    { "savegeometry",    {.noArg = cmd_savegeometry},     CMD_NOARG },
    { NULL, {.raw = NULL}, CMD_NOARG } // sizeof helper
};

void ProcessConsoleCommand(const char* command) {
    if (command == NULL || strlen(command) == 0) { ToggleConsole(); return; }

    char ts[TEXT_BUFFER_SIZE];
    strncpy(ts, command, sizeof(ts)-1);
    ts[sizeof(ts)-1] = '\0';
    const char* command_trimmed = ts;
    while (*command_trimmed && isspace((unsigned char)*command_trimmed)) command_trimmed++;
    const char* space = command_trimmed;
    while (*space && !isspace((unsigned char)*space)) space++;
    const char* arg_start = space;
    while (*arg_start && isspace((unsigned char)*arg_start)) arg_start++;
    AddToHistory(command);
    bool commandProcessed = false;
    for (const ConsoleCommand* cmd = g_ConsoleCommands; cmd->name; ++cmd) {
        if (CommandMatch(command_trimmed, cmd->name)) {
            if (cmd->type == CMD_NOARG) {      cmd->func.noArg();                              commandProcessed = true;
            } else if (cmd->type == CMD_STR) { cmd->func.withStr(*arg_start ? arg_start : ""); commandProcessed = true;
            } else { // CMD_INT
                if (!*arg_start) {
                    CenterStatusPrint("Missing argument, usage: %s <number>", cmd->name);
                } else {
                    cmd->func.withInt(atoi(arg_start));                                        commandProcessed = true;
                }
            }
        }
    }

    if (!commandProcessed) CenterStatusPrint("%s%s", stringTable[1014], command_trimmed); // "Unknown command or function: "
    consoleEntryText[0] = '\0';
    currentEntryLength = 0;
    historyPos = numHistory; // Position beyond newest for empty
    ToggleConsole();
}

void ConsoleEmulator(int32_t keycode) {
    if (keycode == GLFW_KEY_UP) RecallHistory(1);
    else if (keycode == GLFW_KEY_DOWN) RecallHistory(-1);
    
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
    } else if (keycode == GLFW_KEY_MINUS || keycode == GLFW_KEY_KP_SUBTRACT) {
        if (currentEntryLength < (TEXT_BUFFER_SIZE - 1)) {
            char c = (keyStates[GLFW_KEY_LEFT_SHIFT].down || keyStates[GLFW_KEY_RIGHT_SHIFT].down) ? '_' : '-';
            consoleEntryText[currentEntryLength] = c;
            consoleEntryText[currentEntryLength + 1] = '\0';
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
