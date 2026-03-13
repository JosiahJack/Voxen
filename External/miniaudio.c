// miniaudio.c - Audio System
#define MINIAUDIO_IMPLEMENTATION
#define MA_ENABLE_ONLY_SPECIFIC_BACKENDS
// #define MA_ENABLE_WASAPI // For windows
// #define MA_ENABLE_DSOUND
#define MA_ENABLE_PULSEAUDIO // For Linux
// #define MA_ENABLE_COREAUDIO  // For Mac
#define MA_NO_ENCODING          // (you probably don't need this either)
#define MA_NO_WAV               // if you don't need WAV loading
#define MA_NO_FLAC
#define MA_NO_MP3
#define MA_NO_EFFECTS           // (redundant if node graph is off)
#define MA_NO_GENERATION        // removes sine/square/noise generators
#define MA_NO_SSE2              // optional, removes SSE2 paths
#define MA_NO_AVX2
#define MA_NO_NEON
#define MA_NO_RUNTIME_LINKING      // removes a ton of GetProcAddress crap on Windows
#define MA_NO_STDIO                // removes all fopen/fread fallback paths
#define MA_NO_DEVICE_ID            // you don’t need named devices, just default
#define MA_NO_DEFAULT_DEVICES      // removes the giant default-device-name tables
#define MA_NO_LOOPING              // removes all looping logic in data sources
#define MA_NO_PULSEAUDIO_CONTEXT
#include "../voxen.h"
#include "miniaudio.h"
