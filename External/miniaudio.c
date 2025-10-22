#define MINIAUDIO_IMPLEMENTATION
#define MA_ENABLE_ONLY_SPECIFIC_BACKENDS
#define MA_ENABLE_WASAPI // For windows
#define MA_ENABLE_WINMM  // For windows
#define MA_ENABLE_DSOUND // For windows
#define MA_ENABLE_ALSA       // For Linux
#define MA_ENABLE_PULSEAUDIO // For Linux
#define MA_ENABLE_COREAUDIO  // For Mac
#define MA_NO_ENCODING // Don't need to save out audio files.
#define MA_NO_GENERATION // Disable waveform/noise generation (not needed for file playback)
#define MA_NO_FLAC
#define MA_NO_NEON
#include "./External/miniaudio.h"
// Minor modification made to miniaudio.h, otherwise unchanged vs https://github.com/mackron/miniaudio version 0.11.22 (350784a...)
// In file included from audio.c:3:
// miniaudio.h: In function ‘ma_data_source_read_pcm_frames’:
// miniaudio.h:57841:30: warning: ‘framesProcessed’ may be used uninitialized [-Wmaybe-uninitialized]
// 57841 |         totalFramesProcessed += framesProcessed;
//       |         ~~~~~~~~~~~~~~~~~~~~~^~~~~~~~~~~~~~~~~~
// Added `= 0` to line 57827 to make it `ma_uint64 framesProcessed = 0;`
