// glfw.c - Heavily reduced glfw for only Windows and Linux X11
// GLFW 3.5 This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
#define GLFW_CURSOR_DISABLED 0x00034003
int sprintf(char *str, const char *format, ...); char *strstr(const char *haystack, const char *needle); int strcmp(const char *s1, const char *s2);
char *strncpy(char *dest, const char *src, size_t n); void *memmove(void *dest, const void *src, size_t n);
void free(void *ptr); void *calloc(size_t nmemb, size_t size); void *realloc(void *ptr, size_t size); void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *)); char *getenv(const char *name);
typedef int GLFWbool; typedef void (*GLFWproc)(void);
typedef struct _GLFWfbconfig _GLFWfbconfig; typedef struct _GLFWcontext _GLFWcontext; typedef struct _GLFWwindow _GLFWwindow; typedef struct _GLFWlibrary _GLFWlibrary; typedef struct _GLFWmonitor _GLFWmonitor; typedef struct _GLFWjoystick _GLFWjoystick; typedef struct _GLFWtls _GLFWtls;
#if defined(WINDOWS)
    #define IsWindows8OrGreater()                              \
        _glfwIsWindowsVersionOrGreaterWin32(HIBYTE(0x0602),LOBYTE(0x0602), 0)
    #define IsWindows8Point1OrGreater()                     \
        _glfwIsWindowsVersionOrGreaterWin32(HIBYTE(0x0603),LOBYTE(0x0603), 0)
    typedef DWORD (WINAPI * PFN_XInputGetCapabilities)(DWORD,DWORD,XINPUT_CAPABILITIES*);
    typedef DWORD (WINAPI * PFN_XInputGetState)(DWORD,XINPUT_STATE*);
    #define XInputGetCapabilities _glfw.win32.xinput.GetCapabilities
    #define XInputGetState _glfw.win32.xinput.GetState
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
    #define GLFW_WGL_CONTEXT_STATE          _GLFWcontextWGL wgl;
    #define GLFW_WGL_LIBRARY_CONTEXT_STATE  _GLFWlibraryWGL wgl;
    typedef struct _GLFWcontextWGL { HDC dc; HGLRC handle; int interval; } _GLFWcontextWGL;
    typedef struct _GLFWlibraryWGL { HINSTANCE instance; PFN_wglCreateContext CreateContext; PFN_wglGetProcAddress GetProcAddress; PFN_wglGetCurrentDC GetCurrentDC; PFN_wglGetCurrentContext GetCurrentContext; PFN_wglMakeCurrent MakeCurrent; PFN_wglShareLists ShareLists; PFNWGLSWAPINTERVALEXTPROC SwapIntervalEXT; PFNWGLGETPIXELFORMATATTRIBIVARBPROC GetPixelFormatAttribivARB; PFNWGLGETEXTENSIONSSTRINGEXTPROC GetExtensionsStringEXT; PFNWGLGETEXTENSIONSSTRINGARBPROC GetExtensionsStringARB; PFNWGLCREATECONTEXTATTRIBSARBPROC CreateContextAttribsARB; GLFWbool EXT_swap_control,ARB_create_context,ARB_create_context_profile; } _GLFWlibraryWGL;
    typedef struct _GLFWwindowWin32 { HWND handle; GLFWbool cursorTracked,frameAction,keymenu; int width,height,lastCursorPosX,lastCursorPosY; } _GLFWwindowWin32;
    typedef struct _GLFWlibraryWin32 {
        HINSTANCE           instance;
        HWND                helperWindowHandle;
        ATOM helperWindowClass,mainWindowClass;
        HDEVNOTIFY          deviceNotificationHandle;
        short int           keycodes[512],scancodes[GLFW_KEY_LAST + 1];
        double              restoreCursorPosX, restoreCursorPosY;
        _GLFWwindow *disabledCursorWindow, *capturedCursorWindow;
        HCURSOR blankCursor;
        struct { HINSTANCE instance; PFN_XInputGetCapabilities GetCapabilities; PFN_XInputGetState GetState; } xinput;
        struct { HINSTANCE instance; PFN_DwmIsCompositionEnabled IsCompositionEnabled; PFN_DwmFlush Flush; } dwmapi;
        struct { HINSTANCE instance; PFN_RtlVerifyVersionInfo RtlVerifyVersionInfo_; } ntdll;
    } _GLFWlibraryWin32;
    typedef struct _GLFWmonitorWin32 { HMONITOR handle; WCHAR adapterName[32],displayName[32]; char publicAdapterName[32],publicDisplayName[32]; GLFWbool modesPruned,modeChanged; } _GLFWmonitorWin32;
    WCHAR* _glfwCreateWideStringFromUTF8Win32(const char* source);
    BOOL _glfwIsWindowsVersionOrGreaterWin32(WORD major, WORD minor, WORD sp);
    void _glfwPollMonitorsWin32(void);
    void _glfwGetWindowSizeWin32(_GLFWwindow* window, int* width, int* height);
    void _glfwGetCursorPosWin32(_GLFWwindow* window, double* xpos, double* ypos);
    void _glfwSetCursorPosWin32(_GLFWwindow* window, double xpos, double ypos);
    void _glfwGetVideoModeWin32(_GLFWmonitor* monitor, GLFWvidmode* mode);
    #define GLFW_X11_WINDOW_STATE
    #define GLFW_X11_MONITOR_STATE
    #define GLFW_X11_LIBRARY_WINDOW_STATE
    #define GLFW_GLX_CONTEXT_STATE
    #define GLFW_GLX_LIBRARY_CONTEXT_STATE
    #define GLFW_WIN32_JOYSTICK_STATE _GLFWjoystickWin32 win32;
    #define GLFW_WIN32_LIBRARY_JOYSTICK_STATE
    typedef struct _GLFWjoystickWin32{ int objectCount; DWORD index; GUID guid; } _GLFWjoystickWin32;
    void _glfwDetectJoystickConnectionWin32(void);
    void _glfwDetectJoystickDisconnectionWin32(void);
    #define GLFW_LINUX_JOYSTICK_STATE
    #define GLFW_LINUX_LIBRARY_JOYSTICK_STATE
    #define GLFW_WIN32_TLS_STATE _GLFWtlsWin32 win32;
    typedef struct _GLFWtlsWin32 { GLFWbool allocated; DWORD index; } _GLFWtlsWin32;
    typedef struct _GLFWmutexWin32 { GLFWbool allocated; CRITICAL_SECTION section; } _GLFWmutexWin32;
    #define GLFW_PLATFORM_TLS_STATE GLFW_WIN32_TLS_STATE
    #define GLFW_WIN32_LIBRARY_TIMER_STATE  _GLFWtimerWin32 win32;
    typedef struct _GLFWtimerWin32 { u64 frequency; } _GLFWtimerWin32;
    #define GLFW_PLATFORM_LIBRARY_TIMER_STATE GLFW_WIN32_LIBRARY_TIMER_STATE
#else // LINUX
    #define GLFW_WIN32_WINDOW_STATE
    #define GLFW_WIN32_MONITOR_STATE
    #define GLFW_WIN32_LIBRARY_WINDOW_STATE
    #define GLFW_WGL_CONTEXT_STATE
    #define GLFW_WGL_LIBRARY_CONTEXT_STATE
    typedef unsigned char KeyCode; typedef int Bool; typedef unsigned long Atom; typedef unsigned long KeySym;
    typedef char *XPointer;
    typedef unsigned int XcursorUInt; typedef struct _XcursorImage { XcursorUInt version; XcursorUInt size,width,height,xhot,yhot; XcursorUInt delay; XcursorUInt *pixels; } XcursorImage;
    typedef unsigned short Rotation,SubpixelOrder,Connection;
    typedef struct { long flags; int x,y, width,height,min_width,min_height,max_width,max_height,width_inc,height_inc; struct {int x; int y;} min_aspect,max_aspect; int base_width, base_height; int win_gravity; } XSizeHints;
    typedef unsigned long XID,Mask,Atom,VisualID,Time;
    typedef XID Window,Drawable,Font,Pixmap,Cursor,Colormap;
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
    #ifdef __clang__
        #pragma clang diagnostic pop
    #endif
    typedef struct { Visual *visual; VisualID visualid; int screen; int depth; int class; unsigned long red_mask; unsigned long green_mask; unsigned long blue_mask; int colormap_size; int bits_per_rgb; } XVisualInfo;
    typedef int XContext;
    typedef XID RROutput,RRCrtc,RRMode;
    typedef unsigned long XRRModeFlags;
    typedef struct _XRRModeInfo { RRMode id; unsigned int width; unsigned int height; unsigned long dotClock; unsigned int hSyncStart; unsigned int hSyncEnd; unsigned int hTotal; unsigned int hSkew; unsigned int vSyncStart; unsigned int vSyncEnd; unsigned int vTotal; char *name; unsigned int nameLength; XRRModeFlags modeFlags; } XRRModeInfo;
    typedef struct _XRRScreenResources { Time timestamp; Time configTimestamp; int ncrtc; RRCrtc *crtcs; int noutput; RROutput *outputs; int nmode; XRRModeInfo *modes; } XRRScreenResources;
    typedef struct _XRROutputInfo { Time timestamp; RRCrtc crtc; char *name; int nameLen; unsigned long mm_width; unsigned long mm_height; Connection connection; SubpixelOrder subpixel_order; int ncrtc; RRCrtc *crtcs; int nclone; RROutput *clones; int nmode; int npreferred; RRMode *modes; } XRROutputInfo;
    typedef struct _XRRCrtcInfo { Time timestamp; int x, y; unsigned int width, height; RRMode mode; Rotation rotation; int noutput; RROutput *outputs; Rotation rotations; int npossible; RROutput *possible; } XRRCrtcInfo;
    typedef XID GLXWindow,GLXDrawable;
    typedef struct __GLXFBConfig* GLXFBConfig;
    typedef struct __GLXcontext* GLXContext;
    typedef void (*__GLXextproc)(void);
    typedef XSizeHints* (* PFN_XAllocSizeHints)(void);
    typedef int (* PFN_XChangeProperty)(Display*,Window,Atom,Atom,int,int,const unsigned char*,int);
    typedef int (* PFN_XChangeWindowAttributes)(Display*,Window,unsigned long,XSetWindowAttributes*);
    typedef Bool (* PFN_XCheckTypedWindowEvent)(Display*,Window,int,XEvent*);
    typedef Colormap (* PFN_XCreateColormap)(Display*,Window,Visual*,int);
    typedef Window (* PFN_XCreateWindow)(Display*,Window,int,int,unsigned int,unsigned int,unsigned int,int,unsigned int,Visual*,unsigned long,XSetWindowAttributes*);
    typedef int (* PFN_XDefineCursor)(Display*,Window,Cursor);
    typedef int (* PFN_XDeleteProperty)(Display*,Window,Atom);
    typedef int (* PFN_XDisplayKeycodes)(Display*,int*,int*);
    typedef Bool (* PFN_XFilterEvent)(XEvent*,Window);
    typedef int (* PFN_XFindContext)(Display*,XID,XContext,XPointer*);
    typedef int (* PFN_XFree)(void*);
    typedef void (* PFN_XFreeEventData)(Display*,XGenericEventCookie*);
    typedef int (* PFN_XGetInputFocus)(Display*,Window*,int*);
    typedef KeySym* (* PFN_XGetKeyboardMapping)(Display*,KeyCode,int,int*);
    typedef Status (* PFN_XGetWMNormalHints)(Display*,Window,XSizeHints*,long*);
    typedef Status (* PFN_XGetWindowAttributes)(Display*,Window,XWindowAttributes*);
    typedef int (* PFN_XGetWindowProperty)(Display*,Window,Atom,long,long,Bool,Atom,Atom*,int*,unsigned long*,unsigned long*,unsigned char**);
    typedef int (* PFN_XGrabPointer)(Display*,Window,Bool,unsigned int,int,int,Window,Cursor,Time);
    typedef Atom (* PFN_XInternAtom)(Display*,const char*,Bool);
    typedef int (* PFN_XMapWindow)(Display*,Window);
    typedef int (* PFN_XMoveResizeWindow)(Display*,Window,int,int,unsigned int,unsigned int);
    typedef int (* PFN_XMoveWindow)(Display*,Window,int,int);
    typedef Status (* PFN_XInitThreads)(void);
    typedef int (* PFN_XNextEvent)(Display*,XEvent*);
    typedef Display* (* PFN_XOpenDisplay)(const char*);
    typedef int (* PFN_XPending)(Display*);
    typedef Bool (* PFN_XQueryExtension)(Display*,const char*,int*,int*,int*);
    typedef Bool (* PFN_XQueryPointer)(Display*,Window,Window*,Window*,int*,int*,int*,int*,unsigned int*);
    typedef int (* PFN_XRaiseWindow)(Display*,Window);
    typedef int (* PFN_XResizeWindow)(Display*,Window,unsigned int,unsigned int);
    typedef int (* PFN_XSaveContext)(Display*,XID,XContext,const char*);
    typedef Status (* PFN_XSendEvent)(Display*,Window,Bool,long,XEvent*);
    typedef void (* PFN_XSetICFocus)(XIC);
    typedef int (* PFN_XSetInputFocus)(Display*,Window,int,Time);
    typedef void (* PFN_XSetWMNormalHints)(Display*,Window,XSizeHints*);
    typedef Status (* PFN_XSetWMProtocols)(Display*,Window,Atom*,int);
    typedef Bool (* PFN_XTranslateCoordinates)(Display*,Window,Window,int,int,int*,int*,Window*);
    typedef int (* PFN_XUndefineCursor)(Display*,Window);
    typedef int (* PFN_XUngrabPointer)(Display*,Time);
    typedef void (* PFN_XUnsetICFocus)(XIC);
    typedef int (* PFN_XWarpPointer)(Display*,Window,Window,int,int,unsigned int,unsigned int,int,int);
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
    typedef int (*PFNGLXGETFBCONFIGATTRIBPROC)(Display*,GLXFBConfig,int,int*);
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
    #define GLFW_X11_WINDOW_STATE           _GLFWwindowX11 x11;
    #define GLFW_X11_LIBRARY_WINDOW_STATE   _GLFWlibraryX11 x11;
    #define GLFW_X11_MONITOR_STATE          _GLFWmonitorX11 x11;
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
        Atom NET_SUPPORTED,NET_SUPPORTING_WM_CHECK,WM_PROTOCOLS,WM_STATE,WM_DELETE_WINDOW,NET_WM_NAME,NET_WM_ICON,NET_WM_PING,NET_WM_WINDOW_TYPE,NET_WM_WINDOW_TYPE_NORMAL,NET_WM_STATE,NET_WM_STATE_FULLSCREEN,NET_WM_BYPASS_COMPOSITOR,NET_WORKAREA,NET_CURRENT_DESKTOP,NET_ACTIVE_WINDOW,MOTIF_WM_HINTS,UTF8_STRING;
        struct {
            void*       handle;
            GLFWbool    utf8;
            PFN_XAllocSizeHints AllocSizeHints;
            PFN_XChangeProperty ChangeProperty;
            PFN_XChangeWindowAttributes ChangeWindowAttributes;
            PFN_XCheckTypedWindowEvent CheckTypedWindowEvent;
            PFN_XCreateColormap CreateColormap;
            PFN_XCreateWindow CreateWindow;
            PFN_XDefineCursor DefineCursor;
            PFN_XDeleteProperty DeleteProperty;
            PFN_XDisplayKeycodes DisplayKeycodes;
            PFN_XFilterEvent FilterEvent;
            PFN_XFindContext FindContext;
            PFN_XFree Free;
            PFN_XFreeEventData FreeEventData;
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
            PFN_XResizeWindow ResizeWindow;
            PFN_XSaveContext SaveContext;
            PFN_XSendEvent SendEvent;
            PFN_XSetICFocus SetICFocus;
            PFN_XSetInputFocus SetInputFocus;
            PFN_XSetWMNormalHints SetWMNormalHints;
            PFN_XSetWMProtocols SetWMProtocols;
            PFN_XTranslateCoordinates TranslateCoordinates;
            PFN_XUndefineCursor UndefineCursor;
            PFN_XUngrabPointer UngrabPointer;
            PFN_XUnsetICFocus UnsetICFocus;
            PFN_XWarpPointer WarpPointer;
        } xlib;
        struct { void* handle; int eventBase,errorBase,major,minor; PFN_XRRFreeCrtcInfo FreeCrtcInfo; PFN_XRRFreeOutputInfo FreeOutputInfo; PFN_XRRFreeScreenResources FreeScreenResources; PFN_XRRGetCrtcInfo GetCrtcInfo; PFN_XRRGetOutputInfo GetOutputInfo; PFN_XRRGetOutputPrimary GetOutputPrimary; PFN_XRRGetScreenResourcesCurrent GetScreenResourcesCurrent; PFN_XRRQueryExtension QueryExtension; PFN_XRRQueryVersion QueryVersion; PFN_XRRSelectInput SelectInput; PFN_XRRSetCrtcConfig SetCrtcConfig; PFN_XRRUpdateConfiguration UpdateConfiguration; } randr;
        struct { void* handle; PFN_XcursorImageCreate ImageCreate; PFN_XcursorImageDestroy ImageDestroy; PFN_XcursorImageLoadCursor ImageLoadCursor; } xcursor;
    } _GLFWlibraryX11;
    typedef struct _GLFWmonitorX11 { RROutput output; RRCrtc crtc; int index; } _GLFWmonitorX11;
    GLFWbool _glfwWindowVisibleX11(_GLFWwindow* window);
    void _glfwSetWindowDecoratedX11(_GLFWwindow* window, GLFWbool enabled);
    void _glfwGetCursorPosX11(_GLFWwindow* window, double* xpos, double* ypos);
    void _glfwSetCursorPosX11(_GLFWwindow* window, double xpos, double ypos);
    void _glfwGetVideoModeX11(_GLFWmonitor* monitor, GLFWvidmode* mode);
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
    #define GLFW_POSIX_LIBRARY_TIMER_STATE _GLFWtimerPOSIX posix;
    typedef struct _GLFWtimerPOSIX { clockid_t clock; u64 frequency; } _GLFWtimerPOSIX;
    #define GLFW_PLATFORM_LIBRARY_TIMER_STATE  GLFW_POSIX_LIBRARY_TIMER_STATE
#endif
#define GLFW_PLATFORM_WINDOW_STATE \
    GLFW_WIN32_WINDOW_STATE \
    GLFW_X11_WINDOW_STATE
struct _GLFWfbconfig { int redBits,greenBits,blueBits,alphaBits,depthBits,stencilBits,accumRedBits,accumGreenBits,accumBlueBits,accumAlphaBits; GLFWbool stereo; int samples; GLFWbool sRGB,doublebuffer; uintptr_t handle; };
struct _GLFWcontext {
    int client,source,major,minor;
    PFNGLGETINTEGERVPROC GetIntegerv;
    void (*makeCurrent)(_GLFWwindow*);
    void (*swapBuffers)(_GLFWwindow*);
    void (*swapInterval)(int);
    int (*extensionSupported)(const char*);
    GLFWglproc (*getProcAddress)(const char*);
    GLFW_WGL_CONTEXT_STATE
    GLFW_GLX_CONTEXT_STATE
};

struct _GLFWwindow {
    struct _GLFWwindow* next;
    GLFWbool decorated,shouldClose,doublebuffer;
    GLFWvidmode videoMode;
    int minwidth,minheight,maxwidth,maxheight;
    int cursorMode;
    char mouseButtons[GLFW_MOUSE_BUTTON_LAST + 1],keys[GLFW_KEY_LAST + 1];
    double virtualCursorPosX,virtualCursorPosY;
    _GLFWcontext context;
    GLFW_PLATFORM_WINDOW_STATE
};

struct _GLFWmonitor { char name[128]; int widthMM,heightMM; GLFWvidmode currentMode; GLFW_WIN32_MONITOR_STATE GLFW_X11_MONITOR_STATE };
struct _GLFWjoystick {
    GLFWbool allocated,connected;
    float*  axes;
    int axisCount;
    unsigned char* buttons;
    int buttonCount;
    unsigned char* hats;
    int hatCount;
    char name[128],guid[33];
    #if defined(LINUX)
        GLFW_LINUX_JOYSTICK_STATE
    #else
        GLFW_WIN32_JOYSTICK_STATE
    #endif
};

struct _GLFWtls { GLFW_PLATFORM_TLS_STATE };
struct _GLFWlibrary { _GLFWwindow* windowListHead; _GLFWmonitor** monitors; int monitorCount; GLFWbool joysticksInitialized; _GLFWjoystick joysticks[GLFW_JOYSTICK_LAST + 1]; _GLFWtls errorSlot,contextSlot; struct { u64 offset; GLFW_PLATFORM_LIBRARY_TIMER_STATE } timer; GLFW_WIN32_LIBRARY_WINDOW_STATE GLFW_X11_LIBRARY_WINDOW_STATE GLFW_WGL_LIBRARY_CONTEXT_STATE GLFW_GLX_LIBRARY_CONTEXT_STATE GLFW_WIN32_LIBRARY_JOYSTICK_STATE GLFW_LINUX_LIBRARY_JOYSTICK_STATE };
extern _GLFWlibrary _glfw;
void _glfwPlatformInitTimer(void);
u64 _glfwPlatformGetTimerValue(void);
u64 _glfwPlatformGetTimerFrequency(void);
void* _glfwPlatformGetTls(_GLFWtls* tls);
void _glfwPlatformSetTls(_GLFWtls* tls, void* value);
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
const _GLFWfbconfig* _glfwChooseFBConfig(const _GLFWfbconfig* alternatives, unsigned int count);
_GLFWmonitor* _glfwAllocMonitor(const char* name, int widthMM, int heightMM);
_GLFWjoystick* _glfwAllocJoystick(const char* name, const char* guid, int axisCount, int buttonCount, int hatCount);
void _glfwFreeJoystick(_GLFWjoystick* js);
#if defined(WINDOWS)
    #include <wchar.h>
    static DWORD getWindowStyle(const _GLFWwindow* window) { return (WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_SYSMENU | WS_MINIMIZEBOX) | (window->decorated ? WS_CAPTION : WS_POPUP); }
    static HICON createIcon(const GLFWimage* image,int xhot,int yhot,GLFWbool icon) {
        HDC dc; HICON handle; HBITMAP color,mask; BITMAPV5HEADER bi; ICONINFO ii;
        unsigned char* target=NULL; unsigned char* source=image->pixels;
        ZeroMemory(&bi,sizeof(bi));
        bi.bV5Size=sizeof(bi); bi.bV5Width=image->width; bi.bV5Height=-image->height; bi.bV5Planes=1; bi.bV5BitCount=32; bi.bV5Compression=BI_BITFIELDS; bi.bV5RedMask=0x00ff0000; bi.bV5GreenMask=0x0000ff00; bi.bV5BlueMask=0x000000ff; bi.bV5AlphaMask=0xff000000;
        dc=GetDC(NULL);
        color=CreateDIBSection(dc,(BITMAPINFO*)&bi,DIB_RGB_COLORS,(void**)&target,NULL,(DWORD)0);
        ReleaseDC(NULL,dc);
        mask=CreateBitmap(image->width,image->height,1,1,NULL);
        for (int i=0;i<image->width*image->height;i++) { target[0]=source[2]; target[1]=source[1]; target[2]=source[0]; target[3]=source[3]; target+=4; source+=4; }
        ZeroMemory(&ii,sizeof(ii));
        ii.fIcon=icon; ii.xHotspot=xhot; ii.yHotspot=yhot; ii.hbmMask=mask; ii.hbmColor=color;
        handle=CreateIconIndirect(&ii); DeleteObject(color); DeleteObject(mask); return handle;
    }

    static void updateCursorImage(_GLFWwindow* window) { if (window->cursorMode==0x00034001/*GLFW_CURSOR_NORMAL*/) {SetCursor(LoadCursorW(NULL,(LPCWSTR)IDC_ARROW));} else {SetCursor(_glfw.win32.blankCursor);} }
    static void captureCursor(_GLFWwindow* window) { RECT clipRect; GetClientRect(window->win32.handle,&clipRect); ClientToScreen(window->win32.handle,(POINT*)&clipRect.left); ClientToScreen(window->win32.handle,(POINT*)&clipRect.right); ClipCursor(&clipRect); _glfw.win32.capturedCursorWindow=window; }
    static void releaseCursor(void) { ClipCursor(NULL); _glfw.win32.capturedCursorWindow=NULL; }
    static void disableCursor(_GLFWwindow* window) { _glfw.win32.disabledCursorWindow=window; _glfwGetCursorPosWin32(window,&_glfw.win32.restoreCursorPosX,&_glfw.win32.restoreCursorPosY); updateCursorImage(window); captureCursor(window); }
    static void enableCursor(_GLFWwindow* window) { _glfw.win32.disabledCursorWindow=NULL; releaseCursor(); _glfwSetCursorPosWin32(window,_glfw.win32.restoreCursorPosX,_glfw.win32.restoreCursorPosY); updateCursorImage(window); }
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
                if (HIWORD(lParam)==WM_LBUTTONDOWN && LOWORD(lParam)!=HTCLIENT) window->win32.frameAction= 1;
                break;
            }
            case WM_CAPTURECHANGED: {
                if (lParam==0&&window->win32.frameAction) {
                    if (window->cursorMode==GLFW_CURSOR_DISABLED) disableCursor(window);
                    window->win32.frameAction=0;
                }
                break;
            }
            case WM_SETFOCUS: {
                _glfwInputWindowFocus(window, 1);
                if (window->win32.frameAction) break;
                if (window->cursorMode==GLFW_CURSOR_DISABLED) disableCursor(window);
                return 0;
            }
            case WM_KILLFOCUS: {
                if (window->cursorMode==GLFW_CURSOR_DISABLED) enableCursor(window);
                _glfwInputWindowFocus(window,0);
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
            case WM_CLOSE: window->shouldClose =  1; return 0;
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
                    window->win32.cursorTracked= 1;
                }
                if (window->cursorMode==GLFW_CURSOR_DISABLED) {
                    const int dx=x-window->win32.lastCursorPosX,dy=y-window->win32.lastCursorPosY;
                    if (_glfw.win32.disabledCursorWindow!=window) break;
                    _glfwInputCursorPos(window,window->virtualCursorPosX+dx,window->virtualCursorPosY+dy);
                }
                window->win32.lastCursorPosX=x; window->win32.lastCursorPosY=y;
                return 0;
            }
            case WM_INPUT: break;
            case WM_MOUSELEAVE: { window->win32.cursorTracked=0; return 0; }
            case WM_MOUSEWHEEL: { Sys_Input.scrollDelta += (SHORT)HIWORD(wParam)/(double)WHEEL_DELTA; return 0; }
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
            case WM_SETCURSOR: { if (LOWORD(lParam)==HTCLIENT) { updateCursorImage(window); return TRUE; } break; }
        }
        return DefWindowProcW(hWnd,uMsg,wParam,lParam);
    }

    static void createNativeWindow(_GLFWwindow* window, char* title, int width, int height) {
        DWORD style=getWindowStyle(window);
        WNDCLASSEXW wc={0}; wc.cbSize=sizeof(wc),wc.style=CS_HREDRAW|CS_VREDRAW|CS_OWNDC,wc.lpfnWndProc=windowProc,wc.hInstance=_glfw.win32.instance,wc.lpszClassName=L"Voxen",wc.hIcon=wc.hCursor=NULL; _glfw.win32.mainWindowClass=RegisterClassExW(&wc);
        RECT rect={0,0,width,height};
        AdjustWindowRectEx(&rect,style,FALSE,WS_EX_APPWINDOW);
        int frameX=CW_USEDEFAULT, frameY=CW_USEDEFAULT;
        int frameWidth=rect.right-rect.left, frameHeight=rect.bottom-rect.top;
        WCHAR* wideTitle=_glfwCreateWideStringFromUTF8Win32(title);
        window->win32.handle=CreateWindowExW(WS_EX_APPWINDOW,(LPCWSTR)MAKEINTATOM(_glfw.win32.mainWindowClass),wideTitle,style,frameX,frameY,frameWidth,frameHeight,NULL,NULL,_glfw.win32.instance,(LPVOID)NULL),free(wideTitle);
        SetPropW(window->win32.handle,L"GLFW",window);
        window->win32.keymenu=0; WINDOWPLACEMENT wp={0}; wp.length=sizeof(wp); AdjustWindowRectEx(&rect,style,FALSE,WS_EX_APPWINDOW);
        GetWindowPlacement(window->win32.handle,&wp), OffsetRect(&rect,wp.rcNormalPosition.left-rect.left,wp.rcNormalPosition.top-rect.top);
        wp.rcNormalPosition=rect, wp.showCmd=SW_HIDE, SetWindowPlacement(window->win32.handle,&wp);
        _glfwGetWindowSizeWin32(window,&window->win32.width,&window->win32.height);
    }
    
    void _glfwSetWindowIconWin32(_GLFWwindow* window, const GLFWimage* image) {
        HICON hIcon = createIcon(image,0,0, 1);
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

    void _glfwSetWindowMonitorWin32(_GLFWwindow* window,int xpos,int ypos,int width,int height) {
        RECT r = {xpos,ypos,xpos+width,ypos+height}; DWORD s = GetWindowLongW(window->win32.handle,GWL_STYLE); UINT f = SWP_NOACTIVATE|SWP_NOCOPYBITS;
        if (window->decorated) { s &= ~WS_POPUP, s |= getWindowStyle(window), SetWindowLongW(window->win32.handle,GWL_STYLE,s), f |= SWP_FRAMECHANGED; }
        AdjustWindowRectEx(&r,getWindowStyle(window),FALSE,WS_EX_APPWINDOW);
        SetWindowPos(window->win32.handle,(HWND)HWND_NOTOPMOST,r.left,r.top,r.right-r.left,r.bottom-r.top,f);
    }

    void _glfwSetWindowDecoratedWin32(_GLFWwindow* window,GLFWbool enabled) { (void)enabled; updateWindowStyles(window); }
    void _glfwPollEventsWin32(void) {
        MSG msg; HWND handle; _GLFWwindow* window;
        while (PeekMessageW(&msg,NULL,0,0,PM_REMOVE)) {
            if (msg.message==WM_QUIT) { window=_glfw.windowListHead; while (window) { window->shouldClose =  1; window=window->next; } }
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

    void _glfwSetCursorModeWin32(_GLFWwindow* window) { if (window->win32.handle==GetActiveWindow()) {_glfwGetCursorPosWin32(window,&_glfw.win32.restoreCursorPosX,&_glfw.win32.restoreCursorPosY); captureCursor(window); _glfw.win32.disabledCursorWindow=window;} else Sys_Global.gamePaused = true; updateCursorImage(window); }
    GLFWproc _glfwPlatformGetModuleSymbol(void* module, const char* name) { return (GLFWproc)GetProcAddress((HMODULE)module,name); }
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

    WCHAR* _glfwCreateWideStringFromUTF8Win32(const char* source) { WCHAR* target; int count = MultiByteToWideChar(CP_UTF8,0,source,-1,NULL,0); target = calloc(count,sizeof(WCHAR)); MultiByteToWideChar(CP_UTF8,0,source,-1,target,count); return target; }
    char* _glfwCreateUTF8FromWideStringWin32(const WCHAR* source) { int size = WideCharToMultiByte(CP_UTF8,0,source,-1,NULL,0,NULL,NULL); char* target = calloc(size,1); WideCharToMultiByte(CP_UTF8,0,source,-1,target,size,NULL,NULL); return target; }
    BOOL _glfwIsWindowsVersionOrGreaterWin32(WORD major,WORD minor,WORD sp) {
        OSVERSIONINFOEXW osvi={0}; osvi.dwOSVersionInfoSize=sizeof(osvi), osvi.dwMajorVersion=major, osvi.dwMinorVersion=minor, osvi.wServicePackMajor=sp;
        DWORD mask=VER_MAJORVERSION|VER_MINORVERSION|VER_SERVICEPACKMAJOR;
        ULONGLONG cond=VerSetConditionMask(VerSetConditionMask(VerSetConditionMask(0,VER_MAJORVERSION,VER_GREATER_EQUAL),VER_MINORVERSION,VER_GREATER_EQUAL),VER_SERVICEPACKMAJOR,VER_GREATER_EQUAL);
        return RtlVerifyVersionInfo(&osvi,mask,cond)==0;
    }

    static void closeJoystick(_GLFWjoystick* js) { _glfwInputJoystick(js,0x00040002/*disconnected*/); _glfwFreeJoystick(js); }
    void _glfwDetectJoystickConnectionWin32(void) {
        if (_glfw.win32.xinput.instance) {
            DWORD index;
            for (index=0;index<4;index++) {
                int jid; char guid[33]; XINPUT_CAPABILITIES xic; _GLFWjoystick* js;
                for (jid = 0;  jid <= GLFW_JOYSTICK_LAST;  jid++) {
                    if (_glfw.joysticks[jid].connected && _glfw.joysticks[jid].win32.index == index) break;
                }

                if (jid <= GLFW_JOYSTICK_LAST) continue;
                if (XInputGetCapabilities(index, 0, &xic) != ERROR_SUCCESS) continue;

                sprintf(guid, "78696e707574%02x000000000000000000",xic.SubType & 0xff);
                js = _glfwAllocJoystick("Gamepad", guid, 6, 10, 1);
                if (!js) continue;

                js->win32.index = index;
                _glfwInputJoystick(js,0x00040001/*connected*/);
            }
        }
    }

    GLFWbool _glfwInitJoysticksWin32(void) { _glfwDetectJoystickConnectionWin32(); return  1; }
    GLFWbool _glfwPollJoystickWin32(_GLFWjoystick* js) {
        int i, dpad = 0; DWORD result; XINPUT_STATE xis;
        const WORD buttons[10] = {XINPUT_GAMEPAD_A,XINPUT_GAMEPAD_B,XINPUT_GAMEPAD_X,XINPUT_GAMEPAD_Y,XINPUT_GAMEPAD_LEFT_SHOULDER,XINPUT_GAMEPAD_RIGHT_SHOULDER,XINPUT_GAMEPAD_BACK,XINPUT_GAMEPAD_START,XINPUT_GAMEPAD_LEFT_THUMB,XINPUT_GAMEPAD_RIGHT_THUMB};
        result = XInputGetState(js->win32.index, &xis);
        if (result != ERROR_SUCCESS) { if (result == ERROR_DEVICE_NOT_CONNECTED) {closeJoystick(js);} return 0; }

        _glfwInputJoystickAxis(js, 0, (xis.Gamepad.sThumbLX + 0.5f) / 32767.5f);
        _glfwInputJoystickAxis(js, 1, -(xis.Gamepad.sThumbLY + 0.5f) / 32767.5f);
        _glfwInputJoystickAxis(js, 2, (xis.Gamepad.sThumbRX + 0.5f) / 32767.5f);
        _glfwInputJoystickAxis(js, 3, -(xis.Gamepad.sThumbRY + 0.5f) / 32767.5f);
        _glfwInputJoystickAxis(js, 4, xis.Gamepad.bLeftTrigger / 127.5f - 1.f);
        _glfwInputJoystickAxis(js, 5, xis.Gamepad.bRightTrigger / 127.5f - 1.f);
        for (i = 0;  i < 10;  i++) { const char value = (xis.Gamepad.wButtons & buttons[i]) ? 1 : 0; _glfwInputJoystickButton(js,i,value); }
        if (xis.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) dpad |= GLFW_HAT_UP;
        if (xis.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) dpad |= GLFW_HAT_RIGHT;
        if (xis.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) dpad |= GLFW_HAT_DOWN;
        if (xis.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) dpad |= GLFW_HAT_LEFT;
        if ((dpad & GLFW_HAT_RIGHT) && (dpad & GLFW_HAT_LEFT)) dpad &= ~(GLFW_HAT_RIGHT | GLFW_HAT_LEFT);
        if ((dpad & GLFW_HAT_UP) && (dpad & GLFW_HAT_DOWN)) dpad &= ~(GLFW_HAT_UP | GLFW_HAT_DOWN);
        _glfwInputJoystickHat(js, 0, dpad);
        return  1;
    }
    
    void _glfwDetectJoystickDisconnectionWin32(void) { for (int jid = 0;  jid <= GLFW_JOYSTICK_LAST;  jid++) { _GLFWjoystick* js = _glfw.joysticks + jid; if (js->connected) {_glfwPollJoystickWin32(js);} } }
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
        name = display ? _glfwCreateUTF8FromWideStringWin32(display->DeviceString) : _glfwCreateUTF8FromWideStringWin32(adapter->DeviceString);
        ZeroMemory(&dm,sizeof(dm)); dm.dmSize = sizeof(dm);
        EnumDisplaySettingsW(adapter->DeviceName, ENUM_CURRENT_SETTINGS, &dm);
        dc = CreateDCW(L"DISPLAY", adapter->DeviceName, NULL, NULL);
        if (IsWindows8Point1OrGreater()) { widthMM  = GetDeviceCaps(dc, HORZSIZE); heightMM = GetDeviceCaps(dc, VERTSIZE); }
        else { widthMM  = (int) (dm.dmPelsWidth * 25.4f / GetDeviceCaps(dc, LOGPIXELSX)); heightMM = (int) (dm.dmPelsHeight * 25.4f / GetDeviceCaps(dc, LOGPIXELSY)); }

        DeleteDC(dc); monitor = _glfwAllocMonitor(name,widthMM,heightMM); free(name);
        if (adapter->StateFlags & DISPLAY_DEVICE_MODESPRUNED) monitor->win32.modesPruned =  1;
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
            disconnected = calloc(_glfw.monitorCount, sizeof(_GLFWmonitor*));
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

                _glfwInputMonitor(monitor,0x00040001/*connected*/,type); type = 1;
            }

            if (displayIndex == 0) {
                for (i = 0;  i < disconnectedCount;  i++) {
                    if (disconnected[i] && wcscmp(disconnected[i]->win32.adapterName,adapter.DeviceName) == 0) { disconnected[i] = NULL; break; }
                }

                if (i < disconnectedCount) continue;

                monitor = createMonitor(&adapter, NULL);
                if (!monitor) { free(disconnected); return; }

                _glfwInputMonitor(monitor, 0x00040001/*connected*/, type);
            }
        }

        for (i = 0;  i < disconnectedCount;  i++) {
            if (disconnected[i]) _glfwInputMonitor(disconnected[i],0x00040002/*disconnected*/,0);
        }

        free(disconnected);
    }

    void _glfwGetMonitorPosWin32(_GLFWmonitor* monitor, int* xpos, int* ypos) { DEVMODEW dm; ZeroMemory(&dm,sizeof(dm)); dm.dmSize = sizeof(dm); EnumDisplaySettingsExW(monitor->win32.adapterName,ENUM_CURRENT_SETTINGS,&dm,0x00000004); *xpos = dm.dmPosition.x; *ypos = dm.dmPosition.y; }
    void _glfwGetMonitorWorkareaWin32(_GLFWmonitor* monitor, int* xpos, int* ypos, int* width, int* height) { MONITORINFO mi = {0}; mi.cbSize = sizeof(mi); GetMonitorInfoW(monitor->win32.handle, &mi); *xpos = mi.rcWork.left; *ypos = mi.rcWork.top; *width = mi.rcWork.right - mi.rcWork.left; *height = mi.rcWork.bottom - mi.rcWork.top; }
    void _glfwGetVideoModeWin32(_GLFWmonitor* monitor, GLFWvidmode* mode) { DEVMODEW dm; ZeroMemory(&dm, sizeof(dm)); dm.dmSize = sizeof(dm); EnumDisplaySettingsW(monitor->win32.adapterName,ENUM_CURRENT_SETTINGS,&dm); mode->width=dm.dmPelsWidth; mode->height=dm.dmPelsHeight; mode->refreshRate=dm.dmDisplayFrequency; }
    void _glfwPlatformInitTimer(void) { QueryPerformanceFrequency((LARGE_INTEGER*) &_glfw.timer.win32.frequency); }
    u64 _glfwPlatformGetTimerValue(void) { u64 value; QueryPerformanceCounter((LARGE_INTEGER*)&value); return value; }
    u64 _glfwPlatformGetTimerFrequency(void) { return _glfw.timer.win32.frequency; }
    void _glfwPlatformCreateTls(_GLFWtls* tls) { tls->win32.index = TlsAlloc(); tls->win32.allocated =  1; }
    void* _glfwPlatformGetTls(_GLFWtls* tls) { return TlsGetValue(tls->win32.index); }
    void _glfwPlatformSetTls(_GLFWtls* tls, void* value) { TlsSetValue(tls->win32.index,value); }
    static int choosePixelFormatWGL(_GLFWwindow* window) {
        int attribs[24],values[24],attribCount=0,i,pixelFormat,nativeCount,usableCount=0;
        const int query = 0x2000/*num pixel formats*/; wglGetPixelFormatAttribivARB(window->context.wgl.dc,1,0,1,&query,&nativeCount);
        attribs[attribCount++] = 0x2010/*support opengl*/; attribs[attribCount++] = 0x2001/*draw to window*/; attribs[attribCount++] = 0x2013/*pixel type*/; attribs[attribCount++] = 0x2003/*accelaration*/;
        attribs[attribCount++] = 0x2011/*double buffer*/; attribs[attribCount++] = 0x2015/*r bits*/; attribs[attribCount++] = 0x2017/*g bits*/;
        attribs[attribCount++] = 0x2019/*b bits*/; attribs[attribCount++] = 0x201b/*a bits*/; attribs[attribCount++] = 0x2022/*depth bits*/; attribs[attribCount++] = 0x2023/*stencil bits*/;
        _GLFWfbconfig* usableConfigs = calloc(nativeCount,sizeof(_GLFWfbconfig));
        for (i = 0; i < nativeCount; i++) {
            _GLFWfbconfig* u = usableConfigs + usableCount; pixelFormat = i + 1;
            wglGetPixelFormatAttribivARB(window->context.wgl.dc,pixelFormat,0,attribCount,attribs,values);
            if (values[0] == 0 || values[1] == 0/* support OpenGL + draw to window */ || values[2] != 0x202b/*type rgba*/ || values[3] == 0x2025/*no accel*/ || values[4] !=  1) continue;
            
            u->redBits=values[5]; u->greenBits=values[6]; u->blueBits=values[7]; u->alphaBits=values[8]; u->depthBits=values[9]; u->stencilBits=values[10]; u->handle=pixelFormat; usableCount++;
        }

        const _GLFWfbconfig* closest = _glfwChooseFBConfig(usableConfigs,usableCount);
        pixelFormat = (int)closest->handle; free(usableConfigs);
        return pixelFormat;
    }

    static void makeContextCurrentWGL(_GLFWwindow* window) { wglMakeCurrent(window->context.wgl.dc,window->context.wgl.handle); _glfwPlatformSetTls(&_glfw.contextSlot,window); }
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
        if (!extensions) return 0;
        return _glfwStringInExtensionString(extension, extensions);
    }

    static GLFWglproc getProcAddressWGL(const char* procname) { const GLFWglproc proc = (GLFWglproc)wglGetProcAddress(procname); if (proc) {return proc;} return (GLFWglproc)_glfwPlatformGetModuleSymbol(_glfw.wgl.instance,procname); }
    #define PLATFORM_getCursorPos(w,x,y)            _glfwGetCursorPosWin32(w,x,y)
    #define PLATFORM_setCursorPos(w,x,y)            _glfwSetCursorPosWin32(w,x,y)
    #define PLATFORM_setCursorMode(w)               _glfwSetCursorModeWin32(w)
    #define PLATFORM_initJoysticks()                _glfwInitJoysticksWin32()
    #define PLATFORM_pollJoystick(js)               _glfwPollJoystickWin32(js)
    #define PLATFORM_getMonitorPos(m,x,y)           _glfwGetMonitorPosWin32(m,x,y)
    #define PLATFORM_getMonitorWorkarea(m,x,y,w,h)  _glfwGetMonitorWorkareaWin32(m,x,y,w,h)
    #define PLATFORM_getVideoMode(m,cur)            _glfwGetVideoModeWin32(m,cur)
    #define PLATFORM_setWindowIcon(w,i)             _glfwSetWindowIconWin32(w,i)
    #define PLATFORM_getWindowPos(w,x,y)            _glfwGetWindowPosWin32(w,x,y)
    #define PLATFORM_setWindowPos(w,x,y)            _glfwSetWindowPosWin32(w,x,y)
    #define PLATFORM_getWindowSize(w,wi,h)          _glfwGetWindowSizeWin32(w,wi,h)
    #define PLATFORM_setWindowSize(w,wi,h)          _glfwSetWindowSizeWin32(w,wi,h)
    #define PLATFORM_setWindowMonitor(w,x,y,wi,h)   _glfwSetWindowMonitorWin32(w,x,y,wi,h)
    #define PLATFORM_setWindowDecorated(w,v)        _glfwSetWindowDecoratedWin32(w,v)
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
    void _glfwPlatformCreateTls(_GLFWtls* tls) { pthread_key_create(&tls->posix.key,NULL); tls->posix.allocated =  1; }
    void* _glfwPlatformGetTls(_GLFWtls* tls) { return pthread_getspecific(tls->posix.key); }
    void _glfwPlatformSetTls(_GLFWtls* tls, void* value) { pthread_setspecific(tls->posix.key, value); }
    void* _glfwPlatformLoadModule(const char* path) { return dlopen(path, RTLD_LAZY | RTLD_LOCAL); }
    GLFWproc _glfwPlatformGetModuleSymbol(void* module, const char* name) { return dlsym(module, name); }
    unsigned long _glfwGetWindowPropertyX11(Window window,Atom property,Atom type,unsigned char** value) {
        Atom actualType; int actualFormat; unsigned long itemCount,bytesAfter;
        _glfw.x11.xlib.GetWindowProperty(_glfw.x11.display,window,property,0,2147483647,False,type,&actualType,&actualFormat,&itemCount,&bytesAfter,value);
        return itemCount;
    }

    static int translateKey(int scancode) { return (scancode<0||scancode>255) ? GLFW_KEY_UNKNOWN : _glfw.x11.keycodes[scancode]; }
    static void sendEventToWM(_GLFWwindow* window,Atom type,long a,long b,long c,long d,long e) {
        XEvent event={33/*ClientMessage*/};
        event.xclient.window=window->x11.handle; event.xclient.format=32; event.xclient.message_type=type;
        event.xclient.data.l[0]=a; event.xclient.data.l[1]=b; event.xclient.data.l[2]=c; event.xclient.data.l[3]=d; event.xclient.data.l[4]=e;
        _glfw.x11.xlib.SendEvent(_glfw.x11.display,_glfw.x11.root,False,(1L<<19)|(1L<<20),&event);
    }

    static void updateNormalHints(_GLFWwindow* window,int width,int height) {
        XSizeHints* hints=_glfw.x11.xlib.AllocSizeHints(); long supplied;
        _glfw.x11.xlib.GetWMNormalHints(_glfw.x11.display,window->x11.handle,hints,&supplied);
        hints->flags &= ~((1L << 4)/*PMinSize*/|(1L << 5)/*PMaxSize*/|(1L << 7)/*PAspect*/);
        hints->flags|=((1L << 4)/*PMinSize*/|(1L << 5)/*PMaxSize*/);
        hints->min_width=hints->max_width=width; hints->min_height=hints->max_height=height;
        _glfw.x11.xlib.SetWMNormalHints(_glfw.x11.display,window->x11.handle,hints);
        _glfw.x11.xlib.Free(hints);
    }
    
    static void updateCursorImage(_GLFWwindow* window) { if (window->cursorMode==0x00034001/*GLFW_CURSOR_NORMAL*/) { _glfw.x11.xlib.UndefineCursor(_glfw.x11.display,window->x11.handle); } else {_glfw.x11.xlib.DefineCursor(_glfw.x11.display,window->x11.handle,_glfw.x11.hiddenCursorHandle);} }
    static void captureCursor(_GLFWwindow* window) { _glfw.x11.xlib.GrabPointer(_glfw.x11.display,window->x11.handle,True,(1L<<2)|(1L<<3)|(1L<<6),1/*GrabModeAsync*/,1/*GrabModeAsync*/,window->x11.handle,0L,0L); }
    static void releaseCursor(void) { _glfw.x11.xlib.UngrabPointer(_glfw.x11.display,0L); }
    static void disableCursor(_GLFWwindow* window) { _glfw.x11.disabledCursorWindow=window; _glfwGetCursorPosX11(window,&_glfw.x11.restoreCursorPosX,&_glfw.x11.restoreCursorPosY); updateCursorImage(window); captureCursor(window); }
    static void enableCursor(_GLFWwindow* window) { _glfw.x11.disabledCursorWindow=NULL; releaseCursor(); _glfwSetCursorPosX11(window,_glfw.x11.restoreCursorPosX,_glfw.x11.restoreCursorPosY); updateCursorImage(window); }
    void _glfwGetMonitorPosX11(_GLFWmonitor* monitor, int* xpos, int* ypos) {
        XRRScreenResources* sr = XRRGetScreenResourcesCurrent(_glfw.x11.display, _glfw.x11.root);
        XRRCrtcInfo* ci = XRRGetCrtcInfo(_glfw.x11.display, sr, monitor->x11.crtc);
        if (ci) { *xpos = ci->x; *ypos = ci->y; XRRFreeCrtcInfo(ci); }
        XRRFreeScreenResources(sr);
    }

    void _glfwSetWindowIconX11(_GLFWwindow* window, const GLFWimage* images) {
        int longCount=0;
        longCount+=2+images[0].width*images[0].height;
        unsigned long* icon=calloc(longCount,sizeof(unsigned long)), *target=icon;
        *target++=images[0].width; *target++=images[0].height;
        for (int j=0;j<images[0].width*images[0].height;++j) *target++=(((unsigned long)images[0].pixels[j*4+0])<<16)|(((unsigned long)images[0].pixels[j*4+1])<<8)|(((unsigned long)images[0].pixels[j*4+2])<<0)|(((unsigned long)images[0].pixels[j*4+3])<<24);
        _glfw.x11.xlib.ChangeProperty(_glfw.x11.display,window->x11.handle,_glfw.x11.NET_WM_ICON,((Atom) 6),32,0/*PropModeReplace*/,(unsigned char*)icon,longCount);
        free(icon);
    }

    void _glfwGetWindowPosX11(_GLFWwindow* window,int* xpos,int* ypos) { Window dummy; _glfw.x11.xlib.TranslateCoordinates(_glfw.x11.display,window->x11.handle,_glfw.x11.root,0,0,xpos,ypos,&dummy); }
    void _glfwSetWindowPosX11(_GLFWwindow* window,int xpos,int ypos) {
        if (!_glfwWindowVisibleX11(window)) {
            long supplied; XSizeHints* hints=_glfw.x11.xlib.AllocSizeHints();
            if (_glfw.x11.xlib.GetWMNormalHints(_glfw.x11.display,window->x11.handle,hints,&supplied)) { hints->flags|=(1L << 2)/*PPosition*/; hints->x=hints->y=0; _glfw.x11.xlib.SetWMNormalHints(_glfw.x11.display,window->x11.handle,hints); }
            _glfw.x11.xlib.Free(hints);
        }
        _glfw.x11.xlib.MoveWindow(_glfw.x11.display,window->x11.handle,xpos,ypos);
    }

    void _glfwGetWindowSizeX11(_GLFWwindow* window,int* width,int* height) { XWindowAttributes attribs; _glfw.x11.xlib.GetWindowAttributes(_glfw.x11.display,window->x11.handle,&attribs); *width=attribs.width; *height=attribs.height; }
    void _glfwSetWindowSizeX11(_GLFWwindow* window,int width,int height) { width=vmax(1,width); height=vmax(1,height); updateNormalHints(window,width,height); _glfw.x11.xlib.ResizeWindow(_glfw.x11.display,window->x11.handle,width,height); }
    void _glfwFocusWindowX11(_GLFWwindow* window) {
        if (_glfw.x11.NET_ACTIVE_WINDOW) sendEventToWM(window,_glfw.x11.NET_ACTIVE_WINDOW,1,0,0,0,0);
        else if (_glfwWindowVisibleX11(window)) { _glfw.x11.xlib.RaiseWindow(_glfw.x11.display,window->x11.handle); _glfw.x11.xlib.SetInputFocus(_glfw.x11.display,window->x11.handle,2/*RevertToParent*/,0L); }
    }

    void _glfwSetWindowMonitorX11(_GLFWwindow* window,int xpos,int ypos,int width,int height) {
        updateNormalHints(window,width,height);
        if (_glfw.x11.NET_WM_STATE && _glfw.x11.NET_WM_STATE_FULLSCREEN) sendEventToWM(window,_glfw.x11.NET_WM_STATE,0/*remove*/,_glfw.x11.NET_WM_STATE_FULLSCREEN,0,1,0);
        else {
            XSetWindowAttributes attributes; attributes.override_redirect=False;
            _glfw.x11.xlib.ChangeWindowAttributes(_glfw.x11.display,window->x11.handle,(1L<<9)/*override redirect*/,&attributes);
            window->x11.overrideRedirect=0;
        }
        
        _glfw.x11.xlib.DeleteProperty(_glfw.x11.display,window->x11.handle,_glfw.x11.NET_WM_BYPASS_COMPOSITOR);
        _glfw.x11.xlib.MoveResizeWindow(_glfw.x11.display,window->x11.handle,xpos,ypos,width,height);
    }
    
    GLFWbool _glfwWindowFocusedX11(_GLFWwindow* window) { Window focused; int state; _glfw.x11.xlib.GetInputFocus(_glfw.x11.display,&focused,&state); return window->x11.handle==focused; }
    GLFWbool _glfwWindowVisibleX11(_GLFWwindow* window) { XWindowAttributes wa; _glfw.x11.xlib.GetWindowAttributes(_glfw.x11.display,window->x11.handle,&wa); return wa.map_state==2/*IsViewable*/; }
    void UpdateScreenSize(i32 width, i32 height);
    static void processEvent(XEvent* event) {
        unsigned int keycode=0; Bool filtered=False;
        if (event->type==2/*KeyPress*/ || event->type==3/*KeyRelease*/) keycode=event->xkey.keycode;
        filtered=_glfw.x11.xlib.FilterEvent(event,0L);
        if (event->type==_glfw.x11.randr.eventBase+1/*notify*/) { XRRUpdateConfiguration(event); _glfwPollMonitorsX11(); return; }
        if (event->type==35/*GenericEvent*/) return;
        _GLFWwindow* window=NULL; if (_glfw.x11.xlib.FindContext(_glfw.x11.display,event->xany.window,_glfw.x11.context,(XPointer*)&window)!=0) return;

        switch (event->type) {
            case 21/*ReparentNotify*/: window->x11.parent=event->xreparent.parent; return;
            case 2/*KeyPress*/:
            case 3/*KeyRelease*/: {
                const int key=translateKey(keycode),action=(event->type==2/*KeyPress*/)?GLFW_PRESS:GLFW_RELEASE;
                if (key!=GLFW_KEY_UNKNOWN) _glfwInputKey(window,key,action);
                return;
            }
            case 4/*ButtonPress*/: {
                if      (event->xbutton.button==1) _glfwInputMouseClick(window,GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS);
                else if (event->xbutton.button==2) _glfwInputMouseClick(window,GLFW_MOUSE_BUTTON_MIDDLE,GLFW_PRESS);
                else if (event->xbutton.button==3) _glfwInputMouseClick(window,GLFW_MOUSE_BUTTON_RIGHT,GLFW_PRESS);
                else if (event->xbutton.button==4) Sys_Input.scrollDelta += 1.0;
                else if (event->xbutton.button==5) Sys_Input.scrollDelta += -1.0;
                else _glfwInputMouseClick(window,event->xbutton.button - 1 - 4,GLFW_PRESS);
                return;
            }
            case 5/*ButtonRelease*/: {
                if      (event->xbutton.button==1) _glfwInputMouseClick(window,GLFW_MOUSE_BUTTON_LEFT,GLFW_RELEASE);
                else if (event->xbutton.button==2) _glfwInputMouseClick(window,GLFW_MOUSE_BUTTON_MIDDLE,GLFW_RELEASE);
                else if (event->xbutton.button==3) _glfwInputMouseClick(window,GLFW_MOUSE_BUTTON_RIGHT,GLFW_RELEASE);
                else if (event->xbutton.button>7)  _glfwInputMouseClick(window,event->xbutton.button - 1 - 4,GLFW_RELEASE);
                return;
            }
            case 7/*EnterNotify*/: {
                const int x=event->xcrossing.x,y=event->xcrossing.y;
                _glfwInputCursorPos(window,x,y);
                window->x11.lastCursorPosX=x; window->x11.lastCursorPosY=y;
                return;
            }
            case 6/*MotionNotify*/: {
                const int x=event->xmotion.x,y=event->xmotion.y;
                if (x!=window->x11.warpCursorPosX || y!=window->x11.warpCursorPosY) {
                    if (window->cursorMode==GLFW_CURSOR_DISABLED) {
                        if (_glfw.x11.disabledCursorWindow!=window) return;
                        _glfwInputCursorPos(window,window->virtualCursorPosX+(x-window->x11.lastCursorPosX),window->virtualCursorPosY+(y-window->x11.lastCursorPosY));
                    } else _glfwInputCursorPos(window,x,y);
                }
                window->x11.lastCursorPosX=x; window->x11.lastCursorPosY=y;
                return;
            }
            case 22/*ConfigureNotify*/: {
                if (event->xconfigure.width!=window->x11.width || event->xconfigure.height!=window->x11.height) { window->x11.width=event->xconfigure.width; window->x11.height=event->xconfigure.height; UpdateScreenSize(event->xconfigure.width,event->xconfigure.height); }
                int xpos=event->xconfigure.x,ypos=event->xconfigure.y;
                if (!event->xany.send_event && window->x11.parent!=_glfw.x11.root) {
                    Window dummy;
                    _glfw.x11.xlib.TranslateCoordinates(_glfw.x11.display,window->x11.parent,_glfw.x11.root,xpos,ypos,&xpos,&ypos,&dummy);
                }
                if (xpos!=window->x11.xpos || ypos!=window->x11.ypos) { window->x11.xpos=xpos; window->x11.ypos=ypos; }
                return;
            }
            case 33/*ClientMessage*/: {
                if (filtered) return;
                if (event->xclient.message_type==0L) return;
                
                if (event->xclient.message_type==_glfw.x11.WM_PROTOCOLS) {
                    const Atom protocol=event->xclient.data.l[0];
                    if (protocol==0L) return;
                    
                    if (protocol == _glfw.x11.WM_DELETE_WINDOW) window->shouldClose =  1;
                    if (protocol==_glfw.x11.NET_WM_PING) {
                        XEvent reply=*event; reply.xclient.window=_glfw.x11.root;
                        _glfw.x11.xlib.SendEvent(_glfw.x11.display,_glfw.x11.root,False,(1L<<19)|(1L<<20),&reply);
                    }
                }
                return;
            }
            case 9/*FocusIn*/: {
                if (event->xfocus.mode==1/*NotifyGrab*/ || event->xfocus.mode==2/*NotifyUngrab*/) return;
                if (window->cursorMode==GLFW_CURSOR_DISABLED) disableCursor(window);
                if (window->x11.ic) _glfw.x11.xlib.SetICFocus(window->x11.ic);
                _glfwInputWindowFocus(window, 1);
                return;
            }
            case 10/*FocusOut*/: {
                if (event->xfocus.mode==1/*NotifyGrab*/ || event->xfocus.mode==2/*NotifyUngrab*/) return;
                if (window->cursorMode==GLFW_CURSOR_DISABLED) enableCursor(window);
                if (window->x11.ic) _glfw.x11.xlib.UnsetICFocus(window->x11.ic);
                _glfwInputWindowFocus(window,0);
                return;
            }
        }
    }

    void _glfwSetWindowDecoratedX11(_GLFWwindow* window,GLFWbool enabled) {
        struct { unsigned long flags,functions,decorations; long input_mode; unsigned long status; } hints={0};
        hints.flags=2; hints.decorations=enabled?1:0;
        _glfw.x11.xlib.ChangeProperty(_glfw.x11.display,window->x11.handle,_glfw.x11.MOTIF_WM_HINTS,_glfw.x11.MOTIF_WM_HINTS,32,0/*PropModeReplace*/,(unsigned char*)&hints,sizeof(hints)/sizeof(long));
    }

    void _glfwPollEventsX11(void) {
        if (_glfw.joysticksInitialized) _glfwDetectJoystickConnectionLinux();
        _glfw.x11.xlib.Pending(_glfw.x11.display);
        while (QLength(_glfw.x11.display)) { XEvent event; _glfw.x11.xlib.NextEvent(_glfw.x11.display,&event); processEvent(&event); }
        _GLFWwindow* window = _glfw.x11.disabledCursorWindow;
        if (window) {
            int width,height; _glfwGetWindowSizeX11(window,&width,&height);
            if (window->x11.lastCursorPosX!=width/2 || window->x11.lastCursorPosY!=height/2) _glfwSetCursorPosX11(window,width/2,height/2);
        }
    }

    void _glfwGetCursorPosX11(_GLFWwindow* window,double* xpos,double* ypos) { Window root,child; int rootX,rootY,childX,childY; unsigned int mask; _glfw.x11.xlib.QueryPointer(_glfw.x11.display,window->x11.handle,&root,&child,&rootX,&rootY,&childX,&childY,&mask); *xpos=childX; *ypos=childY; }
    void _glfwSetCursorPosX11(_GLFWwindow* window,double x,double y) { window->x11.warpCursorPosX=(int)x; window->x11.warpCursorPosY=(int)y; _glfw.x11.xlib.WarpPointer(_glfw.x11.display,0L,window->x11.handle,0,0,0,0,(int)x,(int)y); }
    void _glfwSetCursorModeX11(_GLFWwindow* window) { if (_glfwWindowFocusedX11(window)) {_glfwGetCursorPosX11(window,&_glfw.x11.restoreCursorPosX,&_glfw.x11.restoreCursorPosY); captureCursor(window); _glfw.x11.disabledCursorWindow=window;} else Sys_Global.gamePaused = true; updateCursorImage(window); }
    static const XRRModeInfo* getModeInfo(const XRRScreenResources* sr, RRMode id) { for (int i = 0;  i < sr->nmode;  i++){ if (sr->modes[i].id == id) {return sr->modes + i;} } return NULL; }
    static GLFWvidmode vidmodeFromModeInfo(const XRRModeInfo* mi, const XRRCrtcInfo* ci) {
        GLFWvidmode mode;
        if (ci->rotation == 2 || ci->rotation == 8) {  mode.width  = mi->height; mode.height = mi->width; } // ==90, ==270
        else { mode.width = mi->width; mode.height = mi->height; }
        mode.refreshRate = (mi->hTotal && mi->vTotal) ? (int)vround((double) mi->dotClock / ((double) mi->hTotal * (double) mi->vTotal)) : 0;
        return mode;
    }

    void _glfwPollMonitorsX11(void) {
        int disconnectedCount; _GLFWmonitor** disconnected = NULL;
        XRRScreenResources* sr = XRRGetScreenResourcesCurrent(_glfw.x11.display,_glfw.x11.root);
        RROutput primary = XRRGetOutputPrimary(_glfw.x11.display,_glfw.x11.root);
        disconnectedCount = _glfw.monitorCount;
        if (disconnectedCount) { disconnected = calloc(_glfw.monitorCount, sizeof(_GLFWmonitor*)); __builtin_memcpy(disconnected,_glfw.monitors,_glfw.monitorCount * sizeof(_GLFWmonitor*)); }
        for (int i = 0;  i < sr->noutput;  i++) {
            int j, type, widthMM, heightMM;
            XRROutputInfo* oi = XRRGetOutputInfo(_glfw.x11.display, sr, sr->outputs[i]);
            if (oi->connection != 0/*connected*/ || oi->crtc == 0L) { XRRFreeOutputInfo(oi); continue; }

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
            type = (monitor->x11.output == primary) ? 0 : 1; _glfwInputMonitor(monitor,0x00040001/*connected*/,type); XRRFreeOutputInfo(oi); XRRFreeCrtcInfo(ci);
        }

        XRRFreeScreenResources(sr);
        for (int i = 0;  i < disconnectedCount;  i++) { if (disconnected[i]) {_glfwInputMonitor(disconnected[i],0x00040002/*disconnected*/,0);} }
        free(disconnected);
    }

    void _glfwGetMonitorWorkareaX11(_GLFWmonitor* monitor,int* xpos,int* ypos,int* width,int* height) {
        int areaX = 0, areaY = 0, areaWidth = 0, areaHeight = 0;
        XRRScreenResources* sr = XRRGetScreenResourcesCurrent(_glfw.x11.display,_glfw.x11.root);
        XRRCrtcInfo* ci = XRRGetCrtcInfo(_glfw.x11.display,sr,monitor->x11.crtc);
        const XRRModeInfo* mi = getModeInfo(sr,ci->mode);
        areaX = ci->x, areaY = ci->y;
        if (ci->rotation == 2 || ci->rotation == 8) { areaWidth = mi->height, areaHeight = mi->width; } // ==90, ==270
        else { areaWidth = mi->width, areaHeight = mi->height; }
        XRRFreeCrtcInfo(ci), XRRFreeScreenResources(sr);
        if (_glfw.x11.NET_WORKAREA && _glfw.x11.NET_CURRENT_DESKTOP) {
            Atom *extents = NULL, *desktop = NULL;
            const unsigned long extentCount = _glfwGetWindowPropertyX11(_glfw.x11.root,_glfw.x11.NET_WORKAREA,((Atom) 6),(unsigned char**) &extents);
            if (_glfwGetWindowPropertyX11(_glfw.x11.root,_glfw.x11.NET_CURRENT_DESKTOP,((Atom) 6),(unsigned char**) &desktop) > 0) {
                if (extentCount >= 4 && *desktop < extentCount / 4) {
                    const int gx = extents[*desktop * 4 + 0], gy = extents[*desktop * 4 + 1], gw = extents[*desktop * 4 + 2], gh = extents[*desktop * 4 + 3];
                    if (areaX < gx) { areaWidth -= gx - areaX, areaX = gx; }
                    if (areaY < gy) { areaHeight -= gy - areaY, areaY = gy; }
                    if (areaX + areaWidth > gx + gw) areaWidth = gx - areaX + gw;
                    if (areaY + areaHeight > gy + gh) areaHeight = gy - areaY + gh;
                }
            }
            if (extents) {_glfw.x11.xlib.Free(extents);} if (desktop) {_glfw.x11.xlib.Free(desktop);}
        }
        *xpos = areaX; *ypos = areaY; *width = areaWidth; *height = areaHeight;
    }

    void _glfwGetVideoModeX11(_GLFWmonitor* monitor,GLFWvidmode* mode) {
        XRRScreenResources* sr = XRRGetScreenResourcesCurrent(_glfw.x11.display,_glfw.x11.root);
        const XRRModeInfo* mi = NULL;
        XRRCrtcInfo* ci = XRRGetCrtcInfo(_glfw.x11.display,sr,monitor->x11.crtc);
        if (ci) { mi = getModeInfo(sr,ci->mode); if (mi) {*mode = vidmodeFromModeInfo(mi,ci);} XRRFreeCrtcInfo(ci); }
        XRRFreeScreenResources(sr);
    }

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
        __builtin_memset(_glfw.x11.keycodes,-1,sizeof(_glfw.x11.keycodes));
        __builtin_memset(_glfw.x11.scancodes,-1,sizeof(_glfw.x11.scancodes));
        _glfw.x11.xlib.DisplayKeycodes(_glfw.x11.display,&scancodeMin,&scancodeMax);
        int width; KeySym* keysyms = _glfw.x11.xlib.GetKeyboardMapping(_glfw.x11.display,scancodeMin,scancodeMax - scancodeMin + 1,&width);
        for (int sc = scancodeMin; sc <= scancodeMax; sc++) {
            if (_glfw.x11.keycodes[sc] < 0) _glfw.x11.keycodes[sc] = translateKeySyms(&keysyms[(sc - scancodeMin) * width],width);
            if (_glfw.x11.keycodes[sc] > 0) _glfw.x11.scancodes[_glfw.x11.keycodes[sc]] = sc;
        }
        _glfw.x11.xlib.Free(keysyms);
    }

    static Atom getAtomIfSupported(Atom* atoms, unsigned long count, const char* name) { const Atom atom=_glfw.x11.xlib.InternAtom(_glfw.x11.display,name,False); for (unsigned long i=0;i<count;i++) {if (atoms[i] == atom) {return atom;}} return 0L; }
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
            if (strcmp(_glfw.joysticks[jid].linjs.path, path) == 0) return 0;
        }

        _GLFWjoystickLinux linjs = {0};
        linjs.fd = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
        if (linjs.fd == -1) return 0;

        char evBits[(EV_CNT + 7) / 8] = {0};
        char keyBits[(KEY_CNT + 7) / 8] = {0};
        char absBits[(ABS_CNT + 7) / 8] = {0};
        struct input_id id;
        if (ioctl(linjs.fd, EVIOCGBIT(0, sizeof(evBits)), evBits) < 0 || ioctl(linjs.fd, EVIOCGBIT(EV_KEY, sizeof(keyBits)), keyBits) < 0 || ioctl(linjs.fd, EVIOCGBIT(EV_ABS, sizeof(absBits)), absBits) < 0 || ioctl(linjs.fd, EVIOCGID, &id) < 0) { close(linjs.fd); return 0; }
        if (!isBitSet(EV_ABS, evBits)) { close(linjs.fd); return 0; }

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
        if (!js) { close(linjs.fd); return 0; }

        strncpy(linjs.path, path, sizeof(linjs.path) - 1);
        __builtin_memcpy(&js->linjs, &linjs, sizeof(linjs));
        pollAbsState(js);
        _glfwInputJoystick(js, 0x00040001/*connected*/);
        return  1;
    }
    #undef isBitSet

    static void closeJoystick(_GLFWjoystick* js) { _glfwInputJoystick(js,0x00040002/*disconnected*/); close(js->linjs.fd); _glfwFreeJoystick(js); }
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
        int count = 0; DIR* dir = opendir(dirname);
        if (dir) {
            struct dirent* entry;
            while ((entry = readdir(dir))) {
                regmatch_t match;
                if (regexec(&_glfw.linjs.regex,entry->d_name,1,&match,0) != 0) continue;

                char path[PATH_MAX];
                snprintf(path, sizeof(path),"%s/%s",dirname,entry->d_name);
                if (openJoystickDevice(path)) count++;
            }

            closedir(dir);
        }

        qsort(_glfw.joysticks,count,sizeof(_GLFWjoystick),compareJoysticks);
        return  1;
    }

    GLFWbool _glfwPollJoystickLinux(_GLFWjoystick* js) {
        for (;;) {
            struct input_event e;
            if (read(js->linjs.fd,&e,sizeof(e)) < 0) { closeJoystick(js); break; }

            if (e.type == EV_SYN) {
                if (e.code == 3/*sync dropped*/) _glfw.linjs.dropped =  1;
                else if (e.code == SYN_REPORT) { _glfw.linjs.dropped = 0; pollAbsState(js); }
            }

            if (_glfw.linjs.dropped) continue;

            if (e.type == EV_KEY) handleKeyEvent(js, e.code, e.value);
            else if (e.type == EV_ABS) handleAbsEvent(js, e.code, e.value);
        }

        return js->connected;
    }

    static int getGLXFBConfigAttrib(GLXFBConfig fbconfig, int attrib) { int value; _glfw.glx.GetFBConfigAttrib(_glfw.x11.display, fbconfig, attrib, &value); return value; }
    static GLXContext createLegacyContextGLX(_GLFWwindow* window, GLXFBConfig fbconfig, GLXContext share) { (void)window; return _glfw.glx.CreateNewContext(_glfw.x11.display,fbconfig,0x8014/*rgba type*/,share,True); }
    static void makeContextCurrentGLX(_GLFWwindow* window) { _glfw.glx.MakeCurrent(_glfw.x11.display,window->context.glx.window,window->context.glx.handle); _glfwPlatformSetTls(&_glfw.contextSlot,window); }
    static void swapBuffersGLX(_GLFWwindow* window) { _glfw.glx.SwapBuffers(_glfw.x11.display, window->context.glx.window); }
    static void swapIntervalGLX(int interval) { _GLFWwindow* window = _glfwPlatformGetTls(&_glfw.contextSlot); _glfw.glx.SwapIntervalEXT(_glfw.x11.display,window->context.glx.window,interval); }
    static int extensionSupportedGLX(const char* extension) {
        const char* extensions = _glfw.glx.QueryExtensionsString(_glfw.x11.display, _glfw.x11.screen);
        if (extensions) {
            if (_glfwStringInExtensionString(extension, extensions)) return  1;
        }

        return 0;
    }

    static GLFWglproc getProcAddressGLX(const char* procname) {
        if (_glfw.glx.GetProcAddress) return _glfw.glx.GetProcAddress((const GLubyte*) procname);
        else if (_glfw.glx.GetProcAddressARB) return _glfw.glx.GetProcAddressARB((const GLubyte*) procname);
        else return _glfwPlatformGetModuleSymbol(_glfw.glx.handle, procname);
    }

    #define PLATFORM_getCursorPos(w,x,y)            _glfwGetCursorPosX11(w,x,y)
    #define PLATFORM_setCursorPos(w,x,y)            _glfwSetCursorPosX11(w,x,y)
    #define PLATFORM_setCursorMode(w)               _glfwSetCursorModeX11(w)
    #define PLATFORM_initJoysticks()                _glfwInitJoysticksLinux()
    #define PLATFORM_pollJoystick(js)               _glfwPollJoystickLinux(js)
    #define PLATFORM_getMonitorPos(m,x,y)           _glfwGetMonitorPosX11(m,x,y)
    #define PLATFORM_getMonitorWorkarea(m,x,y,w,h)  _glfwGetMonitorWorkareaX11(m,x,y,w,h)
    #define PLATFORM_getVideoMode(m,cur)            _glfwGetVideoModeX11(m,cur)
    #define PLATFORM_setWindowIcon(w,i)             _glfwSetWindowIconX11(w,i)
    #define PLATFORM_getWindowPos(w,x,y)            _glfwGetWindowPosX11(w,x,y)
    #define PLATFORM_setWindowPos(w,x,y)            _glfwSetWindowPosX11(w,x,y)
    #define PLATFORM_getWindowSize(w,wi,h)          _glfwGetWindowSizeX11(w,wi,h)
    #define PLATFORM_setWindowSize(w,wi,h)          _glfwSetWindowSizeX11(w,wi,h)
    #define PLATFORM_setWindowMonitor(w,x,y,wi,h)   _glfwSetWindowMonitorX11(w,x,y,wi,h)
    #define PLATFORM_setWindowDecorated(w,v)        _glfwSetWindowDecoratedX11(w,v)
    #define PLATFORM_pollEvents()                   _glfwPollEventsX11()
#endif
_GLFWlibrary _glfw={0};
int WindowInit(void) {
    __builtin_memset(&_glfw,0,sizeof(_glfw));
    #if defined(WINDOWS)
        GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(const WCHAR*)&_glfw,(HMODULE*)&_glfw.win32.instance);
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
        createKeyTables();
        MSG msg; WNDCLASSEXW wc={0}; wc.cbSize=sizeof(wc); // Start making of a helper window
        wc.style = CS_OWNDC; wc.lpfnWndProc = (WNDPROC) helperWindowProc; wc.hInstance = _glfw.win32.instance; wc.lpszClassName = L"GLFW3 Helper";
        _glfw.win32.helperWindowClass = RegisterClassExW(&wc);
        _glfw.win32.helperWindowHandle = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW,(LPCWSTR)MAKEINTATOM(_glfw.win32.helperWindowClass),L"GLFW message window",WS_CLIPSIBLINGS|WS_CLIPCHILDREN,0,0,1,1,NULL,NULL,_glfw.win32.instance,NULL);
        ShowWindow(_glfw.win32.helperWindowHandle, SW_HIDE);
        DEV_BROADCAST_DEVICEINTERFACE_W dbi;
        ZeroMemory(&dbi, sizeof(dbi));
        dbi.dbcc_size = sizeof(dbi);
        dbi.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
        dbi.dbcc_classguid = (GUID){0x4d1e55b2,0xf16f,0x11cf,{0x88,0xcb,0x00,0x11,0x11,0x00,0x00,0x30}};
        _glfw.win32.deviceNotificationHandle = RegisterDeviceNotificationW(_glfw.win32.helperWindowHandle,(DEV_BROADCAST_HDR*)&dbi,DEVICE_NOTIFY_WINDOW_HANDLE);
        while (PeekMessageW(&msg, _glfw.win32.helperWindowHandle, 0, 0, PM_REMOVE)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
        _glfwPollMonitorsWin32();
    #else
        void* module = _glfwPlatformLoadModule("libX11.so.6");
        PFN_XInitThreads XInitThreads = (PFN_XInitThreads)_glfwPlatformGetModuleSymbol(module,"XInitThreads");
        PFN_XOpenDisplay XOpenDisplay = (PFN_XOpenDisplay)_glfwPlatformGetModuleSymbol(module,"XOpenDisplay");
        XInitThreads();
        Display* display = XOpenDisplay(NULL);
        _glfw.x11.display = display;
        _glfw.x11.xlib.handle = module;
        _glfw.x11.xlib.AllocSizeHints = (PFN_XAllocSizeHints)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle,"XAllocSizeHints");
        _glfw.x11.xlib.ChangeProperty = (PFN_XChangeProperty)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle,"XChangeProperty");
        _glfw.x11.xlib.ChangeWindowAttributes = (PFN_XChangeWindowAttributes)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle,"XChangeWindowAttributes");
        _glfw.x11.xlib.CheckTypedWindowEvent = (PFN_XCheckTypedWindowEvent)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle,"XCheckTypedWindowEvent");
        _glfw.x11.xlib.CreateColormap = (PFN_XCreateColormap)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle,"XCreateColormap");
        _glfw.x11.xlib.CreateWindow = (PFN_XCreateWindow)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle,"XCreateWindow");
        _glfw.x11.xlib.DefineCursor = (PFN_XDefineCursor)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle,"XDefineCursor");
        _glfw.x11.xlib.DeleteProperty = (PFN_XDeleteProperty)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle,"XDeleteProperty");
        _glfw.x11.xlib.DisplayKeycodes = (PFN_XDisplayKeycodes)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle,"XDisplayKeycodes");
        _glfw.x11.xlib.FilterEvent = (PFN_XFilterEvent)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle,"XFilterEvent");
        _glfw.x11.xlib.FindContext = (PFN_XFindContext)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle,"XFindContext");
        _glfw.x11.xlib.Free = (PFN_XFree)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle,"XFree");
        _glfw.x11.xlib.FreeEventData = (PFN_XFreeEventData)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle,"XFreeEventData");
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
        _glfw.x11.xlib.ResizeWindow = (PFN_XResizeWindow)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XResizeWindow");
        _glfw.x11.xlib.SaveContext = (PFN_XSaveContext)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XSaveContext");
        _glfw.x11.xlib.SendEvent = (PFN_XSendEvent)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XSendEvent");
        _glfw.x11.xlib.SetICFocus = (PFN_XSetICFocus)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XSetICFocus");
        _glfw.x11.xlib.SetInputFocus = (PFN_XSetInputFocus)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XSetInputFocus");
        _glfw.x11.xlib.SetWMNormalHints = (PFN_XSetWMNormalHints)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XSetWMNormalHints");
        _glfw.x11.xlib.SetWMProtocols = (PFN_XSetWMProtocols)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XSetWMProtocols");
        _glfw.x11.xlib.TranslateCoordinates = (PFN_XTranslateCoordinates)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XTranslateCoordinates");
        _glfw.x11.xlib.UndefineCursor = (PFN_XUndefineCursor)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XUndefineCursor");
        _glfw.x11.xlib.UngrabPointer = (PFN_XUngrabPointer)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XUngrabPointer");
        _glfw.x11.xlib.UnsetICFocus = (PFN_XUnsetICFocus)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XUnsetICFocus");
        _glfw.x11.xlib.WarpPointer = (PFN_XWarpPointer)_glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XWarpPointer");
        _glfw.x11.screen = DefaultScreen(_glfw.x11.display);
        _glfw.x11.root = RootWindow(_glfw.x11.display,_glfw.x11.screen);
        static XContext lastContext = 0;
        _glfw.x11.context = ++lastContext;
        _glfw.x11.randr.handle = _glfwPlatformLoadModule("libXrandr.so.2");
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
        XRRScreenResources* sr = XRRGetScreenResourcesCurrent(_glfw.x11.display,_glfw.x11.root);
        XRRFreeScreenResources(sr);
        XRRSelectInput(_glfw.x11.display,_glfw.x11.root,(1L << 2)/*change notify mask*/);
        _glfw.x11.xcursor.handle = _glfwPlatformLoadModule("libXcursor.so.1");
        _glfw.x11.xcursor.ImageCreate      = (PFN_XcursorImageCreate)      _glfwPlatformGetModuleSymbol(_glfw.x11.xcursor.handle,"XcursorImageCreate");
        _glfw.x11.xcursor.ImageDestroy     = (PFN_XcursorImageDestroy)     _glfwPlatformGetModuleSymbol(_glfw.x11.xcursor.handle,"XcursorImageDestroy");
        _glfw.x11.xcursor.ImageLoadCursor  = (PFN_XcursorImageLoadCursor)  _glfwPlatformGetModuleSymbol(_glfw.x11.xcursor.handle,"XcursorImageLoadCursor");
        createKeyTables();
        #define IA(n) _glfw.x11.xlib.InternAtom(_glfw.x11.display, n, False)
        _glfw.x11.UTF8_STRING        = IA("UTF8_STRING");
        _glfw.x11.WM_PROTOCOLS       = IA("WM_PROTOCOLS");
        _glfw.x11.WM_STATE           = IA("WM_STATE");
        _glfw.x11.WM_DELETE_WINDOW   = IA("WM_DELETE_WINDOW");
        _glfw.x11.NET_SUPPORTED          = IA("_NET_SUPPORTED");
        _glfw.x11.NET_SUPPORTING_WM_CHECK= IA("_NET_SUPPORTING_WM_CHECK");
        _glfw.x11.NET_WM_ICON            = IA("_NET_WM_ICON");
        _glfw.x11.NET_WM_PING            = IA("_NET_WM_PING");
        _glfw.x11.NET_WM_NAME            = IA("_NET_WM_NAME");
        _glfw.x11.NET_WM_BYPASS_COMPOSITOR = IA("_NET_WM_BYPASS_COMPOSITOR");
        _glfw.x11.MOTIF_WM_HINTS         = IA("_MOTIF_WM_HINTS");
        #undef IA
        Window* wfr = NULL; _glfwGetWindowPropertyX11(_glfw.x11.root,_glfw.x11.NET_SUPPORTING_WM_CHECK,((Atom) 33),(unsigned char**)&wfr);
        Window* wfc = NULL; _glfwGetWindowPropertyX11(*wfr,_glfw.x11.NET_SUPPORTING_WM_CHECK,((Atom) 33),(unsigned char**)&wfc);
        _glfw.x11.xlib.Free(wfr); _glfw.x11.xlib.Free(wfc);
        Atom* sa = NULL; const unsigned long ac = _glfwGetWindowPropertyX11(_glfw.x11.root,_glfw.x11.NET_SUPPORTED,((Atom) 4),(unsigned char**)&sa);
        #define GA(name) getAtomIfSupported(sa, ac, name)
        _glfw.x11.NET_WM_STATE              = GA("_NET_WM_STATE");
        _glfw.x11.NET_WM_STATE_FULLSCREEN   = GA("_NET_WM_STATE_FULLSCREEN");
        _glfw.x11.NET_WM_WINDOW_TYPE        = GA("_NET_WM_WINDOW_TYPE");
        _glfw.x11.NET_WM_WINDOW_TYPE_NORMAL = GA("_NET_WM_WINDOW_TYPE_NORMAL");
        _glfw.x11.NET_WORKAREA              = GA("_NET_WORKAREA");
        _glfw.x11.NET_CURRENT_DESKTOP       = GA("_NET_CURRENT_DESKTOP");
        _glfw.x11.NET_ACTIVE_WINDOW         = GA("_NET_ACTIVE_WINDOW");
        #undef GA
        if (sa) _glfw.x11.xlib.Free(sa);
        XSetWindowAttributes wa; wa.event_mask = (1L<<22);
        _glfw.x11.helperWindowHandle = _glfw.x11.xlib.CreateWindow(_glfw.x11.display,_glfw.x11.root,0,0,1,1,0,0,2/*input only*/,DefaultVisual(_glfw.x11.display,_glfw.x11.screen),(1L<<11)/*event mask*/,&wa);
        XcursorImage* native = XcursorImageCreate(16,16); __builtin_memset(native->pixels,0,256*sizeof(XcursorUInt)); native->xhot=native->yhot=0;
        _glfw.x11.hiddenCursorHandle = XcursorImageLoadCursor(_glfw.x11.display,native); XcursorImageDestroy(native);
        _glfwPollMonitorsX11();
    #endif
    _glfwPlatformCreateTls(&_glfw.contextSlot);
    _glfwPlatformInitTimer();
    _glfw.timer.offset = _glfwPlatformGetTimerValue();
    return  1;
}

const _GLFWfbconfig* _glfwChooseFBConfig(const _GLFWfbconfig* alts, unsigned int count) {
    unsigned int missing, leastMissing = 2147483647, colorDiff, leastColorDiff = 2147483647, extraDiff, leastExtraDiff = 2147483647;
    const _GLFWfbconfig* closest = NULL;
    for (unsigned int i = 0; i < count; i++) {
        const _GLFWfbconfig* cur = alts + i;        
        missing = 0;
        if (cur->alphaBits == 0) missing++;
        if (cur->depthBits == 0) missing++;
        if (cur->stencilBits == 0) missing++;
        colorDiff = 0;
        colorDiff += (8 - cur->redBits)   * (8 - cur->redBits);
        colorDiff += (8 - cur->greenBits) * (8 - cur->greenBits);
        colorDiff += (8 - cur->blueBits)  * (8 - cur->blueBits);
        extraDiff = 0;
        extraDiff += (8 - cur->alphaBits)   * (8 - cur->alphaBits);
        extraDiff += (8 - cur->depthBits)   * (8 - cur->depthBits);
        extraDiff += (8 - cur->stencilBits) * (8 - cur->stencilBits);
        if (missing < leastMissing || (missing == leastMissing && (colorDiff < leastColorDiff || (colorDiff == leastColorDiff && extraDiff < leastExtraDiff)))) closest = cur;
        if (cur == closest) { leastMissing = missing; leastColorDiff = colorDiff; leastExtraDiff = extraDiff; }
    }
    return closest;
}

GLFWbool _glfwStringInExtensionString(const char* string, const char* extensions) {
    const char* start = extensions;
    for (;;) {
        const char* where = strstr(start, string);
        if (!where) return 0;
        const char* terminator = where + GetStringLength(string);
        if ((where == start || *(where - 1) == ' ') && (*terminator == ' ' || *terminator == '\0')) break;
        start = terminator;
    }
    return  1;
}

void glfwMakeContextCurrent(GLFWwindow* handle) {
    _GLFWwindow* window = (_GLFWwindow*) handle;
    _GLFWwindow* previous = _glfwPlatformGetTls(&_glfw.contextSlot);
    if (previous && (!window || window->context.source != previous->context.source)) previous->context.makeCurrent(NULL);
    if (window) window->context.makeCurrent(window);
}

void glfwSwapBuffers(GLFWwindow* handle) { _GLFWwindow* window = (_GLFWwindow*)handle; window->context.swapBuffers(window); }
GLFWglproc glfwGetProcAddress(const char* procname) { _GLFWwindow* window = _glfwPlatformGetTls(&_glfw.contextSlot); return window->context.getProcAddress(procname); }
void _glfwInputMonitor(_GLFWmonitor* monitor, int action, int placement) {
    if (action == 0x00040001/*connected*/) {
        _glfw.monitorCount++;
        _glfw.monitors = _glfw.monitors ? realloc(_glfw.monitors,sizeof(_GLFWmonitor*) * _glfw.monitorCount) : calloc(_glfw.monitorCount,sizeof(_GLFWmonitor*));
        if (placement == 0) { memmove(_glfw.monitors + 1,_glfw.monitors,((size_t) _glfw.monitorCount - 1) * sizeof(_GLFWmonitor*)); _glfw.monitors[0] = monitor; }
        else _glfw.monitors[_glfw.monitorCount - 1] = monitor;
    } else if (action == 0x00040002/*disconnected*/) {
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

GLFWmonitor** glfwGetMonitors(int* count) { *count = _glfw.monitorCount; return (GLFWmonitor**) _glfw.monitors; }
GLFWmonitor* glfwGetPrimaryMonitor(void) { if (!_glfw.monitorCount) {return NULL;} return (GLFWmonitor*) _glfw.monitors[0]; }
void glfwGetMonitorPos(GLFWmonitor* handle, int* xpos, int* ypos) { *xpos = 0; *ypos = 0; _GLFWmonitor* monitor = (_GLFWmonitor*)handle; PLATFORM_getMonitorPos(monitor,xpos,ypos); }
void glfwGetMonitorWorkarea(GLFWmonitor* handle, int* xpos, int* ypos, int* width, int* height) { *xpos=*ypos=*width=*height=0; _GLFWmonitor* monitor = (_GLFWmonitor*)handle; PLATFORM_getMonitorWorkarea(monitor,xpos,ypos,width,height); }
const GLFWvidmode* glfwGetVideoMode(GLFWmonitor* handle) { _GLFWmonitor* monitor=(_GLFWmonitor*)handle; PLATFORM_getVideoMode(monitor,&monitor->currentMode); return &monitor->currentMode; }
void SetCursorMode(GLFWwindow* handle,int value) { _GLFWwindow* window = (_GLFWwindow*)handle; if (window->cursorMode != value){window->cursorMode = value; PLATFORM_getCursorPos(window,&window->virtualCursorPosX,&window->virtualCursorPosY); PLATFORM_setCursorMode(window);} }
void _glfwInputWindowFocus(_GLFWwindow* window, GLFWbool focused) {
    Sys_Input.window_has_focus = focused != 0; Sys_Input.ignore_next_mouse_delta = true;
    SetCursorMode((GLFWwindow*)window,Sys_Input.window_has_focus ? GLFW_CURSOR_DISABLED : 0x00034001/*GLFW_CURSOR_NORMAL*/);
    if (!focused) {
        for (int key = 0;  key <= GLFW_KEY_LAST;  key++) {
            if (window->keys[key] == GLFW_PRESS) _glfwInputKey(window,key,GLFW_RELEASE);
        }

        for (int button = 0;  button <= GLFW_MOUSE_BUTTON_LAST;  button++) {
            if (window->mouseButtons[button] == GLFW_PRESS)  _glfwInputMouseClick(window,button,GLFW_RELEASE);
        }
    }
}

GLFWwindow* glfwCreateWindow(int width, int height, char* title) {
    _GLFWwindow* window=calloc(1,sizeof(_GLFWwindow));
    window->next = _glfw.windowListHead;
    _glfw.windowListHead = window; window->videoMode.width=width; window->videoMode.height=height;
    window->videoMode.redBits=window->videoMode.greenBits=window->videoMode.blueBits=8;
    window->videoMode.refreshRate = GLFW_DONT_CARE; window->decorated =  1; window->cursorMode = 0x00034003/*disabled*/; window->doublebuffer= 1;
    window->minwidth = window->minheight = window->maxwidth = window->maxheight = GLFW_DONT_CARE;
#ifdef WINDOWS
    createNativeWindow(window,title,width,height);
    PIXELFORMATDESCRIPTOR pfd; HGLRC prc,rc; HDC pdc,dc;
    _glfw.wgl.instance = LoadLibraryA("opengl32.dll");
    _glfw.wgl.CreateContext = (PFN_wglCreateContext)_glfwPlatformGetModuleSymbol(_glfw.wgl.instance,"wglCreateContext");
    _glfw.wgl.GetProcAddress = (PFN_wglGetProcAddress)_glfwPlatformGetModuleSymbol(_glfw.wgl.instance,"wglGetProcAddress");
    _glfw.wgl.GetCurrentDC = (PFN_wglGetCurrentDC)_glfwPlatformGetModuleSymbol(_glfw.wgl.instance,"wglGetCurrentDC");
    _glfw.wgl.GetCurrentContext = (PFN_wglGetCurrentContext)_glfwPlatformGetModuleSymbol(_glfw.wgl.instance,"wglGetCurrentContext");
    _glfw.wgl.MakeCurrent = (PFN_wglMakeCurrent)_glfwPlatformGetModuleSymbol(_glfw.wgl.instance,"wglMakeCurrent");
    _glfw.wgl.ShareLists = (PFN_wglShareLists)_glfwPlatformGetModuleSymbol(_glfw.wgl.instance,"wglShareLists");
    dc = GetDC(_glfw.win32.helperWindowHandle);
    ZeroMemory(&pfd,sizeof(pfd)); pfd.nSize = sizeof(pfd); pfd.nVersion = 1; pfd.dwFlags = PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER; pfd.iPixelType = PFD_TYPE_RGBA; pfd.cColorBits = 24;
    SetPixelFormat(dc,ChoosePixelFormat(dc,&pfd),&pfd);
    rc = wglCreateContext(dc); pdc=wglGetCurrentDC(); prc=wglGetCurrentContext(); wglMakeCurrent(dc,rc);
    _glfw.wgl.GetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");
    _glfw.wgl.GetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
    _glfw.wgl.CreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
    _glfw.wgl.SwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
    _glfw.wgl.GetPixelFormatAttribivARB = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)wglGetProcAddress("wglGetPixelFormatAttribivARB");
    _glfw.wgl.ARB_create_context_profile = extensionSupportedWGL("WGL_ARB_create_context_profile");
    _glfw.wgl.EXT_swap_control = extensionSupportedWGL("WGL_EXT_swap_control");
    wglMakeCurrent(pdc,prc);
    int attribs[40],pixelFormat; PIXELFORMATDESCRIPTOR pfd2;
    window->context.wgl.dc = GetDC(window->win32.handle);
    pixelFormat = choosePixelFormatWGL(window);
    DescribePixelFormat(window->context.wgl.dc,pixelFormat,sizeof(pfd2),&pfd2); SetPixelFormat(window->context.wgl.dc,pixelFormat,&pfd2);
    int index=0,mask=0,flags=0; mask |= 0x00000001/*WGL_CONTEXT_CORE_PROFILE_BIT_ARB*/;
    attribs[index++] = 0x2091/*major*/; attribs[index++] = 4; // OpenGL 4.3
    attribs[index++] = 0x2092/*minor*/; attribs[index++] = 3; 
    if (flags) { attribs[index++] = 0x2094/*flags*/; attribs[index++] = flags; }
    if (mask) { attribs[index++] = 0x9126/*context profile mask*/; attribs[index++] = mask; }
    attribs[index++] = 0; attribs[index++] = 0;
    window->context.wgl.handle = wglCreateContextAttribsARB(window->context.wgl.dc,NULL,attribs);
    window->context.makeCurrent = makeContextCurrentWGL;
    window->context.swapBuffers = swapBuffersWGL;
    window->context.swapInterval = swapIntervalWGL;
    window->context.extensionSupported = extensionSupportedWGL;
    window->context.getProcAddress = getProcAddressWGL;
    int showCommand = SW_SHOWNA; ShowWindow(window->win32.handle,showCommand); BringWindowToTop(window->win32.handle); SetForegroundWindow(window->win32.handle); SetFocus(window->win32.handle);
#else
    const char* names[] = {"libGLX.so.0","libGL.so.1","libGL.so",NULL};
    for (int i=0;names[i] && !_glfw.glx.handle;i++) _glfw.glx.handle = _glfwPlatformLoadModule(names[i]);
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
    _glfw.glx.GetProcAddress = (PFNGLXGETPROCADDRESSPROC)_glfwPlatformGetModuleSymbol(_glfw.glx.handle,"glXGetProcAddress");
    _glfw.glx.GetProcAddressARB = (PFNGLXGETPROCADDRESSPROC)_glfwPlatformGetModuleSymbol(_glfw.glx.handle,"glXGetProcAddressARB");
    glXQueryExtension(_glfw.x11.display,&_glfw.glx.errorBase,&_glfw.glx.eventBase);
    glXQueryVersion(_glfw.x11.display,&_glfw.glx.major,&_glfw.glx.minor);
    if (extensionSupportedGLX("GLX_EXT_swap_control")) { _glfw.glx.SwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)getProcAddressGLX("glXSwapIntervalEXT"); if (_glfw.glx.SwapIntervalEXT) _glfw.glx.EXT_swap_control =  1; }
    if (extensionSupportedGLX("GLX_ARB_create_context")) { _glfw.glx.CreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)getProcAddressGLX("glXCreateContextAttribsARB"); if (_glfw.glx.CreateContextAttribsARB) _glfw.glx.ARB_create_context =  1; }
    if (extensionSupportedGLX("GLX_ARB_create_context_profile")) _glfw.glx.ARB_create_context_profile =  1;
    GLXFBConfig native; XVisualInfo* result;
    GLXFBConfig* nativeConfigs; _GLFWfbconfig* usableConfigs; const _GLFWfbconfig* closest; int nativeCount,usableCount;
    nativeConfigs = _glfw.glx.GetFBConfigs(_glfw.x11.display, _glfw.x11.screen, &nativeCount);        
    usableConfigs = calloc(nativeCount, sizeof(_GLFWfbconfig)); usableCount = 0;
    for (int i = 0;  i < nativeCount;  i++) {
        const GLXFBConfig n = nativeConfigs[i];
        _GLFWfbconfig* u = usableConfigs + usableCount;
        if (!(getGLXFBConfigAttrib(n,0x8011/*render type*/) & 0x00000001/*rgba bit*/)) continue;
        if (!(getGLXFBConfigAttrib(n,0x8010/*drawable type*/) & 0x00000001/*window bit*/)) continue;
        if (getGLXFBConfigAttrib(n,5) !=  1) continue;

        u->redBits = getGLXFBConfigAttrib(n,8); u->greenBits = getGLXFBConfigAttrib(n,9); u->blueBits = getGLXFBConfigAttrib(n,10); u->alphaBits = getGLXFBConfigAttrib(n,11); u->depthBits = getGLXFBConfigAttrib(n,12); u->stencilBits = getGLXFBConfigAttrib(n,13);
        u->accumRedBits = getGLXFBConfigAttrib(n,14); u->accumGreenBits = getGLXFBConfigAttrib(n,15); u->accumBlueBits = getGLXFBConfigAttrib(n,16); u->accumAlphaBits = getGLXFBConfigAttrib(n,17);
        if (getGLXFBConfigAttrib(n,6)) u->stereo =  1;
        u->handle = (uintptr_t) n;
        usableCount++;
    }

    closest = _glfwChooseFBConfig(usableConfigs,usableCount); native = (GLXFBConfig)closest->handle;
    _glfw.x11.xlib.Free(nativeConfigs); free(usableConfigs);
    result = _glfw.glx.GetVisualFromFBConfig(_glfw.x11.display,native);
    Visual* visual=result->visual; int depth = result->depth; _glfw.x11.xlib.Free(result);
    if (!visual) { visual=DefaultVisual(_glfw.x11.display,_glfw.x11.screen); depth=DefaultDepth(_glfw.x11.display,_glfw.x11.screen); }
    int xpos=0,ypos=0;
    window->x11.colormap=_glfw.x11.xlib.CreateColormap(_glfw.x11.display,_glfw.x11.root,visual,0);
    XSetWindowAttributes wa={0};
    wa.colormap=window->x11.colormap;
    wa.event_mask=(1L<<17)|(1L<<0)|(1L<<1)|(1L<<6)|(1L<<2)|(1L<<3)|(1L<<15)|(1L<<21)|(1L<<16)|(1L<<4)|(1L<<5)|(1L<<22);
    window->x11.parent=_glfw.x11.root;
    window->x11.handle=_glfw.x11.xlib.CreateWindow(_glfw.x11.display,_glfw.x11.root,xpos,ypos,width,height,0,depth,1/*output only*/,visual,(1L<<3)/*border pixel*/|(1L<<13)/*colormap*/|(1L<<11)/*event mask*/,&wa);
    _glfw.x11.xlib.SaveContext(_glfw.x11.display,window->x11.handle,_glfw.x11.context,(XPointer)window); // Needed to allow input.
    Atom protocols[]={_glfw.x11.WM_DELETE_WINDOW,_glfw.x11.NET_WM_PING};
    _glfw.x11.xlib.SetWMProtocols(_glfw.x11.display,window->x11.handle,protocols,sizeof(protocols)/sizeof(Atom));
    if (_glfw.x11.NET_WM_WINDOW_TYPE && _glfw.x11.NET_WM_WINDOW_TYPE_NORMAL) { Atom type=_glfw.x11.NET_WM_WINDOW_TYPE_NORMAL;
    _glfw.x11.xlib.ChangeProperty(_glfw.x11.display,window->x11.handle,_glfw.x11.NET_WM_WINDOW_TYPE,((Atom) 4),32,0/*PropModeReplace*/,(unsigned char*)&type,1); }
    XSizeHints* szhints=_glfw.x11.xlib.AllocSizeHints();
    szhints->flags|=((1L << 4)/*PMinSize*/|(1L << 5)/*PMaxSize*/); szhints->min_width=szhints->max_width=width; szhints->min_height=szhints->max_height=height;
    szhints->flags|=(1L << 9)/*PWinGravity*/; szhints->win_gravity=10/*static gravity*/;
    _glfw.x11.xlib.SetWMNormalHints(_glfw.x11.display,window->x11.handle,szhints); _glfw.x11.xlib.Free(szhints);
    _glfw.x11.xlib.ChangeProperty(_glfw.x11.display,window->x11.handle,_glfw.x11.NET_WM_NAME,_glfw.x11.UTF8_STRING,8,0/*PropModeReplace*/,(unsigned char*)title,GetStringLength(title)); // Set title
    _glfwGetWindowPosX11(window,&window->x11.xpos,&window->x11.ypos);
    _glfwGetWindowSizeX11(window,&window->x11.width,&window->x11.height);
    int attribs[40], index=0,flags=0;
    if (_glfw.glx.ARB_create_context) {
        int mask=0; mask |= 0x00000001/*core profile*/;
        attribs[index++] = 0x2091/*major*/; attribs[index++] = 4; attribs[index++] = 0x2092/*minor*/; attribs[index++] = 3; // OpenGL 4.3
        if (mask) { attribs[index++] = 0x9126/*profile mask arb*/; attribs[index++] = mask; }
        if (flags) { attribs[index++] = 0x2094/*context flags arb*/; attribs[index++] = flags; }
        attribs[index++] = 0L; attribs[index++] = 0L;
        window->context.glx.handle = _glfw.glx.CreateContextAttribsARB(_glfw.x11.display,native,NULL,True,attribs);
        window->context.glx.window = _glfw.glx.CreateWindow(_glfw.x11.display,native,window->x11.handle,NULL);
    } else window->context.glx.handle = createLegacyContextGLX(window,native,NULL);
    window->context.glx.fbconfig = native; window->context.makeCurrent = makeContextCurrentGLX;
    window->context.swapBuffers = swapBuffersGLX; window->context.swapInterval = swapIntervalGLX;
    window->context.extensionSupported = extensionSupportedGLX; window->context.getProcAddress = getProcAddressGLX;
    if (!_glfwWindowVisibleX11(window)) _glfw.x11.xlib.MapWindow(_glfw.x11.display,window->x11.handle);
    _glfwFocusWindowX11(window);
#endif
    return (GLFWwindow*)window;
}

void glfwSetWindowIcon(GLFWwindow* handle, const GLFWimage* images) { _GLFWwindow* window = (_GLFWwindow*) handle; PLATFORM_setWindowIcon(window,images); }
void glfwSetWindowPos(GLFWwindow* handle, int xpos, int ypos) { _GLFWwindow* window = (_GLFWwindow*)handle; PLATFORM_setWindowPos(window,xpos,ypos); }
void glfwSetWindowSize(GLFWwindow* handle, int width, int height) { _GLFWwindow* window = (_GLFWwindow*)handle; window->videoMode.width=width; window->videoMode.height=height; PLATFORM_setWindowSize(window,width,height); }
void glfwSetWindowMonitor(GLFWwindow* wh, int xpos, int ypos, int width, int height) { _GLFWwindow* window = (_GLFWwindow*)wh; window->videoMode.width=width; window->videoMode.height=height; PLATFORM_setWindowMonitor(window,xpos,ypos,width,height); }
void TextEntry(i32 k) {
    if (k == GLFW_KEY_U && Sys_Input.keyStates[GLFW_KEY_LEFT_CONTROL].down) { Sys_Global.playerName[0] = '\0'; currentPlayerNameLength = 0; return; }
    if (k == GLFW_KEY_ENTER || k == GLFW_KEY_KP_ENTER) { currentMenuItem++; return; }
    if (k == GLFW_KEY_BACKSPACE && currentPlayerNameLength > 0) { Sys_Global.playerName[--currentPlayerNameLength] = '\0'; return; }
    if (currentPlayerNameLength >= 26) return;
    char c = (k >= GLFW_KEY_A && k <= GLFW_KEY_Z) ? 'a' + (k - GLFW_KEY_A) : ((k >= GLFW_KEY_1 && k <= GLFW_KEY_9) ? '1' + (k - GLFW_KEY_1) : ((k == GLFW_KEY_0) ? '0' : ((k == GLFW_KEY_SPACE) ? ' ' : 0)));
    if (c) { Sys_Global.playerName[currentPlayerNameLength] = c; Sys_Global.playerName[++currentPlayerNameLength] = '\0'; }
}

void GoIntoGame(void); void ConsoleEmulator(i32 keycode); extern bool enteringPlayerName;
void _glfwInputKey(_GLFWwindow* window,int key,int action) {
    if (key >= 0 && key <= GLFW_KEY_LAST) {
        GLFWbool repeated = 0;
        if (action == GLFW_RELEASE && window->keys[key] == GLFW_RELEASE) return;
        if (action == GLFW_PRESS && window->keys[key] == GLFW_PRESS) repeated =  1;
        window->keys[key] = (char)action;
        if (repeated) action = GLFW_REPEAT;
    }

    if (!Sys_Input.window_has_focus) return;
    
    if (key == GLFW_KEY_F10 && action) OS_Exit(0); // Suppress warnings about unused parameters forced upon me by glfw3 dependency deadweight anchor.
    if (Sys_Global.menuActive && !returnToPause) {
        if ((key == GLFW_KEY_RIGHT_ALT || key == GLFW_KEY_LEFT_ALT) && action && Sys_Input.keyStates[GLFW_KEY_ENTER].down)                    GoIntoGame();
        if (key == GLFW_KEY_ENTER && action && (Sys_Input.keyStates[GLFW_KEY_LEFT_ALT].down || Sys_Input.keyStates[GLFW_KEY_RIGHT_ALT].down)) GoIntoGame();
    }

    if (key >=0 && key < MAX_KEYS && (action == GLFW_PRESS || (action == GLFW_REPEAT && !(key == GLFW_KEY_KP_ENTER || key == GLFW_KEY_ENTER || key == GLFW_KEY_TAB || key == GLFW_KEY_ESCAPE)))) {
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
    if (event == 0x00040001/*connected*/) js->connected =  1;
    else if (event == 0x00040002/*disconnected*/) js->connected = 0;
    
    int jid = (int)(js - _glfw.joysticks);
    if (jid > GLFW_JOYSTICK_LAST) return;
    bool connected = (event == 0x00040001/*connected*/);
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

_GLFWjoystick* _glfwAllocJoystick(const char* name,const char* guid,int axisCount,int buttonCount,int hatCount) {
    int jid; _GLFWjoystick* js;
    for (jid = 0; jid <= GLFW_JOYSTICK_LAST; jid++) { if (!_glfw.joysticks[jid].allocated) break; }
    if (jid > GLFW_JOYSTICK_LAST) return NULL;
    js = _glfw.joysticks + jid;
    js->allocated =  1; js->axisCount = axisCount; js->buttonCount = buttonCount; js->hatCount = hatCount;
    js->axes = calloc(axisCount,sizeof(float));
    js->buttons = calloc(buttonCount + (size_t)hatCount * 4,1);
    js->hats = calloc(hatCount,1);
    strncpy(js->name,name,sizeof(js->name)-1); strncpy(js->guid,guid,sizeof(js->guid)-1);
    return js;
}

void _glfwFreeJoystick(_GLFWjoystick* js) { free(js->axes); free(js->buttons); free(js->hats); __builtin_memset(js,0,sizeof(_GLFWjoystick)); }
