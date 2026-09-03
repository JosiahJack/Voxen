// voxen.c - A realtime OpenGL 4.3+ Game Engine for Citadel: The System Shock Fan Remake.  Main translation unit.  Core renderer.  OS Shim Layer.
#include "common.h"
#include "credits.h"
#include "Shaders/shaders.h"
// Rendering
u32 inputImageID,inputUIID,inputDepthID,inputWorldPosID,inputSpecID,inputNormalID,gBufferFBO,uiFBO,outputImageID,depthPrepassSP,chunkSP,chunkVAO,chunkVBO,uiSP,debugUnlitSP,shadowmapsSP,shadowmapsClearSP,shadowMapSSBO,shadowMapsIndirectionID,ssrSP,imageBlitSP,quadVAO,quadVBO,
    textSP,textVAO,textVBO,debugLinesVAO,debugLinesVBO,matricesBufferID,cellVisibleDataID,debugLineColors,colorBufferID,texPalID,texPalOfsID,textureOffsetsID,textureSizesID,lightsID,voxListCntsID,voxelLightListsID,voxelUpdateSP,vbos[MAX_MDLS],tbos[MAX_MDLS];
float berserkSeedTime,rasterPerspectiveProjection[16],shadowmapsPerspectiveProjection[16],lightView[LIGHT_COUNT][6][4][4],lightViewProj[LIGHT_COUNT][6][16];
// Entity Management
float modelMatrices[INSTANCE_COUNT*16];
float *world_from_mdl = modelMatrices; // Alias for physics collision
u16** modelTriangles; u32 modelVertexCounts[MAX_MDLS]; u16 modelTriangleCounts[MAX_MDLS]; float modelBounds[MAX_MDLS]; u16 mdlsCnt; float **physPos; u16** physTris; u32* physVertCounts;
bool mouseMovementThisFrame,window_has_focus,ignore_next_mouse_delta,returnToPause=false,fovSliderActive=false,gammaSliderActive=false,masterVolumeSliderActive=false,musicVolumeSliderActive=false,messageVolumeSliderActive=false,sfxVolumeSliderActive=false,enteringPlayerName=false;
u8 currentPlayerNameLength=0; i8 currentMenuItem=0, currentMenuTab=0, menuItemCount=4, menuTabCount=1; i32 threadCnt=0; u32 globalframe=0,globalframesPerLastSecond;
SettingsSystem Sys_Settings = { // Potato defaults so initial state is good on first run for potatoes (e.g. won't crash for out of VRAM, or won't take 5min to init).
    .InputCodeSettings = {  5,/*Forward=F*/      0,/*Strafe Left=A*/      18,/*Backpedal=S*/        3,/*Strafe Right=D*/ 100,/*Jump=SPACE*/        2,/*Crouch=C*/     23,/*Prone=X*/      16,/*Lean Left=Q*/       4,/*Lean Right=E*/
                           45,/*Sprint=LSHIFT*/ 38,/*Turn Left=LARROW*/   39,/*Turn Right=RARROW*/ 36,/*Look Up=UARROW*/  37,/*Look Down=DARROW*/ 20,/*Recent Log=U*/ 26,/*Biomonitor=1*/ 27,/*Sensaround=2*/     28,/*Lantern=3*/
                           29,/*Shield=4*/      30,/*Infrared=5*/         31,/*Email=6*/           32,/*Booster=7*/       33,/*Jumpjets=8*/       56,/*Attack=LMB*/   57,/*Use=RMB*/      99,/*Menu/Back=ESCAPE*/ 97,/*Toggle Mode=TAB*/
                           17,/*Reload=R*/     127,/*Weapon+=MWHEEL+*/   128,/* Weapon-=MWHEEL-*/   6,/* Grenade=G*/      19,/*Grenade + = T*/   131,/*Grenade-=*/    21,/*Ammo Type=V*/
                           9,/*Patch Use=J*/     8,/*Patch+=I*/          132,/*Patch-=,*/          12,/*Full Map=M*/      21,/*Swim Up= V*/        2,/*Swim Down=C*/ 102,/*Console=`*/   101/*Screenshot=F12*/},
    .ScreenWidth=800u,.ScreenHeight=600u,.Fullscreen=0u,.FOV=65u,.Brightness=50u,.Gamma=50u,.FXAA=0u,.Shadows=0u,.Reflections=0u,.Vsync=0u,.ModelDetail=0u,.CurrentMonitor=0u, .GI=0u,.SpeakerMode=1u,.Reverb=0u,.VolumeMaster=100u,.VolumeMusic=25u,.VolumeMessage=75u,.VolumeEffects=100u,.Language=0u,.DynamicMusic=1u,.Footsteps=1u,.InvertLook=0u,
    .InvertCyberspaceLook=0u,.QuickItemPickup=0u,.QuickReloadWeapons=0u,.MouseSensitivity=10u,.NoShootMode=0u,.HeadBob=1u,.SSR_RES=4u};/*Ratio is (1 / SSR_RES) * res*/
InputSystem Sys_Input;
TextSystem Sys_Text;
CheatsSystem Cheats = {.god=false, .noclip=false, .showLocation=false, .showFPS=false, .editMode=false, .showPhys=false};
static bool shadowBuffersCreated = false;
CamView camViews[64], levelCamViews[14][64]; u8 camViewCount, levelCamViewCount[14]; u32 camViewTextures[64], levelCamViewTextures[14][64], drawCalls, uiDrawCalls, shadDrawCalls, vertsRendered, drawCallsNormal;
FrustumPlane lightFrustumPlanes[LIGHT_COUNT][6][6], playerFrustumPlanes[6];
u16 editModeSelection, editModeTestEntityDefinition=343; u16 lastSpawned=U16_MAX;
double game_start_time,game_actual_start_time,shadowTime,physTime,renderTime,prePhys,gameTime; u32 shadowmapIndirectionList[LIGHT_COUNT]; u16 texCnt; bool doubleSidedTexture[MAX_TXRS],transparentTexture[MAX_TXRS];
static u32 gpuQ[5][5]; static u8 gpuQFrame=0; /* [frame][shad,pre,main,ssr,comp] */
static const u8 Mpg_FrontPage=0,Mpg_Singleplayer=1,Mpg_Multiplayer=2,Mpg_NewGame=3,Mpg_Load=4,Mpg_Options=5,Mpg_Save=6,Mpg_IntroVideo=7,Mpg_CreditsVideo=8; u8 currentMenuPage = Mpg_FrontPage; bool resDropdownOpen = false; int resDropdownCount=0,resSelectedIdx=0;
typedef struct {int w,h;} ResMode; ResMode resModes[8];
GlobalContext World = {0};
Color textColors[] = {{1.0f,1.0f,1.0f,1.0f},/* 0 White T_WHITE*/ {0.890196078f,0.874509804f,0.0f,1.0f},/* 1 Yellow T_YELLOW*/  {0.623529412f,0.611764706f,0.0f,1.0f},/* 2 Dark Yellow (Yellow * 0.7f) T_DARK_YELLOW*/ {0.372549020f,0.654901961f,0.168627451f,1.0f},/* 3 Green T_GREEN*/ {0.917647059f,0.137254902f,0.168627451f,1.0f},/* 4 Red T_RED*/
                      {1.0f,0.498039216f,0.0f,1.0f}, /* 5 Orange T_ORANGE*/ {0.674509804f,0.058823529f,0.070588235f,1.0f},/* 6 StopD Red T_STOPD_RED*/ {0.941176471f,0.282352941f,0.298039216f,1.0f},/* 7 StopD Red Highlight T_STOPD_RED_HIGHLIGHT*/ {0.909803922f,0.203921569f,0.219607843f,1.0f}, /* 8 StopD Red Pause Title T_STOPD_RED_PAUSETITLE*/
                      {0.470588235f,0.721568627f,0.172549020f,1.0f},/* 9 Green Menu Title T_GREEN_MENU*/ {0.137254902f,0.356862745f,0.109803922f,1.0f},/* 10 Green Menu Title Shadow T_GREEN_MENU_SHADOW*/ {0.239215686f,0.466666667f,0.129411765f,1.0f}, /* 11 Green Menu Title Glow T_GREEN_MENU_GLOW*/ {0.392156863f,0.031372549f,0.039215686f,1.0f} /* 12 Red Menu Text Dark T_RED_MENU*/ };
// Wireline Rendering
typedef struct { float x,y,z,r,g,b,a; } DebugLineVertex;
DebugLineVertex* debugLineVerts = NULL;
INLINE void DrawDebugLines(float* viewProj) {
    if (!debugLineVerts || World.debugLineVertCount == 0) {return;}
    glBindBuffer(GL_ARRAY_BUFFER,debugLinesVBO); glBufferSubData(GL_ARRAY_BUFFER,0,World.debugLineVertCount * sizeof(DebugLineVertex),debugLineVerts); glUseProgram(debugUnlitSP); glUniformMatrix4fv(0,1,GL_FALSE,viewProj); glLineWidth(1.0f); glDisable(GL_DEPTH_TEST); glBindVertexArray(debugLinesVAO); 
    glDrawArrays(0x0001/*GL_LINES*/,0,World.debugLineVertCount); drawCalls++; vertsRendered += World.debugLineVertCount; glEnable(GL_DEPTH_TEST); World.debugLineVertCount = 0;
}

void BioMonitorUpdate(void);
void DrawLine(V3 start, V3 end, Color col) {
    if (!debugLineVerts || World.debugLineVertCount >= MAX_WIRELINE_VRTS - 2) return;
    int i = World.debugLineVertCount;
    debugLineVerts[i].x = start.x; debugLineVerts[i].y = start.y; debugLineVerts[i].z = start.z; debugLineVerts[i].r = col.r; debugLineVerts[i].g = col.g; debugLineVerts[i].b = col.b; debugLineVerts[i].a = col.a; i++;
    debugLineVerts[i].x = end.x; debugLineVerts[i].y = end.y; debugLineVerts[i].z = end.z;       debugLineVerts[i].r = col.r; debugLineVerts[i].g = col.g; debugLineVerts[i].b = col.b; debugLineVerts[i].a = col.a; i++;
    World.debugLineVertCount = i;
}

INLINE Color ColliderColor(u16 i) { return (!(World.instances[i].entflags & EF_RIGIDBODY) || PhysIsAsleep(i)) ? textColors[T_GREEN_MENU_SHADOW] : ((World.colliding[i]) ? textColors[T_RED] : textColors[T_GREEN]); }
void DrawVelocityVector(u16 i) {
    if (!(World.instances[i].entflags & EF_RIGIDBODY)) {return;}
    V3 tip = V3_AplusB(World.position[i],V3_ScaleByF(World.velocity[i],0.25f)); DrawLine(World.position[i],tip,textColors[T_ORANGE]); V3 perp = V3_Normalize(V3_Cross(World.velocity[i],(vabs(World.velocity[i].y/V3_Mag(World.velocity[i])) < 0.9f) ? (V3){0,1,0} : (V3){1,0,0}));
    DrawLine(V3_AplusB(tip,V3_ScaleByF(perp,0.05f)),V3_AsubB(tip,V3_ScaleByF(perp,0.05f)),textColors[T_ORANGE]); // Small cross at tip so zero-length vecs are still visible when barely moving
}

void DrawBoxColliderColored(u16 i, Color col) {
    ShapeBox b = Entity_GetBox(i); V3 c[8],px,py,pz, ax=quat_rot_v3(b.rot,(V3){1,0,0}), ay=quat_rot_v3(b.rot,(V3){0,1,0}), az=quat_rot_v3(b.rot,(V3){0,0,1}); px=V3_ScaleByF(ax,b.hExt.x); py=V3_ScaleByF(ay,b.hExt.y); pz=V3_ScaleByF(az,b.hExt.z);
    for (int s=0;s<8;s++) { float sx=(s&1)?1.f:-1.f,sy=(s&2)?1.f:-1.f,sz=(s&4)?1.f:-1.f; c[s]=V3_AplusB(b.ctr,V3_AplusB(V3_AplusB(V3_ScaleByF(px,sx),V3_ScaleByF(py,sy)),V3_ScaleByF(pz,sz))); }
    DrawLine(c[0],c[1],col); DrawLine(c[2],c[3],col); DrawLine(c[4],c[5],col); DrawLine(c[6],c[7],col); DrawLine(c[0],c[2],col); DrawLine(c[1],c[3],col); DrawLine(c[4],c[6],col); DrawLine(c[5],c[7],col); DrawLine(c[0],c[4],col); DrawLine(c[1],c[5],col); DrawLine(c[2],c[6],col); DrawLine(c[3],c[7],col); DrawVelocityVector(i);
}

static void DrawBoxCollider(u16 i) { DrawBoxColliderColored(i,ColliderColor(i)); }
void DrawSphereWireframe(Color col, ShapeSphere s) {
    float step=6.28318530f/12;
    for (int seg=0;seg<12;seg++) {
        float a0=seg*step,a1=a0+step,c0=vcosf(a0),s0=vsinf(a0),c1=vcosf(a1),s1=vsinf(a1);
        DrawLine(V3_AplusB(s.ctr,(V3){c0*s.rad,0,s0*s.rad}),V3_AplusB(s.ctr,(V3){c1*s.rad,0,s1*s.rad}),col); DrawLine(V3_AplusB(s.ctr,(V3){c0*s.rad,s0*s.rad,0}),V3_AplusB(s.ctr,(V3){c1*s.rad,s1*s.rad,0}),col); DrawLine(V3_AplusB(s.ctr,(V3){0,c0*s.rad,s0*s.rad}),V3_AplusB(s.ctr,(V3){0,c1*s.rad,s1*s.rad}),col);
    }
}

void DrawSphereCollider(u16 i) { Color col = ColliderColor(i); ShapeSphere s = Entity_GetSph(i); DrawSphereWireframe(col,s); DrawVelocityVector(i); }
void DrawSphereContact(V3 pos, float rad) { if (Cheats.showPhys) {Color col = (Color){0.0f,0.0f,1.0f,1.0f}; ShapeSphere s = (ShapeSphere){pos,rad}; DrawSphereWireframe(col,s);} }
void DrawMeshCollider(u16 i) {
    Color col = ColliderColor(i); u16 mi = (World.col[i] == COLTYPE_CVX) ? World.instances[i].colMeshIndex : World.instances[i].modelIndex; if (mi >= MAX_MDLS || mi >= mdlsCnt) return;
    u32 triCount = modelTriangleCounts[mi]; if(!triCount){return;} float M[16]; mcpy(M, &modelMatrices[i*16], 64); float m00=M[0],m10=M[1],m20=M[2],m01=M[4],m11=M[5],m21=M[6],m02=M[8],m12=M[9],m22=M[10],tx=M[12],ty=M[13],tz=M[14]; const float* pos = physPos[mi]; const u16* tris = modelTriangles[mi];
    for (u32 j=0; j<triCount; j++) { V3 w[3]; u32 b=j*3; for (int k=0;k<3;++k) { u32 vi=tris[b + k]; float x=pos[vi*3 + 0]; float y=pos[vi*3 + 1]; float z=pos[vi*3 + 2]; w[k]=(V3){m00*x + m01*y + m02*z + tx, m10*x + m11*y + m12*z + ty, m20*x + m21*y + m22*z + tz}; } DrawLine(w[0],w[1],col); DrawLine(w[1],w[2],col); DrawLine(w[2],w[0],col); }
    DrawVelocityVector(i);
}

void DrawCapsuleCollider(u16 i) {
    Color col = ColliderColor(i); ShapeCapsule cap = Entity_GetCap(i); V3 diff = V3_AsubB(cap.tip, cap.base); V3 axis = (vabs(diff.x) + vabs(diff.y) + vabs(diff.z) > 0.0001f) ? V3_Normalize(diff) : (V3){0.0f, 1.0f, 0.0f};     
    V3 ref = (vabs(axis.y) < 0.9f) ? (V3){0,1,0} : (V3){1,0,0}; V3 perp0 = V3_Normalize(V3_Cross(axis, ref)); V3 perp1 = V3_Cross(axis, perp0); float step = 6.28318530f / 12, r = cap.rad;
    for (int seg=0;seg<12;++seg) { // Draw top and bottom rings
        float a0=seg * step, a1=a0 + step; float c0=vcosf(a0), s0=vsinf(a0), c1=vcosf(a1), s1=vsinf(a1); V3 r0=V3_AplusB(V3_ScaleByF(perp0,c0*r),V3_ScaleByF(perp1,s0*r)); V3 r1=V3_AplusB(V3_ScaleByF(perp0,c1*r),V3_ScaleByF(perp1,s1*r));
        DrawLine(V3_AplusB(cap.base,r0),V3_AplusB(cap.base,r1),col); DrawLine(V3_AplusB(cap.tip,r0),V3_AplusB(cap.tip,r1),col);
    }
    for (int seg=0;seg<6;++seg) { // Draw the hemispheres
        float a0=seg*step, a1=a0+step; float c0=vcosf(a0), s0=vsinf(a0), c1=vcosf(a1), s1=vsinf(a1);
        DrawLine(V3_AplusB(cap.base,V3_AplusB(V3_ScaleByF(perp0,c0*r),V3_ScaleByF(axis,-s0*r))),V3_AplusB(cap.base,V3_AplusB(V3_ScaleByF(perp0,c1*r),V3_ScaleByF(axis,-s1*r))),col); 
        DrawLine(V3_AplusB(cap.base,V3_AplusB(V3_ScaleByF(perp1,c0*r),V3_ScaleByF(axis,-s0*r))),V3_AplusB(cap.base,V3_AplusB(V3_ScaleByF(perp1,c1*r),V3_ScaleByF(axis,-s1*r))),col);
        DrawLine(V3_AplusB(cap.tip,V3_AplusB(V3_ScaleByF(perp0,c0*r),V3_ScaleByF(axis,s0*r))),V3_AplusB(cap.tip,V3_AplusB(V3_ScaleByF(perp0,c1*r),V3_ScaleByF(axis,s1*r))),col); 
        DrawLine(V3_AplusB(cap.tip,V3_AplusB(V3_ScaleByF(perp1,c0*r),V3_ScaleByF(axis,s0*r))),V3_AplusB(cap.tip,V3_AplusB(V3_ScaleByF(perp1,c1*r),V3_ScaleByF(axis,s1*r))),col);
    }
    for(int seg=0;seg<4;++seg){float a=seg*(6.28318530f / 4.f); V3 off=V3_AplusB(V3_ScaleByF(perp0,vcosf(a)*r),V3_ScaleByF(perp1,vsinf(a)*r)); DrawLine(V3_AplusB(cap.base,off),V3_AplusB(cap.tip,off),col); } // Draw the longitudinal lines
    DrawVelocityVector(i);
}

void DrawAngularVelocity(u16 i) {
    if (!(World.instances[i].entflags & EF_RIGIDBODY) || V3_Mag(World.angularVelocity[i]) < 0.0001f) return; // skip near-zero
    Color purple = (Color){0.5f,0.0f,1.0f,1.0f}; V3 dir=V3_Normalize(World.angularVelocity[i]); V3 tip=V3_AplusB(World.position[i],V3_ScaleByF(World.angularVelocity[i],0.35f)); DrawLine(World.position[i],tip,purple); // Arrow (line vector)
    V3 ref=(vabs(dir.y) < 0.9f) ? (V3){0,1,0} : (V3){1,0,0}; V3 perp=V3_Normalize(V3_Cross(dir,ref)); V3 perp2 = V3_Cross(dir,perp);
    DrawLine(V3_AplusB(tip,V3_ScaleByF(perp, 0.05f)),V3_AplusB(tip,V3_ScaleByF(perp, -0.05f)), purple); // Small cross at tip so zero-length vectors are still visible
    DrawLine(V3_AplusB(tip,V3_ScaleByF(perp2,0.05f)),V3_AplusB(tip,V3_ScaleByF(perp2,-0.05f)), purple);
    float rad=0.6f; /*Quarter circle arc (visualizes rotation plane + sense)*/ float step = 1.57079632679f / 8.0f; /*quarter circle divided into 8 segments*/
    V3 axis=dir; V3 p1=V3_Normalize(V3_Cross(axis,ref)); V3 p2=V3_Cross(axis,p1); V3 prev = V3_AplusB(World.position[i], V3_ScaleByF(p1,rad)); // Find two vectors perpendicular to angular axis
    for (int j=1;j<=8;++j) { float a = j * step; float c = vcosf(a); float s = vsinf(a); V3 cur = V3_AplusB(World.position[i],V3_AplusB(V3_ScaleByF(p1,c * rad),V3_ScaleByF(p2,s * rad))); DrawLine(prev,cur,purple); prev = cur; }
}
#include "winput.c"
// Console System - CHEATS!
static i32 currentEntryLength=0, numHistory=0, historyPos=0; char consoleEntryText[T_BUFFER_SIZE],history[7][T_BUFFER_SIZE];
V3 ressurectionLocations[10] = {{-27.386f,-54.488f,26.5941f}/*0/R*/, {40.903f,-41.372f,-30.78f}/*1*/, {30.67407f,-24.832f,10.21412f}/*2*/, {38.26813f,-14.498f,20.37825f}/*3*/, {-19.48f,-6.928f,22.954f}/*4*/, {-24.358f,13.5956f,31.8497f}/*5*/,{-22.3568f,34.7845f,-30.728f}/*6*/,  {2.228084f,51.95243f,7.532025f}/*7*/, {10.068f,59.897f,13.973f}/*8*/, {2.303f,107.77f,-38.554f}/*9*/};
static V3 cyberSpaceEntryLocations[8] = {{210.6834f,2.812f,-24.378f}/*0*/, {195.42f,-13.44f, 33.28f}/*1*/, {157.1608f,-15.53f,47.331f}/*2a, if cyberport localPosition.x < -26.0f*/, {256.0416f,-0.716f,62.48789f}/*2b level 2 secondary cyberport position*/,{126.43f,29.56733f,34.24f}/*5*/, {177.612f,3.29494f,108.7725f}/*6*/, {244.735f,41.99257f,-19.695f}/*8*/, {185.161f,84.502f,-46.04246f},/*9*/ };
static void AddToHistory(const char* entry) { if (slen(entry) == 0 || (numHistory > 0 && sEqual(entry,history[numHistory - 1]))){return;} if (numHistory < 7) { scpy_to_a_from_b(history[numHistory],entry,T_BUFFER_SIZE); numHistory++; } else { for (int i = 0; i < 7 - 1; i++) {scpy_to_a_from_b(history[i],history[i + 1],T_BUFFER_SIZE);/*Shift list toward 0*/} scpy_to_a_from_b(history[7 - 1],entry,T_BUFFER_SIZE); } }
void RecallHistory(int direction) { // direction 1 up (older), -1 down (newer)
    if (direction == 1) { if (historyPos > 0) { historyPos--; scpy_to_a_from_b(consoleEntryText,history[historyPos],T_BUFFER_SIZE); currentEntryLength = slen(consoleEntryText); } } // up
    else if (direction == -1) { if (historyPos < numHistory) { historyPos++; if (historyPos == numHistory) { consoleEntryText[0] = currentEntryLength = 0; } else { scpy_to_a_from_b(consoleEntryText,history[historyPos],T_BUFFER_SIZE); currentEntryLength = slen(consoleEntryText); } } } // down
}

typedef void (*ConsoleCmdFuncNoArg)(); typedef void (*ConsoleCmdFuncInt)(int); typedef void (*ConsoleCmdFuncStr)(const char*);
typedef struct { const char* name; union {ConsoleCmdFuncNoArg noArg; ConsoleCmdFuncInt withInt; ConsoleCmdFuncStr withStr; void* raw;} func; enum {NOARG,CMD_INT,CMD_STR}type;} ConsoleCommand;
int CommandMatch(const char* in, const char* cmd) { while (*cmd && *in) { char c1 = c2Lower((u8)*in++); char c2 = c2Lower((u8)*cmd++); if (c1 == ' ' || c1 == '_') {c1 = ' ';} if (c2 == ' ' || c2 == '_') {c2 = ' ';} if (c1 != c2) {return 0;} } return *cmd == '\0' && (*in == '\0' || cEmpty((u8)*in) || *in == '_'); }
void cmd_noclip() { Cheats.noclip = !Cheats.noclip; if (Cheats.noclip) { World.velocity[PLAYER1] = (V3){ 0.0f, 0.0f, 0.0f }; CenterStatusPrint("noclip: %s", Sys_Text.stringTable[1000]); /*"ACTIVATED"*/} else {CenterStatusPrint("noclip: %s", Sys_Text.stringTable[717]); /*"DISABLED"*/} }
void cmd_showphys() { Cheats.showPhys = !Cheats.showPhys; if (Cheats.showPhys) { debugLineVerts = (DebugLineVertex*)OS_Alloc((size_t)MAX_WIRELINE_VRTS * 2 * sizeof(DebugLineVertex)); DebugRAM("showPhys ON"); CenterStatusPrint("showPhys: %s", Sys_Text.stringTable[1000]); /*"ACTIVATED"*/ } else { OS_Free(debugLineVerts, (size_t)MAX_WIRELINE_VRTS * 2 * sizeof(DebugLineVertex)); debugLineVerts = NULL; DebugRAM("showPhys OFF"); CenterStatusPrint("showPhys: %s", Sys_Text.stringTable[717]); /*"DISABLED"*/ } }
void cmd_shownpc() { Cheats.showNPC = !Cheats.showNPC; if (Cheats.showPhys || Cheats.showNPC) { if (!debugLineVerts) { debugLineVerts = (DebugLineVertex*)OS_Alloc((size_t)MAX_WIRELINE_VRTS * 2 * sizeof(DebugLineVertex)); DebugRAM("showNPC ON"); } } else { if (debugLineVerts) { OS_Free(debugLineVerts, (size_t)MAX_WIRELINE_VRTS * 2 * sizeof(DebugLineVertex)); debugLineVerts = NULL; DebugRAM("showNPC OFF"); } } CenterStatusPrint("shownpc: %s", Cheats.showNPC ? Sys_Text.stringTable[1000] : Sys_Text.stringTable[717]); }
void EnableCheatArsenal(u8 level) {
    switch(level) {
        case 1: // pipe, dartgun, pistol, sparqbeam, stungun, ammo tranq, ammo tranq, ammo needle, ammo needle, ammo needle, ammo standard, battery, battery, berserk, stami, medi, medi, navunit, system, ereader
        case 2: // card std, pipe, dartgun, pistol, sparqbeam, tranq, needle, needle, needle, standard, battery, battery, berserk, stami, medi, medi, navunit, system, ereader, standard, tefl, standard, grenfrag, grengas
        case 3: // card std, card eng, card sci, dartgun, pistol, sparqbeam, needle, needle, needle, standard, battery, battery, berserk, stami, medi, medi, navunit, system, ereader, standard, teflon, standard, grenfrag, grengas, grenfrag, teflon, standard, grenmine
        case 4: case 5: // flechette, card eng, card sci, card std, rapier, dartgun, pistol, sparqbeam, needle, needle, needle, standard, battery, battery, berserk, stami, medi, medi, navunit, system, ereader, standard, teflon, standard, grenfrag, grengas, grenfrag, teflon, standard, grenmine, hornet, splinter, hornet
        case 6: // flechetter, magnum, card eng, card sci, card std, rapier, pistol, sparqbeam, standard, battery, battery, grenconc, medi, medi, navunit, system, ereader, standard, teflon, standard, grenfrag, grenfrag, teflon, hollow, standard, grenmine, hornet, splinter, hornet, hollow
        case 7: // flechetter, magnum, magpulse, shield, card eng, card sci, card std, grenemp, rapier, pistol, battery, battery, grenconc, medi, blaster, medi, magcart, navunit, system, ereader, teflon, standard, grenfrag, grenfrag, hollow, hornet, splinter, hornet, hollow, battery, hollow, hornet, grenconc, grenemp
        case 8: // skorpion, slaglarge, slag, flechette, magnum, mk3, magpulse, shield, card eng, card sci, card std, grenemp, rapier, ionrifle, grenconc, medi, medi, magcart, navunit, system, ereader, grenmine, grenearth, grenfrag, grenfrag, hollow, grennitro, icad, splinter, hollow, magnesium, hollow, hornet, grenconc, grenemp, slag, slug, icad, grenconc, grenmine, grenmine, grenmine, grenmine, grenearth, grennitro, magnesium, magnesium, slug, slug
        case 9: break; // skorpion, slaglarge, slag, magnum, mk3, magpulse, shield, grenemp, rapier, ionrifle, grenconc, medi, medi, magcart, navunit, ereader, grenmine, grenearth, grenfrag, grenfrag, hollow, grennitro, icad, hollow, magnesium, hollow, healthkit, grenconc, grenemp, slag, icad, grenearth, grennitro, magnesium, magnesium, slug, magcart, plasma, magcart, medi, icad, healthkit
    }
} // TODO
void cmd_kill() { World.instances[PLAYER1].health = World.instances[PLAYER1].cyberHealth = 0.0f; CenterStatusPrint("%s", Sys_Text.stringTable[1011]); } // "Player decides to become a cyborg."
void cmd_undo() { if (Cheats.editMode) { if (lastSpawned < U16_MAX && lastSpawned >= INSTS_1ST_IDX) { DeleteInstance(lastSpawned); lastSpawned = U16_MAX; CenterStatusPrint("Last spawned object removed"); } else { CenterStatusPrint("Nothing to undo"); } } else { CenterStatusPrint("Cannot undo when not in Edit Mode"); } }
void ScreenShake(float force, double duration) { World.shakeFinished = World.pauseRelativeTime + duration; float shakeForce = (force < 0.48f) ? force : 0.48f; (void)shakeForce; } // TODO actually shake
void Shake(float force) { float forc = (force <= 0.0f) ? 1.0f : force; ScreenShake(forc,1.0); }// The whole station is a shakin' and a movin'!
void cmd_shake() { Shake(-1.0f); CenterStatusPrint("SHAKIN LIKE A LEAF!"); }
void cmd_edit() { Cheats.editMode = !Cheats.editMode; if (Cheats.editMode) { Cheats.noclip=Cheats.notarget=true; CenterStatusPrint("edit mode: %s","Edit Mode activated!"); } else { Cheats.noclip=Cheats.notarget=false; CenterStatusPrint("%s","Edit Mode deactivated"); } }
int ParseLevelArg(const char* arg) {
    if (!arg || !*arg) return -1;
    char clean[64] = {0}; int j = 0; for (int i = 0; arg[i] && j < 60; i++) { if (arg[i] != ' ' && arg[i] != '_') clean[j++] = c2Lower((u8)arg[i]); }   clean[j] = '\0';
    if (sEqual(clean,"r")|| sFindSub(clean, "reactor")){return 0;} if (sFindSub(clean,"g1") || sFindSub(clean,"10")){return 10;} if (sFindSub(clean,"g2") || sFindSub(clean,"11")){return 11;} if (sFindSub(clean,"g4") || sFindSub(clean,"12")){return 12;}
    if (sFindSub(clean, "g3")) { CenterStatusPrint("%s", Sys_Text.stringTable[1001]); return -2; }// "Gamma grove already jettisoned! Those poor arrogant people."
    int level = s2i32(clean); if (level >= 0 && level < World.numLevels) return level;
    return -1; // Invalid
}

u8 queuedLevelToLoad = 255u; V3 queuedLevelPos;
static void cmd_loadlevel(const char* arg) {
    if (World.menuActive) { CenterStatusPrint("%s", Sys_Text.stringTable[1015]); return; } // "Cannot load levels via cheat while on the main menu!"
    int level=ParseLevelArg(arg); if(level == -2){return;/*Already printed g3 message*/} if(level < 0 || level > 12){CenterStatusPrint("cmd_loadlevel invalid level argument %d",level); return;}
    CenterStatusPrint("Loading level %u",level); queuedLevelToLoad=level; queuedLevelPos=(level == 13) ? cyberSpaceEntryLocations[World.currentLevel < 8 ? (u8)World.currentLevel : 0] : ressurectionLocations[level > 9 ? 6 : level]; LoadLevel(level,queuedLevelPos);
}

static void cmd_loadarsenal(const char* arg) { int level = ParseLevelArg(arg); if (level >= 0 && level < World.numLevels) { EnableCheatArsenal(level); } }
static void cmd_summon(int itemConstIndex) { if (IdxInBounds(itemConstIndex)) { u16 spawned = SpawnDynamicObject(itemConstIndex,true); if (spawned < U16_MAX) { lastSpawned = spawned; } CenterStatusPrint("Summoned object ID %d",itemConstIndex); } else { CenterStatusPrint("Invalid object ID: %s",itemConstIndex); } }
static void cmd_notarget() { Cheats.notarget = !Cheats.notarget; CenterStatusPrint("notarget: %s", Cheats.notarget ? Sys_Text.stringTable[1000] : Sys_Text.stringTable[717]); }
static void cmd_showfps() { Cheats.showFPS = !Cheats.showFPS; }                         static void cmd_showlocation() { Cheats.showLocation = !Cheats.showLocation; }
static void cmd_help() { CenterStatusPrint("There's no one to save you now Hacker!"); } static void cmd_nomoney() { CenterStatusPrint("Nice try, there's no money here."); }
static void cmd_god() { Cheats.god = !Cheats.god; CenterStatusPrint("god mode: %s", Cheats.god ? Sys_Text.stringTable[1000] : Sys_Text.stringTable[717]); }
static void cmd_energy() { Cheats.redbull = !Cheats.redbull; if (Cheats.redbull) {CenterStatusPrint("%s", Sys_Text.stringTable[1006]);/*"I feel the power! 0 energy consumption!"*/} else {CenterStatusPrint("%s", Sys_Text.stringTable[1005]);/*Energy usage normal*/} }
static void SetSkyRotateSpeed() { static const float skyRotateSpeeds[] = { 0.05f, 1.0f, 2.5f, 3.75f, 6.25f }; glUseProgram(imageBlitSP); glUniform1f(30,skyRotateSpeeds[Cheats.dizzyLevel]); }
static void cmd_dizzy() { Cheats.dizzyLevel = (Cheats.dizzyLevel >= 3) ? 0 : Cheats.dizzyLevel + 1; SetSkyRotateSpeed(); }
static void cmd_bottomless() { Cheats.bottomless = !Cheats.bottomless; if (Cheats.bottomless) {CenterStatusPrint("bottomlessclip! %s",Sys_Text.stringTable[1002]);/*"Bring it!"*/} else {CenterStatusPrint("%s",Sys_Text.stringTable[1003]);/*"Hose disconnected from interdimensional wormhole. Normal ammo operation restored."*/} }
static void cmd_animtest() { Cheats.animTest++; if (Cheats.animTest > 2) {Cheats.animTest = 0;} if (Cheats.animTest == 1) {CenterStatusPrint("animation test looping enabled!");} else if (Cheats.animTest == 2) {CenterStatusPrint("animation test step enabled! Press 1");} else {CenterStatusPrint("animation test disabled");} }
static void cmd_nohud() { Cheats.noHUD = !Cheats.noHUD; if (Cheats.noHUD) {CenterStatusPrint("%s",Sys_Text.stringTable[1004]);/*"No HUD! Enjoy the cinematic screenshot experience!"*/} else { CenterStatusPrint("HUD %s",Sys_Text.stringTable[1000]);/*"ACTIVATED"*/} }
static void cmd_iamshodan() { Cheats.superoverride = !Cheats.superoverride; if (Cheats.superoverride) {CenterStatusPrint("%s",Sys_Text.stringTable[1010]);/*"Full security override enabled!"*/ } else {CenterStatusPrint("%s",Sys_Text.stringTable[1009]);/*"SHODAN has regained control of security from you"*/} }
static void cmd_staminup() { Cheats.fatigueCheat = !Cheats.fatigueCheat; if (Cheats.fatigueCheat) { CenterStatusPrint("Stamin-Up! %s",Sys_Text.stringTable[1013]); World.invP1.fatigue=0.0f; } else {CenterStatusPrint("%s",Sys_Text.stringTable[1012]); } }
static void cmd_qb_set(const char* arg) { if(!arg||!*arg){CenterStatusPrint("Usage: qb_set <0-%d>",QB_COUNT-1);return;} int qb=s2i32(arg); if(qb<0||qb>=QB_COUNT){CenterStatusPrint("Invalid quest bit: %d (valid 0-%d)",qb,QB_COUNT-1);return;} QuestBitSet((u8)qb); CenterStatusPrint("Quest bit %d SET -> %u",qb,QuestBitIsSet((u8)qb)); }
static void cmd_qb_clear(const char* arg) { if(!arg||!*arg){CenterStatusPrint("Usage: qb_clear <0-%d>",QB_COUNT-1);return;} int qb=s2i32(arg); if(qb<0||qb>=QB_COUNT){CenterStatusPrint("Invalid quest bit: %d (valid 0-%d)",qb,QB_COUNT-1);return;} QuestBitClear((u8)qb); CenterStatusPrint("Quest bit %d CLEARED -> %u",qb,QuestBitIsSet((u8)qb)); }
static void cmd_qb_toggle(const char* arg) { if(!arg||!*arg){CenterStatusPrint("Usage: qb_toggle <0-%d>",QB_COUNT-1);return;} int qb=s2i32(arg); if(qb<0||qb>=QB_COUNT){CenterStatusPrint("Invalid quest bit: %d (valid 0-%d)",qb,QB_COUNT-1);return;} QuestBitToggle((u8)qb); CenterStatusPrint("Quest bit %d TOGGLED -> %u",qb,QuestBitIsSet((u8)qb)); }
static void cmd_qb_list() { CenterStatusPrint("missionBits: 0x%08X",(unsigned)World.missionBits); for(int i=0;i<QB_COUNT;++i){bool q=QuestBitIsSet((u8)i); CenterStatusPrint("  [%2d] %s = %u  note:%s  chk:%s",i,q?"ON ":"OFF",q,World.questNotesActive[i]?"yes":"no ",World.questNotesChecked[i]?"yes":"no");} }
static void cmd_mrbean()  { CenterStatusPrint("Nice try, there are no go carts to slow down here"); } static void cmd_simonfoster()   { CenterStatusPrint("Nice try, nothing to paint here"); } static void cmd_richardbranson() { CenterStatusPrint("Nice try, there's no money here. You do realize this isn't Rollercoaster Tycoon right?"); } static void cmd_johnwardley()       { CenterStatusPrint("WOW!"); }
static void cmd_johnmace(){ CenterStatusPrint("Nice try, there's nothing to pay double for here"); }  static void cmd_melaniewarn()   { CenterStatusPrint("I feel happy!!!"); }                 static void cmd_damonhill()      { CenterStatusPrint("Nice try, there are no go carts to speed up here"); }                                       static void cmd_michaelschumacher() { CenterStatusPrint("Nice try, there are no go carts to give ludicrous speed here"); }
static void cmd_tonyday() { CenterStatusPrint("Ok, now I want a hamburger"); }                        static void cmd_katiebrayshaw() { CenterStatusPrint("Hi there! Hello! Hey! Howdy!"); }
static void cmd_sudo()    { CenterStatusPrint("Super user access granted...ERROR: access restricted by SHODAN!"); }
static void cmd_git(const char* arg) {
    if (!arg) arg = "";
    static const char* cmds[] = {"pull","remote: Enumerating objects: 24601, done.\nFailed, could not connect with origin/triop.","fetch", "remote: Enumerating objects: 24601, done.\nFailed, could not connect with origin/triop.",     "status","Your branch is up to date with origin/triop.\nWorking directory clean.",
                                 "log", "<Merge pull request #451 from SHODAN/NeuralLinkBugfix> 6 months ago...",                 "reflog","dc51440 HEAD0 -> master: commit: Establish neural connection ... ERROR: invalid ID `2-4601`.","merge", "Failed, could not connect with origin/triop.",
                                 "push","Could not find Username for 'triopttp://192.168.1.451'.",                                "clone", "Failed, connection blocked by SHODAN. Employee ID invalid." };
    for (int i = 0; i < 16; i += 2) { if (sFindSub(arg,cmds[i])) { CenterStatusPrint(cmds[i+1]); return; } }
    if(sFindSub(arg,"branch") || sFindSub(arg, "-b")){ const char *last = StringFindLastChar(arg,' '); CenterStatusPrint("Created new branch %s",last ? last + 1 : "unknown");} else {CenterStatusPrint("Branch name not recognized. Contact your TriopBucket representative.");}
}

static void cmd_restart()     { CenterStatusPrint("Yeah...better not"); }                             static void cmd_cd()          { CenterStatusPrint("Attempting to access directory... already at root"); }
static void cmd_justinbailey(){ CenterStatusPrint("Well, you don't have a suit already so..."); }     static void cmd_woodstock()   { CenterStatusPrint("How much wood could a woodchuck chuck...there's no wood in SPACE!"); }
static void cmd_zelda()       { CenterStatusPrint("Too late, already been to level 1"); }             static void cmd_quarry()      { CenterStatusPrint("There's obsidian on levels 6 and 8 if you want to feel decadent,\notherwise we are lacking in the stone department."); }
static void cmd_iamironman(){ CenterStatusPrint("That's nice dear."); }                               static void cmd_allyourbase() { CenterStatusPrint("ERROR: SHODAN has overriden your command, remove SHODAN first."); }
static void cmd_idkfa()       { CenterStatusPrint("I can only hold 7 weapons!! Nice try dearies!"); } static void cmd_ai()          { CenterStatusPrint("Only AI allowed around here is SHODAN"); }
static void cmd_quit()      { OS_Exit(0); }                                                           static void cmd_aireal()      { CenterStatusPrint("In my magnificence, I shape clay, crafting new lifeforms..."); }
static const ConsoleCommand consoleCmds[] = {
    {"noclip",         {.noArg=cmd_noclip},        NOARG},{"idclip",          {.noArg=cmd_noclip},NOARG},         {"no clip",     {.noArg = cmd_noclip},NOARG},  {"showphys",      {.noArg = cmd_showphys},NOARG},  { "god",           {.noArg=cmd_god}, NOARG},       {"overwhelming",            {.noArg=cmd_god}, NOARG}, 
    {"whosyourdaddy",  {.noArg = cmd_god},         NOARG},{"iddqd",           {.noArg=cmd_god}, NOARG},           {"notarget",    {.noArg=cmd_notarget},NOARG},  {"no target",     {.noArg = cmd_notarget},NOARG},  {"editmode",       {.noArg=cmd_edit},NOARG},       {"edit",                    {.noArg=cmd_edit},NOARG},
    {"edit mode",      {.noArg = cmd_edit},        NOARG},{"editor",          {.noArg=cmd_edit},NOARG},           {"undo",        {.noArg=cmd_undo},    NOARG},  {"showfps",       {.noArg = cmd_showfps}, NOARG},  {"show fps",       {.noArg=cmd_showfps},NOARG},    {"showlocation",            {.noArg=cmd_showlocation},NOARG},
    {"show location",  {.noArg = cmd_showlocation},NOARG},{"nohud",           {.noArg=cmd_nohud},NOARG},          {"no hud",      {.noArg=cmd_nohud},   NOARG},  {"bottomlessclip",{.noArg = cmd_bottomless},NOARG},{"bottomless clip",{.noArg=cmd_bottomless},NOARG}, {"load",                    {.withStr=cmd_loadlevel},CMD_STR},
    {"loadarsenal",    {.withStr = cmd_loadarsenal},CMD_STR},{"load arsenal", {.withStr=cmd_loadarsenal},CMD_STR},{"summon_obj",  {.withInt=cmd_summon},CMD_INT},{"summonobj",     {.withInt = cmd_summon},CMD_INT},{"motherlode",     {.noArg=cmd_nomoney},   NOARG}, {"rosebud",                 {.noArg=cmd_nomoney},NOARG},
    {"kaching",        {.noArg=cmd_nomoney},       NOARG},{"money",           {.noArg=cmd_nomoney},NOARG},        {"dizzy",       {.noArg=cmd_dizzy},   NOARG},  {"help",          {.noArg=cmd_help},        NOARG},{"ifeelthepower",  {.noArg = cmd_energy},  NOARG}, {"power",                   {.noArg=cmd_energy}, NOARG},
    {"energy",         {.noArg=cmd_energy},        NOARG},{"i feel the power",{.noArg = cmd_energy},NOARG},       {"i am shodan", {.noArg=cmd_iamshodan},NOARG}, {"iamshodan",     {.noArg=cmd_iamshodan},   NOARG},{"mr. bean",       {.noArg = cmd_mrbean},  NOARG}, {"simon foster",            {.noArg=cmd_simonfoster},NOARG},
    {"richard branson",{.noArg=cmd_richardbranson},NOARG},{"john wardley",    {.noArg = cmd_johnwardley},NOARG},  {"john mace",   {.noArg=cmd_johnmace}, NOARG}, {"melanie warn",  {.noArg=cmd_melaniewarn}, NOARG},{"damon hill",     {.noArg = cmd_damonhill},NOARG},{"michael schumacher",      {.noArg=cmd_michaelschumacher},NOARG},
    {"tony day",       {.noArg=cmd_tonyday},       NOARG},{"katie brayshaw",  {.noArg = cmd_katiebrayshaw},NOARG},{"sudo",        {.noArg=cmd_sudo},     NOARG}, {"admin",         {.noArg=cmd_sudo},        NOARG},{"git",            {.withStr=cmd_git},CMD_STR},        {"restart",             {.noArg=cmd_restart},NOARG},
    {"quit",           {.noArg=cmd_quit},          NOARG},{"exit",            {.noArg = cmd_quit},         NOARG},{"cd",          {.noArg=cmd_cd},        NOARG},{"./",            {.noArg=cmd_cd},          NOARG},{"kill",           {.noArg = cmd_kill},     NOARG},{"suicide",                 {.noArg=cmd_kill},NOARG},
    {"die",            {.noArg=cmd_kill},          NOARG},{"justinbailey",    {.noArg = cmd_justinbailey}, NOARG},{"woodstock",   {.noArg=cmd_woodstock}, NOARG},{"quarry",        {.noArg=cmd_quarry},      NOARG},{"zelda",          {.noArg = cmd_zelda},    NOARG},{"allyourbasearebelongtous",{.noArg=cmd_allyourbase},NOARG},
    {"all your base",  {.noArg=cmd_allyourbase},   NOARG},{"i am iron man",   {.noArg = cmd_iamironman},   NOARG},{"i am amazing",{.noArg=cmd_iamironman},NOARG},{"i am cool",     {.noArg=cmd_iamironman},  NOARG},{"i am best",      {.noArg =cmd_iamironman},NOARG},{"idkfa",                   {.noArg=cmd_idkfa},      NOARG},
    {"impulse 9",      {.noArg=cmd_idkfa},         NOARG},{"undo",            {.noArg = cmd_undo},         NOARG},{"shake",       {.noArg=cmd_shake},     NOARG},{"tired",         {.noArg=cmd_staminup},    NOARG},{"staminup",       {.noArg = cmd_staminup}, NOARG},{"grok",                    {.noArg=cmd_ai},         NOARG},
    {"chatgpt",        {.noArg=cmd_ai},            NOARG},{"claude",          {.noArg = cmd_ai},           NOARG},{"gemini",      {.noArg=cmd_ai},        NOARG},{"shodan",        {.noArg=cmd_aireal},      NOARG},{"animtest",       {.noArg = cmd_animtest}, NOARG},{"qb_set",                  {.withStr=cmd_qb_set},   CMD_STR},
    {"qb_clear",       {.withStr=cmd_qb_clear},  CMD_STR},{"qb_toggle",       {.withStr=cmd_qb_toggle},  CMD_STR},{"qb_list",     {.noArg=cmd_qb_list},   NOARG},{"shownpc",       {.noArg=cmd_shownpc},     NOARG},{NULL,{.raw = NULL},NOARG}/*sizeof helper*/ };
void ToggleConsole();
void ProcessConsoleCommand(const char* c) {
    if (c == NULL || slen(c) == 0) { ToggleConsole(); return; }
    char ts[T_BUFFER_SIZE]; sCpy2aSubFromb(ts,sizeof(ts)-1,c,T_BUFFER_SIZE); ts[sizeof(ts)-1] = '\0';
    const char* ct=ts; while(*ct && cEmpty((u8)*ct)){ct++;} const char* space=ct; while(*space && !cEmpty((u8)*space)){space++;} const char* arg_start=space; while(*arg_start && cEmpty((u8)*arg_start)){arg_start++;} AddToHistory(c); bool commandProcessed = false;
    for (u16 i=0;consoleCmds[i].name!=NULL;++i) {
        const ConsoleCommand* cmd = &consoleCmds[i];
        if (CommandMatch(ct,cmd->name)) {
            if (cmd->type == NOARG) {cmd->func.noArg(); commandProcessed = true; } else if (cmd->type == CMD_STR && *arg_start) { cmd->func.withStr(*arg_start ? arg_start : ""); commandProcessed = true;
            } else { if(!*arg_start){CenterStatusPrint("Missing argument, usage: %s <number>",cmd->name);}else{cmd->func.withInt(s2i32(arg_start)); commandProcessed=true;} }
        }
    }
    if (!commandProcessed){CenterStatusPrint("%s%s",Sys_Text.stringTable[1014],ct);} /*"Unknown command or function: "*/ consoleEntryText[0] = currentEntryLength = 0; historyPos = numHistory; /*Position beyond newest for empt*/ ToggleConsole();
}

void ConsoleEmulator(i32 keycode) {
    if (keycode == KEY_UP || keycode == KEY_DOWN) { RecallHistory(keycode == KEY_UP ? 1 : -1); return;/*get history*/} if (keycode == KEY_U && Sys_Input.keyStates[KEY_LEFT_CONTROL].down) { consoleEntryText[0]='\0'; currentEntryLength=0; return; } // Clear the input
         if (keycode >= KEY_A && keycode <= KEY_Z) { /*Handle alphabet keys*/ if (currentEntryLength < (T_BUFFER_SIZE - 1)) { char c = 'a' + (keycode - KEY_A); /*lowercase*/ consoleEntryText[currentEntryLength] = c; consoleEntryText[currentEntryLength + 1] = '\0'; currentEntryLength++; } }
    else if (keycode >= KEY_1 && keycode <= KEY_9) { /*Handle number keys 1-9*/ if (currentEntryLength < (T_BUFFER_SIZE - 1)) { char c = '1' + (keycode - KEY_1); /*Map to '1'-'9'*/ consoleEntryText[currentEntryLength] = c; consoleEntryText[currentEntryLength + 1] = '\0'; currentEntryLength++; } }
    else if (keycode == KEY_0) { /*Handle '0'*/ if (currentEntryLength < (T_BUFFER_SIZE - 1)) { consoleEntryText[currentEntryLength]='0'; consoleEntryText[currentEntryLength + 1]='\0'; currentEntryLength++; } }
    else if (keycode == KEY_MINUS || keycode == KEY_KP_SUBTRACT) { if (currentEntryLength < (T_BUFFER_SIZE - 1)) { consoleEntryText[currentEntryLength]=(Sys_Input.keyStates[KEY_LEFT_SHIFT].down || Sys_Input.keyStates[KEY_RIGHT_SHIFT].down) ? '_' : '-'; consoleEntryText[currentEntryLength + 1]='\0'; currentEntryLength++; } }
    else if (keycode == KEY_BACKSPACE && currentEntryLength > 0) { currentEntryLength--; consoleEntryText[currentEntryLength]='\0'; } // Handle backspace
    else if (keycode == KEY_SPACE) { /*Handle space*/ if (currentEntryLength < (T_BUFFER_SIZE - 1)) { consoleEntryText[currentEntryLength]=' '; consoleEntryText[currentEntryLength + 1]='\0'; currentEntryLength++; } }
    else if (keycode == KEY_ENTER || keycode == KEY_KP_ENTER) { DualLog("Console command: %s\n",consoleEntryText); ProcessConsoleCommand(consoleEntryText); }
}
// Raycast System
RaycastHit RayTriangle(V3 origin, V3 dir, V3 posA, V3 posB, V3 posC) {
    V3 AB=V3_AsubB(posB,posA), AC=V3_AsubB(posC,posA); V3 n=V3_Cross(AB,AC); V3 ao=V3_AsubB(origin,posA); V3 dao=V3_Cross(ao,dir);
    float det=(-V3_dot(dir,n)); float invDet=1.0f / det; float d=V3_dot(ao,n) * invDet; float u=V3_dot(AC,dao) * invDet, v=(-V3_dot(AB,dao)) * invDet; float w=1.0f - u - v;
    return (RaycastHit){.point=V3_AplusB(origin,V3_ScaleByF(dir,d)), .normal=V3_Normalize(n), .distance=d, .hitInstanceIndex=INSTANCE_COUNT, .hit=vabs(det) >= 0.00000001f && d >= 0 && u >= 0 && v >= 0 && w >= 0};
}

INLINE RaycastHit RaySphere(V3 origin, V3 dir, ShapeSphere sph, float maxDist) {
    RaycastHit h = {.hit=false,.distance=maxDist,.point={0,0,0},.normal={0,0,0},.hitInstanceIndex=INSTANCE_COUNT};
    float r = sph.rad; if (r < 0.0001f) return h;
    V3 oc = V3_AsubB(origin, sph.ctr); float b=V3_dot(oc,dir), c=V3_dot(oc,oc) - r * r; float disc=b*b - c; if (disc < 0.0f) return h;
    float s = vsqrtf(disc); float t = -b - s; if (t < 0.0f) t = -b + s; if (t < 0.0f || t > maxDist) return h;
    V3 p = V3_AplusB(origin, V3_ScaleByF(dir, t)); V3 n = V3_Normalize(V3_ScaleByF(V3_AsubB(p, sph.ctr), 1.0f / r)); h.hit = true; h.distance = t; h.point = p; h.normal = n; return h;
}

INLINE RaycastHit RayCapsule(V3 origin, V3 dir, ShapeCapsule cap, float maxDist) {
    RaycastHit h={.hit=false,.distance=maxDist,.point={0,0,0},.normal={0,0,0},.hitInstanceIndex=INSTANCE_COUNT};
    float r = cap.rad; if (r < 0.0001f) return h;
    V3 ba = V3_AsubB(cap.tip,cap.base), oa = V3_AsubB(origin,cap.base), nBest = {0,0,0}; float baba = V3_dot(ba,ba); if (baba < 0.00001f) return RaySphere(origin, dir, (ShapeSphere){cap.base, r}, maxDist);
    float bard = V3_dot(ba,dir), baoa = V3_dot(ba,oa), tBest=-1.0f; float a = baba - bard * bard, b = baba * V3_dot(dir,oa) - baoa * bard, c = baba * V3_dot(oa,oa) - baoa * baoa - r * r * baba; float disc = b * b - a * c;
    if (vabs(a) >= 0.00001f && disc >= 0.0f) {
        float sh = vsqrtf(disc); float t0 = (-b - sh) / a; float y0 = baoa + t0 * bard;
        if (t0 >= 0.0f && t0 <= maxDist && y0 > 0.0f && y0 < baba) { tBest = t0; V3 p = V3_AplusB(origin,V3_ScaleByF(dir,t0)); V3 q = V3_AplusB(cap.base,V3_ScaleByF(ba,y0/baba)); nBest = V3_Normalize(V3_AsubB(p, q)); }
        else { float t1 = (-b + sh) / a; float y1 = baoa + t1 * bard; if(t1 >= 0.0f && t1 <= maxDist && y1 > 0.0f && y1 < baba && tBest < 0.0f){tBest=t1; V3 p=V3_AplusB(origin,V3_ScaleByF(dir,t1)); V3 q=V3_AplusB(cap.base,V3_ScaleByF(ba,y1/baba)); nBest=V3_Normalize(V3_AsubB(p,q));} }
    }
    for (int k = 0; k < 2; k++) {
        V3 ctr = k == 0 ? cap.base : cap.tip;
        V3 oc = V3_AsubB(origin, ctr);
        float bs = V3_dot(oc, dir); float cs = V3_dot(oc, oc) - r * r; float ds = bs * bs - cs; if (ds < 0.0f) continue;
        float shs = vsqrtf(ds); float ts = -bs - shs; if (ts < 0.0f) ts = -bs + shs; if (ts < 0.0f || ts > maxDist) continue; if (tBest >= 0.0f && ts >= tBest) continue;
        V3 ps = V3_AplusB(origin, V3_ScaleByF(dir, ts)); float y = V3_dot(V3_AsubB(ps, cap.base), ba); if ((k == 0 && y > 0.0f) || (k == 1 && y < baba)) continue;
        tBest = ts; nBest = V3_Normalize(V3_ScaleByF(V3_AsubB(ps, ctr), 1.0f / r));
    }
    if (tBest >= 0.0f) { h.hit = true; h.distance = tBest; h.point = V3_AplusB(origin, V3_ScaleByF(dir, tBest)); h.normal = nBest; } return h;
}

float BvhRayAABBHit(V3 origin, V3 dir, V3 mn, V3 mx, float maxDist);
RaycastHit Raycast(V3 origin, V3 dir, float maxDist, u32 layerMask) {
    RaycastHit result = { .hit = false, .distance = maxDist, .point = {0.0f, 0.0f, 0.0f}, .normal = {0.0f, 0.0f, 0.0f}, .hitInstanceIndex = INSTANCE_COUNT };
    dir = V3_Normalize(dir);
    for (u16 i = 0; i < World.instCount; ++i) {
        if (!(layerMask & World.layer[i])){continue;} if (!(World.instances[i].entflags & EF_ACTIVE)){continue;}
        u16 mindex = World.instances[i].modelIndex;
        if (mindex >= MAX_MDLS) {
            ColliderType ct = World.col[i]; if (ct != COLTYPE_CAP && ct != COLTYPE_SPH) continue;
            V3 objPos = World.position[i]; float scaleMax = vmax(World.scale[i].x, vmax(World.scale[i].y, World.scale[i].z)); float boundRad = 0.0f;
            if (ct == COLTYPE_CAP) { float rad = World.colliderSize[i].x * scaleMax; float hi = vmax(0.0f, World.colliderSize[i].y * 0.5f * scaleMax - rad); boundRad = hi + rad; } else { boundRad = World.colliderSize[i].x * scaleMax; }
            boundRad = vmax(boundRad, 0.1f);
            u16 instCellIdx = PosGetCellCoords(objPos.x, objPos.z);
            if (!IdxIsPortalBlockingDoor(World.instances[i].index)) { if(((gridCellStates[instCellIdx] & (CELL_VISIBLE | CELL_OPEN)) == CELL_OPEN) && (World.instances[i].index != 754 || !SkyIsVisible())){continue;} }
            V3 delta = V3_AsubB(objPos, origin);
            float distSqrd = V3_dot(delta, delta);
            float maxDistToObj = vmax(maxDist - boundRad, maxDist); if (distSqrd >= maxDistToObj * maxDistToObj) continue;
            RaycastHit ch = {0};
            if (ct == COLTYPE_CAP) ch = RayCapsule(origin, dir, Entity_GetCap(i), result.distance); else ch = RaySphere(origin, dir, Entity_GetSph(i), result.distance);
            if (!ch.hit || ch.distance >= result.distance) continue;
            ch.hitInstanceIndex = i; result = ch; continue;
        }
        if (mindex >= mdlsCnt) continue;
        V3 objPos = World.position[i]; u16 instCellIdx = PosGetCellCoords(objPos.x,objPos.z); V3 delta = V3_AsubB(objPos,origin); float distSqrd = V3_dot(delta,delta), radBounds = vmax(modelBounds[mindex],1.81f);
        float maxDistToObj = vmax(maxDist - radBounds,maxDist); if (distSqrd >= (maxDistToObj * maxDistToObj)) continue;
        if (!IdxIsPortalBlockingDoor(World.instances[i].index)) { if(((gridCellStates[instCellIdx] & (CELL_VISIBLE | CELL_OPEN)) == CELL_OPEN) && (World.instances[i].index != 754 || !SkyIsVisible())){continue;} }
        u32 triCount = modelTriangleCounts[mindex]; if (triCount < 1) continue;
        float M[16]; mcpy(M,&modelMatrices[i * 16],16 * sizeof(float)); float m00=M[0], m10=M[1], m20=M[2], m01=M[4], m11=M[5], m21=M[6], m02=M[8], m12=M[9], m22=M[10], tx=M[12], ty=M[13], tz=M[14];
        float sclx = vsqrtf(m00*m00 + m10*m10 + m20*m20); float sclx2 = sclx * sclx; float scly = vsqrtf(m01*m01 + m11*m11 + m21*m21); float scly2 = scly * scly; float sclz = vsqrtf(m02*m02 + m12*m12 + m22*m22); float sclz2 = sclz * sclz;
        V3 rel = {origin.x - tx, origin.y - ty, origin.z - tz};
        V3 localOrigin = {(rel.x*m00 + rel.y*m10 + rel.z*m20) / sclx2, (rel.x*m01 + rel.y*m11 + rel.z*m21) / scly2, (rel.x*m02 + rel.y*m12 + rel.z*m22) / sclz2};
        V3 localDir =    {(dir.x*m00 + dir.y*m10 + dir.z*m20) / sclx2, (dir.x*m01 + dir.y*m11 + dir.z*m21) / scly2, (dir.x*m02 + dir.y*m12 + dir.z*m22) / sclz2};
        localDir = V3_Normalize(localDir); const float* posPtr = physPos[mindex]; const u16* tris = physTris[mindex];
        if (BvhHasBVH(mindex)) {
            const BvhNode* nodes = modelBVHNodes[mindex]; const u16* triOrder = modelBVHTriOrder[mindex];
            float minScale = vmin(sclx, vmin(scly,sclz)); if(minScale < 0.0001f){minScale=0.0001f;} float localMax=maxDist/minScale; float bestT=localMax; const BvhNode* stack[64]; int sp = 0; stack[sp++] = &nodes[0];
            while (sp > 0) {
                const BvhNode* node = stack[--sp];
                float tEntry = BvhRayAABBHit(localOrigin, localDir, node->mn, node->mx, bestT);
                if (tEntry < 0.0f) continue;
                if (node->triCount > 0) {
                    for (u32 k=0;k<node->triCount;k++) {
                        u32 base = triOrder[node->triStart + k] * 3;
                        u32 iA=tris[base + 0], iB=tris[base + 1], iC=tris[base + 2];
                        V3 posA = {posPtr[iA*3],posPtr[iA*3+1],posPtr[iA*3+2]}, posB={posPtr[iB*3],posPtr[iB*3+1],posPtr[iB*3+2]}, posC={posPtr[iC*3],posPtr[iC*3+1],posPtr[iC*3+2]};
                        RaycastHit tryTri = RayTriangle(localOrigin,localDir,posA,posB,posC); if (!tryTri.hit) continue;
                        V3 worldPoint = { m00*tryTri.point.x + m01*tryTri.point.y + m02*tryTri.point.z + tx, m10*tryTri.point.x + m11*tryTri.point.y + m12*tryTri.point.z + ty, m20*tryTri.point.x + m21*tryTri.point.y + m22*tryTri.point.z + tz };
                        float worldDist = V3_Dist(worldPoint,origin); if (worldDist >= result.distance) continue;
                        V3 worldNormal={(m00/sclx)*tryTri.normal.x + (m01/scly)*tryTri.normal.y + (m02/sclz)*tryTri.normal.z,(m10/sclx)*tryTri.normal.x + (m11/scly)*tryTri.normal.y + (m12/sclz)*tryTri.normal.z,(m20/sclx)*tryTri.normal.x + (m21/scly)*tryTri.normal.y + (m22/sclz)*tryTri.normal.z };
                        worldNormal = V3_Normalize(worldNormal); result.hit=true; result.point=worldPoint; result.normal=V3_Normalize(worldNormal); result.distance=worldDist; result.hitInstanceIndex=i; bestT = tryTri.distance;
                    }
                } else {  for(int o=0;o<8&&sp<64;++o) { if(node->children[o] >= 0){stack[sp++]=&nodes[node->children[o]];} }  }
            } continue;
        } DualLogError("Missing bvh for %u!!\n",mindex); OS_Exit(1);
    } return result;
}
// Credits Sys
char creditStats[4096];
INLINE float GetScore(float stupid, bool isFinal) { float v=(float)(World.kills + World.cyberkills); if (isFinal) {v -= vmin(World.ressurections*10.0f,v*0.666f);} float s=vfloor((float)World.pauseRelativeTime / 3600.0f), score=v*10000.0f; score -= vmin(score*0.666f,s*100.0f); score *= (stupid + 1.0f) / 37.0f; if (stupid > 35.0f) {score += 2222222.0f;} return vfloor(score); }
INLINE void DecomposeTime(double t, u32* h, u32* m, double* s) { double tb = vfloor(t / 3600.0); *h = (u32)tb; t -= tb * 3600.0; tb = vfloor(t / 60.0); *m = (u32)tb; *s = t - tb * 60.0; }
INLINE void CreditsStats() {
    size_t off = 0; u32 h,m; double s;
    off += sFormat(creditStats + off, sizeof(creditStats)-off,"============================================================================\nCITADEL\n============================================================================\nCONGRATULATIONS %s\n",World.playerName);
    DecomposeTime(World.pauseRelativeTime,&h,&m,&s); off += sFormat(creditStats + off, sizeof(creditStats)-off,"Straight Time: %uh %um %.3fs\n",h,m,s);
    DecomposeTime(World.absoluteTime,&h,&m,&s);      off += sFormat(creditStats + off,sizeof(creditStats)-off,"Total Time (with reload from deaths): %uh %um %.3fs\n",h,m,s);
    float stupid = ((float)(World.diffCbt * World.diffCbt)) + ((float)(World.diffPuz * World.diffPuz)) + ((float)(World.diffMis * World.diffMis)) + ((float)(World.diffCyb * World.diffCyb)); u32 finalSubscore = GetScore(stupid,false), finalScore = (u32)GetScore(stupid,true);
    off += sFormat(creditStats + off,sizeof(creditStats)-off,"Kills: %u\nKills in Cyberspace: %u\nScoreSubtotal: %u\nDeaths: %u\nRessurections: %u\n",World.kills,World.cyberkills,(u32)finalSubscore,World.deaths,World.ressurections);
    off += sFormat(creditStats + off,sizeof(creditStats)-off,"Combat: %u | Puzzle: %u | Mission: %u | Cyber: %u\n",World.diffCbt,World.diffPuz,World.diffMis,World.diffCyb);
    off += sFormat(creditStats + off,sizeof(creditStats)-off,"Difficulty Index: %.2f\nFinal Score: %u\n\n",stupid,finalScore);
    off += sFormat(creditStats + off,sizeof(creditStats)-off,"Shots Fired: %u\nGrenades Thrown: %u\n",World.shotsFired,World.grenadesThrown);
    off += sFormat(creditStats + off,sizeof(creditStats)-off,"Damage Dealt: %f\nDamage Received: %f\nSaves Scummed: %u\n\nClick to continue...\n",World.damageDealt,World.damageReceived,World.savesScummed);
}
// Rendering Sys
INLINE void ShaderError(u32 s, const char* name) { char er[512]; glGetShaderInfoLog(s,512,NULL,er); DualLogError("%s Comp Fail: %s\n",name,er); OS_Exit(1); }
INLINE u32 CompileShader(u32 type, const char* source, const char* name) { u32 s = glCreateShader(type); glShaderSource(s,1,&source,NULL); glCompileShader(s); i32 ok; glGetShaderiv(s,0x8B81/*GL_COMPILE_STATUS*/,&ok); if (!ok) ShaderError(s,name); return s; }
INLINE u32 LinkProgram(u32* s, i32 num, const char* name) { u32 p = glCreateProgram(); for (i32 i=0;i<num;++i) { glAttachShader(p,s[i]); } glLinkProgram(p); i32 ok; glGetProgramiv(p,0x8B82/*GL_LINK_STATUS*/,&ok); if (!ok) ShaderError(p,name); return p; }
u32 CompileAnyShader(const char* v, const char* s, const char* name) { return (v) ? LinkProgram((u32[]){CompileShader(0x8B31/*GL_VERTEX_SHADER*/,v,name),CompileShader(0x8B30/*GL_FRAGMENT_SHADER*/,s,name)},2,name) : LinkProgram((u32[]){CompileShader(0x91B9/*GL_COMPUTE_SHADER*/,s,name)},1,name); }
void CompileShaders() {
    depthPrepassSP=CompileAnyShader(depthPrepassVertSrc,depthPrepassFragSrc,"DPre"); chunkSP=CompileAnyShader(vertSrc,fragSrc,"Main"); uiSP=CompileAnyShader(vertUISrc,fragUISrc,"UI"); debugUnlitSP=CompileAnyShader(debugUnlitVertSrc,debugUnlitFragSrc,"Ln");
    shadowmapsSP=CompileAnyShader(shadowmapVertSrc,shadowmapFragSrc,"Shad"); textSP=CompileAnyShader(textVertSrc,textFragSrc,"Txt"); imageBlitSP=CompileAnyShader(quadVertSrc,quadFragSrc,"Comp"); ssrSP=CompileAnyShader(NULL,ssrCSSrc,"SSR");
    voxelUpdateSP=CompileAnyShader(NULL,voxUpdCSSrc,"Vox"); shadowmapsClearSP=CompileAnyShader(NULL,shadClearCSSrc,"ShadCl");
}

INLINE u32 MakeSSBO(u32* id, u32 bindx, size_t sz, const void* d, u32 typ) { glGenBuffers(1,id); glBindBuffer(GL_SSBO,*id); glBufferData(GL_SSBO,sz,d,typ); glBindBufferBase(GL_SSBO,bindx,*id); return *id; }
static void mat4_lookat_from(float* m, Quaternion* camRotation, V3 eye) { // Kept around for light views for shadowmap cubemap faces.
    float x=camRotation->x, y=camRotation->y, z=camRotation->z, w=camRotation->w;
    float x2=x*x, y2=y*y, z2=z*z; float xy=x*y, xz=x*z, yz=y*z; float wx=w*x, wy=w*y, wz=w*z;
    V3 right={1.0f - 2.0f*(y2 + z2),2.0f*(xy + wz),2.0f*(xz - wy)};/*X+(right)*/ V3 up={2.0f*(xy - wz),1.0f - 2.0f*(x2 + z2),2.0f*(yz + wx)};/*Y+(up)*/ V3 forward={2.0f*(xz + wy),2.0f*(yz - wx), 1.0f - 2.0f*(x2 + y2)};/*Z+(forward)*/
    m[0]=right.x; m[1]=up.x; m[2]=-forward.x; m[3]=0.0f; m[4]=right.y; m[5]=up.y; m[6]=-forward.y; m[7]=0.0f; m[8]=right.z; m[9]=up.z; m[10]=-forward.z; m[11]=0.0f; m[12]=-V3_dot(right,eye); m[13]=-V3_dot(up,eye); m[14]=V3_dot(forward,eye); m[15]=1.0f;
}

INLINE bool SphereInFrustum(FrustumPlane* ps, V3 c, float radius) { for (int i=0;i<6;++i) { if ((V3_dot(ps[i].normal,c) + ps[i].d) < -radius) return false; } return true; }
void ExtractFrustumPlanes(float* m, FrustumPlane* ps) {
    ps[0].normal.x = m[3] + m[0]; ps[0].normal.y = m[7] + m[4]; ps[0].normal.z = m[11] + m[8];  ps[0].d = m[15] + m[12]; // Left
    ps[1].normal.x = m[3] - m[0]; ps[1].normal.y = m[7] - m[4]; ps[1].normal.z = m[11] - m[8];  ps[1].d = m[15] - m[12]; // Right
    ps[2].normal.x = m[3] + m[1]; ps[2].normal.y = m[7] + m[5]; ps[2].normal.z = m[11] + m[9];  ps[2].d = m[15] + m[13]; // Bottom
    ps[3].normal.x = m[3] - m[1]; ps[3].normal.y = m[7] - m[5]; ps[3].normal.z = m[11] - m[9];  ps[3].d = m[15] - m[13]; // Top
    ps[4].normal.x = m[3] + m[2]; ps[4].normal.y = m[7] + m[6]; ps[4].normal.z = m[11] + m[10]; ps[4].d = m[15] + m[14]; // Near
    ps[5].normal.x = m[3] - m[2]; ps[5].normal.y = m[7] - m[6]; ps[5].normal.z = m[11] - m[10]; ps[5].d = m[15] - m[14]; // Far
    for (int i=0;i<6;i++) { float len = V3_Mag(ps[i].normal); if(len > 1e-6f){ps[i].normal.x /= len; ps[i].normal.y /= len; ps[i].normal.z /= len; ps[i].d /= len;} } //Normalize (could use V3_Normalize but need len for d term of FrustumPlane).
}

void mul_mat4(float *out, const float *a, const float *b) { // out = a * b
    out[0] =  a[0] * b[0]  + a[4] * b[1]  + a[8]  * b[2] + a[12]  * b[3]; out[1] =  a[1] * b[0]  + a[5] * b[1]  + a[9]  * b[2] + a[13]  * b[3];
    out[2] =  a[2] * b[0]  + a[6] * b[1] + a[10]  * b[2] + a[14]  * b[3]; out[3] =  a[3] * b[0]  + a[7] * b[1] + a[11]  * b[2] + a[15]  * b[3];
    out[4] =  a[0] * b[4]  + a[4] * b[5]  + a[8]  * b[6] + a[12]  * b[7]; out[5] =  a[1] * b[4]  + a[5] * b[5]  + a[9]  * b[6] + a[13]  * b[7];
    out[6] =  a[2] * b[4]  + a[6] * b[5] + a[10]  * b[6] + a[14]  * b[7]; out[7] =  a[3] * b[4]  + a[7] * b[5] + a[11]  * b[6] + a[15]  * b[7];
    out[8] =  a[0] * b[8]  + a[4] * b[9]  + a[8] * b[10] + a[12] * b[11]; out[9] =  a[1] * b[8]  + a[5] * b[9]  + a[9] * b[10] + a[13] * b[11];
    out[10] = a[2] * b[8]  + a[6] * b[9] + a[10] * b[10] + a[14] * b[11]; out[11] = a[3] * b[8]  + a[7] * b[9] + a[11] * b[10] + a[15] * b[11];
    out[12] = a[0] * b[12] + a[4] * b[13] + a[8] * b[14] + a[12] * b[15]; out[13] = a[1] * b[12] + a[5] * b[13] + a[9] * b[14] + a[13] * b[15];
    out[14] = a[2] * b[12] + a[6] * b[13] + a[10]* b[14] + a[14] * b[15]; out[15] = a[3] * b[12] + a[7] * b[13] + a[11]* b[14] + a[15] * b[15];
}

__attribute__((noinline)) void RenderUIImage(i16 x, i16 y, i16 width, i16 height, u32 texIndex) {
    glUseProgram(uiSP); glDisable(GL_BLEND); glBindVertexArray(textVAO); glUniform1ui(0,texIndex); glBindBuffer(GL_ARRAY_BUFFER,textVBO);
    float x1=x + width, y1=y + height, z=0.0f; float vertices[30] = {x,y1,z,0.0f,0.0f,x1,y,z,1.0f,1.0f,x1,y1,z,1.0f,0.0f,x,y1,z,0.0f,0.0f,x,y,z,0.0f,1.0f,x1,y,z,1.0f,1.0f};
    glBufferData(GL_ARRAY_BUFFER,30 * sizeof(float),vertices,GL_DYNAMIC_DRAW); glDrawArrays(0x0004/*GL_TRIANGLES*/,0,6); drawCalls++; uiDrawCalls++; vertsRendered += 6; glBindBuffer(GL_ARRAY_BUFFER,0);
}

void RenderLoading(const char * restrict text) { glBindFramebuffer(GL_FRAMEBUFFER,0); glClear(GL_COLOR_BUFFER_BIT); glViewport(0,0,Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight); RenderTextC(683,384,T_WHITE,FONT_NORMAL,1,text); window->context.swapBuffers(window); }
void GenerateAndBindTexture(u32 *id, i32 internalFormat, i32 width, i32 height, u32 format, u32 type, i32 filt, u8* bmp) { if (*id == 0) {glGenTextures(1,id);} glBindTexture(GL_TEXTURE_2D,*id); glTexImage2D(GL_TEXTURE_2D,0,internalFormat,width,height,0,format,type,bmp); glTexParameteri(GL_TEXTURE_2D,0x2801/*GL_TEXTURE_MIN_FILTER*/,filt); glTexParameteri(GL_TEXTURE_2D,0x2800/*GL_TEXTURE_MAG_FILTER*/,filt); }
void AddCamView(V3 p, Quaternion r, u8 fv, u16 w, u16 h, float nr, float fr) { if(camViewCount >= 64){DualLogWarn("Too many cam views!  Skipped at %f %f %f\n",p.x,p.y,p.z); return;} camViews[camViewCount] = (CamView){p,r,fv,w,h,nr,fr,World.pauseRelativeTime + (camViewCount * 0.05f) + 0.5f,false}; GenerateAndBindTexture(&camViewTextures[camViewCount],GL_RGBA8,w,h,GL_RGBA,GL_UNSIGNED_BYTE,0x2600/*GL_NEAREST*/,NULL); camViewCount++; }
void UpdateScreenSize(i32 width, i32 height) {
    u16 w = Sys_Settings.ScreenWidth = vmax(vmin((u16)width,7680u),320u), h = Sys_Settings.ScreenHeight = vmax(vmin((u16)height,4320u),200u); // Cap at minimum Quake resolution and maximum 8k.
    float wf = (float)w, hf = (float)h; Sys_Settings.ScreenCenterX = wf * 0.5f; Sys_Settings.ScreenCenterY = hf * 0.5f;
    glViewport(0,0,w,h);
    glUseProgram(imageBlitSP); glUniform1ui(2,w); glUniform1ui(3,h); glUniform1i(26,Sys_Settings.SSR_RES); glUseProgram(chunkSP); glUniform1ui(6,w); glUniform1ui(7,h); glUseProgram(ssrSP); glUniform1ui(0,w / Sys_Settings.SSR_RES); glUniform1ui(1,h / Sys_Settings.SSR_RES); glUniform1i(2,Sys_Settings.SSR_RES);
    GenerateAndBindTexture(&inputImageID, GL_RGBA8,w,h,GL_RGBA,GL_UNSIGNED_BYTE,0x2600/*GL_NEAREST*/,NULL); // Lit Raster
    GenerateAndBindTexture(&inputSpecID,  GL_RGBA8,w,h,GL_RGBA,GL_UNSIGNED_BYTE,0x2600/*GL_NEAREST*/,NULL); // Specular Colors
    GenerateAndBindTexture(&inputNormalID,GL_RG16F,w,h, GL_RGB,        GL_FLOAT,0x2600/*GL_NEAREST*/,NULL); // Normal XYZ
    GenerateAndBindTexture(&inputDepthID,0x81A7/*GL_DEPTH_COMPONENT32*/,w,h,0x1902/*GL_DEPTH_COMPONENT*/,GL_FLOAT,0x2600/*GL_NEAREST*/,NULL); // Raster Depth
    GenerateAndBindTexture(&outputImageID,GL_RGBA8,w / Sys_Settings.SSR_RES,h / Sys_Settings.SSR_RES,GL_RGBA,GL_UNSIGNED_BYTE,0x2601/*GL_LINEAR*/,NULL);
    glBindFramebuffer(GL_FRAMEBUFFER,gBufferFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,inputImageID,0); glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT1,GL_TEXTURE_2D,inputSpecID,0); glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT2,GL_TEXTURE_2D,inputNormalID,0);
    glFramebufferTexture2D(GL_FRAMEBUFFER,0x8D00/*GL_DEPTH_ATTACHMENT*/,GL_TEXTURE_2D,inputDepthID,0);
    glBindImageTexture(0,inputImageID,0,GL_FALSE,0,GL_READ_WRITE,GL_RGBA8);      // Main Rendered Color
    glBindImageTexture(2,inputSpecID,0,GL_FALSE,0,GL_READ_WRITE,GL_RGBA8);       // Specular
    glBindImageTexture(4,outputImageID,0,GL_FALSE,0,GL_READ_WRITE,GL_RGBA8);     // SSR result
    glBindImageTexture(5,inputNormalID,0,GL_FALSE,0,GL_READ_WRITE,GL_RG16F);     // Normal XYZ
    glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D,outputImageID);
    glBindFramebuffer(GL_FRAMEBUFFER,0); ignore_next_mouse_delta = true;
}
#include "ui.c"
// Lights
#define INVSQRT2 0.70710678118f
Quaternion cubeQuats[6] = {{0.0f,INVSQRT2,0.0f,INVSQRT2}/*+X:Right*/,{0.0f,-INVSQRT2,0.0f,INVSQRT2}/*-X:Left*/,{-INVSQRT2,0.0f,0.0f,INVSQRT2}/*+Y:Up*/,{INVSQRT2,0.0f,0.0f,INVSQRT2}/*-Y:Down*/,{0.0f,0.0f,0.0f,1.0f}/*+Z:Forward*/,{0.0f,1.0f,0.0f,0.0f}/*-Z:Backward*/ };
void UpdateLights() {
    for (u16 lightIdx=0;lightIdx<World.loadedLights;++lightIdx) {
        V3 lightPos = World.lightsNewPosition[lightIdx];
        World.lights[lightIdx].pos = lightPos;
        if (World.lights[lightIdx].lflags & LDIRTY) { // Marked all as true at level load.
            flag_set(&World.lights[lightIdx].lflags,LDIRTY,false);
            #pragma GCC unroll 6 // Update to new position
            for (int j=0;j<6;++j) { mat4_lookat_from((float*)lightView[lightIdx][j],&cubeQuats[j],lightPos); mul_mat4((float*)lightViewProj[lightIdx][j],shadowmapsPerspectiveProjection,(float*)lightView[lightIdx][j]); ExtractFrustumPlanes((float*)lightViewProj[lightIdx][j],lightFrustumPlanes[lightIdx][j]); }
        }
    }
    if (!World.paused && !World.menuActive) {
        for (int i=0;i<World.loadedLights;++i) { // Just lerps/flickers in intensity
            if (World.lanims[i].numIntervalSteps < 1) continue;
            if (!(World.lights[i].lflags & LIGHTON)) { World.lights[i].intensity = 0.0f; continue; }
            if (World.lanims[i].lerpTime < (float)World.pauseRelativeTime) {
                World.lights[i].intensity = World.lanims[i].lerpUp ? World.lights[i].maxIntensity : World.lights[i].minIntensity; // Pick target to lerp towards
                World.lanims[i].lerpUp = !World.lanims[i].lerpUp;
                World.lanims[i].currentStep++; if (World.lanims[i].currentStep >= World.lanims[i].numIntervalSteps) World.lanims[i].currentStep = 0; // Wrap and start over continuous looping
                World.lanims[i].lerpStepTime = World.lanims[i].intervalSteps[World.lanims[i].currentStep];
                World.lanims[i].lerpTime = (float)World.pauseRelativeTime + World.lanims[i].lerpStepTime;
                World.lanims[i].lerpStartTime = (float)World.pauseRelativeTime;
            } else if (World.lights[i].lflags & LERPON) {
                if (World.lanims[i].currentStep < World.lanims[i].numLerpSteps) {
                    if (World.lanims[i].stepIsLerping[World.lanims[i].currentStep]) {
                        World.lanims[i].lerpValue = ((float)World.pauseRelativeTime - World.lanims[i].lerpStartTime)/(World.lanims[i].lerpTime - World.lanims[i].lerpStartTime); // percent towards goal time
                        float lerpVal = World.lanims[i].lerpUp ? World.lanims[i].lerpValue : (1.0f - World.lanims[i].lerpValue);
                        World.lanims[i].lerpValue = World.lights[i].minIntensity + ((World.lights[i].maxIntensity - World.lights[i].minIntensity) * lerpVal);
                        World.lights[i].intensity = World.lanims[i].lerpValue;
                    }
                }
            }
        }
    }
    glBindBuffer(GL_SSBO,lightsID); glBufferData(GL_SSBO,World.loadedLights * sizeof(Light),World.lights,GL_DYNAMIC_DRAW); // Always update the light intensity for flickers and such.
    glUseProgram(voxelUpdateSP); glUniform3f(5,World.position[PLAYER1].x,World.position[PLAYER1].y,World.position[PLAYER1].z); glDispatchCompute((VOXELS_X+15)/16,(VOXELS_Z+15)/16,1);
}
// Shadowmapping
#define SHADOW_NEARMESH_MAX 512
typedef struct {float depth; u16 index; } DepthSort;
DepthSort shadows_nearMeshes[SHADOW_NEARMESH_MAX];
INLINE bool EntNotVisible(u16 i, bool otherCondition) { Entity* e = &World.instances[i]; return e->texIndex > texCnt || !(e->entflags & EF_ACTIVE) || e->index >= MAX_ENTITIES || e->modelIndex >= MAX_MDLS || e->texIndex >= MAX_TXRS || otherCondition; }
INLINE u16 GetAndBindModel(u16 i, u16 currentModelType) { glUniform1ui(0,i); u16 modelType = (instanceIsLODArray[i] || Sys_Settings.ModelDetail < 1u) && World.instances[i].lodIndex < mdlsCnt ? World.instances[i].lodIndex : World.instances[i].modelIndex; if (currentModelType == modelType && currentModelType != 0) return currentModelType; glBindVertexBuffer(0,vbos[modelType],0,VRT_ATT_SZ); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,tbos[modelType]); return modelType; }
typedef float __m256 __attribute__((__vector_size__(32), __may_alias__));
typedef long long __m256i __attribute__((__vector_size__(32), __may_alias__));
typedef float __v8sf __attribute__((__vector_size__(32), __may_alias__));
#define _CMP_LT_OQ 0x11
#define _CMP_GT_OQ 0x1e
extern __inline __m256 __attribute__((__gnu_inline__, __always_inline__, __artificial__, target("avx2,fma"))) _mm256_load_ps(float const *__P) { return *(const __m256 *)__P; }
extern __inline __m256 __attribute__((__gnu_inline__, __always_inline__, __artificial__, target("avx2,fma"))) _mm256_loadu_ps(float const *__P) { __m256 __W; __builtin_memcpy(&__W, __P, sizeof(__m256)); return __W; }
extern __inline void __attribute__((__gnu_inline__, __always_inline__, __artificial__, target("avx2,fma"))) _mm256_store_ps(float *__P, __m256 __W) { *(__m256 *)__P = __W; }
extern __inline __m256 __attribute__((__gnu_inline__, __always_inline__, __artificial__, target("avx2,fma"))) _mm256_set1_ps(float __A) { return (__m256){__A, __A, __A, __A, __A, __A, __A, __A}; }
extern __inline __m256i __attribute__((__gnu_inline__, __always_inline__, __artificial__, target("avx2,fma"))) _mm256_set1_epi32(int __A) { return (__m256i){ (long long)__A, (long long)__A, (long long)__A, (long long)__A }; }
extern __inline __m256 __attribute__((__gnu_inline__, __always_inline__, __artificial__, target("avx2,fma"))) _mm256_castsi256_ps(__m256i __A) { return (__m256)__A; }
extern __inline __m256 __attribute__((__gnu_inline__, __always_inline__, __artificial__, target("avx2,fma"))) _mm256_sub_ps(__m256 __A, __m256 __B) { return __A - __B; }
extern __inline __m256 __attribute__((__gnu_inline__, __always_inline__, __artificial__, target("avx2,fma"))) _mm256_add_ps(__m256 __A, __m256 __B) { return __A + __B; }
extern __inline __m256 __attribute__((__gnu_inline__, __always_inline__, __artificial__, target("avx2,fma"))) _mm256_mul_ps(__m256 __A, __m256 __B) { return __A * __B; }
extern __inline __m256 __attribute__((__gnu_inline__, __always_inline__, __artificial__, target("avx2,fma"))) _mm256_fmadd_ps(__m256 __A, __m256 __B, __m256 __C) { return __A * __B + __C; }
extern __inline __m256 __attribute__((__gnu_inline__, __always_inline__, __artificial__, target("avx2,fma"))) _mm256_max_ps(__m256 __A, __m256 __B) { return (__m256) __builtin_ia32_maxps256((__v8sf)__A, (__v8sf)__B); }
#define _mm256_cmp_ps(A, B, C) ((__m256) __builtin_ia32_cmpps256 ((__v8sf)(__m256)(A), (__v8sf)(__m256)(B), (int)(C))) 
extern __inline __m256 __attribute__((__gnu_inline__, __always_inline__, __artificial__, target("avx2,fma"))) _mm256_andnot_ps(__m256 __A, __m256 __B) { return (__m256)(~(__m256i)__A & (__m256i)__B); }
extern __inline __m256 __attribute__((__gnu_inline__, __always_inline__, __artificial__, target("avx2,fma"))) _mm256_and_ps(__m256 __A, __m256 __B) { return (__m256)((__m256i)__A & (__m256i)__B); }
extern __inline __m256 __attribute__((__gnu_inline__, __always_inline__, __artificial__, target("avx2,fma"))) _mm256_or_ps(__m256 __A, __m256 __B) { return (__m256)((__m256i)__A | (__m256i)__B); }
extern __inline __m256 __attribute__((__gnu_inline__, __always_inline__, __artificial__, target("avx2,fma"))) _mm256_xor_ps(__m256 __A, __m256 __B) { return (__m256)((__m256i)__A ^ (__m256i)__B); }
extern __inline int __attribute__((__gnu_inline__, __always_inline__, __artificial__, target("avx2,fma"))) _mm256_movemask_ps(__m256 __A) { return __builtin_ia32_movmskps256((__v8sf)__A); }
extern __inline __m256 __attribute__((__gnu_inline__, __always_inline__, __artificial__, target("avx2,fma"))) _mm256_setzero_ps(void) { return (__m256){ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }; }
#define SC_MAX (SHADOW_NEARMESH_MAX * MAX_SHADOWMAPS)
DepthSort shadows_nearMeshes[SHADOW_NEARMESH_MAX]; u16 shadowCasterIndices[SC_MAX], candidates[MAX_SHADOWMAPS]; static __attribute__((aligned(64))) float sc_posX[SC_MAX], sc_posY[SC_MAX], sc_posZ[SC_MAX], sc_radius[SC_MAX], sc_shadRadius[SC_MAX];
static u16 sc_origIdx[SC_MAX], shadowSlot[LIGHT_COUNT];
static u8 shadowFaces[LIGHT_COUNT];
static float shadowPosSum[LIGHT_COUNT];
static u32 shadowIdSum[LIGHT_COUNT], shadClearFace[SHADOW_MAP_SIZE*SHADOW_MAP_SIZE];
static i8 shadowLevel=-1; u32 shadowNextSlot=0; static const i8 faceAxis[6] = {0,0,1,1,2,2}; static const float faceSign[6] = {1.f,-1.f,1.f,-1.f,1.f,-1.f};
INLINE u8 GetCubemapFaceMask(V3 d, float r) {
    u8 m=0; float absX=vabs(d.x),absY=vabs(d.y),absZ=vabs(d.z); float maxAbsYZ = absY > absZ ? absY : absZ; float maxAbsXZ = absX > absZ ? absX : absZ; float maxAbsXY = absX > absY ? absX : absY;
    if (d.x+r > maxAbsYZ) m|=(1<<0); if (d.x-r < -maxAbsYZ) m|=(1<<1); if (d.y+r > maxAbsXZ) m|=(1<<2); if (d.y-r < -maxAbsXZ) m|=(1<<3); if (d.z+r > maxAbsXY) m|=(1<<4); if (d.z-r < -maxAbsXY) m|=(1<<5); return m;
}

INLINE bool ShadowCasterMoved(u16 i) { return i != PLAYER1 && (World.instances[i].entflags & EF_MOVING) && !IdxIsNPC(World.instances[i].index); }
__attribute__((hot, target("avx2,fma"))) void RenderShadowmaps(void) {
    double shadowStartTime = get_time(); mset(candidates,U16_MAX,MAX_SHADOWMAPS * sizeof(u16)); V3 playerPos=World.position[PLAYER1], pf=World.instances[PLAYER1].forward; u16 numCandidates=0; i32 numCasters=0;
    for (u16 i = 0; i < World.loadedLights; ++i) {
        if (unlikely(!(World.lights[i].lflags & SHADON) || !(World.lights[i].lflags & LIGHTON))) continue;
        V3 lightPos = World.lights[i].pos; float intensity = World.lights[i].maxIntensity; if (unlikely(intensity < 0.1f)) continue;
        float range = World.lights[i].range; float luminosity = (intensity / (range * range)); if (luminosity < 0.008f && (range < 8.0f || intensity < 0.5f)) continue;
        u16 cellX=PosGetCellCoordX(lightPos.x), cellZ=PosGetCellCoordZ(lightPos.z); int lightCellIdx = (cellZ * WORLDX) + cellX; u8 r = vmax(vceil(range * (1.0f / CELLSZ)),2); 
        bool inPVS = (gridCellStates[lightCellIdx] & CELL_VISIBLE); if (likely(!inPVS)) inPVS = NeighborhoodInPVS(cellX,cellZ,r); if (!inPVS) continue;
        float dx = lightPos.x - playerPos.x, dy = lightPos.y - playerPos.y, dz = lightPos.z - playerPos.z; float distSqrdToPlayer = dx*dx + dy*dy + dz*dz; float dotResult = (dx*pf.x + dy*pf.y + dz*pf.z); if (dotResult < 0.0f && distSqrdToPlayer > (range * range)) continue;
        candidates[numCandidates++] = i; if (numCandidates >= MAX_SHADOWMAPS) break;
    }
    if (numCandidates == 0) { shadowTime = get_time() - shadowStartTime; return; }
    for (u16 i=INSTS_1ST_IDX;i<World.instCount;++i) { if (EntNotVisible(i, (World.instances[i].entflags & EF_NO_SHADOWS)) || IdxIsNPC(World.instances[i].index)){continue;} shadowCasterIndices[numCasters++]=i; if(numCasters >= SC_MAX){break;} }
    for (i32 i=0;i+8<=numCasters;i+=8) {
        float lx[8], ly[8], lz[8], lr[8], lsr[8]; for (int k=0;k<8;++k){u16 j=shadowCasterIndices[i + k]; lx[k]=World.position[j].x; ly[k]=World.position[j].y; lz[k]=World.position[j].z; lr[k]=World.radius[j]; lsr[k]=World.instances[j].shadRadius; sc_origIdx[i + k]=j;}
        _mm256_store_ps(&sc_posX[i],_mm256_loadu_ps(lx)); _mm256_store_ps(&sc_posY[i],_mm256_loadu_ps(ly)); _mm256_store_ps(&sc_posZ[i],_mm256_loadu_ps(lz)); _mm256_store_ps(&sc_radius[i],_mm256_loadu_ps(lr)); _mm256_store_ps(&sc_shadRadius[i],_mm256_loadu_ps(lsr));
    }
    for (i32 i=0;i<numCasters;++i) { u16 j=shadowCasterIndices[i]; sc_posX[i]=World.position[j].x; sc_posY[i]=World.position[j].y; sc_posZ[i]=World.position[j].z; sc_radius[i]=World.radius[j]; sc_shadRadius[i]=World.instances[j].shadRadius; sc_origIdx[i]=j; }
    const u16 numCastersAligned = numCasters & ~7u;
    if (shadowLevel != (i8)World.curLev) { mset(shadowSlot,0xFF,sizeof(shadowSlot)); mset(shadowFaces,0,sizeof(shadowFaces)); mset(shadowPosSum,0,sizeof(shadowPosSum)); mset(shadowIdSum,0,sizeof(shadowIdSum)); mset(shadClearFace,0xFF,sizeof(shadClearFace)); shadowNextSlot=0; shadowLevel=(i8)World.curLev; }
    shadDrawCalls = 0U; glBindBuffer(GL_SSBO, shadowMapSSBO); glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE); glUseProgram(shadowmapsSP);
    u32 currentSortKey = 0xFFFFFFFF, currentTriCount = 0; u16 currentModelType = 0xFFFF, currentTexIndex = 0xFFFF; bool currentIsTransparent = false, useDetail = Sys_Settings.ModelDetail;
    typedef struct { u32 sortKey; u16 instanceIdx; } SortedMesh;
    SortedMesh localMeshes[SHADOW_NEARMESH_MAX];
    for (u16 c = 0; c < numCandidates; ++c) {
        u16 lightIdx = candidates[c]; if (lightIdx == U16_MAX) continue;
        V3 lpos = World.lights[lightIdx].pos; float effectiveRadius = vmin(World.lights[lightIdx].range,15.36f); V3 toLight = V3_AsubB(lpos, playerPos);
        const float addX = (pf.x >= 0.0f) ? effectiveRadius : -effectiveRadius; const float addY = (pf.y >= 0.0f) ? effectiveRadius : -effectiveRadius; const float addZ = (pf.z >= 0.0f) ? effectiveRadius : -effectiveRadius;
        __attribute__((aligned(32))) float cX[8],cY[8],cZ[8];
        for (int f = 0; f < 6; ++f) {
            const int axis = faceAxis[f]; const float sign = faceSign[f]; float x = toLight.x + addX, y = toLight.y + addY, z = toLight.z + addZ;
            if (axis == 0) x = toLight.x + sign * effectiveRadius; else if (axis == 1) y = toLight.y + sign * effectiveRadius; else z = toLight.z + sign * effectiveRadius;
            cX[f] = x; cY[f] = y; cZ[f] = z;
        }
        cX[6] = cY[6] = cZ[6] = 0.0f; cX[7] = cY[7] = cZ[7] = 0.0f;
        const __m256 cx = _mm256_load_ps(cX); const __m256 cy = _mm256_load_ps(cY); const __m256 cz = _mm256_load_ps(cZ); const __m256 fx = _mm256_set1_ps(pf.x); const __m256 fy = _mm256_set1_ps(pf.y); const __m256 fz = _mm256_set1_ps(pf.z);
        const __m256 dot = _mm256_fmadd_ps(cx, fx, _mm256_fmadd_ps(cy, fy, _mm256_mul_ps(cz, fz))); const __m256 visible = _mm256_cmp_ps(dot, _mm256_setzero_ps(), _CMP_GT_OQ);
        u8 faceMask = (u8)(_mm256_movemask_ps(visible) & 0x3F); for (u8 face = 0; face < 6; ++face) { if (!(faceMask & (1u << face))) { if (SphereInFrustum(lightFrustumPlanes[lightIdx][face], playerPos, 0.48f)) {faceMask |= (u8)(1u << face);} } } if (faceMask == 0) continue;
        const __m256 lposX = _mm256_set1_ps(lpos.x); const __m256 lposY = _mm256_set1_ps(lpos.y); const __m256 lposZ = _mm256_set1_ps(lpos.z); const __m256 effR  = _mm256_set1_ps(effectiveRadius);
        const __m256 signMask = _mm256_castsi256_ps(_mm256_set1_epi32(0x80000000u)); u16 nearbyMeshCount = 0; i32 k = 0; bool anyMoved=false; float posSum=0.0f;
        for (; k < numCastersAligned; k += 8) {
            const __m256 px = _mm256_load_ps(&sc_posX[k]); const __m256 py = _mm256_load_ps(&sc_posY[k]); const __m256 pz = _mm256_load_ps(&sc_posZ[k]); const __m256 r  = _mm256_load_ps(&sc_radius[k]); const __m256 sr = _mm256_load_ps(&sc_shadRadius[k]);
            const __m256 dx = _mm256_sub_ps(px, lposX); const __m256 dy = _mm256_sub_ps(py, lposY); const __m256 dz = _mm256_sub_ps(pz, lposZ);
            const __m256 distSq   = _mm256_fmadd_ps(dx, dx, _mm256_fmadd_ps(dy, dy, _mm256_mul_ps(dz, dz)));
            const __m256 radSum   = _mm256_add_ps(effR, r);
            const __m256 radSumSq = _mm256_mul_ps(radSum, radSum);
            const __m256 inRange  = _mm256_cmp_ps(distSq, radSumSq, _CMP_LT_OQ);
            const __m256 absX = _mm256_andnot_ps(signMask, dx); const __m256 absY = _mm256_andnot_ps(signMask, dy); const __m256 absZ = _mm256_andnot_ps(signMask, dz);
            const __m256 maxAbsYZ = _mm256_max_ps(absY,absZ); const __m256 maxAbsXZ = _mm256_max_ps(absX,absZ); const __m256 maxAbsXY = _mm256_max_ps(absX,absY); // Face Math: A > |B| && A > |C| <==> A > max(|B|, |C|)
            const __m256 negMaxAbsYZ = _mm256_or_ps(maxAbsYZ,signMask); const __m256 negMaxAbsXZ = _mm256_or_ps(maxAbsXZ,signMask); const __m256 negMaxAbsXY = _mm256_or_ps(maxAbsXY,signMask);
            const __m256 dxp = _mm256_add_ps(dx, sr); const __m256 posXface = _mm256_cmp_ps(dxp, maxAbsYZ, _CMP_GT_OQ);
            const __m256 dxm = _mm256_sub_ps(dx, sr); const __m256 negXface = _mm256_cmp_ps(dxm, negMaxAbsYZ, _CMP_LT_OQ);
            const __m256 dyp = _mm256_add_ps(dy, sr); const __m256 posYface = _mm256_cmp_ps(dyp, maxAbsXZ, _CMP_GT_OQ);
            const __m256 dym = _mm256_sub_ps(dy, sr); const __m256 negYface = _mm256_cmp_ps(dym, negMaxAbsXZ, _CMP_LT_OQ);
            const __m256 dzp = _mm256_add_ps(dz, sr); const __m256 posZface = _mm256_cmp_ps(dzp, maxAbsXY, _CMP_GT_OQ);
            const __m256 dzm = _mm256_sub_ps(dz, sr); const __m256 negZface = _mm256_cmp_ps(dzm, negMaxAbsXY, _CMP_LT_OQ);
            const __m256 anyFace = _mm256_or_ps(_mm256_or_ps(posXface, negXface),_mm256_or_ps(_mm256_or_ps(posYface, negYface), _mm256_or_ps(posZface, negZface))); const __m256 valid = _mm256_and_ps(inRange, anyFace);
            unsigned mask = (unsigned)_mm256_movemask_ps(valid);
            while (mask) {
                int bit = __builtin_ctz(mask); mask &= mask - 1;
                if (unlikely(nearbyMeshCount >= SHADOW_NEARMESH_MAX)) { DualLogWarn("Shadowmapping ran out of nearMeshes at %u!  Skipping some renderables for light %u!\n", SHADOW_NEARMESH_MAX, lightIdx); k = numCastersAligned; break; }
                u16 instIdx = sc_origIdx[k + bit]; Entity* e = &World.instances[instIdx];
                u16 modelType = (instanceIsLODArray[instIdx] || useDetail < 1u) && e->lodIndex < mdlsCnt ? e->lodIndex : e->modelIndex;
                localMeshes[nearbyMeshCount].instanceIdx = instIdx; localMeshes[nearbyMeshCount].sortKey = ((u32)modelType << 16) | e->texIndex;
                nearbyMeshCount++; posSum += World.position[instIdx].x + World.position[instIdx].y + World.position[instIdx].z;
                if (ShadowCasterMoved(instIdx)) anyMoved = true;
            }
            if (k == numCastersAligned && nearbyMeshCount >= SHADOW_NEARMESH_MAX) break;
        }
        if (nearbyMeshCount < SHADOW_NEARMESH_MAX) {
            for (; k < numCasters; ++k) {
                V3 d = V3_AsubB(World.position[sc_origIdx[k]],lpos); float distToLightSqrd = V3_dot(d,d); float radSum = (effectiveRadius + World.radius[sc_origIdx[k]]); if (distToLightSqrd >= radSum * radSum) continue;
                u8 faceMaskScalar = GetCubemapFaceMask(d, World.instances[sc_origIdx[k]].shadRadius); if (faceMaskScalar == 0) continue;
                u16 instIdx = sc_origIdx[k]; Entity* e = &World.instances[instIdx];
                u16 modelType = (instanceIsLODArray[instIdx] || useDetail < 1u) && e->lodIndex < mdlsCnt ? e->lodIndex : e->modelIndex;
                localMeshes[nearbyMeshCount].instanceIdx = instIdx; localMeshes[nearbyMeshCount].sortKey = ((u32)modelType << 16) | e->texIndex; nearbyMeshCount++; posSum += World.position[instIdx].x + World.position[instIdx].y + World.position[instIdx].z; if (ShadowCasterMoved(instIdx)) anyMoved = true;
                if (nearbyMeshCount >= SHADOW_NEARMESH_MAX) { DualLogWarn("Shadowmapping ran out of nearMeshes at %u!  Skipping some renderables for light %u!\n", SHADOW_NEARMESH_MAX, lightIdx); break; }
            }
        }
        if (nearbyMeshCount == 0) { shadowmapIndirectionList[lightIdx] = LIGHT_COUNT; continue; }
        for (u16 j = 1; j < nearbyMeshCount; ++j) { SortedMesh key = localMeshes[j]; int sk = (int)j - 1; while(sk >= 0 && localMeshes[sk].sortKey > key.sortKey){localMeshes[sk + 1]=localMeshes[sk]; --sk;} localMeshes[sk + 1] = key; }
        u32 idSum=0; for (u16 j=0;j<nearbyMeshCount;++j) idSum += localMeshes[j].instanceIdx + localMeshes[j].sortKey;
        u16 slot = shadowSlot[lightIdx];
        bool contentDirty = (anyMoved || posSum != shadowPosSum[lightIdx] || idSum != shadowIdSum[lightIdx] || slot == U16_MAX);
        u8 needFaces = (u8)(faceMask & ~shadowFaces[lightIdx]);
        if (contentDirty || needFaces) {
            if (slot == U16_MAX) { if (shadowNextSlot >= MAX_SHADOWMAPS) { mset(shadowSlot,0xFF,sizeof(shadowSlot)); mset(shadowFaces,0,sizeof(shadowFaces)); shadowNextSlot=0; } shadowSlot[lightIdx]=(u16)shadowNextSlot++; slot=shadowSlot[lightIdx]; }
            u32 slotOff=(u32)slot*(SHADOW_MAP_SIZE*SHADOW_MAP_SIZE*6);
            glUniform3f(3, lpos.x, lpos.y, lpos.z);
            u8 renderFaces = contentDirty ? faceMask : needFaces; // Content change invalidates all faces, but viewpoint change only needs relevant faces.
            #pragma GCC unroll 6
            for (u8 face = 0; face < 6; ++face) {
                if (!(renderFaces & (1u << face))) {continue;}
                glBufferSubData(GL_SSBO,(slotOff + face*SHADOW_MAP_SIZE*SHADOW_MAP_SIZE)*4,SHADOW_MAP_SIZE*SHADOW_MAP_SIZE*4,shadClearFace);
                glUniform1ui(2,face); glUniformMatrix4fv(1,1,GL_FALSE,(float*)lightViewProj[lightIdx][face]); glUniform1ui(7,slotOff + (face * SHADOW_MAP_SIZE * SHADOW_MAP_SIZE));
                for (u16 j=0;j<nearbyMeshCount;++j) {
                    u16 instIdx = localMeshes[j].instanceIdx; u32 sortKey = localMeshes[j].sortKey;
                    glUniform1ui(0,instIdx);
                    if (currentSortKey != sortKey) {
                        currentSortKey = sortKey; u16 modelType = (u16)(sortKey >> 16); u16 texIndex = (u16)(sortKey & 0xFFFF);
                        if (currentModelType != modelType) { currentModelType = modelType; glBindVertexBuffer(0, vbos[modelType], 0, VRT_ATT_SZ); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tbos[modelType]); currentTriCount = modelTriangleCounts[currentModelType] * 3; }
                        if (currentTexIndex != texIndex) { currentTexIndex = texIndex; glUniform1ui(6, texIndex); bool texIsTransparent = transparentTexture[texIndex]; if (currentIsTransparent != texIsTransparent) { currentIsTransparent = texIsTransparent; glUniform1ui(8, (u32)currentIsTransparent); } }
                    } glDrawElements(0x0004, currentTriCount, GL_UNSIGNED_SHORT, 0); drawCalls++; shadDrawCalls++; vertsRendered += currentTriCount;
                }
            } shadowPosSum[lightIdx]=posSum; shadowIdSum[lightIdx]=idSum; shadowFaces[lightIdx] = contentDirty ? faceMask : (u8)(shadowFaces[lightIdx] | faceMask);
        } shadowmapIndirectionList[lightIdx]=slot;
    } glViewport(0, 0, Sys_Settings.ScreenWidth, Sys_Settings.ScreenHeight); glBindBuffer(GL_SSBO, shadowMapsIndirectionID); glBufferData(GL_SSBO, World.loadedLights * sizeof(u32), shadowmapIndirectionList, GL_DYNAMIC_DRAW); shadowTime = get_time() - shadowStartTime;
}

DepthSort visibleInstances[INSTANCE_COUNT];
__attribute__((pure)) i32 dsort(const void* a, const void* b) { float da = ((const DepthSort*)a)->depth; float db = ((const DepthSort*)b)->depth; return (db > da) - (db < da); }
__attribute__((pure)) i32 dsortInv(const void* a, const void* b) { float da = ((const DepthSort*)a)->depth; float db = ((const DepthSort*)b)->depth; return (da > db) - (da < db); }
void DrawEntity(Entity* e, u16 i, u16 constIndex, u16 tex, u16* curN, u16* curT, u16* curG, u16* curS, u16* curM, bool grayscaleEnabled) {
    u16 glow=e->glowIndex,norm=e->normIndex,spec=e->specIndex;
    if (Cheats.showPhys) {if (World.col[i] == COLTYPE_BOX) {DrawBoxCollider(i);} else if (World.col[i] == COLTYPE_SPH) {DrawSphereCollider(i);} else if (World.col[i] == COLTYPE_CVX) {DrawMeshCollider(i);} else if (World.col[i] == COLTYPE_MSH) {DrawMeshCollider(i);} else if (World.col[i] == COLTYPE_CAP) {DrawCapsuleCollider(i);} DrawAngularVelocity(i);}
    glUniform1ui(17,tex==316?1u:0u); glUniform1ui(25,constIndex); glUniform1f(27,(float)(1.0f - (vclamp((float)(World.pauseRelativeTime - 0.0f) / 2.0f, 0.0f, 1.0f)))); /* cyber wall panel alpha with fade */ glUniform1ui(13,(tex==36||tex==887) ? 1u : 0u);
    if (grayscaleEnabled) { float npcHeat = IdxIsNPC(constIndex) ? ((constIndex==419 || constIndex==422 || constIndex==424 || constIndex==429 || constIndex==430 || constIndex==431||constIndex==433||constIndex==437||constIndex==438||constIndex==441) ? 1.5f : 4.0f) : 0.0f; glUniform1f(9,npcHeat); }
    glUniform1ui(30,e->camView < camViewCount ? 1u : 0u);
    if(e->camView < camViewCount) { glActiveTexture(GL_TEXTURE6); glBindTexture(GL_TEXTURE_2D,camViewTextures[e->camView]); glUniform2ui(28,camViews[e->camView].width,camViews[e->camView].height); glUniform1i(29,6); }
    if((*curN) != (norm) || norm==0) { *curN=norm; glUniform1ui( 1,(u32)norm); } if((*curT) != ( tex) ||  tex==0) { *curT= tex; glUniform1ui(18,(u32)tex ); } 
    if((*curG) != (glow) || glow==0) { *curG=glow; glUniform1ui(19,(u32)glow); } if((*curS) != (spec) || spec==0) { *curS=spec; glUniform1ui(20,(u32)spec); }
    *curM=GetAndBindModel(i,*curM); u32 vc=modelTriangleCounts[*curM]*3; glDrawElements(0x0004,vc,GL_UNSIGNED_SHORT,0); drawCalls++; vertsRendered+=vc;
}

bool mat4_inverse(const float* m, float* out) {
    float inv[16],det;
    inv[0] =  m[5]*m[10]*m[15] - m[5]*m[14]*m[11] - m[9]*m[6]*m[15] + m[9]*m[14]*m[7] + m[13]*m[6]*m[11] - m[13]*m[10]*m[7]; inv[4] = -m[4]*m[10]*m[15] + m[4]*m[14]*m[11] + m[8]*m[6]*m[15] - m[8]*m[14]*m[7] - m[12]*m[6]*m[11] + m[12]*m[10]*m[7];
    inv[8] =  m[4]*m[9]*m[15]  - m[4]*m[13]*m[11] - m[8]*m[5]*m[15] + m[8]*m[13]*m[7]  + m[12]*m[5]*m[11] - m[12]*m[9]*m[7]; inv[12]= -m[4]*m[9]*m[14]  + m[4]*m[13]*m[10] + m[8]*m[5]*m[14] - m[8]*m[13]*m[6]  - m[12]*m[5]*m[10] + m[12]*m[9]*m[6];
    inv[1] = -m[1]*m[10]*m[15] + m[1]*m[14]*m[11] + m[9]*m[2]*m[15] - m[9]*m[14]*m[3] - m[13]*m[2]*m[11] + m[13]*m[10]*m[3]; inv[5] =  m[0]*m[10]*m[15] - m[0]*m[14]*m[11] - m[8]*m[2]*m[15] + m[8]*m[14]*m[3]  + m[12]*m[2]*m[11] - m[12]*m[10]*m[3];
    inv[9] = -m[0]*m[9]*m[15]  + m[0]*m[13]*m[11] + m[8]*m[1]*m[15] - m[8]*m[13]*m[3]  - m[12]*m[1]*m[11] + m[12]*m[9]*m[3]; inv[13]=  m[0]*m[9]*m[14]  - m[0]*m[13]*m[10] - m[8]*m[1]*m[14] + m[8]*m[13]*m[2]  + m[12]*m[1]*m[10] - m[12]*m[9]*m[2];
    inv[2] =  m[1]*m[6]*m[15] - m[1]*m[14]*m[7] - m[5]*m[2]*m[15] + m[5]*m[14]*m[3] + m[13]*m[2]*m[7] - m[13]*m[6]*m[3]; inv[6] = -m[0]*m[6]*m[15] + m[0]*m[14]*m[7] + m[4]*m[2]*m[15] - m[4]*m[14]*m[3] - m[12]*m[2]*m[7] + m[12]*m[6]*m[3];
    inv[10]=  m[0]*m[5]*m[15] - m[0]*m[13]*m[7] - m[4]*m[1]*m[15] + m[4]*m[13]*m[3] + m[12]*m[1]*m[7] - m[12]*m[5]*m[3]; inv[14]= -m[0]*m[5]*m[14] + m[0]*m[13]*m[6] + m[4]*m[1]*m[14] - m[4]*m[13]*m[2] - m[12]*m[1]*m[6] + m[12]*m[5]*m[2];
    inv[3] = -m[1]*m[6]*m[11] + m[1]*m[10]*m[7] + m[5]*m[2]*m[11] - m[5]*m[10]*m[3] - m[9]*m[2]*m[7]  + m[9]*m[6]*m[3]; inv[7] =  m[0]*m[6]*m[11] - m[0]*m[10]*m[7] - m[4]*m[2]*m[11] + m[4]*m[10]*m[3] + m[8]*m[2]*m[7]  - m[8]*m[6]*m[3];
    inv[11]= -m[0]*m[5]*m[11] + m[0]*m[9]*m[7]  + m[4]*m[1]*m[11] - m[4]*m[9]*m[3]  - m[8]*m[1]*m[7]  + m[8]*m[5]*m[3]; inv[15]=  m[0]*m[5]*m[10] - m[0]*m[9]*m[6]  - m[4]*m[1]*m[10] + m[4]*m[9]*m[2]  + m[8]*m[1]*m[6]  - m[8]*m[5]*m[2];
    det = m[0]*inv[0] + m[1]*inv[4] + m[2]*inv[8] + m[3]*inv[12]; if (det == 0.0f) { for(int i=0;i<16;++i) {out[i] = (i%5==0) ? 1.0f : 0.0f;} return false; }
    det = 1.0f / det; for (int i=0;i<16;++i) out[i] = inv[i] * det;
    return true;
}

void GetProjections(float* view, float* viewProj, float* invViewRot, float* invViewProj, float sfov, float aspect3D, float snear, float sfar) {
    float f = vcot(sfov * PI / 360.0f); float* m = rasterPerspectiveProjection;
    m[0]=f / aspect3D; m[1]=0.0f; m[2]=0.0f; m[3]=0.0f; m[4]=0.0f; m[5]=f; m[6]=0.0f; m[7]=0.0f; m[8]=0.0f; m[9]=0.0f; m[10]= -(sfar + snear) / (sfar - snear); m[11]=-1.0f; m[12]=0.0f; m[13]=0.0f; m[14]=-2.0f*sfar*snear / (sfar - snear); m[15]=0.0f;
    mat4_lookat_from(view,&World.rotation[PLAYER1],World.position[PLAYER1]); mul_mat4(viewProj,rasterPerspectiveProjection,view); 
    invViewRot[0]=view[0]; invViewRot[1]=view[4]; invViewRot[2]=view[8]; invViewRot[3]=view[1]; invViewRot[4]=view[5]; invViewRot[5]=view[9]; invViewRot[6]=view[2]; invViewRot[7]=view[6]; invViewRot[8]=view[10]; mat4_inverse(viewProj,invViewProj);
}
//                        0 mk3 assault rifle              1 blaster             2 dartgun               3 flech                 4 ion  5 rapier    6 pipe               7 magnum            8 magpulse               9 pistol               10 plasma                 11 rail                              12 riot              13 skorp              14 sparq               15 stun
Quaternion vWepRot[16]={{0,.67623f,.73802f,0},{-.67623f,0,0,.73802f},{.10363f,0,0,.99456f},{0,.66976f,.74389f,0},{0,.68903f,.72611f,0},{0,0,0,1},{0,0,0,1},{.63662f,0,0,-.77238f},{0,.63662f,.77238f,0},{-.67623f,0,0,.73802f},{0,-.70781f,-.70781f,0},{0,-.65003f,-.76116f,0},{-.44581f,-.44581f,-.55061f,.55061f},{0,.67623f,.73802f,0},{0,.67623f,.73802f,0},{0,.67623f,.73802f,0}};                        
        V3 vWepOfs[16]={{      0,-.54f,.451f},        {0,-.5f,0.28f},  {-.015f,-.34f,.18f},       {0,-.43f,.27f},     {0,-0.57f,0.56f},  {0,0,0},  {0,0,0},        {0,-.39f,.02f},       {0,-.54f,.44f},        {0,-.58f,.43f},     {-.02f,-.64f,.79f},         {0,-.46f,.43f},                       {0,-.5f,.08f},       {0,-.62f,.69f},       {0,-.55f,.58f},       {0,-.56f,.55f}};
extern const u16 wepModelIndices[16]; extern WeaponFireCtx wfx;
static __attribute__((hot)) void Render(bool camView, u8 camViewIdx) {
    u16 swidth, sheight; float sfov, snear, sfar;
    if (camView) { CamView* cv=&camViews[camViewIdx]; swidth=cv->width; sheight=cv->height; sfov=(float)cv->fov; snear=cv->near; sfar=cv->far; }
    else { swidth=Sys_Settings.ScreenWidth; sheight=Sys_Settings.ScreenHeight; sfov=(float)Sys_Settings.FOV; snear=0.02f; sfar=World.farPlane[World.curLev]; }
    V3 playerPos = World.position[PLAYER1]; float px=playerPos.x, py=playerPos.y, pz=playerPos.z, aspect3D=(float)swidth / (float)sheight;
    float view[16],viewProj[16],invViewRot[9],invViewProj[16];
    GetProjections(view,viewProj,invViewRot,invViewProj,sfov,aspect3D,snear,sfar);
    ExtractFrustumPlanes(viewProj,playerFrustumPlanes);
    glBindVertexArray(chunkVAO); // Common vao for RenderDynamicShadowmaps and Rasterized Geometry
    glEnable(GL_DEPTH_TEST);
    glBeginQuery(0x88BF/*GL_TIME_ELAPSED*/,gpuQ[gpuQFrame][0]); if (likely(Sys_Settings.Shadows > 0u)) RenderShadowmaps(); glEndQuery(0x88BF/*GL_TIME_ELAPSED*/); glBeginQuery(0x88BF/*GL_TIME_ELAPSED*/,gpuQ[gpuQFrame][1]);
    double rendStart = get_time();
    UpdateLights(); // This is where the voxels get updated!
    glViewport(0,0,swidth,sheight); glBindFramebuffer(GL_FRAMEBUFFER,gBufferFBO); glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); 
    glEnable(GL_CULL_FACE); glDisable(GL_BLEND); // Opaques
    u16 currentTexIndex = 0, currentNormIndex = 0, currentGlowIndex = 0, currentSpecIndex = 0, currentModelType = 0, opaqueCount = 0;
    bool skyVisible = (gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX);
    DepthSort tmpTransparent[1024]; u16 tcnt = 0;
    for (u16 i = INSTS_1ST_IDX; i < World.instCount; ++i) { // Determine base visibility
        if (EntNotVisible(i,false)) continue; // must be transparent && transparents or neither
        Entity* e = &World.instances[i]; u16 instCellIdx = e->cellIndex; u16 entIdx = e->index;
        V3 delta = V3_AsubB(World.position[i],playerPos); float distSqrd = V3_dot(delta,delta);
        float radius = modelBounds[e->modelIndex] * 2.0f * vmax(vmax(World.scale[i].x,World.scale[i].y),World.scale[i].z);
        if (!SphereInFrustum(playerFrustumPlanes,World.position[i],radius)) continue;
        if (IdxIsPortalBlockingDoor(entIdx)) { // Extra checks only needed for opaque portal blocking doors.
            if (!(gridCellStates[instCellIdx] & CELL_VISIBLE) && !NeighborhoodInPVS(e->cellX,e->cellZ,2u)/*!in pvs*/) continue;
        } else {
            if (((gridCellStates[instCellIdx] & (CELL_VISIBLE | CELL_OPEN)) == CELL_OPEN)) continue;
            if (!(gridCellStates[instCellIdx] & CELL_OPEN) && distSqrd >= 943.7184f) continue; // 30.72 * 30.72, 12 cells
        }
        if (World.instances[i].camView != 255) camViews[World.instances[i].camView].visible = true;
        if (transparentTexture[World.instances[i].texIndex]) { if(tcnt>1023){continue;} tmpTransparent[tcnt].index = i; tmpTransparent[tcnt].depth = distSqrd; tcnt++; }
        else { visibleInstances[opaqueCount].index = i; visibleInstances[opaqueCount].depth = distSqrd; opaqueCount++; }
    }
    if (World.shd1 < U16_MAX && skyVisible && World.instCount < (INSTANCE_COUNT - 4) && opaqueCount < (INSTANCE_COUNT - 4)) { // Add shield generators in skybox.
        visibleInstances[opaqueCount].index=World.shd1; visibleInstances[opaqueCount].depth=300.0f; opaqueCount++; visibleInstances[opaqueCount].index=World.shd2; visibleInstances[opaqueCount].depth=300.0f; opaqueCount++;
        visibleInstances[opaqueCount].index=World.shd3; visibleInstances[opaqueCount].depth=300.0f; opaqueCount++; visibleInstances[opaqueCount].index=World.shd4; visibleInstances[opaqueCount].depth=300.0f; opaqueCount++;
    }
    if (editModeSelection < U16_MAX && Cheats.editMode) {
        if (transparentTexture[World.instances[editModeSelection].texIndex]) { if(tcnt<=1023){ tmpTransparent[tcnt].index = editModeSelection; tmpTransparent[tcnt].depth = 1.28f; tcnt++;} }
        else { visibleInstances[opaqueCount].index = editModeSelection; visibleInstances[opaqueCount].depth = 1.28f; opaqueCount++; }
    }
    mcpy(visibleInstances + opaqueCount,tmpTransparent,tcnt * sizeof(DepthSort));
    glUseProgram(depthPrepassSP);
    glUniformMatrix4fv(2,1,0,viewProj);
    glEnable(GL_DEPTH_TEST); glColorMask(0,0,0,0); glDepthMask(1); glDepthFunc(0x0201/*GL_LESS*/); glDisable(GL_BLEND);
    if (opaqueCount > 1) qsort_new(visibleInstances,opaqueCount,sizeof(DepthSort),dsortInv); // Needed for cutout bushes/foliage
    if (tcnt > 1) qsort_new(visibleInstances + opaqueCount,tcnt,sizeof(DepthSort),dsort);
    u8 cullBlendState = 0xFF;
    for (u16 visibleIndex = 0; visibleIndex < opaqueCount + tcnt; ++visibleIndex) {
        u16 i = visibleInstances[visibleIndex].index;
        Entity* e = &World.instances[i]; u16 tex = e->texIndex;
        if (unlikely(transparentTexture[tex])) { glEnable(GL_CULL_FACE); glEnable(GL_BLEND); } // Transparents (with sort)
        else if (unlikely(doubleSidedTexture[tex] || World.scale[i].x < 0.0f || World.scale[i].y < 0.0f || World.scale[i].z < 0.0f)) { glDisable(GL_CULL_FACE); glEnable(GL_BLEND); } // Doublesided
        else { glEnable(GL_CULL_FACE); glDisable(GL_BLEND); } // Opaque
        currentModelType = GetAndBindModel(i,currentModelType);
        glUniform1ui(3,(u32)tex);
        u32 vertCount = modelTriangleCounts[currentModelType] * 3;
        glDrawElements(0x0004/*GL_TRIANGLES*/,vertCount,GL_UNSIGNED_SHORT,0); drawCalls++; vertsRendered += vertCount;
    }
    glEndQuery(0x88BF/*GL_TIME_ELAPSED*/); glBeginQuery(0x88BF/*GL_TIME_ELAPSED*/,gpuQ[gpuQFrame][2]);
    glUseProgram(chunkSP); glUniformMatrix4fv(2,1,0,viewProj); glUniform1ui(25,0u);/*default constIndex*/ cullBlendState = 0xFF;
    bool grayscaleEnabled = ModRequestsGrayscale(); glUniform1ui(26,(u32)grayscaleEnabled);
    float fogActual = World.fogColor[World.curLev].a + (float)(World.fogFac / 255u); // Alpha is base density for level.
    glUniform3f(12,World.fogColor[World.curLev].r * fogActual,World.fogColor[World.curLev].g * fogActual,World.fogColor[World.curLev].b * fogActual); // Fog Color(which is density)
    glUniform1ui(14,Sys_Settings.Reflections);
    glUniform1ui(15,Sys_Settings.Shadows);
    glUniform2f(8,World.worldMin_x[World.curLev],World.worldMin_z[World.curLev]);
    glUniform3f(10,px,py,pz);
    glColorMask(1,1,1,1); glDepthMask(0); glDepthFunc(0x0203/*GL_LEQUAL*/); // Opaque Pass
    currentTexIndex = currentNormIndex = currentGlowIndex = currentSpecIndex = currentModelType = 0;
    glUniform1f(9,0.0f); // Reset heat for infrared vision
    for (u16 visibleIndex = 0; visibleIndex < opaqueCount; ++visibleIndex) { // Opaques (already front-to-back)
        u16 i = visibleInstances[visibleIndex].index;
        Entity* e = &World.instances[i]; u16 tex = e->texIndex; u32 constIndex = e->index;
        if (unlikely(transparentTexture[tex])) continue;
        
        if (doubleSidedTexture[tex] || World.scale[i].x < 0.0f || World.scale[i].y < 0.0f || World.scale[i].z < 0.0f) { if(cullBlendState != 2){glDisable(GL_CULL_FACE); glEnable(GL_BLEND); cullBlendState=2;} } // Doublesided (either)
        else { if(cullBlendState != 0){glEnable(GL_CULL_FACE); glDisable(GL_BLEND); cullBlendState=0;} } // Opaque
        DrawEntity(e,i,constIndex,tex,&currentNormIndex,&currentTexIndex,&currentGlowIndex,&currentSpecIndex,&currentModelType,grayscaleEnabled);
    }
    glDepthMask(1); currentTexIndex = currentNormIndex = currentGlowIndex = currentSpecIndex = currentModelType = 0; // Transparents Pass
    for (u16 visibleIndex = opaqueCount; visibleIndex < (opaqueCount + tcnt); ++visibleIndex) {
        u16 i = visibleInstances[visibleIndex].index;
        Entity* e = &World.instances[i]; u16 tex = e->texIndex; u32 constIndex = e->index;
        if (likely(transparentTexture[tex])) { if(cullBlendState != 1){glEnable(GL_CULL_FACE); glEnable(GL_BLEND); cullBlendState=1;} } // Transparents (with sort)
        else if (unlikely(doubleSidedTexture[tex] || World.scale[i].x < 0.0f || World.scale[i].y < 0.0f || World.scale[i].z < 0.0f)) { if(cullBlendState != 2){glDisable(GL_CULL_FACE); glEnable(GL_BLEND); cullBlendState=2;} } // Doublesided (either)
        else continue; // Opaque
        if (unlikely((constIndex >= 561 && constIndex <= 565) || (constIndex >= 568 && constIndex <= 573))) glDepthFunc(0x0202/*GL_EQUAL*/); // Cutouts
        else glDepthFunc(0x0203/*GL_LEQUAL*/); // Actual alphas
        DrawEntity(e,i,constIndex,tex,&currentNormIndex,&currentTexIndex,&currentGlowIndex,&currentSpecIndex,&currentModelType,grayscaleEnabled);
    }
    // View weapon model rendered after transparents, before SSR (current weapon only, gl depth off, centered out in front)
    u16 wvi = World.weaponVModelIndex; // dedicated view-model instance
    if (wvi > 0 && wvi < INSTANCE_COUNT) {
        int wep16 = Get16WeaponIndexFromConstIndex(World.instances[wvi].index);
        if (wep16 >= 0 && wep16 < 16 && World.instances[wvi].modelIndex < MAX_MDLS) { // appearance set by CompleteWeaponChange
            // Offset in player-local space (right,down,forward), rotated into world by the player view; reload/swap dip added on Y.  Pivot = player position, weapon stays locked to view.
            World.weaponViewOffset = vWepOfs[wep16];
            World.weaponViewOffset.y += wfx.reloadContainerPos.y;
            V3 weaponPos = V3_AplusB(World.position[PLAYER1], quat_rot_v3(World.rotation[PLAYER1], World.weaponViewOffset));
            World.position[wvi] = weaponPos;
            World.rotation[wvi] = quat_multiply(World.rotation[PLAYER1],vWepRot[wep16]); // view orientation + per-model correction
            u16 curN=0, curT=0, curG=0, curS=0, curM=0;
            DrawEntity(&World.instances[wvi],wvi,World.instances[wvi].index,World.instances[wvi].texIndex,&curN,&curT,&curG,&curS,&curM,false);
        }
    }
    if(unlikely(camView)) {
        glEndQuery(0x88BF/*GL_TIME_ELAPSED*/);
        glBindFramebuffer(0x8CA8/*GL_READ_FRAMEBUFFER*/,gBufferFBO); glReadBuffer(GL_COLOR_ATTACHMENT0); glBindTexture(GL_TEXTURE_2D,camViewTextures[camViewIdx]);
        glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0,0,0,swidth,sheight); // Store the render result for the camview
        glBindTexture(GL_TEXTURE_2D,0); return; // After copying render result, skip SSR and composite for camviews <<<<<<<<<<<<< CAM VIEW BARRIER
    }
    if(unlikely(World.debugLineVertCount > 1)) DrawDebugLines(viewProj); // Draw Debug Lines
    glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D,inputDepthID);
    glEndQuery(0x88BF/*GL_TIME_ELAPSED*/); glBeginQuery(0x88BF/*GL_TIME_ELAPSED*/,gpuQ[gpuQFrame][3]);
    if(likely(Sys_Settings.Reflections>0u)){
        glUseProgram(ssrSP); glUniform3f(3,playerPos.x,playerPos.y,playerPos.z); glUniform1i(5,3); glUniformMatrix4fv(6,1,0,invViewProj); glUniformMatrix4fv(4,1,GL_FALSE,viewProj); u32 groupX_ssr=((Sys_Settings.ScreenWidth/Sys_Settings.SSR_RES)+31)/32, groupY_ssr=((Sys_Settings.ScreenHeight/Sys_Settings.SSR_RES)+31)/32;
        glDispatchCompute(groupX_ssr,groupY_ssr,1);
    }
    glBindFramebuffer(GL_FRAMEBUFFER,uiFBO); glClearColor(0,0,0,0); glClear(GL_COLOR_BUFFER_BIT); glClearColor(0,0,0,1);
    glViewport(0,0,1366,768); glDisable(GL_CULL_FACE); renderTime = get_time() - rendStart;
    RenderUI();
    if ((World.inventoryMode && !Cheats.noHUD) || World.menuActive || World.paused) RenderUIImage((i16)(World.cursorPos_x) - 20,(i16)(World.cursorPos_y) - 20,40,40,GetCursorTexture());
    else if (!Cheats.noHUD) RenderUIImage(663,364,40,40,GetCursorTexture()); // Centered on UI fixed resolution 1366x768 FBO
    glEndQuery(0x88BF/*GL_TIME_ELAPSED*/); glBeginQuery(0x88BF/*GL_TIME_ELAPSED*/,gpuQ[gpuQFrame][4]);
    glBindFramebuffer(GL_FRAMEBUFFER,0); glViewport(0,0,swidth,sheight);
    glUseProgram(imageBlitSP); glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D,inputImageID); glUniform1i(4,4); // outputImage texture sampler2D, don't remember why when active texture is texture 0. meh.... oh maybe to not read and write same binding?
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D,inputUIID); glUniform1i(31,1); glUniform1i(32,3); glUniformMatrix4fv(33,1,0,invViewProj);
    double berserkTimeRemainingNormalized = World.invP1.berserkFinished > 0.0001 ? (World.invP1.berserkFinished - World.pauseRelativeTime) / BERSERK_TIME : 0.0;
    if (World.invP1.berserkFinished < World.pauseRelativeTime && World.invP1.berserkFinished > 0.0001) World.invP1.berserkFinished = berserkTimeRemainingNormalized = 0.0;
    glUniform1ui(5,Sys_Settings.Reflections); glUniform1ui(6,Sys_Settings.FXAA); glUniform1f(14,Sys_Settings.FOV); glUniform1f(16,aspect3D); glUniform1ui(22,Sys_Settings.Shadows); glUniform1f(9,(float)berserkTimeRemainingNormalized); glUniform1f(10,berserkSeedTime); glUniform1ui(11,Sys_Settings.Brightness);
    float shakeOffset = (World.shakeFinished > World.pauseRelativeTime) ? (0.15f * vcosf((float)(World.pauseRelativeTime * 20.0f))) * (World.shakeFinished - World.pauseRelativeTime) : 0.0f; glUniform3f(12,deg2rad(World.cam_yaw + shakeOffset),deg2rad(World.cam_pitch + shakeOffset * 0.5f),deg2rad(World.cam_roll)); glUniform3f(13,px,py,pz); glUniform1f(15,(float)World.pauseRelativeTime * 0.1f); glUniform1ui(17,(gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX) || World.curLev == LEVEL_CYBERSPACE);
    glUniform1ui(18,(gridCellStates[playerCellIdx] & CELL_SEES_SUN) && World.curLev != LEVEL_CYBERSPACE); glUniform1ui(19,((World.curLev >= 10 && World.curLev < LEVEL_CYBERSPACE) ? 1u : 0u) && (gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX));
    u32 shieldOnType = 0u/*No shield green tint*/; if (World.instances[WORLD].ioflags & Q_SHIELD_ACTIVATED) {shieldOnType=(World.curLev <= 5) ? 1u/*Shielding everywhere*/ : 2u/*Shielding only below, levels 6+*/;} glUniform1ui(20,shieldOnType); // Green Shield
    Color3 painStaticColor = (Color3){1.0f,0.0f,0.0f}; glUniform3f(23,painStaticColor.r,painStaticColor.g,painStaticColor.b);
    glUniformMatrix4fv(24,1,0,viewProj);          glUniformMatrix3fv(25,1,0,invViewRot);        glUniform1i(27,0);
    glUniform1f(28,vclamp(World.painStaticAlpha + World.empStaticAlpha,0.0f,1.0f));              glUniform1ui(29,(u32)ModRequestsGrayscale()); glBindVertexArray(quadVAO); glDisable(GL_DEPTH_TEST);
    glDrawArrays(0x0006/*GL_TRIANGLE_FAN*/,0,4); drawCalls++; vertsRendered += 4;
    glEndQuery(0x88BF/*GL_TIME_ELAPSED*/);
    if ((World.last_time - World.lastFrameSecCountTime) >= 1.00) { World.lastFrameSecCountTime=World.last_time; globalframesPerLastSecond=globalframe - World.lastFrameSecCount; World.lastFrameSecCount=globalframe; } // Update Diagnostic Poll
}

void RenderCameraViews() { // Render in-world camera views.  Pops player position to elsewhere, renders to tiny fbo, pops player back.
    if (unlikely(World.paused || World.menuActive || camViewCount == 0 || World.curLev >= LEVEL_CYBERSPACE)){return;}
    V3 tempPlayerPos = World.position[PLAYER1]; Quaternion tempPlayerRot = World.rotation[PLAYER1];
    for (int cm=0;cm<camViewCount;++cm) { if (camViews[cm].finished < World.pauseRelativeTime && camViews[cm].visible) { camViews[cm].finished = World.pauseRelativeTime + 0.5f; World.position[PLAYER1] = camViews[cm].position; World.rotation[PLAYER1] = camViews[cm].rotation; CullCore(); Render(true/*camview*/,cm); } }
    World.position[PLAYER1] = tempPlayerPos; World.rotation[PLAYER1] = tempPlayerRot; // Restore player for normal render.
}

void UpdateInstanceMatrix4x4s() {
    i32 dirtyMin = -1, dirtyMax = -1;
    for (u32 i = INSTS_1ST_IDX; i < World.instCount; i++) {        
        float x=World.rotation[i].x, y=World.rotation[i].y, z=World.rotation[i].z, w=World.rotation[i].w; float x2=x*x, y2=y*y, z2=z*z, xy=x*y, xz=x*z, yz=y*z, wx=w*x, wy=w*y, wz=w*z; float sclx=World.scale[i].x, scly=World.scale[i].y, sclz=World.scale[i].z; u32 m = i*16;
        modelMatrices[m+0]=(1.0f-2.0f*(y2+z2))*sclx; modelMatrices[m+1]=(2.0f*(xy+wz))*sclx; modelMatrices[m+2]=(2.0f*(xz-wy))*sclx; modelMatrices[m+3]=modelMatrices[m+7]=modelMatrices[m+11]=0.0f; modelMatrices[m+4]=(2.0f*(xy-wz))*scly; modelMatrices[m+5]=(1.0f-2.0f*(x2+z2))*scly; modelMatrices[m+6]=(2.0f*(yz+wx))*scly;
        modelMatrices[m+8]=(2.0f*(xz+wy))*sclz; modelMatrices[m+9]=(2.0f*(yz-wx))*sclz; modelMatrices[m+10]=(1.0f-2.0f*(x2+y2))*sclz; modelMatrices[m+12]=World.position[i].x; modelMatrices[m+13]=World.position[i].y; modelMatrices[m+14]=World.position[i].z; modelMatrices[m+15]=1.0f;
        if (dirtyMin < 0) {dirtyMin = (i32)i;} dirtyMax = (i32)i;
    }
    if (dirtyMin >= 0) { glBindBuffer(GL_SSBO,matricesBufferID); u32 offsetFloats=(u32)dirtyMin * 16; u32 countFloats=((u32)dirtyMax - (u32)dirtyMin + 1) * 16; glBufferSubData(GL_SSBO,offsetFloats * 4,countFloats * 4,modelMatrices + offsetFloats); }
}

static const Color fogLUT[MAX_LEVELS] = { {0.3207547f, 0.29200783f,0.29200783f,0.07f},/*0*/  {0.34509805f,0.38431373f,0.49019608f,0.055f},/*1*/  {0.47058824f,0.3882353f, 0.3928334f,0.05f},/*2*/  {0.32941177f,0.29411766f,0.2509804f,0.065f},/*3*/ {0.3882353f,0.452415f, 0.47058824f,0.075f},/*4*/
                                          {0.3882353f, 0.4117647f, 0.47058824f,0.03f},/*5*/  {0.3f,       0.24f,      0.33f,      0.070f},/*6*/  {0.38679248f,0.3471719f, 0.3302332f,0.07f},/*7*/  {0.44708973f,0.45681614f,0.4811321f,0.040f},/*8*/ {0.4056604f,0.3992963f,0.36930403f,0.050f},/*9*/
                                          {0.48235294f,0.58431375f,0.5176471f, 0.04f},/*10*/ {0.52872473f,0.58431375f,0.48235294f,0.040f},/*11*/ {0.48235294f,0.58431375f,0.5176471f,0.05f},/*12*/ {0.0f,       0.0f,       0.0f,      0.005f},/*13*/ };
                                          static const V2 levMins[MAX_LEVELS]={{-37.3600f,-52.7600f},/*0*/  {-53.8000f,-64.0800f},/*1*/  {-46.12f,-56.34f},/*2*/  {-51.266f,-51.246f},/*3*/  {-29.462f, -53.7872f},/*4*/ {-47.3622f,-55.04f},/*5*/ {-65.94f,-71.6833f},/*6*/ {-66.8989f,-82.0144f},/*7*/ {-43.7456f,-43.9872f},/*8*/ {-51.5039f,-69.0306f},/*9*/
                                     {-24.0994f,-39.7972f},/*10*/ {-27.1772f,-28.3394f},/*11*/ {-18.05f,-30.50f},/*12*/ {-64.000f,-60.120f}/*13*/};
static const float lFars[MAX_LEVELS] = { 56.32f/*R*/, 56.32f/*1*/, 51.2f/*2*/, 51.2f/*3*/, 40.96f/*4*/, 58.88f/*5*/, 79.36f/*6*/, 56.32f/*7*/, 69.12f/*8*/, 53.76f/*9*/,  51.2f/*10*/,  51.2f/*11*/, 38.4f/*12*/, 71.68f/*13*/};
int EdgeCompare(const void* a, const void* b) { u32 ea = *(const u32*)a, eb = *(const u32*)b; return (ea > eb) - (ea < eb); }
void AddHardwareToInventory(int index,int hwversion);
u16 uniqueCvxMeshIndices[MAX_UNIQUE_CVX_MESHES]; u32 uniqueCvxMeshCount=0;
// Init && Main
__attribute__((cold)) void NewGame() { // Reset World States
    DualLog("Loading new game...\n"); RenderLoading("Loading new game...");
    World.menuActive = World.paused = enteringPlayerName = fovSliderActive = gammaSliderActive = masterVolumeSliderActive = musicVolumeSliderActive = messageVolumeSliderActive = sfxVolumeSliderActive = returnToPause = false;
    for (int i=0;i<World.numLevels;++i) { World.worldMin_x[i] = levMins[i].x; World.worldMin_z[i] = levMins[i].y; World.voxMinCtrX[i] = World.worldMin_x[i] + VOXEL_HALF; World.voxMinCtrZ[i] = World.worldMin_z[i] + VOXEL_HALF; World.farPlane[i] = lFars[i]; World.fogColor[i] = fogLUT[i]; World.fogColor[i].a *= 3.8f; }
    SetLevelPointers(0);
    World.curLev = 0; World.mass[0] = 0.0f; World.dynamicFriction[0] = 0.4f; World.col[0]=COLTYPE_NONE; currentMenuItem = currentMenuTab = 0; currentMenuPage = Mpg_FrontPage;
    World.current_time = World.pauseRelativeTime = World.last_physics_time = World.pauseRelativeTime = World.last_physics_time=0.0; World.deltaTime=0.0166666666f; 
    mset(World.instances,0,3 * sizeof(Entity)); // Blank out player entities
    World.instances[PLAYER1].index = 767; World.layer[PLAYER1] = L_Player; World.scale[PLAYER1] = (V3){1.0f,1.0f,1.0f}; World.rotation[PLAYER1] = (Quaternion){0.0f,0.7071f,0.0f,0.7071f}; // 90deg rotation CW about Y axis as viewed from the top looking down onto player
    World.instances[PLAYER1].entflags = EF_ACTIVE|EF_RIGIDBODY; World.instances[PLAYER1].modelIndex = MAX_MDLS; World.col[PLAYER1] = COLTYPE_CAP; World.colliderCenter[PLAYER1].y = -PLAYER_CAM_OFFSET_Y; World.colliderSize[PLAYER1] = (V3){PLAYER_RADIUS,PLAYER_HEIGHT,COLCAP_DIR_Y_F}; // Radius, Overall height including end radii (Unity convention, blech), Direction, 1.0 == Y-Axis
    World.mass[PLAYER1] = 1.0f; World.velocity[PLAYER1] = (V3){0.0f,0.0f,0.0f}; World.cam_yaw = 90.0f; World.cam_pitch = World.cam_roll = World.invP1.leanTarget = World.invP1.leanShift = 0.0f; World.gravity[PLAYER1] = 1.0f; World.dynamicFriction[PLAYER1] = 0.6f; World.staticFriction[PLAYER1] = 0.8f; 
    World.instances[PLAYER1].health = 211.0f; World.invP1.noiseFinished = World.pauseRelativeTime; World.invP1.energy = 54.0f; World.invP1.energyDrainTickFinished = World.pauseRelativeTime + 0.1 + (double)random_range(0.5f,1.0f);
    World.invP1.hardwareInvReferenceIndex[0]  = 21; World.invP1.hardwareInvReferenceIndex[1]  = 22; World.invP1.hardwareInvReferenceIndex[2]  = 23; World.invP1.hardwareInvReferenceIndex[3]  = 24; World.invP1.hardwareInvReferenceIndex[4]  = 25; World.invP1.hardwareInvReferenceIndex[5]  = 26;
    World.invP1.hardwareInvReferenceIndex[6]  = 27; World.invP1.hardwareInvReferenceIndex[7]  = 28; World.invP1.hardwareInvReferenceIndex[8]  = 29; World.invP1.hardwareInvReferenceIndex[9]  = 30; World.invP1.hardwareInvReferenceIndex[10] = 31; World.invP1.hardwareInvReferenceIndex[11] = 32;
    World.invP1.hardwareInvReferenceIndex[12] =  0; World.invP1.hardwareInvReferenceIndex[13] =  0; World.invP1.generalInventoryIndexRef[0] = 81; // Hardcoded lookup indices into the Const main table.
    for (int i=1;i<HW_COUNT;i++) World.invP1.generalInventoryIndexRef[i] = -1; // Skips 0th index on purpose as it always holds access cards "item".
    for (int i=0;i<HW_COUNT;++i) World.invP1.hardwareVersion[i] = World.invP1.hardwareVersionSetting[i] = 0;
    World.invP1.nitroTimeSetting = NITRO_DEFAULT_TIME; World.invP1.earthShakerTimeSetting = EARTH_SHAKER_DEFAULT_TIME; World.invP1.lastAddedIndex = World.invP1.globalLookupIndex = -1; World.invP1.hasNewEmail = World.invP1.hasNewNotes = World.invP1.isPulserNotDrill = true;
    for (int i=0;i<7;++i) World.invP1.weaponInventoryIndices[i] = World.invP1.weaponInventoryAmmoIndices[i] = -1;
    World.invP1.sparqSetting = 50.0f; World.invP1.ionSetting = 100.0f; World.invP1.blasterSetting = 15.0f; World.invP1.plasmaSetting = 40.0f; World.invP1.stungunSetting = 20.0f; World.invP1.justFired = (World.pauseRelativeTime - 31.0); // Set >30s before pauseRelativeTime to not immediately play action music.
    World.invP1.resetAfterDeathTime = 0.5; World.invP1.painSoundFinished = World.invP1.radSoundFinished = World.invP1.radFXFinished = World.pauseRelativeTime; World.Sys_UI.lastMultiMediaTabOpened = MM_EMAIL_TABLE;
    World.Sys_UI.logFinished = World.pauseRelativeTime; World.Sys_UI.tickFinished = World.Sys_UI.centerTabsTickFinished = World.current_time + 0.1 + (double)random_range(0.0f,1.0f); World.Sys_UI.blinkFinished = 1.0 + World.pauseRelativeTime; World.Sys_UI.beepFinished = 3.0 + World.pauseRelativeTime;
    World.invP1.mediFinished = World.invP1.reflexFinishedTime = World.invP1.sightFinishedTime = -1.0; World.invP1.berserkIncrement = World.invP1.patchActive = 0; World.invP1.staminupActive = World.geniusActive = false; World.timeScale = DEFAULT_TIME_SCALE; 
    World.cam_yaw = 90.0f; World.cam_pitch = 0.0f; World.cam_roll = 0.0f; World.inventoryMode = Sys_Settings.NoShootMode; World.gameFinished = World.creditsActive = World.decoyActive = false; World.damageDealt = World.damageReceived = 0.0f;
    World.ressurections = World.deaths = World.kills = World.cyberkills = 0u; World.shotsFired = World.grenadesThrown = World.savesScummed = 0U; World.creditsPageIndex = 0u;
    for (int i=0;i<14;++i) {World.levelSecurity[i] = 100u;}
    mset(&Sys_Input,0,sizeof(Sys_Input)); World.currentMouse_dx = World.currentMouse_dy = 0; last_mouse_x = last_mouse_y = 0; ignore_next_mouse_delta = true;
    Sys_Input.lastUse = Sys_Input.isCapsLockOn = false; // As far as we're concerned, don't worry about OS capslock actual state.
    for (u8 lev = 1; lev < World.numLevels; ++lev) CopyPlayerState(0,lev);
    DebugRAM("before runtime LoadAllLevels"); LoadAllLevels(); DebugRAM("after runtime LoadAllLevels"); LoadLevel(World.startLevel,(V3){10.52f,-43.792f + 0.84f,20.2908f}); DebugRAM("after runtime LoadLevel"); World.invP1.currentCrouchRatio = 1.0f;
    for (u32 lev = 0; lev < MAX_LEVELS; ++lev) { // 1. Find unique convex mesh indices across all levels
        for (u32 i = 0; i < INSTANCE_COUNT; ++i) {
            World.levelInstances[lev][i].adjacencyIdx = U16_MAX;
            if (World.levelCollider[lev][i] == COLTYPE_CVX) {
                u16 colMeshIdx = World.levelInstances[lev][i].colMeshIndex;
                if (colMeshIdx > MAX_MDLS) {DualLogWarn("Improper convex mesh colMeshIndex on level %u, instance %u with constindex %u for convex mesh uniques, colMeshIndex: %u\n",lev,i,World.levelInstances[lev][i].index,World.levelInstances[lev][i].colMeshIndex); continue;}
                bool isUnique=true; u32 foundIdx=U16_MAX;
                for (u32 u=0;u<uniqueCvxMeshCount;++u) { if (uniqueCvxMeshIndices[u] == colMeshIdx) { isUnique = false; foundIdx = u; World.levelInstances[lev][i].adjacencyIdx = (u16)foundIdx; break; } }
                if (isUnique) { if (uniqueCvxMeshCount >= MAX_UNIQUE_CVX_MESHES) { DualLogWarn("Exceeded MAX_UNIQUE_CVX_MESHES!\n"); World.levelInstances[lev][i].adjacencyIdx=U16_MAX; continue; } uniqueCvxMeshIndices[uniqueCvxMeshCount]=colMeshIdx; World.levelInstances[lev][i].adjacencyIdx=(u16)uniqueCvxMeshCount; uniqueCvxMeshCount++; }
            }
        }
    } DebugRAM("before edge adjacency");
    for (u32 u = 0; u < uniqueCvxMeshCount; ++u) { // 2. Generate edge adjacency list for each unique mesh
        u16 m = uniqueCvxMeshIndices[u]; if (m >= MAX_MDLS) { continue;}
        u32 vCount = physVertCounts[m], tCount = modelTriangleCounts[m]; if (!vCount || !tCount || !physPos[m] || !physTris[m]) continue;
        u32 edgeCount = 0; u32* tempEdges = OS_Alloc(tCount * 3 * sizeof(u32));
        for (u32 t = 0; t < tCount; ++t) { u16 i0=physTris[m][t*3+0], i1=physTris[m][t*3+1], i2=physTris[m][t*3+2]; tempEdges[edgeCount++]=((u32)vmin(i0,i1) << 16) | vmax(i0,i1); tempEdges[edgeCount++]=((u32)vmin(i1,i2) << 16) | vmax(i1,i2); tempEdges[edgeCount++]=((u32)vmin(i2,i0) << 16) | vmax(i2,i0); }
        qsort_new(tempEdges,edgeCount,sizeof(u32),EdgeCompare); u32 uniqueEdgeCount=0; u32* degree=OS_Alloc(vCount * sizeof(u32)); 
        for (u32 i = 0; i < edgeCount; ++i) { if (i == 0 || tempEdges[i] != tempEdges[i-1]) { tempEdges[uniqueEdgeCount++]=tempEdges[i]; u16 a=(u16)(tempEdges[i] >> 16); u16 b=(u16)(tempEdges[i] & 0xFFFF); degree[a]++; degree[b]++; } }
        u32* offsets=OS_Alloc((vCount + 1) * sizeof(u32)); offsets[0]=0; for(u32 i=0;i<vCount;++i){offsets[i+1]=offsets[i] + degree[i];}
        u16* adjList = OS_Alloc(uniqueEdgeCount * 2 * sizeof(u16)); u32* writePos = OS_Alloc(vCount * sizeof(u32));
        mcpy(writePos, offsets, vCount * sizeof(u32));
        for (u32 i=0;i<uniqueEdgeCount;++i) { u16 a=(u16)(tempEdges[i] >> 16); u16 b=(u16)(tempEdges[i] & 0xFFFF); adjList[writePos[a]++]=b; adjList[writePos[b]++]=a; }
        cvxAdjOffsets[u]=offsets; cvxAdjLists[u]=adjList;
        OS_Free(tempEdges,tCount * 3 * sizeof(u32)); OS_Free(degree,vCount * sizeof(u32)); OS_Free(writePos,vCount * sizeof(u32));
    } DebugRAM("after edge adjacency");
    World.lev1SecCode = random_range_u8(0u,9u); World.lev2SecCode = random_range_u8(0u,9u); World.lev3SecCode = random_range_u8(0u,9u); World.lev4SecCode = random_range_u8(0u,9u); World.lev5SecCode = random_range_u8(0u,9u); World.lev6SecCode = random_range_u8(0u,9u); World.missionBits = 0; // Must do rand's repeatedly to prevent these all being the same number.
    firstFrameMouselook = true; // Prevent jumps after cursor is centered once menu turned off.
    //TESTING TODO REMOVE! AddHardwareToInventory(0,4); AddHardwareToInventory(1,4); AddHardwareToInventory(2,4); AddHardwareToInventory(3,4); AddHardwareToInventory(4,4); AddHardwareToInventory(5,4); AddHardwareToInventory(6,4); AddHardwareToInventory(7,4); AddHardwareToInventory(8,4); AddHardwareToInventory(9,4); AddHardwareToInventory(10,4); AddHardwareToInventory(11,4);
}

void PlayVmail(u8 i) { World.Sys_UI.vmailActive=i; World.Sys_UI.vmailFrame=vmailStartFrames[i]; World.Sys_UI.vmailFrameFinished=World.pauseRelativeTime + 0.1; ForceInventoryMode(); }
void GoIntoGame() { NewGame(); PlayGameMusic(); DualLog("Player named \"%s\" started the game!\n", World.playerName); game_actual_start_time = get_time(); }
void LoadModels(),LoadTextures(),InitAudio(),LoadConfig(),synth_set_room(float,float);
void InitalizeEnvironment() {
    game_start_time = get_time(); random_range_rng = (u32)game_start_time; /*Seed global rand uniquely with time since system boot.*/ console_log_file = OS_OpenWriteonly("./voxen.log"); // Initialize log system for all prints to go to both stdout and voxen.log file
    OS_ScratchInit(); // Set up the 465 MB scratch arena for init phases
    DebugRAM("program start"); DualLog("Voxen, the Voxel Lit Open Source Game Engine by W. Josiah Jack, MIT-0 licensed\nEntity size: %u\n",sizeof(Entity));
    SetLevelPointers(0); WindowInit(); threadCnt = clamp(OS_GetNumThreads(),1,32); globalframe=0,World.menuActive=true,World.screenshotTimeout=1.0,World.creditsPageIndex=1,World.diffCbt=World.diffCyb=World.diffPuz=World.diffMis=2,World.deaths=0,World.cursorPos_x=680,World.cursorPos_y=384;
    World.numLevels=MAX_LEVELS; World.startLevel=1/*medical*/; LoadConfig();/*Get settings before setting window size.*/ window = VCreateWindow(Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight); CenterWindowOnMonitor(); SetGLContext_GetFunctionPointers();
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); ((WSWin*)window)->context.swapBuffers(((WSWin*)window)); // Black out the window as early as possible for better presentation.
    i32 major=0,minor=0; glGetIntegerv(0x821B/*GL_MAJOR_VERSION*/,&major); glGetIntegerv(0x821C/*GL_MINOR_VERSION*/,&minor); if (major < 4 || (major == 4 && minor < 3)) { DualLogError("Need OpenGL >= 4.3, got %d.%d\n",major,minor); OS_Exit(1); }
    glFrontFace(0x0901/*GL_CCW*/); // Set triangle winding order
    glBlendFuncSeparate(0x0302/*GL_SRC_ALPHA*/, 0x0303/*GL_ONE_MINUS_SRC_ALPHA*/, 1, 0x0303/*GL_ONE_MINUS_SRC_ALPHA*/); glClearColor(0,0,0,1);
    CompileShaders();
    u32 tvaos[4],tvbos[4]; glGenVertexArrays(4,tvaos); glGenBuffers(4,tvbos); quadVAO=tvaos[0]; quadVBO=tvbos[0]; chunkVAO=tvaos[1]; chunkVBO=tvbos[1]; textVAO=tvaos[2]; textVBO=tvbos[2]; debugLinesVAO=tvaos[3]; debugLinesVBO=tvbos[3]; 
    float quadBlit_vertices[] = {1.0f,-1.0f,1.0f,0.0f, 1.0f,1.0f,1.0f,1.0f, -1.0f,1.0f,0.0f,1.0f, -1.0f,-1.0f,0.0f,0.0f}; // 4 verts, 4 floats each x,y,u,v
    glBindVertexArray(quadVAO); glBindBuffer(GL_ARRAY_BUFFER,quadVBO); glBufferData(GL_ARRAY_BUFFER,sizeof(quadBlit_vertices),quadBlit_vertices,GL_STATIC_DRAW);
    glVertexAttribFormat(0,2,GL_FLOAT,GL_FALSE,0);                 glVertexAttribBinding(0,0); glEnableVertexAttribArray(0); // pos xy float @ offset 0
    glVertexAttribFormat(1,2,GL_FLOAT,GL_FALSE,2 * sizeof(float)); glVertexAttribBinding(1,0); glEnableVertexAttribArray(1); // uv (s,t)
    glBindVertexBuffer(0,quadVBO,0,4 * sizeof(float));
    glBindVertexArray(chunkVAO);
    glVertexAttribFormat(0,3,0x140B/*GL_HALF_FLOAT*/,GL_FALSE,0);  glVertexAttribBinding(0,0); glEnableVertexAttribArray(0); // pos xyz half-float @ offset 0
    glVertexAttribFormat(1,3,0x140B/*GL_HALF_FLOAT*/,GL_FALSE,6);  glVertexAttribBinding(1,0); glEnableVertexAttribArray(1); // normal xyz float   @ offset 6  (after 3×2 bytes)
    glVertexAttribFormat(2,2,0x140B/*GL_HALF_FLOAT*/,GL_FALSE,12); glVertexAttribBinding(2,0); glEnableVertexAttribArray(2); // uv st float
    glBindVertexArray(textVAO);
    glVertexAttribFormat(0,3,GL_FLOAT,GL_FALSE,0);                 glVertexAttribBinding(0,0); glEnableVertexAttribArray(0); // pos (x,y,z) 4 floats per vertex, stride = 4*sizeof(float)
    glVertexAttribFormat(1,2,GL_FLOAT,GL_FALSE,3 * sizeof(float)); glVertexAttribBinding(1,0); glEnableVertexAttribArray(1); // uv (s,t)
    glBindVertexBuffer(0, textVBO,0,5 * sizeof(float));
    glBindVertexArray(debugLinesVAO); glBindBuffer(GL_ARRAY_BUFFER,debugLinesVBO); glBufferData(GL_ARRAY_BUFFER,MAX_WIRELINE_VRTS * 2 * sizeof(DebugLineVertex),NULL,GL_DYNAMIC_DRAW);
    glVertexAttribFormat(0,3,GL_FLOAT,GL_FALSE,__builtin_offsetof(DebugLineVertex,x)); glVertexAttribBinding(0,0); glEnableVertexAttribArray(0);
    glVertexAttribFormat(1,4,GL_FLOAT,GL_FALSE,__builtin_offsetof(DebugLineVertex,r)); glVertexAttribBinding(1,0); glEnableVertexAttribArray(1);
    glBindVertexBuffer(0,debugLinesVBO,0,sizeof(DebugLineVertex));
    InitFontAtlasses(); GenerateAndBindTexture(&inputUIID,GL_RGBA8,1366,768,GL_RGBA,GL_UNSIGNED_BYTE,0x2600/*GL_NEAREST*/,NULL);/*UI Fixed Size Raster*/
    glGenFramebuffers(1,&uiFBO); glBindFramebuffer(GL_FRAMEBUFFER,uiFBO); glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D,inputUIID); glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,inputUIID,0);
    u32 drawBuffersUI[] = {GL_COLOR_ATTACHMENT0}; glDrawBuffers(1,drawBuffersUI); glCheckFramebufferStatus(GL_FRAMEBUFFER); glBindImageTexture(0,inputUIID,0,GL_FALSE,0,GL_READ_WRITE,GL_RGBA8);/* UI Rendered Color*/ glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,inputUIID,0);
    RenderLoading("Loading...");
    float* m = shadowmapsPerspectiveProjection; float lightRangeMax=15.36f; float viewRange=(lightRangeMax - 0.02f);
    m[0]=1.0f; m[1]=0.0f; m[2]=0.0f; m[3]=0.0f; m[4]=0.0f; m[5]=1.0f; m[6]=0.0f; m[7]=0.0f; m[8]=0.0f; m[9]=0.0f; m[10]=-(lightRangeMax + 0.02f) / viewRange; m[11]=-1.0f; m[12]=0.0f; m[13]=0.0f; m[14]=-2.0f * lightRangeMax * 0.02f / viewRange; m[15]=0.0f;
    InitAudio(); synth_set_room(0.66f,0.8f);
    glGenFramebuffers(1,&gBufferFBO); ChangeFullScreenWindowed(false); SetSkyRotateSpeed(); SetVSync(); LoadTextForLanguage(Sys_Settings.Language); LoadLogTextForLanguage(Sys_Settings.Language);
    glBindFramebuffer(GL_FRAMEBUFFER,gBufferFBO); u32 drawBuffers[] = {GL_COLOR_ATTACHMENT0,GL_COLOR_ATTACHMENT1,GL_COLOR_ATTACHMENT2}; glDrawBuffers(3,drawBuffers);
    u32 status = glCheckFramebufferStatus(GL_FRAMEBUFFER); if (status != 0x8CD5/*GL_FRAMEBUFFER_COMPLETE*/) DualLogError("Framebuffer incomplete: Error code %d\n",status);
    float mat[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    mcpy(&modelMatrices[0],mat,16 * sizeof(float)); // Null instance matrix used for UI
    matricesBufferID = MakeSSBO(&matricesBufferID, 1,INSTANCE_COUNT * 16 * sizeof(float),modelMatrices,GL_STATIC_DRAW);     cellVisibleDataID= MakeSSBO(&cellVisibleDataID,7,ARRSIZE * sizeof(u32),NULL,GL_STATIC_DRAW); 
    voxListCntsID    = MakeSSBO(&voxListCntsID,    2,VOXEL_COUNT * sizeof(u32),NULL,GL_STATIC_DRAW);                        texPalID         = MakeSSBO(&texPalID,         8,MAX_UNIQUE_COLORS * sizeof(u32),NULL,GL_STATIC_DRAW);
    voxelLightListsID= MakeSSBO(&voxelLightListsID,3,VOXEL_COUNT * MAX_LIGHTS_PER_VOXEL * sizeof(u32),NULL,GL_STATIC_DRAW); texPalOfsID      = MakeSSBO(&texPalOfsID,      9,MAX_TXRS * sizeof(u32),NULL,GL_STATIC_DRAW);
    lightsID         = MakeSSBO(&lightsID,         4,LIGHT_COUNT * sizeof(Light),NULL,GL_STATIC_DRAW);                      colorBufferID    = MakeSSBO(&colorBufferID,   12,MAX_TOTAL_PIXELS * sizeof(u8),NULL,GL_STATIC_DRAW);
    if (Sys_Settings.Shadows) CreateShadowBuffers();/*5,6*/                                                                 textureOffsetsID = MakeSSBO(&textureOffsetsID,14,MAX_TXRS * sizeof(u32),NULL,GL_STATIC_DRAW);          textureSizesID = MakeSSBO(&textureSizesID,15,MAX_TXRS * 2 * sizeof(i32),NULL,GL_STATIC_DRAW);
    glUseProgram(shadowmapsSP); glUniform1ui(9,SHADOW_MAP_SIZE); glUseProgram(shadowmapsClearSP); glUniform1ui(0,SHADOW_MAP_SIZE); glUseProgram(chunkSP); glUniform1ui(21,SHADOW_MAP_SIZE); glUniform1f(22,(float)SHADOW_MAP_SIZE); glUniform1ui(23,LIGHT_COUNT); glUniform1ui(24,(u32)MAX_LIGHTS_PER_VOXEL); glUniform1ui(11,SHADOW_MAP_SIZE*SHADOW_MAP_SIZE); // One time set uniforms
    for (int f=0;f<5;++f) glGenQueries(5,gpuQ[f]);
    RenderLoading("Loading textures..."); DebugRAM("before LoadTextures"); LoadTextures(); DebugRAM("after LoadTextures"); RenderLoading("Loading models..."); DebugRAM("before LoadModels"); LoadModels(); DebugRAM("after LoadModels");
    if (World.introNotPlayed) { currentMenuPage = Mpg_IntroVideo; PlayMenuMusic(); World.menuActive = true; World.introNotPlayed = false; } World.absoluteTime = World.current_time = get_time(); World.pauseRelativeTime = World.last_physics_time = 0.0;
    NewGame();
    PlayMenuMusic(); World.menuActive = true; currentMenuPage = Mpg_FrontPage; // Comment out for immediate testing
    OS_ScratchFree(); DualLog("Game Initialized in %f secs\n",get_time() - game_start_time); DebugRAM("InitializeEnvironment after scratch free");
}

void Physics(float dt); void UpdateAnims(void); void UpdateAudio(); bool ScrshotPressed();
i32 main() {
    InitalizeEnvironment();
    while(1) {
        if (queuedLevelToLoad != 255u) { LoadLevel(queuedLevelToLoad,queuedLevelPos); queuedLevelToLoad = 255u; continue; }
        double curtime = get_time(); World.deltaTime=World.current_time < 0.001f ? 0.000f : vmax(curtime - World.current_time,0.0); World.absoluteTime+=World.deltaTime; World.current_time=curtime; World.painStaticAlpha = vmax(0.0f,World.painStaticAlpha - (float)World.deltaTime * 2.0f); World.empStaticAlpha = vmax(0.0f,World.empStaticAlpha - (float)World.deltaTime * 4.0f);
        if (!World.paused && !World.menuActive) { if (World.pauseRelativeTime < 0.001f) {World.pauseRelativeTime = World.last_physics_time = curtime;} World.pauseRelativeTime += World.deltaTime; }
        double input_start = get_time();
        InputProcessing(); // Before anims and physics to allow them to respond immediately.
        UpdateAnims();     // Before physics to allow model swap out to affect physics state immediately.  Before rendering to affect shadowmaps immediately.
        prePhys = get_time() - input_start;
        if (!World.paused && !World.menuActive) {
            double ps=get_time();
            float dt=(float)vclamp((World.pauseRelativeTime - World.last_physics_time),0.0005,0.1);
            World.last_physics_time=World.pauseRelativeTime; World.dt=dt;
            Physics(dt);
            physTime=get_time() - ps;
        } else physTime=0.0;
        double gameT_start = get_time();
        ModUpdate(); // After physics so mod/gamecode can modify velocities before next frame.
        if (World.invP1.hasHardware & HW_BIO) BioMonitorUpdate();
        UpdateAudio(); gameTime = get_time() - gameT_start;
        if (likely(!World.paused && !World.menuActive)) UpdateInstanceMatrix4x4s(); // Before camviews so camview shadows render same as main pass
        drawCalls=uiDrawCalls=shadDrawCalls=vertsRendered=0; RenderCameraViews(); if (likely(!World.paused && !World.menuActive)) CullCore();
        Render(false/*!camview*/,0u);
        if (ScrshotPressed() && World.current_time > World.screenshotTimeout) Screenshot();
        for(i32 i=0;i<MAX_KEYS;++i){Sys_Input.keyStates[i].pressed=Sys_Input.keyStates[i].released=false;} for (i32 i=0;i<MAX_MOUSE_BUTTONS;i++) {Sys_Input.mouseButtons[i].pressed=Sys_Input.mouseButtons[i].released=false;} Sys_Input.scrollDelta=0; World.currentMouse_dx=World.currentMouse_dy=0; // Reset Input states, can't mset as we want to preserve down state
        globalframe++; World.cpuTime = get_time() - World.current_time; // Measure time over everything this frame before GPU swap buffers for diagnostic text.
        if (globalframe > 4) { u8 r=(gpuQFrame+1)%5; u64 v;
          glGetQueryObjectui64v(gpuQ[r][0],0x8866/*GL_QUERY_RESULT*/,&v); World.gpuShadowMs=(double)v * 0.000001; glGetQueryObjectui64v(gpuQ[r][1],0x8866/*GL_QUERY_RESULT*/,&v); World.gpuPreMs=(double)v * 0.000001;
          glGetQueryObjectui64v(gpuQ[r][2],0x8866/*GL_QUERY_RESULT*/,&v); World.gpuMainMs=(double)v * 0.000001;   glGetQueryObjectui64v(gpuQ[r][3],0x8866/*GL_QUERY_RESULT*/,&v); World.gpuSsrMs=(double)v * 0.000001;
          glGetQueryObjectui64v(gpuQ[r][4],0x8866/*GL_QUERY_RESULT*/,&v); World.gpuCompMs=(double)v * 0.000001;
          World.gpuFrameMs=World.gpuShadowMs+World.gpuPreMs+World.gpuMainMs+World.gpuSsrMs+World.gpuCompMs; } gpuQFrame=(gpuQFrame+1)%5;
        ((WSWin*)window)->context.swapBuffers(((WSWin*)window)); // Present frame (almost always waiting for GPU since GPU bound).
        CHECK_GL_ERROR(); // Lone catch for inadvertent issues.
        { static const u32 dbgFrm[] = {4,100,200,500,1000}; static const char* dbgLbl[] = {"frame 4","frame 100","frame 200","frame 500","frame 1000"}; for (int d=0;d<5;d++) if (globalframe == dbgFrm[d]) {DebugRAM(dbgLbl[d]); if (globalframe == 1000) break;} }
    }
    return 0;
}
