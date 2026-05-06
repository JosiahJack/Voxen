// console.c - Console Emulator
#define MAX_HISTORY 7
static i32 currentEntryLength=0, numHistory=0, historyPos=0;
char consoleEntryText[TEXT_BUFFER_SIZE],history[MAX_HISTORY][TEXT_BUFFER_SIZE];
ENGINE_TO_MOD void ToggleConsole(void) {
    static bool inventoryModeWasActivePriorToConsole = false;
    if (!Sys_Cheats.consoleActive) inventoryModeWasActivePriorToConsole = Sys_Global.inventoryMode;
    Sys_Cheats.consoleActive = !Sys_Cheats.consoleActive; // Tilde
    if (Sys_Cheats.consoleActive) Sys_Global.inventoryMode = true;
    else if (!inventoryModeWasActivePriorToConsole && Sys_Global.inventoryMode) ForceShootMode();
}

static void AddToHistory(const char* entry) {
    if (GetStringLength(entry) == 0) return;
    if (numHistory > 0 && StringsEqual(entry, history[numHistory - 1])) return;
    
    if (numHistory < MAX_HISTORY) {
        StringCopyInto_A_From_B(history[numHistory], entry, TEXT_BUFFER_SIZE);
        numHistory++;
    } else {
        for (int i = 0; i < MAX_HISTORY - 1; i++) StringCopyInto_A_From_B(history[i], history[i + 1], TEXT_BUFFER_SIZE); // Shift list toward 0
        StringCopyInto_A_From_B(history[MAX_HISTORY - 1], entry, TEXT_BUFFER_SIZE);
    }
}

static void RecallHistory(int direction) { // direction 1 up (older), -1 down (newer)
    if (direction == 1) { // up
        if (historyPos > 0) {
            historyPos--;
            StringCopyInto_A_From_B(consoleEntryText, history[historyPos], TEXT_BUFFER_SIZE);
            currentEntryLength = GetStringLength(consoleEntryText);
        }
    } else if (direction == -1) { // down
        if (historyPos < numHistory) {
            historyPos++;
            if (historyPos == numHistory) {
                consoleEntryText[0] = '\0';
                currentEntryLength = 0;
            } else {
                StringCopyInto_A_From_B(consoleEntryText, history[historyPos], TEXT_BUFFER_SIZE);
                currentEntryLength = GetStringLength(consoleEntryText);
            }
        }
    }
}

typedef void (*ConsoleCmdFuncNoArg)(void);
typedef void (*ConsoleCmdFuncInt)(int);
typedef void (*ConsoleCmdFuncStr)(const char*);
typedef struct {
    const char* name;
    union { ConsoleCmdFuncNoArg noArg; ConsoleCmdFuncInt withInt; ConsoleCmdFuncStr withStr; void* raw; } func;
    enum { CMD_NOARG,CMD_INT,CMD_STR } type;
} ConsoleCommand;

char CharToLower(const char c);
__attribute__((pure)) static int CommandMatch(const char* input, const char* cmd) {
    while (*cmd && *input) {
        char c1 = CharToLower((unsigned char)*input++);
        char c2 = CharToLower((unsigned char)*cmd++);
        if (c1 == ' ' || c1 == '_') c1 = ' ';
        if (c2 == ' ' || c2 == '_') c2 = ' ';
        if (c1 != c2) return 0;
    }

    return *cmd == '\0' && (*input == '\0' || CharacterIsEmpty((unsigned char)*input) || *input == '_');
}

static void cmd_noclip(void) {
    Sys_Cheats.noclip = !Sys_Cheats.noclip;
    if (Sys_Cheats.noclip) {
        CenterStatusPrint("noclip: %s", Sys_Text.stringTable[1000]); // "ACTIVATED"
        Sys_Global.instances[PLAYER1].velocity = (Vector3){ 0.0f, 0.0f, 0.0f };
    } else CenterStatusPrint("noclip: %s", Sys_Text.stringTable[717]); // "DISABLED"
}

void EnableCheatArsenal(u8 level) { // TODO
    switch(level) {
        default: break;
    }
}

void cmd_kill(void) {
    CenterStatusPrint("%s", Sys_Text.stringTable[1011]); // "Player decides to become a cyborg."  TakeDamage(...) // TODO
}

void cmd_undo(void) {
    if (Sys_Cheats.editMode) {
        // Utils.SafeDestroy(lastSpawnedGO); lastSpawnedGO = NULL;
        CenterStatusPrint("Last spawned object removed");
    } else {
        CenterStatusPrint("Cannot undo when not in Edit Mode");
    }
}

void ScreenShake(float force, double duration) {
    Sys_Global.shakeFinished = Sys_Global.pauseRelativeTime + duration;
    float shakeForce = (force < 0.48f) ? force : 0.48f;
    (void)shakeForce;
    // TODO actually shake
}

void Shake(float force) {
    float forc = (force <= 0.0f) ? 1.0f : force;
    ScreenShake(forc,1.0); // The whole station is a shakin' and a movin'!
}

void cmd_shake(void) { Shake(-1); CenterStatusPrint("SHAKIN LIKE A LEAF!"); }

static void cmd_edit(void) {
    Sys_Cheats.editMode = !Sys_Cheats.editMode;
    if (Sys_Cheats.editMode) {
        CenterStatusPrint("edit mode: %s", Sys_Text.stringTable[998]); // "Edit Mode activated! The current level can be shaped to your heart's content!"
        Sys_Cheats.noclip = true;
        Sys_Cheats.notarget = true;
    } else {
        CenterStatusPrint("%s", Sys_Text.stringTable[999]); // "Edit Mode deactivated, normal play"
        Sys_Cheats.noclip = false;
        Sys_Cheats.notarget = false;
    }
}

i32 StringToInt(const char *str);
static int ParseLevelArg(const char* arg) {
    if (!arg || !*arg) return -1;

    char clean[64] = {0};
    int j = 0;
    for (int i = 0; arg[i] && j < 60; i++) {
        if (arg[i] != ' ' && arg[i] != '_') clean[j++] = CharToLower((unsigned char)arg[i]);
    }
    
    clean[j] = '\0';

    // Special cases
    if (StringsEqual(clean, "r")      || StringFindSubstring(clean, "reactor")) return 0;
    if (StringFindSubstring(clean, "g1") || StringFindSubstring(clean, "10")) return 10;
    if (StringFindSubstring(clean, "g2") || StringFindSubstring(clean, "11")) return 11;
    if (StringFindSubstring(clean, "g4") || StringFindSubstring(clean, "12")) return 12;
    if (StringFindSubstring(clean, "g3")) {
        CenterStatusPrint("%s", Sys_Text.stringTable[1001]); // "Gamma grove already jettisoned! Those poor arrogant people."
        return -2; // Special code: do not load
    }

    int level = StringToInt(clean);
    if (level >= 0 && level < Sys_Global.numLevels) return level;
    return -1; // Invalid
}

static Vector3 ressurectionLocations[10] = {
    {-27.386f, -55.488f, 26.5941f}, // 0/R
    {40.903f, -42.372f, -30.78f}, // 1
    {30.67407f, -25.832f, 10.21412f}, // 2
    {38.26813f, -15.498f, 20.37825f}, // 3
    {-19.48f, -7.928f, 22.954f}, // 4
    {-24.358f, 12.5956f, 31.8497f}, // 5
    {-22.3568f, 33.7845f, -30.728f}, // 6
    {2.228084f, 50.95243f, 7.532025f}, // 7
    {10.068f, 58.897f, 13.973f}, // 8
    {2.303f, 106.77f, -38.554f} // 9
};

static Vector3 cyberSpaceEntryLocations[8] = {
    {210.6834f,2.812f,-24.378f}, // 0
    {195.42f,-13.44f,33.28f}, // 1
    {157.1608f,-15.53f,47.331f}, // 2a, if cyberport localPosition.x < -26.0f
    {256.0416f,-0.716f,62.48789f}, // 2b level 2 secondary cyberport position
    {126.43f,29.56733f,34.24f}, // 5
    {177.612f,3.29494f,108.7725f}, // 6
    {244.735f,41.99257f,-19.695f}, // 8
    {185.161f,84.502f,-46.04246f}, // 9
};

extern u8 queuedLevelToLoad;
static void cmd_loadlevel(const char* arg) {
    if (Sys_Global.menuActive) { CenterStatusPrint("%s", Sys_Text.stringTable[1015]); return; } // "Cannot load levels via cheat while on the main menu!"

    int level = ParseLevelArg(arg);
    if (level >= 0) {
        if (level == -2) return; // Already printed g3 message
        if (level < 0 || level > 12) { CenterStatusPrint("cmd_loadlevel invalid level argument %u",level); return; }
        
        CenterStatusPrint("Loading level %u", level);
        queuedLevelToLoad = level;
        LoadLevel(level);
        if (level > 9) level = 6;
        Sys_Global.instances[PLAYER1].position = ressurectionLocations[level];
    }
}

static void cmd_loadarsenal(const char* arg) { int level = ParseLevelArg(arg); if (level >= 0 && level < Sys_Global.numLevels) { EnableCheatArsenal(level); } }

static void cmd_summon(int itemConstIndex) {
    if (!ConstIndexInBounds(itemConstIndex)) {
        SpawnDynamicObject(itemConstIndex, true);
        CenterStatusPrint("Summoned object ID %d", itemConstIndex);
    } else {
        CenterStatusPrint("Invalid object ID: %s", itemConstIndex);
    }
}

static void cmd_notarget(void) { Sys_Cheats.notarget = !Sys_Cheats.notarget; CenterStatusPrint("notarget: %s", Sys_Cheats.notarget ? Sys_Text.stringTable[1000] : Sys_Text.stringTable[717]); }
static void cmd_showfps(void) { Sys_Cheats.showFPS = !Sys_Cheats.showFPS; }
static void cmd_showlocation(void) { Sys_Cheats.showLocation = !Sys_Cheats.showLocation; }
static void cmd_help(void) { CenterStatusPrint("There's no one to save you now Hacker!"); }
static void cmd_nomoney(void) { CenterStatusPrint("Nice try, there's no money here."); }
static void cmd_god(void) { Sys_Cheats.god = !Sys_Cheats.god; CenterStatusPrint("god mode: %s", Sys_Cheats.god ? Sys_Text.stringTable[1000] : Sys_Text.stringTable[717]); }
static void cmd_energy(void) {
    Sys_Cheats.redbull = !Sys_Cheats.redbull; 
    if (Sys_Cheats.redbull) CenterStatusPrint("%s", Sys_Text.stringTable[1006]); // "I feel the power! 0 energy consumption!"
    else CenterStatusPrint("%s", Sys_Text.stringTable[1005]); // Energy usage normal
}

void SetSkyRotateSpeed(void);
static void cmd_dizzy(void) { Sys_Cheats.dizzyLevel = (Sys_Cheats.dizzyLevel >= 3) ? 0 : Sys_Cheats.dizzyLevel + 1; SetSkyRotateSpeed(); }
static void cmd_bottomless(void) {
    Sys_Cheats.bottomless = !Sys_Cheats.bottomless;
    if (Sys_Cheats.bottomless) CenterStatusPrint("bottomlessclip! %s", Sys_Text.stringTable[1002]); // "Bring it!"
    else CenterStatusPrint("%s", Sys_Text.stringTable[1003]); // "Hose disconnected from interdimensional wormhole. Normal ammo operation restored."
}

static void cmd_nohud(void) {
    Sys_Cheats.noHUD = !Sys_Cheats.noHUD;
    if (Sys_Cheats.noHUD) CenterStatusPrint("%s", Sys_Text.stringTable[1004]); // "No HUD! Enjoy the cinematic screenshot experience!"
    else CenterStatusPrint("HUD %s", Sys_Text.stringTable[1000]); // "ACTIVATED"
}

static void cmd_iamshodan(void) {
    Sys_Cheats.superoverride = !Sys_Cheats.superoverride;
    if (Sys_Cheats.superoverride) {
        CenterStatusPrint("%s", Sys_Text.stringTable[1010]); // "Full security override enabled!"
    } else {
        CenterStatusPrint("%s", Sys_Text.stringTable[1009]); // "SHODAN has regained control of security from you"
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
// static void cmd_git(const char* arg) {
//     if (!arg) arg = "";
//     if (StringFindSubstring(arg, "pull") || StringFindSubstring(arg, "fetch")) CenterStatusPrint("remote: Enumerating objects: 24601, done. Failed, could not connect with origin/triop.");
//     else if (StringFindSubstring(arg, "status")) CenterStatusPrint("Your branch is up to date with origin/triop. Working directory clean.");
//     else if (StringFindSubstring(arg, "log"))    CenterStatusPrint("<Merge pull request #451 from SHODAN/NeuralLinkBugfix> 6 months ago...");
//     else if (StringFindSubstring(arg, "reflog")) CenterStatusPrint("dc51440 HEAD0 -> master: commit: Establish neural connection ... ERROR: invalid ID `2-4601`");
//     else if (StringFindSubstring(arg, "merge"))  CenterStatusPrint("Failed, could not connect with origin/triop");
//     else if (StringFindSubstring(arg, "push"))   CenterStatusPrint("Could not find Username for 'triopttp://192.168.1.451'");
//     else if (StringFindSubstring(arg, "clone"))  CenterStatusPrint("Failed, connection blocked by SHODAN. Employee ID invalid.");
//     else if (StringFindSubstring(arg, "branch") || StringFindSubstring(arg,"-b")) { const char *last = StringFindLastChar(arg,' '), *name = last ? last + 1 : "unknown"; CenterStatusPrint("Created new branch %s",name); }
//     else CenterStatusPrint("Branch name not recognized. Contact your TriopBucket representative.");
// } 1.03kb
static void cmd_git(const char* arg) {
    if (!arg) arg = "";
    static const char* cmds[] = {
        "pull", "remote: Enumerating objects: 24601, done. Failed, could not connect with origin/triop.",
        "fetch", "remote: Enumerating objects: 24601, done. Failed, could not connect with origin/triop.",
        "status", "Your branch is up to date with origin/triop. Working directory clean.",
        "log", "<Merge pull request #451 from SHODAN/NeuralLinkBugfix> 6 months ago...",
        "reflog", "dc51440 HEAD0 -> master: commit: Establish neural connection ... ERROR: invalid ID `2-4601`.",
        "merge", "Failed, could not connect with origin/triop.",
        "push", "Could not find Username for 'triopttp://192.168.1.451'.",
        "clone", "Failed, connection blocked by SHODAN. Employee ID invalid."
    };

    for (int i = 0; i < 16; i += 2) {
        if (StringFindSubstring(arg, cmds[i])) { CenterStatusPrint(cmds[i+1]); return; }
    }

    if (StringFindSubstring(arg, "branch") || StringFindSubstring(arg, "-b")) { const char *last = StringFindLastChar(arg, ' '); CenterStatusPrint("Created new branch %s", last ? last + 1 : "unknown"); }
    else CenterStatusPrint("Branch name not recognized. Contact your TriopBucket representative.");
} // 334 b

static void cmd_restart(void)        { CenterStatusPrint("Yeah...better not"); }
static void cmd_quit(void)           { OS_Exit(0); }
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
    Sys_Cheats.fatigueCheat = !Sys_Cheats.fatigueCheat;
    if (Sys_Cheats.fatigueCheat) { CenterStatusPrint("Stamin-Up! %s",Sys_Text.stringTable[1013]); SetModFatigue(0.0f); } else CenterStatusPrint("%s",Sys_Text.stringTable[1012]);
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
    { NULL, {.raw = NULL}, CMD_NOARG } // sizeof helper
};

void ProcessConsoleCommand(const char* command) {
    if (command == NULL || GetStringLength(command) == 0) { ToggleConsole(); return; }

    char ts[TEXT_BUFFER_SIZE];
    StringCopyInto_A_SubstringFrom_B(ts, sizeof(ts)-1, command, TEXT_BUFFER_SIZE);
    ts[sizeof(ts)-1] = '\0';
    const char* command_trimmed = ts;
    while (*command_trimmed && CharacterIsEmpty((unsigned char)*command_trimmed)) command_trimmed++;
    const char* space = command_trimmed;
    while (*space && !CharacterIsEmpty((unsigned char)*space)) space++;
    const char* arg_start = space;
    while (*arg_start && CharacterIsEmpty((unsigned char)*arg_start)) arg_start++;
    AddToHistory(command);
    bool commandProcessed = false;
    for (u16 i = 0; g_ConsoleCommands[i].name != NULL; ++i) {
        const ConsoleCommand* cmd = &g_ConsoleCommands[i];
        if (CommandMatch(command_trimmed, cmd->name)) {
            if (cmd->type == CMD_NOARG) {      cmd->func.noArg();                              commandProcessed = true;
            } else if (cmd->type == CMD_STR && *arg_start) { cmd->func.withStr(*arg_start ? arg_start : ""); commandProcessed = true;
            } else { // CMD_INT
                if (!*arg_start) {
                    CenterStatusPrint("Missing argument, usage: %s <number>", cmd->name);
                } else {
                    cmd->func.withInt(StringToInt(arg_start));                                 commandProcessed = true;
                }
            }
        }
    }

    if (!commandProcessed) CenterStatusPrint("%s%s",Sys_Text.stringTable[1014],command_trimmed); // "Unknown command or function: "
    consoleEntryText[0] = '\0';
    currentEntryLength = 0;
    historyPos = numHistory; // Position beyond newest for empty
    ToggleConsole();
}

void ConsoleEmulator(i32 keycode) {
    if (keycode == GLFW_KEY_UP) { RecallHistory(1); return; }
    else if (keycode == GLFW_KEY_DOWN) { RecallHistory(-1); return; }
    
    if (keycode == GLFW_KEY_U && Sys_Input.keyStates[GLFW_KEY_LEFT_CONTROL].down) {
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
            char c = (Sys_Input.keyStates[GLFW_KEY_LEFT_SHIFT].down || Sys_Input.keyStates[GLFW_KEY_RIGHT_SHIFT].down) ? '_' : '-';
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
