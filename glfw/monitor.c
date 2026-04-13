#include "internal.h"
#include <math.h>
#include <float.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

static int compareVideoModes(const void* fp, const void* sp) {
    const GLFWvidmode *fm=fp, *sm=sp;
    const int fbpp=fm->redBits + fm->greenBits + fm->blueBits, sbpp=sm->redBits + sm->greenBits + sm->blueBits;
    const int farea=fm->width * fm->height, sarea=sm->width * sm->height;
    if (fbpp != sbpp) return fbpp - sbpp;
    if (farea != sarea) return farea - sarea;
    if (fm->width != sm->width) return fm->width - sm->width;
    return fm->refreshRate - sm->refreshRate;
}

static GLFWbool refreshVideoModes(_GLFWmonitor* monitor) {
    int modeCount; GLFWvidmode* modes;
    if (monitor->modes) return GLFW_TRUE;

    modes = _glfw.platform.getVideoModes(monitor, &modeCount);
    if (!modes) return GLFW_FALSE;

    qsort(modes, modeCount, sizeof(GLFWvidmode), compareVideoModes);
    _glfw_free(monitor->modes);
    monitor->modes = modes;
    monitor->modeCount = modeCount;
    return GLFW_TRUE;
}

void _glfwInputMonitor(_GLFWmonitor* monitor, int action, int placement) {
    if (action == GLFW_CONNECTED) {
        _glfw.monitorCount++;
        _glfw.monitors = _glfw_realloc(_glfw.monitors,sizeof(_GLFWmonitor*) * _glfw.monitorCount);
        if (placement == _GLFW_INSERT_FIRST) {
            memmove(_glfw.monitors + 1,_glfw.monitors,((size_t) _glfw.monitorCount - 1) * sizeof(_GLFWmonitor*));
            _glfw.monitors[0] = monitor;
        } else _glfw.monitors[_glfw.monitorCount - 1] = monitor;
    } else if (action == GLFW_DISCONNECTED) {
        int i;
        _GLFWwindow* window;
        for (window = _glfw.windowListHead;  window;  window = window->next) {
            if (window->monitor == monitor) {
                int width, height, xoff, yoff;
                _glfw.platform.getWindowSize(window, &width, &height);
                _glfw.platform.setWindowMonitor(window, NULL, 0, 0, width, height, 0);
                _glfw.platform.getWindowFrameSize(window, &xoff, &yoff, NULL, NULL);
                _glfw.platform.setWindowPos(window, xoff, yoff);
            }
        }

        for (i = 0;  i < _glfw.monitorCount;  i++) {
            if (_glfw.monitors[i] == monitor) {
                _glfw.monitorCount--;
                memmove(_glfw.monitors + i, _glfw.monitors + i + 1,((size_t) _glfw.monitorCount - i) * sizeof(_GLFWmonitor*));
                break;
            }
        }
    }

    if (_glfw.callbacks.monitor) _glfw.callbacks.monitor((GLFWmonitor*) monitor, action);
}

void _glfwInputMonitorWindow(_GLFWmonitor* monitor, _GLFWwindow* window) { monitor->window = window; }
_GLFWmonitor* _glfwAllocMonitor(const char* name, int widthMM, int heightMM) {
    _GLFWmonitor* monitor = _glfw_calloc(1, sizeof(_GLFWmonitor));
    monitor->widthMM = widthMM; monitor->heightMM = heightMM;
    strncpy(monitor->name, name, sizeof(monitor->name) - 1);
    return monitor;
}

void _glfwAllocGammaArrays(GLFWgammaramp* ramp, unsigned int size) {
    ramp->red = _glfw_calloc(size, sizeof(unsigned short));
    ramp->green = _glfw_calloc(size, sizeof(unsigned short));
    ramp->blue = _glfw_calloc(size, sizeof(unsigned short));
    ramp->size = size;
}

void _glfwFreeGammaArrays(GLFWgammaramp* ramp) { _glfw_free(ramp->red); _glfw_free(ramp->green); _glfw_free(ramp->blue); memset(ramp,0,sizeof(GLFWgammaramp)); }
const GLFWvidmode* _glfwChooseVideoMode(_GLFWmonitor* monitor, const GLFWvidmode* desired) {
    int i; unsigned int sizeDiff,leastSizeDiff=UINT_MAX,rateDiff,leastRateDiff=UINT_MAX,colorDiff,leastColorDiff=UINT_MAX; const GLFWvidmode*current, *closest=NULL;
    if (!refreshVideoModes(monitor)) return NULL;

    for (i = 0;  i < monitor->modeCount;  i++) {
        current = monitor->modes + i;
        colorDiff = 0;
        if (desired->redBits != GLFW_DONT_CARE) colorDiff += abs(current->redBits - desired->redBits);
        if (desired->greenBits != GLFW_DONT_CARE) colorDiff += abs(current->greenBits - desired->greenBits);
        if (desired->blueBits != GLFW_DONT_CARE) colorDiff += abs(current->blueBits - desired->blueBits);
        sizeDiff = abs((current->width - desired->width) * (current->width - desired->width) + (current->height - desired->height) * (current->height - desired->height));
        if (desired->refreshRate != GLFW_DONT_CARE) rateDiff = abs(current->refreshRate - desired->refreshRate);
        else rateDiff = UINT_MAX - current->refreshRate;

        if ((colorDiff < leastColorDiff) || (colorDiff == leastColorDiff && sizeDiff < leastSizeDiff) || (colorDiff == leastColorDiff && sizeDiff == leastSizeDiff && rateDiff < leastRateDiff)) {
            closest = current;
            leastSizeDiff = sizeDiff;
            leastRateDiff = rateDiff;
            leastColorDiff = colorDiff;
        }
    }

    return closest;
}

int _glfwCompareVideoModes(const GLFWvidmode* fm, const GLFWvidmode* sm) { return compareVideoModes(fm, sm); }
void _glfwSplitBPP(int bpp, int* red, int* green, int* blue) {
    int delta;
    if (bpp == 32) bpp = 24; // We assume that by 32 the user really meant 24
    *red = *green = *blue = bpp / 3; // Convert "bits per pixel" to red, green & blue sizes
    delta = bpp - (*red * 3);
    if (delta >= 1) *green = *green + 1;
    if (delta == 2) *red = *red + 1;
}

GLFWAPI GLFWmonitor** glfwGetMonitors(int* count) {
    *count = 0;
    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);
    *count = _glfw.monitorCount;
    return (GLFWmonitor**) _glfw.monitors;
}

GLFWAPI GLFWmonitor* glfwGetPrimaryMonitor(void) {
    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);
    if (!_glfw.monitorCount) return NULL;

    return (GLFWmonitor*) _glfw.monitors[0];
}

GLFWAPI void glfwGetMonitorPos(GLFWmonitor* handle, int* xpos, int* ypos) {
    if (xpos) *xpos = 0;
    if (ypos) *ypos = 0;
    _GLFW_REQUIRE_INIT();
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;
    _glfw.platform.getMonitorPos(monitor, xpos, ypos);
}

GLFWAPI void glfwGetMonitorWorkarea(GLFWmonitor* handle, int* xpos, int* ypos, int* width, int* height) {
    if (xpos) *xpos = 0;
    if (ypos) *ypos = 0;
    if (width) *width = 0;
    if (height) *height = 0;
    _GLFW_REQUIRE_INIT();
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;
    _glfw.platform.getMonitorWorkarea(monitor, xpos, ypos, width, height);
}

GLFWAPI void glfwGetMonitorPhysicalSize(GLFWmonitor* handle, int* widthMM, int* heightMM) {
    if (widthMM) *widthMM = 0;
    if (heightMM) *heightMM = 0;
    _GLFW_REQUIRE_INIT();
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;
    if (widthMM) *widthMM = monitor->widthMM;
    if (heightMM) *heightMM = monitor->heightMM;
}

GLFWAPI void glfwGetMonitorContentScale(GLFWmonitor* handle, float* xscale, float* yscale) {
    if (xscale) *xscale = 0.f;
    if (yscale) *yscale = 0.f;
    _GLFW_REQUIRE_INIT();
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;
    _glfw.platform.getMonitorContentScale(monitor, xscale, yscale);
}

GLFWAPI const char* glfwGetMonitorName(GLFWmonitor* handle) {
    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;
    return monitor->name;
}

GLFWAPI void glfwSetMonitorUserPointer(GLFWmonitor* handle, void* pointer) {
    _GLFW_REQUIRE_INIT();
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;
    monitor->userPointer = pointer;
}

GLFWAPI void* glfwGetMonitorUserPointer(GLFWmonitor* handle) {
    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;
    return monitor->userPointer;
}

GLFWAPI GLFWmonitorfun glfwSetMonitorCallback(GLFWmonitorfun cbfun) {
    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);
    _GLFW_SWAP(GLFWmonitorfun, _glfw.callbacks.monitor, cbfun);
    return cbfun;
}

GLFWAPI const GLFWvidmode* glfwGetVideoModes(GLFWmonitor* handle, int* count) {
    *count = 0;
    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;
    if (!refreshVideoModes(monitor)) return NULL;
    *count = monitor->modeCount;
    return monitor->modes;
}

GLFWAPI const GLFWvidmode* glfwGetVideoMode(GLFWmonitor* handle) {
    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;
    if (!_glfw.platform.getVideoMode(monitor, &monitor->currentMode)) return NULL;
    return &monitor->currentMode;
}

GLFWAPI void glfwSetGamma(GLFWmonitor* handle, float gamma) {
    unsigned int i; unsigned short* values;
    GLFWgammaramp ramp;
    const GLFWgammaramp* original;
    _GLFW_REQUIRE_INIT();
    if (gamma != gamma || gamma <= 0.f || gamma > FLT_MAX) { _glfwInputError(GLFW_INVALID_VALUE, "Invalid gamma value %f", gamma); return; }

    original = glfwGetGammaRamp(handle);
    if (!original) return;

    values = _glfw_calloc(original->size, sizeof(unsigned short));
    for (i = 0;  i < original->size;  i++) {
        float value;
        value = i / (float) (original->size - 1);
        value = powf(value, 1.f / gamma) * 65535.f + 0.5f;
        value = fminf(value, 65535.f);
        values[i] = (unsigned short) value;
    }

    ramp.red = values;
    ramp.green = values;
    ramp.blue = values;
    ramp.size = original->size;
    glfwSetGammaRamp(handle, &ramp);
    _glfw_free(values);
}

GLFWAPI const GLFWgammaramp* glfwGetGammaRamp(GLFWmonitor* handle) {
    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;
    _glfwFreeGammaArrays(&monitor->currentRamp);
    if (!_glfw.platform.getGammaRamp(monitor, &monitor->currentRamp)) return NULL;
    return &monitor->currentRamp;
}

GLFWAPI void glfwSetGammaRamp(GLFWmonitor* handle, const GLFWgammaramp* ramp) {
    _GLFW_REQUIRE_INIT();
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;
    if (ramp->size <= 0) { _glfwInputError(GLFW_INVALID_VALUE,"Invalid gamma ramp size %i",ramp->size); return; }

    if (!monitor->originalRamp.size) {
        if (!_glfw.platform.getGammaRamp(monitor, &monitor->originalRamp)) return;
    }

    _glfw.platform.setGammaRamp(monitor, ramp);
}
