#include "internal.h"
#if defined(_GLFW_X11)
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <locale.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>

static int translateKeySyms(const KeySym* keysyms, int width) {
    if (width > 1) {
        switch (keysyms[1]) {
            case XK_KP_0: return GLFW_KEY_KP_0; case XK_KP_1: return GLFW_KEY_KP_1;
            case XK_KP_2: return GLFW_KEY_KP_2; case XK_KP_3: return GLFW_KEY_KP_3;
            case XK_KP_4: return GLFW_KEY_KP_4; case XK_KP_5: return GLFW_KEY_KP_5;
            case XK_KP_6: return GLFW_KEY_KP_6; case XK_KP_7: return GLFW_KEY_KP_7;
            case XK_KP_8: return GLFW_KEY_KP_8; case XK_KP_9: return GLFW_KEY_KP_9;
            case XK_KP_Separator: case XK_KP_Decimal: return GLFW_KEY_KP_DECIMAL;
            case XK_KP_Equal: return GLFW_KEY_KP_EQUAL;
            case XK_KP_Enter: return GLFW_KEY_KP_ENTER;
            default: break;
        }
    }
    switch (keysyms[0]) {
        case XK_Escape: return GLFW_KEY_ESCAPE;       case XK_Tab: return GLFW_KEY_TAB;
        case XK_Shift_L: return GLFW_KEY_LEFT_SHIFT;  case XK_Shift_R: return GLFW_KEY_RIGHT_SHIFT;
        case XK_Control_L: return GLFW_KEY_LEFT_CONTROL; case XK_Control_R: return GLFW_KEY_RIGHT_CONTROL;
        case XK_Meta_L: case XK_Alt_L: return GLFW_KEY_LEFT_ALT;
        case XK_Mode_switch: case XK_ISO_Level3_Shift: case XK_Meta_R: case XK_Alt_R: return GLFW_KEY_RIGHT_ALT;
        case XK_Super_L: return GLFW_KEY_LEFT_SUPER;  case XK_Super_R: return GLFW_KEY_RIGHT_SUPER;
        case XK_Menu: return GLFW_KEY_MENU;            case XK_Num_Lock: return GLFW_KEY_NUM_LOCK;
        case XK_Caps_Lock: return GLFW_KEY_CAPS_LOCK;  case XK_Print: return GLFW_KEY_PRINT_SCREEN;
        case XK_Scroll_Lock: return GLFW_KEY_SCROLL_LOCK; case XK_Pause: return GLFW_KEY_PAUSE;
        case XK_Delete: return GLFW_KEY_DELETE;        case XK_BackSpace: return GLFW_KEY_BACKSPACE;
        case XK_Return: return GLFW_KEY_ENTER;         case XK_Home: return GLFW_KEY_HOME;
        case XK_End: return GLFW_KEY_END;              case XK_Page_Up: return GLFW_KEY_PAGE_UP;
        case XK_Page_Down: return GLFW_KEY_PAGE_DOWN;  case XK_Insert: return GLFW_KEY_INSERT;
        case XK_Left: return GLFW_KEY_LEFT;            case XK_Right: return GLFW_KEY_RIGHT;
        case XK_Down: return GLFW_KEY_DOWN;            case XK_Up: return GLFW_KEY_UP;
        case XK_F1:  return GLFW_KEY_F1;  case XK_F2:  return GLFW_KEY_F2;  case XK_F3:  return GLFW_KEY_F3;
        case XK_F4:  return GLFW_KEY_F4;  case XK_F5:  return GLFW_KEY_F5;  case XK_F6:  return GLFW_KEY_F6;
        case XK_F7:  return GLFW_KEY_F7;  case XK_F8:  return GLFW_KEY_F8;  case XK_F9:  return GLFW_KEY_F9;
        case XK_F10: return GLFW_KEY_F10; case XK_F11: return GLFW_KEY_F11; case XK_F12: return GLFW_KEY_F12;
        case XK_F13: return GLFW_KEY_F13; case XK_F14: return GLFW_KEY_F14; case XK_F15: return GLFW_KEY_F15;
        case XK_F16: return GLFW_KEY_F16; case XK_F17: return GLFW_KEY_F17; case XK_F18: return GLFW_KEY_F18;
        case XK_F19: return GLFW_KEY_F19; case XK_F20: return GLFW_KEY_F20; case XK_F21: return GLFW_KEY_F21;
        case XK_F22: return GLFW_KEY_F22; case XK_F23: return GLFW_KEY_F23; case XK_F24: return GLFW_KEY_F24;
        case XK_F25: return GLFW_KEY_F25;
        case XK_KP_Divide: return GLFW_KEY_KP_DIVIDE;   case XK_KP_Multiply: return GLFW_KEY_KP_MULTIPLY;
        case XK_KP_Subtract: return GLFW_KEY_KP_SUBTRACT; case XK_KP_Add: return GLFW_KEY_KP_ADD;
        case XK_KP_Insert: return GLFW_KEY_KP_0;    case XK_KP_End: return GLFW_KEY_KP_1;
        case XK_KP_Down: return GLFW_KEY_KP_2;       case XK_KP_Page_Down: return GLFW_KEY_KP_3;
        case XK_KP_Left: return GLFW_KEY_KP_4;       case XK_KP_Right: return GLFW_KEY_KP_6;
        case XK_KP_Home: return GLFW_KEY_KP_7;       case XK_KP_Up: return GLFW_KEY_KP_8;
        case XK_KP_Page_Up: return GLFW_KEY_KP_9;    case XK_KP_Delete: return GLFW_KEY_KP_DECIMAL;
        case XK_KP_Equal: return GLFW_KEY_KP_EQUAL;  case XK_KP_Enter: return GLFW_KEY_KP_ENTER;
        case XK_a: return GLFW_KEY_A; case XK_b: return GLFW_KEY_B; case XK_c: return GLFW_KEY_C;
        case XK_d: return GLFW_KEY_D; case XK_e: return GLFW_KEY_E; case XK_f: return GLFW_KEY_F;
        case XK_g: return GLFW_KEY_G; case XK_h: return GLFW_KEY_H; case XK_i: return GLFW_KEY_I;
        case XK_j: return GLFW_KEY_J; case XK_k: return GLFW_KEY_K; case XK_l: return GLFW_KEY_L;
        case XK_m: return GLFW_KEY_M; case XK_n: return GLFW_KEY_N; case XK_o: return GLFW_KEY_O;
        case XK_p: return GLFW_KEY_P; case XK_q: return GLFW_KEY_Q; case XK_r: return GLFW_KEY_R;
        case XK_s: return GLFW_KEY_S; case XK_t: return GLFW_KEY_T; case XK_u: return GLFW_KEY_U;
        case XK_v: return GLFW_KEY_V; case XK_w: return GLFW_KEY_W; case XK_x: return GLFW_KEY_X;
        case XK_y: return GLFW_KEY_Y; case XK_z: return GLFW_KEY_Z;
        case XK_1: return GLFW_KEY_1; case XK_2: return GLFW_KEY_2; case XK_3: return GLFW_KEY_3;
        case XK_4: return GLFW_KEY_4; case XK_5: return GLFW_KEY_5; case XK_6: return GLFW_KEY_6;
        case XK_7: return GLFW_KEY_7; case XK_8: return GLFW_KEY_8; case XK_9: return GLFW_KEY_9;
        case XK_0: return GLFW_KEY_0;
        case XK_space: return GLFW_KEY_SPACE;           case XK_minus: return GLFW_KEY_MINUS;
        case XK_equal: return GLFW_KEY_EQUAL;           case XK_bracketleft: return GLFW_KEY_LEFT_BRACKET;
        case XK_bracketright: return GLFW_KEY_RIGHT_BRACKET; case XK_backslash: return GLFW_KEY_BACKSLASH;
        case XK_semicolon: return GLFW_KEY_SEMICOLON;   case XK_apostrophe: return GLFW_KEY_APOSTROPHE;
        case XK_grave: return GLFW_KEY_GRAVE_ACCENT;    case XK_comma: return GLFW_KEY_COMMA;
        case XK_period: return GLFW_KEY_PERIOD;         case XK_slash: return GLFW_KEY_SLASH;
        case XK_less: return GLFW_KEY_WORLD_1;
        default: break;
    }
    return GLFW_KEY_UNKNOWN;
}

static void createKeyTables(void)
{
    int scancodeMin, scancodeMax;
    memset(_glfw.x11.keycodes, -1, sizeof(_glfw.x11.keycodes));
    memset(_glfw.x11.scancodes, -1, sizeof(_glfw.x11.scancodes));

    if (_glfw.x11.xkb.available) {
        XkbDescPtr desc = XkbGetMap(_glfw.x11.display, 0, XkbUseCoreKbd);
        XkbGetNames(_glfw.x11.display, XkbKeyNamesMask | XkbKeyAliasesMask, desc);
        scancodeMin = desc->min_key_code;
        scancodeMax = desc->max_key_code;

        const struct { int key; char* name; } keymap[] = {
            {GLFW_KEY_GRAVE_ACCENT,"TLDE"},{GLFW_KEY_1,"AE01"},{GLFW_KEY_2,"AE02"},{GLFW_KEY_3,"AE03"},
            {GLFW_KEY_4,"AE04"},{GLFW_KEY_5,"AE05"},{GLFW_KEY_6,"AE06"},{GLFW_KEY_7,"AE07"},
            {GLFW_KEY_8,"AE08"},{GLFW_KEY_9,"AE09"},{GLFW_KEY_0,"AE10"},{GLFW_KEY_MINUS,"AE11"},
            {GLFW_KEY_EQUAL,"AE12"},{GLFW_KEY_Q,"AD01"},{GLFW_KEY_W,"AD02"},{GLFW_KEY_E,"AD03"},
            {GLFW_KEY_R,"AD04"},{GLFW_KEY_T,"AD05"},{GLFW_KEY_Y,"AD06"},{GLFW_KEY_U,"AD07"},
            {GLFW_KEY_I,"AD08"},{GLFW_KEY_O,"AD09"},{GLFW_KEY_P,"AD10"},{GLFW_KEY_LEFT_BRACKET,"AD11"},
            {GLFW_KEY_RIGHT_BRACKET,"AD12"},{GLFW_KEY_A,"AC01"},{GLFW_KEY_S,"AC02"},{GLFW_KEY_D,"AC03"},
            {GLFW_KEY_F,"AC04"},{GLFW_KEY_G,"AC05"},{GLFW_KEY_H,"AC06"},{GLFW_KEY_J,"AC07"},
            {GLFW_KEY_K,"AC08"},{GLFW_KEY_L,"AC09"},{GLFW_KEY_SEMICOLON,"AC10"},{GLFW_KEY_APOSTROPHE,"AC11"},
            {GLFW_KEY_Z,"AB01"},{GLFW_KEY_X,"AB02"},{GLFW_KEY_C,"AB03"},{GLFW_KEY_V,"AB04"},
            {GLFW_KEY_B,"AB05"},{GLFW_KEY_N,"AB06"},{GLFW_KEY_M,"AB07"},{GLFW_KEY_COMMA,"AB08"},
            {GLFW_KEY_PERIOD,"AB09"},{GLFW_KEY_SLASH,"AB10"},{GLFW_KEY_BACKSLASH,"BKSL"},
            {GLFW_KEY_WORLD_1,"LSGT"},{GLFW_KEY_SPACE,"SPCE"},{GLFW_KEY_ESCAPE,"ESC"},
            {GLFW_KEY_ENTER,"RTRN"},{GLFW_KEY_TAB,"TAB"},{GLFW_KEY_BACKSPACE,"BKSP"},
            {GLFW_KEY_INSERT,"INS"},{GLFW_KEY_DELETE,"DELE"},{GLFW_KEY_RIGHT,"RGHT"},
            {GLFW_KEY_LEFT,"LEFT"},{GLFW_KEY_DOWN,"DOWN"},{GLFW_KEY_UP,"UP"},
            {GLFW_KEY_PAGE_UP,"PGUP"},{GLFW_KEY_PAGE_DOWN,"PGDN"},{GLFW_KEY_HOME,"HOME"},
            {GLFW_KEY_END,"END"},{GLFW_KEY_CAPS_LOCK,"CAPS"},{GLFW_KEY_SCROLL_LOCK,"SCLK"},
            {GLFW_KEY_NUM_LOCK,"NMLK"},{GLFW_KEY_PRINT_SCREEN,"PRSC"},{GLFW_KEY_PAUSE,"PAUS"},
            {GLFW_KEY_F1,"FK01"},{GLFW_KEY_F2,"FK02"},{GLFW_KEY_F3,"FK03"},{GLFW_KEY_F4,"FK04"},
            {GLFW_KEY_F5,"FK05"},{GLFW_KEY_F6,"FK06"},{GLFW_KEY_F7,"FK07"},{GLFW_KEY_F8,"FK08"},
            {GLFW_KEY_F9,"FK09"},{GLFW_KEY_F10,"FK10"},{GLFW_KEY_F11,"FK11"},{GLFW_KEY_F12,"FK12"},
            {GLFW_KEY_F13,"FK13"},{GLFW_KEY_F14,"FK14"},{GLFW_KEY_F15,"FK15"},{GLFW_KEY_F16,"FK16"},
            {GLFW_KEY_F17,"FK17"},{GLFW_KEY_F18,"FK18"},{GLFW_KEY_F19,"FK19"},{GLFW_KEY_F20,"FK20"},
            {GLFW_KEY_F21,"FK21"},{GLFW_KEY_F22,"FK22"},{GLFW_KEY_F23,"FK23"},{GLFW_KEY_F24,"FK24"},
            {GLFW_KEY_F25,"FK25"},{GLFW_KEY_KP_0,"KP0"},{GLFW_KEY_KP_1,"KP1"},{GLFW_KEY_KP_2,"KP2"},
            {GLFW_KEY_KP_3,"KP3"},{GLFW_KEY_KP_4,"KP4"},{GLFW_KEY_KP_5,"KP5"},{GLFW_KEY_KP_6,"KP6"},
            {GLFW_KEY_KP_7,"KP7"},{GLFW_KEY_KP_8,"KP8"},{GLFW_KEY_KP_9,"KP9"},
            {GLFW_KEY_KP_DECIMAL,"KPDL"},{GLFW_KEY_KP_DIVIDE,"KPDV"},{GLFW_KEY_KP_MULTIPLY,"KPMU"},
            {GLFW_KEY_KP_SUBTRACT,"KPSU"},{GLFW_KEY_KP_ADD,"KPAD"},{GLFW_KEY_KP_ENTER,"KPEN"},
            {GLFW_KEY_KP_EQUAL,"KPEQ"},{GLFW_KEY_LEFT_SHIFT,"LFSH"},{GLFW_KEY_LEFT_CONTROL,"LCTL"},
            {GLFW_KEY_LEFT_ALT,"LALT"},{GLFW_KEY_LEFT_SUPER,"LWIN"},{GLFW_KEY_RIGHT_SHIFT,"RTSH"},
            {GLFW_KEY_RIGHT_CONTROL,"RCTL"},{GLFW_KEY_RIGHT_ALT,"RALT"},{GLFW_KEY_RIGHT_ALT,"LVL3"},
            {GLFW_KEY_RIGHT_ALT,"MDSW"},{GLFW_KEY_RIGHT_SUPER,"RWIN"},{GLFW_KEY_MENU,"MENU"},
        };

        for (int sc = scancodeMin; sc <= scancodeMax; sc++) {
            int key = GLFW_KEY_UNKNOWN;
            for (int i = 0; i < (int)(sizeof(keymap)/sizeof(keymap[0])); i++) {
                if (strncmp(desc->names->keys[sc].name, keymap[i].name, XkbKeyNameLength) == 0)
                    { key = keymap[i].key; break; }
            }
            for (int i = 0; i < desc->names->num_key_aliases && key == GLFW_KEY_UNKNOWN; i++) {
                if (strncmp(desc->names->key_aliases[i].real, desc->names->keys[sc].name, XkbKeyNameLength) != 0) continue;
                for (int j = 0; j < (int)(sizeof(keymap)/sizeof(keymap[0])); j++) {
                    if (strncmp(desc->names->key_aliases[i].alias, keymap[j].name, XkbKeyNameLength) == 0)
                        { key = keymap[j].key; break; }
                }
            }
            _glfw.x11.keycodes[sc] = key;
        }
        XkbFreeNames(desc, XkbKeyNamesMask, True);
        XkbFreeKeyboard(desc, 0, True);
    } else
        XDisplayKeycodes(_glfw.x11.display, &scancodeMin, &scancodeMax);

    int width;
    KeySym* keysyms = XGetKeyboardMapping(_glfw.x11.display, scancodeMin, scancodeMax - scancodeMin + 1, &width);
    for (int sc = scancodeMin; sc <= scancodeMax; sc++) {
        if (_glfw.x11.keycodes[sc] < 0) _glfw.x11.keycodes[sc] = translateKeySyms(&keysyms[(sc - scancodeMin) * width], width);
        if (_glfw.x11.keycodes[sc] > 0) _glfw.x11.scancodes[_glfw.x11.keycodes[sc]] = sc;
    }
    XFree(keysyms);
}

static GLFWbool hasUsableInputMethodStyle(void) {
    GLFWbool found = GLFW_FALSE;
    XIMStyles* styles = NULL;
    if (XGetIMValues(_glfw.x11.im, XNQueryInputStyle, &styles, NULL) != NULL) return GLFW_FALSE;
    for (unsigned int i = 0; i < styles->count_styles; i++) {
        if (styles->supported_styles[i] == (XIMPreeditNothing | XIMStatusNothing)) { found = GLFW_TRUE; break; }
    }
    XFree(styles);
    return found;
}

static void inputMethodDestroyCallback(XIM im, XPointer clientData, XPointer callData) { (void)im; (void)clientData; (void)callData; _glfw.x11.im = NULL; }
static void inputMethodInstantiateCallback(Display* display, XPointer clientData, XPointer callData) {
    (void)display; (void)clientData; (void)callData;
    if (_glfw.x11.im) return;
    _glfw.x11.im = XOpenIM(_glfw.x11.display, 0, NULL, NULL);
    if (_glfw.x11.im) {
        if (!hasUsableInputMethodStyle()) { XCloseIM(_glfw.x11.im); _glfw.x11.im = NULL; }
    }
    if (_glfw.x11.im) {
        XIMCallback cb = { .callback = (XIMProc)inputMethodDestroyCallback, .client_data = NULL };
        XSetIMValues(_glfw.x11.im, XNDestroyCallback, &cb, NULL);
        for (_GLFWwindow* w = _glfw.windowListHead; w; w = w->next)
            _glfwCreateInputContextX11(w);
    }
}

static Atom getAtomIfSupported(Atom* atoms, unsigned long count, const char* name) {
    const Atom atom = XInternAtom(_glfw.x11.display, name, False);
    for (unsigned long i = 0; i < count; i++) if (atoms[i] == atom) return atom;
    return None;
}

static void detectEWMH(void) {
    Window* wfr = NULL;
    if (!_glfwGetWindowPropertyX11(_glfw.x11.root, _glfw.x11.NET_SUPPORTING_WM_CHECK, XA_WINDOW, (unsigned char**)&wfr)) return;
    _glfwGrabErrorHandlerX11();
    Window* wfc = NULL;
    if (!_glfwGetWindowPropertyX11(*wfr, _glfw.x11.NET_SUPPORTING_WM_CHECK, XA_WINDOW, (unsigned char**)&wfc))
        { _glfwReleaseErrorHandlerX11(); XFree(wfr); return; }
    _glfwReleaseErrorHandlerX11();
    if (*wfr != *wfc) { XFree(wfr); XFree(wfc); return; }
    XFree(wfr); XFree(wfc);

    Atom* sa = NULL;
    const unsigned long ac = _glfwGetWindowPropertyX11(_glfw.x11.root, _glfw.x11.NET_SUPPORTED, XA_ATOM, (unsigned char**)&sa);
    #define GA(name) getAtomIfSupported(sa, ac, name)
    _glfw.x11.NET_WM_STATE                  = GA("_NET_WM_STATE");
    _glfw.x11.NET_WM_STATE_ABOVE            = GA("_NET_WM_STATE_ABOVE");
    _glfw.x11.NET_WM_STATE_FULLSCREEN       = GA("_NET_WM_STATE_FULLSCREEN");
    _glfw.x11.NET_WM_STATE_MAXIMIZED_VERT   = GA("_NET_WM_STATE_MAXIMIZED_VERT");
    _glfw.x11.NET_WM_STATE_MAXIMIZED_HORZ   = GA("_NET_WM_STATE_MAXIMIZED_HORZ");
    _glfw.x11.NET_WM_STATE_DEMANDS_ATTENTION= GA("_NET_WM_STATE_DEMANDS_ATTENTION");
    _glfw.x11.NET_WM_FULLSCREEN_MONITORS    = GA("_NET_WM_FULLSCREEN_MONITORS");
    _glfw.x11.NET_WM_WINDOW_TYPE            = GA("_NET_WM_WINDOW_TYPE");
    _glfw.x11.NET_WM_WINDOW_TYPE_NORMAL     = GA("_NET_WM_WINDOW_TYPE_NORMAL");
    _glfw.x11.NET_WORKAREA                  = GA("_NET_WORKAREA");
    _glfw.x11.NET_CURRENT_DESKTOP           = GA("_NET_CURRENT_DESKTOP");
    _glfw.x11.NET_ACTIVE_WINDOW             = GA("_NET_ACTIVE_WINDOW");
    _glfw.x11.NET_FRAME_EXTENTS             = GA("_NET_FRAME_EXTENTS");
    _glfw.x11.NET_REQUEST_FRAME_EXTENTS     = GA("_NET_REQUEST_FRAME_EXTENTS");
    #undef GA
    if (sa) XFree(sa);
}

static GLFWbool initExtensions(void) {
    _glfw.x11.vidmode.handle = _glfwPlatformLoadModule("libXxf86vm.so.1");
    if (_glfw.x11.vidmode.handle) {
        _glfw.x11.vidmode.QueryExtension  = (PFN_XF86VidModeQueryExtension)  _glfwPlatformGetModuleSymbol(_glfw.x11.vidmode.handle, "XF86VidModeQueryExtension");
        _glfw.x11.vidmode.GetGammaRamp    = (PFN_XF86VidModeGetGammaRamp)    _glfwPlatformGetModuleSymbol(_glfw.x11.vidmode.handle, "XF86VidModeGetGammaRamp");
        _glfw.x11.vidmode.SetGammaRamp    = (PFN_XF86VidModeSetGammaRamp)    _glfwPlatformGetModuleSymbol(_glfw.x11.vidmode.handle, "XF86VidModeSetGammaRamp");
        _glfw.x11.vidmode.GetGammaRampSize= (PFN_XF86VidModeGetGammaRampSize)_glfwPlatformGetModuleSymbol(_glfw.x11.vidmode.handle, "XF86VidModeGetGammaRampSize");
        _glfw.x11.vidmode.available = XF86VidModeQueryExtension(_glfw.x11.display, &_glfw.x11.vidmode.eventBase, &_glfw.x11.vidmode.errorBase);
    }

    _glfw.x11.xi.handle = _glfwPlatformLoadModule("libXi.so.6");
    if (_glfw.x11.xi.handle) {
        _glfw.x11.xi.QueryVersion = (PFN_XIQueryVersion) _glfwPlatformGetModuleSymbol(_glfw.x11.xi.handle, "XIQueryVersion");
        _glfw.x11.xi.SelectEvents = (PFN_XISelectEvents) _glfwPlatformGetModuleSymbol(_glfw.x11.xi.handle, "XISelectEvents");
        if (XQueryExtension(_glfw.x11.display, "XInputExtension", &_glfw.x11.xi.majorOpcode, &_glfw.x11.xi.eventBase, &_glfw.x11.xi.errorBase)) {
            _glfw.x11.xi.major = 2; _glfw.x11.xi.minor = 0;
            if (XIQueryVersion(_glfw.x11.display, &_glfw.x11.xi.major, &_glfw.x11.xi.minor) == Success)
                _glfw.x11.xi.available = GLFW_TRUE;
        }
    }

    _glfw.x11.randr.handle = _glfwPlatformLoadModule("libXrandr.so.2");
    if (_glfw.x11.randr.handle) {
        _glfw.x11.randr.AllocGamma             = (PFN_XRRAllocGamma)             _glfwPlatformGetModuleSymbol(_glfw.x11.randr.handle, "XRRAllocGamma");
        _glfw.x11.randr.FreeGamma              = (PFN_XRRFreeGamma)              _glfwPlatformGetModuleSymbol(_glfw.x11.randr.handle, "XRRFreeGamma");
        _glfw.x11.randr.FreeCrtcInfo           = (PFN_XRRFreeCrtcInfo)           _glfwPlatformGetModuleSymbol(_glfw.x11.randr.handle, "XRRFreeCrtcInfo");
        _glfw.x11.randr.FreeOutputInfo         = (PFN_XRRFreeOutputInfo)         _glfwPlatformGetModuleSymbol(_glfw.x11.randr.handle, "XRRFreeOutputInfo");
        _glfw.x11.randr.FreeScreenResources    = (PFN_XRRFreeScreenResources)    _glfwPlatformGetModuleSymbol(_glfw.x11.randr.handle, "XRRFreeScreenResources");
        _glfw.x11.randr.GetCrtcGamma           = (PFN_XRRGetCrtcGamma)           _glfwPlatformGetModuleSymbol(_glfw.x11.randr.handle, "XRRGetCrtcGamma");
        _glfw.x11.randr.GetCrtcGammaSize       = (PFN_XRRGetCrtcGammaSize)       _glfwPlatformGetModuleSymbol(_glfw.x11.randr.handle, "XRRGetCrtcGammaSize");
        _glfw.x11.randr.GetCrtcInfo            = (PFN_XRRGetCrtcInfo)            _glfwPlatformGetModuleSymbol(_glfw.x11.randr.handle, "XRRGetCrtcInfo");
        _glfw.x11.randr.GetOutputInfo          = (PFN_XRRGetOutputInfo)          _glfwPlatformGetModuleSymbol(_glfw.x11.randr.handle, "XRRGetOutputInfo");
        _glfw.x11.randr.GetOutputPrimary       = (PFN_XRRGetOutputPrimary)       _glfwPlatformGetModuleSymbol(_glfw.x11.randr.handle, "XRRGetOutputPrimary");
        _glfw.x11.randr.GetScreenResourcesCurrent=(PFN_XRRGetScreenResourcesCurrent)_glfwPlatformGetModuleSymbol(_glfw.x11.randr.handle, "XRRGetScreenResourcesCurrent");
        _glfw.x11.randr.QueryExtension         = (PFN_XRRQueryExtension)         _glfwPlatformGetModuleSymbol(_glfw.x11.randr.handle, "XRRQueryExtension");
        _glfw.x11.randr.QueryVersion           = (PFN_XRRQueryVersion)           _glfwPlatformGetModuleSymbol(_glfw.x11.randr.handle, "XRRQueryVersion");
        _glfw.x11.randr.SelectInput            = (PFN_XRRSelectInput)            _glfwPlatformGetModuleSymbol(_glfw.x11.randr.handle, "XRRSelectInput");
        _glfw.x11.randr.SetCrtcConfig          = (PFN_XRRSetCrtcConfig)          _glfwPlatformGetModuleSymbol(_glfw.x11.randr.handle, "XRRSetCrtcConfig");
        _glfw.x11.randr.SetCrtcGamma           = (PFN_XRRSetCrtcGamma)           _glfwPlatformGetModuleSymbol(_glfw.x11.randr.handle, "XRRSetCrtcGamma");
        _glfw.x11.randr.UpdateConfiguration   = (PFN_XRRUpdateConfiguration)    _glfwPlatformGetModuleSymbol(_glfw.x11.randr.handle, "XRRUpdateConfiguration");
        if (XRRQueryExtension(_glfw.x11.display, &_glfw.x11.randr.eventBase, &_glfw.x11.randr.errorBase)) {
            if (XRRQueryVersion(_glfw.x11.display, &_glfw.x11.randr.major, &_glfw.x11.randr.minor)) {
                if (_glfw.x11.randr.major > 1 || _glfw.x11.randr.minor >= 3) _glfw.x11.randr.available = GLFW_TRUE;
            } else _glfwInputError(GLFW_PLATFORM_ERROR, "X11: Failed to query RandR version");
        }
    }

    if (_glfw.x11.randr.available) {
        XRRScreenResources* sr = XRRGetScreenResourcesCurrent(_glfw.x11.display, _glfw.x11.root);
        if (!sr->ncrtc || !XRRGetCrtcGammaSize(_glfw.x11.display, sr->crtcs[0])) _glfw.x11.randr.gammaBroken  = GLFW_TRUE;
        if (!sr->ncrtc)                                                             _glfw.x11.randr.monitorBroken = GLFW_TRUE;
        XRRFreeScreenResources(sr);
    }
    if (_glfw.x11.randr.available && !_glfw.x11.randr.monitorBroken)
        XRRSelectInput(_glfw.x11.display, _glfw.x11.root, RROutputChangeNotifyMask);

    _glfw.x11.xcursor.handle = _glfwPlatformLoadModule("libXcursor.so.1");
    if (_glfw.x11.xcursor.handle) {
        _glfw.x11.xcursor.ImageCreate      = (PFN_XcursorImageCreate)      _glfwPlatformGetModuleSymbol(_glfw.x11.xcursor.handle, "XcursorImageCreate");
        _glfw.x11.xcursor.ImageDestroy     = (PFN_XcursorImageDestroy)     _glfwPlatformGetModuleSymbol(_glfw.x11.xcursor.handle, "XcursorImageDestroy");
        _glfw.x11.xcursor.ImageLoadCursor  = (PFN_XcursorImageLoadCursor)  _glfwPlatformGetModuleSymbol(_glfw.x11.xcursor.handle, "XcursorImageLoadCursor");
        _glfw.x11.xcursor.GetTheme         = (PFN_XcursorGetTheme)         _glfwPlatformGetModuleSymbol(_glfw.x11.xcursor.handle, "XcursorGetTheme");
        _glfw.x11.xcursor.GetDefaultSize   = (PFN_XcursorGetDefaultSize)   _glfwPlatformGetModuleSymbol(_glfw.x11.xcursor.handle, "XcursorGetDefaultSize");
        _glfw.x11.xcursor.LibraryLoadImage = (PFN_XcursorLibraryLoadImage) _glfwPlatformGetModuleSymbol(_glfw.x11.xcursor.handle, "XcursorLibraryLoadImage");
    }

    _glfw.x11.xinerama.handle = _glfwPlatformLoadModule("libXinerama.so.1");
    if (_glfw.x11.xinerama.handle) {
        _glfw.x11.xinerama.IsActive       = (PFN_XineramaIsActive)       _glfwPlatformGetModuleSymbol(_glfw.x11.xinerama.handle, "XineramaIsActive");
        _glfw.x11.xinerama.QueryExtension = (PFN_XineramaQueryExtension) _glfwPlatformGetModuleSymbol(_glfw.x11.xinerama.handle, "XineramaQueryExtension");
        _glfw.x11.xinerama.QueryScreens   = (PFN_XineramaQueryScreens)   _glfwPlatformGetModuleSymbol(_glfw.x11.xinerama.handle, "XineramaQueryScreens");
        if (XineramaQueryExtension(_glfw.x11.display, &_glfw.x11.xinerama.major, &_glfw.x11.xinerama.minor))
            if (XineramaIsActive(_glfw.x11.display)) _glfw.x11.xinerama.available = GLFW_TRUE;
    }

    _glfw.x11.xkb.major = 1; _glfw.x11.xkb.minor = 0;
    _glfw.x11.xkb.available = XkbQueryExtension(_glfw.x11.display, &_glfw.x11.xkb.majorOpcode,
        &_glfw.x11.xkb.eventBase, &_glfw.x11.xkb.errorBase, &_glfw.x11.xkb.major, &_glfw.x11.xkb.minor);
    if (_glfw.x11.xkb.available) {
        Bool supported;
        if (XkbSetDetectableAutoRepeat(_glfw.x11.display, True, &supported) && supported)
            _glfw.x11.xkb.detectable = GLFW_TRUE;
        XkbStateRec state;
        if (XkbGetState(_glfw.x11.display, XkbUseCoreKbd, &state) == Success)
            _glfw.x11.xkb.group = (unsigned int)state.group;
        XkbSelectEventDetails(_glfw.x11.display, XkbUseCoreKbd, XkbStateNotify, XkbGroupStateMask, XkbGroupStateMask);
    }

    // x11xcb: only needed for Vulkan surface, skip entirely
    // xrender
    _glfw.x11.xrender.handle = _glfwPlatformLoadModule("libXrender.so.1");
    if (_glfw.x11.xrender.handle) {
        _glfw.x11.xrender.QueryExtension   = (PFN_XRenderQueryExtension)   _glfwPlatformGetModuleSymbol(_glfw.x11.xrender.handle, "XRenderQueryExtension");
        _glfw.x11.xrender.QueryVersion     = (PFN_XRenderQueryVersion)     _glfwPlatformGetModuleSymbol(_glfw.x11.xrender.handle, "XRenderQueryVersion");
        _glfw.x11.xrender.FindVisualFormat = (PFN_XRenderFindVisualFormat) _glfwPlatformGetModuleSymbol(_glfw.x11.xrender.handle, "XRenderFindVisualFormat");
        if (XRenderQueryExtension(_glfw.x11.display, &_glfw.x11.xrender.errorBase, &_glfw.x11.xrender.eventBase))
            if (XRenderQueryVersion(_glfw.x11.display, &_glfw.x11.xrender.major, &_glfw.x11.xrender.minor))
                _glfw.x11.xrender.available = GLFW_TRUE;
    }

    _glfw.x11.xshape.handle = _glfwPlatformLoadModule("libXext.so.6");
    if (_glfw.x11.xshape.handle) {
        _glfw.x11.xshape.QueryExtension    = (PFN_XShapeQueryExtension)  _glfwPlatformGetModuleSymbol(_glfw.x11.xshape.handle, "XShapeQueryExtension");
        _glfw.x11.xshape.ShapeCombineRegion= (PFN_XShapeCombineRegion)   _glfwPlatformGetModuleSymbol(_glfw.x11.xshape.handle, "XShapeCombineRegion");
        _glfw.x11.xshape.QueryVersion      = (PFN_XShapeQueryVersion)    _glfwPlatformGetModuleSymbol(_glfw.x11.xshape.handle, "XShapeQueryVersion");
        _glfw.x11.xshape.ShapeCombineMask  = (PFN_XShapeCombineMask)     _glfwPlatformGetModuleSymbol(_glfw.x11.xshape.handle, "XShapeCombineMask");
        if (XShapeQueryExtension(_glfw.x11.display, &_glfw.x11.xshape.errorBase, &_glfw.x11.xshape.eventBase))
            if (XShapeQueryVersion(_glfw.x11.display, &_glfw.x11.xshape.major, &_glfw.x11.xshape.minor))
                _glfw.x11.xshape.available = GLFW_TRUE;
    }

    createKeyTables();

    #define IA(n) XInternAtom(_glfw.x11.display, n, False)
    _glfw.x11.NULL_              = IA("NULL");
    _glfw.x11.UTF8_STRING        = IA("UTF8_STRING");
    _glfw.x11.ATOM_PAIR          = IA("ATOM_PAIR");
    _glfw.x11.GLFW_SELECTION     = IA("GLFW_SELECTION");
    _glfw.x11.TARGETS            = IA("TARGETS");
    _glfw.x11.MULTIPLE           = IA("MULTIPLE");
    _glfw.x11.PRIMARY            = IA("PRIMARY");
    _glfw.x11.INCR               = IA("INCR");
    _glfw.x11.CLIPBOARD          = IA("CLIPBOARD");
    _glfw.x11.CLIPBOARD_MANAGER  = IA("CLIPBOARD_MANAGER");
    _glfw.x11.SAVE_TARGETS       = IA("SAVE_TARGETS");
    _glfw.x11.XdndAware          = IA("XdndAware");
    _glfw.x11.XdndEnter          = IA("XdndEnter");
    _glfw.x11.XdndPosition       = IA("XdndPosition");
    _glfw.x11.XdndStatus         = IA("XdndStatus");
    _glfw.x11.XdndActionCopy     = IA("XdndActionCopy");
    _glfw.x11.XdndDrop           = IA("XdndDrop");
    _glfw.x11.XdndFinished       = IA("XdndFinished");
    _glfw.x11.XdndSelection      = IA("XdndSelection");
    _glfw.x11.XdndTypeList       = IA("XdndTypeList");
    _glfw.x11.text_uri_list      = IA("text/uri-list");
    _glfw.x11.WM_PROTOCOLS       = IA("WM_PROTOCOLS");
    _glfw.x11.WM_STATE           = IA("WM_STATE");
    _glfw.x11.WM_DELETE_WINDOW   = IA("WM_DELETE_WINDOW");
    _glfw.x11.NET_SUPPORTED          = IA("_NET_SUPPORTED");
    _glfw.x11.NET_SUPPORTING_WM_CHECK= IA("_NET_SUPPORTING_WM_CHECK");
    _glfw.x11.NET_WM_ICON            = IA("_NET_WM_ICON");
    _glfw.x11.NET_WM_PING            = IA("_NET_WM_PING");
    _glfw.x11.NET_WM_PID             = IA("_NET_WM_PID");
    _glfw.x11.NET_WM_NAME            = IA("_NET_WM_NAME");
    _glfw.x11.NET_WM_ICON_NAME       = IA("_NET_WM_ICON_NAME");
    _glfw.x11.NET_WM_BYPASS_COMPOSITOR = IA("_NET_WM_BYPASS_COMPOSITOR");
    _glfw.x11.NET_WM_WINDOW_OPACITY  = IA("_NET_WM_WINDOW_OPACITY");
    _glfw.x11.MOTIF_WM_HINTS         = IA("_MOTIF_WM_HINTS");
    #undef IA
    { char name[32]; snprintf(name, sizeof(name), "_NET_WM_CM_S%u", _glfw.x11.screen);
      _glfw.x11.NET_WM_CM_Sx = XInternAtom(_glfw.x11.display, name, False); }

    detectEWMH();
    return GLFW_TRUE;
}

static void getSystemContentScale(float* xscale, float* yscale)
{
    float xdpi = 96.f, ydpi = 96.f;
    char* rms = XResourceManagerString(_glfw.x11.display);
    if (rms) {
        XrmDatabase db = XrmGetStringDatabase(rms);
        if (db) {
            XrmValue value; char* type = NULL;
            if (XrmGetResource(db, "Xft.dpi", "Xft.Dpi", &type, &value))
                if (type && strcmp(type, "String") == 0) xdpi = ydpi = atof(value.addr);
            XrmDestroyDatabase(db);
        }
    }
    *xscale = xdpi / 96.f; *yscale = ydpi / 96.f;
}

static Cursor createHiddenCursor(void)
    { unsigned char px[16*16*4] = {0}; GLFWimage img = {16,16,px}; return _glfwCreateNativeCursorX11(&img,0,0); }

static Window createHelperWindow(void)
{
    XSetWindowAttributes wa; wa.event_mask = PropertyChangeMask;
    return XCreateWindow(_glfw.x11.display, _glfw.x11.root, 0,0,1,1,0,0, InputOnly,
                         DefaultVisual(_glfw.x11.display, _glfw.x11.screen), CWEventMask, &wa);
}

static GLFWbool createEmptyEventPipe(void)
{
    if (pipe(_glfw.x11.emptyEventPipe) != 0)
        { _glfwInputError(GLFW_PLATFORM_ERROR, "X11: Failed to create empty event pipe: %s", strerror(errno)); return GLFW_FALSE; }
    for (int i = 0; i < 2; i++) {
        const int sf = fcntl(_glfw.x11.emptyEventPipe[i], F_GETFL, 0);
        const int df = fcntl(_glfw.x11.emptyEventPipe[i], F_GETFD, 0);
        if (sf == -1 || df == -1 ||
            fcntl(_glfw.x11.emptyEventPipe[i], F_SETFL, sf | O_NONBLOCK) == -1 ||
            fcntl(_glfw.x11.emptyEventPipe[i], F_SETFD, df | FD_CLOEXEC) == -1)
            { _glfwInputError(GLFW_PLATFORM_ERROR, "X11: Failed to set flags for empty event pipe: %s", strerror(errno)); return GLFW_FALSE; }
    }
    return GLFW_TRUE;
}

static int errorHandler(Display* display, XErrorEvent* event)
    { if (_glfw.x11.display == display) _glfw.x11.errorCode = event->error_code; return 0; }

void _glfwGrabErrorHandlerX11(void)
    { assert(_glfw.x11.errorHandler == NULL); _glfw.x11.errorCode = Success; _glfw.x11.errorHandler = XSetErrorHandler(errorHandler); }

void _glfwReleaseErrorHandlerX11(void)
    { XSync(_glfw.x11.display, False); XSetErrorHandler(_glfw.x11.errorHandler); _glfw.x11.errorHandler = NULL; }

void _glfwInputErrorX11(int error, const char* message)
{
    char buffer[_GLFW_MESSAGE_SIZE];
    XGetErrorText(_glfw.x11.display, _glfw.x11.errorCode, buffer, sizeof(buffer));
    _glfwInputError(error, "%s: %s", message, buffer);
}

Cursor _glfwCreateNativeCursorX11(const GLFWimage* image, int xhot, int yhot)
{
    if (!_glfw.x11.xcursor.handle) return None;
    XcursorImage* native = XcursorImageCreate(image->width, image->height);
    if (!native) return None;
    native->xhot = xhot; native->yhot = yhot;
    unsigned char* src = (unsigned char*)image->pixels;
    XcursorPixel* dst = native->pixels;
    for (int i = 0; i < image->width * image->height; i++, dst++, src += 4) {
        unsigned int a = src[3];
        *dst = (a<<24) | ((unsigned char)((src[0]*a)/255)<<16) | ((unsigned char)((src[1]*a)/255)<<8) | ((unsigned char)((src[2]*a)/255));
    }
    Cursor cursor = XcursorImageLoadCursor(_glfw.x11.display, native);
    XcursorImageDestroy(native);
    return cursor;
}

GLFWbool _glfwConnectX11(int platformID, _GLFWplatform* platform) {
    const _GLFWplatform x11 = {
        .platformID = GLFW_PLATFORM_X11,
        .init = _glfwInitX11,
        .getCursorPos = _glfwGetCursorPosX11, .setCursorPos = _glfwSetCursorPosX11,
        .setCursorMode = _glfwSetCursorModeX11, .setRawMouseMotion = _glfwSetRawMouseMotionX11,
        .rawMouseMotionSupported = _glfwRawMouseMotionSupportedX11,
        .createCursor = _glfwCreateCursorX11, .createStandardCursor = _glfwCreateStandardCursorX11,
        .destroyCursor = _glfwDestroyCursorX11, .setCursor = _glfwSetCursorX11,
        .getScancodeName = _glfwGetScancodeNameX11, .getKeyScancode = _glfwGetKeyScancodeX11,
        .setClipboardString = _glfwSetClipboardStringX11, .getClipboardString = _glfwGetClipboardStringX11,
#if defined(GLFW_BUILD_LINUX_JOYSTICK)
        .initJoysticks = _glfwInitJoysticksLinux,
        .pollJoystick = _glfwPollJoystickLinux, .getMappingName = _glfwGetMappingNameLinux,
        .updateGamepadGUID = _glfwUpdateGamepadGUIDLinux,
#else
        .initJoysticks = _glfwInitJoysticksNull,
        .pollJoystick = _glfwPollJoystickNull, .getMappingName = _glfwGetMappingNameNull,
        .updateGamepadGUID = _glfwUpdateGamepadGUIDNull,
#endif
        .freeMonitor = _glfwFreeMonitorX11, .getMonitorPos = _glfwGetMonitorPosX11,
        .getMonitorContentScale = _glfwGetMonitorContentScaleX11, .getMonitorWorkarea = _glfwGetMonitorWorkareaX11,
        .getVideoModes = _glfwGetVideoModesX11, .getVideoMode = _glfwGetVideoModeX11,
        .getGammaRamp = _glfwGetGammaRampX11, .setGammaRamp = _glfwSetGammaRampX11,
        .createWindow = _glfwCreateWindowX11,
        .setWindowTitle = _glfwSetWindowTitleX11, .setWindowIcon = _glfwSetWindowIconX11,
        .getWindowPos = _glfwGetWindowPosX11, .setWindowPos = _glfwSetWindowPosX11,
        .getWindowSize = _glfwGetWindowSizeX11, .setWindowSize = _glfwSetWindowSizeX11,
        .setWindowSizeLimits = _glfwSetWindowSizeLimitsX11, .setWindowAspectRatio = _glfwSetWindowAspectRatioX11,
        .getFramebufferSize = _glfwGetFramebufferSizeX11, .getWindowFrameSize = _glfwGetWindowFrameSizeX11,
        .getWindowContentScale = _glfwGetWindowContentScaleX11,
        .iconifyWindow = _glfwIconifyWindowX11, .restoreWindow = _glfwRestoreWindowX11,
        .maximizeWindow = _glfwMaximizeWindowX11, .showWindow = _glfwShowWindowX11,
        .hideWindow = _glfwHideWindowX11, .requestWindowAttention = _glfwRequestWindowAttentionX11,
        .focusWindow = _glfwFocusWindowX11, .setWindowMonitor = _glfwSetWindowMonitorX11,
        .windowFocused = _glfwWindowFocusedX11, .windowIconified = _glfwWindowIconifiedX11,
        .windowVisible = _glfwWindowVisibleX11, .windowMaximized = _glfwWindowMaximizedX11,
        .windowHovered = _glfwWindowHoveredX11, .framebufferTransparent = _glfwFramebufferTransparentX11,
        .getWindowOpacity = _glfwGetWindowOpacityX11, .setWindowResizable = _glfwSetWindowResizableX11,
        .setWindowDecorated = _glfwSetWindowDecoratedX11, .setWindowFloating = _glfwSetWindowFloatingX11,
        .setWindowOpacity = _glfwSetWindowOpacityX11, .setWindowMousePassthrough = _glfwSetWindowMousePassthroughX11,
        .pollEvents = _glfwPollEventsX11, .waitEvents = _glfwWaitEventsX11,
        .waitEventsTimeout = _glfwWaitEventsTimeoutX11, .postEmptyEvent = _glfwPostEmptyEventX11,
    };

    if (strcmp(setlocale(LC_CTYPE, NULL), "C") == 0) setlocale(LC_CTYPE, "");

    void* module = _glfwPlatformLoadModule("libX11.so.6");
    if (!module) {
        if (platformID == GLFW_PLATFORM_X11) _glfwInputError(GLFW_PLATFORM_ERROR, "X11: Failed to load Xlib");
        return GLFW_FALSE;
    }

    PFN_XInitThreads  XInitThreads  = (PFN_XInitThreads) _glfwPlatformGetModuleSymbol(module, "XInitThreads");
    PFN_XrmInitialize XrmInitialize = (PFN_XrmInitialize)_glfwPlatformGetModuleSymbol(module, "XrmInitialize");
    PFN_XOpenDisplay  XOpenDisplay  = (PFN_XOpenDisplay) _glfwPlatformGetModuleSymbol(module, "XOpenDisplay");
    if (!XInitThreads || !XrmInitialize || !XOpenDisplay) {
        if (platformID == GLFW_PLATFORM_X11) _glfwInputError(GLFW_PLATFORM_ERROR, "X11: Failed to load Xlib entry point");
        _glfwPlatformFreeModule(module); return GLFW_FALSE;
    }

    XInitThreads(); XrmInitialize();
    Display* display = XOpenDisplay(NULL);
    if (!display) {
        if (platformID == GLFW_PLATFORM_X11) {
            const char* name = getenv("DISPLAY");
            _glfwInputError(GLFW_PLATFORM_UNAVAILABLE, name ?
                "X11: Failed to open display %s" : "X11: The DISPLAY environment variable is missing", name);
        }
        _glfwPlatformFreeModule(module); return GLFW_FALSE;
    }

    _glfw.x11.display = display;
    _glfw.x11.xlib.handle = module;
    *platform = x11;
    return GLFW_TRUE;
}

int _glfwInitX11(void)
{
    #define SYM(f, n) _glfw.x11.xlib.f = (PFN_X##n)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "X"#n)
    SYM(AllocClassHint,AllocClassHint); SYM(AllocSizeHints,AllocSizeHints); SYM(AllocWMHints,AllocWMHints);
    SYM(ChangeProperty,ChangeProperty); SYM(ChangeWindowAttributes,ChangeWindowAttributes);
    SYM(CheckIfEvent,CheckIfEvent); SYM(CheckTypedWindowEvent,CheckTypedWindowEvent);
    SYM(CloseDisplay,CloseDisplay); SYM(CloseIM,CloseIM); SYM(ConvertSelection,ConvertSelection);
    SYM(CreateColormap,CreateColormap); SYM(CreateFontCursor,CreateFontCursor);
    SYM(CreateIC,CreateIC); SYM(CreateRegion,CreateRegion); SYM(CreateWindow,CreateWindow);
    SYM(DefineCursor,DefineCursor); SYM(DeleteContext,DeleteContext); SYM(DeleteProperty,DeleteProperty);
    SYM(DestroyIC,DestroyIC); SYM(DestroyRegion,DestroyRegion); SYM(DestroyWindow,DestroyWindow);
    SYM(DisplayKeycodes,DisplayKeycodes); SYM(EventsQueued,EventsQueued); SYM(FilterEvent,FilterEvent);
    SYM(FindContext,FindContext); SYM(Flush,Flush); SYM(Free,Free);
    SYM(FreeColormap,FreeColormap); SYM(FreeCursor,FreeCursor); SYM(FreeEventData,FreeEventData);
    SYM(GetErrorText,GetErrorText); SYM(GetEventData,GetEventData); SYM(GetICValues,GetICValues);
    SYM(GetIMValues,GetIMValues); SYM(GetInputFocus,GetInputFocus); SYM(GetKeyboardMapping,GetKeyboardMapping);
    SYM(GetScreenSaver,GetScreenSaver); SYM(GetSelectionOwner,GetSelectionOwner);
    SYM(GetVisualInfo,GetVisualInfo); SYM(GetWMNormalHints,GetWMNormalHints);
    SYM(GetWindowAttributes,GetWindowAttributes); SYM(GetWindowProperty,GetWindowProperty);
    SYM(GrabPointer,GrabPointer); SYM(IconifyWindow,IconifyWindow); SYM(InternAtom,InternAtom);
    SYM(LookupString,LookupString); SYM(MapRaised,MapRaised); SYM(MapWindow,MapWindow);
    SYM(MoveResizeWindow,MoveResizeWindow); SYM(MoveWindow,MoveWindow); SYM(NextEvent,NextEvent);
    SYM(OpenIM,OpenIM); SYM(PeekEvent,PeekEvent); SYM(Pending,Pending);
    SYM(QueryExtension,QueryExtension); SYM(QueryPointer,QueryPointer); SYM(RaiseWindow,RaiseWindow);
    SYM(RegisterIMInstantiateCallback,RegisterIMInstantiateCallback); SYM(ResizeWindow,ResizeWindow);
    SYM(ResourceManagerString,ResourceManagerString); SYM(SaveContext,SaveContext);
    SYM(SelectInput,SelectInput); SYM(SendEvent,SendEvent); SYM(SetClassHint,SetClassHint);
    SYM(SetErrorHandler,SetErrorHandler); SYM(SetICFocus,SetICFocus); SYM(SetIMValues,SetIMValues);
    SYM(SetInputFocus,SetInputFocus); SYM(SetLocaleModifiers,SetLocaleModifiers);
    SYM(SetScreenSaver,SetScreenSaver); SYM(SetSelectionOwner,SetSelectionOwner);
    SYM(SetWMHints,SetWMHints); SYM(SetWMNormalHints,SetWMNormalHints); SYM(SetWMProtocols,SetWMProtocols);
    SYM(SupportsLocale,SupportsLocale); SYM(Sync,Sync); SYM(TranslateCoordinates,TranslateCoordinates);
    SYM(UndefineCursor,UndefineCursor); SYM(UngrabPointer,UngrabPointer); SYM(UnmapWindow,UnmapWindow);
    SYM(UnsetICFocus,UnsetICFocus); SYM(VisualIDFromVisual,VisualIDFromVisual); SYM(WarpPointer,WarpPointer);
    #undef SYM
    #define KSYM(f, n) _glfw.x11.xkb.f = (PFN_Xkb##n)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "Xkb"#n)
    KSYM(FreeKeyboard,FreeKeyboard); KSYM(FreeNames,FreeNames); KSYM(GetMap,GetMap);
    KSYM(GetNames,GetNames); KSYM(GetState,GetState); KSYM(KeycodeToKeysym,KeycodeToKeysym);
    KSYM(QueryExtension,QueryExtension); KSYM(SelectEventDetails,SelectEventDetails);
    KSYM(SetDetectableAutoRepeat,SetDetectableAutoRepeat);
    #undef KSYM
    #define RSYM(f, n) _glfw.x11.xrm.f = (PFN_Xrm##n)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "Xrm"#n)
    RSYM(DestroyDatabase,DestroyDatabase); RSYM(GetResource,GetResource);
    RSYM(GetStringDatabase,GetStringDatabase); RSYM(UniqueQuark,UniqueQuark);
    #undef RSYM
    _glfw.x11.xlib.UnregisterIMInstantiateCallback = (PFN_XUnregisterIMInstantiateCallback)
        _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XUnregisterIMInstantiateCallback");
    _glfw.x11.xlib.utf8LookupString = (PFN_Xutf8LookupString)
        _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "Xutf8LookupString");
    _glfw.x11.xlib.utf8SetWMProperties = (PFN_Xutf8SetWMProperties)
        _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "Xutf8SetWMProperties");
    if (_glfw.x11.xlib.utf8LookupString && _glfw.x11.xlib.utf8SetWMProperties)
        _glfw.x11.xlib.utf8 = GLFW_TRUE;

    _glfw.x11.screen  = DefaultScreen(_glfw.x11.display);
    _glfw.x11.root    = RootWindow(_glfw.x11.display, _glfw.x11.screen);
    _glfw.x11.context = XUniqueContext();
    getSystemContentScale(&_glfw.x11.contentScaleX, &_glfw.x11.contentScaleY);
    if (!createEmptyEventPipe()) return GLFW_FALSE;
    if (!initExtensions())       return GLFW_FALSE;
    _glfw.x11.helperWindowHandle = createHelperWindow();
    _glfw.x11.hiddenCursorHandle = createHiddenCursor();
    if (XSupportsLocale() && _glfw.x11.xlib.utf8) {
        XSetLocaleModifiers("");
        XRegisterIMInstantiateCallback(_glfw.x11.display, NULL, NULL, NULL, inputMethodInstantiateCallback, NULL);
    }
    _glfwPollMonitorsX11();
    return GLFW_TRUE;
}

// void _glfwTerminateX11(void) {
//     if (_glfw.x11.helperWindowHandle) {
//         if (XGetSelectionOwner(_glfw.x11.display,_glfw.x11.CLIPBOARD) == _glfw.x11.helperWindowHandle) _glfwPushSelectionToManagerX11();
//         XDestroyWindow(_glfw.x11.display, _glfw.x11.helperWindowHandle);
//         _glfw.x11.helperWindowHandle = None;
//     }
//     if (_glfw.x11.hiddenCursorHandle)
//         { XFreeCursor(_glfw.x11.display, _glfw.x11.hiddenCursorHandle); _glfw.x11.hiddenCursorHandle = 0; }
//     _glfw_free(_glfw.x11.primarySelectionString);
//     _glfw_free(_glfw.x11.clipboardString);
//     XUnregisterIMInstantiateCallback(_glfw.x11.display, NULL, NULL, NULL, inputMethodInstantiateCallback, NULL);
//     if (_glfw.x11.im) { XCloseIM(_glfw.x11.im); _glfw.x11.im = NULL; }
//     if (_glfw.x11.display) { XCloseDisplay(_glfw.x11.display); _glfw.x11.display = NULL; }
// //     _glfwTerminateOSMesa();
// //     _glfwTerminateEGL();
//     _glfwTerminateGLX();
//     _glfwPlatformFreeModule(_glfw.x11.x11xcb.handle);
//     _glfwPlatformFreeModule(_glfw.x11.xcursor.handle);
//     _glfwPlatformFreeModule(_glfw.x11.randr.handle);
//     _glfwPlatformFreeModule(_glfw.x11.xinerama.handle);
//     _glfwPlatformFreeModule(_glfw.x11.xrender.handle);
//     _glfwPlatformFreeModule(_glfw.x11.xshape.handle);
//     _glfwPlatformFreeModule(_glfw.x11.vidmode.handle);
//     _glfwPlatformFreeModule(_glfw.x11.xi.handle);
//     _glfwPlatformFreeModule(_glfw.x11.xlib.handle);
//     if (_glfw.x11.emptyEventPipe[0] || _glfw.x11.emptyEventPipe[1])
//         { close(_glfw.x11.emptyEventPipe[0]); close(_glfw.x11.emptyEventPipe[1]); }
//     memset(&_glfw.x11, 0, sizeof(_glfw.x11));
// }

#endif // _GLFW_X11
