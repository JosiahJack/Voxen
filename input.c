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
