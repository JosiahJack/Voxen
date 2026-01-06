// voxen.c
// Description: A realtime OpenGL 4.3+ Game Engine for Citadel: The System Shock Fan Remake
#include "os.h" // Operating System calls shim layer.
#include "voxen.h"
#include "entity.h"
#include "External/stb_image.h"
#include "Shaders/shaders.h"
#include "todo.h"
#include "data_models.c"

Voxen_GlobalContext voxen_globalContext = { .screenshotTimeout = 1.0, .startLevel = 3, .numLevels = 2 };
VoxenDiagnostics      voxen_Diagnostics = { .worstFPS = UINT32_MAX };
Voxen_Cheats               voxen_Cheats = { .god = true, .noclip = true, .showLocation = true, .showFPS = true, .editMode = true };
VoxenSettings            voxen_Settings = { .ScreenWidth = 1366u, .ScreenHeight = 768u, .Shadows = 1u, .AntiAliasing = 1u, .Brightness = 50u, .VolumeMusic = 20u, .FOV = 65.0f, .Reflections = 1u };
uint8_t SSR_RES = 8; // Ratio is (1 / SSR_RES) * render resolution.
Voxen_GL_Comms           voxen_GL_Comms;
uint8_t queuedLevelToLoad = 3;
Entity instances[INSTANCE_COUNT];
float modelMatrices[INSTANCE_COUNT * 16];
uint8_t dirtyInstances[INSTANCE_COUNT];
GLuint instancesBuffer;
QuestBits questData;
Quaternion cam_rotation = { .w = 1.0f };
float cam_forwardx, cam_forwardy, cam_forwardz, cam_rightx, cam_righty, cam_rightz, berserkFinished, berserkSeedTime, aspect3D = 1.0f, cam_pitch, cam_yaw = 90.0f, cam_roll, fogColorR, fogColorG, fogColorB, fogBaseDensityForLevel;
float rasterPerspectiveProjection[16];
float shadowmapsPerspectiveProjection[16];
int32_t cursorPosition_x = 680, cursorPosition_y = 384; // Separate internal cursor from system cursor.  This gets relatively pushed around by real cursor movement to give consistent platform behavior.
char uiTextBuffer[TEXT_BUFFER_SIZE];
float uiOrthoProjection[16];
float lights[LIGHT_COUNT * LIGHT_DATA_SIZE];
bool lightDirty[LIGHT_COUNT];
static float lightView[LIGHT_COUNT][6][4][4]; // Array of Array of 6 Arrays of 16 floats (matrix 4x4).  lightView[i][face][0 ... 15]
static float lightViewProj[LIGHT_COUNT][6][4][4]; // Array of Array of 6 Arrays of 16 floats (matrix 4x4).  lightViewProj[i][face][0 ... 15]
FrustumPlane lightFrustumPlanes[LIGHT_COUNT][6][6]; // Array of Array of 6 Arrays of FrustumPlane structs (four floats).  lightFrustumPlanes[i][face][.nx,.ny,, .nz, .d]
FrustumPlane playerFrustumPlanes[6];
extern uint16_t editModeTestEntityDefinition;
KeyState keyStates[MAX_KEYS] = {{0}};
KeyState mouseButtons[MAX_MOUSE_BUTTONS] = {{0}};
double scrollDelta;
double last_mouse_x = 0.0, last_mouse_y = 0.0;
float mouse_sensitivity = 0.1f;
bool window_has_focus = false;
uint16_t editModeSelection = 682; // Test instance
uint16_t editModeTestEntityDefinition = 0; // Test instance's model index

// GLFW Callbacks
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
static void key_callback(GLFWwindow* window, int32_t key, int32_t scancode, int32_t action, int32_t mods) {
    if (key == GLFW_KEY_F10 && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        if (log_playback) {
            log_playback = false;
            DualLog("Exited log playback manually.  Control returned\n");
        } else {
            EnqueueEvent(EV_QUIT,EV_INT_FIELD_UNUSED,EV_INT_FIELD_UNUSED,EV_FLOAT_FIELD_UNUSED,EV_FLOAT_FIELD_UNUSED);
        }

        return;
    }
    
    if (!log_playback) {
        if (action == GLFW_PRESS || action == GLFW_REPEAT) EnqueueEvent(EV_KEYDOWN, key, EV_INT_FIELD_UNUSED, EV_FLOAT_FIELD_UNUSED, EV_FLOAT_FIELD_UNUSED);
        else if (action == GLFW_RELEASE) EnqueueEvent(EV_KEYUP, key, EV_INT_FIELD_UNUSED, EV_FLOAT_FIELD_UNUSED, EV_FLOAT_FIELD_UNUSED);
    }
}

static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!log_playback && window_has_focus) {
        int32_t dx = (int32_t)(xpos - last_mouse_x);
        int32_t dy = (int32_t)(ypos - last_mouse_y);
        last_mouse_x = xpos;
        last_mouse_y = ypos;
        if (ignore_next_mouse_delta) { ignore_next_mouse_delta = false; return; }
        
        if (voxen_Diagnostics.globalFrameNum > 1) EnqueueEvent(EV_MOUSEMOVE, dx, dy, EV_FLOAT_FIELD_UNUSED, EV_FLOAT_FIELD_UNUSED);
    }
}

static void window_focus_callback(GLFWwindow* window, int32_t focused) {
    window_has_focus = focused != 0;
    ignore_next_mouse_delta = true;
    glfwSetInputMode(window, GLFW_CURSOR, window_has_focus ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

static void mouse_button_callback(GLFWwindow* window, int32_t button, int32_t action, int32_t mods) {
    if (button < 0 || button >= MAX_MOUSE_BUTTONS) return;
    if (action == GLFW_PRESS) {
        mouseButtons[button].down = true;
        mouseButtons[button].pressed = true;
        EnqueueEvent(EV_KEYDOWN, button + 1000, EV_INT_FIELD_UNUSED, EV_FLOAT_FIELD_UNUSED, EV_FLOAT_FIELD_UNUSED); // offset mouse events if needed
    } else if (action == GLFW_RELEASE) {
        mouseButtons[button].down = false;
        mouseButtons[button].released = true;
        EnqueueEvent(EV_KEYUP, button + 1000, EV_INT_FIELD_UNUSED, EV_FLOAT_FIELD_UNUSED, EV_FLOAT_FIELD_UNUSED);
    }
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) { scrollDelta += yoffset; }
#pragma GCC diagnostic pop

void Input_Init(GLFWwindow* window) {
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetWindowFocusCallback(window, window_focus_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
}

int32_t Input_KeyDown(int32_t keycode) {
    if (keycode >= 0 && keycode < MAX_KEYS) keyStates[keycode].pressed = keyStates[keycode].down = true;
    if (voxen_Cheats.consoleActive) { ConsoleEmulator(keycode); return 0; }
    return 0;
}

int32_t Input_KeyUp(int32_t keycode) {
    if (keycode >= 0 && keycode < MAX_KEYS) keyStates[keycode].pressed = keyStates[keycode].down = false;
    return 0;
}

void InputClearRisingAndFallingEdges(void) { // Clear keypress rising and falling edge triggers
    for (int32_t i=0;i<MAX_KEYS;++i)          keyStates[i].pressed = keyStates[i].released = false;       // Can't memset as we want to preserve down state
    for (int32_t i=0;i<MAX_MOUSE_BUTTONS;i++) mouseButtons[i].pressed = mouseButtons[i].released = false; // Can't memset as we want to preserve down state
    scrollDelta = 0;
}

void UpdatePlayerFacingAngles(void) {
    float x2 = cam_rotation.x * cam_rotation.x;
    float y2 = cam_rotation.y * cam_rotation.y;
    float z2 = cam_rotation.z * cam_rotation.z;
    float xy = cam_rotation.x * cam_rotation.y;
    float xz = cam_rotation.x * cam_rotation.z;
    float yz = cam_rotation.y * cam_rotation.z;
    float wx = cam_rotation.w * cam_rotation.x;
    float wy = cam_rotation.w * cam_rotation.y;
    float wz = cam_rotation.w * cam_rotation.z;
    cam_forwardx = 2.0f * (xz + wy);  // Forward X
    cam_forwardy = 2.0f * (yz - wx);  // Forward Y
    cam_forwardz = 1.0f - 2.0f * (x2 + y2); // Forward Z
    cam_rightx = 1.0f - 2.0f * (y2 + z2);  // Right X
    cam_righty = 2.0f * (xy + wz);  // Right Y
    cam_rightz = 2.0f * (xz - wy);  // Right Z
    normalize_vector(&cam_forwardx, &cam_forwardy, &cam_forwardz); // Normalize forward
    normalize_vector(&cam_rightx, &cam_righty, &cam_rightz); // Normalize strafe
}

// Create a quaternion from yaw (around Y), pitch (around X), and roll (around Z) in degrees
void quat_from_yaw_pitch_roll(Quaternion* q, float yaw_deg, float pitch_deg, float roll_deg) {
    float yaw = deg2rad(yaw_deg);   // Around Y (up)
    float pitch = deg2rad(pitch_deg); // Around X (right)
    float roll = deg2rad(roll_deg);  // Around Z (forward)
    float cy = vcosf(yaw * 0.5f);
    float sy = vsinf(yaw * 0.5f);
    float cp = vcosf(pitch * 0.5f);
    float sp = vsinf(pitch * 0.5f); // using vsinf breaks it!
    float cr = vcosf(roll * 0.5f);
    float sr = vsinf(roll * 0.5f);
    q->w = cy * cp * cr + sy * sp * sr;
    q->x = cy * sp * cr + sy * cp * sr; // X-axis (pitch)
    q->y = sy * cp * cr - cy * sp * sr; // Y-axis (yaw)
    q->z = cy * cp * sr - sy * sp * cr; // Z-axis (roll)
    
    // Normalize quaterrnion
    float len = vsqrtf(q->w * q->w + q->x * q->x + q->y * q->y + q->z * q->z);
    if (len > 1e-6f) { q->x /= len; q->y /= len; q->z /= len; q->w /= len; }
    else { q->x = 0.0f; q->y = 0.0f; q->z = 0.0f; q->w = 1.0f; }
}

void Input_MouselookApply(void) {
    if (voxen_globalContext.currentLevel == LEVEL_CYBERSPACE) quat_from_yaw_pitch_roll(&cam_rotation,cam_yaw,cam_pitch,cam_roll);
    else               quat_from_yaw_pitch_roll(&cam_rotation,cam_yaw,cam_pitch,    0.0f);
}

int32_t Input_MouseMove(int32_t xrel, int32_t yrel) {
    if (CursorVisible()) {
        int32_t newX = cursorPosition_x + xrel;
        if (newX > voxen_Settings.ScreenWidth) newX = voxen_Settings.ScreenWidth;
        if (newX < 0) newX = 0;
        cursorPosition_x = newX;
        int32_t newY = cursorPosition_y + yrel;
        if (newY > voxen_Settings.ScreenHeight) newY = voxen_Settings.ScreenHeight;
        if (newY < 0) newY = 0;
        cursorPosition_y = newY;
    }
    
    if (voxen_globalContext.gamePaused || voxen_globalContext.inventoryMode) return 0;
    
    cam_yaw += (float)xrel * mouse_sensitivity;
    if (cam_yaw >= 360.0f) cam_yaw -= 360.0f;
    if (cam_yaw < 0.0f) cam_yaw += 360.0f;
    cam_pitch += (float)yrel * mouse_sensitivity;
    if (cam_pitch > 89.0f) cam_pitch = 89.0f; // Avoid gimbal lock at pure 90deg
    if (cam_pitch < -89.0f) cam_pitch = -89.0f;
    Input_MouselookApply();
    return 0;
}

void ProcessInput(void) {
    if (keyStates[GLFW_KEY_LEFT_CONTROL].down && keyStates[GLFW_KEY_B].pressed) CycleToNextMonitor(voxen_globalContext.window);
    if (keyStates[GLFW_KEY_GRAVE_ACCENT].pressed) ToggleConsole();
    
    if (keyStates[GLFW_KEY_LEFT_CONTROL].down && keyStates[GLFW_KEY_E].pressed) play_wav("./Audio/weapons/wpistol.wav",0.5f);
    // End Debug Inputs
    
    // =========== PAUSE BARRIER ==================
    if (voxen_globalContext.gamePaused || voxen_Cheats.consoleActive) return;
    
    uint16_t testlightIdx = (741 * LIGHT_DATA_SIZE);
    if (keyStates[GLFW_KEY_1].down) {
        lights[testlightIdx + LIGHT_DATA_OFFSET_POSX] += 0.01f; lightDirty[741] = true;
    } else if (keyStates[GLFW_KEY_2].down) {
        lights[testlightIdx + LIGHT_DATA_OFFSET_POSX] -= 0.01f; lightDirty[741] = true;
    }
    
    if (keyStates[GLFW_KEY_3].down) {
        lights[testlightIdx + LIGHT_DATA_OFFSET_POSY] += 0.01f; lightDirty[741] = true;
    } else if (keyStates[GLFW_KEY_4].down) {
        lights[testlightIdx + LIGHT_DATA_OFFSET_POSY] -= 0.01f; lightDirty[741] = true;
    }
    
    if (keyStates[GLFW_KEY_5].down) {
        lights[testlightIdx + LIGHT_DATA_OFFSET_POSZ] += 0.01f; lightDirty[741] = true;
    } else if (keyStates[GLFW_KEY_6].down) {
        lights[testlightIdx + LIGHT_DATA_OFFSET_POSZ] -= 0.01f; lightDirty[741] = true;
    }
    
    if (keyStates[GLFW_KEY_I].pressed) {
        editModeTestEntityDefinition++;
        if (editModeTestEntityDefinition >= entityCount) editModeTestEntityDefinition = 0u;
        Vector3 oldPos = instances[editModeSelection].position;
        Quaternion oldRot = instances[editModeSelection].rotation;
        Vector3 oldScale = instances[editModeSelection].scale;
        instances[editModeSelection] = entities[editModeTestEntityDefinition];
        instances[editModeSelection].position = oldPos;
        instances[editModeSelection].rotation = oldRot;
        instances[editModeSelection].scale = oldScale;
        DualLogEntityInstance(editModeSelection);
    }
        
    if (keyStates[GLFW_KEY_TAB].pressed) {
        ignore_next_mouse_delta = true;
        voxen_globalContext.inventoryMode = !voxen_globalContext.inventoryMode;
        cursorPosition_x = voxen_Settings.ScreenWidth / 2;
        cursorPosition_y = voxen_Settings.ScreenHeight / 2;
    }
}

void GenerateAndBindTexture(GLuint *id, GLint internalFormat, int32_t width, int32_t height, GLenum format, GLenum type, GLenum target) {
    glGenTextures(1, id);
    glBindTexture(target, *id);
    glTexImage2D(target, 0, internalFormat, width, height, 0, format, type, NULL);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

GLuint CompileShader(GLenum type, const char *source, const char *shaderName) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) { char infoLog[512]; glGetShaderInfoLog(shader, 512, NULL, infoLog); DualLogError("%s Compilation Failed: %s\n", shaderName, infoLog); OS_Exit(1); }
    return shader;
}

GLuint LinkProgram(GLuint *shaders, int32_t count, const char *programName) {
    GLuint program = glCreateProgram();
    for (int32_t i = 0; i < count; i++) { glAttachShader(program, shaders[i]); glDeleteShader(shaders[i]); }
    glLinkProgram(program);
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) { char infoLog[512]; glGetProgramInfoLog(program, 512, NULL, infoLog); DualLogError("%s Linking Failed: %s\n", programName, infoLog); OS_Exit(1); }
    return program;
}

void CompileShaders(void) {
    GLuint vertShader, fragShader, computeShader;
    vertShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource, "Chunk Vertex Shader");
    fragShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderTraditional, "Chunk Fragment Shader");
    voxen_GL_Comms.chunkShaderProgram = LinkProgram((GLuint[]){vertShader, fragShader}, 2, "Chunk Shader Program");
    
    vertShader = CompileShader(GL_VERTEX_SHADER, debugUnlitVertexShaderSource, "Debug Unlit Vertex Shader");
    fragShader = CompileShader(GL_FRAGMENT_SHADER, debugUnlitFragmentShaderSource, "Debug Unlit Fragment Shader");
    voxen_GL_Comms.debugUnlitShaderProgram = LinkProgram((GLuint[]){vertShader, fragShader}, 2, "Debug Unlit Shader Program");

    vertShader = CompileShader(GL_VERTEX_SHADER, shadowmapVertexShaderSource, "Shadowmaps Vertex Shader");
    fragShader = CompileShader(GL_FRAGMENT_SHADER, shadowmapFragmentShaderSource, "Shadowmaps Fragment Shader");
    voxen_GL_Comms.shadowmapsShaderProgram = LinkProgram((GLuint[]){vertShader, fragShader}, 2, "Shadowmaps Shader Program");

    vertShader = CompileShader(GL_VERTEX_SHADER, textVertexShaderSource, "Text Vertex Shader");
    fragShader = CompileShader(GL_FRAGMENT_SHADER, textFragmentShaderSource, "Text Fragment Shader");
    voxen_GL_Comms.textShaderProgram = LinkProgram((GLuint[]){vertShader, fragShader}, 2, "Text Shader Program");
    
    computeShader = CompileShader(GL_COMPUTE_SHADER, ssr_computeShader, "Screen Space Reflections Compute Shader");
    voxen_GL_Comms.ssrShaderProgram = LinkProgram((GLuint[]){computeShader}, 1, "Screen Space Reflections Shader Program");
    
    computeShader = CompileShader(GL_COMPUTE_SHADER, voxelUpdate_computeShader, "Voxel Update Compute Shader");
    voxen_GL_Comms.voxelUpdateShaderProgram = LinkProgram((GLuint[]){computeShader}, 1, "Voxel Update Shader Program");
        
    computeShader = CompileShader(GL_COMPUTE_SHADER, shadowmaps_clear_computeShader, "Shadowmaps Clear Compute Shader");
    voxen_GL_Comms.shadowmapsClearShaderProgram = LinkProgram((GLuint[]){computeShader}, 1, "Shadowmaps Clear Shader Program");

    vertShader = CompileShader(GL_VERTEX_SHADER,   quadVertexShaderSource,   "Image Blit Vertex Shader");
    fragShader = CompileShader(GL_FRAGMENT_SHADER, quadFragmentShaderSource, "Image Blit Fragment Shader");
    voxen_GL_Comms.imageBlitShaderProgram = LinkProgram((GLuint[]){vertShader, fragShader}, 2, "Image Blit Shader Program");
}

GLuint SetupSSBO(GLuint* id, GLuint bindingIndex, GLsizeiptr size, const void* data, GLenum usage) {
    glGenBuffers(1, id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, *id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, usage);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingIndex, *id);
    return *id;
}

void SetSkyRotateSpeed(void) {
    static const float speeds[] = { 0.05f, 1.0f, 2.5f, 3.75f, 6.25f };
    float skyRotateSpeed = speeds[voxen_Cheats.dizzyLevel];
    glUseProgram(voxen_GL_Comms.imageBlitShaderProgram);
    glUniform1f(30, skyRotateSpeed);
}

void SetFog(void) {
    glUseProgram(voxen_GL_Comms.chunkShaderProgram);
    glUniform3f(4, fogColorR * fogBaseDensityForLevel, fogColorG * fogBaseDensityForLevel, fogColorB * fogBaseDensityForLevel); // TODO: Add gunsmoke accumulation
}

void SetVSync(void) { glfwSwapInterval(voxen_Settings.Vsync); }

#define SHADOW_NEARMESH_MAX 512 // 350 was too low for light 712 on security atrium
#define MAX_SHADOWMAPS 56u
#define SHADOW_MAP_SIZE 192u
#define TOTAL_SHADOWMAP_PIXELS (MAX_SHADOWMAPS * (SHADOW_MAP_SIZE * SHADOW_MAP_SIZE * 6U))
typedef struct {
    double shadowTime;
    uint32_t numGLCallsForShadows;
	uint32_t numShadowsCouldRender;
	uint32_t shadowmapSizes[MAX_SHADOWMAPS];
	uint32_t shadowmapOffsets[MAX_SHADOWMAPS];
    uint32_t shadowmapIndirectionList[LIGHT_COUNT];
    float shadDotThresh;
	bool useComputeClear;
} VoxenShadowSystem;
VoxenShadowSystem voxen_Shadow_System;

void UpdateProjectionMatrices(void) {
    float* m;
    m = uiOrthoProjection;
    m[0] = 2.0f / (float)voxen_Settings.ScreenWidth; m[1] =                                         0.0f; m[2] =  0.0f; m[3] = 0.0f;
    m[4] =                                     0.0f; m[5] = -2.0f / ((float)voxen_Settings.ScreenHeight); m[6] =  0.0f; m[7] = 0.0f;
    m[8] =                                     0.0f; m[9] =                                         0.0f; m[10]= -1.0f; m[11]= 0.0f;
    m[12]=                                    -1.0f; m[13]=                                         1.0f; m[14]=  0.0f; m[15]= 1.0f;
    
    aspect3D = (float)voxen_Settings.ScreenWidth / (float)voxen_Settings.ScreenHeight;
    float f = vcot(voxen_Settings.FOV * PI / 360.0f);
    m = rasterPerspectiveProjection;
    m[0] = f / aspect3D; m[1] = 0.0f; m[2] =                                                      0.0f; m[3] =  0.0f;
    m[4] =         0.0f; m[5] =    f; m[6] =                                                      0.0f; m[7] =  0.0f;
    m[8] =         0.0f; m[9] = 0.0f; m[10]=      -(FAR_PLANE + NEAR_PLANE) / (FAR_PLANE - NEAR_PLANE); m[11]= -1.0f;
    m[12]=         0.0f; m[13]= 0.0f; m[14]= -2.0f * FAR_PLANE * NEAR_PLANE / (FAR_PLANE - NEAR_PLANE); m[15]=  0.0f;
    voxen_Shadow_System.shadDotThresh = 1.0f / vsqrtf(1.0f + vtan(voxen_Settings.FOV * (float)M_PI / 360.0f) * (1.0f + aspect3D * aspect3D));
}

void UpdateScreenSize(void) {
    UpdateProjectionMatrices();
    glUseProgram(voxen_GL_Comms.imageBlitShaderProgram);
    glUniform1ui(2, voxen_Settings.ScreenWidth);
    glUniform1ui(3, voxen_Settings.ScreenHeight);
    glUniform1i(26, SSR_RES);
    glUseProgram(voxen_GL_Comms.chunkShaderProgram);
    glUniform1ui(6, voxen_Settings.ScreenWidth);
    glUniform1ui(7, voxen_Settings.ScreenHeight);
    glUseProgram(voxen_GL_Comms.ssrShaderProgram);
    glUniform1ui(0, voxen_Settings.ScreenWidth / SSR_RES);
    glUniform1ui(1, voxen_Settings.ScreenHeight / SSR_RES);       
    glUniform1i(2, SSR_RES);
    SetSkyRotateSpeed();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Globally same alpha blending
}

// Generates View Matrix4x4 for Geometry Rasterizer Pass from camera world position + orientation
void mat4_lookat_from(float* m, Quaternion* camRotation, float x, float y, float z) {
    float rotation[16];
    quat_to_matrix(camRotation, rotation);
    float right[3]   = { rotation[0], rotation[1], rotation[2] };   // X+ (right)
    float up[3]      = { rotation[4], rotation[5], rotation[6] };   // Y+ (up)
    float forward[3] = { rotation[8], rotation[9], rotation[10] };  // Z+ (forward)
    m[0]  = right[0];   m[1]  = up[0];   m[2]  = -forward[0]; m[3]  = 0.0f;
    m[4]  = right[1];   m[5]  = up[1];   m[6]  = -forward[1]; m[7]  = 0.0f;
    m[8]  = right[2];   m[9]  = up[2];   m[10] = -forward[2]; m[11] = 0.0f;
    m[12] = -dot(right[0], right[1], right[2], x, y, z);   // -dot(right, eye)
    m[13] = -dot(up[0], up[1], up[2], x, y, z);      // -dot(up, eye)
    m[14] = dot(forward[0], forward[1], forward[2], x, y, z);  // dot(forward, eye)
    m[15] = 1.0f;
}

__attribute__((pure)) bool SphereInFrustum(FrustumPlane* planes, float cx, float cy, float cz, float radius) {
    if ((planes[0].nx * cx + planes[0].ny * cy + planes[0].nz * cz + planes[0].d) < -radius) return false;
    if ((planes[1].nx * cx + planes[1].ny * cy + planes[1].nz * cz + planes[1].d) < -radius) return false;
    if ((planes[2].nx * cx + planes[2].ny * cy + planes[2].nz * cz + planes[2].d) < -radius) return false;
    if ((planes[3].nx * cx + planes[3].ny * cy + planes[3].nz * cz + planes[3].d) < -radius) return false;
    if ((planes[4].nx * cx + planes[4].ny * cy + planes[4].nz * cz + planes[4].d) < -radius) return false;
    if ((planes[5].nx * cx + planes[5].ny * cy + planes[5].nz * cz + planes[5].d) < -radius) return false;
    return true;
}

void ExtractFrustumPlanes(float* m, FrustumPlane* planes) {
    planes[0].nx = m[3]  + m[0];  planes[0].ny = m[7]  + m[4];  planes[0].nz = m[11] + m[8];  planes[0].d = m[15] + m[12]; // Left
    planes[1].nx = m[3]  - m[0];  planes[1].ny = m[7]  - m[4];  planes[1].nz = m[11] - m[8];  planes[1].d = m[15] - m[12]; // Right
    planes[2].nx = m[3]  + m[1];  planes[2].ny = m[7]  + m[5];  planes[2].nz = m[11] + m[9];  planes[2].d = m[15] + m[13]; // Bottom
    planes[3].nx = m[3]  - m[1];  planes[3].ny = m[7]  - m[5];  planes[3].nz = m[11] - m[9];  planes[3].d = m[15] - m[13]; // Top
    planes[4].nx = m[3]  + m[2];  planes[4].ny = m[7]  + m[6];  planes[4].nz = m[11] + m[10]; planes[4].d = m[15] + m[14]; // Near
    planes[5].nx = m[3]  - m[2];  planes[5].ny = m[7]  - m[6];  planes[5].nz = m[11] - m[10]; planes[5].d = m[15] - m[14]; // Far
    for (int i = 0; i < 6; i++) {
        float len = vsqrtf(planes[i].nx*planes[i].nx + planes[i].ny*planes[i].ny + planes[i].nz*planes[i].nz);
        if (len > 0.0f) {
            planes[i].nx /= len; planes[i].ny /= len; planes[i].nz /= len; planes[i].d /= len; // Normalize
        }
    }
}

#define VOXEL_COUNT 262144 // 64 * 64 * 8 * 8
bool UpdateLights(bool* voxelsNeedUpdated) {
    if (voxen_globalContext.gamePaused || voxen_globalContext.menuActive) return false;
    
    for (int lightIdx = 0; lightIdx < loadedLights; ++lightIdx) {
        if (lightDirty[lightIdx]) { // Marked all as true at level load.
            *voxelsNeedUpdated = true;
            uint32_t litIdx = lightIdx * LIGHT_DATA_SIZE;
            float litX = lights[litIdx];
            float litY = lights[litIdx + LIGHT_DATA_OFFSET_POSY];
            float litZ = lights[litIdx + LIGHT_DATA_OFFSET_POSZ];
            #pragma GCC unroll 6
            for (int j=0;j<6;++j) {
                mat4_lookat_from((float*)lightView[lightIdx][j], &cubemapOrientationQuaternion[j], litX, litY, litZ);
                mul_mat4((float*)lightViewProj[lightIdx][j], shadowmapsPerspectiveProjection, (float*)lightView[lightIdx][j]);
                ExtractFrustumPlanes((float*)lightViewProj[lightIdx][j], lightFrustumPlanes[lightIdx][j]);
            }
        }
    }
        
    for (int i=0;i<loadedLights;++i) { // Just lerps/flickers in intensity
        if (lightIntervalStepsLength[i] < 1) continue;
        
        int litIdx = i * LIGHT_DATA_SIZE;
        if (!lightOn[i]) { lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY] = lightMinIntensity[i]; continue; }

        float differenceInIntensity = (lightMaxIntensity[i] - lightMinIntensity[i]);
        if (lightLerpTime[i] < (float)voxen_globalContext.pauseRelativeTime) {
            lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY] = lightLerpUp[i] ? lightMaxIntensity[i] : lightMinIntensity[i]; // Pick target to lerp towards
            lightLerpUp[i] = !lightLerpUp[i];
            lightCurrentStep[i]++;
            if (lightCurrentStep[i] >= lightIntervalStepsLength[i]) lightCurrentStep[i] = 0; // Wrap and start over continuous looping
            lightLerpStepTime[i] = lightIntervalSteps[i][lightCurrentStep[i]];
            lightLerpTime[i] = (float)voxen_globalContext.pauseRelativeTime + lightLerpStepTime[i];
            lightLerpStartTime[i] = (float)voxen_globalContext.pauseRelativeTime;
        } else if (lightLerpOn[i]) {
            if (lightCurrentStep[i] < lightIntervalStepIsLerpingLength[i]) {
                if (intervalStepisLerping[i][lightCurrentStep[i]]) {
                    lightLerpValue[i] = ((float)voxen_globalContext.pauseRelativeTime - lightLerpStartTime[i])/(lightLerpTime[i] - lightLerpStartTime[i]); // percent towards goal time
                    float lerpVal = lightLerpUp[i] ? (lightLerpValue[i]) : (1.0f - lightLerpValue[i]);
                    lightLerpValue[i] = lightMinIntensity[i] + (differenceInIntensity * lerpVal);
                    lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY] = lightLerpValue[i];
                }
            }
        }
    }

    glNamedBufferData(voxen_GL_Comms.lightsID,loadedLights * LIGHT_DATA_SIZE * sizeof(float), lights, GL_DYNAMIC_DRAW);
    if (*voxelsNeedUpdated) {
        glUseProgram(voxen_GL_Comms.voxelUpdateShaderProgram);
        GLuint groupX_voxels = (512 + 31) / 32;
        GLuint groupZ_voxels = (512 + 31) / 32; // Actually just a local size y, but for z axis voxels
        glDispatchCompute(groupX_voxels,groupZ_voxels, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }
    
    return *voxelsNeedUpdated;
}

typedef struct {
    float depth;
    uint16_t index;
} DepthSort;

__attribute__((pure)) int32_t compareDepthSort(const void* a, const void* b) {
    float da = ((const DepthSort*)a)->depth;
    float db = ((const DepthSort*)b)->depth;
    return (db > da) - (db < da);
}

__attribute__((pure)) int32_t compareDepthSortInverted(const void* a, const void* b) {
    float da = ((const DepthSort*)a)->depth;
    float db = ((const DepthSort*)b)->depth;
    return (da > db) - (da < db);
}

// DepthSort shadows_nearMeshes[SHADOW_NEARMESH_MAX]; // Found that this is typically around 172
// float shadows_nearMeshRadii[SHADOW_NEARMESH_MAX];

typedef struct {
    uint16_t index; // Original index in lights array
    float distanceSquared; // Distance to camera squared
    float score; // Priority score (lower distance, higher intensity = higher priority)
} LightCandidate;

// ============================================================================
// UI Rendering and Text
__attribute__((pure)) float GetScreenRelativeX(float percentage) { return (float)voxen_Settings.ScreenWidth * percentage; }
__attribute__((pure)) float GetScreenRelativeY(float percentage) { return (float)voxen_Settings.ScreenHeight * percentage; }

void RenderUIImage(float x, float y, float width, float height, uint32_t texIndex) {
    glEnable(GL_BLEND);
    glClear(GL_DEPTH_BUFFER_BIT); // Clear main FBO.  glClearBufferfv was actually SLOWER!  2nd Clear needed or UI dissappears/flickers!!
    glDisable(GL_CULL_FACE);
    glUseProgram(voxen_GL_Comms.chunkShaderProgram);
    glBindVertexArray(voxen_GL_Comms.textVAO);
    glUniform1ui(1, 0);
    glUniform1ui(3, 1u);  // isUI true
    glUniform1ui(17, 1u); // unlit is true
    glUniform1ui(19, 0);
    glUniform1ui(20, 0);
    glUniformMatrix4fv(2, 1, GL_FALSE, uiOrthoProjection);
    glBindBuffer(GL_ARRAY_BUFFER, voxen_GL_Comms.textVBO);
    float x1 = x + width;
    float y1 = y + height;
    float z = 0.0f;
    float vertices[30] = { x, y1, z, 0.0f, 0.0f, x1,  y, z, 1.0f, 1.0f, x1, y1, z, 1.0f, 0.0f, x, y1, z, 0.0f, 0.0f, x,  y, z, 0.0f, 1.0f, x1,  y, z, 1.0f, 1.0f };
    glUniform1ui(18, texIndex);
    glBufferData(GL_ARRAY_BUFFER, 30 * sizeof(float), vertices, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    voxen_Diagnostics.drawCallsRenderedThisFrame++;
    voxen_Diagnostics.uiImageDrawCallsRenderedThisFrame++;
    voxen_Diagnostics.verticesRenderedThisFrame += 6;    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

__attribute__((pure)) bool CursorIsOverBounds(float startX, float endX, float startY, float endY) {
    return (   cursorPosition_x >= startX && cursorPosition_x <= endX     // 0 == left
            && cursorPosition_y >= endY   && cursorPosition_y <= startY); // 0 == top
}

float textVertexData[8192]; // Reusable buffer for text vertices.  Most text only needs ~3000
void RenderFormattedText(float x, float y, uint32_t color, uint8_t fontID, const char* format, ...) {
    va_list args;
    va_start(args, format); vsnprintf(uiTextBuffer, TEXT_BUFFER_SIZE, format, args); va_end(args);
    glUseProgram(voxen_GL_Comms.textShaderProgram);
    glUniformMatrix4fv(0, 1, GL_FALSE, uiOrthoProjection);
    glUniform4f(3, textColors[color].r, textColors[color].g, textColors[color].b, textColors[color].a);
    if (fontID == FONT_STOPD) glBindTextureUnit(6, fontAtlasTexStopD);
    else glBindTextureUnit(6, fontAtlasTex);
    
    glUniform2f(4, 1.0f / (float)FONT_ATLAS_SIZE, 1.0f / (float)FONT_ATLAS_SIZE);
    glUniform1ui(2, fontID);
    glUniform1i(1, 6); // textTexture sampler2D
    glBindVertexArray(voxen_GL_Comms.textVAO);
    size_t vertexCount = 0;
    const char* p = uiTextBuffer;
    float xpos = x, ypos = y + GetScreenRelativeY(0.0211f);
    float lineSpacing = GetScreenRelativeY(0.03f); // Match RenderUI
    stbtt_aligned_quad q;
    int characterCount = 0;
    float paddingUV = 12.0f / (float)FONT_ATLAS_SIZE; // This is for the black outline around all text for readability.
    float borderWidthPixels = 2.0f;
    while (*p) {
        uint32_t codepoint = DecodeUTF8(&p);
        characterCount++;
        if (codepoint == '\n' || characterCount > 120) {
            xpos = x;
            ypos += lineSpacing;
            characterCount = 0;
            continue;
        }

        int idx = CodepointToPackedIndex(codepoint, fontID);
        if (idx < 0) continue;

        if (fontID == FONT_STOPD) stbtt_GetPackedQuad(fontPackedCharStopD, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, idx, &xpos, &ypos, &q, 1);
        else stbtt_GetPackedQuad(fontPackedChar, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, idx, &xpos, &ypos, &q, 1);
        float vx0 = q.x0 - borderWidthPixels;
        float vy0 = q.y0 - borderWidthPixels;
        float vx1 = q.x1 + borderWidthPixels;
        float vy1 = q.y1 + borderWidthPixels;
        float s0 = q.s0 - paddingUV;
        float t0 = q.t0 - paddingUV;
        float s1 = q.s1 + paddingUV;
        float t1 = q.t1 + paddingUV;
        float z = 0.0f;
        float textVertices[30] = { vx0, vy0, z, s0, t0, vx1, vy1, z, s1, t1, vx1, vy0, z, s1, t0, vx0, vy0, z, s0, t0, vx0, vy1, z, s0, t1, vx1, vy1, z, s1, t1 };
        memcpy(textVertexData + vertexCount * 30, textVertices, sizeof(textVertices));
        vertexCount++;
        if (codepoint >= '0' && codepoint <= '9') {
            if (fontID == FONT_STOPD) xpos = q.x0 + fixedNumberAdvanceWidthStopD;
            else xpos = q.x0 + fixedNumberAdvanceWidth;
        }
    }
    
    if (vertexCount > 0) {
        glNamedBufferData(voxen_GL_Comms.textVBO, vertexCount * 30 * sizeof(float), textVertexData, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount * 6);
        voxen_Diagnostics.drawCallsRenderedThisFrame++;
        voxen_Diagnostics.textDrawCallsRenderedThisFrame++;
        voxen_Diagnostics.verticesRenderedThisFrame += vertexCount * 6;
    }
}

void RenderLoadingProgress(int32_t offset, const char* text) { // Only adds 0.01secs to game startup time.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    RenderFormattedText(voxen_Settings.ScreenWidth / 2 - offset, voxen_Settings.ScreenHeight / 2 - 5, TEXT_WHITE, FONT_NORMAL, text);
    glfwSwapBuffers(voxen_globalContext.window);
}

char statusText[TEXT_BUFFER_SIZE];
void CenterStatusPrint(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    (void)vsnprintf(statusText, TEXT_BUFFER_SIZE, fmt, args);
    va_end(args);
    DualLog("%s\n",statusText);
    voxen_globalContext.statusTextDecayFinished = get_time() + 2.5; // 2.5 second decay time before text dissappears.
}

void NewGame(void) {
    RenderLoadingProgress(100,"Loading new game...");
    memset(&questData, 0, sizeof(QuestBits));
    questData.lev1SecCode = random_range_u8(0u,9u); // Must do rand's repeatedly to prevent
    questData.lev2SecCode = random_range_u8(0u,9u); // these all being the same number.
    questData.lev3SecCode = random_range_u8(0u,9u);
    questData.lev4SecCode = random_range_u8(0u,9u);
    questData.lev5SecCode = random_range_u8(0u,9u);
    questData.lev6SecCode = random_range_u8(0u,9u);
    memset(instances,0,INSTANCE_COUNT * sizeof(Entity)); // Initialize instances, the global entity array for the currently loaded level.
    instances[PLAYER1].index = 767;
    instances[PLAYER1].layer = PhysicsLayer_Player;
    instances[PLAYER1].position = (Vector3) { .x = 10.52f, .y = -43.792f + 0.84f, .z = 20.2908f}; // Start Actual: Puts player on Medical Level in actual game start position.  Added 0.84f y for cam offset from center
    instances[PLAYER1].scale = (Vector3) { 1.0f, 1.0f, 1.0f };
    instances[PLAYER1].rotation.w = 1.0f;
    instances[PLAYER1].entflags = ENTFLAG_ACTIVE | ENTFLAG_USEGRAVITY | ENTFLAG_RIGIDBODY;
    instances[PLAYER1].collider = COLLIDER_TYPE_CAPSULE;
    instances[PLAYER1].colliderCenter.y = 0.84f;
    instances[PLAYER1].colliderSize = (Vector3) { .x = 0.48f, .y = 2.0f, .z = COLLIDER_CAPSULE_DIRECTION_Y_F}; // Radius, Overall height including end radii (Unity convention, blech), Direction, 1.0 == Y-Axis
    instances[PLAYER1].mass = 1.0f;
    instances[PLAYER1].linearDrag = 8.0f;
    instances[PLAYER1].dynamicFriction = 0.6f;
    instances[PLAYER1].staticFriction = 0.8f;
    instances[PLAYER1].frictionCombine = PHYS_COMBINE_MUL;
    LoadLevel(voxen_globalContext.startLevel); // Must be after entities!
    voxen_globalContext.pauseRelativeTime = 0.0;
    voxen_globalContext.last_physics_time = get_time();
    voxen_globalContext.last_topframe_time = voxen_globalContext.last_physics_time - 0.05;
}

#define MAX_DEBUG_LINE_VERTS 4096                // 2048 lines max per frame
float debugLineBuffer[MAX_DEBUG_LINE_VERTS * 3]; // xyz only

void InitializeEnvironment(int32_t argc, char* command, char* command_input1) {
    double init_start_time = get_time();
    voxen_Diagnostics.globalFrameNum = 0;
    DebugRAM("prior to event system init");
    DualLog("Voxen by W. Josiah Jack, MIT-0 licensed\n");
    EventSystemInit(argc,command,command_input1);
    if (!glfwInit()) { DualLogError("GLFW initialization failed\n"); OS_Exit(1); }
    
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SRGB_CAPABLE, 0);
    glfwWindowHint(GLFW_RESIZABLE, 0);
    voxen_globalContext.window = glfwCreateWindow(voxen_Settings.ScreenWidth, voxen_Settings.ScreenHeight, "Voxen", NULL, NULL);
    if (!voxen_globalContext.window) { DualLogError("glfwCreateWindow failed\n"); OS_Exit(1); }
        
    glfwMakeContextCurrent(voxen_globalContext.window);
    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) { DualLogError("Failed to initialize GLAD\n"); OS_Exit(1); }
    
    CycleToNextMonitor(voxen_globalContext.window);
    glfwSetInputMode(voxen_globalContext.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    SetVSync();
    DualLog("OpenGL Version: %s, ", (const char*)glGetString(GL_VERSION));
    DualLog("GPU: %s", (const char*)glGetString(GL_RENDERER));
    OS_CPUInfo();
    GLint maxWorkGroupCountX;
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &maxWorkGroupCountX);
    voxen_Shadow_System.useComputeClear = (maxWorkGroupCountX > 65536); // Some systems limit the workgroup size to uint16_t (e.g. Mesa on Intel HD4400), so fallback to slower big hammer but reliable glClearBufferData instead for shadowmaps clear
    Input_Init(voxen_globalContext.window);
    glFrontFace(GL_CCW); // Set triangle sorting order (GL_CW vs GL_CCW)
    CompileShaders();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the corner where last shadowmap wrote into
    GLuint vaos[4]; GLuint vbos[4];
    glCreateVertexArrays(4, vaos);
    glCreateBuffers(3, vbos);
    voxen_GL_Comms.quadVAO = vaos[0]; voxen_GL_Comms.vao_chunk = vaos[1]; voxen_GL_Comms.textVAO = vaos[2]; voxen_GL_Comms.debugLinesVAO = vaos[3];
    voxen_GL_Comms.quadVBO = vbos[0];                                     voxen_GL_Comms.textVBO = vbos[1]; voxen_GL_Comms.debugLinesVBO = vbos[2];
    float quadBlit_vertices[] = { 1.0f, -1.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,    -1.0f,1.0f, 0.0f, 1.0f,   -1.0f, -1.0f, 0.0f, 0.0f }; // 4 verts, 4 floats each pos.xy, uv.xy
    glNamedBufferData(voxen_GL_Comms.quadVBO, sizeof(quadBlit_vertices), quadBlit_vertices, GL_STATIC_DRAW);

    glVertexArrayAttribFormat(voxen_GL_Comms.quadVAO, 0, 2, GL_FLOAT, GL_FALSE, 0); // DSA: Set position format
    glVertexArrayAttribFormat(voxen_GL_Comms.quadVAO, 1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float)); // DSA: Set texcoord format
    glVertexArrayVertexBuffer(voxen_GL_Comms.quadVAO, 0, voxen_GL_Comms.quadVBO, 0, 4 * sizeof(float)); // DSA: Link VBO to VAO
    for (uint8_t i = 0; i < 2; i++) { glVertexArrayAttribBinding(voxen_GL_Comms.quadVAO, i, 0); glEnableVertexArrayAttrib(voxen_GL_Comms.quadVAO, i); }
    
    glVertexArrayAttribFormat(voxen_GL_Comms.vao_chunk, 0, 3, GL_FLOAT, GL_FALSE, 0); // Position (vec3)
    glVertexArrayAttribFormat(voxen_GL_Comms.vao_chunk, 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float)); // Normal (vec3)
    glVertexArrayAttribFormat(voxen_GL_Comms.vao_chunk, 2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float)); // Tex Coord (vec2)
    for (uint8_t i = 0; i < 3; i++) { glVertexArrayAttribBinding(voxen_GL_Comms.vao_chunk, i, 0); glEnableVertexArrayAttrib(voxen_GL_Comms.vao_chunk, i); }
    
    glVertexArrayAttribFormat(voxen_GL_Comms.textVAO, 0, 3, GL_FLOAT, GL_FALSE, 0); // pos (x,y,z) 4 floats per vertex, stride = 4*sizeof(float)
    glVertexArrayAttribFormat(voxen_GL_Comms.textVAO, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float));  // uv (s,t)
    glVertexArrayVertexBuffer(voxen_GL_Comms.textVAO, 0, voxen_GL_Comms.textVBO, 0, 5 * sizeof(float));
    for (uint8_t i = 0; i < 2; i++) { glVertexArrayAttribBinding(voxen_GL_Comms.textVAO, i, 0); glEnableVertexArrayAttrib(voxen_GL_Comms.textVAO, i); }
    
    glNamedBufferStorage(voxen_GL_Comms.debugLinesVBO, MAX_DEBUG_LINE_VERTS * 3 * sizeof(float), NULL, GL_DYNAMIC_STORAGE_BIT);  // persistent, client-writable
    glVertexArrayAttribFormat(voxen_GL_Comms.debugLinesVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glEnableVertexArrayAttrib(voxen_GL_Comms.debugLinesVAO, 0);
    glVertexArrayAttribBinding(voxen_GL_Comms.debugLinesVAO, 0, 0);
    glVertexArrayVertexBuffer(voxen_GL_Comms.debugLinesVAO, 0, voxen_GL_Comms.debugLinesVBO, 0, 3 * sizeof(float));

    GenerateAndBindTexture(&voxen_GL_Comms.inputImageID,             GL_RGBA8, voxen_Settings.ScreenWidth, voxen_Settings.ScreenHeight,            GL_RGBA, GL_UNSIGNED_BYTE, GL_TEXTURE_2D); // Lit Raster
    GenerateAndBindTexture(&voxen_GL_Comms.inputWorldPosID,        GL_RGBA16F, voxen_Settings.ScreenWidth, voxen_Settings.ScreenHeight,            GL_RGBA,         GL_FLOAT, GL_TEXTURE_2D); // Raster World Positions
    GenerateAndBindTexture(&voxen_GL_Comms.inputDepthID, GL_DEPTH_COMPONENT32, voxen_Settings.ScreenWidth, voxen_Settings.ScreenHeight, GL_DEPTH_COMPONENT,         GL_FLOAT, GL_TEXTURE_2D); // Raster Depth
    GenerateAndBindTexture(&voxen_GL_Comms.inputSpecID,              GL_RGBA8, voxen_Settings.ScreenWidth, voxen_Settings.ScreenHeight,            GL_RGBA, GL_UNSIGNED_BYTE, GL_TEXTURE_2D); // Specular Colors
    GenerateAndBindTexture(&voxen_GL_Comms.inputNormalID,            GL_RG16F, voxen_Settings.ScreenWidth, voxen_Settings.ScreenHeight,              GL_RG,         GL_FLOAT, GL_TEXTURE_2D); // Normal XYZ
    glGenTextures(1, &voxen_GL_Comms.outputImageID);
    glBindTexture(GL_TEXTURE_2D, voxen_GL_Comms.outputImageID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,  voxen_Settings.ScreenWidth / SSR_RES,  voxen_Settings.ScreenHeight / SSR_RES, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glGenFramebuffers(1, &voxen_GL_Comms.gBufferFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, voxen_GL_Comms.gBufferFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, voxen_GL_Comms.inputImageID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, voxen_GL_Comms.inputWorldPosID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, voxen_GL_Comms.inputSpecID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, voxen_GL_Comms.inputNormalID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, voxen_GL_Comms.inputDepthID, 0);
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
    glDrawBuffers(4, drawBuffers);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) DualLogError("Framebuffer incomplete: Error code %d\n", status);
    glBindImageTexture(0, voxen_GL_Comms.inputImageID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8); // Main Rendered Color
    glBindImageTexture(1, voxen_GL_Comms.inputWorldPosID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F); // World Position XYZ
    glBindImageTexture(2, voxen_GL_Comms.inputSpecID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8); // Specular
    //                 3 = depth
    glBindImageTexture(4, voxen_GL_Comms.outputImageID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8); // SSR result
    glBindImageTexture(5, voxen_GL_Comms.inputNormalID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RG16F); // Normal XYZ
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, voxen_GL_Comms.outputImageID);
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Needed to render loading progress.
    glDepthMask(GL_TRUE); // Always true, set just once ever.
    UpdateScreenSize();
    float* m = shadowmapsPerspectiveProjection;
    m[0] = 1.0f; m[1] = 0.0f; m[2] =                                                                  0.0f; m[3] =  0.0f;
    m[4] = 0.0f; m[5] = 1.0f; m[6] =                                                                  0.0f; m[7] =  0.0f;
    m[8] = 0.0f; m[9] = 0.0f; m[10]=      -(LIGHT_RANGE_MAX + NEAR_PLANE) / (LIGHT_RANGE_MAX - NEAR_PLANE); m[11]= -1.0f;
    m[12]= 0.0f; m[13]= 0.0f; m[14]= -2.0f * LIGHT_RANGE_MAX * NEAR_PLANE / (LIGHT_RANGE_MAX - NEAR_PLANE); m[15]=  0.0f;
    InitFontAtlasses();
    RenderLoadingProgress(80,"Loading...");
    Input_MouselookApply(); // Input
    InitializeAudio(); // Audio    
    LoadTextForLanguage(voxen_Settings.Language);
    LoadLogTextForLanguage(voxen_Settings.Language);
    ParseGameData();
    glfwSetWindowTitle(voxen_globalContext.window, voxen_globalContext.global_modname);
    int fp = OS_OpenReadonly("./Textures/UI/menudot1.png");
    int windowIconFileSize = OS_FileSize(fp);
    uint8_t* file_buffer = OS_AllocateFileBackedRAMReadonly(windowIconFileSize, fp, "./Textures/UI/menudot1.png");
    OS_Close(fp);
    int w = 1, h = 1;
    stbi__arena_init();
    unsigned char* pixels = stbi_load_from_memory(file_buffer, windowIconFileSize, &w, &h);
    if (!pixels) { DualLogError("Failed to load icon: ./Textures/UI/menudot1.png\n"); OS_Exit(1); }
    
    GLFWimage image;
    image.width  = w;
    image.height = h;
    image.pixels = pixels;
    glfwSetWindowIcon(voxen_globalContext.window, 1, &image);
    file_buffer = OS_DeallocateRAM(file_buffer, windowIconFileSize);
    stbi__arena_base = OS_DeallocateRAM(stbi__arena_base, STBI_ARENA_SIZE);
    DebugRAM("after freeing window bar icon");
    DualLog("GL buffers, FBO, fonts, audio, localization, and window init took %f secs\n", get_time() - init_start_time);
    LoadEntities();
    float mat[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    memcpy(&modelMatrices[0], mat, 16 * sizeof(float)); // Null instance matrix used for UI
    voxen_GL_Comms.cellVisibleDataID       = SetupSSBO(&voxen_GL_Comms.cellVisibleDataID,        4, ARRSIZE * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
    voxen_GL_Comms.shadowMapSSBO           = SetupSSBO(&voxen_GL_Comms.shadowMapSSBO,            5, TOTAL_SHADOWMAP_PIXELS * sizeof(uint32_t), NULL, GL_STATIC_DRAW);    
    voxen_GL_Comms.voxelLightListCountsID  = SetupSSBO(&voxen_GL_Comms.voxelLightListCountsID,   6, VOXEL_COUNT * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
    voxen_GL_Comms.shadowMapsIndirectionID = SetupSSBO(&voxen_GL_Comms.shadowMapsIndirectionID,  8, LIGHT_COUNT * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
    voxen_GL_Comms.matricesBufferID        = SetupSSBO(&voxen_GL_Comms.matricesBufferID,        11, INSTANCE_COUNT * 16 * sizeof(float), modelMatrices, GL_STATIC_DRAW);
    voxen_GL_Comms.colorBufferID           = SetupSSBO(&voxen_GL_Comms.colorBufferID,           12, MAX_TOTAL_PIXELS * sizeof(uint8_t), NULL, GL_STATIC_DRAW);
    voxen_GL_Comms.blueNoiseBuffer         = SetupSSBO(&voxen_GL_Comms.blueNoiseBuffer,         13, 12288 * sizeof(float), blueNoise, GL_STATIC_DRAW);
    voxen_GL_Comms.textureOffsetsID        = SetupSSBO(&voxen_GL_Comms.textureOffsetsID,        14, MAX_VALID_TEXTURE * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
    voxen_GL_Comms.textureSizesID          = SetupSSBO(&voxen_GL_Comms.textureSizesID,          15, MAX_VALID_TEXTURE * 2 * sizeof(int32_t), NULL, GL_STATIC_DRAW);
    voxen_GL_Comms.texturePalettesID       = SetupSSBO(&voxen_GL_Comms.texturePalettesID,       16, MAX_UNIQUE_COLORS * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
    voxen_GL_Comms.texturePaletteOffsetsID = SetupSSBO(&voxen_GL_Comms.texturePaletteOffsetsID, 17, MAX_VALID_TEXTURE * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
    voxen_GL_Comms.lightsID                = SetupSSBO(&voxen_GL_Comms.lightsID,                19, LIGHT_COUNT * LIGHT_DATA_SIZE * sizeof(float), NULL, GL_STATIC_DRAW);
    voxen_GL_Comms.voxelLightListsID       = SetupSSBO(&voxen_GL_Comms.voxelLightListsID,       27,  VOXEL_COUNT * MAX_LIGHTS_PER_VOXEL * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
//     play_mp3("./Audio/music/TITLOOP-00_menu.mp3",((float)voxen_Settings.VolumeMusic/100.0f) * 0.4f + 0.09f,1500);
    NewGame(); // TODO: Do this from menu not immediately lol
    DebugRAM("InitializeEnvironment end");
}

void DrawDebugLines(float* viewProj) {
    if (voxen_Diagnostics.debugLineVertCount < 1) return;
    
    glNamedBufferSubData(voxen_GL_Comms.debugLinesVBO, 0, voxen_Diagnostics.debugLineVertCount * sizeof(float), debugLineBuffer);
    glUseProgram(voxen_GL_Comms.debugUnlitShaderProgram);
    glUniformMatrix4fv(0, 1, GL_FALSE, viewProj);
    glLineWidth(10.0f);
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(voxen_GL_Comms.debugLinesVAO);
    glDrawArrays(GL_LINES, 0, voxen_Diagnostics.debugLineVertCount / 3);
    glEnable(GL_DEPTH_TEST);
    voxen_Diagnostics.drawCallsRenderedThisFrame++;
    voxen_Diagnostics.verticesRenderedThisFrame += voxen_Diagnostics.debugLineVertCount / 3;
    voxen_Diagnostics.debugLineVertCount = 0;
}

void AddDebugLine(float x1, float y1, float z1, float x2, float y2, float z2) {
    if (voxen_Diagnostics.debugLineVertCount + 6 > MAX_DEBUG_LINE_VERTS * 3) return;

    int32_t i = voxen_Diagnostics.debugLineVertCount;
    debugLineBuffer[i++] = x1; debugLineBuffer[i++] = y1; debugLineBuffer[i++] = z1;
    debugLineBuffer[i++] = x2; debugLineBuffer[i++] = y2; debugLineBuffer[i++] = z2;
    voxen_Diagnostics.debugLineVertCount = i;
}

__attribute__((pure)) bool EntityIsAnimated(uint16_t entIdx) {
    return (   entIdx == 53
            || entIdx == 79
            || (entIdx >= 420 && entIdx <= 442)
            || (entIdx >= 496 && entIdx <= 514)
            || entIdx == 585
            || entIdx == 602
            || (entIdx >= 609 && entIdx <= 614)
            || (entIdx >= 741 && entIdx <= 745));
}

bool StepLoopingAnim(uint16_t i) {
    bool portalsNeedUpdated = false;
    AnimationClip currentClip = modelAnimationClips[instances[i].animationNum][instances[i].clip];
    if (instances[i].currentFrameFinished < voxen_globalContext.current_time) {
        instances[i].currentFrameFinished = voxen_globalContext.current_time + ((double)currentClip.speed * (1.0 / (double)currentClip.framerate));
        instances[i].frame++;
        if (instances[i].frame > currentClip.frameEnd) instances[i].frame = currentClip.frameStart;
        else if (instances[i].frame < currentClip.frameStart) instances[i].frame = currentClip.frameEnd;

        instances[i].modelIndex = (currentClip.frameStartModelIndex + (instances[i].frame - currentClip.frameStart));
        if (EntityIndexIsPortalBlockingDoor(instances[i].index)) {
            uint8_t portalIdx = instances[i].portalIndex;
            if (portalIdx < MAX_PORTALS) {
                uint16_t closedModelIndex = 719;
                switch(instances[i].index) {
                    case 496: closedModelIndex =  719; break; // doorA
                    case 497: closedModelIndex =  699; break; // doorB
                    case 498: closedModelIndex = 1398; break; // doorC
                    case 499: closedModelIndex = 1301; break; // doorD
                    case 500: closedModelIndex = 1612; break; // doorE
                    case 501: closedModelIndex = 1652; break; // doorF
                    case 503: closedModelIndex = 1742; break; // doorH
                    case 504: closedModelIndex = 1792; break; // doorI
                    case 508: closedModelIndex = 1845; break; // door_elevator1
                    case 509: closedModelIndex = 1887; break; // door_elevator2
                    case 510: closedModelIndex = 1929; break; // door_elevator3
                    case 511: closedModelIndex = 1973; break; // door_elevator4
                    case 512: closedModelIndex = 2078; break; // door_secret1
                    case 513: closedModelIndex = 2036; break; // door_secret2
                    case 514: closedModelIndex = 2120; break; // door_secret3
                }
            
                bool currentState = activePortals[portalIdx].open;
                if (instances[i].modelIndex == closedModelIndex && currentState) {
                    activePortals[portalIdx].open = false;
                    activePortals[portalIdx].dirty = true;
                    portalsNeedUpdated = true;
                } else if (instances[i].modelIndex != closedModelIndex && !currentState) {
                    activePortals[portalIdx].open = true;
                    activePortals[portalIdx].dirty = true;
                    portalsNeedUpdated = true;
                }
            }
        }
    }
    
    return portalsNeedUpdated;
}

void PortalCulling(void);

void UpdateAnims(void) {
    bool portalsNeedUpdated = false;
    uint16_t endOfModels = loadedInstances - invalidModelIndexCount;
    for (uint16_t i = START_INDEX_LEVEL_INSTANCES; i < endOfModels; ++i) {
        if (instances[i].animationNum >= MAX_ANIMATED_MODELS) continue; // Invalid animated model index
        if (instances[i].numclips >= MAX_ANIMATION_CLIPS_PER_MODEL) continue; // Invalid animation clip index
        if (instances[i].numclips == 0) continue; // Invalid animation clip index
        
        if (EntityIsAnimated(instances[i].index)) {
            if (StepLoopingAnim(i)) portalsNeedUpdated = true;
        }
    }
    
    if (portalsNeedUpdated) PortalCulling();
}

DepthSort visibleInstances[INSTANCE_COUNT];

RaycastHit Raycast(Vector3 origin, Vector3 dir, float maxDist, uint32_t layerMask) {
    RaycastHit result = {
        .hit = false,
        .distance = maxDist,
        .point = {0.0f, 0.0f, 0.0f},
        .normal = {0.0f, 0.0f, 0.0f},
        .hitInstanceIndex = INSTANCE_COUNT
    };
    
    uint16_t hitObjectIndex = UINT16_MAX;
    for (float curDist=0.0f;curDist<maxDist;curDist+=0.02f) { // 4.9 / 0.04 = 245 tries worst case empty air
        Vector3 checkPoint = Vector3_A_plus_B(origin, scale_vector3(dir,curDist));
        hitObjectIndex = PointInSolid(checkPoint, layerMask);
        if (hitObjectIndex < loadedInstances) {
            result.hit = true;
            result.point = checkPoint;
            result.distance = curDist; // TODO refine the raymarch a little?  nah 0.02 good enough for effects, will apply offset along normal for bullet holes and such anyways.
            result.normal = Vector3_A_minus_B(checkPoint,origin);
            result.hitInstanceIndex = hitObjectIndex;
            return result;
        }
    }
    
    return result;
}

DepthSort shadows_nearMeshes[SHADOW_NEARMESH_MAX]; // Found that this is typically around 172
float shadows_nearMeshRadii[SHADOW_NEARMESH_MAX];
bool UpdatedPlayerCell(void);

__attribute__((pure)) bool CellNotVisible(uint16_t index) {
    if (index > ARRSIZE) return false;
    
    bool cellNotVisible = !(gridCellStates[index] & CELL_VISIBLE);
    bool cellIsOpen = (gridCellStates[index] & CELL_OPEN); // For some shelves that are inset away from cells, need to still draw their items by checking && CELL_OPEN here, unfortunately this means they don't ever get culled :(
    return (cellNotVisible && cellIsOpen);
}

void RenderShadowmaps(float px, float py, float pz) {
    double shadowStartTime = get_time();
    voxen_Shadow_System.numGLCallsForShadows = 0;
    glEnable(GL_DEPTH_TEST);
    voxen_Shadow_System.numGLCallsForShadows++;
    LightCandidate candidates[MAX_SHADOWMAPS];
    uint16_t numberFoundLightCandidatesForShadows = 0;
    float bestScores[MAX_SHADOWMAPS];
    voxen_Shadow_System.numShadowsCouldRender = 0;
    for (uint16_t i = 0; i < loadedLights; ++i) { // Collect candidates: only lights that are enabled, within FAR_PLANE, and in PVS
        if (!lightCastsShadows[i]) continue;

        uint32_t litIdx = i * LIGHT_DATA_SIZE;
        float lightPosX = lights[litIdx];
        float lightPosY = lights[litIdx + LIGHT_DATA_OFFSET_POSY];
        float lightPosZ = lights[litIdx + LIGHT_DATA_OFFSET_POSZ];
        float intensity = lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY];
        if (intensity < 0.1f) continue;
        
        float range =  lights[litIdx + LIGHT_DATA_OFFSET_RANGE];
        float thresh = 0.015f;
        float luminosity = (intensity / (range * range));
        if (luminosity < thresh) continue;

        float dx = lightPosX - px;
        float dy = lightPosY - py;
        float dz = lightPosZ - pz;
        float distSqrdToPlayer = dx*dx + dy*dy + dz*dz;
        uint16_t cellX = (uint16_t)clamp((int32_t)vfloor((lightPosX - worldMin_x + CELLXHALF) / WORLDCELL_WIDTH_F), 0, WORLDX_0BASED);
        uint16_t cellZ = (uint16_t)clamp((int32_t)vfloor((lightPosZ - worldMin_z + CELLXHALF) / WORLDCELL_WIDTH_F), 0, WORLDX_0BASED);
        int lightCellIdx = (cellZ * WORLDX) + cellX;
        bool inPVS = (gridCellStates[lightCellIdx] & CELL_VISIBLE);
        if (!inPVS) {
            int r = vfloor(range * (1.0f / WORLDCELL_WIDTH_F));
            for (int ix = cellX - r; ix <= (int)cellX + r && !inPVS; ++ix) {
                for (int iz = cellZ - r; iz <= (int)cellZ + r; ++iz) {
                    if (!XZPairInBounds(ix, iz)) continue;
                    int subIdx = iz * WORLDX + ix;
                    if ((gridCellStates[subIdx] & CELL_VISIBLE) && get_cull_bit(precomputedVisibleCellsFromHere, lightCellIdx * ARRSIZE + subIdx)) {
                        inPVS = true;
                        break;
                    }
                }
            }
        }
        if (!inPVS) continue;

        float dotResult = dot(dx, dy, dz, cam_forwardx, cam_forwardy, cam_forwardz);
        if (dotResult < 0.0f && distSqrdToPlayer > (range * range)) continue;
        
        float score = distSqrdToPlayer / vmax(intensity, 0.01f);
        if (dotResult > 0.5f || distSqrdToPlayer < 26.2144f) score *= 0.125f; // Favor lights in player's view cone or within 5.12 (2 world cells)
        else if (dotResult > 0.0f) score *= 0.25f; // Favor lights in player's view cone

        if (numberFoundLightCandidatesForShadows < MAX_SHADOWMAPS) {
            candidates[numberFoundLightCandidatesForShadows] = (LightCandidate){ i, distSqrdToPlayer, score };
            bestScores[numberFoundLightCandidatesForShadows] = score;
            numberFoundLightCandidatesForShadows++;
        } else if (score < bestScores[0]) {  // Only compare against current worst
            // Find worst (highest score) and replace it
            int worstIdx = 0;
            for (uint32_t j = 1; j < numberFoundLightCandidatesForShadows; ++j) {
                if (bestScores[j] > bestScores[worstIdx]) worstIdx = j;
            }
            candidates[worstIdx] = (LightCandidate){ i, distSqrdToPlayer, score };
            bestScores[worstIdx] = score;
        }

        voxen_Shadow_System.numShadowsCouldRender++;
    }

    uint32_t numLightsShadowmapsToRender = vmin(voxen_Shadow_System.numShadowsCouldRender, MAX_SHADOWMAPS);
    if (numLightsShadowmapsToRender > 0) { // Added since there is now work between here and the for loop so this is beneficial to check.
        // Clear shadowmaps
        if (voxen_Shadow_System.useComputeClear) {
            glUseProgram(voxen_GL_Comms.shadowmapsClearShaderProgram); // Way faster
            GLuint groupX_shadClear = (numLightsShadowmapsToRender * (SHADOW_MAP_SIZE * SHADOW_MAP_SIZE * 6U) + 31) / 32;
            glDispatchCompute(groupX_shadClear,1,1);
            voxen_Shadow_System.numGLCallsForShadows += 3;
        } else {
            GLuint clearValue = 0xFFFFFFFFu;
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, voxen_GL_Comms.shadowMapSSBO);
            glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &clearValue); // Adds 72mb to RAM!!  Only used for fallback on some systems (e.g. HD4400) that can't use compute shader.
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
            voxen_Shadow_System.numGLCallsForShadows += 3;
        }

        voxen_Diagnostics.shadowDrawCallsRenderedThisFrame = 0;
        memset(voxen_Shadow_System.shadowmapIndirectionList, MAX_SHADOWMAPS + 1, loadedLights * sizeof(uint32_t)); // Set to invalid values for all
        glUseProgram(voxen_GL_Comms.shadowmapsShaderProgram);
        voxen_Shadow_System.numGLCallsForShadows++;
        uint32_t shadowmapOffsetHead = 0U;
        uint16_t endOfModels = loadedInstances - invalidModelIndexCount;
        uint16_t shadowCasterIndices[SHADOW_NEARMESH_MAX * MAX_SHADOWMAPS];
        uint16_t numShadowCasters = 0;
        for (int i=START_INDEX_LEVEL_INSTANCES;i<endOfModels;++i) {
            if (instances[i].modelIndex >= loadedModelsMaxIndex) continue;
            if (modelVertexCounts[instances[i].modelIndex] < 1) continue;
            if (instances[i].entflags & ENTFLAG_NO_SHADOWS) continue;
                
            uint16_t instCellIdx = PosGetCellCoords(instances[i].position.x, instances[i].position.z); // Cache cell indices once per mesh rather than once per light.
            bool cellNotVisible = (instCellIdx < ARRSIZE && CellNotVisible(instCellIdx));
            if (cellNotVisible && !(voxen_globalContext.currentLevel == 1 && (instances[i].index == 309 ||  instances[i].index == 532))) { // Hack for beaker and beaker holder on level 1 shelf getting culled from door portals.
                if (EntityIndexIsPortalBlockingDoor(instances[i].index) && instances[i].portalIndex < MAX_PORTALS) {
                    Portal doorPortal = activePortals[instances[i].portalIndex];
                    uint16_t cellAIndex = (doorPortal.cellA.z * WORLDX) + doorPortal.cellA.x;
                    uint16_t cellBIndex = (doorPortal.cellA.z * WORLDX) + doorPortal.cellA.x;
                    if (CellNotVisible(cellAIndex) && CellNotVisible(cellBIndex)) continue; // Neither cell is visible for door
                } else continue;
            }

            shadowCasterIndices[numShadowCasters] = i;
            numShadowCasters++;
            if (numShadowCasters >= (SHADOW_NEARMESH_MAX * MAX_SHADOWMAPS)) break; // Ran out of shadowcasters max for frame.
        }
        
        uint16_t numShadowingLightsHandled = 0;
        for (uint32_t c = 0; c < numLightsShadowmapsToRender; ++c) { // Render top MAX_SHADOWMAPS candidates
            uint16_t lightIdx = candidates[c].index;
            uint32_t litIdx = lightIdx * LIGHT_DATA_SIZE;
            float litX = lights[litIdx];
            float litY = lights[litIdx + LIGHT_DATA_OFFSET_POSY];
            float litZ = lights[litIdx + LIGHT_DATA_OFFSET_POSZ];
            float lightRadius = lights[litIdx + LIGHT_DATA_OFFSET_RANGE];
            float effectiveRadius = vmin(lightRadius, 15.36f);
            glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
            voxen_Shadow_System.numGLCallsForShadows++;
            uint16_t nearbyMeshCount = 0;
            for (uint16_t shadowCasterInstanceIdx = 0; shadowCasterInstanceIdx < numShadowCasters; shadowCasterInstanceIdx++) { // Skip player indices and start at 3
                uint16_t j = shadowCasterIndices[shadowCasterInstanceIdx];
                shadows_nearMeshRadii[nearbyMeshCount] = modelBounds[(instances[j].modelIndex * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_RADIUS];
                float distToLightSqrd = squareDistance3D(instances[j].position.x, instances[j].position.y, instances[j].position.z, litX, litY, litZ);
                float radSum = (effectiveRadius + shadows_nearMeshRadii[nearbyMeshCount]);
                if (distToLightSqrd > radSum * radSum) continue;
                
                shadows_nearMeshes[nearbyMeshCount].index = j;
                shadows_nearMeshes[nearbyMeshCount].depth = distToLightSqrd; 
                nearbyMeshCount++;
                if (nearbyMeshCount >= SHADOW_NEARMESH_MAX) { DualLogWarn("Shadowmapping needs larger nearMeshes count than %u!  Skipping some renderables for light %u!\n", SHADOW_NEARMESH_MAX, lightIdx); break; }
            }

            if (nearbyMeshCount < 1) continue;
            
            qsort(shadows_nearMeshes, nearbyMeshCount, sizeof(DepthSort), compareDepthSortInverted); // Sort by depth (ascending for front-to-back)
            glUniform3f(3, litX, litY, litZ);
            voxen_Shadow_System.numGLCallsForShadows++;
            voxen_Shadow_System.shadowmapIndirectionList[lightIdx] = numShadowingLightsHandled;
            uint16_t currentModelType = 0;
            uint16_t currentTexIndex = 0;
            bool currentIsTransparent = 0;
            Vector3 corners[8] = {
                { litX + lightRadius, litY + lightRadius, litZ + lightRadius },
                { litX + lightRadius, litY + lightRadius, litZ - lightRadius },
                { litX + lightRadius, litY - lightRadius, litZ + lightRadius },
                { litX + lightRadius, litY - lightRadius, litZ - lightRadius },
                { litX - lightRadius, litY + lightRadius, litZ + lightRadius },
                { litX - lightRadius, litY + lightRadius, litZ - lightRadius },
                { litX - lightRadius, litY - lightRadius, litZ + lightRadius },
                { litX - lightRadius, litY - lightRadius, litZ - lightRadius }
            };
            
            bool lightPositionInPlayerFrustum = SphereInFrustum(playerFrustumPlanes, litX, litY, litZ, 0.64f); // Use some radius for floating point errors
            for (uint8_t face = 0; face < 6; face++) {                            
                if (!lightPositionInPlayerFrustum) { // Check if at least one of the four points of this cubemap face's frustum are within the player's frustum
                    bool faceOverlapsPlayerView = false;
                    switch (face) {
                        case 0: // +X: Right (CONFIRMED, skipping face 0 causes shadow to not cast in more positive X direction (e.g. positions more +x from light don't get shadows, aligns to face.)
                            if (dot_vector3(Vector3_A_minus_B(corners[0],instances[PLAYER1].position), (Vector3){ cam_forwardx, cam_forwardy, cam_forwardz }) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }
                            if (dot_vector3(Vector3_A_minus_B(corners[1],instances[PLAYER1].position), (Vector3){ cam_forwardx, cam_forwardy, cam_forwardz }) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }
                            if (dot_vector3(Vector3_A_minus_B(corners[2],instances[PLAYER1].position), (Vector3){ cam_forwardx, cam_forwardy, cam_forwardz }) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }
                            if (dot_vector3(Vector3_A_minus_B(corners[3],instances[PLAYER1].position), (Vector3){ cam_forwardx, cam_forwardy, cam_forwardz }) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }
                            break;
                        case 1: // -X: Left (CONFIRMED, skipping face 1 causes shadow to not cast in more negative X direction.)
                            if (dot_vector3(Vector3_A_minus_B(corners[4],instances[PLAYER1].position), (Vector3){ cam_forwardx, cam_forwardy, cam_forwardz }) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }
                            if (dot_vector3(Vector3_A_minus_B(corners[5],instances[PLAYER1].position), (Vector3){ cam_forwardx, cam_forwardy, cam_forwardz }) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }
                            if (dot_vector3(Vector3_A_minus_B(corners[6],instances[PLAYER1].position), (Vector3){ cam_forwardx, cam_forwardy, cam_forwardz }) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }
                            if (dot_vector3(Vector3_A_minus_B(corners[7],instances[PLAYER1].position), (Vector3){ cam_forwardx, cam_forwardy, cam_forwardz }) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }
                            break;
                        case 2: // +Y: Up (CONFIRMED, skipping face 1 causes shadow to not cast in more positive Y direction.)
                            if (dot_vector3(Vector3_A_minus_B(corners[0],instances[PLAYER1].position), (Vector3){ cam_forwardx, cam_forwardy, cam_forwardz }) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }
                            if (dot_vector3(Vector3_A_minus_B(corners[1],instances[PLAYER1].position), (Vector3){ cam_forwardx, cam_forwardy, cam_forwardz }) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }
                            if (dot_vector3(Vector3_A_minus_B(corners[4],instances[PLAYER1].position), (Vector3){ cam_forwardx, cam_forwardy, cam_forwardz }) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }
                            if (dot_vector3(Vector3_A_minus_B(corners[5],instances[PLAYER1].position), (Vector3){ cam_forwardx, cam_forwardy, cam_forwardz }) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }
                            break;
                        case 3: // -Y: Down (CONFIRMED, skipping face 1 causes shadow to not cast in more negative Y direction.)
                            if (dot_vector3(Vector3_A_minus_B(corners[2],instances[PLAYER1].position), (Vector3){ cam_forwardx, cam_forwardy, cam_forwardz }) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }
                            if (dot_vector3(Vector3_A_minus_B(corners[3],instances[PLAYER1].position), (Vector3){ cam_forwardx, cam_forwardy, cam_forwardz }) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }
                            if (dot_vector3(Vector3_A_minus_B(corners[6],instances[PLAYER1].position), (Vector3){ cam_forwardx, cam_forwardy, cam_forwardz }) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }
                            if (dot_vector3(Vector3_A_minus_B(corners[7],instances[PLAYER1].position), (Vector3){ cam_forwardx, cam_forwardy, cam_forwardz }) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }
                            break;
                        case 4: // +Z: Forward (CONFIRMED, skipping face 1 causes shadow to not cast in more positive Z direction.)
                            if (dot_vector3(Vector3_A_minus_B(corners[0],instances[PLAYER1].position), (Vector3){ cam_forwardx, cam_forwardy, cam_forwardz }) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }
                            if (dot_vector3(Vector3_A_minus_B(corners[2],instances[PLAYER1].position), (Vector3){ cam_forwardx, cam_forwardy, cam_forwardz }) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }
                            if (dot_vector3(Vector3_A_minus_B(corners[4],instances[PLAYER1].position), (Vector3){ cam_forwardx, cam_forwardy, cam_forwardz }) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }
                            if (dot_vector3(Vector3_A_minus_B(corners[6],instances[PLAYER1].position), (Vector3){ cam_forwardx, cam_forwardy, cam_forwardz }) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }
                            break;
                        case 5: // -Z: Backward (CONFIRMED, skipping face 1 causes shadow to not cast in more negative Z direction.)
                            if (dot_vector3(Vector3_A_minus_B(corners[1],instances[PLAYER1].position), (Vector3){ cam_forwardx, cam_forwardy, cam_forwardz }) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }
                            if (dot_vector3(Vector3_A_minus_B(corners[3],instances[PLAYER1].position), (Vector3){ cam_forwardx, cam_forwardy, cam_forwardz }) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }
                            if (dot_vector3(Vector3_A_minus_B(corners[5],instances[PLAYER1].position), (Vector3){ cam_forwardx, cam_forwardy, cam_forwardz }) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }
                            if (dot_vector3(Vector3_A_minus_B(corners[7],instances[PLAYER1].position), (Vector3){ cam_forwardx, cam_forwardy, cam_forwardz }) > voxen_Shadow_System.shadDotThresh) { faceOverlapsPlayerView = true; break; }
                            break;
                    }

                    if (!faceOverlapsPlayerView) {
                        if (!SphereInFrustum(lightFrustumPlanes[lightIdx][face], px, py, pz, 0.48f)) continue;
                    }
                }
                
                glUniform1ui(2, face);
                glUniformMatrix4fv(1, 1, GL_FALSE, (float*)lightViewProj[lightIdx][face]);
                glUniform1ui(7, shadowmapOffsetHead + (face * 36864));
                voxen_Shadow_System.numGLCallsForShadows += 3;
                voxen_Diagnostics.shadowDrawCallsRenderedThisFrame++;
                for (uint16_t j = 0; j < nearbyMeshCount; ++j) {
                    int i = shadows_nearMeshes[j].index;            
                    if (!SphereInFrustum(lightFrustumPlanes[lightIdx][face], instances[i].position.x, instances[i].position.y, instances[i].position.z, shadows_nearMeshRadii[j] * 1.41f)) continue;

                    int32_t modelType = instanceIsLODArray[i] && instances[i].lodIndex < loadedModelsMaxIndex ? instances[i].lodIndex : instances[i].modelIndex;
                    if (currentModelType != modelType) {
                        currentModelType = modelType;
                        glBindVertexBuffer(0, voxen_GL_Comms.vbos[currentModelType], 0, VERTEX_ATTRIBUTES_COUNT * sizeof(float));
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, voxen_GL_Comms.tbos[currentModelType]);
                        voxen_Shadow_System.numGLCallsForShadows += 2;
                    }
                    
                    glUniform1ui(0, i);
                    voxen_Shadow_System.numGLCallsForShadows++;
                    if (currentTexIndex != instances[i].texIndex) { currentTexIndex = instances[i].texIndex; glUniform1ui(6, instances[i].texIndex); voxen_Shadow_System.numGLCallsForShadows++; }
                    if (currentIsTransparent != isTransparent(instances[i].texIndex)) { currentIsTransparent = isTransparent(instances[i].texIndex); glUniform1ui(8, isTransparent(instances[i].texIndex)); voxen_Shadow_System.numGLCallsForShadows++; }
                    glDrawElements(GL_TRIANGLES, modelTriangleCounts[currentModelType] * 3, GL_UNSIGNED_INT, 0);
                    voxen_Shadow_System.numGLCallsForShadows++;
                    voxen_Diagnostics.drawCallsRenderedThisFrame++;
                    voxen_Diagnostics.verticesRenderedThisFrame += modelTriangleCounts[currentModelType] * 3;
                }
            }

            shadowmapOffsetHead += (SHADOW_MAP_SIZE * SHADOW_MAP_SIZE) * 6;
            if (shadowmapOffsetHead > TOTAL_SHADOWMAP_PIXELS) { DualLogWarn("Early exit on shadowmap loop due to undersized SSBO\n"); break; }

            numShadowingLightsHandled++;
        }

        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT);
        glViewport(0, 0, voxen_Settings.ScreenWidth, voxen_Settings.ScreenHeight);
        glNamedBufferData(voxen_GL_Comms.shadowMapsIndirectionID, loadedLights * sizeof(uint32_t), voxen_Shadow_System.shadowmapIndirectionList, GL_DYNAMIC_DRAW);
        voxen_Shadow_System.numGLCallsForShadows += 4;
    }
    
    voxen_Shadow_System.shadowTime = get_time() - shadowStartTime;
}

void RenderCompositePass(float px, float py, float pz, float* viewProj, float* invViewRot) {
    glUseProgram(voxen_GL_Comms.imageBlitShaderProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, voxen_GL_Comms.inputImageID);
    glUniform1i(4, 4); // outputImage texture sampler2D
    glUniform1ui(5, voxen_Settings.Reflections);
    glUniform1ui(6, voxen_Settings.AntiAliasing);
    float berserkTimeRemainingNormalized = berserkFinished > 0.0001f ? (berserkFinished - (float)voxen_globalContext.pauseRelativeTime) / PATCH_TIME_BERSERK : 0.0f;
    if (berserkFinished < (float)voxen_globalContext.pauseRelativeTime && berserkFinished > 0.0001f) berserkFinished = berserkTimeRemainingNormalized = 0.0f;
    glUniform1f(9, berserkTimeRemainingNormalized);
    glUniform1f(10, berserkSeedTime);
    glUniform1ui(11, voxen_Settings.Brightness);
    glUniform3f(12, deg2rad(cam_yaw), deg2rad(cam_pitch), deg2rad(cam_roll));
    glUniform3f(13, px, py, pz);
    glUniform1f(14, voxen_Settings.FOV);
    glUniform1f(15, (float)voxen_globalContext.pauseRelativeTime * 0.1f);
    glUniform1f(16, aspect3D);
    glUniform1ui(17, (gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX) || voxen_globalContext.currentLevel == LEVEL_CYBERSPACE);
    glUniform1ui(18, (gridCellStates[playerCellIdx] & CELL_SEES_SUN) && voxen_globalContext.currentLevel != LEVEL_CYBERSPACE);
    glUniform1ui(19, ((voxen_globalContext.currentLevel >= 10 && voxen_globalContext.currentLevel < LEVEL_CYBERSPACE) ? 1u : 0u) && (gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX));
    uint32_t shieldOnType = 0u; // No shield green tint.
    if (questData.ShieldActivated) {
        if (voxen_globalContext.currentLevel == 6 || voxen_globalContext.currentLevel == 7) shieldOnType = 2u; // Shielding only below player for lower levels.
        else if (voxen_globalContext.currentLevel <= 5) shieldOnType = 1u; // Shielding everywhere as levels fully within shield.
    }
    
    glUniform1ui(20, shieldOnType);
    glUniform1ui(22, voxen_Settings.Shadows);
    Color painStaticColor = GetPainStaticColor();
    glUniform3f(23, painStaticColor.r, painStaticColor.g, painStaticColor.b);
    glUniformMatrix4fv(24, 1, GL_FALSE, viewProj);
    glUniformMatrix3fv(25, 1, GL_FALSE, invViewRot);
    glUniform1i(27, 0); // Texture 0 for the rendered geometry color buffer
    glUniform1f(28, GetPainStatic());
    glBindVertexArray(voxen_GL_Comms.quadVAO);
    glDisable(GL_DEPTH_TEST);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    voxen_Diagnostics.drawCallsRenderedThisFrame++;
    voxen_Diagnostics.verticesRenderedThisFrame += 4;
}

double RenderUI(void) {
    voxen_Diagnostics.drawCallsNormal = voxen_Diagnostics.drawCallsRenderedThisFrame;
    float screenCenterX = (float)voxen_Settings.ScreenWidth / 2;
    float screenCenterY = (float)voxen_Settings.ScreenHeight / 2;
    float lineSpacing = GetScreenRelativeY(genericTextHeightFac);
    if (voxen_globalContext.gamePaused) {
        float pauseBGWidth = GetScreenRelativeX(0.24f), pauseBGHeight = GetScreenRelativeY(0.39f);
        float pauseBGX = screenCenterX - (pauseBGWidth * 0.5f);
        float pauseBGY = screenCenterY - (pauseBGHeight * 0.5f) + GetScreenRelativeY(0.08f);
        RenderUIImage(pauseBGX, pauseBGY, pauseBGWidth, pauseBGHeight, 1025); // Pause Menu background
        RenderUIImage(pauseBGX, pauseBGY, pauseBGWidth, pauseBGHeight, 1080); // Pause Menu background
        float quitGame_Height = GetScreenRelativeY(0.05f);
        RenderUIImage(pauseBGX, screenCenterY + GetScreenRelativeY(0.40f) - (quitGame_Height * 0.5f), pauseBGWidth, quitGame_Height, 950); // Pause Quit Game background
        
        RenderFormattedText(screenCenterX - GetScreenRelativeX(genericTextWidthFacStopD * 3.0f), screenCenterY - GetScreenRelativeY(0.3f), TEXT_STOPD_RED_PAUSETITLE, FONT_STOPD, "PAUSED");
        char* pauseButton_ResumeText = "RESUME";
        float pauseButton_ResumeWidth = (TextWidth(pauseButton_ResumeText,FONT_STOPD) * 0.5f);
        float pauseButton_ResumeHeight = GetScreenRelativeY(genericTextHeightFacStopD);
        float pauseButton_ResumeX = screenCenterX - pauseButton_ResumeWidth;
        float pauseButton_ResumeY = screenCenterY - GetScreenRelativeY(0.08f);
        uint8_t pauseButton_ResumeColor = TEXT_STOPD_RED;
        bool pauseButton_CursorIsAbove = CursorIsOverBounds(pauseButton_ResumeX - GetScreenRelativeX(genericTextWidthFacStopD), pauseButton_ResumeX + pauseButton_ResumeWidth,
                                                            pauseButton_ResumeY + (pauseButton_ResumeHeight * 0.5f), pauseButton_ResumeY - (pauseButton_ResumeHeight * 0.5f));
        
        if (pauseButton_CursorIsAbove) pauseButton_ResumeColor = TEXT_STOPD_RED_HIGHLIGHT;
        RenderFormattedText(pauseButton_ResumeX, pauseButton_ResumeY, pauseButton_ResumeColor, FONT_STOPD, "RESUME");
        
        RenderFormattedText(screenCenterX - GetScreenRelativeX(genericTextWidthFacStopD * 2.0f), screenCenterY + GetScreenRelativeY(0.00f), TEXT_STOPD_RED, FONT_STOPD, "LOAD");
        RenderFormattedText(screenCenterX - GetScreenRelativeX(genericTextWidthFacStopD * 2.0f), screenCenterY + GetScreenRelativeY(0.08f), TEXT_STOPD_RED, FONT_STOPD, "SAVE");
        RenderFormattedText(screenCenterX - GetScreenRelativeX(genericTextWidthFacStopD * 3.5f), screenCenterY + GetScreenRelativeY(0.16f), TEXT_STOPD_RED, FONT_STOPD, "OPTIONS");
        RenderFormattedText(screenCenterX - GetScreenRelativeX(genericTextWidthFacStopD * 6.0f), screenCenterY + GetScreenRelativeY(0.24f), TEXT_STOPD_RED, FONT_STOPD, "QUIT TO MENU");
        RenderFormattedText(screenCenterX - GetScreenRelativeX(genericTextWidthFacStopD * 4.5f), screenCenterY + GetScreenRelativeY(0.40f), TEXT_STOPD_RED, FONT_STOPD, "QUIT GAME");
    }
    
    // Diagnostics / Debugging
    float debugTextStartY = GetScreenRelativeY(0.075f);
    float leftPad = GetScreenRelativeX(0.0125f);
    if (!voxen_Cheats.noHUD && voxen_Cheats.showLocation) RenderFormattedText(leftPad, debugTextStartY, TEXT_WHITE, FONT_NORMAL, "x: %.4f, y: %.4f, z: %.4f", (double)instances[PLAYER1].position.x, (double)instances[PLAYER1].position.y, (double)instances[PLAYER1].position.z);
    if (!voxen_Cheats.noHUD) RenderFormattedText(leftPad, debugTextStartY + (lineSpacing * 1), TEXT_WHITE, FONT_NORMAL, "timeSinceLastPhysicsTick: %.6f, numShadowsCouldRender: %u, playerCellIdx: %u, numCellsVisible: %u", voxen_globalContext.timeSinceLastPhysicsTick, voxen_Shadow_System.numShadowsCouldRender, playerCellIdx, numCellsVisible);
    if (!voxen_Cheats.noHUD) RenderFormattedText(leftPad, debugTextStartY + (lineSpacing * 2), TEXT_WHITE, FONT_NORMAL, "Player velocity: %.2f, %.2f, %.2f, accumulated force: %.2f, %.2f, %.2f", (double)instances[PLAYER1].velocity.x, (double)instances[PLAYER1].velocity.y, (double)instances[PLAYER1].velocity.z, (double)instances[PLAYER1].accumulatedForce.x, (double)instances[PLAYER1].accumulatedForce.y, (double)instances[PLAYER1].accumulatedForce.z);
    if (!voxen_Cheats.noHUD) RenderFormattedText(leftPad, debugTextStartY + (lineSpacing * 3), TEXT_WHITE, FONT_NORMAL, "Debug line start: %.2f, %.2f, %.2f, end: %.2f, %.2f, %.2f", (double)voxen_Diagnostics.debugLine_startX, (double)voxen_Diagnostics.debugLine_startY, (double)voxen_Diagnostics.debugLine_startZ, (double)voxen_Diagnostics.debugLine_endX, (double)voxen_Diagnostics.debugLine_endY, (double)voxen_Diagnostics.debugLine_endZ);
    if (!voxen_Cheats.noHUD) RenderFormattedText(leftPad, debugTextStartY + (lineSpacing * 4), TEXT_WHITE, FONT_NORMAL, "Test Entity %s Index: %u, Player inside: %u, named %s, St: %.3f ms, shadGL: %u", GetPrefabNameFromIndex(instances[editModeSelection].index), editModeTestEntityDefinition, testPointInSolid, testPointInSolid == UINT16_MAX ? "-" : GetPrefabNameFromIndex(instances[testPointInSolid].index), voxen_Shadow_System.shadowTime * 1000, voxen_Shadow_System.numGLCallsForShadows);
    if (voxen_Cheats.consoleActive) RenderFormattedText(leftPad, 0, TEXT_WHITE, FONT_NORMAL, "] %s",consoleEntryText);
    if (voxen_globalContext.statusTextDecayFinished > voxen_globalContext.current_time) RenderFormattedText(leftPad + (voxen_Settings.ScreenWidth / 2) - 220, screenCenterY - GetScreenRelativeY(0.30f + (genericTextHeightFac * 2.0f)), TEXT_WHITE, FONT_NORMAL, "%s",statusText);

    double time_now = get_time();
    if (voxen_Cheats.showFPS && !voxen_Cheats.noHUD) {
        double thisFrameTime = (time_now - voxen_globalContext.last_time) * 1000.0;
        double cpuFrameTime = voxen_Diagnostics.cpuTime * 1000.0;
        uint8_t timingColor = TEXT_WHITE;
        if (vabs(thisFrameTime - cpuFrameTime) < 0.451) timingColor = TEXT_GREEN;
        if (thisFrameTime > 6.944444) timingColor = TEXT_RED;
        voxen_Diagnostics.drawCallsRenderedThisFrame += 2; voxen_Diagnostics.textDrawCallsRenderedThisFrame += 2; // Add two more for this text render ;)
        RenderFormattedText(leftPad, debugTextStartY - lineSpacing, timingColor, FONT_NORMAL, "ms: %.2f, CPU %.2f", thisFrameTime,cpuFrameTime);
        RenderFormattedText(leftPad + 230.0f, debugTextStartY - lineSpacing, TEXT_WHITE, FONT_NORMAL, "(FPS: %d, Worst: %d), Drwclls: %d [G %d UI %d Txt %d Shd %d] Vrts: %d Edit:%u",
                            voxen_Diagnostics.framesPerLastSecond, voxen_Diagnostics.worstFPS, voxen_Diagnostics.drawCallsRenderedThisFrame, voxen_Diagnostics.drawCallsNormal, voxen_Diagnostics.uiImageDrawCallsRenderedThisFrame,
                            voxen_Diagnostics.textDrawCallsRenderedThisFrame, voxen_Diagnostics.shadowDrawCallsRenderedThisFrame, voxen_Diagnostics.verticesRenderedThisFrame, voxen_Cheats.editMode);
    }
    
    float shootModeWidth = GetScreenRelativeX(0.01639f), shootModeHeight = GetScreenRelativeX(0.01639f);
    float shootModePos_x = GetScreenRelativeX(0.5f) - (shootModeWidth * 0.5f);
    float shootModePos_y = 0.0f;
    if (!voxen_globalContext.gamePaused && !voxen_Cheats.noHUD) RenderUIImage(shootModePos_x, shootModePos_y, shootModeWidth, shootModeHeight, 1020); // Shoot mode button
    if (voxen_globalContext.inventoryMode) {
        if (CursorIsOverBounds(shootModePos_x, shootModePos_x + shootModeWidth, shootModePos_y + shootModeHeight, shootModePos_y)) {
            if (mouseButtons[GLFW_MOUSE_BUTTON_LEFT].released) {
                voxen_globalContext.inventoryMode = false;
                cursorPosition_x = voxen_Settings.ScreenWidth / 2;
                cursorPosition_y = voxen_Settings.ScreenHeight / 2;
            }
        }
    }
    
    // Cursor [ /// VERY LAST DRAWN OVER EVERYTHING ELSE! /// ]
    bool menuOrInventoryCursorStyle = (voxen_globalContext.gamePaused || voxen_globalContext.menuActive);
    uint16_t cursorTexture = menuOrInventoryCursorStyle ? 1261 : 1260;
    float cursorSize = (float)voxen_Settings.ScreenWidth * CURSOR_SCREEN_PERCENTAGE * (menuOrInventoryCursorStyle ? 3.0f : 1.0f);
    float cursorHalfSize = cursorSize * 0.5f;
    if (CursorVisible()) RenderUIImage(cursorPosition_x - cursorHalfSize, cursorPosition_y - cursorHalfSize, cursorSize, cursorSize, cursorTexture);
    else RenderUIImage(screenCenterX - cursorHalfSize, screenCenterY - cursorHalfSize, cursorSize, cursorSize, cursorTexture);
    
    return time_now;
}

void Frob(Vector3 pos) {
    float offsetX = cursorPosition_x - (voxen_Settings.ScreenWidth * 0.5f);
    float offsetY = cursorPosition_y - (voxen_Settings.ScreenHeight * 0.5f);
    float ndcX = offsetX / (voxen_Settings.ScreenWidth * 0.5f);
    float ndcY = -offsetY / (voxen_Settings.ScreenHeight * 0.5f);  // flip Y
    float tanFov = tanf(voxen_Settings.FOV * 0.5f * PI / 180.0f);
    float viewX = ndcX * tanFov * aspect3D;
    float viewY = ndcY * tanFov;
    float viewZ = -1.0f;
    float len = sqrtf(viewX*viewX + viewY*viewY + viewZ*viewZ);
    viewX /= len; viewY /= len; viewZ /= len;
    float upX = cam_righty * (-cam_forwardz) - cam_rightz * (-cam_forwardy);
    float upY = cam_rightz * (-cam_forwardx) - cam_rightx * (-cam_forwardz);
    float upZ = cam_rightx * (-cam_forwardy) - cam_righty * (-cam_forwardx);
    float upLen = sqrtf(upX*upX + upY*upY + upZ*upZ);
    if (upLen > 0.001f) { upX /= upLen; upY /= upLen; upZ /= upLen; }
    Vector3 dir = (Vector3){ viewX * cam_rightx + viewY * upX + viewZ * (-cam_forwardx), viewX * cam_righty + viewY * upY + viewZ * (-cam_forwardy), viewX * cam_rightz + viewY * upZ + viewZ * (-cam_forwardz) };
    voxen_Diagnostics.debugLine_startX = pos.x;
    voxen_Diagnostics.debugLine_startY = pos.y;
    voxen_Diagnostics.debugLine_startZ = pos.z;
    voxen_Diagnostics.debugLine_endX   = pos.x + dir.x * FROB_DISTANCE;
    voxen_Diagnostics.debugLine_endY   = pos.y + dir.y * FROB_DISTANCE;
    voxen_Diagnostics.debugLine_endZ   = pos.z + dir.z * FROB_DISTANCE;
    RaycastHit tempHit = Raycast(pos, dir, FROB_DISTANCE, LAYER_MASK_PLAYER_FROB);
    if (tempHit.hit) {
        voxen_Diagnostics.debugLine_endX   = tempHit.point.x;
        voxen_Diagnostics.debugLine_endY   = tempHit.point.y;
        voxen_Diagnostics.debugLine_endZ   = tempHit.point.z;
        DualLog("Raycast hit!  Hit object %u named of entity type %s(%u) at hit point %f %f %f\n", tempHit.hitInstanceIndex,
                GetPrefabNameFromIndex(instances[tempHit.hitInstanceIndex].index), instances[tempHit.hitInstanceIndex].index,
                (double)tempHit.point.x, (double)tempHit.point.y, (double)tempHit.point.z);
    }
    
    voxen_Diagnostics.debugLineFinished = voxen_globalContext.current_time + 3.0;
}

void UpdateGameplay(void) {
    if (mouseButtons[GLFW_MOUSE_BUTTON_2].released) Frob(instances[PLAYER1].position);
    if (voxen_globalContext.current_time < voxen_Diagnostics.debugLineFinished) {
        AddDebugLine(voxen_Diagnostics.debugLine_startX, voxen_Diagnostics.debugLine_startY, voxen_Diagnostics.debugLine_startZ, voxen_Diagnostics.debugLine_endX, voxen_Diagnostics.debugLine_endY, voxen_Diagnostics.debugLine_endZ);
    }
    
    UpdateAmbientSounds();
    UpdateAnims();
}

void Render(void) {
    voxen_Diagnostics.drawCallsRenderedThisFrame = 0; // Reset per frame
    voxen_Diagnostics.textDrawCallsRenderedThisFrame = 0;
    voxen_Diagnostics.uiImageDrawCallsRenderedThisFrame = 0;
    voxen_Diagnostics.shadowDrawCallsRenderedThisFrame = 0;
    voxen_Diagnostics.verticesRenderedThisFrame = 0;
    
    // Frame prep, View Matrix, and Projection Matrix
    float view[16]; // Also known as view matrix
    float px = instances[PLAYER1].position.x;
    float py = instances[PLAYER1].position.y;
    float pz = instances[PLAYER1].position.z;
    mat4_lookat_from(view,&cam_rotation, px, py, pz);
    float viewProj[16]; // view-projection matrix
    mul_mat4(viewProj, rasterPerspectiveProjection, view);
    float invViewRot[9];
    invViewRot[0] = view[0]; invViewRot[3] = view[1]; invViewRot[6] = view[2];
    invViewRot[1] = view[4]; invViewRot[4] = view[5]; invViewRot[7] = view[6];
    invViewRot[2] = view[8]; invViewRot[5] = view[9]; invViewRot[8] = view[10];
    ExtractFrustumPlanes(viewProj, playerFrustumPlanes);
    if (!voxen_globalContext.gamePaused && !voxen_globalContext.menuActive) { // !PAUSED BLOCK -------------------------------------------------   
        if (voxen_Settings.Shadows > 0u) RenderShadowmaps(px, py, pz);
        memset(lightDirty,0,LIGHT_COUNT * sizeof(bool));         // Clear dirty after shadowmaps for minimal shadowmap updating.
        memset(dirtyInstances,0,loadedInstances * sizeof(bool)); // Clear dirty after shadowmaps for minimal shadowmap updating.
        
        // 4. Raterized Geometry, Standard vertex + fragment rendering, but with special packing to minimize transfer data amounts
        glBindFramebuffer(GL_FRAMEBUFFER, voxen_GL_Comms.gBufferFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the corner where last shadowmap wrote into  
        glUseProgram(voxen_GL_Comms.chunkShaderProgram);
        glUniformMatrix4fv(2, 1, GL_FALSE, viewProj);
        glUniform1ui(3, 0u); // isUI false
        glUniform1f(8, worldMin_x);
        glUniform1f(9, worldMin_z);
        glUniform3f(10, px, py, pz);
        glUniform1ui(14, voxen_Settings.Reflections);
        glUniform1ui(15, voxen_Settings.Shadows);
        glUniform1ui(17, 0u); // unlit false
        glEnable(GL_CULL_FACE); // Opaques
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        uint16_t visibleCount = 0;
        uint32_t currentTexIndex = 0;
        uint32_t currentNormIndex = 0;
        uint32_t currentGlowIndex = 0;
        uint32_t currentSpecIndex = 0;
        uint16_t currentModelType = 0;
        for (uint16_t i = START_INDEX_LEVEL_INSTANCES; i < startOfDoubleSidedInstances; ++i) {
            float objx = instances[i].position.x;
            float objy = instances[i].position.y;
            float objz = instances[i].position.z;
            uint16_t instCellIdx = PosGetCellCoords(objx, objz);
            float dx = objx - px;
            float dy = objy - py;
            float dz = objz - pz;
            float distSqrd = dx*dx + dy*dy + dz*dz;
            if (distSqrd >= FAR_PLANE_SQUARED) continue;

            if (EntityIndexIsPortalBlockingDoor(instances[i].index)) { // Extra checks only needed for opaque portal blocking doors.
                bool inPVS = (gridCellStates[instCellIdx] & CELL_VISIBLE);
                if (!inPVS) {
                    uint16_t cellX = (uint16_t)clamp((int32_t)vfloor((objx - worldMin_x + CELLXHALF) / WORLDCELL_WIDTH_F), 0, WORLDX_0BASED);
                    uint16_t cellZ = (uint16_t)clamp((int32_t)vfloor((objz - worldMin_z + CELLXHALF) / WORLDCELL_WIDTH_F), 0, WORLDX_0BASED);
                    int r = vfloor(5.12f * (1.0f / WORLDCELL_WIDTH_F));
                    for (int ix = cellX - r; ix <= (int)cellX + r && !inPVS; ++ix) {
                        for (int iz = cellZ - r; iz <= (int)cellZ + r; ++iz) {
                            if (!XZPairInBounds(ix, iz)) continue;
                            int subIdx = iz * WORLDX + ix;
                            if ((gridCellStates[subIdx] & CELL_VISIBLE) && get_cull_bit(precomputedVisibleCellsFromHere, instCellIdx * ARRSIZE + subIdx)) {
                                inPVS = true;
                                break;
                            }
                        }
                    }
                }
                if (!inPVS) continue;
            } else {
                if (instCellIdx < ARRSIZE && CellNotVisible(instCellIdx)) continue;
                
                if (!(gridCellStates[instCellIdx] & CELL_OPEN)) {
                    if (distSqrd >= 943.7184f) continue; // 30.72 * 30.72, 12 cells
                }
            }
            
            float dotResult = dot(dx, dy, dz, cam_forwardx, cam_forwardy, cam_forwardz);
            float radius = modelBounds[(instances[i].modelIndex * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_RADIUS] * 2.0f;
            if (dotResult < 0.0f && distSqrd > (radius * radius)) continue;
            
            visibleInstances[visibleCount].index = i;
            visibleInstances[visibleCount].depth = distSqrd;
            visibleCount++;
        }
        
        if (visibleCount > 1) qsort(visibleInstances, visibleCount, sizeof(DepthSort), compareDepthSortInverted); // Sort by depth (ascending for front-to-back)
        for (uint16_t visibleIndex = 0; visibleIndex < visibleCount; ++visibleIndex) {
            uint16_t i = visibleInstances[visibleIndex].index;
            glUniform1ui(0, i);
            if (currentNormIndex != (uint32_t)instances[i].normIndex) { currentNormIndex = (uint32_t)instances[i].normIndex; glUniform1ui(1, currentNormIndex); }
            if (currentTexIndex  != (uint32_t)instances[i].texIndex)  { currentTexIndex  =  (uint32_t)instances[i].texIndex; glUniform1ui(18, currentTexIndex); }
            if (currentGlowIndex != (uint32_t)instances[i].glowIndex) { currentGlowIndex = (uint32_t)instances[i].glowIndex; glUniform1ui(19, currentGlowIndex); }
            if (currentSpecIndex != (uint32_t)instances[i].specIndex) { currentSpecIndex = (uint32_t)instances[i].specIndex; glUniform1ui(20, currentSpecIndex); }
            int32_t modelType = instanceIsLODArray[i] && instances[i].lodIndex < loadedModelsMaxIndex ? instances[i].lodIndex : instances[i].modelIndex;
            if (currentModelType != modelType) {
                currentModelType = modelType;
                glBindVertexBuffer(0, voxen_GL_Comms.vbos[currentModelType], 0, VERTEX_ATTRIBUTES_COUNT * sizeof(float));
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, voxen_GL_Comms.tbos[currentModelType]);
            }
            
            uint32_t vertCount = modelTriangleCounts[currentModelType] * 3;
            glDrawElements(GL_TRIANGLES, vertCount, GL_UNSIGNED_INT, 0);
            voxen_Diagnostics.drawCallsRenderedThisFrame++;
            voxen_Diagnostics.verticesRenderedThisFrame += vertCount;
        }
        
        glDisable(GL_CULL_FACE); glEnable(GL_BLEND); // Doublesided
        for (uint16_t i = startOfDoubleSidedInstances; i < startOfTransparentInstances; ++i) {
            float objx = instances[i].position.x;
            float objz = instances[i].position.z;
            uint16_t instCellIdx = PosGetCellCoords(objx, objz);
            float dx = objx - px;
            float dy = instances[i].position.y - py;
            float dz = objz - pz;
            float distSqrd = dx*dx + dy*dy + dz*dz;
            if (distSqrd >= FAR_PLANE_SQUARED) continue;
            if (instCellIdx < ARRSIZE && CellNotVisible(instCellIdx)) continue;
            
            float dotResult = dot(dx, dy, dz, cam_forwardx, cam_forwardy, cam_forwardz);
            float radius = modelBounds[(instances[i].modelIndex * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_RADIUS] * 2.0f;
            if (dotResult < 0.0f && distSqrd > (radius * radius)) continue;
            
            glUniform1ui(0, i);
            if (currentNormIndex != (uint32_t)instances[i].normIndex) { currentNormIndex = (uint32_t)instances[i].normIndex; glUniform1ui(1, currentNormIndex); }
            if (currentTexIndex  != (uint32_t)instances[i].texIndex)  { currentTexIndex  =  (uint32_t)instances[i].texIndex; glUniform1ui(18, currentTexIndex); }
            if (currentGlowIndex != (uint32_t)instances[i].glowIndex) { currentGlowIndex = (uint32_t)instances[i].glowIndex; glUniform1ui(19, currentGlowIndex); }
            if (currentSpecIndex != (uint32_t)instances[i].specIndex) { currentSpecIndex = (uint32_t)instances[i].specIndex; glUniform1ui(20, currentSpecIndex); }
            int32_t modelType = instanceIsLODArray[i] && instances[i].lodIndex < loadedModelsMaxIndex ? instances[i].lodIndex : instances[i].modelIndex;
            if (currentModelType != modelType) {
                currentModelType = modelType;
                glBindVertexBuffer(0, voxen_GL_Comms.vbos[currentModelType], 0, VERTEX_ATTRIBUTES_COUNT * sizeof(float));
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, voxen_GL_Comms.tbos[currentModelType]);
            }
            
            uint32_t vertCount = modelTriangleCounts[currentModelType] * 3;
            glDrawElements(GL_TRIANGLES, vertCount, GL_UNSIGNED_INT, 0);
            voxen_Diagnostics.drawCallsRenderedThisFrame++;
            voxen_Diagnostics.verticesRenderedThisFrame += vertCount;
        }
        
        glEnable(GL_CULL_FACE); glEnable(GL_BLEND); // Transparents (with sort)
        uint16_t startOfNextType = loadedInstances - invalidModelIndexCount;
        visibleCount = 0;
        for (uint16_t i = startOfTransparentInstances; i < startOfNextType; ++i) {
            float objx = instances[i].position.x;
            float objz = instances[i].position.z;
            uint16_t instCellIdx = PosGetCellCoords(objx, objz);
            float dx = objx - px;
            float dy = instances[i].position.y - py;
            float dz = objz - pz;
            float distSqrd = dx*dx + dy*dy + dz*dz;
            if (distSqrd >= FAR_PLANE_SQUARED) continue;
            
            if (!(voxen_globalContext.currentLevel == 1 && (instances[i].index == 309 ||  instances[i].index == 532))) { // Hack for beaker and beaker holder on level 1 shelf getting culled from door portals.
                if (instCellIdx < ARRSIZE && CellNotVisible(instCellIdx)) continue;
            }
            
            float dotResult = dot(dx, dy, dz, cam_forwardx, cam_forwardy, cam_forwardz);
            float radius = modelBounds[(instances[i].modelIndex * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_RADIUS] * 2.0f;
            if (dotResult < 0.0f && distSqrd > (radius * radius)) continue;
            
            visibleInstances[visibleCount].index = i;
            visibleInstances[visibleCount].depth = distSqrd;
            visibleCount++;
        }

        if (visibleCount > 1) qsort(visibleInstances, visibleCount, sizeof(DepthSort), compareDepthSort); // Sort by depth (descending for back-to-front)
        for (uint16_t visibleIndex = 0; visibleIndex < visibleCount; ++visibleIndex) {
            uint16_t i = visibleInstances[visibleIndex].index;
            glUniform1ui(0, i);
            if (currentNormIndex != (uint32_t)instances[i].normIndex) { currentNormIndex = (uint32_t)instances[i].normIndex; glUniform1ui(1, currentNormIndex); }
            if (currentTexIndex  != (uint32_t)instances[i].texIndex)  { currentTexIndex  =  (uint32_t)instances[i].texIndex; glUniform1ui(18, currentTexIndex); }
            if (currentGlowIndex != (uint32_t)instances[i].glowIndex) { currentGlowIndex = (uint32_t)instances[i].glowIndex; glUniform1ui(19, currentGlowIndex); }
            if (currentSpecIndex != (uint32_t)instances[i].specIndex) { currentSpecIndex = (uint32_t)instances[i].specIndex; glUniform1ui(20, currentSpecIndex); }
            int32_t modelType = instanceIsLODArray[i] && instances[i].lodIndex < loadedModelsMaxIndex ? instances[i].lodIndex : instances[i].modelIndex;
            if (currentModelType != modelType) {
                currentModelType = modelType;
                glBindVertexBuffer(0, voxen_GL_Comms.vbos[currentModelType], 0, VERTEX_ATTRIBUTES_COUNT * sizeof(float));
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, voxen_GL_Comms.tbos[currentModelType]);
            }
            
            uint32_t vertCount = modelTriangleCounts[currentModelType] * 3;
            glDrawElements(GL_TRIANGLES, vertCount, GL_UNSIGNED_INT, 0);
            voxen_Diagnostics.drawCallsRenderedThisFrame++;
            voxen_Diagnostics.verticesRenderedThisFrame += vertCount;
        }
        
        DrawDebugLines(viewProj);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // 5. SSR (Screen Space Reflections)
        if (voxen_Settings.Reflections > 0) {
            glUseProgram(voxen_GL_Comms.ssrShaderProgram);
            glUniformMatrix4fv(4, 1, GL_FALSE, viewProj);
            glUniform3f(3, px, py, pz);
            GLuint groupX_ssr = ((voxen_Settings.ScreenWidth  / SSR_RES) + 31) / 32;
            GLuint groupY_ssr = ((voxen_Settings.ScreenHeight / SSR_RES) + 31) / 32;
            glDispatchCompute(groupX_ssr, groupY_ssr, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        }
    } else { // END !PAUSED BLOCK -------------------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // Allow text to still render while paused
    }
    
    RenderCompositePass(px, py, pz, viewProj, invViewRot); // 6. Render final meshes' results with full screen quad
    double time_now = RenderUI();                          // 7. UI
    
    // 8. Diagnostics
    voxen_globalContext.last_time = time_now;
    if ((time_now - voxen_Diagnostics.lastFrameSecCountTime) >= 1.00) {
        voxen_Diagnostics.lastFrameSecCountTime = time_now;
        voxen_Diagnostics.framesPerLastSecond = voxen_Diagnostics.globalFrameNum - voxen_Diagnostics.lastFrameSecCount;
        if (voxen_Diagnostics.framesPerLastSecond < voxen_Diagnostics.worstFPS && voxen_Diagnostics.globalFrameNum > 2000) voxen_Diagnostics.worstFPS = voxen_Diagnostics.framesPerLastSecond; // After startup, keep track of worst framerate seen.
        voxen_Diagnostics.lastFrameSecCount = voxen_Diagnostics.globalFrameNum;
    }
    
    if (keyStates[GLFW_KEY_F12].pressed && time_now > voxen_globalContext.screenshotTimeout) {
        Screenshot();
        voxen_globalContext.screenshotTimeout = time_now + 1.0; // Prevent saving more than 1 per second for sanity purposes.
    }
    
    if (keyStates[GLFW_KEY_ESCAPE].pressed) voxen_globalContext.gamePaused = !voxen_globalContext.gamePaused;
    voxen_Diagnostics.cpuTime = get_time() - voxen_globalContext.current_time;
    glfwSwapBuffers(voxen_globalContext.window); // Present frame
    CHECK_GL_ERROR();
}

void MainLoop(void) {
    voxen_globalContext.current_time = get_time();
    double frame_time = voxen_globalContext.current_time - voxen_globalContext.last_topframe_time;
    voxen_globalContext.last_topframe_time = voxen_globalContext.current_time;
    if (!voxen_globalContext.gamePaused) voxen_globalContext.pauseRelativeTime += frame_time;
    
    InputClearRisingAndFallingEdges();
    glfwPollEvents();
    if (glfwWindowShouldClose(voxen_globalContext.window)) EnqueueEvent(EV_QUIT,EV_INT_FIELD_UNUSED,EV_INT_FIELD_UNUSED,EV_FLOAT_FIELD_UNUSED,EV_FLOAT_FIELD_UNUSED);
    voxen_globalContext.timeSinceLastPhysicsTick = voxen_globalContext.pauseRelativeTime - voxen_globalContext.last_physics_time;
    if (!log_playback && !voxen_globalContext.gamePaused && !voxen_globalContext.menuActive) {
        voxen_globalContext.last_physics_time = voxen_globalContext.pauseRelativeTime;
        EnqueueEvent(EV_PHYSICS_TICK,EV_INT_FIELD_UNUSED,EV_INT_FIELD_UNUSED,EV_FLOAT_FIELD_UNUSED,EV_FLOAT_FIELD_UNUSED);
    }

    if (log_playback) { // Enqueue all logged events for the current frame.
        int32_t read_status = ReadActiveLog();
        if (read_status == 2) { // EOF reached, no more events
            DualLog("Log playback completed.  Control returned.\n");
        } else if (read_status == -1) { // Read error
            DualLogError("Error reading log file, exiting playback\n");
            EnqueueEvent(EV_QUIT,EV_INT_FIELD_UNUSED,EV_INT_FIELD_UNUSED,EV_FLOAT_FIELD_UNUSED,EV_FLOAT_FIELD_UNUSED);
        }
    }

    if (EventQueueProcess()) OS_Exit(1); // Do everything
    if (queuedLevelToLoad != 255u) { LoadLevel(queuedLevelToLoad); return; }
         
    if (!voxen_globalContext.gamePaused && !voxen_globalContext.menuActive) { // !PAUSED BLOCK -------------------------------------------------
        UpdateGameplay(); // 0. Gameplay Update Loops
        bool voxelsNeedUpdated = UpdatedPlayerCell();
        voxelsNeedUpdated = UpdateLights(&voxelsNeedUpdated);
        if (voxelsNeedUpdated) CullCore(); // 1. Culling
        bool uploadInstances = false;
        for (uint32_t i = START_INDEX_LEVEL_INSTANCES; i < loadedInstances; i++) { if (dirtyInstances[i]) { uploadInstances = true; UpdateInstanceMatrix(i); } }
        if (uploadInstances) glNamedBufferData(voxen_GL_Comms.matricesBufferID, loadedInstances * 16 * sizeof(float), modelMatrices, GL_DYNAMIC_DRAW);
        glBindVertexArray(voxen_GL_Comms.vao_chunk);
    }
    
    Render();
    voxen_Diagnostics.globalFrameNum++;
    #ifdef DEBUG_RAM_OUTPUT
        if (voxen_Diagnostics.globalFrameNum == 4) { DebugRAM("after 4 frames of running"); }
        else if (voxen_Diagnostics.globalFrameNum == 100) { DebugRAM("after 100 frames of running"); }
        else if (voxen_Diagnostics.globalFrameNum == 200) DebugRAM("after 200 frames of running");
        else if (voxen_Diagnostics.globalFrameNum == 500) DebugRAM("after 500 frames of running");
        else if (voxen_Diagnostics.globalFrameNum == 1000) DebugRAM("after 1000 frames of running");
    #endif
}

int32_t main(int32_t argc, char* argv[]) {
    double game_start_time = get_time();
    random_range_rng = (uint32_t)game_start_time; // Seed global rand uniquely with time since system boot.
    OpenConsoleLogFile();
    DebugRAM("program start");
    if (argc >= 2 && (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0)) { DualLog("-----------------------------------------------------------\nVoxen, the Voxel Lit Open Source Game Engine\nby W. Josiah Jack\nMIT-0 licensed\n"); return 0; }
    if ((argc >= 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0))
        || (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) ) {
        DualLog("Voxen the Voxel Lit Open Source Game Engine\n");
        DualLog("-------------------------------------------------------------\n");
        DualLog("   This is a game engine designed for optimized focused usage\n");
        DualLog("   of OpenGL, making heavy use of GPU Driven rendering\n");
        DualLog("   techniques, a unified event system for debugging and log\n");
        DualLog("   playback, full mod support loading all data from external\n");
        DualLog("   files and using definition files for what to do with the\n");
        DualLog("   data.\n\n");
        DualLog("   This project aims to have minimal overhead, profiling,\n");
        DualLog("   traceability, robustness, and low level control.\n\n");
        DualLog("\n");
        DualLog("Valid arguments:\n");
        DualLog(" < none >\n    Runs the engine as normal, loading data from \n    neighbor directories (./Textures, ./Models, etc.)\n\n");
        DualLog("-v, --version\n    Prints version information\n\n");
        DualLog("play <file>\n    Plays back recorded log from current directory\n\n");
        DualLog("record <file>\n    Records all engine events to designated log\n    as a .dem file\n\n");
        DualLog("dump <file.dem>\n    Dumps the specified log into ./log_dump.txt\n    as human readable text.  You must provide full\n    file name with extension\n\n");
        DualLog("-h, --help\n    Provides this help text.  Neat!\n");
        DualLog("-----------------------------------------------------------\n");
        return 0;
    }

    if (argc == 3 && strcmp(argv[1], "dump") == 0) { DualLog("Converting log to plaintext: %s ...", argv[2]); JournalDump(argv[2]); DualLog("DONE!\n"); return 0; }

    InitializeEnvironment(argc,argv[1],argv[2]);
    DebugRAM("prior to game loop");
    DualLog("Game Initialized in %f secs\n",get_time() - game_start_time);
    while(1) MainLoop();
    return 0;
}
