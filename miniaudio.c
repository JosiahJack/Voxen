// miniaudio.c - Audio System
#define MINIAUDIO_IMPLEMENTATION
#define MA_NO_FLAC
#define MA_NO_SNDIO
#define MA_NO_NULL
#define MA_NO_AUDIO4
#define MA_NO_WEBAUDIO
#define MA_NO_CUSTOM
#define MA_NO_AAUDIO
#define MA_NO_COREAUDIO
#define MA_NO_JACK
#ifndef WINDOWS
    #define MA_NO_WINMM
    #define MA_NO_DSOUND
    #define MA_NO_WASAPI
#endif
#define MA_NO_OPENSL
#define MA_NO_AVX2
#define MA_NO_NEON
#define MA_NO_ENCODING
#include "os.h"
#include "miniaudio.h"
