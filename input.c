InputSystem Sys_Input;
extern uint16_t editModeTestEntityDefinition;
extern uint16_t editModeSelection;

SettingsSystem Sys_Settings = { // Potato defaults so initial state is good on first run for potatoes (e.g. won't crash for out of VRAM, or won't take 5min to init).
    .InputCodeSettings = {
        5,   // Forward       = F
        0,   // Strafe Left   = A
        18,  // Backpedal     = S
        3,   // Strafe Right  = D
        100, // Jump          = SPACE
        2,   // Crouch        = C
        23,  // Prone         = X
        16,  // Lean Left     = Q
        4,   // Lean Right    = E
        45,  // Sprint        = LEFT SHIFT
        38,  // Turn Left     = LF ARROW
        39,  // Turn Right    = RT ARROW
        36,  // Look Up       = UP ARROW
        37,  // Look Down     = DN ARROW
        20,  // Recent Log    = U
        26,  // Biomonitor    = 1
        27,  // Sensaround    = 2
        28,  // Lantern       = 3
        29,  // Shield        = 4
        30,  // Infrared      = 5
        31,  // Email         = 6
        32,  // Booster       = 7
        33,  // Jumpjets      = 8
        56,  // Attack        = LMB
        57,  // Use           = RMB
        99,  // Menu/Back     = ESCAPE
        97,  // Toggle Mode   = TAB
        17,  // Reload        = R
        128, // Weapon +      = MWHEEL +
        129, // Weapon -      = MWHEEL -
        6,   // Grenade       = G
        19,  // Grenade +     = T
        131, // Grenade -     = B
        21,  // Ammo Type     = V
        9,   // Patch Use     = J
        8,   // Patch +       = I
        132, // Patch -       = ,
        12,  // Full Map      = M
        21,  // Swim Up       = V
        2,   // Swim Down     = C
        103, // Console       = `/~
        102  // Screenshot    = F12
    },
    .ScreenWidth = 800u,
    .ScreenHeight = 600u,
    .Fullscreen = 0u,
    .FOV = 65u,
    .Brightness = 50u,
    .Gamma = 50u,
    .AntiAliasing = 0u,
    .Shadows = 0u,
    .Reflections = 0u,
    .Vsync = 0u,
    .ModelDetail = 0u,
    .GI = 0u,
    .SpeakerMode = 1u, // Stereo
    .Reverb = 0u,
    .VolumeMaster = 100u,
    .VolumeMusic = 25u,
    .VolumeMessage = 75u,
    .VolumeEffects = 100u,
    .Language = 0u, // Default English
    .DynamicMusic = 1u,
    .Footsteps = 1u,
    .InvertLook = 0u,
    .InvertCyberspaceLook = 0u,
    .QuickItemPickup = 0u,
    .QuickReloadWeapons = 0u,
    .MouseSensitivity = 10u,
    .NoShootMode = 0u,
    .HeadBob = 1u,
    .SSR_RES = 8u // Ratio is (1 / SSR_RES) * render resolution.
};

typedef struct {
    const char* name;
    int value;
} InputElement;
InputElement inputElements[149] = {
    { "A", GLFW_KEY_A }, // 0
    { "B", GLFW_KEY_B }, // 1
    { "C", GLFW_KEY_C }, // 2
    { "D", GLFW_KEY_D }, // 3
    { "E", GLFW_KEY_E }, // 4
    { "F", GLFW_KEY_F }, // 5
    { "G", GLFW_KEY_G }, // 6
    { "H", GLFW_KEY_H }, // 7
    { "I", GLFW_KEY_I }, // 8
    { "J", GLFW_KEY_J }, { "K", GLFW_KEY_K }, { "L", GLFW_KEY_L }, // 11
    { "M", GLFW_KEY_M }, { "N", GLFW_KEY_N }, { "O", GLFW_KEY_O }, { "P", GLFW_KEY_P }, // 15
    { "Q", GLFW_KEY_Q }, { "R", GLFW_KEY_R }, { "S", GLFW_KEY_S }, { "T", GLFW_KEY_T }, // 19
    { "U", GLFW_KEY_U }, { "V", GLFW_KEY_V }, { "W", GLFW_KEY_W }, { "X", GLFW_KEY_X }, // 23
    { "Y", GLFW_KEY_Y }, { "Z", GLFW_KEY_Z }, // 25
    { "1", GLFW_KEY_1 }, { "2", GLFW_KEY_2 }, { "3", GLFW_KEY_3 }, { "4", GLFW_KEY_4 }, // 29
    { "5", GLFW_KEY_5 }, { "6", GLFW_KEY_6 }, { "7", GLFW_KEY_7 }, { "8", GLFW_KEY_8 }, // 33
    { "9", GLFW_KEY_9 }, { "0", GLFW_KEY_0 }, // 35
    { "UP ARROW", GLFW_KEY_UP }, { "DN ARROW", GLFW_KEY_DOWN }, { "LF ARROW", GLFW_KEY_LEFT }, { "RT ARROW", GLFW_KEY_RIGHT }, // 39
    { "NUM 1", GLFW_KEY_KP_1 }, { "NUM 2", GLFW_KEY_KP_2 }, { "NUM 3", GLFW_KEY_KP_3 }, { "NUM +", GLFW_KEY_KP_ADD }, // 43
    { "ENTER", GLFW_KEY_ENTER }, // 44
    { "RIGHT SHIFT", GLFW_KEY_RIGHT_SHIFT }, { "LEFT SHIFT", GLFW_KEY_LEFT_SHIFT }, { "RIGHT CTRL", GLFW_KEY_RIGHT_CONTROL }, { "LEFT CTRL", GLFW_KEY_LEFT_CONTROL }, // 48
    { "RIGHT ALT", GLFW_KEY_RIGHT_ALT }, { "LEFT ALT", GLFW_KEY_LEFT_ALT }, { "RIGHT CMD", GLFW_KEY_RIGHT_SUPER }, { "LEFT CMD", GLFW_KEY_LEFT_SUPER }, // 52
    { "LMB", GLFW_MOUSE_BUTTON_1 }, { "RMB", GLFW_MOUSE_BUTTON_2 }, { "MMB", GLFW_MOUSE_BUTTON_3 }, { "MB 3", GLFW_MOUSE_BUTTON_4 }, // 56
    { "MB 4", GLFW_MOUSE_BUTTON_5 }, { "MB 5", GLFW_MOUSE_BUTTON_6 }, { "MB 6", GLFW_MOUSE_BUTTON_7 }, { "MB 7", GLFW_MOUSE_BUTTON_8 }, { "MB 8", GLFW_MOUSE_BUTTON_LAST }, // 61, GLFW Only has 8... :(  , 1 indexed in name, so off by 1.
    { "JOY 0", GLFW_JOYSTICK_1 }, { "JOY 1", GLFW_JOYSTICK_2 }, { "JOY 2", GLFW_JOYSTICK_3 }, { "JOY 3", GLFW_JOYSTICK_4 }, // 65
    { "JOY 4", GLFW_JOYSTICK_5 }, { "JOY 5", GLFW_JOYSTICK_6 }, { "JOY 6", GLFW_JOYSTICK_7 }, { "JOY 7", GLFW_JOYSTICK_8 }, // 69
    { "JOY 8", GLFW_JOYSTICK_9 }, { "JOY 9", GLFW_JOYSTICK_10 }, { "JOY 10", GLFW_JOYSTICK_11 }, { "JOY 11", GLFW_JOYSTICK_12 }, // 73
    { "JOY 12", GLFW_JOYSTICK_13 }, { "JOY 13", GLFW_JOYSTICK_14 }, { "JOY 14", GLFW_JOYSTICK_15 }, { "JOY 15", GLFW_JOYSTICK_16 }, // 77
    { "JOY 16", GLFW_HAT_UP }, { "JOY 17", GLFW_HAT_RIGHT }, // 79
    { "BACKSPACE", GLFW_KEY_BACKSPACE }, { "TAB", GLFW_KEY_TAB }, { "NUM ENTER", GLFW_KEY_KP_ENTER }, { "ESCAPE", GLFW_KEY_ESCAPE }, // 83
    { "SPACE", GLFW_KEY_SPACE }, { "DELETE", GLFW_KEY_DELETE }, { "INSERT", GLFW_KEY_INSERT }, // 86
    { "HOME", GLFW_KEY_HOME }, { "END", GLFW_KEY_END }, { "PAGE UP", GLFW_KEY_PAGE_UP }, { "PAGE DN", GLFW_KEY_PAGE_DOWN }, // 90
    { "F1", GLFW_KEY_F1 }, { "F2", GLFW_KEY_F2 }, { "F3", GLFW_KEY_F3 }, // 93
    { "F4", GLFW_KEY_F4 }, { "F5", GLFW_KEY_F5 }, { "F6", GLFW_KEY_F6 }, // 96
    { "F7", GLFW_KEY_F7 }, { "F8", GLFW_KEY_F8 }, { "F9", GLFW_KEY_F9 }, // 99
    { "F10", GLFW_KEY_F10 }, { "F11", GLFW_KEY_F11 }, { "F12", GLFW_KEY_F12 }, // 102
    { "GRAVE", GLFW_KEY_GRAVE_ACCENT }, { "-", GLFW_KEY_MINUS }, // 104
    { "=", GLFW_KEY_EQUAL }, { "[", GLFW_KEY_LEFT_BRACKET }, // 106
    { "]", GLFW_KEY_RIGHT_BRACKET }, { "\\", GLFW_KEY_BACKSLASH }, { "/", GLFW_KEY_SLASH }, { ".", GLFW_KEY_PERIOD }, // 110
    { ",", GLFW_KEY_COMMA }, { ";", GLFW_KEY_SEMICOLON }, { "'", GLFW_KEY_APOSTROPHE }, { "CAPSLOCK", GLFW_KEY_CAPS_LOCK }, // 114
    { "NUM 0", GLFW_KEY_KP_0 }, { "NUM 4", GLFW_KEY_KP_4 }, // 116
    { "NUM 5", GLFW_KEY_KP_5 }, { "NUM 6", GLFW_KEY_KP_6 }, // 118
    { "NUM 7", GLFW_KEY_KP_7 }, { "NUM 8", GLFW_KEY_KP_8 }, // 120
    { "NUM 9", GLFW_KEY_KP_9 }, // 121
    { "NUM *", GLFW_KEY_KP_MULTIPLY }, { "NUM -", GLFW_KEY_KP_SUBTRACT }, { "NUM .", GLFW_KEY_KP_DECIMAL }, // 124
    { "MENU", GLFW_KEY_MENU }, { "PAUSE", GLFW_KEY_PAUSE }, { "NUMLOCK", GLFW_KEY_NUM_LOCK }, // 127
    { "MWHEEL +", 128 }, { "MWHEEL -", 129 }, // 128, 129, Handled special case for mouse wheel + / - respectively
    { "PRINT", GLFW_KEY_PRINT_SCREEN }, // 130
    { "JOY 18", GLFW_HAT_DOWN }, { "JOY 19", GLFW_HAT_LEFT }, // 132
    { "GPAD A", GLFW_GAMEPAD_BUTTON_A }, { "GPAD B", GLFW_GAMEPAD_BUTTON_B }, { "GPAD X", GLFW_GAMEPAD_BUTTON_X }, { "GPAD Y", GLFW_GAMEPAD_BUTTON_Y }, // 136
    { "GPAD L1", GLFW_GAMEPAD_BUTTON_LEFT_BUMPER }, { "GPAD R1", GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER }, // 138
    { "GPAD BACK", GLFW_GAMEPAD_BUTTON_BACK }, { "GPAD START", GLFW_GAMEPAD_BUTTON_START }, { "GPAD GUIDE", GLFW_GAMEPAD_BUTTON_GUIDE }, // 141
    { "GPAD LSTICK CLICK", GLFW_GAMEPAD_BUTTON_LEFT_THUMB }, { "GPAD RSTICK CLICK", GLFW_GAMEPAD_BUTTON_RIGHT_THUMB }, // 143
    { "GPAD D UP", GLFW_GAMEPAD_BUTTON_DPAD_UP }, { "GPAD D RIGHT", GLFW_GAMEPAD_BUTTON_DPAD_RIGHT }, // 145
    { "GPAD D DOWN", GLFW_GAMEPAD_BUTTON_DPAD_DOWN }, { "GPAD D LEFT", GLFW_GAMEPAD_BUTTON_DPAD_LEFT }, // 147
    { "UNUSED", 0 } // 148
};

int32_t GetGLFWIndirectionIndexForAnInput(const char* val) {
    for (int i=0;i<149;++i) {
        if (!strcmp(val, inputElements[i].name)) return i;
    }
    
    return 148;
}

char* data_parser_trim(char* s) {
    while (data_parser_isspace((unsigned char)*s)) s++;
    if (*s == 0) return s;

    char* e = s + strlen(s) - 1;
    while (e > s && data_parser_isspace((unsigned char)*e)) e--;
    e[1] = 0;
    return s;
}

int atoi(const char *str) {
    while (data_parser_isspace(*str)) str++;
    int sign = 1;
         if (*str == '-') { sign = -1; str++; }
    else if (*str == '+') str++;

    if (*str < '0' || *str > '9') return 0;
    int result = 0;
    while (*str >= '0' && *str <= '9') {
        int digit = *str - '0';
        if (result > (INT_MAX - digit) / 10) return (sign == 1) ? INT_MAX : INT_MIN;

        result = result * 10 + digit;
        str++;
    }

    return sign * result;
}

void LoadConfig(void) {
    FILE* f = fopen("./Data/Config.ini", "r");
    if (!f) return;

    char line[512];
    while (fgets(line, sizeof(line), f)) { // Loop by line
        char* s = data_parser_trim(line);
        if (*s == 0) continue;
        if (s[0] == '/' && s[1] == '/') continue;

        char* eq = strchr(s, '=');
        if (!eq) continue;

        *eq = 0;
        char* key = data_parser_trim(s);
        char* val = data_parser_trim(eq + 1); // After trimming key and value whitespace, check for matching setting on this line.
             if (!strcmp(key, "ResolutionWidth"))      Sys_Settings.ScreenWidth          = (uint16_t)atoi(val);
        else if (!strcmp(key, "ResolutionHeight"))     Sys_Settings.ScreenHeight         = (uint16_t)atoi(val);
        else if (!strcmp(key, "Fullscreen"))           Sys_Settings.Fullscreen           = (uint8_t)atoi(val);
        else if (!strcmp(key, "FOV"))                  Sys_Settings.FOV                  = (uint8_t)atoi(val);
        else if (!strcmp(key, "Brightness"))           Sys_Settings.Brightness           = (uint8_t)atoi(val);
        else if (!strcmp(key, "Gamma"))                Sys_Settings.Gamma                = (uint8_t)atoi(val);
        else if (!strcmp(key, "AA"))                   Sys_Settings.AntiAliasing         = (uint8_t)atoi(val);
        else if (!strcmp(key, "Shadows"))              Sys_Settings.Shadows              = (uint8_t)atoi(val);
        else if (!strcmp(key, "SSR"))                  Sys_Settings.Reflections          = (uint8_t)atoi(val);
        else if (!strcmp(key, "VSync"))                Sys_Settings.Vsync                = (uint8_t)atoi(val);
        else if (!strcmp(key, "ModelDetail"))          Sys_Settings.ModelDetail          = (uint8_t)atoi(val);
        else if (!strcmp(key, "GI"))                   Sys_Settings.GI                   = (uint8_t)atoi(val);
        else if (!strcmp(key, "SpeakerMode"))          Sys_Settings.SpeakerMode          = (uint8_t)atoi(val);
        else if (!strcmp(key, "Reverb"))               Sys_Settings.Reverb               = (uint8_t)atoi(val);
        else if (!strcmp(key, "VolumeMaster"))         Sys_Settings.VolumeMaster         = (uint8_t)atoi(val);
        else if (!strcmp(key, "VolumeMusic"))          Sys_Settings.VolumeMusic          = (uint8_t)atoi(val);
        else if (!strcmp(key, "VolumeMessage"))        Sys_Settings.VolumeMessage        = (uint8_t)atoi(val);
        else if (!strcmp(key, "VolumeEffects"))        Sys_Settings.VolumeEffects        = (uint8_t)atoi(val);
        else if (!strcmp(key, "Language"))             Sys_Settings.Language             = (uint8_t)atoi(val);
        else if (!strcmp(key, "DynamicMusic"))         Sys_Settings.DynamicMusic         = (uint8_t)atoi(val);
        else if (!strcmp(key, "Footsteps"))            Sys_Settings.Footsteps            = (uint8_t)atoi(val);
        else if (!strcmp(key, "InvertLook"))           Sys_Settings.InvertLook           = (uint8_t)atoi(val);
        else if (!strcmp(key, "InvertCyberspaceLook")) Sys_Settings.InvertCyberspaceLook = (uint8_t)atoi(val);
        else if (!strcmp(key, "QuickItemPickup"))      Sys_Settings.QuickItemPickup      = (uint8_t)atoi(val);
        else if (!strcmp(key, "QuickReloadWeapons"))   Sys_Settings.QuickReloadWeapons   = (uint8_t)atoi(val);
        else if (!strcmp(key, "MouseSensitivity"))     Sys_Settings.MouseSensitivity     = (uint8_t)atoi(val);
        else if (!strcmp(key, "NoShootMode"))          Sys_Settings.NoShootMode          = (uint8_t)atoi(val);
        else if (!strcmp(key, "HeadBob"))              Sys_Settings.HeadBob              = (uint8_t)atoi(val);
        else if (!strcmp(key, "Forward"))              Sys_Settings.InputCodeSettings[0]  = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Strafe Left"))          Sys_Settings.InputCodeSettings[1]  = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Backpedal"))            Sys_Settings.InputCodeSettings[2]  = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Strafe Right"))         Sys_Settings.InputCodeSettings[3]  = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Jump"))                 Sys_Settings.InputCodeSettings[4]  = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Crouch"))               Sys_Settings.InputCodeSettings[5]  = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Prone"))                Sys_Settings.InputCodeSettings[6]  = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Lean Left"))            Sys_Settings.InputCodeSettings[7]  = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Lean Right"))           Sys_Settings.InputCodeSettings[8]  = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Sprint"))               Sys_Settings.InputCodeSettings[9]  = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Turn Left"))            Sys_Settings.InputCodeSettings[10] = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Turn Right"))           Sys_Settings.InputCodeSettings[11] = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Look Up"))              Sys_Settings.InputCodeSettings[12] = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Look Down"))            Sys_Settings.InputCodeSettings[13] = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Recent Log"))           Sys_Settings.InputCodeSettings[14] = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Biomonitor"))           Sys_Settings.InputCodeSettings[15] = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Sensaround"))           Sys_Settings.InputCodeSettings[16] = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Lantern"))              Sys_Settings.InputCodeSettings[17] = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Shield"))               Sys_Settings.InputCodeSettings[18] = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Infrared"))             Sys_Settings.InputCodeSettings[19] = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "E-reader"))             Sys_Settings.InputCodeSettings[20] = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Booster"))              Sys_Settings.InputCodeSettings[21] = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Jumpjets"))             Sys_Settings.InputCodeSettings[22] = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Attack"))               Sys_Settings.InputCodeSettings[23] = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Use"))                  Sys_Settings.InputCodeSettings[24] = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Menu/Back"))            Sys_Settings.InputCodeSettings[25] = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Toggle Mode"))          Sys_Settings.InputCodeSettings[26] = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Reload"))               Sys_Settings.InputCodeSettings[27] = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Weapon +"))             Sys_Settings.InputCodeSettings[28] = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Weapon -"))             Sys_Settings.InputCodeSettings[29] = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Grenade"))              Sys_Settings.InputCodeSettings[30] = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Grenade +"))            Sys_Settings.InputCodeSettings[31] = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Grenade -"))            Sys_Settings.InputCodeSettings[32] = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Ammo Type"))            Sys_Settings.InputCodeSettings[33] = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Patch Use"))            Sys_Settings.InputCodeSettings[34] = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Patch +"))              Sys_Settings.InputCodeSettings[35] = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Patch -"))              Sys_Settings.InputCodeSettings[36] = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Full Map"))             Sys_Settings.InputCodeSettings[37] = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Swim Up"))              Sys_Settings.InputCodeSettings[38] = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Swim Down"))            Sys_Settings.InputCodeSettings[39] = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Toggle Console"))       Sys_Settings.InputCodeSettings[40] = GetGLFWIndirectionIndexForAnInput(val);
        else if (!strcmp(key, "Screenshot"))           Sys_Settings.InputCodeSettings[41] = GetGLFWIndirectionIndexForAnInput(val);
    }

    fclose(f);
}

void SaveConfig(void) {
    FILE* f = fopen("./Data/Config.ini", "w");
    if (!f) { DualLogError("Unable to save ./Data/Config.ini!\n"); return; }

    fprintf(f, "ResolutionWidth = %u\n",      Sys_Settings.ScreenWidth);
    fprintf(f, "ResolutionHeight = %u\n",     Sys_Settings.ScreenHeight);
    fprintf(f, "Fullscreen = %u\n",           Sys_Settings.Fullscreen);
    fprintf(f, "FOV = %u\n",                  Sys_Settings.FOV);
    fprintf(f, "Brightness = %u\n",           Sys_Settings.Brightness);
    fprintf(f, "Gamma = %u\n",                Sys_Settings.Gamma);
    fprintf(f, "AA = %u\n",                   Sys_Settings.AntiAliasing);
    fprintf(f, "Shadows = %u\n",              Sys_Settings.Shadows);
    fprintf(f, "SSR = %u\n",                  Sys_Settings.Reflections);
    fprintf(f, "VSync = %u\n",                Sys_Settings.Vsync);
    fprintf(f, "ModelDetail = %u\n",          Sys_Settings.ModelDetail);
    fprintf(f, "GI = %u\n",                   Sys_Settings.GI);
    fprintf(f, "SpeakerMode = %u\n",          Sys_Settings.SpeakerMode);
    fprintf(f, "Reverb = %u\n",               Sys_Settings.Reverb);
    fprintf(f, "VolumeMaster = %u\n",         Sys_Settings.VolumeMaster);
    fprintf(f, "VolumeMusic = %u\n",          Sys_Settings.VolumeMusic);
    fprintf(f, "VolumeMessage = %u\n",        Sys_Settings.VolumeMessage);
    fprintf(f, "VolumeEffects = %u\n",        Sys_Settings.VolumeEffects);
    fprintf(f, "Language = %u\n",             Sys_Settings.Language);
    fprintf(f, "DynamicMusic = %u\n",         Sys_Settings.DynamicMusic);
    fprintf(f, "Footsteps = %u\n",            Sys_Settings.Footsteps);
    fprintf(f, "InvertLook = %u\n",           Sys_Settings.InvertLook);
    fprintf(f, "InvertCyberspaceLook = %u\n", Sys_Settings.InvertCyberspaceLook);
    fprintf(f, "QuickItemPickup = %u\n",      Sys_Settings.QuickItemPickup);
    fprintf(f, "QuickReloadWeapons = %u\n",   Sys_Settings.QuickReloadWeapons);
    fprintf(f, "MouseSensitivity = %u\n",     Sys_Settings.MouseSensitivity);
    fprintf(f, "NoShootMode = %u\n",          Sys_Settings.NoShootMode);
    fprintf(f, "HeadBob = %u\n",              Sys_Settings.HeadBob);
    fprintf(f, "Forward = %s\n",            inputElements[Sys_Settings.InputCodeSettings[0]].name);
    fprintf(f, "Strafe Left = %s\n",        inputElements[Sys_Settings.InputCodeSettings[1]].name);
    fprintf(f, "Backpedal = %s\n",          inputElements[Sys_Settings.InputCodeSettings[2]].name);
    fprintf(f, "Strafe Right = %s\n",       inputElements[Sys_Settings.InputCodeSettings[3]].name);
    fprintf(f, "Jump = %s\n",               inputElements[Sys_Settings.InputCodeSettings[4]].name);
    fprintf(f, "Crouch = %s\n",             inputElements[Sys_Settings.InputCodeSettings[5]].name);
    fprintf(f, "Prone = %s\n",              inputElements[Sys_Settings.InputCodeSettings[6]].name);
    fprintf(f, "Lean Left = %s\n",          inputElements[Sys_Settings.InputCodeSettings[7]].name);
    fprintf(f, "Lean Right = %s\n",         inputElements[Sys_Settings.InputCodeSettings[8]].name);
    fprintf(f, "Sprint = %s\n",             inputElements[Sys_Settings.InputCodeSettings[9]].name);
    fprintf(f, "Turn Left = %s\n",          inputElements[Sys_Settings.InputCodeSettings[10]].name);
    fprintf(f, "Turn Right = %s\n",         inputElements[Sys_Settings.InputCodeSettings[11]].name);
    fprintf(f, "Look Up = %s\n",            inputElements[Sys_Settings.InputCodeSettings[12]].name);
    fprintf(f, "Look Down = %s\n",          inputElements[Sys_Settings.InputCodeSettings[13]].name);
    fprintf(f, "Recent Log = %s\n",         inputElements[Sys_Settings.InputCodeSettings[14]].name);
    fprintf(f, "Biomonitor = %s\n",         inputElements[Sys_Settings.InputCodeSettings[15]].name);
    fprintf(f, "Sensaround = %s\n",         inputElements[Sys_Settings.InputCodeSettings[16]].name);
    fprintf(f, "Lantern = %s\n",            inputElements[Sys_Settings.InputCodeSettings[17]].name);
    fprintf(f, "Shield = %s\n",             inputElements[Sys_Settings.InputCodeSettings[18]].name);
    fprintf(f, "Infrared = %s\n",           inputElements[Sys_Settings.InputCodeSettings[19]].name);
    fprintf(f, "E-reader = %s\n",           inputElements[Sys_Settings.InputCodeSettings[20]].name);
    fprintf(f, "Booster = %s\n",            inputElements[Sys_Settings.InputCodeSettings[21]].name);
    fprintf(f, "Jumpjets = %s\n",           inputElements[Sys_Settings.InputCodeSettings[22]].name);
    fprintf(f, "Attack = %s\n",             inputElements[Sys_Settings.InputCodeSettings[23]].name);
    fprintf(f, "Use = %s\n",                inputElements[Sys_Settings.InputCodeSettings[24]].name);
    fprintf(f, "Menu/Back = %s\n",          inputElements[Sys_Settings.InputCodeSettings[25]].name);
    fprintf(f, "Toggle Mode = %s\n",        inputElements[Sys_Settings.InputCodeSettings[26]].name);
    fprintf(f, "Reload = %s\n",             inputElements[Sys_Settings.InputCodeSettings[27]].name);
    fprintf(f, "Weapon + = %s\n",           inputElements[Sys_Settings.InputCodeSettings[28]].name);
    fprintf(f, "Weapon - = %s\n",           inputElements[Sys_Settings.InputCodeSettings[29]].name);
    fprintf(f, "Grenade = %s\n",            inputElements[Sys_Settings.InputCodeSettings[30]].name);
    fprintf(f, "Grenade + = %s\n",          inputElements[Sys_Settings.InputCodeSettings[31]].name);
    fprintf(f, "Grenade - = %s\n",          inputElements[Sys_Settings.InputCodeSettings[32]].name);
    fprintf(f, "Ammo Type = %s\n",          inputElements[Sys_Settings.InputCodeSettings[33]].name);
    fprintf(f, "Patch Use = %s\n",          inputElements[Sys_Settings.InputCodeSettings[34]].name);
    fprintf(f, "Patch + = %s\n",            inputElements[Sys_Settings.InputCodeSettings[35]].name);
    fprintf(f, "Patch - = %s\n",            inputElements[Sys_Settings.InputCodeSettings[36]].name);
    fprintf(f, "Full Map = %s\n",           inputElements[Sys_Settings.InputCodeSettings[37]].name);
    fprintf(f, "Swim Up = %s\n",            inputElements[Sys_Settings.InputCodeSettings[38]].name);
    fprintf(f, "Swim Down = %s\n",          inputElements[Sys_Settings.InputCodeSettings[39]].name);
    fprintf(f, "Toggle Console = %s\n",     inputElements[Sys_Settings.InputCodeSettings[40]].name);
    fprintf(f, "Screenshot = %s\n",         inputElements[Sys_Settings.InputCodeSettings[41]].name);
    fclose(f);
}

void SetSkyRotateSpeed(void) {
    static const float speeds[] = { 0.05f, 1.0f, 2.5f, 3.75f, 6.25f };
    float skyRotateSpeed = speeds[Sys_Cheats.dizzyLevel];
    glUseProgram(Sys_Render.imageBlitShaderProgram);
    glUniform1f(30, skyRotateSpeed);
}

void SetFog(void) {
    glUseProgram(Sys_Render.chunkShaderProgram);
    glUniform3f(4, fogColorR * fogBaseDensityForLevel, fogColorG * fogBaseDensityForLevel, fogColorB * fogBaseDensityForLevel); // TODO: Add gunsmoke accumulation
}

void SetVSync(void) { glfwSwapInterval((int32_t)Sys_Settings.Vsync); }

void UpdateProjectionMatrices(void) {
    float* m;
    m = uiOrthoProjection;
    m[0] = 2.0f / (float)Sys_Settings.ScreenWidth; m[1] =                                       0.0f; m[2] =  0.0f; m[3] = 0.0f;
    m[4] =                                   0.0f; m[5] = -2.0f / ((float)Sys_Settings.ScreenHeight); m[6] =  0.0f; m[7] = 0.0f;
    m[8] =                                   0.0f; m[9] =                                       0.0f; m[10]= -1.0f; m[11]= 0.0f;
    m[12]=                                  -1.0f; m[13]=                                       1.0f; m[14]=  0.0f; m[15]= 1.0f;
    
    aspect3D = (float)Sys_Settings.ScreenWidth / (float)Sys_Settings.ScreenHeight;
    float f = vcot((float)Sys_Settings.FOV * PI / 360.0f);
    m = rasterPerspectiveProjection;
    m[0] = f / aspect3D; m[1] = 0.0f; m[2] =                                                      0.0f; m[3] =  0.0f;
    m[4] =         0.0f; m[5] =    f; m[6] =                                                      0.0f; m[7] =  0.0f;
    m[8] =         0.0f; m[9] = 0.0f; m[10]=      -(FAR_PLANE + NEAR_PLANE) / (FAR_PLANE - NEAR_PLANE); m[11]= -1.0f;
    m[12]=         0.0f; m[13]= 0.0f; m[14]= -2.0f * FAR_PLANE * NEAR_PLANE / (FAR_PLANE - NEAR_PLANE); m[15]=  0.0f;
    voxen_Shadow_System.shadDotThresh = 1.0f / vsqrtf(1.0f + vtan((float)Sys_Settings.FOV * PI / 360.0f) * (1.0f + aspect3D * aspect3D));
}

void GenerateAndBindTexture(GLuint *id, GLint internalFormat, int32_t width, int32_t height, GLenum format, GLenum type, GLenum target) {
    if (*id == 0) glGenTextures(1, id);
    glBindTexture(target, *id);
    glTexImage2D(target, 0, internalFormat, width, height, 0, format, type, NULL);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void UpdateScreenSize(GLFWwindow* window, int32_t width, int32_t height) {
    Sys_Settings.ScreenWidth = vmax(vmin((uint16_t)width, 7680), 320u); Sys_Settings.ScreenHeight = vmax(vmin((uint16_t)height, 4320), 200u); // Cap at minimum Quake 1 resolution and maximum 8k.
    DualLog("Screen size updated to %u x %u from input values %d x %d\n", Sys_Settings.ScreenWidth, Sys_Settings.ScreenHeight, width, height);
    glViewport(0, 0, Sys_Settings.ScreenWidth, Sys_Settings.ScreenHeight);
    UpdateProjectionMatrices();
    glUseProgram(Sys_Render.imageBlitShaderProgram);
    glUniform1ui(2, Sys_Settings.ScreenWidth);
    glUniform1ui(3, Sys_Settings.ScreenHeight);
    glUniform1i(26, Sys_Settings.SSR_RES);
    glUseProgram(Sys_Render.chunkShaderProgram);
    glUniform1ui(6, Sys_Settings.ScreenWidth);
    glUniform1ui(7, Sys_Settings.ScreenHeight);
    glUseProgram(Sys_Render.ssrShaderProgram);
    glUniform1ui(0, Sys_Settings.ScreenWidth / Sys_Settings.SSR_RES);
    glUniform1ui(1, Sys_Settings.ScreenHeight / Sys_Settings.SSR_RES);       
    glUniform1i(2, Sys_Settings.SSR_RES);
    GenerateAndBindTexture(&Sys_Render.inputImageID,             GL_RGBA8, Sys_Settings.ScreenWidth, Sys_Settings.ScreenHeight,            GL_RGBA, GL_UNSIGNED_BYTE, GL_TEXTURE_2D); // Lit Raster
    GenerateAndBindTexture(&Sys_Render.inputWorldPosID,        GL_RGBA16F, Sys_Settings.ScreenWidth, Sys_Settings.ScreenHeight,            GL_RGBA,         GL_FLOAT, GL_TEXTURE_2D); // Raster World Positions
    GenerateAndBindTexture(&Sys_Render.inputDepthID, GL_DEPTH_COMPONENT32, Sys_Settings.ScreenWidth, Sys_Settings.ScreenHeight, GL_DEPTH_COMPONENT,         GL_FLOAT, GL_TEXTURE_2D); // Raster Depth
    GenerateAndBindTexture(&Sys_Render.inputSpecID,              GL_RGBA8, Sys_Settings.ScreenWidth, Sys_Settings.ScreenHeight,            GL_RGBA, GL_UNSIGNED_BYTE, GL_TEXTURE_2D); // Specular Colors
    GenerateAndBindTexture(&Sys_Render.inputNormalID,            GL_RG16F, Sys_Settings.ScreenWidth, Sys_Settings.ScreenHeight,              GL_RG,         GL_FLOAT, GL_TEXTURE_2D); // Normal XYZ
    glBindFramebuffer(GL_FRAMEBUFFER, Sys_Render.gBufferFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Sys_Render.inputImageID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, Sys_Render.inputWorldPosID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, Sys_Render.inputSpecID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, Sys_Render.inputNormalID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, Sys_Render.inputDepthID, 0);
    glBindImageTexture(0, Sys_Render.inputImageID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8); // Main Rendered Color
    glBindImageTexture(1, Sys_Render.inputWorldPosID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F); // World Position XYZ
    glBindImageTexture(2, Sys_Render.inputSpecID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8); // Specular
    //                 3 = depth
    glBindImageTexture(4, Sys_Render.outputImageID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8); // SSR result
    glBindImageTexture(5, Sys_Render.inputNormalID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RG16F); // Normal XYZ
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, Sys_Render.outputImageID);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glGenTextures(1, &Sys_Render.outputImageID);
    glBindTexture(GL_TEXTURE_2D, Sys_Render.outputImageID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,  Sys_Settings.ScreenWidth / Sys_Settings.SSR_RES,  Sys_Settings.ScreenHeight / Sys_Settings.SSR_RES, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
}
#pragma GCC diagnostic pop

void SetGI(void) {
    if (Sys_Settings.GI) {
        // TODO: Set needed Voxel GI uniforms
    }
}

void SetSpeakerMode(void) {
    switch (Sys_Settings.SpeakerMode) {
        case 0: break;//targetMode = AudioSpeakerMode.Mono; break;
        case 1: break;//targetMode = AudioSpeakerMode.Stereo; break;
        case 2: break;//targetMode = AudioSpeakerMode.Quad; break;
        case 3: break;//targetMode = AudioSpeakerMode.Surround; break;
        case 4: break;//targetMode = AudioSpeakerMode.Mode5point1; break;
        case 5: break;//targetMode = AudioSpeakerMode.Mode7point1; break;
        case 6: break;//targetMode = AudioSpeakerMode.Prologic; break;
    }
}

void SetLanguage(void) {
    LoadTextForLanguage(Sys_Settings.Language);
    LoadLogTextForLanguage(Sys_Settings.Language);
}

void ApplySettings(void) {
    UpdateScreenSize(Sys_Global.window, Sys_Settings.ScreenWidth, Sys_Settings.ScreenHeight);
    SetSkyRotateSpeed();
    SetVSync();
    SetFog();
    SetGI();
    SetSpeakerMode();
    SetLanguage();
    glUseProgram(Sys_Render.imageBlitShaderProgram);
    glUniform1ui(5, Sys_Settings.Reflections);
    glUniform1ui(6, Sys_Settings.AntiAliasing);
    glUniform1f(14, Sys_Settings.FOV);
    glUniform1f(16, (float)Sys_Settings.ScreenWidth / (float)Sys_Settings.ScreenHeight);
    glUniform1ui(22, Sys_Settings.Shadows);
    glUseProgram(Sys_Render.chunkShaderProgram);
    glUniform1ui(14, Sys_Settings.Reflections);   glUniform1ui(15, Sys_Settings.Shadows);
    DualLog("Applied configuration settings\n");
    // TODO: Render config view on the menu
}

// GLFW Callbacks
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
static void key_callback(GLFWwindow* window, int32_t key, int32_t scancode, int32_t action, int32_t mods) {
    if (key == GLFW_KEY_F10 && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        if (log_playback) { log_playback = false; DualLog("Exited log playback manually.  Control returned\n"); return; }
        else OS_Exit(0);
    } else {
        if (!log_playback) {
            if (action == GLFW_PRESS || action == GLFW_REPEAT) EnqueueEvent(EV_KEYDOWN, key, EV_INT_FIELD_UNUSED, EV_FLOAT_FIELD_UNUSED, EV_FLOAT_FIELD_UNUSED);
            else if (action == GLFW_RELEASE) EnqueueEvent(EV_KEYUP, key, EV_INT_FIELD_UNUSED, EV_FLOAT_FIELD_UNUSED, EV_FLOAT_FIELD_UNUSED);
        }
    }
}

void Input_PollJoysticks(void) {
    for (int jid = GLFW_JOYSTICK_1; jid <= GLFW_JOYSTICK_LAST; ++jid) {
        if (!glfwJoystickPresent(jid)) continue;

        int buttonCount;
        const unsigned char* buttons = glfwGetJoystickButtons(jid, &buttonCount);
        for (int i = 0; i < buttonCount && i < MAX_JOYSTICK_BUTTONS; ++i) {
            KeyState* k = &Sys_Input.joystickButtons[GLFW_JOYSTICK_1][i];
            bool down = buttons[i] == GLFW_PRESS;

            k->pressed  = down && !k->down;
            k->released = !down && k->down;
            k->down     = down;
        }

        int hatCount;
        const unsigned char* hats = glfwGetJoystickHats(jid, &hatCount);
        for (int i = 0; i < hatCount && i < MAX_JOYSTICK_HATS; ++i) Sys_Input.joystickHats[i].down = hats[i];
    }
}

void Input_PollGamepad(void) {
    GLFWgamepadstate s;
    if (!glfwGetGamepadState(GLFW_JOYSTICK_1, &s)) return;

    for (int i = 0; i < GLFW_GAMEPAD_BUTTON_LAST + 1; ++i) {
        KeyState* k = &Sys_Input.gamepadButtons[i];
        bool down = s.buttons[i] == GLFW_PRESS;

        k->pressed  = down && !k->down;
        k->released = !down && k->down;
        k->down     = down;
    }
}

static void joystick_callback(int32_t jid, int32_t event) {
    if (jid > GLFW_JOYSTICK_LAST) return;
    
    if (event == GLFW_CONNECTED) {
        memset(&Sys_Input.joystickPresent[jid], 1, sizeof(bool));
    } else if (event == GLFW_DISCONNECTED) {
        memset(&Sys_Input.joystickPresent[jid], 0, sizeof(bool));
        memset(Sys_Input.joystickButtons, 0, sizeof(Sys_Input.joystickButtons));
        memset(Sys_Input.joystickHats, 0, sizeof(Sys_Input.joystickHats));
    }
}

static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!log_playback && Sys_Input.window_has_focus) {
        int32_t dx = (int32_t)(xpos - Sys_Input.last_mouse_x);
        int32_t dy = (int32_t)(ypos - Sys_Input.last_mouse_y);
        Sys_Input.last_mouse_x = xpos;
        Sys_Input.last_mouse_y = ypos;
        if (Sys_Input.ignore_next_mouse_delta) { Sys_Input.ignore_next_mouse_delta = false; return; }
        
        if (Sys_Dx.globalFrameNum > 1) EnqueueEvent(EV_MOUSEMOVE, dx, dy, EV_FLOAT_FIELD_UNUSED, EV_FLOAT_FIELD_UNUSED);
    }
}

static void window_focus_callback(GLFWwindow* window, int32_t focused) {
    Sys_Input.window_has_focus = focused != 0;
    Sys_Input.ignore_next_mouse_delta = true;
    glfwSetInputMode(window, GLFW_CURSOR, Sys_Input.window_has_focus ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

static void mouse_button_callback(GLFWwindow* window, int32_t button, int32_t action, int32_t mods) {
    if (button < 0 || button >= MAX_MOUSE_BUTTONS) return;
    if (action == GLFW_PRESS) {
        Sys_Input.mouseButtons[button].down = true;
        Sys_Input.mouseButtons[button].pressed = true;
        EnqueueEvent(EV_KEYDOWN, button + 1000, EV_INT_FIELD_UNUSED, EV_FLOAT_FIELD_UNUSED, EV_FLOAT_FIELD_UNUSED); // offset mouse events if needed
    } else if (action == GLFW_RELEASE) {
        Sys_Input.mouseButtons[button].down = false;
        Sys_Input.mouseButtons[button].released = true;
        EnqueueEvent(EV_KEYUP, button + 1000, EV_INT_FIELD_UNUSED, EV_FLOAT_FIELD_UNUSED, EV_FLOAT_FIELD_UNUSED);
    }
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) { Sys_Input.scrollDelta += yoffset; }
#pragma GCC diagnostic pop

void Input_Init(GLFWwindow* window) {
    glfwSetKeyCallback(window, key_callback);
    glfwSetJoystickCallback(joystick_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetWindowFocusCallback(window, window_focus_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    Sys_Input.isCapsLockOn = false; // As far as we're concerned, don't worry about OS state.
    Sys_Input.lastUse = false;
}

int32_t Input_KeyDown(int32_t keycode) {
    if (keycode >= 0 && keycode < MAX_KEYS) Sys_Input.keyStates[keycode].pressed = Sys_Input.keyStates[keycode].down = true;
    if (Sys_Cheats.consoleActive) { ConsoleEmulator(keycode); return 0; }
    return 0;
}

int32_t Input_KeyUp(int32_t keycode) {
    if (keycode >= 0 && keycode < MAX_KEYS) Sys_Input.keyStates[keycode].pressed = Sys_Input.keyStates[keycode].down = false;
    return 0;
}

void InputClearRisingAndFallingEdges(void) { // Clear keypress rising and falling edge triggers
    for (int32_t i=0;i<MAX_KEYS;++i)          Sys_Input.keyStates[i].pressed = Sys_Input.keyStates[i].released = false;       // Can't memset as we want to preserve down state
    for (int32_t i=0;i<MAX_MOUSE_BUTTONS;i++) Sys_Input.mouseButtons[i].pressed = Sys_Input.mouseButtons[i].released = false; // Can't memset as we want to preserve down state
    Sys_Input.scrollDelta = 0;
}

void UpdatePlayerFacingAngles(void) {
    Quaternion rot = instances[PLAYER1].rotation;
    float y2 = rot.y * rot.y;  float xz = rot.x * rot.z;  float wy = rot.w * rot.y;
    instances[PLAYER1].forward = normalize_vector3((Vector3){ 2.0f * (xz + wy), 2.0f * (rot.y * rot.z - rot.w * rot.x), 1.0f - 2.0f * (rot.x * rot.x + y2) });
    instances[PLAYER1].right = normalize_vector3((Vector3){ 1.0f - 2.0f * (y2 + rot.z * rot.z), 2.0f * (rot.x * rot.y + rot.w * rot.z), 2.0f * (xz - wy) });
}

// Create a quaternion from yaw (around Y), pitch (around X), and roll (around Z) in degrees
void quat_from_yaw_pitch_roll(Quaternion* q, float yaw_deg, float pitch_deg, float roll_deg) {
    float yaw = deg2rad(yaw_deg);   // Around Y (up)
    float pitch = deg2rad(pitch_deg); // Around X (right)
    float roll = deg2rad(roll_deg);  // Around Z (forward)
    float cy = vcosf(yaw * 0.5f);
    float sy = vsinf(yaw * 0.5f);
    float cp = vcosf(pitch * 0.5f);
    float sp = vsinf(pitch * 0.5f);
    float cr = vcosf(roll * 0.5f);
    float sr = vsinf(roll * 0.5f);
    q->w = cy * cp * cr + sy * sp * sr;
    q->x = cy * sp * cr + sy * cp * sr; // X-axis (pitch)
    q->y = sy * cp * cr - cy * sp * sr; // Y-axis (yaw)
    q->z = cy * cp * sr - sy * sp * cr; // Z-axis (roll)
} // Skipping quat normalization, not needed

void Input_MouselookApply(void) {
    if (Sys_Global.currentLevel == LEVEL_CYBERSPACE) quat_from_yaw_pitch_roll(&instances[PLAYER1].rotation,cam_yaw,cam_pitch,cam_roll);
    else               quat_from_yaw_pitch_roll(&instances[PLAYER1].rotation,cam_yaw,cam_pitch,    0.0f);
    
    UpdatePlayerFacingAngles();
}

int32_t Input_MouseMove(int32_t xrel, int32_t yrel) {
    if (CursorVisible()) {
        int32_t newX = cursorPosition_x + xrel;
        if (newX > Sys_Settings.ScreenWidth) newX = Sys_Settings.ScreenWidth;
        if (newX < 0) newX = 0;
        cursorPosition_x = newX;
        int32_t newY = cursorPosition_y + yrel;
        if (newY > Sys_Settings.ScreenHeight) newY = Sys_Settings.ScreenHeight;
        if (newY < 0) newY = 0;
        cursorPosition_y = newY;
    }
    
    if (Sys_Global.gamePaused || Sys_Global.inventoryMode) return 0;
    
    float sensitivity = clampf((float)Sys_Settings.MouseSensitivity / 100.0f, 0.01f, 1.0f) * 0.2f;
    cam_yaw += (float)xrel * sensitivity;
    if (cam_yaw >= 360.0f) cam_yaw -= 360.0f;
    if (cam_yaw < 0.0f) cam_yaw += 360.0f;
    cam_pitch += (float)yrel * sensitivity;
    if (cam_pitch > 89.0f) cam_pitch = 89.0f; // Avoid gimbal lock at pure 90deg
    if (cam_pitch < -89.0f) cam_pitch = -89.0f;
    Input_MouselookApply();
    return 0;
}

KeyState GetCodeMapping(int settingIndex) {
    int32_t i = Sys_Settings.InputCodeSettings[settingIndex]; // Get table index into all recognized inputs
    if (i == 148) return (KeyState){ .down = false, .pressed = false, .released = false }; // UNUSED NULL (e.g. setting unbound)
    
    if (i >= 53 && i <= 61) { // Pick subtable of GLFW values that were set by GLFW callbacks
        return Sys_Input.mouseButtons[inputElements[i].value];
    } else if (i >= 62 && i <= 77) {
        return Sys_Input.joystickButtons[GLFW_JOYSTICK_1][inputElements[i].value];        
    } else if ((i >= 78 && i <= 79) || (i >= 132 && i <= 133)) {
        return Sys_Input.joystickHats[inputElements[i].value];        
    }
    
    return Sys_Input.keyStates[inputElements[i].value];
}

bool GetKeyRiseEdgeOrHeld(int settingIndex, bool risingEdge) {
    int32_t i = Sys_Settings.InputCodeSettings[settingIndex]; // Get table index into all recognized inputs
         if (i == 129) return Sys_Input.scrollDelta > 0.0; // Mousewheel +
    else if (i == 130) return Sys_Input.scrollDelta < 0.0; // Mousewheel -
    
    KeyState keyOfConcern = GetCodeMapping(settingIndex);
    return risingEdge ? keyOfConcern.pressed : keyOfConcern.down;
}

bool GetKey(int settingIndex) { return GetKeyRiseEdgeOrHeld(settingIndex,false); }  // True while held down.
bool GetKeyPressed(int settingIndex) { return GetKeyRiseEdgeOrHeld(settingIndex,true); } // True 1st frame down.

bool Forward(void) {     return GetKey(0); }
bool StrafeLeft(void) {  return GetKey(1); }
bool Backpedal(void) {   return GetKey(2); }
bool StrafeRight(void) { return GetKey(3); }
bool Jump(void) {        return GetKey(4); }
bool JumpDown(void) {    return GetKeyPressed(4); }
bool Crouch(void) {      return GetKeyPressed(5); }
bool Prone(void) {       return GetKeyPressed(6); }
bool LeanLeft(void) {    return GetKey(7); }
bool LeanRight(void) {   return GetKey(8); }
bool Sprint(void) {      return GetKey(9); } // Toggle Sprint unused
bool TurnLeft(void) {    return GetKey(10); }
bool TurnRight(void) {   return GetKey(11); }
bool LookUp(void) {      return GetKey(12); }
bool LookDown(void) {    return GetKey(13); }
bool RecentLog(void) {   return GetKeyPressed(14); }
bool Biomonitor(void) {  return GetKeyPressed(15); }
bool Sensaround(void) {  return GetKeyPressed(16); }
bool Lantern(void) {     return GetKeyPressed(17); }
bool Shield(void) {      return GetKeyPressed(18); }
bool Infrared(void) {    return GetKeyPressed(19); }
bool Email(void) {       return GetKeyPressed(20); }
bool Booster(void) {     return GetKeyPressed(21); }
bool Jumpjets(void) {    return GetKeyPressed(22); }
bool Attack(void) {      return GetKeyPressed(23); }
bool Use(void) {         return GetKeyPressed(24); }
bool Menu(void) {        return GetKeyPressed(25); }
bool ToggleMode(void) {  return GetKeyPressed(26); }
bool Reload(void) {      return GetKeyPressed(27); }
bool WeaponCycUp(void) { return GetKeyPressed(28); }
bool WeaponCycDown(void){return GetKeyPressed(29); }
bool Grenade(void) {     return GetKeyPressed(30); }
bool GrenadeCycUp(void) {return GetKeyPressed(31); }
bool GrenadeCycDown(void){return GetKeyPressed(32); }
bool ChangeAmmoType(void){return GetKeyPressed(33); }
bool Patch(void) {       return GetKeyPressed(34); }
bool PatchCycUp(void) {  return GetKeyPressed(35); }
bool PatchCycDown(void) {return GetKeyPressed(36); }
bool Map(void) {         return GetKeyPressed(37); }
bool SwimUp(void) {      return GetKey(38); }
bool SwimDn(void) {      return GetKey(39); }
bool Console(void) {     return GetKeyPressed(40); }
bool TakeScreenshot(void) {  return GetKeyPressed(41); }

void ProcessInput(void) {
    Input_PollJoysticks();
    Input_PollGamepad();
    if (Sys_Input.keyStates[GLFW_KEY_CAPS_LOCK].pressed) Sys_Input.isCapsLockOn = !Sys_Input.isCapsLockOn; // Change capslock state to match keyboard having toggled.  Must always happen regardless of paused/menu.
    if (Sys_Input.keyStates[GLFW_KEY_LEFT_CONTROL].down && Sys_Input.keyStates[GLFW_KEY_B].pressed) CycleToNextMonitor(Sys_Global.window); // TODO: Remove?  Kinda handy.
    if (Console()) ToggleConsole();
    
    if (TakeScreenshot() && Sys_Global.current_time > Sys_Global.screenshotTimeout) {
        Screenshot();
        Sys_Global.screenshotTimeout = Sys_Global.current_time + 1.0; // Prevent saving more than 1 per second for sanity purposes.
    }
    
    if (Menu()) { Sys_Global.gamePaused = !Sys_Global.gamePaused; return; }
    if (!Sys_Input.window_has_focus || log_playback) return;
    if (Sys_Global.gamePaused || Sys_Cheats.consoleActive) return; // =========== PAUSE BARRIER ==================
    
    // Debug test light TODO: Remove later once player lantern is working
    uint16_t testLight = 741;
    uint16_t testLightIdx = (testLight * LIGHT_DATA_SIZE);
    Vector3 testLightPos = (Vector3){ lights[testLightIdx + LIGHT_DATA_OFFSET_POSX], lights[testLightIdx + LIGHT_DATA_OFFSET_POSY], lights[testLightIdx + LIGHT_DATA_OFFSET_POSZ] };
    float mx = 0.0f, my = 0.0f, mz = 0.0f;
    bool moveTestLight =    Sys_Input.keyStates[GLFW_KEY_1].down || Sys_Input.keyStates[GLFW_KEY_2].down
                         || Sys_Input.keyStates[GLFW_KEY_3].down || Sys_Input.keyStates[GLFW_KEY_4].down
                         || Sys_Input.keyStates[GLFW_KEY_5].down || Sys_Input.keyStates[GLFW_KEY_6].down;
    if (moveTestLight) {
        if (Sys_Input.keyStates[GLFW_KEY_1].down) mx =  0.01f;
        if (Sys_Input.keyStates[GLFW_KEY_2].down) mx = -0.01f;
        if (Sys_Input.keyStates[GLFW_KEY_3].down) my =  0.01f;
        if (Sys_Input.keyStates[GLFW_KEY_4].down) my = -0.01f;
        if (Sys_Input.keyStates[GLFW_KEY_5].down) mz =  0.01f;
        if (Sys_Input.keyStates[GLFW_KEY_6].down) mz = -0.01f;
        MoveLight(testLight, Vector3_A_plus_B(testLightPos, (Vector3){ mx, my, mz }));
    }
    
    // Debug test entity for confirming model+texture setup + collisions TODO: Remove after combing through entity types.
    if (Sys_Input.keyStates[GLFW_KEY_I].pressed) {
        editModeTestEntityDefinition++;
        if (editModeTestEntityDefinition >= entityCount) editModeTestEntityDefinition = 0u;
        Vector3 oldPos = instances[editModeSelection].position;
        Quaternion oldRot = instances[editModeSelection].rotation;
        Vector3 oldScale = instances[editModeSelection].scale;
        instances[editModeSelection] = entities[editModeTestEntityDefinition];
        instances[editModeSelection].position = oldPos;
        instances[editModeSelection].rotation = oldRot;
        instances[editModeSelection].scale = oldScale;
        #ifdef DEBUG_ENTITIES
            DualLogEntityInstance(editModeSelection);
        #endif
    }
        
    if (ToggleMode()) {
        Sys_Input.ignore_next_mouse_delta = true;
        Sys_Global.inventoryMode = !Sys_Global.inventoryMode;
        cursorPosition_x = Sys_Settings.ScreenWidth / 2;
        cursorPosition_y = Sys_Settings.ScreenHeight / 2;
    }
    
    ApplyPlayerMovements();
}
