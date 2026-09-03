// ui.c - User Interface(UI) aka HUD
extern float reloadTime[16];
//                         mk3,bls,drt,flch, ion,rpir,pipe,magn,magp,pstl,plsm,rail,riot,skrp,sprq,stun
u16 wepIconTexIndices[16]={584,636,819,1067,1068,1494,1072,1069,1070,1071,1073,1165,1989,1990,1991,1992};
const char* elevFloorLabels[14] = {"R","1","2","3","4","5","6","7","8","9","G1","G2","G4","C"};
u8 MFD_LefTab=0,MFD_CenterTab=0,MFD_RightTab=0;
void WeaponFireStartWeaponDip(float t);
void WeaponSelectSlot(int slot){
    int wi=(int)World.invP1.weaponInventoryIndices[slot]; if(wi<0||wi>=MAX_ENTITIES)return;
    if((int)World.invP1.weaponCurrent==slot)return;
    if(World.invP1.reloadFinished>World.pauseRelativeTime)return;
    World.invP1.weaponCurrentPending=(i16)slot;
    World.invP1.weaponIndexPending=(i16)wi;
    int w=Get16WeaponIndexFromConstIndex(wi);
    WeaponFireStartWeaponDip((w>=0&&w<16) ? reloadTime[w] : 0.5f);
}

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
    over = over || currentMenuItem == mindex; RenderTextL(xPosForLabel,y,over ? T_YELLOW : T_GREEN,FONT_NORMAL,1.0f,"%s %u",Sys_Text.stringTable[lingdex],*out); return changed;
}

u8 UI_MenuButton(i16 bX, i16 bY, u8 menuItem, i16 bW, i16 bH,  i16 tX, i16 tY, const char* text, i16 pX, i16 pY) {
    bool over = false; u8 retvalue = 0u;
    retvalue = UI_Button(bX,bY,bW,bH,&over,menuItem); if (!retvalue) retvalue = (MenuEnter() && currentMenuItem == menuItem);
    over = over || currentMenuItem == menuItem;
    RenderTextL(tX,tY,over ? T_STOPD_RED : T_RED_MENU,FONT_STOPD,1.5f,text); 
    RenderUIImage(pX,pY,40,40,over ? 1029 : 1028); // Menu pad
    return retvalue;
}

bool UI_Checkbox(i16 x, i16 y, i8 mitem, u16 textIdx, bool currentlyOn) {
    RenderUIImage(x,y,16,16,910); // Checkbox background
    bool over = false; bool changed = (UI_Button(x,y + 16,210,16,&over,mitem) || (MenuEnter() && currentMenuItem == mitem)); over = over || currentMenuItem == mitem;
    if (currentlyOn) RenderUIImage(x + 2,y + 2, 12,12, 912); // Checkbox check
    RenderTextL(x + 20,y,over ? T_YELLOW : T_GREEN,FONT_NORMAL,1.0f,Sys_Text.stringTable[textIdx]);
    return changed;
}

__attribute__((noinline)) void UI_HeaderText(i16 x, const char* text) { RenderTextL(x,50,T_GREEN_MENU_SHADOW,FONT_STOPD,1.75f,text); RenderTextL(x,46,T_GREEN_MENU_GLOW,FONT_STOPD,1.75f,text); RenderTextL(x,48,T_GREEN_MENU,FONT_STOPD,1.75f,text); }
void PlayGameMusic(); void PlayMenuMusic();
__attribute__((noinline)) void MenuGoBack() {
    if (returnToPause) { returnToPause = false; World.paused = true; World.menuActive = false; PlayGameMusic(); }
    if (currentMenuPage == Mpg_Singleplayer || currentMenuPage == Mpg_Multiplayer || currentMenuPage == Mpg_Options) currentMenuPage = Mpg_FrontPage;//News
    else if (currentMenuPage == Mpg_Load || currentMenuPage == Mpg_NewGame || currentMenuPage == Mpg_IntroVideo || currentMenuPage == Mpg_CreditsVideo) currentMenuPage = Mpg_Singleplayer;
}

static void CreateShadowBuffers() { shadowMapSSBO=MakeSSBO(&shadowMapSSBO,5,(MAX_SHADOWMAPS * (SHADOW_MAP_SIZE * SHADOW_MAP_SIZE * 6U)) * sizeof(u32),NULL,GL_STATIC_DRAW); shadowMapsIndirectionID=MakeSSBO(&shadowMapsIndirectionID,6,LIGHT_COUNT * sizeof(u32),NULL,GL_STATIC_DRAW); shadowBuffersCreated=true; }
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
        RenderTextL(1076,732,overBack ? T_STOPD_RED_HIGHLIGHT : T_RED_MENU,FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == Mpg_Multiplayer) {
        menuItemCount = 1; menuTabCount = 1;
        UI_HeaderText(266,/*"MULTIPLAYER"*/Sys_Text.stringTable[720]);
        RenderUIImage(1060,724, 84,36, 1252); // Back Button background
        bool overBack = false;
        if (UI_Button(1060,758, 84,32, &overBack, 0) || (MenuEnter() && currentMenuItem == 0)) MenuGoBack();
        overBack = overBack || currentMenuItem == 0;
        RenderTextL(1076,732,overBack ? T_STOPD_RED_HIGHLIGHT : T_RED_MENU,FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == Mpg_Options) {
        menuTabCount = 3;
        UI_HeaderText(238,/*"CONFIGURATION"*/Sys_Text.stringTable[745]);
        if (currentMenuTab != 0) RenderUIImage(179,220, 1001,548, 1030); // Config background
        if (currentMenuTab == 0) RenderUIImage(179,220, 1001,548, 1033); // Config background graphics (empty alpha center)
        RenderUIImage(520,196, 160,30, currentMenuTab == 2 ? 920 : 921); // Config tab unhighlighted
        if (UI_Button(520,196+30, 160,30, NULL, 2)) currentMenuTab = 2;
        RenderTextL(530,202,currentMenuTab == 2 ? T_YELLOW : T_GREEN,FONT_NORMAL,1.0f,/*"AUDIO / LANG"*/Sys_Text.stringTable[793]);
        RenderUIImage(354,196, 160,30, currentMenuTab == 1 ? 920 : 921); // Config tab unhighlighted
        if (UI_Button(354,196+30, 160,30, NULL, 1)) currentMenuTab = 1;
        RenderTextL(366,202,currentMenuTab == 1 ? T_YELLOW : T_GREEN,FONT_NORMAL,1.0f,/*"INPUT"*/Sys_Text.stringTable[792]);
        RenderUIImage(190,196, 160,30, currentMenuTab == 0 ? 920 : 921); // Config tab highlighted
        if (UI_Button(190,196+30, 160,30, NULL, 0)) currentMenuTab = 0;
        RenderTextL(200,202,currentMenuTab == 0 ? T_YELLOW : T_GREEN,FONT_NORMAL,1.0f,/*"GRAPHICS"*/Sys_Text.stringTable[791]);
        if (currentMenuTab == 0) {
            bool overRes = false, overFull = false, overChgM = false;
            menuItemCount = 11; // Graphics
            if (UI_Checkbox(200,500,0,Sys_Settings.ModelDetail ? /*High*/915 : /*No Detail Level Models*/914,Sys_Settings.ModelDetail)) { Sys_Settings.ModelDetail = Sys_Settings.ModelDetail ? 0u : 1u; SaveConfig(); }
            if (UI_Checkbox(200,530,1,/*"FXAA"*/780,Sys_Settings.FXAA)) { Sys_Settings.FXAA = Sys_Settings.FXAA ? 0u : 1u; SaveConfig(); }
            if (UI_Checkbox(200,560,2,Sys_Settings.Shadows ? /*Soft*/787 : /*No Shadows*/785,Sys_Settings.Shadows)) { Sys_Settings.Shadows = Sys_Settings.Shadows ? 0u : 1u; if (!shadowBuffersCreated) {CreateShadowBuffers();} SaveConfig(); }
            if (UI_Checkbox(200,590,3,/*SSR*/788,Sys_Settings.Reflections)) { Sys_Settings.Reflections = Sys_Settings.Reflections ? 0u : 1u; SaveConfig(); }
            if (UI_Checkbox(200,620,4,/*VSYNC*/1026,Sys_Settings.Vsync)) { Sys_Settings.Vsync = Sys_Settings.Vsync ? 0u : 1u; SetVSync(); SaveConfig(); }
            RenderTextL(310,620,T_GREEN,FONT_NORMAL,1.0f,"(FPS: %d)", globalframesPerLastSecond); // Helper to see vsync take effect.
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
                RenderTextL(200, 710, overRes ? T_YELLOW : T_GREEN,FONT_NORMAL, 1.0f, "RESOLUTION %s", resBuf);
            }
    
            // Fullscreen checkbox
            RenderUIImage(200,740, 16,16, 910); // Checkbox background
            if (UI_Button(200,756, 210,16, &overFull, 8) || (MenuEnter() && currentMenuItem == 8)) { Sys_Settings.Fullscreen = Sys_Settings.Fullscreen == 1u ? 0u : 1u; ChangeFullScreenWindowed(true); SaveConfig(); }
            overFull = overFull || currentMenuItem == 8;
            if (Sys_Settings.Fullscreen) RenderUIImage(202,742, 12,12, 912); // Checkbox check
            RenderTextL(220,740,overFull ? T_YELLOW : T_GREEN,FONT_NORMAL,1.0f,/*"Fullscreen"*/Sys_Text.stringTable[773]);
            RenderUIImage(588,730, 210,30, 1079); // Toggle monitor button background
            if (UI_Button(588,760, 210,30, &overChgM, 9) || (MenuEnter() && currentMenuItem == 9)) { CycleToNextMonitor(); }
            overChgM = overChgM || currentMenuItem == 9;
            RenderTextL(602,735,overChgM ? T_YELLOW : T_GREEN,FONT_NORMAL,1.0f,/*"CHANGE MONITOR"*/Sys_Text.stringTable[1025]);
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
        RenderTextL(1103,731,overBack ? T_STOPD_RED_HIGHLIGHT : T_RED_MENU,FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == Mpg_Load || currentMenuPage == Mpg_Save) {
        menuItemCount = 9; menuTabCount = 1; bool isSave = currentMenuPage == Mpg_Save;
        UI_HeaderText(isSave ? 284 : 340, isSave ? /*"SAVE GAME"*/Sys_Text.stringTable[769] : /*"LOAD"*/Sys_Text.stringTable[726]);
        RenderUIImage(400,214, 586,500, 1037); // Load/Save table background
        RenderUIImage(1060,724, 84,36, 1252); // Back Button background
        bool overBack = false;
        if (UI_Button(1060,758, 84,32, &overBack, 0) || (MenuEnter() && currentMenuItem == 0)) MenuGoBack();
        overBack = overBack || currentMenuItem == 0;
        RenderTextL(1076,732, overBack ? T_STOPD_RED_HIGHLIGHT : T_RED_MENU, FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == Mpg_NewGame) {
        menuItemCount = 7; menuTabCount = (currentMenuItem > 0 && currentMenuItem <= 16) ? 2 : 1;
        UI_HeaderText(290,/*"NEW GAME"*/Sys_Text.stringTable[741]);
        RenderUIImage(136,196,1088,558,1048); // Newgame inset
        RenderUIImage(136,196,1088,558,1049); // Newgame background
        if (UI_MenuButton(276,270,0,795,74, 226,146,/*"NAME:"*/Sys_Text.stringTable[746],299,214)) { /* Just for highlight */ }
        enteringPlayerName = (currentMenuItem == 0);
        if (World.playerName[0] == '\0') RenderTextL(642,232,T_RED_MENU,FONT_STOPD,1.0f,/*"ENTER NAME..."*/Sys_Text.stringTable[748]);
        else                                  RenderTextL(518,232,enteringPlayerName ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.0f,World.playerName);
        if (UI_MenuButton(174,377,1,496,95, 148,202,/*"COMBAT"*/Sys_Text.stringTable[748],185,299)) { World.diffCbt = World.diffCbt >= 3 ? 0 : World.diffCbt + 1; }  if (UI_MenuButton(704,377,3,496,95, 510,202,/*"MISSION"*/Sys_Text.stringTable[749],726,299)) { World.diffMis = World.diffMis >= 3 ? 0 : World.diffMis + 1; }
        RenderTextL(162,270,World.diffCbt == 0 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"0");    RenderTextL(513,270,World.diffMis == 0 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"0");
        RenderTextL(233,270,World.diffCbt == 1 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"1");    RenderTextL(584,270,World.diffMis == 1 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"1");
        RenderTextL(307,270,World.diffCbt == 2 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"2");    RenderTextL(658,270,World.diffMis == 2 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"2");
        RenderTextL(379,270,World.diffCbt == 3 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"3");    RenderTextL(730,270,World.diffMis == 3 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"3");
        if (UI_MenuButton(174,568,2,496,92, 149,330,/*"PUZZLE"*/Sys_Text.stringTable[751],185,490)) { World.diffPuz = World.diffPuz >= 3 ? 0 : World.diffPuz + 1; }  if (UI_MenuButton(704,568,4,496,92, 509,330,/*"CYBERSPACE"*/Sys_Text.stringTable[750],726,490)) { World.diffCyb = World.diffCyb >= 3 ? 0 : World.diffCyb + 1; }
        RenderTextL(162,399,World.diffPuz == 0 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"0");    RenderTextL(513,399,World.diffCyb == 0 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"0");
        RenderTextL(233,399,World.diffPuz == 1 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"1");    RenderTextL(584,399,World.diffCyb == 1 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"1");
        RenderTextL(307,399,World.diffPuz == 2 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"2");    RenderTextL(658,399,World.diffCyb == 2 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"2");
        RenderTextL(379,399,World.diffPuz == 3 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"3");    RenderTextL(730,399,World.diffCyb == 3 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"3");
        if (UI_Button(221,460,82,79,NULL,1)) {World.diffCbt =0; currentMenuItem=1; } if (UI_Button(330,460,82,79,NULL,1)) {World.diffCbt =1; currentMenuItem=1; } if (UI_Button(439,460,82,79,NULL,1)) {World.diffCbt =2; currentMenuItem=1; } if (UI_Button( 547,460,82,79,NULL,1)) {World.diffCbt =3; currentMenuItem=1; }
        if (UI_Button(221,651,82,79,NULL,2)) {World.diffPuz =0; currentMenuItem=2; } if (UI_Button(330,651,82,79,NULL,2)) {World.diffPuz =1; currentMenuItem=2; } if (UI_Button(439,651,82,79,NULL,2)) {World.diffPuz =2; currentMenuItem=2; } if (UI_Button( 547,651,82,79,NULL,2)) {World.diffPuz =3; currentMenuItem=2; }
        if (UI_Button(748,460,82,79,NULL,3)) {World.diffMis=0; currentMenuItem=3; } if (UI_Button(857,460,82,79,NULL,3)) {World.diffMis=1; currentMenuItem=3; } if (UI_Button(966,460,82,79,NULL,3)) {World.diffMis=2; currentMenuItem=3; } if (UI_Button(1074,460,82,79,NULL,3)) {World.diffMis=3; currentMenuItem=3; }
        if (UI_Button(748,651,82,79,NULL,4)) {World.diffCyb  =0; currentMenuItem=4; } if (UI_Button(857,651,82,79,NULL,4)) {World.diffCyb  =1; currentMenuItem=4; } if (UI_Button(966,651,82,79,NULL,4)) {World.diffCyb  =2; currentMenuItem=4; } if (UI_Button(1074,651,82,79,NULL,4)) {World.diffCyb  =3; currentMenuItem=4; }
        bool overBack = false, overStart = false;
        if (UI_Button(544,747, 282,68, &overStart, 5) || (MenuEnter() && currentMenuItem == 5)) GoIntoGame();
        overStart = overStart || currentMenuItem == 5;
        RenderTextL(400,464,overStart ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,/*"START"*/Sys_Text.stringTable[886]);
        if (UI_Button(1060,758, 84,32, &overBack, 6) || (MenuEnter() && currentMenuItem == 6)) MenuGoBack();
        overBack = overBack || currentMenuItem == 6;
        RenderUIImage(1060,724,84,36,1252); // Back Button background
        RenderTextL(1076,732,overBack ? T_STOPD_RED_HIGHLIGHT : T_RED_MENU,FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
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
    RenderTextL(610,210,T_STOPD_RED_PAUSETITLE,FONT_STOPD,1.0f,/*"PAUSED"*/Sys_Text.stringTable[724]);
    if (UI_Button(522,330, 322,52, &overResume, 0) || (MenuEnter() && currentMenuItem == 0)) World.paused = false;
    overResume = overResume || currentMenuItem == 0;
    RenderTextL(610,306,overResume ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.0f,/*"RESUME"*/Sys_Text.stringTable[725]);
    if (UI_Button(522,390, 322,52, &overLoad, 1) || (MenuEnter() && currentMenuItem == 1)) { currentMenuPage = Mpg_Load; PlayMenuMusic(); World.menuActive = true; returnToPause = true; }
    overLoad = overLoad || currentMenuItem == 1;
    RenderTextL(630,364, overLoad ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.0f,/*"LOAD"*/Sys_Text.stringTable[726]);
    if (UI_Button(522,450, 322,60, &overSave, 2) || (MenuEnter() && currentMenuItem == 2)) { currentMenuPage = Mpg_Save; PlayMenuMusic(); World.menuActive = true; returnToPause = true; }
    overSave = overSave || currentMenuItem == 2;
    RenderTextL(635,422,overSave ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.0f,/*"SAVE"*/Sys_Text.stringTable[727]);
    if (UI_Button(522,510, 322,60, &overOptions, 3) || (MenuEnter() && currentMenuItem == 3)) { currentMenuPage = Mpg_Options; PlayMenuMusic(); World.menuActive = true; returnToPause = true; }
    overOptions = overOptions || currentMenuItem == 3;
    RenderTextL(599,480,overOptions ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.0f,/*"OPTIONS"*/Sys_Text.stringTable[721]);
    if (UI_Button(522,570, 322,60, &overQuitMenu, 4) || (MenuEnter() && currentMenuItem == 4)) { PlayMenuMusic(); World.menuActive = true; currentMenuPage = Mpg_FrontPage; }
    overQuitMenu = overQuitMenu || currentMenuItem == 4;
    RenderTextL(546,538,overQuitMenu ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.0f,/*"QUIT TO MENU"*/Sys_Text.stringTable[728]);
    RenderUIImage(519,672,328,42,1252); // Pause Quit Game background
    if (UI_Button(522,714, 322,42, &overQuit, 5) || (MenuEnter() && currentMenuItem == 5)) OS_Exit(0);
    overQuit = overQuit || currentMenuItem == 5;
    RenderTextL(572,690,overQuit ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.0f,/*"QUIT GAME"*/Sys_Text.stringTable[729]);
}

void GetWeaponAmmoText(int slot,char* buf,size_t bufSize) {
    buf[0] = '\0'; int wepIdx = World.invP1.weaponInventoryIndices[slot]; bool alt = World.invP1.wepLoadedWithAlternate[slot]; float heat = World.invP1.currentEnergyWeaponHeat[slot];
    u8 mag = alt ? World.invP1.currentMagazineAmount2[slot] : World.invP1.currentMagazineAmount[slot];
    switch(wepIdx) {
        case 343: if (alt){sFormat(buf,bufSize,"%upn | %umg, %upn",mag,World.invP1.wepAmmo[0],World.invP1.wepAmmoSecondary[0]);}else{sFormat(buf,bufSize,"%umg | %umg, %upn",mag,World.invP1.wepAmmo[0],World.invP1.wepAmmoSecondary[0]);} break; // MK3 Assault Rifle
        case 344: case 347: case 353: case 357: case 358: scpy_to_a_from_b(buf,heat > 80.0f ? Sys_Text.stringTable[14] : Sys_Text.stringTable[15],bufSize); break; // Energy weapons
        case 345: if (alt){sFormat(buf,bufSize,"%utq | %und, %utq",mag,World.invP1.wepAmmo[2],World.invP1.wepAmmoSecondary[2]);}else{sFormat(buf,bufSize,"%und | %und, %utq",mag,World.invP1.wepAmmo[2],World.invP1.wepAmmoSecondary[2]);} break; // SV-23 Dartgun
        case 346: if (alt){sFormat(buf,bufSize,"%usp | %uhn, %usp",mag,World.invP1.wepAmmo[3],World.invP1.wepAmmoSecondary[3]);}else{sFormat(buf,bufSize,"%uhn | %uhn, %usp",mag,World.invP1.wepAmmo[3],World.invP1.wepAmmoSecondary[3]);} break; // AM-27 Flechette
        case 348: case 349: break; // Laser Rapier / Lead Pipe: no ammo
        case 350: if (alt){sFormat(buf,bufSize,"%usg | %uhw, %usg",mag,World.invP1.wepAmmo[7],World.invP1.wepAmmoSecondary[7]);}else{sFormat(buf,bufSize,"%uhw | %uhw, %usg",mag,World.invP1.wepAmmo[7],World.invP1.wepAmmoSecondary[7]);} break; // Magnum 2100
        case 351: if (alt){sFormat(buf,bufSize,"%usu | %ucr, %usu",mag,World.invP1.wepAmmo[8],World.invP1.wepAmmoSecondary[8]);}else{sFormat(buf,bufSize,"%ucr | %ucr, %usu",mag,World.invP1.wepAmmo[8],World.invP1.wepAmmoSecondary[8]);} break; // SB-20 Magpulse
        case 352: if (alt){sFormat(buf,bufSize,"%utf | %ust, %utf",mag,World.invP1.wepAmmo[9],World.invP1.wepAmmoSecondary[9]);}else{sFormat(buf,bufSize,"%ust | %ust, %utf",mag,World.invP1.wepAmmo[9],World.invP1.wepAmmoSecondary[9]);} break; // ML-41 Pistol
        case 354: sFormat(buf,bufSize,"%url | %url",World.invP1.currentMagazineAmount[slot],World.invP1.wepAmmo[11]); break; // MM-76 Railgun
        case 355: sFormat(buf,bufSize,"%urb | %urb",World.invP1.currentMagazineAmount[slot],World.invP1.wepAmmo[12]); break; // DC-05 Riotgun
        case 356: if (alt){sFormat(buf,bufSize,"%ulg | %usm, %ulg",mag,World.invP1.wepAmmo[13],World.invP1.wepAmmoSecondary[13]);}else{sFormat(buf,bufSize,"%usm | %usm, %ulg",mag,World.invP1.wepAmmo[13],World.invP1.wepAmmoSecondary[13]);} break; // RF-07 Skorpion
        default: break;
    }
}

void TickBar(bool isEnergy) {
    RenderUIImage(isEnergy ? 1333 : 1332,isEnergy ? 36 : 2,32,32,isEnergy ? 939 : 956);/*Indicator*/ int p1H = isEnergy ? World.invP1.energy : World.instances[PLAYER1].health; if (p1H > 255){p1H = 255;} i16 tY = isEnergy ? 35 : 4;
    for (int i=7;i>=0;--i) if (i == 7/*Always render at least 1 tick*/ || p1H > (7 - i) * 11){RenderUIImage(1050 - (i * 16),tY,32,32,964);/*Tick Red*/} for (int i=7;i>=0;--i) if (p1H > 88 + (7 - i) * 11){RenderUIImage(1178 - (i * 16),tY,32,32,963);/*Tick Orange*/} for (int i=7;i>=0;--i) if (p1H > 176 + (7 - i) * 11){RenderUIImage(1306 - (i * 16),tY,32,32,962);/*Tick Green*/}
}

void BioMonitorClearGraphs(void);
INLINE bool HwBtnClick(float x0, float x1, float y0, float y1) {
    if (!World.inventoryMode || !CursorIsOverBounds(x0,x1,y0,y1)){return false;} if (!(Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed || Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].pressed)){return false;}
    World.Sys_UI.mouseClickHeldOverGUI=World.uiIsBlocking=true; Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed=Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].pressed=false; return true;
}

INLINE int HwActiveTexIndex(int active, int version, int off, int v1, int v2, int v3, int v4) { if (!active) return off; if (v4 >= 0 && version >= 4) return v4; if (version >= 3) return v3; if (version == 2) return v2; return v1; }
void HardwareButtons() {
    if(Cheats.noHUD){return;} bool noEng=World.invP1.energy<=0.0f, hasBio=World.invP1.hasHardware & HW_BIO, bioOn=World.invP1.hardwareIsActive & HW_BIO, snsOn=World.invP1.hardwareIsActive & HW_SNS, lanOn=World.invP1.hardwareIsActive & HW_LAN, shdOn=World.invP1.hardwareIsActive & HW_SHD, infOn=World.invP1.hardwareIsActive & HW_INF;
    if (hasBio) { RenderUIImage(0,180,40,40,HwActiveTexIndex(bioOn,World.invP1.hwVers[HW_BIO_IDX],989,991,992,992,992)); if (HwBtnClick(0,40,180,220)) { if (World.invP1.hwVersSetting[HW_BIO_IDX] == 0 && noEng) CenterStatusPrint("%s",Sys_Text.stringTable[314]); else { play_wav(sounds[78],SfxVol(),(V3){0.0f,0.0f,0.0f},false); if (hasBio && bioOn) { World.invP1.hardwareIsActive &= ~HW_BIO; if (!Cheats.showFPS) BioMonitorClearGraphs(); } else World.invP1.hardwareIsActive |= HW_BIO; } } }
    if (World.invP1.hasHardware & HW_SNS) { RenderUIImage(0,240,40,40,HwActiveTexIndex(snsOn,World.invP1.hwVers[HW_SNS_IDX],1009,1011,1012,1013,1013)); if (HwBtnClick(0,40,240,280)) { if (noEng) CenterStatusPrint("%s",Sys_Text.stringTable[314]); else if (snsOn) { play_wav(sounds[82],SfxVol(),(V3){0.0f,0.0f,0.0f},false); World.invP1.hardwareIsActive &= ~HW_SNS; } else { play_wav(sounds[93],SfxVol(),(V3){0.0f,0.0f,0.0f},false); World.invP1.hardwareIsActive |= HW_SNS; } } }
    if (World.invP1.hasHardware & HW_LAN) { RenderUIImage(0,300,40,40,HwActiveTexIndex(lanOn,World.invP1.hwVers[HW_LAN_IDX],1004,1006,1007,1008,1008)); if (HwBtnClick(0,40,300,340)) { if (noEng) CenterStatusPrint("%s",Sys_Text.stringTable[314]); else { play_wav(sounds[78],SfxVol(),(V3){0.0f,0.0f,0.0f},false); if (lanOn) World.invP1.hardwareIsActive &= ~HW_LAN; else World.invP1.hardwareIsActive |= HW_LAN; } } }
    if (World.invP1.hasHardware & HW_SHD) { RenderUIImage(0,360,40,40,HwActiveTexIndex(shdOn,World.invP1.hwVers[HW_SHD_IDX],1014,1015,1016,1017,1018)); if (HwBtnClick(0,40,360,400)) { if (noEng) CenterStatusPrint("%s",Sys_Text.stringTable[314]); else if (shdOn) { play_wav(sounds[95],SfxVol(),(V3){0.0f,0.0f,0.0f},false); World.invP1.hardwareIsActive &= ~HW_SHD; } else { play_wav(sounds[96],SfxVol(),(V3){0.0f,0.0f,0.0f},false); World.invP1.hardwareIsActive |= HW_SHD; } } }
    if (World.invP1.hasHardware & HW_INF) { RenderUIImage(1326,180,40,40,HwActiveTexIndex(infOn,World.invP1.hwVers[HW_INF_IDX],998,999,999,999,999)); if (HwBtnClick(1326,1366,180,220)) { if (noEng) CenterStatusPrint("%s",Sys_Text.stringTable[314]); else { bool wasOn=(infOn) != 0; play_wav(wasOn ? sounds[82] : sounds[98],SfxVol(),(V3){0.0f,0.0f,0.0f},false); if (wasOn) World.invP1.hardwareIsActive &= ~HW_INF; else World.invP1.hardwareIsActive |= HW_INF; } } }
    if (World.invP1.hasHardware & HW_ERD) { RenderUIImage(1326,240,40,40,World.inventoryMode && CursorIsOverBounds(1326,1366,240,280) && (Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].down || Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].down) ? 997 : ((World.invP1.hasNewEmail || World.invP1.hasNewLogs) && ((int)World.pauseRelativeTime & 1)) ? 997 : 996); if (HwBtnClick(1326,1366,240,280)) { play_wav(sounds[97],SfxVol(),(V3){0.0f,0.0f,0.0f},false); MFD_CenterTab = 4; MFD_LefTab = 1; } }
    if (World.invP1.hasHardware & HW_BST) { RenderUIImage(1326,300,40,40,HwActiveTexIndex(World.invP1.hardwareIsActive & HW_BST,World.invP1.hwVers[HW_BST_IDX],993,994,995,995,995)); if (HwBtnClick(1326,1366,300,340)) { if (World.invP1.hwVersSetting[HW_BST_IDX] >= 1 && noEng) CenterStatusPrint("%s",Sys_Text.stringTable[314]); else { play_wav(sounds[78],SfxVol(),(V3){0.0f,0.0f,0.0f},false); if (World.invP1.hardwareIsActive & HW_BST) World.invP1.hardwareIsActive &= ~HW_BST; else World.invP1.hardwareIsActive |= HW_BST; } } }
    if (World.invP1.hasHardware & HW_JET) { RenderUIImage(1326,360,40,40,HwActiveTexIndex(World.invP1.hardwareIsActive & HW_JET,World.invP1.hwVers[HW_JET_IDX],1000,1001,1002,1003,1003)); if (HwBtnClick(1326,1366,360,400)) { if (noEng) CenterStatusPrint("%s",Sys_Text.stringTable[314]); else { play_wav(sounds[78],SfxVol(),(V3){0.0f,0.0f,0.0f},false); World.invP1.hardwareIsActive ^= HW_JET; } } }
}

void AddItemToInventory(int index, int custIdx); void ResetHeldItem();
void CenterMFD() {
    RenderUIImage(400,752,64,32,MFD_CenterTab == 0 ? 1024 : 1021);/*Main center tab button*/ RenderUIImage(480,752,64,32,MFD_CenterTab == 1 ? 1024 : 1021);/*Hardware center tab button*/ RenderUIImage(560,752,64,32,MFD_CenterTab == 2 ? 1024 : 1021);/*General center tab button*/ RenderUIImage(902,752,64,32,MFD_CenterTab == 3 ? 1024 : 1021);/*Software center tab button*/
    if (World.inventoryMode && World.invP1.holdingObject && CursorIsOverBounds(345,1021,460,768)) { // Add to Inventory Helper
        World.uiIsBlocking = true; RenderUIImage(345,528,676,240,1075); RenderTextL(586,528,T_GREEN,FONT_NORMAL,1.0f,"ADD TO INVENTORY");
        if (Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed || Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].pressed) { AddItemToInventory(World.invP1.heldObjectIndex,World.invP1.heldObjectCustIdx); ResetHeldItem(); Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed = Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].pressed = false; }
    }
    //if (World.Sys_UI.showSensaroundCenter) { /*SensaroundCenter Plane*/ } TODO
    if(MFD_CenterTab==0) return; // Tabs are off.
    if(MFD_CenterTab==1 && !Cheats.noHUD){ /*MainTab: WeaponInventory,WeaponShotsInventory,GrenadeInventory,PatchInventory*/
        RenderTextL(372,560,T_RED,FONT_NORMAL,0.8f,"WEAPONS"); RenderTextL(574,560,T_RED,FONT_NORMAL,0.8f,"SHOTS"); RenderTextL(768,560,T_RED,FONT_NORMAL,0.8f,"GRENADES"); RenderTextL(920,560,T_RED,FONT_NORMAL,0.8f,"PATCHES"); // Column headers
        for(int slot=0;slot<7;++slot){
            int widx=World.invP1.weaponInventoryIndices[slot]; if(widx<0)continue;
            int y=582+slot*22;
            bool hov = CursorIsOverBounds(372,712,(float)y-5,(float)y+16); // Slight shift of 6 feels better than just doing y and y + 22 as one would expect, then lopped 1 off one end to prevent double highlighting
            u32 col = (hov&&World.inventoryMode && World.invP1.weaponCurrent!=slot) ? T_GREEN_MENU : (World.invP1.weaponCurrent==slot?T_YELLOW:(World.invP1.weaponCurrentPending==slot?T_DARK_YELLOW:T_GREEN));
            RenderTextL(372,y,col,FONT_NORMAL,0.8f,"%s",Sys_Text.stringTable[ItemStringIdx((i32)widx)]); // Weapon text
            char b[64]; GetWeaponAmmoText(slot,b,sizeof(b)); RenderTextL(574,y,col,FONT_NORMAL,0.8f,"%s",b); // Ammo text
            if(hov&&World.inventoryMode&&Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed){WeaponSelectSlot(slot);Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed=false; World.uiIsBlocking=true;}
        }
    } else if (MFD_CenterTab == 2) { /*HardwareTab: Label, HardwareInventory*/ }
    else if (MFD_CenterTab == 3) { /*GeneralTab: Label, GeneralInventory, AccessCards*/ }
    else if (MFD_CenterTab == 4) { /*SoftwareTab: Label, SoftwareInventory, ICEDrill, Pulser, Turbo, Decoy, Recall*/ }
    else if (MFD_CenterTab == 5) { /*MultiMediaDataReader: LogTableofContents, LogsLevelFolder, LogTextReader, EmailTab, DataTab, NotesTab*/ }
}

void SideMFD(bool isRH) {
    int wep16 = Get16WeaponIndexFromConstIndex(World.invP1.weaponIndex), tab = isRH ? MFD_RightTab : MFD_LefTab;
    RenderUIImage(isRH ? 1350 : -16,520,32,40,tab == 0 ? 1024 : 1022); // Weapon side tab button
    RenderUIImage(isRH ? 1350 : -16,576,32,40,tab == 1 ? 1024 : 1022); // Item side tab button
    RenderUIImage(isRH ? 1350 : -16,632,32,40,tab == 2 ? 1024 : 1022); // Automap side tab button
    RenderUIImage(isRH ? 1350 : -16,688,32,40,tab == 3 ? 1024 : 1022); // Data side tab button
    if ((World.invP1.hardwareIsActive & HW_SNS) && World.invP1.hwVers[HW_SNS_IDX] > 1) { /*TODO Sensaround Plane*/ }
     if (tab == 0){return;} 
    //RenderUIImage(isRH ? 1022 : 24,520,320,240,1025); // TODO REMOVE Test BG for ensuring fit into 320x240 to match 1:1 scale that Doom's 320x200 would map to after 4:3 scaling applied (since the CRT's had non-square pixels that stretched 320x200 into 320x240 space, ish) TODO gate by search active
    if (tab == 1) { /*WeaponTabLH: WepNameTextLH, WepIconLH, ClipBox, EnergyHeatTicks, ReloadButtons, EnergySlider*/
        int widx=World.invP1.weaponInventoryIndices[World.invP1.weaponCurrent];
        if (widx >= 0) { RenderTextL(isRH ? 1342 : 24,520,T_RED,FONT_NORMAL,0.8f,"%s",Sys_Text.stringTable[ItemStringIdx((i32)widx)]);/*Weapon Name*/ if (wep16 >=0 && wep16 < 16){RenderUIImage(isRH ? 1207 : 24,548,270,100,wepIconTexIndices[wep16]);/*WepIconLH*/} }
    } else if (tab == 2) { /*ItemTab: ItemIcon, ItemText, Vaporize/Apply/Use Buttons, EReaderSections, AccessCardsList*/ }
    else if (tab == 3) { /*AutomapTab: AutomapMask, Overlays, PlayerIcon, ZoomIn/Out/Full/Side Buttons*/ }
    else if (tab == 4) { /*TargetTab*/ }
    else if (tab == 5) { /*DataTab: Security, DataHeaders, ElevatorUIControl, KeycodeUIControl, SearchContents, AudioLogInfo, PuzzleGrid, PuzzleWire, SystemAnalyzer Display*/ }
}

static const u16 vmailStartFrames[6]={1579,1645,1713,1784,1864,1931}; static const u16 vmailEndFrames[6]={1644,1712,1783,1863,1930,1988}; double avgCPUt[AVG_CPU_TAPS]={0}; int avgCPUt_idx = 0;
i32 tWrnTextIdx[10],tWrnTextIdx2[10],tWrnTextIdx3[10],tWrnColorIdx[10]; double tWrnFinished[10];
void AppendTextWarning(i32 sidx, i32 sidx2, i32 sidx3, i32 col, i32 id) { tWrnTextIdx[id]=sidx; tWrnTextIdx2[id]=sidx2; tWrnTextIdx3[id]=sidx3; tWrnFinished[id]=tWrnFinished[id] < World.pauseRelativeTime ? World.pauseRelativeTime + 0.1f : tWrnFinished[id] + 0.1f; tWrnColorIdx[id] = col; }
extern double game_actual_start_time; extern u16 editModeTestEntityDefinition;
static double RenderUI() {
    drawCallsNormal = drawCalls;
    World.uiIsBlocking = false;
    if (World.creditsActive) { // Render Credits
        if (Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed) { ++World.creditsPageIndex; if(World.creditsPageIndex > CREDITS_PAGES){World.creditsActive=false; return get_time();} /*Finished with Erthang!  That's it, go home.*/ }
        if (World.creditsPageIndex == 1) { CreditsStats(); RenderTextL(300,10,T_WHITE,FONT_NORMAL,1.0f,(const char*)&creditStats); } else {RenderTextL(300,10,T_WHITE,FONT_NORMAL,1.0f,creditPages[World.creditsPageIndex]);}
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
        if (!Cheats.noHUD) {
            TickBar(false/*health*/); TickBar(true/*energy*/);/*Health and Energy Bars*/ HardwareButtons();
            RenderUIImage(667,0,32,32,1020);/*ShootModeButton*/
            if (World.inventoryMode && CursorIsOverBounds(667,699,0,32)) { /*ShootModeButton*/ World.uiIsBlocking = true; if (Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed || Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].pressed) { ForceShootMode(); Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed = Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].pressed = false; } }
            for (int i=0;i<10;++i){ if (tWrnFinished[i] > World.pauseRelativeTime){
                char flt[6]; if(tWrnTextIdx[i] == 185){sFormat(flt,6,"%.1f",(double)World.instances[PLAYER1].radiation);} RenderTextL(340,72+(i*18),tWrnColorIdx[i],FONT_NORMAL,0.8f,"%s%s%s",Sys_Text.stringTable[tWrnTextIdx[i]],tWrnTextIdx[i] == 185 ? flt : tWrnTextIdx2[i] >= 0 ? Sys_Text.stringTable[tWrnTextIdx2[i]] : "",tWrnTextIdx3[i] >= 0 ? Sys_Text.stringTable[tWrnTextIdx3[i]] : "");
            } /*Text Warnings System (e.g. radiation hazard + biohazard), stacks with timeout*/}
        }
//         if (World.Sys_UI.showTeleportFX) { /*TeleportFX*/ } if (World.Sys_UI.showRadiationFX) { /*RadiationFX*/ } if (World.Sys_UI.showHealingFX) { /*HealingFX*/ } if (World.Sys_UI.showShieldFX) { /*ShieldFX*/ } 
//         if (World.Sys_UI.showShieldActivation) { /*waveup*/ /*wavedn*/ } if (World.Sys_UI.showShieldDeactivation) { /*waveup*/ /*wavedn*/ } if (World.Sys_UI.showDeathRessurectionFX) { /*spawndelaycontainers...*/ }
//         if (World.Sys_UI.showAutomapFull) { /*AutomapFullRawImage*/ /*PlayerIconFull*/ /*CloseFullmapButton*/ } if (World.Sys_UI.showMissionTimer) { /*MissionTimerT*/ /*MissionTimer*/ }
//         if (World.Sys_UI.showCyberTimer) { /*CyberTimerT*/ /*CyberTimer*/ }
        if(!Cheats.noHUD){SideMFD(false/*Left*/); CenterMFD(); SideMFD(true/*Right*/);} // MFD
    }
    if (World.Sys_UI.vmailActive) {
        if (World.Sys_UI.vmailFrameFinished < World.pauseRelativeTime && World.Sys_UI.vmailFrame < vmailEndFrames[World.Sys_UI.vmailActive]) {
            if (World.Sys_UI.vmailFrame == (vmailStartFrames[World.Sys_UI.vmailActive]+11)) play_wav(sounds[99],1.0f,(V3){0,0,0},false);
            World.Sys_UI.vmailFrameFinished=World.pauseRelativeTime + 0.1; World.Sys_UI.vmailFrame++; if (World.Sys_UI.vmailFrame > vmailEndFrames[World.Sys_UI.vmailActive]) World.Sys_UI.vmailFrame = vmailEndFrames[World.Sys_UI.vmailActive];
        }
        RenderUIImage(283,184,800,400,World.Sys_UI.vmailFrame); // Vmail viewer
    }
    i16 debugTextStartY = 48; /* Diagnostics / Debugging */
    if (Cheats.showLocation && !World.menuActive) RenderTextL(16, debugTextStartY, T_WHITE, FONT_NORMAL,1.0f, "x: %.4f, y: %.4f, z: %.4f, rx: %.4f, ry: %.4f, rz: %.4f, rw: %.4f",World.position[PLAYER1].x,World.position[PLAYER1].y,World.position[PLAYER1].z,World.rotation[PLAYER1].x,World.rotation[PLAYER1].y,World.rotation[PLAYER1].z,World.rotation[PLAYER1].w);
    i16 lineSpacing = 18;
    if (!World.menuActive && !Cheats.noHUD && Cheats.showFPS) RenderTextL(16,debugTextStartY + (lineSpacing * 1),T_WHITE,FONT_NORMAL,1.0f,"GPU ms::All:%.2f, Shad:%.2f, Pre:%.2f, Main:%.2f, SSR:%.2f, Comp:%.2f",World.gpuFrameMs,World.gpuShadowMs,World.gpuPreMs,World.gpuMainMs,World.gpuSsrMs,World.gpuCompMs);
    if (!World.menuActive && !Cheats.noHUD && Cheats.showFPS) RenderTextL(16,debugTextStartY + (lineSpacing * 2),T_WHITE,FONT_NORMAL,1.0f,"CPU ms::Shad:%.3f, Phys:%.3f, Subs:%u, Rend:%.3f, Pre Phys:%.3f, Logic:%.3f",shadowTime * 1000,physTime * 1000,World.substeps,renderTime * 1000,prePhys * 1000,gameTime * 1000);
    if (!World.menuActive && !Cheats.noHUD && !World.paused && Cheats.showFPS) RenderTextL(16,debugTextStartY + (lineSpacing * 3),T_WHITE,FONT_NORMAL,1.0f,"Grounded: %u  weaponCurrent: %d  weaponIndex: %d  pendingIdx: %d  wep16: %d  viewModel: %u  reloadDone: %.2f",(World.instances[PLAYER1].entflags & EF_GROUNDED) > 0,(int)World.invP1.weaponCurrent,(int)World.invP1.weaponIndex,(int)World.invP1.weaponIndexPending,Get16WeaponIndexFromConstIndex((int)World.invP1.weaponIndex),((Get16WeaponIndexFromConstIndex((int)World.invP1.weaponIndex)==5||Get16WeaponIndexFromConstIndex((int)World.invP1.weaponIndex)==6)?49u:((Get16WeaponIndexFromConstIndex((int)World.invP1.weaponIndex)==0||Get16WeaponIndexFromConstIndex((int)World.invP1.weaponIndex)==1)?50u:0u)),World.invP1.reloadFinished);
    if (!World.menuActive && !Cheats.noHUD && Cheats.showFPS) RenderTextL(16,debugTextStartY + (lineSpacing * 4),T_WHITE,FONT_NORMAL,1.0f,"Test Edx: %u, Time Elapsed: %.3f, Fatigue: %.2f, Sprinting: %u",editModeTestEntityDefinition,World.pauseRelativeTime - game_actual_start_time,World.invP1.fatigue,Sprint());
    if (!Cheats.noHUD && Cheats.showLocation) RenderTextL(16,debugTextStartY + (lineSpacing * 5),T_WHITE,FONT_NORMAL,1.0f,"Cursor: %d, %d  dx:%d dy:%d",World.cursorPos_x,World.cursorPos_y,World.currentMouse_dx,World.currentMouse_dy);
    if (Cheats.consoleActive) RenderTextL(16,0,T_WHITE,FONT_NORMAL,1.0f, "] %s",consoleEntryText);
    if (World.statusTextDecayFinished > World.current_time) RenderTextC(683,164,T_WHITE,FONT_NORMAL,1.0f, "%s",statusText);
    double time_now = get_time();
    if (Cheats.showFPS) {
        World.thisFrameTime = (time_now - World.last_time) * 1000.0; World.last_time = time_now; World.cpuFrameTime = World.cpuTime * 1000.0;
        u8 timingColor = (get_time() - World.current_time) > (World.thisFrameTime - 0.2) ? T_RED : T_WHITE;
        double avgCPU = 0.0;
        avgCPUt[avgCPUt_idx] = World.cpuFrameTime; avgCPUt_idx++; if (avgCPUt_idx >= AVG_CPU_TAPS) avgCPUt_idx = 0;
        u32 avgmax = globalframe > AVG_CPU_TAPS ? AVG_CPU_TAPS : globalframe;
        for (u32 i=0;i<avgmax;++i) avgCPU += avgCPUt[i];
        avgCPU /= (double)avgmax;
        RenderTextL(16 + 100, debugTextStartY - lineSpacing,timingColor,FONT_NORMAL,1.0f,"CPU avg %.2f",avgCPU); avgCPU = 0;
        RenderTextL(16, debugTextStartY - lineSpacing, timingColor,FONT_NORMAL,1.0f,"ms: %.2f",World.thisFrameTime);
        RenderTextL(16 + 250, debugTextStartY - lineSpacing,T_WHITE,FONT_NORMAL,1.0f,"(FPS:%d),Drwclls:%d [G:%d UI:%d Sh:%d] Vrt:%d E:%u|M:%u|P:%u",globalframesPerLastSecond,drawCalls,drawCallsNormal,uiDrawCalls,shadDrawCalls,vertsRendered,Cheats.editMode,World.menuActive,World.paused);
    }
    if ((World.inventoryMode && !Cheats.noHUD) || World.menuActive || World.paused){RenderUIImage((i16)(World.cursorPos_x) - 20,(i16)(World.cursorPos_y) - 20,40,40,GetCursorTexture());}else if (!Cheats.noHUD){RenderUIImage(663,364,40,40,GetCursorTexture());} // Centered on UI fixed resolution 1366x768 FBO
    return time_now;
}
