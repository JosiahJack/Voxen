// menu.c - The main menu and pause menu
#include "os.h"
#include "voxen.h"
MenuPages currentMenuPage = MenuPages_FrontPage;
bool returnToPause;
int8_t currentMenuItem = 0;
int8_t currentMenuTab = 0;
int8_t menuItemCount = 4;
int8_t menuTabCount = 1;
float RelX(int16_t x);
float RelY(int16_t y);
__attribute__((pure)) bool CursorIsOverBounds(float startX, float endX, float startY, float endY);
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

double monitorSwitchTime;
int currentMonitorIndex = 1; // Start on primary after first cycle, puts it a 0.
void CycleToNextMonitor(GLFWwindow* window) {
    if (get_time() < monitorSwitchTime) return;
    
    monitorSwitchTime = get_time() + 0.5; // Prevent toggling rapidly on accident
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

void MenuGoBack(void) {
    if (returnToPause) { returnToPause = false; Sys_Global.gamePaused = true; Sys_Global.menuActive = false; }
    if (currentMenuPage == MenuPages_Singleplayer || currentMenuPage == MenuPages_Multiplayer || currentMenuPage == MenuPages_Options) currentMenuPage = MenuPages_FrontPage;//News
    else if (currentMenuPage == MenuPages_Load || currentMenuPage == MenuPages_NewGame || currentMenuPage == MenuPages_IntroVideo || currentMenuPage == MenuPages_CreditsVideo) currentMenuPage = MenuPages_Singleplayer;
}

bool MenuEnter(void) { return (Sys_Input.keyStates[GLFW_KEY_KP_ENTER].pressed || Sys_Input.keyStates[GLFW_KEY_ENTER].pressed || Sys_Input.gamepadButtons[GLFW_GAMEPAD_BUTTON_A].pressed); }
void ChangeMenuPage(MenuPages pg) { currentMenuPage = pg; currentMenuItem = currentMenuTab = 0; }

bool fovSliderActive = false, gammaSliderActive = false;
void RenderUIImage(int16_t x, int16_t y, int16_t width, int16_t height, uint32_t texIndex);
void RenderMenu(void) {
    if (Sys_Input.gamepadButtons[GLFW_GAMEPAD_BUTTON_B].pressed && currentMenuPage != MenuPages_FrontPage) { MenuGoBack(); return; } // TODO Android Back button
    
    if (currentMenuPage != MenuPages_IntroVideo && currentMenuPage != MenuPages_CreditsVideo && currentMenuPage != MenuPages_Options) RenderUIImage(-417,-384, 2200,1536, 1026); // Menu background
    if (currentMenuPage == MenuPages_IntroVideo || currentMenuPage == MenuPages_CreditsVideo) RenderUIImage(-417,-384, 2200,1536, 0); // Video blackground
    if (currentMenuPage == MenuPages_Options) RenderUIImage(-417,-384, 2200,1536, 1032); // Menu background
    bool shiftHeld = Sys_Input.keyStates[GLFW_KEY_LEFT_SHIFT].down || Sys_Input.keyStates[GLFW_KEY_RIGHT_SHIFT].down;
    if (currentMenuPage == MenuPages_FrontPage) {
        menuItemCount = 4; menuTabCount = 1;
        RenderUIImage(282,46, 800,128, 1031); // Title CITADEL with strikethrough effect
        bool overS = false, overM = false, overO = false, overQ = false;
        if (UI_Button(408,340, 574,84, &overS, 0) || (MenuEnter() && currentMenuItem == 0)) ChangeMenuPage(MenuPages_Singleplayer);
        overS = overS || currentMenuItem == 0;
        RenderFormattedText(320,188, overS ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"SINGLEPLAYER");
        RenderUIImage(430,278, 32,32, overS ? 1029 : 1028); // Menu pad
        if (UI_Button(408,458, 574,84, &overM, 1) || (MenuEnter() && currentMenuItem == 1)) ChangeMenuPage(MenuPages_Multiplayer);
        overM = overM || currentMenuItem == 1;
        RenderFormattedText(320,268, overM ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"MULTIPLAYER");
        RenderUIImage(430,398, 32,32, overM ? 1029 : 1028); // Menu pad
        if (UI_Button(408,582, 574,84, &overO, 2) || (MenuEnter() && currentMenuItem == 2)) ChangeMenuPage(MenuPages_Options);
        overO = overO || currentMenuItem == 2;
        RenderFormattedText(320,350, overO ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"OPTIONS");
        RenderUIImage(430,522, 32,32, overO ? 1029 : 1028); // Menu pad
        if (UI_Button(408,702, 574,84, &overQ, 3) || (MenuEnter() && currentMenuItem == 3)) OS_Exit(0);
        overQ = overQ || currentMenuItem == 3;
        RenderFormattedText(320,430, overQ ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"QUIT");
        RenderUIImage(430,640, 32,32, overQ ? 1029 : 1028); // Menu pad
    } else if (currentMenuPage == MenuPages_Singleplayer) {
        menuItemCount = 5; menuTabCount = 1;
        RenderFormattedText(250,50,TEXT_GREEN_MENU_SHADOW,FONT_STOPD,1.75f,"SINGLEPLAYER");
        RenderFormattedText(250,46,TEXT_GREEN_MENU_GLOW,FONT_STOPD,1.75f,"SINGLEPLAYER");
        RenderFormattedText(250,48,TEXT_GREEN_MENU,FONT_STOPD,1.75f,"SINGLEPLAYER");
        bool overS = false, overM = false, overO = false, overQ = false, overBack = false;
        if (UI_Button(408,340, 574,84, &overS, 0) || (MenuEnter() && currentMenuItem == 0)) ChangeMenuPage(MenuPages_Load);
        overS = overS || currentMenuItem == 0;
        RenderFormattedText(320,188, overS ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"CONTINUE");
        RenderUIImage(430,278,  32,32, overS ? 1029 : 1028); // Menu pad
        if (UI_Button(408,458, 574,84, &overM, 1) || (MenuEnter() && currentMenuItem == 1)) ChangeMenuPage(MenuPages_NewGame);
        overM = overM || currentMenuItem == 1;
        RenderFormattedText(320,268, overM ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"NEW GAME");
        RenderUIImage(430,398,  32,32, overM ? 1029 : 1028); // Menu pad
        if (UI_Button(408,582, 574,84, &overO, 2) || (MenuEnter() && currentMenuItem == 2)) ChangeMenuPage(MenuPages_IntroVideo);
        overO = overO || currentMenuItem == 2;
        RenderFormattedText(320,350, overO ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"PLAY INTRO");
        RenderUIImage(430,522,  32,32, overO ? 1029 : 1028); // Menu pad
        if (UI_Button(408,702, 574,84, &overQ, 3) || (MenuEnter() && currentMenuItem == 3)) ChangeMenuPage(MenuPages_CreditsVideo);
        overQ = overQ || currentMenuItem == 3;
        RenderFormattedText(320,430, overQ ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"PLAY CREDITS");
        RenderUIImage(430,640, 32,32, overQ ? 1029 : 1028); // Menu pad
        RenderUIImage(1060,724, 84,36, 1252); // Back Button background
        if (UI_Button(1060,758, 84,32, &overBack, 4) || (MenuEnter() && currentMenuItem == 4)) MenuGoBack();
        overBack = overBack || currentMenuItem == 4;
        RenderFormattedText(1076,732,overBack ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_NORMAL,1.0f,"BACK");
    } else if (currentMenuPage == MenuPages_Multiplayer) {
        menuItemCount = 1; menuTabCount = 1;
        bool overBack = false;
        RenderFormattedText(266,50,TEXT_GREEN_MENU_SHADOW,FONT_STOPD,1.75f,"MULTIPLAYER");
        RenderFormattedText(266,46,TEXT_GREEN_MENU_GLOW,FONT_STOPD,1.75f,"MULTIPLAYER");
        RenderFormattedText(266,48,TEXT_GREEN_MENU,FONT_STOPD,1.75f,"MULTIPLAYER");
        RenderUIImage(1060,724, 84,36, 1252); // Back Button background
        if (UI_Button(1060,758, 84,32, &overBack, 0) || (MenuEnter() && currentMenuItem == 0)) MenuGoBack();
        overBack = overBack || currentMenuItem == 0;
        RenderFormattedText(1076,732,overBack ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_NORMAL,1.0f,"BACK");
    } else if (currentMenuPage == MenuPages_Options) {
        menuTabCount = 3;
        bool overBack = false;
        RenderFormattedText(238,50,TEXT_GREEN_MENU_SHADOW,FONT_STOPD,1.75f,"CONFIGURATION");
        RenderFormattedText(238,46,TEXT_GREEN_MENU_GLOW,FONT_STOPD,1.75f,"CONFIGURATION");
        RenderFormattedText(238,48,TEXT_GREEN_MENU,FONT_STOPD,1.75f,"CONFIGURATION");
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
            RenderFormattedText(220,620,overVsync ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,"VSYNC (FPS: %d)", Sys_Dx.framesPerLastSecond);
            
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
            RenderFormattedText(200,650,overFOV ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,"FIELD OF VIEW %u", Sys_Settings.FOV);
            
            // Brightness Slider
            RenderUIImage(400,680, 128,16, 1079); // Slider background
            RenderUIImage(400 + ((Sys_Settings.Brightness / 100.0f) * 112),680, 16,16, 1078); // Slider handle [45, 150]
            if (UI_Slider(200,696, 328,16, &overBrightness, 6)) gammaSliderActive = true;
            if (gammaSliderActive && Sys_Input.currentMouse_dx != 0) {
                int32_t new = (int32_t)Sys_Settings.Brightness + vmin(vmax(Sys_Input.currentMouse_dx,-1),1); Sys_Settings.Brightness = (uint8_t)vmin(vmax(new,0),100);
                UpdateProjectionMatrices();
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
            RenderFormattedText(200,680,overBrightness ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,"BRIGHTNESS %u", Sys_Settings.Brightness);
            
            // Resolution
            if (UI_Button(200,726, 280,16, &overRes, 7) || (MenuEnter() && currentMenuItem == 7)) { /* TODO */ }
            overRes = overRes || currentMenuItem == 7;
            RenderFormattedText(200,710,overRes ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,"RESOLUTION %u x %u", Sys_Settings.ScreenWidth, Sys_Settings.ScreenHeight);            
            
            RenderUIImage(200,740, 16,16, 910); // Checkbox background
            if (UI_Button(200,756, 210,16, &overFull, 8) || (MenuEnter() && currentMenuItem == 8)) {
                Sys_Settings.Fullscreen = Sys_Settings.Fullscreen == 1u ? 0u : 1u;
                int monitorCount;
                GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
                GLFWmonitor* next = monitors[currentMonitorIndex];
                if (Sys_Settings.Fullscreen) {
                    int xpos, ypos, width, height;
                    glfwGetMonitorWorkarea(next, &xpos, &ypos, &width, &height);
                    Sys_Settings.ScreenWidth = width; Sys_Settings.ScreenHeight = height;
                    glfwSetWindowSize(Sys_Global.window, Sys_Settings.ScreenWidth, Sys_Settings.ScreenHeight);
                    glfwSetWindowPos(Sys_Global.window, xpos, ypos-18);
                } else {
                    int monitorCount;
                    GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
                    GLFWmonitor* next = monitors[currentMonitorIndex];
                    int mx, my;
                    glfwGetMonitorPos(next, &mx, &my);
                    const GLFWvidmode* mode = glfwGetVideoMode(next);
                    int xpos = mx + (mode->width - Sys_Settings.ScreenWidth) / 2;
                    int ypos = my + (mode->height - Sys_Settings.ScreenHeight) / 2;
                    glfwSetWindowPos(Sys_Global.window, xpos, ypos);
                }
                
                UpdateScreenSize(Sys_Global.window, Sys_Settings.ScreenWidth, Sys_Settings.ScreenHeight);
                Sys_Input.ignore_next_mouse_delta = true;
                SaveConfig();
            }
            
            overFull = overFull || currentMenuItem == 8;
            if (Sys_Settings.Fullscreen) RenderUIImage(202,742, 12,12, 912); // Checkbox check
            RenderFormattedText(220,740,overFull ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,"FULLSCREEN");
            
            RenderUIImage(588,730, 210,30, 1079); // Toggle monitor button background
            if (UI_Button(588,760, 210,30, &overChgM, 9) || (MenuEnter() && currentMenuItem == 9)) { CycleToNextMonitor(Sys_Global.window); }
            overChgM = overChgM || currentMenuItem == 9;
            RenderFormattedText(602,735,overChgM ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,"CHANGE MONITOR");
        } else if (currentMenuTab == 1) {
            menuItemCount = 49; // Input
        } else {
            menuItemCount = 10; // Audio / Lang
        }
        
        RenderUIImage(1087,723, 84,36, 1252); // Back Button background
        int8_t lastItem = menuItemCount - 1;
        if (UI_Button(1087,757, 84,32, &overBack, lastItem) || (MenuEnter() && currentMenuItem == lastItem)) MenuGoBack();
        overBack = overBack || currentMenuItem == lastItem;
        RenderFormattedText(1103,731,overBack ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_NORMAL,1.0f,"BACK");
    } else if (currentMenuPage == MenuPages_Load) {
        menuItemCount = 9; menuTabCount = 1;
        bool overBack = false;
        RenderFormattedText(280,50,TEXT_GREEN_MENU_SHADOW,FONT_STOPD,1.75f,"LOAD GAME");
        RenderFormattedText(280,46,TEXT_GREEN_MENU_GLOW,FONT_STOPD,1.75f,"LOAD GAME");
        RenderFormattedText(280,48,TEXT_GREEN_MENU,FONT_STOPD,1.75f,"LOAD GAME");
        RenderUIImage(400,214, 586,500, 1037); // Load/Save table background
        RenderUIImage(1060,724, 84,36, 1252); // Back Button background
        if (UI_Button(1060,758, 84,32, &overBack, 0) || (MenuEnter() && currentMenuItem == 0)) MenuGoBack();
        overBack = overBack || currentMenuItem == 0;
        RenderFormattedText(1076,732,overBack ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_NORMAL,1.0f,"BACK");
    } else if (currentMenuPage == MenuPages_Save) {
        menuItemCount = 9; menuTabCount = 1;
        bool overBack = false;
        RenderFormattedText(284,50,TEXT_GREEN_MENU_SHADOW,FONT_STOPD,1.75f,"SAVE GAME");
        RenderFormattedText(284,46,TEXT_GREEN_MENU_GLOW,FONT_STOPD,1.75f,"SAVE GAME");
        RenderFormattedText(284,48,TEXT_GREEN_MENU,FONT_STOPD,1.75f,"SAVE GAME");
        RenderUIImage(400,214, 586,500, 1037); // Load/Save table background
        RenderUIImage(1060,724, 84,36, 1252); // Back Button background
        if (UI_Button(1060,758, 84,32, &overBack, 0) || (MenuEnter() && currentMenuItem == 0)) MenuGoBack();
        overBack = overBack || currentMenuItem == 0;
        RenderFormattedText(1076,732,overBack ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_NORMAL,1.0f,"BACK");
    } else if (currentMenuPage == MenuPages_NewGame) {
        menuItemCount = 19;
        menuTabCount = (currentMenuItem > 0 && currentMenuItem <= 16) ? 2 : 1;
        
        bool overBack = false, overStart = false;
        RenderFormattedText(290,50,TEXT_GREEN_MENU_SHADOW,FONT_STOPD,1.75f,"NEW GAME");
        RenderFormattedText(290,46,TEXT_GREEN_MENU_GLOW,FONT_STOPD,1.75f,"NEW GAME");
        RenderFormattedText(290,48,TEXT_GREEN_MENU,FONT_STOPD,1.75f,"NEW GAME");
        RenderUIImage(136,196,1088,558,1048); // Newgame inset
        RenderUIImage(136,196,1088,558,1049); // Newgame background
        RenderUIImage(1060,724,84,36,1252); // Back Button background
        RenderFormattedText(220,146,TEXT_STOPD_RED,FONT_STOPD,1.5f,"NAME");
        RenderUIImage(297,217,32,32, 1028); // Menu pad
        RenderFormattedText(145,202,TEXT_STOPD_RED,FONT_STOPD,1.5f,"COMBAT");
        RenderUIImage(185,301,32,32,1028); // Menu pad
        RenderFormattedText(145,330,TEXT_STOPD_RED,FONT_STOPD,1.5f,"PUZZLE");
        RenderUIImage(185,493,32,32, 1028); // Menu pad
        RenderFormattedText(505,202,TEXT_STOPD_RED,FONT_STOPD,1.5f,"MISSION");
        RenderUIImage(725,301,32,32, 1028); // Menu pad
        RenderFormattedText(505,330,TEXT_STOPD_RED,FONT_STOPD,1.5f,"CYBERSPACE");
        RenderUIImage(725,493,32,32,1028); // Menu pad
        if (UI_Button(1060,758, 84,32, &overBack, 1) || (MenuEnter() && currentMenuItem == 1)) MenuGoBack();
        overBack = overBack || currentMenuItem == 1;
        RenderFormattedText(1076,732,overBack ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_NORMAL,1.0f,"BACK");
        if (UI_Button(544,747, 282,68, &overStart, 0) || (MenuEnter() && currentMenuItem == 0)) Sys_Global.menuActive = Sys_Global.gamePaused = false; // TODO reload game.
        overStart = overStart || currentMenuItem == 0;
        RenderFormattedText(400,464,overStart ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.5f,"START");
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
    if (Sys_Input.keyStates[GLFW_KEY_RIGHT].pressed) currentMenuTab = (currentMenuTab + 1) >= menuTabCount ? 0 : (currentMenuTab + 1);
    if (Sys_Input.keyStates[GLFW_KEY_LEFT].pressed) currentMenuTab =  (currentMenuTab - 1) < 0 ? (menuTabCount - 1) : (currentMenuTab - 1);
}

void RenderPausedUI(void) {
    menuItemCount = 6; menuTabCount = 1;
    bool overResume = false, overLoad /* ;) */ = false, overSave = false, overOptions = false, overQuitMenu = false, overQuit = false;
    RenderUIImage(519,276,328,300,1025); // Pause Menu background
    RenderUIImage(519,276,328,300,1080); // Pause Menu background outline
    RenderFormattedText(610,210,TEXT_STOPD_RED_PAUSETITLE,FONT_STOPD,1.0f,"PAUSED");
    if (UI_Button(522,330, 322,52, &overResume, 0) || (MenuEnter() && currentMenuItem == 0)) Sys_Global.gamePaused = false;
    overResume = overResume || currentMenuItem == 0;
    RenderFormattedText(610,306,overResume ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.0f,"RESUME");
    if (UI_Button(522,390, 322,52, &overLoad, 1) || (MenuEnter() && currentMenuItem == 1)) { currentMenuPage = MenuPages_Load; Sys_Global.menuActive = true; returnToPause = true; }
    overLoad = overLoad || currentMenuItem == 1;
    RenderFormattedText(630,364, overLoad ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.0f,"LOAD");
    if (UI_Button(522,450, 322,60, &overSave, 2) || (MenuEnter() && currentMenuItem == 2)) { currentMenuPage = MenuPages_Save; Sys_Global.menuActive = true; returnToPause = true; }
    overSave = overSave || currentMenuItem == 2;
    RenderFormattedText(635,422,overSave ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.0f,"SAVE");
    if (UI_Button(522,510, 322,60, &overOptions, 3) || (MenuEnter() && currentMenuItem == 3)) { currentMenuPage = MenuPages_Options; Sys_Global.menuActive = true; returnToPause = true; }
    overOptions = overOptions || currentMenuItem == 3;
    RenderFormattedText(599,480,overOptions ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.0f,"OPTIONS");
    if (UI_Button(522,570, 322,60, &overQuitMenu, 4) || (MenuEnter() && currentMenuItem == 4)) { Sys_Global.menuActive = true; currentMenuPage = MenuPages_FrontPage; }
    overQuitMenu = overQuitMenu || currentMenuItem == 4;
    RenderFormattedText(546,538,overQuitMenu ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.0f,"QUIT TO MENU");
    RenderUIImage(519,672,328,42,1252); // Pause Quit Game background
    if (UI_Button(522,672, 322,42, &overQuit, 5) || (MenuEnter() && currentMenuItem == 5)) OS_Exit(0);
    overQuit = overQuit || currentMenuItem == 5;
    RenderFormattedText(572,690,overQuit ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.0f,"QUIT GAME");
    if (Sys_Input.keyStates[GLFW_KEY_DOWN].pressed) currentMenuItem = (currentMenuItem + 1) >= menuItemCount ? 0 : (currentMenuItem + 1);
    if (Sys_Input.keyStates[GLFW_KEY_UP].pressed) currentMenuItem = (currentMenuItem - 1) < 0 ? (menuItemCount - 1) : (currentMenuItem - 1);
}
