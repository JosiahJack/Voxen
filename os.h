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
#if defined(_WIN32) || defined(_WIN64)
    #define WINDOWS
    #define __LONG32 long
    #define __int64 long long
    typedef unsigned int ULONG;
    typedef ULONG *PULONG;
    typedef unsigned short USHORT;
    typedef USHORT *PUSHORT;
    typedef unsigned char UCHAR;
    typedef UCHAR *PUCHAR;
    typedef char *PSZ;
    typedef unsigned short wchar_t;
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
    #define __MSABI_LONG(x) x
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
    typedef unsigned int UINT32,*PUINT32;
    typedef __int64 INT_PTR,*PINT_PTR;
    typedef unsigned __int64 UINT_PTR,*PUINT_PTR;
    typedef __int64 LONG_PTR,*PLONG_PTR;
    typedef unsigned __int64 ULONG_PTR,*PULONG_PTR;
    typedef ULONG_PTR DWORD_PTR,*PDWORD_PTR;
    typedef ULONG_PTR SIZE_T,*PSIZE_T;
    #define DECLSPEC_IMPORT __declspec (dllimport)
    #define DECLSPEC_NORETURN __declspec (noreturn)
    #define NTSYSAPI DECLSPEC_IMPORT
    #define NTAPI __stdcall
    #define MAKEINTRESOURCE(r) ((ULONG_PTR) (USHORT) r)
    #define VER_MINORVERSION 0x0000001
    #define VER_MAJORVERSION 0x0000002
    #define VER_SERVICEPACKMAJOR 0x0000020
    #define VER_GREATER_EQUAL 3
    #define VOID void
    typedef wchar_t WCHAR;
    typedef char CHAR;
    typedef short SHORT;
    typedef __LONG32 LONG;
    typedef void *HANDLE;
    typedef void *PVOID;
    typedef CHAR *NPSTR,*LPSTR,*PSTR;
    typedef CONST CHAR *LPCSTR,*PCSTR;
    typedef CONST WCHAR *LPCWSTR,*PCWSTR;
    typedef WCHAR *NWPSTR,*LPWSTR,*PWSTR;
    typedef CONST WCHAR *LPCWCH,*PCWCH;
    typedef CONST CHAR *LPCCH,*PCCH;
    typedef LPWSTR PTSTR,LPTSTR;
    typedef struct _GUID { unsigned __LONG32 Data1; unsigned short Data2,Data3; unsigned char Data4[8]; } GUID; typedef GUID IID,CLSID;
    typedef __int64 LONGLONG;
    typedef unsigned __int64 ULONGLONG;
    typedef LONG HRESULT;
    #if defined (__WIDL__)
    typedef struct _LARGE_INTEGER {
    #else
        typedef union _LARGE_INTEGER {
        struct { DWORD LowPart; LONG HighPart; } DUMMYSTRUCTNAME;
        struct { DWORD LowPart; LONG HighPart; } u;
    #endif
        LONGLONG QuadPart;
    } LARGE_INTEGER;
    typedef LARGE_INTEGER *PLARGE_INTEGER;
    #define DECLARE_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name
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
    typedef INT_PTR (WINAPI *FARPROC)(void);
    typedef INT_PTR (WINAPI *NEARPROC)(void);
    typedef INT_PTR (WINAPI *PROC)(void);
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
    typedef struct _OSVERSIONINFOEXW { DWORD dwOSVersionInfoSize; DWORD dwMajorVersion; DWORD dwMinorVersion; DWORD dwBuildNumber; DWORD dwPlatformId; WCHAR szCSDVersion[128]; WORD wServicePackMajor; WORD wServicePackMinor; WORD wSuiteMask; BYTE wProductType; BYTE wReserved; } OSVERSIONINFOEXW,*POSVERSIONINFOEXW,*LPOSVERSIONINFOEXW,RTL_OSVERSIONINFOEXW,*PRTL_OSVERSIONINFOEXW;
    DECLARE_HANDLE(DPI_AWARENESS_CONTEXT);
    #define WINBASEAPI DECLSPEC_IMPORT
    #define WINUSERAPI DECLSPEC_IMPORT
    #define RtlZeroMemory(Destination,Length) MemSetToValueForNBytes((Destination),0,(Length))
    #define ZeroMemory RtlZeroMemory
    typedef struct _SECURITY_ATTRIBUTES { DWORD nLength; LPVOID lpSecurityDescriptor; WINBOOL bInheritHandle; } SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;
    typedef struct _OVERLAPPED { ULONG_PTR Internal; ULONG_PTR InternalHigh; union {struct {DWORD Offset; DWORD OffsetHigh;} DUMMYSTRUCTNAME; PVOID Pointer;} DUMMYUNIONNAME; HANDLE hEvent; } OVERLAPPED, *LPOVERLAPPED;
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
    typedef struct _SYSTEM_INFO { union { DWORD dwOemId; struct { WORD wProcessorArchitecture; WORD wReserved; } DUMMYSTRUCTNAME; } DUMMYUNIONNAME; DWORD dwPageSize; LPVOID lpMinimumApplicationAddress; LPVOID lpMaximumApplicationAddress; DWORD_PTR dwActiveProcessorMask; DWORD dwNumberOfProcessors; DWORD dwProcessorType; DWORD dwAllocationGranularity; WORD wProcessorLevel; WORD wProcessorRevision; } SYSTEM_INFO, *LPSYSTEM_INFO;
    WINBASEAPI VOID WINAPI GetSystemInfo (LPSYSTEM_INFO lpSystemInfo);
    int __cdecl wcscmp(const wchar_t *_Str1,const wchar_t *_Str2);
    wchar_t* wcscpy(wchar_t* restrict destination, const wchar_t* restrict source);
    #define MAKEINTATOM(i) (LPTSTR) ((ULONG_PTR)((WORD)(i)))
    #define ERROR_SUCCESS __MSABI_LONG(0)
    #define ERROR_DEVICE_NOT_CONNECTED __MSABI_LONG(1167)
    #define SUCCEEDED(hr) ((HRESULT)(hr) >= 0)
    #define FAILED(hr) ((HRESULT)(hr) < 0)
    WINBASEAPI HANDLE WINAPI CreateFileMappingA (HANDLE hFile, LPSECURITY_ATTRIBUTES lpFileMappingAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCSTR lpName);
    #define PFD_DOUBLEBUFFER 0x00000001
    #define PFD_DRAW_TO_WINDOW 0x00000004
    #define PFD_SUPPORT_OPENGL 0x00000020
    #define PFD_TYPE_RGBA 0
    #define DISPLAY_DEVICE_ACTIVE 0x00000001
    #define DISPLAY_DEVICE_PRIMARY_DEVICE 0x00000004
    #define DISPLAY_DEVICE_MODESPRUNED 0x08000000
    #define ENUM_CURRENT_SETTINGS ((DWORD)-1)
    #define CCHDEVICENAME 32
    #define CCHFORMNAME 32
    #define BI_BITFIELDS __MSABI_LONG(3)
    #define DIB_RGB_COLORS 0
    #define LOGPIXELSX 88
    #define LOGPIXELSY 90
    #define HORZSIZE 4
    #define VERTSIZE 6
    typedef __LONG32 FXPT2DOT30,*LPFXPT2DOT30;
    typedef struct tagCIEXYZ { FXPT2DOT30 ciexyzX; FXPT2DOT30 ciexyzY; FXPT2DOT30 ciexyzZ; } CIEXYZ;
    typedef struct tagICEXYZTRIPLE { CIEXYZ ciexyzRed; CIEXYZ ciexyzGreen; CIEXYZ ciexyzBlue; } CIEXYZTRIPLE;
    typedef struct _DISPLAY_DEVICEW { DWORD cb; WCHAR DeviceName[32]; WCHAR DeviceString[128]; DWORD StateFlags; WCHAR DeviceID[128]; WCHAR DeviceKey[128]; } DISPLAY_DEVICEW,*PDISPLAY_DEVICEW,*LPDISPLAY_DEVICEW;
    typedef struct tagPIXELFORMATDESCRIPTOR {
        WORD nSize; WORD nVersion; DWORD dwFlags; BYTE iPixelType; BYTE cColorBits; BYTE cRedBits; BYTE cRedShift; BYTE cGreenBits; BYTE cGreenShift; BYTE cBlueBits; BYTE cBlueShift;
        BYTE cAlphaBits; BYTE cAlphaShift; BYTE cAccumBits; BYTE cAccumRedBits; BYTE cAccumGreenBits; BYTE cAccumBlueBits; BYTE cAccumAlphaBits; BYTE cDepthBits; BYTE cStencilBits;
        BYTE cAuxBuffers; BYTE iLayerType; BYTE bReserved; DWORD dwLayerMask; DWORD dwVisibleMask; DWORD dwDamageMask;
    } PIXELFORMATDESCRIPTOR,*PPIXELFORMATDESCRIPTOR,*LPPIXELFORMATDESCRIPTOR;
    typedef struct { DWORD bV5Size; LONG bV5Width; LONG bV5Height; WORD bV5Planes; WORD bV5BitCount; DWORD bV5Compression; DWORD bV5SizeImage; LONG bV5XPelsPerMeter;
        LONG bV5YPelsPerMeter; DWORD bV5ClrUsed; DWORD bV5ClrImportant; DWORD bV5RedMask; DWORD bV5GreenMask; DWORD bV5BlueMask; DWORD bV5AlphaMask; DWORD bV5CSType;
        CIEXYZTRIPLE bV5Endpoints; DWORD bV5GammaRed; DWORD bV5GammaGreen; DWORD bV5GammaBlue; DWORD bV5Intent;DWORD bV5ProfileData; DWORD bV5ProfileSize; DWORD bV5Reserved;
    } BITMAPV5HEADER,*LPBITMAPV5HEADER,*PBITMAPV5HEADER;
    typedef struct tagRGBQUAD { BYTE rgbBlue; BYTE rgbGreen; BYTE rgbRed; BYTE rgbReserved; } RGBQUAD;
    typedef struct tagBITMAPINFOHEADER { DWORD biSize; LONG biWidth; LONG biHeight; WORD biPlanes; WORD biBitCount; DWORD biCompression; DWORD biSizeImage; LONG biXPelsPerMeter; LONG biYPelsPerMeter; DWORD biClrUsed; DWORD biClrImportant; } BITMAPINFOHEADER,*LPBITMAPINFOHEADER,*PBITMAPINFOHEADER;
    typedef struct tagBITMAPINFO { BITMAPINFOHEADER bmiHeader; RGBQUAD bmiColors[1]; } BITMAPINFO,*LPBITMAPINFO,*PBITMAPINFO;
    typedef struct _devicemodeW {
        WCHAR dmDeviceName[CCHDEVICENAME]; WORD dmSpecVersion; WORD dmDriverVersion; WORD dmSize; WORD dmDriverExtra; DWORD dmFields;
        union { struct { short dmOrientation,dmPaperSize,dmPaperLength,dmPaperWidth,dmScale,dmCopies,dmDefaultSource,dmPrintQuality; }; struct { POINTL dmPosition; DWORD dmDisplayOrientation; DWORD dmDisplayFixedOutput; }; };
        short dmColor,dmDuplex,dmYResolution,dmTTOption,dmCollate; WCHAR dmFormName[CCHFORMNAME]; WORD dmLogPixels; DWORD dmBitsPerPel; DWORD dmPelsWidth; DWORD dmPelsHeight; union { DWORD dmDisplayFlags; DWORD dmNup; };
        DWORD dmDisplayFrequency; DWORD dmICMMethod; DWORD dmICMIntent; DWORD dmMediaType; DWORD dmDitherType; DWORD dmReserved1; DWORD dmReserved2; DWORD dmPanningWidth; DWORD dmPanningHeight;
    } DEVMODEW,*PDEVMODEW,*NPDEVMODEW,*LPDEVMODEW;
    #define WINGDIAPI DECLSPEC_IMPORT
    WINUSERAPI WINBOOL WINAPI EnumDisplaySettingsW(LPCWSTR lpszDeviceName,DWORD iModeNum,LPDEVMODEW lpDevMode);
    WINUSERAPI WINBOOL WINAPI EnumDisplayDevicesW(LPCWSTR lpDevice,DWORD iDevNum,PDISPLAY_DEVICEW lpDisplayDevice,DWORD dwFlags);
    WINUSERAPI WINBOOL WINAPI EnumDisplaySettingsExW(LPCWSTR lpszDeviceName,DWORD iModeNum,LPDEVMODEW lpDevMode,DWORD dwFlags);
    WINGDIAPI WINBOOL WINAPI SetPixelFormat(HDC hdc,int format,CONST PIXELFORMATDESCRIPTOR *ppfd);
    WINGDIAPI int WINAPI ChoosePixelFormat(HDC hdc,CONST PIXELFORMATDESCRIPTOR *ppfd);
    WINGDIAPI int WINAPI DescribePixelFormat(HDC hdc,int iPixelFormat,UINT nBytes,LPPIXELFORMATDESCRIPTOR ppfd);
    WINGDIAPI HBITMAP WINAPI CreateBitmap(int nWidth,int nHeight,UINT nPlanes,UINT nBitCount,CONST VOID *lpBits);
    WINGDIAPI HDC WINAPI wglGetCurrentDC(VOID);
    WINGDIAPI HGLRC WINAPI wglGetCurrentContext(VOID);
    WINGDIAPI PROC WINAPI wglGetProcAddress(LPCSTR);
    WINGDIAPI WINBOOL WINAPI wglMakeCurrent(HDC,HGLRC);
    WINGDIAPI HBITMAP WINAPI CreateDIBSection(HDC hdc,CONST BITMAPINFO *lpbmi,UINT usage,VOID **ppvBits,HANDLE hSection,DWORD offset);
    WINGDIAPI WINBOOL WINAPI DeleteObject(HGDIOBJ ho);
    WINGDIAPI WINBOOL WINAPI DeleteDC(HDC hdc);
    WINGDIAPI WINBOOL WINAPI SwapBuffers(HDC);
    WINGDIAPI int WINAPI GetDeviceCaps(HDC hdc,int index);
    WINGDIAPI HDC WINAPI CreateDCW(LPCWSTR pwszDriver,LPCWSTR pwszDevice,LPCWSTR pszPort,CONST DEVMODEW *pdm);
    #define GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT (0x2)
    #define GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS (0x4)
    WINBASEAPI FARPROC WINAPI GetProcAddress (HMODULE hModule, LPCSTR lpProcName);
    WINBASEAPI WINBOOL WINAPI GetModuleHandleExW (DWORD dwFlags, LPCWSTR lpModuleName, HMODULE *phModule);
    WINBASEAPI HMODULE WINAPI LoadLibraryA(LPCSTR lpLibFileName);
    typedef LRESULT (__stdcall *WNDPROC)(HWND, UINT, WPARAM, LPARAM);
    typedef BOOL (__stdcall *MONITORENUMPROC)(HMONITOR, HDC, LPRECT, LPARAM);
    #define WS_POPUP (0x80000000)
    #define WS_CLIPSIBLINGS (0x04000000)
    #define WS_CLIPCHILDREN (0x02000000)
    #define WS_SYSMENU (0x00080000)
    #define WS_MINIMIZEBOX (0x00020000)
    #define WS_CAPTION (0x00C00000)
    #define CP_UTF8 65001
    #define WM_SIZE 0x0005
    #define WM_MOUSEACTIVATE 0x0021
    #define WM_LBUTTONDOWN 0x0201
    #define WM_SETFOCUS 0x0007
    #define WM_KILLFOCUS 0x0008
    #define WM_SYSCOMMAND 0x0112
    #define WM_CLOSE 0x0010
    #define WM_CAPTURECHANGED 0x0215
    #define WM_KEYDOWN 0x0100
    #define WM_KEYUP 0x0101
    #define WM_SYSKEYDOWN 0x0104
    #define WM_SYSKEYUP 0x0105
    #define WM_RBUTTONDOWN 0x0204
    #define WM_MBUTTONDOWN 0x0207
    #define WM_XBUTTONDOWN 0x020B
    #define WM_LBUTTONUP 0x0202
    #define WM_RBUTTONUP 0x0205
    #define WM_XBUTTONUP 0x020C
    #define WM_MBUTTONUP 0x0208
    #define WM_MOUSEMOVE 0x0200
    #define WM_MOUSEWHEEL 0x020A
    #define WM_MOVE 0x0003
    #define WM_INPUT 0x00FF
    #define WM_ERASEBKGND 0x0014
    #define WHEEL_DELTA 120
    #define WM_MOUSELEAVE 0x02A3
    #define WM_SETCURSOR 0x0020
    #define WM_ENTERSIZEMOVE 0x0231
    #define WM_EXITSIZEMOVE 0x0232
    #define WM_ENTERMENULOOP 0x0211
    #define WM_EXITMENULOOP 0x0212
    #define WM_GETMINMAXINFO 0x0024
    #define WM_NCPAINT 0x0085
    #define WM_NCACTIVATE 0x0086
    #define WM_QUIT 0x0012
    #define WM_DISPLAYCHANGE 0x007E
    #define WM_DEVICECHANGE 0x0219
    #define XBUTTON1 0x0001
    #define SC_SCREENSAVE 0xF140
    #define SC_MONITORPOWER 0xF170
    #define SC_KEYMENU 0xF100
    #define HTCLIENT 1
    #define KF_EXTENDED 0x0100
    #define KF_UP 0x8000
    #define MAPVK_VK_TO_VSC (0)
    #define VK_CONTROL 0x11
    #define VK_MENU 0x12
    #define VK_PROCESSKEY 0xE5
    #define VK_SHIFT 0x10
    #define VK_LSHIFT 0xA0
    #define VK_RSHIFT 0xA1
    #define VK_SNAPSHOT 0x2C
    #define VK_LWIN 0x5B
    #define VK_RWIN 0x5C
    #define PM_NOREMOVE 0x0000
    #define IDC_ARROW MAKEINTRESOURCE(32512)
    #define GET_XBUTTON_WPARAM(wParam) (HIWORD(wParam))
    #define TME_LEAVE 0x00000002
    #define SIZE_MINIMIZED 1
    #define MONITOR_DEFAULTTONEAREST 0x00000002
    #define WS_EX_APPWINDOW (0x00040000)
    #define WM_SETICON 0x0080
    #define ICON_SMALL 0
    #define ICON_BIG 1
    #define PM_REMOVE 0x0001
    #define HWND_TOP ((HWND)0)
    #define HWND_NOTOPMOST ((HWND)-2)
    #define SWP_NOSIZE 0x0001
    #define SWP_NOMOVE 0x0002
    #define SWP_NOACTIVATE 0x0010
    #define SWP_NOCOPYBITS 0x0100
    #define SWP_FRAMECHANGED 0x0020
    #define SWP_NOZORDER 0x0004
    #define SWP_NOOWNERZORDER 0x0200
    #define SW_HIDE 0
    #define SW_SHOWNA 8
    #define CCHDEVICENAME 32
    #define GWL_STYLE (-16)
    #define WS_OVERLAPPED (0x00000000)
    #define WS_SYSMENU (0x00080000)
    #define WS_THICKFRAME (0x00040000)
    #define WS_MINIMIZEBOX (0x00020000)
    #define WS_MAXIMIZEBOX (0x00010000)
    #define WS_EX_WINDOWEDGE (0x00000100)
    #define WS_EX_CLIENTEDGE (0x00000200)
    #define CW_USEDEFAULT ((int)0x80000000)
    #define CS_VREDRAW 0x0001
    #define CS_HREDRAW 0x0002
    #define CS_OWNDC 0x0020
    #define WS_OVERLAPPEDWINDOW (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)
    #define WS_EX_OVERLAPPEDWINDOW (WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE)
    typedef PVOID HDEVNOTIFY;
    typedef struct _ICONINFO { WINBOOL fIcon; DWORD xHotspot; DWORD yHotspot; HBITMAP hbmMask; HBITMAP hbmColor; } ICONINFO; typedef ICONINFO *PICONINFO;
    typedef struct tagMSG { HWND hwnd; UINT message; WPARAM wParam; LPARAM lParam; DWORD time; POINT pt; } MSG,*PMSG,*NPMSG,*LPMSG;
    typedef struct tagTRACKMOUSEEVENT { DWORD cbSize; DWORD dwFlags; HWND hwndTrack; DWORD dwHoverTime; } TRACKMOUSEEVENT,*LPTRACKMOUSEEVENT;
    typedef struct tagMINMAXINFO { POINT ptReserved; POINT ptMaxSize; POINT ptMaxPosition; POINT ptMinTrackSize; POINT ptMaxTrackSize; } MINMAXINFO,*PMINMAXINFO,*LPMINMAXINFO;
    typedef struct tagMONITORINFO { DWORD cbSize; RECT rcMonitor; RECT rcWork; DWORD dwFlags; } MONITORINFO,*LPMONITORINFO;
    typedef struct tagMONITORINFOEXW { DWORD cbSize; RECT rcMonitor; RECT rcWork; DWORD dwFlags; WCHAR szDevice[CCHDEVICENAME]; } MONITORINFOEXW, *LPMONITORINFOEXW;
    typedef struct tagWINDOWPLACEMENT { UINT length; UINT flags; UINT showCmd; POINT ptMinPosition; POINT ptMaxPosition; RECT rcNormalPosition; } WINDOWPLACEMENT;
    typedef struct tagWNDCLASSEXW { UINT cbSize; UINT style; WNDPROC lpfnWndProc; int cbClsExtra; int cbWndExtra; HINSTANCE hInstance; HICON hIcon; HCURSOR hCursor; HBRUSH hbrBackground; LPCWSTR lpszMenuName; LPCWSTR lpszClassName; HICON hIconSm; } WNDCLASSEXW,*PWNDCLASSEXW,*NPWNDCLASSEXW,*LPWNDCLASSEXW;
    WINBASEAPI int WINAPI MultiByteToWideChar (UINT CodePage, DWORD dwFlags, LPCCH lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar);
    WINBASEAPI int WINAPI WideCharToMultiByte (UINT CodePage, DWORD dwFlags, LPCWCH lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, LPCCH lpDefaultChar, LPBOOL lpUsedDefaultChar);
    WINUSERAPI HDC WINAPI GetDC(HWND hWnd);
    WINUSERAPI int WINAPI ReleaseDC(HWND hWnd,HDC hDC);
    WINUSERAPI HICON WINAPI CreateIconIndirect(PICONINFO piconinfo);
    WINUSERAPI WINBOOL WINAPI SetCursorPos(int X,int Y);
    WINUSERAPI HCURSOR WINAPI SetCursor(HCURSOR hCursor);
    WINUSERAPI WINBOOL WINAPI GetCursorPos(LPPOINT lpPoint);
    WINUSERAPI WINBOOL WINAPI ClipCursor(CONST RECT *lpRect);
    WINUSERAPI WINBOOL WINAPI ClientToScreen(HWND hWnd,LPPOINT lpPoint);
    WINUSERAPI HANDLE WINAPI GetPropW(HWND hWnd,LPCWSTR lpString);
    WINUSERAPI LRESULT WINAPI DefWindowProcW (HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
    WINUSERAPI HCURSOR WINAPI LoadCursorW(HINSTANCE hInstance,LPCWSTR lpCursorName);
    WINUSERAPI WINBOOL WINAPI GetClientRect(HWND hWnd,LPRECT lpRect); // Haha get rect!
    WINUSERAPI UINT WINAPI MapVirtualKeyW(UINT uCode,UINT uMapType);
    WINUSERAPI LONG WINAPI GetMessageTime(VOID);
    WINUSERAPI WINBOOL WINAPI PeekMessageW(LPMSG lpMsg,HWND hWnd,UINT wMsgFilterMin,UINT wMsgFilterMax,UINT wRemoveMsg);
    WINUSERAPI HWND WINAPI SetCapture(HWND hWnd);
    WINUSERAPI WINBOOL WINAPI ReleaseCapture(VOID);
    WINUSERAPI WINBOOL WINAPI TrackMouseEvent(LPTRACKMOUSEEVENT lpEventTrack);
    WINUSERAPI WINBOOL WINAPI AdjustWindowRect(LPRECT lpRect,DWORD dwStyle,WINBOOL bMenu);
    WINUSERAPI WINBOOL WINAPI AdjustWindowRectEx(LPRECT lpRect,DWORD dwStyle,WINBOOL bMenu,DWORD dwExStyle);
    WINUSERAPI HMONITOR WINAPI MonitorFromWindow(HWND hwnd,DWORD dwFlags);
    WINUSERAPI WINBOOL WINAPI GetMonitorInfoW(HMONITOR hMonitor,LPMONITORINFO lpmi);
    WINUSERAPI LRESULT WINAPI SendMessageW(HWND hWnd,UINT Msg,WPARAM wParam,LPARAM lParam);
    WINUSERAPI WINBOOL WINAPI SetWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags);
    WINUSERAPI LONG WINAPI GetWindowLongW(HWND hWnd,int nIndex);
    WINUSERAPI LONG WINAPI SetWindowLongW(HWND hWnd,int nIndex,LONG dwNewLong);
    WINUSERAPI HWND WINAPI GetActiveWindow(VOID);
    WINUSERAPI WINBOOL WINAPI TranslateMessage(CONST MSG *lpMsg);
    WINUSERAPI SHORT WINAPI GetKeyState(int nVirtKey);
    WINUSERAPI LRESULT WINAPI DispatchMessageW(CONST MSG *lpMsg);
    WINUSERAPI WINBOOL WINAPI ShowWindow(HWND hWnd,int nCmdShow);
    WINUSERAPI WINBOOL WINAPI BringWindowToTop (HWND hWnd);
    WINUSERAPI WINBOOL WINAPI SetForegroundWindow(HWND hWnd);
    WINUSERAPI HWND WINAPI SetFocus(HWND hWnd);
    WINUSERAPI WINBOOL WINAPI GetWindowPlacement(HWND hWnd, WINDOWPLACEMENT *lpwndpl);
    WINUSERAPI WINBOOL WINAPI SetWindowPlacement(HWND hWnd, CONST WINDOWPLACEMENT *lpwndpl);
    WINUSERAPI HWND WINAPI CreateWindowExW(DWORD dwExStyle,LPCWSTR lpClassName,LPCWSTR lpWindowName,DWORD dwStyle,int X,int Y,int nWidth,int nHeight,HWND hWndParent,HMENU hMenu,HINSTANCE hInstance,LPVOID lpParam);
    WINUSERAPI WINBOOL WINAPI SetPropW(HWND hWnd,LPCWSTR lpString,HANDLE hData);
    WINUSERAPI WINBOOL WINAPI OffsetRect(LPRECT lprc,int dx,int dy);
    WINUSERAPI ATOM WINAPI RegisterClassExW (CONST WNDCLASSEXW *);
    WINUSERAPI HDEVNOTIFY WINAPI RegisterDeviceNotificationW(HANDLE hRecipient,LPVOID NotificationFilter,DWORD Flags);
    WINUSERAPI WINBOOL WINAPI EnumDisplayMonitors(HDC hdc,LPCRECT lprcClip,MONITORENUMPROC lpfnEnum,LPARAM dwData);
    NTSYSAPI ULONGLONG NTAPI VerSetConditionMask(ULONGLONG ConditionMask, ULONG TypeMask, UCHAR Condition);
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
    #define eRender 0
    #define eConsole 0
    typedef struct IMMDevice IMMDevice; typedef struct IMMDeviceEnumerator IMMDeviceEnumerator;
    typedef struct{HRESULT(__stdcall*q)(void*,const void*,void**);ULONG(__stdcall*a)(void*);ULONG(__stdcall*Release)(void*);HRESULT(__stdcall* Activate)(void*,const void*,DWORD,void*,void**);}IMMDeviceVtbl;struct IMMDevice{IMMDeviceVtbl*lpVtbl;};
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
    #define STDAPI extern HRESULT WINAPI
    #define WINOLEAPI STDAPI
    #define REFIID const IID *const
    typedef struct IUnknown IUnknown;
    typedef struct IUnknownVtbl { HRESULT (__stdcall *QueryInterface)(IUnknown* This, const IID* riid, void** ppvObject); ULONG   (__stdcall *AddRef)(IUnknown* This); ULONG   (__stdcall *Release)(IUnknown* This); } IUnknownVtbl;
    struct IUnknown { const IUnknownVtbl* lpVtbl; };
    WINOLEAPI CoInitializeEx(LPVOID pvReserved, DWORD dwCoInit);
    WINOLEAPI CoCreateInstance(const IID *const rclsid, IUnknown* pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv);
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
    typedef __int64	__time64_t;
    typedef __time64_t time_t;
    struct timespec { time_t tv_sec; long tv_nsec; };
    struct sched_param { int sched_priority; };
    typedef uintptr_t pthread_t;
    typedef struct pthread_attr_t { unsigned p_state; void *stack; size_t s_size; struct sched_param param; } pthread_attr_t;
    int pthread_create(pthread_t *th, const pthread_attr_t *attr, void *(* func)(void *), void *arg);
    int pthread_join(pthread_t t, void **res);
#else
    #define LINUX
    void *dlopen(const char *filename, int flags); void *dlsym(void *handle, const char *symbol);
    typedef unsigned int mode_t; typedef long off_t; typedef u64 dev_t,ino_t; typedef long unsigned int nlink_t; typedef u32 uid_t,gid_t; typedef i64 blksize_t,blkcnt_t;
    struct input_id { u16 bustype,vendor,product,version;};
    struct input_absinfo {i32 value,minimum,maximum,fuzz,flat,resolution;};
    struct input_event { struct { long tv_sec,tv_usec; } time; u16 type,code; i32 value; };
    typedef int OsFileHandle;
    #define OS_INVALID_HANDLE -1
    typedef int wchar_t;
    typedef long time_t;
    typedef unsigned long int pthread_t;
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
    struct timespec { time_t tv_sec; long tv_nsec; };
    typedef struct { unsigned int flags; void* stack; } pthread_attr_t;
    int pthread_create(pthread_t* restrict thread, const pthread_attr_t* restrict attr, void* (*start_routine)(void*), void* restrict arg);
    int pthread_join(pthread_t thread, void** retval);
#endif
static inline __attribute__((always_inline)) void* OS_Alloc(size_t amount) { return OS_AllocateRAM(NULL,amount,0x1|0x2,0x02|0x20,OS_INVALID_HANDLE); }
static inline __attribute__((always_inline)) void* OS_Calloc(size_t amount, size_t count) { return OS_AllocateRAM(NULL,amount * count,0x1|0x2,0x02|0x20,OS_INVALID_HANDLE); }
static inline __attribute__((always_inline)) void OS_Write(OsFileHandle f,const void* buf, size_t s, const char* p) { size_t total=0; while(total < s) { i64 w=OS_RawWrite(f,(const char*)buf + total,s - total); if(w < 0) { DualLogError("Write error to %s: %s[%d]\n",p,w,(i32)-w); OS_Exit(1); } total += (size_t)w; } }
static inline __attribute__((always_inline)) void* OS_OpenAndAllocateFileBufferReadonly(const char* p,OsFileHandle* f,int* s){void* r;return((*f=OS_OpenReadonly(p))==(OsFileHandle)-1)?*s=0,(void*)0:((*s=OS_FileSize(*f))<=0)?DualLogError("Skipping empty:%s\n",p),OS_Close(*f),OS_Exit(1),NULL:(r=OS_AllocateFileBackedRAMReadonly(*s,*f,(char*)p))?(OS_Close(*f),r):NULL;}
static inline __attribute__((always_inline)) void* OSCopyMemoryFromBtoAForNBytes(void *dst, const void *src, size_t n) { unsigned char *d=(unsigned char *)dst; const unsigned char *s=(const unsigned char *)src; while (n--) {*d++=*s++;} return dst; } // memcpy replacement
static inline __attribute__((always_inline)) void* OS_Realloc(void* old, size_t olds, size_t news) { void* n; return !old ? OS_Alloc(news) : news <= olds ? old : (n=OS_Alloc(news)) ? (OSCopyMemoryFromBtoAForNBytes(n,old,olds),OS_DeallocateRAM(old,olds),n) : 0; }
