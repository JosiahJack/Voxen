// input.c - Input and Configuration System for Config.ini, keyboard and mouse support.
#include "common.h"
#include "lib.h"
double last_mouse_x,last_mouse_y;
InputElement inputElements[134]={{"A",KEY_A},{"B",KEY_B},{"C",KEY_C},{"D",KEY_D},{"E",KEY_E},{"F",KEY_F},{"G",KEY_G},{"H",KEY_H},{"I",KEY_I},{"J",KEY_J},{"K",KEY_K},{"L",KEY_L},{"M",KEY_M},{"N",KEY_N},{"O",KEY_O},{"P",KEY_P},{"Q",KEY_Q},{"R",KEY_R},{"S",KEY_S},{"T",KEY_T},{"U",KEY_U},{"V",KEY_V},{"W",KEY_W},{"X",KEY_X},{"Y",KEY_Y},{"Z",KEY_Z},
                                 {"1",KEY_1},{"2",KEY_2},{"3",KEY_3},{"4",KEY_4},{"5",KEY_5},{"6",KEY_6},{"7",KEY_7},{"8",KEY_8},{"9",KEY_9},{"0",KEY_0},{"UPARROW",KEY_UP},{"DNARROW",KEY_DOWN},{"LFARROW",KEY_LEFT},{"RTARROW",KEY_RIGHT},{"NUM1",KEY_KP_1},{"NUM2",KEY_KP_2},{"NUM3",KEY_KP_3},{"NUM+",KEY_KP_ADD},{"ENTER",KEY_ENTER},
                                 {"RIGHTSHIFT",KEY_RIGHT_SHIFT},{"LEFTSHIFT",KEY_LEFT_SHIFT},{"RIGHTCTRL",KEY_RIGHT_CONTROL},{"LEFTCTRL",KEY_LEFT_CONTROL},{"RIGHTALT",KEY_RIGHT_ALT},{"LEFTALT",KEY_LEFT_ALT},{"RIGHTCMD",KEY_RIGHT_SUPER},{"LEFTCMD",KEY_LEFT_SUPER},
                                 {"LMB",MOUSE_BUTTON_1},{"RMB",MOUSE_BUTTON_2},{"MMB",MOUSE_BUTTON_3},{"MB3",MOUSE_BUTTON_4},{"MB4",MOUSE_BUTTON_5},{"MB5",MOUSE_BUTTON_6},{"MB6",MOUSE_BUTTON_7},{"MB7",MOUSE_BUTTON_8},{"JOY0",JOYSTICK_1},{"JOY1",JOYSTICK_2},{"JOY2",JOYSTICK_3},{"JOY3",JOYSTICK_4},{"JOY4",JOYSTICK_5},{"JOY5",JOYSTICK_6},
                                 {"JOY6",JOYSTICK_7},{"JOY7",JOYSTICK_8},{"JOY8",JOYSTICK_9},{"JOY9",JOYSTICK_10},{"JOY10",JOYSTICK_11},{"JOY11",JOYSTICK_12},{"JOY12",JOYSTICK_13},{"JOY13",JOYSTICK_14},{"JOY14",JOYSTICK_15},{"JOY15",JOYSTICK_16},{"JOY16",JOYHAT_UP},{"JOY17",JOYHAT_RIGHT},
                                 {"BACKSPACE",KEY_BACKSPACE},{"TAB",KEY_TAB},{"NUMENTER",KEY_KP_ENTER},{"ESCAPE",KEY_ESCAPE},{"SPACE",KEY_SPACE},{"DELETE",KEY_DELETE},{"INSERT",KEY_INSERT},{"HOME",KEY_HOME},{"END",KEY_END},{"PAGEUP",KEY_PAGE_UP},{"PAGEDN",KEY_PAGE_DOWN},
                                 {"F1",KEY_F1},{"F2",KEY_F2},{"F3",KEY_F3},{"F4",KEY_F4},{"F5",KEY_F5},{"F6",KEY_F6},{"F7",KEY_F7},{"F8",KEY_F8},{"F9",KEY_F9},{"F10",KEY_F10},{"F11",KEY_F11},{"F12",KEY_F12},{"GRAVE",KEY_GRAVE_ACCENT},{"-",KEY_MINUS},{"=",KEY_EQUAL},{"[",KEY_LEFT_BRACKET},{"]",KEY_RIGHT_BRACKET},{"\\",KEY_BACKSLASH},{"/",KEY_SLASH},
                                 {".",KEY_PERIOD},{",",KEY_COMMA},{";",KEY_SEMICOLON},{"'",KEY_APOSTROPHE},{"CAPSLOCK",KEY_CAPS_LOCK},{"NUM0",KEY_KP_0},{"NUM4",KEY_KP_4},{"NUM5",KEY_KP_5},{"NUM6",KEY_KP_6},{"NUM7",KEY_KP_7},{"NUM8",KEY_KP_8},{"NUM9",KEY_KP_9},{"NUM*",KEY_KP_MULTIPLY},{"NUM-",KEY_KP_SUBTRACT},{"NUM.",KEY_KP_DECIMAL},{"MENU",KEY_MENU},
                                 {"PAUSE",KEY_PAUSE},{"NUMLOCK",KEY_NUM_LOCK},{"MWHEEL+",128},{"MWHEEL-",129},/*128,129,Handledspecialcaseformousewheel+/-respectively*/{"PRINT",KEY_PRINT_SCREEN},{"JOY18",JOYHAT_DOWN},{"JOY19",JOYHAT_LEFT},{"UNUSED",0}};
KeyState* GetCodeMapping(int settingIndex) {
    i32 i = Sys_Settings.InputCodeSettings[settingIndex]; // Get table index into all recognized inputs
    if (i == 148 || i >= MAX_KEYS) return &Sys_Input.keyStates[MAX_KEYS - 1]; // UNUSED NULL (e.g. setting unbound)
    if (i >= 53 && i <= 61) return &Sys_Input.mouseButtons[inputElements[i].value];
    return &Sys_Input.keyStates[inputElements[i].value];
}

void TextEntry(i32 k) {
    if (k == KEY_U && Sys_Input.keyStates[KEY_LEFT_CONTROL].down) { World.playerName[0] = '\0'; currentPlayerNameLength = 0; return; }
    if (k == KEY_ENTER || k == KEY_KP_ENTER) { currentMenuItem++; return; }
    if (k == KEY_BACKSPACE && currentPlayerNameLength > 0) { World.playerName[--currentPlayerNameLength] = '\0'; return; }
    if (currentPlayerNameLength >= 26) return;
    char c = (k >= KEY_A && k <= KEY_Z) ? 'a' + (k - KEY_A) : ((k >= KEY_1 && k <= KEY_9) ? '1' + (k - KEY_1) : ((k == KEY_0) ? '0' : ((k == KEY_SPACE) ? ' ' : 0)));
    if (c) { World.playerName[currentPlayerNameLength] = c; World.playerName[++currentPlayerNameLength] = '\0'; }
}

void ConsoleEmulator(i32 keycode); extern bool enteringPlayerName;
void InputKey(char* keys,int key,int action) {
    if (key >= 0 && key <= 348) { i32 repeated=0; if(action == INPUT_RELEASE && keys[key] == INPUT_RELEASE){return;} if (action == INPUT_PRESS && keys[key] == INPUT_PRESS){repeated=1;} keys[key]=(char)action; if(repeated){action=INPUT_REPEAT;} }
    if (!window_has_focus) return;
    if (key == KEY_F10 && action) OS_Exit(0);
    if (World.menuActive && !returnToPause) { if (((key == KEY_RIGHT_ALT || key == KEY_LEFT_ALT) && action && Sys_Input.keyStates[KEY_ENTER].down) || (key == KEY_ENTER && action && (Sys_Input.keyStates[KEY_LEFT_ALT].down || Sys_Input.keyStates[KEY_RIGHT_ALT].down))){GoIntoGame();} }
    if (key >=0 && key < MAX_KEYS && (action == INPUT_PRESS || (action == INPUT_REPEAT && !(key == KEY_KP_ENTER || key == KEY_ENTER || key == KEY_TAB || key == KEY_ESCAPE)))) {
        Sys_Input.keyStates[key].down = true; if (action == INPUT_PRESS) Sys_Input.keyStates[key].pressed = true; else Sys_Input.keyStates[key].pressed = false;
        if (Cheats.consoleActive) ConsoleEmulator(key);
        else if (enteringPlayerName && World.menuActive) TextEntry(key);
    } else if (key >= 0 && key < MAX_KEYS && action == INPUT_RELEASE) { Sys_Input.keyStates[key].pressed=false; Sys_Input.keyStates[key].down=false; }
}

void InputMouseClick(char* mouseButtons, int button, int action) { if (button<0 || button>7) {return;} char wasDown = mouseButtons[button]; mouseButtons[button] = (char)action; bool down = (action == 1); Sys_Input.mouseButtons[button].pressed  = down && !wasDown; Sys_Input.mouseButtons[button].released = !down && wasDown; Sys_Input.mouseButtons[button].down=down; }
void quat_from_yaw_pitch_roll(Quaternion* q, float yaw_deg, float pitch_deg, float roll_deg) { float yaw=deg2rad(yaw_deg), pitch=deg2rad(pitch_deg), roll=deg2rad(roll_deg/*Around Z (forward)*/); float cy=vcosf(yaw * 0.5f), sy=vsinf(yaw * 0.5f), cp=vcosf(pitch * 0.5f), sp=vsinf(pitch * 0.5f), cr=vcosf(roll * 0.5f), sr=vsinf(roll * 0.5f); q->w=cy*cp*cr + sy*sp*sr; q->x=cy*sp*cr + sy*cp*sr;/*X(pitch)*/ q->y=sy*cp*cr - cy*sp*sr;/*Y(yaw)*/ q->z=cy*cp*sr - sy*sp*cr;/*Z(roll)*/ } // Skipping quat normalization, not needed
bool firstFrameMouselook = true;
void InputCursorPos(double* x, double* y, double xpos, double ypos) { // static const float HeadBobRate   = 0.2f, HeadBobAmount = 0.08f,bobTarget = 0.3f; TODO
    if (firstFrameMouselook) { firstFrameMouselook=false; *x=xpos; *y=ypos; }
    if (*x == xpos && *y == ypos) { last_mouse_x=xpos; last_mouse_y=ypos; return;}
    *x=xpos; *y=ypos; if (!window_has_focus){return;}
    if (ignore_next_mouse_delta) { World.currentMouse_dx = World.currentMouse_dy = 0; ignore_next_mouse_delta = mouseMovementThisFrame = false; return; }
    World.currentMouse_dx = (i32)(xpos - last_mouse_x); World.currentMouse_dy = (i32)(ypos - last_mouse_y); last_mouse_x = xpos; last_mouse_y = ypos;
    if ((World.inventoryMode && !Cheats.noHUD) || World.menuActive || World.paused) { // Uses UI baseline resolution 1366x768
        i32 newX = clamp(World.cursorPosition_x + World.currentMouse_dx,0,1366); if (newX != World.cursorPosition_x) {mouseMovementThisFrame = true;} World.cursorPosition_x = newX;
        i32 newY = clamp(World.cursorPosition_y + World.currentMouse_dy,0, 768); if (newY != World.cursorPosition_y) {mouseMovementThisFrame = true;} World.cursorPosition_y = newY;
    }
}

bool GetKeyRiseEdgeOrHeld(int sI, bool onRise) { i32 i = Sys_Settings.InputCodeSettings[sI]; if (i == 128) {return Sys_Input.scrollDelta > 0;} if (i == 129) {return Sys_Input.scrollDelta < 0;} KeyState* k = GetCodeMapping(sI); return onRise ? k->pressed : k->down; }
bool GetKey(int settingIndex) { return GetKeyRiseEdgeOrHeld(settingIndex,false); }  // True while held down.
bool GetKeyPressed(int settingIndex) { return (settingIndex < 0) ? Sys_Input.keyStates[KEY_GRAVE_ACCENT].pressed : GetKeyRiseEdgeOrHeld(settingIndex,true); } // True 1st frame down.
bool Forward() { return GetKey(0); }                bool StrafeLeft() { return GetKey(1); }             bool Backpedal() { return GetKey(2); }            bool StrafeRight() { return GetKey(3); }            bool Jump() { return GetKey(4); }                   bool JumpDown() { return GetKeyPressed(4); }
bool Crouch() { return GetKeyPressed(5); }          bool Prone() { return GetKeyPressed(6); }           bool LeanLeft() { return GetKey(7); }             bool LeanRight() { return GetKey(8); }              bool Sprint() { return GetKey(9); }                 bool TurnLeft() { return GetKey(10); }
bool TurnRight() { return GetKey(11); }             bool LookUp() { return GetKey(12); }                bool LookDown() { return GetKey(13); }            bool RecentLog() { return GetKeyPressed(14); }      bool Biomonitor() { return GetKeyPressed(15); }     bool Sensaround() { return GetKeyPressed(16); }
bool Lantern() { return GetKeyPressed(17); }        bool Shield() { return GetKeyPressed(18); }         bool Infrared() { return GetKeyPressed(19); }     bool Email() { return GetKeyPressed(20); }          bool Booster() { return GetKeyPressed(21); }        bool Jumpjets() { return GetKeyPressed(22); }
bool Attack() { return GetKeyPressed(23); }         bool Use() { return GetKeyPressed(24); }            bool Menu() { return GetKeyPressed(25); }         bool ToggleMode() { return GetKeyPressed(26); }     bool Reload() { return GetKeyPressed(27); }         bool WeaponCycUp() { return GetKeyPressed(28); }
bool WeaponCycDown() { return GetKeyPressed(29); }  bool Grenade() { return GetKeyPressed(30); }        bool GrenadeCycUp() { return GetKeyPressed(31); } bool GrenadeCycDown() { return GetKeyPressed(32); } bool ChangeAmmoType() { return GetKeyPressed(33); } bool Patch() { return GetKeyPressed(34); }
bool PatchCycUp() { return GetKeyPressed(35); }     bool PatchCycDown() { return GetKeyPressed(36); }   bool Map() { return GetKeyPressed(37); }          bool SwimUp() {return Cheats.noclip && GetKey(38);} bool SwimDn() {return Cheats.noclip && GetKey(39);} bool Console() { return GetKeyPressed(-1); }     bool ScrshotPressed() { return GetKeyPressed(41); } 
bool DoubleTapLeanLeft(void)  { if(!GetKeyPressed(7)){return false;} if (World.pauseRelativeTime < World.invP1.leanLeftTapFinished) { World.invP1.leanLeftTapFinished = 0.0; return true; } World.invP1.leanLeftTapFinished = World.pauseRelativeTime + 0.5; return false; }
bool DoubleTapLeanRight(void) { if(!GetKeyPressed(8)){return false;} if (World.pauseRelativeTime < World.invP1.leanRightTapFinished) { World.invP1.leanRightTapFinished = 0.0; return true; } World.invP1.leanRightTapFinished = World.pauseRelativeTime + 0.5; return false; } 
void CloseFullmap();
void ForceShootMode() { if (Sys_Settings.NoShootMode){return;} World.Sys_UI.mouseClickHeldOverGUI=World.inventoryMode=false; CloseFullmap(); World.cursorPosition_x=663; World.cursorPosition_y=371/*Centered UI fixed 1366x768*/; ignore_next_mouse_delta=true; if(World.Sys_UI.vmailActive){World.Sys_UI.vmailActive=0; World.Sys_UI.vmailActive=false;} }
void ForceInventoryMode() { World.inventoryMode = true; World.cursorPosition_x = 663; World.cursorPosition_y = 371; ignore_next_mouse_delta = true; } // Centered on UI baseline resolution 1366x768
void ToggleInventoryMode() { if (World.inventoryMode) {ForceShootMode();} else {ForceInventoryMode();} }
void ToggleConsole() { static bool imWasActPrior = false; if (!Cheats.consoleActive) {imWasActPrior = World.inventoryMode;} Cheats.consoleActive = !Cheats.consoleActive; World.paused = !World.paused; if (Cheats.consoleActive) { World.inventoryMode = true; } else if (!imWasActPrior && World.inventoryMode) {ForceShootMode();} }
void MenuGoBack(); void SaveGame(u8 slot, const char* savename); void LoadGame(u8 slot); void ApplyPlayerMovements(float dt); void PollEvents();
void play_synth_laser(float volume,float freq,float sweep,float fmrate,float decay); void play_synth_door(float volume,float pitch); void play_synth_impact(float volume,float ring_freq,float decay,float noise_amt,float ring_amt);
void InputProcessing() {
    mouseMovementThisFrame = false; PollEvents();
    if (window_has_focus) {
        float v = 0.1f;
        if (Sys_Input.keyStates[KEY_E].pressed) play_wav("./Audio/cyborgs/yourlevelsareterrible.wav",0.1f,(V3){0.0f,0.0f,0.0f},false);
        if (Sys_Input.keyStates[KEY_W].pressed) play_synth_door(v,50); // thud slide
        if (Sys_Input.keyStates[KEY_T].pressed) play_synth_impact(v,4500,18,0.3f,0.6f); // Glass ting
        if (Sys_Input.keyStates[KEY_R].pressed) play_synth_impact(v,1800,30,0.5f,0.3f); // cartridge drop
        if (Sys_Input.keyStates[KEY_Y].pressed) play_synth_laser(v,800,-2.0f,40,12);
        if (Sys_Input.keyStates[KEY_U].pressed) play_synth_laser(v,800,2.0f,40,12);
        if (Sys_Input.keyStates[KEY_CAPS_LOCK].pressed) Sys_Input.isCapsLockOn = !Sys_Input.isCapsLockOn;
        if (Sys_Input.keyStates[KEY_F6].pressed && (get_time() - World.justSavedTimeStamp) > 0.2) { Sys_Input.keyStates[KEY_F6].pressed = false; SaveGame(7,"quicksave"); return; }
        if (Sys_Input.keyStates[KEY_F9].pressed && (get_time() - World.justSavedTimeStamp) > 0.2) { Sys_Input.keyStates[KEY_F9].pressed = false; LoadGame(7); return; }
        if (Console()) ToggleConsole();
        if (Menu() && !World.menuActive) { World.paused = !World.paused; return; }
        if (Menu() && World.menuActive) { MenuGoBack(); return; }
        if (World.paused || World.menuActive || Cheats.consoleActive) return; // Pause/Menu barrier <<<<<<<
        if (ToggleMode()) ToggleInventoryMode();
        if (Lantern()) World.invP1.hardwareIsActive ^= HW_LAN;
        if (Infrared()) World.invP1.hardwareIsActive ^= HW_INF;
        ApplyPlayerMovements(World.dt);
        if (!World.paused && !World.menuActive && !World.inventoryMode) { // Apply mouselook/keyboardlook/lean
            float s = vclamp((float)Sys_Settings.MouseSensitivity / 100.0f, 0.01f, 1.0f) * 0.2f;
            World.cam_yaw += (float)World.currentMouse_dx * s; if (World.cam_yaw >= 360.0f) {World.cam_yaw -= 360.0f;} if (World.cam_yaw < 0.0f)     {World.cam_yaw  += 360.0f;}
            World.cam_pitch+=(float)World.currentMouse_dy * s; if (World.cam_pitch > 89.0f) {World.cam_pitch = 89.0f;} if (World.cam_pitch < -89.0f) {World.cam_pitch = -89.0f;} // Avoid gimbal lock at pure 90deg
            quat_from_yaw_pitch_roll(&World.rotation[PLAYER1],World.cam_yaw,World.cam_pitch,World.cam_roll);
        }
    }
}

void ResetInput() { for (i32 i=0;i<MAX_KEYS;++i) {Sys_Input.keyStates[i].pressed = Sys_Input.keyStates[i].released = false;} for (i32 i=0;i<MAX_MOUSE_BUTTONS;i++) {Sys_Input.mouseButtons[i].pressed = Sys_Input.mouseButtons[i].released = false;} Sys_Input.scrollDelta = 0; World.currentMouse_dx = World.currentMouse_dy = 0; } // Can't memset as we want to preserve down state

// Configuration Options Settings Sys
typedef enum { SETTING_U8, SETTING_U16, SETTING_INPUT } SettingType; typedef struct { const char* name; void* ptr; SettingType type; } Setting;
#define S_U8(n, v)  { n, &Sys_Settings.v, SETTING_U8 }
#define S_U16(n, v) { n, &Sys_Settings.v, SETTING_U16 }
#define S_IN(n, i)  { n, &Sys_Settings.InputCodeSettings[i], SETTING_INPUT }
const Setting configTable[] = {
    S_U16("ResolutionWidth",ScreenWidth),S_U16("ResolutionHeight",ScreenHeight),S_U8("Fullscreen",Fullscreen),      S_U8("FOV",FOV),                     S_U8("Brightness",Brightness),
    S_U8("Gamma",Gamma),S_U8("AA",FXAA),  S_U8("Shadows",Shadows),              S_U8("SSR",Reflections),            S_U8("VSync",Vsync),                 S_U8("ModelDetail",ModelDetail),
    S_U8("GI",GI),                        S_U8("SpeakerMode",SpeakerMode),      S_U8("Reverb",Reverb),              S_U8("VolumeMaster",VolumeMaster),   S_U8("VolumeMusic",VolumeMusic),
    S_U8("VolumeMessage",VolumeMessage),  S_U8("VolumeEffects",VolumeEffects),  S_U8("Language",Language),          S_U8("DynamicMusic",DynamicMusic),   S_U8("Footsteps",Footsteps),
    S_U8("InvertLook",InvertLook),        S_U8("Monitor",CurrentMonitor),       
    S_U8("InvertCyberspaceLook",InvertCyberspaceLook),  S_U8("InvertInventoryCycling",InvertInventoryCycling),S_U8("QuickItemPickup",QuickItemPickup),
    S_U8("QuickReloadWeapons",QuickReloadWeapons),      S_U8("MouseSensitivity",MouseSensitivity),            S_U8("NoShootMode",NoShootMode),           S_U8("HeadBob",HeadBob),
    S_IN("Forward",0),    S_IN("Strafe Left",1),S_IN("Backpedal",2), S_IN("Strafe Right",3),S_IN("Jump",4),        S_IN("Crouch",5),    S_IN("Prone",6),       S_IN("Lean Left",7),
    S_IN("Lean Right",8), S_IN("Sprint",9),     S_IN("Turn Left",10),S_IN("Turn Right",11), S_IN("Look Up",12),    S_IN("Look Down",13),S_IN("Recent Log",14),
    S_IN("Biomonitor",15),S_IN("Sensaround",16),S_IN("Lantern",17),  S_IN("Shield",18),     S_IN("Infrared",19),   S_IN("Email",20),    S_IN("Booster",21),
    S_IN("Jumpjets",22),  S_IN("Attack",23),    S_IN("Use",24),      S_IN("Menu/Back",25),  S_IN("Toggle Mode",26),S_IN("Reload",27),
    S_IN("Weapon +",28),  S_IN("Weapon -",29),  S_IN("Grenade",30),  S_IN("Grenade +",31),  S_IN("Grenade -",32),  S_IN("Ammo Type",33),S_IN("Patch Use",34),
    S_IN("Patch +",35),   S_IN("Patch -",36),   S_IN("Full Map",37), S_IN("Swim Up",38),    S_IN("Swim Down",39),  S_IN("Screenshot",40)
};

const int configTableSize = sizeof(configTable) / sizeof(Setting);
INLINE i32 GetWinSysIndirectionIndexForAnInput(const char* val) { for (int i=0;i<134;++i) {if (sEqual(val,inputElements[i].name)) return i;} return 148; }
void LoadConfig() {
    FHandle f = OS_OpenReadonly("./Data/Config.ini");
    char line[512];
    while (sUpToEndLine(line,sizeof(line),f)) {
        char* s = data_parser_trim(line); if (*s == 0 || (s[0] == '/' && s[1] == '/')) continue;
        char* eq = StringFindFirstCharWithin(s, '='); if (!eq) continue;
        *eq = 0; char *key = data_parser_trim(s), *val = data_parser_trim(eq + 1);
        for (int i = 0; i < configTableSize; i++) {
            if (sEqual(key,configTable[i].name)) {
                if (configTable[i].type == SETTING_U8)         *( u8*)configTable[i].ptr = (u8)s2i32(val);
                else if (configTable[i].type == SETTING_U16)   *(u16*)configTable[i].ptr = (u16)s2i32(val);
                else if (configTable[i].type == SETTING_INPUT) *(u16*)configTable[i].ptr = GetWinSysIndirectionIndexForAnInput(val);
                break;
            }
        }
    }
    Sys_Settings.ScreenWidth = vmax(Sys_Settings.ScreenWidth,320); Sys_Settings.ScreenHeight = vmax(Sys_Settings.ScreenHeight,200);
    OS_Close(f);
}

void FilePrintString(FHandle f, const char* fmt, ...) { va_list a; __builtin_va_start(a,fmt); char b[128]; va_list c; __builtin_va_copy(c,a); sFormatV(b,sizeof(b),fmt,c); __builtin_va_end(c); OS_RawWrite(f,b,slen(b)); __builtin_va_end(a); }
void SaveConfig() {
    DualLog("Saving config\n");
    FHandle f = OS_OpenWriteonly("./Data/Config.ini");
    for (int i=0;i<configTableSize;++i) {
        if (configTable[i].type == SETTING_U8)         FilePrintString(f,"%s = %u\n",configTable[i].name,*(u8*)configTable[i].ptr);
        else if (configTable[i].type == SETTING_U16)   FilePrintString(f,"%s = %u\n",configTable[i].name,*(u16*)configTable[i].ptr);
        else if (configTable[i].type == SETTING_INPUT) FilePrintString(f,"%s = %s\n",configTable[i].name,inputElements[*(u16*)configTable[i].ptr].name);
    }
    OS_Close(f);
    DualLog("Saved settings to ./Data/Config.ini! framenum %u\n",globalframe);
}
