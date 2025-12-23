// console.c - Console Emulator
#include <ctype.h> // For tolower
#include <stdlib.h> // For atoi
#include <stdio.h> // For snprintf
#include "voxen.h"
#include "event.h"
#include "entity.h"
#include "todo.h"

#define MAX_HISTORY 7
static int32_t currentEntryLength = 0;
char consoleEntryText[TEXT_BUFFER_SIZE] = "Enter a command...";
char history[MAX_HISTORY][TEXT_BUFFER_SIZE] = {0};
static int numHistory = 0;
static int historyPos = 0;

void ToggleConsole(void) {
    static bool inventoryModeWasActivePriorToConsole = false;
    if (!voxen_Cheats.consoleActive) inventoryModeWasActivePriorToConsole = voxen_globalContext.inventoryMode;
    voxen_Cheats.consoleActive = !voxen_Cheats.consoleActive; // Tilde
    if (voxen_Cheats.consoleActive) voxen_globalContext.inventoryMode = true;
    else if (!inventoryModeWasActivePriorToConsole && voxen_globalContext.inventoryMode) {
        voxen_globalContext.inventoryMode = false;
        cursorPosition_x = (int32_t)((float)voxen_Settings.ScreenWidth * 0.5f);
        cursorPosition_y = (int32_t)((float)voxen_Settings.ScreenHeight * 0.5f);
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

__attribute__((pure)) static int CommandMatch(const char* input, const char* cmd) {
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
    voxen_Cheats.noclip = !voxen_Cheats.noclip;
    if (voxen_Cheats.noclip) CenterStatusPrint("noclip: %s", voxen_Text.stringTable[1000]); // "ACTIVATED"
    else CenterStatusPrint("noclip: %s", voxen_Text.stringTable[717]); // "DISABLED"
}

static void cmd_edit(void) {
    voxen_Cheats.editMode = !voxen_Cheats.editMode;
    if (voxen_Cheats.editMode) {
        CenterStatusPrint("edit mode: %s", voxen_Text.stringTable[998]); // "Edit Mode activated! The current level can be shaped to your heart's content!"
        voxen_Cheats.noclip = true;
        voxen_Cheats.notarget = true;
    } else {
        CenterStatusPrint("%s", voxen_Text.stringTable[999]); // "Edit Mode deactivated, normal play"
        voxen_Cheats.noclip = false;
        voxen_Cheats.notarget = false;
    }
}

static void cmd_savegeometry(void) {
    if (!voxen_Cheats.editMode) { CenterStatusPrint("savegeometry only works in edit mode!"); return; }

    char filename[64];
    snprintf(filename, sizeof(filename), "./Data/level%u.txt", voxen_globalContext.currentLevel);
    FILE* f = fopen(filename, "w");
    if (!f) { CenterStatusPrint("Failed to open %s for writing!", filename); return; }

    DualLog("Saving current level geometry to %s...", filename);
    // We'll save only geometry chunks for now (constIndex 0–306 except 112, and 760)
    int saved = 0;
    for (uint16_t i = START_INDEX_LEVEL_INSTANCES; i < loadedInstances; ++i) {
        Entity* ent = &instances[i];
        uint16_t idx = ent->index;
        if (!ConstIndexIsGeometry(idx)) continue;

        fprintf(f, "constIndex:%u|geometry instance (%u)|", idx, i - START_INDEX_LEVEL_INSTANCES + 1);
        fprintf(f, "localPosition.x:%08.5f|", (double)ent->position.x);
        fprintf(f, "localPosition.y:%08.5f|", (double)ent->position.y);
        fprintf(f, "localPosition.z:%08.5f|", (double)ent->position.z);
        fprintf(f, "localRotation.x:%08.5f|", (double)ent->rotation.x);
        fprintf(f, "localRotation.y:%08.5f|", (double)ent->rotation.y);
        fprintf(f, "localRotation.z:%08.5f|", (double)ent->rotation.z);
        fprintf(f, "localRotation.w:%08.5f|", (double)ent->rotation.w);
        fprintf(f, "localScale.x:%08.5f|",    (double)ent->scale.x);
        fprintf(f, "localScale.y:%08.5f|",    (double)ent->scale.y);
        fprintf(f, "localScale.z:%08.5f|",    (double)ent->scale.z);
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
        CenterStatusPrint("%s", voxen_Text.stringTable[1001]); // "Gamma grove already jettisoned! Those poor arrogant people."
        return -2; // Special code: do not load
    }

    int level = atoi(clean);
    if (level >= 0 && level < voxen_globalContext.numLevels) return level;
    return -1; // Invalid
}

static void cmd_loadlevel(const char* arg) {
    if (voxen_globalContext.menuActive) { CenterStatusPrint("%s", voxen_Text.stringTable[1015]); return; } // "Cannot load levels via cheat while on the main menu!"

    int level = ParseLevelArg(arg);
    if (level >= 0) {
        if (level == -2) return; // Already printed g3 message
        if (level < 0) { CenterStatusPrint("Invalid level argument"); return; }
        
        CenterStatusPrint("Loading level %u", level);
        queuedLevelToLoad = level;
        LoadLevel(level);
    }
}

static void cmd_loadarsenal(const char* arg) { int level = ParseLevelArg(arg); if (level >= 0 && level < voxen_globalContext.numLevels) { EnableCheatArsenal(level); } }

static void cmd_summon(int itemConstIndex) {
    if (!ConstIndexInBounds(itemConstIndex)) {
        SpawnDynamicObject(itemConstIndex, true);
        CenterStatusPrint("Summoned object ID %d", itemConstIndex);
    } else {
        CenterStatusPrint("Invalid object ID: %s", itemConstIndex);
    }
}

static void cmd_notarget(void) { voxen_Cheats.notarget = !voxen_Cheats.notarget; CenterStatusPrint("notarget: %s", voxen_Cheats.notarget ? voxen_Text.stringTable[1000] : voxen_Text.stringTable[717]); }
static void cmd_showfps(void) { voxen_Cheats.showFPS = !voxen_Cheats.showFPS; }
static void cmd_showlocation(void) { voxen_Cheats.showLocation = !voxen_Cheats.showLocation; }
static void cmd_help(void) { CenterStatusPrint("There's no one to save you now Hacker!"); }
static void cmd_nomoney(void) { CenterStatusPrint("Nice try, there's no money here."); }
static void cmd_god(void) { voxen_Cheats.god = !voxen_Cheats.god; CenterStatusPrint("god mode: %s", voxen_Cheats.god ? voxen_Text.stringTable[1000] : voxen_Text.stringTable[717]); }
static void cmd_energy(void) {
    voxen_Cheats.redbull = !voxen_Cheats.redbull; 
    if (voxen_Cheats.redbull) CenterStatusPrint("%s", voxen_Text.stringTable[1006]); // "I feel the power! 0 energy consumption!"
    else CenterStatusPrint("%s", voxen_Text.stringTable[1005]); // Energy usage normal
}

static void cmd_dizzy(void) {
    voxen_Cheats.dizzyLevel++;
    if (voxen_Cheats.dizzyLevel > 4) voxen_Cheats.dizzyLevel = 0;
    SetSkyRotateSpeed();
}

static void cmd_bottomless(void) {
    voxen_Cheats.bottomless = !voxen_Cheats.bottomless;
    if (voxen_Cheats.bottomless) CenterStatusPrint("bottomlessclip! %s", voxen_Text.stringTable[1002]); // "Bring it!"
    else CenterStatusPrint("%s", voxen_Text.stringTable[1003]); // "Hose disconnected from interdimensional wormhole. Normal ammo operation restored."
}

static void cmd_nohud(void) {
    voxen_Cheats.noHUD = !voxen_Cheats.noHUD;
    if (voxen_Cheats.noHUD) CenterStatusPrint("%s", voxen_Text.stringTable[1004]); // "No HUD! Enjoy the cinematic screenshot experience!"
    else CenterStatusPrint("HUD %s", voxen_Text.stringTable[1000]); // "ACTIVATED"
}

static void cmd_iamshodan(void) {
    voxen_Cheats.superoverride = !voxen_Cheats.superoverride;
    if (voxen_Cheats.superoverride) {
        CenterStatusPrint("%s", voxen_Text.stringTable[1010]); // "Full security override enabled!"
    } else {
        CenterStatusPrint("%s", voxen_Text.stringTable[1009]); // "SHODAN has regained control of security from you"
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

static void cmd_justinbailey(void)   { CenterStatusPrint("Well, you don't have a suit already so..."); }
static void cmd_woodstock(void)      { CenterStatusPrint("How much wood could a woodchuck chuck...there's no wood in SPACE!"); }
static void cmd_quarry(void)         { CenterStatusPrint("There's obsidian on levels 6 and 8 if you want to feel decadent, otherwise we are lacking in the stone department."); }
static void cmd_zelda(void)          { CenterStatusPrint("Too late, already been to level 1"); }
static void cmd_allyourbase(void)    { CenterStatusPrint("ERROR: SHODAN has overriden your command, remove SHODAN first."); }
static void cmd_iamironman(void)     { CenterStatusPrint("That's nice dear."); }
static void cmd_idkfa(void)          { CenterStatusPrint("I can only hold 7 weapons!! Nice try dearies!"); }
static void cmd_ai(void)             { CenterStatusPrint("Only AI allowed around here is SHODAN"); }
static void cmd_aireal(void)         { CenterStatusPrint("In my magnificence, I shape clay, crafting new lifeforms..."); }

static void cmd_staminup(void) {
    voxen_Cheats.fatigueCheat = !voxen_Cheats.fatigueCheat;
    if (voxen_Cheats.fatigueCheat) {
        CenterStatusPrint("Stamin-Up! %s", voxen_Text.stringTable[1013]);
    } else {
        CenterStatusPrint("%s", voxen_Text.stringTable[1012]);
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
    { "undo",    {.noArg = cmd_undo}, CMD_NOARG},
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

    if (!commandProcessed) CenterStatusPrint("%s%s", voxen_Text.stringTable[1014], command_trimmed); // "Unknown command or function: "
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
