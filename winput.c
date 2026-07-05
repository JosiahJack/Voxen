// winput.c - Windowing and Input System interfacing with the OS.
typedef void (*WinSysglproc)(void); typedef struct WinSyswindow WinSyswindow; WinSyswindow* window; typedef struct { int width,height,redBits,greenBits,blueBits,refreshRate; } vidmode; typedef struct { u8 buttons[15]; float axes[6]; } WinSysgamepadstate;
typedef void (*WinSysproc)(void); typedef struct WinSysfbconfig WinSysfbconfig; typedef struct WinSyscontext WinSyscontext; typedef struct WinSyswindow WinSyswindow; typedef struct WinSyslibrary WinSyslibrary; typedef struct WinSysmonitor WinSysmonitor; typedef struct WinSysjoystick WinSysjoystick;
struct WinSysfbconfig { int redBits,greenBits,blueBits,alphaBits,depthBits,stencilBits,accumRedBits,accumGreenBits,accumBlueBits,accumAlphaBits; i32 samples,stereo,sRGB,doublebuffer; uintptr_t handle; }; extern WinSyslibrary WinSys;
WinSysproc PlatformGetModuleSymbol(void* module, const char* name); void UpdateScreenSize(i32 width, i32 height); void SaveConfig(); void InputWindowFocus(i32); void InputKey(WinSyswindow*,int,int); void InputMouseClick(WinSyswindow*,int,int); void InputCursorPos(WinSyswindow*,double,double); void JoystickConnection(WinSysjoystick*,int); void InputJoystickAxis(WinSysjoystick*,int,float);
void InputJoystickButton(WinSysjoystick*,int,char); void InputJoystickHat(WinSysjoystick*,int,char); void InputMonitor(WinSysmonitor*,int,int); const WinSysfbconfig* ChooseFBConfig(const WinSysfbconfig* alternatives, u32); WinSysmonitor* AllocMonitor(const char*,int,int); WinSysjoystick* WinSysAllocJoystick(const char*,const char*,int,int,int); void FreeJoystick(WinSysjoystick*);
#define INPUT_RELEASE 0
#define INPUT_PRESS   1
#define INPUT_REPEAT  2
#if defined(WINDOWS)
    #define MAKEWORD(a,b) ((u16) (((u8) (((u64) (a)) & 0xff)) | ((u16) ((u8) (((u64) (b)) & 0xff))) << 8))
    #define MAKELONG(a, b) ((i32) (((u16) (((u64) (a)) & 0xffff)) | ((u32) ((u16) (((u64) (b)) & 0xffff))) << 16))
    #define LOWORD(l) ((u16)(((u64) (l)) & 0xffff))
    #define HIWORD(l) ((u16)((((u64) (l)) >> 16) & 0xffff))
    #define LOBYTE(w) ((u8)(((u64) (w)) & 0xff))
    #define HIBYTE(w) ((u8)((((u64) (w)) >> 8) & 0xff))
    typedef void *HWND, *HBITMAP, *HBRUSH, *HDC, *HGLRC, *HICON, *HMENU, *HMONITOR;
    typedef struct tagPOINT { i32 x,y; } POINT,*PPOINT,*NPPOINT,*LPPOINT; typedef struct _POINTL { i32 x,y; } POINTL,*PPOINTL; typedef struct tagRECT { i32 left,top,right,bottom; } RECT,*PRECT,*NPRECT,*LPRECT; typedef struct tagSIZE { i32 cx,cy; } SIZE,*PSIZE,*LPSIZE;
    typedef struct _OSVERSIONINFOEXW { u32 dwOSVersionInfoSize,dwMajorVersion,dwMinorVersion,dwBuildNumber,dwPlatformId; u16 szCSDVersion[128]; u16 wServicePackMajor,wServicePackMinor,wSuiteMask; u8 wProductType,wReserved; } OSVERSIONINFOEXW;
    int __cdecl wcscmp(const u16 *_Str1,const u16 *_Str2); u16* wcscpy(u16* restrict destination, const u16* restrict source);
    #define MAKEINTATOM(i) (u16*)((u64)((u16)(i)))
    typedef struct _devicemodeW {u16 dmDeviceName[32]; u16 dmSpecVersion,dmDriverVersion,dmSize,dmDriverExtra; u32 dmFields; union { struct { i16 dmOrientation,dmPaperSize,dmPaperLength,dmPaperWidth,dmScale,dmCopies,dmDefaultSource,dmPrintQuality; }; struct { POINTL dmPosition; u32 dmDisplayOrientation,dmDisplayFixedOutput; }; };
                                 i16 dmColor,dmDuplex,dmYResolution,dmTTOption,dmCollate; u16 dmFormName[32],dmLogPixels; u32 dmBitsPerPel,dmPelsWidth,dmPelsHeight; union { u32 dmDisplayFlags,dmNup; }; u32 dmDisplayFrequency,dmICMMethod,dmICMIntent,dmMediaType,dmDitherType,dmReserved1,dmReserved2,dmPanningWidth,dmPanningHeight; } DEVMODEW,*LPDEVMODEW;
    typedef i64 (__stdcall *WNDPROC)(HWND,u32,u64,i64); typedef i32 (__stdcall *MONITORENUMPROC)(HMONITOR,HDC,LPRECT,i64);
    typedef struct _ICONINFO { i32 fIcon; u32 xHotspot,yHotspot; HBITMAP hbmMask,hbmColor; } ICONINFO; typedef ICONINFO *PICONINFO; typedef struct tagMSG { HWND hwnd; u32 message; u64 wParam; i64 lParam; u32 time; POINT pt; } MSG,*PMSG,*NPMSG,*LPMSG;
    typedef struct tagMONITORINFO { u32 cbSize; RECT rcMonitor; RECT rcWork; u32 dwFlags; } MONITORINFO,*LPMONITORINFO;             typedef struct tagMONITORINFOEXW { u32 cbSize; RECT rcMonitor; RECT rcWork; u32 dwFlags; u16 szDevice[32]; } MONITORINFOEXW;
    typedef struct tagWINDOWPLACEMENT { u32 length; u32 flags; u32 showCmd; POINT ptMinPosition; POINT ptMaxPosition; RECT rcNormalPosition; } WINDOWPLACEMENT;
    typedef struct tagWNDCLASSEXW { u32 cbSize,style; WNDPROC lpfnWndProc; i32 cbClsExtra,cbWndExtra; HINSTANCE hInstance; HICON hIcon,hCursor; HBRUSH hbrBackground; u16 *lpszMenuName,*lpszClassName; HICON hIconSm; } WNDCLASSEXW;
    typedef struct {u16 wButtons; u8 bLeftTrigger,bRightTrigger; i16 sThumbLX,sThumbLY,sThumbRX,sThumbRY; } XINPUT_GAMEPAD; typedef struct {u16 wLeftMotorSpeed, wRightMotorSpeed;} XINPUT_VIBRATION;
    typedef struct {u8 Type,SubType; u16 Flags; XINPUT_GAMEPAD Gamepad; XINPUT_VIBRATION Vibration;} XINPUT_CAPABILITIES;   typedef struct {u32 dwPacketNumber; XINPUT_GAMEPAD Gamepad;} XINPUT_STATE;
    typedef u32 (WINAPI * PFN_XInputGetCapabilities)(u32,u32,XINPUT_CAPABILITIES*);                                         typedef u32 (WINAPI * PFN_XInputGetState)(u32,XINPUT_STATE*);
    typedef struct { u32 dbch_size,dbch_devicetype,dbch_reserved; } DEV_BROADCAST_HDR;                                      typedef struct { u32 dbcc_size,dbcc_devicetype,dbcc_reserved; GUID dbcc_classguid; u16 dbcc_name[1]; } DEV_BROADCAST_DEVICEINTERFACE_W;
    typedef i32 (WINAPI * PFN_DwmIsCompositionEnabled)(i32*);            typedef i32 (WINAPI * PFN_DwmFlush)();                  typedef i32 (WINAPI * PFN_RtlVerifyVersionInfo)(OSVERSIONINFOEXW*,u32,u64); typedef i32 (WINAPI * PFN_SWE)(int);
    typedef i32 (WINAPI * PFN_GPFAIVA)(HDC,int,int,u32,const int*,int*); typedef HGLRC (WINAPI * FP_CCAA)(HDC,HGLRC,const int*); typedef HGLRC (WINAPI * PFN_CC)(HDC);                                       typedef PROC (WINAPI * PFN_wglGetProcAddress)(const char*);
    typedef HDC (WINAPI * PFN_wglGetCurrentDC)();                        typedef HGLRC (WINAPI * PFN_wglGetCurrentContext)();    typedef i32 (WINAPI * PFN_wglMakeCurrent)(HDC,HGLRC);
    typedef struct WGLContext { HDC dc; HGLRC handle; int interval; } WGLContext;
    PFN_wglGetCurrentDC wglGetCurrentDC; PFN_CC wglCreateContext; FP_CCAA wglCreateContextAttribsARB; PFN_wglGetCurrentContext wglGetCurrentContext; PFN_wglMakeCurrent wglMakeCurrent; PFN_wglGetProcAddress wglGetProcAddress; PFN_GPFAIVA wglGetPixelFormatAttribivARB; PFN_SWE wglSwapIntervalEXT;
    typedef struct WinSyslibraryWGL { HINSTANCE instance; } WinSyslibraryWGL;
    typedef struct WinSyswindowWin32 { HWND handle; i32 cursorTracked,frameAction,keymenu; int width,height,lastCurX,lastCurY; } WinSyswindowWin32;
    typedef struct WinSyslibraryWin32 { HINSTANCE instance; HWND helperWindowHandle; u16 helperWindowClass,mainWindowClass; void* deviceNotificationHandle; short int keycodes[512],scancodes[349]; double restoreCurPosX,restoreCurPosY; WinSyswindow *disabledCursorWindow, *capturedCursorWindow; HICON blankCursor; struct {HINSTANCE instance; PFN_XInputGetCapabilities GetCapabilities; PFN_XInputGetState GetState;} xinput; struct {HINSTANCE instance; PFN_DwmIsCompositionEnabled IsCompositionEnabled; PFN_DwmFlush Flush;} dwmapi; struct {HINSTANCE instance; PFN_RtlVerifyVersionInfo RtlVerifyVersionInfo;} ntdll;} WinSyslibraryWin32;
    typedef struct WinSysmonitorWin32 { HMONITOR handle; u16 adapterName[32],displayName[32]; char publicAdapterName[32],publicDisplayName[32]; i32 modesPruned,modeChanged; } WinSysmonitorWin32;
    typedef struct WinSysjoystickWin32{ int objectCount; u32 index; GUID guid; } WinSysjoystickWin32;
    typedef long FXPT2DOT30; typedef struct tagCIEXYZ { FXPT2DOT30 x,y,z; } CIEXYZ; typedef struct tagICEXYZTRIPLE {CIEXYZ r,g,b;} CIEXYZTRIPLE;
    typedef struct _DISPLAY_DEVICEW { u32 cb; u16 DeviceName[32],DeviceString[128]; u32 StateFlags; u16 DeviceID[128],DeviceKey[128]; } DISPLAY_DEVICEW,*PDISPLAY_DEVICEW,*LPDISPLAY_DEVICEW;
    typedef struct tagPIXELFORMATDESCRIPTOR { u16 nSize,nVersion; u32 dwFlags; u8 iPixelType,cColorBits,cRedBits,cRedShift,cGreenBits,cGreenShift,cBlueBits,cBlueShift,cAlphaBits,cAlphaShift,cAccumBits,cAccumRedBits,cAccumGreenBits,cAccumBlueBits,cAccumAlphaBits,cDepthBits,cStencilBits,cAuxBuffers,iLayerType,bReserved; u32 dwLayerMask,dwVisibleMask,dwDamageMask; } PIXELFORMATDESCRIPTOR,*PPIXELFORMATDESCRIPTOR,*LPPIXELFORMATDESCRIPTOR;
    typedef struct { u32 bV5Size; i32 bV5Width,bV5Height; u16 bV5Planes,bV5BitCount; u32 bV5Compression,bV5SizeImage; i32 bV5XPelsPerMeter; i32 bV5YPelsPerMeter; u32 bV5ClrUsed,bV5ClrImportant,bV5RedMask,bV5GreenMask,bV5BlueMask,bV5AlphaMask,bV5CSType; CIEXYZTRIPLE bV5Endpoints; u32 bV5GammaRed,bV5GammaGreen,bV5GammaBlue,bV5Intent,bV5ProfileData,bV5ProfileSize,bV5Reserved; } BITMAPV5HEADER,*LPBITMAPV5HEADER,*PBITMAPV5HEADER;
    typedef struct tagRGBQUAD { u8 rgbBlue,rgbGreen,rgbRed,rgbReserved; } RGBQUAD;
    typedef struct tagBITMAPINFOHEADER { u32 biSize; i32 biWidth,biHeight; u16 biPlanes,biBitCount; u32 biCompression; u32 biSizeImage; i32 biXPelsPerMeter; i32 biYPelsPerMeter; u32 biClrUsed; u32 biClrImportant; } BITMAPINFOHEADER,*LPBITMAPINFOHEADER,*PBITMAPINFOHEADER;
    typedef struct tagBITMAPINFO { BITMAPINFOHEADER bmiHeader; RGBQUAD bmiColors[1]; } BITMAPINFO,*LPBITMAPINFO,*PBITMAPINFO;
    DLL_IMP HICON WINAPI CreateIconIndirect(PICONINFO); DLL_IMP HDC WINAPI GetDC(HWND); DLL_IMP i32 WINAPI GetModuleHandleExW(u32,const u16*,HINSTANCE*); DLL_IMP int WINAPI ReleaseDC(HWND,HDC); DLL_IMP i32 WINAPI SetCursorPos(int,int); DLL_IMP int WINAPI WideCharToMultiByte(u32,u32,u16*,int,char*,int,const char*,i32*); DLL_IMP HICON WINAPI SetCursor(HICON); DLL_IMP i32 WINAPI GetCursorPos(LPPOINT);
    DLL_IMP int WINAPI MultiByteToWideChar(u32,u32,const char*,int,u16*,int); DLL_IMP i32 WINAPI ClipCursor(const RECT*); DLL_IMP i32 WINAPI ClientToScreen(HWND,LPPOINT); DLL_IMP HDC WINAPI CreateDCW(const u16*,const u16*,const u16*,const DEVMODEW*); DLL_IMP void* WINAPI GetPropW(HWND,u16*); DLL_IMP i32 WINAPI GetMessageTime(); DLL_IMP i32 WINAPI GetClientRect(HWND,LPRECT);
    DLL_IMP HICON WINAPI LoadCursorW(HINSTANCE,u16*); DLL_IMP u32 WINAPI MapVirtualKeyW(u32,u32); DLL_IMP i32 WINAPI SetWindowPos(HWND,HWND,int,int,int,int,u32); DLL_IMP HWND WINAPI SetCapture(HWND hWnd); DLL_IMP i32 WINAPI ReleaseCapture(); DLL_IMP i32 WINAPI PeekMessageW(LPMSG,HWND,u32,u32,u32); DLL_IMP i32 WINAPI AdjustWindowRect(LPRECT,u32,i32);DLL_IMP i32 WINAPI GetWindowLongW(HWND,int);
    DLL_IMP i64 WINAPI DefWindowProcW(HWND,u32,u64,i64); DLL_IMP HMONITOR WINAPI MonitorFromWindow(HWND,u32);DLL_IMP HWND WINAPI GetActiveWindow(); DLL_IMP i32 WINAPI AdjustWindowRectEx(LPRECT,u32,i32,u32); DLL_IMP i64 WINAPI SendMessageW(HWND,u32,u64,i64); DLL_IMP i32 WINAPI SetWindowLongW(HWND,int,i32); DLL_IMP i32 WINAPI GetMonitorInfoW(HMONITOR,LPMONITORINFO); 
    DLL_IMP i32 WINAPI TranslateMessage(const MSG*); DLL_IMP i16 WINAPI GetKeyState(int); DLL_IMP i64 WINAPI DispatchMessageW(const MSG*); DLL_IMP i32 WINAPI ShowWindow(HWND,int); DLL_IMP i32 WINAPI BringWindowToTop(HWND); DLL_IMP i32 WINAPI SetWindowPlacement(HWND,const WINDOWPLACEMENT*); DLL_IMP HWND WINAPI SetFocus(HWND); DLL_IMP i32 WINAPI SetForegroundWindow(HWND); 
    DLL_IMP i32 WINAPI GetWindowPlacement(HWND,WINDOWPLACEMENT*); DLL_IMP i32 WINAPI SetPropW(HWND,u16*,void*); DLL_IMP i32 WINAPI OffsetRect(LPRECT,int,int); DLL_IMP HWND WINAPI CreateWindowExW(u32,u16*,u16*,u32,int,int,int,int,HWND,HMENU,HINSTANCE,void*); DLL_IMP u64 WINAPI VerSetConditionMask(u64,u32,u8); DLL_IMP u16 WINAPI RegisterClassExW(const WNDCLASSEXW *); DLL_IMP i32 WINAPI DeleteObject(void*); 
    DLL_IMP i32 WINAPI DeleteDC(HDC); DLL_IMP void* WINAPI RegisterDeviceNotificationW(void*,void*,u32); DLL_IMP i32 WINAPI SwapBuffers(HDC); DLL_IMP i32 WINAPI EnumDisplayMonitors(HDC,const RECT*,MONITORENUMPROC,i64); DLL_IMP i32 WINAPI EnumDisplaySettingsW(u16*,u32,LPDEVMODEW); DLL_IMP i32 WINAPI EnumDisplayDevicesW(u16*,u32,PDISPLAY_DEVICEW,u32);
    DLL_IMP i32 WINAPI EnumDisplaySettingsExW(u16*,u32,LPDEVMODEW,u32); DLL_IMP i32 WINAPI SetPixelFormat(HDC,i32,const PIXELFORMATDESCRIPTOR *); DLL_IMP i32 WINAPI ChoosePixelFormat(HDC hdc,const PIXELFORMATDESCRIPTOR *ppfd); DLL_IMP i32 WINAPI DescribePixelFormat(HDC,i32,u32,LPPIXELFORMATDESCRIPTOR); DLL_IMP HBITMAP WINAPI CreateBitmap(i32,i32,u32,u32,const void *); 
    DLL_IMP HBITMAP WINAPI CreateDIBSection(HDC,const BITMAPINFO*,u32,void**,void*,u32); DLL_IMP i32 WINAPI GetDeviceCaps(HDC,i32);
    u16* CreateWideStringFromUTF8Win32(const char* source); i32 IsWindowsVersionOrGreaterWin32(u16 major, u16 minor, u16 sp); void WinSysPollMonitorsWin32();
    struct WinSysjoystick { i32 allocated,connected; size_t axesSize,buttonsSize,hatsSize; float*  axes; int axisCount; u8* buttons; int buttonCount; u8* hats; int hatCount; char name[128],guid[33]; WinSysjoystickWin32 win32; };
    struct WinSyslibrary { WinSysmonitor** monitors; int monitorCount; i32 joysInited; WinSysjoystick joysticks[JOYSTICK_LAST + 1]; WinSyslibraryWin32 win32; WinSyslibraryWGL wgl; };
    struct WinSyscontext { int client,source,major,minor; void (*makeCurrent)(WinSyswindow*); void (*swapBuffers)(WinSyswindow*); void (*swapInterval)(int); WinSysglproc (*getProcAddress)(const char*); WGLContext wgl; };
    struct WinSyswindow { i32 decorated,doublebuffer; vidmode videoMode; int cursorMode; char mouseButtons[8],keys[349]; double virtualCursorPosX,virtualCursorPosY; WinSyscontext context; WinSyswindowWin32 win32; };
    struct WinSysmonitor { char name[128]; int widthMM,heightMM; vidmode currentMode; WinSysmonitorWin32 win32; };
    static u32 getWindowStyle(const WinSyswindow* w) { return 0x060A0000 | (w->decorated ? 0x00C00000 : 0x80000000); } // clipping,sysmenu,minimize,title,border,and borderless raw
    static HICON createIcon(const WinSysIcon* image,int xhot,int yhot,i32 icon) {
        HDC dc; HICON handle; HBITMAP color,mask; u8* target=NULL; u8* source=image->pixels;
        BITMAPV5HEADER bi={0}; bi.bV5Size=sizeof(bi); bi.bV5Width=image->width; bi.bV5Height=-image->height; bi.bV5Planes=1; bi.bV5BitCount=32; bi.bV5Compression=3; bi.bV5RedMask=0x00ff0000; bi.bV5GreenMask=0x0000ff00; bi.bV5BlueMask=0x000000ff; bi.bV5AlphaMask=0xff000000;
        dc=GetDC(NULL); color=CreateDIBSection(dc,(BITMAPINFO*)&bi,0,(void**)&target,NULL,(u32)0U); ReleaseDC(NULL,dc); mask=CreateBitmap(image->width,image->height,1,1,NULL);
        for (int i=0;i<image->width*image->height;i++) { target[0]=source[2]; target[1]=source[1]; target[2]=source[0]; target[3]=source[3]; target+=4; source+=4; }
        ICONINFO ii={0}; ii.fIcon=icon; ii.xHotspot=xhot; ii.yHotspot=yhot; ii.hbmMask=mask; ii.hbmColor=color; handle=CreateIconIndirect(&ii); DeleteObject(color); DeleteObject(mask); return handle;
    }

    static void updateCursorImage(WinSyswindow* win) { if (win->cursorMode==0x00034001/*WinSys_CURSOR_NORMAL*/) {SetCursor(LoadCursorW(NULL,(u16*)((u64)(u16)32512)));} else {SetCursor(WinSys.win32.blankCursor);} }
    static void captureCursor(WinSyswindow* win) { RECT clipRect; GetClientRect(win->win32.handle,&clipRect); ClientToScreen(win->win32.handle,(POINT*)&clipRect.left); ClientToScreen(win->win32.handle,(POINT*)&clipRect.right); ClipCursor(&clipRect); WinSys.win32.capturedCursorWindow=win; }
    static void releaseCursor() { ClipCursor(NULL); WinSys.win32.capturedCursorWindow=NULL; }
    static void disableCursor(WinSyswindow* win) { WinSys.win32.disabledCursorWindow = win; POINT pos; GetCursorPos(&pos); WinSys.win32.restoreCurPosX = pos.x; WinSys.win32.restoreCurPosY = pos.y; updateCursorImage(win); captureCursor(win); }
    static void SetCurV(WinSyswindow* win, double xpos, double ypos) { win->win32.lastCurX = (int)xpos; win->win32.lastCurY = (int)ypos; POINT pos = {(int)xpos,(int)ypos}; ClientToScreen(win->win32.handle,&pos); SetCursorPos(pos.x,pos.y); }
    static void enableCursor(WinSyswindow* win) { WinSys.win32.disabledCursorWindow = NULL; releaseCursor(); SetCurV(win,WinSys.win32.restoreCurPosX,WinSys.win32.restoreCurPosY); updateCursorImage(win); }
    static i64 __stdcall windowProc(HWND hWnd, u32 uMsg, u64 wParam, i64 lParam) {
        WinSyswindow* win=GetPropW(hWnd,L"WinSys"); if (!win) return DefWindowProcW(hWnd,uMsg,wParam,lParam);
        switch (uMsg) {
            case 0x0021/*WM_MOUSEACTIVATE*/:  if (HIWORD(lParam) == 0x0201/*WM_LBUTTONDOWN*/ && LOWORD(lParam)!=1) {win->win32.frameAction= 1;} break;
            case 0x0215/*WM_CAPTURECHANGED*/: if (lParam==0&&win->win32.frameAction) { if (win->cursorMode==0x00034003/*CURSOR_DISABLED*/) {disableCursor(win);} win->win32.frameAction=0; } break;
            case 0x0007/*WM_SETFOCUS*/:   InputWindowFocus(1); if (win->win32.frameAction) {break;} if (win->cursorMode==0x00034003/*CURSOR_DISABLED*/) {disableCursor(win);} return 0;
            case 0x0008/*WM_KILLFOCUS*/:  if (win->cursorMode==0x00034003/*CURSOR_DISABLED*/) {enableCursor(win);} InputWindowFocus(0); return 0;
            case 0x0112/*WM_SYSCOMMAND*/: switch (wParam&0xfff0) { case 0xF140/*SC_SCREENSAVE*/: case 0xF170/*SC_MONITORPOWER*/: break; case 0xF100/*SC_KEYMENU*/: if (!win->win32.keymenu) return 0; break; } break;
            case 0x0010/*WM_CLOSE*/:      OS_Exit(0);
            case 0x0100/*WM_KEYDOWN*/: case 0x0104/*WM_SYSKEYDOWN*/: case 0x0101/*WM_KEYUP*/: case 0x0105/*WM_SYSKEYUP*/: {
                const int action=(HIWORD(lParam)&0x8000)?INPUT_RELEASE:INPUT_PRESS;
                int scancode=(HIWORD(lParam)&(0x0100|0xff));
                if (!scancode) scancode=MapVirtualKeyW((u32)wParam,0);
                if (scancode==0x54) {scancode=0x137;}   if (scancode==0x146) {scancode=0x45;}   if (scancode==0x136) {scancode=0x36;}
                int key = WinSys.win32.keycodes[scancode];
                if (wParam==0x11/*VK_CONTROL*/) {
                    if (HIWORD(lParam)&0x0100) key=KEY_RIGHT_CONTROL;
                    else {
                        MSG next; const u32 time=GetMessageTime();
                        if (PeekMessageW(&next,NULL,0,0,0)) {
                            if (next.message == 0x0100/*WM_KEYDOWN*/ || next.message == 0x0104/*WM_SYSKEYDOWN*/ || next.message == 0x0101/*WM_KEYUP*/ || next.message == 0x0105/*WM_SYSKEYUP*/) {
                                if (next.wParam == 0x12/*VK_MENU*/ && (HIWORD(next.lParam) & 0x0100)&&next.time==time) break;
                            }
                        }
                        key=KEY_LEFT_CONTROL;
                    }
                } else if (wParam == 0xE5/*VK_PROCESSKEY*/) break;
                if (action == INPUT_RELEASE && wParam == 0x10/*VK_SHIFT*/) { InputKey(win,KEY_LEFT_SHIFT,action); InputKey(win,KEY_RIGHT_SHIFT,action); }
                else if (wParam == 0x2C/*VK_SNAPSHOT*/) { InputKey(win,key,INPUT_PRESS); InputKey(win,key,INPUT_RELEASE); }
                else InputKey(win,key,action);
                break; }
            case 0x0201/*WM_LBUTTONDOWN*/: case 0x0204/*WM_RBUTTONDOWN*/: case 0x0207/*WM_MBUTTONDOWN*/: case 0x020B/*WM_XBUTTONDOWN*/: case 0x0202/*WM_LBUTTONUP*/:   case 0x0205/*WM_RBUTTONUP*/:   case 0x0208/*WM_MBUTTONUP*/:   case 0x020C/*WM_XBUTTONUP*/: {
                int i,action,button = (uMsg==0x0201/*WM_LBUTTONDOWN*/ || uMsg == 0x0202/*WM_LBUTTONUP*/) ? MOUSE_BUTTON_LEFT : ((uMsg == 0x0204/*WM_RBUTTONDOWN*/ || uMsg == 0x0205/*WM_RBUTTONUP*/) ? MOUSE_BUTTON_RIGHT : ((uMsg == 0x0207/*WM_MBUTTONDOWN*/ || uMsg == 0x0208/*WM_MBUTTONUP*/) ? MOUSE_BUTTON_MIDDLE : (((HIWORD(wParam)) == 0x0001/*XBUTTON1*/) ? MOUSE_BUTTON_4 : MOUSE_BUTTON_5)));
                action=(uMsg == 0x0201/*WM_LBUTTONDOWN*/ || uMsg == 0x0204/*WM_RBUTTONDOWN*/ || uMsg == 0x0207/*WM_MBUTTONDOWN*/ || uMsg == 0x020B/*WM_XBUTTONDOWN*/) ? INPUT_PRESS : INPUT_RELEASE;
                for (i=0;i<=7;i++) { if (win->mouseButtons[i]==INPUT_PRESS) break; }
                if (i>7) {SetCapture(hWnd);} InputMouseClick(win,button,action);
                for (i=0;i<=7;i++) { if (win->mouseButtons[i]==INPUT_PRESS) break; }
                if (i>7) {ReleaseCapture();} if (uMsg == 0x020B/*WM_XBUTTONDOWN*/ || uMsg == 0x020C/*WM_XBUTTONUP*/) return 1;
                return 0; }
            case 0x0200/*WM_MOUSEMOVE*/: {                
                const int x=((int)(i16)(lParam & 0xFFFF)), y=((int)(i16)(lParam >> 16));
                if (win->cursorMode==0x00034003/*CURSOR_DISABLED*/) { const int dx=x-win->win32.lastCurX,dy=y-win->win32.lastCurY; if (WinSys.win32.disabledCursorWindow!=win) {break;} InputCursorPos(win,win->virtualCursorPosX+dx,win->virtualCursorPosY+dy); }
                win->win32.lastCurX=x; win->win32.lastCurY=y;
                return 0; }
            case 0x02A3/*WM_MOUSELEAVE*/: { win->win32.cursorTracked=0; return 0; }
            case 0x020A/*WM_MOUSEWHEEL*/: { Sys_Input.scrollDelta += (i16)HIWORD(wParam)/(double)120; return 0; }
            case 0x0005/*WM_SIZE*/: if (wParam == 1) {World.paused = true;} return 0;
            case 0x0003/*WM_MOVE*/: if (WinSys.win32.capturedCursorWindow==win) {captureCursor(win);} return 0;
            case 0x0086/*WM_NCACTIVATE*/: case 0x0085/*WM_NCPAINT*/: { if (!win->decorated) return 1; break; }
            case 0x0020/*WM_SETCURSOR*/: { if (LOWORD(lParam)==1) { updateCursorImage(win); return 1; } break; }
            case 0x0084/*WM_NCHITTEST*/: ;i64 hit = DefWindowProcW(hWnd,uMsg,wParam,lParam); if (hit >= 10 && hit <= 17) { return 1; } return hit;
        }
        return DefWindowProcW(hWnd,uMsg,wParam,lParam);
    }

    void SetWindowIcon(const WinSysIcon* image) { HICON hIcon = createIcon(image,0,0, 1); SendMessageW(((WinSyswindow*)window)->win32.handle,0x0080,1,(i64)hIcon); SendMessageW(((WinSyswindow*)window)->win32.handle,0x0080,0,(i64)hIcon); }
    void GetWindowPos(WinSyswindow* win, int* x, int* y) { POINT pos={0,0}; ClientToScreen(win->win32.handle,&pos); *x=pos.x; *y=pos.y; }
    void GetWindowSize(WinSyswindow* win, int* w, int* h) { RECT area; GetClientRect(win->win32.handle,&area); *w=area.right; *h=area.bottom; }
    void SetWindowSize(WinSyswindow* win, int w, int h) { RECT rect={0,0,w,h}; AdjustWindowRectEx(&rect,getWindowStyle(win),0,0); SetWindowPos(win->win32.handle,(HWND)0,0,0,rect.right-rect.left,rect.bottom-rect.top,0x0010|0x0200|0x0002|0x0004); }
    void SetWindowMonitor(WinSyswindow* w, int x, int y, int wd, int h) { RECT r = {x,y,x+wd,y+h}; u32 s=GetWindowLongW(w->win32.handle,-16); u32 f=0x0010|0x0100; if (w->decorated) {s&=~0x80000000,s|=getWindowStyle(w),SetWindowLongW(w->win32.handle,-16,s),f|=0x0020;} AdjustWindowRectEx(&r,getWindowStyle(w),0,0); SetWindowPos(w->win32.handle,(HWND)-2,r.left,r.top,r.right-r.left,r.bottom-r.top,f); }
    void SetWindowDecorated(WinSyswindow* win,i32 enabled) {
        (void)enabled; RECT rect; u32 style=GetWindowLongW(win->win32.handle,-16);
        style &= ~(0x00C00000/*WS_CAPTION*/ | 0x00080000/*WS_SYSMENU*/ | 0x00040000/*WS_THICKFRAME*/ | 0x00020000/*WS_MINIMIZEBOX*/ | 0x00010000/*WS_MAXIMIZEBOX*/ | 0x80000000/*WS_POPUP*/); style |= getWindowStyle(win);
        GetClientRect(win->win32.handle,&rect); AdjustWindowRectEx(&rect,style,0,0); ClientToScreen(win->win32.handle,(POINT*)&rect.left); ClientToScreen(win->win32.handle,(POINT*)&rect.right); SetWindowLongW(win->win32.handle,-16,style);
        SetWindowPos(win->win32.handle,(HWND)0,rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top,0x0020|0x0010|0x0004);
    }
    
    void PollEvents() {
        HWND handle = GetActiveWindow();
        WinSyswindow* win = GetPropW(handle,L"WinSys"); MSG msg;
        while (PeekMessageW(&msg,NULL,0,0,0x0001)) { if (msg.message==0x0012/*WM_QUIT*/) { OS_Exit(0); } else { TranslateMessage(&msg); DispatchMessageW(&msg); } }
        const int keys[4][2]={{0xA0/*VK_LSHIFT*/,KEY_LEFT_SHIFT},{0xA1/*VK_RSHIFT*/,KEY_RIGHT_SHIFT},{0x5B/*VK_LWIN*/,KEY_LEFT_SUPER},{0x5C/*VK_RWIN*/,KEY_RIGHT_SUPER}};
        for (int i=0;i<4;i++) { const int vk=keys[i][0],key=keys[i][1]; if ((GetKeyState(vk)&0x8000)||win->keys[key]!=INPUT_PRESS) {continue;} InputKey(win,key,INPUT_RELEASE); }
        win = WinSys.win32.disabledCursorWindow;
        if (win) { int width,height; GetWindowSize(win,&width,&height); if (win->win32.lastCurX != width/2 || win->win32.lastCurY != height/2) {SetCurV(win,width/2,height/2);} }
    }

    WinSysproc PlatformGetModuleSymbol(void* module, const char* name) { return (WinSysproc)GetProcAddress((HMODULE)module,name); }
    typedef struct {u16 index; i32 vkey;} WinKeyRemap;
    static const WinKeyRemap winkeyRemapTable[] = {{0x00B,KEY_0},{0x002,KEY_1},{0x003,KEY_2},{0x004,KEY_3},{0x005,KEY_4},{0x006,KEY_5},{0x007,KEY_6},{0x008,KEY_7},{0x009,KEY_8},{0x00A,KEY_9},{0x01E,KEY_A},{0x030,KEY_B},{0x02E,KEY_C},{0x020,KEY_D},{0x012,KEY_E},{0x021,KEY_F},{0x022,KEY_G},{0x023,KEY_H},{0x017,KEY_I},{0x024,KEY_J},{0x025,KEY_K},{0x026,KEY_L},{0x032,KEY_M},{0x031,KEY_N},{0x018,KEY_O},{0x019,KEY_P},
                                                   {0x010,KEY_Q},{0x013,KEY_R},{0x01F,KEY_S},{0x014,KEY_T},{0x016,KEY_U},{0x02F,KEY_V},{0x011,KEY_W},{0x02D,KEY_X},{0x015,KEY_Y},{0x02C,KEY_Z},{0x028,KEY_APOSTROPHE},{0x02B,KEY_BACKSLASH},{0x033,KEY_COMMA},{0x00D,KEY_EQUAL},{0x029,KEY_GRAVE_ACCENT},{0x01A,KEY_LEFT_BRACKET},{0x00C,KEY_MINUS},{0x034,KEY_PERIOD},{0x01B,KEY_RIGHT_BRACKET},{0x027,KEY_SEMICOLON},{0x035,KEY_SLASH},
                                                   {0x00E,KEY_BACKSPACE},{0x153,KEY_DELETE},{0x14F,KEY_END},{0x01C,KEY_ENTER},{0x001,KEY_ESCAPE},{0x147,KEY_HOME},{0x152,KEY_INSERT},{0x15D,KEY_MENU},{0x151,KEY_PAGE_DOWN},{0x149,KEY_PAGE_UP},{0x045,KEY_PAUSE},{0x039,KEY_SPACE},{0x00F,KEY_TAB},{0x03A,KEY_CAPS_LOCK},{0x145,KEY_NUM_LOCK},{0x046,KEY_SCROLL_LOCK},{0x03B,KEY_F1},{0x03C,KEY_F2},{0x03D,KEY_F3},{0x03E,KEY_F4},
                                                   {0x03F,KEY_F5},{0x040,KEY_F6},{0x041,KEY_F7},{0x042,KEY_F8},{0x043,KEY_F9},{0x044,KEY_F10},{0x057,KEY_F11},{0x058,KEY_F12},{0x038,KEY_LEFT_ALT},{0x01D,KEY_LEFT_CONTROL},{0x02A,KEY_LEFT_SHIFT},{0x15B,KEY_LEFT_SUPER},{0x137,KEY_PRINT_SCREEN},{0x138,KEY_RIGHT_ALT},{0x11D,KEY_RIGHT_CONTROL},{0x036,KEY_RIGHT_SHIFT},{0x15C,KEY_RIGHT_SUPER},{0x150,KEY_DOWN},{0x14B,KEY_LEFT},{0x14D,KEY_RIGHT},
                                                   {0x148,KEY_UP},{0x052,KEY_KP_0},{0x04F,KEY_KP_1},{0x050,KEY_KP_2},{0x051,KEY_KP_3},{0x04B,KEY_KP_4},{0x04C,KEY_KP_5},{0x04D,KEY_KP_6},{0x047,KEY_KP_7},{0x048,KEY_KP_8},{0x049,KEY_KP_9},{0x04E,KEY_KP_ADD},{0x053,KEY_KP_DECIMAL},{0x135,KEY_KP_DIVIDE},{0x11C,KEY_KP_ENTER},{0x059,KEY_KP_EQUAL},{0x037,KEY_KP_MULTIPLY},{0x04A,KEY_KP_SUBTRACT}};
    static void createKeyTables() {
        mset(WinSys.win32.keycodes,-1,sizeof(WinSys.win32.keycodes)); mset(WinSys.win32.scancodes,-1,sizeof(WinSys.win32.scancodes));
        for (size_t i=0;i<sizeof(winkeyRemapTable)/sizeof(winkeyRemapTable[0]);++i) WinSys.win32.keycodes[winkeyRemapTable[i].index] = winkeyRemapTable[i].vkey;
        for (int scancode=0;scancode<512;scancode++) { if (WinSys.win32.keycodes[scancode] > 0) {WinSys.win32.scancodes[WinSys.win32.keycodes[scancode]] = scancode;} }
    }

    u16* CreateWideStringFromUTF8Win32(const char* src) { u16* target; int count = MultiByteToWideChar(65001,0,(char*)src,-1,NULL,0); target = OS_Calloc(count,sizeof(u16)); MultiByteToWideChar(65001,0,(char*)src,-1,target,count); return target; }
    char* CreateUTF8FromWideStringWin32(const u16* src, int* size) { *size = WideCharToMultiByte(65001,0,(u16*)src,-1,NULL,0,NULL,NULL); char* target = OS_Calloc(*size,1); WideCharToMultiByte(65001,0,(u16*)src,-1,target,*size,NULL,NULL); return target; }
    i32 IsWindowsVersionOrGreaterWin32(u16 major, u16 minor, u16 sp) { OSVERSIONINFOEXW osvi={0}; osvi.dwOSVersionInfoSize=sizeof(osvi), osvi.dwMajorVersion=major, osvi.dwMinorVersion=minor, osvi.wServicePackMajor=sp; u32 mask=0x0000002|0x0000001|0x0000020; u64 cond=VerSetConditionMask(VerSetConditionMask(VerSetConditionMask(0,0x0000002,3),0x0000001,3),0x0000020,3); return WinSys.win32.ntdll.RtlVerifyVersionInfo(&osvi,mask,cond)==0; }
    static void closeJoystick(WinSysjoystick* js) { JoystickConnection(js,0x00040002/*disconnected*/); FreeJoystick(js); }
    void WinSysDetectJoystickConnectionWin32() {
        if (WinSys.win32.xinput.instance) {
            for (u32 index=0;index<4;index++) {
                int jid; XINPUT_CAPABILITIES xic; WinSysjoystick* js;
                for (jid = 0;  jid <= JOYSTICK_LAST;  jid++) { if (WinSys.joysticks[jid].connected && WinSys.joysticks[jid].win32.index == index) {break;} }
                if (jid <= JOYSTICK_LAST || WinSys.win32.xinput.GetCapabilities(index,0,&xic) != 0) continue;
                char guid[33]; sFormat(guid,sizeof(guid),"78696e707574%02x000000000000000000",xic.SubType & 0xff); js = WinSysAllocJoystick("Gamepad",guid,6,10,1); if (!js) continue;
                js->win32.index = index; JoystickConnection(js,0x00040001/*connected*/);
            }
        }
    }

    i32 InitJoysticks() { WinSysDetectJoystickConnectionWin32(); return 1; }
    i32 PollJoystick(WinSysjoystick* js) {
        u32 result; XINPUT_STATE xis;
        const u16 buttons[14] = {0x0001/*XINPUT_GAMEPAD_DPAD_UP*/,0x0002/*XINPUT_GAMEPAD_DPAD_DOWN*/,0x0008/*XINPUT_GAMEPAD_DPAD_RIGHT*/,0x0004/*XINPUT_GAMEPAD_DPAD_LEFT*/,0x1000/*XINPUT_GAMEPAD_A*/,0x2000/*XINPUT_GAMEPAD_B*/,0x4000/*XINPUT_GAMEPAD_X*/,0x8000/*XINPUT_GAMEPAD_Y*/,0x0100/*XINPUT_GAMEPAD_LEFT_SHOULDER*/,0x0200/*XINPUT_GAMEPAD_RIGHT_SHOULDER*/,0x0020/*XINPUT_GAMEPAD_BACK*/,0x0010/*XINPUT_GAMEPAD_START*/,0x0040/*XINPUT_GAMEPAD_LEFT_THUMB*/,0x0080/*XINPUT_GAMEPAD_RIGHT_THUMB*/};
        result = WinSys.win32.xinput.GetState(js->win32.index, &xis);
        if (result != 0) { if (result == 1167/*not connected*/) {closeJoystick(js);} return 0; }
        const i16 axis_vals[] = {xis.Gamepad.sThumbLX,-xis.Gamepad.sThumbLY,xis.Gamepad.sThumbRX,-xis.Gamepad.sThumbRY};
        for (int i=0;i<4;++i) InputJoystickAxis(js,i,(axis_vals[i] + 0.5f) / 32767.5f);
        InputJoystickAxis(js,4,xis.Gamepad.bLeftTrigger / 127.5f - 1.f); InputJoystickAxis(js,5,xis.Gamepad.bRightTrigger / 127.5f - 1.f);
        for (int i=0;i<10;++i) { const char value = (xis.Gamepad.wButtons & buttons[i]) ? 1 : 0; InputJoystickButton(js,i,value); }
        int dpad = ((const int[]){0,1,2,3,4,0,0,0,8,0,0,0,0,0,0,0})[xis.Gamepad.wButtons & 0xF];
        if ((dpad & JOYHAT_RIGHT) && (dpad & JOYHAT_LEFT)) dpad &= ~(JOYHAT_RIGHT | JOYHAT_LEFT);
        if ((dpad & JOYHAT_UP) && (dpad & JOYHAT_DOWN)) dpad &= ~(JOYHAT_UP | JOYHAT_DOWN);
        InputJoystickHat(js,0,dpad);
        return  1;
    }
    
    void WinSysDetectJoystickDisconnectionWin32() { for(int jid=0;jid<=JOYSTICK_LAST;jid++){WinSysjoystick* js = WinSys.joysticks + jid; if(js->connected){PollJoystick(js);}} }
    static i32 __stdcall monitorCallback(HMONITOR h, HDC c, RECT* r, i64 d) { MONITORINFOEXW mi; (void)c; (void)r; mset(&mi,0,sizeof(mi)); mi.cbSize = sizeof(mi); if (GetMonitorInfoW(h,(MONITORINFO*)&mi)) { WinSysmonitor* monitor = (WinSysmonitor*)d; if (wcscmp(mi.szDevice, monitor->win32.adapterName) == 0) {monitor->win32.handle = h;} } return 1; }
    static WinSysmonitor* createMonitor(DISPLAY_DEVICEW* adapter, DISPLAY_DEVICEW* display) {
        WinSysmonitor* monitor; int widthMM,heightMM,nameSize=0; HDC dc; RECT rect;
        char* name = CreateUTF8FromWideStringWin32(display ? display->DeviceString : adapter->DeviceString,&nameSize);
        DEVMODEW dm; mset(&dm,0,sizeof(dm)); dm.dmSize = sizeof(dm);
        EnumDisplaySettingsW(adapter->DeviceName,0xFFFFFFFFU,&dm);
        dc = CreateDCW(L"DISPLAY",adapter->DeviceName,NULL,NULL);
        if (IsWindowsVersionOrGreaterWin32(HIBYTE(0x0603),LOBYTE(0x0603),0)) { widthMM  = GetDeviceCaps(dc,4); heightMM = GetDeviceCaps(dc,6); } // Is Windows 8.10 or greater
        else { widthMM  = (int) (dm.dmPelsWidth * 25.4f / GetDeviceCaps(dc,88)); heightMM = (int)(dm.dmPelsHeight * 25.4f / GetDeviceCaps(dc,90)); }
        DeleteDC(dc); monitor = AllocMonitor(name,widthMM,heightMM); OS_Free(name,nameSize);
        if (adapter->StateFlags & 0x08000000/*DISPLAY_DEVICE_MODESPRUNED*/) monitor->win32.modesPruned =  1;
        wcscpy(monitor->win32.adapterName, adapter->DeviceName);
        if (display) wcscpy(monitor->win32.displayName,display->DeviceName);
        rect.left=dm.dmPosition.x; rect.top=dm.dmPosition.y; rect.right=dm.dmPosition.x + dm.dmPelsWidth; rect.bottom=dm.dmPosition.y + dm.dmPelsHeight;
        EnumDisplayMonitors(NULL,&rect,monitorCallback,(i64)monitor);
        return monitor;
    }

    void WinSysPollMonitorsWin32() {
        int i, disconnectedCount = WinSys.monitorCount; WinSysmonitor** disconnected = NULL; u32 adapterIndex,displayIndex; DISPLAY_DEVICEW adapter, display; WinSysmonitor* monitor;
        if (disconnectedCount) { disconnected = OS_Calloc(WinSys.monitorCount,sizeof(WinSysmonitor*)); mcpy(disconnected,WinSys.monitors,WinSys.monitorCount * sizeof(WinSysmonitor*)); }
        for (adapterIndex = 0;;adapterIndex++) {
            int type = 1; mset(&adapter,0,sizeof(adapter)); adapter.cb = sizeof(adapter); if (!EnumDisplayDevicesW(NULL,adapterIndex,&adapter,0)) break;
            if (!(adapter.StateFlags&1)) continue;
            if (adapter.StateFlags & 0x00000004/*DISPLAY_DEVICE_PRIMARY_DEVICE*/) type = 0;
            for (displayIndex=0;;++displayIndex) {
                mset(&display,0,sizeof(display)); display.cb = sizeof(display); if (!EnumDisplayDevicesW(adapter.DeviceName,displayIndex,&display,0)) break;
                if (!(display.StateFlags&1)) continue;
                for (i=0;i<disconnectedCount;++i) { if(disconnected[i] && wcscmp(disconnected[i]->win32.displayName,display.DeviceName) == 0){disconnected[i] = NULL; EnumDisplayMonitors(NULL,NULL,monitorCallback,(i64)WinSys.monitors[i]); break;} }
                if (i < disconnectedCount) continue;
                monitor = createMonitor(&adapter,&display); if (!monitor) { OS_Free(disconnected,WinSys.monitorCount*sizeof(WinSysmonitor*)); return; }
                InputMonitor(monitor,0x00040001/*connected*/,type); type = 1;
            }
            if (displayIndex == 0) {
                for (i=0;i<disconnectedCount;++i) { if (disconnected[i] && wcscmp(disconnected[i]->win32.adapterName,adapter.DeviceName) == 0) {disconnected[i]=NULL; break;} }
                if (i < disconnectedCount) continue;
                monitor = createMonitor(&adapter,NULL); if (!monitor) { OS_Free(disconnected,WinSys.monitorCount*sizeof(WinSysmonitor*)); return; }
                InputMonitor(monitor,0x00040001/*connected*/,type);
            }
        }
        for (i=0;i<disconnectedCount;++i) { if (disconnected[i]) {InputMonitor(disconnected[i],0x00040002/*disconnected*/,0);} }
        if (disconnected) OS_Free(disconnected,WinSys.monitorCount*sizeof(WinSysmonitor*));
    }
    
    static i64 __stdcall helperWindowProc(HWND hWnd, u32 uMsg, u64 wParam, i64 lParam) {
        switch (uMsg) {
            case 0x007E/*WM_DISPLAYCHANGE*/: WinSysPollMonitorsWin32(); break;
            case 0x0219/*WM_DEVICECHANGE*/: if (!WinSys.joysInited) break;
                if (wParam == 0x8000/*DBT_DEVICEARRIVAL*/ || wParam == 0x8004/*DBT_DEVICEREMOVECOMPLETE*/) {
                    DEV_BROADCAST_HDR* dbh = (DEV_BROADCAST_HDR*) lParam;
                    if (dbh && dbh->dbch_devicetype == 0x0005/*DBT_DEVTYP_DEVICEINTERFACE*/ && wParam == 0x8000/*DBT_DEVICEARRIVAL*/)           WinSysDetectJoystickConnectionWin32();
                    if (dbh && dbh->dbch_devicetype == 0x0005/*DBT_DEVTYP_DEVICEINTERFACE*/ && wParam == 0x8004/*DBT_DEVICEREMOVECOMPLETE*/) WinSysDetectJoystickDisconnectionWin32();
                }
                break;
        }
        return DefWindowProcW(hWnd,uMsg,wParam,lParam);
    }

    void GetMonitorPos(WinSysmonitor* monitor, int* x, int* y) { DEVMODEW dm; mset(&dm,0,sizeof(dm)); dm.dmSize = sizeof(dm); EnumDisplaySettingsExW(monitor->win32.adapterName,0xFFFFFFFFU,&dm,0x00000004); *x = dm.dmPosition.x; *y = dm.dmPosition.y; }
    void GetMonitorWorkarea(WinSysmonitor* monitor, int* x, int* y, int* width, int* height) { MONITORINFO mi = {0}; mi.cbSize = sizeof(mi); GetMonitorInfoW(monitor->win32.handle, &mi); *x = mi.rcWork.left; *y = mi.rcWork.top; *width = mi.rcWork.right - mi.rcWork.left; *height = mi.rcWork.bottom - mi.rcWork.top; }
    void GetVideoMode(WinSysmonitor* monitor, vidmode* mode) { DEVMODEW dm; mset(&dm,0,sizeof(dm)); dm.dmSize = sizeof(dm); EnumDisplaySettingsW(monitor->win32.adapterName,0xFFFFFFFFU,&dm); mode->width=dm.dmPelsWidth; mode->height=dm.dmPelsHeight; mode->refreshRate=dm.dmDisplayFrequency; }
    static int choosePixelFormatWGL(WinSyswindow* win) {
        int attribs[24],values[24],attribCount=0,i,pixelFormat,nativeCount,usableCount=0;
        const int query = 0x2000/*num pixel formats*/; wglGetPixelFormatAttribivARB(win->context.wgl.dc,1,0,1,&query,&nativeCount);
        attribs[attribCount++] = 0x2010/*support opengl*/; attribs[attribCount++] = 0x2001/*draw to window*/; attribs[attribCount++] = 0x2013/*pixel type*/; attribs[attribCount++] = 0x2003/*accelaration*/;
        attribs[attribCount++] = 0x2011/*double buffer*/; attribs[attribCount++] = 0x2015/*r bits*/; attribs[attribCount++] = 0x2017/*g bits*/;
        attribs[attribCount++] = 0x2019/*b bits*/; attribs[attribCount++] = 0x201b/*a bits*/; attribs[attribCount++] = 0x2022/*depth bits*/; attribs[attribCount++] = 0x2023/*stencil bits*/;
        WinSysfbconfig* usableConfigs = OS_Calloc(nativeCount,sizeof(WinSysfbconfig));
        for (i = 0; i < nativeCount; i++) {
            WinSysfbconfig* u = usableConfigs + usableCount; pixelFormat = i + 1; wglGetPixelFormatAttribivARB(win->context.wgl.dc,pixelFormat,0,attribCount,attribs,values);
            if (values[0] == 0 || values[1] == 0/* support OpenGL + draw to window */ || values[2] != 0x202b/*type rgba*/ || values[3] == 0x2025/*no accel*/ || values[4] !=  1) continue;
            u->redBits=values[5]; u->greenBits=values[6]; u->blueBits=values[7]; u->alphaBits=values[8]; u->depthBits=values[9]; u->stencilBits=values[10]; u->handle=pixelFormat; usableCount++;
        }
        const WinSysfbconfig* closest = ChooseFBConfig(usableConfigs,usableCount);
        pixelFormat = (int)closest->handle; OS_Free(usableConfigs,nativeCount * sizeof(WinSysfbconfig));
        return pixelFormat;
    }

    static void makeContextCurrentWGL(WinSyswindow* win) { wglMakeCurrent(win->context.wgl.dc,win->context.wgl.handle); }
    static void swapBuffersWGL(WinSyswindow* win) { if (!IsWindowsVersionOrGreaterWin32(HIBYTE(0x0602),LOBYTE(0x0602),0)) { i32 enabled = 0; if ((i32)(WinSys.win32.dwmapi.IsCompositionEnabled(&enabled) >= 0) && enabled) { int count = vabs(win->context.wgl.interval); while (count--) {WinSys.win32.dwmapi.Flush();} } }/*>=Win8.0*/ SwapBuffers(win->context.wgl.dc); }
    static void swapIntervalWGL(int interval) { ((WinSyswindow*)window)->context.wgl.interval = interval; if (!IsWindowsVersionOrGreaterWin32(HIBYTE(0x0602),LOBYTE(0x0602),0)) { i32 enabled = 0; if ((i32)(WinSys.win32.dwmapi.IsCompositionEnabled(&enabled) >= 0) && enabled) interval = 0; }/*>=Win8.0*/ wglSwapIntervalEXT(interval); }
    static WinSysglproc getProcAddressWGL(const char* procname) { const WinSysglproc proc = (WinSysglproc)wglGetProcAddress(procname); if (proc) {return proc;} return (WinSysglproc)PlatformGetModuleSymbol(WinSys.wgl.instance,procname); }
    void SetWindowPosition(WinSyswindow* handle, int x, int y) { WinSyswindow* win = (WinSyswindow*)handle; RECT rect = {x,y,x,y}; AdjustWindowRectEx(&rect,getWindowStyle(win),0,0x00040000/*WS_EX_APPWINDOW*/); SetWindowPos(win->win32.handle,((HWND)0),rect.left,rect.top,0,0,0x0010|0x0200|0x0001|0x0004); }
    WinSyswindow* VCreateWindow(int width, int height, char* title) {
        WinSyswindow* win = OS_Calloc(1,sizeof(WinSyswindow)); win->videoMode = (vidmode){width,height,8,8,8,-1}; win->decorated = win->doublebuffer = 1; win->cursorMode = 0x00034003/*disabled*/;
        u32 style = getWindowStyle(win);
        WNDCLASSEXW wc= (WNDCLASSEXW){sizeof(wc),0x23/*Redraws + Owns Device Context*/,windowProc,0,0,WinSys.win32.instance,NULL,NULL,NULL,NULL,L"Voxen",NULL};
        WinSys.win32.mainWindowClass=RegisterClassExW(&wc);
        RECT rect={0,0,width,height}; //AdjustWindowRectEx(&rect,style,0,0);
        int frameX,frameY; frameX=frameY=0x80000000;
        int frameWidth=rect.right-rect.left, frameHeight=rect.bottom-rect.top;
        u16* wideTitle=CreateWideStringFromUTF8Win32(title);
        win->win32.handle=CreateWindowExW(0,(u16*)MAKEINTATOM(WinSys.win32.mainWindowClass),wideTitle,style,frameX,frameY,frameWidth,frameHeight,NULL,NULL,WinSys.win32.instance,(void*)NULL);
        SetPropW(win->win32.handle,L"WinSys",win);
        win->win32.keymenu=0; WINDOWPLACEMENT wp={0}; wp.length=sizeof(wp); AdjustWindowRectEx(&rect,style,0,0);
        GetWindowPlacement(win->win32.handle,&wp);
        OffsetRect(&rect,wp.rcNormalPosition.left-rect.left,wp.rcNormalPosition.top-rect.top);
        wp.rcNormalPosition=rect; wp.showCmd=0; 
        SetWindowPlacement(win->win32.handle,&wp);
        GetWindowSize(win,&win->win32.width,&win->win32.height);
        PIXELFORMATDESCRIPTOR pfd; HGLRC prc,rc; HDC pdc,dc;
        WinSys.wgl.instance = LoadLibraryA("opengl32.dll");
        wglCreateContext = (PFN_CC)PlatformGetModuleSymbol(WinSys.wgl.instance,"wglCreateContext");
        wglGetProcAddress = (PFN_wglGetProcAddress)PlatformGetModuleSymbol(WinSys.wgl.instance,"wglGetProcAddress");
        wglGetCurrentDC = (PFN_wglGetCurrentDC)PlatformGetModuleSymbol(WinSys.wgl.instance,"wglGetCurrentDC");
        wglGetCurrentContext = (PFN_wglGetCurrentContext)PlatformGetModuleSymbol(WinSys.wgl.instance,"wglGetCurrentContext");
        wglMakeCurrent = (PFN_wglMakeCurrent)PlatformGetModuleSymbol(WinSys.wgl.instance,"wglMakeCurrent");
        dc = GetDC(WinSys.win32.helperWindowHandle);
        mset(&pfd,0,sizeof(pfd)); pfd.nSize = sizeof(pfd); pfd.dwFlags=0x25; SetPixelFormat(dc,ChoosePixelFormat(dc,&pfd),&pfd);
        rc = wglCreateContext(dc); pdc=wglGetCurrentDC(); prc=wglGetCurrentContext(); wglMakeCurrent(dc,rc);
        wglCreateContextAttribsARB = (FP_CCAA)wglGetProcAddress("wglCreateContextAttribsARB");
        wglSwapIntervalEXT = (PFN_SWE)wglGetProcAddress("wglSwapIntervalEXT");
        wglGetPixelFormatAttribivARB = (PFN_GPFAIVA)wglGetProcAddress("wglGetPixelFormatAttribivARB");
        wglMakeCurrent(pdc,prc);
        int attribs[40],pixelFormat; PIXELFORMATDESCRIPTOR pfd2;
        win->context.wgl.dc = GetDC(win->win32.handle);
        pixelFormat = choosePixelFormatWGL(win);
        DescribePixelFormat(win->context.wgl.dc,pixelFormat,sizeof(pfd2),&pfd2); SetPixelFormat(win->context.wgl.dc,pixelFormat,&pfd2);
        int index=0; attribs[index++] = 0x2091/*major*/; attribs[index++] = 4;/*OpenGL 4.3*/ attribs[index++] = 0x2092/*minor*/; attribs[index++] = 3; attribs[index++] = 0x9126/*context profile mask*/; attribs[index++] = 1; attribs[index++] = 0; attribs[index++] = 0;
        win->context.wgl.handle = wglCreateContextAttribsARB(win->context.wgl.dc,NULL,attribs);
        win->context.makeCurrent = makeContextCurrentWGL; win->context.swapBuffers = swapBuffersWGL;
        win->context.swapInterval = swapIntervalWGL; win->context.getProcAddress = getProcAddressWGL;
        int showCommand = 8; ShowWindow(win->win32.handle,showCommand); BringWindowToTop(win->win32.handle); SetForegroundWindow(win->win32.handle); SetFocus(win->win32.handle);
        return (WinSyswindow*)win;
    }
#else // LINUX
    typedef u8 KeyCode; typedef u16 Rotation,SubpixelOrder,Connection; typedef i32 Bool; typedef int Status; typedef u64 XID,Mask,Atom,VisualID,Time,KeySym; typedef char *XPointer; typedef u32 XcursorUInt;
    typedef struct _XcursorImage { XcursorUInt version; XcursorUInt size,width,height,xhot,yhot; XcursorUInt delay; XcursorUInt *pixels; } XcursorImage;
    typedef struct { i64 flags; int x,y, width,height,min_width,min_height,max_width,max_height,width_inc,height_inc; struct {int x; int y;} min_aspect,max_aspect; int base_width, base_height; int win_gravity; } XSizeHints;
    typedef XID Window,Drawable,Font,Pixmap,Cursor,Colormap; typedef struct _XExtData { int number; struct _XExtData *next; int (*free_private)(struct _XExtData*); XPointer private_data; } XExtData;
    typedef struct { int extension, major_opcode, first_event, first_error; } XExtCodes; typedef struct { int depth, bits_per_pixel, scanline_pad; } XPixmapFormatValues;
    typedef struct _XGC *GC; typedef struct { XExtData *ext_data; VisualID visualid; int class; u64 red_mask, green_mask, blue_mask; int bits_per_rgb; int map_entries;} Visual; 
    typedef struct { int depth,nvisuals; Visual *visuals; } Depth;
    typedef struct { XExtData *ext_data; struct _XDisplay *display; Window root; int width,height,mwidth,mheight,ndepths; Depth *depths; int root_depth; Visual *root_visual; GC default_gc; Colormap cmap; u64 white_pixel, black_pixel; int max_maps, min_maps, backing_store; int save_unders; i64 root_input_mask; } Screen;
    typedef struct { XExtData *ext_data; int depth, bits_per_pixel, scanline_pad; } ScreenFormat;
    typedef struct { Pixmap background_pixmap; u64 background_pixel; Pixmap border_pixmap; u64 border_pixel; int bit_gravity, win_gravity, backing_store; u64 backing_planes, backing_pixel; int save_under; i64 event_mask, do_not_propagate_mask; int override_redirect; Colormap colormap; Cursor cursor; } XSetWindowAttributes;
    typedef struct { int x,y,width,height,border_width,depth; Visual *visual; Window root; int class,bit_gravity,win_gravity,backing_store; u64 backing_planes,backing_pixel; int save_under; Colormap colormap; int map_installed,map_state; i64 all_event_masks,your_event_mask,do_not_propagate_mask; i32 override_redirect; Screen *screen; } XWindowAttributes;
    typedef struct _XDisplay Display; typedef struct { XExtData *ext_data; struct _XPrivate *private1; int fd, private2, proto_major_version, proto_minor_version; char *vendor; XID private3, private4, private5; int private6; XID (*resource_alloc)(struct _XDisplay*); int byte_order, bitmap_unit, bitmap_pad, bitmap_bit_order, nformats; ScreenFormat *pixmap_format; int private8; struct _XPrivate *private9, *private10; int qlen; u64 last_request_read,request; XPointer private11,private12,private13,private14; unsigned max_request_size; struct _XrmHashBucketRec *db; int (*private15)(struct _XDisplay*); char *display_name; i32 default_screen, nscreens; Screen *screens; u64 motion_buffer, private16; i32 min_keycode,max_keycode; XPointer private17,private18; i32 private19; char *xdefaults; } *_XPrivDisplay;
    typedef struct { int a; u64 b; int c; void *d; u64 e,f,g,h; int i,j,k,l; u32 m,keycode; int n; } XKeyEvent; // Don't care the names of the unused fields here, so just stuff alphabet in there
    typedef struct { int a; u64 b; int c; Display *d; Window e,f,g; Time h; int i,j,k,l; u32 m,button; int n; } XButtonEvent;  typedef struct { int a; u64 b; int c; Display *d; Window e,f,g; Time h; int x,y,i,j; u32 k; char l; int m; } XMotionEvent;
    typedef struct { int a; u64 b; int c; Display *d; Window e,f,g; Time h; int x,y,i,j,k,l; int m,n; u32 o; } XCrossingEvent; typedef struct { int a; u64 b; i32 c; Display *d; Window e; int mode,f; } XFocusChangeEvent;
    typedef struct { int a; u64 b; i32 c; Display *d; Window e,f,parent; int g,h,i; } XReparentEvent;                          typedef struct { int a; u64 b; i32 c; Display *d; Window e,f; int x,y,width,height,g; Window h; int i; } XConfigureEvent;
    typedef struct { int a; u64 b; int c; Display *d; Window window; Atom message_type; int format; union { char b[20]; short s[10]; long l[5]; } data; } XClientMessageEvent;
    typedef struct { int a; u64 b; int send_event; Display *c; Window window; } XAnyEvent;
    typedef struct { int type; u64 serial; int send_event; Display *display; int extension, evtype; u32 cookie; void *data; } XGenericEventCookie;
    typedef union _XEvent { int type; XAnyEvent xany; XKeyEvent xkey; XButtonEvent xbutton; XMotionEvent xmotion; XCrossingEvent xcrossing; XFocusChangeEvent xfocus; u8 p0[528]; XReparentEvent xreparent; XConfigureEvent xcfg; u8 p1[648]; XClientMessageEvent xclient; u8 p2[224]; } XEvent;
    typedef struct _XIC *XIC; typedef struct { Visual *visual; VisualID visualid; int screen,depth; int class; u64 red_mask,green_mask,blue_mask; int colormap_size,bits_per_rgb; } XVisualInfo;
    typedef int XContext; typedef XID RROutput,RRCrtc,RRMode; typedef u64 XRRModeFlags;
    typedef struct { RRMode id; u32 width,height; u64 dotClock; u32 hSyncStart,hSyncEnd,hTotal,hSkew,vSyncStart,vSyncEnd,vTotal; char *name; u32 nameLength; XRRModeFlags modeFlags; } XRRModeInfo;
    typedef struct { Time timestamp; Time configTimestamp; int ncrtc; RRCrtc *crtcs; int noutput; RROutput *outputs; int nmode; XRRModeInfo *modes; } XRRScreenResources;
    typedef struct { Time timestamp; RRCrtc crtc; char *name; int nameLen; u64 mm_width,mm_height; Connection connection; SubpixelOrder subpixel_order; i32 ncrtc; RRCrtc *crtcs; i32 nclone; RROutput *clones; i32 nmode,npreferred; RRMode *modes; } XRROutputInfo;
    typedef struct { Time timestamp; i32 x,y; u32 width,height; RRMode mode; Rotation rotation; i32 noutput; RROutput *outputs; Rotation rotations; i32 npossible; RROutput *possible; } XRRCrtcInfo; typedef XID GLXWindow,GLXDrawable; typedef struct __GLXFBConfig* GLXFBConfig; typedef struct __GLXcontext* GLXContext;
    typedef void(*__GLXextproc)();                               typedef XSizeHints*(*PFN_XAllocSizeHints)();                       typedef int(*PFN_XChangeProperty)(Display*,Window,Atom,Atom,int,int,const u8*,int);         typedef void(*PFN_XCID)(XcursorImage*);                         typedef void(*PFN_XRRFreeOutputInfo)(XRROutputInfo*);                    typedef Colormap(*PFN_XCreateColormap)(Display*,Window,Visual*,int);
    typedef int(*PFN_XDefineCursor)(Display*,Window,Cursor);     typedef int(*PFN_XDeleteProperty)(Display*,Window,Atom);           typedef int(*PFN_XDisplayKeycodes)(Display*,int*,int*);                                     typedef Bool(*PFN_XFilterEvent)(XEvent*,Window);                typedef int(*PFN_XFindContext)(Display*,XID,XContext,XPointer*);         typedef Window(*PFN_XCreateWindow)(Display*,Window,int,int,u32,u32,u32,int,u32,Visual*,u64,XSetWindowAttributes*);
    typedef int(*PFN_XFree)(void*);                              typedef void(*PFN_XFreeEventData)(Display*,XGenericEventCookie*);  typedef int(*PFN_XGrabPointer)(Display*,Window,Bool,u32,int,int,Window,Cursor,Time);        typedef void(*PFN_XSetICFocus)(XIC);                            typedef KeySym*(*PFN_XGetKeyboardMapping)(Display*,KeyCode,int,int*);    typedef Status(*PFN_XGetWMNormalHints)(Display*,Window,XSizeHints*,long*);
    typedef Atom(*PFN_XInternAtom)(Display*,const char*,Bool);   typedef int(*PFN_XGetInputFocus)(Display*,Window*,int*);           typedef int(*PFN_XMapWindow)(Display*,Window);                                              typedef int(*PFN_XMoveWindow)(Display*,Window,int,int);         typedef int(*PFN_XMoveResizeWindow)(Display*,Window,int,int,u32,u32);    typedef int(*PFN_XGetWindowProperty)(Display*,Window,Atom,long,long,Bool,Atom,Atom*,int*,u64*,u64*,u8**);
    typedef Status(*PFN_XInitThreads)();                         typedef int(*PFN_XNextEvent)(Display*,XEvent*);                    typedef XRRCrtcInfo*(*PFN_XRRGetCrtcInfo)(Display*,XRRScreenResources*,RRCrtc);             typedef int(*PFN_XPending)(Display*);                           typedef Bool(*PFN_XQueryExtension)(Display*,const char*,int*,int*,int*); typedef Bool(*PFN_XQueryPointer)(Display*,Window,Window*,Window*,int*,int*,int*,int*,u32*);
    typedef int(*PFN_XRaiseWindow)(Display*,Window);             typedef int(*PFN_XSaveContext)(Display*,XID,XContext,const char*); typedef int(*PFN_XResizeWindow)(Display*,Window,u32,u32);                                   typedef XcursorImage*(*PFN_XCIC)(int,int);                      typedef Status(*PFN_XSendEvent)(Display*,Window,Bool,long,XEvent*);      typedef int(*PFN_XSetInputFocus)(Display*,Window,int,Time);
    typedef void(*PFN_XUnsetICFocus)(XIC);                       typedef Status(*PFN_XSetWMProtocols)(Display*,Window,Atom*,int);   typedef Bool(*PFN_XTranslateCoordinates)(Display*,Window,Window,int,int,int*,int*,Window*); typedef int(*PFN_XUndefineCursor)(Display*,Window);             typedef void(*PFN_XSetWMNormalHints)(Display*,Window,XSizeHints*);       typedef int(*PFN_XWarpPointer)(Display*,Window,Window,int,int,u32,u32,int,int);
    typedef void(*PFN_XRRFreeCrtcInfo)(XRRCrtcInfo*);            typedef int(*PFN_XUngrabPointer)(Display*,Time);                   typedef int(*PFN_XChangeWindowAttributes)(Display*,Window,u64,XSetWindowAttributes*);       typedef void(*PFN_XRRFreeScreenResources)(XRRScreenResources*); typedef Display*(*PFN_XOpenDisplay)(const char*);                        typedef XRROutputInfo*(*PFN_XRRGetOutputInfo)(Display*,XRRScreenResources*,RROutput);
    typedef RROutput(*PFN_XRRGetOutputPrimary)(Display*,Window); typedef void(*PFN_XRRSelectInput)(Display*,Window,int);            typedef XRRScreenResources*(*PFN_XRRGetScreenResourcesCurrent)(Display*,Window);            typedef int(*PFN_XRRUpdateConfiguration)(XEvent*);              typedef Bool(*PFN_XCheckTypedWindowEvent)(Display*,Window,int,XEvent*);  typedef Status(*PFN_XGetWindowAttributes)(Display*,Window,XWindowAttributes*);
    typedef Bool(*GLX_QEP)(Display*,int*,int*);                  typedef int(*GLX_GFBCAP)(Display*,GLXFBConfig,int,int*);           typedef GLXContext(*GLX_CNCP)(Display*,GLXFBConfig,int,GLXContext,Bool);                    typedef Bool(*GLX_QVP)(Display*,int*,int*);                     typedef Bool(*GLX_MCP)(Display*,GLXDrawable,GLXContext);                 typedef void(*GLX_SBP)(Display*,GLXDrawable);
    typedef Cursor(*PFN_XCILC)(Display*,const XcursorImage*);    typedef const char*(*GLX_QESP)(Display*,int);                      typedef GLXFBConfig*(*GLX_GFBCP)(Display*,int,int*);                                        typedef __GLXextproc(*GLX_GPAP)(const u8*);                     typedef void(*GLX_SIEP)(Display*,GLXDrawable,int);                       typedef XVisualInfo*(*GLX_GVFFBCP)(Display*,GLXFBConfig);
    typedef GLXWindow(*GLX_CWP)(Display*,GLXFBConfig,Window,const int*); typedef GLXContext(*GLX_CCAA)(Display*,GLXFBConfig,GLXContext,Bool,const int*); typedef struct WinSyscontextGLX { GLXContext handle; GLXWindow window; GLXFBConfig fbconfig; } WinSyscontextGLX;
    typedef struct WinSyslibraryGLX { int major,minor,eventBase,errorBase; void* handle; GLX_GFBCP GetFBConfigs; GLX_GFBCAP GetFBConfigAttrib; GLX_QEP QueryExtension; GLX_QVP QueryVersion; GLX_MCP MakeCurrent; GLX_SBP SwapBuffers; GLX_QESP QueryExtensionsString; GLX_CNCP CreateNewContext; GLX_GVFFBCP GetVisualFromFBConfig; GLX_CWP CreateWindow; GLX_GPAP GetProcAddress; GLX_SIEP SwapIntervalEXT; GLX_CCAA CreateContextAttribsARB; } WinSyslibraryGLX;
    typedef struct WinSyswindowX11 { Colormap colormap; Window handle,parent; XIC ic; i32 overrideRedirect; int width,height,xpos,ypos,lastCurX,lastCurY,warpCursorPosX,warpCursorPosY; } WinSyswindowX11;
    typedef struct WinSyslibraryX11 { Display* display; int screen; Window root; Cursor hiddenCursorHandle; XContext context; short int keycodes[256],scancodes[349]; double restoreCurPosX, restoreCurPosY; WinSyswindow* disabledCursorWindow;
                                     Atom NET_SUPPORTED,NET_SUPPORTING_WM_CHECK,WM_PROTOCOLS,WM_STATE,WM_DELETE_WINDOW,NWM_NAME,NWM_ICON,NWM_PING,NWM_WINDOW_TYPE,NWM_WINDOW_TYPE_NORMAL,NWM_STATE,NWM_STATE_FULLSCREEN,NWM_BYPASS_COMPOSITOR,NET_WORKAREA,NET_CURRENT_DESKTOP,NET_ACTIVE_WINDOW,MOTIF_WM_HINTS,UTF8_STRING;
                                     struct { void* handle; i32 utf8; PFN_XAllocSizeHints AllocSizeHints; PFN_XChangeProperty ChangeProperty; PFN_XChangeWindowAttributes ChangeWindowAttributes; PFN_XCheckTypedWindowEvent CheckTypedWindowEvent; PFN_XCreateColormap CreateColormap; PFN_XCreateWindow CreateWindow; PFN_XDefineCursor DefineCursor;
                                     PFN_XDeleteProperty DeleteProperty; PFN_XDisplayKeycodes DisplayKeycodes; PFN_XFilterEvent FilterEvent; PFN_XFindContext FindContext; PFN_XFree Free; PFN_XFreeEventData FreeEventData; PFN_XGetInputFocus GetInputFocus; PFN_XGetKeyboardMapping GetKeyboardMapping; PFN_XGetWMNormalHints GetWMNormalHints;
                                     PFN_XGetWindowAttributes GetWindowAttributes; PFN_XGetWindowProperty GetWindowProperty; PFN_XGrabPointer GrabPointer; PFN_XInternAtom InternAtom; PFN_XMapWindow MapWindow; PFN_XMoveResizeWindow MoveResizeWindow; PFN_XMoveWindow MoveWindow; PFN_XPending Pending; PFN_XQueryExtension QueryExtension;
                                     PFN_XQueryPointer QueryPointer; PFN_XRaiseWindow RaiseWindow; PFN_XResizeWindow ResizeWindow; PFN_XSaveContext SaveContext; PFN_XSendEvent SendEvent; PFN_XSetICFocus SetICFocus; PFN_XSetInputFocus SetInputFocus; PFN_XSetWMNormalHints SetWMNormalHints; PFN_XSetWMProtocols SetWMProtocols;
                                     PFN_XTranslateCoordinates TranslateCoordinates; PFN_XUndefineCursor UndefineCursor; PFN_XUngrabPointer UngrabPointer; PFN_XUnsetICFocus UnsetICFocus; PFN_XWarpPointer WarpPointer; } xlib;
                                     struct {void* handle; int eventBase,errorBase,major,minor; PFN_XRRFreeCrtcInfo FreeCrtcInfo; PFN_XRRFreeOutputInfo FreeOutputInfo; PFN_XRRFreeScreenResources FreeScreenResources; PFN_XRRGetCrtcInfo GetCrtcInfo; PFN_XRRGetOutputInfo GetOutputInfo; PFN_XRRGetOutputPrimary GetOutputPrimary; PFN_XRRGetScreenResourcesCurrent GetScreenResourcesCurrent; PFN_XRRSelectInput SelectInput; PFN_XRRUpdateConfiguration UpdateConfiguration;}randr; } WinSyslibraryX11;
    PFN_XNextEvent XNextEvent; typedef struct WinSysmonitorX11 { RROutput output; RRCrtc crtc; int index; } WinSysmonitorX11; typedef struct WinSysjoystickLinux { FHandle fd; char path[260]; int keyMap[0x300/*KEY_CNT*/ - 0x100/*BTN_MISC*/],absMap[0x40/*ABS_CNT*/]; struct input_absinfo absInfo[0x40/*ABS_CNT*/]; int hats[4][2]; } WinSysjoystickLinux; typedef struct WinSyslibraryLinux { int inotify,watch; i32 dropped; } WinSyslibraryLinux;
    void GetCursorPosV(WinSyswindow*,double*,double*); void SetCurV(WinSyswindow*,double,double);
    struct WinSysjoystick { i32 allocated,connected; size_t axesSize,buttonsSize,hatsSize; float*  axes; int axisCount; u8* buttons; int buttonCount; u8* hats; int hatCount; char name[128],guid[33]; WinSysjoystickLinux linjs; };
    struct WinSyslibrary { WinSysmonitor** monitors; int monitorCount; i32 joysInited; WinSysjoystick joysticks[JOYSTICK_LAST + 1]; WinSyslibraryX11 x11; WinSyslibraryGLX glx; WinSyslibraryLinux linjs; };
    struct WinSyscontext { int client,source,major,minor; FGL_GIV GetIntegerv; void (*makeCurrent)(WinSyswindow*); void (*swapBuffers)(WinSyswindow*); void (*swapInterval)(int); WinSysglproc (*getProcAddress)(const char*); WinSyscontextGLX glx; };
    struct WinSyswindow { i32 decorated,doublebuffer; vidmode videoMode; int minwidth,minheight,maxwidth,maxheight,cursorMode; char mouseButtons[8],keys[349]; double virtualCursorPosX,virtualCursorPosY; WinSyscontext context; WinSyswindowX11 x11; };
    struct WinSysmonitor { char name[128]; int widthMM,heightMM; vidmode currentMode; WinSysmonitorX11 x11; };
    void* WinSysPlatformLoadModule(const char* path) { return dlopen(path,2); }
    WinSysproc PlatformGetModuleSymbol(void* module, const char* name) { return dlsym(module,name); }
    u64 WinSysGetWindowPropertyX11(Window win, Atom prop, Atom type, u8** val) { Atom actType; i32 actFmt; u64 itemCount,bytesAfter; WinSys.x11.xlib.GetWindowProperty(WinSys.x11.display,win,prop,0,2147483647,0,type,&actType,&actFmt,&itemCount,&bytesAfter,val); return itemCount; }
    static int translateKey(int scancode) { return (scancode<0||scancode>255) ? KEY_UNKNOWN : WinSys.x11.keycodes[scancode]; }
    static void sendEventToWM(WinSyswindow* win, Atom type, i64 a, i64 b, i64 c, i64 d, i64 f) { XEvent e={33/*ClientMessage*/}; e.xclient.window = win->x11.handle; e.xclient.format = 32; e.xclient.message_type = type; e.xclient.data.l[0]=a; e.xclient.data.l[1]=b; e.xclient.data.l[2]=c; e.xclient.data.l[3]=d; e.xclient.data.l[4]=f; WinSys.x11.xlib.SendEvent(WinSys.x11.display,WinSys.x11.root,0,(1L<<19)|(1L<<20),&e); }
    static void updateNormalHints(WinSyswindow* win, int w, int h) { XSizeHints* hs=WinSys.x11.xlib.AllocSizeHints(); i64 s; WinSys.x11.xlib.GetWMNormalHints(WinSys.x11.display,win->x11.handle,hs,&s); hs->flags &= ~((1L<<4)|(1L<<5)|(1L<<7)); hs->flags|=((1L<<4)|(1L<<5)); hs->min_width=hs->max_width=w; hs->min_height=hs->max_height=h; WinSys.x11.xlib.SetWMNormalHints(WinSys.x11.display,win->x11.handle,hs); WinSys.x11.xlib.Free(hs); }
    static void updateCursorImage(WinSyswindow* win) { if (win->cursorMode==0x00034001/*WinSys_CURSOR_NORMAL*/) { WinSys.x11.xlib.UndefineCursor(WinSys.x11.display,win->x11.handle); } else {WinSys.x11.xlib.DefineCursor(WinSys.x11.display,win->x11.handle,WinSys.x11.hiddenCursorHandle);} }
    static void captureCursor(WinSyswindow* win) { WinSys.x11.xlib.GrabPointer(WinSys.x11.display,win->x11.handle,1,(1L<<2)|(1L<<3)|(1L<<6),1/*GrabModeAsync*/,1/*GrabModeAsync*/,win->x11.handle,0L,0L); }
    static void releaseCursor() { WinSys.x11.xlib.UngrabPointer(WinSys.x11.display,0L); }
    static void disableCursor(WinSyswindow* win) { WinSys.x11.disabledCursorWindow=win; GetCursorPosV(win,&WinSys.x11.restoreCurPosX,&WinSys.x11.restoreCurPosY); updateCursorImage(win); captureCursor(win); }
    static void enableCursor(WinSyswindow* win) { WinSys.x11.disabledCursorWindow = NULL; releaseCursor(); SetCurV(win,WinSys.x11.restoreCurPosX,WinSys.x11.restoreCurPosY); updateCursorImage(win); }
    void GetMonitorPos(WinSysmonitor* monitor, int* x, int* y) { XRRScreenResources* sr=WinSys.x11.randr.GetScreenResourcesCurrent(WinSys.x11.display,WinSys.x11.root); XRRCrtcInfo* ci=WinSys.x11.randr.GetCrtcInfo(WinSys.x11.display,sr,monitor->x11.crtc); if(ci){*x=ci->x; *y=ci->y; WinSys.x11.randr.FreeCrtcInfo(ci);} WinSys.x11.randr.FreeScreenResources(sr); }
    void SetWindowIcon(const WinSysIcon* image) {
        int longCount=0; longCount+=2+image[0].width*image[0].height; u64* icon=OS_Calloc(longCount,sizeof(u64)), *target=icon; *target++=image[0].width; *target++=image[0].height;
        for (int j=0;j<image[0].width*image[0].height;++j) *target++=(((u64)image[0].pixels[j*4+0])<<16)|(((u64)image[0].pixels[j*4+1])<<8)|(((u64)image[0].pixels[j*4+2])<<0)|(((u64)image[0].pixels[j*4+3])<<24);
        WinSys.x11.xlib.ChangeProperty(WinSys.x11.display,((WinSyswindow*)window)->x11.handle,WinSys.x11.NWM_ICON,((Atom) 6),32,0/*PropModeReplace*/,(u8*)icon,longCount);
        OS_Free(icon,longCount*sizeof(u64));
    }

    void GetWindowSize(WinSyswindow* win, int* width, int* height) { XWindowAttributes attribs; WinSys.x11.xlib.GetWindowAttributes(WinSys.x11.display,win->x11.handle,&attribs); *width=attribs.width; *height=attribs.height; }
    void SetWindowSize(WinSyswindow* win, int width, int height) { width=vmax(1,width); height=vmax(1,height); updateNormalHints(win,width,height); WinSys.x11.xlib.ResizeWindow(WinSys.x11.display,win->x11.handle,width,height); }
    void SetWindowMonitor(WinSyswindow* win,int x,int y,int width,int height) {
        updateNormalHints(win,width,height);
        if (WinSys.x11.NWM_STATE && WinSys.x11.NWM_STATE_FULLSCREEN) sendEventToWM(win,WinSys.x11.NWM_STATE,0/*remove*/,WinSys.x11.NWM_STATE_FULLSCREEN,0,1,0);
        else { XSetWindowAttributes attributes; attributes.override_redirect=0; WinSys.x11.xlib.ChangeWindowAttributes(WinSys.x11.display,win->x11.handle,(1L<<9)/*override redirect*/,&attributes); win->x11.overrideRedirect=0; }
        WinSys.x11.xlib.DeleteProperty(WinSys.x11.display,win->x11.handle,WinSys.x11.NWM_BYPASS_COMPOSITOR);
        WinSys.x11.xlib.MoveResizeWindow(WinSys.x11.display,win->x11.handle,x,y,width,height);
    }
    
    i32 WindowFocused() { Window focused; int state; WinSys.x11.xlib.GetInputFocus(WinSys.x11.display,&focused,&state); return ((WinSyswindow*)window)->x11.handle==focused; }
    i32 WindowVisible() { XWindowAttributes wa; WinSys.x11.xlib.GetWindowAttributes(WinSys.x11.display,((WinSyswindow*)window)->x11.handle,&wa); return wa.map_state==2/*IsViewable*/; }
    void GetWindowPos(WinSyswindow* win, int* x, int* y) { Window dummy; WinSys.x11.xlib.TranslateCoordinates(WinSys.x11.display,win->x11.handle,WinSys.x11.root,0,0,x,y,&dummy); }
    void SetWindowPos(WinSyswindow* win, int x, int y) { if (!WindowVisible()) { i64 s; XSizeHints* h=WinSys.x11.xlib.AllocSizeHints(); if (WinSys.x11.xlib.GetWMNormalHints(WinSys.x11.display,win->x11.handle,h,&s)) {h->flags|=(1L << 2)/*PPosition*/; h->x=h->y=0; WinSys.x11.xlib.SetWMNormalHints(WinSys.x11.display,win->x11.handle,h);} WinSys.x11.xlib.Free(h); } WinSys.x11.xlib.MoveWindow(WinSys.x11.display,win->x11.handle,x,y); }
    void SetWindowDecorated(WinSyswindow* win,i32 enabled) { struct {u64 flags,functions,decorations; i64 input_mode; u64 status;} hints={0}; hints.flags=2; hints.decorations=enabled?1:0; WinSys.x11.xlib.ChangeProperty(WinSys.x11.display,win->x11.handle,WinSys.x11.MOTIF_WM_HINTS,WinSys.x11.MOTIF_WM_HINTS,32,0/*PropModeReplace*/,(u8*)&hints,sizeof(hints)/sizeof(i64)); }
    void GetCursorPosV(WinSyswindow* win, double* xpos, double* ypos) { Window root,child; int rootX,rootY,childX,childY; u32 mask; WinSys.x11.xlib.QueryPointer(WinSys.x11.display,win->x11.handle,&root,&child,&rootX,&rootY,&childX,&childY,&mask); *xpos=childX; *ypos=childY; }
    void SetCurV(WinSyswindow* win, double x, double y) { win->x11.warpCursorPosX=(int)x; win->x11.warpCursorPosY=(int)y; WinSys.x11.xlib.WarpPointer(WinSys.x11.display,0L,win->x11.handle,0,0,0,0,(int)x,(int)y); }    
    static const XRRModeInfo* getModeInfo(const XRRScreenResources* sr, RRMode id) { for (int i = 0;  i < sr->nmode;  i++){ if (sr->modes[i].id == id) {return sr->modes + i;} } return NULL; }
    static vidmode vidmodeFromModeInfo(const XRRModeInfo* mi, const XRRCrtcInfo* ci) { vidmode mode; if(ci->rotation == 2 || ci->rotation == 8){mode.width=mi->height; mode.height=mi->width;/*90,270*/} else { mode.width = mi->width; mode.height = mi->height; } mode.refreshRate = (mi->hTotal && mi->vTotal) ? (int)vround((double) mi->dotClock / ((double) mi->hTotal * (double) mi->vTotal)) : 0; return mode; }
    void PollMonitors() {
        XRRScreenResources* sr = WinSys.x11.randr.GetScreenResourcesCurrent(WinSys.x11.display,WinSys.x11.root);
        RROutput primary = WinSys.x11.randr.GetOutputPrimary(WinSys.x11.display,WinSys.x11.root);
        int disconnectedCount = WinSys.monitorCount; WinSysmonitor** disconnected = NULL;
        if (disconnectedCount) { disconnected = OS_Calloc(WinSys.monitorCount,sizeof(WinSysmonitor*)); mcpy(disconnected,WinSys.monitors,WinSys.monitorCount * sizeof(WinSysmonitor*)); }
        for (int i = 0;  i < sr->noutput;  i++) {
            int j, type, widthMM, heightMM;
            XRROutputInfo* oi = WinSys.x11.randr.GetOutputInfo(WinSys.x11.display, sr, sr->outputs[i]);
            if (oi->connection != 0/*connected*/ || oi->crtc == 0L) { WinSys.x11.randr.FreeOutputInfo(oi); continue; }
            for (j=0;j<disconnectedCount;++j) { if(disconnected[j] && disconnected[j]->x11.output == sr->outputs[i]){disconnected[j]=NULL; break;} }
            if (j < disconnectedCount) { WinSys.x11.randr.FreeOutputInfo(oi); continue; }
            XRRCrtcInfo* ci = WinSys.x11.randr.GetCrtcInfo(WinSys.x11.display, sr, oi->crtc); if (!ci) { WinSys.x11.randr.FreeOutputInfo(oi); continue; }

            if (ci->rotation == 2 || ci->rotation == 8) { widthMM  = oi->mm_height; heightMM = oi->mm_width; } // == 90, == 270
            else { widthMM  = oi->mm_width; heightMM = oi->mm_height; }
            
            if (widthMM <= 0 || heightMM <= 0) { widthMM  = (int) (ci->width * 25.4f / 96.f); heightMM = (int) (ci->height * 25.4f / 96.f); }
            WinSysmonitor* monitor = AllocMonitor(oi->name, widthMM, heightMM);
            monitor->x11.output = sr->outputs[i]; monitor->x11.crtc   = oi->crtc;
            type = (monitor->x11.output == primary) ? 0 : 1; InputMonitor(monitor,0x00040001/*connected*/,type); WinSys.x11.randr.FreeOutputInfo(oi); WinSys.x11.randr.FreeCrtcInfo(ci);
        }

        WinSys.x11.randr.FreeScreenResources(sr);
        for (int i=0;i<disconnectedCount;++i) { if (disconnected[i]) {InputMonitor(disconnected[i],0x00040002/*disconnected*/,0);} }
        if (disconnected) OS_Free(disconnected,WinSys.monitorCount*sizeof(WinSysmonitor*));
    }
    
    static void processEvent(XEvent* e) {
        u32 keycode=0; if (e->type==2/*KeyPress*/ || e->type==3/*KeyRelease*/) keycode=e->xkey.keycode;
        Bool filt=WinSys.x11.xlib.FilterEvent(e,0L);
        if (e->type==WinSys.x11.randr.eventBase+1/*notify*/) { WinSys.x11.randr.UpdateConfiguration(e); PollMonitors(); return; }
        if (e->type==35/*GenericEvent*/) return;
        WinSyswindow* win=NULL; if (WinSys.x11.xlib.FindContext(WinSys.x11.display,e->xany.window,WinSys.x11.context,(XPointer*)&win)!=0) return;

        switch (e->type) {
            case 21/*ReparentNotify*/: win->x11.parent=e->xreparent.parent; return;
            case 2/*KeyPress*/: case 3/*KeyRelease*/: { const int key=translateKey(keycode),action=(e->type==2/*KeyPress*/)?INPUT_PRESS:INPUT_RELEASE; if (key!=KEY_UNKNOWN) {InputKey(win,key,action);} return; }
            case 4: /* ButtonPress */ case 5: /* ButtonRelease */ {
                const int btn = e->xbutton.button;
                const int action = (e->type == 4) ? INPUT_PRESS : INPUT_RELEASE;
                if      (btn == 1) InputMouseClick(win,MOUSE_BUTTON_LEFT,action);
                else if (btn == 2) InputMouseClick(win,MOUSE_BUTTON_MIDDLE,action);
                else if (btn == 3) InputMouseClick(win,MOUSE_BUTTON_RIGHT,action);
                else if (action == INPUT_PRESS && btn == 4) Sys_Input.scrollDelta += 1.0f;
                else if (action == INPUT_PRESS && btn == 5) Sys_Input.scrollDelta -= 1.0f;
                else if (btn > 7) InputMouseClick(win,btn - 5,action);
                return; }
            case 7/*EnterNotify*/: { const int x=e->xcrossing.x,y=e->xcrossing.y; InputCursorPos(win,x,y); win->x11.lastCurX=x; win->x11.lastCurY=y; return; }
            case 6/*MotionNotify*/: { const int x=e->xmotion.x, y=e->xmotion.y; if (x!=win->x11.warpCursorPosX || y!=win->x11.warpCursorPosY) { if (win->cursorMode==0x00034003/*CURSOR_DISABLED*/) { if(WinSys.x11.disabledCursorWindow!=win){return;} InputCursorPos(win,win->virtualCursorPosX + (x - win->x11.lastCurX),win->virtualCursorPosY + (y - win->x11.lastCurY)); } else {InputCursorPos(win,x,y);} } win->x11.lastCurX=x; win->x11.lastCurY=y; return; }
            case 22/*ConfigureNotify*/: {
                if (e->xcfg.width!=win->x11.width || e->xcfg.height!=win->x11.height) { win->x11.width=e->xcfg.width; win->x11.height=e->xcfg.height; UpdateScreenSize(e->xcfg.width,e->xcfg.height); }
                int xpos=e->xcfg.x, ypos=e->xcfg.y;
                if (!e->xany.send_event && win->x11.parent!=WinSys.x11.root) { Window dummy; WinSys.x11.xlib.TranslateCoordinates(WinSys.x11.display,win->x11.parent,WinSys.x11.root,xpos,ypos,&xpos,&ypos,&dummy); }
                if (xpos!=win->x11.xpos || ypos!=win->x11.ypos) { win->x11.xpos=xpos; win->x11.ypos=ypos; }
                return; }
            case 33/*ClientMessage*/: { if(filt || e->xclient.message_type==0L){return;} if (e->xclient.message_type==WinSys.x11.WM_PROTOCOLS) { const Atom p=e->xclient.data.l[0]; if (p==0L) {return;} if (p == WinSys.x11.WM_DELETE_WINDOW) {OS_Exit(0);} if (p == WinSys.x11.NWM_PING) { XEvent rp=*e; rp.xclient.window=WinSys.x11.root; WinSys.x11.xlib.SendEvent(WinSys.x11.display,WinSys.x11.root,0,(1L<<19)|(1L<<20),&rp); } } return; }
            case  9/*FocusIn */: { if (e->xfocus.mode==1/*Grab*/ || e->xfocus.mode==2/*Ungrab*/) {return;} if (win->cursorMode==0x00034003/*CURSOR_DISABLED*/) {disableCursor(win);} if (win->x11.ic) {WinSys.x11.xlib.SetICFocus(  win->x11.ic);} InputWindowFocus(1); return; }
            case 10/*FocusOut*/: { if (e->xfocus.mode==1/*Grab*/ || e->xfocus.mode==2/*Ungrab*/) {return;} if (win->cursorMode==0x00034003/*CURSOR_DISABLED*/) { enableCursor(win);} if (win->x11.ic) {WinSys.x11.xlib.UnsetICFocus(win->x11.ic);} InputWindowFocus(0); return; }
        }
    }

    void GetMonitorWorkarea(WinSysmonitor* monitor,int* xpos,int* ypos,int* width,int* height) {
        int areaWidth = 0, areaHeight = 0; XRRScreenResources* sr = WinSys.x11.randr.GetScreenResourcesCurrent(WinSys.x11.display,WinSys.x11.root); XRRCrtcInfo* ci = WinSys.x11.randr.GetCrtcInfo(WinSys.x11.display,sr,monitor->x11.crtc);
        const XRRModeInfo* mi = getModeInfo(sr,ci->mode); int areaX = ci->x, areaY = ci->y;
        if (ci->rotation == 2 || ci->rotation == 8) { areaWidth = mi->height, areaHeight = mi->width; } // ==90, ==270
        else { areaWidth = mi->width, areaHeight = mi->height; }
        WinSys.x11.randr.FreeCrtcInfo(ci); WinSys.x11.randr.FreeScreenResources(sr);
        if (WinSys.x11.NET_WORKAREA && WinSys.x11.NET_CURRENT_DESKTOP) {
            Atom *extents = NULL, *desktop = NULL;
            const unsigned long extentCount = WinSysGetWindowPropertyX11(WinSys.x11.root,WinSys.x11.NET_WORKAREA,((Atom) 6),(u8**) &extents);
            if (WinSysGetWindowPropertyX11(WinSys.x11.root,WinSys.x11.NET_CURRENT_DESKTOP,((Atom) 6),(u8**) &desktop) > 0) {
                if (extentCount >= 4 && *desktop < extentCount / 4) {
                    const int gx = extents[*desktop * 4 + 0], gy = extents[*desktop * 4 + 1], gw = extents[*desktop * 4 + 2], gh = extents[*desktop * 4 + 3];
                    if (areaX < gx) { areaWidth  -= gx - areaX, areaX = gx; }
                    if (areaY < gy) { areaHeight -= gy - areaY, areaY = gy; }
                    if (areaX +  areaWidth > gx + gw)  areaWidth = gx - areaX + gw;
                    if (areaY + areaHeight > gy + gh) areaHeight = gy - areaY + gh;
                }
            }
            if (extents) {WinSys.x11.xlib.Free(extents);} if (desktop) {WinSys.x11.xlib.Free(desktop);}
        }
        *xpos = areaX; *ypos = areaY; *width = areaWidth; *height = areaHeight;
    }

    void GetVideoMode(WinSysmonitor* m, vidmode* v) { XRRScreenResources* sr = WinSys.x11.randr.GetScreenResourcesCurrent(WinSys.x11.display,WinSys.x11.root); const XRRModeInfo* mi=NULL; XRRCrtcInfo* ci = WinSys.x11.randr.GetCrtcInfo(WinSys.x11.display,sr,m->x11.crtc); if(ci){ mi = getModeInfo(sr,ci->mode); if(mi){*v = vidmodeFromModeInfo(mi,ci);} WinSys.x11.randr.FreeCrtcInfo(ci); } WinSys.x11.randr.FreeScreenResources(sr); }
    static int translateKeySyms(const KeySym* keysyms, int width) {
        if (width > 1) { // Numpad with numlock ON (keysyms[1]) - contiguous 0xffb0..0xffb9
            if (keysyms[1] >= 0xffb0 && keysyms[1] <= 0xffb9) return KEY_KP_0 + (keysyms[1] - 0xffb0);
            switch (keysyms[1]) {
                case 0xffac: case 0xffae: return KEY_KP_DECIMAL; // KP_Separator, KP_Decimal
                case 0xffbd:              return KEY_KP_EQUAL;   // KP_Equal
                case 0xff8d:              return KEY_KP_ENTER;   // KP_Enter
                default: break;
            }
        }
        KeySym k = keysyms[0];
        if (k >= 0x0061 && k <= 0x007a) return KEY_A + (k - 0x0061);  // a-z
        if (k >= 0x0030 && k <= 0x0039) return KEY_0 + (k - 0x0030);  // 0-9
        if (k >= 0xffbe && k <= 0xffd6) return KEY_F1 + (k - 0xffbe); // F1..F25: 0xffbe..0xffd6
        if (k >= 0xff95 && k <= 0xff9f) { static const int kp_off[] = {KEY_KP_7,KEY_KP_4,KEY_KP_8,KEY_KP_6,KEY_KP_2,KEY_KP_9,KEY_KP_3,KEY_KP_1,-1,KEY_KP_0,KEY_KP_DECIMAL }; int r = kp_off[k - 0xff95]; if(r != -1){return r;} } // KP numpad with numlock OFF (cursor keys): 0xff95..0xff9f
        switch (k) {
            case 0xff1b: return KEY_ESCAPE;        case 0xff09: return KEY_TAB;          case 0xff0d: return KEY_ENTER;
            case 0xff08: return KEY_BACKSPACE;     case 0xffff: return KEY_DELETE;       case 0xff50: return KEY_HOME;
            case 0xff57: return KEY_END;           case 0xff55: return KEY_PAGE_UP;      case 0xff56: return KEY_PAGE_DOWN;
            case 0xff63: return KEY_INSERT;        case 0xff51: return KEY_LEFT;         case 0xff53: return KEY_RIGHT;
            case 0xff54: return KEY_DOWN;          case 0xff52: return KEY_UP;           case 0xff13: return KEY_PAUSE;
            case 0xff14: return KEY_SCROLL_LOCK;   case 0xff61: return KEY_PRINT_SCREEN; case 0xff7f: return KEY_NUM_LOCK;
            case 0xffe5: return KEY_CAPS_LOCK;     case 0xff67: return KEY_MENU;         case 0xffe1: return KEY_LEFT_SHIFT;
            case 0xffe2: return KEY_RIGHT_SHIFT;   case 0xffe3: return KEY_LEFT_CONTROL; case 0xffe4: return KEY_RIGHT_CONTROL;
            case 0xffe7: case 0xffe9: return KEY_LEFT_ALT;   // Meta_L, Alt_L
            case 0xff7e: case 0xfe03: case 0xffe8: case 0xffea: return KEY_RIGHT_ALT; // Mode_switch, ISO_Level3_Shift, Meta_R, Alt_R
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
        int scancodeMin, scancodeMax; mset(WinSys.x11.keycodes,-1,sizeof(WinSys.x11.keycodes)); mset(WinSys.x11.scancodes,-1,sizeof(WinSys.x11.scancodes)); WinSys.x11.xlib.DisplayKeycodes(WinSys.x11.display,&scancodeMin,&scancodeMax);
        int width; KeySym* keysyms = WinSys.x11.xlib.GetKeyboardMapping(WinSys.x11.display,scancodeMin,scancodeMax - scancodeMin + 1,&width);
        for (int sc = scancodeMin; sc <= scancodeMax; sc++) { if (WinSys.x11.keycodes[sc] < 0) {WinSys.x11.keycodes[sc] = translateKeySyms(&keysyms[(sc - scancodeMin) * width],width);} if (WinSys.x11.keycodes[sc] > 0) {WinSys.x11.scancodes[WinSys.x11.keycodes[sc]] = sc;} }
        WinSys.x11.xlib.Free(keysyms);
    }

    static Atom getAtomIfSupported(Atom* atoms, unsigned long count, const char* name) { const Atom atom=WinSys.x11.xlib.InternAtom(WinSys.x11.display,name,0); for (unsigned long i=0;i<count;i++) {if (atoms[i] == atom) {return atom;}} return 0L; }
    static void handleKeyEvent(WinSysjoystick* js, int code, int value) { InputJoystickButton(js,js->linjs.keyMap[code - 0x100/*BTN_MISC*/],value ? INPUT_PRESS : INPUT_RELEASE); }
    static void handleAbsEvent(WinSysjoystick* js, int code, int value) {
        const int index = js->linjs.absMap[code];
        if (code >= 0x10/*ABS_HAT0X*/ && code <= 0x17/*ABS_HAT3Y*/) {
            static const char stateMap[3][3] = {{JOYHAT_CENTERED,JOYHAT_UP,JOYHAT_DOWN},{JOYHAT_LEFT,JOYHAT_LEFT_UP,JOYHAT_LEFT_DOWN},{JOYHAT_RIGHT,JOYHAT_RIGHT_UP,JOYHAT_RIGHT_DOWN},};
            const int hat = (code - 0x10/*ABS_HAT0X*/) / 2, axis = (code - 0x10/*ABS_HAT0X*/) % 2;
            int* state = js->linjs.hats[hat];
            state[axis] = (value == 0) ? 0 : value < 0 ? 1 : value > 0 ? 2 : state[axis];
            InputJoystickHat(js, index, stateMap[state[0]][state[1]]);
        } else {
            const struct input_absinfo* info = &js->linjs.absInfo[code];
            float normalized = value;
            const int range = info->maximum - info->minimum;
            if (range) { normalized = (normalized - info->minimum) / range; normalized = normalized * 2.0f - 1.0f; }
            InputJoystickAxis(js, index, normalized);
        }
    }

    static void pollAbsState(WinSysjoystick* js) { for (int code=0;code<0x40/*ABS_CNT*/;code++) { if (js->linjs.absMap[code] < 0) {continue;} struct input_absinfo* info = &js->linjs.absInfo[code]; if (OS_IOControl(js->linjs.fd,(0x80184540 + (code)),info) < 0) {continue;} handleAbsEvent(js,code,info->value); } }
    #define isBitSet(bit, arr) (arr[(bit) / 8] & (1 << ((bit) % 8)))
    #define EVIOCGBIT(ev, len) (0x80004520 + (ev) + ((len) << 16))
    static i32 openJoystickDevice(const char* path) {
        for (int jid = 0;  jid <= JOYSTICK_LAST;  jid++) { if(!WinSys.joysticks[jid].connected){continue;} if(sEqual(WinSys.joysticks[jid].linjs.path,path)){return 0;} }
        WinSysjoystickLinux linjs = {0}; linjs.fd = OS_Open(path,00004000|02000000,0); if (linjs.fd == -1) return 0;
        char evBits[(0x20/*EV_CNT*/ + 7) / 8] = {0},keyBits[(0x300/*KEY_CNT*/ + 7) / 8] = {0},absBits[(0x40/*ABS_CNT*/ + 7) / 8] = {0};
        struct input_id id; if (OS_IOControl(linjs.fd,EVIOCGBIT(0,sizeof(evBits)),evBits) < 0 || OS_IOControl(linjs.fd,EVIOCGBIT(0x01/*EV_KEY*/,sizeof(keyBits)),keyBits) < 0 || OS_IOControl(linjs.fd,EVIOCGBIT(0x03/*EV_ABS*/,sizeof(absBits)),absBits) < 0 || OS_IOControl(linjs.fd,0x80084501/*EVIOCGID*/,&id) < 0) { OS_Close(linjs.fd); return 0; }
        if (!isBitSet(0x03/*EV_ABS*/,evBits)) { OS_Close(linjs.fd); return 0; }
        char name[256] = "",guid[33] = "";
        if (OS_IOControl(linjs.fd,(0x80004506 | (((sizeof(name)) & 0x1fff) << 16)),name) < 0) scpy_to_a_from_b(name,"Unknown",sizeof(name));
        if (id.vendor && id.product && id.version) sFormat(guid,sizeof(guid),"%02x%02x0000%02x%02x0000%02x%02x0000%02x%02x0000",id.bustype & 0xff, id.bustype >> 8,id.vendor & 0xff,  id.vendor >> 8,id.product & 0xff, id.product >> 8,id.version & 0xff, id.version >> 8);
        else sFormat(guid,sizeof(guid),"%02x%02x0000%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x00",id.bustype & 0xff, id.bustype >> 8,name[0], name[1], name[2], name[3],name[4], name[5], name[6], name[7],name[8], name[9], name[10]);
        int axisCount = 0, buttonCount = 0, hatCount = 0;
        for (int code=0x100/*BTN_MISC*/;code<0x300/*KEY_CNT*/;code++) { if(!isBitSet(code,keyBits)){continue;} linjs.keyMap[code - 0x100/*BTN_MISC*/]=buttonCount++; }
        for (int code=0;code<0x40/*ABS_CNT*/;code++) {
            linjs.absMap[code] = -1; if (!isBitSet(code,absBits)) continue;
            if (code >= 0x10/*ABS_HAT0X*/ && code <= 0x17/*ABS_HAT3Y*/) { linjs.absMap[code] = hatCount; hatCount++; code++; } // Skip the Y axis
            else { if(OS_IOControl(linjs.fd,(0x80184540 + (code)),&linjs.absInfo[code]) < 0){continue;} linjs.absMap[code]=axisCount++; }
        }
        WinSysjoystick* js = WinSysAllocJoystick(name,guid,axisCount,buttonCount,hatCount); if (!js) { OS_Close(linjs.fd); return 0; }
        scpy_to_a_from_b(linjs.path,path,sizeof(linjs.path)); mcpy(&js->linjs,&linjs,sizeof(linjs)); pollAbsState(js); JoystickConnection(js,0x00040001/*connected*/);
        return  1;
    }
    
    struct linux_dirent64 { u64 d_ino; i64 d_off; u16 d_reclen; u8 d_type; char d_name[]; };
    struct inotify_event { i32 wd; u32 mask,cookie,len; char name[]; };
    static void closeJoystick(WinSysjoystick* js) { JoystickConnection(js,0x00040002/*disconnected*/); if (js->linjs.fd > 0) { OS_Close(js->linjs.fd); js->linjs.fd = -1; } FreeJoystick(js); }
    static i32 isEventDevice(const char* name) { if (!name || !sCompUpToLen(name, "event", 5) || name[5] == '\0') {return 0;} for (const char* p=name+5;*p;++p) if (*p < '0' || *p > '9') {return 0;} return 1; }
    static void iterateInputDevices(void (*callback)(const char* fullpath)) {
        const char* dirname = "/dev/input"; FHandle fd = OS_Open(dirname,00200000|02000000,0); if (fd < 0) return;
        char buf[8192];
        for (;;) {
            register long rax __asm__("rax") = 217/*__NR_getdents64*/, rdi __asm__("rdi") = fd; register char* rsi __asm__("rsi") = buf; register size_t rdx __asm__("rdx") = sizeof(buf);
            __asm__ __volatile__("syscall":"+r"(rax):"r"(rdi),"r"(rsi),"r"(rdx):"rcx","r11","memory"); if (rax <= 0) break;
            long offset = 0;
            while (offset < rax) {
                struct linux_dirent64* d = (struct linux_dirent64*)(buf + offset);
                if (d->d_name[0] != '.' && isEventDevice(d->d_name)) { char path[260]; sFormat(path,sizeof(path),"%s/%s",dirname,d->d_name); callback(path); }
                offset += d->d_reclen;
            }
        }
        OS_Close(fd);
    }
    
    static void openJoystickCallback(const char* path) { openJoystickDevice(path); }
    static char joyConbuffer[16384],joyPath[260];
    void DetectJoyCnx() {
        if (WinSys.linjs.inotify <= 0) return;
        i32 size = OS_Read(WinSys.linjs.inotify,joyConbuffer,sizeof(joyConbuffer)); if (size <= 0) return;
        i32 offset = 0;
        while (size >= offset + (i32)sizeof(struct inotify_event)) {
            const struct inotify_event* e = (struct inotify_event*)(joyConbuffer + offset);
            offset += (i32)sizeof(struct inotify_event) + e->len;
            if (e->len == 0 || !isEventDevice(e->name)) continue;
            sFormat(joyPath,sizeof(joyPath), "/dev/input/%s", e->name);
            if (e->mask & (0x00000100/*IN_CREATE*/|0x00000004/*IN_ATTRIB*/)) openJoystickDevice(joyPath);
            else if (e->mask & 0x00000200/*IN_DELETE*/) {
                for (int jid = 0; jid <= JOYSTICK_LAST; jid++) {
                    if (sEqual(WinSys.joysticks[jid].linjs.path,joyPath)) { closeJoystick(WinSys.joysticks + jid); break; }
                }
            }
        }
    }

    i32 InitJoysticks() {
        const char* dirname = "/dev/input";
        {register long rax __asm__("rax") = 294/*__NR_inotify_init1*/; register u32 rdi __asm__("rdi") = 0x800/*IN_NONBLOCK*/|0x80000/*IN_CLOEXEC*/;
        __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi) : "rcx", "r11", "memory");
        WinSys.linjs.inotify = (int)rax; }
        if (WinSys.linjs.inotify >= 0) {
            register long rax __asm__("rax") = 295/*__NR_inotify_add_watch*/; register int rdi __asm__("rdi") = WinSys.linjs.inotify; register const char* rsi __asm__("rsi") = dirname;
            register u32 rdx __asm__("rdx") = 0x00000100/*IN_CREATE*/|0x00000004/*IN_ATTRIB*/|0x00000200/*IN_DELETE*/;
            __asm__ __volatile__("syscall":"+r"(rax):"r"(rdi),"r"(rsi),"r"(rdx):"rcx","r11","memory");
            WinSys.linjs.watch = (int)rax;
        }
        iterateInputDevices(openJoystickCallback);
        return 1;
    }

    i32 PollJoystick(WinSysjoystick* js) {
        if (js->linjs.fd <= 0) return 0;
        for (;;) {
            struct input_event e; long n = OS_Read(js->linjs.fd,&e,sizeof(e));
            if (n < 0) { closeJoystick(js); break; } if (n == 0) { break; } if (n < (long)sizeof(e)) { closeJoystick(js); break; }
            if (e.type == 0x00/*EV_SYN*/) { if(e.code == 3/*SYN_DROPPED*/){WinSys.linjs.dropped=1;}else if(e.code == 0/*SYN_REPORT*/){WinSys.linjs.dropped=0; pollAbsState(js);} }
            if (WinSys.linjs.dropped) continue;
                 if (e.type == 0x01/*EV_KEY*/) handleKeyEvent(js,e.code,e.value);
            else if (e.type == 0x03/*EV_ABS*/) handleAbsEvent(js,e.code,e.value);
        }
        return js->connected;
    }
    
    void PollEvents() { if (WinSys.joysInited) {DetectJoyCnx();} WinSys.x11.xlib.Pending(WinSys.x11.display); while (((_XPrivDisplay)(WinSys.x11.display))->qlen) { XEvent e; XNextEvent(WinSys.x11.display,&e); processEvent(&e); } WinSyswindow* win = WinSys.x11.disabledCursorWindow; if(win){ int w,h; GetWindowSize(win,&w,&h); if(win->x11.lastCurX!=w/2 || win->x11.lastCurY!=h/2){SetCurV(win,w/2,h/2);} } }
    static int getGLXFBConfigAttrib(GLXFBConfig fbconfig, int attrib) { int value; WinSys.glx.GetFBConfigAttrib(WinSys.x11.display, fbconfig, attrib, &value); return value; }
    static void makeContextCurrentGLX(WinSyswindow* win) { WinSys.glx.MakeCurrent(WinSys.x11.display,win->context.glx.window,win->context.glx.handle); }
    static void swapBuffersGLX(WinSyswindow* win) { WinSys.glx.SwapBuffers(WinSys.x11.display, win->context.glx.window); }
    static void swapIntervalGLX(int interval) { WinSyswindow* handle = (WinSyswindow*)window; WinSys.glx.SwapIntervalEXT(WinSys.x11.display,handle->context.glx.window,interval); }
    static WinSysglproc getProcAddressGLX(const char* procname) { return WinSys.glx.GetProcAddress((const u8*) procname); }
    void SetWindowPosition(WinSyswindow* handle, int xpos, int ypos) { WinSyswindow* win = (WinSyswindow*)handle; SetWindowPos(win,xpos,ypos); }
    WinSyswindow* VCreateWindow(int width, int height, char* title) {
        WinSyswindow* win = OS_Calloc(1,sizeof(WinSyswindow)); win->videoMode = (vidmode){width,height,8,8,8,-1}; win->decorated = win->doublebuffer = 1; win->cursorMode = 0x00034003/*disabled*/;
        const char* names[] = {"libGLX.so.0","libGL.so.1","libGL.so",NULL};
        for (int i=0;names[i] && !WinSys.glx.handle;i++) WinSys.glx.handle = WinSysPlatformLoadModule(names[i]);
        WinSys.glx.GetFBConfigs = (GLX_GFBCP)PlatformGetModuleSymbol(WinSys.glx.handle,"glXGetFBConfigs");
        WinSys.glx.GetFBConfigAttrib = (GLX_GFBCAP)PlatformGetModuleSymbol(WinSys.glx.handle,"glXGetFBConfigAttrib");
        WinSys.glx.QueryExtension = (GLX_QEP)PlatformGetModuleSymbol(WinSys.glx.handle,"glXQueryExtension");
        WinSys.glx.QueryVersion = (GLX_QVP)PlatformGetModuleSymbol(WinSys.glx.handle,"glXQueryVersion");
        WinSys.glx.MakeCurrent = (GLX_MCP)PlatformGetModuleSymbol(WinSys.glx.handle,"glXMakeCurrent");
        WinSys.glx.SwapBuffers = (GLX_SBP)PlatformGetModuleSymbol(WinSys.glx.handle,"glXSwapBuffers");
        WinSys.glx.QueryExtensionsString = (GLX_QESP)PlatformGetModuleSymbol(WinSys.glx.handle,"glXQueryExtensionsString");
        WinSys.glx.CreateNewContext = (GLX_CNCP)PlatformGetModuleSymbol(WinSys.glx.handle,"glXCreateNewContext");
        WinSys.glx.CreateWindow = (GLX_CWP)PlatformGetModuleSymbol(WinSys.glx.handle,"glXCreateWindow");
        WinSys.glx.GetVisualFromFBConfig = (GLX_GVFFBCP)PlatformGetModuleSymbol(WinSys.glx.handle,"glXGetVisualFromFBConfig");
        WinSys.glx.GetProcAddress = (GLX_GPAP)PlatformGetModuleSymbol(WinSys.glx.handle,"glXGetProcAddress");
        WinSys.glx.QueryExtension(WinSys.x11.display,&WinSys.glx.errorBase,&WinSys.glx.eventBase);
        WinSys.glx.QueryVersion(WinSys.x11.display,&WinSys.glx.major,&WinSys.glx.minor);
        WinSys.glx.SwapIntervalEXT = (GLX_SIEP)getProcAddressGLX("glXSwapIntervalEXT");
        WinSys.glx.CreateContextAttribsARB = (GLX_CCAA)getProcAddressGLX("glXCreateContextAttribsARB");
        GLXFBConfig native; XVisualInfo* result;
        GLXFBConfig* nativeConfigs; WinSysfbconfig* usableConfigs; const WinSysfbconfig* closest; int nativeCount,usableCount;
        nativeConfigs = WinSys.glx.GetFBConfigs(WinSys.x11.display,WinSys.x11.screen,&nativeCount);        
        usableConfigs = OS_Calloc(nativeCount,sizeof(WinSysfbconfig)); usableCount = 0;
        for (int i = 0;  i < nativeCount;  i++) {
            const GLXFBConfig n = nativeConfigs[i];
            WinSysfbconfig* u = usableConfigs + usableCount;
            if (!(getGLXFBConfigAttrib(n,0x8011/*render type*/) & 0x00000001/*rgba bit*/)) continue;
            if (!(getGLXFBConfigAttrib(n,0x8010/*drawable type*/) & 0x00000001/*window bit*/)) continue;
            if (getGLXFBConfigAttrib(n,5) !=  1) continue;
            u->redBits = getGLXFBConfigAttrib(n,8); u->greenBits = getGLXFBConfigAttrib(n,9); u->blueBits = getGLXFBConfigAttrib(n,10); u->alphaBits = getGLXFBConfigAttrib(n,11); u->depthBits = getGLXFBConfigAttrib(n,12); u->stencilBits = getGLXFBConfigAttrib(n,13);
            u->accumRedBits = getGLXFBConfigAttrib(n,14); u->accumGreenBits = getGLXFBConfigAttrib(n,15); u->accumBlueBits = getGLXFBConfigAttrib(n,16); u->accumAlphaBits = getGLXFBConfigAttrib(n,17);
            if (getGLXFBConfigAttrib(n,6)) u->stereo =  1;
            u->handle = (uintptr_t) n;
            usableCount++;
        }
        closest = ChooseFBConfig(usableConfigs,usableCount); native = (GLXFBConfig)closest->handle;
        WinSys.x11.xlib.Free(nativeConfigs); if (usableConfigs) OS_Free(usableConfigs,nativeCount*sizeof(WinSysfbconfig));
        result = WinSys.glx.GetVisualFromFBConfig(WinSys.x11.display,native);
        Visual* visual=result->visual; int depth = result->depth; WinSys.x11.xlib.Free(result); int xpos=0,ypos=0;
        win->x11.colormap=WinSys.x11.xlib.CreateColormap(WinSys.x11.display,WinSys.x11.root,visual,0);
        XSetWindowAttributes wa = {0}; wa.colormap = win->x11.colormap; wa.event_mask = 0x63807F;
        win->x11.parent=WinSys.x11.root; win->x11.handle=WinSys.x11.xlib.CreateWindow(WinSys.x11.display,WinSys.x11.root,xpos,ypos,width,height,0,depth,1/*output only*/,visual,(1L<<3)/*border pixel*/|(1L<<13)/*colormap*/|(1L<<11)/*event mask*/,&wa);
        WinSys.x11.xlib.SaveContext(WinSys.x11.display,win->x11.handle,WinSys.x11.context,(XPointer)win); // Needed to allow input.
        Atom protocols[]={WinSys.x11.WM_DELETE_WINDOW,WinSys.x11.NWM_PING};
        WinSys.x11.xlib.SetWMProtocols(WinSys.x11.display,win->x11.handle,protocols,sizeof(protocols)/sizeof(Atom));
        if (WinSys.x11.NWM_WINDOW_TYPE && WinSys.x11.NWM_WINDOW_TYPE_NORMAL) { Atom type=WinSys.x11.NWM_WINDOW_TYPE_NORMAL;
        WinSys.x11.xlib.ChangeProperty(WinSys.x11.display,win->x11.handle,WinSys.x11.NWM_WINDOW_TYPE,((Atom) 4),32,0/*PropModeReplace*/,(u8*)&type,1); }
        XSizeHints* szhints=WinSys.x11.xlib.AllocSizeHints();
        szhints->flags|=((1L << 4)/*PMinSize*/|(1L << 5)/*PMaxSize*/); szhints->min_width=szhints->max_width=width; szhints->min_height=szhints->max_height=height; szhints->flags|=(1L << 9)/*PWinGravity*/; szhints->win_gravity=10/*static gravity*/;
        WinSys.x11.xlib.SetWMNormalHints(WinSys.x11.display,win->x11.handle,szhints); WinSys.x11.xlib.Free(szhints);
        WinSys.x11.xlib.ChangeProperty(WinSys.x11.display,win->x11.handle,WinSys.x11.NWM_NAME,WinSys.x11.UTF8_STRING,8,0/*PropModeReplace*/,(u8*)title,slen(title)); // Set title
        GetWindowPos(win,&win->x11.xpos,&win->x11.ypos); GetWindowSize(win,&win->x11.width,&win->x11.height);
        int attribs[40],index=0; attribs[index++] = 0x2091/*major*/; attribs[index++] = 4; attribs[index++] = 0x2092/*minor*/; attribs[index++] = 3; /*OpenGL 4.3*/ attribs[index++] = 0x9126/*profile mask arb*/; attribs[index++] = 1/*core profile*/; attribs[index++] = 0L; attribs[index++] = 0L;
        win->context.glx.handle     = WinSys.glx.CreateContextAttribsARB(WinSys.x11.display,native,NULL,1,attribs); win->context.glx.window  = WinSys.glx.CreateWindow(WinSys.x11.display,native,win->x11.handle,NULL);
        win->context.glx.fbconfig   = native; win->context.makeCurrent = makeContextCurrentGLX;                   win->context.swapBuffers = swapBuffersGLX; win->context.swapInterval = swapIntervalGLX;
        win->context.getProcAddress = getProcAddressGLX; WinSys.x11.xlib.MapWindow(WinSys.x11.display,win->x11.handle);
        if (WinSys.x11.NET_ACTIVE_WINDOW) sendEventToWM(win,WinSys.x11.NET_ACTIVE_WINDOW,1,0,0,0,0);
        else if (WindowVisible()) { WinSys.x11.xlib.RaiseWindow(WinSys.x11.display,win->x11.handle); WinSys.x11.xlib.SetInputFocus(WinSys.x11.display,win->x11.handle,2/*RevertToParent*/,0L); }
        return (WinSyswindow*)win;
    }
#endif
WinSyslibrary WinSys={0};
int WindowInit() {
    mset(&WinSys,0,sizeof(WinSys));
    #if defined(WINDOWS)
        GetModuleHandleExW(0x4|0x2,(const u16*)&WinSys,(HMODULE*)&WinSys.win32.instance);
        const char* names[] = {"xinput1_4.dll","xinput1_3.dll","xinput9_1_0.dll","xinput1_2.dll","xinput1_1.dll",NULL};
        for (int i=0;names[i];++i) {
            WinSys.win32.xinput.instance = LoadLibraryA(names[i]);
            if (WinSys.win32.xinput.instance) { WinSys.win32.xinput.GetCapabilities = (PFN_XInputGetCapabilities)PlatformGetModuleSymbol(WinSys.win32.xinput.instance, "XInputGetCapabilities"); WinSys.win32.xinput.GetState = (PFN_XInputGetState)PlatformGetModuleSymbol(WinSys.win32.xinput.instance, "XInputGetState"); break; }
        }
        WinSys.win32.dwmapi.instance = LoadLibraryA("dwmapi.dll");
        if (WinSys.win32.dwmapi.instance) { WinSys.win32.dwmapi.IsCompositionEnabled = (PFN_DwmIsCompositionEnabled)PlatformGetModuleSymbol(WinSys.win32.dwmapi.instance, "DwmIsCompositionEnabled"); WinSys.win32.dwmapi.Flush = (PFN_DwmFlush)PlatformGetModuleSymbol(WinSys.win32.dwmapi.instance, "DwmFlush"); }
        WinSys.win32.ntdll.instance = LoadLibraryA("ntdll.dll");
        if (WinSys.win32.ntdll.instance) WinSys.win32.ntdll.RtlVerifyVersionInfo = (PFN_RtlVerifyVersionInfo)PlatformGetModuleSymbol(WinSys.win32.ntdll.instance, "RtlVerifyVersionInfo");
        createKeyTables();
        MSG msg; WNDCLASSEXW wc={0}; wc.cbSize=sizeof(wc); // Start making of a helper window
        wc.style = 0x0020/*CS_OWNDC*/; wc.lpfnWndProc = (WNDPROC)helperWindowProc; wc.hInstance = WinSys.win32.instance; wc.lpszClassName = L"WinSys3 Helper";
        WinSys.win32.helperWindowClass = RegisterClassExW(&wc);
        WinSys.win32.helperWindowHandle = CreateWindowExW(0x00000100/*WS_EX_WINDOWEDGE*/ | 0x00000200/*WS_EX_CLIENTEDGE*/,(u16*)MAKEINTATOM(WinSys.win32.helperWindowClass),L"WinSys message window",0x04000000/*WS_CLIPSIBLINGS*/|0x02000000/*WS_CLIPCHILDREN*/,0,0,1,1,NULL,NULL,WinSys.win32.instance,NULL);
        ShowWindow(WinSys.win32.helperWindowHandle,0);
        DEV_BROADCAST_DEVICEINTERFACE_W dbi={0}; dbi.dbcc_size = sizeof(dbi); dbi.dbcc_devicetype = 0x0005/*DBT_DEVTYP_DEVICEINTERFACE*/; dbi.dbcc_classguid = (GUID){0x4d1e55b2,0xf16f,0x11cf,{0x88,0xcb,0x00,0x11,0x11,0x00,0x00,0x30}};
        WinSys.win32.deviceNotificationHandle = RegisterDeviceNotificationW(WinSys.win32.helperWindowHandle,(DEV_BROADCAST_HDR*)&dbi,0);
        while (PeekMessageW(&msg, WinSys.win32.helperWindowHandle,0,0,0x0001)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
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
        static XContext lastContext = 0;
        WinSys.x11.context = ++lastContext;
        WinSys.x11.randr.handle = WinSysPlatformLoadModule("libXrandr.so.2");
        #define X(n) WinSys.x11.randr.n = (PFN_XRR##n)PlatformGetModuleSymbol(WinSys.x11.randr.handle,"XRR"#n);
            X(FreeCrtcInfo) X(FreeOutputInfo) X(FreeScreenResources) X(GetCrtcInfo) X(GetOutputInfo) X(GetOutputPrimary) X(GetScreenResourcesCurrent) X(SelectInput) X(UpdateConfiguration)
        #undef X
        XRRScreenResources* sr = WinSys.x11.randr.GetScreenResourcesCurrent(WinSys.x11.display,WinSys.x11.root);
        WinSys.x11.randr.FreeScreenResources(sr); WinSys.x11.randr.SelectInput(WinSys.x11.display,WinSys.x11.root,(1L << 2)/*change notify mask*/); void* xcurhandle = WinSysPlatformLoadModule("libXcursor.so.1");
        PFN_XCIC ImageCreate = (PFN_XCIC)PlatformGetModuleSymbol(xcurhandle,"XcursorImageCreate"); PFN_XCID ImageDestroy = (PFN_XCID)PlatformGetModuleSymbol(xcurhandle,"XcursorImageDestroy"); PFN_XCILC ImageLoadCursor = (PFN_XCILC)PlatformGetModuleSymbol(xcurhandle,"XcursorImageLoadCursor");
        createKeyTables();
        #define IA(n) WinSys.x11.xlib.InternAtom(WinSys.x11.display,n,0)
            WinSys.x11.UTF8_STRING=IA("UTF8_STRING");  WinSys.x11.WM_PROTOCOLS=IA("WM_PROTOCOLS"); WinSys.x11.WM_STATE   =IA("WM_STATE");             WinSys.x11.WM_DELETE_WINDOW=IA("WM_DELETE_WINDOW");          WinSys.x11.NET_SUPPORTED =IA("_NET_SUPPORTED"); WinSys.x11.NET_SUPPORTING_WM_CHECK=IA("_NET_SUPPORTING_WM_CHECK");
            WinSys.x11.NWM_ICON=IA("_NWM_ICON"); WinSys.x11.NWM_PING =IA("_NWM_PING"); WinSys.x11.NWM_NAME=IA("_NWM_NAME"); WinSys.x11.NWM_BYPASS_COMPOSITOR=IA("_NWM_BYPASS_COMPOSITOR"); WinSys.x11.MOTIF_WM_HINTS=IA("_MOTIF_WM_HINTS");
        #undef IA
        Window* wfr = NULL; WinSysGetWindowPropertyX11(WinSys.x11.root,WinSys.x11.NET_SUPPORTING_WM_CHECK,((Atom) 33),(u8**)&wfr);
        Window* wfc = NULL; WinSysGetWindowPropertyX11(*wfr,WinSys.x11.NET_SUPPORTING_WM_CHECK,((Atom) 33),(u8**)&wfc);
        WinSys.x11.xlib.Free(wfr); WinSys.x11.xlib.Free(wfc); Atom* sa = NULL; const unsigned long ac = WinSysGetWindowPropertyX11(WinSys.x11.root,WinSys.x11.NET_SUPPORTED,((Atom) 4),(u8**)&sa);
        #define GA(name) getAtomIfSupported(sa, ac, name)
            WinSys.x11.NWM_STATE=GA("_NWM_STATE"); WinSys.x11.NWM_STATE_FULLSCREEN=GA("_NWM_STATE_FULLSCREEN"); WinSys.x11.NWM_WINDOW_TYPE=GA("_NWM_WINDOW_TYPE"); WinSys.x11.NWM_WINDOW_TYPE_NORMAL=GA("_NWM_WINDOW_TYPE_NORMAL"); WinSys.x11.NET_WORKAREA=GA("_NET_WORKAREA"); WinSys.x11.NET_CURRENT_DESKTOP=GA("_NET_CURRENT_DESKTOP"); WinSys.x11.NET_ACTIVE_WINDOW=GA("_NET_ACTIVE_WINDOW");
        #undef GA
        if (sa) WinSys.x11.xlib.Free(sa);
        XSetWindowAttributes wa; wa.event_mask = (1L<<22); WinSys.x11.xlib.CreateWindow(WinSys.x11.display,WinSys.x11.root,0,0,1,1,0,0,2/*input only*/,(&((_XPrivDisplay)(WinSys.x11.display))->screens[WinSys.x11.screen])->root_visual,(1L<<11)/*event mask*/,&wa);
        XcursorImage* native = ImageCreate(16,16); mset(native->pixels,0,256*sizeof(XcursorUInt)); native->xhot=native->yhot=0; WinSys.x11.hiddenCursorHandle = ImageLoadCursor(WinSys.x11.display,native); ImageDestroy(native);
        PollMonitors();
    #endif
    return  1;
}

const WinSysfbconfig* ChooseFBConfig(const WinSysfbconfig* alts, u32 count) {
    u32 missing, leastMissing = 2147483647, colorDiff, leastColorDiff = 2147483647, extraDiff, leastExtraDiff = 2147483647;
    const WinSysfbconfig* closest = NULL;
    for (u32 i = 0; i < count; i++) {
        const WinSysfbconfig* c = alts + i; missing = (c->alphaBits==0)+(c->depthBits==0)+(c->stencilBits==0);
        colorDiff = 0; colorDiff+=(8-c->redBits)  *(8-c->redBits);   colorDiff+=(8-c->greenBits)*(8-c->greenBits); colorDiff+=(8-c->blueBits)   *(8-c->blueBits);
        extraDiff = 0; extraDiff+=(8-c->alphaBits)*(8-c->alphaBits); extraDiff+=(8-c->depthBits)*(8-c->depthBits); extraDiff+=(8-c->stencilBits)*(8-c->stencilBits);
        if (missing < leastMissing || (missing == leastMissing && (colorDiff < leastColorDiff || (colorDiff == leastColorDiff && extraDiff < leastExtraDiff)))) closest = c;
        if (c == closest) { leastMissing=missing; leastColorDiff=colorDiff; leastExtraDiff=extraDiff; }
    }
    return closest;
}

void SetGLContext_GetFunctionPointers() {
    WinSyswindow* handle=(WinSyswindow*)window; handle->context.makeCurrent(handle);
    #define X(n,t) n=(t)handle->context.getProcAddress(#n);
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
void InputMonitor(WinSysmonitor* monitor, int action, int placement) {
    if (action == 0x00040001/*connected*/) {
        WinSys.monitorCount++;
        WinSys.monitors = WinSys.monitors ? OS_Realloc(WinSys.monitors,monitorAllocationSize,sizeof(WinSysmonitor*) * WinSys.monitorCount) : OS_Alloc(WinSys.monitorCount * sizeof(WinSysmonitor*));
        monitorAllocationSize = WinSys.monitorCount * sizeof(WinSysmonitor*);
        if (placement == 0) { mmov(WinSys.monitors + 1,WinSys.monitors,((size_t) WinSys.monitorCount - 1) * sizeof(WinSysmonitor*)); WinSys.monitors[0] = monitor; }
        else WinSys.monitors[WinSys.monitorCount - 1] = monitor;
    } else if (action == 0x00040002/*disconnected*/) {
        for (int i=0;i<WinSys.monitorCount;++i) {
            if (WinSys.monitors[i] == monitor) { WinSys.monitorCount--; mmov(WinSys.monitors + i, WinSys.monitors + i + 1,((size_t) WinSys.monitorCount - i) * sizeof(WinSysmonitor*)); break; }
        }
    }
}

WinSysmonitor* AllocMonitor(const char* n, int w, int h) { WinSysmonitor* monitor = OS_Calloc(1, sizeof(WinSysmonitor)); monitor->widthMM = w; monitor->heightMM = h; scpy_to_a_from_b(monitor->name,n,sizeof(monitor->name)); return monitor; }
WinSysmonitor** WinSysGetMonitors(int* count) { *count = WinSys.monitorCount; return (WinSysmonitor**) WinSys.monitors; }
WinSysmonitor* GetPrimaryMonitor(void) { if (!WinSys.monitorCount) {return NULL;} return (WinSysmonitor*) WinSys.monitors[0]; }
void WinSysGetMonitorPos(WinSysmonitor* handle, int* xpos, int* ypos) { *xpos = 0; *ypos = 0; WinSysmonitor* monitor = (WinSysmonitor*)handle; GetMonitorPos(monitor,xpos,ypos); }
void WinSysGetMonitorWorkarea(WinSysmonitor* handle, int* xpos, int* ypos, int* width, int* height) { *xpos=*ypos=*width=*height=0; WinSysmonitor* monitor = (WinSysmonitor*)handle; GetMonitorWorkarea(monitor,xpos,ypos,width,height); }
const vidmode* WinSysGetVideoMode(WinSysmonitor* handle) { WinSysmonitor* monitor=(WinSysmonitor*)handle; GetVideoMode(monitor,&monitor->currentMode); return &monitor->currentMode; }
void InputWindowFocus(i32 f) { window_has_focus = f != 0; ignore_next_mouse_delta = true; WinSyswindow* win = (WinSyswindow*)window; if (!f) { for (int k=0;k<=348;++k) { if (win->keys[k] == INPUT_PRESS) {InputKey(win,k,INPUT_RELEASE);} } for (int b=0;b<=  7;++b) { if (win->mouseButtons[b] == INPUT_PRESS) {InputMouseClick(win,b,INPUT_RELEASE);} } } }
void VSetWindowIcon(WinSysIcon* images) { SetWindowIcon(images); }
void VSetWindowSize(int w, int h) { WinSyswindow* win = (WinSyswindow*)window; win->videoMode.width=w; win->videoMode.height=h; SetWindowSize(win,w,h); }
void WinSysSetWindowMonitor(int xpos, int ypos, int width, int height) { WinSyswindow* win = (WinSyswindow*)window; win->videoMode.width=width; win->videoMode.height=height; SetWindowMonitor(win,xpos,ypos,width,height); }
InputElement inputElements[134]={{"A",KEY_A},{"B",KEY_B},{"C",KEY_C},{"D",KEY_D},{"E",KEY_E},{"F",KEY_F},{"G",KEY_G},{"H",KEY_H},{"I",KEY_I},{"J",KEY_J},{"K",KEY_K},{"L",KEY_L},{"M",KEY_M},{"N",KEY_N},{"O",KEY_O},{"P",KEY_P},{"Q",KEY_Q},{"R",KEY_R},{"S",KEY_S},{"T",KEY_T},{"U",KEY_U},{"V",KEY_V},{"W",KEY_W},{"X",KEY_X},{"Y",KEY_Y},{"Z",KEY_Z},
                                 {"1",KEY_1},{"2",KEY_2},{"3",KEY_3},{"4",KEY_4},{"5",KEY_5},{"6",KEY_6},{"7",KEY_7},{"8",KEY_8},{"9",KEY_9},{"0",KEY_0},{"UPARROW",KEY_UP},{"DNARROW",KEY_DOWN},{"LFARROW",KEY_LEFT},{"RTARROW",KEY_RIGHT},{"NUM1",KEY_KP_1},{"NUM2",KEY_KP_2},{"NUM3",KEY_KP_3},{"NUM+",KEY_KP_ADD},{"ENTER",KEY_ENTER},
                                 {"RIGHTSHIFT",KEY_RIGHT_SHIFT},{"LEFTSHIFT",KEY_LEFT_SHIFT},{"RIGHTCTRL",KEY_RIGHT_CONTROL},{"LEFTCTRL",KEY_LEFT_CONTROL},{"RIGHTALT",KEY_RIGHT_ALT},{"LEFTALT",KEY_LEFT_ALT},{"RIGHTCMD",KEY_RIGHT_SUPER},{"LEFTCMD",KEY_LEFT_SUPER},
                                 {"LMB",MOUSE_BUTTON_1},{"RMB",MOUSE_BUTTON_2},{"MMB",MOUSE_BUTTON_3},{"MB3",MOUSE_BUTTON_4},{"MB4",MOUSE_BUTTON_5},{"MB5",MOUSE_BUTTON_6},{"MB6",MOUSE_BUTTON_7},{"MB7",MOUSE_BUTTON_8},{"JOY0",JOYSTICK_1},{"JOY1",JOYSTICK_2},{"JOY2",JOYSTICK_3},{"JOY3",JOYSTICK_4},{"JOY4",JOYSTICK_5},{"JOY5",JOYSTICK_6},
                                 {"JOY6",JOYSTICK_7},{"JOY7",JOYSTICK_8},{"JOY8",JOYSTICK_9},{"JOY9",JOYSTICK_10},{"JOY10",JOYSTICK_11},{"JOY11",JOYSTICK_12},{"JOY12",JOYSTICK_13},{"JOY13",JOYSTICK_14},{"JOY14",JOYSTICK_15},{"JOY15",JOYSTICK_16},{"JOY16",JOYHAT_UP},{"JOY17",JOYHAT_RIGHT},
                                 {"BACKSPACE",KEY_BACKSPACE},{"TAB",KEY_TAB},{"NUMENTER",KEY_KP_ENTER},{"ESCAPE",KEY_ESCAPE},{"SPACE",KEY_SPACE},{"DELETE",KEY_DELETE},{"INSERT",KEY_INSERT},{"HOME",KEY_HOME},{"END",KEY_END},{"PAGEUP",KEY_PAGE_UP},{"PAGEDN",KEY_PAGE_DOWN},
                                 {"F1",KEY_F1},{"F2",KEY_F2},{"F3",KEY_F3},{"F4",KEY_F4},{"F5",KEY_F5},{"F6",KEY_F6},{"F7",KEY_F7},{"F8",KEY_F8},{"F9",KEY_F9},{"F10",KEY_F10},{"F11",KEY_F11},{"F12",KEY_F12},{"GRAVE",KEY_GRAVE_ACCENT},{"-",KEY_MINUS},{"=",KEY_EQUAL},{"[",KEY_LEFT_BRACKET},{"]",KEY_RIGHT_BRACKET},{"\\",KEY_BACKSLASH},{"/",KEY_SLASH},
                                 {".",KEY_PERIOD},{",",KEY_COMMA},{";",KEY_SEMICOLON},{"'",KEY_APOSTROPHE},{"CAPSLOCK",KEY_CAPS_LOCK},{"NUM0",KEY_KP_0},{"NUM4",KEY_KP_4},{"NUM5",KEY_KP_5},{"NUM6",KEY_KP_6},{"NUM7",KEY_KP_7},{"NUM8",KEY_KP_8},{"NUM9",KEY_KP_9},{"NUM*",KEY_KP_MULTIPLY},{"NUM-",KEY_KP_SUBTRACT},{"NUM.",KEY_KP_DECIMAL},{"MENU",KEY_MENU},
                                 {"PAUSE",KEY_PAUSE},{"NUMLOCK",KEY_NUM_LOCK},{"MWHEEL+",128},{"MWHEEL-",129},/*128,129,Handledspecialcaseformousewheel+/-respectively*/{"PRINT",KEY_PRINT_SCREEN},{"JOY18",JOYHAT_DOWN},{"JOY19",JOYHAT_LEFT},{"UNUSED",0}};
KeyState* GetCodeMapping(int settingIndex) {
    i32 i = Sys_Settings.InputCodeSettings[settingIndex]; // Get table index into all recognized inputs
    if (i == 148 || i >= MAX_KEYS) return &Sys_Input.keyStates[MAX_KEYS - 1]; // UNUSED NULL (e.g. setting unbound)
    if (i >= 53 && i <= 61) return &Sys_Input.mouseButtons[inputElements[i].value];
    if (i >= 62 && i <= 77) return &Sys_Input.joystickButtons[JOYSTICK_1][inputElements[i].value];
    if ((i >= 78 && i <= 79) || (i >= 132 && i <= 133)) return &Sys_Input.joystickHats[inputElements[i].value];
    return &Sys_Input.keyStates[inputElements[i].value];
}

bool GetKeyRiseEdgeOrHeld(int sI, bool onRise) { i32 i = Sys_Settings.InputCodeSettings[sI]; if (i == 128) {return Sys_Input.scrollDelta > 0;} if (i == 129) {return Sys_Input.scrollDelta < 0;} KeyState* k = GetCodeMapping(sI); return onRise ? k->pressed : k->down; }
ENGINE_TO_MOD bool GetKey(int settingIndex) { return GetKeyRiseEdgeOrHeld(settingIndex,false); }  // True while held down.
ENGINE_TO_MOD bool GetKeyPressed(int settingIndex) { return (settingIndex < 0) ? Sys_Input.keyStates[KEY_GRAVE_ACCENT].pressed : GetKeyRiseEdgeOrHeld(settingIndex,true); } // True 1st frame down.
ENGINE_TO_MOD void IgnoreNextMouseDelta() { ignore_next_mouse_delta = true; }
void TextEntry(i32 k) {
    if (k == KEY_U && Sys_Input.keyStates[KEY_LEFT_CONTROL].down) { World.playerName[0] = '\0'; currentPlayerNameLength = 0; return; }
    if (k == KEY_ENTER || k == KEY_KP_ENTER) { currentMenuItem++; return; }
    if (k == KEY_BACKSPACE && currentPlayerNameLength > 0) { World.playerName[--currentPlayerNameLength] = '\0'; return; }
    if (currentPlayerNameLength >= 26) return;
    char c = (k >= KEY_A && k <= KEY_Z) ? 'a' + (k - KEY_A) : ((k >= KEY_1 && k <= KEY_9) ? '1' + (k - KEY_1) : ((k == KEY_0) ? '0' : ((k == KEY_SPACE) ? ' ' : 0)));
    if (c) { World.playerName[currentPlayerNameLength] = c; World.playerName[++currentPlayerNameLength] = '\0'; }
}

void GoIntoGame(); void ConsoleEmulator(i32 keycode); extern bool enteringPlayerName;
void InputKey(WinSyswindow* win,int key,int action) {
    if (key >= 0 && key <= 348) {
        i32 repeated = 0;
        if (action == INPUT_RELEASE && win->keys[key] == INPUT_RELEASE) return;
        if (action ==   INPUT_PRESS && win->keys[key] == INPUT_PRESS) repeated =  1;
        win->keys[key] = (char)action; if (repeated) action = INPUT_REPEAT;
    }
    if (!window_has_focus) return;
    if (key == KEY_F10 && action) OS_Exit(0); // Suppress warnings about unused parameters forced upon me by WinSys3 dependency deadweight anchor.
    if (World.menuActive && !returnToPause) {
        if ((key == KEY_RIGHT_ALT || key == KEY_LEFT_ALT) && action && Sys_Input.keyStates[KEY_ENTER].down)                    GoIntoGame();
        if (key == KEY_ENTER && action && (Sys_Input.keyStates[KEY_LEFT_ALT].down || Sys_Input.keyStates[KEY_RIGHT_ALT].down)) GoIntoGame();
    }
    if (key >=0 && key < MAX_KEYS && (action == INPUT_PRESS || (action == INPUT_REPEAT && !(key == KEY_KP_ENTER || key == KEY_ENTER || key == KEY_TAB || key == KEY_ESCAPE)))) {
        Sys_Input.keyStates[key].pressed = Sys_Input.keyStates[key].down = true;
        if (Cheats.consoleActive) ConsoleEmulator(key);
        else if (enteringPlayerName && World.menuActive) TextEntry(key);
    } else if (key >= 0 && key < MAX_KEYS && action == INPUT_RELEASE) Sys_Input.keyStates[key].pressed = Sys_Input.keyStates[key].down = false;
}

void InputMouseClick(WinSyswindow* win, int button, int action) { if (button<0 || button>7) {return;} if (button<=7) {win->mouseButtons[button] = (char)action;} Sys_Input.mouseButtons[button].down = Sys_Input.mouseButtons[button].pressed = (action == 1); Sys_Input.mouseButtons[button].released = (action == 0); }
void quat_from_yaw_pitch_roll(Quaternion* q, float yaw_deg, float pitch_deg, float roll_deg) {
    float yaw = deg2rad(yaw_deg), pitch = deg2rad(pitch_deg), roll = deg2rad(roll_deg);  // Around Z (forward)
    float cy = vcosf(yaw * 0.5f), sy = vsinf(yaw * 0.5f), cp = vcosf(pitch * 0.5f), sp = vsinf(pitch * 0.5f), cr = vcosf(roll * 0.5f), sr = vsinf(roll * 0.5f);
    q->w = cy*cp*cr + sy*sp*sr; q->x = cy*sp*cr + sy*cp*sr; /* X-axis (pitch) */ q->y = sy*cp*cr - cy*sp*sr; /* Y-axis (yaw) */ q->z = cy*cp*sr - sy*sp*cr; /* Z-axis (roll) */ // Skipping quat normalization, not needed
} 

float cam_pitch,cam_yaw=90.0f,cam_roll;
void InputCursorPos(WinSyswindow* win, double xpos, double ypos) { // static const float HeadBobRate   = 0.2f, HeadBobAmount = 0.08f,bobTarget = 0.3f; TODO
    if (win->virtualCursorPosX == xpos && win->virtualCursorPosY == ypos) return;
    win->virtualCursorPosX = xpos; win->virtualCursorPosY = ypos; if (!window_has_focus) return;
    currentMouse_dx = (i32)(xpos - last_mouse_x); currentMouse_dy = (i32)(ypos - last_mouse_y); last_mouse_x = xpos; last_mouse_y = ypos; if (ignore_next_mouse_delta) { ignore_next_mouse_delta = mouseMovementThisFrame = false; return; }
    if ((World.inventoryMode && !Cheats.noHUD) || World.menuActive || World.paused) { // Uses UI baseline resolution 1366x768
        i32 newX = clamp(World.cursorPosition_x + currentMouse_dx,0,1366); if (newX != World.cursorPosition_x) {mouseMovementThisFrame = true;} World.cursorPosition_x = newX;
        i32 newY = clamp(World.cursorPosition_y + currentMouse_dy,0, 768); if (newY != World.cursorPosition_y) {mouseMovementThisFrame = true;} World.cursorPosition_y = newY;
    }
    if (World.paused || World.menuActive || World.inventoryMode) return;
    float s = vclamp((float)Sys_Settings.MouseSensitivity / 100.0f, 0.01f, 1.0f) * 0.2f;
    cam_yaw += (float)currentMouse_dx * s; if (cam_yaw >= 360.0f) {cam_yaw -= 360.0f;} if (cam_yaw < 0.0f)     {cam_yaw  += 360.0f;}
    cam_pitch+=(float)currentMouse_dy * s; if (cam_pitch > 89.0f) {cam_pitch = 89.0f;} if (cam_pitch < -89.0f) {cam_pitch = -89.0f;} // Avoid gimbal lock at pure 90deg
    quat_from_yaw_pitch_roll(&World.rotation[PLAYER1],cam_yaw,cam_pitch,(World.curLev == LEVEL_CYBERSPACE) ? cam_roll : 0.0f);
}

void JoystickConnection(WinSysjoystick* js, int e) {
    js->connected = (e == 0x00040001/*connected*/) ? 1 : (e == 0x00040002/*disconnected*/) ? 0 : js->connected;    
    int jid = (int)(js - WinSys.joysticks); if (jid > JOYSTICK_LAST) return;
    Sys_Input.joystickPresent[jid] = (e == 0x00040001/*connected*/); if (!Sys_Input.joystickPresent[jid]) { mset(Sys_Input.joystickButtons,0,sizeof(Sys_Input.joystickButtons)); mset(Sys_Input.joystickHats,0,sizeof(Sys_Input.joystickHats)); } // Clear
}

void InputJoystickAxis(WinSysjoystick* js,int axis, float value) { js->axes[axis] = value; }
void InputJoystickButton(WinSysjoystick* js,int button, char value) { js->buttons[button] = value; }
void InputJoystickHat(WinSysjoystick* js, int hat, char value) { int base=js->buttonCount + hat * 4; js->buttons[base+0]=(value & 0x01) ? INPUT_PRESS : INPUT_RELEASE; js->buttons[base+1]=(value & 0x02) ? INPUT_PRESS : INPUT_RELEASE; js->buttons[base+2]=(value & 0x04) ? INPUT_PRESS : INPUT_RELEASE; js->buttons[base+3]=(value & 0x08) ? INPUT_PRESS : INPUT_RELEASE; js->hats[hat]=value; }
WinSysjoystick* WinSysAllocJoystick(const char* name,const char* guid,int axisCount,int buttonCount,int hatCount) {
    int jid; WinSysjoystick* js;
    for (jid = 0; jid <= JOYSTICK_LAST; jid++) { if (!WinSys.joysticks[jid].allocated) break; }
    if (jid > JOYSTICK_LAST) return NULL;
    js = WinSys.joysticks + jid;
    js->allocated = 1; js->axisCount = axisCount; js->buttonCount = buttonCount; js->hatCount = hatCount;
    js->axesSize = axisCount*sizeof(float); js->axes = OS_Calloc(axisCount,sizeof(float)); js->buttonsSize = (buttonCount + (size_t)hatCount * 4);
    js->buttons = OS_Calloc(buttonCount + (size_t)hatCount * 4,1); js->hatsSize = hatCount; js->hats = OS_Calloc(hatCount,1);
    scpy_to_a_from_b(js->name,name,sizeof(js->name)); scpy_to_a_from_b(js->guid,guid,sizeof(js->guid));
    return js;
}

bool JoystickPresent(int jid) { if (jid < 0 || jid > JOYSTICK_LAST || (!WinSys.joysInited && !InitJoysticks())) {return false;} WinSys.joysInited = 1; WinSysjoystick* js = WinSys.joysticks + jid; return js->connected ? PollJoystick(js) : false; }
void FreeJoystick(WinSysjoystick* js) { OS_Free(js->axes,js->axesSize); OS_Free(js->buttons,js->buttonsSize); OS_Free(js->hats,js->hatsSize); mset(js,0,sizeof(WinSysjoystick)); }
void play_synth_laser(float volume,float freq,float sweep,float fmrate,float decay);
void play_synth_door(float volume,float pitch);
void play_synth_impact(float volume,float ring_freq,float decay,float noise_amt,float ring_amt);
void InputProcessing() {
    mouseMovementThisFrame = false; PollEvents();
    for (int jid = JOYSTICK_1; jid <= JOYSTICK_LAST; ++jid) { // Input Poll
        if (!JoystickPresent(jid)) continue;
        WinSysjoystick* js = WinSys.joysticks + jid; if (!js->connected) continue;
        PollJoystick(js); int totalButtons = js->buttonCount + js->hatCount * 4;
        for (int i = 0; i < totalButtons && i < 16; ++i) { KeyState* k = &Sys_Input.joystickButtons[jid - JOYSTICK_1][i]; bool down = js->buttons[i] == INPUT_PRESS; k->pressed = down && !k->down; k->released = !down && k->down; k->down = down; }
        for (int i = 0; i < js->hatCount && i < 5; ++i) { Sys_Input.joystickHats[i].down = js->hats[i]; }
//         for (int i = 0; i < js->axisCount && i < MAX_JOYSTICK_AXES; ++i) { Sys_Input.joystickAxes[jid - JOYSTICK_1][i] = js->axes[i]; } TODO??
    }

    float v = 0.1f;
    if (Sys_Input.keyStates[KEY_E].pressed) play_wav("./Audio/cyborgs/yourlevelsareterrible.wav",0.1f,(V3){},false);
    if (Sys_Input.keyStates[KEY_W].pressed) play_synth_door(v,50); // thud slide
    if (Sys_Input.keyStates[KEY_T].pressed) play_synth_impact(v,4500,18,0.3f,0.6f); // Glass ting
    if (Sys_Input.keyStates[KEY_R].pressed) play_synth_impact(v,1800,30,0.5f,0.3f); // cartridge drop
    if (Sys_Input.keyStates[KEY_Y].pressed) play_synth_laser(v,800,-2.0f,40,12);
    if (Sys_Input.keyStates[KEY_U].pressed) play_synth_laser(v,800,2.0f,40,12);
    if (window_has_focus) { if(Sys_Input.keyStates[KEY_CAPS_LOCK].pressed){Sys_Input.isCapsLockOn=!Sys_Input.isCapsLockOn;} ProcessInput(); }
}

void SetVSync() { ((WinSyswindow*)window)->context.swapInterval((i32)Sys_Settings.Vsync); }
void ResetInput() { for (i32 i=0;i<MAX_KEYS;++i) {Sys_Input.keyStates[i].pressed = Sys_Input.keyStates[i].released = false;} for (i32 i=0;i<MAX_MOUSE_BUTTONS;i++) {Sys_Input.mouseButtons[i].pressed = Sys_Input.mouseButtons[i].released = false;} Sys_Input.scrollDelta = 0; currentMouse_dx = currentMouse_dy = 0; } // Can't memset as we want to preserve down state
void CenterWindowOnMonitor() {
    int c; WinSysmonitor** monitors = WinSysGetMonitors(&c); if (Sys_Settings.CurrentMonitor > (c - 1)) { Sys_Settings.CurrentMonitor = 0; SaveConfig(); }
    int mx,my; WinSysmonitor* next = monitors[Sys_Settings.CurrentMonitor]; WinSysGetMonitorPos(next,&mx,&my);
    const vidmode* mode = WinSysGetVideoMode(next); int xpos = mx + (mode->width - Sys_Settings.ScreenWidth) / 2, ypos = my + (mode->height - Sys_Settings.ScreenHeight) / 2;
    SetWindowPosition(window,xpos,ypos); ignore_next_mouse_delta = true;
}

WinSysmonitor* GetCurrentMonitor() {
    int wx=0,wy=0,ww=0,wh=0; GetWindowPos(((WinSyswindow*)window),&wx,&wy); GetWindowSize(((WinSyswindow*)window),&ww,&wh); WinSysmonitor* bestMonitor = GetPrimaryMonitor(); int bestArea=0,c; WinSysmonitor** monitors = WinSysGetMonitors(&c);
    for (int i=0;i<c;++i) {
        int mx,my; WinSysGetMonitorPos(monitors[i],&mx,&my); const vidmode* mode = WinSysGetVideoMode(monitors[i]);
        int left=vmax(wx,mx), right=vmin(wx + ww,mx + mode->width), top=vmax(wy,my), bottom=vmin(wy + wh,my + mode->height);
        int area = (right > left && bottom > top) ? (right - left) * (bottom - top) : 0;
        if (area > bestArea) { bestArea = area; bestMonitor = monitors[i]; }
    }
    return bestMonitor;
}

void ChangeResolution() {
    if (resDropdownCount < 1) return;
    resSelectedIdx = (resSelectedIdx + 1) % resDropdownCount; Sys_Settings.ScreenWidth = (u32)resModes[resSelectedIdx].w; Sys_Settings.ScreenHeight = (u32)resModes[resSelectedIdx].h;
    WinSysmonitor* monitor = GetCurrentMonitor(); if(!monitor){monitor=GetPrimaryMonitor();}
    int mx,my; WinSysGetMonitorPos(monitor,&mx,&my); const vidmode* desktop = WinSysGetVideoMode(monitor);
    int x = mx + (desktop->width - (int)Sys_Settings.ScreenWidth) / 2, y = my + (desktop->height - (int)Sys_Settings.ScreenHeight) / 2;
    VSetWindowSize((int)Sys_Settings.ScreenWidth,(int)Sys_Settings.ScreenHeight); SetWindowPosition(window,x,y); UpdateScreenSize((int)Sys_Settings.ScreenWidth,(int)Sys_Settings.ScreenHeight);
    resDropdownOpen = false; SaveConfig();
}

void GatherResolutionModes() {
    resDropdownCount = 0; WinSysmonitor* monitor = GetCurrentMonitor(); if (!monitor){monitor=GetPrimaryMonitor();} const vidmode* desktop = WinSysGetVideoMode(monitor); if(!desktop){return;}
    static const struct {int w,h;} commonRes[] = {{320,200},{640,400},{640,480},{800,600},{1024,768},{1280,720},{1280,800},{1366,768},{1440,900},{1600,900},{1920,1080},{2560,1440}};
    int maxW = desktop->width, maxH = desktop->height,j;
    for (int i = 0; i < (int)(sizeof(commonRes)/sizeof(commonRes[0])) && resDropdownCount < 8; ++i) {
        if (commonRes[i].w > maxW || commonRes[i].h > maxH || commonRes[i].w < 320 || commonRes[i].h < 200) continue;
        for (j = 0; j < resDropdownCount; ++j) { if (resModes[j].w == commonRes[i].w && resModes[j].h == commonRes[i].h) {break;} }
        if (j == resDropdownCount) resModes[resDropdownCount++] = (ResMode){commonRes[i].w,commonRes[i].h};
    }
    if (resDropdownCount < 8) resModes[resDropdownCount++] = (ResMode){desktop->width,desktop->height};
    resSelectedIdx = 0;
    for (int i = 0; i < resDropdownCount; ++i) { if(resModes[i].w == (int)Sys_Settings.ScreenWidth && resModes[i].h == (int)Sys_Settings.ScreenHeight){resSelectedIdx=i; break;} }
}

void ChangeFullScreenWindowed() {
    int x,y,w,h,mx,my,monitorCount; WinSysmonitor** monitors = WinSysGetMonitors(&monitorCount); WinSysmonitor* monitor = monitors[Sys_Settings.CurrentMonitor]; const vidmode* mode = WinSysGetVideoMode(monitor); WinSysGetMonitorWorkarea(monitor,&x,&y,&w,&h);
    ((WinSyswindow*)window)->decorated = (i32)(!Sys_Settings.Fullscreen); SetWindowDecorated(((WinSyswindow*)window),(i32)(!Sys_Settings.Fullscreen));
    if (Sys_Settings.Fullscreen) {WinSysSetWindowMonitor(x,y,w,h); Sys_Settings.ScreenWidth = w; Sys_Settings.ScreenHeight = h;}
    else { WinSysGetMonitorPos(monitor,&mx,&my); Sys_Settings.ScreenWidth = vmax(vmin((w*3)/4,1366),320); Sys_Settings.ScreenHeight = vmax(vmin((h*3)/4,768),200); WinSysSetWindowMonitor(mx + (mode->width - Sys_Settings.ScreenWidth) / 2,my + (mode->height - Sys_Settings.ScreenHeight) / 2,Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight); }
    UpdateScreenSize(Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight);
}

static double monitorSwitchTime;
void CycleToNextMonitor() {
    if (get_time() < monitorSwitchTime) return;
    monitorSwitchTime = get_time() + 0.5; // Prevent toggling rapidly on accident
    int monitorCount; WinSysmonitor** monitors = WinSysGetMonitors(&monitorCount);
    if (Sys_Settings.CurrentMonitor > (monitorCount - 1)) { Sys_Settings.CurrentMonitor = 0; SaveConfig(); }
    if (!monitors || monitorCount < 2) return;
    Sys_Settings.CurrentMonitor = (Sys_Settings.CurrentMonitor + 1) % monitorCount;
    SaveConfig(); CenterWindowOnMonitor();
}

char statusText[T_BUFFER_SIZE];
void CenterStatusPrint(const char * restrict fmt, ...) { va_list args; __builtin_va_start(args, fmt); sFormatV(statusText,T_BUFFER_SIZE,fmt,args); __builtin_va_end(args); DualLog("%s\n",statusText); World.statusTextDecayFinished = get_time() + 3.5;/*secs decay time before text dissappears.*/ }
// Configuration Options Settings Sys
typedef enum { SETTING_U8, SETTING_U16, SETTING_INPUT } SettingType; typedef struct { const char* name; void* ptr; SettingType type; } Setting;
#define S_U8(n, v)  { n, &Sys_Settings.v, SETTING_U8 }
#define S_U16(n, v) { n, &Sys_Settings.v, SETTING_U16 }
#define S_IN(n, i)  { n, &Sys_Settings.InputCodeSettings[i], SETTING_INPUT }
const Setting configTable[] = {
    S_U16("ResolutionWidth",ScreenWidth),S_U16("ResolutionHeight",ScreenHeight),S_U8("Fullscreen",Fullscreen),      S_U8("FOV",FOV),                     S_U8("Brightness",Brightness),
    S_U8("Gamma",Gamma),S_U8("AA",FXAA),  S_U8("Shadows",Shadows),              S_U8("SSR",Reflections),            S_U8("VSync",Vsync),                 S_U8("ModelDetail",ModelDetail),
    S_U8("GI",GI),                        S_U8("SpeakerMode",SpeakerMode),      S_U8("Reverb",Reverb),              S_U8("VolumeMaster",VolumeMaster),   S_U8("VolumeMusic",VolumeMusic),
    S_U8("VolumeMessage",VolumeMessage),  S_U8("VolumeEffects",VolumeEffects),  S_U8("Language",Language),          S_U8("DynamicMusic",DynamicMusic),   S_U8("Footsteps",Footsteps),
    S_U8("InvertLook",InvertLook),        S_U8("Monitor",CurrentMonitor),       
    S_U8("InvertCyberspaceLook",InvertCyberspaceLook),  S_U8("InvertInventoryCycling",InvertInventoryCycling),S_U8("QuickItemPickup",QuickItemPickup),
    S_U8("QuickReloadWeapons",QuickReloadWeapons),      S_U8("MouseSensitivity",MouseSensitivity),            S_U8("NoShootMode",NoShootMode),           S_U8("HeadBob",HeadBob),
    S_IN("Forward",0),    S_IN("Strafe Left",1),S_IN("Backpedal",2), S_IN("Strafe Right",3),S_IN("Jump",4),        S_IN("Crouch",5),    S_IN("Prone",6),       S_IN("Lean Left",7),
    S_IN("Lean Right",8), S_IN("Sprint",9),     S_IN("Turn Left",10),S_IN("Turn Right",11), S_IN("Look Up",12),    S_IN("Look Down",13),S_IN("Recent Log",14),
    S_IN("Biomonitor",15),S_IN("Sensaround",16),S_IN("Lantern",17),  S_IN("Shield",18),     S_IN("Infrared",19),   S_IN("Email",20),    S_IN("Booster",21),
    S_IN("Jumpjets",22),  S_IN("Attack",23),    S_IN("Use",24),      S_IN("Menu/Back",25),  S_IN("Toggle Mode",26),S_IN("Reload",27),
    S_IN("Weapon +",28),  S_IN("Weapon -",29),  S_IN("Grenade",30),  S_IN("Grenade +",31),  S_IN("Grenade -",32),  S_IN("Ammo Type",33),S_IN("Patch Use",34),
    S_IN("Patch +",35),   S_IN("Patch -",36),   S_IN("Full Map",37), S_IN("Swim Up",38),    S_IN("Swim Down",39),  S_IN("Screenshot",40)
};

const int configTableSize = sizeof(configTable) / sizeof(Setting);
INLINE i32 GetWinSysIndirectionIndexForAnInput(const char* val) { for (int i=0;i<134;++i) {if (sEqual(val,inputElements[i].name)) return i;} return 148; }
void LoadConfig() {
    FHandle f = OS_OpenReadonly("./Data/Config.ini");
    char line[512];
    while (sUpToEndLine(line,sizeof(line),f)) {
        char* s = data_parser_trim(line); if (*s == 0 || (s[0] == '/' && s[1] == '/')) continue;
        char* eq = StringFindFirstCharWithin(s, '='); if (!eq) continue;
        *eq = 0; char *key = data_parser_trim(s), *val = data_parser_trim(eq + 1);
        for (int i = 0; i < configTableSize; i++) {
            if (sEqual(key,configTable[i].name)) {
                if (configTable[i].type == SETTING_U8)         *( u8*)configTable[i].ptr = (u8)s2i32(val);
                else if (configTable[i].type == SETTING_U16)   *(u16*)configTable[i].ptr = (u16)s2i32(val);
                else if (configTable[i].type == SETTING_INPUT) *(u16*)configTable[i].ptr = GetWinSysIndirectionIndexForAnInput(val);
                break;
            }
        }
    }
    Sys_Settings.ScreenWidth = vmax(Sys_Settings.ScreenWidth,320); Sys_Settings.ScreenHeight = vmax(Sys_Settings.ScreenHeight,200);
    OS_Close(f);
}

void FilePrintString(FHandle f, const char* fmt, ...) { va_list a; __builtin_va_start(a,fmt); char b[128]; va_list c; __builtin_va_copy(c,a); sFormatV(b,sizeof(b),fmt,c); __builtin_va_end(c); OS_RawWrite(f,b,slen(b)); __builtin_va_end(a); }
void SaveConfig() {
    DualLog("Saving config\n");
    FHandle f = OS_OpenWriteonly("./Data/Config.ini");
    for (int i=0;i<configTableSize;++i) {
        if (configTable[i].type == SETTING_U8)         FilePrintString(f,"%s = %u\n",configTable[i].name,*(u8*)configTable[i].ptr);
        else if (configTable[i].type == SETTING_U16)   FilePrintString(f,"%s = %u\n",configTable[i].name,*(u16*)configTable[i].ptr);
        else if (configTable[i].type == SETTING_INPUT) FilePrintString(f,"%s = %s\n",configTable[i].name,inputElements[*(u16*)configTable[i].ptr].name);
    }
    OS_Close(f);
    DualLog("Saved settings to ./Data/Config.ini! framenum %u\n",globalframe);
}
