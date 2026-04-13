// GLFW 3.5 This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
#define _GNU_SOURCE
#include "internal.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <locale.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <X11/cursorfont.h>
#include <X11/Xmd.h>
#include <poll.h>
#include <math.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/inotify.h>
#include <sys/ioctl.h>
#include <time.h>
#include <dirent.h>
#include <dlfcn.h>
#define _NET_WM_STATE_REMOVE 0
#define _NET_WM_STATE_ADD    1
#define _NET_WM_STATE_TOGGLE 2
#define Button6 6
#define Button7 7
#define MWM_HINTS_DECORATIONS 2
#define MWM_DECOR_ALL         1
void _glfwPlatformInitTimer(void) {
    _glfw.timer.posix.clock = CLOCK_REALTIME;
    _glfw.timer.posix.frequency = 1000000000;
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) _glfw.timer.posix.clock = CLOCK_MONOTONIC;
}

uint64_t _glfwPlatformGetTimerValue(void) {
    struct timespec ts;
    clock_gettime(_glfw.timer.posix.clock, &ts);
    return (uint64_t) ts.tv_sec * _glfw.timer.posix.frequency + (uint64_t) ts.tv_nsec;
}

uint64_t _glfwPlatformGetTimerFrequency(void) { return _glfw.timer.posix.frequency; }

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
GLFWbool _glfwPollPOSIX(struct pollfd* fds, nfds_t count, double* timeout) {
    for (;;) {
        if (timeout) {
            const uint64_t base = _glfwPlatformGetTimerValue();
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
    struct { CARD32 state; Window icon; }* state=NULL;
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
        if (_glfw.x11.xinerama.available && _glfw.x11.NET_WM_FULLSCREEN_MONITORS)
            sendEventToWM(window,_glfw.x11.NET_WM_FULLSCREEN_MONITORS,window->monitor->x11.index,window->monitor->x11.index,window->monitor->x11.index,window->monitor->x11.index,0);
        if (_glfw.x11.NET_WM_STATE && _glfw.x11.NET_WM_STATE_FULLSCREEN)
            sendEventToWM(window,_glfw.x11.NET_WM_STATE,_NET_WM_STATE_ADD,_glfw.x11.NET_WM_STATE_FULLSCREEN,0,1,0);
        else {
            XSetWindowAttributes attributes; attributes.override_redirect=True;
            XChangeWindowAttributes(_glfw.x11.display,window->x11.handle,CWOverrideRedirect,&attributes);
            window->x11.overrideRedirect=GLFW_TRUE;
        }
        if (!window->x11.transparent) { const unsigned long value=1; XChangeProperty(_glfw.x11.display,window->x11.handle,_glfw.x11.NET_WM_BYPASS_COMPOSITOR,XA_CARDINAL,32,PropModeReplace,(unsigned char*)&value,1); }
    } else {
        if (_glfw.x11.xinerama.available && _glfw.x11.NET_WM_FULLSCREEN_MONITORS)
            XDeleteProperty(_glfw.x11.display,window->x11.handle,_glfw.x11.NET_WM_FULLSCREEN_MONITORS);
        if (_glfw.x11.NET_WM_STATE && _glfw.x11.NET_WM_STATE_FULLSCREEN)
            sendEventToWM(window,_glfw.x11.NET_WM_STATE,_NET_WM_STATE_REMOVE,_glfw.x11.NET_WM_STATE_FULLSCREEN,0,1,0);
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

static GLFWbool createNativeWindow(_GLFWwindow* window,const _GLFWwndconfig* wndconfig,Visual* visual,int depth) {
    int width=wndconfig->width,height=wndconfig->height;
    if (wndconfig->scaleToMonitor) { width*=_glfw.x11.contentScaleX; height*=_glfw.x11.contentScaleY; }
    width=_glfw_max(1,width); height=_glfw_max(1,height);
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

GLFWbool _glfwCreateWindowX11(_GLFWwindow* window,const _GLFWwndconfig* wndconfig,const _GLFWctxconfig* ctxconfig,const _GLFWfbconfig* fbconfig) {
    Visual* visual=NULL; int depth;
    if (ctxconfig->client!=GLFW_NO_API && ctxconfig->source==GLFW_NATIVE_CONTEXT_API) {
        if (!_glfwInitGLX()) return GLFW_FALSE;
        if (!_glfwChooseVisualGLX(wndconfig,ctxconfig,fbconfig,&visual,&depth)) return GLFW_FALSE;
    }
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
        _glfw_free(icon);
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
    width=_glfw_max(1,width); height=_glfw_max(1,height);
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
        CARD32* value=NULL;
        if (_glfwGetWindowPropertyX11(window->x11.handle,_glfw.x11.NET_WM_WINDOW_OPACITY,XA_CARDINAL,(unsigned char**)&value)) opacity=(float)(*value/(double)0xffffffffu);
        if (value) XFree(value);
    }
    return opacity;
}

void _glfwSetWindowOpacityX11(_GLFWwindow* window,float opacity) {
    const CARD32 value=(CARD32)(0xffffffffu*(double)opacity);
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

GLFWbool _glfwCreateCursorX11(_GLFWcursor* cursor,const GLFWimage* image,int xhot,int yhot) {
    cursor->x11.handle=_glfwCreateNativeCursorX11(image,xhot,yhot);
    return cursor->x11.handle ? GLFW_TRUE : GLFW_FALSE;
}

GLFWbool _glfwCreateStandardCursorX11(_GLFWcursor* cursor,int shape) {
    if (_glfw.x11.xcursor.handle) {
        char* theme=XcursorGetTheme(_glfw.x11.display);
        if (theme) {
            const int size=XcursorGetDefaultSize(_glfw.x11.display);
            const char* name=NULL;
            switch (shape) {
                case GLFW_ARROW_CURSOR:         name="default"; break;
                case GLFW_IBEAM_CURSOR:         name="text"; break;
                case GLFW_CROSSHAIR_CURSOR:     name="crosshair"; break;
                case GLFW_POINTING_HAND_CURSOR: name="pointer"; break;
                case GLFW_RESIZE_EW_CURSOR:     name="ew-resize"; break;
                case GLFW_RESIZE_NS_CURSOR:     name="ns-resize"; break;
                case GLFW_RESIZE_NWSE_CURSOR:   name="nwse-resize"; break;
                case GLFW_RESIZE_NESW_CURSOR:   name="nesw-resize"; break;
                case GLFW_RESIZE_ALL_CURSOR:    name="all-scroll"; break;
                case GLFW_NOT_ALLOWED_CURSOR:   name="not-allowed"; break;
            }
            XcursorImage* image=XcursorLibraryLoadImage(name,theme,size);
            if (image) { cursor->x11.handle=XcursorImageLoadCursor(_glfw.x11.display,image); XcursorImageDestroy(image); }
        }
    }
    if (!cursor->x11.handle) {
        unsigned int native=0;
        switch (shape) {
            case GLFW_ARROW_CURSOR:         native=XC_left_ptr; break;
            case GLFW_IBEAM_CURSOR:         native=XC_xterm; break;
            case GLFW_CROSSHAIR_CURSOR:     native=XC_crosshair; break;
            case GLFW_POINTING_HAND_CURSOR: native=XC_hand2; break;
            case GLFW_RESIZE_EW_CURSOR:     native=XC_sb_h_double_arrow; break;
            case GLFW_RESIZE_NS_CURSOR:     native=XC_sb_v_double_arrow; break;
            case GLFW_RESIZE_ALL_CURSOR:    native=XC_fleur; break;
            default: _glfwInputError(GLFW_CURSOR_UNAVAILABLE,"X11: Standard cursor shape unavailable"); return GLFW_FALSE;
        }
        cursor->x11.handle=XCreateFontCursor(_glfw.x11.display,native);
        if (!cursor->x11.handle) { _glfwInputError(GLFW_PLATFORM_ERROR,"X11: Failed to create standard cursor"); return GLFW_FALSE; }
    }
    return GLFW_TRUE;
}

void _glfwDestroyCursorX11(_GLFWcursor* cursor) { if (cursor->x11.handle) XFreeCursor(_glfw.x11.display,cursor->x11.handle); }

void _glfwSetCursorX11(_GLFWwindow* window,_GLFWcursor* cursor) {
    (void)cursor;
    if (window->cursorMode==GLFW_CURSOR_NORMAL || window->cursorMode==GLFW_CURSOR_CAPTURED) { updateCursorImage(window); XFlush(_glfw.x11.display); }
}

static GLFWbool modeIsGood(const XRRModeInfo* mi) { return (mi->modeFlags & RR_Interlace) == 0; }

// Calculates the refresh rate, in Hz, from the specified RandR mode info
//
static int calculateRefreshRate(const XRRModeInfo* mi)
{
    if (mi->hTotal && mi->vTotal)
        return (int) round((double) mi->dotClock / ((double) mi->hTotal * (double) mi->vTotal));
    else
        return 0;
}

// Returns the mode info for a RandR mode XID
//
static const XRRModeInfo* getModeInfo(const XRRScreenResources* sr, RRMode id)
{
    for (int i = 0;  i < sr->nmode;  i++)
    {
        if (sr->modes[i].id == id)
            return sr->modes + i;
    }

    return NULL;
}

// Convert RandR mode info to GLFW video mode
//
static GLFWvidmode vidmodeFromModeInfo(const XRRModeInfo* mi,
                                       const XRRCrtcInfo* ci)
{
    GLFWvidmode mode;

    if (ci->rotation == RR_Rotate_90 || ci->rotation == RR_Rotate_270)
    {
        mode.width  = mi->height;
        mode.height = mi->width;
    }
    else
    {
        mode.width  = mi->width;
        mode.height = mi->height;
    }

    mode.refreshRate = calculateRefreshRate(mi);

    _glfwSplitBPP(DefaultDepth(_glfw.x11.display, _glfw.x11.screen),
                  &mode.redBits, &mode.greenBits, &mode.blueBits);

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
            memcpy(disconnected,_glfw.monitors,_glfw.monitorCount * sizeof(_GLFWmonitor*));
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

        _glfw_free(disconnected);
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
void _glfwGetMonitorWorkareaX11(_GLFWmonitor* monitor,
                                int* xpos, int* ypos,
                                int* width, int* height)
{
    int areaX = 0, areaY = 0, areaWidth = 0, areaHeight = 0;

    if (_glfw.x11.randr.available && !_glfw.x11.randr.monitorBroken)
    {
        XRRScreenResources* sr =
            XRRGetScreenResourcesCurrent(_glfw.x11.display, _glfw.x11.root);
        XRRCrtcInfo* ci = XRRGetCrtcInfo(_glfw.x11.display, sr, monitor->x11.crtc);

        areaX = ci->x;
        areaY = ci->y;

        const XRRModeInfo* mi = getModeInfo(sr, ci->mode);

        if (ci->rotation == RR_Rotate_90 || ci->rotation == RR_Rotate_270)
        {
            areaWidth  = mi->height;
            areaHeight = mi->width;
        }
        else
        {
            areaWidth  = mi->width;
            areaHeight = mi->height;
        }

        XRRFreeCrtcInfo(ci);
        XRRFreeScreenResources(sr);
    }
    else
    {
        areaWidth  = DisplayWidth(_glfw.x11.display, _glfw.x11.screen);
        areaHeight = DisplayHeight(_glfw.x11.display, _glfw.x11.screen);
    }

    if (_glfw.x11.NET_WORKAREA && _glfw.x11.NET_CURRENT_DESKTOP)
    {
        Atom* extents = NULL;
        Atom* desktop = NULL;
        const unsigned long extentCount =
            _glfwGetWindowPropertyX11(_glfw.x11.root,
                                      _glfw.x11.NET_WORKAREA,
                                      XA_CARDINAL,
                                      (unsigned char**) &extents);

        if (_glfwGetWindowPropertyX11(_glfw.x11.root,
                                      _glfw.x11.NET_CURRENT_DESKTOP,
                                      XA_CARDINAL,
                                      (unsigned char**) &desktop) > 0)
        {
            if (extentCount >= 4 && *desktop < extentCount / 4)
            {
                const int globalX = extents[*desktop * 4 + 0];
                const int globalY = extents[*desktop * 4 + 1];
                const int globalWidth  = extents[*desktop * 4 + 2];
                const int globalHeight = extents[*desktop * 4 + 3];

                if (areaX < globalX)
                {
                    areaWidth -= globalX - areaX;
                    areaX = globalX;
                }

                if (areaY < globalY)
                {
                    areaHeight -= globalY - areaY;
                    areaY = globalY;
                }

                if (areaX + areaWidth > globalX + globalWidth)
                    areaWidth = globalX - areaX + globalWidth;
                if (areaY + areaHeight > globalY + globalHeight)
                    areaHeight = globalY - areaY + globalHeight;
            }
        }

        if (extents)
            XFree(extents);
        if (desktop)
            XFree(desktop);
    }

    if (xpos)
        *xpos = areaX;
    if (ypos)
        *ypos = areaY;
    if (width)
        *width = areaWidth;
    if (height)
        *height = areaHeight;
}

GLFWvidmode* _glfwGetVideoModesX11(_GLFWmonitor* monitor, int* count)
{
    GLFWvidmode* result;

    *count = 0;

    if (_glfw.x11.randr.available && !_glfw.x11.randr.monitorBroken)
    {
        XRRScreenResources* sr =
            XRRGetScreenResourcesCurrent(_glfw.x11.display, _glfw.x11.root);
        XRRCrtcInfo* ci = XRRGetCrtcInfo(_glfw.x11.display, sr, monitor->x11.crtc);
        XRROutputInfo* oi = XRRGetOutputInfo(_glfw.x11.display, sr, monitor->x11.output);

        result = _glfw_calloc(oi->nmode, sizeof(GLFWvidmode));

        for (int i = 0;  i < oi->nmode;  i++)
        {
            const XRRModeInfo* mi = getModeInfo(sr, oi->modes[i]);
            if (!modeIsGood(mi))
                continue;

            const GLFWvidmode mode = vidmodeFromModeInfo(mi, ci);
            int j;

            for (j = 0;  j < *count;  j++)
            {
                if (_glfwCompareVideoModes(result + j, &mode) == 0)
                    break;
            }

            // Skip duplicate modes
            if (j < *count)
                continue;

            (*count)++;
            result[*count - 1] = mode;
        }

        XRRFreeOutputInfo(oi);
        XRRFreeCrtcInfo(ci);
        XRRFreeScreenResources(sr);
    }
    else
    {
        *count = 1;
        result = _glfw_calloc(1, sizeof(GLFWvidmode));
        _glfwGetVideoModeX11(monitor, result);
    }

    return result;
}

GLFWbool _glfwGetVideoModeX11(_GLFWmonitor* monitor, GLFWvidmode* mode) {
    if (_glfw.x11.randr.available && !_glfw.x11.randr.monitorBroken) {
        XRRScreenResources* sr = XRRGetScreenResourcesCurrent(_glfw.x11.display, _glfw.x11.root);
        const XRRModeInfo* mi = NULL;
        XRRCrtcInfo* ci = XRRGetCrtcInfo(_glfw.x11.display, sr, monitor->x11.crtc);
        if (ci) {
            mi = getModeInfo(sr, ci->mode);
            if (mi) *mode = vidmodeFromModeInfo(mi, ci);
            XRRFreeCrtcInfo(ci);
        }

        XRRFreeScreenResources(sr);
        if (!mi) { _glfwInputError(GLFW_PLATFORM_ERROR, "X11: Failed to query video mode"); return GLFW_FALSE; }
    } else {
        mode->width = DisplayWidth(_glfw.x11.display, _glfw.x11.screen);
        mode->height = DisplayHeight(_glfw.x11.display, _glfw.x11.screen);
        mode->refreshRate = 0;
        _glfwSplitBPP(DefaultDepth(_glfw.x11.display, _glfw.x11.screen),&mode->redBits, &mode->greenBits, &mode->blueBits);
    }

    return GLFW_TRUE;
}

GLFWbool _glfwGetGammaRampX11(_GLFWmonitor* monitor, GLFWgammaramp* ramp) {
    if (_glfw.x11.randr.available && !_glfw.x11.randr.gammaBroken) {
        const size_t size = XRRGetCrtcGammaSize(_glfw.x11.display,monitor->x11.crtc);
        XRRCrtcGamma* gamma = XRRGetCrtcGamma(_glfw.x11.display,monitor->x11.crtc);
        _glfwAllocGammaArrays(ramp, size);
        memcpy(ramp->red,  gamma->red,  size * sizeof(unsigned short));
        memcpy(ramp->green,gamma->green,size * sizeof(unsigned short));
        memcpy(ramp->blue, gamma->blue, size * sizeof(unsigned short));
        XRRFreeGamma(gamma);
        return GLFW_TRUE;
    } else if (_glfw.x11.vidmode.available) {
        int size;
        XF86VidModeGetGammaRampSize(_glfw.x11.display, _glfw.x11.screen, &size);
        _glfwAllocGammaArrays(ramp, size);
        XF86VidModeGetGammaRamp(_glfw.x11.display,_glfw.x11.screen,ramp->size,ramp->red,ramp->green,ramp->blue);
        return GLFW_TRUE;
    } else { _glfwInputError(GLFW_PLATFORM_ERROR,"X11: Gamma ramp access not supported by server"); return GLFW_FALSE; }
}

void _glfwSetGammaRampX11(_GLFWmonitor* monitor, const GLFWgammaramp* ramp) {
    if (_glfw.x11.randr.available && !_glfw.x11.randr.gammaBroken) {
        if (XRRGetCrtcGammaSize(_glfw.x11.display, monitor->x11.crtc) != (int)ramp->size) { _glfwInputError(GLFW_PLATFORM_ERROR,"X11: Gamma ramp size must match current ramp size"); return; }

        XRRCrtcGamma* gamma = XRRAllocGamma(ramp->size);
        memcpy(gamma->red,   ramp->red,   ramp->size * sizeof(unsigned short));
        memcpy(gamma->green, ramp->green, ramp->size * sizeof(unsigned short));
        memcpy(gamma->blue,  ramp->blue,  ramp->size * sizeof(unsigned short));
        XRRSetCrtcGamma(_glfw.x11.display, monitor->x11.crtc, gamma);
        XRRFreeGamma(gamma);
    } else if (_glfw.x11.vidmode.available) {
        XF86VidModeSetGammaRamp(_glfw.x11.display,_glfw.x11.screen,ramp->size,(unsigned short*)ramp->red,(unsigned short*)ramp->green,(unsigned short*)ramp->blue);
    } else _glfwInputError(GLFW_PLATFORM_ERROR,"X11: Gamma ramp access not supported by server");
}

GLFWAPI RRCrtc glfwGetX11Adapter(GLFWmonitor* handle) {
    _GLFW_REQUIRE_INIT_OR_RETURN(None);
    if (_glfw.platform.platformID != GLFW_PLATFORM_X11) { _glfwInputError(GLFW_PLATFORM_UNAVAILABLE, "X11: Platform not initialized"); return None; }

    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;
    return monitor->x11.crtc;
}

GLFWAPI RROutput glfwGetX11Monitor(GLFWmonitor* handle) {
    _GLFW_REQUIRE_INIT_OR_RETURN(None);
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

static Window createHelperWindow(void) {
    XSetWindowAttributes wa; wa.event_mask = PropertyChangeMask;
    return XCreateWindow(_glfw.x11.display, _glfw.x11.root, 0,0,1,1,0,0, InputOnly,
                         DefaultVisual(_glfw.x11.display, _glfw.x11.screen), CWEventMask, &wa);
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

Cursor _glfwCreateNativeCursorX11(const GLFWimage* image, int xhot, int yhot) {
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
        .initJoysticks = _glfwInitJoysticksLinux,
        .pollJoystick = _glfwPollJoystickLinux, .getMappingName = _glfwGetMappingNameLinux,
        .updateGamepadGUID = _glfwUpdateGamepadGUIDLinux,
        .getMonitorPos = _glfwGetMonitorPosX11,
        .getMonitorContentScale = _glfwGetMonitorContentScaleX11, .getMonitorWorkarea = _glfwGetMonitorWorkareaX11,
        .getVideoModes = _glfwGetVideoModesX11, .getVideoMode = _glfwGetVideoModeX11,
        .getGammaRamp = _glfwGetGammaRampX11, .setGammaRamp = _glfwSetGammaRampX11,
        .createWindow = _glfwCreateWindowX11,
        .setWindowTitle = _glfwSetWindowTitleX11, .setWindowIcon = _glfwSetWindowIconX11,
        .getWindowPos = _glfwGetWindowPosX11, .setWindowPos = _glfwSetWindowPosX11,
        .getWindowSize = _glfwGetWindowSizeX11, .setWindowSize = _glfwSetWindowSizeX11,
        .getWindowFrameSize = _glfwGetWindowFrameSizeX11,
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
    if (!module) { _glfwInputError(GLFW_PLATFORM_ERROR, "X11: Failed to load Xlib"); return GLFW_FALSE; }

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

int _glfwInitX11(void) {
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
    _glfw.x11.xlib.UnregisterIMInstantiateCallback = (PFN_XUnregisterIMInstantiateCallback) _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "XUnregisterIMInstantiateCallback");
    _glfw.x11.xlib.utf8LookupString = (PFN_Xutf8LookupString) _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "Xutf8LookupString");
    _glfw.x11.xlib.utf8SetWMProperties = (PFN_Xutf8SetWMProperties) _glfwPlatformGetModuleSymbol(_glfw.x11.xlib.handle, "Xutf8SetWMProperties");
    if (_glfw.x11.xlib.utf8LookupString && _glfw.x11.xlib.utf8SetWMProperties) _glfw.x11.xlib.utf8 = GLFW_TRUE;
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
    if (id.vendor && id.product && id.version) {
        sprintf(guid, "%02x%02x0000%02x%02x0000%02x%02x0000%02x%02x0000",
                id.bustype & 0xff, id.bustype >> 8,
                id.vendor & 0xff,  id.vendor >> 8,
                id.product & 0xff, id.product >> 8,
                id.version & 0xff, id.version >> 8);
    } else {
        sprintf(guid, "%02x%02x0000%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x00",
                id.bustype & 0xff, id.bustype >> 8,
                name[0], name[1], name[2], name[3],
                name[4], name[5], name[6], name[7],
                name[8], name[9], name[10]);
    }

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
    memcpy(&js->linjs, &linjs, sizeof(linjs));
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
    GLXFBConfig* nativeConfigs;
    _GLFWfbconfig* usableConfigs;
    const _GLFWfbconfig* closest;
    int nativeCount, usableCount;
    const char* vendor;
    GLFWbool trustWindowBit = GLFW_TRUE;

    // HACK: This is a (hopefully temporary) workaround for Chromium
    //       (VirtualBox GL) not setting the window bit on any GLXFBConfigs
    vendor = glXGetClientString(_glfw.x11.display, GLX_VENDOR);
    if (vendor && strcmp(vendor, "Chromium") == 0)
        trustWindowBit = GLFW_FALSE;

    nativeConfigs =
        glXGetFBConfigs(_glfw.x11.display, _glfw.x11.screen, &nativeCount);
    if (!nativeConfigs || !nativeCount)
    {
        _glfwInputError(GLFW_API_UNAVAILABLE, "GLX: No GLXFBConfigs returned");
        return GLFW_FALSE;
    }

    usableConfigs = _glfw_calloc(nativeCount, sizeof(_GLFWfbconfig));
    usableCount = 0;

    for (int i = 0;  i < nativeCount;  i++)
    {
        const GLXFBConfig n = nativeConfigs[i];
        _GLFWfbconfig* u = usableConfigs + usableCount;

        // Only consider RGBA GLXFBConfigs
        if (!(getGLXFBConfigAttrib(n, GLX_RENDER_TYPE) & GLX_RGBA_BIT))
            continue;

        // Only consider window GLXFBConfigs
        if (!(getGLXFBConfigAttrib(n, GLX_DRAWABLE_TYPE) & GLX_WINDOW_BIT))
        {
            if (trustWindowBit)
                continue;
        }

        if (getGLXFBConfigAttrib(n, GLX_DOUBLEBUFFER) != desired->doublebuffer)
            continue;

        if (desired->transparent)
        {
            XVisualInfo* vi = glXGetVisualFromFBConfig(_glfw.x11.display, n);
            if (vi)
            {
                u->transparent = _glfwIsVisualTransparentX11(vi->visual);
                XFree(vi);
            }
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

        if (getGLXFBConfigAttrib(n, GLX_STEREO))
            u->stereo = GLFW_TRUE;

        if (_glfw.glx.ARB_multisample)
            u->samples = getGLXFBConfigAttrib(n, GLX_SAMPLES);

        if (_glfw.glx.ARB_framebuffer_sRGB || _glfw.glx.EXT_framebuffer_sRGB)
            u->sRGB = getGLXFBConfigAttrib(n, GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB);

        u->handle = (uintptr_t) n;
        usableCount++;
    }

    closest = _glfwChooseFBConfig(desired, usableConfigs, usableCount);
    if (closest)
        *result = (GLXFBConfig) closest->handle;

    XFree(nativeConfigs);
    _glfw_free(usableConfigs);

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

static void destroyContextGLX(_GLFWwindow* window)
{
    if (window->context.glx.window)
    {
        glXDestroyWindow(_glfw.x11.display, window->context.glx.window);
        window->context.glx.window = None;
    }

    if (window->context.glx.handle)
    {
        glXDestroyContext(_glfw.x11.display, window->context.glx.handle);
        window->context.glx.handle = NULL;
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

GLFWbool _glfwInitGLX(void)
{
    const char* sonames[] =
    {
#if defined(_GLFW_GLX_LIBRARY)
        _GLFW_GLX_LIBRARY,
#elif defined(__CYGWIN__)
        "libGL-1.so",
#elif defined(__OpenBSD__) || defined(__NetBSD__)
        "libGL.so",
#else
        "libGLX.so.0",
        "libGL.so.1",
        "libGL.so",
#endif
        NULL
    };

    if (_glfw.glx.handle)
        return GLFW_TRUE;

    for (int i = 0;  sonames[i];  i++)
    {
        _glfw.glx.handle = _glfwPlatformLoadModule(sonames[i]);
        if (_glfw.glx.handle)
            break;
    }

    if (!_glfw.glx.handle)
    {
        _glfwInputError(GLFW_API_UNAVAILABLE, "GLX: Failed to load GLX");
        return GLFW_FALSE;
    }

    _glfw.glx.GetFBConfigs = (PFNGLXGETFBCONFIGSPROC)
        _glfwPlatformGetModuleSymbol(_glfw.glx.handle, "glXGetFBConfigs");
    _glfw.glx.GetFBConfigAttrib = (PFNGLXGETFBCONFIGATTRIBPROC)
        _glfwPlatformGetModuleSymbol(_glfw.glx.handle, "glXGetFBConfigAttrib");
    _glfw.glx.GetClientString = (PFNGLXGETCLIENTSTRINGPROC)
        _glfwPlatformGetModuleSymbol(_glfw.glx.handle, "glXGetClientString");
    _glfw.glx.QueryExtension = (PFNGLXQUERYEXTENSIONPROC)
        _glfwPlatformGetModuleSymbol(_glfw.glx.handle, "glXQueryExtension");
    _glfw.glx.QueryVersion = (PFNGLXQUERYVERSIONPROC)
        _glfwPlatformGetModuleSymbol(_glfw.glx.handle, "glXQueryVersion");
    _glfw.glx.DestroyContext = (PFNGLXDESTROYCONTEXTPROC)
        _glfwPlatformGetModuleSymbol(_glfw.glx.handle, "glXDestroyContext");
    _glfw.glx.MakeCurrent = (PFNGLXMAKECURRENTPROC)
        _glfwPlatformGetModuleSymbol(_glfw.glx.handle, "glXMakeCurrent");
    _glfw.glx.SwapBuffers = (PFNGLXSWAPBUFFERSPROC)
        _glfwPlatformGetModuleSymbol(_glfw.glx.handle, "glXSwapBuffers");
    _glfw.glx.QueryExtensionsString = (PFNGLXQUERYEXTENSIONSSTRINGPROC)
        _glfwPlatformGetModuleSymbol(_glfw.glx.handle, "glXQueryExtensionsString");
    _glfw.glx.CreateNewContext = (PFNGLXCREATENEWCONTEXTPROC)
        _glfwPlatformGetModuleSymbol(_glfw.glx.handle, "glXCreateNewContext");
    _glfw.glx.CreateWindow = (PFNGLXCREATEWINDOWPROC)
        _glfwPlatformGetModuleSymbol(_glfw.glx.handle, "glXCreateWindow");
    _glfw.glx.DestroyWindow = (PFNGLXDESTROYWINDOWPROC)
        _glfwPlatformGetModuleSymbol(_glfw.glx.handle, "glXDestroyWindow");
    _glfw.glx.GetVisualFromFBConfig = (PFNGLXGETVISUALFROMFBCONFIGPROC)
        _glfwPlatformGetModuleSymbol(_glfw.glx.handle, "glXGetVisualFromFBConfig");

    if (!_glfw.glx.GetFBConfigs ||
        !_glfw.glx.GetFBConfigAttrib ||
        !_glfw.glx.GetClientString ||
        !_glfw.glx.QueryExtension ||
        !_glfw.glx.QueryVersion ||
        !_glfw.glx.DestroyContext ||
        !_glfw.glx.MakeCurrent ||
        !_glfw.glx.SwapBuffers ||
        !_glfw.glx.QueryExtensionsString ||
        !_glfw.glx.CreateNewContext ||
        !_glfw.glx.CreateWindow ||
        !_glfw.glx.DestroyWindow ||
        !_glfw.glx.GetVisualFromFBConfig)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "GLX: Failed to load required entry points");
        return GLFW_FALSE;
    }

    // NOTE: Unlike GLX 1.3 entry points these are not required to be present
    _glfw.glx.GetProcAddress = (PFNGLXGETPROCADDRESSPROC)
        _glfwPlatformGetModuleSymbol(_glfw.glx.handle, "glXGetProcAddress");
    _glfw.glx.GetProcAddressARB = (PFNGLXGETPROCADDRESSPROC)
        _glfwPlatformGetModuleSymbol(_glfw.glx.handle, "glXGetProcAddressARB");

    if (!glXQueryExtension(_glfw.x11.display,
                           &_glfw.glx.errorBase,
                           &_glfw.glx.eventBase))
    {
        _glfwInputError(GLFW_API_UNAVAILABLE, "GLX: GLX extension not found");
        return GLFW_FALSE;
    }

    if (!glXQueryVersion(_glfw.x11.display, &_glfw.glx.major, &_glfw.glx.minor))
    {
        _glfwInputError(GLFW_API_UNAVAILABLE,
                        "GLX: Failed to query GLX version");
        return GLFW_FALSE;
    }

    if (_glfw.glx.major == 1 && _glfw.glx.minor < 3)
    {
        _glfwInputError(GLFW_API_UNAVAILABLE,
                        "GLX: GLX version 1.3 is required");
        return GLFW_FALSE;
    }

    if (extensionSupportedGLX("GLX_EXT_swap_control"))
    {
        _glfw.glx.SwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)
            getProcAddressGLX("glXSwapIntervalEXT");

        if (_glfw.glx.SwapIntervalEXT)
            _glfw.glx.EXT_swap_control = GLFW_TRUE;
    }

    if (extensionSupportedGLX("GLX_SGI_swap_control"))
    {
        _glfw.glx.SwapIntervalSGI = (PFNGLXSWAPINTERVALSGIPROC)
            getProcAddressGLX("glXSwapIntervalSGI");

        if (_glfw.glx.SwapIntervalSGI)
            _glfw.glx.SGI_swap_control = GLFW_TRUE;
    }

    if (extensionSupportedGLX("GLX_MESA_swap_control"))  {
        _glfw.glx.SwapIntervalMESA = (PFNGLXSWAPINTERVALMESAPROC)getProcAddressGLX("glXSwapIntervalMESA");
        if (_glfw.glx.SwapIntervalMESA) _glfw.glx.MESA_swap_control = GLFW_TRUE;
    }

    if (extensionSupportedGLX("GLX_ARB_multisample")) _glfw.glx.ARB_multisample = GLFW_TRUE;
    if (extensionSupportedGLX("GLX_ARB_framebuffer_sRGB")) _glfw.glx.ARB_framebuffer_sRGB = GLFW_TRUE;
    if (extensionSupportedGLX("GLX_EXT_framebuffer_sRGB")) _glfw.glx.EXT_framebuffer_sRGB = GLFW_TRUE;
    if (extensionSupportedGLX("GLX_ARB_create_context")) {
        _glfw.glx.CreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)getProcAddressGLX("glXCreateContextAttribsARB");
        if (_glfw.glx.CreateContextAttribsARB) _glfw.glx.ARB_create_context = GLFW_TRUE;
    }
    if (extensionSupportedGLX("GLX_ARB_create_context_robustness")) _glfw.glx.ARB_create_context_robustness = GLFW_TRUE;
    if (extensionSupportedGLX("GLX_ARB_create_context_profile")) _glfw.glx.ARB_create_context_profile = GLFW_TRUE;
    if (extensionSupportedGLX("GLX_EXT_create_context_es2_profile")) _glfw.glx.EXT_create_context_es2_profile = GLFW_TRUE;
    if (extensionSupportedGLX("GLX_ARB_create_context_no_error")) _glfw.glx.ARB_create_context_no_error = GLFW_TRUE;
    if (extensionSupportedGLX("GLX_ARB_context_flush_control")) _glfw.glx.ARB_context_flush_control = GLFW_TRUE;
    return GLFW_TRUE;
}

#define SET_ATTRIB(a, v) \
{ \
    attribs[index++] = a; \
    attribs[index++] = v; \
}

GLFWbool _glfwCreateContextGLX(_GLFWwindow* window, const _GLFWctxconfig* ctxconfig, const _GLFWfbconfig* fbconfig) {
    int attribs[40];
    GLXFBConfig native = NULL;
    GLXContext share = NULL;
    if (ctxconfig->share) share = ctxconfig->share->context.glx.handle;
    if (!chooseGLXFBConfig(fbconfig, &native)) { _glfwInputError(GLFW_FORMAT_UNAVAILABLE,"GLX: Failed to find a suitable GLXFBConfig"); return GLFW_FALSE; }

    if (ctxconfig->client == GLFW_OPENGL_ES_API) {
        if (!_glfw.glx.ARB_create_context || !_glfw.glx.ARB_create_context_profile || !_glfw.glx.EXT_create_context_es2_profile) { _glfwInputError(GLFW_API_UNAVAILABLE, "GLX: OpenGL ES requested but GLX_EXT_create_context_es2_profile is unavailable"); return GLFW_FALSE; }
    }

    if (ctxconfig->forward) {
        if (!_glfw.glx.ARB_create_context) { _glfwInputError(GLFW_VERSION_UNAVAILABLE,"GLX: Forward compatibility requested but GLX_ARB_create_context_profile is unavailable"); return GLFW_FALSE; }
    }

    if (ctxconfig->profile) {
        if (!_glfw.glx.ARB_create_context || !_glfw.glx.ARB_create_context_profile) { _glfwInputError(GLFW_VERSION_UNAVAILABLE,"GLX: An OpenGL profile requested but GLX_ARB_create_context_profile is unavailable"); return GLFW_FALSE; }
    }

    _glfwGrabErrorHandlerX11();
    if (_glfw.glx.ARB_create_context) {
        int index = 0, mask = 0, flags = 0;
        if (ctxconfig->client == GLFW_OPENGL_API) {
            if (ctxconfig->forward) flags |= GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
            if (ctxconfig->profile == GLFW_OPENGL_CORE_PROFILE) mask |= GLX_CONTEXT_CORE_PROFILE_BIT_ARB;
            else if (ctxconfig->profile == GLFW_OPENGL_COMPAT_PROFILE) mask |= GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
        } else mask |= GLX_CONTEXT_ES2_PROFILE_BIT_EXT;

        if (ctxconfig->debug) flags |= GLX_CONTEXT_DEBUG_BIT_ARB;
        if (ctxconfig->robustness) {
            if (_glfw.glx.ARB_create_context_robustness) {
                if (ctxconfig->robustness == GLFW_NO_RESET_NOTIFICATION) {
                    SET_ATTRIB(GLX_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB,GLX_NO_RESET_NOTIFICATION_ARB);
                } else if (ctxconfig->robustness == GLFW_LOSE_CONTEXT_ON_RESET) {
                    SET_ATTRIB(GLX_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB,GLX_LOSE_CONTEXT_ON_RESET_ARB);
                }
                flags |= GLX_CONTEXT_ROBUST_ACCESS_BIT_ARB;
            }
        }

        if (ctxconfig->release) {
            if (_glfw.glx.ARB_context_flush_control) {
                if (ctxconfig->release == GLFW_RELEASE_BEHAVIOR_NONE) {
                    SET_ATTRIB(GLX_CONTEXT_RELEASE_BEHAVIOR_ARB,GLX_CONTEXT_RELEASE_BEHAVIOR_NONE_ARB);
                } else if (ctxconfig->release == GLFW_RELEASE_BEHAVIOR_FLUSH) {
                    SET_ATTRIB(GLX_CONTEXT_RELEASE_BEHAVIOR_ARB,GLX_CONTEXT_RELEASE_BEHAVIOR_FLUSH_ARB);
                }
            }
        }

        if (ctxconfig->noerror) {
            if (_glfw.glx.ARB_create_context_no_error) {
                SET_ATTRIB(GLX_CONTEXT_OPENGL_NO_ERROR_ARB, GLFW_TRUE);
            }
        }

        if (ctxconfig->major != 1 || ctxconfig->minor != 0) {
            SET_ATTRIB(GLX_CONTEXT_MAJOR_VERSION_ARB, ctxconfig->major);
            SET_ATTRIB(GLX_CONTEXT_MINOR_VERSION_ARB, ctxconfig->minor);
        }

        if (mask) {
            SET_ATTRIB(GLX_CONTEXT_PROFILE_MASK_ARB, mask);
        }
        if (flags) {
            SET_ATTRIB(GLX_CONTEXT_FLAGS_ARB, flags);
        }
        SET_ATTRIB(None, None);
        window->context.glx.handle = _glfw.glx.CreateContextAttribsARB(_glfw.x11.display,native,share,True,attribs);
        if (!window->context.glx.handle) {
            if (_glfw.x11.errorCode == _glfw.glx.errorBase + GLXBadProfileARB && ctxconfig->client == GLFW_OPENGL_API && ctxconfig->profile == GLFW_OPENGL_ANY_PROFILE && ctxconfig->forward == GLFW_FALSE) window->context.glx.handle = createLegacyContextGLX(window, native, share);
        }
    } else {
        window->context.glx.handle = createLegacyContextGLX(window, native, share);
    }

    _glfwReleaseErrorHandlerX11();
    if (!window->context.glx.handle) { _glfwInputErrorX11(GLFW_VERSION_UNAVAILABLE, "GLX: Failed to create context"); return GLFW_FALSE; }

    window->context.glx.window = glXCreateWindow(_glfw.x11.display, native, window->x11.handle, NULL);
    if (!window->context.glx.window) { _glfwInputError(GLFW_PLATFORM_ERROR, "GLX: Failed to create window"); return GLFW_FALSE; }

    window->context.glx.fbconfig = native;
    window->context.makeCurrent = makeContextCurrentGLX;
    window->context.swapBuffers = swapBuffersGLX;
    window->context.swapInterval = swapIntervalGLX;
    window->context.extensionSupported = extensionSupportedGLX;
    window->context.getProcAddress = getProcAddressGLX;
    window->context.destroy = destroyContextGLX;
    return GLFW_TRUE;
}
#undef SET_ATTRIB

GLFWbool _glfwChooseVisualGLX(const _GLFWwndconfig* wndconfig, const _GLFWctxconfig* ctxconfig, const _GLFWfbconfig* fbconfig, Visual** visual, int* depth) {
    GLXFBConfig native; XVisualInfo* result; (void)wndconfig; (void)ctxconfig;
    if (!chooseGLXFBConfig(fbconfig, &native)) { _glfwInputError(GLFW_FORMAT_UNAVAILABLE,"GLX: Failed to find a suitable GLXFBConfig"); return GLFW_FALSE; }

    result = glXGetVisualFromFBConfig(_glfw.x11.display, native);
    if (!result) { _glfwInputError(GLFW_PLATFORM_ERROR,"GLX: Failed to retrieve Visual for GLXFBConfig"); return GLFW_FALSE; }

    *visual = result->visual;
    *depth  = result->depth;
    XFree(result);
    return GLFW_TRUE;
}

GLFWAPI GLXContext glfwGetGLXContext(GLFWwindow* handle) {
    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);
    if (_glfw.platform.platformID != GLFW_PLATFORM_X11) { _glfwInputError(GLFW_PLATFORM_UNAVAILABLE, "GLX: Platform not initialized"); return NULL; }

    _GLFWwindow* window = (_GLFWwindow*) handle;
    if (window->context.source != GLFW_NATIVE_CONTEXT_API) { _glfwInputError(GLFW_NO_WINDOW_CONTEXT, NULL); return NULL; }

    return window->context.glx.handle;
}

GLFWAPI GLXWindow glfwGetGLXWindow(GLFWwindow* handle) {
    _GLFW_REQUIRE_INIT_OR_RETURN(None);
    if (_glfw.platform.platformID != GLFW_PLATFORM_X11) { _glfwInputError(GLFW_PLATFORM_UNAVAILABLE, "GLX: Platform not initialized"); return None; }

    _GLFWwindow* window = (_GLFWwindow*) handle;
    if (window->context.source != GLFW_NATIVE_CONTEXT_API) { _glfwInputError(GLFW_NO_WINDOW_CONTEXT, NULL); return None; }
    return window->context.glx.window;
}

GLFWAPI int glfwGetGLXFBConfig(GLFWwindow* handle, GLXFBConfig* config) {
    _GLFW_REQUIRE_INIT_OR_RETURN(GLFW_FALSE);
    if (_glfw.platform.platformID != GLFW_PLATFORM_X11) { _glfwInputError(GLFW_PLATFORM_UNAVAILABLE, "GLX: Platform not initialized"); return GLFW_FALSE; }

    _GLFWwindow* window = (_GLFWwindow*) handle;
    if (window->context.source != GLFW_NATIVE_CONTEXT_API) { _glfwInputError(GLFW_NO_WINDOW_CONTEXT, NULL); return GLFW_FALSE; }

    *config = window->context.glx.fbconfig;
    return GLFW_TRUE;
}
