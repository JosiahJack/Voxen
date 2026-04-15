// GLFW 3.5 This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
#define GLFW_TRUE 1
#define GLFW_FALSE 0
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
#if defined(WINDOWS)
    #define NOMINMAX
    #define VC_EXTRALEAN
    #undef APIENTRY
    #define UNICODE
    #if WINVER < 0x0601 // GLFW requires Windows 7 or later
        #undef WINVER
        #define WINVER 0x0601
    #endif
    #if _WIN32_WINNT < 0x0601
        #undef _WIN32_WINNT
        #define _WIN32_WINNT 0x0601
    #endif
    #define OEMRESOURCE // GLFW uses OEM cursor resources
    #ifndef far
        #define far
    #endif
    #ifndef near
        #define near
    #endif
    #include <dwmapi.h>
    #include <dinput.h>
    #include <xinput.h>
    #include <dbt.h>
    #define OCR_NORMAL 32512
    #ifndef DPI_ENUMS_DECLARED
        typedef enum { PROCESS_DPI_UNAWARE = 0, PROCESS_SYSTEM_DPI_AWARE = 1, PROCESS_PER_MONITOR_DPI_AWARE = 2 } PROCESS_DPI_AWARENESS;
        typedef enum { MDT_EFFECTIVE_DPI = 0, MDT_ANGULAR_DPI = 1, MDT_RAW_DPI = 2, MDT_DEFAULT = MDT_EFFECTIVE_DPI } MONITOR_DPI_TYPE;
    #endif /*DPI_ENUMS_DECLARED*/

    #ifndef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
        #define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((HANDLE) -4)
    #endif /*DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2*/

    #define IsWindows8OrGreater()                              \
        _glfwIsWindowsVersionOrGreaterWin32(HIBYTE(0x0602),    \
                                            LOBYTE(0x0602), 0)
    #define IsWindows8Point1OrGreater()                     \
        _glfwIsWindowsVersionOrGreaterWin32(HIBYTE(0x0603), \
                                            LOBYTE(0x0603), 0)
    // Windows 10 Anniversary Update
    #define _glfwIsWindows10Version1607OrGreaterWin32() \
        _glfwIsWindows10BuildOrGreaterWin32(14393)

    // Windows 10 Creators Update
    #define _glfwIsWindows10Version1703OrGreaterWin32() \
        _glfwIsWindows10BuildOrGreaterWin32(15063)

    #ifndef XINPUT_CAPS_WIRELESS
    #define XINPUT_CAPS_WIRELESS 0x0002
    #endif
    #ifndef DIDFT_OPTIONAL
    #define DIDFT_OPTIONAL 0x80000000 // HACK: Define macros that some dinput.h variants don't
    #endif
    #define WGL_NUMBER_PIXEL_FORMATS_ARB 0x2000
    #define WGL_SUPPORT_OPENGL_ARB 0x2010
    #define WGL_DRAW_TO_WINDOW_ARB 0x2001
    #define WGL_PIXEL_TYPE_ARB 0x2013
    #define WGL_TYPE_RGBA_ARB 0x202b
    #define WGL_ACCELERATION_ARB 0x2003
    #define WGL_NO_ACCELERATION_ARB 0x2025
    #define WGL_RED_BITS_ARB 0x2015
    #define WGL_RED_SHIFT_ARB 0x2016
    #define WGL_GREEN_BITS_ARB 0x2017
    #define WGL_GREEN_SHIFT_ARB 0x2018
    #define WGL_BLUE_BITS_ARB 0x2019
    #define WGL_BLUE_SHIFT_ARB 0x201a
    #define WGL_ALPHA_BITS_ARB 0x201b
    #define WGL_ALPHA_SHIFT_ARB 0x201c
    #define WGL_ACCUM_BITS_ARB 0x201d
    #define WGL_ACCUM_RED_BITS_ARB 0x201e
    #define WGL_ACCUM_GREEN_BITS_ARB 0x201f
    #define WGL_ACCUM_BLUE_BITS_ARB 0x2020
    #define WGL_ACCUM_ALPHA_BITS_ARB 0x2021
    #define WGL_DEPTH_BITS_ARB 0x2022
    #define WGL_STENCIL_BITS_ARB 0x2023
    #define WGL_AUX_BUFFERS_ARB 0x2024
    #define WGL_STEREO_ARB 0x2012
    #define WGL_DOUBLE_BUFFER_ARB 0x2011
    #define WGL_SAMPLES_ARB 0x2042
    #define WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB 0x20a9
    #define WGL_CONTEXT_DEBUG_BIT_ARB 0x00000001
    #define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x00000002
    #define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
    #define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
    #define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
    #define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
    #define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
    #define WGL_CONTEXT_FLAGS_ARB 0x2094
    #define WGL_CONTEXT_ES2_PROFILE_BIT_EXT 0x00000004
    #define WGL_CONTEXT_ROBUST_ACCESS_BIT_ARB 0x00000004
    #define WGL_LOSE_CONTEXT_ON_RESET_ARB 0x8252
    #define WGL_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB 0x8256
    #define WGL_NO_RESET_NOTIFICATION_ARB 0x8261
    #define WGL_CONTEXT_RELEASE_BEHAVIOR_ARB 0x2097
    #define WGL_CONTEXT_RELEASE_BEHAVIOR_NONE_ARB 0
    #define WGL_CONTEXT_RELEASE_BEHAVIOR_FLUSH_ARB 0x2098
    #define WGL_CONTEXT_OPENGL_NO_ERROR_ARB 0x31b3
    #define WGL_COLORSPACE_EXT 0x309d
    #define WGL_COLORSPACE_SRGB_EXT 0x3089
    #define ERROR_INVALID_VERSION_ARB 0x2095
    #define ERROR_INVALID_PROFILE_ARB 0x2096
    #define ERROR_INCOMPATIBLE_DEVICE_CONTEXTS_ARB 0x2054
    typedef DWORD (WINAPI * PFN_XInputGetCapabilities)(DWORD,DWORD,XINPUT_CAPABILITIES*);
    typedef DWORD (WINAPI * PFN_XInputGetState)(DWORD,XINPUT_STATE*);
    #define XInputGetCapabilities _glfw.win32.xinput.GetCapabilities
    #define XInputGetState _glfw.win32.xinput.GetState
    typedef HRESULT (WINAPI * PFN_DirectInput8Create)(HINSTANCE,DWORD,REFIID,LPVOID*,LPUNKNOWN);
    #define DirectInput8Create _glfw.win32.dinput8.Create
    typedef BOOL (WINAPI * PFN_EnableNonClientDpiScaling)(HWND);
    typedef BOOL (WINAPI * PFN_SetProcessDpiAwarenessContext)(HANDLE);
    typedef UINT (WINAPI * PFN_GetDpiForWindow)(HWND);
    typedef BOOL (WINAPI * PFN_AdjustWindowRectExForDpi)(LPRECT,DWORD,BOOL,DWORD,UINT);
    typedef int (WINAPI * PFN_GetSystemMetricsForDpi)(int,UINT);
    #define EnableNonClientDpiScaling _glfw.win32.user32.EnableNonClientDpiScaling_
    #define SetProcessDpiAwarenessContext _glfw.win32.user32.SetProcessDpiAwarenessContext_
    #define GetDpiForWindow _glfw.win32.user32.GetDpiForWindow_
    #define AdjustWindowRectExForDpi _glfw.win32.user32.AdjustWindowRectExForDpi_
    #define GetSystemMetricsForDpi _glfw.win32.user32.GetSystemMetricsForDpi_
    typedef HRESULT (WINAPI * PFN_DwmIsCompositionEnabled)(BOOL*);
    typedef HRESULT (WINAPI * PFN_DwmFlush)(VOID);
    typedef HRESULT(WINAPI * PFN_DwmEnableBlurBehindWindow)(HWND,const DWM_BLURBEHIND*);
    typedef HRESULT (WINAPI * PFN_DwmGetColorizationColor)(DWORD*,BOOL*);
    #define DwmIsCompositionEnabled _glfw.win32.dwmapi.IsCompositionEnabled
    #define DwmFlush _glfw.win32.dwmapi.Flush
    #define DwmEnableBlurBehindWindow _glfw.win32.dwmapi.EnableBlurBehindWindow
    #define DwmGetColorizationColor _glfw.win32.dwmapi.GetColorizationColor
    typedef HRESULT (WINAPI * PFN_SetProcessDpiAwareness)(PROCESS_DPI_AWARENESS);
    typedef HRESULT (WINAPI * PFN_GetDpiForMonitor)(HMONITOR,MONITOR_DPI_TYPE,UINT*,UINT*);
    #define SetProcessDpiAwareness _glfw.win32.shcore.SetProcessDpiAwareness_
    #define GetDpiForMonitor _glfw.win32.shcore.GetDpiForMonitor_

    // ntdll.dll function pointer typedefs
    typedef LONG (WINAPI * PFN_RtlVerifyVersionInfo)(OSVERSIONINFOEXW*,ULONG,ULONGLONG);
    #define RtlVerifyVersionInfo _glfw.win32.ntdll.RtlVerifyVersionInfo_

    // WGL extension pointer typedefs
    typedef BOOL (WINAPI * PFNWGLSWAPINTERVALEXTPROC)(int);
    typedef BOOL (WINAPI * PFNWGLGETPIXELFORMATATTRIBIVARBPROC)(HDC,int,int,UINT,const int*,int*);
    typedef const char* (WINAPI * PFNWGLGETEXTENSIONSSTRINGEXTPROC)(void);
    typedef const char* (WINAPI * PFNWGLGETEXTENSIONSSTRINGARBPROC)(HDC);
    typedef HGLRC (WINAPI * PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC,HGLRC,const int*);
    #define wglSwapIntervalEXT _glfw.wgl.SwapIntervalEXT
    #define wglGetPixelFormatAttribivARB _glfw.wgl.GetPixelFormatAttribivARB
    #define wglGetExtensionsStringEXT _glfw.wgl.GetExtensionsStringEXT
    #define wglGetExtensionsStringARB _glfw.wgl.GetExtensionsStringARB
    #define wglCreateContextAttribsARB _glfw.wgl.CreateContextAttribsARB

    // opengl32.dll function pointer typedefs
    typedef HGLRC (WINAPI * PFN_wglCreateContext)(HDC);
    typedef BOOL (WINAPI * PFN_wglDeleteContext)(HGLRC);
    typedef PROC (WINAPI * PFN_wglGetProcAddress)(LPCSTR);
    typedef HDC (WINAPI * PFN_wglGetCurrentDC)(void);
    typedef HGLRC (WINAPI * PFN_wglGetCurrentContext)(void);
    typedef BOOL (WINAPI * PFN_wglMakeCurrent)(HDC,HGLRC);
    typedef BOOL (WINAPI * PFN_wglShareLists)(HGLRC,HGLRC);
    #define wglCreateContext _glfw.wgl.CreateContext
    #define wglDeleteContext _glfw.wgl.DeleteContext
    #define wglGetProcAddress _glfw.wgl.GetProcAddress
    #define wglGetCurrentDC _glfw.wgl.GetCurrentDC
    #define wglGetCurrentContext _glfw.wgl.GetCurrentContext
    #define wglMakeCurrent _glfw.wgl.MakeCurrent
    #define wglShareLists _glfw.wgl.ShareLists
    #define GLFW_WIN32_WINDOW_STATE         _GLFWwindowWin32  win32;
    #define GLFW_WIN32_LIBRARY_WINDOW_STATE _GLFWlibraryWin32 win32;
    #define GLFW_WIN32_MONITOR_STATE        _GLFWmonitorWin32 win32;
    #define GLFW_WIN32_CURSOR_STATE         _GLFWcursorWin32  win32;
    #define GLFW_WGL_CONTEXT_STATE          _GLFWcontextWGL wgl;
    #define GLFW_WGL_LIBRARY_CONTEXT_STATE  _GLFWlibraryWGL wgl;
    typedef struct _GLFWcontextWGL { HDC dc; HGLRC handle; int interval; } _GLFWcontextWGL;
    typedef struct _GLFWlibraryWGL {
        HINSTANCE                           instance;
        PFN_wglCreateContext                CreateContext;
        PFN_wglDeleteContext                DeleteContext;
        PFN_wglGetProcAddress               GetProcAddress;
        PFN_wglGetCurrentDC                 GetCurrentDC;
        PFN_wglGetCurrentContext            GetCurrentContext;
        PFN_wglMakeCurrent                  MakeCurrent;
        PFN_wglShareLists                   ShareLists;
        PFNWGLSWAPINTERVALEXTPROC           SwapIntervalEXT;
        PFNWGLGETPIXELFORMATATTRIBIVARBPROC GetPixelFormatAttribivARB;
        PFNWGLGETEXTENSIONSSTRINGEXTPROC    GetExtensionsStringEXT;
        PFNWGLGETEXTENSIONSSTRINGARBPROC    GetExtensionsStringARB;
        PFNWGLCREATECONTEXTATTRIBSARBPROC   CreateContextAttribsARB;
        GLFWbool                            EXT_swap_control;
        GLFWbool                            EXT_colorspace;
        GLFWbool                            ARB_multisample;
        GLFWbool                            ARB_framebuffer_sRGB;
        GLFWbool                            EXT_framebuffer_sRGB;
        GLFWbool                            ARB_pixel_format;
        GLFWbool                            ARB_create_context;
        GLFWbool                            ARB_create_context_profile;
        GLFWbool                            EXT_create_context_es2_profile;
        GLFWbool                            ARB_create_context_robustness;
        GLFWbool                            ARB_create_context_no_error;
        GLFWbool                            ARB_context_flush_control;
    } _GLFWlibraryWGL;

    typedef struct _GLFWwindowWin32 { HWND handle; HICON bigIcon,smallIcon; GLFWbool cursorTracked,frameAction,iconified,maximized,transparent,scaleToMonitor,keymenu,showDefault; int width,height,lastCursorPosX,lastCursorPosY; WCHAR highSurrogate; } _GLFWwindowWin32;
    typedef struct _GLFWlibraryWin32 {
        HINSTANCE           instance;
        HWND                helperWindowHandle;
        ATOM                helperWindowClass;
        ATOM                mainWindowClass;
        HDEVNOTIFY          deviceNotificationHandle;
        int                 acquiredMonitorCount;
        short int           keycodes[512],scancodes[GLFW_KEY_LAST + 1];
        char                keynames[GLFW_KEY_LAST + 1][5];
        double              restoreCursorPosX, restoreCursorPosY;
        _GLFWwindow*        disabledCursorWindow;
        _GLFWwindow*        capturedCursorWindow;
        RAWINPUT*           rawInput;
        int                 rawInputSize;
        UINT                mouseTrailSize;
        HCURSOR             blankCursor;
        struct { HINSTANCE instance; PFN_DirectInput8Create Create; IDirectInput8W* api; } dinput8;
        struct { HINSTANCE instance; PFN_XInputGetCapabilities GetCapabilities; PFN_XInputGetState GetState; } xinput;
        struct {
            HINSTANCE                       instance;
            PFN_EnableNonClientDpiScaling   EnableNonClientDpiScaling_;
            PFN_SetProcessDpiAwarenessContext SetProcessDpiAwarenessContext_;
            PFN_GetDpiForWindow             GetDpiForWindow_;
            PFN_AdjustWindowRectExForDpi    AdjustWindowRectExForDpi_;
            PFN_GetSystemMetricsForDpi      GetSystemMetricsForDpi_;
        } user32;

        struct {
            HINSTANCE                       instance;
            PFN_DwmIsCompositionEnabled     IsCompositionEnabled;
            PFN_DwmFlush                    Flush;
            PFN_DwmEnableBlurBehindWindow   EnableBlurBehindWindow;
            PFN_DwmGetColorizationColor     GetColorizationColor;
        } dwmapi;

        struct {
            HINSTANCE                       instance;
            PFN_SetProcessDpiAwareness      SetProcessDpiAwareness_;
            PFN_GetDpiForMonitor            GetDpiForMonitor_;
        } shcore;

        struct {
            HINSTANCE                       instance;
            PFN_RtlVerifyVersionInfo        RtlVerifyVersionInfo_;
        } ntdll;
    } _GLFWlibraryWin32;

    typedef struct _GLFWmonitorWin32 {
        HMONITOR            handle;
        WCHAR               adapterName[32];
        WCHAR               displayName[32];
        char                publicAdapterName[32];
        char                publicDisplayName[32];
        GLFWbool            modesPruned;
        GLFWbool            modeChanged;
    } _GLFWmonitorWin32;

    typedef struct _GLFWcursorWin32 {
        HCURSOR             handle;
    } _GLFWcursorWin32;

    int _glfwInitWin32(void);
    WCHAR* _glfwCreateWideStringFromUTF8Win32(const char* source);
    char* _glfwCreateUTF8FromWideStringWin32(const WCHAR* source);
    BOOL _glfwIsWindowsVersionOrGreaterWin32(WORD major, WORD minor, WORD sp);
    BOOL _glfwIsWindows10BuildOrGreaterWin32(WORD build);
    void _glfwInputErrorWin32(int error, const char* description);
    void _glfwUpdateKeyNamesWin32(void);
    void _glfwPollMonitorsWin32(void);
    void _glfwSetVideoModeWin32(_GLFWmonitor* monitor, const GLFWvidmode* desired);
    void _glfwRestoreVideoModeWin32(_GLFWmonitor* monitor);
    void _glfwGetHMONITORContentScaleWin32(HMONITOR handle, float* xscale, float* yscale);
    GLFWbool _glfwCreateWindowWin32(_GLFWwindow* window, const _GLFWwndconfig* wndconfig, const _GLFWctxconfig* ctxconfig, const _GLFWfbconfig* fbconfig);
    void _glfwSetWindowTitleWin32(_GLFWwindow* window, const char* title);
    void _glfwSetWindowIconWin32(_GLFWwindow* window, int count, const GLFWimage* images);
    void _glfwGetWindowPosWin32(_GLFWwindow* window, int* xpos, int* ypos);
    void _glfwSetWindowPosWin32(_GLFWwindow* window, int xpos, int ypos);
    void _glfwGetWindowSizeWin32(_GLFWwindow* window, int* width, int* height);
    void _glfwSetWindowSizeWin32(_GLFWwindow* window, int width, int height);
    void _glfwSetWindowSizeLimitsWin32(_GLFWwindow* window, int minwidth, int minheight, int maxwidth, int maxheight);
    void _glfwSetWindowAspectRatioWin32(_GLFWwindow* window, int numer, int denom);
    void _glfwGetFramebufferSizeWin32(_GLFWwindow* window, int* width, int* height);
    void _glfwGetWindowFrameSizeWin32(_GLFWwindow* window, int* left, int* top, int* right, int* bottom);
    void _glfwGetWindowContentScaleWin32(_GLFWwindow* window, float* xscale, float* yscale);
    void _glfwIconifyWindowWin32(_GLFWwindow* window);
    void _glfwRestoreWindowWin32(_GLFWwindow* window);
    void _glfwMaximizeWindowWin32(_GLFWwindow* window);
    void _glfwShowWindowWin32(_GLFWwindow* window);
    void _glfwHideWindowWin32(_GLFWwindow* window);
    void _glfwRequestWindowAttentionWin32(_GLFWwindow* window);
    void _glfwFocusWindowWin32(_GLFWwindow* window);
    void _glfwSetWindowMonitorWin32(_GLFWwindow* window, _GLFWmonitor* monitor, int xpos, int ypos, int width, int height, int refreshRate);
    GLFWbool _glfwWindowFocusedWin32(_GLFWwindow* window);
    GLFWbool _glfwWindowIconifiedWin32(_GLFWwindow* window);
    GLFWbool _glfwWindowVisibleWin32(_GLFWwindow* window);
    GLFWbool _glfwWindowMaximizedWin32(_GLFWwindow* window);
    GLFWbool _glfwWindowHoveredWin32(_GLFWwindow* window);
    GLFWbool _glfwFramebufferTransparentWin32(_GLFWwindow* window);
    void _glfwSetWindowResizableWin32(_GLFWwindow* window, GLFWbool enabled);
    void _glfwSetWindowDecoratedWin32(_GLFWwindow* window, GLFWbool enabled);
    void _glfwSetWindowFloatingWin32(_GLFWwindow* window, GLFWbool enabled);
    void _glfwSetWindowMousePassthroughWin32(_GLFWwindow* window, GLFWbool enabled);
    float _glfwGetWindowOpacityWin32(_GLFWwindow* window);
    void _glfwSetWindowOpacityWin32(_GLFWwindow* window, float opacity);
    void _glfwSetRawMouseMotionWin32(_GLFWwindow *window, GLFWbool enabled);
    GLFWbool _glfwRawMouseMotionSupportedWin32(void);
    void _glfwPollEventsWin32(void);
    void _glfwWaitEventsWin32(void);
    void _glfwWaitEventsTimeoutWin32(double timeout);
    void _glfwPostEmptyEventWin32(void);
    void _glfwGetCursorPosWin32(_GLFWwindow* window, double* xpos, double* ypos);
    void _glfwSetCursorPosWin32(_GLFWwindow* window, double xpos, double ypos);
    void _glfwSetCursorModeWin32(_GLFWwindow* window, int mode);
    GLFWbool _glfwCreateCursorWin32(_GLFWcursor* cursor, const GLFWimage* image, int xhot, int yhot);
    GLFWbool _glfwCreateStandardCursorWin32(_GLFWcursor* cursor, int shape);
    void _glfwDestroyCursorWin32(_GLFWcursor* cursor);
    void _glfwSetCursorWin32(_GLFWwindow* window, _GLFWcursor* cursor);
    void _glfwGetMonitorPosWin32(_GLFWmonitor* monitor, int* xpos, int* ypos);
    void _glfwGetMonitorWorkareaWin32(_GLFWmonitor* monitor, int* xpos, int* ypos, int* width, int* height);
    GLFWvidmode* _glfwGetVideoModesWin32(_GLFWmonitor* monitor, int* count);
    GLFWbool _glfwGetVideoModeWin32(_GLFWmonitor* monitor, GLFWvidmode* mode);

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

#if defined(_GLFW_X11)
    #include <unistd.h>
    #include <signal.h>
    typedef unsigned char KeyCode; typedef int Bool; typedef unsigned long Atom; typedef unsigned long KeySym;
    #include <X11/extensions/XKBstr.h>
    #define	XkbActionMessageLength 6
    #define XA_ATOM ((Atom) 4)
    #define XA_CARDINAL ((Atom) 6)
    #define XA_WINDOW ((Atom) 33)
    #include <X11/Xresource.h>
    typedef unsigned int XcursorUInt; typedef XcursorUInt XcursorDim; typedef XcursorUInt XcursorPixel;
    typedef struct _XcursorImage { XcursorUInt version; XcursorDim size,width,height,xhot,yhot; XcursorUInt delay; XcursorPixel *pixels; } XcursorImage;
    #include <X11/extensions/Xrandr.h>
    #include <X11/XKBlib.h>
    #include <X11/extensions/Xinerama.h>
    #include <X11/extensions/XInput2.h>
    #define ShapeSet 0
    #define ShapeInput 2
    #define GLX_VENDOR 1
    #define GLX_RGBA_BIT 0x00000001
    #define GLX_WINDOW_BIT 0x00000001
    #define GLX_DRAWABLE_TYPE 0x8010
    #define GLX_RENDER_TYPE 0x8011
    #define GLX_RGBA_TYPE 0x8014
    #define GLX_DOUBLEBUFFER 5
    #define GLX_STEREO 6
    #define GLX_AUX_BUFFERS 7
    #define GLX_RED_SIZE 8
    #define GLX_GREEN_SIZE 9
    #define GLX_BLUE_SIZE 10
    #define GLX_ALPHA_SIZE 11
    #define GLX_DEPTH_SIZE 12
    #define GLX_STENCIL_SIZE 13
    #define GLX_ACCUM_RED_SIZE 14
    #define GLX_ACCUM_GREEN_SIZE 15
    #define GLX_ACCUM_BLUE_SIZE 16
    #define GLX_ACCUM_ALPHA_SIZE 17
    #define GLX_SAMPLES 0x186a1
    #define GLX_VISUAL_ID 0x800b
    #define GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB 0x20b2
    #define GLX_CONTEXT_DEBUG_BIT_ARB 0x00000001
    #define GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
    #define GLX_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
    #define GLX_CONTEXT_PROFILE_MASK_ARB 0x9126
    #define GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x00000002
    #define GLX_CONTEXT_MAJOR_VERSION_ARB 0x2091
    #define GLX_CONTEXT_MINOR_VERSION_ARB 0x2092
    #define GLX_CONTEXT_FLAGS_ARB 0x2094
    #define GLX_CONTEXT_ES2_PROFILE_BIT_EXT 0x00000004
    #define GLX_CONTEXT_ROBUST_ACCESS_BIT_ARB 0x00000004
    #define GLX_LOSE_CONTEXT_ON_RESET_ARB 0x8252
    #define GLX_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB 0x8256
    #define GLX_NO_RESET_NOTIFICATION_ARB 0x8261
    #define GLX_CONTEXT_RELEASE_BEHAVIOR_ARB 0x2097
    #define GLX_CONTEXT_RELEASE_BEHAVIOR_NONE_ARB 0
    #define GLX_CONTEXT_RELEASE_BEHAVIOR_FLUSH_ARB 0x2098
    #define GLX_CONTEXT_OPENGL_NO_ERROR_ARB 0x31b3

    typedef XID GLXWindow;
    typedef XID GLXDrawable;
    typedef struct __GLXFBConfig* GLXFBConfig;
    typedef struct __GLXcontext* GLXContext;
    typedef void (*__GLXextproc)(void);

    typedef XClassHint* (* PFN_XAllocClassHint)(void);
    typedef XSizeHints* (* PFN_XAllocSizeHints)(void);
    typedef XWMHints* (* PFN_XAllocWMHints)(void);
    typedef int (* PFN_XChangeProperty)(Display*,Window,Atom,Atom,int,int,const unsigned char*,int);
    typedef int (* PFN_XChangeWindowAttributes)(Display*,Window,unsigned long,XSetWindowAttributes*);
    typedef Bool (* PFN_XCheckIfEvent)(Display*,XEvent*,Bool(*)(Display*,XEvent*,XPointer),XPointer);
    typedef Bool (* PFN_XCheckTypedWindowEvent)(Display*,Window,int,XEvent*);
    typedef int (* PFN_XCloseDisplay)(Display*);
    typedef Status (* PFN_XCloseIM)(XIM);
    typedef int (* PFN_XConvertSelection)(Display*,Atom,Atom,Atom,Window,Time);
    typedef Colormap (* PFN_XCreateColormap)(Display*,Window,Visual*,int);
    typedef Cursor (* PFN_XCreateFontCursor)(Display*,unsigned int);
    typedef XIC (* PFN_XCreateIC)(XIM,...);
    typedef Region (* PFN_XCreateRegion)(void);
    typedef Window (* PFN_XCreateWindow)(Display*,Window,int,int,unsigned int,unsigned int,unsigned int,int,unsigned int,Visual*,unsigned long,XSetWindowAttributes*);
    typedef int (* PFN_XDefineCursor)(Display*,Window,Cursor);
    typedef int (* PFN_XDeleteContext)(Display*,XID,XContext);
    typedef int (* PFN_XDeleteProperty)(Display*,Window,Atom);
    typedef void (* PFN_XDestroyIC)(XIC);
    typedef int (* PFN_XDestroyRegion)(Region);
    typedef int (* PFN_XDestroyWindow)(Display*,Window);
    typedef int (* PFN_XDisplayKeycodes)(Display*,int*,int*);
    typedef int (* PFN_XEventsQueued)(Display*,int);
    typedef Bool (* PFN_XFilterEvent)(XEvent*,Window);
    typedef int (* PFN_XFindContext)(Display*,XID,XContext,XPointer*);
    typedef int (* PFN_XFlush)(Display*);
    typedef int (* PFN_XFree)(void*);
    typedef int (* PFN_XFreeColormap)(Display*,Colormap);
    typedef int (* PFN_XFreeCursor)(Display*,Cursor);
    typedef void (* PFN_XFreeEventData)(Display*,XGenericEventCookie*);
    typedef int (* PFN_XGetErrorText)(Display*,int,char*,int);
    typedef Bool (* PFN_XGetEventData)(Display*,XGenericEventCookie*);
    typedef char* (* PFN_XGetICValues)(XIC,...);
    typedef char* (* PFN_XGetIMValues)(XIM,...);
    typedef int (* PFN_XGetInputFocus)(Display*,Window*,int*);
    typedef KeySym* (* PFN_XGetKeyboardMapping)(Display*,KeyCode,int,int*);
    typedef int (* PFN_XGetScreenSaver)(Display*,int*,int*,int*,int*);
    typedef Window (* PFN_XGetSelectionOwner)(Display*,Atom);
    typedef XVisualInfo* (* PFN_XGetVisualInfo)(Display*,long,XVisualInfo*,int*);
    typedef Status (* PFN_XGetWMNormalHints)(Display*,Window,XSizeHints*,long*);
    typedef Status (* PFN_XGetWindowAttributes)(Display*,Window,XWindowAttributes*);
    typedef int (* PFN_XGetWindowProperty)(Display*,Window,Atom,long,long,Bool,Atom,Atom*,int*,unsigned long*,unsigned long*,unsigned char**);
    typedef int (* PFN_XGrabPointer)(Display*,Window,Bool,unsigned int,int,int,Window,Cursor,Time);
    typedef Status (* PFN_XIconifyWindow)(Display*,Window,int);
    typedef Status (* PFN_XInitThreads)(void);
    typedef Atom (* PFN_XInternAtom)(Display*,const char*,Bool);
    typedef int (* PFN_XLookupString)(XKeyEvent*,char*,int,KeySym*,XComposeStatus*);
    typedef int (* PFN_XMapRaised)(Display*,Window);
    typedef int (* PFN_XMapWindow)(Display*,Window);
    typedef int (* PFN_XMoveResizeWindow)(Display*,Window,int,int,unsigned int,unsigned int);
    typedef int (* PFN_XMoveWindow)(Display*,Window,int,int);
    typedef int (* PFN_XNextEvent)(Display*,XEvent*);
    typedef Display* (* PFN_XOpenDisplay)(const char*);
    typedef XIM (* PFN_XOpenIM)(Display*,XrmDatabase*,char*,char*);
    typedef int (* PFN_XPeekEvent)(Display*,XEvent*);
    typedef int (* PFN_XPending)(Display*);
    typedef Bool (* PFN_XQueryExtension)(Display*,const char*,int*,int*,int*);
    typedef Bool (* PFN_XQueryPointer)(Display*,Window,Window*,Window*,int*,int*,int*,int*,unsigned int*);
    typedef int (* PFN_XRaiseWindow)(Display*,Window);
    typedef Bool (* PFN_XRegisterIMInstantiateCallback)(Display*,void*,char*,char*,XIDProc,XPointer);
    typedef int (* PFN_XResizeWindow)(Display*,Window,unsigned int,unsigned int);
    typedef char* (* PFN_XResourceManagerString)(Display*);
    typedef int (* PFN_XSaveContext)(Display*,XID,XContext,const char*);
    typedef int (* PFN_XSelectInput)(Display*,Window,long);
    typedef Status (* PFN_XSendEvent)(Display*,Window,Bool,long,XEvent*);
    typedef int (* PFN_XSetClassHint)(Display*,Window,XClassHint*);
    typedef XErrorHandler (* PFN_XSetErrorHandler)(XErrorHandler);
    typedef void (* PFN_XSetICFocus)(XIC);
    typedef char* (* PFN_XSetIMValues)(XIM,...);
    typedef int (* PFN_XSetInputFocus)(Display*,Window,int,Time);
    typedef char* (* PFN_XSetLocaleModifiers)(const char*);
    typedef int (* PFN_XSetScreenSaver)(Display*,int,int,int,int);
    typedef int (* PFN_XSetSelectionOwner)(Display*,Atom,Window,Time);
    typedef int (* PFN_XSetWMHints)(Display*,Window,XWMHints*);
    typedef void (* PFN_XSetWMNormalHints)(Display*,Window,XSizeHints*);
    typedef Status (* PFN_XSetWMProtocols)(Display*,Window,Atom*,int);
    typedef Bool (* PFN_XSupportsLocale)(void);
    typedef int (* PFN_XSync)(Display*,Bool);
    typedef Bool (* PFN_XTranslateCoordinates)(Display*,Window,Window,int,int,int*,int*,Window*);
    typedef int (* PFN_XUndefineCursor)(Display*,Window);
    typedef int (* PFN_XUngrabPointer)(Display*,Time);
    typedef int (* PFN_XUnmapWindow)(Display*,Window);
    typedef void (* PFN_XUnsetICFocus)(XIC);
    typedef VisualID (* PFN_XVisualIDFromVisual)(Visual*);
    typedef int (* PFN_XWarpPointer)(Display*,Window,Window,int,int,unsigned int,unsigned int,int,int);
    typedef void (* PFN_XkbFreeKeyboard)(XkbDescPtr,unsigned int,Bool);
    typedef void (* PFN_XkbFreeNames)(XkbDescPtr,unsigned int,Bool);
    typedef XkbDescPtr (* PFN_XkbGetMap)(Display*,unsigned int,unsigned int);
    typedef Status (* PFN_XkbGetNames)(Display*,unsigned int,XkbDescPtr);
    typedef Status (* PFN_XkbGetState)(Display*,unsigned int,XkbStatePtr);
    typedef KeySym (* PFN_XkbKeycodeToKeysym)(Display*,KeyCode,int,int);
    typedef Bool (* PFN_XkbQueryExtension)(Display*,int*,int*,int*,int*,int*);
    typedef Bool (* PFN_XkbSelectEventDetails)(Display*,unsigned int,unsigned int,unsigned long,unsigned long);
    typedef Bool (* PFN_XkbSetDetectableAutoRepeat)(Display*,Bool,Bool*);
    typedef void (* PFN_XrmDestroyDatabase)(XrmDatabase);
    typedef Bool (* PFN_XrmGetResource)(XrmDatabase,const char*,const char*,char**,XrmValue*);
    typedef XrmDatabase (* PFN_XrmGetStringDatabase)(const char*);
    typedef void (* PFN_XrmInitialize)(void);
    typedef XrmQuark (* PFN_XrmUniqueQuark)(void);
    typedef Bool (* PFN_XUnregisterIMInstantiateCallback)(Display*,void*,char*,char*,XIDProc,XPointer);
    typedef int (* PFN_Xutf8LookupString)(XIC,XKeyPressedEvent*,char*,int,KeySym*,Status*);
    typedef void (* PFN_Xutf8SetWMProperties)(Display*,Window,const char*,const char*,char**,int,XSizeHints*,XWMHints*,XClassHint*);
    #define XAllocClassHint _glfw.x11.xlib.AllocClassHint
    #define XAllocSizeHints _glfw.x11.xlib.AllocSizeHints
    #define XAllocWMHints _glfw.x11.xlib.AllocWMHints
    #define XChangeProperty _glfw.x11.xlib.ChangeProperty
    #define XChangeWindowAttributes _glfw.x11.xlib.ChangeWindowAttributes
    #define XCheckIfEvent _glfw.x11.xlib.CheckIfEvent
    #define XCheckTypedWindowEvent _glfw.x11.xlib.CheckTypedWindowEvent
    #define XCloseDisplay _glfw.x11.xlib.CloseDisplay
    #define XCloseIM _glfw.x11.xlib.CloseIM
    #define XConvertSelection _glfw.x11.xlib.ConvertSelection
    #define XCreateColormap _glfw.x11.xlib.CreateColormap
    #define XCreateFontCursor _glfw.x11.xlib.CreateFontCursor
    #define XCreateIC _glfw.x11.xlib.CreateIC
    #define XCreateRegion _glfw.x11.xlib.CreateRegion
    #define XCreateWindow _glfw.x11.xlib.CreateWindow
    #define XDefineCursor _glfw.x11.xlib.DefineCursor
    #define XDeleteContext _glfw.x11.xlib.DeleteContext
    #define XDeleteProperty _glfw.x11.xlib.DeleteProperty
    #define XDestroyIC _glfw.x11.xlib.DestroyIC
    #define XDestroyRegion _glfw.x11.xlib.DestroyRegion
    #define XDestroyWindow _glfw.x11.xlib.DestroyWindow
    #define XDisplayKeycodes _glfw.x11.xlib.DisplayKeycodes
    #define XEventsQueued _glfw.x11.xlib.EventsQueued
    #define XFilterEvent _glfw.x11.xlib.FilterEvent
    #define XFindContext _glfw.x11.xlib.FindContext
    #define XFlush _glfw.x11.xlib.Flush
    #define XFree _glfw.x11.xlib.Free
    #define XFreeColormap _glfw.x11.xlib.FreeColormap
    #define XFreeCursor _glfw.x11.xlib.FreeCursor
    #define XFreeEventData _glfw.x11.xlib.FreeEventData
    #define XGetErrorText _glfw.x11.xlib.GetErrorText
    #define XGetEventData _glfw.x11.xlib.GetEventData
    #define XGetICValues _glfw.x11.xlib.GetICValues
    #define XGetIMValues _glfw.x11.xlib.GetIMValues
    #define XGetInputFocus _glfw.x11.xlib.GetInputFocus
    #define XGetKeyboardMapping _glfw.x11.xlib.GetKeyboardMapping
    #define XGetScreenSaver _glfw.x11.xlib.GetScreenSaver
    #define XGetSelectionOwner _glfw.x11.xlib.GetSelectionOwner
    #define XGetVisualInfo _glfw.x11.xlib.GetVisualInfo
    #define XGetWMNormalHints _glfw.x11.xlib.GetWMNormalHints
    #define XGetWindowAttributes _glfw.x11.xlib.GetWindowAttributes
    #define XGetWindowProperty _glfw.x11.xlib.GetWindowProperty
    #define XGrabPointer _glfw.x11.xlib.GrabPointer
    #define XIconifyWindow _glfw.x11.xlib.IconifyWindow
    #define XInternAtom _glfw.x11.xlib.InternAtom
    #define XLookupString _glfw.x11.xlib.LookupString
    #define XMapRaised _glfw.x11.xlib.MapRaised
    #define XMapWindow _glfw.x11.xlib.MapWindow
    #define XMoveResizeWindow _glfw.x11.xlib.MoveResizeWindow
    #define XMoveWindow _glfw.x11.xlib.MoveWindow
    #define XNextEvent _glfw.x11.xlib.NextEvent
    #define XOpenIM _glfw.x11.xlib.OpenIM
    #define XPeekEvent _glfw.x11.xlib.PeekEvent
    #define XPending _glfw.x11.xlib.Pending
    #define XQueryExtension _glfw.x11.xlib.QueryExtension
    #define XQueryPointer _glfw.x11.xlib.QueryPointer
    #define XRaiseWindow _glfw.x11.xlib.RaiseWindow
    #define XRegisterIMInstantiateCallback _glfw.x11.xlib.RegisterIMInstantiateCallback
    #define XResizeWindow _glfw.x11.xlib.ResizeWindow
    #define XResourceManagerString _glfw.x11.xlib.ResourceManagerString
    #define XSaveContext _glfw.x11.xlib.SaveContext
    #define XSelectInput _glfw.x11.xlib.SelectInput
    #define XSendEvent _glfw.x11.xlib.SendEvent
    #define XSetClassHint _glfw.x11.xlib.SetClassHint
    #define XSetErrorHandler _glfw.x11.xlib.SetErrorHandler
    #define XSetICFocus _glfw.x11.xlib.SetICFocus
    #define XSetIMValues _glfw.x11.xlib.SetIMValues
    #define XSetInputFocus _glfw.x11.xlib.SetInputFocus
    #define XSetLocaleModifiers _glfw.x11.xlib.SetLocaleModifiers
    #define XSetScreenSaver _glfw.x11.xlib.SetScreenSaver
    #define XSetSelectionOwner _glfw.x11.xlib.SetSelectionOwner
    #define XSetWMHints _glfw.x11.xlib.SetWMHints
    #define XSetWMNormalHints _glfw.x11.xlib.SetWMNormalHints
    #define XSetWMProtocols _glfw.x11.xlib.SetWMProtocols
    #define XSupportsLocale _glfw.x11.xlib.SupportsLocale
    #define XSync _glfw.x11.xlib.Sync
    #define XTranslateCoordinates _glfw.x11.xlib.TranslateCoordinates
    #define XUndefineCursor _glfw.x11.xlib.UndefineCursor
    #define XUngrabPointer _glfw.x11.xlib.UngrabPointer
    #define XUnmapWindow _glfw.x11.xlib.UnmapWindow
    #define XUnsetICFocus _glfw.x11.xlib.UnsetICFocus
    #define XVisualIDFromVisual _glfw.x11.xlib.VisualIDFromVisual
    #define XWarpPointer _glfw.x11.xlib.WarpPointer
    #define XkbFreeKeyboard _glfw.x11.xkb.FreeKeyboard
    #define XkbFreeNames _glfw.x11.xkb.FreeNames
    #define XkbGetMap _glfw.x11.xkb.GetMap
    #define XkbGetNames _glfw.x11.xkb.GetNames
    #define XkbGetState _glfw.x11.xkb.GetState
    #define XkbKeycodeToKeysym _glfw.x11.xkb.KeycodeToKeysym
    #define XkbQueryExtension _glfw.x11.xkb.QueryExtension
    #define XkbSelectEventDetails _glfw.x11.xkb.SelectEventDetails
    #define XkbSetDetectableAutoRepeat _glfw.x11.xkb.SetDetectableAutoRepeat
    #define XrmDestroyDatabase _glfw.x11.xrm.DestroyDatabase
    #define XrmGetResource _glfw.x11.xrm.GetResource
    #define XrmGetStringDatabase _glfw.x11.xrm.GetStringDatabase
    #define XrmUniqueQuark _glfw.x11.xrm.UniqueQuark
    #define XUnregisterIMInstantiateCallback _glfw.x11.xlib.UnregisterIMInstantiateCallback
    #define Xutf8LookupString _glfw.x11.xlib.utf8LookupString
    #define Xutf8SetWMProperties _glfw.x11.xlib.utf8SetWMProperties

    typedef XRRCrtcGamma* (* PFN_XRRAllocGamma)(int);
    typedef void (* PFN_XRRFreeCrtcInfo)(XRRCrtcInfo*);
    typedef void (* PFN_XRRFreeGamma)(XRRCrtcGamma*);
    typedef void (* PFN_XRRFreeOutputInfo)(XRROutputInfo*);
    typedef void (* PFN_XRRFreeScreenResources)(XRRScreenResources*);
    typedef XRRCrtcGamma* (* PFN_XRRGetCrtcGamma)(Display*,RRCrtc);
    typedef int (* PFN_XRRGetCrtcGammaSize)(Display*,RRCrtc);
    typedef XRRCrtcInfo* (* PFN_XRRGetCrtcInfo) (Display*,XRRScreenResources*,RRCrtc);
    typedef XRROutputInfo* (* PFN_XRRGetOutputInfo)(Display*,XRRScreenResources*,RROutput);
    typedef RROutput (* PFN_XRRGetOutputPrimary)(Display*,Window);
    typedef XRRScreenResources* (* PFN_XRRGetScreenResourcesCurrent)(Display*,Window);
    typedef Bool (* PFN_XRRQueryExtension)(Display*,int*,int*);
    typedef Status (* PFN_XRRQueryVersion)(Display*,int*,int*);
    typedef void (* PFN_XRRSelectInput)(Display*,Window,int);
    typedef Status (* PFN_XRRSetCrtcConfig)(Display*,XRRScreenResources*,RRCrtc,Time,int,int,RRMode,Rotation,RROutput*,int);
    typedef void (* PFN_XRRSetCrtcGamma)(Display*,RRCrtc,XRRCrtcGamma*);
    typedef int (* PFN_XRRUpdateConfiguration)(XEvent*);
    #define XRRAllocGamma _glfw.x11.randr.AllocGamma
    #define XRRFreeCrtcInfo _glfw.x11.randr.FreeCrtcInfo
    #define XRRFreeGamma _glfw.x11.randr.FreeGamma
    #define XRRFreeOutputInfo _glfw.x11.randr.FreeOutputInfo
    #define XRRFreeScreenResources _glfw.x11.randr.FreeScreenResources
    #define XRRGetCrtcGamma _glfw.x11.randr.GetCrtcGamma
    #define XRRGetCrtcGammaSize _glfw.x11.randr.GetCrtcGammaSize
    #define XRRGetCrtcInfo _glfw.x11.randr.GetCrtcInfo
    #define XRRGetOutputInfo _glfw.x11.randr.GetOutputInfo
    #define XRRGetOutputPrimary _glfw.x11.randr.GetOutputPrimary
    #define XRRGetScreenResourcesCurrent _glfw.x11.randr.GetScreenResourcesCurrent
    #define XRRQueryExtension _glfw.x11.randr.QueryExtension
    #define XRRQueryVersion _glfw.x11.randr.QueryVersion
    #define XRRSelectInput _glfw.x11.randr.SelectInput
    #define XRRSetCrtcConfig _glfw.x11.randr.SetCrtcConfig
    #define XRRSetCrtcGamma _glfw.x11.randr.SetCrtcGamma
    #define XRRUpdateConfiguration _glfw.x11.randr.UpdateConfiguration
    typedef XcursorImage* (* PFN_XcursorImageCreate)(int,int);
    typedef void (* PFN_XcursorImageDestroy)(XcursorImage*);
    typedef Cursor (* PFN_XcursorImageLoadCursor)(Display*,const XcursorImage*);
    typedef char* (* PFN_XcursorGetTheme)(Display*);
    typedef int (* PFN_XcursorGetDefaultSize)(Display*);
    typedef XcursorImage* (* PFN_XcursorLibraryLoadImage)(const char*,const char*,int);
    #define XcursorImageCreate _glfw.x11.xcursor.ImageCreate
    #define XcursorImageDestroy _glfw.x11.xcursor.ImageDestroy
    #define XcursorImageLoadCursor _glfw.x11.xcursor.ImageLoadCursor
    #define XcursorGetTheme _glfw.x11.xcursor.GetTheme
    #define XcursorGetDefaultSize _glfw.x11.xcursor.GetDefaultSize
    #define XcursorLibraryLoadImage _glfw.x11.xcursor.LibraryLoadImage
    typedef Bool (* PFN_XineramaIsActive)(Display*);
    typedef Bool (* PFN_XineramaQueryExtension)(Display*,int*,int*);
    typedef XineramaScreenInfo* (* PFN_XineramaQueryScreens)(Display*,int*);
    #define XineramaIsActive _glfw.x11.xinerama.IsActive
    #define XineramaQueryExtension _glfw.x11.xinerama.QueryExtension
    #define XineramaQueryScreens _glfw.x11.xinerama.QueryScreens
    typedef XID xcb_window_t;
    typedef XID xcb_visualid_t;
    typedef struct xcb_connection_t xcb_connection_t;
    typedef xcb_connection_t* (* PFN_XGetXCBConnection)(Display*);
    #define XGetXCBConnection _glfw.x11.x11xcb.GetXCBConnection
    typedef Bool (* PFN_XF86VidModeQueryExtension)(Display*,int*,int*);
    #define XF86VidModeQueryExtension _glfw.x11.vidmode.QueryExtension
    typedef Status (* PFN_XIQueryVersion)(Display*,int*,int*);
    typedef int (* PFN_XISelectEvents)(Display*,Window,XIEventMask*,int);
    #define XIQueryVersion _glfw.x11.xi.QueryVersion
    #define XISelectEvents _glfw.x11.xi.SelectEvents
    typedef Bool (* PFN_XRenderQueryExtension)(Display*,int*,int*);
    typedef Status (* PFN_XRenderQueryVersion)(Display*dpy,int*,int*);
    typedef XRenderPictFormat* (* PFN_XRenderFindVisualFormat)(Display*,Visual const*);
    #define XRenderQueryExtension _glfw.x11.xrender.QueryExtension
    #define XRenderQueryVersion _glfw.x11.xrender.QueryVersion
    #define XRenderFindVisualFormat _glfw.x11.xrender.FindVisualFormat
    typedef Bool (* PFN_XShapeQueryExtension)(Display*,int*,int*);
    typedef Status (* PFN_XShapeQueryVersion)(Display*dpy,int*,int*);
    typedef void (* PFN_XShapeCombineRegion)(Display*,Window,int,int,int,Region,int);
    typedef void (* PFN_XShapeCombineMask)(Display*,Window,int,int,int,Pixmap,int);
    #define XShapeQueryExtension _glfw.x11.xshape.QueryExtension
    #define XShapeQueryVersion _glfw.x11.xshape.QueryVersion
    #define XShapeCombineRegion _glfw.x11.xshape.ShapeCombineRegion
    #define XShapeCombineMask _glfw.x11.xshape.ShapeCombineMask
    typedef int (*PFNGLXGETFBCONFIGATTRIBPROC)(Display*,GLXFBConfig,int,int*);
    typedef const char* (*PFNGLXGETCLIENTSTRINGPROC)(Display*,int);
    typedef Bool (*PFNGLXQUERYEXTENSIONPROC)(Display*,int*,int*);
    typedef Bool (*PFNGLXQUERYVERSIONPROC)(Display*,int*,int*);
    typedef void (*PFNGLXDESTROYCONTEXTPROC)(Display*,GLXContext);
    typedef Bool (*PFNGLXMAKECURRENTPROC)(Display*,GLXDrawable,GLXContext);
    typedef void (*PFNGLXSWAPBUFFERSPROC)(Display*,GLXDrawable);
    typedef const char* (*PFNGLXQUERYEXTENSIONSSTRINGPROC)(Display*,int);
    typedef GLXFBConfig* (*PFNGLXGETFBCONFIGSPROC)(Display*,int,int*);
    typedef GLXContext (*PFNGLXCREATENEWCONTEXTPROC)(Display*,GLXFBConfig,int,GLXContext,Bool);
    typedef __GLXextproc (* PFNGLXGETPROCADDRESSPROC)(const GLubyte *procName);
    typedef void (*PFNGLXSWAPINTERVALEXTPROC)(Display*,GLXDrawable,int);
    typedef XVisualInfo* (*PFNGLXGETVISUALFROMFBCONFIGPROC)(Display*,GLXFBConfig);
    typedef GLXWindow (*PFNGLXCREATEWINDOWPROC)(Display*,GLXFBConfig,Window,const int*);
    typedef void (*PFNGLXDESTROYWINDOWPROC)(Display*,GLXWindow);
    typedef int (*PFNGLXSWAPINTERVALMESAPROC)(int);
    typedef int (*PFNGLXSWAPINTERVALSGIPROC)(int);
    typedef GLXContext (*PFNGLXCREATECONTEXTATTRIBSARBPROC)(Display*,GLXFBConfig,GLXContext,Bool,const int*);
    #define glXGetFBConfigs _glfw.glx.GetFBConfigs
    #define glXGetFBConfigAttrib _glfw.glx.GetFBConfigAttrib
    #define glXGetClientString _glfw.glx.GetClientString
    #define glXQueryExtension _glfw.glx.QueryExtension
    #define glXQueryVersion _glfw.glx.QueryVersion
    #define glXDestroyContext _glfw.glx.DestroyContext
    #define glXMakeCurrent _glfw.glx.MakeCurrent
    #define glXSwapBuffers _glfw.glx.SwapBuffers
    #define glXQueryExtensionsString _glfw.glx.QueryExtensionsString
    #define glXCreateNewContext _glfw.glx.CreateNewContext
    #define glXGetVisualFromFBConfig _glfw.glx.GetVisualFromFBConfig
    #define glXCreateWindow _glfw.glx.CreateWindow
    #define glXDestroyWindow _glfw.glx.DestroyWindow
    #include <poll.h>
    GLFWbool _glfwPollPOSIX(struct pollfd* fds, nfds_t count, double* timeout);
    #define GLFW_X11_WINDOW_STATE           _GLFWwindowX11 x11;
    #define GLFW_X11_LIBRARY_WINDOW_STATE   _GLFWlibraryX11 x11;
    #define GLFW_X11_MONITOR_STATE          _GLFWmonitorX11 x11;
    #define GLFW_X11_CURSOR_STATE           _GLFWcursorX11 x11;
    #define GLFW_GLX_CONTEXT_STATE          _GLFWcontextGLX glx;
    #define GLFW_GLX_LIBRARY_CONTEXT_STATE  _GLFWlibraryGLX glx;
    typedef struct _GLFWcontextGLX { GLXContext handle; GLXWindow window; GLXFBConfig fbconfig; } _GLFWcontextGLX;
    typedef struct _GLFWlibraryGLX {
        int             major, minor,eventBase,errorBase;
        void*           handle;
        PFNGLXGETFBCONFIGSPROC              GetFBConfigs;
        PFNGLXGETFBCONFIGATTRIBPROC         GetFBConfigAttrib;
        PFNGLXGETCLIENTSTRINGPROC           GetClientString;
        PFNGLXQUERYEXTENSIONPROC            QueryExtension;
        PFNGLXQUERYVERSIONPROC              QueryVersion;
        PFNGLXDESTROYCONTEXTPROC            DestroyContext;
        PFNGLXMAKECURRENTPROC               MakeCurrent;
        PFNGLXSWAPBUFFERSPROC               SwapBuffers;
        PFNGLXQUERYEXTENSIONSSTRINGPROC     QueryExtensionsString;
        PFNGLXCREATENEWCONTEXTPROC          CreateNewContext;
        PFNGLXGETVISUALFROMFBCONFIGPROC     GetVisualFromFBConfig;
        PFNGLXCREATEWINDOWPROC              CreateWindow;
        PFNGLXDESTROYWINDOWPROC             DestroyWindow;
        PFNGLXGETPROCADDRESSPROC            GetProcAddress;
        PFNGLXGETPROCADDRESSPROC            GetProcAddressARB;
        PFNGLXSWAPINTERVALSGIPROC           SwapIntervalSGI;
        PFNGLXSWAPINTERVALEXTPROC           SwapIntervalEXT;
        PFNGLXSWAPINTERVALMESAPROC          SwapIntervalMESA;
        PFNGLXCREATECONTEXTATTRIBSARBPROC   CreateContextAttribsARB;
        GLFWbool        SGI_swap_control;
        GLFWbool        EXT_swap_control;
        GLFWbool        MESA_swap_control;
        GLFWbool        ARB_multisample;
        GLFWbool        ARB_framebuffer_sRGB;
        GLFWbool        EXT_framebuffer_sRGB;
        GLFWbool        ARB_create_context;
        GLFWbool        ARB_create_context_profile;
        GLFWbool        ARB_create_context_robustness;
        GLFWbool        EXT_create_context_es2_profile;
        GLFWbool        ARB_create_context_no_error;
        GLFWbool        ARB_context_flush_control;
    } _GLFWlibraryGLX;

    typedef struct _GLFWwindowX11 { Colormap colormap; Window handle,parent; XIC ic; GLFWbool overrideRedirect,iconified,maximized,transparent; int width,height,xpos,ypos,lastCursorPosX,lastCursorPosY,warpCursorPosX,warpCursorPosY; Time keyPressTimes[256]; } _GLFWwindowX11;
    typedef struct _GLFWlibraryX11 {
        Display*        display;
        int             screen;
        Window          root;
        float           contentScaleX, contentScaleY;
        Window          helperWindowHandle;
        Cursor          hiddenCursorHandle;
        XContext        context;
        XIM             im;
        XErrorHandler   errorHandler;
        int             errorCode;
        char            keynames[GLFW_KEY_LAST + 1][5];
        short int       keycodes[256];
        short int       scancodes[GLFW_KEY_LAST + 1];
        double          restoreCursorPosX, restoreCursorPosY;
        _GLFWwindow*    disabledCursorWindow;
        int             emptyEventPipe[2];
        Atom            NET_SUPPORTED;
        Atom            NET_SUPPORTING_WM_CHECK;
        Atom            WM_PROTOCOLS;
        Atom            WM_STATE;
        Atom            WM_DELETE_WINDOW;
        Atom            NET_WM_NAME;
        Atom            NET_WM_ICON_NAME;
        Atom            NET_WM_ICON;
        Atom            NET_WM_PID;
        Atom            NET_WM_PING;
        Atom            NET_WM_WINDOW_TYPE;
        Atom            NET_WM_WINDOW_TYPE_NORMAL;
        Atom            NET_WM_STATE;
        Atom            NET_WM_STATE_ABOVE;
        Atom            NET_WM_STATE_FULLSCREEN;
        Atom            NET_WM_STATE_MAXIMIZED_VERT;
        Atom            NET_WM_STATE_MAXIMIZED_HORZ;
        Atom            NET_WM_STATE_DEMANDS_ATTENTION;
        Atom            NET_WM_BYPASS_COMPOSITOR;
        Atom            NET_WM_FULLSCREEN_MONITORS;
        Atom            NET_WM_WINDOW_OPACITY;
        Atom            NET_WM_CM_Sx;
        Atom            NET_WORKAREA;
        Atom            NET_CURRENT_DESKTOP;
        Atom            NET_ACTIVE_WINDOW;
        Atom            NET_FRAME_EXTENTS;
        Atom            NET_REQUEST_FRAME_EXTENTS;
        Atom            MOTIF_WM_HINTS;
        Atom            XdndAware;
        Atom            XdndEnter;
        Atom            XdndPosition;
        Atom            XdndStatus;
        Atom            XdndActionCopy;
        Atom            XdndDrop;
        Atom            XdndFinished;
        Atom            XdndSelection;
        Atom            XdndTypeList;
        Atom            text_uri_list;
        Atom            UTF8_STRING;
        struct {
            void*       handle;
            GLFWbool    utf8;
            PFN_XAllocClassHint AllocClassHint;
            PFN_XAllocSizeHints AllocSizeHints;
            PFN_XAllocWMHints AllocWMHints;
            PFN_XChangeProperty ChangeProperty;
            PFN_XChangeWindowAttributes ChangeWindowAttributes;
            PFN_XCheckIfEvent CheckIfEvent;
            PFN_XCheckTypedWindowEvent CheckTypedWindowEvent;
            PFN_XCloseDisplay CloseDisplay;
            PFN_XCloseIM CloseIM;
            PFN_XConvertSelection ConvertSelection;
            PFN_XCreateColormap CreateColormap;
            PFN_XCreateFontCursor CreateFontCursor;
            PFN_XCreateIC CreateIC;
            PFN_XCreateRegion CreateRegion;
            PFN_XCreateWindow CreateWindow;
            PFN_XDefineCursor DefineCursor;
            PFN_XDeleteContext DeleteContext;
            PFN_XDeleteProperty DeleteProperty;
            PFN_XDestroyIC DestroyIC;
            PFN_XDestroyRegion DestroyRegion;
            PFN_XDestroyWindow DestroyWindow;
            PFN_XDisplayKeycodes DisplayKeycodes;
            PFN_XEventsQueued EventsQueued;
            PFN_XFilterEvent FilterEvent;
            PFN_XFindContext FindContext;
            PFN_XFlush Flush;
            PFN_XFree Free;
            PFN_XFreeColormap FreeColormap;
            PFN_XFreeCursor FreeCursor;
            PFN_XFreeEventData FreeEventData;
            PFN_XGetErrorText GetErrorText;
            PFN_XGetEventData GetEventData;
            PFN_XGetICValues GetICValues;
            PFN_XGetIMValues GetIMValues;
            PFN_XGetInputFocus GetInputFocus;
            PFN_XGetKeyboardMapping GetKeyboardMapping;
            PFN_XGetScreenSaver GetScreenSaver;
            PFN_XGetSelectionOwner GetSelectionOwner;
            PFN_XGetVisualInfo GetVisualInfo;
            PFN_XGetWMNormalHints GetWMNormalHints;
            PFN_XGetWindowAttributes GetWindowAttributes;
            PFN_XGetWindowProperty GetWindowProperty;
            PFN_XGrabPointer GrabPointer;
            PFN_XIconifyWindow IconifyWindow;
            PFN_XInternAtom InternAtom;
            PFN_XLookupString LookupString;
            PFN_XMapRaised MapRaised;
            PFN_XMapWindow MapWindow;
            PFN_XMoveResizeWindow MoveResizeWindow;
            PFN_XMoveWindow MoveWindow;
            PFN_XNextEvent NextEvent;
            PFN_XOpenIM OpenIM;
            PFN_XPeekEvent PeekEvent;
            PFN_XPending Pending;
            PFN_XQueryExtension QueryExtension;
            PFN_XQueryPointer QueryPointer;
            PFN_XRaiseWindow RaiseWindow;
            PFN_XRegisterIMInstantiateCallback RegisterIMInstantiateCallback;
            PFN_XResizeWindow ResizeWindow;
            PFN_XResourceManagerString ResourceManagerString;
            PFN_XSaveContext SaveContext;
            PFN_XSelectInput SelectInput;
            PFN_XSendEvent SendEvent;
            PFN_XSetClassHint SetClassHint;
            PFN_XSetErrorHandler SetErrorHandler;
            PFN_XSetICFocus SetICFocus;
            PFN_XSetIMValues SetIMValues;
            PFN_XSetInputFocus SetInputFocus;
            PFN_XSetLocaleModifiers SetLocaleModifiers;
            PFN_XSetScreenSaver SetScreenSaver;
            PFN_XSetSelectionOwner SetSelectionOwner;
            PFN_XSetWMHints SetWMHints;
            PFN_XSetWMNormalHints SetWMNormalHints;
            PFN_XSetWMProtocols SetWMProtocols;
            PFN_XSupportsLocale SupportsLocale;
            PFN_XSync Sync;
            PFN_XTranslateCoordinates TranslateCoordinates;
            PFN_XUndefineCursor UndefineCursor;
            PFN_XUngrabPointer UngrabPointer;
            PFN_XUnmapWindow UnmapWindow;
            PFN_XUnsetICFocus UnsetICFocus;
            PFN_XVisualIDFromVisual VisualIDFromVisual;
            PFN_XWarpPointer WarpPointer;
            PFN_XUnregisterIMInstantiateCallback UnregisterIMInstantiateCallback;
            PFN_Xutf8LookupString utf8LookupString;
            PFN_Xutf8SetWMProperties utf8SetWMProperties;
        } xlib;

        struct {
            PFN_XrmDestroyDatabase DestroyDatabase;
            PFN_XrmGetResource GetResource;
            PFN_XrmGetStringDatabase GetStringDatabase;
            PFN_XrmUniqueQuark UniqueQuark;
        } xrm;

        struct {
            GLFWbool    available;
            void*       handle;
            int         eventBase;
            int         errorBase;
            int         major;
            int         minor;
            GLFWbool    gammaBroken;
            GLFWbool    monitorBroken;
            PFN_XRRAllocGamma AllocGamma;
            PFN_XRRFreeCrtcInfo FreeCrtcInfo;
            PFN_XRRFreeGamma FreeGamma;
            PFN_XRRFreeOutputInfo FreeOutputInfo;
            PFN_XRRFreeScreenResources FreeScreenResources;
            PFN_XRRGetCrtcGamma GetCrtcGamma;
            PFN_XRRGetCrtcGammaSize GetCrtcGammaSize;
            PFN_XRRGetCrtcInfo GetCrtcInfo;
            PFN_XRRGetOutputInfo GetOutputInfo;
            PFN_XRRGetOutputPrimary GetOutputPrimary;
            PFN_XRRGetScreenResourcesCurrent GetScreenResourcesCurrent;
            PFN_XRRQueryExtension QueryExtension;
            PFN_XRRQueryVersion QueryVersion;
            PFN_XRRSelectInput SelectInput;
            PFN_XRRSetCrtcConfig SetCrtcConfig;
            PFN_XRRSetCrtcGamma SetCrtcGamma;
            PFN_XRRUpdateConfiguration UpdateConfiguration;
        } randr;

        struct {
            GLFWbool available,detectable;
            int majorOpcode,eventBase,errorBase,major,minor;
            unsigned int group;
            PFN_XkbFreeKeyboard FreeKeyboard;
            PFN_XkbFreeNames FreeNames;
            PFN_XkbGetMap GetMap;
            PFN_XkbGetNames GetNames;
            PFN_XkbGetState GetState;
            PFN_XkbKeycodeToKeysym KeycodeToKeysym;
            PFN_XkbQueryExtension QueryExtension;
            PFN_XkbSelectEventDetails SelectEventDetails;
            PFN_XkbSetDetectableAutoRepeat SetDetectableAutoRepeat;
        } xkb;
        struct { int count,timeout,interval,blanking,exposure; } saver;
        struct { int version; Window source; Atom format; } xdnd;
        struct { void* handle; PFN_XcursorImageCreate ImageCreate; PFN_XcursorImageDestroy ImageDestroy; PFN_XcursorImageLoadCursor ImageLoadCursor; PFN_XcursorGetTheme GetTheme; PFN_XcursorGetDefaultSize GetDefaultSize; PFN_XcursorLibraryLoadImage LibraryLoadImage; } xcursor;

        struct {
            GLFWbool    available;
            void*       handle;
            int         major;
            int         minor;
            PFN_XineramaIsActive IsActive;
            PFN_XineramaQueryExtension QueryExtension;
            PFN_XineramaQueryScreens QueryScreens;
        } xinerama;

        struct { void* handle; PFN_XGetXCBConnection GetXCBConnection; } x11xcb;

        struct {
            GLFWbool    available;
            void*       handle;
            int         eventBase;
            int         errorBase;
            PFN_XF86VidModeQueryExtension QueryExtension;
        } vidmode;

        struct {
            GLFWbool    available;
            void*       handle;
            int         majorOpcode;
            int         eventBase;
            int         errorBase;
            int         major;
            int         minor;
            PFN_XIQueryVersion QueryVersion;
            PFN_XISelectEvents SelectEvents;
        } xi;

        struct {
            GLFWbool    available;
            void*       handle;
            int         major;
            int         minor;
            int         eventBase;
            int         errorBase;
            PFN_XRenderQueryExtension QueryExtension;
            PFN_XRenderQueryVersion QueryVersion;
            PFN_XRenderFindVisualFormat FindVisualFormat;
        } xrender;

        struct {
            GLFWbool    available;
            void*       handle;
            int         major,minor,eventBase, errorBase;
            PFN_XShapeQueryExtension QueryExtension;
            PFN_XShapeCombineRegion ShapeCombineRegion;
            PFN_XShapeQueryVersion QueryVersion;
            PFN_XShapeCombineMask ShapeCombineMask;
        } xshape;
    } _GLFWlibraryX11;

    typedef struct _GLFWmonitorX11 { RROutput output; RRCrtc crtc; RRMode oldMode; int index; } _GLFWmonitorX11;
    typedef struct _GLFWcursorX11 { Cursor handle; } _GLFWcursorX11;
    void _glfwSetWindowTitleX11(_GLFWwindow* window, const char* title);
    void _glfwGetWindowPosX11(_GLFWwindow* window, int* xpos, int* ypos);
    void _glfwGetWindowSizeX11(_GLFWwindow* window, int* width, int* height);
    void _glfwIconifyWindowX11(_GLFWwindow* window);
    void _glfwShowWindowX11(_GLFWwindow* window);
    void _glfwFocusWindowX11(_GLFWwindow* window);
    GLFWbool _glfwWindowIconifiedX11(_GLFWwindow* window);
    GLFWbool _glfwWindowVisibleX11(_GLFWwindow* window);
    GLFWbool _glfwWindowMaximizedX11(_GLFWwindow* window);
    void _glfwSetWindowDecoratedX11(_GLFWwindow* window, GLFWbool enabled);
    void _glfwSetWindowFloatingX11(_GLFWwindow* window, GLFWbool enabled);
    void _glfwSetWindowMousePassthroughX11(_GLFWwindow* window, GLFWbool enabled);
    void _glfwGetCursorPosX11(_GLFWwindow* window, double* xpos, double* ypos);
    void _glfwSetCursorPosX11(_GLFWwindow* window, double xpos, double ypos);
    void _glfwGetMonitorPosX11(_GLFWmonitor* monitor, int* xpos, int* ypos);
    GLFWbool _glfwGetVideoModeX11(_GLFWmonitor* monitor, GLFWvidmode* mode);
    void _glfwPollMonitorsX11(void);
    void _glfwSetVideoModeX11(_GLFWmonitor* monitor, const GLFWvidmode* desired);
    void _glfwRestoreVideoModeX11(_GLFWmonitor* monitor);
    unsigned long _glfwGetWindowPropertyX11(Window window, Atom property, Atom type, unsigned char** value);
    GLFWbool _glfwIsVisualTransparentX11(Visual* visual);
    void _glfwGrabErrorHandlerX11(void);
    void _glfwReleaseErrorHandlerX11(void);
    void _glfwInputErrorX11(int error, const char* message);
    void _glfwCreateInputContextX11(_GLFWwindow* window);
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

#if defined(WINDOWS)
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
        GLFW_GLX_CONTEXT_STATE

#define GLFW_PLATFORM_LIBRARY_CONTEXT_STATE \
        GLFW_WGL_LIBRARY_CONTEXT_STATE \
        GLFW_GLX_LIBRARY_CONTEXT_STATE

#if defined(WINDOWS)
    #undef APIENTRY
    #include <windows.h>
    #define GLFW_WIN32_TLS_STATE            _GLFWtlsWin32     win32;
    #define GLFW_WIN32_MUTEX_STATE          _GLFWmutexWin32   win32;
    typedef struct _GLFWtlsWin32 { GLFWbool allocated; DWORD index; } _GLFWtlsWin32;
    typedef struct _GLFWmutexWin32 { GLFWbool allocated; CRITICAL_SECTION section; } _GLFWmutexWin32;
    #define GLFW_PLATFORM_TLS_STATE    GLFW_WIN32_TLS_STATE
    #define GLFW_PLATFORM_MUTEX_STATE  GLFW_WIN32_MUTEX_STATE
#else
    #define SIZE_MAX (~(size_t)0)
    #define GLFW_POSIX_TLS_STATE _GLFWtlsPOSIX   posix;
    #define GLFW_POSIX_MUTEX_STATE _GLFWmutexPOSIX posix;
    typedef struct _GLFWtlsPOSIX { GLFWbool allocated; pthread_key_t key; } _GLFWtlsPOSIX;
    typedef struct _GLFWmutexPOSIX { GLFWbool allocated; pthread_mutex_t handle; } _GLFWmutexPOSIX;
    #define GLFW_PLATFORM_TLS_STATE    GLFW_POSIX_TLS_STATE
    #define GLFW_PLATFORM_MUTEX_STATE  GLFW_POSIX_MUTEX_STATE
#endif

#if defined(WINDOWS)
    #undef APIENTRY
    #include <windows.h>
    #define GLFW_WIN32_LIBRARY_TIMER_STATE  _GLFWtimerWin32   win32;
    typedef struct _GLFWtimerWin32 { u64 frequency; } _GLFWtimerWin32;
    #define GLFW_PLATFORM_LIBRARY_TIMER_STATE  GLFW_WIN32_LIBRARY_TIMER_STATE
#else
    #define GLFW_POSIX_LIBRARY_TIMER_STATE _GLFWtimerPOSIX posix;
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

#if defined(WINDOWS)
    #include <stdlib.h>
    #include <stdio.h>
    #include <math.h>
    #include <string.h>
    #include <windowsx.h>
    #include <shellapi.h>
    #include <wchar.h>
    static DWORD getWindowStyle(const _GLFWwindow* window) {
        DWORD style = WS_CLIPSIBLINGS|WS_CLIPCHILDREN;
        if (window->monitor) style |= WS_POPUP;
        else {
            style |= WS_SYSMENU|WS_MINIMIZEBOX;
            if (window->decorated) { style |= WS_CAPTION; if (window->resizable) style |= WS_MAXIMIZEBOX|WS_THICKFRAME; }
            else style |= WS_POPUP;
        }
        return style;
    }

    static DWORD getWindowExStyle(const _GLFWwindow* window) {
        DWORD style = WS_EX_APPWINDOW;
        if (window->monitor || window->floating) style |= WS_EX_TOPMOST;
        return style;
    }

    static const GLFWimage* chooseImage(int count,const GLFWimage* images,int width,int height) {
        int leastDiff=INT_MAX; const GLFWimage* closest=NULL;
        for (int i=0;i<count;i++) { const int currDiff=abs(images[i].width*images[i].height-width*height); if (currDiff<leastDiff) { closest=images+i; leastDiff=currDiff; } }
        return closest;
    }

    static HICON createIcon(const GLFWimage* image,int xhot,int yhot,GLFWbool icon) {
        HDC dc; HICON handle; HBITMAP color,mask; BITMAPV5HEADER bi; ICONINFO ii;
        unsigned char* target=NULL; unsigned char* source=image->pixels;
        ZeroMemory(&bi,sizeof(bi));
        bi.bV5Size=sizeof(bi); bi.bV5Width=image->width; bi.bV5Height=-image->height;
        bi.bV5Planes=1; bi.bV5BitCount=32; bi.bV5Compression=BI_BITFIELDS;
        bi.bV5RedMask=0x00ff0000; bi.bV5GreenMask=0x0000ff00; bi.bV5BlueMask=0x000000ff; bi.bV5AlphaMask=0xff000000;
        dc=GetDC(NULL);
        color=CreateDIBSection(dc,(BITMAPINFO*)&bi,DIB_RGB_COLORS,(void**)&target,NULL,(DWORD)0);
        ReleaseDC(NULL,dc);
        if (!color) { _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,"Win32: Failed to create RGBA bitmap"); return NULL; }
        mask=CreateBitmap(image->width,image->height,1,1,NULL);
        if (!mask) { _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,"Win32: Failed to create mask bitmap"); DeleteObject(color); return NULL; }
        for (int i=0;i<image->width*image->height;i++) { target[0]=source[2]; target[1]=source[1]; target[2]=source[0]; target[3]=source[3]; target+=4; source+=4; }
        ZeroMemory(&ii,sizeof(ii));
        ii.fIcon=icon; ii.xHotspot=xhot; ii.yHotspot=yhot; ii.hbmMask=mask; ii.hbmColor=color;
        handle=CreateIconIndirect(&ii);
        DeleteObject(color); DeleteObject(mask);
        if (!handle) _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,icon?"Win32: Failed to create icon":"Win32: Failed to create cursor");
        return handle;
    }

    static void applyAspectRatio(_GLFWwindow* window,int edge,RECT* area) {
        RECT frame={0};
        const float ratio=(float)window->numer/(float)window->denom;
        const DWORD style=getWindowStyle(window),exStyle=getWindowExStyle(window);
        if (_glfwIsWindows10Version1607OrGreaterWin32()) AdjustWindowRectExForDpi(&frame,style,FALSE,exStyle,GetDpiForWindow(window->win32.handle));
        else AdjustWindowRectEx(&frame,style,FALSE,exStyle);
        if (edge==WMSZ_LEFT||edge==WMSZ_BOTTOMLEFT||edge==WMSZ_RIGHT||edge==WMSZ_BOTTOMRIGHT)
            area->bottom=area->top+(frame.bottom-frame.top)+(int)(((area->right-area->left)-(frame.right-frame.left))/ratio);
        else if (edge==WMSZ_TOPLEFT||edge==WMSZ_TOPRIGHT)
            area->top=area->bottom-(frame.bottom-frame.top)-(int)(((area->right-area->left)-(frame.right-frame.left))/ratio);
        else if (edge==WMSZ_TOP||edge==WMSZ_BOTTOM)
            area->right=area->left+(frame.right-frame.left)+(int)(((area->bottom-area->top)-(frame.bottom-frame.top))*ratio);
    }

    static void updateCursorImage(_GLFWwindow* window) {
        if (window->cursorMode==GLFW_CURSOR_NORMAL||window->cursorMode==GLFW_CURSOR_CAPTURED) {
            if (window->cursor) SetCursor(window->cursor->win32.handle);
            else SetCursor(LoadCursorW(NULL,(LPCWSTR)IDC_ARROW));
        } else SetCursor(_glfw.win32.blankCursor);
    }

    static void captureCursor(_GLFWwindow* window) {
        RECT clipRect;
        GetClientRect(window->win32.handle,&clipRect);
        ClientToScreen(window->win32.handle,(POINT*)&clipRect.left);
        ClientToScreen(window->win32.handle,(POINT*)&clipRect.right);
        ClipCursor(&clipRect);
        _glfw.win32.capturedCursorWindow=window;
    }

    static void releaseCursor(void) { ClipCursor(NULL); _glfw.win32.capturedCursorWindow=NULL; }

    static void enableRawMouseMotion(_GLFWwindow* window) {
        const RAWINPUTDEVICE rid={0x01,0x02,0,window->win32.handle};
        if (!RegisterRawInputDevices(&rid,1,sizeof(rid))) _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,"Win32: Failed to register raw input device");
    }

    static void disableRawMouseMotion(_GLFWwindow* window) {
        const RAWINPUTDEVICE rid={0x01,0x02,RIDEV_REMOVE,NULL}; (void)window;
        if (!RegisterRawInputDevices(&rid,1,sizeof(rid))) _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,"Win32: Failed to remove raw input device");
    }

    static void disableCursor(_GLFWwindow* window) {
        _glfw.win32.disabledCursorWindow=window;
        _glfwGetCursorPosWin32(window,&_glfw.win32.restoreCursorPosX,&_glfw.win32.restoreCursorPosY);
        updateCursorImage(window);
        _glfwCenterCursorInContentArea(window);
        captureCursor(window);
        if (window->rawMouseMotion) enableRawMouseMotion(window);
    }

    static void enableCursor(_GLFWwindow* window) {
        if (window->rawMouseMotion) disableRawMouseMotion(window);
        _glfw.win32.disabledCursorWindow=NULL;
        releaseCursor();
        _glfwSetCursorPosWin32(window,_glfw.win32.restoreCursorPosX,_glfw.win32.restoreCursorPosY);
        updateCursorImage(window);
    }

    static GLFWbool cursorInContentArea(_GLFWwindow* window) {
        RECT area; POINT pos;
        if (!GetCursorPos(&pos)) return GLFW_FALSE;
        if (WindowFromPoint(pos)!=window->win32.handle) return GLFW_FALSE;
        GetClientRect(window->win32.handle,&area);
        ClientToScreen(window->win32.handle,(POINT*)&area.left);
        ClientToScreen(window->win32.handle,(POINT*)&area.right);
        return PtInRect(&area,pos);
    }

    static void updateWindowStyles(const _GLFWwindow* window) {
        RECT rect;
        DWORD style=GetWindowLongW(window->win32.handle,GWL_STYLE);
        style &= ~(WS_OVERLAPPEDWINDOW|WS_POPUP);
        style |= getWindowStyle(window);
        GetClientRect(window->win32.handle,&rect);
        if (_glfwIsWindows10Version1607OrGreaterWin32()) AdjustWindowRectExForDpi(&rect,style,FALSE,getWindowExStyle(window),GetDpiForWindow(window->win32.handle));
        else AdjustWindowRectEx(&rect,style,FALSE,getWindowExStyle(window));
        ClientToScreen(window->win32.handle,(POINT*)&rect.left);
        ClientToScreen(window->win32.handle,(POINT*)&rect.right);
        SetWindowLongW(window->win32.handle,GWL_STYLE,style);
        SetWindowPos(window->win32.handle,HWND_TOP,rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top,SWP_FRAMECHANGED|SWP_NOACTIVATE|SWP_NOZORDER);
    }

    static void updateFramebufferTransparency(const _GLFWwindow* window) {
        BOOL composition,opaque; DWORD color;
        if (FAILED(DwmIsCompositionEnabled(&composition))||!composition) return;
        if (IsWindows8OrGreater()||(SUCCEEDED(DwmGetColorizationColor(&color,&opaque))&&!opaque)) {
            HRGN region=CreateRectRgn(0,0,-1,-1);
            DWM_BLURBEHIND bb={0};
            bb.dwFlags=DWM_BB_ENABLE|DWM_BB_BLURREGION; bb.hRgnBlur=region; bb.fEnable=TRUE;
            DwmEnableBlurBehindWindow(window->win32.handle,&bb);
            DeleteObject(region);
        } else {
            DWM_BLURBEHIND bb={0};
            bb.dwFlags=DWM_BB_ENABLE;
            DwmEnableBlurBehindWindow(window->win32.handle,&bb);
        }
    }

    static int getKeyMods(void) {
        int mods=0;
        if (GetKeyState(VK_SHIFT)&0x8000)   mods|=GLFW_MOD_SHIFT;
        if (GetKeyState(VK_CONTROL)&0x8000) mods|=GLFW_MOD_CONTROL;
        if (GetKeyState(VK_MENU)&0x8000)    mods|=GLFW_MOD_ALT;
        if ((GetKeyState(VK_LWIN)|GetKeyState(VK_RWIN))&0x8000) mods|=GLFW_MOD_SUPER;
        if (GetKeyState(VK_CAPITAL)&1) mods|=GLFW_MOD_CAPS_LOCK;
        if (GetKeyState(VK_NUMLOCK)&1) mods|=GLFW_MOD_NUM_LOCK;
        return mods;
    }

    static void fitToMonitor(_GLFWwindow* window) {
        MONITORINFO mi = {0}; mi.cbSize = sizeof(mi);
        GetMonitorInfoW(window->monitor->win32.handle,&mi);
        SetWindowPos(window->win32.handle,HWND_TOPMOST,mi.rcMonitor.left,mi.rcMonitor.top,mi.rcMonitor.right-mi.rcMonitor.left,mi.rcMonitor.bottom-mi.rcMonitor.top,SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOCOPYBITS);
    }

    static void acquireMonitor(_GLFWwindow* window) {
        if (!_glfw.win32.acquiredMonitorCount) {
            SetThreadExecutionState(ES_CONTINUOUS|ES_DISPLAY_REQUIRED);
            SystemParametersInfoW(SPI_GETMOUSETRAILS,0,&_glfw.win32.mouseTrailSize,0);
            SystemParametersInfoW(SPI_SETMOUSETRAILS,0,0,0);
        }
        if (!window->monitor->window) _glfw.win32.acquiredMonitorCount++;
        _glfwSetVideoModeWin32(window->monitor,&window->videoMode);
        _glfwInputMonitorWindow(window->monitor,window);
    }

    static void releaseMonitor(_GLFWwindow* window) {
        if (window->monitor->window!=window) return;
        if (!--_glfw.win32.acquiredMonitorCount) {
            SetThreadExecutionState(ES_CONTINUOUS);
            SystemParametersInfoW(SPI_SETMOUSETRAILS,_glfw.win32.mouseTrailSize,0,0);
        }
        _glfwInputMonitorWindow(window->monitor,NULL);
        _glfwRestoreVideoModeWin32(window->monitor);
    }

    static void maximizeWindowManually(_GLFWwindow* window) {
        RECT rect; MONITORINFO mi = {0}; mi.cbSize = sizeof(mi);
        GetMonitorInfoW(MonitorFromWindow(window->win32.handle,MONITOR_DEFAULTTONEAREST),&mi);
        rect=mi.rcWork;
        if (window->maxwidth!=GLFW_DONT_CARE&&window->maxheight!=GLFW_DONT_CARE) { rect.right=vmin(rect.right,rect.left+window->maxwidth); rect.bottom=vmin(rect.bottom,rect.top+window->maxheight); }
        DWORD style=GetWindowLongW(window->win32.handle,GWL_STYLE)|WS_MAXIMIZE;
        SetWindowLongW(window->win32.handle,GWL_STYLE,style);
        if (window->decorated) {
            const DWORD exStyle=GetWindowLongW(window->win32.handle,GWL_EXSTYLE);
            if (_glfwIsWindows10Version1607OrGreaterWin32()) { const UINT dpi=GetDpiForWindow(window->win32.handle); AdjustWindowRectExForDpi(&rect,style,FALSE,exStyle,dpi); OffsetRect(&rect,0,GetSystemMetricsForDpi(SM_CYCAPTION,dpi)); }
            else { AdjustWindowRectEx(&rect,style,FALSE,exStyle); OffsetRect(&rect,0,GetSystemMetrics(SM_CYCAPTION)); }
            rect.bottom=vmin(rect.bottom,mi.rcWork.bottom);
        }
        SetWindowPos(window->win32.handle,HWND_TOP,rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top,SWP_NOACTIVATE|SWP_NOZORDER|SWP_FRAMECHANGED);
    }

    static LRESULT CALLBACK windowProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam) {
        _GLFWwindow* window=GetPropW(hWnd,L"GLFW");
        if (!window) {
            if (uMsg==WM_NCCREATE) {
                if (_glfwIsWindows10Version1607OrGreaterWin32()) {
                    const CREATESTRUCTW* cs=(const CREATESTRUCTW*)lParam;
                    const _GLFWwndconfig* wndconfig=cs->lpCreateParams;
                    if (wndconfig&&wndconfig->scaleToMonitor) EnableNonClientDpiScaling(hWnd);
                }
            }
            return DefWindowProcW(hWnd,uMsg,wParam,lParam);
        }

        switch (uMsg) {
            case WM_MOUSEACTIVATE: {
                if (HIWORD(lParam)==WM_LBUTTONDOWN && LOWORD(lParam)!=HTCLIENT) window->win32.frameAction=GLFW_TRUE;
                break;
            }
            case WM_CAPTURECHANGED: {
                if (lParam==0&&window->win32.frameAction) {
                    if (window->cursorMode==GLFW_CURSOR_DISABLED) disableCursor(window);
                    else if (window->cursorMode==GLFW_CURSOR_CAPTURED) captureCursor(window);
                    window->win32.frameAction=GLFW_FALSE;
                }
                break;
            }
            case WM_SETFOCUS: {
                _glfwInputWindowFocus(window,GLFW_TRUE);
                if (window->win32.frameAction) break;
                if (window->cursorMode==GLFW_CURSOR_DISABLED) disableCursor(window);
                else if (window->cursorMode==GLFW_CURSOR_CAPTURED) captureCursor(window);
                return 0;
            }
            case WM_KILLFOCUS: {
                if (window->cursorMode==GLFW_CURSOR_DISABLED) enableCursor(window);
                else if (window->cursorMode==GLFW_CURSOR_CAPTURED) releaseCursor();
                if (window->monitor&&window->autoIconify) _glfwIconifyWindowWin32(window);
                _glfwInputWindowFocus(window,GLFW_FALSE);
                return 0;
            }
            case WM_SYSCOMMAND: {
                switch (wParam&0xfff0) {
                    case SC_SCREENSAVE:
                    case SC_MONITORPOWER: if (window->monitor) return 0; break;
                    case SC_KEYMENU: if (!window->win32.keymenu) return 0; break;
                }
                break;
            }
            case WM_CLOSE: _glfwInputWindowCloseRequest(window); return 0;
            case WM_INPUTLANGCHANGE: _glfwUpdateKeyNamesWin32(); break;
            case WM_CHAR:
            case WM_SYSCHAR: {
                if (wParam>=0xd800&&wParam<=0xdbff) window->win32.highSurrogate=(WCHAR)wParam;
                else {
                    u32 codepoint=0;
                    if (wParam>=0xdc00&&wParam<=0xdfff) {
                        if (window->win32.highSurrogate) { codepoint+=(window->win32.highSurrogate-0xd800)<<10; codepoint+=(WCHAR)wParam-0xdc00; codepoint+=0x10000; }
                    } else codepoint=(WCHAR)wParam;
                    window->win32.highSurrogate=0;
                    _glfwInputChar(window,codepoint,getKeyMods(),uMsg!=WM_SYSCHAR);
                }
                if (uMsg==WM_SYSCHAR&&window->win32.keymenu) break;
                return 0;
            }
            case WM_UNICHAR: {
                if (wParam==UNICODE_NOCHAR) return TRUE;
                _glfwInputChar(window,(u32)wParam,getKeyMods(),GLFW_TRUE);
                return 0;
            }
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYUP: {
                int key,scancode;
                const int action=(HIWORD(lParam)&KF_UP)?GLFW_RELEASE:GLFW_PRESS,mods=getKeyMods();
                scancode=(HIWORD(lParam)&(KF_EXTENDED|0xff));
                if (!scancode) scancode=MapVirtualKeyW((UINT)wParam,MAPVK_VK_TO_VSC);
                if (scancode==0x54) scancode=0x137;
                if (scancode==0x146) scancode=0x45;
                if (scancode==0x136) scancode=0x36;
                key=_glfw.win32.keycodes[scancode];
                if (wParam==VK_CONTROL) {
                    if (HIWORD(lParam)&KF_EXTENDED) key=GLFW_KEY_RIGHT_CONTROL;
                    else {
                        MSG next; const DWORD time=GetMessageTime();
                        if (PeekMessageW(&next,NULL,0,0,PM_NOREMOVE)) {
                            if (next.message==WM_KEYDOWN||next.message==WM_SYSKEYDOWN||next.message==WM_KEYUP||next.message==WM_SYSKEYUP) {
                                if (next.wParam==VK_MENU&&(HIWORD(next.lParam)&KF_EXTENDED)&&next.time==time) break;
                            }
                        }
                        key=GLFW_KEY_LEFT_CONTROL;
                    }
                } else if (wParam==VK_PROCESSKEY) break;
                if (action==GLFW_RELEASE&&wParam==VK_SHIFT) {
                    _glfwInputKey(window,GLFW_KEY_LEFT_SHIFT,scancode,action,mods);
                    _glfwInputKey(window,GLFW_KEY_RIGHT_SHIFT,scancode,action,mods);
                } else if (wParam==VK_SNAPSHOT) {
                    _glfwInputKey(window,key,scancode,GLFW_PRESS,mods);
                    _glfwInputKey(window,key,scancode,GLFW_RELEASE,mods);
                } else _glfwInputKey(window,key,scancode,action,mods);
                break;
            }
            case WM_LBUTTONDOWN: case WM_RBUTTONDOWN: case WM_MBUTTONDOWN: case WM_XBUTTONDOWN:
            case WM_LBUTTONUP:   case WM_RBUTTONUP:   case WM_MBUTTONUP:   case WM_XBUTTONUP: {
                int i,button,action;
                if      (uMsg==WM_LBUTTONDOWN||uMsg==WM_LBUTTONUP) button=GLFW_MOUSE_BUTTON_LEFT;
                else if (uMsg==WM_RBUTTONDOWN||uMsg==WM_RBUTTONUP) button=GLFW_MOUSE_BUTTON_RIGHT;
                else if (uMsg==WM_MBUTTONDOWN||uMsg==WM_MBUTTONUP) button=GLFW_MOUSE_BUTTON_MIDDLE;
                else if (GET_XBUTTON_WPARAM(wParam)==XBUTTON1)     button=GLFW_MOUSE_BUTTON_4;
                else                                                button=GLFW_MOUSE_BUTTON_5;
                action=(uMsg==WM_LBUTTONDOWN||uMsg==WM_RBUTTONDOWN||uMsg==WM_MBUTTONDOWN||uMsg==WM_XBUTTONDOWN)?GLFW_PRESS:GLFW_RELEASE;
                for (i=0;i<=GLFW_MOUSE_BUTTON_LAST;i++) { if (window->mouseButtons[i]==GLFW_PRESS) break; }
                if (i>GLFW_MOUSE_BUTTON_LAST) SetCapture(hWnd);
                _glfwInputMouseClick(window,button,action,getKeyMods());
                for (i=0;i<=GLFW_MOUSE_BUTTON_LAST;i++) { if (window->mouseButtons[i]==GLFW_PRESS) break; }
                if (i>GLFW_MOUSE_BUTTON_LAST) ReleaseCapture();
                if (uMsg==WM_XBUTTONDOWN||uMsg==WM_XBUTTONUP) return TRUE;
                return 0;
            }
            case WM_MOUSEMOVE: {
                const int x=GET_X_LPARAM(lParam),y=GET_Y_LPARAM(lParam);
                if (!window->win32.cursorTracked) {
                    TRACKMOUSEEVENT tme; ZeroMemory(&tme,sizeof(tme));
                    tme.cbSize=sizeof(tme); tme.dwFlags=TME_LEAVE; tme.hwndTrack=window->win32.handle;
                    TrackMouseEvent(&tme);
                    window->win32.cursorTracked=GLFW_TRUE;
                    _glfwInputCursorEnter(window,GLFW_TRUE);
                }
                if (window->cursorMode==GLFW_CURSOR_DISABLED) {
                    const int dx=x-window->win32.lastCursorPosX,dy=y-window->win32.lastCursorPosY;
                    if (_glfw.win32.disabledCursorWindow!=window||window->rawMouseMotion) break;
                    _glfwInputCursorPos(window,window->virtualCursorPosX+dx,window->virtualCursorPosY+dy);
                } else _glfwInputCursorPos(window,x,y);
                window->win32.lastCursorPosX=x; window->win32.lastCursorPosY=y;
                return 0;
            }
            case WM_INPUT: {
                UINT size=0; HRAWINPUT ri=(HRAWINPUT)lParam; RAWINPUT* data=NULL; int dx,dy;
                if (_glfw.win32.disabledCursorWindow!=window||!window->rawMouseMotion) break;
                GetRawInputData(ri,RID_INPUT,NULL,&size,sizeof(RAWINPUTHEADER));
                if (size>(UINT)_glfw.win32.rawInputSize) {
                    free(_glfw.win32.rawInput);
                    _glfw.win32.rawInput=_glfw_calloc(size,1);
                    _glfw.win32.rawInputSize=size;
                }
                size=_glfw.win32.rawInputSize;
                if (GetRawInputData(ri,RID_INPUT,_glfw.win32.rawInput,&size,sizeof(RAWINPUTHEADER))==(UINT)-1) { _glfwInputError(GLFW_PLATFORM_ERROR,"Win32: Failed to retrieve raw input data"); break; }
                data=_glfw.win32.rawInput;
                if (data->data.mouse.usFlags&MOUSE_MOVE_ABSOLUTE) {
                    POINT pos={0}; int width,height;
                    if (data->data.mouse.usFlags&MOUSE_VIRTUAL_DESKTOP) { pos.x+=GetSystemMetrics(SM_XVIRTUALSCREEN); pos.y+=GetSystemMetrics(SM_YVIRTUALSCREEN); width=GetSystemMetrics(SM_CXVIRTUALSCREEN); height=GetSystemMetrics(SM_CYVIRTUALSCREEN); }
                    else { width=GetSystemMetrics(SM_CXSCREEN); height=GetSystemMetrics(SM_CYSCREEN); }
                    pos.x+=(int)((data->data.mouse.lLastX/65535.f)*width);
                    pos.y+=(int)((data->data.mouse.lLastY/65535.f)*height);
                    ScreenToClient(window->win32.handle,&pos);
                    dx=pos.x-window->win32.lastCursorPosX; dy=pos.y-window->win32.lastCursorPosY;
                } else { dx=data->data.mouse.lLastX; dy=data->data.mouse.lLastY; }
                _glfwInputCursorPos(window,window->virtualCursorPosX+dx,window->virtualCursorPosY+dy);
                window->win32.lastCursorPosX+=dx; window->win32.lastCursorPosY+=dy;
                break;
            }
            case WM_MOUSELEAVE: { window->win32.cursorTracked=GLFW_FALSE; _glfwInputCursorEnter(window,GLFW_FALSE); return 0; }
            case WM_MOUSEWHEEL: { _glfwInputScroll(window,0.0,(SHORT)HIWORD(wParam)/(double)WHEEL_DELTA); return 0; }
            case WM_MOUSEHWHEEL: { _glfwInputScroll(window,-((SHORT)HIWORD(wParam)/(double)WHEEL_DELTA),0.0); return 0; }
            case WM_ENTERSIZEMOVE:
            case WM_ENTERMENULOOP: {
                if (window->win32.frameAction) break;
                if (window->cursorMode==GLFW_CURSOR_DISABLED) enableCursor(window);
                else if (window->cursorMode==GLFW_CURSOR_CAPTURED) releaseCursor();
                break;
            }
            case WM_EXITSIZEMOVE:
            case WM_EXITMENULOOP: {
                if (window->win32.frameAction) break;
                if (window->cursorMode==GLFW_CURSOR_DISABLED) disableCursor(window);
                else if (window->cursorMode==GLFW_CURSOR_CAPTURED) captureCursor(window);
                break;
            }
            case WM_SIZE: {
                const int width=LOWORD(lParam),height=HIWORD(lParam);
                const GLFWbool iconified=wParam==SIZE_MINIMIZED;
                const GLFWbool maximized=wParam==SIZE_MAXIMIZED||(window->win32.maximized&&wParam!=SIZE_RESTORED);
                if (_glfw.win32.capturedCursorWindow==window) captureCursor(window);
                if (window->win32.iconified!=iconified) _glfwInputWindowIconify(window,iconified);
                if (window->win32.maximized!=maximized) _glfwInputWindowMaximize(window,maximized);
                if (width!=window->win32.width||height!=window->win32.height) {
                    window->win32.width=width; window->win32.height=height;
                    _glfwInputFramebufferSize(window,width,height);
                    _glfwInputWindowSize(window,width,height);
                }
                if (window->monitor&&window->win32.iconified!=iconified) {
                    if (iconified) releaseMonitor(window);
                    else { acquireMonitor(window); fitToMonitor(window); }
                }
                window->win32.iconified=iconified; window->win32.maximized=maximized;
                return 0;
            }
            case WM_MOVE: {
                if (_glfw.win32.capturedCursorWindow==window) captureCursor(window);
                _glfwInputWindowPos(window,GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
                return 0;
            }
            case WM_SIZING: {
                if (window->numer==GLFW_DONT_CARE||window->denom==GLFW_DONT_CARE) break;
                applyAspectRatio(window,(int)wParam,(RECT*)lParam);
                return TRUE;
            }
            case WM_GETMINMAXINFO: {
                RECT frame={0}; MINMAXINFO* mmi=(MINMAXINFO*)lParam;
                const DWORD style=getWindowStyle(window),exStyle=getWindowExStyle(window);
                if (window->monitor) break;
                if (_glfwIsWindows10Version1607OrGreaterWin32()) AdjustWindowRectExForDpi(&frame,style,FALSE,exStyle,GetDpiForWindow(window->win32.handle));
                else AdjustWindowRectEx(&frame,style,FALSE,exStyle);
                if (window->minwidth!=GLFW_DONT_CARE&&window->minheight!=GLFW_DONT_CARE) { mmi->ptMinTrackSize.x=window->minwidth+frame.right-frame.left; mmi->ptMinTrackSize.y=window->minheight+frame.bottom-frame.top; }
                if (window->maxwidth!=GLFW_DONT_CARE&&window->maxheight!=GLFW_DONT_CARE) { mmi->ptMaxTrackSize.x=window->maxwidth+frame.right-frame.left; mmi->ptMaxTrackSize.y=window->maxheight+frame.bottom-frame.top; }
                if (!window->decorated) {
                    MONITORINFO mi; const HMONITOR mh=MonitorFromWindow(window->win32.handle,MONITOR_DEFAULTTONEAREST);
                    ZeroMemory(&mi,sizeof(mi)); mi.cbSize=sizeof(mi); GetMonitorInfoW(mh,&mi);
                    mmi->ptMaxPosition.x=mi.rcWork.left-mi.rcMonitor.left; mmi->ptMaxPosition.y=mi.rcWork.top-mi.rcMonitor.top;
                    mmi->ptMaxSize.x=mi.rcWork.right-mi.rcWork.left; mmi->ptMaxSize.y=mi.rcWork.bottom-mi.rcWork.top;
                }
                return 0;
            }
            case WM_PAINT: { _glfwInputWindowDamage(window); break; }
            case WM_ERASEBKGND: return TRUE;
            case WM_NCACTIVATE:
            case WM_NCPAINT: { if (!window->decorated) return TRUE; break; }
            case WM_DWMCOMPOSITIONCHANGED:
            case WM_DWMCOLORIZATIONCOLORCHANGED: { if (window->win32.transparent) updateFramebufferTransparency(window); return 0; }
            case 0x02e4/*get dpi scaled*/: {
                if (window->win32.scaleToMonitor) break;
                if (_glfwIsWindows10Version1703OrGreaterWin32()) {
                    RECT source={0},target={0}; SIZE* size=(SIZE*)lParam;
                    AdjustWindowRectExForDpi(&source,getWindowStyle(window),FALSE,getWindowExStyle(window),GetDpiForWindow(window->win32.handle));
                    AdjustWindowRectExForDpi(&target,getWindowStyle(window),FALSE,getWindowExStyle(window),LOWORD(wParam));
                    size->cx+=(target.right-target.left)-(source.right-source.left);
                    size->cy+=(target.bottom-target.top)-(source.bottom-source.top);
                    return TRUE;
                }
                break;
            }
            case 0x02E0: { // DPI changed
                const float xscale=HIWORD(wParam)/96.0f,yscale=LOWORD(wParam)/96.0f;
                if (!window->monitor&&(window->win32.scaleToMonitor||_glfwIsWindows10Version1703OrGreaterWin32())) {
                    RECT* suggested=(RECT*)lParam;
                    SetWindowPos(window->win32.handle,HWND_TOP,suggested->left,suggested->top,suggested->right-suggested->left,suggested->bottom-suggested->top,SWP_NOACTIVATE|SWP_NOZORDER);
                }
                _glfwInputWindowContentScale(window,xscale,yscale);
                break;
            }
            case WM_SETCURSOR: { if (LOWORD(lParam)==HTCLIENT) { updateCursorImage(window); return TRUE; } break; }
        }
        return DefWindowProcW(hWnd,uMsg,wParam,lParam);
    }

    static int createNativeWindow(_GLFWwindow* window,const _GLFWwndconfig* wndconfig,const _GLFWfbconfig* fbconfig) {
        int frameX,frameY,frameWidth,frameHeight; DWORD style=getWindowStyle(window),exStyle=getWindowExStyle(window);
        if (!_glfw.win32.mainWindowClass) {
            WNDCLASSEXW wc={0}; wc.cbSize=sizeof(wc); wc.style=CS_HREDRAW|CS_VREDRAW|CS_OWNDC, wc.lpfnWndProc=windowProc, wc.hInstance=_glfw.win32.instance, wc.hCursor=LoadCursorW(NULL,(LPCWSTR)IDC_ARROW), wc.lpszClassName=L"GLFW30";
            wc.hIcon=LoadImageW(GetModuleHandleW(NULL),L"GLFW_ICON",IMAGE_ICON,0,0,LR_DEFAULTSIZE|LR_SHARED);
            if (!wc.hIcon) wc.hIcon=LoadImageW(NULL,(LPCWSTR)IDI_APPLICATION,IMAGE_ICON,0,0,LR_DEFAULTSIZE|LR_SHARED);
            if (!(_glfw.win32.mainWindowClass=RegisterClassExW(&wc))) { _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,"Win32: Failed to register window class"); return GLFW_FALSE; }
        }
        if (GetSystemMetrics(SM_REMOTESESSION)&&!_glfw.win32.blankCursor) {
            const int cw=GetSystemMetrics(SM_CXCURSOR),ch=GetSystemMetrics(SM_CYCURSOR); unsigned char* px=_glfw_calloc(cw*ch,4);
            if (!px) return GLFW_FALSE;
            px[3]=1; const GLFWimage img={cw,ch,px}; _glfw.win32.blankCursor=createIcon(&img,0,0,FALSE), free(px);
            if (!_glfw.win32.blankCursor) return GLFW_FALSE;
        }
        if (window->monitor) {
            MONITORINFO mi={0}; mi.cbSize=sizeof(mi), GetMonitorInfoW(window->monitor->win32.handle,&mi);
            frameX=mi.rcMonitor.left, frameY=mi.rcMonitor.top, frameWidth=mi.rcMonitor.right-mi.rcMonitor.left, frameHeight=mi.rcMonitor.bottom-mi.rcMonitor.top;
        } else {
            RECT rect={0,0,wndconfig->width,wndconfig->height}; window->win32.maximized=wndconfig->maximized;
            if (wndconfig->maximized) style|=WS_MAXIMIZE;
            AdjustWindowRectEx(&rect,style,FALSE,exStyle);
            if (wndconfig->xpos==(int)GLFW_ANY_POSITION&&wndconfig->ypos==(int)GLFW_ANY_POSITION) frameX=CW_USEDEFAULT, frameY=CW_USEDEFAULT;
            else frameX=wndconfig->xpos+rect.left, frameY=wndconfig->ypos+rect.top;
            frameWidth=rect.right-rect.left, frameHeight=rect.bottom-rect.top;
        }
        WCHAR* wideTitle=_glfwCreateWideStringFromUTF8Win32(window->title); if (!wideTitle) return GLFW_FALSE;
        window->win32.handle=CreateWindowExW(exStyle,(LPCWSTR)MAKEINTATOM(_glfw.win32.mainWindowClass),wideTitle,style,frameX,frameY,frameWidth,frameHeight,NULL,NULL,_glfw.win32.instance,(LPVOID)wndconfig), free(wideTitle);
        if (!window->win32.handle) { _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,"Win32: Failed to create window"); return GLFW_FALSE; }
        SetPropW(window->win32.handle,L"GLFW",window), ChangeWindowMessageFilterEx(window->win32.handle,WM_DROPFILES,MSGFLT_ALLOW,NULL), ChangeWindowMessageFilterEx(window->win32.handle,WM_COPYDATA,MSGFLT_ALLOW,NULL), ChangeWindowMessageFilterEx(window->win32.handle,0x0049,MSGFLT_ALLOW,NULL);
        window->win32.scaleToMonitor=wndconfig->scaleToMonitor, window->win32.keymenu=wndconfig->win32.keymenu, window->win32.showDefault=wndconfig->win32.showDefault;
        if (!window->monitor) {
            RECT rect={0,0,wndconfig->width,wndconfig->height}; WINDOWPLACEMENT wp={0}; wp.length=sizeof(wp);
            const HMONITOR mh=MonitorFromWindow(window->win32.handle,MONITOR_DEFAULTTONEAREST);
            if (wndconfig->scaleToMonitor) { float xs,ys; _glfwGetHMONITORContentScaleWin32(mh,&xs,&ys); if (xs>0.f&&ys>0.f) { rect.right=(int)(rect.right*xs), rect.bottom=(int)(rect.bottom*ys); }}
            if (_glfwIsWindows10Version1607OrGreaterWin32()) AdjustWindowRectExForDpi(&rect,style,FALSE,exStyle,GetDpiForWindow(window->win32.handle));
            else AdjustWindowRectEx(&rect,style,FALSE,exStyle);
            GetWindowPlacement(window->win32.handle,&wp), OffsetRect(&rect,wp.rcNormalPosition.left-rect.left,wp.rcNormalPosition.top-rect.top);
            wp.rcNormalPosition=rect, wp.showCmd=SW_HIDE, SetWindowPlacement(window->win32.handle,&wp);
            if (wndconfig->maximized&&!wndconfig->decorated) { MONITORINFO mi={0}; mi.cbSize=sizeof(mi), GetMonitorInfoW(mh,&mi), SetWindowPos(window->win32.handle,HWND_TOP,mi.rcWork.left,mi.rcWork.top,mi.rcWork.right-mi.rcWork.left,mi.rcWork.bottom-mi.rcWork.top,SWP_NOACTIVATE|SWP_NOZORDER); }
        }
        DragAcceptFiles(window->win32.handle,TRUE); if (fbconfig->transparent) { updateFramebufferTransparency(window); window->win32.transparent=GLFW_TRUE; }
        _glfwGetWindowSizeWin32(window,&window->win32.width,&window->win32.height); return GLFW_TRUE;
    }

    void _glfwSetWindowTitleWin32(_GLFWwindow* window,const char* title) {
        WCHAR* wideTitle=_glfwCreateWideStringFromUTF8Win32(title);
        if (!wideTitle) return;
        SetWindowTextW(window->win32.handle,wideTitle);
        free(wideTitle);
    }

    void _glfwSetWindowIconWin32(_GLFWwindow* window,int count,const GLFWimage* images) {
        HICON bigIcon=NULL,smallIcon=NULL;
        if (count) {
            bigIcon=createIcon(chooseImage(count,images,GetSystemMetrics(SM_CXICON),GetSystemMetrics(SM_CYICON)),0,0,GLFW_TRUE);
            smallIcon=createIcon(chooseImage(count,images,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON)),0,0,GLFW_TRUE);
        } else {
            bigIcon=(HICON)GetClassLongPtrW(window->win32.handle,GCLP_HICON);
            smallIcon=(HICON)GetClassLongPtrW(window->win32.handle,GCLP_HICONSM);
        }
        SendMessageW(window->win32.handle,WM_SETICON,ICON_BIG,(LPARAM)bigIcon);
        SendMessageW(window->win32.handle,WM_SETICON,ICON_SMALL,(LPARAM)smallIcon);
        if (window->win32.bigIcon) DestroyIcon(window->win32.bigIcon);
        if (window->win32.smallIcon) DestroyIcon(window->win32.smallIcon);
        if (count) { window->win32.bigIcon=bigIcon; window->win32.smallIcon=smallIcon; }
    }

    void _glfwGetWindowPosWin32(_GLFWwindow* window,int* xpos,int* ypos) {
        POINT pos={0,0}; ClientToScreen(window->win32.handle,&pos);
        if (xpos) *xpos=pos.x; if (ypos) *ypos=pos.y;
    }

    void _glfwSetWindowPosWin32(_GLFWwindow* window,int xpos,int ypos) {
        RECT rect={xpos,ypos,xpos,ypos};
        if (_glfwIsWindows10Version1607OrGreaterWin32()) AdjustWindowRectExForDpi(&rect,getWindowStyle(window),FALSE,getWindowExStyle(window),GetDpiForWindow(window->win32.handle));
        else AdjustWindowRectEx(&rect,getWindowStyle(window),FALSE,getWindowExStyle(window));
        SetWindowPos(window->win32.handle,NULL,rect.left,rect.top,0,0,SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOSIZE);
    }

    void _glfwGetWindowSizeWin32(_GLFWwindow* window,int* width,int* height) {
        RECT area; GetClientRect(window->win32.handle,&area);
        if (width) *width=area.right; if (height) *height=area.bottom;
    }

    void _glfwSetWindowSizeWin32(_GLFWwindow* window,int width,int height) {
        if (window->monitor) { if (window->monitor->window==window) { acquireMonitor(window); fitToMonitor(window); } }
        else {
            RECT rect={0,0,width,height};
            if (_glfwIsWindows10Version1607OrGreaterWin32()) AdjustWindowRectExForDpi(&rect,getWindowStyle(window),FALSE,getWindowExStyle(window),GetDpiForWindow(window->win32.handle));
            else AdjustWindowRectEx(&rect,getWindowStyle(window),FALSE,getWindowExStyle(window));
            SetWindowPos(window->win32.handle,HWND_TOP,0,0,rect.right-rect.left,rect.bottom-rect.top,SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOMOVE|SWP_NOZORDER);
        }
    }

    void _glfwSetWindowSizeLimitsWin32(_GLFWwindow* window,int minwidth,int minheight,int maxwidth,int maxheight) {
        RECT area;
        if ((minwidth==GLFW_DONT_CARE||minheight==GLFW_DONT_CARE)&&(maxwidth==GLFW_DONT_CARE||maxheight==GLFW_DONT_CARE)) return;
        GetWindowRect(window->win32.handle,&area);
        MoveWindow(window->win32.handle,area.left,area.top,area.right-area.left,area.bottom-area.top,TRUE);
    }

    void _glfwSetWindowAspectRatioWin32(_GLFWwindow* window,int numer,int denom) {
        RECT area;
        if (numer==GLFW_DONT_CARE||denom==GLFW_DONT_CARE) return;
        GetWindowRect(window->win32.handle,&area);
        applyAspectRatio(window,WMSZ_BOTTOMRIGHT,&area);
        MoveWindow(window->win32.handle,area.left,area.top,area.right-area.left,area.bottom-area.top,TRUE);
    }

    void _glfwGetFramebufferSizeWin32(_GLFWwindow* window,int* width,int* height) { _glfwGetWindowSizeWin32(window,width,height); }

    void _glfwGetWindowFrameSizeWin32(_GLFWwindow* window,int* left,int* top,int* right,int* bottom) {
        RECT rect; int width,height;
        _glfwGetWindowSizeWin32(window,&width,&height);
        SetRect(&rect,0,0,width,height);
        if (_glfwIsWindows10Version1607OrGreaterWin32()) AdjustWindowRectExForDpi(&rect,getWindowStyle(window),FALSE,getWindowExStyle(window),GetDpiForWindow(window->win32.handle));
        else AdjustWindowRectEx(&rect,getWindowStyle(window),FALSE,getWindowExStyle(window));
        if (left) *left=-rect.left; if (top) *top=-rect.top; if (right) *right=rect.right-width; if (bottom) *bottom=rect.bottom-height;
    }

    void _glfwGetWindowContentScaleWin32(_GLFWwindow* window,float* xscale,float* yscale) { _glfwGetHMONITORContentScaleWin32(MonitorFromWindow(window->win32.handle,MONITOR_DEFAULTTONEAREST),xscale,yscale); }
    void _glfwIconifyWindowWin32(_GLFWwindow* window) { ShowWindow(window->win32.handle,SW_MINIMIZE); }
    void _glfwRestoreWindowWin32(_GLFWwindow* window) { ShowWindow(window->win32.handle,SW_RESTORE); }
    void _glfwMaximizeWindowWin32(_GLFWwindow* window) { if (IsWindowVisible(window->win32.handle)) ShowWindow(window->win32.handle,SW_MAXIMIZE); else maximizeWindowManually(window); }
    void _glfwShowWindowWin32(_GLFWwindow* window) {
        int showCommand = SW_SHOWNA;
        if (window->win32.showDefault) {
            STARTUPINFOW si = {0}; si.cb = sizeof(si),GetStartupInfoW(&si);
            if (si.dwFlags & STARTF_USESHOWWINDOW) showCommand = si.wShowWindow;
            window->win32.showDefault = GLFW_FALSE;
        }
        ShowWindow(window->win32.handle, showCommand);
    }

    void _glfwHideWindowWin32(_GLFWwindow* window) { ShowWindow(window->win32.handle,SW_HIDE); }
    void _glfwRequestWindowAttentionWin32(_GLFWwindow* window) { FlashWindow(window->win32.handle,TRUE); }
    void _glfwFocusWindowWin32(_GLFWwindow* window) { BringWindowToTop(window->win32.handle); SetForegroundWindow(window->win32.handle); SetFocus(window->win32.handle); }
    void _glfwSetWindowMonitorWin32(_GLFWwindow* window,_GLFWmonitor* monitor,int xpos,int ypos,int width,int height,int refreshRate) {
        (void)refreshRate;
        if (window->monitor == monitor) {
            if (monitor) { if (monitor->window == window) acquireMonitor(window), fitToMonitor(window); }
            else {
                RECT r = {xpos,ypos,xpos+width,ypos+height};
                if (_glfwIsWindows10Version1607OrGreaterWin32()) AdjustWindowRectExForDpi(&r,getWindowStyle(window),FALSE,getWindowExStyle(window),GetDpiForWindow(window->win32.handle));
                else AdjustWindowRectEx(&r,getWindowStyle(window),FALSE,getWindowExStyle(window));
                SetWindowPos(window->win32.handle,HWND_TOP,r.left,r.top,r.right-r.left,r.bottom-r.top,SWP_NOCOPYBITS|SWP_NOACTIVATE|SWP_NOZORDER);
            }
            return;
        }
        if (window->monitor) releaseMonitor(window);
        _glfwInputWindowMonitor(window,monitor);
        if (window->monitor) {
            MONITORINFO mi = {0}; mi.cbSize = sizeof(mi); UINT f = SWP_SHOWWINDOW|SWP_NOACTIVATE|SWP_NOCOPYBITS;
            if (window->decorated) { DWORD s = GetWindowLongW(window->win32.handle,GWL_STYLE); s &= ~WS_OVERLAPPEDWINDOW, s |= getWindowStyle(window), SetWindowLongW(window->win32.handle,GWL_STYLE,s), f |= SWP_FRAMECHANGED; }
            acquireMonitor(window), GetMonitorInfoW(window->monitor->win32.handle,&mi), SetWindowPos(window->win32.handle,HWND_TOPMOST,mi.rcMonitor.left,mi.rcMonitor.top,mi.rcMonitor.right-mi.rcMonitor.left,mi.rcMonitor.bottom-mi.rcMonitor.top,f);
        } else {
            RECT r = {xpos,ypos,xpos+width,ypos+height}; DWORD s = GetWindowLongW(window->win32.handle,GWL_STYLE); UINT f = SWP_NOACTIVATE|SWP_NOCOPYBITS;
            if (window->decorated) { s &= ~WS_POPUP, s |= getWindowStyle(window), SetWindowLongW(window->win32.handle,GWL_STYLE,s), f |= SWP_FRAMECHANGED; }
            HWND a = window->floating ? HWND_TOPMOST : HWND_NOTOPMOST;
            if (_glfwIsWindows10Version1607OrGreaterWin32()) AdjustWindowRectExForDpi(&r,getWindowStyle(window),FALSE,getWindowExStyle(window),GetDpiForWindow(window->win32.handle));
            else AdjustWindowRectEx(&r,getWindowStyle(window),FALSE,getWindowExStyle(window));
            SetWindowPos(window->win32.handle,a,r.left,r.top,r.right-r.left,r.bottom-r.top,f);
        }
    }

    GLFWbool _glfwWindowFocusedWin32(_GLFWwindow* window) { return window->win32.handle==GetActiveWindow(); }
    GLFWbool _glfwWindowIconifiedWin32(_GLFWwindow* window) { return IsIconic(window->win32.handle); }
    GLFWbool _glfwWindowVisibleWin32(_GLFWwindow* window) { return IsWindowVisible(window->win32.handle); }
    GLFWbool _glfwWindowMaximizedWin32(_GLFWwindow* window) { return IsZoomed(window->win32.handle); }
    GLFWbool _glfwWindowHoveredWin32(_GLFWwindow* window) { return cursorInContentArea(window); }

    GLFWbool _glfwFramebufferTransparentWin32(_GLFWwindow* window) {
        BOOL composition,opaque; DWORD color;
        if (!window->win32.transparent) return GLFW_FALSE;
        if (FAILED(DwmIsCompositionEnabled(&composition))||!composition) return GLFW_FALSE;
        if (!IsWindows8OrGreater()&&(FAILED(DwmGetColorizationColor(&color,&opaque))||opaque)) return GLFW_FALSE;
        return GLFW_TRUE;
    }

    void _glfwSetWindowResizableWin32(_GLFWwindow* window,GLFWbool enabled) { (void)enabled; updateWindowStyles(window); }
    void _glfwSetWindowDecoratedWin32(_GLFWwindow* window,GLFWbool enabled) { (void)enabled; updateWindowStyles(window); }
    void _glfwSetWindowFloatingWin32(_GLFWwindow* window,GLFWbool enabled) { SetWindowPos(window->win32.handle,enabled?HWND_TOPMOST:HWND_NOTOPMOST,0,0,0,0,SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE); }

    void _glfwSetWindowMousePassthroughWin32(_GLFWwindow* window,GLFWbool enabled) {
        COLORREF key=0; BYTE alpha=0; DWORD flags=0;
        DWORD exStyle=GetWindowLongW(window->win32.handle,GWL_EXSTYLE);
        if (exStyle&WS_EX_LAYERED) GetLayeredWindowAttributes(window->win32.handle,&key,&alpha,&flags);
        if (enabled) exStyle|=(WS_EX_TRANSPARENT|WS_EX_LAYERED);
        else { exStyle&=~WS_EX_TRANSPARENT; if ((exStyle&WS_EX_LAYERED)&&!(flags&LWA_ALPHA)) exStyle&=~WS_EX_LAYERED; }
        SetWindowLongW(window->win32.handle,GWL_EXSTYLE,exStyle);
        if (enabled) SetLayeredWindowAttributes(window->win32.handle,key,alpha,flags);
    }

    float _glfwGetWindowOpacityWin32(_GLFWwindow* window) {
        BYTE alpha; DWORD flags;
        if ((GetWindowLongW(window->win32.handle,GWL_EXSTYLE)&WS_EX_LAYERED)&&GetLayeredWindowAttributes(window->win32.handle,NULL,&alpha,&flags)&&(flags&LWA_ALPHA)) return alpha/255.f;
        return 1.f;
    }

    void _glfwSetWindowOpacityWin32(_GLFWwindow* window,float opacity) {
        LONG exStyle=GetWindowLongW(window->win32.handle,GWL_EXSTYLE);
        if (opacity<1.f||(exStyle&WS_EX_TRANSPARENT)) { exStyle|=WS_EX_LAYERED; SetWindowLongW(window->win32.handle,GWL_EXSTYLE,exStyle); SetLayeredWindowAttributes(window->win32.handle,0,(BYTE)(255*opacity),LWA_ALPHA); }
        else if (exStyle&WS_EX_TRANSPARENT) SetLayeredWindowAttributes(window->win32.handle,0,0,0);
        else { exStyle&=~WS_EX_LAYERED; SetWindowLongW(window->win32.handle,GWL_EXSTYLE,exStyle); }
    }

    void _glfwSetRawMouseMotionWin32(_GLFWwindow* window,GLFWbool enabled) {
        if (_glfw.win32.disabledCursorWindow!=window) return;
        if (enabled) enableRawMouseMotion(window); else disableRawMouseMotion(window);
    }

    GLFWbool _glfwRawMouseMotionSupportedWin32(void) { return GLFW_TRUE; }

    void _glfwPollEventsWin32(void) {
        MSG msg; HWND handle; _GLFWwindow* window;
        while (PeekMessageW(&msg,NULL,0,0,PM_REMOVE)) {
            if (msg.message==WM_QUIT) { window=_glfw.windowListHead; while (window) { _glfwInputWindowCloseRequest(window); window=window->next; } }
            else { TranslateMessage(&msg); DispatchMessageW(&msg); }
        }
        handle=GetActiveWindow();
        if (handle) {
            window=GetPropW(handle,L"GLFW");
            if (window) {
                int i;
                const int keys[4][2]={{VK_LSHIFT,GLFW_KEY_LEFT_SHIFT},{VK_RSHIFT,GLFW_KEY_RIGHT_SHIFT},{VK_LWIN,GLFW_KEY_LEFT_SUPER},{VK_RWIN,GLFW_KEY_RIGHT_SUPER}};
                for (i=0;i<4;i++) {
                    const int vk=keys[i][0],key=keys[i][1],scancode=_glfw.win32.scancodes[key];
                    if ((GetKeyState(vk)&0x8000)||window->keys[key]!=GLFW_PRESS) continue;
                    _glfwInputKey(window,key,scancode,GLFW_RELEASE,getKeyMods());
                }
            }
        }
        window=_glfw.win32.disabledCursorWindow;
        if (window) {
            int width,height; _glfwGetWindowSizeWin32(window,&width,&height);
            if (window->win32.lastCursorPosX!=width/2||window->win32.lastCursorPosY!=height/2) _glfwSetCursorPosWin32(window,width/2,height/2);
        }
    }

    void _glfwWaitEventsWin32(void) { WaitMessage(); _glfwPollEventsWin32(); }
    void _glfwWaitEventsTimeoutWin32(double timeout) { MsgWaitForMultipleObjects(0,NULL,FALSE,(DWORD)(timeout*1e3),QS_ALLINPUT); _glfwPollEventsWin32(); }
    void _glfwPostEmptyEventWin32(void) { PostMessageW(_glfw.win32.helperWindowHandle,WM_NULL,0,0); }

    void _glfwGetCursorPosWin32(_GLFWwindow* window,double* xpos,double* ypos) {
        POINT pos;
        if (GetCursorPos(&pos)) { ScreenToClient(window->win32.handle,&pos); if (xpos) *xpos=pos.x; if (ypos) *ypos=pos.y; }
    }

    void _glfwSetCursorPosWin32(_GLFWwindow* window,double xpos,double ypos) {
        POINT pos={(int)xpos,(int)ypos};
        window->win32.lastCursorPosX=pos.x; window->win32.lastCursorPosY=pos.y;
        ClientToScreen(window->win32.handle,&pos); SetCursorPos(pos.x,pos.y);
    }

    void _glfwSetCursorModeWin32(_GLFWwindow* window,int mode) {
        if (_glfwWindowFocusedWin32(window)) {
            if (mode==GLFW_CURSOR_DISABLED) { _glfwGetCursorPosWin32(window,&_glfw.win32.restoreCursorPosX,&_glfw.win32.restoreCursorPosY); _glfwCenterCursorInContentArea(window); if (window->rawMouseMotion) enableRawMouseMotion(window); }
            else if (_glfw.win32.disabledCursorWindow==window) { if (window->rawMouseMotion) disableRawMouseMotion(window); }
            if (mode==GLFW_CURSOR_DISABLED||mode==GLFW_CURSOR_CAPTURED) captureCursor(window); else releaseCursor();
            if (mode==GLFW_CURSOR_DISABLED) _glfw.win32.disabledCursorWindow=window;
            else if (_glfw.win32.disabledCursorWindow==window) { _glfw.win32.disabledCursorWindow=NULL; _glfwSetCursorPosWin32(window,_glfw.win32.restoreCursorPosX,_glfw.win32.restoreCursorPosY); }
        }
        if (cursorInContentArea(window)) updateCursorImage(window);
    }

    GLFWbool _glfwCreateCursorWin32(_GLFWcursor* cursor,const GLFWimage* image,int xhot,int yhot) {
        cursor->win32.handle=(HCURSOR)createIcon(image,xhot,yhot,GLFW_FALSE);
        return cursor->win32.handle ? GLFW_TRUE : GLFW_FALSE;
    }

    GLFWbool _glfwCreateStandardCursorWin32(_GLFWcursor* cursor,int shape) {
        cursor->win32.handle=LoadImageW(NULL,MAKEINTRESOURCEW(OCR_NORMAL),IMAGE_CURSOR,0,0,LR_DEFAULTSIZE|LR_SHARED); (void)shape;
        if (!cursor->win32.handle) { _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,"Win32: Failed to create standard cursor"); return GLFW_FALSE; }
        return GLFW_TRUE;
    }

    void _glfwDestroyCursorWin32(_GLFWcursor* cursor) { if (cursor->win32.handle) DestroyIcon((HICON)cursor->win32.handle); }
    void _glfwSetCursorWin32(_GLFWwindow* window,_GLFWcursor* cursor) { (void)cursor; if (cursorInContentArea(window)) updateCursorImage(window); }
    static const GUID _glfw_GUID_DEVINTERFACE_HID = {0x4d1e55b2,0xf16f,0x11cf,{0x88,0xcb,0x00,0x11,0x11,0x00,0x00,0x30}};

    #define GUID_DEVINTERFACE_HID _glfw_GUID_DEVINTERFACE_HID
    #if defined(_GLFW_USE_HYBRID_HPG) || defined(_GLFW_USE_OPTIMUS_HPG)
        #if defined(_GLFW_BUILD_DLL)
            #pragma message("These symbols must be exported by the executable and have no effect in a DLL")
        #endif
        __declspec(dllexport) DWORD NvOptimusEnablement = 1;
        __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
    #endif // _GLFW_USE_HYBRID_HPG

    #if defined(_GLFW_BUILD_DLL)
        BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved) { return TRUE; }
    #endif // _GLFW_BUILD_DLL

    void* _glfwPlatformLoadModule(const char* path) { return LoadLibraryA(path); }
    void _glfwPlatformFreeModule(void* module) { if (module) {FreeLibrary((HMODULE)module);} }
    GLFWproc _glfwPlatformGetModuleSymbol(void* module, const char* name) { return (GLFWproc)GetProcAddress((HMODULE)module,name); }
    static GLFWbool loadLibraries(void) {
        if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                    GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                (const WCHAR*) &_glfw,
                                (HMODULE*) &_glfw.win32.instance))
        {
            _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
                                "Win32: Failed to retrieve own module handle");
            return GLFW_FALSE;
        }

        _glfw.win32.user32.instance = _glfwPlatformLoadModule("user32.dll");
        if (!_glfw.win32.user32.instance)
        {
            _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
                                "Win32: Failed to load user32.dll");
            return GLFW_FALSE;
        }

        _glfw.win32.user32.EnableNonClientDpiScaling_ = (PFN_EnableNonClientDpiScaling)
            _glfwPlatformGetModuleSymbol(_glfw.win32.user32.instance, "EnableNonClientDpiScaling");
        _glfw.win32.user32.SetProcessDpiAwarenessContext_ = (PFN_SetProcessDpiAwarenessContext)
            _glfwPlatformGetModuleSymbol(_glfw.win32.user32.instance, "SetProcessDpiAwarenessContext");
        _glfw.win32.user32.GetDpiForWindow_ = (PFN_GetDpiForWindow)
            _glfwPlatformGetModuleSymbol(_glfw.win32.user32.instance, "GetDpiForWindow");
        _glfw.win32.user32.AdjustWindowRectExForDpi_ = (PFN_AdjustWindowRectExForDpi)
            _glfwPlatformGetModuleSymbol(_glfw.win32.user32.instance, "AdjustWindowRectExForDpi");
        _glfw.win32.user32.GetSystemMetricsForDpi_ = (PFN_GetSystemMetricsForDpi)
            _glfwPlatformGetModuleSymbol(_glfw.win32.user32.instance, "GetSystemMetricsForDpi");

        _glfw.win32.dinput8.instance = _glfwPlatformLoadModule("dinput8.dll");
        if (_glfw.win32.dinput8.instance)
        {
            _glfw.win32.dinput8.Create = (PFN_DirectInput8Create)
                _glfwPlatformGetModuleSymbol(_glfw.win32.dinput8.instance, "DirectInput8Create");
        }

        {
            int i;
            const char* names[] =
            {
                "xinput1_4.dll",
                "xinput1_3.dll",
                "xinput9_1_0.dll",
                "xinput1_2.dll",
                "xinput1_1.dll",
                NULL
            };

            for (i = 0;  names[i];  i++)
            {
                _glfw.win32.xinput.instance = _glfwPlatformLoadModule(names[i]);
                if (_glfw.win32.xinput.instance)
                {
                    _glfw.win32.xinput.GetCapabilities = (PFN_XInputGetCapabilities)
                        _glfwPlatformGetModuleSymbol(_glfw.win32.xinput.instance, "XInputGetCapabilities");
                    _glfw.win32.xinput.GetState = (PFN_XInputGetState)
                        _glfwPlatformGetModuleSymbol(_glfw.win32.xinput.instance, "XInputGetState");

                    break;
                }
            }
        }

        _glfw.win32.dwmapi.instance = _glfwPlatformLoadModule("dwmapi.dll");
        if (_glfw.win32.dwmapi.instance)
        {
            _glfw.win32.dwmapi.IsCompositionEnabled = (PFN_DwmIsCompositionEnabled)
                _glfwPlatformGetModuleSymbol(_glfw.win32.dwmapi.instance, "DwmIsCompositionEnabled");
            _glfw.win32.dwmapi.Flush = (PFN_DwmFlush)
                _glfwPlatformGetModuleSymbol(_glfw.win32.dwmapi.instance, "DwmFlush");
            _glfw.win32.dwmapi.EnableBlurBehindWindow = (PFN_DwmEnableBlurBehindWindow)
                _glfwPlatformGetModuleSymbol(_glfw.win32.dwmapi.instance, "DwmEnableBlurBehindWindow");
            _glfw.win32.dwmapi.GetColorizationColor = (PFN_DwmGetColorizationColor)
                _glfwPlatformGetModuleSymbol(_glfw.win32.dwmapi.instance, "DwmGetColorizationColor");
        }

        _glfw.win32.shcore.instance = _glfwPlatformLoadModule("shcore.dll");
        if (_glfw.win32.shcore.instance)
        {
            _glfw.win32.shcore.SetProcessDpiAwareness_ = (PFN_SetProcessDpiAwareness)
                _glfwPlatformGetModuleSymbol(_glfw.win32.shcore.instance, "SetProcessDpiAwareness");
            _glfw.win32.shcore.GetDpiForMonitor_ = (PFN_GetDpiForMonitor)
                _glfwPlatformGetModuleSymbol(_glfw.win32.shcore.instance, "GetDpiForMonitor");
        }

        _glfw.win32.ntdll.instance = _glfwPlatformLoadModule("ntdll.dll");
        if (_glfw.win32.ntdll.instance)
        {
            _glfw.win32.ntdll.RtlVerifyVersionInfo_ = (PFN_RtlVerifyVersionInfo)
                _glfwPlatformGetModuleSymbol(_glfw.win32.ntdll.instance, "RtlVerifyVersionInfo");
        }

        return GLFW_TRUE;
    }

    static void createKeyTables(void) {
        int scancode;
        memset(_glfw.win32.keycodes, -1, sizeof(_glfw.win32.keycodes));
        memset(_glfw.win32.scancodes, -1, sizeof(_glfw.win32.scancodes));
        _glfw.win32.keycodes[0x00B] = GLFW_KEY_0;
        _glfw.win32.keycodes[0x002] = GLFW_KEY_1;
        _glfw.win32.keycodes[0x003] = GLFW_KEY_2;
        _glfw.win32.keycodes[0x004] = GLFW_KEY_3;
        _glfw.win32.keycodes[0x005] = GLFW_KEY_4;
        _glfw.win32.keycodes[0x006] = GLFW_KEY_5;
        _glfw.win32.keycodes[0x007] = GLFW_KEY_6;
        _glfw.win32.keycodes[0x008] = GLFW_KEY_7;
        _glfw.win32.keycodes[0x009] = GLFW_KEY_8;
        _glfw.win32.keycodes[0x00A] = GLFW_KEY_9;
        _glfw.win32.keycodes[0x01E] = GLFW_KEY_A;
        _glfw.win32.keycodes[0x030] = GLFW_KEY_B;
        _glfw.win32.keycodes[0x02E] = GLFW_KEY_C;
        _glfw.win32.keycodes[0x020] = GLFW_KEY_D;
        _glfw.win32.keycodes[0x012] = GLFW_KEY_E;
        _glfw.win32.keycodes[0x021] = GLFW_KEY_F;
        _glfw.win32.keycodes[0x022] = GLFW_KEY_G;
        _glfw.win32.keycodes[0x023] = GLFW_KEY_H;
        _glfw.win32.keycodes[0x017] = GLFW_KEY_I;
        _glfw.win32.keycodes[0x024] = GLFW_KEY_J;
        _glfw.win32.keycodes[0x025] = GLFW_KEY_K;
        _glfw.win32.keycodes[0x026] = GLFW_KEY_L;
        _glfw.win32.keycodes[0x032] = GLFW_KEY_M;
        _glfw.win32.keycodes[0x031] = GLFW_KEY_N;
        _glfw.win32.keycodes[0x018] = GLFW_KEY_O;
        _glfw.win32.keycodes[0x019] = GLFW_KEY_P;
        _glfw.win32.keycodes[0x010] = GLFW_KEY_Q;
        _glfw.win32.keycodes[0x013] = GLFW_KEY_R;
        _glfw.win32.keycodes[0x01F] = GLFW_KEY_S;
        _glfw.win32.keycodes[0x014] = GLFW_KEY_T;
        _glfw.win32.keycodes[0x016] = GLFW_KEY_U;
        _glfw.win32.keycodes[0x02F] = GLFW_KEY_V;
        _glfw.win32.keycodes[0x011] = GLFW_KEY_W;
        _glfw.win32.keycodes[0x02D] = GLFW_KEY_X;
        _glfw.win32.keycodes[0x015] = GLFW_KEY_Y;
        _glfw.win32.keycodes[0x02C] = GLFW_KEY_Z;
        _glfw.win32.keycodes[0x028] = GLFW_KEY_APOSTROPHE;
        _glfw.win32.keycodes[0x02B] = GLFW_KEY_BACKSLASH;
        _glfw.win32.keycodes[0x033] = GLFW_KEY_COMMA;
        _glfw.win32.keycodes[0x00D] = GLFW_KEY_EQUAL;
        _glfw.win32.keycodes[0x029] = GLFW_KEY_GRAVE_ACCENT;
        _glfw.win32.keycodes[0x01A] = GLFW_KEY_LEFT_BRACKET;
        _glfw.win32.keycodes[0x00C] = GLFW_KEY_MINUS;
        _glfw.win32.keycodes[0x034] = GLFW_KEY_PERIOD;
        _glfw.win32.keycodes[0x01B] = GLFW_KEY_RIGHT_BRACKET;
        _glfw.win32.keycodes[0x027] = GLFW_KEY_SEMICOLON;
        _glfw.win32.keycodes[0x035] = GLFW_KEY_SLASH;
        _glfw.win32.keycodes[0x056] = GLFW_KEY_WORLD_2;
        _glfw.win32.keycodes[0x00E] = GLFW_KEY_BACKSPACE;
        _glfw.win32.keycodes[0x153] = GLFW_KEY_DELETE;
        _glfw.win32.keycodes[0x14F] = GLFW_KEY_END;
        _glfw.win32.keycodes[0x01C] = GLFW_KEY_ENTER;
        _glfw.win32.keycodes[0x001] = GLFW_KEY_ESCAPE;
        _glfw.win32.keycodes[0x147] = GLFW_KEY_HOME;
        _glfw.win32.keycodes[0x152] = GLFW_KEY_INSERT;
        _glfw.win32.keycodes[0x15D] = GLFW_KEY_MENU;
        _glfw.win32.keycodes[0x151] = GLFW_KEY_PAGE_DOWN;
        _glfw.win32.keycodes[0x149] = GLFW_KEY_PAGE_UP;
        _glfw.win32.keycodes[0x045] = GLFW_KEY_PAUSE;
        _glfw.win32.keycodes[0x039] = GLFW_KEY_SPACE;
        _glfw.win32.keycodes[0x00F] = GLFW_KEY_TAB;
        _glfw.win32.keycodes[0x03A] = GLFW_KEY_CAPS_LOCK;
        _glfw.win32.keycodes[0x145] = GLFW_KEY_NUM_LOCK;
        _glfw.win32.keycodes[0x046] = GLFW_KEY_SCROLL_LOCK;
        _glfw.win32.keycodes[0x03B] = GLFW_KEY_F1;
        _glfw.win32.keycodes[0x03C] = GLFW_KEY_F2;
        _glfw.win32.keycodes[0x03D] = GLFW_KEY_F3;
        _glfw.win32.keycodes[0x03E] = GLFW_KEY_F4;
        _glfw.win32.keycodes[0x03F] = GLFW_KEY_F5;
        _glfw.win32.keycodes[0x040] = GLFW_KEY_F6;
        _glfw.win32.keycodes[0x041] = GLFW_KEY_F7;
        _glfw.win32.keycodes[0x042] = GLFW_KEY_F8;
        _glfw.win32.keycodes[0x043] = GLFW_KEY_F9;
        _glfw.win32.keycodes[0x044] = GLFW_KEY_F10;
        _glfw.win32.keycodes[0x057] = GLFW_KEY_F11;
        _glfw.win32.keycodes[0x058] = GLFW_KEY_F12;
        _glfw.win32.keycodes[0x064] = GLFW_KEY_F13;
        _glfw.win32.keycodes[0x065] = GLFW_KEY_F14;
        _glfw.win32.keycodes[0x066] = GLFW_KEY_F15;
        _glfw.win32.keycodes[0x067] = GLFW_KEY_F16;
        _glfw.win32.keycodes[0x068] = GLFW_KEY_F17;
        _glfw.win32.keycodes[0x069] = GLFW_KEY_F18;
        _glfw.win32.keycodes[0x06A] = GLFW_KEY_F19;
        _glfw.win32.keycodes[0x06B] = GLFW_KEY_F20;
        _glfw.win32.keycodes[0x06C] = GLFW_KEY_F21;
        _glfw.win32.keycodes[0x06D] = GLFW_KEY_F22;
        _glfw.win32.keycodes[0x06E] = GLFW_KEY_F23;
        _glfw.win32.keycodes[0x076] = GLFW_KEY_F24;
        _glfw.win32.keycodes[0x038] = GLFW_KEY_LEFT_ALT;
        _glfw.win32.keycodes[0x01D] = GLFW_KEY_LEFT_CONTROL;
        _glfw.win32.keycodes[0x02A] = GLFW_KEY_LEFT_SHIFT;
        _glfw.win32.keycodes[0x15B] = GLFW_KEY_LEFT_SUPER;
        _glfw.win32.keycodes[0x137] = GLFW_KEY_PRINT_SCREEN;
        _glfw.win32.keycodes[0x138] = GLFW_KEY_RIGHT_ALT;
        _glfw.win32.keycodes[0x11D] = GLFW_KEY_RIGHT_CONTROL;
        _glfw.win32.keycodes[0x036] = GLFW_KEY_RIGHT_SHIFT;
        _glfw.win32.keycodes[0x15C] = GLFW_KEY_RIGHT_SUPER;
        _glfw.win32.keycodes[0x150] = GLFW_KEY_DOWN;
        _glfw.win32.keycodes[0x14B] = GLFW_KEY_LEFT;
        _glfw.win32.keycodes[0x14D] = GLFW_KEY_RIGHT;
        _glfw.win32.keycodes[0x148] = GLFW_KEY_UP;
        _glfw.win32.keycodes[0x052] = GLFW_KEY_KP_0;
        _glfw.win32.keycodes[0x04F] = GLFW_KEY_KP_1;
        _glfw.win32.keycodes[0x050] = GLFW_KEY_KP_2;
        _glfw.win32.keycodes[0x051] = GLFW_KEY_KP_3;
        _glfw.win32.keycodes[0x04B] = GLFW_KEY_KP_4;
        _glfw.win32.keycodes[0x04C] = GLFW_KEY_KP_5;
        _glfw.win32.keycodes[0x04D] = GLFW_KEY_KP_6;
        _glfw.win32.keycodes[0x047] = GLFW_KEY_KP_7;
        _glfw.win32.keycodes[0x048] = GLFW_KEY_KP_8;
        _glfw.win32.keycodes[0x049] = GLFW_KEY_KP_9;
        _glfw.win32.keycodes[0x04E] = GLFW_KEY_KP_ADD;
        _glfw.win32.keycodes[0x053] = GLFW_KEY_KP_DECIMAL;
        _glfw.win32.keycodes[0x135] = GLFW_KEY_KP_DIVIDE;
        _glfw.win32.keycodes[0x11C] = GLFW_KEY_KP_ENTER;
        _glfw.win32.keycodes[0x059] = GLFW_KEY_KP_EQUAL;
        _glfw.win32.keycodes[0x037] = GLFW_KEY_KP_MULTIPLY;
        _glfw.win32.keycodes[0x04A] = GLFW_KEY_KP_SUBTRACT;
        for (scancode = 0;  scancode < 512;  scancode++) {
            if (_glfw.win32.keycodes[scancode] > 0) _glfw.win32.scancodes[_glfw.win32.keycodes[scancode]] = scancode;
        }
    }

    static LRESULT CALLBACK helperWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        switch (uMsg) {
            case WM_DISPLAYCHANGE: _glfwPollMonitorsWin32(); break;
            case WM_DEVICECHANGE: {
                if (!_glfw.joysticksInitialized) break;
                
                if (wParam == DBT_DEVICEARRIVAL) {
                    DEV_BROADCAST_HDR* dbh = (DEV_BROADCAST_HDR*) lParam;
                    if (dbh && dbh->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) _glfwDetectJoystickConnectionWin32();
                } else if (wParam == DBT_DEVICEREMOVECOMPLETE) {
                    DEV_BROADCAST_HDR* dbh = (DEV_BROADCAST_HDR*) lParam;
                    if (dbh && dbh->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) _glfwDetectJoystickDisconnectionWin32();
                }

                break;
            }
        }

        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }

    static GLFWbool createHelperWindow(void) {
        MSG msg;
        WNDCLASSEXW wc={0}; wc.cbSize=sizeof(wc);
        wc.style         = CS_OWNDC;
        wc.lpfnWndProc   = (WNDPROC) helperWindowProc;
        wc.hInstance     = _glfw.win32.instance;
        wc.lpszClassName = L"GLFW3 Helper";
        _glfw.win32.helperWindowClass = RegisterClassExW(&wc);
        if (!_glfw.win32.helperWindowClass) { _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,"Win32: Failed to register helper window class"); return GLFW_FALSE; }

        _glfw.win32.helperWindowHandle = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW,(LPCWSTR)MAKEINTATOM(_glfw.win32.helperWindowClass),L"GLFW message window",WS_CLIPSIBLINGS|WS_CLIPCHILDREN,0,0,1,1,NULL,NULL,_glfw.win32.instance,NULL);
        if (!_glfw.win32.helperWindowHandle) { _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,"Win32: Failed to create helper window"); return GLFW_FALSE; }

        ShowWindow(_glfw.win32.helperWindowHandle, SW_HIDE);
        {
            DEV_BROADCAST_DEVICEINTERFACE_W dbi;
            ZeroMemory(&dbi, sizeof(dbi));
            dbi.dbcc_size = sizeof(dbi);
            dbi.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
            dbi.dbcc_classguid = GUID_DEVINTERFACE_HID;
            _glfw.win32.deviceNotificationHandle = RegisterDeviceNotificationW(_glfw.win32.helperWindowHandle,(DEV_BROADCAST_HDR*)&dbi,DEVICE_NOTIFY_WINDOW_HANDLE);
        }

        while (PeekMessageW(&msg, _glfw.win32.helperWindowHandle, 0, 0, PM_REMOVE)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
        return GLFW_TRUE;
    }

    WCHAR* _glfwCreateWideStringFromUTF8Win32(const char* source) {
        WCHAR* target; int count = MultiByteToWideChar(CP_UTF8,0,source,-1,NULL,0); if (!count) { _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,"Win32: Failed to convert string from UTF-8"); return NULL; }
        target = _glfw_calloc(count, sizeof(WCHAR)); if (!MultiByteToWideChar(CP_UTF8, 0, source, -1, target, count)) { _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,"Win32: Failed to convert string from UTF-8"); free(target); return NULL; }
        return target;
    }

    char* _glfwCreateUTF8FromWideStringWin32(const WCHAR* source) {
        int size = WideCharToMultiByte(CP_UTF8,0,source, -1,NULL,0,NULL,NULL); if (!size) { _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,"Win32: Failed to convert string to UTF-8"); return NULL; }
        char* target = _glfw_calloc(size, 1); if (!WideCharToMultiByte(CP_UTF8, 0, source, -1, target, size, NULL, NULL)) { _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,"Win32: Failed to convert string to UTF-8"); free(target); return NULL; }
        return target;
    }

    void _glfwInputErrorWin32(int error, const char* description) {
        WCHAR buffer[_GLFW_MESSAGE_SIZE] = L""; char message[_GLFW_MESSAGE_SIZE] = "";
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS|FORMAT_MESSAGE_MAX_WIDTH_MASK,
                    NULL,
                    GetLastError() & 0xffff,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    buffer,
                    sizeof(buffer) / sizeof(WCHAR),
                    NULL);
        WideCharToMultiByte(CP_UTF8, 0, buffer, -1, message, sizeof(message), NULL, NULL);

        _glfwInputError(error, "%s: %s", description, message);
    }

    // Updates key names according to the current keyboard layout
    //
    void _glfwUpdateKeyNamesWin32(void)
    {
        int key;
        BYTE state[256] = {0};

        memset(_glfw.win32.keynames, 0, sizeof(_glfw.win32.keynames));

        for (key = GLFW_KEY_SPACE;  key <= GLFW_KEY_LAST;  key++)
        {
            UINT vk;
            int scancode, length;
            WCHAR chars[16];

            scancode = _glfw.win32.scancodes[key];
            if (scancode == -1)
                continue;

            if (key >= GLFW_KEY_KP_0 && key <= GLFW_KEY_KP_ADD)
            {
                const UINT vks[] =
                {
                    VK_NUMPAD0,  VK_NUMPAD1,  VK_NUMPAD2, VK_NUMPAD3,
                    VK_NUMPAD4,  VK_NUMPAD5,  VK_NUMPAD6, VK_NUMPAD7,
                    VK_NUMPAD8,  VK_NUMPAD9,  VK_DECIMAL, VK_DIVIDE,
                    VK_MULTIPLY, VK_SUBTRACT, VK_ADD
                };

                vk = vks[key - GLFW_KEY_KP_0];
            }
            else
                vk = MapVirtualKeyW(scancode, MAPVK_VSC_TO_VK);

            length = ToUnicode(vk, scancode, state,
                            chars, sizeof(chars) / sizeof(WCHAR),
                            0);

            if (length == -1)
            {
                // This is a dead key, so we need a second simulated key press
                // to make it output its own character (usually a diacritic)
                length = ToUnicode(vk, scancode, state,
                                chars, sizeof(chars) / sizeof(WCHAR),
                                0);
            }

            if (length < 1)
                continue;

            WideCharToMultiByte(CP_UTF8, 0, chars, 1,
                                _glfw.win32.keynames[key],
                                sizeof(_glfw.win32.keynames[key]),
                                NULL, NULL);
        }
    }

    BOOL _glfwIsWindowsVersionOrGreaterWin32(WORD major,WORD minor,WORD sp) {
        OSVERSIONINFOEXW osvi={0}; osvi.dwOSVersionInfoSize=sizeof(osvi), osvi.dwMajorVersion=major, osvi.dwMinorVersion=minor, osvi.wServicePackMajor=sp;
        DWORD mask=VER_MAJORVERSION|VER_MINORVERSION|VER_SERVICEPACKMAJOR;
        ULONGLONG cond=VerSetConditionMask(VerSetConditionMask(VerSetConditionMask(0,VER_MAJORVERSION,VER_GREATER_EQUAL),VER_MINORVERSION,VER_GREATER_EQUAL),VER_SERVICEPACKMAJOR,VER_GREATER_EQUAL);
        return RtlVerifyVersionInfo(&osvi,mask,cond)==0;
    }

    BOOL _glfwIsWindows10BuildOrGreaterWin32(WORD build) {
        OSVERSIONINFOEXW osvi={0}; osvi.dwOSVersionInfoSize=sizeof(osvi), osvi.dwMajorVersion=10, osvi.dwBuildNumber=build;
        DWORD mask=VER_MAJORVERSION|VER_MINORVERSION|VER_BUILDNUMBER;
        ULONGLONG cond=VerSetConditionMask(VerSetConditionMask(VerSetConditionMask(0,VER_MAJORVERSION,VER_GREATER_EQUAL),VER_MINORVERSION,VER_GREATER_EQUAL),VER_BUILDNUMBER,VER_GREATER_EQUAL);
        return RtlVerifyVersionInfo(&osvi,mask,cond)==0;
    }

    int _glfwInitWin32(void) {
        if (!loadLibraries()) return GLFW_FALSE;

        createKeyTables();
        _glfwUpdateKeyNamesWin32();
        if (_glfwIsWindows10Version1703OrGreaterWin32()) SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        else if (IsWindows8Point1OrGreater()) SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
        else SetProcessDPIAware();

        if (!createHelperWindow()) return GLFW_FALSE;

        _glfwPollMonitorsWin32();
        return GLFW_TRUE;
    }

    #define _GLFW_TYPE_AXIS     0
    #define _GLFW_TYPE_SLIDER   1
    #define _GLFW_TYPE_BUTTON   2
    #define _GLFW_TYPE_POV      3
    typedef struct _GLFWobjenumWin32 { IDirectInputDevice8W* device; _GLFWjoyobjectWin32* objects; int objectCount,axisCount,sliderCount,buttonCount,povCount; } _GLFWobjenumWin32;
    static const GUID _glfw_IID_IDirectInput8W = {0xbf798031,0x483a,0x4da2,{0xaa,0x99,0x5d,0x64,0xed,0x36,0x97,0x00}};
    static const GUID _glfw_GUID_XAxis = {0xa36d02e0,0xc9f3,0x11cf,{0xbf,0xc7,0x44,0x45,0x53,0x54,0x00,0x00}};
    static const GUID _glfw_GUID_YAxis = {0xa36d02e1,0xc9f3,0x11cf,{0xbf,0xc7,0x44,0x45,0x53,0x54,0x00,0x00}};
    static const GUID _glfw_GUID_ZAxis = {0xa36d02e2,0xc9f3,0x11cf,{0xbf,0xc7,0x44,0x45,0x53,0x54,0x00,0x00}};
    static const GUID _glfw_GUID_RxAxis = {0xa36d02f4,0xc9f3,0x11cf,{0xbf,0xc7,0x44,0x45,0x53,0x54,0x00,0x00}};
    static const GUID _glfw_GUID_RyAxis = {0xa36d02f5,0xc9f3,0x11cf,{0xbf,0xc7,0x44,0x45,0x53,0x54,0x00,0x00}};
    static const GUID _glfw_GUID_RzAxis = {0xa36d02e3,0xc9f3,0x11cf,{0xbf,0xc7,0x44,0x45,0x53,0x54,0x00,0x00}};
    static const GUID _glfw_GUID_Slider = {0xa36d02e4,0xc9f3,0x11cf,{0xbf,0xc7,0x44,0x45,0x53,0x54,0x00,0x00}};
    static const GUID _glfw_GUID_POV = {0xa36d02f2,0xc9f3,0x11cf,{0xbf,0xc7,0x44,0x45,0x53,0x54,0x00,0x00}};
    #define IID_IDirectInput8W _glfw_IID_IDirectInput8W
    #define GUID_XAxis _glfw_GUID_XAxis
    #define GUID_YAxis _glfw_GUID_YAxis
    #define GUID_ZAxis _glfw_GUID_ZAxis
    #define GUID_RxAxis _glfw_GUID_RxAxis
    #define GUID_RyAxis _glfw_GUID_RyAxis
    #define GUID_RzAxis _glfw_GUID_RzAxis
    #define GUID_Slider _glfw_GUID_Slider
    #define GUID_POV _glfw_GUID_POV
    static DIOBJECTDATAFORMAT _glfwObjectDataFormats[] = {
        { &GUID_XAxis,DIJOFS_X,DIDFT_AXIS|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,DIDOI_ASPECTPOSITION },
        { &GUID_YAxis,DIJOFS_Y,DIDFT_AXIS|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,DIDOI_ASPECTPOSITION },
        { &GUID_ZAxis,DIJOFS_Z,DIDFT_AXIS|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,DIDOI_ASPECTPOSITION },
        { &GUID_RxAxis,DIJOFS_RX,DIDFT_AXIS|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,DIDOI_ASPECTPOSITION },
        { &GUID_RyAxis,DIJOFS_RY,DIDFT_AXIS|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,DIDOI_ASPECTPOSITION },
        { &GUID_RzAxis,DIJOFS_RZ,DIDFT_AXIS|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,DIDOI_ASPECTPOSITION },
        { &GUID_Slider,DIJOFS_SLIDER(0),DIDFT_AXIS|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,DIDOI_ASPECTPOSITION },
        { &GUID_Slider,DIJOFS_SLIDER(1),DIDFT_AXIS|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,DIDOI_ASPECTPOSITION },
        { &GUID_POV,DIJOFS_POV(0),DIDFT_POV|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
        { &GUID_POV,DIJOFS_POV(1),DIDFT_POV|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
        { &GUID_POV,DIJOFS_POV(2),DIDFT_POV|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
        { &GUID_POV,DIJOFS_POV(3),DIDFT_POV|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
        { NULL,DIJOFS_BUTTON(0),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
        { NULL,DIJOFS_BUTTON(1),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
        { NULL,DIJOFS_BUTTON(2),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
        { NULL,DIJOFS_BUTTON(3),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
        { NULL,DIJOFS_BUTTON(4),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
        { NULL,DIJOFS_BUTTON(5),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
        { NULL,DIJOFS_BUTTON(6),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
        { NULL,DIJOFS_BUTTON(7),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
        { NULL,DIJOFS_BUTTON(8),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
        { NULL,DIJOFS_BUTTON(9),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
        { NULL,DIJOFS_BUTTON(10),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
        { NULL,DIJOFS_BUTTON(11),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
        { NULL,DIJOFS_BUTTON(12),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
        { NULL,DIJOFS_BUTTON(13),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
        { NULL,DIJOFS_BUTTON(14),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
        { NULL,DIJOFS_BUTTON(15),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
        { NULL,DIJOFS_BUTTON(16),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
        { NULL,DIJOFS_BUTTON(17),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
        { NULL,DIJOFS_BUTTON(18),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
        { NULL,DIJOFS_BUTTON(19),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
        { NULL,DIJOFS_BUTTON(20),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
        { NULL,DIJOFS_BUTTON(21),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
        { NULL,DIJOFS_BUTTON(22),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
        { NULL,DIJOFS_BUTTON(23),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
        { NULL,DIJOFS_BUTTON(24),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
        { NULL,DIJOFS_BUTTON(25),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
        { NULL,DIJOFS_BUTTON(26),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
        { NULL,DIJOFS_BUTTON(27),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
        { NULL,DIJOFS_BUTTON(28),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
        { NULL,DIJOFS_BUTTON(29),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
        { NULL,DIJOFS_BUTTON(30),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
        { NULL,DIJOFS_BUTTON(31),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    };

    static const DIDATAFORMAT _glfwDataFormat = {sizeof(DIDATAFORMAT),sizeof(DIOBJECTDATAFORMAT),DIDFT_ABSAXIS,sizeof(DIJOYSTATE),sizeof(_glfwObjectDataFormats) / sizeof(DIOBJECTDATAFORMAT),_glfwObjectDataFormats };
    static const char* getDeviceDescription(const XINPUT_CAPABILITIES* xic) {
        switch (xic->SubType) {
            case 0x02: return "XInput Wheel";
            case 0x03: return "XInput Arcade Stick";
            case 0x04: return "XInput Flight Stick";
            case XINPUT_DEVSUBTYPE_GAMEPAD: {
                if (xic->Flags & XINPUT_CAPS_WIRELESS) return "Wireless Xbox Controller";
                else return "Xbox Controller";
            }
        }

        return "Unknown XInput Device";
    }

    static int compareJoystickObjects(const void* first, const void* second) {
        const _GLFWjoyobjectWin32* fo = first; const _GLFWjoyobjectWin32* so = second;
        if (fo->type != so->type) return fo->type - so->type;
        return fo->offset - so->offset;
    }

    static GLFWbool supportsXInput(const GUID* guid) {
        UINT i, count = 0;
        RAWINPUTDEVICELIST* ridl;
        GLFWbool result = GLFW_FALSE;
        if (GetRawInputDeviceList(NULL, &count, sizeof(RAWINPUTDEVICELIST)) != 0) return GLFW_FALSE;

        ridl = _glfw_calloc(count, sizeof(RAWINPUTDEVICELIST));
        if (GetRawInputDeviceList(ridl, &count, sizeof(RAWINPUTDEVICELIST)) == (UINT) -1) { free(ridl); return GLFW_FALSE; }

        for (i = 0;  i < count;  i++) {
            RID_DEVICE_INFO rdi; char name[256]; UINT size;
            if (ridl[i].dwType != RIM_TYPEHID) continue;

            ZeroMemory(&rdi, sizeof(rdi)); rdi.cbSize = sizeof(rdi); size = sizeof(rdi);
            if ((INT) GetRawInputDeviceInfoA(ridl[i].hDevice,RIDI_DEVICEINFO,&rdi, &size) == -1) continue;
            if (MAKELONG(rdi.hid.dwVendorId, rdi.hid.dwProductId) != (LONG) guid->Data1) continue;

            memset(name, 0, sizeof(name)); size = sizeof(name);
            if ((INT) GetRawInputDeviceInfoA(ridl[i].hDevice,RIDI_DEVICENAME,name,&size) == -1) break;

            name[sizeof(name) - 1] = '\0';
            if (strstr(name, "IG_")) { result = GLFW_TRUE; break; }
        }

        free(ridl);
        return result;
    }

    static void closeJoystick(_GLFWjoystick* js) {
        _glfwInputJoystick(js,GLFW_DISCONNECTED);
        if (js->win32.device) { IDirectInputDevice8_Unacquire(js->win32.device); IDirectInputDevice8_Release(js->win32.device); }
        free(js->win32.objects);
        _glfwFreeJoystick(js);
    }

    static BOOL CALLBACK deviceObjectCallback(const DIDEVICEOBJECTINSTANCEW* doi, void* user) {
        _GLFWobjenumWin32* data = user;
        _GLFWjoyobjectWin32* object = data->objects + data->objectCount;
        if (DIDFT_GETTYPE(doi->dwType) & DIDFT_AXIS) {
            DIPROPRANGE dipr;
            if (memcmp(&doi->guidType, &GUID_Slider, sizeof(GUID)) == 0) object->offset = DIJOFS_SLIDER(data->sliderCount);
            else if (memcmp(&doi->guidType, &GUID_XAxis, sizeof(GUID)) == 0) object->offset = DIJOFS_X;
            else if (memcmp(&doi->guidType, &GUID_YAxis, sizeof(GUID)) == 0) object->offset = DIJOFS_Y;
            else if (memcmp(&doi->guidType, &GUID_ZAxis, sizeof(GUID)) == 0) object->offset = DIJOFS_Z;
            else if (memcmp(&doi->guidType, &GUID_RxAxis, sizeof(GUID)) == 0) object->offset = DIJOFS_RX;
            else if (memcmp(&doi->guidType, &GUID_RyAxis, sizeof(GUID)) == 0) object->offset = DIJOFS_RY;
            else if (memcmp(&doi->guidType, &GUID_RzAxis, sizeof(GUID)) == 0) object->offset = DIJOFS_RZ;
            else return DIENUM_CONTINUE;

            ZeroMemory(&dipr, sizeof(dipr)); dipr.diph.dwSize = sizeof(dipr); dipr.diph.dwHeaderSize = sizeof(dipr.diph);
            dipr.diph.dwObj = doi->dwType; dipr.diph.dwHow = DIPH_BYID; dipr.lMin = -32768; dipr.lMax =  32767;
            if (FAILED(IDirectInputDevice8_SetProperty(data->device,DIPROP_RANGE,&dipr.diph))) return DIENUM_CONTINUE;

            if (memcmp(&doi->guidType, &GUID_Slider, sizeof(GUID)) == 0) { object->type = _GLFW_TYPE_SLIDER; data->sliderCount++; }
            else { object->type = _GLFW_TYPE_AXIS; data->axisCount++; }
        } else if (DIDFT_GETTYPE(doi->dwType) & DIDFT_BUTTON) { object->offset = DIJOFS_BUTTON(data->buttonCount); object->type = _GLFW_TYPE_BUTTON; data->buttonCount++;
        } else if (DIDFT_GETTYPE(doi->dwType) & DIDFT_POV) { object->offset = DIJOFS_POV(data->povCount); object->type = _GLFW_TYPE_POV; data->povCount++; }
        
        data->objectCount++;
        return DIENUM_CONTINUE;
    }

    static BOOL CALLBACK deviceCallback(const DIDEVICEINSTANCEW* di,void* user) {
        int jid; DIDEVCAPS dc; DIPROPDWORD dipd; IDirectInputDevice8W* device; _GLFWobjenumWin32 data; _GLFWjoystick* js; char guid[33],name[256]; (void)user;
        for (jid=0;jid<=GLFW_JOYSTICK_LAST;jid++) {
            js=_glfw.joysticks+jid;
            if (js->connected && memcmp(&js->win32.guid,&di->guidInstance,sizeof(GUID))==0) return DIENUM_CONTINUE;
        }
        if (supportsXInput(&di->guidProduct)) return DIENUM_CONTINUE;
        if (FAILED(IDirectInput8_CreateDevice(_glfw.win32.dinput8.api,&di->guidInstance,&device,NULL))) return DIENUM_CONTINUE;
        if (FAILED(IDirectInputDevice8_SetDataFormat(device,&_glfwDataFormat))) { IDirectInputDevice8_Release(device); return DIENUM_CONTINUE; }
        
        memset(&dc,0,sizeof(dc)), dc.dwSize=sizeof(dc);
        if (FAILED(IDirectInputDevice8_GetCapabilities(device,&dc))) { IDirectInputDevice8_Release(device); return DIENUM_CONTINUE; }
        
        memset(&dipd,0,sizeof(dipd)), dipd.diph.dwSize=sizeof(dipd), dipd.diph.dwHeaderSize=sizeof(dipd.diph), dipd.diph.dwHow=DIPH_DEVICE, dipd.dwData=DIPROPAXISMODE_ABS;
        if (FAILED(IDirectInputDevice8_SetProperty(device,DIPROP_AXISMODE,&dipd.diph))) { IDirectInputDevice8_Release(device); return DIENUM_CONTINUE; }
        
        memset(&data,0,sizeof(data)), data.device=device, data.objects=_glfw_calloc(dc.dwAxes+(size_t)dc.dwButtons+dc.dwPOVs,sizeof(_GLFWjoyobjectWin32));
        if (FAILED(IDirectInputDevice8_EnumObjects(device,deviceObjectCallback,&data,DIDFT_AXIS|DIDFT_BUTTON|DIDFT_POV))) { IDirectInputDevice8_Release(device), free(data.objects); return DIENUM_CONTINUE; }
        
        qsort(data.objects,data.objectCount,sizeof(_GLFWjoyobjectWin32),compareJoystickObjects);
        if (!WideCharToMultiByte(CP_UTF8,0,di->tszInstanceName,-1,name,sizeof(name),NULL,NULL)) { IDirectInputDevice8_Release(device), free(data.objects); return DIENUM_STOP; }
        
        if (memcmp(&di->guidProduct.Data4[2],"PIDVID",6)==0) sprintf(guid,"03000000%02x%02x0000%02x%02x000000000000",(u8)di->guidProduct.Data1,(u8)(di->guidProduct.Data1>>8),(u8)(di->guidProduct.Data1>>16),(u8)(di->guidProduct.Data1>>24));
        else sprintf(guid,"05000000%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x00",name[0],name[1],name[2],name[3],name[4],name[5],name[6],name[7],name[8],name[9],name[10]);
        if (!(js=_glfwAllocJoystick(name,guid,data.axisCount+data.sliderCount,data.buttonCount,data.povCount))) { IDirectInputDevice8_Release(device), free(data.objects); return DIENUM_STOP; }
        
        js->win32.device=device, js->win32.guid=di->guidInstance, js->win32.objects=data.objects, js->win32.objectCount=data.objectCount;
        _glfwInputJoystick(js,GLFW_CONNECTED); return DIENUM_CONTINUE;
    }


    void _glfwDetectJoystickConnectionWin32(void) {
        if (_glfw.win32.xinput.instance) {
            DWORD index;
            for (index=0;index<4;index++) {
                int jid; char guid[33]; XINPUT_CAPABILITIES xic; _GLFWjoystick* js;
                for (jid = 0;  jid <= GLFW_JOYSTICK_LAST;  jid++) {
                    if (_glfw.joysticks[jid].connected && _glfw.joysticks[jid].win32.device == NULL && _glfw.joysticks[jid].win32.index == index) break;
                }

                if (jid <= GLFW_JOYSTICK_LAST) continue;
                if (XInputGetCapabilities(index, 0, &xic) != ERROR_SUCCESS) continue;

                sprintf(guid, "78696e707574%02x000000000000000000",xic.SubType & 0xff);
                js = _glfwAllocJoystick(getDeviceDescription(&xic), guid, 6, 10, 1);
                if (!js) continue;

                js->win32.index = index;
                _glfwInputJoystick(js, GLFW_CONNECTED);
            }
        }

        if (_glfw.win32.dinput8.api) {
            if (FAILED(IDirectInput8_EnumDevices(_glfw.win32.dinput8.api,DI8DEVCLASS_GAMECTRL,deviceCallback,NULL,DIEDFL_ALLDEVICES))) {_glfwInputError(GLFW_PLATFORM_ERROR,"Failed to enumerate DirectInput8 devices"); return; }
        }
    }

    GLFWbool _glfwInitJoysticksWin32(void) {
        if (_glfw.win32.dinput8.instance) {
            if (FAILED(DirectInput8Create(_glfw.win32.instance,0x0800,&IID_IDirectInput8W,(void**) &_glfw.win32.dinput8.api,NULL))) { _glfwInputError(GLFW_PLATFORM_ERROR,"Win32: Failed to create interface"); return GLFW_FALSE; }
        }

        _glfwDetectJoystickConnectionWin32();
        return GLFW_TRUE;
    }

    void _glfwTerminateJoysticksWin32(void) {
        for (int jid = GLFW_JOYSTICK_1;jid <= GLFW_JOYSTICK_LAST;++jid) closeJoystick(_glfw.joysticks + jid);
        if (_glfw.win32.dinput8.api) IDirectInput8_Release(_glfw.win32.dinput8.api);
    }

    GLFWbool _glfwPollJoystickWin32(_GLFWjoystick* js, int mode) {
        if (js->win32.device) {
            int i, ai = 0, bi = 0, pi = 0; HRESULT result; DIJOYSTATE state = {0};
            IDirectInputDevice8_Poll(js->win32.device);
            result = IDirectInputDevice8_GetDeviceState(js->win32.device,sizeof(state),&state);
            if (result == DIERR_NOTACQUIRED || result == DIERR_INPUTLOST) {
                IDirectInputDevice8_Acquire(js->win32.device);
                IDirectInputDevice8_Poll(js->win32.device);
                result = IDirectInputDevice8_GetDeviceState(js->win32.device,sizeof(state),&state);
            }

            if (FAILED(result)) { closeJoystick(js); return GLFW_FALSE; }
            if (mode == _GLFW_POLL_PRESENCE) return GLFW_TRUE;

            for (i = 0;  i < js->win32.objectCount;  i++) {
                const void* data = (char*) &state + js->win32.objects[i].offset;
                switch (js->win32.objects[i].type) {
                    case _GLFW_TYPE_AXIS:
                    case _GLFW_TYPE_SLIDER: {
                        const float value = (*((LONG*) data) + 0.5f) / 32767.5f;
                        _glfwInputJoystickAxis(js, ai, value);
                        ai++;
                        break;
                    }

                    case _GLFW_TYPE_BUTTON: {
                        const char value = (*((BYTE*) data) & 0x80) != 0;
                        _glfwInputJoystickButton(js, bi, value);
                        bi++;
                        break;
                    }

                    case _GLFW_TYPE_POV: {
                        const int states[9] = {GLFW_HAT_UP,GLFW_HAT_RIGHT_UP,GLFW_HAT_RIGHT,GLFW_HAT_RIGHT_DOWN,GLFW_HAT_DOWN,GLFW_HAT_LEFT_DOWN,GLFW_HAT_LEFT,GLFW_HAT_LEFT_UP,GLFW_HAT_CENTERED};

                        // Screams of horror are appropriate at this point
                        int stateIndex = LOWORD(*(DWORD*) data) / (45 * DI_DEGREES);
                        if (stateIndex < 0 || stateIndex > 8) stateIndex = 8;
                        _glfwInputJoystickHat(js, pi, states[stateIndex]);
                        pi++;
                        break;
                    }
                }
            }
        } else {
            int i, dpad = 0;
            DWORD result;
            XINPUT_STATE xis;
            const WORD buttons[10] = {XINPUT_GAMEPAD_A,XINPUT_GAMEPAD_B,XINPUT_GAMEPAD_X,XINPUT_GAMEPAD_Y,XINPUT_GAMEPAD_LEFT_SHOULDER,XINPUT_GAMEPAD_RIGHT_SHOULDER,XINPUT_GAMEPAD_BACK,XINPUT_GAMEPAD_START,XINPUT_GAMEPAD_LEFT_THUMB,XINPUT_GAMEPAD_RIGHT_THUMB};
            result = XInputGetState(js->win32.index, &xis);
            if (result != ERROR_SUCCESS) {
                if (result == ERROR_DEVICE_NOT_CONNECTED) closeJoystick(js);
                return GLFW_FALSE;
            }

            if (mode == _GLFW_POLL_PRESENCE) return GLFW_TRUE;

            _glfwInputJoystickAxis(js, 0, (xis.Gamepad.sThumbLX + 0.5f) / 32767.5f);
            _glfwInputJoystickAxis(js, 1, -(xis.Gamepad.sThumbLY + 0.5f) / 32767.5f);
            _glfwInputJoystickAxis(js, 2, (xis.Gamepad.sThumbRX + 0.5f) / 32767.5f);
            _glfwInputJoystickAxis(js, 3, -(xis.Gamepad.sThumbRY + 0.5f) / 32767.5f);
            _glfwInputJoystickAxis(js, 4, xis.Gamepad.bLeftTrigger / 127.5f - 1.f);
            _glfwInputJoystickAxis(js, 5, xis.Gamepad.bRightTrigger / 127.5f - 1.f);

            for (i = 0;  i < 10;  i++) {
                const char value = (xis.Gamepad.wButtons & buttons[i]) ? 1 : 0;
                _glfwInputJoystickButton(js, i, value);
            }

            if (xis.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) dpad |= GLFW_HAT_UP;
            if (xis.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) dpad |= GLFW_HAT_RIGHT;
            if (xis.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) dpad |= GLFW_HAT_DOWN;
            if (xis.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) dpad |= GLFW_HAT_LEFT;
            if ((dpad & GLFW_HAT_RIGHT) && (dpad & GLFW_HAT_LEFT)) dpad &= ~(GLFW_HAT_RIGHT | GLFW_HAT_LEFT);
            if ((dpad & GLFW_HAT_UP) && (dpad & GLFW_HAT_DOWN)) dpad &= ~(GLFW_HAT_UP | GLFW_HAT_DOWN);
            _glfwInputJoystickHat(js, 0, dpad);
        }

        return GLFW_TRUE;
    }
    
    void _glfwDetectJoystickDisconnectionWin32(void) {
        for (int jid = 0;  jid <= GLFW_JOYSTICK_LAST;  jid++) {
            _GLFWjoystick* js = _glfw.joysticks + jid;
            if (js->connected) _glfwPollJoystickWin32(js, _GLFW_POLL_PRESENCE);
        }
    }

    const char* _glfwGetMappingNameWin32(void) { return "Windows"; }
    void _glfwUpdateGamepadGUIDWin32(char* guid) {
        if (strcmp(guid + 20, "504944564944") == 0) {
            char original[33];
            strncpy(original,guid,sizeof(original) - 1);
            sprintf(guid,"03000000%.4s0000%.4s000000000000",original,original + 4);
        }
    }

    static BOOL CALLBACK monitorCallback(HMONITOR handle, HDC dc, RECT* rect, LPARAM data) {
        MONITORINFOEXW mi; (void)dc; (void)rect;
        ZeroMemory(&mi, sizeof(mi));
        mi.cbSize = sizeof(mi);

        if (GetMonitorInfoW(handle, (MONITORINFO*) &mi))
        {
            _GLFWmonitor* monitor = (_GLFWmonitor*) data;
            if (wcscmp(mi.szDevice, monitor->win32.adapterName) == 0)
                monitor->win32.handle = handle;
        }

        return TRUE;
    }

    static _GLFWmonitor* createMonitor(DISPLAY_DEVICEW* adapter, DISPLAY_DEVICEW* display) {
        _GLFWmonitor* monitor; int widthMM, heightMM; char* name; HDC dc; DEVMODEW dm; RECT rect;
        if (display) name = _glfwCreateUTF8FromWideStringWin32(display->DeviceString);
        else name = _glfwCreateUTF8FromWideStringWin32(adapter->DeviceString);
        if (!name) return NULL;

        ZeroMemory(&dm, sizeof(dm));
        dm.dmSize = sizeof(dm);
        EnumDisplaySettingsW(adapter->DeviceName, ENUM_CURRENT_SETTINGS, &dm);
        dc = CreateDCW(L"DISPLAY", adapter->DeviceName, NULL, NULL);
        if (IsWindows8Point1OrGreater()) { widthMM  = GetDeviceCaps(dc, HORZSIZE); heightMM = GetDeviceCaps(dc, VERTSIZE); }
        else { widthMM  = (int) (dm.dmPelsWidth * 25.4f / GetDeviceCaps(dc, LOGPIXELSX)); heightMM = (int) (dm.dmPelsHeight * 25.4f / GetDeviceCaps(dc, LOGPIXELSY)); }

        DeleteDC(dc);
        monitor = _glfwAllocMonitor(name,widthMM,heightMM);
        free(name);
        if (adapter->StateFlags & DISPLAY_DEVICE_MODESPRUNED) monitor->win32.modesPruned = GLFW_TRUE;
        wcscpy(monitor->win32.adapterName, adapter->DeviceName);
        WideCharToMultiByte(CP_UTF8,0,adapter->DeviceName,-1,monitor->win32.publicAdapterName,sizeof(monitor->win32.publicAdapterName),NULL,NULL);
        if (display) {
            wcscpy(monitor->win32.displayName, display->DeviceName);
            WideCharToMultiByte(CP_UTF8, 0,
                                display->DeviceName, -1,
                                monitor->win32.publicDisplayName,
                                sizeof(monitor->win32.publicDisplayName),
                                NULL, NULL);
        }

        rect.left   = dm.dmPosition.x;
        rect.top    = dm.dmPosition.y;
        rect.right  = dm.dmPosition.x + dm.dmPelsWidth;
        rect.bottom = dm.dmPosition.y + dm.dmPelsHeight;

        EnumDisplayMonitors(NULL, &rect, monitorCallback, (LPARAM) monitor);
        return monitor;
    }

    void _glfwPollMonitorsWin32(void) {
        int i, disconnectedCount;
        _GLFWmonitor** disconnected = NULL;
        DWORD adapterIndex, displayIndex;
        DISPLAY_DEVICEW adapter, display;
        _GLFWmonitor* monitor;
        disconnectedCount = _glfw.monitorCount;
        if (disconnectedCount) {
            disconnected = _glfw_calloc(_glfw.monitorCount, sizeof(_GLFWmonitor*));
            __builtin_memcpy(disconnected,_glfw.monitors,_glfw.monitorCount * sizeof(_GLFWmonitor*));
        }

        for (adapterIndex = 0;  ;  adapterIndex++) {
            int type = _GLFW_INSERT_LAST;
            ZeroMemory(&adapter, sizeof(adapter));
            adapter.cb = sizeof(adapter);
            if (!EnumDisplayDevicesW(NULL, adapterIndex, &adapter, 0)) break;
            if (!(adapter.StateFlags & DISPLAY_DEVICE_ACTIVE)) continue;

            if (adapter.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) type = _GLFW_INSERT_FIRST;
            for (displayIndex = 0;  ;  displayIndex++) {
                ZeroMemory(&display, sizeof(display));
                display.cb = sizeof(display);
                if (!EnumDisplayDevicesW(adapter.DeviceName, displayIndex, &display, 0)) break;
                if (!(display.StateFlags & DISPLAY_DEVICE_ACTIVE)) continue;

                for (i = 0;  i < disconnectedCount;  i++) {
                    if (disconnected[i] && wcscmp(disconnected[i]->win32.displayName,display.DeviceName) == 0) {
                        disconnected[i] = NULL;
                        EnumDisplayMonitors(NULL, NULL, monitorCallback, (LPARAM) _glfw.monitors[i]);
                        break;
                    }
                }

                if (i < disconnectedCount) continue;

                monitor = createMonitor(&adapter, &display);
                if (!monitor) { free(disconnected); return; }

                _glfwInputMonitor(monitor, GLFW_CONNECTED, type);
                type = _GLFW_INSERT_LAST;
            }

            if (displayIndex == 0) {
                for (i = 0;  i < disconnectedCount;  i++) {
                    if (disconnected[i] && wcscmp(disconnected[i]->win32.adapterName,adapter.DeviceName) == 0) { disconnected[i] = NULL; break; }
                }

                if (i < disconnectedCount) continue;

                monitor = createMonitor(&adapter, NULL);
                if (!monitor) { free(disconnected); return; }

                _glfwInputMonitor(monitor, GLFW_CONNECTED, type);
            }
        }

        for (i = 0;  i < disconnectedCount;  i++) {
            if (disconnected[i]) _glfwInputMonitor(disconnected[i],GLFW_DISCONNECTED,0);
        }

        free(disconnected);
    }

    void _glfwSetVideoModeWin32(_GLFWmonitor* monitor, const GLFWvidmode* desired) {
        GLFWvidmode current; const GLFWvidmode* best; DEVMODEW dm; LONG result;
        best = _glfwChooseVideoMode(monitor, desired);
        _glfwGetVideoModeWin32(monitor, &current);
        if (_glfwCompareVideoModes(&current, best) == 0) return;

        ZeroMemory(&dm, sizeof(dm));
        dm.dmSize = sizeof(dm);
        dm.dmFields           = DM_PELSWIDTH|DM_PELSHEIGHT|DM_BITSPERPEL|DM_DISPLAYFREQUENCY;
        dm.dmPelsWidth = best->width; dm.dmPelsHeight = best->height;
        dm.dmBitsPerPel       = best->redBits + best->greenBits + best->blueBits;
        dm.dmDisplayFrequency = best->refreshRate;
        if (dm.dmBitsPerPel < 15 || dm.dmBitsPerPel >= 24) dm.dmBitsPerPel = 32;
        result = ChangeDisplaySettingsExW(monitor->win32.adapterName,&dm,NULL,CDS_FULLSCREEN,NULL);
        if (result == DISP_CHANGE_SUCCESSFUL) monitor->win32.modeChanged = GLFW_TRUE;
        else {
            const char* description = "Unknown error";
            if (result == DISP_CHANGE_BADDUALVIEW) description = "The system uses DualView";
            else if (result == DISP_CHANGE_BADFLAGS) description = "Invalid flags";
            else if (result == DISP_CHANGE_BADMODE) description = "Graphics mode not supported";
            else if (result == DISP_CHANGE_BADPARAM) description = "Invalid parameter";
            else if (result == DISP_CHANGE_FAILED) description = "Graphics mode failed";
            else if (result == DISP_CHANGE_NOTUPDATED) description = "Failed to write to registry";
            else if (result == DISP_CHANGE_RESTART) description = "Computer restart required";
            _glfwInputError(GLFW_PLATFORM_ERROR,"Win32: Failed to set video mode: %s",description);
        }
    }

    void _glfwRestoreVideoModeWin32(_GLFWmonitor* monitor) { if (monitor->win32.modeChanged) { ChangeDisplaySettingsExW(monitor->win32.adapterName,NULL,NULL,CDS_FULLSCREEN,NULL); monitor->win32.modeChanged = GLFW_FALSE; } }
    void _glfwGetHMONITORContentScaleWin32(HMONITOR handle, float* xscale, float* yscale) {
        UINT xdpi, ydpi; if (xscale) {*xscale = 0.f;} if (yscale) {*yscale = 0.f;}
        if (IsWindows8Point1OrGreater()) {
            if (GetDpiForMonitor(handle, MDT_EFFECTIVE_DPI, &xdpi, &ydpi) != S_OK) { _glfwInputError(GLFW_PLATFORM_ERROR, "Win32: Failed to query monitor DPI"); return; }
        } else {
            const HDC dc = GetDC(NULL);
            xdpi = GetDeviceCaps(dc,LOGPIXELSX); ydpi = GetDeviceCaps(dc,LOGPIXELSY);
            ReleaseDC(NULL,dc);
        }

        if (xscale) {*xscale = xdpi / 96.0f;} if (yscale) {*yscale = ydpi / 96.0f;}
    }

    void _glfwGetMonitorPosWin32(_GLFWmonitor* monitor, int* xpos, int* ypos) {
        DEVMODEW dm; ZeroMemory(&dm,sizeof(dm));
        dm.dmSize = sizeof(dm);
        EnumDisplaySettingsExW(monitor->win32.adapterName,ENUM_CURRENT_SETTINGS,&dm,0x00000004);
        if (xpos) {*xpos = dm.dmPosition.x;} if (ypos) {*ypos = dm.dmPosition.y;}
    }

    void _glfwGetMonitorWorkareaWin32(_GLFWmonitor* monitor, int* xpos, int* ypos, int* width, int* height) {
        MONITORINFO mi = {0}; mi.cbSize = sizeof(mi); GetMonitorInfoW(monitor->win32.handle, &mi);
        if (xpos) {*xpos = mi.rcWork.left;} if (ypos) {*ypos = mi.rcWork.top;}
        if (width) {*width = mi.rcWork.right - mi.rcWork.left;} if (height) {*height = mi.rcWork.bottom - mi.rcWork.top;}
    }

    GLFWvidmode* _glfwGetVideoModesWin32(_GLFWmonitor* monitor, int* count) {
        int modeIndex = 0, size = 0; GLFWvidmode* result = NULL; *count = 0;
        for (;;) {
            int i; GLFWvidmode mode; DEVMODEW dm; ZeroMemory(&dm,sizeof(dm)); dm.dmSize = sizeof(dm);
            if (!EnumDisplaySettingsW(monitor->win32.adapterName, modeIndex, &dm)) break;

            modeIndex++;
            if (dm.dmBitsPerPel < 15) continue; // Skip modes with less than 15 BPP

            mode.width  = dm.dmPelsWidth; mode.height = dm.dmPelsHeight; mode.refreshRate = dm.dmDisplayFrequency;
            _glfwSplitBPP(dm.dmBitsPerPel,&mode.redBits,&mode.greenBits,&mode.blueBits);
            for (i = 0;  i < *count;  i++) { if (_glfwCompareVideoModes(result + i, &mode) == 0) {break;} }
            if (i < *count) continue; // Skip duplicate modes
            if (monitor->win32.modesPruned) {
                // Skip modes not supported by the connected displays
                if (ChangeDisplaySettingsExW(monitor->win32.adapterName,&dm,NULL,CDS_TEST,NULL) != DISP_CHANGE_SUCCESSFUL) continue;
            }

            if (*count == size) { size += 128; result = (GLFWvidmode*)_glfw_realloc(result,size * sizeof(GLFWvidmode)); }
            (*count)++;
            result[*count - 1] = mode;
        }

        if (!*count) { result = _glfw_calloc(1, sizeof(GLFWvidmode)); _glfwGetVideoModeWin32(monitor,result); *count = 1; }
        return result;
    }

    GLFWbool _glfwGetVideoModeWin32(_GLFWmonitor* monitor, GLFWvidmode* mode) {
        DEVMODEW dm;
        ZeroMemory(&dm, sizeof(dm));
        dm.dmSize = sizeof(dm);
        if (!EnumDisplaySettingsW(monitor->win32.adapterName, ENUM_CURRENT_SETTINGS, &dm)) { _glfwInputError(GLFW_PLATFORM_ERROR, "Win32: Failed to query display settings"); return GLFW_FALSE; }

        mode->width  = dm.dmPelsWidth;
        mode->height = dm.dmPelsHeight;
        mode->refreshRate = dm.dmDisplayFrequency;
        _glfwSplitBPP(dm.dmBitsPerPel,&mode->redBits,&mode->greenBits,&mode->blueBits);
        return GLFW_TRUE;
    }

    void _glfwPlatformInitTimer(void) { QueryPerformanceFrequency((LARGE_INTEGER*) &_glfw.timer.win32.frequency); }
    u64 _glfwPlatformGetTimerValue(void) { u64 value; QueryPerformanceCounter((LARGE_INTEGER*)&value); return value; }
    u64 _glfwPlatformGetTimerFrequency(void) { return _glfw.timer.win32.frequency; }
    GLFWbool _glfwPlatformCreateTls(_GLFWtls* tls) {
        tls->win32.index = TlsAlloc();
        if (tls->win32.index == TLS_OUT_OF_INDEXES) { _glfwInputError(GLFW_PLATFORM_ERROR, "Win32: Failed to allocate TLS index"); return GLFW_FALSE; }

        tls->win32.allocated = GLFW_TRUE;
        return GLFW_TRUE;
    }

    void _glfwPlatformDestroyTls(_GLFWtls* tls) { if (tls->win32.allocated) {TlsFree(tls->win32.index);} memset(tls, 0, sizeof(_GLFWtls)); }
    void* _glfwPlatformGetTls(_GLFWtls* tls) { return TlsGetValue(tls->win32.index); }
    void _glfwPlatformSetTls(_GLFWtls* tls, void* value) { TlsSetValue(tls->win32.index,value); }
    GLFWbool _glfwPlatformCreateMutex(_GLFWmutex* mutex) { InitializeCriticalSection(&mutex->win32.section); return mutex->win32.allocated = GLFW_TRUE; }
    void _glfwPlatformDestroyMutex(_GLFWmutex* mutex) { if (mutex->win32.allocated) {DeleteCriticalSection(&mutex->win32.section);} memset(mutex, 0, sizeof(_GLFWmutex)); }
    void _glfwPlatformLockMutex(_GLFWmutex* mutex) { EnterCriticalSection(&mutex->win32.section); }
    void _glfwPlatformUnlockMutex(_GLFWmutex* mutex) { LeaveCriticalSection(&mutex->win32.section); }
    static int findPixelFormatAttribValueWGL(const int* attribs, int attribCount, const int* values, int attrib) {
        int i;
        for (i = 0;  i < attribCount;  i++) {
            if (attribs[i] == attrib) return values[i];
        }

        _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,"WGL: Unknown pixel format attribute requested");
        return 0;
    }

    #define ADD_ATTRIB(a) \
    { \
        attribs[attribCount++] = a; \
    }
    #define FIND_ATTRIB_VALUE(a) \
        findPixelFormatAttribValueWGL(attribs, attribCount, values, a)
    static int choosePixelFormatWGL(_GLFWwindow* window, const _GLFWctxconfig* ctxconfig, const _GLFWfbconfig* fbconfig) {
        _GLFWfbconfig* usableConfigs; const _GLFWfbconfig* closest;
        int i, pixelFormat, nativeCount, usableCount = 0, attribCount = 0, attribs[40];
        int values[sizeof(attribs) / sizeof(attribs[0])];
        nativeCount = DescribePixelFormat(window->context.wgl.dc,1,sizeof(PIXELFORMATDESCRIPTOR),NULL);
        if (_glfw.wgl.ARB_pixel_format) {
            ADD_ATTRIB(WGL_SUPPORT_OPENGL_ARB);
            ADD_ATTRIB(WGL_DRAW_TO_WINDOW_ARB);
            ADD_ATTRIB(WGL_PIXEL_TYPE_ARB);
            ADD_ATTRIB(WGL_ACCELERATION_ARB);
            ADD_ATTRIB(WGL_RED_BITS_ARB);
            ADD_ATTRIB(WGL_RED_SHIFT_ARB);
            ADD_ATTRIB(WGL_GREEN_BITS_ARB);
            ADD_ATTRIB(WGL_GREEN_SHIFT_ARB);
            ADD_ATTRIB(WGL_BLUE_BITS_ARB);
            ADD_ATTRIB(WGL_BLUE_SHIFT_ARB);
            ADD_ATTRIB(WGL_ALPHA_BITS_ARB);
            ADD_ATTRIB(WGL_ALPHA_SHIFT_ARB);
            ADD_ATTRIB(WGL_DEPTH_BITS_ARB);
            ADD_ATTRIB(WGL_STENCIL_BITS_ARB);
            ADD_ATTRIB(WGL_ACCUM_BITS_ARB);
            ADD_ATTRIB(WGL_ACCUM_RED_BITS_ARB);
            ADD_ATTRIB(WGL_ACCUM_GREEN_BITS_ARB);
            ADD_ATTRIB(WGL_ACCUM_BLUE_BITS_ARB);
            ADD_ATTRIB(WGL_ACCUM_ALPHA_BITS_ARB);
            ADD_ATTRIB(WGL_AUX_BUFFERS_ARB);
            ADD_ATTRIB(WGL_STEREO_ARB);
            ADD_ATTRIB(WGL_DOUBLE_BUFFER_ARB);
            if (_glfw.wgl.ARB_multisample) ADD_ATTRIB(WGL_SAMPLES_ARB);
            if (ctxconfig->client == GLFW_OPENGL_API) {
                if (_glfw.wgl.ARB_framebuffer_sRGB || _glfw.wgl.EXT_framebuffer_sRGB) ADD_ATTRIB(WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB);
            } else {
                if (_glfw.wgl.EXT_colorspace) ADD_ATTRIB(WGL_COLORSPACE_EXT);
            }

            const int attrib = WGL_NUMBER_PIXEL_FORMATS_ARB; int extensionCount;
            if (!wglGetPixelFormatAttribivARB(window->context.wgl.dc,1,0,1,&attrib,&extensionCount)) { _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,"WGL: Failed to retrieve pixel format attribute"); return 0; }

            nativeCount = vmin(nativeCount, extensionCount);
        }

        usableConfigs = _glfw_calloc(nativeCount, sizeof(_GLFWfbconfig));

        for (i = 0;  i < nativeCount;  i++) {
            _GLFWfbconfig* u = usableConfigs + usableCount;
            pixelFormat = i + 1;
            if (_glfw.wgl.ARB_pixel_format) {
                if (!wglGetPixelFormatAttribivARB(window->context.wgl.dc,pixelFormat,0,attribCount,attribs,values)){ _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,"WGL: Failed to retrieve pixel format attributes"); free(usableConfigs); return 0; }
                if (!FIND_ATTRIB_VALUE(WGL_SUPPORT_OPENGL_ARB) || !FIND_ATTRIB_VALUE(WGL_DRAW_TO_WINDOW_ARB)) continue;
                if (FIND_ATTRIB_VALUE(WGL_PIXEL_TYPE_ARB) != WGL_TYPE_RGBA_ARB) continue;
                if (FIND_ATTRIB_VALUE(WGL_ACCELERATION_ARB) == WGL_NO_ACCELERATION_ARB) continue;
                if (FIND_ATTRIB_VALUE(WGL_DOUBLE_BUFFER_ARB) != fbconfig->doublebuffer) continue;

                u->redBits = FIND_ATTRIB_VALUE(WGL_RED_BITS_ARB);
                u->greenBits = FIND_ATTRIB_VALUE(WGL_GREEN_BITS_ARB);
                u->blueBits = FIND_ATTRIB_VALUE(WGL_BLUE_BITS_ARB);
                u->alphaBits = FIND_ATTRIB_VALUE(WGL_ALPHA_BITS_ARB);
                u->depthBits = FIND_ATTRIB_VALUE(WGL_DEPTH_BITS_ARB);
                u->stencilBits = FIND_ATTRIB_VALUE(WGL_STENCIL_BITS_ARB);
                u->accumRedBits = FIND_ATTRIB_VALUE(WGL_ACCUM_RED_BITS_ARB);
                u->accumGreenBits = FIND_ATTRIB_VALUE(WGL_ACCUM_GREEN_BITS_ARB);
                u->accumBlueBits = FIND_ATTRIB_VALUE(WGL_ACCUM_BLUE_BITS_ARB);
                u->accumAlphaBits = FIND_ATTRIB_VALUE(WGL_ACCUM_ALPHA_BITS_ARB);
                u->auxBuffers = FIND_ATTRIB_VALUE(WGL_AUX_BUFFERS_ARB);
                if (FIND_ATTRIB_VALUE(WGL_STEREO_ARB)) u->stereo = GLFW_TRUE;
                if (_glfw.wgl.ARB_multisample) u->samples = FIND_ATTRIB_VALUE(WGL_SAMPLES_ARB);
                if (ctxconfig->client == GLFW_OPENGL_API) {
                    if (_glfw.wgl.ARB_framebuffer_sRGB || _glfw.wgl.EXT_framebuffer_sRGB) {
                        if (FIND_ATTRIB_VALUE(WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB)) u->sRGB = GLFW_TRUE;
                    }
                } else {
                    if (_glfw.wgl.EXT_colorspace) {
                        if (FIND_ATTRIB_VALUE(WGL_COLORSPACE_EXT) == WGL_COLORSPACE_SRGB_EXT) u->sRGB = GLFW_TRUE;
                    }
                }
            } else {
                PIXELFORMATDESCRIPTOR pfd;
                if (!DescribePixelFormat(window->context.wgl.dc,pixelFormat,sizeof(PIXELFORMATDESCRIPTOR),&pfd)) { _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,"WGL: Failed to describe pixel format"); free(usableConfigs); return 0; }
                if (!(pfd.dwFlags & PFD_DRAW_TO_WINDOW) || !(pfd.dwFlags & PFD_SUPPORT_OPENGL)) continue;
                if (!(pfd.dwFlags & PFD_GENERIC_ACCELERATED) && (pfd.dwFlags & PFD_GENERIC_FORMAT)) continue;
                if (pfd.iPixelType != PFD_TYPE_RGBA) continue;
                if (!!(pfd.dwFlags & PFD_DOUBLEBUFFER) != fbconfig->doublebuffer) continue;

                u->redBits = pfd.cRedBits;
                u->greenBits = pfd.cGreenBits;
                u->blueBits = pfd.cBlueBits;
                u->alphaBits = pfd.cAlphaBits;
                u->depthBits = pfd.cDepthBits;
                u->stencilBits = pfd.cStencilBits;
                u->accumRedBits = pfd.cAccumRedBits;
                u->accumGreenBits = pfd.cAccumGreenBits;
                u->accumBlueBits = pfd.cAccumBlueBits;
                u->accumAlphaBits = pfd.cAccumAlphaBits;
                u->auxBuffers = pfd.cAuxBuffers;
                if (pfd.dwFlags & PFD_STEREO) u->stereo = GLFW_TRUE;
            }

            u->handle = pixelFormat;
            usableCount++;
        }

        if (!usableCount) { _glfwInputError(GLFW_API_UNAVAILABLE,"WGL: The driver does not appear to support OpenGL"); free(usableConfigs); return 0; }

        closest = _glfwChooseFBConfig(fbconfig, usableConfigs, usableCount);
        if (!closest) { _glfwInputError(GLFW_FORMAT_UNAVAILABLE,"WGL: Failed to find a suitable pixel format"); free(usableConfigs); return 0; }

        pixelFormat = (int) closest->handle;
        free(usableConfigs);
        return pixelFormat;
    }
    #undef ADD_ATTRIB
    #undef FIND_ATTRIB_VALUE

    static void makeContextCurrentWGL(_GLFWwindow* window) {
        if (window) {
            if (wglMakeCurrent(window->context.wgl.dc,window->context.wgl.handle)) _glfwPlatformSetTls(&_glfw.contextSlot,window);
            else { _glfwInputErrorWin32(GLFW_PLATFORM_ERROR, "WGL: Failed to make context current"); _glfwPlatformSetTls(&_glfw.contextSlot,NULL); }
        } else {
            if (!wglMakeCurrent(NULL, NULL)) _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,"WGL: Failed to clear current context");
            _glfwPlatformSetTls(&_glfw.contextSlot,NULL);
        }
    }

    static void swapBuffersWGL(_GLFWwindow* window) {
        if (!window->monitor) {
            if (!IsWindows8OrGreater()) {
                BOOL enabled = FALSE;
                if (SUCCEEDED(DwmIsCompositionEnabled(&enabled)) && enabled) {
                    int count = abs(window->context.wgl.interval);
                    while (count--) DwmFlush();
                }
            }
        }

        SwapBuffers(window->context.wgl.dc);
    }

    static void swapIntervalWGL(int interval) {
        _GLFWwindow* window = _glfwPlatformGetTls(&_glfw.contextSlot);
        window->context.wgl.interval = interval;
        if (!window->monitor) {
            if (!IsWindows8OrGreater()) {
                BOOL enabled = FALSE;
                if (SUCCEEDED(DwmIsCompositionEnabled(&enabled)) && enabled) interval = 0;
            }
        }

        if (_glfw.wgl.EXT_swap_control) wglSwapIntervalEXT(interval);
    }

    static int extensionSupportedWGL(const char* extension) {
        const char* extensions = NULL;
        if (_glfw.wgl.GetExtensionsStringARB) extensions = wglGetExtensionsStringARB(wglGetCurrentDC());
        else if (_glfw.wgl.GetExtensionsStringEXT) extensions = wglGetExtensionsStringEXT();
        if (!extensions) return GLFW_FALSE;
        return _glfwStringInExtensionString(extension, extensions);
    }

    static GLFWglproc getProcAddressWGL(const char* procname) {
        const GLFWglproc proc = (GLFWglproc) wglGetProcAddress(procname);
        if (proc) return proc;
        return (GLFWglproc) _glfwPlatformGetModuleSymbol(_glfw.wgl.instance, procname);
    }

    GLFWbool _glfwInitWGL(void) {
        PIXELFORMATDESCRIPTOR pfd;
        HGLRC prc,rc; HDC pdc,dc;
        if (_glfw.wgl.instance) return GLFW_TRUE;
        _glfw.wgl.instance = _glfwPlatformLoadModule("opengl32.dll"); if (!_glfw.wgl.instance) { _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,"WGL: Failed to load opengl32.dll"); return GLFW_FALSE; }

        _glfw.wgl.CreateContext = (PFN_wglCreateContext)_glfwPlatformGetModuleSymbol(_glfw.wgl.instance, "wglCreateContext");
        _glfw.wgl.DeleteContext = (PFN_wglDeleteContext)_glfwPlatformGetModuleSymbol(_glfw.wgl.instance, "wglDeleteContext");
        _glfw.wgl.GetProcAddress = (PFN_wglGetProcAddress)_glfwPlatformGetModuleSymbol(_glfw.wgl.instance, "wglGetProcAddress");
        _glfw.wgl.GetCurrentDC = (PFN_wglGetCurrentDC)_glfwPlatformGetModuleSymbol(_glfw.wgl.instance, "wglGetCurrentDC");
        _glfw.wgl.GetCurrentContext = (PFN_wglGetCurrentContext)_glfwPlatformGetModuleSymbol(_glfw.wgl.instance, "wglGetCurrentContext");
        _glfw.wgl.MakeCurrent = (PFN_wglMakeCurrent)_glfwPlatformGetModuleSymbol(_glfw.wgl.instance, "wglMakeCurrent");
        _glfw.wgl.ShareLists = (PFN_wglShareLists)_glfwPlatformGetModuleSymbol(_glfw.wgl.instance, "wglShareLists");
        dc = GetDC(_glfw.win32.helperWindowHandle);
        ZeroMemory(&pfd,sizeof(pfd));
        pfd.nSize = sizeof(pfd);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 24;
        if (!SetPixelFormat(dc, ChoosePixelFormat(dc, &pfd), &pfd)) { _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,"WGL: Failed to set pixel format for dummy context"); return GLFW_FALSE; }

        rc = wglCreateContext(dc);
        if (!rc) { _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,"WGL: Failed to create dummy context"); return GLFW_FALSE; }

        pdc = wglGetCurrentDC(); prc = wglGetCurrentContext();
        if (!wglMakeCurrent(dc, rc)) { _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,"WGL: Failed to make dummy context current"); wglMakeCurrent(pdc, prc); wglDeleteContext(rc); return GLFW_FALSE; }

        _glfw.wgl.GetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");
        _glfw.wgl.GetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
        _glfw.wgl.CreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
        _glfw.wgl.SwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
        _glfw.wgl.GetPixelFormatAttribivARB = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)wglGetProcAddress("wglGetPixelFormatAttribivARB");
        _glfw.wgl.ARB_multisample = extensionSupportedWGL("WGL_ARB_multisample");
        _glfw.wgl.ARB_framebuffer_sRGB = extensionSupportedWGL("WGL_ARB_framebuffer_sRGB");
        _glfw.wgl.EXT_framebuffer_sRGB = extensionSupportedWGL("WGL_EXT_framebuffer_sRGB");
        _glfw.wgl.ARB_create_context = extensionSupportedWGL("WGL_ARB_create_context");
        _glfw.wgl.ARB_create_context_profile = extensionSupportedWGL("WGL_ARB_create_context_profile");
        _glfw.wgl.EXT_create_context_es2_profile = extensionSupportedWGL("WGL_EXT_create_context_es2_profile");
        _glfw.wgl.ARB_create_context_robustness = extensionSupportedWGL("WGL_ARB_create_context_robustness");
        _glfw.wgl.ARB_create_context_no_error = extensionSupportedWGL("WGL_ARB_create_context_no_error");
        _glfw.wgl.EXT_swap_control = extensionSupportedWGL("WGL_EXT_swap_control");
        _glfw.wgl.EXT_colorspace = extensionSupportedWGL("WGL_EXT_colorspace");
        _glfw.wgl.ARB_pixel_format = extensionSupportedWGL("WGL_ARB_pixel_format");
        _glfw.wgl.ARB_context_flush_control = extensionSupportedWGL("WGL_ARB_context_flush_control");
        wglMakeCurrent(pdc,prc);
        wglDeleteContext(rc);
        return GLFW_TRUE;
    }

    #define SET_ATTRIB(a, v) \
    { \
        attribs[index++] = a; \
        attribs[index++] = v; \
    }
    GLFWbool _glfwCreateContextWGL(_GLFWwindow* window, const _GLFWctxconfig* ctxconfig, const _GLFWfbconfig* fbconfig) {
        int attribs[40],pixelFormat; PIXELFORMATDESCRIPTOR pfd; HGLRC share = NULL;
        if (ctxconfig->share) share = ctxconfig->share->context.wgl.handle;
        window->context.wgl.dc = GetDC(window->win32.handle);
        if (!window->context.wgl.dc) { _glfwInputError(GLFW_PLATFORM_ERROR,"WGL: Failed to retrieve DC for window"); return GLFW_FALSE; }

        pixelFormat = choosePixelFormatWGL(window, ctxconfig, fbconfig);
        if (!pixelFormat) return GLFW_FALSE;
        if (!DescribePixelFormat(window->context.wgl.dc,pixelFormat,sizeof(pfd),&pfd)) { _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,"WGL: Failed to retrieve PFD for selected pixel format"); return GLFW_FALSE; }
        if (!SetPixelFormat(window->context.wgl.dc, pixelFormat, &pfd)) { _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,"WGL: Failed to set selected pixel format"); return GLFW_FALSE; }

        if (ctxconfig->client == GLFW_OPENGL_API) {
            if (ctxconfig->forward) {
                if (!_glfw.wgl.ARB_create_context) { _glfwInputError(GLFW_VERSION_UNAVAILABLE,"WGL: A forward compatible OpenGL context requested but WGL_ARB_create_context is unavailable"); return GLFW_FALSE; }
            }

            if (ctxconfig->profile) {
                if (!_glfw.wgl.ARB_create_context_profile) { _glfwInputError(GLFW_VERSION_UNAVAILABLE,"WGL: OpenGL profile requested but WGL_ARB_create_context_profile is unavailable"); return GLFW_FALSE; }
            }
        } else {
            if (!_glfw.wgl.ARB_create_context || !_glfw.wgl.ARB_create_context_profile || !_glfw.wgl.EXT_create_context_es2_profile) { _glfwInputError(GLFW_API_UNAVAILABLE,"WGL: OpenGL ES requested but WGL_ARB_create_context_es2_profile is unavailable"); return GLFW_FALSE; }
        }

        if (_glfw.wgl.ARB_create_context) {
            int index = 0, mask = 0, flags = 0;
            if (ctxconfig->client == GLFW_OPENGL_API) {
                if (ctxconfig->forward) flags |= WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
                if (ctxconfig->profile == GLFW_OPENGL_CORE_PROFILE) mask |= WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
                else if (ctxconfig->profile == GLFW_OPENGL_COMPAT_PROFILE) mask |= WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
            } else mask |= WGL_CONTEXT_ES2_PROFILE_BIT_EXT;

            if (ctxconfig->debug) flags |= WGL_CONTEXT_DEBUG_BIT_ARB;
            if (ctxconfig->robustness) {
                if (_glfw.wgl.ARB_create_context_robustness) {
                    if (ctxconfig->robustness == GLFW_NO_RESET_NOTIFICATION) {
                        SET_ATTRIB(WGL_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB,WGL_NO_RESET_NOTIFICATION_ARB);
                    } else if (ctxconfig->robustness == GLFW_LOSE_CONTEXT_ON_RESET) {
                        SET_ATTRIB(WGL_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB,WGL_LOSE_CONTEXT_ON_RESET_ARB);
                    }

                    flags |= WGL_CONTEXT_ROBUST_ACCESS_BIT_ARB;
                }
            }

            if (ctxconfig->release) {
                if (_glfw.wgl.ARB_context_flush_control) {
                    if (ctxconfig->release == GLFW_RELEASE_BEHAVIOR_NONE) {
                        SET_ATTRIB(WGL_CONTEXT_RELEASE_BEHAVIOR_ARB,WGL_CONTEXT_RELEASE_BEHAVIOR_NONE_ARB);
                    } else if (ctxconfig->release == GLFW_RELEASE_BEHAVIOR_FLUSH) {
                        SET_ATTRIB(WGL_CONTEXT_RELEASE_BEHAVIOR_ARB,WGL_CONTEXT_RELEASE_BEHAVIOR_FLUSH_ARB);
                    }
                }
            }

            if (ctxconfig->noerror) {
                if (_glfw.wgl.ARB_create_context_no_error) SET_ATTRIB(WGL_CONTEXT_OPENGL_NO_ERROR_ARB, GLFW_TRUE);
            }

            if (ctxconfig->major != 1 || ctxconfig->minor != 0) {
                SET_ATTRIB(WGL_CONTEXT_MAJOR_VERSION_ARB, ctxconfig->major);
                SET_ATTRIB(WGL_CONTEXT_MINOR_VERSION_ARB, ctxconfig->minor);
            }

            if (flags)
                SET_ATTRIB(WGL_CONTEXT_FLAGS_ARB, flags);

            if (mask)
                SET_ATTRIB(WGL_CONTEXT_PROFILE_MASK_ARB, mask);

            SET_ATTRIB(0, 0);
            window->context.wgl.handle = wglCreateContextAttribsARB(window->context.wgl.dc, share, attribs);
            if (!window->context.wgl.handle) {
                const DWORD error = GetLastError();
                if (error == (0xc0070000 | ERROR_INVALID_VERSION_ARB)) {
                    if (ctxconfig->client == GLFW_OPENGL_API) _glfwInputError(GLFW_VERSION_UNAVAILABLE,"WGL: Driver does not support OpenGL version %i.%i",ctxconfig->major,ctxconfig->minor);
                    else _glfwInputError(GLFW_VERSION_UNAVAILABLE,"WGL: Driver does not support OpenGL ES version %i.%i",ctxconfig->major,ctxconfig->minor);
                } else if (error == (0xc0070000 | ERROR_INVALID_PROFILE_ARB)) _glfwInputError(GLFW_VERSION_UNAVAILABLE,"WGL: Driver does not support the requested OpenGL profile");
                else if (error == (0xc0070000 | ERROR_INCOMPATIBLE_DEVICE_CONTEXTS_ARB)) _glfwInputError(GLFW_INVALID_VALUE,"WGL: The share context is not compatible with the requested context");
                else {
                    if (ctxconfig->client == GLFW_OPENGL_API) _glfwInputError(GLFW_VERSION_UNAVAILABLE,"WGL: Failed to create OpenGL context");
                    else _glfwInputError(GLFW_VERSION_UNAVAILABLE,"WGL: Failed to create OpenGL ES context");
                }

                return GLFW_FALSE;
            }
        } else {
            window->context.wgl.handle = wglCreateContext(window->context.wgl.dc);
            if (!window->context.wgl.handle) { _glfwInputErrorWin32(GLFW_VERSION_UNAVAILABLE,"WGL: Failed to create OpenGL context"); return GLFW_FALSE; }

            if (share) {
                if (!wglShareLists(share, window->context.wgl.handle)) { _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,"WGL: Failed to enable sharing with specified OpenGL context"); return GLFW_FALSE; }
            }
        }

        window->context.makeCurrent = makeContextCurrentWGL;
        window->context.swapBuffers = swapBuffersWGL;
        window->context.swapInterval = swapIntervalWGL;
        window->context.extensionSupported = extensionSupportedWGL;
        window->context.getProcAddress = getProcAddressWGL;
        return GLFW_TRUE;
    }
    #undef SET_ATTRIB
    
    GLFWbool _glfwCreateWindowWin32(_GLFWwindow* window,const _GLFWwndconfig* wndconfig,const _GLFWctxconfig* ctxconfig,const _GLFWfbconfig* fbconfig) {
        if (!createNativeWindow(window,wndconfig,fbconfig)) return GLFW_FALSE;
        if (ctxconfig->client!=GLFW_NO_API) {
            if (ctxconfig->source==GLFW_NATIVE_CONTEXT_API) {
                if (!_glfwInitWGL()) return GLFW_FALSE;
                if (!_glfwCreateContextWGL(window,ctxconfig,fbconfig)) return GLFW_FALSE;
            }
            if (!_glfwRefreshContextAttribs(window,ctxconfig)) return GLFW_FALSE;
        }
        if (wndconfig->mousePassthrough) _glfwSetWindowMousePassthroughWin32(window,GLFW_TRUE);
        if (window->monitor) { _glfwShowWindowWin32(window); _glfwFocusWindowWin32(window); acquireMonitor(window); fitToMonitor(window); if (wndconfig->centerCursor) _glfwCenterCursorInContentArea(window); }
        else if (wndconfig->visible) { _glfwShowWindowWin32(window); if (wndconfig->focused) _glfwFocusWindowWin32(window); }
        return GLFW_TRUE;
    }

    int _glfwGetKeyScancodeWin32(int key) { return _glfw.win32.scancodes[key]; }
    GLFWAPI HGLRC glfwGetWGLContext(GLFWwindow* handle) { _GLFWwindow* window = (_GLFWwindow*) handle; if (window->context.source != GLFW_NATIVE_CONTEXT_API) { _glfwInputError(GLFW_NO_WINDOW_CONTEXT, NULL); return NULL; } return window->context.wgl.handle; }
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
    #include <locale.h>
    #include <errno.h>
    #include <sys/time.h>
    #include <sys/inotify.h>
    #include <sys/ioctl.h>
    #include <dirent.h>
    #define _NET_WM_STATE_REMOVE 0
    #define _NET_WM_STATE_ADD    1
    #define _NET_WM_STATE_TOGGLE 2
    #define Button6 6
    #define Button7 7
    #define MWM_HINTS_DECORATIONS 2
    #define MWM_DECOR_ALL         1
    XContext XUniqueContext(void) { static XContext lastContext = 0; return ++lastContext; }
    void _glfwPlatformInitTimer(void) {
        _glfw.timer.posix.clock = CLOCK_REALTIME;
        _glfw.timer.posix.frequency = 1000000000;
        struct timespec ts;
        if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) _glfw.timer.posix.clock = CLOCK_MONOTONIC;
    }

    u64 _glfwPlatformGetTimerValue(void) {
        struct timespec ts;
        clock_gettime(_glfw.timer.posix.clock, &ts);
        return (u64) ts.tv_sec * _glfw.timer.posix.frequency + (u64) ts.tv_nsec;
    }

    u64 _glfwPlatformGetTimerFrequency(void) { return _glfw.timer.posix.frequency; }

    GLFWbool _glfwPlatformCreateTls(_GLFWtls* tls) {
        if (pthread_key_create(&tls->posix.key, NULL) != 0) { _glfwInputError(GLFW_PLATFORM_ERROR,"POSIX: Failed to create context TLS"); return GLFW_FALSE; }

        tls->posix.allocated = GLFW_TRUE;
        return GLFW_TRUE;
    }

    void _glfwPlatformDestroyTls(_GLFWtls* tls) { if (tls->posix.allocated) {pthread_key_delete(tls->posix.key);} memset(tls,0,sizeof(_GLFWtls)); }
    void* _glfwPlatformGetTls(_GLFWtls* tls) { return pthread_getspecific(tls->posix.key); }
    void _glfwPlatformSetTls(_GLFWtls* tls, void* value) { pthread_setspecific(tls->posix.key, value); }
    GLFWbool _glfwPlatformCreateMutex(_GLFWmutex* mutex) {
        if (pthread_mutex_init(&mutex->posix.handle, NULL) != 0) { _glfwInputError(GLFW_PLATFORM_ERROR, "POSIX: Failed to create mutex"); return GLFW_FALSE; }
        return mutex->posix.allocated = GLFW_TRUE;
    }

    void _glfwPlatformDestroyMutex(_GLFWmutex* mutex) { if (mutex->posix.allocated) {pthread_mutex_destroy(&mutex->posix.handle);} memset(mutex, 0, sizeof(_GLFWmutex)); }
    void _glfwPlatformLockMutex(_GLFWmutex* mutex) { pthread_mutex_lock(&mutex->posix.handle); }
    void _glfwPlatformUnlockMutex(_GLFWmutex* mutex) { pthread_mutex_unlock(&mutex->posix.handle); }
    int ppoll(struct pollfd *fds, nfds_t nfds, const struct timespec *tmo_p, const sigset_t *sigmask);
    GLFWbool _glfwPollPOSIX(struct pollfd* fds, nfds_t count, double* timeout) {
        for (;;) {
            if (timeout) {
                const u64 base = _glfwPlatformGetTimerValue();
                const time_t seconds = (time_t) *timeout;
                const long nanoseconds = (long) ((*timeout - seconds) * 1e9);
                const struct timespec ts = { seconds, nanoseconds };
                const int result = ppoll(fds, count, &ts, NULL);
                const int error = errno; // clock_gettime may overwrite our error
                *timeout -= (_glfwPlatformGetTimerValue() - base) / (double) _glfwPlatformGetTimerFrequency();
                if (result > 0) return GLFW_TRUE;
                else if (result == -1 && error != EINTR && error != EAGAIN) return GLFW_FALSE;
                else if (*timeout <= 0.0) return GLFW_FALSE;
            } else {
                const int result = poll(fds, count, -1);
                if (result > 0) return GLFW_TRUE;
                else if (result == -1 && errno != EINTR && errno != EAGAIN) return GLFW_FALSE;
            }
        }
    }

    void* _glfwPlatformLoadModule(const char* path) { return dlopen(path, RTLD_LAZY | RTLD_LOCAL); }
    void _glfwPlatformFreeModule(void* module) { if (module) {dlclose(module);} }
    GLFWproc _glfwPlatformGetModuleSymbol(void* module, const char* name) { return dlsym(module, name); }

    static GLFWbool waitForX11Event(double* timeout) {
        struct pollfd fd = {ConnectionNumber(_glfw.x11.display),POLLIN,0U};
        while (!XPending(_glfw.x11.display)) { if (!_glfwPollPOSIX(&fd,1,timeout)) return GLFW_FALSE; }
        return GLFW_TRUE;
    }

    static GLFWbool waitForAnyEvent(double* timeout) {
        enum { XLIB_FD,PIPE_FD,INOTIFY_FD };
        struct pollfd fds[] = {
            [XLIB_FD]    = {ConnectionNumber(_glfw.x11.display),POLLIN,0U},
            [PIPE_FD]    = {_glfw.x11.emptyEventPipe[0],POLLIN,0U},
            [INOTIFY_FD] = {-1,POLLIN,0U}
        };
        
        if (_glfw.joysticksInitialized) fds[INOTIFY_FD].fd = _glfw.linjs.inotify;
        while (!XPending(_glfw.x11.display)) {
            if (!_glfwPollPOSIX(fds,sizeof(fds)/sizeof(fds[0]),timeout)) return GLFW_FALSE;
            for (int i=1;i<(int)(sizeof(fds)/sizeof(fds[0]));++i) { if (fds[i].revents & POLLIN) return GLFW_TRUE; }
        }
        return GLFW_TRUE;
    }

    static void writeEmptyEvent(void) {
        for (;;) { const char byte=0; const ssize_t r=write(_glfw.x11.emptyEventPipe[1],&byte,1); if (r==1||(r==-1&&errno!=EINTR)) break; }
    }

    static void drainEmptyEvents(void) {
        for (;;) { char dummy[64]; const ssize_t r=read(_glfw.x11.emptyEventPipe[0],dummy,sizeof(dummy)); if (r==-1&&errno!=EINTR) break; }
    }

    static GLFWbool waitForVisibilityNotify(_GLFWwindow* window) {
        XEvent dummy; double timeout=0.1;
        while (!XCheckTypedWindowEvent(_glfw.x11.display,window->x11.handle,VisibilityNotify,&dummy)) { if (!waitForX11Event(&timeout)) return GLFW_FALSE; }
        return GLFW_TRUE;
    }

    static int getWindowState(_GLFWwindow* window) {
        int result=WithdrawnState;
        struct { u32 state; Window icon; }* state=NULL;
        if (_glfwGetWindowPropertyX11(window->x11.handle,_glfw.x11.WM_STATE,_glfw.x11.WM_STATE,(unsigned char**)&state) >= 2) result=state->state;
        if (state) XFree(state);
        return result;
    }

    static Bool isFrameExtentsEvent(Display* display,XEvent* event,XPointer pointer) {
        _GLFWwindow* window=(_GLFWwindow*)pointer; (void)display;
        return event->type==PropertyNotify && event->xproperty.state==PropertyNewValue && event->xproperty.window==window->x11.handle && event->xproperty.atom==_glfw.x11.NET_FRAME_EXTENTS;
    }

    static int translateState(int state) {
        int mods=0;
        if (state & ShiftMask)   mods |= GLFW_MOD_SHIFT;
        if (state & ControlMask) mods |= GLFW_MOD_CONTROL;
        if (state & Mod1Mask)    mods |= GLFW_MOD_ALT;
        if (state & Mod4Mask)    mods |= GLFW_MOD_SUPER;
        if (state & LockMask)    mods |= GLFW_MOD_CAPS_LOCK;
        if (state & Mod2Mask)    mods |= GLFW_MOD_NUM_LOCK;
        return mods;
    }

    static int translateKey(int scancode) { return (scancode<0||scancode>255) ? GLFW_KEY_UNKNOWN : _glfw.x11.keycodes[scancode]; }

    static void sendEventToWM(_GLFWwindow* window,Atom type,long a,long b,long c,long d,long e) {
        XEvent event={ClientMessage};
        event.xclient.window=window->x11.handle; event.xclient.format=32; event.xclient.message_type=type;
        event.xclient.data.l[0]=a; event.xclient.data.l[1]=b; event.xclient.data.l[2]=c; event.xclient.data.l[3]=d; event.xclient.data.l[4]=e;
        XSendEvent(_glfw.x11.display,_glfw.x11.root,False,SubstructureNotifyMask|SubstructureRedirectMask,&event);
    }

    static void updateNormalHints(_GLFWwindow* window,int width,int height) {
        XSizeHints* hints=XAllocSizeHints(); long supplied;
        XGetWMNormalHints(_glfw.x11.display,window->x11.handle,hints,&supplied);
        hints->flags &= ~(PMinSize|PMaxSize|PAspect);
        if (!window->monitor) {
            if (window->resizable) {
                if (window->minwidth!=GLFW_DONT_CARE && window->minheight!=GLFW_DONT_CARE) { hints->flags|=PMinSize; hints->min_width=window->minwidth; hints->min_height=window->minheight; }
                if (window->maxwidth!=GLFW_DONT_CARE && window->maxheight!=GLFW_DONT_CARE) { hints->flags|=PMaxSize; hints->max_width=window->maxwidth; hints->max_height=window->maxheight; }
                if (window->numer!=GLFW_DONT_CARE && window->denom!=GLFW_DONT_CARE) { hints->flags|=PAspect; hints->min_aspect.x=hints->max_aspect.x=window->numer; hints->min_aspect.y=hints->max_aspect.y=window->denom; }
            } else {
                hints->flags|=(PMinSize|PMaxSize);
                hints->min_width=hints->max_width=width; hints->min_height=hints->max_height=height;
            }
        }
        XSetWMNormalHints(_glfw.x11.display,window->x11.handle,hints);
        XFree(hints);
    }

    static void updateWindowMode(_GLFWwindow* window) {
        if (window->monitor) {
            if (_glfw.x11.xinerama.available && _glfw.x11.NET_WM_FULLSCREEN_MONITORS) sendEventToWM(window,_glfw.x11.NET_WM_FULLSCREEN_MONITORS,window->monitor->x11.index,window->monitor->x11.index,window->monitor->x11.index,window->monitor->x11.index,0);
            if (_glfw.x11.NET_WM_STATE && _glfw.x11.NET_WM_STATE_FULLSCREEN) sendEventToWM(window,_glfw.x11.NET_WM_STATE,_NET_WM_STATE_ADD,_glfw.x11.NET_WM_STATE_FULLSCREEN,0,1,0);
            else {
                XSetWindowAttributes attributes; attributes.override_redirect=True;
                XChangeWindowAttributes(_glfw.x11.display,window->x11.handle,CWOverrideRedirect,&attributes);
                window->x11.overrideRedirect=GLFW_TRUE;
            }
            if (!window->x11.transparent) { const unsigned long value=1; XChangeProperty(_glfw.x11.display,window->x11.handle,_glfw.x11.NET_WM_BYPASS_COMPOSITOR,XA_CARDINAL,32,PropModeReplace,(unsigned char*)&value,1); }
        } else {
            if (_glfw.x11.xinerama.available && _glfw.x11.NET_WM_FULLSCREEN_MONITORS) XDeleteProperty(_glfw.x11.display,window->x11.handle,_glfw.x11.NET_WM_FULLSCREEN_MONITORS);
            if (_glfw.x11.NET_WM_STATE && _glfw.x11.NET_WM_STATE_FULLSCREEN) sendEventToWM(window,_glfw.x11.NET_WM_STATE,_NET_WM_STATE_REMOVE,_glfw.x11.NET_WM_STATE_FULLSCREEN,0,1,0);
            else {
                XSetWindowAttributes attributes; attributes.override_redirect=False;
                XChangeWindowAttributes(_glfw.x11.display,window->x11.handle,CWOverrideRedirect,&attributes);
                window->x11.overrideRedirect=GLFW_FALSE;
            }
            if (!window->x11.transparent) XDeleteProperty(_glfw.x11.display,window->x11.handle,_glfw.x11.NET_WM_BYPASS_COMPOSITOR);
        }
    }

    static void updateCursorImage(_GLFWwindow* window) {
        if (window->cursorMode==GLFW_CURSOR_NORMAL || window->cursorMode==GLFW_CURSOR_CAPTURED) {
            if (window->cursor) XDefineCursor(_glfw.x11.display,window->x11.handle,window->cursor->x11.handle);
            else XUndefineCursor(_glfw.x11.display,window->x11.handle);
        } else XDefineCursor(_glfw.x11.display,window->x11.handle,_glfw.x11.hiddenCursorHandle);
    }

    static void captureCursor(_GLFWwindow* window) { XGrabPointer(_glfw.x11.display,window->x11.handle,True,ButtonPressMask|ButtonReleaseMask|PointerMotionMask,GrabModeAsync,GrabModeAsync,window->x11.handle,None,CurrentTime); }
    static void releaseCursor(void) { XUngrabPointer(_glfw.x11.display,CurrentTime); }

    static void enableRawMouseMotion(_GLFWwindow* window) {
        XIEventMask em; unsigned char mask[XIMaskLen(XI_RawMotion)]={0}; (void)window;
        em.deviceid=XIAllMasterDevices; em.mask_len=sizeof(mask); em.mask=mask;
        XISetMask(mask,XI_RawMotion);
        XISelectEvents(_glfw.x11.display,_glfw.x11.root,&em,1);
    }

    static void disableRawMouseMotion(_GLFWwindow* window) {
        XIEventMask em; unsigned char mask[]={0}; (void)window;
        em.deviceid=XIAllMasterDevices; em.mask_len=sizeof(mask); em.mask=mask;
        XISelectEvents(_glfw.x11.display,_glfw.x11.root,&em,1);
    }

    static void disableCursor(_GLFWwindow* window) {
        if (window->rawMouseMotion) enableRawMouseMotion(window);
        _glfw.x11.disabledCursorWindow=window;
        _glfwGetCursorPosX11(window,&_glfw.x11.restoreCursorPosX,&_glfw.x11.restoreCursorPosY);
        updateCursorImage(window);
        _glfwCenterCursorInContentArea(window);
        captureCursor(window);
    }

    static void enableCursor(_GLFWwindow* window) {
        if (window->rawMouseMotion) disableRawMouseMotion(window);
        _glfw.x11.disabledCursorWindow=NULL;
        releaseCursor();
        _glfwSetCursorPosX11(window,_glfw.x11.restoreCursorPosX,_glfw.x11.restoreCursorPosY);
        updateCursorImage(window);
    }

    static void inputContextDestroyCallback(XIC ic,XPointer clientData,XPointer callData) { _GLFWwindow* window=(_GLFWwindow*)clientData; window->x11.ic=NULL; (void)ic; (void)callData; }

    static void acquireMonitor(_GLFWwindow* window) {
        if (_glfw.x11.saver.count==0) {
            XGetScreenSaver(_glfw.x11.display,&_glfw.x11.saver.timeout,&_glfw.x11.saver.interval,&_glfw.x11.saver.blanking,&_glfw.x11.saver.exposure);
            XSetScreenSaver(_glfw.x11.display,0,0,DontPreferBlanking,DefaultExposures);
        }
        if (!window->monitor->window) _glfw.x11.saver.count++;
        _glfwSetVideoModeX11(window->monitor,&window->videoMode);
        if (window->x11.overrideRedirect) {
            int xpos,ypos; GLFWvidmode mode;
            _glfwGetMonitorPosX11(window->monitor,&xpos,&ypos);
            _glfwGetVideoModeX11(window->monitor,&mode);
            XMoveResizeWindow(_glfw.x11.display,window->x11.handle,xpos,ypos,mode.width,mode.height);
        }
        _glfwInputMonitorWindow(window->monitor,window);
    }

    static void releaseMonitor(_GLFWwindow* window) {
        if (window->monitor->window!=window) return;
        _glfwInputMonitorWindow(window->monitor,NULL);
        _glfwRestoreVideoModeX11(window->monitor);
        if (--_glfw.x11.saver.count==0)
            XSetScreenSaver(_glfw.x11.display,_glfw.x11.saver.timeout,_glfw.x11.saver.interval,_glfw.x11.saver.blanking,_glfw.x11.saver.exposure);
    }

    static void processEvent(XEvent* event) {
        unsigned int keycode=0; Bool filtered=False;
        if (event->type==KeyPress || event->type==KeyRelease) keycode=event->xkey.keycode;
        filtered=XFilterEvent(event,None);
        if (_glfw.x11.randr.available && event->type==_glfw.x11.randr.eventBase+RRNotify) { XRRUpdateConfiguration(event); _glfwPollMonitorsX11(); return; }
        if (_glfw.x11.xkb.available && event->type==_glfw.x11.xkb.eventBase+XkbEventCode) {
            if (((XkbEvent*)event)->any.xkb_type==XkbStateNotify && (((XkbEvent*)event)->state.changed & XkbGroupStateMask)) _glfw.x11.xkb.group=((XkbEvent*)event)->state.group;
            return;
        }
        if (event->type==GenericEvent) {
            if (_glfw.x11.xi.available) {
                _GLFWwindow* window=_glfw.x11.disabledCursorWindow;
                if (window && window->rawMouseMotion && event->xcookie.extension==_glfw.x11.xi.majorOpcode && XGetEventData(_glfw.x11.display,&event->xcookie) && event->xcookie.evtype==XI_RawMotion) {
                    XIRawEvent* re=event->xcookie.data;
                    if (re->valuators.mask_len) {
                        const double* values=re->raw_values;
                        double xpos=window->virtualCursorPosX,ypos=window->virtualCursorPosY;
                        if (XIMaskIsSet(re->valuators.mask,0)) { xpos+=*values; values++; }
                        if (XIMaskIsSet(re->valuators.mask,1)) ypos+=*values;
                        _glfwInputCursorPos(window,xpos,ypos);
                    }
                }
                XFreeEventData(_glfw.x11.display,&event->xcookie);
            }
            return;
        }

        _GLFWwindow* window=NULL;
        if (XFindContext(_glfw.x11.display,event->xany.window,_glfw.x11.context,(XPointer*)&window)!=0) return;

        switch (event->type) {
            case ReparentNotify: window->x11.parent=event->xreparent.parent; return;
            case KeyPress:
            case KeyRelease: {
                const int key=translateKey(keycode),mods=translateState(event->xkey.state),action=(event->type==KeyPress)?GLFW_PRESS:GLFW_RELEASE;
                if (key!=GLFW_KEY_UNKNOWN) _glfwInputKey(window,key,keycode,action,mods);
                return;
            }
            case ButtonPress: {
                const int mods=translateState(event->xbutton.state);
                if      (event->xbutton.button==Button1) _glfwInputMouseClick(window,GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS,mods);
                else if (event->xbutton.button==Button2) _glfwInputMouseClick(window,GLFW_MOUSE_BUTTON_MIDDLE,GLFW_PRESS,mods);
                else if (event->xbutton.button==Button3) _glfwInputMouseClick(window,GLFW_MOUSE_BUTTON_RIGHT,GLFW_PRESS,mods);
                else if (event->xbutton.button==Button4) _glfwInputScroll(window,0.0,1.0);
                else if (event->xbutton.button==Button5) _glfwInputScroll(window,0.0,-1.0);
                else if (event->xbutton.button==Button6) _glfwInputScroll(window,1.0,0.0);
                else if (event->xbutton.button==Button7) _glfwInputScroll(window,-1.0,0.0);
                else _glfwInputMouseClick(window,event->xbutton.button-Button1-4,GLFW_PRESS,mods);
                return;
            }
            case ButtonRelease: {
                const int mods=translateState(event->xbutton.state);
                if      (event->xbutton.button==Button1) _glfwInputMouseClick(window,GLFW_MOUSE_BUTTON_LEFT,GLFW_RELEASE,mods);
                else if (event->xbutton.button==Button2) _glfwInputMouseClick(window,GLFW_MOUSE_BUTTON_MIDDLE,GLFW_RELEASE,mods);
                else if (event->xbutton.button==Button3) _glfwInputMouseClick(window,GLFW_MOUSE_BUTTON_RIGHT,GLFW_RELEASE,mods);
                else if (event->xbutton.button>Button7)  _glfwInputMouseClick(window,event->xbutton.button-Button1-4,GLFW_RELEASE,mods);
                return;
            }
            case EnterNotify: {
                const int x=event->xcrossing.x,y=event->xcrossing.y;
                if (window->cursorMode==GLFW_CURSOR_HIDDEN) updateCursorImage(window);
                _glfwInputCursorEnter(window,GLFW_TRUE);
                _glfwInputCursorPos(window,x,y);
                window->x11.lastCursorPosX=x; window->x11.lastCursorPosY=y;
                return;
            }
            case LeaveNotify: _glfwInputCursorEnter(window,GLFW_FALSE); return;
            case MotionNotify: {
                const int x=event->xmotion.x,y=event->xmotion.y;
                if (x!=window->x11.warpCursorPosX || y!=window->x11.warpCursorPosY) {
                    if (window->cursorMode==GLFW_CURSOR_DISABLED) {
                        if (_glfw.x11.disabledCursorWindow!=window || window->rawMouseMotion) return;
                        _glfwInputCursorPos(window,window->virtualCursorPosX+(x-window->x11.lastCursorPosX),window->virtualCursorPosY+(y-window->x11.lastCursorPosY));
                    } else _glfwInputCursorPos(window,x,y);
                }
                window->x11.lastCursorPosX=x; window->x11.lastCursorPosY=y;
                return;
            }
            case ConfigureNotify: {
                if (event->xconfigure.width!=window->x11.width || event->xconfigure.height!=window->x11.height) {
                    window->x11.width=event->xconfigure.width; window->x11.height=event->xconfigure.height;
                    _glfwInputFramebufferSize(window,event->xconfigure.width,event->xconfigure.height);
                    _glfwInputWindowSize(window,event->xconfigure.width,event->xconfigure.height);
                }
                int xpos=event->xconfigure.x,ypos=event->xconfigure.y;
                if (!event->xany.send_event && window->x11.parent!=_glfw.x11.root) {
                    _glfwGrabErrorHandlerX11();
                    Window dummy;
                    XTranslateCoordinates(_glfw.x11.display,window->x11.parent,_glfw.x11.root,xpos,ypos,&xpos,&ypos,&dummy);
                    _glfwReleaseErrorHandlerX11();
                    if (_glfw.x11.errorCode==BadWindow) return;
                }
                if (xpos!=window->x11.xpos || ypos!=window->x11.ypos) { window->x11.xpos=xpos; window->x11.ypos=ypos; _glfwInputWindowPos(window,xpos,ypos); }
                return;
            }
            case ClientMessage: {
                if (filtered) return;
                if (event->xclient.message_type==None) return;
                if (event->xclient.message_type==_glfw.x11.WM_PROTOCOLS) {
                    const Atom protocol=event->xclient.data.l[0];
                    if (protocol==None) return;
                    if (protocol==_glfw.x11.WM_DELETE_WINDOW) _glfwInputWindowCloseRequest(window);
                    else if (protocol==_glfw.x11.NET_WM_PING) {
                        XEvent reply=*event; reply.xclient.window=_glfw.x11.root;
                        XSendEvent(_glfw.x11.display,_glfw.x11.root,False,SubstructureNotifyMask|SubstructureRedirectMask,&reply);
                    }
                }
                return;
            }
            case FocusIn: {
                if (event->xfocus.mode==NotifyGrab || event->xfocus.mode==NotifyUngrab) return;
                if (window->cursorMode==GLFW_CURSOR_DISABLED) disableCursor(window);
                else if (window->cursorMode==GLFW_CURSOR_CAPTURED) captureCursor(window);
                if (window->x11.ic) XSetICFocus(window->x11.ic);
                _glfwInputWindowFocus(window,GLFW_TRUE);
                return;
            }
            case FocusOut: {
                if (event->xfocus.mode==NotifyGrab || event->xfocus.mode==NotifyUngrab) return;
                if (window->cursorMode==GLFW_CURSOR_DISABLED) enableCursor(window);
                else if (window->cursorMode==GLFW_CURSOR_CAPTURED) releaseCursor();
                if (window->x11.ic) XUnsetICFocus(window->x11.ic);
                if (window->monitor && window->autoIconify) _glfwIconifyWindowX11(window);
                _glfwInputWindowFocus(window,GLFW_FALSE);
                return;
            }
            case Expose: _glfwInputWindowDamage(window); return;
            case PropertyNotify: {
                if (event->xproperty.state!=PropertyNewValue) return;
                if (event->xproperty.atom==_glfw.x11.WM_STATE) {
                    const int state=getWindowState(window);
                    if (state!=IconicState && state!=NormalState) return;
                    const GLFWbool iconified=(state==IconicState);
                    if (window->x11.iconified!=iconified) {
                        if (window->monitor) { if (iconified) releaseMonitor(window); else acquireMonitor(window); }
                        window->x11.iconified=iconified;
                        _glfwInputWindowIconify(window,iconified);
                    }
                } else if (event->xproperty.atom==_glfw.x11.NET_WM_STATE) {
                    const GLFWbool maximized=_glfwWindowMaximizedX11(window);
                    if (window->x11.maximized!=maximized) { window->x11.maximized=maximized; _glfwInputWindowMaximize(window,maximized); }
                }
                return;
            }
            case DestroyNotify: return;
        }
    }

    unsigned long _glfwGetWindowPropertyX11(Window window,Atom property,Atom type,unsigned char** value) {
        Atom actualType; int actualFormat; unsigned long itemCount,bytesAfter;
        XGetWindowProperty(_glfw.x11.display,window,property,0,2147483647,False,type,&actualType,&actualFormat,&itemCount,&bytesAfter,value);
        return itemCount;
    }

    GLFWbool _glfwIsVisualTransparentX11(Visual* visual) {
        if (!_glfw.x11.xrender.available) return GLFW_FALSE;
        XRenderPictFormat* pf=XRenderFindVisualFormat(_glfw.x11.display,visual);
        return pf && pf->direct.alphaMask;
    }

    void _glfwCreateInputContextX11(_GLFWwindow* window) {
        XIMCallback callback; callback.callback=(XIMProc)inputContextDestroyCallback; callback.client_data=(XPointer)window;
        window->x11.ic=XCreateIC(_glfw.x11.im,XNInputStyle,XIMPreeditNothing|XIMStatusNothing,XNClientWindow,window->x11.handle,XNFocusWindow,window->x11.handle,XNDestroyCallback,&callback,NULL);
        if (window->x11.ic) {
            XWindowAttributes attribs; XGetWindowAttributes(_glfw.x11.display,window->x11.handle,&attribs);
            unsigned long filter=0;
            if (XGetICValues(window->x11.ic,XNFilterEvents,&filter,NULL)==NULL) XSelectInput(_glfw.x11.display,window->x11.handle,attribs.your_event_mask|filter);
        }
    }

    void _glfwSetWindowTitleX11(_GLFWwindow* window,const char* title) {
        if (_glfw.x11.xlib.utf8) Xutf8SetWMProperties(_glfw.x11.display,window->x11.handle,title,title,NULL,0,NULL,NULL,NULL);
        XChangeProperty(_glfw.x11.display,window->x11.handle,_glfw.x11.NET_WM_NAME,_glfw.x11.UTF8_STRING,8,PropModeReplace,(unsigned char*)title,strlen(title));
        XChangeProperty(_glfw.x11.display,window->x11.handle,_glfw.x11.NET_WM_ICON_NAME,_glfw.x11.UTF8_STRING,8,PropModeReplace,(unsigned char*)title,strlen(title));
        XFlush(_glfw.x11.display);
    }

    void _glfwSetWindowIconX11(_GLFWwindow* window,int count,const GLFWimage* images) {
        if (count) {
            int longCount=0;
            for (int i=0;i<count;i++) longCount+=2+images[i].width*images[i].height;
            unsigned long* icon=_glfw_calloc(longCount,sizeof(unsigned long)),*target=icon;
            for (int i=0;i<count;i++) {
                *target++=images[i].width; *target++=images[i].height;
                for (int j=0;j<images[i].width*images[i].height;++j)
                    *target++=(((unsigned long)images[i].pixels[j*4+0])<<16)|(((unsigned long)images[i].pixels[j*4+1])<<8)|(((unsigned long)images[i].pixels[j*4+2])<<0)|(((unsigned long)images[i].pixels[j*4+3])<<24);
            }
            XChangeProperty(_glfw.x11.display,window->x11.handle,_glfw.x11.NET_WM_ICON,XA_CARDINAL,32,PropModeReplace,(unsigned char*)icon,longCount);
            free(icon);
        } else XDeleteProperty(_glfw.x11.display,window->x11.handle,_glfw.x11.NET_WM_ICON);
        XFlush(_glfw.x11.display);
    }

    void _glfwGetWindowPosX11(_GLFWwindow* window,int* xpos,int* ypos) {
        Window dummy; int x,y;
        XTranslateCoordinates(_glfw.x11.display,window->x11.handle,_glfw.x11.root,0,0,&x,&y,&dummy);
        if (xpos) *xpos=x; if (ypos) *ypos=y;
    }

    void _glfwSetWindowPosX11(_GLFWwindow* window,int xpos,int ypos) {
        if (!_glfwWindowVisibleX11(window)) {
            long supplied; XSizeHints* hints=XAllocSizeHints();
            if (XGetWMNormalHints(_glfw.x11.display,window->x11.handle,hints,&supplied)) { hints->flags|=PPosition; hints->x=hints->y=0; XSetWMNormalHints(_glfw.x11.display,window->x11.handle,hints); }
            XFree(hints);
        }
        XMoveWindow(_glfw.x11.display,window->x11.handle,xpos,ypos);
        XFlush(_glfw.x11.display);
    }

    void _glfwGetWindowSizeX11(_GLFWwindow* window,int* width,int* height) {
        XWindowAttributes attribs; XGetWindowAttributes(_glfw.x11.display,window->x11.handle,&attribs);
        if (width) *width=attribs.width; if (height) *height=attribs.height;
    }

    void _glfwSetWindowSizeX11(_GLFWwindow* window,int width,int height) {
        width=vmax(1,width); height=vmax(1,height);
        if (window->monitor) { if (window->monitor->window==window) acquireMonitor(window); }
        else { if (!window->resizable) updateNormalHints(window,width,height); XResizeWindow(_glfw.x11.display,window->x11.handle,width,height); }
        XFlush(_glfw.x11.display);
    }

    void _glfwSetWindowSizeLimitsX11(_GLFWwindow* window,int minwidth,int minheight,int maxwidth,int maxheight) {
        int width,height; (void)minwidth;(void)minheight;(void)maxwidth;(void)maxheight;
        _glfwGetWindowSizeX11(window,&width,&height); updateNormalHints(window,width,height); XFlush(_glfw.x11.display);
    }

    void _glfwSetWindowAspectRatioX11(_GLFWwindow* window,int numer,int denom) {
        int width,height; (void)numer;(void)denom;
        _glfwGetWindowSizeX11(window,&width,&height); updateNormalHints(window,width,height); XFlush(_glfw.x11.display);
    }

    void _glfwGetFramebufferSizeX11(_GLFWwindow* window,int* width,int* height) { _glfwGetWindowSizeX11(window,width,height); }

    void _glfwGetWindowFrameSizeX11(_GLFWwindow* window,int* left,int* top,int* right,int* bottom) {
        long* extents=NULL;
        if (window->monitor || !window->decorated || _glfw.x11.NET_FRAME_EXTENTS==None) return;
        if (!_glfwWindowVisibleX11(window) && _glfw.x11.NET_REQUEST_FRAME_EXTENTS) {
            XEvent event; double timeout=0.5;
            sendEventToWM(window,_glfw.x11.NET_REQUEST_FRAME_EXTENTS,0,0,0,0,0);
            while (!XCheckIfEvent(_glfw.x11.display,&event,isFrameExtentsEvent,(XPointer)window)) {
                if (!waitForX11Event(&timeout)) { _glfwInputError(GLFW_PLATFORM_ERROR,"X11: The window manager has a broken _NET_REQUEST_FRAME_EXTENTS implementation; please report this issue"); return; }
            }
        }
        if (_glfwGetWindowPropertyX11(window->x11.handle,_glfw.x11.NET_FRAME_EXTENTS,XA_CARDINAL,(unsigned char**)&extents)==4) {
            if (left) *left=extents[0]; if (top) *top=extents[2]; if (right) *right=extents[1]; if (bottom) *bottom=extents[3];
        }
        if (extents) XFree(extents);
    }

    void _glfwGetWindowContentScaleX11(_GLFWwindow* window,float* xscale,float* yscale) {
        (void)window;
        if (xscale) *xscale=_glfw.x11.contentScaleX; if (yscale) *yscale=_glfw.x11.contentScaleY;
    }

    void _glfwIconifyWindowX11(_GLFWwindow* window) {
        if (window->x11.overrideRedirect) { _glfwInputError(GLFW_PLATFORM_ERROR,"X11: Iconification of full screen windows requires a WM that supports EWMH full screen"); return; }
        XIconifyWindow(_glfw.x11.display,window->x11.handle,_glfw.x11.screen);
        XFlush(_glfw.x11.display);
    }

    void _glfwRestoreWindowX11(_GLFWwindow* window) {
        if (window->x11.overrideRedirect) { _glfwInputError(GLFW_PLATFORM_ERROR,"X11: Iconification of full screen windows requires a WM that supports EWMH full screen"); return; }
        if (_glfwWindowIconifiedX11(window)) { XMapWindow(_glfw.x11.display,window->x11.handle); waitForVisibilityNotify(window); }
        else if (_glfwWindowVisibleX11(window) && _glfw.x11.NET_WM_STATE && _glfw.x11.NET_WM_STATE_MAXIMIZED_VERT && _glfw.x11.NET_WM_STATE_MAXIMIZED_HORZ)
            sendEventToWM(window,_glfw.x11.NET_WM_STATE,_NET_WM_STATE_REMOVE,_glfw.x11.NET_WM_STATE_MAXIMIZED_VERT,_glfw.x11.NET_WM_STATE_MAXIMIZED_HORZ,1,0);
        XFlush(_glfw.x11.display);
    }

    void _glfwMaximizeWindowX11(_GLFWwindow* window) {
        if (!_glfw.x11.NET_WM_STATE || !_glfw.x11.NET_WM_STATE_MAXIMIZED_VERT || !_glfw.x11.NET_WM_STATE_MAXIMIZED_HORZ) return;
        if (_glfwWindowVisibleX11(window)) {
            sendEventToWM(window,_glfw.x11.NET_WM_STATE,_NET_WM_STATE_ADD,_glfw.x11.NET_WM_STATE_MAXIMIZED_VERT,_glfw.x11.NET_WM_STATE_MAXIMIZED_HORZ,1,0);
        } else {
            Atom* states=NULL;
            unsigned long count=_glfwGetWindowPropertyX11(window->x11.handle,_glfw.x11.NET_WM_STATE,XA_ATOM,(unsigned char**)&states);
            Atom missing[2]={_glfw.x11.NET_WM_STATE_MAXIMIZED_VERT,_glfw.x11.NET_WM_STATE_MAXIMIZED_HORZ};
            unsigned long missingCount=2;
            for (unsigned long i=0;i<count;i++) {
                for (unsigned long j=0;j<missingCount;j++) { if (states[i]==missing[j]) { missing[j]=missing[missingCount-1]; missingCount--; } }
            }
            if (states) XFree(states);
            if (!missingCount) return;
            XChangeProperty(_glfw.x11.display,window->x11.handle,_glfw.x11.NET_WM_STATE,XA_ATOM,32,PropModeAppend,(unsigned char*)missing,missingCount);
        }
        XFlush(_glfw.x11.display);
    }

    void _glfwShowWindowX11(_GLFWwindow* window) {
        if (_glfwWindowVisibleX11(window)) return;
        if (window->floating && _glfw.x11.NET_WM_STATE && _glfw.x11.NET_WM_STATE_ABOVE) {
            Atom* states=NULL;
            const unsigned long count=_glfwGetWindowPropertyX11(window->x11.handle,_glfw.x11.NET_WM_STATE,XA_ATOM,(unsigned char**)&states);
            unsigned long i;
            for (i=0;i<count;i++) { if (states[i]==_glfw.x11.NET_WM_STATE_ABOVE) break; }
            if (i==count) XChangeProperty(_glfw.x11.display,window->x11.handle,_glfw.x11.NET_WM_STATE,XA_ATOM,32,PropModeAppend,(unsigned char*)&_glfw.x11.NET_WM_STATE_ABOVE,1);
            if (states) XFree(states);
        }
        XMapWindow(_glfw.x11.display,window->x11.handle);
        waitForVisibilityNotify(window);
    }

    void _glfwHideWindowX11(_GLFWwindow* window) { XUnmapWindow(_glfw.x11.display,window->x11.handle); XFlush(_glfw.x11.display); }

    void _glfwRequestWindowAttentionX11(_GLFWwindow* window) {
        if (!_glfw.x11.NET_WM_STATE || !_glfw.x11.NET_WM_STATE_DEMANDS_ATTENTION) return;
        sendEventToWM(window,_glfw.x11.NET_WM_STATE,_NET_WM_STATE_ADD,_glfw.x11.NET_WM_STATE_DEMANDS_ATTENTION,0,1,0);
    }

    void _glfwFocusWindowX11(_GLFWwindow* window) {
        if (_glfw.x11.NET_ACTIVE_WINDOW) sendEventToWM(window,_glfw.x11.NET_ACTIVE_WINDOW,1,0,0,0,0);
        else if (_glfwWindowVisibleX11(window)) { XRaiseWindow(_glfw.x11.display,window->x11.handle); XSetInputFocus(_glfw.x11.display,window->x11.handle,RevertToParent,CurrentTime); }
        XFlush(_glfw.x11.display);
    }

    void _glfwSetWindowMonitorX11(_GLFWwindow* window,_GLFWmonitor* monitor,int xpos,int ypos,int width,int height,int refreshRate) {
        (void)refreshRate;
        if (window->monitor==monitor) {
            if (monitor) { if (monitor->window==window) acquireMonitor(window); }
            else { if (!window->resizable) updateNormalHints(window,width,height); XMoveResizeWindow(_glfw.x11.display,window->x11.handle,xpos,ypos,width,height); }
            XFlush(_glfw.x11.display); return;
        }
        if (window->monitor) { _glfwSetWindowDecoratedX11(window,window->decorated); _glfwSetWindowFloatingX11(window,window->floating); releaseMonitor(window); }
        _glfwInputWindowMonitor(window,monitor);
        updateNormalHints(window,width,height);
        if (window->monitor) {
            if (!_glfwWindowVisibleX11(window)) { XMapRaised(_glfw.x11.display,window->x11.handle); waitForVisibilityNotify(window); }
            updateWindowMode(window); acquireMonitor(window);
        } else { updateWindowMode(window); XMoveResizeWindow(_glfw.x11.display,window->x11.handle,xpos,ypos,width,height); }
        XFlush(_glfw.x11.display);
    }

    GLFWbool _glfwWindowFocusedX11(_GLFWwindow* window) { Window focused; int state; XGetInputFocus(_glfw.x11.display,&focused,&state); return window->x11.handle==focused; }
    GLFWbool _glfwWindowIconifiedX11(_GLFWwindow* window) { return getWindowState(window)==IconicState; }
    GLFWbool _glfwWindowVisibleX11(_GLFWwindow* window) { XWindowAttributes wa; XGetWindowAttributes(_glfw.x11.display,window->x11.handle,&wa); return wa.map_state==IsViewable; }

    static GLFWbool createNativeWindow(_GLFWwindow* window,const _GLFWwndconfig* wndconfig,Visual* visual,int depth) {
        int width=wndconfig->width,height=wndconfig->height;
        if (wndconfig->scaleToMonitor) { width*=_glfw.x11.contentScaleX; height*=_glfw.x11.contentScaleY; }
        width=vmax(1,width); height=vmax(1,height);
        int xpos=0,ypos=0;
        if (wndconfig->xpos!=(int)GLFW_ANY_POSITION && wndconfig->ypos!=(int)GLFW_ANY_POSITION) { xpos=wndconfig->xpos; ypos=wndconfig->ypos; }
        window->x11.colormap=XCreateColormap(_glfw.x11.display,_glfw.x11.root,visual,AllocNone);
        window->x11.transparent=_glfwIsVisualTransparentX11(visual);
        XSetWindowAttributes wa={0};
        wa.colormap=window->x11.colormap;
        wa.event_mask=StructureNotifyMask|KeyPressMask|KeyReleaseMask|PointerMotionMask|ButtonPressMask|ButtonReleaseMask|ExposureMask|FocusChangeMask|VisibilityChangeMask|EnterWindowMask|LeaveWindowMask|PropertyChangeMask;
        _glfwGrabErrorHandlerX11();
        window->x11.parent=_glfw.x11.root;
        window->x11.handle=XCreateWindow(_glfw.x11.display,_glfw.x11.root,xpos,ypos,width,height,0,depth,InputOutput,visual,CWBorderPixel|CWColormap|CWEventMask,&wa);
        _glfwReleaseErrorHandlerX11();
        if (!window->x11.handle) { _glfwInputErrorX11(GLFW_PLATFORM_ERROR,"X11: Failed to create window"); return GLFW_FALSE; }
        XSaveContext(_glfw.x11.display,window->x11.handle,_glfw.x11.context,(XPointer)window);
        if (!wndconfig->decorated) _glfwSetWindowDecoratedX11(window,GLFW_FALSE);
        if (_glfw.x11.NET_WM_STATE && !window->monitor) {
            Atom states[3]; int count=0;
            if (wndconfig->floating && _glfw.x11.NET_WM_STATE_ABOVE) states[count++]=_glfw.x11.NET_WM_STATE_ABOVE;
            if (wndconfig->maximized && _glfw.x11.NET_WM_STATE_MAXIMIZED_VERT && _glfw.x11.NET_WM_STATE_MAXIMIZED_HORZ) {
                states[count++]=_glfw.x11.NET_WM_STATE_MAXIMIZED_VERT; states[count++]=_glfw.x11.NET_WM_STATE_MAXIMIZED_HORZ;
                window->x11.maximized=GLFW_TRUE;
            }
            if (count) XChangeProperty(_glfw.x11.display,window->x11.handle,_glfw.x11.NET_WM_STATE,XA_ATOM,32,PropModeReplace,(unsigned char*)states,count);
        }
        { Atom protocols[]={_glfw.x11.WM_DELETE_WINDOW,_glfw.x11.NET_WM_PING}; XSetWMProtocols(_glfw.x11.display,window->x11.handle,protocols,sizeof(protocols)/sizeof(Atom)); }
        { const long pid=getpid(); XChangeProperty(_glfw.x11.display,window->x11.handle,_glfw.x11.NET_WM_PID,XA_CARDINAL,32,PropModeReplace,(unsigned char*)&pid,1); }
        if (_glfw.x11.NET_WM_WINDOW_TYPE && _glfw.x11.NET_WM_WINDOW_TYPE_NORMAL) { Atom type=_glfw.x11.NET_WM_WINDOW_TYPE_NORMAL; XChangeProperty(_glfw.x11.display,window->x11.handle,_glfw.x11.NET_WM_WINDOW_TYPE,XA_ATOM,32,PropModeReplace,(unsigned char*)&type,1); }
        {
            XWMHints* hints=XAllocWMHints();
            if (!hints) { _glfwInputError(GLFW_OUT_OF_MEMORY,"X11: Failed to allocate WM hints"); return GLFW_FALSE; }
            hints->flags=StateHint; hints->initial_state=NormalState;
            XSetWMHints(_glfw.x11.display,window->x11.handle,hints); XFree(hints);
        }
        {
            XSizeHints* hints=XAllocSizeHints();
            if (!hints) { _glfwInputError(GLFW_OUT_OF_MEMORY,"X11: Failed to allocate size hints"); return GLFW_FALSE; }
            if (!wndconfig->resizable) { hints->flags|=(PMinSize|PMaxSize); hints->min_width=hints->max_width=width; hints->min_height=hints->max_height=height; }
            if (wndconfig->xpos!=(int)GLFW_ANY_POSITION && wndconfig->ypos!=(int)GLFW_ANY_POSITION) { hints->flags|=PPosition; hints->x=0; hints->y=0; }
            hints->flags|=PWinGravity; hints->win_gravity=StaticGravity;
            XSetWMNormalHints(_glfw.x11.display,window->x11.handle,hints); XFree(hints);
        }
        {
            XClassHint* hint=XAllocClassHint();
            if (strlen(wndconfig->x11.instanceName) && strlen(wndconfig->x11.className)) {
                hint->res_name=(char*)wndconfig->x11.instanceName; hint->res_class=(char*)wndconfig->x11.className;
            } else {
                const char* resourceName=getenv("RESOURCE_NAME");
                hint->res_name=(char*)(resourceName&&strlen(resourceName)?resourceName:(strlen(window->title)?window->title:"glfw-application"));
                hint->res_class=(char*)(strlen(window->title)?window->title:"GLFW-Application");
            }
            XSetClassHint(_glfw.x11.display,window->x11.handle,hint); XFree(hint);
        }
        if (_glfw.x11.im) _glfwCreateInputContextX11(window);
        _glfwSetWindowTitleX11(window,window->title);
        _glfwGetWindowPosX11(window,&window->x11.xpos,&window->x11.ypos);
        _glfwGetWindowSizeX11(window,&window->x11.width,&window->x11.height);
        return GLFW_TRUE;
    }
    
    GLFWbool _glfwWindowMaximizedX11(_GLFWwindow* window) {
        Atom* states; GLFWbool maximized=GLFW_FALSE;
        if (!_glfw.x11.NET_WM_STATE || !_glfw.x11.NET_WM_STATE_MAXIMIZED_VERT || !_glfw.x11.NET_WM_STATE_MAXIMIZED_HORZ) return maximized;
        const unsigned long count=_glfwGetWindowPropertyX11(window->x11.handle,_glfw.x11.NET_WM_STATE,XA_ATOM,(unsigned char**)&states);
        for (unsigned long i=0;i<count;i++) {
            if (states[i]==_glfw.x11.NET_WM_STATE_MAXIMIZED_VERT || states[i]==_glfw.x11.NET_WM_STATE_MAXIMIZED_HORZ) { maximized=GLFW_TRUE; break; }
        }
        if (states) XFree(states);
        return maximized;
    }

    GLFWbool _glfwWindowHoveredX11(_GLFWwindow* window) {
        Window w=_glfw.x11.root;
        while (w) {
            Window root; int rootX,rootY,childX,childY; unsigned int mask;
            _glfwGrabErrorHandlerX11();
            const Bool result=XQueryPointer(_glfw.x11.display,w,&root,&w,&rootX,&rootY,&childX,&childY,&mask);
            _glfwReleaseErrorHandlerX11();
            if (_glfw.x11.errorCode==BadWindow) w=_glfw.x11.root;
            else if (!result) return GLFW_FALSE;
            else if (w==window->x11.handle) return GLFW_TRUE;
        }
        return GLFW_FALSE;
    }

    GLFWbool _glfwFramebufferTransparentX11(_GLFWwindow* window) {
        if (!window->x11.transparent) return GLFW_FALSE;
        return XGetSelectionOwner(_glfw.x11.display,_glfw.x11.NET_WM_CM_Sx)!=None;
    }

    void _glfwSetWindowResizableX11(_GLFWwindow* window,GLFWbool enabled) { int width,height; (void)enabled; _glfwGetWindowSizeX11(window,&width,&height); updateNormalHints(window,width,height); }

    void _glfwSetWindowDecoratedX11(_GLFWwindow* window,GLFWbool enabled) {
        struct { unsigned long flags,functions,decorations; long input_mode; unsigned long status; } hints={0};
        hints.flags=MWM_HINTS_DECORATIONS; hints.decorations=enabled?MWM_DECOR_ALL:0;
        XChangeProperty(_glfw.x11.display,window->x11.handle,_glfw.x11.MOTIF_WM_HINTS,_glfw.x11.MOTIF_WM_HINTS,32,PropModeReplace,(unsigned char*)&hints,sizeof(hints)/sizeof(long));
    }

    void _glfwSetWindowFloatingX11(_GLFWwindow* window,GLFWbool enabled) {
        if (!_glfw.x11.NET_WM_STATE || !_glfw.x11.NET_WM_STATE_ABOVE) return;
        if (_glfwWindowVisibleX11(window)) {
            sendEventToWM(window,_glfw.x11.NET_WM_STATE,enabled?_NET_WM_STATE_ADD:_NET_WM_STATE_REMOVE,_glfw.x11.NET_WM_STATE_ABOVE,0,1,0);
        } else {
            if (enabled) return;
            Atom* states=NULL;
            const unsigned long count=_glfwGetWindowPropertyX11(window->x11.handle,_glfw.x11.NET_WM_STATE,XA_ATOM,(unsigned char**)&states);
            unsigned long i;
            for (i=0;i<count;i++) { if (states[i]==_glfw.x11.NET_WM_STATE_ABOVE) break; }
            if (i<count) { states[i]=states[count-1]; XChangeProperty(_glfw.x11.display,window->x11.handle,_glfw.x11.NET_WM_STATE,XA_ATOM,32,PropModeReplace,(unsigned char*)states,count-1); }
            if (states) XFree(states);
        }
        XFlush(_glfw.x11.display);
    }

    void _glfwSetWindowMousePassthroughX11(_GLFWwindow* window,GLFWbool enabled) {
        if (!_glfw.x11.xshape.available) return;
        if (enabled) { Region region=XCreateRegion(); XShapeCombineRegion(_glfw.x11.display,window->x11.handle,ShapeInput,0,0,region,ShapeSet); XDestroyRegion(region); }
        else XShapeCombineMask(_glfw.x11.display,window->x11.handle,ShapeInput,0,0,None,ShapeSet);
    }

    float _glfwGetWindowOpacityX11(_GLFWwindow* window) {
        float opacity=1.f;
        if (XGetSelectionOwner(_glfw.x11.display,_glfw.x11.NET_WM_CM_Sx)) {
            u32* value=NULL;
            if (_glfwGetWindowPropertyX11(window->x11.handle,_glfw.x11.NET_WM_WINDOW_OPACITY,XA_CARDINAL,(unsigned char**)&value)) opacity=(float)(*value/(double)0xffffffffu);
            if (value) XFree(value);
        }
        return opacity;
    }

    void _glfwSetWindowOpacityX11(_GLFWwindow* window,float opacity) {
        const u32 value=(u32)(0xffffffffu*(double)opacity);
        XChangeProperty(_glfw.x11.display,window->x11.handle,_glfw.x11.NET_WM_WINDOW_OPACITY,XA_CARDINAL,32,PropModeReplace,(unsigned char*)&value,1);
    }

    void _glfwSetRawMouseMotionX11(_GLFWwindow* window,GLFWbool enabled) {
        if (!_glfw.x11.xi.available || _glfw.x11.disabledCursorWindow!=window) return;
        if (enabled) enableRawMouseMotion(window); else disableRawMouseMotion(window);
    }

    GLFWbool _glfwRawMouseMotionSupportedX11(void) { return _glfw.x11.xi.available; }

    void _glfwPollEventsX11(void) {
        drainEmptyEvents();
        if (_glfw.joysticksInitialized) _glfwDetectJoystickConnectionLinux();
        XPending(_glfw.x11.display);
        while (QLength(_glfw.x11.display)) { XEvent event; XNextEvent(_glfw.x11.display,&event); processEvent(&event); }
        _GLFWwindow* window=_glfw.x11.disabledCursorWindow;
        if (window) {
            int width,height; _glfwGetWindowSizeX11(window,&width,&height);
            if (window->x11.lastCursorPosX!=width/2 || window->x11.lastCursorPosY!=height/2) _glfwSetCursorPosX11(window,width/2,height/2);
        }
        XFlush(_glfw.x11.display);
    }

    void _glfwWaitEventsX11(void) { waitForAnyEvent(NULL); _glfwPollEventsX11(); }
    void _glfwWaitEventsTimeoutX11(double timeout) { waitForAnyEvent(&timeout); _glfwPollEventsX11(); }
    void _glfwPostEmptyEventX11(void) { writeEmptyEvent(); }

    void _glfwGetCursorPosX11(_GLFWwindow* window,double* xpos,double* ypos) {
        Window root,child; int rootX,rootY,childX,childY; unsigned int mask;
        XQueryPointer(_glfw.x11.display,window->x11.handle,&root,&child,&rootX,&rootY,&childX,&childY,&mask);
        if (xpos) *xpos=childX; if (ypos) *ypos=childY;
    }

    void _glfwSetCursorPosX11(_GLFWwindow* window,double x,double y) {
        window->x11.warpCursorPosX=(int)x; window->x11.warpCursorPosY=(int)y;
        XWarpPointer(_glfw.x11.display,None,window->x11.handle,0,0,0,0,(int)x,(int)y);
        XFlush(_glfw.x11.display);
    }

    void _glfwSetCursorModeX11(_GLFWwindow* window,int mode) {
        if (_glfwWindowFocusedX11(window)) {
            if (mode==GLFW_CURSOR_DISABLED) {
                _glfwGetCursorPosX11(window,&_glfw.x11.restoreCursorPosX,&_glfw.x11.restoreCursorPosY);
                _glfwCenterCursorInContentArea(window);
                if (window->rawMouseMotion) enableRawMouseMotion(window);
            } else if (_glfw.x11.disabledCursorWindow==window) { if (window->rawMouseMotion) disableRawMouseMotion(window); }
            if (mode==GLFW_CURSOR_DISABLED || mode==GLFW_CURSOR_CAPTURED) captureCursor(window); else releaseCursor();
            if (mode==GLFW_CURSOR_DISABLED) _glfw.x11.disabledCursorWindow=window;
            else if (_glfw.x11.disabledCursorWindow==window) { _glfw.x11.disabledCursorWindow=NULL; _glfwSetCursorPosX11(window,_glfw.x11.restoreCursorPosX,_glfw.x11.restoreCursorPosY); }
        }
        updateCursorImage(window); XFlush(_glfw.x11.display);
    }

    void _glfwDestroyCursorX11(_GLFWcursor* cursor) { if (cursor->x11.handle) XFreeCursor(_glfw.x11.display,cursor->x11.handle); }

    void _glfwSetCursorX11(_GLFWwindow* window,_GLFWcursor* cursor) {
        (void)cursor;
        if (window->cursorMode==GLFW_CURSOR_NORMAL || window->cursorMode==GLFW_CURSOR_CAPTURED) { updateCursorImage(window); XFlush(_glfw.x11.display); }
    }

    static GLFWbool modeIsGood(const XRRModeInfo* mi) { return (mi->modeFlags & RR_Interlace) == 0; }

    static const XRRModeInfo* getModeInfo(const XRRScreenResources* sr, RRMode id) {
        for (int i = 0;  i < sr->nmode;  i++){
            if (sr->modes[i].id == id) return sr->modes + i;
        }

        return NULL;
    }

    // Convert RandR mode info to GLFW video mode
    //
    static GLFWvidmode vidmodeFromModeInfo(const XRRModeInfo* mi, const XRRCrtcInfo* ci) {
        GLFWvidmode mode;
        if (ci->rotation == RR_Rotate_90 || ci->rotation == RR_Rotate_270) {
            mode.width  = mi->height;
            mode.height = mi->width;
        } else {
            mode.width  = mi->width;
            mode.height = mi->height;
        }
        
        mode.refreshRate = (mi->hTotal && mi->vTotal) ? (int)vround((double) mi->dotClock / ((double) mi->hTotal * (double) mi->vTotal)) : 0;
        _glfwSplitBPP(DefaultDepth(_glfw.x11.display,_glfw.x11.screen),&mode.redBits,&mode.greenBits,&mode.blueBits);
        return mode;
    }

    void _glfwPollMonitorsX11(void) {
        if (_glfw.x11.randr.available && !_glfw.x11.randr.monitorBroken) {
            int disconnectedCount, screenCount = 0;
            _GLFWmonitor** disconnected = NULL;
            XineramaScreenInfo* screens = NULL;
            XRRScreenResources* sr = XRRGetScreenResourcesCurrent(_glfw.x11.display,_glfw.x11.root);
            RROutput primary = XRRGetOutputPrimary(_glfw.x11.display,_glfw.x11.root);
            if (_glfw.x11.xinerama.available) screens = XineramaQueryScreens(_glfw.x11.display, &screenCount);
            disconnectedCount = _glfw.monitorCount;
            if (disconnectedCount) {
                disconnected = _glfw_calloc(_glfw.monitorCount, sizeof(_GLFWmonitor*));
                __builtin_memcpy(disconnected,_glfw.monitors,_glfw.monitorCount * sizeof(_GLFWmonitor*));
            }

            for (int i = 0;  i < sr->noutput;  i++) {
                int j, type, widthMM, heightMM;
                XRROutputInfo* oi = XRRGetOutputInfo(_glfw.x11.display, sr, sr->outputs[i]);
                if (oi->connection != RR_Connected || oi->crtc == None) { XRRFreeOutputInfo(oi); continue; }

                for (j = 0;  j < disconnectedCount;  j++) {
                    if (disconnected[j] && disconnected[j]->x11.output == sr->outputs[i]) { disconnected[j] = NULL; break; }
                }

                if (j < disconnectedCount) { XRRFreeOutputInfo(oi); continue; }

                XRRCrtcInfo* ci = XRRGetCrtcInfo(_glfw.x11.display, sr, oi->crtc);
                if (!ci)
                {
                    XRRFreeOutputInfo(oi);
                    continue;
                }

                if (ci->rotation == RR_Rotate_90 || ci->rotation == RR_Rotate_270)
                {
                    widthMM  = oi->mm_height;
                    heightMM = oi->mm_width;
                }
                else
                {
                    widthMM  = oi->mm_width;
                    heightMM = oi->mm_height;
                }

                if (widthMM <= 0 || heightMM <= 0)
                {
                    // HACK: If RandR does not provide a physical size, assume the
                    //       X11 default 96 DPI and calculate from the CRTC viewport
                    // NOTE: These members are affected by rotation, unlike the mode
                    //       info and output info members
                    widthMM  = (int) (ci->width * 25.4f / 96.f);
                    heightMM = (int) (ci->height * 25.4f / 96.f);
                }

                _GLFWmonitor* monitor = _glfwAllocMonitor(oi->name, widthMM, heightMM);
                monitor->x11.output = sr->outputs[i];
                monitor->x11.crtc   = oi->crtc;
                for (j = 0;  j < screenCount;  j++) {
                    if (screens[j].x_org == ci->x && screens[j].y_org == ci->y && screens[j].width == (short)ci->width && screens[j].height == (short)ci->height) { monitor->x11.index = j; break; }
                }

                if (monitor->x11.output == primary) type = _GLFW_INSERT_FIRST;
                else type = _GLFW_INSERT_LAST;

                _glfwInputMonitor(monitor, GLFW_CONNECTED, type);
                XRRFreeOutputInfo(oi);
                XRRFreeCrtcInfo(ci);
            }

            XRRFreeScreenResources(sr);
            if (screens) XFree(screens);
            for (int i = 0;  i < disconnectedCount;  i++) {
                if (disconnected[i]) _glfwInputMonitor(disconnected[i], GLFW_DISCONNECTED, 0);
            }

            free(disconnected);
        } else {
            const int widthMM = DisplayWidthMM(_glfw.x11.display, _glfw.x11.screen);
            const int heightMM = DisplayHeightMM(_glfw.x11.display, _glfw.x11.screen);
            _glfwInputMonitor(_glfwAllocMonitor("Display", widthMM, heightMM),GLFW_CONNECTED,_GLFW_INSERT_FIRST);
        }
    }

    // Set the current video mode for the specified monitor
    //
    void _glfwSetVideoModeX11(_GLFWmonitor* monitor, const GLFWvidmode* desired) {
        if (_glfw.x11.randr.available && !_glfw.x11.randr.monitorBroken) {
            GLFWvidmode current;
            RRMode native = None;

            const GLFWvidmode* best = _glfwChooseVideoMode(monitor, desired);
            _glfwGetVideoModeX11(monitor, &current);
            if (_glfwCompareVideoModes(&current, best) == 0)
                return;

            XRRScreenResources* sr =
                XRRGetScreenResourcesCurrent(_glfw.x11.display, _glfw.x11.root);
            XRRCrtcInfo* ci = XRRGetCrtcInfo(_glfw.x11.display, sr, monitor->x11.crtc);
            XRROutputInfo* oi = XRRGetOutputInfo(_glfw.x11.display, sr, monitor->x11.output);

            for (int i = 0;  i < oi->nmode;  i++)
            {
                const XRRModeInfo* mi = getModeInfo(sr, oi->modes[i]);
                if (!modeIsGood(mi))
                    continue;

                const GLFWvidmode mode = vidmodeFromModeInfo(mi, ci);
                if (_glfwCompareVideoModes(best, &mode) == 0)
                {
                    native = mi->id;
                    break;
                }
            }

            if (native)
            {
                if (monitor->x11.oldMode == None)
                    monitor->x11.oldMode = ci->mode;

                XRRSetCrtcConfig(_glfw.x11.display,
                                sr, monitor->x11.crtc,
                                CurrentTime,
                                ci->x, ci->y,
                                native,
                                ci->rotation,
                                ci->outputs,
                                ci->noutput);
            }

            XRRFreeOutputInfo(oi);
            XRRFreeCrtcInfo(ci);
            XRRFreeScreenResources(sr);
        }
    }

    // Restore the saved (original) video mode for the specified monitor
    //
    void _glfwRestoreVideoModeX11(_GLFWmonitor* monitor)
    {
        if (_glfw.x11.randr.available && !_glfw.x11.randr.monitorBroken)
        {
            if (monitor->x11.oldMode == None)
                return;

            XRRScreenResources* sr =
                XRRGetScreenResourcesCurrent(_glfw.x11.display, _glfw.x11.root);
            XRRCrtcInfo* ci = XRRGetCrtcInfo(_glfw.x11.display, sr, monitor->x11.crtc);

            XRRSetCrtcConfig(_glfw.x11.display,
                            sr, monitor->x11.crtc,
                            CurrentTime,
                            ci->x, ci->y,
                            monitor->x11.oldMode,
                            ci->rotation,
                            ci->outputs,
                            ci->noutput);

            XRRFreeCrtcInfo(ci);
            XRRFreeScreenResources(sr);

            monitor->x11.oldMode = None;
        }
    }

    void _glfwGetMonitorPosX11(_GLFWmonitor* monitor, int* xpos, int* ypos) {
        if (_glfw.x11.randr.available && !_glfw.x11.randr.monitorBroken) {
            XRRScreenResources* sr = XRRGetScreenResourcesCurrent(_glfw.x11.display, _glfw.x11.root);
            XRRCrtcInfo* ci = XRRGetCrtcInfo(_glfw.x11.display, sr, monitor->x11.crtc);

            if (ci) {
                if (xpos) *xpos = ci->x;
                if (ypos) *ypos = ci->y;
                XRRFreeCrtcInfo(ci);
            }

            XRRFreeScreenResources(sr);
        }
    }

    void _glfwGetMonitorContentScaleX11(_GLFWmonitor* monitor, float* xscale, float* yscale) { (void)monitor; if (xscale) {*xscale = _glfw.x11.contentScaleX;} if (yscale) {*yscale = _glfw.x11.contentScaleY;} }
    void _glfwGetMonitorWorkareaX11(_GLFWmonitor* monitor,int* xpos,int* ypos,int* width,int* height) {
        int areaX = 0, areaY = 0, areaWidth = 0, areaHeight = 0;
        if (_glfw.x11.randr.available && !_glfw.x11.randr.monitorBroken) {
            XRRScreenResources* sr = XRRGetScreenResourcesCurrent(_glfw.x11.display,_glfw.x11.root);
            XRRCrtcInfo* ci = XRRGetCrtcInfo(_glfw.x11.display,sr,monitor->x11.crtc);
            const XRRModeInfo* mi = getModeInfo(sr,ci->mode);
            areaX = ci->x, areaY = ci->y;
            if (ci->rotation == RR_Rotate_90 || ci->rotation == RR_Rotate_270) { areaWidth = mi->height, areaHeight = mi->width; }
            else { areaWidth = mi->width, areaHeight = mi->height; }
            XRRFreeCrtcInfo(ci), XRRFreeScreenResources(sr);
        } else { areaWidth = DisplayWidth(_glfw.x11.display,_glfw.x11.screen), areaHeight = DisplayHeight(_glfw.x11.display,_glfw.x11.screen); }
        if (_glfw.x11.NET_WORKAREA && _glfw.x11.NET_CURRENT_DESKTOP) {
            Atom *extents = NULL, *desktop = NULL;
            const unsigned long extentCount = _glfwGetWindowPropertyX11(_glfw.x11.root,_glfw.x11.NET_WORKAREA,XA_CARDINAL,(unsigned char**) &extents);
            if (_glfwGetWindowPropertyX11(_glfw.x11.root,_glfw.x11.NET_CURRENT_DESKTOP,XA_CARDINAL,(unsigned char**) &desktop) > 0) {
                if (extentCount >= 4 && *desktop < extentCount / 4) {
                    const int gx = extents[*desktop * 4 + 0], gy = extents[*desktop * 4 + 1], gw = extents[*desktop * 4 + 2], gh = extents[*desktop * 4 + 3];
                    if (areaX < gx) { areaWidth -= gx - areaX, areaX = gx; }
                    if (areaY < gy) { areaHeight -= gy - areaY, areaY = gy; }
                    if (areaX + areaWidth > gx + gw) areaWidth = gx - areaX + gw;
                    if (areaY + areaHeight > gy + gh) areaHeight = gy - areaY + gh;
                }
            }
            if (extents) {XFree(extents);} if (desktop) {XFree(desktop);}
        }
        if (xpos) {*xpos = areaX;} if (ypos) {*ypos = areaY;} if (width) {*width = areaWidth;} if (height) {*height = areaHeight;}
    }

    GLFWvidmode* _glfwGetVideoModesX11(_GLFWmonitor* monitor,int* count) {
        GLFWvidmode* result;
        *count = 0;
        if (_glfw.x11.randr.available && !_glfw.x11.randr.monitorBroken) {
            XRRScreenResources* sr = XRRGetScreenResourcesCurrent(_glfw.x11.display,_glfw.x11.root);
            XRRCrtcInfo* ci = XRRGetCrtcInfo(_glfw.x11.display,sr,monitor->x11.crtc);
            XRROutputInfo* oi = XRRGetOutputInfo(_glfw.x11.display,sr,monitor->x11.output);
            result = _glfw_calloc(oi->nmode,sizeof(GLFWvidmode));
            for (int i = 0; i < oi->nmode; i++) {
                const XRRModeInfo* mi = getModeInfo(sr,oi->modes[i]);
                if (!modeIsGood(mi)) continue;
                const GLFWvidmode mode = vidmodeFromModeInfo(mi,ci);
                int j;
                for (j = 0; j < *count; j++) if (_glfwCompareVideoModes(result+j,&mode) == 0) break;
                if (j < *count) continue;
                (*count)++, result[*count - 1] = mode;
            }
            XRRFreeOutputInfo(oi), XRRFreeCrtcInfo(ci), XRRFreeScreenResources(sr);
        } else {
            *count = 1, result = _glfw_calloc(1,sizeof(GLFWvidmode));
            _glfwGetVideoModeX11(monitor,result);
        }
        return result;
    }

    GLFWbool _glfwGetVideoModeX11(_GLFWmonitor* monitor,GLFWvidmode* mode) {
        if (_glfw.x11.randr.available && !_glfw.x11.randr.monitorBroken) {
            XRRScreenResources* sr = XRRGetScreenResourcesCurrent(_glfw.x11.display,_glfw.x11.root);
            const XRRModeInfo* mi = NULL;
            XRRCrtcInfo* ci = XRRGetCrtcInfo(_glfw.x11.display,sr,monitor->x11.crtc);
            if (ci) {
                mi = getModeInfo(sr,ci->mode);
                if (mi) *mode = vidmodeFromModeInfo(mi,ci);
                XRRFreeCrtcInfo(ci);
            }
            XRRFreeScreenResources(sr);
            if (!mi) { _glfwInputError(GLFW_PLATFORM_ERROR,"X11: Failed to query video mode"); return GLFW_FALSE; }
        } else {
            mode->width = DisplayWidth(_glfw.x11.display,_glfw.x11.screen), mode->height = DisplayHeight(_glfw.x11.display,_glfw.x11.screen), mode->refreshRate = 0;
            _glfwSplitBPP(DefaultDepth(_glfw.x11.display,_glfw.x11.screen),&mode->redBits,&mode->greenBits,&mode->blueBits);
        }
        return GLFW_TRUE;
    }

    int _glfwGetKeyScancodeX11(int key) { return _glfw.x11.scancodes[key]; }
    GLFWAPI RRCrtc glfwGetX11Adapter(GLFWmonitor* handle) {
        if (_glfw.platform.platformID != GLFW_PLATFORM_X11) { _glfwInputError(GLFW_PLATFORM_UNAVAILABLE, "X11: Platform not initialized"); return None; }

        _GLFWmonitor* monitor = (_GLFWmonitor*) handle;
        return monitor->x11.crtc;
    }

    GLFWAPI RROutput glfwGetX11Monitor(GLFWmonitor* handle) {
        if (_glfw.platform.platformID != GLFW_PLATFORM_X11) { _glfwInputError(GLFW_PLATFORM_UNAVAILABLE, "X11: Platform not initialized"); return None; }

        _GLFWmonitor* monitor = (_GLFWmonitor*) handle;
        return monitor->x11.output;
    }

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
            if (XkbSetDetectableAutoRepeat(_glfw.x11.display,True,&supported) && supported) _glfw.x11.xkb.detectable = GLFW_TRUE;
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
        _glfw.x11.UTF8_STRING        = IA("UTF8_STRING");
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

    static void getSystemContentScale(float* xscale, float* yscale) {
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

    static Window createHelperWindow(void) {
        XSetWindowAttributes wa; wa.event_mask = PropertyChangeMask;
        return XCreateWindow(_glfw.x11.display,_glfw.x11.root,0,0,1,1,0,0,InputOnly,DefaultVisual(_glfw.x11.display,_glfw.x11.screen),CWEventMask,&wa);
    }

    static GLFWbool createEmptyEventPipe(void) {
        if (pipe(_glfw.x11.emptyEventPipe) != 0) { _glfwInputError(GLFW_PLATFORM_ERROR, "X11: Failed to create empty event pipe: %s", strerror(errno)); return GLFW_FALSE; }
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

    static int errorHandler(Display* display, XErrorEvent* event) { if (_glfw.x11.display == display) _glfw.x11.errorCode = event->error_code; return 0; }
    void _glfwGrabErrorHandlerX11(void) { _glfw.x11.errorCode = Success; _glfw.x11.errorHandler = XSetErrorHandler(errorHandler); }
    void _glfwReleaseErrorHandlerX11(void) { XSync(_glfw.x11.display, False); XSetErrorHandler(_glfw.x11.errorHandler); _glfw.x11.errorHandler = NULL; }
    void _glfwInputErrorX11(int error, const char* message) {
        char buffer[_GLFW_MESSAGE_SIZE];
        XGetErrorText(_glfw.x11.display, _glfw.x11.errorCode, buffer, sizeof(buffer));
        _glfwInputError(error, "%s: %s", message, buffer);
    }

    GLFWbool _glfwConnectX11(void) {
        if (strcmp(setlocale(LC_CTYPE, NULL), "C") == 0) setlocale(LC_CTYPE, "");
        void* module = _glfwPlatformLoadModule("libX11.so.6");
        if (!module) { _glfwInputError(GLFW_PLATFORM_ERROR, "X11: Failed to load Xlib"); return GLFW_FALSE; }

        PFN_XInitThreads  XInitThreads  = (PFN_XInitThreads) _glfwPlatformGetModuleSymbol(module, "XInitThreads");
        PFN_XrmInitialize XrmInitialize = (PFN_XrmInitialize)_glfwPlatformGetModuleSymbol(module, "XrmInitialize");
        PFN_XOpenDisplay  XOpenDisplay  = (PFN_XOpenDisplay) _glfwPlatformGetModuleSymbol(module, "XOpenDisplay");
        if (!XInitThreads || !XrmInitialize || !XOpenDisplay) { _glfwInputError(GLFW_PLATFORM_ERROR, "X11: Failed to load Xlib entry point"); _glfwPlatformFreeModule(module); return GLFW_FALSE; }

        XInitThreads(); XrmInitialize();
        Display* display = XOpenDisplay(NULL);
        if (!display) {
            const char* name = getenv("DISPLAY");
            _glfwInputError(GLFW_PLATFORM_UNAVAILABLE, name ? "X11: Failed to open display %s" : "X11: The DISPLAY environment variable is missing", name);
            _glfwPlatformFreeModule(module); return GLFW_FALSE;
        }

        _glfw.x11.display = display;
        _glfw.x11.xlib.handle = module;
        return GLFW_TRUE;
    }

    int _glfwInitX11(void) {
        _glfwConnectX11();
        _glfw.x11.xlib.AllocClassHint = (PFN_XAllocClassHint)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XAllocClassHint");
        _glfw.x11.xlib.AllocSizeHints = (PFN_XAllocSizeHints)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XAllocSizeHints");
        _glfw.x11.xlib.AllocWMHints = (PFN_XAllocWMHints)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XAllocWMHints");
        _glfw.x11.xlib.ChangeProperty = (PFN_XChangeProperty)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XChangeProperty");
        _glfw.x11.xlib.ChangeWindowAttributes = (PFN_XChangeWindowAttributes)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XChangeWindowAttributes");
        _glfw.x11.xlib.CheckIfEvent = (PFN_XCheckIfEvent)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XCheckIfEvent");
        _glfw.x11.xlib.CheckTypedWindowEvent = (PFN_XCheckTypedWindowEvent)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XCheckTypedWindowEvent");
        _glfw.x11.xlib.CloseDisplay = (PFN_XCloseDisplay)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XCloseDisplay");
        _glfw.x11.xlib.CloseIM = (PFN_XCloseIM)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XCloseIM");
        _glfw.x11.xlib.ConvertSelection = (PFN_XConvertSelection)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XConvertSelection");
        _glfw.x11.xlib.CreateColormap = (PFN_XCreateColormap)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XCreateColormap");
        _glfw.x11.xlib.CreateFontCursor = (PFN_XCreateFontCursor)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XCreateFontCursor");
        _glfw.x11.xlib.CreateIC = (PFN_XCreateIC)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XCreateIC");
        _glfw.x11.xlib.CreateRegion = (PFN_XCreateRegion)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XCreateRegion");
        _glfw.x11.xlib.CreateWindow = (PFN_XCreateWindow)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XCreateWindow");
        _glfw.x11.xlib.DefineCursor = (PFN_XDefineCursor)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XDefineCursor");
        _glfw.x11.xlib.DeleteContext = (PFN_XDeleteContext)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XDeleteContext");
        _glfw.x11.xlib.DeleteProperty = (PFN_XDeleteProperty)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XDeleteProperty");
        _glfw.x11.xlib.DestroyIC = (PFN_XDestroyIC)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XDestroyIC");
        _glfw.x11.xlib.DestroyRegion = (PFN_XDestroyRegion)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XDestroyRegion");
        _glfw.x11.xlib.DestroyWindow = (PFN_XDestroyWindow)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XDestroyWindow");
        _glfw.x11.xlib.DisplayKeycodes = (PFN_XDisplayKeycodes)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XDisplayKeycodes");
        _glfw.x11.xlib.EventsQueued = (PFN_XEventsQueued)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XEventsQueued");
        _glfw.x11.xlib.FilterEvent = (PFN_XFilterEvent)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XFilterEvent");
        _glfw.x11.xlib.FindContext = (PFN_XFindContext)
            _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XFindContext");
        _glfw.x11.xlib.Flush = (PFN_XFlush)
            _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XFlush");
        _glfw.x11.xlib.Free = (PFN_XFree)
            _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XFree");
        _glfw.x11.xlib.FreeColormap = (PFN_XFreeColormap)
            _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XFreeColormap");
        _glfw.x11.xlib.FreeCursor = (PFN_XFreeCursor)
            _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XFreeCursor");
        _glfw.x11.xlib.FreeEventData = (PFN_XFreeEventData)
            _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XFreeEventData");
        _glfw.x11.xlib.GetErrorText = (PFN_XGetErrorText)
            _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XGetErrorText");
        _glfw.x11.xlib.GetEventData = (PFN_XGetEventData)
            _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XGetEventData");
        _glfw.x11.xlib.GetICValues = (PFN_XGetICValues)
            _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XGetICValues");
        _glfw.x11.xlib.GetIMValues = (PFN_XGetIMValues)
            _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XGetIMValues");
        _glfw.x11.xlib.GetInputFocus = (PFN_XGetInputFocus)
            _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XGetInputFocus");
        _glfw.x11.xlib.GetKeyboardMapping = (PFN_XGetKeyboardMapping)
            _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XGetKeyboardMapping");
        _glfw.x11.xlib.GetScreenSaver = (PFN_XGetScreenSaver)
            _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XGetScreenSaver");
        _glfw.x11.xlib.GetSelectionOwner = (PFN_XGetSelectionOwner)
            _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XGetSelectionOwner");
        _glfw.x11.xlib.GetVisualInfo = (PFN_XGetVisualInfo)
            _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XGetVisualInfo");
        _glfw.x11.xlib.GetWMNormalHints = (PFN_XGetWMNormalHints)
            _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XGetWMNormalHints");
        _glfw.x11.xlib.GetWindowAttributes = (PFN_XGetWindowAttributes)
            _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XGetWindowAttributes");
        _glfw.x11.xlib.GetWindowProperty = (PFN_XGetWindowProperty)
            _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XGetWindowProperty");
        _glfw.x11.xlib.GrabPointer = (PFN_XGrabPointer)
            _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XGrabPointer");
        _glfw.x11.xlib.IconifyWindow = (PFN_XIconifyWindow)
            _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XIconifyWindow");
        _glfw.x11.xlib.InternAtom = (PFN_XInternAtom)
            _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XInternAtom");
        _glfw.x11.xlib.LookupString = (PFN_XLookupString)
            _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XLookupString");
        _glfw.x11.xlib.MapRaised = (PFN_XMapRaised)
            _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XMapRaised");
        _glfw.x11.xlib.MapWindow = (PFN_XMapWindow)
            _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XMapWindow");
        _glfw.x11.xlib.MoveResizeWindow = (PFN_XMoveResizeWindow)
            _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XMoveResizeWindow");
        _glfw.x11.xlib.MoveWindow = (PFN_XMoveWindow)
            _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XMoveWindow");
        _glfw.x11.xlib.NextEvent = (PFN_XNextEvent)
            _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XNextEvent");
        _glfw.x11.xlib.OpenIM = (PFN_XOpenIM)
            _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XOpenIM");
        _glfw.x11.xlib.PeekEvent = (PFN_XPeekEvent)
            _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XPeekEvent");
        _glfw.x11.xlib.Pending = (PFN_XPending)
            _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XPending");
        _glfw.x11.xlib.QueryExtension = (PFN_XQueryExtension)
            _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XQueryExtension");
        _glfw.x11.xlib.QueryPointer = (PFN_XQueryPointer)
            _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XQueryPointer");
        _glfw.x11.xlib.RaiseWindow = (PFN_XRaiseWindow)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XRaiseWindow");
        _glfw.x11.xlib.RegisterIMInstantiateCallback = (PFN_XRegisterIMInstantiateCallback)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XRegisterIMInstantiateCallback");
        _glfw.x11.xlib.ResizeWindow = (PFN_XResizeWindow)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XResizeWindow");
        _glfw.x11.xlib.ResourceManagerString = (PFN_XResourceManagerString)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XResourceManagerString");
        _glfw.x11.xlib.SaveContext = (PFN_XSaveContext)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XSaveContext");
        _glfw.x11.xlib.SelectInput = (PFN_XSelectInput)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XSelectInput");
        _glfw.x11.xlib.SendEvent = (PFN_XSendEvent)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XSendEvent");
        _glfw.x11.xlib.SetClassHint = (PFN_XSetClassHint)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XSetClassHint");
        _glfw.x11.xlib.SetErrorHandler = (PFN_XSetErrorHandler)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XSetErrorHandler");
        _glfw.x11.xlib.SetICFocus = (PFN_XSetICFocus)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XSetICFocus");
        _glfw.x11.xlib.SetIMValues = (PFN_XSetIMValues)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XSetIMValues");
        _glfw.x11.xlib.SetInputFocus = (PFN_XSetInputFocus)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XSetInputFocus");
        _glfw.x11.xlib.SetLocaleModifiers = (PFN_XSetLocaleModifiers)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XSetLocaleModifiers");
        _glfw.x11.xlib.SetScreenSaver = (PFN_XSetScreenSaver)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XSetScreenSaver");
        _glfw.x11.xlib.SetSelectionOwner = (PFN_XSetSelectionOwner)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XSetSelectionOwner");
        _glfw.x11.xlib.SetWMHints = (PFN_XSetWMHints)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XSetWMHints");
        _glfw.x11.xlib.SetWMNormalHints = (PFN_XSetWMNormalHints)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XSetWMNormalHints");
        _glfw.x11.xlib.SetWMProtocols = (PFN_XSetWMProtocols)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XSetWMProtocols");
        _glfw.x11.xlib.SupportsLocale = (PFN_XSupportsLocale)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XSupportsLocale");
        _glfw.x11.xlib.Sync = (PFN_XSync)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XSync");
        _glfw.x11.xlib.TranslateCoordinates = (PFN_XTranslateCoordinates)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XTranslateCoordinates");
        _glfw.x11.xlib.UndefineCursor = (PFN_XUndefineCursor)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XUndefineCursor");
        _glfw.x11.xlib.UngrabPointer = (PFN_XUngrabPointer)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XUngrabPointer");
        _glfw.x11.xlib.UnmapWindow = (PFN_XUnmapWindow)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XUnmapWindow");
        _glfw.x11.xlib.UnsetICFocus = (PFN_XUnsetICFocus)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XUnsetICFocus");
        _glfw.x11.xlib.VisualIDFromVisual = (PFN_XVisualIDFromVisual)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XVisualIDFromVisual");
        _glfw.x11.xlib.WarpPointer = (PFN_XWarpPointer)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XWarpPointer");
        _glfw.x11.xkb.FreeKeyboard = (PFN_XkbFreeKeyboard) _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XkbFreeKeyboard");
        _glfw.x11.xkb.FreeNames = (PFN_XkbFreeNames) _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XkbFreeNames");
        _glfw.x11.xkb.GetMap = (PFN_XkbGetMap) _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XkbGetMap");
        _glfw.x11.xkb.GetNames = (PFN_XkbGetNames) _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XkbGetNames");
        _glfw.x11.xkb.GetState = (PFN_XkbGetState) _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XkbGetState");
        _glfw.x11.xkb.KeycodeToKeysym = (PFN_XkbKeycodeToKeysym) _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XkbKeycodeToKeysym");
        _glfw.x11.xkb.QueryExtension = (PFN_XkbQueryExtension) _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XkbQueryExtension");
        _glfw.x11.xkb.SelectEventDetails = (PFN_XkbSelectEventDetails) _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XkbSelectEventDetails");
        _glfw.x11.xkb.SetDetectableAutoRepeat = (PFN_XkbSetDetectableAutoRepeat)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XkbSetDetectableAutoRepeat");
        _glfw.x11.xrm.DestroyDatabase = (PFN_XrmDestroyDatabase)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XrmDestroyDatabase");
        _glfw.x11.xrm.GetResource = (PFN_XrmGetResource)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XrmGetResource");
        _glfw.x11.xrm.GetStringDatabase = (PFN_XrmGetStringDatabase)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XrmGetStringDatabase");
        _glfw.x11.xrm.UniqueQuark = (PFN_XrmUniqueQuark)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XrmUniqueQuark");
        _glfw.x11.xlib.UnregisterIMInstantiateCallback = (PFN_XUnregisterIMInstantiateCallback)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XUnregisterIMInstantiateCallback");
        _glfw.x11.xlib.utf8LookupString = (PFN_Xutf8LookupString)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "Xutf8LookupString");
        _glfw.x11.xlib.utf8SetWMProperties = (PFN_Xutf8SetWMProperties)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "Xutf8SetWMProperties");
        if (_glfw.x11.xlib.utf8LookupString && _glfw.x11.xlib.utf8SetWMProperties) _glfw.x11.xlib.utf8 = GLFW_TRUE;
        _glfw.x11.screen = DefaultScreen(_glfw.x11.display); // Segfaults
        _glfw.x11.root = RootWindow(_glfw.x11.display,_glfw.x11.screen);
        _glfw.x11.context = XUniqueContext();
        getSystemContentScale(&_glfw.x11.contentScaleX, &_glfw.x11.contentScaleY);
        if (!createEmptyEventPipe()) return GLFW_FALSE;
        if (!initExtensions()) return GLFW_FALSE;
        _glfw.x11.helperWindowHandle = createHelperWindow();
        XcursorImage* native = XcursorImageCreate(16,16); memset(native->pixels,0,256*sizeof(XcursorPixel)); native->xhot=native->yhot=0;
        _glfw.x11.hiddenCursorHandle = XcursorImageLoadCursor(_glfw.x11.display,native); XcursorImageDestroy(native);
        if (XSupportsLocale() && _glfw.x11.xlib.utf8) { XSetLocaleModifiers(""); XRegisterIMInstantiateCallback(_glfw.x11.display,NULL,NULL,NULL,inputMethodInstantiateCallback,NULL); }
        _glfwPollMonitorsX11();
        return GLFW_TRUE;
    }
    //=============================================================================
    // Joystick
    #ifndef SYN_DROPPED // < v2.6.39 kernel headers
    // Workaround for CentOS-6, which is supported till 2020-11-30, but still on v2.6.32
    #define SYN_DROPPED 3
    #endif
    static void handleKeyEvent(_GLFWjoystick* js, int code, int value) { _glfwInputJoystickButton(js,js->linjs.keyMap[code - BTN_MISC],value ? GLFW_PRESS : GLFW_RELEASE); }
    static void handleAbsEvent(_GLFWjoystick* js, int code, int value) {
        const int index = js->linjs.absMap[code];
        if (code >= ABS_HAT0X && code <= ABS_HAT3Y) {
            static const char stateMap[3][3] = {
                { GLFW_HAT_CENTERED, GLFW_HAT_UP,       GLFW_HAT_DOWN },
                { GLFW_HAT_LEFT,     GLFW_HAT_LEFT_UP,  GLFW_HAT_LEFT_DOWN },
                { GLFW_HAT_RIGHT,    GLFW_HAT_RIGHT_UP, GLFW_HAT_RIGHT_DOWN },
            };

            const int hat = (code - ABS_HAT0X) / 2;
            const int axis = (code - ABS_HAT0X) % 2;
            int* state = js->linjs.hats[hat];
            if (value == 0) state[axis] = 0;
            else if (value < 0) state[axis] = 1;
            else if (value > 0) state[axis] = 2;
            _glfwInputJoystickHat(js, index, stateMap[state[0]][state[1]]);
        } else {
            const struct input_absinfo* info = &js->linjs.absInfo[code];
            float normalized = value;
            const int range = info->maximum - info->minimum;
            if (range) { normalized = (normalized - info->minimum) / range; normalized = normalized * 2.0f - 1.0f; }
            _glfwInputJoystickAxis(js, index, normalized);
        }
    }

    static void pollAbsState(_GLFWjoystick* js) {
        for (int code = 0;  code < ABS_CNT;  code++) {
            if (js->linjs.absMap[code] < 0) continue;

            struct input_absinfo* info = &js->linjs.absInfo[code];
            if (ioctl(js->linjs.fd, EVIOCGABS(code), info) < 0) continue;

            handleAbsEvent(js, code, info->value);
        }
    }

    #define isBitSet(bit, arr) (arr[(bit) / 8] & (1 << ((bit) % 8)))
    static GLFWbool openJoystickDevice(const char* path) {
        for (int jid = 0;  jid <= GLFW_JOYSTICK_LAST;  jid++) {
            if (!_glfw.joysticks[jid].connected) continue;
            if (strcmp(_glfw.joysticks[jid].linjs.path, path) == 0) return GLFW_FALSE;
        }

        _GLFWjoystickLinux linjs = {0};
        linjs.fd = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
        if (linjs.fd == -1) return GLFW_FALSE;

        char evBits[(EV_CNT + 7) / 8] = {0};
        char keyBits[(KEY_CNT + 7) / 8] = {0};
        char absBits[(ABS_CNT + 7) / 8] = {0};
        struct input_id id;
        if (ioctl(linjs.fd, EVIOCGBIT(0, sizeof(evBits)), evBits) < 0 || ioctl(linjs.fd, EVIOCGBIT(EV_KEY, sizeof(keyBits)), keyBits) < 0 || ioctl(linjs.fd, EVIOCGBIT(EV_ABS, sizeof(absBits)), absBits) < 0 || ioctl(linjs.fd, EVIOCGID, &id) < 0) { _glfwInputError(GLFW_PLATFORM_ERROR,"Linux: Failed to query input device: %s",strerror(errno)); close(linjs.fd); return GLFW_FALSE; }
        if (!isBitSet(EV_ABS, evBits)) { close(linjs.fd); return GLFW_FALSE; }

        char name[256] = "";
        if (ioctl(linjs.fd, EVIOCGNAME(sizeof(name)), name) < 0) strncpy(name, "Unknown", sizeof(name));
        char guid[33] = "";
        if (id.vendor && id.product && id.version) sprintf(guid, "%02x%02x0000%02x%02x0000%02x%02x0000%02x%02x0000",id.bustype & 0xff, id.bustype >> 8,id.vendor & 0xff,  id.vendor >> 8,id.product & 0xff, id.product >> 8,id.version & 0xff, id.version >> 8);
        else sprintf(guid, "%02x%02x0000%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x00",id.bustype & 0xff, id.bustype >> 8,name[0], name[1], name[2], name[3],name[4], name[5], name[6], name[7],name[8], name[9], name[10]);

        int axisCount = 0, buttonCount = 0, hatCount = 0;
        for (int code = BTN_MISC;  code < KEY_CNT;  code++) {
            if (!isBitSet(code, keyBits)) continue;

            linjs.keyMap[code - BTN_MISC] = buttonCount;
            buttonCount++;
        }

        for (int code = 0;  code < ABS_CNT;  code++) {
            linjs.absMap[code] = -1;
            if (!isBitSet(code, absBits)) continue;

            if (code >= ABS_HAT0X && code <= ABS_HAT3Y) {
                linjs.absMap[code] = hatCount;
                hatCount++;
                code++; // Skip the Y axis
            } else {
                if (ioctl(linjs.fd, EVIOCGABS(code), &linjs.absInfo[code]) < 0) continue;

                linjs.absMap[code] = axisCount;
                axisCount++;
            }
        }

        _GLFWjoystick* js = _glfwAllocJoystick(name,guid,axisCount,buttonCount,hatCount);
        if (!js) { close(linjs.fd); return GLFW_FALSE; }

        strncpy(linjs.path, path, sizeof(linjs.path) - 1);
        __builtin_memcpy(&js->linjs, &linjs, sizeof(linjs));
        pollAbsState(js);
        _glfwInputJoystick(js, GLFW_CONNECTED);
        return GLFW_TRUE;
    }
    #undef isBitSet

    static void closeJoystick(_GLFWjoystick* js) { _glfwInputJoystick(js, GLFW_DISCONNECTED); close(js->linjs.fd); _glfwFreeJoystick(js); }
    static int compareJoysticks(const void* fp, const void* sp) { const _GLFWjoystick* fj = fp; const _GLFWjoystick* sj = sp; return strcmp(fj->linjs.path, sj->linjs.path); }
    void _glfwDetectJoystickConnectionLinux(void) {
        if (_glfw.linjs.inotify <= 0) return;

        ssize_t offset = 0;
        char buffer[16384];
        const ssize_t size = read(_glfw.linjs.inotify, buffer, sizeof(buffer));
        while (size > offset) {
            regmatch_t match;
            const struct inotify_event* e = (struct inotify_event*) (buffer + offset);
            offset += sizeof(struct inotify_event) + e->len;
            if (regexec(&_glfw.linjs.regex, e->name, 1, &match, 0) != 0) continue;

            char path[PATH_MAX];
            snprintf(path, sizeof(path), "/dev/input/%s", e->name);
            if (e->mask & (IN_CREATE | IN_ATTRIB)) openJoystickDevice(path);
            else if (e->mask & IN_DELETE) {
                for (int jid = 0;  jid <= GLFW_JOYSTICK_LAST;  jid++) {
                    if (strcmp(_glfw.joysticks[jid].linjs.path, path) == 0) { closeJoystick(_glfw.joysticks + jid); break; }
                }
            }
        }
    }

    GLFWbool _glfwInitJoysticksLinux(void) {
        const char* dirname = "/dev/input";
        _glfw.linjs.inotify = inotify_init();
        if (_glfw.linjs.inotify > 0) {
            int flags = fcntl(_glfw.linjs.inotify,F_GETFL,0);
            fcntl(_glfw.linjs.inotify,F_SETFL,flags|O_NONBLOCK); //     _glfw.linjs.inotify = inotify_init1(IN_NONBLOCK | IN_CLOEXEC); Not supported in underversioned glibc 2.7
            fcntl(_glfw.linjs.inotify,F_SETFD,FD_CLOEXEC);
            _glfw.linjs.watch = inotify_add_watch(_glfw.linjs.inotify,dirname,IN_CREATE|IN_ATTRIB|IN_DELETE); // HACK: Register for IN_ATTRIB to get notified when udev is done.  This works well in practice but the true way is libudev.
        }

        _glfw.linjs.regexCompiled = (regcomp(&_glfw.linjs.regex, "^event[0-9]\\+$", 0) == 0); // Continue without device connection notifications if inotify fails
        if (!_glfw.linjs.regexCompiled) { _glfwInputError(GLFW_PLATFORM_ERROR, "Linux: Failed to compile regex"); return GLFW_FALSE; }

        int count = 0;
        DIR* dir = opendir(dirname);
        if (dir) {
            struct dirent* entry;
            while ((entry = readdir(dir))) {
                regmatch_t match;
                if (regexec(&_glfw.linjs.regex, entry->d_name, 1, &match, 0) != 0) continue;

                char path[PATH_MAX];
                snprintf(path, sizeof(path),"%s/%s",dirname,entry->d_name);
                if (openJoystickDevice(path)) count++;
            }

            closedir(dir);
        }

        qsort(_glfw.joysticks, count, sizeof(_GLFWjoystick), compareJoysticks);
        return GLFW_TRUE;
    }

    void _glfwTerminateJoysticksLinux(void) {
        for (int jid = 0;  jid <= GLFW_JOYSTICK_LAST;  jid++) {
            _GLFWjoystick* js = _glfw.joysticks + jid;
            if (js->connected) closeJoystick(js);
        }

        if (_glfw.linjs.inotify > 0) {
            if (_glfw.linjs.watch > 0) inotify_rm_watch(_glfw.linjs.inotify, _glfw.linjs.watch);
            close(_glfw.linjs.inotify);
        }

        if (_glfw.linjs.regexCompiled) regfree(&_glfw.linjs.regex);
    }

    GLFWbool _glfwPollJoystickLinux(_GLFWjoystick* js, int mode) {
        (void)mode; for (;;) {
            struct input_event e;
            errno = 0;
            if (read(js->linjs.fd, &e, sizeof(e)) < 0) {
                // Reset the joystick slot if the device was disconnected
                if (errno == ENODEV) closeJoystick(js);
                break;
            }

            if (e.type == EV_SYN) {
                if (e.code == SYN_DROPPED) _glfw.linjs.dropped = GLFW_TRUE;
                else if (e.code == SYN_REPORT) { _glfw.linjs.dropped = GLFW_FALSE; pollAbsState(js); }
            }

            if (_glfw.linjs.dropped) continue;

            if (e.type == EV_KEY) handleKeyEvent(js, e.code, e.value);
            else if (e.type == EV_ABS) handleAbsEvent(js, e.code, e.value);
        }

        return js->connected;
    }

    const char* _glfwGetMappingNameLinux(void) { return "Linux"; }
    void _glfwUpdateGamepadGUIDLinux(char* guid) { (void)guid; }
    //=============================================================================
    // GLX Context
    #ifndef GLXBadProfileARB
    #define GLXBadProfileARB 13
    #endif

    static int getGLXFBConfigAttrib(GLXFBConfig fbconfig, int attrib) {
        int value;
        glXGetFBConfigAttrib(_glfw.x11.display, fbconfig, attrib, &value);
        return value;
    }

    static GLFWbool chooseGLXFBConfig(const _GLFWfbconfig* desired, GLXFBConfig* result) {
        GLXFBConfig* nativeConfigs; _GLFWfbconfig* usableConfigs; const _GLFWfbconfig* closest; int nativeCount, usableCount; const char* vendor; GLFWbool trustWindowBit = GLFW_TRUE;
        vendor = glXGetClientString(_glfw.x11.display, GLX_VENDOR);
        if (vendor && strcmp(vendor, "Chromium") == 0) trustWindowBit = GLFW_FALSE;
        nativeConfigs = glXGetFBConfigs(_glfw.x11.display, _glfw.x11.screen, &nativeCount);
        if (!nativeConfigs || !nativeCount) { DualLogError("GLX: No GLXFBConfigs returned"); return GLFW_FALSE; }
        usableConfigs = _glfw_calloc(nativeCount, sizeof(_GLFWfbconfig)); usableCount = 0;
        for (int i = 0;  i < nativeCount;  i++) {
            const GLXFBConfig n = nativeConfigs[i];
            _GLFWfbconfig* u = usableConfigs + usableCount;
            if (!(getGLXFBConfigAttrib(n, GLX_RENDER_TYPE) & GLX_RGBA_BIT)) continue;
            if (!(getGLXFBConfigAttrib(n, GLX_DRAWABLE_TYPE) & GLX_WINDOW_BIT)) {
                if (trustWindowBit) continue;
            }

            if (getGLXFBConfigAttrib(n, GLX_DOUBLEBUFFER) != desired->doublebuffer) continue;
            if (desired->transparent) {
                XVisualInfo* vi = glXGetVisualFromFBConfig(_glfw.x11.display, n);
                if (vi) { u->transparent = _glfwIsVisualTransparentX11(vi->visual); XFree(vi); }
            }

            u->redBits = getGLXFBConfigAttrib(n, GLX_RED_SIZE);
            u->greenBits = getGLXFBConfigAttrib(n, GLX_GREEN_SIZE);
            u->blueBits = getGLXFBConfigAttrib(n, GLX_BLUE_SIZE);
            u->alphaBits = getGLXFBConfigAttrib(n, GLX_ALPHA_SIZE);
            u->depthBits = getGLXFBConfigAttrib(n, GLX_DEPTH_SIZE);
            u->stencilBits = getGLXFBConfigAttrib(n, GLX_STENCIL_SIZE);
            u->accumRedBits = getGLXFBConfigAttrib(n, GLX_ACCUM_RED_SIZE);
            u->accumGreenBits = getGLXFBConfigAttrib(n, GLX_ACCUM_GREEN_SIZE);
            u->accumBlueBits = getGLXFBConfigAttrib(n, GLX_ACCUM_BLUE_SIZE);
            u->accumAlphaBits = getGLXFBConfigAttrib(n, GLX_ACCUM_ALPHA_SIZE);
            u->auxBuffers = getGLXFBConfigAttrib(n, GLX_AUX_BUFFERS);
            if (getGLXFBConfigAttrib(n, GLX_STEREO)) u->stereo = GLFW_TRUE;
            if (_glfw.glx.ARB_multisample) u->samples = getGLXFBConfigAttrib(n, GLX_SAMPLES);
            if (_glfw.glx.ARB_framebuffer_sRGB || _glfw.glx.EXT_framebuffer_sRGB) u->sRGB = getGLXFBConfigAttrib(n, GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB);
            u->handle = (uintptr_t) n;
            usableCount++;
        }

        closest = _glfwChooseFBConfig(desired, usableConfigs, usableCount);
        if (closest) *result = (GLXFBConfig) closest->handle;
        XFree(nativeConfigs);
        free(usableConfigs);
        return closest != NULL;
    }

    static GLXContext createLegacyContextGLX(_GLFWwindow* window, GLXFBConfig fbconfig, GLXContext share) { (void)window; return glXCreateNewContext(_glfw.x11.display,fbconfig,GLX_RGBA_TYPE,share,True); }
    static void makeContextCurrentGLX(_GLFWwindow* window) {
        if (window) {
            if (!glXMakeCurrent(_glfw.x11.display,window->context.glx.window,window->context.glx.handle)) { _glfwInputError(GLFW_PLATFORM_ERROR,"GLX: Failed to make context current"); return; }
        } else {
            if (!glXMakeCurrent(_glfw.x11.display, None, NULL)) { _glfwInputError(GLFW_PLATFORM_ERROR,"GLX: Failed to clear current context"); return; }
        }

        _glfwPlatformSetTls(&_glfw.contextSlot, window);
    }

    static void swapBuffersGLX(_GLFWwindow* window) { glXSwapBuffers(_glfw.x11.display, window->context.glx.window); }
    static void swapIntervalGLX(int interval) {
        _GLFWwindow* window = _glfwPlatformGetTls(&_glfw.contextSlot);
        if (_glfw.glx.EXT_swap_control) {
            _glfw.glx.SwapIntervalEXT(_glfw.x11.display,window->context.glx.window,interval);
        } else if (_glfw.glx.MESA_swap_control) _glfw.glx.SwapIntervalMESA(interval);
        else if (_glfw.glx.SGI_swap_control) {
            if (interval > 0) _glfw.glx.SwapIntervalSGI(interval);
        }
    }

    static int extensionSupportedGLX(const char* extension) {
        const char* extensions = glXQueryExtensionsString(_glfw.x11.display, _glfw.x11.screen);
        if (extensions) {
            if (_glfwStringInExtensionString(extension, extensions)) return GLFW_TRUE;
        }

        return GLFW_FALSE;
    }

    static GLFWglproc getProcAddressGLX(const char* procname)
    {
        if (_glfw.glx.GetProcAddress)
            return _glfw.glx.GetProcAddress((const GLubyte*) procname);
        else if (_glfw.glx.GetProcAddressARB)
            return _glfw.glx.GetProcAddressARB((const GLubyte*) procname);
        else
        {
            // NOTE: glvnd provides GLX 1.4, so this can only happen with libGL
            return _glfwPlatformGetModuleSymbol(_glfw.glx.handle, procname);
        }
    }

    static void destroyContextGLX(_GLFWwindow* window) {
        if (window->context.glx.window) { glXDestroyWindow(_glfw.x11.display, window->context.glx.window); window->context.glx.window = None; }
        if (window->context.glx.handle) { glXDestroyContext(_glfw.x11.display, window->context.glx.handle); window->context.glx.handle = NULL; }
    }
    
    GLFWbool _glfwCreateContextGLX(_GLFWwindow* window, const _GLFWctxconfig* ctxconfig, const _GLFWfbconfig* fbconfig) {
        int attribs[40], index = 0, mask = 0, flags = 0;
        GLXFBConfig native = NULL; GLXContext share = NULL;
        if (ctxconfig->share) share = ctxconfig->share->context.glx.handle;
        if (!chooseGLXFBConfig(fbconfig, &native)) { _glfwInputError(GLFW_FORMAT_UNAVAILABLE,"GLX: Failed to find GLXFBConfig"); return GLFW_FALSE; }

        if (ctxconfig->client == GLFW_OPENGL_ES_API && (!_glfw.glx.ARB_create_context || !_glfw.glx.ARB_create_context_profile || !_glfw.glx.EXT_create_context_es2_profile)) { _glfwInputError(GLFW_API_UNAVAILABLE, "GLX: ES requested but missing extensions"); return GLFW_FALSE; }
        if (ctxconfig->forward && !_glfw.glx.ARB_create_context) { _glfwInputError(GLFW_VERSION_UNAVAILABLE,"GLX: Forward compat requested but missing ARB_create_context"); return GLFW_FALSE; }
        if (ctxconfig->profile && (!_glfw.glx.ARB_create_context || !_glfw.glx.ARB_create_context_profile)) { _glfwInputError(GLFW_VERSION_UNAVAILABLE,"GLX: Profile requested but missing extension"); return GLFW_FALSE; }

        _glfwGrabErrorHandlerX11();
        if (_glfw.glx.ARB_create_context) {
            if (ctxconfig->client == GLFW_OPENGL_API) {
                if (ctxconfig->forward) flags |= GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
                if (ctxconfig->profile == GLFW_OPENGL_CORE_PROFILE) mask |= GLX_CONTEXT_CORE_PROFILE_BIT_ARB;
                else if (ctxconfig->profile == GLFW_OPENGL_COMPAT_PROFILE) mask |= GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
            } else mask |= GLX_CONTEXT_ES2_PROFILE_BIT_EXT;

            if (ctxconfig->debug) flags |= GLX_CONTEXT_DEBUG_BIT_ARB;
            if (ctxconfig->robustness && _glfw.glx.ARB_create_context_robustness) {
                attribs[index++] = GLX_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB;
                attribs[index++] = (ctxconfig->robustness == GLFW_LOSE_CONTEXT_ON_RESET) ? GLX_LOSE_CONTEXT_ON_RESET_ARB : GLX_NO_RESET_NOTIFICATION_ARB;
                flags |= GLX_CONTEXT_ROBUST_ACCESS_BIT_ARB;
            }
            if (ctxconfig->release && _glfw.glx.ARB_context_flush_control) {
                attribs[index++] = GLX_CONTEXT_RELEASE_BEHAVIOR_ARB;
                attribs[index++] = (ctxconfig->release == GLFW_RELEASE_BEHAVIOR_NONE) ? GLX_CONTEXT_RELEASE_BEHAVIOR_NONE_ARB : GLX_CONTEXT_RELEASE_BEHAVIOR_FLUSH_ARB;
            }
            if (ctxconfig->noerror && _glfw.glx.ARB_create_context_no_error) { attribs[index++] = GLX_CONTEXT_OPENGL_NO_ERROR_ARB; attribs[index++] = GLFW_TRUE; }
            if (ctxconfig->major != 1 || ctxconfig->minor != 0) { attribs[index++] = GLX_CONTEXT_MAJOR_VERSION_ARB; attribs[index++] = ctxconfig->major; attribs[index++] = GLX_CONTEXT_MINOR_VERSION_ARB; attribs[index++] = ctxconfig->minor; }
            if (mask) { attribs[index++] = GLX_CONTEXT_PROFILE_MASK_ARB; attribs[index++] = mask; }
            if (flags) { attribs[index++] = GLX_CONTEXT_FLAGS_ARB; attribs[index++] = flags; }
            attribs[index++] = None; attribs[index++] = None;
            window->context.glx.handle = _glfw.glx.CreateContextAttribsARB(_glfw.x11.display,native,share,True,attribs);
            if (!window->context.glx.handle && _glfw.x11.errorCode == _glfw.glx.errorBase + GLXBadProfileARB && ctxconfig->client == GLFW_OPENGL_API && ctxconfig->profile == GLFW_OPENGL_ANY_PROFILE && !ctxconfig->forward) window->context.glx.handle = createLegacyContextGLX(window, native, share);
        } else window->context.glx.handle = createLegacyContextGLX(window, native, share);

        _glfwReleaseErrorHandlerX11();
        if (!window->context.glx.handle) { _glfwInputErrorX11(GLFW_VERSION_UNAVAILABLE, "GLX: Failed to create context"); return GLFW_FALSE; }
        if (!(window->context.glx.window = glXCreateWindow(_glfw.x11.display, native, window->x11.handle, NULL))) { _glfwInputError(GLFW_PLATFORM_ERROR, "GLX: Failed to create window"); return GLFW_FALSE; }

        window->context.glx.fbconfig = native; window->context.makeCurrent = makeContextCurrentGLX;
        window->context.swapBuffers = swapBuffersGLX; window->context.swapInterval = swapIntervalGLX;
        window->context.extensionSupported = extensionSupportedGLX; window->context.getProcAddress = getProcAddressGLX;
        window->context.destroy = destroyContextGLX;
        return GLFW_TRUE;
    }
    
    GLFWbool _glfwChooseVisualGLX(const _GLFWwndconfig* wndconfig, const _GLFWctxconfig* ctxconfig, const _GLFWfbconfig* fbconfig, Visual** visual, int* depth) {
        GLXFBConfig native; XVisualInfo* result; (void)wndconfig; (void)ctxconfig;
        if (!chooseGLXFBConfig(fbconfig, &native)) { DualLogError("GLX: Failed to find a suitable GLXFBConfig"); return GLFW_FALSE; }

        result = glXGetVisualFromFBConfig(_glfw.x11.display, native);
        if (!result) { _glfwInputError(GLFW_PLATFORM_ERROR,"GLX: Failed to retrieve Visual for GLXFBConfig"); return GLFW_FALSE; }

        *visual = result->visual; *depth  = result->depth; XFree(result); return GLFW_TRUE;
    }
    
    GLFWbool _glfwCreateWindowX11(_GLFWwindow* window,const _GLFWwndconfig* wndconfig,const _GLFWctxconfig* ctxconfig,const _GLFWfbconfig* fbconfig) {
        Visual* visual=NULL; int depth;
        const char* names[] = {"libGLX.so.0","libGL.so.1","libGL.so",NULL};
        for (int i=0;names[i] && !_glfw.glx.handle;i++) _glfw.glx.handle = _glfwPlatformLoadModule(names[i]);
        if (!_glfw.glx.handle) { _glfwInputError(GLFW_API_UNAVAILABLE,"GLX: Failed to load GLX"); return GLFW_FALSE; }
        _glfw.glx.GetFBConfigs = (PFNGLXGETFBCONFIGSPROC)_glfwPlatformGetModuleSymbol(_glfw.glx.handle,"glXGetFBConfigs");
        _glfw.glx.GetFBConfigAttrib = (PFNGLXGETFBCONFIGATTRIBPROC)_glfwPlatformGetModuleSymbol(_glfw.glx.handle,"glXGetFBConfigAttrib");
        _glfw.glx.GetClientString = (PFNGLXGETCLIENTSTRINGPROC)_glfwPlatformGetModuleSymbol(_glfw.glx.handle,"glXGetClientString");
        _glfw.glx.QueryExtension = (PFNGLXQUERYEXTENSIONPROC)_glfwPlatformGetModuleSymbol(_glfw.glx.handle,"glXQueryExtension");
        _glfw.glx.QueryVersion = (PFNGLXQUERYVERSIONPROC)_glfwPlatformGetModuleSymbol(_glfw.glx.handle,"glXQueryVersion");
        _glfw.glx.DestroyContext = (PFNGLXDESTROYCONTEXTPROC)_glfwPlatformGetModuleSymbol(_glfw.glx.handle,"glXDestroyContext");
        _glfw.glx.MakeCurrent = (PFNGLXMAKECURRENTPROC)_glfwPlatformGetModuleSymbol(_glfw.glx.handle,"glXMakeCurrent");
        _glfw.glx.SwapBuffers = (PFNGLXSWAPBUFFERSPROC)_glfwPlatformGetModuleSymbol(_glfw.glx.handle,"glXSwapBuffers");
        _glfw.glx.QueryExtensionsString = (PFNGLXQUERYEXTENSIONSSTRINGPROC)_glfwPlatformGetModuleSymbol(_glfw.glx.handle,"glXQueryExtensionsString");
        _glfw.glx.CreateNewContext = (PFNGLXCREATENEWCONTEXTPROC)_glfwPlatformGetModuleSymbol(_glfw.glx.handle,"glXCreateNewContext");
        _glfw.glx.CreateWindow = (PFNGLXCREATEWINDOWPROC)_glfwPlatformGetModuleSymbol(_glfw.glx.handle,"glXCreateWindow");
        _glfw.glx.DestroyWindow = (PFNGLXDESTROYWINDOWPROC)_glfwPlatformGetModuleSymbol(_glfw.glx.handle,"glXDestroyWindow");
        _glfw.glx.GetVisualFromFBConfig = (PFNGLXGETVISUALFROMFBCONFIGPROC)_glfwPlatformGetModuleSymbol(_glfw.glx.handle,"glXGetVisualFromFBConfig");
        if (!_glfw.glx.GetFBConfigs || !_glfw.glx.GetFBConfigAttrib || !_glfw.glx.GetClientString || !_glfw.glx.QueryExtension || !_glfw.glx.QueryVersion || !_glfw.glx.DestroyContext || !_glfw.glx.MakeCurrent || !_glfw.glx.SwapBuffers || !_glfw.glx.QueryExtensionsString || !_glfw.glx.CreateNewContext || !_glfw.glx.CreateWindow || !_glfw.glx.DestroyWindow || !_glfw.glx.GetVisualFromFBConfig) { _glfwInputError(GLFW_PLATFORM_ERROR,"GLX: Failed to load entry points"); return GLFW_FALSE; }
        _glfw.glx.GetProcAddress = (PFNGLXGETPROCADDRESSPROC)_glfwPlatformGetModuleSymbol(_glfw.glx.handle,"glXGetProcAddress");
        _glfw.glx.GetProcAddressARB = (PFNGLXGETPROCADDRESSPROC)_glfwPlatformGetModuleSymbol(_glfw.glx.handle,"glXGetProcAddressARB");
        if (!glXQueryExtension(_glfw.x11.display,&_glfw.glx.errorBase,&_glfw.glx.eventBase)) { _glfwInputError(GLFW_API_UNAVAILABLE,"GLX: Extension not found"); return GLFW_FALSE; }
        if (!glXQueryVersion(_glfw.x11.display,&_glfw.glx.major,&_glfw.glx.minor)) { _glfwInputError(GLFW_API_UNAVAILABLE,"GLX: Failed to query version"); return GLFW_FALSE; }
        if (_glfw.glx.major == 1 && _glfw.glx.minor < 3) { _glfwInputError(GLFW_API_UNAVAILABLE,"GLX: Version 1.3 required"); return GLFW_FALSE; }
        if (extensionSupportedGLX("GLX_EXT_swap_control")) { _glfw.glx.SwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)getProcAddressGLX("glXSwapIntervalEXT"); if (_glfw.glx.SwapIntervalEXT) _glfw.glx.EXT_swap_control = GLFW_TRUE; }
        if (extensionSupportedGLX("GLX_SGI_swap_control")) { _glfw.glx.SwapIntervalSGI = (PFNGLXSWAPINTERVALSGIPROC)getProcAddressGLX("glXSwapIntervalSGI"); if (_glfw.glx.SwapIntervalSGI) _glfw.glx.SGI_swap_control = GLFW_TRUE; }
        if (extensionSupportedGLX("GLX_MESA_swap_control")) { _glfw.glx.SwapIntervalMESA = (PFNGLXSWAPINTERVALMESAPROC)getProcAddressGLX("glXSwapIntervalMESA"); if (_glfw.glx.SwapIntervalMESA) _glfw.glx.MESA_swap_control = GLFW_TRUE; }
        if (extensionSupportedGLX("GLX_ARB_multisample")) _glfw.glx.ARB_multisample = GLFW_TRUE;
        if (extensionSupportedGLX("GLX_ARB_framebuffer_sRGB")) _glfw.glx.ARB_framebuffer_sRGB = GLFW_TRUE;
        if (extensionSupportedGLX("GLX_EXT_framebuffer_sRGB")) _glfw.glx.EXT_framebuffer_sRGB = GLFW_TRUE;
        if (extensionSupportedGLX("GLX_ARB_create_context")) { _glfw.glx.CreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)getProcAddressGLX("glXCreateContextAttribsARB"); if (_glfw.glx.CreateContextAttribsARB) _glfw.glx.ARB_create_context = GLFW_TRUE; }
        if (extensionSupportedGLX("GLX_ARB_create_context_robustness")) _glfw.glx.ARB_create_context_robustness = GLFW_TRUE;
        if (extensionSupportedGLX("GLX_ARB_create_context_profile")) _glfw.glx.ARB_create_context_profile = GLFW_TRUE;
        if (extensionSupportedGLX("GLX_EXT_create_context_es2_profile")) _glfw.glx.EXT_create_context_es2_profile = GLFW_TRUE;
        if (extensionSupportedGLX("GLX_ARB_create_context_no_error")) _glfw.glx.ARB_create_context_no_error = GLFW_TRUE;
        if (extensionSupportedGLX("GLX_ARB_context_flush_control")) _glfw.glx.ARB_context_flush_control = GLFW_TRUE;
        if (!_glfwChooseVisualGLX(wndconfig,ctxconfig,fbconfig,&visual,&depth)) return GLFW_FALSE;
        if (!visual) { visual=DefaultVisual(_glfw.x11.display,_glfw.x11.screen); depth=DefaultDepth(_glfw.x11.display,_glfw.x11.screen); }
        if (!createNativeWindow(window,wndconfig,visual,depth)) return GLFW_FALSE;
        if (ctxconfig->client!=GLFW_NO_API && ctxconfig->source==GLFW_NATIVE_CONTEXT_API) {
            if (!_glfwCreateContextGLX(window,ctxconfig,fbconfig)) return GLFW_FALSE;
            if (!_glfwRefreshContextAttribs(window,ctxconfig)) return GLFW_FALSE;
        }
        if (wndconfig->mousePassthrough) _glfwSetWindowMousePassthroughX11(window,GLFW_TRUE);
        if (window->monitor) {
            _glfwShowWindowX11(window); updateWindowMode(window); acquireMonitor(window);
            if (wndconfig->centerCursor) _glfwCenterCursorInContentArea(window);
        } else if (wndconfig->visible) {
            _glfwShowWindowX11(window);
            if (wndconfig->focused) _glfwFocusWindowX11(window);
        }
        XFlush(_glfw.x11.display);
        return GLFW_TRUE;
    }

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
    char desc[_GLFW_MESSAGE_SIZE];
    va_list vl; __builtin_va_start(vl, format); 
    vsnprintf(desc, sizeof(desc), format ? format : "Error code %d", vl); 
    __builtin_va_end(vl);
    fprintf(stderr, "GLFW ERROR [%d]: %s\n", code, desc);
    if (_glfw.initialized) {
        _GLFWerror* e = _glfwPlatformGetTls(&_glfw.errorSlot);
        if (e) { e->code = code; strcpy(e->description, desc); }
    }
}

GLFWAPI int glfwInit(void) {
    memset(&_glfw,0,sizeof(_glfw)); _glfw.hints.init = _glfwInitHints; _glfw.allocator = _glfwInitAllocator;
    #if defined(WINDOWS)
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
    if (!PLATFORM_createWindow(window, &wndconfig, &ctxconfig, &fbconfig)) { return NULL; }
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
                i8 minimum = -1,maximum = 1;
                if (*c == '+') { minimum = 0; c++; }
                else if (*c == '-') { maximum = 0; c++; }
                if (*c == 'a') e->type = _GLFW_JOYSTICK_AXIS;
                else if (*c == 'b') e->type = _GLFW_JOYSTICK_BUTTON;
                else if (*c == 'h') e->type = _GLFW_JOYSTICK_HATBIT;
                else break;
                if (e->type == _GLFW_JOYSTICK_HATBIT) {
                    const unsigned long hat = strtoul(c+1,(char**)&c,10),bit = strtoul(c+1,(char**)&c,10);
                    e->index = (u8)((hat << 4) | bit);
                } else e->index = (u8)strtoul(c+1,(char**)&c,10);
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

void _glfwInputChar(_GLFWwindow* window, u32 codepoint, int mods, GLFWbool plain) {
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

#undef near
#undef far
