// ui.c - User Interface(UI) aka HUD
extern float reloadTime[16];
//                         mk3,bls,drt,flch, ion,rpir,pipe,magn,magp,pstl,plsm,rail,riot,skrp,sprq,stun
u16 wepIconTexIndices[16]={584,636,819,1067,1068,1494,1072,1069,1070,1071,1073,1165,1989,1990,1991,1992};
const char* elevFloorLabels[14] = {"R","1","2","3","4","5","6","7","8","9","G1","G2","G4","C"};
u8 MFD_LefTab=0,MFD_CenterTab=0,MFD_RightTab=0;
u8 MFD_DataL=0,MFD_DataR=0; // DataTab subview per side: 0=none,1=elevator,2=keycode,3=grid,4=wire,5=search,6=audiolog,7=sysanalyzer,8=blocked,9=minigames
u8 MFD_MediaTab=1; // EReader table: MM_EMAIL_TABLE=0,MM_LOG_TABLE=1,MM_DATA_TABLE=2,MM_NOTES=3
u8 MFD_ReaderView=MFD_READER_CONTENTS;
static u8 mfdSelected[3]={1,1,1},mfdReturnTab[3]={1,1,1},mfdReturnView[3],mfdItemReader[2];
INLINE void MFD_SelectTab(u8 panel,u8 tab,bool toggle) {
    u8* current=panel==0?&MFD_CenterTab:panel==1?&MFD_LefTab:&MFD_RightTab;
    *current=toggle && *current==tab ? 0 : tab; mfdSelected[panel]=tab;
    if (!(panel && tab==2 && mfdItemReader[panel-1])) { mfdReturnTab[panel]=*current; mfdReturnView[panel]=panel==0?0:panel==1?MFD_DataL:MFD_DataR; }
    play_wav(sounds[97],SfxVol(),(V3){0,0,0},false);
}
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
void PlayMenuMusic(),mp3_clear();
__attribute__((noinline)) void MenuGoBack() {
    if (returnToPause) { returnToPause = false; World.paused = true; World.menuActive = false; mp3_clear(); }
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
    if (World.invP1.hasHardware & HW_ERD) { RenderUIImage(1326,240,40,40,World.inventoryMode && CursorIsOverBounds(1326,1366,240,280) && (Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].down || Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].down) ? 997 : ((World.invP1.hasNewEmail || World.invP1.hasNewLogs) && ((int)World.pauseRelativeTime & 1)) ? 997 : 996); }
    if (World.invP1.hasHardware & HW_BST) { RenderUIImage(1326,300,40,40,HwActiveTexIndex(World.invP1.hardwareIsActive & HW_BST,World.invP1.hwVers[HW_BST_IDX],993,994,995,995,995)); if (HwBtnClick(1326,1366,300,340)) { if (World.invP1.hwVersSetting[HW_BST_IDX] >= 1 && noEng) CenterStatusPrint("%s",Sys_Text.stringTable[314]); else { play_wav(sounds[78],SfxVol(),(V3){0.0f,0.0f,0.0f},false); if (World.invP1.hardwareIsActive & HW_BST) World.invP1.hardwareIsActive &= ~HW_BST; else World.invP1.hardwareIsActive |= HW_BST; } } }
    if (World.invP1.hasHardware & HW_JET) { RenderUIImage(1326,360,40,40,HwActiveTexIndex(World.invP1.hardwareIsActive & HW_JET,World.invP1.hwVers[HW_JET_IDX],1000,1001,1002,1003,1003)); if (HwBtnClick(1326,1366,360,400)) { if (noEng) CenterStatusPrint("%s",Sys_Text.stringTable[314]); else { play_wav(sounds[78],SfxVol(),(V3){0.0f,0.0f,0.0f},false); World.invP1.hardwareIsActive ^= HW_JET; } } }
}

void AddItemToInventory(int index, int custIdx); void ResetHeldItem();
void CenterMFD() {
    RenderUIImage(400,752,64,32,mfdSelected[0] == 1 && MFD_CenterTab!=5 ? 1024 : 1021);/*Main center tab button*/ RenderUIImage(480,752,64,32,mfdSelected[0] == 2 && MFD_CenterTab!=5 ? 1024 : 1021);/*Hardware center tab button*/ RenderUIImage(560,752,64,32,mfdSelected[0] == 3 && MFD_CenterTab!=5 ? 1024 : 1021);/*General center tab button*/ RenderUIImage(902,752,64,32,mfdSelected[0] == 4 && MFD_CenterTab!=5 ? 1024 : 1021);/*Software center tab button*/

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
    u8 selected=tab?tab:mfdSelected[isRH?2:1];
    RenderUIImage(isRH ? 1350 : -16,520,32,40,selected == 1 ? 1024 : 1022); // Weapon side tab button

    RenderUIImage(isRH ? 1350 : -16,576,32,40,selected == 2 ? 1024 : 1022); // Item side tab button

    RenderUIImage(isRH ? 1350 : -16,632,32,40,selected == 3 ? 1024 : 1022); // Automap side tab button

    RenderUIImage(isRH ? 1350 : -16,688,32,40,selected == 5 ? 1024 : 1022); // Data side tab button

    if ((World.invP1.hardwareIsActive & HW_SNS) && World.invP1.hwVers[HW_SNS_IDX] > 1) { /*TODO Sensaround Plane*/ }
     if (tab == 0){return;} 
    //RenderUIImage(isRH ? 1022 : 24,520,320,240,1025); // TODO REMOVE Test BG for ensuring fit into 320x240 to match 1:1 scale that Doom's 320x200 would map to after 4:3 scaling applied (since the CRT's had non-square pixels that stretched 320x200 into 320x240 space, ish) TODO gate by search active
    if (tab == 1) { /*WeaponTabLH: WepNameTextLH, WepIconLH, ClipBox, EnergyHeatTicks, ReloadButtons, EnergySlider*/
        int widx=World.invP1.weaponInventoryIndices[World.invP1.weaponCurrent];
        if (widx >= 0) { RenderTextL(isRH ? 1342 : 24,520,T_RED,FONT_NORMAL,0.8f,"%s",Sys_Text.stringTable[ItemStringIdx((i32)widx)]);/*Weapon Name*/ if (wep16 >=0 && wep16 < 16){RenderUIImage(isRH ? 1207 : 24,548,270,100,wepIconTexIndices[wep16]);/*WepIconLH*/} }
    } else if (tab == 2 && mfdItemReader[isRH]) {
        i16 x=isRH?1080:22; static const u16 labels[4]={42,39,43,885};
        RenderTextL(x+6,540,T_YELLOW,FONT_NORMAL,0.6,"%s",Sys_Text.stringTable[349]);
        for (u8 section=0;section<4;++section) { if (section==MM_NOTES && !World.diffMis) continue;
            bool sectionSelected=MFD_MediaTab==section,unread=World.Sys_UI.highlightStatus[section];
            RenderUIImage(x+65*section,718,65,40,sectionSelected||unread?1087:1086);
            RenderTextL(x+65*section,718,sectionSelected?T_GREEN_MENU:T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"%s",Sys_Text.stringTable[labels[section]]);
        }
    }
    else if (tab == 3) { /*AutomapTab: AutomapMask, Overlays, PlayerIcon, ZoomIn/Out/Full/Side Buttons*/ }
    else if (tab == 4) { /*TargetTab*/ }
    else if (tab == 5) { /*DataTab: Security, DataHeaders, ElevatorUIControl, KeycodeUIControl, SearchContents, AudioLogInfo, PuzzleGrid, PuzzleWire, SystemAnalyzer Display*/ }
}

static const u16 vmailStartFrames[6]={1579,1645,1713,1784,1864,1931}; static const u16 vmailEndFrames[6]={1644,1712,1783,1863,1930,1988}; double avgCPUt[AVG_CPU_TAPS]={0}; int avgCPUt_idx = 0;
i32 tWrnTextIdx[10],tWrnTextIdx2[10],tWrnTextIdx3[10],tWrnColorIdx[10]; double tWrnFinished[10];
void AppendTextWarning(i32 sidx, i32 sidx2, i32 sidx3, i32 col, i32 id) { tWrnTextIdx[id]=sidx; tWrnTextIdx2[id]=sidx2; tWrnTextIdx3[id]=sidx3; tWrnFinished[id]=tWrnFinished[id] < World.pauseRelativeTime ? World.pauseRelativeTime + 0.1f : tWrnFinished[id] + 0.1f; tWrnColorIdx[id] = col; }
extern double game_actual_start_time; extern u16 editModeTestEntityDefinition;
// Edit-mode info panel text editing (console-style entry)
enum { EF_POSX,EF_POSY,EF_POSZ,EF_ROTX,EF_ROTY,EF_ROTZ,EF_ROTW,EF_SCLX,EF_SCLY,EF_SCLZ,EF_TEX,EF_MODEL,EF_GLOW,EF_SPEC,EF_NORM,EF_LAST };
bool editFieldEditing=false; static u8 editFieldSlot=EF_LAST; static char editFieldBuffer[40]={0};
#define EF_LABELX 982
#define EF_VALUEX 1088
static const i16 efRowY[EF_LAST]={160,188,216,244,272,300,328,356,384,412,440,468,496,524,552};
static const char* efRowLabel[EF_LAST]={"position x","position y","position z","rotation x","rotation y","rotation z","rotation w","scale x","scale y","scale z","texIndex","modelIndex","glowIndex","specIndex","normIndex"};
extern Quaternion quat_normalize(Quaternion);
static void EditFieldValueText(u8 slot,u16 sel,char* out,size_t n){switch(slot){
    case EF_POSX:sFormat(out,n,"%.2f",World.position[sel].x);break; case EF_POSY:sFormat(out,n,"%.2f",World.position[sel].y);break; case EF_POSZ:sFormat(out,n,"%.2f",World.position[sel].z);break;
    case EF_ROTX:sFormat(out,n,"%.3f",World.rotation[sel].x);break; case EF_ROTY:sFormat(out,n,"%.3f",World.rotation[sel].y);break; case EF_ROTZ:sFormat(out,n,"%.3f",World.rotation[sel].z);break; case EF_ROTW:sFormat(out,n,"%.3f",World.rotation[sel].w);break;
    case EF_SCLX:sFormat(out,n,"%.2f",World.scale[sel].x);break; case EF_SCLY:sFormat(out,n,"%.2f",World.scale[sel].y);break; case EF_SCLZ:sFormat(out,n,"%.2f",World.scale[sel].z);break;
    case EF_TEX:sFormat(out,n,"%u",World.instances[sel].texIndex);break; case EF_MODEL:sFormat(out,n,"%u",World.instances[sel].modelIndex);break; case EF_GLOW:sFormat(out,n,"%u",World.instances[sel].glowIndex);break; case EF_SPEC:sFormat(out,n,"%u",World.instances[sel].specIndex);break; case EF_NORM:sFormat(out,n,"%u",World.instances[sel].normIndex);break;
    default:sFormat(out,n,"");break;}}
static bool EditSelIsActive(void){return Cheats.editMode&&editModeSelection<U16_MAX&&editModeSelection>=INSTS_1ST_IDX&&editModeSelection<World.instCount&&(World.instances[editModeSelection].entflags&EF_ACTIVE);}
bool EditPanelPointerHover(void){if(!EditSelIsActive()||!World.inventoryMode)return false;u16 sel=editModeSelection;char v[40];for(int i=0;i<EF_LAST;++i){EditFieldValueText((u8)i,sel,v,40);float w=MeasureLineAdvance(v,FONT_NORMAL);if(World.cursorPos_x>=EF_VALUEX&&World.cursorPos_x<=EF_VALUEX+w&&World.cursorPos_y>=efRowY[i]&&World.cursorPos_y<=efRowY[i]+26)return true;}return false;}
static void EditStop(void){editFieldEditing=false;editFieldSlot=EF_LAST;}
static void EditFieldWrite(u16 s,Entity* e,float f,i32 iv){switch(editFieldSlot){
    case EF_POSX:World.position[s].x=f;break; case EF_POSY:World.position[s].y=f;break; case EF_POSZ:World.position[s].z=f;break;
    case EF_ROTX:World.rotation[s].x=f;World.rotation[s]=quat_normalize(World.rotation[s]);break; case EF_ROTY:World.rotation[s].y=f;World.rotation[s]=quat_normalize(World.rotation[s]);break; case EF_ROTZ:World.rotation[s].z=f;World.rotation[s]=quat_normalize(World.rotation[s]);break; case EF_ROTW:World.rotation[s].w=f;World.rotation[s]=quat_normalize(World.rotation[s]);break;
    case EF_SCLX:World.scale[s].x=f;break; case EF_SCLY:World.scale[s].y=f;break; case EF_SCLZ:World.scale[s].z=f;break;
    case EF_TEX:e->texIndex=(u16)vclamp(iv,0,MAX_TXRS-1);break; case EF_MODEL:e->modelIndex=(u16)vclamp(iv,0,MAX_MDLS-1);break; case EF_GLOW:e->glowIndex=(u16)vclamp(iv,0,MAX_TXRS-1);break; case EF_SPEC:e->specIndex=(u16)vclamp(iv,0,MAX_TXRS-1);break; case EF_NORM:e->normIndex=(u16)vclamp(iv,0,MAX_TXRS-1);break;}
}

static void EditFieldCommitLive(void){if(!EditSelIsActive())return;u16 s=editModeSelection;Entity* e=&World.instances[s];float f=0;{const char* p=editFieldBuffer;f=fast_atof(&p);}EditFieldWrite(s,e,f,s2i32(editFieldBuffer));}
static void EditFieldStep(float delta){if(!editFieldEditing||!EditSelIsActive())return;float cur;{const char* p=editFieldBuffer;cur=fast_atof(&p);}
    if(editFieldSlot<EF_TEX){int steps=delta>0?(int)(delta+0.5f):-(int)((-delta)+0.5f);cur+=steps*0.01f;int prec=(editFieldSlot>=EF_ROTX&&editFieldSlot<=EF_ROTW)?3:2;char fmt[8];sFormat(fmt,8,"%%.%df",prec);sFormat(editFieldBuffer,40,fmt,cur);}
    else{int steps=delta>0?(int)(delta+0.5f):-(int)((-delta)+0.5f);i32 iv=(i32)cur+steps;iv=vclamp(iv,0,(editFieldSlot==EF_MODEL?MAX_MDLS-1:MAX_TXRS-1));sFormat(editFieldBuffer,40,"%d",iv);}
    EditFieldCommitLive();
}

void EditFieldKey(i32 keycode){if(!editFieldEditing)return;
    if(keycode==KEY_ESCAPE){EditStop();return;} if(keycode==KEY_ENTER||keycode==KEY_KP_ENTER){EditStop();return;}
    size_t len=slen(editFieldBuffer); bool changed=false;
    if(keycode>=KEY_0&&keycode<=KEY_9){if(len<39){editFieldBuffer[len]=(char)('0'+(keycode-KEY_0));editFieldBuffer[len+1]='\0';changed=true;}}
    else if(keycode>=KEY_KP_0&&keycode<=KEY_KP_9){if(len<39){editFieldBuffer[len]=(char)('0'+(keycode-KEY_KP_0));editFieldBuffer[len+1]='\0';changed=true;}}
    else if((keycode==KEY_MINUS||keycode==KEY_KP_SUBTRACT)&&len==0){if(len<39){editFieldBuffer[len]='-';editFieldBuffer[len+1]='\0';changed=true;}}
    else if(keycode==KEY_PERIOD||keycode==KEY_KP_DECIMAL){bool hasdot=false;for(size_t k=0;k<len&&!hasdot;++k)if(editFieldBuffer[k]=='.')hasdot=true;if(!hasdot&&len<39){editFieldBuffer[len]='.';editFieldBuffer[len+1]='\0';changed=true;}}
    else if(keycode==KEY_BACKSPACE){if(len>0){editFieldBuffer[len-1]='\0';changed=true;}}
    if(changed)EditFieldCommitLive();
}

bool UI_PointerBlocksGameplay(void) {
    if (!World.inventoryMode) return false;
    if (World.menuActive || World.paused || World.creditsActive || Cheats.consoleActive) return true;
    if (EditPanelPointerHover()) return true;
    if (Cheats.noHUD || World.Sys_UI.vmailActive) return false;
    if (CursorIsOverBounds(667,699,0,32)) return true;
    u32 hw=World.invP1.hasHardware;
    if (((hw&HW_BIO)&&CursorIsOverBounds(0,40,180,220)) || ((hw&HW_SNS)&&CursorIsOverBounds(0,40,240,280)) || ((hw&HW_LAN)&&CursorIsOverBounds(0,40,300,340)) || ((hw&HW_SHD)&&CursorIsOverBounds(0,40,360,400))) return true;
    if (((hw&HW_INF)&&CursorIsOverBounds(1326,1366,180,220)) || ((hw&HW_ERD)&&CursorIsOverBounds(1326,1366,240,280)) || ((hw&HW_BST)&&CursorIsOverBounds(1326,1366,300,340)) || ((hw&HW_JET)&&CursorIsOverBounds(1326,1366,360,400))) return true;
    if (CursorIsOverBounds(400,464,752,784) || CursorIsOverBounds(480,544,752,784) || CursorIsOverBounds(560,624,752,784) || CursorIsOverBounds(902,966,752,784)) return true;
    for (int side=0;side<2;++side) for (int tab=0;tab<4;++tab) if (CursorIsOverBounds(side?1350:-16,side?1382:16,520+56*tab,560+56*tab)) return true;
    if (World.invP1.holdingObject && CursorIsOverBounds(345,1021,460,768)) return true;
    return (MFD_CenterTab && CursorIsOverBounds(345,1021,552,768)) || (MFD_LefTab && CursorIsOverBounds(24,344,520,768)) || (MFD_RightTab && CursorIsOverBounds(1022,1342,520,768));
}

void UI_ProcessNavigation(void) {
    if (World.menuActive || World.paused || World.creditsActive || Cheats.consoleActive || World.Sys_UI.vmailActive) return;
    static const u16 keys[7]={KEY_F1,KEY_F2,KEY_F3,KEY_F4,KEY_F5,KEY_F7,KEY_F8}; static const u8 tabs[7]={1,2,3,5,1,2,3};
    for (u8 i=0;i<7;++i) if (Sys_Input.keyStates[keys[i]].pressed) { Sys_Input.keyStates[keys[i]].pressed=false; MFD_SelectTab(i<4?1:2,tabs[i],true); }
    for (u8 up=0;up<2;++up) { u16 key=up?KEY_PAGE_UP:KEY_PAGE_DOWN; if (!Sys_Input.keyStates[key].pressed) continue;
        Sys_Input.keyStates[key].pressed=false; u8 tab=MFD_CenterTab?MFD_CenterTab:mfdSelected[0];
        MFD_SelectTab(0,tab==5?1:1+(tab-1+(up?3:1))%4,false); MFD_ReaderView=MFD_READER_CONTENTS;
    }
    if (!World.inventoryMode || Cheats.noHUD) return;
    if (HwBtnClick(667,699,0,32)) { ForceShootMode(); return; }
    if ((World.invP1.hasHardware&HW_ERD) && HwBtnClick(1326,1366,240,280)) {
        MFD_CenterTab=5; MFD_LefTab=2; mfdItemReader[0]=true; MFD_ReaderView=MFD_READER_CONTENTS;
        MFD_MediaTab=World.Sys_UI.lastMultiMediaTabOpened; if (MFD_MediaTab>MM_NOTES || (MFD_MediaTab==MM_NOTES && !World.diffMis)) MFD_MediaTab=MM_LOG_TABLE;
        play_wav(sounds[97],SfxVol(),(V3){0,0,0},false); return;
    }
    static const i16 centerX[4]={400,480,560,902};
    for (u8 i=0;i<4;++i) if (HwBtnClick(centerX[i],centerX[i]+64,752,784)) { MFD_SelectTab(0,i+1,true); return; }
    for (u8 side=0;side<2;++side) {
        for (u8 i=0;i<4;++i) if (HwBtnClick(side?1350:-16,side?1382:16,520+56*i,560+56*i)) { MFD_SelectTab(side+1,tabs[i],true); return; }
        if ((side?MFD_RightTab:MFD_LefTab)!=2 || !mfdItemReader[side]) continue;
        for (u8 section=0;section<4;++section) { if (section==MM_NOTES && !World.diffMis) continue; i16 x=(side?1080:22)+65*section;
            if (!HwBtnClick(x,x+64,718,758)) continue;
            MFD_CenterTab=5; MFD_MediaTab=World.Sys_UI.lastMultiMediaTabOpened=section; MFD_ReaderView=MFD_READER_CONTENTS;
            if (section>=MM_DATA_TABLE) { World.Sys_UI.highlightStatus[section]=false; World.Sys_UI.highlightTickCount[section]=0; }
            play_wav(sounds[97],SfxVol(),(V3){0,0,0},false); return;
        }
    }
}

void RenderSearchFX(void) {
    for (int side=0;side<2;++side) {
        if (!World.Sys_UI.searchFXActive[side]) continue;
        double elapsed = World.pauseRelativeTime - World.Sys_UI.searchFXStartTime[side];
        if (elapsed >= 1.0) { World.Sys_UI.searchFXActive[side] = false; continue; }
        float t = (float)elapsed / 1.0f;
        if (t > 1.0f) t = 1.0f;
        float p = t < 0.4f ? t / 0.4f : 1.0f;  // scale up first 0.4s, hold
        float ep = 1.0f - (1.0f - p) * (1.0f - p) * (1.0f - p);  // ease-out cubic
        float scale = 40.0f + ep * (263.0f - 40.0f);
        float w = scale, h = scale * 240.0f / 263.0f;
        float sx = World.Sys_UI.searchFXCursorX[side];
        float sy = World.Sys_UI.searchFXCursorY[side];
        float ex = (side ? 1210.5f : 151.5f);
        float ey = 648.0f;
        float cx = sx + (ex - sx) * ep;
        float cy = sy + (ey - sy) * ep;
        RenderUIImage((i16)(cx - w * 0.5f),(i16)(cy - h * 0.5f),(i16)(w + 0.5f),(i16)(h + 0.5f),1074);
    }
}

static double RenderUI() {
    drawCallsNormal = drawCalls;
    World.uiIsBlocking = false;
    if(!EditSelIsActive())editFieldEditing=false;
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
            for (int i=0;i<10;++i){ if (tWrnFinished[i] > World.pauseRelativeTime){
                char flt[6]; if(tWrnTextIdx[i] == 185){sFormat(flt,6,"%.1f",(double)World.instances[PLAYER1].radiation);} RenderTextL(340,72+(i*18),tWrnColorIdx[i],FONT_NORMAL,0.8f,"%s%s%s",Sys_Text.stringTable[tWrnTextIdx[i]],tWrnTextIdx[i] == 185 ? flt : tWrnTextIdx2[i] >= 0 ? Sys_Text.stringTable[tWrnTextIdx2[i]] : "",tWrnTextIdx3[i] >= 0 ? Sys_Text.stringTable[tWrnTextIdx3[i]] : "");
            } /*Text Warnings System (e.g. radiation hazard + biohazard), stacks with timeout*/}
        }
//         if (World.Sys_UI.showTeleportFX) { /*TeleportFX*/ } if (World.Sys_UI.showRadiationFX) { /*RadiationFX*/ } if (World.Sys_UI.showHealingFX) { /*HealingFX*/ } if (World.Sys_UI.showShieldFX) { /*ShieldFX*/ } 
//         if (World.Sys_UI.showShieldActivation) { /*waveup*/ /*wavedn*/ } if (World.Sys_UI.showShieldDeactivation) { /*waveup*/ /*wavedn*/ } if (World.Sys_UI.showDeathRessurectionFX) { /*spawndelaycontainers...*/ }
//         if (World.Sys_UI.showAutomapFull) { /*AutomapFullRawImage*/ /*PlayerIconFull*/ /*CloseFullmapButton*/ } if (World.Sys_UI.showMissionTimer) { /*MissionTimerT*/ /*MissionTimer*/ }
//         if (World.Sys_UI.showCyberTimer) { /*CyberTimerT*/ /*CyberTimer*/ }
        if(!Cheats.noHUD){SideMFD(false/*Left*/); CenterMFD(); SideMFD(true/*Right*/);} // MFD

// UI setup pass: every Canvas element in place (Automap + Main Menu omitted,FX occluders skipped)
// C# MFDManager: MFDManager.cs
// C# MFDManager: QuestLogNotesManager.cs
// C# MFDManager: Automap.cs
RenderTextL(43,2,T_YELLOW,FONT_NORMAL,0.6,"0"); // MissionTimerT dummy
RenderTextL(258,2,T_YELLOW,FONT_NORMAL,0.6,"0"); // MissionTimer dummy
// C# MissionTimer: MissionTimer.cs
// C# TabsLH: LeftMFDTabs.cs
// C# ItemTabLH: ItemTabManager.cs
if(MFD_LefTab==2 && !mfdItemReader[0]){ // ItemTabLH
RenderUIImage(33,528,237,237,1025); // ItemIcon UNMAPPED:[Textures/UI/itemicons/paperico.png]
// C# ItemIcon: ItemIconManager.cs
// C# ItemIcon: UIPointerMask.cs
RenderTextL(28,540,T_YELLOW,FONT_NORMAL,0.6,"TEST"); // ItemText
// C# ItemText: UIPointerMask.cs
RenderUIImage(72,628,160,40,1087); // VaporizeButton
// BTN VaporizeButton: VaporizeButton.OnVaporizeClick()
// C# VaporizeButton: VaporizeButton.cs
// C# VaporizeButton: UIButtonMask.cs
RenderTextL(72,628,T_GREEN_MENU,FONT_NORMAL,0.6,"%s",883<1100?Sys_Text.stringTable[883]:"VAPORIZE"); // Text
RenderUIImage(72,691,160,40,1087); // ApplyButton
// BTN ApplyButton: MFDManager.ApplyButtonClicked()
// C# ApplyButton: UIButtonMask.cs
RenderTextL(72,691,T_GREEN_MENU,FONT_NORMAL,0.6,"%s",736<1100?Sys_Text.stringTable[736]:"APPLY"); // Text
RenderUIImage(72,691,160,40,1087); // UseButton
// BTN UseButton: UseButton.OnActivateClick()
// C# UseButton: ActivateButton.cs
RenderTextL(72,691,T_GREEN_MENU,FONT_NORMAL,0.6,"USE"); // Text
RenderTextL(60,614,T_YELLOW,FONT_NORMAL,0.6,"STD"); // AccessCardsList
// C# AccessCardsList: UIPointerMask.cs
// C# GrenadeTimerSliderLH: UIPointerMask.cs
// C# GrenadeTimerSliderLH: GrenadeTimerSlider.cs
RenderUIImage(37,714,230,29,952); // Background
// C# Background: UIPointerMask.cs
RenderUIImage(37,714,72,29,1079); // Fill
// C# Fill: UIPointerMask.cs
RenderUIImage(104,711,24,36,953); // Handle
// C# Handle: UIPointerMask.cs
RenderTextL(60,747,T_YELLOW,FONT_NORMAL,0.6,"TEST"); // TimeNumberText
RenderUIImage(37,712,230,32,952); // Background
RenderUIImage(37,712,16,32,0); // Fill QUAD:builtin-white
RenderUIImage(37,696,32,64,953); // Handle
}
if(MFD_LefTab==5){ // DataTabLH
if(MFD_DataL==8){ // Blocked
RenderUIImage(31,535,227,209,1025); // BlockedBySecurityLH UNMAPPED:[Resources/BlockedBySecurity/blocked_00.
// C# BlockedBySecurityLH: ImageSequenceTextureArrayUI.cs
// C# BlockedBySecurityLH: PooledItemDestroy.cs
RenderTextL(45,542,T_YELLOW,FONT_NORMAL,0.6,"%s",890<1100?Sys_Text.stringTable[890]:"Blocked by SHODAN level Security."); // BlockedBySecurityText
// C# BlockedBySecurityText: UIPointerMask.cs
}
if(MFD_DataL==5){ // Search
RenderTextL(34,536,T_YELLOW,FONT_NORMAL,0.6,"DEAD CORTEX REAVER"); // DataHeaderTextLH
// C# DataHeaderTextLH: UIPointerMask.cs
}
if(MFD_DataL==0){ // idle
RenderTextL(24,633,T_YELLOW,FONT_NORMAL,0.6,"%s",891<1100?Sys_Text.stringTable[891]:"No Items"); // DataNoItemsTextLH
// C# DataNoItemsTextLH: UIPointerMask.cs
// C# ElevatorUIControlLH: ElevatorKeypad.cs
}
if(MFD_DataL==1){ // Elevator
RenderUIImage(132,531,32,32,929); // CurrentFloorIndicator
RenderUIImage(86,578,45,168,0); // ButtonBankLH QUAD:builtin-knob
RenderUIImage(86,578,45,39,1025); // ElevButton1 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]
// C# ElevButton1: UIButtonMask.cs
RenderUIImage(88,583,40,34,1025); // Keypad.Button (1) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Keypad.Button (1): Keypad.Button
// C# Keypad.Button (1): ElevatorButton.cs
// C# Keypad.Button (1): UIButtonMask.cs
RenderTextL(89,580,T_GREEN,FONT_NORMAL,0.6,"R"); // Text (1)
RenderUIImage(86,620,45,39,1025); // ElevButton2 UNMAPPED:[Textures/UI/hudbuttons/keypad_mid.png]
// C# ElevButton2: UIButtonMask.cs
RenderUIImage(88,623,40,34,1025); // Keypad.Button (2) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Keypad.Button (2): Keypad.Button
// C# Keypad.Button (2): ElevatorButton.cs
// C# Keypad.Button (2): UIButtonMask.cs
RenderTextL(89,620,T_GREEN,FONT_NORMAL,0.6,"1"); // Text (2)
RenderUIImage(86,663,45,39,1025); // ElevButton3 UNMAPPED:[Textures/UI/hudbuttons/keypad_mid.png]
// C# ElevButton3: UIButtonMask.cs
RenderUIImage(88,666,40,34,1025); // Keypad.Button (3) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Keypad.Button (3): Keypad.Button
// C# Keypad.Button (3): ElevatorButton.cs
// C# Keypad.Button (3): UIButtonMask.cs
RenderTextL(89,663,T_GREEN,FONT_NORMAL,0.6,"2"); // Text (3)
RenderUIImage(86,706,45,39,1025); // ElevButton4 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]
// C# ElevButton4: UIButtonMask.cs
RenderUIImage(88,707,40,34,1025); // Keypad.Button (4) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Keypad.Button (4): Keypad.Button
// C# Keypad.Button (4): ElevatorButton.cs
// C# Keypad.Button (4): UIButtonMask.cs
RenderTextL(89,704,T_GREEN,FONT_NORMAL,0.6,"3"); // Text (4)
RenderUIImage(164,578,45,168,0); // ButtonBankRH QUAD:builtin-knob
RenderUIImage(164,578,45,39,1025); // ElevButton5 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]
// C# ElevButton5: UIButtonMask.cs
RenderUIImage(167,582,40,34,1025); // Keypad.Button (5) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Keypad.Button (5): Keypad.Button
// C# Keypad.Button (5): ElevatorButton.cs
// C# Keypad.Button (5): UIButtonMask.cs
RenderTextL(168,580,T_GREEN,FONT_NORMAL,0.6,"6"); // Text (5)
RenderUIImage(164,620,45,39,1025); // ElevButton6 UNMAPPED:[Textures/UI/hudbuttons/keypad_mid.png]
// C# ElevButton6: UIButtonMask.cs
RenderUIImage(167,623,40,34,1025); // Keypad.Button (6) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Keypad.Button (6): Keypad.Button
// C# Keypad.Button (6): ElevatorButton.cs
// C# Keypad.Button (6): UIButtonMask.cs
RenderTextL(168,620,T_GREEN,FONT_NORMAL,0.6,"7"); // Text (6)
RenderUIImage(164,663,45,39,1025); // ElevButton7 UNMAPPED:[Textures/UI/hudbuttons/keypad_mid.png]
// C# ElevButton7: UIButtonMask.cs
RenderUIImage(167,666,40,34,1025); // Keypad.Button (7) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Keypad.Button (7): Keypad.Button
// C# Keypad.Button (7): ElevatorButton.cs
// C# Keypad.Button (7): UIButtonMask.cs
RenderTextL(168,663,T_GREEN,FONT_NORMAL,0.6,"8"); // Text (7)
RenderUIImage(164,706,45,39,1025); // ElevButton8 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]
// C# ElevButton8: UIButtonMask.cs
RenderUIImage(167,707,40,34,1025); // Keypad.Button (8) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Keypad.Button (8): Keypad.Button
// C# Keypad.Button (8): ElevatorButton.cs
// C# Keypad.Button (8): UIButtonMask.cs
RenderTextL(168,704,T_GREEN,FONT_NORMAL,0.6,"9"); // Text (8)
RenderUIImage(246,528,29,29,899); // CloseButton
// BTN CloseButton: MFDManager.CloseElevatorPad()
// C# CloseButton: UIButtonMask.cs
RenderTextL(246,528,T_STOPD_RED,FONT_NORMAL,0.6,"X"); // Text
// C# KeycodeUIControlLH: KeypadKeycodeButtons.cs
}
if(MFD_DataL==2){ // Keycode
RenderUIImage(86,577,42,38,1025); // KeycodeButton1 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]
RenderUIImage(88,580,38,35,1025); // Button (1) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Button (1): ?
// C# Button (1): KeycodeButton.cs
RenderTextL(80,572,T_GREEN,FONT_NORMAL,0.6,"1"); // Text
RenderUIImage(127,577,42,38,1025); // KeycodeButton2 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]
RenderUIImage(129,580,38,35,1025); // Button (2) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Button (2): ?
// C# Button (2): KeycodeButton.cs
RenderTextL(121,572,T_GREEN,FONT_NORMAL,0.6,"2"); // Text
RenderUIImage(169,577,42,38,1025); // KeycodeButton3 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]
RenderUIImage(169,580,38,35,1025); // Button (3) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Button (3): ?
// C# Button (3): KeycodeButton.cs
RenderTextL(161,572,T_GREEN,FONT_NORMAL,0.6,"3"); // Text
RenderUIImage(86,620,42,38,1025); // KeycodeButton4 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]
RenderUIImage(88,621,38,35,1025); // Button (4) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Button (4): ?
// C# Button (4): KeycodeButton.cs
RenderTextL(80,614,T_GREEN,FONT_NORMAL,0.6,"4"); // Text
RenderUIImage(127,620,42,38,1025); // KeycodeButton5 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]
RenderUIImage(129,621,38,35,1025); // Button (5) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Button (5): ?
// C# Button (5): KeycodeButton.cs
RenderTextL(121,614,T_GREEN,FONT_NORMAL,0.6,"5"); // Text
RenderUIImage(169,620,42,38,1025); // KeycodeButton6 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]
RenderUIImage(169,621,38,35,1025); // Button (6) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Button (6): ?
// C# Button (6): KeycodeButton.cs
RenderTextL(162,614,T_GREEN,FONT_NORMAL,0.6,"6"); // Text
RenderUIImage(86,663,42,38,1025); // KeycodeButton7 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]
RenderUIImage(88,665,38,35,1025); // Button (7) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Button (7): ?
// C# Button (7): KeycodeButton.cs
RenderTextL(80,657,T_GREEN,FONT_NORMAL,0.6,"7"); // Text
RenderUIImage(127,663,42,38,1025); // KeycodeButton8 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]
RenderUIImage(129,665,38,35,1025); // Button (8) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Button (8): ?
// C# Button (8): KeycodeButton.cs
RenderTextL(121,657,T_GREEN,FONT_NORMAL,0.6,"8"); // Text
RenderUIImage(169,663,42,38,1025); // KeycodeButton9 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]
RenderUIImage(169,665,38,35,1025); // Button (9) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Button (9): ?
// C# Button (9): KeycodeButton.cs
RenderTextL(162,657,T_GREEN,FONT_NORMAL,0.6,"9"); // Text
RenderUIImage(86,706,42,38,1025); // KeycodeButtonBackSpace UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]
RenderUIImage(88,707,38,35,1025); // Button (-) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Button (-): ?
// C# Button (-): KeycodeButton.cs
RenderTextL(80,700,T_GREEN,FONT_NORMAL,0.6,"-"); // Text
RenderUIImage(127,706,42,38,1025); // KeycodeButton0 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]
RenderUIImage(129,707,38,35,1025); // Button (0) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Button (0): ?
// C# Button (0): KeycodeButton.cs
RenderTextL(121,700,T_GREEN,FONT_NORMAL,0.6,"0"); // Text
RenderUIImage(169,706,42,38,1025); // KeycodeButtonC UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]
RenderUIImage(169,707,38,35,1025); // Button (C) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Button (C): ?
// C# Button (C): KeycodeButton.cs
RenderTextL(161,700,T_GREEN,FONT_NORMAL,0.6,"C"); // Text
RenderUIImage(173,526,32,32,1025); // KeycodeOnes UNMAPPED:[Textures/UI/elnum_null.png]
// C# KeycodeOnes: KeycodeDigitImage.cs
RenderUIImage(132,526,32,32,1025); // KeycodeTens UNMAPPED:[Textures/UI/elnum_null.png]
// C# KeycodeTens: KeycodeDigitImage.cs
RenderUIImage(255,525,29,29,899); // CloseButton
// BTN CloseButton: MFDManager.CloseKeycodePad()
// C# CloseButton: UIButtonMask.cs
RenderTextL(255,525,T_STOPD_RED,FONT_NORMAL,0.6,"X"); // Text
RenderUIImage(90,526,32,32,1025); // KeycodeHuns UNMAPPED:[Textures/UI/elnum_null.png]
// C# KeycodeHuns: KeycodeDigitImage.cs
}
if(MFD_DataL==5){ // Search
RenderUIImage(20,528,263,240,0); // SearchContentsContainerLH QUAD:builtin-knob
// C# SearchContentsContainerLH: SearchButton.cs
RenderUIImage(259,534,29,29,899); // SearchCloseButtonLH
// BTN SearchCloseButtonLH: MFDManager.CloseSearch()
// C# SearchCloseButtonLH: UIButtonMask.cs
RenderTextL(259,534,T_STOPD_RED,FONT_NORMAL,0.6,"X"); // Text
RenderUIImage(84,584,64,64,965); // SearchContentLH1 QUAD:none
// BTN SearchContentLH1: SearchContentsContainerLH.SearchButtonClick()
// C# SearchContentLH1: UIButtonMask.cs
// C# SearchContentLH1: SearchContainerButton.cs
RenderUIImage(174,584,64,64,965); // SearchContentLH2 QUAD:none
// BTN SearchContentLH2: SearchContentsContainerLH.SearchButtonClick(1)
// C# SearchContentLH2: UIButtonMask.cs
// C# SearchContentLH2: SearchContainerButton.cs
RenderUIImage(84,674,64,64,965); // SearchContentLH3 QUAD:none
// BTN SearchContentLH3: SearchContentsContainerLH.SearchButtonClick(2)
// C# SearchContentLH3: UIButtonMask.cs
// C# SearchContentLH3: SearchContainerButton.cs
RenderUIImage(174,674,64,64,965); // SearchContentLH4 QUAD:none
// BTN SearchContentLH4: SearchContentsContainerLH.SearchButtonClick(3)
// C# SearchContentLH4: UIButtonMask.cs
// C# SearchContentLH4: SearchContainerButton.cs
if (World.invP1.currentSearchItem >= 0) {
    int s = World.invP1.currentSearchItem;
    for (int i = 0; i < 4; i++) {
        int tex = 965; // frobicon dummy
        int cx[4] = {84, 174, 84, 174}; int cy[4] = {584, 584, 674, 674};
        RenderUIImage(cx[i], cy[i], 64, 64, tex); // Search content slot (dummy for positioning)
    }
}
// C# AudioLogInfoLH: LogDataTabContainerManager.cs
}
if(MFD_DataL==6){ // AudioLog
RenderUIImage(20,528,263,240,1272); // LogImage
RenderTextL(29,540,T_YELLOW,FONT_NORMAL,0.6,"HACKER IS AWESOME"); // LogName
// C# LogName: UIPointerMask.cs
RenderTextL(29,557,T_YELLOW,FONT_NORMAL,0.6,"Sender: SHODAN"); // SenderText
// C# SenderText: UIPointerMask.cs
RenderTextL(29,701,T_YELLOW,FONT_NORMAL,0.6,"Subject:\n\nif only i had a sparq beam then all the world would be right"); // SubjectText
// C# SubjectText: UIPointerMask.cs
// C# PuzzleGridLH: PuzzleGrid.cs
}
if(MFD_DataL==3){ // GridPuzzle
RenderUIImage(42,555,221,163,1025); // OuterColorBorder UNMAPPED:[Textures/UI/puzzle/gridcontainer_gray.p
RenderUIImage(46,558,214,157,1025); // ContainerEdge UNMAPPED:[Textures/UI/puzzle/gridcontainer.png]
RenderUIImage(25,621,29,29,1025); // NodeSource UNMAPPED:[Textures/UI/puzzle/node_source.png]
RenderUIImage(250,621,29,29,1025); // Node UNMAPPED:[Textures/UI/puzzle/node_off.png]
RenderUIImage(51,565,29,29,1025); // Button UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button: PuzzleGridLH.OnGridCellClick()
// C# Button: UIButtonMask.cs
// C# Button: PuzzleUIButton.cs
RenderUIImage(51,565,29,29,1025); // GeniusHighlight UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderTextL(51,565,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(80,565,29,29,1025); // Button (1) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (1): PuzzleGridLH.OnGridCellClick(1)
// C# Button (1): UIButtonMask.cs
// C# Button (1): PuzzleUIButton.cs
RenderTextL(80,565,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(80,565,29,29,1025); // GeniusHighlight (1) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(109,565,29,29,1025); // Button (2) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (2): PuzzleGridLH.OnGridCellClick(2)
// C# Button (2): UIButtonMask.cs
// C# Button (2): PuzzleUIButton.cs
RenderTextL(109,565,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(109,565,29,29,1025); // GeniusHighlight (2) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(138,565,29,29,1025); // Button (3) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (3): PuzzleGridLH.OnGridCellClick(3)
// C# Button (3): UIButtonMask.cs
// C# Button (3): PuzzleUIButton.cs
RenderTextL(138,565,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(138,565,29,29,1025); // GeniusHighlight (3) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(166,565,29,29,1025); // Button (4) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (4): PuzzleGridLH.OnGridCellClick(4)
// C# Button (4): UIButtonMask.cs
// C# Button (4): PuzzleUIButton.cs
RenderTextL(166,565,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(166,565,29,29,1025); // GeniusHighlight (4) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(195,565,29,29,1025); // Button (5) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (5): PuzzleGridLH.OnGridCellClick(5)
// C# Button (5): UIButtonMask.cs
// C# Button (5): PuzzleUIButton.cs
RenderTextL(195,565,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(195,565,29,29,1025); // GeniusHighlight (5) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(224,565,29,29,1025); // Button (6) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (6): PuzzleGridLH.OnGridCellClick(6)
// C# Button (6): UIButtonMask.cs
// C# Button (6): PuzzleUIButton.cs
RenderTextL(224,565,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(224,565,29,29,1025); // GeniusHighlight (6) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(51,594,29,29,1025); // Button (7) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (7): PuzzleGridLH.OnGridCellClick(7)
// C# Button (7): UIButtonMask.cs
// C# Button (7): PuzzleUIButton.cs
RenderTextL(51,594,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(51,594,29,29,1025); // GeniusHighlight (7) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(80,594,29,29,1025); // Button (8) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (8): PuzzleGridLH.OnGridCellClick(8)
// C# Button (8): UIButtonMask.cs
// C# Button (8): PuzzleUIButton.cs
RenderTextL(80,594,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(80,594,29,29,1025); // GeniusHighlight (8) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(109,594,29,29,1025); // Button (9) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (9): PuzzleGridLH.OnGridCellClick(9)
// C# Button (9): UIButtonMask.cs
// C# Button (9): PuzzleUIButton.cs
RenderTextL(109,594,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(109,594,29,29,1025); // GeniusHighlight (9) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(138,594,29,29,1025); // Button (10) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (10): PuzzleGridLH.OnGridCellClick(10)
// C# Button (10): UIButtonMask.cs
// C# Button (10): PuzzleUIButton.cs
RenderTextL(138,594,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(138,594,29,29,1025); // GeniusHighlight (10) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(166,594,29,29,1025); // Button (11) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (11): PuzzleGridLH.OnGridCellClick(11)
// C# Button (11): UIButtonMask.cs
// C# Button (11): PuzzleUIButton.cs
RenderTextL(166,594,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(166,594,29,29,1025); // GeniusHighlight (11) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(195,594,29,29,1025); // Button (12) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (12): PuzzleGridLH.OnGridCellClick(12)
// C# Button (12): UIButtonMask.cs
// C# Button (12): PuzzleUIButton.cs
RenderTextL(195,594,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(195,594,29,29,1025); // GeniusHighlight (12) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(224,594,29,29,1025); // Button (13) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (13): PuzzleGridLH.OnGridCellClick(13)
// C# Button (13): UIButtonMask.cs
// C# Button (13): PuzzleUIButton.cs
RenderTextL(224,594,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(224,594,29,29,1025); // GeniusHighlight (13) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(51,622,29,29,1025); // Button (14) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (14): PuzzleGridLH.OnGridCellClick(14)
// C# Button (14): UIButtonMask.cs
// C# Button (14): PuzzleUIButton.cs
RenderTextL(51,622,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(51,622,29,29,1025); // GeniusHighlight (14) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(80,622,29,29,1025); // Button (15) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (15): PuzzleGridLH.OnGridCellClick(15)
// C# Button (15): UIButtonMask.cs
// C# Button (15): PuzzleUIButton.cs
RenderTextL(80,622,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(80,622,29,29,1025); // GeniusHighlight (15) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(109,622,29,29,1025); // Button (16) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (16): PuzzleGridLH.OnGridCellClick(16)
// C# Button (16): UIButtonMask.cs
// C# Button (16): PuzzleUIButton.cs
RenderTextL(109,622,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(109,622,29,29,1025); // GeniusHighlight (16) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(138,622,29,29,1025); // Button (17) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (17): PuzzleGridLH.OnGridCellClick(17)
// C# Button (17): UIButtonMask.cs
// C# Button (17): PuzzleUIButton.cs
RenderTextL(138,622,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(138,622,29,29,1025); // GeniusHighlight (17) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(166,622,29,29,1025); // Button (18) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (18): PuzzleGridLH.OnGridCellClick(18)
// C# Button (18): UIButtonMask.cs
// C# Button (18): PuzzleUIButton.cs
RenderTextL(166,622,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(166,622,29,29,1025); // GeniusHighlight (18) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(195,622,29,29,1025); // Button (19) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (19): PuzzleGridLH.OnGridCellClick(19)
// C# Button (19): UIButtonMask.cs
// C# Button (19): PuzzleUIButton.cs
RenderTextL(195,622,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(195,622,29,29,1025); // GeniusHighlight (19) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(224,622,29,29,1025); // Button (20) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (20): PuzzleGridLH.OnGridCellClick(20)
// C# Button (20): UIButtonMask.cs
// C# Button (20): PuzzleUIButton.cs
RenderTextL(224,622,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(224,622,29,29,1025); // GeniusHighlight (20) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(51,651,29,29,1025); // Button (21) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (21): PuzzleGridLH.OnGridCellClick(21)
// C# Button (21): UIButtonMask.cs
// C# Button (21): PuzzleUIButton.cs
RenderTextL(51,651,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(51,651,29,29,1025); // GeniusHighlight (21) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(80,651,29,29,1025); // Button (22) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (22): PuzzleGridLH.OnGridCellClick(22)
// C# Button (22): UIButtonMask.cs
// C# Button (22): PuzzleUIButton.cs
RenderTextL(80,651,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(80,651,29,29,1025); // GeniusHighlight (22) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(109,651,29,29,1025); // Button (23) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (23): PuzzleGridLH.OnGridCellClick(23)
// C# Button (23): UIButtonMask.cs
// C# Button (23): PuzzleUIButton.cs
RenderTextL(109,651,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(109,651,29,29,1025); // GeniusHighlight (23) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(138,651,29,29,1025); // Button (24) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (24): PuzzleGridLH.OnGridCellClick(24)
// C# Button (24): UIButtonMask.cs
// C# Button (24): PuzzleUIButton.cs
RenderTextL(138,651,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(138,651,29,29,1025); // GeniusHighlight (24) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(166,651,29,29,1025); // Button (25) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (25): PuzzleGridLH.OnGridCellClick(25)
// C# Button (25): UIButtonMask.cs
// C# Button (25): PuzzleUIButton.cs
RenderTextL(166,651,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(166,651,29,29,1025); // GeniusHighlight (25) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(195,651,29,29,1025); // Button (26) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (26): PuzzleGridLH.OnGridCellClick(26)
// C# Button (26): UIButtonMask.cs
// C# Button (26): PuzzleUIButton.cs
RenderTextL(195,651,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(195,651,29,29,1025); // GeniusHighlight (26) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(224,651,29,29,1025); // Button (27) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (27): PuzzleGridLH.OnGridCellClick(27)
// C# Button (27): UIButtonMask.cs
// C# Button (27): PuzzleUIButton.cs
RenderTextL(224,651,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(224,651,29,29,1025); // GeniusHighlight (27) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(51,680,29,29,1025); // Button (28) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (28): PuzzleGridLH.OnGridCellClick(28)
// C# Button (28): UIButtonMask.cs
// C# Button (28): PuzzleUIButton.cs
RenderTextL(51,680,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(51,680,29,29,1025); // GeniusHighlight (28) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(80,680,29,29,1025); // Button (29) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (29): PuzzleGridLH.OnGridCellClick(29)
// C# Button (29): UIButtonMask.cs
// C# Button (29): PuzzleUIButton.cs
RenderTextL(80,680,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(80,680,29,29,1025); // GeniusHighlight (29) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(109,680,29,29,1025); // Button (30) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (30): PuzzleGridLH.OnGridCellClick(30)
// C# Button (30): UIButtonMask.cs
// C# Button (30): PuzzleUIButton.cs
RenderTextL(109,680,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(109,680,29,29,1025); // GeniusHighlight (30) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(138,680,29,29,1025); // Button (31) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (31): PuzzleGridLH.OnGridCellClick(31)
// C# Button (31): UIButtonMask.cs
// C# Button (31): PuzzleUIButton.cs
RenderTextL(138,680,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(138,680,29,29,1025); // GeniusHighlight (31) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(166,680,29,29,1025); // Button (32) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (32): PuzzleGridLH.OnGridCellClick(32)
// C# Button (32): UIButtonMask.cs
// C# Button (32): PuzzleUIButton.cs
RenderTextL(166,680,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(166,680,29,29,1025); // GeniusHighlight (32) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(195,680,29,29,1025); // Button (33) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (33): PuzzleGridLH.OnGridCellClick(33)
// C# Button (33): UIButtonMask.cs
// C# Button (33): PuzzleUIButton.cs
RenderTextL(195,680,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(195,680,29,29,1025); // GeniusHighlight (33) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(224,680,29,29,1025); // Button (34) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (34): PuzzleGridLH.OnGridCellClick(34)
// C# Button (34): UIButtonMask.cs
// C# Button (34): PuzzleUIButton.cs
RenderTextL(224,680,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(224,680,29,29,1025); // GeniusHighlight (34) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(42,720,221,26,1025); // ProgressContainer UNMAPPED:[Textures/UI/puzzle/gridcontainer_gray.p
RenderUIImage(45,726,225,13,0); // Background QUAD:builtin-knob
RenderUIImage(48,726,6,13,1025); // Fill UNMAPPED:[Textures/UI/puzzle/puzzlesliderwire.png
RenderUIImage(45,720,22,26,1078); // Handle
RenderUIImage(259,527,29,29,899); // CloseButton
// BTN CloseButton: MFDManager.ClosePuzzleGrid()
// C# CloseButton: UIButtonMask.cs
RenderTextL(259,527,T_STOPD_RED,FONT_NORMAL,0.6,"X"); // Text
// C# PuzzleWireLH: PuzzleWire.cs
}
if(MFD_DataL==4){ // WirePuzzle
RenderUIImage(82,570,139,192,1025); // ContainerCenter UNMAPPED:[Textures/UI/puzzle/wire_center.png]
RenderUIImage(34,521,235,44,1025); // LevelsBox UNMAPPED:[Textures/UI/puzzle/wire_levelsbox.png]
RenderUIImage(40,526,235,34,0); // Background QUAD:builtin-knob
RenderUIImage(43,526,6,34,1025); // Fill UNMAPPED:[Textures/UI/puzzle/puzzlesliderwire.png
RenderUIImage(40,509,22,69,1078); // Handle
RenderUIImage(204,522,66,42,1025); // TargetLine UNMAPPED:[Textures/UI/puzzle/wire_levelstargetlin
RenderUIImage(57,566,26,29,1025); // NodeBase UNMAPPED:[Textures/UI/puzzle/wire_node.png]
// BTN NodeBase: PuzzleWireLH.ClickLHNode()
// C# NodeBase: UIButtonMask.cs
// C# NodeBase: PuzzleUIButton.cs
RenderUIImage(61,572,16,16,0); // SelectedIndicator QUAD:none
RenderUIImage(58,569,22,22,0); // GeniusHint QUAD:none
RenderUIImage(57,594,26,29,1025); // NodeBase (1) UNMAPPED:[Textures/UI/puzzle/wire_node.png]
// BTN NodeBase (1): PuzzleWireLH.ClickLHNode(1)
// C# NodeBase (1): UIButtonMask.cs
// C# NodeBase (1): PuzzleUIButton.cs
RenderUIImage(61,600,16,16,0); // SelectedIndicator (1) QUAD:none
RenderUIImage(58,597,22,22,0); // GeniusHint (1) QUAD:none
RenderUIImage(57,623,26,29,1025); // NodeBase (2) UNMAPPED:[Textures/UI/puzzle/wire_node.png]
// BTN NodeBase (2): PuzzleWireLH.ClickLHNode(2)
// C# NodeBase (2): UIButtonMask.cs
// C# NodeBase (2): PuzzleUIButton.cs
RenderUIImage(61,629,16,16,0); // SelectedIndicator (2) QUAD:none
RenderUIImage(58,626,22,22,0); // GeniusHint (2) QUAD:none
RenderUIImage(57,651,26,29,1025); // NodeBase (3) UNMAPPED:[Textures/UI/puzzle/wire_node.png]
// BTN NodeBase (3): PuzzleWireLH.ClickLHNode(3)
// C# NodeBase (3): UIButtonMask.cs
// C# NodeBase (3): PuzzleUIButton.cs
RenderUIImage(61,657,16,16,0); // SelectedIndicator (3) QUAD:none
RenderUIImage(58,654,22,22,0); // GeniusHint (3) QUAD:none
RenderUIImage(57,679,26,29,1025); // NodeBase (4) UNMAPPED:[Textures/UI/puzzle/wire_node.png]
// BTN NodeBase (4): PuzzleWireLH.ClickLHNode(4)
// C# NodeBase (4): UIButtonMask.cs
// C# NodeBase (4): PuzzleUIButton.cs
RenderUIImage(61,685,16,16,0); // SelectedIndicator (4) QUAD:none
RenderUIImage(58,682,22,22,0); // GeniusHint (4) QUAD:none
RenderUIImage(57,707,26,29,1025); // NodeBase (5) UNMAPPED:[Textures/UI/puzzle/wire_node.png]
// BTN NodeBase (5): PuzzleWireLH.ClickLHNode(5)
// C# NodeBase (5): UIButtonMask.cs
// C# NodeBase (5): PuzzleUIButton.cs
RenderUIImage(61,713,16,16,0); // SelectedIndicator (5) QUAD:none
RenderUIImage(58,710,22,22,0); // GeniusHint (5) QUAD:none
RenderUIImage(57,737,26,29,1025); // NodeBase (6) UNMAPPED:[Textures/UI/puzzle/wire_node.png]
// BTN NodeBase (6): PuzzleWireLH.ClickLHNode(6)
// C# NodeBase (6): UIButtonMask.cs
// C# NodeBase (6): PuzzleUIButton.cs
RenderUIImage(61,743,16,16,0); // SelectedIndicator (6) QUAD:none
RenderUIImage(58,740,22,22,0); // GeniusHint (6) QUAD:none
RenderUIImage(222,566,26,29,1025); // NodeBase UNMAPPED:[Textures/UI/puzzle/wire_node.png]
// BTN NodeBase: PuzzleWireLH.ClickRHNode()
// C# NodeBase: UIButtonMask.cs
// C# NodeBase: PuzzleUIButton.cs
RenderUIImage(227,572,16,16,0); // SelectedIndicator QUAD:none
RenderUIImage(223,569,22,22,0); // GeniusHint QUAD:none
RenderUIImage(222,594,26,29,1025); // NodeBase (1) UNMAPPED:[Textures/UI/puzzle/wire_node.png]
// BTN NodeBase (1): PuzzleWireLH.ClickRHNode(1)
// C# NodeBase (1): UIButtonMask.cs
// C# NodeBase (1): PuzzleUIButton.cs
RenderUIImage(227,600,16,16,0); // SelectedIndicator (1) QUAD:none
RenderUIImage(223,597,22,22,0); // GeniusHint (1) QUAD:none
RenderUIImage(222,623,26,29,1025); // NodeBase (2) UNMAPPED:[Textures/UI/puzzle/wire_node.png]
// BTN NodeBase (2): PuzzleWireLH.ClickRHNode(2)
// C# NodeBase (2): UIButtonMask.cs
// C# NodeBase (2): PuzzleUIButton.cs
RenderUIImage(227,629,16,16,0); // SelectedIndicator (2) QUAD:none
RenderUIImage(223,626,22,22,0); // GeniusHint (2) QUAD:none
RenderUIImage(222,651,26,29,1025); // NodeBase (3) UNMAPPED:[Textures/UI/puzzle/wire_node.png]
// BTN NodeBase (3): PuzzleWireLH.ClickRHNode(3)
// C# NodeBase (3): UIButtonMask.cs
// C# NodeBase (3): PuzzleUIButton.cs
RenderUIImage(227,657,16,16,0); // SelectedIndicator (3) QUAD:none
RenderUIImage(223,654,22,22,0); // GeniusHint (3) QUAD:none
RenderUIImage(222,679,26,29,1025); // NodeBase (4) UNMAPPED:[Textures/UI/puzzle/wire_node.png]
// BTN NodeBase (4): PuzzleWireLH.ClickRHNode(4)
// C# NodeBase (4): UIButtonMask.cs
// C# NodeBase (4): PuzzleUIButton.cs
RenderUIImage(227,685,16,16,0); // SelectedIndicator (4) QUAD:none
RenderUIImage(223,682,22,22,0); // GeniusHint (4) QUAD:none
RenderUIImage(222,707,26,29,1025); // NodeBase (5) UNMAPPED:[Textures/UI/puzzle/wire_node.png]
// BTN NodeBase (5): PuzzleWireLH.ClickRHNode(5)
// C# NodeBase (5): UIButtonMask.cs
// C# NodeBase (5): PuzzleUIButton.cs
RenderUIImage(227,713,16,16,0); // SelectedIndicator (5) QUAD:none
RenderUIImage(223,710,22,22,0); // GeniusHint (5) QUAD:none
RenderUIImage(222,736,26,29,1025); // NodeBase (6) UNMAPPED:[Textures/UI/puzzle/wire_node.png]
// BTN NodeBase (6): PuzzleWireLH.ClickRHNode(6)
// C# NodeBase (6): UIButtonMask.cs
// C# NodeBase (6): PuzzleUIButton.cs
RenderUIImage(227,743,16,16,0); // SelectedIndicator (6) QUAD:none
RenderUIImage(223,740,22,22,0); // GeniusHint (6) QUAD:none
RenderUIImage(259,736,29,29,899); // CloseButton
// BTN CloseButton: MFDManager.ClosePuzzleWire()
// C# CloseButton: UIButtonMask.cs
RenderTextL(259,736,T_STOPD_RED,FONT_NORMAL,0.6,"X"); // Text
// C# SystemAnalyzerDisplayLH: SystemAnalyzer.cs
}
if(MFD_DataL==7){ // SysAnalyzer
RenderTextL(24,523,T_YELLOW,FONT_NORMAL,0.6,"%s",892<1100?Sys_Text.stringTable[892]:"SYSTEM ANALYZER"); // Header
// C# Header: UIPointerMask.cs
RenderTextL(24,547,T_GREEN,FONT_NORMAL,0.6,"Current level security:"); // DescriptionLevelSecurity
// C# DescriptionLevelSecurity: UIPointerMask.cs
RenderTextL(180,547,T_GREEN,FONT_NORMAL,0.6,"100%%"); // TextLevelSecurity
// C# TextLevelSecurity: UIPointerMask.cs
RenderTextL(24,566,T_GREEN,FONT_NORMAL,0.6,"Mining laser status:"); // DescriptionMiningLaser
// C# DescriptionMiningLaser: UIPointerMask.cs
RenderTextL(180,566,T_GREEN,FONT_NORMAL,0.6,"Charging"); // TextLaserStatus
// C# TextLaserStatus: UIPointerMask.cs
RenderTextL(24,585,T_GREEN,FONT_NORMAL,0.6,"Lifepod status:"); // DescriptionLifepods
// C# DescriptionLifepods: UIPointerMask.cs
RenderTextL(180,585,T_GREEN,FONT_NORMAL,0.6,"Disabled"); // TextLifepodStatus
// C# TextLifepodStatus: UIPointerMask.cs
RenderTextL(24,605,T_GREEN,FONT_NORMAL,0.6,"Station shield status:"); // DescriptionShield
// C# DescriptionShield: UIPointerMask.cs
RenderTextL(180,605,T_GREEN,FONT_NORMAL,0.6,"Off"); // TextShieldStatus
// C# TextShieldStatus: UIPointerMask.cs
RenderTextL(24,624,T_GREEN,FONT_NORMAL,0.6,"Reactor status:"); // DescriptionReactor
// C# DescriptionReactor: UIPointerMask.cs
RenderTextL(180,624,T_GREEN,FONT_NORMAL,0.6,"Normal"); // TextReactorStatus
// C# TextReactorStatus: UIPointerMask.cs
RenderTextL(24,643,T_GREEN,FONT_NORMAL,0.6,"Processor nodes:"); // DescriptionProcessors
// C# DescriptionProcessors: UIPointerMask.cs
RenderTextL(180,643,T_GREEN,FONT_NORMAL,0.6,"99"); // TextProcessors
// C# TextProcessors: UIPointerMask.cs
RenderTextL(24,662,T_GREEN,FONT_NORMAL,0.6,"Main Program:"); // DescriptionMainProgram
// C# DescriptionMainProgram: UIPointerMask.cs
RenderTextL(179,662,T_GREEN,FONT_NORMAL,0.6,"Downloading to earth"); // TextMainProgram
// C# TextMainProgram: UIPointerMask.cs
RenderTextL(24,681,T_GREEN,FONT_NORMAL,0.6,"Alpha Grove status:"); // DescriptionGroveAlphaStatus
// C# DescriptionGroveAlphaStatus: UIPointerMask.cs
RenderTextL(180,681,T_GREEN,FONT_NORMAL,0.6,"normal"); // TextGroveAlpha
// C# TextGroveAlpha: UIPointerMask.cs
RenderTextL(24,701,T_GREEN,FONT_NORMAL,0.6,"Beta Grove status:"); // DescriptionGroveBetaStatus
// C# DescriptionGroveBetaStatus: UIPointerMask.cs
RenderTextL(180,701,T_GREEN,FONT_NORMAL,0.6,"normal"); // TextGroveBeta
// C# TextGroveBeta: UIPointerMask.cs
RenderTextL(24,720,T_GREEN,FONT_NORMAL,0.6,"Gamma Grove status:"); // DescriptionGroveGammaStatus
// C# DescriptionGroveGammaStatus: UIPointerMask.cs
RenderTextL(180,720,T_GREEN,FONT_NORMAL,0.6,"launched"); // TextGroveGamma
// C# TextGroveGamma: UIPointerMask.cs
RenderTextL(24,739,T_GREEN,FONT_NORMAL,0.6,"Delta Grove status:"); // DescriptionGroveDeltaStatus
// C# DescriptionGroveDeltaStatus: UIPointerMask.cs
RenderTextL(180,739,T_GREEN,FONT_NORMAL,0.6,"launched"); // TextGroveDelta
// C# TextGroveDelta: UIPointerMask.cs
RenderUIImage(259,527,29,29,899); // CloseButton
// BTN CloseButton: SystemAnalyzerDisplayLH.Close()
// C# CloseButton: UIButtonMask.cs
RenderTextL(259,527,T_STOPD_RED,FONT_NORMAL,0.6,"X"); // Text
}
if(MFD_DataL==9){ // Minigames
RenderUIImage(21,501,262,262,1025); // MinigamesContainer
// C# MinigamesContainer: UIPointerMask.cs
RenderTextL(28,503,T_RED,FONT_NORMAL,0.6,"TRIOPTIMUM FUNPACK"); // Header
RenderUIImage(32,540,115,24,0); // MiniGameButton0_Ping QUAD:builtin-white
// BTN MiniGameButton0_Ping: MFDManager.MinigameStart_Ping()
// C# MiniGameButton0_Ping: UIButtonMask.cs
RenderTextL(37,541,T_GREEN,FONT_NORMAL,0.6,"Ping"); // Text
RenderUIImage(32,575,115,24,0); // MiniGameButton1_15 QUAD:builtin-white
// BTN MiniGameButton1_15: MFDManager.MinigameStart_15()
// C# MiniGameButton1_15: UIButtonMask.cs
RenderTextL(37,577,T_GREEN,FONT_NORMAL,0.6,"15"); // Text
RenderUIImage(32,610,115,24,0); // MiniGameButton2_Wing0 QUAD:builtin-white
// BTN MiniGameButton2_Wing0: MFDManager.MinigameStart_Wing0()
// C# MiniGameButton2_Wing0: UIButtonMask.cs
RenderTextL(37,612,T_GREEN,FONT_NORMAL,0.6,"Wing 0"); // Text
RenderUIImage(32,646,115,24,0); // MiniGameButton3_Botbounce QUAD:builtin-white
// BTN MiniGameButton3_Botbounce: MFDManager.MinigameStart_Botbounce()
// C# MiniGameButton3_Botbounce: UIButtonMask.cs
RenderTextL(37,647,T_GREEN,FONT_NORMAL,0.6,"Botbounce"); // Text
RenderUIImage(156,540,115,24,0); // MiniGameButton4_EelZapper QUAD:builtin-white
// BTN MiniGameButton4_EelZapper: MFDManager.MinigameStart_EelZapper()
// C# MiniGameButton4_EelZapper: UIButtonMask.cs
RenderTextL(161,541,T_GREEN,FONT_NORMAL,0.6,"Eel Zapper"); // Text
RenderUIImage(156,575,115,24,0); // MiniGameButton5_Road QUAD:builtin-white
// BTN MiniGameButton5_Road: MFDManager.MinigameStart_Road()
// C# MiniGameButton5_Road: UIButtonMask.cs
RenderTextL(161,577,T_GREEN,FONT_NORMAL,0.6,"Road"); // Text
RenderUIImage(156,610,115,24,0); // MiniGameButton6_TriopToe QUAD:builtin-white
// BTN MiniGameButton6_TriopToe: MFDManager.MinigameStart_TriopToe()
// C# MiniGameButton6_TriopToe: UIButtonMask.cs
RenderTextL(161,612,T_GREEN,FONT_NORMAL,0.6,"TriopToe"); // Text
RenderUIImage(156,646,115,24,0); // MiniGameButton7_CorporateConquer QUAD:builtin-white
// BTN MiniGameButton7_CorporateConquer: MFDManager.MinigameStart_CorporateConquer()
// C# MiniGameButton7_CorporateConquer: UIButtonMask.cs
RenderTextL(161,647,T_GREEN,FONT_NORMAL,0.6,"Corp Conq"); // Text
RenderUIImage(32,681,115,24,0); // MiniGameButton8_Chess QUAD:builtin-white
// BTN MiniGameButton8_Chess: MFDManager.MinigameStart_Chess()
// C# MiniGameButton8_Chess: UIButtonMask.cs
RenderTextL(37,682,T_GREEN,FONT_NORMAL,0.6,"Chess"); // Text
RenderTextL(97,726,T_RED,FONT_NORMAL,0.6,"Don't Play on\n\nCompany Time"); // Footer
RenderUIImage(261,504,19,19,0); // MinigameClose QUAD:none
// BTN MinigameClose: MFDManager.TabReset()
// C# MinigameClose: UIButtonMask.cs
RenderUIImage(259,502,22,22,899); // Border
RenderUIImage(21,501,262,262,0); // MinigameView QUAD:none
// C# MinigameView: UIPointerMask.cs
RenderUIImage(21,501,262,262,0); // PingGameOver QUAD:builtin-white
// BTN PingGameOver: Ping.ResetOnGameOver()|Fifteen.Reset()
RenderTextL(30,545,T_WHITE,FONT_NORMAL,0.6,"PUZZLE SOLVED!"); // gameOverText
RenderTextL(91,710,T_WHITE,FONT_NORMAL,0.6,"YOU LOSE"); // winText
RenderUIImage(261,504,19,19,0); // MinigameBack QUAD:none
// BTN MinigameBack: MFDManager.OpenMinigames()
// C# MinigameBack: UIButtonMask.cs
RenderUIImage(259,502,22,22,899); // Border
// C# TabButtonsPanelLH: TabButtons.cs
}
}
if(World.curLev==LEVEL_CYBERSPACE){ // CyberTimer
RenderTextL(28,530,T_WHITE,FONT_NORMAL,0.6,"T -"); // CyberTimerT
RenderTextL(68,530,T_WHITE,FONT_NORMAL,0.6,"99:99"); // CyberTimer
// C# CyberTimer: CyberTimer.cs
}
if(MFD_CenterTab==1){ // Main
RenderUIImage(553,584,91,191,0); // WeaponShotsInventory QUAD:builtin-knob
RenderTextL(553,584,T_RED,FONT_NORMAL,0.6,"%s",871<1100?Sys_Text.stringTable[871]:"SHOTS"); // TextShotsHeader
// C# TextShotsHeader: UIPointerMask.cs
RenderTextL(553,604,T_YELLOW,FONT_NORMAL,0.6,"SHOTS1"); // Text
RenderTextL(553,624,T_GREEN,FONT_NORMAL,0.6,"SHOTS2"); // Text1
RenderTextL(553,644,T_GREEN,FONT_NORMAL,0.6,"SHOTS3"); // Text1
RenderTextL(553,664,T_GREEN,FONT_NORMAL,0.6,"SHOTS4"); // Text3
RenderTextL(553,685,T_GREEN,FONT_NORMAL,0.6,"SHOTS5"); // Text4
RenderTextL(553,705,T_GREEN,FONT_NORMAL,0.6,"SHOTS6"); // Text5
RenderTextL(553,725,T_GREEN,FONT_NORMAL,0.6,"SHOTS7"); // Text6
RenderUIImage(734,584,88,191,0); // GrenadeInventory QUAD:builtin-knob
// C# GrenadeInventory: GrenadeButtonsManager.cs
RenderTextL(734,584,T_RED,FONT_NORMAL,0.6,"%s",872<1100?Sys_Text.stringTable[872]:"GRENADES"); // Text
// C# Text: UIPointerMask.cs
RenderUIImage(734,604,59,20,0); // FragGrenadeButton QUAD:builtin-white
// BTN FragGrenadeButton: ?
// C# FragGrenadeButton: GrenadeButton.cs
// C# FragGrenadeButton: UIButtonMask.cs
RenderTextL(734,604,T_YELLOW,FONT_NORMAL,0.6,"%s",900<1100?Sys_Text.stringTable[900]:"FRAG"); // Text
RenderUIImage(795,605,20,20,1086); // Button (1)
// BTN Button (1): MainCamera.UseGrenade(7)
// C# Button (1): UIPointerMask.cs
RenderUIImage(800,609,11,11,1079); // Image
RenderUIImage(734,624,59,20,0); // EMPGrenadeButton QUAD:builtin-white
// BTN EMPGrenadeButton: ?
// C# EMPGrenadeButton: GrenadeButton.cs
// C# EMPGrenadeButton: UIButtonMask.cs
RenderTextL(734,624,T_GREEN,FONT_NORMAL,0.6,"%s",901<1100?Sys_Text.stringTable[901]:"EMP"); // Text
RenderUIImage(795,625,20,20,1086); // Button (2)
// BTN Button (2): MainCamera.UseGrenade(9)
// C# Button (2): UIPointerMask.cs
RenderUIImage(800,630,11,11,1079); // Image
RenderUIImage(734,644,59,20,0); // GasGrenadeButton QUAD:builtin-white
// BTN GasGrenadeButton: ?
// C# GasGrenadeButton: GrenadeButton.cs
// C# GasGrenadeButton: UIButtonMask.cs
RenderTextL(734,644,T_GREEN,FONT_NORMAL,0.6,"%s",902<1100?Sys_Text.stringTable[902]:"GAS"); // Text
RenderUIImage(795,645,20,20,1086); // Button (3)
// BTN Button (3): MainCamera.UseGrenade(13)
// C# Button (3): UIPointerMask.cs
RenderUIImage(800,650,11,11,1079); // Image
RenderUIImage(734,664,59,20,0); // ConcussionGrenadeButton QUAD:builtin-white
// BTN ConcussionGrenadeButton: ?
// C# ConcussionGrenadeButton: GrenadeButton.cs
// C# ConcussionGrenadeButton: UIButtonMask.cs
RenderTextL(734,664,T_GREEN,FONT_NORMAL,0.6,"%s",903<1100?Sys_Text.stringTable[903]:"CONC"); // Text
RenderUIImage(795,665,20,20,1086); // Button (4)
// BTN Button (4): MainCamera.UseGrenade(8)
// C# Button (4): UIPointerMask.cs
RenderUIImage(800,670,11,11,1079); // Image
RenderUIImage(734,685,59,20,0); // LandMineGrenadeButton QUAD:builtin-white
// BTN LandMineGrenadeButton: ?
// C# LandMineGrenadeButton: GrenadeButton.cs
// C# LandMineGrenadeButton: UIButtonMask.cs
RenderTextL(734,685,T_GREEN,FONT_NORMAL,0.6,"%s",904<1100?Sys_Text.stringTable[904]:"MINE"); // Text
RenderUIImage(795,686,20,20,1086); // Button (5)
// BTN Button (5): MainCamera.UseGrenade(11)
// C# Button (5): UIPointerMask.cs
RenderUIImage(800,690,11,11,1079); // Image
RenderUIImage(734,705,59,20,0); // NitropackGrenadeButton QUAD:builtin-white
// BTN NitropackGrenadeButton: ?
// C# NitropackGrenadeButton: GrenadeButton.cs
// C# NitropackGrenadeButton: UIButtonMask.cs
RenderTextL(734,705,T_GREEN,FONT_NORMAL,0.6,"%s",905<1100?Sys_Text.stringTable[905]:"NTRO"); // Text
RenderUIImage(795,706,20,20,1086); // Button (6)
// BTN Button (6): MainCamera.UseGrenade(12)
// C# Button (6): UIPointerMask.cs
RenderUIImage(800,711,11,11,1079); // Image
RenderUIImage(734,725,59,20,0); // EarthShakerGrenadeButton QUAD:builtin-white
// BTN EarthShakerGrenadeButton: ?
// C# EarthShakerGrenadeButton: GrenadeButton.cs
// C# EarthShakerGrenadeButton: UIButtonMask.cs
RenderTextL(734,725,T_GREEN,FONT_NORMAL,0.6,"%s",906<1100?Sys_Text.stringTable[906]:"SHKR"); // Text
RenderUIImage(795,726,20,20,1086); // Button (7)
// BTN Button (7): MainCamera.UseGrenade(10)
// C# Button (7): UIPointerMask.cs
RenderUIImage(800,731,11,11,1079); // Image
RenderUIImage(706,584,88,191,0); // GrenadeCountsInventory QUAD:builtin-knob
RenderTextL(706,584,T_RED,FONT_NORMAL,0.6,"#"); // Text
RenderUIImage(706,604,86,20,0); // GrenadeCounts1 QUAD:builtin-white
RenderTextL(706,604,T_YELLOW,FONT_NORMAL,0.6,"#1"); // Text
RenderUIImage(706,624,86,20,0); // GrenadeCounts2 QUAD:builtin-white
RenderTextL(706,624,T_GREEN,FONT_NORMAL,0.6,"#2"); // Text
RenderUIImage(706,644,86,20,0); // GrenadeCounts3 QUAD:builtin-white
RenderTextL(706,644,T_GREEN,FONT_NORMAL,0.6,"#3"); // Text
RenderUIImage(706,664,86,20,0); // GrenadeCounts4 QUAD:builtin-white
RenderTextL(706,664,T_GREEN,FONT_NORMAL,0.6,"#4"); // Text
RenderUIImage(706,685,86,20,0); // GrenadeCounts5 QUAD:builtin-white
RenderTextL(706,685,T_GREEN,FONT_NORMAL,0.6,"#5"); // Text
RenderUIImage(706,705,86,20,0); // GrenadeCounts6 QUAD:builtin-white
RenderTextL(706,705,T_GREEN,FONT_NORMAL,0.6,"#6"); // Text
RenderUIImage(706,725,86,20,0); // GrenadeCounts7 QUAD:builtin-white
RenderTextL(706,725,T_GREEN,FONT_NORMAL,0.6,"#7"); // Text
RenderUIImage(831,584,99,191,0); // PatchInventory QUAD:builtin-knob
RenderTextL(831,584,T_RED,FONT_NORMAL,0.6,"%s",873<1100?Sys_Text.stringTable[873]:"PATCHES"); // Text
// C# Text: UIPointerMask.cs
// BTN Button: ?
// C# Button: PatchButton.cs
// C# Button: UIButtonMask.cs
RenderTextL(831,604,T_YELLOW,FONT_NORMAL,0.6,"%s",907<1100?Sys_Text.stringTable[907]:"STAMUP"); // Text
RenderUIImage(910,605,20,20,1086); // Button
// BTN Button: Button.DoubleClick()
// C# Button: UIPointerMask.cs
RenderUIImage(914,609,11,11,1079); // Image
RenderTextL(869,604,T_YELLOW,FONT_NORMAL,0.6,"#1"); // CountsText
// BTN Button (1): ?
// C# Button (1): PatchButton.cs
// C# Button (1): UIButtonMask.cs
RenderTextL(831,624,T_GREEN,FONT_NORMAL,0.6,"%s",908<1100?Sys_Text.stringTable[908]:"SIGHT"); // Text
RenderUIImage(910,625,20,20,1086); // Button (1)
// BTN Button (1): Button
// C# Button (1): UIPointerMask.cs
RenderUIImage(914,630,11,11,1079); // Image
RenderTextL(869,624,T_GREEN,FONT_NORMAL,0.6,"#2"); // CountsText1
// BTN Button (2): ?
// C# Button (2): PatchButton.cs
// C# Button (2): UIButtonMask.cs
RenderTextL(831,644,T_GREEN,FONT_NORMAL,0.6,"%s",909<1100?Sys_Text.stringTable[909]:"B'SERK"); // Text
RenderUIImage(910,645,20,20,1086); // Button (2)
// BTN Button (2): Button
// C# Button (2): UIPointerMask.cs
RenderUIImage(914,650,11,11,1079); // Image
RenderTextL(869,644,T_GREEN,FONT_NORMAL,0.6,"#3"); // CountsText2
// BTN Button (3): ?
// C# Button (3): PatchButton.cs
// C# Button (3): UIButtonMask.cs
RenderTextL(831,664,T_GREEN,FONT_NORMAL,0.6,"%s",910<1100?Sys_Text.stringTable[910]:"MEDI"); // Text
RenderUIImage(910,665,20,20,1086); // Button (3)
// BTN Button (3): Button
// C# Button (3): UIPointerMask.cs
RenderUIImage(914,670,11,11,1079); // Image
RenderTextL(869,664,T_GREEN,FONT_NORMAL,0.6,"#4"); // CountsText3
// BTN Button (4): ?
// C# Button (4): PatchButton.cs
// C# Button (4): UIButtonMask.cs
RenderTextL(831,685,T_GREEN,FONT_NORMAL,0.6,"%s",911<1100?Sys_Text.stringTable[911]:"REFLEX"); // Text
RenderUIImage(910,686,20,20,1086); // Button (4)
// BTN Button (4): Button
// C# Button (4): UIPointerMask.cs
RenderUIImage(914,690,11,11,1079); // Image
RenderTextL(869,684,T_GREEN,FONT_NORMAL,0.6,"#5"); // CountsText4
// BTN Button (5): ?
// C# Button (5): PatchButton.cs
// C# Button (5): UIButtonMask.cs
RenderTextL(831,705,T_GREEN,FONT_NORMAL,0.6,"%s",912<1100?Sys_Text.stringTable[912]:"GENIUS"); // Text
RenderUIImage(910,706,20,20,1086); // Button (5)
// BTN Button (5): Button
// C# Button (5): UIPointerMask.cs
RenderUIImage(914,711,11,11,1079); // Image
RenderTextL(869,705,T_GREEN,FONT_NORMAL,0.6,"#6"); // CountsText5
// BTN Button (6): ?
// C# Button (6): PatchButton.cs
// C# Button (6): UIButtonMask.cs
RenderTextL(831,725,T_GREEN,FONT_NORMAL,0.6,"%s",913<1100?Sys_Text.stringTable[913]:"DETOX"); // Text
RenderUIImage(910,726,20,20,1086); // Button (6)
// BTN Button (6): Button
// C# Button (6): UIPointerMask.cs
RenderUIImage(914,731,11,11,1079); // Image
RenderTextL(869,725,T_GREEN,FONT_NORMAL,0.6,"#7"); // CountsText6
RenderUIImage(870,584,37,191,0); // PatchCountsInventory QUAD:builtin-knob
RenderTextL(870,584,T_RED,FONT_NORMAL,0.6,"#"); // Text
}
if(MFD_CenterTab==2){ // Hardware
RenderTextL(454,557,T_RED,FONT_NORMAL,0.6,"%s",874<1100?Sys_Text.stringTable[874]:"HARDWARE"); // Label
// C# Label: UIPointerMask.cs
RenderUIImage(458,575,445,188,0); // HarwareInventory QUAD:builtin-knob
RenderUIImage(458,575,222,23,0); // Button QUAD:builtin-white
// BTN Button: ?
// C# Button: UIButtonMask.cs
// C# Button: HardwareInvButton.cs
RenderTextL(458,575,T_YELLOW,FONT_NORMAL,0.6,"HARDWARE #1"); // Text
RenderUIImage(458,598,222,23,0); // Button (1) QUAD:builtin-white
// BTN Button (1): ?
// C# Button (1): UIButtonMask.cs
// C# Button (1): HardwareInvButton.cs
RenderTextL(458,598,T_GREEN,FONT_NORMAL,0.6,"HARDWARE #2"); // Text
RenderUIImage(458,621,222,23,0); // Button (2) QUAD:builtin-white
// BTN Button (2): ?
// C# Button (2): UIButtonMask.cs
// C# Button (2): HardwareInvButton.cs
RenderTextL(458,621,T_GREEN,FONT_NORMAL,0.6,"HARDWARE #3"); // Text
RenderUIImage(458,644,222,23,0); // Button (3) QUAD:builtin-white
// BTN Button (3): ?
// C# Button (3): UIButtonMask.cs
// C# Button (3): HardwareInvButton.cs
RenderTextL(458,644,T_GREEN,FONT_NORMAL,0.6,"HARDWARE #4"); // Text
RenderUIImage(458,667,222,23,0); // Button (4) QUAD:builtin-white
// BTN Button (4): ?
// C# Button (4): UIButtonMask.cs
// C# Button (4): HardwareInvButton.cs
RenderTextL(458,667,T_GREEN,FONT_NORMAL,0.6,"HARDWARE #5"); // Text
RenderUIImage(458,691,222,23,0); // Button (5) QUAD:builtin-white
// BTN Button (5): ?
// C# Button (5): UIButtonMask.cs
// C# Button (5): HardwareInvButton.cs
RenderTextL(458,691,T_GREEN,FONT_NORMAL,0.6,"HARDWARE #6"); // Text
RenderUIImage(458,714,222,23,0); // Button (6) QUAD:builtin-white
// BTN Button (6): ?
// C# Button (6): UIButtonMask.cs
// C# Button (6): HardwareInvButton.cs
RenderTextL(458,714,T_GREEN,FONT_NORMAL,0.6,"HARDWARE #7"); // Text
RenderUIImage(681,575,222,23,0); // Button (7) QUAD:builtin-white
// BTN Button (7): ?
// C# Button (7): UIButtonMask.cs
// C# Button (7): HardwareInvButton.cs
RenderTextL(681,575,T_GREEN,FONT_NORMAL,0.6,"HARDWARE #8"); // Text
RenderUIImage(681,598,222,23,0); // Button (8) QUAD:builtin-white
// BTN Button (8): ?
// C# Button (8): UIButtonMask.cs
// C# Button (8): HardwareInvButton.cs
RenderTextL(681,598,T_GREEN,FONT_NORMAL,0.6,"HARDWARE #9"); // Text
RenderUIImage(681,621,222,23,0); // Button (9) QUAD:builtin-white
// BTN Button (9): ?
// C# Button (9): UIButtonMask.cs
// C# Button (9): HardwareInvButton.cs
RenderTextL(681,621,T_GREEN,FONT_NORMAL,0.6,"HARDWARE #10"); // Text
RenderUIImage(681,644,222,23,0); // Button (10) QUAD:builtin-white
// BTN Button (10): ?
// C# Button (10): UIButtonMask.cs
// C# Button (10): HardwareInvButton.cs
RenderTextL(681,644,T_GREEN,FONT_NORMAL,0.6,"HARDWARE #11"); // Text
RenderUIImage(681,667,222,23,0); // Button (11) QUAD:builtin-white
// BTN Button (11): ?
// C# Button (11): UIButtonMask.cs
// C# Button (11): HardwareInvButton.cs
RenderTextL(681,667,T_GREEN,FONT_NORMAL,0.6,"HARDWARE #12"); // Text
RenderUIImage(681,691,222,23,0); // Button (12) QUAD:builtin-white
// BTN Button (12): ?
// C# Button (12): UIButtonMask.cs
// C# Button (12): HardwareInvButton.cs
RenderTextL(681,691,T_GREEN,FONT_NORMAL,0.6,"HARDWARE #13"); // Text
RenderUIImage(681,714,222,23,0); // Button (13) QUAD:builtin-white
// BTN Button (13): ?
// C# Button (13): UIButtonMask.cs
// C# Button (13): HardwareInvButton.cs
RenderTextL(681,714,T_GREEN,FONT_NORMAL,0.6,"HARDWARE #14"); // Text
}
if(MFD_CenterTab==3){ // General
RenderTextL(454,557,T_RED,FONT_NORMAL,0.6,"%s",875<1100?Sys_Text.stringTable[875]:"GENERAL"); // Label
// C# Label: UIPointerMask.cs
RenderUIImage(454,573,453,191,0); // GeneralInventory QUAD:builtin-knob
RenderUIImage(454,573,226,24,0); // AccessCardsButton QUAD:builtin-white
// BTN AccessCardsButton: ?
// C# AccessCardsButton: GeneralInvButton.cs
// C# AccessCardsButton: UIButtonMask.cs
RenderTextL(454,573,T_YELLOW,FONT_NORMAL,0.6,"ACCESS CARDS"); // Text
RenderUIImage(454,597,226,24,0); // Button (1) QUAD:builtin-white
// BTN Button (1): ?
// C# Button (1): GeneralInvButton.cs
// C# Button (1): UIButtonMask.cs
RenderTextL(454,597,T_GREEN,FONT_NORMAL,0.6,"GENERIC OBJECT #2"); // Text
RenderUIImage(641,599,20,20,1086); // ApplySubButton (1)
// BTN ApplySubButton (1): Button
// C# ApplySubButton (1): UIPointerMask.cs
RenderUIImage(646,603,11,11,1079); // Image
RenderUIImage(454,620,226,24,0); // Button (2) QUAD:builtin-white
// BTN Button (2): ?
// C# Button (2): GeneralInvButton.cs
// C# Button (2): UIButtonMask.cs
RenderTextL(454,620,T_GREEN,FONT_NORMAL,0.6,"GENERIC OBJECT #3"); // Text
RenderUIImage(641,622,20,20,1086); // ApplySubButton (2)
// BTN ApplySubButton (2): Button
// C# ApplySubButton (2): UIPointerMask.cs
RenderUIImage(646,627,11,11,1079); // Image
RenderUIImage(454,644,226,24,0); // Button (3) QUAD:builtin-white
// BTN Button (3): ?
// C# Button (3): GeneralInvButton.cs
// C# Button (3): UIButtonMask.cs
RenderTextL(454,644,T_GREEN,FONT_NORMAL,0.6,"GENERIC OBJECT #4"); // Text
RenderUIImage(641,646,20,20,1086); // ApplySubButton (3)
// BTN ApplySubButton (3): Button
// C# ApplySubButton (3): UIPointerMask.cs
RenderUIImage(646,650,11,11,1079); // Image
RenderUIImage(454,667,226,24,0); // Button (4) QUAD:builtin-white
// BTN Button (4): ?
// C# Button (4): GeneralInvButton.cs
// C# Button (4): UIButtonMask.cs
RenderTextL(454,667,T_GREEN,FONT_NORMAL,0.6,"GENERIC OBJECT #5"); // Text
RenderUIImage(641,669,20,20,1086); // ApplySubButton (4)
// BTN ApplySubButton (4): Button
// C# ApplySubButton (4): UIPointerMask.cs
RenderUIImage(646,674,11,11,1079); // Image
RenderUIImage(454,691,226,24,0); // Button (5) QUAD:builtin-white
// BTN Button (5): ?
// C# Button (5): GeneralInvButton.cs
// C# Button (5): UIButtonMask.cs
RenderTextL(454,691,T_GREEN,FONT_NORMAL,0.6,"GENERIC OBJECT #6"); // Text
RenderUIImage(641,693,20,20,1086); // ApplySubButton (5)
// BTN ApplySubButton (5): Button
// C# ApplySubButton (5): UIPointerMask.cs
RenderUIImage(646,697,11,11,1079); // Image
RenderUIImage(454,714,226,24,0); // Button (6) QUAD:builtin-white
// BTN Button (6): ?
// C# Button (6): GeneralInvButton.cs
// C# Button (6): UIButtonMask.cs
RenderTextL(454,714,T_GREEN,FONT_NORMAL,0.6,"GENERIC OBJECT #7"); // Text
RenderUIImage(641,716,20,20,1086); // ApplySubButton (6)
// BTN ApplySubButton (6): Button
// C# ApplySubButton (6): UIPointerMask.cs
RenderUIImage(646,721,11,11,1079); // Image
RenderUIImage(681,573,226,24,0); // Button (7) QUAD:builtin-white
// BTN Button (7): ?
// C# Button (7): GeneralInvButton.cs
// C# Button (7): UIButtonMask.cs
RenderTextL(681,573,T_GREEN,FONT_NORMAL,0.6,"GENERIC OBJECT #8"); // Text
RenderUIImage(868,575,20,20,1086); // ApplySubButton (7)
// BTN ApplySubButton (7): Button
// C# ApplySubButton (7): UIPointerMask.cs
RenderUIImage(872,580,11,11,1079); // Image
RenderUIImage(681,597,226,24,0); // Button (8) QUAD:builtin-white
// BTN Button (8): ?
// C# Button (8): GeneralInvButton.cs
// C# Button (8): UIButtonMask.cs
RenderTextL(681,597,T_GREEN,FONT_NORMAL,0.6,"GENERIC OBJECT #9"); // Text
RenderUIImage(868,599,20,20,1086); // ApplySubButton (8)
// BTN ApplySubButton (8): Button
// C# ApplySubButton (8): UIPointerMask.cs
RenderUIImage(872,603,11,11,1079); // Image
RenderUIImage(681,620,226,24,0); // Button (9) QUAD:builtin-white
// BTN Button (9): ?
// C# Button (9): GeneralInvButton.cs
// C# Button (9): UIButtonMask.cs
RenderTextL(681,620,T_GREEN,FONT_NORMAL,0.6,"GENERIC OBJECT #10"); // Text
RenderUIImage(868,622,20,20,1086); // ApplySubButton (9)
// BTN ApplySubButton (9): Button
// C# ApplySubButton (9): UIPointerMask.cs
RenderUIImage(872,627,11,11,1079); // Image
RenderUIImage(681,644,226,24,0); // Button (10) QUAD:builtin-white
// BTN Button (10): ?
// C# Button (10): GeneralInvButton.cs
// C# Button (10): UIButtonMask.cs
RenderTextL(681,644,T_GREEN,FONT_NORMAL,0.6,"GENERIC OBJECT #11"); // Text
RenderUIImage(868,646,20,20,1086); // ApplySubButton (10)
// BTN ApplySubButton (10): Button
// C# ApplySubButton (10): UIPointerMask.cs
RenderUIImage(872,650,11,11,1079); // Image
RenderUIImage(681,667,226,24,0); // Button (11) QUAD:builtin-white
// BTN Button (11): ?
// C# Button (11): GeneralInvButton.cs
// C# Button (11): UIButtonMask.cs
RenderTextL(681,667,T_GREEN,FONT_NORMAL,0.6,"GENERIC OBJECT #12"); // Text
RenderUIImage(868,669,20,20,1086); // ApplySubButton (11)
// BTN ApplySubButton (11): Button
// C# ApplySubButton (11): UIPointerMask.cs
RenderUIImage(872,674,11,11,1079); // Image
RenderUIImage(681,691,226,24,0); // Button (12) QUAD:builtin-white
// BTN Button (12): ?
// C# Button (12): GeneralInvButton.cs
// C# Button (12): UIButtonMask.cs
RenderTextL(681,691,T_GREEN,FONT_NORMAL,0.6,"GENERIC OBJECT #13"); // Text
RenderUIImage(868,693,20,20,1086); // ApplySubButton (12)
// BTN ApplySubButton (12): Button
// C# ApplySubButton (12): UIPointerMask.cs
RenderUIImage(872,697,11,11,1079); // Image
RenderUIImage(681,714,226,24,0); // Button (13) QUAD:builtin-white
// BTN Button (13): ?
// C# Button (13): GeneralInvButton.cs
// C# Button (13): UIButtonMask.cs
RenderTextL(681,714,T_GREEN,FONT_NORMAL,0.6,"GENERIC OBJECT #14"); // Text
RenderUIImage(868,716,20,20,1086); // ApplySubButton (13)
// BTN ApplySubButton (13): Button
// C# ApplySubButton (13): UIPointerMask.cs
RenderUIImage(872,721,11,11,1079); // Image
}
if(MFD_CenterTab==4){ // Software
RenderTextL(454,588,T_RED,FONT_NORMAL,0.6,"%s",876<1100?Sys_Text.stringTable[876]:"SOFTS"); // Label
// C# Label: UIPointerMask.cs
RenderUIImage(454,604,226,24,0); // ICEDrill QUAD:builtin-white
// BTN ICEDrill: ICEDrill.SoftInvClick()
// C# ICEDrill: UIButtonMask.cs
// C# ICEDrill: SoftwareInvButton.cs
RenderTextL(454,604,T_YELLOW,FONT_NORMAL,0.6,"I.C.E. DRILL"); // Text
// C# Text: SoftwareInvText.cs
RenderTextL(512,605,T_YELLOW,FONT_NORMAL,0.6,"V1"); // VersionText
RenderUIImage(454,624,226,24,0); // Pulser QUAD:builtin-white
// BTN Pulser: Pulser.SoftInvClick()
// C# Pulser: UIButtonMask.cs
// C# Pulser: SoftwareInvButton.cs
RenderTextL(454,624,T_GREEN,FONT_NORMAL,0.6,"PULSER"); // Text
// C# Text: SoftwareInvText.cs
RenderTextL(512,625,T_GREEN,FONT_NORMAL,0.6,"V1"); // VersionText
RenderUIImage(454,723,226,24,0); // CyberShield QUAD:builtin-white
// BTN CyberShield: CyberShield.SoftInvClick()
// C# CyberShield: UIButtonMask.cs
// C# CyberShield: SoftwareInvButton.cs
RenderTextL(454,723,T_GREEN,FONT_NORMAL,0.6,"CYBER SHIELD"); // Text
// C# Text: SoftwareInvText.cs
RenderTextL(512,724,T_GREEN,FONT_NORMAL,0.6,"V1"); // VersionText
RenderUIImage(681,604,226,24,0); // Turbo QUAD:builtin-white
// BTN Turbo: Turbo.SoftInvClick()
// C# Turbo: UIButtonMask.cs
// C# Turbo: SoftwareInvButton.cs
RenderTextL(681,604,T_GREEN,FONT_NORMAL,0.6,"TURBO"); // Text
// C# Text: SoftwareInvText.cs
RenderTextL(738,601,T_GREEN,FONT_NORMAL,0.6,"V1"); // TurboCountText
// C# TurboCountText: SoftwareButtonText.cs
RenderUIImage(839,605,20,20,1086); // UseButton
// BTN UseButton: Turbo.SoftInvClick(7)
// C# UseButton: UIPointerMask.cs
RenderUIImage(843,609,11,11,1079); // Image
RenderUIImage(681,624,226,24,0); // Decoy QUAD:builtin-white
// BTN Decoy: Decoy.SoftInvClick()
// C# Decoy: UIButtonMask.cs
// C# Decoy: SoftwareInvButton.cs
RenderTextL(681,624,T_GREEN,FONT_NORMAL,0.6,"DECOY"); // Text
// C# Text: SoftwareInvText.cs
RenderTextL(738,625,T_GREEN,FONT_NORMAL,0.6,"V1"); // DecoyCountText
// C# DecoyCountText: SoftwareButtonText.cs
RenderUIImage(839,625,20,20,1086); // UseButton
// BTN UseButton: Decoy.SoftInvClick(7)
// C# UseButton: UIPointerMask.cs
RenderUIImage(843,629,11,11,1079); // Image
RenderUIImage(681,643,226,24,0); // Recall QUAD:builtin-white
// BTN Recall: Recall.SoftInvClick()
// C# Recall: UIButtonMask.cs
// C# Recall: SoftwareInvButton.cs
RenderTextL(681,643,T_GREEN,FONT_NORMAL,0.6,"RECALL"); // Text
// C# Text: SoftwareInvText.cs
RenderTextL(738,644,T_GREEN,FONT_NORMAL,0.6,"V1"); // RecallCountText
// C# RecallCountText: SoftwareButtonText.cs
RenderUIImage(839,645,20,20,1086); // UseButton
// BTN UseButton: Recall.SoftInvClick(7)
// C# UseButton: UIPointerMask.cs
RenderUIImage(843,649,11,11,1079); // Image
RenderUIImage(681,723,226,24,0); // Games QUAD:builtin-white
// BTN Games: Games.SoftInvClick()
// C# Games: UIButtonMask.cs
// C# Games: SoftwareInvButton.cs
RenderTextL(681,723,T_GREEN,FONT_NORMAL,0.6,"GAMES"); // Text
// C# Text: SoftwareInvText.cs
}
if(MFD_CenterTab==5){ // EReader
RenderTextL(454,557,T_RED,FONT_NORMAL,0.6,"%s",877<1100?Sys_Text.stringTable[877]:"LOGS"); // MultiMediaHeaderLabel
// C# MultiMediaHeaderLabel: UIPointerMask.cs
if(MFD_MediaTab==MM_LOG_TABLE){ // LogTable
if(MFD_ReaderView==MFD_READER_CONTENTS){
RenderUIImage(454,573,453,191,0); // LogTableofContents QUAD:builtin-knob
// C# LogTableofContents: LogTableContentsButtonsManager.cs
RenderUIImage(454,573,226,24,0); // Button QUAD:builtin-white
// BTN Button: ?
// C# Button: UIButtonMask.cs
// C# Button: MultiMediaLogTableButton.cs
RenderTextL(454,573,T_GREEN,FONT_NORMAL,0.6,"Level R Logs"); // Text
RenderTextL(531,573,T_GREEN,FONT_NORMAL,0.6,"3"); // CountText
// C# CountText: LogCountsText.cs
RenderUIImage(454,597,226,24,0); // Button (1) QUAD:builtin-white
// BTN Button (1): ?
// C# Button (1): UIButtonMask.cs
// C# Button (1): MultiMediaLogTableButton.cs
RenderTextL(454,597,T_GREEN,FONT_NORMAL,0.6,"Level 1 Logs"); // Text
RenderTextL(531,597,T_GREEN,FONT_NORMAL,0.6,"3"); // CountText (1)
// C# CountText (1): LogCountsText.cs
RenderUIImage(454,620,226,24,0); // Button (2) QUAD:builtin-white
// BTN Button (2): ?
// C# Button (2): UIButtonMask.cs
// C# Button (2): MultiMediaLogTableButton.cs
RenderTextL(454,620,T_GREEN,FONT_NORMAL,0.6,"Level 2 Logs"); // Text
RenderTextL(531,620,T_GREEN,FONT_NORMAL,0.6,"3"); // CountText (2)
// C# CountText (2): LogCountsText.cs
RenderUIImage(454,644,226,24,0); // Button (3) QUAD:builtin-white
// BTN Button (3): ?
// C# Button (3): UIButtonMask.cs
// C# Button (3): MultiMediaLogTableButton.cs
RenderTextL(454,644,T_GREEN,FONT_NORMAL,0.6,"Level 3 Logs"); // Text
RenderTextL(531,644,T_GREEN,FONT_NORMAL,0.6,"3"); // CountText (3)
// C# CountText (3): LogCountsText.cs
RenderUIImage(454,667,226,24,0); // Button (4) QUAD:builtin-white
// BTN Button (4): ?
// C# Button (4): UIButtonMask.cs
// C# Button (4): MultiMediaLogTableButton.cs
RenderTextL(454,667,T_GREEN,FONT_NORMAL,0.6,"Level 4 Logs"); // Text
RenderTextL(531,667,T_GREEN,FONT_NORMAL,0.6,"3"); // CountText (4)
// C# CountText (4): LogCountsText.cs
RenderUIImage(454,691,226,24,0); // Button (5) QUAD:builtin-white
// BTN Button (5): ?
// C# Button (5): UIButtonMask.cs
// C# Button (5): MultiMediaLogTableButton.cs
RenderTextL(454,691,T_GREEN,FONT_NORMAL,0.6,"Level 5 Logs"); // Text
RenderTextL(531,691,T_GREEN,FONT_NORMAL,0.6,"3"); // CountText (5)
// C# CountText (5): LogCountsText.cs
RenderUIImage(454,714,226,24,0); // Button (6) QUAD:builtin-white
// BTN Button (6): ?
// C# Button (6): UIButtonMask.cs
// C# Button (6): MultiMediaLogTableButton.cs
RenderTextL(454,714,T_GREEN,FONT_NORMAL,0.6,"Level 6 Logs"); // Text
RenderTextL(531,714,T_GREEN,FONT_NORMAL,0.6,"3"); // CountText (6)
// C# CountText (6): LogCountsText.cs
RenderUIImage(681,573,226,24,0); // Button (7) QUAD:builtin-white
// BTN Button (7): ?
// C# Button (7): UIButtonMask.cs
// C# Button (7): MultiMediaLogTableButton.cs
RenderTextL(681,573,T_GREEN,FONT_NORMAL,0.6,"Level 7 Logs"); // Text
RenderTextL(751,573,T_GREEN,FONT_NORMAL,0.6,"3"); // CountText (7)
// C# CountText (7): LogCountsText.cs
RenderUIImage(681,597,226,24,0); // Button (8) QUAD:builtin-white
// BTN Button (8): ?
// C# Button (8): UIButtonMask.cs
// C# Button (8): MultiMediaLogTableButton.cs
RenderTextL(681,597,T_GREEN,FONT_NORMAL,0.6,"Level 8 Logs"); // Text
RenderTextL(751,597,T_GREEN,FONT_NORMAL,0.6,"3"); // CountText (8)
// C# CountText (8): LogCountsText.cs
RenderUIImage(681,620,226,24,0); // Button (9) QUAD:builtin-white
// BTN Button (9): ?
// C# Button (9): UIButtonMask.cs
// C# Button (9): MultiMediaLogTableButton.cs
RenderTextL(681,620,T_GREEN,FONT_NORMAL,0.6,"Level 9 Logs"); // Text
RenderTextL(751,620,T_GREEN,FONT_NORMAL,0.6,"3"); // CountText (9)
// C# CountText (9): LogCountsText.cs
}else if(MFD_ReaderView==MFD_READER_FOLDER){
RenderUIImage(458,570,445,188,0); // LogsLevelFolder QUAD:builtin-knob
// C# LogsLevelFolder: LogContentsButtonsManager.cs
RenderUIImage(458,570,222,21,0); // Button QUAD:builtin-white
// BTN Button: ?
// C# Button: UIButtonMask.cs
// C# Button: MultiMediaLogButton.cs
RenderTextL(458,570,T_GREEN,FONT_NORMAL,0.6,"Log"); // Text0
RenderUIImage(458,591,222,21,0); // Button (1) QUAD:builtin-white
// BTN Button (1): ?
// C# Button (1): UIButtonMask.cs
// C# Button (1): MultiMediaLogButton.cs
RenderTextL(458,591,T_GREEN,FONT_NORMAL,0.6,"Log"); // Text1
RenderUIImage(458,612,222,21,0); // Button (2) QUAD:builtin-white
// BTN Button (2): ?
// C# Button (2): UIButtonMask.cs
// C# Button (2): MultiMediaLogButton.cs
RenderTextL(458,612,T_GREEN,FONT_NORMAL,0.6,"Log"); // Text2
RenderUIImage(458,633,222,21,0); // Button (3) QUAD:builtin-white
// BTN Button (3): ?
// C# Button (3): UIButtonMask.cs
// C# Button (3): MultiMediaLogButton.cs
RenderTextL(458,633,T_GREEN,FONT_NORMAL,0.6,"Log"); // Text3
RenderUIImage(458,654,222,21,0); // Button (4) QUAD:builtin-white
// BTN Button (4): ?
// C# Button (4): UIButtonMask.cs
// C# Button (4): MultiMediaLogButton.cs
RenderTextL(458,654,T_GREEN,FONT_NORMAL,0.6,"Log"); // Text4
RenderUIImage(458,675,222,21,0); // Button (5) QUAD:builtin-white
// BTN Button (5): ?
// C# Button (5): UIButtonMask.cs
// C# Button (5): MultiMediaLogButton.cs
RenderTextL(458,675,T_GREEN,FONT_NORMAL,0.6,"Log"); // Text5
RenderUIImage(458,696,222,21,0); // Button (6) QUAD:builtin-white
// BTN Button (6): ?
// C# Button (6): UIButtonMask.cs
// C# Button (6): MultiMediaLogButton.cs
RenderTextL(458,696,T_GREEN,FONT_NORMAL,0.6,"Log"); // Text6
RenderUIImage(458,717,222,21,0); // Button (7) QUAD:builtin-white
// BTN Button (7): ?
// C# Button (7): UIButtonMask.cs
// C# Button (7): MultiMediaLogButton.cs
RenderTextL(458,717,T_GREEN,FONT_NORMAL,0.6,"Log"); // Text7
RenderUIImage(681,570,222,21,0); // Button (8) QUAD:builtin-white
// BTN Button (8): ?
// C# Button (8): UIButtonMask.cs
// C# Button (8): MultiMediaLogButton.cs
RenderTextL(681,570,T_GREEN,FONT_NORMAL,0.6,"Log"); // Text8
RenderUIImage(681,591,222,21,0); // Button (9) QUAD:builtin-white
// BTN Button (9): ?
// C# Button (9): UIButtonMask.cs
// C# Button (9): MultiMediaLogButton.cs
RenderTextL(681,591,T_GREEN,FONT_NORMAL,0.6,"Log"); // Text9
RenderUIImage(681,612,222,21,0); // Button (10) QUAD:builtin-white
// BTN Button (10): ?
// C# Button (10): UIButtonMask.cs
// C# Button (10): MultiMediaLogButton.cs
RenderTextL(681,612,T_GREEN,FONT_NORMAL,0.6,"Log"); // Text10
RenderUIImage(681,633,222,21,0); // Button (11) QUAD:builtin-white
// BTN Button (11): ?
// C# Button (11): UIButtonMask.cs
// C# Button (11): MultiMediaLogButton.cs
RenderTextL(681,633,T_GREEN,FONT_NORMAL,0.6,"Log"); // Text11
RenderUIImage(681,654,222,21,0); // Button (12) QUAD:builtin-white
// BTN Button (12): ?
// C# Button (12): UIButtonMask.cs
// C# Button (12): MultiMediaLogButton.cs
RenderTextL(681,654,T_GREEN,FONT_NORMAL,0.6,"Log"); // Text12
RenderUIImage(681,675,222,21,0); // Button (13) QUAD:builtin-white
// BTN Button (13): ?
// C# Button (13): UIButtonMask.cs
// C# Button (13): MultiMediaLogButton.cs
RenderTextL(681,675,T_GREEN,FONT_NORMAL,0.6,"Log"); // Text13
RenderUIImage(681,696,222,21,0); // Button (14) QUAD:builtin-white
// BTN Button (14): ?
// C# Button (14): UIButtonMask.cs
// C# Button (14): MultiMediaLogButton.cs
RenderTextL(681,696,T_GREEN,FONT_NORMAL,0.6,"Log"); // Text14
}else if(MFD_ReaderView==MFD_READER_TEXT){
// C# LogTextReader: LogTextReaderManager.cs
RenderTextL(449,576,T_GREEN,FONT_NORMAL,0.6,"\"abc def ghi jkl mno pqrs tuv wxyz ABC DEF GHI JKL MNO PQRS TUV WXYZ !\"\\xA7\n$%%& /() =?* '<> #|; \\xB2\\xB3~ @`\\xB4 \\xA9\\xAB\\xBB \\xA4\\xBC\\x..."); // LogTextOutput
// C# LogTextOutput: UIPointerMask.cs
RenderUIImage(454,576,456,174,0); // MoreButton QUAD:builtin-white
// BTN MoreButton: ?
// C# MoreButton: UIButtonMask.cs
// C# MoreButton: LogMoreButton.cs
RenderTextL(654,647,T_YELLOW,FONT_NORMAL,0.6,"%s",26<1100?Sys_Text.stringTable[26]:"[MORE]"); // Text0
RenderUIImage(453,718,69,31,0); // BackButton QUAD:builtin-white
// BTN BackButton: ?
// C# BackButton: UIButtonMask.cs
// C# BackButton: LogBackButton.cs
RenderTextL(453,718,T_YELLOW,FONT_NORMAL,0.6,"%s",879<1100?Sys_Text.stringTable[879]:"[BACK]"); // Text0
}
}
if(MFD_MediaTab==MM_EMAIL_TABLE){ // Email
RenderUIImage(458,570,445,188,0); // EmailTab QUAD:builtin-knob
// C# EmailTab: EmailContentsButtonsManager.cs
RenderUIImage(458,570,223,21,0); // Button QUAD:builtin-white
// BTN Button: ?
// C# Button: UIButtonMask.cs
// C# Button: MultiMediaLogButton.cs
RenderTextL(458,570,T_GREEN,FONT_NORMAL,0.6,"Email"); // Text0
RenderUIImage(458,591,223,21,0); // Button (1) QUAD:builtin-white
// BTN Button (1): ?
// C# Button (1): UIButtonMask.cs
// C# Button (1): MultiMediaLogButton.cs
RenderTextL(458,591,T_GREEN,FONT_NORMAL,0.6,"Email"); // Text1
RenderUIImage(458,612,223,21,0); // Button (2) QUAD:builtin-white
// BTN Button (2): ?
// C# Button (2): UIButtonMask.cs
// C# Button (2): MultiMediaLogButton.cs
RenderTextL(458,612,T_GREEN,FONT_NORMAL,0.6,"Email"); // Text2
RenderUIImage(458,633,223,21,0); // Button (3) QUAD:builtin-white
// BTN Button (3): ?
// C# Button (3): UIButtonMask.cs
// C# Button (3): MultiMediaLogButton.cs
RenderTextL(458,633,T_GREEN,FONT_NORMAL,0.6,"Email"); // Text3
RenderUIImage(458,654,223,21,0); // Button (4) QUAD:builtin-white
// BTN Button (4): ?
// C# Button (4): UIButtonMask.cs
// C# Button (4): MultiMediaLogButton.cs
RenderTextL(458,654,T_GREEN,FONT_NORMAL,0.6,"Email"); // Text4
RenderUIImage(458,675,223,21,0); // Button (5) QUAD:builtin-white
// BTN Button (5): ?
// C# Button (5): UIButtonMask.cs
// C# Button (5): MultiMediaLogButton.cs
RenderTextL(458,675,T_GREEN,FONT_NORMAL,0.6,"Email"); // Text5
RenderUIImage(458,696,223,21,0); // Button (6) QUAD:builtin-white
// BTN Button (6): ?
// C# Button (6): UIButtonMask.cs
// C# Button (6): MultiMediaLogButton.cs
RenderTextL(458,696,T_GREEN,FONT_NORMAL,0.6,"Email"); // Text6
RenderUIImage(458,717,223,21,0); // Button (7) QUAD:builtin-white
// BTN Button (7): ?
// C# Button (7): UIButtonMask.cs
// C# Button (7): MultiMediaLogButton.cs
RenderTextL(458,717,T_GREEN,FONT_NORMAL,0.6,"Email"); // Text7
RenderUIImage(681,570,223,21,0); // Button (8) QUAD:builtin-white
// BTN Button (8): ?
// C# Button (8): UIButtonMask.cs
// C# Button (8): MultiMediaLogButton.cs
RenderTextL(681,570,T_GREEN,FONT_NORMAL,0.6,"Email"); // Text8
RenderUIImage(681,591,223,21,0); // Button (9) QUAD:builtin-white
// BTN Button (9): ?
// C# Button (9): UIButtonMask.cs
// C# Button (9): MultiMediaLogButton.cs
RenderTextL(681,591,T_GREEN,FONT_NORMAL,0.6,"Email"); // Text9
RenderUIImage(681,612,223,21,0); // Button (10) QUAD:builtin-white
// BTN Button (10): ?
// C# Button (10): UIButtonMask.cs
// C# Button (10): MultiMediaLogButton.cs
RenderTextL(681,612,T_GREEN,FONT_NORMAL,0.6,"Email"); // Text10
RenderUIImage(681,633,223,21,0); // Button (11) QUAD:builtin-white
// BTN Button (11): ?
// C# Button (11): UIButtonMask.cs
// C# Button (11): MultiMediaLogButton.cs
RenderTextL(681,633,T_GREEN,FONT_NORMAL,0.6,"Email"); // Text11
RenderUIImage(681,654,223,21,0); // Button (12) QUAD:builtin-white
// BTN Button (12): ?
// C# Button (12): UIButtonMask.cs
// C# Button (12): MultiMediaLogButton.cs
RenderTextL(681,654,T_GREEN,FONT_NORMAL,0.6,"Email"); // Text12
RenderUIImage(681,675,223,21,0); // Button (13) QUAD:builtin-white
// BTN Button (13): ?
// C# Button (13): UIButtonMask.cs
// C# Button (13): MultiMediaLogButton.cs
RenderTextL(681,675,T_GREEN,FONT_NORMAL,0.6,"Email"); // Text13
RenderUIImage(681,696,223,21,0); // Button (14) QUAD:builtin-white
// BTN Button (14): ?
// C# Button (14): UIButtonMask.cs
// C# Button (14): MultiMediaLogButton.cs
RenderTextL(681,696,T_GREEN,FONT_NORMAL,0.6,"Email"); // Text14
RenderUIImage(681,696,223,21,0); // Button (15) QUAD:builtin-white
// BTN Button (15): ?
// C# Button (15): UIButtonMask.cs
// C# Button (15): MultiMediaLogButton.cs
RenderTextL(681,696,T_GREEN,FONT_NORMAL,0.6,"Email"); // Text14
RenderUIImage(681,696,223,21,0); // Button (16) QUAD:builtin-white
// BTN Button (16): ?
// C# Button (16): UIButtonMask.cs
// C# Button (16): MultiMediaLogButton.cs
RenderTextL(681,696,T_GREEN,FONT_NORMAL,0.6,"Email"); // Text14
RenderUIImage(681,696,223,21,0); // Button (17) QUAD:builtin-white
// BTN Button (17): ?
// C# Button (17): UIButtonMask.cs
// C# Button (17): MultiMediaLogButton.cs
RenderTextL(681,696,T_GREEN,FONT_NORMAL,0.6,"Email"); // Text14
RenderUIImage(681,696,223,21,0); // Button (18) QUAD:builtin-white
// BTN Button (18): ?
// C# Button (18): UIButtonMask.cs
// C# Button (18): MultiMediaLogButton.cs
RenderTextL(681,696,T_GREEN,FONT_NORMAL,0.6,"Email"); // Text14
RenderUIImage(681,696,223,21,0); // Button (19) QUAD:builtin-white
// BTN Button (19): ?
// C# Button (19): UIButtonMask.cs
// C# Button (19): MultiMediaLogButton.cs
RenderTextL(681,696,T_GREEN,FONT_NORMAL,0.6,"Email"); // Text14
RenderUIImage(681,696,223,21,0); // Button (20) QUAD:builtin-white
// BTN Button (20): ?
// C# Button (20): UIButtonMask.cs
// C# Button (20): MultiMediaLogButton.cs
RenderTextL(681,696,T_GREEN,FONT_NORMAL,0.6,"Email"); // Text14
RenderUIImage(681,696,223,21,0); // Button (21) QUAD:builtin-white
// BTN Button (21): ?
// C# Button (21): UIButtonMask.cs
// C# Button (21): MultiMediaLogButton.cs
RenderTextL(681,696,T_GREEN,FONT_NORMAL,0.6,"Email"); // Text14
RenderUIImage(681,696,223,21,0); // Button (22) QUAD:builtin-white
// BTN Button (22): ?
// C# Button (22): UIButtonMask.cs
// C# Button (22): MultiMediaLogButton.cs
RenderTextL(681,696,T_GREEN,FONT_NORMAL,0.6,"Email"); // Text14
RenderUIImage(681,696,223,21,0); // Button (23) QUAD:builtin-white
// BTN Button (23): ?
// C# Button (23): UIButtonMask.cs
// C# Button (23): MultiMediaLogButton.cs
RenderTextL(681,696,T_GREEN,FONT_NORMAL,0.6,"Email"); // Text14
RenderUIImage(681,696,223,21,0); // Button (24) QUAD:builtin-white
// BTN Button (24): ?
// C# Button (24): UIButtonMask.cs
// C# Button (24): MultiMediaLogButton.cs
RenderTextL(681,696,T_GREEN,FONT_NORMAL,0.6,"Email"); // Text14
RenderUIImage(681,696,223,21,0); // Button (25) QUAD:builtin-white
// BTN Button (25): ?
// C# Button (25): UIButtonMask.cs
// C# Button (25): MultiMediaLogButton.cs
RenderTextL(681,696,T_GREEN,FONT_NORMAL,0.6,"Email"); // Text14
}
if(MFD_MediaTab==MM_DATA_TABLE){ // DataTab
RenderUIImage(458,570,445,188,0); // DataTab QUAD:builtin-knob
// C# DataTab: EmailContentsButtonsManager.cs
RenderUIImage(458,570,223,21,0); // Button QUAD:builtin-white
// BTN Button: ?
// C# Button: UIButtonMask.cs
// C# Button: MultiMediaLogButton.cs
RenderTextL(458,570,T_GREEN,FONT_NORMAL,0.6,"Data"); // Text0
RenderUIImage(458,591,223,21,0); // Button (1) QUAD:builtin-white
// BTN Button (1): ?
// C# Button (1): UIButtonMask.cs
// C# Button (1): MultiMediaLogButton.cs
RenderTextL(458,591,T_GREEN,FONT_NORMAL,0.6,"Data"); // Text1
RenderUIImage(458,612,223,21,0); // Button (2) QUAD:builtin-white
// BTN Button (2): ?
// C# Button (2): UIButtonMask.cs
// C# Button (2): MultiMediaLogButton.cs
RenderTextL(458,612,T_GREEN,FONT_NORMAL,0.6,"Data"); // Text2
RenderUIImage(458,633,223,21,0); // Button (3) QUAD:builtin-white
// BTN Button (3): ?
// C# Button (3): UIButtonMask.cs
// C# Button (3): MultiMediaLogButton.cs
RenderTextL(458,633,T_GREEN,FONT_NORMAL,0.6,"Data"); // Text3
RenderUIImage(458,654,223,21,0); // Button (4) QUAD:builtin-white
// BTN Button (4): ?
// C# Button (4): UIButtonMask.cs
// C# Button (4): MultiMediaLogButton.cs
RenderTextL(458,654,T_GREEN,FONT_NORMAL,0.6,"Data"); // Text4
RenderUIImage(458,675,223,21,0); // Button (5) QUAD:builtin-white
// BTN Button (5): ?
// C# Button (5): UIButtonMask.cs
// C# Button (5): MultiMediaLogButton.cs
RenderTextL(458,675,T_GREEN,FONT_NORMAL,0.6,"Data"); // Text5
RenderUIImage(458,696,223,21,0); // Button (6) QUAD:builtin-white
// BTN Button (6): ?
// C# Button (6): UIButtonMask.cs
// C# Button (6): MultiMediaLogButton.cs
RenderTextL(458,696,T_GREEN,FONT_NORMAL,0.6,"Data"); // Text6
RenderUIImage(458,717,223,21,0); // Button (7) QUAD:builtin-white
// BTN Button (7): ?
// C# Button (7): UIButtonMask.cs
// C# Button (7): MultiMediaLogButton.cs
RenderTextL(458,717,T_GREEN,FONT_NORMAL,0.6,"Data"); // Text7
RenderUIImage(681,570,223,21,0); // Button (8) QUAD:builtin-white
// BTN Button (8): ?
// C# Button (8): UIButtonMask.cs
// C# Button (8): MultiMediaLogButton.cs
RenderTextL(681,570,T_GREEN,FONT_NORMAL,0.6,"Data"); // Text8
RenderUIImage(681,591,223,21,0); // Button (9) QUAD:builtin-white
// BTN Button (9): ?
// C# Button (9): UIButtonMask.cs
// C# Button (9): MultiMediaLogButton.cs
RenderTextL(681,591,T_GREEN,FONT_NORMAL,0.6,"Data"); // Text9
RenderUIImage(681,612,223,21,0); // Button (10) QUAD:builtin-white
// BTN Button (10): ?
// C# Button (10): UIButtonMask.cs
// C# Button (10): MultiMediaLogButton.cs
RenderTextL(681,612,T_GREEN,FONT_NORMAL,0.6,"Data"); // Text10
RenderUIImage(681,633,223,21,0); // Button (11) QUAD:builtin-white
// BTN Button (11): ?
// C# Button (11): UIButtonMask.cs
// C# Button (11): MultiMediaLogButton.cs
RenderTextL(681,633,T_GREEN,FONT_NORMAL,0.6,"Data"); // Text11
RenderUIImage(681,654,223,21,0); // Button (12) QUAD:builtin-white
// BTN Button (12): ?
// C# Button (12): UIButtonMask.cs
// C# Button (12): MultiMediaLogButton.cs
RenderTextL(681,654,T_GREEN,FONT_NORMAL,0.6,"Data"); // Text12
}
if(MFD_MediaTab==MM_NOTES){ // Notes
RenderUIImage(453,570,166,39,0); // NoteToggle QUAD:none
// C# NoteToggle: UIPointerMask.cs
RenderUIImage(453,572,19,18,910); // Background
RenderTextL(474,573,T_GREEN,FONT_NORMAL,0.6,"%s%d%s%s%u.",Sys_Text.stringTable[556],1,Sys_Text.stringTable[557],Sys_Text.stringTable[558],World.lev1SecCode); // Label
RenderUIImage(453,599,166,39,0); // NoteToggle1 QUAD:none
// C# NoteToggle1: UIPointerMask.cs
RenderUIImage(453,600,19,18,910); // Background
RenderTextL(474,602,T_GREEN,FONT_NORMAL,0.6,"%s%d%s%s%u.",Sys_Text.stringTable[556],2,Sys_Text.stringTable[557],Sys_Text.stringTable[558],World.lev2SecCode); // Label1
RenderUIImage(453,628,166,39,0); // NoteToggle2 QUAD:none
// C# NoteToggle2: UIPointerMask.cs
RenderUIImage(453,629,19,18,910); // Background
RenderTextL(474,631,T_GREEN,FONT_NORMAL,0.6,"%s%d%s%s%u.",Sys_Text.stringTable[556],3,Sys_Text.stringTable[557],Sys_Text.stringTable[558],World.lev3SecCode); // Label2
RenderUIImage(453,657,166,39,0); // NoteToggle3 QUAD:none
// C# NoteToggle3: UIPointerMask.cs
RenderUIImage(453,658,19,18,910); // Background
RenderTextL(474,660,T_GREEN,FONT_NORMAL,0.6,"%s%d%s%s%u.",Sys_Text.stringTable[556],4,Sys_Text.stringTable[557],Sys_Text.stringTable[558],World.lev4SecCode); // Label3
RenderUIImage(453,686,166,39,0); // NoteToggle4 QUAD:none
// C# NoteToggle4: UIPointerMask.cs
RenderUIImage(453,687,19,18,910); // Background
RenderTextL(474,689,T_GREEN,FONT_NORMAL,0.6,"%s%d%s%s%u.",Sys_Text.stringTable[556],5,Sys_Text.stringTable[557],Sys_Text.stringTable[558],World.lev5SecCode); // Label4
RenderUIImage(453,715,166,39,0); // NoteToggle5 QUAD:none
// C# NoteToggle5: UIPointerMask.cs
RenderUIImage(453,716,19,18,910); // Background
RenderTextL(474,718,T_GREEN,FONT_NORMAL,0.6,"%s%d%s%s%u.",Sys_Text.stringTable[556],6,Sys_Text.stringTable[557],Sys_Text.stringTable[558],World.lev6SecCode); // Label5
RenderUIImage(620,570,166,39,0); // NoteToggle6 QUAD:none
// C# NoteToggle6: UIPointerMask.cs
RenderUIImage(620,572,19,18,910); // Background
RenderTextL(641,573,T_GREEN,FONT_NORMAL,0.6,"Escape neurosurgery suite.  Keycode is 451."); // Label6
RenderUIImage(620,599,166,39,0); // NoteToggle7 QUAD:none
// C# NoteToggle7: UIPointerMask.cs
RenderUIImage(620,600,19,18,910); // Background
RenderTextL(641,602,T_GREEN,FONT_NORMAL,0.6,"Disengage laser safety override."); // Label7
RenderUIImage(620,628,166,39,0); // NoteToggle8 QUAD:none
// C# NoteToggle8: UIPointerMask.cs
RenderUIImage(620,629,19,18,910); // Background
RenderTextL(641,631,T_GREEN,FONT_NORMAL,0.6,"Activate the station energy shield."); // Label8
RenderUIImage(620,657,166,39,0); // NoteToggle9 QUAD:none
// C# NoteToggle9: UIPointerMask.cs
RenderUIImage(620,658,19,18,910); // Background
RenderTextL(641,660,T_GREEN,FONT_NORMAL,0.6,"Destroy the mining laser."); // Label9
RenderUIImage(620,686,166,39,0); // NoteToggle10 QUAD:none
// C# NoteToggle10: UIPointerMask.cs
RenderUIImage(620,687,19,18,910); // Background
RenderTextL(641,689,T_GREEN,FONT_NORMAL,0.6,"Enable master jettison."); // Label10
RenderUIImage(620,715,166,39,0); // NoteToggle11 QUAD:none
// C# NoteToggle11: UIPointerMask.cs
RenderUIImage(620,716,19,18,910); // Background
RenderTextL(641,718,T_GREEN,FONT_NORMAL,0.6,"Diagnose and repair broken relay: 428."); // Label11
RenderUIImage(787,570,166,39,0); // NoteToggle12 QUAD:none
// C# NoteToggle12: UIPointerMask.cs
RenderUIImage(787,572,19,18,910); // Background
RenderTextL(808,573,T_GREEN,FONT_NORMAL,0.6,"Jettison Beta Grove."); // Label12
RenderUIImage(787,599,166,39,0); // NoteToggle13 QUAD:none
// C# NoteToggle13: UIPointerMask.cs
RenderUIImage(787,600,19,18,910); // Background
RenderTextL(808,602,T_GREEN,FONT_NORMAL,0.6,"Destroy the four relay antennae."); // Label13
RenderUIImage(787,628,166,39,0); // NoteToggle14 QUAD:none
// C# NoteToggle14: UIPointerMask.cs
RenderUIImage(787,629,19,18,910); // Background
RenderTextL(808,631,T_GREEN,FONT_NORMAL,0.6,"Engage reactor self-destruct."); // Label14
RenderUIImage(787,657,166,39,0); // NoteToggle15 QUAD:none
// C# NoteToggle15: UIPointerMask.cs
RenderUIImage(787,658,19,18,910); // Background
RenderTextL(808,660,T_GREEN,FONT_NORMAL,0.6,"Escape on escape pod."); // Label15
RenderUIImage(787,686,166,39,0); // NoteToggle16 QUAD:none
// C# NoteToggle16: UIPointerMask.cs
RenderUIImage(787,687,19,18,910); // Background
RenderTextL(808,689,T_GREEN,FONT_NORMAL,0.6,"Access the bridge."); // Label16
RenderUIImage(787,715,166,39,0); // NoteToggle17 QUAD:none
// C# NoteToggle17: UIPointerMask.cs
RenderUIImage(787,716,19,18,910); // Background
RenderTextL(808,718,T_GREEN,FONT_NORMAL,0.6,"Destroy SHODAN."); // Label17
// CenterMFD tab buttons + AddToInventoryHelper live in CenterMFD already
// C# TabsRH: LeftMFDTabs.cs
// C# ItemTabRH: ItemTabManager.cs
}
}
if(MFD_RightTab==2 && !mfdItemReader[1]){ // ItemTabRH
RenderUIImage(1092,528,237,237,1025); // ItemIcon UNMAPPED:[Textures/UI/itemicons/paperico.png]
// C# ItemIcon: ItemIconManager.cs
// C# ItemIcon: UIPointerMask.cs
RenderTextL(1087,540,T_YELLOW,FONT_NORMAL,0.6,"TEST"); // ItemText
// C# ItemText: UIPointerMask.cs
RenderUIImage(1131,628,160,40,1087); // VaporizeButton
// BTN VaporizeButton: VaporizeButton.OnVaporizeClick()
// C# VaporizeButton: VaporizeButton.cs
// C# VaporizeButton: UIButtonMask.cs
RenderTextL(1131,628,T_GREEN_MENU,FONT_NORMAL,0.6,"%s",883<1100?Sys_Text.stringTable[883]:"VAPORIZE"); // Text
RenderUIImage(1131,691,160,40,1087); // ApplyButton
// BTN ApplyButton: MFDManager.ApplyButtonClicked()
// C# ApplyButton: UIButtonMask.cs
RenderTextL(1131,691,T_GREEN_MENU,FONT_NORMAL,0.6,"%s",736<1100?Sys_Text.stringTable[736]:"APPLY"); // Text
RenderUIImage(1131,691,160,40,1087); // UseButton
// BTN UseButton: UseButton.OnActivateClick()
// C# UseButton: ActivateButton.cs
RenderTextL(1131,691,T_GREEN_MENU,FONT_NORMAL,0.6,"USE"); // Text
RenderTextL(1124,615,T_YELLOW,FONT_NORMAL,0.6,"STD"); // AccessCardsList
// C# AccessCardsList: UIPointerMask.cs
// C# GrenadeTimerSliderRH: UIPointerMask.cs
// C# GrenadeTimerSliderRH: GrenadeTimerSlider.cs
RenderUIImage(1095,714,230,29,952); // Background
// C# Background: UIPointerMask.cs
RenderUIImage(1095,714,72,29,1079); // Fill
// C# Fill: UIPointerMask.cs
RenderUIImage(1162,711,24,36,953); // Handle
// C# Handle: UIPointerMask.cs
RenderTextL(1119,747,T_YELLOW,FONT_NORMAL,0.6,"TEST"); // TimeNumberText
RenderUIImage(1094,712,230,32,952); // Background
RenderUIImage(1094,712,16,32,0); // Fill QUAD:builtin-white
RenderUIImage(1094,696,32,64,953); // Handle
}
if(MFD_RightTab==5){ // DataTabRH
if(MFD_DataR==8){ // Blocked
RenderUIImage(1090,535,227,209,1025); // BlockedBySecurityRH UNMAPPED:[Resources/BlockedBySecurity/blocked_00.
// C# BlockedBySecurityRH: ImageSequenceTextureArrayUI.cs
// C# BlockedBySecurityRH: PooledItemDestroy.cs
RenderTextL(1104,542,T_YELLOW,FONT_NORMAL,0.6,"%s",890<1100?Sys_Text.stringTable[890]:"Blocked by SHODAN level Security."); // BlockedBySecurityText
// C# BlockedBySecurityText: UIPointerMask.cs
}
if(MFD_DataR==5){ // Search
RenderTextL(1092,536,T_YELLOW,FONT_NORMAL,0.6,"DEAD CORTEX REAVER"); // DataHeaderTextRH
// C# DataHeaderTextRH: UIPointerMask.cs
}
if(MFD_DataR==0){ // idle
RenderTextL(1083,633,T_YELLOW,FONT_NORMAL,0.6,"%s",891<1100?Sys_Text.stringTable[891]:"No Items"); // DataNoItemsTextRH
// C# DataNoItemsTextRH: UIPointerMask.cs
// C# ElevatorUIControlRH: ElevatorKeypad.cs
}
if(MFD_DataR==1){ // Elevator
RenderUIImage(1191,531,32,32,929); // CurrentFloorIndicator
RenderUIImage(1145,578,45,168,0); // ButtonBankLH QUAD:builtin-knob
RenderUIImage(1145,578,45,39,1025); // ElevButton1 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]
// C# ElevButton1: UIButtonMask.cs
RenderUIImage(1147,583,40,34,1025); // Keypad.Button (1) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Keypad.Button (1): Keypad.Button
// C# Keypad.Button (1): ElevatorButton.cs
// C# Keypad.Button (1): UIButtonMask.cs
RenderTextL(1148,580,T_GREEN,FONT_NORMAL,0.6,"R"); // Text (1)
RenderUIImage(1145,620,45,39,1025); // ElevButton2 UNMAPPED:[Textures/UI/hudbuttons/keypad_mid.png]
// C# ElevButton2: UIButtonMask.cs
RenderUIImage(1147,623,40,34,1025); // Keypad.Button (2) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Keypad.Button (2): Keypad.Button
// C# Keypad.Button (2): ElevatorButton.cs
// C# Keypad.Button (2): UIButtonMask.cs
RenderTextL(1148,620,T_GREEN,FONT_NORMAL,0.6,"1"); // Text (2)
RenderUIImage(1145,663,45,39,1025); // ElevButton3 UNMAPPED:[Textures/UI/hudbuttons/keypad_mid.png]
// C# ElevButton3: UIButtonMask.cs
RenderUIImage(1147,666,40,34,1025); // Keypad.Button (3) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Keypad.Button (3): Keypad.Button
// C# Keypad.Button (3): ElevatorButton.cs
// C# Keypad.Button (3): UIButtonMask.cs
RenderTextL(1148,663,T_GREEN,FONT_NORMAL,0.6,"2"); // Text (3)
RenderUIImage(1145,706,45,39,1025); // ElevButton4 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]
// C# ElevButton4: UIButtonMask.cs
RenderUIImage(1147,707,40,34,1025); // Keypad.Button (4) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Keypad.Button (4): Keypad.Button
// C# Keypad.Button (4): ElevatorButton.cs
// C# Keypad.Button (4): UIButtonMask.cs
RenderTextL(1148,704,T_GREEN,FONT_NORMAL,0.6,"3"); // Text (4)
RenderUIImage(1223,578,45,168,0); // ButtonBankRH QUAD:builtin-knob
RenderUIImage(1223,578,45,39,1025); // ElevButton5 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]
// C# ElevButton5: UIButtonMask.cs
RenderUIImage(1226,582,40,34,1025); // Keypad.Button (5) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Keypad.Button (5): Keypad.Button
// C# Keypad.Button (5): ElevatorButton.cs
// C# Keypad.Button (5): UIButtonMask.cs
RenderTextL(1227,580,T_GREEN,FONT_NORMAL,0.6,"6"); // Text (5)
RenderUIImage(1223,620,45,39,1025); // ElevButton6 UNMAPPED:[Textures/UI/hudbuttons/keypad_mid.png]
// C# ElevButton6: UIButtonMask.cs
RenderUIImage(1226,623,40,34,1025); // Keypad.Button (6) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Keypad.Button (6): Keypad.Button
// C# Keypad.Button (6): ElevatorButton.cs
// C# Keypad.Button (6): UIButtonMask.cs
RenderTextL(1227,620,T_GREEN,FONT_NORMAL,0.6,"7"); // Text (6)
RenderUIImage(1223,663,45,39,1025); // ElevButton7 UNMAPPED:[Textures/UI/hudbuttons/keypad_mid.png]
// C# ElevButton7: UIButtonMask.cs
RenderUIImage(1226,666,40,34,1025); // Keypad.Button (7) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Keypad.Button (7): Keypad.Button
// C# Keypad.Button (7): ElevatorButton.cs
// C# Keypad.Button (7): UIButtonMask.cs
RenderTextL(1227,663,T_GREEN,FONT_NORMAL,0.6,"8"); // Text (7)
RenderUIImage(1223,706,45,39,1025); // ElevButton8 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]
// C# ElevButton8: UIButtonMask.cs
RenderUIImage(1226,707,40,34,1025); // Keypad.Button (8) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Keypad.Button (8): Keypad.Button
// C# Keypad.Button (8): ElevatorButton.cs
// C# Keypad.Button (8): UIButtonMask.cs
RenderTextL(1227,704,T_GREEN,FONT_NORMAL,0.6,"9"); // Text (8)
RenderUIImage(1305,528,29,29,899); // CloseButton
// BTN CloseButton: MFDManager.CloseElevatorPad()
// C# CloseButton: UIButtonMask.cs
RenderTextL(1305,531,T_STOPD_RED,FONT_NORMAL,0.6,"X"); // Text
// C# KeycodeUIControlRH: KeypadKeycodeButtons.cs
}
if(MFD_DataR==2){ // Keycode
RenderUIImage(1144,577,42,38,1025); // KeycodeButton1 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]
RenderUIImage(1147,580,38,35,1025); // Button (1) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Button (1): ?
// C# Button (1): KeycodeButton.cs
RenderTextL(1139,572,T_GREEN,FONT_NORMAL,0.6,"1"); // Text
RenderUIImage(1186,577,42,38,1025); // KeycodeButton2 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]
RenderUIImage(1188,580,38,35,1025); // Button (2) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Button (2): ?
// C# Button (2): KeycodeButton.cs
RenderTextL(1180,572,T_GREEN,FONT_NORMAL,0.6,"2"); // Text
RenderUIImage(1228,577,42,38,1025); // KeycodeButton3 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]
RenderUIImage(1228,580,38,35,1025); // Button (3) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Button (3): ?
// C# Button (3): KeycodeButton.cs
RenderTextL(1220,572,T_GREEN,FONT_NORMAL,0.6,"3"); // Text
RenderUIImage(1144,620,42,38,1025); // KeycodeButton4 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]
RenderUIImage(1147,621,38,35,1025); // Button (4) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Button (4): ?
// C# Button (4): KeycodeButton.cs
RenderTextL(1139,614,T_GREEN,FONT_NORMAL,0.6,"4"); // Text
RenderUIImage(1186,620,42,38,1025); // KeycodeButton5 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]
RenderUIImage(1188,621,38,35,1025); // Button (5) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Button (5): ?
// C# Button (5): KeycodeButton.cs
RenderTextL(1180,614,T_GREEN,FONT_NORMAL,0.6,"5"); // Text
RenderUIImage(1228,620,42,38,1025); // KeycodeButton6 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]
RenderUIImage(1228,621,38,35,1025); // Button (6) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Button (6): ?
// C# Button (6): KeycodeButton.cs
RenderTextL(1220,614,T_GREEN,FONT_NORMAL,0.6,"6"); // Text
RenderUIImage(1144,663,42,38,1025); // KeycodeButton7 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]
RenderUIImage(1147,665,38,35,1025); // Button (7) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Button (7): ?
// C# Button (7): KeycodeButton.cs
RenderTextL(1139,657,T_GREEN,FONT_NORMAL,0.6,"7"); // Text
RenderUIImage(1186,663,42,38,1025); // KeycodeButton8 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]
RenderUIImage(1188,665,38,35,1025); // Button (8) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Button (8): ?
// C# Button (8): KeycodeButton.cs
RenderTextL(1180,657,T_GREEN,FONT_NORMAL,0.6,"8"); // Text
RenderUIImage(1228,663,42,38,1025); // KeycodeButton9 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]
RenderUIImage(1228,665,38,35,1025); // Button (9) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Button (9): ?
// C# Button (9): KeycodeButton.cs
RenderTextL(1220,657,T_GREEN,FONT_NORMAL,0.6,"9"); // Text
RenderUIImage(1144,706,42,38,1025); // KeycodeButtonBackSpace UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]
RenderUIImage(1147,707,38,35,1025); // Button (-) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Button (-): ?
// C# Button (-): KeycodeButton.cs
RenderTextL(1139,700,T_GREEN,FONT_NORMAL,0.6,"-"); // Text
RenderUIImage(1186,706,42,38,1025); // KeycodeButton0 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]
RenderUIImage(1188,707,38,35,1025); // Button (0) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Button (0): ?
// C# Button (0): KeycodeButton.cs
RenderTextL(1180,700,T_GREEN,FONT_NORMAL,0.6,"0"); // Text
RenderUIImage(1228,706,42,38,1025); // KeycodeButtonC UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]
RenderUIImage(1228,707,38,35,1025); // Button (C) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on.
// BTN Button (C): ?
// C# Button (C): KeycodeButton.cs
RenderTextL(1220,700,T_GREEN,FONT_NORMAL,0.6,"C"); // Text
RenderUIImage(1232,526,32,32,1025); // KeycodeOnes UNMAPPED:[Textures/UI/elnum_null.png]
// C# KeycodeOnes: KeycodeDigitImage.cs
RenderUIImage(1191,526,32,32,1025); // KeycodeTens UNMAPPED:[Textures/UI/elnum_null.png]
// C# KeycodeTens: KeycodeDigitImage.cs
RenderUIImage(1149,526,32,32,1025); // KeycodeHuns UNMAPPED:[Textures/UI/elnum_null.png]
// C# KeycodeHuns: KeycodeDigitImage.cs
RenderUIImage(1314,525,29,29,899); // CloseButton
// BTN CloseButton: MFDManager.CloseKeycodePad()
// C# CloseButton: UIButtonMask.cs
RenderTextL(1314,529,T_STOPD_RED,FONT_NORMAL,0.6,"X"); // Text
}
if(MFD_DataR==5){ // Search
RenderUIImage(1079,528,263,240,0); // SearchContentsContainerRH QUAD:builtin-knob
// C# SearchContentsContainerRH: SearchButton.cs
RenderUIImage(1318,528,29,29,899); // SearchCloseButtonRH
// BTN SearchCloseButtonRH: MFDManager.CloseSearch()
// C# SearchCloseButtonRH: UIButtonMask.cs
RenderTextL(1318,531,T_STOPD_RED,FONT_NORMAL,0.6,"X"); // Text
RenderUIImage(1143,584,64,64,0); // SearchContentRH1 QUAD:none
// BTN SearchContentRH1: SearchContentsContainerRH.SearchButtonClick()
// C# SearchContentRH1: UIButtonMask.cs
// C# SearchContentRH1: SearchContainerButton.cs
RenderUIImage(1233,584,64,64,0); // SearchContentRH2 QUAD:none
// BTN SearchContentRH2: SearchContentsContainerRH.SearchButtonClick(1)
// C# SearchContentRH2: UIButtonMask.cs
// C# SearchContentRH2: SearchContainerButton.cs
RenderUIImage(1143,674,64,64,0); // SearchContentRH3 QUAD:none
// BTN SearchContentRH3: SearchContentsContainerRH.SearchButtonClick(2)
// C# SearchContentRH3: UIButtonMask.cs
// C# SearchContentRH3: SearchContainerButton.cs
RenderUIImage(1233,674,64,64,0); // SearchContentRH4 QUAD:none
// BTN SearchContentRH4: SearchContentsContainerRH.SearchButtonClick(3)
// C# SearchContentRH4: UIButtonMask.cs
// C# SearchContentRH4: SearchContainerButton.cs
if (World.invP1.currentSearchItem >= 0) {
    int s = World.invP1.currentSearchItem;
    for (int i = 0; i < 4; i++) {
        int tex = 965; // frobicon dummy (RH)
        int cx[4] = {1143, 1233, 1143, 1233}; int cy[4] = {584, 584, 674, 674};
        RenderUIImage(cx[i], cy[i], 64, 64, tex); // Search content slot (dummy for positioning)
    }
}
// C# AudioLogInfoRH: LogDataTabContainerManager.cs
}
if(MFD_DataR==6){ // AudioLog
RenderUIImage(1079,528,263,240,1272); // LogImage
RenderTextL(1088,540,T_YELLOW,FONT_NORMAL,0.6,"HACKER IS AWESOME"); // LogName
// C# LogName: UIPointerMask.cs
RenderTextL(1088,557,T_YELLOW,FONT_NORMAL,0.6,"Sender: SHODAN"); // SenderText
// C# SenderText: UIPointerMask.cs
RenderTextL(1088,701,T_YELLOW,FONT_NORMAL,0.6,"Subject:\n\nif only i had a sparq beam then all the world would be right"); // SubjectText
// C# SubjectText: UIPointerMask.cs
// C# PuzzleGridRH: PuzzleGrid.cs
}
if(MFD_DataR==3){ // GridPuzzle
RenderUIImage(1101,554,221,163,1025); // OuterColorBorder UNMAPPED:[Textures/UI/puzzle/gridcontainer_gray.p
RenderUIImage(1104,558,214,157,1025); // ContainerEdge UNMAPPED:[Textures/UI/puzzle/gridcontainer.png]
RenderUIImage(1084,620,29,29,1025); // NodeSource UNMAPPED:[Textures/UI/puzzle/node_source.png]
RenderUIImage(1309,620,29,29,1025); // Node UNMAPPED:[Textures/UI/puzzle/node_off.png]
RenderUIImage(1110,564,29,29,1025); // Button UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button: PuzzleGridRH.OnGridCellClick()
// C# Button: UIButtonMask.cs
// C# Button: PuzzleUIButton.cs
RenderTextL(1110,564,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(1110,564,29,29,1025); // GeniusHighlight UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(1139,564,29,29,1025); // Button (1) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (1): PuzzleGridRH.OnGridCellClick(1)
// C# Button (1): UIButtonMask.cs
// C# Button (1): PuzzleUIButton.cs
RenderTextL(1139,564,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(1139,564,29,29,1025); // GeniusHighlight (1) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(1168,564,29,29,1025); // Button (2) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (2): PuzzleGridRH.OnGridCellClick(2)
// C# Button (2): UIButtonMask.cs
// C# Button (2): PuzzleUIButton.cs
RenderTextL(1168,564,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(1168,564,29,29,1025); // GeniusHighlight (2) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(1196,564,29,29,1025); // Button (3) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (3): PuzzleGridRH.OnGridCellClick(3)
// C# Button (3): UIButtonMask.cs
// C# Button (3): PuzzleUIButton.cs
RenderTextL(1196,564,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(1196,564,29,29,1025); // GeniusHighlight (3) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(1225,564,29,29,1025); // Button (4) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (4): PuzzleGridRH.OnGridCellClick(4)
// C# Button (4): UIButtonMask.cs
// C# Button (4): PuzzleUIButton.cs
RenderTextL(1225,564,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(1225,564,29,29,1025); // GeniusHighlight (4) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(1254,564,29,29,1025); // Button (5) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (5): PuzzleGridRH.OnGridCellClick(5)
// C# Button (5): UIButtonMask.cs
// C# Button (5): PuzzleUIButton.cs
RenderTextL(1254,564,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(1254,564,29,29,1025); // GeniusHighlight (5) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(1283,564,29,29,1025); // Button (6) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (6): PuzzleGridRH.OnGridCellClick(6)
// C# Button (6): UIButtonMask.cs
// C# Button (6): PuzzleUIButton.cs
RenderTextL(1283,564,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(1283,564,29,29,1025); // GeniusHighlight (6) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(1110,593,29,29,1025); // Button (7) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (7): PuzzleGridRH.OnGridCellClick(7)
// C# Button (7): UIButtonMask.cs
// C# Button (7): PuzzleUIButton.cs
RenderTextL(1110,593,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(1110,593,29,29,1025); // GeniusHighlight (7) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(1139,593,29,29,1025); // Button (8) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (8): PuzzleGridRH.OnGridCellClick(8)
// C# Button (8): UIButtonMask.cs
// C# Button (8): PuzzleUIButton.cs
RenderTextL(1139,593,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(1139,593,29,29,1025); // GeniusHighlight (8) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(1168,593,29,29,1025); // Button (9) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (9): PuzzleGridRH.OnGridCellClick(9)
// C# Button (9): UIButtonMask.cs
// C# Button (9): PuzzleUIButton.cs
RenderTextL(1168,593,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(1168,593,29,29,1025); // GeniusHighlight (9) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(1196,593,29,29,1025); // Button (10) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (10): PuzzleGridRH.OnGridCellClick(10)
// C# Button (10): UIButtonMask.cs
// C# Button (10): PuzzleUIButton.cs
RenderTextL(1196,593,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(1196,593,29,29,1025); // GeniusHighlight (10) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(1225,593,29,29,1025); // Button (11) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (11): PuzzleGridRH.OnGridCellClick(11)
// C# Button (11): UIButtonMask.cs
// C# Button (11): PuzzleUIButton.cs
RenderTextL(1225,593,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(1225,593,29,29,1025); // GeniusHighlight (11) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(1254,593,29,29,1025); // Button (12) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (12): PuzzleGridRH.OnGridCellClick(12)
// C# Button (12): UIButtonMask.cs
// C# Button (12): PuzzleUIButton.cs
RenderTextL(1254,593,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(1254,593,29,29,1025); // GeniusHighlight (12) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(1283,593,29,29,1025); // Button (13) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (13): PuzzleGridRH.OnGridCellClick(13)
// C# Button (13): UIButtonMask.cs
// C# Button (13): PuzzleUIButton.cs
RenderTextL(1283,593,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(1283,593,29,29,1025); // GeniusHighlight (13) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(1110,622,29,29,1025); // Button (14) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (14): PuzzleGridRH.OnGridCellClick(14)
// C# Button (14): UIButtonMask.cs
// C# Button (14): PuzzleUIButton.cs
RenderTextL(1110,622,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(1110,622,29,29,1025); // GeniusHighlight (14) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(1139,622,29,29,1025); // Button (15) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (15): PuzzleGridRH.OnGridCellClick(15)
// C# Button (15): UIButtonMask.cs
// C# Button (15): PuzzleUIButton.cs
RenderTextL(1139,622,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(1139,622,29,29,1025); // GeniusHighlight (15) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(1168,622,29,29,1025); // Button (16) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (16): PuzzleGridRH.OnGridCellClick(16)
// C# Button (16): UIButtonMask.cs
// C# Button (16): PuzzleUIButton.cs
RenderTextL(1168,622,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(1168,622,29,29,1025); // GeniusHighlight (16) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(1196,622,29,29,1025); // Button (17) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (17): PuzzleGridRH.OnGridCellClick(17)
// C# Button (17): UIButtonMask.cs
// C# Button (17): PuzzleUIButton.cs
RenderTextL(1196,622,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(1196,622,29,29,1025); // GeniusHighlight (17) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(1225,622,29,29,1025); // Button (18) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (18): PuzzleGridRH.OnGridCellClick(18)
// C# Button (18): UIButtonMask.cs
// C# Button (18): PuzzleUIButton.cs
RenderTextL(1225,622,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(1225,622,29,29,1025); // GeniusHighlight (18) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(1254,622,29,29,1025); // Button (19) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (19): PuzzleGridRH.OnGridCellClick(19)
// C# Button (19): UIButtonMask.cs
// C# Button (19): PuzzleUIButton.cs
RenderTextL(1254,622,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(1254,622,29,29,1025); // GeniusHighlight (19) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(1283,622,29,29,1025); // Button (20) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (20): PuzzleGridRH.OnGridCellClick(20)
// C# Button (20): UIButtonMask.cs
// C# Button (20): PuzzleUIButton.cs
RenderTextL(1283,622,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(1283,622,29,29,1025); // GeniusHighlight (20) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(1110,650,29,29,1025); // Button (21) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (21): PuzzleGridRH.OnGridCellClick(21)
// C# Button (21): UIButtonMask.cs
// C# Button (21): PuzzleUIButton.cs
RenderTextL(1110,650,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(1110,650,29,29,1025); // GeniusHighlight (21) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(1139,650,29,29,1025); // Button (22) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (22): PuzzleGridRH.OnGridCellClick(22)
// C# Button (22): UIButtonMask.cs
// C# Button (22): PuzzleUIButton.cs
RenderTextL(1139,650,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(1139,650,29,29,1025); // GeniusHighlight (22) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(1168,650,29,29,1025); // Button (23) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (23): PuzzleGridRH.OnGridCellClick(23)
// C# Button (23): UIButtonMask.cs
// C# Button (23): PuzzleUIButton.cs
RenderTextL(1168,650,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(1168,650,29,29,1025); // GeniusHighlight (23) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(1196,650,29,29,1025); // Button (24) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (24): PuzzleGridRH.OnGridCellClick(24)
// C# Button (24): UIButtonMask.cs
// C# Button (24): PuzzleUIButton.cs
RenderTextL(1196,650,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(1196,650,29,29,1025); // GeniusHighlight (24) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(1225,650,29,29,1025); // Button (25) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (25): PuzzleGridRH.OnGridCellClick(25)
// C# Button (25): UIButtonMask.cs
// C# Button (25): PuzzleUIButton.cs
RenderTextL(1225,650,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(1225,650,29,29,1025); // GeniusHighlight (25) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(1254,650,29,29,1025); // Button (26) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (26): PuzzleGridRH.OnGridCellClick(26)
// C# Button (26): UIButtonMask.cs
// C# Button (26): PuzzleUIButton.cs
RenderTextL(1254,650,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(1254,650,29,29,1025); // GeniusHighlight (26) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(1283,650,29,29,1025); // Button (27) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (27): PuzzleGridRH.OnGridCellClick(27)
// C# Button (27): UIButtonMask.cs
// C# Button (27): PuzzleUIButton.cs
RenderTextL(1283,650,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(1283,650,29,29,1025); // GeniusHighlight (27) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(1110,679,29,29,1025); // Button (28) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (28): PuzzleGridRH.OnGridCellClick(28)
// C# Button (28): UIButtonMask.cs
// C# Button (28): PuzzleUIButton.cs
RenderTextL(1110,679,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(1110,679,29,29,1025); // GeniusHighlight (28) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(1139,679,29,29,1025); // Button (29) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (29): PuzzleGridRH.OnGridCellClick(29)
// C# Button (29): UIButtonMask.cs
// C# Button (29): PuzzleUIButton.cs
RenderTextL(1139,679,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(1139,679,29,29,1025); // GeniusHighlight (29) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(1168,679,29,29,1025); // Button (30) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (30): PuzzleGridRH.OnGridCellClick(30)
// C# Button (30): UIButtonMask.cs
// C# Button (30): PuzzleUIButton.cs
RenderTextL(1168,679,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(1168,679,29,29,1025); // GeniusHighlight (30) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(1196,679,29,29,1025); // Button (31) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (31): PuzzleGridRH.OnGridCellClick(31)
// C# Button (31): UIButtonMask.cs
// C# Button (31): PuzzleUIButton.cs
RenderTextL(1196,679,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(1196,679,29,29,1025); // GeniusHighlight (31) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(1225,679,29,29,1025); // Button (32) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (32): PuzzleGridRH.OnGridCellClick(32)
// C# Button (32): UIButtonMask.cs
// C# Button (32): PuzzleUIButton.cs
RenderTextL(1225,679,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(1225,679,29,29,1025); // GeniusHighlight (32) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(1254,679,29,29,1025); // Button (33) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (33): PuzzleGridRH.OnGridCellClick(33)
// C# Button (33): UIButtonMask.cs
// C# Button (33): PuzzleUIButton.cs
RenderTextL(1254,679,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(1254,679,29,29,1025); // GeniusHighlight (33) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(1283,679,29,29,1025); // Button (34) UNMAPPED:[Textures/UI/puzzle/grid1_base.png]
// BTN Button (34): PuzzleGridRH.OnGridCellClick(34)
// C# Button (34): UIButtonMask.cs
// C# Button (34): PuzzleUIButton.cs
RenderTextL(1283,679,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?"); // Text dummy
RenderUIImage(1283,679,29,29,1025); // GeniusHighlight (34) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight
RenderUIImage(1101,719,221,26,1025); // ProgressContainer UNMAPPED:[Textures/UI/puzzle/gridcontainer_gray.p
RenderUIImage(1104,726,225,13,0); // Background QUAD:builtin-knob
RenderUIImage(1107,726,6,13,1025); // Fill UNMAPPED:[Textures/UI/puzzle/puzzlesliderwire.png
RenderUIImage(1104,719,22,26,1078); // Handle
RenderUIImage(1318,526,29,29,899); // CloseButton
// BTN CloseButton: MFDManager.ClosePuzzleGrid()
// C# CloseButton: UIButtonMask.cs
RenderTextL(1318,530,T_STOPD_RED,FONT_NORMAL,0.6,"X"); // Text
// C# PuzzleWireRH: PuzzleWire.cs
}
if(MFD_DataR==4){ // WirePuzzle
RenderUIImage(1141,570,139,192,1025); // ContainerCenter UNMAPPED:[Textures/UI/puzzle/wire_center.png]
RenderUIImage(1093,521,235,44,1025); // LevelsBox UNMAPPED:[Textures/UI/puzzle/wire_levelsbox.png]
RenderUIImage(1099,526,235,34,0); // Background QUAD:builtin-knob
RenderUIImage(1102,526,6,34,1025); // Fill UNMAPPED:[Textures/UI/puzzle/puzzlesliderwire.png
RenderUIImage(1099,509,22,69,1078); // Handle
RenderUIImage(1262,522,66,42,1025); // TargetLine UNMAPPED:[Textures/UI/puzzle/wire_levelstargetlin
RenderUIImage(1116,566,26,29,1025); // NodeBase UNMAPPED:[Textures/UI/puzzle/wire_node.png]
// BTN NodeBase: PuzzleWireRH.ClickLHNode()
// C# NodeBase: UIButtonMask.cs
// C# NodeBase: PuzzleUIButton.cs
RenderUIImage(1120,572,16,16,0); // SelectedIndicator QUAD:none
RenderUIImage(1117,569,22,22,0); // GeniusHint QUAD:none
RenderUIImage(1116,594,26,29,1025); // NodeBase (1) UNMAPPED:[Textures/UI/puzzle/wire_node.png]
// BTN NodeBase (1): PuzzleWireRH.ClickLHNode(1)
// C# NodeBase (1): UIButtonMask.cs
// C# NodeBase (1): PuzzleUIButton.cs
RenderUIImage(1120,600,16,16,0); // SelectedIndicator (1) QUAD:none
RenderUIImage(1117,597,22,22,0); // GeniusHint (1) QUAD:none
RenderUIImage(1116,623,26,29,1025); // NodeBase (2) UNMAPPED:[Textures/UI/puzzle/wire_node.png]
// BTN NodeBase (2): PuzzleWireRH.ClickLHNode(2)
// C# NodeBase (2): UIButtonMask.cs
// C# NodeBase (2): PuzzleUIButton.cs
RenderUIImage(1120,629,16,16,0); // SelectedIndicator (2) QUAD:none
RenderUIImage(1117,626,22,22,0); // GeniusHint (2) QUAD:none
RenderUIImage(1116,651,26,29,1025); // NodeBase (3) UNMAPPED:[Textures/UI/puzzle/wire_node.png]
// BTN NodeBase (3): PuzzleWireRH.ClickLHNode(3)
// C# NodeBase (3): UIButtonMask.cs
// C# NodeBase (3): PuzzleUIButton.cs
RenderUIImage(1120,657,16,16,0); // SelectedIndicator (3) QUAD:none
RenderUIImage(1117,654,22,22,0); // GeniusHint (3) QUAD:none
RenderUIImage(1116,679,26,29,1025); // NodeBase (4) UNMAPPED:[Textures/UI/puzzle/wire_node.png]
// BTN NodeBase (4): PuzzleWireRH.ClickLHNode(4)
// C# NodeBase (4): UIButtonMask.cs
// C# NodeBase (4): PuzzleUIButton.cs
RenderUIImage(1120,685,16,16,0); // SelectedIndicator (4) QUAD:none
RenderUIImage(1117,682,22,22,0); // GeniusHint (4) QUAD:none
RenderUIImage(1116,707,26,29,1025); // NodeBase (5) UNMAPPED:[Textures/UI/puzzle/wire_node.png]
// BTN NodeBase (5): PuzzleWireRH.ClickLHNode(5)
// C# NodeBase (5): UIButtonMask.cs
// C# NodeBase (5): PuzzleUIButton.cs
RenderUIImage(1120,713,16,16,0); // SelectedIndicator (5) QUAD:none
RenderUIImage(1117,710,22,22,0); // GeniusHint (5) QUAD:none
RenderUIImage(1116,736,26,29,1025); // NodeBase (6) UNMAPPED:[Textures/UI/puzzle/wire_node.png]
// BTN NodeBase (6): PuzzleWireRH.ClickLHNode(6)
// C# NodeBase (6): UIButtonMask.cs
// C# NodeBase (6): PuzzleUIButton.cs
RenderUIImage(1120,743,16,16,0); // SelectedIndicator (6) QUAD:none
RenderUIImage(1117,740,22,22,0); // GeniusHint (6) QUAD:none
RenderUIImage(1281,566,26,29,1025); // NodeBase UNMAPPED:[Textures/UI/puzzle/wire_node.png]
// BTN NodeBase: PuzzleWireRH.ClickRHNode()
// C# NodeBase: UIButtonMask.cs
// C# NodeBase: PuzzleUIButton.cs
RenderUIImage(1285,572,16,16,0); // SelectedIndicator QUAD:none
RenderUIImage(1282,569,22,22,0); // GeniusHint QUAD:none
RenderUIImage(1281,594,26,29,1025); // NodeBase (1) UNMAPPED:[Textures/UI/puzzle/wire_node.png]
// BTN NodeBase (1): PuzzleWireRH.ClickRHNode(1)
// C# NodeBase (1): UIButtonMask.cs
// C# NodeBase (1): PuzzleUIButton.cs
RenderUIImage(1285,600,16,16,0); // SelectedIndicator (1) QUAD:none
RenderUIImage(1282,597,22,22,0); // GeniusHint (1) QUAD:none
RenderUIImage(1281,623,26,29,1025); // NodeBase (2) UNMAPPED:[Textures/UI/puzzle/wire_node.png]
// BTN NodeBase (2): PuzzleWireRH.ClickRHNode(2)
// C# NodeBase (2): UIButtonMask.cs
// C# NodeBase (2): PuzzleUIButton.cs
RenderUIImage(1285,629,16,16,0); // SelectedIndicator (2) QUAD:none
RenderUIImage(1282,626,22,22,0); // GeniusHint (2) QUAD:none
RenderUIImage(1281,651,26,29,1025); // NodeBase (3) UNMAPPED:[Textures/UI/puzzle/wire_node.png]
// BTN NodeBase (3): PuzzleWireRH.ClickRHNode(3)
// C# NodeBase (3): UIButtonMask.cs
// C# NodeBase (3): PuzzleUIButton.cs
RenderUIImage(1285,657,16,16,0); // SelectedIndicator (3) QUAD:none
RenderUIImage(1282,654,22,22,0); // GeniusHint (3) QUAD:none
RenderUIImage(1281,679,26,29,1025); // NodeBase (4) UNMAPPED:[Textures/UI/puzzle/wire_node.png]
// BTN NodeBase (4): PuzzleWireRH.ClickRHNode(4)
// C# NodeBase (4): UIButtonMask.cs
// C# NodeBase (4): PuzzleUIButton.cs
RenderUIImage(1285,685,16,16,0); // SelectedIndicator (4) QUAD:none
RenderUIImage(1282,682,22,22,0); // GeniusHint (4) QUAD:none
RenderUIImage(1281,707,26,29,1025); // NodeBase (5) UNMAPPED:[Textures/UI/puzzle/wire_node.png]
// BTN NodeBase (5): PuzzleWireRH.ClickRHNode(5)
// C# NodeBase (5): UIButtonMask.cs
// C# NodeBase (5): PuzzleUIButton.cs
RenderUIImage(1285,713,16,16,0); // SelectedIndicator (5) QUAD:none
RenderUIImage(1282,710,22,22,0); // GeniusHint (5) QUAD:none
RenderUIImage(1281,736,26,29,1025); // NodeBase (6) UNMAPPED:[Textures/UI/puzzle/wire_node.png]
// BTN NodeBase (6): PuzzleWireRH.ClickRHNode(6)
// C# NodeBase (6): UIButtonMask.cs
// C# NodeBase (6): PuzzleUIButton.cs
RenderUIImage(1285,743,16,16,0); // SelectedIndicator (6) QUAD:none
RenderUIImage(1282,740,22,22,0); // GeniusHint (6) QUAD:none
RenderUIImage(1318,736,29,29,899); // CloseButton
// BTN CloseButton: MFDManager.ClosePuzzleWire()
// C# CloseButton: UIButtonMask.cs
RenderTextL(1318,739,T_STOPD_RED,FONT_NORMAL,0.6,"X"); // Text
// C# SystemAnalyzerDisplayRH: SystemAnalyzer.cs
}
if(MFD_DataR==7){ // SysAnalyzer
RenderTextL(1082,523,T_YELLOW,FONT_NORMAL,0.6,"%s",892<1100?Sys_Text.stringTable[892]:"SYSTEM ANALYZER"); // Header
// C# Header: UIPointerMask.cs
RenderTextL(1083,547,T_GREEN,FONT_NORMAL,0.6,"Current level security:"); // DescriptionLevelSecurity
// C# DescriptionLevelSecurity: UIPointerMask.cs
RenderTextL(1239,547,T_GREEN,FONT_NORMAL,0.6,"100%%"); // TextLevelSecurity
// C# TextLevelSecurity: UIPointerMask.cs
RenderTextL(1083,566,T_GREEN,FONT_NORMAL,0.6,"Mining laser status:"); // DescriptionMiningLaser
// C# DescriptionMiningLaser: UIPointerMask.cs
RenderTextL(1238,566,T_GREEN,FONT_NORMAL,0.6,"Charging"); // TextLaserStatus
// C# TextLaserStatus: UIPointerMask.cs
RenderTextL(1083,585,T_GREEN,FONT_NORMAL,0.6,"Lifepod status:"); // DescriptionLifepods
// C# DescriptionLifepods: UIPointerMask.cs
RenderTextL(1238,585,T_GREEN,FONT_NORMAL,0.6,"Disabled"); // TextLifepodStatus
// C# TextLifepodStatus: UIPointerMask.cs
RenderTextL(1083,605,T_GREEN,FONT_NORMAL,0.6,"Station shield status:"); // DescriptionShield
// C# DescriptionShield: UIPointerMask.cs
RenderTextL(1238,605,T_GREEN,FONT_NORMAL,0.6,"Off"); // TextShieldStatus
// C# TextShieldStatus: UIPointerMask.cs
RenderTextL(1083,624,T_GREEN,FONT_NORMAL,0.6,"Reactor status:"); // DescriptionReactor
// C# DescriptionReactor: UIPointerMask.cs
RenderTextL(1238,624,T_GREEN,FONT_NORMAL,0.6,"Normal"); // TextReactorStatus
// C# TextReactorStatus: UIPointerMask.cs
RenderTextL(1083,643,T_GREEN,FONT_NORMAL,0.6,"Processor nodes:"); // DescriptionProcessors
// C# DescriptionProcessors: UIPointerMask.cs
RenderTextL(1238,643,T_GREEN,FONT_NORMAL,0.6,"99"); // TextProcessors
// C# TextProcessors: UIPointerMask.cs
RenderTextL(1083,662,T_GREEN,FONT_NORMAL,0.6,"Main Program:"); // DescriptionMainProgram
// C# DescriptionMainProgram: UIPointerMask.cs
RenderTextL(1238,662,T_GREEN,FONT_NORMAL,0.6,"Downloading to earth"); // TextMainProgram
// C# TextMainProgram: UIPointerMask.cs
RenderTextL(1083,681,T_GREEN,FONT_NORMAL,0.6,"Alpha Grove status:"); // DescriptionGroveAlphaStatus
// C# DescriptionGroveAlphaStatus: UIPointerMask.cs
RenderTextL(1238,681,T_GREEN,FONT_NORMAL,0.6,"normal"); // TextGroveAlpha
// C# TextGroveAlpha: UIPointerMask.cs
RenderTextL(1083,701,T_GREEN,FONT_NORMAL,0.6,"Beta Grove status:"); // DescriptionGroveBetaStatus
// C# DescriptionGroveBetaStatus: UIPointerMask.cs
RenderTextL(1238,701,T_GREEN,FONT_NORMAL,0.6,"normal"); // TextGroveBeta
// C# TextGroveBeta: UIPointerMask.cs
RenderTextL(1083,720,T_GREEN,FONT_NORMAL,0.6,"Gamma Grove status:"); // DescriptionGroveGammaStatus
// C# DescriptionGroveGammaStatus: UIPointerMask.cs
RenderTextL(1238,720,T_GREEN,FONT_NORMAL,0.6,"launched"); // TextGroveGamma
// C# TextGroveGamma: UIPointerMask.cs
RenderTextL(1083,739,T_GREEN,FONT_NORMAL,0.6,"Delta Grove status:"); // DescriptionGroveDeltaStatus
// C# DescriptionGroveDeltaStatus: UIPointerMask.cs
RenderTextL(1238,739,T_GREEN,FONT_NORMAL,0.6,"launched"); // TextGroveDelta
// C# TextGroveDelta: UIPointerMask.cs
RenderUIImage(1318,527,29,29,899); // CloseButton
// BTN CloseButton: SystemAnalyzerDisplayRH.Close()
// C# CloseButton: UIButtonMask.cs
RenderTextL(1318,527,T_STOPD_RED,FONT_NORMAL,0.6,"X"); // Text
// C# TabButtonsPanelRH: TabButtons.cs
}
}
// TabButtonsPanelRH side tab buttons live in SideMFD already
RenderTextL(1137,570,T_YELLOW,FONT_NORMAL,0.6,"level 1 elevator taken off line - SHODAN security block established 04.NOV.72"); // CyberSPrint
// C# CyberSPrint: UIPointerMask.cs
// C# CyberSPrint: PooledItemDestroy.cs
// C# BioMonitorContainer: BioMonitor.cs
// C# BioMonitorContainer: BiomonitorGraphSystem.cs
if((World.invP1.hardwareIsActive & HW_BIO)!=0){ // BioMonitor
RenderUIImage(0,0,480,80,0); // Graph QUAD:none
RenderTextL(4,83,T_YELLOW,FONT_NORMAL,0.6,"%s",895<1100?Sys_Text.stringTable[895]:"Biomonitor:"); // BiomonitorHeader
RenderTextL(4,99,T_GREEN,FONT_NORMAL,0.6,"%s",896<1100?Sys_Text.stringTable[896]:"Heart Rate:"); // BiomonitorTextHeart
RenderTextL(70,99,T_GREEN,FONT_NORMAL,0.6,"100"); // BiomonitorTextHeartRate
RenderTextL(122,99,T_GREEN,FONT_NORMAL,0.6,"BPM"); // BiomonitorTextBPM
RenderTextL(4,131,T_GREEN,FONT_NORMAL,0.6,"%s",897<1100?Sys_Text.stringTable[897]:"Patches Active:"); // BiomonitorTextPatch
RenderTextL(119,131,T_GREEN,FONT_NORMAL,0.6,"MEDI STAMINUP SIGHT GENIUS BERSERK REFLEX"); // BiomonitorTextPatchEffects
RenderTextL(4,115,T_GREEN,FONT_NORMAL,0.6,"%s",898<1100?Sys_Text.stringTable[898]:"Fatigue:"); // BiomonitorTextFatigueDetail
RenderTextL(66,115,T_GREEN,FONT_NORMAL,0.6,"Moderate"); // BiomonitorTextFatigue
}
RenderTextL(1270,78,T_WHITE,FONT_NORMAL,0.6,"0"); // EnergyDrainText dummy
RenderTextL(1308,78,T_WHITE,FONT_NORMAL,0.6,"0"); // EnergyJPMText dummy
RenderSearchFX();
        if (EditSelIsActive()) { // Edit mode selection highlight + object info panel
            u16 sel=editModeSelection; Entity* e=&World.instances[sel];
            V3 f=World.instances[PLAYER1].forward,rt=World.instances[PLAYER1].right,ff=(V3){-f.x,-f.y,-f.z},up=V3_Normalize(V3_Cross(rt,ff)),d=V3_AsubB(World.position[sel],World.position[PLAYER1]); float bz=V3_dot(d,f);
            if (bz > 0.01f) { float tanFov=vtan((float)Sys_Settings.FOV*0.5f*PI/180.0f),k=384.0f/(bz*tanFov); float sx=683.0f+V3_dot(d,rt)*k, sy=384.0f-V3_dot(d,up)*k; if (sx > -48.0f && sx < 1414.0f && sy > -48.0f && sy < 816.0f) RenderUIImage((i16)(sx-24.0f),(i16)(sy-24.0f),48,48,1051); }
            RenderUIImage(966,84,400,600,1025); // Edit object info panel bg
            RenderTextL(EF_LABELX,104,T_YELLOW,FONT_NORMAL,1.0f,"EDIT OBJECT #%u",sel); {char v[40];sFormat(v,40,"%u",e->index);RenderTextL(EF_LABELX,132,T_GREEN,FONT_NORMAL,1.0f,"const index");RenderTextL(EF_VALUEX,132,T_GREEN,FONT_NORMAL,1.0f,"%s",v);} bool caretOn=((u32)(get_time()*2.0f)&1)!=0;
            for(int i=0;i<EF_LAST;++i){u8 slot=(u8)i;i16 y=efRowY[i];char v[40];EditFieldValueText(slot,sel,v,40);
                bool editingThis=editFieldEditing&&editFieldSlot==slot; RenderTextL(EF_LABELX,y,editingThis?T_RED:T_GREEN,FONT_NORMAL,1.0f,"%s",efRowLabel[i]); 
                if(editingThis){char buf[44];sFormat(buf,44,"%s%s",editFieldBuffer,caretOn?"|":"");RenderTextL(EF_VALUEX,y,T_RED,FONT_NORMAL,1.0f,"%s",buf);}
                else {
                    float w=MeasureLineAdvance(v,FONT_NORMAL);bool hov=World.inventoryMode&&World.cursorPos_x>=EF_VALUEX&&World.cursorPos_x<=EF_VALUEX+w&&World.cursorPos_y>=y&&World.cursorPos_y<=y+26; 
                    if(hov){World.uiIsBlocking=true;if(!editFieldEditing&&(Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed||Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].pressed)){sFormat(editFieldBuffer,40,"%s",v);editFieldSlot=slot;editFieldEditing=true;Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed=Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].pressed=false;}} 
                    RenderTextL(EF_VALUEX,y,hov?T_YELLOW:(i<EF_TEX?T_WHITE:T_GREEN),FONT_NORMAL,1.0f,"%s",v);
                }
            }
            if(editFieldEditing&&Sys_Input.scrollDelta!=0.0f){EditFieldStep((float)Sys_Input.scrollDelta);Sys_Input.scrollDelta=0.0f;}
            if(sel==World.weaponVModelIndex){int wep16=Get16WeaponIndexFromConstIndex(e->index);if(wep16>=0&&wep16<16){V3 offs=vWepOfs[wep16];offs.y+=wfx.reloadContainerPos.y;RenderTextL(EF_LABELX,580,T_GREEN,FONT_NORMAL,1.0f,"vm offset");RenderTextL(EF_VALUEX,580,T_YELLOW,FONT_NORMAL,1.0f,"%.2f %.2f %.2f",offs.x,offs.y,offs.z);}}
        }
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
