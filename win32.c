// GLFW 3.5 This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
#include <limits.h>
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
        else SetCursor(LoadCursorW(NULL,IDC_ARROW));
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
                uint32_t codepoint=0;
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
            _glfwInputChar(window,(uint32_t)wParam,getKeyMods(),GLFW_TRUE);
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
        case WM_GETDPISCALEDSIZE: {
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
        case WM_DPICHANGED: {
            const float xscale=HIWORD(wParam)/(float)USER_DEFAULT_SCREEN_DPI,yscale=LOWORD(wParam)/(float)USER_DEFAULT_SCREEN_DPI;
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
    int frameX,frameY,frameWidth,frameHeight;
    DWORD style=getWindowStyle(window),exStyle=getWindowExStyle(window);
    if (!_glfw.win32.mainWindowClass) {
        WNDCLASSEXW wc={sizeof(wc)};
        wc.style=CS_HREDRAW|CS_VREDRAW|CS_OWNDC; wc.lpfnWndProc=windowProc; wc.hInstance=_glfw.win32.instance;
        wc.hCursor=LoadCursorW(NULL,IDC_ARROW);
#if defined(_GLFW_WNDCLASSNAME)
        wc.lpszClassName=_GLFW_WNDCLASSNAME;
#else
        wc.lpszClassName=L"GLFW30";
#endif
        wc.hIcon=LoadImageW(GetModuleHandleW(NULL),L"GLFW_ICON",IMAGE_ICON,0,0,LR_DEFAULTSIZE|LR_SHARED);
        if (!wc.hIcon) wc.hIcon=LoadImageW(NULL,IDI_APPLICATION,IMAGE_ICON,0,0,LR_DEFAULTSIZE|LR_SHARED);
        _glfw.win32.mainWindowClass=RegisterClassExW(&wc);
        if (!_glfw.win32.mainWindowClass) { _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,"Win32: Failed to register window class"); return GLFW_FALSE; }
    }
    if (GetSystemMetrics(SM_REMOTESESSION)&&!_glfw.win32.blankCursor) {
        const int cw=GetSystemMetrics(SM_CXCURSOR),ch=GetSystemMetrics(SM_CYCURSOR);
        unsigned char* cursorPixels=_glfw_calloc(cw*ch,4);
        if (!cursorPixels) return GLFW_FALSE;
        cursorPixels[3]=1;
        const GLFWimage cursorImage={cw,ch,cursorPixels};
        _glfw.win32.blankCursor=createIcon(&cursorImage,0,0,FALSE);
        free(cursorPixels);
        if (!_glfw.win32.blankCursor) return GLFW_FALSE;
    }
    if (window->monitor) {
        MONITORINFO mi={sizeof(mi)};
        GetMonitorInfoW(window->monitor->win32.handle,&mi);
        frameX=mi.rcMonitor.left; frameY=mi.rcMonitor.top;
        frameWidth=mi.rcMonitor.right-mi.rcMonitor.left; frameHeight=mi.rcMonitor.bottom-mi.rcMonitor.top;
    } else {
        RECT rect={0,0,wndconfig->width,wndconfig->height};
        window->win32.maximized=wndconfig->maximized;
        if (wndconfig->maximized) style|=WS_MAXIMIZE;
        AdjustWindowRectEx(&rect,style,FALSE,exStyle);
        if (wndconfig->xpos==(int)GLFW_ANY_POSITION&&wndconfig->ypos==(int)GLFW_ANY_POSITION) { frameX=CW_USEDEFAULT; frameY=CW_USEDEFAULT; }
        else { frameX=wndconfig->xpos+rect.left; frameY=wndconfig->ypos+rect.top; }
        frameWidth=rect.right-rect.left; frameHeight=rect.bottom-rect.top;
    }
    WCHAR* wideTitle=_glfwCreateWideStringFromUTF8Win32(window->title);
    if (!wideTitle) return GLFW_FALSE;
    window->win32.handle=CreateWindowExW(exStyle,MAKEINTATOM(_glfw.win32.mainWindowClass),wideTitle,style,frameX,frameY,frameWidth,frameHeight,NULL,NULL,_glfw.win32.instance,(LPVOID)wndconfig);
    free(wideTitle);
    if (!window->win32.handle) { _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,"Win32: Failed to create window"); return GLFW_FALSE; }
    SetPropW(window->win32.handle,L"GLFW",window);
    ChangeWindowMessageFilterEx(window->win32.handle,WM_DROPFILES,MSGFLT_ALLOW,NULL);
    ChangeWindowMessageFilterEx(window->win32.handle,WM_COPYDATA,MSGFLT_ALLOW,NULL);
    ChangeWindowMessageFilterEx(window->win32.handle,WM_COPYGLOBALDATA,MSGFLT_ALLOW,NULL);
    window->win32.scaleToMonitor=wndconfig->scaleToMonitor;
    window->win32.keymenu=wndconfig->win32.keymenu;
    window->win32.showDefault=wndconfig->win32.showDefault;
    if (!window->monitor) {
        RECT rect={0,0,wndconfig->width,wndconfig->height};
        WINDOWPLACEMENT wp={sizeof(wp)};
        const HMONITOR mh=MonitorFromWindow(window->win32.handle,MONITOR_DEFAULTTONEAREST);
        if (wndconfig->scaleToMonitor) {
            float xscale,yscale; _glfwGetHMONITORContentScaleWin32(mh,&xscale,&yscale);
            if (xscale>0.f&&yscale>0.f) { rect.right=(int)(rect.right*xscale); rect.bottom=(int)(rect.bottom*yscale); }
        }
        if (_glfwIsWindows10Version1607OrGreaterWin32()) AdjustWindowRectExForDpi(&rect,style,FALSE,exStyle,GetDpiForWindow(window->win32.handle));
        else AdjustWindowRectEx(&rect,style,FALSE,exStyle);
        GetWindowPlacement(window->win32.handle,&wp);
        OffsetRect(&rect,wp.rcNormalPosition.left-rect.left,wp.rcNormalPosition.top-rect.top);
        wp.rcNormalPosition=rect; wp.showCmd=SW_HIDE;
        SetWindowPlacement(window->win32.handle,&wp);
        if (wndconfig->maximized&&!wndconfig->decorated) {
            MONITORINFO mi={sizeof(mi)}; GetMonitorInfoW(mh,&mi);
            SetWindowPos(window->win32.handle,HWND_TOP,mi.rcWork.left,mi.rcWork.top,mi.rcWork.right-mi.rcWork.left,mi.rcWork.bottom-mi.rcWork.top,SWP_NOACTIVATE|SWP_NOZORDER);
        }
    }
    DragAcceptFiles(window->win32.handle,TRUE);
    if (fbconfig->transparent) { updateFramebufferTransparency(window); window->win32.transparent=GLFW_TRUE; }
    _glfwGetWindowSizeWin32(window,&window->win32.width,&window->win32.height);
    return GLFW_TRUE;
}

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
    int showCommand=SW_SHOWNA;
    if (window->win32.showDefault) {
        STARTUPINFOW si={sizeof(si)}; GetStartupInfoW(&si);
        if (si.dwFlags&STARTF_USESHOWWINDOW) showCommand=si.wShowWindow;
        window->win32.showDefault=GLFW_FALSE;
    }
    ShowWindow(window->win32.handle,showCommand);
}

void _glfwHideWindowWin32(_GLFWwindow* window) { ShowWindow(window->win32.handle,SW_HIDE); }
void _glfwRequestWindowAttentionWin32(_GLFWwindow* window) { FlashWindow(window->win32.handle,TRUE); }
void _glfwFocusWindowWin32(_GLFWwindow* window) { BringWindowToTop(window->win32.handle); SetForegroundWindow(window->win32.handle); SetFocus(window->win32.handle); }

void _glfwSetWindowMonitorWin32(_GLFWwindow* window,_GLFWmonitor* monitor,int xpos,int ypos,int width,int height,int refreshRate) {
    (void)refreshRate;
    if (window->monitor==monitor) {
        if (monitor) { if (monitor->window==window) { acquireMonitor(window); fitToMonitor(window); } }
        else {
            RECT rect={xpos,ypos,xpos+width,ypos+height};
            if (_glfwIsWindows10Version1607OrGreaterWin32()) AdjustWindowRectExForDpi(&rect,getWindowStyle(window),FALSE,getWindowExStyle(window),GetDpiForWindow(window->win32.handle));
            else AdjustWindowRectEx(&rect,getWindowStyle(window),FALSE,getWindowExStyle(window));
            SetWindowPos(window->win32.handle,HWND_TOP,rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top,SWP_NOCOPYBITS|SWP_NOACTIVATE|SWP_NOZORDER);
        }
        return;
    }
    if (window->monitor) releaseMonitor(window);
    _glfwInputWindowMonitor(window,monitor);
    if (window->monitor) {
        MONITORINFO mi={sizeof(mi)}; UINT flags=SWP_SHOWWINDOW|SWP_NOACTIVATE|SWP_NOCOPYBITS;
        if (window->decorated) { DWORD style=GetWindowLongW(window->win32.handle,GWL_STYLE); style&=~WS_OVERLAPPEDWINDOW; style|=getWindowStyle(window); SetWindowLongW(window->win32.handle,GWL_STYLE,style); flags|=SWP_FRAMECHANGED; }
        acquireMonitor(window);
        GetMonitorInfoW(window->monitor->win32.handle,&mi);
        SetWindowPos(window->win32.handle,HWND_TOPMOST,mi.rcMonitor.left,mi.rcMonitor.top,mi.rcMonitor.right-mi.rcMonitor.left,mi.rcMonitor.bottom-mi.rcMonitor.top,flags);
    } else {
        HWND after; RECT rect={xpos,ypos,xpos+width,ypos+height};
        DWORD style=GetWindowLongW(window->win32.handle,GWL_STYLE); UINT flags=SWP_NOACTIVATE|SWP_NOCOPYBITS;
        if (window->decorated) { style&=~WS_POPUP; style|=getWindowStyle(window); SetWindowLongW(window->win32.handle,GWL_STYLE,style); flags|=SWP_FRAMECHANGED; }
        after=window->floating?HWND_TOPMOST:HWND_NOTOPMOST;
        if (_glfwIsWindows10Version1607OrGreaterWin32()) AdjustWindowRectExForDpi(&rect,getWindowStyle(window),FALSE,getWindowExStyle(window),GetDpiForWindow(window->win32.handle));
        else AdjustWindowRectEx(&rect,getWindowStyle(window),FALSE,getWindowExStyle(window));
        SetWindowPos(window->win32.handle,after,rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top,flags);
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
    cursor->win32.handle=LoadImageW(NULL,MAKEINTRESOURCEW(OCR_NORMAL),IMAGE_CURSOR,0,0,LR_DEFAULTSIZE|LR_SHARED);
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
    WNDCLASSEXW wc = { sizeof(wc) };
    wc.style         = CS_OWNDC;
    wc.lpfnWndProc   = (WNDPROC) helperWindowProc;
    wc.hInstance     = _glfw.win32.instance;
    wc.lpszClassName = L"GLFW3 Helper";
    _glfw.win32.helperWindowClass = RegisterClassExW(&wc);
    if (!_glfw.win32.helperWindowClass) { _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,"Win32: Failed to register helper window class"); return GLFW_FALSE; }

    _glfw.win32.helperWindowHandle = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW,MAKEINTATOM(_glfw.win32.helperWindowClass),L"GLFW message window",WS_CLIPSIBLINGS|WS_CLIPCHILDREN,0,0,1,1,NULL,NULL,_glfw.win32.instance,NULL);
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

// Replacement for IsWindowsVersionOrGreater, as we cannot rely on the
// application having a correct embedded manifest
//
BOOL _glfwIsWindowsVersionOrGreaterWin32(WORD major, WORD minor, WORD sp)
{
    OSVERSIONINFOEXW osvi = { sizeof(osvi), major, minor, 0, 0, {0}, sp };
    DWORD mask = VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR;
    ULONGLONG cond = VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL);
    cond = VerSetConditionMask(cond, VER_MINORVERSION, VER_GREATER_EQUAL);
    cond = VerSetConditionMask(cond, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);
    // HACK: Use RtlVerifyVersionInfo instead of VerifyVersionInfoW as the
    //       latter lies unless the user knew to embed a non-default manifest
    //       announcing support for Windows 10 via supportedOS GUID
    return RtlVerifyVersionInfo(&osvi, mask, cond) == 0;
}

// Checks whether we are on at least the specified build of Windows 10
//
BOOL _glfwIsWindows10BuildOrGreaterWin32(WORD build)
{
    OSVERSIONINFOEXW osvi = { sizeof(osvi), 10, 0, build };
    DWORD mask = VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER;
    ULONGLONG cond = VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL);
    cond = VerSetConditionMask(cond, VER_MINORVERSION, VER_GREATER_EQUAL);
    cond = VerSetConditionMask(cond, VER_BUILDNUMBER, VER_GREATER_EQUAL);
    // HACK: Use RtlVerifyVersionInfo instead of VerifyVersionInfoW as the
    //       latter lies unless the user knew to embed a non-default manifest
    //       announcing support for Windows 10 via supportedOS GUID
    return RtlVerifyVersionInfo(&osvi, mask, cond) == 0;
}

GLFWbool _glfwConnectWin32(int platformID, _GLFWplatform* platform) {
    (void)platformID;
    const _GLFWplatform win32 = {
        .platformID = GLFW_PLATFORM_WIN32,
        .getCursorPos = _glfwGetCursorPosWin32,
        .setCursorPos = _glfwSetCursorPosWin32,
        .setCursorMode = _glfwSetCursorModeWin32,
        .setRawMouseMotion = _glfwSetRawMouseMotionWin32,
        .rawMouseMotionSupported = _glfwRawMouseMotionSupportedWin32,
        .createCursor = _glfwCreateCursorWin32,
        .createStandardCursor = _glfwCreateStandardCursorWin32,
        .destroyCursor = _glfwDestroyCursorWin32,
        .setCursor = _glfwSetCursorWin32,
        .initJoysticks = _glfwInitJoysticksWin32,
        .pollJoystick = _glfwPollJoystickWin32,
        .getMappingName = _glfwGetMappingNameWin32,
        .updateGamepadGUID = _glfwUpdateGamepadGUIDWin32,
        .getMonitorPos = _glfwGetMonitorPosWin32,
        .getMonitorWorkarea = _glfwGetMonitorWorkareaWin32,
        .getVideoModes = _glfwGetVideoModesWin32,
        .getVideoMode = _glfwGetVideoModeWin32,
        .createWindow = _glfwCreateWindowWin32,
        .setWindowTitle = _glfwSetWindowTitleWin32,
        .setWindowIcon = _glfwSetWindowIconWin32,
        .getWindowPos = _glfwGetWindowPosWin32,
        .setWindowPos = _glfwSetWindowPosWin32,
        .getWindowSize = _glfwGetWindowSizeWin32,
        .setWindowSize = _glfwSetWindowSizeWin32,
        .getWindowFrameSize = _glfwGetWindowFrameSizeWin32,
        .iconifyWindow = _glfwIconifyWindowWin32,
        .restoreWindow = _glfwRestoreWindowWin32,
        .maximizeWindow = _glfwMaximizeWindowWin32,
        .showWindow = _glfwShowWindowWin32,
        .hideWindow = _glfwHideWindowWin32,
        .requestWindowAttention = _glfwRequestWindowAttentionWin32,
        .focusWindow = _glfwFocusWindowWin32,
        .setWindowMonitor = _glfwSetWindowMonitorWin32,
        .windowFocused = _glfwWindowFocusedWin32,
        .windowIconified = _glfwWindowIconifiedWin32,
        .windowVisible = _glfwWindowVisibleWin32,
        .windowMaximized = _glfwWindowMaximizedWin32,
        .windowHovered = _glfwWindowHoveredWin32,
        .framebufferTransparent = _glfwFramebufferTransparentWin32,
        .getWindowOpacity = _glfwGetWindowOpacityWin32,
        .setWindowResizable = _glfwSetWindowResizableWin32,
        .setWindowDecorated = _glfwSetWindowDecoratedWin32,
        .setWindowFloating = _glfwSetWindowFloatingWin32,
        .setWindowOpacity = _glfwSetWindowOpacityWin32,
        .setWindowMousePassthrough = _glfwSetWindowMousePassthroughWin32,
        .pollEvents = _glfwPollEventsWin32,
        .waitEvents = _glfwWaitEventsWin32,
        .waitEventsTimeout = _glfwWaitEventsTimeoutWin32,
        .postEmptyEvent = _glfwPostEmptyEventWin32,
    };

    *platform = win32;
    return GLFW_TRUE;
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
        case XINPUT_DEVSUBTYPE_WHEEL: return "XInput Wheel";
        case XINPUT_DEVSUBTYPE_ARCADE_STICK: return "XInput Arcade Stick";
        case XINPUT_DEVSUBTYPE_FLIGHT_STICK: return "XInput Flight Stick";
        case XINPUT_DEVSUBTYPE_DANCE_PAD: return "XInput Dance Pad";
        case XINPUT_DEVSUBTYPE_GUITAR: return "XInput Guitar";
        case XINPUT_DEVSUBTYPE_DRUM_KIT: return "XInput Drum Kit";
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

static BOOL CALLBACK deviceCallback(const DIDEVICEINSTANCE* di, void* user) {
    int jid = 0; DIDEVCAPS dc; DIPROPDWORD dipd; IDirectInputDevice8* device; _GLFWobjenumWin32 data; _GLFWjoystick* js; char guid[33],name[256];
    for (jid = 0;  jid <= GLFW_JOYSTICK_LAST;  jid++) {
        js = _glfw.joysticks + jid;
        if (js->connected) {
            if (memcmp(&js->win32.guid, &di->guidInstance, sizeof(GUID)) == 0) return DIENUM_CONTINUE;
        }
    }

    if (supportsXInput(&di->guidProduct)) return DIENUM_CONTINUE;
    if (FAILED(IDirectInput8_CreateDevice(_glfw.win32.dinput8.api,&di->guidInstance,&device,NULL))) { _glfwInputError(GLFW_PLATFORM_ERROR, "Win32: Failed to create device"); return DIENUM_CONTINUE; }
    if (FAILED(IDirectInputDevice8_SetDataFormat(device, &_glfwDataFormat))) { _glfwInputError(GLFW_PLATFORM_ERROR,"Win32: Failed to set device data format"); IDirectInputDevice8_Release(device); return DIENUM_CONTINUE; }

    ZeroMemory(&dc, sizeof(dc));
    dc.dwSize = sizeof(dc);
    if (FAILED(IDirectInputDevice8_GetCapabilities(device, &dc))) { _glfwInputError(GLFW_PLATFORM_ERROR,"Win32: Failed to query device capabilities"); IDirectInputDevice8_Release(device); return DIENUM_CONTINUE; }

    ZeroMemory(&dipd, sizeof(dipd));
    dipd.diph.dwSize = sizeof(dipd);
    dipd.diph.dwHeaderSize = sizeof(dipd.diph);
    dipd.diph.dwHow = DIPH_DEVICE;
    dipd.dwData = DIPROPAXISMODE_ABS;
    if (FAILED(IDirectInputDevice8_SetProperty(device,DIPROP_AXISMODE,&dipd.diph))) { _glfwInputError(GLFW_PLATFORM_ERROR,"Win32: Failed to set device axis mode"); IDirectInputDevice8_Release(device); return DIENUM_CONTINUE; }

    memset(&data, 0, sizeof(data));
    data.device = device;
    data.objects = _glfw_calloc(dc.dwAxes + (size_t) dc.dwButtons + dc.dwPOVs,sizeof(_GLFWjoyobjectWin32));
    if (FAILED(IDirectInputDevice8_EnumObjects(device,deviceObjectCallback,&data,DIDFT_AXIS|DIDFT_BUTTON|DIDFT_POV))) { _glfwInputError(GLFW_PLATFORM_ERROR,"Win32: Failed to enumerate device objects"); IDirectInputDevice8_Release(device); free(data.objects); return DIENUM_CONTINUE; }

    qsort(data.objects, data.objectCount,sizeof(_GLFWjoyobjectWin32),compareJoystickObjects);
    if (!WideCharToMultiByte(CP_UTF8,0,di->tszInstanceName,-1,name,sizeof(name),NULL,NULL)) { _glfwInputError(GLFW_PLATFORM_ERROR, "Win32: Failed to convert joystick name to UTF-8"); IDirectInputDevice8_Release(device); free(data.objects); return DIENUM_STOP; }

    if (memcmp(&di->guidProduct.Data4[2], "PIDVID", 6) == 0) sprintf(guid, "03000000%02x%02x0000%02x%02x000000000000",(uint8_t)di->guidProduct.Data1,(uint8_t)(di->guidProduct.Data1 >> 8),(uint8_t)(di->guidProduct.Data1 >> 16),(uint8_t)(di->guidProduct.Data1 >> 24));
    else sprintf(guid, "05000000%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x00",name[0],name[1],name[2],name[3],name[4],name[5],name[6],name[7],name[8],name[9],name[10]);

    js = _glfwAllocJoystick(name, guid,data.axisCount + data.sliderCount,data.buttonCount,data.povCount);
    if (!js) { IDirectInputDevice8_Release(device); free(data.objects); return DIENUM_STOP; }

    js->win32.device = device;
    js->win32.guid = di->guidInstance;
    js->win32.objects = data.objects;
    js->win32.objectCount = data.objectCount;
    _glfwInputJoystick(js, GLFW_CONNECTED);
    return DIENUM_CONTINUE;
}

void _glfwDetectJoystickConnectionWin32(void) {
    if (_glfw.win32.xinput.instance) {
        DWORD index;
        for (index = 0;  index < XUSER_MAX_COUNT;  index++) {
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

void _glfwDetectJoystickDisconnectionWin32(void) {
    for (int jid = 0;  jid <= GLFW_JOYSTICK_LAST;  jid++) {
        _GLFWjoystick* js = _glfw.joysticks + jid;
        if (js->connected) _glfwPollJoystickWin32(js, _GLFW_POLL_PRESENCE);
    }
}

GLFWbool _glfwInitJoysticksWin32(void) {
    if (_glfw.win32.dinput8.instance) {
        if (FAILED(DirectInput8Create(_glfw.win32.instance,DIRECTINPUT_VERSION,&IID_IDirectInput8W,(void**) &_glfw.win32.dinput8.api,NULL))) { _glfwInputError(GLFW_PLATFORM_ERROR,"Win32: Failed to create interface"); return GLFW_FALSE; }
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

const char* _glfwGetMappingNameWin32(void) { return "Windows"; }
void _glfwUpdateGamepadGUIDWin32(char* guid) {
    if (strcmp(guid + 20, "504944564944") == 0) {
        char original[33];
        strncpy(original,guid,sizeof(original) - 1);
        sprintf(guid,"03000000%.4s0000%.4s000000000000",original,original + 4);
    }
}

static BOOL CALLBACK monitorCallback(HMONITOR handle, HDC dc, RECT* rect, LPARAM data) {
    MONITORINFOEXW mi;
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

// Create monitor from an adapter and (optionally) a display
//
static _GLFWmonitor* createMonitor(DISPLAY_DEVICEW* adapter,
                                   DISPLAY_DEVICEW* display)
{
    _GLFWmonitor* monitor;
    int widthMM, heightMM;
    char* name;
    HDC dc;
    DEVMODEW dm;
    RECT rect;

    if (display)
        name = _glfwCreateUTF8FromWideStringWin32(display->DeviceString);
    else
        name = _glfwCreateUTF8FromWideStringWin32(adapter->DeviceString);
    if (!name)
        return NULL;

    ZeroMemory(&dm, sizeof(dm));
    dm.dmSize = sizeof(dm);
    EnumDisplaySettingsW(adapter->DeviceName, ENUM_CURRENT_SETTINGS, &dm);

    dc = CreateDCW(L"DISPLAY", adapter->DeviceName, NULL, NULL);

    if (IsWindows8Point1OrGreater())
    {
        widthMM  = GetDeviceCaps(dc, HORZSIZE);
        heightMM = GetDeviceCaps(dc, VERTSIZE);
    }
    else
    {
        widthMM  = (int) (dm.dmPelsWidth * 25.4f / GetDeviceCaps(dc, LOGPIXELSX));
        heightMM = (int) (dm.dmPelsHeight * 25.4f / GetDeviceCaps(dc, LOGPIXELSY));
    }

    DeleteDC(dc);

    monitor = _glfwAllocMonitor(name, widthMM, heightMM);
    free(name);
    if (adapter->StateFlags & DISPLAY_DEVICE_MODESPRUNED) monitor->win32.modesPruned = GLFW_TRUE;
    wcscpy(monitor->win32.adapterName, adapter->DeviceName);
    WideCharToMultiByte(CP_UTF8, 0,
                        adapter->DeviceName, -1,
                        monitor->win32.publicAdapterName,
                        sizeof(monitor->win32.publicAdapterName),
                        NULL, NULL);

    if (display)
    {
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
        xdpi = GetDeviceCaps(dc, LOGPIXELSX);
        ydpi = GetDeviceCaps(dc, LOGPIXELSY);
        ReleaseDC(NULL, dc);
    }

    if (xscale) *xscale = xdpi / (float) USER_DEFAULT_SCREEN_DPI;
    if (yscale) *yscale = ydpi / (float) USER_DEFAULT_SCREEN_DPI;
}

void _glfwGetMonitorPosWin32(_GLFWmonitor* monitor, int* xpos, int* ypos) {
    DEVMODEW dm; ZeroMemory(&dm,sizeof(dm));
    dm.dmSize = sizeof(dm);
    EnumDisplaySettingsExW(monitor->win32.adapterName,ENUM_CURRENT_SETTINGS,&dm,EDS_ROTATEDMODE);
    if (xpos) {*xpos = dm.dmPosition.x;} if (ypos) {*ypos = dm.dmPosition.y;}
}

void _glfwGetMonitorWorkareaWin32(_GLFWmonitor* monitor, int* xpos, int* ypos, int* width, int* height) {
    MONITORINFO mi = { sizeof(mi) }; GetMonitorInfoW(monitor->win32.handle, &mi);
    if (xpos) {*xpos = mi.rcWork.left;} if (ypos) {*ypos = mi.rcWork.top;}
    if (width) {*width = mi.rcWork.right - mi.rcWork.left;} if (height) {*height = mi.rcWork.bottom - mi.rcWork.top;}
}

GLFWvidmode* _glfwGetVideoModesWin32(_GLFWmonitor* monitor, int* count) {
    int modeIndex = 0, size = 0; GLFWvidmode* result = NULL;
    *count = 0;
    for (;;) {
        int i;
        GLFWvidmode mode;
        DEVMODEW dm;
        ZeroMemory(&dm, sizeof(dm));
        dm.dmSize = sizeof(dm);
        if (!EnumDisplaySettingsW(monitor->win32.adapterName, modeIndex, &dm)) break;

        modeIndex++;
        if (dm.dmBitsPerPel < 15) continue; // Skip modes with less than 15 BPP

        mode.width  = dm.dmPelsWidth;
        mode.height = dm.dmPelsHeight;
        mode.refreshRate = dm.dmDisplayFrequency;
        _glfwSplitBPP(dm.dmBitsPerPel,&mode.redBits,&mode.greenBits,&mode.blueBits);
        for (i = 0;  i < *count;  i++) { if (_glfwCompareVideoModes(result + i, &mode) == 0) {break;} }
        if (i < *count) continue; // Skip duplicate modes
        if (monitor->win32.modesPruned) {
            // Skip modes not supported by the connected displays
            if (ChangeDisplaySettingsExW(monitor->win32.adapterName,&dm,NULL,CDS_TEST,NULL) != DISP_CHANGE_SUCCESSFUL) continue;
        }

        if (*count == size) {
            size += 128;
            result = result ? (GLFWvidmode*)calloc(size * sizeof(GLFWvidmode) : (GLFWvidmode*)realloc(result, size * sizeof(GLFWvidmode));
        }

        (*count)++;
        result[*count - 1] = mode;
    }

    if (!*count) {
        // HACK: Report the current mode if no valid modes were found
        result = _glfw_calloc(1, sizeof(GLFWvidmode));
        _glfwGetVideoModeWin32(monitor, result);
        *count = 1;
    }

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
uint64_t _glfwPlatformGetTimerValue(void) { uint64_t value; QueryPerformanceCounter((LARGE_INTEGER*)&value); return value; }
uint64_t _glfwPlatformGetTimerFrequency(void) { return _glfw.timer.win32.frequency; }

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

GLFWAPI HGLRC glfwGetWGLContext(GLFWwindow* handle) {
    if (_glfw.platform.platformID != GLFW_PLATFORM_WIN32) { _glfwInputError(GLFW_PLATFORM_UNAVAILABLE,"WGL: Platform not initialized"); return NULL; }

    _GLFWwindow* window = (_GLFWwindow*) handle;
    if (window->context.source != GLFW_NATIVE_CONTEXT_API) { _glfwInputError(GLFW_NO_WINDOW_CONTEXT, NULL); return NULL; }
    return window->context.wgl.handle;
}
