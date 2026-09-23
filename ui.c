// ui.c - User Interface(UI) aka HUD
void BiomonitorBlitToUI();
#define UI_MFD_IDS(P) UI_ID_##P##_TAB_WEAPON,UI_ID_##P##_TAB_ITEM,UI_ID_##P##_TAB_AUTOMAP,UI_ID_##P##_TAB_DATA,UI_ID_##P##_PANEL,UI_ID_##P##_WEAPON_NAME,UI_ID_##P##_WEAPON_ICON,UI_ID_##P##_MEDIA_HEADER,UI_ID_##P##_MEDIA_TAB_0,UI_ID_##P##_MEDIA_TAB_3=UI_ID_##P##_MEDIA_TAB_0+3,UI_ID_##P##_ITEM_NAME,UI_ID_##P##_ITEM_ICON,UI_ID_##P##_ITEM_USE,UI_ID_##P##_ITEM_VAPORIZE,UI_ID_##P##_ITEM_TIMER_VALUE,UI_ID_##P##_ITEM_TIMER_SLIDER,UI_ID_##P##_ITEM_ACCESS_CARDS,UI_ID_##P##_BLOCKED_SECURITY_TEXT,\
    UI_ID_##P##_ELEV_FLOOR_INDICATOR,UI_ID_##P##_ELEV_BUTTON_0,UI_ID_##P##_ELEV_BUTTON_7=UI_ID_##P##_ELEV_BUTTON_0+7,UI_ID_##P##_ELEV_CLOSE,UI_ID_##P##_KEYCODE_0,UI_ID_##P##_KEYCODE_11=UI_ID_##P##_KEYCODE_0+11,UI_ID_##P##_KEYCODE_DIGIT_0,UI_ID_##P##_KEYCODE_DIGIT_2=UI_ID_##P##_KEYCODE_DIGIT_0+2,UI_ID_##P##_KEYCODE_CLOSE,UI_ID_##P##_AUDIOLOG_IMAGE,UI_ID_##P##_AUDIOLOG_NAME,UI_ID_##P##_AUDIOLOG_SENDER,UI_ID_##P##_AUDIOLOG_SUBJECT,\
    UI_ID_##P##_PUZZLE_NODE_SOURCE,UI_ID_##P##_PUZZLE_NODE,UI_ID_##P##_PUZZLE_CELL_0,UI_ID_##P##_PUZZLE_CELL_34=UI_ID_##P##_PUZZLE_CELL_0+34,UI_ID_##P##_PUZZLE_SLIDER,UI_ID_##P##_PUZZLE_CLOSE,UI_ID_##P##_WIRE_SLIDER,UI_ID_##P##_WIRE_TARGET,UI_ID_##P##_WIRE_NODE_0,UI_ID_##P##_WIRE_NODE_13=UI_ID_##P##_WIRE_NODE_0+13,UI_ID_##P##_WIRE_CLOSE,UI_ID_##P##_SYS_HEADER,UI_ID_##P##_SYS_DESC_0,UI_ID_##P##_SYS_DESC_10=UI_ID_##P##_SYS_DESC_0+10,UI_ID_##P##_SYS_VAL_0,UI_ID_##P##_SYS_VAL_10=UI_ID_##P##_SYS_VAL_0+10,UI_ID_##P##_SYS_CLOSE,\
    UI_ID_##P##_MINIGAMES_HEADER,UI_ID_##P##_MINIGAME_0,UI_ID_##P##_MINIGAME_8=UI_ID_##P##_MINIGAME_0+8,UI_ID_##P##_MINIGAMES_FOOTER,UI_ID_##P##_MINIGAME_VIEW,UI_ID_##P##_MINIGAME_BACK,UI_ID_##P##_MINIGAME_CLOSE,UI_ID_##P##_SEARCH_NAME,UI_ID_##P##_SEARCH_ICON_0,UI_ID_##P##_SEARCH_ICON_3=UI_ID_##P##_SEARCH_ICON_0+3,UI_ID_##P##_SEARCH_EMPTY,UI_ID_##P##_SEARCH_CLOSE,UI_ID_##P##_AUTOMAP_ZOOM_IN,UI_ID_##P##_AUTOMAP_ZOOM_OUT
typedef enum{UI_ID_NONE,
    /*Menu*/UI_ID_MENU_SINGLEPLAYER,UI_ID_MENU_MULTIPLAYER,UI_ID_MENU_OPTIONS,UI_ID_MENU_QUIT,UI_ID_MENU_CONTINUE,UI_ID_MENU_NEW_GAME,UI_ID_MENU_PLAY_INTRO,UI_ID_MENU_PLAY_CREDITS,UI_ID_MENU_BACK,UI_ID_MENU_TAB_GRAPHICS,UI_ID_MENU_TAB_INPUT,UI_ID_MENU_TAB_AUDIO_LANG,UI_ID_MENU_MODEL_DETAIL,UI_ID_MENU_FXAA,UI_ID_MENU_SHADOWS,UI_ID_MENU_SSR,UI_ID_MENU_VSYNC,UI_ID_MENU_FOV_SLIDER,UI_ID_MENU_GAMMA_SLIDER,UI_ID_MENU_RESOLUTION,UI_ID_MENU_FULLSCREEN,UI_ID_MENU_TOGGLE_MONITOR,UI_ID_MENU_RES_APPLY,UI_ID_MENU_MASTER_VOLUME_SLIDER,UI_ID_MENU_MUSIC_VOLUME_SLIDER,
    /*NewGame*/UI_ID_MENU_NAME_INPUT,UI_ID_MENU_DIFF_COMBAT,UI_ID_MENU_DIFF_PUZZLE,UI_ID_MENU_DIFF_MISSION,UI_ID_MENU_DIFF_CYBER,UI_ID_MENU_DIFF_CELL_0,UI_ID_MENU_DIFF_CELL_15=UI_ID_MENU_DIFF_CELL_0+15,UI_ID_MENU_START,
    /*Pause*/UI_ID_PAUSE_RESUME,UI_ID_PAUSE_LOAD,UI_ID_PAUSE_SAVE,UI_ID_PAUSE_OPTIONS,UI_ID_PAUSE_QUIT_TO_MENU,UI_ID_PAUSE_QUIT_GAME,
    /*HUD*/UI_ID_HUD_SHOOTMODE,UI_ID_HUD_HW_0,UI_ID_HUD_HW_7=UI_ID_HUD_HW_0+7,
    /*Center MFD*/UI_ID_CMFD_ADD_TO_INVENTORY,UI_ID_CMFD_TAB_MAIN,UI_ID_CMFD_TAB_HARDWARE,UI_ID_CMFD_TAB_GENERAL,UI_ID_CMFD_TAB_SOFTWARE,UI_ID_CMFD_WEAPON_HEADER,UI_ID_CMFD_SHOTS_HEADER,UI_ID_CMFD_WEAPON_ROW_0,UI_ID_CMFD_WEAPON_ROW_6=UI_ID_CMFD_WEAPON_ROW_0+6,UI_ID_CMFD_GREN_HEADER,UI_ID_CMFD_PATCH_HEADER,UI_ID_CMFD_GREN_USE_0,UI_ID_CMFD_GREN_USE_6=UI_ID_CMFD_GREN_USE_0+6,UI_ID_CMFD_GREN_ROW_0,UI_ID_CMFD_GREN_ROW_6=UI_ID_CMFD_GREN_ROW_0+6,
    UI_ID_CMFD_PATCH_USE_0,UI_ID_CMFD_PATCH_USE_6=UI_ID_CMFD_PATCH_USE_0+6,UI_ID_CMFD_PATCH_ROW_0,UI_ID_CMFD_PATCH_ROW_6=UI_ID_CMFD_PATCH_ROW_0+6,UI_ID_CMFD_HARDWARE_HEADER,UI_ID_CMFD_HARDWARE_ROW_0,UI_ID_CMFD_HARDWARE_ROW_11=UI_ID_CMFD_HARDWARE_ROW_0+11,UI_ID_CMFD_GENERAL_HEADER,UI_ID_CMFD_GENERAL_USE_0,UI_ID_CMFD_GENERAL_USE_13=UI_ID_CMFD_GENERAL_USE_0+13,UI_ID_CMFD_GENERAL_ROW_0,UI_ID_CMFD_GENERAL_ROW_13=UI_ID_CMFD_GENERAL_ROW_0+13,UI_ID_CMFD_SOFTWARE_HEADER,UI_ID_CMFD_SOFTWARE_ROW_0,UI_ID_CMFD_SOFTWARE_ROW_6=UI_ID_CMFD_SOFTWARE_ROW_0+6,
    UI_ID_CMFD_MEDIA_HEADER,UI_ID_CMFD_LOG_TABLE_0,UI_ID_CMFD_LOG_TABLE_9=UI_ID_CMFD_LOG_TABLE_0+9,UI_ID_CMFD_LOG_ENTRY_0,UI_ID_CMFD_LOG_ENTRY_14=UI_ID_CMFD_LOG_ENTRY_0+14,UI_ID_CMFD_LOG_TEXT,UI_ID_CMFD_LOG_MORE,UI_ID_CMFD_LOG_BACK,UI_ID_CMFD_EMAIL_ENTRY_0,UI_ID_CMFD_EMAIL_ENTRY_14=UI_ID_CMFD_EMAIL_ENTRY_0+14,UI_ID_CMFD_DATA_ENTRY_0,UI_ID_CMFD_DATA_ENTRY_12=UI_ID_CMFD_DATA_ENTRY_0+12,UI_ID_CMFD_NOTE_TOGGLE_0,UI_ID_CMFD_NOTE_TOGGLE_17=UI_ID_CMFD_NOTE_TOGGLE_0+17,
    /*Misc overlays*/UI_ID_CMFD_MISSION_TIMER,UI_ID_CMFD_CYBER_TIMER,UI_ID_CMFD_BIOMONITOR,UI_ID_CMFD_EDIT_PANEL,UI_ID_CMFD_EDIT_ROW_0,UI_ID_CMFD_EDIT_ROW_14=UI_ID_CMFD_EDIT_ROW_0+14,UI_ID_VMAIL_VIEWER,UI_ID_SENSA_CTR,UI_ID_SENSA_LH,UI_ID_SENSA_RH,
    UI_MFD_IDS(LMFD),UI_ID_SIDEMFD_PAD,UI_MFD_IDS(RMFD),UI_ID_COUNT,
} UIRegionID;
#define UI_MFD_STRIDE (UI_ID_RMFD_TAB_WEAPON-UI_ID_LMFD_TAB_WEAPON)
#define MID(rh,n) ((u32)UI_ID_LMFD_##n+((rh)?(u32)UI_MFD_STRIDE:0u))/*Mirror an LMFD id onto whichever side we are drawing*/
#define UIC(i) World.uiComponents[i]
#define UI_DBLCLICK 0.5
#define UI_KEY_BACKSPACE 10
#define UI_KEY_CLEAR 11
INLINE bool CursorIsOverBounds(float x0, float x1, float y0, float y1) { return World.cursorPos_x >= x0 && World.cursorPos_x <= x1 && World.cursorPos_y >= y0 && World.cursorPos_y <= y1;/*0,0=top left*/ }
INLINE void UIR(u32 id, i16 x, i16 y, i16 w, i16 h){if(!id)return; if(!World.uiComponents[id].initialized){World.uiComponents[id].min=(V2){(float)x,(float)y}; World.uiComponents[id].max=(V2){(float)(x+w),(float)(y+h)}; World.uiComponents[id].initialized=true;} World.uiComponents[id].active=true;}
INLINE void UIRImg(u32 id, i16 x, i16 y, i16 w, i16 h, u16 tex) { UIR(id,x,y,w,h); RenderUIImage(x,y,w,h,tex); }
INLINE i16 UITextW(const char* t, float sc, i16 maxW) { float w=MeasureLineAdvance(t,FONT_NORMAL)*sc; if (w<1.0f) w=1.0f; return (maxW>0 && w>(float)maxW) ? maxW : (i16)w; }
INLINE void UIRText(u32 id, i16 x, i16 y, u32 col, u8 font, float sc, i16 maxW, const char* t) { UIR(id,x,y,UITextW(t,sc,maxW),(i16)(22.0f*sc)); RenderTextL(x,y,col,font,sc,"%s",t); }
INLINE bool UIOver(u32 id) { return UIC(id).active && CursorIsOverBounds(UIC(id).min.x,UIC(id).max.x,UIC(id).min.y,UIC(id).max.y); }
void CreateUIElement(V2 min, V2 max, u32 idx) { if (!idx) return; UIC(idx).min=min; UIC(idx).max=max; UIC(idx).initialized=UIC(idx).active=true; }
static void UI_BeginFrame() { for (u32 i=1;i<UI_ID_COUNT;++i) UIC(i).active=false; }
/*Consume a click over a region. Returns 0 none, 1 LMB, 2 RMB, +4 when it was a double click of that same button on that same region.*/
static u8 UIClicked(u32 id) {
    if (!UIOver(id)) return 0; bool l=Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed, r=Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].pressed; if (!l && !r) return 0;
    Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed=Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].pressed=false; World.Sys_UI.mouseClickHeldOverGUI=World.uiIsBlocking=true;
    double* t=l?&World.uiComponents[id].lastLMB:&World.uiComponents[id].lastRMB; u8 dbl=(*t>0.0 && (World.pauseRelativeTime-*t)<=UI_DBLCLICK)?4u:0u; *t=dbl?0.0:World.pauseRelativeTime; return (u8)((l?1u:2u)|dbl);
}
                         /*mk3,bls,drt,flch, ion,rpir,pipe,magn,magp,pstl,plsm,rail,riot,skrp,sprq,stun*/
u16 wepIconTexIndices[16]={584,636,819,1067,1068,1494,1072,1069,1070,1071,1073,1165,1989,1990,1991,1992}; const char* elevFloorLabels[14] = {"R","1","2","3","4","5","6","7","8","9","G1","G2","G4","C"}; extern float reloadTime[16];
void MFD_NewGame() {
    World.Sys_UI=(SystemUI){.MFD_MediaTab=MM_LOG_TABLE,.MFD_ReaderView=MFD_READER_CONTENTS,.mfdSelected={1,1,1},.mfdReturnTab={1,1,1},.consumableClickRow=-1,.generalClickSlot=-1,.generalClickItem=-1,.generalClickCustom=U16_MAX,.applyButtonReferenceIndex=-1,.linkedElevatorDoor=U16_MAX,.tetheredPGP=U16_MAX,.tetheredPWP=U16_MAX,.tetheredSearchable=U16_MAX,.tetheredKeypadElevator=U16_MAX,.tetheredKeypadKeycode=U16_MAX,.keycodeHuns=-1,.keycodeTens=-1,.keycodeOnes=-1,.keycodeEntry=-1,.logReaderPage=-1,.mg_current=-1,.pw_selectedWire=-1};
}
void MFD_GeneralChanged() { World.Sys_UI.generalClickSlot=-1; }
void MFD_ResetGeneral() { World.Sys_UI.mfdGeneralItem=false; World.Sys_UI.mfdConsumable=0; World.Sys_UI.consumableClickRow=-1; MFD_GeneralChanged(); }
void MFD_ShowGeneralItem() { u8 side=World.Sys_UI.lastItemSideRH?2:1; World.Sys_UI.mfdConsumable=0; World.Sys_UI.mfdGeneralItem=true; World.Sys_UI.mfdItemReader[0]=World.Sys_UI.mfdItemReader[1]=false; if (side==2) World.Sys_UI.MFD_RightTab=2; else World.Sys_UI.MFD_LefTab=2; World.Sys_UI.mfdSelected[side]=2; }
void MFD_OpenSearch(bool isRH) { for (u8 side=0;side<2;++side) {u8 tab=side?World.Sys_UI.MFD_RightTab:World.Sys_UI.MFD_LefTab,view=side?World.Sys_UI.MFD_DataR:World.Sys_UI.MFD_DataL; if (!(tab==2 && World.Sys_UI.mfdItemReader[side]) && view!=5) { World.Sys_UI.mfdReturnTab[side+1]=tab; World.Sys_UI.mfdReturnView[side+1]=view;}} World.Sys_UI.MFD_DataL=World.Sys_UI.MFD_DataR=5; if (isRH) World.Sys_UI.MFD_RightTab=4; else World.Sys_UI.MFD_LefTab=4;}
void MFD_CloseSearch() {for (u8 side=0;side<2;++side) {u8* tab=side?&World.Sys_UI.MFD_RightTab:&World.Sys_UI.MFD_LefTab; u8* view=side?&World.Sys_UI.MFD_DataR:&World.Sys_UI.MFD_DataL; if (*view!=5) continue; *view=World.Sys_UI.mfdReturnView[side+1]; if (*view==5) *view=0; if (*tab==4) *tab=World.Sys_UI.mfdReturnTab[side+1];}}
/*Data tab sub-views that are driven by frobbed objects; these are the "open" object panels. view 5 (search) is handled separately by MFD_OpenSearch/MFD_CloseSearch.*/
INLINE bool SystemUIDataViewActive(u8 v) { return v==1||v==2||v==3||v==4||v==6||v==7||v==8||v==9; }
void MFD_OpenData(bool isRH,u8 code) { u8 side=isRH?3:1; u8 tab=isRH?World.Sys_UI.MFD_RightTab:World.Sys_UI.MFD_LefTab, view=isRH?World.Sys_UI.MFD_DataR:World.Sys_UI.MFD_DataL; if (tab!=4 && !SystemUIDataViewActive(view)) { World.Sys_UI.mfdReturnTab[side]=tab; World.Sys_UI.mfdReturnView[side]=view; } if (isRH) { World.Sys_UI.MFD_DataR=code; World.Sys_UI.MFD_RightTab=4; } else { World.Sys_UI.MFD_DataL=code; World.Sys_UI.MFD_LefTab=4; } }
void MFD_CloseDataSide(bool isRH) { u8 side=isRH?3:1; u8* tab=isRH?&World.Sys_UI.MFD_RightTab:&World.Sys_UI.MFD_LefTab; u8* view=isRH?&World.Sys_UI.MFD_DataR:&World.Sys_UI.MFD_DataL; if (!SystemUIDataViewActive(*view)) return; u8 rt=World.Sys_UI.mfdReturnTab[side]; *tab=rt; *view=(rt==4)?(((World.invP1.hasHardware&HW_SYS)?7:0)):0; }
INLINE void MFD_SelectTab(u8 panel,u8 tab,bool toggle) {
    MFD_GeneralChanged(); if (panel && tab==2) World.Sys_UI.lastItemSideRH=panel==2; u8* current=panel==0?&World.Sys_UI.MFD_CenterTab:panel==1?&World.Sys_UI.MFD_LefTab:&World.Sys_UI.MFD_RightTab; *current=toggle && *current==tab ? 0 : tab; World.Sys_UI.mfdSelected[panel]=tab; u8 view=panel==0?0:panel==1?World.Sys_UI.MFD_DataL:World.Sys_UI.MFD_DataR;
    if (!(panel && ((tab==2 && World.Sys_UI.mfdItemReader[panel-1]) || (tab==4 && view==5)))) { World.Sys_UI.mfdReturnTab[panel]=*current; if (view!=5) World.Sys_UI.mfdReturnView[panel]=view; } if (panel && tab==4 && view==5) World.Sys_UI.lastSearchSideRH=panel==2; play_wav(sounds[97],SfxVol(),(V3){0,0,0},false);
}

void WeaponFireStartWeaponDip(float t);
void WeaponSelectSlot(int slot){int wi=(int)World.invP1.weaponInventoryIndices[slot]; if(wi<0||wi>=MAX_ENTITIES)return; if((int)World.invP1.weaponCurrent==slot)return; if(World.invP1.reloadFinished>World.pauseRelativeTime)return; World.invP1.weaponCurrentPending=(i16)slot; World.invP1.weaponIndexPending=(i16)wi; int w=Get16WeaponIndexFromConstIndex(wi); WeaponFireStartWeaponDip((w>=0&&w<16) ? reloadTime[w] : 0.5f);}
__attribute__((noinline)) bool MenuEnter() { return !Cheats.consoleActive && (Sys_Input.keyStates[KEY_KP_ENTER].pressed || Sys_Input.keyStates[KEY_ENTER].pressed); }
__attribute__((noinline)) u8 UI_MenuInteractable(u32 id, i16 x, i16 y, float w, float h, bool* cursorOver, i8 this, bool sustained) {
    UIR(id,x,(i16)((float)y-h),(i16)w,(i16)h); bool cursorIsOver = CursorIsOverBounds(x, x + w, (float)y - h, (float)y); if (cursorIsOver && mouseMovementThisFrame && !resDropdownOpen) { currentMenuItem = this; if (cursorOver != NULL) {*cursorOver = cursorIsOver;} } if ((sustained ? Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT ].down : Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT ].pressed) && cursorIsOver) return 1u;
    if ((sustained ? Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].down : Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].pressed) && cursorIsOver) return 2u; return 0u;
}

__attribute__((noinline)) u8 UI_Button(u32 id, i16 x, i16 y, float w, float h, bool* cursorOver, i8 this) { return UI_MenuInteractable(id,x,y,w,h,cursorOver,this,false); }
__attribute__((noinline)) bool AnyLeftRightMouseDown() { return (Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].down || Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].down); }
bool UI_Slider(u32 id, i16 x, i16 y, i16 w, i16 h, i16 sliderPos, i16 xPosForLabel, u8 currentValue, u8* out, bool* sliderActive, u8 min, u8 max, u8 step, u8 mindex, u16 lingdex) {
    bool over=false,changed=false; *out = currentValue; RenderUIImage(x,y, w,h, 1079);/*Slider background*/ RenderUIImage(x + sliderPos,y, h,h,1078);/*Slider handle*/ if (UI_MenuInteractable(id,xPosForLabel,y,(float)((x + w) - xPosForLabel),(float)h,&over,mindex,true)) *sliderActive = true; if(*sliderActive && World.currentMouse_dx!=0){i32 new=(i32)currentValue+vmin(vmax(World.currentMouse_dx,-1),1); *out=(u8)vmin(vmax(new,min),max); if(*out!=currentValue){changed=true;}}
    if (!AnyLeftRightMouseDown()) { if (*sliderActive) { *sliderActive = false; SaveConfig(); } } if (MenuEnter() && currentMenuItem == mindex) {bool shiftHeld = Sys_Input.keyStates[KEY_LEFT_SHIFT].down || Sys_Input.keyStates[KEY_RIGHT_SHIFT].down; if (shiftHeld)*out=*out<=((min+step)-1) ? max : *out-step; else *out=*out >=((max-step)+1) ?  min : *out+step; changed=true;}
    over=over||currentMenuItem==mindex; RenderTextL(xPosForLabel,y,over ? T_YELLOW : T_GREEN,FONT_NORMAL,1.0f,"%s %u",Sys_Text.stringTable[lingdex],*out); return changed;
}

u8 UI_MenuButton(u32 id, i16 bX, i16 bY, u8 menuItem, i16 bW, i16 bH, i16 tX, i16 tY, const char* text, i16 pX, i16 pY){bool over=false; u8 retvalue=0u; retvalue=UI_Button(id,bX,bY,bW,bH,&over,menuItem); if(!retvalue)retvalue=(MenuEnter()&&currentMenuItem==menuItem); over=over||currentMenuItem==menuItem; RenderTextL(tX,tY,over ? T_STOPD_RED : T_RED_MENU,FONT_STOPD,1.5f,text); RenderUIImage(pX,pY,40,40,over ? 1029 : 1028);/*Menu pad*/ return retvalue;}
bool UI_Checkbox(u32 id, i16 x, i16 y, i8 mitem, u16 textIdx, bool currentlyOn){RenderUIImage(x,y,16,16,910);/*Checkbox background*/ bool over=false; bool changed=(UI_Button(id,x,y+16,210,16,&over,mitem)||(MenuEnter()&&currentMenuItem==mitem)); over=over||currentMenuItem==mitem; if(currentlyOn)RenderUIImage(x+2,y+2,12,12,912);/*Checkbox check*/ RenderTextL(x+20,y,over ? T_YELLOW : T_GREEN,FONT_NORMAL,1.0f,Sys_Text.stringTable[textIdx]); return changed;}
__attribute__((noinline)) void UI_HeaderText(i16 x, const char* text) { RenderTextL(x,50,T_GREEN_MENU_SHADOW,FONT_STOPD,1.75f,text); RenderTextL(x,46,T_GREEN_MENU_GLOW,FONT_STOPD,1.75f,text); RenderTextL(x,48,T_GREEN_MENU,FONT_STOPD,1.75f,text); }
void PlayMenuMusic(),mp3_clear();
__attribute__((noinline)) void MenuGoBack() {if(returnToPause){returnToPause=World.menuActive=false; World.paused=true; mp3_clear();} if(currentMenuPage==Mpg_Singleplayer||currentMenuPage==Mpg_Multiplayer||currentMenuPage==Mpg_Options)currentMenuPage=Mpg_FrontPage;/*News*/else if(currentMenuPage==Mpg_Load||currentMenuPage==Mpg_NewGame||currentMenuPage==Mpg_IntroVideo||currentMenuPage==Mpg_CreditsVideo)currentMenuPage=Mpg_Singleplayer;}
static void CreateShadowBuffers() { shadowMapSSBO=MakeSSBO(&shadowMapSSBO,5,(MAX_SHADOWMAPS * (SHADOW_MAP_SIZE * SHADOW_MAP_SIZE * 6U)) * sizeof(u32),NULL,GL_STATIC_DRAW); shadowMapsIndirectionID=MakeSSBO(&shadowMapsIndirectionID,6,LIGHT_COUNT * sizeof(u32),NULL,GL_STATIC_DRAW); shadowBuffersCreated=true; }
__attribute__((noinline)) void ChangeMenuPage(u8 pg) { currentMenuPage = pg; currentMenuItem = currentMenuTab = 0; resDropdownOpen = false; resHoverIdx = -1; }
static void MenuBackButton(i16 bgX, i16 bgY, i16 tX, i16 tY, i8 item) { RenderUIImage(bgX,bgY,84,36,1252);/*Back Button background*/ bool over=false; if (UI_Button(UI_ID_MENU_BACK,bgX,bgY+34,84,32,&over,item) || (MenuEnter() && currentMenuItem==item)) MenuGoBack(); over=over||currentMenuItem==item; RenderTextL(tX,tY,over ? T_STOPD_RED_HIGHLIGHT : T_RED_MENU,FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]); }
static void DiffDigits(i16 tx, i16 ty, u8 cur) { static const i16 dx4[4]={0,71,145,217}; for (u8 i=0;i<4;++i) RenderTextL(tx+dx4[i],ty,cur==i ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"%u",i); }
void RenderMenu() {
    if (currentMenuPage != Mpg_IntroVideo && currentMenuPage != Mpg_CreditsVideo && currentMenuPage != Mpg_Options) RenderUIImage(-417,-384, 2200,1536, 1026);/*Menu background*/
    if (currentMenuPage == Mpg_IntroVideo || currentMenuPage == Mpg_CreditsVideo) RenderUIImage(-417,-384, 2200,1536, 0);/*Video blackground*/
    if (currentMenuPage == Mpg_Options) RenderUIImage(-417,-384, 2200,1536, 1032);/*Menu background*/
    if (currentMenuPage == Mpg_FrontPage) {
        menuItemCount = 4; menuTabCount = 1; RenderUIImage(282,46, 800,128, 1031);/*Title CITADEL with strikethrough effect*/
        if (UI_MenuButton(UI_ID_MENU_SINGLEPLAYER,408,340, 0, 574,84, 304,188,/*"SINGLEPLAYER"*/Sys_Text.stringTable[719],413,276)) ChangeMenuPage(Mpg_Singleplayer);
        if (UI_MenuButton(UI_ID_MENU_MULTIPLAYER, 408,458, 1, 574,84, 304,268,/*"MULTIPLAYER"*/ Sys_Text.stringTable[720],413,396)) ChangeMenuPage(Mpg_Multiplayer);
        if (UI_MenuButton(UI_ID_MENU_OPTIONS,     408,582, 2, 574,84, 304,350,/*"OPTIONS"*/     Sys_Text.stringTable[721],413,520)) ChangeMenuPage(Mpg_Options);
        if (UI_MenuButton(UI_ID_MENU_QUIT,        408,702, 3, 574,84, 304,430,/*"QUIT"*/        Sys_Text.stringTable[722],413,638)) OS_Exit(0);
    } else if (currentMenuPage == Mpg_Singleplayer) {
        menuItemCount = 5; menuTabCount = 1; UI_HeaderText(250,/*"SINGLEPLAYER"*/Sys_Text.stringTable[719]);
        if (UI_MenuButton(UI_ID_MENU_CONTINUE,    408,340,0,574,84, 304,188,/*"CONTINUE"*/    Sys_Text.stringTable[723],413,276)) ChangeMenuPage(Mpg_Load);
        if (UI_MenuButton(UI_ID_MENU_NEW_GAME,    408,458,1,574,84, 304,268,/*"NEW GAME"*/    Sys_Text.stringTable[741],413,396)) ChangeMenuPage(Mpg_NewGame);
        if (UI_MenuButton(UI_ID_MENU_PLAY_INTRO,  408,582,2,574,84, 304,350,/*"PLAY INTRO"*/  Sys_Text.stringTable[742],413,520)) ChangeMenuPage(Mpg_IntroVideo);
        if (UI_MenuButton(UI_ID_MENU_PLAY_CREDITS,408,702,3,574,84, 304,430,/*"PLAY CREDITS"*/Sys_Text.stringTable[743],413,638)) ChangeMenuPage(Mpg_CreditsVideo);
        MenuBackButton(1060,724,1076,732,4);
    } else if (currentMenuPage == Mpg_Multiplayer) {
        menuItemCount = 1; menuTabCount = 1; UI_HeaderText(266,/*"MULTIPLAYER"*/Sys_Text.stringTable[720]); MenuBackButton(1060,724,1076,732,0);
    } else if (currentMenuPage == Mpg_Options) {
        menuTabCount = 3; UI_HeaderText(238,/*"CONFIGURATION"*/Sys_Text.stringTable[745]); if(currentMenuTab!=0){resDropdownOpen=false; resHoverIdx=-1;}
        if (currentMenuTab != 0) RenderUIImage(179,220, 1001,548, 1030);/*Config background*/
        if (currentMenuTab == 0) RenderUIImage(179,220, 1001,548, 1033);/*Config background graphics (empty alpha center)*/
        static const i16 cfgTabX[3]={190,354,520}; static const u16 cfgTabStr[3]={791,792,793}; static const i16 cfgTabTx[3]={200,366,530}; static const u32 cfgTabId[3]={UI_ID_MENU_TAB_GRAPHICS,UI_ID_MENU_TAB_INPUT,UI_ID_MENU_TAB_AUDIO_LANG};
        for (i8 t=2;t>=0;--t) { RenderUIImage(cfgTabX[t],196,160,30,currentMenuTab==t ? 920 : 921); if (UI_Button(cfgTabId[t],cfgTabX[t],196+30,160,30,NULL,t)) currentMenuTab=(u8)t; RenderTextL(cfgTabTx[t],202,currentMenuTab==t ? T_YELLOW : T_GREEN,FONT_NORMAL,1.0f,Sys_Text.stringTable[cfgTabStr[t]]); }
        if (currentMenuTab == 0) {
            bool overRes = false, overFull = false, overChgM = false, overApply = false; menuItemCount = 12;/*Graphics*/ u8 newVal;
            if(resDropdownOpen){ bool lmb=Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed, rmb=Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].pressed;
                if(lmb||rmb){ int rlh=resDropdownCount*16; bool inList=resDropdownCount>0 && World.cursorPos_x>=336 && World.cursorPos_x<=496 && World.cursorPos_y>=710-rlh && World.cursorPos_y<=710;
                    bool inHeader=World.cursorPos_x>=190 && World.cursorPos_x<=518 && World.cursorPos_y>710 && World.cursorPos_y<=726;
                    if(inList){ int row=(World.cursorPos_y-(710-rlh))/16; if(row>=0 && row<resDropdownCount){resSelectedIdx=row;} }
                    resDropdownOpen=false;
                    if(inList||inHeader){ Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed=Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].pressed=false; World.Sys_UI.mouseClickHeldOverGUI=World.uiIsBlocking=true; } } }
            if (UI_Checkbox(UI_ID_MENU_MODEL_DETAIL,200,500,0,Sys_Settings.ModelDetail ?/*High*/915 :/*No Detail Level Models*/914,Sys_Settings.ModelDetail)) { Sys_Settings.ModelDetail = Sys_Settings.ModelDetail ? 0u : 1u; SaveConfig(); }
            if (UI_Checkbox(UI_ID_MENU_FXAA,200,530,1,/*"FXAA"*/780,Sys_Settings.FXAA)) { Sys_Settings.FXAA = Sys_Settings.FXAA ? 0u : 1u; SaveConfig(); }
            if (UI_Checkbox(UI_ID_MENU_SHADOWS,200,560,2,Sys_Settings.Shadows ?/*Soft*/787 :/*No Shadows*/785,Sys_Settings.Shadows)) { Sys_Settings.Shadows = Sys_Settings.Shadows ? 0u : 1u; if (!shadowBuffersCreated) {CreateShadowBuffers();} SaveConfig(); }
            if (UI_Checkbox(UI_ID_MENU_SSR,200,590,3,/*SSR*/788,Sys_Settings.Reflections)) { Sys_Settings.Reflections = Sys_Settings.Reflections ? 0u : 1u; SaveConfig(); }
            if (UI_Checkbox(UI_ID_MENU_VSYNC,200,620,4,/*VSYNC*/1026,Sys_Settings.Vsync)) { Sys_Settings.Vsync = Sys_Settings.Vsync ? 0u : 1u; SetVSync(); SaveConfig(); }
            RenderTextL(310,620,T_GREEN,FONT_NORMAL,1.0f,"(FPS: %d)", globalframesPerLastSecond);/*Helper to see vsync take effect.*/
            if (UI_Slider(UI_ID_MENU_FOV_SLIDER,400,650,128,16,(((Sys_Settings.FOV - 45.0f) / 105.0f) * (128 - 16)),200,Sys_Settings.FOV,&newVal,&fovSliderActive,45,150,5,5,/*Field of View*/775)) { Sys_Settings.FOV = newVal; if (!AnyLeftRightMouseDown()) {SaveConfig();} }
            if (UI_Slider(UI_ID_MENU_GAMMA_SLIDER,400,680,128,16,((Sys_Settings.Brightness / 100.0f) * (128 - 16)),200,Sys_Settings.Brightness,&newVal,&gammaSliderActive,0,100,2,6,/*Gamma*/774)) { Sys_Settings.Brightness = newVal; if (!AnyLeftRightMouseDown()) {SaveConfig();} }
            if (UI_Button(UI_ID_MENU_RESOLUTION,190,726,328,16,&overRes,7) || (MenuEnter() && currentMenuItem == 7)) { DualLog("Resolution dropdown clicked! %u\n",globalframe); if(!resDropdownOpen){ GatherResolutionModes(); if(resDropdownCount>0){resDropdownOpen=true;} } else{resDropdownOpen=false;} currentMenuItem = 7; }
            overRes = overRes || currentMenuItem == 7; char resBuf[32];
            if (resDropdownCount > 0) sFormat(resBuf, sizeof(resBuf), "%ux%u",(u32)resModes[resSelectedIdx].w,(u32)resModes[resSelectedIdx].h); else sFormat(resBuf, sizeof(resBuf), "%ux%u",Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight);
            RenderUIImage(476,710,16,16,overRes ? 1119 : 1077); RenderTextL(200,710,overRes ? T_YELLOW : T_GREEN,FONT_NORMAL,1.0f,"RESOLUTION %s",resBuf);
            if(resDropdownOpen && resDropdownCount>0){ int rlh2=resDropdownCount*16, rTop=710-rlh2;
                resHoverIdx=(World.cursorPos_x>=336 && World.cursorPos_x<=496 && World.cursorPos_y>=rTop && World.cursorPos_y<rTop+rlh2) ? (World.cursorPos_y-rTop)/16 : -1;
                RenderUIImage(336,rTop,160,rlh2,922);/*datapanel behind list*/
                for(int ri=0;ri<resDropdownCount;++ri){char rb[32]; sFormat(rb,sizeof(rb),"%ux%u",(u32)resModes[ri].w,(u32)resModes[ri].h); if(ri==resSelectedIdx){RenderUIImage(336,rTop-2+ri*16,160,18,1087);} RenderTextL(366,rTop+ri*16,ri==resHoverIdx ? T_YELLOW : T_GREEN,FONT_NORMAL,0.9f,"%s",rb);}}
            else{resHoverIdx=-1;}
            RenderUIImage(200,740, 16,16, 910);/*Fullscreen checkbox background*/
            if (UI_Button(UI_ID_MENU_FULLSCREEN,200,756, 210,16, &overFull, 8) || (MenuEnter() && currentMenuItem == 8)) { Sys_Settings.Fullscreen = Sys_Settings.Fullscreen == 1u ? 0u : 1u; ChangeFullScreenWindowed(true); SaveConfig(); }
            overFull = overFull || currentMenuItem == 8; if (Sys_Settings.Fullscreen) RenderUIImage(202,742, 12,12, 912);/*Checkbox check*/
            RenderTextL(220,740,overFull ? T_YELLOW : T_GREEN,FONT_NORMAL,1.0f,/*"Fullscreen"*/Sys_Text.stringTable[773]); RenderUIImage(588,730, 210,30, 1079);/*Toggle monitor button background*/
            if (UI_Button(UI_ID_MENU_TOGGLE_MONITOR,588,760, 210,30, &overChgM, 9) || (MenuEnter() && currentMenuItem == 9)) { CycleToNextMonitor(); }
            overChgM = overChgM || currentMenuItem == 9; RenderTextL(602,735,overChgM ? T_YELLOW : T_GREEN,FONT_NORMAL,1.0f,/*"CHANGE MONITOR"*/Sys_Text.stringTable[1025]);
            RenderUIImage(588,690, 210,30, 1079);/*Apply button background*/
            if (UI_Button(UI_ID_MENU_RES_APPLY,588,720, 210,30, &overApply, 10) || (MenuEnter() && currentMenuItem == 10)) { ApplyStagedWindowedSize(); GatherResolutionModes(); currentMenuItem = 10; }
            overApply = overApply || currentMenuItem == 10; RenderTextL(602,695,overApply ? T_YELLOW : T_GREEN,FONT_NORMAL,1.0f,"APPLY");
        } else if (currentMenuTab == 1) { menuItemCount = 49;/*Input - TODO: rebind rows not ported yet, only the BACK item is live*/ }
        else {
            menuItemCount = 10;/*Audio / Lang*/ u8 newVal;
            if (UI_Slider(UI_ID_MENU_MASTER_VOLUME_SLIDER,426,240,128,16,((Sys_Settings.VolumeMaster / 100.0f) * (128 - 16)),200,Sys_Settings.VolumeMaster,&newVal,&masterVolumeSliderActive,0,100,5,0,/*Master Volume*/802)) { Sys_Settings.VolumeMaster = newVal; if (!AnyLeftRightMouseDown()) {SaveConfig();} }
            if (UI_Slider(UI_ID_MENU_MUSIC_VOLUME_SLIDER,426,270,128,16,((Sys_Settings.VolumeMusic / 100.0f) * (128 - 16)),200,Sys_Settings.VolumeMusic,&newVal,&musicVolumeSliderActive,0,100,5,1,/*Music Volume*/803)) { Sys_Settings.VolumeMusic = newVal; if (!AnyLeftRightMouseDown()) {SaveConfig();} }
        }
        MenuBackButton(1087,723,1103,731,(i8)(menuItemCount - 1));
    } else if (currentMenuPage == Mpg_Load || currentMenuPage == Mpg_Save) {
        menuItemCount = 9; menuTabCount = 1; bool isSave = currentMenuPage == Mpg_Save;/*TODO save/load slot rows are not ported; only BACK is live*/
        UI_HeaderText(isSave ? 284 : 340, isSave ?/*"SAVE GAME"*/Sys_Text.stringTable[769] :/*"LOAD"*/Sys_Text.stringTable[726]); RenderUIImage(400,214, 586,500, 1037);/*Load/Save table background*/ MenuBackButton(1060,724,1076,732,0);
    } else if (currentMenuPage == Mpg_NewGame) {
        menuItemCount = 7; menuTabCount = (currentMenuItem > 0 && currentMenuItem <= 16) ? 2 : 1; UI_HeaderText(290,/*"NEW GAME"*/Sys_Text.stringTable[741]);
        RenderUIImage(136,196,1088,558,1048);/*Newgame inset*/ RenderUIImage(136,196,1088,558,1049);/*Newgame background*/
        if (UI_MenuButton(UI_ID_MENU_NAME_INPUT,276,270,0,795,74, 226,146,/*"NAME:"*/Sys_Text.stringTable[746],299,214)) {/*Just for highlight*/ }
        enteringPlayerName = (currentMenuItem == 0);
        if (World.playerName[0] == '\0') RenderTextL(642,232,T_RED_MENU,FONT_STOPD,1.0f,/*"ENTER NAME..."*/Sys_Text.stringTable[749]); else RenderTextL(518,232,enteringPlayerName ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.0f,World.playerName);
        if (UI_MenuButton(UI_ID_MENU_DIFF_COMBAT, 174,377,1,496,95, 148,202,/*"COMBAT"*/Sys_Text.stringTable[750],185,299)) { World.diffCbt = World.diffCbt >= 3 ? 0 : World.diffCbt + 1; }
        if (UI_MenuButton(UI_ID_MENU_DIFF_MISSION,704,377,3,496,95, 510,202,/*"MISSION"*/Sys_Text.stringTable[751],726,299)) { World.diffMis = World.diffMis >= 3 ? 0 : World.diffMis + 1; }
        if (UI_MenuButton(UI_ID_MENU_DIFF_PUZZLE, 174,568,2,496,92, 149,330,/*"PUZZLE"*/Sys_Text.stringTable[753],185,490)) { World.diffPuz = World.diffPuz >= 3 ? 0 : World.diffPuz + 1; }
        if (UI_MenuButton(UI_ID_MENU_DIFF_CYBER,  704,568,4,496,92, 509,330,/*"CYBERSPACE"*/Sys_Text.stringTable[752],726,490)) { World.diffCyb = World.diffCyb >= 3 ? 0 : World.diffCyb + 1; }
        DiffDigits(162,270,World.diffCbt); DiffDigits(513,270,World.diffMis); DiffDigits(162,399,World.diffPuz); DiffDigits(730-217,399,World.diffCyb);
        {static const i16 dcx[4]={221,330,439,547}; for (u8 c=0;c<4;++c) for (u8 i=0;i<4;++i) { if (!UI_Button(UI_ID_MENU_DIFF_CELL_0+c*4+i,(i16)(dcx[i]+(c>=2?527:0)),(c&1)?651:460,82,79,NULL,(i8)(c+1))) continue; switch(c){case 0:World.diffCbt=i;break; case 1:World.diffPuz=i;break; case 2:World.diffMis=i;break; default:World.diffCyb=i;break;} currentMenuItem=(i8)(c+1); }}
        bool overStart = false; if (UI_Button(UI_ID_MENU_START,544,747, 282,68, &overStart, 5) || (MenuEnter() && currentMenuItem == 5)) GoIntoGame();
        overStart = overStart || currentMenuItem == 5; RenderTextL(400,464,overStart ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,/*"START"*/Sys_Text.stringTable[886]); MenuBackButton(1060,724,1076,732,6);
    } else if (currentMenuPage == Mpg_IntroVideo || currentMenuPage == Mpg_CreditsVideo) { menuItemCount = menuTabCount = 1; if (MenuEnter()) MenuGoBack(); }
    if (menuTabCount <= currentMenuTab) currentMenuTab = 0;
    if (menuItemCount <= currentMenuItem) currentMenuItem = 0;
    static const i8 ngSwap[7] = {0,3,4,1,2,6,5};
    if (Sys_Input.keyStates[KEY_RIGHT].pressed || Sys_Input.keyStates[KEY_LEFT].pressed) { int dir = Sys_Input.keyStates[KEY_RIGHT].pressed ? 1 : -1; currentMenuTab = (currentMenuTab + menuTabCount + dir) % menuTabCount; if (currentMenuPage == Mpg_NewGame && currentMenuItem < 7) {currentMenuItem=ngSwap[currentMenuItem];} }
}

void RenderPausedUI() {
    menuItemCount = 6; menuTabCount = 1; static const i16 pY[6]={330,390,450,510,570,714},pH[6]={52,52,60,60,60,42},pTx[6]={610,630,635,599,546,572},pTy[6]={306,364,422,480,538,690}; static const u16 pStr[6]={725,726,727,721,728,729};
    RenderUIImage(519,276,328,300,1025);/*Pause Menu background*/ RenderUIImage(519,276,328,300,1080);/*Pause Menu background outline*/ RenderUIImage(519,672,328,42,1252);/*Pause Quit Game background*/ RenderTextL(610,210,T_STOPD_RED_PAUSETITLE,FONT_STOPD,1.0f,/*"PAUSED"*/Sys_Text.stringTable[724]);
    for (u8 i=0;i<6;++i) { bool over=false; if (UI_Button(UI_ID_PAUSE_RESUME+i,522,pY[i],322,pH[i],&over,(i8)i) || (MenuEnter() && currentMenuItem==i)) { if (i==0) World.paused=false; else if (i==5) OS_Exit(0);
            else { currentMenuPage = i==1?Mpg_Load:i==2?Mpg_Save:i==3?Mpg_Options:Mpg_FrontPage; PlayMenuMusic(); World.menuActive=true; returnToPause=(i!=4); } }
        over=over||currentMenuItem==i; RenderTextL(pTx[i],pTy[i],over ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.0f,Sys_Text.stringTable[pStr[i]]); }
}

void GetWeaponAmmoText(int slot,char* buf,size_t bufSize) {
    buf[0] = '\0'; int wepIdx = World.invP1.weaponInventoryIndices[slot]; bool alt = World.invP1.wepLoadedWithAlternate[slot]; float heat = World.invP1.currentEnergyWeaponHeat[slot];
    u8 mag = alt ? World.invP1.currentMagazineAmount2[slot] : World.invP1.currentMagazineAmount[slot];
    switch(wepIdx) {
        case 343: if (alt){sFormat(buf,bufSize,"%upn | %umg, %upn",mag,World.invP1.wepAmmo[0],World.invP1.wepAmmoSecondary[0]);}else{sFormat(buf,bufSize,"%umg | %umg, %upn",mag,World.invP1.wepAmmo[0],World.invP1.wepAmmoSecondary[0]);} break;/*MK3 Assault Rifle*/
        case 344: case 347: case 353: case 357: case 358: scpy_to_a_from_b(buf,heat > 80.0f ? Sys_Text.stringTable[14] : Sys_Text.stringTable[15],bufSize); break;/*Energy weapons*/
        case 345: if (alt){sFormat(buf,bufSize,"%utq | %und, %utq",mag,World.invP1.wepAmmo[2],World.invP1.wepAmmoSecondary[2]);}else{sFormat(buf,bufSize,"%und | %und, %utq",mag,World.invP1.wepAmmo[2],World.invP1.wepAmmoSecondary[2]);} break;/*SV-23 Dartgun*/
        case 346: if (alt){sFormat(buf,bufSize,"%usp | %uhn, %usp",mag,World.invP1.wepAmmo[3],World.invP1.wepAmmoSecondary[3]);}else{sFormat(buf,bufSize,"%uhn | %uhn, %usp",mag,World.invP1.wepAmmo[3],World.invP1.wepAmmoSecondary[3]);} break;/*AM-27 Flechette*/
        case 348: case 349: break;/*Laser Rapier / Lead Pipe: no ammo*/
        case 350: if (alt){sFormat(buf,bufSize,"%usg | %uhw, %usg",mag,World.invP1.wepAmmo[7],World.invP1.wepAmmoSecondary[7]);}else{sFormat(buf,bufSize,"%uhw | %uhw, %usg",mag,World.invP1.wepAmmo[7],World.invP1.wepAmmoSecondary[7]);} break;/*Magnum 2100*/
        case 351: if (alt){sFormat(buf,bufSize,"%usu | %ucr, %usu",mag,World.invP1.wepAmmo[8],World.invP1.wepAmmoSecondary[8]);}else{sFormat(buf,bufSize,"%ucr | %ucr, %usu",mag,World.invP1.wepAmmo[8],World.invP1.wepAmmoSecondary[8]);} break;/*SB-20 Magpulse*/
        case 352: if (alt){sFormat(buf,bufSize,"%utf | %ust, %utf",mag,World.invP1.wepAmmo[9],World.invP1.wepAmmoSecondary[9]);}else{sFormat(buf,bufSize,"%ust | %ust, %utf",mag,World.invP1.wepAmmo[9],World.invP1.wepAmmoSecondary[9]);} break;/*ML-41 Pistol*/
        case 354: sFormat(buf,bufSize,"%url | %url",World.invP1.currentMagazineAmount[slot],World.invP1.wepAmmo[11]); break;/*MM-76 Railgun*/
        case 355: sFormat(buf,bufSize,"%urb | %urb",World.invP1.currentMagazineAmount[slot],World.invP1.wepAmmo[12]); break;/*DC-05 Riotgun*/
        case 356: if (alt){sFormat(buf,bufSize,"%ulg | %usm, %ulg",mag,World.invP1.wepAmmo[13],World.invP1.wepAmmoSecondary[13]);}else{sFormat(buf,bufSize,"%usm | %usm, %ulg",mag,World.invP1.wepAmmo[13],World.invP1.wepAmmoSecondary[13]);} break;/*RF-07 Skorpion*/
        default: break;
    }
}

void TickBar(bool isEnergy) {
    RenderUIImage(isEnergy ? 1333 : 1332,isEnergy ? 36 : 2,32,32,isEnergy ? 939 : 956);/*Indicator*/ int p1H=isEnergy ? World.invP1.energy : World.instances[PLAYER1].health; if(p1H>255){p1H=255;} i16 tY=isEnergy ? 35 : 4;
    for (int i=7;i>=0;--i) if(i==7/*Always render at least 1 tick*/||p1H>(7-i)*11){RenderUIImage(1050-(i*16),tY,32,32,964);/*Tick Red*/} for (int i=7;i>=0;--i) if(p1H>88+(7-i)*11){RenderUIImage(1178-(i*16),tY,32,32,963);/*Tick Orange*/} for (int i=7;i>=0;--i) if(p1H>176+(7-i)*11){RenderUIImage(1306-(i*16),tY,32,32,962);/*Tick Green*/}
}

INLINE int HwActiveTexIndex(int active, int version, int off, int v1, int v2, int v3, int v4) { if (!active) return off; if (v4 >= 0 && version >= 4) return v4; if (version >= 3) return v3; if (version == 2) return v2; return v1; }
/*eng: 0 needs energy, 1 needs energy only when hwVersSetting==0, 2 needs energy only when hwVersSetting>=1, 3 not a toggle (e-reader)*/
typedef struct { u32 bit; u8 idx; i16 x,y; u16 t[5]; u8 sOn,sOff,eng; } HwBtn;
static const HwBtn hwBtns[8]={{HW_BIO,HW_BIO_IDX,0,180,{989,991,992,992,992},78,78,1},{HW_SNS,HW_SNS_IDX,0,240,{1009,1011,1012,1013,1013},93,82,0},{HW_LAN,HW_LAN_IDX,0,300,{1004,1006,1007,1008,1008},78,78,0},{HW_SHD,HW_SHD_IDX,0,360,{1014,1015,1016,1017,1018},96,95,0},
                              {HW_INF,HW_INF_IDX,1326,180,{998,999,999,999,999},98,82,0},{HW_ERD,HW_ERD_IDX,1326,240,{996,997,997,997,997},0,0,3},{HW_BST,HW_BST_IDX,1326,300,{993,994,995,995,995},78,78,2},{HW_JET,HW_JET_IDX,1326,360,{1000,1001,1002,1003,1003},78,78,0}};
void HardwareButtons() {
    u32 hw=World.invP1.hasHardware;
    for(u8 i=0;i<8;++i){const HwBtn* b=&hwBtns[i]; if (!(hw&b->bit))continue; u16 tex; if (b->eng==3) tex=((World.inventoryMode && UIOver(UI_ID_HUD_HW_0+i) && AnyLeftRightMouseDown()) || ((World.invP1.hasNewEmail || World.invP1.hasNewLogs) && ((int)World.pauseRelativeTime & 1))) ? 997 : 996; else tex=(u16)HwActiveTexIndex((World.invP1.hardwareIsActive & b->bit)!=0,World.invP1.hwVers[b->idx],b->t[0],b->t[1],b->t[2],b->t[3],b->t[4]); UIRImg(UI_ID_HUD_HW_0+i,b->x,b->y,40,40,tex); }
}

static void HwToggle(u8 i) {
    const HwBtn* b=&hwBtns[i]; bool noEng=World.invP1.energy<=0.0f, on=(World.invP1.hardwareIsActive & b->bit)!=0;
    if (b->eng==3) { MFD_ResetGeneral(); World.Sys_UI.MFD_CenterTab=5; World.Sys_UI.MFD_LefTab=2; World.Sys_UI.mfdItemReader[0]=true; World.Sys_UI.MFD_ReaderView=MFD_READER_CONTENTS; World.Sys_UI.MFD_MediaTab=World.Sys_UI.lastMultiMediaTabOpened;
        if (World.Sys_UI.MFD_MediaTab>MM_NOTES || (World.Sys_UI.MFD_MediaTab==MM_NOTES && !World.diffMis)) World.Sys_UI.MFD_MediaTab=MM_LOG_TABLE; play_wav(sounds[97],SfxVol(),(V3){0,0,0},false); return; }
    if (noEng && (b->eng==0 || (b->eng==1 && World.invP1.hwVersSetting[b->idx]==0) || (b->eng==2 && World.invP1.hwVersSetting[b->idx]>=1))) { CenterStatusPrint("%s",Sys_Text.stringTable[314]); return; }
    play_wav(sounds[on ? b->sOff : b->sOn],SfxVol(),(V3){0.0f,0.0f,0.0f},false); World.invP1.hardwareIsActive ^= b->bit; if (b->bit==HW_BIO && on && !Cheats.showFPS) BioMonitorClearGraphs();
}

extern V3 queuedLevelPos; extern u8 queuedLevelToLoad;
void ActualChangeAmmoType(void); void OverloadButtonAction(void); void PlayLog(int logIndex); void CheckForUnreadLogs(void); void UseTargets(u16,u16);
static const char* mgName[9]={"Ping","15","Wing 0","Botbounce","Eel Zapper","Road","TriopToe","Corp Conq","Chess"};
static void SysUIDataClose(bool rh) { MFD_CloseDataSide(rh); World.Sys_UI.objectInUsePos=(V3){999.0f,999.0f,999.0f}; World.Sys_UI.usingObject=false; }
static void KeycodeSetDigit(int n) { if (World.Sys_UI.keycodeOnes < 0) { World.Sys_UI.keycodeOnes=(i8)n; World.Sys_UI.keycodeEntry=World.Sys_UI.keycodeOnes; } else if (World.Sys_UI.keycodeTens < 0) { World.Sys_UI.keycodeTens=World.Sys_UI.keycodeOnes; World.Sys_UI.keycodeOnes=(i8)n; World.Sys_UI.keycodeEntry=World.Sys_UI.keycodeOnes+World.Sys_UI.keycodeTens*10; } else if (World.Sys_UI.keycodeHuns < 0) { World.Sys_UI.keycodeHuns=World.Sys_UI.keycodeTens; World.Sys_UI.keycodeTens=World.Sys_UI.keycodeOnes; World.Sys_UI.keycodeOnes=(i8)n; World.Sys_UI.keycodeEntry=World.Sys_UI.keycodeOnes+World.Sys_UI.keycodeTens*10+World.Sys_UI.keycodeHuns*100; } else { World.Sys_UI.keycodeHuns=World.Sys_UI.keycodeTens; World.Sys_UI.keycodeTens=World.Sys_UI.keycodeOnes; World.Sys_UI.keycodeOnes=(i8)n; World.Sys_UI.keycodeEntry=World.Sys_UI.keycodeOnes+World.Sys_UI.keycodeTens*10+World.Sys_UI.keycodeHuns*100; } }
static void KeycodeKeypress(int k) { SystemUI* s=&World.Sys_UI; if (!s->keycodeValid) return; if (s->keycodeSolved) return; play_wav(sounds[39],SfxVol(),(V3){0.0f,0.0f,0.0f},false);
    if (k>=0&&k<=9) KeycodeSetDigit(k);
    else if (k==UI_KEY_BACKSPACE) { if (s->keycodeHuns>=0) { s->keycodeOnes=s->keycodeTens; s->keycodeTens=s->keycodeHuns; s->keycodeHuns=-1; s->keycodeEntry=s->keycodeOnes+s->keycodeTens*10; return; } else if (s->keycodeTens>=0) { s->keycodeOnes=s->keycodeTens; s->keycodeTens=-1; s->keycodeEntry=s->keycodeOnes; return; } else if (s->keycodeOnes>=0) { s->keycodeOnes=-1; s->keycodeEntry=-1; return; } else return;/*empty backspace leaves the preloaded easy-difficulty value intact*/ }
    else if (k==UI_KEY_CLEAR) { s->keycodeHuns=s->keycodeTens=s->keycodeOnes=-1; s->keycodeEntry=-1; }
    if (s->keycodeEntry==s->keycodeValue) { if (s->keycodeHuns>=0) play_wav(sounds[46],SfxVol(),(V3){0.0f,0.0f,0.0f},false);/*code accepted*/ u16 pad=s->tetheredKeypadKeycode; if (pad>=INSTS_1ST_IDX && pad<World.instCount) { UseTargets(pad,World.instances[pad].targetIdx); if (World.instances[pad].messageLingdex) CenterStatusPrint("%s",Sys_Text.stringTable[World.instances[pad].messageLingdex]); } s->keycodeSolved=true; }
    else if (s->keycodeHuns>=0) play_wav(sounds[43],SfxVol(),(V3){0.0f,0.0f,0.0f},false);/*code not accepted*/
}

void UI_KeycodeKey(bool rh,int k) { World.Sys_UI.mouseClickHeldOverGUI=true; (void)rh; if (k<0||k>11) return; KeycodeKeypress(k); }
void UI_KeycodeClose(bool rh) { World.Sys_UI.mouseClickHeldOverGUI=true; World.Sys_UI.tetheredKeypadKeycode=U16_MAX; World.Sys_UI.keycodeValid=false; World.Sys_UI.keycodeHuns=World.Sys_UI.keycodeTens=World.Sys_UI.keycodeOnes=-1; World.Sys_UI.keycodeEntry=-1; World.Sys_UI.keycodeValue=0; World.Sys_UI.keycodeSolved=false; SysUIDataClose(rh); }
void UI_ElevFloorClick(bool rh,int btn) { World.Sys_UI.mouseClickHeldOverGUI=true; (void)rh; if (btn<0||btn>7) return;
    if (World.Sys_UI.linkedElevatorDoor==U16_MAX) { CenterStatusPrint("%s",Sys_Text.stringTable[6]); return; }/*Too far away from that.*/
    Entity* door=&World.instances[World.Sys_UI.linkedElevatorDoor]; bool doorClosed=door->doorOpen==DoorState_Closed; float dist=V3_Dist(World.Sys_UI.objectInUsePos,World.position[PLAYER1]);
    if (dist > 2.0f/*tether dist*/ && !doorClosed) { CenterStatusPrint("%s",Sys_Text.stringTable[6]); return; } if (!doorClosed) { CenterStatusPrint("%s",Sys_Text.stringTable[7]); return; }/*Door not closed.*/
    if (!World.Sys_UI.buttonsEnabled[btn] || World.Sys_UI.buttonsDarkened[btn]) { CenterStatusPrint("%s",Sys_Text.stringTable[8]); return; }/*Floor not accessible.*/
    u16 floor=World.Sys_UI.elevButtonSpawnIdx[btn]; queuedLevelPos=(floor!=U16_MAX && floor<World.instCount && (World.instances[floor].entflags&EF_ACTIVE)) ? World.position[floor] : (V3){0.0f,0.0f,0.0f}; queuedLevelToLoad=World.Sys_UI.elevButtonLevelIdx[btn];
}

void UI_ElevClose(bool rh) { World.Sys_UI.mouseClickHeldOverGUI=true; World.Sys_UI.tetheredKeypadElevator=U16_MAX; World.Sys_UI.linkedElevatorDoor=U16_MAX; World.Sys_UI.elevCurrentFloor=0; SysUIDataClose(rh); }
void UI_AudioLogClick(bool rh) { World.Sys_UI.mouseClickHeldOverGUI=true; World.Sys_UI.lastLogSideRH=rh; if (!(World.invP1.hasHardware&HW_ERD)) return;
    if (World.Sys_UI.logActive) { World.Sys_UI.logActive=false; World.Sys_UI.audPaused=true; CenterStatusPrint("%s",Sys_Text.stringTable[1019]); return; }/*Log playback stopped.*/
    if (World.Sys_UI.logReferenceIndex<(u16)LOGCNT && World.invP1.hasLog[World.Sys_UI.logReferenceIndex] && audioLogs[World.Sys_UI.logReferenceIndex] && audioLogs[World.Sys_UI.logReferenceIndex][0]) { PlayLog((int)World.Sys_UI.logReferenceIndex); World.Sys_UI.logActive=true; }
}

static void PGToggleLine(int i,int dx,int dy) { int w=World.Sys_UI.pg_width,h=World.Sys_UI.pg_height,r=i/w+dy,c=i%w+dx; while (r>=0&&r<h&&c>=0&&c<w) { int ci=r*w+c; if (World.Sys_UI.pg_type[ci]!=PuzzleCellType_Standard) break; World.Sys_UI.pg_cell[ci]=!World.Sys_UI.pg_cell[ci]; r+=dy; c+=dx; } }
static void PGFlipperPawn(int i) { World.Sys_UI.pg_cell[i]=!World.Sys_UI.pg_cell[i]; }
static void PGFlipperKing(int i) { int w=World.Sys_UI.pg_width,h=World.Sys_UI.pg_height,r=i/w,c=i%w; World.Sys_UI.pg_cell[i]=!World.Sys_UI.pg_cell[i]; for (i8 dr=-1;dr<=1;++dr) for (i8 dc=-1;dc<=1;++dc) { if (!dr&&!dc) continue; int rr=r+dr,cc=c+dc; if (rr<0||rr>=h||cc<0||cc>=w) continue; int ci=rr*w+cc; if (World.Sys_UI.pg_type[ci]==PuzzleCellType_Standard) World.Sys_UI.pg_cell[ci]=!World.Sys_UI.pg_cell[ci]; } }
static void PGFlipperRook(int i) { World.Sys_UI.pg_cell[i]=!World.Sys_UI.pg_cell[i]; PGToggleLine(i,1,0); PGToggleLine(i,-1,0); PGToggleLine(i,0,1); PGToggleLine(i,0,-1); }
static void PGFlipperBishop(int i) { World.Sys_UI.pg_cell[i]=!World.Sys_UI.pg_cell[i]; PGToggleLine(i,1,1); PGToggleLine(i,-1,1); PGToggleLine(i,1,-1); PGToggleLine(i,-1,-1); }
static void PGFlipperQueen(int i) { World.Sys_UI.pg_cell[i]=!World.Sys_UI.pg_cell[i]; PGToggleLine(i,1,0); PGToggleLine(i,-1,0); PGToggleLine(i,0,1); PGToggleLine(i,0,-1); PGToggleLine(i,1,1); PGToggleLine(i,-1,1); PGToggleLine(i,1,-1); PGToggleLine(i,-1,-1); }
static void PGFlipperKnight(int i) { int w=World.Sys_UI.pg_width,h=World.Sys_UI.pg_height,r=i/w,c=i%w; World.Sys_UI.pg_cell[i]=!World.Sys_UI.pg_cell[i]; static const i8 dr[8]={-2,-1,1,2,2,1,-1,-2},dc[8]={1,2,2,1,-1,-2,-2,-1}; for (u8 n=0;n<8;++n) { int rr=r+dr[n],cc=c+dc[n]; if (rr<0||rr>=h||cc<0||cc>=w) continue; int ci=rr*w+cc; if (World.Sys_UI.pg_type[ci]==PuzzleCellType_Standard) World.Sys_UI.pg_cell[ci]=!World.Sys_UI.pg_cell[ci]; } }
static void PGEvalPuzzle(void) { SystemUI* s=&World.Sys_UI; int w=(int)s->pg_width,h=(int)s->pg_height; if (w<=0||h<=0) return; int n=w*h; if (n>35) n=35;
    for (int i=0;i<n;++i) { s->pg_powered[i]=false; s->pg_checked[i]=false; } int src=(int)s->pg_source; if (src<0||src>=n) return; s->pg_powered[src]=true;
    int q[35],qt=0,qh=0; q[qt++]=src; while (qh<qt) { int idx=q[qh++],r=idx/w,c=idx%w;
        for (i8 dr=-1;dr<=1;++dr) for (i8 dc=-1;dc<=1;++dc) { if (dr&&dc) continue; if (!dr&&!dc) continue; int rr=r+dr,cc=c+dc; if (rr<0||rr>=h||cc<0||cc>=w) continue; int ni=rr*w+cc; if (s->pg_checked[ni]) continue; int pc=0;
            for (i8 dr2=-1;dr2<=1;++dr2) for (i8 dc2=-1;dc2<=1;++dc2) { if (!dr2&&!dc2) continue; int cr=r+dr2,cl=c+dc2; if (cr<0||cr>=h||cl<0||cl>=w) continue; if (s->pg_powered[cr*w+cl]) ++pc; }
            bool power=false; if (s->pg_type[ni]==PuzzleCellType_Standard && s->pg_cell[ni] && pc>0) power=true; else if (s->pg_type[ni]==PuzzleCellType_And && pc>1) power=true; else if (s->pg_type[ni]==PuzzleCellType_Bypass && pc>0) power=true;
            if (power) { s->pg_powered[ni]=true; s->pg_checked[ni]=true; q[qt++]=ni; } } }
    int out=(int)s->pg_output; s->pg_solved=(out>=0 && out<n && s->pg_powered[out]); float cnt=0.0f; for (int i=0;i<n;++i) if (s->pg_powered[i]) cnt+=1.0f; s->pg_progress=cnt/(float)n;
}
void UI_PuzzleGridCell(bool rh,int cell) { World.Sys_UI.mouseClickHeldOverGUI=true; (void)rh; if (World.Sys_UI.tetheredPGP==U16_MAX) return; if (World.Sys_UI.pg_solved) return;/*TODO Genius patch: while active, show hover move-preview highlights (Citadel PuzzleGrid.cs:134).*/
    int n=(int)(World.Sys_UI.pg_width*World.Sys_UI.pg_height); if (n<=0||cell<0||cell>=n) return; if (World.Sys_UI.pg_type[cell]!=PuzzleCellType_Standard) return;
    PuzzleGridType gt=(World.diffPuz==1)?(PuzzleGridType)PuzzleGridType_King:(PuzzleGridType)World.Sys_UI.pg_gridType;
    switch (gt) { case PuzzleGridType_King:PGFlipperKing(cell); break; case PuzzleGridType_Queen:PGFlipperQueen(cell); break; case PuzzleGridType_Knight:PGFlipperKnight(cell); break; case PuzzleGridType_Rook:PGFlipperRook(cell); break; case PuzzleGridType_Bishop:PGFlipperBishop(cell); break; default:PGFlipperPawn(cell); break; }
    PGEvalPuzzle(); if (World.Sys_UI.pg_solved) { u16 pg=World.Sys_UI.tetheredPGP; if (pg>=INSTS_1ST_IDX && pg<World.instCount) { UseTargets(pg,World.instances[pg].targetIdx); if (World.instances[pg].messageLingdex) CenterStatusPrint("%s",Sys_Text.stringTable[World.instances[pg].messageLingdex]); } }
}
void UI_PuzzleGridSlide(bool rh,float f) { (void)rh; (void)f; /*Unity's puzzle progress handle is a server-authoritative display; the fill is driven by puzzle state, not the drag.*/ }
void UI_PuzzleGridClose(bool rh) { World.Sys_UI.mouseClickHeldOverGUI=true; World.Sys_UI.tetheredPGP=U16_MAX; World.Sys_UI.pg_solved=false; World.Sys_UI.pg_width=World.Sys_UI.pg_height=0; SysUIDataClose(rh); }
/*---- Wire puzzle (Unity PuzzleWire.cs). curL/curR hold the wire id occupying each column on that side; selectedWire is the chosen column (0..6) of the held wire, selectedWireRH is the side it was grabbed from. Clicking the same side re-grabs the wire there, clicking the other side swaps that column in. TODO Genius patch: while active on hard difficulty, reveal all wire colors (Citadel PuzzleWire.cs:147, wirePuzzle.geniusActive).--*/
static i8 PWFindCol(const i8* arr,int wire) { for (i8 c=0;c<7;++c) if ((int)arr[c]==wire) return c; return -1; }
static void PWClickEnd(int spot,bool colRH) { if (spot<0||spot>6) return; i8* col=colRH?World.Sys_UI.pw_curR:World.Sys_UI.pw_curL;
    if (World.Sys_UI.pw_selectedWire < 0) { if (col[spot]>=0) { World.Sys_UI.pw_selectedWire=(i8)spot; World.Sys_UI.pw_selectedWireRH=colRH; } return; }
    if (World.Sys_UI.pw_selectedWireRH == colRH) { World.Sys_UI.pw_selectedWire=(col[spot]>=0)?(i8)spot:(i8)-1; return; }/*same side: change which wire is held*/
    i8 sel=World.Sys_UI.pw_selectedWire; i8 temp=col[spot]; col[spot]=col[sel]; col[sel]=temp; World.Sys_UI.pw_selectedWire=-1;
}
static void PWEval(void) { float match=0.0f; for (i8 w=0;w<7;++w) { i8 l=PWFindCol(World.Sys_UI.pw_curL,w),r=PWFindCol(World.Sys_UI.pw_curR,w); if (World.Sys_UI.pw_wireOn[w] && l>=0 && r>=0 && l==World.Sys_UI.pw_tgtL[w] && r==World.Sys_UI.pw_tgtR[w]) match+=0.19f; } if (World.diffPuz==1) match+=0.19f;/*Easy draws a free 0.19, mirroring the Unity bonus*/ World.Sys_UI.pw_temp=match; World.Sys_UI.pw_solved=(match>0.92f); }
void UI_WireNodeClick(bool rh,int node) { World.Sys_UI.mouseClickHeldOverGUI=true; (void)rh; if (World.Sys_UI.tetheredPWP==U16_MAX) return; if (World.Sys_UI.pw_solved) return; if (node<0||node>13) return;
    PWClickEnd(node%7, node>=7); PWEval();
    if (World.Sys_UI.pw_solved) { u16 pw=World.Sys_UI.tetheredPWP; if (pw>=INSTS_1ST_IDX && pw<World.instCount) { UseTargets(pw,World.instances[pw].targetIdx); if (World.instances[pw].messageLingdex) CenterStatusPrint("%s",Sys_Text.stringTable[World.instances[pw].messageLingdex]); } }
}
void UI_WireSlide(bool rh,float f) { (void)rh; (void)f; /*Unity's wire level handle is a server-authoritative display, dragging is not a real action.*/ }
void UI_WireClose(bool rh) { World.Sys_UI.mouseClickHeldOverGUI=true; World.Sys_UI.tetheredPWP=U16_MAX; World.Sys_UI.pw_solved=false; World.Sys_UI.pw_selectedWire=-1; SysUIDataClose(rh); }
void UI_SysAnalyzerClose(bool rh) { World.Sys_UI.mouseClickHeldOverGUI=true; MFD_CloseDataSide(rh); }
/*---- Minigames (Unity MFDManager OpenMinigames/MinigameStart_*) ----*/
void UI_MinigameStart(bool rh,int game) { World.Sys_UI.mouseClickHeldOverGUI=true; World.Sys_UI.lastMinigameSideRH=rh; if (game<0||game>8) return; if (!World.invP1.hasMinigame) return; World.Sys_UI.mg_current=(i8)game; World.Sys_UI.mg_running[rh?1:0]=true; CenterStatusPrint("%s %s",Sys_Text.stringTable[1021],mgName[game]); play_wav(sounds[97],SfxVol(),(V3){0.0f,0.0f,0.0f},false); }
void UI_MinigameInput(bool rh,int x,int y) { (void)rh;(void)x;(void)y; /*The Funpack gameplay engines are not ported yet; a click inside the running view would land here and is only consumed.*/ World.Sys_UI.mouseClickHeldOverGUI=true; }
void UI_MinigameBack(bool rh) { World.Sys_UI.mouseClickHeldOverGUI=true; if (World.Sys_UI.mg_current<0) return; World.Sys_UI.mg_current=-1; World.Sys_UI.mg_running[rh?1:0]=false; play_wav(sounds[97],SfxVol(),(V3){0.0f,0.0f,0.0f},false); }
void UI_MinigameClose(bool rh) { World.Sys_UI.mouseClickHeldOverGUI=true; if (World.Sys_UI.mg_running[rh?1:0]) World.Sys_UI.mg_running[rh?1:0]=false; World.Sys_UI.mg_current=-1; MFD_CloseDataSide(rh); }
/*---- E-reader log/email/data navigation (Unity MFDManager + ReaderView) ----*/
static void UIOpenEntryAndRead(int idx,bool playable) { if (idx<0||idx>=LOGCNT) return; World.Sys_UI.logReferenceIndex=(u16)idx; World.Sys_UI.MFD_ReaderView=MFD_READER_TEXT; World.Sys_UI.logReaderPage=0; World.invP1.readLog[idx]=true;
    if (playable && (World.invP1.hasHardware&HW_ERD) && audioLogs[idx] && audioLogs[idx][0]) PlayLog(idx);
    CheckForUnreadLogs(); if (!World.invP1.hasNewEmail && !World.invP1.hasNewLogs && !World.invP1.hasNewNotes) World.Sys_UI.highlightStatus[MM_EMAIL_TABLE]=World.Sys_UI.highlightStatus[MM_LOG_TABLE]=World.Sys_UI.highlightStatus[MM_DATA_TABLE]=false; }
void UI_LogTableClick(int level) { World.Sys_UI.mouseClickHeldOverGUI=true; if (level<0||level>9) return; World.Sys_UI.logFolderCount=0; World.Sys_UI.MFD_ReaderView=MFD_READER_FOLDER;
    for (int i=0;i<LOGCNT && World.Sys_UI.logFolderCount<16;++i) if (World.invP1.hasLog[i] && Sys_Text.audioLogType[i]==AudioLogType_Normal && Sys_Text.audioLogLevelFound[i]==(u8)level) { World.Sys_UI.logFolderList[World.Sys_UI.logFolderCount]=(i16)i; World.Sys_UI.logFolderCount++; }
    play_wav(sounds[97],SfxVol(),(V3){0.0f,0.0f,0.0f},false); }
void UI_LogEntryClick(int entry) { World.Sys_UI.mouseClickHeldOverGUI=true; if (entry<0||entry>=(int)World.Sys_UI.logFolderCount) return; UIOpenEntryAndRead((int)World.Sys_UI.logFolderList[entry],true); }
static int UIEmailRefAt(int pos) { int c=0; for (int i=0;i<LOGCNT;++i) if (World.invP1.hasLog[i] && Sys_Text.audioLogType[i]==AudioLogType_Email) { if (c==pos) return i; ++c; } return -1; }
static int UIDataRefAt(int pos) { int c=0; for (int i=0;i<LOGCNT;++i) if (World.invP1.hasLog[i] && Sys_Text.audioLogType[i]==AudioLogType_Papers) { if (c==pos) return i; ++c; } return -1; }
void UI_EmailEntryClick(int entry) { World.Sys_UI.mouseClickHeldOverGUI=true; if (entry<0||entry>14) return; int idx=UIEmailRefAt(entry); if (idx<0) return; UIOpenEntryAndRead(idx,false); }/*emails have no voice track*/
void UI_DataEntryClick(int entry) { World.Sys_UI.mouseClickHeldOverGUI=true; if (entry<0||entry>12) return; int idx=UIDataRefAt(entry); if (idx<0) return; UIOpenEntryAndRead(idx,false); }
void UI_LogMore(void) { World.Sys_UI.mouseClickHeldOverGUI=true; if (World.Sys_UI.MFD_ReaderView!=MFD_READER_TEXT) return; World.Sys_UI.logReaderPage++; }
void UI_LogBack(void) { World.Sys_UI.mouseClickHeldOverGUI=true; if (World.Sys_UI.MFD_ReaderView==MFD_READER_TEXT) World.Sys_UI.MFD_ReaderView=MFD_READER_FOLDER; else if (World.Sys_UI.MFD_ReaderView==MFD_READER_FOLDER) World.Sys_UI.MFD_ReaderView=MFD_READER_CONTENTS; else World.Sys_UI.MFD_ReaderView=MFD_READER_CONTENTS; }
void UI_NoteToggleClick(int note) { World.Sys_UI.mouseClickHeldOverGUI=true; if (note<0||note>17) return; if (!World.questNotesActive[note]) return; World.questNotesChecked[note]=!World.questNotesChecked[note]; play_wav(sounds[97],SfxVol(),(V3){0.0f,0.0f,0.0f},false); }
void UI_HardwareRowClick(int idx) { World.Sys_UI.mouseClickHeldOverGUI=true; if (idx<0||idx>=HW_COUNT) return; World.invP1.hardwareInvCurrent=idx; play_wav(sounds[97],SfxVol(),(V3){0.0f,0.0f,0.0f},false); }
void UI_SoftwareRowClick(int idx) { World.Sys_UI.mouseClickHeldOverGUI=true; if (idx<0||idx>6) return;
    if (idx==SW_GAMES) { if (World.invP1.hasMinigame) { World.Sys_UI.mg_current=-1; MFD_OpenData(false,9); play_wav(sounds[97],SfxVol(),(V3){0.0f,0.0f,0.0f},false); } return; }
    if (idx<=2) World.invP1.cyberItemIndex=(i8)idx;/*row 0 Turbo, 1 Decoy, 2 Recall arm the cyber item*/
    play_wav(sounds[97],SfxVol(),(V3){0.0f,0.0f,0.0f},false); }
void UI_WeaponIconClick(bool rh) { World.Sys_UI.mouseClickHeldOverGUI=true; World.Sys_UI.lastWeaponSideRH=rh; if (CurrentWeaponUsesEnergy()) OverloadButtonAction(); else ActualChangeAmmoType(); }
void UI_AutomapClick(bool rh,int action) { World.Sys_UI.mouseClickHeldOverGUI=true; World.Sys_UI.lastAutomapSideRH=rh; u8 side=rh?1:0; u8* z=&World.automapZoom;/*single shared zoom: both MFDs blit the same automap texture, so either side's buttons drive it*/
    if (World.invP1.hwVers[HW_NAV_IDX]<2) { CenterStatusPrint("%s",Sys_Text.stringTable[465]); return; }/*Map hardware version doesn't support zoom.*/
    if (action==UI_AUTOMAP_ZOOM_IN) { if (*z==0) { CenterStatusPrint("%s",Sys_Text.stringTable[317]); return; } (*z)--; }
    else if (action==UI_AUTOMAP_ZOOM_OUT) { if (*z==2) { CenterStatusPrint("%s",Sys_Text.stringTable[316]); return; } (*z)++; }
    else if (action==UI_AUTOMAP_FULL) { World.Sys_UI.fullMapOpen[side]=!World.Sys_UI.fullMapOpen[side]; }
    else if (action==UI_AUTOMAP_SIDE) { World.Sys_UI.autoSide[side]=!World.Sys_UI.autoSide[side]; }
    else return; play_wav(sounds[97],SfxVol(),(V3){0.0f,0.0f,0.0f},false); }

void AddItemToInventory(int index, int custIdx); void ResetHeldItem();
static const u16 vmailStartFrames[6]={1579,1645,1713,1784,1864,1931}; static const u16 vmailEndFrames[6]={1644,1712,1783,1863,1930,1988}; double avgCPUt[AVG_CPU_TAPS]={0}; int avgCPUt_idx = 0;
void AppendTextWarning(i32 sidx, i32 sidx2, i32 sidx3, i32 col, i32 id) { World.Sys_UI.tWrnTextIdx[id]=sidx; World.Sys_UI.tWrnTextIdx2[id]=sidx2; World.Sys_UI.tWrnTextIdx3[id]=sidx3; World.Sys_UI.tWrnFinished[id]=World.Sys_UI.tWrnFinished[id] < World.pauseRelativeTime ? World.pauseRelativeTime + 0.1f : World.Sys_UI.tWrnFinished[id] + 0.1f; World.Sys_UI.tWrnColorIdx[id] = col; }
extern double game_actual_start_time; extern u16 editModeTestEntityDefinition;
/*Edit-mode info panel text editing (console-style entry)*/
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
bool EditPanelPointerHover(void){if(!EditSelIsActive()||!World.inventoryMode)return false;for(int i=0;i<EF_LAST;++i)if(UIOver(UI_ID_CMFD_EDIT_ROW_0+i))return true;return false;}
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

/*The whole point of the region table: one loop decides whether the pointer is over any live UI, which winput.c turns into World.uiIsBlocking so use/fire are suppressed.*/
bool UIInteractions(void) {
    if (!World.inventoryMode) return false;
    if (World.menuActive || World.paused || World.creditsActive || Cheats.consoleActive) return true;
    if (Cheats.noHUD || World.Sys_UI.vmailActive) return false;
    for (u32 i=UI_ID_NONE+1;i<UI_ID_COUNT;++i) if (UIOver(i)) return true;
    return false;
}
bool InventoryPointerHover(void) { return UIInteractions(); }

int GeneralInvItem(int slot);
bool GeneralInvCanUse(int slot),GeneralInvCanVaporize(int slot),GeneralInvTake(int slot);
void GeneralInvClick(int slot,int custom),GeneralInvApply(int slot,int custom),VaporizeClick(void);
bool InventoryHasAccessCard(AccCardType card);
const char* AccessCardCodeForType(AccCardType card);
u16 GetItemFrobTexture(u16 index);
static const i16 generalStartRowY=586,generalColX[2]={372,712},generalRowY[7]={generalStartRowY,generalStartRowY+1*TXT_H,generalStartRowY+2*TXT_H,generalStartRowY+3*TXT_H,generalStartRowY+4*TXT_H,generalStartRowY+5*TXT_H,generalStartRowY+6*TXT_H};
static const char* GeneralInvLabel(int slot) { int item=GeneralInvItem(slot); return item<0?"":Sys_Text.stringTable[slot?item+326:597]; }
static const u8 grenadeItems[7]={7,9,13,8,11,12,10},patchSlots[7]={6,5,0,3,4,2,1};
void UseGrenade(int),PatchUse(int);
int ConsumableSlot(bool patch,int row) { return row<0 || row>=7?-1:patch?patchSlots[row]:row; }
int ConsumableItem(bool patch,int row) { int slot=ConsumableSlot(patch,row); return slot<0?-1:patch?14+slot:grenadeItems[row]; }
static u8 ConsumableCount(bool patch,int row) { int slot=ConsumableSlot(patch,row); return slot<0?0:patch?World.invP1.patchCounts[slot]:World.invP1.grenAmmo[slot]; }
void ConsumableSelect(bool patch,int row) {
    if (!ConsumableCount(patch,row)) return;
    if (patch) World.invP1.patchCur=(u8)ConsumableSlot(true,row); else World.invP1.grenCur=(u8)row;
    MFD_ShowGeneralItem(); World.Sys_UI.mfdGeneralItem=false; World.Sys_UI.mfdConsumable=patch?2:1;
}
void ConsumableUse(bool patch,int row) {
    if (!ConsumableCount(patch,row) || World.invP1.holdingObject) return;
    ConsumableSelect(patch,row); if (patch) PatchUse(ConsumableSlot(true,row)); else UseGrenade(ConsumableItem(false,row)+307);
    World.Sys_UI.consumableClickRow=-1;
}
static int ConsumableSelectedRow(void) {
    if (World.Sys_UI.mfdConsumable==1) return World.invP1.grenCur<7?World.invP1.grenCur:-1;
    if (World.Sys_UI.mfdConsumable==2) for (int row=0;row<7;++row) if (patchSlots[row]==World.invP1.patchCur) return row;
    return -1;
}
void ConsumableSetTimer(float fraction) {
    int row=ConsumableSelectedRow(); if (World.Sys_UI.mfdConsumable!=1 || row<5 || !ConsumableCount(false,row)) return;
    float min=row==5?2.0f:4.0f,value=min+vclamp(fraction,0.0f,1.0f)*(60.0f-min);
    if (row==5) World.invP1.nitroTimeSetting=value; else World.invP1.earthShakerTimeSetting=value;
}

void RenderConsumableItem(bool isRH) {
    int row=ConsumableSelectedRow(); bool patch=World.Sys_UI.mfdConsumable==2; if (row<0 || !ConsumableCount(patch,row)) return;
    i16 dx=isRH?1059:0; int item=ConsumableItem(patch,row); const char* text=Sys_Text.stringTable[item+326];
    UIRText(MID(isRH,ITEM_NAME),dx+28,540,T_YELLOW,FONT_NORMAL,0.8f,260,text);
    u16 tex=GetItemFrobTexture(item+307); if (tex<MAX_TXRS) UIRImg(MID(isRH,ITEM_ICON),dx+112,560,80,64,tex);
    UIRImg(MID(isRH,ITEM_USE),dx+72,691,160,40,1087); RenderTextL(dx+72,691,T_GREEN_MENU,FONT_NORMAL,0.8f,"%s",Sys_Text.stringTable[736]);
    if (!patch && row>=5) {
        float min=row==5?2.0f:4.0f,value=row==5?World.invP1.nitroTimeSetting:World.invP1.earthShakerTimeSetting;
        UIR(MID(isRH,ITEM_TIMER_VALUE),dx+112,626,58,20); RenderTextL(dx+112,626,T_GREEN,FONT_NORMAL,0.8f,"%.1f",(double)value);
        UIRImg(MID(isRH,ITEM_TIMER_SLIDER),dx+40,650,224,24,1087); RenderUIImage(dx+40+(i16)(204.0f*vclamp((value-min)/(60.0f-min),0.0f,1.0f)),652,20,20,1086);
    }
}

void RenderGeneralItem(bool isRH) {
    if (World.Sys_UI.mfdConsumable) { RenderConsumableItem(isRH); return; }
    if (!World.Sys_UI.mfdGeneralItem) return;
    int slot=World.invP1.generalInvCurrent,item=GeneralInvItem(slot); if (item<0) return;
    i16 dx=isRH?1059:0; UIRText(MID(isRH,ITEM_NAME),dx+28,540,T_YELLOW,FONT_NORMAL,0.8f,260,GeneralInvLabel(slot));
    u16 tex=GetItemFrobTexture((u16)(item+307));
    if (item>=92 && item<=94) { static const u8 heads[19]={37,11,32,1,7,9,10,12,13,14,15,17,25,27,28,31,33,35,36}; u16 custom=World.invP1.generalInvCustIdx[slot]; tex=(u16)(1272+heads[custom<19?custom:0]); }
    if (tex<MAX_TXRS) UIRImg(MID(isRH,ITEM_ICON),dx+112,560,80,64,tex);
    if (!slot) {
        i16 x=dx+36,y=638;
        for (int card=ACC_Std;card<=ACC_Per5;++card) if (InventoryHasAccessCard((AccCardType)card)) {
            const char* code=AccessCardCodeForType((AccCardType)card); float w=MeasureLineAdvance(code,FONT_NORMAL)*0.8f;
            if (x+w>dx+280) { x=dx+36; y+=20; }
            RenderTextL(x,y,T_YELLOW,FONT_NORMAL,0.8,"%s",code); x+=(i16)(w+8);
        }
        UIR(MID(isRH,ITEM_ACCESS_CARDS),dx+36,638,244,(i16)((y+20)-638));/*One region over the whole wrapped card block; the codes are display only*/
    } else if (GeneralInvCanUse(slot) || GeneralInvCanVaporize(slot)) {
        bool use=GeneralInvCanUse(slot); i16 y=use?691:628;
        UIRImg(use?MID(isRH,ITEM_USE):MID(isRH,ITEM_VAPORIZE),dx+72,y,160,40,1087); RenderTextL(dx+72,y,T_GREEN_MENU,FONT_NORMAL,0.8,"%s",Sys_Text.stringTable[use?736:883]);
    }
}
void UpdateSearchTether(void),CloseSearch(void); bool SearchTakeSlot(u8 slot);
void RenderSearchFX(void) {
    for (int side=0;side<2;++side) {
        if (!World.Sys_UI.searchFXActive[side]) continue;
        double elapsed = World.pauseRelativeTime - World.Sys_UI.searchFXStartTime[side];
        if (elapsed >= 1.0) { World.Sys_UI.searchFXActive[side] = false; continue; }
        if (Cheats.noHUD || (side?World.Sys_UI.MFD_RightTab:World.Sys_UI.MFD_LefTab)!=4 || (side?World.Sys_UI.MFD_DataR:World.Sys_UI.MFD_DataL)!=5 || World.Sys_UI.tetheredSearchable==U16_MAX) continue;
        float t = (float)elapsed / 1.0f; if (t > 1.0f) t = 1.0f;
        float p = t < 0.4f ? t / 0.4f : 1.0f;/*scale up first 0.4s, hold*/
        float ep = 1.0f - (1.0f - p) * (1.0f - p) * (1.0f - p);/*ease-out cubic*/
        float scale = 40.0f + ep * (263.0f - 40.0f), w = scale, h = scale * 240.0f / 263.0f;
        float sx = World.Sys_UI.searchFXCursorX[side], sy = World.Sys_UI.searchFXCursorY[side], ex = (side ? 1210.5f : 151.5f), ey = 648.0f;
        float cx = sx + (ex - sx) * ep, cy = sy + (ey - sy) * ep;
        RenderUIImage((i16)(cx - w * 0.5f),(i16)(cy - h * 0.5f),(i16)(w + 0.5f),(i16)(h + 0.5f),1074);
    }
}

void RenderSearch(bool isRH) {
    u16 s=World.Sys_UI.tetheredSearchable;
    if (s<INSTS_1ST_IDX || s>=World.instCount || !(World.instances[s].entflags&EF_ACTIVE) || !World.instances[s].srchInUse) return;
    Entity* e=&World.instances[s]; i16 dx=isRH?1059:0; u16 label=0;
    switch (e->index) {
        case 464: label=895; break; case 531: label=896; break; case 530: label=898; break;
        case 465: case 466: case 467: case 468: case 469: case 470: case 471: label=897; break;
        case 472: case 473: case 474: case 475: case 476: label=899; break;
    }
    if (label) RenderTextL(dx+34,536,T_YELLOW,FONT_NORMAL,0.8f,Sys_Text.stringTable[label]);
    else if (IdxIsNPC(e->index)) RenderTextL(dx+34,536,T_YELLOW,FONT_NORMAL,0.8f,npcTable[e->index-419].name);
    bool any=false;
    for (u8 slot=0;slot<4;++slot) {
        i16 item=e->contents[slot]; if (item<0 || item>110) continue;
        any=true; u16 tex=GetItemFrobTexture((u16)(item+307));
        if (tex<MAX_TXRS) UIRImg(MID(isRH,SEARCH_ICON_0)+slot,(i16)(dx+84+90*(slot&1)),(i16)(584+90*(slot>>1)),64,64,tex);
    }
    if (!any) RenderTextL(dx+24,633,T_YELLOW,FONT_NORMAL,0.8f,Sys_Text.stringTable[891]);
    UIRImg(MID(isRH,SEARCH_CLOSE),dx+259,isRH?528:534,29,29,899); RenderTextL(dx+259,isRH?531:534,T_STOPD_RED,FONT_NORMAL,0.8,"X");
}

static const i16 elevBtnY[4]={578,620,663,705}; static const char* elevBtnLabel[8]={"R","1","2","3","6","7","8","9"};/*TODO drive off elevFloorLabels[] + the linked elevator's floor set instead of this fixed strip*/
static const i16 keyBtnX[3]={86,127,169},keyBtnY[4]={577,620,663,706}; static const u8 keyBtnK[12]={1,2,3,4,5,6,7,8,9,UI_KEY_BACKSPACE,0,UI_KEY_CLEAR}; static const char* keyBtnLabel[12]={"1","2","3","4","5","6","7","8","9","-","0","C"};
static const i16 puzCellX[7]={51,80,109,138,166,195,224},puzCellY[5]={565,594,622,651,680};
static const i16 wireNodeY[7]={566,594,623,651,679,707,736};
static const i16 mgX[9]={32,32,32,32,156,156,156,156,32},mgY[9]={540,575,610,646,540,575,610,646,681};/*mgName is defined with the UI row handlers*/
static const struct { const char* d,*v; i16 y; } sysRows[11]={{"Current level security:","100%",547},{"Mining laser status:","Charging",566},{"Lifepod status:","Disabled",585},{"Station shield status:","Off",605},{"Reactor status:","Normal",624},{"Processor nodes:","99",643},{"Main Program:","Downloading to earth",662},{"Alpha Grove status:","normal",681},{"Beta Grove status:","normal",701},{"Gamma Grove status:","launched",720},{"Delta Grove status:","launched",739}};
void SideMFD(bool isRH) { // 320x240
    int wep16 = Get16WeaponIndexFromConstIndex(World.invP1.weaponIndex), tab = isRH ? World.Sys_UI.MFD_RightTab : World.Sys_UI.MFD_LefTab; u8 selected=tab?tab:World.Sys_UI.mfdSelected[isRH?2:1];
    for (u8 i=0;i<4;++i) UIRImg(MID(isRH,TAB_WEAPON)+i,isRH ? 1350 : -TAB_THICK,(i16)(520+56*i),32,40,selected==i+1 ? 1024 : 1022);/*Weapon/Item/Automap/Data side tab buttons*/
    if ((World.invP1.hardwareIsActive & HW_SNS) && World.invP1.hwVers[HW_SNS_IDX] > 1) {
        /*TODO Sensaround Plane*/
        UIR(isRH ? UI_ID_SENSA_RH : UI_ID_SENSA_LH,isRH ? UI_H-TAB_THICK-MFD_SPACING-SIDE_MFD_W: TAB_THICK+MFD_SPACING,isRH ? UI_H-TAB_THICK-MFD_SPACING: TAB_THICK+MFD_SPACING+SIDE_MFD_W,UI_H-TAB_THICK-TXT_PAD-SIDE_MFD_H,UI_H-TAB_THICK-TXT_PAD);
    } else {
        if (tab == 1) {/*WeaponTab: WepNameText, WepIcon, ClipBox, EnergyHeatTicks, ReloadButtons, EnergySlider*/
            i16 slot=World.invP1.weaponCurrent; if (slot>=0 && slot<7) { i32 widx=World.invP1.weaponInventoryIndices[slot]; if (widx >= 0) {UIRText(MID(isRH,WEAPON_NAME),isRH ? UI_W-TAB_THICK-MFD_SPACING-SIDE_MFD_W+TXT_PAD : TAB_THICK+MFD_SPACING+TXT_PAD,520,T_RED,FONT_NORMAL,0.8f,270,Sys_Text.stringTable[ItemStringIdx((i32)widx)]);/*Weapon Name*/ if (wep16 >=0 && wep16 < 16) UIRImg(MID(isRH,WEAPON_ICON),isRH ? 1207 : 24,548,270,100,wepIconTexIndices[wep16]);/*WepIcon*/} }
        } else if (tab == 2 && World.Sys_UI.mfdItemReader[isRH]) {
            i16 x=isRH?1080:TAB_THICK+MFD_SPACING; static const u16 labels[4]={42,39,43,885};
            UIRText(MID(isRH,MEDIA_HEADER),x+6,540,T_YELLOW,FONT_NORMAL,0.8f,260,Sys_Text.stringTable[349]);
            for (u8 section=0;section<4;++section) { if (section==MM_NOTES && !World.diffMis) continue;
                bool sectionSelected=World.Sys_UI.MFD_MediaTab==section,unread=World.Sys_UI.highlightStatus[section];
                UIRImg(MID(isRH,MEDIA_TAB_0)+section,(i16)(x+65*section),718,65,40,sectionSelected||unread?1087:1086); RenderTextL(x+65*section,718,sectionSelected?T_GREEN_MENU:T_GREEN_MENU_SHADOW,FONT_NORMAL,0.8,"%s",Sys_Text.stringTable[labels[section]]);
            }
        } else if (tab == 3) {/*AutomapTab: AutomapMask, Overlays, PlayerIcon, ZoomIn/Out/Full/Side Buttons - TODO not ported*/ }
        else if (tab==2 && !World.Sys_UI.mfdItemReader[isRH?1:0]) RenderGeneralItem(isRH);
        else if(tab==4){/*DataTab*/
            u8 data = isRH ? World.Sys_UI.MFD_DataR : World.Sys_UI.MFD_DataL; i16 dx = isRH ? 1059 : 0;
            if (data==8) {/*Blocked by SHODAN level security*/ UIRImg(UI_ID_NONE,31+dx,535,227,209,1110); UIRText(MID(isRH,BLOCKED_SECURITY_TEXT),45+dx,542,T_YELLOW,FONT_NORMAL,0.8f,0,890<1100?Sys_Text.stringTable[890]:"Blocked by SHODAN level Security."); }
            if (data==1) {/*Elevator*/
                UIRImg(MID(isRH,ELEV_FLOOR_INDICATOR),132+dx,531,32,32,929);/*CurrentFloorIndicator*/
                for (u8 b=0;b<2;++b) { i16 ex=(i16)(86+78*b+dx); RenderUIImage(ex,578,45,168,0);/*ButtonBank QUAD:builtin-knob*/
                    for (u8 i=0;i<4;++i) { u8 f=(u8)(b*4+i); i16 ey=elevBtnY[i]; UIRImg(MID(isRH,ELEV_BUTTON_0)+f,ex,ey,45,39,(i==0||i==3)?2133:2135);/*keypad_end / keypad_mid*/
                        RenderUIImage((i16)(ex+2),(i16)(ey+4),40,34,2134);/*keypad_inner_on*/ RenderTextL((i16)(ex+3),(i16)(ey+2),T_GREEN,FONT_NORMAL,0.8,"%s",elevBtnLabel[f]); } }
                UIRImg(MID(isRH,ELEV_CLOSE),246+dx,528,29,29,899); RenderTextL(246+dx,528,T_STOPD_RED,FONT_NORMAL,0.8,"X");
            }
            if (data==2) {/*Keycode pad*/
                for (u8 i=0;i<12;++i) { i16 kx=(i16)(keyBtnX[i%3]+dx),ky=keyBtnY[i/3]; UIRImg(MID(isRH,KEYCODE_0)+keyBtnK[i],kx,ky,42,38,2133);/*keypad_end*/
                    RenderUIImage((i16)(kx+2),(i16)(ky+3),38,35,2134);/*keypad_inner_on*/ RenderTextL((i16)(kx-6),(i16)(ky-5),T_GREEN,FONT_NORMAL,0.8,"%s",keyBtnLabel[i]); }
                for (u8 d=0;d<3;++d) UIRImg(MID(isRH,KEYCODE_DIGIT_0)+d,(i16)(90+41*d+dx),526,32,32,2132);/*Hundreds/Tens/Ones elnum_null*/
                UIRImg(MID(isRH,KEYCODE_CLOSE),255+dx,525,29,29,899); RenderTextL(255+dx,525,T_STOPD_RED,FONT_NORMAL,0.8,"X");
            }
            if (data==5) RenderSearch(isRH);
            if (data==6) {/*AudioLog*/
                UIRImg(MID(isRH,AUDIOLOG_IMAGE),20+dx,528,263,240,1272);/*LogImage*/
                UIRText(MID(isRH,AUDIOLOG_NAME),29+dx,540,T_YELLOW,FONT_NORMAL,0.8f,0,"HACKER IS AWESOME"); UIRText(MID(isRH,AUDIOLOG_SENDER),29+dx,557,T_YELLOW,FONT_NORMAL,0.8f,0,"Sender: SHODAN");
                UIR(MID(isRH,AUDIOLOG_SUBJECT),29+dx,701,(i16)(MeasureLineAdvance("Subject:",FONT_NORMAL)*0.8f),(i16)(3*22.0f*0.8f)); RenderTextL(29+dx,701,T_YELLOW,FONT_NORMAL,0.8,"Subject:\n\nif only i had a sparq beam then all the world would be right");
            }
            if (data==3) {/*GridPuzzle*/
                RenderUIImage(42+dx,555,221,163,2139);/*OuterColorBorder gridcontainer_gray*/ RenderUIImage(46+dx,558,214,157,2138);/*ContainerEdge gridcontainer*/
                UIRImg(MID(isRH,PUZZLE_NODE_SOURCE),25+dx,621,29,29,2141); UIRImg(MID(isRH,PUZZLE_NODE),250+dx,621,29,29,2140);
                for (u8 c=0;c<35;++c) { i16 cx=(i16)(puzCellX[c%7]+dx),cy=puzCellY[c/7]; UIRImg(MID(isRH,PUZZLE_CELL_0)+c,cx,cy,29,29,2137);/*grid1_base*/ RenderTextL(cx,cy,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.8,"?"); RenderUIImage(cx,cy,29,29,2136);/*geniusgrid_highlight*/ }
                RenderUIImage(42+dx,720,221,26,2139);/*ProgressContainer*/ RenderUIImage(45+dx,726,225,13,0);/*Background QUAD:builtin-knob*/ RenderUIImage(48+dx,726,6,13,2142);/*Fill puzzlesliderwire*/
                UIRImg(MID(isRH,PUZZLE_SLIDER),45+dx,720,225,26,1078);/*Handle - the whole bar is the drag region*/
                UIRImg(MID(isRH,PUZZLE_CLOSE),259+dx,527,29,29,899); RenderTextL(259+dx,527,T_STOPD_RED,FONT_NORMAL,0.8,"X");
            }
            if (data==4) {/*WirePuzzle*/
                RenderUIImage(82+dx,570,139,192,2143);/*ContainerCenter wire_center*/ RenderUIImage(34+dx,521,235,44,2144);/*LevelsBox*/ RenderUIImage(40+dx,526,235,34,0);/*Background*/ RenderUIImage(43+dx,526,6,34,2142);/*Fill*/
                UIRImg(MID(isRH,WIRE_SLIDER),40+dx,509,235,69,1078);/*Handle - whole levels box drags*/ UIRImg(MID(isRH,WIRE_TARGET),204+dx,522,66,42,2145);/*TargetLine*/
                for (u8 n=0;n<14;++n) { i16 nx=(i16)((n<7?57:222)+dx),ny=wireNodeY[n%7]; UIRImg(MID(isRH,WIRE_NODE_0)+n,nx,ny,26,29,2146);/*wire_node*/ RenderUIImage((i16)(nx+4),(i16)(ny+6),16,16,0);/*SelectedIndicator*/ RenderUIImage((i16)(nx+1),(i16)(ny+3),22,22,0);/*GeniusHint*/ }
                UIRImg(MID(isRH,WIRE_CLOSE),259+dx,736,29,29,899); RenderTextL(259+dx,736,T_STOPD_RED,FONT_NORMAL,0.8,"X");
            }
            if (data==7) {/*SysAnalyzer*/
                UIRText(MID(isRH,SYS_HEADER),24+dx,523,T_YELLOW,FONT_NORMAL,0.8f,0,892<1100?Sys_Text.stringTable[892]:"SYSTEM ANALYZER");
                for (u8 r=0;r<11;++r) { UIRText(MID(isRH,SYS_DESC_0)+r,24+dx,sysRows[r].y,T_GREEN,FONT_NORMAL,0.8f,0,sysRows[r].d); UIRText(MID(isRH,SYS_VAL_0)+r,180+dx,sysRows[r].y,T_GREEN,FONT_NORMAL,0.8f,0,sysRows[r].v); }/*TODO values are placeholders, hook to level state*/
                UIRImg(MID(isRH,SYS_CLOSE),259+dx,527,29,29,899); RenderTextL(259+dx,527,T_STOPD_RED,FONT_NORMAL,0.8,"X");
            }
            if (data==9) {/*Minigames*/
                RenderUIImage(21+dx,501,262,262,1025);/*MinigamesContainer*/ UIRText(MID(isRH,MINIGAMES_HEADER),28+dx,503,T_RED,FONT_NORMAL,0.8f,0,"TRIOPTIMUM FUNPACK");
                for (u8 g=0;g<9;++g) { UIRImg(MID(isRH,MINIGAME_0)+g,(i16)(mgX[g]+dx),mgY[g],115,24,0);/*QUAD:builtin-white*/ RenderTextL((i16)(mgX[g]+5+dx),(i16)(mgY[g]+1),T_GREEN,FONT_NORMAL,0.8,"%s",mgName[g]); }
                UIR(MID(isRH,MINIGAMES_FOOTER),97+dx,726,(i16)(MeasureLineAdvance("Don't Play on",FONT_NORMAL)*0.8f),(i16)(3*22.0f*0.8f)); RenderTextL(97+dx,726,T_RED,FONT_NORMAL,0.8,"Don't Play on\n\nCompany Time");
                UIRImg(MID(isRH,MINIGAME_VIEW),21+dx,501,262,262,0);/*MinigameView QUAD:none*/
                UIRImg(MID(isRH,MINIGAME_CLOSE),259+dx,502,22,22,899); UIRImg(MID(isRH,MINIGAME_BACK),259+dx,502,22,22,899);/*TODO back/close share the corner; only one is live at a time*/
                RenderTextL(30+dx,545,T_WHITE,FONT_NORMAL,0.8,"PUZZLE SOLVED!"); RenderTextL(91+dx,710,T_WHITE,FONT_NORMAL,0.8,"YOU LOSE");
            }
        }
    }
}

static const i16 mediaColX[2]={458,681},mediaRowY[8]={570,591,612,633,654,675,696,717};
static const char* logLevelName[10]={"R","1","2","3","4","5","6","7","8","9"};
static const char* noteText[12]={"Escape neurosurgery suite.  Keycode is 451.","Disengage laser safety override.","Activate the station energy shield.","Destroy the mining laser.","Enable master jettison.","Diagnose and repair broken relay: 428.","Jettison Beta Grove.","Destroy the four relay antennae.","Engage reactor self-destruct.","Escape on escape pod.","Access the bridge.","Destroy SHODAN."};
static const char* swLabels[7]={"ICE DRILL","PULSER/DRILL","SHIELD","TURBO","DECOY","RECALL","GAMES"};
void CenterMFD() { //640x240
    if (Cheats.noHUD) return;
    static const i16 centerX[4]={400,480,560,902};
    for (u8 i=0;i<4;++i) UIRImg(UI_ID_CMFD_TAB_MAIN+i,centerX[i],752,64,32,(World.Sys_UI.mfdSelected[0]==i+1 && World.Sys_UI.MFD_CenterTab!=5) ? 1024 : 1021);/*Main/Hardware/General/Software center tab buttons*/
    if (World.inventoryMode && World.invP1.holdingObject) { UIR(UI_ID_CMFD_ADD_TO_INVENTORY,345,460,676,308); if (UIOver(UI_ID_CMFD_ADD_TO_INVENTORY)) { RenderUIImage(345,528,676,240,1075); RenderTextL(586,528,T_GREEN,FONT_NORMAL,0.8f,Sys_Text.stringTable[878]/*ADD TO INVENTORY*/); } }
    if (World.Sys_UI.showSensaroundCenter){
        /*TODO SensaroundCenter Center rearview image 630x240 texture*/
        UIR(UI_ID_SENSA_CTR,TAB_THICK+MFD_SPACING+SIDE_MFD_W+MFD_SPACINGCTR,UI_H-TAB_THICK-TXT_PAD-CTR_MFD_H,TAB_THICK+MFD_SPACING+SIDE_MFD_W+MFD_SPACINGCTR+CTR_MFD_W,UI_W-TAB_THICK-TXT_PAD);
    } else {
        i16 hdrH=UI_H-TAB_THICK-TXT_PAD-CTR_MFD_H+TXT_PAD;
        if (World.Sys_UI.MFD_CenterTab==1) {/*Main*/
            /*Column Hdrs*/UIRText(UI_ID_CMFD_WEAPON_HEADER,372,hdrH,T_RED,FONT_NORMAL,0.8f,200,Sys_Text.stringTable[870]/*"WEAPONS"*/); UIRText(UI_ID_CMFD_SHOTS_HEADER,574,hdrH,T_RED,FONT_NORMAL,0.8f,120,Sys_Text.stringTable[871]/*"SHOTS"*/); UIRText(UI_ID_CMFD_GREN_HEADER,722,hdrH,T_RED,FONT_NORMAL,0.8f,120,Sys_Text.stringTable[872/*GRENADES*/]); UIRText(UI_ID_CMFD_PATCH_HEADER,868,hdrH,T_RED,FONT_NORMAL,0.8f,120,Sys_Text.stringTable[873/*PATCHES*/]);
            for (int row=0;row<7;++row) {/*Grenades*/
                int slot=ConsumableSlot(0,row); u8 count=slot<0?0:World.invP1.grenAmmo[slot]; if (!count){continue;} u32 color=World.invP1.grenCur==slot?T_YELLOW:T_GREEN; i16 y=generalRowY[row]; const char* text=Sys_Text.stringTable[900+row];
                UIR((UI_ID_CMFD_GREN_ROW_0)+row,720,y,109,21); RenderTextL(722,y,color,FONT_NORMAL,0.8f,"%s",text); RenderTextL(780,y,color,FONT_NORMAL,0.8f,"%u",count); UIRImg(UI_ID_CMFD_GREN_USE_0+row,805,y-6,20,20,1086); RenderUIImage(810,y-2,11,11,1079);/*Row spans label+count+use, USE ids sort first so they win the overlap*/
            }
            for (int row=0;row<7;++row) {/*Patches*/
                int slot=ConsumableSlot(1,row); u8 count=slot<0?0:World.invP1.patchCounts[slot]; if (!count){continue;} u32 color=World.invP1.patchCur==slot?T_YELLOW:T_GREEN; i16 y=generalRowY[row]; const char* text=Sys_Text.stringTable[907+row];
                UIR(UI_ID_CMFD_PATCH_ROW_0+row,831,y,109,21); RenderTextL(868,y,color,FONT_NORMAL,0.8f,"%s",text); RenderTextL(949,y,color,FONT_NORMAL,0.8f,"%u",count); UIRImg(UI_ID_CMFD_PATCH_USE_0+row,974,y-6,20,20,1086); RenderUIImage(979,y-2,11,11,1079);/*Row spans label+count+use, USE ids sort first so they win the overlap*/
            }
            for(int slot=0;slot<7;++slot){
                int widx=World.invP1.weaponInventoryIndices[slot]; if(widx<0)continue; int y=generalRowY[slot]; UIR(UI_ID_CMFD_WEAPON_ROW_0+slot,372,(i16)(y-5),340,21); bool hov=UIOver(UI_ID_CMFD_WEAPON_ROW_0+slot); /*Slight shift of 6 feels better than just doing y and y + 22 as one would expect, then lopped 1 off one end to prevent double highlighting*/
                u32 col = (hov&&World.inventoryMode && World.invP1.weaponCurrent!=slot) ? T_GREEN_MENU : (World.invP1.weaponCurrent==slot?T_YELLOW:(World.invP1.weaponCurrentPending==slot?T_DARK_YELLOW:T_GREEN)); RenderTextL(372,y,col,FONT_NORMAL,0.8f,"%s",Sys_Text.stringTable[ItemStringIdx((i32)widx)]);/*Weapon text*/
                char b[64]; GetWeaponAmmoText(slot,b,sizeof(b)); RenderTextL(574,y,col,FONT_NORMAL,0.8f,"%s",b);/*Ammo text*/
            }
        }
        if (World.Sys_UI.MFD_CenterTab==2) {/*Hardware*/
            UIRText(UI_ID_CMFD_HARDWARE_HEADER,372,hdrH,T_RED,FONT_NORMAL,0.8f,260,Sys_Text.stringTable[874]/*HARDWARE*/);
            for (int slot=0;slot<14;++slot) {
                int ref=World.invP1.hardwareInvReferenceIndex[slot]; if (ref<0 || World.invP1.hwVers[slot] <= 0) continue;
                i16 x=generalColX[slot < 7 ? 0 : 1], y=generalRowY[slot%7];
                const char* label=Sys_Text.stringTable[ref+326]; UIRText(UI_ID_CMFD_HARDWARE_ROW_0+slot,x,y,World.invP1.hardwareInvCurrent==slot ? T_YELLOW : (World.invP1.hasHardware&(1u<<slot) ? T_GREEN_MENU : T_GREEN_MENU_SHADOW),FONT_NORMAL,0.8f,210,label); RenderTextL((i16)(x+300),y,World.invP1.hardwareInvCurrent==slot ? T_YELLOW : T_GREEN_MENU,FONT_NORMAL,0.8f,"v%d",(int)World.invP1.hwVers[slot]);
            }
        }
        if (World.Sys_UI.MFD_CenterTab==3) {/*General*/
            UIRText(UI_ID_CMFD_GENERAL_HEADER,372,hdrH,T_RED,FONT_NORMAL,0.8f,260,Sys_Text.stringTable[875]/*GENERAL*/);
            for (u8 slot=0;slot<14;++slot) {
                if (GeneralInvItem(slot)<0) continue;
                i16 x=slot<7?372:681,y=generalRowY[slot%7]; float width=MeasureLineAdvance(GeneralInvLabel(slot),FONT_NORMAL),scale=width>0?vmin(0.8f,185.0f/width):0.8f;
                UIRText(UI_ID_CMFD_GENERAL_ROW_0+slot,x,y,World.invP1.generalInvCurrent==slot?T_YELLOW:T_GREEN,FONT_NORMAL,scale,185,GeneralInvLabel(slot));
                if (GeneralInvCanUse(slot)) { UIRImg(UI_ID_CMFD_GENERAL_USE_0+slot,(i16)(x+187),(i16)(y+2),20,20,1086); RenderUIImage(x+192,y+7,11,11,1079); }
            }
        }
        if (World.Sys_UI.MFD_CenterTab==4) {/*Software*/
            UIRText(UI_ID_CMFD_SOFTWARE_HEADER,372,hdrH,T_RED,FONT_NORMAL,0.8f,260,Sys_Text.stringTable[876]/*SOFTS*/);
            for (int i=0;i<7;++i) { bool owned; int count=0;
                if (i<=2) owned=(World.invP1.hasSoft&(1u<<(i+3)))!=0; else if (i<=5) { count=World.invP1.softVersions[i]; owned=count>0 || (World.invP1.hasSoft&(1u<<(i+3)))!=0; if (count<0) count=0; } else owned=World.invP1.hasMinigame;
                if (!owned) continue;
                i16 y=(i16)(588+i*32); const char* label=swLabels[i]; bool selected=i==World.invP1.cyberItemIndex; float w=MeasureLineAdvance(label,FONT_NORMAL),sc=w>0?vmin(0.8f,210.0f/w):0.8f;
                UIRText(UI_ID_CMFD_SOFTWARE_ROW_0+i,454,y,selected?T_YELLOW:T_GREEN_MENU,FONT_NORMAL,sc,210,label);
                if (i<=2) RenderTextL(680,y,selected?T_YELLOW:T_GREEN_MENU,FONT_NORMAL,0.8f,"v%d",World.invP1.softVersions[i]+1); else if (i<=5) RenderTextL(680,y,selected?T_YELLOW:T_GREEN_MENU,FONT_NORMAL,0.8f,"x%d",count); else RenderTextL(680,y,selected?T_YELLOW:T_GREEN_MENU,FONT_NORMAL,0.8f,"%d",World.invP1.hasMinigame?1:0); }
        }
        if (World.Sys_UI.MFD_CenterTab!=5) return;/*EReader*/
        UIRText(UI_ID_CMFD_MEDIA_HEADER,372,hdrH,T_RED,FONT_NORMAL,0.8f,260,Sys_Text.stringTable[877]/*LOGS*/);
        if (World.Sys_UI.MFD_MediaTab==MM_LOG_TABLE) {
            if (World.Sys_UI.MFD_ReaderView==MFD_READER_CONTENTS) { RenderUIImage(454,573,453,191,0);/*LogTableofContents*/
                for (u8 i=0;i<10;++i) { i16 x=(i16)(i<7?454:681),y=generalRowY[i%7]; UIRImg(UI_ID_CMFD_LOG_TABLE_0+i,x,y,226,24,0);/*QUAD:builtin-white*/ RenderTextL(x,y,T_GREEN,FONT_NORMAL,0.8,"Level %s Logs",logLevelName[i]); RenderTextL((i16)(x+77),y,T_GREEN,FONT_NORMAL,0.8,"3");/*TODO real per level counts*/ }
            } else if (World.Sys_UI.MFD_ReaderView==MFD_READER_FOLDER) { RenderUIImage(458,570,445,188,0);/*LogsLevelFolder*/
                for (u8 i=0;i<15;++i) { i16 x=mediaColX[i/8],y=mediaRowY[i%8]; UIRImg(UI_ID_CMFD_LOG_ENTRY_0+i,x,y,222,21,0); RenderTextL(x,y,T_GREEN,FONT_NORMAL,0.8,"Log"); }
            } else if (World.Sys_UI.MFD_ReaderView==MFD_READER_TEXT) {
                UIR(UI_ID_CMFD_LOG_TEXT,372,576,456,140); RenderTextL(372,576,T_GREEN,FONT_NORMAL,0.8,"\"abc def ghi jkl mno pqrs tuv wxyz ABC DEF GHI JKL MNO PQRS TUV WXYZ !\"\\xA7\n$%%& /() =?* '<> #|; \\xB2\\xB3~ @`\\xB4 \\xA9\\xAB\\xBB \\xA4\\xBC\\x...");
                UIRImg(UI_ID_CMFD_LOG_MORE,372,576,456,174,0); RenderTextL(654,647,T_YELLOW,FONT_NORMAL,0.8,"%s",Sys_Text.stringTable[26]/*[MORE]*/); 
                UIRImg(UI_ID_CMFD_LOG_BACK,372,718,69,31,0); RenderTextL(372,718,T_YELLOW,FONT_NORMAL,0.8,"%s",Sys_Text.stringTable[879]/*[BACK]*/);
            }
        }
        if (World.Sys_UI.MFD_MediaTab==MM_EMAIL_TABLE) { RenderUIImage(458,hdrH+22,445,188,0);/*EmailTab*/ for (u8 i=0;i<15;++i) { i16 x=mediaColX[i/8],y=mediaRowY[i%8]; UIRImg(UI_ID_CMFD_EMAIL_ENTRY_0+i,x,y,223,21,0); RenderTextL(x,y,T_GREEN,FONT_NORMAL,0.8,"Email"); } }
        if (World.Sys_UI.MFD_MediaTab==MM_DATA_TABLE) { RenderUIImage(458,hdrH+22,445,188,0);/*DataTab*/ for (u8 i=0;i<13;++i) { i16 x=mediaColX[i/8],y=mediaRowY[i%8]; UIRImg(UI_ID_CMFD_DATA_ENTRY_0+i,x,y,223,21,0); RenderTextL(x,y,T_GREEN,FONT_NORMAL,0.8,"Data"); } }
        if (World.Sys_UI.MFD_MediaTab==MM_NOTES) {
            const u32 secCode[6]={World.lev1SecCode,World.lev2SecCode,World.lev3SecCode,World.lev4SecCode,World.lev5SecCode,World.lev6SecCode};
            for(u8 i=0;i<18;++i){i16 x=(i16)(372+(i/6)*167),y=(i16)(hdrH+22+(i%6)*29); UIR(UI_ID_CMFD_NOTE_TOGGLE_0+i,x,y,166,39); RenderUIImage(x,(i16)(y+2),19,18,910);/*Background*/ if (i<6)RenderTextL((i16)(x+21),(i16)(y+3),T_GREEN,FONT_NORMAL,0.8,"%s%d%s%s%u.",Sys_Text.stringTable[556],i+1,Sys_Text.stringTable[557],Sys_Text.stringTable[558],secCode[i]); else RenderTextL((i16)(x+21),(i16)(y+3),T_GREEN,FONT_NORMAL,0.8,"%s",noteText[i-6]);}
        }
    }
}

/*Sliders need .down rather than .pressed, so they get their own short pass ahead of the click walk.*/
static void UI_DragRegions(void) {
    if (!Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].down) return;
    for (u8 s=0;s<2;++s) {
        u32 id=MID(s,ITEM_TIMER_SLIDER); if (UIOver(id)) { World.Sys_UI.lastItemSideRH=s!=0; ConsumableSetTimer(((float)World.cursorPos_x-UIC(id).min.x)/224.0f); World.Sys_UI.mouseClickHeldOverGUI=World.uiIsBlocking=true; }
        id=MID(s,PUZZLE_SLIDER); if (UIOver(id)) { UI_PuzzleGridSlide(s!=0,vclamp(((float)World.cursorPos_x-UIC(id).min.x)/225.0f,0.0f,1.0f)); World.Sys_UI.mouseClickHeldOverGUI=World.uiIsBlocking=true; }
        id=MID(s,WIRE_SLIDER); if (UIOver(id)) { UI_WireSlide(s!=0,vclamp(((float)World.cursorPos_x-UIC(id).min.x)/235.0f,0.0f,1.0f)); World.Sys_UI.mouseClickHeldOverGUI=World.uiIsBlocking=true; }
    }
}
/*c: 1 LMB, 2 RMB, +4 double click of that same button.*/
static void UI_OnRegionClick(u32 id, u8 c) {
    bool rh=false, dbl=(c&4)!=0, left=(c&3)==1; if (id>=UI_ID_RMFD_TAB_WEAPON) { id-=UI_MFD_STRIDE; rh=true; }
    switch (id) {
        case UI_ID_HUD_SHOOTMODE: ForceShootMode(); return;
        case UI_ID_HUD_HW_0 ... UI_ID_HUD_HW_7: HwToggle((u8)(id-UI_ID_HUD_HW_0)); return;
        case UI_ID_CMFD_ADD_TO_INVENTORY: if (World.invP1.holdingObject) { AddItemToInventory(World.invP1.heldObjectIndex,World.invP1.heldObjectCustIdx); ResetHeldItem(); } return;
        case UI_ID_CMFD_TAB_MAIN ... UI_ID_CMFD_TAB_SOFTWARE: MFD_SelectTab(0,(u8)(id-UI_ID_CMFD_TAB_MAIN+1),true); if(World.Sys_UI.MFD_CenterTab==1){World.Sys_UI.firstMain=true;}else if(World.Sys_UI.MFD_CenterTab==2){World.Sys_UI.firstHardware=true;}else if(World.Sys_UI.MFD_CenterTab==3){World.Sys_UI.firstGeneral=true;} return;
        case UI_ID_CMFD_WEAPON_ROW_0 ... UI_ID_CMFD_WEAPON_ROW_6: if (left) WeaponSelectSlot((int)(id-UI_ID_CMFD_WEAPON_ROW_0)); return;
        case UI_ID_CMFD_GREN_USE_0 ... UI_ID_CMFD_GREN_USE_6: ConsumableUse(false,(int)(id-UI_ID_CMFD_GREN_USE_0)); return;
        case UI_ID_CMFD_GREN_ROW_0 ... UI_ID_CMFD_GREN_ROW_6: { int row=(int)(id-UI_ID_CMFD_GREN_ROW_0); ConsumableSelect(false,row); if (dbl) ConsumableUse(false,row); return; }
        case UI_ID_CMFD_PATCH_USE_0 ... UI_ID_CMFD_PATCH_USE_6: ConsumableUse(true,(int)(id-UI_ID_CMFD_PATCH_USE_0)); return;
        case UI_ID_CMFD_PATCH_ROW_0 ... UI_ID_CMFD_PATCH_ROW_6: { int row=(int)(id-UI_ID_CMFD_PATCH_ROW_0); ConsumableSelect(true,row); if (dbl) ConsumableUse(true,row); return; }
        case UI_ID_CMFD_HARDWARE_ROW_0 ... UI_ID_CMFD_HARDWARE_ROW_11: UI_HardwareRowClick((int)(id-UI_ID_CMFD_HARDWARE_ROW_0)); return;
        case UI_ID_CMFD_SOFTWARE_ROW_0 ... UI_ID_CMFD_SOFTWARE_ROW_6: UI_SoftwareRowClick((int)(id-UI_ID_CMFD_SOFTWARE_ROW_0)); return;
        case UI_ID_CMFD_GENERAL_USE_0 ... UI_ID_CMFD_GENERAL_USE_13: { u8 s=(u8)(id-UI_ID_CMFD_GENERAL_USE_0); MFD_GeneralChanged(); GeneralInvClick(s,World.invP1.generalInvCustIdx[s]); GeneralInvApply(s,World.invP1.generalInvCustIdx[s]); return; }
        case UI_ID_CMFD_GENERAL_ROW_0 ... UI_ID_CMFD_GENERAL_ROW_13: { u8 s=(u8)(id-UI_ID_CMFD_GENERAL_ROW_0); int item=GeneralInvItem(s); if (item<0) return;
            if (!left) { MFD_GeneralChanged(); if (s) GeneralInvTake(s); return; }
            /*Identity guard: the slot can be refilled between the two clicks of a double click.*/
            bool twice=dbl && World.Sys_UI.generalClickSlot==(i8)s && World.Sys_UI.generalClickItem==(i16)item && World.Sys_UI.generalClickCustom==World.invP1.generalInvCustIdx[s];
            MFD_GeneralChanged(); GeneralInvClick(s,World.invP1.generalInvCustIdx[s]);
            if (twice) GeneralInvApply(s,World.invP1.generalInvCustIdx[s]); else { World.Sys_UI.generalClickSlot=(i8)s; World.Sys_UI.generalClickItem=(i16)item; World.Sys_UI.generalClickCustom=World.invP1.generalInvCustIdx[s]; World.Sys_UI.generalClickTime=World.pauseRelativeTime; } return; }
        case UI_ID_CMFD_LOG_TABLE_0 ... UI_ID_CMFD_LOG_TABLE_9: UI_LogTableClick((int)(id-UI_ID_CMFD_LOG_TABLE_0)); return;
        case UI_ID_CMFD_LOG_ENTRY_0 ... UI_ID_CMFD_LOG_ENTRY_14: UI_LogEntryClick((int)(id-UI_ID_CMFD_LOG_ENTRY_0)); return;
        case UI_ID_CMFD_LOG_MORE: UI_LogMore(); return;
        case UI_ID_CMFD_LOG_BACK: UI_LogBack(); return;
        case UI_ID_CMFD_EMAIL_ENTRY_0 ... UI_ID_CMFD_EMAIL_ENTRY_14: UI_EmailEntryClick((int)(id-UI_ID_CMFD_EMAIL_ENTRY_0)); return;
        case UI_ID_CMFD_DATA_ENTRY_0 ... UI_ID_CMFD_DATA_ENTRY_12: UI_DataEntryClick((int)(id-UI_ID_CMFD_DATA_ENTRY_0)); return;
        case UI_ID_CMFD_NOTE_TOGGLE_0 ... UI_ID_CMFD_NOTE_TOGGLE_17: UI_NoteToggleClick((int)(id-UI_ID_CMFD_NOTE_TOGGLE_0)); return;
        case UI_ID_CMFD_EDIT_ROW_0 ... UI_ID_CMFD_EDIT_ROW_14: { if (!EditSelIsActive() || editFieldEditing) return; u8 slot=(u8)(id-UI_ID_CMFD_EDIT_ROW_0); char v[40]; EditFieldValueText(slot,editModeSelection,v,40); sFormat(editFieldBuffer,40,"%s",v); editFieldSlot=slot; editFieldEditing=true; return; }
        case UI_ID_LMFD_TAB_WEAPON ... UI_ID_LMFD_TAB_DATA: MFD_SelectTab((u8)(rh?2:1),(u8)(id-UI_ID_LMFD_TAB_WEAPON+1),true); return;
        case UI_ID_LMFD_AUTOMAP_ZOOM_IN: UI_AutomapClick(rh,UI_AUTOMAP_ZOOM_IN); return;
        case UI_ID_LMFD_AUTOMAP_ZOOM_OUT: UI_AutomapClick(rh,UI_AUTOMAP_ZOOM_OUT); return;
        case UI_ID_LMFD_MEDIA_TAB_0 ... UI_ID_LMFD_MEDIA_TAB_3: { u8 section=(u8)(id-UI_ID_LMFD_MEDIA_TAB_0); if (section==MM_NOTES && !World.diffMis) return;
            World.Sys_UI.MFD_CenterTab=5; World.Sys_UI.MFD_MediaTab=World.Sys_UI.lastMultiMediaTabOpened=section; World.Sys_UI.MFD_ReaderView=MFD_READER_CONTENTS;
            if (section>=MM_DATA_TABLE) { World.Sys_UI.highlightStatus[section]=false; World.Sys_UI.highlightTickCount[section]=0; } play_wav(sounds[97],SfxVol(),(V3){0,0,0},false); return; }
        case UI_ID_LMFD_ITEM_USE: { World.Sys_UI.lastItemSideRH=rh; if (World.Sys_UI.mfdConsumable) { int row=ConsumableSelectedRow(); ConsumableUse(World.Sys_UI.mfdConsumable==2,row); } else { int s=World.invP1.generalInvCurrent; MFD_GeneralChanged(); if (GeneralInvCanUse(s)) GeneralInvApply(s,World.invP1.generalInvCustIdx[s]); } return; }
        case UI_ID_LMFD_ITEM_VAPORIZE: World.Sys_UI.lastItemSideRH=rh; MFD_GeneralChanged(); if (GeneralInvCanVaporize(World.invP1.generalInvCurrent)) VaporizeClick(); return;
        case UI_ID_LMFD_ITEM_TIMER_SLIDER: World.Sys_UI.lastItemSideRH=rh; ConsumableSetTimer(((float)World.cursorPos_x-UIC(MID(rh,ITEM_TIMER_SLIDER)).min.x)/224.0f); return;
        case UI_ID_LMFD_WEAPON_ICON: UI_WeaponIconClick(rh); return;
        case UI_ID_LMFD_ELEV_BUTTON_0 ... UI_ID_LMFD_ELEV_BUTTON_7: UI_ElevFloorClick(rh,(int)(id-UI_ID_LMFD_ELEV_BUTTON_0)); return;
        case UI_ID_LMFD_ELEV_CLOSE: UI_ElevClose(rh); return;
        case UI_ID_LMFD_KEYCODE_0 ... UI_ID_LMFD_KEYCODE_11: UI_KeycodeKey(rh,(int)(id-UI_ID_LMFD_KEYCODE_0)); return;
        case UI_ID_LMFD_KEYCODE_CLOSE: UI_KeycodeClose(rh); return;
        case UI_ID_LMFD_AUDIOLOG_IMAGE: UI_AudioLogClick(rh); return;
        case UI_ID_LMFD_PUZZLE_CELL_0 ... UI_ID_LMFD_PUZZLE_CELL_34: UI_PuzzleGridCell(rh,(int)(id-UI_ID_LMFD_PUZZLE_CELL_0)); return;
        case UI_ID_LMFD_PUZZLE_CLOSE: UI_PuzzleGridClose(rh); return;
        case UI_ID_LMFD_WIRE_NODE_0 ... UI_ID_LMFD_WIRE_NODE_13: UI_WireNodeClick(rh,(int)(id-UI_ID_LMFD_WIRE_NODE_0)); return;
        case UI_ID_LMFD_WIRE_CLOSE: UI_WireClose(rh); return;
        case UI_ID_LMFD_SYS_CLOSE: UI_SysAnalyzerClose(rh); return;
        case UI_ID_LMFD_MINIGAME_0 ... UI_ID_LMFD_MINIGAME_8: UI_MinigameStart(rh,(int)(id-UI_ID_LMFD_MINIGAME_0)); return;
        case UI_ID_LMFD_MINIGAME_VIEW: UI_MinigameInput(rh,World.cursorPos_x,World.cursorPos_y); return;
        case UI_ID_LMFD_MINIGAME_BACK: UI_MinigameBack(rh); return;
        case UI_ID_LMFD_MINIGAME_CLOSE: UI_MinigameClose(rh); return;
        case UI_ID_LMFD_SEARCH_ICON_0 ... UI_ID_LMFD_SEARCH_ICON_3: World.Sys_UI.lastSearchSideRH=rh; SearchTakeSlot((u8)(id-UI_ID_LMFD_SEARCH_ICON_0)); return;
        case UI_ID_LMFD_SEARCH_CLOSE: CloseSearch(); return;
        default: return;/*Decoration, labels and panel backdrops: they block the pointer but have no action*/
    }
}
static bool UI_DispatchRegions(void) { for (u32 id=UI_ID_NONE+1;id<UI_ID_COUNT;++id) { if (!UIC(id).active) continue; u8 c=UIClicked(id); if (!c) continue; UI_OnRegionClick(id,c); return true; } return false; }
void UI_ProcessNavigation(void) {
    UpdateSearchTether();
    if (World.menuActive || World.paused || World.creditsActive || Cheats.consoleActive || World.Sys_UI.vmailActive) { MFD_GeneralChanged(); return; }
    static const u16 keys[7]={KEY_F1,KEY_F2,KEY_F3,KEY_F4,KEY_F5,KEY_F7,KEY_F8}; static const u8 tabs[7]={1,2,3,4,1,2,3};/*TODO no key for RH data tab*/
    for (u8 i=0;i<7;++i) if (Sys_Input.keyStates[keys[i]].pressed) { Sys_Input.keyStates[keys[i]].pressed=false; MFD_SelectTab(i<4?1:2,tabs[i],true); }
    for (u8 up=0;up<2;++up) { u16 key=up?KEY_PAGE_UP:KEY_PAGE_DOWN; if (!Sys_Input.keyStates[key].pressed) continue;
        Sys_Input.keyStates[key].pressed=false; u8 tab=World.Sys_UI.MFD_CenterTab?World.Sys_UI.MFD_CenterTab:World.Sys_UI.mfdSelected[0];
        MFD_SelectTab(0,tab==5?1:1+(tab-1+(up?3:1))%4,false); World.Sys_UI.MFD_ReaderView=MFD_READER_CONTENTS; if(tab==1){World.Sys_UI.firstMain=true;}else if(tab==2){World.Sys_UI.firstHardware=true;}else if(tab==3){World.Sys_UI.firstGeneral=true;}
    }
    if (!World.inventoryMode || Cheats.noHUD) { MFD_GeneralChanged(); return; }
    if (World.Sys_UI.generalClickSlot>=0 && World.pauseRelativeTime-World.Sys_UI.generalClickTime>UI_DBLCLICK) MFD_GeneralChanged();
    UI_DragRegions(); UI_DispatchRegions();
}

static double RenderUI() {
    drawCallsNormal=drawCalls; UI_BeginFrame();
    if (!EditSelIsActive()) editFieldEditing=false;
    if (World.creditsActive) {
        if (Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed) { ++World.creditsPageIndex; if (World.creditsPageIndex>CREDITS_PAGES) { World.creditsActive=false; return get_time(); } }
        if (World.creditsPageIndex==1) { CreditsStats(); RenderTextL(300,10,T_WHITE,FONT_NORMAL,1.0f,(const char*)&creditStats); }
        else RenderTextL(300,10,T_WHITE,FONT_NORMAL,1.0f,creditPages[World.creditsPageIndex]);
        return get_time();
    }
    if (World.menuActive) RenderMenu(); else if (World.paused) RenderPausedUI();
    if (World.menuActive || World.paused) {
        if (resDropdownOpen && currentMenuPage==Mpg_Options && currentMenuTab==0) {
            if (Sys_Input.keyStates[KEY_DOWN].pressed) { if(resSelectedIdx<resDropdownCount-1){resSelectedIdx++;} }
            else if (Sys_Input.keyStates[KEY_UP].pressed) { if(resSelectedIdx>0){resSelectedIdx--;} }
        }
        else if (Sys_Input.keyStates[KEY_DOWN].pressed) currentMenuItem=(currentMenuItem+1)>=menuItemCount?0:currentMenuItem+1;
        else if (Sys_Input.keyStates[KEY_UP].pressed) currentMenuItem=(currentMenuItem-1)<0?menuItemCount-1:currentMenuItem-1;
    } else if (!World.Sys_UI.vmailActive) {
        if (!Cheats.noHUD) {
            TickBar(false); TickBar(true); HardwareButtons(); UIRImg(UI_ID_HUD_SHOOTMODE,667,0,32,32,1020);
            for (u8 i=0;i<10;++i) if (World.Sys_UI.tWrnFinished[i]>World.pauseRelativeTime) {
                char flt[6]; if (World.Sys_UI.tWrnTextIdx[i]==185) sFormat(flt,6,"%.1f",(double)World.instances[PLAYER1].radiation);
                RenderTextL(340,72+i*18,World.Sys_UI.tWrnColorIdx[i],FONT_NORMAL,0.8f,"%s%s%s",Sys_Text.stringTable[World.Sys_UI.tWrnTextIdx[i]],World.Sys_UI.tWrnTextIdx[i]==185?flt:World.Sys_UI.tWrnTextIdx2[i]>=0?Sys_Text.stringTable[World.Sys_UI.tWrnTextIdx2[i]]:"",World.Sys_UI.tWrnTextIdx3[i]>=0?Sys_Text.stringTable[World.Sys_UI.tWrnTextIdx3[i]]:"");
            }
            SideMFD(false); CenterMFD(); SideMFD(true);
            if(World.diffMis>=3){UIR(UI_ID_CMFD_MISSION_TIMER,43,2,260,14); RenderTextL(43,2,T_YELLOW,FONT_NORMAL,0.8,"%s",World.misTimerMission<1100?Sys_Text.stringTable[World.misTimerMission]:"");/*MissionTimerT*/ {char misT[8]; if(World.misTimerTimesUP) sFormat(misT,sizeof(misT),"%s",869<1100?Sys_Text.stringTable[869]:""); else {float mt=World.misTimerT<0.0f?0.0f:World.misTimerT; int mm=(int)(mt/60.0f),ss=(int)(mt-(float)(mm*60)); sFormat(misT,sizeof(misT),"%02d:%02d",mm,ss);} RenderTextL(258,2,T_YELLOW,FONT_NORMAL,0.8,"%s",misT);}}
            if (World.curLev==LEVEL_CYBERSPACE) { UIR(UI_ID_CMFD_CYBER_TIMER,28,530,80,14); RenderTextL(28,530,T_WHITE,FONT_NORMAL,0.8,"T -"); RenderTextL(68,530,T_WHITE,FONT_NORMAL,0.8,"99:99"); }
            if (World.curLev==LEVEL_CYBERSPACE)RenderTextL(1137,570,T_YELLOW,FONT_NORMAL,0.8,Sys_Text.stringTable[442]/*"level 1 elevator taken off line - SHODAN security block established 04.NOV.72"*/);/*CyberSPrint*/
            if((World.invP1.hardwareIsActive & HW_BIO)!=0){/*BioMonitor*/
                BiomonitorBlitToUI();
                char biomText[128];
                int y = 83;
                RenderTextL(4,y,T_YELLOW,FONT_NORMAL,0.8,"%s",Sys_Text.stringTable[526]); /*Biomonitor Active:*/ y+=16;
                RenderTextL(4,y,T_GREEN,FONT_NORMAL,0.8,"%s",Sys_Text.stringTable[527]); /*Heart Rate:*/
                sFormat(biomText,sizeof(biomText),"%d",(int)bioMonitor.heartRate); RenderTextL(120,y,T_GREEN,FONT_NORMAL,0.8,"%s",biomText);
                RenderTextL(142,y,T_GREEN,FONT_NORMAL,0.8,"%s",Sys_Text.stringTable[529]); /*BPM*/ y+=16;
                if (World.invP1.hwVers[HW_BIO_IDX] > 1 && World.invP1.patchActive) {
                    RenderTextL(4,y,T_GREEN,FONT_NORMAL,0.8,"%s",Sys_Text.stringTable[528]); /*Patches Active:*/
                    int px = 119; bool first = true;
                    if (World.invP1.patchActive & PATCH_MEDI) { RenderTextL(px,y,T_GREEN,FONT_NORMAL,0.8,"%s",first?Sys_Text.stringTable[520]:" "); px += 40; first = false; }
                    if (World.invP1.patchActive & PATCH_STAMINUP) { RenderTextL(px,y,T_GREEN,FONT_NORMAL,0.8,"%s",first?Sys_Text.stringTable[521]:" "); px += 48; first = false; }
                    if (World.invP1.patchActive & PATCH_SIGHT) { RenderTextL(px,y,T_GREEN,FONT_NORMAL,0.8,"%s",first?Sys_Text.stringTable[522]:" "); px += 36; first = false; }
                    if (World.invP1.patchActive & PATCH_GENIUS) { RenderTextL(px,y,T_GREEN,FONT_NORMAL,0.8,"%s",first?Sys_Text.stringTable[523]:" "); px += 42; first = false; }
                    if (World.invP1.patchActive & PATCH_BERSERK) { RenderTextL(px,y,T_GREEN,FONT_NORMAL,0.8,"%s",first?Sys_Text.stringTable[524]:" "); px += 42; first = false; }
                    if (World.invP1.patchActive & PATCH_REFLEX) { RenderTextL(px,y,T_GREEN,FONT_NORMAL,0.8,"%s",first?Sys_Text.stringTable[525]:" "); px += 36; first = false; }
                    if (World.invP1.patchActive & PATCH_DETOX) { RenderTextL(px,y,T_GREEN,FONT_NORMAL,0.8,"%s",first?Sys_Text.stringTable[530]:" "); px += 36; first = false; }
                    y+=16;
                }
                RenderTextL(4,y,T_GREEN,FONT_NORMAL,0.8,"%s",Sys_Text.stringTable[531]); /*Fatigue:*/
                if (World.invP1.fatigue >= 80.0f) RenderTextL(120,y,T_GREEN,FONT_NORMAL,0.8,"%s",Sys_Text.stringTable[532]); /*High!*/
                else if (World.invP1.fatigue > 30.0f) RenderTextL(120,y,T_GREEN,FONT_NORMAL,0.8,"%s",Sys_Text.stringTable[533]); /*Moderate*/
                else RenderTextL(120,y,T_GREEN,FONT_NORMAL,0.8,"%s",Sys_Text.stringTable[534]); /*Low*/
            }
            RenderTextL(1270,78,T_WHITE,FONT_NORMAL,0.8,"0"); RenderTextL(1308,78,T_WHITE,FONT_NORMAL,0.8,"0"); RenderSearchFX();
        }
        if (EditSelIsActive()) {/*Edit mode selection highlight + object info panel*/
            u16 sel=editModeSelection; Entity* e=&World.instances[sel];
            V3 f=World.instances[PLAYER1].forward,rt=World.instances[PLAYER1].right,ff=(V3){-f.x,-f.y,-f.z},up=V3_Normalize(V3_Cross(rt,ff)),d=V3_AsubB(World.position[sel],World.position[PLAYER1]); float bz=V3_dot(d,f);
            if (bz > 0.01f) { float tanFov=vtan((float)Sys_Settings.FOV*0.5f*PI/180.0f),k=384.0f/(bz*tanFov); float sx=683.0f+V3_dot(d,rt)*k, sy=384.0f-V3_dot(d,up)*k; if (sx > -48.0f && sx < 1414.0f && sy > -48.0f && sy < 816.0f) RenderUIImage((i16)(sx-24.0f),(i16)(sy-24.0f),48,48,1051); }
            UIRImg(UI_ID_CMFD_EDIT_PANEL,966,84,400,600,1025);/*Edit object info panel bg*/
            RenderTextL(EF_LABELX,104,T_YELLOW,FONT_NORMAL,1.0f,"EDIT OBJECT #%u",sel); {char v[40];sFormat(v,40,"%u",e->index);RenderTextL(EF_LABELX,132,T_GREEN,FONT_NORMAL,1.0f,"const index");RenderTextL(EF_VALUEX,132,T_GREEN,FONT_NORMAL,1.0f,"%s",v);} bool caretOn=((u32)(get_time()*2.0f)&1)!=0;
            for(int i=0;i<EF_LAST;++i){u8 slot=(u8)i;i16 y=efRowY[i];char v[40];EditFieldValueText(slot,sel,v,40);
                bool editingThis=editFieldEditing&&editFieldSlot==slot; RenderTextL(EF_LABELX,y,editingThis?T_RED:T_GREEN,FONT_NORMAL,1.0f,"%s",efRowLabel[i]);
                if(editingThis){char buf[44];sFormat(buf,44,"%s%s",editFieldBuffer,caretOn?"|":"");RenderTextL(EF_VALUEX,y,T_RED,FONT_NORMAL,1.0f,"%s",buf);}
                else { float w=MeasureLineAdvance(v,FONT_NORMAL); if (World.inventoryMode) UIR(UI_ID_CMFD_EDIT_ROW_0+i,EF_VALUEX,y,(i16)w,26); bool hov=UIOver(UI_ID_CMFD_EDIT_ROW_0+i); RenderTextL(EF_VALUEX,y,hov?T_YELLOW:(i<EF_TEX?T_WHITE:T_GREEN),FONT_NORMAL,1.0f,"%s",v); }
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
        UIRImg(UI_ID_VMAIL_VIEWER,283,184,800,400,World.Sys_UI.vmailFrame);/*Vmail viewer*/
    }
    i16 debugTextStartY = 48;/*Diagnostics / Debugging*/
    if (Cheats.showLocation && !World.menuActive) RenderTextL(16, debugTextStartY, T_WHITE, FONT_NORMAL,1.0f, "x: %.4f, y: %.4f, z: %.4f, rx: %.4f, ry: %.4f, rz: %.4f, rw: %.4f",World.position[PLAYER1].x,World.position[PLAYER1].y,World.position[PLAYER1].z,World.rotation[PLAYER1].x,World.rotation[PLAYER1].y,World.rotation[PLAYER1].z,World.rotation[PLAYER1].w);
    i16 lineSpacing = 18;
    if (!World.menuActive && !Cheats.noHUD && Cheats.showFPS) RenderTextL(16,debugTextStartY + (lineSpacing * 1),T_WHITE,FONT_NORMAL,1.0f,"GPU ms::All:%.2f, Shad:%.2f, Pre:%.2f, Main:%.2f, SSR:%.2f, Comp:%.2f",World.gpuFrameMs,World.gpuShadowMs,World.gpuPreMs,World.gpuMainMs,World.gpuSsrMs,World.gpuCompMs);
    if (!World.menuActive && !Cheats.noHUD && Cheats.showFPS) RenderTextL(16,debugTextStartY + (lineSpacing * 2),T_WHITE,FONT_NORMAL,1.0f,"CPU ms::Shad:%.3f, Phys:%.3f, Subs:%u, Rend:%.3f, Pre Phys:%.3f, Logic:%.3f",shadowTime * 1000,physTime * 1000,World.substeps,renderTime * 1000,prePhys * 1000,gameTime * 1000);
    if (!World.menuActive && !Cheats.noHUD && !World.paused && Cheats.showFPS) RenderTextL(16,debugTextStartY + (lineSpacing * 3),T_WHITE,FONT_NORMAL,1.0f,"Grounded: %u  weaponCurrent: %d  weaponIndex: %d  pendingIdx: %d  wep16: %d  viewModel: %u  reloadDone: %.2f",(World.instances[PLAYER1].entflags & EF_GROUNDED) > 0,(int)World.invP1.weaponCurrent,(int)World.invP1.weaponIndex,(int)World.invP1.weaponIndexPending,Get16WeaponIndexFromConstIndex((int)World.invP1.weaponIndex),((Get16WeaponIndexFromConstIndex((int)World.invP1.weaponIndex)==5||Get16WeaponIndexFromConstIndex((int)World.invP1.weaponIndex)==6)?49u:((Get16WeaponIndexFromConstIndex((int)World.invP1.weaponIndex)==0||Get16WeaponIndexFromConstIndex((int)World.invP1.weaponIndex)==1)?50u:0u)),World.invP1.reloadFinished);
    if (!World.menuActive && !Cheats.noHUD && Cheats.showFPS) RenderTextL(16,debugTextStartY + (lineSpacing * 4),T_WHITE,FONT_NORMAL,1.0f,"Test Edx: %u, Time Elapsed: %.3f, Fatigue: %.2f, Sprinting: %u, Reverb: %u",editModeTestEntityDefinition,World.pauseRelativeTime - game_actual_start_time,World.invP1.fatigue,Sprint(),World.invP1.inReverbZone);
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
    i16 curhalf=CURSOR_SZ/2; float curhalff=(float)curhalf;
    if ((World.inventoryMode && !Cheats.noHUD) || World.menuActive || World.paused){RenderUIImage((i16)(World.cursorPos_x) - curhalf,(i16)(World.cursorPos_y) - curhalf,CURSOR_SZ,CURSOR_SZ,GetCursorTexture());}else if (!Cheats.noHUD){RenderUIImage((i16)((float)UI_W*0.5f-curhalff),(i16)((float)UI_H*0.5f-curhalff),CURSOR_SZ,CURSOR_SZ,GetCursorTexture());}/*Centered on UI fixed resolution 1366x768 FBO*/
    return time_now;
}
