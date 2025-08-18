#include "External/miniaudio.h"
// Minor modification made to miniaudio.h, otherwise unchanged vs https://github.com/mackron/miniaudio version 0.11.22 (350784a...)
// In file included from audio.c:3:
// miniaudio.h: In function ‘ma_data_source_read_pcm_frames’:
// miniaudio.h:57841:30: warning: ‘framesProcessed’ may be used uninitialized [-Wmaybe-uninitialized]
// 57841 |         totalFramesProcessed += framesProcessed;
//       |         ~~~~~~~~~~~~~~~~~~~~~^~~~~~~~~~~~~~~~~~
// Added `= 0` to line 57827 to make it `ma_uint64 framesProcessed = 0;`
