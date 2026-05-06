// voxen.c - A realtime OpenGL 4.3+ Game Engine for Citadel: The System Shock Fan Remake
#include "os.h" // Operating System calls shim layer.
#include "gl.h"
GLFWwindow* window;
#define MOD_INTEROP_ENGINE
#if defined(LINUX)
//     #define DEBUG_RAM_OUTPUT // Debug and Compile Flags
#endif
#include "common.h"
#include "interop.h"
#include "Shaders/shaders.h"
#define MAX_KEYS 512
#define MAX_MOUSE_BUTTONS 8
#define VERTEX_ATTRIBUTES_SIZE 16 // Was 32ls
#define TEXT_BUFFER_SIZE 1024
#define FONT_ATLAS_SIZE 4672
#define MAX_GLYPHS 4096
#define MAX_CHANNELS 48 // Max concurrent sounds, must keep track of for volume setting
typedef struct {bool down,pressed,released;} KeyState;
typedef struct {
	double last_mouse_x,last_mouse_y,scrollDelta;
	KeyState keyStates[MAX_KEYS],mouseButtons[MAX_MOUSE_BUTTONS],joystickButtons[16][16],joystickHats[5]; // What can I say, I'm a man of many hats. ^^D
    i32 currentMouse_dx,currentMouse_dy;
	bool window_has_focus,ignore_next_mouse_delta,lastUse,isCapsLockOn,joystickPresent[16];
} InputSystem;
typedef struct { Vector3 normal; float d; } FrustumPlane;
typedef struct StbiArena { u8*base,*cursor,*end; } StbiArena;
typedef struct {
    u32 inputImageID,inputUIID,inputDepthID,inputWorldPosID,inputSpecID,inputNormalID,gBufferFBO,uiFBO,outputImageID;
    u32 depthPrepassShaderProgram,chunkShaderProgram,vao_chunk,uiShaderProgram,debugUnlitShaderProgram;
    u32 shadowmapsShaderProgram,shadowmapsClearShaderProgram,shadowMapSSBO,shadowMapsIndirectionID;
    u32 ssrShaderProgram,imageBlitShaderProgram,quadVAO,quadVBO,textShaderProgram,textVAO,textVBO;
    u32 debugLinesVAO,debugLinesVBO,matricesBufferID,cellVisibleDataID;
    u32 colorBufferID,texturePalettesID,texturePaletteOffsetsID,textureOffsetsID,textureSizesID;
    u32 lightsID,voxelLightListCountsID,voxelLightListsID,voxelUpdateShaderProgram,shadowViewProjID;
    u32 vbos[MODEL_IDX_MAX],tbos[MODEL_IDX_MAX];
} RenderSystem;
#define STBI_ARENA_SIZE 16*1024*1024
u8 queuedLevelToLoad = 255u; static float berserkSeedTime,cam_pitch,cam_yaw=90.0f,cam_roll,rasterPerspectiveProjection[16],shadowmapsPerspectiveProjection[16],lightView[LIGHT_COUNT][6][4][4],lightViewProj[LIGHT_COUNT][6][16];
float modelMatrices[INSTANCE_COUNT*16];
bool mouseMovementThisFrame,returnToPause=false,fovSliderActive=false,gammaSliderActive=false,masterVolumeSliderActive=false,musicVolumeSliderActive=false,messageVolumeSliderActive=false,sfxVolumeSliderActive=false,enteringPlayerName=false;
u8 currentPlayerNameLength=0; i8 currentMenuItem=0, currentMenuTab=0, menuItemCount=4, menuTabCount=1;
static int num_parse_threads = 0;
#define CHECK_GL_ERROR() do { u32 err = glGetError(); if (err != 0) DualLogError("GL Error at %s:%d: %d\n", __FILE__, __LINE__, err); } while(0)
#define SHADOW_MAP_SIZE 128u
#define MAX_SHADOWMAPS 256u
#define MAX_LIGHTS_PER_VOXEL 64
#define NEAR_PLANE (0.02f)
GlobalContext Sys_Global = {0}; TextSystem Sys_Text; InputSystem Sys_Input; CheatsSystem Sys_Cheats = {.god=false,.noclip=true,.showLocation=true,.showFPS=true,.editMode=true}; RenderSystem Sys_Render; SystemUI Sys_UI;
SettingsSystem Sys_Settings = { // Potato defaults so initial state is good on first run for potatoes (e.g. won't crash for out of VRAM, or won't take 5min to init).
    .InputCodeSettings = {
        5,  /* Forward    = F */     0,/* Strafe Left= A */         18,/* Backpedal  = S */        3,/* Strafe Right= D */       100,/* Jump    = SPACE */      2,/* Crouch   = C        */ 23,/* Prone     = X */ 16,/* Lean Left = Q  */
        4,  /* Lean Right = E */    45,/* Sprint     = LEFT SHIFT */38,/* Turn Left  = LF ARROW */39,/* Turn Right  = RT ARROW */ 36,/* Look Up = UP ARROW */  37,/* Look Down= DN ARROW */ 20,/* Recent Log= U */ 26,/* Biomonitor= 1  */
        27, /* Sensaround = 2 */    28,/* Lantern    = 3 */         29,/* Shield     = 4 */       30,/* Infrared    = 5 */        31,/* Email   = 6 */         32,/* Booster  = 7        */ 33,/* Jumpjets  = 8 */ 56,/* Attack    = LMB*/
        57, /* Use        = RMB */  99,/* Menu/Back  = ESCAPE */    97,/* Toggle Mode= TAB */     17,/* Reload      = R */       128,/* Weapon += MWHEEL + */ 129,/* Weapon - = MWHEEL - */  6,/* Grenade   = G */ 19,/* Grenade + = T  */
        131,/* Grenade -  = B */    21,/* Ammo Type  = V */          9,/* Patch Use  = J */        8,/* Patch +     = I */       132,/* Patch - = , */         12,/* Full Map = M        */ 21,/* Swim Up   = V */  2,/* Swim Down = C  */
        103,/* Console    = `/~ */ 102/* Screenshot  = F12 */},
    .ScreenWidth=800u,.ScreenHeight=600u,.Fullscreen=0u,.FOV=65u,.Brightness=50u,.Gamma=50u,.FXAA=0u,.Shadows=0u,.Reflections=0u,.Vsync=0u,.ModelDetail=0u,.CurrentMonitor=0u,
    .GI=0u,.SpeakerMode=1u,.Reverb=0u,.VolumeMaster=100u,.VolumeMusic=25u,.VolumeMessage=75u,.VolumeEffects=100u,.Language=0u,.DynamicMusic=1u,.Footsteps=1u,.InvertLook=0u,
    .InvertCyberspaceLook=0u,.QuickItemPickup=0u,.QuickReloadWeapons=0u,.MouseSensitivity=10u,.NoShootMode=0u,.HeadBob=1u,.SSR_RES=4u};/*Ratio is (1 / SSR_RES) * res*/
Light lights[LIGHT_COUNT]; LightAnimation lanims[LIGHT_COUNT];
FrustumPlane lightFrustumPlanes[LIGHT_COUNT][6][6],playerFrustumPlanes[6];
u16 editModeSelection,editModeTestEntityDefinition=0; // Test instance and its model index
typedef struct { double shadowTime; u32 shadowmapIndirectionList[LIGHT_COUNT]; float shadDotThresh; } VoxenShadowSystem;
VoxenShadowSystem voxen_Shadow_System;
u16 loadedTexturesMaxIndex;
bool doubleSidedTexture[MAX_VALID_TEXTURE],transparentTexture[MAX_VALID_TEXTURE];
u32 drawCallsRenderedThisFrame,uiImageDrawCallsRenderedThisFrame,shadowDrawCallsRenderedThisFrame,verticesRenderedThisFrame,drawCallsNormal;
static const u8 Mpg_FrontPage=0,Mpg_Singleplayer=1,Mpg_Multiplayer=2,Mpg_NewGame=3,Mpg_Load=4,Mpg_Options=5,Mpg_Save=6,Mpg_IntroVideo=7,Mpg_CreditsVideo=8;
u8 currentMenuPage = Mpg_FrontPage;
static bool resDropdownOpen = false; static int resDropdownCount=0,resSelectedIdx=0;
typedef struct {int w,h;} ResMode;
static ResMode resModes[8];
typedef struct { Vector3 position; Quaternion rotation; u8 fov; u16 width,height; float near,far,finished; bool visible; } CamView;
CamView camViews[64]; u32 camViewTextures[64]; u8 camViewCount = 0; // Max is 8 cam views on level 8 + 3 sensaround views = 11.
OsFileHandle console_log_file=0;
static inline __attribute__((always_inline)) i32 PosGetCellCoordX(float pos_x) { return (u16)clamp((i32)vfloor((pos_x - Sys_Global.worldMin_x + CELLXHALF) / CELL_SIZE), 0, WORLDX_0BASED); }
static inline __attribute__((always_inline)) i32 PosGetCellCoordZ(float pos_z) { return (u16)clamp((i32)vfloor((pos_z - Sys_Global.worldMin_z + CELLXHALF) / CELL_SIZE), 0, WORLDX_0BASED); }
static void DualLogMain(const char *prefix, const char *fmt, va_list args) { // Logs both to log file and console, usage same as printf
    char buf[4096]; va_list copy; __builtin_va_copy(copy,args); StringFormatV(buf,sizeof(buf),fmt,copy); __builtin_va_end(copy);
    #ifdef WINDOWS // Write to console (stdout / stderr)
        OsFileHandle out = GetStdHandle((prefix && prefix[0] == '\033') ? (DWORD)-12 : (DWORD)-11);
        if (prefix) OS_RawWrite(out, prefix, GetStringLength(prefix));
        OS_RawWrite(out, buf, GetStringLength(buf));
    #else // Linux - write to stdout (fd 1) or stderr (fd 2)
        OsFileHandle out = (prefix && prefix[0] == '\033') ? 2 : 1;  // use stderr for colored warnings/errors
        if (prefix) { OS_RawWrite(out, prefix, GetStringLength(prefix)); OS_RawWrite(out,"\033[0m ", 5); }
        OS_RawWrite(out, buf, GetStringLength(buf));
    #endif
    if (console_log_file != OS_INVALID_HANDLE) { // Write to console_log_file
        if (prefix) { OS_Write(console_log_file, prefix, GetStringLength(prefix), "console.log"); OS_Write(console_log_file,"\033[0m ",5,"console.log"); }
        OS_Write(console_log_file, buf, GetStringLength(buf), "console.log");
    }
}

ENGINE_TO_MOD void DualLog(const char* fmt, ...) { va_list args; __builtin_va_start(args,fmt); DualLogMain(NULL,fmt,args); __builtin_va_end(args); }
ENGINE_TO_MOD void DualLogWarn(const char* fmt, ...) { va_list args; __builtin_va_start(args,fmt); DualLogMain("\033[1;38;5;208mWARN:",fmt,args); __builtin_va_end(args); }
ENGINE_TO_MOD void DualLogError(const char* fmt, ...) { va_list args; __builtin_va_start(args,fmt); DualLogMain("\033[1;31mERROR:",fmt,args); __builtin_va_end(args); }
#include "helpers.c"
#include "glfw.c"
#include "console.c"
#include "textures.c"
#include "models.c"
#include "culling.c"
#include "ray.c"
#include "physics.c"
#include "audio.c"
static inline __attribute__((always_inline)) void LogShaderError(u32 s, const char* name) { char er[512]; glGetShaderInfoLog(s,512,NULL,er); DualLogError("%s Compilation Failed: %s\n",name,er); OS_Exit(1); }
static inline __attribute__((always_inline)) u32 CompileShader(u32 type, const char* source, const char* name) { u32 s = glCreateShader(type); glShaderSource(s,1,&source,NULL); glCompileShader(s); i32 ok; glGetShaderiv(s,0x8B81/*GL_COMPILE_STATUS*/,&ok); if (!ok) LogShaderError(s,name); return s; }
static inline __attribute__((always_inline)) u32 LinkProgram(u32* s, i32 num, const char* name) { u32 p = glCreateProgram(); for (i32 i=0;i<num;++i) { glAttachShader(p,s[i]); } glLinkProgram(p); i32 ok; glGetProgramiv(p,0x8B82/*GL_LINK_STATUS*/,&ok); if (!ok) LogShaderError(p,name); return p; }
u32 CompileAnyShader(const char* vsrc, const char* src, const char* name) { return (vsrc) ? LinkProgram((u32[]){CompileShader(0x8B31/*GL_VERTEX_SHADER*/,vsrc,name),CompileShader(0x8B30/*GL_FRAGMENT_SHADER*/,src,name)},2,name) : LinkProgram((u32[]){CompileShader(0x91B9/*GL_COMPUTE_SHADER*/,src,name)},1,name); }
void CompileShaders(void) {
    Sys_Render.depthPrepassShaderProgram= CompileAnyShader(depthPrepassVertSrc,depthPrepassFragSrc,"Depth Prepass");
    Sys_Render.chunkShaderProgram       = CompileAnyShader(vertSrc,fragSrc,"Main");
    Sys_Render.uiShaderProgram          = CompileAnyShader(vertUISrc,fragUISrc,"UI");
    Sys_Render.debugUnlitShaderProgram  = CompileAnyShader(debugUnlitVertSrc,debugUnlitFragSrc,"Debug Unlit");
    Sys_Render.shadowmapsShaderProgram  = CompileAnyShader(shadowmapVertSrc,shadowmapFragSrc,"Shadowmaps");
    Sys_Render.textShaderProgram        = CompileAnyShader(textVertSrc,textFragSrc,"Text");
    Sys_Render.imageBlitShaderProgram   = CompileAnyShader(quadVertSrc,quadFragSrc,"Image Blit");
    Sys_Render.ssrShaderProgram            = CompileAnyShader(NULL,ssrComputeSrc,"SSR");
    Sys_Render.voxelUpdateShaderProgram    = CompileAnyShader(NULL,voxelUpdateComputeSrc,"Voxel Update");
    Sys_Render.shadowmapsClearShaderProgram= CompileAnyShader(NULL,shadowmapsClearComputeSrc,"Shadowmaps Clear");
}

u32 SetupSSBO(u32* id, u32 bindx, size_t sz, const void* d, u32 typ) { glGenBuffers(1,id); glBindBuffer(GL_SSBO,*id); glBufferData(GL_SSBO,sz,d,typ); glBindBufferBase(GL_SSBO,bindx,*id); return *id; }
void mat4_lookat_from(float* m, Quaternion* camRotation, Vector3 eye) { // Kept around for light views for shadowmap cubemap faces.
    float x = camRotation->x, y = camRotation->y, z = camRotation->z, w = camRotation->w;
    float x2 = x * x, y2 = y * y, z2 = z * z;
    float xy = x * y, xz = x * z, yz = y * z;
    float wx = w * x, wy = w * y, wz = w * z;
    Vector3 right   = { 1.0f - 2.0f * (y2 + z2),        2.0f * (xy + wz),        2.0f * (xz - wy) };  // X+ (right)
    Vector3 up      = {        2.0f * (xy - wz), 1.0f - 2.0f * (x2 + z2),        2.0f * (yz + wx) };  // Y+ (up)
    Vector3 forward = {        2.0f * (xz + wy),        2.0f * (yz - wx), 1.0f - 2.0f * (x2 + y2) };  // Z+ (forward)
    m[0]  = right.x;   m[1]  = up.x;   m[2]  = -forward.x;// m[3]  = 0.0f;
    m[4]  = right.y;   m[5]  = up.y;   m[6]  = -forward.y;// m[7]  = 0.0f;
    m[8]  = right.z;   m[9]  = up.z;   m[10] = -forward.z;// m[11] = 0.0f;
    m[12] = -dot_vector3(right, eye); m[13] = -dot_vector3(up, eye); m[14] = dot_vector3(forward, eye); m[15] = 1.0f;
}

__attribute__((pure,always_inline)) bool SphereInFrustum(FrustumPlane* ps, Vector3 c, float radius) { for (int i=0;i<6;++i) { if ((dot_vector3(ps[i].normal,c) + ps[i].d) < -radius) return false; } return true; }
void ExtractFrustumPlanes(float* m, FrustumPlane* ps) {
    ps[0].normal.x = m[3] + m[0]; ps[0].normal.y = m[7] + m[4]; ps[0].normal.z = m[11] + m[8];  ps[0].d = m[15] + m[12]; // Left
    ps[1].normal.x = m[3] - m[0]; ps[1].normal.y = m[7] - m[4]; ps[1].normal.z = m[11] - m[8];  ps[1].d = m[15] - m[12]; // Right
    ps[2].normal.x = m[3] + m[1]; ps[2].normal.y = m[7] + m[5]; ps[2].normal.z = m[11] + m[9];  ps[2].d = m[15] + m[13]; // Bottom
    ps[3].normal.x = m[3] - m[1]; ps[3].normal.y = m[7] - m[5]; ps[3].normal.z = m[11] - m[9];  ps[3].d = m[15] - m[13]; // Top
    ps[4].normal.x = m[3] + m[2]; ps[4].normal.y = m[7] + m[6]; ps[4].normal.z = m[11] + m[10]; ps[4].d = m[15] + m[14]; // Near
    ps[5].normal.x = m[3] - m[2]; ps[5].normal.y = m[7] - m[6]; ps[5].normal.z = m[11] - m[10]; ps[5].d = m[15] - m[14]; // Far
    for (int i=0;i<6;i++) {
        float len = magnitude_vector3(ps[i].normal); if (len > 1e-6f) { ps[i].normal.x /= len; ps[i].normal.y /= len; ps[i].normal.z /= len; ps[i].d /= len; } // Normalize (could use normalize_vector3 but need len for d term of FrustumPlane).
    }
}

ENGINE_TO_MOD void InitializeEntity(Entity* entry) { // Blank entity, no index yet, for initial list population or temporary Entity.
    entry->index = U16_MAX; // memset here would be harmful as only a handful of fields are the same.
    entry->entflags = (ENTFLAG_KINEMATIC | ENTFLAG_ACTIVE); // Zeroes the rest out.
    entry->modelIndex = MODEL_IDX_MAX;
    entry->layer = Layer_Default;
    entry->texIndex = entry->glowIndex = entry->specIndex = entry->normIndex = MAX_VALID_TEXTURE;
    entry->lodIndex  = MODEL_IDX_MAX;
    entry->camView = 255;
    entry->rotation = QUAT_IDENTITY;
    entry->scale.x = entry->scale.y = entry->scale.z = 1.0f;
    entry->collider = COLLIDER_TYPE_NONE;
    entry->colliderMeshIndex = MODEL_IDX_MAX;
    entry->tickTime = 0.35f;
    entry->mass = 1.0f;
    entry->angularDrag = 0.05f;
    entry->dynamicFriction = entry->staticFriction = 0.6f;
    entry->frictionCombine = entry->bounceCombine = PHYS_COMBINE_AVG;
    entry->volume = 1.0f;
}

Vector3 lightsNewPosition[LIGHT_COUNT];
ENGINE_TO_MOD i32 AddLight(Light* lit, LightAnimation* lanim) {
    i32 i = Sys_Global.loadedLights;
    Sys_Global.loadedLights++;
    if (Sys_Global.loadedLights >= LIGHT_COUNT) { DualLogError("Too many lights %u added in level %d!\n",i,Sys_Global.currentLevel); OS_Exit(1); }

    CopyMemoryFromBtoAForNBytes(&lights[i],lit,sizeof(Light));
    CopyMemoryFromBtoAForNBytes(&lanims[i],lanim,sizeof(LightAnimation));
    lightsNewPosition[i] = lit->pos;
    flag_setu32(&lights[i].lflags,LDIRTY,true);
    return i;
}

ENGINE_TO_MOD void TurnLightOff(u16 litIdx) { if (litIdx < Sys_Global.loadedLights) {flag_setu32(&lights[litIdx].lflags,LIGHTON,false);} }
bool alreadyReadLightOnOnce[LIGHT_COUNT] = {0};
ENGINE_TO_MOD void LoadFieldIntoLight(char* k, char* v, char* il, u32 ln, Light* lit, LightAnimation* lam, u16 lIdx) {
    char* br = StringFindFirstCharWithin(k,'[');
    if (br) {
        int i = parse_numberu32(br + 1,il,ln);
        if (i >= 0 && i < 32) { // "intervalSteps[" index 12 is 's', "intervalStepisLerping[" index 12 is 'i'
            if (k[12] == 's') lam->intervalSteps[i] = parse_float(v,il,ln);
            else              lam->stepIsLerping[i] = parse_float(v,il,ln);
        }
        return;
    }

    static const struct { const char* key; u16 offset; u8 type; } map[] = {
        {"currentStep",__builtin_offsetof(LightAnimation,currentStep),1},{"lerpValue",__builtin_offsetof(LightAnimation,lerpValue),0},
        {"intervalSteps.Length",__builtin_offsetof(LightAnimation,numIntervalSteps),1},{"intervalStepisLerping.Length",__builtin_offsetof(LightAnimation, numLerpSteps),1},
        {"localPosition.x",__builtin_offsetof(Light,pos.x),0},{"localPosition.y",__builtin_offsetof(Light,pos.y),0},{"localPosition.z",__builtin_offsetof(Light,pos.z),0},
        {"localRotation.x",__builtin_offsetof(Light,spotDir.x),0},{"localRotation.y",__builtin_offsetof(Light,spotDir.y),0},{"localRotation.z",__builtin_offsetof(Light,spotDir.z),0},{"localRotation.w",__builtin_offsetof(Light,spotDir.w),0},
        {"range",__builtin_offsetof(Light,range),0},{"spotAngle",__builtin_offsetof(Light,spotAng),0},{"minIntensity",__builtin_offsetof(Light,minIntensity),0},{"maxIntensity",__builtin_offsetof(Light,maxIntensity),0},
        {"color.r",__builtin_offsetof(Light,col.r),0},{"color.g",__builtin_offsetof(Light,col.g),0},{"color.b",__builtin_offsetof(Light,col.b),0}
    };

    for (int i = 0; i < (int)(sizeof(map)/sizeof(map[0])); i++) {
        if (StringsEqual(k, map[i].key)) { // Types: 0 = float, 1 = u8.  Check key prefix to decide if pointing at 'lit' or 'lam'
            void* dest = (k[0] == 'l' && k[1] == 'o') ? (void*)lit : (void*)lam;
            if (k[0] == 'r' || k[0] == 's' || k[0] == 'm' || k[0] == 'c') {
                if (k[1] != 'u') dest = (void*)lit; // range, spot, max, color (not currentStep)
            }
            
            char* ptr = (char*)dest + map[i].offset;
            if (map[i].type == 0) *(float*)ptr = parse_float(v,il,ln);
            else                  *(u8*)ptr = parse_numberu8(v,il,ln);
            return;
        }
    }

    if (StringsEqual(k,"intensity")) lit->intensity = lit->maxIntensity = parse_float(v,il,ln) * 0.35f;
    else if (StringsEqual(k,"type")) flag_setu32(&lit->lflags, (v[0] == 'S') ? LSPOT : LDIR, true);
    else if (StringsEqual(k,"lightOn") && !alreadyReadLightOnOnce[lIdx]) { alreadyReadLightOnOnce[lIdx] = true; flag_setu32(&lit->lflags,LIGHTON,parse_bool(v,il,ln)); }
    else if (StringsEqual(k,"lerpOn")) flag_setu32(&lit->lflags,LERPON,parse_bool(v,il,ln));
}

static inline __attribute__((always_inline)) void mul_mat4(float *out, const float *a, const float *b) { // out = a * b
    out[0] =  a[0] * b[0]  + a[4] * b[1]  + a[8]  * b[2] + a[12]  * b[3]; out[1] =  a[1] * b[0]  + a[5] * b[1]  + a[9]  * b[2] + a[13]  * b[3];
    out[2] =  a[2] * b[0]  + a[6] * b[1] + a[10]  * b[2] + a[14]  * b[3]; out[3] =  a[3] * b[0]  + a[7] * b[1] + a[11]  * b[2] + a[15]  * b[3];
    out[4] =  a[0] * b[4]  + a[4] * b[5]  + a[8]  * b[6] + a[12]  * b[7]; out[5] =  a[1] * b[4]  + a[5] * b[5]  + a[9]  * b[6] + a[13]  * b[7];
    out[6] =  a[2] * b[4]  + a[6] * b[5] + a[10]  * b[6] + a[14]  * b[7]; out[7] =  a[3] * b[4]  + a[7] * b[5] + a[11]  * b[6] + a[15]  * b[7];
    out[8] =  a[0] * b[8]  + a[4] * b[9]  + a[8] * b[10] + a[12] * b[11]; out[9] =  a[1] * b[8]  + a[5] * b[9]  + a[9] * b[10] + a[13] * b[11];
    out[10] = a[2] * b[8]  + a[6] * b[9] + a[10] * b[10] + a[14] * b[11]; out[11] = a[3] * b[8]  + a[7] * b[9] + a[11] * b[10] + a[15] * b[11];
    out[12] = a[0] * b[12] + a[4] * b[13] + a[8] * b[14] + a[12] * b[15]; out[13] = a[1] * b[12] + a[5] * b[13] + a[9] * b[14] + a[13] * b[15];
    out[14] = a[2] * b[12] + a[6] * b[13] + a[10]* b[14] + a[14] * b[15]; out[15] = a[3] * b[12] + a[7] * b[13] + a[11]* b[14] + a[15] * b[15];
}

#define QTR90 0.707106781f
Quaternion cubeQuats[6] = {{0.0f,QTR90,0.0f,QTR90}/*+X:Right*/,{0.0f,-QTR90,0.0f,QTR90}/*-X:Left*/,{-QTR90,0.0f,0.0f,QTR90}/*+Y:Up*/,{QTR90,0.0f,0.0f,QTR90}/*-Y:Down*/,{0.0f,0.0f,0.0f,1.0f}/*+Z:Forward*/,{0.0f,1.0f,0.0f,0.0f}/*-Z:Backward*/ };
bool NeighborhoodInPVS(u16 cellX, u16 cellZ, int r);
void UpdateLights(void) {
    for (u16 lightIdx = 0; lightIdx < Sys_Global.loadedLights; ++lightIdx) {
        Vector3 lightPos = lightsNewPosition[lightIdx];
        lights[lightIdx].pos = lightPos;
        if (lights[lightIdx].lflags & LDIRTY) { // Marked all as true at level load.
            flag_setu32(&lights[lightIdx].lflags,LDIRTY,false);
            #pragma GCC unroll 6
            for (int j=0;j<6;++j) { // Update to new position
                mat4_lookat_from((float*)lightView[lightIdx][j],&cubeQuats[j],lightPos);
                mul_mat4((float*)lightViewProj[lightIdx][j],shadowmapsPerspectiveProjection,(float*)lightView[lightIdx][j]);
                ExtractFrustumPlanes((float*)lightViewProj[lightIdx][j],lightFrustumPlanes[lightIdx][j]);
            }
        }
    }
    
    if (!Sys_Global.gamePaused && !Sys_Global.menuActive) {
        for (int i=0;i<Sys_Global.loadedLights;++i) { // Just lerps/flickers in intensity
            if (lanims[i].numIntervalSteps < 1) continue;
            if (!(lights[i].lflags & LIGHTON)) { lights[i].intensity = 0.0f; continue; }

            if (lanims[i].lerpTime < (float)Sys_Global.pauseRelativeTime) {
                lights[i].intensity = lanims[i].lerpUp ? lights[i].maxIntensity : lights[i].minIntensity; // Pick target to lerp towards
                lanims[i].lerpUp = !lanims[i].lerpUp;
                lanims[i].currentStep++; if (lanims[i].currentStep >= lanims[i].numIntervalSteps) lanims[i].currentStep = 0; // Wrap and start over continuous looping
                lanims[i].lerpStepTime = lanims[i].intervalSteps[lanims[i].currentStep];
                lanims[i].lerpTime = (float)Sys_Global.pauseRelativeTime + lanims[i].lerpStepTime;
                lanims[i].lerpStartTime = (float)Sys_Global.pauseRelativeTime;
            } else if (lights[i].lflags & LERPON) {
                if (lanims[i].currentStep < lanims[i].numLerpSteps) {
                    if (lanims[i].stepIsLerping[lanims[i].currentStep]) {
                        lanims[i].lerpValue = ((float)Sys_Global.pauseRelativeTime - lanims[i].lerpStartTime)/(lanims[i].lerpTime - lanims[i].lerpStartTime); // percent towards goal time
                        float lerpVal = lanims[i].lerpUp ? lanims[i].lerpValue : (1.0f - lanims[i].lerpValue);
                        lanims[i].lerpValue = lights[i].minIntensity + ((lights[i].maxIntensity - lights[i].minIntensity) * lerpVal);
                        lights[i].intensity = lanims[i].lerpValue;
                    }
                }
            }
        }
    }

    glBindBuffer(GL_SSBO,Sys_Render.lightsID); glBufferData(GL_SSBO,Sys_Global.loadedLights * sizeof(Light),lights,GL_DYNAMIC_DRAW);
    glUseProgram(Sys_Render.voxelUpdateShaderProgram);
    glUniform3f(5,Sys_Global.instances[PLAYER1].position.x,Sys_Global.instances[PLAYER1].position.y,Sys_Global.instances[PLAYER1].position.z);
    glDispatchCompute((VOXELS_X+31)/32,(VOXELS_Z+31)/32,1);
}

void UploadGridCellVisibility(void) { glNamedBufferData(Sys_Render.cellVisibleDataID,ARRSIZE * sizeof(u32),gridCellStates,GL_DYNAMIC_DRAW); }
#define CHGD(a,b) (vabs((a) - (b)) > 0.0001f)
ENGINE_TO_MOD void UpdateLight(u16 i, Vector3 pos, Color3 col, float range, float intensity, float max, float min, float spotAng, Quaternion spotDir, bool on, bool shad) {
    bool changed = ((!!(lights[i].lflags & SHADON) - shad) || (!!(lights[i].lflags & LIGHTON) -  on) || CHGD(lights[i].range,range) || CHGD(lights[i].pos.x,pos.x) || CHGD(lights[i].pos.y,pos.y) || CHGD(lights[i].pos.z,pos.z));
    lights[i].intensity=intensity; lights[i].minIntensity=min; lights[i].maxIntensity=max; lights[i].spotAng=spotAng; lights[i].spotDir=spotDir; lights[i].col=col; lights[i].pos=lightsNewPosition[i]=pos; lights[i].range=range;
    flag_setu32(&lights[i].lflags,19,(lights[i].lflags&LDIRTY)|changed<<4|on|shad<<1);
}
#undef CHGD

typedef struct { void* ptr; size_t sz; } TAlloc;
static TAlloc ttAllocs[4474]; static int tallocCount=0;
static void* TempAlloc(size_t n){if(tallocCount>=4474){DualLogError("TempAlloc too many!\n");return NULL;}void*p=OS_Alloc(n);if(!p){DualLogError("TempAlloc: OS_Alloc failed!\n");return NULL;}ttAllocs[tallocCount++]=(TAlloc){p,n};return p;}
static void  TempFree (void* p){if(!p||tallocCount==0||ttAllocs[tallocCount-1].ptr!=p)return;OS_DeallocateRAM(p,ttAllocs[tallocCount-1].sz);tallocCount--;}
#define ttBYTE(p)  (*(u8*)(p))
#define ttCHAR(p)  (*(i8*)(p))
static u16 ttUSHORT(u8*p){return p[0]*256+p[1];}
static i16 ttSHORT (u8*p){return p[0]*256+p[1];}
static u32 ttULONG (u8*p){return((u32)p[0]<<24)|((u32)p[1]<<16)|((u32)p[2]<<8)|p[3];}
static i32 ttLONG  (u8*p){return((i32)p[0]<<24)|((i32)p[1]<<16)|((i32)p[2]<<8)|p[3];}
#define stbtt_tag4(p,a,b,c,d) ((p)[0]==(a)&&(p)[1]==(b)&&(p)[2]==(c)&&(p)[3]==(d))
#define stbtt_tag(p,s)         stbtt_tag4(p,s[0],s[1],s[2],s[3])
typedef struct { unsigned char*data; int cursor,size; } stbtt__buf;
static stbtt__buf stbtt__new_buf(const void*p,size_t s){stbtt__buf r;r.data=(u8*)p;r.size=(int)s;r.cursor=0;return r;}
static u8  _bg8(stbtt__buf*b){return b->cursor>=b->size?0:b->data[b->cursor++];}
static u8  _bp8(stbtt__buf*b){return b->cursor>=b->size?0:b->data[b->cursor];}
static void _bsk(stbtt__buf*b,int o){b->cursor=(o>b->size||o<0)?b->size:o;}
static void _bskip(stbtt__buf*b,int o){_bsk(b,b->cursor+o);}
static u32 _bg(stbtt__buf*b,int n){u32 v=0;for(int i=0;i<n;i++)v=(v<<8)|_bg8(b);return v;}
#define _bg16(b) _bg(b,2)
#define _bg32(b) _bg(b,4)
static stbtt__buf _brange(const stbtt__buf*b,int o,int s){stbtt__buf r=stbtt__new_buf(NULL,0);if(o<0||s<0||o>b->size||s>b->size-o)return r;r.data=b->data+o;r.size=s;return r;}
static stbtt__buf _cff_idx(stbtt__buf*b){int c=b->cursor,n=_bg16(b);if(n){int os=_bg8(b);_bskip(b,os*n);_bskip(b,_bg(b,os)-1);}return _brange(b,c,b->cursor-c);}
static u32 _cff_int(stbtt__buf*b){int b0=_bg8(b);if(b0>=32&&b0<=246)return b0-139;if(b0>=247&&b0<=250)return(b0-247)*256+_bg8(b)+108;if(b0>=251&&b0<=254)return-(b0-251)*256-_bg8(b)-108;if(b0==28)return _bg16(b);if(b0==29)return _bg32(b);return 0;}
static void _cff_skip_op(stbtt__buf*b){if(_bp8(b)==30){_bskip(b,1);while(b->cursor<b->size){int v=_bg8(b);if((v&0xF)==0xF||(v>>4)==0xF)break;}}else _cff_int(b);}
static stbtt__buf _dict_get(stbtt__buf*b,int key){_bsk(b,0);while(b->cursor<b->size){int s=b->cursor,e,op;while(_bp8(b)>=28)_cff_skip_op(b);e=b->cursor;op=_bg8(b);if(op==12)op=_bg8(b)|0x100;if(op==key)return _brange(b,s,e-s);}return _brange(b,0,0);}
static void _dict_ints(stbtt__buf*b,int key,int n,u32*out){stbtt__buf op=_dict_get(b,key);for(int i=0;i<n&&op.cursor<op.size;++i)out[i]=(u32)_cff_int(&op);}
static int  _cff_idx_cnt(stbtt__buf*b){_bsk(b,0);return _bg16(b);}
static stbtt__buf _cff_idx_get(stbtt__buf b,int i){_bsk(&b,0);int n=_bg16(&b),os=_bg8(&b);_bskip(&b,i*os);int s=_bg(&b,os),e=_bg(&b,os);return _brange(&b,2+(n+1)*os+s,e-s);}
enum{STBTT_vmove=1,STBTT_vline,STBTT_vcurve,STBTT_vcubic};
#define stbtt_vertex_type short
typedef struct{stbtt_vertex_type x,y,cx,cy,cx1,cy1;unsigned char type,padding;}stbtt_vertex;
typedef struct{void*userdata;unsigned char*data;int fontstart,numGlyphs,loca,head,glyf,hhea,hmtx,kern,gpos,svg,index_map,indexToLocFormat;stbtt__buf cff,charstrings,gsubrs,subrs,fontdicts,fdselect;}stbtt_fontinfo;
static u32 _find_table(u8*d,u32 fs,const char*tag){i32 n=ttUSHORT(d+fs+4);u32 td=fs+12;for(i32 i=0;i<n;++i){u32 l=td+16*i;if(stbtt_tag(d+l+0,tag))return ttULONG(d+l+8);}return 0;}
static stbtt__buf _get_subrs(stbtt__buf cff,stbtt__buf fd){u32 so=0,pl[2]={0,0};_dict_ints(&fd,18,2,pl);if(!pl[1]||!pl[0])return stbtt__new_buf(NULL,0);stbtt__buf pd=_brange(&cff,pl[1],pl[0]);_dict_ints(&pd,19,1,&so);if(!so)return stbtt__new_buf(NULL,0);_bsk(&cff,pl[1]+so);return _cff_idx(&cff);}
static int stbtt_InitFont_internal(stbtt_fontinfo*info,unsigned char*data,int fs){
    u32 cmap,t,i,nt;info->data=data;info->fontstart=fs;info->cff=stbtt__new_buf(NULL,0);
    cmap=_find_table(data,fs,"cmap");info->loca=_find_table(data,fs,"loca");info->head=_find_table(data,fs,"head");
    info->glyf=_find_table(data,fs,"glyf");info->hhea=_find_table(data,fs,"hhea");info->hmtx=_find_table(data,fs,"hmtx");
    info->kern=_find_table(data,fs,"kern");info->gpos=_find_table(data,fs,"GPOS");
    if(!cmap||!info->head||!info->hhea||!info->hmtx)return 0;
    if(info->glyf){if(!info->loca)return 0;}
    else{
        u32 cs=2,chstr=0,fda=0,fds=0,cff=_find_table(data,fs,"CFF ");if(!cff)return 0;
        info->fontdicts=stbtt__new_buf(NULL,0);info->fdselect=stbtt__new_buf(NULL,0);
        info->cff=stbtt__new_buf(data+cff,16*1024*1024);stbtt__buf b=info->cff;
        _bskip(&b,2);_bsk(&b,_bg8(&b));_cff_idx(&b);
        stbtt__buf tdi=_cff_idx(&b),td=_cff_idx_get(tdi,0);_cff_idx(&b);info->gsubrs=_cff_idx(&b);
        _dict_ints(&td,17,1,&chstr);_dict_ints(&td,0x100|6,1,&cs);_dict_ints(&td,0x100|36,1,&fda);_dict_ints(&td,0x100|37,1,&fds);
        info->subrs=_get_subrs(b,td);
        if(cs!=2||chstr==0)return 0;
        if(fda){if(!fds)return 0;_bsk(&b,fda);info->fontdicts=_cff_idx(&b);info->fdselect=_brange(&b,fds,b.size-fds);}
        _bsk(&b,chstr);info->charstrings=_cff_idx(&b);
    }
    t=_find_table(data,fs,"maxp");info->numGlyphs=t?ttUSHORT(data+t+4):0xffff;
    info->svg=-1;nt=ttUSHORT(data+cmap+2);info->index_map=0;
    for(i=0;i<nt;++i){u32 er=cmap+4+8*i;switch(ttUSHORT(data+er)){case 3:switch(ttUSHORT(data+er+2)){case 1:case 10:info->index_map=cmap+ttULONG(data+er+4);}break;case 0:info->index_map=cmap+ttULONG(data+er+4);break;}}
    if(!info->index_map)return 0;
    info->indexToLocFormat=ttUSHORT(data+info->head+50);return 1;
}

static int _font_offset(unsigned char*d,int idx){
    if(stbtt_tag4(d,'1',0,0,0)||stbtt_tag(d,"typ1")||stbtt_tag(d,"OTTO")||stbtt_tag4(d,0,1,0,0)||stbtt_tag(d,"true"))return idx==0?0:-1;
    if(stbtt_tag(d,"ttcf")&&(ttULONG(d+4)==0x00010000||ttULONG(d+4)==0x00020000)){i32 n=ttLONG(d+8);if(idx>=n)return -1;return ttULONG(d+12+idx*4);}
    return -1;
}

static __attribute__((pure)) int stbtt_GetFontOffsetForIndex(const unsigned char*d,int i){return _font_offset((unsigned char*)d,i);}
static __attribute__((pure)) int stbtt_FindGlyphIndex(const stbtt_fontinfo*info,int cp){
    u8*d=info->data;u32 im=info->index_map;u16 fmt=ttUSHORT(d+im);
    if(fmt==0){i32 b=ttUSHORT(d+im+2);return cp<b-6?ttBYTE(d+im+6+cp):0;}
    if(fmt==6){u32 f=ttUSHORT(d+im+6),n=ttUSHORT(d+im+8);return(u32)cp>=f&&(u32)cp<f+n?ttUSHORT(d+im+10+(cp-f)*2):0;}
    if(fmt==2)return 0;
    if(fmt==4){
        u16 sc=ttUSHORT(d+im+6)>>1,sr=ttUSHORT(d+im+8)>>1,es=ttUSHORT(d+im+10),rs=ttUSHORT(d+im+12)>>1;
        u32 ec=im+14,s=ec;if(cp>0xffff)return 0;
        if(cp>=ttUSHORT(d+s+rs*2))s+=rs*2;s-=2;
        while(es){sr>>=1;u16 e=ttUSHORT(d+s+sr*2);if(cp>e)s+=sr*2;--es;}
        s+=2;{u16 it=(u16)((s-ec)>>1),st=ttUSHORT(d+im+14+sc*2+2+2*it),la=ttUSHORT(d+ec+2*it);
        if(cp<st||cp>la)return 0;u16 off=ttUSHORT(d+im+14+sc*6+2+2*it);
        return off?ttUSHORT(d+off+(cp-st)*2+im+14+sc*6+2+2*it):(u16)(cp+ttSHORT(d+im+14+sc*4+2+2*it));}
    }
    if(fmt==12||fmt==13){u32 ng=ttULONG(d+im+12);i32 lo=0,hi=(i32)ng;
        while(lo<hi){i32 m=lo+((hi-lo)>>1);u32 sc=ttULONG(d+im+16+m*12),ec=ttULONG(d+im+16+m*12+4);
        if((u32)cp<sc)hi=m;else if((u32)cp>ec)lo=m+1;else{u32 sg=ttULONG(d+im+16+m*12+8);return fmt==12?sg+cp-sc:sg;}}return 0;}
    return 0;
}

static void _sv(stbtt_vertex*v,u8 t,i32 x,i32 y,i32 cx,i32 cy){v->type=t;v->x=(i16)x;v->y=(i16)y;v->cx=(i16)cx;v->cy=(i16)cy;}
static int _glyf_off(const stbtt_fontinfo*info,int gi){
    if(gi>=info->numGlyphs||info->indexToLocFormat>=2)return-1;
    int g1,g2;if(info->indexToLocFormat==0){g1=info->glyf+ttUSHORT(info->data+info->loca+gi*2)*2;g2=info->glyf+ttUSHORT(info->data+info->loca+gi*2+2)*2;}
    else{g1=info->glyf+ttULONG(info->data+info->loca+gi*4);g2=info->glyf+ttULONG(info->data+info->loca+gi*4+4);}
    return g1==g2?-1:g1;
}

static int _close_shape(stbtt_vertex*v,int n,int wo,int so,i32 sx,i32 sy,i32 scx,i32 scy,i32 cx,i32 cy){
    if(so){if(wo)_sv(&v[n++],STBTT_vcurve,(cx+scx)>>1,(cy+scy)>>1,cx,cy);_sv(&v[n++],STBTT_vcurve,sx,sy,scx,scy);}
    else{if(wo)_sv(&v[n++],STBTT_vcurve,sx,sy,cx,cy);else _sv(&v[n++],STBTT_vline,sx,sy,0,0);}
    return n;
}

static int _GetGlyphShapeT2(const stbtt_fontinfo*,int,stbtt_vertex**);
int stbtt_GetGlyphShape(const stbtt_fontinfo*info,int gi,stbtt_vertex**pv);
static int _GetGlyphShapeTT(const stbtt_fontinfo*info,int gi,stbtt_vertex**pv){
    u8*d=info->data;stbtt_vertex*verts=0;int nv=0,g=_glyf_off(info,gi);*pv=NULL;if(g<0)return 0;
    i16 nc=ttSHORT(d+g);
    if(nc>0){
        u8*ep=d+g+10;int ins=ttUSHORT(d+g+10+nc*2);u8*pts=d+g+10+nc*2+2+ins;
        int n=1+ttUSHORT(ep+nc*2-2),m=n+2*nc;verts=(stbtt_vertex*)TempAlloc(m*sizeof(verts[0]));if(!verts)return 0;
        int off=m-n;u8 fl=0,fc=0;
        for(int i=0;i<n;++i){if(fc==0){fl=*pts++;if(fl&8)fc=*pts++;}else--fc;verts[off+i].type=fl;}
        i32 x=0;for(int i=0;i<n;++i){fl=verts[off+i].type;if(fl&2){i16 dx=*pts++;x+=(fl&16)?dx:-dx;}else if(!(fl&16)){x+=(i16)(pts[0]*256+pts[1]);pts+=2;}verts[off+i].x=(i16)x;}
        i32 y=0;for(int i=0;i<n;++i){fl=verts[off+i].type;if(fl&4){i16 dy=*pts++;y+=(fl&32)?dy:-dy;}else if(!(fl&32)){y+=(i16)(pts[0]*256+pts[1]);pts+=2;}verts[off+i].y=(i16)y;}
        i32 sx=0,sy=0,cx=0,cy=0,scx=0,scy=0;int wo=0,so=0,nm=0,j=0;
        for(int i=0;i<n;++i){fl=verts[off+i].type;x=(i16)verts[off+i].x;y=(i16)verts[off+i].y;
            if(nm==i){if(i)nv=_close_shape(verts,nv,wo,so,sx,sy,scx,scy,cx,cy);so=!(fl&1);
                if(so){scx=x;scy=y;if(!(verts[off+i+1].type&1)){sx=(x+(i32)verts[off+i+1].x)>>1;sy=(y+(i32)verts[off+i+1].y)>>1;}else{sx=verts[off+i+1].x;sy=verts[off+i+1].y;++i;}}else{sx=x;sy=y;}
                _sv(&verts[nv++],STBTT_vmove,sx,sy,0,0);wo=0;nm=1+ttUSHORT(ep+j++*2);
            }else{if(!(fl&1)){if(wo)_sv(&verts[nv++],STBTT_vcurve,(cx+x)>>1,(cy+y)>>1,cx,cy);cx=x;cy=y;wo=1;}
                else{_sv(&verts[nv++],wo?STBTT_vcurve:STBTT_vline,x,y,wo?cx:0,wo?cy:0);wo=0;}}}
        nv=_close_shape(verts,nv,wo,so,sx,sy,scx,scy,cx,cy);
    }else if(nc<0){
        u8*comp=d+g+10;int more=1;
        while(more){stbtt_vertex*cv=0,*tmp=0;float mtx[6]={1,0,0,1,0,0};
            u16 fl=ttSHORT(comp);comp+=2;u16 gidx=ttSHORT(comp);comp+=2;
            if(fl&2){if(fl&1){mtx[4]=ttSHORT(comp);comp+=2;mtx[5]=ttSHORT(comp);comp+=2;}else{mtx[4]=ttCHAR(comp);comp++;mtx[5]=ttCHAR(comp);comp++;}}
            if(fl&(1<<3)){mtx[0]=mtx[3]=ttSHORT(comp)/16384.0f;comp+=2;mtx[1]=mtx[2]=0;}
            else if(fl&(1<<6)){mtx[0]=ttSHORT(comp)/16384.0f;comp+=2;mtx[1]=mtx[2]=0;mtx[3]=ttSHORT(comp)/16384.0f;comp+=2;}
            else if(fl&(1<<7)){mtx[0]=ttSHORT(comp)/16384.0f;comp+=2;mtx[1]=ttSHORT(comp)/16384.0f;comp+=2;mtx[2]=ttSHORT(comp)/16384.0f;comp+=2;mtx[3]=ttSHORT(comp)/16384.0f;comp+=2;}
            float fm=vsqrtf(mtx[0]*mtx[0]+mtx[1]*mtx[1]),fn=vsqrtf(mtx[2]*mtx[2]+mtx[3]*mtx[3]);
            int cn=stbtt_GetGlyphShape(info,gidx,&cv);
            if(cn>0){for(int i=0;i<cn;++i){stbtt_vertex*v=&cv[i];stbtt_vertex_type vx=v->x,vy=v->y;v->x=(stbtt_vertex_type)(fm*(mtx[0]*vx+mtx[2]*vy+mtx[4]));v->y=(stbtt_vertex_type)(fn*(mtx[1]*vx+mtx[3]*vy+mtx[5]));vx=v->cx;vy=v->cy;v->cx=(stbtt_vertex_type)(fm*(mtx[0]*vx+mtx[2]*vy+mtx[4]));v->cy=(stbtt_vertex_type)(fn*(mtx[1]*vx+mtx[3]*vy+mtx[5]));}
                tmp=(stbtt_vertex*)TempAlloc((nv+cn)*sizeof(stbtt_vertex));if(!tmp){TempFree(verts);TempFree(cv);return 0;}
                if(nv>0&&verts) CopyMemoryFromBtoAForNBytes(tmp,verts,nv*sizeof(stbtt_vertex)); CopyMemoryFromBtoAForNBytes(tmp+nv,cv,cn*sizeof(stbtt_vertex));TempFree(verts);TempFree(cv);verts=tmp;nv+=cn;}
            more=fl&(1<<5);}
    }
    *pv=verts;return nv;
}

typedef struct{int bounds,started;float first_x,first_y,x,y;i32 min_x,max_x,min_y,max_y;stbtt_vertex*pvertices;int num_vertices;}stbtt__csctx;
#define CSCTX_INIT(b) {b,0,0,0,0,0,0,0,0,0,NULL,0}
static void _trk(stbtt__csctx*c,i32 x,i32 y){if(x>c->max_x||!c->started)c->max_x=x;if(y>c->max_y||!c->started)c->max_y=y;if(x<c->min_x||!c->started)c->min_x=x;if(y<c->min_y||!c->started)c->min_y=y;c->started=1;}
static void _csv(stbtt__csctx*c,u8 t,i32 x,i32 y,i32 cx,i32 cy,i32 cx1,i32 cy1){if(c->bounds){_trk(c,x,y);if(t==STBTT_vcubic){_trk(c,cx,cy);_trk(c,cx1,cy1);}}else{_sv(&c->pvertices[c->num_vertices],t,x,y,cx,cy);c->pvertices[c->num_vertices].cx1=(i16)cx1;c->pvertices[c->num_vertices].cy1=(i16)cy1;}c->num_vertices++;}
static void _csclose(stbtt__csctx*c){if(c->first_x!=c->x||c->first_y!=c->y)_csv(c,STBTT_vline,(int)c->first_x,(int)c->first_y,0,0,0,0);}
static void _csmove(stbtt__csctx*c,float dx,float dy){_csclose(c);c->first_x=c->x=c->x+dx;c->first_y=c->y=c->y+dy;_csv(c,STBTT_vmove,(int)c->x,(int)c->y,0,0,0,0);}
static void _csline(stbtt__csctx*c,float dx,float dy){c->x+=dx;c->y+=dy;_csv(c,STBTT_vline,(int)c->x,(int)c->y,0,0,0,0);}
static void _cscurve(stbtt__csctx*c,float d1,float e1,float d2,float e2,float d3,float e3){float cx1=c->x+d1,cy1=c->y+e1,cx2=cx1+d2,cy2=cy1+e2;c->x=cx2+d3;c->y=cy2+e3;_csv(c,STBTT_vcubic,(int)c->x,(int)c->y,(int)cx1,(int)cy1,(int)cx2,(int)cy2);}
static stbtt__buf _subr(stbtt__buf idx,int n){int c=_cff_idx_cnt(&idx),bias=c>=33900?32768:c>=1240?1131:107;n+=bias;return(n<0||n>=c)?stbtt__new_buf(NULL,0):_cff_idx_get(idx,n);}
static stbtt__buf _cid_subrs(const stbtt_fontinfo*info,int gi){stbtt__buf fd=info->fdselect;int nr,st,end,v,fmt,sel=-1,i;_bsk(&fd,0);fmt=_bg8(&fd);
    if(fmt==0){_bskip(&fd,gi);sel=_bg8(&fd);}
    else if(fmt==3){nr=_bg16(&fd);st=_bg16(&fd);for(i=0;i<nr;i++){v=_bg8(&fd);end=_bg16(&fd);if(gi>=st&&gi<end){sel=v;break;}st=end;}}
    if(sel==-1)return stbtt__new_buf(NULL,0);return _get_subrs(info->cff,_cff_idx_get(info->fontdicts,sel));}

static int _run_cs(const stbtt_fontinfo*info,int gi,stbtt__csctx*c){
    int hdr=1,mb=0,ssh=0,sp=0,hs=0,i,b0;float s[48],f;
    stbtt__buf ss[10],subrs=info->subrs,b=_cff_idx_get(info->charstrings,gi);
#define ERR(x) return 0
#define CHK(n) if(sp<(n))ERR(#n)
    while(b.cursor<b.size){int cs=1;i=0;b0=_bg8(&b);
        switch(b0){
        case 0x13:case 0x14:if(hdr)mb+=sp/2;hdr=0;_bskip(&b,(mb+7)/8);break;
        case 0x01:case 0x03:case 0x12:case 0x17:mb+=sp/2;break;
        case 0x15:hdr=0;CHK(2);_csmove(c,s[sp-2],s[sp-1]);break;
        case 0x04:hdr=0;CHK(1);_csmove(c,0,s[sp-1]);break;
        case 0x16:hdr=0;CHK(1);_csmove(c,s[sp-1],0);break;
        case 0x05:CHK(2);for(;i+1<sp;i+=2)_csline(c,s[i],s[i+1]);break;
        case 0x07:CHK(1);goto vlt;
        case 0x06:CHK(1);for(;;){if(i>=sp)break;_csline(c,s[i++],0);vlt:if(i>=sp)break;_csline(c,0,s[i++]);}break;
        case 0x1F:CHK(4);goto hvc;
        case 0x1E:CHK(4);for(;;){if(i+3>=sp)break;_cscurve(c,0,s[i],s[i+1],s[i+2],s[i+3],(sp-i==5)?s[i+4]:0);i+=4;hvc:if(i+3>=sp)break;_cscurve(c,s[i],0,s[i+1],s[i+2],(sp-i==5)?s[i+4]:0,s[i+3]);i+=4;}break;
        case 0x08:CHK(6);for(;i+5<sp;i+=6)_cscurve(c,s[i],s[i+1],s[i+2],s[i+3],s[i+4],s[i+5]);break;
        case 0x18:CHK(8);for(;i+5<sp-2;i+=6)_cscurve(c,s[i],s[i+1],s[i+2],s[i+3],s[i+4],s[i+5]);_csline(c,s[i],s[i+1]);break;
        case 0x19:CHK(8);for(;i+1<sp-6;i+=2)_csline(c,s[i],s[i+1]);_cscurve(c,s[i],s[i+1],s[i+2],s[i+3],s[i+4],s[i+5]);break;
        case 0x1A:case 0x1B:CHK(4);f=0;if(sp&1)f=s[i++];for(;i+3<sp;i+=4,f=0)_cscurve(c,b0==0x1B?s[i]:f,b0==0x1B?f:s[i],s[i+1],s[i+2],b0==0x1B?s[i+3]:0,b0==0x1B?0:s[i+3]);break;
        case 0x0A:if(!hs){if(info->fdselect.size)subrs=_cid_subrs(info,gi);hs=1;}
        case 0x1D:CHK(1);if(ssh>=10)ERR("recursion");ss[ssh++]=b;b=_subr(b0==0x0A?subrs:info->gsubrs,(int)s[--sp]);if(!b.size)ERR("subr");b.cursor=0;cs=0;break;
        case 0x0B:if(ssh<=0)ERR("return");b=ss[--ssh];cs=0;break;
        case 0x0E:_csclose(c);return 1;
        case 0x0C:{int b1=_bg8(&b);switch(b1){
            case 0x22:CHK(7);_cscurve(c,s[0],0,s[1],s[2],s[3],0);_cscurve(c,s[4],0,s[5],-s[2],s[6],0);break;
            case 0x23:CHK(13);_cscurve(c,s[0],s[1],s[2],s[3],s[4],s[5]);_cscurve(c,s[6],s[7],s[8],s[9],s[10],s[11]);break;
            case 0x24:CHK(9);_cscurve(c,s[0],s[1],s[2],s[3],s[4],0);_cscurve(c,s[5],0,s[6],s[7],s[8],-(s[1]+s[3]+s[7]));break;
            case 0x25:CHK(11);{float dx=s[0]+s[2]+s[4]+s[6]+s[8],dy=s[1]+s[3]+s[5]+s[7]+s[9],d6x=s[10],d6y=s[10];if(vabs(dx)>vabs(dy))d6y=-dy;else d6x=-dx;_cscurve(c,s[0],s[1],s[2],s[3],s[4],s[5]);_cscurve(c,s[6],s[7],s[8],s[9],d6x,d6y);}break;
            default:ERR("escape");}}break;
        default:if(b0!=255&&b0!=28&&b0<32)ERR("reserved");f=(b0==255)?(float)(i32)_bg32(&b)/0x10000:(_bskip(&b,-1),(float)(i16)_cff_int(&b));if(sp>=48)ERR("overflow");s[sp++]=f;cs=0;break;}
        if(cs)sp=0;}ERR("no endchar");
#undef ERR
#undef CHK
}

int stbtt_GetGlyphShape(const stbtt_fontinfo*info,int gi,stbtt_vertex**pv){return info->cff.size?_GetGlyphShapeT2(info,gi,pv):_GetGlyphShapeTT(info,gi,pv);}
static int _GetGlyphShapeT2(const stbtt_fontinfo*info,int gi,stbtt_vertex**pv){stbtt__csctx cc=CSCTX_INIT(1),oc=CSCTX_INIT(0);if(_run_cs(info,gi,&cc)){*pv=(stbtt_vertex*)TempAlloc(cc.num_vertices*sizeof(stbtt_vertex));oc.pvertices=*pv;if(_run_cs(info,gi,&oc))return oc.num_vertices;}*pv=NULL;return 0;}
static int _GetGlyphInfoT2(const stbtt_fontinfo*info,int gi,int*x0,int*y0,int*x1,int*y1){stbtt__csctx c=CSCTX_INIT(1);int r=_run_cs(info,gi,&c);if(x0)*x0=r?c.min_x:0;if(y0)*y0=r?c.min_y:0;if(x1)*x1=r?c.max_x:0;if(y1)*y1=r?c.max_y:0;return r?c.num_vertices:0;}
static int stbtt_GetGlyphBox(const stbtt_fontinfo*info,int gi,int*x0,int*y0,int*x1,int*y1){
    if(info->cff.size){_GetGlyphInfoT2(info,gi,x0,y0,x1,y1);}
    else{int g=_glyf_off(info,gi);if(g<0)return 0;if(x0)*x0=ttSHORT(info->data+g+2);if(y0)*y0=ttSHORT(info->data+g+4);if(x1)*x1=ttSHORT(info->data+g+6);if(y1)*y1=ttSHORT(info->data+g+8);}
    return 1;
}

static void stbtt_GetGlyphHMetrics(const stbtt_fontinfo*info,int gi,int*adv,int*lsb){
    u16 n=ttUSHORT(info->data+info->hhea+34);
    if(gi<n){if(adv)*adv=ttSHORT(info->data+info->hmtx+4*gi);if(lsb)*lsb=ttSHORT(info->data+info->hmtx+4*gi+2);}
    else{if(adv)*adv=ttSHORT(info->data+info->hmtx+4*(n-1));if(lsb)*lsb=ttSHORT(info->data+info->hmtx+4*n+2*(gi-n));}
}

static __attribute__((pure)) float stbtt_ScaleForPixelHeight(const stbtt_fontinfo*info,float h){return h/(float)(ttSHORT(info->data+info->hhea+4)-ttSHORT(info->data+info->hhea+6));}
static __attribute__((pure)) float stbtt_ScaleForMappingEmToPixels(const stbtt_fontinfo*info,float px){return px/(float)ttUSHORT(info->data+info->head+18);}
static void stbtt_GetGlyphBitmapBoxSubpixel(const stbtt_fontinfo*font,int g,float sx,float sy,float shx,float shy,int*ix0,int*iy0,int*ix1,int*iy1){
    int x0=0,y0=0,x1,y1;if(!stbtt_GetGlyphBox(font,g,&x0,&y0,&x1,&y1)){if(ix0)*ix0=0;if(iy0)*iy0=0;if(ix1)*ix1=0;if(iy1)*iy1=0;}
    else{if(ix0)*ix0=(int)vfloor(x0*sx+shx);if(iy0)*iy0=(int)vfloor(-y1*sy+shy);if(ix1)*ix1=(int)vceil(x1*sx+shx);if(iy1)*iy1=(int)vceil(-y0*sy+shy);}
}

static void stbtt_GetGlyphBitmapBox(const stbtt_fontinfo*f,int g,float sx,float sy,int*ix0,int*iy0,int*ix1,int*iy1){stbtt_GetGlyphBitmapBoxSubpixel(f,g,sx,sy,0,0,ix0,iy0,ix1,iy1);}
typedef struct{int w,h,stride;unsigned char*pixels;}stbtt__bitmap;
typedef struct stbtt__hheap_chunk{ struct stbtt__hheap_chunk* next; }stbtt__hheap_chunk;
typedef struct{ stbtt__hheap_chunk* head; void* first_free; int num_remaining_in_head_chunk; }stbtt__hheap;
static void* _hha(stbtt__hheap* hh,size_t sz){ if(hh->first_free){void*p=hh->first_free;hh->first_free=*(void**)p;return p;}if(!hh->num_remaining_in_head_chunk){int c=sz<32?2000:sz<128?800:100;stbtt__hheap_chunk*ck=(stbtt__hheap_chunk*)TempAlloc(sizeof(*ck)+sz*c);if(!ck)return NULL;ck->next=hh->head;hh->head=ck;hh->num_remaining_in_head_chunk=c;}--hh->num_remaining_in_head_chunk;return(char*)hh->head+sizeof(stbtt__hheap_chunk)+sz*hh->num_remaining_in_head_chunk; }
static void _hhf(stbtt__hheap* hh,void*p) { *(void**)p=hh->first_free;hh->first_free=p; }
typedef struct{ float x0,y0,x1,y1; int invert; }stbtt__edge;
typedef struct stbtt__active_edge{ struct stbtt__active_edge*next; float fx,fdx,fdy,direction,sy,ey; }stbtt__active_edge;
static void _hce(float*sl,int x,stbtt__active_edge*e,float x0,float y0,float x1,float y1){
    if(y0==y1||y0>e->ey||y1<e->sy)return;if(y0<e->sy){x0+=(x1-x0)*(e->sy-y0)/(y1-y0);y0=e->sy;}if(y1>e->ey){x1+=(x1-x0)*(e->ey-y1)/(y1-y0);y1=e->ey;}
    if(x0<=x&&x1<=x)sl[x]+=e->direction*(y1-y0);else if(x0>=x+1&&x1>=x+1);else sl[x]+=e->direction*(y1-y0)*(1.0f-((x0-(float)x)+(x1-(float)x))/2.0f);
}

static float _ptz(float h,float t0,float t1,float b0,float b1){ return ((t1-t0)+(b1-b0))/2.0f*h; }
static void _fae(float*sl,float*sf,int len,stbtt__active_edge*e,float yt){
    float yb=yt+1;
    while(e){
        if(e->fdx==0){float x0=e->fx;if(x0<len){if(x0>=0){_hce(sl,(int)x0,e,x0,yt,x0,yb);_hce(sf-1,(int)x0+1,e,x0,yt,x0,yb);}else _hce(sf-1,0,e,x0,yt,x0,yb);}}
        else{float x0=e->fx,dx=e->fdx,dy=e->fdy,xb=x0+dx,xt,xbt,sy0,sy1;
            if(e->sy>yt){xt=x0+dx*(e->sy-yt);sy0=e->sy;}else{xt=x0;sy0=yt;}
            if(e->ey<yb){xbt=x0+dx*(e->ey-yt);sy1=e->ey;}else{xbt=xb;sy1=yb;}
            if(xt>=0&&xbt>=0&&xt<len&&xbt<len){
                if((int)xt==(int)xbt){int x=(int)xt;float h=(sy1-sy0)*e->direction;sl[x]+=_ptz(h,xt,(float)x+1.0f,xbt,(float)x+1.0f);sf[x]+=h;}
                else{float yc,yf,step,sign,area;
                    if(xt>xbt){float t;sy0=yb-(sy0-yt);sy1=yb-(sy1-yt);t=sy0;sy0=sy1;sy1=t;t=xbt;xbt=xt;xt=t;dx=-dx;dy=-dy;t=x0;x0=xb;xb=t;}
                    int x1=(int)xt,x2=(int)xbt;yc=yt+dy*((float)(x1+1)-x0);yf=yt+dy*((float)x2-x0);if(yc>yb)yc=yb;sign=e->direction;area=sign*(yc-sy0);
                    sl[x1] += area*((float)(x1+1)-xt)/2;
                    if(yf>yb){yf=yb;dy=(yf-yc)/((float)x2-(float)(x1+1));}
                    step=sign*dy;for(int x=x1+1;x<x2;++x){sl[x]+=area+step/2;area+=step;}
                    sl[x2]+=area+sign*_ptz(sy1-yf,(float)x2,(float)x2+1.0f,xbt,(float)x2+1.0f);sf[x2]+=sign*(sy1-sy0);}
            }else{for(int x=0;x<len;++x){float y0=yt,x1f=(float)x,x2f=(float)(x+1),x3=xb,y3=yb;float y1=((float)x-x0)/dx+yt,y2=((float)(x+1)-x0)/dx+yt;
                if(x0<x1f&&x3>x2f){_hce(sl,x,e,x0,y0,x1f,y1);_hce(sl,x,e,x1f,y1,x2f,y2);_hce(sl,x,e,x2f,y2,x3,y3);}
                else if(x3<x1f&&x0>x2f){_hce(sl,x,e,x0,y0,x2f,y2);_hce(sl,x,e,x2f,y2,x1f,y1);_hce(sl,x,e,x1f,y1,x3,y3);}
                else if(x0<x1f&&x3>x1f){_hce(sl,x,e,x0,y0,x1f,y1);_hce(sl,x,e,x1f,y1,x3,y3);}
                else if(x3<x1f&&x0>x1f){_hce(sl,x,e,x0,y0,x1f,y1);_hce(sl,x,e,x1f,y1,x3,y3);}
                else if(x0<x2f&&x3>x2f){_hce(sl,x,e,x0,y0,x2f,y2);_hce(sl,x,e,x2f,y2,x3,y3);}
                else if(x3<x2f&&x0>x2f){_hce(sl,x,e,x0,y0,x2f,y2);_hce(sl,x,e,x2f,y2,x3,y3);}
                else _hce(sl,x,e,x0,y0,x3,y3);}}
        }e=e->next;}
}

static void _rse(stbtt__bitmap*res,stbtt__edge*e,int n,int ox,int oy){
    stbtt__hheap hh={0,0,0};stbtt__active_edge*active=NULL;int y,j=0,i;
    float sd[129],*sl,*sl2;if(res->w>64)sl=(float*)TempAlloc((size_t)(res->w*2+1)*sizeof(float));else sl=sd;
    sl2=sl+res->w;y=oy;e[n].y0=(float)(oy+res->h)+1;
    while(j<res->h){float syt=(float)y,syb=(float)y+1;stbtt__active_edge**step=&active;
        MemSetToValueForNBytes(sl,0,(size_t)res->w*sizeof(sl[0]));MemSetToValueForNBytes(sl2,0,((size_t)res->w+1)*sizeof(sl[0]));
        while(*step){stbtt__active_edge*z=*step;if(z->ey<=syt){*step=z->next;z->direction=0;_hhf(&hh,z);}else step=&(*step)->next;}
        while(e->y0<=syb){
            if(e->y0!=e->y1){
                stbtt__active_edge* z=(stbtt__active_edge*)_hha(&hh,sizeof(*z));
                if(z) {
                    float dxdy = (e->x1-e->x0)/(e->y1-e->y0);
                    z->fdx = dxdy; z->fdy = dxdy ? 1.0f/dxdy : 0;
                    z->fx = e->x0 + dxdy * (syt - e->y0) - (float)ox;
                    z->direction = e->invert ? 1.0f : -1.0f;
                    z->sy = e->y0; z->ey = e->y1; z->next = 0;
                    if(j == 0 && oy != 0 && z->ey < syt) z->ey = syt;
                    z->next = active; active = z;
                }
            }++e;
        }
        if(active)_fae(sl,sl2+1,res->w,active,syt);
        {float sum=0;for(i=0;i<res->w;++i){float k;int m;sum+=sl2[i];k=(float)vabs(sl[i]+sum)*255.0f+0.5f;m=(int)k;if(m>255)m=255;res->pixels[j*res->stride+i]=(unsigned char)m;}}
        step=&active;while(*step){stbtt__active_edge*z=*step;z->fx+=z->fdx;step=&(*step)->next;}++y;++j;}
    stbtt__hheap_chunk* c = hh.head; while(c){ stbtt__hheap_chunk* n = c->next; TempFree(c); c = n;} if(sl != sd) TempFree(sl);
}

#define _CMP(a,b) ((a)->y0<(b)->y0)
#define _SWP(a,b) {stbtt__edge t_=(a);(a)=(b);(b)=t_;}
static void _eis(stbtt__edge*p,int n){for(int i=1;i<n;++i){stbtt__edge t=p[i];int j=i;while(j>0&&_CMP(&t,&p[j-1])){p[j]=p[j-1];--j;}p[j]=t;}}
static void _eqs(stbtt__edge*p,int n){while(n>12){int m=n>>1,c01=_CMP(&p[0],&p[m]),c12=_CMP(&p[m],&p[n-1]);if(c01!=c12){int z=(_CMP(&p[0],&p[n-1])==c12)?0:n-1;_SWP(p[z],p[m]);}_SWP(p[0],p[m]);int i=1,j=n-1;for(;;){while(_CMP(&p[i],&p[0]))++i;while(_CMP(&p[0],&p[j]))--j;if(i>=j)break;_SWP(p[i],p[j]);++i;--j;}if(j<n-i){_eqs(p,j);p+=i;n-=i;}else{_eqs(p+i,n-i);n=j;}}}
static void _esort(stbtt__edge*p,int n){_eqs(p,n);_eis(p,n);}
static void _add_pt(Vector2*p,int n,float x,float y){if(p){p[n].x=x;p[n].y=y;}}
static int _tess_c(Vector2*pts,int*np,float x0,float y0,float x1,float y1,float x2,float y2,float fsq,int n){
    float mx=(x0+2*x1+x2)/4,my=(y0+2*y1+y2)/4,dx=(x0+x2)/2-mx,dy=(y0+y2)/2-my;
    if(n>16||dx*dx+dy*dy<=fsq){_add_pt(pts,(*np)++,x2,y2);return 1;}
    _tess_c(pts,np,x0,y0,(x0+x1)/2,(y0+y1)/2,mx,my,fsq,n+1);_tess_c(pts,np,mx,my,(x1+x2)/2,(y1+y2)/2,x2,y2,fsq,n+1);return 1;
}

static void _tess_cb(Vector2*pts,int*np,float x0,float y0,float x1,float y1,float x2,float y2,float x3,float y3,float fsq,int n){
    float d0=vsqrtf((x1-x0)*(x1-x0)+(y1-y0)*(y1-y0)),d1=vsqrtf((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1)),d2=vsqrtf((x3-x2)*(x3-x2)+(y3-y2)*(y3-y2)),ds=vsqrtf((x3-x0)*(x3-x0)+(y3-y0)*(y3-y0)),ll=d0+d1+d2;
    if(n>16||ll*ll-ds*ds<=fsq){_add_pt(pts,(*np)++,x3,y3);return;}
    float x01=(x0+x1)/2,y01=(y0+y1)/2,x12=(x1+x2)/2,y12=(y1+y2)/2,x23=(x2+x3)/2,y23=(y2+y3)/2,xa=(x01+x12)/2,ya=(y01+y12)/2,xb=(x12+x23)/2,yb=(y12+y23)/2,mx=(xa+xb)/2,my=(ya+yb)/2;
    _tess_cb(pts,np,x0,y0,x01,y01,xa,ya,mx,my,fsq,n+1);_tess_cb(pts,np,mx,my,xb,yb,x23,y23,x3,y3,fsq,n+1);
}

static Vector2* _flatten(stbtt_vertex*v,int nv,float flat,int**cl,int*nc){
    float fsq=flat*flat;int n=0;for(int i=0;i<nv;++i)if(v[i].type==STBTT_vmove)++n;
    *nc=n;if(!n)return 0;*cl=(int*)TempAlloc(sizeof(int)*(size_t)n);Vector2*pts=0;int np=0;
    for(int pass=0;pass<2;++pass){float x=0,y=0;int start=0;n=-1;if(pass==1){pts=(Vector2*)TempAlloc((size_t)np*sizeof(Vector2));if(!pts)goto err;}np=0;
        for(int i=0;i<nv;++i){switch(v[i].type){
            case STBTT_vmove:if(n>=0)(*cl)[n]=np-start;start=np;++n;x=v[i].x;y=v[i].y;_add_pt(pts,np++,x,y);break;
            case STBTT_vline:x=v[i].x;y=v[i].y;_add_pt(pts,np++,x,y);break;
            case STBTT_vcurve:_tess_c(pts,&np,x,y,v[i].cx,v[i].cy,v[i].x,v[i].y,fsq,0);x=v[i].x;y=v[i].y;break;
            case STBTT_vcubic:_tess_cb(pts,&np,x,y,v[i].cx,v[i].cy,v[i].cx1,v[i].cy1,v[i].x,v[i].y,fsq,0);x=v[i].x;y=v[i].y;break;}}
        (*cl)[n]=np-start;}return pts;
    err:TempFree(pts);TempFree(*cl);*cl=0;*nc=0;return NULL;
}

static void _rasterize(stbtt__bitmap*res,Vector2*pts,int*wc,int nw,float sx,float sy,float shx,float shy,int ox,int oy,int inv){
    float ysi=inv?-sy:sy;stbtt__edge*e;int n=0,i,j,k;for(i=0;i<nw;++i)n+=wc[i];
    e=(stbtt__edge*)TempAlloc(sizeof(*e)*((size_t)n+1));if(!e)return;n=0;int m=0;
    for(i=0;i<nw;++i){Vector2*p=pts+m;m+=wc[i];j=wc[i]-1;for(k=0;k<wc[i];j=k++){int a=k,b=j;if(p[j].y==p[k].y)continue;e[n].invert=0;if(inv?p[j].y>p[k].y:p[j].y<p[k].y){e[n].invert=1;a=j;b=k;}e[n].x0=p[a].x*sx+shx;e[n].y0=p[a].y*ysi+shy;e[n].x1=p[b].x*sx+shx;e[n].y1=p[b].y*ysi+shy;++n;}}
    _esort(e,n);_rse(res,e,n,ox,oy);TempFree(e);
}

void stbtt_MakeGlyphBitmapSubpixel(const stbtt_fontinfo*info,unsigned char*out,int ow,int oh,int ostr,float sx,float sy,float shx,float shy,int g){
    stbtt_vertex*v;int ix0,iy0,nv=stbtt_GetGlyphShape(info,g,&v);stbtt__bitmap gbm;
    stbtt_GetGlyphBitmapBoxSubpixel(info,g,sx,sy,shx,shy,&ix0,&iy0,0,0);gbm.pixels=out;gbm.w=ow;gbm.h=oh;gbm.stride=ostr;
    float scale=sx>sy?sy:sx;int wc=0;int*wl=NULL;Vector2*win=_flatten(v,nv,0.35f/scale,&wl,&wc);
    if(win){_rasterize(&gbm,win,wl,wc,sx,sy,shx,shy,ix0,iy0,1);TempFree(wl);TempFree(win);}if(v)TempFree(v);
}

typedef int stbrp_coord;
typedef struct{int width,height,x,y,bottom_y;}stbrp_context;
typedef struct{unsigned char x;}stbrp_node;
typedef struct{stbrp_coord x,y;int id,w,h,was_packed;}stbrp_rect;
static void stbrp_pack_rects(stbrp_context*con,stbrp_rect*rects,int n){
    int i;for(i=0;i<n;++i){if(con->x+rects[i].w>con->width){con->x=0;con->y=con->bottom_y;}if(con->y+rects[i].h>con->height)break;rects[i].x=con->x;rects[i].y=con->y;rects[i].was_packed=1;con->x+=rects[i].w;if(con->y+rects[i].h>con->bottom_y)con->bottom_y=con->y+rects[i].h;}for(;i<n;++i)rects[i].was_packed=0;
}

typedef struct{unsigned short x0,y0,x1,y1;float xoff,yoff,xadvance,xoff2,yoff2;}stbtt_packedchar;
typedef struct{float x0,y0,s0,t0,x1,y1,s1,t1;}stbtt_aligned_quad;
typedef struct{void*uac;void*pack_info;int width,height,stride_in_bytes,padding,skip_missing;unsigned int h_oversample,v_oversample;unsigned char*pixels;}stbtt_pack_context;
typedef struct{float font_size;int first_unicode_codepoint_in_range;int*array_of_unicode_codepoints;int num_chars;stbtt_packedchar*chardata_for_range;unsigned char h_oversample,v_oversample;}FPackRange;
static void stbtt_GetPackedQuad(const stbtt_packedchar*cd,int pw,int ph,int ci,float*xpos,float*ypos,stbtt_aligned_quad*q,int ai){
    float ipw=1.0f/pw,iph=1.0f/ph;const stbtt_packedchar*b=cd+ci;
    if(ai){float x=vfloor((*xpos+b->xoff)+0.5f),y=vfloor((*ypos+b->yoff)+0.5f);q->x0=x;q->y0=y;q->x1=x+b->xoff2-b->xoff;q->y1=y+b->yoff2-b->yoff;}
    else{q->x0=*xpos+b->xoff;q->y0=*ypos+b->yoff;q->x1=*xpos+b->xoff2;q->y1=*ypos+b->yoff2;}
    q->s0=b->x0*ipw;q->t0=b->y0*iph;q->s1=b->x1*ipw;q->t1=b->y1*iph;*xpos+=b->xadvance;
}

int stbtt_PackBegin(stbtt_pack_context*spc,unsigned char*px,int pw,int ph,int str,int pad,void*a){
    stbrp_context*ctx=(stbrp_context*)TempAlloc(sizeof(*ctx));*ctx=(stbrp_context){pw-pad,ph-pad,0,0,0};if(px)MemSetToValueForNBytes(px,0,(size_t)(pw*ph));
    return *spc=(stbtt_pack_context){a,ctx,pw,ph,str?str:pw,pad,0,1,1,px},1;
}

#define STBTT_MAX_OVERSAMPLE 8
#define OVER_MASK (STBTT_MAX_OVERSAMPLE-1)
static void _hpre(unsigned char*p,int w,int h,int str,unsigned int kw){for(int j=0;j<h;++j,p+=str){unsigned char buf[STBTT_MAX_OVERSAMPLE]={0};int tot=0;for(int i=0;i<w;++i){if(i<=w-(int)kw){tot+=p[i]-buf[i&OVER_MASK];buf[(i+kw)&OVER_MASK]=p[i];}else tot-=buf[i&OVER_MASK];p[i]=(unsigned char)(tot/kw);}}}
static void _vpre(unsigned char*p,int w,int h,int str,unsigned int kw){for(int j=0;j<w;++j,++p){unsigned char buf[STBTT_MAX_OVERSAMPLE]={0};int tot=0;for(int i=0;i<h;++i){if(i<=h-(int)kw){tot+=p[i*str]-buf[i&OVER_MASK];buf[(i+kw)&OVER_MASK]=p[i*str];}else tot-=buf[i&OVER_MASK];p[i*str]=(unsigned char)(tot/kw);}}}
static float _oshift(int os){return os?-(float)(os-1)/(2.0f*(float)os):0.0f;}
static int stbtt_PackFontRangesGatherRects(stbtt_pack_context*spc,const stbtt_fontinfo*info,FPackRange*ranges,int nr,stbrp_rect*rects){
    int mga=0,k=0;for(int i=0;i<nr;++i){float fh=ranges[i].font_size,sc=fh>0?stbtt_ScaleForPixelHeight(info,fh):stbtt_ScaleForMappingEmToPixels(info,-fh);ranges[i].h_oversample=(unsigned char)spc->h_oversample;ranges[i].v_oversample=(unsigned char)spc->v_oversample;
        for(int j=0;j<ranges[i].num_chars;++j){int x0,y0,x1,y1,cp=ranges[i].array_of_unicode_codepoints?ranges[i].array_of_unicode_codepoints[j]:ranges[i].first_unicode_codepoint_in_range+j,g=stbtt_FindGlyphIndex(info,cp);
            if(g==0&&(spc->skip_missing||mga)){rects[k].w=rects[k].h=0;}else{stbtt_GetGlyphBitmapBoxSubpixel(info,g,sc*(float)spc->h_oversample,sc*(float)spc->v_oversample,0,0,&x0,&y0,&x1,&y1);rects[k].w=(stbrp_coord)(x1-x0+spc->padding+(int)spc->h_oversample-1);rects[k].h=(stbrp_coord)(y1-y0+spc->padding+(int)spc->v_oversample-1);if(g==0)mga=1;}++k;}}return k;
}

static int stbtt_PackFontRangesRenderIntoRects(stbtt_pack_context*spc,const stbtt_fontinfo*info,FPackRange*ranges,int nr,stbrp_rect*rects){
    int i,j,k=0,mg=-1,rv=1,oh=(int)spc->h_oversample,ov=(int)spc->v_oversample;
    for(i=0;i<nr;++i){float fh=ranges[i].font_size,sc=fh>0?stbtt_ScaleForPixelHeight(info,fh):stbtt_ScaleForMappingEmToPixels(info,-fh),rh,rv2,sbx,sby;spc->h_oversample=ranges[i].h_oversample;spc->v_oversample=ranges[i].v_oversample;rh=1.0f/(float)spc->h_oversample;rv2=1.0f/(float)spc->v_oversample;sbx=(float)_oshift((int)spc->h_oversample);sby=(float)_oshift((int)spc->v_oversample);
        for(j=0;j<ranges[i].num_chars;++j){stbrp_rect*r=&rects[k];
            if(r->was_packed&&r->w&&r->h){stbtt_packedchar*bc=&ranges[i].chardata_for_range[j];int adv,lsb,x0,y0,x1,y1,cp=ranges[i].array_of_unicode_codepoints?ranges[i].array_of_unicode_codepoints[j]:ranges[i].first_unicode_codepoint_in_range+j,g=stbtt_FindGlyphIndex(info,cp);stbrp_coord pad=(stbrp_coord)spc->padding;r->x+=pad;r->y+=pad;r->w-=pad;r->h-=pad;stbtt_GetGlyphHMetrics(info,g,&adv,&lsb);stbtt_GetGlyphBitmapBox(info,g,sc*(float)spc->h_oversample,sc*(float)spc->v_oversample,&x0,&y0,&x1,&y1);stbtt_MakeGlyphBitmapSubpixel(info,spc->pixels+r->x+r->y*spc->stride_in_bytes,r->w-(int)spc->h_oversample+1,r->h-(int)spc->v_oversample+1,spc->stride_in_bytes,sc*(float)spc->h_oversample,sc*(float)spc->v_oversample,0,0,g);if(spc->h_oversample>1)_hpre(spc->pixels+r->x+r->y*spc->stride_in_bytes,r->w,r->h,spc->stride_in_bytes,spc->h_oversample);if(spc->v_oversample>1)_vpre(spc->pixels+r->x+r->y*spc->stride_in_bytes,r->w,r->h,spc->stride_in_bytes,spc->v_oversample);bc->x0=(unsigned short)r->x;bc->y0=(unsigned short)r->y;bc->x1=(unsigned short)(r->x+r->w);bc->y1=(unsigned short)(r->y+r->h);bc->xadvance=sc*(float)adv;bc->xoff=(float)x0*rh+sbx;bc->yoff=(float)y0*rv2+sby;bc->xoff2=(float)(x0+r->w)*rh+sbx;bc->yoff2=(float)(y0+r->h)*rv2+sby;if(g==0)mg=j;}else if(spc->skip_missing){rv=0;}else if(r->was_packed&&!r->w&&!r->h&&mg>=0){ranges[i].chardata_for_range[j]=ranges[i].chardata_for_range[mg];}else rv=0;++k;}}
    spc->h_oversample=(unsigned int)oh;spc->v_oversample=(unsigned int)ov;return rv;
}

static int stbtt_PackFontRanges(stbtt_pack_context*spc,const unsigned char*fontdata,int fi,FPackRange*ranges,int nr){
    stbtt_fontinfo info;int i,j,n=0,rv=1;stbrp_rect*rects;
    for(i=0;i<nr;++i)for(j=0;j<ranges[i].num_chars;++j)ranges[i].chardata_for_range[j].x0=ranges[i].chardata_for_range[j].y0=ranges[i].chardata_for_range[j].x1=ranges[i].chardata_for_range[j].y1=0;
    for(i=0;i<nr;++i)n+=ranges[i].num_chars;rects=(stbrp_rect*)TempAlloc(sizeof(*rects)*(size_t)n);if(!rects)return 0;
    info.userdata=spc->uac;stbtt_InitFont_internal(&info,(unsigned char*)fontdata,stbtt_GetFontOffsetForIndex(fontdata,fi));
    n=stbtt_PackFontRangesGatherRects(spc,&info,ranges,nr,rects);stbrp_pack_rects(spc->pack_info,rects,n);rv=stbtt_PackFontRangesRenderIntoRects(spc,&info,ranges,nr,rects);TempFree(rects);return rv;
}

int numPackedGlyphs=0,numPackedGlyphsStopD=0;
u32 fontAtlasTex,fontAtlasTexStopD;
stbtt_packedchar fontPackedChar[MAX_GLYPHS],fontPackedCharStopD[MAX_GLYPHS];
float fixedNumberAdvanceWidth=0.0f,fixedNumberAdvanceWidthStopD=0.0f;
static const char* fallbackFontPaths[]={"./Fonts/FreeSerifBold.ttf","./Fonts/cambriab.ttf","./Fonts/NotoSansCJK-Bold.ttc"}, *fontPaths[]={"./Fonts/SystemShockText.ttf","./Fonts/StopD.ttf"};
static stbtt_fontinfo fontInfo[5]; static unsigned char *fontData[5]; static char uiTextBuffer[TEXT_BUFFER_SIZE];
typedef struct{char*path;unsigned char*data;size_t size;stbtt_fontinfo info;}LoadedFont;
LoadedFont fallbackFonts[3];
typedef struct{i32 first,count,startIndex;}GlyphRange;
GlyphRange fontRanges[]     ={{0x0020,0x7E - 0x20 + 1,0},{0x00A0,0xFF - 0xA0 + 1,95},{0x0400,0x04FF - 0x0400 + 1,95+96},{0x3040,0x30FF - 0x3040 + 1,95+96+256}};
GlyphRange fontRangesStopD[]={{0x0020,0x7E - 0x20 + 1,0},{0x00A0,0xFF - 0xA0 + 1,95},{0x0400,0x04FF - 0x0400 + 1,95+96},{0x3040,0x30FF - 0x3040 + 1,95+96+256}};
i32 numFontRanges=sizeof(fontRanges)/sizeof(fontRanges[0]);
__attribute__((pure)) i32 CodepointToPackedIndex(i32 cp,int fontID){
    if(cp<32)cp=32;if(cp>=447)cp=446;
    const GlyphRange*ranges=(fontID==FONT_STOPD)?fontRangesStopD:fontRanges;
    i32 total=(fontID==FONT_STOPD)?numPackedGlyphsStopD:numPackedGlyphs;
    for(i32 i=0;i<numFontRanges;i++){if(cp>=ranges[i].first&&cp<ranges[i].first+ranges[i].count){i32 idx=ranges[i].startIndex+vmax((cp-ranges[i].first),0);if(idx<total)return idx;}}
    return 0;
}
static LoadedFont LoadFallbackFont(const char*path,int fii,int ci){
    OsFileHandle fd;int fsz;fontData[fii]=OS_OpenAndAllocateFileBufferReadonly(path,&fd,&fsz);
    int off=stbtt_GetFontOffsetForIndex(fontData[fii],ci);if(off<0){DualLogError("Invalid collection index %d for font %s\n",ci,path);OS_Exit(1);}
    if(!stbtt_InitFont_internal(&fontInfo[fii],fontData[fii],off)){DualLogError("Failed to init font at index %d in %s\n",ci,path);OS_Exit(1);}
    return (LoadedFont){(char*)path,fontData[fii],fsz,fontInfo[fii]};
}
static int GetGlyphAndFont(u32 cp,stbtt_fontinfo**outFont,u8 fontID){
    int g=stbtt_FindGlyphIndex(fontID==FONT_STOPD?&fontInfo[1]:&fontInfo[0],cp);if(g){*outFont=fontID==FONT_STOPD?&fontInfo[1]:&fontInfo[0];return g;}
    for(int i=0;i<3;i++){g=stbtt_FindGlyphIndex(&fallbackFonts[i].info,cp);if(g){*outFont=&fallbackFonts[i].info;return g;}}
    return 0;
}

static void GenerateAndBindTexture(u32 *id, i32 internalFormat, i32 width, i32 height, u32 format, u32 type, i32 filt, unsigned char* bmp);
static void InitFontAtlasses(void){
    double t0=get_time();DualLog("Loading    5 fonts...");
    OsFileHandle fd1,fd2;int sz1,sz2;
    fontData[0]=OS_OpenAndAllocateFileBufferReadonly(fontPaths[0],&fd1,&sz1);
    fontData[1]=OS_OpenAndAllocateFileBufferReadonly(fontPaths[1],&fd2,&sz2);
    if(!stbtt_InitFont_internal(&fontInfo[0],fontData[0],0)){DualLogError("%s font init failed\n",fontPaths[0]);OS_Exit(1);}
    if(!stbtt_InitFont_internal(&fontInfo[1],fontData[1],0)){DualLogError("%s font init failed\n",fontPaths[1]);OS_Exit(1);}
    fallbackFonts[0]=LoadFallbackFont(fallbackFontPaths[0],2,0);
    fallbackFonts[1]=LoadFallbackFont(fallbackFontPaths[1],3,0);
    fallbackFonts[2]=LoadFallbackFont(fallbackFontPaths[2],4,2);
    unsigned char*bmp=OS_Alloc(FONT_ATLAS_SIZE*FONT_ATLAS_SIZE);

    // Primary atlas
    stbtt_pack_context pc;stbtt_PackBegin(&pc,bmp,FONT_ATLAS_SIZE,FONT_ATLAS_SIZE,0,16,NULL);pc.h_oversample=4;pc.v_oversample=4;pc.skip_missing=1;numPackedGlyphs=0;
    for(int r=0;r<numFontRanges;++r){fontRanges[r].startIndex=numPackedGlyphs;
        for(int i=0;i<fontRanges[r].count;++i){if(numPackedGlyphs>=MAX_GLYPHS)break;u32 cp=fontRanges[r].first+i;stbtt_fontinfo*font=&fontInfo[0];unsigned char*data=fontData[0];
            int g=stbtt_FindGlyphIndex(font,cp);if(!g){g=GetGlyphAndFont(cp,&font,FONT_NORMAL);if(!g)continue;data=(font==&fontInfo[0])?fontData[0]:((LoadedFont*)((char*)font-__builtin_offsetof(LoadedFont,info)))->data;}
            float h=20.0f;if(font!=&fontInfo[0])h*=1.2f;FPackRange range={h,cp,NULL,1,&fontPackedChar[numPackedGlyphs],0,0};stbtt_PackFontRanges(&pc,data,0,&range,1);
            int idx=numPackedGlyphs++;if(cp>='0'&&cp<='9')fixedNumberAdvanceWidth=vmax(fixedNumberAdvanceWidth,fontPackedChar[idx].xadvance);}}
    TempFree(pc.pack_info);GenerateAndBindTexture(&fontAtlasTex,0x8229/*GL_R8*/,FONT_ATLAS_SIZE,FONT_ATLAS_SIZE,0x1903/*GL_RED*/,GL_UNSIGNED_BYTE,0x2601/*GL_LINEAR*/,bmp);

    // Secondary atlas
    MemSetToValueForNBytes(bmp,0,FONT_ATLAS_SIZE*FONT_ATLAS_SIZE);
    stbtt_pack_context pc2;stbtt_PackBegin(&pc2,bmp,FONT_ATLAS_SIZE,FONT_ATLAS_SIZE,0,16,NULL);pc2.h_oversample=4;pc2.v_oversample=4;pc2.skip_missing=1;numPackedGlyphsStopD=0;
    for(int r=0;r<numFontRanges;++r){fontRangesStopD[r].startIndex=numPackedGlyphsStopD;
        for(int i=0;i<fontRangesStopD[r].count;++i){if(numPackedGlyphsStopD>=MAX_GLYPHS)break;u32 cp=fontRangesStopD[r].first+i;stbtt_fontinfo*font=&fontInfo[1];unsigned char*data=fontData[1];
            int g=stbtt_FindGlyphIndex(font,cp);if(!g){g=GetGlyphAndFont(cp,&font,FONT_STOPD);if(!g)continue;data=(font==&fontInfo[0])?fontData[0]:((LoadedFont*)((char*)font-__builtin_offsetof(LoadedFont,info)))->data;}
            float h=54.0f;if(font!=&fontInfo[1])h*=1.2f;FPackRange range={h,cp,NULL,1,&fontPackedCharStopD[numPackedGlyphsStopD],0,0};stbtt_PackFontRanges(&pc2,data,0,&range,1);
            int idx=numPackedGlyphsStopD++;if(cp>='0'&&cp<='9')fixedNumberAdvanceWidthStopD=vmax(fixedNumberAdvanceWidthStopD,fontPackedCharStopD[idx].xadvance);}}
    TempFree(pc2.pack_info);GenerateAndBindTexture(&fontAtlasTexStopD,0x8229/*GL_R8*/,FONT_ATLAS_SIZE,FONT_ATLAS_SIZE,0x1903/*GL_RED*/,GL_UNSIGNED_BYTE,0x2601/*GL_LINEAR*/,bmp);
    
    OS_DeallocateRAM(bmp,FONT_ATLAS_SIZE*FONT_ATLAS_SIZE);
    DualLog(" took %f s\n",get_time()-t0);
}

char *strncpy(char*dest,const char*src,size_t n); u16 logImages=1272;
size_t utf16le_to_utf8(const u8*src,size_t slen,char*dst,size_t dlen){
    size_t dp=0,sp=0;
    while(sp<slen&&dp<dlen-4){if(sp+1>=slen)break;u32 c=(u32)src[sp+1]<<8|src[sp];sp+=2;
        if(c<0x80){dst[dp++]=(char)c;}
        else if(c<0x800){dst[dp++]=(char)(0xC0|(c>>6));dst[dp++]=(char)(0x80|(c&0x3F));}
        else if(c<0x10000){dst[dp++]=(char)(0xE0|(c>>12));dst[dp++]=(char)(0x80|((c>>6)&0x3F));dst[dp++]=(char)(0x80|(c&0x3F));}
        else continue;}
    dst[dp]='\0';return dp;
}

static const char* localizations[8]={"./Data/text_english.txt","./Data/text_espanol.txt","./Data/text_deutsch.txt","./Data/text_francais.txt","./Data/text_nihongo.txt","./Data/text_russkiy.txt","./Data/text_italiano.txt","./Data/text_portugues.txt"};
void LoadTextForLanguage(u8 lang){
    char tf[256]={0};strncpy(tf,localizations[lang<8?lang:0],255);
    OsFileHandle dfd=OS_INVALID_HANDLE;int asz=0;
    if(Sys_Text.file_data){OS_DeallocateRAM(Sys_Text.file_data,Sys_Text.file_size);Sys_Text.file_data=NULL;Sys_Text.file_size=0;}
    Sys_Text.file_data=(u8*)OS_OpenAndAllocateFileBufferReadonly(tf,&dfd,&asz);if(!Sys_Text.file_data||asz<=0){DualLogError("Failed to load text file: %s\n",tf);return;}
    Sys_Text.file_size=(size_t)asz;
    size_t dp=0;int utf16=0;
    if(Sys_Text.file_size>=2&&Sys_Text.file_data[0]==0xFF&&Sys_Text.file_data[1]==0xFE){dp=2;utf16=1;}
    else if(Sys_Text.file_size>=3&&Sys_Text.file_data[0]==0xEF&&Sys_Text.file_data[1]==0xBB&&Sys_Text.file_data[2]==0xBF){dp=3;}
    else{size_t nl=0;for(size_t i=1;i<Sys_Text.file_size&&i<1024;i+=2)if(Sys_Text.file_data[i]==0)nl++;if(nl*3>Sys_Text.file_size)utf16=1;}
    char line[TEXT_LOCALIZATION_MAX_LENGTH];int ln=0;
    while(dp<Sys_Text.file_size){size_t ls=dp;
        if(utf16){while(dp+1<Sys_Text.file_size){u16 ch=Sys_Text.file_data[dp]|(Sys_Text.file_data[dp+1]<<8);dp+=2;if(ch=='\r'||ch=='\n'){if(ch=='\r'&&dp+1<Sys_Text.file_size){u16 nx=Sys_Text.file_data[dp]|(Sys_Text.file_data[dp+1]<<8);if(nx=='\n')dp+=2;}break;}}}
        else{while(dp<Sys_Text.file_size){u8 c=Sys_Text.file_data[dp];if(c=='\r'||c=='\n'){if(c=='\r'&&dp+1<Sys_Text.file_size&&Sys_Text.file_data[dp+1]=='\n')++dp;++dp;break;}++dp;}}
        size_t ll=dp-ls;if(ll==0){if(ln<TEXT_STRING_COUNT)Sys_Text.stringTable[ln][0]='\0';++ln;continue;}
        if(utf16)utf16le_to_utf8(&Sys_Text.file_data[ls],ll,line,sizeof(line));else{if(ll>=sizeof(line))ll=sizeof(line)-1; CopyMemoryFromBtoAForNBytes(line,&Sys_Text.file_data[ls],ll);line[ll]='\0';}
        size_t sl=GetStringLength(line);while(sl>0&&(line[sl-1]=='\r'||line[sl-1]=='\n'))line[--sl]='\0';
        if(sl==0){if(ln<TEXT_STRING_COUNT)Sys_Text.stringTable[ln][0]='\0';++ln;continue;}
        if(ln<TEXT_STRING_COUNT) {CopyMemoryFromBtoAForNBytes(Sys_Text.stringTable[ln],line,sl);Sys_Text.stringTable[ln][sl]='\0';++ln;} }
}

static inline __attribute__((always_inline)) int StringToIntLen(const char*str,size_t len){int v=0;for(size_t i=0;i<len&&str[i]>='0'&&str[i]<='9';++i)v=v*10+(str[i]-'0');return v;}
static const char* logLocalizations[8]={"./Data/logs_text_english.txt","./Data/logs_text_espanol.txt","./Data/logs_text_deutsch.txt","./Data/logs_text_francais.txt","./Data/logs_text_nihongo.txt","./Data/logs_text_russkiy.txt","./Data/logs_text_italiano.txt","./Data/logs_text_portugues.txt"};
void LoadLogTextForLanguage(u8 lang){
    MemSetToValueForNBytes(Sys_Text.audioLogImagesRefIndicesLH,0,TEXT_LOGS_COUNT*sizeof(u16));MemSetToValueForNBytes(Sys_Text.audioLogImagesRefIndicesRH,0,TEXT_LOGS_COUNT*sizeof(u16));MemSetToValueForNBytes(Sys_Text.audioLogType,0,TEXT_LOGS_COUNT*sizeof(u8));MemSetToValueForNBytes(Sys_Text.audioLogLevelFound,0,TEXT_LOGS_COUNT*sizeof(u8));
    char tf[256]={0};strncpy(tf,logLocalizations[lang<8?lang:0],255);
    OsFileHandle dfd=OS_INVALID_HANDLE;int asz=0;
    if(Sys_Text.filelog_data){OS_DeallocateRAM(Sys_Text.filelog_data,Sys_Text.filelog_size);Sys_Text.filelog_data=NULL;Sys_Text.filelog_size=0;}
    Sys_Text.filelog_data=(u8*)OS_OpenAndAllocateFileBufferReadonly(tf,&dfd,&asz);if(!Sys_Text.filelog_data||asz<=0){DualLogError("Failed to load log text file: %s\n",tf);return;}
    Sys_Text.filelog_size=(size_t)asz;
    size_t dp=0;int utf16=0;
    if(Sys_Text.filelog_size>=2&&Sys_Text.filelog_data[0]==0xFF&&Sys_Text.filelog_data[1]==0xFE){dp=2;utf16=1;}
    else if(Sys_Text.filelog_size>=3&&Sys_Text.filelog_data[0]==0xEF&&Sys_Text.filelog_data[1]==0xBB&&Sys_Text.filelog_data[2]==0xBF){dp=3;}
    else{int nl=0;for(size_t i=1;i<Sys_Text.filelog_size&&i<2048;i+=2)if(Sys_Text.filelog_data[i]==0)nl++;if(nl>(int)(Sys_Text.filelog_size/5))utf16=1;}
    char line[1024];
    while(dp<Sys_Text.filelog_size){size_t ls=dp;
        if(utf16){while(dp+1<Sys_Text.filelog_size){u16 ch=Sys_Text.filelog_data[dp]|(Sys_Text.filelog_data[dp+1]<<8);dp+=2;if(ch=='\r'||ch=='\n'){if(ch=='\r'&&dp+1<Sys_Text.filelog_size){u16 nx=Sys_Text.filelog_data[dp]|(Sys_Text.filelog_data[dp+1]<<8);if(nx=='\n')dp+=2;}break;}}}
        else{while(dp<Sys_Text.filelog_size){u8 c=Sys_Text.filelog_data[dp];if(c=='\r'||c=='\n'){if(c=='\r'&&dp+1<Sys_Text.filelog_size&&Sys_Text.filelog_data[dp+1]=='\n')++dp;++dp;break;}++dp;}}
        size_t ll=dp-ls;if(!ll)continue;
        if(utf16)utf16le_to_utf8(&Sys_Text.filelog_data[ls],ll,line,sizeof(line));else{if(ll>=sizeof(line))ll=sizeof(line)-1;CopyMemoryFromBtoAForNBytes(line,&Sys_Text.filelog_data[ls],ll);line[ll]='\0';}
        size_t sl=GetStringLength(line);while(sl>0&&(line[sl-1]=='\r'||line[sl-1]=='\n'))line[--sl]='\0';if(!sl)continue;
        int li=-1,ilh=-1,irh=-1,lt=0,lf=0,fi=0;char*pos=line;
        while(*pos&&fi<32){while(*pos==' ')++pos;char*st=pos;int q=(*pos=='"');if(q)++pos;while(*pos){if(*pos==','&&!q)break;if(*pos=='"'&&q){if(pos[1]==','){pos++;break;}if(pos[1]=='"'){pos+=2;continue;}}++pos;}char*en=pos;if(q&&*en=='"')--en;size_t tl=(size_t)(en-st);if(!tl){if(*pos==',')++pos;fi++;continue;}
            switch(fi){case 0:li=StringToIntLen(st,tl);if(li<0||li>=TEXT_LOGS_COUNT)goto nxt;break;case 1:ilh=StringToIntLen(st,tl);break;case 2:irh=StringToIntLen(st,tl);break;case 3:if(li>=0&&li<TEXT_LOGS_COUNT)StringCopyInto_A_SubstringFrom_B(Sys_Global.audiologNames[li],tl,st,sizeof(Sys_Global.audiologNames[0]));break;case 4:if(li>=0&&li<TEXT_LOGS_COUNT)StringCopyInto_A_SubstringFrom_B(Sys_Global.audiologSenders[li],tl,st,sizeof(Sys_Global.audiologSenders[0]));break;case 5:if(li>=0&&li<TEXT_LOGS_COUNT)StringCopyInto_A_SubstringFrom_B(Sys_Global.audiologSubjects[li],tl,st,sizeof(Sys_Global.audiologSubjects[0]));break;case 6:lt=StringToIntLen(st,tl);break;case 7:lf=StringToIntLen(st,tl);break;default:if(li>=0&&li<TEXT_LOGS_COUNT){char*d=Sys_Global.audioLogSpeech2Text[li];size_t cur=GetStringLength(d);if(cur>0&&cur<TEXT_LOCALIZATION_MAX_LENGTH*4-2){d[cur++]=',';d[cur]='\0';}size_t left=TEXT_LOCALIZATION_MAX_LENGTH*4-cur-1;if(left>0){size_t cl=tl>left?left:tl;StringCopyInto_A_SubstringFrom_B(d+cur,cl,st,left+1);}}break;}
            if(*pos==',')++pos;fi++;}
        if(li>=0&&li<TEXT_LOGS_COUNT){Sys_Text.audioLogImagesRefIndicesLH[li]=(u16)ilh;Sys_Text.audioLogImagesRefIndicesRH[li]=(u16)irh;Sys_Text.audioLogType[li]=(u8)lt;Sys_Text.audioLogLevelFound[li]=(u8)lf;}
        nxt:continue;}
}

Color textColors[] = {
    {         1.0f,         1.0f,          1.0f, 1.0f}, // 0 White                       TEXT_WHITE
    { 0.890196078f, 0.874509804f,          0.0f, 1.0f}, // 1 Yellow                      TEXT_YELLOW
    { 0.623529412f, 0.611764706f,          0.0f, 1.0f}, // 2 Dark Yellow (Yellow * 0.7f) TEXT_DARK_YELLOW
    { 0.372549020f, 0.654901961f,  0.168627451f, 1.0f}, // 3 Green                       TEXT_GREEN
    { 0.917647059f, 0.137254902f,  0.168627451f, 1.0f}, // 4 Red                         TEXT_RED
    {         1.0f, 0.498039216f,          0.0f, 1.0f}, // 5 Orange                      TEXT_ORANGE
    { 0.674509804f, 0.058823529f,  0.070588235f, 1.0f}, // 6 StopD Red                   TEXT_STOPD_RED
    { 0.941176471f, 0.282352941f,  0.298039216f, 1.0f}, // 7 StopD Red Highlight         TEXT_STOPD_RED_HIGHLIGHT
    { 0.909803922f, 0.203921569f,  0.219607843f, 1.0f}, // 8 StopD Red Pause Title       TEXT_STOPD_RED_PAUSETITLE
    { 0.470588235f, 0.721568627f,  0.172549020f, 1.0f}, // 9 Green Menu Title            TEXT_GREEN_MENU
    { 0.137254902f, 0.356862745f,  0.109803922f, 1.0f}, // 10 Green Menu Title Shadow    TEXT_GREEN_MENU_SHADOW
    { 0.239215686f, 0.466666667f,  0.129411765f, 1.0f}, // 11 Green Menu Title Glow      TEXT_GREEN_MENU_GLOW
    { 0.392156863f, 0.031372549f,  0.039215686f, 1.0f}  // 12 Red Menu Text Dark         TEXT_RED_MENU
};

float textVertexData[8192];
void RenderFormattedText(i16 x,i16 y,u32 color,u8 fontID,float scaleInput,const char* restrict format,...){
    float scale=scaleInput;
    va_list args;__builtin_va_start(args,format);StringFormatV(uiTextBuffer,TEXT_BUFFER_SIZE,format,args);__builtin_va_end(args);
    glUseProgram(Sys_Render.textShaderProgram);
    glUniform4f(3,textColors[color].r,textColors[color].g,textColors[color].b,textColors[color].a);
    glBindTextureUnit(6,fontID==FONT_STOPD?fontAtlasTexStopD:fontAtlasTex);
    glUniform2f(4,1.0f/(float)FONT_ATLAS_SIZE,1.0f/(float)FONT_ATLAS_SIZE);glUniform1ui(2,fontID);glUniform1i(1,6);
    glBindVertexArray(Sys_Render.textVAO);
    size_t vc=0;const char*p=uiTextBuffer;float xpos=x,ypos=y+(16*scale),ls=22*scale;stbtt_aligned_quad q;int cc=0;
    float puv=10.0f/(float)FONT_ATLAS_SIZE,bw=2.0f;
    while(*p){const unsigned char*s=(const unsigned char*)p;u32 cp=0;
        if(*s<0x80){cp=*s++;}
        else if((*s&0xE0)==0xC0){cp=(*s&0x1F)<<6;cp|=(s[1]&0x3F);s+=2;}
        else if((*s&0xF0)==0xE0){cp=(*s&0x0F)<<12;cp|=(s[1]&0x3F)<<6;cp|=(s[2]&0x3F);s+=3;}
        else if((*s&0xF8)==0xF0){cp=(*s&0x07)<<18;cp|=(s[1]&0x3F)<<12;cp|=(s[2]&0x3F)<<6;cp|=(s[3]&0x3F);s+=4;}
        else s++;
        p=(const char*)s;cc++;
        if(cp=='\n'||cc>120){xpos=x;ypos+=ls;cc=0;continue;}
        int idx=CodepointToPackedIndex(cp,fontID);
        if(fontID==FONT_STOPD)stbtt_GetPackedQuad(fontPackedCharStopD,FONT_ATLAS_SIZE,FONT_ATLAS_SIZE,idx,&xpos,&ypos,&q,1);
        else stbtt_GetPackedQuad(fontPackedChar,FONT_ATLAS_SIZE,FONT_ATLAS_SIZE,idx,&xpos,&ypos,&q,1);
        float vx0=q.x0*scale-bw,vy0=q.y0*scale-bw,vx1=q.x1*scale+bw,vy1=q.y1*scale+bw;
        float s0=q.s0-puv,t0=q.t0-puv,s1=q.s1+puv,t1=q.t1+puv,z=0.0f;
        float tv[30]={vx0,vy0,z,s0,t0,vx1,vy1,z,s1,t1,vx1,vy0,z,s1,t0,vx0,vy0,z,s0,t0,vx0,vy1,z,s0,t1,vx1,vy1,z,s1,t1};
        CopyMemoryFromBtoAForNBytes(textVertexData+vc*30,tv,sizeof(tv));vc++;
        if(cp>='0'&&cp<='9'){if(fontID==FONT_STOPD)xpos=q.x0+fixedNumberAdvanceWidthStopD;else xpos=q.x0+fixedNumberAdvanceWidth;}
    }
    if(vc){glNamedBufferData(Sys_Render.textVBO,vc*30*sizeof(float),textVertexData,GL_DYNAMIC_DRAW);glDrawArrays(0x0004/*GL_TRIANGLES*/,0,vc*6);}
}

void RenderUIImage(i16 x, i16 y, i16 width, i16 height, u32 texIndex) {
    glUseProgram(Sys_Render.uiShaderProgram);
    glBindVertexArray(Sys_Render.textVAO);
    glUniform1ui(0,texIndex);
    glBindBuffer(GL_ARRAY_BUFFER,Sys_Render.textVBO);
    float x1=x + width, y1=y + height, z=0.0f;
    float vertices[30] = {x,y1,z,0.0f,0.0f,x1,y,z,1.0f,1.0f,x1,y1,z,1.0f,0.0f,x,y1,z,0.0f,0.0f,x,y,z,0.0f,1.0f,x1,y,z,1.0f,1.0f};
    glBufferData(GL_ARRAY_BUFFER,30 * sizeof(float),vertices,GL_DYNAMIC_DRAW);
    glDrawArrays(0x0004/*GL_TRIANGLES*/,0,6);
    drawCallsRenderedThisFrame++; uiImageDrawCallsRenderedThisFrame++; verticesRenderedThisFrame += 6;    
    glBindBuffer(GL_ARRAY_BUFFER,0);
}

void ClearAll(void) {
    glBindFramebuffer(GL_FRAMEBUFFER,Sys_Render.gBufferFBO);
    glClearColor(0.0f,0.0f,0.0f,1.0f); glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER,Sys_Render.uiFBO);
    glClearColor(0.0f,0.0f,0.0f,0.0f); glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    glClearColor(0.0f,0.0f,0.0f,1.0f); glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

static inline __attribute__((always_inline,pure)) bool CursorIsOverBounds(float startX, float endX, float startY, float endY) { return Sys_Global.cursorPosition_x >= startX && Sys_Global.cursorPosition_x <= endX /* 0 == left */ && Sys_Global.cursorPosition_y >= endY && Sys_Global.cursorPosition_y <= startY; /* 0 == top */ }
void RenderLoadingProgress(i32 offset, const char * restrict text) { // Only adds 0.01secs to game startup time.
    ClearAll();
    RenderFormattedText(Sys_Settings.ScreenWidth / 2 - offset, Sys_Settings.ScreenHeight / 2 - 5, TEXT_WHITE, FONT_NORMAL,1.0f,text);
    glfwSwapBuffers();
}

char statusText[TEXT_BUFFER_SIZE];
void CenterStatusPrint(const char * restrict fmt, ...) {
    va_list args; __builtin_va_start(args, fmt); StringFormatV(statusText,TEXT_BUFFER_SIZE,fmt,args); __builtin_va_end(args);
    DualLog("%s\n",statusText);
    Sys_Global.statusTextDecayFinished = get_time() + 2.5; // 2.5 second decay time before text dissappears.
}

typedef struct { const char* name; int value; } InputElement;
InputElement inputElements[134] = {
    { "A", GLFW_KEY_A }, { "B", GLFW_KEY_B }, { "C", GLFW_KEY_C }, { "D", GLFW_KEY_D }, { "E", GLFW_KEY_E }, { "F", GLFW_KEY_F }, { "G", GLFW_KEY_G }, { "H", GLFW_KEY_H }, { "I", GLFW_KEY_I }, { "J", GLFW_KEY_J },
    { "K", GLFW_KEY_K }, { "L", GLFW_KEY_L }, { "M", GLFW_KEY_M }, { "N", GLFW_KEY_N }, { "O", GLFW_KEY_O }, { "P", GLFW_KEY_P }, { "Q", GLFW_KEY_Q }, { "R", GLFW_KEY_R }, { "S", GLFW_KEY_S }, { "T", GLFW_KEY_T },
    { "U", GLFW_KEY_U }, { "V", GLFW_KEY_V }, { "W", GLFW_KEY_W }, { "X", GLFW_KEY_X }, { "Y", GLFW_KEY_Y }, { "Z", GLFW_KEY_Z }, { "1", GLFW_KEY_1 }, { "2", GLFW_KEY_2 }, { "3", GLFW_KEY_3 }, { "4", GLFW_KEY_4 },
    { "5", GLFW_KEY_5 }, { "6", GLFW_KEY_6 }, { "7", GLFW_KEY_7 }, { "8", GLFW_KEY_8 }, { "9", GLFW_KEY_9 }, { "0", GLFW_KEY_0 }, { "UP ARROW", GLFW_KEY_UP }, { "DN ARROW", GLFW_KEY_DOWN }, { "LF ARROW", GLFW_KEY_LEFT }, { "RT ARROW", GLFW_KEY_RIGHT },
    { "NUM 1", GLFW_KEY_KP_1 }, { "NUM 2", GLFW_KEY_KP_2 }, { "NUM 3", GLFW_KEY_KP_3 }, { "NUM +", GLFW_KEY_KP_ADD }, { "ENTER", GLFW_KEY_ENTER }, { "RIGHT SHIFT", GLFW_KEY_RIGHT_SHIFT }, { "LEFT SHIFT", GLFW_KEY_LEFT_SHIFT }, { "RIGHT CTRL", GLFW_KEY_RIGHT_CONTROL }, { "LEFT CTRL", GLFW_KEY_LEFT_CONTROL }, { "RIGHT ALT", GLFW_KEY_RIGHT_ALT },
    { "LEFT ALT", GLFW_KEY_LEFT_ALT }, { "RIGHT CMD", GLFW_KEY_RIGHT_SUPER }, { "LEFT CMD", GLFW_KEY_LEFT_SUPER }, { "LMB", GLFW_MOUSE_BUTTON_1 }, { "RMB", GLFW_MOUSE_BUTTON_2 }, { "MMB", GLFW_MOUSE_BUTTON_3 }, { "MB 3", GLFW_MOUSE_BUTTON_4 }, { "MB 4", GLFW_MOUSE_BUTTON_5 }, { "MB 5", GLFW_MOUSE_BUTTON_6 }, { "MB 6", GLFW_MOUSE_BUTTON_7 },
    { "MB 7", GLFW_MOUSE_BUTTON_8 }, { "MB 8", GLFW_MOUSE_BUTTON_LAST }, { "JOY 0", GLFW_JOYSTICK_1 }, { "JOY 1", GLFW_JOYSTICK_2 }, { "JOY 2", GLFW_JOYSTICK_3 }, { "JOY 3", GLFW_JOYSTICK_4 }, { "JOY 4", GLFW_JOYSTICK_5 }, { "JOY 5", GLFW_JOYSTICK_6 }, { "JOY 6", GLFW_JOYSTICK_7 }, { "JOY 7", GLFW_JOYSTICK_8 },
    { "JOY 8", GLFW_JOYSTICK_9 }, { "JOY 9", GLFW_JOYSTICK_10 }, { "JOY 10", GLFW_JOYSTICK_11 }, { "JOY 11", GLFW_JOYSTICK_12 }, { "JOY 12", GLFW_JOYSTICK_13 }, { "JOY 13", GLFW_JOYSTICK_14 }, { "JOY 14", GLFW_JOYSTICK_15 }, { "JOY 15", GLFW_JOYSTICK_16 }, { "JOY 16", GLFW_HAT_UP }, { "JOY 17", GLFW_HAT_RIGHT },
    { "BACKSPACE", GLFW_KEY_BACKSPACE }, { "TAB", GLFW_KEY_TAB }, { "NUM ENTER", GLFW_KEY_KP_ENTER }, { "ESCAPE", GLFW_KEY_ESCAPE }, { "SPACE", GLFW_KEY_SPACE }, { "DELETE", GLFW_KEY_DELETE }, { "INSERT", GLFW_KEY_INSERT }, { "HOME", GLFW_KEY_HOME }, { "END", GLFW_KEY_END }, { "PAGE UP", GLFW_KEY_PAGE_UP },
    { "PAGE DN", GLFW_KEY_PAGE_DOWN }, { "F1", GLFW_KEY_F1 }, { "F2", GLFW_KEY_F2 }, { "F3", GLFW_KEY_F3 }, { "F4", GLFW_KEY_F4 }, { "F5", GLFW_KEY_F5 }, { "F6", GLFW_KEY_F6 }, { "F7", GLFW_KEY_F7 }, { "F8", GLFW_KEY_F8 }, { "F9", GLFW_KEY_F9 },
    { "F10", GLFW_KEY_F10 }, { "F11", GLFW_KEY_F11 }, { "F12", GLFW_KEY_F12 }, { "GRAVE", GLFW_KEY_GRAVE_ACCENT }, { "-", GLFW_KEY_MINUS }, { "=", GLFW_KEY_EQUAL }, { "[", GLFW_KEY_LEFT_BRACKET }, { "]", GLFW_KEY_RIGHT_BRACKET }, { "\\", GLFW_KEY_BACKSLASH }, { "/", GLFW_KEY_SLASH },
    { ".", GLFW_KEY_PERIOD }, { ",", GLFW_KEY_COMMA }, { ";", GLFW_KEY_SEMICOLON }, { "'", GLFW_KEY_APOSTROPHE }, { "CAPSLOCK", GLFW_KEY_CAPS_LOCK }, { "NUM 0", GLFW_KEY_KP_0 }, { "NUM 4", GLFW_KEY_KP_4 }, { "NUM 5", GLFW_KEY_KP_5 }, { "NUM 6", GLFW_KEY_KP_6 }, { "NUM 7", GLFW_KEY_KP_7 },
    { "NUM 8", GLFW_KEY_KP_8 }, { "NUM 9", GLFW_KEY_KP_9 }, { "NUM *", GLFW_KEY_KP_MULTIPLY }, { "NUM -", GLFW_KEY_KP_SUBTRACT }, { "NUM .", GLFW_KEY_KP_DECIMAL }, { "MENU", GLFW_KEY_MENU }, { "PAUSE", GLFW_KEY_PAUSE }, { "NUMLOCK", GLFW_KEY_NUM_LOCK }, { "MWHEEL +", 128 }, { "MWHEEL -", 129 }, // 128, 129, Handled special case for mouse wheel + / - respectively
    { "PRINT", GLFW_KEY_PRINT_SCREEN }, { "JOY 18", GLFW_HAT_DOWN }, { "JOY 19", GLFW_HAT_LEFT },{ "UNUSED", 0 } //, {}
};

typedef enum { SETTING_U8, SETTING_U16, SETTING_INPUT } SettingType;
typedef struct { const char* name; void* ptr; SettingType type; } Setting;
#define S_U8(n, v)  { n, &Sys_Settings.v, SETTING_U8 }
#define S_U16(n, v) { n, &Sys_Settings.v, SETTING_U16 }
#define S_IN(n, i)  { n, &Sys_Settings.InputCodeSettings[i], SETTING_INPUT }
const Setting configTable[] = {
    S_U16("ResolutionWidth",ScreenWidth),S_U16("ResolutionHeight",ScreenHeight),S_U8("Fullscreen",Fullscreen),S_U8("FOV",FOV),
    S_U8("Brightness",Brightness),S_U8("Gamma",Gamma),S_U8("AA",FXAA),S_U8("Shadows",Shadows),S_U8("SSR",Reflections),
    S_U8("VSync",Vsync),S_U8("ModelDetail",ModelDetail),S_U8("GI",GI),S_U8("SpeakerMode",SpeakerMode),S_U8("Reverb",Reverb),
    S_U8("VolumeMaster",VolumeMaster),S_U8("VolumeMusic",VolumeMusic),S_U8("VolumeMessage",VolumeMessage),S_U8("VolumeEffects",VolumeEffects),
    S_U8("Language",Language),S_U8("DynamicMusic",DynamicMusic),S_U8("Footsteps",Footsteps),S_U8("InvertLook",InvertLook),S_U8("Monitor",CurrentMonitor),
    S_U8("InvertCyberspaceLook",InvertCyberspaceLook),S_U8("InvertInventoryCycling",InvertInventoryCycling),S_U8("QuickItemPickup",QuickItemPickup),
    S_U8("QuickReloadWeapons",QuickReloadWeapons),S_U8("MouseSensitivity",MouseSensitivity),S_U8("NoShootMode",NoShootMode),S_U8("HeadBob",HeadBob),
    S_IN("Forward",0),S_IN("Strafe Left",1),S_IN("Backpedal",2),S_IN("Strafe Right",3),S_IN("Jump",4),S_IN("Crouch",5),S_IN("Prone",6),S_IN("Lean Left",7),
    S_IN("Lean Right",8),S_IN("Sprint",9),S_IN("Turn Left",10),S_IN("Turn Right",11),S_IN("Look Up",12),S_IN("Look Down",13),S_IN("Recent Log",14),
    S_IN("Biomonitor",15),S_IN("Sensaround",16),S_IN("Lantern",17),S_IN("Shield",18),S_IN("Infrared",19),S_IN("Email",20),S_IN("Booster",21),
    S_IN("Jumpjets",22),S_IN("Attack",23),S_IN("Use",24), S_IN("Menu/Back",25),S_IN("Toggle Mode",26),S_IN("Reload",27),
    S_IN("Weapon +",28),S_IN("Weapon -",29),S_IN("Grenade",30),S_IN("Grenade +",31),S_IN("Grenade -",32),S_IN("Ammo Type",33),S_IN("Patch Use",34),
    S_IN("Patch +",35),S_IN("Patch -",36),S_IN("Full Map",37),S_IN("Swim Up",38),S_IN("Swim Down",39),S_IN("Screenshot",40)
};
const int configTableSize = sizeof(configTable) / sizeof(Setting);
static inline __attribute__((always_inline)) i32 GetGLFWIndirectionIndexForAnInput(const char* val) { for (int i=0;i<134;++i) {if (StringsEqual(val,inputElements[i].name)) return i;} return 148; }
char *GetNextStringUpToNewlineOrEOF(char*,int,OsFileHandle),*data_parser_trim(char*); i32 StringToInt(const char*);
void LoadConfig(void) {
    OsFileHandle f = OS_OpenReadonly("./Data/Config.ini");
    char line[512];
    while (GetNextStringUpToNewlineOrEOF(line,sizeof(line),f)) {
        char* s = data_parser_trim(line); if (*s == 0 || (s[0] == '/' && s[1] == '/')) continue;
        char* eq = StringFindFirstCharWithin(s, '='); if (!eq) continue;
        
        *eq = 0; char *key = data_parser_trim(s), *val = data_parser_trim(eq + 1);
        for (int i = 0; i < configTableSize; i++) {
            if (StringsEqual(key,configTable[i].name)) {
                if (configTable[i].type == SETTING_U8)         *( u8*)configTable[i].ptr = (u8)StringToInt(val);
                else if (configTable[i].type == SETTING_U16)   *(u16*)configTable[i].ptr = (u16)StringToInt(val);
                else if (configTable[i].type == SETTING_INPUT) *(u16*)configTable[i].ptr = GetGLFWIndirectionIndexForAnInput(val);
                break;
            }
        }
    }
    Sys_Settings.ScreenWidth = vmax(Sys_Settings.ScreenWidth,320);
    Sys_Settings.ScreenHeight = vmax(Sys_Settings.ScreenHeight,200);
    OS_Close(f);
}

void FilePrintString(OsFileHandle f, const char* fmt, ...);
void SaveConfig(void) {
    OsFileHandle f = OS_OpenWriteonly("./Data/Config.ini");
    for (int i=0;i<configTableSize;++i) {
        if (configTable[i].type == SETTING_U8)         FilePrintString(f,"%s = %u\n",configTable[i].name,*(u8*)configTable[i].ptr);
        else if (configTable[i].type == SETTING_U16)   FilePrintString(f,"%s = %u\n",configTable[i].name,*(u16*)configTable[i].ptr);
        else if (configTable[i].type == SETTING_INPUT) FilePrintString(f,"%s = %s\n",configTable[i].name,inputElements[*(u16*)configTable[i].ptr].name);
    }

    OS_Close(f);
    DualLog("Saved settings to ./Data/Config.ini! framenum %u\n",Sys_Global.globalFrameNum);
}

KeyState* GetCodeMapping(int settingIndex) {
    i32 i = Sys_Settings.InputCodeSettings[settingIndex]; // Get table index into all recognized inputs
    if (i == 148 || i >= MAX_KEYS) return &Sys_Input.keyStates[MAX_KEYS - 1]; // UNUSED NULL (e.g. setting unbound)
    
    if (i >= 53 && i <= 61) { // Pick subtable of GLFW values that were set by GLFW callbacks
        return &Sys_Input.mouseButtons[inputElements[i].value];
    } else if (i >= 62 && i <= 77) {
        return &Sys_Input.joystickButtons[GLFW_JOYSTICK_1][inputElements[i].value];        
    } else if ((i >= 78 && i <= 79) || (i >= 132 && i <= 133)) {
        return &Sys_Input.joystickHats[inputElements[i].value];        
    }
    
    return &Sys_Input.keyStates[inputElements[i].value];
}

bool GetKeyRiseEdgeOrHeld(int settingIndex, bool risingEdge) {
    i32 i = Sys_Settings.InputCodeSettings[settingIndex]; // Get table index into all recognized inputs
         if (i == 128) return Sys_Input.scrollDelta > 0.0; // Mousewheel +
    else if (i == 129) return Sys_Input.scrollDelta < 0.0; // Mousewheel -
    
    KeyState* keyOfConcern = GetCodeMapping(settingIndex);
    bool retval = risingEdge ? keyOfConcern->pressed : keyOfConcern->down;
    return retval;
}

ENGINE_TO_MOD bool GetKey(int settingIndex) { return GetKeyRiseEdgeOrHeld(settingIndex,false); }  // True while held down.
ENGINE_TO_MOD bool GetKeyPressed(int settingIndex) { return (settingIndex < 0) ? Sys_Input.keyStates[GLFW_KEY_GRAVE_ACCENT].pressed : GetKeyRiseEdgeOrHeld(settingIndex,true); } // True 1st frame down.
ENGINE_TO_MOD void IgnoreNextMouseDelta(void) { Sys_Input.ignore_next_mouse_delta = true; }
OsFileHandle levelFileHandle; const char** creditPages = NULL;
ENGINE_TO_MOD void LoadLevel(u8 curlevel) {
    double start_time = get_time();
    DebugRAM("start of LoadLevel");
    Sys_Global.levelCurrentlyLoading = true; Sys_Global.gamePaused = false; Sys_Global.menuActive = false;
    RenderLoadingProgress(100,"Loading level...");
    MemSetToValueForNBytes(lights,0,LIGHT_COUNT * sizeof(Light)); MemSetToValueForNBytes(lanims,0,LIGHT_COUNT * sizeof(LightAnimation));
    MemSetToValueForNBytes(alreadyReadLightOnOnce,0,sizeof(alreadyReadLightOnOnce));
    MemSetToValueForNBytes(modelMatrices,0,INSTANCE_COUNT * 16 * sizeof(float)); // Matrix4x4 = 16
    MemSetToValueForNBytes(camViews,0,64 * sizeof(CamView)); camViewCount = 0;
    MemSetToValueForNBytes(Sys_Global.instances + 3,0,(INSTANCE_COUNT - 3) * sizeof(Entity)); // Initialize instances, the global entity array for the currently loaded level.
    char filename[20]; // Minimum size for 0 through 13.
    StringFormat(filename, sizeof(filename), "./Data/level%d.txt", curlevel);
    levelFileHandle = OS_OpenReadonly(filename);
    LoadLevelMod(curlevel);
    OS_Close(levelFileHandle);
    for (int i=0;i<Sys_Global.loadedLights;++i) {/* lights[i].maxIntensity *= 2.0f; */lightsNewPosition[i]=lights[i].pos; }
    DualLog("Loaded %d entities, %u static lights for Level %d... took %f secs\n",Sys_Global.loadedInstances,Sys_Global.loadedLights,curlevel,get_time() - start_time);
    RenderLoadingProgress(110,"Initialize entities...");
    for (int i=PLAYER1;i<Sys_Global.loadedInstances;++i) {
        Entity* e = &Sys_Global.instances[i];
        i32 cellIdx = PosGetCellCoords(e->position.x,e->position.z);
        e->cellIndex = cellIdx; e->cellX=PosGetCellCoordX(e->position.x); e->cellZ=PosGetCellCoordZ(e->position.z);
        e->radius = modelBounds[e->modelIndex]*vmax(vmax(e->scale.x,e->scale.y),e->scale.z);
        e->shadRadius = e->radius * 1.41;
    }
    
    ModInitAfterLoad(); ResetLevelAudio(); ResetLevelMusic(); creditPages = GetCreditsText();
    DualLog("Entity instances initialized after load\n");
    RenderLoadingProgress(110,"Loading cull system...");
    CullInit(); // Must be after level! MUST BE AFTER SortInstances!!
    glUseProgram(Sys_Render.voxelUpdateShaderProgram);
    glUniform1f(0,Sys_Global.voxelMinCenterX);
    glUniform1f(1,Sys_Global.voxelMinCenterZ);
    glUniform1ui(2,Sys_Global.loadedLights);
    glUniform1f(3,Sys_Global.worldMin_x);
    glUniform1f(4,Sys_Global.worldMin_z);
    glUniform1f(7,Sys_Global.farPlane * Sys_Global.farPlane);
    RenderLoadingProgress(120,"Loading voxel lighting data...");
    for (u16 i = START_INDEX_LEVEL_INSTANCES; i < Sys_Global.loadedInstances; i++) Sys_Global.dirtyInstances[i] = true;
    for (u16 i = 0; i < Sys_Global.loadedLights; i++) { lightsNewPosition[i] = lights[i].pos; }
    MemSetToValueForNBytes(voxen_Shadow_System.shadowmapIndirectionList,MAX_SHADOWMAPS + 1,Sys_Global.loadedLights * sizeof(u32)); // Set to invalid values for all
    Sys_Global.levelCurrentlyLoading = false;
    DebugRAM("end of LoadLevel");
}

void InputClearRisingAndFallingEdges(void) { // Clear keypress rising and falling edge triggers
    for (i32 i=0;i<MAX_KEYS;++i)          Sys_Input.keyStates[i].pressed = Sys_Input.keyStates[i].released = false;       // Can't memset as we want to preserve down state
    for (i32 i=0;i<MAX_MOUSE_BUTTONS;i++) Sys_Input.mouseButtons[i].pressed = Sys_Input.mouseButtons[i].released = false; // Can't memset as we want to preserve down state
    Sys_Input.scrollDelta = 0;
}

__attribute__((cold)) void NewGame(void) { // Reset World States
    DualLog("Loading new game...\n");
    RenderLoadingProgress(100,"Loading new game...");
    Sys_Global.menuActive = Sys_Global.gamePaused = enteringPlayerName = fovSliderActive = gammaSliderActive = masterVolumeSliderActive = musicVolumeSliderActive = messageVolumeSliderActive = sfxVolumeSliderActive = returnToPause = false;
    currentMenuItem = currentMenuTab = 0; currentMenuPage = Mpg_FrontPage;
    Sys_Global.pauseRelativeTime = Sys_Global.last_physics_time = 0.0;
    Sys_Global.inventoryMode = false;
    MemSetToValueForNBytes(Sys_Global.instances,0,2 * sizeof(Entity)); // Blank out player entities
    PlayerInit(PLAYER1); PlayerInit(PLAYER2);
    Sys_Global.instances[WORLD].ioflags = 0u;
    cam_yaw = 90.0f; cam_pitch = 0.0f; cam_roll = 0.0f;
    Sys_Global.inventoryMode = Sys_Settings.NoShootMode;
    Sys_Global.pauseRelativeTime =  Sys_Global.last_physics_time = 0.0;
    Sys_Global.last_topframe_time = Sys_Global.last_physics_time - 0.05;
    Sys_Global.timeSinceLastPhysicsTick = 0.0166666666f;
    Sys_Global.gameFinished = Sys_Global.creditsActive = Sys_Global.decoyActive = false;
	Sys_Global.ressurections = Sys_Global.deaths = Sys_Global.kills = Sys_Global.cyberkills = 0u;
	Sys_Global.shotsFired = Sys_Global.grenadesThrown = Sys_Global.savesScummed = 0U;
    Sys_Global.damageDealt = Sys_Global.damageReceived = 0.0f;
	Sys_Global.creditsPageIndex = 0u;
    for (int i=0;i<14;++i) Sys_Global.levelSecurity[i] = 100u;
    InputClearRisingAndFallingEdges();
    Sys_Input.currentMouse_dx = Sys_Input.currentMouse_dy = 0;
    Sys_Input.last_mouse_x = Sys_Input.last_mouse_y = 0;
    Sys_Input.ignore_next_mouse_delta = true;
    Sys_Input.isCapsLockOn = false; // As far as we're concerned, don't worry about OS state.
    Sys_Input.lastUse = false;
    LoadLevel(Sys_Global.startLevel); // Must be after entities!
    ModNewGame();
}

static void GoIntoGame(void) { NewGame(); PlayGameMusic(); DualLog("Player named \"%s\" started the game!\n", Sys_Global.playerName); }
static __attribute__((noinline)) void GenerateAndBindTexture(u32 *id, i32 internalFormat, i32 width, i32 height, u32 format, u32 type, i32 filt, unsigned char* bmp) {
    if (*id == 0) {glGenTextures(1,id);} glBindTexture(GL_TEXTURE_2D,*id);
    glTexImage2D(GL_TEXTURE_2D,0,internalFormat,width,height,0,format,type,bmp);
    glTexParameteri(GL_TEXTURE_2D,0x2801/*GL_TEXTURE_MIN_FILTER*/,filt); glTexParameteri(GL_TEXTURE_2D,0x2800/*GL_TEXTURE_MAG_FILTER*/,filt);
}
static void GenBTexture(u32 *id, i32 internalFormat, i32 width, i32 height, u32 format, u32 type, i32 filt) { GenerateAndBindTexture(id,internalFormat,width,height,format,type,filt,NULL); }
static void UpdateScreenSize(i32 width, i32 height) {
    u16 w = Sys_Settings.ScreenWidth = vmax(vmin((u16)width,7680u),320u), h = Sys_Settings.ScreenHeight = vmax(vmin((u16)height,4320u),200u); // Cap at minimum Quake resolution and maximum 8k.
    float wf = (float)w, hf = (float)h; Sys_Settings.ScreenCenterX = wf * 0.5f; Sys_Settings.ScreenCenterY = hf * 0.5f;
    glViewport(0,0,w,h); RenderSystem* rs = &Sys_Render;
    glUseProgram(rs->imageBlitShaderProgram); glUniform1ui(2,w); glUniform1ui(3,h); glUniform1i(26,Sys_Settings.SSR_RES);
    glUseProgram(rs->chunkShaderProgram); glUniform1ui(6,w); glUniform1ui(7,h);
    glUseProgram(rs->ssrShaderProgram); glUniform1ui(0,w / Sys_Settings.SSR_RES); glUniform1ui(1,h / Sys_Settings.SSR_RES); glUniform1i(2,Sys_Settings.SSR_RES);
    GenBTexture(&rs->inputImageID,     GL_RGBA8,w,h,GL_RGBA,GL_UNSIGNED_BYTE,0x2600/*GL_NEAREST*/); // Lit Raster
    GenBTexture(&rs->inputWorldPosID,GL_RGBA32F,w,h,GL_RGBA,        GL_FLOAT,0x2600/*GL_NEAREST*/); // Raster World Positions
    GenBTexture(&rs->inputSpecID,      GL_RGBA8,w,h,GL_RGBA,GL_UNSIGNED_BYTE,0x2600/*GL_NEAREST*/); // Specular Colors
    GenBTexture(&rs->inputNormalID,    GL_RG16F,w,h, GL_RGB,        GL_FLOAT,0x2600/*GL_NEAREST*/); // Normal XYZ
    GenBTexture(&rs->inputDepthID,0x81A7/*GL_DEPTH_COMPONENT32*/,w,h,0x1902/*GL_DEPTH_COMPONENT*/,GL_FLOAT,0x2600/*GL_NEAREST*/); // Raster Depth
    GenBTexture(&rs->outputImageID,GL_RGBA8,w / Sys_Settings.SSR_RES,h / Sys_Settings.SSR_RES,GL_RGBA,GL_UNSIGNED_BYTE,0x2601/*GL_LINEAR*/);
    glBindFramebuffer(GL_FRAMEBUFFER,rs->gBufferFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,rs->inputImageID,0);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT1,GL_TEXTURE_2D,rs->inputWorldPosID,0);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT2,GL_TEXTURE_2D,rs->inputSpecID,0);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT3,GL_TEXTURE_2D,rs->inputNormalID,0);
    glFramebufferTexture2D(GL_FRAMEBUFFER,0x8D00/*GL_DEPTH_ATTACHMENT*/,GL_TEXTURE_2D,rs->inputDepthID,0);
    glBindImageTexture(0,rs->inputImageID,0,GL_FALSE,0,GL_READ_WRITE,GL_RGBA8);      // Main Rendered Color
    glBindImageTexture(1,rs->inputWorldPosID,0,GL_FALSE,0,GL_READ_WRITE,GL_RGBA32F); // World Position XYZ
    glBindImageTexture(2,rs->inputSpecID,0,GL_FALSE,0,GL_READ_WRITE,GL_RGBA8);       // Specular
    glBindImageTexture(4,rs->outputImageID,0,GL_FALSE,0,GL_READ_WRITE,GL_RGBA8);     // SSR result
    glBindImageTexture(5,rs->inputNormalID,0,GL_FALSE,0,GL_READ_WRITE,GL_RG16F);     // Normal XYZ
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D,rs->outputImageID);
    glBindFramebuffer(GL_FRAMEBUFFER,0);
}

ENGINE_TO_MOD void AddCamView(Vector3 pos, Quaternion rot, u8 fov, u16 width, u16 height, float near, float far) {    
    camViews[camViewCount] = (CamView){pos,rot,fov,width,height,near,far,Sys_Global.pauseRelativeTime + (camViewCount * 0.05f) + 0.5f,false}; // Staggered starts so not all at once for performance.
    GenBTexture(&camViewTextures[camViewCount],GL_RGBA8,width,height,GL_RGBA,GL_UNSIGNED_BYTE,0x2600/*GL_NEAREST*/); camViewCount++;
}

static i32 initJoysticks(void) { if (!_glfw.joysticksInitialized && !PLATFORM_initJoysticks()) {return 0;} return _glfw.joysticksInitialized =  1; }
bool JoystickPresent(int jid) {
    if (jid < 0 || jid > GLFW_JOYSTICK_LAST) return false;
    if (!initJoysticks()) return false;
    _GLFWjoystick* js = _glfw.joysticks + jid;
    return js->connected ? PLATFORM_pollJoystick(js) : false;
}

static inline __attribute__((always_inline)) __attribute__((hot)) void Input_Poll(void) {
    PLATFORM_pollEvents();
    for (int jid = GLFW_JOYSTICK_1; jid <= GLFW_JOYSTICK_LAST; ++jid) {
        if (!JoystickPresent(jid)) continue;

        _GLFWjoystick* js = _glfw.joysticks + jid;
        if (!js->connected) continue;

        PLATFORM_pollJoystick(js);
        int totalButtons = js->buttonCount + js->hatCount * 4;
        for (int i = 0; i < totalButtons && i < 16; ++i) {
            KeyState* k = &Sys_Input.joystickButtons[jid - GLFW_JOYSTICK_1][i];
            bool down = js->buttons[i] == GLFW_PRESS;
            k->pressed  = down && !k->down;
            k->released = !down && k->down;
            k->down     = down;
        }

        for (int i = 0; i < js->hatCount && i < 5; ++i) { Sys_Input.joystickHats[i].down = js->hats[i]; }
//         for (int i = 0; i < js->axisCount && i < MAX_JOYSTICK_AXES; ++i) { Sys_Input.joystickAxes[jid - GLFW_JOYSTICK_1][i] = js->axes[i]; } TODO??
    }
}

void CenterWindowOnMonitor(void) {
    int monitorCount; GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
    if (Sys_Settings.CurrentMonitor > (monitorCount - 1)) { Sys_Settings.CurrentMonitor = 0; SaveConfig(); }
    int mx, my; GLFWmonitor* next = monitors[Sys_Settings.CurrentMonitor];
    glfwGetMonitorPos(next,&mx,&my);
    const GLFWvidmode* mode = glfwGetVideoMode(next);
    int xpos = mx + (mode->width - Sys_Settings.ScreenWidth) / 2;
    int ypos = my + (mode->height - Sys_Settings.ScreenHeight) / 2;
    glfwSetWindowPos(window,xpos,ypos);
    Sys_Input.ignore_next_mouse_delta = true;
}

double monitorSwitchTime;
void CycleToNextMonitor(void) {
    if (get_time() < monitorSwitchTime) return;
    
    monitorSwitchTime = get_time() + 0.5; // Prevent toggling rapidly on accident
    int monitorCount; GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
    if (Sys_Settings.CurrentMonitor > (monitorCount - 1)) { Sys_Settings.CurrentMonitor = 0; SaveConfig(); }
    if (!monitors || monitorCount < 2) return;

    Sys_Settings.CurrentMonitor = (Sys_Settings.CurrentMonitor + 1) % monitorCount;
    SaveConfig();
    CenterWindowOnMonitor();
}

GLFWmonitor* GetCurrentMonitor(void) {
    int wx=0,wy=0,ww=0,wh=0; PLATFORM_getWindowPos(((_GLFWwindow*)window),&wx,&wy); PLATFORM_getWindowSize(((_GLFWwindow*)window),&ww,&wh);
    GLFWmonitor* bestMonitor = glfwGetPrimaryMonitor();
    int bestArea=0,monitorCount; GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
    for (int i=0;i<monitorCount;++i) {
        int mx, my;
        glfwGetMonitorPos(monitors[i], &mx, &my);
        const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
        int mw = mode->width;
        int mh = mode->height;
        int left=vmax(wx,mx), right=vmin(wx + ww,mx + mw), top=vmax(wy,my), bottom=vmin(wy + wh,my + mh);
        int area = (right > left && bottom > top) ? (right - left) * (bottom - top) : 0;
        if (area > bestArea) { bestArea = area; bestMonitor = monitors[i]; }
    }
    return bestMonitor;
}

void GatherResolutionModes(void) {
    resDropdownCount = 0;
    GLFWmonitor* monitor = GetCurrentMonitor(); if (!monitor) monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* desktop = glfwGetVideoMode(monitor); if (!desktop) return;

    static const struct {int w,h;} commonRes[] = {{320,200}, {640,400}, {640,480}, {800,600}, {1024,768}, {1280,720}, {1280,800}, {1366,768}, {1440,900}, {1600,900}, {1920,1080}, {2560,1440}};
    int maxW = desktop->width, maxH = desktop->height,j;
    for (int i = 0; i < (int)(sizeof(commonRes)/sizeof(commonRes[0])) && resDropdownCount < 8; ++i) {
        int w = commonRes[i].w, h = commonRes[i].h;
        if (w > maxW || h > maxH || w < 320 || h < 200) continue;

        for (j = 0; j < resDropdownCount; ++j) {
            if (resModes[j].w == w && resModes[j].h == h) break;
        }
        if (j == resDropdownCount) resModes[resDropdownCount++] = (ResMode){w,h};
    }

    if (resDropdownCount < 8) resModes[resDropdownCount++] = (ResMode){desktop->width,desktop->height};
    resSelectedIdx = 0;
    for (int i = 0; i < resDropdownCount; ++i) {
        if (resModes[i].w == (int)Sys_Settings.ScreenWidth && resModes[i].h == (int)Sys_Settings.ScreenHeight) { resSelectedIdx = i; break; }
    }
}

void SetSkyRotateSpeed(void) { static const float skyRotateSpeeds[] = { 0.05f, 1.0f, 2.5f, 3.75f, 6.25f }; glUseProgram(Sys_Render.imageBlitShaderProgram); glUniform1f(30,skyRotateSpeeds[Sys_Cheats.dizzyLevel]); }
void ChangeResolution(void) {
    if (resDropdownCount < 1) return;

    resSelectedIdx = (resSelectedIdx + 1) % resDropdownCount;
    Sys_Settings.ScreenWidth  = (u32)resModes[resSelectedIdx].w;
    Sys_Settings.ScreenHeight = (u32)resModes[resSelectedIdx].h;
    GLFWmonitor* monitor = GetCurrentMonitor();
    if (!monitor) monitor = glfwGetPrimaryMonitor();

    int mx, my;
    glfwGetMonitorPos(monitor, &mx, &my);
    const GLFWvidmode* desktop = glfwGetVideoMode(monitor);
    int xpos = mx + (desktop->width  - (int)Sys_Settings.ScreenWidth)  / 2;
    int ypos = my + (desktop->height - (int)Sys_Settings.ScreenHeight) / 2;
    glfwSetWindowSize(window, (int)Sys_Settings.ScreenWidth, (int)Sys_Settings.ScreenHeight);
    glfwSetWindowPos(window,xpos,ypos);
    UpdateScreenSize((int)Sys_Settings.ScreenWidth, (int)Sys_Settings.ScreenHeight);
    Sys_Input.ignore_next_mouse_delta = true;
    resDropdownOpen = false;
    SaveConfig();
}

void ChangeFullScreenWindowed(void) {
    int monitorCount; GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
    GLFWmonitor* monitor = monitors[Sys_Settings.CurrentMonitor];
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    if (Sys_Settings.Fullscreen) {
        ((_GLFWwindow*)window)->decorated = 0; PLATFORM_setWindowDecorated(((_GLFWwindow*)window),0);
        int x,y,w,h;
        glfwGetMonitorWorkarea(monitor,&x,&y,&w,&h);
        glfwSetWindowMonitor(window,x,y,w,h);
        Sys_Settings.ScreenWidth = w;
        Sys_Settings.ScreenHeight = h;
    } else {
        ((_GLFWwindow*)window)->decorated = 1; PLATFORM_setWindowDecorated(((_GLFWwindow*)window),1);
        int mx, my; 
        glfwGetMonitorPos(monitor,&mx,&my);
        int x,y,w,h;
        glfwGetMonitorWorkarea(monitor,&x,&y,&w,&h);
        Sys_Settings.ScreenWidth  = vmax(vmin((w*3)/4,1366),320);
        Sys_Settings.ScreenHeight = vmax(vmin((h*3)/4, 768),200);
        int xpos = mx + (mode->width - Sys_Settings.ScreenWidth) / 2;
        int ypos = my + (mode->height - Sys_Settings.ScreenHeight) / 2;
        glfwSetWindowMonitor(window,xpos,ypos,Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight);
    }
    
    UpdateScreenSize(Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight);
    Sys_Input.ignore_next_mouse_delta = true;
}

void SetVSync(void) { _GLFWwindow* handle = (_GLFWwindow*)window; handle->context.swapInterval((i32)Sys_Settings.Vsync); }
void SetGI(void) { }// TODO: Set needed Voxel GI uniforms from Sys_Settings.GI
void LoadTextForLanguage(u8),LoadLogTextForLanguage(u8); bool GetKey(int settingIndex),GetKeyPressed(int settingIndex); void* mod_handle = NULL;
void SetLanguage(void) { LoadTextForLanguage(Sys_Settings.Language); LoadLogTextForLanguage(Sys_Settings.Language); }
void ApplySettings(void) { ChangeFullScreenWindowed(); SetSkyRotateSpeed(); SetVSync(); SetGI(); SetLanguage(); }
void StringConcatenate(char* a, const char* b, size_t bufferSize);
void OpenMainMenu(void) { PlayMenuMusic(); Sys_Global.menuActive = true; currentMenuPage = Mpg_FrontPage; }
bool MenuEnter(void) { return (Sys_Input.keyStates[GLFW_KEY_KP_ENTER].pressed || Sys_Input.keyStates[GLFW_KEY_ENTER].pressed); }
u8 UI_Interactable(i16 x, i16 y, float w, float h, bool* cursorOver, i8 this, bool sustained) {
    bool cursorIsOver = CursorIsOverBounds(x,x + w,y + h,y);
    if (cursorIsOver && mouseMovementThisFrame) { currentMenuItem = this; if (cursorOver != NULL) {*cursorOver = cursorIsOver;} }
    if ((sustained ? Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_LEFT ].down : Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_LEFT ].pressed) && cursorIsOver) return 1u;
    if ((sustained ? Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_RIGHT].down : Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_RIGHT].pressed) && cursorIsOver) return 2u;
    return 0u;
}

u8 UI_Button(i16 x, i16 y, float w, float h, bool* cursorOver, i8 this) { return UI_Interactable(x,y,w,h,cursorOver,this,false); }
bool AnyLeftRightMouseDown(void) { return (Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_LEFT].down || Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_RIGHT].down); }
bool UI_Slider(i16 x, i16 y, i16 w, i16 h, i16 sliderPos, i16 xPosForLabel, u8 currentValue, u8* out, bool* sliderActive, u8 min, u8 max, u8 step, u8 mindex, u16 lingdex) {
    bool over=false,changed=false; *out = currentValue;
    RenderUIImage(x,y, w,h, 1079); // Slider background
    RenderUIImage(x + sliderPos,y, h,h,1078); // Slider handle
    if (UI_Interactable(xPosForLabel,y + h,xPosForLabel + w,h,&over,mindex,true)) *sliderActive = true;
    if (*sliderActive && Sys_Input.currentMouse_dx != 0) {
        i32 new = (i32)currentValue + vmin(vmax(Sys_Input.currentMouse_dx,-1),1); *out = (u8)vmin(vmax(new,min),max); if (*out != currentValue) {changed = true;}
    }
    
    if (!AnyLeftRightMouseDown()) { if (*sliderActive) { *sliderActive = false; SaveConfig(); } }
    if (MenuEnter() && currentMenuItem == mindex) {
        bool shiftHeld = Sys_Input.keyStates[GLFW_KEY_LEFT_SHIFT].down || Sys_Input.keyStates[GLFW_KEY_RIGHT_SHIFT].down;
        if (shiftHeld) *out = *out <=  ((min + step) - 1) ? max : *out - step;
        else           *out = *out >= ((max - step) + 1) ?  min : *out + step;
        changed = true;
    }
    
    over = over || currentMenuItem == mindex;
    RenderFormattedText(xPosForLabel,y,over ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,"%s %u",Sys_Text.stringTable[lingdex],*out);
    return changed;
}

u8 UI_MenuButton(i16 bX, i16 bY, u8 menuItem, i16 bW, i16 bH,  i16 tX, i16 tY, const char* text, i16 pX, i16 pY) {
    bool over = false; u8 retvalue = 0u;
    retvalue = UI_Button(bX,bY, bW,bH, &over, menuItem);
    if (!retvalue) retvalue = (MenuEnter() && currentMenuItem == menuItem);
    over = over || currentMenuItem == menuItem;
    RenderFormattedText(tX,tY, over ? TEXT_STOPD_RED : TEXT_RED_MENU,FONT_STOPD,1.5f,text);
    RenderUIImage(pX,pY, 40,40, over ? 1029 : 1028); // Menu pad
    return retvalue;
}

bool UI_Checkbox(i16 x, i16 y, i8 mitem, u16 textIdx, bool currentlyOn) {
    RenderUIImage(x,y,16,16,910); // Checkbox background
    bool over = false;
    bool changed = (UI_Button(x,y + 16,210,16,&over,mitem) || (MenuEnter() && currentMenuItem == mitem));
    over = over || currentMenuItem == mitem;
    if (currentlyOn) RenderUIImage(x + 2,y + 2, 12,12, 912); // Checkbox check
    RenderFormattedText(x + 20,y,over ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,Sys_Text.stringTable[textIdx]);
    return changed;
}

void UI_HeaderText(i16 x, const char* text) {
    RenderFormattedText(x,50,TEXT_GREEN_MENU_SHADOW,FONT_STOPD,1.75f,text);
    RenderFormattedText(x,46,TEXT_GREEN_MENU_GLOW,FONT_STOPD,1.75f,text);
    RenderFormattedText(x,48,TEXT_GREEN_MENU,FONT_STOPD,1.75f,text);
}

ENGINE_TO_MOD void MenuGoBack(void) {
    if (returnToPause) { returnToPause = false; Sys_Global.gamePaused = true; Sys_Global.menuActive = false; PlayGameMusic(); }
    if (currentMenuPage == Mpg_Singleplayer || currentMenuPage == Mpg_Multiplayer || currentMenuPage == Mpg_Options) currentMenuPage = Mpg_FrontPage;//News
    else if (currentMenuPage == Mpg_Load || currentMenuPage == Mpg_NewGame || currentMenuPage == Mpg_IntroVideo || currentMenuPage == Mpg_CreditsVideo) currentMenuPage = Mpg_Singleplayer;
}

void ChangeMenuPage(u8 pg) { currentMenuPage = pg; currentMenuItem = currentMenuTab = 0; }
void RenderMenu(void) {    
    if (currentMenuPage != Mpg_IntroVideo && currentMenuPage != Mpg_CreditsVideo && currentMenuPage != Mpg_Options) RenderUIImage(-417,-384, 2200,1536, 1026); // Menu background
    if (currentMenuPage == Mpg_IntroVideo || currentMenuPage == Mpg_CreditsVideo) RenderUIImage(-417,-384, 2200,1536, 0); // Video blackground
    if (currentMenuPage == Mpg_Options) RenderUIImage(-417,-384, 2200,1536, 1032); // Menu background
    if (currentMenuPage == Mpg_FrontPage) {
        menuItemCount = 4; menuTabCount = 1;
        RenderUIImage(282,46, 800,128, 1031); // Title CITADEL with strikethrough effect
        if (UI_MenuButton(408,340,0,574,84, 304,188,/*"SINGLEPLAYER"*/Sys_Text.stringTable[719],413,276)) ChangeMenuPage(Mpg_Singleplayer);
        if (UI_MenuButton(408,458,1,574,84, 304,268,/*"MULTIPLAYER"*/Sys_Text.stringTable[720], 413,396)) ChangeMenuPage(Mpg_Multiplayer);
        if (UI_MenuButton(408,582,2,574,84, 304,350,/*"OPTIONS"*/Sys_Text.stringTable[721],     413,520)) ChangeMenuPage(Mpg_Options);
        if (UI_MenuButton(408,702,3,574,84, 304,430,/*"QUIT"*/Sys_Text.stringTable[722],        413,638)) OS_Exit(0);
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
        RenderFormattedText(1076,732,overBack ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_RED_MENU,FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == Mpg_Multiplayer) {
        menuItemCount = 1; menuTabCount = 1;
        UI_HeaderText(266,/*"MULTIPLAYER"*/Sys_Text.stringTable[720]);
        RenderUIImage(1060,724, 84,36, 1252); // Back Button background
        bool overBack = false;
        if (UI_Button(1060,758, 84,32, &overBack, 0) || (MenuEnter() && currentMenuItem == 0)) MenuGoBack();
        overBack = overBack || currentMenuItem == 0;
        RenderFormattedText(1076,732,overBack ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_RED_MENU,FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == Mpg_Options) {
        menuTabCount = 3;
        UI_HeaderText(238,/*"CONFIGURATION"*/Sys_Text.stringTable[745]);
        if (currentMenuTab != 0) RenderUIImage(179,220, 1001,548, 1030); // Config background
        if (currentMenuTab == 0) RenderUIImage(179,220, 1001,548, 1033); // Config background graphics (empty alpha center)
        RenderUIImage(520,196, 160,30, currentMenuTab == 2 ? 920 : 921); // Config tab unhighlighted
        if (UI_Button(520,196+30, 160,30, NULL, 2)) currentMenuTab = 2;
        RenderFormattedText(530,202,currentMenuTab == 2 ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,/*"AUDIO / LANG"*/Sys_Text.stringTable[793]);
        RenderUIImage(354,196, 160,30, currentMenuTab == 1 ? 920 : 921); // Config tab unhighlighted
        if (UI_Button(354,196+30, 160,30, NULL, 1)) currentMenuTab = 1;
        RenderFormattedText(366,202,currentMenuTab == 1 ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,/*"INPUT"*/Sys_Text.stringTable[792]);
        RenderUIImage(190,196, 160,30, currentMenuTab == 0 ? 920 : 921); // Config tab highlighted
        if (UI_Button(190,196+30, 160,30, NULL, 0)) currentMenuTab = 0;
        RenderFormattedText(200,202,currentMenuTab == 0 ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL,1.0f,/*"GRAPHICS"*/Sys_Text.stringTable[791]);
        if (currentMenuTab == 0) {
            bool overRes = false, overFull = false, overChgM = false;
            menuItemCount = 11; // Graphics
            if (UI_Checkbox(200,500,0,Sys_Settings.ModelDetail ? /*High*/915 : /*No Detail Level Models*/914,Sys_Settings.ModelDetail)) { Sys_Settings.ModelDetail = Sys_Settings.ModelDetail ? 0u : 1u; SaveConfig(); }
            if (UI_Checkbox(200,530,1,/*"FXAA"*/780,Sys_Settings.FXAA)) { Sys_Settings.FXAA = Sys_Settings.FXAA ? 0u : 1u; SaveConfig(); }
            if (UI_Checkbox(200,560,2,Sys_Settings.Shadows ? /*Soft*/787 : /*No Shadows*/785,Sys_Settings.Shadows)) { Sys_Settings.Shadows = Sys_Settings.Shadows ? 0u : 1u; SaveConfig(); }
            if (UI_Checkbox(200,590,3,/*SSR*/788,Sys_Settings.Reflections)) { Sys_Settings.Reflections = Sys_Settings.Reflections ? 0u : 1u; SaveConfig(); }
            if (UI_Checkbox(200,620,4,/*VSYNC*/1026,Sys_Settings.Vsync)) { Sys_Settings.Vsync = Sys_Settings.Vsync ? 0u : 1u; SetVSync(); SaveConfig(); }
            RenderFormattedText(310,620,TEXT_GREEN,FONT_NORMAL,1.0f,"(FPS: %d)", Sys_Global.framesPerLastSecond); // Helper to see vsync take effect.
            u8 newVal;
            if (UI_Slider(400,650,128,16,(((Sys_Settings.FOV - 45.0f) / 105.0f) * (128 - 16)),200,Sys_Settings.FOV,&newVal,&fovSliderActive,45,150,5,5,/*Field of View*/775)) { Sys_Settings.FOV = newVal; if (!AnyLeftRightMouseDown()) {SaveConfig();} }
            if (UI_Slider(400,680,128,16,((Sys_Settings.Brightness / 100.0f) * (128 - 16)),200,Sys_Settings.Brightness,&newVal,&gammaSliderActive,0,100,2,6,/*Gamma*/774)) { Sys_Settings.Brightness = newVal; if (!AnyLeftRightMouseDown()) {SaveConfig();} }
            
            // Resolution
            {
                // Header hit area - UI_Button subtracts h from y internally, so pass y+h as y
                if (UI_Button(190,726,328,16,&overRes,7) || (MenuEnter() && currentMenuItem == 7)) { resDropdownOpen = !resDropdownOpen; currentMenuItem = 7; }
                overRes = overRes || currentMenuItem == 7;
                char resBuf[32];
                if (resDropdownCount > 0) StringFormat(resBuf, sizeof(resBuf), "%ux%u",(u32)resModes[resSelectedIdx].w,(u32)resModes[resSelectedIdx].h);
                else StringFormat(resBuf, sizeof(resBuf), "%ux%u",Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight);

                RenderUIImage(476, 710, 16, 16, overRes ? 1119 : 1077);
                RenderFormattedText(200, 710, overRes ? TEXT_YELLOW : TEXT_GREEN,FONT_NORMAL, 1.0f, "RESOLUTION %s", resBuf);
                if (resDropdownOpen) { // TODO
                    for (int i = 0; i < resDropdownCount; ++i) {
                        i16 itemBaseY = (i16)(710 - (resDropdownCount - i) * 24);
                        bool overItem = false;
                        bool clicked = (UI_Button(190, (i16)(itemBaseY + 24),328,24,&overItem,(i8)(10 + i)) != 0);
                        bool isEnterSelected = (MenuEnter() && currentMenuItem == (i8)(10 + i));
                        bool isSelected = (i == resSelectedIdx);
                        u8 color = isSelected ? TEXT_YELLOW : (overItem ? TEXT_GREEN : TEXT_WHITE);
                        RenderUIImage(190, (i16)(itemBaseY + 24),328,24,1120);
                        char itemBuf[32];
                        StringFormat(itemBuf,sizeof(itemBuf),"%dx%d",(u32)resModes[i].w,(u32)resModes[i].h);
                        RenderFormattedText(200,(i16)(itemBaseY + 4),color,FONT_NORMAL,1.0f,"%s",itemBuf);
                        if (clicked || isEnterSelected) {
                            resSelectedIdx = i;
                            Sys_Settings.ScreenWidth  = (u32)resModes[i].w; Sys_Settings.ScreenHeight = (u32)resModes[i].h;
                            GLFWmonitor* monitor = GetCurrentMonitor();
                            if (!monitor) monitor = glfwGetPrimaryMonitor();
                            int mx,my; glfwGetMonitorPos(monitor,&mx,&my);
                            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
                            int xpos = mx + (mode->width  - (int)Sys_Settings.ScreenWidth)  / 2;
                            int ypos = my + (mode->height - (int)Sys_Settings.ScreenHeight) / 2;
                            glfwSetWindowSize(window, (int)Sys_Settings.ScreenWidth,(int)Sys_Settings.ScreenHeight);
                            glfwSetWindowPos(window,xpos,ypos);
                            UpdateScreenSize((int)Sys_Settings.ScreenWidth,(int)Sys_Settings.ScreenHeight);
                            Sys_Input.ignore_next_mouse_delta = true;
                            resDropdownOpen = false;
                            SaveConfig();
                            GatherResolutionModes();
                            break;
                        }
                    }

                    if (Sys_Input.keyStates[GLFW_KEY_UP].pressed && currentMenuItem >= 10 && currentMenuItem > 10) currentMenuItem--;
                    else if (Sys_Input.keyStates[GLFW_KEY_UP].pressed && currentMenuItem == 10) currentMenuItem = (i8)(10 + resDropdownCount - 1);
                    
                    if (Sys_Input.keyStates[GLFW_KEY_DOWN].pressed && currentMenuItem >= 10 && currentMenuItem < (i8)(10 + resDropdownCount - 1)) currentMenuItem++;
                    else if (Sys_Input.keyStates[GLFW_KEY_DOWN].pressed && currentMenuItem == (i8)(10 + resDropdownCount - 1)) currentMenuItem = 10;

                    if (Sys_Input.keyStates[GLFW_KEY_ESCAPE].pressed) resDropdownOpen = false;
                    if (Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_LEFT].pressed && currentMenuItem < 10 && currentMenuItem != 7) resDropdownOpen = false;
                }
            }
    
            // Fullscreen checkbox
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
            menuItemCount = 10; // Audio / Lang
            u8 newVal;
            if (UI_Slider(426,240,128,16,((Sys_Settings.VolumeMaster / 100.0f) * (128 - 16)),200,Sys_Settings.VolumeMaster,&newVal,&masterVolumeSliderActive,0,100,5,0,/*Master Volume*/802)) { Sys_Settings.VolumeMaster = newVal; if (!AnyLeftRightMouseDown()) {SaveConfig();} }
            if (UI_Slider(426,270,128,16,((Sys_Settings.VolumeMusic / 100.0f) * (128 - 16)),200,Sys_Settings.VolumeMusic,&newVal,&musicVolumeSliderActive,0,100,5,1,/*Music Volume*/803)) { Sys_Settings.VolumeMusic = newVal; if (!AnyLeftRightMouseDown()) {SaveConfig();} }
        }
        
        RenderUIImage(1087,723, 84,36, 1252); // Back Button background
        i8 lastItem = menuItemCount - 1;
        bool overBack = false;
        if (UI_Button(1087,757, 84,32, &overBack, lastItem) || (MenuEnter() && currentMenuItem == lastItem)) MenuGoBack();
        overBack = overBack || currentMenuItem == lastItem;
        RenderFormattedText(1103,731,overBack ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_RED_MENU,FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == Mpg_Load || currentMenuPage == Mpg_Save) {
        menuItemCount = 9; menuTabCount = 1;
        bool isSave = currentMenuPage == Mpg_Save;
        UI_HeaderText(isSave ? 284 : 340, isSave ? /*"SAVE GAME"*/Sys_Text.stringTable[769] : /*"LOAD"*/Sys_Text.stringTable[726]);
        RenderUIImage(400,214, 586,500, 1037); // Load/Save table background
        RenderUIImage(1060,724, 84,36, 1252); // Back Button background
        bool overBack = false;
        if (UI_Button(1060,758, 84,32, &overBack, 0) || (MenuEnter() && currentMenuItem == 0)) MenuGoBack();
        overBack = overBack || currentMenuItem == 0;
        RenderFormattedText(1076,732, overBack ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_RED_MENU, FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == Mpg_NewGame) {
        menuItemCount = 7; menuTabCount = (currentMenuItem > 0 && currentMenuItem <= 16) ? 2 : 1;
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
    } else if (currentMenuPage == Mpg_IntroVideo) {
        menuItemCount = menuTabCount = 1;
        if (MenuEnter()) MenuGoBack();
    } else if (currentMenuPage == Mpg_CreditsVideo) {
        menuItemCount = menuTabCount = 1;
        if (MenuEnter()) MenuGoBack();
    }
    
    if (menuTabCount <= currentMenuTab) currentMenuTab = 0;
    if (menuItemCount <= currentMenuItem) currentMenuItem = 0;
    static const i8 ngSwap[7] = {0,3,4,1,2,6,5};
    if (Sys_Input.keyStates[GLFW_KEY_RIGHT].pressed || Sys_Input.keyStates[GLFW_KEY_LEFT].pressed) {
        int dir = Sys_Input.keyStates[GLFW_KEY_RIGHT].pressed ? 1 : -1;
        currentMenuTab = (currentMenuTab + menuTabCount + dir) % menuTabCount;
        if (currentMenuPage == Mpg_NewGame && currentMenuItem < 7) currentMenuItem = ngSwap[currentMenuItem];
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
    if (UI_Button(522,390, 322,52, &overLoad, 1) || (MenuEnter() && currentMenuItem == 1)) { currentMenuPage = Mpg_Load; PlayMenuMusic(); Sys_Global.menuActive = true; returnToPause = true; }
    overLoad = overLoad || currentMenuItem == 1;
    RenderFormattedText(630,364, overLoad ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.0f,/*"LOAD"*/Sys_Text.stringTable[726]);
    if (UI_Button(522,450, 322,60, &overSave, 2) || (MenuEnter() && currentMenuItem == 2)) { currentMenuPage = Mpg_Save; PlayMenuMusic(); Sys_Global.menuActive = true; returnToPause = true; }
    overSave = overSave || currentMenuItem == 2;
    RenderFormattedText(635,422,overSave ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.0f,/*"SAVE"*/Sys_Text.stringTable[727]);
    if (UI_Button(522,510, 322,60, &overOptions, 3) || (MenuEnter() && currentMenuItem == 3)) { currentMenuPage = Mpg_Options; PlayMenuMusic(); Sys_Global.menuActive = true; returnToPause = true; }
    overOptions = overOptions || currentMenuItem == 3;
    RenderFormattedText(599,480,overOptions ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.0f,/*"OPTIONS"*/Sys_Text.stringTable[721]);
    if (UI_Button(522,570, 322,60, &overQuitMenu, 4) || (MenuEnter() && currentMenuItem == 4)) OpenMainMenu();
    overQuitMenu = overQuitMenu || currentMenuItem == 4;
    RenderFormattedText(546,538,overQuitMenu ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.0f,/*"QUIT TO MENU"*/Sys_Text.stringTable[728]);
    RenderUIImage(519,672,328,42,1252); // Pause Quit Game background
    if (UI_Button(522,714, 322,42, &overQuit, 5) || (MenuEnter() && currentMenuItem == 5)) OS_Exit(0);
    overQuit = overQuit || currentMenuItem == 5;
    RenderFormattedText(572,690,overQuit ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_STOPD_RED,FONT_STOPD,1.0f,/*"QUIT GAME"*/Sys_Text.stringTable[729]);
}

static float g_debugLineColorR = 0.0f, g_debugLineColorG = 1.0f, g_debugLineColorB = 0.0f;
float debugLineBuffer[MAX_DEBUG_LINE_VERTS * 3]; // xyz only
void SetDebugLineColor(float r, float g, float b) { g_debugLineColorR = r; g_debugLineColorG = g; g_debugLineColorB = b; }
static inline __attribute__((always_inline)) void DrawDebugLines(float* viewProj) {
    if (Sys_Global.debugLineVertCount == 0) return;
    
    glNamedBufferSubData(Sys_Render.debugLinesVBO,0,Sys_Global.debugLineVertCount * sizeof(float),debugLineBuffer);
    glUseProgram(Sys_Render.debugUnlitShaderProgram);
    glUniformMatrix4fv(0,1,GL_FALSE,viewProj);
    glUniform3f(1, g_debugLineColorR,g_debugLineColorG,g_debugLineColorB);
    glLineWidth(10.0f);
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(Sys_Render.debugLinesVAO);
    glDrawArrays(0x0001/*GL_LINES*/,0,Sys_Global.debugLineVertCount / 3);
    glEnable(GL_DEPTH_TEST);
    drawCallsRenderedThisFrame++; verticesRenderedThisFrame += Sys_Global.debugLineVertCount / 3;
    Sys_Global.debugLineVertCount = 0;
}

ENGINE_TO_MOD void AddDebugLine(Vector3 start, Vector3 end) {
    i32 i = Sys_Global.debugLineVertCount;
    debugLineBuffer[i++] = start.x; debugLineBuffer[i++] = start.y; debugLineBuffer[i++] = start.z;
    debugLineBuffer[i++] =   end.x; debugLineBuffer[i++] =   end.y; debugLineBuffer[i++] =   end.z;
    Sys_Global.debugLineVertCount = i;
}

char creditStats[4096];
static inline __attribute__((always_inline)) float GetScore(float stupid, bool isFinal) {
    float victories = (float)(Sys_Global.kills + Sys_Global.cyberkills);
    if (isFinal) victories -= vmin(Sys_Global.ressurections * 10.0f, victories * 0.666f);
    float secs  = vfloor((float)Sys_Global.pauseRelativeTime / 3600.0f);
    float score = victories * 10000.0f;
    score -= vmin(score * 0.666f, secs * 100.0f);
    score *= (stupid + 1.0f) / 37.0f;
    if (stupid > 35.0f) score += 2222222.0f;
    return vfloor(score);
}

static inline __attribute__((always_inline)) void DecomposeTime(double t, u32* h, u32* m, double* s) {
    double tb = vfloor(t / 3600.0); *h = (u32)tb; t -= tb * 3600.0;
    tb = vfloor(t / 60.0);          *m = (u32)tb; *s = t - tb * 60.0;
}

static inline __attribute__((always_inline)) void CreditsStats(void) {
    size_t off = 0;
    off += StringFormat(creditStats + off, sizeof(creditStats),"================================================================================\nCITADEL\n================================================================================\nCONGRATULATIONS %s\n",Sys_Global.playerName);
    u32 h,m; double s;
    DecomposeTime(Sys_Global.pauseRelativeTime,&h,&m,&s);
    off += StringFormat(creditStats + off, sizeof(creditStats),"Straight Time: %uh %um %.3fs\n",h,m,s);
    DecomposeTime(Sys_Global.absoluteTime,&h,&m,&s);
    off += StringFormat(creditStats + off,sizeof(creditStats),"Total Time (with reload from deaths): %uh %um %.3fs\n",h,m,s);
    float stupid = ((float)(Sys_Global.difficultyCombat * Sys_Global.difficultyCombat)) + ((float)(Sys_Global.difficultyPuzzle * Sys_Global.difficultyPuzzle)) + ((float)(Sys_Global.difficultyMission * Sys_Global.difficultyMission)) + ((float)(Sys_Global.difficultyCyber * Sys_Global.difficultyCyber));
    u32 finalSubscore = GetScore(stupid,false), finalScore = (u32)GetScore(stupid,true);
    off += StringFormat(creditStats + off,sizeof(creditStats),"Kills: %u\nKills in Cyberspace: %u\nScoreSubtotal: %u\nDeaths: %u\nRessurections: %u\n",Sys_Global.kills,Sys_Global.cyberkills,(u32)finalSubscore,Sys_Global.deaths,Sys_Global.ressurections);
    off += StringFormat(creditStats + off,sizeof(creditStats),"Combat: %u | Puzzle: %u | Mission: %u | Cyber: %u\n",Sys_Global.difficultyCombat,Sys_Global.difficultyPuzzle,Sys_Global.difficultyMission,Sys_Global.difficultyCyber);
    off += StringFormat(creditStats + off,sizeof(creditStats),"Difficulty Index: %.2f\nFinal Score: %u\n\n",stupid,finalScore);
    off += StringFormat(creditStats + off,sizeof(creditStats),"Shots Fired: %u\nGrenades Thrown: %u\n",Sys_Global.shotsFired,Sys_Global.grenadesThrown);
    off += StringFormat(creditStats + off,sizeof(creditStats),"Damage Dealt: %f\nDamage Received: %f\nSaves Scummed: %u\n\nClick to continue...\n",Sys_Global.damageDealt,Sys_Global.damageReceived,Sys_Global.savesScummed);
}

u8 MFD_LefTab=0,MFD_CenterTab=0,MFD_RightTab=0;
static inline __attribute__((always_inline)) double RenderUI(void) {
    drawCallsNormal = drawCallsRenderedThisFrame;
    if (Sys_Global.creditsActive) { // Render Credits
        if (Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_LEFT].pressed) {
            ++Sys_Global.creditsPageIndex;
            if (Sys_Global.creditsPageIndex > CREDITS_PAGES) {Sys_Global.creditsActive = false; return get_time(); } // Finished with Erthang!  That's it, go home.
        }

        if (Sys_Global.creditsPageIndex == 1) { CreditsStats(); RenderFormattedText(300,10,TEXT_WHITE,FONT_NORMAL,1.0f,(const char*)&creditStats); }
        else                                                    RenderFormattedText(300,10,TEXT_WHITE,FONT_NORMAL,1.0f,creditPages[Sys_Global.creditsPageIndex]);
        
        return get_time();
    }
    if (Sys_Global.menuActive) RenderMenu();
    else if (Sys_Global.gamePaused) RenderPausedUI();
    if ((Sys_Global.menuActive || Sys_Global.gamePaused)) {
        if (Sys_Input.keyStates[GLFW_KEY_DOWN].pressed) currentMenuItem = (currentMenuItem + 1) >= menuItemCount ? 0 : (currentMenuItem + 1);
        else if (Sys_Input.keyStates[GLFW_KEY_UP].pressed) currentMenuItem = (currentMenuItem - 1) < 0 ? (menuItemCount - 1) : (currentMenuItem - 1);
    } else {
        if (!Sys_Global.gamePaused && !Sys_Cheats.noHUD) RenderUIImage(672,0,22,22,1020); // Shoot mode button
        bool mouseReleased = Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_LEFT].pressed;
        if (Sys_Global.inventoryMode && mouseReleased && CursorIsOverBounds(672,694,22,0)) ForceShootMode();
        
        // Left MFD
        RenderUIImage(-16,552,32,40,MFD_LefTab == 0 ? 1024 : 1022);
        RenderUIImage(-16,600,32,40,MFD_LefTab == 1 ? 1024 : 1022);
        RenderUIImage(-16,648,32,40,MFD_LefTab == 2 ? 1024 : 1022);
        RenderUIImage(-16,696,32,40,MFD_LefTab == 3 ? 1024 : 1022);
        
        // Center Tabs
        RenderUIImage(1350,552,32,40,MFD_RightTab == 0 ? 1024 : 1022);
        RenderUIImage(1350,600,32,40,MFD_RightTab == 1 ? 1024 : 1022);
        RenderUIImage(1350,648,32,40,MFD_RightTab == 2 ? 1024 : 1022);
        RenderUIImage(1350,696,32,40,MFD_RightTab == 3 ? 1024 : 1022);
        
        // Right MFD
        RenderUIImage(400,752,64,32,MFD_CenterTab == 0 ? 1024 : 1021);
        RenderUIImage(480,752,64,32,MFD_CenterTab == 1 ? 1024 : 1021);
        RenderUIImage(560,752,64,32,MFD_CenterTab == 2 ? 1024 : 1021);
        RenderUIImage(902,752,64,32,MFD_CenterTab == 3 ? 1024 : 1021);
    }
    
    // Diagnostics / Debugging
    i16 debugTextStartY = 48;
    if (Sys_Cheats.showLocation && !Sys_Global.menuActive) RenderFormattedText(16, debugTextStartY, TEXT_WHITE, FONT_NORMAL,1.0f, "x: %.4f, y: %.4f, z: %.4f, rx: %.4f, ry: %.4f, rz: %.4f, rw: %.4f",Sys_Global.instances[PLAYER1].position.x,Sys_Global.instances[PLAYER1].position.y,Sys_Global.instances[PLAYER1].position.z,Sys_Global.instances[PLAYER1].rotation.x,Sys_Global.instances[PLAYER1].rotation.y,Sys_Global.instances[PLAYER1].rotation.z,Sys_Global.instances[PLAYER1].rotation.w);
    i16 lineSpacing = 18;
    if (!Sys_Global.menuActive && !Sys_Cheats.noHUD) RenderFormattedText(16,debugTextStartY + (lineSpacing * 1),TEXT_WHITE,FONT_NORMAL,1.0f,"playerCellIdx: %u, Shadow cpu ms: %.3f",playerCellIdx,voxen_Shadow_System.shadowTime * 1000);
    if (!Sys_Global.menuActive && !Sys_Cheats.noHUD) RenderFormattedText(16,debugTextStartY + (lineSpacing * 2),TEXT_WHITE,FONT_NORMAL,1.0f,"Player velocity: %.2f, %.2f, %.2f",Sys_Global.instances[PLAYER1].velocity.x,Sys_Global.instances[PLAYER1].velocity.y,Sys_Global.instances[PLAYER1].velocity.z);
    RenderFormattedText(16,debugTextStartY + (lineSpacing * 4),TEXT_WHITE,FONT_NORMAL,1.0f,"Cursor: %d, %d  dx:%d dy:%d",Sys_Global.cursorPosition_x,Sys_Global.cursorPosition_y,Sys_Input.currentMouse_dx,Sys_Input.currentMouse_dy);
    if (Sys_Cheats.consoleActive) RenderFormattedText(16,0,TEXT_WHITE,FONT_NORMAL,1.0f, "] %s",consoleEntryText);
    if (Sys_Global.statusTextDecayFinished > Sys_Global.current_time) RenderFormattedText(479,114,TEXT_WHITE,FONT_NORMAL,1.0f, "%s",statusText);
    double time_now = get_time();
    if (Sys_Cheats.showFPS) {
        Sys_Global.thisFrameTime = (time_now - Sys_Global.last_time) * 1000.0;
        Sys_Global.cpuFrameTime = Sys_Global.cpuTime * 1000.0;
        u8 timingColor = TEXT_WHITE;
        if (vabs(Sys_Global.thisFrameTime - Sys_Global.cpuFrameTime) < 0.451) timingColor = TEXT_GREEN;
        if (Sys_Global.thisFrameTime > 6.944444) timingColor = TEXT_RED;
        drawCallsRenderedThisFrame += 2; // Add two more for this text render ;)
        RenderFormattedText(16, debugTextStartY - lineSpacing, timingColor, FONT_NORMAL,1.0f, "ms: %.2f, CPU %.2f", Sys_Global.thisFrameTime,Sys_Global.cpuFrameTime);
        RenderFormattedText(16 + 230.0f, debugTextStartY - lineSpacing, TEXT_WHITE, FONT_NORMAL,1.0f, "(FPS:%d, Worst:%d),Drwclls:%d [G:%d UI:%d Sh:%d] Vrt:%d E:%u|M:%u|P:%u",Sys_Global.framesPerLastSecond,Sys_Global.worstFPS,drawCallsRenderedThisFrame,drawCallsNormal,uiImageDrawCallsRenderedThisFrame,shadowDrawCallsRenderedThisFrame,verticesRenderedThisFrame,Sys_Cheats.editMode,Sys_Global.menuActive,Sys_Global.gamePaused);
    }
    
    return time_now;
}

#define SHADOW_NEARMESH_MAX 1024
typedef struct {float depth; u16 index; } DepthSort;
DepthSort shadows_nearMeshes[SHADOW_NEARMESH_MAX];
float shadows_nearMeshRadii[SHADOW_NEARMESH_MAX];
static inline __attribute__((always_inline)) bool EntNotVisible(u16 i, bool otherCondition) { Entity* e = &Sys_Global.instances[i]; return e->texIndex > loadedTexturesMaxIndex || !(e->entflags & ENTFLAG_ACTIVE) || e->index >= MAX_ENTITIES || e->modelIndex >= MODEL_IDX_MAX || e->texIndex >= MAX_VALID_TEXTURE || otherCondition; }
static inline __attribute__((always_inline,hot)) u16 GetAndBindModel(u16 i, u16 currentModelType) {
    glUniform1ui(0,i);
    u16 modelType = (instanceIsLODArray[i] || Sys_Settings.ModelDetail < 1u) && Sys_Global.instances[i].lodIndex < loadedModelsMaxIndex ? Sys_Global.instances[i].lodIndex : Sys_Global.instances[i].modelIndex;
    if (currentModelType == modelType && currentModelType != 0) return currentModelType;
    
    glBindVertexBuffer(0,Sys_Render.vbos[modelType],0,VERTEX_ATTRIBUTES_SIZE);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,Sys_Render.tbos[modelType]);
    return modelType;
}

#define SC_MAX (SHADOW_NEARMESH_MAX * MAX_SHADOWMAPS)
static const u32 groupX_shadClear = ((SHADOW_MAP_SIZE * SHADOW_MAP_SIZE) + 31) / 32;
static inline __attribute__((always_inline,hot)) void RenderShadowmaps(void) {    
    double shadowStartTime = get_time();
    u16 candidates[MAX_SHADOWMAPS];
    u16 numShadowsCouldRender = 0;
    Vector3 playerPos = Sys_Global.instances[PLAYER1].position;
    Vector3 pf = Sys_Global.instances[PLAYER1].forward;
    float minx = Sys_Global.worldMin_x, minz = Sys_Global.worldMin_z;
    for (u16 i = 0; i < Sys_Global.loadedLights; ++i) { // Collect candidates: only lights that are enabled and in PVS
        if (unlikely(!(lights[i].lflags & SHADON) || !(lights[i].lflags & LIGHTON))) continue;

        Vector3 lightPos = lights[i].pos;
        float intensity = lights[i].maxIntensity; // Much more stable than actual intensity (from fade/flickers).  Since gated by on above, this is fine now.
        if (unlikely(intensity < 0.1f)) continue;
        
        float range =  lights[i].range;
        float luminosity = (intensity / (range * range));
        if (luminosity < 0.008f && (range < 8.0f || intensity < 0.5f)) continue;
        
        u16 cellX = (u16)clamp((i32)vfloor((lightPos.x - minx + CELLXHALF) / CELL_SIZE), 0, WORLDX_0BASED);
        u16 cellZ = (u16)clamp((i32)vfloor((lightPos.z - minz + CELLXHALF) / CELL_SIZE), 0, WORLDX_0BASED);
        int lightCellIdx = (cellZ * WORLDX) + cellX;
        int r = vceil(range * (1.0f / CELL_SIZE));
        bool inPVS = (gridCellStates[lightCellIdx] & CELL_VISIBLE);
        if (likely(!inPVS)) inPVS = NeighborhoodInPVS(cellX,cellZ,r);
        if (!inPVS) continue;
        
        float dx = lightPos.x - playerPos.x; float dy = lightPos.y - playerPos.y; float dz = lightPos.z - playerPos.z;
        float distSqrdToPlayer = dx*dx + dy*dy + dz*dz;
        float dotResult = (dx*pf.x + dy*pf.y + dz*pf.z);
        if (dotResult < 0.0f && distSqrdToPlayer > (range * range)) continue;

        candidates[numShadowsCouldRender] = i;
        numShadowsCouldRender++;
        if (numShadowsCouldRender >= MAX_SHADOWMAPS) break;
    }

    if (numShadowsCouldRender > 0) { // Added since there is now work between here and the for loop so this is beneficial to check.
        glUseProgram(Sys_Render.shadowmapsClearShaderProgram); // Clear shadowmaps.  One might think that this would be less performant than standard shadowmap FBO with gl clears and textures but in fact this is faster on all but the oldest hardware (e.g. 10yrs old is fine, 13yrs suffers a small hit).
        for (u32 c=0;c<numShadowsCouldRender;++c) {
            glUniform1ui(0,c); glDispatchCompute(groupX_shadClear,6,1);
        }

        shadowDrawCallsRenderedThisFrame = 0;
        glViewport(0,0,SHADOW_MAP_SIZE,SHADOW_MAP_SIZE);
        glUseProgram(Sys_Render.shadowmapsShaderProgram);
        u32 shadowmapOffsetHead = 0U;
        u16 shadowCasterIndices[SC_MAX];
        u32 numShadowCasters = 0;
        for (int i=START_INDEX_LEVEL_INSTANCES;i<INSTANCE_COUNT;++i) {
            if (EntNotVisible(i,(Sys_Global.instances[i].entflags & ENTFLAG_NO_SHADOWS))) continue;

            shadowCasterIndices[numShadowCasters] = i;
            numShadowCasters++;
            if (numShadowCasters >= (SC_MAX)) break; // Ran out of shadowcasters max for frame.
        }
        
        u16 shadowMapIdx=0,currentModelType=0,currentTexIndex=0; bool currentIsTransparent=0;
        bool useDetail = Sys_Settings.ModelDetail;
        for (u32 c = 0; c < numShadowsCouldRender; ++c, ++shadowMapIdx) { // Render top MAX_SHADOWMAPS candidates
            u16 lightIdx = candidates[c];
            float effectiveRadius = vmin(lights[lightIdx].range,15.36f);
            u16 nearbyMeshCount = 0;
            Vector3 lpos = lights[lightIdx].pos;
            float cellCenterX=vround(lpos.x / CELL_SIZE) * CELL_SIZE, cellCenterZ=vround(lpos.z / CELL_SIZE) * CELL_SIZE;
            Vector3 deltaCellCenter = Vector3_A_minus_B((Vector3){lpos.x,0.0f,lpos.z},(Vector3){cellCenterX,0.0f,cellCenterZ});
            float distToCenterSqrd = dot_vector3(deltaCellCenter,deltaCellCenter);
            bool skipNPCs = (distToCenterSqrd < 0.4096f); // 0.64 * 0.64
            for (u16 shadowCasterInstanceIdx = 0; shadowCasterInstanceIdx < numShadowCasters; shadowCasterInstanceIdx++) {
                u16 j = shadowCasterIndices[shadowCasterInstanceIdx];
                Entity* e = &Sys_Global.instances[j];
                Vector3 d = Vector3_A_minus_B(e->position,lpos);
                float distToLightSqrd = dot_vector3(d,d);
                float radSum = (effectiveRadius + e->radius);
                if (distToLightSqrd >= radSum * radSum) continue;
                if (skipNPCs && ConstIndexIsNPC(e->index)) continue;
                
                shadows_nearMeshes[nearbyMeshCount].index = j; shadows_nearMeshes[nearbyMeshCount].depth = distToLightSqrd; 
                nearbyMeshCount++; if (nearbyMeshCount >= SHADOW_NEARMESH_MAX) { DualLogWarn("Shadowmapping needs larger nearMeshes count than %u!  Skipping some renderables for light %u!\n", SHADOW_NEARMESH_MAX, lightIdx); break; }
            }

            if (unlikely(nearbyMeshCount < 1)) continue;

            glUniform3f(3,lpos.x,lpos.y,lpos.z);
            voxen_Shadow_System.shadowmapIndirectionList[lightIdx] = shadowMapIdx;
            #pragma GCC unroll 6
            for (u8 face = 0; face < 6; face++) {                                            
                glUniform1ui(2,face);
                glUniformMatrix4fv(1,1,GL_FALSE,(float*)lightViewProj[lightIdx][face]);
                glUniform1ui(7,shadowmapOffsetHead + (face * SHADOW_MAP_SIZE * SHADOW_MAP_SIZE));
                for (u16 j = 0; j < nearbyMeshCount; ++j) {
                    int i = shadows_nearMeshes[j].index;
                    Entity* e = &Sys_Global.instances[i];
                    if (!SphereInFrustum(lightFrustumPlanes[lightIdx][face],e->position,e->shadRadius)) continue;

                    glUniform1ui(0,i);
                    u16 modelType = (instanceIsLODArray[i] || useDetail < 1u) && e->lodIndex < loadedModelsMaxIndex ? e->lodIndex : e->modelIndex;
                    if (currentModelType != modelType || currentModelType == 0) { currentModelType = modelType; glBindVertexBuffer(0,Sys_Render.vbos[modelType],0,VERTEX_ATTRIBUTES_SIZE); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,Sys_Render.tbos[modelType]); }
                    if (currentTexIndex != e->texIndex) { currentTexIndex = e->texIndex; glUniform1ui(6,e->texIndex); }
                    bool texIsTransparent = transparentTexture[e->texIndex];
                    if (currentIsTransparent != texIsTransparent) { currentIsTransparent = texIsTransparent; glUniform1ui(8,currentIsTransparent ? 1u : 0u); }
                    glDrawElements(0x0004/*GL_TRIANGLES*/,modelTriangleCounts[currentModelType]*3,GL_UNSIGNED_SHORT,0); drawCallsRenderedThisFrame++; shadowDrawCallsRenderedThisFrame++; verticesRenderedThisFrame += modelTriangleCounts[currentModelType] * 3;
                }
            }
            
            shadowmapOffsetHead += (SHADOW_MAP_SIZE * SHADOW_MAP_SIZE) * 6;
        }

        glViewport(0,0,Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight);
        glNamedBufferData(Sys_Render.shadowMapsIndirectionID,Sys_Global.loadedLights * sizeof(u32),voxen_Shadow_System.shadowmapIndirectionList,GL_DYNAMIC_DRAW);
    }

    voxen_Shadow_System.shadowTime = get_time() - shadowStartTime;
}

DepthSort visibleInstances[INSTANCE_COUNT];
static inline __attribute__((always_inline)) bool DetermineIfInstanceVisible(u16 i, bool otherCondition, bool skyVisible, Vector3 playerPos, float* distSqrd) {
    if (EntNotVisible(i,otherCondition)) return false; // must be transparent && transparents or neither
    
    Entity* e = &Sys_Global.instances[i];
    u16 instCellIdx = e->cellIndex; u16 entIdx = e->index;
    Vector3 delta = Vector3_A_minus_B(e->position,playerPos);
    *distSqrd = delta.x*delta.x + delta.y*delta.y + delta.z*delta.z;
    float radius = modelBounds[e->modelIndex] * 2.0f * vmax(vmax(e->scale.x,e->scale.y),e->scale.z);
    if (!SphereInFrustum(playerFrustumPlanes,e->position,radius) && (entIdx != 754 || !skyVisible) && i != editModeSelection) return false;
    
    if (ConstIndexIsPortalBlockingDoor(entIdx)) { // Extra checks only needed for opaque portal blocking doors.
        bool inPVS = (gridCellStates[instCellIdx] & CELL_VISIBLE);
        if (!inPVS) inPVS = NeighborhoodInPVS(e->cellX,e->cellZ,2);
        if (!inPVS) return false;
    } else {
        if (((gridCellStates[instCellIdx] & (CELL_VISIBLE | CELL_OPEN)) == CELL_OPEN) && (entIdx != 754 || !skyVisible)) return false;
        if (!(gridCellStates[instCellIdx] & CELL_OPEN) && *distSqrd >= 943.7184f && (entIdx != 754 || !skyVisible)) return false; // 30.72 * 30.72, 12 cells
    }

    if (Sys_Global.instances[i].camView != 255) camViews[Sys_Global.instances[i].camView].visible = true;
    return true;
}

float GetPainStatic(void) { return 0.0f; } // TODO: Hook into pain/health management and shield impact effect
Color GetPainStaticColor(void) { return (Color){1.0f,0.0f,0.0f,1.0f}; } // TODO: Hook staticColor up to red or blue for pain or shield impact.

__attribute__((pure)) i32 dsort(const void* a, const void* b) { float da = ((const DepthSort*)a)->depth; float db = ((const DepthSort*)b)->depth; return (db > da) - (db < da); }
__attribute__((pure)) i32 dsortInv(const void* a, const void* b) { float da = ((const DepthSort*)a)->depth; float db = ((const DepthSort*)b)->depth; return (da > db) - (da < db); }
void qsort(void* base, size_t nmemb, size_t size, int (*cmp)(const void*, const void*));
// Bind textures only on change (norm/tex/glow/spec); cmi<camViewCount handled inline
#define MAX_VISIBLE 4096
#define BIND_TEX(slot,cur,next) if((cur)!=(next)||(next)==0){(cur)=(next);glUniform1ui(slot,(u32)(next));}
#define SET_CAMVIEW(cmi,constIndex,gs) \
    glUniform1ui(30,(cmi)<camViewCount?1u:0u); \
    if((cmi)<camViewCount){ \
        glActiveTexture(GL_TEXTURE6);glBindTexture(GL_TEXTURE_2D,camViewTextures[cmi]); \
        glUniform2ui(28,camViews[cmi].width,camViews[cmi].height);glUniform1i(29,6); \
        if(gs){float _h=ConstIndexIsNPC(constIndex)?(( \
            constIndex==419||constIndex==422||constIndex==424||constIndex==429||constIndex==430|| \
            constIndex==431||constIndex==433||constIndex==437||constIndex==438||constIndex==441)?1.5f:4.0f):0.0f; \
            glUniform1f(9,_h);} \
    }
#define DRAW_ENTITY(i,curN,curT,curG,curS,curM) \
    {Entity*e=&Sys_Global.instances[i];u16 tex=e->texIndex,glow=e->glowIndex,norm=e->normIndex,spec=e->specIndex; \
     u32 ci=e->index; \
     glUniform1ui(17,tex==316?1u:0u); \
     glUniform1ui(25,ci); \
     glUniform1f(27,e->volume); \
     glUniform1ui(13,(tex==36||tex==887)?1u:0u); \
     SET_CAMVIEW(e->camView,ci,grayscaleEnabled) \
     BIND_TEX(1,curN,norm) BIND_TEX(18,curT,tex) BIND_TEX(19,curG,glow) BIND_TEX(20,curS,spec) \
     curM=GetAndBindModel(i,curM); \
     u32 vc=modelTriangleCounts[curM]*3; \
     glDrawElements(0x0004,vc,GL_UNSIGNED_SHORT,0);drawCallsRenderedThisFrame++;verticesRenderedThisFrame+=vc;}

static inline __attribute__((always_inline)) __attribute__((hot)) void Render(bool camView, u8 camViewIdx) {
    u16 swidth = camView ? camViews[camViewIdx].width : Sys_Settings.ScreenWidth, sheight = camView ? camViews[camViewIdx].height : Sys_Settings.ScreenHeight;
    float sfov = camView ? (float)camViews[camViewIdx].fov : (float)Sys_Settings.FOV;
    float snear = camView ? camViews[camViewIdx].near : NEAR_PLANE; float sfar = camView ? camViews[camViewIdx].far : Sys_Global.farPlane;
    Vector3 playerPos = Sys_Global.instances[PLAYER1].position;
    float px = playerPos.x, py = playerPos.y, pz = playerPos.z;
    float aspect3D = (float)swidth / (float)sheight;
    float f = vcot(sfov * PI / 360.0f);
    float* m = rasterPerspectiveProjection;
    m[0] = f / aspect3D; m[1] = 0.0f; m[2] = 0.0f; m[3] = 0.0f;
    m[4] = 0.0f; m[5] = f; m[6] = 0.0f; m[7] = 0.0f;
    m[8] = 0.0f; m[9] = 0.0f; m[10]= -(sfar + snear) / (sfar - snear); m[11]= -1.0f;
    m[12]= 0.0f; m[13]= 0.0f; m[14]= -2.0f * sfar * snear / (sfar - snear); m[15]= 0.0f;
    voxen_Shadow_System.shadDotThresh = 1.0f / vsqrtf(1.0f + vtan(sfov * PI / 360.0f) * (1.0f + aspect3D * aspect3D));
    float x = Sys_Global.instances[PLAYER1].rotation.x, y = Sys_Global.instances[PLAYER1].rotation.y, z = Sys_Global.instances[PLAYER1].rotation.z, w = Sys_Global.instances[PLAYER1].rotation.w;
    float x2=x*x, y2=y*y, z2=z*z, xy=x*y, xz=x*z, yz=y*z, wx=w*x, wy=w*y, wz=w*z;
    Vector3 right = { 1.0f - 2.0f * (y2 + z2), 2.0f * (xy + wz), 2.0f * (xz - wy) };   // X+ (right)
    Vector3 up = { 2.0f * (xy - wz), 1.0f - 2.0f * (x2 + z2), 2.0f * (yz + wx) };      // Y+ (up)
    Vector3 forward = { 2.0f * (xz + wy), 2.0f * (yz - wx), 1.0f - 2.0f * (x2 + y2) }; // Z+ (forward)
    float view[16]; // view matrix
    view[0] = right.x; view[1] = up.x; view[2] = -forward.x; view[3] = 0.0f;
    view[4] = right.y; view[5] = up.y; view[6] = -forward.y; view[7] = 0.0f;
    view[8] = right.z; view[9] = up.z; view[10] = -forward.z; view[11] = 0.0f;
    view[12] = -dot_vector3(right,playerPos); view[13] = -dot_vector3(up,playerPos); view[14] = dot_vector3(forward,playerPos); view[15] = 1.0f;
    float viewProj[16]; // view-projection matrix
    mul_mat4(viewProj,rasterPerspectiveProjection,view);
    float invViewRot[9] = {view[0],view[4],view[8], view[1],view[5],view[9], view[2],view[6],view[10]};
    ExtractFrustumPlanes(viewProj,playerFrustumPlanes);
    glBindVertexArray(Sys_Render.vao_chunk); // Common vao for RenderDynamicShadowmaps and Rasterized Geometry
    glEnable(GL_DEPTH_TEST);
    if (likely(Sys_Settings.Shadows > 0u)) RenderShadowmaps();
    UpdateLights(); // This is where the voxels get updated!
    for (int i=0;i<LIGHT_COUNT;++i) flag_setu32(&lights[i].lflags,LDIRTY,false);
    MemSetToValueForNBytes(Sys_Global.dirtyInstances,0,Sys_Global.loadedInstances * sizeof(bool));
    glViewport(0,0,swidth,sheight);
    ClearAll();
    glBindFramebuffer(GL_FRAMEBUFFER,Sys_Render.gBufferFBO);
    glEnable(GL_CULL_FACE); glDisable(GL_BLEND); // Opaques
    u16 visibleCount = 0, currentTexIndex = 0, currentNormIndex = 0, currentGlowIndex = 0, currentSpecIndex = 0, currentModelType = 0, opaqueCount = 0;
    bool skyVisible = (gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX);
    float distSqrd = sfar * sfar;
    DepthSort tmpTransparent[MAX_VISIBLE]; u16 tcnt = 0;
    for (u16 i = START_INDEX_LEVEL_INSTANCES; i < INSTANCE_COUNT; ++i) { // Determine base visibility
        if (!DetermineIfInstanceVisible(i,false,skyVisible,playerPos,&distSqrd)) continue;
       
        if (transparentTexture[Sys_Global.instances[i].texIndex]) { tmpTransparent[tcnt].index = i; tmpTransparent[tcnt].depth = distSqrd; tcnt++; }
        else { visibleInstances[opaqueCount].index = i; visibleInstances[opaqueCount].depth = distSqrd; opaqueCount++; }
    }

    CopyMemoryFromBtoAForNBytes(visibleInstances + opaqueCount,tmpTransparent,tcnt * sizeof(DepthSort));
    visibleCount = opaqueCount + tcnt;
    glUseProgram(Sys_Render.depthPrepassShaderProgram); // Depth Prepass - Eliminates some overdraw for ~6.1% performance improvement in spite of added draw calls
    glUniformMatrix4fv(2,1,0,viewProj);
    glEnable(GL_DEPTH_TEST); glColorMask(0,0,0,0); glDepthMask(1); glDepthFunc(0x0201/*GL_LESS*/); glDisable(GL_BLEND);
    if (opaqueCount > 1) qsort(visibleInstances,opaqueCount,sizeof(DepthSort),dsortInv);
    if (tcnt > 1) qsort(visibleInstances + opaqueCount,tcnt,sizeof(DepthSort),dsort);
    for (u16 visibleIndex = 0; visibleIndex < visibleCount; ++visibleIndex) {
        u16 i = visibleInstances[visibleIndex].index;
        Entity* e = &Sys_Global.instances[i]; u16 tex = e->texIndex;
        if (transparentTexture[tex]) { glEnable(GL_CULL_FACE); glEnable(GL_BLEND); } // Transparents (with sort)
        else if (doubleSidedTexture[tex] || e->scale.x < 0.0f || e->scale.y < 0.0f || e->scale.z < 0.0f) { glDisable(GL_CULL_FACE); glEnable(GL_BLEND); } // Doublesided
        else { glEnable(GL_CULL_FACE); glDisable(GL_BLEND); } // Opaque
       
        currentModelType = GetAndBindModel(i,currentModelType);
        glUniform1ui(3,(u32)tex);
        u32 vertCount = modelTriangleCounts[currentModelType] * 3;
        glDrawElements(0x0004/*GL_TRIANGLES*/,vertCount,GL_UNSIGNED_SHORT,0); drawCallsRenderedThisFrame++; verticesRenderedThisFrame += vertCount;
    }

    glUseProgram(Sys_Render.chunkShaderProgram); // Main Pass
    glUniformMatrix4fv(2,1,0,viewProj);
    glUniform1ui(25,0u); // default constIndex
    bool grayscaleEnabled = ModRequestsGrayscale();
    glUniform1ui(26,(u32)grayscaleEnabled);
    float fogActual = Sys_Global.fogColor.a + (float)(Sys_Global.fogFac / 255u); // Alpha is base density for level.
    glUniform3f(12,Sys_Global.fogColor.r * fogActual,Sys_Global.fogColor.g * fogActual,Sys_Global.fogColor.b * fogActual); // Fog Color(which is density)
    glUniform1ui(14,Sys_Settings.Reflections); glUniform1ui(15,Sys_Settings.Shadows);
    glUniform2f(8,Sys_Global.worldMin_x,Sys_Global.worldMin_z); glUniform3f(10,playerPos.x,playerPos.y,playerPos.z);
   
    glColorMask(1,1,1,1); glDepthMask(0); glDepthFunc(0x0203/*GL_LEQUAL*/); // Opaque Pass
    visibleCount = currentTexIndex = currentNormIndex = currentGlowIndex = currentSpecIndex = currentModelType = 0;
    glUniform1f(9,0.0f); // Reset heat for infrared vision
    for (u16 visibleIndex = 0; visibleIndex < opaqueCount; ++visibleIndex) { // Opaques (already front-to-back)
        u16 i = visibleInstances[visibleIndex].index;
        Entity* e = &Sys_Global.instances[i]; u16 tex = e->texIndex;
        if (transparentTexture[tex]) continue;
        else if (doubleSidedTexture[tex] || e->scale.x < 0.0f || e->scale.y < 0.0f || e->scale.z < 0.0f) { glDisable(GL_CULL_FACE); glEnable(GL_BLEND); } // Doublesided (either)
        else { glEnable(GL_CULL_FACE); glDisable(GL_BLEND); } // Opaque
       
        DRAW_ENTITY(i,currentNormIndex,currentTexIndex,currentGlowIndex,currentSpecIndex,currentModelType)
    }

    glDepthMask(1); currentTexIndex = currentNormIndex = currentGlowIndex = currentSpecIndex = currentModelType = 0; // Transparents Pass
    glUniform1f(9,0.0f); // Reset heat for infrared vision
    for (u16 visibleIndex = opaqueCount; visibleIndex < (opaqueCount + tcnt); ++visibleIndex) {
        u16 i = visibleInstances[visibleIndex].index;
        Entity* e = &Sys_Global.instances[i]; u16 tex = e->texIndex;
        if (transparentTexture[tex]) { glEnable(GL_CULL_FACE); glEnable(GL_BLEND); } // Transparents (with sort)
        else if (doubleSidedTexture[tex] || e->scale.x < 0.0f || e->scale.y < 0.0f || e->scale.z < 0.0f) { glDisable(GL_CULL_FACE); glEnable(GL_BLEND); } // Doublesided (either)
        else continue; // Opaque
       
        u32 constIndex = e->index;
        if ((constIndex >= 561 && constIndex <= 565) || (constIndex >= 568 && constIndex <= 573)) glDepthFunc(0x0202/*GL_EQUAL*/); // Cutouts
        else glDepthFunc(0x0203/*GL_LEQUAL*/); // Actual alphas
       
        DRAW_ENTITY(i,currentNormIndex,currentTexIndex,currentGlowIndex,currentSpecIndex,currentModelType)
    }
   
    if (camView) {
        glBindFramebuffer(0x8CA8/*GL_READ_FRAMEBUFFER*/,Sys_Render.gBufferFBO);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glBindTexture(GL_TEXTURE_2D,camViewTextures[camViewIdx]);
        glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0,0,0,swidth,sheight);
        glBindTexture(GL_TEXTURE_2D,0);
        return; // After copying render result, skip SSR and composite for camviews <<<<<<<<<<<<< CAM VIEW BARRIER
    }

    if (unlikely(Sys_Global.debugLineVertCount > 1)) DrawDebugLines(viewProj); // Draw Debug Lines
    if (likely(Sys_Settings.Reflections > 0u)) { // Screen Space Reflections
        glUseProgram(Sys_Render.ssrShaderProgram);
        glUniform3f(3,playerPos.x,playerPos.y,playerPos.z);
        glUniformMatrix4fv(4,1,GL_FALSE,viewProj);
        u32 groupX_ssr = ((Sys_Settings.ScreenWidth / Sys_Settings.SSR_RES) + 31) / 32, groupY_ssr = ((Sys_Settings.ScreenHeight / Sys_Settings.SSR_RES) + 31) / 32;
        glDispatchCompute(groupX_ssr,groupY_ssr,1);
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER,Sys_Render.uiFBO); glViewport(0,0,1366,768); glDisable(GL_CULL_FACE); glDisable(GL_BLEND); // MUST DISABLE BLEND OR UI DISSAPPEARS!!
    Sys_Global.last_time = RenderUI();
    if ((Sys_Global.inventoryMode && !Sys_Cheats.noHUD) || Sys_Global.menuActive || Sys_Global.gamePaused) RenderUIImage((i16)(Sys_Global.cursorPosition_x) - 20,(i16)(Sys_Global.cursorPosition_y) - 20,40,40,GetCursorTexture());
    else RenderUIImage(663,371,40,40,GetCursorTexture()); // Centered on UI baseline resolution 1366x768
    glBindFramebuffer(GL_FRAMEBUFFER,0); glViewport(0,0,swidth,sheight); // Restore normal output size for final composite blit
    
    glUseProgram(Sys_Render.imageBlitShaderProgram);
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D,Sys_Render.inputImageID);
    glUniform1i(4,4); // outputImage texture sampler2D, don't remember why when active texture is texture 0. meh.... oh maybe to not read and write same binding?
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D,Sys_Render.inputUIID);
    glUniform1i(31,1);
    double berserkTimeRemainingNormalized = Sys_Global.invP1.berserkFinishedTime > 0.0001 ? (Sys_Global.invP1.berserkFinishedTime - Sys_Global.pauseRelativeTime) / BERSERK_TIME : 0.0;
    if (Sys_Global.invP1.berserkFinishedTime < Sys_Global.pauseRelativeTime && Sys_Global.invP1.berserkFinishedTime > 0.0001) Sys_Global.invP1.berserkFinishedTime = berserkTimeRemainingNormalized = 0.0;
    glUniform1ui(5,Sys_Settings.Reflections);
    glUniform1ui(6,Sys_Settings.FXAA);
    glUniform1f(14,Sys_Settings.FOV);
    glUniform1f(16,(float)swidth / (float)sheight);
    glUniform1ui(22,Sys_Settings.Shadows);
    glUniform1f(9,(float)berserkTimeRemainingNormalized);
    glUniform1f(10,berserkSeedTime);
    glUniform1ui(11,Sys_Settings.Brightness);
    glUniform3f(12,deg2rad(cam_yaw),deg2rad(cam_pitch),deg2rad(cam_roll));
    glUniform3f(13,px,py,pz);
    glUniform1f(15,(float)Sys_Global.pauseRelativeTime * 0.1f);
    glUniform1ui(17,(gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX) || Sys_Global.currentLevel == LEVEL_CYBERSPACE);
    glUniform1ui(18,(gridCellStates[playerCellIdx] & CELL_SEES_SUN) && Sys_Global.currentLevel != LEVEL_CYBERSPACE);
    glUniform1ui(19,((Sys_Global.currentLevel >= 10 && Sys_Global.currentLevel < LEVEL_CYBERSPACE) ? 1u : 0u) && (gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX));
    u32 shieldOnType = 0u; // No shield green tint.
    if (Sys_Global.instances[WORLD].ioflags & QUESTBIT_SHIELD_ACTIVATED) {
        if (Sys_Global.currentLevel == 6 || Sys_Global.currentLevel == 7) shieldOnType = 2u; // Shielding only below player for lower levels.
        else if (Sys_Global.currentLevel <= 5) shieldOnType = 1u; // Shielding everywhere as levels fully within shield.
    }
   
    glUniform1ui(20,shieldOnType);
    Color painStaticColor = GetPainStaticColor();
    glUniform3f(23,painStaticColor.r,painStaticColor.g,painStaticColor.b);
    glUniformMatrix4fv(24,1,0,viewProj);
    glUniformMatrix3fv(25,1,0,invViewRot);
    glUniform1i(27,0); // Texture 0 for the rendered geometry color buffer
    glUniform1f(28,GetPainStatic());
    glUniform1ui(29,(u32)ModRequestsGrayscale()); // Grayscale
    glBindVertexArray(Sys_Render.quadVAO);
    glDisable(GL_DEPTH_TEST);
    glDrawArrays(0x0006/*GL_TRIANGLE_FAN*/,0,4);
    drawCallsRenderedThisFrame++; verticesRenderedThisFrame += 4;
    if ((Sys_Global.last_time - Sys_Global.lastFrameSecCountTime) >= 1.00) { // Update Diagnostic Poll
        Sys_Global.lastFrameSecCountTime = Sys_Global.last_time;
        Sys_Global.framesPerLastSecond = Sys_Global.globalFrameNum - Sys_Global.lastFrameSecCount;
        if (Sys_Global.framesPerLastSecond < Sys_Global.worstFPS && Sys_Global.globalFrameNum > 2000) Sys_Global.worstFPS = Sys_Global.framesPerLastSecond; // After startup, keep track of worst framerate seen.
        Sys_Global.lastFrameSecCount = Sys_Global.globalFrameNum;
    }
}

i32 main(void) {
    double game_start_time = get_time();
    random_range_rng = (u32)game_start_time; // Seed global rand uniquely with time since system boot.
    console_log_file = OS_OpenWriteonly("./voxen.log"); // Initialize log system for all prints to go to both stdout and voxen.log file
    DebugRAM("program start");
    DualLog("Voxen, the Voxel Lit Open Source Game Engine by W. Josiah Jack, MIT-0 licensed\n");
    DualLog("Entity size: %u\n",sizeof(Entity));
    WindowInit();
    Sys_Global.globalFrameNum=0,Sys_Global.menuActive=true,Sys_Global.screenshotTimeout=1.0,Sys_Global.creditsPageIndex=1,Sys_Global.difficultyCombat=Sys_Global.difficultyCyber=Sys_Global.difficultyPuzzle=Sys_Global.difficultyMission=2,Sys_Global.deaths=0,Sys_Global.worstFPS=0,Sys_Global.cursorPosition_x=680,Sys_Global.cursorPosition_y=384;
    DualLog("Loading game definition...");
    OsFileHandle gmFP = OS_OpenReadonly("./Data/gamedata.txt");
    if (!gmFP) { DualLogError("\nCannot open ./Data/gamedata.txt\n"); OS_Exit(1);  }
    { // [BLOCK] Initialization (wrapped to free temporaries from stack)
        char gmLine[512],global_modname[256],global_dllname[256]; u32 lineNum = 0;
        while (GetNextStringUpToNewlineOrEOF(gmLine,sizeof(gmLine),gmFP)) {
            lineNum++;
            char* s = data_parser_trim(gmLine); if (*s == 0 || (s[0] == '/' && s[1] == '/')) continue;
            char* colon = StringFindFirstCharWithin(s, ':'); if (!colon) continue;
            *colon = '\0'; char* key = data_parser_trim(s); char* val = data_parser_trim(colon + 1); if (*key == 0 || *val == 0) continue;

            if (StringsEqual(key, "modname")) StringCopyInto_A_From_B(global_modname,val,sizeof(global_modname));
            else if (StringsEqual(key, "dllname")) StringCopyInto_A_From_B(global_dllname,val,sizeof(global_dllname));
            else if (StringsEqual(key, "windowicon")) StringCopyInto_A_From_B(Sys_Global.global_winicon,val,sizeof(Sys_Global.global_winicon));
            else if (StringsEqual(key, "levelcount")) Sys_Global.numLevels = parse_numberu8(val,gmLine,lineNum);
            else if (StringsEqual(key, "startlevel")) Sys_Global.startLevel = parse_numberu8(val,gmLine,lineNum);
        }
        
        OS_Close(gmFP); DualLog(" %s:: num levels: %d, start level: %d\n",global_modname,Sys_Global.numLevels,Sys_Global.startLevel);
        LoadConfig(); // Get settings before setting window size.
        window = glfwCreateWindow(Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight,&global_modname[0]);
        CenterWindowOnMonitor();
        _GLFWwindow* handle = (_GLFWwindow*)window; handle->context.makeCurrent(handle);
        glClear = (PFNGLCLEARPROC)glfwGetProcAddress("glClear");
        glClearColor = (PFNGLCLEARCOLORPROC)glfwGetProcAddress("glClearColor");
        glColorMask = (PFNGLCOLORMASKPROC)glfwGetProcAddress("glColorMask");
        glDepthFunc = (PFNGLDEPTHFUNCPROC)glfwGetProcAddress("glDepthFunc");
        glDepthMask = (PFNGLDEPTHMASKPROC)glfwGetProcAddress("glDepthMask");
        glDisable = (PFNGLDISABLEPROC)glfwGetProcAddress("glDisable");
        glEnable = (PFNGLENABLEPROC)glfwGetProcAddress("glEnable");
        glFinish = (PFNGLFINISHPROC)glfwGetProcAddress("glFinish");
        glFlush = (PFNGLFLUSHPROC)glfwGetProcAddress("glFlush");
        glFrontFace = (PFNGLFRONTFACEPROC)glfwGetProcAddress("glFrontFace");
        glGetError = (PFNGLGETERRORPROC)glfwGetProcAddress("glGetError");
        glGetIntegerv = (PFNGLGETINTEGERVPROC)glfwGetProcAddress("glGetIntegerv");
        glLineWidth = (PFNGLLINEWIDTHPROC)glfwGetProcAddress("glLineWidth");
        glReadBuffer = (PFNGLREADBUFFERPROC)glfwGetProcAddress("glReadBuffer");
        glReadPixels = (PFNGLREADPIXELSPROC)glfwGetProcAddress("glReadPixels");
        glTexImage2D = (PFNGLTEXIMAGE2DPROC)glfwGetProcAddress("glTexImage2D");
        glTexParameteri = (PFNGLTEXPARAMETERIPROC)glfwGetProcAddress("glTexParameteri");
        glViewport = (PFNGLVIEWPORTPROC)glfwGetProcAddress("glViewport");
        glBindTexture = (PFNGLBINDTEXTUREPROC)glfwGetProcAddress("glBindTexture");
        glCopyTexSubImage2D = (PFNGLCOPYTEXSUBIMAGE2DPROC)glfwGetProcAddress("glCopyTexSubImage2D");
        glDrawArrays = (PFNGLDRAWARRAYSPROC)glfwGetProcAddress("glDrawArrays");
        glDrawElements = (PFNGLDRAWELEMENTSPROC)glfwGetProcAddress("glDrawElements");
        glGenTextures = (PFNGLGENTEXTURESPROC)glfwGetProcAddress("glGenTextures");
        glActiveTexture = (PFNGLACTIVETEXTUREPROC)glfwGetProcAddress("glActiveTexture");
        glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC)glfwGetProcAddress("glBlendFuncSeparate");
        glBindBuffer = (PFNGLBINDBUFFERPROC)glfwGetProcAddress("glBindBuffer");
        glBufferData = (PFNGLBUFFERDATAPROC)glfwGetProcAddress("glBufferData");
        glGenBuffers = (PFNGLGENBUFFERSPROC)glfwGetProcAddress("glGenBuffers");
        glUnmapBuffer = (PFNGLUNMAPBUFFERPROC)glfwGetProcAddress("glUnmapBuffer");
        glAttachShader = (PFNGLATTACHSHADERPROC)glfwGetProcAddress("glAttachShader");
        glCompileShader = (PFNGLCOMPILESHADERPROC)glfwGetProcAddress("glCompileShader");
        glCreateProgram = (PFNGLCREATEPROGRAMPROC)glfwGetProcAddress("glCreateProgram");
        glCreateShader = (PFNGLCREATESHADERPROC)glfwGetProcAddress("glCreateShader");
        glDrawBuffers = (PFNGLDRAWBUFFERSPROC)glfwGetProcAddress("glDrawBuffers");
        glGetProgramiv = (PFNGLGETPROGRAMIVPROC)glfwGetProcAddress("glGetProgramiv");
        glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)glfwGetProcAddress("glGetShaderInfoLog");
        glGetShaderiv = (PFNGLGETSHADERIVPROC)glfwGetProcAddress("glGetShaderiv");
        glLinkProgram = (PFNGLLINKPROGRAMPROC)glfwGetProcAddress("glLinkProgram");
        glShaderSource = (PFNGLSHADERSOURCEPROC)glfwGetProcAddress("glShaderSource");
        glUniform1f = (PFNGLUNIFORM1FPROC)glfwGetProcAddress("glUniform1f");
        glUniform1i = (PFNGLUNIFORM1IPROC)glfwGetProcAddress("glUniform1i");
        glUniform2f = (PFNGLUNIFORM2FPROC)glfwGetProcAddress("glUniform2f");
        glUniform3f = (PFNGLUNIFORM3FPROC)glfwGetProcAddress("glUniform3f");
        glUniform4f = (PFNGLUNIFORM4FPROC)glfwGetProcAddress("glUniform4f");
        glUniform1ui = (PFNGLUNIFORM1UIPROC)glfwGetProcAddress("glUniform1ui");
        glUniform2ui = (PFNGLUNIFORM2UIPROC)glfwGetProcAddress("glUniform2ui");
        glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC)glfwGetProcAddress("glUniformMatrix3fv");
        glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)glfwGetProcAddress("glUniformMatrix4fv");
        glUseProgram = (PFNGLUSEPROGRAMPROC)glfwGetProcAddress("glUseProgram");
        glBindBufferBase = (PFNGLBINDBUFFERBASEPROC)glfwGetProcAddress("glBindBufferBase");
        glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)glfwGetProcAddress("glBindFramebuffer");
        glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)glfwGetProcAddress("glBindVertexArray");
        glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)glfwGetProcAddress("glCheckFramebufferStatus");
        glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)glfwGetProcAddress("glFramebufferTexture2D");
        glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)glfwGetProcAddress("glGenFramebuffers");
        glMapBufferRange = (PFNGLMAPBUFFERRANGEPROC)glfwGetProcAddress("glMapBufferRange");
        glBindImageTexture = (PFNGLBINDIMAGETEXTUREPROC)glfwGetProcAddress("glBindImageTexture");
        glBindVertexBuffer = (PFNGLBINDVERTEXBUFFERPROC)glfwGetProcAddress("glBindVertexBuffer");
        glDispatchCompute = (PFNGLDISPATCHCOMPUTEPROC)glfwGetProcAddress("glDispatchCompute");
        glBindTextureUnit = (PFNGLBINDTEXTUREUNITPROC)glfwGetProcAddress("glBindTextureUnit");
        glCreateBuffers = (PFNGLCREATEBUFFERSPROC)glfwGetProcAddress("glCreateBuffers");
        glCreateTextures = (PFNGLCREATETEXTURESPROC)glfwGetProcAddress("glCreateTextures");
        glCreateVertexArrays = (PFNGLCREATEVERTEXARRAYSPROC)glfwGetProcAddress("glCreateVertexArrays");
        glEnableVertexArrayAttrib = (PFNGLENABLEVERTEXARRAYATTRIBPROC)glfwGetProcAddress("glEnableVertexArrayAttrib");
        glNamedBufferData = (PFNGLNAMEDBUFFERDATAPROC)glfwGetProcAddress("glNamedBufferData");
        glNamedBufferStorage = (PFNGLNAMEDBUFFERSTORAGEPROC)glfwGetProcAddress("glNamedBufferStorage");
        glNamedBufferSubData = (PFNGLNAMEDBUFFERSUBDATAPROC)glfwGetProcAddress("glNamedBufferSubData");
        glTextureParameteri = (PFNGLTEXTUREPARAMETERIPROC)glfwGetProcAddress("glTextureParameteri");
        glTextureStorage2D = (PFNGLTEXTURESTORAGE2DPROC)glfwGetProcAddress("glTextureStorage2D");
        glTextureSubImage2D = (PFNGLTEXTURESUBIMAGE2DPROC)glfwGetProcAddress("glTextureSubImage2D");
        glVertexArrayAttribBinding = (PFNGLVERTEXARRAYATTRIBBINDINGPROC)glfwGetProcAddress("glVertexArrayAttribBinding");
        glVertexArrayAttribFormat = (PFNGLVERTEXARRAYATTRIBFORMATPROC)glfwGetProcAddress("glVertexArrayAttribFormat");
        glVertexArrayVertexBuffer = (PFNGLVERTEXARRAYVERTEXBUFFERPROC)glfwGetProcAddress("glVertexArrayVertexBuffer");
        glClearBufferFv = (PFNGLCLEARBUFFERFVPROC)glfwGetProcAddress("glClearBufferFv");
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); glfwSwapBuffers(); // Black out the window as early as possible for better presentation.
        i32 major=0,minor=0; glGetIntegerv(0x821B/*GL_MAJOR_VERSION*/,&major); glGetIntegerv(0x821C/*GL_MINOR_VERSION*/,&minor);
        if (major < 4 || (major == 4 && minor < 3)) { DualLogError("Need OpenGL >= 4.3, got %d.%d\n",major,minor); OS_Exit(1); }
        glFrontFace(0x0901/*GL_CCW*/); // Set triangle winding order
        glBlendFuncSeparate(0x0302/*GL_SRC_ALPHA*/,0x0303/*GL_ONE_MINUS_SRC_ALPHA*/,0,1);
        CompileShaders();
        u32 vaos[4],vbos[4]; glCreateVertexArrays(4,vaos); glCreateBuffers(3,vbos);
        Sys_Render.quadVAO = vaos[0]; Sys_Render.vao_chunk = vaos[1]; Sys_Render.textVAO = vaos[2]; Sys_Render.debugLinesVAO = vaos[3];
        Sys_Render.quadVBO = vbos[0]; Sys_Render.textVBO = vbos[1]; Sys_Render.debugLinesVBO = vbos[2];
        float quadBlit_vertices[] = {1.0f,-1.0f,1.0f,0.0f, 1.0f,1.0f,1.0f,1.0f, -1.0f,1.0f,0.0f,1.0f, -1.0f,-1.0f,0.0f,0.0f}; // 4 verts, 4 floats each x,y,u,v
        glNamedBufferData(Sys_Render.quadVBO,sizeof(quadBlit_vertices),quadBlit_vertices,GL_STATIC_DRAW);
        glVertexArrayAttribFormat(Sys_Render.quadVAO,0,2,GL_FLOAT,GL_FALSE,0); // DSA: Set position format
        glVertexArrayAttribFormat(Sys_Render.quadVAO,1,2,GL_FLOAT,GL_FALSE,2 * sizeof(float)); // DSA: Set texcoord format
        glVertexArrayVertexBuffer(Sys_Render.quadVAO,0,Sys_Render.quadVBO,0,4 * sizeof(float)); // DSA: Link VBO to VAO
        for (u8 i = 0; i < 2; i++) { glVertexArrayAttribBinding(Sys_Render.quadVAO,i,0); glEnableVertexArrayAttrib(Sys_Render.quadVAO,i); }
        glVertexArrayAttribFormat(Sys_Render.vao_chunk,0,3,0x140B/*GL_HALF_FLOAT*/,GL_FALSE,0);      // pos xyz half-float @ offset 0
        glVertexArrayAttribFormat(Sys_Render.vao_chunk,1,3,0x140B/*GL_HALF_FLOAT*/,GL_FALSE,6);      // normal xyz float   @ offset 6  (after 3×2 bytes)
        glVertexArrayAttribFormat(Sys_Render.vao_chunk,2,2,0x140B/*GL_HALF_FLOAT*/,GL_FALSE,12);     // uv st float
        for (u8 i = 0; i < 3; i++) { glVertexArrayAttribBinding(Sys_Render.vao_chunk,i,0); glEnableVertexArrayAttrib(Sys_Render.vao_chunk,i); }
        glVertexArrayAttribFormat(Sys_Render.textVAO,0,3,GL_FLOAT,GL_FALSE,0);             // pos (x,y,z) 4 floats per vertex, stride = 4*sizeof(float)
        glVertexArrayAttribFormat(Sys_Render.textVAO,1,2,GL_FLOAT,GL_FALSE,3 * sizeof(float));  // uv (s,t)
        glVertexArrayVertexBuffer(Sys_Render.textVAO,0,Sys_Render.textVBO,0,5 * sizeof(float));
        for (u8 i = 0; i < 2; i++) { glVertexArrayAttribBinding(Sys_Render.textVAO,i,0); glEnableVertexArrayAttrib(Sys_Render.textVAO,i); }
        InitFontAtlasses();
        GenerateAndBindTexture(&Sys_Render.inputUIID,GL_RGBA8,1366,768,GL_RGBA,GL_UNSIGNED_BYTE,0x2600/*GL_NEAREST*/,NULL); // UI Fixed Size Raster
        glGenFramebuffers(1,&Sys_Render.uiFBO);
        glBindFramebuffer(GL_FRAMEBUFFER,Sys_Render.uiFBO);
        glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D,Sys_Render.inputUIID);
        glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,Sys_Render.inputUIID,0);
        u32 drawBuffersUI[] = {GL_COLOR_ATTACHMENT0}; glDrawBuffers(1,drawBuffersUI);
        u32 uistatus = glCheckFramebufferStatus(GL_FRAMEBUFFER); if (uistatus != 0x8CD5/*GL_FRAMEBUFFER_COMPLETE*/) DualLogError("UI Framebuffer incomplete: Error code %d\n",uistatus);
        glBindImageTexture(0,Sys_Render.inputUIID,0,GL_FALSE,0,GL_READ_WRITE,GL_RGBA8); // UI Rendered Color
        glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,Sys_Render.inputUIID,0);
        RenderLoadingProgress(22,"Loading...");
        glNamedBufferStorage(Sys_Render.debugLinesVBO,MAX_DEBUG_LINE_VERTS * 3 * sizeof(float),NULL,0x0100/*GL_DYNAMIC_STORAGE_BIT*/);
        glVertexArrayAttribFormat(Sys_Render.debugLinesVAO,0,3,GL_FLOAT,GL_FALSE,0);
        glEnableVertexArrayAttrib(Sys_Render.debugLinesVAO,0);
        glVertexArrayAttribBinding(Sys_Render.debugLinesVAO,0,0);
        glVertexArrayVertexBuffer(Sys_Render.debugLinesVAO,0,Sys_Render.debugLinesVBO,0,3 * sizeof(float));
        float* m = shadowmapsPerspectiveProjection; float lightRangeMax=15.36f; float viewRange=(lightRangeMax - NEAR_PLANE);
        m[0] = 1.0f; m[1] = 0.0f; m[2] =                                           0.0f; m[3] =  0.0f;
        m[4] = 0.0f; m[5] = 1.0f; m[6] =                                           0.0f; m[7] =  0.0f;
        m[8] = 0.0f; m[9] = 0.0f; m[10]=      -(lightRangeMax + NEAR_PLANE) / viewRange; m[11]= -1.0f;
        m[12]= 0.0f; m[13]= 0.0f; m[14]= -2.0f * lightRangeMax * NEAR_PLANE / viewRange; m[15]=  0.0f;
        InitAudio();
        DualLog("Loading mod code...");
        char mod_path[256];
        StringCopyInto_A_From_B(mod_path,"./",256); StringConcatenate(mod_path,global_dllname,256); StringConcatenate(mod_path,MOD_EXTENSION,256);
        mod_handle = PLATFORM_DLOPEN(mod_path);
        if (!mod_handle) { DualLogError("Failed to load mod at:%s",mod_path); OS_Exit(1); }
        
        #define X(ret, name, params) \
            name = (ret (*) params)PLATFORM_DLSYM(mod_handle, #name); \
            if(!name) DualLogError("Failed to load mod function: %s", #name);
        MOD_FUNCTION_LIST(X)
        #undef X
        ModLink(&Sys_Global,&Sys_Cheats,&Sys_Settings,&Sys_Text,&Sys_UI); // Link engine to mod
        Sys_Global.GetKey = GetKey; Sys_Global.GetKeyPressed = GetKeyPressed; // Link mod to engine
        DualLog("done!\n");
        ModEntityDefinitionsInitAfterLoad();
        glGenFramebuffers(1,&Sys_Render.gBufferFBO);
        ApplySettings(); // After loading of text and game data.
        glBindFramebuffer(GL_FRAMEBUFFER,Sys_Render.gBufferFBO);
        u32 drawBuffers[] = {GL_COLOR_ATTACHMENT0,GL_COLOR_ATTACHMENT1,GL_COLOR_ATTACHMENT2,GL_COLOR_ATTACHMENT3};
        glDrawBuffers(4,drawBuffers);
        u32 status = glCheckFramebufferStatus(GL_FRAMEBUFFER); if (status != 0x8CD5/*GL_FRAMEBUFFER_COMPLETE*/) DualLogError("Framebuffer incomplete: Error code %d\n",status);
        glBindFramebuffer(GL_FRAMEBUFFER,0);
        float mat[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
        CopyMemoryFromBtoAForNBytes(&modelMatrices[0],mat,16 * sizeof(float)); // Null instance matrix used for UI
        Sys_Render.matricesBufferID        = SetupSSBO(&Sys_Render.matricesBufferID,        1,INSTANCE_COUNT * 16 * sizeof(float),modelMatrices, GL_STATIC_DRAW);
        Sys_Render.voxelLightListCountsID  = SetupSSBO(&Sys_Render.voxelLightListCountsID,  2,VOXEL_COUNT * sizeof(u32),NULL,GL_STATIC_DRAW);
        Sys_Render.voxelLightListsID       = SetupSSBO(&Sys_Render.voxelLightListsID,       3,VOXEL_COUNT * MAX_LIGHTS_PER_VOXEL * sizeof(u32),NULL,GL_STATIC_DRAW);
        Sys_Render.lightsID                = SetupSSBO(&Sys_Render.lightsID,                4,LIGHT_COUNT * sizeof(Light),NULL,GL_STATIC_DRAW);
        Sys_Render.shadowMapSSBO           = SetupSSBO(&Sys_Render.shadowMapSSBO,           5,(MAX_SHADOWMAPS * (SHADOW_MAP_SIZE * SHADOW_MAP_SIZE * 6U)) * sizeof(u32), NULL, GL_STATIC_DRAW);    
        Sys_Render.shadowMapsIndirectionID = SetupSSBO(&Sys_Render.shadowMapsIndirectionID, 6,LIGHT_COUNT * sizeof(u32),NULL,GL_STATIC_DRAW);
        Sys_Render.cellVisibleDataID       = SetupSSBO(&Sys_Render.cellVisibleDataID,       7,ARRSIZE * sizeof(u32),NULL,GL_STATIC_DRAW);
        Sys_Render.colorBufferID           = SetupSSBO(&Sys_Render.colorBufferID,          12,MAX_TOTAL_PIXELS * sizeof(u8),NULL,GL_STATIC_DRAW);
        Sys_Render.textureOffsetsID        = SetupSSBO(&Sys_Render.textureOffsetsID,       14,MAX_VALID_TEXTURE * sizeof(u32),NULL,GL_STATIC_DRAW);
        Sys_Render.textureSizesID          = SetupSSBO(&Sys_Render.textureSizesID,         15,MAX_VALID_TEXTURE * 2 * sizeof(i32),NULL, GL_STATIC_DRAW);
        Sys_Render.texturePalettesID       = SetupSSBO(&Sys_Render.texturePalettesID,      16,MAX_UNIQUE_COLORS * sizeof(u32),NULL,GL_STATIC_DRAW);
        Sys_Render.texturePaletteOffsetsID = SetupSSBO(&Sys_Render.texturePaletteOffsetsID,17,MAX_VALID_TEXTURE * sizeof(u32),NULL,GL_STATIC_DRAW);
        glUseProgram(Sys_Render.shadowmapsShaderProgram); glUniform1ui(9,SHADOW_MAP_SIZE);
        glUseProgram(Sys_Render.shadowmapsClearShaderProgram); glUniform1ui(1,SHADOW_MAP_SIZE);
        glUseProgram(Sys_Render.chunkShaderProgram); glUniform1ui(21,SHADOW_MAP_SIZE); glUniform1f(22,(float)SHADOW_MAP_SIZE); glUniform1ui(23,LIGHT_COUNT); glUniform1ui(24,(u32)MAX_LIGHTS_PER_VOXEL); glUniform1ui(11,SHADOW_MAP_SIZE*SHADOW_MAP_SIZE);
        glUseProgram(Sys_Render.voxelUpdateShaderProgram); glUniform1ui(6,(u32)MAX_LIGHTS_PER_VOXEL); glUniform1ui(8,SHADOW_MAP_SIZE); glUniform1f(9,(float)SHADOW_MAP_SIZE); glUniform1ui(10,SHADOW_MAP_SIZE*SHADOW_MAP_SIZE); glUniform1ui(11,LIGHT_COUNT);
        RenderLoadingProgress(52,"Loading textures...");
        LoadTextures();
        RenderLoadingProgress(38,"Loading models...");
        LoadModels();
        if (Sys_Global.introNotPlayed) {} // TODO: Play intro
        Sys_Global.absoluteTime = Sys_Global.last_topframe_time = Sys_Global.current_time = get_time();
        Sys_Global.pauseRelativeTime = Sys_Global.last_physics_time = 0.0;
    //     NewGame(); // Almost works, just causes GL errors once entering game and SSR doesn't appear to work.  Needed to fix bug where you can't see options take effect on config menu unless returned to from after starting a game.
        OpenMainMenu();
        DebugRAM("InitializeEnvironment end");
        DualLog("Game Initialized in %f secs\n",get_time() - game_start_time);
    }
    
    while(1) { // Main Loop
        if (((_GLFWwindow*)window)->shouldClose) OS_Exit(0);
        if (queuedLevelToLoad != 255u) { LoadLevel(queuedLevelToLoad); queuedLevelToLoad = 255u; continue; }

        drawCallsRenderedThisFrame = uiImageDrawCallsRenderedThisFrame = shadowDrawCallsRenderedThisFrame = verticesRenderedThisFrame = 0; // Reset per frame
        Sys_Global.current_time = get_time(); // Update Time
        Sys_Global.deltaTime = Sys_Global.current_time - Sys_Global.last_topframe_time;
        Sys_Global.absoluteTime += Sys_Global.deltaTime;
        Sys_Global.last_topframe_time = Sys_Global.current_time;
        if (!Sys_Global.gamePaused && !Sys_Global.menuActive) Sys_Global.pauseRelativeTime += Sys_Global.deltaTime;
        mouseMovementThisFrame = false;
        Input_Poll();
        if (Sys_Input.keyStates[GLFW_KEY_E].pressed) play_wav("./Audio/cyborgs/yourlevelsareterrible.wav",0.1f,(Vector3){},false);
        if (Sys_Input.window_has_focus) {
            if (Sys_Input.keyStates[GLFW_KEY_CAPS_LOCK].pressed) Sys_Input.isCapsLockOn = !Sys_Input.isCapsLockOn; // Change capslock state to match keyboard having toggled.  Must always happen regardless of paused/menu.
            ProcessInput(); // Calls ApplyPlayerMovements(), needs called without checking paused state for menus handling.
        }
        
        Sys_Global.timeSinceLastPhysicsTick = Sys_Global.pauseRelativeTime - Sys_Global.last_physics_time;
        if (likely(!Sys_Global.gamePaused || Sys_Global.menuActive)) UpdateAnims(); // Changes collision positions
        if (likely(!Sys_Global.gamePaused && !Sys_Global.menuActive)) { // Update Gameplay
            if (Sys_Global.timeSinceLastPhysicsTick > (1.0 / 144.0)) { Sys_Global.last_physics_time = Sys_Global.pauseRelativeTime; Physics(); }
            
            Vector3 pDelta = Vector3_A_minus_B(Sys_Global.instances[PLAYER1].lastPosition,Sys_Global.instances[PLAYER1].position);
            bool playerMoved = ((vabs(pDelta.x) + vabs(pDelta.y) + vabs(pDelta.z)) > 0.02f);
            ModUpdate(playerMoved);
            UpdateAmbientSounds();
        }

        UpdateMusic();
        if (likely(!Sys_Global.gamePaused) && camViewCount > 0) { // Render in-world camera views.  Pops player elsewhere, renders to tiny fbo, pops player back, renders as normal below.
            Vector3 tempPlayerPos = Sys_Global.instances[PLAYER1].position;
            Quaternion tempPlayerRot = Sys_Global.instances[PLAYER1].rotation;
            for (int cm=0;cm<camViewCount;++cm) {
                if (camViews[cm].finished < Sys_Global.pauseRelativeTime && camViews[cm].visible) {
                    camViews[cm].finished = Sys_Global.pauseRelativeTime + 0.5f;
                    Sys_Global.instances[PLAYER1].position = camViews[cm].position;
                    Sys_Global.instances[PLAYER1].rotation = camViews[cm].rotation;
                    CullCore();
                    Render(true,cm); // Ok culling and light clusters (in voxels) have been updated, now render the view.
                }
            }

            Sys_Global.instances[PLAYER1].position = tempPlayerPos; // Restore player for normal render.
            Sys_Global.instances[PLAYER1].rotation = tempPlayerRot;
        }
        
        if (likely(!Sys_Global.gamePaused || Sys_Global.menuActive)) {
            CullCore();
            bool uploadInstances = false;
            for (u32 i = START_INDEX_LEVEL_INSTANCES; i < Sys_Global.loadedInstances; i++) {
                if (Sys_Global.dirtyInstances[i]) {
                    Entity* e = &Sys_Global.instances[i]; u16 mdx = e->modelIndex;
                    if (mdx >= loadedModelsMaxIndex || modelVertexCounts[mdx] < 1) { Sys_Global.dirtyInstances[i] = false; continue; } // No model or empty model

                    uploadInstances = true;
                    float x=e->rotation.x, y = e->rotation.y, z = e->rotation.z, w = e->rotation.w;
                    float x2 = x*x, y2 = y*y, z2 = z*z, xy = x*y, xz = x*z, yz = y*z, wx = w*x, wy = w*y, wz = w*z;
                    float sclx = e->scale.x; float scly = e->scale.y; float sclz = e->scale.z;
                    u32 m = i*16;
                    modelMatrices[m+0] = (1.0f - 2.0f * (y2 + z2)) * sclx;/*Right X*/ modelMatrices[m+1] = (2.0f * (xy + wz)) * sclx;/*Right Y*/ modelMatrices[m+2] = (2.0f * (xz - wy)) * sclx;/*Right Z*/
                    modelMatrices[m+3] = modelMatrices[m + 7] = modelMatrices[m + 11] = 0.0f;
                    modelMatrices[m+4] = (2.0f * (xy - wz)) * scly;/*Up X*/      modelMatrices[m+5] = (1.0f - 2.0f * (x2 + z2)) * scly;/*Up Y*/      modelMatrices[m+6] =        (2.0f * (yz + wx)) * scly;/*Up Z*/
                    modelMatrices[m+8] = (2.0f * (xz + wy)) * sclz;/*Forward X*/ modelMatrices[m+9] =        (2.0f * (yz - wx)) * sclz;/*Forward Y*/ modelMatrices[m+10]= (1.0f - 2.0f * (x2 + y2)) * sclz;/*Forward Z*/
                    modelMatrices[m+12]= e->position.x; modelMatrices[m + 13] = e->position.y; modelMatrices[m + 14] = e->position.z;
                    modelMatrices[m+15]= 1.0f;
                }
            }
            if (uploadInstances) glNamedBufferData(Sys_Render.matricesBufferID,Sys_Global.loadedInstances * 16 * sizeof(float),modelMatrices,GL_DYNAMIC_DRAW);
        }

        AudioUpdate();
        Render(false,0u); // Not a cam view, no camview index.  This is the normal main render.
        CheckAndTakeScreenshot();
        Sys_Global.globalFrameNum++;
        InputClearRisingAndFallingEdges();
        Sys_Input.currentMouse_dx = Sys_Input.currentMouse_dy = 0;
        Sys_Global.cpuTime = get_time() - Sys_Global.current_time; // Measure time over everything this frame before GPU swap buffers
        glfwSwapBuffers(); // Present frame
        CHECK_GL_ERROR();
        #ifdef DEBUG_RAM_OUTPUT
            static const u32 dbgFrames[] = {4,100,200,500,1000};
            static const char*    dbgLabels[] = {"after 4 frames","after 100 frames","after 200 frames","after 500 frames","after 1000 frames"};
            for (int _d=0;_d<5;_d++) if (Sys_Global.globalFrameNum == dbgFrames[_d]) { DebugRAM(dbgLabels[_d]); break; }
        #endif
    }
    return 0;
}
