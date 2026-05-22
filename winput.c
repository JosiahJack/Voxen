// winput.c - Window and Input System
// GLFW 3.5 This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
#include "os.h"
#include "gl.h"
#include "common.h"
#include "interop.h"
extern GLFWwindow* window;
typedef struct { bool down,pressed,released; } KeyState;
typedef struct { double last_mouse_x,last_mouse_y,scrollDelta; KeyState keyStates[MAX_KEYS],mouseButtons[MAX_MOUSE_BUTTONS],joystickButtons[16][16],joystickHats[5]; /* What can I say, I'm a man of many hats. ^^D*/ i32 currentMouse_dx,currentMouse_dy; bool window_has_focus,ignore_next_mouse_delta,lastUse,isCapsLockOn,joystickPresent[16]; } InputSystem;
extern InputSystem Sys_Input; extern bool returnToPause; extern GlobalContext Sys_Global; extern u8 currentPlayerNameLength; extern i8 currentMenuItem; extern CheatsSystem Sys_Cheats;
extern bool mouseMovementThisFrame; extern SettingsSystem Sys_Settings; extern float cam_pitch,cam_yaw,cam_roll;
typedef struct { const char* name; int value; } InputElement; extern InputElement inputElements[134];
typedef void (*GLFWproc)(void); typedef struct _GLFWfbconfig _GLFWfbconfig; typedef struct _GLFWcontext _GLFWcontext; typedef struct _GLFWwindow _GLFWwindow; typedef struct _GLFWlibrary _GLFWlibrary; typedef struct _GLFWmonitor _GLFWmonitor; typedef struct _GLFWjoystick _GLFWjoystick;
void* CopyMemoryFromBtoAForNBytes(void *dst, const void *src, size_t n); int StringCompareUpToLength(const char* s1, const char* s2, size_t n); void UpdateScreenSize(i32 width, i32 height); void SaveConfig(void);
struct _GLFWfbconfig { int redBits,greenBits,blueBits,alphaBits,depthBits,stencilBits,accumRedBits,accumGreenBits,accumBlueBits,accumAlphaBits; i32 samples,stereo,sRGB,doublebuffer; uintptr_t handle; };
extern _GLFWlibrary _glfw;
GLFWproc PlatformGetModuleSymbol(void* module, const char* name);
void InputWindowFocus(_GLFWwindow* window, i32 focused);             void InputKey(_GLFWwindow* window, int key, int action);
void InputMouseClick(_GLFWwindow* window, int button, int action);   void InputCursorPos(_GLFWwindow* window, double xpos, double ypos);
void JoystickConnection(_GLFWjoystick* js, int event);               void InputJoystickAxis(_GLFWjoystick* js, int axis, float value);
void InputJoystickButton(_GLFWjoystick* js, int button, char value); void InputJoystickHat(_GLFWjoystick* js, int hat, char value);
void InputMonitor(_GLFWmonitor* monitor, int action, int placement); const _GLFWfbconfig* _glfwChooseFBConfig(const _GLFWfbconfig* alternatives, unsigned int count);
_GLFWmonitor* AllocMonitor(const char* name, int widthMM, int heightMM); _GLFWjoystick* _glfwAllocJoystick(const char* name, const char* guid, int axisCount, int buttonCount, int hatCount);
void _glfwFreeJoystick(_GLFWjoystick* js);
#if defined(WINDOWS)
    #define MAKEWORD(a,b) ((u16) (((u8) (((u64) (a)) & 0xff)) | ((u16) ((u8) (((u64) (b)) & 0xff))) << 8))
    #define MAKELONG(a, b) ((i32) (((u16) (((u64) (a)) & 0xffff)) | ((u32) ((u16) (((u64) (b)) & 0xffff))) << 16))
    #define LOWORD(l) ((u16) (((u64) (l)) & 0xffff))
    #define HIWORD(l) ((u16) ((((u64) (l)) >> 16) & 0xffff))
    #define LOBYTE(w) ((u8) (((u64) (w)) & 0xff))
    #define HIBYTE(w) ((u8) ((((u64) (w)) >> 8) & 0xff))
    #define SUCCEEDED(hr) ((i32)(hr) >= 0)
    typedef struct HWND__ { int unused; } *HWND;   typedef struct HBITMAP__ { int unused; } *HBITMAP; typedef struct HBRUSH__ { int unused; } *HBRUSH; typedef struct HDC__ { int unused; } *HDC;
    typedef struct HGLRC__ { int unused; } *HGLRC; typedef struct HICON__ { int unused; } *HICON;     typedef struct HMENU__ { int unused; } *HMENU;   typedef struct HMONITOR__ { int unused; } *HMONITOR;
    typedef struct tagPOINT { i32 x,y; } POINT,*PPOINT,*NPPOINT,*LPPOINT;              typedef struct _POINTL { i32 x,y; } POINTL,*PPOINTL;
    typedef struct tagRECT { i32 left,top,right,bottom; } RECT,*PRECT,*NPRECT,*LPRECT; typedef struct tagSIZE { i32 cx,cy; } SIZE,*PSIZE,*LPSIZE;
    typedef struct _OSVERSIONINFOEXW { u32 dwOSVersionInfoSize,dwMajorVersion,dwMinorVersion,dwBuildNumber,dwPlatformId; u16 szCSDVersion[128]; u16 wServicePackMajor,wServicePackMinor,wSuiteMask; u8 wProductType,wReserved; } OSVERSIONINFOEXW;
    int __cdecl wcscmp(const u16 *_Str1,const u16 *_Str2); u16* wcscpy(u16* restrict destination, const u16* restrict source);
    #define MAKEINTATOM(i) (u16*)((u64)((u16)(i)))
    typedef struct _devicemodeW {
        u16 dmDeviceName[32]; u16 dmSpecVersion,dmDriverVersion,dmSize,dmDriverExtra; u32 dmFields;
        union { struct { i16 dmOrientation,dmPaperSize,dmPaperLength,dmPaperWidth,dmScale,dmCopies,dmDefaultSource,dmPrintQuality; }; struct { POINTL dmPosition; u32 dmDisplayOrientation,dmDisplayFixedOutput; }; };
        i16 dmColor,dmDuplex,dmYResolution,dmTTOption,dmCollate; u16 dmFormName[32],dmLogPixels; u32 dmBitsPerPel,dmPelsWidth,dmPelsHeight; union { u32 dmDisplayFlags,dmNup; };
        u32 dmDisplayFrequency,dmICMMethod,dmICMIntent,dmMediaType,dmDitherType,dmReserved1,dmReserved2,dmPanningWidth,dmPanningHeight;
    } DEVMODEW,*LPDEVMODEW;
    typedef i64 (__stdcall *WNDPROC)(HWND,u32,u64,i64);
    typedef i32 (__stdcall *MONITORENUMPROC)(HMONITOR,HDC,LPRECT,i64);
    typedef struct _ICONINFO { i32 fIcon; u32 xHotspot,yHotspot; HBITMAP hbmMask,hbmColor; } ICONINFO; typedef ICONINFO *PICONINFO;
    typedef struct tagMSG { HWND hwnd; u32 message; u64 wParam; i64 lParam; u32 time; POINT pt; } MSG,*PMSG,*NPMSG,*LPMSG;
    typedef struct tagMONITORINFO { u32 cbSize; RECT rcMonitor; RECT rcWork; u32 dwFlags; } MONITORINFO,*LPMONITORINFO;
    typedef struct tagMONITORINFOEXW { u32 cbSize; RECT rcMonitor; RECT rcWork; u32 dwFlags; u16 szDevice[32]; } MONITORINFOEXW;
    typedef struct tagWINDOWPLACEMENT { u32 length; u32 flags; u32 showCmd; POINT ptMinPosition; POINT ptMaxPosition; RECT rcNormalPosition; } WINDOWPLACEMENT;
    typedef struct tagWNDCLASSEXW { u32 cbSize,style; WNDPROC lpfnWndProc; i32 cbClsExtra,cbWndExtra; HINSTANCE hInstance; HICON hIcon,hCursor; HBRUSH hbrBackground; u16 *lpszMenuName,*lpszClassName; HICON hIconSm; } WNDCLASSEXW;
    typedef struct {u16 wButtons; u8 bLeftTrigger,bRightTrigger; i16 sThumbLX,sThumbLY,sThumbRX,sThumbRY; } XINPUT_GAMEPAD;
    typedef struct {u16 wLeftMotorSpeed, wRightMotorSpeed;} XINPUT_VIBRATION;
    typedef struct {u8 Type,SubType; u16 Flags; XINPUT_GAMEPAD Gamepad; XINPUT_VIBRATION Vibration;} XINPUT_CAPABILITIES;
    typedef struct {u32 dwPacketNumber; XINPUT_GAMEPAD Gamepad;} XINPUT_STATE;
    typedef u32 (WINAPI * PFN_XInputGetCapabilities)(u32,u32,XINPUT_CAPABILITIES*);
    typedef u32 (WINAPI * PFN_XInputGetState)(u32,XINPUT_STATE*);
    typedef struct { u32 dbch_size,dbch_devicetype,dbch_reserved; } DEV_BROADCAST_HDR;
    typedef struct { u32 dbcc_size,dbcc_devicetype,dbcc_reserved; GUID dbcc_classguid; u16 dbcc_name[1]; } DEV_BROADCAST_DEVICEINTERFACE_W;
    typedef i32 (WINAPI * PFN_DwmIsCompositionEnabled)(i32*);
    typedef i32 (WINAPI * PFN_DwmFlush)(void);
    typedef i32 (WINAPI * PFN_RtlVerifyVersionInfo)(OSVERSIONINFOEXW*,u32,u64);
    typedef i32 (WINAPI * PFNWGLSWAPINTERVALEXTPROC)(int);
    typedef i32 (WINAPI * PFNWGLGETPIXELFORMATATTRIBIVARBPROC)(HDC,int,int,u32,const int*,int*);
    typedef HGLRC (WINAPI * PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC,HGLRC,const int*);
    typedef HGLRC (WINAPI * PFN_wglCreateContext)(HDC);
    typedef PROC (WINAPI * PFN_wglGetProcAddress)(const char*);
    typedef HDC (WINAPI * PFN_wglGetCurrentDC)(void);
    typedef HGLRC (WINAPI * PFN_wglGetCurrentContext)(void);
    typedef i32 (WINAPI * PFN_wglMakeCurrent)(HDC,HGLRC);
    typedef struct _GLFWcontextWGL { HDC dc; HGLRC handle; int interval; } _GLFWcontextWGL;
    typedef struct _GLFWlibraryWGL { HINSTANCE instance; PFN_wglCreateContext CreateContext; PFN_wglGetProcAddress GetProcAddress; PFN_wglGetCurrentDC GetCurrentDC; PFN_wglGetCurrentContext GetCurrentContext; PFN_wglMakeCurrent MakeCurrent; PFNWGLSWAPINTERVALEXTPROC SwapIntervalEXT; PFNWGLGETPIXELFORMATATTRIBIVARBPROC GetPixelFormatAttribivARB; PFNWGLCREATECONTEXTATTRIBSARBPROC CreateContextAttribsARB; } _GLFWlibraryWGL;
    typedef struct _GLFWwindowWin32 { HWND handle; i32 cursorTracked,frameAction,keymenu; int width,height,lastCursorPosX,lastCursorPosY; } _GLFWwindowWin32;
    typedef struct _GLFWlibraryWin32 { HINSTANCE instance; HWND helperWindowHandle; u16 helperWindowClass,mainWindowClass; void* deviceNotificationHandle; short int keycodes[512],scancodes[349]; double restoreCurPosX,restoreCurPosY; _GLFWwindow *disabledCursorWindow, *capturedCursorWindow; HICON blankCursor; struct {HINSTANCE instance; PFN_XInputGetCapabilities GetCapabilities; PFN_XInputGetState GetState;} xinput; struct {HINSTANCE instance; PFN_DwmIsCompositionEnabled IsCompositionEnabled; PFN_DwmFlush Flush;} dwmapi; struct {HINSTANCE instance; PFN_RtlVerifyVersionInfo RtlVerifyVersionInfo;} ntdll;} _GLFWlibraryWin32;
    typedef struct _GLFWmonitorWin32 { HMONITOR handle; u16 adapterName[32],displayName[32]; char publicAdapterName[32],publicDisplayName[32]; i32 modesPruned,modeChanged; } _GLFWmonitorWin32;
    typedef struct _GLFWjoystickWin32{ int objectCount; u32 index; GUID guid; } _GLFWjoystickWin32;
    typedef long FXPT2DOT30; typedef struct tagCIEXYZ { FXPT2DOT30 x,y,z; } CIEXYZ; typedef struct tagICEXYZTRIPLE {CIEXYZ r,g,b;} CIEXYZTRIPLE;
    typedef struct _DISPLAY_DEVICEW { u32 cb; u16 DeviceName[32],DeviceString[128]; u32 StateFlags; u16 DeviceID[128],DeviceKey[128]; } DISPLAY_DEVICEW,*PDISPLAY_DEVICEW,*LPDISPLAY_DEVICEW;
    typedef struct tagPIXELFORMATDESCRIPTOR { u16 nSize,nVersion; u32 dwFlags; u8 iPixelType,cColorBits,cRedBits,cRedShift,cGreenBits,cGreenShift,cBlueBits,cBlueShift,cAlphaBits,cAlphaShift,cAccumBits,cAccumRedBits,cAccumGreenBits,cAccumBlueBits,cAccumAlphaBits,cDepthBits,cStencilBits,cAuxBuffers,iLayerType,bReserved; u32 dwLayerMask,dwVisibleMask,dwDamageMask; } PIXELFORMATDESCRIPTOR,*PPIXELFORMATDESCRIPTOR,*LPPIXELFORMATDESCRIPTOR;
    typedef struct { u32 bV5Size; i32 bV5Width,bV5Height; u16 bV5Planes,bV5BitCount; u32 bV5Compression,bV5SizeImage; i32 bV5XPelsPerMeter; i32 bV5YPelsPerMeter; u32 bV5ClrUsed,bV5ClrImportant,bV5RedMask,bV5GreenMask,bV5BlueMask,bV5AlphaMask,bV5CSType; CIEXYZTRIPLE bV5Endpoints; u32 bV5GammaRed,bV5GammaGreen,bV5GammaBlue,bV5Intent,bV5ProfileData,bV5ProfileSize,bV5Reserved; } BITMAPV5HEADER,*LPBITMAPV5HEADER,*PBITMAPV5HEADER;
    typedef struct tagRGBQUAD { u8 rgbBlue,rgbGreen,rgbRed,rgbReserved; } RGBQUAD;
    typedef struct tagBITMAPINFOHEADER { u32 biSize; i32 biWidth,biHeight; u16 biPlanes,biBitCount; u32 biCompression; u32 biSizeImage; i32 biXPelsPerMeter; i32 biYPelsPerMeter; u32 biClrUsed; u32 biClrImportant; } BITMAPINFOHEADER,*LPBITMAPINFOHEADER,*PBITMAPINFOHEADER;
    typedef struct tagBITMAPINFO { BITMAPINFOHEADER bmiHeader; RGBQUAD bmiColors[1]; } BITMAPINFO,*LPBITMAPINFO,*PBITMAPINFO;
    DECLSPEC_IMPORT HICON WINAPI CreateIconIndirect(PICONINFO); DECLSPEC_IMPORT HDC WINAPI GetDC(HWND);                     DECLSPEC_IMPORT i32 WINAPI GetModuleHandleExW(u32,const u16*,HINSTANCE*);
    DECLSPEC_IMPORT int WINAPI ReleaseDC(HWND,HDC);             DECLSPEC_IMPORT i32 WINAPI SetCursorPos(int,int);           DECLSPEC_IMPORT int WINAPI WideCharToMultiByte(u32,u32,u16*,int,char*,int,const char*,i32*);
    DECLSPEC_IMPORT HICON WINAPI SetCursor(HICON);              DECLSPEC_IMPORT i32 WINAPI GetCursorPos(LPPOINT);           DECLSPEC_IMPORT int WINAPI MultiByteToWideChar(u32,u32,const char*,int,u16*,int);
    DECLSPEC_IMPORT i32 WINAPI ClipCursor(const RECT*);         DECLSPEC_IMPORT i32 WINAPI ClientToScreen(HWND,LPPOINT);    DECLSPEC_IMPORT HDC WINAPI CreateDCW(const u16*,const u16*,const u16*,const DEVMODEW*);
    DECLSPEC_IMPORT void* WINAPI GetPropW(HWND,u16*);           DECLSPEC_IMPORT i32 WINAPI GetMessageTime(void);            DECLSPEC_IMPORT i32 WINAPI GetClientRect(HWND,LPRECT); // Haha get rect!
    DECLSPEC_IMPORT HICON WINAPI LoadCursorW(HINSTANCE,u16*);   DECLSPEC_IMPORT u32 WINAPI MapVirtualKeyW(u32,u32);         DECLSPEC_IMPORT i32 WINAPI SetWindowPos(HWND,HWND,int,int,int,int,u32);    
    DECLSPEC_IMPORT HWND WINAPI SetCapture(HWND hWnd);          DECLSPEC_IMPORT i32 WINAPI ReleaseCapture(void);            DECLSPEC_IMPORT i32 WINAPI PeekMessageW(LPMSG,HWND,u32,u32,u32);
    DECLSPEC_IMPORT i32 WINAPI AdjustWindowRect(LPRECT,u32,i32);DECLSPEC_IMPORT i32 WINAPI GetWindowLongW(HWND,int);        DECLSPEC_IMPORT i64 WINAPI DefWindowProcW(HWND,u32,u64,i64);
    DECLSPEC_IMPORT HMONITOR WINAPI MonitorFromWindow(HWND,u32);DECLSPEC_IMPORT HWND WINAPI GetActiveWindow(void);          DECLSPEC_IMPORT i32 WINAPI AdjustWindowRectEx(LPRECT,u32,i32,u32);
    DECLSPEC_IMPORT i64 WINAPI SendMessageW(HWND,u32,u64,i64);  DECLSPEC_IMPORT i32 WINAPI SetWindowLongW(HWND,int,i32);    DECLSPEC_IMPORT i32 WINAPI GetMonitorInfoW(HMONITOR,LPMONITORINFO);
    DECLSPEC_IMPORT i32 WINAPI TranslateMessage(const MSG*);    DECLSPEC_IMPORT i16 WINAPI GetKeyState(int);                DECLSPEC_IMPORT i64 WINAPI DispatchMessageW(const MSG*);
    DECLSPEC_IMPORT i32 WINAPI ShowWindow(HWND,int);            DECLSPEC_IMPORT i32 WINAPI BringWindowToTop(HWND);          DECLSPEC_IMPORT i32 WINAPI SetWindowPlacement(HWND,const WINDOWPLACEMENT*);
    DECLSPEC_IMPORT HWND WINAPI SetFocus(HWND);                 DECLSPEC_IMPORT i32 WINAPI SetForegroundWindow(HWND);       DECLSPEC_IMPORT i32 WINAPI GetWindowPlacement(HWND,WINDOWPLACEMENT*);
    DECLSPEC_IMPORT i32 WINAPI SetPropW(HWND,u16*,void*);       DECLSPEC_IMPORT i32 WINAPI OffsetRect(LPRECT,int,int);      DECLSPEC_IMPORT HWND WINAPI CreateWindowExW(u32,u16*,u16*,u32,int,int,int,int,HWND,HMENU,HINSTANCE,void*);
    DECLSPEC_IMPORT u64 WINAPI VerSetConditionMask(u64,u32,u8); DECLSPEC_IMPORT HDC WINAPI wglGetCurrentDC(void);           DECLSPEC_IMPORT u16 WINAPI RegisterClassExW(const WNDCLASSEXW *);
    DECLSPEC_IMPORT i32 WINAPI DeleteObject(void*);             DECLSPEC_IMPORT i32 WINAPI DeleteDC(HDC);                   DECLSPEC_IMPORT void* WINAPI RegisterDeviceNotificationW(void*,void*,u32);
    DECLSPEC_IMPORT i32 WINAPI SwapBuffers(HDC);                DECLSPEC_IMPORT HGLRC WINAPI wglGetCurrentContext(void);    DECLSPEC_IMPORT i32 WINAPI EnumDisplayMonitors(HDC,const RECT*,MONITORENUMPROC,i64);
    DECLSPEC_IMPORT i32 WINAPI wglMakeCurrent(HDC,HGLRC);       DECLSPEC_IMPORT PROC WINAPI wglGetProcAddress(const char*); DECLSPEC_IMPORT i32 WINAPI EnumDisplaySettingsW(u16*,u32,LPDEVMODEW); 
    DECLSPEC_IMPORT i32 WINAPI EnumDisplayDevicesW(u16*,u32,PDISPLAY_DEVICEW,u32);               DECLSPEC_IMPORT i32 WINAPI EnumDisplaySettingsExW(u16*,u32,LPDEVMODEW,u32);
    DECLSPEC_IMPORT i32 WINAPI SetPixelFormat(HDC,i32,const PIXELFORMATDESCRIPTOR *);            DECLSPEC_IMPORT i32 WINAPI ChoosePixelFormat(HDC hdc,const PIXELFORMATDESCRIPTOR *ppfd);
    DECLSPEC_IMPORT i32 WINAPI DescribePixelFormat(HDC,i32,u32,LPPIXELFORMATDESCRIPTOR);         DECLSPEC_IMPORT HBITMAP WINAPI CreateBitmap(i32,i32,u32,u32,const void *);
    DECLSPEC_IMPORT HBITMAP WINAPI CreateDIBSection(HDC,const BITMAPINFO*,u32,void**,void*,u32); DECLSPEC_IMPORT i32 WINAPI GetDeviceCaps(HDC,i32);
    u16* CreateWideStringFromUTF8Win32(const char* source); i32 IsWindowsVersionOrGreaterWin32(u16 major, u16 minor, u16 sp); void _glfwPollMonitorsWin32(void);
    struct _GLFWjoystick { i32 allocated,connected; size_t axesSize,buttonsSize,hatsSize; float*  axes; int axisCount; unsigned char* buttons; int buttonCount; unsigned char* hats; int hatCount; char name[128],guid[33]; _GLFWjoystickWin32 win32; };
    struct _GLFWlibrary { _GLFWmonitor** monitors; int monitorCount; i32 joysticksInitialized; _GLFWjoystick joysticks[GLFW_JOYSTICK_LAST + 1]; _GLFWlibraryWin32 win32; _GLFWlibraryWGL wgl; };
    struct _GLFWcontext { int client,source,major,minor; PFNGLGETINTEGERV GetIntegerv; void (*makeCurrent)(_GLFWwindow*); void (*swapBuffers)(_GLFWwindow*); void (*swapInterval)(int); GLFWglproc (*getProcAddress)(const char*); _GLFWcontextWGL wgl; };
    struct _GLFWwindow { i32 decorated,doublebuffer; GLFWvidmode videoMode; int cursorMode; char mouseButtons[8],keys[349]; double virtualCursorPosX,virtualCursorPosY; _GLFWcontext context; _GLFWwindowWin32 win32; };
    struct _GLFWmonitor { char name[128]; int widthMM,heightMM; GLFWvidmode currentMode; _GLFWmonitorWin32 win32; };
    static u32 getWindowStyle(const _GLFWwindow* w) { return 0x060A0000 | (w->decorated ? 0x00C00000 : 0x80000000); } // clipping,sysmenu,minimize,title,border,and borderless raw
    static HICON createIcon(const GLFWimage* image,int xhot,int yhot,i32 icon) {
        HDC dc; HICON handle; HBITMAP color,mask; BITMAPV5HEADER bi; ICONINFO ii;
        unsigned char* target=NULL; unsigned char* source=image->pixels;
        MemSetToVForNBytes(&bi,0,sizeof(bi));
        bi.bV5Size=sizeof(bi); bi.bV5Width=image->width; bi.bV5Height=-image->height; bi.bV5Planes=1; bi.bV5BitCount=32; bi.bV5Compression=3; bi.bV5RedMask=0x00ff0000; bi.bV5GreenMask=0x0000ff00; bi.bV5BlueMask=0x000000ff; bi.bV5AlphaMask=0xff000000;
        dc=GetDC(NULL);
        color=CreateDIBSection(dc,(BITMAPINFO*)&bi,0,(void**)&target,NULL,(u32)0U);
        ReleaseDC(NULL,dc);
        mask=CreateBitmap(image->width,image->height,1,1,NULL);
        for (int i=0;i<image->width*image->height;i++) { target[0]=source[2]; target[1]=source[1]; target[2]=source[0]; target[3]=source[3]; target+=4; source+=4; }
        MemSetToVForNBytes(&ii,0,sizeof(ii));
        ii.fIcon=icon; ii.xHotspot=xhot; ii.yHotspot=yhot; ii.hbmMask=mask; ii.hbmColor=color;
        handle=CreateIconIndirect(&ii); DeleteObject(color); DeleteObject(mask); return handle;
    }

    static void updateCursorImage(_GLFWwindow* window) { if (window->cursorMode==0x00034001/*GLFW_CURSOR_NORMAL*/) {SetCursor(LoadCursorW(NULL,(u16*)((u64)(u16)32512)));} else {SetCursor(_glfw.win32.blankCursor);} }
    static void captureCursor(_GLFWwindow* window) { RECT clipRect; GetClientRect(window->win32.handle,&clipRect); ClientToScreen(window->win32.handle,(POINT*)&clipRect.left); ClientToScreen(window->win32.handle,(POINT*)&clipRect.right); ClipCursor(&clipRect); _glfw.win32.capturedCursorWindow=window; }
    static void releaseCursor(void) { ClipCursor(NULL); _glfw.win32.capturedCursorWindow=NULL; }
    static void disableCursor(_GLFWwindow* window) { _glfw.win32.disabledCursorWindow = window; POINT pos; GetCursorPos(&pos); _glfw.win32.restoreCurPosX = pos.x; _glfw.win32.restoreCurPosY = pos.y; updateCursorImage(window); captureCursor(window); }
    static void SetCursorPosV(_GLFWwindow* window, double xpos, double ypos) { window->win32.lastCursorPosX = (int)xpos; window->win32.lastCursorPosY = (int)ypos; POINT pos = {(int)xpos,(int)ypos}; ClientToScreen(window->win32.handle,&pos); SetCursorPos(pos.x,pos.y); }
    static void enableCursor(_GLFWwindow* window) { _glfw.win32.disabledCursorWindow = NULL; releaseCursor(); SetCursorPosV(window,_glfw.win32.restoreCurPosX,_glfw.win32.restoreCurPosY); updateCursorImage(window); }
    static i64 __stdcall windowProc(HWND hWnd, u32 uMsg, u64 wParam, i64 lParam) {
        _GLFWwindow* window=GetPropW(hWnd,L"GLFW"); if (!window) return DefWindowProcW(hWnd,uMsg,wParam,lParam);
        switch (uMsg) {
            case 0x0021/*WM_MOUSEACTIVATE*/:  if (HIWORD(lParam) == 0x0201/*WM_LBUTTONDOWN*/ && LOWORD(lParam)!=1) {window->win32.frameAction= 1;} break;
            case 0x0215/*WM_CAPTURECHANGED*/: if (lParam==0&&window->win32.frameAction) { if (window->cursorMode==0x00034003/*CURSOR_DISABLED*/) {disableCursor(window);} window->win32.frameAction=0; } break;
            case 0x0007/*WM_SETFOCUS*/:   InputWindowFocus(window, 1); if (window->win32.frameAction) {break;} if (window->cursorMode==0x00034003/*CURSOR_DISABLED*/) {disableCursor(window);} return 0;
            case 0x0008/*WM_KILLFOCUS*/:  if (window->cursorMode==0x00034003/*CURSOR_DISABLED*/) {enableCursor(window);} InputWindowFocus(window,0); return 0;
            case 0x0112/*WM_SYSCOMMAND*/: switch (wParam&0xfff0) { case 0xF140/*SC_SCREENSAVE*/: case 0xF170/*SC_MONITORPOWER*/: break; case 0xF100/*SC_KEYMENU*/: if (!window->win32.keymenu) return 0; break; } break;
            case 0x0010/*WM_CLOSE*/:      OS_Exit(0);
            case 0x0100/*WM_KEYDOWN*/: case 0x0104/*WM_SYSKEYDOWN*/: case 0x0101/*WM_KEYUP*/: case 0x0105/*WM_SYSKEYUP*/: {
                const int action=(HIWORD(lParam)&0x8000)?GLFW_RELEASE:GLFW_PRESS;
                int scancode=(HIWORD(lParam)&(0x0100|0xff));
                if (!scancode) scancode=MapVirtualKeyW((u32)wParam,0);
                if (scancode==0x54) {scancode=0x137;}   if (scancode==0x146) {scancode=0x45;}   if (scancode==0x136) {scancode=0x36;}
                int key = _glfw.win32.keycodes[scancode];
                if (wParam==0x11/*VK_CONTROL*/) {
                    if (HIWORD(lParam)&0x0100) key=GLFW_KEY_RIGHT_CONTROL;
                    else {
                        MSG next; const u32 time=GetMessageTime();
                        if (PeekMessageW(&next,NULL,0,0,0)) {
                            if (next.message == 0x0100/*WM_KEYDOWN*/ || next.message == 0x0104/*WM_SYSKEYDOWN*/ || next.message == 0x0101/*WM_KEYUP*/ || next.message == 0x0105/*WM_SYSKEYUP*/) {
                                if (next.wParam == 0x12/*VK_MENU*/ && (HIWORD(next.lParam) & 0x0100)&&next.time==time) break;
                            }
                        }
                        
                        key=GLFW_KEY_LEFT_CONTROL;
                    }
                } else if (wParam == 0xE5/*VK_PROCESSKEY*/) break;
                if (action == GLFW_RELEASE && wParam == 0x10/*VK_SHIFT*/) { InputKey(window,GLFW_KEY_LEFT_SHIFT,action); InputKey(window,GLFW_KEY_RIGHT_SHIFT,action); }
                else if (wParam == 0x2C/*VK_SNAPSHOT*/) { InputKey(window,key,GLFW_PRESS); InputKey(window,key,GLFW_RELEASE); }
                else InputKey(window,key,action);
                break;
            }
            case 0x0201/*WM_LBUTTONDOWN*/: case 0x0204/*WM_RBUTTONDOWN*/: case 0x0207/*WM_MBUTTONDOWN*/: case 0x020B/*WM_XBUTTONDOWN*/:
            case 0x0202/*WM_LBUTTONUP*/:   case 0x0205/*WM_RBUTTONUP*/:   case 0x0208/*WM_MBUTTONUP*/:   case 0x020C/*WM_XBUTTONUP*/: {
                int i,action,button = (uMsg==0x0201/*WM_LBUTTONDOWN*/ || uMsg == 0x0202/*WM_LBUTTONUP*/) ? GLFW_MOUSE_BUTTON_LEFT : ((uMsg == 0x0204/*WM_RBUTTONDOWN*/ || uMsg == 0x0205/*WM_RBUTTONUP*/) ? GLFW_MOUSE_BUTTON_RIGHT : ((uMsg == 0x0207/*WM_MBUTTONDOWN*/ || uMsg == 0x0208/*WM_MBUTTONUP*/) ? GLFW_MOUSE_BUTTON_MIDDLE : (((HIWORD(wParam)) == 0x0001/*XBUTTON1*/) ? GLFW_MOUSE_BUTTON_4 : GLFW_MOUSE_BUTTON_5)));
                action=(uMsg == 0x0201/*WM_LBUTTONDOWN*/ || uMsg == 0x0204/*WM_RBUTTONDOWN*/ || uMsg == 0x0207/*WM_MBUTTONDOWN*/ || uMsg == 0x020B/*WM_XBUTTONDOWN*/) ? GLFW_PRESS : GLFW_RELEASE;
                for (i=0;i<=7;i++) { if (window->mouseButtons[i]==GLFW_PRESS) break; }
                if (i>7) {SetCapture(hWnd);} InputMouseClick(window,button,action);
                for (i=0;i<=7;i++) { if (window->mouseButtons[i]==GLFW_PRESS) break; }
                if (i>7) {ReleaseCapture();} if (uMsg == 0x020B/*WM_XBUTTONDOWN*/ || uMsg == 0x020C/*WM_XBUTTONUP*/) return 1;
                return 0;
            }
            case 0x0200/*WM_MOUSEMOVE*/: {                
                const int x=((int)(short)(lParam & 0xFFFF)), y=((int)(short)(lParam >> 16));
                if (window->cursorMode==0x00034003/*CURSOR_DISABLED*/) {
                    const int dx=x-window->win32.lastCursorPosX,dy=y-window->win32.lastCursorPosY;
                    if (_glfw.win32.disabledCursorWindow!=window) break;
                    InputCursorPos(window,window->virtualCursorPosX+dx,window->virtualCursorPosY+dy);
                }
                
                window->win32.lastCursorPosX=x; window->win32.lastCursorPosY=y;
                return 0;
            }
            case 0x02A3/*WM_MOUSELEAVE*/: { window->win32.cursorTracked=0; return 0; }
            case 0x020A/*WM_MOUSEWHEEL*/: { Sys_Input.scrollDelta += (i16)HIWORD(wParam)/(double)120; return 0; }
            case 0x0005/*WM_SIZE*/: if (wParam == 1) {Sys_Global.gamePaused = true;} return 0;
            case 0x0003/*WM_MOVE*/: if (_glfw.win32.capturedCursorWindow==window) {captureCursor(window);} return 0;
            case 0x0086/*WM_NCACTIVATE*/: case 0x0085/*WM_NCPAINT*/: { if (!window->decorated) return 1; break; }
            case 0x0020/*WM_SETCURSOR*/: { if (LOWORD(lParam)==1) { updateCursorImage(window); return 1; } break; }
            case 0x0084/*WM_NCHITTEST*/: ;i64 hit = DefWindowProcW(hWnd,uMsg,wParam,lParam); if (hit >= 10 && hit <= 17) { return 1; } return hit;
        }
        
        return DefWindowProcW(hWnd,uMsg,wParam,lParam);
    }

    void SetWindowIcon(_GLFWwindow* window, const GLFWimage* image) { HICON hIcon = createIcon(image,0,0, 1); SendMessageW(window->win32.handle,0x0080,1,(i64)hIcon); SendMessageW(window->win32.handle,0x0080,0,(i64)hIcon); }
    void GetWindowPos(_GLFWwindow* window, int* xpos, int* ypos) { POINT pos={0,0}; ClientToScreen(window->win32.handle,&pos); *xpos=pos.x; *ypos=pos.y; }
    void GetWindowSize(_GLFWwindow* window, int* width, int* height) { RECT area; GetClientRect(window->win32.handle,&area); *width=area.right; *height=area.bottom; }
    void SetWindowSize(_GLFWwindow* window, int width, int height) { RECT rect={0,0,width,height}; AdjustWindowRectEx(&rect,getWindowStyle(window),0,0); SetWindowPos(window->win32.handle,(HWND)0,0,0,rect.right-rect.left,rect.bottom-rect.top,0x0010|0x0200|0x0002|0x0004); }
    void SetWindowMonitor(_GLFWwindow* window, int xpos, int ypos, int width, int height) {
        RECT r = {xpos,ypos,xpos+width,ypos+height}; u32 s = GetWindowLongW(window->win32.handle,-16); u32 f = 0x0010|0x0100;
        if (window->decorated) { s &= ~0x80000000/*WS_POPUP*/, s |= getWindowStyle(window), SetWindowLongW(window->win32.handle,-16,s), f |= 0x0020; }
        AdjustWindowRectEx(&r,getWindowStyle(window),0,0);
        SetWindowPos(window->win32.handle,(HWND)-2,r.left,r.top,r.right-r.left,r.bottom-r.top,f);
    }

    void SetWindowDecorated(_GLFWwindow* window,i32 enabled) {
        (void)enabled; RECT rect; u32 style=GetWindowLongW(window->win32.handle,-16);
        style &= ~(0x00C00000/*WS_CAPTION*/ | 0x00080000/*WS_SYSMENU*/ | 0x00040000/*WS_THICKFRAME*/ | 0x00020000/*WS_MINIMIZEBOX*/ | 0x00010000/*WS_MAXIMIZEBOX*/ | 0x80000000/*WS_POPUP*/); style |= getWindowStyle(window);
        GetClientRect(window->win32.handle,&rect);
        AdjustWindowRectEx(&rect,style,0,0);
        ClientToScreen(window->win32.handle,(POINT*)&rect.left); ClientToScreen(window->win32.handle,(POINT*)&rect.right);
        SetWindowLongW(window->win32.handle,-16,style);
        SetWindowPos(window->win32.handle,(HWND)0,rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top,0x0020|0x0010|0x0004);
    }
    
    void PollEvents(void) {
        HWND handle = GetActiveWindow();
        _GLFWwindow* window = GetPropW(handle,L"GLFW"); MSG msg;
        while (PeekMessageW(&msg,NULL,0,0,0x0001)) { if (msg.message==0x0012/*WM_QUIT*/) { OS_Exit(0); } else { TranslateMessage(&msg); DispatchMessageW(&msg); } }
        const int keys[4][2]={{0xA0/*VK_LSHIFT*/,GLFW_KEY_LEFT_SHIFT},{0xA1/*VK_RSHIFT*/,GLFW_KEY_RIGHT_SHIFT},{0x5B/*VK_LWIN*/,GLFW_KEY_LEFT_SUPER},{0x5C/*VK_RWIN*/,GLFW_KEY_RIGHT_SUPER}};
        for (int i=0;i<4;i++) { const int vk=keys[i][0],key=keys[i][1]; if ((GetKeyState(vk)&0x8000)||window->keys[key]!=GLFW_PRESS) {continue;} InputKey(window,key,GLFW_RELEASE); }
        window=_glfw.win32.disabledCursorWindow;
        if (window) { int width,height; GetWindowSize(window,&width,&height); if (window->win32.lastCursorPosX!=width/2||window->win32.lastCursorPosY!=height/2) {SetCursorPosV(window,width/2,height/2);} }
    }

    void SetCursorMode(GLFWwindow* handle, int value) {
        _GLFWwindow* window = (_GLFWwindow*)handle; if (window->cursorMode == value) return;
        
        window->cursorMode = value; POINT pos; GetCursorPos(&pos);
        if (window->win32.handle == GetActiveWindow()) { _glfw.win32.restoreCurPosX=pos.x; _glfw.win32.restoreCurPosY=pos.y; captureCursor(window); _glfw.win32.disabledCursorWindow=window; } else Sys_Global.gamePaused = true;
        updateCursorImage(window);
    }

    GLFWproc PlatformGetModuleSymbol(void* module, const char* name) { return (GLFWproc)GetProcAddress((HMODULE)module,name); }
    typedef struct {u16 index; i32 vkey;} WinKeyRemap;
    static const WinKeyRemap winkeyRemapTable[] = {
        {0x00B,GLFW_KEY_0},{0x002,GLFW_KEY_1},{0x003,GLFW_KEY_2},{0x004,GLFW_KEY_3},{0x005,GLFW_KEY_4},{0x006,GLFW_KEY_5},{0x007,GLFW_KEY_6},{0x008,GLFW_KEY_7},{0x009,GLFW_KEY_8},{0x00A,GLFW_KEY_9},{0x01E,GLFW_KEY_A},{0x030,GLFW_KEY_B},{0x02E,GLFW_KEY_C},
        {0x020,GLFW_KEY_D},{0x012,GLFW_KEY_E},{0x021,GLFW_KEY_F},{0x022,GLFW_KEY_G},{0x023,GLFW_KEY_H},{0x017,GLFW_KEY_I},{0x024,GLFW_KEY_J},{0x025,GLFW_KEY_K},{0x026,GLFW_KEY_L},{0x032,GLFW_KEY_M},{0x031,GLFW_KEY_N},{0x018,GLFW_KEY_O},{0x019,GLFW_KEY_P},
        {0x010,GLFW_KEY_Q},{0x013,GLFW_KEY_R},{0x01F,GLFW_KEY_S},{0x014,GLFW_KEY_T},{0x016,GLFW_KEY_U},{0x02F,GLFW_KEY_V},{0x011,GLFW_KEY_W},{0x02D,GLFW_KEY_X},{0x015,GLFW_KEY_Y},{0x02C,GLFW_KEY_Z},{0x028,GLFW_KEY_APOSTROPHE},{0x02B,GLFW_KEY_BACKSLASH},
        {0x033,GLFW_KEY_COMMA},{0x00D,GLFW_KEY_EQUAL},{0x029,GLFW_KEY_GRAVE_ACCENT},{0x01A,GLFW_KEY_LEFT_BRACKET},{0x00C,GLFW_KEY_MINUS},{0x034,GLFW_KEY_PERIOD},{0x01B,GLFW_KEY_RIGHT_BRACKET},{0x027,GLFW_KEY_SEMICOLON},{0x035,GLFW_KEY_SLASH},
        {0x00E,GLFW_KEY_BACKSPACE},{0x153,GLFW_KEY_DELETE},{0x14F,GLFW_KEY_END},{0x01C,GLFW_KEY_ENTER},{0x001,GLFW_KEY_ESCAPE},{0x147,GLFW_KEY_HOME},{0x152,GLFW_KEY_INSERT},{0x15D,GLFW_KEY_MENU},{0x151,GLFW_KEY_PAGE_DOWN},{0x149,GLFW_KEY_PAGE_UP},
        {0x045,GLFW_KEY_PAUSE},{0x039,GLFW_KEY_SPACE},{0x00F,GLFW_KEY_TAB},{0x03A,GLFW_KEY_CAPS_LOCK},{0x145,GLFW_KEY_NUM_LOCK},{0x046,GLFW_KEY_SCROLL_LOCK},{0x03B,GLFW_KEY_F1},{0x03C,GLFW_KEY_F2},{0x03D,GLFW_KEY_F3},{0x03E,GLFW_KEY_F4},
        {0x03F,GLFW_KEY_F5},{0x040,GLFW_KEY_F6},{0x041,GLFW_KEY_F7},{0x042,GLFW_KEY_F8},{0x043,GLFW_KEY_F9},{0x044,GLFW_KEY_F10},{0x057,GLFW_KEY_F11},{0x058,GLFW_KEY_F12},{0x038,GLFW_KEY_LEFT_ALT},{0x01D,GLFW_KEY_LEFT_CONTROL},{0x02A,GLFW_KEY_LEFT_SHIFT},
        {0x15B,GLFW_KEY_LEFT_SUPER},{0x137,GLFW_KEY_PRINT_SCREEN},{0x138,GLFW_KEY_RIGHT_ALT},{0x11D,GLFW_KEY_RIGHT_CONTROL},{0x036,GLFW_KEY_RIGHT_SHIFT},{0x15C,GLFW_KEY_RIGHT_SUPER},{0x150,GLFW_KEY_DOWN},{0x14B,GLFW_KEY_LEFT},{0x14D,GLFW_KEY_RIGHT},
        {0x148,GLFW_KEY_UP},{0x052,GLFW_KEY_KP_0},{0x04F,GLFW_KEY_KP_1},{0x050,GLFW_KEY_KP_2},{0x051,GLFW_KEY_KP_3},{0x04B,GLFW_KEY_KP_4},{0x04C,GLFW_KEY_KP_5},{0x04D,GLFW_KEY_KP_6},{0x047,GLFW_KEY_KP_7},{0x048,GLFW_KEY_KP_8},{0x049,GLFW_KEY_KP_9},
        {0x04E,GLFW_KEY_KP_ADD},{0x053,GLFW_KEY_KP_DECIMAL},{0x135,GLFW_KEY_KP_DIVIDE},{0x11C,GLFW_KEY_KP_ENTER},{0x059,GLFW_KEY_KP_EQUAL},{0x037,GLFW_KEY_KP_MULTIPLY},{0x04A,GLFW_KEY_KP_SUBTRACT}
    };
    
    static void createKeyTables(void) {
        MemSetToVForNBytes(_glfw.win32.keycodes,-1,sizeof(_glfw.win32.keycodes)); MemSetToVForNBytes(_glfw.win32.scancodes,-1,sizeof(_glfw.win32.scancodes));
        for (size_t i=0;i<sizeof(winkeyRemapTable)/sizeof(winkeyRemapTable[0]);++i) _glfw.win32.keycodes[winkeyRemapTable[i].index] = winkeyRemapTable[i].vkey;
        for (int scancode=0;scancode<512;scancode++) { if (_glfw.win32.keycodes[scancode] > 0) {_glfw.win32.scancodes[_glfw.win32.keycodes[scancode]] = scancode;} }
    }

    u16* CreateWideStringFromUTF8Win32(const char* src) { u16* target; int count = MultiByteToWideChar(65001,0,(char*)src,-1,NULL,0); target = OS_Calloc(count,sizeof(u16)); MultiByteToWideChar(65001,0,(char*)src,-1,target,count); return target; }
    char* CreateUTF8FromWideStringWin32(const u16* src, int* size) { *size = WideCharToMultiByte(65001,0,(u16*)src,-1,NULL,0,NULL,NULL); char* target = OS_Calloc(*size,1); WideCharToMultiByte(65001,0,(u16*)src,-1,target,*size,NULL,NULL); return target; }
    i32 IsWindowsVersionOrGreaterWin32(u16 major, u16 minor, u16 sp) {
        OSVERSIONINFOEXW osvi={0}; osvi.dwOSVersionInfoSize=sizeof(osvi), osvi.dwMajorVersion=major, osvi.dwMinorVersion=minor, osvi.wServicePackMajor=sp;
        u32 mask=0x0000002|0x0000001|0x0000020;
        u64 cond=VerSetConditionMask(VerSetConditionMask(VerSetConditionMask(0,0x0000002,3),0x0000001,3),0x0000020,3);
        return _glfw.win32.ntdll.RtlVerifyVersionInfo(&osvi,mask,cond)==0;
    }

    static void closeJoystick(_GLFWjoystick* js) { JoystickConnection(js,0x00040002/*disconnected*/); _glfwFreeJoystick(js); }
    void _glfwDetectJoystickConnectionWin32(void) {
        if (_glfw.win32.xinput.instance) {
            for (u32 index=0;index<4;index++) {
                int jid; char guid[33]; XINPUT_CAPABILITIES xic; _GLFWjoystick* js;
                for (jid = 0;  jid <= GLFW_JOYSTICK_LAST;  jid++) {
                    if (_glfw.joysticks[jid].connected && _glfw.joysticks[jid].win32.index == index) break;
                }

                if (jid <= GLFW_JOYSTICK_LAST) continue;
                if (_glfw.win32.xinput.GetCapabilities(index,0,&xic) != 0) continue;

                StringFormat(guid,sizeof(guid),"78696e707574%02x000000000000000000",xic.SubType & 0xff);
                js = _glfwAllocJoystick("Gamepad", guid, 6, 10, 1);
                if (!js) continue;

                js->win32.index = index;
                JoystickConnection(js,0x00040001/*connected*/);
            }
        }
    }

    i32 InitJoysticks(void) { _glfwDetectJoystickConnectionWin32(); return  1; }
    i32 PollJoystick(_GLFWjoystick* js) {
        u32 result; XINPUT_STATE xis;
        const u16 buttons[14] = {0x0001/*XINPUT_GAMEPAD_DPAD_UP*/,0x0002/*XINPUT_GAMEPAD_DPAD_DOWN*/,0x0008/*XINPUT_GAMEPAD_DPAD_RIGHT*/,0x0004/*XINPUT_GAMEPAD_DPAD_LEFT*/,0x1000/*XINPUT_GAMEPAD_A*/,0x2000/*XINPUT_GAMEPAD_B*/,0x4000/*XINPUT_GAMEPAD_X*/,0x8000/*XINPUT_GAMEPAD_Y*/,0x0100/*XINPUT_GAMEPAD_LEFT_SHOULDER*/,0x0200/*XINPUT_GAMEPAD_RIGHT_SHOULDER*/,0x0020/*XINPUT_GAMEPAD_BACK*/,0x0010/*XINPUT_GAMEPAD_START*/,0x0040/*XINPUT_GAMEPAD_LEFT_THUMB*/,0x0080/*XINPUT_GAMEPAD_RIGHT_THUMB*/};
        result = _glfw.win32.xinput.GetState(js->win32.index, &xis);
        if (result != 0) { if (result == 1167/*not connected*/) {closeJoystick(js);} return 0; }

        const i16 axis_vals[] = {xis.Gamepad.sThumbLX,-xis.Gamepad.sThumbLY,xis.Gamepad.sThumbRX,-xis.Gamepad.sThumbRY};
        for (int i=0;i<4;++i) InputJoystickAxis(js,i,(axis_vals[i] + 0.5f) / 32767.5f);
        InputJoystickAxis(js,4,xis.Gamepad.bLeftTrigger / 127.5f - 1.f); InputJoystickAxis(js,5,xis.Gamepad.bRightTrigger / 127.5f - 1.f);
        for (int i=0;i<10;++i) { const char value = (xis.Gamepad.wButtons & buttons[i]) ? 1 : 0; InputJoystickButton(js,i,value); }
        int dpad = ((const int[]){0,1,2,3,4,0,0,0,8,0,0,0,0,0,0,0})[xis.Gamepad.wButtons & 0xF];
        if ((dpad & GLFW_HAT_RIGHT) && (dpad & GLFW_HAT_LEFT)) dpad &= ~(GLFW_HAT_RIGHT | GLFW_HAT_LEFT);
        if ((dpad & GLFW_HAT_UP) && (dpad & GLFW_HAT_DOWN)) dpad &= ~(GLFW_HAT_UP | GLFW_HAT_DOWN);
        InputJoystickHat(js, 0, dpad);
        return  1;
    }
    
    void _glfwDetectJoystickDisconnectionWin32(void) { for (int jid = 0;  jid <= GLFW_JOYSTICK_LAST;  jid++) { _GLFWjoystick* js = _glfw.joysticks + jid; if (js->connected) {PollJoystick(js);} } }
    static i32 __stdcall monitorCallback(HMONITOR handle, HDC dc, RECT* rect, i64 data) {
        MONITORINFOEXW mi; (void)dc; (void)rect;
        MemSetToVForNBytes(&mi,0,sizeof(mi));
        mi.cbSize = sizeof(mi);
        if (GetMonitorInfoW(handle, (MONITORINFO*) &mi)) {
            _GLFWmonitor* monitor = (_GLFWmonitor*) data;
            if (wcscmp(mi.szDevice, monitor->win32.adapterName) == 0) monitor->win32.handle = handle;
        }

        return 1;
    }

    static _GLFWmonitor* createMonitor(DISPLAY_DEVICEW* adapter, DISPLAY_DEVICEW* display) {
        _GLFWmonitor* monitor; int widthMM,heightMM,nameSize=0; HDC dc; DEVMODEW dm; RECT rect;
        char* name = CreateUTF8FromWideStringWin32(display ? display->DeviceString : adapter->DeviceString,&nameSize);
        MemSetToVForNBytes(&dm,0,sizeof(dm)); dm.dmSize = sizeof(dm);
        EnumDisplaySettingsW(adapter->DeviceName,0xFFFFFFFFU,&dm);
        dc = CreateDCW(L"DISPLAY", adapter->DeviceName,NULL,NULL);
        if (IsWindowsVersionOrGreaterWin32(HIBYTE(0x0603),LOBYTE(0x0603),0)) { widthMM  = GetDeviceCaps(dc,4); heightMM = GetDeviceCaps(dc,6); } // Is Windows 8.10 or greater
        else { widthMM  = (int) (dm.dmPelsWidth * 25.4f / GetDeviceCaps(dc,88)); heightMM = (int) (dm.dmPelsHeight * 25.4f / GetDeviceCaps(dc,90)); }

        DeleteDC(dc); monitor = AllocMonitor(name,widthMM,heightMM); OS_DeallocateRAM(name,nameSize);
        if (adapter->StateFlags & 0x08000000/*DISPLAY_DEVICE_MODESPRUNED*/) monitor->win32.modesPruned =  1;
        wcscpy(monitor->win32.adapterName, adapter->DeviceName);
        if (display) wcscpy(monitor->win32.displayName,display->DeviceName);
        rect.left=dm.dmPosition.x; rect.top=dm.dmPosition.y; rect.right=dm.dmPosition.x + dm.dmPelsWidth; rect.bottom=dm.dmPosition.y + dm.dmPelsHeight;
        EnumDisplayMonitors(NULL,&rect,monitorCallback,(i64)monitor);
        return monitor;
    }

    void _glfwPollMonitorsWin32(void) {
        int i, disconnectedCount = _glfw.monitorCount; _GLFWmonitor** disconnected = NULL; u32 adapterIndex,displayIndex; DISPLAY_DEVICEW adapter, display; _GLFWmonitor* monitor;
        if (disconnectedCount) { disconnected = OS_Calloc(_glfw.monitorCount,sizeof(_GLFWmonitor*)); CopyMemoryFromBtoAForNBytes(disconnected,_glfw.monitors,_glfw.monitorCount * sizeof(_GLFWmonitor*)); }
        for (adapterIndex = 0;;adapterIndex++) {
            int type = 1; MemSetToVForNBytes(&adapter,0,sizeof(adapter)); adapter.cb = sizeof(adapter);
            if (!EnumDisplayDevicesW(NULL, adapterIndex, &adapter, 0)) break;
            if (!(adapter.StateFlags&1)) continue;

            if (adapter.StateFlags & 0x00000004/*DISPLAY_DEVICE_PRIMARY_DEVICE*/) type = 0;
            for (displayIndex=0;;++displayIndex) {
                MemSetToVForNBytes(&display,0,sizeof(display)); display.cb = sizeof(display);
                if (!EnumDisplayDevicesW(adapter.DeviceName, displayIndex, &display, 0)) break;
                if (!(display.StateFlags&1)) continue;

                for (i=0;i<disconnectedCount;++i) {
                    if (disconnected[i] && wcscmp(disconnected[i]->win32.displayName,display.DeviceName) == 0) {
                        disconnected[i] = NULL;
                        EnumDisplayMonitors(NULL,NULL,monitorCallback,(i64)_glfw.monitors[i]);
                        break;
                    }
                }

                if (i < disconnectedCount) continue;
                monitor = createMonitor(&adapter,&display); if (!monitor) { OS_DeallocateRAM(disconnected,_glfw.monitorCount*sizeof(_GLFWmonitor*)); return; }

                InputMonitor(monitor,0x00040001/*connected*/,type); type = 1;
            }

            if (displayIndex == 0) {
                for (i=0;i<disconnectedCount;++i) { if (disconnected[i] && wcscmp(disconnected[i]->win32.adapterName,adapter.DeviceName) == 0) {disconnected[i]=NULL; break;} }
                if (i < disconnectedCount) continue;
                monitor = createMonitor(&adapter,NULL); if (!monitor) { OS_DeallocateRAM(disconnected,_glfw.monitorCount*sizeof(_GLFWmonitor*)); return; }

                InputMonitor(monitor, 0x00040001/*connected*/, type);
            }
        }

        for (i=0;i<disconnectedCount;++i) { if (disconnected[i]) {InputMonitor(disconnected[i],0x00040002/*disconnected*/,0);} }
        if (disconnected) OS_DeallocateRAM(disconnected,_glfw.monitorCount*sizeof(_GLFWmonitor*));
    }
    
    static i64 __stdcall helperWindowProc(HWND hWnd, u32 uMsg, u64 wParam, i64 lParam) {
        switch (uMsg) {
            case 0x007E/*WM_DISPLAYCHANGE*/: _glfwPollMonitorsWin32(); break;
            case 0x0219/*WM_DEVICECHANGE*/: if (!_glfw.joysticksInitialized) break;
                if (wParam == 0x8000/*DBT_DEVICEARRIVAL*/ || wParam == 0x8004/*DBT_DEVICEREMOVECOMPLETE*/) {
                    DEV_BROADCAST_HDR* dbh = (DEV_BROADCAST_HDR*) lParam;
                    if (dbh && dbh->dbch_devicetype == 0x0005/*DBT_DEVTYP_DEVICEINTERFACE*/ && wParam == 0x8000/*DBT_DEVICEARRIVAL*/)           _glfwDetectJoystickConnectionWin32();
                    if (dbh && dbh->dbch_devicetype == 0x0005/*DBT_DEVTYP_DEVICEINTERFACE*/ && wParam == 0x8004/*DBT_DEVICEREMOVECOMPLETE*/) _glfwDetectJoystickDisconnectionWin32();
                }

                break;
        }

        return DefWindowProcW(hWnd,uMsg,wParam,lParam);
    }

    void GetMonitorPos(_GLFWmonitor* monitor, int* xpos, int* ypos) { DEVMODEW dm; MemSetToVForNBytes(&dm,0,sizeof(dm)); dm.dmSize = sizeof(dm); EnumDisplaySettingsExW(monitor->win32.adapterName,0xFFFFFFFFU,&dm,0x00000004); *xpos = dm.dmPosition.x; *ypos = dm.dmPosition.y; }
    void GetMonitorWorkarea(_GLFWmonitor* monitor, int* xpos, int* ypos, int* width, int* height) { MONITORINFO mi = {0}; mi.cbSize = sizeof(mi); GetMonitorInfoW(monitor->win32.handle, &mi); *xpos = mi.rcWork.left; *ypos = mi.rcWork.top; *width = mi.rcWork.right - mi.rcWork.left; *height = mi.rcWork.bottom - mi.rcWork.top; }
    void GetVideoMode(_GLFWmonitor* monitor, GLFWvidmode* mode) { DEVMODEW dm; MemSetToVForNBytes(&dm,0,sizeof(dm)); dm.dmSize = sizeof(dm); EnumDisplaySettingsW(monitor->win32.adapterName,0xFFFFFFFFU,&dm); mode->width=dm.dmPelsWidth; mode->height=dm.dmPelsHeight; mode->refreshRate=dm.dmDisplayFrequency; }
    static int choosePixelFormatWGL(_GLFWwindow* window) {
        int attribs[24],values[24],attribCount=0,i,pixelFormat,nativeCount,usableCount=0;
        const int query = 0x2000/*num pixel formats*/; _glfw.wgl.GetPixelFormatAttribivARB(window->context.wgl.dc,1,0,1,&query,&nativeCount);
        attribs[attribCount++] = 0x2010/*support opengl*/; attribs[attribCount++] = 0x2001/*draw to window*/; attribs[attribCount++] = 0x2013/*pixel type*/; attribs[attribCount++] = 0x2003/*accelaration*/;
        attribs[attribCount++] = 0x2011/*double buffer*/; attribs[attribCount++] = 0x2015/*r bits*/; attribs[attribCount++] = 0x2017/*g bits*/;
        attribs[attribCount++] = 0x2019/*b bits*/; attribs[attribCount++] = 0x201b/*a bits*/; attribs[attribCount++] = 0x2022/*depth bits*/; attribs[attribCount++] = 0x2023/*stencil bits*/;
        _GLFWfbconfig* usableConfigs = OS_Calloc(nativeCount,sizeof(_GLFWfbconfig));
        for (i = 0; i < nativeCount; i++) {
            _GLFWfbconfig* u = usableConfigs + usableCount; pixelFormat = i + 1;
            _glfw.wgl.GetPixelFormatAttribivARB(window->context.wgl.dc,pixelFormat,0,attribCount,attribs,values);
            if (values[0] == 0 || values[1] == 0/* support OpenGL + draw to window */ || values[2] != 0x202b/*type rgba*/ || values[3] == 0x2025/*no accel*/ || values[4] !=  1) continue;
            
            u->redBits=values[5]; u->greenBits=values[6]; u->blueBits=values[7]; u->alphaBits=values[8]; u->depthBits=values[9]; u->stencilBits=values[10]; u->handle=pixelFormat; usableCount++;
        }

        const _GLFWfbconfig* closest = _glfwChooseFBConfig(usableConfigs,usableCount);
        pixelFormat = (int)closest->handle; OS_DeallocateRAM(usableConfigs,nativeCount * sizeof(_GLFWfbconfig));
        return pixelFormat;
    }

    static void makeContextCurrentWGL(_GLFWwindow* window) { wglMakeCurrent(window->context.wgl.dc,window->context.wgl.handle); }
    static void swapBuffersWGL(_GLFWwindow* window) {
        if (!IsWindowsVersionOrGreaterWin32(HIBYTE(0x0602),LOBYTE(0x0602),0)) { i32 enabled = 0; if (SUCCEEDED(_glfw.win32.dwmapi.IsCompositionEnabled(&enabled)) && enabled) { int count = vabs(window->context.wgl.interval); while (count--) {_glfw.win32.dwmapi.Flush();} } } // Is Windows 8.0 or greater
        SwapBuffers(window->context.wgl.dc);
    }

    static void swapIntervalWGL(int interval) {
        _GLFWwindow* handle = (_GLFWwindow*)window;
        handle->context.wgl.interval = interval;
        if (!IsWindowsVersionOrGreaterWin32(HIBYTE(0x0602),LOBYTE(0x0602),0)) { i32 enabled = 0; if (SUCCEEDED(_glfw.win32.dwmapi.IsCompositionEnabled(&enabled)) && enabled) interval = 0; } // Is Windows 8.0 or greater
        _glfw.wgl.SwapIntervalEXT(interval);
    }

    static GLFWglproc getProcAddressWGL(const char* procname) { const GLFWglproc proc = (GLFWglproc)wglGetProcAddress(procname); if (proc) {return proc;} return (GLFWglproc)PlatformGetModuleSymbol(_glfw.wgl.instance,procname); }
    void glfwSetWindowPosition(GLFWwindow* handle, int xpos, int ypos) { _GLFWwindow* window = (_GLFWwindow*)handle; RECT rect = {xpos,ypos,xpos,ypos}; AdjustWindowRectEx(&rect,getWindowStyle(window),0,0x00040000/*WS_EX_APPWINDOW*/); SetWindowPos(window->win32.handle,((HWND)0),rect.left,rect.top,0,0,0x0010|0x0200|0x0001|0x0004); }
#else // LINUX
    typedef unsigned char KeyCode; typedef int Bool; typedef int Status; typedef unsigned long Atom; typedef unsigned long KeySym;
    typedef char *XPointer;
    typedef unsigned int XcursorUInt; typedef struct _XcursorImage { XcursorUInt version; XcursorUInt size,width,height,xhot,yhot; XcursorUInt delay; XcursorUInt *pixels; } XcursorImage;
    typedef unsigned short Rotation,SubpixelOrder,Connection;
    typedef struct { long flags; int x,y, width,height,min_width,min_height,max_width,max_height,width_inc,height_inc; struct {int x; int y;} min_aspect,max_aspect; int base_width, base_height; int win_gravity; } XSizeHints;
    typedef unsigned long XID,Mask,Atom,VisualID,Time;
    typedef XID Window,Drawable,Font,Pixmap,Cursor,Colormap;
    typedef struct _XExtData { int number; struct _XExtData *next; int (*free_private)(struct _XExtData*); XPointer private_data; } XExtData;
    typedef struct { int extension, major_opcode, first_event, first_error; } XExtCodes;
    typedef struct { int depth, bits_per_pixel, scanline_pad; } XPixmapFormatValues;
    typedef struct _XGC *GC;
    typedef struct { XExtData *ext_data; VisualID visualid; int class; u64 red_mask, green_mask, blue_mask; int bits_per_rgb; int map_entries;} Visual;
    typedef struct { int depth,nvisuals; Visual *visuals; } Depth;
    typedef struct { XExtData *ext_data; struct _XDisplay *display; Window root; int width,height,mwidth,mheight,ndepths; Depth *depths; int root_depth; Visual *root_visual; GC default_gc; Colormap cmap; u64 white_pixel, black_pixel; int max_maps, min_maps, backing_store; int save_unders; i64 root_input_mask; } Screen;
    typedef struct { XExtData *ext_data; int depth, bits_per_pixel, scanline_pad; } ScreenFormat;
    typedef struct { Pixmap background_pixmap; u64 background_pixel; Pixmap border_pixmap; u64 border_pixel; int bit_gravity, win_gravity, backing_store; u64 backing_planes, backing_pixel; int save_under; i64 event_mask, do_not_propagate_mask; int override_redirect; Colormap colormap; Cursor cursor; } XSetWindowAttributes;
    typedef struct { int x,y,width,height,border_width,depth; Visual *visual; Window root; int class,bit_gravity,win_gravity,backing_store; unsigned long backing_planes, backing_pixel; int save_under; Colormap colormap; int map_installed; int map_state; i64 all_event_masks, your_event_mask, do_not_propagate_mask; Bool override_redirect; Screen *screen; } XWindowAttributes;
    typedef struct _XDisplay Display;
    typedef struct { XExtData *ext_data; struct _XPrivate *private1; int fd, private2, proto_major_version, proto_minor_version; char *vendor; XID private3, private4, private5; int private6; XID (*resource_alloc)(struct _XDisplay*); int byte_order, bitmap_unit, bitmap_pad, bitmap_bit_order, nformats; ScreenFormat *pixmap_format; int private8; struct _XPrivate *private9, *private10; int qlen; unsigned long last_request_read, request; XPointer private11, private12, private13, private14; unsigned max_request_size; struct _XrmHashBucketRec *db; int (*private15)(struct _XDisplay*); char *display_name; int default_screen, nscreens; Screen *screens; unsigned long motion_buffer, private16; int min_keycode, max_keycode; XPointer private17, private18; int private19; char *xdefaults; } *_XPrivDisplay;
    typedef struct { int a; u64 b; int c; void *d; u64 e,f,g,h; int i,j,k,l; u32 m,keycode; int n; } XKeyEvent;
    typedef struct { int a; u64 b; int c; Display *d; Window e,f,g; Time h; int i,j,k,l; u32 m,button; int n; } XButtonEvent;
    typedef struct { int a; u64 b; int c; Display *d; Window e, f, g; Time h; int x,y,i,j; u32 k; char l; int m; } XMotionEvent;
    typedef struct { int type; u64 serial; int send_event; Display *display; Window window, root, subwindow; Time time; int x,y,x_root,y_root,mode,detail; int same_screen, focus; u32 state; } XCrossingEvent;
    typedef struct { int type; u64 serial; int send_event; Display *display; Window window; int mode, detail; } XFocusChangeEvent;
    typedef struct { int type; u64 serial; int send_event; Display *display; Window window; char key_vector[32]; } XKeymapEvent;
    typedef struct { int type; u64 serial; Bool send_event; Display *display; Window window; int x, y, width, height, count; } XExposeEvent;
    typedef struct { int type; u64 serial; Bool send_event; Display *display; Drawable drawable; int x, y, width, height, count, major_code, minor_code; } XGraphicsExposeEvent;
    typedef struct { int type; u64 serial; Bool send_event; Display *display; Drawable drawable; int major_code, minor_code; } XNoExposeEvent;
    typedef struct { int type; u64 serial; Bool send_event; Display *display; Window window; int state; } XVisibilityEvent;
    typedef struct { int type; u64 serial; Bool send_event; Display *display; Window parent, window; int x, y, width, height, border_width; int override_redirect; } XCreateWindowEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Window event, window; } XDestroyWindowEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Window event, window; int from_configure; } XUnmapEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Window event, window; int override_redirect; } XMapEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Window parent, window; } XMapRequestEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Window event, window, parent; int x, y; int override_redirect; } XReparentEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Window event, window; int x, y, width, height, border_width; Window above; int override_redirect; } XConfigureEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Window event, window; int x, y; } XGravityEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Window window; int width, height; } XResizeRequestEvent;
    typedef struct { int type; unsigned long serial; Bool send_event; Display *display; Window parent, window; int x, y, width, height, border_width; Window above; int detail; unsigned long value_mask; } XConfigureRequestEvent;
    typedef struct { int type; unsigned long serial; int send_event; Display *display; Window event, window; int place; } XCirculateEvent;
    typedef struct { int type; unsigned long serial; int send_event; Display *display; Window parent, window; int place; } XCirculateRequestEvent;
    typedef struct { int type; unsigned long serial; int send_event; Display *display; Window window; Atom atom; Time time; int state; } XPropertyEvent;
    typedef struct { int type; unsigned long serial; int send_event; Display *display; Window window; Atom selection; Time time; } XSelectionClearEvent;
    typedef struct { int type; unsigned long serial; int send_event; Display *display; Window owner, requestor; Atom selection, target, property; Time time; } XSelectionRequestEvent;
    typedef struct { int type; unsigned long serial; int send_event; Display *display; Window requestor; Atom selection, target, property; Time time; } XSelectionEvent;
    typedef struct { int type; u64 serial; int send_event; Display *display; Window window; Colormap colormap; int new; int state; } XColormapEvent;
    typedef struct { int type; u64 serial; int send_event; Display *display; Window window; Atom message_type; int format; union { char b[20]; short s[10]; long l[5]; } data; } XClientMessageEvent;
    typedef struct { int type; u64 serial; int send_event; Display *display; Window window; int request, first_keycode, count; } XMappingEvent;
    typedef struct { int type; Display *display; XID resourceid; u64 serial; unsigned char error_code, request_code, minor_code; } XErrorEvent;
    typedef struct { int a; u64 b; int send_event; Display *c; Window window; } XAnyEvent;
    typedef struct { int type; u64 serial; int send_event; Display *display; int extension, evtype; } XGenericEvent;
    typedef struct { int type; u64 serial; int send_event; Display *display; int extension, evtype; u32 cookie; void *data; } XGenericEventCookie;
    typedef union _XEvent { int type; XAnyEvent xany; XKeyEvent xkey; XButtonEvent xbutton; XMotionEvent xmotion; XCrossingEvent xcrossing; XFocusChangeEvent xfocus; XExposeEvent xexpose; XGraphicsExposeEvent xgraphicsexpose; XNoExposeEvent xnoexpose; XVisibilityEvent xvisibility; XCreateWindowEvent xcreatewindow; XDestroyWindowEvent xdestroywindow; XUnmapEvent xunmap; XMapEvent xmap; XMapRequestEvent xmaprequest; XReparentEvent xreparent; XConfigureEvent xconfigure; XGravityEvent xgravity; XResizeRequestEvent xresizerequest; XConfigureRequestEvent xconfigurerequest; XCirculateEvent xcirculate; XCirculateRequestEvent xcirculaterequest; XPropertyEvent xproperty; XSelectionClearEvent xselectionclear; XSelectionRequestEvent xselectionrequest; XSelectionEvent xselection; XColormapEvent xcolormap; XClientMessageEvent xclient; XMappingEvent xmapping; XErrorEvent xerror; XKeymapEvent xkeymap; XGenericEvent xgeneric; XGenericEventCookie xcookie; long pad[24]; } XEvent;
    typedef struct _XIC *XIC;
    typedef struct { Visual *visual; VisualID visualid; int screen,depth; int class; u64 red_mask,green_mask,blue_mask; int colormap_size,bits_per_rgb; } XVisualInfo;
    typedef int XContext; typedef XID RROutput,RRCrtc,RRMode; typedef u64 XRRModeFlags;
    typedef struct _XRRModeInfo { RRMode id; u32 width,height; u64 dotClock; u32 hSyncStart,hSyncEnd,hTotal,hSkew,vSyncStart,vSyncEnd,vTotal; char *name; u32 nameLength; XRRModeFlags modeFlags; } XRRModeInfo;
    typedef struct _XRRScreenResources { Time timestamp; Time configTimestamp; int ncrtc; RRCrtc *crtcs; int noutput; RROutput *outputs; int nmode; XRRModeInfo *modes; } XRRScreenResources;
    typedef struct _XRROutputInfo { Time timestamp; RRCrtc crtc; char *name; int nameLen; unsigned long mm_width; unsigned long mm_height; Connection connection; SubpixelOrder subpixel_order; int ncrtc; RRCrtc *crtcs; int nclone; RROutput *clones; int nmode; int npreferred; RRMode *modes; } XRROutputInfo;
    typedef struct _XRRCrtcInfo { Time timestamp; int x, y; unsigned int width, height; RRMode mode; Rotation rotation; int noutput; RROutput *outputs; Rotation rotations; int npossible; RROutput *possible; } XRRCrtcInfo;
    typedef XID GLXWindow,GLXDrawable; typedef struct __GLXFBConfig* GLXFBConfig; typedef struct __GLXcontext* GLXContext;
    typedef void(*__GLXextproc)(void);                                      typedef XSizeHints*(*PFN_XAllocSizeHints)(void);                           typedef int(*PFN_XChangeProperty)(Display*,Window,Atom,Atom,int,int,const unsigned char*,int);
    typedef Bool(*PFN_XCheckTypedWindowEvent)(Display*,Window,int,XEvent*); typedef void(*PFN_XRRFreeOutputInfo)(XRROutputInfo*);                      typedef Colormap(*PFN_XCreateColormap)(Display*,Window,Visual*,int);
    typedef int(*PFN_XDefineCursor)(Display*,Window,Cursor);                typedef int(*PFN_XDeleteProperty)(Display*,Window,Atom);                   typedef Window(*PFN_XCreateWindow)(Display*,Window,int,int,unsigned int,unsigned int,unsigned int,int,unsigned int,Visual*,unsigned long,XSetWindowAttributes*);
    typedef int(*PFN_XDisplayKeycodes)(Display*,int*,int*);                 typedef Bool(*PFN_XFilterEvent)(XEvent*,Window);                           typedef int(*PFN_XFindContext)(Display*,XID,XContext,XPointer*);
    typedef int(*PFN_XFree)(void*);                                         typedef void(*PFN_XFreeEventData)(Display*,XGenericEventCookie*);          typedef int(*PFN_XGrabPointer)(Display*,Window,Bool,unsigned int,int,int,Window,Cursor,Time);
    typedef KeySym*(*PFN_XGetKeyboardMapping)(Display*,KeyCode,int,int*);   typedef Status(*PFN_XGetWMNormalHints)(Display*,Window,XSizeHints*,long*); typedef Status(*PFN_XGetWindowAttributes)(Display*,Window,XWindowAttributes*);
    typedef Atom(*PFN_XInternAtom)(Display*,const char*,Bool);              typedef int(*PFN_XGetInputFocus)(Display*,Window*,int*);                   typedef int(*PFN_XGetWindowProperty)(Display*,Window,Atom,long,long,Bool,Atom,Atom*,int*,unsigned long*,unsigned long*,unsigned char**); 
    typedef int(*PFN_XMapWindow)(Display*,Window);                          typedef int(*PFN_XMoveWindow)(Display*,Window,int,int);                    typedef int(*PFN_XMoveResizeWindow)(Display*,Window,int,int,unsigned int,unsigned int);
    typedef Status(*PFN_XInitThreads)(void);                                typedef int(*PFN_XNextEvent)(Display*,XEvent*);                            typedef XRRCrtcInfo*(*PFN_XRRGetCrtcInfo)(Display*,XRRScreenResources*,RRCrtc);
    typedef int(*PFN_XPending)(Display*);                                   typedef Bool(*PFN_XQueryExtension)(Display*,const char*,int*,int*,int*);   typedef Bool(*PFN_XQueryPointer)(Display*,Window,Window*,Window*,int*,int*,int*,int*,unsigned int*);
    typedef int(*PFN_XRaiseWindow)(Display*,Window);                        typedef int(*PFN_XSaveContext)(Display*,XID,XContext,const char*);         typedef int(*PFN_XResizeWindow)(Display*,Window,unsigned int,unsigned int);
    typedef Status(*PFN_XSendEvent)(Display*,Window,Bool,long,XEvent*);     typedef void(*PFN_XSetICFocus)(XIC);                                       typedef int(*PFN_XSetInputFocus)(Display*,Window,int,Time);
    typedef void(*PFN_XSetWMNormalHints)(Display*,Window,XSizeHints*);      typedef Status(*PFN_XSetWMProtocols)(Display*,Window,Atom*,int);           typedef Bool(*PFN_XTranslateCoordinates)(Display*,Window,Window,int,int,int*,int*,Window*);
    typedef int(*PFN_XUndefineCursor)(Display*,Window);                     typedef void(*PFN_XUnsetICFocus)(XIC);                                     typedef int(*PFN_XWarpPointer)(Display*,Window,Window,int,int,unsigned int,unsigned int,int,int);
    typedef void(*PFN_XRRFreeCrtcInfo)(XRRCrtcInfo*);                       typedef int(*PFN_XUngrabPointer)(Display*,Time);                           typedef int(*PFN_XChangeWindowAttributes)(Display*,Window,unsigned long,XSetWindowAttributes*); 
    typedef void(*PFN_XRRFreeScreenResources)(XRRScreenResources*);         typedef Display*(*PFN_XOpenDisplay)(const char*);                          typedef XRROutputInfo*(*PFN_XRRGetOutputInfo)(Display*,XRRScreenResources*,RROutput);
    typedef RROutput(*PFN_XRRGetOutputPrimary)(Display*,Window);            typedef void(*PFN_XRRSelectInput)(Display*,Window,int);                    typedef XRRScreenResources*(*PFN_XRRGetScreenResourcesCurrent)(Display*,Window);
    typedef int(*PFN_XRRUpdateConfiguration)(XEvent*);                      typedef XcursorImage*(*PFN_XcursorImageCreate)(int,int);                   typedef void(*PFN_XcursorImageDestroy)(XcursorImage*);
    typedef Bool(*PFNGLXQUERYEXTENSIONPROC)(Display*,int*,int*);            typedef int(*PFNGLXGETFBCONFIGATTRIBPROC)(Display*,GLXFBConfig,int,int*);  typedef Cursor(*PFN_XcursorImageLoadCursor)(Display*,const XcursorImage*);
    typedef Bool(*PFNGLXQUERYVERSIONPROC)(Display*,int*,int*);              typedef Bool(*PFNGLXMAKECURRENTPROC)(Display*,GLXDrawable,GLXContext);     typedef void(*PFNGLXSWAPBUFFERSPROC)(Display*,GLXDrawable);
    typedef const char*(*PFNGLXQUERYEXTENSIONSSTRINGPROC)(Display*,int);    typedef GLXFBConfig*(*PFNGLXGETFBCONFIGSPROC)(Display*,int,int*);          typedef GLXContext(*PFNGLXCREATENEWCONTEXTPROC)(Display*,GLXFBConfig,int,GLXContext,Bool);
    typedef __GLXextproc(*PFNGLXGETPROCADDRESSPROC)(const u8*);             typedef void(*PFNGLXSWAPINTERVALEXTPROC)(Display*,GLXDrawable,int);        typedef XVisualInfo*(*PFNGLXGETVISUALFROMFBCONFIGPROC)(Display*,GLXFBConfig);
    typedef GLXWindow(*PFNGLXCREATEWINDOWPROC)(Display*,GLXFBConfig,Window,const int*); typedef GLXContext(*PFNGLXCREATECONTEXTATTRIBSARBPROC)(Display*,GLXFBConfig,GLXContext,Bool,const int*);
    typedef struct _GLFWcontextGLX { GLXContext handle; GLXWindow window; GLXFBConfig fbconfig; } _GLFWcontextGLX;
    typedef struct _GLFWlibraryGLX { int major,minor,eventBase,errorBase; void* handle; PFNGLXGETFBCONFIGSPROC GetFBConfigs; PFNGLXGETFBCONFIGATTRIBPROC GetFBConfigAttrib; PFNGLXQUERYEXTENSIONPROC QueryExtension; PFNGLXQUERYVERSIONPROC QueryVersion; PFNGLXMAKECURRENTPROC MakeCurrent; PFNGLXSWAPBUFFERSPROC SwapBuffers;
                                     PFNGLXQUERYEXTENSIONSSTRINGPROC QueryExtensionsString; PFNGLXCREATENEWCONTEXTPROC CreateNewContext; PFNGLXGETVISUALFROMFBCONFIGPROC GetVisualFromFBConfig; PFNGLXCREATEWINDOWPROC CreateWindow; PFNGLXGETPROCADDRESSPROC GetProcAddress; PFNGLXSWAPINTERVALEXTPROC SwapIntervalEXT;
                                     PFNGLXCREATECONTEXTATTRIBSARBPROC CreateContextAttribsARB; } _GLFWlibraryGLX;
                                     
    typedef struct _GLFWwindowX11 { Colormap colormap; Window handle,parent; XIC ic; i32 overrideRedirect; int width,height,xpos,ypos,lastCursorPosX,lastCursorPosY,warpCursorPosX,warpCursorPosY; } _GLFWwindowX11;
    typedef struct _GLFWlibraryX11 { Display* display; int screen; Window root; Cursor hiddenCursorHandle; XContext context; short int keycodes[256],scancodes[349]; double restoreCurPosX, restoreCurPosY; _GLFWwindow* disabledCursorWindow;
                                     Atom NET_SUPPORTED,NET_SUPPORTING_WM_CHECK,WM_PROTOCOLS,WM_STATE,WM_DELETE_WINDOW,NET_WM_NAME,NET_WM_ICON,NET_WM_PING,NET_WM_WINDOW_TYPE,NET_WM_WINDOW_TYPE_NORMAL,NET_WM_STATE,NET_WM_STATE_FULLSCREEN,NET_WM_BYPASS_COMPOSITOR,NET_WORKAREA,NET_CURRENT_DESKTOP,NET_ACTIVE_WINDOW,MOTIF_WM_HINTS,UTF8_STRING;
                                     struct { void* handle; i32 utf8; PFN_XAllocSizeHints AllocSizeHints; PFN_XChangeProperty ChangeProperty; PFN_XChangeWindowAttributes ChangeWindowAttributes; PFN_XCheckTypedWindowEvent CheckTypedWindowEvent; PFN_XCreateColormap CreateColormap; PFN_XCreateWindow CreateWindow; PFN_XDefineCursor DefineCursor;
                                     PFN_XDeleteProperty DeleteProperty; PFN_XDisplayKeycodes DisplayKeycodes; PFN_XFilterEvent FilterEvent; PFN_XFindContext FindContext; PFN_XFree Free; PFN_XFreeEventData FreeEventData; PFN_XGetInputFocus GetInputFocus; PFN_XGetKeyboardMapping GetKeyboardMapping; PFN_XGetWMNormalHints GetWMNormalHints;
                                     PFN_XGetWindowAttributes GetWindowAttributes; PFN_XGetWindowProperty GetWindowProperty; PFN_XGrabPointer GrabPointer; PFN_XInternAtom InternAtom; PFN_XMapWindow MapWindow; PFN_XMoveResizeWindow MoveResizeWindow; PFN_XMoveWindow MoveWindow; PFN_XPending Pending; PFN_XQueryExtension QueryExtension;
                                     PFN_XQueryPointer QueryPointer; PFN_XRaiseWindow RaiseWindow; PFN_XResizeWindow ResizeWindow; PFN_XSaveContext SaveContext; PFN_XSendEvent SendEvent; PFN_XSetICFocus SetICFocus; PFN_XSetInputFocus SetInputFocus; PFN_XSetWMNormalHints SetWMNormalHints; PFN_XSetWMProtocols SetWMProtocols;
                                     PFN_XTranslateCoordinates TranslateCoordinates; PFN_XUndefineCursor UndefineCursor; PFN_XUngrabPointer UngrabPointer; PFN_XUnsetICFocus UnsetICFocus; PFN_XWarpPointer WarpPointer; } xlib;
                                     struct {void* handle; int eventBase,errorBase,major,minor; PFN_XRRFreeCrtcInfo FreeCrtcInfo; PFN_XRRFreeOutputInfo FreeOutputInfo; PFN_XRRFreeScreenResources FreeScreenResources; PFN_XRRGetCrtcInfo GetCrtcInfo; PFN_XRRGetOutputInfo GetOutputInfo; PFN_XRRGetOutputPrimary GetOutputPrimary;
                                             PFN_XRRGetScreenResourcesCurrent GetScreenResourcesCurrent; PFN_XRRSelectInput SelectInput; PFN_XRRUpdateConfiguration UpdateConfiguration;}randr;
                                     struct { void* handle; PFN_XcursorImageCreate ImageCreate; PFN_XcursorImageDestroy ImageDestroy; PFN_XcursorImageLoadCursor ImageLoadCursor; } xcursor; } _GLFWlibraryX11; 
    PFN_XNextEvent XNextEvent;
    typedef struct _GLFWmonitorX11 { RROutput output; RRCrtc crtc; int index; } _GLFWmonitorX11;
    typedef struct _GLFWjoystickLinux { FHandle fd; char path[260]; int keyMap[0x300/*KEY_CNT*/ - 0x100/*BTN_MISC*/],absMap[0x40/*ABS_CNT*/]; struct input_absinfo absInfo[0x40/*ABS_CNT*/]; int hats[4][2]; } _GLFWjoystickLinux;
    typedef struct _GLFWlibraryLinux { int inotify,watch; i32 dropped; } _GLFWlibraryLinux;
    void GetCursorPosV(_GLFWwindow* window, double* xpos, double* ypos);
    void SetCursorPosV(_GLFWwindow* window, double xpos, double ypos);
    struct _GLFWjoystick { i32 allocated,connected; size_t axesSize,buttonsSize,hatsSize; float*  axes; int axisCount; unsigned char* buttons; int buttonCount; unsigned char* hats; int hatCount; char name[128],guid[33]; _GLFWjoystickLinux linjs; };
    struct _GLFWlibrary { _GLFWmonitor** monitors; int monitorCount; i32 joysticksInitialized; _GLFWjoystick joysticks[GLFW_JOYSTICK_LAST + 1]; _GLFWlibraryX11 x11; _GLFWlibraryGLX glx; _GLFWlibraryLinux linjs; };
    struct _GLFWcontext { int client,source,major,minor; PFNGLGETINTEGERV GetIntegerv; void (*makeCurrent)(_GLFWwindow*); void (*swapBuffers)(_GLFWwindow*); void (*swapInterval)(int); GLFWglproc (*getProcAddress)(const char*); _GLFWcontextGLX glx; };
    struct _GLFWwindow { i32 decorated,doublebuffer; GLFWvidmode videoMode; int minwidth,minheight,maxwidth,maxheight,cursorMode; char mouseButtons[8],keys[349]; double virtualCursorPosX,virtualCursorPosY; _GLFWcontext context; _GLFWwindowX11 x11; };
    struct _GLFWmonitor { char name[128]; int widthMM,heightMM; GLFWvidmode currentMode; _GLFWmonitorX11 x11; };
    void* _glfwPlatformLoadModule(const char* path) { return dlopen(path,2); }
    GLFWproc PlatformGetModuleSymbol(void* module, const char* name) { return dlsym(module,name); }
    unsigned long _glfwGetWindowPropertyX11(Window window,Atom property,Atom type,unsigned char** value) {
        Atom actualType; int actualFormat; unsigned long itemCount,bytesAfter;
        _glfw.x11.xlib.GetWindowProperty(_glfw.x11.display,window,property,0,2147483647,0,type,&actualType,&actualFormat,&itemCount,&bytesAfter,value);
        return itemCount;
    }

    static int translateKey(int scancode) { return (scancode<0||scancode>255) ? GLFW_KEY_UNKNOWN : _glfw.x11.keycodes[scancode]; }
    static void sendEventToWM(_GLFWwindow* window,Atom type,long a,long b,long c,long d,long e) {
        XEvent event={33/*ClientMessage*/};
        event.xclient.window=window->x11.handle; event.xclient.format=32; event.xclient.message_type=type;
        event.xclient.data.l[0]=a; event.xclient.data.l[1]=b; event.xclient.data.l[2]=c; event.xclient.data.l[3]=d; event.xclient.data.l[4]=e;
        _glfw.x11.xlib.SendEvent(_glfw.x11.display,_glfw.x11.root,0,(1L<<19)|(1L<<20),&event);
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
    static void captureCursor(_GLFWwindow* window) { _glfw.x11.xlib.GrabPointer(_glfw.x11.display,window->x11.handle,1,(1L<<2)|(1L<<3)|(1L<<6),1/*GrabModeAsync*/,1/*GrabModeAsync*/,window->x11.handle,0L,0L); }
    static void releaseCursor(void) { _glfw.x11.xlib.UngrabPointer(_glfw.x11.display,0L); }
    static void disableCursor(_GLFWwindow* window) { _glfw.x11.disabledCursorWindow=window; GetCursorPosV(window,&_glfw.x11.restoreCurPosX,&_glfw.x11.restoreCurPosY); updateCursorImage(window); captureCursor(window); }
    static void enableCursor(_GLFWwindow* window) { _glfw.x11.disabledCursorWindow = NULL; releaseCursor(); SetCursorPosV(window,_glfw.x11.restoreCurPosX,_glfw.x11.restoreCurPosY); updateCursorImage(window); }
    void GetMonitorPos(_GLFWmonitor* monitor, int* xpos, int* ypos) {
        XRRScreenResources* sr = _glfw.x11.randr.GetScreenResourcesCurrent(_glfw.x11.display, _glfw.x11.root);
        XRRCrtcInfo* ci = _glfw.x11.randr.GetCrtcInfo(_glfw.x11.display, sr, monitor->x11.crtc);
        if (ci) { *xpos = ci->x; *ypos = ci->y; _glfw.x11.randr.FreeCrtcInfo(ci); }
        _glfw.x11.randr.FreeScreenResources(sr);
    }

    void SetWindowIcon(_GLFWwindow* window, const GLFWimage* images) {
        int longCount=0;
        longCount+=2+images[0].width*images[0].height;
        unsigned long* icon=OS_Calloc(longCount,sizeof(unsigned long)), *target=icon;
        *target++=images[0].width; *target++=images[0].height;
        for (int j=0;j<images[0].width*images[0].height;++j) *target++=(((unsigned long)images[0].pixels[j*4+0])<<16)|(((unsigned long)images[0].pixels[j*4+1])<<8)|(((unsigned long)images[0].pixels[j*4+2])<<0)|(((unsigned long)images[0].pixels[j*4+3])<<24);
        _glfw.x11.xlib.ChangeProperty(_glfw.x11.display,window->x11.handle,_glfw.x11.NET_WM_ICON,((Atom) 6),32,0/*PropModeReplace*/,(unsigned char*)icon,longCount);
        OS_DeallocateRAM(icon,longCount*sizeof(unsigned long));
    }

    void GetWindowSize(_GLFWwindow* window, int* width, int* height) { XWindowAttributes attribs; _glfw.x11.xlib.GetWindowAttributes(_glfw.x11.display,window->x11.handle,&attribs); *width=attribs.width; *height=attribs.height; }
    void SetWindowSize(_GLFWwindow* window, int width, int height) { width=vmax(1,width); height=vmax(1,height); updateNormalHints(window,width,height); _glfw.x11.xlib.ResizeWindow(_glfw.x11.display,window->x11.handle,width,height); }
    void SetWindowMonitor(_GLFWwindow* window,int xpos,int ypos,int width,int height) {
        updateNormalHints(window,width,height);
        if (_glfw.x11.NET_WM_STATE && _glfw.x11.NET_WM_STATE_FULLSCREEN) sendEventToWM(window,_glfw.x11.NET_WM_STATE,0/*remove*/,_glfw.x11.NET_WM_STATE_FULLSCREEN,0,1,0);
        else {
            XSetWindowAttributes attributes; attributes.override_redirect=0;
            _glfw.x11.xlib.ChangeWindowAttributes(_glfw.x11.display,window->x11.handle,(1L<<9)/*override redirect*/,&attributes);
            window->x11.overrideRedirect=0;
        }
        
        _glfw.x11.xlib.DeleteProperty(_glfw.x11.display,window->x11.handle,_glfw.x11.NET_WM_BYPASS_COMPOSITOR);
        _glfw.x11.xlib.MoveResizeWindow(_glfw.x11.display,window->x11.handle,xpos,ypos,width,height);
    }
    
    i32 WindowFocused(_GLFWwindow* window) { Window focused; int state; _glfw.x11.xlib.GetInputFocus(_glfw.x11.display,&focused,&state); return window->x11.handle==focused; }
    i32 WindowVisible(_GLFWwindow* window) { XWindowAttributes wa; _glfw.x11.xlib.GetWindowAttributes(_glfw.x11.display,window->x11.handle,&wa); return wa.map_state==2/*IsViewable*/; }
    void GetWindowPos(_GLFWwindow* window, int* xpos, int* ypos) { Window dummy; _glfw.x11.xlib.TranslateCoordinates(_glfw.x11.display,window->x11.handle,_glfw.x11.root,0,0,xpos,ypos,&dummy); }
    void SetWindowPos(_GLFWwindow* window, int xpos, int ypos) {
        if (!WindowVisible(window)) {
            long supplied; XSizeHints* hints=_glfw.x11.xlib.AllocSizeHints();
            if (_glfw.x11.xlib.GetWMNormalHints(_glfw.x11.display,window->x11.handle,hints,&supplied)) { hints->flags|=(1L << 2)/*PPosition*/; hints->x=hints->y=0; _glfw.x11.xlib.SetWMNormalHints(_glfw.x11.display,window->x11.handle,hints); }
            _glfw.x11.xlib.Free(hints);
        }
        _glfw.x11.xlib.MoveWindow(_glfw.x11.display,window->x11.handle,xpos,ypos);
    }

    void SetWindowDecorated(_GLFWwindow* window,i32 enabled) {
        struct { unsigned long flags,functions,decorations; long input_mode; unsigned long status; } hints={0};
        hints.flags=2; hints.decorations=enabled?1:0;
        _glfw.x11.xlib.ChangeProperty(_glfw.x11.display,window->x11.handle,_glfw.x11.MOTIF_WM_HINTS,_glfw.x11.MOTIF_WM_HINTS,32,0/*PropModeReplace*/,(unsigned char*)&hints,sizeof(hints)/sizeof(long));
    }

    void GetCursorPosV(_GLFWwindow* window, double* xpos, double* ypos) { Window root,child; int rootX,rootY,childX,childY; unsigned int mask; _glfw.x11.xlib.QueryPointer(_glfw.x11.display,window->x11.handle,&root,&child,&rootX,&rootY,&childX,&childY,&mask); *xpos=childX; *ypos=childY; }
    void SetCursorPosV(_GLFWwindow* window, double x, double y) { window->x11.warpCursorPosX=(int)x; window->x11.warpCursorPosY=(int)y; _glfw.x11.xlib.WarpPointer(_glfw.x11.display,0L,window->x11.handle,0,0,0,0,(int)x,(int)y); }
    void SetCursorMode(GLFWwindow* handle, int value) {
        _GLFWwindow* window = (_GLFWwindow*)handle;
        if (window->cursorMode != value) {
            window->cursorMode = value;
            GetCursorPosV(window,&window->virtualCursorPosX,&window->virtualCursorPosY);
            if (WindowFocused(window)) {
                GetCursorPosV(window,&_glfw.x11.restoreCurPosX,&_glfw.x11.restoreCurPosY);
                captureCursor(window);
                _glfw.x11.disabledCursorWindow=window;
            } else Sys_Global.gamePaused = true;
            
            updateCursorImage(window);
        }
    }
    
    static const XRRModeInfo* getModeInfo(const XRRScreenResources* sr, RRMode id) { for (int i = 0;  i < sr->nmode;  i++){ if (sr->modes[i].id == id) {return sr->modes + i;} } return NULL; }
    static GLFWvidmode vidmodeFromModeInfo(const XRRModeInfo* mi, const XRRCrtcInfo* ci) {
        GLFWvidmode mode;
        if (ci->rotation == 2 || ci->rotation == 8) {  mode.width  = mi->height; mode.height = mi->width; } // ==90, ==270
        else { mode.width = mi->width; mode.height = mi->height; }
        mode.refreshRate = (mi->hTotal && mi->vTotal) ? (int)vround((double) mi->dotClock / ((double) mi->hTotal * (double) mi->vTotal)) : 0;
        return mode;
    }

    void PollMonitors(void) {
        int disconnectedCount; _GLFWmonitor** disconnected = NULL;
        XRRScreenResources* sr = _glfw.x11.randr.GetScreenResourcesCurrent(_glfw.x11.display,_glfw.x11.root);
        RROutput primary = _glfw.x11.randr.GetOutputPrimary(_glfw.x11.display,_glfw.x11.root);
        disconnectedCount = _glfw.monitorCount;
        if (disconnectedCount) { disconnected = OS_Calloc(_glfw.monitorCount,sizeof(_GLFWmonitor*)); CopyMemoryFromBtoAForNBytes(disconnected,_glfw.monitors,_glfw.monitorCount * sizeof(_GLFWmonitor*)); }
        for (int i = 0;  i < sr->noutput;  i++) {
            int j, type, widthMM, heightMM;
            XRROutputInfo* oi = _glfw.x11.randr.GetOutputInfo(_glfw.x11.display, sr, sr->outputs[i]);
            if (oi->connection != 0/*connected*/ || oi->crtc == 0L) { _glfw.x11.randr.FreeOutputInfo(oi); continue; }

            for (j = 0;  j < disconnectedCount;  j++) {
                if (disconnected[j] && disconnected[j]->x11.output == sr->outputs[i]) { disconnected[j] = NULL; break; }
            }

            if (j < disconnectedCount) { _glfw.x11.randr.FreeOutputInfo(oi); continue; }

            XRRCrtcInfo* ci = _glfw.x11.randr.GetCrtcInfo(_glfw.x11.display, sr, oi->crtc);
            if (!ci) { _glfw.x11.randr.FreeOutputInfo(oi); continue; }

            if (ci->rotation == 2 || ci->rotation == 8) { widthMM  = oi->mm_height; heightMM = oi->mm_width; } // == 90, == 270
            else { widthMM  = oi->mm_width; heightMM = oi->mm_height; }
            
            if (widthMM <= 0 || heightMM <= 0) { widthMM  = (int) (ci->width * 25.4f / 96.f); heightMM = (int) (ci->height * 25.4f / 96.f); }
            _GLFWmonitor* monitor = AllocMonitor(oi->name, widthMM, heightMM);
            monitor->x11.output = sr->outputs[i]; monitor->x11.crtc   = oi->crtc;
            type = (monitor->x11.output == primary) ? 0 : 1; InputMonitor(monitor,0x00040001/*connected*/,type); _glfw.x11.randr.FreeOutputInfo(oi); _glfw.x11.randr.FreeCrtcInfo(ci);
        }

        _glfw.x11.randr.FreeScreenResources(sr);
        for (int i = 0;  i < disconnectedCount;  i++) { if (disconnected[i]) {InputMonitor(disconnected[i],0x00040002/*disconnected*/,0);} }
        if (disconnected) OS_DeallocateRAM(disconnected,_glfw.monitorCount*sizeof(_GLFWmonitor*));
    }
    
    static void processEvent(XEvent* event) {
        unsigned int keycode=0; Bool filtered=0;
        if (event->type==2/*KeyPress*/ || event->type==3/*KeyRelease*/) keycode=event->xkey.keycode;
        filtered=_glfw.x11.xlib.FilterEvent(event,0L);
        if (event->type==_glfw.x11.randr.eventBase+1/*notify*/) { _glfw.x11.randr.UpdateConfiguration(event); PollMonitors(); return; }
        if (event->type==35/*GenericEvent*/) return;
        _GLFWwindow* window=NULL; if (_glfw.x11.xlib.FindContext(_glfw.x11.display,event->xany.window,_glfw.x11.context,(XPointer*)&window)!=0) return;

        switch (event->type) {
            case 21/*ReparentNotify*/: window->x11.parent=event->xreparent.parent; return;
            case 2/*KeyPress*/:
            case 3/*KeyRelease*/: {
                const int key=translateKey(keycode),action=(event->type==2/*KeyPress*/)?GLFW_PRESS:GLFW_RELEASE;
                if (key!=GLFW_KEY_UNKNOWN) InputKey(window,key,action);
                return;
            }
            case 4/*ButtonPress*/: {
                if      (event->xbutton.button==1) InputMouseClick(window,GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS);
                else if (event->xbutton.button==2) InputMouseClick(window,GLFW_MOUSE_BUTTON_MIDDLE,GLFW_PRESS);
                else if (event->xbutton.button==3) InputMouseClick(window,GLFW_MOUSE_BUTTON_RIGHT,GLFW_PRESS);
                else if (event->xbutton.button==4) Sys_Input.scrollDelta += 1.0;
                else if (event->xbutton.button==5) Sys_Input.scrollDelta += -1.0;
                else InputMouseClick(window,event->xbutton.button - 1 - 4,GLFW_PRESS);
                return;
            }
            case 5/*ButtonRelease*/: {
                if      (event->xbutton.button==1) InputMouseClick(window,GLFW_MOUSE_BUTTON_LEFT,GLFW_RELEASE);
                else if (event->xbutton.button==2) InputMouseClick(window,GLFW_MOUSE_BUTTON_MIDDLE,GLFW_RELEASE);
                else if (event->xbutton.button==3) InputMouseClick(window,GLFW_MOUSE_BUTTON_RIGHT,GLFW_RELEASE);
                else if (event->xbutton.button>7)  InputMouseClick(window,event->xbutton.button - 1 - 4,GLFW_RELEASE);
                return;
            }
            case 7/*EnterNotify*/: {
                const int x=event->xcrossing.x,y=event->xcrossing.y;
                InputCursorPos(window,x,y);
                window->x11.lastCursorPosX=x; window->x11.lastCursorPosY=y;
                return;
            }
            case 6/*MotionNotify*/: {
                const int x=event->xmotion.x,y=event->xmotion.y;
                if (x!=window->x11.warpCursorPosX || y!=window->x11.warpCursorPosY) {
                    if (window->cursorMode==0x00034003/*CURSOR_DISABLED*/) {
                        if (_glfw.x11.disabledCursorWindow!=window) return;
                        InputCursorPos(window,window->virtualCursorPosX+(x-window->x11.lastCursorPosX),window->virtualCursorPosY+(y-window->x11.lastCursorPosY));
                    } else InputCursorPos(window,x,y);
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
                if (filtered || event->xclient.message_type==0L) return;
                
                if (event->xclient.message_type==_glfw.x11.WM_PROTOCOLS) {
                    const Atom protocol=event->xclient.data.l[0];
                    if (protocol==0L) return;
                    
                    if (protocol == _glfw.x11.WM_DELETE_WINDOW) OS_Exit(0);
                    if (protocol == _glfw.x11.NET_WM_PING) {
                        XEvent reply=*event; reply.xclient.window=_glfw.x11.root;
                        _glfw.x11.xlib.SendEvent(_glfw.x11.display,_glfw.x11.root,0,(1L<<19)|(1L<<20),&reply);
                    }
                }
                return;
            }
            case 9/*FocusIn*/: {
                if (event->xfocus.mode==1/*NotifyGrab*/ || event->xfocus.mode==2/*NotifyUngrab*/) return;
                if (window->cursorMode==0x00034003/*CURSOR_DISABLED*/) disableCursor(window);
                if (window->x11.ic) _glfw.x11.xlib.SetICFocus(window->x11.ic);
                InputWindowFocus(window,1);
                return;
            }
            case 10/*FocusOut*/: {
                if (event->xfocus.mode==1/*NotifyGrab*/ || event->xfocus.mode==2/*NotifyUngrab*/) return;
                if (window->cursorMode==0x00034003/*CURSOR_DISABLED*/) enableCursor(window);
                if (window->x11.ic) _glfw.x11.xlib.UnsetICFocus(window->x11.ic);
                InputWindowFocus(window,0);
                return;
            }
        }
    }

    void GetMonitorWorkarea(_GLFWmonitor* monitor,int* xpos,int* ypos,int* width,int* height) {
        int areaX = 0, areaY = 0, areaWidth = 0, areaHeight = 0;
        XRRScreenResources* sr = _glfw.x11.randr.GetScreenResourcesCurrent(_glfw.x11.display,_glfw.x11.root);
        XRRCrtcInfo* ci = _glfw.x11.randr.GetCrtcInfo(_glfw.x11.display,sr,monitor->x11.crtc);
        const XRRModeInfo* mi = getModeInfo(sr,ci->mode);
        areaX = ci->x, areaY = ci->y;
        if (ci->rotation == 2 || ci->rotation == 8) { areaWidth = mi->height, areaHeight = mi->width; } // ==90, ==270
        else { areaWidth = mi->width, areaHeight = mi->height; }
        _glfw.x11.randr.FreeCrtcInfo(ci); _glfw.x11.randr.FreeScreenResources(sr);
        if (_glfw.x11.NET_WORKAREA && _glfw.x11.NET_CURRENT_DESKTOP) {
            Atom *extents = NULL, *desktop = NULL;
            const unsigned long extentCount = _glfwGetWindowPropertyX11(_glfw.x11.root,_glfw.x11.NET_WORKAREA,((Atom) 6),(unsigned char**) &extents);
            if (_glfwGetWindowPropertyX11(_glfw.x11.root,_glfw.x11.NET_CURRENT_DESKTOP,((Atom) 6),(unsigned char**) &desktop) > 0) {
                if (extentCount >= 4 && *desktop < extentCount / 4) {
                    const int gx = extents[*desktop * 4 + 0], gy = extents[*desktop * 4 + 1], gw = extents[*desktop * 4 + 2], gh = extents[*desktop * 4 + 3];
                    if (areaX < gx) { areaWidth  -= gx - areaX, areaX = gx; }
                    if (areaY < gy) { areaHeight -= gy - areaY, areaY = gy; }
                    if (areaX +  areaWidth > gx + gw)  areaWidth = gx - areaX + gw;
                    if (areaY + areaHeight > gy + gh) areaHeight = gy - areaY + gh;
                }
            }
            if (extents) {_glfw.x11.xlib.Free(extents);} if (desktop) {_glfw.x11.xlib.Free(desktop);}
        }
        *xpos = areaX; *ypos = areaY; *width = areaWidth; *height = areaHeight;
    }

    void GetVideoMode(_GLFWmonitor* monitor,GLFWvidmode* mode) {
        XRRScreenResources* sr = _glfw.x11.randr.GetScreenResourcesCurrent(_glfw.x11.display,_glfw.x11.root);
        const XRRModeInfo* mi = NULL;
        XRRCrtcInfo* ci = _glfw.x11.randr.GetCrtcInfo(_glfw.x11.display,sr,monitor->x11.crtc);
        if (ci) { mi = getModeInfo(sr,ci->mode); if (mi) {*mode = vidmodeFromModeInfo(mi,ci);} _glfw.x11.randr.FreeCrtcInfo(ci); }
        _glfw.x11.randr.FreeScreenResources(sr);
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
            case 0xff1b: return GLFW_KEY_ESCAPE; case 0xff09: return GLFW_KEY_TAB; case 0xff0d: return GLFW_KEY_ENTER;
            case 0xff08: return GLFW_KEY_BACKSPACE; case 0xffff: return GLFW_KEY_DELETE; case 0xff50: return GLFW_KEY_HOME;
            case 0xff57: return GLFW_KEY_END; case 0xff55: return GLFW_KEY_PAGE_UP; case 0xff56: return GLFW_KEY_PAGE_DOWN;
            case 0xff63: return GLFW_KEY_INSERT; case 0xff51: return GLFW_KEY_LEFT; case 0xff53: return GLFW_KEY_RIGHT;
            case 0xff54: return GLFW_KEY_DOWN; case 0xff52: return GLFW_KEY_UP; case 0xff13: return GLFW_KEY_PAUSE;
            case 0xff14: return GLFW_KEY_SCROLL_LOCK; case 0xff61: return GLFW_KEY_PRINT_SCREEN; case 0xff7f: return GLFW_KEY_NUM_LOCK;
            case 0xffe5: return GLFW_KEY_CAPS_LOCK; case 0xff67: return GLFW_KEY_MENU; case 0xffe1: return GLFW_KEY_LEFT_SHIFT;
            case 0xffe2: return GLFW_KEY_RIGHT_SHIFT; case 0xffe3: return GLFW_KEY_LEFT_CONTROL; case 0xffe4: return GLFW_KEY_RIGHT_CONTROL;
            case 0xffe7: case 0xffe9: return GLFW_KEY_LEFT_ALT;   // Meta_L, Alt_L
            case 0xff7e: case 0xfe03: case 0xffe8: case 0xffea: return GLFW_KEY_RIGHT_ALT; // Mode_switch, ISO_Level3_Shift, Meta_R, Alt_R
            case 0xffeb: return GLFW_KEY_LEFT_SUPER; case 0xffec: return GLFW_KEY_RIGHT_SUPER; case 0xffaa: return GLFW_KEY_KP_MULTIPLY;
            case 0xffab: return GLFW_KEY_KP_ADD; case 0xffad: return GLFW_KEY_KP_SUBTRACT; case 0xffaf: return GLFW_KEY_KP_DIVIDE;
            case 0xffbd: return GLFW_KEY_KP_EQUAL; case 0xff8d: return GLFW_KEY_KP_ENTER; case 0x0020: return GLFW_KEY_SPACE;
            case 0x0027: return GLFW_KEY_APOSTROPHE; case 0x002c: return GLFW_KEY_COMMA; case 0x002d: return GLFW_KEY_MINUS;
            case 0x002e: return GLFW_KEY_PERIOD; case 0x002f: return GLFW_KEY_SLASH; case 0x003b: return GLFW_KEY_SEMICOLON;
            case 0x003d: return GLFW_KEY_EQUAL; case 0x005b: return GLFW_KEY_LEFT_BRACKET; case 0x005c: return GLFW_KEY_BACKSLASH;
            case 0x005d: return GLFW_KEY_RIGHT_BRACKET; case 0x0060: return GLFW_KEY_GRAVE_ACCENT; default: return GLFW_KEY_UNKNOWN;
        }
    }

    static void createKeyTables(void) {
        int scancodeMin, scancodeMax;
        MemSetToVForNBytes(_glfw.x11.keycodes,-1,sizeof(_glfw.x11.keycodes));
        MemSetToVForNBytes(_glfw.x11.scancodes,-1,sizeof(_glfw.x11.scancodes));
        _glfw.x11.xlib.DisplayKeycodes(_glfw.x11.display,&scancodeMin,&scancodeMax);
        int width; KeySym* keysyms = _glfw.x11.xlib.GetKeyboardMapping(_glfw.x11.display,scancodeMin,scancodeMax - scancodeMin + 1,&width);
        for (int sc = scancodeMin; sc <= scancodeMax; sc++) {
            if (_glfw.x11.keycodes[sc] < 0) _glfw.x11.keycodes[sc] = translateKeySyms(&keysyms[(sc - scancodeMin) * width],width);
            if (_glfw.x11.keycodes[sc] > 0) _glfw.x11.scancodes[_glfw.x11.keycodes[sc]] = sc;
        }
        _glfw.x11.xlib.Free(keysyms);
    }

    static Atom getAtomIfSupported(Atom* atoms, unsigned long count, const char* name) { const Atom atom=_glfw.x11.xlib.InternAtom(_glfw.x11.display,name,0); for (unsigned long i=0;i<count;i++) {if (atoms[i] == atom) {return atom;}} return 0L; }
    static void handleKeyEvent(_GLFWjoystick* js, int code, int value) { InputJoystickButton(js,js->linjs.keyMap[code - 0x100/*BTN_MISC*/],value ? GLFW_PRESS : GLFW_RELEASE); }
    static void handleAbsEvent(_GLFWjoystick* js, int code, int value) {
        const int index = js->linjs.absMap[code];
        if (code >= 0x10/*ABS_HAT0X*/ && code <= 0x17/*ABS_HAT3Y*/) {
            static const char stateMap[3][3] = {{GLFW_HAT_CENTERED,GLFW_HAT_UP,GLFW_HAT_DOWN},{GLFW_HAT_LEFT,GLFW_HAT_LEFT_UP,GLFW_HAT_LEFT_DOWN},{GLFW_HAT_RIGHT,GLFW_HAT_RIGHT_UP,GLFW_HAT_RIGHT_DOWN},};
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

    static void pollAbsState(_GLFWjoystick* js) {
        for (int code=0;code<0x40/*ABS_CNT*/;code++) {
            if (js->linjs.absMap[code] < 0) continue;

            struct input_absinfo* info = &js->linjs.absInfo[code];
            if (OS_IOControl(js->linjs.fd,(0x80184540 + (code)),info) < 0) continue;

            handleAbsEvent(js, code, info->value);
        }
    }

    #define isBitSet(bit, arr) (arr[(bit) / 8] & (1 << ((bit) % 8)))
    #define EVIOCGBIT(ev, len) (0x80004520 + (ev) + ((len) << 16))
    static i32 openJoystickDevice(const char* path) {
        for (int jid = 0;  jid <= GLFW_JOYSTICK_LAST;  jid++) {
            if (!_glfw.joysticks[jid].connected) continue;
            if (StringsEqual(_glfw.joysticks[jid].linjs.path,path)) return 0;
        }

        _GLFWjoystickLinux linjs = {0}; linjs.fd = OS_Open(path,00004000|02000000,0); if (linjs.fd == -1) return 0;

        char evBits[(0x20/*EV_CNT*/ + 7) / 8] = {0},keyBits[(0x300/*KEY_CNT*/ + 7) / 8] = {0},absBits[(0x40/*ABS_CNT*/ + 7) / 8] = {0};
        struct input_id id; if (OS_IOControl(linjs.fd,EVIOCGBIT(0,sizeof(evBits)),evBits) < 0 || OS_IOControl(linjs.fd,EVIOCGBIT(0x01/*EV_KEY*/,sizeof(keyBits)),keyBits) < 0 || OS_IOControl(linjs.fd,EVIOCGBIT(0x03/*EV_ABS*/,sizeof(absBits)),absBits) < 0 || OS_IOControl(linjs.fd,0x80084501/*EVIOCGID*/,&id) < 0) { OS_Close(linjs.fd); return 0; }
        if (!isBitSet(0x03/*EV_ABS*/,evBits)) { OS_Close(linjs.fd); return 0; }

        char name[256] = "",guid[33] = "";
        if (OS_IOControl(linjs.fd,(0x80004506 | (((sizeof(name)) & 0x1fff) << 16)),name) < 0) StringCopyInto_A_From_B(name,"Unknown",sizeof(name));
        if (id.vendor && id.product && id.version) StringFormat(guid,sizeof(guid),"%02x%02x0000%02x%02x0000%02x%02x0000%02x%02x0000",id.bustype & 0xff, id.bustype >> 8,id.vendor & 0xff,  id.vendor >> 8,id.product & 0xff, id.product >> 8,id.version & 0xff, id.version >> 8);
        else StringFormat(guid,sizeof(guid),"%02x%02x0000%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x00",id.bustype & 0xff, id.bustype >> 8,name[0], name[1], name[2], name[3],name[4], name[5], name[6], name[7],name[8], name[9], name[10]);

        int axisCount = 0, buttonCount = 0, hatCount = 0;
        for (int code=0x100/*BTN_MISC*/;code<0x300/*KEY_CNT*/;code++) {
            if (!isBitSet(code,keyBits)) continue;

            linjs.keyMap[code - 0x100/*BTN_MISC*/] = buttonCount++;
        }

        for (int code=0;code<0x40/*ABS_CNT*/;code++) {
            linjs.absMap[code] = -1; if (!isBitSet(code,absBits)) continue;

            if (code >= 0x10/*ABS_HAT0X*/ && code <= 0x17/*ABS_HAT3Y*/) { linjs.absMap[code] = hatCount; hatCount++; code++; } // Skip the Y axis
            else {
                if (OS_IOControl(linjs.fd,(0x80184540 + (code)), &linjs.absInfo[code]) < 0) continue;

                linjs.absMap[code] = axisCount++;
            }
        }

        _GLFWjoystick* js = _glfwAllocJoystick(name,guid,axisCount,buttonCount,hatCount);
        if (!js) { OS_Close(linjs.fd); return 0; }

        StringCopyInto_A_From_B(linjs.path,path,sizeof(linjs.path));
        CopyMemoryFromBtoAForNBytes(&js->linjs,&linjs,sizeof(linjs));
        pollAbsState(js); JoystickConnection(js,0x00040001/*connected*/);
        return  1;
    }
    
    struct linux_dirent64 { u64 d_ino; i64 d_off; unsigned short d_reclen; unsigned char d_type; char d_name[]; };
    struct inotify_event { i32 wd; u32 mask,cookie,len; char name[]; };
    static void closeJoystick(_GLFWjoystick* js) { JoystickConnection(js,0x00040002/*disconnected*/); if (js->linjs.fd > 0) { OS_Close(js->linjs.fd); js->linjs.fd = -1; } _glfwFreeJoystick(js); }    
    static i32 isEventDevice(const char* name) { if (!name || !StringCompareUpToLength(name, "event", 5) || name[5] == '\0') {return 0;} for (const char* p=name+5;*p;++p) if (*p < '0' || *p > '9') {return 0;} return 1; }
    static void iterateInputDevices(void (*callback)(const char* fullpath)) {
        const char* dirname = "/dev/input"; FHandle fd = OS_Open(dirname,00200000|02000000,0); if (fd < 0) return;

        char buf[8192];
        for (;;) {
            register long rax __asm__("rax") = 217/*__NR_getdents64*/, rdi __asm__("rdi") = fd; register char* rsi __asm__("rsi") = buf; register size_t rdx __asm__("rdx") = sizeof(buf);
            __asm__ __volatile__("syscall":"+r"(rax):"r"(rdi),"r"(rsi),"r"(rdx):"rcx","r11","memory"); if (rax <= 0) break;

            long offset = 0;
            while (offset < rax) {
                struct linux_dirent64* d = (struct linux_dirent64*)(buf + offset);
                if (d->d_name[0] != '.' && isEventDevice(d->d_name)) { char path[260]; StringFormat(path,sizeof(path),"%s/%s",dirname,d->d_name); callback(path); }
                offset += d->d_reclen;
            }
        }

        OS_Close(fd);
    }
    
    static void openJoystickCallback(const char* path) { openJoystickDevice(path); }
    static char joyConbuffer[16384],joyPath[260];
    void _glfwDetectJoystickConnectionLinux(void) {
        if (_glfw.linjs.inotify <= 0) return;
        i32 size = OS_Read(_glfw.linjs.inotify,joyConbuffer,sizeof(joyConbuffer)); if (size <= 0) return;
        
        i32 offset = 0;
        while (size >= offset + (i32)sizeof(struct inotify_event)) {
            const struct inotify_event* e = (struct inotify_event*)(joyConbuffer + offset);
            offset += (i32)sizeof(struct inotify_event) + e->len;
            if (e->len == 0 || !isEventDevice(e->name)) continue;

            StringFormat(joyPath,sizeof(joyPath), "/dev/input/%s", e->name);
            if (e->mask & (0x00000100/*IN_CREATE*/|0x00000004/*IN_ATTRIB*/)) openJoystickDevice(joyPath);
            else if (e->mask & 0x00000200/*IN_DELETE*/) {
                for (int jid = 0; jid <= GLFW_JOYSTICK_LAST; jid++) {
                    if (StringsEqual(_glfw.joysticks[jid].linjs.path,joyPath)) { closeJoystick(_glfw.joysticks + jid); break; }
                }
            }
        }
    }

    i32 InitJoysticks(void) {
        const char* dirname = "/dev/input";
        {register long rax __asm__("rax") = 294/*__NR_inotify_init1*/; register unsigned int rdi __asm__("rdi") = 0x800/*IN_NONBLOCK*/|0x80000/*IN_CLOEXEC*/; 
        __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi) : "rcx", "r11", "memory");
        _glfw.linjs.inotify = (int)rax; }
        if (_glfw.linjs.inotify >= 0) {
            register long rax __asm__("rax") = 295/*__NR_inotify_add_watch*/; register int rdi __asm__("rdi") = _glfw.linjs.inotify; register const char* rsi __asm__("rsi") = dirname;
            register u32 rdx __asm__("rdx") = 0x00000100/*IN_CREATE*/|0x00000004/*IN_ATTRIB*/|0x00000200/*IN_DELETE*/;
            __asm__ __volatile__("syscall":"+r"(rax):"r"(rdi),"r"(rsi),"r"(rdx):"rcx","r11","memory");
            _glfw.linjs.watch = (int)rax;
        }
        
        iterateInputDevices(openJoystickCallback);
        return 1;
    }

    i32 PollJoystick(_GLFWjoystick* js) {
        if (js->linjs.fd <= 0) return 0;

        for (;;) {
            struct input_event e;
            long n = OS_Read(js->linjs.fd, &e, sizeof(e));

            if (n < 0) { closeJoystick(js); break; }
            if (n == 0) { break; }
            if (n < (long)sizeof(e)) { closeJoystick(js); break; }

            if (e.type == 0x00/*EV_SYN*/) {
                if (e.code == 3/*SYN_DROPPED*/) _glfw.linjs.dropped = 1;
                else if (e.code == 0/*SYN_REPORT*/) { _glfw.linjs.dropped = 0; pollAbsState(js); }
            }

            if (_glfw.linjs.dropped) continue;

                 if (e.type == 0x01/*EV_KEY*/) handleKeyEvent(js,e.code,e.value);
            else if (e.type == 0x03/*EV_ABS*/) handleAbsEvent(js,e.code,e.value);
        }

        return js->connected;
    }
    
    void PollEvents(void) {
        if (_glfw.joysticksInitialized) _glfwDetectJoystickConnectionLinux();
        _glfw.x11.xlib.Pending(_glfw.x11.display);
        while (((_XPrivDisplay)(_glfw.x11.display))->qlen) { XEvent e; XNextEvent(_glfw.x11.display,&e); processEvent(&e); }
        _GLFWwindow* window = _glfw.x11.disabledCursorWindow;
        if (window) {
            int width,height; GetWindowSize(window,&width,&height);
            if (window->x11.lastCursorPosX!=width/2 || window->x11.lastCursorPosY!=height/2) SetCursorPosV(window,width/2,height/2);
        }
    }

    static int getGLXFBConfigAttrib(GLXFBConfig fbconfig, int attrib) { int value; _glfw.glx.GetFBConfigAttrib(_glfw.x11.display, fbconfig, attrib, &value); return value; }
    static void makeContextCurrentGLX(_GLFWwindow* window) { _glfw.glx.MakeCurrent(_glfw.x11.display,window->context.glx.window,window->context.glx.handle); /*_glfwPlatformSetTls(&_glfw.contextSlot,window);*/ }
    static void swapBuffersGLX(_GLFWwindow* window) { _glfw.glx.SwapBuffers(_glfw.x11.display, window->context.glx.window); }
    static void swapIntervalGLX(int interval) { _GLFWwindow* handle = (_GLFWwindow*)window; _glfw.glx.SwapIntervalEXT(_glfw.x11.display,handle->context.glx.window,interval); }
    static GLFWglproc getProcAddressGLX(const char* procname) { return _glfw.glx.GetProcAddress((const u8*) procname); }
    void glfwSetWindowPosition(GLFWwindow* handle, int xpos, int ypos) { _GLFWwindow* window = (_GLFWwindow*)handle; SetWindowPos(window,xpos,ypos); }
#endif
_GLFWlibrary _glfw={0};
int WindowInit(void) {
    MemSetToVForNBytes(&_glfw,0,sizeof(_glfw));
    #if defined(WINDOWS)
        GetModuleHandleExW(0x4|0x2,(const u16*)&_glfw,(HMODULE*)&_glfw.win32.instance);
        const char* names[] = {"xinput1_4.dll","xinput1_3.dll","xinput9_1_0.dll","xinput1_2.dll","xinput1_1.dll",NULL};
        for (int i=0;names[i];++i) {
            _glfw.win32.xinput.instance = LoadLibraryA(names[i]);
            if (_glfw.win32.xinput.instance) {
                _glfw.win32.xinput.GetCapabilities = (PFN_XInputGetCapabilities)PlatformGetModuleSymbol(_glfw.win32.xinput.instance, "XInputGetCapabilities");
                _glfw.win32.xinput.GetState = (PFN_XInputGetState)PlatformGetModuleSymbol(_glfw.win32.xinput.instance, "XInputGetState");
                break;
            }
        }

        _glfw.win32.dwmapi.instance = LoadLibraryA("dwmapi.dll");
        if (_glfw.win32.dwmapi.instance) {
            _glfw.win32.dwmapi.IsCompositionEnabled = (PFN_DwmIsCompositionEnabled)PlatformGetModuleSymbol(_glfw.win32.dwmapi.instance, "DwmIsCompositionEnabled");
            _glfw.win32.dwmapi.Flush = (PFN_DwmFlush)PlatformGetModuleSymbol(_glfw.win32.dwmapi.instance, "DwmFlush");
        }

        _glfw.win32.ntdll.instance = LoadLibraryA("ntdll.dll");
        if (_glfw.win32.ntdll.instance) _glfw.win32.ntdll.RtlVerifyVersionInfo = (PFN_RtlVerifyVersionInfo)PlatformGetModuleSymbol(_glfw.win32.ntdll.instance, "RtlVerifyVersionInfo");
        createKeyTables();
        MSG msg; WNDCLASSEXW wc={0}; wc.cbSize=sizeof(wc); // Start making of a helper window
        wc.style = 0x0020/*CS_OWNDC*/; wc.lpfnWndProc = (WNDPROC)helperWindowProc; wc.hInstance = _glfw.win32.instance; wc.lpszClassName = L"GLFW3 Helper";
        _glfw.win32.helperWindowClass = RegisterClassExW(&wc);
        _glfw.win32.helperWindowHandle = CreateWindowExW(0x00000100/*WS_EX_WINDOWEDGE*/ | 0x00000200/*WS_EX_CLIENTEDGE*/,(u16*)MAKEINTATOM(_glfw.win32.helperWindowClass),L"GLFW message window",0x04000000/*WS_CLIPSIBLINGS*/|0x02000000/*WS_CLIPCHILDREN*/,0,0,1,1,NULL,NULL,_glfw.win32.instance,NULL);
        ShowWindow(_glfw.win32.helperWindowHandle,0);
        DEV_BROADCAST_DEVICEINTERFACE_W dbi;
        MemSetToVForNBytes(&dbi,0,sizeof(dbi));
        dbi.dbcc_size = sizeof(dbi);
        dbi.dbcc_devicetype = 0x0005/*DBT_DEVTYP_DEVICEINTERFACE*/;
        dbi.dbcc_classguid = (GUID){0x4d1e55b2,0xf16f,0x11cf,{0x88,0xcb,0x00,0x11,0x11,0x00,0x00,0x30}};
        _glfw.win32.deviceNotificationHandle = RegisterDeviceNotificationW(_glfw.win32.helperWindowHandle,(DEV_BROADCAST_HDR*)&dbi,0);
        while (PeekMessageW(&msg, _glfw.win32.helperWindowHandle,0,0,0x0001)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
        _glfwPollMonitorsWin32();
    #else
        void* module = _glfwPlatformLoadModule("libX11.so.6");
        PFN_XInitThreads XInitThreads = (PFN_XInitThreads)PlatformGetModuleSymbol(module,"XInitThreads");
        PFN_XOpenDisplay XOpenDisplay = (PFN_XOpenDisplay)PlatformGetModuleSymbol(module,"XOpenDisplay");
        XInitThreads();
        Display* display = XOpenDisplay(NULL);
        _glfw.x11.display = display;
        _glfw.x11.xlib.handle = module;
        #define X(n) _glfw.x11.xlib.n = (PFN_X##n)PlatformGetModuleSymbol(_glfw.x11.xlib.handle, "X" #n);
            X(AllocSizeHints)        X(ChangeProperty)       X(ChangeWindowAttributes) X(CheckTypedWindowEvent) X(CreateColormap)       X(CreateWindow)
            X(DefineCursor)          X(DeleteProperty)       X(DisplayKeycodes)        X(FilterEvent)           X(FindContext)          X(Free)
            X(FreeEventData)         X(GetInputFocus)        X(GetKeyboardMapping)     X(GetWMNormalHints)      X(GetWindowAttributes)  X(GetWindowProperty)
            X(GrabPointer)           X(InternAtom)           X(MapWindow)              X(MoveResizeWindow)      X(MoveWindow)           X(Pending)
            X(QueryExtension)        X(QueryPointer)         X(RaiseWindow)            X(ResizeWindow)          X(SaveContext)          X(SendEvent)
            X(SetICFocus)            X(SetInputFocus)        X(SetWMNormalHints)       X(SetWMProtocols)        X(TranslateCoordinates) X(UndefineCursor)
            X(UngrabPointer)         X(UnsetICFocus)         X(WarpPointer)
        #undef X
            
        XNextEvent = (PFN_XNextEvent)PlatformGetModuleSymbol(_glfw.x11.xlib.handle,"XNextEvent");
        _glfw.x11.screen = ((_XPrivDisplay)(_glfw.x11.display))->default_screen;
        _glfw.x11.root = (&((_XPrivDisplay)(_glfw.x11.display))->screens[_glfw.x11.screen])->root;
        static XContext lastContext = 0;
        _glfw.x11.context = ++lastContext;
        _glfw.x11.randr.handle = _glfwPlatformLoadModule("libXrandr.so.2");
        #define X(n) _glfw.x11.randr.n = (PFN_XRR##n)PlatformGetModuleSymbol(_glfw.x11.randr.handle,"XRR"#n);
            X(FreeCrtcInfo) X(FreeOutputInfo) X(FreeScreenResources) X(GetCrtcInfo) X(GetOutputInfo) X(GetOutputPrimary) X(GetScreenResourcesCurrent) X(SelectInput) X(UpdateConfiguration)
        #undef X
            
        XRRScreenResources* sr = _glfw.x11.randr.GetScreenResourcesCurrent(_glfw.x11.display,_glfw.x11.root);
        _glfw.x11.randr.FreeScreenResources(sr);
        _glfw.x11.randr.SelectInput(_glfw.x11.display,_glfw.x11.root,(1L << 2)/*change notify mask*/);
        _glfw.x11.xcursor.handle = _glfwPlatformLoadModule("libXcursor.so.1");
        _glfw.x11.xcursor.ImageCreate     = (PFN_XcursorImageCreate)    PlatformGetModuleSymbol(_glfw.x11.xcursor.handle,"XcursorImageCreate");
        _glfw.x11.xcursor.ImageDestroy    = (PFN_XcursorImageDestroy)   PlatformGetModuleSymbol(_glfw.x11.xcursor.handle,"XcursorImageDestroy");
        _glfw.x11.xcursor.ImageLoadCursor = (PFN_XcursorImageLoadCursor)PlatformGetModuleSymbol(_glfw.x11.xcursor.handle,"XcursorImageLoadCursor");
        createKeyTables();
        #define IA(n) _glfw.x11.xlib.InternAtom(_glfw.x11.display,n,0)
            _glfw.x11.UTF8_STRING     = IA("UTF8_STRING");     _glfw.x11.WM_PROTOCOLS = IA("WM_PROTOCOLS");  _glfw.x11.WM_STATE               = IA("WM_STATE");
            _glfw.x11.WM_DELETE_WINDOW= IA("WM_DELETE_WINDOW");_glfw.x11.NET_SUPPORTED= IA("_NET_SUPPORTED");_glfw.x11.NET_SUPPORTING_WM_CHECK= IA("_NET_SUPPORTING_WM_CHECK");
            _glfw.x11.NET_WM_ICON     = IA("_NET_WM_ICON");    _glfw.x11.NET_WM_PING  = IA("_NET_WM_PING");  _glfw.x11.NET_WM_NAME            = IA("_NET_WM_NAME");
            _glfw.x11.NET_WM_BYPASS_COMPOSITOR = IA("_NET_WM_BYPASS_COMPOSITOR");                            _glfw.x11.MOTIF_WM_HINTS         = IA("_MOTIF_WM_HINTS");
        #undef IA
            
        Window* wfr = NULL; _glfwGetWindowPropertyX11(_glfw.x11.root,_glfw.x11.NET_SUPPORTING_WM_CHECK,((Atom) 33),(unsigned char**)&wfr);
        Window* wfc = NULL; _glfwGetWindowPropertyX11(*wfr,_glfw.x11.NET_SUPPORTING_WM_CHECK,((Atom) 33),(unsigned char**)&wfc);
        _glfw.x11.xlib.Free(wfr); _glfw.x11.xlib.Free(wfc);
        Atom* sa = NULL; const unsigned long ac = _glfwGetWindowPropertyX11(_glfw.x11.root,_glfw.x11.NET_SUPPORTED,((Atom) 4),(unsigned char**)&sa);
        #define GA(name) getAtomIfSupported(sa, ac, name)
            _glfw.x11.NET_WM_STATE              = GA("_NET_WM_STATE");       _glfw.x11.NET_WM_STATE_FULLSCREEN   = GA("_NET_WM_STATE_FULLSCREEN");
            _glfw.x11.NET_WM_WINDOW_TYPE        = GA("_NET_WM_WINDOW_TYPE"); _glfw.x11.NET_WM_WINDOW_TYPE_NORMAL = GA("_NET_WM_WINDOW_TYPE_NORMAL");
            _glfw.x11.NET_WORKAREA              = GA("_NET_WORKAREA");       _glfw.x11.NET_CURRENT_DESKTOP       = GA("_NET_CURRENT_DESKTOP");
            _glfw.x11.NET_ACTIVE_WINDOW         = GA("_NET_ACTIVE_WINDOW");
        #undef GA

        if (sa) _glfw.x11.xlib.Free(sa);
        XSetWindowAttributes wa; wa.event_mask = (1L<<22);
        _glfw.x11.xlib.CreateWindow(_glfw.x11.display,_glfw.x11.root,0,0,1,1,0,0,2/*input only*/,(&((_XPrivDisplay)(_glfw.x11.display))->screens[_glfw.x11.screen])->root_visual,(1L<<11)/*event mask*/,&wa);
        XcursorImage* native = _glfw.x11.xcursor.ImageCreate(16,16); MemSetToVForNBytes(native->pixels,0,256*sizeof(XcursorUInt)); native->xhot=native->yhot=0;
        _glfw.x11.hiddenCursorHandle = _glfw.x11.xcursor.ImageLoadCursor(_glfw.x11.display,native); _glfw.x11.xcursor.ImageDestroy(native);
        PollMonitors();
    #endif
    return  1;
}

const _GLFWfbconfig* _glfwChooseFBConfig(const _GLFWfbconfig* alts, u32 count) {
    u32 missing, leastMissing = 2147483647, colorDiff, leastColorDiff = 2147483647, extraDiff, leastExtraDiff = 2147483647;
    const _GLFWfbconfig* closest = NULL;
    for (u32 i = 0; i < count; i++) {
        const _GLFWfbconfig* cur = alts + i;        missing = 0;
        if (cur->alphaBits == 0) {missing++;} if (cur->depthBits == 0) {missing++;} if (cur->stencilBits == 0) {missing++;}
        colorDiff = 0; colorDiff+=(8-cur->redBits)  *(8-cur->redBits);     colorDiff+=(8-cur->greenBits)*(8-cur->greenBits); colorDiff+=(8-cur->blueBits)   *(8-cur->blueBits);
        extraDiff = 0; extraDiff+=(8-cur->alphaBits)*(8 - cur->alphaBits); extraDiff+=(8-cur->depthBits)*(8-cur->depthBits); extraDiff+=(8-cur->stencilBits)*(8-cur->stencilBits);
        if (missing < leastMissing || (missing == leastMissing && (colorDiff < leastColorDiff || (colorDiff == leastColorDiff && extraDiff < leastExtraDiff)))) closest = cur;
        if (cur == closest) { leastMissing = missing; leastColorDiff = colorDiff; leastExtraDiff = extraDiff; }
    }
    return closest;
}

void glfwSwapBuffers(void) { _GLFWwindow* handle = (_GLFWwindow*)window; handle->context.swapBuffers(handle); }
void SetGLContext_GetFunctionPointers(void) {
    _GLFWwindow* handle=(_GLFWwindow*)window; handle->context.makeCurrent(handle);
    #define X(n,t) n=(t)handle->context.getProcAddress(#n);
    X(glClear,PFNGLCLEAR)                           X(glClearColor,PFNGLCLEARCOLOR)                     X(glColorMask,PFNGLCOLORMASK)
    X(glDepthFunc,PFNGLDEPTHFUNC)                   X(glDepthMask,PFNGLDEPTHMASK)                       X(glDisable,PFNGLDISABLE)
    X(glEnable,PFNGLENABLE)                         X(glFinish,PFNGLFINISH)                             X(glFlush,PFNGLFLUSH)
    X(glFrontFace,PFNGLFRONTFACE)                   X(glGetError,PFNGLGETERROR)                         X(glGetIntegerv,PFNGLGETINTEGERV)
    X(glLineWidth,PFNGLLINEWIDTH)                   X(glReadBuffer,PFNGLREADBUFFER)                     X(glReadPixels,PFNGLREADPIXELS)
    X(glTexImage2D,PFNGLTEXIMAGE2D)                 X(glViewport,PFNGLVIEWPORT)                         X(glBindTexture,PFNGLBINDTEXTURE)
    X(glCopyTexSubImage2D,PFNGLCOPYTEXSUBIMAGE2D)   X(glDrawArrays,PFNGLDRAWARRAYS)                     X(glDrawElements,PFNGLDRAWELEMENTS)
    X(glGenTextures,PFNGLGENTEXTURES)               X(glActiveTexture,PFNGLACTIVETEXTURE)               X(glBlendFuncSeparate,PFNGLBLENDFUNCSEPARATE)
    X(glBindBuffer,PFNGLBINDBUFFER)                 X(glBufferData,PFNGLBUFFERDATA)                     X(glGenBuffers,PFNGLGENBUFFERS)
    X(glUnmapBuffer,PFNGLUNMAPBUFFER)               X(glAttachShader,PFNGLATTACHSHADER)                 X(glCompileShader,PFNGLCOMPILESHADER)
    X(glCreateProgram,PFNGLCREATEPROGRAM)           X(glCreateShader,PFNGLCREATESHADER)                 X(glDrawBuffers,PFNGLDRAWBUFFERS)
    X(glGetProgramiv,PFNGLGETPROGRAMIV)             X(glGetShaderInfoLog,PFNGLGETSHADERINFOLOG)         X(glGetShaderiv,PFNGLGETSHADERIV)
    X(glLinkProgram,PFNGLLINKPROGRAM)               X(glShaderSource,PFNGLSHADERSOURCE)                 X(glUniform1f,PFNGLUNIFORM1F)
    X(glUniform1i,PFNGLUNIFORM1I)                   X(glUniform2f,PFNGLUNIFORM2F)                       X(glUniform3f,PFNGLUNIFORM3F)
    X(glUniform4f,PFNGLUNIFORM4F)                   X(glTexParameteri,PFNGLTEXPARAMETERI)               X(glUniform1ui,PFNGLUNIFORM1UI)
    X(glUniform2ui,PFNGLUNIFORM2UI)                 X(glUniformMatrix3fv,PFNGLUNIFORMMATRIX3FV)         X(glUniformMatrix4fv,PFNGLUNIFORMMATRIX4FV)
    X(glUseProgram,PFNGLUSEPROGRAM)                 X(glBindBufferBase,PFNGLBINDBUFFERBASE)             X(glBindFramebuffer,PFNGLBINDFRAMEBUFFER)
    X(glGenFramebuffers,PFNGLGENFRAMEBUFFERS)       X(glMapBufferRange,PFNGLMAPBUFFERRANGE)             X(glBindImageTexture,PFNGLBINDIMAGETEXTURE)
    X(glBindVertexBuffer,PFNGLBINDVERTEXBUFFER)     X(glDispatchCompute,PFNGLDISPATCHCOMPUTE)           X(glGenVertexArrays,PFNGLGENVERTEXARRAYS)
    X(glVertexAttribFormat,PFNGLVERTEXATTRIBFORMAT) X(glFramebufferTexture2D,PFNGLFRAMEBUFFERTEXTURE2D) X(glBufferSubData,PFNGLBUFFERSUBDATA)
    X(glClearBufferFv,PFNGLCLEARBUFFERFV)           X(glVertexAttribBinding,PFNGLVERTEXATTRIBBINDING)   X(glEnableVertexAttribArray,PFNGLENABLEVERTEXATTRIBARRAY)
    X(glBindVertexArray,PFNGLBINDVERTEXARRAY)       X(glCheckFramebufferStatus,PFNGLCHECKFRAMEBUFFERSTATUS)
    #undef X
}

size_t monitorAllocationSize = 0;
void InputMonitor(_GLFWmonitor* monitor, int action, int placement) {
    if (action == 0x00040001/*connected*/) {
        _glfw.monitorCount++;
        _glfw.monitors = _glfw.monitors ? OS_Realloc(_glfw.monitors,monitorAllocationSize,sizeof(_GLFWmonitor*) * _glfw.monitorCount) : OS_Alloc(_glfw.monitorCount * sizeof(_GLFWmonitor*));
        monitorAllocationSize = _glfw.monitorCount * sizeof(_GLFWmonitor*);
        if (placement == 0) { MoveMemoryFromBtoAForNBytes(_glfw.monitors + 1,_glfw.monitors,((size_t) _glfw.monitorCount - 1) * sizeof(_GLFWmonitor*)); _glfw.monitors[0] = monitor; }
        else _glfw.monitors[_glfw.monitorCount - 1] = monitor;
    } else if (action == 0x00040002/*disconnected*/) {
        for (int i=0;i<_glfw.monitorCount;++i) {
            if (_glfw.monitors[i] == monitor) {
                _glfw.monitorCount--;
                MoveMemoryFromBtoAForNBytes(_glfw.monitors + i, _glfw.monitors + i + 1,((size_t) _glfw.monitorCount - i) * sizeof(_GLFWmonitor*));
                break;
            }
        }
    }
}

_GLFWmonitor* AllocMonitor(const char* n, int w, int h) { _GLFWmonitor* monitor = OS_Calloc(1, sizeof(_GLFWmonitor)); monitor->widthMM = w; monitor->heightMM = h; StringCopyInto_A_From_B(monitor->name,n,sizeof(monitor->name)); return monitor; }
_GLFWmonitor** glfwGetMonitors(int* count) { *count = _glfw.monitorCount; return (_GLFWmonitor**) _glfw.monitors; }
_GLFWmonitor* glfwGetPrimaryMonitor(void) { if (!_glfw.monitorCount) {return NULL;} return (_GLFWmonitor*) _glfw.monitors[0]; }
void glfwGetMonitorPos(_GLFWmonitor* handle, int* xpos, int* ypos) { *xpos = 0; *ypos = 0; _GLFWmonitor* monitor = (_GLFWmonitor*)handle; GetMonitorPos(monitor,xpos,ypos); }
void glfwGetMonitorWorkarea(_GLFWmonitor* handle, int* xpos, int* ypos, int* width, int* height) { *xpos=*ypos=*width=*height=0; _GLFWmonitor* monitor = (_GLFWmonitor*)handle; GetMonitorWorkarea(monitor,xpos,ypos,width,height); }
const GLFWvidmode* glfwGetVideoMode(_GLFWmonitor* handle) { _GLFWmonitor* monitor=(_GLFWmonitor*)handle; GetVideoMode(monitor,&monitor->currentMode); return &monitor->currentMode; }
void InputWindowFocus(_GLFWwindow* window, i32 focused) {
    Sys_Input.window_has_focus = focused != 0; Sys_Input.ignore_next_mouse_delta = true;
    if (!focused) {
        for (int k=0;k<=348;++k) { if (window->keys[k]         == GLFW_PRESS) {       InputKey(window,k,GLFW_RELEASE);} }
        for (int b=0;b<=  7;++b) { if (window->mouseButtons[b] == GLFW_PRESS) {InputMouseClick(window,b,GLFW_RELEASE);} }
    }
}

GLFWwindow* VCreateWindow(int width, int height, char* title) {
    _GLFWwindow* window=OS_Calloc(1,sizeof(_GLFWwindow));
    window->videoMode = (GLFWvidmode){width,height,8,8,8,-1}; window->decorated = window->doublebuffer = 1; window->cursorMode = 0x00034003/*disabled*/;
#ifdef WINDOWS
    u32 style = getWindowStyle(window);
    WNDCLASSEXW wc= (WNDCLASSEXW){sizeof(wc),0x23/*Redraws + Owns Device Context*/,windowProc,0,0,_glfw.win32.instance,NULL,NULL,NULL,NULL,L"Voxen",NULL};
    _glfw.win32.mainWindowClass=RegisterClassExW(&wc);
    RECT rect={0,0,width,height}; //AdjustWindowRectEx(&rect,style,0,0);
    int frameX,frameY; frameX=frameY=0x80000000;
    int frameWidth=rect.right-rect.left, frameHeight=rect.bottom-rect.top;
    u16* wideTitle=CreateWideStringFromUTF8Win32(title);
    window->win32.handle=CreateWindowExW(0,(u16*)MAKEINTATOM(_glfw.win32.mainWindowClass),wideTitle,style,frameX,frameY,frameWidth,frameHeight,NULL,NULL,_glfw.win32.instance,(void*)NULL);
    SetPropW(window->win32.handle,L"GLFW",window);
    window->win32.keymenu=0; WINDOWPLACEMENT wp={0}; wp.length=sizeof(wp); AdjustWindowRectEx(&rect,style,0,0);
    GetWindowPlacement(window->win32.handle,&wp);
    OffsetRect(&rect,wp.rcNormalPosition.left-rect.left,wp.rcNormalPosition.top-rect.top);
    wp.rcNormalPosition=rect; wp.showCmd=0; 
    SetWindowPlacement(window->win32.handle,&wp);
    GetWindowSize(window,&window->win32.width,&window->win32.height);
    PIXELFORMATDESCRIPTOR pfd; HGLRC prc,rc; HDC pdc,dc;
    _glfw.wgl.instance = LoadLibraryA("opengl32.dll");
    _glfw.wgl.CreateContext = (PFN_wglCreateContext)PlatformGetModuleSymbol(_glfw.wgl.instance,"wglCreateContext");
    _glfw.wgl.GetProcAddress = (PFN_wglGetProcAddress)PlatformGetModuleSymbol(_glfw.wgl.instance,"wglGetProcAddress");
    _glfw.wgl.GetCurrentDC = (PFN_wglGetCurrentDC)PlatformGetModuleSymbol(_glfw.wgl.instance,"wglGetCurrentDC");
    _glfw.wgl.GetCurrentContext = (PFN_wglGetCurrentContext)PlatformGetModuleSymbol(_glfw.wgl.instance,"wglGetCurrentContext");
    _glfw.wgl.MakeCurrent = (PFN_wglMakeCurrent)PlatformGetModuleSymbol(_glfw.wgl.instance,"wglMakeCurrent");
    dc = GetDC(_glfw.win32.helperWindowHandle);
    MemSetToVForNBytes(&pfd,0,sizeof(pfd)); pfd.nSize = sizeof(pfd); pfd.dwFlags=0x25; SetPixelFormat(dc,ChoosePixelFormat(dc,&pfd),&pfd);
    rc = _glfw.wgl.CreateContext(dc); pdc=wglGetCurrentDC(); prc=wglGetCurrentContext(); wglMakeCurrent(dc,rc);
    _glfw.wgl.CreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
    _glfw.wgl.SwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
    _glfw.wgl.GetPixelFormatAttribivARB = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)wglGetProcAddress("wglGetPixelFormatAttribivARB");
    wglMakeCurrent(pdc,prc);
    int attribs[40],pixelFormat; PIXELFORMATDESCRIPTOR pfd2;
    window->context.wgl.dc = GetDC(window->win32.handle);
    pixelFormat = choosePixelFormatWGL(window);
    DescribePixelFormat(window->context.wgl.dc,pixelFormat,sizeof(pfd2),&pfd2); SetPixelFormat(window->context.wgl.dc,pixelFormat,&pfd2);
    int index=0; attribs[index++] = 0x2091/*major*/; attribs[index++] = 4;/*OpenGL 4.3*/ attribs[index++] = 0x2092/*minor*/; attribs[index++] = 3; attribs[index++] = 0x9126/*context profile mask*/; attribs[index++] = 1; attribs[index++] = 0; attribs[index++] = 0;
    window->context.wgl.handle = _glfw.wgl.CreateContextAttribsARB(window->context.wgl.dc,NULL,attribs);
    window->context.makeCurrent = makeContextCurrentWGL; window->context.swapBuffers = swapBuffersWGL;
    window->context.swapInterval = swapIntervalWGL; window->context.getProcAddress = getProcAddressWGL;
    int showCommand = 8; ShowWindow(window->win32.handle,showCommand); BringWindowToTop(window->win32.handle); SetForegroundWindow(window->win32.handle); SetFocus(window->win32.handle);
#else
    const char* names[] = {"libGLX.so.0","libGL.so.1","libGL.so",NULL};
    for (int i=0;names[i] && !_glfw.glx.handle;i++) _glfw.glx.handle = _glfwPlatformLoadModule(names[i]);
    _glfw.glx.GetFBConfigs = (PFNGLXGETFBCONFIGSPROC)PlatformGetModuleSymbol(_glfw.glx.handle,"glXGetFBConfigs");
    _glfw.glx.GetFBConfigAttrib = (PFNGLXGETFBCONFIGATTRIBPROC)PlatformGetModuleSymbol(_glfw.glx.handle,"glXGetFBConfigAttrib");
    _glfw.glx.QueryExtension = (PFNGLXQUERYEXTENSIONPROC)PlatformGetModuleSymbol(_glfw.glx.handle,"glXQueryExtension");
    _glfw.glx.QueryVersion = (PFNGLXQUERYVERSIONPROC)PlatformGetModuleSymbol(_glfw.glx.handle,"glXQueryVersion");
    _glfw.glx.MakeCurrent = (PFNGLXMAKECURRENTPROC)PlatformGetModuleSymbol(_glfw.glx.handle,"glXMakeCurrent");
    _glfw.glx.SwapBuffers = (PFNGLXSWAPBUFFERSPROC)PlatformGetModuleSymbol(_glfw.glx.handle,"glXSwapBuffers");
    _glfw.glx.QueryExtensionsString = (PFNGLXQUERYEXTENSIONSSTRINGPROC)PlatformGetModuleSymbol(_glfw.glx.handle,"glXQueryExtensionsString");
    _glfw.glx.CreateNewContext = (PFNGLXCREATENEWCONTEXTPROC)PlatformGetModuleSymbol(_glfw.glx.handle,"glXCreateNewContext");
    _glfw.glx.CreateWindow = (PFNGLXCREATEWINDOWPROC)PlatformGetModuleSymbol(_glfw.glx.handle,"glXCreateWindow");
    _glfw.glx.GetVisualFromFBConfig = (PFNGLXGETVISUALFROMFBCONFIGPROC)PlatformGetModuleSymbol(_glfw.glx.handle,"glXGetVisualFromFBConfig");
    _glfw.glx.GetProcAddress = (PFNGLXGETPROCADDRESSPROC)PlatformGetModuleSymbol(_glfw.glx.handle,"glXGetProcAddress");
    _glfw.glx.QueryExtension(_glfw.x11.display,&_glfw.glx.errorBase,&_glfw.glx.eventBase);
    _glfw.glx.QueryVersion(_glfw.x11.display,&_glfw.glx.major,&_glfw.glx.minor);
    _glfw.glx.SwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)getProcAddressGLX("glXSwapIntervalEXT");
    _glfw.glx.CreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)getProcAddressGLX("glXCreateContextAttribsARB");
    GLXFBConfig native; XVisualInfo* result;
    GLXFBConfig* nativeConfigs; _GLFWfbconfig* usableConfigs; const _GLFWfbconfig* closest; int nativeCount,usableCount;
    nativeConfigs = _glfw.glx.GetFBConfigs(_glfw.x11.display, _glfw.x11.screen, &nativeCount);        
    usableConfigs = OS_Calloc(nativeCount,sizeof(_GLFWfbconfig)); usableCount = 0;
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
    _glfw.x11.xlib.Free(nativeConfigs); if (usableConfigs) OS_DeallocateRAM(usableConfigs,nativeCount*sizeof(_GLFWfbconfig));
    result = _glfw.glx.GetVisualFromFBConfig(_glfw.x11.display,native);
    Visual* visual=result->visual; int depth = result->depth; _glfw.x11.xlib.Free(result);
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
    GetWindowPos(window,&window->x11.xpos,&window->x11.ypos); GetWindowSize(window,&window->x11.width,&window->x11.height);
    int attribs[40],index=0; attribs[index++] = 0x2091/*major*/; attribs[index++] = 4; attribs[index++] = 0x2092/*minor*/; attribs[index++] = 3; /*OpenGL 4.3*/ attribs[index++] = 0x9126/*profile mask arb*/; attribs[index++] = 1/*core profile*/; attribs[index++] = 0L; attribs[index++] = 0L;
    window->context.glx.handle     = _glfw.glx.CreateContextAttribsARB(_glfw.x11.display,native,NULL,1,attribs);
    window->context.glx.window     = _glfw.glx.CreateWindow(_glfw.x11.display,native,window->x11.handle,NULL);
    window->context.glx.fbconfig   = native; window->context.makeCurrent = makeContextCurrentGLX;
    window->context.swapBuffers    = swapBuffersGLX; window->context.swapInterval = swapIntervalGLX;
    window->context.getProcAddress = getProcAddressGLX;
    _glfw.x11.xlib.MapWindow(_glfw.x11.display,window->x11.handle);
    if (_glfw.x11.NET_ACTIVE_WINDOW) sendEventToWM(window,_glfw.x11.NET_ACTIVE_WINDOW,1,0,0,0,0);
    else if (WindowVisible(window)) { _glfw.x11.xlib.RaiseWindow(_glfw.x11.display,window->x11.handle); _glfw.x11.xlib.SetInputFocus(_glfw.x11.display,window->x11.handle,2/*RevertToParent*/,0L); }
#endif
    return (GLFWwindow*)window;
}

void glfwSetWindowIcon(GLFWwindow* handle, const GLFWimage* images) { _GLFWwindow* window = (_GLFWwindow*) handle; SetWindowIcon(window,images); }
void glfwSetWindowSize(GLFWwindow* handle, int width, int height) { _GLFWwindow* window = (_GLFWwindow*)handle; window->videoMode.width=width; window->videoMode.height=height; SetWindowSize(window,width,height); }
void glfwSetWindowMonitor(GLFWwindow* wh, int xpos, int ypos, int width, int height) { _GLFWwindow* window = (_GLFWwindow*)wh; window->videoMode.width=width; window->videoMode.height=height; SetWindowMonitor(window,xpos,ypos,width,height); }
void TextEntry(i32 k) {
    if (k == GLFW_KEY_U && Sys_Input.keyStates[GLFW_KEY_LEFT_CONTROL].down) { Sys_Global.playerName[0] = '\0'; currentPlayerNameLength = 0; return; }
    if (k == GLFW_KEY_ENTER || k == GLFW_KEY_KP_ENTER) { currentMenuItem++; return; }
    if (k == GLFW_KEY_BACKSPACE && currentPlayerNameLength > 0) { Sys_Global.playerName[--currentPlayerNameLength] = '\0'; return; }
    if (currentPlayerNameLength >= 26) return;
    char c = (k >= GLFW_KEY_A && k <= GLFW_KEY_Z) ? 'a' + (k - GLFW_KEY_A) : ((k >= GLFW_KEY_1 && k <= GLFW_KEY_9) ? '1' + (k - GLFW_KEY_1) : ((k == GLFW_KEY_0) ? '0' : ((k == GLFW_KEY_SPACE) ? ' ' : 0)));
    if (c) { Sys_Global.playerName[currentPlayerNameLength] = c; Sys_Global.playerName[++currentPlayerNameLength] = '\0'; }
}

void GoIntoGame(void); void ConsoleEmulator(i32 keycode); extern bool enteringPlayerName;
void InputKey(_GLFWwindow* window,int key,int action) {
    if (key >= 0 && key <= 348) {
        i32 repeated = 0;
        if (action == GLFW_RELEASE && window->keys[key] == GLFW_RELEASE) return;
        if (action == GLFW_PRESS && window->keys[key] == GLFW_PRESS) repeated =  1;
        window->keys[key] = (char)action; if (repeated) action = GLFW_REPEAT;
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

void InputMouseClick(_GLFWwindow* window,int button,int action) { if (button<0 || button>7) {return;} if (button<=7) {window->mouseButtons[button] = (char)action;} Sys_Input.mouseButtons[button].down = Sys_Input.mouseButtons[button].pressed = (action == 1); Sys_Input.mouseButtons[button].released = (action == 0); }
void quat_from_yaw_pitch_roll(Quaternion* q, float yaw_deg, float pitch_deg, float roll_deg) {
    float yaw = deg2rad(yaw_deg), pitch = deg2rad(pitch_deg), roll = deg2rad(roll_deg);  // Around Z (forward)
    float cy = vcosf(yaw * 0.5f), sy = vsinf(yaw * 0.5f), cp = vcosf(pitch * 0.5f), sp = vsinf(pitch * 0.5f), cr = vcosf(roll * 0.5f), sr = vsinf(roll * 0.5f);
    q->w = cy*cp*cr + sy*sp*sr; q->x = cy*sp*cr + sy*cp*sr; /* X-axis (pitch) */ q->y = sy*cp*cr - cy*sp*sr; /* Y-axis (yaw) */ q->z = cy*cp*sr - sy*sp*cr; /* Z-axis (roll) */ // Skipping quat normalization, not needed
} 

void InputCursorPos(_GLFWwindow* window,double xpos,double ypos) { // static const float HeadBobRate   = 0.2f, HeadBobAmount = 0.08f,bobTarget = 0.3f; TODO
    if (window->virtualCursorPosX == xpos && window->virtualCursorPosY == ypos) return;
    window->virtualCursorPosX = xpos; window->virtualCursorPosY = ypos; if (!Sys_Input.window_has_focus) return;
    
    Sys_Input.currentMouse_dx = (i32)(xpos - Sys_Input.last_mouse_x); Sys_Input.currentMouse_dy = (i32)(ypos - Sys_Input.last_mouse_y);
    Sys_Input.last_mouse_x = xpos; Sys_Input.last_mouse_y = ypos;
    if (Sys_Input.ignore_next_mouse_delta) { Sys_Input.ignore_next_mouse_delta = mouseMovementThisFrame = false; return; }

    if ((Sys_Global.inventoryMode && !Sys_Cheats.noHUD) || Sys_Global.menuActive || Sys_Global.gamePaused) { // Uses UI baseline resolution 1366x768
        i32 newX = clamp(Sys_Global.cursorPosition_x + Sys_Input.currentMouse_dx,0,1366); if (newX != Sys_Global.cursorPosition_x) {mouseMovementThisFrame = true;} Sys_Global.cursorPosition_x = newX;
        i32 newY = clamp(Sys_Global.cursorPosition_y + Sys_Input.currentMouse_dy,0, 768); if (newY != Sys_Global.cursorPosition_y) {mouseMovementThisFrame = true;} Sys_Global.cursorPosition_y = newY;
    }
    
    if (Sys_Global.gamePaused || Sys_Global.menuActive || Sys_Global.inventoryMode) return;
    
    float s = vclamp((float)Sys_Settings.MouseSensitivity / 100.0f, 0.01f, 1.0f) * 0.2f;
    cam_yaw += (float)Sys_Input.currentMouse_dx * s; if (cam_yaw >= 360.0f) {cam_yaw -= 360.0f;} if (cam_yaw < 0.0f)     {cam_yaw  += 360.0f;}
    cam_pitch+=(float)Sys_Input.currentMouse_dy * s; if (cam_pitch > 89.0f) {cam_pitch = 89.0f;} if (cam_pitch < -89.0f) {cam_pitch = -89.0f;} // Avoid gimbal lock at pure 90deg
    quat_from_yaw_pitch_roll(&Sys_Global.instances[PLAYER1].rotation,cam_yaw,cam_pitch,(Sys_Global.currentLevel == LEVEL_CYBERSPACE) ? cam_roll : 0.0f);
    Quaternion rot = Sys_Global.instances[PLAYER1].rotation; float y2 = rot.y * rot.y;  float xz = rot.x * rot.z;  float wy = rot.w * rot.y;
    Sys_Global.instances[PLAYER1].forward = normalize_vector3((Vector3){ 2.0f * (xz + wy), 2.0f * (rot.y * rot.z - rot.w * rot.x), 1.0f - 2.0f * (rot.x * rot.x + y2) });
    Sys_Global.instances[PLAYER1].right = normalize_vector3((Vector3){ 1.0f - 2.0f * (y2 + rot.z * rot.z), 2.0f * (rot.x * rot.y + rot.w * rot.z), 2.0f * (xz - wy) });
}

void JoystickConnection(_GLFWjoystick* js, int e) {
    js->connected = (e == 0x00040001/*connected*/) ? 1 : (e == 0x00040002/*disconnected*/) ? 0 : js->connected;    
    int jid = (int)(js - _glfw.joysticks); if (jid > GLFW_JOYSTICK_LAST) return;
    
    Sys_Input.joystickPresent[jid] = (e == 0x00040001/*connected*/);
    if (!Sys_Input.joystickPresent[jid]) { MemSetToVForNBytes(Sys_Input.joystickButtons,0,sizeof(Sys_Input.joystickButtons)); MemSetToVForNBytes(Sys_Input.joystickHats,0,sizeof(Sys_Input.joystickHats)); } // Clear
}

void InputJoystickAxis(_GLFWjoystick* js,int axis,float value) { js->axes[axis] = value; }
void InputJoystickButton(_GLFWjoystick* js,int button,char value) { js->buttons[button] = value; }
void InputJoystickHat(_GLFWjoystick* js,int hat,char value) {
    int base = js->buttonCount + hat * 4;
    js->buttons[base+0] = (value & 0x01) ? GLFW_PRESS : GLFW_RELEASE; js->buttons[base+1] = (value & 0x02) ? GLFW_PRESS : GLFW_RELEASE; 
    js->buttons[base+2] = (value & 0x04) ? GLFW_PRESS : GLFW_RELEASE; js->buttons[base+3] = (value & 0x08) ? GLFW_PRESS : GLFW_RELEASE;
    js->hats[hat] = value;
}

_GLFWjoystick* _glfwAllocJoystick(const char* name,const char* guid,int axisCount,int buttonCount,int hatCount) {
    int jid; _GLFWjoystick* js;
    for (jid = 0; jid <= GLFW_JOYSTICK_LAST; jid++) { if (!_glfw.joysticks[jid].allocated) break; }
    if (jid > GLFW_JOYSTICK_LAST) return NULL;
    js = _glfw.joysticks + jid;
    js->allocated = 1; js->axisCount = axisCount; js->buttonCount = buttonCount; js->hatCount = hatCount;
    js->axesSize = axisCount*sizeof(float); js->axes = OS_Calloc(axisCount,sizeof(float)); js->buttonsSize = (buttonCount + (size_t)hatCount * 4);
    js->buttons = OS_Calloc(buttonCount + (size_t)hatCount * 4,1); js->hatsSize = hatCount; js->hats = OS_Calloc(hatCount,1);
    StringCopyInto_A_From_B(js->name,name,sizeof(js->name)); StringCopyInto_A_From_B(js->guid,guid,sizeof(js->guid));
    return js;
}

bool JoystickPresent(int jid) {
    if (jid < 0 || jid > GLFW_JOYSTICK_LAST || (!_glfw.joysticksInitialized && !InitJoysticks())) return false;
    
    _glfw.joysticksInitialized = 1; _GLFWjoystick* js = _glfw.joysticks + jid; return js->connected ? PollJoystick(js) : false;
}

void _glfwFreeJoystick(_GLFWjoystick* js) { OS_DeallocateRAM(js->axes,js->axesSize); OS_DeallocateRAM(js->buttons,js->buttonsSize); OS_DeallocateRAM(js->hats,js->hatsSize); MemSetToVForNBytes(js,0,sizeof(_GLFWjoystick)); }
void InputProcessing(void) {
    PollEvents();
    for (int jid = GLFW_JOYSTICK_1; jid <= GLFW_JOYSTICK_LAST; ++jid) { // Input Poll
        if (!JoystickPresent(jid)) continue;
        _GLFWjoystick* js = _glfw.joysticks + jid; if (!js->connected) continue;

        PollJoystick(js);
        int totalButtons = js->buttonCount + js->hatCount * 4;
        for (int i = 0; i < totalButtons && i < 16; ++i) {
            KeyState* k = &Sys_Input.joystickButtons[jid - GLFW_JOYSTICK_1][i];
            bool down = js->buttons[i] == GLFW_PRESS;
            k->pressed = down && !k->down; k->released = !down && k->down; k->down = down;
        }

        for (int i = 0; i < js->hatCount && i < 5; ++i) { Sys_Input.joystickHats[i].down = js->hats[i]; }
//         for (int i = 0; i < js->axisCount && i < MAX_JOYSTICK_AXES; ++i) { Sys_Input.joystickAxes[jid - GLFW_JOYSTICK_1][i] = js->axes[i]; } TODO??
    }

    if (Sys_Input.keyStates[GLFW_KEY_E].pressed) play_wav("./Audio/cyborgs/yourlevelsareterrible.wav",0.1f,(Vector3){},false);
    if (Sys_Input.window_has_focus) {
        if (Sys_Input.keyStates[GLFW_KEY_CAPS_LOCK].pressed) Sys_Input.isCapsLockOn = !Sys_Input.isCapsLockOn; // Change capslock state to match keyboard having toggled.  Must always happen regardless of paused/menu.
        ProcessInput(); // Calls ApplyPlayerMovements(), needs called without checking paused state for menus handling.
    }
}

KeyState* GetCodeMapping(int settingIndex) {
    i32 i = Sys_Settings.InputCodeSettings[settingIndex]; // Get table index into all recognized inputs
    if (i == 148 || i >= MAX_KEYS) return &Sys_Input.keyStates[MAX_KEYS - 1]; // UNUSED NULL (e.g. setting unbound)
    if (i >= 53 && i <= 61) return &Sys_Input.mouseButtons[inputElements[i].value];
    if (i >= 62 && i <= 77) return &Sys_Input.joystickButtons[GLFW_JOYSTICK_1][inputElements[i].value];
    if ((i >= 78 && i <= 79) || (i >= 132 && i <= 133)) return &Sys_Input.joystickHats[inputElements[i].value];
    return &Sys_Input.keyStates[inputElements[i].value];
}

void SetVSync(void) { _GLFWwindow* handle = (_GLFWwindow*)window; handle->context.swapInterval((i32)Sys_Settings.Vsync); }
bool GetKeyRiseEdgeOrHeld(int sI, bool onRise) { i32 i = Sys_Settings.InputCodeSettings[sI]; if (i == 128) {return Sys_Input.scrollDelta > 0;} if (i == 129) {return Sys_Input.scrollDelta < 0;} KeyState* k = GetCodeMapping(sI); return onRise ? k->pressed : k->down; }
void InputClearRisingAndFallingEdges(void) { for (i32 i=0;i<MAX_KEYS;++i) {Sys_Input.keyStates[i].pressed = Sys_Input.keyStates[i].released = false;} for (i32 i=0;i<MAX_MOUSE_BUTTONS;i++) {Sys_Input.mouseButtons[i].pressed = Sys_Input.mouseButtons[i].released = false;} Sys_Input.scrollDelta = 0; } // Can't memset as we want to preserve down state
void CenterWindowOnMonitor(void) {
    int monitorCount; _GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
    if (Sys_Settings.CurrentMonitor > (monitorCount - 1)) { Sys_Settings.CurrentMonitor = 0; SaveConfig(); }
    int mx,my; _GLFWmonitor* next = monitors[Sys_Settings.CurrentMonitor];
    glfwGetMonitorPos(next,&mx,&my);
    const GLFWvidmode* mode = glfwGetVideoMode(next);
    int xpos = mx + (mode->width - Sys_Settings.ScreenWidth) / 2, ypos = my + (mode->height - Sys_Settings.ScreenHeight) / 2;
    glfwSetWindowPosition(window,xpos,ypos);
    Sys_Input.ignore_next_mouse_delta = true;
}

extern bool resDropdownOpen; extern int resDropdownCount,resSelectedIdx; typedef struct {int w,h;} ResMode; extern ResMode resModes[8];
_GLFWmonitor* GetCurrentMonitor(void) {
    int wx=0,wy=0,ww=0,wh=0; GetWindowPos(((_GLFWwindow*)window),&wx,&wy); GetWindowSize(((_GLFWwindow*)window),&ww,&wh);
    _GLFWmonitor* bestMonitor = glfwGetPrimaryMonitor();
    int bestArea=0,monitorCount; _GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
    for (int i=0;i<monitorCount;++i) {
        int mx,my; glfwGetMonitorPos(monitors[i],&mx,&my);
        const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
        int left=vmax(wx,mx), right=vmin(wx + ww,mx + mode->width), top=vmax(wy,my), bottom=vmin(wy + wh,my + mode->height);
        int area = (right > left && bottom > top) ? (right - left) * (bottom - top) : 0;
        if (area > bestArea) { bestArea = area; bestMonitor = monitors[i]; }
    }
    return bestMonitor;
}

void ChangeResolution(void) {
    if (resDropdownCount < 1) return;

    resSelectedIdx = (resSelectedIdx + 1) % resDropdownCount;
    Sys_Settings.ScreenWidth  = (u32)resModes[resSelectedIdx].w; Sys_Settings.ScreenHeight = (u32)resModes[resSelectedIdx].h;
    _GLFWmonitor* monitor = GetCurrentMonitor(); if (!monitor) monitor = glfwGetPrimaryMonitor();
    int mx,my; glfwGetMonitorPos(monitor,&mx,&my);
    const GLFWvidmode* desktop = glfwGetVideoMode(monitor);
    int xpos = mx + (desktop->width - (int)Sys_Settings.ScreenWidth) / 2, ypos = my + (desktop->height - (int)Sys_Settings.ScreenHeight) / 2;
    glfwSetWindowSize(window, (int)Sys_Settings.ScreenWidth,(int)Sys_Settings.ScreenHeight);
    glfwSetWindowPosition(window,xpos,ypos);
    UpdateScreenSize((int)Sys_Settings.ScreenWidth,(int)Sys_Settings.ScreenHeight);
    resDropdownOpen = false;
    SaveConfig();
}

void GatherResolutionModes(void) {
    resDropdownCount = 0; _GLFWmonitor* monitor = GetCurrentMonitor(); if (!monitor) monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* desktop = glfwGetVideoMode(monitor); if (!desktop) return;

    static const struct {int w,h;} commonRes[] = {{320,200},{640,400},{640,480},{800,600},{1024,768},{1280,720},{1280,800},{1366,768},{1440,900},{1600,900},{1920,1080},{2560,1440}};
    int maxW = desktop->width, maxH = desktop->height,j;
    for (int i = 0; i < (int)(sizeof(commonRes)/sizeof(commonRes[0])) && resDropdownCount < 8; ++i) {
        if (commonRes[i].w > maxW || commonRes[i].h > maxH || commonRes[i].w < 320 || commonRes[i].h < 200) continue;

        for (j = 0; j < resDropdownCount; ++j) { if (resModes[j].w == commonRes[i].w && resModes[j].h == commonRes[i].h) {break;} }
        if (j == resDropdownCount) resModes[resDropdownCount++] = (ResMode){commonRes[i].w,commonRes[i].h};
    }

    if (resDropdownCount < 8) resModes[resDropdownCount++] = (ResMode){desktop->width,desktop->height};
    resSelectedIdx = 0;
    for (int i = 0; i < resDropdownCount; ++i) {
        if (resModes[i].w == (int)Sys_Settings.ScreenWidth && resModes[i].h == (int)Sys_Settings.ScreenHeight) { resSelectedIdx = i; break; }
    }
}

void ChangeFullScreenWindowed(void) {
    int x,y,w,h,mx,my,monitorCount; _GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
    _GLFWmonitor* monitor = monitors[Sys_Settings.CurrentMonitor];
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    glfwGetMonitorWorkarea(monitor,&x,&y,&w,&h);
    ((_GLFWwindow*)window)->decorated = (i32)(!Sys_Settings.Fullscreen); SetWindowDecorated(((_GLFWwindow*)window),(i32)(!Sys_Settings.Fullscreen));
    if (Sys_Settings.Fullscreen) {
        glfwSetWindowMonitor(window,x,y,w,h);
        Sys_Settings.ScreenWidth = w; Sys_Settings.ScreenHeight = h;
    } else {
        glfwGetMonitorPos(monitor,&mx,&my);
        Sys_Settings.ScreenWidth  = vmax(vmin((w*3)/4,1366),320); Sys_Settings.ScreenHeight = vmax(vmin((h*3)/4,768),200);
        int xpos = mx + (mode->width - Sys_Settings.ScreenWidth) / 2, ypos = my + (mode->height - Sys_Settings.ScreenHeight) / 2;
        glfwSetWindowMonitor(window,xpos,ypos,Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight);
    }

    UpdateScreenSize(Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight);
}

static double monitorSwitchTime;
void CycleToNextMonitor(void) {
    if (get_time() < monitorSwitchTime) return;

    monitorSwitchTime = get_time() + 0.5; // Prevent toggling rapidly on accident
    int monitorCount; _GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
    if (Sys_Settings.CurrentMonitor > (monitorCount - 1)) { Sys_Settings.CurrentMonitor = 0; SaveConfig(); }
    if (!monitors || monitorCount < 2) return;

    Sys_Settings.CurrentMonitor = (Sys_Settings.CurrentMonitor + 1) % monitorCount;
    SaveConfig(); CenterWindowOnMonitor();
}
