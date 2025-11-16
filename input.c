#define MAX_KEYS 512
#define MAX_MOUSE_BUTTONS 8

typedef struct {
    bool down;
    bool pressed;
    bool released;
} KeyState;

static KeyState keyStates[MAX_KEYS];
static KeyState mouseButtons[MAX_MOUSE_BUTTONS];
static double scrollDelta;
static bool cursorLocked = true;
double last_mouse_x = 0.0, last_mouse_y = 0.0;
float mouse_sensitivity = 0.1f;
float move_speed = 0.06;

void Input_ClearFrame(void) {
    for (int i = 0; i < MAX_KEYS; i++) {
        keyStates[i].pressed = false;
        keyStates[i].released = false;
    }
    
    for (int i = 0; i < MAX_MOUSE_BUTTONS; i++) {
        mouseButtons[i].pressed = false;
        mouseButtons[i].released = false;
    }
    
    scrollDelta = 0;
}

bool GetInput_Key(int key)              { return keyStates[key].down; }
bool GetInput_KeyDown(int key)          { return keyStates[key].pressed; }
bool GetInput_KeyUp(int key)            { return keyStates[key].released; }
bool GetInput_MouseButton(int b)        { return mouseButtons[b].down; }
bool GetInput_MouseButtonDown(int b)    { return mouseButtons[b].pressed; }
bool GetInput_MouseButtonUp(int b)      { return mouseButtons[b].released; }
double GetInput_ScrollDelta(void)       { return scrollDelta; }
void Input_LockCursor(bool locked) { cursorLocked = locked; }

// GLFW Callbacks
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F10 && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        if (log_playback) {
            log_playback = false;
            DualLog("Exited log playback manually.  Control returned\n");
        } else {
            EnqueueEvent_Simple(EV_QUIT);
        }
        return;
    }
    if (!log_playback) {
        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            EnqueueEvent_Int(EV_KEYDOWN, key);
        } else if (action == GLFW_RELEASE) {
            EnqueueEvent_Int(EV_KEYUP, key);
        }
    }
}

static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!log_playback && window_has_focus) {
        int32_t dx = (int32_t)(xpos - last_mouse_x);
        int32_t dy = (int32_t)(ypos - last_mouse_y);
        last_mouse_x = xpos;
        last_mouse_y = ypos;
        if (ignore_next_mouse_delta) { ignore_next_mouse_delta = false; return; }
        
        if (globalFrameNum > 1) EnqueueEvent_IntInt(EV_MOUSEMOVE, dx, dy);
    }
}

static void window_focus_callback(GLFWwindow* window, int focused) {
    window_has_focus = focused != 0;
    if (window_has_focus) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button < 0 || button >= MAX_MOUSE_BUTTONS) return;
    if (action == GLFW_PRESS) {
        mouseButtons[button].down = true;
        mouseButtons[button].pressed = true;
        EnqueueEvent_Int(EV_KEYDOWN, button + 1000); // offset mouse events if needed
    } else if (action == GLFW_RELEASE) {
        mouseButtons[button].down = false;
        mouseButtons[button].released = true;
        EnqueueEvent_Int(EV_KEYUP, button + 1000);
    }
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    scrollDelta += yoffset;
}
#pragma GCC diagnostic pop

void Input_Init(GLFWwindow* window) {
    memset(keyStates, 0, sizeof(keyStates));
    memset(mouseButtons, 0, sizeof(mouseButtons));
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetWindowFocusCallback(window, window_focus_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
}

bool window_has_focus = false;
bool keys[NUM_KEYS] = {0};
uint16_t mouse_x = 0, mouse_y = 0; // Mouse position

int32_t Input_KeyDown(int32_t keycode) {
    if (keycode >= 0 && keycode < NUM_KEYS) keys[keycode] = true;    
    if (keys[GLFW_KEY_ESCAPE]) gamePaused = !gamePaused;
    if (keys[GLFW_KEY_GRAVE_ACCENT]) ToggleConsole();
    if (consoleActive) { ConsoleEmulator(keycode); return 0; }
    
    if (keys[GLFW_KEY_TAB]) inventoryMode = !inventoryMode; // After consoleActive check to allow tab completion
    if (keys[GLFW_KEY_R]) {
        debugView++;
        if (debugView > 4) debugView = 0;
        glProgramUniform1i(chunkShaderProgram, debugViewLoc_chunk, debugView);
        glProgramUniform1i(imageBlitShaderProgram, debugViewLoc_quadblit, debugView);
    }

    if (keys[GLFW_KEY_Y]) {
        debugValue++;
        if (debugValue > 6) debugValue = 0;
        glProgramUniform1i(imageBlitShaderProgram, debugValueLoc_quadblit, debugValue);
        glProgramUniform1i(chunkShaderProgram, debugValueLoc_chunk, debugValue);
    }

    if (keys[GLFW_KEY_E]) {
        play_wav("./Audio/weapons/wpistol.wav",0.5f);
    }

    if (keys[GLFW_KEY_1]) {
        fogColorRUsed += 0.01f;
            DualLog("Set fog to %f %f %f\n",fogColorRUsed,fogColorGUsed,fogColorBUsed);

    } else if (keys[GLFW_KEY_2]) {
        fogColorRUsed -= 0.01f;
            DualLog("Set fog to %f %f %f\n",fogColorRUsed,fogColorGUsed,fogColorBUsed);

    }
    
    if (keys[GLFW_KEY_3]) {
        fogColorGUsed += 0.01f;
            DualLog("Set fog to %f %f %f\n",fogColorRUsed,fogColorGUsed,fogColorBUsed);

    } else if (keys[GLFW_KEY_4]) {
        fogColorGUsed -= 0.01f;
            DualLog("Set fog to %f %f %f\n",fogColorRUsed,fogColorGUsed,fogColorBUsed);

    }
    
    if (keys[GLFW_KEY_5]) {
        fogColorBUsed += 0.01f;
            DualLog("Set fog to %f %f %f\n",fogColorRUsed,fogColorGUsed,fogColorBUsed);

    } else if (keys[GLFW_KEY_6]) {
        fogColorBUsed -= 0.01f;
            DualLog("Set fog to %f %f %f\n",fogColorRUsed,fogColorGUsed,fogColorBUsed);

    }

    return 0;
}

int32_t Input_KeyUp(int32_t keycode) {
    if (keycode >= 0 && keycode < NUM_KEYS) keys[keycode] = false;
    return 0;
}

void UpdatePlayerFacingAngles() {
    float rotation[16]; // Extract forward and right vectors from quaternion
    quat_to_matrix(&cam_rotation, rotation);
    cam_forwardx = rotation[8];  // Forward X
    cam_forwardy = rotation[9];  // Forward Y
    cam_forwardz = rotation[10]; // Forward Z
    cam_rightx = rotation[0];  // Right X
    cam_righty = rotation[1];  // Right Y
    cam_rightz = rotation[2];  // Right Z
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

void Input_MouselookApply() {
    if (currentLevel == LEVEL_CYBERSPACE) quat_from_yaw_pitch_roll(&cam_rotation,cam_yaw,cam_pitch,cam_roll);
    else               quat_from_yaw_pitch_roll(&cam_rotation,cam_yaw,cam_pitch,    0.0f);
}

int32_t Input_MouseMove(int32_t xrel, int32_t yrel) {
    if (CursorVisible()) {
        int32_t newX = cursorPosition_x + xrel;
        if (newX > screen_width) newX = screen_width;
        if (newX < 0) newX = 0;
        cursorPosition_x = newX;
        int32_t newY = cursorPosition_y + yrel;
        if (newY > screen_height) newY = screen_height;
        if (newY < 0) newY = 0;
        cursorPosition_y = newY;
    }
    
    if (gamePaused || inventoryMode) return 0;
    
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
    if (gamePaused || consoleActive) return;

    float ms = keys[GLFW_KEY_LEFT_SHIFT] ? move_speed * 1.75f : move_speed;
    Vector3 delta = {0};
    if (keys[GLFW_KEY_F]) delta = add_vector3(delta, scale_vector3((Vector3){cam_forwardx, cam_forwardy, cam_forwardz},  ms));
    if (keys[GLFW_KEY_S]) delta = add_vector3(delta, scale_vector3((Vector3){cam_forwardx, cam_forwardy, cam_forwardz}, -ms));
    if (keys[GLFW_KEY_D]) delta = add_vector3(delta, scale_vector3((Vector3){cam_rightx,   cam_righty,   cam_rightz},    ms));
    if (keys[GLFW_KEY_A]) delta = add_vector3(delta, scale_vector3((Vector3){cam_rightx,   cam_righty,   cam_rightz},   -ms));
    if (noclip) {
        if (keys[GLFW_KEY_V])    delta.y += ms;
        if (keys[GLFW_KEY_C])    delta.y -= ms;
    } else if (currentLevel != LEVEL_CYBERSPACE) delta.y = 0;

    instances[PLAYER1].position = add_vector3(instances[PLAYER1].position, delta);
    if (keys[GLFW_KEY_Q]) { cam_roll += move_speed * 5.0f; Input_MouselookApply(); }
    if (keys[GLFW_KEY_T]) { cam_roll -= move_speed * 5.0f; Input_MouselookApply(); }
}
