// voxen.c - A realtime OpenGL 4.3+ Game Engine for Citadel: The System Shock Fan Remake.  Main translation unit.  Main loop.  Core rendering engine.  OS Shim Layer.  Windowing and Input.  Config.  Cheats Console.  Core systems init.
#include "common.h"
#include "credits.h"
#include "Shaders/shaders.h"
GlobalContext World = {0}; // Full game state
double game_start_time, game_actual_start_time, shadowTime, physTime, renderTime, prePhys, gameTime, raycastMs;
float rasterPerspectiveProjection[16],modelMatrices[INSTANCE_COUNT*16],*world_from_mdl = modelMatrices,modelBounds[MAX_MDLS],**physPos,lightView[MAX_SHADOWMAPS][6][4][4],lightViewProj[MAX_SHADOWMAPS][6][16],shadowmapsPerspectiveProjection[16],sunShadowMatrix[16],berserkSeedTime;
static float shakeAmp = 0.15f,amMapScaleX,amMapScaleY,amMapOffsetX;
static u32 globalframesPerLastSecond,inputImageID,inputUIID,inputDepthID,inputSpecID,inputNormalID,outputImageID,gBufferFBO,uiSP,depthPrepassSP,chunkSP,chunkVAO,matricesBufferID,ssrSP,rtSP,rtCopySP,imageBlitSP,quadVAO,quadVBO,rtInstanceInfoID,rtCellCountsID,rtCellListsID,rtGeomID,rtOrderBufID,gpuQ[5][5],wireVertCount,raycastPickMode;
u32 globalframe,threadCnt = 1,*physVertCounts,*rtSrcVertCounts,uiFBO,textDecalVBO[MAX_LEVELS][INSTANCE_COUNT],textSP,textVAO,textVBO,textDecalVertexCount[MAX_LEVELS][INSTANCE_COUNT],lightsID,voxListCntsID,voxelLightListsID,voxelUpdateSP,cellVisibleDataID,shadowmapsSP,shadowMapSSBO,shadowMapsIndirectionID,sunShadowFBO,sunShadowTex,shadowmapIndirectionList[LIGHT_COUNT],colorBufferID,texPalID,texPalOfsID,textureOffsetsID,textureSizesID,psysInstancesID,
    psysTrailsID,psysquadVAO,psysquadVBO,particleSP,trailSP,camViewTextures[64],levelCamViewTextures[14][64],drawCalls,uiDrawCalls,shadDrawCalls,vertsRendered,drawCallsNormal,biomTexId,biomFBO,raycastCalls,debugUnlitSP,debugLinesVAO,debugLinesVBO,wireLinesVAO,wireLinesVBO;
u16 **modelTriangles, modelTriangleCounts[MAX_MDLS], mdlsCnt, **physTris, texCnt, decalStyleCount, decalInlineTextLevel[DECAL_INLINE_TEXT_MAX], decalInlineTextInst[DECAL_INLINE_TEXT_MAX], decalInlineTextCount;
static u8 currentPlayerNameLength,currentMenuPage,resDropdownCount,resSelectedIdx,gpuQFrame,uiMouseCaptured;
static i8 currentMenuItem,currentMenuTab, menuItemCount = 4, menuTabCount = 1, resHoverIdx = -1;
u8 particleBlendTexture[MAXTEX],camViewCount,levelCamViewCount[14];
char decalInlineText[DECAL_INLINE_TEXT_MAX][DECAL_INLINE_TEXT_LEN];
bool doubleSidedTexture[MAXTEX],transparentTexture[MAXTEX],shadowBuffersCreated,resDropdownOpen,mouseMovementThisFrame,window_has_focus,ignore_next_mouse_delta,returnToPause,fovSliderActive,gammaSliderActive,masterVolumeSliderActive,musicVolumeSliderActive,messageVolumeSliderActive,sfxVolumeSliderActive,enteringPlayerName;
typedef struct { u32 nodeBase,triBase,vtxBase,uvBase,orderBase; } ModelRTInfo; typedef struct { u32 geom[4]; u32 info[4]; float iscl2[4]; } InstanceRTInfo; static InstanceRTInfo rtInstanceInfo[INSTANCE_COUNT]; static InstanceRTInfo rtPrevInfo[INSTANCE_COUNT]; static ModelRTInfo rtModelInfo[MAX_MDLS];
V3 modelMin[MAX_MDLS], modelMax[MAX_MDLS],sunDirection={0.0f,1.0f,0.0f}; FrustumPlane lightFrustumPlanes[MAX_SHADOWMAPS][6][6],playerFrustumPlanes[6],sunShadowFrustum[6]; DecalStyle decalStyles[DECAL_STYLE_MAX]; CamView camViews[64], levelCamViews[14][64];
typedef struct {int w,h;} ResMode; ResMode resModes[16];
SettingsSystem Sys_Settings = { // Potato defaults so initial state is good on first run for potatoes (e.g. won't crash for out of VRAM, or won't take 5min to init).
    .InputCodeSettings = { 5,/*Fwd=F*/      0,/*Strf L=A*/   18,/*Backpedal=S*/ 3,/*Strf R=D*/ 100,/*Jump=SPACE*/        2,/*Crouch=C*/        23,/*Prone=X*/   16,/*Lean L=Q*/       4,/*Lean Right=E*/ 45,/*Sprint=LSHIFT*/ 38,/*Turn L=LARROW*/ 39,/*Turn Right=RARROW*/ 36,/*Look Up=UARROW*/ 37,/*Look Down=DARROW*/ 20,/*Recent Log=U*/ 26,/*Biomonitor=1*/ 27,/*Sensaround=2*/ 28,/*Lantern=3*/ 29,/*Shield=4*/     30,/*Infrared=5*/      31,/*Email=6*/
                          32,/*Booster=7*/ 33,/*Jumpjets=8*/ 56,/*Attack=LMB*/ 57,/*Use=RMB*/   99,/*Menu/Back=ESCAPE*/ 80,/*Toggle Mode=TAB*/ 17,/*Reload=R*/ 127,/*Wep+=MWHEEL+*/ 128,/* Wep-=MWHEEL-*/ 6,/* Grenade=G*/    19,/*Gren+= T*/     131,/*Gren-=*/            21,/*Ammo Type=V*/     9,/*Patch Use=J*/       8,/*Patch+=I*/    132,/*Patch-=,*/     12,/*Full Map=M*/   21,/*Swim Up= V*/ 2,/*Swim Down=C*/ 101,/*Screenshot=F12*/ 102,/*Console=GRAVE*/},
    .ScreenWidth=800u,.ScreenHeight=600u,.Fullscreen=0u,.FOV=65u,.Brightness=50u,.FXAA=0u,.Shadows=0u,.Reflections=0u,.Vsync=0u,.ModelDetail=0u,.CurrentMonitor=0u, .GI=0u,.SpeakerMode=1u,.Reverb=0u,.VolumeMaster=100u,.VolumeMusic=25u,.VolumeMessage=75u,.VolumeEffects=100u,.Language=0u,.DynamicMusic=1u,.Footsteps=1u,.InvertLook=0u,.InvCybLook=0u,.QuickItemPickup=0u,.QuickReloadWeapons=0u,.MouseSensitivity=10u,.NoShootMode=0u,.HeadBob=1u,.SSR_RES=4u};/*Ratio is (1 / SSR_RES) * res*/
InputSystem Sys_Input; TextSystem Sys_Text; CheatsSystem Cheats = {.god=false, .noclip=false, .showLocation=false, .showFPS=true, .editMode=false, .showPhys=false};
u16 editModeSelection = U16_MAX, editModeLightSelection = U16_MAX, editModeTestEntityDefinition = 343, lastSpawned = U16_MAX;
Color textColors[] = {{1.000000000f,1.000000000f,1.000000000f,1.0f}, /* 0 White T_WHITE*/                             {0.890196078f,0.874509804f,0.000000000f,1.0f}, /* 1 Yellow T_YELLOW*/                              {0.623529412f,0.611764706f,0.000000000f,1.0f}, /* 2 Dark Yellow (Yellow * 0.7f) T_DARK_YELLOW*/ {0.372549020f,0.654901961f,0.168627451f,1.0f}, /* 3 Green T_GREEN*/ {0.917647059f,0.137254902f,0.168627451f,1.0f}, /* 4 Red T_RED*/
                      {1.000000000f,0.498039216f,0.000000000f,1.0f}, /* 5 Orange T_ORANGE*/                           {0.674509804f,0.058823529f,0.070588235f,1.0f}, /* 6 StopD Red T_STOPD_RED*/                        {0.941176471f,0.282352941f,0.298039216f,1.0f}, /* 7 StopD Red Highlight T_STOPD_RED_HIGHLIGHT*/ {0.909803922f,0.203921569f,0.219607843f,1.0f}, /* 8 StopD Red Pause Title T_STOPD_RED_PAUSETITLE*/
                      {0.470588235f,0.721568627f,0.172549020f,1.0f}, /* 9 Green Menu Title T_GREEN_MENU*/             {0.137254902f,0.356862745f,0.109803922f,1.0f}, /* 10 Green Menu Title Shadow T_GREEN_MENU_SHADOW*/ {0.239215686f,0.466666667f,0.129411765f,1.0f}, /* 11 Green Menu Title Glow T_GREEN_MENU_GLOW*/  {0.392156863f,0.031372549f,0.039215686f,1.0f}, /* 12 Red Menu Text Dark T_RED_MENU*/
                      {0.594339600f,0.232689545f,0.232689545f,1.0f}, /* 13 Intro/Outro Subtitle #973B3B T_VIDEOTEXT*/ {0.000000000f,0.039215688f,0.835294200f,1.0f}, /* 14 PuzzleWire actualColorBlue T_BLUE*/           {0.546864600f,0.152723400f,0.830188700f,1.0f}  /* 15 PuzzleWire actualColorPurple T_PURPLE*/ };
// Wireline Rendering
typedef struct { float x,y,z,r,g,b,a; } WireVrt; static WireVrt* lineVerts; static WireVrt* wireVerts;
static void EmitWireQuad(V3 c0, V3 c1, V3 c2, V3 c3, Color c) { const V3 p[6] = {c0,c1,c2, c0,c2,c3}; for (int v = 0; v < 6; ++v) { WireVrt* w = &wireVerts[wireVertCount++]; w->x = p[v].x; w->y = p[v].y; w->z = p[v].z; w->r = c.r; w->g = c.g; w->b = c.b; w->a = c.a; } }
void DrawCrossedLine(V3 a, V3 b, V3 perp, float halfThick, Color c) { V3 o0 = V3_ScaleByF(perp,halfThick); if (wireVertCount + 6 < (u32)(MAX_WIRELINE_QUADS*6)) EmitWireQuad(V3_AsubB(a,o0),V3Add(a,o0),V3Add(b,o0),V3_AsubB(b,o0),c); V3 q = V3_Normalize(V3_Cross(V3_Normalize(V3_AsubB(b,a)),perp)); V3 r0 = V3_ScaleByF(q,halfThick); if (wireVertCount + 6 < (u32)(MAX_WIRELINE_QUADS*6)) EmitWireQuad(V3_AsubB(a,r0),V3Add(a,r0),V3Add(b,r0),V3_AsubB(b,r0),c); }
void DrawLine(V3 s, V3 e, Color c) { if (World.lineVertCnt >= MAX_LINEVRTS - 2) { return; } int i = World.lineVertCnt; lineVerts[i++] = (WireVrt){ s.x, s.y, s.z, c.r, c.g, c.b, c.a }; lineVerts[i++] = (WireVrt){ e.x, e.y, e.z, c.r, c.g, c.b, c.a }; World.lineVertCnt = i; }
INLINE Color ColliderColor(u16 i) { return (!(World.instances[i].entflags & EF_RIGIDBODY) || World.physSleep[i] != 0) ? textColors[T_GREEN_MENU_SHADOW] : ((World.colliding[i]) ? textColors[T_RED] : textColors[T_GREEN]); }
void DrawVelV3(u16 i) { if (!(World.instances[i].entflags & EF_RIGIDBODY)) {return;} V3 tip = V3Add(World.position[i],V3_ScaleByF(World.velocity[i],0.25f)); DrawLine(World.position[i],tip,textColors[T_ORANGE]); V3 perp = V3_Normalize(V3_Cross(World.velocity[i],(vabs(World.velocity[i].y/V3_Mag(World.velocity[i])) < 0.9f) ? (V3){0,1,0} : (V3){1,0,0})); DrawLine(V3Add(tip,V3_ScaleByF(perp,0.05f)),V3_AsubB(tip,V3_ScaleByF(perp,0.05f)),textColors[T_ORANGE]); }
void DrawBoxColliderColored(u16 i, Color col) {
    ShapeBox b = Entity_GetBox(i); V3 c[8]; V3 px = V3_ScaleByF(quat_rot_v3(b.rot,(V3){1,0,0}),b.hExt.x); V3 py = V3_ScaleByF(quat_rot_v3(b.rot,(V3){0,1,0}),b.hExt.y); V3 pz = V3_ScaleByF(quat_rot_v3(b.rot,(V3){0,0,1}),b.hExt.z); for (int s=0;s<8;s++) { float sx=(s&1)?1.f:-1.f,sy=(s&2)?1.f:-1.f,sz=(s&4)?1.f:-1.f; c[s]=V3Add(b.ctr,V3Add(V3Add(V3_ScaleByF(px,sx),V3_ScaleByF(py,sy)),V3_ScaleByF(pz,sz))); }
    DrawLine(c[0],c[1],col); DrawLine(c[2],c[3],col); DrawLine(c[4],c[5],col); DrawLine(c[6],c[7],col); DrawLine(c[0],c[2],col); DrawLine(c[1],c[3],col); DrawLine(c[4],c[6],col); DrawLine(c[5],c[7],col); DrawLine(c[0],c[4],col); DrawLine(c[1],c[5],col); DrawLine(c[2],c[6],col); DrawLine(c[3],c[7],col); DrawVelV3(i);
}
void DrawSphereWireframe(Color col, ShapeSphere s) { float step = 6.28318530f / 12; for (int seg=0;seg<12;seg++) { float a0 = seg * step, a1 = a0 + step, c0 = vcosf(a0), s0 = vsinf(a0), c1 = vcosf(a1), s1 = vsinf(a1); DrawLine(V3Add(s.ctr,(V3){c0*s.rad,0,s0*s.rad}),V3Add(s.ctr,(V3){c1*s.rad,0,s1*s.rad}),col); DrawLine(V3Add(s.ctr,(V3){c0*s.rad,s0*s.rad,0}),V3Add(s.ctr,(V3){c1*s.rad,s1*s.rad,0}),col); DrawLine(V3Add(s.ctr,(V3){0,c0*s.rad,s0*s.rad}),V3Add(s.ctr,(V3){0,c1*s.rad,s1*s.rad}),col); } }
void DrawSphereCollider(u16 i) { DrawSphereWireframe(ColliderColor(i),Entity_GetSph(i)); DrawVelV3(i); }
void DrawSphereContact(V3 pos, float rad) { if (Cheats.showPhys) { DrawSphereWireframe((Color){0.0f,0.0f,1.0f,1.0f},(ShapeSphere){pos,rad}); } }
void DrawMeshCollider(u16 i) {
    u16 mi = (World.col[i] == CT_CVX) ? World.instances[i].colMeshIndex : World.instances[i].modelIndex; if (mi >= mdlsCnt) { return; } u32 triCount = modelTriangleCounts[mi]; if(!triCount){return;}
    const float* M=&modelMatrices[i*16]; float m00=M[0],m10=M[1],m20=M[2],m01=M[4],m11=M[5],m21=M[6],m02=M[8],m12=M[9],m22=M[10],tx=M[12],ty=M[13],tz=M[14]; const float* pos = physPos[mi]; const u16* tris = modelTriangles[mi]; Color col = ColliderColor(i);
    for (u32 j=0; j<triCount; j++) { V3 w[3]; u32 b=j*3; for (int k=0;k<3;++k) { u32 vi=tris[b + k]; float x=pos[vi*3 + 0]; float y=pos[vi*3 + 1]; float z=pos[vi*3 + 2]; w[k]=(V3){m00*x + m01*y + m02*z + tx, m10*x + m11*y + m12*z + ty, m20*x + m21*y + m22*z + tz}; } DrawLine(w[0],w[1],col); DrawLine(w[1],w[2],col); DrawLine(w[2],w[0],col); }    DrawVelV3(i);
}
void DrawCapsuleCollider(u16 i) {
    Color col = ColliderColor(i); ShapeCapsule cap = Entity_GetCap(i); V3 diff = V3_AsubB(cap.tip, cap.base); V3 axis = (vabs(diff.x) + vabs(diff.y) + vabs(diff.z) > 0.0001f) ? V3_Normalize(diff) : (V3){0.0f, 1.0f, 0.0f}; V3 ref = (vabs(axis.y) < 0.9f) ? (V3){0,1,0} : (V3){1,0,0}; V3 perp0 = V3_Normalize(V3_Cross(axis, ref)); V3 perp1 = V3_Cross(axis, perp0); float step = 6.28318530f / 12, r = cap.rad;
    for (int seg=0;seg<12;++seg) { /*Draw top and bottom rings*/ float a0=seg * step, a1=a0 + step; float c0=vcosf(a0), s0=vsinf(a0), c1=vcosf(a1), s1=vsinf(a1); V3 r0=V3Add(V3_ScaleByF(perp0,c0*r),V3_ScaleByF(perp1,s0*r)); V3 r1=V3Add(V3_ScaleByF(perp0,c1*r),V3_ScaleByF(perp1,s1*r)); DrawLine(V3Add(cap.base,r0),V3Add(cap.base,r1),col); DrawLine(V3Add(cap.tip,r0),V3Add(cap.tip,r1),col); }
    for (int seg=0;seg<6;++seg) { /*Draw the hemispheres*/
        float a0=seg*step, a1=a0+step; float c0=vcosf(a0), s0=vsinf(a0), c1=vcosf(a1), s1=vsinf(a1);
        DrawLine(V3Add(cap.base,V3Add(V3_ScaleByF(perp0,c0*r),V3_ScaleByF(axis,-s0*r))),V3Add(cap.base,V3Add(V3_ScaleByF(perp0,c1*r),V3_ScaleByF(axis,-s1*r))),col); DrawLine(V3Add(cap.base,V3Add(V3_ScaleByF(perp1,c0*r),V3_ScaleByF(axis,-s0*r))),V3Add(cap.base,V3Add(V3_ScaleByF(perp1,c1*r),V3_ScaleByF(axis,-s1*r))),col);
        DrawLine(V3Add(cap.tip,V3Add(V3_ScaleByF(perp0,c0*r),V3_ScaleByF(axis,s0*r))),V3Add(cap.tip,V3Add(V3_ScaleByF(perp0,c1*r),V3_ScaleByF(axis,s1*r))),col); DrawLine(V3Add(cap.tip,V3Add(V3_ScaleByF(perp1,c0*r),V3_ScaleByF(axis,s0*r))),V3Add(cap.tip,V3Add(V3_ScaleByF(perp1,c1*r),V3_ScaleByF(axis,s1*r))),col);
    }
    for (int seg=0;seg<4;++seg) { /*Draw the longitudinal lines*/ float a = seg * (6.28318530f / 4.f); V3 off = V3Add(V3_ScaleByF(perp0,vcosf(a)*r),V3_ScaleByF(perp1,vsinf(a)*r)); DrawLine(V3Add(cap.base,off),V3Add(cap.tip,off),col); }
    DrawVelV3(i);
}
void DrawAngularVelocity(u16 i) {
    if (!(World.instances[i].entflags & EF_RIGIDBODY) || V3_Mag(World.angularVelocity[i]) < 0.0001f) return;/*skip near-zero*/
    Color purple = (Color){0.5f,0.0f,1.0f,1.0f};
    V3 dir = V3_Normalize(World.angularVelocity[i]);
    V3 tip = V3Add(World.position[i],V3_ScaleByF(World.angularVelocity[i],0.35f)); DrawLine(World.position[i],tip,purple); // Arrow (line vector)
    V3 ref = (vabs(dir.y) < 0.9f) ? (V3){0,1,0} : (V3){1,0,0};
    V3 perp=V3_Normalize(V3_Cross(dir,ref)); V3 perp2 = V3_Cross(dir,perp);
    DrawLine(V3Add(tip,V3_ScaleByF(perp, 0.05f)),V3Add(tip,V3_ScaleByF(perp, -0.05f)), purple);
    DrawLine(V3Add(tip,V3_ScaleByF(perp2,0.05f)),V3Add(tip,V3_ScaleByF(perp2,-0.05f)), purple);
    float rad = 0.6f/*Quarter circle arc (visualizes rotation plane + sense)*/, step = 1.57079632679f / 8.0f; /*quarter circle divided into 8 segments*/
    V3 axis = dir; V3 p1 = V3_Normalize(V3_Cross(axis,ref)); V3 p2 = V3_Cross(axis,p1), prev = V3Add(World.position[i], V3_ScaleByF(p1,rad)); // Find two vectors perpendicular to angular axis
    for (int j=1;j<=8;++j) { float a = j * step; float c = vcosf(a), s = vsinf(a); V3 cur = V3Add(World.position[i],V3Add(V3_ScaleByF(p1,c * rad),V3_ScaleByF(p2,s * rad))); DrawLine(prev,cur,purple); prev = cur; }
}
// WinSys Windowing and Input System interfacing with the OS.
typedef struct WSWin WSWin; static WSWin* window; typedef struct WSCtx WSCtx; typedef struct WSLib WSLib; typedef struct WSMon WSMon; static WSLib WinSys; typedef void (*WSP)(void);
void CycleWeaponSlot(int),CloseFullmap(),UI_AutomapClick(bool,int),UpdateScreenSize(i32,i32),SaveConfig(),InputWindowFocus(i32),InputKey(char*,int,int),InputMouseClick(char*,int,int),InputCursorPos(double*,double*,double,double),InputMonitor(WSMon*,int,int),ComputeWindowPlacement(int,int*,int*);
#if defined(_WIN32)
    int __cdecl wcscmp(const u16*,const u16*); u16* wcscpy(u16*,const u16*);
    typedef struct { i32 x,y; } POINT;
    typedef struct { i32 l,t,r,b; } RECT;
    typedef struct { u32 s,maj,min; u8 _p[264]; u16 sp; u8 _p2[6]; } OSVERSIONINFOEXW;
    typedef struct _devicemodeW { u16 a[34]; u16 dmSize,b; u32 c; POINT dmPosition; u8 d[18]; u16 e[33]; u32 f,dmPelsWidth,dmPelsHeight,g,dmDisplayFrequency; u8 h[32]; } DEVMODEW,*LPDEVMODEW;
    typedef i64 (__stdcall *WNDPROC)(void*,u32,u64,i64);
    typedef i32 (__stdcall *MONITORENUMPROC)(void*,void*,RECT*,i64);
    typedef struct _ICONINFO { i32 fIcon; u32 xHotspot,yHotspot; void *hbmMask,*hbmColor; } ICONINFO;
    typedef struct { u8 a[8]; u32 message; u8 b[4]; u64 wParam; i64 lParam; u32 time; u8 c[12]; } MSG;
    typedef struct { u32 cbSize; RECT rcMonitor,rcWork; u8 b[4]; } MONITORINFO,*LPMONITORINFO;
    typedef struct { u32 cbSize; u8 a[36]; u16 szDevice[32]; } MONITORINFOEXW;
    typedef struct { u32 length; u8 a[4]; u32 showCmd; u8 b[16]; RECT rcNormalPosition; } WINDOWPLACEMENT;
    typedef struct { u32 cbSize,style; WNDPROC lpfnWndProc; i32 a,b; HINSTANCE hInstance; void *c,*d,*e; u16 *f,*n; void *g; } WNDCLASSEXW;
    typedef i32 (WINAPI * PFN_DwmIsCompositionEnabled)(i32*),(WINAPI * PFN_DwmFlush)(),(WINAPI * PFN_RtlVerifyVersionInfo)(OSVERSIONINFOEXW*,u32,u64),(WINAPI * PFN_SWE)(int),(WINAPI * PFN_GPFAIVA)(void*,int,int,u32,const int*,int*),(WINAPI * PFN_wglMakeCurrent)(void*,void*);
    typedef PROC (WINAPI * PFN_wglGetProcAddress)(const char*);
    typedef void *(WINAPI * FP_CCAA)(void*,void*,const int*),*(WINAPI * PFN_CC)(void*),*(WINAPI * PFN_wglGetCurrentDC)(),*(WINAPI * PFN_wglGetCurrentContext)();
    typedef struct WGLContext { void* dc; void* handle; int interval; } WGLContext;
    PFN_wglGetCurrentDC wglGetCurrentDC; PFN_CC wglCreateContext; PFN_wglGetCurrentContext wglGetCurrentContext; PFN_wglMakeCurrent wglMakeCurrent; PFN_wglGetProcAddress wglGetProcAddress; PFN_GPFAIVA GPFAivARB; PFN_SWE wglSwapIntervalEXT;
    typedef struct WSWinWin32 { void* handle; bool frameAction; int lastCurX,lastCurY; } WSWinWin32;
    typedef struct{int redBits,greenBits,blueBits,alphaBits,depthBits,stencilBits; uintptr_t handle;} FBC;
    typedef struct WSLibWin32 { HINSTANCE instance; void* helperWindowHandle; u16 helperWindowClass,mainWindowClass; short int keycodes[512]; double restoreCurPosX,restoreCurPosY; WSWin *disabledCursorWindow, *capturedCursorWindow; struct { HINSTANCE instance; PFN_DwmIsCompositionEnabled IsCompositionEnabled; PFN_DwmFlush Flush; } dwmapi; struct { HINSTANCE instance; PFN_RtlVerifyVersionInfo RtlVerifyVersionInfo; } ntdll; } WSLibWin32;
    typedef struct WSMonWin32 { void* handle; u16 adapterName[32],displayName[32]; int isPrimary; } WSMonWin32;
    typedef struct _DISPLAY_DEVICEW { u32 cb; u16 DeviceName[32],DeviceString[128]; u32 StateFlags; u8 _p[256]; } DISPLAY_DEVICEW;
    typedef struct { u16 nSize,nVersion; u32 dwFlags; u8 _p[32]; } PIXELFORMATDESCRIPTOR;
    typedef struct { u32 bV5Size; i32 bV5Width,bV5Height; u16 bV5Planes,bV5BitCount; u32 bV5Compression; u8 _p[20]; u32 bV5RedMask,bV5GreenMask,bV5BlueMask,bV5AlphaMask; u8 _p2[52]; } BITMAPV5HEADER;
    DLL_IMP void* WINAPI CreateIconIndirect(ICONINFO*);                       DLL_IMP void* WINAPI GetDC(void*); DLL_IMP i32 WINAPI GetModuleHandleExW(u32,const u16*,HINSTANCE*); DLL_IMP int WINAPI ReleaseDC(void*,void*); DLL_IMP i32 WINAPI SetCursorPos(int,int); DLL_IMP void* WINAPI SetCursor(void*); DLL_IMP i32 WINAPI GetCursorPos(POINT*);
    DLL_IMP int WINAPI MultiByteToWideChar(u32,u32,const char*,int,u16*,int); DLL_IMP i32 WINAPI ClipCursor(const RECT*); DLL_IMP i32 WINAPI ClientToScreen(void*,POINT*); DLL_IMP void* WINAPI CreateDCW(const u16*,const u16*,const u16*,const DEVMODEW*); DLL_IMP void* WINAPI GetPropW(void*,u16*); DLL_IMP i32 WINAPI GetMessageTime(); DLL_IMP i32 WINAPI GetClientRect(void*,RECT*); DLL_IMP void* WINAPI LoadCursorW(HINSTANCE,u16*);
    DLL_IMP u32 WINAPI MapVirtualKeyW(u32,u32); DLL_IMP i32 WINAPI SetWindowPos(void*,void*,int,int,int,int,u32); DLL_IMP void* WINAPI SetCapture(void* hWnd); DLL_IMP i32 WINAPI ReleaseCapture(); DLL_IMP i32 WINAPI PeekMessageW(MSG*,void*,u32,u32,u32); DLL_IMP i32 WINAPI GetWindowLongW(void*,int); DLL_IMP i64 WINAPI DefWindowProcW(void*,u32,u64,i64);
    DLL_IMP i32 WINAPI AdjustWindowRectEx(RECT*,u32,i32,u32); DLL_IMP i64 WINAPI SendMessageW(void*,u32,u64,i64); DLL_IMP i32 WINAPI SetWindowLongW(void*,int,i32); DLL_IMP i32 WINAPI GetMonitorInfoW(void*,LPMONITORINFO); DLL_IMP i32 WINAPI TranslateMessage(const MSG*); DLL_IMP i16 WINAPI GetKeyState(int); DLL_IMP i64 WINAPI DispatchMessageW(const MSG*); DLL_IMP i32 WINAPI ShowWindow(void*,int);
    DLL_IMP i32 WINAPI BringWindowToTop(void*); DLL_IMP i32 WINAPI SetWindowPlacement(void*,const WINDOWPLACEMENT*); DLL_IMP void* WINAPI SetFocus(void*); DLL_IMP i32 WINAPI SetForegroundWindow(void*); DLL_IMP i32 WINAPI GetWindowPlacement(void*,WINDOWPLACEMENT*); DLL_IMP i32 WINAPI SetPropW(void*,u16*,void*); DLL_IMP i32 WINAPI OffsetRect(RECT*,int,int); DLL_IMP void* WINAPI CreateWindowExW(u32,u16*,u16*,u32,int,int,int,int,void*,void*,HINSTANCE,void*);
    DLL_IMP u64 WINAPI VerSetConditionMask(u64,u32,u8); DLL_IMP u16 WINAPI RegisterClassExW(const WNDCLASSEXW *); DLL_IMP i32 WINAPI DeleteObject(void*); DLL_IMP i32 WINAPI DeleteDC(void*); DLL_IMP i32 WINAPI SwapBuffers(void*); DLL_IMP i32 WINAPI EnumDisplayMonitors(void*,const RECT*,MONITORENUMPROC,i64); DLL_IMP i32 WINAPI EnumDisplaySettingsW(u16*,u32,LPDEVMODEW); DLL_IMP i32 WINAPI EnumDisplayDevicesW(u16*,u32,DISPLAY_DEVICEW*,u32);
    DLL_IMP i32 WINAPI EnumDisplaySettingsExW(u16*,u32,LPDEVMODEW,u32); DLL_IMP i32 WINAPI SetPixelFormat(void*,i32,const PIXELFORMATDESCRIPTOR *); DLL_IMP i32 WINAPI ChoosePixelFormat(void* hdc,const PIXELFORMATDESCRIPTOR *ppfd); DLL_IMP i32 WINAPI DescribePixelFormat(void*,i32,u32,PIXELFORMATDESCRIPTOR*); DLL_IMP void* WINAPI CreateBitmap(i32,i32,u32,u32,const void *); DLL_IMP void* WINAPI CreateDIBSection(void*,void*,u32,void**,void*,u32);
    DLL_IMP i32 WINAPI GetDeviceCaps(void*,i32);
    struct WSLib { WSMon** monitors; int monitorCount; WSLibWin32 win32; HINSTANCE instance; };
    struct WSCtx { void (*makeCurrent)(WSWin*); void (*swapBuffers)(WSWin*); void (*swapInterval)(int); WSP (*getProcAddress)(const char*); WGLContext wgl; }; 
    struct WSWin { i32 decorated; char mouseButtons[MAX_MOUSE_BUTTONS],keys[WS_KEYCOUNT]; double virtualCursorPosX,virtualCursorPosY; WSCtx context; WSWinWin32 win32; };
    struct WSMon { WSMonWin32 win32; };
    void SetWindowIcon(const WinSysIcon* image) {
        void *dc, *handle, *color, *mask; u8 *target=NULL, *source=image->pixels; BITMAPV5HEADER bi={0}; bi.bV5Size=sizeof(bi); bi.bV5Width=image->width; bi.bV5Height=-image->height; bi.bV5Planes=1; bi.bV5BitCount=32; bi.bV5Compression=3; bi.bV5RedMask=0xff0000; bi.bV5GreenMask=0xff00; bi.bV5BlueMask=0xff; bi.bV5AlphaMask=0xff000000;
        dc=GetDC(NULL); color=CreateDIBSection(dc,&bi,0,(void**)&target,NULL,0); ReleaseDC(NULL,dc); mask=CreateBitmap(image->width,image->height,1,1,NULL); for (int i=0;i<image->width*image->height;i++) { target[0]=source[2]; target[1]=source[1]; target[2]=source[0]; target[3]=source[3]; target+=4; source+=4; }
        ICONINFO ii={0}; ii.fIcon=1; ii.xHotspot=0; ii.yHotspot=0; ii.hbmMask=mask; ii.hbmColor=color; handle=CreateIconIndirect(&ii); DeleteObject(color); DeleteObject(mask); SendMessageW(window->win32.handle,0x80,1,(i64)handle); SendMessageW(window->win32.handle,0x80,0,(i64)handle);
    }
    static void updateCursorImage(WSWin* w) { if (WinSys.win32.disabledCursorWindow != w) { SetCursor(LoadCursorW(NULL, (u16*)(uintptr_t)32512)); } else { SetCursor(NULL); } }
    static void captureCursor(WSWin* w) { RECT r; GetClientRect(w->win32.handle,&r); ClientToScreen(w->win32.handle,(POINT*)&r.l); ClientToScreen(w->win32.handle,(POINT*)&r.r); ClipCursor(&r); WinSys.win32.capturedCursorWindow=w; }
    static void releaseCursor() { ClipCursor(NULL); WinSys.win32.capturedCursorWindow=NULL; }
    static void disableCursor(WSWin* w) { WinSys.win32.disabledCursorWindow = w; POINT p; GetCursorPos(&p); WinSys.win32.restoreCurPosX = p.x; WinSys.win32.restoreCurPosY = p.y; updateCursorImage(w); captureCursor(w); }
    static void SetCurV(WSWin* w, double x, double y) { w->win32.lastCurX = (int)x; w->win32.lastCurY = (int)y; POINT p = {(int)x,(int)y}; ClientToScreen(w->win32.handle,&p); SetCursorPos(p.x,p.y); }
    static void enableCursor(WSWin* w) { WinSys.win32.disabledCursorWindow = NULL; releaseCursor(); SetCurV(w,WinSys.win32.restoreCurPosX,WinSys.win32.restoreCurPosY); updateCursorImage(w); }
    static i64 __stdcall windowProc(void* h, u32 m, u64 w, i64 l) {
        WSWin* win=GetPropW(h,L"WinSys"); if (!win) return DefWindowProcW(h,m,w,l);
        switch (m) {
            case 0x21: if (((u16)((((u64)(l))>>16)&0xFFFF)) == 0x201 && ((u16)(((u64)(l)) & 0xFFFF))!=1) win->win32.frameAction=1; break;
            case 0x215: if (l==0&&win->win32.frameAction) { disableCursor(win); win->win32.frameAction=0; } break;
            case 0x7: InputWindowFocus(1); if (win->win32.frameAction) break; disableCursor(win); return 0;
            case 0x8: enableCursor(win); InputWindowFocus(0); return 0;
            case 0x112: switch(w&0xFFF0){ case 0xF140:case 0xF170:break; case 0xF100:return 0; } break;
            case 0x10: OS_Exit(0);
            case 0x100: case 0x104: case 0x101: case 0x105: {
                const int action=(((u16)((((u64)(l))>>16)&0xFFFF)) & 0x8000)?INPUT_RELEASE:INPUT_PRESS; int scancode=(((u16)((((u64)(l))>>16)&0xFFFF)) & (0x100|0xff)); if (!scancode) scancode=MapVirtualKeyW((u32)w,0); if (scancode==0x54) scancode=0x137; if (scancode==0x146) scancode=0x45; if (scancode==0x136) scancode=0x36; int key = WinSys.win32.keycodes[scancode];
                if (w==0x11) {if (((u16)((((u64)(l))>>16)&0xFFFF)) & 0x100) key=KEY_RIGHT_CONTROL; else { MSG g; const u32 time=GetMessageTime(); if (PeekMessageW(&g,NULL,0,0,0)) { if (g.message == 0x100 || g.message == 0x104 || g.message == 0x101 || g.message == 0x105) { if(g.wParam == 0x12 && (((u16)((((u64)g.lParam)>>16)&0xFFFF)) & 0x100)&&g.time==time){break;} } } key=KEY_LEFT_CONTROL; }} else if (w == 0xE5) break;
                if (action == INPUT_RELEASE && w == 0x10) { InputKey(win->keys,KEY_LEFT_SHIFT,action); InputKey(win->keys,KEY_RIGHT_SHIFT,action); } else if (w == 0x2C) { InputKey(win->keys,key,INPUT_PRESS); InputKey(win->keys,key,INPUT_RELEASE); } else InputKey(win->keys,key,action);
                break; }
            case 0x201: case 0x204: case 0x207: case 0x20B: case 0x202: case 0x205: case 0x208: case 0x20C: {
                int i,action,button = (m==0x201 || m == 0x202) ? MOUSE_BUTTON_LEFT : ((m == 0x204 || m == 0x205) ? MOUSE_BUTTON_RIGHT : ((m == 0x207 || m == 0x208) ? MOUSE_BUTTON_MIDDLE : (((((u16)((((u64)(w))>>16)&0xFFFF))) == 0x1) ? MOUSE_BUTTON_4 : MOUSE_BUTTON_5))); action=(m == 0x201 || m == 0x204 || m == 0x207 || m == 0x20B) ? INPUT_PRESS : INPUT_RELEASE;
                i=0; while(i<8 && win->mouseButtons[i]!=INPUT_PRESS) i++; if (i>7) SetCapture(h); InputMouseClick(win->mouseButtons,button,action); i=0; while(i<8 && win->mouseButtons[i]!=INPUT_PRESS) i++; if (i>7) ReleaseCapture(); if (m == 0x20B || m == 0x20C) return 1; return 0; }
            case 0x200: {const int x=((int)(i16)(l & 0xFFFF)), y=((int)(i16)(l >> 16)); const int dx=x-win->win32.lastCurX,dy=y-win->win32.lastCurY; if (WinSys.win32.disabledCursorWindow!=win) break; InputCursorPos(&win->virtualCursorPosX,&win->virtualCursorPosY,win->virtualCursorPosX+dx,win->virtualCursorPosY+dy); win->win32.lastCurX=x; win->win32.lastCurY=y; return 0; }
            case 0x20A: Sys_Input.scrollDelta += (i16)(((u16)((((u64)(w))>>16)&0xFFFF)))/(double)120; return 0;
            case 0x5: if (w == 1) World.paused = true; return 0;
            case 0x3: if (WinSys.win32.capturedCursorWindow==win) captureCursor(win); return 0;
            case 0x86: case 0x85: if (!win->decorated) return 1; break;
            case 0x20: if (((u16)(((u64)(l)) & 0xFFFF)) == 1) { updateCursorImage(win); return 1; } break;
            case 0x84: { i64 hit = DefWindowProcW(h,m,w,l); if (hit >= 10 && hit <= 17) return 1; return hit; }
        } return DefWindowProcW(h,m,w,l);
    }
    static void GetWindowPos(WSWin* w, int* x, int* y) { POINT p={0,0}; ClientToScreen(w->win32.handle,&p); *x=p.x; *y=p.y; }
    static void GetWindowSize(WSWin* w, int* w_, int* h) { RECT a; GetClientRect(w->win32.handle,&a); *w_=a.r; *h=a.b; }
    static void SetWindowSize(int w_, int h) { RECT r={0,0,w_,h}; AdjustWindowRectEx(&r,0x60A0000 | (window->decorated ? 0xC00000 : 0x80000000),0,0); SetWindowPos(window->win32.handle,NULL,0,0,r.r-r.l,r.b-r.t,0x216); }
    static void SetWindowMonitor(int x, int y, int wd, int h) { RECT r = {x,y,x+wd,y+h}; u32 s=GetWindowLongW(window->win32.handle,-16),f=0x110; if (window->decorated) { s&=~0x80000000,s|=(0x60A0000 | 0xC00000),SetWindowLongW(window->win32.handle,-16,s),f|=0x20; } AdjustWindowRectEx(&r,0x60A0000 | (window->decorated ? 0xC00000 : 0x80000000),0,0); SetWindowPos(window->win32.handle,(void*)-2,r.l,r.t,r.r-r.l,r.b-r.t,f); }
    static void SetWindowDecorated(WSWin* w,i32 e) {
        (void)e; RECT r; u32 s = GetWindowLongW(w->win32.handle,-16); s &= ~(0xC00000|0x80000|0x40000|0x20000|0x10000|0x80000000); s |= (0x60A0000|(window->decorated ? 0xC00000 : 0x80000000)); GetClientRect(w->win32.handle,&r); AdjustWindowRectEx(&r,s,0,0); ClientToScreen(w->win32.handle,(POINT*)&r.l); ClientToScreen(w->win32.handle,(POINT*)&r.r); SetWindowLongW(w->win32.handle,-16,s); SetWindowPos(w->win32.handle,NULL,r.l,r.t,r.r-r.l,r.b-r.t,0x34);
    }
    static void PollEvents() {
        MSG m; while (PeekMessageW(&m,NULL,0,0,0x1)) { if (m.message==0x12) OS_Exit(0); else { TranslateMessage(&m); DispatchMessageW(&m); } } const int k[4][2]={{0xA0,KEY_LEFT_SHIFT},{0xA1,KEY_RIGHT_SHIFT},{0x5B,KEY_LEFT_SUPER},{0x5C,KEY_RIGHT_SUPER}}; for (int i=0;i<4;i++) { if ((GetKeyState(k[i][0])&0x8000)||window->keys[k[i][1]]!=INPUT_PRESS) continue; InputKey(window->keys,k[i][1],INPUT_RELEASE); }
        if (WinSys.win32.disabledCursorWindow==window) { int W,H; GetWindowSize(window,&W,&H); if (window->win32.lastCurX != W/2 || window->win32.lastCurY != H/2) SetCurV(window,W/2,H/2); }/*Only re-center while the cursor is actually captured*/
    }
    WSP PlatformGetModuleSymbol(void* m, const char* n) { return (WSP)GetProcAddress((HMODULE)m,n); }
    typedef struct {u16 i; i32 v;} WinKeyRemap;
    static const WinKeyRemap wkm[]={{0xB,KEY_0},{0x2,KEY_1},{0x3,KEY_2},{0x4,KEY_3},{0x5,KEY_4},{0x6,KEY_5},{0x7,KEY_6},{0x8,KEY_7},{0x9,KEY_8},{0xA,KEY_9},{0x1E,KEY_A},{0x30,KEY_B},{0x2E,KEY_C},{0x20,KEY_D},{0x12,KEY_E},{0x21,KEY_F},{0x22,KEY_G},{0x23,KEY_H},{0x17,KEY_I},{0x24,KEY_J},{0x25,KEY_K},{0x26,KEY_L},{0x32,KEY_M},{0x31,KEY_N},{0x18,KEY_O},{0x19,KEY_P},{0x10,KEY_Q},{0x13,KEY_R},{0x1F,KEY_S},{0x14,KEY_T},{0x16,KEY_U},{0x2F,KEY_V},{0x11,KEY_W},
                                    {0x2D,KEY_X},{0x15,KEY_Y},{0x2C,KEY_Z},{0x28,KEY_APOSTROPHE},{0x2B,KEY_BACKSLASH},{0x33,KEY_COMMA},{0xD,KEY_EQUAL},{0x29,KEY_GRAVE_ACCENT},{0x1A,KEY_LEFT_BRACKET},{0xC,KEY_MINUS},{0x34,KEY_PERIOD},{0x1B,KEY_RIGHT_BRACKET},{0x27,KEY_SEMICOLON},{0x35,KEY_SLASH},{0xE,KEY_BACKSPACE},{0x153,KEY_DELETE},{0x14F,KEY_END},{0x1C,KEY_ENTER},{0x1,KEY_ESCAPE},{0x147,KEY_HOME},{0x152,KEY_INSERT},{0x15D,KEY_MENU},{0x151,KEY_PAGE_DOWN},
                                    {0x149,KEY_PAGE_UP},{0x45,KEY_PAUSE},{0x39,KEY_SPACE},{0xF,KEY_TAB},{0x3A,KEY_CAPS_LOCK},{0x145,KEY_NUM_LOCK},{0x46,KEY_SCROLL_LOCK},{0x3B,KEY_F1},{0x3C,KEY_F2},{0x3D,KEY_F3},{0x3E,KEY_F4},{0x3F,KEY_F5},{0x40,KEY_F6},{0x41,KEY_F7},{0x42,KEY_F8},{0x43,KEY_F9},{0x44,KEY_F10},{0x57,KEY_F11},{0x58,KEY_F12},{0x38,KEY_LEFT_ALT},{0x1D,KEY_LEFT_CONTROL},{0x2A,KEY_LEFT_SHIFT},{0x15B,KEY_LEFT_SUPER},{0x137,KEY_PRINT_SCREEN},
                                    {0x138,KEY_RIGHT_ALT},{0x11D,KEY_RIGHT_CONTROL},{0x36,KEY_RIGHT_SHIFT},{0x15C,KEY_RIGHT_SUPER},{0x150,KEY_DOWN},{0x14B,KEY_LEFT},{0x14D,KEY_RIGHT},{0x148,KEY_UP},{0x52,KEY_KP_0},{0x4F,KEY_KP_1},{0x50,KEY_KP_2},{0x51,KEY_KP_3},{0x4B,KEY_KP_4},{0x4C,KEY_KP_5},{0x4D,KEY_KP_6},{0x47,KEY_KP_7},{0x48,KEY_KP_8},{0x49,KEY_KP_9},{0x4E,KEY_KP_ADD},{0x53,KEY_KP_DECIMAL},{0x135,KEY_KP_DIVIDE},{0x11C,KEY_KP_ENTER},{0x59,KEY_KP_EQUAL},
                                    {0x37,KEY_KP_MULTIPLY},{0x4A,KEY_KP_SUBTRACT}};
    u16* CreateWideStringFromUTF8Win32(const char* s) { u16* t; int c = MultiByteToWideChar(65001,0,(char*)s,-1,NULL,0); t = OS_Alloc(c*sizeof(u16)); MultiByteToWideChar(65001,0,(char*)s,-1,t,c); return t; }
    i32 IsWindowsVersionOrGreaterWin32(u16 major, u16 minor, u16 sp) { OSVERSIONINFOEXW o={0}; o.s=sizeof(o), o.maj=major, o.min=minor, o.sp=sp; u64 c=VerSetConditionMask(VerSetConditionMask(VerSetConditionMask(0,0x2,3),0x1,3),0x20,3); return WinSys.win32.ntdll.RtlVerifyVersionInfo(&o,0x23,c)==0; }
    static i32 __stdcall monitorCallback(void* h, void* c, RECT* r, i64 d) { (void)c; (void)r; MONITORINFOEXW mi; mset(&mi,0,sizeof(mi)); mi.cbSize = sizeof(mi); if (GetMonitorInfoW(h,(MONITORINFO*)&mi)) { WSMon* m = (WSMon*)d; u16* dn = m->win32.displayName[0] ? m->win32.displayName : m->win32.adapterName; int match=1; for(int k=0;;++k){ if(!dn[k]){break;} if(mi.szDevice[k]!=dn[k]){match=0; break;} } if (match) m->win32.handle = h; } return 1; }
    static WSMon* createMonitor(DISPLAY_DEVICEW* a, DISPLAY_DEVICEW* d) {
        WSMon* m; void* dc; RECT r; DEVMODEW dm; mset(&dm,0,sizeof(dm)); dm.dmSize=sizeof(dm);
        EnumDisplaySettingsW(a->DeviceName,0xFFFFFFFFU,&dm);
        dc = CreateDCW(L"DISPLAY",a->DeviceName,NULL,NULL); DeleteDC(dc);
        m = OS_Alloc(sizeof(WSMon)); m->win32.isPrimary = ((a->StateFlags & 0x4) || (d && (d->StateFlags & 0x4))) ? 1 : 0; wcscpy(m->win32.adapterName, a->DeviceName); if (d) wcscpy(m->win32.displayName,d->DeviceName);
        r.l=dm.dmPosition.x; r.t=dm.dmPosition.y; r.r=dm.dmPosition.x + dm.dmPelsWidth; r.b=dm.dmPosition.y + dm.dmPelsHeight; EnumDisplayMonitors(NULL,&r,monitorCallback,(i64)m); 
        return m;
    }
    void WinSysPollMonitorsWin32() {
        int i, dC = WinSys.monitorCount; WSMon** d = NULL; u32 aI,dI; DISPLAY_DEVICEW a, dp; WSMon* m;
        if (dC) { d = OS_Alloc(WinSys.monitorCount*sizeof(WSMon*)); mcpy(d,WinSys.monitors,WinSys.monitorCount * sizeof(WSMon*)); }
        for (aI = 0;;aI++) {
            mset(&a,0,sizeof(a)); a.cb = sizeof(a); if (!EnumDisplayDevicesW(NULL,aI,&a,0)) break; if (!(a.StateFlags&1)) continue;
            const int aType = (a.StateFlags & 0x4) ? 0 : 1;/*DISPLAY_DEVICE_PRIMARY_DEVICE lives on the adapter, and 0 means insert-first in InputMonitor*/
            for (dI=0;;++dI) {
                mset(&dp,0,sizeof(dp)); dp.cb=sizeof(dp); if(!EnumDisplayDevicesW(a.DeviceName,dI,&dp,0))break; if(!(dp.StateFlags&1))continue;
                for(i=0;i<dC;++i){if(d[i]&&wcscmp(d[i]->win32.displayName,dp.DeviceName)==0){d[i]=NULL; EnumDisplayMonitors(NULL,NULL,monitorCallback,(i64)WinSys.monitors[i]); break;}} if(i<dC)continue; m=createMonitor(&a,&dp); if(!m){OS_Free(d,WinSys.monitorCount*sizeof(WSMon*)); return;} InputMonitor(m,0x40001,(m->win32.isPrimary ? 0 : 1));
            } if (dI == 0) {for (i=0;i<dC;++i) { if (d[i] && wcscmp(d[i]->win32.adapterName,a.DeviceName) == 0) {d[i]=NULL; break;} } if (i < dC) continue; m = createMonitor(&a,NULL); if (!m) { OS_Free(d,WinSys.monitorCount*sizeof(WSMon*)); return; } InputMonitor(m,0x40001,aType);}
        } for (i=0;i<dC;++i) { if (d[i]) InputMonitor(d[i],0x40002,0); } if (d) OS_Free(d,WinSys.monitorCount*sizeof(WSMon*));
        WSMon* sel=(Sys_Settings.CurrentMonitor<WinSys.monitorCount) ? WinSys.monitors[Sys_Settings.CurrentMonitor] : NULL;
          int pi=-1;
          for (i=0;i<WinSys.monitorCount;++i){ if (WinSys.monitors[i]->win32.isPrimary){pi=i; break;} }
          if (pi>0){ WSMon* pm=WinSys.monitors[pi]; mmov(WinSys.monitors+1,WinSys.monitors,(size_t)pi*sizeof(WSMon*)); WinSys.monitors[0]=pm; }
          if (sel){ for (i=0;i<WinSys.monitorCount;++i){ if (WinSys.monitors[i]==sel){Sys_Settings.CurrentMonitor=(u8)i; break;} } }
    }
    static i64 __stdcall helperWindowProc(void* h, u32 m, u64 w, i64 l) { if (m == 0x7E) WinSysPollMonitorsWin32(); return DefWindowProcW(h,m,w,l); }
    static void GetMonitorFullarea(WSMon* m, int* x, int* y, int* w, int* h) { DEVMODEW dm; mset(&dm,0,sizeof(dm)); dm.dmSize=sizeof(dm); EnumDisplaySettingsExW(m->win32.adapterName,0xFFFFFFFFU,&dm,0x4); *x=dm.dmPosition.x; *y=dm.dmPosition.y; *w=dm.dmPelsWidth; *h=dm.dmPelsHeight; }
    static void GetMonitorWorkarea(WSMon* m, int* x, int* y, int* w, int* h) {
        MONITORINFO mi={0}; mi.cbSize=sizeof(mi); if (m->win32.handle && GetMonitorInfoW(m->win32.handle,&mi) && mi.rcWork.r>mi.rcWork.l && mi.rcWork.b>mi.rcWork.t) { *x=mi.rcWork.l; *y=mi.rcWork.t; *w=mi.rcWork.r - mi.rcWork.l; *h=mi.rcWork.b - mi.rcWork.t; return; }
        DEVMODEW dm; mset(&dm,0,sizeof(dm)); dm.dmSize=sizeof(dm); EnumDisplaySettingsExW(m->win32.adapterName,0xFFFFFFFFU,&dm,0x4);*x=dm.dmPosition.x; *y=dm.dmPosition.y; *w=dm.dmPelsWidth; *h=dm.dmPelsHeight;
    }
    static void makeContextCurrentWGL(WSWin* w) { wglMakeCurrent(w->context.wgl.dc,w->context.wgl.handle); }
    static void swapBuffersWGL(WSWin* w) { if (!IsWindowsVersionOrGreaterWin32(((u8)((((u64)(0x602))>>8)&0xff)),((u8)(((u64)(0x602))&0xff)),0)) { i32 e = 0; if ((i32)(WinSys.win32.dwmapi.IsCompositionEnabled(&e) >= 0) && e) { int c = vabs(w->context.wgl.interval); while (c--) WinSys.win32.dwmapi.Flush(); } } SwapBuffers(w->context.wgl.dc); }
    static void swapIntervalWGL(int i) { window->context.wgl.interval = i; if (!IsWindowsVersionOrGreaterWin32(((u8)((((u64)(0x602))>>8)&0xff)),((u8)(((u64)(0x602))&0xff)),0)) { i32 e = 0; if ((i32)(WinSys.win32.dwmapi.IsCompositionEnabled(&e) >= 0) && e) i = 0; } wglSwapIntervalEXT(i); }
    static WSP getProcAddressWGL(const char* p) { const WSP proc = (WSP)wglGetProcAddress(p); if (proc) return proc; return (WSP)PlatformGetModuleSymbol(WinSys.instance,p); }
    static void SetWindowPosition(WSWin* w, int x, int y) { RECT r = {x,y,x,y}; AdjustWindowRectEx(&r,0x60A0000 | (w->decorated ? 0xC00000 : 0x80000000),0,0x40000); SetWindowPos(w->win32.handle,NULL,r.l,r.t,0,0,0x215); }
    const FBC* ChooseFBConfig(const FBC* a, u32 c) {
        u32 lowest = -1; const FBC* f = 0;
        for (u32 i=0;i<c;++i) { const FBC* x = a + i; u32 score = (((x->alphaBits == 0) + (x->depthBits == 0) + (x->stencilBits == 0)) * 65536) + (((8 - x->redBits) * (8 - x->redBits) + (8 - x->greenBits) * (8 - x->greenBits) + (8 - x->blueBits) * (8 - x->blueBits)) * 256) + ((8 - x->alphaBits) * (8 - x->alphaBits) + (8 - x->depthBits) * (8 - x->depthBits) + (8 - x->stencilBits) * (8 - x->stencilBits)); if (score < lowest) { lowest = score; f = x; } }
        return f;
    }
    WSWin* VCreateWindow(WSWin* w, int width, int height) {
        u32 style = 0x60A0000 | 0xC00000; WNDCLASSEXW wc= (WNDCLASSEXW){sizeof(wc),0x23,windowProc,0,0,WinSys.win32.instance,NULL,NULL,(void*)4,NULL,L"Voxen",NULL};
        WinSys.win32.mainWindowClass=RegisterClassExW(&wc);
        RECT r={0,0,width,height}; PIXELFORMATDESCRIPTOR pfd; void *prc,*rc,*pdc,*dc; int fW=r.r-r.l, fH=r.b-r.t; u16* wt = CreateWideStringFromUTF8Win32(GAME_TITLE); w->win32.handle=CreateWindowExW(0,(u16*)(uintptr_t)WinSys.win32.mainWindowClass,wt,style,0x80000000,0x80000000,fW,fH,NULL,NULL,WinSys.win32.instance,NULL);
        SetPropW(w->win32.handle,L"WinSys",w);
        WINDOWPLACEMENT wp = {0}; wp.length = sizeof(wp); AdjustWindowRectEx(&r,style,0,0); GetWindowPlacement(w->win32.handle,&wp); OffsetRect(&r,wp.rcNormalPosition.l-r.l,wp.rcNormalPosition.t-r.t); wp.rcNormalPosition = r; wp.showCmd = 0; SetWindowPlacement(w->win32.handle,&wp);
        WinSys.instance=LoadLibraryA("opengl32.dll");
        wglCreateContext     = (PFN_CC)PlatformGetModuleSymbol(                  WinSys.instance,"wglCreateContext");
        wglGetProcAddress    = (PFN_wglGetProcAddress)PlatformGetModuleSymbol(   WinSys.instance,"wglGetProcAddress");
        wglGetCurrentDC      = (PFN_wglGetCurrentDC)PlatformGetModuleSymbol(     WinSys.instance,"wglGetCurrentDC");
        wglGetCurrentContext = (PFN_wglGetCurrentContext)PlatformGetModuleSymbol(WinSys.instance,"wglGetCurrentContext");
        wglMakeCurrent       = (PFN_wglMakeCurrent)PlatformGetModuleSymbol(      WinSys.instance,"wglMakeCurrent");
        dc = GetDC(WinSys.win32.helperWindowHandle);
        mset(&pfd,0,sizeof(pfd)); pfd.nSize=sizeof(pfd); pfd.dwFlags=0x25;
        SetPixelFormat(dc,ChoosePixelFormat(dc,&pfd),&pfd);
        rc=wglCreateContext(dc);
        pdc=wglGetCurrentDC();
        prc=wglGetCurrentContext();
        wglMakeCurrent(dc,rc);
        FP_CCAA CCAARB     = (FP_CCAA)wglGetProcAddress("wglCreateContextAttribsARB");
        wglSwapIntervalEXT = (PFN_SWE)wglGetProcAddress("wglSwapIntervalEXT");
        GPFAivARB          = (PFN_GPFAIVA)wglGetProcAddress("wglGetPixelFormatAttribivARB");
        wglMakeCurrent(pdc,prc);
        if (!CCAARB || !GPFAivARB) { DualLogError("Required WGL extensions missing (WGL_ARB_create_context / WGL_ARB_pixel_format) - GPU driver too old or Microsoft Basic Display is active\n"); OS_Exit(1); } /*Compat: clean exit instead of NULL-deref crash on Win7 basic display / pre-4.3 drivers*/
        int pixelFormat; PIXELFORMATDESCRIPTOR pfd2; w->context.wgl.dc = GetDC(w->win32.handle); static const int attribs2[] = {0x2010,0x2001,0x2013,0x2003,0x2011,0x2015,0x2017,0x2019,0x201b,0x2022,0x2023}; int values[11], i, nativeCount, usableCount = 0; const int query = 0x2000;
        GPFAivARB(w->context.wgl.dc,1,0,1,&query,&nativeCount); FBC* usableConfigs = OS_Alloc(nativeCount*sizeof(FBC));
        for (i = 0; i < nativeCount; i++) { 
            FBC* u = usableConfigs + usableCount; pixelFormat = i + 1; GPFAivARB(w->context.wgl.dc,pixelFormat,0,11,attribs2,values); if (values[0] == 0 || values[1] == 0 || (values[2] != 0x202b && values[2] != 0x202a) || values[3] == 0x2025 || values[4] !=  1) continue;
            u->redBits=values[5]; u->greenBits=values[6]; u->blueBits=values[7]; u->alphaBits=values[8]; u->depthBits=values[9]; u->stencilBits=values[10]; u->handle=pixelFormat; usableCount++;
        }
        const FBC* closest = ChooseFBConfig(usableConfigs,usableCount); if (!closest) { OS_Free(usableConfigs,nativeCount * sizeof(FBC)); DualLogError("No usable WGL framebuffer config\n"); OS_Exit(1); }
        pixelFormat = (int)closest->handle; OS_Free(usableConfigs,nativeCount * sizeof(FBC));
        DescribePixelFormat(w->context.wgl.dc,pixelFormat,sizeof(pfd2),&pfd2); SetPixelFormat(w->context.wgl.dc,pixelFormat,&pfd2);
        int attribs[40], index=0; attribs[index++] = 0x2091; attribs[index++] = 4; attribs[index++] = 0x2092; attribs[index++] = 3; attribs[index++] = 0x9126; attribs[index++] = 1; attribs[index++] = 0; attribs[index++] = 0;
        w->context.wgl.handle = CCAARB(w->context.wgl.dc,NULL,attribs); w->context.makeCurrent=makeContextCurrentWGL; w->context.swapBuffers=swapBuffersWGL; w->context.swapInterval=swapIntervalWGL; w->context.getProcAddress=getProcAddressWGL;
        if (!w->context.wgl.handle) { DualLogError("Failed to create an OpenGL 4.3 core context - GPU driver must support OpenGL 4.3 or newer\n"); OS_Exit(1); } /*Compat: attribs request 4.3 core, so NULL means the driver cannot provide it*/
        return w;
    }
// End Windows section
#else // LINUX
    typedef u8 KeyCode;
    typedef char *XPointer;
    typedef u16 Rotation,Connection;
    typedef u32 XcursorUInt;
    typedef i32 Bool,Status;
    typedef u64 XID,Atom,Time,KeySym;
    typedef struct _XcursorImage { u8 a[16]; XcursorUInt xhot,yhot; u8 b[8]; XcursorUInt *pixels; } XcursorImage;
    typedef struct { i64 flags; int x,y; u8 a[8]; int min_width,min_height,max_width,max_height; u8 b[32]; int win_gravity; } XSizeHints;
    typedef struct { u8 a[56]; } Visual;
    typedef struct { u8 a[16]; XID root; u8 b[40]; Visual *root_visual; u8 c[56]; } Screen;
    typedef struct { u8 a[72]; i64 event_mask; u8 b[16]; XID colormap; u8 c[8]; } XSetWindowAttributes;
    typedef struct { u8 a[8]; int width,height; u8 b[84]; int map_state; u8 c[40]; } XWindowAttributes;
    typedef struct _XDisplay Display; 
    typedef struct{u8 a[136]; int qlen; u8 b[84]; i32 default_screen, nscreens; Screen *screens; u8 c[56];} *_XPrivDisplay; typedef struct{int a; u64 b; int c; void *d; u64 e,f,g,h; int i,j,k,l; u32 m,keycode; int n;} XKeyEvent; typedef struct{int a; u64 b; int c; Display *d; XID e,f,g; Time h; int i,j,k,l; u32 m,button; int n;}XButtonEvent; typedef struct{int a; u64 b; int c; Display *d; XID e,f,g; Time h; int x,y,i,j; u32 k; char l; int m;}XMotionEvent;
    typedef struct { int a; u64 b; int c; Display *d; XID e,f,g; Time h; int x,y; u8 i[32]; } XCrossingEvent; typedef struct { int a; u64 b; i32 c; Display *d; XID e; int mode,f; } XFocusChangeEvent; typedef struct { int a; u64 b; i32 c; Display *d; XID e,f,parent; int g,h,i; } XReparentEvent; typedef struct { int a; u64 b; i32 c; Display *d; XID e,f; int x,y,width,height,g; XID h; int i; } XConfigureEvent;
    typedef struct { int a; u64 b; int c; Display *d; XID window; Atom message_type; int format; union { char b[20]; short s[10]; long l[5]; } data; } XClientMessageEvent; typedef struct { int a; u64 b; int send_event; Display *c; XID window; } XAnyEvent;
    typedef union _XEvent { int type; XAnyEvent xany; XKeyEvent xkey; XButtonEvent xbutton; XMotionEvent xmotion; XCrossingEvent xcrossing; XFocusChangeEvent xfocus; u8 p0[528]; XReparentEvent xreparent; XConfigureEvent xcfg; u8 p1[648]; XClientMessageEvent xclient; u8 p2[224]; } XEvent; typedef struct { Visual *visual; u8 a[12]; int depth; u8 b[40]; } XVisualInfo; typedef int XContext; typedef XID RROutput,RRCrtc;
    typedef struct { u8 a[32]; int noutput; u8 b[4]; RROutput *outputs; u8 c[16];  } XRRScreenResources;
    typedef struct { u8 a[8]; RRCrtc crtc; char *name; u8 b[8]; u64 mm_width,mm_height; Connection connection; u8 c[52]; } XRROutputInfo; typedef struct { u8 a[8]; i32 x,y; u32 width,height; XID mode; Rotation rotation; u8 b[36]; } XRRCrtcInfo; typedef XID GLXWindow,GLXDrawable; typedef struct __GLXFBConfig* GLXFBConfig; typedef struct __GLXcontext* GLXContext;
    typedef void(*__GLXextproc)();                            typedef XSizeHints*(*PFN_XAllocSizeHints)();                       typedef int(*PFN_XChangeProperty)(Display*,XID,Atom,Atom,int,int,const u8*,int);   typedef void(*PFN_XCID)(XcursorImage*);                         typedef void(*PFN_XRRFreeOutputInfo)(XRROutputInfo*);                    typedef XID(*PFN_XCreateColormap)(Display*,XID,Visual*,int);
    typedef int(*PFN_XDefineCursor)(Display*,XID,XID);        typedef int(*PFN_XDisplayKeycodes)(Display*,int*,int*);            typedef Bool(*PFN_XFilterEvent)(XEvent*,XID);                                      typedef int(*PFN_XFindContext)(Display*,XID,XContext,XPointer*);typedef XID(*PFN_XCreateWindow)(Display*,XID,int,int,u32,u32,u32,int,u32,Visual*,u64,XSetWindowAttributes*);
    typedef int(*PFN_XFree)(void*);                           typedef int(*PFN_XGrabPointer)(Display*,XID,Bool,u32,int,int,XID,XID,Time);        typedef XVisualInfo*(*GLX_GVFFBCP)(Display*,GLXFBConfig);          typedef KeySym*(*PFN_XGetKeyboardMapping)(Display*,KeyCode,int,int*);    typedef Status(*PFN_XGetWMNormalHints)(Display*,XID,XSizeHints*,long*);
    typedef Atom(*PFN_XInternAtom)(Display*,const char*,Bool);typedef int(*PFN_XMapWindow)(Display*,XID);                        typedef int(*PFN_XMoveWindow)(Display*,XID,int,int);                               typedef int(*PFN_XMoveResizeWindow)(Display*,XID,int,int,u32,u32);       typedef int(*PFN_XGetWindowProperty)(Display*,XID,Atom,long,long,Bool,Atom,Atom*,int*,u64*,u64*,u8**);
    typedef Status(*PFN_XInitThreads)();                      typedef int(*PFN_XNextEvent)(Display*,XEvent*);                    typedef XRRCrtcInfo*(*PFN_XRRGetCrtcInfo)(Display*,XRRScreenResources*,RRCrtc);    typedef int(*PFN_XPending)(Display*);                           typedef Bool(*PFN_XQueryPointer)(Display*,XID,XID*,XID*,int*,int*,int*,int*,u32*);
    typedef int(*PFN_XRaiseWindow)(Display*,XID);             typedef int(*PFN_XSaveContext)(Display*,XID,XContext,const char*); typedef int(*PFN_XResizeWindow)(Display*,XID,u32,u32);                             typedef XcursorImage*(*PFN_XCIC)(int,int);                      typedef Status(*PFN_XSendEvent)(Display*,XID,Bool,long,XEvent*);         typedef int(*PFN_XSetInputFocus)(Display*,XID,int,Time);
    typedef void(*GLX_SBP)(Display*,GLXDrawable);             typedef Status(*PFN_XSetWMProtocols)(Display*,XID,Atom*,int);      typedef Bool(*PFN_XTranslateCoordinates)(Display*,XID,XID,int,int,int*,int*,XID*); typedef int(*PFN_XUndefineCursor)(Display*,XID);                typedef void(*PFN_XSetWMNormalHints)(Display*,XID,XSizeHints*);          typedef int(*PFN_XWarpPointer)(Display*,XID,XID,int,int,u32,u32,int,int);
    typedef void(*PFN_XRRFreeCrtcInfo)(XRRCrtcInfo*);         typedef int(*PFN_XUngrabPointer)(Display*,Time);                   typedef void(*PFN_XRRFreeScreenResources)(XRRScreenResources*);                    typedef Display*(*PFN_XOpenDisplay)(const char*);               typedef XRROutputInfo*(*PFN_XRRGetOutputInfo)(Display*,XRRScreenResources*,RROutput);
    typedef RROutput(*PFN_XRRGetOutputPrimary)(Display*,XID); typedef void(*PFN_XRRSelectInput)(Display*,XID,int);               typedef XRRScreenResources*(*PFN_XRRGetScreenResourcesCurrent)(Display*,XID);      typedef int(*PFN_XRRUpdateConfiguration)(XEvent*);              typedef Bool(*PFN_XCheckTypedWindowEvent)(Display*,XID,int,XEvent*);     typedef Status(*PFN_XGetWindowAttributes)(Display*,XID,XWindowAttributes*);
    typedef Bool(*GLX_QEP)(Display*,int*,int*);               typedef int(*GLX_GFBCAP)(Display*,GLXFBConfig,int,int*);           typedef Bool(*GLX_QVP)(Display*,int*,int*);                                        typedef Bool(*GLX_MCP)(Display*,GLXDrawable,GLXContext);                 
    typedef XID(*PFN_XCILC)(Display*,const XcursorImage*);    typedef const char*(*GLX_QESP)(Display*,int);                      typedef GLXFBConfig*(*GLX_CFBCP)(Display*,int,const int*,int*);                               typedef __GLXextproc(*GLX_GPAP)(const u8*);                     typedef void(*GLX_SIEP)(Display*,GLXDrawable,int);                       
    typedef GLXWindow(*GLX_CWP)(Display*,GLXFBConfig,XID,const int*); typedef GLXContext(*GLX_CCAA)(Display*,GLXFBConfig,GLXContext,Bool,const int*); typedef struct WSCtxGLX { GLXContext handle; GLXWindow window; } WSCtxGLX;
    typedef struct WSLibGLX { void* handle; GLX_CFBCP ChooseFBConfigs; GLX_MCP MakeCurrent; GLX_SBP SwapBuffers; GLX_GVFFBCP GetVisualFromFBConfig; GLX_CWP CreateWindow; GLX_GPAP GetProcAddress; GLX_SIEP SwapIntervalEXT; GLX_CCAA CreateContextAttribsARB; } WSLibGLX;
    typedef struct WSWinX11 { XID colormap; XID handle,parent; int width,height,xpos,ypos,lastCurX,lastCurY,warpCursorPosX,warpCursorPosY; } WSWinX11;
    typedef struct WSLibX11 { Display* display; int screen; XID root; XID hiddenCursorHandle; XContext context; short int keycodes[256]; double restoreCurPosX, restoreCurPosY; WSWin* disabledCursorWindow; Atom NET_SUPPORTED,WM_PROTOCOLS,WM_DELETE_WINDOW,NWM_NAME,NWM_ICON,NWM_PING,NWM_WINDOW_TYPE,NWM_WINDOW_TYPE_NORMAL,NWM_STATE,NWM_STATE_FULLSCREEN,NET_ACTIVE_WINDOW,MOTIF_WM_HINTS,UTF8_STRING;
                              struct { void* handle; PFN_XAllocSizeHints AllocSizeHints; PFN_XChangeProperty ChangeProperty; PFN_XCheckTypedWindowEvent CheckTypedWindowEvent; PFN_XCreateColormap CreateColormap; PFN_XCreateWindow CreateWindow; PFN_XDefineCursor DefineCursor;
                              PFN_XDisplayKeycodes DisplayKeycodes; PFN_XFilterEvent FilterEvent; PFN_XFindContext FindContext; PFN_XFree Free; PFN_XGetKeyboardMapping GetKeyboardMapping; PFN_XGetWMNormalHints GetWMNormalHints;
                              PFN_XGetWindowAttributes GetWindowAttributes; PFN_XGetWindowProperty GetWindowProperty; PFN_XGrabPointer GrabPointer; PFN_XInternAtom InternAtom; PFN_XMapWindow MapWindow; PFN_XMoveResizeWindow MoveResizeWindow; PFN_XMoveWindow MoveWindow; PFN_XPending Pending; 
                              PFN_XQueryPointer QueryPointer; PFN_XRaiseWindow RaiseWindow; PFN_XResizeWindow ResizeWindow; PFN_XSaveContext SaveContext; PFN_XSendEvent SendEvent; PFN_XSetInputFocus SetInputFocus; PFN_XSetWMNormalHints SetWMNormalHints; PFN_XSetWMProtocols SetWMProtocols; PFN_XTranslateCoordinates TranslateCoordinates; PFN_XUndefineCursor UndefineCursor; PFN_XUngrabPointer UngrabPointer; PFN_XWarpPointer WarpPointer; } xlib;
                              struct {void* handle; int eventBase; PFN_XRRFreeCrtcInfo FreeCrtcInfo; PFN_XRRFreeOutputInfo FreeOutputInfo; PFN_XRRFreeScreenResources FreeScreenResources; PFN_XRRGetCrtcInfo GetCrtcInfo; PFN_XRRGetOutputInfo GetOutputInfo; PFN_XRRGetOutputPrimary GetOutputPrimary; PFN_XRRGetScreenResourcesCurrent GetScreenResourcesCurrent; PFN_XRRSelectInput SelectInput; PFN_XRRUpdateConfiguration UpdateConfiguration;}randr;} WSLibX11;
    PFN_XNextEvent XNextEvent; typedef struct WSMonX11 { RROutput output; RRCrtc crtc; } WSMonX11; static void GetCursorPosV(WSWin*,double*,double*),SetCurV(WSWin*,double,double);
    struct WSLib {WSMon** monitors; int monitorCount; WSLibX11 x11; WSLibGLX glx;}; struct WSCtx {void (*makeCurrent)(WSWin*); void (*swapBuffers)(WSWin*); void (*swapInterval)(int); WSP (*getProcAddress)(const char*); WSCtxGLX glx;}; struct WSWin{i32 decorated; char mouseButtons[MAX_MOUSE_BUTTONS],keys[WS_KEYCOUNT]; double virtualCursorPosX,virtualCursorPosY; WSCtx context; WSWinX11 x11;}; struct WSMon{WSMonX11 x11;};
    WSP PlatformGetModuleSymbol(void* m, const char* n) { return dlsym(m,n); }
    u64 WinSysGetWindowPropertyX11(XID win, Atom prop, Atom type, u8** val) { Atom actType; i32 actFmt; u64 itemCount,bytesAfter; WinSys.x11.xlib.GetWindowProperty(WinSys.x11.display,win,prop,0,2147483647,0,type,&actType,&actFmt,&itemCount,&bytesAfter,val); return itemCount; }
    static void sendEventToWM(WSWin* w, Atom type, i64 a, i64 b, i64 c, i64 d, i64 f) { XEvent e={33}; e.xclient.window = w->x11.handle; e.xclient.format = 32; e.xclient.message_type = type; e.xclient.data.l[0]=a; e.xclient.data.l[1]=b; e.xclient.data.l[2]=c; e.xclient.data.l[3]=d; e.xclient.data.l[4]=f; WinSys.x11.xlib.SendEvent(WinSys.x11.display,WinSys.x11.root,0,(1L<<19)|(1L<<20),&e); }
    static void updateNormalHints(WSWin* w, int w_, int h) { XSizeHints* hs=WinSys.x11.xlib.AllocSizeHints(); i64 s; WinSys.x11.xlib.GetWMNormalHints(WinSys.x11.display,w->x11.handle,hs,&s); hs->flags &= ~((1L<<4)|(1L<<5)|(1L<<7)); hs->flags|=((1L<<4)|(1L<<5)); hs->min_width=hs->max_width=w_; hs->min_height=hs->max_height=h; WinSys.x11.xlib.SetWMNormalHints(WinSys.x11.display,w->x11.handle,hs); WinSys.x11.xlib.Free(hs); }
    INLINE void updateCursorImage(WSWin* w) { if (WinSys.x11.disabledCursorWindow != w) { WinSys.x11.xlib.UndefineCursor(WinSys.x11.display,w->x11.handle); } else { WinSys.x11.xlib.DefineCursor(WinSys.x11.display,w->x11.handle,WinSys.x11.hiddenCursorHandle); } }
    static void captureCursor(WSWin* w) { WinSys.x11.xlib.GrabPointer(WinSys.x11.display,w->x11.handle,1,(1L<<2)|(1L<<3)|(1L<<6),1,1,w->x11.handle,0L,0L); }
    static void releaseCursor() { WinSys.x11.xlib.UngrabPointer(WinSys.x11.display,0L); }
    static void disableCursor(WSWin* w) { WinSys.x11.disabledCursorWindow=w; GetCursorPosV(w,&WinSys.x11.restoreCurPosX,&WinSys.x11.restoreCurPosY); updateCursorImage(w); captureCursor(w); }
    static void enableCursor(WSWin* w) { WinSys.x11.disabledCursorWindow = NULL; releaseCursor(); SetCurV(w,WinSys.x11.restoreCurPosX,WinSys.x11.restoreCurPosY); updateCursorImage(w); }
    void SetWindowIcon(const WinSysIcon* image) {
        int lC=2+image[0].width*image[0].height; u64* icon=OS_Alloc(lC*sizeof(u64)), *t=icon; *t++=image[0].width; *t++=image[0].height; for (int j=0;j<image[0].width*image[0].height;++j) *t++=(((u64)image[0].pixels[j*4+0])<<16)|(((u64)image[0].pixels[j*4+1])<<8)|(((u64)image[0].pixels[j*4+2]))|(((u64)image[0].pixels[j*4+3])<<24); 
        WinSys.x11.xlib.ChangeProperty(WinSys.x11.display,window->x11.handle,WinSys.x11.NWM_ICON,6,32,0,(u8*)icon,lC); OS_Free(icon,lC*sizeof(u64));
    }
    static void GetWindowSize(WSWin* w, int* w_, int* h) { XWindowAttributes a; WinSys.x11.xlib.GetWindowAttributes(WinSys.x11.display,w->x11.handle,&a); *w_=a.width; *h=a.height; }
    static void SetWindowSize(int w_, int h) { w_=vmax(1,w_); h=vmax(1,h); updateNormalHints(window,w_,h); WinSys.x11.xlib.ResizeWindow(WinSys.x11.display,window->x11.handle,w_,h); }
    static void SetWindowMonitor(int x,int y,int w_,int h){updateNormalHints(window,w_,h); sendEventToWM(window,WinSys.x11.NWM_STATE,0,WinSys.x11.NWM_STATE_FULLSCREEN,0,1,0); WinSys.x11.xlib.MoveResizeWindow(WinSys.x11.display,window->x11.handle,x,y,w_,h); sendEventToWM(window,WinSys.x11.NWM_STATE,1,WinSys.x11.NWM_STATE_FULLSCREEN,0,1,0);}
    i32 WindowVisible() { XWindowAttributes w; WinSys.x11.xlib.GetWindowAttributes(WinSys.x11.display,window->x11.handle,&w); return w.map_state==2; }
    static void GetWindowPos(WSWin* w, int* x, int* y) { XID d; WinSys.x11.xlib.TranslateCoordinates(WinSys.x11.display,w->x11.handle,WinSys.x11.root,0,0,x,y,&d); }
    static void SetWindowDecorated(WSWin* w,i32 e) { struct {u64 f,fun,dec; i64 im; u64 st;} h={0}; h.f=2; h.dec=e?1:0; WinSys.x11.xlib.ChangeProperty(WinSys.x11.display,w->x11.handle,WinSys.x11.MOTIF_WM_HINTS,WinSys.x11.MOTIF_WM_HINTS,32,0,(u8*)&h,sizeof(h)/sizeof(i64)); }
    static void GetCursorPosV(WSWin* w, double* x, double* y) { XID r,c; int rx,ry,cx,cy; u32 m; WinSys.x11.xlib.QueryPointer(WinSys.x11.display,w->x11.handle,&r,&c,&rx,&ry,&cx,&cy,&m); *x=cx; *y=cy; }
    static void SetCurV(WSWin* w, double x, double y) { w->x11.warpCursorPosX=(int)x; w->x11.warpCursorPosY=(int)y; WinSys.x11.xlib.WarpPointer(WinSys.x11.display,0L,w->x11.handle,0,0,0,0,(int)x,(int)y); }    
    static void PollMonitors() {
        XRRScreenResources* sr = WinSys.x11.randr.GetScreenResourcesCurrent(WinSys.x11.display,WinSys.x11.root);
        RROutput p = WinSys.x11.randr.GetOutputPrimary(WinSys.x11.display,WinSys.x11.root);
        int dC = WinSys.monitorCount; WSMon** d = NULL; if (dC) { d = OS_Alloc(WinSys.monitorCount*sizeof(WSMon*)); mcpy(d,WinSys.monitors,WinSys.monitorCount * sizeof(WSMon*)); }
        for (int i = 0;  i < sr->noutput;  i++) {
            int j, t;
            XRROutputInfo* oi = WinSys.x11.randr.GetOutputInfo(WinSys.x11.display, sr, sr->outputs[i]); if (oi->connection != 0 || oi->crtc == 0L) { WinSys.x11.randr.FreeOutputInfo(oi); continue; } for (j=0;j<dC;++j) if(d[j] && d[j]->x11.output == sr->outputs[i]){d[j]=NULL; break;} if (j < dC) { WinSys.x11.randr.FreeOutputInfo(oi); continue; }
            WSMon* m = OS_Alloc(sizeof(WSMon)); m->x11.output = sr->outputs[i]; m->x11.crtc = oi->crtc; t = (m->x11.output == p) ? 0 : 1; InputMonitor(m,0x40001,t); WinSys.x11.randr.FreeOutputInfo(oi);
        }
        WinSys.x11.randr.FreeScreenResources(sr); for (int i=0;i<dC;++i)  {if (d[i]) { InputMonitor(d[i],0x40002,0); } } if (d) OS_Free(d,WinSys.monitorCount*sizeof(WSMon*));
    }
    static void processEvent(XEvent* e) {
        static int firstFocus = 1; u32 kc = (e->type==2 || e->type==3) ? e->xkey.keycode : 0; if (e->type==WinSys.x11.randr.eventBase+1) { WinSys.x11.randr.UpdateConfiguration(e); PollMonitors(); return; } if (e->type==35) return; WSWin* w=NULL; if (WinSys.x11.xlib.FindContext(WinSys.x11.display,e->xany.window,WinSys.x11.context,(XPointer*)&w)!=0) return;
        switch (e->type) {
            case 21: w->x11.parent=e->xreparent.parent; return;
            case 2: case 3: { const int k = (kc > 255) ? KEY_UNKNOWN : WinSys.x11.keycodes[kc]; if (k==KEY_UNKNOWN) return; if (e->type == 3) { XEvent n; if(WinSys.x11.xlib.CheckTypedWindowEvent(WinSys.x11.display,e->xany.window,2,&n)){if(n.xkey.keycode == kc && n.xkey.h == e->xkey.h){InputKey(w->keys,k,INPUT_PRESS); return;}else{InputKey(w->keys,k,INPUT_RELEASE); processEvent(&n); return;}}} const int a=(e->type==2)?INPUT_PRESS:INPUT_RELEASE; InputKey(w->keys,k,a); return;}
            case 4: case 5: {
                const int b = e->xbutton.button; const int a = (e->type == 4) ? INPUT_PRESS : INPUT_RELEASE;
                if (a == INPUT_PRESS && WinSys.x11.disabledCursorWindow != w) { disableCursor(w); } // Explicitly require a click to lock the mouse
                if (b == 1) InputMouseClick(w->mouseButtons,MOUSE_BUTTON_LEFT,a); else if (b == 2) InputMouseClick(w->mouseButtons,MOUSE_BUTTON_MIDDLE,a); else if (b == 3) InputMouseClick(w->mouseButtons,MOUSE_BUTTON_RIGHT,a); else if (a == INPUT_PRESS && b == 4) Sys_Input.scrollDelta += 1.0f; else if (a == INPUT_PRESS && b == 5) Sys_Input.scrollDelta -= 1.0f; else if (b > 7) InputMouseClick(w->mouseButtons,b - 5,a); return; }
            case 7: { const int x=e->xcrossing.x,y=e->xcrossing.y; InputCursorPos(&w->virtualCursorPosX,&w->virtualCursorPosY,x,y); w->x11.lastCurX=x; w->x11.lastCurY=y; return; }
            case 6/*MotionNotify*/: { 
                const int x=e->xmotion.x, y=e->xmotion.y;
                if (x!=w->x11.warpCursorPosX || y!=w->x11.warpCursorPosY) { if(WinSys.x11.disabledCursorWindow!=w) return; InputCursorPos(&w->virtualCursorPosX,&w->virtualCursorPosY,w->virtualCursorPosX + (x - w->x11.lastCurX),w->virtualCursorPosY + (y - w->x11.lastCurY)); int w_,h; GetWindowSize(w,&w_,&h);/*Re-center ONLY in response to physical motion to prevent WM feedback loops*/ SetCurV(w, w_/2, h/2); }
                w->x11.lastCurX=x; w->x11.lastCurY=y; 
                return; }
            case 22: {
                if (e->xcfg.width!=w->x11.width || e->xcfg.height!=w->x11.height) { w->x11.width=e->xcfg.width; w->x11.height=e->xcfg.height; UpdateScreenSize(e->xcfg.width,e->xcfg.height); }
                int x=e->xcfg.x, y=e->xcfg.y;
                if (!e->xany.send_event && w->x11.parent!=WinSys.x11.root) { XID d; WinSys.x11.xlib.TranslateCoordinates(WinSys.x11.display,w->x11.parent,WinSys.x11.root,x,y,&x,&y,&d); }
                w->x11.xpos=x; w->x11.ypos=y;
                return; }
            case 33: { if(WinSys.x11.xlib.FilterEvent(e,0L) || e->xclient.message_type==0L) return; if (e->xclient.message_type==WinSys.x11.WM_PROTOCOLS) {const Atom p=e->xclient.data.l[0]; if (p==0L) return; if (p == WinSys.x11.WM_DELETE_WINDOW) OS_Exit(0); if (p == WinSys.x11.NWM_PING) { XEvent r=*e; r.xclient.window=WinSys.x11.root; WinSys.x11.xlib.SendEvent(WinSys.x11.display,WinSys.x11.root,0,(1L<<19)|(1L<<20),&r); }} return; }
            case  9: { if (e->xfocus.mode==1 || e->xfocus.mode==2) return; if (firstFocus) { disableCursor(w); firstFocus = 0; } InputWindowFocus(1); return; }
            case 10: { if (e->xfocus.mode==1 || e->xfocus.mode==2) return; enableCursor(w); InputWindowFocus(0); return; }
        }
    }
    void GetMonitorFullarea(WSMon* m, int* x, int* y, int* w, int* h) { XRRScreenResources* sr = WinSys.x11.randr.GetScreenResourcesCurrent(WinSys.x11.display,WinSys.x11.root); XRRCrtcInfo* ci = WinSys.x11.randr.GetCrtcInfo(WinSys.x11.display,sr,m->x11.crtc); if (ci) { *x = ci->x; *y = ci->y; *w = ci->width; *h = ci->height; WinSys.x11.randr.FreeCrtcInfo(ci); } else { *x = 0; *y = 0; *w = 0; *h = 0; } WinSys.x11.randr.FreeScreenResources(sr);}
    void GetMonitorWorkarea(WSMon* m, int* x, int* y, int* w, int* h) {
        int fx,fy,fw,fh; GetMonitorFullarea(m,&fx,&fy,&fw,&fh); *x=fx; *y=fy; *w=fw; *h=fh; if(!WinSys.x11.display){return;} Atom waA=WinSys.x11.xlib.InternAtom(WinSys.x11.display,"_NET_WORKAREA",0), cdA=WinSys.x11.xlib.InternAtom(WinSys.x11.display,"_NET_CURRENT_DESKTOP",0); u8* raw=0; u64 desk=0; u8* draw=0;
        if(WinSysGetWindowPropertyX11(WinSys.x11.root,cdA,6,(u8**)&draw)>=1){ desk=(u64)(*(long*)draw); WinSys.x11.xlib.Free(draw); } if(WinSysGetWindowPropertyX11(WinSys.x11.root,waA,6,(u8**)&raw)>=(desk+1)*4){ long* v=(long*)raw; int ax=(int)v[desk*4+0],ay=(int)v[desk*4+1],aw=(int)v[desk*4+2],ah=(int)v[desk*4+3]; int l=vmax(fx,ax),t=vmax(fy,ay),r=vmin(fx+fw,ax+aw),b=vmin(fy+fh,ay+ah); if(r>l&&b>t){*x=l;*y=t;*w=r-l;*h=b-t;} WinSys.x11.xlib.Free(raw); }
    }
    static int translateKeySyms(const KeySym* k, int w) {
        if (w > 1) { if (k[1] >= 0xffb0 && k[1] <= 0xffb9) return KEY_KP_0 + (k[1] - 0xffb0); switch (k[1]) { case 0xffac: case 0xffae: return KEY_KP_DECIMAL; case 0xffbd:return KEY_KP_EQUAL; case 0xff8d:return KEY_KP_ENTER; } } KeySym c = k[0]; if(c >= 0x61 && c <= 0x7a){return KEY_A + (c - 0x61);} if(c >= 0x30 && c <= 0x39){return KEY_0 + (c - 0x30);} if(c >= 0xffbe && c <= 0xffd6){return KEY_F1 + (c - 0xffbe);}
        if(c >= 0xff95 && c <= 0xff9f){static const int o[] = {KEY_KP_7,KEY_KP_4,KEY_KP_8,KEY_KP_6,KEY_KP_2,KEY_KP_9,KEY_KP_3,KEY_KP_1,-1,KEY_KP_0,KEY_KP_DECIMAL }; int r = o[c - 0xff95]; if(r != -1) return r;} 
        switch (c) {
            case 0xff1b: return KEY_ESCAPE; case 0xff09: return KEY_TAB; case 0xff0d: return KEY_ENTER; case 0xff08: return KEY_BACKSPACE; case 0xffff: return KEY_DELETE; case 0xff50: return KEY_HOME; case 0xff57: return KEY_END; case 0xff55: return KEY_PAGE_UP; case 0xff56: return KEY_PAGE_DOWN; case 0xff63: return KEY_INSERT; case 0xff51: return KEY_LEFT; case 0xff53: return KEY_RIGHT; case 0xff54: return KEY_DOWN; case 0xff52: return KEY_UP;
            case 0xff13: return KEY_PAUSE; case 0xff14: return KEY_SCROLL_LOCK; case 0xff61: return KEY_PRINT_SCREEN; case 0xff7f: return KEY_NUM_LOCK; case 0xffe5: return KEY_CAPS_LOCK; case 0xff67: return KEY_MENU; case 0xffe1: return KEY_LEFT_SHIFT; case 0xffe2: return KEY_RIGHT_SHIFT; case 0xffe3: return KEY_LEFT_CONTROL; case 0xffe4: return KEY_RIGHT_CONTROL; case 0xffe7: case 0xffe9: return KEY_LEFT_ALT;
            case 0xff7e: case 0xfe03: case 0xffe8: case 0xffea: return KEY_RIGHT_ALT; case 0xffeb: return KEY_LEFT_SUPER; case 0xffec: return KEY_RIGHT_SUPER; case 0xffaa: return KEY_KP_MULTIPLY; case 0xffab: return KEY_KP_ADD; case 0xffad: return KEY_KP_SUBTRACT; case 0xffaf: return KEY_KP_DIVIDE; case 0xffbd: return KEY_KP_EQUAL; case 0xff8d: return KEY_KP_ENTER; case 0x20: return KEY_SPACE; case 0x27: return KEY_APOSTROPHE; case 0x2c: return KEY_COMMA;
            case 0x2d: return KEY_MINUS; case 0x2e: return KEY_PERIOD; case 0x2f: return KEY_SLASH; case 0x3b: return KEY_SEMICOLON; case 0x3d: return KEY_EQUAL; case 0x5b: return KEY_LEFT_BRACKET; case 0x5c: return KEY_BACKSLASH; case 0x5d: return KEY_RIGHT_BRACKET; case 0x60: return KEY_GRAVE_ACCENT; default: return KEY_UNKNOWN;
        }
    }
    static Atom getAtomIfSupported(Atom* a, unsigned long c, const char* n) { const Atom t=WinSys.x11.xlib.InternAtom(WinSys.x11.display,n,0); for (unsigned long i=0;i<c;i++) if (a[i] == t) return t; return 0L; }
    void PollEvents() { WinSys.x11.xlib.Pending(WinSys.x11.display); while (((_XPrivDisplay)(WinSys.x11.display))->qlen) { XEvent e; XNextEvent(WinSys.x11.display,&e); processEvent(&e); } }
    static void makeContextCurrentGLX(WSWin* w) { WinSys.glx.MakeCurrent(WinSys.x11.display,w->context.glx.window,w->context.glx.handle); }
    static void swapBuffersGLX(WSWin* w) { WinSys.glx.SwapBuffers(WinSys.x11.display, w->context.glx.window); }
    static void swapIntervalGLX(int i) { WinSys.glx.SwapIntervalEXT(WinSys.x11.display,window->context.glx.window,i); }
    static WSP getProcAddressGLX(const char* p) { return WinSys.glx.GetProcAddress((const u8*) p); }
    void SetWindowPosition(WSWin* w, int x, int y) {if (!WindowVisible()) { i64 s; XSizeHints* h=WinSys.x11.xlib.AllocSizeHints(); if (WinSys.x11.xlib.GetWMNormalHints(WinSys.x11.display,w->x11.handle,h,&s)) {h->flags|=(1L<<2); h->x=h->y=0; WinSys.x11.xlib.SetWMNormalHints(WinSys.x11.display,w->x11.handle,h);} WinSys.x11.xlib.Free(h); } WinSys.x11.xlib.MoveWindow(WinSys.x11.display,w->x11.handle,x,y); }
    WSWin* VCreateWindow(WSWin* w, int width, int height) {
        const char* names[]={"libGLX.so.0","libGL.so.1","libGL.so",NULL};
        for (int i=0;names[i] && !WinSys.glx.handle;i++) { WinSys.glx.handle = dlopen(names[i],2); }
        WinSys.glx.ChooseFBConfigs=(GLX_CFBCP)PlatformGetModuleSymbol(WinSys.glx.handle,"glXChooseFBConfig");                  WinSys.glx.MakeCurrent = (GLX_MCP)PlatformGetModuleSymbol(WinSys.glx.handle,"glXMakeCurrent");        WinSys.glx.SwapBuffers = (GLX_SBP)PlatformGetModuleSymbol(WinSys.glx.handle,"glXSwapBuffers");  WinSys.glx.CreateWindow = (GLX_CWP)PlatformGetModuleSymbol(WinSys.glx.handle,"glXCreateWindow");
        WinSys.glx.GetVisualFromFBConfig = (GLX_GVFFBCP)PlatformGetModuleSymbol(WinSys.glx.handle,"glXGetVisualFromFBConfig"); WinSys.glx.GetProcAddress = (GLX_GPAP)PlatformGetModuleSymbol(WinSys.glx.handle,"glXGetProcAddress"); WinSys.glx.SwapIntervalEXT = (GLX_SIEP)getProcAddressGLX("glXSwapIntervalEXT");                 WinSys.glx.CreateContextAttribsARB = (GLX_CCAA)getProcAddressGLX("glXCreateContextAttribsARB");
        if (!WinSys.glx.CreateContextAttribsARB) { DualLogError("GLX_ARB_create_context unavailable - GPU driver too old for OpenGL 4.3\n"); OS_Exit(1); } /*Compat: clean exit instead of NULL-deref crash*/
        GLXFBConfig native=NULL; XVisualInfo* result; GLXFBConfig* nativeConfigs; int nativeCount; static const int chooseAttribs[] = {0x8011/*GLX_DRAWABLE_TYPE*/,1/*GLX_WINDOW_BIT*/, 0x8010/*GLX_RENDER_TYPE*/,1/*GLX_RGBA_BIT*/, 5/*GLX_DOUBLEBUFFER*/,1, 0};
        nativeConfigs = WinSys.glx.ChooseFBConfigs(WinSys.x11.display,WinSys.x11.screen,chooseAttribs,&nativeCount); if (nativeCount > 0) native = nativeConfigs[0]; WinSys.x11.xlib.Free(nativeConfigs); if (!native) { DualLogError("No usable GLX framebuffer config\n"); OS_Exit(1); }
        result = WinSys.glx.GetVisualFromFBConfig(WinSys.x11.display,native); Visual* visual=result->visual; int depth = result->depth; WinSys.x11.xlib.Free(result);
        w->x11.colormap=WinSys.x11.xlib.CreateColormap(WinSys.x11.display,WinSys.x11.root,visual,0);
        XSetWindowAttributes wa = {0}; wa.colormap = w->x11.colormap; wa.event_mask = 0x63807F;
        w->x11.parent = WinSys.x11.root;
        int wxp=0,wyp=0; if (WinSys.monitorCount > 0) ComputeWindowPlacement((int)Sys_Settings.CurrentMonitor,&wxp,&wyp); else { DualLogError("No monitors!\n"); OS_Exit(1); }
        w->x11.handle = WinSys.x11.xlib.CreateWindow(WinSys.x11.display,WinSys.x11.root,wxp,wyp,width,height,0,depth,1,visual,(1L<<1/*CWBackPixel*/)|(1L<<3)|(1L<<13)|(1L<<11),&wa);
        WinSys.x11.xlib.SaveContext(WinSys.x11.display,w->x11.handle,WinSys.x11.context,(XPointer)w);
        Atom protocols[]={WinSys.x11.WM_DELETE_WINDOW,WinSys.x11.NWM_PING};
        WinSys.x11.xlib.SetWMProtocols(WinSys.x11.display,w->x11.handle,protocols,2);
        if (WinSys.x11.NWM_WINDOW_TYPE && WinSys.x11.NWM_WINDOW_TYPE_NORMAL) { Atom t=WinSys.x11.NWM_WINDOW_TYPE_NORMAL; WinSys.x11.xlib.ChangeProperty(WinSys.x11.display,w->x11.handle,WinSys.x11.NWM_WINDOW_TYPE,4,32,0,(u8*)&t,1); }
        XSizeHints* sz = WinSys.x11.xlib.AllocSizeHints(); sz->flags|=((1L << 4)|(1L << 5)); sz->min_width=sz->max_width=width; sz->min_height=sz->max_height=height; sz->flags|=(1L << 9); sz->win_gravity=10; sz->x=wxp; sz->y=wyp; sz->flags|=(1L << 2); WinSys.x11.xlib.SetWMNormalHints(WinSys.x11.display,w->x11.handle,sz); WinSys.x11.xlib.Free(sz);
        WinSys.x11.xlib.ChangeProperty(WinSys.x11.display,w->x11.handle,WinSys.x11.NWM_NAME,WinSys.x11.UTF8_STRING,8,0,(u8*)GAME_TITLE,sizeof(GAME_TITLE) - 1); 
        GetWindowPos(w,&w->x11.xpos,&w->x11.ypos); GetWindowSize(w,&w->x11.width,&w->x11.height);
        WinSys.x11.xlib.MapWindow(WinSys.x11.display,w->x11.handle);
        int attribs[40], index=0; attribs[index++] = 0x2091; attribs[index++] = 4; attribs[index++] = 0x2092; attribs[index++] = 3; attribs[index++] = 0x9126; attribs[index++] = 1; attribs[index++] = 0; attribs[index++] = 0;
        w->context.glx.handle = WinSys.glx.CreateContextAttribsARB(WinSys.x11.display,native,NULL,1,attribs); w->context.glx.window = WinSys.glx.CreateWindow(WinSys.x11.display,native,w->x11.handle,NULL); w->context.makeCurrent = makeContextCurrentGLX; w->context.swapBuffers = swapBuffersGLX; w->context.swapInterval = swapIntervalGLX; w->context.getProcAddress = getProcAddressGLX;
        if (!w->context.glx.handle) { DualLogError("Failed to create an OpenGL 4.3 core context via GLX - GPU driver must support OpenGL 4.3 or newer\n"); OS_Exit(1); } /*Compat: attribs request 4.3 core, so NULL means the driver cannot provide it*/
        if (WinSys.x11.NET_ACTIVE_WINDOW) sendEventToWM(w,WinSys.x11.NET_ACTIVE_WINDOW,1,0,0,0,0); else { XWindowAttributes attr; if (WinSys.x11.xlib.GetWindowAttributes(WinSys.x11.display,w->x11.handle,&attr) && attr.map_state==2) { WinSys.x11.xlib.RaiseWindow(WinSys.x11.display,w->x11.handle); WinSys.x11.xlib.SetInputFocus(WinSys.x11.display,w->x11.handle,2,0L); } }
        return w;
    }
#endif // End Linux section
FGL_AT glActiveTexture; FGL_AS glAttachShader; FGL_BB glBindBuffer;  FGL_BBB glBindBufferBase;    FGL_CPIV glGetProgramiv;FGL_CC glClearColor;    FGL_U4F glUniform4f;        FGL_BFB glBindFramebuffer;FGL_VP glViewport;    FGL_BVA glBindVertexArray; FGL_EVAA glEnableVertexAttribArray;         
FGL_UP glUseProgram;    FGL_DM glDepthMask;    FGL_VAB glVertexAttribBinding;   FGL_DF glDepthFunc;   FGL_DC glDispatchCompute;    FGL_DB glDrawBuffers;   FGL_GSIV glGetShaderiv; FGL_BVB glBindVertexBuffer; FGL_LW glLineWidth;       FGL_LP glLinkProgram;       FGL_U3F glUniform3f;
FGL_D glDisable;FGL_CM glColorMask;     FGL_CS glCompileShader;FGL_UM3FV glUniformMatrix3fv;    FGL_DA glDrawArrays;  FGL_VAF glVertexAttribFormat;FGL_CP glCreateProgram;   FGL_CRS glCreateShader;  FGL_BFS glBlendFuncSeparate; FGL_UB glUnmapBuffer; FGL_BD glBufferData; FGL_F glFinish;
FGL_C glClear;  FGL_DE glDrawElements;  FGL_U2UI glUniform2ui; FGL_UM4FV glUniformMatrix4fv;    FGL_GIV glGetIntegerv;FGL_GSIL glGetShaderInfoLog; FGL_GPIL glGetProgramInfoLog; FGL_U2F glUniform2f;      FGL_U1UI glUniform1ui;   FGL_GVA glGenVertexArrays;  FGL_RP glReadPixels;      FGL_SS glShaderSource;FGL_TPI glTexParameteri; FGL_TPFV glTexParameterfv;   FGL_U1F glUniform1f;
FGL_E glEnable; FGL_FF glFrontFace;     FGL_GB glGenBuffers;   FGL_FBT2D glFramebufferTexture2D;FGL_GERR glGetError;  FGL_GFS glGenFramebuffers;   FGL_GT glGenTextures;     FGL_BSD glBufferSubData; FGL_MBR glMapBufferRange;   FGL_U1I glUniform1i;      FGL_T2D glTexImage2D; FGL_BIT glBindImageTexture;FGL_BT glBindTexture; FGL_BBR glBindBufferRange;
FGL_GIQ glGenQueries; FGL_GQOU64 glGetQueryObjectui64v; FGL_BQ glBeginQuery; FGL_D glEndQuery; FGL_DAI glDrawArraysInstanced; FGL_BB glBlendFunc;  FGL_BF glBlitFramebuffer; FGL_T2D glTexSubImage2D; FGL_GTKP glGetTexLevelParameteriv; FGL_CBSD glCopyBufferSubData; FGL_CLBSD glClearBufferSubData; FGL_CISD glCopyImageSubData; FGL_CPIV glGetBufferParameteriv; FGL_GBSD glGetBufferSubData; FGL_GB glDeleteBuffers; FGL_MB glMemoryBarrier;
void SetGLContext_GetFunctionPointers() {
    WSWin* h=window;
    #define X(n,t) n=(t)h->context.getProcAddress(#n);
    X(glClear,FGL_C)              X(glClearColor,FGL_CC)      X(glColorMask,FGL_CM)        X(glDepthFunc,FGL_DF)           X(glDepthMask,FGL_DM)               X(glDisable,FGL_D)             X(glEnableVertexAttribArray,FGL_EVAA) X(glEnable,FGL_E)   X(glFrontFace,FGL_FF)                 X(glGetError,FGL_GERR)            X(glGetIntegerv,FGL_GIV)
    X(glLineWidth,FGL_LW)         X(glReadPixels,FGL_RP)      X(glTexImage2D,FGL_T2D)      X(glViewport,FGL_VP)            X(glBindTexture,FGL_BT)             X(glDrawArrays,FGL_DA)         X(glDrawElements,FGL_DE)      X(glGenTextures,FGL_GT)     X(glActiveTexture,FGL_AT)             X(glBlendFuncSeparate,FGL_BFS)    X(glBindVertexArray,FGL_BVA)    X(glVertexAttribBinding,FGL_VAB)
    X(glBindBuffer,FGL_BB)        X(glBufferData,FGL_BD)      X(glGenBuffers,FGL_GB)       X(glUnmapBuffer,FGL_UB)         X(glAttachShader,FGL_AS)            X(glCompileShader,FGL_CS)      X(glCreateProgram,FGL_CP)     X(glCreateShader,FGL_CRS)   X(glDrawBuffers,FGL_DB)               X(glGetProgramiv,FGL_CPIV)        X(glGetShaderInfoLog,FGL_GSIL)  X(glGetProgramInfoLog,FGL_GPIL)     X(glGetShaderiv,FGL_GSIV)
    X(glLinkProgram,FGL_LP)       X(glShaderSource,FGL_SS)    X(glUniform1f,FGL_U1F)       X(glUniform1i,FGL_U1I)          X(glUniform2f,FGL_U2F)              X(glUniform3f,FGL_U3F)         X(glUniform4f,FGL_U4F)        X(glTexParameteri,FGL_TPI)  X(glTexParameterfv,FGL_TPFV)          X(glUniform1ui,FGL_U1UI)          X(glUniform2ui,FGL_U2UI)        X(glUniformMatrix3fv,FGL_UM3FV)     X(glUniformMatrix4fv,FGL_UM4FV)
    X(glUseProgram,FGL_UP)        X(glBindBufferBase,FGL_BBB) X(glFinish,FGL_F)            X(glBindFramebuffer,FGL_BFB)    X(glGenFramebuffers,FGL_GFS)        X(glMapBufferRange,FGL_MBR)    X(glBindImageTexture,FGL_BIT) X(glBindVertexBuffer,FGL_BVB) X(glDispatchCompute,FGL_DC)         X(glGenVertexArrays,FGL_GVA)      X(glVertexAttribFormat,FGL_VAF) X(glFramebufferTexture2D,FGL_FBT2D) X(glBufferSubData,FGL_BSD) X(glBindBufferRange,FGL_BBR)
    X(glGenQueries,FGL_GIQ) X(glGetQueryObjectui64v,FGL_GQOU64) X(glBeginQuery,FGL_BQ) X(glEndQuery,FGL_D) X(glDrawArraysInstanced,FGL_DAI) X(glBlendFunc,FGL_BB) X(glBlitFramebuffer,FGL_BF) X(glTexSubImage2D,FGL_T2D) X(glGetTexLevelParameteriv,FGL_GTKP) X(glCopyBufferSubData,FGL_CBSD) X(glClearBufferSubData,FGL_CLBSD) X(glCopyImageSubData,FGL_CISD)  X(glGetBufferParameteriv,FGL_CPIV)  X(glGetBufferSubData,FGL_GBSD) X(glDeleteBuffers,FGL_GB) X(glMemoryBarrier,FGL_MB)
    #undef X
}
size_t monitorAllocationSize = 0;
void InputMonitor(WSMon* m, int a, int p) {
    if (a == 0x40001) {WinSys.monitorCount++; WinSys.monitors = WinSys.monitors ? OS_Realloc(WinSys.monitors,monitorAllocationSize,sizeof(WSMon*) * WinSys.monitorCount) : OS_Alloc(WinSys.monitorCount * sizeof(WSMon*)); monitorAllocationSize = WinSys.monitorCount * sizeof(WSMon*); if(p == 0){mmov(WinSys.monitors+1,WinSys.monitors,((size_t)WinSys.monitorCount-1)*sizeof(WSMon*)); WinSys.monitors[0]=m;}else WinSys.monitors[WinSys.monitorCount-1]=m;}
    else if (a == 0x40002) { for (int i=0;i<WinSys.monitorCount;++i) { if (WinSys.monitors[i] == m) { WinSys.monitorCount--; mmov(WinSys.monitors + i, WinSys.monitors + i + 1,((size_t) WinSys.monitorCount - i) * sizeof(WSMon*)); break; } } }
}
void InputWindowFocus(i32 f) { window_has_focus = f != 0; ignore_next_mouse_delta = true; WSWin* w = window; if (!f) { for (int k=0;k<WS_KEYCOUNT;++k) if (w->keys[k] == INPUT_PRESS) InputKey(w->keys,k,INPUT_RELEASE); for (int b=0;b<MAX_MOUSE_BUTTONS;++b) if (w->mouseButtons[b] == INPUT_PRESS) InputMouseClick(w->mouseButtons,b,INPUT_RELEASE); } }
WSMon* GetCurrentMonitor() {int wx=0,wy=0,ww=0,wh=0,bA=0; GetWindowPos(window,&wx,&wy); GetWindowSize(window,&ww,&wh); WSMon *bM = WinSys.monitors[0]; for(int i=0;i<WinSys.monitorCount;++i){int mx,my,mw,mh; GetMonitorFullarea(WinSys.monitors[i],&mx,&my,&mw,&mh); int l=vmax(wx,mx),r=vmin(wx+ww,mx+mw), t=vmax(wy,my),b=vmin(wy+wh,my+mh); int area=(r>l&&b>t) ? (r-l)*(b-t) : 0; if(area > bA){bA =area; bM=WinSys.monitors[i];}} return bM;}
int GetCurrentMonitorIndex() {WSMon* cur=GetCurrentMonitor(); if(!cur){return 0;} for(int i=0;i<WinSys.monitorCount;++i){ if(WinSys.monitors[i]==cur){return i;} } return 0; }
void GetFrameExtents(int* l,int* t,int* r,int* b) {
#if defined(_WIN32)
    RECT rc={0,0,0,0}; AdjustWindowRectEx(&rc,0x60A0000|0xC00000,0,0); *l=-rc.l; *t=-rc.t; *r=rc.r; *b=rc.b;
#else
    static int cl=0,ct=0,cr=0,cb=0,have=0; *l=*t=*r=*b=0;
    if(window && WinSys.x11.display){ Atom fe=WinSys.x11.xlib.InternAtom(WinSys.x11.display,"_NET_FRAME_EXTENTS",0); u8* raw=0; if(WinSysGetWindowPropertyX11(window->x11.handle,fe,6,(u8**)&raw)>=4){ long* v=(long*)raw; *l=(int)v[0]; *r=(int)v[1]; *t=(int)v[2]; *b=(int)v[3]; WinSys.x11.xlib.Free(raw); } }
    if(*l||*t||*r||*b){ cl=*l; ct=*t; cr=*r; cb=*b; have=1; } else if(have){ *l=cl; *t=ct; *r=cr; *b=cb; }/*undecorated reads go stale: serve last decorated values*/
#endif
}
void GetDecorationOverhead(int* dw,int* dh) {int l,t,r,b; GetFrameExtents(&l,&t,&r,&b); *dw=l+r; *dh=t+b;}
void ClampWindowSizeToMonitor() {
    int ci=(int)Sys_Settings.CurrentMonitor; if (ci>=WinSys.monitorCount) { ci = 0; Sys_Settings.CurrentMonitor = 0; } int x,y,w,h; GetMonitorWorkarea(WinSys.monitors[ci],&x,&y,&w,&h); int dw,dh; GetDecorationOverhead(&dw,&dh); bool ch=false; if(w-dw>=320 && (int)Sys_Settings.ScreenWidth>w-dw){Sys_Settings.ScreenWidth=(w-dw); ch=true;} if(h-dh>=200 && (int)Sys_Settings.ScreenHeight>h-dh){Sys_Settings.ScreenHeight=(h-dh); ch=true;} if(ch){SaveConfig();}
}
void ComputeWindowPlacement(int monIdx, int* px, int* py) { if (monIdx >= WinSys.monitorCount) { monIdx = 0; } WSMon* m=WinSys.monitors[monIdx]; int w = (int)Sys_Settings.ScreenWidth, h = (int)Sys_Settings.ScreenHeight, wx,wy,ww,wh,ddw,ddh, fl = 0; GetMonitorWorkarea(m,&wx,&wy,&ww,&wh); GetDecorationOverhead(&ddw,&ddh); int ft,fr,fb; GetFrameExtents(&fl,&ft,&fr,&fb); *px=(w==ww-ddw) ? wx+fl : wx+(ww-w)/2; *py=(h==wh-ddh) ? wy : wy+(wh-h)/2; }
void PlaceWindowOnMonitor(int monIdx) {
    int px,py; ComputeWindowPlacement(monIdx,&px,&py); SetWindowPosition(window,px,py); ignore_next_mouse_delta=true;
    #if defined(_WIN32)
        void* hw = window->win32.handle; ShowWindow(hw,5); BringWindowToTop(hw); SetForegroundWindow(hw); SetFocus(hw);
    #endif
}
void ApplyStagedWindowedSizeOn(int monIdx) {
    if(monIdx>=WinSys.monitorCount){monIdx=0;} if(resDropdownCount>0){ if(resSelectedIdx<0){resSelectedIdx=0;} if(resSelectedIdx>=resDropdownCount){resSelectedIdx=resDropdownCount-1;} Sys_Settings.ScreenWidth=(u32)resModes[resSelectedIdx].w; Sys_Settings.ScreenHeight=(u32)resModes[resSelectedIdx].h; } Sys_Settings.Fullscreen=0; window->decorated=1; SetWindowDecorated(window,1);
    WSMon* m=WinSys.monitors[monIdx]; int wx,wy,ww,wh; GetMonitorWorkarea(m,&wx,&wy,&ww,&wh); int ddw,ddh; GetDecorationOverhead(&ddw,&ddh); int w=vmax(vmin((int)Sys_Settings.ScreenWidth,ww-ddw),320),h=vmax(vmin((int)Sys_Settings.ScreenHeight,wh-ddh),200); Sys_Settings.ScreenWidth=(u16)w; Sys_Settings.ScreenHeight=(u16)h; 
    SetWindowSize(w,h); PlaceWindowOnMonitor(monIdx); UpdateScreenSize(w,h); resDropdownOpen=false; resHoverIdx=-1; SaveConfig();
}
void ApplyStagedWindowedSize() {int i=GetCurrentMonitorIndex(); Sys_Settings.CurrentMonitor=(u8)i; ApplyStagedWindowedSizeOn(i);}
void GatherResolutionModesFor(int monIdx) {
    int keepW=-1,keepH=-1; if(resSelectedIdx>=0 && resSelectedIdx<resDropdownCount){keepW=resModes[resSelectedIdx].w; keepH=resModes[resSelectedIdx].h;}
    resDropdownCount = 0; if(monIdx>=WinSys.monitorCount){monIdx=0;} WSMon* m = WinSys.monitors[monIdx]; int wx,wy,ww,wh; GetMonitorWorkarea(m,&wx,&wy,&ww,&wh); static const struct {int w,h;} cr[] = {{320,200},{640,400},{640,480},{800,600},{1024,768},{1280,720},{1280,800},{1366,768},{1440,900},{1600,900},{1920,1080},{2560,1440}}; int j; int fddw,fddh;
    GetDecorationOverhead(&fddw,&fddh);
    for (int i = 0; i < 12; ++i) { if (cr[i].w+fddw > ww || cr[i].h+fddh > wh) continue; for (j = 0; j < resDropdownCount; ++j) { if (resModes[j].w == cr[i].w && resModes[j].h == cr[i].h) break; } if (j == resDropdownCount) resModes[resDropdownCount++] = (ResMode){cr[i].w,cr[i].h}; }
    { int fw=ww-fddw, fh=wh-fddh; if (fw>=320 && fh>=200){ for (j = 0; j < resDropdownCount; ++j) { if (resModes[j].w == fw && resModes[j].h == fh) break; } if (j == resDropdownCount) resModes[resDropdownCount++] = (ResMode){fw,fh}; } }
    if (resDropdownCount==0) { resModes[resDropdownCount++]=(ResMode){vmax((int)Sys_Settings.ScreenWidth,320),vmax((int)Sys_Settings.ScreenHeight,200)};} resSelectedIdx=-1;
    for (int i=0;i<resDropdownCount;++i) { if(resModes[i].w == keepW && resModes[i].h == keepH){resSelectedIdx=i; break;} if (resModes[i].w == (int)Sys_Settings.ScreenWidth && resModes[i].h == (int)Sys_Settings.ScreenHeight) { resSelectedIdx = i; } }
    if(resSelectedIdx<0){ int cw=(int)Sys_Settings.ScreenWidth, chh=(int)Sys_Settings.ScreenHeight; if (cw >= 320 && chh >= 200 && cw + fddw <= ww && chh + fddh <= wh) { resModes[resDropdownCount++] = (ResMode){cw,chh}; } resSelectedIdx = resDropdownCount - 1; }
}
void ChangeFullScreenWindowed(bool adjustToFit) {
    int ci=(int)Sys_Settings.CurrentMonitor; if(ci<0||ci>=WinSys.monitorCount){ci=0; Sys_Settings.CurrentMonitor=0;}
    WSMon* m = WinSys.monitors[ci]; int x,y,w,h; GetMonitorFullarea(m,&x,&y,&w,&h);
    int sddw,sddh; GetDecorationOverhead(&sddw,&sddh);/*snapshot before style flip; live read goes stale while undecorated*/
    window->decorated = (i32)(!Sys_Settings.Fullscreen); SetWindowDecorated(window, (i32)(!Sys_Settings.Fullscreen));
    if (Sys_Settings.Fullscreen) { SetWindowMonitor(x,y,w,h); Sys_Settings.ScreenWidth  = w; Sys_Settings.ScreenHeight = h; }
    else {
        if (adjustToFit) { int ax,ay,aw,ah; GetMonitorWorkarea(m,&ax,&ay,&aw,&ah); if (aw-sddw>=320 && ah-sddh>=200) { Sys_Settings.ScreenWidth=aw-sddw; Sys_Settings.ScreenHeight=ah-sddh; } else { Sys_Settings.ScreenWidth=vmax(vmin((w*3)/4,1366),320); Sys_Settings.ScreenHeight=vmax(vmin((h*3)/4,768),200); } SetWindowSize((int)Sys_Settings.ScreenWidth,(int)Sys_Settings.ScreenHeight); }
        #if defined(_WIN32) /*decorated style recalc above restores frame*/
        #else
            if(WinSys.x11.NWM_STATE && WinSys.x11.NWM_STATE_FULLSCREEN){ sendEventToWM(window,WinSys.x11.NWM_STATE,0,WinSys.x11.NWM_STATE_FULLSCREEN,0,1,0); }
        #endif
        PlaceWindowOnMonitor(ci);
    } UpdateScreenSize(Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight);
}
void CycleToNextMonitor() {
    static double monitorSwitchTime = 0.0; if (get_time() < monitorSwitchTime) {return;} monitorSwitchTime = get_time() + 0.5; Sys_Settings.CurrentMonitor++; if(Sys_Settings.CurrentMonitor>=WinSys.monitorCount)Sys_Settings.CurrentMonitor=0; SaveConfig(); DualLog("CycleToNextMonitor cur:%u\n",Sys_Settings.CurrentMonitor);
    if (Sys_Settings.Fullscreen) { WSMon* m = WinSys.monitors[Sys_Settings.CurrentMonitor]; int x,y,w,h; GetMonitorFullarea(m,&x,&y,&w,&h); SetWindowMonitor(x,y,w,h); Sys_Settings.ScreenWidth=w; Sys_Settings.ScreenHeight=h; } else { GatherResolutionModesFor(Sys_Settings.CurrentMonitor); ApplyStagedWindowedSizeOn(Sys_Settings.CurrentMonitor); }
}
// Input and Configuration System for Config.ini, keyboard and mouse support.
double last_mouse_x,last_mouse_y;
InputElement inputElements[134]={{"A",KEY_A},{"B",KEY_B},{"C",KEY_C},{"D",KEY_D},{"E",KEY_E},{"F",KEY_F},{"G",KEY_G},{"H",KEY_H},{"I",KEY_I},{"J",KEY_J},{"K",KEY_K},{"L",KEY_L},{"M",KEY_M},{"N",KEY_N},{"O",KEY_O},{"P",KEY_P},{"Q",KEY_Q},{"R",KEY_R},{"S",KEY_S},{"T",KEY_T},{"U",KEY_U},{"V",KEY_V},{"W",KEY_W},{"X",KEY_X},{"Y",KEY_Y},{"Z",KEY_Z},{"1",KEY_1},{"2",KEY_2},{"3",KEY_3},{"4",KEY_4},{"5",KEY_5},{"6",KEY_6},{"7",KEY_7},{"8",KEY_8},{"9",KEY_9},
                                 {"0",KEY_0},{"UPARROW",KEY_UP},{"DNARROW",KEY_DOWN},{"LFARROW",KEY_LEFT},{"RTARROW",KEY_RIGHT},{"NUM1",KEY_KP_1},{"NUM2",KEY_KP_2},{"NUM3",KEY_KP_3},{"NUM+",KEY_KP_ADD},{"ENTER",KEY_ENTER},{"RIGHTSHIFT",KEY_RIGHT_SHIFT},{"LEFTSHIFT",KEY_LEFT_SHIFT},{"RIGHTCTRL",KEY_RIGHT_CONTROL},{"LEFTCTRL",KEY_LEFT_CONTROL},{"RIGHTALT",KEY_RIGHT_ALT},{"LEFTALT",KEY_LEFT_ALT},{"RIGHTCMD",KEY_RIGHT_SUPER},{"LEFTCMD",KEY_LEFT_SUPER},
                                 {"LMB",MOUSE_BUTTON_1},{"RMB",MOUSE_BUTTON_2},{"MMB",MOUSE_BUTTON_3},{"MB3",MOUSE_BUTTON_4},{"MB4",MOUSE_BUTTON_5},{"MB5",MOUSE_BUTTON_6},{"MB6",MOUSE_BUTTON_7},{"MB7",MOUSE_BUTTON_8},{"JOY0",JOYSTICK_1},{"JOY1",JOYSTICK_2},{"JOY2",JOYSTICK_3},{"JOY3",JOYSTICK_4},{"JOY4",JOYSTICK_5},{"JOY5",JOYSTICK_6},{"JOY6",JOYSTICK_7},{"JOY7",JOYSTICK_8},{"JOY8",JOYSTICK_9},{"JOY9",JOYSTICK_10},{"JOY10",JOYSTICK_11},
                                 {"JOY11",JOYSTICK_12},{"JOY12",JOYSTICK_13},{"JOY13",JOYSTICK_14},{"JOY14",JOYSTICK_15},{"JOY15",JOYSTICK_16},{"JOY16",JOYHAT_UP},{"JOY17",JOYHAT_RIGHT},{"BACKSPACE",KEY_BACKSPACE},{"TAB",KEY_TAB},{"NUMENTER",KEY_KP_ENTER},{"ESCAPE",KEY_ESCAPE},{"SPACE",KEY_SPACE},{"DELETE",KEY_DELETE},{"INSERT",KEY_INSERT},{"HOME",KEY_HOME},{"END",KEY_END},{"PAGEUP",KEY_PAGE_UP},{"PAGEDN",KEY_PAGE_DOWN},{"F1",KEY_F1},{"F2",KEY_F2},
                                 {"F3",KEY_F3},{"F4",KEY_F4},{"F5",KEY_F5},{"F6",KEY_F6},{"F7",KEY_F7},{"F8",KEY_F8},{"F9",KEY_F9},{"F10",KEY_F10},{"F11",KEY_F11},{"F12",KEY_F12},{"GRAVE",KEY_GRAVE_ACCENT},{"-",KEY_MINUS},{"=",KEY_EQUAL},{"[",KEY_LEFT_BRACKET},{"]",KEY_RIGHT_BRACKET},{"\\",KEY_BACKSLASH},{"/",KEY_SLASH},{".",KEY_PERIOD},{",",KEY_COMMA},{";",KEY_SEMICOLON},{"'",KEY_APOSTROPHE},{"CAPSLOCK",KEY_CAPS_LOCK},{"NUM0",KEY_KP_0},{"NUM4",KEY_KP_4},
                                 {"NUM5",KEY_KP_5},{"NUM6",KEY_KP_6},{"NUM7",KEY_KP_7},{"NUM8",KEY_KP_8},{"NUM9",KEY_KP_9},{"NUM*",KEY_KP_MULTIPLY},{"NUM-",KEY_KP_SUBTRACT},{"NUM.",KEY_KP_DECIMAL},{"MENU",KEY_MENU},{"PAUSE",KEY_PAUSE},{"NUMLOCK",KEY_NUM_LOCK},{"MWHEEL+",127},{"MWHEEL-",128},/*Handled special case for mousewheel +/-respectively*/{"PRINT",KEY_PRINT_SCREEN},{"JOY18",JOYHAT_DOWN},{"JOY19",JOYHAT_LEFT},{"UNUSED",0}};
static bool uiWheelBlocked; static KeyState unboundInput; bool rebindCaptureActive = false;/*Set by the Input tab when a binding row is capturing a key; suppresses Menu() ESC-back + wheel capture while capturing.*/ bool UIInteractions(void); void UI_ProcessNavigation(void);
KeyState* GetCodeMapping(int settingIndex) { if (settingIndex<0 || settingIndex>=42) return &unboundInput; i32 i=Sys_Settings.InputCodeSettings[settingIndex]; if (i<0 || i>=(i32)(sizeof(inputElements)/sizeof(inputElements[0]))) return &unboundInput; if (i>=53 && i<=60) { int button=inputElements[i].value; return (uiMouseCaptured&(1u<<button)) ? &unboundInput : &Sys_Input.mouseButtons[button]; } return &Sys_Input.keyStates[inputElements[i].value]; }
void TextEntry(i32 k) {
    if (k == KEY_U && Sys_Input.keyStates[KEY_LEFT_CONTROL].down) { World.playerName[0] = '\0'; currentPlayerNameLength = 0; return; } if (k == KEY_ENTER || k == KEY_KP_ENTER) { currentMenuItem++; return; } if (k == KEY_BACKSPACE && currentPlayerNameLength > 0) { World.playerName[--currentPlayerNameLength] = '\0'; return; } if (currentPlayerNameLength >= 26) return;
    char c = (k >= KEY_A && k <= KEY_Z) ? 'a' + (k - KEY_A) : ((k >= KEY_1 && k <= KEY_9) ? '1' + (k - KEY_1) : ((k == KEY_0) ? '0' : ((k == KEY_SPACE) ? ' ' : 0))); if (c) { World.playerName[currentPlayerNameLength] = c; World.playerName[++currentPlayerNameLength] = '\0'; }
}
void ConsoleEmulator(i32 keycode); extern bool editFieldEditing; extern void EditFieldKey(i32 keycode); extern bool enteringSaveName; extern void SaveNameEntry(i32 keycode); extern void KeycodeEntryKey(i32 keycode);
void InputKey(char* keys,int key,int action) {
    if (key >= 0 && key < WS_KEYCOUNT) { if (action == INPUT_RELEASE && keys[key] == INPUT_RELEASE) { return; } if (action == INPUT_PRESS && keys[key] == INPUT_PRESS) { action = INPUT_REPEAT; } else { keys[key] = action; } }
    if (key == KEY_F10 && action) OS_Exit(0); if (!window_has_focus && action != INPUT_RELEASE) return;
    if (World.menuActive && !returnToPause) { if (((key == KEY_RIGHT_ALT || key == KEY_LEFT_ALT) && action && Sys_Input.keyStates[KEY_ENTER].down) || (key == KEY_ENTER && action && (Sys_Input.keyStates[KEY_LEFT_ALT].down || Sys_Input.keyStates[KEY_RIGHT_ALT].down /*Qmaster's Cheat!*/))){GoIntoGame();} }
    if (key >=0 && key < MAX_KEYS && (action == INPUT_PRESS || (action == INPUT_REPEAT && !(key == KEY_KP_ENTER || key == KEY_ENTER || key == KEY_TAB || key == KEY_ESCAPE)))) {
        Sys_Input.keyStates[key].down = true; Sys_Input.keyStates[key].pressed = (action == INPUT_PRESS);
        if (Cheats.consoleActive) ConsoleEmulator(key); else if (editFieldEditing) EditFieldKey(key); else if (World.Sys_UI.keycodeValid) KeycodeEntryKey(key); else if (enteringSaveName && World.menuActive) SaveNameEntry(key); else if (enteringPlayerName && World.menuActive) TextEntry(key);
    } else if (key >= 0 && key < MAX_KEYS && action == INPUT_RELEASE) { Sys_Input.keyStates[key].pressed=false; Sys_Input.keyStates[key].down=false; }
}
void InputMouseClick(char* mouseButtons, int button, int action) { if (button<0 || button>7) {return;} char wasDown = mouseButtons[button]; mouseButtons[button] = (char)action; bool down = (action == 1); Sys_Input.mouseButtons[button].pressed  = down && !wasDown; Sys_Input.mouseButtons[button].down=down; }
void quat_from_yaw_pitch_roll(Quaternion* q, float yaw_deg, float pitch_deg, float roll_deg) { float yaw=deg2rad(yaw_deg), pitch=deg2rad(pitch_deg), roll=deg2rad(roll_deg/*Around Z (forward)*/); float cy=vcosf(yaw * 0.5f), sy=vsinf(yaw * 0.5f), cp=vcosf(pitch * 0.5f), sp=vsinf(pitch * 0.5f), cr=vcosf(roll * 0.5f), sr=vsinf(roll * 0.5f); q->w=cy*cp*cr + sy*sp*sr; q->x=cy*sp*cr + sy*cp*sr;/*X(pitch)*/ q->y=sy*cp*cr - cy*sp*sr;/*Y(yaw)*/ q->z=cy*cp*sr - sy*sp*cr;/*Z(roll)*/ } // Skipping quat normalization, not needed
bool firstFrameMouselook = true;
void InputCursorPos(double* x, double* y, double xpos, double ypos) {
    if (firstFrameMouselook) { firstFrameMouselook=false; *x=xpos; *y=ypos; } if (*x == xpos && *y == ypos) { last_mouse_x=xpos; last_mouse_y=ypos; return;} *x=xpos; *y=ypos; if (!window_has_focus){return;} if (ignore_next_mouse_delta) { World.currentMouse_dx = World.currentMouse_dy = 0; ignore_next_mouse_delta = mouseMovementThisFrame = false; return; }
    World.currentMouse_dx += (i32)(xpos - last_mouse_x); World.currentMouse_dy += (i32)(ypos - last_mouse_y); last_mouse_x = xpos; last_mouse_y = ypos;
    if ((World.inventoryMode && !Cheats.noHUD) || World.menuActive || World.paused) {/*Uses UI baseline resolution 1366x768*/ i32 newX = vclampi(World.cursorPos_x + World.currentMouse_dx,0,UI_W); if (newX != World.cursorPos_x) {mouseMovementThisFrame = true;} World.cursorPos_x = newX; i32 newY = vclampi(World.cursorPos_y + World.currentMouse_dy,0,UI_H); if (newY != World.cursorPos_y) {mouseMovementThisFrame = true;} World.cursorPos_y = newY;}
}
static bool GetKeyRiseEdgeOrHeld(int sI, bool onRise) { i32 i = Sys_Settings.InputCodeSettings[sI]; if (i == 127) {return !uiWheelBlocked && Sys_Input.scrollDelta > 0;} if (i == 128) {return !uiWheelBlocked && Sys_Input.scrollDelta < 0;} KeyState* k = GetCodeMapping(sI); return onRise ? k->pressed : k->down; }
static bool GetKey(int settingIndex) { return GetKeyRiseEdgeOrHeld(settingIndex,false); }  // True while held down.
static bool GetKeyPressed(int settingIndex) { return GetKeyRiseEdgeOrHeld(settingIndex,true); } // True 1st frame down.
bool Forward() { return GetKey(0); }                bool StrafeLeft() { return GetKey(1); }             bool Backpedal() { return GetKey(2); }            bool StrafeRight() { return GetKey(3); }            bool Jump() { return GetKey(4); }                   bool JumpDown() { return GetKeyPressed(4); }
bool Crouch() { return GetKeyPressed(5); }          bool Prone() { return GetKeyPressed(6); }           bool LeanLeft() { return GetKey(7); }             bool LeanRight() { return GetKey(8); }              bool Sprint() { return GetKey(9); }                 bool TurnLeft() { return GetKey(10); }
bool TurnRight() { return GetKey(11); }             bool LookUp() { return GetKey(12); }                bool LookDown() { return GetKey(13); }            bool RecentLog() { return GetKeyPressed(14); }
bool Attack() { return GetKey(23); }         bool Use() { return GetKeyPressed(24); }            bool Menu() { return GetKeyPressed(25); }         bool Reload() { return GetKeyPressed(27); }
bool Grenade() { return GetKeyPressed(30); }        bool GrenCycUp() { return GetKeyPressed(31); }      bool GrenCycDn() { return GetKeyPressed(32); }    bool ChangeAmmoType() { return GetKeyPressed(33); } bool Patch() { return GetKeyPressed(34); }
bool PatchCycUp() { return GetKeyPressed(35); }     bool PatchCycDown() { return GetKeyPressed(36); }   bool SwimUp() {return (Cheats.noclip || World.curLev == LEVEL_CYBERSPACE) && GetKey(38);}             bool SwimDn() {return (Cheats.noclip || World.curLev == LEVEL_CYBERSPACE) && GetKey(39);}
bool DoubleTapLeanLeft(void)  { if(!GetKeyPressed(7)){ return false; } if (World.pauseRelativeTime < World.invP1.leanLTapFinished) { World.invP1.leanLTapFinished = 0.0; return true; } World.invP1.leanLTapFinished = World.pauseRelativeTime + 0.5; return false; }
bool DoubleTapLeanRight(void) { if(!GetKeyPressed(8)){ return false; } if (World.pauseRelativeTime < World.invP1.leanRTapFinished) { World.invP1.leanRTapFinished = 0.0; return true; } World.invP1.leanRTapFinished = World.pauseRelativeTime + 0.5; return false; } 
void ForceShootMode() { if (Sys_Settings.NoShootMode){ return; } if (World.inventoryMode) { World.cursorPos_x = 663; World.cursorPos_y = 364; ignore_next_mouse_delta = true; } World.Sys_UI.mouseClickHeldOverGUI=World.inventoryMode = false; CloseFullmap(); World.Sys_UI.vmailActive = 0; }
void ForceInventoryMode() { if (!World.inventoryMode) { World.inventoryMode = true; World.cursorPos_x = 683; World.cursorPos_y = 364; ignore_next_mouse_delta = true;} }
void ToggleInventoryMode() { if (World.inventoryMode) { ForceShootMode();} else {ForceInventoryMode(); } }
void ToggleConsole() { static bool imWasActPrior = false; editFieldEditing = false; if (!Cheats.consoleActive) {imWasActPrior = World.inventoryMode;} Cheats.consoleActive = !Cheats.consoleActive; World.paused = !World.paused; if (Cheats.consoleActive) { World.inventoryMode = true; } else if (!imWasActPrior && World.inventoryMode) {ForceShootMode();} }
void SaveGame(u8,const char*),LoadGame(u8),ApplyPlayerMovements(float);
void HwToggle(u8);
extern V3 debugWepOffset;
void InputProcessing() {
    for (int i=0;i<MAX_MOUSE_BUTTONS;++i) { if(!Sys_Input.mouseButtons[i].down){uiMouseCaptured &= ~(1u<<i);} }
    mouseMovementThisFrame = false; Sys_Input.scrollDelta = 0.0f; PollEvents(); World.Sys_UI.mouseClickHeldOverGUI = false; uiWheelBlocked = window_has_focus && UIInteractions(); World.uiIsBlocking = uiWheelBlocked;
    for (int i=0;i<MAX_MOUSE_BUTTONS;++i) if (uiWheelBlocked && Sys_Input.mouseButtons[i].pressed) uiMouseCaptured|=1u<<i;
    if (window_has_focus) {
        UI_ProcessNavigation();
        if (Sys_Input.keyStates[KEY_R].pressed && Cheats.editMode && !editFieldEditing) { bool foundValidDynamic = false; i8 dir = Sprint() ? -1 : 1; while (!foundValidDynamic) { editModeTestEntityDefinition += dir; if (Sprint() ? (editModeTestEntityDefinition < 307) : (editModeTestEntityDefinition > 767)) editModeTestEntityDefinition = Sprint() ? 767 : 307; if (IdxIsDynamicObject(editModeTestEntityDefinition)) foundValidDynamic = true; } }
        if (Sys_Input.keyStates[KEY_CAPS_LOCK].pressed) Sys_Input.isCapsLockOn = !Sys_Input.isCapsLockOn;
        if (Sys_Input.keyStates[KEY_F6].pressed && (get_time() - World.justSavedTimeStamp) > 0.2) { Sys_Input.keyStates[KEY_F6].pressed = false; if (!World.menuActive) { bool wasPaused=World.paused; World.paused=false; SaveGame(7,"quicksave"); if (wasPaused) World.paused=true; } return; }
        if (Sys_Input.keyStates[KEY_F9].pressed && (get_time() - World.justSavedTimeStamp) > 0.2) { Sys_Input.keyStates[KEY_F9].pressed = false; LoadGame(7); return; }
        if (GetKeyPressed(41)/*Console`~*/ && !rebindCaptureActive) ToggleConsole();
        if (Menu() && !World.menuActive && !editFieldEditing) { World.paused = !World.paused; return; }
        if (Menu() && World.menuActive && !rebindCaptureActive && VidMenuKeyArmed()) { MenuGoBack(); return; }
        if (World.paused || World.menuActive || Cheats.consoleActive || World.invP1.ressurectingFinished > World.pauseRelativeTime) return; // Pause/Menu barrier <<<<<<<
        if (Sys_Input.keyStates[KEY_1].pressed) { debugWepOffset.x += 0.05f; } else if (Sys_Input.keyStates[KEY_2].pressed) { debugWepOffset.x -= 0.05f; } // Debug weapon offset adjustment hooks TEMP TODO DELETE ME ONCE PIPE AND RAPIER OFFSETS ARE FIXED!!
        if (Sys_Input.keyStates[KEY_3].pressed) { debugWepOffset.y += 0.05f; } else if (Sys_Input.keyStates[KEY_4].pressed) { debugWepOffset.y -= 0.05f; }
        if (Sys_Input.keyStates[KEY_5].pressed) { debugWepOffset.z += 0.05f; } else if (Sys_Input.keyStates[KEY_6].pressed) { debugWepOffset.z -= 0.05f; }
        if (GetKeyPressed(26)/*ToggleMode*/) ToggleInventoryMode();
        if (GetKeyPressed(17)/*   Lantern*/) HwToggle(2);/*hwBtns[2]=HW_LAN, sOn/sOff 78*/
        if (GetKeyPressed(19)/*  Infrared*/) HwToggle(4);/*hwBtns[4]=HW_INF, sOn/sOff 98/82*/
        if (GetKeyPressed(15)/*Biomonitor*/) HwToggle(0);/*hwBtns[0]=HW_BIO; no-op while Cheats.showFPS, which holds the biomonitor on for the frame-time graphs*/
        if (GetKeyPressed(16)/*Sensaroung*/) { if (!(World.invP1.hasHardware & HW_SNS) || World.invP1.energy<=0.0f) CenterStatusPrint("%s",Sys_Text.stringTable[314]); else { play_wav(sounds[(World.invP1.hardwareIsActive&HW_SNS)?82:93],AppliedFXVol(1.0f),(V3){0},false); World.invP1.hardwareIsActive ^= HW_SNS; } }
        if (GetKeyPressed(18)/*    Shield*/) { if (!(World.invP1.hasHardware & HW_SHD) || World.invP1.energy<=0.0f) CenterStatusPrint("%s",Sys_Text.stringTable[314]); else { bool on = (World.invP1.hardwareIsActive & HW_SHD) != 0; play_wav(sounds[on ? 95 : 96],AppliedFXVol(1.0f),(V3){0},false); World.invP1.hardwareIsActive ^= HW_SHD; World.Sys_UI.shieldAnimationIsActivate = !on; World.Sys_UI.shieldAnimationStarted = World.pauseRelativeTime; World.Sys_UI.shieldAnimationFinished = World.pauseRelativeTime + (World.Sys_UI.shieldAnimationIsActivate ? 1.0 : 1.167); } }
        if (GetKeyPressed(20)/*     Email*/) {
            if (World.invP1.hasHardware & HW_ERD) {
                World.invP1.hardwareIsActive|=HW_ERD; MFD_ResetGeneral(); World.Sys_UI.mfdTab[0]=5; World.Sys_UI.mfdTab[1]=2; World.Sys_UI.mfdItemReader[0]=true; World.Sys_UI.MFD_ReaderView=MFD_READER_CONTENTS; World.Sys_UI.MFD_MediaTab=World.Sys_UI.lastMultiMediaTabOpened; if (World.Sys_UI.MFD_MediaTab>MM_NOTES || (World.Sys_UI.MFD_MediaTab==MM_NOTES && !World.diffMis)) World.Sys_UI.MFD_MediaTab=MM_LOG_TABLE; 
                play_wav(sounds[97],AppliedFXVol(1.0f),(V3){0},false);
            }
        }
        if (GetKeyPressed(21)/*  Booster*/) { if (!(World.invP1.hasHardware & HW_BST) || (World.invP1.energy <= 0.0f && World.invP1.hwVersSetting[HW_BST_IDX] >= 1)) { CenterStatusPrint("%s",Sys_Text.stringTable[314]); /*Power supply exhausted.*/ } else { play_wav(sounds[78],AppliedFXVol(1.0f),(V3){0},false); World.invP1.hardwareIsActive ^= HW_BST; } }
        if (GetKeyPressed(22)/* JumpJets*/) { if (!(World.invP1.hasHardware & HW_JET) || World.invP1.energy <= 0.0f) { CenterStatusPrint("%s",Sys_Text.stringTable[314]); /*Power supply exhausted.*/ } else { play_wav(sounds[78],AppliedFXVol(1.0f),(V3){0},false); World.invP1.hardwareIsActive ^= HW_JET; } }
        if (!editFieldEditing && (GetKeyPressed(28)/*WepCycUp*/ || (!uiWheelBlocked && Sys_Input.scrollDelta > 0))) { CycleWeaponSlot(Sys_Settings.InvertInventoryCycling ? -1 : +1); Sys_Input.scrollDelta = 0; }
        if (!editFieldEditing && (GetKeyPressed(29)/*WepCycDn*/ || (!uiWheelBlocked && Sys_Input.scrollDelta < 0))) { CycleWeaponSlot(Sys_Settings.InvertInventoryCycling ? +1 : -1); Sys_Input.scrollDelta = 0; }
        if (!editFieldEditing &&  GetKeyPressed(37)/*Map*/) UI_AutomapClick(false,UI_AUTOMAP_FULL);
        ApplyPlayerMovements(World.dt);
        if (!World.inventoryMode) { // Apply mouselook/keyboardlook/lean
            bool cyber = World.curLev == LEVEL_CYBERSPACE, invLook = cyber ? Sys_Settings.InvCybLook : Sys_Settings.InvertLook;
            float ns = vclamp((float)Sys_Settings.MouseSensitivity / 100.0f,0.01f,1.0f), s = ns * 0.2f, kts = 900.0f * ns;
            float dyaw = (float)World.currentMouse_dx * s + ((float)TurnRight() - (float)TurnLeft()) * kts * World.dt;
            float dpitch = ((float)World.currentMouse_dy * s + ((float)LookDown() - (float)LookUp()) * kts * World.dt) * (invLook ? -1.0f : 1.0f);
            World.cam_yaw += dyaw; if (World.cam_yaw >= 360.0f) {World.cam_yaw -= 360.0f;} if(World.cam_yaw<0.0f){World.cam_yaw+=360.0f;}
            World.cam_pitch += dpitch;
            if (cyber) {
                World.cam_pitch -= 360.0f * vfloor(World.cam_pitch * (1.0f/360.0f));
                Quaternion q = World.rotation[PLAYER1];
                if (dyaw != 0.0f) { float half = deg2rad(dyaw) * 0.5f; q = quat_multiply(q,(Quaternion){0.0f,vsinf(half),0.0f,vcosf(half)}); }
                if (dpitch != 0.0f) { float half = deg2rad(dpitch) * 0.5f; q = quat_multiply(q,(Quaternion){vsinf(half),0.0f,0.0f,vcosf(half)}); }
                float droll = ((float)LeanLeft() - (float)LeanRight()) * 60.0f * World.dt; if (droll != 0.0f) { float half = deg2rad(droll) * 0.5f; q = quat_multiply(q,(Quaternion){0.0f,0.0f,vsinf(half),vcosf(half)}); }
                float qlen = vsqrtf(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w); if (qlen > 0.000001f) { float invLen = 1.0f / qlen; q.x *= invLen; q.y *= invLen; q.z *= invLen; q.w *= invLen; }
                World.rotation[PLAYER1] = q;
            } else { if (World.cam_pitch > 89.0f) {World.cam_pitch = 89.0f;} else if (World.cam_pitch < -89.0f) {World.cam_pitch = -89.0f;} quat_from_yaw_pitch_roll(&World.rotation[PLAYER1],World.cam_yaw,World.cam_pitch,World.cam_roll); }
        }
    }
}
// Configuration Options Settings Sys
typedef enum { SETTING_U8, SETTING_U16, SETTING_INPUT } SettingType; typedef struct { const char* name; void* ptr; SettingType type; } Setting;
#define S_U8(n, v)  { n, &Sys_Settings.v, SETTING_U8 }
#define S_U16(n, v) { n, &Sys_Settings.v, SETTING_U16 }
#define S_IN(n, i)  { n, &Sys_Settings.InputCodeSettings[i], SETTING_INPUT }
const Setting configTable[] = {
    S_U16("ResolutionWidth",ScreenWidth), S_U16("ResolutionHeight",ScreenHeight), S_U8("Fullscreen",Fullscreen),           S_U8("FOV",FOV),                                       S_U8("Brightness",Brightness),           S_U8("AA",FXAA),          S_U8("Shadows",Shadows),                   S_U8("SSR",Reflections),           S_U8("VSync",Vsync),         S_U8("ModelDetail",ModelDetail), S_U8("GI",GI),
    S_U8("SpeakerMode",SpeakerMode),      S_U8("Reverb",Reverb),                  S_U8("VolumeMaster",VolumeMaster),       S_U8("VolumeMusic",VolumeMusic),                       S_U8("VolumeMessage",VolumeMessage),     S_U8("VolumeEffects",VolumeEffects),           S_U8("Language",Language),                 S_U8("DynamicMusic",DynamicMusic), S_U8("Footsteps",Footsteps),
    S_U8("InvertLook",InvertLook),        S_U8("Monitor",CurrentMonitor),         S_U8("InvertCyberspaceLook",InvCybLook), S_U8("InvertInventoryCycling",InvertInventoryCycling), S_U8("QuickItemPickup",QuickItemPickup), S_U8("QuickReloadWeapons",QuickReloadWeapons), S_U8("MouseSensitivity",MouseSensitivity), S_U8("NoShootMode",NoShootMode),   S_U8("HeadBob",HeadBob),
    S_IN("Forward",0),     S_IN("Strafe Left",1), S_IN("Backpedal",2),   S_IN("Strafe Right",3), S_IN("Jump",4),       S_IN("Crouch",5),     S_IN("Prone",6),      S_IN("Lean Left",7), S_IN("Lean Right",8), S_IN("Sprint",9),    S_IN("Turn Left",10), S_IN("Turn Right",11), S_IN("Look Up",12),     S_IN("Look Down",13),
    S_IN("Recent Log",14), S_IN("Biomonitor",15), S_IN("Sensaround",16), S_IN("Lantern",17),     S_IN("Shield",18),    S_IN("Infrared",19),  S_IN("Email",20),     S_IN("Booster",21),  S_IN("Jumpjets",22),  S_IN("Attack",23),   S_IN("Use",24),       S_IN("Menu/Back",25),  S_IN("Toggle Mode",26), S_IN("Reload",27),
    S_IN("Weapon +",28),   S_IN("Weapon -",29),   S_IN("Grenade",30),    S_IN("Grenade +",31),   S_IN("Grenade -",32), S_IN("Ammo Type",33), S_IN("Patch Use",34), S_IN("Patch +",35),  S_IN("Patch -",36),   S_IN("Full Map",37), S_IN("Swim Up",38),   S_IN("Swim Down",39),  S_IN("Screenshot",40),  S_IN("Console",41) };
void FilePrintString(FHandle f, const char* fmt, ...) { va_list a; __builtin_va_start(a,fmt); char b[128]; sFormatV(b,sizeof(b),fmt,a); __builtin_va_end(a); OS_RawWrite(f,b,slen(b)); }
void SaveConfig() {
    DualLog("Saving config\n"); FHandle f = OS_OpenWriteonly("./Data/Config.ini");
    for (int i=0;i<CONFIG_CNT;++i) { if (configTable[i].type == SETTING_U8) FilePrintString(f,"%s = %u\n",configTable[i].name,*(u8*)configTable[i].ptr); else if (configTable[i].type == SETTING_U16) FilePrintString(f,"%s = %u\n", configTable[i].name,*(u16*)configTable[i].ptr); else if (configTable[i].type == SETTING_INPUT) FilePrintString(f,"%s = %s\n", configTable[i].name,inputElements[*(u16*)configTable[i].ptr].name); }
    OS_Close(f); DualLog("Saved settings to ./Data/Config.ini! framenum %u\n", globalframe);
}
#include "audio.c"
// Biomonitor Graph and Text displays.
typedef struct { double beatFinished,ticksFinished[3],tickFinished; float heartRate,max[3],min[3],ecgValue,ergValue,chiValue,beatShift; u16 header; u8 rings[3][BIOM_GRAPH_W]; Color backgroundColor,ergColor,chiColor,ecgColor,col0,col1,col2; int ymax,currentIndex[3]; } BioMonitorSystem;
BioMonitorSystem bioMonitor; static u8 biomPx[BIOM_GRAPH_W*BIOM_GRAPH_H*4]; extern u32 biomTexId, biomFBO;
INLINE void biomPutPx(int x,int y,Color c){ u8* p=&biomPx[((u32)y*BIOM_GRAPH_W+(u32)x)*4]; p[0]=(u8)(vclamp(c.r,0.0f,1.0f)*255.0f); p[1]=(u8)(vclamp(c.g,0.0f,1.0f)*255.0f); p[2]=(u8)(vclamp(c.b,0.0f,1.0f)*255.0f); p[3]=(u8)(vclamp(c.a,0.0f,1.0f)*255.0f); }
void BiomonitorBlitToUI() { if(!(World.invP1.hardwareIsActive & HW_BIO)){return;} glBindFramebuffer(GL_READ_FRAMEBUFFER,biomFBO); glBindFramebuffer(GL_DRAW_FRAMEBUFFER,uiFBO); int x0=4,y0=UI_H-(41+BIOM_GRAPH_H),x1=4+BIOM_GRAPH_W,y1=UI_H-41; glBlitFramebuffer(0,0,BIOM_GRAPH_W,BIOM_GRAPH_H, x0,y0,x1,y1, GL_COLOR_BUFFER_BIT,GL_LINEAR); glBindFramebuffer(GL_READ_FRAMEBUFFER,uiFBO); glBindFramebuffer(GL_DRAW_FRAMEBUFFER,uiFBO); }
void BioMonitorClearGraphs() {
    bioMonitor.ymax = (BIOM_GRAPH_H - 1); bioMonitor.beatFinished=World.pauseRelativeTime; bioMonitor.ticksFinished[0]=World.pauseRelativeTime + 0.0211f; bioMonitor.ticksFinished[1]=World.pauseRelativeTime + 0.05f; bioMonitor.ticksFinished[2]=World.pauseRelativeTime + 0.0104f; bioMonitor.tickFinished=World.pauseRelativeTime + 0.02f;
    bioMonitor.currentIndex[0]=(int)(BIOM_GRAPH_W * random_range(0.0f,1.0f)); bioMonitor.currentIndex[1]=(int)(BIOM_GRAPH_W * random_range(0.0f,1.0f)); bioMonitor.currentIndex[2]=(int)(BIOM_GRAPH_W * random_range(0.0f,1.0f)); mset(bioMonitor.rings,0,sizeof(bioMonitor.rings));
}
void BioMonitorAlignCursors() { BioMonitorClearGraphs(); int idx=(int)(BIOM_GRAPH_W * random_range(0.0f,1.0f)); bioMonitor.currentIndex[0]=bioMonitor.currentIndex[1]=bioMonitor.currentIndex[2]=idx; }
static void IncrementGraph(int graph) { if (++bioMonitor.currentIndex[graph]>=BIOM_GRAPH_W) bioMonitor.currentIndex[graph]=0; }
static void Push(int index,float val) { if (index<BIOM_ERG || index>BIOM_ECG) return; bioMonitor.rings[index][bioMonitor.currentIndex[index]]=(u8)vclamp((int)(inverse_lerp(bioMonitor.min[index],bioMonitor.max[index],val)*BIOM_GRAPH_H),0,bioMonitor.ymax); }
void BiomonitorEnergyPulse(float take) { Push(0,take); IncrementGraph(0); Push(0,take); IncrementGraph(0); }
INLINE float biomMsToGraph(int graph, double ms, float fullScale) { return bioMonitor.min[graph] + vclamp((float)ms/fullScale,0.0f,1.0f) * (bioMonitor.max[graph] - bioMonitor.min[graph]); }
void BioMonitorUpdate() {
    if (!Cheats.showFPS && !(World.invP1.hasHardware & HW_BIO)) return; if (!(World.invP1.hardwareIsActive & HW_BIO)) return;
    bioMonitor.header = 526; if (bioMonitor.beatFinished < World.pauseRelativeTime) bioMonitor.heartRate = vfloor((70.0f + ((World.invP1.fatigue / 100.0f) * 110.0f)) * random_range(0.95f,1.05f));
    static const float beatThresh=0.1f, beatVariation=0.05f;
    /*Energy Usage / showFPS: GPU ms*/ bioMonitor.ergValue = Cheats.showFPS ? biomMsToGraph(BIOM_ERG,World.gpuFrameMs,33.0f) : vclamp((World.invP1.drainJPM / 255.0f),0.0f,1.0f);
    /*Chi Brain Waves / showFPS: CPU ms*/ float brainFactor = 0.15f; if (World.invP1.geniusFinished > World.pauseRelativeTime) brainFactor = 0.35f + random_range(-0.3f,0.3f); if(Cheats.showFPS){bioMonitor.chiValue=biomMsToGraph(BIOM_CHI,World.cpuFrameTime,33.0f);}else{bioMonitor.chiValue=(float)(vsinf(World.pauseRelativeTime * 10.0 * (double)brainFactor));}
    /*ECG: Create shifted sine wave for heart beat.  Apply percent fatigued to 200bpm max heart rate with baseline 50bpm.  showFPS overrides with total frame ms.*/ if (Cheats.showFPS) { bioMonitor.ecgValue = biomMsToGraph(BIOM_ECG,World.thisFrameTime,16.6666f); }
    else { float fatigueFactor = ((World.invP1.fatigue / 100.0f) * 120.0f) + 20.0f; fatigueFactor = fatigueFactor / 60.0f;
    if (bioMonitor.beatFinished < World.pauseRelativeTime) bioMonitor.beatFinished = World.pauseRelativeTime + (1.0 / (double)fatigueFactor);
    bioMonitor.beatShift = (bioMonitor.beatFinished - World.pauseRelativeTime) / (1.0 / (double)fatigueFactor);
    if (bioMonitor.beatShift > 0.94f) bioMonitor.ecgValue = vsinf(bioMonitor.beatShift * 35.0f); else bioMonitor.ecgValue = 0.0f;
    if (bioMonitor.ecgValue > beatThresh || bioMonitor.ecgValue < (beatThresh * -1.0f)) bioMonitor.ecgValue += random_range(-beatVariation,beatVariation); }
    if (Cheats.showFPS) {
        if (bioMonitor.ticksFinished[0] < World.pauseRelativeTime) { bioMonitor.ticksFinished[0] = World.pauseRelativeTime + 0.0104f; Push(0,bioMonitor.ergValue); IncrementGraph(0); }
        if (bioMonitor.ticksFinished[1] < World.pauseRelativeTime) { bioMonitor.ticksFinished[1] = World.pauseRelativeTime + 0.0104f; Push(1,bioMonitor.chiValue); IncrementGraph(1); }
        if (bioMonitor.ticksFinished[2] < World.pauseRelativeTime) { bioMonitor.ticksFinished[2] = World.pauseRelativeTime + 0.0104f; Push(2,bioMonitor.ecgValue); IncrementGraph(2); }
    } else {
        if (bioMonitor.ticksFinished[0] < World.pauseRelativeTime) { bioMonitor.ticksFinished[0] = World.pauseRelativeTime + 0.0211f; Push(0,bioMonitor.ergValue); IncrementGraph(0); Push(0,bioMonitor.ergValue); IncrementGraph(0); Push(0,bioMonitor.ergValue); }
        if (bioMonitor.ticksFinished[1] < World.pauseRelativeTime) { bioMonitor.ticksFinished[1] = World.pauseRelativeTime + 0.05f; Push(1,bioMonitor.chiValue); IncrementGraph(1); Push(1,bioMonitor.chiValue); IncrementGraph(1); Push(1,bioMonitor.chiValue); IncrementGraph(1); Push(1,bioMonitor.chiValue); }
        if (bioMonitor.ticksFinished[2] < World.pauseRelativeTime) { bioMonitor.ticksFinished[2] = World.pauseRelativeTime + 0.0104f; Push(2,bioMonitor.ecgValue); IncrementGraph(2); Push(2,bioMonitor.ecgValue); }
    }
    mset(biomPx,0,sizeof(biomPx)); const float fade[3]={200,180,275},invFade[3]={1.0f/200,1.0f/180,1.0f/275}; Color colors[3]={bioMonitor.ergColor,bioMonitor.chiColor,bioMonitor.ecgColor};
    for (int x=0;x<BIOM_GRAPH_W;++x) {
        float alpha[3]={0};
        for (int g=0;g<3;++g) {
            int y=bioMonitor.rings[g][x]; if (y<=0 || y>=BIOM_GRAPH_H) continue;
            float dp=bioMonitor.currentIndex[g]-x; if (BIOM_GRAPH_W-x<fade[g] && bioMonitor.currentIndex[g]<fade[g]) dp+=BIOM_GRAPH_W; if (dp<0 || dp>fade[g]) dp=fade[g]; alpha[g]=vclamp((fade[g]-dp)*invFade[g],0.0f,1.0f); if (alpha[g]<=0) continue;
            bool blocked=false; for (int k=0;k<g;++k) if (bioMonitor.rings[k][x]==y && alpha[k]>=0.01f) { blocked=true; break; } if (!blocked) { Color c=colors[g]; c.a=alpha[g]; biomPutPx(x,y,c); }
        }
    } glBindTexture(GL_TEXTURE_2D,biomTexId); glTexSubImage2D(GL_TEXTURE_2D,0,0,0,BIOM_GRAPH_W,BIOM_GRAPH_H,GL_RGBA,GL_UNSIGNED_BYTE,biomPx);
}
// Console System - CHEATS!
static i32 currentEntryLength=0, numHistory=0, historyPos=0; char consoleEntryText[T_BUFFER_SIZE],history[7][T_BUFFER_SIZE];
V3 ressurectionLocations[10] = {{-27.386f,-54.488f,26.5941f}/*0/R*/, {40.903f,-41.372f,-30.78f}/*1*/, {30.67407f,-24.832f,10.21412f}/*2*/, {38.26813f,-14.498f,20.37825f}/*3*/, {-19.48f,-6.928f,22.954f}/*4*/, {-24.358f,13.5956f,31.8497f}/*5*/,{-22.3568f,34.7845f,-30.728f}/*6*/,  {2.228084f,51.95243f,7.532025f}/*7*/, {10.068f,59.897f,13.973f}/*8*/, {2.303f,107.77f,-38.554f}/*9*/};
V3 cyberSpaceEntryLocations[8] = {{210.6834f,2.812f,-24.378f}/*0*/, {195.42f,-13.44f, 33.28f}/*1*/, {157.1608f,-15.53f,47.331f}/*2a, if cyberport localPosition.x < -26.0f*/, {256.0416f,-0.716f,62.48789f}/*2b level 2 secondary cyberport position*/,{126.43f,29.56733f,34.24f}/*5*/, {177.612f,3.29494f,108.7725f}/*6*/, {244.735f,41.99257f,-19.695f}/*8*/, {185.161f,84.502f,-46.04246f},/*9*/ };
void RecallHistory(int d) { if (d==1 && historyPos>0) --historyPos; else if (d==-1 && historyPos<numHistory) ++historyPos; else return; if (historyPos==numHistory) consoleEntryText[0]=currentEntryLength=0; else { scpy_to_a_from_b(consoleEntryText,history[historyPos],T_BUFFER_SIZE); currentEntryLength=slen(consoleEntryText); } }
typedef void (*ConsoleCmdFuncNoArg)(); typedef void (*ConsoleCmdFuncInt)(int); typedef void (*ConsoleCmdFuncStr)(const char*);
typedef struct { const char* name; union {ConsoleCmdFuncNoArg noArg; ConsoleCmdFuncInt withInt; ConsoleCmdFuncStr withStr; const char* text; u8 questOp;} func; enum {NOARG,CMD_INT,CMD_STR,CMD_PRINT,CMD_QUEST}type;} ConsoleCommand;
INLINE int CommandMatch(const char* in, const char* cmd) { while (*cmd && *in) { char c1 = c2Lower((u8)*in++); char c2 = c2Lower((u8)*cmd++); if (c1 == ' ' || c1 == '_') {c1 = ' ';} if (c2 == ' ' || c2 == '_') {c2 = ' ';} if (c1 != c2) {return 0;} } return *cmd == '\0' && (*in == '\0' || cEmpty((u8)*in) || *in == '_'); }
void cmd_noclip() { Cheats.noclip = !Cheats.noclip; if (Cheats.noclip) { World.velocity[PLAYER1] = (V3){ 0.0f, 0.0f, 0.0f }; CenterStatusPrint("noclip: %s", Sys_Text.stringTable[1000]); /*"ACTIVATED"*/} else {CenterStatusPrint("noclip: %s", Sys_Text.stringTable[717]); /*"DISABLED"*/} }
void cmd_showphys() { Cheats.showPhys = !Cheats.showPhys; CenterStatusPrint("showPhys: %s", Sys_Text.stringTable[Cheats.showPhys?1000:717]);/*"ACTIVATED"/"DISABLED"*/ }
void cmd_shownpc() { Cheats.showNPC = !Cheats.showNPC; CenterStatusPrint("shownpc: %s", Sys_Text.stringTable[Cheats.showNPC ? 1000 : 717]); }
static const i8 ArsenalSpawnOffsets[][2]={{2,0},{-2,0},{0,2},{0,-2},{1,1},{-1,1},{-1,-1},{1,-1},{3,0},{-3,0},{0,3},{0,-3},{2,1},{-2,1},{-2,-1},{2,-1},{1,2},{-1,2},{-1,-2},{1,-2},{4,0},{-4,0},{0,4},{0,-4},{3,1},{-3,1},{-3,-1},{3,-1},{2,2},{-2,2},{-2,-2},{2,-2},{1,3},{-1,3},{-1,-3},{1,-3},{5,0},{-5,0},{0,5},{0,-5},{4,1},{-4,1},{-4,-1},{4,-1},{3,2},{-3,2},{-3,-2},{3,-2},{2,3},{-2,3},{-2,-3},{2,-3},{1,4},{-1,4},{-1,-4},{1,-4}};
static bool IsCellOpenAt(V3 pos) { float x = pos.x - World.worldMin_x[World.curLev] + CELLXHALF; float z = pos.z - World.worldMin_z[World.curLev] + CELLXHALF; int cx = (int)(x / CELLSZ); int cz = (int)(z / CELLSZ); if (cx < 0 || cx >= WORLDX || cz < 0 || cz >= WORLDX) return false; u32 cell = (u32)cz * WORLDX + (u32)cx; return (gridCellStates[cell] & CELL_OPEN) != 0; }
static void SpawnArsenalItem(u16 c, int* ofs) { V3 p = World.position[PLAYER1]; for (int i = *ofs; i < (int)(sizeof(ArsenalSpawnOffsets)/sizeof(ArsenalSpawnOffsets[0])); ++i) { V3 sp={p.x+ArsenalSpawnOffsets[i][0]*0.32f,p.y,p.z+ArsenalSpawnOffsets[i][1]*0.32f}; if (IsCellOpenAt(sp)) { u16 spawned = SpawnDynamicObject(c); if (EntIdxIsValid(spawned)) { World.position[spawned] = sp; World.velocity[spawned] = (V3){0}; *ofs = i + 1; return; } } } }
void EnableCheatArsenal(u8 level) {
    static const u16 items0[]={349,345,352,357,358,374,374,373,373,373,375,359,359,321,321,327,327,324,324,329,328,330}; static const u16 items1[]={388,349,345,352,357,374,373,373,373,375,359,359,321,327,324,324,329,328,330,375,375,376,314,320}; static const u16 items2[]={388,392,391,345,352,357,373,373,373,375,359,359,321,327,324,324,329,328,330,375,375,376,314,320,314,376,375,318};
    static const u16 items3[]={346,392,391,388,348,345,352,357,373,373,373,375,359,359,321,327,324,324,329,328,330,375,375,376,314,320,314,376,375,318,381,382,381}; static const u16 items4[]={346,350,392,391,388,348,352,357,375,359,359,315,324,324,329,328,330,375,375,376,314,314,376,377,375,318,381,382,381,377};
    static const u16 items5[]={346,350,351,333,392,391,388,316,348,352,359,359,315,324,344,324,386,329,328,330,376,375,314,314,377,381,382,381,377,359,377,381,315,316}; static const u16 items6[]={356,385,384,346,350,343,351,333,392,391,388,316,348,347,315,324,324,386,329,328,330,318,317,314,314,377,319,384,382,377,379,377,381,315,316,384,383,384,315,318,318,318,318,317,319,379,379,383,383};
    static const u16 items7[]={356,385,384,350,343,351,333,316,348,347,315,324,324,386,329,330,318,317,314,314,377,319,384,377,379,377,362,315,316,384,384,317,319,379,379,383,383,386,353,386,324,384,362}; const u16* items; u32 count;
    switch (level) {
        case 1: items=items0; count=sizeof(items0)/sizeof(items0[0]); break; case 2: items=items1; count=sizeof(items1)/sizeof(items1[0]); break; case 3: items=items2; count=sizeof(items2)/sizeof(items2[0]); break; case 4: case 5: items=items3; count=sizeof(items3)/sizeof(items3[0]); break;
        case 6: items=items4; count=sizeof(items4)/sizeof(items4[0]); break; case 7: items=items5; count=sizeof(items5)/sizeof(items5[0]); break; case 8: items=items6; count=sizeof(items6)/sizeof(items6[0]); break; case 9: items=items7; count=sizeof(items7)/sizeof(items7[0]); break;          default:return;
    } int offset=0; for (u32 i=0;i<count;++i) SpawnArsenalItem(items[i],&offset);
}
void cmd_kill() { World.instances[PLAYER1].health = World.instances[PLAYER1].cyberHealth = 0.0f; CenterStatusPrint("%s", Sys_Text.stringTable[1011]); } // "Player decides to become a cyborg."
void cmd_undo() { if (Cheats.editMode) { if (lastSpawned < U16_MAX && lastSpawned >= INSTS_1ST_IDX) { DeleteInstance(lastSpawned); lastSpawned = U16_MAX; CenterStatusPrint("Last spawned object removed"); } else { CenterStatusPrint("Nothing to undo"); } } else { CenterStatusPrint("Cannot undo when not in Edit Mode"); } }
// Screen Shake
void ScreenShake() { World.shakeFinished = World.pauseRelativeTime + 1.0f; shakeAmp = 0.48f; } // The whole station is a shakin' and a movin'!
void cmd_shake() { ScreenShake(); CenterStatusPrint("SHAKIN LIKE A LEAF!"); }
// Edit Mode
void cmd_edit() { Cheats.editMode = !Cheats.editMode; if (Cheats.editMode) { Cheats.noclip = Cheats.notarget =  true; Cheats.editSubMode = 0; CenterStatusPrint("Edit Mode activated!" ); } else { Cheats.noclip = Cheats.notarget = false; editModeSelection = U16_MAX; editModeLightSelection = U16_MAX; CenterStatusPrint("Edit Mode deactivated"); } }
static void cmd_editsub(int submode) { if (submode < 0 || submode > 1) { CenterStatusPrint("Invalid edit submode: %d (0=select, 1=spawn physics objects)", submode); return; } Cheats.editSubMode = (u8)submode; CenterStatusPrint("Edit submode: %s", submode == 0 ? "select" : "spawn physics objects"); }
// Cheat Level Loading
int ParseLevelArg(const char* arg) {    
    char clean[64] = {0}; int j = 0; for (int i = 0; arg[i] && j < 60; i++) { if (arg[i] != ' ' && arg[i] != '_') { clean[j++] = c2Lower((u8)arg[i]); } } clean[j] = '\0'; if (sEqual(clean,"r")) { return 0; } if (sFindSub(clean,"g1") || sFindSub(clean,"10")) { return 10; } if (sFindSub(clean,"g2") || sFindSub(clean,"11")) { return 11; } if (sFindSub(clean,"g4") || sFindSub(clean,"12")) { return 12; }
    if (sFindSub(clean, "g3")) { CenterStatusPrint("%s", Sys_Text.stringTable[1001]); return -2; } /*"Gamma grove already jettisoned!"  Those poor arrogant people.*/ int level = s2i32(clean); if (level >= 0 && level < MAX_LEVELS) { return level; } return -1; // Invalid
}
u8 queuedLevelToLoad = 255u; V3 queuedLevelPos; bool ElevatorDefaultDestinationPos(u8,V3*);
static void cmd_loadlevel(const char* arg) {
    if (World.menuActive) { CenterStatusPrint("%s", Sys_Text.stringTable[1015]); return; /*"Cannot load levels via cheat while on the main menu!"*/} int level = ParseLevelArg(arg); if (level == -2) { return; /*Already printed message*/ } if (level < 0) { CenterStatusPrint("cmd_loadlevel invalid level argument %d", level); return; } if (level == World.curLev) { CenterStatusPrint("cmd_loadlevel already at level %d", level); return; }
    CenterStatusPrint("Loading level %u",level); queuedLevelToLoad=level;
    if (level == LEVEL_CYBERSPACE) { queuedLevelPos = cyberSpaceEntryLocations[World.curLev < 8 ? (u8)World.curLev : 0]; } else if (level > 9) { if (!ElevatorDefaultDestinationPos(level, &queuedLevelPos)) { queuedLevelPos = (V3){0.0f, 0.0f, 0.0f}; } else {  queuedLevelPos.y += PLYR_CAM_OFS_Y; } } else { queuedLevelPos = ressurectionLocations[level]; queuedLevelPos.y -= (1.0f - PLYR_CAM_OFS_Y); } LoadLevel(level,queuedLevelPos);
}
static void cmd_loadarsenal(const char* arg) { int level = ParseLevelArg(arg); if (level >= 0 && level < MAX_LEVELS) { EnableCheatArsenal(level); } }
static void cmd_summon(int itemConstIndex) {
    if (IdxInBounds(itemConstIndex)) {
        u16 spawned = SpawnDynamicObject(itemConstIndex);
        if (EntIdxIsValid(spawned)) { lastSpawned = spawned; V3 sdir = ScreenPointToRay(World.instances[PLAYER1].forward, World.instances[PLAYER1].right); World.position[spawned] = V3Add(World.position[PLAYER1], V3_ScaleByF(sdir, 2.0f)); World.velocity[spawned] = (V3){0.0f, 0.0f, 0.0f}; if (IdxIsHardware(itemConstIndex)) { int v = (int)World.invP1.hwVers[itemConstIndex - 328] + 1; World.instances[spawned].customIndex = (i16)(v > 4 ? 4 : v); } }
        CenterStatusPrint("Summoned object ID %d", itemConstIndex);
    } else { CenterStatusPrint("Invalid object ID: %s", itemConstIndex); }
}
static void cmd_notarget() { Cheats.notarget = !Cheats.notarget; CenterStatusPrint("notarget: %s", Cheats.notarget ? Sys_Text.stringTable[1000] : Sys_Text.stringTable[717]); }
static u16 showfpsSavedBioBit = 0;
static void cmd_showfps() { Cheats.showFPS = !Cheats.showFPS; if (Cheats.showFPS) { showfpsSavedBioBit = World.invP1.hardwareIsActive & HW_BIO; World.invP1.hardwareIsActive |= HW_BIO; BioMonitorAlignCursors(); } else { World.invP1.hardwareIsActive = (World.invP1.hardwareIsActive & ~HW_BIO) | showfpsSavedBioBit; } }
static void cmd_showlocation() { Cheats.showLocation = !Cheats.showLocation; }
static void cmd_god() { Cheats.god = !Cheats.god; CenterStatusPrint("god mode: %s", Cheats.god ? Sys_Text.stringTable[1000] : Sys_Text.stringTable[717]); }
static void cmd_energy() { Cheats.redbull = !Cheats.redbull; if (Cheats.redbull) {CenterStatusPrint("%s", Sys_Text.stringTable[1006]);/*"I feel the power! 0 energy consumption!"*/} else {CenterStatusPrint("%s", Sys_Text.stringTable[1005]);/*Energy usage normal*/} }
static const float skyRotateSpeeds[] = { 0.05f, 1.0f, 2.5f, 3.75f, 6.25f };
static void cmd_dizzy() { Cheats.dizzyLevel = (Cheats.dizzyLevel >= 3) ? 0 : Cheats.dizzyLevel + 1; }
static void cmd_bottomless() { Cheats.bottomless = !Cheats.bottomless; if (Cheats.bottomless) {CenterStatusPrint("bottomlessclip! %s",Sys_Text.stringTable[1002]);/*"Bring it!"*/} else {CenterStatusPrint("%s",Sys_Text.stringTable[1003]);/*"Hose disconnected from interdimensional wormhole. Normal ammo operation restored."*/} }
static void cmd_animtest() { Cheats.animTest++; if (Cheats.animTest > 2) {Cheats.animTest = 0;} if (Cheats.animTest == 1) {CenterStatusPrint("animation test looping enabled!");} else if (Cheats.animTest == 2) {CenterStatusPrint("animation test step enabled! Press 1");} else {CenterStatusPrint("animation test disabled");} }
static void cmd_nohud() { Cheats.noHUD = !Cheats.noHUD; if (Cheats.noHUD) {CenterStatusPrint("%s",Sys_Text.stringTable[1004]);/*"No HUD! Enjoy the cinematic screenshot experience!"*/} else { CenterStatusPrint("HUD %s",Sys_Text.stringTable[1000]);/*"ACTIVATED"*/} }
static void cmd_iamshodan() { Cheats.superoverride = !Cheats.superoverride; if (Cheats.superoverride) {CenterStatusPrint("%s",Sys_Text.stringTable[1010]);/*"Full security override enabled!"*/ } else {CenterStatusPrint("%s",Sys_Text.stringTable[1009]);/*"SHODAN has regained control of security from you"*/} }
static void cmd_staminup() { Cheats.fatigueCheat = !Cheats.fatigueCheat; if (Cheats.fatigueCheat) { CenterStatusPrint("Stamin-Up! %s",Sys_Text.stringTable[1013]); World.invP1.fatigue=0.0f; } else {CenterStatusPrint("%s",Sys_Text.stringTable[1012]); } }
static void cmd_qb_list() { CenterStatusPrint("missionBits: 0x%08X",(unsigned)World.missionBits); for(int i=0;i<QB_COUNT;++i){bool q=QuestBitIsSet((u8)i); CenterStatusPrint("  [%2d] %s = %u  note:%s  chk:%s",i,q?"ON ":"OFF",q,World.questNotesActive[i]?"yes":"no ",World.questNotesChecked[i]?"yes":"no");} }
static void cmd_git(const char* arg) {
    static const char* cmds[] = {"pull","remote: Enumerating objects: 24601, done.\nFailed, could not connect with origin/triop.","fetch", "remote: Enumerating objects: 24601, done.\nFailed, could not connect with origin/triop.","status","Your branch is up to date with origin/triop.\nWorking directory clean.","log", "<Merge pull request #451 from SHODAN/NeuralLinkBugfix> 6 months ago...",
                                 "reflog","dc51440 HEAD0 -> master: commit: Establish neural connection ... ERROR: invalid ID `2-4601`.","merge", "Failed, could not connect with origin/triop.","push","Could not find Username for 'triopttp://192.168.1.451'.","clone", "Failed, connection blocked by SHODAN. Employee ID invalid." };
    for (int i = 0; i < 16; i += 2) { if (sFindSub(arg,cmds[i])) { CenterStatusPrint(cmds[i+1]); return; } } if(sFindSub(arg,"branch") || sFindSub(arg, "-b")){ const char *last = StringFindLastChar(arg,' '); CenterStatusPrint("Created new branch %s",last ? last + 1 : "unknown");} else {CenterStatusPrint("Branch name not recognized. Contact your TriopBucket representative.");}
}
static void cmd_quit()      { OS_Exit(0); }
static const ConsoleCommand consoleCmds[] = {{"noclip",{.noArg=cmd_noclip},NOARG},{"idclip",{.noArg=cmd_noclip},NOARG},{"showphys",{.noArg = cmd_showphys},NOARG},{"god",{.noArg=cmd_god},NOARG},{"overwhelming",{.noArg=cmd_god},NOARG},{"whosyourdaddy",{.noArg = cmd_god},NOARG},{"iddqd",{.noArg=cmd_god},NOARG},{"notarget",{.noArg=cmd_notarget},NOARG},{"editmode",{.noArg=cmd_edit},NOARG},{"undo",{.noArg=cmd_undo},NOARG},{"showfps",{.noArg=cmd_showfps},NOARG},
                                             {"showlocation",{.noArg=cmd_showlocation},NOARG},{"editsub",{.withInt=cmd_editsub},CMD_INT},{"nohud",{.noArg=cmd_nohud},NOARG},{"bottomlessclip",{.noArg = cmd_bottomless},NOARG},{"load",{.withStr=cmd_loadlevel},CMD_STR},{"loadarsenal",{.withStr = cmd_loadarsenal},CMD_STR},{"summon_obj",{.withInt=cmd_summon},CMD_INT},{"motherlode",{.text="Nice try, there's no money here."},CMD_PRINT},
                                             {"rosebud",{.text="Nice try, there's no money here."},CMD_PRINT},{"kaching",{.text="Nice try, there's no money here."},CMD_PRINT},{"money",{.text="Nice try, there's no money here."},CMD_PRINT},{"dizzy",{.noArg=cmd_dizzy},NOARG},{"help",{.text="There's no one to save you now Hacker!"},CMD_PRINT},{"ifeelthepower",{.noArg = cmd_energy},NOARG},{"i am shodan", {.noArg=cmd_iamshodan},NOARG},
                                             {"iamshodan",{.noArg=cmd_iamshodan},NOARG},{"mr. bean",{.text="Nice try, there are no go carts to slow down here"},CMD_PRINT},{"simon foster",{.text="Nice try, nothing to paint here"},CMD_PRINT},{"richard branson",{.text="Nice try, there's no money here. You do realize this isn't Rollercoaster Tycoon right?"},CMD_PRINT},{"john wardley",{.text="WOW!"},CMD_PRINT},
                                             {"john mace",{.text="Nice try, there's nothing to pay double for here"},CMD_PRINT},{"melanie warn",{.text="I feel happy!!!"},CMD_PRINT},{"damon hill",{.text="Nice try, there are no go carts to speed up here"},CMD_PRINT},{"michael schumacher",{.text="Nice try, there are no go carts to give ludicrous speed here"},CMD_PRINT},{"tony day",{.text="Ok, now I want a hamburger"},CMD_PRINT},
                                             {"katie brayshaw",{.text="Hi there! Hello! Hey! Howdy!"},CMD_PRINT},{"sudo",{.text="Super user access granted...ERROR: access restricted by SHODAN!"},CMD_PRINT},{"admin",{.text="Super user access granted...ERROR: access restricted by SHODAN!"},CMD_PRINT},{"git",{.withStr=cmd_git},CMD_STR},{"restart",{.text="Yeah...better not"},CMD_PRINT},{"quit",{.noArg=cmd_quit},NOARG},{"exit", {.noArg = cmd_quit},NOARG},
                                             {"cd",{.text="Attempting to access directory... already at root"},CMD_PRINT},{"./",{.text="Attempting to access directory... already at root"},CMD_PRINT},{"kill",{.noArg = cmd_kill},NOARG},{"suicide",{.noArg=cmd_kill},NOARG},{"die",{.noArg=cmd_kill},NOARG},{"justinbailey",{.text="Well, you don't have a suit already so..."},CMD_PRINT},
                                             {"woodstock",{.text="How much wood could a woodchuck chuck...there's no wood in SPACE!"},CMD_PRINT},{"quarry",{.text="There's obsidian on levels 6 and 8 if you want to feel decadent,\notherwise we are lacking in the stone department."},CMD_PRINT},{"zelda",{.text="Too late, already been to level 1"},CMD_PRINT},
                                             {"allyourbasearebelongtous",{.text="ERROR: SHODAN has overriden your command, remove SHODAN first."},CMD_PRINT},{"all your base are belong to us",{.text="ERROR: SHODAN has overriden your command, remove SHODAN first."},CMD_PRINT},{"i am iron man",{.text="That's nice dear."},CMD_PRINT},{"i am amazing",{.text="That's nice dear."},CMD_PRINT},{"i am cool",{.text="That's nice dear."},CMD_PRINT},
                                             {"i am best",{.text="That's nice dear."},CMD_PRINT},{"idkfa",{.text="I can only hold 7 weapons!! Nice try dearies!"},CMD_PRINT},{"impulse 9",{.text="I can only hold 7 weapons!! Nice try dearies!"},CMD_PRINT},{"shake",{.noArg=cmd_shake},NOARG},{"tired",{.noArg=cmd_staminup},NOARG},{"staminup",{.noArg = cmd_staminup},NOARG},{"grok",{.text="Only AI allowed around here is SHODAN"},CMD_PRINT},
                                             {"chatgpt",{.text="Only AI allowed around here is SHODAN"},CMD_PRINT},{"claude",{.text="Only AI allowed around here is SHODAN"},CMD_PRINT},{"gemini",{.text="Only AI allowed around here is SHODAN"},CMD_PRINT},{"shodan",{.text="In my magnificence, I shape clay, crafting new lifeforms..."},CMD_PRINT},{"animtest",{.noArg = cmd_animtest},NOARG},{"qb_set",{.questOp=0},CMD_QUEST},{"qb_clear",{.questOp=1},CMD_QUEST},
                                             {"qb_toggle",{.questOp=2},CMD_QUEST},{"qb_list",{.noArg=cmd_qb_list},NOARG},{"shownpc",{.noArg=cmd_shownpc},NOARG},{NULL,{.noArg = NULL},NOARG}/*sizeof helper*/ };
void ProcessConsoleCommand(const char* c) {
    if (c == NULL || slen(c) == 0) { ToggleConsole(); return; }
    const char* ct = c;            while (*ct && cEmpty((u8)*ct)) { ct++; }
    const char* space = ct;        while (*space && !cEmpty((u8)*space)) { space++; }
    const char* arg_start = space; while (*arg_start && cEmpty((u8)*arg_start)) { arg_start++; }
    if ((numHistory == 0 || !sEqual(c,history[numHistory - 1]))) { if (numHistory < 7) { scpy_to_a_from_b(history[numHistory],c,T_BUFFER_SIZE); numHistory++; } else {for (int i = 0; i < 7 - 1; i++) {scpy_to_a_from_b(history[i],history[i + 1],T_BUFFER_SIZE);/*Shift list toward 0*/} scpy_to_a_from_b(history[7 - 1],c,T_BUFFER_SIZE);} }
    bool commandProcessed = false;
    for (u16 i=0;consoleCmds[i].name!=NULL;++i) {
        const ConsoleCommand* cmd = &consoleCmds[i];
        if (CommandMatch(ct, cmd->name)) {
            commandProcessed = true; if (cmd->type==CMD_PRINT) { CenterStatusPrint("%s",cmd->func.text); break; }
            if (cmd->type==CMD_QUEST && *arg_start) { int qb=s2i32(arg_start); if (qb<0 || qb>=QB_COUNT) CenterStatusPrint("Invalid quest bit: %d (valid 0-%d)",qb,QB_COUNT-1); else { u8 op=cmd->func.questOp; if (!op) QuestBitSet((u8)qb); else if (op==1) QuestBitClear((u8)qb); else QuestBitToggle((u8)qb); CenterStatusPrint("Quest bit %d %s -> %u",qb,op==0 ? "SET" : op==1 ? "CLEARED" : "TOGGLED",QuestBitIsSet((u8)qb)); } }
            else if (cmd->type == NOARG) { cmd->func.noArg(); }
            else if (cmd->type == CMD_STR && *arg_start) { cmd->func.withStr(arg_start); }
            else { if (!*arg_start) { CenterStatusPrint("Missing argument, usage: %s <number>", cmd->name); } else { cmd->func.withInt(s2i32(arg_start)); } }
            break;
        }
    }
    if (!commandProcessed) { CenterStatusPrint("%s%s",Sys_Text.stringTable[1014],ct); } /*"Unknown command or function: "*/ consoleEntryText[0] = currentEntryLength = 0; historyPos = numHistory; ToggleConsole();
}
void ConsoleEmulator(i32 keycode) {
    if (keycode==KEY_UP || keycode==KEY_DOWN) { RecallHistory(keycode==KEY_UP ? 1 : -1); return; } if (keycode==KEY_U && Sys_Input.keyStates[KEY_LEFT_CONTROL].down) { consoleEntryText[0]=currentEntryLength=0; return; }
    char c=keycode>=KEY_A && keycode<=KEY_Z ? 'a'+keycode-KEY_A : keycode>=KEY_1 && keycode<=KEY_9 ? '1'+keycode-KEY_1 : keycode==KEY_0 ? '0' : keycode==KEY_SPACE ? ' ' : 0; if (keycode==KEY_MINUS || keycode==KEY_KP_SUBTRACT) c=Sys_Input.keyStates[KEY_LEFT_SHIFT].down || Sys_Input.keyStates[KEY_RIGHT_SHIFT].down ? '_' : '-';
    if (c) { if (currentEntryLength<T_BUFFER_SIZE-1) { consoleEntryText[currentEntryLength++]=c; consoleEntryText[currentEntryLength]=0; } }
    else if (keycode==KEY_BACKSPACE && currentEntryLength>0) consoleEntryText[--currentEntryLength]=0;
    else if (keycode==KEY_ENTER || keycode==KEY_KP_ENTER) { DualLog("Console command: %s\n",consoleEntryText); ProcessConsoleCommand(consoleEntryText); }
}
// Raycast System
RaycastHit RayTriangle(V3 origin, V3 dir, V3 A, V3 B, V3 C) {
    RaycastHit miss = { .hit = false }; V3 AB = V3_AsubB(B, A), AC = V3_AsubB(C, A); V3 n = V3_Cross(AB, AC); V3 ao = V3_AsubB(origin, A); float det = (-V3_dot(dir, n)); float invDet = 1.0f / det; float d = V3_dot(ao,n)*invDet; if (d < 0.0f) return miss;
    V3 dao = V3_Cross(ao, dir); float u = V3_dot(AC,dao) * invDet; if (u < 0.0f) return miss; float v = (-V3_dot(AB, dao)) * invDet; if (v < 0.0f) return miss; float w = 1.0f - u-v; if (w < 0.0f) return miss;
    return (RaycastHit){.point = V3Add(origin, V3_ScaleByF(dir, d)), .normal = V3_Normalize(n), .distance = d, .hitInstanceIndex = INSTANCE_COUNT, .hit = vabs(det) >= 0.00000001f};
}
INLINE RaycastHit RaySphere(V3 origin, V3 dir, ShapeSphere sph, float maxDist) {
    RaycastHit h = {.distance=maxDist,.hitInstanceIndex=INSTANCE_COUNT}; float r = sph.rad; if (r < 0.0001f) return h; V3 oc = V3_AsubB(origin, sph.ctr); float b=V3_dot(oc,dir), c=V3_dot(oc,oc) - r * r; float disc=b*b - c; if (disc < 0.0f) return h; float s = vsqrtf(disc); float t = -b - s; if (t < 0.0f) t = -b + s; if (t < 0.0f || t > maxDist) return h;
    h.hit = true; h.distance = t; h.point = V3Add(origin, V3_ScaleByF(dir, t)); h.normal = V3_ScaleByF(V3_AsubB(h.point, sph.ctr), 1.0f / r); return h;
}
u16 EditModePickLight(V3 o, V3 dir, float maxDist, float* outDist) { u16 best = U16_MAX; float bd = maxDist; for (u16 i = 0; i < World.loadedLights; ++i) { Light* L = &World.lights[i]; if ((L->lflags & (LSPOT | LDIR)) || L->range <= 0.001f) { continue; } RaycastHit h = RaySphere(o,dir,(ShapeSphere){L->pos,0.02f*L->range},maxDist); if (h.hit && h.distance < bd) { best = i; bd = h.distance; } } if (outDist) { *outDist = bd; } return best; }
INLINE RaycastHit RayCapsule(V3 origin, V3 dir, ShapeCapsule cap, float maxDist) {
    RaycastHit h={.hit=false,.distance=maxDist,.point={0},.normal={0},.hitInstanceIndex=INSTANCE_COUNT}; float r = cap.rad; if (r < 0.0001f) return h; V3 ba = V3_AsubB(cap.tip,cap.base), oa = V3_AsubB(origin,cap.base), nBest = {0}; float baba = V3_dot(ba,ba); if (baba < 0.00001f) return RaySphere(origin, dir, (ShapeSphere){cap.base, r}, maxDist);
    float bard = V3_dot(ba,dir), baoa = V3_dot(ba,oa), tBest=-1.0f; float a = baba - bard * bard, b = baba * V3_dot(dir,oa) - baoa * bard, c = baba * V3_dot(oa,oa) - baoa * baoa - r * r * baba; float disc = b * b - a * c;
    if (vabs(a) >= 0.00001f && disc >= 0.0f) {
        float sh = vsqrtf(disc); float t0 = (-b - sh) / a; float y0 = baoa + t0 * bard; 
        if (t0 >= 0.0f && t0 <= maxDist && y0 > 0.0f && y0 < baba) { tBest = t0; V3 p = V3Add(origin,V3_ScaleByF(dir,t0)); V3 q=V3Add(cap.base,V3_ScaleByF(ba,y0/baba)); nBest=V3_Normalize(V3_AsubB(p,q));}
        else { float t1=(-b+sh)/a; float y1=baoa+t1*bard; if (t1 >= 0.0f && t1 <= maxDist && y1 > 0.0f && y1 < baba) { tBest=t1; V3 p=V3Add(origin,V3_ScaleByF(dir,t1)),q=V3Add(cap.base,V3_ScaleByF(ba,y1/baba)); nBest=V3_Normalize(V3_AsubB(p,q)); } }
    }
    for (int k = 0; k < 2; k++) {
        V3 ctr = k == 0 ? cap.base : cap.tip; V3 oc = V3_AsubB(origin, ctr); float bs = V3_dot(oc, dir); float cs = V3_dot(oc, oc) - r * r; float ds = bs * bs - cs; if (ds < 0.0f) continue; float shs = vsqrtf(ds); float ts = -bs - shs; if (ts < 0.0f) ts = -bs + shs; if (ts < 0.0f || ts > maxDist) continue; if (tBest >= 0.0f && ts >= tBest) continue;
        V3 ps = V3Add(origin,V3_ScaleByF(dir,ts)); float y = V3_dot(V3_AsubB(ps,cap.base), ba); if ((k == 0 && y > 0.0f) || (k == 1 && y < baba)) continue; tBest = ts; nBest = V3_ScaleByF(V3_AsubB(ps,ctr),1.0f / r);
    }
    if (tBest >= 0.0f) { h.hit = true; h.distance = tBest; h.point = V3Add(origin, V3_ScaleByF(dir, tBest)); h.normal = nBest; } return h;
}
INLINE RaycastHit RayBox(V3 origin, V3 dir, ShapeBox box, float maxDist) {
    RaycastHit h = {.hit=false,.distance=maxDist,.point={0},.normal={0},.hitInstanceIndex=INSTANCE_COUNT}; Quaternion inv = {-box.rot.x,-box.rot.y,-box.rot.z,box.rot.w}; V3 lo = quat_rot_v3(inv, V3_AsubB(origin, box.ctr)), ld = quat_rot_v3(inv, dir);
    float o[3] = {lo.x, lo.y, lo.z}, d[3] = {ld.x, ld.y, ld.z}, he[3] = {box.hExt.x, box.hExt.y, box.hExt.z}; float tmin = 0.0f, tmax = maxDist; int axis = -1; float asign = 0.0f;
    for (int a = 0; a < 3; ++a) { if (vabs(d[a]) < 0.000001f) { if (vabs(o[a]) > he[a]) return h; } else { float t1 = (-he[a] - o[a]) / d[a], t2 = (he[a] - o[a]) / d[a]; float tNear = vmin(t1,t2), tFar = vmax(t1,t2), ns = (d[a] > 0.0f) ? -1.0f : 1.0f; if (tNear > tmin) { tmin = tNear; axis = a; asign = ns; } if (tFar < tmax) { tmax = tFar; } if (tmin > tmax) { return h; } } }
    if (axis < 0) { h.hit = true; h.distance = 0.0f; h.point = origin; h.normal = V3_ScaleByF(dir, -1.0f); return h; } V3 ln = {0.0f, 0.0f, 0.0f}; if (axis == 0) { ln.x = asign; } else if (axis == 1) { ln.y = asign; } else { ln.z = asign; } h.hit = true; h.distance = tmin; h.point = V3Add(origin, V3_ScaleByF(dir, tmin)); h.normal = quat_rot_v3(box.rot, ln); return h;
}
float BvhRayAABBHit(V3 o, V3 d, V3 a, V3 b, float m) {
    float mn = 0.0f, mx = m;
    if (vabs(d.x) < 0.00000001f) { if (o.x < a.x || o.x > b.x) return -1.0f; } else { float inv=1.0f/d.x; float t1=(a.x - o.x) * inv, t2=(b.x - o.x) * inv; if(t1 > t2){float t=t1; t1=t2; t2=t;} if(t1 > mn){mn=t1;} if(t2 < mx){mx=t2;} if (mn > mx) return -1.0f; } /*X slab*/
    if (vabs(d.y) < 0.00000001f) { if (o.y < a.y || o.y > b.y) return -1.0f; } else { float inv=1.0f/d.y; float t1=(a.y - o.y) * inv, t2=(b.y - o.y) * inv; if(t1 > t2){float t=t1; t1=t2; t2=t;} if(t1 > mn){mn=t1;} if(t2 < mx){mx=t2;} if (mn > mx) return -1.0f; } /*Y slab*/
    if (vabs(d.z) < 0.00000001f) { if (o.z < a.z || o.z > b.z) return -1.0f; } else { float inv=1.0f/d.z; float t1=(a.z - o.z) * inv, t2=(b.z - o.z) * inv; if(t1 > t2){float t=t1; t1=t2; t2=t;} if(t1 > mn){mn=t1;} if(t2 < mx){mx=t2;} if (mn > mx) return -1.0f; } /*Z slab*/ return mn;
}
static u16 rayCellHeads[ARRSIZE],rayNext[INSTANCE_COUNT]; static u32 rayGridFrame=U32_MAX;
INLINE void RayGridRebuild(void) { rayGridFrame = globalframe; for (u32 c=0;c<ARRSIZE;++c) { rayCellHeads[c] = INSTANCE_COUNT; } for (u16 i=0;i<World.instCount;++i){ u16 c=(u16)World.instances[i].cellIndex; if (c>=ARRSIZE){ c = (u16)PosGetCellCoords(World.position[i].x, World.position[i].z); } rayNext[i] = rayCellHeads[c]; rayCellHeads[c] = i; } }
RaycastHit Raycast(V3 origin, V3 dir, float maxDist, u32 layerMask) {
    double rStart = get_time(); const float maxDistSq = maxDist * maxDist; dir = V3_Normalize(dir); RaycastHit result = {.hit = false,.distance = maxDist,.point = origin,.normal = dir,.hitInstanceIndex = INSTANCE_COUNT}; if (unlikely(rayGridFrame != globalframe)) { RayGridRebuild(); }
    float wx = World.worldMin_x[World.curLev], wz = World.worldMin_z[World.curLev], ex = origin.x + dir.x*maxDist, ez = origin.z + dir.z*maxDist;
    float xA = vmin(origin.x, ex) - 24.0f, xB = vmax(origin.x, ex) + 24.0f, zA = vmin(origin.z, ez) - 24.0f, zB = vmax(origin.z, ez) + 24.0f;
    i32 b0x = (i32)vfloor((xA - wx + CELLXHALF) / CELLSZ), b1x = (i32)vfloor((xB - wx + CELLXHALF) / CELLSZ), b0z = (i32)vfloor((zA - wz + CELLXHALF) / CELLSZ), b1z = (i32)vfloor((zB - wz + CELLXHALF) / CELLSZ);
    b0x = vclampi(b0x, 0, WORLDX - 1); b1x = vclampi(b1x, 0, WORLDX - 1); b0z = vclampi(b0z, 0, WORLDZ - 1); b1z = vclampi(b1z, 0, WORLDZ - 1);
    for (i32 cz=b0z;cz<=b1z;++cz) {
        for (i32 cx=b0x;cx<=b1x;++cx) {
            u32 cellP = (u32)cz*WORLDX + (u32)cx;
            for (u16 i=rayCellHeads[cellP]; i<INSTANCE_COUNT; i=rayNext[i]) {
                if (!(layerMask & World.layer[i])){ continue; } if (!(World.instances[i].entflags & EF_ACTIVE)){ continue; }
                u16 mindex = World.instances[i].modelIndex;
                V3 objPos = World.position[i];
                V3 delta = V3_AsubB(objPos,origin);
                u16 instCellIdx = PosGetCellCoords(objPos.x,objPos.z); if (!IdxIsPortalBlockingDoor(World.instances[i].index)) { if(((gridCellStates[instCellIdx] & (CELL_VISIBLE | CELL_OPEN)) == CELL_OPEN) && (World.instances[i].index != 754 || !SkyIsVisible())){continue;} }
                if (mindex >= MAX_MDLS) {
                    if (raycastPickMode == 1) continue; ColliderType ct = World.col[i]; if (ct != CT_CAP && ct != CT_SPH && (ct != CT_BOX || raycastPickMode != 2)) continue; if (V3_dot(delta, delta) >= maxDistSq) continue;
                    RaycastHit ch = {0}; if (ct == CT_CAP) { ch = RayCapsule(origin, dir, Entity_GetCap(i), result.distance); } else if (ct == CT_SPH) { ch = RaySphere(origin, dir, Entity_GetSph(i), result.distance); } else { ch = RayBox(origin, dir, Entity_GetBox(i), result.distance); } if (!ch.hit || ch.distance >= result.distance) { continue; }
                    ch.hitInstanceIndex = i; result = ch; continue;
                }
                if (mindex >= mdlsCnt) continue; if (raycastPickMode == 2) continue; if (V3_dot(delta,delta) >= maxDistSq) continue; u32 triCount = modelTriangleCounts[mindex]; if (triCount < 1) continue;
                const float* M=&modelMatrices[i*16];
                float m00=M[0], m10=M[1], m20=M[2], m01=M[4], m11=M[5], m21=M[6], m02=M[8], m12=M[9], m22=M[10], tx=M[12], ty=M[13], tz=M[14];
                float sclx2=m00*m00 + m10*m10 + m20*m20,sclx=vsqrtf(sclx2);
                float scly2=m01*m01 + m11*m11 + m21*m21,scly=vsqrtf(scly2);
                float sclz2=m02*m02 + m12*m12 + m22*m22,sclz=vsqrtf(sclz2);
                V3 rel = {origin.x - tx, origin.y - ty, origin.z - tz};
                V3 localOrigin = {(rel.x*m00 + rel.y*m10 + rel.z*m20) / sclx2, (rel.x*m01 + rel.y*m11 + rel.z*m21) / scly2, (rel.x*m02 + rel.y*m12 + rel.z*m22) / sclz2};
                V3 localDir    = {(dir.x*m00 + dir.y*m10 + dir.z*m20) / sclx2, (dir.x*m01 + dir.y*m11 + dir.z*m21) / scly2, (dir.x*m02 + dir.y*m12 + dir.z*m22) / sclz2};
                localDir = V3_Normalize(localDir);
                const float* posPtr = physPos[mindex];
                const   u16*   tris = physTris[mindex]; 
                if (BvhHasBVH(mindex)) {
                    const BvhNode* nodes = modelBVHNodes[mindex]; const u16* triOrder = modelBVHTriOrder[mindex]; float minScale = vmin(sclx, vmin(scly,sclz)); if(minScale < 0.0001f){minScale=0.0001f;} float localMax=maxDist/minScale; float bestT=localMax; const BvhNode* stack[64]; int sp = 0; stack[sp++] = &nodes[0];
                    while (sp > 0) {
                        const BvhNode* node = stack[--sp]; float tEntry = BvhRayAABBHit(localOrigin, localDir, node->mn, node->mx, bestT); if (tEntry < 0.0f) continue;
                        if (node->triCount > 0) {
                            for (u32 k=0;k<node->triCount;k++) {
                                u32 base = triOrder[node->triStart + k] * 3;
                                u32 iA=tris[base + 0], iB=tris[base + 1], iC=tris[base + 2];
                                V3 posA = {posPtr[iA*3],posPtr[iA*3+1],posPtr[iA*3+2]}, posB={posPtr[iB*3],posPtr[iB*3+1],posPtr[iB*3+2]}, posC={posPtr[iC*3],posPtr[iC*3+1],posPtr[iC*3+2]};
                                RaycastHit tryTri = RayTriangle(localOrigin,localDir,posA,posB,posC); if (!tryTri.hit) continue;
                                V3 worldPoint = { m00*tryTri.point.x + m01*tryTri.point.y + m02*tryTri.point.z + tx, m10*tryTri.point.x + m11*tryTri.point.y + m12*tryTri.point.z + ty, m20*tryTri.point.x + m21*tryTri.point.y + m22*tryTri.point.z + tz }; float worldDist = V3_Dist(worldPoint,origin); if (worldDist >= result.distance) continue;
                                V3 worldNormal={(m00/sclx)*tryTri.normal.x + (m01/scly)*tryTri.normal.y + (m02/sclz)*tryTri.normal.z,(m10/sclx)*tryTri.normal.x + (m11/scly)*tryTri.normal.y + (m12/sclz)*tryTri.normal.z,(m20/sclx)*tryTri.normal.x + (m21/scly)*tryTri.normal.y + (m22/sclz)*tryTri.normal.z };
                                result.hit=true; result.point=worldPoint; result.normal=V3_Normalize(worldNormal); result.distance=worldDist; result.hitInstanceIndex=i; bestT = tryTri.distance;
                            }
                        } else { for(int o=0;o<8&&sp<64;++o) { if(node->children[o] >= 0){stack[sp++]=&nodes[node->children[o]];} }  }
                    } continue;
                } DualLogError("Missing bvh for %u!!\n",mindex); OS_Exit(1);
            }
        }
    }
    raycastMs += get_time() - rStart;
    raycastCalls++;
    return result;
}
RaycastHit RaycastEditPick(V3 origin, V3 dir, float maxDist, u32 layerMask) { raycastPickMode = 1; RaycastHit h = Raycast(origin, dir, maxDist, layerMask); if (!h.hit) { raycastPickMode = 2; h = Raycast(origin, dir, maxDist, layerMask); } raycastPickMode = 0; return h; }
// Credits Sys
char creditStats[4096];
INLINE float GetScore(float stupid, bool isFinal) { float v=(float)(World.kills + World.cyberkills); if (isFinal) {v -= vmin(World.ressurections*10.0f,v*0.666f);} float s=vfloor((float)World.pauseRelativeTime / 3600.0f), score=v*10000.0f; score -= vmin(score*0.666f,s*100.0f); score *= (stupid + 1.0f) / 37.0f; if (stupid > 35.0f) {score += 2222222.0f;} return vfloor(score); }
INLINE void DecomposeTime(double t, u32* h, u32* m, double* s) { double tb = vfloor(t / 3600.0); *h = (u32)tb; t -= tb * 3600.0; tb = vfloor(t / 60.0); *m = (u32)tb; *s = t - tb * 60.0; }
INLINE void CreditsStats() {
    size_t off = 0; u32 h,m; double s; off += sFormat(creditStats + off, sizeof(creditStats)-off,"============================================================================\nCITADEL\n============================================================================\nCONGRATULATIONS %s\n",World.playerName);
    DecomposeTime(World.pauseRelativeTime,&h,&m,&s); off += sFormat(creditStats + off, sizeof(creditStats)-off,"Straight Time: %uh %um %.3fs\n",h,m,s); DecomposeTime(World.absoluteTime,&h,&m,&s);      off += sFormat(creditStats + off,sizeof(creditStats)-off,"Total Time (with reload from deaths): %uh %um %.3fs\n",h,m,s);
    float stupid = ((float)(World.diffCbt * World.diffCbt)) + ((float)(World.diffPuz * World.diffPuz)) + ((float)(World.diffMis * World.diffMis)) + ((float)(World.diffCyb * World.diffCyb)); u32 finalSubscore = GetScore(stupid,false), finalScore = (u32)GetScore(stupid,true);
    off += sFormat(creditStats + off,sizeof(creditStats)-off,"Kills: %u\nKills in Cyberspace: %u\nScoreSubtotal: %u\nDeaths: %u\nRessurections: %u\n",World.kills,World.cyberkills,(u32)finalSubscore,World.deaths,World.ressurections); off += sFormat(creditStats + off,sizeof(creditStats)-off,"Combat: %u | Puzzle: %u | Mission: %u | Cyber: %u\n",World.diffCbt,World.diffPuz,World.diffMis,World.diffCyb);
    off+=sFormat(creditStats+off,sizeof(creditStats)-off,"Difficulty Index: %.2f\nFinal Score: %u\n\n",stupid,finalScore); off+=sFormat(creditStats+off,sizeof(creditStats)-off,"Shots Fired: %u\nGrenades Thrown: %u\n",World.shotsFired,World.grenadesThrown); off+=sFormat(creditStats+off,sizeof(creditStats)-off,"Damage Dealt: %f\nDamage Received: %f\nSaves Scummed: %u\n\nClick to continue...\n",World.damageDealt,World.damageReceived,World.savesScummed);
}
// Rendering Sys
INLINE void ShaderError(u32 s, const char* name) { char er[4096]; glGetShaderInfoLog(s,4096,NULL,er); DualLogError("%s Comp Fail: %s\n",name,er); OS_Exit(1); }
INLINE void ProgramError(u32 p, const char* name) { char er[4096]; glGetProgramInfoLog(p,4096,NULL,er); DualLogError("%s Link Fail: %s\n",name,er); OS_Exit(1); }
INLINE u32 CompileShader(u32 type, const char* source, const char* name) { u32 s = glCreateShader(type); glShaderSource(s,1,&source,NULL); glCompileShader(s); i32 ok; glGetShaderiv(s,0x8B81/*GL_COMPILE_STATUS*/,&ok); if (!ok) ShaderError(s,name); return s; }
INLINE u32 LinkProgram(u32* s, i32 num, const char* name) { u32 p = glCreateProgram(); for (i32 i=0;i<num;++i) { glAttachShader(p,s[i]); } glLinkProgram(p); i32 ok; glGetProgramiv(p,0x8B82/*GL_LINK_STATUS*/,&ok); if (!ok) ProgramError(p,name); return p; }
u32 CompileAnyShader(const char* v, const char* s, const char* name) { return (v) ? LinkProgram((u32[]){CompileShader(0x8B31/*GL_VERTEX_SHADER*/,v,name),CompileShader(0x8B30/*GL_FRAGMENT_SHADER*/,s,name)},2,name) : LinkProgram((u32[]){CompileShader(0x91B9/*GL_COMPUTE_SHADER*/,s,name)},1,name); }
void CompileShaders() {
    depthPrepassSP = CompileAnyShader(depthPrepassVertSrc,depthPrepassFragSrc,"DPre"); chunkSP=CompileAnyShader(vertSrc,fragSrc,"Main"); uiSP=CompileAnyShader(vertUISrc,fragUISrc,"UI"); debugUnlitSP=CompileAnyShader(debugUnlitVertSrc,debugUnlitFragSrc,"Ln"); shadowmapsSP=CompileAnyShader(shadowmapVertSrc,shadowmapFragSrc,"Shad"); textSP=CompileAnyShader(textVertSrc,textFragSrc,"Txt");
    imageBlitSP=CompileAnyShader(quadVertSrc,quadFragSrc,"Comp"); ssrSP=CompileAnyShader(NULL,ssrCSSrc,"SSR"); rtSP=CompileAnyShader(NULL,rtCSSrc,"RT"); rtCopySP=CompileAnyShader(NULL,rtCopyCSSrc,"RTCopy"); voxelUpdateSP=CompileAnyShader(NULL,voxUpdCSSrc,"Vox"); particleSP=CompileAnyShader(particleVertSrc,particleFragSrc,"Part"); trailSP=CompileAnyShader(trailVertSrc,trailFragSrc,"Trail");
}
INLINE u32 MakeSSBO(u32* id, u32 bindx, size_t sz, const void* d, u32 typ) { glGenBuffers(1,id); glBindBuffer(GL_SSBO,*id); glBufferData(GL_SSBO,sz,d,typ); glBindBufferBase(GL_SSBO,bindx,*id); return *id; }
static void mat4_lookat_from(float* m, Quaternion* camRotation, V3 eye) {
    float x=camRotation->x, y=camRotation->y, z=camRotation->z, w=camRotation->w; float x2=x*x, y2=y*y, z2=z*z; float xy=x*y, xz=x*z, yz=y*z; float wx=w*x, wy=w*y, wz=w*z; V3 right={1.0f - 2.0f*(y2 + z2),2.0f*(xy + wz),2.0f*(xz - wy)};/*X+(right)*/ V3 up={2.0f*(xy - wz),1.0f - 2.0f*(x2 + z2),2.0f*(yz + wx)};/*Y+(up)*/ V3 forward={2.0f*(xz + wy),2.0f*(yz - wx), 1.0f - 2.0f*(x2 + y2)};/*Z+(forward)*/
    m[0]=right.x; m[1]=up.x; m[2]=-forward.x; m[3]=0.0f; m[4]=right.y; m[5]=up.y; m[6]=-forward.y; m[7]=0.0f; m[8]=right.z; m[9]=up.z; m[10]=-forward.z; m[11]=0.0f; m[12]=-V3_dot(right,eye); m[13]=-V3_dot(up,eye); m[14]=V3_dot(forward,eye); m[15]=1.0f;
}
void ExtractFrustumPlanes(float* m, FrustumPlane* ps) {
    ps[0].normal.x = m[3] + m[0]; ps[0].normal.y = m[7] + m[4]; ps[0].normal.z = m[11] + m[8];  ps[0].d = m[15] + m[12];/*Left*/
    ps[1].normal.x = m[3] - m[0]; ps[1].normal.y = m[7] - m[4]; ps[1].normal.z = m[11] - m[8];  ps[1].d = m[15] - m[12];/*Right*/
    ps[2].normal.x = m[3] + m[1]; ps[2].normal.y = m[7] + m[5]; ps[2].normal.z = m[11] + m[9];  ps[2].d = m[15] + m[13];/*Bottom*/
    ps[3].normal.x = m[3] - m[1]; ps[3].normal.y = m[7] - m[5]; ps[3].normal.z = m[11] - m[9];  ps[3].d = m[15] - m[13];/*Top*/
    ps[4].normal.x = m[3] + m[2]; ps[4].normal.y = m[7] + m[6]; ps[4].normal.z = m[11] + m[10]; ps[4].d = m[15] + m[14];/*Near*/
    ps[5].normal.x = m[3] - m[2]; ps[5].normal.y = m[7] - m[6]; ps[5].normal.z = m[11] - m[10]; ps[5].d = m[15] - m[14];/*Far*/
    for (int i=0;i<6;i++) { /*Normalize (could use V3_Normalize but need len for d term of FrustumPlane).*/ float len = V3_Mag(ps[i].normal); if (len > 0.000001f) { ps[i].normal.x /= len; ps[i].normal.y /= len; ps[i].normal.z /= len; ps[i].d /= len; } }
}
void mul_mat4(float* restrict out,const float* restrict a,const float* restrict b) { for (int c=0;c<4;++c) for (int r=0;r<4;++r) { float sum=0.0f; for (int k=0;k<4;++k) sum+=a[k*4+r]*b[c*4+k]; out[c*4+r]=sum; } }
__attribute__((noinline)) void RenderUIImageExtended(i16 x, i16 y, i16 width, i16 height, u32 texIndex, float radians, float pivotX, float pivotY, float tintR, float tintG, float tintB) {
    glUseProgram(uiSP);
    glBindVertexArray(textVAO);
    glUniform1ui(0, texIndex);
    glUniform4f(1, tintR, tintG, tintB, 1.0f);
    glBindBuffer(GL_ARRAY_BUFFER,textVBO);
    float w=(float)width, h=(float)height, cs=vcosf(radians), sn=vsinf(radians), pvx=w*pivotX, pvy=h*pivotY, z = 0.0f;
    const float cornerX[6] = {0.0f, w, w, 0.0f, 0.0f, w}, cornerY[6] = {h, 0.0f, h, h, 0.0f, 0.0f}, uvX[6] = {0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f}, uvY[6] = {0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f};
    float vertices[30];
    for (int i=0;i<6;++i) { float dx = cornerX[i] - pvx, dy = cornerY[i] - pvy; vertices[i*5 + 0] = (float)x + pvx + dx*cs - dy*sn; vertices[i*5 + 1] = (float)y + pvy + dx*sn + dy*cs; vertices[i*5 + 2] = z; /*Pos*/ vertices[i*5 + 3] = uvX[i]; vertices[i*5 + 4] = uvY[i]; /*UV*/ }
    glBufferSubData(GL_ARRAY_BUFFER,0,30 * sizeof(float),vertices);
    glDrawArrays(0x4/*GL_TRIANGLES*/,0,6); drawCalls++; uiDrawCalls++; vertsRendered += 6;
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
__attribute__((noinline)) void RenderUIImage(i16 x, i16 y, i16 width, i16 height, u32 texIndex) { RenderUIImageExtended(x,y,width,height,texIndex,0.0f,0.5f,0.5f,1.0f,1.0f,1.0f); }
void RenderLoading(const char * restrict text) { glBindFramebuffer(GL_FRAMEBUFFER, 0); glClear(GL_COLOR_BUFFER_BIT); glViewport(0, 0, Sys_Settings.ScreenWidth, Sys_Settings.ScreenHeight); RenderTextC(683, 384, T_WHITE, FONT_NORMAL, 1, text); window->context.swapBuffers(window); }
void GenerateAndBindTexture(u32 *id, i32 internalFormat, i32 width, i32 height, u32 format, u32 type, i32 filt, u8* bmp) { if (*id == 0) { glGenTextures(1, id); } glBindTexture(GL_TEXTURE_2D,*id); glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, bmp); glTexParameteri(GL_TEXTURE_2D, 0x2801/*GL_TEXTURE_MIN_FILTER*/, filt); glTexParameteri(GL_TEXTURE_2D, 0x2800/*GL_TEXTURE_MAG_FILTER*/, filt); }
void AddCamView(V3 p, Quaternion r, u8 fv, u16 w, u16 h, float nr, float fr) { if(camViewCount >= 64) { DualLogWarn("Too many cam views!  Skipped at %f %f %f\n",p.x,p.y,p.z); return; } camViews[camViewCount] = (CamView){p, r, fv, w, h, nr, fr, World.pauseRelativeTime + (camViewCount * 0.05f) + 0.5f, false}; GenerateAndBindTexture(&camViewTextures[camViewCount], GL_RGBA8, w, h, GL_RGBA,GL_UNSIGNED_BYTE, 0x2600/*GL_NEAREST*/, NULL); camViewCount++; }
void UpdateScreenSize(i32 width, i32 height) {
    u16 w = Sys_Settings.ScreenWidth = vmax(vmin((u16)width,7680u),320u), h = Sys_Settings.ScreenHeight = vmax(vmin((u16)height,4320u),200u); /*Cap at minimum Quake resolution and maximum 8k.*/
    glViewport(0,0,w,h);
    glUseProgram(imageBlitSP); glUniform1ui(2,w); glUniform1ui(3,h); glUniform1i(26,Sys_Settings.SSR_RES);
    glUseProgram(chunkSP);     glUniform1ui(6,w); glUniform1ui(7,h);
    glUseProgram(ssrSP);       glUniform1ui(0,w / Sys_Settings.SSR_RES); glUniform1ui(1,h / Sys_Settings.SSR_RES); glUniform1i(2,Sys_Settings.SSR_RES);
    glUseProgram(rtSP);        glUniform1ui(0,w / Sys_Settings.SSR_RES); glUniform1ui(1,h / Sys_Settings.SSR_RES); glUniform1i(2,Sys_Settings.SSR_RES);
    GenerateAndBindTexture(&inputImageID,                       GL_RGBA8,                       w,                       h,                     GL_RGBA,GL_UNSIGNED_BYTE,0x2600/*GL_NEAREST*/,NULL); /*Lit Raster*/
    GenerateAndBindTexture(&inputSpecID,                        GL_RGBA8,                       w,                       h,                     GL_RGBA,GL_UNSIGNED_BYTE,0x2600/*GL_NEAREST*/,NULL); /*Specular Colors*/
    GenerateAndBindTexture(&inputNormalID,                      GL_RG16F,                       w,                       h,                      GL_RGB,        GL_FLOAT,0x2600/*GL_NEAREST*/,NULL); /*Normal XYZ*/ 
    GenerateAndBindTexture(&inputDepthID, 0x81A7/*GL_DEPTH_COMPONENT32*/,                       w,                       h,0x1902/*GL_DEPTH_COMPONENT*/,        GL_FLOAT,0x2600/*GL_NEAREST*/,NULL); /*Raster Depth*/
    if (Sys_Settings.Reflections >= 1) { GenerateAndBindTexture(&outputImageID,GL_RGBA8,w / Sys_Settings.SSR_RES,h / Sys_Settings.SSR_RES,      GL_RGBA,GL_UNSIGNED_BYTE,0x2601/*GL_LINEAR*/ ,NULL); }
    glBindFramebuffer(GL_FRAMEBUFFER,gBufferFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER,         GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,inputImageID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER,         GL_COLOR_ATTACHMENT1,GL_TEXTURE_2D,inputSpecID,  0);
    glFramebufferTexture2D(GL_FRAMEBUFFER,         GL_COLOR_ATTACHMENT2,GL_TEXTURE_2D,inputNormalID,0);
    glFramebufferTexture2D(GL_FRAMEBUFFER,0x8D00/*GL_DEPTH_ATTACHMENT*/,GL_TEXTURE_2D,inputDepthID, 0);
    glBindImageTexture(0, inputImageID,0,GL_FALSE,0,GL_READ_WRITE,GL_RGBA8); /*Main Rendered Color*/
    glBindImageTexture(2,  inputSpecID,0,GL_FALSE,0,GL_READ_WRITE,GL_RGBA8); /*Specular*/
    glBindImageTexture(4, Sys_Settings.Reflections >= 1 ? outputImageID : 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8); /*SSR result*/
    glBindImageTexture(5,inputNormalID,0,GL_FALSE,0,GL_READ_WRITE,GL_RG16F); /*Normal XYZ*/
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, Sys_Settings.Reflections >= 1 ? outputImageID : 0);
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    ignore_next_mouse_delta = true;
}
//                        0 mk3 assault rifle              1 blaster             2 dartgun               3 flech                 4 ion       5 rapier         6 pipe               7 magnum            8 magpulse               9 pistol               10 plasma                 11 rail                              12 riot              13 skorp              14 sparq               15 stun
Quaternion vWepRot[16]={{0,.67623f,.73802f,0},{-.67623f,0,0,.73802f},{.10363f,0,0,.99456f},{0,.66976f,.74389f,0},{0,.68903f,.72611f,0},     {0,0,0,1},     {0,0,0,1},{.63662f,0,0,-.77238f},{0,.63662f,.77238f,0},{-.67623f,0,0,.73802f},{0,-.70781f,-.70781f,0},{0,-.65003f,-.76116f,0},{-.44581f,-.44581f,-.55061f,.55061f},{0,.67623f,.73802f,0},{0,.67623f,.73802f,0},{0,.67623f,.73802f,0}};                        
        V3 vWepOfs[16]={{      0,-.54f,.451f},        {0,-.5f,0.28f},  {-.015f,-.34f,.18f},       {0,-.43f,.27f},     {0,-0.57f,0.56f},{-.2f,.5f,.8f},  {0,-.3f,.7f},        {0,-.39f,.02f},       {0,-.54f,.44f},        {0,-.58f,.43f},     {-.02f,-.64f,.79f},         {0,-.46f,.43f},                       {0,-.5f,.08f},       {0,-.62f,.69f},       {0,-.55f,.58f},       {0,-.56f,.55f}};
extern const u16 wepModelIndices[16];
extern WeaponFireCtx wfx; // weapon view model: needed by RenderUI (ui.c)
static bool clearRenderTargetsOnNextFrame = false;
// CPU automap, side MFD, 320x200
static u8 amPx[AM_W*AM_H*4],amFullPx[AM_FULL_W*AM_FULL_H*4],amBuiltLev=255,amMdlState[MAX_MDLS]/*0=unknown,1=ok,2=no mesh*/,amMdlWall[MAX_MDLS]/*1=wall-like (draw), 0=floor-like (skip)*/,amHullN[MAX_MDLS],amCardState[MAX_MDLS]/*0=unknown,1=ok*/,amRadCells[ARRSIZE],amElevCells[ARRSIZE],amDiagCell[ARRSIZE],amDoorXR[256]/*1=X-running span, 0=Z-running*/;
static bool amBuiltFull=false;
static u32 amTexId = 0,amFBO = 0,amFullTexId = 0,amFullFBO = 0,amBuiltDoorHash = 0; static i32 amDoorN = 0,amDrawW,amDrawH; static u16 amWedgeInst[ARRSIZE]; static u8* amDrawPx;
typedef struct {float x,z;}AmPt;
static float amBuiltPX=1000000000000000000000000000000.0f,amBuiltPZ=1000000000000000000000000000000.0f,amBuiltFX=0.0f,amBuiltFZ=-1.0f,amWinX0,amWinZ1,amPxPerUnit,amHullX[MAX_MDLS][AM_MAXHULL],amHullZ[MAX_MDLS][AM_MAXHULL],amMinX[MAX_MDLS],amMaxX[MAX_MDLS],amMinZ[MAX_MDLS],amMaxZ[MAX_MDLS],amMinY[MAX_MDLS],amMaxY[MAX_MDLS],amCardX[MAX_MDLS][4],amCardY[MAX_MDLS][4],amCardZ[MAX_MDLS][4],amCardNx[MAX_MDLS],amCardNy[MAX_MDLS],amCardNz[MAX_MDLS],amFloorY[ARRSIZE],amDoorX0[256],amDoorZ0[256],amDoorX1[256],amDoorZ1[256];
INLINE int amNavVer() {/*nav hw ver: 0=none,1..3=v1..v3; gates zoom/overlays/cadence*/ return World.invP1.hwVers[HW_NAV_IDX]; }
static int amPtCmp(const void* a,const void* b) { const AmPt* pa=(const AmPt*)a,*pb=(const AmPt*)b; if(pa->x<pb->x){return -1;} if(pa->x>pb->x){return 1;} if(pa->z<pb->z){return -1;} if(pa->z>pb->z){return 1;} return 0; }/*monotone-chain hull of (x,z) verts; returns count*/
static u8 amComputeModel(u16 m) {
    if(amMdlState[m]){return amHullN[m];} amMdlState[m]=2; amHullN[m]=0; amMdlWall[m]=0; if(!physPos || !physVertCounts){return 0;} float* pos=physPos[m]; u32 vc=physVertCounts[m]; if(!pos||!vc){return 0;} float minX=1000000000000000000000000000000.0f,maxX=-1000000000000000000000000000000.0f,minY=1000000000000000000000000000000.0f,maxY=-1000000000000000000000000000000.0f,minZ=1000000000000000000000000000000.0f,maxZ=-1000000000000000000000000000000.0f; for(u32 i=0;i<vc;++i){float x=pos[i*3+0],y=pos[i*3+1],z=pos[i*3+2]; if(x<minX){minX=x;} if(x>maxX){maxX=x;} if(y<minY){minY=y;} if(y>maxY){maxY=y;} if(z<minZ){minZ=z;} if(z>maxZ){maxZ=z;}}
    float ySz=maxY-minY,xSz=maxX-minX,zSz=maxZ-minZ; amMinY[m]=minY; amMaxY[m]=maxY;/*cache Y extents even if floor-like (height detect)*/ if(ySz < 0.5f*vmin(xSz,zSz)){return 0;}/*floor/ceiling-like: no wall*/ static AmPt pts[2048]; u32 n=0; u32 stride=(vc>2048)?((vc+2047)/2048):1;  for (u32 i=0;i<vc && n<2048;i+=stride) { pts[n].x=pos[i*3+0]; pts[n].z=pos[i*3+2]; ++n; } if(n<3){return 0;} qsort_new(pts,n,sizeof(AmPt),amPtCmp); static AmPt hull[4096];
    u32 k=0; for (u32 i=0;i<n;++i) { while (k>=2) { AmPt a=hull[k-2],b=hull[k-1],c=pts[i]; float cr=(b.x-a.x)*(c.z-a.z)-(b.z-a.z)*(c.x-a.x); if(cr<=0.000000001f){--k;} else break; } hull[k++]=pts[i]; } u32 t=k+1; for (i32 i=(i32)n-2;i>=0;--i) { while (k>=t) { AmPt a=hull[k-2],b=hull[k-1],c=pts[i]; float cr=(b.x-a.x)*(c.z-a.z)-(b.z-a.z)*(c.x-a.x); if(cr<=0.000000001f){--k;} else break; } hull[k++]=pts[i]; } if(k>1){--k;}/*last point duplicates first*/
    if(k>AM_MAXHULL){k=AM_MAXHULL;} for (u32 i=0;i<k;++i) { amHullX[m][i]=hull[i].x; amHullZ[m][i]=hull[i].z; } amHullN[m]=(u8)k; amMdlWall[m]=1; amMdlState[m]=1; amMinX[m]=minX; amMaxX[m]=maxX; amMinZ[m]=minZ; amMaxZ[m]=maxZ; amMinY[m]=minY; amMaxY[m]=maxY; return amHullN[m];
}
INLINE u16 amCardModel(u16 m) { return (m < 306) ? 178 : m; }/*4 verts furthest from origin (card corners, no greebles). 1=ok,0=no mesh. normal via Newell.  model<306 uses 178 quad card. greebled geo ignored for diagonal/height; canonical card used*/
static u8 amComputeCard(u16 m) {
    if(amCardState[m]){return 1;} amCardState[m]=1;/*mark computed even on fail (no retry)*/ if(!physPos || !physVertCounts){return 0;} float* pos=physPos[m]; u32 vc=physVertCounts[m]; if(!pos || !vc || vc<4){return 0;}  float best[4]={-1,-1,-1,-1};/*4 largest dists from origin*/ u32 bidx[4]={0,0,0,0}; 
    for(u32 i=0;i<vc;++i){float x=pos[i*3+0],y=pos[i*3+1],z=pos[i*3+2]; float d2=x*x+y*y+z*z; for(int j=0;j<4;++j){if(d2>best[j]){for(int k=3;k>j;--k){best[k]=best[k-1]; bidx[k]=bidx[k-1];} best[j]=d2; bidx[j]=i; break;}}/*insertion sort best[4], descending*/}
    for (int j=0;j<4;++j) {u32 i=bidx[j]; amCardX[m][j]=pos[i*3+0]; amCardY[m][j]=pos[i*3+1]; amCardZ[m][j]=pos[i*3+2];}
    float nx=0,ny=0,nz=0; for (int j=0;j<4;++j){/*Newell normal*/ int k=(j+1)%4; float x0=amCardX[m][j],y0=amCardY[m][j],z0=amCardZ[m][j]; float x1=amCardX[m][k],y1=amCardY[m][k],z1=amCardZ[m][k]; nx+=(y0-y1)*(z0+z1); ny+=(z0-z1)*(x0+x1); nz+=(x0-x1)*(y0+y1);} float nl=vsqrtf(nx*nx+ny*ny+nz*nz); if(nl>0.000001f){ nx/=nl; ny/=nl; nz/=nl; } amCardNx[m]=nx; amCardNy[m]=ny; amCardNz[m]=nz; return 1;
}
INLINE void amPutPx(int x,int y,u8 r,u8 g,u8 b) {if((u32)x>=(u32)amDrawW || (u32)y>=(u32)amDrawH){return;} u8* p=&amDrawPx[((u32)y*amDrawW+(u32)x)*4]; p[0]=r; p[1]=g; p[2]=b; p[3]=255; }
static void amFillRect(int x0,int y0,int x1,int y1,u8 r,u8 g,u8 b) {/*x1,y1 exclusive*/if(x0<0){x0=0;} if(y0<0){y0=0;} if(x1>amDrawW){x1=amDrawW;} if(y1>amDrawH){y1=amDrawH;} for (int y=y0;y<y1;++y) for (int x=x0;x<x1;++x) { u8* p=&amDrawPx[((u32)y*amDrawW+(u32)x)*4]; p[0]=r; p[1]=g; p[2]=b; p[3]=255; } }
static void amFillTri(float ax,float ay,float bx,float by,float cx,float cy,u8 r,u8 g,u8 b) {
    float d=(by-cy)*(ax-cx)+(cx-bx)*(ay-cy); if(vabs(d)<0.000001f){return;} int x0=(int)vfloor(vmin(vmin(ax,bx),cx)),x1=(int)vceil(vmax(vmax(ax,bx),cx)); int y0=(int)vfloor(vmin(vmin(ay,by),cy)),y1=(int)vceil(vmax(vmax(ay,by),cy)); for(int y=y0;y<=y1;++y)for(int x=x0;x<=x1;++x){float px=(float)x+.5f,py=(float)y+.5f; float l1=((by-cy)*(px-cx)+(cx-bx)*(py-cy))/d,l2=((cy-ay)*(px-cx)+(ax-cx)*(py-cy))/d,l3=1.0f-l1-l2; if(l1>=-.001f && l2>=-.001f && l3>=-.001f){amPutPx(x,y,r,g,b);}}
}
static void amLine(float ax,float ay,float bx,float by,u8 r,u8 g,u8 b,int w) { float dx=bx-ax,dy=by-ay,len=vsqrtf(dx*dx+dy*dy); int n=(int)vceil(len); if(n<1){n=1;} int ww=w;/*fixed pixel width: line thickness is never scaled by map resolution*/ if(ww<1){ww=1;} int o0=-(ww/2),o1=o0+ww-1; for (int i=0;i<=n;++i) { float t=(float)i/(float)n; int x=(int)vround(ax+dx*t),y=(int)vround(ay+dy*t); for (int oy=o0;oy<=o1;++oy) for (int ox=o0;ox<=o1;++ox) amPutPx(x+ox,y+oy,r,g,b); }}
static void amCircle(float cx,float cy,float r,u8 cr,u8 cg,u8 cb) {int n=(int)(r*0.5f); if(n<16){n=16;} if(n>256){n=256;} float px=cx+r,py=cy; for (int i=1;i<=n;++i) {float a=(float)i/(float)n*6.2831853f; float x=cx+r*vcosf(a),y=cy+r*vsinf(a); amLine(px,py,x,y,cr,cg,cb,1); px=x; py=y;}}
INLINE float amWX(float x) { return (x-amWinX0)*amMapScaleX+amMapOffsetX; }
INLINE float amWZ(float z) { return (amWinZ1-z)*amMapScaleY; }
static u8 amEdgeKind(float x0,float z0,float x1,float z1,u8 lev) {/*edge kind: 0=closed-wall green, 1=height-step dark green*/
    float mx=(x0+x1)*0.5f,mz=(z0+z1)*0.5f; float x0c=World.worldMin_x[lev]-CELLXHALF,z0c=World.worldMin_z[lev]-CELLXHALF; int cx=(int)((mx-x0c)/CELLSZ),cz=(int)((mz-z0c)/CELLSZ); if(cx<0||cx>63||cz<0||cz>63){return 0;} float dx=x1-x0,dz=z1-z0; int ncx=cx,ncz=cz; u32 closedBit;
    if(vabs(dx)>=vabs(dz)){/*E-W: check N/S boundary*/float zb0=z0c+cz*CELLSZ,zb1=zb0+CELLSZ; if(vabs(mz-zb0)<0.35f){ closedBit=CELL_CLOSEDSOUTH; ncz=cz-1; } else if(vabs(mz-zb1)<0.35f){ closedBit=CELL_CLOSEDNORTH; ncz=cz+1; } else return 0; } else { float xb0=x0c+cx*CELLSZ,xb1=xb0+CELLSZ; if(vabs(mx-xb0)<0.35f){ closedBit=CELL_CLOSEDWEST; ncx=cx-1; } else if(vabs(mx-xb1)<0.35f){ closedBit=CELL_CLOSEDEAST; ncx=cx+1; } else return 0; }
    u32 cell=(u32)cz*64+(u32)cx; if(gridCellStates[cell]&closedBit){return 0;} if(ncx<0||ncx>63||ncz<0||ncz>63){return 0;} u32 ncell=(u32)ncz*64+(u32)ncx; if((gridCellStates[cell]&CELL_OPEN) && (gridCellStates[ncell]&CELL_OPEN)){return 1;} return 0;
}
INLINE int amEdgeHasDoor(int xrun,float fix,float a0,float a1) {
    for(int d=0;d<amDoorN;++d){if(amDoorXR[d]!=(u8)xrun){continue;} float midx=(amDoorX0[d]+amDoorX1[d])*0.5f, midz=(amDoorZ0[d]+amDoorZ1[d])*0.5f; float gap=xrun?vabs(midz-fix):vabs(midx-fix); if(gap>0.5f){continue;} float b0=xrun?vmin(amDoorX0[d],amDoorX1[d]):vmin(amDoorZ0[d],amDoorZ1[d]); float b1=xrun?vmax(amDoorX0[d],amDoorX1[d]):vmax(amDoorZ0[d],amDoorZ1[d]); float lo=a0>b0?a0:b0, hi=a1<b1?a1:b1; if(hi-lo>0.1f){return 1;} } return 0;
}
static void amNPCColor(NPCType t,u8* r,u8* g,u8* b) { if(t==NPCType_Cyborg||t==NPCType_Supercyborg||t==NPCType_MutantCyborg){ *r=159; *g=76; *b=77; }/*cyborg*/else if(t==NPCType_Mutant||t==NPCType_Supermutant){ *r=185; *g=134; *b=37; }/*mutant*/else { *r=104; *g=95; *b=166; }/*robot*/}
static int amNPCNeedVer(NPCType t) {if(t==NPCType_Robot){return 2;}if(t==NPCType_Cyborg||t==NPCType_Supercyborg||t==NPCType_MutantCyborg){return 3;}if(t==NPCType_Mutant||t==NPCType_Supermutant){return 3;}return 99;/*Cyber/unknown: no overlay*/}
static void amRaster(u8 zoom, bool full) {
    u8 lev=World.curLev; int nav=amNavVer(); int cells=16<<zoom;/*16,32,64; 0=closest*/ float x0c=World.worldMin_x[lev]-CELLXHALF,z0c=World.worldMin_z[lev]-CELLXHALF; float winW=(float)cells*CELLSZ; V3 app=World.position[PLAYER1]; float wantX0=app.x-winW*0.5f, wantZ1=app.z+winW*0.5f; float minX0=x0c,maxX0=x0c+64.0f*CELLSZ-winW; float minZ1=z0c+winW,maxZ1=z0c+64.0f*CELLSZ;
    amDrawW=full?AM_FULL_W:AM_W; amDrawH=full?AM_FULL_H:AM_H; amDrawPx=full?amFullPx:amPx;
    if (full) { amWinX0 = x0c; amWinZ1 = z0c + 64.0f*CELLSZ;}
    else      { amWinX0 = wantX0 < minX0 ? minX0 : (wantX0>maxX0?maxX0:wantX0); amWinZ1 = wantZ1 < minZ1 ? minZ1 : (wantZ1 > maxZ1 ? maxZ1 : wantZ1); if (maxX0 < minX0) { amWinX0 = minX0; } if (maxZ1 < minZ1) { amWinZ1 = maxZ1; } }
    amPxPerUnit=(float)amDrawH/winW; amMapScaleX=full?(float)amDrawW/winW:amPxPerUnit; amMapScaleY=amPxPerUnit; amMapOffsetX=full ? 0.0f : (float)((AM_W - AM_H) / 2);  u8* expl=World.automapExplored[lev]; mset(amDrawPx,0,(size_t)amDrawW*amDrawH*4);/*transparent*/
    mset(amRadCells,0,sizeof(amRadCells)); mset(amElevCells,0,sizeof(amElevCells)); mset(amWedgeInst,0xFF,sizeof(amWedgeInst)); mset(amDiagCell,0,sizeof(amDiagCell));
    if (nav >= 3) {/*rad triggers (nav v3+): expand bounds to overlapped cells*/
        for (u32 i=INSTS_1ST_IDX;i<World.instCount;++i) {
            Entity* e=&World.instances[i]; if(e->index!=601||!(e->entflags&EF_ACTIVE)){continue;} V3 c=World.colliderCenter[i],s=World.colliderSize[i],p=World.position[i]; float x0=p.x+c.x-s.x*0.5f,x1=p.x+c.x+s.x*0.5f,z0=p.z+c.z-s.z*0.5f,z1=p.z+c.z+s.z*0.5f;
            int ax=(int)vfloor((x0-x0c)/CELLSZ),bx=(int)vceil((x1-x0c)/CELLSZ); int az=(int)vfloor((z0-z0c)/CELLSZ),bz=(int)vceil((z1-z0c)/CELLSZ); if(ax<0){ax=0;} if(az<0){az=0;} if(bx>63){bx=63;} if(bz>63){bz=63;} for(int cz=az;cz<=bz;++cz)for(int cx=ax;cx<=bx;++cx)amRadCells[cz*64+cx]=1;
        }
    }
    for (u32 i=INSTS_1ST_IDX;i<World.instCount;++i) {/*elev dest cells (706): fill every cell the elev_volume box overlaps, not just the origin cell*/
        Entity* e=&World.instances[i]; if(e->index!=706||!(e->entflags&EF_ACTIVE)){continue;} V3 c=World.colliderCenter[i],s=World.colliderSize[i],p=World.position[i]; V3 rc=quat_rot_v3(World.rotation[i],c); float x0=p.x+rc.x-s.x*0.5f,x1=p.x+rc.x+s.x*0.5f,z0=p.z+rc.z-s.z*0.5f,z1=p.z+rc.z+s.z*0.5f;
        int ax=(int)vfloor((x0-x0c)/CELLSZ),bx=(int)vceil((x1-x0c)/CELLSZ); int az=(int)vfloor((z0-z0c)/CELLSZ),bz=(int)vceil((z1-z0c)/CELLSZ); if(ax<0){ax=0;} if(az<0){az=0;} if(bx>63){bx=63;} if(bz>63){bz=63;} for(int cz=az;cz<=bz;++cz)for(int cx=ax;cx<=bx;++cx)amElevCells[(u32)cz*64+(u32)cx]=1;
    }
    for(u32 c=0;c<ARRSIZE;++c){amFloorY[c]=-100000000000000000000000000000.0f;} float x0f=World.worldMin_x[lev]-CELLXHALF,z0f=World.worldMin_z[lev]-CELLXHALF;
    for (u32 i=INSTS_1ST_IDX;i<World.instCount;++i) {/*Get floor heights for difference check to draw dark green line between cells of different heights.*/
        Entity* e=&World.instances[i]; if(InstIsFuncWallChild(i)){continue;} if(!IdxIsGeometry(e->index) || !(e->entflags&EF_ACTIVE) || e->modelIndex>=MAX_MDLS){continue;} u16 m=e->modelIndex; u16 cm=amCardModel(m); if(!amComputeCard(cm)){continue;} V3 n=quat_rot_v3(World.rotation[i],(V3){amCardNx[cm],amCardNy[cm],amCardNz[cm]}); if(vabs(n.y)<0.7f){continue;}/*not horizontal*/
        V3 ip=World.position[i]; Quaternion iq=World.rotation[i]; V3 is=World.scale[i]; float sy=0,wx0=1000000000000000000000000000000.0f,wx1=-1000000000000000000000000000000.0f,wz0=1000000000000000000000000000000.0f,wz1=-1000000000000000000000000000000.0f;
        for(int k=0;k<4;++k){V3 v=quat_rot_v3(iq,(V3){amCardX[cm][k]*is.x,amCardY[cm][k]*is.y,amCardZ[cm][k]*is.z}); sy+=ip.y+v.y; float wx=ip.x+v.x, wz=ip.z+v.z; if(wx<wx0){wx0=wx;} if(wx>wx1){wx1=wx;} if(wz<wz0){wz0=wz;} if(wz>wz1){wz1=wz;}}
        float topY=sy*0.25f; int ax=(int)vfloor((wx0-x0f)/CELLSZ), bx=(int)vceil((wx1-x0f)/CELLSZ); int az=(int)vfloor((wz0-z0f)/CELLSZ), bz=(int)vceil((wz1-z0f)/CELLSZ); if(ax<0){ax=0;} if(az<0){az=0;} if(bx>63){bx=63;} if(bz>63){bz=63;} for (int cz=az;cz<=bz;++cz) for(int cx=ax;cx<=bx;++cx){u32 cell=(u32)cz*64+(u32)cx; if(topY>amFloorY[cell]){amFloorY[cell]=topY;}}
    }
    for (u32 i=INSTS_1ST_IDX;i<World.instCount;++i) {/*wedge inst per cell*/Entity* e=&World.instances[i]; if(InstIsFuncWallChild(i)){continue;} if(!IdxIsGeometry(e->index) || !(e->entflags&EF_ACTIVE) || e->modelIndex>=MAX_MDLS){continue;} u16 m=e->modelIndex; if(!amComputeModel(m) || !amMdlWall[m] || amHullN[m]!=3){continue;} int cx=PosGetCellCoordX(World.position[i].x),cz=PosGetCellCoordZ(World.position[i].z); if(cx>=0&&cx<64&&cz>=0&&cz<64){amWedgeInst[(u32)cz*64+(u32)cx]=(u16)i;}}
    for (u32 i=INSTS_1ST_IDX;i<World.instCount;++i) {/*diag-wall cells (card-based). diag cells skip closed edges; neighbors draw backs*/
        Entity* e=&World.instances[i]; if(InstIsFuncWallChild(i)){continue;} if(!IdxIsGeometry(e->index) || !(e->entflags&EF_ACTIVE) || e->modelIndex>=MAX_MDLS){continue;} u16 m=e->modelIndex; u16 cm=amCardModel(m); if(!amComputeCard(cm)){continue;} V3 wn=quat_rot_v3(World.rotation[i],(V3){amCardNx[cm],amCardNy[cm],amCardNz[cm]}); if(vabs(wn.y)>0.7f){continue;}/*horizontal: no wall*/ if(vabs(wn.x)<0.3f || vabs(wn.z)<0.3f){continue;}/*axis-aligned, not diagonal*/
        V3 ip=World.position[i]; Quaternion iq=World.rotation[i]; V3 is=World.scale[i]; float ccx=0,ccz=0; for (int k=0;k<4;++k) {V3 v=quat_rot_v3(iq,(V3){amCardX[cm][k]*is.x,amCardY[cm][k]*is.y,amCardZ[cm][k]*is.z}); ccx+=ip.x+v.x; ccz+=ip.z+v.z;} ccx*=0.25f; ccz*=0.25f; int cx=PosGetCellCoordX(ccx),cz=PosGetCellCoordZ(ccz); if(cx>=0&&cx<64&&cz>=0&&cz<64){amDiagCell[(u32)cz*64+(u32)cx]=1;}
    }
    for (int cz=0;cz<64;++cz) for (int cx=0;cx<64;++cx) {/*cell fills: transparent; rad dark orange; wedge cells get access tri only*/
        u32 cell=(u32)cz*64+(u32)cx; if(!(gridCellStates[cell]&CELL_OPEN)||!expl[cell]){continue;} float wx0=x0c+(float)cx*CELLSZ,wx1=wx0+CELLSZ; float wz0=z0c+(float)cz*CELLSZ,wz1=wz0+CELLSZ; float rx0=amWX(wx0),rx1=amWX(wx1),ry0=amWZ(wz1),ry1=amWZ(wz0);/*top-left px*/ if(rx1<0.0f||rx0>(float)amDrawW||ry1<0.0f||ry0>(float)amDrawH){continue;}/*outside zoom window*/ bool rad=amRadCells[cell]!=0,elev=amElevCells[cell]!=0; if(!rad&&!elev){continue;}/*black -> transparent: leave cleared alpha 0*/
        u8 r,g,b; if(elev){r=90;g=49;b=8;}/*elev cell 0.353,0.192,0.031*/ else if(lev==5||lev==11){r=132;g=104;b=0;}/*rad 0.518,0.408,0 on L5/L11*/ else {r=112;g=4;b=4;}/*rad 0.439,0.016,0.016*/ u16 wi=amWedgeInst[cell]; 
        if(rad&&wi!=U16_MAX){/*access tri = empty corner + hypotenuse ends*/
            Entity* e=&World.instances[wi]; u16 m=e->modelIndex; float wx[3],wz[3]; for (u8 k=0;k<3;++k) { V3 v=quat_rot_v3(World.rotation[wi],(V3){amHullX[m][k]*World.scale[wi].x,0.0f,amHullZ[m][k]*World.scale[wi].z}); wx[k]=amWX(World.position[wi].x+v.x); wz[k]=amWZ(World.position[wi].z+v.z); } float gx=(wx[0]+wx[1]+wx[2])/3.0f,gy=(wz[0]+wz[1]+wz[2])/3.0f; float corns[4][2]={{rx0,ry0},{rx1,ry0},{rx1,ry1},{rx0,ry1}}; int ei=0; float bd=-1.0f;
            for (int k=0;k<4;++k) { float dx=corns[k][0]-gx,dy=corns[k][1]-gy,d2=dx*dx+dy*dy; if(d2>bd){bd=d2;ei=k;} } int h0=0,h1=1; float bl=-1.0f;/*longest wedge edge = hypotenuse*/ for (int k=0;k<3;++k) { int a=k,bb=(k+1)%3; float dx=wx[a]-wx[bb],dy=wz[a]-wz[bb],d2=dx*dx+dy*dy; if(d2>bl){bl=d2;h0=a;h1=bb;} } amFillTri(corns[ei][0],corns[ei][1],wx[h0],wz[h0],wx[h1],wz[h1],r,g,b);
        } else {amFillRect((int)rx0,(int)ry0,(int)(rx1+0.5f),(int)(ry1+0.5f),r,g,b);}
    }
    amDoorN=0;
    for (u32 i=INSTS_1ST_IDX;i<World.instCount && amDoorN<256;++i) {/*door spans, world XZ, explored only. skip wall lines under doors*/
        Entity* de=&World.instances[i]; if(!IdxIsDoor(de->index) || !(de->entflags&EF_ACTIVE) || de->modelIndex>=MAX_MDLS){continue;} u16 dm=de->modelIndex; if(!amComputeModel(dm)){continue;} bool dis2cell=(de->index==499||de->index==508||de->index==509||de->index==510||de->index==511); float dmaxW=dis2cell?5.12f:2.56f; float dlx0,dlz0,dlx1,dlz1;
        if((amMaxX[dm]-amMinX[dm]) >= (amMaxZ[dm]-amMinZ[dm])){float dw=amMaxX[dm]-amMinX[dm]; if(dw>dmaxW){dw=dmaxW;} float dcx=(amMinX[dm]+amMaxX[dm])*0.5f, dcz=(amMinZ[dm]+amMaxZ[dm])*0.5f; dlx0=dcx-dw*0.5f; dlz0=dcz; dlx1=dcx+dw*0.5f; dlz1=dcz;} else {float dw=amMaxZ[dm]-amMinZ[dm]; if(dw>dmaxW){dw=dmaxW;} float dcx=(amMinX[dm]+amMaxX[dm])*0.5f, dcz=(amMinZ[dm]+amMaxZ[dm])*0.5f; dlx0=dcx; dlz0=dcz-dw*0.5f; dlx1=dcx; dlz1=dcz+dw*0.5f;}
        V3 dip=World.position[i]; Quaternion diq=World.rotation[i]; V3 dis=World.scale[i]; V3 dva=quat_rot_v3(diq,(V3){dlx0*dis.x,0.0f,dlz0*dis.z}); V3 dvb=quat_rot_v3(diq,(V3){dlx1*dis.x,0.0f,dlz1*dis.z}); float dax=dip.x+dva.x,daz=dip.z+dva.z,dbx=dip.x+dvb.x,dbz=dip.z+dvb.z; int decx=PosGetCellCoordX((dax+dbx)*0.5f),decz=PosGetCellCoordZ((daz+dbz)*0.5f); if(!expl[(u32)decz*64+(u32)decx]){continue;}/*door not drawn: keep wall line*/
        amDoorX0[amDoorN]=dax; amDoorZ0[amDoorN]=daz; amDoorX1[amDoorN]=dbx; amDoorZ1[amDoorN]=dbz; amDoorXR[amDoorN]=(u8)(vabs(dbx-dax)>=vabs(dbz-daz)); ++amDoorN;
    }
    for (int cz=0;cz<64;++cz) for (int cx=0;cx<64;++cx) {/*walls from grid closed edges (light green). grid-implicit; hulls miss them. diag cells skip closed edges. door-coincident edges skipped*/
        u32 cell=(u32)cz*64+(u32)cx; if(!expl[cell]){continue;} if(amDiagCell[cell]){continue;}/*diag: diagonal only*/ u32 st=gridCellStates[cell]; if(!(st&(CELL_CLOSEDNORTH|CELL_CLOSEDSOUTH|CELL_CLOSEDWEST|CELL_CLOSEDEAST))){continue;} /*cell px rect: rx0,ry0=top-left (NW)*/ float rx0=amWX(x0c+(float)cx*CELLSZ),rx1=amWX(x0c+(float)(cx+1)*CELLSZ); float ry0=amWZ(z0c+(float)(cz+1)*CELLSZ),ry1=amWZ(z0c+(float)cz*CELLSZ);
        if(rx1<0.0f||rx0>(float)amDrawW||ry1<0.0f||ry0>(float)amDrawH){continue;}/*outside zoom window*/ float ewx0=x0c+(float)cx*CELLSZ,ewx1=ewx0+CELLSZ; float ewz0=z0c+(float)cz*CELLSZ,ewz1=ewz0+CELLSZ; if(st&CELL_CLOSEDNORTH){ if(!amEdgeHasDoor(1,ewz1,ewx0,ewx1)){amLine(rx0,ry0,rx1,ry0,0,220,0,2);} } if(st&CELL_CLOSEDSOUTH){ if(!amEdgeHasDoor(1,ewz0,ewx0,ewx1)){amLine(rx0,ry1,rx1,ry1,0,220,0,2);} }
        if(st&CELL_CLOSEDWEST){ if(!amEdgeHasDoor(0,ewx0,ewz0,ewz1)){amLine(rx0,ry0,rx0,ry1,0,220,0,2);} } if(st&CELL_CLOSEDEAST){ if(!amEdgeHasDoor(0,ewx1,ewz0,ewz1)){amLine(rx1,ry0,rx1,ry1,0,220,0,2);} }
    }
    for (int cz=0;cz<64;++cz) for (int cx=0;cx<64;++cx) {/*height steps: dark green where neighbor floors differ. N+E neighbors only, no double-draw. closed walls skipped*/
        u32 cell=(u32)cz*64+(u32)cx; if(!expl[cell] || amFloorY[cell]<-100000000000000000000000000000.0f){continue;} float rx0=amWX(x0c+(float)cx*CELLSZ),rx1=amWX(x0c+(float)(cx+1)*CELLSZ); float ry0=amWZ(z0c+(float)(cz+1)*CELLSZ),ry1=amWZ(z0c+(float)cz*CELLSZ); if(rx1<0.0f||rx0>(float)amDrawW||ry1<0.0f||ry0>(float)amDrawH){continue;}/*outside zoom window*/ u32 st=gridCellStates[cell];
        if(cz+1<64){/*N neighbor*/u32 ncell=(u32)(cz+1)*64+(u32)cx; u32 nst=gridCellStates[ncell]; if(!(st&CELL_CLOSEDNORTH) && !(nst&CELL_CLOSEDSOUTH) && expl[ncell] && amFloorY[ncell]>-100000000000000000000000000000.0f){float dh=vabs(amFloorY[cell]-amFloorY[ncell]); if(dh>=0.16f){amLine(rx0,ry0,rx1,ry0,0,110,0,2);}/*north edge*/}}
        if(cx+1<64){/*E neighbor*/u32 ncell=(u32)cz*64+(u32)(cx+1); u32 nst=gridCellStates[ncell]; if(!(st&CELL_CLOSEDEAST) && !(nst&CELL_CLOSEDWEST) && expl[ncell] && amFloorY[ncell]>-100000000000000000000000000000.0f){float dh=vabs(amFloorY[cell]-amFloorY[ncell]); if(dh>=0.16f){amLine(rx1,ry0,rx1,ry1,0,110,0,2);}/*east edge*/}}
    }
    for (u32 i=INSTS_1ST_IDX;i<World.instCount;++i) {/*diag walls: vertical cards with X+Z normal (non-axis). longest XZ span drawn. chunks use 178 card*/
        Entity* e=&World.instances[i]; if(InstIsFuncWallChild(i)){continue;} if(!IdxIsGeometry(e->index) || !(e->entflags&EF_ACTIVE) || e->modelIndex>=MAX_MDLS){continue;} u16 m=e->modelIndex; u16 cm=amCardModel(m); if(!amComputeCard(cm)){continue;} /*world normal via instance rot*/ V3 wn=quat_rot_v3(World.rotation[i],(V3){amCardNx[cm],amCardNy[cm],amCardNz[cm]}); /*diag wall check: vertical, non-axis*/ if(vabs(wn.y)>0.7f){continue;}/*horizontal: no wall*/
        if(vabs(wn.x)<0.3f || vabs(wn.z)<0.3f){continue;}/*axis-aligned, grid handles it*/
        V3 ip=World.position[i]; Quaternion iq=World.rotation[i]; V3 is=World.scale[i]; float wx[4],wz[4]; for (int k=0;k<4;++k) { V3 v=quat_rot_v3(iq,(V3){amCardX[cm][k]*is.x,amCardY[cm][k]*is.y,amCardZ[cm][k]*is.z}); wx[k]=ip.x+v.x; wz[k]=ip.z+v.z; }
        /*FoW: card center*/float ccx=(wx[0]+wx[1]+wx[2]+wx[3])*0.25f, ccz=(wz[0]+wz[1]+wz[2]+wz[3])*0.25f; int ecx=PosGetCellCoordX(ccx),ecz=PosGetCellCoordZ(ccz); if(!expl[(u32)ecz*64+(u32)ecx]){continue;}
        /*longest XZ span*/int bi=0,bj=1; float bd=-1.0f; for (int a=0;a<4;++a) for (int b=a+1;b<4;++b) {float dx=wx[a]-wx[b],dz=wz[a]-wz[b],d2=dx*dx+dz*dz; if(d2>bd){bd=d2;bi=a;bj=b;} }
        float x0=amWX(wx[bi]),y0=amWZ(wz[bi]),x1=amWX(wx[bj]),y1=amWZ(wz[bj]); amLine(x0,y0,x1,y1,0,220,0,2);
    }
    for (u32 i=INSTS_1ST_IDX;i<World.instCount;++i) {/*hull outlines: dark green open steps only. closed walls drawn from grid*/
        Entity* e=&World.instances[i]; if(InstIsFuncWallChild(i)){continue;} if(!IdxIsGeometry(e->index) || !(e->entflags&EF_ACTIVE) || e->modelIndex>=MAX_MDLS){continue;} u16 m=e->modelIndex; if(!amComputeModel(m) || !amMdlWall[m]){continue;} u8 hn=amHullN[m]; if(hn<3){continue;} float px[AM_MAXHULL],pz[AM_MAXHULL],wx[AM_MAXHULL],wz[AM_MAXHULL]; V3 ip=World.position[i]; Quaternion iq=World.rotation[i]; V3 is=World.scale[i];
        for (u8 k=0;k<hn;++k) { V3 v=quat_rot_v3(iq,(V3){amHullX[m][k]*is.x,0.0f,amHullZ[m][k]*is.z}); wx[k]=ip.x+v.x; wz[k]=ip.z+v.z; px[k]=amWX(wx[k]); pz[k]=amWZ(wz[k]); }
        for (u8 k=0;k<hn;++k) {
            u8 a=k,b=(u8)((k+1)%hn); float mx=(wx[a]+wx[b])*0.5f,mz=(wz[a]+wz[b])*0.5f; int ecx=PosGetCellCoordX(mx),ecz=PosGetCellCoordZ(mz); if(!expl[(u32)ecz*64+(u32)ecx]){continue;} if(amEdgeKind(wx[a],wz[a],wx[b],wz[b],lev)){amLine(px[a],pz[a],px[b],pz[b],0,110,0,2);}
        }
    }
    for (u32 i=INSTS_1ST_IDX;i<World.instCount;++i) {/*doors: closed=yellow span; open=yellow center ticks. bulkheads (499/509) + big elevator doors (508/510/511): end ticks only, 2 per cell. span=width axis, clamp 1 cell (2.56), 2-cell doors 2 cells (5.12). no card geo*/
        Entity* e=&World.instances[i]; if(!IdxIsDoor(e->index) || !(e->entflags&EF_ACTIVE) || e->modelIndex>=MAX_MDLS){continue;} u16 m=e->modelIndex; if(!amComputeModel(m)){continue;} /*width-axis ends, local space, clamped*/bool is2cell=(e->index==499||e->index==508||e->index==509||e->index==510||e->index==511); float maxW=is2cell?5.12f:2.56f; float lx0,lz0,lx1,lz1;
        if((amMaxX[m]-amMinX[m]) >= (amMaxZ[m]-amMinZ[m])){float w=amMaxX[m]-amMinX[m]; if(w>maxW){w=maxW;} float cx=(amMinX[m]+amMaxX[m])*0.5f, cz=(amMinZ[m]+amMaxZ[m])*0.5f; lx0=cx-w*0.5f; lz0=cz; lx1=cx+w*0.5f; lz1=cz;}else {float w=amMaxZ[m]-amMinZ[m]; if(w>maxW){w=maxW;} float cx=(amMinX[m]+amMaxX[m])*0.5f, cz=(amMinZ[m]+amMaxZ[m])*0.5f; lx0=cx; lz0=cz-w*0.5f; lx1=cx; lz1=cz+w*0.5f;}
        V3 ip=World.position[i]; Quaternion iq=World.rotation[i]; V3 is=World.scale[i]; V3 va=quat_rot_v3(iq,(V3){lx0*is.x,0.0f,lz0*is.z}); V3 vb=quat_rot_v3(iq,(V3){lx1*is.x,0.0f,lz1*is.z}); int ecx=PosGetCellCoordX(ip.x+(va.x+vb.x)*0.5f),ecz=PosGetCellCoordZ(ip.z+(va.z+vb.z)*0.5f); if(!expl[(u32)ecz*64+(u32)ecx]){continue;}
        float ax=amWX(ip.x+va.x),ay=amWZ(ip.z+va.z),bx=amWX(ip.x+vb.x),by=amWZ(ip.z+vb.z); bool closed=(e->doorState==DoorState_Closed||e->doorState==DoorState_Closing); bool bulkhead=(e->index==499||e->index==509); if(closed){ amLine(ax,ay,bx,by,255,255,0,2); continue; }
        float dx=bx-ax,dy=by-ay,len=vsqrtf(dx*dx+dy*dy); if(len<0.000001f){continue;}
        dx/=len; dy/=len; float nx=-dy,ny=dx;/*span dir + perp*/ float cx=(ax+bx)*0.5f,cy=(ay+by)*0.5f; if(!bulkhead){amLine(cx-dx*1.0f,cy-dy*1.0f,cx+dx*1.0f,cy+dy*1.0f,255,255,0,2);}/*center*/ amLine(ax-nx*1.0f,ay-ny*1.0f,ax+nx*1.0f,ay+ny*1.0f,255,255,0,2);/*end ticks*/ amLine(bx-nx*1.0f,by-ny*1.0f,bx+nx*1.0f,by+ny*1.0f,255,255,0,2); if(bulkhead){amLine(cx-nx*1.0f,cy-ny*1.0f,cx+nx*1.0f,cy+ny*1.0f,255,255,0,2);}/*mid tick*/
    }
    for (u32 i=INSTS_1ST_IDX;i<World.instCount;++i) {/*overlays (nav-gated): NPC dots, red cameras/CPU nodes*/
        Entity* e=&World.instances[i]; if(!(e->entflags&EF_ACTIVE) || e->health<=0.0f){continue;} int ci=e->index; float x=amWX(World.position[i].x),y=amWZ(World.position[i].z); if(x<-12.0f||y<-12.0f||x>amDrawW+12.0f||y>amDrawH+12.0f){continue;} int ecx=PosGetCellCoordX(World.position[i].x),ecz=PosGetCellCoordZ(World.position[i].z); if(!expl[(u32)ecz*64+(u32)ecx]){continue;} 
        if(IdxIsNPC(ci)){NPCType t=npcTable[ci-419].type; if(nav<amNPCNeedVer(t)){continue;} u8 r,g,b; amNPCColor(t,&r,&g,&b); amFillRect((int)(x-5.0f),(int)(y-5.0f),(int)(x+5.0f),(int)(y+5.0f),r,g,b);}
        else if(ci==477){/*Cameras*/amFillRect((int)(x-1.5f),(int)(y-1.5f),(int)(x+1.5f),(int)(y+1.5f),255,0,0);}
        else if(ci==478||ci==479){/*Nodes*/amFillRect((int)(x-5.0f),(int)(y-5.0f),(int)(x+5.0f),(int)(y+5.0f),255,0,0);}
    }
    {V3 p=World.position[PLAYER1],f=World.instances[PLAYER1].forward; float x=amWX(p.x),y=amWZ(p.z); float fl=vsqrtf(f.x*f.x+f.z*f.z); float dx=0.0f,dy=-1.0f;
        if(fl>0.0001f){ dx=f.x/fl; dy=-f.z/fl; }/*screen: +x right, -z up*/
        float ox=dx*amMapScaleX/amPxPerUnit,oy=dy,ol=vsqrtf(ox*ox+oy*oy);
        if(ol>0.0001f){ox/=ol; oy/=ol;} else {ox=0.0f;oy=-1.0f;}
        float s=1.0f,nx=-oy,ny=ox; float tx=x+ox*8.0f*s,ty=y+oy*8.0f*s,lx=x-ox*5.0f*s+nx*5.0f*s,ly=y-oy*5.0f*s+ny*5.0f*s,rx=x-ox*5.0f*s-nx*5.0f*s,ry=y-oy*5.0f*s-ny*5.0f*s,ov=0.2f;/*hollow chevron: side edges overrun their tail verts by 20%, tip vertex unchanged*/ amLine(tx,ty,lx+(lx-tx)*ov,ly+(ly-ty)*ov,240,73,77,1); amLine(tx,ty,rx+(rx-tx)*ov,ry+(ry-ty)*ov,240,73,77,1); amLine(lx,ly,rx,ry,240,73,77,1);/*240,73,77 = 0.941f,0.286f,0.302f*/}
    if(nav>=3){V3 p=World.position[PLAYER1]; float x=amWX(p.x),y=amWZ(p.z); amCircle(x,y,14.08f*amPxPerUnit,0,200,200);/*small: cyan*/amCircle(x,y,20.48f*amPxPerUnit,0,200,200);/*large: cyan*/}
}
static bool amReveal() {
    u8 lev=World.curLev; V3 p=World.position[PLAYER1]; int pcx=PosGetCellCoordX(p.x),pcz=PosGetCellCoordZ(p.z); int r=(int)2; bool changed=false;
    for (int dz=-r;dz<=r;++dz) for (int dx=-r;dx<=r;++dx) {int cx=pcx+dx,cz=pcz+dz; if(cx<0||cx>63||cz<0||cz>63){continue;} float wx=World.worldMin_x[lev]+cx*CELLSZ-p.x,wz=World.worldMin_z[lev]+cz*CELLSZ-p.z; if(wx*wx+wz*wz>(30.0f * 30.0f)){continue;} u32 cell=(u32)cz*64+(u32)cx; if(!(gridCellStates[cell]&CELL_OPEN)){continue;} if(!World.automapExplored[lev][cell]){ World.automapExplored[lev][cell]=1; changed=true; } } return changed;
}
static u32 amDoorHash() {u32 h=0; for (u32 i=INSTS_1ST_IDX;i<World.instCount;++i) {Entity* e=&World.instances[i]; if(IdxIsDoor(e->index)){h=h*31u+(u32)(i*7u+(u32)e->doorState);}} return h;}
void AutomapNewGame() {mset(World.automapExplored,0,sizeof(World.automapExplored)); World.automapZoom=0; World.automapNextRaster=0.0; amBuiltLev=255; amBuiltPX=1000000000000000000000000000000.0f; amBuiltPZ=1000000000000000000000000000000.0f;}
void AutomapOnLoad() { amBuiltLev=255; World.automapNextRaster=0.0; amBuiltPX=1000000000000000000000000000000.0f; amBuiltPZ=1000000000000000000000000000000.0f; }/*force re-raster from loaded FoW*/
void AutomapTick() {
    if (World.menuActive) { return; } u8 lev=World.curLev; if(lev>=LEVEL_CYBERSPACE){return;} bool fowChanged=amReveal(); u32 dh=amDoorHash(); double now=get_time(); V3 tpp=World.position[PLAYER1]; V3 tpf=World.instances[PLAYER1].forward; bool full=World.Sys_UI.fullMapOpen[0] || World.Sys_UI.fullMapOpen[1]; u8 renderZoom=full ? 2 : World.automapZoom; int drawH=full?AM_FULL_H:AM_H; float wpp=((float)(16<<renderZoom)*CELLSZ)/(float)drawH;/*world/px*/ float mdx=tpp.x-amBuiltPX,mdz=tpp.z-amBuiltPZ,me=(0.25f*wpp)*(0.25f*wpp); float fdx=tpf.x-amBuiltFX,fdz=tpf.z-amBuiltFZ;
    bool moved=(mdx*mdx+mdz*mdz)>me || (fdx*fdx+fdz*fdz)>0.000001f; static u8 amBuiltZoom = 255;
    if(fowChanged || moved || dh!=amBuiltDoorHash || lev!=amBuiltLev || renderZoom!=amBuiltZoom || full!=amBuiltFull || now>=World.automapNextRaster){amRaster(renderZoom,full); amBuiltLev=lev; amBuiltDoorHash=dh; amBuiltZoom=renderZoom; amBuiltFull=full; amBuiltPX=tpp.x; amBuiltPZ=tpp.z; amBuiltFX=tpf.x; amBuiltFZ=tpf.z; int nav=amNavVer(); World.automapNextRaster=now+(nav>2?0:(nav>1?0.1:0.2)); glBindTexture(GL_TEXTURE_2D,full?amFullTexId:amTexId); glTexSubImage2D(GL_TEXTURE_2D,0,0,0,full?AM_FULL_W:AM_W,full?AM_FULL_H:AM_H,GL_RGBA,GL_UNSIGNED_BYTE,full?amFullPx:amPx);}
}
void AutomapBlitFullToUI() {
    if (Cheats.noHUD || World.curLev>=LEVEL_CYBERSPACE || (!World.Sys_UI.fullMapOpen[0] && !World.Sys_UI.fullMapOpen[1])) { return; }
    i16 x=(i16)((UI_W-AM_FULL_W)/2),y=(i16)((UI_H-AM_FULL_H)/2); glBindFramebuffer(GL_READ_FRAMEBUFFER,amFullFBO); glBindFramebuffer(GL_DRAW_FRAMEBUFFER,uiFBO); glBlitFramebuffer(0,AM_FULL_H,AM_FULL_W,0,x,768-(y+AM_FULL_H),x+AM_FULL_W,768-y,GL_COLOR_BUFFER_BIT,GL_LINEAR); glBindFramebuffer(GL_READ_FRAMEBUFFER,uiFBO); glBindFramebuffer(GL_DRAW_FRAMEBUFFER,uiFBO);
}
// UI
#include "ui.c"
// Lights
static const float INVSQRT2=0.70710678118f; Quaternion cubeQuats[6] = { {0.0f,INVSQRT2,0.0f,INVSQRT2}/*+X:Right*/, {0.0f,-INVSQRT2,0.0f,INVSQRT2}/*-X:Left*/, {-INVSQRT2,0.0f,0.0f,INVSQRT2}/*+Y:Up*/, {INVSQRT2,0.0f,0.0f,INVSQRT2}/*-Y:Down*/, {0.0f,0.0f,0.0f,1.0f}/*+Z:Forward*/, {0.0f,1.0f,0.0f,0.0f}/*-Z:Backward*/ };
static void ShadowRecomputeLight(u16 slot, V3 lightPos) { for (int j=0;j<6;++j) { mat4_lookat_from((float*)lightView[slot][j], &cubeQuats[j], lightPos); mul_mat4((float*)lightViewProj[slot][j], shadowmapsPerspectiveProjection, (float*)lightView[slot][j]); ExtractFrustumPlanes((float*)lightViewProj[slot][j], lightFrustumPlanes[slot][j]); } }
void UpdateLights() {
    for (u16 lightIdx=0;lightIdx<World.loadedLights;++lightIdx) { V3 lightPos = World.lightsNewPosition[lightIdx]; World.lights[lightIdx].pos = lightPos; }
    if (!World.paused && !World.menuActive) {
        for (int k=0;k<World.lanimCount;++k) { // Just lerps/flickers in intensity; lanims is a compact list of the level's animated lights
            LightAnimation* la=&World.lanims[k]; int i=World.lanimLightIdx[k];
            if (!(World.lights[i].lflags & LIGHTON)) { World.lights[i].intensity = 0.0f; continue; }
            if (la->lerpTime < (float)World.pauseRelativeTime) {
                World.lights[i].intensity = la->lerpUp ? World.lights[i].maxIntensity : World.lights[i].minIntensity;/*Pick target to lerp towards*/
                la->lerpUp = !la->lerpUp; la->currentStep++;
                if (la->currentStep >= la->numIntervalSteps) { la->currentStep = 0; /*Wrap and start over continuous looping*/ }
                la->lerpStepTime = la->intervalSteps[la->currentStep];
                la->lerpTime = (float)World.pauseRelativeTime + la->lerpStepTime;
                la->lerpStartTime = (float)World.pauseRelativeTime;
            } else if (World.lights[i].lflags & LERPON) {
                if (la->currentStep < la->numLerpSteps) {
                    if (la->stepIsLerping[la->currentStep]) {
                        la->lerpValue = ((float)World.pauseRelativeTime - la->lerpStartTime) / (la->lerpTime - la->lerpStartTime); /*percent towards goal time*/
                        float lerpVal = la->lerpUp ? la->lerpValue : (1.0f - la->lerpValue);
                        la->lerpValue = World.lights[i].minIntensity + ((World.lights[i].maxIntensity - World.lights[i].minIntensity) * lerpVal);
                        World.lights[i].intensity = la->lerpValue;
                    }
                }
            }
        }
    }
    glBindBuffer(GL_SSBO, lightsID); glBufferSubData(GL_SSBO, 0, World.loadedLights * sizeof(Light), World.lights);
    glUseProgram(voxelUpdateSP); glUniform3f(5, World.position[PLAYER1].x, World.position[PLAYER1].y, World.position[PLAYER1].z); glUniform3f(8, World.instances[PLAYER1].forward.x, World.instances[PLAYER1].forward.y, World.instances[PLAYER1].forward.z); glUniform1ui(9, Sys_Settings.Reflections); glUniform1ui(2, World.loadedLights);
    glDispatchCompute((VOXELS_X + 15) / 16, (VOXELS_Z + 15) / 16, 1); glMemoryBarrier(0x2000/*GL_SHADER_STORAGE_BARRIER_BIT*/);
}

// Shadowmapping
typedef struct {float depth; u16 index; } DepthSort;
INLINE bool EntNotVisible(u16 i, bool otherCondition) { Entity* e = &World.instances[i]; return e->texIndex >= texCnt || !(e->entflags & EF_ACTIVE) || e->index >= MAX_ENTITIES || (e->modelIndex >= MAX_MDLS && !Cheats.showPhys) || otherCondition; }
static u32 curIboDrawOfs = 0; static bool mdlFallbackLogged[MAX_MDLS];
static u16 MdlBind(u16 modelType) {
    if (modelType >= mdlsCnt || mdlVboOfs[modelType] == 0xFFFFFFFFu) { if (modelType < mdlsCnt && (modelVertexCounts[modelType] == 0)) { curIboDrawOfs = 0; return modelType; } if (modelType < MAX_MDLS && !mdlFallbackLogged[modelType]) { mdlFallbackLogged[modelType] = true; DualLogError("Model %u not resident!\n",modelType); } modelType = 178; if (modelType >= mdlsCnt || mdlVboOfs[modelType] == 0xFFFFFFFFu) { curIboDrawOfs = 0; return modelType; } }
    curIboDrawOfs = mdlIboOfs[modelType]; glBindVertexBuffer(0, modelVBO, (intptr_t)mdlVboOfs[modelType], VRT_ATT_SZ); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelIBO);
    return modelType;
}
INLINE u16 GetAndBindModel(u16 i, u16 currentModelType) { glUniform1ui(0,i); u16 modelType = (instanceIsLODArray[i] || Sys_Settings.ModelDetail < 1u) && World.instances[i].lodIndex < mdlsCnt ? World.instances[i].lodIndex : World.instances[i].modelIndex; if (currentModelType == modelType) return currentModelType; return MdlBind(modelType); }
typedef float __m256 __attribute__((__vector_size__(32), __may_alias__)); typedef long long __m256i __attribute__((__vector_size__(32), __may_alias__)); typedef float __v8sf __attribute__((__vector_size__(32), __may_alias__));
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
u16 shadowCasterIndices[SC_MAX], candidates[MAX_SHADOWMAPS]; static __attribute__((aligned(64))) float sc_posX[SC_MAX],sc_posY[SC_MAX],sc_posZ[SC_MAX],sc_radius[SC_MAX],sc_shadRadius[SC_MAX]; static u16 sc_origIdx[SC_MAX], shadowSlot[LIGHT_COUNT]; static u8 shadowFaces[LIGHT_COUNT]; static float shadowPosSum[LIGHT_COUNT]; static u32 shadowIdSum[LIGHT_COUNT];
static const u32 shadClearSentinel = 0xFFFFFFFFu;
static i8 shadowLevel=-1; u32 shadowNextSlot=0; static const i8 faceAxis[6] = {0,0,1,1,2,2}; static const float faceSign[6] = {1.f,-1.f,1.f,-1.f,1.f,-1.f};
static const __m256i faceBitVecs[6] = { (__m256i){0x100000001LL,0x100000001LL,0x100000001LL,0x100000001LL}, (__m256i){0x200000002LL,0x200000002LL,0x200000002LL,0x200000002LL}, (__m256i){0x400000004LL,0x400000004LL,0x400000004LL,0x400000004LL}, (__m256i){0x800000008LL,0x800000008LL,0x800000008LL,0x800000008LL}, (__m256i){0x1000000010LL,0x1000000010LL,0x1000000010LL,0x1000000010LL}, (__m256i){0x2000000020LL,0x2000000020LL,0x2000000020LL,0x2000000020LL}};
INLINE u8 GetCubemapFaceMask(V3 d, float r) {u8 m=0; float absX=vabs(d.x),absY=vabs(d.y),absZ=vabs(d.z); float maxAbsYZ = absY > absZ ? absY : absZ; float maxAbsXZ = absX > absZ ? absX : absZ; float maxAbsXY = absX > absY ? absX : absY; if (d.x+r > maxAbsYZ) m|=(1<<0); if (d.x-r < -maxAbsYZ) m|=(1<<1); if (d.y+r > maxAbsXZ) m|=(1<<2); if (d.y-r < -maxAbsXZ) m|=(1<<3); if (d.z+r > maxAbsXY) m|=(1<<4); if (d.z-r < -maxAbsXY) m|=(1<<5); return m;}
INLINE u8 FaceMaskFromViewer(V3 d, V3 fdir, float r) {
    float T = V3_dot(d,fdir) + r*(vabs(fdir.x)+vabs(fdir.y)+vabs(fdir.z)), rx = 2.0f*r*vabs(fdir.x), ry = 2.0f*r*vabs(fdir.y), rz = 2.0f*r*vabs(fdir.z); u8 m = 0;
    if (fdir.x >= 0.0f) { if (T > 0.0f) { m |=  1; } if (T > rx) { m |=  2; } } else { if (T > 0.0f) { m |=  2; } if (T > rx) { m |=  1; } }
    if (fdir.y >= 0.0f) { if (T > 0.0f) { m |=  4; } if (T > ry) { m |=  8; } } else { if (T > 0.0f) { m |=  8; } if (T > ry) { m |=  4; } }
    if (fdir.z >= 0.0f) { if (T > 0.0f) { m |= 16; } if (T > rz) { m |= 32; } } else { if (T > 0.0f) { m |= 32; } if (T > rz) { m |= 16; } }    return m;
}
typedef struct { u32 sortKey; u16 instanceIdx; u8 faceMask; } SortedMesh;
static u16 AddShadMsh(SortedMesh *sm, u16 i, float *ps, u32 *is, bool *moved, u8 fm) {
    V3 sc = World.scale[i]; 
    if (vabs(sc.x) + vabs(sc.y) + vabs(sc.z) >= 0.1f) { Entity* e = &World.instances[i]; u16 mi = ((instanceIsLODArray[i] || Sys_Settings.ModelDetail < 1u) && e->lodIndex < mdlsCnt ? e->lodIndex : e->modelIndex); *sm = (SortedMesh){((u32)mi << 16) | e->texIndex,i,fm}; *is += i + sm->sortKey; *ps += World.position[i].x + World.position[i].y + World.position[i].z; if ((e->entflags & EF_MOVING) && !IdxIsNPC(e->index)) { *moved = true; } return 1u; }
    return 0u;
}
__attribute__((hot, target("avx2,fma"))) void RenderShadowmaps(void) {
    double shadowStartTime = get_time(); V3 playerPos=World.position[PLAYER1], pf=World.instances[PLAYER1].forward; u16 numCandidates=0; i32 numCasters=0;
    for (u16 i = 0; i < World.loadedLights; ++i) {
        if (unlikely(!(World.lights[i].lflags & SHADON) || !(World.lights[i].lflags & LIGHTON))) { continue; }
        V3 lightPos = World.lights[i].pos; float intensity = World.lights[i].maxIntensity; if (unlikely(intensity < 0.1f)) { continue; }
        float range = World.lights[i].range; float luminosity = (intensity / (range * range)); if (luminosity < 0.008f && (range < 8.0f || intensity < 0.5f)) { continue; }
        u16 cellX=PosGetCellCoordX(lightPos.x), cellZ=PosGetCellCoordZ(lightPos.z);
        int lightCellIdx = (cellZ * WORLDX) + cellX;
        u8 r = vmax(vceil(range * (1.0f / CELLSZ)),2);
        bool camViewCandidate = false;
        for (u8 v=0;v<camViewCount;++v) { if (!camViews[v].visible) { continue; } V3 clp = V3_AsubB(lightPos, camViews[v].position); if (V3_dot(clp,clp) < ((range + 4.0f)*(range + 4.0f))) { camViewCandidate = true; break; } }
        bool inPVS = (gridCellStates[lightCellIdx] & CELL_VISIBLE); if (likely(!inPVS)) inPVS = NeighborhoodInPVS(cellX,cellZ,r); if (!inPVS && !camViewCandidate) { continue; }
        float dx = lightPos.x - playerPos.x, dy = lightPos.y - playerPos.y, dz = lightPos.z - playerPos.z;
        float distSqrdToPlayer = dx*dx + dy*dy + dz*dz;
        float dotResult = (dx*pf.x + dy*pf.y + dz*pf.z); if (dotResult < 0.0f && distSqrdToPlayer > (range * range) && !camViewCandidate) { continue; }
        candidates[numCandidates++] = i; if (numCandidates >= MAX_SHADOWMAPS) break;
    }
    for (u16 i=0;i<World.loadedLights;++i) { shadowmapIndirectionList[i] = (shadowSlot[i] != U16_MAX) ? (u32)shadowSlot[i] : LIGHT_COUNT; }
    if (numCandidates == 0) { glBindBuffer(GL_SSBO, shadowMapsIndirectionID); glBufferSubData(GL_SSBO, 0, World.loadedLights*sizeof(u32), shadowmapIndirectionList); shadowTime = get_time() - shadowStartTime; return; }
    u16 unslotted=0;
    for (u16 c=0;c<numCandidates;++c) { if (shadowSlot[candidates[c]]==U16_MAX) { ++unslotted; } }
    if (shadowNextSlot+unslotted > MAX_SHADOWMAPS) { mset(shadowSlot, 0xFF, sizeof(shadowSlot)); mset(shadowFaces, 0, sizeof(shadowFaces)); shadowNextSlot = 0; }
    for (u16 i=INSTS_1ST_IDX;i<World.instCount;++i) { if(EntNotVisible(i,(World.instances[i].entflags & EF_NO_SHADOWS)) || IdxIsNPC(World.instances[i].index)){continue;} shadowCasterIndices[numCasters++]=i; if(numCasters >= SC_MAX){break;} }
    const u16 numCastersAligned = numCasters & ~7u;
    for (i32 i=0;i<numCastersAligned;++i) { u16 j=shadowCasterIndices[i]; sc_origIdx[i]=j; sc_posX[i]=World.position[j].x; sc_posY[i]=World.position[j].y; sc_posZ[i]=World.position[j].z; sc_radius[i]=World.radius[j]; sc_shadRadius[i]=World.instances[j].shadRadius; }
    for (i32 i=numCastersAligned;i<numCasters;++i) { u16 j=shadowCasterIndices[i]; sc_posX[i]=World.position[j].x; sc_posY[i]=World.position[j].y; sc_posZ[i]=World.position[j].z; sc_radius[i]=World.radius[j]; sc_shadRadius[i]=World.instances[j].shadRadius; sc_origIdx[i]=j; }
    if (shadowLevel != (i8)World.curLev) { mset(shadowSlot, 0xFF, sizeof(shadowSlot)); mset(shadowFaces, 0, sizeof(shadowFaces)); mset(shadowPosSum, 0, sizeof(shadowPosSum)); mset(shadowIdSum, 0, sizeof(shadowIdSum)); shadowNextSlot = 0; shadowLevel = (i8)World.curLev; }
    glBindBuffer(GL_SSBO, shadowMapSSBO); glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE); 
    glUseProgram(shadowmapsSP);
    shadDrawCalls = 0U; u32 currentSortKey = 0xFFFFFFFF, currentTriCount = 0; u16 currentModelType = 0xFFFF, currentTexIndex = 0xFFFF; bool currentIsTransparent = false; SortedMesh localMeshes[SHADOW_NEARMESH_MAX]; V3 camFwds[64];
    for (u8 v=0;v<camViewCount;++v) { camFwds[v] = V3_Normalize(quat_rot_v3(camViews[v].rotation,(V3){0.0f,0.0f,1.0f})); } /*per-frame: sensaround slots re-orient every tick*/
    for (u16 c = 0; c < numCandidates; ++c) {
        u16 lightIdx = candidates[c];
        V3 lpos = World.lights[lightIdx].pos;
        u16 slot = shadowSlot[lightIdx];
        bool slotNew = false;
        if (slot == U16_MAX) { slot = (u16)shadowNextSlot++; shadowSlot[lightIdx] = slot; slotNew = true; }
        if (slotNew || (World.lights[lightIdx].lflags & LDIRTY)) { ShadowRecomputeLight(slot, lpos); flag_set(&World.lights[lightIdx].lflags, LDIRTY, false); }
        float effectiveRadius = vmin(World.lights[lightIdx].range,15.36f);
        V3 toLight = V3_AsubB(lpos, playerPos);
        const float addX = (pf.x >= 0.0f) ? effectiveRadius : -effectiveRadius;
        const float addY = (pf.y >= 0.0f) ? effectiveRadius : -effectiveRadius;
        const float addZ = (pf.z >= 0.0f) ? effectiveRadius : -effectiveRadius;
        __attribute__((aligned(32))) float cX[8],cY[8],cZ[8];
        for (int f = 0; f < 6; ++f) { const int axis = faceAxis[f]; const float sign = faceSign[f]; float x = toLight.x + addX, y = toLight.y + addY, z = toLight.z + addZ; if (axis == 0) x = toLight.x + sign * effectiveRadius; else if (axis == 1) y = toLight.y + sign * effectiveRadius; else z = toLight.z + sign * effectiveRadius; cX[f] = x; cY[f] = y; cZ[f] = z; }
        cX[6] = cY[6] = cZ[6] = 0.0f; cX[7] = cY[7] = cZ[7] = 0.0f;
        const __m256 cx = _mm256_load_ps(cX); const __m256 cy = _mm256_load_ps(cY); const __m256 cz = _mm256_load_ps(cZ);
        const __m256 fx = _mm256_set1_ps(pf.x); const __m256 fy = _mm256_set1_ps(pf.y); const __m256 fz = _mm256_set1_ps(pf.z);
        const __m256 dot = _mm256_fmadd_ps(cx, fx, _mm256_fmadd_ps(cy, fy, _mm256_mul_ps(cz, fz)));
        const __m256 visible = _mm256_cmp_ps(dot,_mm256_setzero_ps(),0x1E);
        u8 faceMask = (u8)(_mm256_movemask_ps(visible) & 0x3F);
        for (u8 face = 0; face < 6; ++face) { if (!(faceMask & (1u << face))) { if (SphereInFrustum(lightFrustumPlanes[slot][face], playerPos, 0.48f)) {faceMask |= (u8)(1u << face);} else { for (u8 v=0;v<camViewCount;++v) { if (camViews[v].visible && SphereInFrustum(lightFrustumPlanes[slot][face], camViews[v].position, 0.48f)) { faceMask |= (u8)(1u << face); break; } } } } }
        for (u8 v=0;v<camViewCount;++v) { if (camViews[v].visible) { faceMask |= FaceMaskFromViewer(V3_AsubB(lpos, camViews[v].position), camFwds[v], effectiveRadius); } } if (faceMask == 0) { if (slotNew) { shadowSlot[lightIdx] = U16_MAX; shadowNextSlot--; } continue; }
        const __m256 lposX = _mm256_set1_ps(lpos.x); const __m256 lposY = _mm256_set1_ps(lpos.y); const __m256 lposZ = _mm256_set1_ps(lpos.z); const __m256 effR  = _mm256_set1_ps(effectiveRadius); const __m256 signMask = _mm256_set1_ps(-0.0f); u16 nearbyMeshCount = 0; i32 k = 0; bool anyMoved=false; float posSum=0.0f; u32 idSum=0;
        for (; k < numCastersAligned; k += 8) {
            const __m256 px = _mm256_load_ps(&sc_posX[k]); const __m256 py = _mm256_load_ps(&sc_posY[k]); const __m256 pz = _mm256_load_ps(&sc_posZ[k]);
            const __m256 r  = _mm256_load_ps(&sc_radius[k]); const __m256 sr = _mm256_load_ps(&sc_shadRadius[k]);
            const __m256 dx = _mm256_sub_ps(px,lposX); const __m256 dy = _mm256_sub_ps(py,lposY); const __m256 dz = _mm256_sub_ps(pz,lposZ);
            const __m256 distSq = _mm256_fmadd_ps(dx,dx,_mm256_fmadd_ps(dy,dy,_mm256_mul_ps(dz,dz)));
            const __m256 radSum   = _mm256_add_ps(effR,r); const __m256 radSumSq = _mm256_mul_ps(radSum,radSum);
            const __m256 inRange  = _mm256_cmp_ps(distSq,radSumSq,0x11);
            const __m256 absX = _mm256_andnot_ps(signMask,dx); const __m256 absY = _mm256_andnot_ps(signMask,dy); const __m256 absZ = _mm256_andnot_ps(signMask,dz);
            const __m256 maxAbsYZ = _mm256_max_ps(absY,absZ); const __m256 maxAbsXZ = _mm256_max_ps(absX,absZ); const __m256 maxAbsXY = _mm256_max_ps(absX,absY); // Face Math: A > |B| && A > |C| <==> A > max(|B|, |C|)
            const __m256 negMaxAbsYZ = _mm256_or_ps(maxAbsYZ,signMask); const __m256 negMaxAbsXZ = _mm256_or_ps(maxAbsXZ,signMask); const __m256 negMaxAbsXY = _mm256_or_ps(maxAbsXY,signMask);
            const __m256 dxp = _mm256_add_ps(dx,sr); const __m256 posXface = _mm256_cmp_ps(dxp,maxAbsYZ,0x1E);
            const __m256 dxm = _mm256_sub_ps(dx,sr); const __m256 negXface = _mm256_cmp_ps(dxm,negMaxAbsYZ,0x11);
            const __m256 dyp = _mm256_add_ps(dy,sr); const __m256 posYface = _mm256_cmp_ps(dyp,maxAbsXZ,0x1E);
            const __m256 dym = _mm256_sub_ps(dy,sr); const __m256 negYface = _mm256_cmp_ps(dym,negMaxAbsXZ,0x11);
            const __m256 dzp = _mm256_add_ps(dz,sr); const __m256 posZface = _mm256_cmp_ps(dzp,maxAbsXY,0x1E);
            const __m256 dzm = _mm256_sub_ps(dz,sr); const __m256 negZface = _mm256_cmp_ps(dzm,negMaxAbsXY,0x11);
            const __m256 anyFace = _mm256_or_ps(_mm256_or_ps(posXface, negXface), _mm256_or_ps(_mm256_or_ps(posYface, negYface), _mm256_or_ps(posZface, negZface)));
            __attribute__((aligned(32))) u32 laneFaceView[8];
            const __m256 faceTwo = _mm256_or_ps(_mm256_and_ps(posXface,_mm256_castsi256_ps(faceBitVecs[0])), _mm256_and_ps(negXface,_mm256_castsi256_ps(faceBitVecs[1])));
            const __m256 faceSecond = _mm256_or_ps(_mm256_and_ps(posYface,_mm256_castsi256_ps(faceBitVecs[2])), _mm256_and_ps(negYface,_mm256_castsi256_ps(faceBitVecs[3])));
            const __m256 faceThird = _mm256_or_ps(_mm256_and_ps(posZface,_mm256_castsi256_ps(faceBitVecs[4])), _mm256_and_ps(negZface,_mm256_castsi256_ps(faceBitVecs[5])));
            _mm256_store_ps((float*)laneFaceView, _mm256_or_ps(_mm256_or_ps(faceTwo, faceSecond), faceThird));
            const __m256 valid = _mm256_and_ps(inRange, anyFace);
            unsigned mask = (unsigned)_mm256_movemask_ps(valid);
            while (mask) { int bit = __builtin_ctz(mask); mask &= mask - 1; if (unlikely(nearbyMeshCount >= SHADOW_NEARMESH_MAX)) { DualLogWarn("Shadowmapping ran out of nearMeshes at %u!  Skipping some renderables for light %u!\n", SHADOW_NEARMESH_MAX, lightIdx); k = numCastersAligned; break; } u16 instIdx = sc_origIdx[k + bit]; nearbyMeshCount += AddShadMsh(&localMeshes[nearbyMeshCount],instIdx,&posSum,&idSum,&anyMoved,(u8)laneFaceView[bit]); }
        }
        if (nearbyMeshCount < SHADOW_NEARMESH_MAX) {
            for (; k < numCasters; ++k) {
                V3 d = V3_AsubB(World.position[sc_origIdx[k]], lpos); float distToLightSqrd = V3_dot(d,d); float radSum = (effectiveRadius + World.radius[sc_origIdx[k]]); if (distToLightSqrd >= radSum * radSum) { continue; } u8 faceMaskScalar = GetCubemapFaceMask(d, World.instances[sc_origIdx[k]].shadRadius); if (faceMaskScalar == 0) { continue; }
                u16 instIdx = sc_origIdx[k]; nearbyMeshCount += AddShadMsh(&localMeshes[nearbyMeshCount],instIdx,&posSum,&idSum,&anyMoved,faceMaskScalar); if (nearbyMeshCount >= SHADOW_NEARMESH_MAX) { DualLogWarn("Shadowmapping ran out of nearMeshes at %u!  Skipping some renderables for light %u!\n", SHADOW_NEARMESH_MAX, lightIdx); break; }
            }
        }
        if (nearbyMeshCount == 0) { shadowmapIndirectionList[lightIdx] = LIGHT_COUNT; if (slotNew) { shadowSlot[lightIdx] = U16_MAX; shadowNextSlot--; } continue; }
        bool contentDirty = true;//(anyMoved || posSum != shadowPosSum[lightIdx] || idSum != shadowIdSum[lightIdx] || slot == U16_MAX);
        u8 needFaces = (u8)(faceMask & ~shadowFaces[lightIdx]);
        if (contentDirty || needFaces) {
            for (u16 j=1;j<nearbyMeshCount;++j) { SortedMesh key = localMeshes[j]; int sk = (int)j - 1; while (sk >= 0 && localMeshes[sk].sortKey > key.sortKey) { localMeshes[sk + 1] = localMeshes[sk]; --sk; } localMeshes[sk + 1] = key; }
            u32 slotOff = (u32)slot * (SHADOW_MAP_SIZE * SHADOW_MAP_SIZE * 6);
            glUniform3f(3, lpos.x, lpos.y, lpos.z);
            u8 renderFaces = contentDirty ? faceMask : needFaces; // Content change invalidates all faces, but viewpoint change only needs relevant faces.
            #pragma GCC unroll 6
            for (u8 face = 0; face < 6; ++face) {
                if (!(renderFaces & (1u << face))) {continue;}
                if (likely(glClearBufferSubData)) { glClearBufferSubData(GL_SSBO, GL_R32UI, (slotOff + face*SHADOW_MAP_SIZE*SHADOW_MAP_SIZE) * (intptr_t)4, (SHADOW_MAP_SIZE*SHADOW_MAP_SIZE) * (intptr_t)4, GL_RED_INTEGER, GL_UNSIGNED_INT, &shadClearSentinel); }
                else { static u32 faceBlock[SHADOW_MAP_SIZE*SHADOW_MAP_SIZE]; if (faceBlock[0] != 0xFFFFFFFFu) { for (u32 t=0;t<(u32)(SHADOW_MAP_SIZE*SHADOW_MAP_SIZE);++t) { faceBlock[t] = 0xFFFFFFFFu; } } glBufferSubData(GL_SSBO, (slotOff + face*SHADOW_MAP_SIZE*SHADOW_MAP_SIZE) * 4, SHADOW_MAP_SIZE*SHADOW_MAP_SIZE * 4, faceBlock); } /*ancient-driver fallback path: same all-0xFF block the old shadClearFace array held*/
                const u8 faceBit = (u8)(1u << face);
                u16 faceCasters = 0;
                for (u16 j=0;j<nearbyMeshCount;++j) { if (localMeshes[j].faceMask & faceBit) { ++faceCasters; } }
                if (faceCasters == 0) { continue; }
                glUniform1ui(2, face);
                glUniformMatrix4fv(1, 1, GL_FALSE, (float*)lightViewProj[slot][face]);
                glUniform1ui(7, slotOff + (face * SHADOW_MAP_SIZE * SHADOW_MAP_SIZE));
                for (u16 j=0;j<nearbyMeshCount;++j) {
                    if (!(localMeshes[j].faceMask & faceBit)) { continue; }
                    u16 instIdx = localMeshes[j].instanceIdx;
                    u32 sortKey = localMeshes[j].sortKey;
                    glUniform1ui(0,instIdx);
                    if (currentSortKey != sortKey) {
                        currentSortKey = sortKey;
                        u16 modelType = (u16)(sortKey >> 16), texIndex = (u16)(sortKey & 0xFFFF);
                        if (currentModelType != modelType) { currentModelType = MdlBind(modelType); currentTriCount = modelTriangleCounts[currentModelType] * 3; }
                        if (currentTexIndex != texIndex) { currentTexIndex = texIndex; glUniform1ui(6, texIndex); bool texIsTransparent = transparentTexture[texIndex]; if (currentIsTransparent != texIsTransparent) { currentIsTransparent = texIsTransparent; glUniform1ui(8, (u32)currentIsTransparent); } }
                    }
                    glDrawElements(0x4, currentTriCount, GL_UNSIGNED_SHORT, (void*)(uintptr_t)curIboDrawOfs); drawCalls++; shadDrawCalls++; vertsRendered += currentTriCount;
                }
            }
            shadowPosSum[lightIdx]=posSum;
            shadowIdSum[lightIdx]=idSum;
            shadowFaces[lightIdx] = contentDirty ? faceMask : (u8)(shadowFaces[lightIdx] | faceMask);
        } shadowmapIndirectionList[lightIdx]=slot;
    } glViewport(0, 0, Sys_Settings.ScreenWidth, Sys_Settings.ScreenHeight); glBindBuffer(GL_SSBO, shadowMapsIndirectionID); glBufferSubData(GL_SSBO, 0, World.loadedLights * sizeof(u32), shadowmapIndirectionList); shadowTime = get_time() - shadowStartTime;
}
float sunYawCur = 0.0f;
V3 SunDirection(void) { double yawRaw = World.pauseRelativeTime * 0.1 * skyRotateSpeeds[Cheats.dizzyLevel]; sunYawCur = (float)(yawRaw - 6.283185307179586 * (double)((long long)(yawRaw * 0.15915494309189535))); /*1/TAU = 0.15915494309189535; trunc floor for the always-positive monotonic phase*/ float c = vcosf(sunYawCur), s = vsinf(sunYawCur); V3 b = {0.0f, 0.1f, 0.99498743f}; return V3_Normalize((V3){b.x*c - b.z*s, b.y, b.x*s + b.z*c}); }
void InitSunShadowmap(void) {
    i32 prevUnit=0; glGetIntegerv(GL_ACTIVE_TEXTURE,&prevUnit); glActiveTexture(GL_TEXTURE10);
    glGenTextures(1,&sunShadowTex); glBindTexture(GL_TEXTURE_2D,sunShadowTex);
    glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT24,SUN_SHADOW_SIZE,SUN_SHADOW_SIZE,0,GL_DEPTH_COMPONENT,GL_UNSIGNED_INT,NULL); glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_BORDER); glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_BORDER); const float sunBorderCol[4]={1.0f,1.0f,1.0f,1.0f}; glTexParameterfv(GL_TEXTURE_2D,GL_TEXTURE_BORDER_COLOR,sunBorderCol);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_COMPARE_MODE,GL_COMPARE_REF_TO_TEXTURE); glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_COMPARE_FUNC,GL_LEQUAL);
    glGenFramebuffers(1,&sunShadowFBO); glBindFramebuffer(GL_FRAMEBUFFER,sunShadowFBO); glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,sunShadowTex,0); glDrawBuffers(0,NULL); glActiveTexture((u32)prevUnit); glBindFramebuffer(GL_FRAMEBUFFER,gBufferFBO);
}
void RenderSunShadowmap(void) {    
    V3 sunDir=SunDirection(); sunDirection=sunDir;
    V3 target=World.position[PLAYER1];
    V3 fwd={-sunDir.x,-sunDir.y,-sunDir.z};
    V3 right=V3_Normalize(V3_Cross(fwd,(V3){0.0f,0.0f,1.0f}));
    V3 up=V3_Cross(right,fwd);
    const float S=64.0f, eyeDist=S*2.0f;
    V3 eye={target.x+sunDir.x*eyeDist,target.y+sunDir.y*eyeDist,target.z+sunDir.z*eyeDist};
    float view[16]={right.x,up.x,sunDir.x,0.0f, right.y,up.y,sunDir.y,0.0f, right.z,up.z,sunDir.z,0.0f, -(right.x*eye.x+right.y*eye.y+right.z*eye.z),-(up.x*eye.x+up.y*eye.y+up.z*eye.z),-(sunDir.x*eye.x+sunDir.y*eye.y+sunDir.z*eye.z),1.0f};
    float ex=right.x*eye.x+right.y*eye.y+right.z*eye.z, ey=up.x*eye.x+up.y*eye.y+up.z*eye.z;
    view[12]=-ex; view[13]=-ey; const float znear=S*0.5f, zfar=S*4.0f; float ortho[16]={1.0f/S,0.0f,0.0f,0.0f, 0.0f,1.0f/S,0.0f,0.0f, 0.0f,0.0f,-2.0f/(zfar-znear),0.0f, 0.0f,0.0f,-(zfar+znear)/(zfar-znear),1.0f};
    mul_mat4(sunShadowMatrix,ortho,view); ExtractFrustumPlanes(sunShadowMatrix,sunShadowFrustum);
    glBindFramebuffer(GL_FRAMEBUFFER,sunShadowFBO); glViewport(0,0,SUN_SHADOW_SIZE,SUN_SHADOW_SIZE); glClear(GL_DEPTH_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST); glDepthMask(GL_TRUE); glDepthFunc(0x201/*GL_LESS*/); glDisable(GL_BLEND);
    glUseProgram(depthPrepassSP); glUniformMatrix4fv(2,1,GL_FALSE,sunShadowMatrix);
    u16 currentModelType=U16_MAX,currentTexIndex=U16_MAX; u32 currentSortKey=U32_MAX, currentTriCount=0; bool useDetail=Sys_Settings.ModelDetail;
    for (u16 i=INSTS_1ST_IDX;i<World.instCount;++i) {
        if (i == World.weaponVModelIndex) continue;
        Entity* e=&World.instances[i]; if (EntNotVisible(i,(e->entflags & EF_NO_SHADOWS))) continue;
        float radius=(e->modelIndex < MAX_MDLS ? modelBounds[e->modelIndex] : 0.0f) * 2.0f * vmax(vmax(World.scale[i].x,World.scale[i].y),World.scale[i].z); if (!SphereInFrustum(sunShadowFrustum,World.position[i],radius)) continue;
        u16 modelType=(instanceIsLODArray[i]||useDetail<1u)&&e->lodIndex<mdlsCnt?e->lodIndex:e->modelIndex;
        u32 sortKey=((u32)modelType<<16)|e->texIndex;
        glUniform1ui(0,i);
        if (sortKey != currentSortKey) { currentSortKey=sortKey; if (modelType != currentModelType) { currentModelType=MdlBind(modelType); currentTriCount=modelTriangleCounts[currentModelType]*3; } if (e->texIndex != currentTexIndex) { currentTexIndex=e->texIndex; glUniform1ui(3,(u32)currentTexIndex); } }
        glDrawElements(0x4/*GL_TRIANGLES*/,currentTriCount,GL_UNSIGNED_SHORT,(void*)(uintptr_t)curIboDrawOfs); drawCalls++; shadDrawCalls++; vertsRendered+=currentTriCount;
    }
    glEnable(GL_CULL_FACE); glBindFramebuffer(GL_FRAMEBUFFER,gBufferFBO); glViewport(0,0,Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight);
}
static DepthSort visibleInstances[INSTANCE_COUNT];
__attribute__((pure)) i32 dsort(const void* a, const void* b) { float da = ((const DepthSort*)a)->depth; float db = ((const DepthSort*)b)->depth; return (db > da) - (db < da); }
__attribute__((pure)) i32 dsortInv(const void* a, const void* b) { float da = ((const DepthSort*)a)->depth; float db = ((const DepthSort*)b)->depth; return (da > db) - (da < db); }
void DrawEntity(Entity* e, u16 i, u16 constIndex, u16 tex, u16* curN, u16* curT, u16* curG, u16* curS, u16* curM, float* curV, bool grayscaleEnabled) {
    u16 glow = e->glowIndex, norm = e->normIndex, spec = e->specIndex;
    if (constIndex == 505 && e->doorState == DoorState_Closed) { // Draw X door lasers
        if (V3_SqDist(World.position[i], World.position[PLAYER1]) <= 2500.0f) {
            AnimationClip closing = mdlAnimClips[e->animationNum][A_CLOSING]; u16 m = (u16)(closing.frameStartModelIndex + (u16)(closing.frameEnd - closing.frameStart)); V3 mn = modelMin[m], mx = modelMax[m];
            V3 ctr = (V3){(mn.x+mx.x)*0.5f * World.scale[i].x, (mn.y+mx.y)*0.5f * World.scale[i].y, (mn.z+mx.z)*0.5f * World.scale[i].z};
            V3 he  = (V3){(mx.x-mn.x)*0.5f * World.scale[i].x, (mx.y-mn.y)*0.5f * World.scale[i].y, (mx.z-mn.z)*0.5f * World.scale[i].z};
            V3 c = V3Add(World.position[i], quat_rot_v3(World.rotation[i], ctr)); bool wideX = he.x >= he.z; V3 right = quat_rot_v3(World.rotation[i], wideX ? (V3){1.0f,0.0f,0.0f} : (V3){0.0f,0.0f,1.0f}); V3 up = quat_rot_v3(World.rotation[i], (V3){0.0f,1.0f,0.0f}); float hw = (wideX ? he.x : he.z), hh = he.y;
            if (hw > 0.0f && hh > 0.0f) {
                float shrink = 0.65f;
                V3 lo = V3Add(c, V3Add(V3_ScaleByF(right, -hw * shrink), V3_ScaleByF(up, -hh * shrink))); V3 hi = V3Add(c, V3Add(V3_ScaleByF(right,  hw * shrink), V3_ScaleByF(up,  hh * shrink))); V3 lf = V3Add(lo, V3_ScaleByF(up, 2.56f * shrink)); V3 rf = V3_AsubB(hi, V3_ScaleByF(up, 2.56f * shrink)); V3 face = V3_Normalize(V3_Cross(right, up)), toPlayer = V3_Normalize(V3_AsubB(World.position[PLAYER1], c));
                const Color band[3] = { {1.0f, 0.82f, 0.08f, 1.0f}, {1.0f, 0.42f, 0.04f, 1.0f}, {1.0f, 0.82f, 0.08f, 1.0f} }; const float spread[3] = {-0.01125f, 0.0f, 0.01125f};                
                for (int pass = 0; pass < 2; ++pass) { V3 a = pass ? lf : lo, b = pass ? rf : hi; V3 dir = V3_Normalize(V3_AsubB(b, a)); V3 perp = V3_Normalize(V3_Cross(dir, face)); for (int k = 0; k < 3; ++k) { V3 o = V3_ScaleByF(perp, spread[k]); if (k == 1) o = V3Add(o,V3_ScaleByF(toPlayer,0.01f)); /*Small shift to eliminate z fighting*/ DrawCrossedLine(V3Add(a,o),V3Add(b,o),perp,0.0075f,band[k]); } }
            }
        }
    }
    if (Cheats.showPhys) { if (World.col[i] == CT_BOX) { DrawBoxColliderColored(i,ColliderColor(i)); } else if (World.col[i] == CT_SPH) { DrawSphereCollider(i); } else if (World.col[i] == CT_CVX || World.col[i] == CT_MSH) { DrawMeshCollider(i); } else if (World.col[i] == CT_CAP) { DrawCapsuleCollider(i); }    DrawAngularVelocity(i); }
    if (e->modelIndex < MAX_MDLS) {
        float redTint = 0.0f, rimStrength = 0.0f; if (e->deathAnimationActive && e->index == 433) { float elapsed = vmax((float)(World.pauseRelativeTime - (double)e->deathAnimationStart),0.0f); u32 tick = (u32)(elapsed / 0.05f); redTint = elapsed < 0.35f ? 0.5f * vmin((float)tick, 6.0f) : 0.0f; rimStrength = vmax(0.0f, 1.0f - ((float)tick * 15.0f / 255.0f)); } glUniform2f(33, redTint, rimStrength); // Set Hopper death glow effect strength.
        glUniform1ui(17,tex==316?1u:0u);
        glUniform1ui(25,constIndex);
        float vol = (constIndex == 21 || constIndex == 22) ? vmax(0.02f, cyberWallFade[i]) : 0.0f; if (*curV != vol) { *curV = vol; glUniform1f(27, vol); }
        glUniform1ui(13,(tex==36||tex==887) ? 1u : 0u);
        if (grayscaleEnabled) { float npcHeat = IdxIsNPC(constIndex) ? ((constIndex==419 || constIndex==422 || constIndex==424 || constIndex==429 || constIndex==430 || constIndex==431 || constIndex==433 || constIndex==437 || constIndex==438 || constIndex==441) ? 1.5f : 4.0f) : 0.0f; glUniform1f(9,npcHeat); }
        glUniform1ui(30,e->camView < camViewCount ? 1u : 0u);
        if(e->camView < camViewCount) { glActiveTexture(GL_TEXTURE6); glBindTexture(GL_TEXTURE_2D,camViewTextures[e->camView]); glUniform2ui(28,camViews[e->camView].width,camViews[e->camView].height); }
        if((*curN) != (norm)) { *curN=norm; glUniform1ui( 1,(u32)norm); }
        if((*curT) != ( tex)) { *curT= tex; glUniform1ui(18,(u32)tex ); }
        if((*curG) != (glow) || glow == 0) { *curG=glow; glUniform1ui(19,(u32)glow); }
        if((*curS) != (spec)) { *curS=spec; glUniform1ui(20,(u32)spec); }
        *curM=GetAndBindModel(i,*curM);
        u32 vc=modelTriangleCounts[*curM]*3;
        glDrawElements(0x4,vc,GL_UNSIGNED_SHORT,(void*)(uintptr_t)curIboDrawOfs); drawCalls++; vertsRendered += vc;
    }
}
void mat4_inverse(const float* m, float* out) {
    float inv[16]; float s0 = m[0]*m[5]   - m[4]*m[1],   s1 = m[0]*m[6]   - m[4]*m[2],   s2 = m[0]*m[7]   - m[4]*m[3], s3 = m[1]*m[6]   - m[5]*m[2],   s4 = m[1]*m[7]   - m[5]*m[3],   s5 = m[2]*m[7]   - m[6]*m[3], c0 = m[8]*m[13]  - m[12]*m[9],  c1 = m[8]*m[14]  - m[12]*m[10], c2 = m[8]*m[15]  - m[12]*m[11], c3 = m[9]*m[14]  - m[13]*m[10], c4 = m[9]*m[15]  - m[13]*m[11], c5 = m[10]*m[15] - m[14]*m[11];
    float det = s0*c5 - s1*c4 + s2*c3 + s3*c2 - s4*c1 + s5*c0; if (det == 0.0f) { for (int i = 0; i < 16; ++i) out[i] = (i % 5 == 0) ? 1.0f : 0.0f; return; }
    float id = 1.0f / det;
    inv[0]  = ( m[5]*c5  - m[6]*c4   + m[7]*c3 ) * id;  inv[1]  = (-m[1]*c5  + m[2]*c4   - m[3]*c3 ) * id; inv[2]  = ( m[13]*s5 - m[14]*s4  + m[15]*s3) * id;  inv[3]  = (-m[9]*s5  + m[10]*s4  - m[11]*s3) * id; inv[4]  = (-m[4]*c5  + m[6]*c2   - m[7]*c1 ) * id;  inv[5]  = ( m[0]*c5  - m[2]*c2   + m[3]*c1 ) * id; inv[6]  = (-m[12]*s5 + m[14]*s2  - m[15]*s1) * id;  inv[7]  = ( m[8]*s5  - m[10]*s2  + m[11]*s1) * id;
    inv[8]  = ( m[4]*c4  - m[5]*c2   + m[7]*c0 ) * id;  inv[9]  = (-m[0]*c4  + m[1]*c2   - m[3]*c0 ) * id; inv[10] = ( m[12]*s4 - m[13]*s2  + m[15]*s0) * id;  inv[11] = (-m[8]*s4  + m[9]*s2   - m[11]*s0) * id; inv[12] = (-m[4]*c3  + m[5]*c1   - m[6]*c0 ) * id;  inv[13] = ( m[0]*c3  - m[1]*c1   + m[2]*c0 ) * id; inv[14] = (-m[12]*s3 + m[13]*s1  - m[14]*s0) * id;  inv[15] = ( m[8]*s3  - m[9]*s1   + m[10]*s0) * id;
    for (int i = 0; i < 16; ++i) out[i] = inv[i];
}
void PSys_Render(float*,V3,V3,V3,V3,u32,float,float,float,float);
static void ClearAll() { glClearColor(0.0f,0.0f,0.0f,0.0f); glBindFramebuffer(GL_FRAMEBUFFER,gBufferFBO); glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); glBindFramebuffer(GL_FRAMEBUFFER,uiFBO); glClear(GL_COLOR_BUFFER_BIT); glBindFramebuffer(GL_FRAMEBUFFER,amFBO); glClear(GL_COLOR_BUFFER_BIT); glBindFramebuffer(GL_FRAMEBUFFER,amFullFBO); glClear(GL_COLOR_BUFFER_BIT); glBindFramebuffer(GL_FRAMEBUFFER,biomFBO); glClear(GL_COLOR_BUFFER_BIT); }
V3 debugWepOffset = {0, 0, 0}; // debug weapon view offset
static __attribute__((hot)) void Render(bool camView, u8 camViewIdx) {
    u16 swidth, sheight; float sfov, snear, sfar;
    if (camView) { CamView* cv = &camViews[camViewIdx]; swidth  = cv->width;                sheight = cv->height;                sfov = (float)cv->fov;          snear = cv->near; sfar = cv->far; }
    else         {                                      swidth  = Sys_Settings.ScreenWidth; sheight = Sys_Settings.ScreenHeight; sfov = (float)Sys_Settings.FOV; snear = 0.02f;    sfar = World.farPlane[World.curLev]; }
    float shakeOffset = (World.shakeFinished > World.pauseRelativeTime) ? (shakeAmp * vcosf((float)(World.pauseRelativeTime * 20.0f))) * (World.shakeFinished - World.pauseRelativeTime) : 0.0f;
    Quaternion cameraRotation=World.rotation[PLAYER1]; if (shakeOffset!=0.0f && World.curLev!=LEVEL_CYBERSPACE) quat_from_yaw_pitch_roll(&cameraRotation,World.cam_yaw+shakeOffset,World.cam_pitch+shakeOffset*0.5f,World.cam_roll);
    V3 playerPos = World.position[PLAYER1]; float px = playerPos.x, py = playerPos.y, pz = playerPos.z, aspect3D = (float)swidth / (float)sheight, view[16], viewProj[16], invViewRot[9], invViewProj[16]; // Camera projection
    { float f = vcot(sfov * PI / 360.0f); float* m = rasterPerspectiveProjection; m[0]=f / aspect3D; m[1]=0.0f; m[2]=0.0f; m[3]=0.0f; m[4]=0.0f; m[5]=f; m[6]=0.0f; m[7]=0.0f; m[8]=0.0f; m[9]=0.0f; m[10]= -(sfar + snear) / (sfar - snear); m[11]=-1.0f; m[12]=0.0f; m[13]=0.0f; m[14]=-2.0f*sfar*snear / (sfar - snear); m[15]=0.0f;
    mat4_lookat_from(view,&cameraRotation,World.position[PLAYER1]); mul_mat4(viewProj,rasterPerspectiveProjection,view); invViewRot[0]=view[0]; invViewRot[1]=view[4]; invViewRot[2]=view[8]; invViewRot[3]=view[1]; invViewRot[4]=view[5]; invViewRot[5]=view[9]; invViewRot[6]=view[2]; invViewRot[7]=view[6]; invViewRot[8]=view[10]; mat4_inverse(viewProj,invViewProj); }
    ExtractFrustumPlanes(viewProj, playerFrustumPlanes);
    glBindVertexArray(chunkVAO);/*Common vao for RenderDynamicShadowmaps and Rasterized Geometry*/
    glEnable(GL_DEPTH_TEST);
    bool sunActive = !camView && likely(Sys_Settings.Shadows > 0u) && SkySunIsVisible();
    if (sunActive && sunShadowFBO == 0u) { InitSunShadowmap(); }
    glBeginQuery(0x88BF/*GL_TIME_ELAPSED*/, gpuQ[gpuQFrame][0]);
    if (likely(Sys_Settings.Shadows > 0u) && !camView) { glDisable(GL_DEPTH_TEST); RenderShadowmaps(); glEnable(GL_DEPTH_TEST); if (sunActive) { RenderSunShadowmap(); } }
    glEndQuery(0x88BF/*GL_TIME_ELAPSED*/);
    glBeginQuery(0x88BF/*GL_TIME_ELAPSED*/, gpuQ[gpuQFrame][1]);
    double rendStart = get_time();
    UpdateLights(); // This is where the voxels get updated!
    if (unlikely(clearRenderTargetsOnNextFrame)) { clearRenderTargetsOnNextFrame = false; ClearAll(); }
    glViewport(0, 0, swidth, sheight);
    glBindFramebuffer(GL_FRAMEBUFFER,gBufferFBO);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);/*Opaques*/
    u16 currentTexIndex = U16_MAX, currentNormIndex = U16_MAX, currentGlowIndex = U16_MAX, currentSpecIndex = U16_MAX, currentModelType = U16_MAX, opaqueCount = 0;
    bool skyVisible = (gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX);
    static DepthSort tmpTransparent[INSTANCE_COUNT];
    u16 tcnt = 0;
    for (u16 i = INSTS_1ST_IDX; i < World.instCount; ++i) { // Determine base visibility
        if (i == World.weaponVModelIndex || EntNotVisible(i,false)) { continue; }
        Entity* e = &World.instances[i];
        V3 delta = V3_AsubB(World.position[i],playerPos);
        float distSqrd = V3_dot(delta,delta);
        float radius = (e->modelIndex < MAX_MDLS ? modelBounds[e->modelIndex] : 0.0f) * 2.0f * vmax(vmax(World.scale[i].x,World.scale[i].y),World.scale[i].z);
        if (!SphereInFrustum(playerFrustumPlanes,World.position[i],radius)) { continue; }
        if (IdxIsPortalBlockingDoor(e->index)) { /*Extra checks only needed for opaque portal blocking doors.*/
            if (!(gridCellStates[e->cellIndex] & CELL_VISIBLE) && !NeighborhoodInPVS(e->cellX,e->cellZ,2u) /*!in pvs*/) { continue; }
        } else {
            if (((gridCellStates[e->cellIndex] & (CELL_VISIBLE | CELL_OPEN)) == CELL_OPEN)){ continue; }
            if (!(gridCellStates[e->cellIndex] & CELL_OPEN) && distSqrd >= 943.7184f) { continue; /*30.72 * 30.72, 12 cells*/ }
        }
        if (World.instances[i].camView != 255) camViews[World.instances[i].camView].visible = true;
        if (transparentTexture[World.instances[i].texIndex]) { if (tcnt < INSTANCE_COUNT) { tmpTransparent[tcnt].index = i; tmpTransparent[tcnt].depth = distSqrd; tcnt++; } } else { visibleInstances[opaqueCount].index = i; visibleInstances[opaqueCount].depth = distSqrd; opaqueCount++; }
    }
    if (World.shields[0] < U16_MAX && skyVisible && World.instCount < (INSTANCE_COUNT - 4)) { for (u8 k=0;k<4;++k) { visibleInstances[opaqueCount].index=World.shields[k]; visibleInstances[opaqueCount++].depth=300.0f; } }
    if (editModeSelection < U16_MAX && Cheats.editMode) {
        u16 selIdx=World.instances[editModeSelection].index;
        if (selIdx != 592 && selIdx != 593) { /* decals have no mesh: baked 3D text drawn in decal pass; force-draw would hit modelIndex U16_MAX */
            if (transparentTexture[World.instances[editModeSelection].texIndex]) {
                if (tcnt < INSTANCE_COUNT) {          tmpTransparent[tcnt].index = editModeSelection;          tmpTransparent[tcnt].depth = 1.28f;        tcnt++; }
            } else                         { visibleInstances[opaqueCount].index = editModeSelection; visibleInstances[opaqueCount].depth = 1.28f; opaqueCount++; }
        }
    }
    mcpy(visibleInstances + opaqueCount,tmpTransparent,tcnt * sizeof(DepthSort));
    glUseProgram(depthPrepassSP);
    glUniformMatrix4fv(2,1,0,viewProj);
    glEnable(GL_DEPTH_TEST); glColorMask(0,0,0,0); glDepthMask(1); glDepthFunc(0x201/*GL_LESS*/); glDisable(GL_BLEND);
    if (opaqueCount > 1) qsort_new(visibleInstances,opaqueCount,sizeof(DepthSort),dsortInv);/*Needed for cutout bushes/foliage*/
    if (tcnt > 1) qsort_new(visibleInstances + opaqueCount,tcnt,sizeof(DepthSort),dsort);
    u8 cullBlendState = 0xFF;
    for (u16 visibleIndex = 0; visibleIndex < opaqueCount + tcnt; ++visibleIndex) {
        u16 i = visibleInstances[visibleIndex].index;
        Entity* e = &World.instances[i];
        u16 tex = e->texIndex;
             if (unlikely(doubleSidedTexture[tex] || World.scale[i].x<0.0f || World.scale[i].y<0.0f || World.scale[i].z<0.0f)) { glDisable(GL_CULL_FACE);  glEnable(GL_BLEND); } /*Doublesided (incl. transparent doublesided, e.g. cyberspace panels)*/
        else if (unlikely(transparentTexture[tex]))                                                                            {  glEnable(GL_CULL_FACE);  glEnable(GL_BLEND); } /*Transparents (with sort)*/
        else                                                                                                                   {  glEnable(GL_CULL_FACE); glDisable(GL_BLEND); } /*Opaque*/
        currentModelType=GetAndBindModel(i,currentModelType);
        if (tex != currentTexIndex) { currentTexIndex=tex; glUniform1ui(3,(u32)tex); }
        u32 vertCount = modelTriangleCounts[currentModelType] * 3;
        glDrawElements(0x4/*GL_TRIANGLES*/, vertCount, GL_UNSIGNED_SHORT, (void*)(uintptr_t)curIboDrawOfs); drawCalls++; vertsRendered += vertCount;
    }
    glEndQuery(0x88BF/*GL_TIME_ELAPSED*/);
    glBeginQuery(0x88BF/*GL_TIME_ELAPSED*/, gpuQ[gpuQFrame][2]);
    glUseProgram(chunkSP);
    glUniformMatrix4fv(2,1,0,viewProj);
    glUniform1ui(25,0u); /*default constIndex*/
    glUniform1ui(32,0u);
    glUniform1ui(34, (World.curLev == LEVEL_CYBERSPACE) ? 1u : 0u); /*cyberspace: shader uses hardcoded neon directionals, else the sun*/
    glUniform1ui(37, (u32)(sunActive?1u:0u));
    if (sunActive) { glUniform3f(38, sunDirection.x, sunDirection.y, sunDirection.z); glUniformMatrix4fv(40, 1, GL_FALSE, sunShadowMatrix); glActiveTexture(GL_TEXTURE10); glBindTexture(GL_TEXTURE_2D, sunShadowTex); glActiveTexture(GL_TEXTURE0); }
    bool grayscaleEnabled = ((World.invP1.hasHardware & HW_INF) && (World.invP1.hardwareIsActive & HW_INF) > 0);
    glUniform1ui(26,(u32)grayscaleEnabled);
    float fogActual = World.fogColor[World.curLev].a + ((float)World.fogFac / 255.0f); /*Alpha is base density for level.*/
    glUniform3f(12, World.fogColor[World.curLev].r * fogActual,World.fogColor[World.curLev].g * fogActual, World.fogColor[World.curLev].b * fogActual); /*Fog Color(which is density)*/
    glUniform1f(39, World.curLev == LEVEL_CYBERSPACE ? 1.0f / 190.0f : 1.0f / World.farPlane[World.curLev]);
    glUniform1f(35, World.fogColor[World.curLev].a);
    glUniform1ui(14,Sys_Settings.Reflections);
    glUniform1ui(15,Sys_Settings.Shadows);
    glUniform2f(8, World.worldMin_x[World.curLev], World.worldMin_z[World.curLev]);
    glUniform3f(10,px,py,pz);
    glColorMask(1,1,1,1);
    glDepthMask(0);
    glDepthFunc(0x203/*GL_LEQUAL*/); /*Opaque Pass*/
    currentTexIndex = currentNormIndex = currentGlowIndex = currentSpecIndex = currentModelType = U16_MAX;
    glUniform1f(9,0.0f); // Reset heat for infrared vision
    float currentVol = -1.0f;
    for (u16 visibleIndex=0;visibleIndex<opaqueCount+tcnt;++visibleIndex) {
        bool alpha=visibleIndex>=opaqueCount; if (visibleIndex==opaqueCount) glDepthMask(1);
        u16 i=visibleInstances[visibleIndex].index; Entity* e=&World.instances[i]; u16 tex=e->texIndex,constIndex=e->index;
        if (!alpha && transparentTexture[tex]) continue;
        bool twoSided = doubleSidedTexture[tex] || World.scale[i].x<0.0f || World.scale[i].y<0.0f || World.scale[i].z<0.0f; if (alpha && !twoSided && !transparentTexture[tex]) continue;
        
        u8 state=twoSided ? 2 : alpha ? 1 : 0;
        if (cullBlendState!=state) { if (twoSided) glDisable(GL_CULL_FACE); else glEnable(GL_CULL_FACE); if (state) glEnable(GL_BLEND); else glDisable(GL_BLEND); cullBlendState=state; }
        if (alpha) glDepthFunc(((constIndex>=561 && constIndex<=565) || (constIndex>=568 && constIndex<=573)) ? 0x202/*GL_EQUAL*/ : 0x203/*GL_LEQUAL*/);
        DrawEntity(e,i,constIndex,tex,&currentNormIndex,&currentTexIndex,&currentGlowIndex,&currentSpecIndex,&currentModelType,&currentVol,grayscaleEnabled);
    }
    glDepthMask(1);
    glUniform1ui(32,1);
    glDepthFunc(0x203/*GL_LEQUAL*/);
    glUniform1ui(1, 0u);
    glUniform1ui(17, 0u);
    glUniform1ui(19, 0u);
    glUniform1ui(20, 0u);
    glUniform1ui(13, 0u);
    glUniform1ui(30, 0u);
    glUniform2f(33, 0.0f, 0.0f);
    glUniform1f(27, 0.0f);
    glActiveTexture(0x84C9/*GL_TEXTURE9*/);
    u8 tdLev = World.curLev;
    for (u16 i = INSTS_1ST_IDX; i < World.instCount; ++i) { // 3D text decals 592 text_decal, 593 text_decalStopDSS1
        if (textDecalVBO[tdLev][i]==0 || textDecalVertexCount[tdLev][i]==0) { continue; }
        u16 cIdx = World.instances[i].index; if (cIdx!=592 && cIdx!=593) { continue; }
        if (!(World.instances[i].entflags & EF_ACTIVE)) { continue; }
        if (!((gridCellStates[World.instances[i].cellIndex] & CELL_VISIBLE) || ((gridCellStates[World.instances[i].cellIndex] & CELL_OPEN) == CELL_OPEN))) { continue; }
        
        if (cullBlendState != 2) { /*Double-sided*/ glDisable(GL_CULL_FACE); glEnable(GL_BLEND); cullBlendState = 2; }
        glUniform1ui(0, (u32)i);
        glUniform1ui(18, (u32)World.instances[i].texIndex);
        glUniform1ui(25, (u32)cIdx);
        glBindTexture(GL_TEXTURE_2D, (cIdx == 593) ? fontAtlasTexStopD : fontAtlasTex);
        glBindVertexBuffer(0, textDecalVBO[tdLev][i], 0, 16/*VRT_ATT_SZ*/);
        glDrawArrays(0x4/*GL_TRIANGLES*/, 0, (i32)textDecalVertexCount[tdLev][i]); drawCalls++; vertsRendered += textDecalVertexCount[tdLev][i];
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glUniform1ui(32, 0u); /* restore chunkSP font-atlas off for subsequent draws */
    u16 wvi = World.weaponVModelIndex;
    if (wvi > 0 && wvi < INSTANCE_COUNT) {
        int wep16 = Get16WeaponIndexFromConstIndex(World.instances[wvi].index);
        if (wep16 >= 0 && wep16 < 16 && World.instances[wvi].modelIndex < MAX_MDLS) { // Offset in player-local space (right,down,forward), rotated into world by the player view; reload/swap dip added on Y.  Pivot = player position, weapon stays locked to view.
            V3 weaponViewOffset=vWepOfs[wep16];
            weaponViewOffset.y += wfx.reloadContainerPos.y;
            weaponViewOffset = V3Add(weaponViewOffset, debugWepOffset);
            World.position[wvi] = V3Add(World.position[PLAYER1], quat_rot_v3(World.rotation[PLAYER1], weaponViewOffset));
            World.rotation[wvi] = quat_multiply(World.rotation[PLAYER1], vWepRot[wep16]); // view orientation + per-model correction
            u16 curN = U16_MAX, curT = U16_MAX, curG = U16_MAX, curS = U16_MAX, curM = U16_MAX; float curV = -1.0f;
            DrawEntity(&World.instances[wvi], wvi, World.instances[wvi].index, World.instances[wvi].texIndex, &curN, &curT, &curG, &curS, &curM, &curV, false);
        }
    }
    if(unlikely(camView)) { glEndQuery(0x88BF/*GL_TIME_ELAPSED*/); i32 cw = vmin((i32)swidth,(i32)vmin(Sys_Settings.ScreenWidth,camViews[camViewIdx].width)),ch=vmin((i32)sheight,(i32)vmin(Sys_Settings.ScreenHeight,camViews[camViewIdx].height)); glCopyImageSubData(inputImageID,GL_TEXTURE_2D,0,0,0,0,camViewTextures[camViewIdx],GL_TEXTURE_2D,0,0,0,0,cw,ch,1); return; } // <<<<<<<<<<<<< CAM VIEW BARRIER
    PSys_Render(viewProj, playerPos,(V3){invViewRot[0], invViewRot[1], invViewRot[2]}, (V3){invViewRot[3], invViewRot[4], invViewRot[5]}, (V3){-invViewRot[6], -invViewRot[7], -invViewRot[8]}, inputDepthID, snear, sfar, (float)swidth, (float)sheight);
    if (wireVertCount) { glBindBuffer(GL_ARRAY_BUFFER,wireLinesVBO); glBufferSubData(GL_ARRAY_BUFFER,0,wireVertCount*sizeof(WireVrt),wireVerts); glUseProgram(debugUnlitSP); glUniformMatrix4fv(0,1,GL_FALSE,viewProj); glEnable(GL_DEPTH_TEST); glDisable(GL_CULL_FACE); glBindVertexArray(wireLinesVAO); glDrawArrays(0x4/*GL_TRIANGLES*/,0,wireVertCount); drawCalls++; vertsRendered += wireVertCount; wireVertCount = 0; glEnable(GL_CULL_FACE); }
    if (Cheats.editMode) { /*Point-light range spheres, editmode only.*/ for (u16 i = 0; i < World.loadedLights; ++i) { Light* L = &World.lights[i]; DrawSphereWireframe(i == editModeLightSelection ? (Color){1.0f,0.5f,0.5f,1.0f} : (Color){0.9216f,0.9176f,0.7725f,1.0f}, (ShapeSphere){L->pos, 0.02 * L->range}); } }
    if(unlikely(World.lineVertCnt > 0)) { glBindBuffer(GL_ARRAY_BUFFER,debugLinesVBO); glBufferSubData(GL_ARRAY_BUFFER,0,World.lineVertCnt * sizeof(WireVrt),lineVerts); glUseProgram(debugUnlitSP); glUniformMatrix4fv(0,1,GL_FALSE,viewProj); glLineWidth(1.0f); glDisable(GL_DEPTH_TEST); glBindVertexArray(debugLinesVAO); glDrawArrays(0x1/*GL_LINES*/,0,World.lineVertCnt); drawCalls++; vertsRendered += World.lineVertCnt; World.lineVertCnt = 0; glEnable(GL_DEPTH_TEST); }
    glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D,inputDepthID); glEndQuery(0x88BF/*GL_TIME_ELAPSED*/); glBeginQuery(0x88BF/*GL_TIME_ELAPSED*/, gpuQ[gpuQFrame][3]);    
    if (Sys_Settings.Reflections==1u || Sys_Settings.Reflections==2u) {
        bool rt=Sys_Settings.Reflections==2u;
        glUseProgram(rt ? rtSP : ssrSP); if (rt) glBindBufferBase(GL_SSBO,20,modelVBO);
        glUniform3f(3,playerPos.x,playerPos.y,playerPos.z); glUniformMatrix4fv(6,1,0,invViewProj); glUniformMatrix4fv(4,1,GL_FALSE,viewProj); if (rt) { glUniform2f(7,World.worldMin_x[World.curLev],World.worldMin_z[World.curLev]); glUniform1ui(9,texCnt); }
        u32 group = rt ? 8 : 32;
        glDispatchCompute((Sys_Settings.ScreenWidth / Sys_Settings.SSR_RES + group - 1)/group,(Sys_Settings.ScreenHeight / Sys_Settings.SSR_RES + group - 1) / group,1);
        glMemoryBarrier(0x20/*GL_SHADER_IMAGE_ACCESS_BARRIER_BIT*/ | 0x8/*GL_TEXTURE_FETCH_BARRIER_BIT*/); /*Compat: SSR/RT wrote outputImage via imageStore; the composite pass samples it as a texture right after - undefined without a barrier per GL4.3/ES3.1*/
    }
    glEndQuery(0x88BF/*GL_TIME_ELAPSED*/);  /*DIAG: RT pass ends here*/ glBeginQuery(0x88BF/*GL_TIME_ELAPSED*/, gpuQ[gpuQFrame][4]);
    glBindFramebuffer(GL_FRAMEBUFFER,uiFBO); glClearColor(0,0,0,0); glClear(GL_COLOR_BUFFER_BIT); glViewport(0,0,UI_W,UI_H); glDisable(GL_CULL_FACE); glEnable(GL_BLEND);
    renderTime = get_time() - rendStart; 
    RenderUI();
    glBindFramebuffer(GL_FRAMEBUFFER,0); glViewport(0,0,swidth,sheight);
    glUseProgram(imageBlitSP);
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D,inputImageID); glUniform1i(4,4); // outputImage texture sampler2D, don't remember why when active texture is texture 0. meh.... oh maybe to not read and write same binding?
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D,inputUIID);    glUniform1i(31,1);
    glUniform1i(32,3); glUniformMatrix4fv(33,1,0,invViewProj);
    double berserkTimeRemainingNormalized = World.invP1.berserkFinished > 0.0001 ? (World.invP1.berserkFinished - World.pauseRelativeTime) / 20.0 : 0.0;
    if (World.invP1.berserkFinished < World.pauseRelativeTime && World.invP1.berserkFinished > 0.0001) { World.invP1.berserkFinished = berserkTimeRemainingNormalized = 0.0; }
    glUniform1ui(5,Sys_Settings.Reflections); glUniform1ui(6,Sys_Settings.FXAA); glUniform1f(14,Sys_Settings.FOV); glUniform1ui(22,Sys_Settings.Shadows); glUniform1ui(11,Sys_Settings.Brightness);
    glUniform1f(16,aspect3D);
    glUniform1f(9,(float)berserkTimeRemainingNormalized);
    glUniform1f(10,berserkSeedTime);
    glUniform3f(12,deg2rad(World.cam_yaw), deg2rad(World.cam_pitch), deg2rad(World.cam_roll));
    glUniform3f(13,px,py,pz);
    glUniform1f(15,(float)World.pauseRelativeTime * 0.1f);
    SunDirection(); glUniform1f(30,sunYawCur); /*Sky yaw phase from the SAME float the sun shadow direction uses, so the skybox sun and the chunk shadow rotation are locked together and smooth (SunDirection() is pure + cheap; also refreshes it when shadows are disabled and the shadow pass never ran this frame)*/
    glUniform1ui(17,(gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX) || World.curLev == LEVEL_CYBERSPACE);
    glUniform1ui(18,(gridCellStates[playerCellIdx] & CELL_SEES_SUN) && World.curLev != LEVEL_CYBERSPACE);
    glUniform1ui(19,((World.curLev >= 10 && World.curLev < LEVEL_CYBERSPACE) ? 1u : 0u) && (gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX));
    u32 shieldOnType = 0u/*No shield green tint*/;
    if (World.instances[WORLD].ioflags & Q_SHIELD_ACTIVATED) { shieldOnType=(World.curLev <= 5) ? 1u/*Shielding everywhere*/ : 2u/*Shielding only below, levels 6+*/; }
    glUniform1ui(20,shieldOnType); // Green Shield
    Color3 painStaticColor = (Color3){1.0f,0.0f,0.0f}; // TODO reuse for blue EMP static effect
    glUniform3f(23,painStaticColor.r, painStaticColor.g, painStaticColor.b);
    glUniformMatrix4fv(24,1,0,viewProj);
    glUniformMatrix3fv(25,1,0,invViewRot);
    glUniform1i(27,0);
    glUniform1f(28,vclamp(World.painStaticAlpha + World.empStaticAlpha, 0.0f, 1.0f));
    glUniform1ui(29,(u32)((World.invP1.hasHardware & HW_INF) && (World.invP1.hardwareIsActive & HW_INF) > 0));
    glBindVertexArray(quadVAO);
    glDisable(GL_DEPTH_TEST);
    glDrawArrays(0x6/*GL_TRIANGLE_FAN*/, 0, 4); drawCalls++; vertsRendered += 4; /*Just a quad for the final composite*/
    glEndQuery(0x88BF/*GL_TIME_ELAPSED*/);
    if (Cheats.editMode && editModeSelection < U16_MAX && World.editTextInstanceIndex < INSTANCE_COUNT) {
        World.position[World.editTextInstanceIndex]=World.position[editModeSelection];
        float wx = World.position[editModeSelection].x, wy = World.position[editModeSelection].y, wz = World.position[editModeSelection].z;
        float clipX = viewProj[0]*wx + viewProj[4]*wy + viewProj[8]*wz + viewProj[12], clipY = viewProj[1]*wx + viewProj[5]*wy + viewProj[9]*wz + viewProj[13], clipW = viewProj[3]*wx + viewProj[7]*wy + viewProj[11]*wz + viewProj[15];
        float projXf = 0, projYf = 0;
        if (clipW > 0.01f) { float ndcX = clipX / clipW; float ndcY = clipY / clipW; projXf = ((ndcX + 1.0f) * 0.5f) * (float)swidth; projYf = ((1.0f - ndcY) * 0.5f) * (float)sheight; }
        char editTextStr[64]; sFormat(editTextStr, sizeof(editTextStr), "index: %d", editModeSelection);
        RenderText3DC((V3){projXf, projYf, 0.0f}, T_WHITE,FONT_NORMAL, 1.0f, editTextStr);
    }
    if ((World.last_time - World.lastFrameSecCountTime) >= 1.00) { World.lastFrameSecCountTime=World.last_time; globalframesPerLastSecond=globalframe - World.lastFrameSecCount; World.lastFrameSecCount=globalframe; } // Update Diagnostic Poll
}
void RenderCameraViews() { /*Render in-world camera views.  Pops player position to elsewhere, renders to tiny fbo, pops player back.*/
    if (unlikely(World.paused || World.menuActive || camViewCount == 0 || World.curLev >= LEVEL_CYBERSPACE)) { return; }
    V3 tempPlayerPos = World.position[PLAYER1]; Quaternion tempPlayerRot = World.rotation[PLAYER1]; // Store player position.
    for (int cm=0;cm<camViewCount;++cm) { if (camViews[cm].finished < World.pauseRelativeTime && camViews[cm].visible) { camViews[cm].finished = World.pauseRelativeTime + 0.5f; World.position[PLAYER1] = camViews[cm].position; World.rotation[PLAYER1] = camViews[cm].rotation; CullCore(); Render(true/*camview*/, cm); } }
    World.position[PLAYER1] = tempPlayerPos; World.rotation[PLAYER1] = tempPlayerRot; // Restore player position.
}
static void SetRTInstance(u32 i, u32 model, u32 tex, u32 glow, u32 active, float sx, float sy, float sz) {
    bool ok = (active != 0u) && (model < mdlsCnt) && (rtModelInfo[model].nodeBase != 0xFFFFFFFFu) && (sx > 1e-4f) && (sy > 1e-4f) && (sz > 1e-4f);
    rtInstanceInfo[i].geom[0] = ok ? rtModelInfo[model].nodeBase : 0xFFFFFFFFu; rtInstanceInfo[i].geom[1] = ok ? rtModelInfo[model].triBase : 0u; rtInstanceInfo[i].geom[2] = ok ? rtModelInfo[model].vtxBase : 0u; rtInstanceInfo[i].geom[3] = ok ? rtModelInfo[model].uvBase : 0u;
    rtInstanceInfo[i].info[0] = tex; rtInstanceInfo[i].info[1] = ok ? 1u : 0u; rtInstanceInfo[i].info[2] = model; rtInstanceInfo[i].info[3] = glow;
    rtInstanceInfo[i].iscl2[0] = ok ? 1.0f / (sx * sx) : 0.0f; rtInstanceInfo[i].iscl2[1] = ok ? 1.0f / (sy * sy) : 0.0f; rtInstanceInfo[i].iscl2[2] = ok ? 1.0f / (sz * sz) : 0.0f; rtInstanceInfo[i].iscl2[3] = 0.0f;
}
void UpdateInstanceMatrix4x4s() {
    float servHalf = -99.0f * (PI / 180.0f) * 0.5f;
    Quaternion servYaw = {0.0f, vsinf(servHalf), 0.0f, vcosf(servHalf)};
    for (u32 i = INSTS_1ST_IDX; i < World.instCount; i++) {        
        u32 m = i * 16;
        float x = World.rotation[i].x, y=World.rotation[i].y, z=World.rotation[i].z, w=World.rotation[i].w;
        float x2=x*x, y2=y*y, z2=z*z, xy=x*y, xz=x*z, yz=y*z, wx=w*x, wy=w*y, wz=w*z;
        float sclx=World.scale[i].x, scly=World.scale[i].y, sclz=World.scale[i].z;
        if (World.instances[i].index == 437/*npc_servbot: render-only mesh yaw*/) { Quaternion q = quat_multiply(servYaw, (Quaternion){x, y, z, w}); x = q.x; y = q.y; z = q.z; w = q.w; x2 = x*x; y2 = y*y; z2 = z*z; xy = x*y; xz = x*z; yz = y*z; wx = w*x; wy = w*y; wz = w*z; }
        modelMatrices[m + 0]  = (1.0f - 2.0f * (y2 + z2)) * sclx; modelMatrices[m + 1]  = (2.0f * (xy + wz))        * sclx; modelMatrices[m+2]  = (2.0f * (xz - wy))        * sclx; modelMatrices[m + 3]  = 0.0f; 
        modelMatrices[m + 4]  = (2.0f * (xy - wz))        * scly; modelMatrices[m + 5]  = (1.0f - 2.0f * (x2 + z2)) * scly; modelMatrices[m+6]  = (2.0f * (yz + wx))        * scly; modelMatrices[m + 7]  = 0.0f;
        modelMatrices[m + 8]  = (2.0f * (xz + wy))        * sclz; modelMatrices[m + 9]  = (2.0f * (yz - wx))        * sclz; modelMatrices[m+10] = (1.0f - 2.0f * (x2 + y2)) * sclz; modelMatrices[m + 11] = 0.0f;
        modelMatrices[m + 12] = World.position[i].x;              modelMatrices[m + 13] = World.position[i].y;              modelMatrices[m+14] = World.position[i].z;              modelMatrices[m + 15] = 1.0f;
    }
    if (World.instCount > INSTS_1ST_IDX) { glBindBuffer(GL_SSBO,matricesBufferID); u32 offsetFloats=(u32)INSTS_1ST_IDX * 16; u32 countFloats=((u32)World.instCount - (u32)INSTS_1ST_IDX) * 16; glBufferSubData(GL_SSBO, offsetFloats * 4, countFloats * 4, modelMatrices + offsetFloats); }
    if (Sys_Settings.Reflections == 2u) {
        u32 chFirst = INSTANCE_COUNT, chLast = 0u;
        for (u32 i=0; i<World.instCount; ++i) {
            u16 mi = World.instances[i].modelIndex; if ((instanceIsLODArray[i] || Sys_Settings.ModelDetail < 1u) && World.instances[i].lodIndex < mdlsCnt) mi = World.instances[i].lodIndex;
            SetRTInstance(i, (mi < mdlsCnt) ? mi : 0xFFFFFFFFu, World.instances[i].texIndex, World.instances[i].glowIndex, (World.instances[i].entflags & EF_ACTIVE) ? 1u : 0u, vabs(World.scale[i].x), vabs(World.scale[i].y), vabs(World.scale[i].z));
            InstanceRTInfo* cur = &rtInstanceInfo[i], *prev = &rtPrevInfo[i];
            if (cur->geom[0]!=prev->geom[0] || cur->geom[1]!=prev->geom[1] || cur->geom[2]!=prev->geom[2] || cur->geom[3]!=prev->geom[3] || cur->info[0]!=prev->info[0] || cur->info[1]!=prev->info[1] || cur->info[2]!=prev->info[2] || cur->info[3]!=prev->info[3] || cur->iscl2[0]!=prev->iscl2[0] || cur->iscl2[1]!=prev->iscl2[1] || cur->iscl2[2]!=prev->iscl2[2] || cur->iscl2[3]!=prev->iscl2[3]) {
                if (chFirst == INSTANCE_COUNT) chFirst = i; chLast = i; mcpy(prev, cur, sizeof(InstanceRTInfo));
            }
        }
        if (chFirst != INSTANCE_COUNT) { glBindBuffer(GL_SSBO, rtInstanceInfoID); glBufferSubData(GL_SSBO, (size_t)chFirst * sizeof(InstanceRTInfo), (size_t)(chLast - chFirst + 1u) * sizeof(InstanceRTInfo), rtInstanceInfo + chFirst); }
        glBindBuffer(GL_SSBO,rtCellCountsID); glBufferData(GL_SSBO,ARRSIZE * sizeof(u16),NULL,GL_DYNAMIC_DRAW);                glBufferSubData(GL_SSBO,0,ARRSIZE * sizeof(u16),cellCounts);
        glBindBuffer(GL_SSBO,rtCellListsID);  glBufferData(GL_SSBO,(size_t)ARRSIZE * 128u * sizeof(u16),NULL,GL_DYNAMIC_DRAW); glBufferSubData(GL_SSBO,0,(size_t)ARRSIZE * 128u * sizeof(u16),cellLists);
    }
}
static const Color fogLUT[MAX_LEVELS] = { {0.3207547f, 0.29200783f,0.29200783f,0.07f},/*0*/  {0.34509805f,0.38431373f,0.49019608f,0.055f},/*1*/  {0.47058824f,0.3882353f, 0.3928334f,0.05f},/*2*/  {0.32941177f,0.29411766f,0.2509804f,0.065f},/*3*/ {0.3882353f,0.452415f, 0.47058824f,0.075f},/*4*/ {0.3882353f, 0.4117647f, 0.47058824f,0.03f},/*5*/ {0.3f,0.24f,0.33f,0.070f},/*6*/ 
                                          {0.38679248f,0.3471719f, 0.3302332f,0.07f},/*7*/  {0.44708973f,0.45681614f,0.4811321f,0.040f},/*8*/ {0.4056604f,0.3992963f,0.36930403f,0.050f},/*9*/ {0.48235294f,0.58431375f,0.5176471f, 0.04f},/*10*/ {0.52872473f,0.58431375f,0.48235294f,0.040f},/*11*/ {0.48235294f,0.58431375f,0.5176471f,0.05f},/*12*/ {0.0f,0.0f,0.0f,0.5f},/*13*/ };
static const V2 levMins[MAX_LEVELS]={{-37.3600f,-52.7600f},/*0*/  {-53.8000f,-64.0800f},/*1*/  {-46.12f,-56.34f},/*2*/  {-51.266f,-51.246f},/*3*/  {-29.462f, -53.7872f},/*4*/ {-47.3622f,-55.04f},/*5*/ {-65.94f,-71.6833f},/*6*/ {-66.8989f,-82.0144f},/*7*/ {-43.7456f,-43.9872f},/*8*/ {-51.5039f,-69.0306f},/*9*/ {-24.0994f,-39.7972f},/*10*/ {-27.1772f,-28.3394f},/*11*/ {-18.05f,-30.50f},/*12*/ {99.4200f,-60.120f}/*13*/};
static const float lFars[MAX_LEVELS] = { 56.32f/*R*/, 56.32f/*1*/, 51.2f/*2*/, 51.2f/*3*/, 40.96f/*4*/, 58.88f/*5*/, 79.36f/*6*/, 56.32f/*7*/, 69.12f/*8*/, 53.76f/*9*/,  51.2f/*10*/,  51.2f/*11*/, 38.4f/*12*/, 1300.00f/*13*/};
int EdgeCompare(const void* a, const void* b) { u32 ea = *(const u32*)a, eb = *(const u32*)b; return (ea > eb) - (ea < eb); }
u16 uniqueCvxMeshIndices[MAX_UNIQUE_CVX_MESHES]; u32 uniqueCvxMeshCount = 0;
void ResetDoorPortals(), MissionTimerInit(), AddHardwareToInventory(int,int), mp3_clear();
static void ResetLoadTallies(void) {
    decalInlineTextCount = decalStyleCount = 0;
    mset(World.levelCameraCount,0,sizeof(World.levelCameraCount));     mset(World.levelSmallNodeCount,0,sizeof(World.levelSmallNodeCount));     mset(World.levelLargeNodeCount,0,sizeof(World.levelLargeNodeCount));
    mset(World.levCamDestroyedCnt,0,sizeof(World.levCamDestroyedCnt)); mset(World.levSmNodeDestroyedCnt,0,sizeof(World.levSmNodeDestroyedCnt)); mset(World.levNodeDestroyedCnt,0,sizeof(World.levNodeDestroyedCnt));
    for (u32 u = 0; u < uniqueCvxMeshCount; ++u) { if (cvxAdjOffsets[u]) { OS_Free(cvxAdjOffsets[u], cvxAdjOffsetBytes[u]); cvxAdjOffsets[u] = 0; cvxAdjOffsetBytes[u] = 0; } if (cvxAdjLists[u]) { OS_Free(cvxAdjLists[u], cvxAdjListBytes[u]); cvxAdjLists[u] = 0; cvxAdjListBytes[u] = 0; } }
    uniqueCvxMeshCount = 0; ResetDoorPortals(); mset(World.physSleep, 0, sizeof(World.physSleep));
}
// Init && Main
bool MdlIsWinMemberSet(u16,u8); void MdlSwapWindowDisposeData(u16);
void MdlBuildLevelConvexAdjacency(u8 lev) {
    if (lev >= MAX_LEVELS) return;
    for (u32 i = 0; i < World.levelInstCount[lev]; ++i) {
        World.levelInstances[lev][i].adjacencyIdx = U16_MAX;
        if (World.levelCollider[lev][i] == CT_CVX) {
            u16 colMeshIdx = World.levelInstances[lev][i].colMeshIndex;
            if (colMeshIdx > MAX_MDLS) {DualLogWarn("Improper convex mesh colMeshIndex on level %u, instance %u with constindex %u for convex mesh uniques, colMeshIndex: %u\n",lev,i,World.levelInstances[lev][i].index,World.levelInstances[lev][i].colMeshIndex); continue;}
            bool isUnique = true; u32 foundIdx = U16_MAX;
            for (u32 u=0;u<uniqueCvxMeshCount;++u) { if (uniqueCvxMeshIndices[u] == colMeshIdx) { isUnique = false; foundIdx = u; World.levelInstances[lev][i].adjacencyIdx = (u16)foundIdx; break; } }
            if (isUnique) { if (uniqueCvxMeshCount >= MAX_UNIQUE_CVX_MESHES) { DualLogWarn("Exceeded MAX_UNIQUE_CVX_MESHES!\n"); World.levelInstances[lev][i].adjacencyIdx = U16_MAX; continue; } uniqueCvxMeshIndices[uniqueCvxMeshCount] = colMeshIdx; World.levelInstances[lev][i].adjacencyIdx = (u16)uniqueCvxMeshCount; uniqueCvxMeshCount++; }
        }
    }
    for (u32 u = 0; u < uniqueCvxMeshCount; ++u) {
        u16 m = uniqueCvxMeshIndices[u]; if (m >= MAX_MDLS || cvxAdjOffsets[u]) { continue; } u32 vCount = physVertCounts[m], tCount = modelTriangleCounts[m]; if (!vCount || !tCount || !physPos[m] || !physTris[m]) { continue; }
        u32 edgeCount = 0, *tempEdges = OS_Alloc(tCount * 3 * sizeof(u32));
        for (u32 t = 0; t < tCount; ++t) { u16 i0=physTris[m][t*3+0], i1=physTris[m][t*3+1], i2=physTris[m][t*3+2]; tempEdges[edgeCount++]=((u32)vmin(i0,i1) << 16) | vmax(i0,i1); tempEdges[edgeCount++]=((u32)vmin(i1,i2) << 16) | vmax(i1,i2); tempEdges[edgeCount++]=((u32)vmin(i2,i0) << 16) | vmax(i2,i0); }
        qsort_new(tempEdges,edgeCount,sizeof(u32),EdgeCompare);
        u32 uniqueEdgeCount = 0, *degree = OS_Alloc(vCount * sizeof(u32));
        for (u32 i = 0; i < edgeCount; ++i) { if (i == 0 || tempEdges[i] != tempEdges[i - 1]) { tempEdges[uniqueEdgeCount++]=tempEdges[i]; u16 a = (u16)(tempEdges[i]   >>  16); u16 b = (u16)(tempEdges[i] & 0xFFFF); degree[a]++; degree[b]++; } }
        u32* offsets=OS_Alloc((vCount + 1) * sizeof(u32)); offsets[0]=0; for (u32 i=0;i<vCount;++i) { offsets[i + 1] = offsets[i] + degree[i]; }
        u16* adjList = OS_Alloc(uniqueEdgeCount * 2 * sizeof(u16));
        u32* writePos = OS_Alloc(vCount * sizeof(u32)); mcpy(writePos, offsets, vCount * sizeof(u32));
        for (u32 i=0;i<uniqueEdgeCount;++i) { u16 a = (u16)(tempEdges[i] >> 16); u16 b = (u16)(tempEdges[i] & 0xFFFF); adjList[writePos[a]++]=b; adjList[writePos[b]++]=a; }
        cvxAdjOffsets[u] = offsets; cvxAdjLists[u] = adjList; cvxAdjOffsetBytes[u] = (vCount+1) * sizeof(u32); cvxAdjListBytes[u] = uniqueEdgeCount * 2 * sizeof(u16);
        OS_Free(tempEdges,tCount * 3 * sizeof(u32)); OS_Free(degree,vCount * sizeof(u32)); OS_Free(writePos,vCount * sizeof(u32));
    }
}
__attribute__((cold)) void NewGame() { // Reset World States
    double newGamePrepStart = get_time();
    CloseSearch();
    World.Sys_UI=(SystemUI){.MFD_MediaTab=MM_LOG_TABLE,.MFD_ReaderView=MFD_READER_CONTENTS,.mfdSelected={1,1,1},.mfdReturnTab={1,1,1},.lastAutomapSideRH=true,.generalClickSlot=-1,.generalClickItem=-1,.generalClickCustom=U16_MAX,.linkedElevatorDoor=U16_MAX,.tetheredPGP=U16_MAX,.tetheredPWP=U16_MAX,.tetheredSearchable=U16_MAX,.tetheredKeypadElevator=U16_MAX,.tetheredKeypadKeycode=U16_MAX,.keycodeHuns=-1,.keycodeTens=-1,.keycodeOnes=-1,.keycodeEntry=-1,
                            .logReaderPage=-1,.mg_current=-1,.pw_selectedWire=-1};
    DualLog("Loading new game");
    RenderLoading("Loading new game...");
    World.menuActive = World.paused = enteringPlayerName = fovSliderActive = gammaSliderActive = masterVolumeSliderActive = musicVolumeSliderActive = messageVolumeSliderActive = sfxVolumeSliderActive = returnToPause = false;
    for (int i=0;i<MAX_LEVELS;++i) { World.worldMin_x[i] = levMins[i].x; World.worldMin_z[i] = levMins[i].y; World.voxMinCtrX[i] = World.worldMin_x[i] + (VOXEL_SIZE * 0.5f); World.voxMinCtrZ[i] = World.worldMin_z[i] + (VOXEL_SIZE * 0.5f); World.farPlane[i] = lFars[i]; World.fogColor[i] = fogLUT[i]; World.fogColor[i].a *= 3.8f; }
    World.curLev = 0; World.col[0] = CT_NONE;
    currentMenuItem = currentMenuTab = 0; currentMenuPage = MPG_FRONT;
    World.current_time = newGamePrepStart; World.pauseRelativeTime = World.last_physics_time = 0.0; World.dt = 0.0166666666f;
    mset(World.instances,0,3 * sizeof(Entity));
    World.instances[PLAYER1].index = 767;
    World.layer[PLAYER1] = L_Player; World.scale[PLAYER1] = (V3){1.0f,1.0f,1.0f}; World.rotation[PLAYER1] = (Quaternion){0.0f,0.7071f,0.0f,0.7071f}; // 90deg rotation CW about Y axis as viewed from the top looking down onto player
    World.instances[PLAYER1].entflags = EF_ACTIVE|EF_RIGIDBODY; World.instances[PLAYER1].modelIndex = MAX_MDLS;
    World.col[PLAYER1] = CT_CAP; World.colliderCenter[PLAYER1].y = -PLYR_CAM_OFS_Y; World.colliderSize[PLAYER1] = (V3){0.48f,2.0f,1.0f}; World.velocity[PLAYER1] = (V3){0.0f,0.0f,0.0f};
    World.cam_yaw = 90.0f; World.cam_pitch = World.cam_roll = World.invP1.leanTarget = 0.0f; World.instances[PLAYER1].hasGrav = true;
    World.instances[PLAYER1].health = 211.0f; World.instances[PLAYER1].cyberHealth = 255.0f; World.instances[PLAYER1].bloodType = BloodType_Red;
    World.invP1.noiseFinished = World.pauseRelativeTime; World.invP1.energy = 54.0f; World.invP1.energyDrainTickFinished = World.pauseRelativeTime + 0.1 + (double)random_range(0.5f,1.0f);
    for (int i=0;i<12;++i) { World.invP1.hardwareInvReferenceIndex[i] = 21 + i; } World.invP1.hardwareInvReferenceIndex[12] = World.invP1.hardwareInvReferenceIndex[13] = 0;
    World.invP1.genInvIdxRef[0] = 81; // Hardcoded lookup indices into the Const main table.
    for (int i=1;i<HW_COUNT;i++) World.invP1.genInvIdxRef[i] = -1; // Skips 0th index on purpose as it always holds access cards "item".
    for (int i=0;i<HW_COUNT;++i) World.invP1.hwVers[i] = World.invP1.hwVersSetting[i] = 0;
    World.invP1.nitroTimeSetting = 7.0; World.invP1.earthShakerTimeSetting = 10.0; World.invP1.lastAddedIndex = -1; World.invP1.hasNewEmail = World.invP1.hasNewNotes = World.invP1.isPulserNotDrill = true;
    MissionTimerInit();
    for (int i=0;i<7;++i) World.invP1.wepInvIndices[i] = World.invP1.wepInvAmIndices[i] = -1;
    World.invP1.justFired = (World.pauseRelativeTime - 31.0); /*Set >30s before pauseRelativeTime to not immediately play action music.*/ World.invP1.painSoundFinished = World.invP1.radSoundFinished = World.invP1.ressurectingFinished = World.pauseRelativeTime;
    World.invP1.mediFinished = World.invP1.reflexFinishedTime = World.invP1.sightFinishedTime = -1.0; World.invP1.mediPatchPulseFinished = 0.0; World.invP1.mediPatchPulseCount = 0; World.invP1.berserkIncrement = World.invP1.patchActive = 0; World.invP1.staminupActive = World.geniusActive = World.invP1.inReverbZone = false;
    World.Sys_UI.lastMultiMediaTabOpened = MM_EMAIL_TABLE; World.Sys_UI.tickFinished = World.current_time + 0.1 + (double)random_range(0.0f,1.0f);
    World.timeScale = 1.0; World.inventoryMode = Sys_Settings.NoShootMode; World.gameFinished = World.creditsActive = World.decoyActive = false; World.decoyInstance = U16_MAX; World.damageDealt = World.damageReceived = 0.0f;
    World.ressurections = World.deaths = World.kills = World.cyberkills = 0u; World.shotsFired = World.grenadesThrown = World.savesScummed = 0U; World.creditsPageIndex = 0u;
    for (int i=0;i<14;++i) {World.levelSecurity[i] = 100u;} World.levelSecurity[LEVEL_CYBERSPACE] = 0u/*Cyberspace has no security levels.*/;
    mset(&Sys_Input,0,sizeof(Sys_Input));
    World.currentMouse_dx = World.currentMouse_dy = 0; last_mouse_x = last_mouse_y = 0; ignore_next_mouse_delta = true; Sys_Input.isCapsLockOn = false; /*As far as we're concerned, don't worry about OS capslock actual state.*/
    for (u8 lev=1;lev<MAX_LEVELS;++lev) { CopyPlayerState(0,lev); }
    ResetLoadTallies();
    DualLog(" prep...............................................................took %.6f secs\n",get_time() - newGamePrepStart);
    LoadAllLevels();
    for (u8 lev=0;lev<MAX_LEVELS;++lev) { /*Done after levels are loaded to remove difficulty related items.*/
        u8 entryLevel = World.curLev; if (entryLevel != lev) SetLevelPointers(lev);
        for (u16 i = INSTS_1ST_IDX; i < World.instCount; ++i) { u16 ci = World.instances[i].index; if (((World.diffMis <= 1 && (ci == 341 || ci == 388 || (ci >= 390 && ci <= 398) || ci == 417)) || (World.diffMis == 0 && (ci == 313 || ci == 448)))) { DeleteInstance(i); } /*Remove access cards and audio logs.*/ }
        if (World.curLev != entryLevel) SetLevelPointers(entryLevel);
    }
    LoadModels();
    LoadLevel(World.startLevel,(V3){10.52f, -43.792f + 0.84f, 20.2908f});
    double newGameCvxStart = get_time();
    World.invP1.currentCrouchRatio = 1.0f;
    for (u32 m=0; m<mdlsCnt; ++m) { if (mdlPersistentSet[m]) continue; if (World.curLev < MAX_LEVELS && MdlIsWinMemberSet((u16)m,World.curLev)) continue; if (physPos[m] || modelTriangles[m] || modelVertexCounts[m]) { MdlSwapWindowDisposeData((u16)m); } }
    World.lev1SecCode = World.lev2SecCode = World.lev3SecCode = World.lev4SecCode = World.lev5SecCode = World.lev6SecCode = -1; World.missionBits = 0;
    AutomapNewGame();
    firstFrameMouselook = true; // Prevent jumps after cursor is centered once menu turned off.
    DualLog("Convex mesh and automap init........................................................took %.6f secs\n",get_time() - newGameCvxStart);
}
void PlayVmail(u8 i) {/*Starts vmail clip i: the ui.c vmailStartFrames/vmailEndFrames slots. Sys_UI.vmailActive also gates the frob/fire "click skips the mail" paths and the UI draws as a bool, so it stores clip+1 (0 stays "no mail playing").*/ if(i>=sizeof(vmailStartFrames)/sizeof(*vmailStartFrames)){DualLogError("PlayVmail: clip %u out of range\n",i);return;} World.Sys_UI.vmailActive=(u8)(i+1); World.Sys_UI.vmailFrame=vmailStartFrames[i]; World.Sys_UI.vmailFrameFinished=World.pauseRelativeTime + 0.1; ForceInventoryMode(); }
void GoIntoGame() { NewGame(); mp3_clear(); DualLog("Player named \"%s\" started the game!\n", World.playerName); game_actual_start_time = get_time(); DualLog("Entity counts::0:%4u|1:%4u|2:%4u|3:%4u|4:%4u|5:%4u|6:%4u|7:%4u|8:%4u|9:%4u|10:%4u|11:%4u|12:%4u|13:%4u\n Light counts::0:%4u|1:%4u|2:%4u|3:%4u|4:%4u|5:%4u|6:%4u|7:%4u|8:%4u|9:%4u|10:%4u|11:%4u|12:%4u|13:%4u\n",World.levelInstCount[0],World.levelInstCount[1],World.levelInstCount[2],World.levelInstCount[3],World.levelInstCount[4],
                                              World.levelInstCount[5],World.levelInstCount[6],World.levelInstCount[7],World.levelInstCount[8],World.levelInstCount[9],World.levelInstCount[10],World.levelInstCount[11],World.levelInstCount[12],World.levelInstCount[13],World.levelLoadedLights[0],World.levelLoadedLights[1],World.levelLoadedLights[2],World.levelLoadedLights[3],
                                              World.levelLoadedLights[4],World.levelLoadedLights[5],World.levelLoadedLights[6],World.levelLoadedLights[7],World.levelLoadedLights[8],World.levelLoadedLights[9],World.levelLoadedLights[10],World.levelLoadedLights[11],World.levelLoadedLights[12],World.levelLoadedLights[13]); }
void LoadModels(), LoadTextures(), InitAudio();
extern u32 random_range_rng, biomTexId, biomFBO;
u8* scratch_base,*scratch_cur,*scratch_end; size_t scratch_peak;
void* OS_AllocScratch(size_t amount) { size_t aligned = (amount + 15) & ~(size_t)15; u8* p = (u8*)__atomic_fetch_add(&scratch_cur,aligned,__ATOMIC_RELAXED); if (p + aligned > scratch_end) { DualLogError("Scratch ovr!\n"); OS_Exit(1); } size_t used = (size_t)((p + aligned) - scratch_base),peak;  do { peak = scratch_peak; } while (used > peak && !__atomic_compare_exchange_n(&scratch_peak,&peak,used,false,__ATOMIC_RELAXED,__ATOMIC_RELAXED)); return p; }
void OS_FreeInitPhase() { scratch_cur=scratch_base; mset(scratch_cur,0,scratch_peak); }
static void* InitFontAtlasses_Thread(void* arg) { (void)arg; InitFontAtlasses(); return NULL; }
static bool rtAccelBuilt = false;
static u32 rtTriSec = 0;
INLINE u32 F2U(float f) { union { float f; u32 u; } c; c.f = f; return c.u; }
static void RtWriteModelNodes(u32* nodes, u32 nodeBase, u32 triBase, u16 m) {
    BvhNode* srcArr = modelBVHNodes[m]; u32 ncnt = modelBVHNodeCounts[m];
    for (u32 i=0;i<ncnt;++i) {
        BvhNode* src = &srcArr[i]; u32* d = nodes + ((size_t)nodeBase + (size_t)i * 4u) * 4u;
        d[0]=F2U(src->mn.x-vabs(src->mn.x)*0.002f-1e-4f); d[1]=F2U(src->mn.y-vabs(src->mn.y)*0.002f-1e-4f); d[2]=F2U(src->mn.z-vabs(src->mn.z)*0.002f-1e-4f);
        d[4]=F2U(src->mx.x+vabs(src->mx.x)*0.002f+1e-4f); d[5]=F2U(src->mx.y+vabs(src->mx.y)*0.002f+1e-4f); d[6]=F2U(src->mx.z+vabs(src->mx.z)*0.002f+1e-4f);
        d[3] = (src->triCount > 0) ? (triBase + src->triStart * 2u) : 0xFFFFFFFFu; d[7] = src->triCount; for (int c=0;c<8;++c) { d[8+c] = (src->children[c] >= 0) ? (nodeBase + (u32)src->children[c] * 4u) : 0xFFFFFFFFu; }
    }
}
static void RtDispatchModelCopy(u16 m) { u32 tcnt = modelBVHTriOrderCounts[m]; if (!tcnt) { return; } glBindBufferRange(GL_SSBO, 18, modelIBO, (intptr_t)mdlIboOfs[m], (size_t)((size_t)modelTriangleCounts[m]*6u + 4u)); glUniform1ui(3, tcnt); glUniform1ui(4, rtModelInfo[m].triBase); glUniform1ui(5, rtModelInfo[m].orderBase); glDispatchCompute((tcnt + 255u) / 256u, 1, 1); glMemoryBarrier(0x2000/*GL_SHADER_STORAGE_BARRIER_BIT*/); }
void EnsureReflectionResources(void) {
    if (Sys_Settings.Reflections >= 1u && outputImageID == 0) { GenerateAndBindTexture(&outputImageID,GL_RGBA8,Sys_Settings.ScreenWidth / Sys_Settings.SSR_RES,Sys_Settings.ScreenHeight / Sys_Settings.SSR_RES,GL_RGBA,GL_UNSIGNED_BYTE,0x2601/*GL_LINEAR*/,NULL); glBindImageTexture(4,outputImageID,0,GL_FALSE,0,GL_READ_WRITE,GL_RGBA8); glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D,outputImageID); }
    if (Sys_Settings.Reflections == 2u && !rtAccelBuilt) {
        if (!modelVBO || !mdlsCnt) { return; }
        double startTime = get_time();
        RenderLoading("Loading raytracing data...");
        u32 nodeWords = rtCapNodeWords, triWords = rtCapTriWords; u32 triSec = (nodeWords + 3u) & ~3u; rtTriSec = triSec; u32 totalWords = triSec + ((triWords + 3u) & ~3u); size_t geomBytes = (size_t)totalWords * sizeof(u32); u32 nodeOff = 0, triOff = 0, orderAcc = 0;
        for (u32 m=0; m<mdlsCnt; ++m) {
            if (!mdlPersistentSet[m] || !BvhHasBVH((u16)m) || mdlVboOfs[m] == 0xFFFFFFFFu) { rtModelInfo[m].nodeBase = 0xFFFFFFFFu; rtModelInfo[m].triBase = rtModelInfo[m].vtxBase = rtModelInfo[m].uvBase = 0; rtModelInfo[m].orderBase = 0; continue; }
            rtModelInfo[m].orderBase = (orderAcc + 1u) & ~1u; orderAcc = rtModelInfo[m].orderBase + modelBVHTriOrderCounts[m]; rtModelInfo[m].nodeBase = nodeOff * 4u; rtModelInfo[m].triBase = triSec + triOff * 2u; rtModelInfo[m].vtxBase = mdlVboOfs[m] / VRT_ATT_SZ; rtModelInfo[m].uvBase = 0; nodeOff += modelBVHNodeCounts[m]; triOff += modelBVHTriOrderCounts[m];
        }
        rtGeomID = MakeSSBO(&rtGeomID, 16, geomBytes, NULL, GL_STATIC_DRAW);
        u32* nodes = (u32*)((u8*)glMapBufferRange(GL_SSBO,0,(size_t)rtPersistNodeWords * sizeof(u32),0x2/*GL_MAP_WRITE_BIT*/|0x4/*GL_MAP_INVALIDATE_RANGE_BIT*/));
        for (u32 m=0; m<mdlsCnt; ++m) { if (rtModelInfo[m].nodeBase != 0xFFFFFFFFu) RtWriteModelNodes(nodes, rtModelInfo[m].nodeBase, rtModelInfo[m].triBase, (u16)m); }
        glUnmapBuffer(GL_SSBO);
        u16* orderAll = (u16*)OS_Alloc((size_t)rtCapOrderU16 * sizeof(u16));
        u32 orderFill = 0;
        for (u32 m=0; m<mdlsCnt; ++m) { if (rtModelInfo[m].nodeBase == 0xFFFFFFFFu) { continue; } u32 ob = orderFill; if (orderFill & 1u) ++ob; rtModelInfo[m].orderBase = ob; mcpy(orderAll + ob, modelBVHTriOrder[m], (size_t)modelBVHTriOrderCounts[m] * sizeof(u16)); orderFill = ob + modelBVHTriOrderCounts[m]; }
        orderAll[orderFill] = orderAll[orderFill + 1] = 0;
        glUseProgram(rtCopySP);
        if (rtOrderBufID == 0) { rtOrderBufID = MakeSSBO(&rtOrderBufID, 19, (size_t)rtCapOrderU16 * sizeof(u16), orderAll, GL_STATIC_DRAW); } else { glBindBuffer(GL_SSBO, rtOrderBufID); glBindBufferBase(GL_SSBO, 19, rtOrderBufID); glBufferSubData(GL_SSBO, 0, (size_t)((size_t)(orderFill + 2u) * sizeof(u16)), orderAll); } /*re-enable path (should only happen if the first build ran before pools existed)*/
        OS_Free(orderAll, (size_t)rtCapOrderU16 * sizeof(u16));
        for (u32 m=0; m<mdlsCnt; ++m) { if (rtModelInfo[m].nodeBase != 0xFFFFFFFFu) RtDispatchModelCopy((u16)m); }
        rtInstanceInfoID = MakeSSBO(&rtInstanceInfoID, 21, (size_t)INSTANCE_COUNT * sizeof(InstanceRTInfo), NULL, GL_DYNAMIC_DRAW);
        rtCellCountsID   = MakeSSBO(&rtCellCountsID,   23, (size_t)ARRSIZE * sizeof(u16), NULL, GL_DYNAMIC_DRAW);
        rtCellListsID    = MakeSSBO(&rtCellListsID,    24, (size_t)ARRSIZE * 128 * sizeof(u16), NULL, GL_DYNAMIC_DRAW);
        rtAccelBuilt = true; RtSwapPatchModels(); DualLog("Raytraced reflections upload........................................................took %.6f secs\n",get_time() - startTime);
    }
}
extern u8 mdlSwapLev, *fontBmp1, *fontBmp2;
void RtSwapPatchModels() {
    if (!rtAccelBuilt) return;
    u8 lev = mdlSwapLev; static u16 rtPatchList[MAX_MDLS]; static u16 rtPatchStamp[MAX_MDLS]; u32 patchCnt = 0; if (lev < MAX_LEVELS) { u32 cnt = 0; for (u32 m=0; m<mdlsCnt; ++m) { if (!(MdlIsWinMemberSet((u16)m,lev) && m < mdlsCnt && modelVertexCounts[m] && rtSrcVertCounts[m])) continue; if (cnt < MAX_MDLS) rtPatchList[cnt] = (u16)m; ++cnt; } patchCnt = cnt; }
    mset(rtPatchStamp, 0, (size_t)mdlsCnt * sizeof(u16));
    for (u32 k=0;k<patchCnt;++k) { rtPatchStamp[rtPatchList[k]] = 1; }
    u32 nodeOff = rtPersistNodeWords / 16u, triOff = rtPersistTriWords / 2u, orderAcc = rtPersistOrderU16;
    for (u32 k=0;k<patchCnt;++k) {
        u16 m = rtPatchList[k]; if (mdlVboOfs[m] == 0xFFFFFFFFu || mdlIboOfs[m] == 0xFFFFFFFFu || !BvhHasBVH(m)) { rtModelInfo[m].nodeBase = 0xFFFFFFFFu; rtModelInfo[m].triBase = rtModelInfo[m].vtxBase = rtModelInfo[m].uvBase = 0; rtModelInfo[m].orderBase = 0; rtPatchStamp[m] = 0; continue; }
        rtModelInfo[m].orderBase = (orderAcc + 1u) & ~1u; rtModelInfo[m].nodeBase = nodeOff * 4u; rtModelInfo[m].triBase = rtTriSec + triOff * 2u; rtModelInfo[m].vtxBase = mdlVboOfs[m] / VRT_ATT_SZ; rtModelInfo[m].uvBase = 0; u32 ordn = (orderAcc + 1u) & ~1u; orderAcc = ordn + modelBVHTriOrderCounts[m];
        nodeOff += modelBVHNodeCounts[m]; triOff += modelBVHTriOrderCounts[m];
        rtPatchStamp[m] = 2;
    }
    for (u32 m=0; m<mdlsCnt; ++m) { if (mdlPersistentSet[m] || rtPatchStamp[m] || rtModelInfo[m].nodeBase == 0xFFFFFFFFu) continue; rtModelInfo[m].nodeBase = 0xFFFFFFFFu; rtModelInfo[m].triBase = rtModelInfo[m].vtxBase = rtModelInfo[m].uvBase = 0; rtModelInfo[m].orderBase = 0; }
    if (lev >= MAX_LEVELS || !patchCnt) return;    
    if (rtCapNodeWords > rtPersistNodeWords) {
        glBindBuffer(GL_SSBO, rtGeomID); u32* nodes = (u32*)((u8*)glMapBufferRange(GL_SSBO,(intptr_t)(rtPersistNodeWords * 4u),(size_t)((size_t)(rtCapNodeWords - rtPersistNodeWords) * sizeof(u32)),0x2/*GL_MAP_WRITE_BIT*/|0x4/*GL_MAP_INVALIDATE_RANGE_BIT*/|0x20/*GL_MAP_UNSYNCHRONIZED_BIT*/));
        u32 winNodeBase0 = rtPersistNodeWords / 4u; if (nodes) { for (u32 k=0;k<patchCnt;++k) { u16 m = rtPatchList[k]; if (rtPatchStamp[m] == 2) RtWriteModelNodes(nodes, rtModelInfo[m].nodeBase - winNodeBase0, rtModelInfo[m].triBase, m); } glUnmapBuffer(GL_SSBO); }
    }
    u32 winOrderBytes = (rtCapOrderU16 - rtPersistOrderU16) * 2u;
    u16* orderStaging = OS_Alloc(winOrderBytes); mset(orderStaging,0,(size_t)winOrderBytes);
    for (u32 k=0;k<patchCnt;++k) { u16 m = rtPatchList[k]; if (rtPatchStamp[m] != 2) continue; u32 w = rtModelInfo[m].orderBase - rtPersistOrderU16; mcpy(orderStaging + w, modelBVHTriOrder[m], (size_t)modelBVHTriOrderCounts[m] * sizeof(u16)); }
    glBindBuffer(GL_SSBO, rtOrderBufID); glBufferSubData(GL_SSBO, (intptr_t)((size_t)rtPersistOrderU16 * 2u), (size_t)winOrderBytes, orderStaging);
    OS_Free(orderStaging,winOrderBytes);
    glUseProgram(rtCopySP); glBindBufferBase(GL_SSBO,19,rtOrderBufID); glBindBuffer(GL_SSBO,rtGeomID); glBindBufferBase(GL_SSBO,16,rtGeomID);
    for (u32 k=0;k<patchCnt;++k) { u16 m = rtPatchList[k]; if (rtPatchStamp[m] == 2) RtDispatchModelCopy(m); }
    glFinish();
}
void InitalizeEnvironment() {
    game_start_time = get_time();
    random_range_rng = (u32)game_start_time;
    console_log_file = OS_OpenWriteonly("./voxen.log");
    DualLog("=========== Voxen, the Voxel Lit Open Source Game Engine by W. Josiah Jack, MIT-0 licensed ===========\nGame: Citadel, the System Shock (1994) Fan Remake                                1/03/2013 - 11/5/2026\n\nOpenGL 4.3+, libc 2.7(Linux) / none(Windows), depdendencies: ALSA/WASAPI, X11/GDI, dlopen+dlsym / none\nEntity byte size:%u\n",sizeof(Entity) + (6*4) + 4+4+1+1+4+4+4+4+(4*4)+1+(3*4)+(3*4)+(3*4)+(3*4)+(3*4)+(3*4));
    scratch_base = OS_Alloc(SCRATCH_ARENA_SIZE); scratch_cur = scratch_base; scratch_end = scratch_base + SCRATCH_ARENA_SIZE; /*Temporary scratch arena for all initializations.*/
    SetLevelPointers(0);
    #if defined(_WIN32)
        GetModuleHandleExW(0x6,(const u16*)&WinSys,(HMODULE*)&WinSys.win32.instance);
        WinSys.win32.dwmapi.instance = LoadLibraryA("dwmapi.dll"); if (WinSys.win32.dwmapi.instance) { WinSys.win32.dwmapi.IsCompositionEnabled = (PFN_DwmIsCompositionEnabled)PlatformGetModuleSymbol(WinSys.win32.dwmapi.instance,"DwmIsCompositionEnabled"); WinSys.win32.dwmapi.Flush = (PFN_DwmFlush)PlatformGetModuleSymbol(WinSys.win32.dwmapi.instance,"DwmFlush"); }
        WinSys.win32.ntdll.instance  = LoadLibraryA("ntdll.dll");  if (WinSys.win32.ntdll.instance ) { WinSys.win32.ntdll.RtlVerifyVersionInfo  = (PFN_RtlVerifyVersionInfo)PlatformGetModuleSymbol(WinSys.win32.ntdll.instance,"RtlVerifyVersionInfo"); }
        mset(WinSys.win32.keycodes,-1,sizeof(WinSys.win32.keycodes)); for (size_t i=0;i<sizeof(wkm)/sizeof(wkm[0]);i++) WinSys.win32.keycodes[wkm[i].i] = wkm[i].v; // Create key tables.
        WNDCLASSEXW wc={0}; wc.cbSize=sizeof(wc); wc.style = 0x20; wc.lpfnWndProc = (WNDPROC)helperWindowProc; wc.hInstance = WinSys.win32.instance; wc.n = L"WinSys3 Helper";
        WinSys.win32.helperWindowClass = RegisterClassExW(&wc);
        WinSys.win32.helperWindowHandle = CreateWindowExW(0x300,(u16*)(uintptr_t)WinSys.win32.helperWindowClass,L"WinSys message window",0x6000000,0,0,1,1,NULL,NULL,WinSys.win32.instance,NULL);
        WinSysPollMonitorsWin32();
    #else
        void* module = dlopen("libX11.so.6",2); PFN_XOpenDisplay XOpenDisplay = (PFN_XOpenDisplay)PlatformGetModuleSymbol(module,"XOpenDisplay"); Display* display = XOpenDisplay(NULL); WinSys.x11.display = display; WinSys.x11.xlib.handle = module;
        #define X(n) WinSys.x11.xlib.n = (PFN_X##n)PlatformGetModuleSymbol(WinSys.x11.xlib.handle, "X" #n);
            X(AllocSizeHints) X(ChangeProperty) X(CheckTypedWindowEvent) X(CreateColormap)   X(CreateWindow) X(DefineCursor) X(DisplayKeycodes) X(FilterEvent) X(FindContext)  X(Free)        X(UngrabPointer) X(GetKeyboardMapping) X(GetWMNormalHints) X(GetWindowAttributes)  X(GetWindowProperty)    X(WarpPointer)
            X(GrabPointer)    X(InternAtom)     X(MapWindow)             X(MoveResizeWindow) X(MoveWindow)   X(Pending)      X(QueryPointer)    X(RaiseWindow) X(ResizeWindow) X(SaveContext) X(SendEvent)     X(SetInputFocus)      X(SetWMNormalHints) X(SetWMProtocols)       X(TranslateCoordinates) X(UndefineCursor) 
        #undef X
        XNextEvent = (PFN_XNextEvent)PlatformGetModuleSymbol(WinSys.x11.xlib.handle,"XNextEvent"); WinSys.x11.screen = ((_XPrivDisplay)(WinSys.x11.display))->default_screen; WinSys.x11.root = (&((_XPrivDisplay)(WinSys.x11.display))->screens[WinSys.x11.screen])->root; WinSys.x11.context = 1; WinSys.x11.randr.handle = dlopen("libXrandr.so.2",2);
        #define X(n) WinSys.x11.randr.n = (PFN_XRR##n)PlatformGetModuleSymbol(WinSys.x11.randr.handle,"XRR"#n);
            X(FreeCrtcInfo) X(FreeOutputInfo) X(FreeScreenResources) X(GetCrtcInfo) X(GetOutputInfo) X(GetOutputPrimary) X(GetScreenResourcesCurrent) X(SelectInput) X(UpdateConfiguration)
        #undef X
        WinSys.x11.randr.SelectInput(WinSys.x11.display,WinSys.x11.root,4);
        void* xcurhandle = dlopen("libXcursor.so.1",2); PFN_XCIC ImageCreate = (PFN_XCIC)PlatformGetModuleSymbol(xcurhandle,"XcursorImageCreate"); PFN_XCID ImageDestroy = (PFN_XCID)PlatformGetModuleSymbol(xcurhandle,"XcursorImageDestroy"); PFN_XCILC ImageLoadCursor = (PFN_XCILC)PlatformGetModuleSymbol(xcurhandle,"XcursorImageLoadCursor");
        int sMin,sMax; mset(WinSys.x11.keycodes,-1,sizeof(WinSys.x11.keycodes)); WinSys.x11.xlib.DisplayKeycodes(WinSys.x11.display,&sMin,&sMax); int wk; KeySym* k = WinSys.x11.xlib.GetKeyboardMapping(WinSys.x11.display,sMin,sMax - sMin + 1,&wk); for (int s=sMin;s<=sMax;s++) { WinSys.x11.keycodes[s] = translateKeySyms(&k[(s - sMin) * wk],wk); } WinSys.x11.xlib.Free(k); // Create key tables.
        #define IA(n) WinSys.x11.xlib.InternAtom(WinSys.x11.display,n,0)
            WinSys.x11.UTF8_STRING=IA("UTF8_STRING"); WinSys.x11.WM_PROTOCOLS=IA("WM_PROTOCOLS"); WinSys.x11.WM_DELETE_WINDOW=IA("WM_DELETE_WINDOW"); WinSys.x11.NET_SUPPORTED =IA("_NET_SUPPORTED"); WinSys.x11.NWM_ICON=IA("_NET_WM_ICON"); WinSys.x11.NWM_PING=IA("_NET_WM_PING"); WinSys.x11.NWM_NAME=IA("_NET_WM_NAME"); WinSys.x11.MOTIF_WM_HINTS=IA("_MOTIF_WM_HINTS");
        #undef IA
        Atom* sa = NULL; const unsigned long ac = WinSysGetWindowPropertyX11(WinSys.x11.root,WinSys.x11.NET_SUPPORTED,4,(u8**)&sa);
        #define GA(name) getAtomIfSupported(sa, ac, name)
            WinSys.x11.NWM_STATE=GA("_NET_WM_STATE"); WinSys.x11.NWM_STATE_FULLSCREEN=GA("_NET_WM_STATE_FULLSCREEN"); WinSys.x11.NWM_WINDOW_TYPE=GA("_NET_WM_WINDOW_TYPE"); WinSys.x11.NWM_WINDOW_TYPE_NORMAL=GA("_NET_WM_WINDOW_TYPE_NORMAL"); WinSys.x11.NET_ACTIVE_WINDOW=GA("_NET_ACTIVE_WINDOW");
        #undef GA
        if (sa) { WinSys.x11.xlib.Free(sa); } XcursorImage* native = ImageCreate(16,16); mset(native->pixels,0,256*sizeof(XcursorUInt)); native->xhot=native->yhot = 0; WinSys.x11.hiddenCursorHandle = ImageLoadCursor(WinSys.x11.display,native); ImageDestroy(native); PollMonitors();
    #endif
    threadCnt = vclampi(OS_GetNumThreads(),1,32); World.diffCbt = World.diffCyb = World.diffPuz = World.diffMis = 2; /*Default difficulties of normal*/ World.cursorPos_x = 680; World.cursorPos_y = 384; /*Internal mouse cursor representation (separate from OS cursor) starts at center*/ World.startLevel = 1 /*medical*/;
    FHandle f = OS_OpenReadonly("./Data/Config.ini"); char line[512]; // Load Config.ini
    while (sUpToEndLine(line,sizeof(line),f)) {
        char* s = data_parser_trim(line); if (*s == 0 || (s[0] == '/' && s[1] == '/')) { continue; } char* eq = StringFindFirstCharWithin(s, '='); if (!eq) { continue; } *eq = 0; char *key = data_parser_trim(s), *val = data_parser_trim(eq + 1);
        for (int i=0;i<CONFIG_CNT;++i) {
            if (sEqual(key,configTable[i].name)) {
                if (configTable[i].type == SETTING_U8) *( u8*)configTable[i].ptr = (u8)s2i32(val);
                else if (configTable[i].type == SETTING_U16) *(u16*)configTable[i].ptr = (u16)s2i32(val); 
                else if (configTable[i].type == SETTING_INPUT) { i32 unused = 0, targ = -1; for (int j=0;j<(i32)(sizeof(inputElements)/sizeof(inputElements[0]));++j) { if (!inputElements[j].name) continue; if (sEqual(val,inputElements[j].name)) { targ = j; break; } if (!unused && sEqual("UNUSED",inputElements[j].name)) { unused = j; } } *(u16*)configTable[i].ptr = (u16)(targ != -1 ? targ : unused); }
                break;
            }
        }
    }
    Sys_Settings.ScreenWidth = vmax(Sys_Settings.ScreenWidth,320); Sys_Settings.ScreenHeight = vmax(Sys_Settings.ScreenHeight,200); OS_Close(f); /*Get settings before setting window size.*/
    OS_Thread fontInitThr; OS_ThreadCreate(&fontInitThr,InitFontAtlasses_Thread,NULL);
    ClampWindowSizeToMonitor();
    WSWin* w=OS_Alloc(sizeof(WSWin)); w->decorated=1;
    window = VCreateWindow(w,Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight); PlaceWindowOnMonitor((int)Sys_Settings.CurrentMonitor); window->context.makeCurrent(window); ((WSWin*)window)->context.swapBuffers(((WSWin*)window)); SetGLContext_GetFunctionPointers();
    i32 major = 0, minor = 0; glGetIntegerv(0x821B/*GL_MAJOR_VERSION*/,&major); glGetIntegerv(0x821C/*GL_MINOR_VERSION*/,&minor); if (major < 4 || (major == 4 && minor < 3)) { DualLogError("Need OpenGL >= 4.3, got %d.%d\n",major,minor); OS_Exit(1); }
    glBlendFuncSeparate(0x302/*GL_SRC_ALPHA*/,0x303/*GL_ONE_MINUS_SRC_ALPHA*/,1,0x303/*GL_ONE_MINUS_SRC_ALPHA*/); glClearColor(0,0,0,1);
    DualLog("Window creation and GL context......................................................took %.6f secs\n",get_time() - game_start_time);
    double initShadersStart = get_time();
    CompileShaders();
    u32 tvaos[5],tvbos[5]; glGenVertexArrays(5,tvaos); glGenBuffers(5,tvbos); quadVAO=tvaos[0]; quadVBO=tvbos[0]; chunkVAO=tvaos[1]; textVAO=tvaos[2]; textVBO=tvbos[2]; debugLinesVAO=tvaos[3]; debugLinesVBO=tvbos[3]; wireLinesVAO=tvaos[4]; wireLinesVBO=tvbos[4]; 
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
    glBindVertexBuffer(0,textVBO,0,5*sizeof(float)); glBindBuffer(GL_ARRAY_BUFFER,textVBO);
    glBufferData(GL_ARRAY_BUFFER,TEXT_VTX_FLOATS * sizeof(float),NULL,GL_DYNAMIC_DRAW);
    glBindVertexArray(debugLinesVAO); glBindBuffer(GL_ARRAY_BUFFER,debugLinesVBO);
    glBufferData(GL_ARRAY_BUFFER,MAX_LINEVRTS * sizeof(WireVrt),NULL,GL_DYNAMIC_DRAW);
    lineVerts = (WireVrt*)OS_Alloc((size_t)MAX_LINEVRTS * sizeof(WireVrt));
    glVertexAttribFormat(0,3,GL_FLOAT,GL_FALSE,__builtin_offsetof(WireVrt,x)); glVertexAttribBinding(0,0); glEnableVertexAttribArray(0);
    glVertexAttribFormat(1,4,GL_FLOAT,GL_FALSE,__builtin_offsetof(WireVrt,r)); glVertexAttribBinding(1,0); glEnableVertexAttribArray(1);
    glBindVertexBuffer(0,debugLinesVBO,0,sizeof(WireVrt));
    glBindVertexArray(wireLinesVAO); glBindBuffer(GL_ARRAY_BUFFER,wireLinesVBO);
    glBufferData(GL_ARRAY_BUFFER,MAX_WIRELINE_QUADS*6*sizeof(WireVrt),NULL,GL_DYNAMIC_DRAW);
    wireVerts = (WireVrt*)OS_Alloc((size_t)MAX_WIRELINE_QUADS*6*sizeof(WireVrt));
    glVertexAttribFormat(0,3,GL_FLOAT,GL_FALSE,__builtin_offsetof(WireVrt,x)); glVertexAttribBinding(0,0); glEnableVertexAttribArray(0);
    glVertexAttribFormat(1,4,GL_FLOAT,GL_FALSE,__builtin_offsetof(WireVrt,r)); glVertexAttribBinding(1,0); glEnableVertexAttribArray(1);
    glBindVertexBuffer(0,wireLinesVBO,0,sizeof(WireVrt));
    glGenVertexArrays(1,&psysquadVAO); glGenBuffers(1,&psysquadVBO);
    static const float quadVerts[16]={-1.0f,-1.0f,0.0f,0.0f,1.0f,-1.0f,1.0f,0.0f,-1.0f,1.0f,0.0f,1.0f,1.0f,1.0f,1.0f,1.0f,}; 
    glBindVertexArray(psysquadVAO); glBindBuffer(GL_ARRAY_BUFFER,psysquadVBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(quadVerts),quadVerts,GL_STATIC_DRAW);
    glVertexAttribFormat(0,2,GL_FLOAT,GL_FALSE,0);               glVertexAttribBinding(0,0); glEnableVertexAttribArray(0);
    glVertexAttribFormat(1,2,GL_FLOAT,GL_FALSE,2*sizeof(float)); glVertexAttribBinding(1,0); glEnableVertexAttribArray(1);
    glBindVertexBuffer(0,psysquadVBO,0,4 * sizeof(float));
    OS_ThreadJoin(&fontInitThr);
    GenerateAndBindTexture(&fontAtlasTex,0x8229,FONT_ATLAS_SIZE,FONT_ATLAS_SIZE,0x1903,GL_UNSIGNED_BYTE,0x2601,fontBmp1);
    GenerateAndBindTexture(&fontAtlasTexStopD,0x8229,FONT_ATLAS_SIZE2,FONT_ATLAS_SIZE2,0x1903,GL_UNSIGNED_BYTE,0x2601,fontBmp2);
    glUseProgram(textSP); glUniform1i(1,2);
    GenerateAndBindTexture(&inputUIID, GL_RGBA8, UI_W, UI_H, GL_RGBA, GL_UNSIGNED_BYTE, 0x2600/*GL_NEAREST*/, NULL);/*UI Fixed Size Raster*/ glGenFramebuffers(1, &uiFBO); glBindFramebuffer(GL_FRAMEBUFFER, uiFBO); glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, inputUIID);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,inputUIID,0); u32 drawBuffersUI[] = {GL_COLOR_ATTACHMENT0}; glDrawBuffers(1,drawBuffersUI); glBindImageTexture(0,inputUIID,0,GL_FALSE,0,GL_READ_WRITE,GL_RGBA8);/* UI Rendered Color*/
    RenderLoading("Loading...");
    float* m = shadowmapsPerspectiveProjection, lightRangeMax = 15.36f; float viewRange = (lightRangeMax - 0.02f);
    m[0] = 1.0f; m[1] = 0.0f; m[2] = 0.0f; m[3] = 0.0f; m[4] = 0.0f; m[5] = 1.0f; m[6] = 0.0f; m[7] = 0.0f; m[8] = 0.0f; m[9] = 0.0f; m[10] = -(lightRangeMax + 0.02f) / viewRange; m[11] = -1.0f; m[12] = 0.0f; m[13] = 0.0f; m[14] = -2.0f * lightRangeMax * 0.02f / viewRange; m[15] = 0.0f;
    for (int i=0;i<23;++i) { g_scf_long[0][i] = g_scf_long[2][i] = g_scf_long[4][i] = g_sfc_long_024[i]; }
    glGenFramebuffers(1,&gBufferFBO); ChangeFullScreenWindowed(false); window->context.swapInterval((i32)Sys_Settings.Vsync); 
    DualLog("GL shaders and buffer setup.........................................................took %.6f secs\n",get_time() - initShadersStart);
    double initSSBOStart = get_time();
    glBindFramebuffer(GL_FRAMEBUFFER,gBufferFBO); u32 drawBuffers[] = {GL_COLOR_ATTACHMENT0,GL_COLOR_ATTACHMENT1,GL_COLOR_ATTACHMENT2}; glDrawBuffers(3,drawBuffers);
    float mat[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1}; mcpy(&modelMatrices[0],mat,16 * sizeof(float)); 
    matricesBufferID  = MakeSSBO(&matricesBufferID,1,INSTANCE_COUNT*16*sizeof(float),modelMatrices,GL_STATIC_DRAW);
    voxListCntsID     = MakeSSBO(&voxListCntsID,2,VOXEL_COUNT*sizeof(u32),NULL,GL_STATIC_DRAW);
    voxelLightListsID = MakeSSBO(&voxelLightListsID,3,VOXEL_COUNT*MAX_LIGHTS_PER_VOXEL*sizeof(u32),NULL,GL_STATIC_DRAW);
    lightsID          = MakeSSBO(&lightsID,4,LIGHT_COUNT*sizeof(Light),NULL,GL_STATIC_DRAW);
    if (Sys_Settings.Shadows) { CreateShadowBuffers();/*5,6*/ InitSunShadowmap(); }
    cellVisibleDataID = MakeSSBO(&cellVisibleDataID,7,ARRSIZE*sizeof(u32),NULL,GL_STATIC_DRAW);
    texPalID          = MakeSSBO(&texPalID,8,MAX_UNIQUE_COLORS*sizeof(u32),NULL,GL_STATIC_DRAW);
    texPalOfsID       = MakeSSBO(&texPalOfsID,9,MAXTEX*sizeof(u32),NULL,GL_STATIC_DRAW);
    psysInstancesID   = MakeSSBO(&psysInstancesID,10,MAX_PARTICLES*sizeof(GpuPartInst),NULL,GL_DYNAMIC_DRAW);
    psysTrailsID      = MakeSSBO(&psysTrailsID,11,MAX_TRAIL_SEGS*sizeof(TrlSegInst),NULL,GL_DYNAMIC_DRAW);
    colorBufferID     = MakeSSBO(&colorBufferID,12,MAX_TOTAL_PIXELS*sizeof(u8),NULL,GL_STATIC_DRAW);
    textureOffsetsID  = MakeSSBO(&textureOffsetsID,14,MAXTEX * sizeof(u32),NULL,GL_STATIC_DRAW);
    textureSizesID    = MakeSSBO(&textureSizesID,15,MAXTEX*2*sizeof(i32),NULL,GL_STATIC_DRAW);
    glUseProgram(shadowmapsSP); glUniform1ui( 9, SHADOW_MAP_SIZE); // Set permanent uniforms on shaders (keeps c in sync with glsl for constants)
    glUseProgram(chunkSP);      glUniform1ui(21, SHADOW_MAP_SIZE); glUniform1f(22, (float)SHADOW_MAP_SIZE); glUniform1ui(23, LIGHT_COUNT); glUniform1ui(24, (u32)MAX_LIGHTS_PER_VOXEL); glUniform1ui(11, SHADOW_MAP_SIZE * SHADOW_MAP_SIZE); glUniform1i(29,6); glUniform1i(31,9); glUniform1i(36,10);
    glUseProgram(ssrSP);        glUniform1i(5,3); glUseProgram(rtSP); glUniform1i(5,3);
    for (int q=0;q<5;++q) { glGenQueries(5,gpuQ[q]); }
    DualLog("SSBO and shadow buffer setup........................................................took %.6f secs\n",get_time() - initSSBOStart);
    RenderLoading("Loading textures..."); LoadTextures(); RenderLoading("Loading models..."); ParseAllModels(); bool playIntroAtBoot = !OS_FileExists("./Data/introdone.dat");
    if (playIntroAtBoot) { FHandle introdone=OS_OpenWriteonly("./Data/introdone.dat"); OS_Close(introdone); }
    double initAmBiomStart = World.absoluteTime = get_time();
    GenerateAndBindTexture(&amTexId,GL_RGBA8,AM_W,AM_H,GL_RGBA,GL_UNSIGNED_BYTE,0x2601/*GL_LINEAR*/,NULL);               glGenFramebuffers(1,&amFBO);     glBindFramebuffer(GL_FRAMEBUFFER,amFBO);     glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,amTexId,0);
    GenerateAndBindTexture(&amFullTexId,GL_RGBA8,AM_FULL_W,AM_FULL_H,GL_RGBA,GL_UNSIGNED_BYTE,0x2601/*GL_LINEAR*/,NULL); glGenFramebuffers(1,&amFullFBO); glBindFramebuffer(GL_FRAMEBUFFER,amFullFBO); glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,amFullTexId,0);
    GenerateAndBindTexture(&biomTexId,GL_RGBA8,BIOM_GRAPH_W,BIOM_GRAPH_H,GL_RGBA,GL_UNSIGNED_BYTE,GL_LINEAR,NULL);       glGenFramebuffers(1,&biomFBO);   glBindFramebuffer(GL_FRAMEBUFFER,biomFBO); glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,biomTexId,0);
    bioMonitor.beatFinished=get_time() + 0.5; bioMonitor.backgroundColor=(Color){0.2f,0.2f,1.0f,0.01f}; bioMonitor.ergColor=(Color){0,0.5f,1.0f,1.0f}; bioMonitor.chiColor=(Color){.7f,0,1.f,1.f}; bioMonitor.ecgColor=(Color){1.f,0,0,1.f}; bioMonitor.min[BIOM_ERG]=0; bioMonitor.min[BIOM_CHI]=-2.f; bioMonitor.min[BIOM_ECG]=-1.f; bioMonitor.max[BIOM_ERG]=1.f; bioMonitor.max[BIOM_CHI]=2.f; bioMonitor.max[BIOM_ECG]=1.f; BioMonitorClearGraphs();
    DualLog("Automap and biomonitor buffers......................................................took %.6f secs\n",get_time() - initAmBiomStart);
    NewGame();
    if (Sys_Settings.Reflections >= 1u) { EnsureReflectionResources(); }
    double initAudioStart = get_time(); InitAudio(); synth_set_reverb_preset(0); 
    World.menuActive = true; //COMMENT for immediate into game
    if (playIntroAtBoot && World.menuActive) ChangeMenuPage(MPG_INTRO); else if (World.menuActive) { PlayMenuMusic(); } else { PlayTrack(TT_Walking, MT_Walking); }
    DualLog("Audio and Menu init.................................................................took %.6f secs\n",get_time() - initAudioStart);
    DualLog("Scratch arena total peak: %u MB, ", (u32)(scratch_peak >> 20)); OS_Free(scratch_base,SCRATCH_ARENA_SIZE); // Free the initialization scratch arena.
    DebugRAMPeak(); DualLog(".......Game Initialization TOTAL %.6f secs\n",get_time() - game_start_time); DebugRAMActual("init"); DualLog("\n");
}
void RessurectionTextUpdate() { static int lastStage = -1; double fin = World.invP1.ressurectingFinished; if (fin <= World.pauseRelativeTime) { lastStage = -1; return; } double el = World.pauseRelativeTime - (fin - 3.0); int stage = (int)(el * 8.0 / 3.0); if (stage < 0) stage = 0; if (stage > 7) stage = 7; if (stage != lastStage) { lastStage = stage; CenterStatusPrint("%s", Sys_Text.stringTable[860 + stage]); } }
void Physics(float),UpdateAnims(),UpdateAudio(),AudioUpdateMix(),PSys_Update(float),BioMonitorUpdate();
i32 main() {
    InitalizeEnvironment();
    while(1) {
        if (queuedLevelToLoad != 255u) { LoadLevel(queuedLevelToLoad,queuedLevelPos); queuedLevelToLoad = 255u; continue; }
        double curtime = get_time(); raycastCalls = 0; raycastMs = 0.0; World.dt = World.current_time < 0.001f ? 0.000f : vmax(curtime - World.current_time, 0.0); World.absoluteTime += World.dt; World.current_time = curtime;
        World.painStaticAlpha = vmax(0.0f, World.painStaticAlpha - (float)World.dt * 2.0f);
        World.empStaticAlpha = vmax(0.0f, World.empStaticAlpha - (float)World.dt * 4.0f);
        if (!World.paused && !World.menuActive) { if (World.pauseRelativeTime < 0.001f) { World.pauseRelativeTime = World.last_physics_time = curtime; } World.pauseRelativeTime += World.dt * World.timeScale; /*Reflex slow-time at 0.25x while the patch is active, mouse-look stays raw*/ }
        double input_start = get_time(); InputProcessing();/*Before anims and physics to allow them to respond immediately.*/ UpdateAnims();/*Before physics to allow model swap out to affect physics state immediately.  Before rendering to affect shadowmaps immediately.*/
        prePhys = get_time() - input_start;
        if (!World.paused && !World.menuActive) { double ps = get_time(); float dt = (float)vclamp((World.pauseRelativeTime - World.last_physics_time), 0.0005, 0.1); World.last_physics_time = World.pauseRelativeTime; World.dt = dt; Physics(dt); physTime = get_time() - ps; } else physTime=0.0;
        double gameT_start = get_time(); ModUpdate();/*After physics so mod/gamecode can modify velocities before next frame.*/ BioMonitorUpdate(); RessurectionTextUpdate(); if (!World.paused && !World.menuActive){ PSys_Update(World.dt); }/*Particles*/ UpdateAudio();/*Updates states, music triggers, ambients, etc.*/ gameTime = get_time() - gameT_start;
        if (likely((!World.paused && !World.menuActive) || Cheats.editMode)) { UpdateInstanceMatrix4x4s(); /*Before camviews so camview shadows render same as main pass*/ }
        drawCalls = uiDrawCalls = shadDrawCalls = vertsRendered = 0;
        UpdateSensaroundCamViews(); RenderCameraViews();/*Before main Render() so that tv screens are up to date*/ if(likely(!World.paused && !World.menuActive)) { CullCore(); } /*Before automap and audio update due to audio occlusion and automap FoW discovery*/ AudioUpdateMix();/*Actually mixes all the audio for output.*/ AutomapTick();
        Render(false/*!camview*/,0u);
        if (GetKeyPressed(40)/*Screenshot*/ && World.current_time > World.screenshotTimeout) { Screenshot(); }
        for(i32 i=0;i<MAX_KEYS;++i){ Sys_Input.keyStates[i].pressed = false; } /*Clear input states, can't mset as we want to preserve down(held) state*/
        for (i32 i=0;i<MAX_MOUSE_BUTTONS;i++) { Sys_Input.mouseButtons[i].pressed = false; }
        Sys_Input.scrollDelta = 0.0; World.currentMouse_dx = World.currentMouse_dy = 0; globalframe++; World.cpuTime = get_time() - World.current_time; // Measure time over everything this frame before GPU swap buffers for diagnostic text.
        if (globalframe > 4) {
            u8 r = (gpuQFrame + 1) % 5; u64 v;
            glGetQueryObjectui64v(gpuQ[r][0],0x8866/*GL_QUERY_RESULT*/,&v); World.gpuShadowMs = (double)v * 0.000001; glGetQueryObjectui64v(gpuQ[r][1],0x8866/*GL_QUERY_RESULT*/,&v); World.gpuPreMs = (double)v * 0.000001;
            glGetQueryObjectui64v(gpuQ[r][2],0x8866/*GL_QUERY_RESULT*/,&v); World.gpuMainMs   = (double)v * 0.000001; glGetQueryObjectui64v(gpuQ[r][3],0x8866/*GL_QUERY_RESULT*/,&v); World.gpuSsrMs = (double)v * 0.000001;
            glGetQueryObjectui64v(gpuQ[r][4],0x8866/*GL_QUERY_RESULT*/,&v); World.gpuCompMs   = (double)v * 0.000001; World.gpuFrameMs = World.gpuShadowMs + World.gpuPreMs + World.gpuMainMs + World.gpuSsrMs + World.gpuCompMs;
        }
        gpuQFrame=(gpuQFrame+1)%5;
        ((WSWin*)window)->context.swapBuffers(((WSWin*)window));
        CHECK_GL_ERROR();
        static u32 dbgGameFrame = 0; if (!World.menuActive && !World.paused) { dbgGameFrame++; } static const u32 dbgFrm[] = {4, 100, 200, 500, 1000}; static const char* dbgLbl[] = {"   4"," 100"," 200"," 500","1000"};
        for (int d=0;d<5;d++) { if (dbgGameFrame == dbgFrm[d]) { DualLog("Frm %s timings:::GPU ms::All:%.2f Shad:%.2f Pre:%.2f Main:%.2f SSR:%.2f Comp:%.2f | CPU ms::Shad:%.3f Phys:%.3f Subs:%u Rend:%.3f PrePhys:%.3f Logic:%.3f Raycasts:%.3f(%u) | Shadmap draws:%u\n",dbgLbl[d],World.gpuFrameMs,World.gpuShadowMs,World.gpuPreMs,World.gpuMainMs,World.gpuSsrMs,World.gpuCompMs,shadowTime*1000,physTime*1000,World.substeps,renderTime*1000,prePhys*1000,gameTime*1000,raycastMs*1000,raycastCalls,shadDrawCalls); } }
    }
    return 0;
}
