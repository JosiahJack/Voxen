// GLFW 3.5 This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
#pragma once
#include "gl.h"
#define GLFW_TRUE 1
#define GLFW_FALSE 0
typedef __SIZE_TYPE__ size_t;
typedef void* (* GLFWallocatefun)(size_t size, void* user);
typedef void* (* GLFWreallocatefun)(void* block, size_t size, void* user);
typedef void (* GLFWdeallocatefun)(void* block, void* user);
typedef struct GLFWallocator { GLFWallocatefun allocate; GLFWreallocatefun reallocate; GLFWdeallocatefun deallocate; void* user; } GLFWallocator;
#define _GLFW_INSERT_FIRST      0
#define _GLFW_INSERT_LAST       1
#define _GLFW_POLL_PRESENCE     0
#define _GLFW_POLL_AXES         1
#define _GLFW_POLL_BUTTONS      2
#define _GLFW_POLL_ALL          (_GLFW_POLL_AXES | _GLFW_POLL_BUTTONS)
#define _GLFW_MESSAGE_SIZE      1024
typedef int GLFWbool;
typedef void (*GLFWproc)(void);
typedef struct _GLFWerror       _GLFWerror;
typedef struct _GLFWinitconfig  _GLFWinitconfig;
typedef struct _GLFWwndconfig   _GLFWwndconfig;
typedef struct _GLFWctxconfig   _GLFWctxconfig;
typedef struct _GLFWfbconfig    _GLFWfbconfig;
typedef struct _GLFWcontext     _GLFWcontext;
typedef struct _GLFWwindow      _GLFWwindow;
typedef struct _GLFWplatform    _GLFWplatform;
typedef struct _GLFWlibrary     _GLFWlibrary;
typedef struct _GLFWmonitor     _GLFWmonitor;
typedef struct _GLFWcursor      _GLFWcursor;
typedef struct _GLFWmapelement  _GLFWmapelement;
typedef struct _GLFWmapping     _GLFWmapping;
typedef struct _GLFWjoystick    _GLFWjoystick;
typedef struct _GLFWtls         _GLFWtls;
typedef struct _GLFWmutex       _GLFWmutex;
#define GL_NONE 0
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_UNSIGNED_BYTE 0x1401
#define GL_CONTEXT_FLAGS 0x821e
#define GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT 0x00000001
#define GL_CONTEXT_FLAG_DEBUG_BIT 0x00000002
#define GL_CONTEXT_PROFILE_MASK 0x9126
#define GL_CONTEXT_COMPATIBILITY_PROFILE_BIT 0x00000002
#define GL_CONTEXT_CORE_PROFILE_BIT 0x00000001
#define GL_RESET_NOTIFICATION_STRATEGY_ARB 0x8256
#define GL_LOSE_CONTEXT_ON_RESET_ARB 0x8252
#define GL_NO_RESET_NOTIFICATION_ARB 0x8261
#define GL_CONTEXT_RELEASE_BEHAVIOR 0x82fb
#define GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH 0x82fc
#define GL_CONTEXT_FLAG_NO_ERROR_BIT_KHR 0x00000008
typedef int GLint;
typedef unsigned int GLuint;
typedef unsigned int GLenum;
typedef unsigned int GLbitfield;
typedef unsigned char GLubyte;
typedef void (APIENTRY * PFNGLCLEARPROC)(GLbitfield);
typedef const GLubyte* (APIENTRY * PFNGLGETSTRINGPROC)(GLenum);
typedef void (APIENTRY * PFNGLGETINTEGERVPROC)(GLenum,GLint*);
typedef const GLubyte* (APIENTRY * PFNGLGETSTRINGIPROC)(GLenum,GLuint);
#if defined(_GLFW_WIN32)
 #include "win32.h"
 #define GLFW_EXPOSE_NATIVE_WIN32
 #define GLFW_EXPOSE_NATIVE_WGL
#else
 #define GLFW_WIN32_WINDOW_STATE
 #define GLFW_WIN32_MONITOR_STATE
 #define GLFW_WIN32_CURSOR_STATE
 #define GLFW_WIN32_LIBRARY_WINDOW_STATE
 #define GLFW_WGL_CONTEXT_STATE
 #define GLFW_WGL_LIBRARY_CONTEXT_STATE
#endif

 #define GLFW_NSGL_CONTEXT_STATE
 #define GLFW_NSGL_LIBRARY_CONTEXT_STATE

#if defined(_GLFW_X11)
    #include "x11.h"
    #define GLFW_EXPOSE_NATIVE_X11
    #define GLFW_EXPOSE_NATIVE_GLX
    #include <linux/input.h>
    #include <linux/limits.h>
    #include <regex.h>
    #define GLFW_LINUX_JOYSTICK_STATE _GLFWjoystickLinux linjs;
    #define GLFW_LINUX_LIBRARY_JOYSTICK_STATE _GLFWlibraryLinux  linjs;
    typedef struct _GLFWjoystickLinux { int fd; char path[PATH_MAX]; int keyMap[KEY_CNT - BTN_MISC],absMap[ABS_CNT]; struct input_absinfo absInfo[ABS_CNT]; int hats[4][2]; } _GLFWjoystickLinux;
    typedef struct _GLFWlibraryLinux { int inotify,watch; regex_t regex; GLFWbool regexCompiled,dropped; } _GLFWlibraryLinux;
    void _glfwDetectJoystickConnectionLinux(void);
    GLFWbool _glfwInitJoysticksLinux(void);
    void _glfwTerminateJoysticksLinux(void);
    GLFWbool _glfwPollJoystickLinux(_GLFWjoystick* js, int mode);
    const char* _glfwGetMappingNameLinux(void);
    void _glfwUpdateGamepadGUIDLinux(char* guid);
#else
    #define GLFW_X11_WINDOW_STATE
    #define GLFW_X11_MONITOR_STATE
    #define GLFW_X11_CURSOR_STATE
    #define GLFW_X11_LIBRARY_WINDOW_STATE
    #define GLFW_GLX_CONTEXT_STATE
    #define GLFW_GLX_LIBRARY_CONTEXT_STATE
#endif

#if defined(_GLFW_WIN32)
    #define GLFW_WIN32_JOYSTICK_STATE _GLFWjoystickWin32 win32;
    #define GLFW_WIN32_LIBRARY_JOYSTICK_STATE
    typedef struct _GLFWjoyobjectWin32 { int offset,type; } _GLFWjoyobjectWin32;
    typedef struct _GLFWjoystickWin32{ _GLFWjoyobjectWin32* objects; int objectCount; IDirectInputDevice8W* device; DWORD index; GUID guid; } _GLFWjoystickWin32;
    void _glfwDetectJoystickConnectionWin32(void);
    void _glfwDetectJoystickDisconnectionWin32(void);
    #define GLFW_LINUX_JOYSTICK_STATE
    #define GLFW_LINUX_LIBRARY_JOYSTICK_STATE
#else
    #define GLFW_WIN32_JOYSTICK_STATE
    #define GLFW_WIN32_LIBRARY_JOYSTICK_STATE
#endif

#define GLFW_PLATFORM_WINDOW_STATE \
        GLFW_WIN32_WINDOW_STATE \
        GLFW_X11_WINDOW_STATE

#define GLFW_PLATFORM_MONITOR_STATE \
        GLFW_WIN32_MONITOR_STATE \
        GLFW_X11_MONITOR_STATE

#define GLFW_PLATFORM_CURSOR_STATE \
        GLFW_WIN32_CURSOR_STATE \
        GLFW_X11_CURSOR_STATE

#define GLFW_PLATFORM_LIBRARY_WINDOW_STATE \
        GLFW_WIN32_LIBRARY_WINDOW_STATE \
        GLFW_X11_LIBRARY_WINDOW_STATE

#define GLFW_PLATFORM_LIBRARY_JOYSTICK_STATE \
        GLFW_WIN32_LIBRARY_JOYSTICK_STATE \
        GLFW_LINUX_LIBRARY_JOYSTICK_STATE

#define GLFW_PLATFORM_CONTEXT_STATE \
        GLFW_WGL_CONTEXT_STATE \
        GLFW_NSGL_CONTEXT_STATE \
        GLFW_GLX_CONTEXT_STATE

#define GLFW_PLATFORM_LIBRARY_CONTEXT_STATE \
        GLFW_WGL_LIBRARY_CONTEXT_STATE \
        GLFW_NSGL_LIBRARY_CONTEXT_STATE \
        GLFW_GLX_LIBRARY_CONTEXT_STATE

#if defined(_WIN32)
 #define GLFW_BUILD_WIN32_THREAD
#else
 #define GLFW_BUILD_POSIX_THREAD
#endif

#if defined(GLFW_BUILD_WIN32_THREAD)
    #undef APIENTRY
    #include <windows.h>
    #define GLFW_WIN32_TLS_STATE            _GLFWtlsWin32     win32;
    #define GLFW_WIN32_MUTEX_STATE          _GLFWmutexWin32   win32;
    typedef struct _GLFWtlsWin32 { GLFWbool allocated; DWORD index; } _GLFWtlsWin32;
    typedef struct _GLFWmutexWin32 { GLFWbool allocated; CRITICAL_SECTION section; } _GLFWmutexWin32;
    #define GLFW_PLATFORM_TLS_STATE    GLFW_WIN32_TLS_STATE
    #define GLFW_PLATFORM_MUTEX_STATE  GLFW_WIN32_MUTEX_STATE
#elif defined(GLFW_BUILD_POSIX_THREAD)
    #include <pthread.h>
    #define GLFW_POSIX_TLS_STATE _GLFWtlsPOSIX   posix;
    #define GLFW_POSIX_MUTEX_STATE _GLFWmutexPOSIX posix;
    typedef struct _GLFWtlsPOSIX { GLFWbool allocated; pthread_key_t key; } _GLFWtlsPOSIX;
    typedef struct _GLFWmutexPOSIX { GLFWbool allocated; pthread_mutex_t handle; } _GLFWmutexPOSIX;
 #define GLFW_PLATFORM_TLS_STATE    GLFW_POSIX_TLS_STATE
 #define GLFW_PLATFORM_MUTEX_STATE  GLFW_POSIX_MUTEX_STATE
#endif

#if defined(_WIN32)
    #undef APIENTRY
    #include <windows.h>
    #define GLFW_WIN32_LIBRARY_TIMER_STATE  _GLFWtimerWin32   win32;
    typedef struct _GLFWtimerWin32 { u64 frequency; } _GLFWtimerWin32;
    #define GLFW_PLATFORM_LIBRARY_TIMER_STATE  GLFW_WIN32_LIBRARY_TIMER_STATE
#else
    #define GLFW_POSIX_LIBRARY_TIMER_STATE _GLFWtimerPOSIX posix;
    #include <stdint.h>
    #include <time.h>
    typedef struct _GLFWtimerPOSIX { clockid_t clock; u64 frequency; } _GLFWtimerPOSIX;
 #define GLFW_PLATFORM_LIBRARY_TIMER_STATE  GLFW_POSIX_LIBRARY_TIMER_STATE
#endif

#define _GLFW_SWAP(type, x, y) \
    {                          \
        type t;                \
        t = x;                 \
        x = y;                 \
        y = t;                 \
    }

struct _GLFWerror { _GLFWerror* next; i32 code; char description[_GLFW_MESSAGE_SIZE]; };
struct _GLFWinitconfig {
    GLFWbool hatButtons;
    i32  angleType,platformID;
    struct { GLFWbool menubar,chdir; } ns;
    struct { i32 libdecorMode; } wl;
};

struct _GLFWwndconfig {
    i32 xpos,ypos,width,height;
    GLFWbool resizable,visible,decorated,focused,autoIconify,floating,maximized,centerCursor,focusOnShow,mousePassthrough,scaleToMonitor,scaleFramebuffer;
    struct { char frameName[256]; } ns;
    struct { char className[256],instanceName[256]; } x11;
    struct { GLFWbool keymenu,showDefault; } win32;
    struct { char appId[256]; } wl;
};

struct _GLFWctxconfig {
    int           client;
    int           source;
    int           major;
    int           minor;
    GLFWbool      forward;
    GLFWbool      debug;
    GLFWbool      noerror;
    int           profile;
    int           robustness;
    int           release;
    _GLFWwindow*  share;
    struct { GLFWbool offline; } nsgl;
};

struct _GLFWfbconfig {
    int         redBits;
    int         greenBits;
    int         blueBits;
    int         alphaBits;
    int         depthBits;
    int         stencilBits;
    int         accumRedBits;
    int         accumGreenBits;
    int         accumBlueBits;
    int         accumAlphaBits;
    int         auxBuffers;
    GLFWbool    stereo;
    int         samples;
    GLFWbool    sRGB;
    GLFWbool    doublebuffer;
    GLFWbool    transparent;
    uintptr_t   handle;
};

struct _GLFWcontext {
    int                 client;
    int                 source;
    int                 major, minor, revision;
    GLFWbool            forward, debug, noerror;
    int                 profile;
    int                 robustness;
    int                 release;
    PFNGLGETSTRINGIPROC  GetStringi;
    PFNGLGETINTEGERVPROC GetIntegerv;
    PFNGLGETSTRINGPROC   GetString;
    void (*makeCurrent)(_GLFWwindow*);
    void (*swapBuffers)(_GLFWwindow*);
    void (*swapInterval)(int);
    int (*extensionSupported)(const char*);
    GLFWglproc (*getProcAddress)(const char*);
    void (*destroy)(_GLFWwindow*);
    GLFW_PLATFORM_CONTEXT_STATE
};

struct _GLFWwindow {
    struct _GLFWwindow* next;
    GLFWbool            resizable;
    GLFWbool            decorated;
    GLFWbool            autoIconify;
    GLFWbool            floating;
    GLFWbool            focusOnShow;
    GLFWbool            mousePassthrough;
    GLFWbool            shouldClose;
    void*               userPointer;
    GLFWbool            doublebuffer;
    GLFWvidmode         videoMode;
    _GLFWmonitor*       monitor;
    _GLFWcursor*        cursor;
    char*               title;
    int                 minwidth, minheight;
    int                 maxwidth, maxheight;
    int                 numer, denom;
    GLFWbool            stickyKeys;
    GLFWbool            stickyMouseButtons;
    GLFWbool            lockKeyMods;
    GLFWbool            disableMouseButtonLimit;
    int                 cursorMode;
    char                mouseButtons[GLFW_MOUSE_BUTTON_LAST + 1];
    char                keys[GLFW_KEY_LAST + 1];
    double              virtualCursorPosX, virtualCursorPosY;
    GLFWbool            rawMouseMotion;
    _GLFWcontext        context;
    struct {
        GLFWwindowposfun          pos;
        GLFWwindowsizefun         size;
        GLFWwindowclosefun        close;
        GLFWwindowrefreshfun      refresh;
        GLFWwindowfocusfun        focus;
        GLFWwindowiconifyfun      iconify;
        GLFWwindowmaximizefun     maximize;
        GLFWframebuffersizefun    fbsize;
        GLFWwindowcontentscalefun scale;
        GLFWmousebuttonfun        mouseButton;
        GLFWcursorposfun          cursorPos;
        GLFWcursorenterfun        cursorEnter;
        GLFWscrollfun             scroll;
        GLFWkeyfun                key;
        GLFWcharfun               character;
        GLFWcharmodsfun           charmods;
        GLFWdropfun               drop;
    } callbacks;
    GLFW_PLATFORM_WINDOW_STATE
};

struct _GLFWmonitor {
    char            name[128];
    void*           userPointer;
    int             widthMM, heightMM;
    _GLFWwindow*    window;
    GLFWvidmode*    modes;
    int             modeCount;
    GLFWvidmode     currentMode;
    GLFW_WIN32_MONITOR_STATE
    GLFW_X11_MONITOR_STATE
};

struct _GLFWcursor { _GLFWcursor* next; GLFW_PLATFORM_CURSOR_STATE };
struct _GLFWmapelement { u8 type,index; i8 axisScale,axisOffset; }; // Gamepad mapping element structure
struct _GLFWmapping { char name[128],guid[33]; _GLFWmapelement buttons[15],axes[6]; }; // Gamepad mapping structure
struct _GLFWjoystick {
    GLFWbool allocated,connected;
    float*  axes;
    int axisCount;
    unsigned char* buttons;
    int buttonCount;
    unsigned char* hats;
    int hatCount;
    char name[128];
    void* userPointer;
    char guid[33];
    _GLFWmapping* mapping;
    #if defined(_GLFW_X11)
        GLFW_LINUX_JOYSTICK_STATE
    #else
        GLFW_WIN32_JOYSTICK_STATE
    #endif
};

struct _GLFWtls { GLFW_PLATFORM_TLS_STATE };
struct _GLFWmutex { GLFW_PLATFORM_MUTEX_STATE };
struct _GLFWplatform {
    int platformID;
    void (*getCursorPos)(_GLFWwindow*,double*,double*);
    void (*setCursorPos)(_GLFWwindow*,double,double);
    void (*setCursorMode)(_GLFWwindow*,int);
    void (*setRawMouseMotion)(_GLFWwindow*,GLFWbool);
    GLFWbool (*rawMouseMotionSupported)(void);
    GLFWbool (*createCursor)(_GLFWcursor*,const GLFWimage*,int,int);
    GLFWbool (*createStandardCursor)(_GLFWcursor*,int);
    void (*destroyCursor)(_GLFWcursor*);
    void (*setCursor)(_GLFWwindow*,_GLFWcursor*);
    const char* (*getScancodeName)(int);
    int (*getKeyScancode)(int);
    GLFWbool (*initJoysticks)(void);
    GLFWbool (*pollJoystick)(_GLFWjoystick*,int);
    const char* (*getMappingName)(void);
    void (*updateGamepadGUID)(char*);
    void (*getMonitorPos)(_GLFWmonitor*,int*,int*);
    void (*getMonitorContentScale)(_GLFWmonitor*,float*,float*);
    void (*getMonitorWorkarea)(_GLFWmonitor*,int*,int*,int*,int*);
    GLFWvidmode* (*getVideoModes)(_GLFWmonitor*,int*);
    GLFWbool (*getVideoMode)(_GLFWmonitor*,GLFWvidmode*);
    GLFWbool (*createWindow)(_GLFWwindow*,const _GLFWwndconfig*,const _GLFWctxconfig*,const _GLFWfbconfig*);
    void (*destroyWindow)(_GLFWwindow*);
    void (*setWindowTitle)(_GLFWwindow*,const char*);
    void (*setWindowIcon)(_GLFWwindow*,int,const GLFWimage*);
    void (*getWindowPos)(_GLFWwindow*,int*,int*);
    void (*setWindowPos)(_GLFWwindow*,int,int);
    void (*getWindowSize)(_GLFWwindow*,int*,int*);
    void (*setWindowSize)(_GLFWwindow*,int,int);
    void (*getWindowFrameSize)(_GLFWwindow*,int*,int*,int*,int*);
    void (*iconifyWindow)(_GLFWwindow*);
    void (*restoreWindow)(_GLFWwindow*);
    void (*maximizeWindow)(_GLFWwindow*);
    void (*showWindow)(_GLFWwindow*);
    void (*hideWindow)(_GLFWwindow*);
    void (*requestWindowAttention)(_GLFWwindow*);
    void (*focusWindow)(_GLFWwindow*);
    void (*setWindowMonitor)(_GLFWwindow*,_GLFWmonitor*,int,int,int,int,int);
    GLFWbool (*windowFocused)(_GLFWwindow*);
    GLFWbool (*windowIconified)(_GLFWwindow*);
    GLFWbool (*windowVisible)(_GLFWwindow*);
    GLFWbool (*windowMaximized)(_GLFWwindow*);
    GLFWbool (*windowHovered)(_GLFWwindow*);
    GLFWbool (*framebufferTransparent)(_GLFWwindow*);
    float (*getWindowOpacity)(_GLFWwindow*);
    void (*setWindowResizable)(_GLFWwindow*,GLFWbool);
    void (*setWindowDecorated)(_GLFWwindow*,GLFWbool);
    void (*setWindowFloating)(_GLFWwindow*,GLFWbool);
    void (*setWindowOpacity)(_GLFWwindow*,float);
    void (*setWindowMousePassthrough)(_GLFWwindow*,GLFWbool);
    void (*pollEvents)(void);
    void (*waitEvents)(void);
    void (*waitEventsTimeout)(double);
    void (*postEmptyEvent)(void);
};

struct _GLFWlibrary {
    GLFWbool initialized;
    GLFWallocator allocator;
    _GLFWplatform platform;
    struct { _GLFWinitconfig init; _GLFWfbconfig framebuffer; _GLFWwndconfig window; _GLFWctxconfig context; int refreshRate; } hints;
    _GLFWerror* errorListHead;
    _GLFWcursor* cursorListHead;
    _GLFWwindow* windowListHead;
    _GLFWmonitor** monitors;
    int monitorCount;
    GLFWbool joysticksInitialized;
    _GLFWjoystick joysticks[GLFW_JOYSTICK_LAST + 1];
    _GLFWmapping* mappings;
    int mappingCount;
    _GLFWtls errorSlot,contextSlot;
    _GLFWmutex errorLock;
    struct { u64 offset; GLFW_PLATFORM_LIBRARY_TIMER_STATE } timer;
    struct { GLFWmonitorfun  monitor; GLFWjoystickfun joystick; } callbacks;
    GLFW_PLATFORM_LIBRARY_WINDOW_STATE
    GLFW_PLATFORM_LIBRARY_CONTEXT_STATE
    GLFW_PLATFORM_LIBRARY_JOYSTICK_STATE
};

extern _GLFWlibrary _glfw;
void _glfwPlatformInitTimer(void);
u64 _glfwPlatformGetTimerValue(void);
u64 _glfwPlatformGetTimerFrequency(void);
GLFWbool _glfwPlatformCreateTls(_GLFWtls* tls);
void _glfwPlatformDestroyTls(_GLFWtls* tls);
void* _glfwPlatformGetTls(_GLFWtls* tls);
void _glfwPlatformSetTls(_GLFWtls* tls, void* value);
GLFWbool _glfwPlatformCreateMutex(_GLFWmutex* mutex);
void _glfwPlatformDestroyMutex(_GLFWmutex* mutex);
void _glfwPlatformLockMutex(_GLFWmutex* mutex);
void _glfwPlatformUnlockMutex(_GLFWmutex* mutex);
void* _glfwPlatformLoadModule(const char* path);
void _glfwPlatformFreeModule(void* module);
GLFWproc _glfwPlatformGetModuleSymbol(void* module, const char* name);
void _glfwInputWindowFocus(_GLFWwindow* window, GLFWbool focused);
void _glfwInputWindowPos(_GLFWwindow* window, int xpos, int ypos);
void _glfwInputWindowSize(_GLFWwindow* window, int width, int height);
void _glfwInputFramebufferSize(_GLFWwindow* window, int width, int height);
void _glfwInputWindowContentScale(_GLFWwindow* window, float xscale, float yscale);
void _glfwInputWindowIconify(_GLFWwindow* window, GLFWbool iconified);
void _glfwInputWindowMaximize(_GLFWwindow* window, GLFWbool maximized);
void _glfwInputWindowDamage(_GLFWwindow* window);
void _glfwInputWindowCloseRequest(_GLFWwindow* window);
void _glfwInputWindowMonitor(_GLFWwindow* window, _GLFWmonitor* monitor);
void _glfwInputKey(_GLFWwindow* window, int key, int scancode, int action, int mods);
void _glfwInputChar(_GLFWwindow* window, u32 codepoint, int mods, GLFWbool plain);
void _glfwInputScroll(_GLFWwindow* window, double xoffset, double yoffset);
void _glfwInputMouseClick(_GLFWwindow* window, int button, int action, int mods);
void _glfwInputCursorPos(_GLFWwindow* window, double xpos, double ypos);
void _glfwInputCursorEnter(_GLFWwindow* window, GLFWbool entered);
void _glfwInputDrop(_GLFWwindow* window, int count, const char** names);
void _glfwInputJoystick(_GLFWjoystick* js, int event);
void _glfwInputJoystickAxis(_GLFWjoystick* js, int axis, float value);
void _glfwInputJoystickButton(_GLFWjoystick* js, int button, char value);
void _glfwInputJoystickHat(_GLFWjoystick* js, int hat, char value);
void _glfwInputMonitor(_GLFWmonitor* monitor, int action, int placement);
void _glfwInputMonitorWindow(_GLFWmonitor* monitor, _GLFWwindow* window);
#if defined(__GNUC__)
    void _glfwInputError(int code, const char* format, ...) __attribute__((format(printf, 2, 3)));
#else
    void _glfwInputError(int code, const char* format, ...);
#endif
GLFWbool _glfwStringInExtensionString(const char* string, const char* extensions);
const _GLFWfbconfig* _glfwChooseFBConfig(const _GLFWfbconfig* desired, const _GLFWfbconfig* alternatives, unsigned int count);
GLFWbool _glfwRefreshContextAttribs(_GLFWwindow* window, const _GLFWctxconfig* ctxconfig);
GLFWbool _glfwIsValidContextConfig(const _GLFWctxconfig* ctxconfig);
const GLFWvidmode* _glfwChooseVideoMode(_GLFWmonitor* monitor, const GLFWvidmode* desired);
int _glfwCompareVideoModes(const GLFWvidmode* first, const GLFWvidmode* second);
_GLFWmonitor* _glfwAllocMonitor(const char* name, int widthMM, int heightMM);
void _glfwAllocGammaArrays(GLFWgammaramp* ramp, unsigned int size);
void _glfwFreeGammaArrays(GLFWgammaramp* ramp);
void _glfwSplitBPP(int bpp, int* red, int* green, int* blue);
void _glfwInitGamepadMappings(void);
_GLFWjoystick* _glfwAllocJoystick(const char* name, const char* guid, int axisCount, int buttonCount, int hatCount);
void _glfwFreeJoystick(_GLFWjoystick* js);
void _glfwCenterCursorInContentArea(_GLFWwindow* window);
char** _glfwParseUriList(char* text, int* count);
char* _glfw_strdup(const char* source);
#include "mappings.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#define DBL_MAX 1.7976931348623158e+308
#define _GLFW_STICK 3
#define _GLFW_JOYSTICK_AXIS 1
#define _GLFW_JOYSTICK_BUTTON 2
#define _GLFW_JOYSTICK_HATBIT 3
#define GLFW_MOD_MASK (GLFW_MOD_SHIFT|GLFW_MOD_CONTROL|GLFW_MOD_ALT|GLFW_MOD_SUPER|GLFW_MOD_CAPS_LOCK|GLFW_MOD_NUM_LOCK)
void* _glfw_calloc(size_t count, size_t size) {
    if (count && size) {
        void* block;
        if (count > SIZE_MAX / size) { _glfwInputError(GLFW_INVALID_VALUE, "Allocation size overflow"); return NULL; }

        block = malloc(count * size);
        if (block) return memset(block, 0, count * size);
        else { _glfwInputError(GLFW_OUT_OF_MEMORY, NULL); return NULL; }
    } else return NULL;
}

void* _glfw_realloc(void* block, size_t size) {
    if (block && size) {
        void* resized = realloc(block, size);
        if (resized) return resized;
        else {
            _glfwInputError(GLFW_OUT_OF_MEMORY, NULL);
            return NULL;
        }
    } else if (block) { free(block); return NULL;
    } else return calloc(1,size);
}

#if defined(_GLFW_WIN32)
    #include "win32.c"
    #define PLATFORM_getCursorPos(w,x,y)            _glfwGetCursorPosWin32(w,x,y)
    #define PLATFORM_setCursorPos(w,x,y)            _glfwSetCursorPosWin32(w,x,y)
    #define PLATFORM_setCursorMode(w,m)             _glfwSetCursorModeWin32(w,m)
    #define PLATFORM_rawMouseMotionSupported()      _glfwRawMouseMotionSupportedWin32()
    #define PLATFORM_setRawMouseMotion(w,e)         _glfwSetRawMouseMotionWin32(w,e)
    #define PLATFORM_initJoysticks()                _glfwInitJoysticksWin32()
    #define PLATFORM_pollJoystick(js,m)             _glfwPollJoystickWin32(js,m)
    #define PLATFORM_getMappingName()               _glfwGetMappingNameWin32()
    #define PLATFORM_updateGamepadGUID(g)           _glfwUpdateGamepadGUIDWin32(g)
    #define PLATFORM_getMonitorPos(m,x,y)           _glfwGetMonitorPosWin32(m,x,y)
    #define PLATFORM_getMonitorWorkarea(m,x,y,w,h)  _glfwGetMonitorWorkareaWin32(m,x,y,w,h)
    #define PLATFORM_getVideoModes(m,c)             _glfwGetVideoModesWin32(m,c)
    #define PLATFORM_getVideoMode(m,cur)            _glfwGetVideoModeWin32(m,cur)
    #define PLATFORM_createWindow(w,wc,cc,fc)       _glfwCreateWindowWin32(w,wc,cc,fc)
    #define PLATFORM_setWindowTitle(w,t)            _glfwSetWindowTitleWin32(w,t)
    #define PLATFORM_setWindowIcon(w,c,i)           _glfwSetWindowIconWin32(w,c,i)
    #define PLATFORM_getWindowPos(w,x,y)            _glfwGetWindowPosWin32(w,x,y)
    #define PLATFORM_setWindowPos(w,x,y)            _glfwSetWindowPosWin32(w,x,y)
    #define PLATFORM_getWindowSize(w,wi,h)          _glfwGetWindowSizeWin32(w,wi,h)
    #define PLATFORM_setWindowSize(w,wi,h)          _glfwSetWindowSizeWin32(w,wi,h)
    #define PLATFORM_getWindowFrameSize(w,l,t,r,b)  _glfwGetWindowFrameSizeWin32(w,l,t,r,b)
    #define PLATFORM_setWindowMonitor(w,m,x,y,wi,h,r) _glfwSetWindowMonitorWin32(w,m,x,y,wi,h,r)
    #define PLATFORM_setWindowDecorated(w,v)        _glfwSetWindowDecoratedWin32(w,v)
    #define PLATFORM_getKeyScancode(k)              _glfwGetKeyScancodeWin32(k)
    #define PLATFORM_pollEvents()                   _glfwPollEventsWin32()
#else
    #include "x11.c"
    #define PLATFORM_getCursorPos(w,x,y)            _glfwGetCursorPosX11(w,x,y)
    #define PLATFORM_setCursorPos(w,x,y)            _glfwSetCursorPosX11(w,x,y)
    #define PLATFORM_setCursorMode(w,m)             _glfwSetCursorModeX11(w,m)
    #define PLATFORM_rawMouseMotionSupported()      _glfwRawMouseMotionSupportedX11()
    #define PLATFORM_setRawMouseMotion(w,e)         _glfwSetRawMouseMotionX11(w,e)
    #define PLATFORM_initJoysticks()                _glfwInitJoysticksLinux()
    #define PLATFORM_pollJoystick(js,m)             _glfwPollJoystickLinux(js,m)
    #define PLATFORM_getMappingName()               _glfwGetMappingNameLinux()
    #define PLATFORM_updateGamepadGUID(g)           _glfwUpdateGamepadGUIDLinux(g)
    #define PLATFORM_getMonitorPos(m,x,y)           _glfwGetMonitorPosX11(m,x,y)
    #define PLATFORM_getMonitorWorkarea(m,x,y,w,h)  _glfwGetMonitorWorkareaX11(m,x,y,w,h)
    #define PLATFORM_getVideoModes(m,c)             _glfwGetVideoModesX11(m,c)
    #define PLATFORM_getVideoMode(m,cur)            _glfwGetVideoModeX11(m,cur)
    #define PLATFORM_createWindow(w,wc,cc,fc)       _glfwCreateWindowX11(w,wc,cc,fc)
    #define PLATFORM_setWindowTitle(w,t)            _glfwSetWindowTitleX11(w,t)
    #define PLATFORM_setWindowIcon(w,c,i)           _glfwSetWindowIconX11(w,c,i)
    #define PLATFORM_getWindowPos(w,x,y)            _glfwGetWindowPosX11(w,x,y)
    #define PLATFORM_setWindowPos(w,x,y)            _glfwSetWindowPosX11(w,x,y)
    #define PLATFORM_getWindowSize(w,wi,h)          _glfwGetWindowSizeX11(w,wi,h)
    #define PLATFORM_setWindowSize(w,wi,h)          _glfwSetWindowSizeX11(w,wi,h)
    #define PLATFORM_getWindowFrameSize(w,l,t,r,b)  _glfwGetWindowFrameSizeX11(w,l,t,r,b)
    #define PLATFORM_setWindowMonitor(w,m,x,y,wi,h,r) _glfwSetWindowMonitorX11(w,m,x,y,wi,h,r)
    #define PLATFORM_setWindowDecorated(w,v)        _glfwSetWindowDecoratedX11(w,v)
    #define PLATFORM_getKeyScancode(k)              _glfwGetKeyScancodeX11(k)
    #define PLATFORM_pollEvents()                   _glfwPollEventsX11()
#endif

_GLFWlibrary _glfw = { GLFW_FALSE };
static _GLFWerror _glfwMainThreadError; static GLFWerrorfun _glfwErrorCallback; static GLFWallocator _glfwInitAllocator;
typedef __builtin_va_list va_list;
static _GLFWinitconfig _glfwInitHints = { .hatButtons = GLFW_TRUE, .platformID = GLFW_ANY_PLATFORM, .ns = { .menubar = GLFW_TRUE, .chdir = GLFW_TRUE } };

char** _glfwParseUriList(char* text, int* count) {
    const char* prefix = "file://";
    char** paths = NULL; char* line; *count = 0;
    while ((line = strtok(text, "\r\n"))) {
        char* path;
        text = NULL;
        if (line[0] == '#') continue;

        if (strncmp(line, prefix, strlen(prefix)) == 0) {
            line += strlen(prefix);
            while (*line != '/') line++;
        }

        (*count)++;
        path = calloc(strlen(line) + 1,1);
        paths = paths ? calloc(*count,sizeof(char*)) : realloc(paths,*count * sizeof(char*));
        paths[*count - 1] = path;
        while (*line) {
            if (line[0] == '%' && line[1] && line[2]) {
                const char digits[3] = { line[1], line[2], '\0' };
                *path = (char) strtol(digits, NULL, 16);
                line += 2;
            } else *path = *line;

            path++;
            line++;
        }
    }

    return paths;
}

char* _glfw_strdup(const char* source) {
    const size_t length = strlen(source);
    char* result = calloc(length + 1,1);
    strcpy(result, source);
    return result;
}

void _glfwInputError(int code, const char* format, ...) {
    _GLFWerror* error;
    char description[_GLFW_MESSAGE_SIZE];
    if (format) {
        va_list vl; __builtin_va_start(vl,format); vsnprintf(description, sizeof(description),format,vl); __builtin_va_end(vl);
        description[sizeof(description) - 1] = '\0';
    } else {
        if (code == GLFW_NOT_INITIALIZED) strcpy(description, "The GLFW library is not initialized");
        else if (code == GLFW_NO_CURRENT_CONTEXT) strcpy(description, "There is no current context");
        else if (code == GLFW_INVALID_ENUM) strcpy(description, "Invalid argument for enum parameter");
        else if (code == GLFW_INVALID_VALUE) strcpy(description, "Invalid value for parameter");
        else if (code == GLFW_OUT_OF_MEMORY) strcpy(description, "Out of memory");
        else if (code == GLFW_API_UNAVAILABLE) strcpy(description, "The requested API is unavailable");
        else if (code == GLFW_VERSION_UNAVAILABLE) strcpy(description, "The requested API version is unavailable");
        else if (code == GLFW_PLATFORM_ERROR) strcpy(description, "A platform-specific error occurred");
        else if (code == GLFW_FORMAT_UNAVAILABLE) strcpy(description, "The requested format is unavailable");
        else if (code == GLFW_NO_WINDOW_CONTEXT) strcpy(description, "The specified window has no context");
        else if (code == GLFW_CURSOR_UNAVAILABLE) strcpy(description, "The specified cursor shape is unavailable");
        else if (code == GLFW_FEATURE_UNAVAILABLE) strcpy(description, "The requested feature cannot be implemented for this platform");
        else if (code == GLFW_FEATURE_UNIMPLEMENTED) strcpy(description, "The requested feature has not yet been implemented for this platform");
        else if (code == GLFW_PLATFORM_UNAVAILABLE) strcpy(description, "The requested platform is unavailable");
        else strcpy(description, "ERROR: UNKNOWN GLFW ERROR");
    }

    if (_glfw.initialized) {
        error = _glfwPlatformGetTls(&_glfw.errorSlot);
        if (!error) {
            error = calloc(1,sizeof(_GLFWerror));
            _glfwPlatformSetTls(&_glfw.errorSlot, error);
            _glfwPlatformLockMutex(&_glfw.errorLock);
            error->next = _glfw.errorListHead;
            _glfw.errorListHead = error;
            _glfwPlatformUnlockMutex(&_glfw.errorLock);
        }
    } else error = &_glfwMainThreadError;

    error->code = code;
    strcpy(error->description, description);
    if (_glfwErrorCallback) _glfwErrorCallback(code, description);
}

GLFWAPI int glfwInit(void) {
    memset(&_glfw,0,sizeof(_glfw)); _glfw.hints.init = _glfwInitHints; _glfw.allocator = _glfwInitAllocator;
#if defined(_GLFW_WIN32)
    if (!_glfwInitWin32()) return GLFW_FALSE;
#else
    if (!_glfwInitX11()) return GLFW_FALSE;
#endif
    if (!_glfwPlatformCreateMutex(&_glfw.errorLock) || !_glfwPlatformCreateTls(&_glfw.errorSlot) || !_glfwPlatformCreateTls(&_glfw.contextSlot)) return GLFW_FALSE;

    _glfwPlatformSetTls(&_glfw.errorSlot, &_glfwMainThreadError);
    _glfwInitGamepadMappings();
    _glfwPlatformInitTimer();
    _glfw.timer.offset = _glfwPlatformGetTimerValue();
    _glfw.initialized = GLFW_TRUE;
    memset(&_glfw.hints.context, 0, sizeof(_glfw.hints.context));
    _glfw.hints.context.client = GLFW_OPENGL_API;
    _glfw.hints.context.source = GLFW_NATIVE_CONTEXT_API;
    _glfw.hints.context.major = 4; _glfw.hints.context.minor = 3;
    _glfw.hints.context.profile = GLFW_OPENGL_CORE_PROFILE;
    memset(&_glfw.hints.window,0,sizeof(_glfw.hints.window));
    _glfw.hints.window.resizable = _glfw.hints.window.visible = _glfw.hints.window.decorated = _glfw.hints.window.focused = _glfw.hints.window.autoIconify = _glfw.hints.window.centerCursor = _glfw.hints.window.focusOnShow = GLFW_TRUE;
    _glfw.hints.window.xpos = _glfw.hints.window.ypos = GLFW_ANY_POSITION;
    _glfw.hints.window.scaleFramebuffer = GLFW_TRUE;
    memset(&_glfw.hints.framebuffer,0,sizeof(_glfw.hints.framebuffer));
    _glfw.hints.framebuffer.redBits = _glfw.hints.framebuffer.greenBits = _glfw.hints.framebuffer.blueBits = _glfw.hints.framebuffer.alphaBits = _glfw.hints.framebuffer.stencilBits = 8;
    _glfw.hints.framebuffer.depthBits = 24;
    _glfw.hints.framebuffer.doublebuffer = GLFW_TRUE;
    _glfw.hints.refreshRate = GLFW_DONT_CARE;
    return GLFW_TRUE;
}

GLFWbool _glfwIsValidContextConfig(const _GLFWctxconfig* c) {
    if (c->source != GLFW_NATIVE_CONTEXT_API) { _glfwInputError(GLFW_INVALID_ENUM, "Invalid context creation API 0x%08X", c->source); return GLFW_FALSE; }
    if (c->client != GLFW_OPENGL_API) { _glfwInputError(GLFW_INVALID_ENUM, "Invalid client API 0x%08X", c->client); return GLFW_FALSE; }
    if (c->share) {
        if (c->source != c->share->context.source) { _glfwInputError(GLFW_INVALID_ENUM, "Context creation APIs do not match between contexts"); return GLFW_FALSE; }
    }

    if (c->profile) {
        if (c->profile != GLFW_OPENGL_CORE_PROFILE && c->profile != GLFW_OPENGL_COMPAT_PROFILE) { _glfwInputError(GLFW_INVALID_ENUM, "Invalid OpenGL profile 0x%08X", c->profile); return GLFW_FALSE; }
    }
    if (c->robustness && c->robustness != GLFW_NO_RESET_NOTIFICATION && c->robustness != GLFW_LOSE_CONTEXT_ON_RESET) { _glfwInputError(GLFW_INVALID_ENUM, "Invalid context robustness mode 0x%08X", c->robustness); return GLFW_FALSE; }
    if (c->release && c->release != GLFW_RELEASE_BEHAVIOR_NONE && c->release != GLFW_RELEASE_BEHAVIOR_FLUSH) { _glfwInputError(GLFW_INVALID_ENUM, "Invalid context release behavior 0x%08X", c->release); return GLFW_FALSE; }
    return GLFW_TRUE;
}

const _GLFWfbconfig* _glfwChooseFBConfig(const _GLFWfbconfig* desired, const _GLFWfbconfig* alts, unsigned int count) {
    unsigned int missing, leastMissing = 2147483647, colorDiff, leastColorDiff = 2147483647, extraDiff, leastExtraDiff = 2147483647;
    const _GLFWfbconfig* closest = NULL;
    for (unsigned int i = 0; i < count; i++) {
        const _GLFWfbconfig* cur = alts + i;
        if (desired->stereo > 0 && cur->stereo == 0) continue;
        missing = 0;
        if (desired->alphaBits   > 0 && cur->alphaBits == 0)   missing++;
        if (desired->depthBits   > 0 && cur->depthBits == 0)   missing++;
        if (desired->stencilBits > 0 && cur->stencilBits == 0) missing++;
        if (desired->auxBuffers  > 0 && cur->auxBuffers < desired->auxBuffers) missing += desired->auxBuffers - cur->auxBuffers;
        if (desired->samples     > 0 && cur->samples == 0)     missing++;
        if (desired->transparent != cur->transparent)          missing++;
        colorDiff = 0;
        if (desired->redBits   != GLFW_DONT_CARE) colorDiff += (desired->redBits   - cur->redBits)   * (desired->redBits   - cur->redBits);
        if (desired->greenBits != GLFW_DONT_CARE) colorDiff += (desired->greenBits - cur->greenBits) * (desired->greenBits - cur->greenBits);
        if (desired->blueBits  != GLFW_DONT_CARE) colorDiff += (desired->blueBits  - cur->blueBits)  * (desired->blueBits  - cur->blueBits);
        extraDiff = 0;
        if (desired->alphaBits      != GLFW_DONT_CARE) extraDiff += (desired->alphaBits      - cur->alphaBits)      * (desired->alphaBits      - cur->alphaBits);
        if (desired->depthBits      != GLFW_DONT_CARE) extraDiff += (desired->depthBits      - cur->depthBits)      * (desired->depthBits      - cur->depthBits);
        if (desired->stencilBits    != GLFW_DONT_CARE) extraDiff += (desired->stencilBits    - cur->stencilBits)    * (desired->stencilBits    - cur->stencilBits);
        if (desired->accumRedBits   != GLFW_DONT_CARE) extraDiff += (desired->accumRedBits   - cur->accumRedBits)   * (desired->accumRedBits   - cur->accumRedBits);
        if (desired->accumGreenBits != GLFW_DONT_CARE) extraDiff += (desired->accumGreenBits - cur->accumGreenBits) * (desired->accumGreenBits - cur->accumGreenBits);
        if (desired->accumBlueBits  != GLFW_DONT_CARE) extraDiff += (desired->accumBlueBits  - cur->accumBlueBits)  * (desired->accumBlueBits  - cur->accumBlueBits);
        if (desired->accumAlphaBits != GLFW_DONT_CARE) extraDiff += (desired->accumAlphaBits - cur->accumAlphaBits) * (desired->accumAlphaBits - cur->accumAlphaBits);
        if (desired->samples        != GLFW_DONT_CARE) extraDiff += (desired->samples        - cur->samples)        * (desired->samples        - cur->samples);
        if (desired->sRGB && !cur->sRGB) extraDiff++;
        if (missing < leastMissing || (missing == leastMissing && (colorDiff < leastColorDiff || (colorDiff == leastColorDiff && extraDiff < leastExtraDiff))))
            closest = cur;
        if (cur == closest) { leastMissing = missing; leastColorDiff = colorDiff; leastExtraDiff = extraDiff; }
    }
    return closest;
}

GLFWAPI int glfwExtensionSupported(const char* extension) {
    _GLFWwindow* window = _glfwPlatformGetTls(&_glfw.contextSlot);
    if (!window) { _glfwInputError(GLFW_NO_CURRENT_CONTEXT, "Cannot query extension without a current OpenGL or OpenGL ES context"); return GLFW_FALSE; }
    if (*extension == '\0') { _glfwInputError(GLFW_INVALID_VALUE, "Extension name cannot be an empty string"); return GLFW_FALSE; }

    if (window->context.major >= 3) {
        GLint count; window->context.GetIntegerv(GL_NUM_EXTENSIONS, &count);
        for (int i = 0; i < count; i++) {
            const char* en = (const char*) window->context.GetStringi(GL_EXTENSIONS, i);
            if (!en) { _glfwInputError(GLFW_PLATFORM_ERROR, "Extension string retrieval is broken"); return GLFW_FALSE; }
            if (strcmp(en, extension) == 0) return GLFW_TRUE;
        }
    } else {
        const char* extensions = (const char*) window->context.GetString(GL_EXTENSIONS);
        if (!extensions) { _glfwInputError(GLFW_PLATFORM_ERROR, "Extension string retrieval is broken"); return GLFW_FALSE; }
        if (_glfwStringInExtensionString(extension, extensions)) return GLFW_TRUE;
    }
    return window->context.extensionSupported(extension);
}

int sscanf(const char *str, const char *format, ...);
GLFWbool _glfwRefreshContextAttribs(_GLFWwindow* window, const _GLFWctxconfig* ctxconfig) {
    const char* prefixes[] = { "OpenGL ES-CM ", "OpenGL ES-CL ", "OpenGL ES ", NULL };
    window->context.source = ctxconfig->source;
    window->context.client = GLFW_OPENGL_API;
    _GLFWwindow* previous = _glfwPlatformGetTls(&_glfw.contextSlot);
    glfwMakeContextCurrent((GLFWwindow*) window);
    if (_glfwPlatformGetTls(&_glfw.contextSlot) != window) return GLFW_FALSE;

    window->context.GetIntegerv = (PFNGLGETINTEGERVPROC) window->context.getProcAddress("glGetIntegerv");
    window->context.GetString   = (PFNGLGETSTRINGPROC)   window->context.getProcAddress("glGetString");
    if (!window->context.GetIntegerv || !window->context.GetString) { _glfwInputError(GLFW_PLATFORM_ERROR, "Entry point retrieval is broken"); glfwMakeContextCurrent((GLFWwindow*) previous); return GLFW_FALSE; }

    const char* version = (const char*) window->context.GetString(GL_VERSION);
    if (!version) { _glfwInputError(GLFW_PLATFORM_ERROR, "OpenGL version string retrieval is broken"); glfwMakeContextCurrent((GLFWwindow*) previous); return GLFW_FALSE; }

    for (int i = 0; prefixes[i]; i++) {
        const size_t len = strlen(prefixes[i]);
        if (strncmp(version, prefixes[i], len) == 0) { version += len; window->context.client = GLFW_OPENGL_ES_API; break; }
    }

    if (!sscanf(version, "%d.%d.%d", &window->context.major, &window->context.minor, &window->context.revision)) { _glfwInputError(GLFW_PLATFORM_ERROR, "No version found in OpenGL version string"); glfwMakeContextCurrent((GLFWwindow*) previous); return GLFW_FALSE; }

    if (window->context.major < ctxconfig->major || (window->context.major == ctxconfig->major && window->context.minor < ctxconfig->minor)) { _glfwInputError(GLFW_VERSION_UNAVAILABLE, "Requested OpenGL version %i.%i, got version %i.%i", ctxconfig->major, ctxconfig->minor, window->context.major, window->context.minor); glfwMakeContextCurrent((GLFWwindow*) previous); return GLFW_FALSE; }

    if (window->context.major >= 3) {
        window->context.GetStringi = (PFNGLGETSTRINGIPROC) window->context.getProcAddress("glGetStringi");
        if (!window->context.GetStringi) { _glfwInputError(GLFW_PLATFORM_ERROR, "Entry point retrieval is broken"); glfwMakeContextCurrent((GLFWwindow*) previous); return GLFW_FALSE; }
        GLint flags; window->context.GetIntegerv(GL_CONTEXT_FLAGS, &flags);
        if (flags & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT) window->context.forward = GLFW_TRUE;
        if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)              window->context.debug   = GLFW_TRUE;
        else if (glfwExtensionSupported("GL_ARB_debug_output") && ctxconfig->debug) window->context.debug = GLFW_TRUE;
        if (flags & GL_CONTEXT_FLAG_NO_ERROR_BIT_KHR)      window->context.noerror = GLFW_TRUE;
    }
    if (window->context.major >= 4 || (window->context.major == 3 && window->context.minor >= 2)) {
        GLint mask; window->context.GetIntegerv(GL_CONTEXT_PROFILE_MASK, &mask);
        if      (mask & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT) window->context.profile = GLFW_OPENGL_COMPAT_PROFILE;
        else if (mask & GL_CONTEXT_CORE_PROFILE_BIT)          window->context.profile = GLFW_OPENGL_CORE_PROFILE;
        else if (glfwExtensionSupported("GL_ARB_compatibility")) window->context.profile = GLFW_OPENGL_COMPAT_PROFILE;
    }
    if (glfwExtensionSupported("GL_ARB_robustness")) {
        GLint strategy; window->context.GetIntegerv(GL_RESET_NOTIFICATION_STRATEGY_ARB, &strategy);
        if      (strategy == GL_LOSE_CONTEXT_ON_RESET_ARB)   window->context.robustness = GLFW_LOSE_CONTEXT_ON_RESET;
        else if (strategy == GL_NO_RESET_NOTIFICATION_ARB)   window->context.robustness = GLFW_NO_RESET_NOTIFICATION;
    }
    if (glfwExtensionSupported("GL_KHR_context_flush_control")) {
        GLint behavior; window->context.GetIntegerv(GL_CONTEXT_RELEASE_BEHAVIOR, &behavior);
        if      (behavior == GL_NONE)                            window->context.release = GLFW_RELEASE_BEHAVIOR_NONE;
        else if (behavior == GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH)  window->context.release = GLFW_RELEASE_BEHAVIOR_FLUSH;
    }

    PFNGLCLEARPROC glClear = (PFNGLCLEARPROC) window->context.getProcAddress("glClear");
    glClear(GL_COLOR_BUFFER_BIT);
    if (window->doublebuffer) window->context.swapBuffers(window);
    glfwMakeContextCurrent((GLFWwindow*) previous);
    return GLFW_TRUE;
}

GLFWbool _glfwStringInExtensionString(const char* string, const char* extensions) {
    const char* start = extensions;
    for (;;) {
        const char* where = strstr(start, string);
        if (!where) return GLFW_FALSE;
        const char* terminator = where + strlen(string);
        if ((where == start || *(where - 1) == ' ') && (*terminator == ' ' || *terminator == '\0')) break;
        start = terminator;
    }
    return GLFW_TRUE;
}

GLFWAPI void glfwMakeContextCurrent(GLFWwindow* handle) {
    _GLFWwindow* window = (_GLFWwindow*) handle;
    _GLFWwindow* previous = _glfwPlatformGetTls(&_glfw.contextSlot);
    if (window && window->context.client == GLFW_NO_API)
        { _glfwInputError(GLFW_NO_WINDOW_CONTEXT, "Cannot make current with a window that has no OpenGL or OpenGL ES context"); return; }
    if (previous && (!window || window->context.source != previous->context.source)) previous->context.makeCurrent(NULL);
    if (window) window->context.makeCurrent(window);
}

GLFWAPI void glfwSwapBuffers(GLFWwindow* handle) {
    _GLFWwindow* window = (_GLFWwindow*) handle;
    window->context.swapBuffers(window);
}

GLFWAPI void glfwSwapInterval(int interval) {
    _GLFWwindow* window = _glfwPlatformGetTls(&_glfw.contextSlot);
    if (!window) { _glfwInputError(GLFW_NO_CURRENT_CONTEXT, "Cannot set swap interval without a current OpenGL or OpenGL ES context"); return; }
    window->context.swapInterval(interval);
}

GLFWAPI GLFWglproc glfwGetProcAddress(const char* procname) {
    _GLFWwindow* window = _glfwPlatformGetTls(&_glfw.contextSlot);
    if (!window) { _glfwInputError(GLFW_NO_CURRENT_CONTEXT, "Cannot query entry point without a current OpenGL or OpenGL ES context"); return NULL; }
    return window->context.getProcAddress(procname);
}

static int compareVideoModes(const void* fp, const void* sp) {
    const GLFWvidmode *fm=fp, *sm=sp;
    const int fbpp=fm->redBits + fm->greenBits + fm->blueBits, sbpp=sm->redBits + sm->greenBits + sm->blueBits;
    const int farea=fm->width * fm->height, sarea=sm->width * sm->height;
    if (fbpp != sbpp) return fbpp - sbpp;
    if (farea != sarea) return farea - sarea;
    if (fm->width != sm->width) return fm->width - sm->width;
    return fm->refreshRate - sm->refreshRate;
}

static GLFWbool refreshVideoModes(_GLFWmonitor* monitor) {
    int modeCount; GLFWvidmode* modes;
    if (monitor->modes) return GLFW_TRUE;

    modes = PLATFORM_getVideoModes(monitor, &modeCount);
    if (!modes) return GLFW_FALSE;

    qsort(modes, modeCount, sizeof(GLFWvidmode), compareVideoModes);
    free(monitor->modes);
    monitor->modes = modes;
    monitor->modeCount = modeCount;
    return GLFW_TRUE;
}

void _glfwInputMonitor(_GLFWmonitor* monitor, int action, int placement) {
    if (action == GLFW_CONNECTED) {
        _glfw.monitorCount++;
        _glfw.monitors = _glfw.monitors ? realloc(_glfw.monitors,sizeof(_GLFWmonitor*) * _glfw.monitorCount) : calloc(_glfw.monitorCount,sizeof(_GLFWmonitor*));
        if (placement == _GLFW_INSERT_FIRST) {
            memmove(_glfw.monitors + 1,_glfw.monitors,((size_t) _glfw.monitorCount - 1) * sizeof(_GLFWmonitor*));
            _glfw.monitors[0] = monitor;
        } else _glfw.monitors[_glfw.monitorCount - 1] = monitor;
    } else if (action == GLFW_DISCONNECTED) {
        int i;
        _GLFWwindow* window;
        for (window = _glfw.windowListHead;  window;  window = window->next) {
            if (window->monitor == monitor) {
                int width, height, xoff, yoff;
                PLATFORM_getWindowSize(window, &width, &height);
                PLATFORM_setWindowMonitor(window, NULL, 0, 0, width, height, 0);
                PLATFORM_getWindowFrameSize(window, &xoff, &yoff, NULL, NULL);
                PLATFORM_setWindowPos(window, xoff, yoff);
            }
        }

        for (i = 0;  i < _glfw.monitorCount;  i++) {
            if (_glfw.monitors[i] == monitor) {
                _glfw.monitorCount--;
                memmove(_glfw.monitors + i, _glfw.monitors + i + 1,((size_t) _glfw.monitorCount - i) * sizeof(_GLFWmonitor*));
                break;
            }
        }
    }

    if (_glfw.callbacks.monitor) _glfw.callbacks.monitor((GLFWmonitor*) monitor, action);
}

void _glfwInputMonitorWindow(_GLFWmonitor* monitor, _GLFWwindow* window) { monitor->window = window; }

_GLFWmonitor* _glfwAllocMonitor(const char* name, int widthMM, int heightMM) {
    _GLFWmonitor* monitor = calloc(1, sizeof(_GLFWmonitor));
    monitor->widthMM = widthMM; monitor->heightMM = heightMM;
    strncpy(monitor->name, name, sizeof(monitor->name) - 1);
    return monitor;
}

void _glfwAllocGammaArrays(GLFWgammaramp* ramp, unsigned int size) {
    ramp->red = calloc(size,sizeof(unsigned short));
    ramp->green = calloc(size,sizeof(unsigned short));
    ramp->blue = calloc(size,sizeof(unsigned short));
    ramp->size = size;
}

void _glfwFreeGammaArrays(GLFWgammaramp* ramp) { free(ramp->red); free(ramp->green); free(ramp->blue); memset(ramp,0,sizeof(GLFWgammaramp)); }

const GLFWvidmode* _glfwChooseVideoMode(_GLFWmonitor* monitor, const GLFWvidmode* desired) {
    int i; unsigned int sizeDiff,leastSizeDiff=0xffffffffU,rateDiff,leastRateDiff=0xffffffffU,colorDiff,leastColorDiff=0xffffffffU; const GLFWvidmode*current, *closest=NULL;
    if (!refreshVideoModes(monitor)) return NULL;

    for (i = 0;  i < monitor->modeCount;  i++) {
        current = monitor->modes + i;
        colorDiff = 0;
        if (desired->redBits != GLFW_DONT_CARE) colorDiff += abs(current->redBits - desired->redBits);
        if (desired->greenBits != GLFW_DONT_CARE) colorDiff += abs(current->greenBits - desired->greenBits);
        if (desired->blueBits != GLFW_DONT_CARE) colorDiff += abs(current->blueBits - desired->blueBits);
        sizeDiff = abs((current->width - desired->width) * (current->width - desired->width) + (current->height - desired->height) * (current->height - desired->height));
        if (desired->refreshRate != GLFW_DONT_CARE) rateDiff = abs(current->refreshRate - desired->refreshRate);
        else rateDiff = 0xffffffffU - current->refreshRate;

        if ((colorDiff < leastColorDiff) || (colorDiff == leastColorDiff && sizeDiff < leastSizeDiff) || (colorDiff == leastColorDiff && sizeDiff == leastSizeDiff && rateDiff < leastRateDiff)) {
            closest = current;
            leastSizeDiff = sizeDiff;
            leastRateDiff = rateDiff;
            leastColorDiff = colorDiff;
        }
    }

    return closest;
}

int _glfwCompareVideoModes(const GLFWvidmode* fm, const GLFWvidmode* sm) { return compareVideoModes(fm, sm); }

void _glfwSplitBPP(int bpp, int* red, int* green, int* blue) {
    int delta;
    if (bpp == 32) bpp = 24;
    *red = *green = *blue = bpp / 3;
    delta = bpp - (*red * 3);
    if (delta >= 1) *green = *green + 1;
    if (delta == 2) *red = *red + 1;
}

GLFWAPI GLFWmonitor** glfwGetMonitors(int* count) { *count = 0; *count = _glfw.monitorCount; return (GLFWmonitor**) _glfw.monitors; }
GLFWAPI GLFWmonitor* glfwGetPrimaryMonitor(void) { if (!_glfw.monitorCount) {return NULL;} return (GLFWmonitor*) _glfw.monitors[0]; }
GLFWAPI void glfwGetMonitorPos(GLFWmonitor* handle, int* xpos, int* ypos) { if (xpos) {*xpos = 0;} if (ypos) {*ypos = 0;} _GLFWmonitor* monitor = (_GLFWmonitor*)handle; PLATFORM_getMonitorPos(monitor,xpos,ypos); }
GLFWAPI void glfwGetMonitorWorkarea(GLFWmonitor* handle, int* xpos, int* ypos, int* width, int* height) {
    if (xpos) *xpos = 0;
    if (ypos) *ypos = 0;
    if (width) *width = 0;
    if (height) *height = 0;
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;
    PLATFORM_getMonitorWorkarea(monitor, xpos, ypos, width, height);
}

GLFWAPI const GLFWvidmode* glfwGetVideoModes(GLFWmonitor* handle, int* count) {
    *count = 0; _GLFWmonitor* monitor = (_GLFWmonitor*) handle;
    if (!refreshVideoModes(monitor)) return NULL;
    *count = monitor->modeCount;
    return monitor->modes;
}

GLFWAPI const GLFWvidmode* glfwGetVideoMode(GLFWmonitor* handle) {
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;
    if (!PLATFORM_getVideoMode(monitor, &monitor->currentMode)) return NULL;
    return &monitor->currentMode;
}

void _glfwInputWindowFocus(_GLFWwindow* window, GLFWbool focused) {
    if (window->callbacks.focus) window->callbacks.focus((GLFWwindow*)window,focused);
    if (!focused) {
        for (int key = 0;  key <= GLFW_KEY_LAST;  key++) {
            if (window->keys[key] == GLFW_PRESS) {
                const int scancode = PLATFORM_getKeyScancode(key);
                _glfwInputKey(window,key,scancode,GLFW_RELEASE,0);
            }
        }

        for (int button = 0;  button <= GLFW_MOUSE_BUTTON_LAST;  button++) {
            if (window->mouseButtons[button] == GLFW_PRESS)  _glfwInputMouseClick(window, button, GLFW_RELEASE, 0);
        }
    }
}

void _glfwInputWindowPos(_GLFWwindow* window, int x, int y) { if (window->callbacks.pos) {window->callbacks.pos((GLFWwindow*)window,x,y);} }
void _glfwInputWindowSize(_GLFWwindow* window, int width, int height) { if (window->callbacks.size) {window->callbacks.size((GLFWwindow*)window,width,height);} }
void _glfwInputWindowIconify(_GLFWwindow* window, GLFWbool iconified) { if (window->callbacks.iconify) {window->callbacks.iconify((GLFWwindow*)window,iconified);} }
void _glfwInputWindowMaximize(_GLFWwindow* window, GLFWbool maximized) { if (window->callbacks.maximize) {window->callbacks.maximize((GLFWwindow*)window,maximized);} }
void _glfwInputFramebufferSize(_GLFWwindow* window, int width, int height) { if (window->callbacks.fbsize) window->callbacks.fbsize((GLFWwindow*)window,width,height); }
void _glfwInputWindowContentScale(_GLFWwindow* window, float xscale, float yscale) { if (window->callbacks.scale) {window->callbacks.scale((GLFWwindow*)window,xscale,yscale);} }
void _glfwInputWindowDamage(_GLFWwindow* window) { if (window->callbacks.refresh) {window->callbacks.refresh((GLFWwindow*) window);} }
void _glfwInputWindowCloseRequest(_GLFWwindow* window) { window->shouldClose = GLFW_TRUE; if (window->callbacks.close) {window->callbacks.close((GLFWwindow*) window);} }
void _glfwInputWindowMonitor(_GLFWwindow* window, _GLFWmonitor* monitor) { window->monitor = monitor; }

GLFWAPI GLFWwindow* glfwCreateWindow(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share) {
    _GLFWfbconfig fbconfig; _GLFWctxconfig ctxconfig; _GLFWwndconfig wndconfig; _GLFWwindow* window;
    fbconfig  = _glfw.hints.framebuffer;
    ctxconfig = _glfw.hints.context;
    wndconfig = _glfw.hints.window;
    wndconfig.width = width; wndconfig.height= height;
    ctxconfig.share = (_GLFWwindow*) share;
    if (!_glfwIsValidContextConfig(&ctxconfig)) return NULL;

    window = calloc(1,sizeof(_GLFWwindow));
    window->next = _glfw.windowListHead;
    _glfw.windowListHead = window;
    window->videoMode.width       = width; window->videoMode.height      = height;
    window->videoMode.redBits     = fbconfig.redBits; window->videoMode.greenBits   = fbconfig.greenBits; window->videoMode.blueBits    = fbconfig.blueBits;
    window->videoMode.refreshRate = _glfw.hints.refreshRate;
    window->monitor          = (_GLFWmonitor*) monitor;
    window->resizable        = wndconfig.resizable;
    window->decorated        = wndconfig.decorated;
    window->autoIconify      = wndconfig.autoIconify;
    window->floating         = wndconfig.floating;
    window->focusOnShow      = wndconfig.focusOnShow;
    window->mousePassthrough = wndconfig.mousePassthrough;
    window->cursorMode       = GLFW_CURSOR_NORMAL;
    window->doublebuffer = fbconfig.doublebuffer;
    window->minwidth = window->minheight = window->maxwidth = window->maxheight = window->numer = window->denom = GLFW_DONT_CARE;
    window->title = _glfw_strdup(title);
    DualLog("glfwCreateWindow 0\n");
    if (!PLATFORM_createWindow(window, &wndconfig, &ctxconfig, &fbconfig)) { return NULL; }
    DualLog("glfwCreateWindow 1\n");
    return (GLFWwindow*) window;
}

GLFWAPI int glfwWindowShouldClose(GLFWwindow* handle) { _GLFWwindow* window = (_GLFWwindow*) handle; return window->shouldClose; }

GLFWAPI void glfwSetWindowTitle(GLFWwindow* handle, const char* title) {
    _GLFWwindow* window=(_GLFWwindow*)handle;
    char* prev=window->title; window->title=_glfw_strdup(title);
    PLATFORM_setWindowTitle(window,title);
    free(prev);
}

GLFWAPI void glfwSetWindowIcon(GLFWwindow* handle, int count, const GLFWimage* images) {
    _GLFWwindow* window = (_GLFWwindow*) handle;
    PLATFORM_setWindowIcon(window,count,images);
}

GLFWAPI void glfwGetWindowPos(GLFWwindow* handle, int* xpos, int* ypos) {
    if (xpos) {*xpos = 0;} if (ypos) {*ypos = 0;}
    _GLFWwindow* window = (_GLFWwindow*) handle;
    PLATFORM_getWindowPos(window, xpos, ypos);
}

GLFWAPI void glfwSetWindowPos(GLFWwindow* handle, int xpos, int ypos) {
    _GLFWwindow* window = (_GLFWwindow*) handle;
    if (window->monitor) return;
    PLATFORM_setWindowPos(window, xpos, ypos);
}

GLFWAPI void glfwGetWindowSize(GLFWwindow* handle, int* width, int* height) {
    if (width) *width = 0;
    if (height) *height = 0;
    _GLFWwindow* window = (_GLFWwindow*) handle;
    PLATFORM_getWindowSize(window, width, height);
}

GLFWAPI void glfwSetWindowSize(GLFWwindow* handle, int width, int height) { _GLFWwindow* window = (_GLFWwindow*)handle; window->videoMode.width=width; window->videoMode.height=height; PLATFORM_setWindowSize(window,width,height); }

GLFWAPI void glfwSetWindowAttrib(GLFWwindow* handle, int attrib, int value) {
    _GLFWwindow* window = (_GLFWwindow*) handle;
    value = value ? GLFW_TRUE : GLFW_FALSE;
    switch (attrib) {
        case 0x00020005/*GLFW_DECORATED*/: window->decorated = value; if (!window->monitor) { PLATFORM_setWindowDecorated(window,value); } return;
    }
    _glfwInputError(GLFW_INVALID_ENUM, "Invalid window attribute 0x%08X", attrib);
}

GLFWAPI void glfwSetWindowMonitor(GLFWwindow* wh, GLFWmonitor* mh, int xpos, int ypos, int width, int height, int refreshRate) {
    _GLFWwindow* window = (_GLFWwindow*) wh;
    _GLFWmonitor* monitor = (_GLFWmonitor*) mh;
    if (width <= 0 || height <= 0) { _glfwInputError(GLFW_INVALID_VALUE,"Invalid window size %ix%i",width,height); return; }
    if (refreshRate < 0 && refreshRate != GLFW_DONT_CARE) { _glfwInputError(GLFW_INVALID_VALUE,"Invalid refresh rate %i",refreshRate); return; }

    window->videoMode.width=width; window->videoMode.height=height; window->videoMode.refreshRate=refreshRate;
    PLATFORM_setWindowMonitor(window,monitor,xpos,ypos,width,height,refreshRate);
}

GLFWAPI GLFWwindowfocusfun glfwSetWindowFocusCallback(GLFWwindow* handle, GLFWwindowfocusfun cbfun) { _GLFWwindow* window = (_GLFWwindow*)handle; _GLFW_SWAP(GLFWwindowfocusfun,window->callbacks.focus,cbfun); return cbfun; }
GLFWAPI GLFWframebuffersizefun glfwSetFramebufferSizeCallback(GLFWwindow* handle, GLFWframebuffersizefun cbfun) { _GLFWwindow* window = (_GLFWwindow*) handle; _GLFW_SWAP(GLFWframebuffersizefun,window->callbacks.fbsize,cbfun); return cbfun; }

//=============================================================================
// Input
GLFWAPI void glfwPollEvents(void) { PLATFORM_pollEvents(); }

static GLFWbool initJoysticks(void) {
    if (!_glfw.joysticksInitialized && !PLATFORM_initJoysticks()) return GLFW_FALSE;
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
    __builtin_memcpy(mapping->guid,c,length); c += length + 1;
    if ((length = strcspn(c,",")) >= sizeof(mapping->name) || c[length] != ',') return _glfwInputError(GLFW_INVALID_VALUE,NULL),GLFW_FALSE;
    __builtin_memcpy(mapping->name,c,length); c += length + 1;
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
                const char* name = PLATFORM_getMappingName();
                if (strncmp(c,name,strlen(name)) != 0) return GLFW_FALSE;
            }
            break;
        }
        c += strcspn(c,","); c += strspn(c,",");
    }
    for (i = 0; i < 32; i++) { if (mapping->guid[i] >= 'A' && mapping->guid[i] <= 'F') mapping->guid[i] += 'a'-'A'; }
    PLATFORM_updateGamepadGUID(mapping->guid);
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
    _glfw.mappings = calloc(count,sizeof(_GLFWmapping));
    for (size_t i = 0; i < count; i++) { if (parseMapping(&_glfw.mappings[_glfw.mappingCount],_glfwDefaultMappings[i])) _glfw.mappingCount++; }
}

_GLFWjoystick* _glfwAllocJoystick(const char* name,const char* guid,int axisCount,int buttonCount,int hatCount) {
    int jid; _GLFWjoystick* js;
    for (jid = 0; jid <= GLFW_JOYSTICK_LAST; jid++) { if (!_glfw.joysticks[jid].allocated) break; }
    if (jid > GLFW_JOYSTICK_LAST) return NULL;
    js = _glfw.joysticks + jid;
    js->allocated = GLFW_TRUE; js->axisCount = axisCount; js->buttonCount = buttonCount; js->hatCount = hatCount;
    js->axes = calloc(axisCount,sizeof(float));
    js->buttons = calloc(buttonCount + (size_t)hatCount * 4,1);
    js->hats = calloc(hatCount,1);
    strncpy(js->name,name,sizeof(js->name)-1); strncpy(js->guid,guid,sizeof(js->guid)-1);
    js->mapping = findValidMapping(js);
    return js;
}

void _glfwFreeJoystick(_GLFWjoystick* js) { free(js->axes); free(js->buttons); free(js->hats); memset(js,0,sizeof(_GLFWjoystick)); }

void _glfwCenterCursorInContentArea(_GLFWwindow* window) {
    int width,height;
    PLATFORM_getWindowSize(window,&width,&height);
    PLATFORM_setCursorPos(window,width/2.0,height/2.0);
}

GLFWAPI void glfwSetInputMode(GLFWwindow* handle,int mode,int value) {
    _GLFWwindow* window = (_GLFWwindow*)handle;
    switch (mode) {
        case GLFW_CURSOR:
            if (value != GLFW_CURSOR_NORMAL && value != GLFW_CURSOR_HIDDEN && value != GLFW_CURSOR_DISABLED && value != GLFW_CURSOR_CAPTURED) { _glfwInputError(GLFW_INVALID_ENUM,"Invalid cursor mode 0x%08X",value); return; }
            if (window->cursorMode == value) return;
            window->cursorMode = value;
            PLATFORM_getCursorPos(window,&window->virtualCursorPosX,&window->virtualCursorPosY);
            PLATFORM_setCursorMode(window,value); return;
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
            if (!PLATFORM_rawMouseMotionSupported()) { _glfwInputError(GLFW_PLATFORM_ERROR,"Raw mouse motion is not supported on this system"); return; }
            value = value ? GLFW_TRUE : GLFW_FALSE;
            if (window->rawMouseMotion == value) return;
            window->rawMouseMotion = value; PLATFORM_setRawMouseMotion(window,value); return;
        case GLFW_UNLIMITED_MOUSE_BUTTONS: window->disableMouseButtonLimit = value ? GLFW_TRUE : GLFW_FALSE; return;
    }
    _glfwInputError(GLFW_INVALID_ENUM,"Invalid input mode 0x%08X",mode);
}

GLFWAPI GLFWkeyfun glfwSetKeyCallback(GLFWwindow* handle,GLFWkeyfun cbfun) { _GLFWwindow* window = (_GLFWwindow*)handle; _GLFW_SWAP(GLFWkeyfun,window->callbacks.key,cbfun); return cbfun; }
GLFWAPI GLFWmousebuttonfun glfwSetMouseButtonCallback(GLFWwindow* handle,GLFWmousebuttonfun cbfun) { _GLFWwindow* window = (_GLFWwindow*)handle; _GLFW_SWAP(GLFWmousebuttonfun,window->callbacks.mouseButton,cbfun); return cbfun; }
GLFWAPI GLFWcursorposfun glfwSetCursorPosCallback(GLFWwindow* handle,GLFWcursorposfun cbfun) { _GLFWwindow* window = (_GLFWwindow*)handle; _GLFW_SWAP(GLFWcursorposfun,window->callbacks.cursorPos,cbfun); return cbfun; }
GLFWAPI GLFWscrollfun glfwSetScrollCallback(GLFWwindow* handle,GLFWscrollfun cbfun) { _GLFWwindow* window = (_GLFWwindow*)handle; _GLFW_SWAP(GLFWscrollfun,window->callbacks.scroll,cbfun); return cbfun; }

GLFWAPI int glfwJoystickPresent(int jid) {
    if (jid < 0 || jid > GLFW_JOYSTICK_LAST) return _glfwInputError(GLFW_INVALID_ENUM,"Invalid joystick ID %i",jid),GLFW_FALSE;
    if (!initJoysticks()) return GLFW_FALSE;
    _GLFWjoystick* js = _glfw.joysticks + jid;
    return js->connected ? PLATFORM_pollJoystick(js,_GLFW_POLL_PRESENCE) : GLFW_FALSE;
}

GLFWAPI const unsigned char* glfwGetJoystickButtons(int jid,int* count) {
    *count = 0; if (jid < 0 || jid > GLFW_JOYSTICK_LAST) return _glfwInputError(GLFW_INVALID_ENUM,"Invalid joystick ID %i",jid),NULL;
    if (!initJoysticks()) return NULL;
    _GLFWjoystick* js = _glfw.joysticks + jid; if (!js->connected || !PLATFORM_pollJoystick(js,_GLFW_POLL_BUTTONS)) return NULL;
    *count = _glfw.hints.init.hatButtons ? js->buttonCount + js->hatCount * 4 : js->buttonCount; return js->buttons;
}

GLFWAPI const unsigned char* glfwGetJoystickHats(int jid,int* count) {
    *count = 0; if (jid < 0 || jid > GLFW_JOYSTICK_LAST) return _glfwInputError(GLFW_INVALID_ENUM,"Invalid joystick ID %i",jid),NULL;
    if (!initJoysticks()) return NULL;
    _GLFWjoystick* js = _glfw.joysticks + jid; if (!js->connected || !PLATFORM_pollJoystick(js,_GLFW_POLL_BUTTONS)) return NULL;
    return *count = js->hatCount, js->hats;
}

GLFWAPI GLFWjoystickfun glfwSetJoystickCallback(GLFWjoystickfun cbfun) { if (!initJoysticks()) {return NULL;} _GLFW_SWAP(GLFWjoystickfun,_glfw.callbacks.joystick,cbfun); return cbfun; }

GLFWAPI int glfwGetGamepadState(int jid,GLFWgamepadstate* state) {
    memset(state,0,sizeof(GLFWgamepadstate));
    if (jid < 0 || jid > GLFW_JOYSTICK_LAST) return _glfwInputError(GLFW_INVALID_ENUM,"Invalid joystick ID %i",jid),GLFW_FALSE;
    if (!initJoysticks()) return GLFW_FALSE;
    _GLFWjoystick* js = _glfw.joysticks + jid;
    if (!js->connected || !PLATFORM_pollJoystick(js,_GLFW_POLL_ALL) || !js->mapping) return GLFW_FALSE;
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
        if (e->type == _GLFW_JOYSTICK_AXIS) state->axes[i] = vmin(vmax(js->axes[e->index] * e->axisScale + e->axisOffset,-1.f),1.f);
        else if (e->type == _GLFW_JOYSTICK_HATBIT) state->axes[i] = (js->hats[e->index >> 4] & (e->index & 0xf)) ? 1.f : -1.f;
        else if (e->type == _GLFW_JOYSTICK_BUTTON) state->axes[i] = js->buttons[e->index] * 2.f - 1.f;
    }
    return GLFW_TRUE;
}
