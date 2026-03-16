/*************************************************************************
 * GLFW 3.5 - www.glfw.org
 * A library for OpenGL, window and input
 *------------------------------------------------------------------------
 * Copyright (c) 2002-2006 Marcus Geelnard
 * Copyright (c) 2006-2019 Camilla Löwy <elmindreda@glfw.org>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would
 *    be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not
 *    be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 *    distribution.  Hey what are you doing, stop!  Oh oops. you mean like this?
 *
 *************************************************************************/

#ifndef _glfw3_h_
#define _glfw3_h_

#if !defined(_WIN32) && (defined(__WIN32__) || defined(WIN32) || defined(__MINGW32__))
 #define _WIN32
#endif /* _WIN32 */

#if !defined(APIENTRY)
 #if defined(_WIN32)
  #define APIENTRY __stdcall
 #else
  #define APIENTRY
 #endif
 #define GLFW_APIENTRY_DEFINED
#endif /* APIENTRY */

#if !defined(WINGDIAPI) && defined(_WIN32)
 #define WINGDIAPI __declspec(dllimport)
 #define GLFW_WINGDIAPI_DEFINED
#endif /* WINGDIAPI */

#if !defined(CALLBACK) && defined(_WIN32)
 #define CALLBACK __stdcall
 #define GLFW_CALLBACK_DEFINED
#endif /* CALLBACK */

#if defined(GLFW_INCLUDE_ES1)
 #include <GLES/gl.h>
 #if defined(GLFW_INCLUDE_GLEXT)
  #include <GLES/glext.h>
 #endif
#elif defined(GLFW_INCLUDE_ES2)
 #include <GLES2/gl2.h>
 #if defined(GLFW_INCLUDE_GLEXT)
  #include <GLES2/gl2ext.h>
 #endif
#elif defined(GLFW_INCLUDE_ES3)
 #include <GLES3/gl3.h>
 #if defined(GLFW_INCLUDE_GLEXT)
  #include <GLES2/gl2ext.h>
 #endif
#elif defined(GLFW_INCLUDE_ES31)
 #include <GLES3/gl31.h>
 #if defined(GLFW_INCLUDE_GLEXT)
  #include <GLES2/gl2ext.h>
 #endif
#elif defined(GLFW_INCLUDE_ES32)
 #include <GLES3/gl32.h>
 #if defined(GLFW_INCLUDE_GLEXT)
  #include <GLES2/gl2ext.h>
 #endif
#elif defined(GLFW_INCLUDE_GLCOREARB)
 #if defined(__APPLE__)
  #include <OpenGL/gl3.h>
  #if defined(GLFW_INCLUDE_GLEXT)
   #include <OpenGL/gl3ext.h>
  #endif /*GLFW_INCLUDE_GLEXT*/
 #else /*__APPLE__*/
  #include <GL/glcorearb.h>
  #if defined(GLFW_INCLUDE_GLEXT)
   #include <GL/glext.h>
  #endif
 #endif /*__APPLE__*/
#elif defined(GLFW_INCLUDE_GLU)
  #if defined(GLFW_INCLUDE_GLU)
   #include <GL/glu.h>
  #endif
#elif !defined(GLFW_INCLUDE_NONE) && \
      !defined(__gl_h_) && \
      !defined(__gles1_gl_h_) && \
      !defined(__gles2_gl2_h_) && \
      !defined(__gles2_gl3_h_) && \
      !defined(__gles2_gl31_h_) && \
      !defined(__gles2_gl32_h_) && \
      !defined(__gl_glcorearb_h_) && \
      !defined(__gl2_h_) /*legacy*/ && \
      !defined(__gl3_h_) /*legacy*/ && \
      !defined(__gl31_h_) /*legacy*/ && \
      !defined(__gl32_h_) /*legacy*/ && \
      !defined(__glcorearb_h_) /*legacy*/ && \
      !defined(__GL_H__) /*non-standard*/ && \
      !defined(__gltypes_h_) /*non-standard*/ && \
      !defined(__glee_h_) /*non-standard*/
 #if defined(__APPLE__)
  #if !defined(GLFW_INCLUDE_GLEXT)
   #define GL_GLEXT_LEGACY
  #endif
  #include <OpenGL/gl.h>
 #else /*__APPLE__*/
  #include <GL/gl.h>
  #if defined(GLFW_INCLUDE_GLEXT)
   #include <GL/glext.h>
  #endif
 #endif /*__APPLE__*/
#endif /* OpenGL and OpenGL ES headers */

#if defined(GLFW_DLL) && defined(_GLFW_BUILD_DLL)
 #error "You must not have both GLFW_DLL and _GLFW_BUILD_DLL defined"
#endif

#if defined(_WIN32) && defined(_GLFW_BUILD_DLL)
 /* We are building GLFW as a Win32 DLL */
 #define GLFWAPI __declspec(dllexport)
#elif defined(_WIN32) && defined(GLFW_DLL)
 /* We are calling a GLFW Win32 DLL */
 #define GLFWAPI __declspec(dllimport)
#elif defined(_GLFW_BUILD_DLL)
 /* We are building GLFW as a Unix shared library */
 #define GLFWAPI __attribute__((visibility("default")))
#else
 #define GLFWAPI
#endif

#include "glfw_defines.h"

typedef void (*GLFWglproc)(void);
typedef void (*GLFWvkproc)(void);
typedef struct GLFWmonitor GLFWmonitor;
typedef struct GLFWwindow GLFWwindow;
typedef struct GLFWcursor GLFWcursor;
typedef void* (* GLFWallocatefun)(int size, void* user);
typedef void* (* GLFWreallocatefun)(void* block, int size, void* user);
typedef void (* GLFWdeallocatefun)(void* block, void* user);
typedef void (* GLFWerrorfun)(int error_code, const char* description);
typedef void (* GLFWwindowposfun)(GLFWwindow* window, int xpos, int ypos);
typedef void (* GLFWwindowsizefun)(GLFWwindow* window, int width, int height);
typedef void (* GLFWwindowclosefun)(GLFWwindow* window);
typedef void (* GLFWwindowrefreshfun)(GLFWwindow* window);
typedef void (* GLFWwindowfocusfun)(GLFWwindow* window, int focused);
typedef void (* GLFWwindowiconifyfun)(GLFWwindow* window, int iconified);
typedef void (* GLFWwindowmaximizefun)(GLFWwindow* window, int maximized);
typedef void (* GLFWframebuffersizefun)(GLFWwindow* window, int width, int height);
typedef void (* GLFWwindowcontentscalefun)(GLFWwindow* window, float xscale, float yscale);
typedef void (* GLFWmousebuttonfun)(GLFWwindow* window, int button, int action, int mods);
typedef void (* GLFWcursorposfun)(GLFWwindow* window, double xpos, double ypos);
typedef void (* GLFWcursorenterfun)(GLFWwindow* window, int entered);
typedef void (* GLFWscrollfun)(GLFWwindow* window, double xoffset, double yoffset);
typedef void (* GLFWkeyfun)(GLFWwindow* window, int key, int scancode, int action, int mods);
typedef void (* GLFWcharfun)(GLFWwindow* window, unsigned int codepoint);
typedef void (* GLFWcharmodsfun)(GLFWwindow* window, unsigned int codepoint, int mods);
typedef void (* GLFWdropfun)(GLFWwindow* window, int path_count, const char* paths[]);
typedef void (* GLFWmonitorfun)(GLFWmonitor* monitor, int event);
typedef void (* GLFWjoystickfun)(int jid, int event);

typedef struct GLFWvidmode {
    int width;
    int height;
    int redBits;
    int greenBits;
    int blueBits;
    int refreshRate;
} GLFWvidmode;

typedef struct GLFWgammaramp {
    unsigned short* red;
    unsigned short* green;
    unsigned short* blue;
    unsigned int size;
} GLFWgammaramp;

typedef struct GLFWimage {
    int width;
    int height;
    unsigned char* pixels;
} GLFWimage;

typedef struct GLFWgamepadstate {
    unsigned char buttons[15];
    float axes[6];
} GLFWgamepadstate;

typedef struct GLFWallocator {
    GLFWallocatefun allocate;
    GLFWreallocatefun reallocate;
    GLFWdeallocatefun deallocate;
    void* user;
} GLFWallocator;

GLFWAPI const GLFWvidmode* glfwGetVideoModes(GLFWmonitor* monitor, int* count);

GLFWAPI void glfwSetWindowTitle(GLFWwindow* window, const char* title);
GLFWAPI void glfwSetWindowIcon(GLFWwindow* window, int count, const GLFWimage* images);
GLFWAPI void glfwSetWindowSize(GLFWwindow* window, int width, int height);
GLFWAPI void glfwIconifyWindow(GLFWwindow* window);
GLFWAPI void glfwRestoreWindow(GLFWwindow* window);
GLFWAPI void glfwMaximizeWindow(GLFWwindow* window);
GLFWAPI GLFWmonitor* glfwGetWindowMonitor(GLFWwindow* window);
GLFWAPI void glfwSetWindowMonitor(GLFWwindow* window, GLFWmonitor* monitor, int xpos, int ypos, int width, int height, int refreshRate);
GLFWAPI void glfwSetWindowAttrib(GLFWwindow* window, int attrib, int value);
GLFWAPI GLFWwindowposfun glfwSetWindowPosCallback(GLFWwindow* window, GLFWwindowposfun callback);
GLFWAPI GLFWwindowsizefun glfwSetWindowSizeCallback(GLFWwindow* window, GLFWwindowsizefun callback);
GLFWAPI GLFWwindowfocusfun glfwSetWindowFocusCallback(GLFWwindow* window, GLFWwindowfocusfun callback);

GLFWAPI const char* glfwGetKeyName(int key, int scancode);
GLFWAPI int glfwGetKeyScancode(int key);
GLFWAPI int glfwGetKey(GLFWwindow* window, int key);

GLFWAPI void glfwSetCursor(GLFWwindow* window, GLFWcursor* cursor);
GLFWAPI GLFWcharfun glfwSetCharCallback(GLFWwindow* window, GLFWcharfun callback);
GLFWAPI GLFWcharmodsfun glfwSetCharModsCallback(GLFWwindow* window, GLFWcharmodsfun callback);
GLFWAPI GLFWcursorenterfun glfwSetCursorEnterCallback(GLFWwindow* window, GLFWcursorenterfun callback);

GLFWAPI int glfwJoystickPresent(int jid);
GLFWAPI const float* glfwGetJoystickAxes(int jid, int* count);
GLFWAPI const unsigned char* glfwGetJoystickButtons(int jid, int* count);
GLFWAPI const unsigned char* glfwGetJoystickHats(int jid, int* count);
GLFWAPI const char* glfwGetJoystickName(int jid);
GLFWAPI const char* glfwGetJoystickGUID(int jid);
GLFWAPI void glfwSetJoystickUserPointer(int jid, void* pointer);
GLFWAPI void* glfwGetJoystickUserPointer(int jid);
GLFWAPI int glfwJoystickIsGamepad(int jid);
GLFWAPI GLFWjoystickfun glfwSetJoystickCallback(GLFWjoystickfun callback);

GLFWAPI int glfwUpdateGamepadMappings(const char* string);
GLFWAPI const char* glfwGetGamepadName(int jid);
GLFWAPI int glfwGetGamepadState(int jid, GLFWgamepadstate* state);

void glfwGetMonitorWorkarea(GLFWmonitor * monitor, int* xpos, int* ypos, int* width, int* height);

// Used by Voxen:
#define GLFW_SRGB_CAPABLE           0x0002100E
#define GLFW_CONTEXT_VERSION_MAJOR  0x00022002
#define GLFW_CONTEXT_VERSION_MINOR  0x00022003
#define GLFW_OPENGL_PROFILE         0x00022008
#define GLFW_CLIENT_API             0x00022001

GLFWframebuffersizefun glfwSetFramebufferSizeCallback (GLFWwindow *window, GLFWframebuffersizefun callback);
GLFWAPI GLFWglproc glfwGetProcAddress(const char* procname);
GLFWAPI int glfwInit(void);
GLFWAPI const char* glfwGetVersionString(void);
GLFWAPI GLFWmonitor** glfwGetMonitors(int* count);
GLFWAPI GLFWmonitor* glfwGetPrimaryMonitor(void);
GLFWAPI void glfwGetMonitorPos(GLFWmonitor* monitor, int* xpos, int* ypos);
GLFWAPI const char* glfwGetMonitorName(GLFWmonitor* monitor);
GLFWAPI const GLFWvidmode* glfwGetVideoMode(GLFWmonitor* monitor);
GLFWAPI int glfwWindowShouldClose(GLFWwindow* window);
GLFWAPI void glfwWindowHint(int hint, int value);
GLFWAPI GLFWwindow* glfwCreateWindow(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share);
GLFWAPI void glfwSetWindowPos(GLFWwindow* window, int xpos, int ypos);
GLFWAPI void glfwPollEvents(void);
GLFWAPI void glfwSetInputMode(GLFWwindow* window, int mode, int value);
GLFWAPI int glfwGetMouseButton(GLFWwindow* window, int button);
GLFWAPI void glfwSetCursorPos(GLFWwindow* window, double xpos, double ypos);
GLFWAPI GLFWkeyfun glfwSetKeyCallback(GLFWwindow* window, GLFWkeyfun callback);
GLFWAPI GLFWmousebuttonfun glfwSetMouseButtonCallback(GLFWwindow* window, GLFWmousebuttonfun callback);
GLFWAPI GLFWcursorposfun glfwSetCursorPosCallback(GLFWwindow* window, GLFWcursorposfun callback);
GLFWAPI GLFWscrollfun glfwSetScrollCallback(GLFWwindow* window, GLFWscrollfun callback);
GLFWAPI void glfwMakeContextCurrent(GLFWwindow* window);
GLFWAPI void glfwSwapBuffers(GLFWwindow* window);
GLFWAPI void glfwSwapInterval(int interval);
GLFWAPI void glfwGetWindowPos(GLFWwindow* window, int* xpos, int* ypos);
GLFWAPI void glfwGetWindowSize(GLFWwindow* window, int* width, int* height);

#ifdef GLFW_WINGDIAPI_DEFINED
 #undef WINGDIAPI
 #undef GLFW_WINGDIAPI_DEFINED
#endif

#ifdef GLFW_CALLBACK_DEFINED
 #undef CALLBACK
 #undef GLFW_CALLBACK_DEFINED
#endif

#ifndef GLAPIENTRY
 #define GLAPIENTRY APIENTRY
 #define GLFW_GLAPIENTRY_DEFINED
#endif

#endif /* _glfw3_h_ */

