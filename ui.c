// ui.c - User Interface(UI) aka HUD
INLINE bool CursorIsOverBounds(float x0, float x1, float y0, float y1) { return World.cursorPos_x >= x0 && World.cursorPos_x <= x1 && World.cursorPos_y >= y0 && World.cursorPos_y <= y1;/*0,0=top left*/ }
__attribute__((noinline)) bool MenuEnter() { return !Cheats.consoleActive && (Sys_Input.keyStates[KEY_KP_ENTER].pressed || Sys_Input.keyStates[KEY_ENTER].pressed); }
__attribute__((noinline)) u8 UI_MenuInteractable(i16 x, i16 y, float w, float h, bool* cursorOver, i8 this, bool sustained) {
    bool cursorIsOver = CursorIsOverBounds(x, x + w, (float)y - h, (float)y);
    if (cursorIsOver && mouseMovementThisFrame) { currentMenuItem = this; if (cursorOver != NULL) {*cursorOver = cursorIsOver;} }
    if ((sustained ? Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT ].down : Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT ].pressed) && cursorIsOver) return 1u;
    if ((sustained ? Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].down : Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].pressed) && cursorIsOver) return 2u;
    return 0u;
}

__attribute__((noinline)) u8 UI_Button(i16 x, i16 y, float w, float h, bool* cursorOver, i8 this) { return UI_MenuInteractable(x,y,w,h,cursorOver,this,false); }
__attribute__((noinline)) bool AnyLeftRightMouseDown() { return (Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].down || Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].down); }
bool UI_Slider(i16 x, i16 y, i16 w, i16 h, i16 sliderPos, i16 xPosForLabel, u8 currentValue, u8* out, bool* sliderActive, u8 min, u8 max, u8 step, u8 mindex, u16 lingdex) {
    bool over=false,changed=false; *out = currentValue;
    RenderUIImage(x,y, w,h, 1079); // Slider background
    RenderUIImage(x + sliderPos,y, h,h,1078); // Slider handle
    if (UI_MenuInteractable(xPosForLabel,y,xPosForLabel + w,h,&over,mindex,true)) *sliderActive = true;
    if (*sliderActive && World.currentMouse_dx != 0) { i32 new = (i32)currentValue + vmin(vmax(World.currentMouse_dx,-1),1); *out = (u8)vmin(vmax(new,min),max); if (*out != currentValue) {changed = true;} }
    if (!AnyLeftRightMouseDown()) { if (*sliderActive) { *sliderActive = false; SaveConfig(); } }
    if (MenuEnter() && currentMenuItem == mindex) {
        bool shiftHeld = Sys_Input.keyStates[KEY_LEFT_SHIFT].down || Sys_Input.keyStates[KEY_RIGHT_SHIFT].down;
        if (shiftHeld) *out = *out <=  ((min + step) - 1) ? max : *out - step;
        else           *out = *out >= ((max - step) + 1) ?  min : *out + step;
        changed = true;
    }
    over = over || currentMenuItem == mindex; RenderFormattedText(xPosForLabel,y,over ? T_YELLOW : T_GREEN,FONT_NORMAL,1.0f,"%s %u",Sys_Text.stringTable[lingdex],*out); return changed;
}

u8 UI_MenuButton(i16 bX, i16 bY, u8 menuItem, i16 bW, i16 bH,  i16 tX, i16 tY, const char* text, i16 pX, i16 pY) {
    bool over = false; u8 retvalue = 0u;
    retvalue = UI_Button(bX,bY,bW,bH,&over,menuItem); if (!retvalue) retvalue = (MenuEnter() && currentMenuItem == menuItem);
    over = over || currentMenuItem == menuItem;
    RenderFormattedText(tX,tY,over ? T_STOPD_RED : T_RED_MENU,FONT_STOPD,1.5f,text); 
    RenderUIImage(pX,pY,40,40,over ? 1029 : 1028); // Menu pad
    return retvalue;
}

bool UI_Checkbox(i16 x, i16 y, i8 mitem, u16 textIdx, bool currentlyOn) {
    RenderUIImage(x,y,16,16,910); // Checkbox background
    bool over = false; bool changed = (UI_Button(x,y + 16,210,16,&over,mitem) || (MenuEnter() && currentMenuItem == mitem)); over = over || currentMenuItem == mitem;
    if (currentlyOn) RenderUIImage(x + 2,y + 2, 12,12, 912); // Checkbox check
    RenderFormattedText(x + 20,y,over ? T_YELLOW : T_GREEN,FONT_NORMAL,1.0f,Sys_Text.stringTable[textIdx]);
    return changed;
}

__attribute__((noinline)) void UI_HeaderText(i16 x, const char* text) { RenderFormattedText(x,50,T_GREEN_MENU_SHADOW,FONT_STOPD,1.75f,text); RenderFormattedText(x,46,T_GREEN_MENU_GLOW,FONT_STOPD,1.75f,text); RenderFormattedText(x,48,T_GREEN_MENU,FONT_STOPD,1.75f,text); }
void PlayGameMusic(); void PlayMenuMusic();
__attribute__((noinline)) void MenuGoBack() {
    if (returnToPause) { returnToPause = false; World.paused = true; World.menuActive = false; PlayGameMusic(); }
    if (currentMenuPage == Mpg_Singleplayer || currentMenuPage == Mpg_Multiplayer || currentMenuPage == Mpg_Options) currentMenuPage = Mpg_FrontPage;//News
    else if (currentMenuPage == Mpg_Load || currentMenuPage == Mpg_NewGame || currentMenuPage == Mpg_IntroVideo || currentMenuPage == Mpg_CreditsVideo) currentMenuPage = Mpg_Singleplayer;
}

void CreateShadowBuffers() { shadowMapSSBO=MakeSSBO(&shadowMapSSBO,5,(MAX_SHADOWMAPS * (SHADOW_MAP_SIZE * SHADOW_MAP_SIZE * 6U)) * sizeof(u32),NULL,GL_STATIC_DRAW); shadowMapsIndirectionID=MakeSSBO(&shadowMapsIndirectionID,6,LIGHT_COUNT * sizeof(u32),NULL,GL_STATIC_DRAW); shadowBuffersCreated=true; }
__attribute__((noinline)) void ChangeMenuPage(u8 pg) { currentMenuPage = pg; currentMenuItem = currentMenuTab = 0; }
void RenderMenu() {    
    if (currentMenuPage != Mpg_IntroVideo && currentMenuPage != Mpg_CreditsVideo && currentMenuPage != Mpg_Options) RenderUIImage(-417,-384, 2200,1536, 1026); // Menu background
    if (currentMenuPage == Mpg_IntroVideo || currentMenuPage == Mpg_CreditsVideo) RenderUIImage(-417,-384, 2200,1536, 0); // Video blackground
    if (currentMenuPage == Mpg_Options) RenderUIImage(-417,-384, 2200,1536, 1032); // Menu background
    if (currentMenuPage == Mpg_FrontPage) {
        menuItemCount = 4; menuTabCount = 1;
        RenderUIImage(282,46, 800,128, 1031); // Title CITADEL with strikethrough effect
        if (UI_MenuButton(408,340, 0, 574,84, 304,188,/*"SINGLEPLAYER"*/Sys_Text.stringTable[719],413,276)) ChangeMenuPage(Mpg_Singleplayer);
        if (UI_MenuButton(408,458, 1, 574,84, 304,268,/*"MULTIPLAYER"*/Sys_Text.stringTable[720], 413,396)) ChangeMenuPage(Mpg_Multiplayer);
        if (UI_MenuButton(408,582, 2, 574,84, 304,350,/*"OPTIONS"*/Sys_Text.stringTable[721],     413,520)) ChangeMenuPage(Mpg_Options);
        if (UI_MenuButton(408,702, 3, 574,84, 304,430,/*"QUIT"*/Sys_Text.stringTable[722],        413,638)) OS_Exit(0);
    } else if (currentMenuPage == Mpg_Singleplayer) {
        menuItemCount = 5; menuTabCount = 1;
        UI_HeaderText(250,/*"SINGLEPLAYER"*/Sys_Text.stringTable[719]);
        if (UI_MenuButton(408,340,0,574,84, 304,188,/*"CONTINUE"*/Sys_Text.stringTable[723],    413,276)) ChangeMenuPage(Mpg_Load);
        if (UI_MenuButton(408,458,1,574,84, 304,268,/*"NEW GAME"*/Sys_Text.stringTable[741],    413,396)) ChangeMenuPage(Mpg_NewGame);
        if (UI_MenuButton(408,582,2,574,84, 304,350,/*"PLAY INTRO"*/Sys_Text.stringTable[742],  413,520)) ChangeMenuPage(Mpg_IntroVideo);
        if (UI_MenuButton(408,702,3,574,84, 304,430,/*"PLAY CREDITS"*/Sys_Text.stringTable[743],413,638)) ChangeMenuPage(Mpg_CreditsVideo);
        RenderUIImage(1060,724, 84,36, 1252); // Back Button background
        bool overBack = false;        
        if (UI_Button(1060,758, 84,32, &overBack, 4) || (MenuEnter() && currentMenuItem == 4)) MenuGoBack();
        overBack = overBack || currentMenuItem == 4;
        RenderFormattedText(1076,732,overBack ? T_STOPD_RED_HIGHLIGHT : T_RED_MENU,FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == Mpg_Multiplayer) {
        menuItemCount = 1; menuTabCount = 1;
        UI_HeaderText(266,/*"MULTIPLAYER"*/Sys_Text.stringTable[720]);
        RenderUIImage(1060,724, 84,36, 1252); // Back Button background
        bool overBack = false;
        if (UI_Button(1060,758, 84,32, &overBack, 0) || (MenuEnter() && currentMenuItem == 0)) MenuGoBack();
        overBack = overBack || currentMenuItem == 0;
        RenderFormattedText(1076,732,overBack ? T_STOPD_RED_HIGHLIGHT : T_RED_MENU,FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == Mpg_Options) {
        menuTabCount = 3;
        UI_HeaderText(238,/*"CONFIGURATION"*/Sys_Text.stringTable[745]);
        if (currentMenuTab != 0) RenderUIImage(179,220, 1001,548, 1030); // Config background
        if (currentMenuTab == 0) RenderUIImage(179,220, 1001,548, 1033); // Config background graphics (empty alpha center)
        RenderUIImage(520,196, 160,30, currentMenuTab == 2 ? 920 : 921); // Config tab unhighlighted
        if (UI_Button(520,196+30, 160,30, NULL, 2)) currentMenuTab = 2;
        RenderFormattedText(530,202,currentMenuTab == 2 ? T_YELLOW : T_GREEN,FONT_NORMAL,1.0f,/*"AUDIO / LANG"*/Sys_Text.stringTable[793]);
        RenderUIImage(354,196, 160,30, currentMenuTab == 1 ? 920 : 921); // Config tab unhighlighted
        if (UI_Button(354,196+30, 160,30, NULL, 1)) currentMenuTab = 1;
        RenderFormattedText(366,202,currentMenuTab == 1 ? T_YELLOW : T_GREEN,FONT_NORMAL,1.0f,/*"INPUT"*/Sys_Text.stringTable[792]);
        RenderUIImage(190,196, 160,30, currentMenuTab == 0 ? 920 : 921); // Config tab highlighted
        if (UI_Button(190,196+30, 160,30, NULL, 0)) currentMenuTab = 0;
        RenderFormattedText(200,202,currentMenuTab == 0 ? T_YELLOW : T_GREEN,FONT_NORMAL,1.0f,/*"GRAPHICS"*/Sys_Text.stringTable[791]);
        if (currentMenuTab == 0) {
            bool overRes = false, overFull = false, overChgM = false;
            menuItemCount = 11; // Graphics
            if (UI_Checkbox(200,500,0,Sys_Settings.ModelDetail ? /*High*/915 : /*No Detail Level Models*/914,Sys_Settings.ModelDetail)) { Sys_Settings.ModelDetail = Sys_Settings.ModelDetail ? 0u : 1u; SaveConfig(); }
            if (UI_Checkbox(200,530,1,/*"FXAA"*/780,Sys_Settings.FXAA)) { Sys_Settings.FXAA = Sys_Settings.FXAA ? 0u : 1u; SaveConfig(); }
            if (UI_Checkbox(200,560,2,Sys_Settings.Shadows ? /*Soft*/787 : /*No Shadows*/785,Sys_Settings.Shadows)) { Sys_Settings.Shadows = Sys_Settings.Shadows ? 0u : 1u; if (!shadowBuffersCreated) {CreateShadowBuffers();} SaveConfig(); }
            if (UI_Checkbox(200,590,3,/*SSR*/788,Sys_Settings.Reflections)) { Sys_Settings.Reflections = Sys_Settings.Reflections ? 0u : 1u; SaveConfig(); }
            if (UI_Checkbox(200,620,4,/*VSYNC*/1026,Sys_Settings.Vsync)) { Sys_Settings.Vsync = Sys_Settings.Vsync ? 0u : 1u; SetVSync(); SaveConfig(); }
            RenderFormattedText(310,620,T_GREEN,FONT_NORMAL,1.0f,"(FPS: %d)", globalframesPerLastSecond); // Helper to see vsync take effect.
            u8 newVal;
            if (UI_Slider(400,650,128,16,(((Sys_Settings.FOV - 45.0f) / 105.0f) * (128 - 16)),200,Sys_Settings.FOV,&newVal,&fovSliderActive,45,150,5,5,/*Field of View*/775)) { Sys_Settings.FOV = newVal; if (!AnyLeftRightMouseDown()) {SaveConfig();} }
            if (UI_Slider(400,680,128,16,((Sys_Settings.Brightness / 100.0f) * (128 - 16)),200,Sys_Settings.Brightness,&newVal,&gammaSliderActive,0,100,2,6,/*Gamma*/774)) { Sys_Settings.Brightness = newVal; if (!AnyLeftRightMouseDown()) {SaveConfig();} }
            
            // Resolution
            {
                // Header hit area - UI_Button subtracts h from y internally, so pass y+h as y
                if (UI_Button(190,726,328,16,&overRes,7) || (MenuEnter() && currentMenuItem == 7)) { resDropdownOpen = !resDropdownOpen; currentMenuItem = 7; }
                overRes = overRes || currentMenuItem == 7;
                char resBuf[32];
                if (resDropdownCount > 0) sFormat(resBuf, sizeof(resBuf), "%ux%u",(u32)resModes[resSelectedIdx].w,(u32)resModes[resSelectedIdx].h);
                else sFormat(resBuf, sizeof(resBuf), "%ux%u",Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight);

                RenderUIImage(476, 710, 16, 16, overRes ? 1119 : 1077);
                RenderFormattedText(200, 710, overRes ? T_YELLOW : T_GREEN,FONT_NORMAL, 1.0f, "RESOLUTION %s", resBuf);
            }
    
            // Fullscreen checkbox
            RenderUIImage(200,740, 16,16, 910); // Checkbox background
            if (UI_Button(200,756, 210,16, &overFull, 8) || (MenuEnter() && currentMenuItem == 8)) { Sys_Settings.Fullscreen = Sys_Settings.Fullscreen == 1u ? 0u : 1u; ChangeFullScreenWindowed(true); SaveConfig(); }
            overFull = overFull || currentMenuItem == 8;
            if (Sys_Settings.Fullscreen) RenderUIImage(202,742, 12,12, 912); // Checkbox check
            RenderFormattedText(220,740,overFull ? T_YELLOW : T_GREEN,FONT_NORMAL,1.0f,/*"Fullscreen"*/Sys_Text.stringTable[773]);
            RenderUIImage(588,730, 210,30, 1079); // Toggle monitor button background
            if (UI_Button(588,760, 210,30, &overChgM, 9) || (MenuEnter() && currentMenuItem == 9)) { CycleToNextMonitor(); }
            overChgM = overChgM || currentMenuItem == 9;
            RenderFormattedText(602,735,overChgM ? T_YELLOW : T_GREEN,FONT_NORMAL,1.0f,/*"CHANGE MONITOR"*/Sys_Text.stringTable[1025]);
        } else if (currentMenuTab == 1) { 
            menuItemCount = 49; // Input
        } else {
            menuItemCount = 10; // Audio / Lang
            u8 newVal;
            if (UI_Slider(426,240,128,16,((Sys_Settings.VolumeMaster / 100.0f) * (128 - 16)),200,Sys_Settings.VolumeMaster,&newVal,&masterVolumeSliderActive,0,100,5,0,/*Master Volume*/802)) { Sys_Settings.VolumeMaster = newVal; if (!AnyLeftRightMouseDown()) {SaveConfig();} }
            if (UI_Slider(426,270,128,16,((Sys_Settings.VolumeMusic / 100.0f) * (128 - 16)),200,Sys_Settings.VolumeMusic,&newVal,&musicVolumeSliderActive,0,100,5,1,/*Music Volume*/803)) { Sys_Settings.VolumeMusic = newVal; if (!AnyLeftRightMouseDown()) {SaveConfig();} }
        }
        RenderUIImage(1087,723, 84,36, 1252); // Back Button background
        i8 lastItem = menuItemCount - 1; bool overBack = false;
        if (UI_Button(1087,757, 84,32, &overBack, lastItem) || (MenuEnter() && currentMenuItem == lastItem)) MenuGoBack();
        overBack = overBack || currentMenuItem == lastItem;
        RenderFormattedText(1103,731,overBack ? T_STOPD_RED_HIGHLIGHT : T_RED_MENU,FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == Mpg_Load || currentMenuPage == Mpg_Save) {
        menuItemCount = 9; menuTabCount = 1; bool isSave = currentMenuPage == Mpg_Save;
        UI_HeaderText(isSave ? 284 : 340, isSave ? /*"SAVE GAME"*/Sys_Text.stringTable[769] : /*"LOAD"*/Sys_Text.stringTable[726]);
        RenderUIImage(400,214, 586,500, 1037); // Load/Save table background
        RenderUIImage(1060,724, 84,36, 1252); // Back Button background
        bool overBack = false;
        if (UI_Button(1060,758, 84,32, &overBack, 0) || (MenuEnter() && currentMenuItem == 0)) MenuGoBack();
        overBack = overBack || currentMenuItem == 0;
        RenderFormattedText(1076,732, overBack ? T_STOPD_RED_HIGHLIGHT : T_RED_MENU, FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == Mpg_NewGame) {
        menuItemCount = 7; menuTabCount = (currentMenuItem > 0 && currentMenuItem <= 16) ? 2 : 1;
        UI_HeaderText(290,/*"NEW GAME"*/Sys_Text.stringTable[741]);
        RenderUIImage(136,196,1088,558,1048); // Newgame inset
        RenderUIImage(136,196,1088,558,1049); // Newgame background
        if (UI_MenuButton(276,270,0,795,74, 226,146,/*"NAME:"*/Sys_Text.stringTable[746],299,214)) { /* Just for highlight */ }
        enteringPlayerName = (currentMenuItem == 0);
        if (World.playerName[0] == '\0') RenderFormattedText(642,232,T_RED_MENU,FONT_STOPD,1.0f,/*"ENTER NAME..."*/Sys_Text.stringTable[748]);
        else                                  RenderFormattedText(518,232,enteringPlayerName ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.0f,World.playerName);
        if (UI_MenuButton(174,377,1,496,95, 148,202,/*"COMBAT"*/Sys_Text.stringTable[748],185,299)) { World.diffCbt = World.diffCbt >= 3 ? 0 : World.diffCbt + 1; }  if (UI_MenuButton(704,377,3,496,95, 510,202,/*"MISSION"*/Sys_Text.stringTable[749],726,299)) { World.diffMis = World.diffMis >= 3 ? 0 : World.diffMis + 1; }
        RenderFormattedText(162,270,World.diffCbt == 0 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"0");    RenderFormattedText(513,270,World.diffMis == 0 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"0");
        RenderFormattedText(233,270,World.diffCbt == 1 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"1");    RenderFormattedText(584,270,World.diffMis == 1 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"1");
        RenderFormattedText(307,270,World.diffCbt == 2 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"2");    RenderFormattedText(658,270,World.diffMis == 2 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"2");
        RenderFormattedText(379,270,World.diffCbt == 3 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"3");    RenderFormattedText(730,270,World.diffMis == 3 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"3");
        if (UI_MenuButton(174,568,2,496,92, 149,330,/*"PUZZLE"*/Sys_Text.stringTable[751],185,490)) { World.diffPuz = World.diffPuz >= 3 ? 0 : World.diffPuz + 1; }  if (UI_MenuButton(704,568,4,496,92, 509,330,/*"CYBERSPACE"*/Sys_Text.stringTable[750],726,490)) { World.diffCyb = World.diffCyb >= 3 ? 0 : World.diffCyb + 1; }
        RenderFormattedText(162,399,World.diffPuz == 0 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"0");    RenderFormattedText(513,399,World.diffCyb == 0 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"0");
        RenderFormattedText(233,399,World.diffPuz == 1 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"1");    RenderFormattedText(584,399,World.diffCyb == 1 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"1");
        RenderFormattedText(307,399,World.diffPuz == 2 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"2");    RenderFormattedText(658,399,World.diffCyb == 2 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"2");
        RenderFormattedText(379,399,World.diffPuz == 3 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"3");    RenderFormattedText(730,399,World.diffCyb == 3 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"3");
        if (UI_Button(221,460,82,79,NULL,1)) {World.diffCbt =0; currentMenuItem=1; } if (UI_Button(330,460,82,79,NULL,1)) {World.diffCbt =1; currentMenuItem=1; } if (UI_Button(439,460,82,79,NULL,1)) {World.diffCbt =2; currentMenuItem=1; } if (UI_Button( 547,460,82,79,NULL,1)) {World.diffCbt =3; currentMenuItem=1; }
        if (UI_Button(221,651,82,79,NULL,2)) {World.diffPuz =0; currentMenuItem=2; } if (UI_Button(330,651,82,79,NULL,2)) {World.diffPuz =1; currentMenuItem=2; } if (UI_Button(439,651,82,79,NULL,2)) {World.diffPuz =2; currentMenuItem=2; } if (UI_Button( 547,651,82,79,NULL,2)) {World.diffPuz =3; currentMenuItem=2; }
        if (UI_Button(748,460,82,79,NULL,3)) {World.diffMis=0; currentMenuItem=3; } if (UI_Button(857,460,82,79,NULL,3)) {World.diffMis=1; currentMenuItem=3; } if (UI_Button(966,460,82,79,NULL,3)) {World.diffMis=2; currentMenuItem=3; } if (UI_Button(1074,460,82,79,NULL,3)) {World.diffMis=3; currentMenuItem=3; }
        if (UI_Button(748,651,82,79,NULL,4)) {World.diffCyb  =0; currentMenuItem=4; } if (UI_Button(857,651,82,79,NULL,4)) {World.diffCyb  =1; currentMenuItem=4; } if (UI_Button(966,651,82,79,NULL,4)) {World.diffCyb  =2; currentMenuItem=4; } if (UI_Button(1074,651,82,79,NULL,4)) {World.diffCyb  =3; currentMenuItem=4; }
        bool overBack = false, overStart = false;
        if (UI_Button(544,747, 282,68, &overStart, 5) || (MenuEnter() && currentMenuItem == 5)) GoIntoGame();
        overStart = overStart || currentMenuItem == 5;
        RenderFormattedText(400,464,overStart ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,/*"START"*/Sys_Text.stringTable[886]);
        if (UI_Button(1060,758, 84,32, &overBack, 6) || (MenuEnter() && currentMenuItem == 6)) MenuGoBack();
        overBack = overBack || currentMenuItem == 6;
        RenderUIImage(1060,724,84,36,1252); // Back Button background
        RenderFormattedText(1076,732,overBack ? T_STOPD_RED_HIGHLIGHT : T_RED_MENU,FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == Mpg_IntroVideo || currentMenuPage == Mpg_CreditsVideo) {
        menuItemCount = menuTabCount = 1;
        if (MenuEnter()) MenuGoBack();
    }
    if (menuTabCount <= currentMenuTab) currentMenuTab = 0;
    if (menuItemCount <= currentMenuItem) currentMenuItem = 0;
    static const i8 ngSwap[7] = {0,3,4,1,2,6,5};
    if (Sys_Input.keyStates[KEY_RIGHT].pressed || Sys_Input.keyStates[KEY_LEFT].pressed) { int dir = Sys_Input.keyStates[KEY_RIGHT].pressed ? 1 : -1; currentMenuTab = (currentMenuTab + menuTabCount + dir) % menuTabCount; if (currentMenuPage == Mpg_NewGame && currentMenuItem < 7) {currentMenuItem=ngSwap[currentMenuItem];} }
}

void RenderPausedUI() {
    menuItemCount = 6; menuTabCount = 1;
    bool overResume = false, overLoad /* ;) */ = false, overSave = false, overOptions = false, overQuitMenu = false, overQuit = false;
    RenderUIImage(519,276,328,300,1025); // Pause Menu background
    RenderUIImage(519,276,328,300,1080); // Pause Menu background outline
    RenderFormattedText(610,210,T_STOPD_RED_PAUSETITLE,FONT_STOPD,1.0f,/*"PAUSED"*/Sys_Text.stringTable[724]);
    if (UI_Button(522,330, 322,52, &overResume, 0) || (MenuEnter() && currentMenuItem == 0)) World.paused = false;
    overResume = overResume || currentMenuItem == 0;
    RenderFormattedText(610,306,overResume ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.0f,/*"RESUME"*/Sys_Text.stringTable[725]);
    if (UI_Button(522,390, 322,52, &overLoad, 1) || (MenuEnter() && currentMenuItem == 1)) { currentMenuPage = Mpg_Load; PlayMenuMusic(); World.menuActive = true; returnToPause = true; }
    overLoad = overLoad || currentMenuItem == 1;
    RenderFormattedText(630,364, overLoad ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.0f,/*"LOAD"*/Sys_Text.stringTable[726]);
    if (UI_Button(522,450, 322,60, &overSave, 2) || (MenuEnter() && currentMenuItem == 2)) { currentMenuPage = Mpg_Save; PlayMenuMusic(); World.menuActive = true; returnToPause = true; }
    overSave = overSave || currentMenuItem == 2;
    RenderFormattedText(635,422,overSave ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.0f,/*"SAVE"*/Sys_Text.stringTable[727]);
    if (UI_Button(522,510, 322,60, &overOptions, 3) || (MenuEnter() && currentMenuItem == 3)) { currentMenuPage = Mpg_Options; PlayMenuMusic(); World.menuActive = true; returnToPause = true; }
    overOptions = overOptions || currentMenuItem == 3;
    RenderFormattedText(599,480,overOptions ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.0f,/*"OPTIONS"*/Sys_Text.stringTable[721]);
    if (UI_Button(522,570, 322,60, &overQuitMenu, 4) || (MenuEnter() && currentMenuItem == 4)) { PlayMenuMusic(); World.menuActive = true; currentMenuPage = Mpg_FrontPage; }
    overQuitMenu = overQuitMenu || currentMenuItem == 4;
    RenderFormattedText(546,538,overQuitMenu ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.0f,/*"QUIT TO MENU"*/Sys_Text.stringTable[728]);
    RenderUIImage(519,672,328,42,1252); // Pause Quit Game background
    if (UI_Button(522,714, 322,42, &overQuit, 5) || (MenuEnter() && currentMenuItem == 5)) OS_Exit(0);
    overQuit = overQuit || currentMenuItem == 5;
    RenderFormattedText(572,690,overQuit ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.0f,/*"QUIT GAME"*/Sys_Text.stringTable[729]);
}

static const u16 vmailStartFrames[6]={1579,1645,1713,1784,1864,1931}; static const u16   vmailEndFrames[6]={1644,1712,1783,1863,1930,1988}; u8 MFD_LefTab=0,MFD_CenterTab=0,MFD_RightTab=0; double avgCPUt[AVG_CPU_TAPS]={0}; int avgCPUt_idx = 0;
void AddItemToInventory(int index, int custIdx); void ResetHeldItem(); extern double game_actual_start_time;
extern u16 editModeTestEntityDefinition;
static double RenderUI() {
    drawCallsNormal = drawCalls;
    World.uiIsBlocking = false;
    if (World.creditsActive) { // Render Credits
        if (Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed) { ++World.creditsPageIndex; if(World.creditsPageIndex > CREDITS_PAGES){World.creditsActive=false; return get_time();} /*Finished with Erthang!  That's it, go home.*/ }
        if (World.creditsPageIndex == 1) { CreditsStats(); RenderFormattedText(300,10,T_WHITE,FONT_NORMAL,1.0f,(const char*)&creditStats); }
        else                                               RenderFormattedText(300,10,T_WHITE,FONT_NORMAL,1.0f,creditPages[World.creditsPageIndex]);
        return get_time();
    }
    if (World.menuActive) RenderMenu();
    else if (World.paused) RenderPausedUI();
    if ((World.menuActive || World.paused)) {
        if (Sys_Input.keyStates[KEY_DOWN].pressed) currentMenuItem = (currentMenuItem + 1) >= menuItemCount ? 0 : (currentMenuItem + 1);
        else if (Sys_Input.keyStates[KEY_UP].pressed) currentMenuItem = (currentMenuItem - 1) < 0 ? (menuItemCount - 1) : (currentMenuItem - 1);
    } else if (!World.Sys_UI.vmailActive) { /* Normal UI */
        //         if (World.Sys_UI.showBioMonitor) { /*Graph*/ /*Biomonitor texts, BPM, Patch, Fatigue*/ } if (World.Sys_UI.showEnergyTickPanel) { /*EnergyTickPanel*/ } if (World.Sys_UI.showHealthTickPanel) { /*HealthTickPanel*/ }
//         if (World.Sys_UI.showEnergyIndicator) { /*EnergyIndicator*/ /*EnergySurge*/ /*EnergyDrainText*/ /*EnergyJPMText*/ }
//         if (World.Sys_UI.showHealthIndicator) { /*HealthIndicator*/ /*HealthIndicatorCyber*/ }
        //---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        // Health and Energy Bars
        if (!Cheats.noHUD) {
            RenderUIImage(1332, 2,32,32,956); // Health Indicator
            int p1H = World.instances[PLAYER1].health; if (p1H > 255) p1H = 255;
            for (int i=7;i>=0;--i) if (i == 7/*Always render at least 1 tick*/ || p1H > (7 - i) * 11)       RenderUIImage(1050 - (i * 16), 4,32,32,964); // Health Tick Red
            for (int i=7;i>=0;--i) if (p1H > 88 + (7 - i) * 11)  RenderUIImage(1178 - (i * 16), 4,32,32,963); // Health Tick Orange
            for (int i=7;i>=0;--i) if (p1H > 176 + (7 - i) * 11) RenderUIImage(1306 - (i * 16), 4,32,32,962); // Health Tick Green
            RenderUIImage(1333,36,32,32,939); // Energy Indicator
            int p1E = World.invP1.energy; if (p1E > 255) p1E = 255;
            for (int i=7;i>=0;--i) if (i == 7/*Always render at least 1 tick*/ || p1E > (7 - i) * 11)       RenderUIImage(1050 - (i * 16),35,32,32,964); // Energy Tick Red
            for (int i=7;i>=0;--i) if (p1E > 88 + (7 - i) * 11)  RenderUIImage(1178 - (i * 16),35,32,32,963); // Energy Tick Orange
            for (int i=7;i>=0;--i) if (p1E > 176 + (7 - i) * 11) RenderUIImage(1306 - (i * 16),35,32,32,962); // Energy Tick Green
        }
        //---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//         if (World.Sys_UI.showTeleportFX) { /*TeleportFX*/ } if (World.Sys_UI.showRadiationFX) { /*RadiationFX*/ } if (World.Sys_UI.showHealingFX) { /*HealingFX*/ } if (World.Sys_UI.showShieldFX) { /*ShieldFX*/ } 
//         if (World.Sys_UI.showShieldActivation) { /*waveup*/ /*wavedn*/ } if (World.Sys_UI.showShieldDeactivation) { /*waveup*/ /*wavedn*/ } if (World.Sys_UI.showDeathRessurectionFX) { /*spawndelaycontainers...*/ } 
        if (World.invP1.hasHardware & HW_BIO && !Cheats.noHUD) RenderUIImage(   0,200,40,40, 989); // Hw Btn: Biomonitor
        if (World.invP1.hasHardware & HW_SNS && !Cheats.noHUD) RenderUIImage(   0,250,40,40,1009); // Hw Btn: Sensaround
        if (World.invP1.hasHardware & HW_LAN && !Cheats.noHUD) RenderUIImage(   0,300,40,40,1004); // Hw Btn: Lantern
        if (World.invP1.hasHardware & HW_SHD && !Cheats.noHUD) RenderUIImage(   0,350,40,40,1014); // Hw Btn: Shield
        if (World.invP1.hasHardware & HW_INF && !Cheats.noHUD) RenderUIImage(1326,200,40,40, 998); // Hw Btn: Infrared
        if (World.invP1.hasHardware & HW_ERD && !Cheats.noHUD) RenderUIImage(1326,250,40,40, 996); // Hw Btn: Ereader
        if (World.invP1.hasHardware & HW_BST && !Cheats.noHUD) RenderUIImage(1326,300,40,40, 993); // Hw Btn: Booster
        if (World.invP1.hasHardware & HW_JET && !Cheats.noHUD) RenderUIImage(1326,350,40,40,1000); // Hw Btn: Jumpjets
        if (!Cheats.noHUD) {RenderUIImage(667,0,32,32,1020);} /*ShootModeButton*/
        if (World.inventoryMode && CursorIsOverBounds(667,699,0,32)) {
            World.uiIsBlocking = true;
            if (Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed || Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].pressed) { ForceShootMode(); Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed = Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].pressed = false; }
        }
//         if (World.Sys_UI.showTextWarnings) { /*WarningTexts...*/ } if (World.Sys_UI.showAutomapFull) { /*AutomapFullRawImage*/ /*PlayerIconFull*/ /*CloseFullmapButton*/ } if (World.Sys_UI.showMissionTimer) { /*MissionTimerT*/ /*MissionTimer*/ }
//         if (World.Sys_UI.showCyberTimer) { /*CyberTimerT*/ /*CyberTimer*/ }
        //---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        // Left MFD
        if (MFD_LefTab == 0) { /*WeaponTabLH: WepNameTextLH, WepIconLH, ClipBox, EnergyHeatTicks, ReloadButtons, EnergySlider*/ }
        else if (MFD_LefTab == 1) { /*ItemTabLH: ItemIcon, ItemText, Vaporize/Apply/Use Buttons, EReaderSections, AccessCardsList, Sliders*/ }
        else if (MFD_LefTab == 2) { /*AutomapTabLH: AutomapMask, Overlays, PlayerIcon, ZoomIn/Out/Full/Side Buttons*/ }
        else if (MFD_LefTab == 3) { /*TargetTabLH*/ }
        else if (MFD_LefTab == 4) { /*DataTabLH: SecurityLH, DataHeaders, ElevatorUIControl, KeycodeUIControl, SearchContents, AudioLogInfo, PuzzleGrid, PuzzleWire, SystemAnalyzer Display*/ }
//         if (World.Sys_UI.showSensaroundLH) { /*SensaroundLH Plane*/ }
        if (!Cheats.noHUD) {
            RenderUIImage(-16,552,32,40,MFD_LefTab == 0 ? 1024 : 1022); // Weapon
            RenderUIImage(-16,600,32,40,MFD_LefTab == 1 ? 1024 : 1022); // Item
            RenderUIImage(-16,648,32,40,MFD_LefTab == 2 ? 1024 : 1022); // Automap
            RenderUIImage(-16,696,32,40,MFD_LefTab == 3 ? 1024 : 1022); // Data
        }
        //---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        // Center MFD
        if (MFD_CenterTab == 0) { /*MainTab: WeaponInventory, WeaponShotsInventory, GrenadeInventory, PatchInventory*/
            RenderFormattedText(400,580,T_RED,FONT_NORMAL,1.0f,"WEAPONS"); /*WEAPONS*/
            for (int _wsi = 0; _wsi < 7; ++_wsi) {
                int _wi = (int)World.invP1.weaponInventoryIndices[_wsi];
                if (_wi >= 0 && _wi < MAX_ENTITIES) RenderFormattedText(400,610 + _wsi*20,(_wsi == (int)World.invP1.weaponCurrent) ? T_YELLOW : T_GREEN,FONT_NORMAL,1.0f,"%d %s",_wsi,Sys_Text.stringTable[ItemStringIdx(_wi)]);
                else RenderFormattedText(400,610 + _wsi*20,T_GREEN,FONT_NORMAL,1.0f,"%d -",_wsi);
            }
            RenderFormattedText(676,580,T_RED,FONT_NORMAL,1.0f,"SHOTS"); /*SHOTS*/
            RenderFormattedText(740,580,T_RED,FONT_NORMAL,1.0f,"GRENADES"); /*GRENADES*/
            RenderFormattedText(920,580,T_RED,FONT_NORMAL,1.0f,"PATCHES"); /*PATCHES*/
        } else if (MFD_CenterTab == 1) { /*HardwareTab: Label, HardwareInventory*/ }
        else if (MFD_CenterTab == 2) { /*GeneralTab: Label, GeneralInventory, AccessCards*/ }
        else if (MFD_CenterTab == 3) { /*SoftwareTab: Label, SoftwareInventory, ICEDrill, Pulser, Turbo, Decoy, Recall*/ }
        else if (MFD_CenterTab == 4) { /*MultiMediaDataReader: LogTableofContents, LogsLevelFolder, LogTextReader, EmailTab, DataTab, NotesTab*/ }
//         if (World.Sys_UI.showSensaroundCenter) { /*SensaroundCenter Plane*/ }
        if (!Cheats.noHUD) {
            RenderUIImage(400,752,64,32,MFD_CenterTab == 0 ? 1024 : 1021); // Main
            RenderUIImage(480,752,64,32,MFD_CenterTab == 1 ? 1024 : 1021); // Hardware
            RenderUIImage(560,752,64,32,MFD_CenterTab == 2 ? 1024 : 1021); // General
            RenderUIImage(902,752,64,32,MFD_CenterTab == 3 ? 1024 : 1021); // Software
        }
        if (World.inventoryMode && World.invP1.holdingObject && CursorIsOverBounds(345,1021,460,768)) { // Add to Inventory Helper
            World.uiIsBlocking = true;
            RenderUIImage(345,528,676,240,1075);
            RenderFormattedText(586,528,T_GREEN,FONT_NORMAL,1.0f,"ADD TO INVENTORY");
            if (Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed || Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].pressed) { AddItemToInventory(World.invP1.heldObjectIndex,World.invP1.heldObjectCustIdx); ResetHeldItem(); Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed = Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].pressed = false; }
        }
        //---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        // Right MFD
        if (MFD_RightTab == 0) { /*WeaponTabRH: WepName, ClipBox, HeatTicks, Reload/Unload, EnergySlider*/ }
        else if (MFD_RightTab == 1) { /*ItemTabRH: Icons, Actions, EReaderSections, Sliders*/ }
        else if (MFD_RightTab == 2) { /*AutomapTabRH: AutomapMask, Zoom controls*/ }
        else if (MFD_RightTab == 3) { /*TargetTabRH*/ }
        else if (MFD_RightTab == 4) { /*DataTabRH: SecurityRH, Elevators, Keycodes, AudioLogs, Puzzles, SystemAnalyzer*/ }
//         if (World.Sys_UI.showSensaroundRH) { /*SensaroundRH Plane*/ }
        if (!Cheats.noHUD) {
            RenderUIImage(1350,552,32,40,MFD_RightTab == 0 ? 1024 : 1022); // Weapon
            RenderUIImage(1350,600,32,40,MFD_RightTab == 1 ? 1024 : 1022); // Item
            RenderUIImage(1350,648,32,40,MFD_RightTab == 2 ? 1024 : 1022); // Automap
            RenderUIImage(1350,696,32,40,MFD_RightTab == 3 ? 1024 : 1022); // Data
        }
        //---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------        
    }
    if (World.Sys_UI.vmailActive) {
        if (World.Sys_UI.vmailFrameFinished < World.pauseRelativeTime && World.Sys_UI.vmailFrame < vmailEndFrames[World.Sys_UI.vmailActive]) {
            if (World.Sys_UI.vmailFrame == (vmailStartFrames[World.Sys_UI.vmailActive]+11)) play_wav(sounds[99],1.0f,(V3){0,0,0},false);
            World.Sys_UI.vmailFrameFinished=World.pauseRelativeTime + 0.1; World.Sys_UI.vmailFrame++; if (World.Sys_UI.vmailFrame > vmailEndFrames[World.Sys_UI.vmailActive]) World.Sys_UI.vmailFrame = vmailEndFrames[World.Sys_UI.vmailActive];
        }
        RenderUIImage(283,184,800,400,World.Sys_UI.vmailFrame); // Vmail viewer
    }
    i16 debugTextStartY = 48; /* Diagnostics / Debugging */
    if (Cheats.showLocation && !World.menuActive) RenderFormattedText(16, debugTextStartY, T_WHITE, FONT_NORMAL,1.0f, "x: %.4f, y: %.4f, z: %.4f, rx: %.4f, ry: %.4f, rz: %.4f, rw: %.4f",World.position[PLAYER1].x,World.position[PLAYER1].y,World.position[PLAYER1].z,World.rotation[PLAYER1].x,World.rotation[PLAYER1].y,World.rotation[PLAYER1].z,World.rotation[PLAYER1].w);
    i16 lineSpacing = 18;
    if (!World.menuActive && !Cheats.noHUD) RenderFormattedText(16,debugTextStartY + (lineSpacing * 1),T_WHITE,FONT_NORMAL,1.0f,"GPU ms::All:%.2f, Shad:%.2f, Pre:%.2f, Main:%.2f, SSR:%.2f, Comp:%.2f",World.gpuFrameMs,World.gpuShadowMs,World.gpuPreMs,World.gpuMainMs,World.gpuSsrMs,World.gpuCompMs);
    if (!World.menuActive && !Cheats.noHUD) RenderFormattedText(16,debugTextStartY + (lineSpacing * 2),T_WHITE,FONT_NORMAL,1.0f,"CPU ms::Shad:%.3f, Phys:%.3f, Subs:%u, Rend:%.3f, Pre Phys:%.3f, Logic:%.3f",shadowTime * 1000,physTime * 1000,World.substeps,renderTime * 1000,prePhys * 1000,gameTime * 1000);
    if (!World.menuActive && !Cheats.noHUD && !World.paused) RenderFormattedText(16,debugTextStartY + (lineSpacing * 3),T_WHITE,FONT_NORMAL,1.0f,"Grounded: %u  weaponCurrent: %d  weaponIndex: %d  pendingIdx: %d  wep16: %d  viewModel: %u  reloadDone: %.2f",(World.instances[PLAYER1].entflags & EF_GROUNDED) > 0,(int)World.invP1.weaponCurrent,(int)World.invP1.weaponIndex,(int)World.invP1.weaponIndexPending,Get16WeaponIndexFromConstIndex((int)World.invP1.weaponIndex),((Get16WeaponIndexFromConstIndex((int)World.invP1.weaponIndex)==5||Get16WeaponIndexFromConstIndex((int)World.invP1.weaponIndex)==6)?49u:((Get16WeaponIndexFromConstIndex((int)World.invP1.weaponIndex)==0||Get16WeaponIndexFromConstIndex((int)World.invP1.weaponIndex)==1)?50u:0u)),World.invP1.reloadFinished);
    if (!World.menuActive && !Cheats.noHUD) RenderFormattedText(16,debugTextStartY + (lineSpacing * 4),T_WHITE,FONT_NORMAL,1.0f,"Test Edx: %u, Time Elapsed: %.3f, Fatigue: %.2f, Sprinting: %u, wep x:%.2f y:%.2f z:%.2f",editModeTestEntityDefinition,World.pauseRelativeTime - game_actual_start_time,World.invP1.fatigue,Sprint(),World.weaponViewOffset.x,World.weaponViewOffset.y,World.weaponViewOffset.z);
    RenderFormattedText(16,debugTextStartY + (lineSpacing * 5),T_WHITE,FONT_NORMAL,1.0f,"Cursor: %d, %d  dx:%d dy:%d",World.cursorPos_x,World.cursorPos_y,World.currentMouse_dx,World.currentMouse_dy);
    if (Cheats.consoleActive) RenderFormattedText(16,0,T_WHITE,FONT_NORMAL,1.0f, "] %s",consoleEntryText);
    if (World.statusTextDecayFinished > World.current_time) RenderFormattedText(460,114,T_WHITE,FONT_NORMAL,1.0f, "%s",statusText);
    double time_now = get_time();
    if (Cheats.showFPS) {
        World.thisFrameTime = (time_now - World.last_time) * 1000.0; World.last_time = time_now; World.cpuFrameTime = World.cpuTime * 1000.0;
        u8 timingColor = (get_time() - World.current_time) > (World.thisFrameTime - 0.2) ? T_RED : T_WHITE;
        double avgCPU = 0.0;
        avgCPUt[avgCPUt_idx] = World.cpuFrameTime; avgCPUt_idx++; if (avgCPUt_idx >= AVG_CPU_TAPS) avgCPUt_idx = 0;
        u32 avgmax = globalframe > AVG_CPU_TAPS ? AVG_CPU_TAPS : globalframe;
        for (u32 i=0;i<avgmax;++i) avgCPU += avgCPUt[i];
        avgCPU /= (double)avgmax;
        RenderFormattedText(16 + 100, debugTextStartY - lineSpacing,timingColor,FONT_NORMAL,1.0f,"CPU avg %.2f",avgCPU); avgCPU = 0;
        RenderFormattedText(16, debugTextStartY - lineSpacing, timingColor,FONT_NORMAL,1.0f,"ms: %.2f",World.thisFrameTime);
        RenderFormattedText(16 + 250, debugTextStartY - lineSpacing,T_WHITE,FONT_NORMAL,1.0f,"(FPS:%d),Drwclls:%d [G:%d UI:%d Sh:%d] Vrt:%d E:%u|M:%u|P:%u",globalframesPerLastSecond,drawCalls,drawCallsNormal,uiDrawCalls,shadDrawCalls,vertsRendered,Cheats.editMode,World.menuActive,World.paused);
    }
    return time_now;
}
