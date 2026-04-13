#include "internal.h"
#include <string.h>
#include <stdlib.h>
#define _GLFW_CONCAT_VERSION(m, n, r) #m "." #n "." #r
#define _GLFW_MAKE_VERSION(m, n, r) _GLFW_CONCAT_VERSION(m, n, r)
static const struct { int ID; GLFWbool (*connect)(int,_GLFWplatform*); } supportedPlatforms[] = {
#if defined(_GLFW_WIN32)
    { GLFW_PLATFORM_WIN32, _glfwConnectWin32 },
#endif
#if defined(_GLFW_X11)
    { GLFW_PLATFORM_X11, _glfwConnectX11 },
#endif
};

GLFWbool _glfwSelectPlatform(int desiredID, _GLFWplatform* platform) {
    const size_t count = sizeof(supportedPlatforms) / sizeof(supportedPlatforms[0]);
    size_t i;
    if (desiredID != GLFW_ANY_PLATFORM && desiredID != GLFW_PLATFORM_WIN32 && desiredID != GLFW_PLATFORM_X11 && desiredID != GLFW_PLATFORM_NULL) {
        _glfwInputError(GLFW_INVALID_ENUM, "Invalid platform ID 0x%08X", desiredID);
        return GLFW_FALSE;
    }

    // Only allow the Null platform if specifically requested
    if (count == 0) {
        _glfwInputError(GLFW_PLATFORM_UNAVAILABLE, "This binary only supports the Null platform");
        return GLFW_FALSE;
    }

// #if defined(_GLFW_WAYLAND) && defined(_GLFW_X11)
//     if (desiredID == GLFW_ANY_PLATFORM) {
//         const char* const session = getenv("XDG_SESSION_TYPE");
//         if (session) {
//             // Only follow XDG_SESSION_TYPE if it is set correctly and the
//             // environment looks plausble; otherwise fall back to detection
//             if (strcmp(session, "wayland") == 0 && getenv("WAYLAND_DISPLAY")) desiredID = GLFW_PLATFORM_WAYLAND;
//             else if (strcmp(session, "x11") == 0 && getenv("DISPLAY")) desiredID = GLFW_PLATFORM_X11;
//         }
//     }
// #endif

    if (desiredID == GLFW_ANY_PLATFORM) {
        // If there is exactly one platform available for auto-selection, let it emit the
        // error on failure as the platform-specific error description may be more helpful
        if (count == 1) return supportedPlatforms[0].connect(supportedPlatforms[0].ID, platform);

        for (i = 0;  i < count;  i++) {
            if (supportedPlatforms[i].connect(desiredID, platform)) return GLFW_TRUE;
        }

        _glfwInputError(GLFW_PLATFORM_UNAVAILABLE, "Failed to detect any supported platform");
    } else {
        for (i = 0;  i < count;  i++) {
            if (supportedPlatforms[i].ID == desiredID) return supportedPlatforms[i].connect(desiredID, platform);
        }

        _glfwInputError(GLFW_PLATFORM_UNAVAILABLE, "The requested platform is not supported");
    }

    return GLFW_FALSE;
}
