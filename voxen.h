// voxen.h - Engine specific shared values
#pragma once
#if defined(LINUX)
//     #define DEBUG_RAM_OUTPUT // Debug and Compile Flags
#endif
#include "common.h" // Types needed first
#include "interop.h"
#define MAX_KEYS 512
#define MAX_MOUSE_BUTTONS 8
#define VERTEX_ATTRIBUTES_SIZE 16 // Was 32
#define TEXT_BUFFER_SIZE 1024
#define FONT_ATLAS_SIZE 4672
#define MAX_GLYPHS 4096
typedef struct {bool down,pressed,released;} KeyState;
typedef struct {
	double last_mouse_x,last_mouse_y,scrollDelta;
	KeyState keyStates[MAX_KEYS],mouseButtons[MAX_MOUSE_BUTTONS],joystickButtons[16][16],joystickHats[5]; // What can I say, I'm a man of many hats. ^^D
    i32 currentMouse_dx,currentMouse_dy;
	bool window_has_focus,ignore_next_mouse_delta,lastUse,isCapsLockOn,joystickPresent[16];
} InputSystem;
extern InputSystem Sys_Input; extern GlobalContext Sys_Global; extern SystemUI Sys_UI;
typedef struct { Vector3 normal; float d; } FrustumPlane;
typedef struct StbiArena { u8*base,*cursor,*end; } StbiArena;
typedef u32 GLuint;
typedef struct {
    GLuint inputImageID,inputUIID,inputDepthID,inputWorldPosID,inputSpecID,inputNormalID,gBufferFBO,uiFBO,outputImageID;
    GLuint depthPrepassShaderProgram,chunkShaderProgram,vao_chunk,uiShaderProgram,debugUnlitShaderProgram;
    GLuint shadowmapsShaderProgram,shadowmapsClearShaderProgram,shadowMapSSBO,shadowMapsIndirectionID;
    GLuint ssrShaderProgram,imageBlitShaderProgram,quadVAO,quadVBO,textShaderProgram,textVAO,textVBO;
    GLuint debugLinesVAO,debugLinesVBO,matricesBufferID,cellVisibleDataID;
    GLuint colorBufferID,texturePalettesID,texturePaletteOffsetsID,textureOffsetsID,textureSizesID;
    GLuint lightsID,voxelLightListCountsID,voxelLightListsID,voxelUpdateShaderProgram,shadowViewProjID;
    GLuint vbos[MODEL_IDX_MAX],tbos[MODEL_IDX_MAX];
} RenderSystem;
size_t GetStringLength(const char *s);
void StringCopyInto_A_SubstringFrom_B(char* a, size_t substringSize, const char* b, size_t bufferSize);
extern RenderSystem Sys_Render; // Added last to make use of all defines for sizes.
int StringCompareUpToLength(const char* s1, const char* s2, size_t n);
static inline __attribute__((always_inline)) i32 PosGetCellCoordX(float pos_x) { return (u16)clamp((i32)vfloor((pos_x - Sys_Global.worldMin_x + CELLXHALF) / CELL_SIZE), 0, WORLDX_0BASED); }
static inline __attribute__((always_inline)) i32 PosGetCellCoordZ(float pos_z) { return (u16)clamp((i32)vfloor((pos_z - Sys_Global.worldMin_z + CELLXHALF) / CELL_SIZE), 0, WORLDX_0BASED); }
