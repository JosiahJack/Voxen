#ifndef VOXEN_AUDIO_H
#define VOXEN_AUDIO_H

#include <SDL2/SDL.h>
#include "miniaudio.h"
// #include <fluidlite.h>
// #include <libxmi.h>

#define MAX_CHANNELS 64

extern ma_engine audio_engine;
// extern fluid_synth_t* midi_synth;
extern ma_sound mp3_sounds[2]; // For crossfading
extern ma_sound wav_sounds[MAX_CHANNELS];
extern int wav_count;

// int InitializeAudio(const char* soundfont_path);
int InitializeAudio();
// void play_midi(const char* midi_path);
void play_mp3(const char* path, float volume, int fade_in_ms);
void play_wav(const char* path, float volume);
void CleanupAudio();

#endif // VOXEN_AUDIO_H
