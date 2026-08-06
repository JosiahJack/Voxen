// winput.c - Windowing and Input System interfacing with the OS.
#include "common.h"
typedef void (*WinSysglproc)(void);
typedef struct WinSyswindow WinSyswindow;
WinSyswindow* window;
typedef struct { int width,height,redBits,greenBits,blueBits,refreshRate; } vidmode;
typedef void (*WinSysproc)(void); typedef struct WinSyscontext WinSyscontext; typedef struct WinSyswindow WinSyswindow; typedef struct WinSyslibrary WinSyslibrary; typedef struct WinSysmonitor WinSysmonitor;
typedef struct { int redBits,greenBits,blueBits,alphaBits,depthBits,stencilBits; uintptr_t handle; } FBC;
extern WinSyslibrary WinSys;
WinSysproc PlatformGetModuleSymbol(void* m, const char* n); void UpdateScreenSize(i32 w, i32 h); void SaveConfig(); void InputWindowFocus(i32); void InputKey(char*,int,int); void InputMouseClick(char*,int,int); void InputCursorPos(double*,double*,double,double);
void InputMonitor(WinSysmonitor*,int,int); const FBC* ChooseFBConfig(const FBC*, u32); WinSysmonitor* AllocMonitor(const char*,int,int);
#if defined(_WIN32)
    #define LOWORD(l) ((u16)(((u64)(l))&0xffff))
    #define HIWORD(l) ((u16)((((u64)(l))>>16)&0xffff))
    #define LOBYTE(w) ((u8)(((u64)(w))&0xff))
    #define HIBYTE(w) ((u8)((((u64)(w))>>8)&0xff))
    typedef struct { i32 x,y; } POINT; typedef struct { i32 l,t,r,b; } RECT; typedef struct { u32 s,maj,min; u8 _p[264]; u16 sp; u8 _p2[6]; } OSVERSIONINFOEXW;
    int __cdecl wcscmp(const u16*,const u16*); u16* wcscpy(u16*,const u16*);
    typedef struct _devicemodeW {u16 a[34]; u16 dmSize,b; u32 c; POINT dmPosition; u8 d[18]; u16 e[33]; u32 f,dmPelsWidth,dmPelsHeight,g,dmDisplayFrequency; u8 h[32]; } DEVMODEW,*LPDEVMODEW;
    typedef i64 (__stdcall *WNDPROC)(void*,u32,u64,i64); typedef i32 (__stdcall *MONITORENUMPROC)(void*,void*,RECT*,i64); typedef struct _ICONINFO { i32 fIcon; u32 xHotspot,yHotspot; void *hbmMask,*hbmColor; } ICONINFO;
    typedef struct { u8 a[8]; u32 message; u8 b[4]; u64 wParam; i64 lParam; u32 time; u8 c[12]; } MSG; typedef struct { u32 cbSize; u8 a[16]; RECT rcWork; u8 b[4]; } MONITORINFO,*LPMONITORINFO; typedef struct { u32 cbSize; u8 a[36]; u16 szDevice[32]; } MONITORINFOEXW;
    typedef struct { u32 length; u8 a[4]; u32 showCmd; u8 b[16]; RECT rcNormalPosition; } WINDOWPLACEMENT; typedef struct { u32 cbSize,style; WNDPROC lpfnWndProc; i32 a,b; HINSTANCE hInstance; void *c,*d,*e; u16 *f,*n; void *g; } WNDCLASSEXW;
    typedef i32 (WINAPI * PFN_DwmIsCompositionEnabled)(i32*); typedef i32 (WINAPI * PFN_DwmFlush)(); typedef i32 (WINAPI * PFN_RtlVerifyVersionInfo)(OSVERSIONINFOEXW*,u32,u64); typedef i32 (WINAPI * PFN_SWE)(int);
    typedef i32 (WINAPI * PFN_GPFAIVA)(void*,int,int,u32,const int*,int*); typedef void* (WINAPI * FP_CCAA)(void*,void*,const int*); typedef void* (WINAPI * PFN_CC)(void*); typedef PROC (WINAPI * PFN_wglGetProcAddress)(const char*);
    typedef void* (WINAPI * PFN_wglGetCurrentDC)(); typedef void* (WINAPI * PFN_wglGetCurrentContext)(); typedef i32 (WINAPI * PFN_wglMakeCurrent)(void*,void*); typedef struct WGLContext { void* dc; void* handle; int interval; } WGLContext;
    PFN_wglGetCurrentDC wglGetCurrentDC; PFN_CC wglCreateContext; FP_CCAA wglCreateContextAttribsARB; PFN_wglGetCurrentContext wglGetCurrentContext; PFN_wglMakeCurrent wglMakeCurrent; PFN_wglGetProcAddress wglGetProcAddress; PFN_GPFAIVA wglGetPixelFormatAttribivARB; PFN_SWE wglSwapIntervalEXT;
    typedef struct WinSyslibraryWGL { HINSTANCE instance; } WinSyslibraryWGL; typedef struct WinSyswindowWin32 { void* handle; i32 frameAction; int width,height,lastCurX,lastCurY; } WinSyswindowWin32;
    typedef struct WinSyslibraryWin32 { HINSTANCE instance; void* helperWindowHandle; u16 helperWindowClass,mainWindowClass; short int keycodes[512],scancodes[349]; double restoreCurPosX,restoreCurPosY; WinSyswindow *disabledCursorWindow, *capturedCursorWindow; struct {HINSTANCE instance; PFN_DwmIsCompositionEnabled IsCompositionEnabled; PFN_DwmFlush Flush;} dwmapi; struct {HINSTANCE instance; PFN_RtlVerifyVersionInfo RtlVerifyVersionInfo;} ntdll;} WinSyslibraryWin32;
    typedef struct WinSysmonitorWin32 { void* handle; u16 adapterName[32],displayName[32]; } WinSysmonitorWin32; typedef struct _DISPLAY_DEVICEW { u32 cb; u16 DeviceName[32],DeviceString[128]; u32 StateFlags; u8 _p[256]; } DISPLAY_DEVICEW,*PDISPLAY_DEVICEW,*LPDISPLAY_DEVICEW;
    typedef struct { u16 nSize,nVersion; u32 dwFlags; u8 _p[32]; } PIXELFORMATDESCRIPTOR,*PPIXELFORMATDESCRIPTOR,*LPPIXELFORMATDESCRIPTOR; typedef struct { u8 rgbBlue,rgbGreen,rgbRed,rgbReserved; } RGBQUAD; typedef struct { u8 a[36]; RGBQUAD bmiColors[1]; } BITMAPINFO;
    typedef struct { u32 bV5Size; i32 bV5Width,bV5Height; u16 bV5Planes,bV5BitCount; u32 bV5Compression; u8 _p[20]; u32 bV5RedMask,bV5GreenMask,bV5BlueMask,bV5AlphaMask; u8 _p2[52]; } BITMAPV5HEADER;
    DLL_IMP void* WINAPI CreateIconIndirect(ICONINFO*); DLL_IMP void* WINAPI GetDC(void*); DLL_IMP i32 WINAPI GetModuleHandleExW(u32,const u16*,HINSTANCE*); DLL_IMP int WINAPI ReleaseDC(void*,void*); DLL_IMP i32 WINAPI SetCursorPos(int,int); DLL_IMP int WINAPI WideCharToMultiByte(u32,u32,u16*,int,char*,int,const char*,i32*); DLL_IMP void* WINAPI SetCursor(void*); DLL_IMP i32 WINAPI GetCursorPos(POINT*);
    DLL_IMP int WINAPI MultiByteToWideChar(u32,u32,const char*,int,u16*,int); DLL_IMP i32 WINAPI ClipCursor(const RECT*); DLL_IMP i32 WINAPI ClientToScreen(void*,POINT*); DLL_IMP void* WINAPI CreateDCW(const u16*,const u16*,const u16*,const DEVMODEW*); DLL_IMP void* WINAPI GetPropW(void*,u16*); DLL_IMP i32 WINAPI GetMessageTime(); DLL_IMP i32 WINAPI GetClientRect(void*,RECT*);
    DLL_IMP void* WINAPI LoadCursorW(HINSTANCE,u16*); DLL_IMP u32 WINAPI MapVirtualKeyW(u32,u32); DLL_IMP i32 WINAPI SetWindowPos(void*,void*,int,int,int,int,u32); DLL_IMP void* WINAPI SetCapture(void* hWnd); DLL_IMP i32 WINAPI ReleaseCapture(); DLL_IMP i32 WINAPI PeekMessageW(MSG*,void*,u32,u32,u32); DLL_IMP i32 WINAPI AdjustWindowRect(RECT*,u32,i32);DLL_IMP i32 WINAPI GetWindowLongW(void*,int);
    DLL_IMP i64 WINAPI DefWindowProcW(void*,u32,u64,i64); DLL_IMP void* WINAPI MonitorFromWindow(void*,u32);DLL_IMP void* WINAPI GetActiveWindow(); DLL_IMP i32 WINAPI AdjustWindowRectEx(RECT*,u32,i32,u32); DLL_IMP i64 WINAPI SendMessageW(void*,u32,u64,i64); DLL_IMP i32 WINAPI SetWindowLongW(void*,int,i32); DLL_IMP i32 WINAPI GetMonitorInfoW(void*,LPMONITORINFO); 
    DLL_IMP i32 WINAPI TranslateMessage(const MSG*); DLL_IMP i16 WINAPI GetKeyState(int); DLL_IMP i64 WINAPI DispatchMessageW(const MSG*); DLL_IMP i32 WINAPI ShowWindow(void*,int); DLL_IMP i32 WINAPI BringWindowToTop(void*); DLL_IMP i32 WINAPI SetWindowPlacement(void*,const WINDOWPLACEMENT*); DLL_IMP void* WINAPI SetFocus(void*); DLL_IMP i32 WINAPI SetForegroundWindow(void*); 
    DLL_IMP i32 WINAPI GetWindowPlacement(void*,WINDOWPLACEMENT*); DLL_IMP i32 WINAPI SetPropW(void*,u16*,void*); DLL_IMP i32 WINAPI OffsetRect(RECT*,int,int); DLL_IMP void* WINAPI CreateWindowExW(u32,u16*,u16*,u32,int,int,int,int,void*,void*,HINSTANCE,void*); DLL_IMP u64 WINAPI VerSetConditionMask(u64,u32,u8); DLL_IMP u16 WINAPI RegisterClassExW(const WNDCLASSEXW *); DLL_IMP i32 WINAPI DeleteObject(void*); 
    DLL_IMP i32 WINAPI DeleteDC(void*); DLL_IMP i32 WINAPI SwapBuffers(void*); DLL_IMP i32 WINAPI EnumDisplayMonitors(void*,const RECT*,MONITORENUMPROC,i64); DLL_IMP i32 WINAPI EnumDisplaySettingsW(u16*,u32,LPDEVMODEW); DLL_IMP i32 WINAPI EnumDisplayDevicesW(u16*,u32,PDISPLAY_DEVICEW,u32);
    DLL_IMP i32 WINAPI EnumDisplaySettingsExW(u16*,u32,LPDEVMODEW,u32); DLL_IMP i32 WINAPI SetPixelFormat(void*,i32,const PIXELFORMATDESCRIPTOR *); DLL_IMP i32 WINAPI ChoosePixelFormat(void* hdc,const PIXELFORMATDESCRIPTOR *ppfd); DLL_IMP i32 WINAPI DescribePixelFormat(void*,i32,u32,LPPIXELFORMATDESCRIPTOR); DLL_IMP void* WINAPI CreateBitmap(i32,i32,u32,u32,const void *); 
    DLL_IMP void* WINAPI CreateDIBSection(void*,const BITMAPINFO*,u32,void**,void*,u32); DLL_IMP i32 WINAPI GetDeviceCaps(void*,i32);
    u16* CreateWideStringFromUTF8Win32(const char* s); i32 IsWindowsVersionOrGreaterWin32(u16 major, u16 minor, u16 sp); void WinSysPollMonitorsWin32();
    struct WinSyslibrary { WinSysmonitor** monitors; int monitorCount; WinSyslibraryWin32 win32; WinSyslibraryWGL wgl; };
    struct WinSyscontext { void (*makeCurrent)(WinSyswindow*); void (*swapBuffers)(WinSyswindow*); void (*swapInterval)(int); WinSysglproc (*getProcAddress)(const char*); WGLContext wgl; };
    struct WinSyswindow { i32 decorated; int cursorMode; char mouseButtons[8],keys[349]; double virtualCursorPosX,virtualCursorPosY; WinSyscontext context; WinSyswindowWin32 win32; };
    struct WinSysmonitor { char name[128]; int widthMM,heightMM; vidmode currentMode; WinSysmonitorWin32 win32; };
    static u32 getWindowStyle(const WinSyswindow* w) { return 0x060A0000 | (w->decorated ? 0x00C00000 : 0x80000000); }
    void SetWindowIcon(const WinSysIcon* image) { 
        void *dc, *handle, *color, *mask; u8 *target=NULL, *source=image->pixels;
        BITMAPV5HEADER bi={0}; bi.bV5Size=sizeof(bi); bi.bV5Width=image->width; bi.bV5Height=-image->height; bi.bV5Planes=1; bi.bV5BitCount=32; bi.bV5Compression=3; bi.bV5RedMask=0x00ff0000; bi.bV5GreenMask=0x0000ff00; bi.bV5BlueMask=0x000000ff; bi.bV5AlphaMask=0xff000000;
        dc=GetDC(NULL); color=CreateDIBSection(dc,(BITMAPINFO*)&bi,0,(void**)&target,NULL,0); ReleaseDC(NULL,dc); mask=CreateBitmap(image->width,image->height,1,1,NULL);
        for (int i=0;i<image->width*image->height;i++) { target[0]=source[2]; target[1]=source[1]; target[2]=source[0]; target[3]=source[3]; target+=4; source+=4; }
        ICONINFO ii={0}; ii.fIcon=1; ii.xHotspot=0; ii.yHotspot=0; ii.hbmMask=mask; ii.hbmColor=color; handle=CreateIconIndirect(&ii); DeleteObject(color); DeleteObject(mask);
        SendMessageW(window->win32.handle,0x0080,1,(i64)handle); SendMessageW(window->win32.handle,0x0080,0,(i64)handle);
    }
    
    static void updateCursorImage(WinSyswindow* w) { if (w->cursorMode==0x00034001) SetCursor(LoadCursorW(NULL,(u16*)32512)); else SetCursor(NULL); }
    static void captureCursor(WinSyswindow* w) { RECT r; GetClientRect(w->win32.handle,&r); ClientToScreen(w->win32.handle,(POINT*)&r.l); ClientToScreen(w->win32.handle,(POINT*)&r.r); ClipCursor(&r); WinSys.win32.capturedCursorWindow=w; }
    static void releaseCursor() { ClipCursor(NULL); WinSys.win32.capturedCursorWindow=NULL; }
    static void disableCursor(WinSyswindow* w) { WinSys.win32.disabledCursorWindow = w; POINT p; GetCursorPos(&p); WinSys.win32.restoreCurPosX = p.x; WinSys.win32.restoreCurPosY = p.y; updateCursorImage(w); captureCursor(w); }
    static void SetCurV(WinSyswindow* w, double x, double y) { w->win32.lastCurX = (int)x; w->win32.lastCurY = (int)y; POINT p = {(int)x,(int)y}; ClientToScreen(w->win32.handle,&p); SetCursorPos(p.x,p.y); }
    static void enableCursor(WinSyswindow* w) { WinSys.win32.disabledCursorWindow = NULL; releaseCursor(); SetCurV(w,WinSys.win32.restoreCurPosX,WinSys.win32.restoreCurPosY); updateCursorImage(w); }
    static i64 __stdcall windowProc(void* h, u32 m, u64 w, i64 l) {
        WinSyswindow* win=GetPropW(h,L"WinSys"); if (!win) return DefWindowProcW(h,m,w,l);
        switch (m) {
            case 0x0021: if (HIWORD(l) == 0x0201 && LOWORD(l)!=1) win->win32.frameAction=1; break;
            case 0x0215: if (l==0&&win->win32.frameAction) { if (win->cursorMode==0x00034003) disableCursor(win); win->win32.frameAction=0; } break;
            case 0x0007: InputWindowFocus(1); if (win->win32.frameAction) break; if (win->cursorMode==0x00034003) disableCursor(win); return 0;
            case 0x0008: if (win->cursorMode==0x00034003) enableCursor(win); InputWindowFocus(0); return 0;
            case 0x0112: switch(w&0xfff0){case 0xF140:case 0xF170:break; case 0xF100:return 0;} break;
            case 0x0010: OS_Exit(0);
            case 0x0100: case 0x0104: case 0x0101: case 0x0105: {
                const int action=(HIWORD(l)&0x8000)?INPUT_RELEASE:INPUT_PRESS;
                int scancode=(HIWORD(l)&(0x0100|0xff)); if (!scancode) scancode=MapVirtualKeyW((u32)w,0);
                if (scancode==0x54) scancode=0x137; if (scancode==0x146) scancode=0x45; if (scancode==0x136) scancode=0x36;
                int key = WinSys.win32.keycodes[scancode];
                if (w==0x11) {
                    if (HIWORD(l)&0x0100) key=KEY_RIGHT_CONTROL;
                    else { MSG next; const u32 time=GetMessageTime(); if (PeekMessageW(&next,NULL,0,0,0)) { if (next.message == 0x0100 || next.message == 0x0104 || next.message == 0x0101 || next.message == 0x0105) { if(next.wParam == 0x12 && (HIWORD(next.lParam) & 0x0100)&&next.time==time){break;} } } key=KEY_LEFT_CONTROL; }
                } else if (w == 0xE5) break;
                if (action == INPUT_RELEASE && w == 0x10) { InputKey(win->keys,KEY_LEFT_SHIFT,action); InputKey(win->keys,KEY_RIGHT_SHIFT,action); }
                else if (w == 0x2C) { InputKey(win->keys,key,INPUT_PRESS); InputKey(win->keys,key,INPUT_RELEASE); }
                else InputKey(win->keys,key,action);
                break; }
            case 0x0201: case 0x0204: case 0x0207: case 0x020B: case 0x0202: case 0x0205: case 0x0208: case 0x020C: {
                int i,action,button = (m==0x0201 || m == 0x0202) ? MOUSE_BUTTON_LEFT : ((m == 0x0204 || m == 0x0205) ? MOUSE_BUTTON_RIGHT : ((m == 0x0207 || m == 0x0208) ? MOUSE_BUTTON_MIDDLE : (((HIWORD(w)) == 0x0001) ? MOUSE_BUTTON_4 : MOUSE_BUTTON_5)));
                action=(m == 0x0201 || m == 0x0204 || m == 0x0207 || m == 0x020B) ? INPUT_PRESS : INPUT_RELEASE;
                i=0; while(i<8 && win->mouseButtons[i]!=INPUT_PRESS) i++; if (i>7) SetCapture(h); InputMouseClick(win->mouseButtons,button,action);
                i=0; while(i<8 && win->mouseButtons[i]!=INPUT_PRESS) i++; if (i>7) ReleaseCapture(); if (m == 0x020B || m == 0x020C) return 1; return 0; }
            case 0x0200: {                
                const int x=((int)(i16)(l & 0xFFFF)), y=((int)(i16)(l >> 16));
                if (win->cursorMode==0x00034003) { const int dx=x-win->win32.lastCurX,dy=y-win->win32.lastCurY; if (WinSys.win32.disabledCursorWindow!=win) break; InputCursorPos(&win->virtualCursorPosX,&win->virtualCursorPosY,win->virtualCursorPosX+dx,win->virtualCursorPosY+dy); }
                win->win32.lastCurX=x; win->win32.lastCurY=y; return 0; }
            case 0x020A: Sys_Input.scrollDelta += (i16)HIWORD(w)/(double)120; return 0;
            case 0x0005: if (w == 1) World.paused = true; return 0;
            case 0x0003: if (WinSys.win32.capturedCursorWindow==win) captureCursor(win); return 0;
            case 0x0086: case 0x0085: if (!win->decorated) return 1; break;
            case 0x0020: if (LOWORD(l)==1) { updateCursorImage(win); return 1; } break;
            case 0x0084: { i64 hit = DefWindowProcW(h,m,w,l); if (hit >= 10 && hit <= 17) return 1; return hit; }
        }
        return DefWindowProcW(h,m,w,l);
    }
    
    void GetWindowPos(WinSyswindow* w, int* x, int* y) { POINT p={0,0}; ClientToScreen(w->win32.handle,&p); *x=p.x; *y=p.y; }
    void GetWindowSize(WinSyswindow* w, int* w_, int* h) { RECT a; GetClientRect(w->win32.handle,&a); *w_=a.r; *h=a.b; }
    void SetWindowSize(int w_, int h) { RECT r={0,0,w_,h}; AdjustWindowRectEx(&r,getWindowStyle(window),0,0); SetWindowPos(window->win32.handle,NULL,0,0,r.r-r.l,r.b-r.t,0x0216); }
    void SetWindowMonitor(int x, int y, int wd, int h) { RECT r = {x,y,x+wd,y+h}; u32 s=GetWindowLongW(window->win32.handle,-16),f=0x0110; if (window->decorated) {s&=~0x80000000,s|=getWindowStyle(window),SetWindowLongW(window->win32.handle,-16,s),f|=0x0020;} AdjustWindowRectEx(&r,getWindowStyle(window),0,0); SetWindowPos(window->win32.handle,(void*)-2,r.l,r.t,r.r-r.l,r.b-r.t,f); }
    void SetWindowDecorated(WinSyswindow* w,i32 e) { (void)e; RECT r; u32 s=GetWindowLongW(w->win32.handle,-16); s &= ~(0x00C00000 | 0x00080000 | 0x00040000 | 0x00020000 | 0x00010000 | 0x80000000); s |= getWindowStyle(w); GetClientRect(w->win32.handle,&r); AdjustWindowRectEx(&r,s,0,0); ClientToScreen(w->win32.handle,(POINT*)&r.l); ClientToScreen(w->win32.handle,(POINT*)&r.r); SetWindowLongW(w->win32.handle,-16,s); SetWindowPos(w->win32.handle,NULL,r.l,r.t,r.r-r.l,r.b-r.t,0x0034); }
    void PollEvents() {
        void* h = GetActiveWindow(); WinSyswindow* w = GetPropW(h,L"WinSys"); MSG m;
        while (PeekMessageW(&m,NULL,0,0,0x0001)) { if (m.message==0x0012) OS_Exit(0); else { TranslateMessage(&m); DispatchMessageW(&m); } }
        const int k[4][2]={{0xA0,KEY_LEFT_SHIFT},{0xA1,KEY_RIGHT_SHIFT},{0x5B,KEY_LEFT_SUPER},{0x5C,KEY_RIGHT_SUPER}};
        for (int i=0;i<4;i++) { if ((GetKeyState(k[i][0])&0x8000)||w->keys[k[i][1]]!=INPUT_PRESS) continue; InputKey(w->keys,k[i][1],INPUT_RELEASE); }
        w = WinSys.win32.disabledCursorWindow;
        if (w) { int W,H; GetWindowSize(w,&W,&H); if (w->win32.lastCurX != W/2 || w->win32.lastCurY != H/2) SetCurV(w,W/2,H/2); }
    }
    
    WinSysproc PlatformGetModuleSymbol(void* m, const char* n) { return (WinSysproc)GetProcAddress((HMODULE)m,n); }
    typedef struct {u16 i; i32 v;} WinKeyRemap;
    static const WinKeyRemap wkm[] = {{0x00B,KEY_0},{0x002,KEY_1},{0x003,KEY_2},{0x004,KEY_3},{0x005,KEY_4},{0x006,KEY_5},{0x007,KEY_6},{0x008,KEY_7},{0x009,KEY_8},{0x00A,KEY_9},{0x01E,KEY_A},{0x030,KEY_B},{0x02E,KEY_C},{0x020,KEY_D},{0x012,KEY_E},{0x021,KEY_F},{0x022,KEY_G},{0x023,KEY_H},{0x017,KEY_I},{0x024,KEY_J},{0x025,KEY_K},{0x026,KEY_L},{0x032,KEY_M},{0x031,KEY_N},{0x018,KEY_O},{0x019,KEY_P},{0x010,KEY_Q},{0x013,KEY_R},{0x01F,KEY_S},{0x014,KEY_T},{0x016,KEY_U},{0x02F,KEY_V},{0x011,KEY_W},{0x02D,KEY_X},{0x015,KEY_Y},{0x02C,KEY_Z},{0x028,KEY_APOSTROPHE},{0x02B,KEY_BACKSLASH},{0x033,KEY_COMMA},{0x00D,KEY_EQUAL},{0x029,KEY_GRAVE_ACCENT},{0x01A,KEY_LEFT_BRACKET},{0x00C,KEY_MINUS},{0x034,KEY_PERIOD},{0x01B,KEY_RIGHT_BRACKET},{0x027,KEY_SEMICOLON},{0x035,KEY_SLASH},{0x00E,KEY_BACKSPACE},{0x153,KEY_DELETE},{0x14F,KEY_END},{0x01C,KEY_ENTER},{0x001,KEY_ESCAPE},{0x147,KEY_HOME},{0x152,KEY_INSERT},{0x15D,KEY_MENU},{0x151,KEY_PAGE_DOWN},{0x149,KEY_PAGE_UP},{0x045,KEY_PAUSE},{0x039,KEY_SPACE},{0x00F,KEY_TAB},{0x03A,KEY_CAPS_LOCK},{0x145,KEY_NUM_LOCK},{0x046,KEY_SCROLL_LOCK},{0x03B,KEY_F1},{0x03C,KEY_F2},{0x03D,KEY_F3},{0x03E,KEY_F4},{0x03F,KEY_F5},{0x040,KEY_F6},{0x041,KEY_F7},{0x042,KEY_F8},{0x043,KEY_F9},{0x044,KEY_F10},{0x057,KEY_F11},{0x058,KEY_F12},{0x038,KEY_LEFT_ALT},{0x01D,KEY_LEFT_CONTROL},{0x02A,KEY_LEFT_SHIFT},{0x15B,KEY_LEFT_SUPER},{0x137,KEY_PRINT_SCREEN},{0x138,KEY_RIGHT_ALT},{0x11D,KEY_RIGHT_CONTROL},{0x036,KEY_RIGHT_SHIFT},{0x15C,KEY_RIGHT_SUPER},{0x150,KEY_DOWN},{0x14B,KEY_LEFT},{0x14D,KEY_RIGHT},{0x148,KEY_UP},{0x052,KEY_KP_0},{0x04F,KEY_KP_1},{0x050,KEY_KP_2},{0x051,KEY_KP_3},{0x04B,KEY_KP_4},{0x04C,KEY_KP_5},{0x04D,KEY_KP_6},{0x047,KEY_KP_7},{0x048,KEY_KP_8},{0x049,KEY_KP_9},{0x04E,KEY_KP_ADD},{0x053,KEY_KP_DECIMAL},{0x135,KEY_KP_DIVIDE},{0x11C,KEY_KP_ENTER},{0x059,KEY_KP_EQUAL},{0x037,KEY_KP_MULTIPLY},{0x04A,KEY_KP_SUBTRACT}};
    static void createKeyTables() {
        mset(WinSys.win32.keycodes,-1,sizeof(WinSys.win32.keycodes)); mset(WinSys.win32.scancodes,-1,sizeof(WinSys.win32.scancodes));
        for (size_t i=0;i<sizeof(wkm)/sizeof(wkm[0]);i++) WinSys.win32.keycodes[wkm[i].i] = wkm[i].v;
        for (int s=0;s<512;s++) if (WinSys.win32.keycodes[s] > 0) WinSys.win32.scancodes[WinSys.win32.keycodes[s]] = s;
    }
    
    u16* CreateWideStringFromUTF8Win32(const char* s) { u16* t; int c = MultiByteToWideChar(65001,0,(char*)s,-1,NULL,0); t = OS_Calloc(c,sizeof(u16)); MultiByteToWideChar(65001,0,(char*)s,-1,t,c); return t; }
    char* CreateUTF8FromWideStringWin32(const u16* s, int* sz) { *sz = WideCharToMultiByte(65001,0,(u16*)s,-1,NULL,0,NULL,NULL); char* t = OS_Calloc(*sz,1); WideCharToMultiByte(65001,0,(u16*)s,-1,t,*sz,NULL,NULL); return t; }
    i32 IsWindowsVersionOrGreaterWin32(u16 major, u16 minor, u16 sp) { OSVERSIONINFOEXW o={0}; o.s=sizeof(o), o.maj=major, o.min=minor, o.sp=sp; u64 c=VerSetConditionMask(VerSetConditionMask(VerSetConditionMask(0,0x0000002,3),0x0000001,3),0x0000020,3); return WinSys.win32.ntdll.RtlVerifyVersionInfo(&o,0x0000023,c)==0; }
    static i32 __stdcall monitorCallback(void* h, void* c, RECT* r, i64 d) { (void)c; (void)r; MONITORINFOEXW mi; mset(&mi,0,sizeof(mi)); mi.cbSize = sizeof(mi); if (GetMonitorInfoW(h,(MONITORINFO*)&mi)) { WinSysmonitor* m = (WinSysmonitor*)d; if (wcscmp(mi.szDevice, m->win32.adapterName) == 0) m->win32.handle = h; } return 1; }
    static WinSysmonitor* createMonitor(DISPLAY_DEVICEW* a, DISPLAY_DEVICEW* d) {
        WinSysmonitor* m; int wMM,hMM,nameSize=0; void* dc; RECT r;
        char* name = CreateUTF8FromWideStringWin32(d ? d->DeviceString : a->DeviceString,&nameSize);
        DEVMODEW dm; mset(&dm,0,sizeof(dm)); dm.dmSize = sizeof(dm);
        EnumDisplaySettingsW(a->DeviceName,0xFFFFFFFFU,&dm);
        dc = CreateDCW(L"DISPLAY",a->DeviceName,NULL,NULL);
        if (IsWindowsVersionOrGreaterWin32(HIBYTE(0x0603),LOBYTE(0x0603),0)) { wMM = GetDeviceCaps(dc,4); hMM = GetDeviceCaps(dc,6); } 
        else { wMM = (int)(dm.dmPelsWidth * 25.4f / GetDeviceCaps(dc,88)); hMM = (int)(dm.dmPelsHeight * 25.4f / GetDeviceCaps(dc,90)); }
        DeleteDC(dc); m = AllocMonitor(name,wMM,hMM); OS_Free(name,nameSize);
        wcscpy(m->win32.adapterName, a->DeviceName);
        if (d) wcscpy(m->win32.displayName,d->DeviceName);
        r.l=dm.dmPosition.x; r.t=dm.dmPosition.y; r.r=dm.dmPosition.x + dm.dmPelsWidth; r.b=dm.dmPosition.y + dm.dmPelsHeight;
        EnumDisplayMonitors(NULL,&r,monitorCallback,(i64)m);
        return m;
    }
    
    void WinSysPollMonitorsWin32() {
        int i, dC = WinSys.monitorCount; WinSysmonitor** d = NULL; u32 aI,dI; DISPLAY_DEVICEW a, dp; WinSysmonitor* m;
        if (dC) { d = OS_Calloc(WinSys.monitorCount,sizeof(WinSysmonitor*)); mcpy(d,WinSys.monitors,WinSys.monitorCount * sizeof(WinSysmonitor*)); }
        for (aI = 0;;aI++) {
            mset(&a,0,sizeof(a)); a.cb = sizeof(a); if (!EnumDisplayDevicesW(NULL,aI,&a,0)) break;
            if (!(a.StateFlags&1)) continue;
            for (dI=0;;++dI) {
                mset(&dp,0,sizeof(dp)); dp.cb = sizeof(dp); if (!EnumDisplayDevicesW(a.DeviceName,dI,&dp,0)) break;
                if (!(dp.StateFlags&1)) continue;
                int dT = (dp.StateFlags & 0x00000004) ? 1 : 0;
                for (i=0;i<dC;++i) { if(d[i] && wcscmp(d[i]->win32.displayName,dp.DeviceName) == 0){d[i] = NULL; EnumDisplayMonitors(NULL,NULL,monitorCallback,(i64)WinSys.monitors[i]); break;} }
                if (i < dC) continue;
                m = createMonitor(&a,&dp); if (!m) { OS_Free(d,WinSys.monitorCount*sizeof(WinSysmonitor*)); return; }
                InputMonitor(m,0x00040001,dT);
            }
            if (dI == 0) {
                int aT = (a.StateFlags & 0x00000004) ? 0 : 1;
                for (i=0;i<dC;++i) { if (d[i] && wcscmp(d[i]->win32.adapterName,a.DeviceName) == 0) {d[i]=NULL; break;} }
                if (i < dC) continue;
                m = createMonitor(&a,NULL); if (!m) { OS_Free(d,WinSys.monitorCount*sizeof(WinSysmonitor*)); return; }
                InputMonitor(m,0x00040001,aT);
            }
        }
        for (i=0;i<dC;++i) { if (d[i]) InputMonitor(d[i],0x00040002,0); }
        if (d) OS_Free(d,WinSys.monitorCount*sizeof(WinSysmonitor*));
    }
    
    static i64 __stdcall helperWindowProc(void* h, u32 m, u64 w, i64 l) { if (m == 0x007E) WinSysPollMonitorsWin32(); return DefWindowProcW(h,m,w,l); }
    void GetMonitorPos(WinSysmonitor* m, int* x, int* y) { DEVMODEW dm; mset(&dm,0,sizeof(dm)); dm.dmSize = sizeof(dm); EnumDisplaySettingsExW(m->win32.adapterName,0xFFFFFFFFU,&dm,0x00000004); *x = dm.dmPosition.x; *y = dm.dmPosition.y; }
    void GetMonitorWorkarea(WinSysmonitor* m, int* x, int* y, int* w, int* h) { MONITORINFO mi = {0}; mi.cbSize = sizeof(mi); GetMonitorInfoW(m->win32.handle,&mi); *x = mi.rcWork.l; *y = mi.rcWork.t; *w = mi.rcWork.r - mi.rcWork.l; *h = mi.rcWork.b - mi.rcWork.t; }
    void GetVideoMode(WinSysmonitor* m, vidmode* mode) { DEVMODEW dm; mset(&dm,0,sizeof(dm)); dm.dmSize = sizeof(dm); EnumDisplaySettingsW(m->win32.adapterName,0xFFFFFFFFU,&dm); mode->width=dm.dmPelsWidth; mode->height=dm.dmPelsHeight; mode->refreshRate=dm.dmDisplayFrequency; }
    static void makeContextCurrentWGL(WinSyswindow* w) { wglMakeCurrent(w->context.wgl.dc,w->context.wgl.handle); }
    static void swapBuffersWGL(WinSyswindow* w) { if (!IsWindowsVersionOrGreaterWin32(HIBYTE(0x0602),LOBYTE(0x0602),0)) { i32 e = 0; if ((i32)(WinSys.win32.dwmapi.IsCompositionEnabled(&e) >= 0) && e) { int c = vabs(w->context.wgl.interval); while (c--) WinSys.win32.dwmapi.Flush(); } } SwapBuffers(w->context.wgl.dc); }
    static void swapIntervalWGL(int i) { window->context.wgl.interval = i; if (!IsWindowsVersionOrGreaterWin32(HIBYTE(0x0602),LOBYTE(0x0602),0)) { i32 e = 0; if ((i32)(WinSys.win32.dwmapi.IsCompositionEnabled(&e) >= 0) && e) i = 0; } wglSwapIntervalEXT(i); }
    static WinSysglproc getProcAddressWGL(const char* p) { const WinSysglproc proc = (WinSysglproc)wglGetProcAddress(p); if (proc) return proc; return (WinSysglproc)PlatformGetModuleSymbol(WinSys.wgl.instance,p); }
    void SetWindowPosition(WinSyswindow* w, int x, int y) { RECT r = {x,y,x,y}; AdjustWindowRectEx(&r,getWindowStyle(w),0,0x00040000); SetWindowPos(w->win32.handle,NULL,r.l,r.t,0,0,0x0215); }
    WinSyswindow* VCreateWindow(int width, int height) {
        WinSyswindow* w = OS_Calloc(1,sizeof(WinSyswindow)); w->decorated = 1; w->cursorMode = 0x00034003; u32 style = getWindowStyle(w);
        WNDCLASSEXW wc= (WNDCLASSEXW){sizeof(wc),0x23,windowProc,0,0,WinSys.win32.instance,NULL,NULL,NULL,NULL,L"Voxen",NULL};
        WinSys.win32.mainWindowClass=RegisterClassExW(&wc);
        RECT r={0,0,width,height}; int fW=r.r-r.l, fH=r.b-r.t;
        u16* wt=CreateWideStringFromUTF8Win32(GAME_TITLE);
        w->win32.handle=CreateWindowExW(0,(u16*)(uintptr_t)WinSys.win32.mainWindowClass,wt,style,0x80000000,0x80000000,fW,fH,NULL,NULL,WinSys.win32.instance,NULL);
        SetPropW(w->win32.handle,L"WinSys",w); WINDOWPLACEMENT wp={0}; wp.length=sizeof(wp); AdjustWindowRectEx(&r,style,0,0); GetWindowPlacement(w->win32.handle,&wp);
        OffsetRect(&r,wp.rcNormalPosition.l-r.l,wp.rcNormalPosition.t-r.t); wp.rcNormalPosition=r; wp.showCmd=0; SetWindowPlacement(w->win32.handle,&wp); GetWindowSize(w,&w->win32.width,&w->win32.height);
        PIXELFORMATDESCRIPTOR pfd; void *prc,*rc,*pdc,*dc; WinSys.wgl.instance=LoadLibraryA("opengl32.dll");            
        wglCreateContext=(PFN_CC)PlatformGetModuleSymbol(WinSys.wgl.instance,"wglCreateContext");
        wglGetProcAddress=(PFN_wglGetProcAddress)PlatformGetModuleSymbol(WinSys.wgl.instance,"wglGetProcAddress");         
        wglGetCurrentDC=(PFN_wglGetCurrentDC)PlatformGetModuleSymbol(WinSys.wgl.instance,"wglGetCurrentDC");
        wglGetCurrentContext=(PFN_wglGetCurrentContext)PlatformGetModuleSymbol(WinSys.wgl.instance,"wglGetCurrentContext"); 
        wglMakeCurrent=(PFN_wglMakeCurrent)PlatformGetModuleSymbol(WinSys.wgl.instance,"wglMakeCurrent");
        dc=GetDC(WinSys.win32.helperWindowHandle); mset(&pfd,0,sizeof(pfd)); pfd.nSize=sizeof(pfd); pfd.dwFlags=0x25; SetPixelFormat(dc,ChoosePixelFormat(dc,&pfd),&pfd); rc=wglCreateContext(dc); pdc=wglGetCurrentDC(); prc=wglGetCurrentContext();
        wglMakeCurrent(dc,rc);
        wglCreateContextAttribsARB=(FP_CCAA)wglGetProcAddress("wglCreateContextAttribsARB"); wglSwapIntervalEXT=(PFN_SWE)wglGetProcAddress("wglSwapIntervalEXT"); wglGetPixelFormatAttribivARB=(PFN_GPFAIVA)wglGetProcAddress("wglGetPixelFormatAttribivARB");
        wglMakeCurrent(pdc,prc); int attribs[40],pixelFormat; PIXELFORMATDESCRIPTOR pfd2; w->context.wgl.dc = GetDC(w->win32.handle);
        static const int attribs2[]={0x2010,0x2001,0x2013,0x2003,0x2011,0x2015,0x2017,0x2019,0x201b,0x2022,0x2023};
        int values[11],i,nativeCount,usableCount=0; const int query = 0x2000; wglGetPixelFormatAttribivARB(w->context.wgl.dc,1,0,1,&query,&nativeCount);
        FBC* usableConfigs = OS_Calloc(nativeCount,sizeof(FBC));
        for (i = 0; i < nativeCount; i++) {
            FBC* u = usableConfigs + usableCount; pixelFormat = i + 1; wglGetPixelFormatAttribivARB(w->context.wgl.dc,pixelFormat,0,11,attribs2,values); if (values[0] == 0 || values[1] == 0 || values[2] != 0x202b || values[3] == 0x2025 || values[4] !=  1) continue;
            u->redBits=values[5]; u->greenBits=values[6]; u->blueBits=values[7]; u->alphaBits=values[8]; u->depthBits=values[9]; u->stencilBits=values[10]; u->handle=pixelFormat; usableCount++;
        }
        const FBC* closest = ChooseFBConfig(usableConfigs,usableCount);
        pixelFormat = (int)closest->handle; OS_Free(usableConfigs,nativeCount * sizeof(FBC));
        DescribePixelFormat(w->context.wgl.dc,pixelFormat,sizeof(pfd2),&pfd2); SetPixelFormat(w->context.wgl.dc,pixelFormat,&pfd2);
        int index=0; attribs[index++] = 0x2091; attribs[index++] = 4; attribs[index++] = 0x2092; attribs[index++] = 3; attribs[index++] = 0x9126; attribs[index++] = 1; attribs[index++] = 0; attribs[index++] = 0;
        w->context.wgl.handle=wglCreateContextAttribsARB(w->context.wgl.dc,NULL,attribs); w->context.makeCurrent=makeContextCurrentWGL; w->context.swapBuffers=swapBuffersWGL; w->context.swapInterval=swapIntervalWGL; w->context.getProcAddress=getProcAddressWGL;
        return w;
    }
#else // LINUX
    typedef u8 KeyCode; typedef u16 Rotation,SubpixelOrder,Connection; typedef i32 Bool; typedef int Status; typedef u64 XID,Mask,Atom,VisualID,Time,KeySym; typedef char *XPointer; typedef u32 XcursorUInt;
    typedef struct _XcursorImage { u8 a[16]; XcursorUInt xhot,yhot; u8 b[8]; XcursorUInt *pixels; } XcursorImage; typedef struct { i64 flags; int x,y; u8 a[8]; int min_width,min_height,max_width,max_height; u8 b[32]; int win_gravity; } XSizeHints;
    typedef struct { u8 a[56]; } Visual; typedef struct { u8 a[16]; XID root; u8 b[40]; Visual *root_visual; u8 c[56]; } Screen;
    typedef struct { u8 a[72]; i64 event_mask; u8 b[16]; XID colormap; u8 c[8]; } XSetWindowAttributes; typedef struct { u8 a[8]; int width,height; u8 b[84]; int map_state; u8 c[40]; } XWindowAttributes; typedef struct _XDisplay Display;
    typedef struct { u8 a[136]; int qlen; u8 b[84]; i32 default_screen, nscreens; Screen *screens; u8 c[56]; } *_XPrivDisplay; typedef struct { int a; u64 b; int c; void *d; u64 e,f,g,h; int i,j,k,l; u32 m,keycode; int n; } XKeyEvent;
    typedef struct { int a; u64 b; int c; Display *d; XID e,f,g; Time h; int i,j,k,l; u32 m,button; int n; } XButtonEvent; typedef struct { int a; u64 b; int c; Display *d; XID e,f,g; Time h; int x,y,i,j; u32 k; char l; int m; } XMotionEvent;
    typedef struct { int a; u64 b; int c; Display *d; XID e,f,g; Time h; int x,y; u8 i[32]; } XCrossingEvent; typedef struct { int a; u64 b; i32 c; Display *d; XID e; int mode,f; } XFocusChangeEvent;
    typedef struct { int a; u64 b; i32 c; Display *d; XID e,f,parent; int g,h,i; } XReparentEvent; typedef struct { int a; u64 b; i32 c; Display *d; XID e,f; int x,y,width,height,g; XID h; int i; } XConfigureEvent;
    typedef struct { int a; u64 b; int c; Display *d; XID window; Atom message_type; int format; union { char b[20]; short s[10]; long l[5]; } data; } XClientMessageEvent; typedef struct { int a; u64 b; int send_event; Display *c; XID window; } XAnyEvent;
    typedef struct { int type; u64 serial; int send_event; Display *display; int extension, evtype; u32 cookie; void *data; } XGenericEventCookie;
    typedef union _XEvent { int type; XAnyEvent xany; XKeyEvent xkey; XButtonEvent xbutton; XMotionEvent xmotion; XCrossingEvent xcrossing; XFocusChangeEvent xfocus; u8 p0[528]; XReparentEvent xreparent; XConfigureEvent xcfg; u8 p1[648]; XClientMessageEvent xclient; u8 p2[224]; } XEvent;
    typedef struct _XIC *XIC; typedef struct { Visual *visual; u8 a[12]; int depth; u8 b[40]; } XVisualInfo; typedef int XContext; typedef XID RROutput,RRCrtc,RRMode; typedef u64 XRRModeFlags;
    typedef struct { RRMode id; u32 width,height; u64 dotClock; u8 b[8]; u32 hTotal; u8 c[12]; u32 vTotal; u8 d[28]; } XRRModeInfo; typedef struct { u8 a[32]; int noutput; u8 b[4]; RROutput *outputs; int nmode; u8 c[4]; XRRModeInfo *modes; } XRRScreenResources;
    typedef struct { u8 a[8]; RRCrtc crtc; char *name; u8 b[8]; u64 mm_width,mm_height; Connection connection; u8 c[52]; } XRROutputInfo; typedef struct { u8 a[8]; i32 x,y; u32 width,height; RRMode mode; Rotation rotation; u8 b[36]; } XRRCrtcInfo;
    typedef XID GLXWindow,GLXDrawable; typedef struct __GLXFBConfig* GLXFBConfig; typedef struct __GLXcontext* GLXContext;
    typedef void(*__GLXextproc)();                            typedef XSizeHints*(*PFN_XAllocSizeHints)();                       typedef int(*PFN_XChangeProperty)(Display*,XID,Atom,Atom,int,int,const u8*,int);   typedef void(*PFN_XCID)(XcursorImage*);                         typedef void(*PFN_XRRFreeOutputInfo)(XRROutputInfo*);                    typedef XID(*PFN_XCreateColormap)(Display*,XID,Visual*,int);
    typedef int(*PFN_XDefineCursor)(Display*,XID,XID);        typedef int(*PFN_XDeleteProperty)(Display*,XID,Atom);              typedef int(*PFN_XDisplayKeycodes)(Display*,int*,int*);                            typedef Bool(*PFN_XFilterEvent)(XEvent*,XID);                   typedef int(*PFN_XFindContext)(Display*,XID,XContext,XPointer*);         typedef XID(*PFN_XCreateWindow)(Display*,XID,int,int,u32,u32,u32,int,u32,Visual*,u64,XSetWindowAttributes*);
    typedef int(*PFN_XFree)(void*);                           typedef void(*PFN_XFreeEventData)(Display*,XGenericEventCookie*);  typedef int(*PFN_XGrabPointer)(Display*,XID,Bool,u32,int,int,XID,XID,Time);        typedef void(*PFN_XSetICFocus)(XIC);                            typedef KeySym*(*PFN_XGetKeyboardMapping)(Display*,KeyCode,int,int*);    typedef Status(*PFN_XGetWMNormalHints)(Display*,XID,XSizeHints*,long*);
    typedef Atom(*PFN_XInternAtom)(Display*,const char*,Bool);typedef int(*PFN_XGetInputFocus)(Display*,XID*,int*);              typedef int(*PFN_XMapWindow)(Display*,XID);                                        typedef int(*PFN_XMoveWindow)(Display*,XID,int,int);            typedef int(*PFN_XMoveResizeWindow)(Display*,XID,int,int,u32,u32);       typedef int(*PFN_XGetWindowProperty)(Display*,XID,Atom,long,long,Bool,Atom,Atom*,int*,u64*,u64*,u8**);
    typedef Status(*PFN_XInitThreads)();                      typedef int(*PFN_XNextEvent)(Display*,XEvent*);                    typedef XRRCrtcInfo*(*PFN_XRRGetCrtcInfo)(Display*,XRRScreenResources*,RRCrtc);    typedef int(*PFN_XPending)(Display*);                           typedef Bool(*PFN_XQueryExtension)(Display*,const char*,int*,int*,int*); typedef Bool(*PFN_XQueryPointer)(Display*,XID,XID*,XID*,int*,int*,int*,int*,u32*);
    typedef int(*PFN_XRaiseWindow)(Display*,XID);             typedef int(*PFN_XSaveContext)(Display*,XID,XContext,const char*); typedef int(*PFN_XResizeWindow)(Display*,XID,u32,u32);                             typedef XcursorImage*(*PFN_XCIC)(int,int);                      typedef Status(*PFN_XSendEvent)(Display*,XID,Bool,long,XEvent*);         typedef int(*PFN_XSetInputFocus)(Display*,XID,int,Time);
    typedef void(*PFN_XUnsetICFocus)(XIC);                    typedef Status(*PFN_XSetWMProtocols)(Display*,XID,Atom*,int);      typedef Bool(*PFN_XTranslateCoordinates)(Display*,XID,XID,int,int,int*,int*,XID*); typedef int(*PFN_XUndefineCursor)(Display*,XID);                typedef void(*PFN_XSetWMNormalHints)(Display*,XID,XSizeHints*);          typedef int(*PFN_XWarpPointer)(Display*,XID,XID,int,int,u32,u32,int,int);
    typedef void(*PFN_XRRFreeCrtcInfo)(XRRCrtcInfo*);         typedef int(*PFN_XUngrabPointer)(Display*,Time);                   typedef int(*PFN_XChangeWindowAttributes)(Display*,XID,u64,XSetWindowAttributes*); typedef void(*PFN_XRRFreeScreenResources)(XRRScreenResources*); typedef Display*(*PFN_XOpenDisplay)(const char*);                        typedef XRROutputInfo*(*PFN_XRRGetOutputInfo)(Display*,XRRScreenResources*,RROutput);
    typedef RROutput(*PFN_XRRGetOutputPrimary)(Display*,XID); typedef void(*PFN_XRRSelectInput)(Display*,XID,int);               typedef XRRScreenResources*(*PFN_XRRGetScreenResourcesCurrent)(Display*,XID);      typedef int(*PFN_XRRUpdateConfiguration)(XEvent*);              typedef Bool(*PFN_XCheckTypedWindowEvent)(Display*,XID,int,XEvent*);     typedef Status(*PFN_XGetWindowAttributes)(Display*,XID,XWindowAttributes*);
    typedef Bool(*GLX_QEP)(Display*,int*,int*);               typedef int(*GLX_GFBCAP)(Display*,GLXFBConfig,int,int*);           typedef GLXContext(*GLX_CNCP)(Display*,GLXFBConfig,int,GLXContext,Bool);           typedef Bool(*GLX_QVP)(Display*,int*,int*);                     typedef Bool(*GLX_MCP)(Display*,GLXDrawable,GLXContext);                 typedef void(*GLX_SBP)(Display*,GLXDrawable);
    typedef XID(*PFN_XCILC)(Display*,const XcursorImage*);    typedef const char*(*GLX_QESP)(Display*,int);                      typedef GLXFBConfig*(*GLX_GFBCP)(Display*,int,int*);                               typedef __GLXextproc(*GLX_GPAP)(const u8*);                     typedef void(*GLX_SIEP)(Display*,GLXDrawable,int);                       typedef XVisualInfo*(*GLX_GVFFBCP)(Display*,GLXFBConfig);
    typedef GLXWindow(*GLX_CWP)(Display*,GLXFBConfig,XID,const int*); typedef GLXContext(*GLX_CCAA)(Display*,GLXFBConfig,GLXContext,Bool,const int*); typedef struct WinSyscontextGLX { GLXContext handle; GLXWindow window; GLXFBConfig fbconfig; } WinSyscontextGLX;
    typedef struct WinSyslibraryGLX { int major,minor,eventBase,errorBase; void* handle; GLX_GFBCP GetFBConfigs; GLX_GFBCAP GetFBConfigAttrib; GLX_QEP QueryExtension; GLX_QVP QueryVersion; GLX_MCP MakeCurrent; GLX_SBP SwapBuffers; GLX_QESP QueryExtensionsString; GLX_CNCP CreateNewContext; GLX_GVFFBCP GetVisualFromFBConfig; GLX_CWP CreateWindow; GLX_GPAP GetProcAddress; GLX_SIEP SwapIntervalEXT; GLX_CCAA CreateContextAttribsARB; } WinSyslibraryGLX;
    typedef struct WinSyswindowX11 { XID colormap; XID handle,parent; XIC ic; i32 overrideRedirect; int width,height,xpos,ypos,lastCurX,lastCurY,warpCursorPosX,warpCursorPosY; } WinSyswindowX11;
    typedef struct WinSyslibraryX11 { Display* display; int screen; XID root; XID hiddenCursorHandle; XContext context; short int keycodes[256],scancodes[349]; double restoreCurPosX, restoreCurPosY; WinSyswindow* disabledCursorWindow;
                                     Atom NET_SUPPORTED,NET_SUPPORTING_WM_CHECK,WM_PROTOCOLS,WM_STATE,WM_DELETE_WINDOW,NWM_NAME,NWM_ICON,NWM_PING,NWM_WINDOW_TYPE,NWM_WINDOW_TYPE_NORMAL,NWM_STATE,NWM_STATE_FULLSCREEN,NWM_BYPASS_COMPOSITOR,NET_WORKAREA,NET_CURRENT_DESKTOP,NET_ACTIVE_WINDOW,MOTIF_WM_HINTS,UTF8_STRING;
                                     struct { void* handle; PFN_XAllocSizeHints AllocSizeHints; PFN_XChangeProperty ChangeProperty; PFN_XChangeWindowAttributes ChangeWindowAttributes; PFN_XCheckTypedWindowEvent CheckTypedWindowEvent; PFN_XCreateColormap CreateColormap; PFN_XCreateWindow CreateWindow; PFN_XDefineCursor DefineCursor;
                                     PFN_XDeleteProperty DeleteProperty; PFN_XDisplayKeycodes DisplayKeycodes; PFN_XFilterEvent FilterEvent; PFN_XFindContext FindContext; PFN_XFree Free; PFN_XFreeEventData FreeEventData; PFN_XGetInputFocus GetInputFocus; PFN_XGetKeyboardMapping GetKeyboardMapping; PFN_XGetWMNormalHints GetWMNormalHints;
                                     PFN_XGetWindowAttributes GetWindowAttributes; PFN_XGetWindowProperty GetWindowProperty; PFN_XGrabPointer GrabPointer; PFN_XInternAtom InternAtom; PFN_XMapWindow MapWindow; PFN_XMoveResizeWindow MoveResizeWindow; PFN_XMoveWindow MoveWindow; PFN_XPending Pending; PFN_XQueryExtension QueryExtension;
                                     PFN_XQueryPointer QueryPointer; PFN_XRaiseWindow RaiseWindow; PFN_XResizeWindow ResizeWindow; PFN_XSaveContext SaveContext; PFN_XSendEvent SendEvent; PFN_XSetICFocus SetICFocus; PFN_XSetInputFocus SetInputFocus; PFN_XSetWMNormalHints SetWMNormalHints; PFN_XSetWMProtocols SetWMProtocols;
                                     PFN_XTranslateCoordinates TranslateCoordinates; PFN_XUndefineCursor UndefineCursor; PFN_XUngrabPointer UngrabPointer; PFN_XUnsetICFocus UnsetICFocus; PFN_XWarpPointer WarpPointer; } xlib;
                                     struct {void* handle; int eventBase,errorBase,major,minor; PFN_XRRFreeCrtcInfo FreeCrtcInfo; PFN_XRRFreeOutputInfo FreeOutputInfo; PFN_XRRFreeScreenResources FreeScreenResources; PFN_XRRGetCrtcInfo GetCrtcInfo; PFN_XRRGetOutputInfo GetOutputInfo; PFN_XRRGetOutputPrimary GetOutputPrimary; PFN_XRRGetScreenResourcesCurrent GetScreenResourcesCurrent; PFN_XRRSelectInput SelectInput; PFN_XRRUpdateConfiguration UpdateConfiguration;}randr; } WinSyslibraryX11;
    PFN_XNextEvent XNextEvent; typedef struct WinSysmonitorX11 { RROutput output; RRCrtc crtc; int index; } WinSysmonitorX11; typedef struct WinSyslibraryLinux { int inotify,watch; i32 dropped; } WinSyslibraryLinux;
    void GetCursorPosV(WinSyswindow*,double*,double*); void SetCurV(WinSyswindow*,double,double); struct WinSyslibrary { WinSysmonitor** monitors; int monitorCount; WinSyslibraryX11 x11; WinSyslibraryGLX glx; WinSyslibraryLinux linjs; };
    struct WinSyscontext { int client,source,major,minor; FGL_GIV GetIntegerv; void (*makeCurrent)(WinSyswindow*); void (*swapBuffers)(WinSyswindow*); void (*swapInterval)(int); WinSysglproc (*getProcAddress)(const char*); WinSyscontextGLX glx; };
    struct WinSyswindow { i32 decorated,cursorMode; char mouseButtons[8],keys[349]; double virtualCursorPosX,virtualCursorPosY; WinSyscontext context; WinSyswindowX11 x11; };
    struct WinSysmonitor { char name[128]; int widthMM,heightMM; vidmode currentMode; WinSysmonitorX11 x11; };
    void* WinSysPlatformLoadModule(const char* path) { return dlopen(path,2); }
    WinSysproc PlatformGetModuleSymbol(void* m, const char* n) { return dlsym(m,n); }
    u64 WinSysGetWindowPropertyX11(XID win, Atom prop, Atom type, u8** val) { Atom actType; i32 actFmt; u64 itemCount,bytesAfter; WinSys.x11.xlib.GetWindowProperty(WinSys.x11.display,win,prop,0,2147483647,0,type,&actType,&actFmt,&itemCount,&bytesAfter,val); return itemCount; }
    static void sendEventToWM(WinSyswindow* w, Atom type, i64 a, i64 b, i64 c, i64 d, i64 f) { XEvent e={33}; e.xclient.window = w->x11.handle; e.xclient.format = 32; e.xclient.message_type = type; e.xclient.data.l[0]=a; e.xclient.data.l[1]=b; e.xclient.data.l[2]=c; e.xclient.data.l[3]=d; e.xclient.data.l[4]=f; WinSys.x11.xlib.SendEvent(WinSys.x11.display,WinSys.x11.root,0,(1L<<19)|(1L<<20),&e); }
    static void updateNormalHints(WinSyswindow* w, int w_, int h) { XSizeHints* hs=WinSys.x11.xlib.AllocSizeHints(); i64 s; WinSys.x11.xlib.GetWMNormalHints(WinSys.x11.display,w->x11.handle,hs,&s); hs->flags &= ~((1L<<4)|(1L<<5)|(1L<<7)); hs->flags|=((1L<<4)|(1L<<5)); hs->min_width=hs->max_width=w_; hs->min_height=hs->max_height=h; WinSys.x11.xlib.SetWMNormalHints(WinSys.x11.display,w->x11.handle,hs); WinSys.x11.xlib.Free(hs); }
    static void updateCursorImage(WinSyswindow* w) { if (w->cursorMode==0x00034001) WinSys.x11.xlib.UndefineCursor(WinSys.x11.display,w->x11.handle); else WinSys.x11.xlib.DefineCursor(WinSys.x11.display,w->x11.handle,WinSys.x11.hiddenCursorHandle); }
    static void captureCursor(WinSyswindow* w) { WinSys.x11.xlib.GrabPointer(WinSys.x11.display,w->x11.handle,1,(1L<<2)|(1L<<3)|(1L<<6),1,1,w->x11.handle,0L,0L); }
    static void releaseCursor() { WinSys.x11.xlib.UngrabPointer(WinSys.x11.display,0L); }
    static void disableCursor(WinSyswindow* w) { WinSys.x11.disabledCursorWindow=w; GetCursorPosV(w,&WinSys.x11.restoreCurPosX,&WinSys.x11.restoreCurPosY); updateCursorImage(w); captureCursor(w); }
    static void enableCursor(WinSyswindow* w) { WinSys.x11.disabledCursorWindow = NULL; releaseCursor(); SetCurV(w,WinSys.x11.restoreCurPosX,WinSys.x11.restoreCurPosY); updateCursorImage(w); }
    void GetMonitorPos(WinSysmonitor* m, int* x, int* y) { XRRScreenResources* sr=WinSys.x11.randr.GetScreenResourcesCurrent(WinSys.x11.display,WinSys.x11.root); XRRCrtcInfo* ci=WinSys.x11.randr.GetCrtcInfo(WinSys.x11.display,sr,m->x11.crtc); if(ci){*x=ci->x; *y=ci->y; WinSys.x11.randr.FreeCrtcInfo(ci);} WinSys.x11.randr.FreeScreenResources(sr); }
    void SetWindowIcon(const WinSysIcon* image) {
        int lC=2+image[0].width*image[0].height; u64* icon=OS_Calloc(lC,sizeof(u64)), *t=icon; *t++=image[0].width; *t++=image[0].height;
        for (int j=0;j<image[0].width*image[0].height;++j) *t++=(((u64)image[0].pixels[j*4+0])<<16)|(((u64)image[0].pixels[j*4+1])<<8)|(((u64)image[0].pixels[j*4+2]))|(((u64)image[0].pixels[j*4+3])<<24);
        WinSys.x11.xlib.ChangeProperty(WinSys.x11.display,window->x11.handle,WinSys.x11.NWM_ICON,6,32,0,(u8*)icon,lC); OS_Free(icon,lC*sizeof(u64));
    }
    
    void GetWindowSize(WinSyswindow* w, int* w_, int* h) { XWindowAttributes a; WinSys.x11.xlib.GetWindowAttributes(WinSys.x11.display,w->x11.handle,&a); *w_=a.width; *h=a.height; }
    void SetWindowSize(int w_, int h) { w_=vmax(1,w_); h=vmax(1,h); updateNormalHints(window,w_,h); WinSys.x11.xlib.ResizeWindow(WinSys.x11.display,window->x11.handle,w_,h); }
    void SetWindowMonitor(int x,int y,int w_,int h) {
        updateNormalHints(window,w_,h); if (WinSys.x11.NWM_STATE && WinSys.x11.NWM_STATE_FULLSCREEN){sendEventToWM(window,WinSys.x11.NWM_STATE,0,WinSys.x11.NWM_STATE_FULLSCREEN,0,1,0);}else { XSetWindowAttributes a; WinSys.x11.xlib.ChangeWindowAttributes(WinSys.x11.display,window->x11.handle,(1L<<9),&a); window->x11.overrideRedirect=0; }
        WinSys.x11.xlib.DeleteProperty(WinSys.x11.display,window->x11.handle,WinSys.x11.NWM_BYPASS_COMPOSITOR); WinSys.x11.xlib.MoveResizeWindow(WinSys.x11.display,window->x11.handle,x,y,w_,h);
    }
    
    i32 WindowFocused() { XID f; int s; WinSys.x11.xlib.GetInputFocus(WinSys.x11.display,&f,&s); return window->x11.handle==f; }
    i32 WindowVisible() { XWindowAttributes w; WinSys.x11.xlib.GetWindowAttributes(WinSys.x11.display,window->x11.handle,&w); return w.map_state==2; }
    void GetWindowPos(WinSyswindow* w, int* x, int* y) { XID d; WinSys.x11.xlib.TranslateCoordinates(WinSys.x11.display,w->x11.handle,WinSys.x11.root,0,0,x,y,&d); }
    void SetWindowDecorated(WinSyswindow* w,i32 e) { struct {u64 f,fun,dec; i64 im; u64 st;} h={0}; h.f=2; h.dec=e?1:0; WinSys.x11.xlib.ChangeProperty(WinSys.x11.display,w->x11.handle,WinSys.x11.MOTIF_WM_HINTS,WinSys.x11.MOTIF_WM_HINTS,32,0,(u8*)&h,sizeof(h)/sizeof(i64)); }
    void GetCursorPosV(WinSyswindow* w, double* x, double* y) { XID r,c; int rx,ry,cx,cy; u32 m; WinSys.x11.xlib.QueryPointer(WinSys.x11.display,w->x11.handle,&r,&c,&rx,&ry,&cx,&cy,&m); *x=cx; *y=cy; }
    void SetCurV(WinSyswindow* w, double x, double y) { w->x11.warpCursorPosX=(int)x; w->x11.warpCursorPosY=(int)y; WinSys.x11.xlib.WarpPointer(WinSys.x11.display,0L,w->x11.handle,0,0,0,0,(int)x,(int)y); }    
    static const XRRModeInfo* getModeInfo(const XRRScreenResources* sr, RRMode id) { for (int i = 0;  i < sr->nmode;  i++) if (sr->modes[i].id == id) return sr->modes + i; return NULL; }
    static vidmode vidmodeFromModeInfo(const XRRModeInfo* mi, const XRRCrtcInfo* ci) { vidmode m; if(ci->rotation == 2 || ci->rotation == 8){m.width=mi->height; m.height=mi->width;} else { m.width = mi->width; m.height = mi->height; } m.refreshRate = (mi->hTotal && mi->vTotal) ? (int)vround((double) mi->dotClock / ((double) mi->hTotal * (double) mi->vTotal)) : 0; return m; }
    void PollMonitors() {
        XRRScreenResources* sr = WinSys.x11.randr.GetScreenResourcesCurrent(WinSys.x11.display,WinSys.x11.root); RROutput p = WinSys.x11.randr.GetOutputPrimary(WinSys.x11.display,WinSys.x11.root);
        int dC = WinSys.monitorCount; WinSysmonitor** d = NULL; if (dC) { d = OS_Calloc(WinSys.monitorCount,sizeof(WinSysmonitor*)); mcpy(d,WinSys.monitors,WinSys.monitorCount * sizeof(WinSysmonitor*)); }
        for (int i = 0;  i < sr->noutput;  i++) {
            int j, t, wMM, hMM;
            XRROutputInfo* oi = WinSys.x11.randr.GetOutputInfo(WinSys.x11.display, sr, sr->outputs[i]);
            if (oi->connection != 0 || oi->crtc == 0L) { WinSys.x11.randr.FreeOutputInfo(oi); continue; }
            for (j=0;j<dC;++j) if(d[j] && d[j]->x11.output == sr->outputs[i]){d[j]=NULL; break;} 
            if (j < dC) { WinSys.x11.randr.FreeOutputInfo(oi); continue; }
            XRRCrtcInfo* ci = WinSys.x11.randr.GetCrtcInfo(WinSys.x11.display, sr, oi->crtc); if (!ci) { WinSys.x11.randr.FreeOutputInfo(oi); continue; }
            if (ci->rotation == 2 || ci->rotation == 8) { wMM = oi->mm_height; hMM = oi->mm_width; } 
            else { wMM = oi->mm_width; hMM = oi->mm_height; }
            if (wMM <= 0 || hMM <= 0) { wMM = (int) (ci->width * 25.4f / 96.f); hMM = (int) (ci->height * 25.4f / 96.f); }
            WinSysmonitor* m = AllocMonitor(oi->name, wMM, hMM);
            m->x11.output = sr->outputs[i]; m->x11.crtc = oi->crtc;
            t = (m->x11.output == p) ? 0 : 1; InputMonitor(m,0x00040001,t); WinSys.x11.randr.FreeOutputInfo(oi); WinSys.x11.randr.FreeCrtcInfo(ci);
        }
        WinSys.x11.randr.FreeScreenResources(sr);
        for (int i=0;i<dC;++i) if (d[i]) InputMonitor(d[i],0x00040002,0);
        if (d) OS_Free(d,WinSys.monitorCount*sizeof(WinSysmonitor*));
    }
    
    static void processEvent(XEvent* e) {
        u32 kc=0; if (e->type==2 || e->type==3) kc=e->xkey.keycode;
        Bool f=WinSys.x11.xlib.FilterEvent(e,0L);
        if (e->type==WinSys.x11.randr.eventBase+1) { WinSys.x11.randr.UpdateConfiguration(e); PollMonitors(); return; }
        if (e->type==35) return;
        WinSyswindow* w=NULL; if (WinSys.x11.xlib.FindContext(WinSys.x11.display,e->xany.window,WinSys.x11.context,(XPointer*)&w)!=0) return;
        switch (e->type) {
            case 21: w->x11.parent=e->xreparent.parent; return;
            case 2: case 3: {
                const int k=(kc<0||kc>255)?KEY_UNKNOWN:WinSys.x11.keycodes[kc]; if (k==KEY_UNKNOWN) return;
                if (e->type==3) {  XEvent n; if(WinSys.x11.xlib.CheckTypedWindowEvent(WinSys.x11.display,e->xany.window,2,&n)){ if(n.xkey.keycode == kc && n.xkey.h == e->xkey.h){InputKey(w->keys,k,INPUT_PRESS); return;}else{InputKey(w->keys,k,INPUT_RELEASE); processEvent(&n); return;} }  }
                const int a=(e->type==2)?INPUT_PRESS:INPUT_RELEASE; InputKey(w->keys,k,a); return; }
            case 4: case 5: {
                const int b = e->xbutton.button; const int a = (e->type == 4) ? INPUT_PRESS : INPUT_RELEASE;
                if (b == 1) InputMouseClick(w->mouseButtons,MOUSE_BUTTON_LEFT,a);
                else if (b == 2) InputMouseClick(w->mouseButtons,MOUSE_BUTTON_MIDDLE,a);
                else if (b == 3) InputMouseClick(w->mouseButtons,MOUSE_BUTTON_RIGHT,a);
                else if (a == INPUT_PRESS && b == 4) Sys_Input.scrollDelta += 1.0f;
                else if (a == INPUT_PRESS && b == 5) Sys_Input.scrollDelta -= 1.0f;
                else if (b > 7) InputMouseClick(w->mouseButtons,b - 5,a);
                return; }
            case 7: { const int x=e->xcrossing.x,y=e->xcrossing.y; InputCursorPos(&w->virtualCursorPosX,&w->virtualCursorPosY,x,y); w->x11.lastCurX=x; w->x11.lastCurY=y; return; }
            case 6: { const int x=e->xmotion.x, y=e->xmotion.y;
                if (x!=w->x11.warpCursorPosX || y!=w->x11.warpCursorPosY) {
                    if (w->cursorMode==0x00034003) { if(WinSys.x11.disabledCursorWindow!=w) return; InputCursorPos(&w->virtualCursorPosX,&w->virtualCursorPosY,w->virtualCursorPosX + (x - w->x11.lastCurX),w->virtualCursorPosY + (y - w->x11.lastCurY)); }
                    else InputCursorPos(&w->virtualCursorPosX,&w->virtualCursorPosY,x,y);
                }
                w->x11.lastCurX=x; w->x11.lastCurY=y; return; }
            case 22: {
                if (e->xcfg.width!=w->x11.width || e->xcfg.height!=w->x11.height) { w->x11.width=e->xcfg.width; w->x11.height=e->xcfg.height; UpdateScreenSize(e->xcfg.width,e->xcfg.height); }
                int x=e->xcfg.x, y=e->xcfg.y;
                if (!e->xany.send_event && w->x11.parent!=WinSys.x11.root) { XID d; WinSys.x11.xlib.TranslateCoordinates(WinSys.x11.display,w->x11.parent,WinSys.x11.root,x,y,&x,&y,&d); }
                if (x!=w->x11.xpos || y!=w->x11.ypos) { w->x11.xpos=x; w->x11.ypos=y; }
                return; }
            case 33: { if(f || e->xclient.message_type==0L) return; if (e->xclient.message_type==WinSys.x11.WM_PROTOCOLS) { const Atom p=e->xclient.data.l[0]; if (p==0L) return; if (p == WinSys.x11.WM_DELETE_WINDOW) OS_Exit(0); if (p == WinSys.x11.NWM_PING) { XEvent r=*e; r.xclient.window=WinSys.x11.root; WinSys.x11.xlib.SendEvent(WinSys.x11.display,WinSys.x11.root,0,(1L<<19)|(1L<<20),&r); } } return; }
            case  9: { if (e->xfocus.mode==1 || e->xfocus.mode==2) return; if (w->cursorMode==0x00034003) disableCursor(w); if (w->x11.ic) WinSys.x11.xlib.SetICFocus(w->x11.ic); InputWindowFocus(1); return; }
            case 10: { if (e->xfocus.mode==1 || e->xfocus.mode==2) return; if (w->cursorMode==0x00034003) enableCursor(w); if (w->x11.ic) WinSys.x11.xlib.UnsetICFocus(w->x11.ic); InputWindowFocus(0); return; }
        }
    }
    
    void GetMonitorWorkarea(WinSysmonitor* m,int* x,int* y,int* w,int* h) {
        int aW = 0, aH = 0; XRRScreenResources* sr = WinSys.x11.randr.GetScreenResourcesCurrent(WinSys.x11.display,WinSys.x11.root); XRRCrtcInfo* ci = WinSys.x11.randr.GetCrtcInfo(WinSys.x11.display,sr,m->x11.crtc);
        const XRRModeInfo* mi = getModeInfo(sr,ci->mode); int aX = ci->x, aY = ci->y;
        if (ci->rotation == 2 || ci->rotation == 8) { aW = mi->height, aH = mi->width; } 
        else { aW = mi->width, aH = mi->height; }
        WinSys.x11.randr.FreeCrtcInfo(ci); WinSys.x11.randr.FreeScreenResources(sr);
        if (WinSys.x11.NET_WORKAREA && WinSys.x11.NET_CURRENT_DESKTOP) {
            Atom *e = NULL, *d = NULL;
            const unsigned long eC = WinSysGetWindowPropertyX11(WinSys.x11.root,WinSys.x11.NET_WORKAREA,6,(u8**) &e);
            if (WinSysGetWindowPropertyX11(WinSys.x11.root,WinSys.x11.NET_CURRENT_DESKTOP,6,(u8**) &d) > 0) {
                if (eC >= 4 && *d < eC / 4) {
                    const int gx = e[*d * 4 + 0], gy = e[*d * 4 + 1], gw = e[*d * 4 + 2], gh = e[*d * 4 + 3];
                    if (aX < gx) { aW -= gx - aX, aX = gx; }
                    if (aY < gy) { aH -= gy - aY, aY = gy; }
                    if (aX +  aW > gx + gw) aW = gx - aX + gw;
                    if (aY + aH > gy + gh) aH = gy - aY + gh;
                }
            }
            if (e) WinSys.x11.xlib.Free(e); if (d) WinSys.x11.xlib.Free(d);
        }
        *x = aX; *y = aY; *w = aW; *h = aH;
    }
    
    void GetVideoMode(WinSysmonitor* m, vidmode* v) { XRRScreenResources* sr = WinSys.x11.randr.GetScreenResourcesCurrent(WinSys.x11.display,WinSys.x11.root); const XRRModeInfo* mi=NULL; XRRCrtcInfo* ci = WinSys.x11.randr.GetCrtcInfo(WinSys.x11.display,sr,m->x11.crtc); if(ci){ mi = getModeInfo(sr,ci->mode); if(mi)*v = vidmodeFromModeInfo(mi,ci); WinSys.x11.randr.FreeCrtcInfo(ci); } WinSys.x11.randr.FreeScreenResources(sr); }
    static int translateKeySyms(const KeySym* k, int w) {
        if (w > 1) { if (k[1] >= 0xffb0 && k[1] <= 0xffb9) return KEY_KP_0 + (k[1] - 0xffb0); switch (k[1]) { case 0xffac: case 0xffae: return KEY_KP_DECIMAL; case 0xffbd:return KEY_KP_EQUAL; case 0xff8d:return KEY_KP_ENTER; } }
        KeySym c = k[0]; if(c >= 0x0061 && c <= 0x007a){return KEY_A + (c - 0x0061);} if(c >= 0x0030 && c <= 0x0039){return KEY_0 + (c - 0x0030);} if(c >= 0xffbe && c <= 0xffd6){return KEY_F1 + (c - 0xffbe);}
        if(c >= 0xff95 && c <= 0xff9f){static const int o[] = {KEY_KP_7,KEY_KP_4,KEY_KP_8,KEY_KP_6,KEY_KP_2,KEY_KP_9,KEY_KP_3,KEY_KP_1,-1,KEY_KP_0,KEY_KP_DECIMAL }; int r = o[c - 0xff95]; if(r != -1) return r;} 
        switch (c) {
            case 0xff1b: return KEY_ESCAPE;        case 0xff09: return KEY_TAB;          case 0xff0d: return KEY_ENTER;
            case 0xff08: return KEY_BACKSPACE;     case 0xffff: return KEY_DELETE;       case 0xff50: return KEY_HOME;
            case 0xff57: return KEY_END;           case 0xff55: return KEY_PAGE_UP;      case 0xff56: return KEY_PAGE_DOWN;
            case 0xff63: return KEY_INSERT;        case 0xff51: return KEY_LEFT;         case 0xff53: return KEY_RIGHT;
            case 0xff54: return KEY_DOWN;          case 0xff52: return KEY_UP;           case 0xff13: return KEY_PAUSE;
            case 0xff14: return KEY_SCROLL_LOCK;   case 0xff61: return KEY_PRINT_SCREEN; case 0xff7f: return KEY_NUM_LOCK;
            case 0xffe5: return KEY_CAPS_LOCK;     case 0xff67: return KEY_MENU;         case 0xffe1: return KEY_LEFT_SHIFT;
            case 0xffe2: return KEY_RIGHT_SHIFT;   case 0xffe3: return KEY_LEFT_CONTROL; case 0xffe4: return KEY_RIGHT_CONTROL;
            case 0xffe7: case 0xffe9: return KEY_LEFT_ALT;   
            case 0xff7e: case 0xfe03: case 0xffe8: case 0xffea: return KEY_RIGHT_ALT; 
            case 0xffeb: return KEY_LEFT_SUPER;    case 0xffec: return KEY_RIGHT_SUPER;  case 0xffaa: return KEY_KP_MULTIPLY;
            case 0xffab: return KEY_KP_ADD;        case 0xffad: return KEY_KP_SUBTRACT;  case 0xffaf: return KEY_KP_DIVIDE;
            case 0xffbd: return KEY_KP_EQUAL;      case 0xff8d: return KEY_KP_ENTER;     case 0x0020: return KEY_SPACE;
            case 0x0027: return KEY_APOSTROPHE;    case 0x002c: return KEY_COMMA;        case 0x002d: return KEY_MINUS;
            case 0x002e: return KEY_PERIOD;        case 0x002f: return KEY_SLASH;        case 0x003b: return KEY_SEMICOLON;
            case 0x003d: return KEY_EQUAL;         case 0x005b: return KEY_LEFT_BRACKET; case 0x005c: return KEY_BACKSLASH;
            case 0x005d: return KEY_RIGHT_BRACKET; case 0x0060: return KEY_GRAVE_ACCENT; default: return KEY_UNKNOWN;
        }
    }
    
    static void createKeyTables() {
        int sMin, sMax; mset(WinSys.x11.keycodes,-1,sizeof(WinSys.x11.keycodes)); mset(WinSys.x11.scancodes,-1,sizeof(WinSys.x11.scancodes)); WinSys.x11.xlib.DisplayKeycodes(WinSys.x11.display,&sMin,&sMax);
        int w; KeySym* k = WinSys.x11.xlib.GetKeyboardMapping(WinSys.x11.display,sMin,sMax - sMin + 1,&w);
        for (int s = sMin; s <= sMax; s++) { if (WinSys.x11.keycodes[s] < 0) WinSys.x11.keycodes[s] = translateKeySyms(&k[(s - sMin) * w],w); if (WinSys.x11.keycodes[s] > 0) WinSys.x11.scancodes[WinSys.x11.keycodes[s]] = s; }
        WinSys.x11.xlib.Free(k);
    }
    
    static Atom getAtomIfSupported(Atom* a, unsigned long c, const char* n) { const Atom t=WinSys.x11.xlib.InternAtom(WinSys.x11.display,n,0); for (unsigned long i=0;i<c;i++) if (a[i] == t) return t; return 0L; }
    void PollEvents() { WinSys.x11.xlib.Pending(WinSys.x11.display); while (((_XPrivDisplay)(WinSys.x11.display))->qlen) { XEvent e; XNextEvent(WinSys.x11.display,&e); processEvent(&e); } WinSyswindow* w = WinSys.x11.disabledCursorWindow; if(w){ int w_,h; GetWindowSize(w,&w_,&h); if(w->x11.lastCurX!=w_/2 || w->x11.lastCurY!=h/2) SetCurV(w,w_/2,h/2); } }
    static void makeContextCurrentGLX(WinSyswindow* w) { WinSys.glx.MakeCurrent(WinSys.x11.display,w->context.glx.window,w->context.glx.handle); }
    static void swapBuffersGLX(WinSyswindow* w) { WinSys.glx.SwapBuffers(WinSys.x11.display, w->context.glx.window); }
    static void swapIntervalGLX(int i) { WinSys.glx.SwapIntervalEXT(WinSys.x11.display,window->context.glx.window,i); }
    static WinSysglproc getProcAddressGLX(const char* p) { return WinSys.glx.GetProcAddress((const u8*) p); }
    void SetWindowPosition(WinSyswindow* w, int x, int y) { 
        if (!WindowVisible()) { i64 s; XSizeHints* h=WinSys.x11.xlib.AllocSizeHints(); if (WinSys.x11.xlib.GetWMNormalHints(WinSys.x11.display,w->x11.handle,h,&s)) {h->flags|=(1L<<2); h->x=h->y=0; WinSys.x11.xlib.SetWMNormalHints(WinSys.x11.display,w->x11.handle,h);} WinSys.x11.xlib.Free(h); } 
        WinSys.x11.xlib.MoveWindow(WinSys.x11.display,w->x11.handle,x,y); 
    }
    
    WinSyswindow* VCreateWindow(int width, int height) {
        WinSyswindow* w = OS_Calloc(1,sizeof(WinSyswindow)); w->decorated = 1; w->cursorMode = 0x00034003;
        const char* names[] = {"libGLX.so.0","libGL.so.1","libGL.so",NULL};
        for (int i=0;names[i] && !WinSys.glx.handle;i++) WinSys.glx.handle = WinSysPlatformLoadModule(names[i]);
        WinSys.glx.GetFBConfigs = (GLX_GFBCP)PlatformGetModuleSymbol(WinSys.glx.handle,"glXGetFBConfigs");                  WinSys.glx.GetFBConfigAttrib = (GLX_GFBCAP)PlatformGetModuleSymbol(WinSys.glx.handle,"glXGetFBConfigAttrib");
        WinSys.glx.QueryExtension = (GLX_QEP)PlatformGetModuleSymbol(WinSys.glx.handle,"glXQueryExtension");                WinSys.glx.QueryVersion = (GLX_QVP)PlatformGetModuleSymbol(WinSys.glx.handle,"glXQueryVersion");
        WinSys.glx.MakeCurrent = (GLX_MCP)PlatformGetModuleSymbol(WinSys.glx.handle,"glXMakeCurrent");                      WinSys.glx.SwapBuffers = (GLX_SBP)PlatformGetModuleSymbol(WinSys.glx.handle,"glXSwapBuffers");
        WinSys.glx.QueryExtensionsString = (GLX_QESP)PlatformGetModuleSymbol(WinSys.glx.handle,"glXQueryExtensionsString"); WinSys.glx.CreateNewContext = (GLX_CNCP)PlatformGetModuleSymbol(WinSys.glx.handle,"glXCreateNewContext");
        WinSys.glx.CreateWindow = (GLX_CWP)PlatformGetModuleSymbol(WinSys.glx.handle,"glXCreateWindow");                    WinSys.glx.GetVisualFromFBConfig = (GLX_GVFFBCP)PlatformGetModuleSymbol(WinSys.glx.handle,"glXGetVisualFromFBConfig");
        WinSys.glx.GetProcAddress = (GLX_GPAP)PlatformGetModuleSymbol(WinSys.glx.handle,"glXGetProcAddress");               WinSys.glx.SwapIntervalEXT = (GLX_SIEP)getProcAddressGLX("glXSwapIntervalEXT");
        WinSys.glx.CreateContextAttribsARB = (GLX_CCAA)getProcAddressGLX("glXCreateContextAttribsARB");
        WinSys.glx.QueryExtension(WinSys.x11.display,&WinSys.glx.errorBase,&WinSys.glx.eventBase); WinSys.glx.QueryVersion(WinSys.x11.display,&WinSys.glx.major,&WinSys.glx.minor); GLXFBConfig native; XVisualInfo* result;
        GLXFBConfig* nativeConfigs; FBC* usableConfigs; const FBC* closest; int nativeCount,usableCount; nativeConfigs=WinSys.glx.GetFBConfigs(WinSys.x11.display,WinSys.x11.screen,&nativeCount); usableConfigs=OS_Calloc(nativeCount,sizeof(FBC)); usableCount=0;
        for (int i = 0;  i < nativeCount;  i++) {
            const GLXFBConfig n = nativeConfigs[i]; FBC* u = usableConfigs + usableCount; int v;
            WinSys.glx.GetFBConfigAttrib(WinSys.x11.display,n,0x8011,&v); if(!(v & 1)) continue;
            WinSys.glx.GetFBConfigAttrib(WinSys.x11.display,n,0x8010,&v); if(!(v & 1)) continue;
            WinSys.glx.GetFBConfigAttrib(WinSys.x11.display,n,5,&v); if(v != 1) continue;
            u->handle = (uintptr_t) n; usableCount++;
        }
        closest = ChooseFBConfig(usableConfigs,usableCount); native = (GLXFBConfig)closest->handle;
        WinSys.x11.xlib.Free(nativeConfigs); if (usableConfigs) OS_Free(usableConfigs,nativeCount*sizeof(FBC));
        result = WinSys.glx.GetVisualFromFBConfig(WinSys.x11.display,native);
        Visual* visual=result->visual; int depth = result->depth; WinSys.x11.xlib.Free(result); 
        w->x11.colormap=WinSys.x11.xlib.CreateColormap(WinSys.x11.display,WinSys.x11.root,visual,0);
        XSetWindowAttributes wa = {0}; wa.colormap = w->x11.colormap; wa.event_mask = 0x63807F;
        w->x11.parent=WinSys.x11.root; w->x11.handle=WinSys.x11.xlib.CreateWindow(WinSys.x11.display,WinSys.x11.root,0,0,width,height,0,depth,1,visual,(1L<<3)|(1L<<13)|(1L<<11),&wa);
        WinSys.x11.xlib.SaveContext(WinSys.x11.display,w->x11.handle,WinSys.x11.context,(XPointer)w); 
        Atom protocols[]={WinSys.x11.WM_DELETE_WINDOW,WinSys.x11.NWM_PING};
        WinSys.x11.xlib.SetWMProtocols(WinSys.x11.display,w->x11.handle,protocols,2);
        if (WinSys.x11.NWM_WINDOW_TYPE && WinSys.x11.NWM_WINDOW_TYPE_NORMAL) { Atom t=WinSys.x11.NWM_WINDOW_TYPE_NORMAL; WinSys.x11.xlib.ChangeProperty(WinSys.x11.display,w->x11.handle,WinSys.x11.NWM_WINDOW_TYPE,4,32,0,(u8*)&t,1); }
        XSizeHints* sz=WinSys.x11.xlib.AllocSizeHints();
        sz->flags|=((1L << 4)|(1L << 5)); sz->min_width=sz->max_width=width; sz->min_height=sz->max_height=height; sz->flags|=(1L << 9); sz->win_gravity=10;
        WinSys.x11.xlib.SetWMNormalHints(WinSys.x11.display,w->x11.handle,sz); WinSys.x11.xlib.Free(sz);
        WinSys.x11.xlib.ChangeProperty(WinSys.x11.display,w->x11.handle,WinSys.x11.NWM_NAME,WinSys.x11.UTF8_STRING,8,0,(u8*)GAME_TITLE,sizeof(GAME_TITLE) - 1); 
        GetWindowPos(w,&w->x11.xpos,&w->x11.ypos); GetWindowSize(w,&w->x11.width,&w->x11.height);
        int attribs[40],index=0; attribs[index++] = 0x2091; attribs[index++] = 4; attribs[index++] = 0x2092; attribs[index++] = 3; attribs[index++] = 0x9126; attribs[index++] = 1; attribs[index++] = 0; attribs[index++] = 0;
        w->context.glx.handle = WinSys.glx.CreateContextAttribsARB(WinSys.x11.display,native,NULL,1,attribs); w->context.glx.window = WinSys.glx.CreateWindow(WinSys.x11.display,native,w->x11.handle,NULL);
        w->context.glx.fbconfig = native; w->context.makeCurrent = makeContextCurrentGLX; w->context.swapBuffers = swapBuffersGLX; w->context.swapInterval = swapIntervalGLX;
        w->context.getProcAddress = getProcAddressGLX; WinSys.x11.xlib.MapWindow(WinSys.x11.display,w->x11.handle);
        if (WinSys.x11.NET_ACTIVE_WINDOW) sendEventToWM(w,WinSys.x11.NET_ACTIVE_WINDOW,1,0,0,0,0);
        else if (WindowVisible()) { WinSys.x11.xlib.RaiseWindow(WinSys.x11.display,w->x11.handle); WinSys.x11.xlib.SetInputFocus(WinSys.x11.display,w->x11.handle,2,0L); }
        return w;
    }
#endif
WinSyslibrary WinSys={0};
int WindowInit() {
    mset(&WinSys,0,sizeof(WinSys));
    #if defined(_WIN32)
        GetModuleHandleExW(0x6,(const u16*)&WinSys,(HMODULE*)&WinSys.win32.instance);
        WinSys.win32.dwmapi.instance = LoadLibraryA("dwmapi.dll");
        if (WinSys.win32.dwmapi.instance) { WinSys.win32.dwmapi.IsCompositionEnabled = (PFN_DwmIsCompositionEnabled)PlatformGetModuleSymbol(WinSys.win32.dwmapi.instance, "DwmIsCompositionEnabled"); WinSys.win32.dwmapi.Flush = (PFN_DwmFlush)PlatformGetModuleSymbol(WinSys.win32.dwmapi.instance, "DwmFlush"); }
        WinSys.win32.ntdll.instance = LoadLibraryA("ntdll.dll");
        if (WinSys.win32.ntdll.instance) WinSys.win32.ntdll.RtlVerifyVersionInfo = (PFN_RtlVerifyVersionInfo)PlatformGetModuleSymbol(WinSys.win32.ntdll.instance, "RtlVerifyVersionInfo");
        createKeyTables();
        MSG msg; WNDCLASSEXW wc={0}; wc.cbSize=sizeof(wc); 
        wc.style = 0x0020; wc.lpfnWndProc = (WNDPROC)helperWindowProc; wc.hInstance = WinSys.win32.instance; wc.n = L"WinSys3 Helper";
        WinSys.win32.helperWindowClass = RegisterClassExW(&wc);
        WinSys.win32.helperWindowHandle = CreateWindowExW(0x300,(u16*)(uintptr_t)WinSys.win32.helperWindowClass,L"WinSys message window",0x06000000,0,0,1,1,NULL,NULL,WinSys.win32.instance,NULL);
        while (PeekMessageW(&msg,WinSys.win32.helperWindowHandle,0,0,0x0001)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
        WinSysPollMonitorsWin32();
    #else
        void* module = WinSysPlatformLoadModule("libX11.so.6");
        PFN_XInitThreads XInitThreads = (PFN_XInitThreads)PlatformGetModuleSymbol(module,"XInitThreads"); PFN_XOpenDisplay XOpenDisplay = (PFN_XOpenDisplay)PlatformGetModuleSymbol(module,"XOpenDisplay");
        XInitThreads(); Display* display = XOpenDisplay(NULL);
        WinSys.x11.display = display; WinSys.x11.xlib.handle = module;
        #define X(n) WinSys.x11.xlib.n = (PFN_X##n)PlatformGetModuleSymbol(WinSys.x11.xlib.handle, "X" #n);
            X(AllocSizeHints) X(ChangeProperty) X(CheckTypedWindowEvent) X(CreateColormap) X(CreateWindow) X(ChangeWindowAttributes) X(DefineCursor) X(DeleteProperty) X(DisplayKeycodes) X(FilterEvent) X(FindContext) X(Free) X(UngrabPointer) X(FreeEventData) X(GetInputFocus) X(GetKeyboardMapping) X(GetWMNormalHints) X(GetWindowAttributes) X(GetWindowProperty)
            X(GrabPointer) X(InternAtom) X(MapWindow) X(MoveResizeWindow) X(MoveWindow) X(Pending) X(UnsetICFocus) X(QueryExtension) X(QueryPointer) X(RaiseWindow) X(ResizeWindow) X(SaveContext) X(SendEvent) X(SetICFocus) X(SetInputFocus) X(SetWMNormalHints) X(SetWMProtocols) X(TranslateCoordinates) X(UndefineCursor) X(WarpPointer)
        #undef X
        XNextEvent = (PFN_XNextEvent)PlatformGetModuleSymbol(WinSys.x11.xlib.handle,"XNextEvent");
        WinSys.x11.screen = ((_XPrivDisplay)(WinSys.x11.display))->default_screen;
        WinSys.x11.root = (&((_XPrivDisplay)(WinSys.x11.display))->screens[WinSys.x11.screen])->root;
        WinSys.x11.context = 1;
        WinSys.x11.randr.handle = WinSysPlatformLoadModule("libXrandr.so.2");
        #define X(n) WinSys.x11.randr.n = (PFN_XRR##n)PlatformGetModuleSymbol(WinSys.x11.randr.handle,"XRR"#n);
            X(FreeCrtcInfo) X(FreeOutputInfo) X(FreeScreenResources) X(GetCrtcInfo) X(GetOutputInfo) X(GetOutputPrimary) X(GetScreenResourcesCurrent) X(SelectInput) X(UpdateConfiguration)
        #undef X
        XRRScreenResources* sr = WinSys.x11.randr.GetScreenResourcesCurrent(WinSys.x11.display,WinSys.x11.root);
        WinSys.x11.randr.FreeScreenResources(sr); WinSys.x11.randr.SelectInput(WinSys.x11.display,WinSys.x11.root,4); void* xcurhandle = WinSysPlatformLoadModule("libXcursor.so.1");
        PFN_XCIC ImageCreate = (PFN_XCIC)PlatformGetModuleSymbol(xcurhandle,"XcursorImageCreate"); PFN_XCID ImageDestroy = (PFN_XCID)PlatformGetModuleSymbol(xcurhandle,"XcursorImageDestroy"); PFN_XCILC ImageLoadCursor = (PFN_XCILC)PlatformGetModuleSymbol(xcurhandle,"XcursorImageLoadCursor");
        createKeyTables();
        #define IA(n) WinSys.x11.xlib.InternAtom(WinSys.x11.display,n,0)
            WinSys.x11.UTF8_STRING=IA("UTF8_STRING"); WinSys.x11.WM_PROTOCOLS=IA("WM_PROTOCOLS"); WinSys.x11.WM_STATE=IA("WM_STATE");  WinSys.x11.WM_DELETE_WINDOW=IA("WM_DELETE_WINDOW"); WinSys.x11.NET_SUPPORTED =IA("_NET_SUPPORTED"); WinSys.x11.NET_SUPPORTING_WM_CHECK=IA("_NET_SUPPORTING_WM_CHECK"); WinSys.x11.NWM_ICON=IA("_NET_WM_ICON"); WinSys.x11.NWM_PING=IA("_NET_WM_PING"); WinSys.x11.NWM_NAME=IA("_NET_WM_NAME"); WinSys.x11.NWM_BYPASS_COMPOSITOR=IA("_NET_WM_BYPASS_COMPOSITOR"); WinSys.x11.MOTIF_WM_HINTS=IA("_MOTIF_WM_HINTS");
        #undef IA
        XID* wfr = NULL; WinSysGetWindowPropertyX11(WinSys.x11.root,WinSys.x11.NET_SUPPORTING_WM_CHECK,33,(u8**)&wfr);
        XID* wfc = NULL; WinSysGetWindowPropertyX11(*wfr,WinSys.x11.NET_SUPPORTING_WM_CHECK,33,(u8**)&wfc);
        WinSys.x11.xlib.Free(wfr); WinSys.x11.xlib.Free(wfc); Atom* sa = NULL; const unsigned long ac = WinSysGetWindowPropertyX11(WinSys.x11.root,WinSys.x11.NET_SUPPORTED,4,(u8**)&sa);
        #define GA(name) getAtomIfSupported(sa, ac, name)
            WinSys.x11.NWM_STATE=GA("_NET_WM_STATE"); WinSys.x11.NWM_STATE_FULLSCREEN=GA("_NET_WM_STATE_FULLSCREEN"); WinSys.x11.NWM_WINDOW_TYPE=GA("_NET_WM_WINDOW_TYPE"); WinSys.x11.NWM_WINDOW_TYPE_NORMAL=GA("_NET_WM_WINDOW_TYPE_NORMAL"); WinSys.x11.NET_WORKAREA=GA("_NET_WORKAREA"); WinSys.x11.NET_CURRENT_DESKTOP=GA("_NET_CURRENT_DESKTOP"); WinSys.x11.NET_ACTIVE_WINDOW=GA("_NET_ACTIVE_WINDOW");
        #undef GA
        if (sa) WinSys.x11.xlib.Free(sa);
        XSetWindowAttributes wa; wa.event_mask = (1L<<22); WinSys.x11.xlib.CreateWindow(WinSys.x11.display,WinSys.x11.root,0,0,1,1,0,0,2,(&((_XPrivDisplay)(WinSys.x11.display))->screens[WinSys.x11.screen])->root_visual,(1L<<11),&wa);
        XcursorImage* native = ImageCreate(16,16); mset(native->pixels,0,256*sizeof(XcursorUInt)); native->xhot=native->yhot=0; WinSys.x11.hiddenCursorHandle = ImageLoadCursor(WinSys.x11.display,native); ImageDestroy(native);
        PollMonitors();
    #endif
    return 1;
}

const FBC* ChooseFBConfig(const FBC* a, u32 c) { u32 l=-1; const FBC* f=0; for (u32 i = 0; i < c; i++) { const FBC* x=a+i; u32 s=(((x->alphaBits==0)+(x->depthBits==0)+(x->stencilBits==0))*65536) + (((8-x->redBits)*(8-x->redBits) + (8-x->greenBits)*(8-x->greenBits) + (8-x->blueBits)*(8-x->blueBits))*256) + ((8-x->alphaBits)*(8-x->alphaBits) + (8-x->depthBits)*(8-x->depthBits) + (8-x->stencilBits)*(8-x->stencilBits)); if(s < l){l=s; f=x;} } return f; }
void SetGLContext_GetFunctionPointers() {
    WinSyswindow* h=window; h->context.makeCurrent(h);
    #define X(n,t) n=(t)h->context.getProcAddress(#n);
    X(glClear,FGL_C)              X(glClearColor,FGL_CC)      X(glColorMask,FGL_CM)        X(glDepthFunc,FGL_DF)           X(glDepthMask,FGL_DM)               X(glDisable,FGL_D)             X(glEnableVertexAttribArray,FGL_EVAA)
    X(glEnable,FGL_E)             X(glFinish,FGL_F)           X(glFlush,FGL_FL)            X(glFrontFace,FGL_FF)           X(glGetError,FGL_GERR)              X(glGetIntegerv,FGL_GIV)       X(glCheckFramebufferStatus,FGL_CFBS) 
    X(glLineWidth,FGL_LW)         X(glReadBuffer,FGL_RB)      X(glReadPixels,FGL_RP)       X(glTexImage2D,FGL_T2D)         X(glViewport,FGL_VP)                X(glBindTexture,FGL_BT)        X(glCopyTexSubImage2D,FGL_CTSI2D)
    X(glDrawArrays,FGL_DA)        X(glDrawElements,FGL_DE)    X(glGenTextures,FGL_GT)      X(glActiveTexture,FGL_AT)       X(glBlendFuncSeparate,FGL_BFS)      X(glBindVertexArray,FGL_BVA)   X(glVertexAttribBinding,FGL_VAB)
    X(glBindBuffer,FGL_BB)        X(glBufferData,FGL_BD)      X(glGenBuffers,FGL_GB)       X(glUnmapBuffer,FGL_UB)         X(glAttachShader,FGL_AS)            X(glCompileShader,FGL_CS)      X(glClearBufferFv,FGL_CBFV)
    X(glCreateProgram,FGL_CP)     X(glCreateShader,FGL_CRS)   X(glDrawBuffers,FGL_DB)      X(glGetProgramiv,FGL_CPIV)      X(glGetShaderInfoLog,FGL_GSIL)      X(glGetShaderiv,FGL_GSIV)
    X(glLinkProgram,FGL_LP)       X(glShaderSource,FGL_SS)    X(glUniform1f,FGL_U1F)       X(glUniform1i,FGL_U1I)          X(glUniform2f,FGL_U2F)              X(glUniform3f,FGL_U3F)
    X(glUniform4f,FGL_U4F)        X(glTexParameteri,FGL_TPI)  X(glUniform1ui,FGL_U1UI)     X(glUniform2ui,FGL_U2UI)        X(glUniformMatrix3fv,FGL_UM3FV)     X(glUniformMatrix4fv,FGL_UM4FV)
    X(glUseProgram,FGL_UP)        X(glBindBufferBase,FGL_BBB) X(glBindFramebuffer,FGL_BFB) X(glGenFramebuffers,FGL_GFS)    X(glMapBufferRange,FGL_MBR)         X(glBindImageTexture,FGL_BIT)
    X(glBindVertexBuffer,FGL_BVB) X(glDispatchCompute,FGL_DC) X(glGenVertexArrays,FGL_GVA) X(glVertexAttribFormat,FGL_VAF) X(glFramebufferTexture2D,FGL_FBT2D) X(glBufferSubData,FGL_BSD)
    #undef X
}

size_t monitorAllocationSize = 0;
void InputMonitor(WinSysmonitor* m, int a, int p) {
    if (a == 0x00040001) {
        WinSys.monitorCount++;
        WinSys.monitors = WinSys.monitors ? OS_Realloc(WinSys.monitors,monitorAllocationSize,sizeof(WinSysmonitor*) * WinSys.monitorCount) : OS_Alloc(WinSys.monitorCount * sizeof(WinSysmonitor*));
        monitorAllocationSize = WinSys.monitorCount * sizeof(WinSysmonitor*);
        if (p == 0) { mmov(WinSys.monitors + 1,WinSys.monitors,((size_t) WinSys.monitorCount - 1) * sizeof(WinSysmonitor*)); WinSys.monitors[0] = m; }
        else WinSys.monitors[WinSys.monitorCount - 1] = m;
    } else if (a == 0x00040002) {
        for (int i=0;i<WinSys.monitorCount;++i) {
            if (WinSys.monitors[i] == m) { WinSys.monitorCount--; mmov(WinSys.monitors + i, WinSys.monitors + i + 1,((size_t) WinSys.monitorCount - i) * sizeof(WinSysmonitor*)); break; }
        }
    }
}

WinSysmonitor* AllocMonitor(const char* n, int w, int h) { WinSysmonitor* m = OS_Calloc(1, sizeof(WinSysmonitor)); m->widthMM = w; m->heightMM = h; scpy_to_a_from_b(m->name,n,sizeof(m->name)); return m; }
WinSysmonitor** WinSysGetMonitors(int* c) { *c = WinSys.monitorCount; return WinSys.monitors; }
WinSysmonitor* GetPrimaryMonitor(void) { if (!WinSys.monitorCount) return NULL; return WinSys.monitors[0]; }
void WinSysGetMonitorPos(WinSysmonitor* m, int* x, int* y) { *x = 0; *y = 0; GetMonitorPos(m,x,y); }
void WinSysGetMonitorWorkarea(WinSysmonitor* m, int* x, int* y, int* w, int* h) { *x=*y=*w=*h=0; GetMonitorWorkarea(m,x,y,w,h); }
const vidmode* WinSysGetVideoMode(WinSysmonitor* m) { GetVideoMode(m,&m->currentMode); return &m->currentMode; }
void InputWindowFocus(i32 f) { window_has_focus = f != 0; ignore_next_mouse_delta = true; WinSyswindow* w = window; if (!f) { for (int k=0;k<=348;++k) if (w->keys[k] == INPUT_PRESS) InputKey(w->keys,k,INPUT_RELEASE); for (int b=0;b<=7;++b) if (w->mouseButtons[b] == INPUT_PRESS) InputMouseClick(w->mouseButtons,b,INPUT_RELEASE); } }
void CenterWindowOnMonitor() {
    int c; WinSysmonitor** monitors=WinSysGetMonitors(&c); if (Sys_Settings.CurrentMonitor > (c - 1)) { Sys_Settings.CurrentMonitor=0; SaveConfig(); } WinSysmonitor* next=monitors[Sys_Settings.CurrentMonitor]; int mx,my; WinSysGetMonitorPos(next,&mx,&my);
    const vidmode* mode=WinSysGetVideoMode(next); int xpos=mx + (mode->width - Sys_Settings.ScreenWidth)/2, ypos=my + (mode->height - Sys_Settings.ScreenHeight)/2; SetWindowPosition(window,xpos,ypos); ignore_next_mouse_delta=true;
    #if defined(_WIN32)
        void* h = window->win32.handle; ShowWindow(h,5); BringWindowToTop(h); SetForegroundWindow(h); SetFocus(h);
    #endif
}

WinSysmonitor* GetCurrentMonitor() {
    int wx=0,wy=0,ww=0,wh=0; GetWindowPos(window,&wx,&wy); GetWindowSize(window,&ww,&wh); WinSysmonitor* bM = GetPrimaryMonitor(); int bA=0,c; WinSysmonitor** monitors = WinSysGetMonitors(&c);
    for (int i=0;i<c;++i) {
        int mx,my; WinSysGetMonitorPos(monitors[i],&mx,&my); const vidmode* mode = WinSysGetVideoMode(monitors[i]);
        int l=vmax(wx,mx), r=vmin(wx + ww,mx + mode->width), t=vmax(wy,my), b=vmin(wy + wh,my + mode->height);
        int area = (r > l && b > t) ? (r - l) * (b - t) : 0;
        if (area > bA) { bA = area; bM = monitors[i]; }
    }
    return bM;
}

void ChangeResolution() {
    if (resDropdownCount > 1) {
        resSelectedIdx = (resSelectedIdx + 1) % resDropdownCount; Sys_Settings.ScreenWidth = (u32)resModes[resSelectedIdx].w; Sys_Settings.ScreenHeight = (u32)resModes[resSelectedIdx].h;
        WinSysmonitor* m = GetCurrentMonitor(); if(!m) m=GetPrimaryMonitor();
        int mx,my; WinSysGetMonitorPos(m,&mx,&my); const vidmode* desktop = WinSysGetVideoMode(m);
        int x = mx + (desktop->width - (int)Sys_Settings.ScreenWidth) / 2, y = my + (desktop->height - (int)Sys_Settings.ScreenHeight) / 2;
        SetWindowSize((int)Sys_Settings.ScreenWidth,(int)Sys_Settings.ScreenHeight); SetWindowPosition(window,x,y); UpdateScreenSize((int)Sys_Settings.ScreenWidth,(int)Sys_Settings.ScreenHeight);
        resDropdownOpen = false; SaveConfig();
    }
}

void GatherResolutionModes() {
    resDropdownCount = 0; WinSysmonitor* m = GetCurrentMonitor(); if (!m) m=GetPrimaryMonitor(); const vidmode* d = WinSysGetVideoMode(m); if(!d) return;
    
    static const struct {int w,h;} cr[] = {{320,200},{640,400},{640,480},{800,600},{1024,768},{1280,720},{1280,800},{1366,768},{1440,900},{1600,900},{1920,1080},{2560,1440}};
    int maxW = d->width, maxH = d->height,j;
    for (int i = 0; i < 12 && resDropdownCount < 8; ++i) {
        if (cr[i].w > maxW || cr[i].h > maxH || cr[i].w < 320 || cr[i].h < 200) continue;
        
        for (j = 0; j < resDropdownCount; ++j) { if (resModes[j].w == cr[i].w && resModes[j].h == cr[i].h) break; }
        if (j == resDropdownCount) resModes[resDropdownCount++] = (ResMode){cr[i].w,cr[i].h};
    }
    
    if (resDropdownCount < 8) resModes[resDropdownCount++] = (ResMode){d->width,d->height};
    resSelectedIdx = 0;
    for (int i = 0; i < resDropdownCount; ++i) { if(resModes[i].w == (int)Sys_Settings.ScreenWidth && resModes[i].h == (int)Sys_Settings.ScreenHeight){resSelectedIdx=i; break;} }
}

void ChangeFullScreenWindowed(bool adjustToFit) {
    int x,y,w,h,mx,my,c; WinSysmonitor** monitors = WinSysGetMonitors(&c); WinSysmonitor* m = monitors[Sys_Settings.CurrentMonitor]; const vidmode* mo = WinSysGetVideoMode(m); WinSysGetMonitorWorkarea(m,&x,&y,&w,&h);
    window->decorated = (i32)(!Sys_Settings.Fullscreen); SetWindowDecorated(window,(i32)(!Sys_Settings.Fullscreen));
    if (Sys_Settings.Fullscreen) {SetWindowMonitor(x,y,w,h); Sys_Settings.ScreenWidth = w; Sys_Settings.ScreenHeight = h;}
    else { WinSysGetMonitorPos(m,&mx,&my);
        
    if (adjustToFit) {Sys_Settings.ScreenWidth = vmax(vmin((w*3)/4,1366),320); Sys_Settings.ScreenHeight = vmax(vmin((h*3)/4,768),200); }
    SetWindowMonitor(mx + (mo->width - Sys_Settings.ScreenWidth) / 2,my + (mo->height - Sys_Settings.ScreenHeight) / 2,Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight); }
    UpdateScreenSize(Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight);
}

void CycleToNextMonitor() {
    static double monitorSwitchTime;
    if (get_time() >= monitorSwitchTime) {
        monitorSwitchTime = get_time() + 0.5; int c; WinSysmonitor** monitors = WinSysGetMonitors(&c);
        if (Sys_Settings.CurrentMonitor > (c - 1)) { Sys_Settings.CurrentMonitor = 0; SaveConfig(); }
        if (monitors && c >= 2) { Sys_Settings.CurrentMonitor = (Sys_Settings.CurrentMonitor + 1) % c; SaveConfig(); CenterWindowOnMonitor(); }
    }
}

void SetVSync() { window->context.swapInterval((i32)Sys_Settings.Vsync); }
