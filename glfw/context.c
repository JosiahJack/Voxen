#include "internal.h"
#include <assert.h>
#include <string.h>
#include <limits.h>

GLFWbool _glfwIsValidContextConfig(const _GLFWctxconfig* c) {
    if (c->source != GLFW_NATIVE_CONTEXT_API) { _glfwInputError(GLFW_INVALID_ENUM, "Invalid context creation API 0x%08X", c->source); return GLFW_FALSE; }
    if (c->client != GLFW_OPENGL_API) { _glfwInputError(GLFW_INVALID_ENUM, "Invalid client API 0x%08X", c->client); return GLFW_FALSE; }
    if (c->share) {
        if (c->source != c->share->context.source) { _glfwInputError(GLFW_INVALID_ENUM, "Context creation APIs do not match between contexts"); return GLFW_FALSE; }
    }
    if (c->client == GLFW_OPENGL_API) {
        if ((c->major < 1 || c->minor < 0) || (c->major == 1 && c->minor > 5) ||
            (c->major == 2 && c->minor > 1) || (c->major == 3 && c->minor > 3))
            { _glfwInputError(GLFW_INVALID_VALUE, "Invalid OpenGL version %i.%i", c->major, c->minor); return GLFW_FALSE; }
        if (c->profile) {
            if (c->profile != GLFW_OPENGL_CORE_PROFILE && c->profile != GLFW_OPENGL_COMPAT_PROFILE)
                { _glfwInputError(GLFW_INVALID_ENUM, "Invalid OpenGL profile 0x%08X", c->profile); return GLFW_FALSE; }
            if (c->major <= 2 || (c->major == 3 && c->minor < 2))
                { _glfwInputError(GLFW_INVALID_VALUE, "Context profiles are only defined for OpenGL version 3.2 and above"); return GLFW_FALSE; }
        }
        if (c->forward && c->major <= 2)
            { _glfwInputError(GLFW_INVALID_VALUE, "Forward-compatibility is only defined for OpenGL version 3.0 and above"); return GLFW_FALSE; }
    } else if (c->client == GLFW_OPENGL_ES_API) {
        if (c->major < 1 || c->minor < 0 || (c->major == 1 && c->minor > 1) || (c->major == 2 && c->minor > 0))
            { _glfwInputError(GLFW_INVALID_VALUE, "Invalid OpenGL ES version %i.%i", c->major, c->minor); return GLFW_FALSE; }
    }
    if (c->robustness && c->robustness != GLFW_NO_RESET_NOTIFICATION && c->robustness != GLFW_LOSE_CONTEXT_ON_RESET)
        { _glfwInputError(GLFW_INVALID_ENUM, "Invalid context robustness mode 0x%08X", c->robustness); return GLFW_FALSE; }
    if (c->release && c->release != GLFW_RELEASE_BEHAVIOR_NONE && c->release != GLFW_RELEASE_BEHAVIOR_FLUSH)
        { _glfwInputError(GLFW_INVALID_ENUM, "Invalid context release behavior 0x%08X", c->release); return GLFW_FALSE; }
    return GLFW_TRUE;
}

const _GLFWfbconfig* _glfwChooseFBConfig(const _GLFWfbconfig* desired, const _GLFWfbconfig* alts, unsigned int count)
{
    unsigned int missing, leastMissing = UINT_MAX, colorDiff, leastColorDiff = UINT_MAX, extraDiff, leastExtraDiff = UINT_MAX;
    const _GLFWfbconfig* closest = NULL;
    for (unsigned int i = 0; i < count; i++) {
        const _GLFWfbconfig* cur = alts + i;
        if (desired->stereo > 0 && cur->stereo == 0) continue;
        missing = 0;
        if (desired->alphaBits   > 0 && cur->alphaBits == 0)   missing++;
        if (desired->depthBits   > 0 && cur->depthBits == 0)   missing++;
        if (desired->stencilBits > 0 && cur->stencilBits == 0) missing++;
        if (desired->auxBuffers  > 0 && cur->auxBuffers < desired->auxBuffers) missing += desired->auxBuffers - cur->auxBuffers;
        if (desired->samples     > 0 && cur->samples == 0)     missing++;
        if (desired->transparent != cur->transparent)          missing++;
        colorDiff = 0;
        if (desired->redBits   != GLFW_DONT_CARE) colorDiff += (desired->redBits   - cur->redBits)   * (desired->redBits   - cur->redBits);
        if (desired->greenBits != GLFW_DONT_CARE) colorDiff += (desired->greenBits - cur->greenBits) * (desired->greenBits - cur->greenBits);
        if (desired->blueBits  != GLFW_DONT_CARE) colorDiff += (desired->blueBits  - cur->blueBits)  * (desired->blueBits  - cur->blueBits);
        extraDiff = 0;
        if (desired->alphaBits      != GLFW_DONT_CARE) extraDiff += (desired->alphaBits      - cur->alphaBits)      * (desired->alphaBits      - cur->alphaBits);
        if (desired->depthBits      != GLFW_DONT_CARE) extraDiff += (desired->depthBits      - cur->depthBits)      * (desired->depthBits      - cur->depthBits);
        if (desired->stencilBits    != GLFW_DONT_CARE) extraDiff += (desired->stencilBits    - cur->stencilBits)    * (desired->stencilBits    - cur->stencilBits);
        if (desired->accumRedBits   != GLFW_DONT_CARE) extraDiff += (desired->accumRedBits   - cur->accumRedBits)   * (desired->accumRedBits   - cur->accumRedBits);
        if (desired->accumGreenBits != GLFW_DONT_CARE) extraDiff += (desired->accumGreenBits - cur->accumGreenBits) * (desired->accumGreenBits - cur->accumGreenBits);
        if (desired->accumBlueBits  != GLFW_DONT_CARE) extraDiff += (desired->accumBlueBits  - cur->accumBlueBits)  * (desired->accumBlueBits  - cur->accumBlueBits);
        if (desired->accumAlphaBits != GLFW_DONT_CARE) extraDiff += (desired->accumAlphaBits - cur->accumAlphaBits) * (desired->accumAlphaBits - cur->accumAlphaBits);
        if (desired->samples        != GLFW_DONT_CARE) extraDiff += (desired->samples        - cur->samples)        * (desired->samples        - cur->samples);
        if (desired->sRGB && !cur->sRGB) extraDiff++;
        if (missing < leastMissing || (missing == leastMissing && (colorDiff < leastColorDiff || (colorDiff == leastColorDiff && extraDiff < leastExtraDiff))))
            closest = cur;
        if (cur == closest) { leastMissing = missing; leastColorDiff = colorDiff; leastExtraDiff = extraDiff; }
    }
    return closest;
}

int sscanf(const char *str, const char *format, ...);
GLFWbool _glfwRefreshContextAttribs(_GLFWwindow* window, const _GLFWctxconfig* ctxconfig) {
    const char* prefixes[] = { "OpenGL ES-CM ", "OpenGL ES-CL ", "OpenGL ES ", NULL };
    window->context.source = ctxconfig->source;
    window->context.client = GLFW_OPENGL_API;

    _GLFWwindow* previous = _glfwPlatformGetTls(&_glfw.contextSlot);
    glfwMakeContextCurrent((GLFWwindow*) window);
    if (_glfwPlatformGetTls(&_glfw.contextSlot) != window) return GLFW_FALSE;

    window->context.GetIntegerv = (PFNGLGETINTEGERVPROC) window->context.getProcAddress("glGetIntegerv");
    window->context.GetString   = (PFNGLGETSTRINGPROC)   window->context.getProcAddress("glGetString");
    if (!window->context.GetIntegerv || !window->context.GetString)
        { _glfwInputError(GLFW_PLATFORM_ERROR, "Entry point retrieval is broken"); glfwMakeContextCurrent((GLFWwindow*) previous); return GLFW_FALSE; }

    const char* version = (const char*) window->context.GetString(GL_VERSION);
    if (!version)
        { _glfwInputError(GLFW_PLATFORM_ERROR, "OpenGL version string retrieval is broken"); glfwMakeContextCurrent((GLFWwindow*) previous); return GLFW_FALSE; }

    for (int i = 0; prefixes[i]; i++) {
        const size_t len = strlen(prefixes[i]);
        if (strncmp(version, prefixes[i], len) == 0) { version += len; window->context.client = GLFW_OPENGL_ES_API; break; }
    }

    if (!sscanf(version, "%d.%d.%d", &window->context.major, &window->context.minor, &window->context.revision))
        { _glfwInputError(GLFW_PLATFORM_ERROR, "No version found in OpenGL version string"); glfwMakeContextCurrent((GLFWwindow*) previous); return GLFW_FALSE; }

    if (window->context.major < ctxconfig->major || (window->context.major == ctxconfig->major && window->context.minor < ctxconfig->minor))
        { _glfwInputError(GLFW_VERSION_UNAVAILABLE, "Requested OpenGL version %i.%i, got version %i.%i", ctxconfig->major, ctxconfig->minor, window->context.major, window->context.minor); glfwMakeContextCurrent((GLFWwindow*) previous); return GLFW_FALSE; }

    if (window->context.major >= 3) {
        window->context.GetStringi = (PFNGLGETSTRINGIPROC) window->context.getProcAddress("glGetStringi");
        if (!window->context.GetStringi)
            { _glfwInputError(GLFW_PLATFORM_ERROR, "Entry point retrieval is broken"); glfwMakeContextCurrent((GLFWwindow*) previous); return GLFW_FALSE; }
        GLint flags; window->context.GetIntegerv(GL_CONTEXT_FLAGS, &flags);
        if (flags & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT) window->context.forward = GLFW_TRUE;
        if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)              window->context.debug   = GLFW_TRUE;
        else if (glfwExtensionSupported("GL_ARB_debug_output") && ctxconfig->debug) window->context.debug = GLFW_TRUE;
        if (flags & GL_CONTEXT_FLAG_NO_ERROR_BIT_KHR)      window->context.noerror = GLFW_TRUE;
    }
    if (window->context.major >= 4 || (window->context.major == 3 && window->context.minor >= 2)) {
        GLint mask; window->context.GetIntegerv(GL_CONTEXT_PROFILE_MASK, &mask);
        if      (mask & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT) window->context.profile = GLFW_OPENGL_COMPAT_PROFILE;
        else if (mask & GL_CONTEXT_CORE_PROFILE_BIT)          window->context.profile = GLFW_OPENGL_CORE_PROFILE;
        else if (glfwExtensionSupported("GL_ARB_compatibility")) window->context.profile = GLFW_OPENGL_COMPAT_PROFILE;
    }
    if (glfwExtensionSupported("GL_ARB_robustness")) {
        GLint strategy; window->context.GetIntegerv(GL_RESET_NOTIFICATION_STRATEGY_ARB, &strategy);
        if      (strategy == GL_LOSE_CONTEXT_ON_RESET_ARB)   window->context.robustness = GLFW_LOSE_CONTEXT_ON_RESET;
        else if (strategy == GL_NO_RESET_NOTIFICATION_ARB)   window->context.robustness = GLFW_NO_RESET_NOTIFICATION;
    }
    if (glfwExtensionSupported("GL_KHR_context_flush_control")) {
        GLint behavior; window->context.GetIntegerv(GL_CONTEXT_RELEASE_BEHAVIOR, &behavior);
        if      (behavior == GL_NONE)                            window->context.release = GLFW_RELEASE_BEHAVIOR_NONE;
        else if (behavior == GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH)  window->context.release = GLFW_RELEASE_BEHAVIOR_FLUSH;
    }

    PFNGLCLEARPROC glClear = (PFNGLCLEARPROC) window->context.getProcAddress("glClear");
    glClear(GL_COLOR_BUFFER_BIT);
    if (window->doublebuffer) window->context.swapBuffers(window);

    glfwMakeContextCurrent((GLFWwindow*) previous);
    return GLFW_TRUE;
}

GLFWbool _glfwStringInExtensionString(const char* string, const char* extensions)
{
    const char* start = extensions;
    for (;;) {
        const char* where = strstr(start, string);
        if (!where) return GLFW_FALSE;
        const char* terminator = where + strlen(string);
        if ((where == start || *(where - 1) == ' ') && (*terminator == ' ' || *terminator == '\0')) break;
        start = terminator;
    }
    return GLFW_TRUE;
}

GLFWAPI void glfwMakeContextCurrent(GLFWwindow* handle)
{
    _GLFW_REQUIRE_INIT();
    _GLFWwindow* window = (_GLFWwindow*) handle;
    _GLFWwindow* previous = _glfwPlatformGetTls(&_glfw.contextSlot);
    if (window && window->context.client == GLFW_NO_API)
        { _glfwInputError(GLFW_NO_WINDOW_CONTEXT, "Cannot make current with a window that has no OpenGL or OpenGL ES context"); return; }
    if (previous && (!window || window->context.source != previous->context.source)) previous->context.makeCurrent(NULL);
    if (window) window->context.makeCurrent(window);
}

GLFWAPI GLFWwindow* glfwGetCurrentContext(void)
    { _GLFW_REQUIRE_INIT_OR_RETURN(NULL); return _glfwPlatformGetTls(&_glfw.contextSlot); }

GLFWAPI void glfwSwapBuffers(GLFWwindow* handle)
{
    _GLFW_REQUIRE_INIT();
    _GLFWwindow* window = (_GLFWwindow*) handle;
    assert(window != NULL);
    if (window->context.client == GLFW_NO_API)
        { _glfwInputError(GLFW_NO_WINDOW_CONTEXT, "Cannot swap buffers of a window that has no OpenGL or OpenGL ES context"); return; }
    window->context.swapBuffers(window);
}

GLFWAPI void glfwSwapInterval(int interval)
{
    _GLFW_REQUIRE_INIT();
    _GLFWwindow* window = _glfwPlatformGetTls(&_glfw.contextSlot);
    if (!window) { _glfwInputError(GLFW_NO_CURRENT_CONTEXT, "Cannot set swap interval without a current OpenGL or OpenGL ES context"); return; }
    window->context.swapInterval(interval);
}

GLFWAPI int glfwExtensionSupported(const char* extension)
{
    assert(extension != NULL);
    _GLFW_REQUIRE_INIT_OR_RETURN(GLFW_FALSE);
    _GLFWwindow* window = _glfwPlatformGetTls(&_glfw.contextSlot);
    if (!window) { _glfwInputError(GLFW_NO_CURRENT_CONTEXT, "Cannot query extension without a current OpenGL or OpenGL ES context"); return GLFW_FALSE; }
    if (*extension == '\0') { _glfwInputError(GLFW_INVALID_VALUE, "Extension name cannot be an empty string"); return GLFW_FALSE; }
    if (window->context.major >= 3) {
        GLint count; window->context.GetIntegerv(GL_NUM_EXTENSIONS, &count);
        for (int i = 0; i < count; i++) {
            const char* en = (const char*) window->context.GetStringi(GL_EXTENSIONS, i);
            if (!en) { _glfwInputError(GLFW_PLATFORM_ERROR, "Extension string retrieval is broken"); return GLFW_FALSE; }
            if (strcmp(en, extension) == 0) return GLFW_TRUE;
        }
    } else {
        const char* extensions = (const char*) window->context.GetString(GL_EXTENSIONS);
        if (!extensions) { _glfwInputError(GLFW_PLATFORM_ERROR, "Extension string retrieval is broken"); return GLFW_FALSE; }
        if (_glfwStringInExtensionString(extension, extensions)) return GLFW_TRUE;
    }
    return window->context.extensionSupported(extension);
}

GLFWAPI GLFWglproc glfwGetProcAddress(const char* procname) {
    assert(procname != NULL);
    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);
    _GLFWwindow* window = _glfwPlatformGetTls(&_glfw.contextSlot);
    if (!window) { _glfwInputError(GLFW_NO_CURRENT_CONTEXT, "Cannot query entry point without a current OpenGL or OpenGL ES context"); return NULL; }
    return window->context.getProcAddress(procname);
}
