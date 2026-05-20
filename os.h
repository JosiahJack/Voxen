// os.h - starts most translation units and defines the shim layer between Voxen and the OS as well as defining project wide OS defines.
#pragma once
typedef __SIZE_TYPE__ size_t;
typedef __UINTPTR_TYPE__ uintptr_t;
typedef __INTPTR_TYPE__ intptr_t;
typedef __INT8_TYPE__ i8;
typedef __INT16_TYPE__ i16;
typedef __INT32_TYPE__ i32;
typedef __UINT8_TYPE__ u8;
typedef __UINT16_TYPE__ u16;
typedef __UINT32_TYPE__ u32;
typedef __INT64_TYPE__ i64;
typedef __UINT64_TYPE__ u64;
#define bool unsigned char
#define true 1
#define false 0
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#define NULL ((void *)0)
#if defined(_WIN32) || defined(__CYGWIN__) // Interop - To Mod (keep the same as interop.h!!)
    #define ENGINE_TO_MOD __declspec(dllexport) __cdecl
#else
    #define ENGINE_TO_MOD __attribute__((visibility("default")))
#endif
ENGINE_TO_MOD void DualLogError(const char* fmt, ...);
char* StringFindSubstring(const char* haystack, const char* needle);
void DebugRAM(const char *context);
#include <pthread.h> // For model and texture loading only
#if defined(_WIN32) || defined(_WIN64)
    #define WINDOWS
    #define WIN32_LEAN_AND_MEAN // Let 'er rip, tater chip
    #define NOMINMAX
    #define VC_EXTRALEAN
    #define UNICODE
    #define OEMRESOURCE // OEM cursor resources for win init
    #define OCR_NORMAL 32512
    #define WINVER 0x0601 // Windows 7 or later
    #define _MINWINDEF_
    #define STRICT 1
    #define WIN32
    #define BASETYPES
    typedef unsigned __LONG32 ULONG;
    typedef ULONG *PULONG;
    typedef unsigned short USHORT;
    typedef USHORT *PUSHORT;
    typedef unsigned char UCHAR;
    typedef UCHAR *PUCHAR;
    typedef char *PSZ;
    #define MAX_PATH 260
    #define FALSE 0
    #define TRUE 1
    #undef far
    #undef near
    #undef pascal
    #define far
    #define near
    #define pascal __stdcall
    #define cdecl
    #define CDECL
    #define CALLBACK __stdcall
    #define WINAPI __stdcall
    #define WINAPIV __cdecl
    #define APIENTRY WINAPI
    #define APIPRIVATE __stdcall
    #define PASCAL __stdcall
    #define WINAPI_INLINE WINAPI
    #undef FAR
    #undef NEAR
    #define FAR
    #define NEAR
    #define CONST const
    #define _DEF_WINBOOL_
    typedef int WINBOOL;
    #pragma push_macro("BOOL")
    #undef BOOL
    typedef int BOOL;
    #define BOOL WINBOOL
    typedef BOOL *PBOOL;
    typedef BOOL *LPBOOL;
    #pragma pop_macro("BOOL")
    typedef unsigned char BYTE;
    typedef unsigned short WORD;
    typedef unsigned __LONG32 DWORD;
    typedef float FLOAT;
    typedef FLOAT *PFLOAT;
    typedef BYTE *PBYTE;
    typedef BYTE *LPBYTE;
    typedef int *PINT;
    typedef int *LPINT;
    typedef WORD *PWORD;
    typedef WORD *LPWORD;
    typedef __LONG32 *LPLONG;
    typedef DWORD *PDWORD;
    typedef DWORD *LPDWORD;
    typedef void *LPVOID;
    #define _LPCVOID_DEFINED
    typedef CONST void *LPCVOID;
    typedef int INT;
    typedef unsigned int UINT;
    typedef unsigned int *PUINT;
    #include <winnt.h>
    typedef UINT_PTR WPARAM;
    typedef LONG_PTR LPARAM;
    typedef LONG_PTR LRESULT;
    #define max(a, b) (((a) > (b)) ? (a) : (b))
    #define min(a, b) (((a) < (b)) ? (a) : (b))
    #define MAKEWORD(a,b) ((WORD) (((BYTE) (((DWORD_PTR) (a)) & 0xff)) | ((WORD) ((BYTE) (((DWORD_PTR) (b)) & 0xff))) << 8))
    #define MAKELONG(a, b) ((LONG) (((WORD) (((DWORD_PTR) (a)) & 0xffff)) | ((DWORD) ((WORD) (((DWORD_PTR) (b)) & 0xffff))) << 16))
    #define LOWORD(l) ((WORD) (((DWORD_PTR) (l)) & 0xffff))
    #define HIWORD(l) ((WORD) ((((DWORD_PTR) (l)) >> 16) & 0xffff))
    #define LOBYTE(w) ((BYTE) (((DWORD_PTR) (w)) & 0xff))
    #define HIBYTE(w) ((BYTE) ((((DWORD_PTR) (w)) >> 8) & 0xff))
    typedef HANDLE *SPHANDLE;
    typedef HANDLE *LPHANDLE;
    typedef HANDLE HGLOBAL;
    typedef HANDLE HLOCAL;
    typedef HANDLE GLOBALHANDLE;
    typedef HANDLE LOCALHANDLE;
    typedef INT_PTR(WINAPI *FARPROC)(void);
    typedef INT_PTR(WINAPI *NEARPROC)(void);
    typedef INT_PTR(WINAPI *PROC)(void);
    typedef WORD ATOM;
    typedef int HFILE;
    DECLARE_HANDLE(HINSTANCE);
    DECLARE_HANDLE(HKEY);
    typedef HKEY *PHKEY;
    DECLARE_HANDLE(HKL);
    DECLARE_HANDLE(HLSURF);
    DECLARE_HANDLE(HMETAFILE);
    typedef HINSTANCE HMODULE;
    DECLARE_HANDLE(HRGN);
    DECLARE_HANDLE(HRSRC);
    DECLARE_HANDLE(HSPRITE);
    DECLARE_HANDLE(HSTR);
    DECLARE_HANDLE(HTASK);
    DECLARE_HANDLE(HWINSTA);
    typedef struct _FILETIME { DWORD dwLowDateTime; DWORD dwHighDateTime; } FILETIME,*PFILETIME,*LPFILETIME;
    #define _FILETIME_
    typedef void *HGDIOBJ;
    DECLARE_HANDLE(HWND);
    DECLARE_HANDLE(HHOOK);
    DECLARE_HANDLE(HEVENT);
    DECLARE_HANDLE(HACCEL);
    DECLARE_HANDLE(HBITMAP);
    DECLARE_HANDLE(HBRUSH);
    DECLARE_HANDLE(HCOLORSPACE);
    DECLARE_HANDLE(HDC);
    DECLARE_HANDLE(HGLRC);
    DECLARE_HANDLE(HDESK);
    DECLARE_HANDLE(HENHMETAFILE);
    DECLARE_HANDLE(HFONT);
    DECLARE_HANDLE(HICON);
    DECLARE_HANDLE(HMENU);
    DECLARE_HANDLE(HPALETTE);
    DECLARE_HANDLE(HPEN);
    DECLARE_HANDLE(HMONITOR);
    #define HMONITOR_DECLARED 1
    DECLARE_HANDLE(HWINEVENTHOOK);
    typedef HICON HCURSOR;
    typedef DWORD COLORREF;
    typedef struct tagPOINT { LONG x; LONG y; } POINT,*PPOINT,*NPPOINT,*LPPOINT;
    typedef struct _POINTL { LONG x; LONG y; } POINTL,*PPOINTL;
    typedef struct tagRECT { LONG left; LONG top; LONG right; LONG bottom; } RECT,*PRECT,*NPRECT,*LPRECT;
    typedef const RECT *LPCRECT;
    typedef struct _RECTL { LONG left; LONG top; LONG right; LONG bottom; } RECTL,*PRECTL,*LPRECTL;
    typedef struct tagSIZE { LONG cx; LONG cy; } SIZE,*PSIZE,*LPSIZE;
    typedef SIZE SIZEL;
    typedef struct tagPOINTS { SHORT x; SHORT y; } POINTS,*PPOINTS,*LPPOINTS;
    DECLARE_HANDLE(DPI_AWARENESS_CONTEXT);
    #include <apisetcconv.h>
    #include <minwinbase.h>
    WINBASEAPI HANDLE WINAPI CreateFileA (LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
    WINBASEAPI WINBOOL WINAPI ReadFile (HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
    WINBASEAPI WINBOOL WINAPI WriteFile (HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
    WINBASEAPI WINBOOL WINAPI GetFileSizeEx (HANDLE hFile, PLARGE_INTEGER lpFileSize);
    WINBASEAPI WINBOOL WINAPI SetFilePointerEx (HANDLE hFile, LARGE_INTEGER liDistanceToMove, PLARGE_INTEGER lpNewFilePointer, DWORD dwMoveMethod);
    #define INVALID_HANDLE_VALUE ((HANDLE) (LONG_PTR)-1)
    WINBASEAPI WINBOOL WINAPI CloseHandle (HANDLE hObject);
    WINBASEAPI WINBOOL WINAPI VirtualFree (LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType);
    WINBASEAPI LPVOID WINAPI VirtualAlloc (LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
    WINBASEAPI LPVOID WINAPI MapViewOfFile (HANDLE hFileMappingObject, DWORD dwDesiredAccess, DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow, SIZE_T dwNumberOfBytesToMap);
    WINBASEAPI LPVOID WINAPI MapViewOfFileEx (HANDLE hFileMappingObject, DWORD dwDesiredAccess, DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow, SIZE_T dwNumberOfBytesToMap, LPVOID lpBaseAddress);
    WINBASEAPI WINBOOL WINAPI UnmapViewOfFile (LPCVOID lpBaseAddress);
    WINBASEAPI HANDLE WINAPI CreateFileMappingW (HANDLE hFile, LPSECURITY_ATTRIBUTES lpFileMappingAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCWSTR lpName);
    #define CreateFileMapping CreateFileMappingW
    WINBASEAPI HANDLE WINAPI GetStdHandle (DWORD nStdHandle);
    WINBASEAPI DECLSPEC_NORETURN VOID WINAPI ExitProcess (UINT uExitCode);
    WINBASEAPI WINBOOL WINAPI QueryPerformanceCounter (LARGE_INTEGER *lpPerformanceCount);
    WINBASEAPI WINBOOL WINAPI QueryPerformanceFrequency (LARGE_INTEGER *lpFrequency);
    typedef struct _SYSTEM_INFO { __C89_NAMELESS union { DWORD dwOemId; __C89_NAMELESS struct { WORD wProcessorArchitecture; WORD wReserved; } DUMMYSTRUCTNAME; } DUMMYUNIONNAME; DWORD dwPageSize; LPVOID lpMinimumApplicationAddress; LPVOID lpMaximumApplicationAddress; DWORD_PTR dwActiveProcessorMask; DWORD dwNumberOfProcessors; DWORD dwProcessorType; DWORD dwAllocationGranularity; WORD wProcessorLevel; WORD wProcessorRevision; } SYSTEM_INFO, *LPSYSTEM_INFO;
    WINBASEAPI VOID WINAPI GetSystemInfo (LPSYSTEM_INFO lpSystemInfo);
    #define MAKEINTATOM(i) (LPTSTR) ((ULONG_PTR)((WORD)(i)))
    #include <winerror.h>
    WINBASEAPI HANDLE WINAPI CreateFileMappingA (HANDLE hFile, LPSECURITY_ATTRIBUTES lpFileMappingAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCSTR lpName);
    #include <wingdi.h>
    #include <winuser.h>
    #include <winnls.h>
    //#define WINAPI __stdcall
    //#define BYTE   u8
    //#define WORD   u16
    //#define SHORT  i16
    //#define DWORD  unsigned long
    //#define LONG   i32
    //#define ULONG  unsigned long
    //#define UINT32 u32
    //#define HANDLE void*
    //#define HRESULT i32
    //typedef const wchar_t* LPCWSTR;
    //typedef void* HGLRC;
    //#define SW_HIDE 0
    //#define WS_CLIPSIBLINGS        0x04000000L
    //#define WS_CLIPCHILDREN        0x02000000L
    //#define WS_EX_OVERLAPPEDWINDOW 0x00000040L
    //#define WS_EX_APPWINDOW        0x00040000L
    //#define ZeroMemory(Destination,Length) MemSetToValueForNBytes((Destination),0,(Length))
    //#define INVALID_HANDLE_VALUE ((void*)(intptr_t)-1)
    //#define SW_SHOWNA 8
    //#define PFD_DRAW_TO_WINDOW 0x00000004
    //#define PFD_SUPPORT_OPENGL 0x00000020
    //#define PFD_DOUBLEBUFFER   0x00000001
    //#define PFD_TYPE_RGBA 0
    //#define FAILED(hr) ((i32)(hr) < 0)
    //#define VOID void
    //#define UINT unsigned int
    //#define WCHAR wchar_t
    //#define WS_MINIMIZEBOX 0x00020000L
    //#define WS_SYSMENU     0x00080000L
    //#define WS_CAPTION     0x00C00000L
    //#define WS_POPUP       0x80000000L
    //#define BI_BITFIELDS 3
    //#define DIB_RGB_COLORS 0
    //#define IDC_ARROW ((const wchar_t*)(uintptr_t)32512)
    //#define WM_SETICON 0x0080
    //#define ICON_BIG 1
    //#define ICON_SMALL 0
    //#define FALSE 0
    //#define TRUE 1
    //#define GWL_STYLE (-16)
    //#define HWND_TOP ((void*)0)
    //#define SWP_NOSIZE     0x0001
    //#define SWP_NOMOVE     0x0002
    //#define SWP_NOZORDER   0x0004
    //#define SWP_NOACTIVATE 0x0010
    //#define CALLBACK __stdcall
    //#define WM_MOUSEACTIVATE 0x0021
    //#define WM_LBUTTONDOWN 0x0201
    //#define WM_CAPTURECHANGED 0x0215
    //#define WM_SETFOCUS 0x0007
    //#define WM_KILLFOCUS 0x0008
    //#define WM_SYSCOMMAND 0x0112
    //#define WM_CLOSE 0x0010
    //#define WM_KEYDOWN 0x0100
    //#define WM_SYSKEYDOWN 0x0104
    //#define WM_KEYUP 0x0101
    //#define WM_SYSKEYUP 0x0105
    //#define HTCLIENT 1
    //#define SC_SCREENSAVE 0xF140
    //#define SC_MONITORPOWER 0xF170
    //#define SC_KEYMENU 0xF100
    //#define KF_EXTENDED 0x0100
    //#define KF_UP 0x8000
    //#define MAPVK_VK_TO_VSC 0
    //#define VK_CONTROL 0x11
    //#define PM_NOREMOVE 0x0000
    //#define LOWORD(l) ((unsigned short)(((uintptr_t)(l)) & 0xffff))
    //#define HIWORD(l) ((unsigned short)((((uintptr_t)(l)) >> 16) & 0xffff))
    //#define VK_SHIFT 0x10
    //#define VK_MENU 0x12
    //#define VK_SNAPSHOT 0x2C
    //#define VK_PROCESSKEY 0xE5
    //#define WM_MOVE 0x0003
    //#define WM_SIZE 0x0005
    //#define WM_MOUSEMOVE 0x0200
    //#define WM_LBUTTONUP 0x0202
    //#define WM_RBUTTONDOWN 0x0204
    //#define WM_RBUTTONUP 0x0205
    //#define WM_MBUTTONDOWN 0x0206
    //#define WM_MBUTTONUP 0x0207
    //#define WM_MOUSEWHEEL 0x020A
    //#define WM_XBUTTONDOWN 0x020B
    //#define WM_XBUTTONUP 0x020C
    //#define WM_MOUSELEAVE 0x02A3
    //#define WM_GETMINMAXINFO 0x0024
    //#define WM_ENTERMENULOOP 0x0211
    //#define WM_EXITMENULOOP 0x0212
    //#define WM_ENTERSIZEMOVE 0x0231
    //#define WM_EXITSIZEMOVE 0x0232
    //#define TME_LEAVE 0x00000002
    //#define WHEEL_DELTA 120
    //#define XBUTTON1 0x0001
    //#define GET_XBUTTON_WPARAM(w) (short)HIWORD(w)
    //#define WM_INPUT 0x00FF
    //#define WM_ERASEBKGND 0x0014
    //#define WM_NCACTIVATE 0x0086
    //#define WM_NCPAINT 0x0085
    //#define WM_SETCURSOR 0x0020
    //#define SIZE_MINIMIZED 1
    //#define MONITOR_DEFAULTTONEAREST 2
    //#define SWP_NOCOPYBITS 0x0100
    //#define SWP_NOOWNERZORDER 0x0200
    //#define SWP_FRAMECHANGED 0x0020
    //#define HWND_NOTOPMOST ((void*)-2)
    //#define WS_OVERLAPPED 0x00000000L
    //#define WS_CAPTION 0x00C00000L
    //#define WS_SYSMENU 0x00080000L
    //#define WS_THICKFRAME 0x00040000L
    //#define WS_MINIMIZEBOX 0x00020000L
    //#define WS_MAXIMIZEBOX 0x00010000L
    //#define WS_OVERLAPPEDWINDOW (WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_THICKFRAME|WS_MINIMIZEBOX|WS_MAXIMIZEBOX)
    //#define PM_REMOVE 0x0001
    //#define WM_QUIT 0x0012
    //#define VK_LSHIFT 0xA0
    //#define VK_RSHIFT 0xA1
    //#define VK_LWIN 0x5B
    //#define VK_RWIN 0x5C
    //#define CP_UTF8 65001
    //#define VER_MAJORVERSION 0x0000001
    //#define VER_MINORVERSION 0x0000002
    //#define VER_SERVICEPACKMAJOR 0x0000020
    //#define VER_GREATER_EQUAL 3
    //#define ERROR_SUCCESS 0
    //#define ERROR_DEVICE_NOT_CONNECTED 1167
    //#define ENUM_CURRENT_SETTINGS ((unsigned long)-1)
    //#define DISPLAY_DEVICE_MODESPRUNED 0x08000000
    //#define HORZSIZE 4
    //#define VERTSIZE 6
    //#define LOGPIXELSX 8
    //#define LOGPIXELSY 10
    //#define HIBYTE(w) ((unsigned char)((((uintptr_t)(w)) >> 8) & 0xff))
    //#define LOWBYTE(w) ((unsigned char)(((uintptr_t)(w)) & 0xff))
    //#define DISPLAY_DEVICE_ACTIVE 0x00000001
    //#define DISPLAY_DEVICE_PRIMARY_DEVICE 0x00000004
    //#define WM_DISPLAYCHANGE 0x007E
    //#define WM_DEVICECHANGE 0x0219
    //#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
    //#define LOBYTE(w) ((unsigned char)(((uintptr_t)(w)) & 0xff))
    //#define CS_VREDRAW 0x0001
    //#define CS_HREDRAW 0x0002
    //#define CS_OWNDC 0x0020
    //#define CW_USEDEFAULT ((int)0x80000000)
    //#define GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS 0x00000004
    //#define GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT 0x00000002
    //#define MAKEINTATOM(i) ((const wchar_t*)(uintptr_t)((unsigned short)(i)))
    //#define WINUSERAPI __declspec(dllimport)
    //#define WINBASEAPI __declspec(dllimport)
    //#define WINGDIAPI  __declspec(dllimport)
    //typedef intptr_t LRESULT;
    //typedef intptr_t LPARAM;
    //typedef unsigned short ATOM;
    //typedef void* HINSTANCE;
    //typedef void* HCURSOR;
    //typedef void* HMONITOR;
    //typedef void* HMODULE;
    //typedef void* HICON;
    //typedef void* HBITMAP;
    //typedef unsigned long long ULONGLONG;
    //typedef intptr_t (__stdcall *PROC)(void);
    //typedef const char* LPCSTR;
    //typedef void* HDC;
    //typedef void* HWND;
    //typedef void* HDEVNOTIFY;
    //typedef uintptr_t WPARAM;
    //typedef intptr_t (__stdcall *WNDPROC)(void*, unsigned int, uintptr_t, intptr_t);
    //typedef void* LPVOID;
    //typedef struct { unsigned long cb; wchar_t DeviceName[32],DeviceString[128]; unsigned long StateFlags; wchar_t DeviceID[128],DeviceKey[128]; } DISPLAY_DEVICEW;
    //typedef struct { unsigned long Data1; unsigned short Data2; unsigned short Data3; unsigned char Data4[8]; } GUID;
    //typedef struct { void* hwnd; unsigned int message; uintptr_t wParam; intptr_t lParam; unsigned long time; struct { long x; long y; } pt; } MSG;
    //typedef struct { unsigned int cbSize; unsigned int style; WNDPROC lpfnWndProc; int cbClsExtra; int cbWndExtra; void* hInstance; void* hIcon; void* hCursor; void* hbrBackground; LPCWSTR lpszMenuName; LPCWSTR lpszClassName; void* hIconSm; } WNDCLASSEXW;
    //typedef union { struct { unsigned long LowPart; long HighPart; }; long long QuadPart; } LARGE_INTEGER;
    //typedef struct { unsigned short nSize; unsigned short nVersion; unsigned long dwFlags; unsigned char iPixelType; unsigned char cColorBits; unsigned char cRedBits; unsigned char cRedShift; unsigned char cGreenBits; unsigned char cGreenShift; unsigned char cBlueBits; unsigned char cBlueShift; unsigned char cAlphaBits; unsigned char cAlphaShift; unsigned char cAccumBits; unsigned char cAccumRedBits; unsigned char cAccumGreenBits; unsigned char cAccumBlueBits; unsigned char cAccumAlphaBits; unsigned char cDepthBits; unsigned char cStencilBits; unsigned char cAuxBuffers; unsigned char iLayerType; unsigned char bReserved; unsigned long dwLayerMask; unsigned long dwVisibleMask; unsigned long dwDamageMask; } PIXELFORMATDESCRIPTOR;
    //typedef struct { unsigned long Data1; unsigned short Data2; unsigned short Data3; unsigned char Data4[8]; } IID;
    //typedef struct { union { unsigned long dwOemId; struct { unsigned short wProcessorArchitecture; unsigned short wReserved; }; }; unsigned long dwPageSize; void* lpMinimumApplicationAddress; void* lpMaximumApplicationAddress; uintptr_t dwActiveProcessorMask; unsigned long dwNumberOfProcessors; unsigned long dwProcessorType; unsigned long dwAllocationGranularity; unsigned short wProcessorLevel; unsigned short wProcessorRevision; } SYSTEM_INFO;
    //typedef struct { unsigned long Data1; unsigned short Data2; unsigned short Data3; unsigned char Data4[8]; } CLSID;
    //typedef struct { unsigned long dwOSVersionInfoSize; unsigned long dwMajorVersion; unsigned long dwMinorVersion; unsigned long dwBuildNumber; unsigned long dwPlatformId; wchar_t szCSDVersion[128]; unsigned short wServicePackMajor; unsigned short wServicePackMinor; unsigned short wSuiteMask; unsigned char wProductType; unsigned char wReserved; } OSVERSIONINFOEXW;
    //typedef struct { unsigned long bV5Size; long bV5Width; long bV5Height; unsigned short bV5Planes; unsigned short bV5BitCount; unsigned long bV5Compression; unsigned long bV5SizeImage; long bV5XPelsPerMeter; long bV5YPelsPerMeter; unsigned long bV5ClrUsed; unsigned long bV5ClrImportant; unsigned long bV5RedMask; unsigned long bV5GreenMask; unsigned long bV5BlueMask; unsigned long bV5AlphaMask; unsigned long bV5CSType; struct { long x, y, z; } bV5Endpoints[3]; unsigned long bV5GammaRed; unsigned long bV5GammaGreen; unsigned long bV5GammaBlue; unsigned long bV5Intent; unsigned long bV5ProfileData; unsigned long bV5ProfileSize; unsigned long bV5Reserved; } BITMAPV5HEADER;
    //typedef struct { int fIcon; unsigned long xHotspot; unsigned long yHotspot; void* hbmMask; void* hbmColor; } ICONINFO;
    //typedef struct { unsigned long biSize; long biWidth; long biHeight; unsigned short biPlanes; unsigned short biBitCount; unsigned long biCompression; unsigned long biSizeImage; long biXPelsPerMeter; long biYPelsPerMeter; unsigned long biClrUsed; unsigned long biClrImportant; } BITMAPINFOHEADER;
    //typedef struct { BITMAPINFOHEADER bmiHeader; unsigned long bmiColors[3]; } BITMAPINFO;
    //typedef struct { long left; long top; long right; long bottom; } RECT;
    //typedef struct { long x; long y; } POINT;
    //typedef struct { unsigned int length; unsigned int flags; unsigned int showCmd; POINT ptMinPosition; POINT ptMaxPosition; RECT rcNormalPosition; } WINDOWPLACEMENT;
    //typedef struct { unsigned long cbSize,dwFlags; void* hwndTrack; unsigned long dwHoverTime; } TRACKMOUSEEVENT;
    //typedef struct { POINT ptReserved,ptMaxSize,ptMaxPosition,ptMinTrackSize,ptMaxTrackSize; } MINMAXINFO;
    //typedef struct { unsigned long cbSize; RECT rcMonitor; RECT rcWork; unsigned long dwFlags; } MONITORINFO;
    //typedef struct { unsigned long cbSize; RECT rcMonitor,rcWork; unsigned long dwFlags; wchar_t szDevice[32]; } MONITORINFOEXW;
    //typedef int (__stdcall *MONITORENUMPROC)(void*, void*, RECT*, intptr_t);
    //typedef struct {
        //wchar_t dmDeviceName[32];
        //unsigned short dmSpecVersion,dmDriverVersion,dmSize,dmDriverExtra;
        //unsigned long dmFields;
        //union {
            //struct { short dmOrientation,dmPaperSize,dmPaperLength,dmPaperWidth,dmScale,dmCopies,dmDefaultSource,dmPrintQuality; };
            //struct { struct { long x,y; } dmPosition; unsigned long dmDisplayOrientation,dmDisplayFixedOutput; };
        //};
        //short dmColor,dmDuplex,dmYResolution,dmTTOption,dmCollate;
        //wchar_t dmFormName[32];
        //unsigned short dmLogPixels;
        //unsigned long dmBitsPerPel,dmPelsWidth,dmPelsHeight;
        //union { unsigned long dmDisplayFlags,dmNup; };
        //unsigned long dmDisplayFrequency,dmICMMethod,dmICMIntent,dmMediaType,dmDitherType,dmReserved1,dmReserved2,dmPanningWidth,dmPanningHeight;
    //} DEVMODEW;
    //WINBASEAPI void         WINAPI ExitProcess(unsigned int uExitCode);
    //WINBASEAPI void         WINAPI GetSystemInfo(SYSTEM_INFO* lpSystemInfo);
    //WINBASEAPI int          WINAPI CloseHandle(void* hObject);
    //WINBASEAPI void*        WINAPI VirtualAlloc(void* lpAddress, size_t dwSize, unsigned long flAllocationType, unsigned long flProtect);
    //WINBASEAPI int          WINAPI VirtualFree(void* lpAddress, size_t dwSize, unsigned long dwFreeType);
    //WINBASEAPI void*        WINAPI CreateFileA(const char* lpFileName, unsigned long dwDesiredAccess, unsigned long dwShareMode, void* lpSecurityAttributes, unsigned long dwCreationDisposition, unsigned long dwFlagsAndAttributes, void* hTemplateFile);
    //WINBASEAPI void*        WINAPI CreateFileMappingA(void* hFile, void* lpAttributes, unsigned long flProtect, unsigned long dwMaximumSizeHigh, unsigned long dwMaximumSizeLow, const char* lpName);
    //WINBASEAPI void*        WINAPI MapViewOfFile(void* hFileMappingObject, unsigned long dwDesiredAccess, unsigned long dwFileOffsetHigh, unsigned long dwFileOffsetLow, size_t dwNumberOfBytesToMap);
    //WINBASEAPI void*        WINAPI MapViewOfFileEx(void* hFileMappingObject, unsigned long dwDesiredAccess, unsigned long dwFileOffsetHigh, unsigned long dwFileOffsetLow, size_t dwNumberOfBytesToMap, void* lpBaseAddress);
    //WINBASEAPI int          WINAPI UnmapViewOfFile(const void* lpBaseAddress);
    //WINBASEAPI int          WINAPI ReadFile(void* hFile, void* lpBuffer, unsigned long nNumberOfBytesToRead, unsigned long* lpNumberOfBytesRead, void* lpOverlapped);
    //WINBASEAPI int          WINAPI WriteFile(void* hFile, const void* lpBuffer, unsigned long nNumberOfBytesToWrite, unsigned long* lpNumberOfBytesWritten, void* lpOverlapped);
    //WINBASEAPI int          WINAPI GetFileSizeEx(void* hFile, LARGE_INTEGER* lpFileSize);
    //WINBASEAPI void*        WINAPI GetStdHandle(unsigned long nStdHandle);
    //WINBASEAPI void*        WINAPI LoadLibraryA(const char* lpLibFileName);
    //WINBASEAPI PROC         WINAPI GetProcAddress(HMODULE hModule, const char* lpProcName);
    //WINBASEAPI int          WINAPI GetModuleHandleExW(unsigned long dwFlags, const wchar_t* lpModuleName, void** phModule);
    //WINBASEAPI int          WINAPI MultiByteToWideChar(unsigned int CodePage, unsigned long dwFlags, const char* lpMultiByteStr, int cbMultiByte, wchar_t* lpWideCharStr, int cchWideChar);
    //WINBASEAPI int          WINAPI WideCharToMultiByte(unsigned int CodePage, unsigned long dwFlags, const wchar_t* lpWideCharStr, int cchWideChar, char* lpMultiByteStr, int cbMultiByte, const char* lpDefaultChar, int* lpUsedDefaultChar);
    //WINBASEAPI unsigned long long WINAPI VerSetConditionMask(unsigned long long ConditionMask, unsigned long TypeMask, unsigned char Condition);
    //WINBASEAPI int          WINAPI QueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount);
    //WINBASEAPI int          WINAPI QueryPerformanceFrequency(LARGE_INTEGER* lpFrequency);
    
    //WINUSERAPI int          WINAPI ShowWindow(void* hWnd, int nCmdShow);
    //WINUSERAPI void*        WINAPI SetFocus(void* hWnd);
    //WINUSERAPI int          WINAPI SetForegroundWindow(void* hWnd);
    //WINUSERAPI int          WINAPI BringWindowToTop(void* hWnd);
    //WINUSERAPI void*        WINAPI GetDC(void* hWnd);
    //WINUSERAPI int          WINAPI ReleaseDC(void* hWnd, void* hDC);
    //WINUSERAPI void*        WINAPI SetCursor(void* hCursor);
    //WINUSERAPI void*        WINAPI LoadCursorW(void* hInstance, const wchar_t* lpCursorName);
    //WINUSERAPI int          WINAPI SetWindowPlacement(void* hWnd, const WINDOWPLACEMENT* lpwndpl);
    //WINUSERAPI int          WINAPI GetWindowPlacement(void* hWnd, WINDOWPLACEMENT* lpwndpl);
    //WINUSERAPI int          WINAPI PeekMessageW(MSG* lpMsg, void* hWnd, unsigned int wMsgFilterMin, unsigned int wMsgFilterMax, unsigned int wRemoveMsg);
    //WINUSERAPI unsigned short WINAPI RegisterClassExW(const WNDCLASSEXW*);
    //WINUSERAPI void*        WINAPI CreateWindowExW(unsigned long dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, unsigned long dwStyle, int X, int Y, int nWidth, int nHeight, void* hWndParent, void* hMenu, void* hInstance, void* lpParam);
    //WINUSERAPI void*        WINAPI GetPropW(void* hWnd, const wchar_t* lpString);
    //WINUSERAPI int          WINAPI SetPropW(void* hWnd, const wchar_t* lpString, void* hData);
    //WINUSERAPI intptr_t     WINAPI DefWindowProcW(void* hWnd, unsigned int Msg, uintptr_t wParam, intptr_t lParam);
    //WINUSERAPI int          WINAPI TranslateMessage(const MSG* lpMsg);
    //WINUSERAPI intptr_t     WINAPI DispatchMessageW(const MSG* lpMsg);
    //WINUSERAPI short        WINAPI GetKeyState(int nVirtKey);
    //WINUSERAPI long         WINAPI GetMessageTime(void);
    //WINUSERAPI void*        WINAPI SetCapture(void* hWnd);
    //WINUSERAPI int          WINAPI ReleaseCapture(void);
    //WINUSERAPI int          WINAPI TrackMouseEvent(TRACKMOUSEEVENT* lpEventTrack);
    //WINUSERAPI void*        WINAPI MonitorFromWindow(void* hwnd, unsigned long dwFlags);
    //WINUSERAPI int          WINAPI GetMonitorInfoW(void* hMonitor, MONITORINFO* lpmi);
    //WINUSERAPI long         WINAPI SetWindowLongW(void* hWnd, int nIndex, long dwNewLong);
    //WINUSERAPI long         WINAPI GetWindowLongW(void* hWnd, int nIndex);
    //WINUSERAPI void*        WINAPI GetActiveWindow(void);
    //WINUSERAPI int          WINAPI EnumDisplaySettingsW(const wchar_t* lpszDeviceName, unsigned long iModeNum, DEVMODEW* lpDevMode);
    //WINUSERAPI int          WINAPI EnumDisplaySettingsExW(const wchar_t* lpszDeviceName, unsigned long iModeNum, DEVMODEW* lpDevMode, unsigned long dwFlags);
    //WINUSERAPI int          WINAPI EnumDisplayMonitors(void* hdc, const RECT* lprcClip, MONITORENUMPROC lpfnEnum, intptr_t dwData);
    //WINUSERAPI int          WINAPI EnumDisplayDevicesW(const wchar_t* lpDevice, unsigned long iDevNum, DISPLAY_DEVICEW* lpDisplayDevice, unsigned long dwFlags);
    //WINUSERAPI void*        WINAPI CreateIconIndirect(ICONINFO* piconinfo);
    //WINUSERAPI int          WINAPI GetClientRect(void* hWnd, RECT* lpRect);
    //WINUSERAPI int          WINAPI ClientToScreen(void* hWnd, POINT* lpPoint);
    //WINUSERAPI int          WINAPI ClipCursor(const RECT* lpRect);
    //WINUSERAPI int          WINAPI GetCursorPos(POINT* lpPoint);
    //WINUSERAPI int          WINAPI SetCursorPos(int X, int Y);
    //WINUSERAPI intptr_t     WINAPI SendMessageW(void* hWnd, unsigned int Msg, uintptr_t wParam, intptr_t lParam);
    //WINUSERAPI int          WINAPI AdjustWindowRectEx(RECT* lpRect, unsigned long dwStyle, int bMenu, unsigned long dwExStyle);
    //WINUSERAPI int          WINAPI SetWindowPos(void* hWnd, void* hWndInsertAfter, int X, int Y, int cx, int cy, unsigned int uFlags);
    //WINUSERAPI int          WINAPI OffsetRect(RECT* lprc, int dx, int dy);
    //WINUSERAPI void*        WINAPI RegisterDeviceNotificationW(void* hRecipient, void* NotificationFilter, unsigned long Flags);
    
    //WINGDIAPI int           WINAPI DescribePixelFormat(void* hdc, int iPixelFormat, unsigned int nBytes, PIXELFORMATDESCRIPTOR* ppfd);
    //WINGDIAPI int           WINAPI SetPixelFormat(void* hdc, int format, const PIXELFORMATDESCRIPTOR* ppfd);
    //WINGDIAPI int           WINAPI ChoosePixelFormat(void* hdc, const PIXELFORMATDESCRIPTOR* ppfd);
    //WINGDIAPI void*         WINAPI CreateDCW(const wchar_t* pwszDriver, const wchar_t* pwszDevice, const wchar_t* pszPort, const DEVMODEW* pdm);
    //WINGDIAPI int           WINAPI GetDeviceCaps(void* hdc, int nIndex);
    //WINGDIAPI int           WINAPI DeleteDC(void* hdc);
    //WINGDIAPI void*         WINAPI CreateBitmap(int nWidth, int nHeight, unsigned int nPlanes, unsigned int nBitCount, const void* lpBits);
    //WINGDIAPI void*         WINAPI CreateDIBSection(void* hdc, const BITMAPINFO* pbmi, unsigned int usage, void** ppvBits, void* hSection, unsigned long offset);
    //WINGDIAPI int           WINAPI DeleteObject(void* hObject);

    //__declspec(dllimport) void* __stdcall CreateFileMapping(void* hFile, void* lpFileMappingAttributes, unsigned long flProtect, unsigned long dwMaximumSizeHigh, unsigned long dwMaximumSizeLow, const char* lpName);
    //__declspec(dllimport) void* __stdcall wglGetCurrentDC(void);
    //__declspec(dllimport) HGLRC __stdcall wglGetCurrentContext(void);
    //__declspec(dllimport) int __stdcall wglMakeCurrent(void* hdc, HGLRC hglrc);
    //__declspec(dllimport) int __stdcall SetFilePointerEx(void* hFile, LARGE_INTEGER liDistanceToMove, LARGE_INTEGER* lpNewFilePointer, unsigned long dwMoveMethod);
    //__declspec(dllimport) unsigned int __stdcall MapVirtualKeyW(unsigned int uCode, unsigned int uMapType);
    //int __cdecl wcscmp(const wchar_t* str1, const wchar_t* str2);
    //wchar_t* __cdecl wcscpy(wchar_t* dest, const wchar_t* src);
    //__declspec(dllimport) int __stdcall SwapBuffers(void* hdc);
    //__declspec(dllimport) PROC __stdcall wglGetProcAddress(const char* lpszProc);

    int __cdecl _mkdir(const char* dirname);
    #define XINPUT_GAMEPAD_DPAD_UP          0x0001
    #define XINPUT_GAMEPAD_DPAD_DOWN        0x0002
    #define XINPUT_GAMEPAD_DPAD_LEFT        0x0004
    #define XINPUT_GAMEPAD_DPAD_RIGHT       0x0008
    #define XINPUT_GAMEPAD_START            0x0010
    #define XINPUT_GAMEPAD_BACK             0x0020
    #define XINPUT_GAMEPAD_LEFT_THUMB       0x0040
    #define XINPUT_GAMEPAD_RIGHT_THUMB      0x0080
    #define XINPUT_GAMEPAD_LEFT_SHOULDER    0x0100
    #define XINPUT_GAMEPAD_RIGHT_SHOULDER   0x0200
    #define XINPUT_GAMEPAD_A                0x1000
    #define XINPUT_GAMEPAD_B                0x2000
    #define XINPUT_GAMEPAD_X                0x4000
    #define XINPUT_GAMEPAD_Y                0x8000
    typedef struct { WORD wButtons; BYTE bLeftTrigger; BYTE bRightTrigger; SHORT sThumbLX; SHORT sThumbLY; SHORT sThumbRX; SHORT sThumbRY; } XINPUT_GAMEPAD;
    typedef struct { WORD wLeftMotorSpeed; WORD wRightMotorSpeed; } XINPUT_VIBRATION;
    typedef struct { BYTE Type; BYTE SubType; WORD Flags; XINPUT_GAMEPAD Gamepad; XINPUT_VIBRATION Vibration; } XINPUT_CAPABILITIES;
    typedef struct { DWORD dwPacketNumber; XINPUT_GAMEPAD Gamepad; } XINPUT_STATE;
    #define DBT_DEVICEARRIVAL          0x8000
    #define DBT_DEVICEREMOVECOMPLETE   0x8004
    #define DBT_DEVTYP_DEVICEINTERFACE 0x0005
    #define DEVICE_NOTIFY_WINDOW_HANDLE 0x00000000
    typedef struct { unsigned long dbch_size,dbch_devicetype,dbch_reserved; } DEV_BROADCAST_HDR;
    typedef struct { unsigned long dbcc_size,dbcc_devicetype,dbcc_reserved; GUID dbcc_classguid; wchar_t dbcc_name[1]; } DEV_BROADCAST_DEVICEINTERFACE_W;
    //__declspec(dllimport) void* __stdcall RegisterDeviceNotificationW(void* hRecipient, void* NotificationFilter, unsigned long Flags);
    #define eRender 0
    #define eConsole 0
    typedef struct IMMDevice IMMDevice; typedef struct IMMDeviceEnumerator IMMDeviceEnumerator;
    typedef struct{HRESULT(__stdcall*q)(void*,const void*,void**);ULONG(__stdcall*a)(void*);ULONG(__stdcall*Release)(void*);HRESULT(__stdcall*Activate)(void*,const void*,DWORD,void*,void**);}IMMDeviceVtbl;struct IMMDevice{IMMDeviceVtbl*lpVtbl;};
    typedef struct{HRESULT(__stdcall*q)(void*,const void*,void**);ULONG(__stdcall*a)(void*);ULONG(__stdcall*Release)(void*);HRESULT(__stdcall*e)(void*,int,DWORD,void**);HRESULT(__stdcall*GetDefaultAudioEndpoint)(void*,int,int,IMMDevice**);}IMMDeviceEnumeratorVtbl;struct IMMDeviceEnumerator{IMMDeviceEnumeratorVtbl*lpVtbl;};
    typedef long long REFERENCE_TIME;
    typedef struct IAudioClient IAudioClient;
    typedef struct IAudioRenderClient IAudioRenderClient;
    typedef struct { WORD t, n; DWORD s, a; WORD b, w, c; } WAVEFORMATEX;
    typedef struct IAudioClientVtbl { HRESULT (__stdcall *QueryInterface)(void*, const void*, void**); ULONG (__stdcall *AddRef)(void*); ULONG (__stdcall *Release)(void*); HRESULT (__stdcall *Initialize)(void*, int, DWORD, REFERENCE_TIME, REFERENCE_TIME, const WAVEFORMATEX*, const void*); 
        HRESULT (__stdcall *GetBufferSize)(void*, DWORD*); HRESULT (__stdcall *GetStreamLength)(void*, REFERENCE_TIME*); HRESULT (__stdcall *GetCurrentPadding)(void*, DWORD*); HRESULT (__stdcall *IsFormatSupported)(void*, int, const WAVEFORMATEX*, WAVEFORMATEX**); 
        HRESULT (__stdcall *GetMixFormat)(void*, WAVEFORMATEX**); HRESULT (__stdcall *GetDevicePeriod)(void*, REFERENCE_TIME*, REFERENCE_TIME*); HRESULT (__stdcall *Start)(void*); HRESULT (__stdcall *Stop)(void*); HRESULT (__stdcall *Reset)(void*); HRESULT (__stdcall *SetEventHandle)(void*, void*);
        HRESULT (__stdcall *GetService)(void*, const void*, void**);
    } IAudioClientVtbl;
    struct IAudioClient { IAudioClientVtbl* lpVtbl; };
    typedef struct IAudioRenderClientVtbl { HRESULT (__stdcall *QueryInterface)(void*, const void*, void**); ULONG (__stdcall *AddRef)(void*); ULONG (__stdcall *Release)(void*); HRESULT (__stdcall *GetBuffer)(void*, DWORD, BYTE**); HRESULT (__stdcall *ReleaseBuffer)(void*, DWORD, DWORD); } IAudioRenderClientVtbl;
    struct IAudioRenderClient { IAudioRenderClientVtbl* lpVtbl; };
    __declspec(dllimport) HRESULT __stdcall CoInitializeEx(void*, DWORD);
    __declspec(dllimport) HRESULT __stdcall CoCreateInstance(const void*, void*, DWORD, const void*, void**);
    typedef HANDLE OsFileHandle;
    #define OS_INVALID_HANDLE INVALID_HANDLE_VALUE
    #undef near
    #undef far
    static inline __attribute__((always_inline, noreturn)) void OS_Exit(i64 exitCode) { ExitProcess((unsigned int)exitCode); __builtin_unreachable(); }
    static inline __attribute__((always_inline)) long OS_Open(const char* path, i32 flags, i32 m) { (void)m; void* h = CreateFileA(path,flags ? 0x40000000L : 0x80000000L, flags ? 0 : 1,0,flags ? 2 : 3,128,0); return h==(void*)-1 ? -1 : (long)(uintptr_t)h; }
    static inline __attribute__((always_inline)) void OS_Close(OsFileHandle fileDescriptor) { CloseHandle(fileDescriptor); }
    static inline __attribute__((always_inline)) void* OS_AllocateRAM(void* a,size_t l,i32 p,i32 f,OsFileHandle fd) { (void)f; if (fd==(HANDLE)-1) return VirtualAlloc(a,l,0x3000,(p&2)?4:2); HANDLE m = CreateFileMapping(fd,NULL,(p&2) ? 4 : 2,(DWORD)(l>>32),(DWORD)l,NULL); void* r=MapViewOfFileEx(m,(p&2)?2:4,0,0,l,a); return CloseHandle(m),r;}    
    #define OS_MakeFolder(path) _mkdir(path)
    static inline __attribute__((always_inline)) long OS_Read(OsFileHandle fd, void* buf, size_t count) { DWORD bytesRead = 0; return (ReadFile((HANDLE)fd,buf,(DWORD)count,&bytesRead,NULL)) ? (long)bytesRead : (long)-1; }
    static inline __attribute__((always_inline)) OsFileHandle OS_OpenReadonly(const char* path) { HANDLE f = CreateFileA(path,0x80000000L,1,NULL,3,0x08000080,NULL); return f == (HANDLE)-1 ? DualLogError("Could not open file %s for reading\n",path), (HANDLE)-1 : f; }
    static inline __attribute__((always_inline)) OsFileHandle OS_OpenWriteonly(const char* path) { OsFileHandle h = CreateFileA(path,0x40000000L,0,NULL,2,128,NULL); return h == (HANDLE)-1 ? DualLogError("Failed to open %s for writing\n",path),(HANDLE)-1 : h; }
    static inline __attribute__((always_inline)) int OS_FileSize(OsFileHandle f) { LARGE_INTEGER s; return (f==(OsFileHandle)-1 || !GetFileSizeEx(f,&s)) ? -1 : (int)s.QuadPart; }
    static inline __attribute__((always_inline)) void* OS_AllocateFileBackedRAMReadonly(size_t s,OsFileHandle fd, char* path) { HANDLE m; void* r; return(fd==(HANDLE)-1||!s||!(m=CreateFileMappingA(fd,NULL,2,0,0,NULL))) ? DualLogError("CreateFileMapping failed for %s\n",path),NULL : (r=MapViewOfFile(m,4,0,0,s)) ? (CloseHandle(m),r) : (DualLogError("Failed to allocate %s\n",path),CloseHandle(m),NULL);}
    static inline __attribute__((always_inline)) i64 OS_Seek(OsFileHandle fd, i64 ofs, int whence /*forth and forsooth pray tell*/) { LARGE_INTEGER l={.QuadPart=ofs},n; return SetFilePointerEx((HANDLE)fd,l,&n,whence) ? n.QuadPart : -1; }
    static inline __attribute__((always_inline)) i64 OS_Tell(OsFileHandle fd) { LARGE_INTEGER l={0},n; return SetFilePointerEx((HANDLE)fd,l,&n,1) ? n.QuadPart : -1; }
    static inline __attribute__((always_inline)) int OS_GetNumThreads(void) { SYSTEM_INFO si; GetSystemInfo(&si); return (int)si.dwNumberOfProcessors; }
    static inline __attribute__((always_inline)) void OS_DeallocateRAM(void* p, size_t s) { (void)s; if(!p) { DualLogError("Attempting to double free!\n"); OS_Exit(1); } if(!UnmapViewOfFile(p) && !VirtualFree(p,0,0x00008000)) DualLogError("VirtualFree failed\n"); }
    static inline __attribute__((always_inline)) i64 OS_RawWrite(OsFileHandle fd, const void* buf, size_t count) { DWORD w; return WriteFile((HANDLE)fd,buf,(DWORD)count,&w,NULL) ? (i64)w : -1; }
    #define MOD_EXTENSION ".dll" // e.g. Citadel.dll
    #define OS_DlOpen(path)       LoadLibraryA(path)
    #define OS_DlSym(handle,name) GetProcAddress((handle),(name))
    static char win_err_buf[512];
#else
    #define LINUX
    #include <sys/ioctl.h>
    #include <sound/asound.h>
    void *dlopen(const char *filename, int flags); void *dlsym(void *handle, const char *symbol);
    typedef unsigned int mode_t; typedef long off_t; typedef u64 dev_t,ino_t; typedef long unsigned int nlink_t; typedef u32 uid_t,gid_t; typedef i64 blksize_t,blkcnt_t;
    struct input_id { u16 bustype,vendor,product,version;};
    struct input_absinfo {i32 value,minimum,maximum,fuzz,flat,resolution;};
    struct input_event { struct { long tv_sec,tv_usec; } time; u16 type,code; i32 value; };
    typedef int OsFileHandle;
    #define OS_INVALID_HANDLE -1
    typedef int wchar_t;
    static inline int OS_IOControl(int fd, unsigned long req, void* arg) { long r = 16; __asm__ __volatile__("syscall":"+a"(r):"D"(fd),"S"(req),"d"(arg):"rcx","r11","memory"); return (int)r; }
    static inline int OS_IOControlSimple(int fd, unsigned long request) { return OS_IOControl(fd,request,0); }
    static inline __attribute__((always_inline)) int OS_MakeFolder(const char* path) { long r = 83; __asm__ __volatile__("syscall":"+a"(r):"D"(path),"S"(0755LL):"rcx","r11","memory"); return (int)r; }
    static inline __attribute__((always_inline)) void* OS_Brk(void* addr) { register uintptr_t rax __asm__("rax") = 12; register void* rdi __asm__("rdi") = addr; __asm__ __volatile__("syscall":"+r"(rax):"r"(rdi):"rcx","r11","memory"); return (void*)rax; }
    static inline __attribute__((always_inline)) long OS_Read(long f,void*b,size_t c) { long r = 0; __asm__ __volatile__("syscall":"+a"(r):"D"(f),"S"(b),"d"(c):"rcx","r11","memory"); return r; }
    static inline __attribute__((always_inline, noreturn)) void OS_Exit(i64 exitCode) { register i64 rax __asm__("rax") = 231; register i64 rdi __asm__("rdi") = exitCode; __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi) : "rcx", "r11", "memory"); __builtin_unreachable(); }
    static inline __attribute__((always_inline)) void OS_Close(OsFileHandle fileDescriptor) { register long rax __asm__("rax") = 3; register long rdi __asm__("rdi") = fileDescriptor; __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi) : "rcx", "r11", "memory"); }
    static inline __attribute__((always_inline)) long OS_Open(const char* path, i32 flags, i32 mode) { long r = 2; __asm__ __volatile__("syscall":"+a"(r):"D"(path),"S"((long)flags),"d"((long)mode):"rcx","r11","memory"); return r; }
    static inline __attribute__((always_inline)) void* OS_AllocateRAM(void* addr, size_t len, i32 prot, i32 flags, OsFileHandle fd){ long r=9; register int r10 __asm__("r10")=flags; register int r8 __asm__("r8")=fd; register long r9 __asm__("r9")=0; __asm__ __volatile__("syscall":"+a"(r):"D"(addr),"S"(len),"d"(prot),"r"(r10),"r"(r8),"r"(r9):"rcx","r11","memory"); return (void*)r; }
    static inline __attribute__((always_inline)) OsFileHandle OS_OpenReadonly(const char* path) { OsFileHandle f=OS_Open(path,0,0); return f < 0 ? DualLogError("Could not open file %s for reading\n",path), -1 : f; }
    static inline __attribute__((always_inline)) OsFileHandle OS_OpenWriteonly(const char* path) { OsFileHandle f=OS_Open(path,1|00000100|00001000,0644); return f < 0 ? DualLogError("Failed to open %s for writing\n",path),-1 : f; }
    static inline __attribute__((always_inline)) int OS_FileSize(OsFileHandle f) { long r=5,s[18]; __asm__ __volatile__("syscall":"+a"(r):"D"(f),"S"(s):"rcx","r11","memory"); return (int)s[6]; }
    static inline __attribute__((always_inline)) void* OS_AllocateFileBackedRAMReadonly(size_t s, OsFileHandle fd, char* path) { void* r=OS_AllocateRAM(NULL,s,1,2,fd); return r==(void*)-1 ? DualLogError("Failed to allocate %s\n",path),NULL : r; }
    static inline __attribute__((always_inline)) i64 OS_Seek(OsFileHandle fd, i64 ofs, int whence /* forth and forsooth pray tell*/) { i64 r = 8; __asm__ __volatile__("syscall":"+a"(r):"D"(fd),"S"(ofs),"d"(whence):"rcx","r11","memory"); return r; }
    static inline __attribute__((always_inline)) i64 OS_Tell(OsFileHandle fd) { i64 r=8; __asm__ __volatile__("syscall":"+a"(r):"D"(fd),"S"(0LL),"d"(1):"rcx","r11","memory"); return r; }
    static inline __attribute__((always_inline)) int OS_GetNumThreads(void) { unsigned long m[16]; long r=204; __asm__ __volatile__("syscall":"+a"(r):"D"(0LL),"S"(128LL),"d"(m):"rcx","r11","memory"); int c = 0; for(int i=0;i<(r/8);i++) {c+=__builtin_popcountll(m[i]);} return r < 0 ? 1 : c; }
    static inline __attribute__((always_inline)) void OS_DeallocateRAM(void* p,size_t s){ long r=11; if(!p || p == (void*)-1) { DualLogError("Attempting to double free!\n"); OS_Exit(1); } __asm__ __volatile__("syscall":"+a"(r):"D"(p),"S"(s):"rcx","r11","memory"); if(r<0) DualLogError("munmap failed\n"); }
    static inline __attribute__((always_inline)) i64 OS_RawWrite(OsFileHandle fd, const void* buf, size_t cnt) { i64 r=1; __asm__ __volatile__("syscall":"+a"(r):"D"(fd),"S"(buf),"d"(cnt):"rcx","r11","memory"); return r; }
    #define MOD_EXTENSION ".so" // e.g. Citadel.so
    #define OS_DlOpen(path)       dlopen((path),2)
    #define OS_DlSym(handle,name) dlsym((handle),(name))
#endif
static inline __attribute__((always_inline)) void* OS_Alloc(size_t amount) { return OS_AllocateRAM(NULL,amount,0x1|0x2,0x02|0x20,OS_INVALID_HANDLE); }
static inline __attribute__((always_inline)) void* OS_Calloc(size_t amount, size_t count) { return OS_AllocateRAM(NULL,amount * count,0x1|0x2,0x02|0x20,OS_INVALID_HANDLE); }
static inline __attribute__((always_inline)) void OS_Write(OsFileHandle f,const void* buf, size_t s, const char* p) { size_t total=0; while(total < s) { i64 w=OS_RawWrite(f,(const char*)buf + total,s - total); if(w < 0) { DualLogError("Write error to %s: %s[%d]\n",p,w,(i32)-w); OS_Exit(1); } total += (size_t)w; } }
static inline __attribute__((always_inline)) void* OS_OpenAndAllocateFileBufferReadonly(const char* p,OsFileHandle* f,int* s){void* r;return((*f=OS_OpenReadonly(p))==(OsFileHandle)-1)?*s=0,(void*)0:((*s=OS_FileSize(*f))<=0)?DualLogError("Skipping empty:%s\n",p),OS_Close(*f),OS_Exit(1),NULL:(r=OS_AllocateFileBackedRAMReadonly(*s,*f,(char*)p))?(OS_Close(*f),r):NULL;}
static inline __attribute__((always_inline)) void* OSCopyMemoryFromBtoAForNBytes(void *dst, const void *src, size_t n) { unsigned char *d=(unsigned char *)dst; const unsigned char *s=(const unsigned char *)src; while (n--) {*d++=*s++;} return dst; } // memcpy replacement
static inline __attribute__((always_inline)) void* OS_Realloc(void* old, size_t olds, size_t news) { void* n; return !old ? OS_Alloc(news) : news <= olds ? old : (n=OS_Alloc(news)) ? (OSCopyMemoryFromBtoAForNBytes(n,old,olds),OS_DeallocateRAM(old,olds),n) : 0; }
