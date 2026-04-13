// GLFW 3.5 This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
#if defined(_GLFW_WIN32)
 #include "win32_platform.h"
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

 #define GLFW_COCOA_WINDOW_STATE
 #define GLFW_COCOA_MONITOR_STATE
 #define GLFW_COCOA_CURSOR_STATE
 #define GLFW_COCOA_LIBRARY_WINDOW_STATE
 #define GLFW_NSGL_CONTEXT_STATE
 #define GLFW_NSGL_LIBRARY_CONTEXT_STATE

#if defined(_GLFW_WAYLAND)
 #include "wl_platform.h"
 #define GLFW_EXPOSE_NATIVE_WAYLAND
#else
 #define GLFW_WAYLAND_WINDOW_STATE
 #define GLFW_WAYLAND_MONITOR_STATE
 #define GLFW_WAYLAND_CURSOR_STATE
 #define GLFW_WAYLAND_LIBRARY_WINDOW_STATE
#endif

#if defined(_GLFW_X11)
    #include "x11.h"
    #define GLFW_EXPOSE_NATIVE_X11
    #define GLFW_EXPOSE_NATIVE_GLX
    #include <linux/input.h>
    #include <linux/limits.h>
    #include <regex.h>
    #define GLFW_LINUX_JOYSTICK_STATE         _GLFWjoystickLinux linjs;
    #define GLFW_LINUX_LIBRARY_JOYSTICK_STATE _GLFWlibraryLinux  linjs;
    typedef struct _GLFWjoystickLinux {
        int fd;
        char path[PATH_MAX];
        int keyMap[KEY_CNT - BTN_MISC],absMap[ABS_CNT];
        struct input_absinfo  absInfo[ABS_CNT];
        int hats[4][2];
    } _GLFWjoystickLinux;
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


#if defined(_GLFW_WIN32)
 #include "win32_joystick.h"
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

#define GLFW_PLATFORM_JOYSTICK_STATE \
        GLFW_WIN32_JOYSTICK_STATE \
        GLFW_LINUX_JOYSTICK_STATE

#define GLFW_PLATFORM_LIBRARY_WINDOW_STATE \
        GLFW_WIN32_LIBRARY_WINDOW_STATE \
        GLFW_X11_LIBRARY_WINDOW_STATE

#define GLFW_PLATFORM_LIBRARY_JOYSTICK_STATE \
        GLFW_WIN32_LIBRARY_JOYSTICK_STATE \
        GLFW_LINUX_LIBRARY_JOYSTICK_STATE

#define GLFW_PLATFORM_CONTEXT_STATE \
        GLFW_WGL_CONTEXT_STATE \
        GLFW_NSGL_CONTEXT_STATE \
        GLFW_GLX_CONTEXT_STATE

#define GLFW_PLATFORM_LIBRARY_CONTEXT_STATE \
        GLFW_WGL_LIBRARY_CONTEXT_STATE \
        GLFW_NSGL_LIBRARY_CONTEXT_STATE \
        GLFW_GLX_LIBRARY_CONTEXT_STATE

#if defined(_WIN32)
 #define GLFW_BUILD_WIN32_THREAD
#else
 #define GLFW_BUILD_POSIX_THREAD
#endif

#if defined(GLFW_BUILD_WIN32_THREAD)
 #include "win32_thread.h"
 #define GLFW_PLATFORM_TLS_STATE    GLFW_WIN32_TLS_STATE
 #define GLFW_PLATFORM_MUTEX_STATE  GLFW_WIN32_MUTEX_STATE
#elif defined(GLFW_BUILD_POSIX_THREAD)
    #include <pthread.h>
    #define GLFW_POSIX_TLS_STATE _GLFWtlsPOSIX   posix;
    #define GLFW_POSIX_MUTEX_STATE _GLFWmutexPOSIX posix;
    typedef struct _GLFWtlsPOSIX { GLFWbool allocated; pthread_key_t key; } _GLFWtlsPOSIX;
    typedef struct _GLFWmutexPOSIX { GLFWbool allocated; pthread_mutex_t handle; } _GLFWmutexPOSIX;
 #define GLFW_PLATFORM_TLS_STATE    GLFW_POSIX_TLS_STATE
 #define GLFW_PLATFORM_MUTEX_STATE  GLFW_POSIX_MUTEX_STATE
#endif

#if defined(_WIN32)
 #define GLFW_BUILD_WIN32_TIMER
#else
 #define GLFW_BUILD_POSIX_TIMER
#endif

#if defined(GLFW_BUILD_WIN32_TIMER)
 #include "win32_time.h"
 #define GLFW_PLATFORM_LIBRARY_TIMER_STATE  GLFW_WIN32_LIBRARY_TIMER_STATE
#elif defined(GLFW_BUILD_POSIX_TIMER)
    #define GLFW_POSIX_LIBRARY_TIMER_STATE _GLFWtimerPOSIX posix;
    #include <stdint.h>
    #include <time.h>
    typedef struct _GLFWtimerPOSIX { clockid_t clock; uint64_t frequency; } _GLFWtimerPOSIX;
 #define GLFW_PLATFORM_LIBRARY_TIMER_STATE  GLFW_POSIX_LIBRARY_TIMER_STATE
#endif

#if defined(_WIN32)
 #define GLFW_BUILD_WIN32_MODULE
#else
 #define GLFW_BUILD_POSIX_MODULE
#endif

#if defined(_GLFW_WAYLAND) || defined(_GLFW_X11)
 #define GLFW_BUILD_POSIX_POLL
#endif

