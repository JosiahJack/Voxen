// voxen.c - A realtime OpenGL 4.3+ Game Engine for Citadel: The System Shock Fan Remake
#include "os.h" // Operating System calls shim layer.
#include "gl.h"
GLFWwindow* window;
#define MOD_INTEROP_ENGINE
#include "voxen.h"
#include "Shaders/shaders.h"
#include "credits.h"
#include "glfw.c"
GlobalContext Sys_Global = {.globalFrameNum=0,.menuActive=true,.screenshotTimeout=1.0,.creditsPageIndex=1,.difficultyCombat=2,.difficultyCyber=2,.difficultyPuzzle=2,.difficultyMission=2,.deaths=0,.worstFPS=0,.cursorPosition_x=680,.cursorPosition_y=384};
CheatsSystem Sys_Cheats = {.god=false,.noclip=true,.showLocation=true,.showFPS=true,.editMode=true}; RenderSystem Sys_Render; SystemUI Sys_UI;
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
    .InvertCyberspaceLook=0u,.QuickItemPickup=0u,.QuickReloadWeapons=0u,.MouseSensitivity=10u,.NoShootMode=0u,.HeadBob=1u,.SSR_RES=8u};/*Ratio is (1 / SSR_RES) * res*/
#define SHADOW_MAP_SIZE 128u
u8 queuedLevelToLoad = 255u;
float berserkSeedTime,cam_pitch,cam_yaw=90.0f,cam_roll,rasterPerspectiveProjection[16],shadowmapsPerspectiveProjection[16],modelMatrices[INSTANCE_COUNT*16];
char uiTextBuffer[TEXT_BUFFER_SIZE];
float uiOrthoProjection[16];
Light lights[LIGHT_COUNT]; LightAnimation lanims[LIGHT_COUNT];
static float lightView[LIGHT_COUNT][6][4][4],lightViewProj[LIGHT_COUNT][6][16];
FrustumPlane lightFrustumPlanes[LIGHT_COUNT][6][6],playerFrustumPlanes[6];
u16 editModeSelection,editModeTestEntityDefinition=0; // Test instance and its model index
typedef struct { double shadowTime; u32 shadowmapIndirectionList[LIGHT_COUNT]; float shadDotThresh; } VoxenShadowSystem;
VoxenShadowSystem voxen_Shadow_System;
u16 loadedTexturesMaxIndex;
bool doubleSidedTexture[MAX_VALID_TEXTURE],transparentTexture[MAX_VALID_TEXTURE];
extern u32 gridCellStates[ARRSIZE],modelVertexCounts[MODEL_IDX_MAX]; extern u16 modelTriangleCounts[MODEL_IDX_MAX];
u32 drawCallsRenderedThisFrame,textDrawCallsRenderedThisFrame,uiImageDrawCallsRenderedThisFrame,shadowDrawCallsRenderedThisFrame,verticesRenderedThisFrame,drawCallsNormal;
extern GLuint fontAtlasTex,fontAtlasTexStopD;
#define MAX_CHANNELS 256
ma_engine audio_engine; ma_sound wav_sounds[MAX_CHANNELS];
float wav_volumes[MAX_CHANNELS]; // Setting independent base sfx volume (e.g. dropped physics object hard or lightly volume, independent of position).
i32 wav_count = 0; ma_sound log_sound;
#define MENUPAD        1028
#define MENUPAD_HILITE 1029
MenuPages currentMenuPage = MenuPages_FrontPage;
bool returnToPause=false,fovSliderActive=false,gammaSliderActive=false,masterVolumeSliderActive=false,musicVolumeSliderActive=false,messageVolumeSliderActive=false,sfxVolumeSliderActive=false,enteringPlayerName=false;
u8 currentPlayerNameLength = 0;
i8 currentMenuItem = 0, currentMenuTab = 0, menuItemCount = 4, menuTabCount = 1;
static bool resDropdownOpen = false;
static int resDropdownCount=0,resSelectedIdx=0;
typedef struct { int w, h, hz; } ResMode;
static ResMode resModes[8];
typedef struct { Vector3 position; Quaternion rotation; u8 fov; u16 width,height; float near,far,finished; bool visible; } CamView;
CamView camViews[64]; // Max is 8 camera views on level 8 + 3 sensaround views = 11.  Populated at level load.
GLuint camViewTextures[64];
u8 camViewCount = 0;
typedef struct { unsigned short x0,y0,x1,y1; float xoff,yoff,xadvance; float xoff2,yoff2; } stbtt_packedchar; // x0,y0,x1,y1 are coordinates of bbox in bitmap
typedef struct { float x0,y0,s0,t0; float x1,y1,s1,t1; } stbtt_aligned_quad; // 0 is top-left, 1 is bottom-right
void stbtt_GetPackedQuad(const stbtt_packedchar *chardata, int pw, int ph, int char_index, float *xpos, float *ypos, stbtt_aligned_quad *q, int align_to_integer); OsFileHandle console_log_file=0;
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
static inline __attribute__((always_inline)) void LogShaderError(GLuint s, const char* name) { char er[512]; glGetShaderInfoLog(s,512,NULL,er); DualLogError("%s Compilation Failed: %s\n",name,er); OS_Exit(1); }
static inline __attribute__((always_inline)) GLuint CompileShader(GLenum type, const char* source, const char* name) { GLuint s = glCreateShader(type); glShaderSource(s,1,&source,NULL); glCompileShader(s); GLint ok; glGetShaderiv(s,GL_COMPILE_STATUS,&ok); if (!ok) LogShaderError(s,name); return s; }
static inline __attribute__((always_inline)) GLuint LinkProgram(GLuint* s, i32 num, const char* name) { GLuint p = glCreateProgram(); for (i32 i=0;i<num;++i) { glAttachShader(p,s[i]); glDeleteShader(s[i]); } glLinkProgram(p); GLint ok; glGetProgramiv(p,GL_LINK_STATUS,&ok); if (!ok) LogShaderError(p,name); return p; }
GLuint CompileStandardShader(const char* vsrc, const char* fsrc, const char* name) { GLuint vertShader = CompileShader(GL_VERTEX_SHADER,vsrc,name); GLuint fragShader = CompileShader(GL_FRAGMENT_SHADER,fsrc,name); return LinkProgram((GLuint[]){vertShader,fragShader},2,name); }
GLuint CompileComputeShader(const char* src, const char* name) { GLuint computeShader = CompileShader(GL_COMPUTE_SHADER, src, name); return LinkProgram((GLuint[]){computeShader}, 1, name); }
void CompileShaders(void) {
    Sys_Render.depthPrepassShaderProgram= CompileStandardShader(depthPrepassVertSrc,depthPrepassFragSrc,"Depth Prepass");
    Sys_Render.chunkShaderProgram       = CompileStandardShader(vertSrc,fragSrc,"Main");
    Sys_Render.debugUnlitShaderProgram  = CompileStandardShader(debugUnlitVertSrc,debugUnlitFragSrc,"Debug Unlit");
    Sys_Render.shadowmapsShaderProgram  = CompileStandardShader(shadowmapVertSrc,shadowmapFragSrc,"Shadowmaps");
    Sys_Render.textShaderProgram        = CompileStandardShader(textVertSrc,textFragSrc,"Text");
    Sys_Render.imageBlitShaderProgram   = CompileStandardShader(quadVertSrc,quadFragSrc,"Image Blit");
    Sys_Render.ssrShaderProgram            = CompileComputeShader(ssrComputeSrc,"SSR");
    Sys_Render.voxelUpdateShaderProgram    = CompileComputeShader(voxelUpdateComputeSrc,"Voxel Update");
    Sys_Render.shadowmapsClearShaderProgram= CompileComputeShader(shadowmapsClearComputeSrc,"Shadowmaps Clear");
}

GLuint SetupSSBO(GLuint* id, GLuint bindx, GLsizeiptr sz, const void* d, GLenum typ) { glGenBuffers(1,id); glBindBuffer(GL_SSBO,*id); glBufferData(GL_SSBO,sz,d,typ); glBindBufferBase(GL_SHADER_STORAGE_BUFFER,bindx,*id); return *id; }

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
    entry->layer = PhysicsLayer_Default;
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
    entry->path[0] = '\0';    
}

Vector3 lightsNewPosition[LIGHT_COUNT];
ENGINE_TO_MOD i32 AddLight(Light* lit, LightAnimation* lanim) {
    i32 i = Sys_Global.loadedLights;
    Sys_Global.loadedLights++;
    if (Sys_Global.loadedLights >= LIGHT_COUNT) { DualLogError("Too many lights %u added in level %d!\n",i,Sys_Global.currentLevel); OS_Exit(1); }

    __builtin_memcpy(&lights[i],lit,sizeof(Light));
    __builtin_memcpy(&lanims[i],lanim,sizeof(LightAnimation));
    lightsNewPosition[i] = lit->pos;
    flag_setu32(&lights[i].lflags,LDIRTY,true);
    return i;
}

ENGINE_TO_MOD void TurnLightOff(u16 litIdx) {
    if (litIdx >= Sys_Global.loadedLights) return;
    
    flag_setu32(&lights[litIdx].lflags,LIGHTON,false);
}

bool alreadyReadLightOnOnce[LIGHT_COUNT] = {0};
ENGINE_TO_MOD void LoadFieldIntoLight(char* trimmed_key, char* trimmed_value, char* initialLine, u32 lineNum, Light* lit, LightAnimation* lam, u16 lightIdx) {
    char buffer[32];
    for (int i=0;i <32;++i) {
        StringFormat(buffer,sizeof(buffer),"intervalSteps[%d]",i);
        if (StringsEqual(trimmed_key,buffer)) { lam->intervalSteps[i] = parse_float(trimmed_value,initialLine,lineNum); return; }
    }
    
    for (int i=0;i <32;++i) {
        StringFormat(buffer,sizeof(buffer),"intervalStepisLerping[%d]",i);
        if (StringsEqual(trimmed_key,buffer)) { lam->stepIsLerping[i] = parse_float(trimmed_value,initialLine,lineNum); return; }
    }
         if (StringsEqual(trimmed_key,"currentStep"))                  lam->currentStep = parse_numberu8(trimmed_value,initialLine,lineNum);
    else if (StringsEqual(trimmed_key,"lerpValue"))                    lam->lerpValue = parse_float(trimmed_value,initialLine,lineNum);
    else if (StringsEqual(trimmed_key,"intervalSteps.Length"))         lam->numIntervalSteps = parse_numberu8(trimmed_value,initialLine,lineNum);
    else if (StringsEqual(trimmed_key,"intervalStepisLerping.Length")) lam->numLerpSteps = parse_numberu8(trimmed_value,initialLine,lineNum);
    
    else if (StringsEqual(trimmed_key,"localPosition.x")) lit->pos.x = parse_float(trimmed_value,initialLine,lineNum);
    else if (StringsEqual(trimmed_key,"localPosition.y")) lit->pos.y = parse_float(trimmed_value,initialLine,lineNum);
    else if (StringsEqual(trimmed_key,"localPosition.z")) lit->pos.z = parse_float(trimmed_value,initialLine,lineNum);
    else if (StringsEqual(trimmed_key,"localRotation.x")) lit->spotDir.x = parse_float(trimmed_value,initialLine,lineNum);
    else if (StringsEqual(trimmed_key,"localRotation.y")) lit->spotDir.y = parse_float(trimmed_value,initialLine,lineNum);
    else if (StringsEqual(trimmed_key,"localRotation.z")) lit->spotDir.z = parse_float(trimmed_value,initialLine,lineNum);
    else if (StringsEqual(trimmed_key,"localRotation.w")) lit->spotDir.w = parse_float(trimmed_value,initialLine,lineNum);
    else if (StringsEqual(trimmed_key,"intensity"))    lit->intensity = lit->maxIntensity = parse_float(trimmed_value,initialLine,lineNum) * 0.35f;
    else if (StringsEqual(trimmed_key,"range"))        lit->range = parse_float(trimmed_value, initialLine, lineNum);
    else if (StringsEqual(trimmed_key,"spotAngle"))    lit->spotAng = parse_float(trimmed_value, initialLine, lineNum);
    else if (StringsEqual(trimmed_key,"type")) {
             if (StringsEqual(trimmed_value,"Spot"))        flag_setu32(&lit->lflags,LSPOT,true);
        else if (StringsEqual(trimmed_value,"Directional")) flag_setu32(&lit->lflags,LDIR,true);
    }
    else if (StringsEqual(trimmed_key,"minIntensity")) lit->minIntensity = parse_float(trimmed_value,initialLine,lineNum);
    else if (StringsEqual(trimmed_key,"maxIntensity")) lit->maxIntensity = parse_float(trimmed_value,initialLine,lineNum);
    else if (StringsEqual(trimmed_key,"color.r"))      lit->col.r = parse_float(trimmed_value,initialLine,lineNum);
    else if (StringsEqual(trimmed_key,"color.g"))      lit->col.g = parse_float(trimmed_value,initialLine,lineNum);
    else if (StringsEqual(trimmed_key,"color.b"))      lit->col.b = parse_float(trimmed_value,initialLine,lineNum);
    else if (StringsEqual(trimmed_key,"lightOn")) { // This exact value is duplicated from the Unity TargetIO class, ugh.
        if (!alreadyReadLightOnOnce[lightIdx]) { alreadyReadLightOnOnce[lightIdx] = true; bool on = parse_bool(trimmed_value,initialLine,lineNum); flag_setu32(&lit->lflags,LIGHTON,on); }
    }
    else if (StringsEqual(trimmed_key,"lerpOn"))       flag_setu32(&lit->lflags,LERPON,parse_bool(trimmed_value,initialLine,lineNum));
}

static inline __attribute__((always_inline)) void mul_mat4(float *out, const float *a, const float *b) { // out = a * b
	out[0] =  a[0] * b[0]  + a[4] * b[1]  + a[8]  * b[2] + a[12]  * b[3];
	out[1] =  a[1] * b[0]  + a[5] * b[1]  + a[9]  * b[2] + a[13]  * b[3];
	out[2] =  a[2] * b[0]  + a[6] * b[1] + a[10]  * b[2] + a[14]  * b[3];
	out[3] =  a[3] * b[0]  + a[7] * b[1] + a[11]  * b[2] + a[15]  * b[3];
	out[4] =  a[0] * b[4]  + a[4] * b[5]  + a[8]  * b[6] + a[12]  * b[7];
	out[5] =  a[1] * b[4]  + a[5] * b[5]  + a[9]  * b[6] + a[13]  * b[7];
	out[6] =  a[2] * b[4]  + a[6] * b[5] + a[10]  * b[6] + a[14]  * b[7];
	out[7] =  a[3] * b[4]  + a[7] * b[5] + a[11]  * b[6] + a[15]  * b[7];
	out[8] =  a[0] * b[8]  + a[4] * b[9]  + a[8] * b[10] + a[12] * b[11];
	out[9] =  a[1] * b[8]  + a[5] * b[9]  + a[9] * b[10] + a[13] * b[11];
	out[10] = a[2] * b[8]  + a[6] * b[9] + a[10] * b[10] + a[14] * b[11];
	out[11] = a[3] * b[8]  + a[7] * b[9] + a[11] * b[10] + a[15] * b[11];
	out[12] = a[0] * b[12] + a[4] * b[13] + a[8] * b[14] + a[12] * b[15];
	out[13] = a[1] * b[12] + a[5] * b[13] + a[9] * b[14] + a[13] * b[15];
	out[14] = a[2] * b[12] + a[6] * b[13] + a[10]* b[14] + a[14] * b[15];
	out[15] = a[3] * b[12] + a[7] * b[13] + a[11]* b[14] + a[15] * b[15];
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
    Vector3 p = Sys_Global.instances[PLAYER1].position;
    glUseProgram(Sys_Render.voxelUpdateShaderProgram);
    glUniform3f(5,p.x,p.y,p.z);
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
// ============================================================================
// UI Rendering and Text
#define BASE_RES_X 1366.0f // Positions done in fixed int positions off base resolution, scaled against current resolution.
#define BASE_RES_Y 768.0f
float UIX(i16 x) { return (float)x / BASE_RES_X; } // Pos or value as percent of 1366x768 resolution
float RelX(i16 x) { return UIX(x) * (float)Sys_Settings.ScreenWidth; } // Pos or value in current resolution
float UIY(i16 y) { return (float)y / BASE_RES_Y; }
float RelY(i16 y) { return UIY(y) * (float)Sys_Settings.ScreenHeight; }

void RenderUIImage(i16 x, i16 y, i16 width, i16 height, u32 texIndex) {
    float xpos = RelX(x); float ypos = RelY(y);
    glUseProgram(Sys_Render.chunkShaderProgram);
    glBindVertexArray(Sys_Render.textVAO);
    glUniform1ui(1,0);
    glUniform1ui(3,1u);  // isUI true
    glUniform1ui(17,1u); // unlit is true
    glUniform1ui(19,0);
    glUniform1ui(20,0);
    glUniformMatrix4fv(2,1,GL_FALSE,uiOrthoProjection);
    glBindBuffer(GL_ARRAY_BUFFER,Sys_Render.textVBO);
    float x1 = xpos + RelX(width);
    float y1 = ypos + RelY(height);
    float z = 0.0f;
    float vertices[30] = {xpos,y1,z,0.0f,0.0f,x1,ypos,z,1.0f,1.0f,x1,y1,z,1.0f,0.0f,xpos,y1,z,0.0f,0.0f,xpos,ypos,z,0.0f,1.0f,x1,ypos,z,1.0f,1.0f};
    glUniform1ui(18,texIndex);    
    glBufferData(GL_ARRAY_BUFFER,30 * sizeof(float),vertices,GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES,0,6);
    drawCallsRenderedThisFrame++; uiImageDrawCallsRenderedThisFrame++; verticesRenderedThisFrame += 6;    
    glBindBuffer(GL_ARRAY_BUFFER,0);
}

__attribute__((pure)) bool CursorIsOverBounds(float startX, float endX, float startY, float endY) {
    return   (Sys_Global.cursorPosition_x >= startX && Sys_Global.cursorPosition_x <= endX   /* 0 == left */
           && Sys_Global.cursorPosition_y >= endY && Sys_Global.cursorPosition_y <= startY); /* 0 == top */
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

float textVertexData[8192]; // Reusable buffer for text vertices.  Most text only needs ~3000
extern stbtt_packedchar fontPackedChar[MAX_GLYPHS]; extern stbtt_packedchar fontPackedCharStopD[MAX_GLYPHS];
extern float fixedNumberAdvanceWidth; extern float fixedNumberAdvanceWidthStopD;
__attribute__((pure)) i32 CodepointToPackedIndex(i32 codepoint, int fontID);
void RenderFormattedText(i16 x, i16 y, u32 color, u8 fontID, float scaleInput, const char * restrict format, ...) {
    float scale = scaleInput;// * UIY(Sys_Settings.ScreenHeight);
    va_list args;
    __builtin_va_start(args, format); StringFormatV(uiTextBuffer,TEXT_BUFFER_SIZE,format,args); __builtin_va_end(args);
    glUseProgram(Sys_Render.textShaderProgram);
    glUniformMatrix4fv(0, 1, GL_FALSE, uiOrthoProjection);
    glUniform4f(3,textColors[color].r,textColors[color].g,textColors[color].b,textColors[color].a);
    if (fontID == FONT_STOPD) glBindTextureUnit(6,fontAtlasTexStopD);
    else glBindTextureUnit(6,fontAtlasTex);
    
    glUniform2f(4,1.0f / (float)FONT_ATLAS_SIZE, 1.0f / (float)FONT_ATLAS_SIZE);
    glUniform1ui(2,fontID);
    glUniform1i(1,6); // textTexture sampler2D
    glBindVertexArray(Sys_Render.textVAO);
    size_t vertexCount = 0;
    const char* p = uiTextBuffer;
    float xpos = RelX(x), ypos = RelY(y) + (RelY(16) * scale);
    float lineSpacing = RelY(22) * scale;
    stbtt_aligned_quad q;
    int characterCount = 0;
    float paddingUV = (10.0f / (float)FONT_ATLAS_SIZE); // This is for the black outline around all text for readability. (2.0f makes an interesting bold effect)
    float borderWidthPixels = 2.0f;
    while (*p) { // Decode UTF8
        const unsigned char *s = (const unsigned char *)p;
        u32 codepoint = 0;
        if (*s < 0x80) { // 1-byte ASCII
            codepoint = *s++;
        } else if ((*s & 0xE0) == 0xC0) { // 2-byte
            codepoint  = (*s & 0x1F) << 6;
            codepoint |= (s[1] & 0x3F);
            s += 2;
        } else if ((*s & 0xF0) == 0xE0) { // 3-byte
            codepoint  = (*s & 0x0F) << 12;
            codepoint |= (s[1] & 0x3F) << 6;
            codepoint |= (s[2] & 0x3F);
            s += 3;
        } else if ((*s & 0xF8) == 0xF0) { // 4-byte
            codepoint  = (*s & 0x07) << 18;
            codepoint |= (s[1] & 0x3F) << 12;
            codepoint |= (s[2] & 0x3F) << 6;
            codepoint |= (s[3] & 0x3F);
            s += 4;
        } else s++; // invalid byte
        p = (const char *)s;
        characterCount++;
        if (codepoint == '\n' || characterCount > 120) {xpos=x; ypos+=lineSpacing; characterCount=0; continue;}
        
        int idx = CodepointToPackedIndex(codepoint, fontID);
        if (fontID == FONT_STOPD) stbtt_GetPackedQuad(fontPackedCharStopD,FONT_ATLAS_SIZE,FONT_ATLAS_SIZE,idx,&xpos,&ypos,&q,1);
        else stbtt_GetPackedQuad(fontPackedChar,FONT_ATLAS_SIZE,FONT_ATLAS_SIZE,idx,&xpos,&ypos,&q,1);
        float vx0 = (q.x0 * scale) - (borderWidthPixels);
        float vy0 = (q.y0 * scale) - (borderWidthPixels);
        float vx1 = (q.x1 * scale) + (borderWidthPixels);
        float vy1 = (q.y1 * scale) + (borderWidthPixels);
        float s0 = (q.s0) - (paddingUV);
        float t0 = (q.t0) - (paddingUV);
        float s1 = (q.s1) + (paddingUV);
        float t1 = (q.t1) + (paddingUV);
        float z = 0.0f;
        float tverts[30] = {vx0,vy0,z,s0,t0,vx1,vy1,z,s1,t1,vx1,vy0,z,s1,t0,vx0,vy0,z,s0,t0,vx0,vy1,z,s0,t1,vx1,vy1,z,s1,t1};
        __builtin_memcpy(textVertexData + vertexCount * 30,tverts,sizeof(tverts));
        vertexCount++;
        if (codepoint >= '0' && codepoint <= '9') {
            if (fontID == FONT_STOPD) xpos = q.x0 + fixedNumberAdvanceWidthStopD;
            else xpos = q.x0 + fixedNumberAdvanceWidth;
        }
    }
    
    if (vertexCount > 0) {
        glNamedBufferData(Sys_Render.textVBO,vertexCount * 30 * sizeof(float),textVertexData,GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES,0,vertexCount * 6);
        drawCallsRenderedThisFrame++; textDrawCallsRenderedThisFrame++; verticesRenderedThisFrame += vertexCount * 6;
    }
}

void RenderLoadingProgress(i32 offset, const char * restrict text) { // Only adds 0.01secs to game startup time.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    RenderFormattedText(Sys_Settings.ScreenWidth / 2 - offset, Sys_Settings.ScreenHeight / 2 - 5, TEXT_WHITE, FONT_NORMAL,1.0f,text);
    glfwSwapBuffers(window);
}

char statusText[TEXT_BUFFER_SIZE];
void CenterStatusPrint(const char * restrict fmt, ...) {
    va_list args; __builtin_va_start(args, fmt); StringFormatV(statusText,TEXT_BUFFER_SIZE,fmt,args); __builtin_va_end(args);
    DualLog("%s\n",statusText);
    Sys_Global.statusTextDecayFinished = get_time() + 2.5; // 2.5 second decay time before text dissappears.
}

InputSystem Sys_Input;
extern u16 editModeTestEntityDefinition;
extern u16 editModeSelection;
bool mouseMovementThisFrame;
typedef struct { const char* name; int value; } InputElement;
InputElement inputElements[149] = {
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
    { "PRINT", GLFW_KEY_PRINT_SCREEN }, { "JOY 18", GLFW_HAT_DOWN }, { "JOY 19", GLFW_HAT_LEFT }, { "GPAD A", GLFW_GAMEPAD_BUTTON_A }, { "GPAD B", GLFW_GAMEPAD_BUTTON_B }, { "GPAD X", GLFW_GAMEPAD_BUTTON_X }, { "GPAD Y", GLFW_GAMEPAD_BUTTON_Y }, { "GPAD L1", GLFW_GAMEPAD_BUTTON_LEFT_BUMPER }, { "GPAD R1", GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER }, { "GPAD BACK", GLFW_GAMEPAD_BUTTON_BACK },
    { "GPAD START", GLFW_GAMEPAD_BUTTON_START }, { "GPAD GUIDE", GLFW_GAMEPAD_BUTTON_GUIDE }, { "GPAD LSTICK CLICK", GLFW_GAMEPAD_BUTTON_LEFT_THUMB }, { "GPAD RSTICK CLICK", GLFW_GAMEPAD_BUTTON_RIGHT_THUMB }, { "GPAD D UP", GLFW_GAMEPAD_BUTTON_DPAD_UP }, { "GPAD D RIGHT", GLFW_GAMEPAD_BUTTON_DPAD_RIGHT }, { "GPAD D DOWN", GLFW_GAMEPAD_BUTTON_DPAD_DOWN }, { "GPAD D LEFT", GLFW_GAMEPAD_BUTTON_DPAD_LEFT }, { "UNUSED", 0 } //, {}
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
static inline __attribute__((always_inline)) i32 GetGLFWIndirectionIndexForAnInput(const char* val) { for (int i=0;i<149;++i) {if (StringsEqual(val,inputElements[i].name)) return i;} return 148; }
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
    DualLog("Saved settings to ./Data/Config.ini!\n");
}

void TextEntry(i32 k) {
    if (k == GLFW_KEY_U && Sys_Input.keyStates[GLFW_KEY_LEFT_CONTROL].down) { Sys_Global.playerName[0] = '\0'; currentPlayerNameLength = 0; return; }
    if (k == GLFW_KEY_ENTER || k == GLFW_KEY_KP_ENTER) { currentMenuItem++; return; }
    if (k == GLFW_KEY_BACKSPACE && currentPlayerNameLength > 0) { Sys_Global.playerName[--currentPlayerNameLength] = '\0'; return; }
    if (currentPlayerNameLength >= 26) return;
    char c = (k >= GLFW_KEY_A && k <= GLFW_KEY_Z) ? 'a' + (k - GLFW_KEY_A) : ((k >= GLFW_KEY_1 && k <= GLFW_KEY_9) ? '1' + (k - GLFW_KEY_1) : ((k == GLFW_KEY_0) ? '0' : ((k == GLFW_KEY_SPACE) ? ' ' : 0)));
    if (c) { Sys_Global.playerName[currentPlayerNameLength] = c; Sys_Global.playerName[++currentPlayerNameLength] = '\0'; }
}

// Create a quaternion from yaw (around Y), pitch (around X), and roll (around Z) in degrees
void quat_from_yaw_pitch_roll(Quaternion* q, float yaw_deg, float pitch_deg, float roll_deg) {
    float yaw = deg2rad(yaw_deg), pitch = deg2rad(pitch_deg), roll = deg2rad(roll_deg);  // Around Z (forward)
    float cy = vcosf(yaw * 0.5f), sy = vsinf(yaw * 0.5f), cp = vcosf(pitch * 0.5f), sp = vsinf(pitch * 0.5f), cr = vcosf(roll * 0.5f), sr = vsinf(roll * 0.5f);
    q->w = cy * cp * cr + sy * sp * sr;
    q->x = cy * sp * cr + sy * cp * sr; // X-axis (pitch)
    q->y = sy * cp * cr - cy * sp * sr; // Y-axis (yaw)
    q->z = cy * cp * sr - sy * sp * cr; // Z-axis (roll)
} // Skipping quat normalization, not needed

extern float cam_yaw,cam_pitch,cam_roll;
void Input_MouselookApply(void) {
    quat_from_yaw_pitch_roll(&Sys_Global.instances[PLAYER1].rotation,cam_yaw,cam_pitch,(Sys_Global.currentLevel == LEVEL_CYBERSPACE) ? cam_roll : 0.0f);
    Quaternion rot = Sys_Global.instances[PLAYER1].rotation;
    float y2 = rot.y * rot.y;  float xz = rot.x * rot.z;  float wy = rot.w * rot.y;
    Sys_Global.instances[PLAYER1].forward = normalize_vector3((Vector3){ 2.0f * (xz + wy), 2.0f * (rot.y * rot.z - rot.w * rot.x), 1.0f - 2.0f * (rot.x * rot.x + y2) });
    Sys_Global.instances[PLAYER1].right = normalize_vector3((Vector3){ 1.0f - 2.0f * (y2 + rot.z * rot.z), 2.0f * (rot.x * rot.y + rot.w * rot.z), 2.0f * (xz - wy) });
    ma_engine_listener_set_direction(&audio_engine,0,Sys_Global.instances[PLAYER1].forward.x,Sys_Global.instances[PLAYER1].forward.y,Sys_Global.instances[PLAYER1].forward.z);
    Vector3 up = cross_vector3(Sys_Global.instances[PLAYER1].forward,Sys_Global.instances[PLAYER1].right);
    ma_engine_listener_set_world_up(&audio_engine,0,up.x,up.y,up.z);
}

// static const float HeadBobRate   = 0.2f; TODO
// static const float HeadBobAmount = 0.08f; TODO
// static const float bobTarget = 0.3f; TODO
i32 Input_MouseMove(i32 xrel, i32 yrel) {
    if ((Sys_Global.inventoryMode && !Sys_Cheats.noHUD) || Sys_Global.menuActive || Sys_Global.gamePaused) { // Uses UI baseline resolution 1366x768
        i32 newX = clamp(Sys_Global.cursorPosition_x + xrel,0,1366); if (newX != Sys_Global.cursorPosition_x) {mouseMovementThisFrame = true;} Sys_Global.cursorPosition_x = newX;
        i32 newY = clamp(Sys_Global.cursorPosition_y + yrel,0,768);  if (newY != Sys_Global.cursorPosition_y) {mouseMovementThisFrame = true;} Sys_Global.cursorPosition_y = newY;
    }
    
    if (Sys_Global.gamePaused || Sys_Global.menuActive || Sys_Global.inventoryMode) return 0;
    
    float sensitivity = vclamp((float)Sys_Settings.MouseSensitivity / 100.0f, 0.01f, 1.0f) * 0.2f;
    cam_yaw += (float)xrel * sensitivity;
    if (cam_yaw >= 360.0f) cam_yaw -= 360.0f;
    if (cam_yaw < 0.0f) cam_yaw += 360.0f;
    cam_pitch += (float)yrel * sensitivity;
    if (cam_pitch > 89.0f) cam_pitch = 89.0f; // Avoid gimbal lock at pure 90deg
    if (cam_pitch < -89.0f) cam_pitch = -89.0f;
    Input_MouselookApply();
    return 0;
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
         if (i == 129) return Sys_Input.scrollDelta > 0.0; // Mousewheel +
    else if (i == 130) return Sys_Input.scrollDelta < 0.0; // Mousewheel -
    
    KeyState* keyOfConcern = GetCodeMapping(settingIndex);
    bool retval = risingEdge ? keyOfConcern->pressed : keyOfConcern->down;
    return retval;
}

ENGINE_TO_MOD bool GetKey(int settingIndex) { return GetKeyRiseEdgeOrHeld(settingIndex,false); }  // True while held down.
ENGINE_TO_MOD bool GetKeyPressed(int settingIndex) { return (settingIndex < 0) ? Sys_Input.keyStates[GLFW_KEY_GRAVE_ACCENT].pressed : GetKeyRiseEdgeOrHeld(settingIndex,true); } // True 1st frame down.
ENGINE_TO_MOD void IgnoreNextMouseDelta(void) { Sys_Input.ignore_next_mouse_delta = true; }
OsFileHandle levelFileHandle; void CullInit(void);
void LoadLevel(u8 curlevel) {
    double start_time = get_time();
    DebugRAM("start of LoadLevel");
    Sys_Global.levelCurrentlyLoading = true; Sys_Global.gamePaused = false; Sys_Global.menuActive = false;
    queuedLevelToLoad = 255u; // Reset any loading state that got us here.
    RenderLoadingProgress(100,"Loading level...");
    __builtin_memset(lights,0,LIGHT_COUNT * sizeof(Light)); __builtin_memset(lanims,0,LIGHT_COUNT * sizeof(LightAnimation));
    __builtin_memset(modelMatrices,0,INSTANCE_COUNT * 16 * sizeof(float)); // Matrix4x4 = 16
    __builtin_memset(camViews,0,64 * sizeof(CamView)); camViewCount = 0;
    __builtin_memset(Sys_Global.instances + 3,0,(INSTANCE_COUNT - 3) * sizeof(Entity)); // Initialize instances, the global entity array for the currently loaded level.
    char filename[20]; // Minimum size for 0 through 13.
    StringFormat(filename, sizeof(filename), "./Data/level%d.txt", curlevel);
    levelFileHandle = OS_OpenReadonly(filename);
    LoadLevelMod(curlevel);
    OS_Close(levelFileHandle);
    for (int i=0;i<Sys_Global.loadedLights;++i) {/* lights[i].maxIntensity *= 2.0f; */lightsNewPosition[i]=lights[i].pos; }
    DualLog("Loaded %d entities, %u static lights for Level %d... took %f secs\n",Sys_Global.loadedInstances,Sys_Global.loadedLights,curlevel,get_time() - start_time);
    DebugRAM("end of LoadLevel instances");
    RenderLoadingProgress(110,"Initialize entities...");
    for (int i=PLAYER1;i<Sys_Global.loadedInstances;++i) {        
        i32 cellIdx = PosGetCellCoords(Sys_Global.instances[i].position.x,Sys_Global.instances[i].position.z);
        Sys_Global.instances[i].cellIndex = cellIdx;
    }
    
    ModInitAfterLoad(); ResetLevelAudio(); ResetLevelMusic();
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
    __builtin_memset(voxen_Shadow_System.shadowmapIndirectionList,MAX_SHADOWMAPS + 1,Sys_Global.loadedLights * sizeof(u32)); // Set to invalid values for all
    Sys_Global.levelCurrentlyLoading = false;
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
    currentMenuItem = currentMenuTab = 0; currentMenuPage = MenuPages_FrontPage;
    Sys_Global.pauseRelativeTime = Sys_Global.last_physics_time = 0.0;
    Sys_Global.inventoryMode = false;
    __builtin_memset(Sys_Global.instances,0,2 * sizeof(Entity)); // Blank out player entities
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

void GoIntoGame(void) { NewGame(); PlayGameMusic(); DualLog("Player named \"%s\" started the game!\n", Sys_Global.playerName); }

void GenerateAndBindTexture(GLuint *id, GLint internalFormat, i32 width, i32 height, GLenum format, GLenum type) {
    if (*id == 0) {glGenTextures(1,id);} glBindTexture(GL_TEXTURE_2D,*id);
    glTexImage2D(GL_TEXTURE_2D,0,internalFormat,width,height,0,format,type,NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST); glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
}

void UpdateScreenSize(GLFWwindow* unused, i32 width, i32 height) {
    u16 w,h; float wf,hf; (void)unused; // Appease glfwSetFramebufferSizeCallback pointer type
    w = Sys_Settings.ScreenWidth = vmax(vmin((u16)width,7680u),320u); h = Sys_Settings.ScreenHeight = vmax(vmin((u16)height,4320u),200u); // Cap at minimum Quake resolution and maximum 8k.
    wf = (float)w; hf = (float)h; Sys_Settings.ScreenCenterX = wf * 0.5f; Sys_Settings.ScreenCenterY = hf * 0.5f;
    glViewport(0,0,w,h);
    glUseProgram(Sys_Render.imageBlitShaderProgram); glUniform1ui(2,w); glUniform1ui(3,h); glUniform1i(26,Sys_Settings.SSR_RES);
    glUseProgram(Sys_Render.chunkShaderProgram); glUniform1ui(6,w); glUniform1ui(7,h);
    glUseProgram(Sys_Render.ssrShaderProgram); glUniform1ui(0,w / Sys_Settings.SSR_RES); glUniform1ui(1,h / Sys_Settings.SSR_RES); glUniform1i(2,Sys_Settings.SSR_RES);
    GenerateAndBindTexture(&Sys_Render.inputImageID,     GL_RGBA8,w,h,GL_RGBA,GL_UNSIGNED_BYTE); // Lit Raster
    GenerateAndBindTexture(&Sys_Render.inputWorldPosID,GL_RGBA32F,w,h,GL_RGBA,        GL_FLOAT); // Raster World Positions
    GenerateAndBindTexture(&Sys_Render.inputSpecID,      GL_RGBA8,w,h,GL_RGBA,GL_UNSIGNED_BYTE); // Specular Colors
    GenerateAndBindTexture(&Sys_Render.inputNormalID,    GL_RG16F,w,h, GL_RGB,        GL_FLOAT); // Normal XYZ
    GenerateAndBindTexture(&Sys_Render.inputDepthID,GL_DEPTH_COMPONENT32,w,h,GL_DEPTH_COMPONENT,GL_FLOAT); // Raster Depth
    if (Sys_Render.outputImageID == 0) glGenTextures(1,&Sys_Render.outputImageID);
    glBindTexture(GL_TEXTURE_2D,Sys_Render.outputImageID);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,w / Sys_Settings.SSR_RES,h / Sys_Settings.SSR_RES,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D,0);
    glBindFramebuffer(GL_FRAMEBUFFER,Sys_Render.gBufferFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,Sys_Render.inputImageID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT1,GL_TEXTURE_2D,Sys_Render.inputWorldPosID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT2,GL_TEXTURE_2D,Sys_Render.inputSpecID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT3,GL_TEXTURE_2D,Sys_Render.inputNormalID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,Sys_Render.inputDepthID, 0);
    glBindImageTexture(0,Sys_Render.inputImageID,0,GL_FALSE,0,GL_READ_WRITE,GL_RGBA8);      // Main Rendered Color
    glBindImageTexture(1,Sys_Render.inputWorldPosID,0,GL_FALSE,0,GL_READ_WRITE,GL_RGBA32F); // World Position XYZ
    glBindImageTexture(2,Sys_Render.inputSpecID,0,GL_FALSE,0,GL_READ_WRITE,GL_RGBA8);       // Specular
    glBindImageTexture(4,Sys_Render.outputImageID,0,GL_FALSE,0,GL_READ_WRITE,GL_RGBA8);     // SSR result
    glBindImageTexture(5,Sys_Render.inputNormalID,0,GL_FALSE,0,GL_READ_WRITE,GL_RG16F);     // Normal XYZ
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D,Sys_Render.outputImageID);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    uiOrthoProjection[0] = 2.0f / wf; uiOrthoProjection[1] =       0.0f; uiOrthoProjection[2] =  0.0f; uiOrthoProjection[3] = 0.0f;
    uiOrthoProjection[4] =      0.0f; uiOrthoProjection[5] = -2.0f / hf; uiOrthoProjection[6] =  0.0f; uiOrthoProjection[7] = 0.0f;
    uiOrthoProjection[8] =      0.0f; uiOrthoProjection[9] =       0.0f; uiOrthoProjection[10]= -1.0f; uiOrthoProjection[11]= 0.0f;
    uiOrthoProjection[12]=     -1.0f; uiOrthoProjection[13]=       1.0f; uiOrthoProjection[14]=  0.0f; uiOrthoProjection[15]= 1.0f;
    glfwSwapBuffers(window);
}

ENGINE_TO_MOD void AddCamView(Vector3 pos, Quaternion rot, u8 fov, u16 width, u16 height, float near, float far) {    
    camViews[camViewCount] = (CamView){pos,rot,fov,width,height,near,far,Sys_Global.pauseRelativeTime + (camViewCount * 0.05f) + 0.5f,false}; // Staggered starts so not all at once for performance.
    GenerateAndBindTexture(&camViewTextures[camViewCount],GL_RGBA8,width,height,GL_RGBA,GL_UNSIGNED_BYTE); camViewCount++;
}

// GLFW Callbacks
bool IsNonRepeatingKey(i32 key) { return key == GLFW_KEY_KP_ENTER || key == GLFW_KEY_ENTER || key == GLFW_KEY_TAB || key == GLFW_KEY_ESCAPE; }
extern bool enteringPlayerName; void ConsoleEmulator(i32 keycode);
static void key_callback(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods) {
    if (key == GLFW_KEY_F10 && action) OS_Exit(0); (void)window; (void)scancode; (void)mods; // Suppress warnings about unused parameters forced upon me by glfw3 dependency deadweight anchor.
    if (Sys_Global.menuActive && !returnToPause) {
        if ((key == GLFW_KEY_RIGHT_ALT || key == GLFW_KEY_LEFT_ALT) && action && Sys_Input.keyStates[GLFW_KEY_ENTER].down)                    GoIntoGame();
        if (key == GLFW_KEY_ENTER && action && (Sys_Input.keyStates[GLFW_KEY_LEFT_ALT].down || Sys_Input.keyStates[GLFW_KEY_RIGHT_ALT].down)) GoIntoGame();
    }

    if (key >=0 && key < MAX_KEYS && (action == GLFW_PRESS || (action == GLFW_REPEAT && !IsNonRepeatingKey(key)))) {
        Sys_Input.keyStates[key].pressed = Sys_Input.keyStates[key].down = true;
        if (Sys_Cheats.consoleActive) ConsoleEmulator(key);
        else if (enteringPlayerName && Sys_Global.menuActive) TextEntry(key);
    } else if (key >= 0 && key < MAX_KEYS && action == GLFW_RELEASE) Sys_Input.keyStates[key].pressed = Sys_Input.keyStates[key].down = false;
}

void Input_Poll(void) {
    glfwPollEvents();
    for (int jid = GLFW_JOYSTICK_1; jid <= GLFW_JOYSTICK_LAST; ++jid) {
        if (!glfwJoystickPresent(jid)) continue;

        int buttonCount;
        const unsigned char* buttons = glfwGetJoystickButtons(jid, &buttonCount);
        for (int i = 0; i < buttonCount && i < MAX_JOYSTICK_BUTTONS; ++i) {
            KeyState* k = &Sys_Input.joystickButtons[GLFW_JOYSTICK_1][i];
            bool down = buttons[i] == GLFW_PRESS;
            k->pressed =down&&!k->down; k->released=!down&&k->down; k->down=down;
        }

        int hatCount; const unsigned char* hats = glfwGetJoystickHats(jid, &hatCount);
        for (int i = 0; i < hatCount && i < MAX_JOYSTICK_HATS; ++i) Sys_Input.joystickHats[i].down = hats[i];
    }

    GLFWgamepadstate s;
    if (!glfwGetGamepadState(GLFW_JOYSTICK_1,&s)) return;

    for (int i = 0; i < GLFW_GAMEPAD_BUTTON_LAST + 1; ++i) {
        KeyState* k = &Sys_Input.gamepadButtons[i];
        bool down = s.buttons[i] == GLFW_PRESS;
        k->pressed=down&&!k->down; k->released=!down&&k->down; k->down=down;
    }
}

static void joystick_callback(i32 jid, i32 event) {
    if (jid > GLFW_JOYSTICK_LAST) return;
    bool connected = event == GLFW_CONNECTED;
    Sys_Input.joystickPresent[jid] = connected;
    if (!connected) { __builtin_memset(Sys_Input.joystickButtons,0,sizeof(Sys_Input.joystickButtons)); __builtin_memset(Sys_Input.joystickHats,0,sizeof(Sys_Input.joystickHats)); } // Clear
}

static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    (void)window;
    if (Sys_Input.window_has_focus) {
        Sys_Input.currentMouse_dx = (i32)(xpos - Sys_Input.last_mouse_x);
        Sys_Input.currentMouse_dy = (i32)(ypos - Sys_Input.last_mouse_y);
        Sys_Input.last_mouse_x = xpos;
        Sys_Input.last_mouse_y = ypos;
        if (Sys_Input.ignore_next_mouse_delta) { Sys_Input.ignore_next_mouse_delta = false; return; }
        
        if (Sys_Global.globalFrameNum > 1) Input_MouseMove(Sys_Input.currentMouse_dx,Sys_Input.currentMouse_dy);
    }
}

static void window_focus_callback(GLFWwindow* window, i32 focused) {
    Sys_Input.window_has_focus = focused != 0; (void)window;
    Sys_Input.ignore_next_mouse_delta = true;
    glfwSetInputMode(window,GLFW_CURSOR,Sys_Input.window_has_focus ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

static void mouse_button_callback(GLFWwindow* window, i32 button, i32 action, i32 mods) {
    Sys_Input.mouseButtons[button].down = Sys_Input.mouseButtons[button].pressed = (action == GLFW_PRESS); (void)window; (void)mods;
    Sys_Input.mouseButtons[button].released = (action == GLFW_RELEASE);
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) { (void)window; (void)xoffset; Sys_Input.scrollDelta += yoffset; }

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
    int wx,wy,ww,wh; glfwGetWindowPos(window,&wx,&wy); glfwGetWindowSize(window,&ww,&wh);
    GLFWmonitor* bestMonitor = glfwGetPrimaryMonitor();
    int bestArea=0,monitorCount; GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
    for (int i=0;i<monitorCount;++i) {
        int mx, my;
        glfwGetMonitorPos(monitors[i], &mx, &my);
        const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
        int mw = mode->width;
        int mh = mode->height;
        int left   = vmax(wx,mx);
        int right  = vmin(wx + ww, mx + mw);
        int top    = vmax(wy,my);
        int bottom = vmin(wy + wh, my + mh);
        int area = (right > left && bottom > top) ? (right - left) * (bottom - top) : 0;
        if (area > bestArea) {
            bestArea = area;
            bestMonitor = monitors[i];
        }
    }
    return bestMonitor;
}

ENGINE_TO_MOD bool GetSoundIsPlaying(ma_sound* sound) { return ma_sound_is_playing(sound); }
float GetSoundRemainingTime(ma_sound* pSound) {
    if (!pSound || !ma_sound_is_playing(pSound)) return 0.0f;

    ma_uint64 currentFrame = ma_sound_get_time_in_pcm_frames(pSound);
    ma_uint64 pcmFramesLength = 0;
    ma_sound_get_length_in_pcm_frames(pSound, &pcmFramesLength);
    if (currentFrame >= pcmFramesLength) return 0.0f;

    u64 deltaFrames = pcmFramesLength - currentFrame;
    u32 sampleRate = ma_engine_get_sample_rate(&audio_engine);
    return (float)deltaFrames / (float)sampleRate;
}

void mp3_clear(void) {
    ma_sound_stop(&Sys_Global.mp3_sounds[0]);
    ma_sound_stop(&Sys_Global.mp3_sounds[1]);
    Sys_Global.mp3_slot = 0;
}

ENGINE_TO_MOD void SoundSetVolume(ma_sound* pSound, float volume) { ma_sound_set_volume(pSound,volume); }
float GetSFXVolume(float volume) { return ((float)Sys_Settings.VolumeMaster/100.0f) * ((float)Sys_Settings.VolumeEffects/100.0f) * volume; }
float GetMusicVolume(void) { return ((float)Sys_Settings.VolumeMaster/100.0f) * ((float)Sys_Settings.VolumeMusic/100.0f); }
float GetMessageVolume(void) { return ((float)Sys_Settings.VolumeMaster/100.0f) * ((float)Sys_Settings.VolumeMessage/100.0f); }
void set_music_volume(void) { for (int i=0;i<2;++i) { ma_sound_set_volume(&Sys_Global.mp3_sounds[i], GetMusicVolume()); } }
void set_sfx_volume(void) { for (int i=0;i<MAX_CHANNELS;++i) { ma_sound_set_volume(&wav_sounds[i], GetSFXVolume(wav_volumes[i])); } }
void set_message_volume(void) { ma_sound_set_volume(&log_sound, GetMessageVolume()); }
void set_master_volume(void) { set_sfx_volume(); set_music_volume(); set_message_volume(); }

void play_mp3(const char* path, i32 fade_in_ms) {
    i32 old_slot = Sys_Global.mp3_slot;
    i32 next_slot = Sys_Global.mp3_slot ? 0 : 1;
    if (ma_sound_is_playing(&Sys_Global.mp3_sounds[old_slot])) ma_sound_set_fade_in_milliseconds(&Sys_Global.mp3_sounds[old_slot], GetMusicVolume(), 0.0f, fade_in_ms);
    ma_sound_uninit(&Sys_Global.mp3_sounds[next_slot]); 
    ma_result result = ma_sound_init_from_file(&audio_engine, path, MA_SOUND_FLAG_STREAM, NULL, NULL, &Sys_Global.mp3_sounds[next_slot]);
    if (result != MA_SUCCESS) { DualLog("ERROR: Failed to load MP3 %s: %d\n", path, result); return; }

    ma_sound_set_fade_in_milliseconds(&Sys_Global.mp3_sounds[next_slot], 0.0f, GetMusicVolume(), fade_in_ms);
    ma_sound_start(&Sys_Global.mp3_sounds[next_slot]);
    Sys_Global.mp3_slot = next_slot;
}

ENGINE_TO_MOD void play_wav(const char* path, float volume, Vector3 pos, bool positional) {
    i32 slot = -1;
    for (i32 i = 0; i < wav_count; i++) { // Try to find a free slot (either unused or finished)
        if (!ma_sound_is_playing(&wav_sounds[i]) && ma_sound_at_end(&wav_sounds[i])) {
            ma_sound_uninit(&wav_sounds[i]);
            slot = i;
            break;
        }
    }

    if (slot == -1 && wav_count < MAX_CHANNELS) slot = wav_count++; // If no free slot, use a new one if available
    if (slot == -1) { DualLog("WARNING: Max effect WAV channels (%d) reached\n", MAX_CHANNELS); return; }

    ma_result result = ma_sound_init_from_file(&audio_engine, path, 0, NULL, NULL, &wav_sounds[slot]);
    if (result != MA_SUCCESS) {
        DualLog("ERROR: Failed to load effect WAV %s: %d\n", path, result);
        if (slot == wav_count - 1) wav_count--; // Revert count if init fails
        return;
    }
    
    if (positional) ma_sound_set_position(&wav_sounds[slot], pos.x, pos.y, pos.z);
    ma_sound_set_spatialization_enabled(&wav_sounds[slot], (ma_bool32)positional);
    wav_volumes[slot] = volume;
    ma_sound_set_volume(&wav_sounds[slot], GetSFXVolume(wav_volumes[slot]));
    ma_sound_start(&wav_sounds[slot]);
}

ENGINE_TO_MOD void play_message(const char* path) {
    if (ma_sound_is_playing(&log_sound)) { ma_sound_stop(&log_sound); ma_sound_uninit(&log_sound); }
    ma_result result = ma_sound_init_from_file(&audio_engine, path, 0, NULL, NULL, &log_sound);
    if (result != MA_SUCCESS) { DualLog("ERROR: Failed to load message WAV %s: %d\n", path, result); return; }
    
    ma_sound_set_spatialization_enabled(&log_sound, false);
    ma_sound_set_volume(&log_sound, GetMessageVolume());
    ma_sound_start(&log_sound);
}

ENGINE_TO_MOD void SoundUninit(ma_sound* snd) { ma_sound_uninit(snd); }
ENGINE_TO_MOD ma_result SoundInit(const char* path, ma_uint32 flags, ma_sound_group* pGroup, ma_fence* pDoneFence, ma_sound* pSound) { return ma_sound_init_from_file(&audio_engine,path,flags,pGroup,pDoneFence,pSound); }
ENGINE_TO_MOD void SoundSetLooping(ma_sound* pSound, ma_bool32 isLooping) { ma_sound_set_looping(pSound,isLooping); }
ENGINE_TO_MOD ma_result SoundStart(ma_sound* pSound) { return ma_sound_start(pSound); }
ENGINE_TO_MOD ma_result SoundStop(ma_sound* pSound) { return ma_sound_stop(pSound); }
ENGINE_TO_MOD ma_result SoundGetCurrentFrameCursor(const ma_sound* pSound, ma_uint64* pCursor) { return ma_sound_get_cursor_in_pcm_frames(pSound,pCursor); }
ENGINE_TO_MOD float SoundGetLength(ma_sound* pSound) {
    if (!pSound) return 0.0f;
    
    ma_uint64 frames;
    if (ma_sound_get_length_in_pcm_frames(pSound, &frames) != MA_SUCCESS) return 0.0f;
    
    ma_uint32 sr = ma_engine_get_sample_rate(ma_sound_get_engine(pSound));
    return (sr == 0) ? 0.0f : (float)frames / (float)sr;
}

void GatherResolutionModes(void) {
    resDropdownCount = 0;
    GLFWmonitor* monitor = GetCurrentMonitor();    if (!monitor) monitor = glfwGetPrimaryMonitor();
    int modeCount; const GLFWvidmode* modes = glfwGetVideoModes(monitor, &modeCount);
    if (!modes || modeCount < 1) return;
    for (int i = modeCount - 1; i >= 0 && resDropdownCount < 8; --i) {
        int w = modes[i].width, h = modes[i].height, hz = modes[i].refreshRate;
        int j = 0;
        for (; j < resDropdownCount; ++j) {
            if (resModes[j].w == w && resModes[j].h == h) { if (hz > resModes[j].hz) resModes[j].hz = hz; break; }
        }
        
        if (j == resDropdownCount) { resModes[resDropdownCount++] = (ResMode){w, h, hz}; }
    }
    
    resSelectedIdx = 0;
    for (int i = 0; i < resDropdownCount; ++i) {
        if (resModes[i].w == (int)Sys_Settings.ScreenWidth && resModes[i].h == (int)Sys_Settings.ScreenHeight) { resSelectedIdx = i; break; }
    }
}

void SaveConfig(void);
void ChangeResolution(void) {
    if (resDropdownCount < 1) return;
    resSelectedIdx = (resSelectedIdx + 1) % resDropdownCount;
    Sys_Settings.ScreenWidth  = (u32)resModes[resSelectedIdx].w;
    Sys_Settings.ScreenHeight = (u32)resModes[resSelectedIdx].h;
    GLFWmonitor* monitor = GetCurrentMonitor();
    if (!monitor) monitor = glfwGetPrimaryMonitor();
    int mx,my; glfwGetMonitorPos(monitor, &mx, &my);
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    int xpos = mx + (mode->width  - (int)Sys_Settings.ScreenWidth)  / 2;
    int ypos = my + (mode->height - (int)Sys_Settings.ScreenHeight) / 2;
    glfwSetWindowSize(window, (int)Sys_Settings.ScreenWidth, (int)Sys_Settings.ScreenHeight);
    glfwSetWindowPos(window, xpos, ypos);
    UpdateScreenSize(NULL,(int)Sys_Settings.ScreenWidth,(int)Sys_Settings.ScreenHeight);
    Sys_Input.ignore_next_mouse_delta = true;
    resDropdownOpen = false;
    SaveConfig();
}

GLFWAPI void glfwSetWindowAttrib(GLFWwindow* window, int attrib, int value);
void ChangeFullScreenWindowed(void) {
    int monitorCount; GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
    GLFWmonitor* monitor = monitors[Sys_Settings.CurrentMonitor];
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    if (Sys_Settings.Fullscreen) {
        glfwSetWindowAttrib(window,0x00020005/*GLFW_DECORATED*/,0);
        int x,y,w,h;
        glfwGetMonitorWorkarea(monitor,&x,&y,&w,&h);
        glfwSetWindowMonitor(window,NULL,x,y,w,h,mode->refreshRate);
        Sys_Settings.ScreenWidth = w;
        Sys_Settings.ScreenHeight = h;
    } else {
        glfwSetWindowAttrib(window,0x00020005/*GLFW_DECORATED*/,1);
        int mx, my; 
        glfwGetMonitorPos(monitor,&mx,&my);
        int x,y,w,h;
        glfwGetMonitorWorkarea(monitor,&x,&y,&w,&h);
        Sys_Settings.ScreenWidth  = vmax(vmin((w*3)/4,1366),320);
        Sys_Settings.ScreenHeight = vmax(vmin((h*3)/4, 768),200);
        int xpos = mx + (mode->width - Sys_Settings.ScreenWidth) / 2;
        int ypos = my + (mode->height - Sys_Settings.ScreenHeight) / 2;
        glfwSetWindowMonitor(window,NULL,xpos,ypos,Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight,mode->refreshRate);
    }
    
    UpdateScreenSize(NULL,Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight);
    Sys_Input.ignore_next_mouse_delta = true;
}

void SetSkyRotateSpeed(void) {
    static const float speeds[] = { 0.05f, 1.0f, 2.5f, 3.75f, 6.25f };
    float skyRotateSpeed = speeds[Sys_Cheats.dizzyLevel];
    glUseProgram(Sys_Render.imageBlitShaderProgram);
    glUniform1f(30, skyRotateSpeed);
}

void SetVSync(void) { glfwSwapInterval((i32)Sys_Settings.Vsync); }
void SetGI(void) { }// TODO: Set needed Voxel GI uniforms from Sys_Settings.GI
void SetSpeakerMode(void) {
    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    switch (Sys_Settings.SpeakerMode) {
        case 0: config.playback.channels = 1; break;  // Mono
        case 1: config.playback.channels = 2; break;  // Stereo
        case 2: config.playback.channels = 4; break;  // Quad (4.0)
        case 3: config.playback.channels = 4; break;  // Surround (often 4.0)
        case 4: config.playback.channels = 6; break;  // 5.1
        case 5: config.playback.channels = 8; break;  // 7.1
        case 6: config.playback.channels = 2; break;  // Prologic → usually handled as stereo + decoder
        default: config.playback.channels = 0; break; // Let device decide
    }
}

#ifdef WINDOWS
    static const char* PLATFORM_DLERROR(void) {
        DWORD err = GetLastError(); if (err == 0) return NULL;
        FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,err,MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),win_err_buf,sizeof(win_err_buf),NULL);
        return win_err_buf;
    }
#else
    #define PLATFORM_DLERROR() dlerror()
#endif
void LoadTextForLanguage(u8),LoadLogTextForLanguage(u8); bool GetKey(int settingIndex),GetKeyPressed(int settingIndex); void* mod_handle = NULL;
void SetLanguage(void) { LoadTextForLanguage(Sys_Settings.Language); LoadLogTextForLanguage(Sys_Settings.Language); }
void ApplySettings(void) { ChangeFullScreenWindowed(); SetSkyRotateSpeed(); SetVSync(); SetGI(); SetSpeakerMode(); SetLanguage(); }
void LoadModFunctions(void) {
    DualLog("Loading mod code...");
    char mod_path[256];
    StringCopyInto_A_From_B(mod_path,"./",256);
    StringConcatenate(mod_path,Sys_Global.global_modname,256);
    StringConcatenate(mod_path,MOD_EXTENSION,256);
    mod_handle = PLATFORM_DLOPEN(mod_path);
    if (!mod_handle) {
        const char* err = PLATFORM_DLERROR();
        if (err && *err) DualLogError("dlopen of %s failed: %s",mod_path,err);
        else             DualLogError("dlopen of %s failed: no detailed error from dlerror() — common with unresolved symbols or format issues",mod_path);
        OS_Exit(1);
    }
    
    #define X(ret, name, params) \
        name = (ret (*) params)PLATFORM_DLSYM(mod_handle, #name); \
        if(!name) DualLogError("Failed to load mod function: %s", #name);
    MOD_FUNCTION_LIST(X)
    #undef X
    ModLink(&Sys_Global,&Sys_Cheats,&Sys_Settings,&Sys_Text,&Sys_UI); // Link engine to mod
    Sys_Global.GetKey = GetKey; Sys_Global.GetKeyPressed = GetKeyPressed; // Link mod to engine
    DualLog("done!\n");
}

void OpenMainMenu(void) { PlayMenuMusic(); Sys_Global.menuActive = true; currentMenuPage = MenuPages_FrontPage; }

extern bool mouseMovementThisFrame;
bool MenuEnter(void) { return (Sys_Input.keyStates[GLFW_KEY_KP_ENTER].pressed || Sys_Input.keyStates[GLFW_KEY_ENTER].pressed || Sys_Input.gamepadButtons[GLFW_GAMEPAD_BUTTON_A].pressed); }
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
    RenderUIImage(pX,pY, 40,40, over ? MENUPAD_HILITE : MENUPAD); // Menu pad
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
    if (currentMenuPage == MenuPages_Singleplayer || currentMenuPage == MenuPages_Multiplayer || currentMenuPage == MenuPages_Options) currentMenuPage = MenuPages_FrontPage;//News
    else if (currentMenuPage == MenuPages_Load || currentMenuPage == MenuPages_NewGame || currentMenuPage == MenuPages_IntroVideo || currentMenuPage == MenuPages_CreditsVideo) currentMenuPage = MenuPages_Singleplayer;
}

void ChangeMenuPage(MenuPages pg) { currentMenuPage = pg; currentMenuItem = currentMenuTab = 0; }

void RenderMenu(void) {
    if (Sys_Input.gamepadButtons[GLFW_GAMEPAD_BUTTON_B].pressed && currentMenuPage != MenuPages_FrontPage) { MenuGoBack(); return; }
    
    if (currentMenuPage != MenuPages_IntroVideo && currentMenuPage != MenuPages_CreditsVideo && currentMenuPage != MenuPages_Options) RenderUIImage(-417,-384, 2200,1536, 1026); // Menu background
    if (currentMenuPage == MenuPages_IntroVideo || currentMenuPage == MenuPages_CreditsVideo) RenderUIImage(-417,-384, 2200,1536, 0); // Video blackground
    if (currentMenuPage == MenuPages_Options) RenderUIImage(-417,-384, 2200,1536, 1032); // Menu background
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
                if (resDropdownCount > 0) StringFormat(resBuf, sizeof(resBuf), "%ux%u %uHz",Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight,(u32)resModes[resSelectedIdx].hz);
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
                        StringFormat(itemBuf,sizeof(itemBuf),"%dx%d %dHz",resModes[i].w,resModes[i].h,resModes[i].hz);
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
                            glfwSetWindowPos(window, xpos, ypos);
                            UpdateScreenSize(NULL, (int)Sys_Settings.ScreenWidth,(int)Sys_Settings.ScreenHeight);
                            Sys_Input.ignore_next_mouse_delta = true;
                            resDropdownOpen = false;
                            SaveConfig();
                            GatherResolutionModes();
                            break;
                        }
                    }

                    // Arrow key navigation within dropdown
                    if (Sys_Input.keyStates[GLFW_KEY_UP].pressed && currentMenuItem >= 10 && currentMenuItem > 10)
                        currentMenuItem--;
                    else if (Sys_Input.keyStates[GLFW_KEY_UP].pressed && currentMenuItem == 10)
                        currentMenuItem = (i8)(10 + resDropdownCount - 1);
                    if (Sys_Input.keyStates[GLFW_KEY_DOWN].pressed && currentMenuItem >= 10 && currentMenuItem < (i8)(10 + resDropdownCount - 1))
                        currentMenuItem++;
                    else if (Sys_Input.keyStates[GLFW_KEY_DOWN].pressed && currentMenuItem == (i8)(10 + resDropdownCount - 1))
                        currentMenuItem = 10;

                    // Escape or click outside closes
                    if (Sys_Input.keyStates[GLFW_KEY_ESCAPE].pressed)
                        resDropdownOpen = false;
                    if (Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_LEFT].pressed
                        && currentMenuItem < 10 && currentMenuItem != 7)
                        resDropdownOpen = false;
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
            if (UI_Slider(426,240,128,16,((Sys_Settings.VolumeMaster / 100.0f) * (128 - 16)),200,Sys_Settings.VolumeMaster,&newVal,&masterVolumeSliderActive,0,100,5,0,/*Master Volume*/802)) { Sys_Settings.VolumeMaster = newVal; set_master_volume(); if (!AnyLeftRightMouseDown()) {SaveConfig();} }
            if (UI_Slider(426,270,128,16,((Sys_Settings.VolumeMusic / 100.0f) * (128 - 16)),200,Sys_Settings.VolumeMusic,&newVal,&musicVolumeSliderActive,0,100,5,1,/*Music Volume*/803)) { Sys_Settings.VolumeMusic = newVal; set_master_volume(); if (!AnyLeftRightMouseDown()) {SaveConfig();} }
        }
        
        RenderUIImage(1087,723, 84,36, 1252); // Back Button background
        i8 lastItem = menuItemCount - 1;
        bool overBack = false;
        if (UI_Button(1087,757, 84,32, &overBack, lastItem) || (MenuEnter() && currentMenuItem == lastItem)) MenuGoBack();
        overBack = overBack || currentMenuItem == lastItem;
        RenderFormattedText(1103,731,overBack ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_RED_MENU,FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == MenuPages_Load || currentMenuPage == MenuPages_Save) {
        menuItemCount = 9; menuTabCount = 1;
        bool isSave = currentMenuPage == MenuPages_Save;
        UI_HeaderText(isSave ? 284 : 340, isSave ? /*"SAVE GAME"*/Sys_Text.stringTable[769] : /*"LOAD"*/Sys_Text.stringTable[726]);
        RenderUIImage(400,214, 586,500, 1037); // Load/Save table background
        RenderUIImage(1060,724, 84,36, 1252); // Back Button background
        bool overBack = false;
        if (UI_Button(1060,758, 84,32, &overBack, 0) || (MenuEnter() && currentMenuItem == 0)) MenuGoBack();
        overBack = overBack || currentMenuItem == 0;
        RenderFormattedText(1076,732, overBack ? TEXT_STOPD_RED_HIGHLIGHT : TEXT_RED_MENU, FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == MenuPages_NewGame) {
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
    } else if (currentMenuPage == MenuPages_IntroVideo) {
        menuItemCount = menuTabCount = 1;
        if (MenuEnter()) MenuGoBack();
    } else if (currentMenuPage == MenuPages_CreditsVideo) {
        menuItemCount = menuTabCount = 1;
        if (MenuEnter()) MenuGoBack();
    }
    
    if (menuTabCount <= currentMenuTab) currentMenuTab = 0;
    if (menuItemCount <= currentMenuItem) currentMenuItem = 0;
    static const i8 ngSwap[7] = {0,3,4,1,2,6,5};
    if (Sys_Input.keyStates[GLFW_KEY_RIGHT].pressed || Sys_Input.keyStates[GLFW_KEY_LEFT].pressed) {
        int dir = Sys_Input.keyStates[GLFW_KEY_RIGHT].pressed ? 1 : -1;
        currentMenuTab = (currentMenuTab + menuTabCount + dir) % menuTabCount;
        if (currentMenuPage == MenuPages_NewGame && currentMenuItem < 7) currentMenuItem = ngSwap[currentMenuItem];
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
    glDrawArrays(GL_LINES,0,Sys_Global.debugLineVertCount / 3);
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
    u32 finalSubscore = GetScore(stupid,false);
    off += StringFormat(creditStats + off,sizeof(creditStats),"Kills: %u\nKills in Cyberspace: %u\nScoreSubtotal: %u\nDeaths: %u\nRessurections: %u\n",Sys_Global.kills,Sys_Global.cyberkills,(u32)finalSubscore,Sys_Global.deaths,Sys_Global.ressurections);
    off += StringFormat(creditStats + off,sizeof(creditStats),"Combat: %u | Puzzle: %u | Mission: %u | Cyber: %u\n",Sys_Global.difficultyCombat,Sys_Global.difficultyPuzzle,Sys_Global.difficultyMission,Sys_Global.difficultyCyber);
    u32 finalScore = (u32)GetScore(stupid,true);
    off += StringFormat(creditStats + off,sizeof(creditStats),"Difficulty Index: %.2f\nFinal Score: %u\n\n",stupid,finalScore);
    off += StringFormat(creditStats + off,sizeof(creditStats),"Shots Fired: %u\nGrenades Thrown: %u\n",Sys_Global.shotsFired,Sys_Global.grenadesThrown);
    off += StringFormat(creditStats + off,sizeof(creditStats),"Damage Dealt: %f\nDamage Received: %f\nSaves Scummed: %u\n\nClick to continue...\n",Sys_Global.damageDealt,Sys_Global.damageReceived,Sys_Global.savesScummed);
}

u8 MFD_LefTab=0,MFD_CenterTab=0,MFD_RightTab=0;
extern u16 playerCellIdx; extern char consoleEntryText[TEXT_BUFFER_SIZE]; void RenderMenu(void); void RenderPausedUI(void);
static inline __attribute__((always_inline)) double RenderUI(void) {
    glBindFramebuffer(GL_FRAMEBUFFER,0);
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
        drawCallsRenderedThisFrame += 2; textDrawCallsRenderedThisFrame += 2; // Add two more for this text render ;)
        RenderFormattedText(16, debugTextStartY - lineSpacing, timingColor, FONT_NORMAL,1.0f, "ms: %.2f, CPU %.2f", Sys_Global.thisFrameTime,Sys_Global.cpuFrameTime);
        RenderFormattedText(16 + 230.0f, debugTextStartY - lineSpacing, TEXT_WHITE, FONT_NORMAL,1.0f, "(FPS:%d, Worst:%d),Drwclls:%d [G:%d UI:%d Tx:%d Sh:%d] Vrt:%d E:%u|M:%u|P:%u|T:%.5f",Sys_Global.framesPerLastSecond,Sys_Global.worstFPS,drawCallsRenderedThisFrame,drawCallsNormal,uiImageDrawCallsRenderedThisFrame,textDrawCallsRenderedThisFrame,shadowDrawCallsRenderedThisFrame,verticesRenderedThisFrame,Sys_Cheats.editMode,Sys_Global.menuActive,Sys_Global.gamePaused,Sys_Global.pauseRelativeTime);
    }
    
    return time_now;
}

#define SHADOW_NEARMESH_MAX 1024
typedef struct {float depth; u16 index; } DepthSort;
DepthSort shadows_nearMeshes[SHADOW_NEARMESH_MAX];
float shadows_nearMeshRadii[SHADOW_NEARMESH_MAX];
static inline __attribute__((always_inline)) bool EntNotVisible(u16 i, bool otherCondition) { Entity* e = &Sys_Global.instances[i]; return e->texIndex > loadedTexturesMaxIndex || !(e->entflags & ENTFLAG_ACTIVE) || e->index >= MAX_ENTITIES || e->modelIndex >= MODEL_IDX_MAX || e->texIndex >= MAX_VALID_TEXTURE || otherCondition; }

extern bool instanceIsLODArray[INSTANCE_COUNT]; extern u16 loadedModelsMaxIndex; extern float modelBounds[MODEL_IDX_MAX]; extern u8** modelVertices; extern u16** modelTriangles;
static inline __attribute__((always_inline,hot)) u16 GetAndBindModel(u16 i, u16 currentModelType) {
    glUniform1ui(0,i);
    u16 modelType = (instanceIsLODArray[i] || Sys_Settings.ModelDetail < 1u) && Sys_Global.instances[i].lodIndex < loadedModelsMaxIndex ? Sys_Global.instances[i].lodIndex : Sys_Global.instances[i].modelIndex;
    if (currentModelType == modelType && currentModelType != 0) return currentModelType;
    
    glBindVertexBuffer(0,Sys_Render.vbos[modelType],0,VERTEX_ATTRIBUTES_SIZE);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,Sys_Render.tbos[modelType]);
    return modelType;
}

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
            glUniform1ui(0,c);
            GLuint groupX_shadClear = ((SHADOW_MAP_SIZE * SHADOW_MAP_SIZE) + 31) / 32;
            glDispatchCompute(groupX_shadClear,6,1);
        }

        shadowDrawCallsRenderedThisFrame = 0;
        glViewport(0,0,SHADOW_MAP_SIZE,SHADOW_MAP_SIZE);
        glUseProgram(Sys_Render.shadowmapsShaderProgram);
        u32 shadowmapOffsetHead = 0U;
        u16 shadowCasterIndices[SHADOW_NEARMESH_MAX * MAX_SHADOWMAPS];
        u32 numShadowCasters = 0;
        for (int i=START_INDEX_LEVEL_INSTANCES;i<INSTANCE_COUNT;++i) {
            if (EntNotVisible(i,(Sys_Global.instances[i].entflags & ENTFLAG_NO_SHADOWS))) continue;

            shadowCasterIndices[numShadowCasters] = i;
            numShadowCasters++;
            if (numShadowCasters >= (SHADOW_NEARMESH_MAX * MAX_SHADOWMAPS)) break; // Ran out of shadowcasters max for frame.
        }
        
        u16 shadowMapIdx=0,currentModelType=0,currentTexIndex=0; bool currentIsTransparent=0;
        bool useDetail = Sys_Settings.ModelDetail;
        for (u32 c = 0; c < numShadowsCouldRender; ++c, ++shadowMapIdx) { // Render top MAX_SHADOWMAPS candidates
            u16 lightIdx = candidates[c];
            float effectiveRadius = vmin(lights[lightIdx].range,15.36f);
            u16 nearbyMeshCount = 0;
            Vector3 lpos = lights[lightIdx].pos;
            float cellCenterX = vround(lpos.x / CELL_SIZE) * CELL_SIZE;
            float cellCenterZ = vround(lpos.z / CELL_SIZE) * CELL_SIZE;
            Vector3 deltaCellCenter = Vector3_A_minus_B((Vector3){lpos.x,0.0f,lpos.z},(Vector3){cellCenterX,0.0f,cellCenterZ});
            float distToCenterSqrd = dot_vector3(deltaCellCenter,deltaCellCenter);
            bool skipNPCs = (distToCenterSqrd < 0.4096f); // 0.64 * 0.64
            for (u16 shadowCasterInstanceIdx = 0; shadowCasterInstanceIdx < numShadowCasters; shadowCasterInstanceIdx++) {
                u16 j = shadowCasterIndices[shadowCasterInstanceIdx];
                Entity* e = &Sys_Global.instances[j];
                shadows_nearMeshRadii[nearbyMeshCount] = modelBounds[e->modelIndex] * 0.99f * vmax(vmax(e->scale.x,e->scale.y),e->scale.z);
                Vector3 d = Vector3_A_minus_B(e->position,lpos);
                float distToLightSqrd = dot_vector3(d,d);
                float radSum = (effectiveRadius + shadows_nearMeshRadii[nearbyMeshCount]);
                if (distToLightSqrd >= radSum * radSum) continue;
                if (skipNPCs && ConstIndexIsNPC(e->index)) continue;
                
                shadows_nearMeshes[nearbyMeshCount].index = j;
                shadows_nearMeshes[nearbyMeshCount].depth = distToLightSqrd; 
                nearbyMeshCount++;
                if (nearbyMeshCount >= SHADOW_NEARMESH_MAX) { DualLogWarn("Shadowmapping needs larger nearMeshes count than %u!  Skipping some renderables for light %u!\n", SHADOW_NEARMESH_MAX, lightIdx); break; }
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
                    if (!SphereInFrustum(lightFrustumPlanes[lightIdx][face],e->position,shadows_nearMeshRadii[j] * 1.41f)) continue;

                    glUniform1ui(0,i);
                    u16 modelType = (instanceIsLODArray[i] || useDetail < 1u) && e->lodIndex < loadedModelsMaxIndex ? e->lodIndex : e->modelIndex;
                    if (currentModelType != modelType || currentModelType == 0) { currentModelType = modelType; glBindVertexBuffer(0,Sys_Render.vbos[modelType],0,VERTEX_ATTRIBUTES_SIZE); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,Sys_Render.tbos[modelType]); }
                    if (currentTexIndex != e->texIndex) { currentTexIndex = e->texIndex; glUniform1ui(6,e->texIndex); }
                    bool texIsTransparent = transparentTexture[e->texIndex];
                    if (currentIsTransparent != texIsTransparent) { currentIsTransparent = texIsTransparent; glUniform1ui(8,currentIsTransparent ? 1u : 0u); }
                    glDrawElements(GL_TRIANGLES,modelTriangleCounts[currentModelType]*3,GL_UNSIGNED_SHORT,0); drawCallsRenderedThisFrame++; shadowDrawCallsRenderedThisFrame++; verticesRenderedThisFrame += modelTriangleCounts[currentModelType] * 3;
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
    u16 cellX = PosGetCellCoordX(e->position.x), cellZ = PosGetCellCoordZ(e->position.z);
    u16 instCellIdx = (cellZ * WORLDX) + cellX; u16 entIdx = e->index;
    Vector3 delta = Vector3_A_minus_B(e->position,playerPos);
    *distSqrd = delta.x*delta.x + delta.y*delta.y + delta.z*delta.z;
    float radius = modelBounds[e->modelIndex] * 2.0f * vmax(vmax(e->scale.x,e->scale.y),e->scale.z);
    if (!SphereInFrustum(playerFrustumPlanes,e->position,radius) && (entIdx != 754 || !skyVisible) && i != editModeSelection) return false;
    
    if (ConstIndexIsPortalBlockingDoor(entIdx)) { // Extra checks only needed for opaque portal blocking doors.
        bool inPVS = (gridCellStates[instCellIdx] & CELL_VISIBLE);
        if (!inPVS) inPVS = NeighborhoodInPVS(cellX,cellZ,2);
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
        if(gs){float _h=(constIndex>=419&&constIndex<=448)?(( \
            constIndex==419||constIndex==422||constIndex==424||constIndex==429||constIndex==430|| \
            constIndex==431||constIndex==433||constIndex==437||constIndex==438||constIndex==441)?1.5f:4.0f):0.0f; \
            glUniform1f(31,_h);} \
    }
#define DRAW_ENTITY(i,curN,curT,curG,curS,curM) \
    {Entity*e=&Sys_Global.instances[i];u16 tex=e->texIndex,glow=e->glowIndex,norm=e->normIndex,spec=e->specIndex; \
     u32 ci=e->index; \
     glUniform1ui(17,tex==316?1u:0u); \
     glUniform1ui(25,ci); \
     glUniform1f(27,e->volume); \
     glUniform1ui(32,(tex==36||tex==887)?1u:0u); \
     SET_CAMVIEW(e->camView,ci,grayscaleEnabled) \
     BIND_TEX(1,curN,norm) BIND_TEX(18,curT,tex) BIND_TEX(19,curG,glow) BIND_TEX(20,curS,spec) \
     curM=GetAndBindModel(i,curM); \
     u32 vc=modelTriangleCounts[curM]*3; \
     glDrawElements(GL_TRIANGLES,vc,GL_UNSIGNED_SHORT,0);drawCallsRenderedThisFrame++;verticesRenderedThisFrame+=vc;}

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
    for (int i=0;i<LIGHT_COUNT;++i) flag_setu32(&lights[i].lflags,LDIRTY,false);
    __builtin_memset(Sys_Global.dirtyInstances,0,Sys_Global.loadedInstances * sizeof(bool));
    glBindFramebuffer(GL_FRAMEBUFFER,Sys_Render.gBufferFBO);
    glViewport(0,0,swidth,sheight);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); glEnable(GL_CULL_FACE); glDisable(GL_BLEND); // Opaques
    u16 visibleCount = 0, currentTexIndex = 0, currentNormIndex = 0, currentGlowIndex = 0, currentSpecIndex = 0, currentModelType = 0, opaqueCount = 0;
    bool skyVisible = (gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX);
    float distSqrd = sfar * sfar;
    DepthSort tmpTransparent[MAX_VISIBLE]; u16 tcnt = 0;
    for (u16 i = START_INDEX_LEVEL_INSTANCES; i < INSTANCE_COUNT; ++i) { // Determine base visibility
        if (!DetermineIfInstanceVisible(i,false,skyVisible,playerPos,&distSqrd)) continue;
       
        if (transparentTexture[Sys_Global.instances[i].texIndex]) { tmpTransparent[tcnt].index = i; tmpTransparent[tcnt].depth = distSqrd; tcnt++; }
        else { visibleInstances[opaqueCount].index = i; visibleInstances[opaqueCount].depth = distSqrd; opaqueCount++; }
    }

    __builtin_memcpy(visibleInstances + opaqueCount,tmpTransparent,tcnt * sizeof(DepthSort));
    visibleCount = opaqueCount + tcnt;
    glUseProgram(Sys_Render.depthPrepassShaderProgram); // Depth Prepass - Eliminates some overdraw for ~6.1% performance improvement in spite of added draw calls
    glUniformMatrix4fv(2,1,0,viewProj);
    glColorMask(0,0,0,0); glDepthMask(1); glDepthFunc(GL_LESS); glDisable(GL_BLEND);
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
        glDrawElements(GL_TRIANGLES,vertCount,GL_UNSIGNED_SHORT,0); drawCallsRenderedThisFrame++; verticesRenderedThisFrame += vertCount;
    }

    // Main Pass
    glUseProgram(Sys_Render.chunkShaderProgram);
    glUniformMatrix4fv(2,1,0,viewProj);
    glUniform1ui(25,0u); // default constIndex
    bool grayscaleEnabled = ModRequestsGrayscale();
    glUniform1ui(26,(u32)grayscaleEnabled);
    glUniform1ui(3,0u); // isUI false
    float fogActual = Sys_Global.fogColor.a + (float)(Sys_Global.fogFac / 255u); // Alpha is base density for level.
    glUniform3f(4,Sys_Global.fogColor.r * fogActual,Sys_Global.fogColor.g * fogActual,Sys_Global.fogColor.b * fogActual); // Fog Color(which is density)
    glUniform1ui(14,Sys_Settings.Reflections); glUniform1ui(15,Sys_Settings.Shadows);
    glUniform1f(8,Sys_Global.worldMin_x); glUniform1f(9,Sys_Global.worldMin_z); glUniform3f(10,playerPos.x,playerPos.y,playerPos.z);
   
    // Opaque Pass
    glColorMask(1,1,1,1); glDepthMask(0); glDepthFunc(GL_LEQUAL);
    visibleCount = currentTexIndex = currentNormIndex = currentGlowIndex = currentSpecIndex = currentModelType = 0;
    glUniform1f(31,0.0f); // Reset heat for infrared vision
    for (u16 visibleIndex = 0; visibleIndex < opaqueCount; ++visibleIndex) { // Opaques (already front-to-back)
        u16 i = visibleInstances[visibleIndex].index;
        Entity* e = &Sys_Global.instances[i]; u16 tex = e->texIndex;
        if (transparentTexture[tex]) continue;
        else if (doubleSidedTexture[tex] || e->scale.x < 0.0f || e->scale.y < 0.0f || e->scale.z < 0.0f) { glDisable(GL_CULL_FACE); glEnable(GL_BLEND); } // Doublesided (either)
        else { glEnable(GL_CULL_FACE); glDisable(GL_BLEND); } // Opaque
       
        DRAW_ENTITY(i, currentNormIndex, currentTexIndex, currentGlowIndex, currentSpecIndex, currentModelType)
    }
   
    // Transparents Pass
    glDepthMask(1);
    currentTexIndex = currentNormIndex = currentGlowIndex = currentSpecIndex = currentModelType = 0;
    glUniform1f(31,0.0f); // Reset heat for infrared vision
    for (u16 visibleIndex = opaqueCount; visibleIndex < (opaqueCount + tcnt); ++visibleIndex) {
        u16 i = visibleInstances[visibleIndex].index;
        Entity* e = &Sys_Global.instances[i]; u16 tex = e->texIndex;
        if (transparentTexture[tex]) { glEnable(GL_CULL_FACE); glEnable(GL_BLEND); } // Transparents (with sort)
        else if (doubleSidedTexture[tex] || e->scale.x < 0.0f || e->scale.y < 0.0f || e->scale.z < 0.0f) { glDisable(GL_CULL_FACE); glEnable(GL_BLEND); } // Doublesided (either)
        else continue; // Opaque
       
        u32 constIndex = e->index;
        if ((constIndex >= 561 && constIndex <= 565) || (constIndex >= 568 && constIndex <= 573)) glDepthFunc(GL_EQUAL); // Cutouts
        else glDepthFunc(GL_LEQUAL); // Actual alphas
       
        DRAW_ENTITY(i,currentNormIndex,currentTexIndex,currentGlowIndex,currentSpecIndex,currentModelType)
    }
   
    glUniform1ui(25,0u); // reset constIndex
    if (camView) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER,Sys_Render.gBufferFBO);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glBindTexture(GL_TEXTURE_2D,camViewTextures[camViewIdx]);
        glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0,0,0,swidth,sheight);
        glBindTexture(GL_TEXTURE_2D,0);
        return; // After copying render result, skip SSR and composite for camviews <<<<<<<<<<<<< CAM VIEW BARRIER
    }
   
    // Draw Debug Lines
    if (unlikely(Sys_Global.debugLineVertCount > 1)) DrawDebugLines(viewProj);
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    if (likely(Sys_Settings.Reflections > 0u)) { // Screen Space Reflections
        glUseProgram(Sys_Render.ssrShaderProgram);
        glUniform3f(3,playerPos.x,playerPos.y,playerPos.z);
        glUniformMatrix4fv(4,1,GL_FALSE,viewProj);
        GLuint groupX_ssr = ((Sys_Settings.ScreenWidth / Sys_Settings.SSR_RES) + 31) / 32, groupY_ssr = ((Sys_Settings.ScreenHeight / Sys_Settings.SSR_RES) + 31) / 32;
        glDispatchCompute(groupX_ssr,groupY_ssr,1);
    }
    glUseProgram(Sys_Render.imageBlitShaderProgram);
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D,Sys_Render.inputImageID);
    glUniform1i(4,4); // outputImage texture sampler2D, don't remember why when active texture is texture 0. meh.... oh maybe to not read and write same binding?
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
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    drawCallsRenderedThisFrame++; verticesRenderedThisFrame += 4;
    
    // UI
    glEnable(GL_BLEND); glDisable(GL_CULL_FACE); glClear(GL_DEPTH_BUFFER_BIT); // Clear main FBO. glClearBufferfv was actually SLOWER! 2nd Clear needed or UI dissappears/flickers!!
    Sys_Global.last_time = RenderUI();
    // Cursor [ /// VERY LAST DRAWN OVER EVERYTHING ELSE! /// ]
    if ((Sys_Global.inventoryMode && !Sys_Cheats.noHUD) || Sys_Global.menuActive || Sys_Global.gamePaused) RenderUIImage((i16)(Sys_Global.cursorPosition_x) - 20,(i16)(Sys_Global.cursorPosition_y) - 20,40,40,GetCursorTexture());
    else RenderUIImage(663,371,40,40,GetCursorTexture()); // Centered on UI baseline resolution 1366x768
    if ((Sys_Global.last_time - Sys_Global.lastFrameSecCountTime) >= 1.00) { // Update Diagnostic Poll
        Sys_Global.lastFrameSecCountTime = Sys_Global.last_time;
        Sys_Global.framesPerLastSecond = Sys_Global.globalFrameNum - Sys_Global.lastFrameSecCount;
        if (Sys_Global.framesPerLastSecond < Sys_Global.worstFPS && Sys_Global.globalFrameNum > 2000) Sys_Global.worstFPS = Sys_Global.framesPerLastSecond; // After startup, keep track of worst framerate seen.
        Sys_Global.lastFrameSecCount = Sys_Global.globalFrameNum;
    }
}

unsigned char *stbi_load_from_memory(const u8* buffer, i32 len, i32 *x, i32 *y);
extern StbiArena stbi_arena_main; extern u32 random_range_rng;
void stbi__arena_init_thread(StbiArena* arena);
#define STBI_ARENA_SIZE 16 * 1024 * 1024
#define LIGHT_RANGE_MAX 15.36f
void InitFontAtlasses(void),LoadTextures(void),LoadModels(void); i32 Physics(void); bool CullCore(void);
i32 main(void) {
    double game_start_time = get_time();
    random_range_rng = (u32)game_start_time; // Seed global rand uniquely with time since system boot.
    console_log_file = OS_OpenWriteonly("./voxen.log"); // Initialize log system for all prints to go to both stdout and voxen.log file
    DebugRAM("program start");
    DualLog("Voxen, the Voxel Lit Open Source Game Engine by W. Josiah Jack, MIT-0 licensed\n");
    if (!glfwInit()) { DualLogError("GLFW initialization failed\n"); OS_Exit(1); }
    
    LoadConfig(); // Get settings before setting window size.
    window = glfwCreateWindow(Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight,"Voxen",NULL,NULL); if (!window) { DualLogError("glfwCreateWindow failed\n"); OS_Exit(1); }
    
    CenterWindowOnMonitor();
    glfwMakeContextCurrent(window); glfwSetFramebufferSizeCallback(window,UpdateScreenSize);
    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) { DualLogError("Failed to initialize GLAD\n"); OS_Exit(1); }
    
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); // Erase the corner where last shadowmap wrote into
    glfwSwapBuffers(window);
    GLint major=0,minor=0; glGetIntegerv(GL_MAJOR_VERSION,&major); glGetIntegerv(GL_MINOR_VERSION,&minor);
    if (major < 4 || (major == 4 && minor < 3)) { DualLogError("Need OpenGL >= 4.3, got %d.%d\n",major,minor); OS_Exit(1); }
    glfwSetKeyCallback(window,key_callback); glfwSetJoystickCallback(joystick_callback);
    glfwSetCursorPosCallback(window,cursor_pos_callback); glfwSetWindowFocusCallback(window,window_focus_callback);
    glfwSetMouseButtonCallback(window,mouse_button_callback); glfwSetScrollCallback(window,scroll_callback);
    glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);
    glFrontFace(GL_CCW); // Set triangle sorting order (GL_CW vs GL_CCW)
    glBlendFuncSeparate(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_ZERO,GL_ONE);
    CompileShaders();
    GLuint vaos[4],vbos[4]; glCreateVertexArrays(4,vaos); glCreateBuffers(3,vbos);
    Sys_Render.quadVAO = vaos[0]; Sys_Render.vao_chunk = vaos[1]; Sys_Render.textVAO = vaos[2]; Sys_Render.debugLinesVAO = vaos[3];
    Sys_Render.quadVBO = vbos[0]; Sys_Render.textVBO = vbos[1]; Sys_Render.debugLinesVBO = vbos[2];
    float quadBlit_vertices[] = {1.0f,-1.0f,1.0f,0.0f, 1.0f,1.0f,1.0f,1.0f, -1.0f,1.0f,0.0f,1.0f, -1.0f,-1.0f,0.0f,0.0f}; // 4 verts, 4 floats each x,y,u,v
    glNamedBufferData(Sys_Render.quadVBO,sizeof(quadBlit_vertices),quadBlit_vertices,GL_STATIC_DRAW);
    glVertexArrayAttribFormat(Sys_Render.quadVAO,0,2,GL_FLOAT,GL_FALSE,0); // DSA: Set position format
    glVertexArrayAttribFormat(Sys_Render.quadVAO,1,2,GL_FLOAT,GL_FALSE,2 * sizeof(float)); // DSA: Set texcoord format
    glVertexArrayVertexBuffer(Sys_Render.quadVAO,0,Sys_Render.quadVBO,0,4 * sizeof(float)); // DSA: Link VBO to VAO
    for (u8 i = 0; i < 2; i++) { glVertexArrayAttribBinding(Sys_Render.quadVAO,i,0); glEnableVertexArrayAttrib(Sys_Render.quadVAO,i); }
    glVertexArrayAttribFormat(Sys_Render.vao_chunk,0,3,GL_HALF_FLOAT,GL_FALSE,0);      // pos xyz half-float @ offset 0
    glVertexArrayAttribFormat(Sys_Render.vao_chunk,1,3,GL_HALF_FLOAT,GL_FALSE,6);      // normal xyz float   @ offset 6  (after 3×2 bytes)
    glVertexArrayAttribFormat(Sys_Render.vao_chunk,2,2,GL_HALF_FLOAT,GL_FALSE,12);     // uv st float
    for (u8 i = 0; i < 3; i++) { glVertexArrayAttribBinding(Sys_Render.vao_chunk,i,0); glEnableVertexArrayAttrib(Sys_Render.vao_chunk,i); }
    glVertexArrayAttribFormat(Sys_Render.textVAO,0,3,GL_FLOAT,GL_FALSE,0);             // pos (x,y,z) 4 floats per vertex, stride = 4*sizeof(float)
    glVertexArrayAttribFormat(Sys_Render.textVAO,1,2,GL_FLOAT,GL_FALSE,3 * sizeof(float));  // uv (s,t)
    glVertexArrayVertexBuffer(Sys_Render.textVAO,0,Sys_Render.textVBO,0,5 * sizeof(float));
    for (u8 i = 0; i < 2; i++) { glVertexArrayAttribBinding(Sys_Render.textVAO,i,0); glEnableVertexArrayAttrib(Sys_Render.textVAO,i); }
    InitFontAtlasses();
    RenderLoadingProgress(80,"Loading...");
    glNamedBufferStorage(Sys_Render.debugLinesVBO,MAX_DEBUG_LINE_VERTS * 3 * sizeof(float),NULL,GL_DYNAMIC_STORAGE_BIT);
    glVertexArrayAttribFormat(Sys_Render.debugLinesVAO,0,3,GL_FLOAT,GL_FALSE,0);
    glEnableVertexArrayAttrib(Sys_Render.debugLinesVAO,0);
    glVertexArrayAttribBinding(Sys_Render.debugLinesVAO,0,0);
    glVertexArrayVertexBuffer(Sys_Render.debugLinesVAO,0,Sys_Render.debugLinesVBO,0,3 * sizeof(float));
    float* m = shadowmapsPerspectiveProjection; float viewRange = (LIGHT_RANGE_MAX - NEAR_PLANE);
    m[0] = 1.0f; m[1] = 0.0f; m[2] =                                             0.0f; m[3] =  0.0f;
    m[4] = 0.0f; m[5] = 1.0f; m[6] =                                             0.0f; m[7] =  0.0f;
    m[8] = 0.0f; m[9] = 0.0f; m[10]=      -(LIGHT_RANGE_MAX + NEAR_PLANE) / viewRange; m[11]= -1.0f;
    m[12]= 0.0f; m[13]= 0.0f; m[14]= -2.0f * LIGHT_RANGE_MAX * NEAR_PLANE / viewRange; m[15]=  0.0f;
    ma_result result;
    ma_engine_config engine_config = ma_engine_config_init();
    engine_config.channels = 2; engine_config.periodSizeInMilliseconds = 10; engine_config.periodSizeInFrames = 512;
    result = ma_engine_init(&engine_config, &audio_engine); if (result != MA_SUCCESS) DualLog("ERROR: Failed to initialize miniaudio engine: %d\n", result);
    DualLog("Loading game definition...");
    OsFileHandle gmFP = OS_OpenReadonly("./Data/gamedata.txt");
    if (!gmFP) { DualLogError("\nCannot open ./Data/gamedata.txt\n"); OS_Exit(1);  }
   
    char gmLine[512]; u32 lineNum = 0;
    while (GetNextStringUpToNewlineOrEOF(gmLine,sizeof(gmLine),gmFP)) {
        lineNum++;
        char* s = data_parser_trim(gmLine); if (*s == 0 || (s[0] == '/' && s[1] == '/')) continue;
        char* colon = StringFindFirstCharWithin(s, ':'); if (!colon) continue;
        *colon = '\0'; char* key = data_parser_trim(s); char* val = data_parser_trim(colon + 1); if (*key == 0 || *val == 0) continue;

        if (StringsEqual(key, "modname")) StringCopyInto_A_From_B(Sys_Global.global_modname,val,sizeof(Sys_Global.global_modname));
        else if (StringsEqual(key, "levelcount")) Sys_Global.numLevels = parse_numberu8(val,gmLine,lineNum);
        else if (StringsEqual(key, "startlevel")) Sys_Global.startLevel = parse_numberu8(val,gmLine,lineNum);
    }
    
    OS_Close(gmFP); DualLog(" %s:: num levels: %d, start level: %d\n",Sys_Global.global_modname,Sys_Global.numLevels,Sys_Global.startLevel);
    LoadModFunctions();
    ModEntityDefinitionsInitAfterLoad();
    glGenFramebuffers(1,&Sys_Render.gBufferFBO);
    ApplySettings(); // After loading of text and game data.
    glBindFramebuffer(GL_FRAMEBUFFER,Sys_Render.gBufferFBO);
    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0,GL_COLOR_ATTACHMENT1,GL_COLOR_ATTACHMENT2,GL_COLOR_ATTACHMENT3};
    glDrawBuffers(4,drawBuffers);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) DualLogError("Framebuffer incomplete: Error code %d\n", status);
    glBindFramebuffer(GL_FRAMEBUFFER,0); // Needed to render loading progress.
    glfwSetWindowTitle(window,Sys_Global.global_modname);
    OsFileHandle fp = OS_OpenReadonly("./Textures/UI/menudot1.png");
    if (fp) {
        int windowIconFileSize = OS_FileSize(fp);
        u8* file_buffer = OS_AllocateFileBackedRAMReadonly(windowIconFileSize,fp,"./Textures/UI/menudot1.png");
        if (!file_buffer) { DualLogError("Could not open backed buffer for ./Textures/UI/menudot1.png\n"); OS_Exit(1); }
        
        OS_Close(fp); stbi__arena_init_thread(&stbi_arena_main);
        int w=1,h=1; unsigned char* pixels = stbi_load_from_memory(file_buffer,windowIconFileSize,&w,&h);
        if (!pixels) { DualLogError("Failed to load icon: ./Textures/UI/menudot1.png\n"); OS_Exit(1); }
        
        GLFWimage image = (GLFWimage){w,h,pixels};
        glfwSetWindowIcon(window,1,&image);
        OS_DeallocateRAM(file_buffer,windowIconFileSize);
        OS_DeallocateRAM(stbi_arena_main.base,STBI_ARENA_SIZE); stbi_arena_main.base = NULL;
    }

    float mat[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    __builtin_memcpy(&modelMatrices[0],mat,16 * sizeof(float)); // Null instance matrix used for UI
    Sys_Render.matricesBufferID        = SetupSSBO(&Sys_Render.matricesBufferID,        1,INSTANCE_COUNT * 16 * sizeof(float),modelMatrices, GL_STATIC_DRAW);
    Sys_Render.voxelLightListCountsID  = SetupSSBO(&Sys_Render.voxelLightListCountsID,  2,VOXEL_COUNT * sizeof(u32),NULL,GL_STATIC_DRAW);
    Sys_Render.voxelLightListsID       = SetupSSBO(&Sys_Render.voxelLightListsID,       3,VOXEL_COUNT * MAX_LIGHTS_PER_VOXEL * sizeof(u32),NULL,GL_STATIC_DRAW);
    Sys_Render.lightsID                = SetupSSBO(&Sys_Render.lightsID,                4,LIGHT_COUNT * sizeof(Light),NULL,GL_STATIC_DRAW);
    Sys_Render.shadowMapSSBO           = SetupSSBO(&Sys_Render.shadowMapSSBO,           5,(MAX_SHADOWMAPS * (SHADOW_MAP_SIZE * SHADOW_MAP_SIZE * 6U)) * sizeof(u32), NULL, GL_STATIC_DRAW);    
    Sys_Render.shadowMapsIndirectionID = SetupSSBO(&Sys_Render.shadowMapsIndirectionID, 6,LIGHT_COUNT * sizeof(u32),NULL,GL_STATIC_DRAW);
    Sys_Render.cellVisibleDataID       = SetupSSBO(&Sys_Render.cellVisibleDataID,       7,ARRSIZE * sizeof(u32),NULL,GL_STATIC_DRAW);
    Sys_Render.colorBufferID           = SetupSSBO(&Sys_Render.colorBufferID,          12,MAX_TOTAL_PIXELS * sizeof(u8),NULL,GL_STATIC_DRAW);
    Sys_Render.blueNoiseBuffer         = SetupSSBO(&Sys_Render.blueNoiseBuffer,        13,12288 * sizeof(float), blueNoise,GL_STATIC_DRAW);
    Sys_Render.textureOffsetsID        = SetupSSBO(&Sys_Render.textureOffsetsID,       14,MAX_VALID_TEXTURE * sizeof(u32),NULL,GL_STATIC_DRAW);
    Sys_Render.textureSizesID          = SetupSSBO(&Sys_Render.textureSizesID,         15,MAX_VALID_TEXTURE * 2 * sizeof(i32),NULL, GL_STATIC_DRAW);
    Sys_Render.texturePalettesID       = SetupSSBO(&Sys_Render.texturePalettesID,      16,MAX_UNIQUE_COLORS * sizeof(u32),NULL,GL_STATIC_DRAW);
    Sys_Render.texturePaletteOffsetsID = SetupSSBO(&Sys_Render.texturePaletteOffsetsID,17,MAX_VALID_TEXTURE * sizeof(u32),NULL,GL_STATIC_DRAW);
    glUseProgram(Sys_Render.shadowmapsShaderProgram); glUniform1ui(9,SHADOW_MAP_SIZE);
    glUseProgram(Sys_Render.shadowmapsClearShaderProgram); glUniform1ui(1,SHADOW_MAP_SIZE);
    glUseProgram(Sys_Render.chunkShaderProgram); glUniform1ui(21,SHADOW_MAP_SIZE); glUniform1f(22,(float)SHADOW_MAP_SIZE); glUniform1ui(23,LIGHT_COUNT); glUniform1ui(24,(u32)MAX_LIGHTS_PER_VOXEL); glUniform1ui(11,SHADOW_MAP_SIZE*SHADOW_MAP_SIZE);
    glUseProgram(Sys_Render.voxelUpdateShaderProgram); glUniform1ui(6,(u32)MAX_LIGHTS_PER_VOXEL);
    RenderLoadingProgress(110,"Loading textures...");
    LoadTextures();
    RenderLoadingProgress(110,"Loading models...");
    LoadModels();
    if (Sys_Global.introNotPlayed) {} // TODO: Play intro
    Sys_Global.absoluteTime = Sys_Global.last_topframe_time = Sys_Global.current_time = get_time();
    Sys_Global.pauseRelativeTime = Sys_Global.last_physics_time = 0.0;
//     NewGame(); // Almost works, just causes GL errors once entering game and SSR doesn't appear to work.  Needed to fix bug where you can't see options take effect on config menu unless returned to from after starting a game.
    OpenMainMenu();
    DebugRAM("InitializeEnvironment end");
    DualLog("Game Initialized in %f secs\n",get_time() - game_start_time);
    while(1) { // Main Loop
        if (glfwWindowShouldClose(window)) OS_Exit(0);
        if (queuedLevelToLoad != 255u) { LoadLevel(queuedLevelToLoad); queuedLevelToLoad = 255u; continue; }

        drawCallsRenderedThisFrame = textDrawCallsRenderedThisFrame = uiImageDrawCallsRenderedThisFrame = shadowDrawCallsRenderedThisFrame = verticesRenderedThisFrame = 0; // Reset per frame
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
            UpdateMusic();
        }

        if (likely(!Sys_Global.gamePaused) && camViewCount > 0) { // Render in-world camera views.  Pops player elsewhere, renders to tiny fbo, pops player back, renders as normal below.
            Vector3 tempPlayerPos = Sys_Global.instances[PLAYER1].position;
            Quaternion tempPlayerRot = Sys_Global.instances[PLAYER1].rotation;
            for (int cm=0;cm<camViewCount;++cm) {
                if (camViews[cm].finished < Sys_Global.pauseRelativeTime && camViews[cm].visible) {
                    camViews[cm].finished = Sys_Global.pauseRelativeTime + 0.5f;
                    Sys_Global.instances[PLAYER1].position = camViews[cm].position;
                    Sys_Global.instances[PLAYER1].rotation = camViews[cm].rotation;
                    CullCore();
                    UpdateLights();
                    Render(true,cm); // Ok culling and light clusters (in voxels) have been updated, now render the view.
                }
            }

            Sys_Global.instances[PLAYER1].position = tempPlayerPos; // Restore player for normal render.
            Sys_Global.instances[PLAYER1].rotation = tempPlayerRot;
        }
        
        if (likely(!Sys_Global.gamePaused || Sys_Global.menuActive)) {
            CullCore();
            UpdateLights();
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

        Render(false,0u); // Not a cam view, no camview index.  This is the normal main render.
        CheckAndTakeScreenshot();
        Sys_Global.globalFrameNum++;
        InputClearRisingAndFallingEdges();
        Sys_Input.currentMouse_dx = Sys_Input.currentMouse_dy = 0;
        Sys_Global.cpuTime = get_time() - Sys_Global.current_time; // Measure time over everything this frame before GPU swap buffers
        glfwSwapBuffers(window); // Present frame
        CHECK_GL_ERROR();
        #ifdef DEBUG_RAM_OUTPUT
            static const u32 dbgFrames[] = {4,100,200,500,1000};
            static const char*    dbgLabels[] = {"after 4 frames","after 100 frames","after 200 frames","after 500 frames","after 1000 frames"};
            for (int _d=0;_d<5;_d++) if (Sys_Global.globalFrameNum == dbgFrames[_d]) { DebugRAM(dbgLabels[_d]); break; }
        #endif
    }
    return 0;
}
