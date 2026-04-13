#include "internal.h"
#include "mappings.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#define DBL_MAX 1.7976931348623158e+308
#define _GLFW_STICK 3
#define _GLFW_JOYSTICK_AXIS 1
#define _GLFW_JOYSTICK_BUTTON 2
#define _GLFW_JOYSTICK_HATBIT 3
#define GLFW_MOD_MASK (GLFW_MOD_SHIFT|GLFW_MOD_CONTROL|GLFW_MOD_ALT|GLFW_MOD_SUPER|GLFW_MOD_CAPS_LOCK|GLFW_MOD_NUM_LOCK)
static GLFWbool initJoysticks(void) {
    if (!_glfw.joysticksInitialized && !_glfw.platform.initJoysticks()) return GLFW_FALSE;
    return _glfw.joysticksInitialized = GLFW_TRUE;
}

static _GLFWmapping* findMapping(const char* guid) {
    for (int i = 0; i < _glfw.mappingCount; i++) { if (strcmp(_glfw.mappings[i].guid,guid) == 0) return _glfw.mappings + i; }
    return NULL;
}

static GLFWbool isValidElementForJoystick(const _GLFWmapelement* e,const _GLFWjoystick* js) {
    if (e->type == _GLFW_JOYSTICK_HATBIT && (e->index >> 4) >= js->hatCount) return GLFW_FALSE;
    if (e->type == _GLFW_JOYSTICK_BUTTON && e->index >= js->buttonCount) return GLFW_FALSE;
    if (e->type == _GLFW_JOYSTICK_AXIS && e->index >= js->axisCount) return GLFW_FALSE;
    return GLFW_TRUE;
}

static _GLFWmapping* findValidMapping(const _GLFWjoystick* js) {
    _GLFWmapping* mapping = findMapping(js->guid);
    if (mapping) {
        for (int i = 0; i <= GLFW_GAMEPAD_BUTTON_LAST; i++) { if (!isValidElementForJoystick(mapping->buttons + i,js)) return NULL; }
        for (int i = 0; i <= GLFW_GAMEPAD_AXIS_LAST; i++) { if (!isValidElementForJoystick(mapping->axes + i,js)) return NULL; }
    }
    return mapping;
}

static GLFWbool parseMapping(_GLFWmapping* mapping,const char* string) {
    const char* c = string;
    size_t i,length;
    struct { const char* name; _GLFWmapelement* element; } fields[] = {
        {"platform",NULL},
        {"a",mapping->buttons + GLFW_GAMEPAD_BUTTON_A},{"b",mapping->buttons + GLFW_GAMEPAD_BUTTON_B},
        {"x",mapping->buttons + GLFW_GAMEPAD_BUTTON_X},{"y",mapping->buttons + GLFW_GAMEPAD_BUTTON_Y},
        {"back",mapping->buttons + GLFW_GAMEPAD_BUTTON_BACK},{"start",mapping->buttons + GLFW_GAMEPAD_BUTTON_START},
        {"guide",mapping->buttons + GLFW_GAMEPAD_BUTTON_GUIDE},{"leftshoulder",mapping->buttons + GLFW_GAMEPAD_BUTTON_LEFT_BUMPER},
        {"rightshoulder",mapping->buttons + GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER},{"leftstick",mapping->buttons + GLFW_GAMEPAD_BUTTON_LEFT_THUMB},
        {"rightstick",mapping->buttons + GLFW_GAMEPAD_BUTTON_RIGHT_THUMB},{"dpup",mapping->buttons + GLFW_GAMEPAD_BUTTON_DPAD_UP},
        {"dpright",mapping->buttons + GLFW_GAMEPAD_BUTTON_DPAD_RIGHT},{"dpdown",mapping->buttons + GLFW_GAMEPAD_BUTTON_DPAD_DOWN},
        {"dpleft",mapping->buttons + GLFW_GAMEPAD_BUTTON_DPAD_LEFT},{"lefttrigger",mapping->axes + GLFW_GAMEPAD_AXIS_LEFT_TRIGGER},
        {"righttrigger",mapping->axes + GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER},{"leftx",mapping->axes + GLFW_GAMEPAD_AXIS_LEFT_X},
        {"lefty",mapping->axes + GLFW_GAMEPAD_AXIS_LEFT_Y},{"rightx",mapping->axes + GLFW_GAMEPAD_AXIS_RIGHT_X},
        {"righty",mapping->axes + GLFW_GAMEPAD_AXIS_RIGHT_Y}
    };
    if ((length = strcspn(c,",")) != 32 || c[length] != ',') return _glfwInputError(GLFW_INVALID_VALUE,NULL),GLFW_FALSE;
    memcpy(mapping->guid,c,length); c += length + 1;
    if ((length = strcspn(c,",")) >= sizeof(mapping->name) || c[length] != ',') return _glfwInputError(GLFW_INVALID_VALUE,NULL),GLFW_FALSE;
    memcpy(mapping->name,c,length); c += length + 1;
    while (*c) {
        if (*c == '+' || *c == '-') return GLFW_FALSE;
        for (i = 0; i < sizeof(fields)/sizeof(fields[0]); i++) {
            length = strlen(fields[i].name);
            if (strncmp(c,fields[i].name,length) != 0 || c[length] != ':') continue;
            c += length + 1;
            if (fields[i].element) {
                _GLFWmapelement* e = fields[i].element;
                int8_t minimum = -1,maximum = 1;
                if (*c == '+') { minimum = 0; c++; }
                else if (*c == '-') { maximum = 0; c++; }
                if (*c == 'a') e->type = _GLFW_JOYSTICK_AXIS;
                else if (*c == 'b') e->type = _GLFW_JOYSTICK_BUTTON;
                else if (*c == 'h') e->type = _GLFW_JOYSTICK_HATBIT;
                else break;
                if (e->type == _GLFW_JOYSTICK_HATBIT) {
                    const unsigned long hat = strtoul(c+1,(char**)&c,10),bit = strtoul(c+1,(char**)&c,10);
                    e->index = (uint8_t)((hat << 4) | bit);
                } else e->index = (uint8_t)strtoul(c+1,(char**)&c,10);
                if (e->type == _GLFW_JOYSTICK_AXIS) {
                    e->axisScale = 2/(maximum - minimum);
                    e->axisOffset = -(maximum + minimum);
                    if (*c == '~') { e->axisScale = -e->axisScale; e->axisOffset = -e->axisOffset; }
                }
            } else {
                const char* name = _glfw.platform.getMappingName();
                if (strncmp(c,name,strlen(name)) != 0) return GLFW_FALSE;
            }
            break;
        }
        c += strcspn(c,","); c += strspn(c,",");
    }
    for (i = 0; i < 32; i++) { if (mapping->guid[i] >= 'A' && mapping->guid[i] <= 'F') mapping->guid[i] += 'a'-'A'; }
    _glfw.platform.updateGamepadGUID(mapping->guid);
    return GLFW_TRUE;
}

void _glfwInputKey(_GLFWwindow* window,int key,int scancode,int action,int mods) {
    if (key >= 0 && key <= GLFW_KEY_LAST) {
        GLFWbool repeated = GLFW_FALSE;
        if (action == GLFW_RELEASE && window->keys[key] == GLFW_RELEASE) return;
        if (action == GLFW_PRESS && window->keys[key] == GLFW_PRESS) repeated = GLFW_TRUE;
        window->keys[key] = (action == GLFW_RELEASE && window->stickyKeys) ? _GLFW_STICK : (char)action;
        if (repeated) action = GLFW_REPEAT;
    }
    if (!window->lockKeyMods) mods &= ~(GLFW_MOD_CAPS_LOCK|GLFW_MOD_NUM_LOCK);
    if (window->callbacks.key) window->callbacks.key((GLFWwindow*)window,key,scancode,action,mods);
}

void _glfwInputChar(_GLFWwindow* window,uint32_t codepoint,int mods,GLFWbool plain) {
    if (codepoint < 32 || (codepoint > 126 && codepoint < 160)) return;
    if (!window->lockKeyMods) mods &= ~(GLFW_MOD_CAPS_LOCK|GLFW_MOD_NUM_LOCK);
    if (window->callbacks.charmods) window->callbacks.charmods((GLFWwindow*)window,codepoint,mods);
    if (plain && window->callbacks.character) window->callbacks.character((GLFWwindow*)window,codepoint);
}

void _glfwInputScroll(_GLFWwindow* window,double xoffset,double yoffset) {
    if (window->callbacks.scroll) window->callbacks.scroll((GLFWwindow*)window,xoffset,yoffset);
}

void _glfwInputMouseClick(_GLFWwindow* window,int button,int action,int mods) {
    if (button < 0 || (!window->disableMouseButtonLimit && button > GLFW_MOUSE_BUTTON_LAST)) return;
    if (!window->lockKeyMods) mods &= ~(GLFW_MOD_CAPS_LOCK|GLFW_MOD_NUM_LOCK);
    if (button <= GLFW_MOUSE_BUTTON_LAST)
        window->mouseButtons[button] = (action == GLFW_RELEASE && window->stickyMouseButtons) ? _GLFW_STICK : (char)action;
    if (window->callbacks.mouseButton) window->callbacks.mouseButton((GLFWwindow*)window,button,action,mods);
}

void _glfwInputCursorPos(_GLFWwindow* window,double xpos,double ypos) {
    if (window->virtualCursorPosX == xpos && window->virtualCursorPosY == ypos) return;
    window->virtualCursorPosX = xpos; window->virtualCursorPosY = ypos;
    if (window->callbacks.cursorPos) window->callbacks.cursorPos((GLFWwindow*)window,xpos,ypos);
}

void _glfwInputCursorEnter(_GLFWwindow* window,GLFWbool entered) {
    if (window->callbacks.cursorEnter) window->callbacks.cursorEnter((GLFWwindow*)window,entered);
}

void _glfwInputDrop(_GLFWwindow* window,int count,const char** paths) {
    if (window->callbacks.drop) window->callbacks.drop((GLFWwindow*)window,count,paths);
}

void _glfwInputJoystick(_GLFWjoystick* js,int event) {
    if (event == GLFW_CONNECTED) js->connected = GLFW_TRUE;
    else if (event == GLFW_DISCONNECTED) js->connected = GLFW_FALSE;
    if (_glfw.callbacks.joystick) _glfw.callbacks.joystick((int)(js - _glfw.joysticks),event);
}

void _glfwInputJoystickAxis(_GLFWjoystick* js,int axis,float value) { js->axes[axis] = value; }
void _glfwInputJoystickButton(_GLFWjoystick* js,int button,char value) { js->buttons[button] = value; }

void _glfwInputJoystickHat(_GLFWjoystick* js,int hat,char value) {
    int base = js->buttonCount + hat * 4;
    js->buttons[base+0] = (value & 0x01) ? GLFW_PRESS : GLFW_RELEASE;
    js->buttons[base+1] = (value & 0x02) ? GLFW_PRESS : GLFW_RELEASE;
    js->buttons[base+2] = (value & 0x04) ? GLFW_PRESS : GLFW_RELEASE;
    js->buttons[base+3] = (value & 0x08) ? GLFW_PRESS : GLFW_RELEASE;
    js->hats[hat] = value;
}

void _glfwInitGamepadMappings(void) {
    const size_t count = sizeof(_glfwDefaultMappings)/sizeof(char*);
    _glfw.mappings = _glfw_calloc(count,sizeof(_GLFWmapping));
    for (size_t i = 0; i < count; i++) { if (parseMapping(&_glfw.mappings[_glfw.mappingCount],_glfwDefaultMappings[i])) _glfw.mappingCount++; }
}

_GLFWjoystick* _glfwAllocJoystick(const char* name,const char* guid,int axisCount,int buttonCount,int hatCount) {
    int jid; _GLFWjoystick* js;
    for (jid = 0; jid <= GLFW_JOYSTICK_LAST; jid++) { if (!_glfw.joysticks[jid].allocated) break; }
    if (jid > GLFW_JOYSTICK_LAST) return NULL;
    js = _glfw.joysticks + jid;
    js->allocated = GLFW_TRUE; js->axisCount = axisCount; js->buttonCount = buttonCount; js->hatCount = hatCount;
    js->axes = _glfw_calloc(axisCount,sizeof(float));
    js->buttons = _glfw_calloc(buttonCount + (size_t)hatCount * 4,1);
    js->hats = _glfw_calloc(hatCount,1);
    strncpy(js->name,name,sizeof(js->name)-1); strncpy(js->guid,guid,sizeof(js->guid)-1);
    js->mapping = findValidMapping(js);
    return js;
}

void _glfwFreeJoystick(_GLFWjoystick* js) { _glfw_free(js->axes); _glfw_free(js->buttons); _glfw_free(js->hats); memset(js,0,sizeof(_GLFWjoystick)); }

void _glfwCenterCursorInContentArea(_GLFWwindow* window) {
    int width,height;
    _glfw.platform.getWindowSize(window,&width,&height);
    _glfw.platform.setCursorPos(window,width/2.0,height/2.0);
}

GLFWAPI int glfwGetInputMode(GLFWwindow* handle,int mode) {
    _GLFW_REQUIRE_INIT_OR_RETURN(0);
    _GLFWwindow* window = (_GLFWwindow*)handle;
    switch (mode) {
        case GLFW_CURSOR: return window->cursorMode;
        case GLFW_STICKY_KEYS: return window->stickyKeys;
        case GLFW_STICKY_MOUSE_BUTTONS: return window->stickyMouseButtons;
        case GLFW_LOCK_KEY_MODS: return window->lockKeyMods;
        case GLFW_RAW_MOUSE_MOTION: return window->rawMouseMotion;
        case GLFW_UNLIMITED_MOUSE_BUTTONS: return window->disableMouseButtonLimit;
    }
    return _glfwInputError(GLFW_INVALID_ENUM,"Invalid input mode 0x%08X",mode),0;
}

GLFWAPI void glfwSetInputMode(GLFWwindow* handle,int mode,int value) {
    _GLFW_REQUIRE_INIT();
    _GLFWwindow* window = (_GLFWwindow*)handle;
    switch (mode) {
        case GLFW_CURSOR:
            if (value != GLFW_CURSOR_NORMAL && value != GLFW_CURSOR_HIDDEN && value != GLFW_CURSOR_DISABLED && value != GLFW_CURSOR_CAPTURED) { _glfwInputError(GLFW_INVALID_ENUM,"Invalid cursor mode 0x%08X",value); return; }
            if (window->cursorMode == value) return;
            window->cursorMode = value;
            _glfw.platform.getCursorPos(window,&window->virtualCursorPosX,&window->virtualCursorPosY);
            _glfw.platform.setCursorMode(window,value); return;
        case GLFW_STICKY_KEYS:
            value = value ? GLFW_TRUE : GLFW_FALSE;
            if (window->stickyKeys == value) return;
            if (!value) { for (int i = 0; i <= GLFW_KEY_LAST; i++) { if (window->keys[i] == _GLFW_STICK) window->keys[i] = GLFW_RELEASE; } }
            window->stickyKeys = value; return;
        case GLFW_STICKY_MOUSE_BUTTONS:
            value = value ? GLFW_TRUE : GLFW_FALSE;
            if (window->stickyMouseButtons == value) return;
            if (!value) { for (int i = 0; i <= GLFW_MOUSE_BUTTON_LAST; i++) { if (window->mouseButtons[i] == _GLFW_STICK) window->mouseButtons[i] = GLFW_RELEASE; } }
            window->stickyMouseButtons = value; return;
        case GLFW_LOCK_KEY_MODS: window->lockKeyMods = value ? GLFW_TRUE : GLFW_FALSE; return;
        case GLFW_RAW_MOUSE_MOTION:
            if (!_glfw.platform.rawMouseMotionSupported()) { _glfwInputError(GLFW_PLATFORM_ERROR,"Raw mouse motion is not supported on this system"); return; }
            value = value ? GLFW_TRUE : GLFW_FALSE;
            if (window->rawMouseMotion == value) return;
            window->rawMouseMotion = value; _glfw.platform.setRawMouseMotion(window,value); return;
        case GLFW_UNLIMITED_MOUSE_BUTTONS: window->disableMouseButtonLimit = value ? GLFW_TRUE : GLFW_FALSE; return;
    }
    _glfwInputError(GLFW_INVALID_ENUM,"Invalid input mode 0x%08X",mode);
}

GLFWAPI int glfwRawMouseMotionSupported(void) { _GLFW_REQUIRE_INIT_OR_RETURN(GLFW_FALSE); return _glfw.platform.rawMouseMotionSupported(); }

GLFWAPI int glfwGetKey(GLFWwindow* handle,int key) {
    _GLFW_REQUIRE_INIT_OR_RETURN(GLFW_RELEASE);
    _GLFWwindow* window = (_GLFWwindow*)handle;
    if (key < GLFW_KEY_SPACE || key > GLFW_KEY_LAST) return _glfwInputError(GLFW_INVALID_ENUM,"Invalid key %i",key),GLFW_RELEASE;
    if (window->keys[key] == _GLFW_STICK) { window->keys[key] = GLFW_RELEASE; return GLFW_PRESS; }
    return (int)window->keys[key];
}

GLFWAPI int glfwGetMouseButton(GLFWwindow* handle,int button) {
    _GLFW_REQUIRE_INIT_OR_RETURN(GLFW_RELEASE);
    _GLFWwindow* window = (_GLFWwindow*)handle;
    if (button < GLFW_MOUSE_BUTTON_1 || button > GLFW_MOUSE_BUTTON_LAST) return _glfwInputError(GLFW_INVALID_ENUM,"Invalid mouse button %i",button),GLFW_RELEASE;
    if (window->mouseButtons[button] == _GLFW_STICK) { window->mouseButtons[button] = GLFW_RELEASE; return GLFW_PRESS; }
    return (int)window->mouseButtons[button];
}

GLFWAPI GLFWkeyfun glfwSetKeyCallback(GLFWwindow* handle,GLFWkeyfun cbfun) { _GLFW_REQUIRE_INIT_OR_RETURN(NULL); _GLFWwindow* window = (_GLFWwindow*)handle; _GLFW_SWAP(GLFWkeyfun,window->callbacks.key,cbfun); return cbfun; }
GLFWAPI GLFWcharfun glfwSetCharCallback(GLFWwindow* handle,GLFWcharfun cbfun) { _GLFW_REQUIRE_INIT_OR_RETURN(NULL); _GLFWwindow* window = (_GLFWwindow*)handle; _GLFW_SWAP(GLFWcharfun,window->callbacks.character,cbfun); return cbfun; }
GLFWAPI GLFWcharmodsfun glfwSetCharModsCallback(GLFWwindow* handle,GLFWcharmodsfun cbfun) { _GLFW_REQUIRE_INIT_OR_RETURN(NULL); _GLFWwindow* window = (_GLFWwindow*)handle; _GLFW_SWAP(GLFWcharmodsfun,window->callbacks.charmods,cbfun); return cbfun; }
GLFWAPI GLFWmousebuttonfun glfwSetMouseButtonCallback(GLFWwindow* handle,GLFWmousebuttonfun cbfun) { _GLFW_REQUIRE_INIT_OR_RETURN(NULL); _GLFWwindow* window = (_GLFWwindow*)handle; _GLFW_SWAP(GLFWmousebuttonfun,window->callbacks.mouseButton,cbfun); return cbfun; }
GLFWAPI GLFWcursorposfun glfwSetCursorPosCallback(GLFWwindow* handle,GLFWcursorposfun cbfun) { _GLFW_REQUIRE_INIT_OR_RETURN(NULL); _GLFWwindow* window = (_GLFWwindow*)handle; _GLFW_SWAP(GLFWcursorposfun,window->callbacks.cursorPos,cbfun); return cbfun; }
GLFWAPI GLFWcursorenterfun glfwSetCursorEnterCallback(GLFWwindow* handle,GLFWcursorenterfun cbfun) { _GLFW_REQUIRE_INIT_OR_RETURN(NULL); _GLFWwindow* window = (_GLFWwindow*)handle; _GLFW_SWAP(GLFWcursorenterfun,window->callbacks.cursorEnter,cbfun); return cbfun; }
GLFWAPI GLFWscrollfun glfwSetScrollCallback(GLFWwindow* handle,GLFWscrollfun cbfun) { _GLFW_REQUIRE_INIT_OR_RETURN(NULL); _GLFWwindow* window = (_GLFWwindow*)handle; _GLFW_SWAP(GLFWscrollfun,window->callbacks.scroll,cbfun); return cbfun; }
GLFWAPI GLFWdropfun glfwSetDropCallback(GLFWwindow* handle,GLFWdropfun cbfun) { _GLFW_REQUIRE_INIT_OR_RETURN(NULL); _GLFWwindow* window = (_GLFWwindow*)handle; _GLFW_SWAP(GLFWdropfun,window->callbacks.drop,cbfun); return cbfun; }

GLFWAPI int glfwJoystickPresent(int jid) {
    _GLFW_REQUIRE_INIT_OR_RETURN(GLFW_FALSE);
    if (jid < 0 || jid > GLFW_JOYSTICK_LAST) return _glfwInputError(GLFW_INVALID_ENUM,"Invalid joystick ID %i",jid),GLFW_FALSE;
    if (!initJoysticks()) return GLFW_FALSE;
    _GLFWjoystick* js = _glfw.joysticks + jid;
    return js->connected ? _glfw.platform.pollJoystick(js,_GLFW_POLL_PRESENCE) : GLFW_FALSE;
}

GLFWAPI const float* glfwGetJoystickAxes(int jid,int* count) {
    *count = 0; _GLFW_REQUIRE_INIT_OR_RETURN(NULL);
    if (jid < 0 || jid > GLFW_JOYSTICK_LAST) return _glfwInputError(GLFW_INVALID_ENUM,"Invalid joystick ID %i",jid),NULL;
    if (!initJoysticks()) return NULL;
    _GLFWjoystick* js = _glfw.joysticks + jid;
    if (!js->connected || !_glfw.platform.pollJoystick(js,_GLFW_POLL_AXES)) return NULL;
    return *count = js->axisCount, js->axes;
}

GLFWAPI const unsigned char* glfwGetJoystickButtons(int jid,int* count) {
    *count = 0; _GLFW_REQUIRE_INIT_OR_RETURN(NULL);
    if (jid < 0 || jid > GLFW_JOYSTICK_LAST) return _glfwInputError(GLFW_INVALID_ENUM,"Invalid joystick ID %i",jid),NULL;
    if (!initJoysticks()) return NULL;
    _GLFWjoystick* js = _glfw.joysticks + jid;
    if (!js->connected || !_glfw.platform.pollJoystick(js,_GLFW_POLL_BUTTONS)) return NULL;
    *count = _glfw.hints.init.hatButtons ? js->buttonCount + js->hatCount * 4 : js->buttonCount;
    return js->buttons;
}

GLFWAPI const unsigned char* glfwGetJoystickHats(int jid,int* count) {
    *count = 0; _GLFW_REQUIRE_INIT_OR_RETURN(NULL);
    if (jid < 0 || jid > GLFW_JOYSTICK_LAST) return _glfwInputError(GLFW_INVALID_ENUM,"Invalid joystick ID %i",jid),NULL;
    if (!initJoysticks()) return NULL;
    _GLFWjoystick* js = _glfw.joysticks + jid;
    if (!js->connected || !_glfw.platform.pollJoystick(js,_GLFW_POLL_BUTTONS)) return NULL;
    return *count = js->hatCount, js->hats;
}

GLFWAPI const char* glfwGetJoystickName(int jid) {
    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);
    if (jid < 0 || jid > GLFW_JOYSTICK_LAST) return _glfwInputError(GLFW_INVALID_ENUM,"Invalid joystick ID %i",jid),NULL;
    if (!initJoysticks()) return NULL;
    _GLFWjoystick* js = _glfw.joysticks + jid;
    if (!js->connected || !_glfw.platform.pollJoystick(js,_GLFW_POLL_PRESENCE)) return NULL;
    return js->name;
}

GLFWAPI const char* glfwGetJoystickGUID(int jid) {
    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);
    if (jid < 0 || jid > GLFW_JOYSTICK_LAST) return _glfwInputError(GLFW_INVALID_ENUM,"Invalid joystick ID %i",jid),NULL;
    if (!initJoysticks()) return NULL;
    _GLFWjoystick* js = _glfw.joysticks + jid;
    if (!js->connected || !_glfw.platform.pollJoystick(js,_GLFW_POLL_PRESENCE)) return NULL;
    return js->guid;
}

GLFWAPI void glfwSetJoystickUserPointer(int jid,void* pointer) { _GLFW_REQUIRE_INIT(); _GLFWjoystick* js = _glfw.joysticks + jid; if (js->allocated) js->userPointer = pointer; }
GLFWAPI void* glfwGetJoystickUserPointer(int jid) { _GLFW_REQUIRE_INIT_OR_RETURN(NULL); _GLFWjoystick* js = _glfw.joysticks + jid; return js->allocated ? js->userPointer : NULL; }

GLFWAPI GLFWjoystickfun glfwSetJoystickCallback(GLFWjoystickfun cbfun) {
    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);
    if (!initJoysticks()) return NULL;
    _GLFW_SWAP(GLFWjoystickfun,_glfw.callbacks.joystick,cbfun);
    return cbfun;
}

GLFWAPI int glfwUpdateGamepadMappings(const char* string) {
    _GLFW_REQUIRE_INIT_OR_RETURN(GLFW_FALSE);
    for (const char* c = string; *c;) {
        if ((*c >= '0' && *c <= '9') || (*c >= 'a' && *c <= 'f') || (*c >= 'A' && *c <= 'F')) {
            char line[1024];
            const size_t length = strcspn(c,"\r\n");
            if (length < sizeof(line)) {
                _GLFWmapping mapping = {{0},{0},{0},{0}};
                memcpy(line,c,length); line[length] = '\0';
                if (parseMapping(&mapping,line)) {
                    _GLFWmapping* previous = findMapping(mapping.guid);
                    if (previous) *previous = mapping;
                    else { _glfw.mappings = _glfw_realloc(_glfw.mappings,sizeof(_GLFWmapping) * ++_glfw.mappingCount); _glfw.mappings[_glfw.mappingCount-1] = mapping; }
                }
            }
            c += length;
        } else { c += strcspn(c,"\r\n"); c += strspn(c,"\r\n"); }
    }
    for (int jid = 0; jid <= GLFW_JOYSTICK_LAST; jid++) { _GLFWjoystick* js = _glfw.joysticks + jid; if (js->connected) js->mapping = findValidMapping(js); }
    return GLFW_TRUE;
}

GLFWAPI int glfwJoystickIsGamepad(int jid) {
    _GLFW_REQUIRE_INIT_OR_RETURN(GLFW_FALSE);
    if (jid < 0 || jid > GLFW_JOYSTICK_LAST) return _glfwInputError(GLFW_INVALID_ENUM,"Invalid joystick ID %i",jid),GLFW_FALSE;
    if (!initJoysticks()) return GLFW_FALSE;
    _GLFWjoystick* js = _glfw.joysticks + jid;
    if (!js->connected || !_glfw.platform.pollJoystick(js,_GLFW_POLL_PRESENCE)) return GLFW_FALSE;
    return js->mapping != NULL;
}

GLFWAPI const char* glfwGetGamepadName(int jid) {
    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);
    if (jid < 0 || jid > GLFW_JOYSTICK_LAST) return _glfwInputError(GLFW_INVALID_ENUM,"Invalid joystick ID %i",jid),NULL;
    if (!initJoysticks()) return NULL;
    _GLFWjoystick* js = _glfw.joysticks + jid;
    if (!js->connected || !_glfw.platform.pollJoystick(js,_GLFW_POLL_PRESENCE) || !js->mapping) return NULL;
    return js->mapping->name;
}

GLFWAPI int glfwGetGamepadState(int jid,GLFWgamepadstate* state) {
    memset(state,0,sizeof(GLFWgamepadstate));
    _GLFW_REQUIRE_INIT_OR_RETURN(GLFW_FALSE);
    if (jid < 0 || jid > GLFW_JOYSTICK_LAST) return _glfwInputError(GLFW_INVALID_ENUM,"Invalid joystick ID %i",jid),GLFW_FALSE;
    if (!initJoysticks()) return GLFW_FALSE;
    _GLFWjoystick* js = _glfw.joysticks + jid;
    if (!js->connected || !_glfw.platform.pollJoystick(js,_GLFW_POLL_ALL) || !js->mapping) return GLFW_FALSE;
    for (int i = 0; i <= GLFW_GAMEPAD_BUTTON_LAST; i++) {
        const _GLFWmapelement* e = js->mapping->buttons + i;
        if (e->type == _GLFW_JOYSTICK_AXIS) {
            const float value = js->axes[e->index] * e->axisScale + e->axisOffset;
            if (e->axisOffset < 0 || (e->axisOffset == 0 && e->axisScale > 0)) { if (value >= 0.f) state->buttons[i] = GLFW_PRESS; }
            else { if (value <= 0.f) state->buttons[i] = GLFW_PRESS; }
        } else if (e->type == _GLFW_JOYSTICK_HATBIT) { if (js->hats[e->index >> 4] & (e->index & 0xf)) state->buttons[i] = GLFW_PRESS; }
        else if (e->type == _GLFW_JOYSTICK_BUTTON) state->buttons[i] = js->buttons[e->index];
    }
    for (int i = 0; i <= GLFW_GAMEPAD_AXIS_LAST; i++) {
        const _GLFWmapelement* e = js->mapping->axes + i;
        if (e->type == _GLFW_JOYSTICK_AXIS) state->axes[i] = fminf(fmaxf(js->axes[e->index] * e->axisScale + e->axisOffset,-1.f),1.f);
        else if (e->type == _GLFW_JOYSTICK_HATBIT) state->axes[i] = (js->hats[e->index >> 4] & (e->index & 0xf)) ? 1.f : -1.f;
        else if (e->type == _GLFW_JOYSTICK_BUTTON) state->axes[i] = js->buttons[e->index] * 2.f - 1.f;
    }
    return GLFW_TRUE;
}
