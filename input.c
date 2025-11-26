// input.c - Input System for keyboard, mouse, and gamepads
KeyState keyStates[MAX_KEYS] = {{0}};
KeyState mouseButtons[MAX_MOUSE_BUTTONS] = {{0}};
double scrollDelta;
double last_mouse_x = 0.0, last_mouse_y = 0.0;
float mouse_sensitivity = 0.1f;
bool window_has_focus = false;

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
        
        if (globalFrameNum > 1) EnqueueEvent(EV_MOUSEMOVE, dx, dy, EV_FLOAT_FIELD_UNUSED, EV_FLOAT_FIELD_UNUSED);
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
    if (consoleActive) { ConsoleEmulator(keycode); return 0; }
    return 0;
}

int32_t Input_KeyUp(int32_t keycode) {
    if (keycode >= 0 && keycode < MAX_KEYS) keyStates[keycode].pressed = keyStates[keycode].down = false;
    return 0;
}

void InputClearRisingAndFallingEdges() { // Clear keypress rising and falling edge triggers
    for (int32_t i=0;i<MAX_KEYS;++i)          keyStates[i].pressed = keyStates[i].released = false;       // Can't memset as we want to preserve down state
    for (int32_t i=0;i<MAX_MOUSE_BUTTONS;i++) mouseButtons[i].pressed = mouseButtons[i].released = false; // Can't memset as we want to preserve down state
    scrollDelta = 0;
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
    if (keyStates[GLFW_KEY_ESCAPE].pressed) gamePaused = !gamePaused;
    if (keyStates[GLFW_KEY_B].pressed) CycleToNextMonitor(window);
    if (keyStates[GLFW_KEY_GRAVE_ACCENT].pressed) ToggleConsole();
    
    // Debug inputs
    if (keyStates[GLFW_KEY_LEFT_CONTROL].down && keyStates[GLFW_KEY_R].pressed) {
        debugView++;
        if (debugView > 4) debugView = 0;
        glProgramUniform1i(chunkShaderProgram, debugViewLoc_chunk, debugView);
        glProgramUniform1i(imageBlitShaderProgram, debugViewLoc_quadblit, debugView);
    }

    if (keyStates[GLFW_KEY_LEFT_CONTROL].down && keyStates[GLFW_KEY_Y].pressed) {
        debugValue++;
        if (debugValue > 6) debugValue = 0;
        glProgramUniform1i(imageBlitShaderProgram, debugValueLoc_quadblit, debugValue);
        glProgramUniform1i(chunkShaderProgram, debugValueLoc_chunk, debugValue);
    }
    
    if (keyStates[GLFW_KEY_LEFT_CONTROL].down && keyStates[GLFW_KEY_E].pressed) play_wav("./Audio/weapons/wpistol.wav",0.5f);
    // End Debug Inputs
    
    if (gamePaused || consoleActive) return;

    float moveForce = 1800.0f;
    float sprintMul = keyStates[GLFW_KEY_LEFT_SHIFT].down ? 1.75f : 1.0f;
    Vector3 input = {0};

    if (keyStates[GLFW_KEY_F].down) input = add_vector3(input, (Vector3){cam_forwardx, 0, cam_forwardz});
    if (keyStates[GLFW_KEY_S].down) input = sub_vector3(input, (Vector3){cam_forwardx, 0, cam_forwardz});
    if (keyStates[GLFW_KEY_D].down) input = add_vector3(input, (Vector3){cam_rightx,   0, cam_rightz});
    if (keyStates[GLFW_KEY_A].down) input = sub_vector3(input, (Vector3){cam_rightx,   0, cam_rightz});
    
    if (noclip) {
        if (keyStates[GLFW_KEY_C].down) input = add_vector3(input, (Vector3){0,  -1.0f, 0});
        else if (keyStates[GLFW_KEY_V].down) input = add_vector3(input, (Vector3){0,  1.0f, 0});
    }

    if (magnitude_vector3(input) > 0.1f) {
        input = normalize_vector3(input);
        Vector3 force = scale_vector3(input, moveForce * sprintMul);
        AddForce(PLAYER1, force, FORCEMODE_ACCUMULATE);
    }

    if (keyStates[GLFW_KEY_SPACE].pressed && (instances[PLAYER1].entflags & ENTFLAG_GROUNDED)) { // Jump
        AddForce(PLAYER1, (Vector3){0, 6.8f, 0}, FORCEMODE_IMPULSE);
        flag_set(&instances[PLAYER1].entflags, ENTFLAG_GROUNDED, false);
    }
    
    if (keyStates[GLFW_KEY_TAB].pressed) {
        ignore_next_mouse_delta = true;
        inventoryMode = !inventoryMode;
    }
}
