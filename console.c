// console.c - Console Emulator CHEATS!  Same type of tilde activated command entry as Quake or Half-Life or Source.
static i32 currentEntryLength=0, numHistory=0, historyPos=0; char consoleEntryText[T_BUFFER_SIZE],history[7][T_BUFFER_SIZE];
static V3 ressurectionLocations[10] = {{-27.386f,-55.488f,26.5941f},/*0/R*/ {40.903f,-42.372f,-30.78f},/*1*/      {30.67407f,-25.832f,10.21412f},/*2*/ {38.26813f,-15.498f,20.37825f},/*3*/ {-19.48f,-7.928f,22.954f},/*4*/ {-24.358f,12.5956f,31.8497f},/*5*/ {-22.3568f,33.7845f,-30.728f},/*6*/  {2.228084f,50.95243f,7.532025f},/*7*/ {10.068f,58.897f,13.973f},/*8*/ {2.303f,106.77f,-38.554f}/*9*/};
static V3 cyberSpaceEntryLocations[8] = { {210.6834f,2.812f,-24.378f},/*0*/ {195.420f,-13.44000f, 33.2800f},/*1*/ {157.1608f,-15.53f,47.331f},/*2a, if cyberport localPosition.x < -26.0f*/ {256.0416f,-0.716f,62.48789f},/*2b level 2 secondary cyberport position*/ {126.43f,29.56733f,34.24f},/*5*/  {177.612f,  3.29494f,108.7725f},/*6*/ {244.735f,41.99257f,-19.695f},/*8*/                                       {185.161f,84.502f,-46.04246f},/*9*/ };
void ForceShootMode(void) {
    if (Sys_Settings.NoShootMode) return; // We are being like the original now!

    Sys_UI.mouseClickHeldOverGUI = false;
//     CloseFullmap(); // TODO
    World.inventoryMode = false; World.cursorPosition_x = 663; World.cursorPosition_y = 371; IgnoreNextMouseDelta(); // Centered on UI baseline resolution 1366x768
//     if (vmailActive) { World.invP1.DeactivateVMail(); vmailActive = false; } // TODO
}

void ForceInventoryMode(void) { World.inventoryMode = true; World.cursorPosition_x = 663; World.cursorPosition_y = 371; IgnoreNextMouseDelta(); } // Centered on UI baseline resolution 1366x768
void ToggleInventoryMode(void) { if (World.inventoryMode) {ForceShootMode();} else {ForceInventoryMode();} }
void ToggleConsole() { static bool imWasActPrior = false; if (!Cheats.consoleActive) {imWasActPrior = World.inventoryMode;} Cheats.consoleActive = !Cheats.consoleActive; if (Cheats.consoleActive) { World.inventoryMode = true; } else if (!imWasActPrior && World.inventoryMode) {ForceShootMode();} }
static void AddToHistory(const char* entry) {
    if (slen(entry) == 0 || (numHistory > 0 && sEqual(entry,history[numHistory - 1]))) return;
    if (numHistory < 7) { scpy_to_a_from_b(history[numHistory],entry,T_BUFFER_SIZE); numHistory++; }
    else { for (int i = 0; i < 7 - 1; i++) {scpy_to_a_from_b(history[i],history[i + 1],T_BUFFER_SIZE);/*Shift list toward 0*/} scpy_to_a_from_b(history[7 - 1],entry,T_BUFFER_SIZE); }
}

void RecallHistory(int direction) { // direction 1 up (older), -1 down (newer)
    if (direction == 1) { // up
        if (historyPos > 0) { historyPos--; scpy_to_a_from_b(consoleEntryText,history[historyPos],T_BUFFER_SIZE); currentEntryLength = slen(consoleEntryText); }
    } else if (direction == -1) { // down
        if (historyPos < numHistory) {
            historyPos++;
            if (historyPos == numHistory) { consoleEntryText[0] = currentEntryLength = 0; } else { scpy_to_a_from_b(consoleEntryText,history[historyPos],T_BUFFER_SIZE); currentEntryLength = slen(consoleEntryText); }
        }
    }
}

typedef void (*ConsoleCmdFuncNoArg)(); typedef void (*ConsoleCmdFuncInt)(int); typedef void (*ConsoleCmdFuncStr)(const char*);
typedef struct { const char* name; union {ConsoleCmdFuncNoArg noArg; ConsoleCmdFuncInt withInt; ConsoleCmdFuncStr withStr; void* raw;} func; enum {CMD_NOARG,CMD_INT,CMD_STR}type;} ConsoleCommand;
int CommandMatch(const char* in, const char* cmd) { while (*cmd && *in) { char c1 = c2Lower((u8)*in++); char c2 = c2Lower((u8)*cmd++); if (c1 == ' ' || c1 == '_') {c1 = ' ';} if (c2 == ' ' || c2 == '_') {c2 = ' ';} if (c1 != c2) {return 0;} } return *cmd == '\0' && (*in == '\0' || cEmpty((u8)*in) || *in == '_'); }
void cmd_noclip() { Cheats.noclip = !Cheats.noclip; if (Cheats.noclip) { World.velocity[PLAYER1] = (V3){ 0.0f, 0.0f, 0.0f }; CenterStatusPrint("noclip: %s", Sys_Text.stringTable[1000]); /*"ACTIVATED"*/} else {CenterStatusPrint("noclip: %s", Sys_Text.stringTable[717]); /*"DISABLED"*/} }
void cmd_showphys() { Cheats.showPhys = !Cheats.showPhys; if (Cheats.showPhys) {CenterStatusPrint("showPhys: %s", Sys_Text.stringTable[1000]); /*"ACTIVATED"*/} else {CenterStatusPrint("showPhys: %s", Sys_Text.stringTable[717]); /*"DISABLED"*/} }
void EnableCheatArsenal(u8 level) { (void)level; } // TODO
void cmd_kill() { World.instances[PLAYER1].health = World.instances[PLAYER1].cyberHealth = 0.0f; CenterStatusPrint("%s", Sys_Text.stringTable[1011]); } // "Player decides to become a cyborg."
void cmd_undo() { if (Cheats.editMode) { CenterStatusPrint("Last spawned object removed"); } else { CenterStatusPrint("Cannot undo when not in Edit Mode"); } } // TODO actually track and despawn last
void ScreenShake(float force, double duration) { World.shakeFinished = World.pauseRelativeTime + duration; float shakeForce = (force < 0.48f) ? force : 0.48f; (void)shakeForce; } // TODO actually shake
void Shake(float force) { float forc = (force <= 0.0f) ? 1.0f : force; ScreenShake(forc,1.0); }// The whole station is a shakin' and a movin'!
void cmd_shake() { Shake(-1); CenterStatusPrint("SHAKIN LIKE A LEAF!"); }
void cmd_edit() { Cheats.editMode = !Cheats.editMode; if (Cheats.editMode) { Cheats.noclip=Cheats.notarget=true; CenterStatusPrint("edit mode: %s","Edit Mode activated!"); } else { Cheats.noclip=Cheats.notarget=false; CenterStatusPrint("%s","Edit Mode deactivated"); } }
int ParseLevelArg(const char* arg) {
    if (!arg || !*arg) return -1;
    char clean[64] = {0}; int j = 0;
    for (int i = 0; arg[i] && j < 60; i++) { if (arg[i] != ' ' && arg[i] != '_') clean[j++] = c2Lower((u8)arg[i]); }   clean[j] = '\0';
    if (sEqual(clean, "r")      || sFindSub(clean, "reactor")) return 0;
    if (sFindSub(clean, "g1") || sFindSub(clean, "10")) return 10;
    if (sFindSub(clean, "g2") || sFindSub(clean, "11")) return 11;
    if (sFindSub(clean, "g4") || sFindSub(clean, "12")) return 12;
    if (sFindSub(clean, "g3")) { CenterStatusPrint("%s", Sys_Text.stringTable[1001]); return -2; }// "Gamma grove already jettisoned! Those poor arrogant people."
    int level = s2i32(clean); if (level >= 0 && level < World.numLevels) return level;
    return -1; // Invalid
}

u8 queuedLevelToLoad = 255u;
void LoadLevel(u8 curlevel);
static void cmd_loadlevel(const char* arg) {
    if (World.menuActive) { CenterStatusPrint("%s", Sys_Text.stringTable[1015]); return; } // "Cannot load levels via cheat while on the main menu!"
    int level = ParseLevelArg(arg); if (level == -2) return; // Already printed g3 message
    if (level < 0 || level > 12) { CenterStatusPrint("cmd_loadlevel invalid level argument %u",level); return; }
    CenterStatusPrint("Loading level %u",level); queuedLevelToLoad = level; LoadLevel(level); SetPosition(PLAYER1,ressurectionLocations[level > 9 ? 6 : level],true); (void)cyberSpaceEntryLocations;
}

static void cmd_loadarsenal(const char* arg) { int level = ParseLevelArg(arg); if (level >= 0 && level < World.numLevels) { EnableCheatArsenal(level); } }
u16 SpawnDynamicObject(int val, bool cheat);
static void cmd_summon(int itemConstIndex) { if (!IdxInBounds(itemConstIndex)) { SpawnDynamicObject(itemConstIndex, true); CenterStatusPrint("Summoned object ID %d", itemConstIndex); } else { CenterStatusPrint("Invalid object ID: %s", itemConstIndex); } }
static void cmd_notarget() { Cheats.notarget = !Cheats.notarget; CenterStatusPrint("notarget: %s", Cheats.notarget ? Sys_Text.stringTable[1000] : Sys_Text.stringTable[717]); }
static void cmd_showfps() { Cheats.showFPS = !Cheats.showFPS; }
static void cmd_showlocation() { Cheats.showLocation = !Cheats.showLocation; }
static void cmd_help() { CenterStatusPrint("There's no one to save you now Hacker!"); }
static void cmd_nomoney() { CenterStatusPrint("Nice try, there's no money here."); }
static void cmd_god() { Cheats.god = !Cheats.god; CenterStatusPrint("god mode: %s", Cheats.god ? Sys_Text.stringTable[1000] : Sys_Text.stringTable[717]); }
static void cmd_energy() { Cheats.redbull = !Cheats.redbull; if (Cheats.redbull) {CenterStatusPrint("%s", Sys_Text.stringTable[1006]);/*"I feel the power! 0 energy consumption!"*/} else {CenterStatusPrint("%s", Sys_Text.stringTable[1005]);/*Energy usage normal*/} }
static void SetSkyRotateSpeed() { static const float skyRotateSpeeds[] = { 0.05f, 1.0f, 2.5f, 3.75f, 6.25f }; glUseProgram(imageBlitSP); glUniform1f(30,skyRotateSpeeds[Cheats.dizzyLevel]); }
static void cmd_dizzy() { Cheats.dizzyLevel = (Cheats.dizzyLevel >= 3) ? 0 : Cheats.dizzyLevel + 1; SetSkyRotateSpeed(); }
static void cmd_bottomless() { Cheats.bottomless = !Cheats.bottomless; if (Cheats.bottomless) {CenterStatusPrint("bottomlessclip! %s",Sys_Text.stringTable[1002]);/*"Bring it!"*/} else {CenterStatusPrint("%s",Sys_Text.stringTable[1003]);/*"Hose disconnected from interdimensional wormhole. Normal ammo operation restored."*/} }
static void cmd_nohud() { Cheats.noHUD = !Cheats.noHUD; if (Cheats.noHUD) {CenterStatusPrint("%s",Sys_Text.stringTable[1004]);/*"No HUD! Enjoy the cinematic screenshot experience!"*/} else { CenterStatusPrint("HUD %s",Sys_Text.stringTable[1000]);/*"ACTIVATED"*/} }
static void cmd_iamshodan() { Cheats.superoverride = !Cheats.superoverride; if (Cheats.superoverride) {CenterStatusPrint("%s",Sys_Text.stringTable[1010]);/*"Full security override enabled!"*/ } else {CenterStatusPrint("%s",Sys_Text.stringTable[1009]);/*"SHODAN has regained control of security from you"*/} }
static void cmd_staminup() { Cheats.fatigueCheat = !Cheats.fatigueCheat; if (Cheats.fatigueCheat) { CenterStatusPrint("Stamin-Up! %s",Sys_Text.stringTable[1013]); World.invP1.fatigue=0.0f; } else {CenterStatusPrint("%s",Sys_Text.stringTable[1012]); } }
static void cmd_mrbean()  { CenterStatusPrint("Nice try, there are no go carts to slow down here"); } static void cmd_simonfoster()   { CenterStatusPrint("Nice try, nothing to paint here"); } static void cmd_richardbranson() { CenterStatusPrint("Nice try, there's no money here. You do realize this isn't Rollercoaster Tycoon right?"); } static void cmd_johnwardley()       { CenterStatusPrint("WOW!"); }
static void cmd_johnmace(){ CenterStatusPrint("Nice try, there's nothing to pay double for here"); }  static void cmd_melaniewarn()   { CenterStatusPrint("I feel happy!!!"); }                 static void cmd_damonhill()      { CenterStatusPrint("Nice try, there are no go carts to speed up here"); }                                       static void cmd_michaelschumacher() { CenterStatusPrint("Nice try, there are no go carts to give ludicrous speed here"); }
static void cmd_tonyday() { CenterStatusPrint("Ok, now I want a hamburger"); }                        static void cmd_katiebrayshaw() { CenterStatusPrint("Hi there! Hello! Hey! Howdy!"); }
static void cmd_sudo()    { CenterStatusPrint("Super user access granted...ERROR: access restricted by SHODAN!"); }
static void cmd_git(const char* arg) {
    if (!arg) arg = "";
    static const char* cmds[] = {"pull",  "remote: Enumerating objects: 24601, done.\nFailed, could not connect with origin/triop.", "fetch", "remote: Enumerating objects: 24601, done.\nFailed, could not connect with origin/triop.",      "status","Your branch is up to date with origin/triop.\nWorking directory clean.",
                                 "log",   "<Merge pull request #451 from SHODAN/NeuralLinkBugfix> 6 months ago...",                  "reflog","dc51440 HEAD0 -> master: commit: Establish neural connection ... ERROR: invalid ID `2-4601`.", "merge", "Failed, could not connect with origin/triop.",
                                 "push",  "Could not find Username for 'triopttp://192.168.1.451'.",                                 "clone", "Failed, connection blocked by SHODAN. Employee ID invalid." };
    for (int i = 0; i < 16; i += 2) { if (sFindSub(arg, cmds[i])) { CenterStatusPrint(cmds[i+1]); return; } }
    if (sFindSub(arg, "branch") || sFindSub(arg, "-b")) { const char *last = StringFindLastChar(arg, ' '); CenterStatusPrint("Created new branch %s", last ? last + 1 : "unknown"); }
    else CenterStatusPrint("Branch name not recognized. Contact your TriopBucket representative.");
}

static void cmd_restart()     { CenterStatusPrint("Yeah...better not"); }                             static void cmd_cd()          { CenterStatusPrint("Attempting to access directory... already at root"); }
static void cmd_justinbailey(){ CenterStatusPrint("Well, you don't have a suit already so..."); }     static void cmd_woodstock()   { CenterStatusPrint("How much wood could a woodchuck chuck...there's no wood in SPACE!"); }
static void cmd_zelda()       { CenterStatusPrint("Too late, already been to level 1"); }             static void cmd_quarry()      { CenterStatusPrint("There's obsidian on levels 6 and 8 if you want to feel decadent,\notherwise we are lacking in the stone department."); }
static void cmd_iamironman(){ CenterStatusPrint("That's nice dear."); }                               static void cmd_allyourbase() { CenterStatusPrint("ERROR: SHODAN has overriden your command, remove SHODAN first."); }
static void cmd_idkfa()       { CenterStatusPrint("I can only hold 7 weapons!! Nice try dearies!"); } static void cmd_ai()          { CenterStatusPrint("Only AI allowed around here is SHODAN"); }
static void cmd_quit()      { OS_Exit(0); }                                                           static void cmd_aireal()      { CenterStatusPrint("In my magnificence, I shape clay, crafting new lifeforms..."); }
static const ConsoleCommand consoleCmds[] = {
    {"noclip",         {.noArg=cmd_noclip},        CMD_NOARG},{"idclip",          {.noArg=cmd_noclip},CMD_NOARG},         {"no clip",     {.noArg = cmd_noclip},CMD_NOARG},  {"showphys",      {.noArg = cmd_showphys},CMD_NOARG},  { "god",           {.noArg=cmd_god}, CMD_NOARG},       {"overwhelming",            {.noArg=cmd_god}, CMD_NOARG},
    {"whosyourdaddy",  {.noArg = cmd_god},         CMD_NOARG},{"iddqd",           {.noArg=cmd_god}, CMD_NOARG},           {"notarget",    {.noArg=cmd_notarget},CMD_NOARG},  {"no target",     {.noArg = cmd_notarget},CMD_NOARG},  {"editmode",       {.noArg=cmd_edit},CMD_NOARG},       {"edit",                    {.noArg=cmd_edit},CMD_NOARG},
    {"edit mode",      {.noArg = cmd_edit},        CMD_NOARG},{"editor",          {.noArg=cmd_edit},CMD_NOARG},           {"undo",        {.noArg=cmd_undo},    CMD_NOARG},  {"showfps",       {.noArg = cmd_showfps}, CMD_NOARG},  {"show fps",       {.noArg=cmd_showfps},CMD_NOARG},    {"showlocation",            {.noArg=cmd_showlocation},CMD_NOARG},
    {"show location",  {.noArg = cmd_showlocation},CMD_NOARG},{"nohud",           {.noArg=cmd_nohud},CMD_NOARG},          {"no hud",      {.noArg=cmd_nohud},   CMD_NOARG},  {"bottomlessclip",{.noArg = cmd_bottomless},CMD_NOARG},{"bottomless clip",{.noArg=cmd_bottomless},CMD_NOARG}, {"load",                    {.withStr=cmd_loadlevel},CMD_STR},
    {"loadarsenal",    {.withStr = cmd_loadarsenal}, CMD_STR},{"load arsenal",    {.withStr = cmd_loadarsenal},CMD_STR},  {"summon_obj",  {.withInt = cmd_summon},CMD_INT},  {"summonobj",     {.withInt = cmd_summon},CMD_INT},    {"motherlode",     {.noArg=cmd_nomoney},   CMD_NOARG}, {"rosebud",                 {.noArg=cmd_nomoney},CMD_NOARG},
    {"kaching",        {.noArg=cmd_nomoney},       CMD_NOARG},{"money",           {.noArg=cmd_nomoney},CMD_NOARG},        {"dizzy",       {.noArg=cmd_dizzy},   CMD_NOARG},  {"help",          {.noArg=cmd_help},        CMD_NOARG},{"ifeelthepower",  {.noArg = cmd_energy},  CMD_NOARG}, {"power",                   {.noArg=cmd_energy}, CMD_NOARG},
    {"energy",         {.noArg=cmd_energy},        CMD_NOARG},{"i feel the power",{.noArg = cmd_energy},CMD_NOARG},       {"i am shodan", {.noArg=cmd_iamshodan},CMD_NOARG}, {"iamshodan",     {.noArg=cmd_iamshodan},   CMD_NOARG},{"mr. bean",       {.noArg = cmd_mrbean},  CMD_NOARG}, {"simon foster",            {.noArg=cmd_simonfoster},CMD_NOARG},
    {"richard branson",{.noArg=cmd_richardbranson},CMD_NOARG},{"john wardley",    {.noArg = cmd_johnwardley},CMD_NOARG},  {"john mace",   {.noArg=cmd_johnmace}, CMD_NOARG}, {"melanie warn",  {.noArg=cmd_melaniewarn}, CMD_NOARG},{"damon hill",     {.noArg = cmd_damonhill},CMD_NOARG},{"michael schumacher",      {.noArg=cmd_michaelschumacher},CMD_NOARG},
    {"tony day",       {.noArg=cmd_tonyday},       CMD_NOARG},{"katie brayshaw",  {.noArg = cmd_katiebrayshaw},CMD_NOARG},{"sudo",        {.noArg=cmd_sudo},     CMD_NOARG}, {"admin",         {.noArg=cmd_sudo},        CMD_NOARG},{"git",            {.withStr=cmd_git},CMD_STR},        {"restart",                 {.noArg=cmd_restart},CMD_NOARG},
    {"quit",           {.noArg=cmd_quit},          CMD_NOARG},{"exit",            {.noArg = cmd_quit},         CMD_NOARG},{"cd",          {.noArg=cmd_cd},        CMD_NOARG},{"./",            {.noArg=cmd_cd},          CMD_NOARG},{"kill",           {.noArg = cmd_kill},     CMD_NOARG},{"suicide",                 {.noArg=cmd_kill},CMD_NOARG},
    {"die",            {.noArg=cmd_kill},          CMD_NOARG},{"justinbailey",    {.noArg = cmd_justinbailey}, CMD_NOARG},{"woodstock",   {.noArg=cmd_woodstock}, CMD_NOARG},{"quarry",        {.noArg=cmd_quarry},      CMD_NOARG},{"zelda",          {.noArg = cmd_zelda},    CMD_NOARG},{"allyourbasearebelongtous",{.noArg=cmd_allyourbase},CMD_NOARG},
    {"all your base",  {.noArg=cmd_allyourbase},   CMD_NOARG},{"i am iron man",   {.noArg = cmd_iamironman},   CMD_NOARG},{"i am amazing",{.noArg=cmd_iamironman},CMD_NOARG},{"i am cool",     {.noArg=cmd_iamironman},  CMD_NOARG},{"i am best",      {.noArg =cmd_iamironman},CMD_NOARG},{"idkfa",                   {.noArg=cmd_idkfa},      CMD_NOARG},
    {"impulse 9",      {.noArg=cmd_idkfa},         CMD_NOARG},{"undo",            {.noArg = cmd_undo},         CMD_NOARG},{"shake",       {.noArg=cmd_shake},     CMD_NOARG},{"tired",         {.noArg=cmd_staminup},    CMD_NOARG},{"staminup",       {.noArg = cmd_staminup}, CMD_NOARG},{"grok",                    {.noArg=cmd_ai},         CMD_NOARG},
    {"chatgpt",        {.noArg=cmd_ai},            CMD_NOARG},{"claude",          {.noArg = cmd_ai},           CMD_NOARG},{"gemini",      {.noArg=cmd_ai},        CMD_NOARG},{"shodan",        {.noArg=cmd_aireal},      CMD_NOARG},{NULL,{.raw = NULL},CMD_NOARG}/*sizeof helper*/ };
void ProcessConsoleCommand(const char* c) {
    if (c == NULL || slen(c) == 0) { ToggleConsole(); return; }
    char ts[T_BUFFER_SIZE]; sCpy2aSubFromb(ts,sizeof(ts)-1,c,T_BUFFER_SIZE); ts[sizeof(ts)-1] = '\0';
    const char* ct=ts; while(*ct && cEmpty((u8)*ct)){ct++;} const char* space=ct; while(*space && !cEmpty((u8)*space)){space++;} const char* arg_start=space; while(*arg_start && cEmpty((u8)*arg_start)){arg_start++;} AddToHistory(c); bool commandProcessed = false;
    for (u16 i=0;consoleCmds[i].name!=NULL;++i) {
        const ConsoleCommand* cmd = &consoleCmds[i];
        if (CommandMatch(ct,cmd->name)) {
            if (cmd->type == CMD_NOARG) {cmd->func.noArg(); commandProcessed = true; } else if (cmd->type == CMD_STR && *arg_start) { cmd->func.withStr(*arg_start ? arg_start : ""); commandProcessed = true;
            } else { if(!*arg_start){CenterStatusPrint("Missing argument, usage: %s <number>",cmd->name);}else{cmd->func.withInt(s2i32(arg_start)); commandProcessed=true;} }
        }
    }
    if (!commandProcessed){CenterStatusPrint("%s%s",Sys_Text.stringTable[1014],ct);} /*"Unknown command or function: "*/ consoleEntryText[0] = currentEntryLength = 0; historyPos = numHistory; /*Position beyond newest for empt*/ ToggleConsole();
}

void ConsoleEmulator(i32 keycode) {
    if (keycode == KEY_UP || keycode == KEY_DOWN) { RecallHistory(keycode == KEY_UP ? 1 : -1); return; }
    if (keycode == KEY_U && Sys_Input.keyStates[KEY_LEFT_CONTROL].down) { consoleEntryText[0]='\0'; currentEntryLength=0; return; } // Clear the input
    if (keycode >= KEY_A && keycode <= KEY_Z) { // Handle alphabet keys
        if (currentEntryLength < (T_BUFFER_SIZE - 1)) { // Ensure we don't overflow the buffer
            char c = 'a' + (keycode - KEY_A); // Map keycode to lowercase character
            consoleEntryText[currentEntryLength] = c; consoleEntryText[currentEntryLength + 1] = '\0'; currentEntryLength++;
        }
    } else if (keycode >= KEY_1 && keycode <= KEY_9) { // Handle number keys 1-9
        if (currentEntryLength < (T_BUFFER_SIZE - 1)) { char c = '1' + (keycode - KEY_1); /*Map to '1'-'9'*/ consoleEntryText[currentEntryLength] = c; consoleEntryText[currentEntryLength + 1] = '\0'; currentEntryLength++; }
    } else if (keycode == KEY_0) { // Handle '0'
        if (currentEntryLength < (T_BUFFER_SIZE - 1)) { consoleEntryText[currentEntryLength]='0'; consoleEntryText[currentEntryLength + 1]='\0'; currentEntryLength++; }
    } else if (keycode == KEY_MINUS || keycode == KEY_KP_SUBTRACT) {
        if (currentEntryLength < (T_BUFFER_SIZE - 1)) { consoleEntryText[currentEntryLength]=(Sys_Input.keyStates[KEY_LEFT_SHIFT].down || Sys_Input.keyStates[KEY_RIGHT_SHIFT].down) ? '_' : '-'; consoleEntryText[currentEntryLength + 1]='\0'; currentEntryLength++; }
    } else if (keycode == KEY_BACKSPACE && currentEntryLength > 0) { currentEntryLength--; consoleEntryText[currentEntryLength]='\0'; } // Handle backspace
    else if (keycode == KEY_SPACE) { // Handle space
        if (currentEntryLength < (T_BUFFER_SIZE - 1)) { consoleEntryText[currentEntryLength]=' '; consoleEntryText[currentEntryLength + 1]='\0'; currentEntryLength++; }
    } else if (keycode == KEY_ENTER || keycode == KEY_KP_ENTER) { DualLog("Console command: %s\n",consoleEntryText); ProcessConsoleCommand(consoleEntryText); }
}
