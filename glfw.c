// glfw.c - Heavily reduced glfw for only Windows and Linux X11
// GLFW 3.5 This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
#include "mappings.h"
#define GLFW_TRUE 1
#define GLFW_FALSE 0
#define GLFW_CURSOR_NORMAL 0x00034001
#define GLFW_CURSOR_DISABLED 0x00034003
#define GLFW_CONNECTED 0x00040001
#define GLFW_DISCONNECTED 0x00040002
#define _GLFW_STICK 3
#define _GLFW_JOYSTICK_AXIS 1
#define _GLFW_JOYSTICK_BUTTON 2
#define _GLFW_JOYSTICK_HATBIT 3
unsigned long strtoul(const char *nptr, char **endptr, int base);
int sprintf(char *str, const char *format, ...); size_t strspn(const char *s, const char *accept); char *strstr(const char *haystack, const char *needle); int strncmp(const char *s1, const char *s2, size_t n); int strcmp(const char *s1, const char *s2);
void glfwMakeContextCurrent(GLFWwindow* window); void UpdateScreenSize(i32 width, i32 height); char *strncpy(char *dest, const char *src, size_t n); void *memmove(void *dest, const void *src, size_t n); size_t strcspn(const char *s, const char *reject);
void *malloc(size_t size); void free(void *ptr); void *calloc(size_t nmemb, size_t size); void *realloc(void *ptr, size_t size); void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *)); char *getenv(const char *name);
typedef void* (* GLFWallocatefun)(size_t size, void* user); typedef void* (* GLFWreallocatefun)(void* block, size_t size, void* user); typedef void (* GLFWdeallocatefun)(void* block, void* user);
typedef int GLFWbool;
typedef void (*GLFWproc)(void);
typedef struct _GLFWinitconfig _GLFWinitconfig; typedef struct _GLFWctxconfig _GLFWctxconfig;
typedef struct _GLFWfbconfig _GLFWfbconfig; typedef struct _GLFWcontext _GLFWcontext; typedef struct _GLFWwindow _GLFWwindow;
typedef struct _GLFWlibrary _GLFWlibrary; typedef struct _GLFWmonitor _GLFWmonitor;
typedef struct _GLFWcursor _GLFWcursor; typedef struct _GLFWmapelement _GLFWmapelement; typedef struct _GLFWmapping _GLFWmapping;
typedef struct _GLFWjoystick _GLFWjoystick; typedef struct _GLFWtls _GLFWtls; typedef struct _GLFWmutex _GLFWmutex;
#if defined(WINDOWS)
    #define IsWindows8OrGreater()                              \
        _glfwIsWindowsVersionOrGreaterWin32(HIBYTE(0x0602),LOBYTE(0x0602), 0)
    #define IsWindows8Point1OrGreater()                     \
        _glfwIsWindowsVersionOrGreaterWin32(HIBYTE(0x0603),LOBYTE(0x0603), 0)
    #define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
    typedef DWORD (WINAPI * PFN_XInputGetCapabilities)(DWORD,DWORD,XINPUT_CAPABILITIES*);
    typedef DWORD (WINAPI * PFN_XInputGetState)(DWORD,XINPUT_STATE*);
    #define XInputGetCapabilities _glfw.win32.xinput.GetCapabilities
    #define XInputGetState _glfw.win32.xinput.GetState
    typedef HRESULT (WINAPI * PFN_DirectInput8Create)(HINSTANCE,DWORD,REFIID,LPVOID*,LPUNKNOWN);
    #define DirectInput8Create _glfw.win32.dinput8.Create
    typedef HRESULT (WINAPI * PFN_DwmIsCompositionEnabled)(BOOL*);
    typedef HRESULT (WINAPI * PFN_DwmFlush)(VOID);
    #define DwmIsCompositionEnabled _glfw.win32.dwmapi.IsCompositionEnabled
    #define DwmFlush _glfw.win32.dwmapi.Flush
    typedef LONG (WINAPI * PFN_RtlVerifyVersionInfo)(OSVERSIONINFOEXW*,ULONG,ULONGLONG);
    #define RtlVerifyVersionInfo _glfw.win32.ntdll.RtlVerifyVersionInfo_
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
        GLFWbool EXT_swap_control,ARB_create_context,ARB_create_context_profile;
    } _GLFWlibraryWGL;

    typedef struct _GLFWwindowWin32 { HWND handle; GLFWbool cursorTracked,frameAction,keymenu; int width,height,lastCursorPosX,lastCursorPosY; } _GLFWwindowWin32;
    typedef struct _GLFWlibraryWin32 {
        HINSTANCE           instance;
        HWND                helperWindowHandle;
        ATOM helperWindowClass,mainWindowClass;
        HDEVNOTIFY          deviceNotificationHandle;
        short int           keycodes[512],scancodes[GLFW_KEY_LAST + 1];
        double              restoreCursorPosX, restoreCursorPosY;
        _GLFWwindow*        disabledCursorWindow;
        _GLFWwindow*        capturedCursorWindow;
        RAWINPUT*           rawInput;
        int                 rawInputSize;
        HCURSOR             blankCursor;
        struct { HINSTANCE instance; PFN_DirectInput8Create Create; IDirectInput8W* api; } dinput8;
        struct { HINSTANCE instance; PFN_XInputGetCapabilities GetCapabilities; PFN_XInputGetState GetState; } xinput;
        struct { HINSTANCE instance; } user32;
        struct { HINSTANCE instance; PFN_DwmIsCompositionEnabled IsCompositionEnabled; PFN_DwmFlush Flush; } dwmapi;
        struct { HINSTANCE instance; PFN_RtlVerifyVersionInfo RtlVerifyVersionInfo_; } ntdll;
    } _GLFWlibraryWin32;
    typedef struct _GLFWmonitorWin32 { HMONITOR handle; WCHAR adapterName[32],displayName[32]; char publicAdapterName[32],publicDisplayName[32]; GLFWbool modesPruned,modeChanged; } _GLFWmonitorWin32;
    typedef struct _GLFWcursorWin32 { HCURSOR handle; } _GLFWcursorWin32;
    WCHAR* _glfwCreateWideStringFromUTF8Win32(const char* source);
    BOOL _glfwIsWindowsVersionOrGreaterWin32(WORD major, WORD minor, WORD sp);
    void _glfwPollMonitorsWin32(void);
    void _glfwGetWindowSizeWin32(_GLFWwindow* window, int* width, int* height);
    void _glfwGetCursorPosWin32(_GLFWwindow* window, double* xpos, double* ypos);
    void _glfwSetCursorPosWin32(_GLFWwindow* window, double xpos, double ypos);
    GLFWvidmode* _glfwGetVideoModesWin32(_GLFWmonitor* monitor, int* count);
    GLFWbool _glfwGetVideoModeWin32(_GLFWmonitor* monitor, GLFWvidmode* mode);
    #define GLFW_X11_WINDOW_STATE
    #define GLFW_X11_MONITOR_STATE
    #define GLFW_X11_CURSOR_STATE
    #define GLFW_X11_LIBRARY_WINDOW_STATE
    #define GLFW_GLX_CONTEXT_STATE
    #define GLFW_GLX_LIBRARY_CONTEXT_STATE
    #define GLFW_WIN32_JOYSTICK_STATE _GLFWjoystickWin32 win32;
    #define GLFW_WIN32_LIBRARY_JOYSTICK_STATE
    typedef struct _GLFWjoyobjectWin32 { int offset,type; } _GLFWjoyobjectWin32;
    typedef struct _GLFWjoystickWin32{ _GLFWjoyobjectWin32* objects; int objectCount; IDirectInputDevice8W* device; DWORD index; GUID guid; } _GLFWjoystickWin32;
    void _glfwDetectJoystickConnectionWin32(void);
    void _glfwDetectJoystickDisconnectionWin32(void);
    #define GLFW_LINUX_JOYSTICK_STATE
    #define GLFW_LINUX_LIBRARY_JOYSTICK_STATE
    #define GLFW_WIN32_TLS_STATE            _GLFWtlsWin32     win32;
    #define GLFW_WIN32_MUTEX_STATE          _GLFWmutexWin32   win32;
    typedef struct _GLFWtlsWin32 { GLFWbool allocated; DWORD index; } _GLFWtlsWin32;
    typedef struct _GLFWmutexWin32 { GLFWbool allocated; CRITICAL_SECTION section; } _GLFWmutexWin32;
    #define GLFW_PLATFORM_TLS_STATE    GLFW_WIN32_TLS_STATE
    #define GLFW_PLATFORM_MUTEX_STATE  GLFW_WIN32_MUTEX_STATE
    #define GLFW_WIN32_LIBRARY_TIMER_STATE  _GLFWtimerWin32   win32;
    typedef struct _GLFWtimerWin32 { u64 frequency; } _GLFWtimerWin32;
    #define GLFW_PLATFORM_LIBRARY_TIMER_STATE  GLFW_WIN32_LIBRARY_TIMER_STATE
#else // LINUX
    #define GLFW_WIN32_WINDOW_STATE
    #define GLFW_WIN32_MONITOR_STATE
    #define GLFW_WIN32_CURSOR_STATE
    #define GLFW_WIN32_LIBRARY_WINDOW_STATE
    #define GLFW_WGL_CONTEXT_STATE
    #define GLFW_WGL_LIBRARY_CONTEXT_STATE
    typedef unsigned char KeyCode; typedef int Bool; typedef unsigned long Atom; typedef unsigned long KeySym;
    #define	XkbActionMessageLength 6
    #define XA_ATOM ((Atom) 4)
    #define XA_CARDINAL ((Atom) 6)
    #define XA_WINDOW ((Atom) 33)
    typedef int XrmQuark, *XrmQuarkList;
    typedef char *XPointer;
    typedef struct { unsigned int size; XPointer addr; } XrmValue, *XrmValuePtr;
    typedef struct _XrmHashBucketRec *XrmDatabase;
    typedef unsigned int XcursorUInt; typedef XcursorUInt XcursorDim; typedef XcursorUInt XcursorPixel;
    typedef struct _XcursorImage { XcursorUInt version; XcursorDim size,width,height,xhot,yhot; XcursorUInt delay; XcursorPixel *pixels; } XcursorImage;
    typedef unsigned short Rotation,SubpixelOrder,Connection;
    typedef struct { long flags; int x,y, width,height,min_width,min_height,max_width,max_height,width_inc,height_inc; struct {int x; int y;} min_aspect,max_aspect; int base_width, base_height; int win_gravity; } XSizeHints;
    typedef unsigned long XID,Mask,Atom,VisualID,Time;
    typedef XID Window,Drawable,Font,Pixmap,Cursor,Colormap;
    #define None                 0L	/* universal null resource or null atom */
    #define CurrentTime          0L	/* special Time */
    #define KeyPressMask			(1L<<0)
    #define KeyReleaseMask			(1L<<1)
    #define ButtonPressMask			(1L<<2)
    #define ButtonReleaseMask		(1L<<3)
    #define EnterWindowMask			(1L<<4)
    #define LeaveWindowMask			(1L<<5)
    #define PointerMotionMask		(1L<<6)
    #define ExposureMask			(1L<<15)
    #define VisibilityChangeMask		(1L<<16)
    #define StructureNotifyMask		(1L<<17)
    #define SubstructureNotifyMask		(1L<<19)
    #define SubstructureRedirectMask	(1L<<20)
    #define FocusChangeMask			(1L<<21)
    #define PropertyChangeMask		(1L<<22)
    #define KeyPress		2
    #define KeyRelease		3
    #define ButtonPress		4
    #define ButtonRelease		5
    #define MotionNotify		6
    #define EnterNotify		7
    #define LeaveNotify		8
    #define FocusIn			9
    #define FocusOut		10
    #define VisibilityNotify	15
    #define DestroyNotify		17
    #define ReparentNotify		21
    #define ConfigureNotify		22
    #define PropertyNotify		28
    #define ClientMessage		33
    #define GenericEvent		35
    #define NotifyGrab		1
    #define NotifyUngrab		2
    #define PropertyNewValue	0
    #define Success		   0	/* everything's okay */
    #define PropModeReplace         0
    #define PropModeAppend          2
    #define DontPreferBlanking	0
    #define DefaultExposures	2
    #ifdef __clang__
        #pragma clang diagnostic push
        #pragma clang diagnostic ignored "-Wpadded"
    #endif
    #define Bool int
    #define Status int
    #define True 1
    #define False 0
    #define ConnectionNumber(dpy) 	(((_XPrivDisplay)(dpy))->fd)
    #define RootWindow(dpy, scr) 	(ScreenOfDisplay(dpy,scr)->root)
    #define DefaultScreen(dpy) 	(((_XPrivDisplay)(dpy))->default_screen)
    #define DefaultVisual(dpy, scr) (ScreenOfDisplay(dpy,scr)->root_visual)
    #define QLength(dpy) 		(((_XPrivDisplay)(dpy))->qlen)
    #define DisplayWidth(dpy, scr) 	(ScreenOfDisplay(dpy,scr)->width)
    #define DisplayHeight(dpy, scr) (ScreenOfDisplay(dpy,scr)->height)
    #define DisplayWidthMM(dpy, scr)(ScreenOfDisplay(dpy,scr)->mwidth)
    #define DisplayHeightMM(dpy, scr)(ScreenOfDisplay(dpy,scr)->mheight)
    #define DefaultDepth(dpy, scr) 	(ScreenOfDisplay(dpy,scr)->root_depth)
    #define ScreenOfDisplay(dpy, scr)(&((_XPrivDisplay)(dpy))->screens[scr])
    typedef struct _XExtData { int number; struct _XExtData *next; int (*free_private)(struct _XExtData*); XPointer private_data; } XExtData;
    typedef struct { int extension, major_opcode, first_event, first_error; } XExtCodes;
    typedef struct { int depth, bits_per_pixel, scanline_pad; } XPixmapFormatValues;
    typedef struct { int function; unsigned long plane_mask, foreground, background; int line_width, line_style, cap_style, join_style, fill_style, fill_rule, arc_mode; Pixmap tile, stipple; int ts_x_origin, ts_y_origin; Font font; int subwindow_mode; Bool graphics_exposures; int clip_x_origin, clip_y_origin; Pixmap clip_mask; int dash_offset; char dashes; } XGCValues;
    typedef struct _XGC
    #ifdef XLIB_ILLEGAL_ACCESS
    {
        XExtData *ext_data;	/* hook for extension to hang data */
        GContext gid;	/* protocol ID for graphics context */
        /* there is more to this structure, but it is private to Xlib */
    }
    #endif
    *GC;
    typedef struct { XExtData *ext_data; VisualID visualid; int class; unsigned long red_mask, green_mask, blue_mask; int bits_per_rgb; int map_entries;} Visual;
    typedef struct { int depth,nvisuals; Visual *visuals; } Depth;
    struct _XDisplay;		/* Forward declare before use for C++ */
    typedef struct { XExtData *ext_data; struct _XDisplay *display; Window root; int width,height,mwidth,mheight,ndepths; Depth *depths; int root_depth; Visual *root_visual; GC default_gc; Colormap cmap; unsigned long white_pixel, black_pixel; int max_maps, min_maps, backing_store; Bool save_unders; long root_input_mask; } Screen;
    typedef struct { XExtData *ext_data; int depth, bits_per_pixel, scanline_pad; } ScreenFormat;
    typedef struct { Pixmap background_pixmap; unsigned long background_pixel; Pixmap border_pixmap; unsigned long border_pixel; int bit_gravity, win_gravity, backing_store; unsigned long backing_planes, backing_pixel; Bool save_under; long event_mask, do_not_propagate_mask; Bool override_redirect; Colormap colormap; Cursor cursor; } XSetWindowAttributes;
    typedef struct { int x,y,width,height,border_width,depth; Visual *visual; Window root; int class,bit_gravity,win_gravity,backing_store; unsigned long backing_planes, backing_pixel; Bool save_under; Colormap colormap; Bool map_installed; int map_state; long all_event_masks, your_event_mask, do_not_propagate_mask; Bool override_redirect; Screen *screen; } XWindowAttributes;
    #ifndef XLIB_ILLEGAL_ACCESS
    typedef struct _XDisplay Display;
    #endif
    struct _XPrivate;		/* Forward declare before use for C++ */
    struct _XrmHashBucketRec;
    typedef struct
    #ifdef XLIB_ILLEGAL_ACCESS
    _XDisplay
    #endif
    { XExtData *ext_data; struct _XPrivate *private1; int fd, private2, proto_major_version, proto_minor_version; char *vendor; XID private3, private4, private5; int private6; XID (*resource_alloc)(struct _XDisplay*); int byte_order, bitmap_unit, bitmap_pad, bitmap_bit_order, nformats; ScreenFormat *pixmap_format; int private8; struct _XPrivate *private9, *private10; int qlen; unsigned long last_request_read, request; XPointer private11, private12, private13, private14; unsigned max_request_size; struct _XrmHashBucketRec *db; int (*private15)(struct _XDisplay*); char *display_name; int default_screen, nscreens; Screen *screens; unsigned long motion_buffer, private16; int min_keycode, max_keycode; XPointer private17, private18; int private19; char *xdefaults; }
    #ifdef XLIB_ILLEGAL_ACCESS
    Display,
    #endif
    *_XPrivDisplay;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Window window, root, subwindow; Time time; int x, y, x_root, y_root; unsigned int state, keycode; Bool same_screen; } XKeyEvent;
    typedef XKeyEvent XKeyPressedEvent;
    typedef XKeyEvent XKeyReleasedEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Window window, root, subwindow; Time time; int x, y, x_root, y_root; unsigned int state, button; Bool same_screen; } XButtonEvent;
    typedef XButtonEvent XButtonPressedEvent;
    typedef XButtonEvent XButtonReleasedEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Window window, root, subwindow; Time time; int x, y, x_root, y_root; unsigned int state; char is_hint; Bool same_screen; } XMotionEvent;
    typedef XMotionEvent XPointerMovedEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Window window, root, subwindow; Time time; int x, y, x_root, y_root, mode, detail; Bool same_screen, focus; unsigned int state; } XCrossingEvent;
    typedef XCrossingEvent XEnterWindowEvent;
    typedef XCrossingEvent XLeaveWindowEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Window window; int mode, detail; } XFocusChangeEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Window window; char key_vector[32]; } XKeymapEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Window window; int x, y, width, height, count; } XExposeEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Drawable drawable; int x, y, width, height, count, major_code, minor_code; } XGraphicsExposeEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Drawable drawable; int major_code, minor_code; } XNoExposeEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Window window; int state; } XVisibilityEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Window parent, window; int x, y, width, height, border_width; Bool override_redirect; } XCreateWindowEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Window event, window; } XDestroyWindowEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Window event, window; Bool from_configure; } XUnmapEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Window event, window; Bool override_redirect; } XMapEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Window parent, window; } XMapRequestEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Window event, window, parent; int x, y; Bool override_redirect; } XReparentEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Window event, window; int x, y, width, height, border_width; Window above; Bool override_redirect; } XConfigureEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Window event, window; int x, y; } XGravityEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Window window; int width, height; } XResizeRequestEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Window parent, window; int x, y, width, height, border_width; Window above; int detail; unsigned long value_mask; } XConfigureRequestEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Window event, window; int place; } XCirculateEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Window parent, window; int place; } XCirculateRequestEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Window window; Atom atom; Time time; int state; } XPropertyEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Window window; Atom selection; Time time; } XSelectionClearEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Window owner, requestor; Atom selection, target, property; Time time; } XSelectionRequestEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Window requestor; Atom selection, target, property; Time time; } XSelectionEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Window window; Colormap colormap; Bool new; int state; } XColormapEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Window window; Atom message_type; int format; union { char b[20]; short s[10]; long l[5]; } data; } XClientMessageEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Window window; int request, first_keycode, count; } XMappingEvent;
    typedef struct { int type; Display *display; XID resourceid; unsigned long serial; unsigned char error_code, request_code, minor_code; } XErrorEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Window window; } XAnyEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; int extension, evtype; } XGenericEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; int extension, evtype; unsigned int cookie; void *data; } XGenericEventCookie;
    typedef union _XEvent { int type; XAnyEvent xany; XKeyEvent xkey; XButtonEvent xbutton; XMotionEvent xmotion; XCrossingEvent xcrossing; XFocusChangeEvent xfocus; XExposeEvent xexpose; XGraphicsExposeEvent xgraphicsexpose; XNoExposeEvent xnoexpose; XVisibilityEvent xvisibility; XCreateWindowEvent xcreatewindow; XDestroyWindowEvent xdestroywindow; XUnmapEvent xunmap; XMapEvent xmap; XMapRequestEvent xmaprequest; XReparentEvent xreparent; XConfigureEvent xconfigure; XGravityEvent xgravity; XResizeRequestEvent xresizerequest; XConfigureRequestEvent xconfigurerequest; XCirculateEvent xcirculate; XCirculateRequestEvent xcirculaterequest; XPropertyEvent xproperty; XSelectionClearEvent xselectionclear; XSelectionRequestEvent xselectionrequest; XSelectionEvent xselection; XColormapEvent xcolormap; XClientMessageEvent xclient; XMappingEvent xmapping; XErrorEvent xerror; XKeymapEvent xkeymap; XGenericEvent xgeneric; XGenericEventCookie xcookie; long pad[24]; } XEvent;
    typedef struct _XIM *XIM; typedef struct _XIC *XIC;
    typedef void (*XIMProc)( XIM, XPointer, XPointer);
    typedef void (*XIDProc)( Display*, XPointer, XPointer);
    typedef unsigned long XIMStyle;
    typedef struct { unsigned short count_styles; XIMStyle *supported_styles; } XIMStyles;
    typedef struct { XPointer client_data; XIMProc callback; } XIMCallback;
    #ifdef __clang__
        #pragma clang diagnostic pop
    #endif
    typedef struct { long flags; Bool input; int initial_state; Pixmap icon_pixmap; Window icon_window;	int icon_x, icon_y;	Pixmap icon_mask; XID window_group; } XWMHints;
    typedef struct { char *res_name; char *res_class; } XClassHint;
    typedef struct _XComposeStatus { XPointer compose_ptr; int chars_matched; } XComposeStatus;
    typedef struct _XRegion *Region;
    typedef struct { Visual *visual; VisualID visualid; int screen; int depth; int class; unsigned long red_mask; unsigned long green_mask; unsigned long blue_mask; int colormap_size; int bits_per_rgb; } XVisualInfo;
    typedef int XContext;
    typedef XID RROutput,RRCrtc,RRMode;
    typedef unsigned long XRRModeFlags;
    typedef struct _XRRModeInfo { RRMode id; unsigned int width; unsigned int height; unsigned long dotClock; unsigned int hSyncStart; unsigned int hSyncEnd; unsigned int hTotal; unsigned int hSkew; unsigned int vSyncStart; unsigned int vSyncEnd; unsigned int vTotal; char *name; unsigned int nameLength; XRRModeFlags modeFlags; } XRRModeInfo;
    typedef struct _XRRScreenResources { Time timestamp; Time configTimestamp; int ncrtc; RRCrtc *crtcs; int noutput; RROutput *outputs; int nmode; XRRModeInfo *modes; } XRRScreenResources;
    typedef struct _XRROutputInfo { Time timestamp; RRCrtc crtc; char *name; int nameLen; unsigned long mm_width; unsigned long mm_height; Connection connection; SubpixelOrder subpixel_order; int ncrtc; RRCrtc *crtcs; int nclone; RROutput *clones; int nmode; int npreferred; RRMode *modes; } XRROutputInfo;
    typedef struct _XRRCrtcInfo { Time timestamp; int x, y; unsigned int width, height; RRMode mode; Rotation rotation; int noutput; RROutput *outputs; Rotation rotations; int npossible; RROutput *possible; } XRRCrtcInfo;
    #define	XkbStateNotify			2
    #define	XkbGroupStateMask		(1L << 4)
    #define	XkbUseCoreKbd		0x0100
    #define	XkbNumKbdGroups		4
    #define	XkbMaxLegalKeyCode	255
    #define	XkbPerKeyBitArraySize	((XkbMaxLegalKeyCode+1)/8)
    #define	XkbNumVirtualMods	16
    #define	XkbNumIndicators	32
    #define	XkbKeyNameLength	4
    #define	XkbKeyNamesMask		(1<<9)
    #define	XkbKeyAliasesMask	(1<<10)
    #define	XkbCharToInt(v)		((v)&0x80?(int)((v)|(~0xff)):(int)((v)&0x7f))
    #define	XkbIntTo2Chars(i,h,l)	(((h)=((i>>8)&0xff)),((l)=((i)&0xff)))
    #define	Xkb2CharsToInt(h,l)	((short)(((h)<<8)|(l)))
    #ifdef __clang__
        #pragma clang diagnostic push
        #pragma clang diagnostic ignored "-Wpadded"
    #endif
    #define	XkbModLocks(s)	 ((s)->locked_mods)
    #define	XkbStateMods(s)	 ((s)->base_mods|(s)->latched_mods|XkbModLocks(s))
    #define	XkbGroupLock(s)	 ((s)->locked_group)
    #define	XkbStateGroup(s) ((s)->base_group+(s)->latched_group+XkbGroupLock(s))
    #define	XkbStateFieldFromRec(s)	XkbBuildCoreState((s)->lookup_mods,(s)->group)
    #define	XkbGrabStateFromRec(s)	XkbBuildCoreState((s)->grab_mods,(s)->group)
    typedef struct { unsigned char group, locked_group; unsigned short base_group, latched_group; unsigned char mods, base_mods, latched_mods, locked_mods, compat_state, grab_mods, compat_grab_mods, lookup_mods, compat_lookup_mods; unsigned short ptr_buttons; } XkbStateRec, *XkbStatePtr;
    typedef struct { unsigned char mask, real_mods; unsigned short vmods; } XkbModsRec, *XkbModsPtr;
    typedef struct { Bool active; unsigned char level; XkbModsRec mods; } XkbKTMapEntryRec, *XkbKTMapEntryPtr;
    typedef struct { XkbModsRec mods; unsigned char num_levels, map_count; XkbKTMapEntryPtr map; XkbModsPtr preserve; Atom name; Atom *level_names; } XkbKeyTypeRec, *XkbKeyTypePtr;
    #define	XkbNumGroups(g)			((g)&0x0f)
    #define	XkbOutOfRangeGroupInfo(g)	((g)&0xf0)
    #define	XkbOutOfRangeGroupAction(g)	((g)&0xc0)
    #define	XkbOutOfRangeGroupNumber(g)	(((g)&0x30)>>4)
    #define	XkbSetGroupInfo(g,w,n)	(((w)&0xc0)|(((n)&3)<<4)|((g)&0x0f))
    #define	XkbSetNumGroups(g,n)	(((g)&0xf0)|((n)&0x0f))
    typedef struct _XkbBehavior { unsigned char type,data; } XkbBehavior;
    #define	XkbAnyActionDataSize 7
    typedef	struct _XkbAnyAction { unsigned char type,data[XkbAnyActionDataSize]; } XkbAnyAction;
    typedef struct _XkbModAction { unsigned char type,flags,mask,real_mods,vmods1,vmods2; } XkbModAction;
    #define	XkbModActionVMods(a)      \
        ((short)(((a)->vmods1<<8)|((a)->vmods2)))
    #define	XkbSetModActionVMods(a,v) \
        (((a)->vmods1=(((v)>>8)&0xff)),(a)->vmods2=((v)&0xff))
    typedef struct _XkbGroupAction { unsigned char type,flags; char group_XXX; } XkbGroupAction;
    #define	XkbSAGroup(a)		(XkbCharToInt((a)->group_XXX))
    #define	XkbSASetGroup(a,g)	((a)->group_XXX=(g))
    typedef struct _XkbISOAction { unsigned char type,flags,mask,real_mods; char group_XXX; unsigned char affect,vmods1,vmods2; } XkbISOAction;
    typedef struct _XkbPtrAction { unsigned char type,flags,high_XXX,low_XXX,high_YYY,low_YYY; } XkbPtrAction;
    typedef struct _XkbPtrBtnAction { unsigned char type,flags,count,button; } XkbPtrBtnAction;
    typedef struct _XkbPtrDfltAction { unsigned char type,flags,affect; char valueXXX; } XkbPtrDfltAction;
    typedef struct _XkbSwitchScreenAction { unsigned char type,flags; char screenXXX; } XkbSwitchScreenAction;
    typedef struct { unsigned char type, flags, ctrls3, ctrls2, ctrls1, ctrls0; } XkbCtrlsAction;
    typedef struct { unsigned char type, flags, message[6]; } XkbMessageAction;
    typedef struct { unsigned char type, new_key, mods_mask, mods, vmods_mask0, vmods_mask1, vmods0, vmods1; } XkbRedirectKeyAction;
    typedef struct { unsigned char type, flags, count, button, device; } XkbDeviceBtnAction;
    typedef struct { unsigned char type, device, v1_what, v1_ndx, v1_value, v2_what, v2_ndx, v2_value; } XkbDeviceValuatorAction;
    typedef union { XkbAnyAction any; XkbModAction mods; XkbGroupAction group; XkbISOAction iso; XkbPtrAction ptr; XkbPtrBtnAction btn; XkbPtrDfltAction dflt; XkbSwitchScreenAction screen; XkbCtrlsAction ctrls; XkbMessageAction msg; XkbRedirectKeyAction redirect; XkbDeviceBtnAction devbtn; XkbDeviceValuatorAction devval; unsigned char type; } XkbAction;
    typedef struct { unsigned char mk_dflt_btn, num_groups, groups_wrap; XkbModsRec internal, ignore_lock; unsigned int enabled_ctrls; unsigned short repeat_delay, repeat_interval, slow_keys_delay, debounce_delay, mk_delay, mk_interval, mk_time_to_max, mk_max_speed; short mk_curve; unsigned short ax_options, ax_timeout, axt_opts_mask, axt_opts_values; unsigned int axt_ctrls_mask, axt_ctrls_values; unsigned char per_key_repeat[XkbPerKeyBitArraySize]; } XkbControlsRec, *XkbControlsPtr;
    typedef struct { unsigned short num_acts, size_acts; XkbAction *acts; XkbBehavior *behaviors; unsigned short *key_acts; unsigned char *explicit, vmods[XkbNumVirtualMods]; unsigned short *vmodmap; } XkbServerMapRec, *XkbServerMapPtr;
    typedef struct { unsigned char kt_index[XkbNumKbdGroups], group_info, width; unsigned short offset; } XkbSymMapRec, *XkbSymMapPtr;
    typedef struct { unsigned char size_types, num_types; XkbKeyTypePtr types; unsigned short size_syms, num_syms; KeySym *syms; XkbSymMapPtr key_sym_map; unsigned char *modmap; } XkbClientMapRec, *XkbClientMapPtr;
    typedef struct { KeySym sym; unsigned char flags, match, mods, virtual_mod; XkbAnyAction act; } XkbSymInterpretRec, *XkbSymInterpretPtr;
    typedef struct { XkbSymInterpretPtr sym_interpret; XkbModsRec groups[XkbNumKbdGroups]; unsigned short num_si, size_si; } XkbCompatMapRec, *XkbCompatMapPtr;
    typedef struct { unsigned char flags, which_groups, groups, which_mods; XkbModsRec mods; unsigned int ctrls; } XkbIndicatorMapRec, *XkbIndicatorMapPtr;
    typedef struct { unsigned long phys_indicators; XkbIndicatorMapRec maps[XkbNumIndicators]; } XkbIndicatorRec, *XkbIndicatorPtr;
    typedef struct { char name[XkbKeyNameLength]; } XkbKeyNameRec, *XkbKeyNamePtr;
    typedef struct { char real[XkbKeyNameLength], alias[XkbKeyNameLength]; } XkbKeyAliasRec, *XkbKeyAliasPtr;
    typedef struct { Atom keycodes, geometry, symbols, types, compat, vmods[XkbNumVirtualMods], indicators[XkbNumIndicators], groups[XkbNumKbdGroups]; XkbKeyNamePtr keys; XkbKeyAliasPtr key_aliases; Atom *radio_groups, phys_symbols; unsigned char num_keys, num_key_aliases; unsigned short num_rg; } XkbNamesRec, *XkbNamesPtr;
    typedef struct _XkbGeometry *XkbGeometryPtr;
    typedef struct { struct _XDisplay *dpy; unsigned short flags, device_spec; KeyCode min_key_code, max_key_code; XkbControlsPtr ctrls; XkbServerMapPtr server; XkbClientMapPtr map; XkbIndicatorPtr indicators; XkbNamesPtr names; XkbCompatMapPtr compat; XkbGeometryPtr geom; } XkbDescRec, *XkbDescPtr;
    typedef struct { unsigned short changed; KeyCode min_key_code, max_key_code; unsigned char first_type, num_types; KeyCode first_key_sym; unsigned char num_key_syms; KeyCode first_key_act; unsigned char num_key_acts; KeyCode first_key_behavior, num_key_behaviors, first_key_explicit, num_key_explicit, first_modmap_key, num_modmap_keys, first_vmodmap_key, num_vmodmap_keys, pad; unsigned short vmods; } XkbMapChangesRec, *XkbMapChangesPtr;
    typedef struct { unsigned int changed_ctrls, enabled_ctrls_changes; Bool num_groups_changed; } XkbControlsChangesRec, *XkbControlsChangesPtr;
    typedef struct { unsigned int state_changes, map_changes; } XkbIndicatorChangesRec, *XkbIndicatorChangesPtr;
    typedef struct { unsigned int changed; unsigned char first_type, num_types, first_lvl, num_lvls, num_aliases, num_rg, first_key, num_keys; unsigned short changed_vmods; unsigned long changed_indicators; unsigned char changed_groups; } XkbNameChangesRec, *XkbNameChangesPtr;
    typedef struct { unsigned char changed_groups; unsigned short first_si, num_si; } XkbCompatChangesRec, *XkbCompatChangesPtr;
    typedef struct { unsigned short device_spec, state_changes; XkbMapChangesRec map; XkbControlsChangesRec ctrls; XkbIndicatorChangesRec indicators; XkbNameChangesRec names; XkbCompatChangesRec compat; } XkbChangesRec, *XkbChangesPtr;
    typedef struct { char *keymap, *keycodes, *types, *compat, *symbols, *geometry; } XkbComponentNamesRec, *XkbComponentNamesPtr;
    typedef struct { unsigned short flags; char *name; } XkbComponentNameRec, *XkbComponentNamePtr;
    typedef struct { int num_keymaps, num_keycodes, num_types, num_compat, num_symbols, num_geometry; XkbComponentNamePtr keymaps, keycodes, types, compat, symbols, geometry; } XkbComponentListRec, *XkbComponentListPtr;
    typedef struct { unsigned short led_class, led_id; unsigned int phys_indicators, maps_present, names_present, state; Atom names[XkbNumIndicators]; XkbIndicatorMapRec maps[XkbNumIndicators]; } XkbDeviceLedInfoRec, *XkbDeviceLedInfoPtr;
    typedef struct { char *name; Atom type; unsigned short device_spec; Bool has_own_state; unsigned short supported, unsupported, num_btns; XkbAction *btn_acts; unsigned short sz_leds, num_leds, dflt_kbd_fb, dflt_led_fb; XkbDeviceLedInfoPtr leds; } XkbDeviceInfoRec, *XkbDeviceInfoPtr;
    typedef struct _XkbDeviceLedChanges { unsigned short led_class, led_id; unsigned int defined; struct _XkbDeviceLedChanges *next; } XkbDeviceLedChangesRec, *XkbDeviceLedChangesPtr;
    typedef struct { unsigned int changed; unsigned short first_btn, num_btns; XkbDeviceLedChangesRec leds; } XkbDeviceChangesRec, *XkbDeviceChangesPtr;
    #ifdef __clang__
        #pragma clang diagnostic pop
    #endif
    typedef struct _XkbAnyEvent { int type; unsigned long serial; Bool send_event; Display *display; Time time; int xkb_type; unsigned int device; } XkbAnyEvent;
    typedef struct _XkbNewKeyboardNotify { int type; unsigned long serial; Bool send_event; Display *display; Time time; int xkb_type; int device; int old_device; int min_key_code; int max_key_code; int old_min_key_code; int old_max_key_code; unsigned int changed; char req_major; char req_minor; } XkbNewKeyboardNotifyEvent;
    typedef struct _XkbMapNotifyEvent { int type; unsigned long serial; Bool send_event; Display *display; Time time; int xkb_type; int device; unsigned int changed; unsigned int flags; int first_type; int num_types; KeyCode min_key_code; KeyCode max_key_code; KeyCode first_key_sym; KeyCode first_key_act; KeyCode first_key_behavior; KeyCode first_key_explicit; KeyCode first_modmap_key; KeyCode first_vmodmap_key; int num_key_syms; int num_key_acts; int num_key_behaviors; int num_key_explicit; int num_modmap_keys; int num_vmodmap_keys; unsigned int vmods; } XkbMapNotifyEvent;
    typedef struct _XkbStateNotifyEvent { int type; unsigned long serial; Bool send_event; Display *display; Time time; int xkb_type; int device; unsigned int changed; int group; int base_group; int latched_group; int locked_group; unsigned int mods; unsigned int base_mods; unsigned int latched_mods; unsigned int locked_mods; int compat_state; unsigned char grab_mods; unsigned char compat_grab_mods; unsigned char lookup_mods; unsigned char compat_lookup_mods; int ptr_buttons; KeyCode keycode; char event_type; char req_major; char req_minor; } XkbStateNotifyEvent;
    typedef struct _XkbControlsNotify { int type; unsigned long serial; Bool send_event; Display *display; Time time; int xkb_type; int device; unsigned int changed_ctrls; unsigned int enabled_ctrls; unsigned int enabled_ctrl_changes; int num_groups; KeyCode keycode; char event_type; char req_major; char req_minor; } XkbControlsNotifyEvent;
    typedef struct _XkbIndicatorNotify { int type; unsigned long serial; Bool send_event; Display *display; Time time; int xkb_type; int device; unsigned int changed; unsigned int state; } XkbIndicatorNotifyEvent;
    typedef struct _XkbNamesNotify { int type; unsigned long serial; Bool send_event; Display *display; Time time; int xkb_type; int device; unsigned int changed; int first_type; int num_types; int first_lvl; int num_lvls; int num_aliases; int num_radio_groups; unsigned int changed_vmods; unsigned int changed_groups; unsigned int changed_indicators; int first_key; int num_keys; } XkbNamesNotifyEvent;
    typedef struct _XkbCompatMapNotify { int type; unsigned long serial; Bool send_event; Display *display; Time time; int xkb_type; int device; unsigned int changed_groups; int first_si; int num_si; int num_total_si; } XkbCompatMapNotifyEvent;
    typedef struct _XkbBellNotify { int type; unsigned long serial; Bool send_event; Display *display; Time time; int xkb_type; int device; int percent; int pitch; int duration; int bell_class; int bell_id; Atom name; Window window; Bool event_only; } XkbBellNotifyEvent;
    typedef struct _XkbActionMessage { int type; unsigned long serial; Bool send_event; Display *display; Time time; int xkb_type; int device; KeyCode keycode; Bool press; Bool key_event_follows; int group; unsigned int mods; char message[XkbActionMessageLength+1]; } XkbActionMessageEvent;
    typedef struct _XkbAccessXNotify { int type; unsigned long serial; Bool send_event; Display *display; Time time; int xkb_type; int device; int detail; int keycode; int sk_delay; int debounce_delay; } XkbAccessXNotifyEvent;
    typedef struct _XkbExtensionDeviceNotify { int type; unsigned long serial; Bool send_event; Display *display; Time time; int xkb_type; int device; unsigned int reason; unsigned int supported; unsigned int unsupported; int first_btn; int num_btns; unsigned int leds_defined; unsigned int led_state; int led_class; int led_id; } XkbExtensionDeviceNotifyEvent;
    typedef union _XkbEvent { int type; XkbAnyEvent any; XkbNewKeyboardNotifyEvent new_kbd; XkbMapNotifyEvent map; XkbStateNotifyEvent state; XkbControlsNotifyEvent ctrls; XkbIndicatorNotifyEvent indicators; XkbNamesNotifyEvent names; XkbCompatMapNotifyEvent compat; XkbBellNotifyEvent bell; XkbActionMessageEvent message; XkbAccessXNotifyEvent accessx; XkbExtensionDeviceNotifyEvent device; XEvent core; } XkbEvent;
    typedef struct { int deviceid,mask_len; unsigned char* mask; } XIEventMask;
    #define XISetMask(ptr,event)   (((unsigned char*)(ptr))[(event)>>3] |=  (1 << ((event) & 7)))
    #define XIMaskIsSet(ptr,event) (((unsigned char*)(ptr))[(event)>>3] &   (1 << ((event) & 7)))
    #define XIMaskLen(event)        (((event) >> 3) + 1)
    #define XIAllMasterDevices 1
    typedef struct { int mask_len; unsigned char *mask; double *values; } XIValuatorState;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; int extension,evtype; Time time; int deviceid,sourceid,detail,flags; XIValuatorState valuators; double *raw_values; } XIRawEvent;
    typedef XID GLXWindow,GLXDrawable;
    typedef struct __GLXFBConfig* GLXFBConfig;
    typedef struct __GLXcontext* GLXContext;
    typedef void (*__GLXextproc)(void);
    typedef XClassHint* (* PFN_XAllocClassHint)(void);
    typedef XSizeHints* (* PFN_XAllocSizeHints)(void);
    typedef XWMHints* (* PFN_XAllocWMHints)(void);
    typedef int (* PFN_XChangeProperty)(Display*,Window,Atom,Atom,int,int,const unsigned char*,int);
    typedef int (* PFN_XChangeWindowAttributes)(Display*,Window,unsigned long,XSetWindowAttributes*);
    typedef Bool (* PFN_XCheckTypedWindowEvent)(Display*,Window,int,XEvent*);
    typedef int (* PFN_XCloseDisplay)(Display*);
    typedef Colormap (* PFN_XCreateColormap)(Display*,Window,Visual*,int);
    typedef XIC (* PFN_XCreateIC)(XIM,...);
    typedef Window (* PFN_XCreateWindow)(Display*,Window,int,int,unsigned int,unsigned int,unsigned int,int,unsigned int,Visual*,unsigned long,XSetWindowAttributes*);
    typedef int (* PFN_XDefineCursor)(Display*,Window,Cursor);
    typedef int (* PFN_XDeleteContext)(Display*,XID,XContext);
    typedef int (* PFN_XDeleteProperty)(Display*,Window,Atom);
    typedef int (* PFN_XDisplayKeycodes)(Display*,int*,int*);
    typedef Bool (* PFN_XFilterEvent)(XEvent*,Window);
    typedef int (* PFN_XFindContext)(Display*,XID,XContext,XPointer*);
    typedef int (* PFN_XFlush)(Display*);
    typedef int (* PFN_XFree)(void*);
    typedef void (* PFN_XFreeEventData)(Display*,XGenericEventCookie*);
    typedef Bool (* PFN_XGetEventData)(Display*,XGenericEventCookie*);
    typedef char* (* PFN_XGetICValues)(XIC,...);
    typedef char* (* PFN_XGetIMValues)(XIM,...);
    typedef int (* PFN_XGetInputFocus)(Display*,Window*,int*);
    typedef KeySym* (* PFN_XGetKeyboardMapping)(Display*,KeyCode,int,int*);
    typedef Status (* PFN_XGetWMNormalHints)(Display*,Window,XSizeHints*,long*);
    typedef Status (* PFN_XGetWindowAttributes)(Display*,Window,XWindowAttributes*);
    typedef int (* PFN_XGetWindowProperty)(Display*,Window,Atom,long,long,Bool,Atom,Atom*,int*,unsigned long*,unsigned long*,unsigned char**);
    typedef int (* PFN_XGrabPointer)(Display*,Window,Bool,unsigned int,int,int,Window,Cursor,Time);
    typedef Status (* PFN_XInitThreads)(void);
    typedef Atom (* PFN_XInternAtom)(Display*,const char*,Bool);
    typedef int (* PFN_XMapWindow)(Display*,Window);
    typedef int (* PFN_XMoveResizeWindow)(Display*,Window,int,int,unsigned int,unsigned int);
    typedef int (* PFN_XMoveWindow)(Display*,Window,int,int);
    typedef int (* PFN_XNextEvent)(Display*,XEvent*);
    typedef Display* (* PFN_XOpenDisplay)(const char*);
    typedef int (* PFN_XPending)(Display*);
    typedef Bool (* PFN_XQueryExtension)(Display*,const char*,int*,int*,int*);
    typedef Bool (* PFN_XQueryPointer)(Display*,Window,Window*,Window*,int*,int*,int*,int*,unsigned int*);
    typedef int (* PFN_XRaiseWindow)(Display*,Window);
    typedef Bool (* PFN_XRegisterIMInstantiateCallback)(Display*,void*,char*,char*,XIDProc,XPointer);
    typedef int (* PFN_XResizeWindow)(Display*,Window,unsigned int,unsigned int);
    typedef int (* PFN_XSaveContext)(Display*,XID,XContext,const char*);
    typedef int (* PFN_XSelectInput)(Display*,Window,long);
    typedef Status (* PFN_XSendEvent)(Display*,Window,Bool,long,XEvent*);
    typedef int (* PFN_XSetClassHint)(Display*,Window,XClassHint*);
    typedef void (* PFN_XSetICFocus)(XIC);
    typedef char* (* PFN_XSetIMValues)(XIM,...);
    typedef int (* PFN_XSetInputFocus)(Display*,Window,int,Time);
    typedef char* (* PFN_XSetLocaleModifiers)(const char*);
    typedef int (* PFN_XSetWMHints)(Display*,Window,XWMHints*);
    typedef void (* PFN_XSetWMNormalHints)(Display*,Window,XSizeHints*);
    typedef Status (* PFN_XSetWMProtocols)(Display*,Window,Atom*,int);
    typedef Bool (* PFN_XSupportsLocale)(void);
    typedef Bool (* PFN_XTranslateCoordinates)(Display*,Window,Window,int,int,int*,int*,Window*);
    typedef int (* PFN_XUndefineCursor)(Display*,Window);
    typedef int (* PFN_XUngrabPointer)(Display*,Time);
    typedef void (* PFN_XUnsetICFocus)(XIC);
    typedef int (* PFN_XWarpPointer)(Display*,Window,Window,int,int,unsigned int,unsigned int,int,int);
    typedef void (* PFN_XkbFreeKeyboard)(XkbDescPtr,unsigned int,Bool);
    typedef void (* PFN_XkbFreeNames)(XkbDescPtr,unsigned int,Bool);
    typedef XkbDescPtr (* PFN_XkbGetMap)(Display*,unsigned int,unsigned int);
    typedef Status (* PFN_XkbGetNames)(Display*,unsigned int,XkbDescPtr);
    typedef Status (* PFN_XkbGetState)(Display*,unsigned int,XkbStatePtr);
    typedef Bool (* PFN_XkbQueryExtension)(Display*,int*,int*,int*,int*,int*);
    typedef Bool (* PFN_XkbSelectEventDetails)(Display*,unsigned int,unsigned int,unsigned long,unsigned long);
    typedef Bool (* PFN_XkbSetDetectableAutoRepeat)(Display*,Bool,Bool*);
    typedef void (* PFN_Xutf8SetWMProperties)(Display*,Window,const char*,const char*,char**,int,XSizeHints*,XWMHints*,XClassHint*);
    #define XAllocClassHint _glfw.x11.xlib.AllocClassHint
    #define XAllocSizeHints _glfw.x11.xlib.AllocSizeHints
    #define XAllocWMHints _glfw.x11.xlib.AllocWMHints
    #define XChangeProperty _glfw.x11.xlib.ChangeProperty
    #define XChangeWindowAttributes _glfw.x11.xlib.ChangeWindowAttributes
    #define XCheckTypedWindowEvent _glfw.x11.xlib.CheckTypedWindowEvent
    #define XCloseDisplay _glfw.x11.xlib.CloseDisplay
    #define XCreateColormap _glfw.x11.xlib.CreateColormap
    #define XCreateIC _glfw.x11.xlib.CreateIC
    #define XCreateWindow _glfw.x11.xlib.CreateWindow
    #define XDefineCursor _glfw.x11.xlib.DefineCursor
    #define XDeleteContext _glfw.x11.xlib.DeleteContext
    #define XDeleteProperty _glfw.x11.xlib.DeleteProperty
    #define XDisplayKeycodes _glfw.x11.xlib.DisplayKeycodes
    #define XFilterEvent _glfw.x11.xlib.FilterEvent
    #define XFindContext _glfw.x11.xlib.FindContext
    #define XFlush _glfw.x11.xlib.Flush
    #define XFree _glfw.x11.xlib.Free
    #define XFreeEventData _glfw.x11.xlib.FreeEventData
    #define XGetEventData _glfw.x11.xlib.GetEventData
    #define XGetICValues _glfw.x11.xlib.GetICValues
    #define XGetIMValues _glfw.x11.xlib.GetIMValues
    #define XGetInputFocus _glfw.x11.xlib.GetInputFocus
    #define XGetKeyboardMapping _glfw.x11.xlib.GetKeyboardMapping
    #define XGetWMNormalHints _glfw.x11.xlib.GetWMNormalHints
    #define XGetWindowAttributes _glfw.x11.xlib.GetWindowAttributes
    #define XGetWindowProperty _glfw.x11.xlib.GetWindowProperty
    #define XGrabPointer _glfw.x11.xlib.GrabPointer
    #define XInternAtom _glfw.x11.xlib.InternAtom
    #define XMapWindow _glfw.x11.xlib.MapWindow
    #define XMoveResizeWindow _glfw.x11.xlib.MoveResizeWindow
    #define XMoveWindow _glfw.x11.xlib.MoveWindow
    #define XNextEvent _glfw.x11.xlib.NextEvent
    #define XPending _glfw.x11.xlib.Pending
    #define XQueryExtension _glfw.x11.xlib.QueryExtension
    #define XQueryPointer _glfw.x11.xlib.QueryPointer
    #define XRaiseWindow _glfw.x11.xlib.RaiseWindow
    #define XRegisterIMInstantiateCallback _glfw.x11.xlib.RegisterIMInstantiateCallback
    #define XResizeWindow _glfw.x11.xlib.ResizeWindow
    #define XSaveContext _glfw.x11.xlib.SaveContext
    #define XSelectInput _glfw.x11.xlib.SelectInput
    #define XSendEvent _glfw.x11.xlib.SendEvent
    #define XSetClassHint _glfw.x11.xlib.SetClassHint
    #define XSetICFocus _glfw.x11.xlib.SetICFocus
    #define XSetIMValues _glfw.x11.xlib.SetIMValues
    #define XSetInputFocus _glfw.x11.xlib.SetInputFocus
    #define XSetLocaleModifiers _glfw.x11.xlib.SetLocaleModifiers
    #define XSetWMHints _glfw.x11.xlib.SetWMHints
    #define XSetWMNormalHints _glfw.x11.xlib.SetWMNormalHints
    #define XSetWMProtocols _glfw.x11.xlib.SetWMProtocols
    #define XSupportsLocale _glfw.x11.xlib.SupportsLocale
    #define XTranslateCoordinates _glfw.x11.xlib.TranslateCoordinates
    #define XUndefineCursor _glfw.x11.xlib.UndefineCursor
    #define XUngrabPointer _glfw.x11.xlib.UngrabPointer
    #define XUnsetICFocus _glfw.x11.xlib.UnsetICFocus
    #define XWarpPointer _glfw.x11.xlib.WarpPointer
    #define XkbFreeKeyboard _glfw.x11.xkb.FreeKeyboard
    #define XkbFreeNames _glfw.x11.xkb.FreeNames
    #define XkbGetMap _glfw.x11.xkb.GetMap
    #define XkbGetNames _glfw.x11.xkb.GetNames
    #define XkbGetState _glfw.x11.xkb.GetState
    #define XkbQueryExtension _glfw.x11.xkb.QueryExtension
    #define XkbSelectEventDetails _glfw.x11.xkb.SelectEventDetails
    #define XkbSetDetectableAutoRepeat _glfw.x11.xkb.SetDetectableAutoRepeat
    #define Xutf8SetWMProperties _glfw.x11.xlib.utf8SetWMProperties
    typedef void (* PFN_XRRFreeCrtcInfo)(XRRCrtcInfo*);
    typedef void (* PFN_XRRFreeOutputInfo)(XRROutputInfo*);
    typedef void (* PFN_XRRFreeScreenResources)(XRRScreenResources*);
    typedef XRRCrtcInfo* (* PFN_XRRGetCrtcInfo) (Display*,XRRScreenResources*,RRCrtc);
    typedef XRROutputInfo* (* PFN_XRRGetOutputInfo)(Display*,XRRScreenResources*,RROutput);
    typedef RROutput (* PFN_XRRGetOutputPrimary)(Display*,Window);
    typedef XRRScreenResources* (* PFN_XRRGetScreenResourcesCurrent)(Display*,Window);
    typedef Bool (* PFN_XRRQueryExtension)(Display*,int*,int*);
    typedef Status (* PFN_XRRQueryVersion)(Display*,int*,int*);
    typedef void (* PFN_XRRSelectInput)(Display*,Window,int);
    typedef Status (* PFN_XRRSetCrtcConfig)(Display*,XRRScreenResources*,RRCrtc,Time,int,int,RRMode,Rotation,RROutput*,int);
    typedef int (* PFN_XRRUpdateConfiguration)(XEvent*);
    #define XRRFreeCrtcInfo _glfw.x11.randr.FreeCrtcInfo
    #define XRRFreeOutputInfo _glfw.x11.randr.FreeOutputInfo
    #define XRRFreeScreenResources _glfw.x11.randr.FreeScreenResources
    #define XRRGetCrtcInfo _glfw.x11.randr.GetCrtcInfo
    #define XRRGetOutputInfo _glfw.x11.randr.GetOutputInfo
    #define XRRGetOutputPrimary _glfw.x11.randr.GetOutputPrimary
    #define XRRGetScreenResourcesCurrent _glfw.x11.randr.GetScreenResourcesCurrent
    #define XRRQueryExtension _glfw.x11.randr.QueryExtension
    #define XRRQueryVersion _glfw.x11.randr.QueryVersion
    #define XRRSelectInput _glfw.x11.randr.SelectInput
    #define XRRSetCrtcConfig _glfw.x11.randr.SetCrtcConfig
    #define XRRUpdateConfiguration _glfw.x11.randr.UpdateConfiguration
    typedef XcursorImage* (* PFN_XcursorImageCreate)(int,int);
    typedef void (* PFN_XcursorImageDestroy)(XcursorImage*);
    typedef Cursor (* PFN_XcursorImageLoadCursor)(Display*,const XcursorImage*);
    #define XcursorImageCreate _glfw.x11.xcursor.ImageCreate
    #define XcursorImageDestroy _glfw.x11.xcursor.ImageDestroy
    #define XcursorImageLoadCursor _glfw.x11.xcursor.ImageLoadCursor
    typedef XID xcb_window_t;
    typedef XID xcb_visualid_t;
    typedef struct xcb_connection_t xcb_connection_t;
    typedef xcb_connection_t* (* PFN_XGetXCBConnection)(Display*);
    typedef Bool (* PFN_XF86VidModeQueryExtension)(Display*,int*,int*);
    #define XF86VidModeQueryExtension _glfw.x11.vidmode.QueryExtension
    typedef Status (* PFN_XIQueryVersion)(Display*,int*,int*);
    typedef int (* PFN_XISelectEvents)(Display*,Window,XIEventMask*,int);
    #define XIQueryVersion _glfw.x11.xi.QueryVersion
    #define XISelectEvents _glfw.x11.xi.SelectEvents
    typedef int (*PFNGLXGETFBCONFIGATTRIBPROC)(Display*,GLXFBConfig,int,int*);
    typedef const char* (*PFNGLXGETCLIENTSTRINGPROC)(Display*,int);
    typedef Bool (*PFNGLXQUERYEXTENSIONPROC)(Display*,int*,int*);
    typedef Bool (*PFNGLXQUERYVERSIONPROC)(Display*,int*,int*);
    typedef Bool (*PFNGLXMAKECURRENTPROC)(Display*,GLXDrawable,GLXContext);
    typedef void (*PFNGLXSWAPBUFFERSPROC)(Display*,GLXDrawable);
    typedef const char* (*PFNGLXQUERYEXTENSIONSSTRINGPROC)(Display*,int);
    typedef GLXFBConfig* (*PFNGLXGETFBCONFIGSPROC)(Display*,int,int*);
    typedef GLXContext (*PFNGLXCREATENEWCONTEXTPROC)(Display*,GLXFBConfig,int,GLXContext,Bool);
    typedef __GLXextproc (* PFNGLXGETPROCADDRESSPROC)(const GLubyte *procName);
    typedef void (*PFNGLXSWAPINTERVALEXTPROC)(Display*,GLXDrawable,int);
    typedef XVisualInfo* (*PFNGLXGETVISUALFROMFBCONFIGPROC)(Display*,GLXFBConfig);
    typedef GLXWindow (*PFNGLXCREATEWINDOWPROC)(Display*,GLXFBConfig,Window,const int*);
    typedef GLXContext (*PFNGLXCREATECONTEXTATTRIBSARBPROC)(Display*,GLXFBConfig,GLXContext,Bool,const int*);
    #define glXGetFBConfigAttrib _glfw.glx.GetFBConfigAttrib
    #define glXQueryExtension _glfw.glx.QueryExtension
    #define glXQueryVersion _glfw.glx.QueryVersion
    #define glXQueryExtensionsString _glfw.glx.QueryExtensionsString
    #define glXCreateNewContext _glfw.glx.CreateNewContext
    #define glXGetVisualFromFBConfig _glfw.glx.GetVisualFromFBConfig
    #define glXCreateWindow _glfw.glx.CreateWindow
    typedef unsigned long int nfds_t;
    struct pollfd { int fd; short events,revents;}; int poll(struct pollfd *fds, nfds_t nfds, int timeout);
    #define POLLIN 0x001
    GLFWbool _glfwPollPOSIX(struct pollfd* fds, nfds_t count, double* timeout);
    #define GLFW_X11_WINDOW_STATE           _GLFWwindowX11 x11;
    #define GLFW_X11_LIBRARY_WINDOW_STATE   _GLFWlibraryX11 x11;
    #define GLFW_X11_MONITOR_STATE          _GLFWmonitorX11 x11;
    #define GLFW_X11_CURSOR_STATE           _GLFWcursorX11 x11;
    #define GLFW_GLX_CONTEXT_STATE          _GLFWcontextGLX glx;
    #define GLFW_GLX_LIBRARY_CONTEXT_STATE  _GLFWlibraryGLX glx;
    typedef struct _GLFWcontextGLX { GLXContext handle; GLXWindow window; GLXFBConfig fbconfig; } _GLFWcontextGLX;
    typedef struct _GLFWlibraryGLX {
        int major, minor,eventBase,errorBase; void* handle;
        PFNGLXGETFBCONFIGSPROC              GetFBConfigs;
        PFNGLXGETFBCONFIGATTRIBPROC         GetFBConfigAttrib;
        PFNGLXQUERYEXTENSIONPROC            QueryExtension;
        PFNGLXQUERYVERSIONPROC              QueryVersion;
        PFNGLXMAKECURRENTPROC               MakeCurrent;
        PFNGLXSWAPBUFFERSPROC               SwapBuffers;
        PFNGLXQUERYEXTENSIONSSTRINGPROC     QueryExtensionsString;
        PFNGLXCREATENEWCONTEXTPROC          CreateNewContext;
        PFNGLXGETVISUALFROMFBCONFIGPROC     GetVisualFromFBConfig;
        PFNGLXCREATEWINDOWPROC              CreateWindow;
        PFNGLXGETPROCADDRESSPROC            GetProcAddress;
        PFNGLXGETPROCADDRESSPROC            GetProcAddressARB;
        PFNGLXSWAPINTERVALEXTPROC           SwapIntervalEXT;
        PFNGLXCREATECONTEXTATTRIBSARBPROC   CreateContextAttribsARB;
        GLFWbool EXT_swap_control,ARB_create_context,ARB_create_context_profile;
    } _GLFWlibraryGLX;

    typedef struct _GLFWwindowX11 { Colormap colormap; Window handle,parent; XIC ic; GLFWbool overrideRedirect; int width,height,xpos,ypos,lastCursorPosX,lastCursorPosY,warpCursorPosX,warpCursorPosY; } _GLFWwindowX11;
    typedef struct _GLFWlibraryX11 {
        Display* display;
        int screen;
        Window root;
        Window helperWindowHandle;
        Cursor hiddenCursorHandle;
        XContext context;
        XIM im;
        short int keycodes[256],scancodes[GLFW_KEY_LAST + 1];
        double restoreCursorPosX, restoreCursorPosY;
        _GLFWwindow* disabledCursorWindow;
        Atom NET_SUPPORTED,NET_SUPPORTING_WM_CHECK,WM_PROTOCOLS,WM_STATE,WM_DELETE_WINDOW,NET_WM_NAME,NET_WM_ICON,NET_WM_PID,NET_WM_PING,NET_WM_WINDOW_TYPE,NET_WM_WINDOW_TYPE_NORMAL,NET_WM_STATE,NET_WM_STATE_FULLSCREEN,NET_WM_BYPASS_COMPOSITOR,NET_WORKAREA,NET_CURRENT_DESKTOP,NET_ACTIVE_WINDOW,MOTIF_WM_HINTS,UTF8_STRING;
        struct {
            void*       handle;
            GLFWbool    utf8;
            PFN_XAllocClassHint AllocClassHint;
            PFN_XAllocSizeHints AllocSizeHints;
            PFN_XAllocWMHints AllocWMHints;
            PFN_XChangeProperty ChangeProperty;
            PFN_XChangeWindowAttributes ChangeWindowAttributes;
            PFN_XCheckTypedWindowEvent CheckTypedWindowEvent;
            PFN_XCloseDisplay CloseDisplay;
            PFN_XCreateColormap CreateColormap;
            PFN_XCreateIC CreateIC;
            PFN_XCreateWindow CreateWindow;
            PFN_XDefineCursor DefineCursor;
            PFN_XDeleteProperty DeleteProperty;
            PFN_XDisplayKeycodes DisplayKeycodes;
            PFN_XFilterEvent FilterEvent;
            PFN_XFindContext FindContext;
            PFN_XFlush Flush;
            PFN_XFree Free;
            PFN_XFreeEventData FreeEventData;
            PFN_XGetEventData GetEventData;
            PFN_XGetICValues GetICValues;
            PFN_XGetIMValues GetIMValues;
            PFN_XGetInputFocus GetInputFocus;
            PFN_XGetKeyboardMapping GetKeyboardMapping;
            PFN_XGetWMNormalHints GetWMNormalHints;
            PFN_XGetWindowAttributes GetWindowAttributes;
            PFN_XGetWindowProperty GetWindowProperty;
            PFN_XGrabPointer GrabPointer;
            PFN_XInternAtom InternAtom;
            PFN_XMapWindow MapWindow;
            PFN_XMoveResizeWindow MoveResizeWindow;
            PFN_XMoveWindow MoveWindow;
            PFN_XNextEvent NextEvent;
            PFN_XPending Pending;
            PFN_XQueryExtension QueryExtension;
            PFN_XQueryPointer QueryPointer;
            PFN_XRaiseWindow RaiseWindow;
            PFN_XRegisterIMInstantiateCallback RegisterIMInstantiateCallback;
            PFN_XResizeWindow ResizeWindow;
            PFN_XSaveContext SaveContext;
            PFN_XSelectInput SelectInput;
            PFN_XSendEvent SendEvent;
            PFN_XSetClassHint SetClassHint;
            PFN_XSetICFocus SetICFocus;
            PFN_XSetIMValues SetIMValues;
            PFN_XSetInputFocus SetInputFocus;
            PFN_XSetLocaleModifiers SetLocaleModifiers;
            PFN_XSetWMHints SetWMHints;
            PFN_XSetWMNormalHints SetWMNormalHints;
            PFN_XSetWMProtocols SetWMProtocols;
            PFN_XSupportsLocale SupportsLocale;
            PFN_XTranslateCoordinates TranslateCoordinates;
            PFN_XUndefineCursor UndefineCursor;
            PFN_XUngrabPointer UngrabPointer;
            PFN_XUnsetICFocus UnsetICFocus;
            PFN_XWarpPointer WarpPointer;
            PFN_Xutf8SetWMProperties utf8SetWMProperties;
        } xlib;
        struct { GLFWbool available; void* handle; int eventBase,errorBase,major,minor; GLFWbool monitorBroken; PFN_XRRFreeCrtcInfo FreeCrtcInfo; PFN_XRRFreeOutputInfo FreeOutputInfo; PFN_XRRFreeScreenResources FreeScreenResources; PFN_XRRGetCrtcInfo GetCrtcInfo; PFN_XRRGetOutputInfo GetOutputInfo; PFN_XRRGetOutputPrimary GetOutputPrimary; PFN_XRRGetScreenResourcesCurrent GetScreenResourcesCurrent; PFN_XRRQueryExtension QueryExtension; PFN_XRRQueryVersion QueryVersion; PFN_XRRSelectInput SelectInput; PFN_XRRSetCrtcConfig SetCrtcConfig; PFN_XRRUpdateConfiguration UpdateConfiguration; } randr;
        struct { GLFWbool available,detectable; int majorOpcode,eventBase,errorBase,major,minor; unsigned int group; PFN_XkbFreeKeyboard FreeKeyboard; PFN_XkbFreeNames FreeNames; PFN_XkbGetMap GetMap; PFN_XkbGetNames GetNames; PFN_XkbGetState GetState; PFN_XkbQueryExtension QueryExtension; PFN_XkbSelectEventDetails SelectEventDetails; PFN_XkbSetDetectableAutoRepeat SetDetectableAutoRepeat; } xkb;
        struct { void* handle; PFN_XcursorImageCreate ImageCreate; PFN_XcursorImageDestroy ImageDestroy; PFN_XcursorImageLoadCursor ImageLoadCursor; } xcursor;
        struct { GLFWbool available; void* handle; int majorOpcode,eventBase,errorBase,major,minor; PFN_XIQueryVersion QueryVersion; PFN_XISelectEvents SelectEvents; } xi;
    } _GLFWlibraryX11;
    typedef struct _GLFWmonitorX11 { RROutput output; RRCrtc crtc; int index; } _GLFWmonitorX11;
    typedef struct _GLFWcursorX11 { Cursor handle; } _GLFWcursorX11;
    GLFWbool _glfwWindowVisibleX11(_GLFWwindow* window);
    void _glfwSetWindowDecoratedX11(_GLFWwindow* window, GLFWbool enabled);
    void _glfwGetCursorPosX11(_GLFWwindow* window, double* xpos, double* ypos);
    void _glfwSetCursorPosX11(_GLFWwindow* window, double xpos, double ypos);
    GLFWbool _glfwGetVideoModeX11(_GLFWmonitor* monitor, GLFWvidmode* mode);
    void _glfwPollMonitorsX11(void);
    #define GLFW_LINUX_JOYSTICK_STATE _GLFWjoystickLinux linjs;
    #define GLFW_LINUX_LIBRARY_JOYSTICK_STATE _GLFWlibraryLinux  linjs;
    typedef struct _GLFWjoystickLinux { int fd; char path[4096]; int keyMap[KEY_CNT - BTN_MISC],absMap[ABS_CNT]; struct input_absinfo absInfo[ABS_CNT]; int hats[4][2]; } _GLFWjoystickLinux;
    typedef struct _GLFWlibraryLinux { int inotify,watch; regex_t regex; GLFWbool regexCompiled,dropped; } _GLFWlibraryLinux;
    void _glfwDetectJoystickConnectionLinux(void);
    #define GLFW_WIN32_JOYSTICK_STATE
    #define GLFW_WIN32_LIBRARY_JOYSTICK_STATE
    #define SIZE_MAX (~(size_t)0)
    #define GLFW_POSIX_TLS_STATE _GLFWtlsPOSIX   posix;
    #define GLFW_POSIX_MUTEX_STATE _GLFWmutexPOSIX posix;
    typedef struct _GLFWtlsPOSIX { GLFWbool allocated; pthread_key_t key; } _GLFWtlsPOSIX;
    typedef struct _GLFWmutexPOSIX { GLFWbool allocated; pthread_mutex_t handle; } _GLFWmutexPOSIX;
    #define GLFW_PLATFORM_TLS_STATE    GLFW_POSIX_TLS_STATE
    #define GLFW_PLATFORM_MUTEX_STATE  GLFW_POSIX_MUTEX_STATE
    #define GLFW_POSIX_LIBRARY_TIMER_STATE _GLFWtimerPOSIX posix;
    typedef struct _GLFWtimerPOSIX { clockid_t clock; u64 frequency; } _GLFWtimerPOSIX;
    #define GLFW_PLATFORM_LIBRARY_TIMER_STATE  GLFW_POSIX_LIBRARY_TIMER_STATE
#endif
#define GLFW_PLATFORM_WINDOW_STATE \
    GLFW_WIN32_WINDOW_STATE \
    GLFW_X11_WINDOW_STATE
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
struct _GLFWinitconfig { GLFWbool hatButtons; };
struct _GLFWctxconfig { int client,source,major,minor; _GLFWwindow* share; };
struct _GLFWfbconfig { int redBits,greenBits,blueBits,alphaBits,depthBits,stencilBits,accumRedBits,accumGreenBits,accumBlueBits,accumAlphaBits,auxBuffers; GLFWbool stereo; int samples; GLFWbool sRGB,doublebuffer; uintptr_t handle; };
struct _GLFWcontext {
    int client,source,major,minor;
    PFNGLGETSTRINGIPROC  GetStringi;
    PFNGLGETINTEGERVPROC GetIntegerv;
    PFNGLGETSTRINGPROC   GetString;
    void (*makeCurrent)(_GLFWwindow*);
    void (*swapBuffers)(_GLFWwindow*);
    void (*swapInterval)(int);
    int (*extensionSupported)(const char*);
    GLFWglproc (*getProcAddress)(const char*);
    GLFW_PLATFORM_CONTEXT_STATE
};

struct _GLFWwindow {
    struct _GLFWwindow* next;
    GLFWbool decorated,shouldClose,doublebuffer;
    GLFWvidmode videoMode;
    _GLFWcursor* cursor;
    int minwidth,minheight,maxwidth,maxheight;
    int cursorMode;
    char mouseButtons[GLFW_MOUSE_BUTTON_LAST + 1],keys[GLFW_KEY_LAST + 1];
    double virtualCursorPosX,virtualCursorPosY;
    GLFWbool rawMouseMotion;
    _GLFWcontext context;
    GLFW_PLATFORM_WINDOW_STATE
};

struct _GLFWmonitor { char name[128]; int widthMM,heightMM; GLFWvidmode* modes; int modeCount; GLFWvidmode currentMode; GLFW_WIN32_MONITOR_STATE GLFW_X11_MONITOR_STATE };
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
    char name[128],guid[33];
    _GLFWmapping* mapping;
    #if defined(LINUX)
        GLFW_LINUX_JOYSTICK_STATE
    #else
        GLFW_WIN32_JOYSTICK_STATE
    #endif
};

struct _GLFWtls { GLFW_PLATFORM_TLS_STATE };
struct _GLFWmutex { GLFW_PLATFORM_MUTEX_STATE };
struct _GLFWlibrary {
    struct { _GLFWinitconfig init; _GLFWfbconfig framebuffer; _GLFWctxconfig context; } hints;
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
    GLFW_PLATFORM_LIBRARY_WINDOW_STATE
    GLFW_PLATFORM_LIBRARY_CONTEXT_STATE
    GLFW_PLATFORM_LIBRARY_JOYSTICK_STATE
};

extern _GLFWlibrary _glfw;
void _glfwPlatformInitTimer(void);
u64 _glfwPlatformGetTimerValue(void);
u64 _glfwPlatformGetTimerFrequency(void);
GLFWbool _glfwPlatformCreateTls(_GLFWtls* tls);
void* _glfwPlatformGetTls(_GLFWtls* tls);
void _glfwPlatformSetTls(_GLFWtls* tls, void* value);
GLFWbool _glfwPlatformCreateMutex(_GLFWmutex* mutex);
void* _glfwPlatformLoadModule(const char* path);
GLFWproc _glfwPlatformGetModuleSymbol(void* module, const char* name);
void _glfwInputWindowFocus(_GLFWwindow* window, GLFWbool focused);
void _glfwInputKey(_GLFWwindow* window, int key, int action);
void _glfwInputMouseClick(_GLFWwindow* window, int button, int action);
void _glfwInputCursorPos(_GLFWwindow* window, double xpos, double ypos);
void _glfwInputJoystick(_GLFWjoystick* js, int event);
void _glfwInputJoystickAxis(_GLFWjoystick* js, int axis, float value);
void _glfwInputJoystickButton(_GLFWjoystick* js, int button, char value);
void _glfwInputJoystickHat(_GLFWjoystick* js, int hat, char value);
void _glfwInputMonitor(_GLFWmonitor* monitor, int action, int placement);
GLFWbool _glfwStringInExtensionString(const char* string, const char* extensions);
const _GLFWfbconfig* _glfwChooseFBConfig(const _GLFWfbconfig* desired, const _GLFWfbconfig* alternatives, unsigned int count);
GLFWbool _glfwRefreshContextAttribs(_GLFWwindow* window, const _GLFWctxconfig* ctxconfig);
int _glfwCompareVideoModes(const GLFWvidmode* first, const GLFWvidmode* second);
_GLFWmonitor* _glfwAllocMonitor(const char* name, int widthMM, int heightMM);
void _glfwSplitBPP(int bpp, int* red, int* green, int* blue);
void _glfwInitGamepadMappings(void);
_GLFWjoystick* _glfwAllocJoystick(const char* name, const char* guid, int axisCount, int buttonCount, int hatCount);
void _glfwFreeJoystick(_GLFWjoystick* js);
void* _glfw_calloc(size_t count, size_t size) {
    if (count && size) {
        void* block;
        if (count > SIZE_MAX / size) { DualLogError("Allocation size overflow"); return NULL; }

        block = malloc(count * size);
        return __builtin_memset(block,0,count * size);
    } else return NULL;
}

void* _glfw_realloc(void* block, size_t size) {
    if (block && size) {
        return realloc(block,size);
    } else if (block) { free(block); return NULL;
    } else return calloc(1,size);
}

#if defined(WINDOWS)
    #include <wchar.h>
    static DWORD getWindowStyle(const _GLFWwindow* window) { return (WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_SYSMENU | WS_MINIMIZEBOX) | (window->decorated ? WS_CAPTION : WS_POPUP); }
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
        if (!color) { DualLogError("Win32: Failed to create RGBA bitmap"); return NULL; }
        mask=CreateBitmap(image->width,image->height,1,1,NULL);
        if (!mask) { DualLogError("Win32: Failed to create mask bitmap"); DeleteObject(color); return NULL; }
        for (int i=0;i<image->width*image->height;i++) { target[0]=source[2]; target[1]=source[1]; target[2]=source[0]; target[3]=source[3]; target+=4; source+=4; }
        ZeroMemory(&ii,sizeof(ii));
        ii.fIcon=icon; ii.xHotspot=xhot; ii.yHotspot=yhot; ii.hbmMask=mask; ii.hbmColor=color;
        handle=CreateIconIndirect(&ii);
        DeleteObject(color); DeleteObject(mask);
        if (!handle) DualLogError(icon?"Win32: Failed to create icon":"Win32: Failed to create cursor");
        return handle;
    }

    static void updateCursorImage(_GLFWwindow* window) {
        if (window->cursorMode==GLFW_CURSOR_NORMAL) {
            if (window->cursor) SetCursor(window->cursor->win32.handle);
            else SetCursor(LoadCursorW(NULL,(LPCWSTR)IDC_ARROW));
        } else SetCursor(_glfw.win32.blankCursor);
    }

    static void captureCursor(_GLFWwindow* window) {
        RECT clipRect; GetClientRect(window->win32.handle,&clipRect);
        ClientToScreen(window->win32.handle,(POINT*)&clipRect.left);
        ClientToScreen(window->win32.handle,(POINT*)&clipRect.right);
        ClipCursor(&clipRect);
        _glfw.win32.capturedCursorWindow=window;
    }

    static void releaseCursor(void) { ClipCursor(NULL); _glfw.win32.capturedCursorWindow=NULL; }

    static void enableRawMouseMotion(_GLFWwindow* window) {
        const RAWINPUTDEVICE rid={0x01,0x02,0,window->win32.handle};
        if (!RegisterRawInputDevices(&rid,1,sizeof(rid))) DualLogError("Win32: Failed to register raw input device");
    }

    static void disableRawMouseMotion(_GLFWwindow* window) {
        const RAWINPUTDEVICE rid={0x01,0x02,RIDEV_REMOVE,NULL}; (void)window;
        if (!RegisterRawInputDevices(&rid,1,sizeof(rid))) DualLogError("Win32: Failed to remove raw input device");
    }

    static void disableCursor(_GLFWwindow* window) {
        _glfw.win32.disabledCursorWindow=window;
        _glfwGetCursorPosWin32(window,&_glfw.win32.restoreCursorPosX,&_glfw.win32.restoreCursorPosY);
        updateCursorImage(window);
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
        AdjustWindowRectEx(&rect,style,FALSE,WS_EX_APPWINDOW);
        ClientToScreen(window->win32.handle,(POINT*)&rect.left);
        ClientToScreen(window->win32.handle,(POINT*)&rect.right);
        SetWindowLongW(window->win32.handle,GWL_STYLE,style);
        SetWindowPos(window->win32.handle,HWND_TOP,rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top,SWP_FRAMECHANGED|SWP_NOACTIVATE|SWP_NOZORDER);
    }

    static LRESULT CALLBACK windowProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam) {
        _GLFWwindow* window=GetPropW(hWnd,L"GLFW"); if (!window) return DefWindowProcW(hWnd,uMsg,wParam,lParam);

        switch (uMsg) {
            case WM_MOUSEACTIVATE: {
                if (HIWORD(lParam)==WM_LBUTTONDOWN && LOWORD(lParam)!=HTCLIENT) window->win32.frameAction=GLFW_TRUE;
                break;
            }
            case WM_CAPTURECHANGED: {
                if (lParam==0&&window->win32.frameAction) {
                    if (window->cursorMode==GLFW_CURSOR_DISABLED) disableCursor(window);
                    window->win32.frameAction=GLFW_FALSE;
                }
                break;
            }
            case WM_SETFOCUS: {
                _glfwInputWindowFocus(window,GLFW_TRUE);
                if (window->win32.frameAction) break;
                if (window->cursorMode==GLFW_CURSOR_DISABLED) disableCursor(window);
                return 0;
            }
            case WM_KILLFOCUS: {
                if (window->cursorMode==GLFW_CURSOR_DISABLED) enableCursor(window);
                _glfwInputWindowFocus(window,GLFW_FALSE);
                return 0;
            }
            case WM_SYSCOMMAND: {
                switch (wParam&0xfff0) {
                    case SC_SCREENSAVE:
                    case SC_MONITORPOWER: break;
                    case SC_KEYMENU: if (!window->win32.keymenu) return 0; break;
                }
                break;
            }
            case WM_CLOSE: window->shouldClose = GLFW_TRUE; return 0;
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYUP: {
                int key,scancode;
                const int action=(HIWORD(lParam)&KF_UP)?GLFW_RELEASE:GLFW_PRESS;
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
                    _glfwInputKey(window,GLFW_KEY_LEFT_SHIFT,action);
                    _glfwInputKey(window,GLFW_KEY_RIGHT_SHIFT,action);
                } else if (wParam==VK_SNAPSHOT) {
                    _glfwInputKey(window,key,GLFW_PRESS);
                    _glfwInputKey(window,key,GLFW_RELEASE);
                } else _glfwInputKey(window,key,action);
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
                _glfwInputMouseClick(window,button,action);
                for (i=0;i<=GLFW_MOUSE_BUTTON_LAST;i++) { if (window->mouseButtons[i]==GLFW_PRESS) break; }
                if (i>GLFW_MOUSE_BUTTON_LAST) ReleaseCapture();
                if (uMsg==WM_XBUTTONDOWN||uMsg==WM_XBUTTONUP) return TRUE;
                return 0;
            }
            case WM_MOUSEMOVE: {                
                const int x=((int)(short)(lParam & 0xFFFF)), y=((int)(short)(lParam >> 16));
                if (!window->win32.cursorTracked) {
                    TRACKMOUSEEVENT tme; ZeroMemory(&tme,sizeof(tme));
                    tme.cbSize=sizeof(tme); tme.dwFlags=TME_LEAVE; tme.hwndTrack=window->win32.handle;
                    TrackMouseEvent(&tme);
                    window->win32.cursorTracked=GLFW_TRUE;
                }
                if (window->cursorMode==GLFW_CURSOR_DISABLED) {
                    const int dx=x-window->win32.lastCursorPosX,dy=y-window->win32.lastCursorPosY;
                    if (_glfw.win32.disabledCursorWindow!=window||window->rawMouseMotion) break;
                    _glfwInputCursorPos(window,window->virtualCursorPosX+dx,window->virtualCursorPosY+dy);
                }
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
                if (GetRawInputData(ri,RID_INPUT,_glfw.win32.rawInput,&size,sizeof(RAWINPUTHEADER))==(UINT)-1) { DualLogError("Win32: Failed to retrieve raw input data"); break; }
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
            case WM_MOUSELEAVE: { window->win32.cursorTracked=GLFW_FALSE; return 0; }
            case WM_MOUSEWHEEL: { Sys_Input.scrollDelta += (SHORT)HIWORD(wParam)/(double)WHEEL_DELTA; return 0; }
            case WM_MOUSEHWHEEL: return 0;
            case WM_ENTERSIZEMOVE:
            case WM_ENTERMENULOOP: {
                if (window->win32.frameAction) break;
                if (window->cursorMode==GLFW_CURSOR_DISABLED) enableCursor(window);
                break;
            }
            case WM_EXITSIZEMOVE:
            case WM_EXITMENULOOP: {
                if (window->win32.frameAction) break;
                if (window->cursorMode==GLFW_CURSOR_DISABLED) disableCursor(window);
                break;
            }
            case WM_SIZE: if (wParam==SIZE_MINIMIZED) {Sys_Global.gamePaused = true;} return 0;
            case WM_MOVE: if (_glfw.win32.capturedCursorWindow==window) {captureCursor(window);} return 0;
            case WM_GETMINMAXINFO: {
                RECT frame={0}; MINMAXINFO* mmi=(MINMAXINFO*)lParam;
                const DWORD style=getWindowStyle(window);
                AdjustWindowRectEx(&frame,style,FALSE,WS_EX_APPWINDOW);
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
            case WM_ERASEBKGND: return TRUE;
            case WM_NCACTIVATE:
            case WM_NCPAINT: { if (!window->decorated) return TRUE; break; }
            case WM_DWMCOMPOSITIONCHANGED:
            case WM_DWMCOLORIZATIONCOLORCHANGED: return 0;
            case WM_SETCURSOR: { if (LOWORD(lParam)==HTCLIENT) { updateCursorImage(window); return TRUE; } break; }
        }
        return DefWindowProcW(hWnd,uMsg,wParam,lParam);
    }

    static int createNativeWindow(_GLFWwindow* window, char* title, int width, int height) {
        DWORD style=getWindowStyle(window);
        if (!_glfw.win32.mainWindowClass) {
            WNDCLASSEXW wc={0}; wc.cbSize=sizeof(wc),wc.style=CS_HREDRAW|CS_VREDRAW|CS_OWNDC,wc.lpfnWndProc=windowProc,wc.hInstance=_glfw.win32.instance,wc.lpszClassName=L"Voxen",wc.hIcon=wc.hCursor=NULL;
            if (!(_glfw.win32.mainWindowClass=RegisterClassExW(&wc))) { DualLogError("Win32: Failed to register window class"); return GLFW_FALSE; }
        }

        RECT rect={0,0,width,height};
        AdjustWindowRectEx(&rect,style,FALSE,WS_EX_APPWINDOW);
        int frameX=CW_USEDEFAULT, frameY=CW_USEDEFAULT;
        int frameWidth=rect.right-rect.left, frameHeight=rect.bottom-rect.top;
        WCHAR* wideTitle=_glfwCreateWideStringFromUTF8Win32(title); if (!wideTitle) return GLFW_FALSE;
        window->win32.handle=CreateWindowExW(WS_EX_APPWINDOW,(LPCWSTR)MAKEINTATOM(_glfw.win32.mainWindowClass),wideTitle,style,frameX,frameY,frameWidth,frameHeight,NULL,NULL,_glfw.win32.instance,(LPVOID)NULL),free(wideTitle);
        if (!window->win32.handle) { DualLogError("Win32: Failed to create window"); return GLFW_FALSE; }
        SetPropW(window->win32.handle,L"GLFW",window);
        window->win32.keymenu=GLFW_FALSE; WINDOWPLACEMENT wp={0}; wp.length=sizeof(wp); AdjustWindowRectEx(&rect,style,FALSE,WS_EX_APPWINDOW);
        GetWindowPlacement(window->win32.handle,&wp), OffsetRect(&rect,wp.rcNormalPosition.left-rect.left,wp.rcNormalPosition.top-rect.top);
        wp.rcNormalPosition=rect, wp.showCmd=SW_HIDE, SetWindowPlacement(window->win32.handle,&wp);
        _glfwGetWindowSizeWin32(window,&window->win32.width,&window->win32.height); return GLFW_TRUE;
    }
    
    void _glfwSetWindowIconWin32(_GLFWwindow* window, int count, const GLFWimage* image) {
        HICON hIcon = createIcon(image,0,0,GLFW_TRUE); (void)count;
        SendMessageW(window->win32.handle, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        SendMessageW(window->win32.handle, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    }

    void _glfwGetWindowPosWin32(_GLFWwindow* window,int* xpos,int* ypos) {
        POINT pos={0,0}; ClientToScreen(window->win32.handle,&pos);
        if (xpos) *xpos=pos.x; if (ypos) *ypos=pos.y;
    }

    void _glfwSetWindowPosWin32(_GLFWwindow* window,int xpos,int ypos) {
        RECT rect={xpos,ypos,xpos,ypos};
        AdjustWindowRectEx(&rect,getWindowStyle(window),FALSE,WS_EX_APPWINDOW);
        SetWindowPos(window->win32.handle,NULL,rect.left,rect.top,0,0,SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOSIZE);
    }

    void _glfwGetWindowSizeWin32(_GLFWwindow* window,int* width,int* height) {
        RECT area; GetClientRect(window->win32.handle,&area);
        if (width) *width=area.right; if (height) *height=area.bottom;
    }

    void _glfwSetWindowSizeWin32(_GLFWwindow* window,int width,int height) {
        RECT rect={0,0,width,height};
        AdjustWindowRectEx(&rect,getWindowStyle(window),FALSE,WS_EX_APPWINDOW);
        SetWindowPos(window->win32.handle,HWND_TOP,0,0,rect.right-rect.left,rect.bottom-rect.top,SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOMOVE|SWP_NOZORDER);
    }

    void _glfwShowWindowWin32(_GLFWwindow* window) { int showCommand = SW_SHOWNA; ShowWindow(window->win32.handle, showCommand); }
    void _glfwSetWindowMonitorWin32(_GLFWwindow* window,int xpos,int ypos,int width,int height) {
        RECT r = {xpos,ypos,xpos+width,ypos+height}; DWORD s = GetWindowLongW(window->win32.handle,GWL_STYLE); UINT f = SWP_NOACTIVATE|SWP_NOCOPYBITS;
        if (window->decorated) { s &= ~WS_POPUP, s |= getWindowStyle(window), SetWindowLongW(window->win32.handle,GWL_STYLE,s), f |= SWP_FRAMECHANGED; }
        AdjustWindowRectEx(&r,getWindowStyle(window),FALSE,WS_EX_APPWINDOW);
        SetWindowPos(window->win32.handle,(HWND)HWND_NOTOPMOST,r.left,r.top,r.right-r.left,r.bottom-r.top,f);
    }

    void _glfwSetWindowDecoratedWin32(_GLFWwindow* window,GLFWbool enabled) { (void)enabled; updateWindowStyles(window); }
    void _glfwSetRawMouseMotionWin32(_GLFWwindow* window,GLFWbool enabled) {
        if (_glfw.win32.disabledCursorWindow!=window) return;
        if (enabled) enableRawMouseMotion(window); else disableRawMouseMotion(window);
    }

    void _glfwPollEventsWin32(void) {
        MSG msg; HWND handle; _GLFWwindow* window;
        while (PeekMessageW(&msg,NULL,0,0,PM_REMOVE)) {
            if (msg.message==WM_QUIT) { window=_glfw.windowListHead; while (window) { window->shouldClose = GLFW_TRUE; window=window->next; } }
            else { TranslateMessage(&msg); DispatchMessageW(&msg); }
        }
        handle=GetActiveWindow();
        if (handle) {
            window=GetPropW(handle,L"GLFW");
            if (window) {
                int i;
                const int keys[4][2]={{VK_LSHIFT,GLFW_KEY_LEFT_SHIFT},{VK_RSHIFT,GLFW_KEY_RIGHT_SHIFT},{VK_LWIN,GLFW_KEY_LEFT_SUPER},{VK_RWIN,GLFW_KEY_RIGHT_SUPER}};
                for (i=0;i<4;i++) {
                    const int vk=keys[i][0],key=keys[i][1];
                    if ((GetKeyState(vk)&0x8000)||window->keys[key]!=GLFW_PRESS) continue;
                    _glfwInputKey(window,key,GLFW_RELEASE);
                }
            }
        }
        window=_glfw.win32.disabledCursorWindow;
        if (window) {
            int width,height; _glfwGetWindowSizeWin32(window,&width,&height);
            if (window->win32.lastCursorPosX!=width/2||window->win32.lastCursorPosY!=height/2) _glfwSetCursorPosWin32(window,width/2,height/2);
        }
    }

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
        if (window->win32.handle==GetActiveWindow()) {
            if (mode==GLFW_CURSOR_DISABLED) { _glfwGetCursorPosWin32(window,&_glfw.win32.restoreCursorPosX,&_glfw.win32.restoreCursorPosY); if (window->rawMouseMotion) enableRawMouseMotion(window); }
            else if (_glfw.win32.disabledCursorWindow==window) { if (window->rawMouseMotion) disableRawMouseMotion(window); }
            if (mode==GLFW_CURSOR_DISABLED) captureCursor(window); else releaseCursor();
            if (mode==GLFW_CURSOR_DISABLED) _glfw.win32.disabledCursorWindow=window;
            else if (_glfw.win32.disabledCursorWindow==window) { _glfw.win32.disabledCursorWindow=NULL; _glfwSetCursorPosWin32(window,_glfw.win32.restoreCursorPosX,_glfw.win32.restoreCursorPosY); }
        }
        if (cursorInContentArea(window)) updateCursorImage(window);
    }

    void _glfwDestroyCursorWin32(_GLFWcursor* cursor) { if (cursor->win32.handle) DestroyIcon((HICON)cursor->win32.handle); }
    static const GUID _glfw_GUID_DEVINTERFACE_HID = {0x4d1e55b2,0xf16f,0x11cf,{0x88,0xcb,0x00,0x11,0x11,0x00,0x00,0x30}};
    GLFWproc _glfwPlatformGetModuleSymbol(void* module, const char* name) { return (GLFWproc)GetProcAddress((HMODULE)module,name); }
    static GLFWbool loadLibraries(void) {
        if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(const WCHAR*) &_glfw,(HMODULE*) &_glfw.win32.instance)){ DualLogError("Win32: Failed to retrieve own module handle"); return GLFW_FALSE;}

        _glfw.win32.user32.instance = LoadLibraryA("user32.dll");
        if (!_glfw.win32.user32.instance) { DualLogError("Win32: Failed to load user32.dll"); return GLFW_FALSE; }

        _glfw.win32.dinput8.instance = LoadLibraryA("dinput8.dll");
        if (_glfw.win32.dinput8.instance) _glfw.win32.dinput8.Create = (PFN_DirectInput8Create)_glfwPlatformGetModuleSymbol(_glfw.win32.dinput8.instance, "DirectInput8Create");
        const char* names[] = {"xinput1_4.dll","xinput1_3.dll","xinput9_1_0.dll","xinput1_2.dll","xinput1_1.dll",NULL};
        for (int i=0;names[i];++i) {
            _glfw.win32.xinput.instance = LoadLibraryA(names[i]);
            if (_glfw.win32.xinput.instance) {
                _glfw.win32.xinput.GetCapabilities = (PFN_XInputGetCapabilities)_glfwPlatformGetModuleSymbol(_glfw.win32.xinput.instance, "XInputGetCapabilities");
                _glfw.win32.xinput.GetState = (PFN_XInputGetState)_glfwPlatformGetModuleSymbol(_glfw.win32.xinput.instance, "XInputGetState");
                break;
            }
        }

        _glfw.win32.dwmapi.instance = LoadLibraryA("dwmapi.dll");
        if (_glfw.win32.dwmapi.instance) {
            _glfw.win32.dwmapi.IsCompositionEnabled = (PFN_DwmIsCompositionEnabled)_glfwPlatformGetModuleSymbol(_glfw.win32.dwmapi.instance, "DwmIsCompositionEnabled");
            _glfw.win32.dwmapi.Flush = (PFN_DwmFlush)_glfwPlatformGetModuleSymbol(_glfw.win32.dwmapi.instance, "DwmFlush");
        }

        _glfw.win32.ntdll.instance = LoadLibraryA("ntdll.dll");
        if (_glfw.win32.ntdll.instance) _glfw.win32.ntdll.RtlVerifyVersionInfo_ = (PFN_RtlVerifyVersionInfo)_glfwPlatformGetModuleSymbol(_glfw.win32.ntdll.instance, "RtlVerifyVersionInfo");
        return GLFW_TRUE;
    }

    static void createKeyTables(void) {
        int scancode;
        __builtin_memset(_glfw.win32.keycodes, -1, sizeof(_glfw.win32.keycodes));
        __builtin_memset(_glfw.win32.scancodes, -1, sizeof(_glfw.win32.scancodes));
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
        if (!_glfw.win32.helperWindowClass) { DualLogError("Win32: Failed to register helper window class"); return GLFW_FALSE; }

        _glfw.win32.helperWindowHandle = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW,(LPCWSTR)MAKEINTATOM(_glfw.win32.helperWindowClass),L"GLFW message window",WS_CLIPSIBLINGS|WS_CLIPCHILDREN,0,0,1,1,NULL,NULL,_glfw.win32.instance,NULL);
        if (!_glfw.win32.helperWindowHandle) { DualLogError("Win32: Failed to create helper window"); return GLFW_FALSE; }

        ShowWindow(_glfw.win32.helperWindowHandle, SW_HIDE);
        {
            DEV_BROADCAST_DEVICEINTERFACE_W dbi;
            ZeroMemory(&dbi, sizeof(dbi));
            dbi.dbcc_size = sizeof(dbi);
            dbi.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
            dbi.dbcc_classguid = _glfw_GUID_DEVINTERFACE_HID;
            _glfw.win32.deviceNotificationHandle = RegisterDeviceNotificationW(_glfw.win32.helperWindowHandle,(DEV_BROADCAST_HDR*)&dbi,DEVICE_NOTIFY_WINDOW_HANDLE);
        }

        while (PeekMessageW(&msg, _glfw.win32.helperWindowHandle, 0, 0, PM_REMOVE)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
        return GLFW_TRUE;
    }

    WCHAR* _glfwCreateWideStringFromUTF8Win32(const char* source) {
        WCHAR* target; int count = MultiByteToWideChar(CP_UTF8,0,source,-1,NULL,0); if (!count) { DualLogError("Win32: Failed to convert string from UTF-8"); return NULL; }
        target = _glfw_calloc(count, sizeof(WCHAR)); if (!MultiByteToWideChar(CP_UTF8, 0, source, -1, target, count)) { DualLogError("Win32: Failed to convert string from UTF-8"); free(target); return NULL; }
        return target;
    }

    char* _glfwCreateUTF8FromWideStringWin32(const WCHAR* source) {
        int size = WideCharToMultiByte(CP_UTF8,0,source, -1,NULL,0,NULL,NULL); if (!size) { DualLogError("Win32: Failed to convert string to UTF-8"); return NULL; }
        char* target = _glfw_calloc(size, 1); if (!WideCharToMultiByte(CP_UTF8, 0, source, -1, target, size, NULL, NULL)) { DualLogError("Win32: Failed to convert string to UTF-8"); free(target); return NULL; }
        return target;
    }

    BOOL _glfwIsWindowsVersionOrGreaterWin32(WORD major,WORD minor,WORD sp) {
        OSVERSIONINFOEXW osvi={0}; osvi.dwOSVersionInfoSize=sizeof(osvi), osvi.dwMajorVersion=major, osvi.dwMinorVersion=minor, osvi.wServicePackMajor=sp;
        DWORD mask=VER_MAJORVERSION|VER_MINORVERSION|VER_SERVICEPACKMAJOR;
        ULONGLONG cond=VerSetConditionMask(VerSetConditionMask(VerSetConditionMask(0,VER_MAJORVERSION,VER_GREATER_EQUAL),VER_MINORVERSION,VER_GREATER_EQUAL),VER_SERVICEPACKMAJOR,VER_GREATER_EQUAL);
        return RtlVerifyVersionInfo(&osvi,mask,cond)==0;
    }

    int _glfwInitWin32(void) {
        if (!loadLibraries()) return GLFW_FALSE;
        createKeyTables();
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
                if (xic->Flags & 0x0002/*Xinput caps wireless*/) return "Wireless Xbox Controller";
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

            __builtin_memset(name, 0, sizeof(name)); size = sizeof(name);
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
        free(js->win32.objects); _glfwFreeJoystick(js);
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
        
        __builtin_memset(&dc,0,sizeof(dc)), dc.dwSize=sizeof(dc);
        if (FAILED(IDirectInputDevice8_GetCapabilities(device,&dc))) { IDirectInputDevice8_Release(device); return DIENUM_CONTINUE; }
        
        __builtin_memset(&dipd,0,sizeof(dipd)), dipd.diph.dwSize=sizeof(dipd), dipd.diph.dwHeaderSize=sizeof(dipd.diph), dipd.diph.dwHow=DIPH_DEVICE, dipd.dwData=DIPROPAXISMODE_ABS;
        if (FAILED(IDirectInputDevice8_SetProperty(device,DIPROP_AXISMODE,&dipd.diph))) { IDirectInputDevice8_Release(device); return DIENUM_CONTINUE; }
        
        __builtin_memset(&data,0,sizeof(data)), data.device=device, data.objects=_glfw_calloc(dc.dwAxes+(size_t)dc.dwButtons+dc.dwPOVs,sizeof(_GLFWjoyobjectWin32));
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
                _glfwInputJoystick(js,GLFW_CONNECTED);
            }
        }

        if (_glfw.win32.dinput8.api) {
            if (FAILED(IDirectInput8_EnumDevices(_glfw.win32.dinput8.api,DI8DEVCLASS_GAMECTRL,deviceCallback,NULL,DIEDFL_ALLDEVICES))) {DualLogError("Failed to enumerate DirectInput8 devices"); return; }
        }
    }

    GLFWbool _glfwInitJoysticksWin32(void) {
        if (_glfw.win32.dinput8.instance) {
            if (FAILED(DirectInput8Create(_glfw.win32.instance,0x0800,&IID_IDirectInput8W,(void**) &_glfw.win32.dinput8.api,NULL))) { DualLogError("Win32: Failed to create interface"); return GLFW_FALSE; }
        }

        _glfwDetectJoystickConnectionWin32();
        return GLFW_TRUE;
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
            if (mode == 0/*presence*/) return GLFW_TRUE;

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

            if (mode == 0/*presence*/) return GLFW_TRUE;

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
            if (js->connected) _glfwPollJoystickWin32(js,0/*presence*/);
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
        if (GetMonitorInfoW(handle, (MONITORINFO*) &mi)) {
            _GLFWmonitor* monitor = (_GLFWmonitor*) data;
            if (wcscmp(mi.szDevice, monitor->win32.adapterName) == 0) monitor->win32.handle = handle;
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
            WideCharToMultiByte(CP_UTF8,0,display->DeviceName,-1,monitor->win32.publicDisplayName,sizeof(monitor->win32.publicDisplayName),NULL,NULL);
        }

        rect.left=dm.dmPosition.x; rect.top=dm.dmPosition.y; rect.right=dm.dmPosition.x + dm.dmPelsWidth; rect.bottom=dm.dmPosition.y + dm.dmPelsHeight;
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

        for (adapterIndex = 0;;adapterIndex++) {
            int type = 1; ZeroMemory(&adapter, sizeof(adapter));
            adapter.cb = sizeof(adapter);
            if (!EnumDisplayDevicesW(NULL, adapterIndex, &adapter, 0)) break;
            if (!(adapter.StateFlags & DISPLAY_DEVICE_ACTIVE)) continue;

            if (adapter.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) type = 0;
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

                _glfwInputMonitor(monitor,GLFW_CONNECTED,type); type = 1;
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
        if (!EnumDisplaySettingsW(monitor->win32.adapterName, ENUM_CURRENT_SETTINGS, &dm)) { DualLogError("Win32: Failed to query display settings"); return GLFW_FALSE; }

        mode->width  = dm.dmPelsWidth; mode->height = dm.dmPelsHeight;
        mode->refreshRate = dm.dmDisplayFrequency;
        _glfwSplitBPP(dm.dmBitsPerPel,&mode->redBits,&mode->greenBits,&mode->blueBits);
        return GLFW_TRUE;
    }

    void _glfwPlatformInitTimer(void) { QueryPerformanceFrequency((LARGE_INTEGER*) &_glfw.timer.win32.frequency); }
    u64 _glfwPlatformGetTimerValue(void) { u64 value; QueryPerformanceCounter((LARGE_INTEGER*)&value); return value; }
    u64 _glfwPlatformGetTimerFrequency(void) { return _glfw.timer.win32.frequency; }
    GLFWbool _glfwPlatformCreateTls(_GLFWtls* tls) {
        tls->win32.index = TlsAlloc();
        if (tls->win32.index == TLS_OUT_OF_INDEXES) { DualLogError("Win32: Failed to allocate TLS index"); return GLFW_FALSE; }

        tls->win32.allocated = GLFW_TRUE;
        return GLFW_TRUE;
    }

    void* _glfwPlatformGetTls(_GLFWtls* tls) { return TlsGetValue(tls->win32.index); }
    void _glfwPlatformSetTls(_GLFWtls* tls, void* value) { TlsSetValue(tls->win32.index,value); }
    GLFWbool _glfwPlatformCreateMutex(_GLFWmutex* mutex) { InitializeCriticalSection(&mutex->win32.section); return mutex->win32.allocated = GLFW_TRUE; }
    static int choosePixelFormatWGL(_GLFWwindow* window, const _GLFWctxconfig* ctxconfig, const _GLFWfbconfig* fbconfig) {
        (void)ctxconfig;
        int attribs[24],values[24],attribCount=0,i,pixelFormat,nativeCount,usableCount=0;
        const int query = 0x2000/*num pixel formats*/;
        if (!wglGetPixelFormatAttribivARB(window->context.wgl.dc,1,0,1,&query,&nativeCount)) { DualLogError("WGL: Failed to retrieve number of pixel formats"); return 0; }

        attribs[attribCount++] = 0x2010/*support opengl*/; attribs[attribCount++] = 0x2001/*draw to window*/; attribs[attribCount++] = 0x2013/*pixel type*/; attribs[attribCount++] = 0x2003/*accelaration*/;
        attribs[attribCount++] = 0x2011/*double buffer*/; attribs[attribCount++] = 0x2015/*r bits*/; attribs[attribCount++] = 0x2017/*g bits*/;
        attribs[attribCount++] = 0x2019/*b bits*/; attribs[attribCount++] = 0x201b/*a bits*/; attribs[attribCount++] = 0x2022/*depth bits*/; attribs[attribCount++] = 0x2023/*stencil bits*/;
        _GLFWfbconfig* usableConfigs = _glfw_calloc(nativeCount, sizeof(_GLFWfbconfig));
        for (i = 0; i < nativeCount; i++) {
            _GLFWfbconfig* u = usableConfigs + usableCount; pixelFormat = i + 1;
            if (!wglGetPixelFormatAttribivARB(window->context.wgl.dc,pixelFormat,0,attribCount,attribs,values)) { DualLogError("WGL: Failed to retrieve pixel format attributes"); free(usableConfigs); return 0; }
            if (values[0] == 0 || values[1] == 0/* support OpenGL + draw to window */ || values[2] != 0x202b/*type rgba*/ || values[3] == 0x2025/*no accel*/ || values[4] != fbconfig->doublebuffer) continue;
            
            u->redBits=values[5]; u->greenBits=values[6]; u->blueBits=values[7]; u->alphaBits=values[8]; u->depthBits=values[9]; u->stencilBits=values[10]; u->handle=pixelFormat; usableCount++;
        }

        const _GLFWfbconfig* closest = _glfwChooseFBConfig(fbconfig, usableConfigs, usableCount);
        if (!closest) { DualLogError("WGL: Failed to find a suitable pixel format"); free(usableConfigs); return 0; }

        pixelFormat = (int)closest->handle; free(usableConfigs);
        return pixelFormat;
    }

    static void makeContextCurrentWGL(_GLFWwindow* window) {
        if (window) {
            if (wglMakeCurrent(window->context.wgl.dc,window->context.wgl.handle)) _glfwPlatformSetTls(&_glfw.contextSlot,window);
            else { DualLogError("WGL: Failed to make context current"); _glfwPlatformSetTls(&_glfw.contextSlot,NULL); }
        } else { if (!wglMakeCurrent(NULL,NULL)) { DualLogError("WGL: Failed to clear current context"); } _glfwPlatformSetTls(&_glfw.contextSlot,NULL); }
    }

    static void swapBuffersWGL(_GLFWwindow* window) {
        if (!IsWindows8OrGreater()) {
            BOOL enabled = FALSE;
            if (SUCCEEDED(DwmIsCompositionEnabled(&enabled)) && enabled) {
                int count = abs(window->context.wgl.interval);
                while (count--) DwmFlush();
            }
        }

        SwapBuffers(window->context.wgl.dc);
    }

    static void swapIntervalWGL(int interval) {
        _GLFWwindow* window = _glfwPlatformGetTls(&_glfw.contextSlot);
        window->context.wgl.interval = interval;
        if (!IsWindows8OrGreater()) {
            BOOL enabled = FALSE;
            if (SUCCEEDED(DwmIsCompositionEnabled(&enabled)) && enabled) interval = 0;
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

    static GLFWglproc getProcAddressWGL(const char* procname) { const GLFWglproc proc = (GLFWglproc)wglGetProcAddress(procname); if (proc) {return proc;} return (GLFWglproc)_glfwPlatformGetModuleSymbol(_glfw.wgl.instance,procname); }
    GLFWbool _glfwInitWGL(void) {
        PIXELFORMATDESCRIPTOR pfd; HGLRC prc,rc; HDC pdc,dc;
        _glfw.wgl.instance = LoadLibraryA("opengl32.dll"); if (!_glfw.wgl.instance) { DualLogError("WGL: Failed to load opengl32.dll"); return GLFW_FALSE; }
        _glfw.wgl.CreateContext = (PFN_wglCreateContext)_glfwPlatformGetModuleSymbol(_glfw.wgl.instance, "wglCreateContext");
        _glfw.wgl.DeleteContext = (PFN_wglDeleteContext)_glfwPlatformGetModuleSymbol(_glfw.wgl.instance, "wglDeleteContext");
        _glfw.wgl.GetProcAddress = (PFN_wglGetProcAddress)_glfwPlatformGetModuleSymbol(_glfw.wgl.instance, "wglGetProcAddress");
        _glfw.wgl.GetCurrentDC = (PFN_wglGetCurrentDC)_glfwPlatformGetModuleSymbol(_glfw.wgl.instance, "wglGetCurrentDC");
        _glfw.wgl.GetCurrentContext = (PFN_wglGetCurrentContext)_glfwPlatformGetModuleSymbol(_glfw.wgl.instance, "wglGetCurrentContext");
        _glfw.wgl.MakeCurrent = (PFN_wglMakeCurrent)_glfwPlatformGetModuleSymbol(_glfw.wgl.instance, "wglMakeCurrent");
        _glfw.wgl.ShareLists = (PFN_wglShareLists)_glfwPlatformGetModuleSymbol(_glfw.wgl.instance, "wglShareLists");
        dc = GetDC(_glfw.win32.helperWindowHandle);
        ZeroMemory(&pfd,sizeof(pfd)); pfd.nSize = sizeof(pfd); pfd.nVersion = 1; pfd.dwFlags = PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER; pfd.iPixelType = PFD_TYPE_RGBA; pfd.cColorBits = 24;
        if (!SetPixelFormat(dc, ChoosePixelFormat(dc, &pfd), &pfd)) { DualLogError("WGL: Failed to set pixel format for dummy context"); return GLFW_FALSE; }
        rc = wglCreateContext(dc); if (!rc) { DualLogError("WGL: Failed to create dummy context"); return GLFW_FALSE; }
        pdc=wglGetCurrentDC(); prc=wglGetCurrentContext(); if (!wglMakeCurrent(dc, rc)) { DualLogError("WGL: Failed to make dummy context current"); wglMakeCurrent(pdc, prc); wglDeleteContext(rc); return GLFW_FALSE; }
        _glfw.wgl.GetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");
        _glfw.wgl.GetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
        _glfw.wgl.CreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
        _glfw.wgl.SwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
        _glfw.wgl.GetPixelFormatAttribivARB = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)wglGetProcAddress("wglGetPixelFormatAttribivARB");
        _glfw.wgl.ARB_create_context = extensionSupportedWGL("WGL_ARB_create_context");
        _glfw.wgl.ARB_create_context_profile = extensionSupportedWGL("WGL_ARB_create_context_profile");
        _glfw.wgl.EXT_swap_control = extensionSupportedWGL("WGL_EXT_swap_control");
        wglMakeCurrent(pdc,prc);
        wglDeleteContext(rc);
        return GLFW_TRUE;
    }

    GLFWbool _glfwCreateContextWGL(_GLFWwindow* window, const _GLFWctxconfig* ctxconfig, const _GLFWfbconfig* fbconfig) {
        int attribs[40],pixelFormat; PIXELFORMATDESCRIPTOR pfd; HGLRC share = NULL;
        if (ctxconfig->share) share = ctxconfig->share->context.wgl.handle;
        window->context.wgl.dc = GetDC(window->win32.handle);
        if (!window->context.wgl.dc) { DualLogError("WGL: Failed to retrieve DC for window"); return GLFW_FALSE; }

        pixelFormat = choosePixelFormatWGL(window, ctxconfig, fbconfig);
        if (!pixelFormat) return GLFW_FALSE;
        if (!DescribePixelFormat(window->context.wgl.dc,pixelFormat,sizeof(pfd),&pfd)) { DualLogError("WGL: Failed to retrieve PFD for selected pixel format"); return GLFW_FALSE; }
        if (!SetPixelFormat(window->context.wgl.dc, pixelFormat, &pfd)) { DualLogError("WGL: Failed to set selected pixel format"); return GLFW_FALSE; }

        if (_glfw.wgl.ARB_create_context) {
            int index = 0, mask = 0, flags = 0;
            mask |= WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
            attribs[index++] = 0x2091/*major*/; attribs[index++] = 4; // OpenGL 4.3
            attribs[index++] = 0x2092/*minor*/; attribs[index++] = 3; 
            if (flags) { attribs[index++] = 0x2094/*flags*/; attribs[index++] = flags; }
            if (mask) { attribs[index++] = 0x9126/*context profile mask*/; attribs[index++] = mask; }
            attribs[index++] = 0; attribs[index++] = 0;
            window->context.wgl.handle = wglCreateContextAttribsARB(window->context.wgl.dc, share, attribs);
            if (!window->context.wgl.handle) {
                const DWORD error = GetLastError();
                if (error == (0xc0070000 | 0x2095)) DualLogError("WGL: Driver does not support OpenGL version 4.3");
                else if (error == (0xc0070000 | 0x2096)) DualLogError("WGL: Driver does not support the requested OpenGL profile");
                else if (error == (0xc0070000 | 0x2054)) DualLogError("WGL: The share context is not compatible with the requested context");
                else DualLogError("WGL: Failed to create OpenGL context");
                return GLFW_FALSE;
            }
        } else {
            window->context.wgl.handle = wglCreateContext(window->context.wgl.dc);
            if (!window->context.wgl.handle) { DualLogError("WGL: Failed to create OpenGL context"); return GLFW_FALSE; }

            if (share) {
                if (!wglShareLists(share, window->context.wgl.handle)) { DualLogError("WGL: Failed to enable sharing with specified OpenGL context"); return GLFW_FALSE; }
            }
        }

        window->context.makeCurrent = makeContextCurrentWGL;
        window->context.swapBuffers = swapBuffersWGL;
        window->context.swapInterval = swapIntervalWGL;
        window->context.extensionSupported = extensionSupportedWGL;
        window->context.getProcAddress = getProcAddressWGL;
        return GLFW_TRUE;
    }
    
    GLFWbool _glfwCreateWindowWin32(_GLFWwindow* window, char* title, int width, int height,const _GLFWctxconfig* ctxconfig,const _GLFWfbconfig* fbconfig) {
        if (!createNativeWindow(window,title,width,height)) return GLFW_FALSE;
        if (!_glfwInitWGL()) return GLFW_FALSE;
        if (!_glfwCreateContextWGL(window,ctxconfig,fbconfig)) return GLFW_FALSE;
        if (!_glfwRefreshContextAttribs(window,ctxconfig)) return GLFW_FALSE;
        _glfwShowWindowWin32(window); BringWindowToTop(window->win32.handle); SetForegroundWindow(window->win32.handle); SetFocus(window->win32.handle);
        return GLFW_TRUE;
    }

    int _glfwGetKeyScancodeWin32(int key) { return _glfw.win32.scancodes[key]; }
    #define PLATFORM_getCursorPos(w,x,y)            _glfwGetCursorPosWin32(w,x,y)
    #define PLATFORM_setCursorPos(w,x,y)            _glfwSetCursorPosWin32(w,x,y)
    #define PLATFORM_setCursorMode(w,m)             _glfwSetCursorModeWin32(w,m)
    #define PLATFORM_setRawMouseMotion(w,e)         _glfwSetRawMouseMotionWin32(w,e)
    #define PLATFORM_initJoysticks()                _glfwInitJoysticksWin32()
    #define PLATFORM_pollJoystick(js,m)             _glfwPollJoystickWin32(js,m)
    #define PLATFORM_getMappingName()               _glfwGetMappingNameWin32()
    #define PLATFORM_updateGamepadGUID(g)           _glfwUpdateGamepadGUIDWin32(g)
    #define PLATFORM_getMonitorPos(m,x,y)           _glfwGetMonitorPosWin32(m,x,y)
    #define PLATFORM_getMonitorWorkarea(m,x,y,w,h)  _glfwGetMonitorWorkareaWin32(m,x,y,w,h)
    #define PLATFORM_getVideoModes(m,c)             _glfwGetVideoModesWin32(m,c)
    #define PLATFORM_getVideoMode(m,cur)            _glfwGetVideoModeWin32(m,cur)
    #define PLATFORM_setWindowIcon(w,c,i)           _glfwSetWindowIconWin32(w,c,i)
    #define PLATFORM_getWindowPos(w,x,y)            _glfwGetWindowPosWin32(w,x,y)
    #define PLATFORM_setWindowPos(w,x,y)            _glfwSetWindowPosWin32(w,x,y)
    #define PLATFORM_getWindowSize(w,wi,h)          _glfwGetWindowSizeWin32(w,wi,h)
    #define PLATFORM_setWindowSize(w,wi,h)          _glfwSetWindowSizeWin32(w,wi,h)
    #define PLATFORM_setWindowMonitor(w,x,y,wi,h) _glfwSetWindowMonitorWin32(w,x,y,wi,h)
    #define PLATFORM_setWindowDecorated(w,v)        _glfwSetWindowDecoratedWin32(w,v)
    #define PLATFORM_getKeyScancode(k)              _glfwGetKeyScancodeWin32(k)
    #define PLATFORM_pollEvents()                   _glfwPollEventsWin32()
#else // LINUX
    #include <errno.h>
    #include <sys/time.h>
    #include <sys/inotify.h>
    #include <dirent.h>
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
        if (pthread_key_create(&tls->posix.key, NULL) != 0) { DualLogError("POSIX: Failed to create context TLS"); return GLFW_FALSE; }

        tls->posix.allocated = GLFW_TRUE;
        return GLFW_TRUE;
    }

    void* _glfwPlatformGetTls(_GLFWtls* tls) { return pthread_getspecific(tls->posix.key); }
    void _glfwPlatformSetTls(_GLFWtls* tls, void* value) { pthread_setspecific(tls->posix.key, value); }
    GLFWbool _glfwPlatformCreateMutex(_GLFWmutex* mutex) {
        if (pthread_mutex_init(&mutex->posix.handle, NULL) != 0) { DualLogError("POSIX: Failed to create mutex"); return GLFW_FALSE; }
        return mutex->posix.allocated = GLFW_TRUE;
    }

    int ppoll(struct pollfd *fds, nfds_t nfds, const struct timespec *tmo_p, const sigset_t *sigmask);
    GLFWbool _glfwPollPOSIX(struct pollfd* fds, nfds_t count, double* timeout) {
        for (;;) {
            if (timeout) {
                const u64 base = _glfwPlatformGetTimerValue();
                const time_t seconds = (time_t) *timeout;
                const long nanoseconds = (long) ((*timeout - seconds) * 1e9);
                const struct timespec ts = { seconds, nanoseconds };
                const int result = ppoll(fds,count,&ts,NULL);
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

    static GLFWbool waitForVisibilityNotify(_GLFWwindow* window) {
        XEvent dummy; double timeout=0.1;
        while (!XCheckTypedWindowEvent(_glfw.x11.display,window->x11.handle,VisibilityNotify,&dummy)) { if (!waitForX11Event(&timeout)) return GLFW_FALSE; }
        return GLFW_TRUE;
    }
    
    unsigned long _glfwGetWindowPropertyX11(Window window,Atom property,Atom type,unsigned char** value) {
        Atom actualType; int actualFormat; unsigned long itemCount,bytesAfter;
        XGetWindowProperty(_glfw.x11.display,window,property,0,2147483647,False,type,&actualType,&actualFormat,&itemCount,&bytesAfter,value);
        return itemCount;
    }

    static int getWindowState(_GLFWwindow* window) {
        int result=0/*WithdrawnState*/;
        struct { u32 state; Window icon; }* state=NULL;
        if (_glfwGetWindowPropertyX11(window->x11.handle,_glfw.x11.WM_STATE,_glfw.x11.WM_STATE,(unsigned char**)&state) >= 2) result=state->state;
        if (state) XFree(state);
        return result;
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
        hints->flags &= ~((1L << 4)/*PMinSize*/|(1L << 5)/*PMaxSize*/|(1L << 7)/*PAspect*/);
        hints->flags|=((1L << 4)/*PMinSize*/|(1L << 5)/*PMaxSize*/);
        hints->min_width=hints->max_width=width; hints->min_height=hints->max_height=height;
        XSetWMNormalHints(_glfw.x11.display,window->x11.handle,hints);
        XFree(hints);
    }
    
    static void updateCursorImage(_GLFWwindow* window) {
        if (window->cursorMode==GLFW_CURSOR_NORMAL) { // Used when pressing windows key to lose focus and while cursor is hovered over window
            if (window->cursor) XDefineCursor(_glfw.x11.display,window->x11.handle,window->cursor->x11.handle);
            else XUndefineCursor(_glfw.x11.display,window->x11.handle);
        } else XDefineCursor(_glfw.x11.display,window->x11.handle,_glfw.x11.hiddenCursorHandle);
    }

    static void captureCursor(_GLFWwindow* window) { XGrabPointer(_glfw.x11.display,window->x11.handle,True,ButtonPressMask|ButtonReleaseMask|PointerMotionMask,1/*GrabModeAsync*/,1/*GrabModeAsync*/,window->x11.handle,None,CurrentTime); }
    static void releaseCursor(void) { XUngrabPointer(_glfw.x11.display,CurrentTime); }

    static void enableRawMouseMotion(_GLFWwindow* window) {
        XIEventMask em; unsigned char mask[XIMaskLen(17/*XI_RawMotion*/)]={0}; (void)window;
        em.deviceid=XIAllMasterDevices; em.mask_len=sizeof(mask); em.mask=mask;
        XISetMask(mask,17/*XI_RawMotion*/);
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

    void _glfwCreateInputContextX11(_GLFWwindow* window) {
        XIMCallback callback; callback.callback=(XIMProc)inputContextDestroyCallback; callback.client_data=(XPointer)window;
        window->x11.ic=XCreateIC(_glfw.x11.im,"inputStyle",0x0008L/*XIMPreeditNothing*/|0x0400L/*XIMStatusNothing*/,"clientWindow",window->x11.handle,"focusWindow",window->x11.handle,"destroyCallback",&callback,NULL);
        if (window->x11.ic) {
            XWindowAttributes attribs; XGetWindowAttributes(_glfw.x11.display,window->x11.handle,&attribs);
            unsigned long filter=0;
            if (XGetICValues(window->x11.ic,"filterEvents",&filter,NULL)==NULL) XSelectInput(_glfw.x11.display,window->x11.handle,attribs.your_event_mask|filter);
        }
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
            if (XGetWMNormalHints(_glfw.x11.display,window->x11.handle,hints,&supplied)) { hints->flags|=(1L << 2)/*PPosition*/; hints->x=hints->y=0; XSetWMNormalHints(_glfw.x11.display,window->x11.handle,hints); }
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
        updateNormalHints(window,width,height);
        XResizeWindow(_glfw.x11.display,window->x11.handle,width,height);
        XFlush(_glfw.x11.display);
    }

    void _glfwShowWindowX11(_GLFWwindow* window) {
        if (_glfwWindowVisibleX11(window)) return;
        XMapWindow(_glfw.x11.display,window->x11.handle);
        waitForVisibilityNotify(window);
    }

    void _glfwFocusWindowX11(_GLFWwindow* window) {
        if (_glfw.x11.NET_ACTIVE_WINDOW) sendEventToWM(window,_glfw.x11.NET_ACTIVE_WINDOW,1,0,0,0,0);
        else if (_glfwWindowVisibleX11(window)) { XRaiseWindow(_glfw.x11.display,window->x11.handle); XSetInputFocus(_glfw.x11.display,window->x11.handle,2/*RevertToParent*/,CurrentTime); }
        XFlush(_glfw.x11.display);
    }

    void _glfwSetWindowMonitorX11(_GLFWwindow* window,int xpos,int ypos,int width,int height) {
        updateNormalHints(window,width,height);
        if (_glfw.x11.NET_WM_STATE && _glfw.x11.NET_WM_STATE_FULLSCREEN) sendEventToWM(window,_glfw.x11.NET_WM_STATE,0/*remove*/,_glfw.x11.NET_WM_STATE_FULLSCREEN,0,1,0);
        else {
            XSetWindowAttributes attributes; attributes.override_redirect=False;
            XChangeWindowAttributes(_glfw.x11.display,window->x11.handle,(1L<<9)/*override redirect*/,&attributes);
            window->x11.overrideRedirect=GLFW_FALSE;
        }
        
        XDeleteProperty(_glfw.x11.display,window->x11.handle,_glfw.x11.NET_WM_BYPASS_COMPOSITOR);
        XMoveResizeWindow(_glfw.x11.display,window->x11.handle,xpos,ypos,width,height);
        XFlush(_glfw.x11.display);
    }
    
    GLFWbool _glfwWindowFocusedX11(_GLFWwindow* window) { Window focused; int state; XGetInputFocus(_glfw.x11.display,&focused,&state); return window->x11.handle==focused; }
    GLFWbool _glfwWindowVisibleX11(_GLFWwindow* window) { XWindowAttributes wa; XGetWindowAttributes(_glfw.x11.display,window->x11.handle,&wa); return wa.map_state==2/*IsViewable*/; }
    static void processEvent(XEvent* event) {
        unsigned int keycode=0; Bool filtered=False;
        if (event->type==KeyPress || event->type==KeyRelease) keycode=event->xkey.keycode;
        filtered=XFilterEvent(event,None);
        if (_glfw.x11.randr.available && event->type==_glfw.x11.randr.eventBase+1/*notify*/) { XRRUpdateConfiguration(event); _glfwPollMonitorsX11(); return; }
        if (_glfw.x11.xkb.available && event->type==_glfw.x11.xkb.eventBase/* +0 XkbEventCode*/) {
            if (((XkbEvent*)event)->any.xkb_type==XkbStateNotify && (((XkbEvent*)event)->state.changed & XkbGroupStateMask)) _glfw.x11.xkb.group=((XkbEvent*)event)->state.group;
            return;
        }
        if (event->type==GenericEvent) {
            if (_glfw.x11.xi.available) {
                _GLFWwindow* window=_glfw.x11.disabledCursorWindow;
                if (window && window->rawMouseMotion && event->xcookie.extension==_glfw.x11.xi.majorOpcode && XGetEventData(_glfw.x11.display,&event->xcookie) && event->xcookie.evtype==17/*XI_RawMotion*/) {
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
                const int key=translateKey(keycode),action=(event->type==KeyPress)?GLFW_PRESS:GLFW_RELEASE;
                if (key!=GLFW_KEY_UNKNOWN) _glfwInputKey(window,key,action);
                return;
            }
            case ButtonPress: {
                if      (event->xbutton.button==1) _glfwInputMouseClick(window,GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS);
                else if (event->xbutton.button==2) _glfwInputMouseClick(window,GLFW_MOUSE_BUTTON_MIDDLE,GLFW_PRESS);
                else if (event->xbutton.button==3) _glfwInputMouseClick(window,GLFW_MOUSE_BUTTON_RIGHT,GLFW_PRESS);
                else if (event->xbutton.button==4) Sys_Input.scrollDelta += 1.0;
                else if (event->xbutton.button==5) Sys_Input.scrollDelta += -1.0;
                else _glfwInputMouseClick(window,event->xbutton.button - 1 - 4,GLFW_PRESS);
                return;
            }
            case ButtonRelease: {
                if      (event->xbutton.button==1) _glfwInputMouseClick(window,GLFW_MOUSE_BUTTON_LEFT,GLFW_RELEASE);
                else if (event->xbutton.button==2) _glfwInputMouseClick(window,GLFW_MOUSE_BUTTON_MIDDLE,GLFW_RELEASE);
                else if (event->xbutton.button==3) _glfwInputMouseClick(window,GLFW_MOUSE_BUTTON_RIGHT,GLFW_RELEASE);
                else if (event->xbutton.button>7)  _glfwInputMouseClick(window,event->xbutton.button - 1 - 4,GLFW_RELEASE);
                return;
            }
            case EnterNotify: {
                const int x=event->xcrossing.x,y=event->xcrossing.y;
                _glfwInputCursorPos(window,x,y);
                window->x11.lastCursorPosX=x; window->x11.lastCursorPosY=y;
                return;
            }
            case LeaveNotify: return;
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
                if (event->xconfigure.width!=window->x11.width || event->xconfigure.height!=window->x11.height) { window->x11.width=event->xconfigure.width; window->x11.height=event->xconfigure.height; UpdateScreenSize(event->xconfigure.width,event->xconfigure.height); }
                int xpos=event->xconfigure.x,ypos=event->xconfigure.y;
                if (!event->xany.send_event && window->x11.parent!=_glfw.x11.root) {
                    Window dummy;
                    XTranslateCoordinates(_glfw.x11.display,window->x11.parent,_glfw.x11.root,xpos,ypos,&xpos,&ypos,&dummy);
                }
                if (xpos!=window->x11.xpos || ypos!=window->x11.ypos) { window->x11.xpos=xpos; window->x11.ypos=ypos; }
                return;
            }
            case ClientMessage: {
                if (filtered) return;
                if (event->xclient.message_type==None) return;
                if (event->xclient.message_type==_glfw.x11.WM_PROTOCOLS) {
                    const Atom protocol=event->xclient.data.l[0];
                    if (protocol==None) return;
                    if (protocol == _glfw.x11.WM_DELETE_WINDOW) window->shouldClose = GLFW_TRUE;
                    if (protocol==_glfw.x11.NET_WM_PING) {
                        XEvent reply=*event; reply.xclient.window=_glfw.x11.root;
                        XSendEvent(_glfw.x11.display,_glfw.x11.root,False,SubstructureNotifyMask|SubstructureRedirectMask,&reply);
                    }
                }
                return;
            }
            case FocusIn: {
                if (event->xfocus.mode==NotifyGrab || event->xfocus.mode==NotifyUngrab) return;
                if (window->cursorMode==GLFW_CURSOR_DISABLED) disableCursor(window);
                if (window->x11.ic) XSetICFocus(window->x11.ic);
                _glfwInputWindowFocus(window,GLFW_TRUE);
                return;
            }
            case FocusOut: {
                if (event->xfocus.mode==NotifyGrab || event->xfocus.mode==NotifyUngrab) return;
                if (window->cursorMode==GLFW_CURSOR_DISABLED) enableCursor(window);
                if (window->x11.ic) XUnsetICFocus(window->x11.ic);
                _glfwInputWindowFocus(window,GLFW_FALSE);
                return;
            }
            case PropertyNotify: {
                if (event->xproperty.state!=PropertyNewValue) return;
                if (event->xproperty.atom==_glfw.x11.WM_STATE) {
                    if (getWindowState(window) == 3/*IconicState*/) { Sys_Global.gamePaused = true; return; };
                }
                return;
            }
            case DestroyNotify: return;
        }
    }

    void _glfwSetWindowDecoratedX11(_GLFWwindow* window,GLFWbool enabled) {
        struct { unsigned long flags,functions,decorations; long input_mode; unsigned long status; } hints={0};
        hints.flags=2; hints.decorations=enabled?1:0;
        XChangeProperty(_glfw.x11.display,window->x11.handle,_glfw.x11.MOTIF_WM_HINTS,_glfw.x11.MOTIF_WM_HINTS,32,PropModeReplace,(unsigned char*)&hints,sizeof(hints)/sizeof(long));
    }

    void _glfwSetRawMouseMotionX11(_GLFWwindow* window,GLFWbool enabled) {
        if (!_glfw.x11.xi.available || _glfw.x11.disabledCursorWindow!=window) return;
        if (enabled) enableRawMouseMotion(window); else disableRawMouseMotion(window);
    }

    void _glfwPollEventsX11(void) {
        if (_glfw.joysticksInitialized) _glfwDetectJoystickConnectionLinux();
        XPending(_glfw.x11.display);
        while (QLength(_glfw.x11.display)) { XEvent event; XNextEvent(_glfw.x11.display,&event); processEvent(&event); }
        _GLFWwindow* window = _glfw.x11.disabledCursorWindow;
        if (window) {
            int width,height; _glfwGetWindowSizeX11(window,&width,&height);
            if (window->x11.lastCursorPosX!=width/2 || window->x11.lastCursorPosY!=height/2) _glfwSetCursorPosX11(window,width/2,height/2);
        }
        XFlush(_glfw.x11.display);
    }

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
                if (window->rawMouseMotion) enableRawMouseMotion(window);
            } else if (_glfw.x11.disabledCursorWindow==window) { if (window->rawMouseMotion) disableRawMouseMotion(window); }
            if (mode==GLFW_CURSOR_DISABLED) captureCursor(window); else releaseCursor();
            if (mode==GLFW_CURSOR_DISABLED) _glfw.x11.disabledCursorWindow=window;
            else if (_glfw.x11.disabledCursorWindow==window) { _glfw.x11.disabledCursorWindow=NULL; _glfwSetCursorPosX11(window,_glfw.x11.restoreCursorPosX,_glfw.x11.restoreCursorPosY); }
        }
        updateCursorImage(window); XFlush(_glfw.x11.display);
    }

    static const XRRModeInfo* getModeInfo(const XRRScreenResources* sr, RRMode id) { for (int i = 0;  i < sr->nmode;  i++){ if (sr->modes[i].id == id) {return sr->modes + i;} } return NULL; }
    static GLFWvidmode vidmodeFromModeInfo(const XRRModeInfo* mi, const XRRCrtcInfo* ci) {
        GLFWvidmode mode;
        if (ci->rotation == 2 || ci->rotation == 8) {  mode.width  = mi->height; mode.height = mi->width; } // ==90, ==270
        else { mode.width = mi->width; mode.height = mi->height; }
        mode.refreshRate = (mi->hTotal && mi->vTotal) ? (int)vround((double) mi->dotClock / ((double) mi->hTotal * (double) mi->vTotal)) : 0;
        _glfwSplitBPP(DefaultDepth(_glfw.x11.display,_glfw.x11.screen),&mode.redBits,&mode.greenBits,&mode.blueBits);
        return mode;
    }

    typedef struct { int screen_number; short x_org, y_org, width, height; } ScreenInfo;
    void _glfwPollMonitorsX11(void) {
        if (_glfw.x11.randr.available && !_glfw.x11.randr.monitorBroken) {
            int disconnectedCount, screenCount = 0;
            _GLFWmonitor** disconnected = NULL;
            ScreenInfo* screens = NULL;
            XRRScreenResources* sr = XRRGetScreenResourcesCurrent(_glfw.x11.display,_glfw.x11.root);
            RROutput primary = XRRGetOutputPrimary(_glfw.x11.display,_glfw.x11.root);
            disconnectedCount = _glfw.monitorCount;
            if (disconnectedCount) {
                disconnected = _glfw_calloc(_glfw.monitorCount, sizeof(_GLFWmonitor*));
                __builtin_memcpy(disconnected,_glfw.monitors,_glfw.monitorCount * sizeof(_GLFWmonitor*));
            }

            for (int i = 0;  i < sr->noutput;  i++) {
                int j, type, widthMM, heightMM;
                XRROutputInfo* oi = XRRGetOutputInfo(_glfw.x11.display, sr, sr->outputs[i]);
                if (oi->connection != 0/*connected*/ || oi->crtc == None) { XRRFreeOutputInfo(oi); continue; }

                for (j = 0;  j < disconnectedCount;  j++) {
                    if (disconnected[j] && disconnected[j]->x11.output == sr->outputs[i]) { disconnected[j] = NULL; break; }
                }

                if (j < disconnectedCount) { XRRFreeOutputInfo(oi); continue; }

                XRRCrtcInfo* ci = XRRGetCrtcInfo(_glfw.x11.display, sr, oi->crtc);
                if (!ci) { XRRFreeOutputInfo(oi); continue; }

                if (ci->rotation == 2 || ci->rotation == 8) { widthMM  = oi->mm_height; heightMM = oi->mm_width; } // == 90, == 270
                else { widthMM  = oi->mm_width; heightMM = oi->mm_height; }
                
                if (widthMM <= 0 || heightMM <= 0) { widthMM  = (int) (ci->width * 25.4f / 96.f); heightMM = (int) (ci->height * 25.4f / 96.f); }
                _GLFWmonitor* monitor = _glfwAllocMonitor(oi->name, widthMM, heightMM);
                monitor->x11.output = sr->outputs[i]; monitor->x11.crtc   = oi->crtc;
                for (j = 0;  j < screenCount;  j++) {
                    if (screens[j].x_org == ci->x && screens[j].y_org == ci->y && screens[j].width == (short)ci->width && screens[j].height == (short)ci->height) { monitor->x11.index = j; break; }
                }

                type = (monitor->x11.output == primary) ? 0 : 1; _glfwInputMonitor(monitor,GLFW_CONNECTED,type); XRRFreeOutputInfo(oi); XRRFreeCrtcInfo(ci);
            }

            XRRFreeScreenResources(sr);
            if (screens) XFree(screens);
            for (int i = 0;  i < disconnectedCount;  i++) {
                if (disconnected[i]) _glfwInputMonitor(disconnected[i], GLFW_DISCONNECTED, 0);
            }

            free(disconnected);
        } else {
            const int widthMM = DisplayWidthMM(_glfw.x11.display, _glfw.x11.screen); const int heightMM = DisplayHeightMM(_glfw.x11.display, _glfw.x11.screen);
            _glfwInputMonitor(_glfwAllocMonitor("Display", widthMM, heightMM),GLFW_CONNECTED,0);
        }
    }

    void _glfwGetMonitorWorkareaX11(_GLFWmonitor* monitor,int* xpos,int* ypos,int* width,int* height) {
        int areaX = 0, areaY = 0, areaWidth = 0, areaHeight = 0;
        if (_glfw.x11.randr.available && !_glfw.x11.randr.monitorBroken) {
            XRRScreenResources* sr = XRRGetScreenResourcesCurrent(_glfw.x11.display,_glfw.x11.root);
            XRRCrtcInfo* ci = XRRGetCrtcInfo(_glfw.x11.display,sr,monitor->x11.crtc);
            const XRRModeInfo* mi = getModeInfo(sr,ci->mode);
            areaX = ci->x, areaY = ci->y;
            if (ci->rotation == 2 || ci->rotation == 8) { areaWidth = mi->height, areaHeight = mi->width; } // ==90, ==270
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
                if (!((mi->modeFlags & 0x00000010/*interlace*/) == 0)) continue;
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
            if (!mi) { DualLogError("X11: Failed to query video mode"); return GLFW_FALSE; }
        } else {
            mode->width = DisplayWidth(_glfw.x11.display,_glfw.x11.screen), mode->height = DisplayHeight(_glfw.x11.display,_glfw.x11.screen), mode->refreshRate = 0;
            _glfwSplitBPP(DefaultDepth(_glfw.x11.display,_glfw.x11.screen),&mode->redBits,&mode->greenBits,&mode->blueBits);
        }
        return GLFW_TRUE;
    }

    int _glfwGetKeyScancodeX11(int key) { return _glfw.x11.scancodes[key]; }
    static int translateKeySyms(const KeySym* keysyms, int width) {
        if (width > 1) { // Numpad with numlock ON (keysyms[1]) - contiguous 0xffb0..0xffb9
            if (keysyms[1] >= 0xffb0 && keysyms[1] <= 0xffb9) return GLFW_KEY_KP_0 + (keysyms[1] - 0xffb0);
            switch (keysyms[1]) {
                case 0xffac: case 0xffae: return GLFW_KEY_KP_DECIMAL; // KP_Separator, KP_Decimal
                case 0xffbd:              return GLFW_KEY_KP_EQUAL;   // KP_Equal
                case 0xff8d:              return GLFW_KEY_KP_ENTER;   // KP_Enter
                default: break;
            }
        }

        KeySym k = keysyms[0];
        if (k >= 0x0061 && k <= 0x007a) return GLFW_KEY_A + (k - 0x0061);  // a-z
        if (k >= 0x0030 && k <= 0x0039) return GLFW_KEY_0 + (k - 0x0030);  // 0-9
        if (k >= 0xffbe && k <= 0xffd6) return GLFW_KEY_F1 + (k - 0xffbe); // F1..F25: 0xffbe..0xffd6
        if (k >= 0xff95 && k <= 0xff9f) { // KP numpad with numlock OFF (cursor keys): 0xff95..0xff9f
            static const int kp_off[] = {GLFW_KEY_KP_7,GLFW_KEY_KP_4,GLFW_KEY_KP_8,GLFW_KEY_KP_6,GLFW_KEY_KP_2,GLFW_KEY_KP_9,GLFW_KEY_KP_3,GLFW_KEY_KP_1,-1,GLFW_KEY_KP_0,GLFW_KEY_KP_DECIMAL };
            int r = kp_off[k - 0xff95];
            if (r != -1) return r;
        }
        switch (k) {
            case 0xff1b: return GLFW_KEY_ESCAPE;
            case 0xff09: return GLFW_KEY_TAB;
            case 0xff0d: return GLFW_KEY_ENTER;
            case 0xff08: return GLFW_KEY_BACKSPACE;
            case 0xffff: return GLFW_KEY_DELETE;
            case 0xff50: return GLFW_KEY_HOME;
            case 0xff57: return GLFW_KEY_END;
            case 0xff55: return GLFW_KEY_PAGE_UP;
            case 0xff56: return GLFW_KEY_PAGE_DOWN;
            case 0xff63: return GLFW_KEY_INSERT;
            case 0xff51: return GLFW_KEY_LEFT;
            case 0xff53: return GLFW_KEY_RIGHT;
            case 0xff54: return GLFW_KEY_DOWN;
            case 0xff52: return GLFW_KEY_UP;
            case 0xff13: return GLFW_KEY_PAUSE;
            case 0xff14: return GLFW_KEY_SCROLL_LOCK;
            case 0xff61: return GLFW_KEY_PRINT_SCREEN;
            case 0xff7f: return GLFW_KEY_NUM_LOCK;
            case 0xffe5: return GLFW_KEY_CAPS_LOCK;
            case 0xff67: return GLFW_KEY_MENU;
            case 0xffe1: return GLFW_KEY_LEFT_SHIFT;
            case 0xffe2: return GLFW_KEY_RIGHT_SHIFT;
            case 0xffe3: return GLFW_KEY_LEFT_CONTROL;
            case 0xffe4: return GLFW_KEY_RIGHT_CONTROL;
            case 0xffe7: case 0xffe9: return GLFW_KEY_LEFT_ALT;   // Meta_L, Alt_L
            case 0xff7e: case 0xfe03: case 0xffe8: case 0xffea: return GLFW_KEY_RIGHT_ALT; // Mode_switch, ISO_Level3_Shift, Meta_R, Alt_R
            case 0xffeb: return GLFW_KEY_LEFT_SUPER;
            case 0xffec: return GLFW_KEY_RIGHT_SUPER;
            case 0xffaa: return GLFW_KEY_KP_MULTIPLY;
            case 0xffab: return GLFW_KEY_KP_ADD;
            case 0xffad: return GLFW_KEY_KP_SUBTRACT;
            case 0xffaf: return GLFW_KEY_KP_DIVIDE;
            case 0xffbd: return GLFW_KEY_KP_EQUAL;
            case 0xff8d: return GLFW_KEY_KP_ENTER;
            case 0x0020: return GLFW_KEY_SPACE;
            case 0x0027: return GLFW_KEY_APOSTROPHE;
            case 0x002c: return GLFW_KEY_COMMA;
            case 0x002d: return GLFW_KEY_MINUS;
            case 0x002e: return GLFW_KEY_PERIOD;
            case 0x002f: return GLFW_KEY_SLASH;
            case 0x003b: return GLFW_KEY_SEMICOLON;
            case 0x003c: return GLFW_KEY_WORLD_1;
            case 0x003d: return GLFW_KEY_EQUAL;
            case 0x005b: return GLFW_KEY_LEFT_BRACKET;
            case 0x005c: return GLFW_KEY_BACKSLASH;
            case 0x005d: return GLFW_KEY_RIGHT_BRACKET;
            case 0x0060: return GLFW_KEY_GRAVE_ACCENT;
            default:     return GLFW_KEY_UNKNOWN;
        }
    }

    static void createKeyTables(void) {
        int scancodeMin, scancodeMax;
        __builtin_memset(_glfw.x11.keycodes, -1, sizeof(_glfw.x11.keycodes));
        __builtin_memset(_glfw.x11.scancodes, -1, sizeof(_glfw.x11.scancodes));
        if (_glfw.x11.xkb.available) {
            XkbDescPtr desc = XkbGetMap(_glfw.x11.display, 0, XkbUseCoreKbd);
            XkbGetNames(_glfw.x11.display, XkbKeyNamesMask | XkbKeyAliasesMask, desc);
            scancodeMin = desc->min_key_code; scancodeMax = desc->max_key_code;
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
                    if (strncmp(desc->names->keys[sc].name, keymap[i].name, XkbKeyNameLength) == 0) { key = keymap[i].key; break; }
                }
                for (int i = 0; i < desc->names->num_key_aliases && key == GLFW_KEY_UNKNOWN; i++) {
                    if (strncmp(desc->names->key_aliases[i].real, desc->names->keys[sc].name, XkbKeyNameLength) != 0) continue;
                    for (int j = 0; j < (int)(sizeof(keymap)/sizeof(keymap[0])); j++) {
                        if (strncmp(desc->names->key_aliases[i].alias, keymap[j].name, XkbKeyNameLength) == 0) { key = keymap[j].key; break; }
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

    static Atom getAtomIfSupported(Atom* atoms, unsigned long count, const char* name) {
        const Atom atom = XInternAtom(_glfw.x11.display, name, False);
        for (unsigned long i = 0; i < count; i++) if (atoms[i] == atom) return atom;
        return None;
    }

    static void detectEWMH(void) {
        Window* wfr = NULL;
        if (!_glfwGetWindowPropertyX11(_glfw.x11.root, _glfw.x11.NET_SUPPORTING_WM_CHECK, XA_WINDOW, (unsigned char**)&wfr)) return;
        Window* wfc = NULL;
        if (!_glfwGetWindowPropertyX11(*wfr, _glfw.x11.NET_SUPPORTING_WM_CHECK, XA_WINDOW, (unsigned char**)&wfc)) { XFree(wfr); return; }
        if (*wfr != *wfc) { XFree(wfr); XFree(wfc); return; }
        
        XFree(wfr); XFree(wfc);
        Atom* sa = NULL;
        const unsigned long ac = _glfwGetWindowPropertyX11(_glfw.x11.root, _glfw.x11.NET_SUPPORTED, XA_ATOM, (unsigned char**)&sa);
        #define GA(name) getAtomIfSupported(sa, ac, name)
        _glfw.x11.NET_WM_STATE                  = GA("_NET_WM_STATE");
        _glfw.x11.NET_WM_STATE_FULLSCREEN       = GA("_NET_WM_STATE_FULLSCREEN");
        _glfw.x11.NET_WM_WINDOW_TYPE            = GA("_NET_WM_WINDOW_TYPE");
        _glfw.x11.NET_WM_WINDOW_TYPE_NORMAL     = GA("_NET_WM_WINDOW_TYPE_NORMAL");
        _glfw.x11.NET_WORKAREA                  = GA("_NET_WORKAREA");
        _glfw.x11.NET_CURRENT_DESKTOP           = GA("_NET_CURRENT_DESKTOP");
        _glfw.x11.NET_ACTIVE_WINDOW             = GA("_NET_ACTIVE_WINDOW");
        #undef GA
        if (sa) XFree(sa);
    }

    static GLFWbool initExtensions(void) {
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
            _glfw.x11.randr.FreeCrtcInfo           = (PFN_XRRFreeCrtcInfo)           _glfwPlatformGetModuleSymbol(_glfw.x11.randr.handle,"XRRFreeCrtcInfo");
            _glfw.x11.randr.FreeOutputInfo         = (PFN_XRRFreeOutputInfo)         _glfwPlatformGetModuleSymbol(_glfw.x11.randr.handle,"XRRFreeOutputInfo");
            _glfw.x11.randr.FreeScreenResources    = (PFN_XRRFreeScreenResources)    _glfwPlatformGetModuleSymbol(_glfw.x11.randr.handle,"XRRFreeScreenResources");
            _glfw.x11.randr.GetCrtcInfo            = (PFN_XRRGetCrtcInfo)            _glfwPlatformGetModuleSymbol(_glfw.x11.randr.handle,"XRRGetCrtcInfo");
            _glfw.x11.randr.GetOutputInfo          = (PFN_XRRGetOutputInfo)          _glfwPlatformGetModuleSymbol(_glfw.x11.randr.handle,"XRRGetOutputInfo");
            _glfw.x11.randr.GetOutputPrimary       = (PFN_XRRGetOutputPrimary)       _glfwPlatformGetModuleSymbol(_glfw.x11.randr.handle,"XRRGetOutputPrimary");
            _glfw.x11.randr.GetScreenResourcesCurrent=(PFN_XRRGetScreenResourcesCurrent)_glfwPlatformGetModuleSymbol(_glfw.x11.randr.handle,"XRRGetScreenResourcesCurrent");
            _glfw.x11.randr.QueryExtension         = (PFN_XRRQueryExtension)         _glfwPlatformGetModuleSymbol(_glfw.x11.randr.handle,"XRRQueryExtension");
            _glfw.x11.randr.QueryVersion           = (PFN_XRRQueryVersion)           _glfwPlatformGetModuleSymbol(_glfw.x11.randr.handle,"XRRQueryVersion");
            _glfw.x11.randr.SelectInput            = (PFN_XRRSelectInput)            _glfwPlatformGetModuleSymbol(_glfw.x11.randr.handle,"XRRSelectInput");
            _glfw.x11.randr.SetCrtcConfig          = (PFN_XRRSetCrtcConfig)          _glfwPlatformGetModuleSymbol(_glfw.x11.randr.handle,"XRRSetCrtcConfig");
            _glfw.x11.randr.UpdateConfiguration   = (PFN_XRRUpdateConfiguration)    _glfwPlatformGetModuleSymbol(_glfw.x11.randr.handle,"XRRUpdateConfiguration");
            if (XRRQueryExtension(_glfw.x11.display, &_glfw.x11.randr.eventBase, &_glfw.x11.randr.errorBase)) {
                if (XRRQueryVersion(_glfw.x11.display, &_glfw.x11.randr.major, &_glfw.x11.randr.minor)) {
                    if (_glfw.x11.randr.major > 1 || _glfw.x11.randr.minor >= 3) _glfw.x11.randr.available = GLFW_TRUE;
                } else DualLogError("X11: Failed to query RandR version");
            }
        }

        if (_glfw.x11.randr.available) {
            XRRScreenResources* sr = XRRGetScreenResourcesCurrent(_glfw.x11.display, _glfw.x11.root);
            if (!sr->ncrtc)                                                             _glfw.x11.randr.monitorBroken = GLFW_TRUE;
            XRRFreeScreenResources(sr);
        }
        if (_glfw.x11.randr.available && !_glfw.x11.randr.monitorBroken) XRRSelectInput(_glfw.x11.display,_glfw.x11.root,(1L << 2)/*change notify mask*/);
        _glfw.x11.xcursor.handle = _glfwPlatformLoadModule("libXcursor.so.1");
        if (_glfw.x11.xcursor.handle) {
            _glfw.x11.xcursor.ImageCreate      = (PFN_XcursorImageCreate)      _glfwPlatformGetModuleSymbol(_glfw.x11.xcursor.handle,"XcursorImageCreate");
            _glfw.x11.xcursor.ImageDestroy     = (PFN_XcursorImageDestroy)     _glfwPlatformGetModuleSymbol(_glfw.x11.xcursor.handle,"XcursorImageDestroy");
            _glfw.x11.xcursor.ImageLoadCursor  = (PFN_XcursorImageLoadCursor)  _glfwPlatformGetModuleSymbol(_glfw.x11.xcursor.handle,"XcursorImageLoadCursor");
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

        createKeyTables();
        #define IA(n) XInternAtom(_glfw.x11.display, n, False)
        _glfw.x11.UTF8_STRING        = IA("UTF8_STRING");
        _glfw.x11.WM_PROTOCOLS       = IA("WM_PROTOCOLS");
        _glfw.x11.WM_STATE           = IA("WM_STATE");
        _glfw.x11.WM_DELETE_WINDOW   = IA("WM_DELETE_WINDOW");
        _glfw.x11.NET_SUPPORTED          = IA("_NET_SUPPORTED");
        _glfw.x11.NET_SUPPORTING_WM_CHECK= IA("_NET_SUPPORTING_WM_CHECK");
        _glfw.x11.NET_WM_ICON            = IA("_NET_WM_ICON");
        _glfw.x11.NET_WM_PING            = IA("_NET_WM_PING");
        _glfw.x11.NET_WM_PID             = IA("_NET_WM_PID");
        _glfw.x11.NET_WM_NAME            = IA("_NET_WM_NAME");
        _glfw.x11.NET_WM_BYPASS_COMPOSITOR = IA("_NET_WM_BYPASS_COMPOSITOR");
        _glfw.x11.MOTIF_WM_HINTS         = IA("_MOTIF_WM_HINTS");
        #undef IA
        detectEWMH();
        return GLFW_TRUE;
    }

    char *setlocale(int category, const char *locale);
    GLFWbool _glfwConnectX11(void) {
        if (strcmp(setlocale(0/*LC_CTYPE*/,NULL), "C") == 0) setlocale(0/*LC_CTYPE*/,"");
        void* module = _glfwPlatformLoadModule("libX11.so.6");
        if (!module) { DualLogError("X11: Failed to load Xlib"); return GLFW_FALSE; }

        PFN_XInitThreads  XInitThreads  = (PFN_XInitThreads) _glfwPlatformGetModuleSymbol(module, "XInitThreads");
        PFN_XOpenDisplay  XOpenDisplay  = (PFN_XOpenDisplay) _glfwPlatformGetModuleSymbol(module, "XOpenDisplay");
        if (!XInitThreads || !XOpenDisplay) { DualLogError("X11: Failed to load Xlib entry point"); if (module) {dlclose(module);} return GLFW_FALSE; }

        XInitThreads();
        Display* display = XOpenDisplay(NULL);
        if (!display) { const char* name = getenv("DISPLAY"); DualLogError(name ? "X11: Failed to open display %s" : "X11: The DISPLAY environment variable is missing", name); if (module) {dlclose(module);} return GLFW_FALSE; }

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
        _glfw.x11.xlib.CheckTypedWindowEvent = (PFN_XCheckTypedWindowEvent)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XCheckTypedWindowEvent");
        _glfw.x11.xlib.CloseDisplay = (PFN_XCloseDisplay)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XCloseDisplay");
        _glfw.x11.xlib.CreateColormap = (PFN_XCreateColormap)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XCreateColormap");
        _glfw.x11.xlib.CreateIC = (PFN_XCreateIC)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XCreateIC");
        _glfw.x11.xlib.CreateWindow = (PFN_XCreateWindow)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XCreateWindow");
        _glfw.x11.xlib.DefineCursor = (PFN_XDefineCursor)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XDefineCursor");
        _glfw.x11.xlib.DeleteProperty = (PFN_XDeleteProperty)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XDeleteProperty");
        _glfw.x11.xlib.DisplayKeycodes = (PFN_XDisplayKeycodes)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XDisplayKeycodes");
        _glfw.x11.xlib.FilterEvent = (PFN_XFilterEvent)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XFilterEvent");
        _glfw.x11.xlib.FindContext = (PFN_XFindContext)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XFindContext");
        _glfw.x11.xlib.Flush = (PFN_XFlush)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XFlush");
        _glfw.x11.xlib.Free = (PFN_XFree)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XFree");
        _glfw.x11.xlib.FreeEventData = (PFN_XFreeEventData)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XFreeEventData");
        _glfw.x11.xlib.GetEventData = (PFN_XGetEventData)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XGetEventData");
        _glfw.x11.xlib.GetICValues = (PFN_XGetICValues)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XGetICValues");
        _glfw.x11.xlib.GetIMValues = (PFN_XGetIMValues)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XGetIMValues");
        _glfw.x11.xlib.GetInputFocus = (PFN_XGetInputFocus)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XGetInputFocus");
        _glfw.x11.xlib.GetKeyboardMapping = (PFN_XGetKeyboardMapping)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XGetKeyboardMapping");
        _glfw.x11.xlib.GetWMNormalHints = (PFN_XGetWMNormalHints)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XGetWMNormalHints");
        _glfw.x11.xlib.GetWindowAttributes = (PFN_XGetWindowAttributes)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XGetWindowAttributes");
        _glfw.x11.xlib.GetWindowProperty = (PFN_XGetWindowProperty)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XGetWindowProperty");
        _glfw.x11.xlib.GrabPointer = (PFN_XGrabPointer)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XGrabPointer");
        _glfw.x11.xlib.InternAtom = (PFN_XInternAtom)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XInternAtom");
        _glfw.x11.xlib.MapWindow = (PFN_XMapWindow)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XMapWindow");
        _glfw.x11.xlib.MoveResizeWindow = (PFN_XMoveResizeWindow)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XMoveResizeWindow");
        _glfw.x11.xlib.MoveWindow = (PFN_XMoveWindow)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XMoveWindow");
        _glfw.x11.xlib.NextEvent = (PFN_XNextEvent)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XNextEvent");
        _glfw.x11.xlib.Pending = (PFN_XPending)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XPending");
        _glfw.x11.xlib.QueryExtension = (PFN_XQueryExtension)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XQueryExtension");
        _glfw.x11.xlib.QueryPointer = (PFN_XQueryPointer)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XQueryPointer");
        _glfw.x11.xlib.RaiseWindow = (PFN_XRaiseWindow)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XRaiseWindow");
        _glfw.x11.xlib.RegisterIMInstantiateCallback = (PFN_XRegisterIMInstantiateCallback)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XRegisterIMInstantiateCallback");
        _glfw.x11.xlib.ResizeWindow = (PFN_XResizeWindow)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XResizeWindow");
        _glfw.x11.xlib.SaveContext = (PFN_XSaveContext)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XSaveContext");
        _glfw.x11.xlib.SelectInput = (PFN_XSelectInput)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XSelectInput");
        _glfw.x11.xlib.SendEvent = (PFN_XSendEvent)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XSendEvent");
        _glfw.x11.xlib.SetClassHint = (PFN_XSetClassHint)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XSetClassHint");
        _glfw.x11.xlib.SetICFocus = (PFN_XSetICFocus)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XSetICFocus");
        _glfw.x11.xlib.SetIMValues = (PFN_XSetIMValues)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XSetIMValues");
        _glfw.x11.xlib.SetInputFocus = (PFN_XSetInputFocus)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XSetInputFocus");
        _glfw.x11.xlib.SetLocaleModifiers = (PFN_XSetLocaleModifiers)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XSetLocaleModifiers");
        _glfw.x11.xlib.SetWMHints = (PFN_XSetWMHints)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XSetWMHints");
        _glfw.x11.xlib.SetWMNormalHints = (PFN_XSetWMNormalHints)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XSetWMNormalHints");
        _glfw.x11.xlib.SetWMProtocols = (PFN_XSetWMProtocols)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XSetWMProtocols");
        _glfw.x11.xlib.SupportsLocale = (PFN_XSupportsLocale)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XSupportsLocale");
        _glfw.x11.xlib.TranslateCoordinates = (PFN_XTranslateCoordinates)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XTranslateCoordinates");
        _glfw.x11.xlib.UndefineCursor = (PFN_XUndefineCursor)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XUndefineCursor");
        _glfw.x11.xlib.UngrabPointer = (PFN_XUngrabPointer)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XUngrabPointer");
        _glfw.x11.xlib.UnsetICFocus = (PFN_XUnsetICFocus)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XUnsetICFocus");
        _glfw.x11.xlib.WarpPointer = (PFN_XWarpPointer)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XWarpPointer");
        _glfw.x11.xkb.FreeKeyboard = (PFN_XkbFreeKeyboard) _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XkbFreeKeyboard");
        _glfw.x11.xkb.FreeNames = (PFN_XkbFreeNames) _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XkbFreeNames");
        _glfw.x11.xkb.GetMap = (PFN_XkbGetMap) _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XkbGetMap");
        _glfw.x11.xkb.GetNames = (PFN_XkbGetNames) _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XkbGetNames");
        _glfw.x11.xkb.GetState = (PFN_XkbGetState) _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XkbGetState");
        _glfw.x11.xkb.QueryExtension = (PFN_XkbQueryExtension) _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XkbQueryExtension");
        _glfw.x11.xkb.SelectEventDetails = (PFN_XkbSelectEventDetails) _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XkbSelectEventDetails");
        _glfw.x11.xkb.SetDetectableAutoRepeat = (PFN_XkbSetDetectableAutoRepeat)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XkbSetDetectableAutoRepeat");
        _glfw.x11.screen = DefaultScreen(_glfw.x11.display);
        _glfw.x11.root = RootWindow(_glfw.x11.display,_glfw.x11.screen);
        static XContext lastContext = 0;
        _glfw.x11.context = ++lastContext;
        if (!initExtensions()) return GLFW_FALSE;
        XSetWindowAttributes wa; wa.event_mask = PropertyChangeMask;
        _glfw.x11.helperWindowHandle = XCreateWindow(_glfw.x11.display,_glfw.x11.root,0,0,1,1,0,0,2/*input only*/,DefaultVisual(_glfw.x11.display,_glfw.x11.screen),(1L<<11)/*event mask*/,&wa);
        XcursorImage* native = XcursorImageCreate(16,16); __builtin_memset(native->pixels,0,256*sizeof(XcursorPixel)); native->xhot=native->yhot=0;
        _glfw.x11.hiddenCursorHandle = XcursorImageLoadCursor(_glfw.x11.display,native); XcursorImageDestroy(native);
        _glfwPollMonitorsX11();
        return GLFW_TRUE;
    }

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
        if (ioctl(linjs.fd, EVIOCGBIT(0, sizeof(evBits)), evBits) < 0 || ioctl(linjs.fd, EVIOCGBIT(EV_KEY, sizeof(keyBits)), keyBits) < 0 || ioctl(linjs.fd, EVIOCGBIT(EV_ABS, sizeof(absBits)), absBits) < 0 || ioctl(linjs.fd, EVIOCGID, &id) < 0) { DualLogError("Linux: Failed to query input device"); close(linjs.fd); return GLFW_FALSE; }
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
        if (!_glfw.linjs.regexCompiled) { DualLogError("Linux: Failed to compile regex"); return GLFW_FALSE; }

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

        qsort(_glfw.joysticks,count,sizeof(_GLFWjoystick),compareJoysticks);
        return GLFW_TRUE;
    }

    GLFWbool _glfwPollJoystickLinux(_GLFWjoystick* js, int mode) {
        (void)mode; for (;;) {
            struct input_event e;
            if (read(js->linjs.fd,&e,sizeof(e)) < 0) { closeJoystick(js); break; }

            if (e.type == EV_SYN) {
                if (e.code == 3/*sync dropped*/) _glfw.linjs.dropped = GLFW_TRUE;
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
    static int getGLXFBConfigAttrib(GLXFBConfig fbconfig, int attrib) { int value; _glfw.glx.GetFBConfigAttrib(_glfw.x11.display, fbconfig, attrib, &value); return value; }
    static GLFWbool chooseGLXFBConfig(const _GLFWfbconfig* desired, GLXFBConfig* result) {
        GLXFBConfig* nativeConfigs; _GLFWfbconfig* usableConfigs; const _GLFWfbconfig* closest; int nativeCount,usableCount;
        nativeConfigs = _glfw.glx.GetFBConfigs(_glfw.x11.display, _glfw.x11.screen, &nativeCount);
        if (!nativeConfigs || !nativeCount) { DualLogError("GLX: No GLXFBConfigs returned"); return GLFW_FALSE; }
        usableConfigs = _glfw_calloc(nativeCount, sizeof(_GLFWfbconfig)); usableCount = 0;
        for (int i = 0;  i < nativeCount;  i++) {
            const GLXFBConfig n = nativeConfigs[i];
            _GLFWfbconfig* u = usableConfigs + usableCount;
            if (!(getGLXFBConfigAttrib(n,0x8011/*render type*/) & 0x00000001/*rgba bit*/)) continue;
            if (!(getGLXFBConfigAttrib(n,0x8010/*drawable type*/) & 0x00000001/*window bit*/)) continue;
            if (getGLXFBConfigAttrib(n,5) != desired->doublebuffer) continue;

            u->redBits = getGLXFBConfigAttrib(n,8); u->greenBits = getGLXFBConfigAttrib(n,9); u->blueBits = getGLXFBConfigAttrib(n,10); u->alphaBits = getGLXFBConfigAttrib(n,11); u->depthBits = getGLXFBConfigAttrib(n,12); u->stencilBits = getGLXFBConfigAttrib(n,13);
            u->accumRedBits = getGLXFBConfigAttrib(n,14); u->accumGreenBits = getGLXFBConfigAttrib(n,15); u->accumBlueBits = getGLXFBConfigAttrib(n,16); u->accumAlphaBits = getGLXFBConfigAttrib(n,17); u->auxBuffers = getGLXFBConfigAttrib(n,7);
            if (getGLXFBConfigAttrib(n,6)) u->stereo = GLFW_TRUE;
            u->handle = (uintptr_t) n;
            usableCount++;
        }

        closest = _glfwChooseFBConfig(desired, usableConfigs, usableCount);
        if (closest) *result = (GLXFBConfig) closest->handle;
        XFree(nativeConfigs);
        free(usableConfigs);
        return closest != NULL;
    }

    static void makeContextCurrentGLX(_GLFWwindow* window) {
        if (window) {
            if (!_glfw.glx.MakeCurrent(_glfw.x11.display,window->context.glx.window,window->context.glx.handle)) { DualLogError("GLX: Failed to make context current"); return; }
        } else {
            if (!_glfw.glx.MakeCurrent(_glfw.x11.display, None, NULL)) { DualLogError("GLX: Failed to clear current context"); return; }
        }

        _glfwPlatformSetTls(&_glfw.contextSlot, window);
    }

    static void swapBuffersGLX(_GLFWwindow* window) { _glfw.glx.SwapBuffers(_glfw.x11.display, window->context.glx.window); }
    static void swapIntervalGLX(int interval) { _GLFWwindow* window = _glfwPlatformGetTls(&_glfw.contextSlot); _glfw.glx.SwapIntervalEXT(_glfw.x11.display,window->context.glx.window,interval); }
    static int extensionSupportedGLX(const char* extension) {
        const char* extensions = glXQueryExtensionsString(_glfw.x11.display, _glfw.x11.screen);
        if (extensions) {
            if (_glfwStringInExtensionString(extension, extensions)) return GLFW_TRUE;
        }

        return GLFW_FALSE;
    }

    static GLFWglproc getProcAddressGLX(const char* procname) {
        if (_glfw.glx.GetProcAddress) return _glfw.glx.GetProcAddress((const GLubyte*) procname);
        else if (_glfw.glx.GetProcAddressARB) return _glfw.glx.GetProcAddressARB((const GLubyte*) procname);
        else return _glfwPlatformGetModuleSymbol(_glfw.glx.handle, procname);
    }
    
    GLFWbool _glfwCreateContextGLX(_GLFWwindow* window, const _GLFWctxconfig* ctxconfig, const _GLFWfbconfig* fbconfig) {
        int attribs[40], index = 0, mask = 0, flags = 0;
        GLXFBConfig native = NULL; GLXContext share = NULL;
        if (ctxconfig->share) share = ctxconfig->share->context.glx.handle;
        if (!chooseGLXFBConfig(fbconfig, &native)) { DualLogError("GLX: Failed to find GLXFBConfig"); return GLFW_FALSE; }

        mask |= 0x00000001/*core profile*/;
        attribs[index++] = 0x2091/*major*/; attribs[index++] = 4; attribs[index++] = 0x2092/*minor*/; attribs[index++] = 3; // OpenGL 4.3
        if (mask) { attribs[index++] = 0x9126/*profile mask arb*/; attribs[index++] = mask; }
        if (flags) { attribs[index++] = 0x2094/*context flags arb*/; attribs[index++] = flags; }
        attribs[index++] = None; attribs[index++] = None;
        window->context.glx.handle = _glfw.glx.CreateContextAttribsARB(_glfw.x11.display,native,share,True,attribs);
        if (!window->context.glx.handle) { DualLogError("GLX: Failed to create context"); return GLFW_FALSE; }
        if (!(window->context.glx.window = glXCreateWindow(_glfw.x11.display, native, window->x11.handle, NULL))) { DualLogError("GLX: Failed to create window"); return GLFW_FALSE; }

        window->context.glx.fbconfig = native; window->context.makeCurrent = makeContextCurrentGLX;
        window->context.swapBuffers = swapBuffersGLX; window->context.swapInterval = swapIntervalGLX;
        window->context.extensionSupported = extensionSupportedGLX; window->context.getProcAddress = getProcAddressGLX;
        return GLFW_TRUE;
    }
    
    GLFWbool _glfwCreateWindowX11(_GLFWwindow* window, char* title, int width, int height, const _GLFWctxconfig* ctxconfig,const _GLFWfbconfig* fbconfig) {
        const char* names[] = {"libGLX.so.0","libGL.so.1","libGL.so",NULL};
        for (int i=0;names[i] && !_glfw.glx.handle;i++) _glfw.glx.handle = _glfwPlatformLoadModule(names[i]);
        if (!_glfw.glx.handle) { DualLogError("GLX: Failed to load GLX"); return GLFW_FALSE; }
        _glfw.glx.GetFBConfigs = (PFNGLXGETFBCONFIGSPROC)_glfwPlatformGetModuleSymbol(_glfw.glx.handle,"glXGetFBConfigs");
        _glfw.glx.GetFBConfigAttrib = (PFNGLXGETFBCONFIGATTRIBPROC)_glfwPlatformGetModuleSymbol(_glfw.glx.handle,"glXGetFBConfigAttrib");
        _glfw.glx.QueryExtension = (PFNGLXQUERYEXTENSIONPROC)_glfwPlatformGetModuleSymbol(_glfw.glx.handle,"glXQueryExtension");
        _glfw.glx.QueryVersion = (PFNGLXQUERYVERSIONPROC)_glfwPlatformGetModuleSymbol(_glfw.glx.handle,"glXQueryVersion");
        _glfw.glx.MakeCurrent = (PFNGLXMAKECURRENTPROC)_glfwPlatformGetModuleSymbol(_glfw.glx.handle,"glXMakeCurrent");
        _glfw.glx.SwapBuffers = (PFNGLXSWAPBUFFERSPROC)_glfwPlatformGetModuleSymbol(_glfw.glx.handle,"glXSwapBuffers");
        _glfw.glx.QueryExtensionsString = (PFNGLXQUERYEXTENSIONSSTRINGPROC)_glfwPlatformGetModuleSymbol(_glfw.glx.handle,"glXQueryExtensionsString");
        _glfw.glx.CreateNewContext = (PFNGLXCREATENEWCONTEXTPROC)_glfwPlatformGetModuleSymbol(_glfw.glx.handle,"glXCreateNewContext");
        _glfw.glx.CreateWindow = (PFNGLXCREATEWINDOWPROC)_glfwPlatformGetModuleSymbol(_glfw.glx.handle,"glXCreateWindow");
        _glfw.glx.GetVisualFromFBConfig = (PFNGLXGETVISUALFROMFBCONFIGPROC)_glfwPlatformGetModuleSymbol(_glfw.glx.handle,"glXGetVisualFromFBConfig");
        if (!_glfw.glx.GetFBConfigs || !_glfw.glx.GetFBConfigAttrib || !_glfw.glx.QueryExtension || !_glfw.glx.QueryVersion || !_glfw.glx.MakeCurrent || !_glfw.glx.SwapBuffers
            || !_glfw.glx.QueryExtensionsString || !_glfw.glx.CreateNewContext || !_glfw.glx.CreateWindow|| !_glfw.glx.GetVisualFromFBConfig) { DualLogError("GLX: Failed to load entry points"); return GLFW_FALSE; }
            
        _glfw.glx.GetProcAddress = (PFNGLXGETPROCADDRESSPROC)_glfwPlatformGetModuleSymbol(_glfw.glx.handle,"glXGetProcAddress");
        _glfw.glx.GetProcAddressARB = (PFNGLXGETPROCADDRESSPROC)_glfwPlatformGetModuleSymbol(_glfw.glx.handle,"glXGetProcAddressARB");
        if (!glXQueryExtension(_glfw.x11.display,&_glfw.glx.errorBase,&_glfw.glx.eventBase)) { DualLogError("GLX: Extension not found"); return GLFW_FALSE; }
        if (!glXQueryVersion(_glfw.x11.display,&_glfw.glx.major,&_glfw.glx.minor)) { DualLogError("GLX: Failed to query version"); return GLFW_FALSE; }
        
        if (extensionSupportedGLX("GLX_EXT_swap_control")) { _glfw.glx.SwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)getProcAddressGLX("glXSwapIntervalEXT"); if (_glfw.glx.SwapIntervalEXT) _glfw.glx.EXT_swap_control = GLFW_TRUE; }
        if (extensionSupportedGLX("GLX_ARB_create_context")) { _glfw.glx.CreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)getProcAddressGLX("glXCreateContextAttribsARB"); if (_glfw.glx.CreateContextAttribsARB) _glfw.glx.ARB_create_context = GLFW_TRUE; }
        if (extensionSupportedGLX("GLX_ARB_create_context_profile")) _glfw.glx.ARB_create_context_profile = GLFW_TRUE;
        GLXFBConfig native; XVisualInfo* result;
        if (!chooseGLXFBConfig(fbconfig,&native)) { DualLogError("GLX: Failed to find a suitable GLXFBConfig"); return GLFW_FALSE; }

        result = glXGetVisualFromFBConfig(_glfw.x11.display,native);
        if (!result) { DualLogError("GLX: Failed to retrieve Visual for GLXFBConfig"); return GLFW_FALSE; }

        Visual* visual=result->visual; int depth = result->depth; XFree(result);
        if (!visual) { visual=DefaultVisual(_glfw.x11.display,_glfw.x11.screen); depth=DefaultDepth(_glfw.x11.display,_glfw.x11.screen); }
        int xpos=0,ypos=0;
        window->x11.colormap=XCreateColormap(_glfw.x11.display,_glfw.x11.root,visual,0);
        XSetWindowAttributes wa={0};
        wa.colormap=window->x11.colormap;
        wa.event_mask=StructureNotifyMask|KeyPressMask|KeyReleaseMask|PointerMotionMask|ButtonPressMask|ButtonReleaseMask|ExposureMask|FocusChangeMask|VisibilityChangeMask|EnterWindowMask|LeaveWindowMask|PropertyChangeMask;
        window->x11.parent=_glfw.x11.root;
        window->x11.handle=XCreateWindow(_glfw.x11.display,_glfw.x11.root,xpos,ypos,width,height,0,depth,1/*output only*/,visual,(1L<<3)/*border pixel*/|(1L<<13)/*colormap*/|(1L<<11)/*event mask*/,&wa);
        if (!window->x11.handle) { DualLogError("X11: Failed to create window"); return GLFW_FALSE; }
        XSaveContext(_glfw.x11.display,window->x11.handle,_glfw.x11.context,(XPointer)window);
        Atom protocols[]={_glfw.x11.WM_DELETE_WINDOW,_glfw.x11.NET_WM_PING}; XSetWMProtocols(_glfw.x11.display,window->x11.handle,protocols,sizeof(protocols)/sizeof(Atom));
        const long pid=getpid();
        XChangeProperty(_glfw.x11.display,window->x11.handle,_glfw.x11.NET_WM_PID,XA_CARDINAL,32,PropModeReplace,(unsigned char*)&pid,1);
        if (_glfw.x11.NET_WM_WINDOW_TYPE && _glfw.x11.NET_WM_WINDOW_TYPE_NORMAL) { Atom type=_glfw.x11.NET_WM_WINDOW_TYPE_NORMAL;
        XChangeProperty(_glfw.x11.display,window->x11.handle,_glfw.x11.NET_WM_WINDOW_TYPE,XA_ATOM,32,PropModeReplace,(unsigned char*)&type,1); }
        XWMHints* wmhints=XAllocWMHints();
        wmhints->flags=(1L << 1)/*StateHint*/; wmhints->initial_state=1/*NormalState*/;
        XSetWMHints(_glfw.x11.display,window->x11.handle,wmhints); XFree(wmhints);
        XSizeHints* szhints=XAllocSizeHints();
        szhints->flags|=((1L << 4)/*PMinSize*/|(1L << 5)/*PMaxSize*/); szhints->min_width=szhints->max_width=width; szhints->min_height=szhints->max_height=height;
        szhints->flags|=(1L << 9)/*PWinGravity*/; szhints->win_gravity=10/*static gravity*/;
        XSetWMNormalHints(_glfw.x11.display,window->x11.handle,szhints); XFree(szhints);
        XClassHint* hint=XAllocClassHint();
        const char* resourceName=getenv("RESOURCE_NAME");
        hint->res_name=(char*)(resourceName&&GetStringLength(resourceName)?resourceName:title); hint->res_class=(char*)title;
        XSetClassHint(_glfw.x11.display,window->x11.handle,hint); XFree(hint);
        if (_glfw.x11.im) _glfwCreateInputContextX11(window);
        XChangeProperty(_glfw.x11.display,window->x11.handle,_glfw.x11.NET_WM_NAME,_glfw.x11.UTF8_STRING,8,PropModeReplace,(unsigned char*)title,GetStringLength(title)); // Set title
        XFlush(_glfw.x11.display);
        _glfwGetWindowPosX11(window,&window->x11.xpos,&window->x11.ypos);
        _glfwGetWindowSizeX11(window,&window->x11.width,&window->x11.height);
        if (!_glfwCreateContextGLX(window,ctxconfig,fbconfig)) return GLFW_FALSE;
        if (!_glfwRefreshContextAttribs(window,ctxconfig)) return GLFW_FALSE;
        _glfwShowWindowX11(window); _glfwFocusWindowX11(window); XFlush(_glfw.x11.display); return GLFW_TRUE;
    }

    #define PLATFORM_getCursorPos(w,x,y)            _glfwGetCursorPosX11(w,x,y)
    #define PLATFORM_setCursorPos(w,x,y)            _glfwSetCursorPosX11(w,x,y)
    #define PLATFORM_setCursorMode(w,m)             _glfwSetCursorModeX11(w,m)
    #define PLATFORM_setRawMouseMotion(w,e)         _glfwSetRawMouseMotionX11(w,e)
    #define PLATFORM_initJoysticks()                _glfwInitJoysticksLinux()
    #define PLATFORM_pollJoystick(js,m)             _glfwPollJoystickLinux(js,m)
    #define PLATFORM_getMappingName()               _glfwGetMappingNameLinux()
    #define PLATFORM_updateGamepadGUID(g)           _glfwUpdateGamepadGUIDLinux(g)
    #define PLATFORM_getMonitorPos(m,x,y)           _glfwGetMonitorPosX11(m,x,y)
    #define PLATFORM_getMonitorWorkarea(m,x,y,w,h)  _glfwGetMonitorWorkareaX11(m,x,y,w,h)
    #define PLATFORM_getVideoModes(m,c)             _glfwGetVideoModesX11(m,c)
    #define PLATFORM_getVideoMode(m,cur)            _glfwGetVideoModeX11(m,cur)
    #define PLATFORM_setWindowIcon(w,c,i)           _glfwSetWindowIconX11(w,c,i)
    #define PLATFORM_getWindowPos(w,x,y)            _glfwGetWindowPosX11(w,x,y)
    #define PLATFORM_setWindowPos(w,x,y)            _glfwSetWindowPosX11(w,x,y)
    #define PLATFORM_getWindowSize(w,wi,h)          _glfwGetWindowSizeX11(w,wi,h)
    #define PLATFORM_setWindowSize(w,wi,h)          _glfwSetWindowSizeX11(w,wi,h)
    #define PLATFORM_setWindowMonitor(w,x,y,wi,h) _glfwSetWindowMonitorX11(w,x,y,wi,h)
    #define PLATFORM_setWindowDecorated(w,v)        _glfwSetWindowDecoratedX11(w,v)
    #define PLATFORM_getKeyScancode(k)              _glfwGetKeyScancodeX11(k)
    #define PLATFORM_pollEvents()                   _glfwPollEventsX11()
#endif
_GLFWlibrary _glfw={0};
static _GLFWinitconfig _glfwInitHints = { .hatButtons = GLFW_TRUE };
int glfwInit(void) {
    __builtin_memset(&_glfw,0,sizeof(_glfw)); _glfw.hints.init = _glfwInitHints;
    #if defined(WINDOWS)
        if (!_glfwInitWin32()) return GLFW_FALSE;
    #else
        if (!_glfwInitX11()) return GLFW_FALSE;
    #endif
    if (!_glfwPlatformCreateMutex(&_glfw.errorLock) || !_glfwPlatformCreateTls(&_glfw.errorSlot) || !_glfwPlatformCreateTls(&_glfw.contextSlot)) return GLFW_FALSE;

    _glfwInitGamepadMappings();
    _glfwPlatformInitTimer();
    _glfw.timer.offset = _glfwPlatformGetTimerValue();
    __builtin_memset(&_glfw.hints.context, 0, sizeof(_glfw.hints.context));
    _glfw.hints.context.major = 4; _glfw.hints.context.minor = 3;
    __builtin_memset(&_glfw.hints.framebuffer,0,sizeof(_glfw.hints.framebuffer));
    _glfw.hints.framebuffer.redBits = _glfw.hints.framebuffer.greenBits = _glfw.hints.framebuffer.blueBits = _glfw.hints.framebuffer.alphaBits = _glfw.hints.framebuffer.stencilBits = 8;
    _glfw.hints.framebuffer.depthBits = 24; _glfw.hints.framebuffer.doublebuffer = GLFW_TRUE;
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
        if (missing < leastMissing || (missing == leastMissing && (colorDiff < leastColorDiff || (colorDiff == leastColorDiff && extraDiff < leastExtraDiff)))) closest = cur;
        if (cur == closest) { leastMissing = missing; leastColorDiff = colorDiff; leastExtraDiff = extraDiff; }
    }
    return closest;
}

int sscanf(const char *str, const char *format, ...);
GLFWbool _glfwRefreshContextAttribs(_GLFWwindow* window, const _GLFWctxconfig* ctxconfig) {
    window->context.source = ctxconfig->source;
    _GLFWwindow* previous = _glfwPlatformGetTls(&_glfw.contextSlot);
    glfwMakeContextCurrent((GLFWwindow*) window);
    if (_glfwPlatformGetTls(&_glfw.contextSlot) != window) return GLFW_FALSE;

    window->context.GetIntegerv = (PFNGLGETINTEGERVPROC)window->context.getProcAddress("glGetIntegerv");
    window->context.GetString = (PFNGLGETSTRINGPROC)window->context.getProcAddress("glGetString");
    if (!window->context.GetIntegerv || !window->context.GetString) { DualLogError("Entry point retrieval is broken"); glfwMakeContextCurrent((GLFWwindow*) previous); return GLFW_FALSE; }

    const char* version = (const char*) window->context.GetString(GL_VERSION);
    if (!version) { DualLogError("OpenGL version string retrieval is broken"); glfwMakeContextCurrent((GLFWwindow*) previous); return GLFW_FALSE; }
    if (!sscanf(version,"%d.%d",&window->context.major,&window->context.minor)) { DualLogError("No version found in OpenGL version string"); glfwMakeContextCurrent((GLFWwindow*) previous); return GLFW_FALSE; }
    if (window->context.major < 4 || (window->context.major == 4 && window->context.minor < 3)) { DualLogError("Requested OpenGL version 4.3, got version %i.%i", window->context.major, window->context.minor); glfwMakeContextCurrent((GLFWwindow*) previous); return GLFW_FALSE; }

    window->context.GetStringi = (PFNGLGETSTRINGIPROC) window->context.getProcAddress("glGetStringi");
    if (!window->context.GetStringi) { DualLogError("Entry point retrieval is broken"); glfwMakeContextCurrent((GLFWwindow*) previous); return GLFW_FALSE; }
    GLint flags; window->context.GetIntegerv(0x821e/*Context flags*/,&flags);
    if (window->doublebuffer) window->context.swapBuffers(window);
    glfwMakeContextCurrent((GLFWwindow*) previous);
    return GLFW_TRUE;
}

GLFWbool _glfwStringInExtensionString(const char* string, const char* extensions) {
    const char* start = extensions;
    for (;;) {
        const char* where = strstr(start, string);
        if (!where) return GLFW_FALSE;
        const char* terminator = where + GetStringLength(string);
        if ((where == start || *(where - 1) == ' ') && (*terminator == ' ' || *terminator == '\0')) break;
        start = terminator;
    }
    return GLFW_TRUE;
}

void glfwMakeContextCurrent(GLFWwindow* handle) {
    _GLFWwindow* window = (_GLFWwindow*) handle;
    _GLFWwindow* previous = _glfwPlatformGetTls(&_glfw.contextSlot);
    if (previous && (!window || window->context.source != previous->context.source)) previous->context.makeCurrent(NULL);
    if (window) window->context.makeCurrent(window);
}

void glfwSwapBuffers(GLFWwindow* handle) { _GLFWwindow* window = (_GLFWwindow*)handle; window->context.swapBuffers(window); }
void glfwSwapInterval(int interval) { _GLFWwindow* window = _glfwPlatformGetTls(&_glfw.contextSlot); window->context.swapInterval(interval); }
GLFWglproc glfwGetProcAddress(const char* procname) { _GLFWwindow* window = _glfwPlatformGetTls(&_glfw.contextSlot); return window->context.getProcAddress(procname); }
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
    if (monitor->modes) return GLFW_TRUE;
    int modeCount; GLFWvidmode* modes = PLATFORM_getVideoModes(monitor, &modeCount); if (!modes) return GLFW_FALSE;

    qsort(modes,modeCount,sizeof(GLFWvidmode),compareVideoModes);
    free(monitor->modes);
    monitor->modes = modes; monitor->modeCount = modeCount;
    return GLFW_TRUE;
}

void _glfwInputMonitor(_GLFWmonitor* monitor, int action, int placement) {
    if (action == GLFW_CONNECTED) {
        _glfw.monitorCount++;
        _glfw.monitors = _glfw.monitors ? realloc(_glfw.monitors,sizeof(_GLFWmonitor*) * _glfw.monitorCount) : calloc(_glfw.monitorCount,sizeof(_GLFWmonitor*));
        if (placement == 0) { memmove(_glfw.monitors + 1,_glfw.monitors,((size_t) _glfw.monitorCount - 1) * sizeof(_GLFWmonitor*)); _glfw.monitors[0] = monitor; }
        else _glfw.monitors[_glfw.monitorCount - 1] = monitor;
    } else if (action == GLFW_DISCONNECTED) {
        for (int i=0;i<_glfw.monitorCount;++i) {
            if (_glfw.monitors[i] == monitor) {
                _glfw.monitorCount--;
                memmove(_glfw.monitors + i, _glfw.monitors + i + 1,((size_t) _glfw.monitorCount - i) * sizeof(_GLFWmonitor*));
                break;
            }
        }
    }
}

_GLFWmonitor* _glfwAllocMonitor(const char* name, int widthMM, int heightMM) {
    _GLFWmonitor* monitor = calloc(1, sizeof(_GLFWmonitor));
    monitor->widthMM = widthMM; monitor->heightMM = heightMM;
    strncpy(monitor->name, name, sizeof(monitor->name) - 1);
    return monitor;
}

int _glfwCompareVideoModes(const GLFWvidmode* fm, const GLFWvidmode* sm) { return compareVideoModes(fm, sm); }
void _glfwSplitBPP(int bpp, int* red, int* green, int* blue) {
    int delta; if (bpp == 32) bpp = 24;
    *red = *green = *blue = bpp / 3;
    delta = bpp - (*red * 3);
    if (delta >= 1) *green = *green + 1;
    if (delta == 2) *red = *red + 1;
}

GLFWmonitor** glfwGetMonitors(int* count) { *count = 0; *count = _glfw.monitorCount; return (GLFWmonitor**) _glfw.monitors; }
GLFWmonitor* glfwGetPrimaryMonitor(void) { if (!_glfw.monitorCount) {return NULL;} return (GLFWmonitor*) _glfw.monitors[0]; }
void glfwGetMonitorPos(GLFWmonitor* handle, int* xpos, int* ypos) { if (xpos) {*xpos = 0;} if (ypos) {*ypos = 0;} _GLFWmonitor* monitor = (_GLFWmonitor*)handle; PLATFORM_getMonitorPos(monitor,xpos,ypos); }
void glfwGetMonitorWorkarea(GLFWmonitor* handle, int* xpos, int* ypos, int* width, int* height) {
    if (xpos) *xpos = 0;
    if (ypos) *ypos = 0;
    if (width) *width = 0;
    if (height) *height = 0;
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;
    PLATFORM_getMonitorWorkarea(monitor, xpos, ypos, width, height);
}

const GLFWvidmode* glfwGetVideoModes(GLFWmonitor* handle, int* count) {
    *count = 0; _GLFWmonitor* monitor = (_GLFWmonitor*) handle;
    if (!refreshVideoModes(monitor)) return NULL;
    *count = monitor->modeCount;
    return monitor->modes;
}

const GLFWvidmode* glfwGetVideoMode(GLFWmonitor* handle) {
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;
    if (!PLATFORM_getVideoMode(monitor, &monitor->currentMode)) return NULL;
    return &monitor->currentMode;
}

void SetCursorMode(GLFWwindow* handle,int value) {
    _GLFWwindow* window = (_GLFWwindow*)handle;
    if (window->cursorMode == value) return;
    window->cursorMode = value;
    PLATFORM_getCursorPos(window,&window->virtualCursorPosX,&window->virtualCursorPosY);
    PLATFORM_setCursorMode(window,value); return;
}

void _glfwInputWindowFocus(_GLFWwindow* window, GLFWbool focused) {
    Sys_Input.window_has_focus = focused != 0;
    Sys_Input.ignore_next_mouse_delta = true;
    SetCursorMode((GLFWwindow*)window,Sys_Input.window_has_focus ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    if (!focused) {
        for (int key = 0;  key <= GLFW_KEY_LAST;  key++) {
            if (window->keys[key] == GLFW_PRESS) _glfwInputKey(window,key,GLFW_RELEASE);
        }

        for (int button = 0;  button <= GLFW_MOUSE_BUTTON_LAST;  button++) {
            if (window->mouseButtons[button] == GLFW_PRESS)  _glfwInputMouseClick(window, button, GLFW_RELEASE);
        }
    }
}

GLFWwindow* glfwCreateWindow(int width, int height, char* title) {
    _GLFWfbconfig fbconfig; _GLFWctxconfig ctxconfig; _GLFWwindow* window;
    fbconfig  = _glfw.hints.framebuffer;
    ctxconfig = _glfw.hints.context;
    ctxconfig.share = NULL;
    window = calloc(1,sizeof(_GLFWwindow));
    window->next = _glfw.windowListHead;
    _glfw.windowListHead = window; window->videoMode.width=width; window->videoMode.height=height;
    window->videoMode.redBits=fbconfig.redBits; window->videoMode.greenBits=fbconfig.greenBits; window->videoMode.blueBits=fbconfig.blueBits;
    window->videoMode.refreshRate = GLFW_DONT_CARE; window->decorated = GLFW_TRUE; window->cursorMode = 0x00034003/*disabled*/; window->doublebuffer = fbconfig.doublebuffer;
    window->minwidth = window->minheight = window->maxwidth = window->maxheight = GLFW_DONT_CARE;
#ifdef WINDOWS
    if (!_glfwCreateWindowWin32(window,title,width,height,&ctxconfig,&fbconfig)) { DualLogError("glfwCreateWindowWin32 failed\n"); OS_Exit(1); }
#else
    if (!_glfwCreateWindowX11(window,title,width,height,&ctxconfig,&fbconfig)) { DualLogError("glfwCreateWindowX11 failed\n"); OS_Exit(1); }
#endif
    return (GLFWwindow*) window;
}

int glfwWindowShouldClose(GLFWwindow* handle) { _GLFWwindow* window = (_GLFWwindow*) handle; return window->shouldClose; }
void glfwSetWindowIcon(GLFWwindow* handle, int count, const GLFWimage* images) { _GLFWwindow* window = (_GLFWwindow*) handle; PLATFORM_setWindowIcon(window,count,images); }
void glfwGetWindowPos(GLFWwindow* handle, int* xpos, int* ypos) { *xpos = *ypos = 0; _GLFWwindow* window = (_GLFWwindow*)handle; PLATFORM_getWindowPos(window,xpos,ypos); }
void glfwSetWindowPos(GLFWwindow* handle, int xpos, int ypos) { _GLFWwindow* window = (_GLFWwindow*)handle; PLATFORM_setWindowPos(window,xpos,ypos); }
void glfwGetWindowSize(GLFWwindow* handle, int* width, int* height) { *width = *height = 0; _GLFWwindow* window = (_GLFWwindow*)handle; PLATFORM_getWindowSize(window,width,height); }
void glfwSetWindowSize(GLFWwindow* handle, int width, int height) { _GLFWwindow* window = (_GLFWwindow*)handle; window->videoMode.width=width; window->videoMode.height=height; PLATFORM_setWindowSize(window,width,height); }
void glfwSetWindowAttrib(GLFWwindow* handle, int attrib, int value) {
    _GLFWwindow* window = (_GLFWwindow*) handle;
    value = value ? GLFW_TRUE : GLFW_FALSE;
    switch (attrib) {
        case 0x00020005/*GLFW_DECORATED*/: window->decorated = value; PLATFORM_setWindowDecorated(window,value); return;
    }
    DualLogError("Invalid window attribute 0x%08X",attrib);
}

void glfwSetWindowMonitor(GLFWwindow* wh, int xpos, int ypos, int width, int height) {
    _GLFWwindow* window = (_GLFWwindow*) wh;
    if (width <= 0 || height <= 0) { DualLogError("Invalid window size %ix%i",width,height); return; }

    window->videoMode.width=width; window->videoMode.height=height;;
    PLATFORM_setWindowMonitor(window,xpos,ypos,width,height);
}

void glfwPollEvents(void) { PLATFORM_pollEvents(); }
static GLFWbool initJoysticks(void) {
    if (!_glfw.joysticksInitialized && !PLATFORM_initJoysticks()) return GLFW_FALSE;
    return _glfw.joysticksInitialized = GLFW_TRUE;
}

static GLFWbool isValidElementForJoystick(const _GLFWmapelement* e,const _GLFWjoystick* js) {
    if (e->type == _GLFW_JOYSTICK_HATBIT && (e->index >> 4) >= js->hatCount) return GLFW_FALSE;
    if (e->type == _GLFW_JOYSTICK_BUTTON && e->index >= js->buttonCount) return GLFW_FALSE;
    if (e->type == _GLFW_JOYSTICK_AXIS && e->index >= js->axisCount) return GLFW_FALSE;
    return GLFW_TRUE;
}

static _GLFWmapping* findValidMapping(const _GLFWjoystick* js) {
    _GLFWmapping* mapping = NULL;
    for (int i = 0; i < _glfw.mappingCount; i++) { if (strcmp(_glfw.mappings[i].guid,js->guid) == 0) {mapping = _glfw.mappings + i; break;} }
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
    if ((length = strcspn(c,",")) != 32 || c[length] != ',') return GLFW_FALSE;
    __builtin_memcpy(mapping->guid,c,length); c += length + 1;
    if ((length = strcspn(c,",")) >= sizeof(mapping->name) || c[length] != ',') return GLFW_FALSE;
    __builtin_memcpy(mapping->name,c,length); c += length + 1;
    while (*c) {
        if (*c == '+' || *c == '-') return GLFW_FALSE;
        for (i = 0; i < sizeof(fields)/sizeof(fields[0]); i++) {
            length = GetStringLength(fields[i].name);
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
                    e->axisScale = 2/(maximum - minimum); e->axisOffset = -(maximum + minimum);
                    if (*c == '~') { e->axisScale = -e->axisScale; e->axisOffset = -e->axisOffset; }
                }
            } else {
                const char* name = PLATFORM_getMappingName();
                if (strncmp(c,name,GetStringLength(name)) != 0) return GLFW_FALSE;
            }
            break;
        }
        c += strcspn(c,","); c += strspn(c,",");
    }
    for (i = 0; i < 32; i++) { if (mapping->guid[i] >= 'A' && mapping->guid[i] <= 'F') mapping->guid[i] += 'a'-'A'; }
    PLATFORM_updateGamepadGUID(mapping->guid);
    return GLFW_TRUE;
}

void TextEntry(i32 k) {
    if (k == GLFW_KEY_U && Sys_Input.keyStates[GLFW_KEY_LEFT_CONTROL].down) { Sys_Global.playerName[0] = '\0'; currentPlayerNameLength = 0; return; }
    if (k == GLFW_KEY_ENTER || k == GLFW_KEY_KP_ENTER) { currentMenuItem++; return; }
    if (k == GLFW_KEY_BACKSPACE && currentPlayerNameLength > 0) { Sys_Global.playerName[--currentPlayerNameLength] = '\0'; return; }
    if (currentPlayerNameLength >= 26) return;
    char c = (k >= GLFW_KEY_A && k <= GLFW_KEY_Z) ? 'a' + (k - GLFW_KEY_A) : ((k >= GLFW_KEY_1 && k <= GLFW_KEY_9) ? '1' + (k - GLFW_KEY_1) : ((k == GLFW_KEY_0) ? '0' : ((k == GLFW_KEY_SPACE) ? ' ' : 0)));
    if (c) { Sys_Global.playerName[currentPlayerNameLength] = c; Sys_Global.playerName[++currentPlayerNameLength] = '\0'; }
}

bool IsNonRepeatingKey(i32 key) { return key == GLFW_KEY_KP_ENTER || key == GLFW_KEY_ENTER || key == GLFW_KEY_TAB || key == GLFW_KEY_ESCAPE; }
void GoIntoGame(void); void ConsoleEmulator(i32 keycode); extern bool enteringPlayerName;
void _glfwInputKey(_GLFWwindow* window,int key,int action) {
    if (key >= 0 && key <= GLFW_KEY_LAST) {
        GLFWbool repeated = GLFW_FALSE;
        if (action == GLFW_RELEASE && window->keys[key] == GLFW_RELEASE) return;
        if (action == GLFW_PRESS && window->keys[key] == GLFW_PRESS) repeated = GLFW_TRUE;
        window->keys[key] = (char)action;
        if (repeated) action = GLFW_REPEAT;
    }

    if (!Sys_Input.window_has_focus) return;
    
    if (key == GLFW_KEY_F10 && action) OS_Exit(0); // Suppress warnings about unused parameters forced upon me by glfw3 dependency deadweight anchor.
    if (Sys_Global.menuActive && !returnToPause) {
        if ((key == GLFW_KEY_RIGHT_ALT || key == GLFW_KEY_LEFT_ALT) && action && Sys_Input.keyStates[GLFW_KEY_ENTER].down)                    GoIntoGame();
        if (key == GLFW_KEY_ENTER && action && (Sys_Input.keyStates[GLFW_KEY_LEFT_ALT].down || Sys_Input.keyStates[GLFW_KEY_RIGHT_ALT].down)) GoIntoGame();
    }

    if (key >=0 && key < MAX_KEYS && (action == GLFW_PRESS || (action == GLFW_REPEAT && !IsNonRepeatingKey(key)))) {
        Sys_Input.keyStates[key].pressed = Sys_Input.keyStates[key].down = true;
        if (Sys_Cheats.consoleActive) ConsoleEmulator(key);
        else if (enteringPlayerName && Sys_Global.menuActive) TextEntry(key);
    } else if (key >= 0 && key < MAX_KEYS && action == GLFW_RELEASE) Sys_Input.keyStates[key].pressed = Sys_Input.keyStates[key].down = false;
}

void _glfwInputMouseClick(_GLFWwindow* window,int button,int action) {
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return;
    if (button <= GLFW_MOUSE_BUTTON_LAST) window->mouseButtons[button] = (char)action;
    Sys_Input.mouseButtons[button].down = Sys_Input.mouseButtons[button].pressed = (action == GLFW_PRESS);
    Sys_Input.mouseButtons[button].released = (action == GLFW_RELEASE);
}

// Create a quaternion from yaw (around Y), pitch (around X), and roll (around Z) in degrees
void quat_from_yaw_pitch_roll(Quaternion* q, float yaw_deg, float pitch_deg, float roll_deg) {
    float yaw = deg2rad(yaw_deg), pitch = deg2rad(pitch_deg), roll = deg2rad(roll_deg);  // Around Z (forward)
    float cy = vcosf(yaw * 0.5f), sy = vsinf(yaw * 0.5f), cp = vcosf(pitch * 0.5f), sp = vsinf(pitch * 0.5f), cr = vcosf(roll * 0.5f), sr = vsinf(roll * 0.5f);
    q->w = cy * cp * cr + sy * sp * sr;
    q->x = cy * sp * cr + sy * cp * sr; // X-axis (pitch)
    q->y = sy * cp * cr - cy * sp * sr; // Y-axis (yaw)
    q->z = cy * cp * sr - sy * sp * cr; // Z-axis (roll)
} // Skipping quat normalization, not needed

void _glfwInputCursorPos(_GLFWwindow* window,double xpos,double ypos) {
    if (window->virtualCursorPosX == xpos && window->virtualCursorPosY == ypos) return;
    window->virtualCursorPosX = xpos; window->virtualCursorPosY = ypos;
    if (Sys_Input.window_has_focus) {
        Sys_Input.currentMouse_dx = (i32)(xpos - Sys_Input.last_mouse_x);
        Sys_Input.currentMouse_dy = (i32)(ypos - Sys_Input.last_mouse_y);
        Sys_Input.last_mouse_x = xpos; Sys_Input.last_mouse_y = ypos;
        if (Sys_Input.ignore_next_mouse_delta) { Sys_Input.ignore_next_mouse_delta = mouseMovementThisFrame = false; return; }
        
        // static const float HeadBobRate   = 0.2f, HeadBobAmount = 0.08f,bobTarget = 0.3f; TODO
        if ((Sys_Global.inventoryMode && !Sys_Cheats.noHUD) || Sys_Global.menuActive || Sys_Global.gamePaused) { // Uses UI baseline resolution 1366x768
            i32 newX = clamp(Sys_Global.cursorPosition_x + Sys_Input.currentMouse_dx,0,1366); if (newX != Sys_Global.cursorPosition_x) {mouseMovementThisFrame = true;} Sys_Global.cursorPosition_x = newX;
            i32 newY = clamp(Sys_Global.cursorPosition_y + Sys_Input.currentMouse_dy,0,768);  if (newY != Sys_Global.cursorPosition_y) {mouseMovementThisFrame = true;} Sys_Global.cursorPosition_y = newY;
        }
        
        if (Sys_Global.gamePaused || Sys_Global.menuActive || Sys_Global.inventoryMode) return;
        
        float sensitivity = vclamp((float)Sys_Settings.MouseSensitivity / 100.0f, 0.01f, 1.0f) * 0.2f;
        cam_yaw += (float)Sys_Input.currentMouse_dx * sensitivity;
        if (cam_yaw >= 360.0f) cam_yaw -= 360.0f;
        if (cam_yaw < 0.0f) cam_yaw += 360.0f;
        cam_pitch += (float)Sys_Input.currentMouse_dy * sensitivity;
        if (cam_pitch > 89.0f) cam_pitch = 89.0f; // Avoid gimbal lock at pure 90deg
        if (cam_pitch < -89.0f) cam_pitch = -89.0f;
        quat_from_yaw_pitch_roll(&Sys_Global.instances[PLAYER1].rotation,cam_yaw,cam_pitch,(Sys_Global.currentLevel == LEVEL_CYBERSPACE) ? cam_roll : 0.0f);
        Quaternion rot = Sys_Global.instances[PLAYER1].rotation;
        float y2 = rot.y * rot.y;  float xz = rot.x * rot.z;  float wy = rot.w * rot.y;
        Sys_Global.instances[PLAYER1].forward = normalize_vector3((Vector3){ 2.0f * (xz + wy), 2.0f * (rot.y * rot.z - rot.w * rot.x), 1.0f - 2.0f * (rot.x * rot.x + y2) });
        Sys_Global.instances[PLAYER1].right = normalize_vector3((Vector3){ 1.0f - 2.0f * (y2 + rot.z * rot.z), 2.0f * (rot.x * rot.y + rot.w * rot.z), 2.0f * (xz - wy) });
        ma_engine_listener_set_direction(&audio_engine,0,Sys_Global.instances[PLAYER1].forward.x,Sys_Global.instances[PLAYER1].forward.y,Sys_Global.instances[PLAYER1].forward.z);
        Vector3 up = cross_vector3(Sys_Global.instances[PLAYER1].forward,Sys_Global.instances[PLAYER1].right);
        ma_engine_listener_set_world_up(&audio_engine,0,up.x,up.y,up.z);
    }
}

void _glfwInputJoystick(_GLFWjoystick* js,int event) {
    if (event == GLFW_CONNECTED) js->connected = GLFW_TRUE;
    else if (event == GLFW_DISCONNECTED) js->connected = GLFW_FALSE;
    
    int jid = (int)(js - _glfw.joysticks);
    if (jid > GLFW_JOYSTICK_LAST) return;
    bool connected = (event == GLFW_CONNECTED);
    Sys_Input.joystickPresent[jid] = connected;
    if (!connected) { __builtin_memset(Sys_Input.joystickButtons,0,sizeof(Sys_Input.joystickButtons)); __builtin_memset(Sys_Input.joystickHats,0,sizeof(Sys_Input.joystickHats)); } // Clear
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

void _glfwFreeJoystick(_GLFWjoystick* js) { free(js->axes); free(js->buttons); free(js->hats); __builtin_memset(js,0,sizeof(_GLFWjoystick)); }
int glfwJoystickPresent(int jid) {
    if (jid < 0 || jid > GLFW_JOYSTICK_LAST) return GLFW_FALSE;
    if (!initJoysticks()) return GLFW_FALSE;
    _GLFWjoystick* js = _glfw.joysticks + jid;
    return js->connected ? PLATFORM_pollJoystick(js,0/*presence*/) : GLFW_FALSE;
}

const unsigned char* glfwGetJoystickButtons(int jid,int* count) {
    *count = 0; if (jid < 0 || jid > GLFW_JOYSTICK_LAST) return NULL;
    if (!initJoysticks()) return NULL;
    _GLFWjoystick* js = _glfw.joysticks + jid; if (!js->connected || !PLATFORM_pollJoystick(js,2/*buttons*/)) return NULL;
    *count = _glfw.hints.init.hatButtons ? js->buttonCount + js->hatCount * 4 : js->buttonCount; return js->buttons;
}

int glfwGetGamepadState(int jid,GLFWgamepadstate* state) {
    __builtin_memset(state,0,sizeof(GLFWgamepadstate));
    if (jid < 0 || jid > GLFW_JOYSTICK_LAST) return GLFW_FALSE;
    if (!initJoysticks()) return GLFW_FALSE;
    _GLFWjoystick* js = _glfw.joysticks + jid;
    if (!js->connected || !PLATFORM_pollJoystick(js,(1/*axes*/ | 2/*buttons*/)) || !js->mapping) return GLFW_FALSE;
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
