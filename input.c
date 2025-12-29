// input.c - Input System for keyboard, mouse, and gamepads
KeyState keyStates[MAX_KEYS] = {{0}};
KeyState mouseButtons[MAX_MOUSE_BUTTONS] = {{0}};
double scrollDelta;
double last_mouse_x = 0.0, last_mouse_y = 0.0;
float mouse_sensitivity = 0.1f;
bool window_has_focus = false;
uint16_t editModeSelection = 653; // Test instance
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
