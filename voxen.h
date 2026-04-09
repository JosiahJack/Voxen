// voxen.h - Engine specific shared values
#pragma once
#if defined(LINUX)
//     #define DEBUG_RAM_OUTPUT // Debug and Compile Flags
#endif
#include "common.h" // Types needed first
#include "interop.h"
#define MAX_KEYS 512
#define MAX_MOUSE_BUTTONS 8
#define MAX_JOYSTICK_BUTTONS 16
#define MAX_JOYSTICK_HATS 5
#define MAX_GAMEPAD_BUTTONS 20
#define MAX_SHADOWMAPS 256u
#define MAX_SAVENAME_LENGTH 24
#define MAX_PALETTE_SIZE 256
#define MAX_TEXTURE_DIMENSION 2048
#define MAX_LIGHTS_PER_VOXEL 64
#define VERTEX_ATTRIBUTES_SIZE 16 // Was 32
#define DEBUG_OPENGL
#ifdef DEBUG_OPENGL
	#define CHECK_GL_ERROR() do { GLenum err = glGetError(); if (err != GL_NO_ERROR) DualLogError("GL Error at %s:%d: %d\n", __FILE__, __LINE__, err); } while(0)
#else
	#define CHECK_GL_ERROR() do {} while(0)
#endif
#define NEAR_PLANE (0.02f)
#define TEXT_BUFFER_SIZE 1024
#define FONT_ATLAS_SIZE 4672
#define MAX_GLYPHS 4096
#define MAX_CAMVIEWS 11
extern GlobalContext Sys_Global;
extern SystemUI Sys_UI;
typedef struct { u16 x,z; } PortalCell;
typedef struct { PortalCell cellA,cellB,cellA2,cellB2; bool portalNS,open,dirty,isBulkhead;} Portal;
typedef struct {bool down,pressed,released;} KeyState;
typedef struct {
	double last_mouse_x,last_mouse_y,scrollDelta;
	KeyState keyStates[MAX_KEYS],mouseButtons[MAX_MOUSE_BUTTONS],gamepadButtons[MAX_GAMEPAD_BUTTONS],joystickButtons[16][MAX_JOYSTICK_BUTTONS],joystickHats[MAX_JOYSTICK_HATS]; // What can I say, I'm a man of many hats. ^^D
    i32 currentMouse_dx,currentMouse_dy;
	bool window_has_focus,ignore_next_mouse_delta,lastUse,isCapsLockOn,joystickPresent[16];
} InputSystem;
extern InputSystem Sys_Input;
typedef struct { Vector3 normal; float d; } FrustumPlane;
typedef struct StbiArena { u8*base,*cursor,*end; } StbiArena;
typedef u32 GLuint;
typedef struct {
    GLuint inputImageID,inputDepthID,inputWorldPosID,inputSpecID,inputNormalID,gBufferFBO,outputImageID;
    GLuint depthPrepassShaderProgram;
    GLuint chunkShaderProgram,vao_chunk; // Generic lit and unlit raster shader forward+
    GLuint debugUnlitShaderProgram;
    GLuint shadowmapsShaderProgram,shadowmapsClearShaderProgram,shadowMapSSBO,shadowMapsIndirectionID;
    GLuint ssrShaderProgram; // SSR (Screen Space Reflections)
    GLuint imageBlitShaderProgram,quadVAO,quadVBO; // Full Screen Quad Blit for rendering final compositing output/image effect passes
    GLuint textShaderProgram,textVAO,textVBO;
    GLuint debugLinesVAO,debugLinesVBO;
    GLuint blueNoiseBuffer;
    GLuint matricesBufferID,cellVisibleDataID;
    GLuint colorBufferID,texturePalettesID,texturePaletteOffsetsID,textureOffsetsID,textureSizesID;
    GLuint lightsID,voxelLightListCountsID,voxelLightListsID,voxelUpdateShaderProgram;
    GLuint vbos[MODEL_IDX_MAX],tbos[MODEL_IDX_MAX];
} RenderSystem;
size_t GetStringLength(const char *s);
bool CharacterIsEmpty(const char c);
bool StringsEqualLimitedBy(const char* a, const char* b, size_t limit);
void StringCopyInto_A_SubstringFrom_B(char* a, size_t substringSize, const char* b, size_t bufferSize);
void StringConcatenate(char* a, const char* b, size_t bufferSize);
extern RenderSystem Sys_Render; // Added last to make use of all defines for sizes.
bool EntityIsAnimated(u16 entIdx);
void InitializeEntity(Entity* entry);
void LoadLevel(u8 curlevel);
float GetPainStatic(void);
Color GetPainStaticColor(void);
int StringCompareUpToLength(const char* s1, const char* s2, size_t n);
static inline __attribute__((always_inline)) u32 parse_numberu32(const char* str, const char* line, u32 lineNum) {
    if (str == 0 || *str == '\0') { DualLogError("Invalid blank string from line[%d]: %s\n", lineNum+1, line); return 0; }
    while (CharacterIsEmpty((char)*str)) str++;
    while (CharacterIsEmpty(*str)) str++;
    if (*str == '+') str++;
    if (*str == '-') { DualLogError("Invalid input, negative not allowed (%s)\n      from line[%d]: %s\n", str, lineNum+1, line); return 0; }
    unsigned long result = 0;
    while (*str >= '0' && *str <= '9') {
        int digit = *str - '0';
        result = result * 10uL + (unsigned long)digit;
        str++;
    }

    return (u32)result;
}

static inline __attribute__((always_inline)) u16 parse_numberu16(const char* str, const char* line, u32 lineNum) {
    u32 retval = parse_numberu32(str, line, lineNum);
    if (retval > U16_MAX) { DualLogError("Value %u out of range for u16 from line[%d]: %s\n", retval, lineNum+1, line); return 0; }
    return (u16)retval;
}

static inline __attribute__((always_inline)) u8 parse_numberu8(const char* str, const char* line, u32 lineNum) {
    u32 retval = parse_numberu32(str, line, lineNum);
    if (retval > U8_MAX) { DualLogError("Value %u out of range for u8 from line[%d]: %s\n", retval, lineNum+1, line); return 0; }
    return (u8)retval;
}

static inline __attribute__((always_inline)) bool parse_bool(const char* str, const char* line, u32 lineNum) {
    u32 parseval = parse_numberu32(str, line, lineNum);
    if (parseval > 1) DualLogWarn("Loaded %u but expected boolean from line[%u]: %s\n",parseval, lineNum+1, line);
    return parseval > 0 ? true : false;
}

static inline __attribute__((always_inline)) float parse_float(const char* str, const char* line, u32 lineNum) {
    if (str == 0 || *str == '\0') { DualLogError("Invalid blank string from line[%d]: %s\n", lineNum+1, line); return 0.0f; }
    
    while (CharacterIsEmpty(*str)) str++;
    bool negative = false;
    if (*str == '-') { negative = true; str++; }
    else if (*str == '+') { str++; }

    double value = 0.0;
    bool has_digit = false;
    while (*str >= '0' && *str <= '9') { // Integer part
        value = value * 10.0 + (*str - '0');
        str++;
        has_digit = true;
    }

    if (*str == '.') { // Decimal part
        str++;
        double frac = 0.0;
        double place = 0.1;
        while (*str >= '0' && *str <= '9') {
            frac += (*str - '0') * place;
            place *= 0.1;
            str++;
            has_digit = true;
        }

        value += frac;
    }

    return (!has_digit) ? 0.0f : (negative ? (float)(-value) : (float)value);
}

static inline __attribute__((always_inline)) i32 PosGetCellCoordX(float pos_x) { return (u16)clamp((i32)vfloor((pos_x - Sys_Global.worldMin_x + CELLXHALF) / CELL_SIZE), 0, WORLDX_0BASED); }
static inline __attribute__((always_inline)) i32 PosGetCellCoordZ(float pos_z) { return (u16)clamp((i32)vfloor((pos_z - Sys_Global.worldMin_z + CELLXHALF) / CELL_SIZE), 0, WORLDX_0BASED); }
typedef u16 half;
static inline __attribute__((always_inline)) float half_to_float(half h){
    u32 s=(h&0x8000)<<16,e=(h&0x7C00)>>10,m=(h&0x03FF),out;
    if (e == 0){
        if (m == 0) out = s;
        else { // normalize subnormal
            e = 1;
            while ((m & 0x0400) == 0) { m <<= 1; e--; }
            m &= 0x03FF;
            e = e + (127 - 15);
            out = s | (e << 23) | (m << 13);
        }
    } else if (e == 31) out = s | 0x7F800000 | (m << 13);
    else { e = e + (127 - 15); out = s | (e << 23) | (m << 13); }
 
    float f;
    __builtin_memcpy(&f, &out, 4);
    return f;
}
