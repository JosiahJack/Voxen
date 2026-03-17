// menu.c - The main menu and pause menu
#include "os.h"
#include "voxen.h"
#include "glfw_defines.h"
float RelX(int16_t x); float RelY(int16_t y);
__attribute__((pure)) bool CursorIsOverBounds(float startX, float endX, float startY, float endY);

#define MENUPAD        1028
#define MENUPAD_HILITE 1029
MenuPages currentMenuPage = MenuPages_FrontPage;
bool returnToPause = false, fovSliderActive = false, gammaSliderActive = false, masterVolumeSliderActive = false, musicVolumeSliderActive = false, messageVolumeSliderActive = false, sfxVolumeSliderActive = false, enteringPlayerName = false;
uint8_t currentPlayerNameLength = 0;
int8_t currentMenuItem = 0, currentMenuTab = 0, menuItemCount = 4, menuTabCount = 1;

void GoIntoGame(void) {
    Sys_Global.menuActive = Sys_Global.gamePaused = enteringPlayerName = gammaSliderActive = fovSliderActive = masterVolumeSliderActive = musicVolumeSliderActive = messageVolumeSliderActive = sfxVolumeSliderActive = returnToPause = false;
    currentMenuItem = currentMenuTab = 0; currentMenuPage = MenuPages_FrontPage;
    Sys_Global.inventoryMode = false;
    NewGame();
    PlayGameMusic();
    DualLog("Player named \"%s\" started the game!\n", Sys_Global.playerName);
}

void TextEntry(int32_t keycode) {    
    if (keycode == GLFW_KEY_U && Sys_Input.keyStates[GLFW_KEY_LEFT_CONTROL].down) { Sys_Global.playerName[0] = '\0'; currentPlayerNameLength = 0; return; } // Clear the input from CTRL+u

    if (keycode >= GLFW_KEY_A && keycode <= GLFW_KEY_Z) { // Handle alphabet keys
        if (currentPlayerNameLength < (27 - 1)) { // Ensure we don't overflow the buffer
            char c = 'a' + (keycode - GLFW_KEY_A); // Map keycode to lowercase character
            Sys_Global.playerName[currentPlayerNameLength] = c;
            Sys_Global.playerName[currentPlayerNameLength + 1] = '\0'; // Null-terminate
            currentPlayerNameLength++;
        }
    } else if (keycode >= GLFW_KEY_1 && keycode <= GLFW_KEY_9) { // Handle number keys 1-9
        if (currentPlayerNameLength < (27 - 1)) {
            char c = '1' + (keycode - GLFW_KEY_1); // Map to '1'-'9'

            Sys_Global.playerName[currentPlayerNameLength] = c;
            Sys_Global.playerName[currentPlayerNameLength + 1] = '\0'; // Null-terminate
            currentPlayerNameLength++;
        }
    } else if (keycode == GLFW_KEY_0) { // Handle '0'
        if (currentPlayerNameLength < (27 - 1)) {
            Sys_Global.playerName[currentPlayerNameLength] = '0';
            Sys_Global.playerName[currentPlayerNameLength + 1] = '\0'; // Null-terminate
            currentPlayerNameLength++;
        }
    } else if (keycode == GLFW_KEY_BACKSPACE && currentPlayerNameLength > 0) { // Handle backspace
        currentPlayerNameLength--;
        Sys_Global.playerName[currentPlayerNameLength] = '\0'; // Null-terminate
    } else if (keycode == GLFW_KEY_SPACE) { // Handle space
        if (currentPlayerNameLength < (27 - 1)) {
            Sys_Global.playerName[currentPlayerNameLength] = ' ';
            Sys_Global.playerName[currentPlayerNameLength + 1] = '\0';
            currentPlayerNameLength++;
        }
    } else if (keycode == GLFW_KEY_ENTER || keycode == GLFW_KEY_KP_ENTER) currentMenuItem++;
}

uint8_t UI_Button(int16_t x, int16_t y, float w, float h, bool* cursorOver, int8_t this) {
    float width = RelX(w); float height = RelY(h);
    float xpos = RelX(x); float ypos = RelY(y) - height;
    bool cursorIsOver = CursorIsOverBounds(xpos, xpos + width, ypos + height, ypos);
    if (cursorIsOver && mouseMovementThisFrame && cursorOver != NULL) {
        currentMenuItem = this;
        *cursorOver = cursorIsOver;
    }
    
    if (Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_LEFT ].pressed && cursorIsOver) return 1u;
    if (Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_RIGHT].pressed && cursorIsOver) return 2u;
    return 0u;
}

void RenderUIImage(int16_t x, int16_t y, int16_t width, int16_t height, uint32_t texIndex);
bool MenuEnter(void) { return (Sys_Input.keyStates[GLFW_KEY_KP_ENTER].pressed || Sys_Input.keyStates[GLFW_KEY_ENTER].pressed || Sys_Input.gamepadButtons[GLFW_GAMEPAD_BUTTON_A].pressed); }
uint8_t UI_MenuButton(int16_t bX, int16_t bY, uint8_t menuItem, int16_t bW, int16_t bH,  int16_t tX, int16_t tY, const char* text, int16_t pX, int16_t pY) {
    bool over = false; uint8_t retvalue = 0u;
    retvalue = UI_Button(bX,bY, bW,bH, &over, menuItem);
    if (!retvalue) retvalue = (MenuEnter() && currentMenuItem == menuItem);
    over = over || currentMenuItem == menuItem;
    RenderFormattedText(tX,tY, over ? TEXT_STOPD_RED : TEXT_RED_MENU,FONT_STOPD,1.5f,text);
    RenderUIImage(pX,pY, 40,40, over ? MENUPAD_HILITE : MENUPAD); // Menu pad
    return retvalue;
}

uint8_t UI_Slider(int16_t x, int16_t y, float w, float h, bool* cursorOver, int8_t this) {
    float width = RelX(w); float height = RelY(h);
    float xpos = RelX(x); float ypos = RelY(y) - height;
    bool cursorIsOver = CursorIsOverBounds(xpos, xpos + width, ypos + height, ypos);
    if (cursorIsOver && mouseMovementThisFrame) {
        currentMenuItem = this;
        if (cursorOver != NULL) *cursorOver = cursorIsOver;
    }
    
    if (Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_LEFT ].down && cursorIsOver) return 1u;
    if (Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_RIGHT].down && cursorIsOver) return 2u;
    return 0u;
}

void UI_HeaderText(int16_t x, const char* text) {
    RenderFormattedText(x,50,TEXT_GREEN_MENU_SHADOW,FONT_STOPD,1.75f,text);
    RenderFormattedText(x,46,TEXT_GREEN_MENU_GLOW,FONT_STOPD,1.75f,text);
    RenderFormattedText(x,48,TEXT_GREEN_MENU,FONT_STOPD,1.75f,text);
}

void CycleToNextMonitor(void);

ENGINE_TO_MOD void MenuGoBack(void) {
    if (returnToPause) { returnToPause = false; Sys_Global.gamePaused = true; Sys_Global.menuActive = false; PlayGameMusic(); }
    if (currentMenuPage == MenuPages_Singleplayer || currentMenuPage == MenuPages_Multiplayer || currentMenuPage == MenuPages_Options) currentMenuPage = MenuPages_FrontPage;//News
    else if (currentMenuPage == MenuPages_Load || currentMenuPage == MenuPages_NewGame || currentMenuPage == MenuPages_IntroVideo || currentMenuPage == MenuPages_CreditsVideo) currentMenuPage = MenuPages_Singleplayer;
}

void ChangeMenuPage(MenuPages pg) { currentMenuPage = pg; currentMenuItem = currentMenuTab = 0; }

void ChangeFullScreenWindowed(void);
void ChangeResolution(void);
void RenderMenu(void) {
    if (Sys_Input.gamepadButtons[GLFW_GAMEPAD_BUTTON_B].pressed && currentMenuPage != MenuPages_FrontPage) { MenuGoBack(); return; }
    
    if (currentMenuPage != MenuPages_IntroVideo && currentMenuPage != MenuPages_CreditsVideo && currentMenuPage != MenuPages_Options) RenderUIImage(-417,-384, 2200,1536, 1026); // Menu background
    if (currentMenuPage == MenuPages_IntroVideo || currentMenuPage == MenuPages_CreditsVideo) RenderUIImage(-417,-384, 2200,1536, 0); // Video blackground
    if (currentMenuPage == MenuPages_Options) RenderUIImage(-417,-384, 2200,1536, 1032); // Menu background
    bool shiftHeld = Sys_Input.keyStates[GLFW_KEY_LEFT_SHIFT].down || Sys_Input.keyStates[GLFW_KEY_RIGHT_SHIFT].down;
    if (currentMenuPage == MenuPages_FrontPage) {
        menuItemCount = 4; menuTabCount = 1;
        RenderUIImage(282,46, 800,128, 1031); // Title CITADEL with strikethrough effect
        if (UI_MenuButton(408,340,0,574,84, 304,188,/*"SINGLEPLAYER"*/Sys_Text.stringTable[719],413,276)) ChangeMenuPage(MenuPages_Singleplayer);
        if (UI_MenuButton(408,458,1,574,84, 304,268,/*"MULTIPLAYER"*/Sys_Text.stringTable[720], 413,396)) ChangeMenuPage(MenuPages_Multiplayer);
        if (UI_MenuButton(408,582,2,574,84, 304,350,/*"OPTIONS"*/Sys_Text.stringTable[721],     413,520)) ChangeMenuPage(MenuPages_Options);
        if (UI_MenuButton(408,702,3,574,84, 304,430,/*"QUIT"*/Sys_Text.stringTable[722],        413,638)) OS_Exit(0);
    } else if (currentMenuPage == MenuPages_Singleplayer) {
        menuItemCount = 5; menuTabCount = 1;
        UI_HeaderText(250,/*"SINGLEPLAYER"*/Sys_Text.stringTable[719]);
        if (UI_MenuButton(408,340,0,574,84, 304,188,/*"CONTINUE"*/Sys_Text.stringTable[723],    413,276)) ChangeMenuPage(MenuPages_Load);
        if (UI_MenuButton(408,458,1,574,84, 304,268,/*"NEW GAME"*/Sys_Text.stringTable[741],    413,396)) ChangeMenuPage(MenuPages_NewGame);
        if (UI_MenuButton(408,582,2,574,84, 304,350,/*"PLAY INTRO"*/Sys_Text.stringTable[742],  413,520)) ChangeMenuPage(MenuPages_IntroVideo);
        if (UI_MenuButton(408,702,3,574,84, 304,430,/*"PLAY CREDITS"*/Sys_Text.stringTable[743],413,638)) ChangeMenuPage(MenuPages_CreditsVideo);
        RenderUIImage(1060,724, 84,36, 1252); // Back Button background
        bool overBack = false;        
        if (UI_Button(1060,758, 84,32, &overBack, 4) || (MenuEnter() && currentMenuItem == 4)) MenuGoBack();
        overBack = overBack || currentMenuItem == 4;
        RenderFormattedText(1076,732,overBack ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_RED_MENU,FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == MenuPages_Multiplayer) {
        menuItemCount = 1; menuTabCount = 1;
        UI_HeaderText(266,/*"MULTIPLAYER"*/Sys_Text.stringTable[720]);
        RenderUIImage(1060,724, 84,36, 1252); // Back Button background
        bool overBack = false;
        if (UI_Button(1060,758, 84,32, &overBack, 0) || (MenuEnter() && currentMenuItem == 0)) MenuGoBack();
        overBack = overBack || currentMenuItem == 0;
        RenderFormattedText(1076,732,overBack ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_RED_MENU,FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == MenuPages_Options) {
        menuTabCount = 3;
        UI_HeaderText(238,/*"CONFIGURATION"*/Sys_Text.stringTable[745]);
        if (currentMenuTab != 0) RenderUIImage(179,220, 1001,548, 1030); // Config background
        if (currentMenuTab == 0) RenderUIImage(179,220, 1001,548, 1033); // Config background graphics (empty alpha center)
        RenderUIImage(520,196, 160,30, currentMenuTab == 2 ? 920 : 921); // Config tab unhighlighted
        if (UI_Button(520,196+30, 160,30, NULL, 2)) currentMenuTab = 2;
        RenderFormattedText(530,202,currentMenuTab == 2 ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,"AUDIO / LANG");
        RenderUIImage(354,196, 160,30, currentMenuTab == 1 ? 920 : 921); // Config tab unhighlighted
        if (UI_Button(354,196+30, 160,30, NULL, 1)) currentMenuTab = 1;
        RenderFormattedText(366,202,currentMenuTab == 1 ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,"INPUT");
        RenderUIImage(190,196, 160,30, currentMenuTab == 0 ? 920 : 921); // Config tab highlighted
        if (UI_Button(190,196+30, 160,30, NULL, 0)) currentMenuTab = 0;
        RenderFormattedText(200,202,currentMenuTab == 0 ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,"GRAPHICS");
        if (currentMenuTab == 0) {
            bool overDetails = false, overAA = false, overShadows = false, overSSR = false, overVsync = false, overFOV = false, overBrightness = false, overRes = false, overFull = false, overChgM = false;
            menuItemCount = 11; // Graphics            
            RenderUIImage(200,500, 16,16, 910); // Checkbox background
            if (UI_Button(200,516, 210,16, &overDetails, 0) || (MenuEnter() && currentMenuItem == 0)) { Sys_Settings.ModelDetail = Sys_Settings.ModelDetail == 1u ? 0u : 1u; SaveConfig(); }
            overDetails = overDetails || currentMenuItem == 0;
            if (Sys_Settings.ModelDetail) RenderUIImage(202,502, 12,12, 912); // Checkbox check
            RenderFormattedText(220,500,overDetails ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,"DETAILED MODELS");
            
            RenderUIImage(200,530, 16,16, 910); // Checkbox background
            if (UI_Button(200,546, 210,16, &overAA, 1) || (MenuEnter() && currentMenuItem == 1)) { Sys_Settings.AntiAliasing = Sys_Settings.AntiAliasing == 1u ? 0u : 1u; SaveConfig(); }
            overAA = overAA || currentMenuItem == 1;
            if (Sys_Settings.AntiAliasing) RenderUIImage(202,532, 12,12, 912); // Checkbox check
            RenderFormattedText(220,530,overAA ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,"ANTIALIASING");
            
            RenderUIImage(200,560, 16,16, 910); // Checkbox background
            if (UI_Button(200,576, 210,16, &overShadows, 2) || (MenuEnter() && currentMenuItem == 2)) { Sys_Settings.Shadows = Sys_Settings.Shadows == 1u ? 0u : 1u; SaveConfig(); }
            overShadows = overShadows || currentMenuItem == 2;
            if (Sys_Settings.Shadows) RenderUIImage(202,562, 12,12, 912); // Checkbox check
            RenderFormattedText(220,560,overShadows ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,"SHADOWS");
            
            RenderUIImage(200,590, 16,16, 910); // Checkbox background
            if (UI_Button(200,606, 210,16, &overSSR, 3) || (MenuEnter() && currentMenuItem == 3)) { Sys_Settings.Reflections = Sys_Settings.Reflections == 1u ? 0u : 1u; SaveConfig(); }
            overSSR = overSSR || currentMenuItem == 3;
            if (Sys_Settings.Reflections) RenderUIImage(202,592, 12,12, 912); // Checkbox check
            RenderFormattedText(220,590,overSSR ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,"REFLECTIONS");
            
            RenderUIImage(200,620, 16,16, 910); // Checkbox background
            if (UI_Button(200,636, 210,16, &overVsync, 4) || (MenuEnter() && currentMenuItem == 4)) { Sys_Settings.Vsync = Sys_Settings.Vsync == 1u ? 0u : 1u; SetVSync(); SaveConfig(); }
            overVsync = overVsync || currentMenuItem == 4;
            if (Sys_Settings.Vsync) RenderUIImage(202,622, 12,12, 912); // Checkbox check
            RenderFormattedText(220,620,overVsync ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,"VSYNC (FPS: %d)", Sys_Global.framesPerLastSecond);
            
            // FOV Slider
            RenderUIImage(400,650, 128,16, 1079); // Slider background
            RenderUIImage(400 + (((Sys_Settings.FOV - 45.0f) / 105.0f) * 112),650, 16,16, 1078); // Slider handle [45, 150]
            if (UI_Slider(200,666, 328,16, &overFOV, 5)) fovSliderActive = true;
            if (fovSliderActive && Sys_Input.currentMouse_dx != 0) {
                int32_t new = (int32_t)Sys_Settings.FOV + vmin(vmax(Sys_Input.currentMouse_dx,-1),1); Sys_Settings.FOV = (uint8_t)vmin(vmax(new,45),150);
                UpdateProjectionMatrices();
            }
            
            if (!Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_LEFT].down && !Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_RIGHT].down) {
                if (fovSliderActive) SaveConfig();
                fovSliderActive = false;
            }
            
            if (MenuEnter() && currentMenuItem == 5) {
                if (shiftHeld) Sys_Settings.FOV = Sys_Settings.FOV <=  49 ? 150 : Sys_Settings.FOV - 5;
                else           Sys_Settings.FOV = Sys_Settings.FOV >= 146 ?  45 : Sys_Settings.FOV + 5;
                UpdateProjectionMatrices(); SaveConfig();
            }
            
            overFOV = overFOV || currentMenuItem == 5;
            RenderFormattedText(200,650,overFOV ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,"%s %u",/*Field of View*/Sys_Text.stringTable[775],Sys_Settings.FOV);
            
            // Brightness Slider
            RenderUIImage(400,680, 128,16, 1079); // Slider background
            RenderUIImage(400 + ((Sys_Settings.Brightness / 100.0f) * 112),680, 16,16, 1078); // Slider handle [45, 150]
            if (UI_Slider(200,696, 328,16, &overBrightness, 6)) gammaSliderActive = true;
            if (gammaSliderActive && Sys_Input.currentMouse_dx != 0) {
                int32_t new = (int32_t)Sys_Settings.Brightness + vmin(vmax(Sys_Input.currentMouse_dx,-1),1); Sys_Settings.Brightness = (uint8_t)vmin(vmax(new,0),100);
            }
            
            if (!Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_LEFT].down && !Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_RIGHT].down) {
                if (gammaSliderActive) SaveConfig();
                gammaSliderActive = false;
            }
            
            if (MenuEnter() && currentMenuItem == 6) {
                if (shiftHeld) Sys_Settings.Brightness = Sys_Settings.Brightness <=  1 ? 100 : Sys_Settings.Brightness - 2;
                else           Sys_Settings.Brightness = Sys_Settings.Brightness >= 99 ?   0 : Sys_Settings.Brightness + 2;
                SaveConfig();
            }
            
            overBrightness = overBrightness || currentMenuItem == 6;
            RenderFormattedText(200,680,overBrightness ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,"%s %u",/*Brightness*/Sys_Text.stringTable[771],Sys_Settings.Brightness);
            
            // Resolution
            if (UI_Button(220,726, 280,16, &overRes, 7) || (MenuEnter() && currentMenuItem == 7)) {
                ChangeResolution();
//                 SaveConfig();
            }
            
            overRes = overRes || currentMenuItem == 7;
            RenderUIImage(476,712, 16,16, overRes ? 1119 : 1077); // Dropdown caret
            RenderFormattedText(200,710,overRes ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,"%s %u x %u",/*Resolution*/Sys_Text.stringTable[771],Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight);            
            
            RenderUIImage(200,740, 16,16, 910); // Checkbox background
            if (UI_Button(200,756, 210,16, &overFull, 8) || (MenuEnter() && currentMenuItem == 8)) {
                Sys_Settings.Fullscreen = Sys_Settings.Fullscreen == 1u ? 0u : 1u;
                ChangeFullScreenWindowed();
                SaveConfig();
            }
            
            overFull = overFull || currentMenuItem == 8;
            if (Sys_Settings.Fullscreen) RenderUIImage(202,742, 12,12, 912); // Checkbox check
            RenderFormattedText(220,740,overFull ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,/*"Fullscreen"*/Sys_Text.stringTable[773]);
            
            RenderUIImage(588,730, 210,30, 1079); // Toggle monitor button background
            if (UI_Button(588,760, 210,30, &overChgM, 9) || (MenuEnter() && currentMenuItem == 9)) { CycleToNextMonitor(); }
            overChgM = overChgM || currentMenuItem == 9;
            RenderFormattedText(602,735,overChgM ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,/*"CHANGE MONITOR"*/Sys_Text.stringTable[1025]);
        } else if (currentMenuTab == 1) {
            menuItemCount = 49; // Input
        } else {
            bool overMasterVolume = false, overMusicSlider = false;
            menuItemCount = 10; // Audio / Lang
            // Master Volume Slider
            RenderUIImage(426,240, 128,16, 1079); // Slider background
            RenderUIImage(426 + ((Sys_Settings.VolumeMaster / 100.0f) * 112),240, 16,16, 1078); // Slider handle [45, 150]
            if (UI_Slider(200,256, 328,16, &overMasterVolume, 0)) masterVolumeSliderActive = true;
            if (masterVolumeSliderActive && Sys_Input.currentMouse_dx != 0) {
                int32_t new = (int32_t)Sys_Settings.VolumeMaster + vmin(vmax(Sys_Input.currentMouse_dx,-1),1); Sys_Settings.VolumeMaster = (uint8_t)vmin(vmax(new,0),100); set_master_volume();
            }
            
            if (!Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_LEFT].down && !Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_RIGHT].down) {
                if (masterVolumeSliderActive) SaveConfig();
                masterVolumeSliderActive = false;
            }
            
            if (MenuEnter() && currentMenuItem == 0) {
                if (shiftHeld) Sys_Settings.VolumeMaster = Sys_Settings.VolumeMaster <=  4 ? 100 : Sys_Settings.VolumeMaster - 5;
                else           Sys_Settings.VolumeMaster = Sys_Settings.VolumeMaster >= 96 ?   0 : Sys_Settings.VolumeMaster + 5;
                set_master_volume();
                SaveConfig();
            }
            
            overMasterVolume = overMasterVolume || currentMenuItem == 0;
            RenderFormattedText(200,240,overMasterVolume ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,"%s %u",/*Master Volume*/Sys_Text.stringTable[802],Sys_Settings.VolumeMaster);
            
            // Music Volume Slider
            RenderUIImage(426,270, 128,16, 1079); // Slider background
            RenderUIImage(426 + ((Sys_Settings.VolumeMusic / 100.0f) * 112),270, 16,16, 1078); // Slider handle [45, 150]
            if (UI_Slider(200,286, 328,16, &overMusicSlider, 1)) musicVolumeSliderActive = true;
            if (musicVolumeSliderActive && Sys_Input.currentMouse_dx != 0) {
                int32_t new = (int32_t)Sys_Settings.VolumeMusic + vmin(vmax(Sys_Input.currentMouse_dx,-1),1); Sys_Settings.VolumeMusic = (uint8_t)vmin(vmax(new,0),100); set_music_volume();
            }
            
            if (!Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_LEFT].down && !Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_RIGHT].down) {
                if (musicVolumeSliderActive) SaveConfig();
                musicVolumeSliderActive = false;
            }
            
            if (MenuEnter() && currentMenuItem == 1) {
                if (shiftHeld) Sys_Settings.VolumeMusic = Sys_Settings.VolumeMusic <=  4 ? 100 : Sys_Settings.VolumeMusic - 5;
                else           Sys_Settings.VolumeMusic = Sys_Settings.VolumeMusic >= 96 ?   0 : Sys_Settings.VolumeMusic + 5;
                set_music_volume();
                SaveConfig();
            }
            
            overMusicSlider = overMusicSlider || currentMenuItem == 1;
            RenderFormattedText(200,270,overMusicSlider ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,"%s %u",/*Music Volume*/Sys_Text.stringTable[803],Sys_Settings.VolumeMusic);
        }
        
        RenderUIImage(1087,723, 84,36, 1252); // Back Button background
        int8_t lastItem = menuItemCount - 1;
        bool overBack = false;
        if (UI_Button(1087,757, 84,32, &overBack, lastItem) || (MenuEnter() && currentMenuItem == lastItem)) MenuGoBack();
        overBack = overBack || currentMenuItem == lastItem;
        RenderFormattedText(1103,731,overBack ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_RED_MENU,FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == MenuPages_Load) {
        menuItemCount = 9; menuTabCount = 1;
        UI_HeaderText(280,/*"LOAD"*/Sys_Text.stringTable[726]);
        RenderUIImage(400,214, 586,500, 1037); // Load/Save table background
        RenderUIImage(1060,724, 84,36, 1252); // Back Button background
        bool overBack = false;
        if (UI_Button(1060,758, 84,32, &overBack, 0) || (MenuEnter() && currentMenuItem == 0)) MenuGoBack();
        overBack = overBack || currentMenuItem == 0;
        RenderFormattedText(1076,732,overBack ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_RED_MENU,FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == MenuPages_Save) {
        menuItemCount = 9; menuTabCount = 1;
        UI_HeaderText(284,/*"SAVE GAME"*/Sys_Text.stringTable[769]);
        RenderUIImage(400,214, 586,500, 1037); // Load/Save table background
        RenderUIImage(1060,724, 84,36, 1252); // Back Button background
        bool overBack = false;
        if (UI_Button(1060,758, 84,32, &overBack, 0) || (MenuEnter() && currentMenuItem == 0)) MenuGoBack();
        overBack = overBack || currentMenuItem == 0;
        RenderFormattedText(1076,732,overBack ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_RED_MENU,FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == MenuPages_NewGame) {
        menuItemCount = 7;
        menuTabCount = (currentMenuItem > 0 && currentMenuItem <= 16) ? 2 : 1;
        UI_HeaderText(290,/*"NEW GAME"*/Sys_Text.stringTable[741]);
        RenderUIImage(136,196,1088,558,1048); // Newgame inset
        RenderUIImage(136,196,1088,558,1049); // Newgame background
        if (UI_MenuButton(276,270,0,795,74, 226,146,/*"NAME:"*/Sys_Text.stringTable[746],299,214)) { /* Just for highlight */ }
        enteringPlayerName = (currentMenuItem == 0);
        if (Sys_Global.playerName[0] == '\0') RenderFormattedText(642,232,TEXT_RED_MENU,FONT_STOPD,1.0f,/*"ENTER NAME..."*/Sys_Text.stringTable[748]);
        else                                  RenderFormattedText(518,232,enteringPlayerName ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.0f,Sys_Global.playerName);

        if (UI_MenuButton(174,377,1,496,95, 148,202,/*"COMBAT"*/Sys_Text.stringTable[748],185,299)) { Sys_Global.difficultyCombat = Sys_Global.difficultyCombat >= 3 ? 0 : Sys_Global.difficultyCombat + 1; }  if (UI_MenuButton(704,377,3,496,95, 510,202,/*"MISSION"*/Sys_Text.stringTable[749],726,299)) { Sys_Global.difficultyMission = Sys_Global.difficultyMission >= 3 ? 0 : Sys_Global.difficultyMission + 1; }
        RenderFormattedText(162,270,Sys_Global.difficultyCombat == 0 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"0");    RenderFormattedText(513,270,Sys_Global.difficultyMission == 0 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"0");
        RenderFormattedText(233,270,Sys_Global.difficultyCombat == 1 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"1");    RenderFormattedText(584,270,Sys_Global.difficultyMission == 1 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"1");
        RenderFormattedText(307,270,Sys_Global.difficultyCombat == 2 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"2");    RenderFormattedText(658,270,Sys_Global.difficultyMission == 2 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"2");
        RenderFormattedText(379,270,Sys_Global.difficultyCombat == 3 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"3");    RenderFormattedText(730,270,Sys_Global.difficultyMission == 3 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"3");
        if (UI_MenuButton(174,568,2,496,92, 149,330,/*"PUZZLE"*/Sys_Text.stringTable[751],185,490)) { Sys_Global.difficultyPuzzle = Sys_Global.difficultyPuzzle >= 3 ? 0 : Sys_Global.difficultyPuzzle + 1; }  if (UI_MenuButton(704,568,4,496,92, 509,330,/*"CYBERSPACE"*/Sys_Text.stringTable[750],726,490)) { Sys_Global.difficultyCyber = Sys_Global.difficultyCyber >= 3 ? 0 : Sys_Global.difficultyCyber + 1; }
        RenderFormattedText(162,399,Sys_Global.difficultyPuzzle == 0 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"0");    RenderFormattedText(513,399,Sys_Global.difficultyCyber == 0 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"0");
        RenderFormattedText(233,399,Sys_Global.difficultyPuzzle == 1 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"1");    RenderFormattedText(584,399,Sys_Global.difficultyCyber == 1 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"1");
        RenderFormattedText(307,399,Sys_Global.difficultyPuzzle == 2 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"2");    RenderFormattedText(658,399,Sys_Global.difficultyCyber == 2 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"2");
        RenderFormattedText(379,399,Sys_Global.difficultyPuzzle == 3 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"3");    RenderFormattedText(730,399,Sys_Global.difficultyCyber == 3 ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"3");
        if (UI_Button(221,460,82,79,NULL,1)) {Sys_Global.difficultyCombat =0; currentMenuItem=1; } if (UI_Button(330,460,82,79,NULL,1)) {Sys_Global.difficultyCombat =1; currentMenuItem=1; } if (UI_Button(439,460,82,79,NULL,1)) {Sys_Global.difficultyCombat =2; currentMenuItem=1; } if (UI_Button( 547,460,82,79,NULL,1)) {Sys_Global.difficultyCombat =3; currentMenuItem=1; }
        if (UI_Button(221,651,82,79,NULL,2)) {Sys_Global.difficultyPuzzle =0; currentMenuItem=2; } if (UI_Button(330,651,82,79,NULL,2)) {Sys_Global.difficultyPuzzle =1; currentMenuItem=2; } if (UI_Button(439,651,82,79,NULL,2)) {Sys_Global.difficultyPuzzle =2; currentMenuItem=2; } if (UI_Button( 547,651,82,79,NULL,2)) {Sys_Global.difficultyPuzzle =3; currentMenuItem=2; }
        if (UI_Button(748,460,82,79,NULL,3)) {Sys_Global.difficultyMission=0; currentMenuItem=3; } if (UI_Button(857,460,82,79,NULL,3)) {Sys_Global.difficultyMission=1; currentMenuItem=3; } if (UI_Button(966,460,82,79,NULL,3)) {Sys_Global.difficultyMission=2; currentMenuItem=3; } if (UI_Button(1074,460,82,79,NULL,3)) {Sys_Global.difficultyMission=3; currentMenuItem=3; }
        if (UI_Button(748,651,82,79,NULL,4)) {Sys_Global.difficultyCyber  =0; currentMenuItem=4; } if (UI_Button(857,651,82,79,NULL,4)) {Sys_Global.difficultyCyber  =1; currentMenuItem=4; } if (UI_Button(966,651,82,79,NULL,4)) {Sys_Global.difficultyCyber  =2; currentMenuItem=4; } if (UI_Button(1074,651,82,79,NULL,4)) {Sys_Global.difficultyCyber  =3; currentMenuItem=4; }
        bool overBack = false, overStart = false;
        if (UI_Button(544,747, 282,68, &overStart, 5) || (MenuEnter() && currentMenuItem == 5)) GoIntoGame(); // TODO reload game.
        overStart = overStart || currentMenuItem == 5;
        RenderFormattedText(400,464,overStart ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,/*"START"*/Sys_Text.stringTable[886]);
        
        if (UI_Button(1060,758, 84,32, &overBack, 6) || (MenuEnter() && currentMenuItem == 6)) MenuGoBack();
        overBack = overBack || currentMenuItem == 6;
        RenderUIImage(1060,724,84,36,1252); // Back Button background
        RenderFormattedText(1076,732,overBack ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_RED_MENU,FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == MenuPages_IntroVideo) {
        menuItemCount = 1; menuTabCount = 1;
        if (MenuEnter()) MenuGoBack();
    } else if (currentMenuPage == MenuPages_CreditsVideo) {
        menuItemCount = 1; menuTabCount = 1;
        if (MenuEnter()) MenuGoBack();
    }
    
    if (menuTabCount <= currentMenuTab) currentMenuTab = 0;
    if (menuItemCount <= currentMenuItem) currentMenuItem = 0;
    if (Sys_Input.keyStates[GLFW_KEY_DOWN].pressed) currentMenuItem = (currentMenuItem + 1) >= menuItemCount ? 0 : (currentMenuItem + 1);
    if (Sys_Input.keyStates[GLFW_KEY_UP].pressed) currentMenuItem = (currentMenuItem - 1) < 0 ? (menuItemCount - 1) : (currentMenuItem - 1);
    if (Sys_Input.keyStates[GLFW_KEY_RIGHT].pressed) {
        currentMenuTab = (currentMenuTab + 1) >= menuTabCount ? 0 : (currentMenuTab + 1);
        if (currentMenuPage == MenuPages_NewGame) {
                 if (currentMenuItem == 1) currentMenuItem = 3;
            else if (currentMenuItem == 3) currentMenuItem = 1;
            else if (currentMenuItem == 2) currentMenuItem = 4;
            else if (currentMenuItem == 4) currentMenuItem = 2;
            else if (currentMenuItem == 5) currentMenuItem = 6;
            else if (currentMenuItem == 6) currentMenuItem = 5;
        }
    }
    if (Sys_Input.keyStates[GLFW_KEY_LEFT].pressed) {
        currentMenuTab =  (currentMenuTab - 1) < 0 ? (menuTabCount - 1) : (currentMenuTab - 1);
        if (currentMenuPage == MenuPages_NewGame) {
                 if (currentMenuItem == 1) currentMenuItem = 3;
            else if (currentMenuItem == 3) currentMenuItem = 1;
            else if (currentMenuItem == 2) currentMenuItem = 4;
            else if (currentMenuItem == 4) currentMenuItem = 2;
            else if (currentMenuItem == 5) currentMenuItem = 6;
            else if (currentMenuItem == 6) currentMenuItem = 5;
        }
    }
}

void RenderPausedUI(void) {
    menuItemCount = 6; menuTabCount = 1;
    bool overResume = false, overLoad /* ;) */ = false, overSave = false, overOptions = false, overQuitMenu = false, overQuit = false;
    RenderUIImage(519,276,328,300,1025); // Pause Menu background
    RenderUIImage(519,276,328,300,1080); // Pause Menu background outline
    RenderFormattedText(610,210,TEXT_STOPD_RED_PAUSETITLE,FONT_STOPD,1.0f,/*"PAUSED"*/Sys_Text.stringTable[724]);
    if (UI_Button(522,330, 322,52, &overResume, 0) || (MenuEnter() && currentMenuItem == 0)) Sys_Global.gamePaused = false;
    overResume = overResume || currentMenuItem == 0;
    RenderFormattedText(610,306,overResume ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.0f,/*"RESUME"*/Sys_Text.stringTable[725]);
    if (UI_Button(522,390, 322,52, &overLoad, 1) || (MenuEnter() && currentMenuItem == 1)) { currentMenuPage = MenuPages_Load; PlayMenuMusic(); Sys_Global.menuActive = true; returnToPause = true; }
    overLoad = overLoad || currentMenuItem == 1;
    RenderFormattedText(630,364, overLoad ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.0f,/*"LOAD"*/Sys_Text.stringTable[726]);
    if (UI_Button(522,450, 322,60, &overSave, 2) || (MenuEnter() && currentMenuItem == 2)) { currentMenuPage = MenuPages_Save; PlayMenuMusic(); Sys_Global.menuActive = true; returnToPause = true; }
    overSave = overSave || currentMenuItem == 2;
    RenderFormattedText(635,422,overSave ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.0f,/*"SAVE"*/Sys_Text.stringTable[727]);
    if (UI_Button(522,510, 322,60, &overOptions, 3) || (MenuEnter() && currentMenuItem == 3)) { currentMenuPage = MenuPages_Options; PlayMenuMusic(); Sys_Global.menuActive = true; returnToPause = true; }
    overOptions = overOptions || currentMenuItem == 3;
    RenderFormattedText(599,480,overOptions ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.0f,/*"OPTIONS"*/Sys_Text.stringTable[721]);
    if (UI_Button(522,570, 322,60, &overQuitMenu, 4) || (MenuEnter() && currentMenuItem == 4)) { PlayMenuMusic(); Sys_Global.menuActive = true; currentMenuPage = MenuPages_FrontPage; }
    overQuitMenu = overQuitMenu || currentMenuItem == 4;
    RenderFormattedText(546,538,overQuitMenu ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.0f,/*"QUIT TO MENU"*/Sys_Text.stringTable[728]);
    RenderUIImage(519,672,328,42,1252); // Pause Quit Game background
    if (UI_Button(522,714, 322,42, &overQuit, 5) || (MenuEnter() && currentMenuItem == 5)) OS_Exit(0);
    overQuit = overQuit || currentMenuItem == 5;
    RenderFormattedText(572,690,overQuit ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.0f,/*"QUIT GAME"*/Sys_Text.stringTable[729]);
    if (Sys_Input.keyStates[GLFW_KEY_DOWN].pressed) currentMenuItem = (currentMenuItem + 1) >= menuItemCount ? 0 : (currentMenuItem + 1);
    if (Sys_Input.keyStates[GLFW_KEY_UP].pressed) currentMenuItem = (currentMenuItem - 1) < 0 ? (menuItemCount - 1) : (currentMenuItem - 1);
}
