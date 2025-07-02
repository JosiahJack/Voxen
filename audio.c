#define MINIAUDIO_IMPLEMENTATION
#define MA_NO_FLAC          // Disable FLAC decoder (not used, only need WAV and MP3)
#define MA_NO_WEBAUDIO      // Disable WebAudio backend (not needed for desktop)
#define MA_NO_AAUDIO        // Disable AAudio backend (Android-specific)
#define MA_NO_OPENSL        // Disable OpenSL|ES backend (Android-specific)
#define MA_NO_JACK          // Disable JACK backend (not commonly used on desktop)
#define MA_NO_SNDIO         // Disable sndio backend (BSD-specific, not needed)
#define MA_NO_AUDIO4        // Disable audio(4) backend (BSD-specific, not needed)
#define MA_NO_OSS           // Disable OSS backend (legacy Linux, not needed)
#define MA_NO_GENERATION    // Disable waveform/noise generation (not needed for file playback)
#include <SDL2/SDL.h>
#include "miniaudio.h"
// Minor modification made to miniaudio.h, otherwise unchanged vs https://github.com/mackron/miniaudio version 0.11.22 (350784a...)
// In file included from audio.c:3:
// miniaudio.h: In function ‘ma_data_source_read_pcm_frames’:
// miniaudio.h:57841:30: warning: ‘framesProcessed’ may be used uninitialized [-Wmaybe-uninitialized]
// 57841 |         totalFramesProcessed += framesProcessed;
//       |         ~~~~~~~~~~~~~~~~~~~~~^~~~~~~~~~~~~~~~~~
// Added `= 0` to line 57827 to make it `ma_uint64 framesProcessed = 0;`
#include "audio.h"

// Temporarily disabled midi support until wav+mp3 is working.
// #include <fluidlite.h>
// #include <libxmi.h>

// fluid_synth_t* midi_synth;
ma_engine audio_engine;
ma_sound mp3_sounds[2]; // For crossfading
ma_sound wav_sounds[MAX_CHANNELS];
int wav_count = 0;

// void InitializeAudio(const char* soundfont_path) {
int InitializeAudio() {
    ma_result result;
    ma_engine_config engine_config = ma_engine_config_init();
    engine_config.channels = 2; // Stereo output, adjust if needed

    result = ma_engine_init(&engine_config, &audio_engine);
    if (result != MA_SUCCESS) {
        printf("ERROR: Failed to initialize miniaudio engine: %d\n", result);
        return 1;
    }
    
//     fluid_settings_t* settings = new_fluid_settings();
//     midi_synth = new_fluid_synth(settings);
//     fluid_synth_sfopen(midi_synth, soundfont_path); // e.g., "./SoundFonts/FluidR3_GM.sf2"

    return 0;
}

// Temporarily disabled midi support until wav+mp3 is working.
// void play_midi(const char* midi_path) {
//     // Convert XMI to MIDI if needed
//     void* midi_data; size_t midi_size;
//     if (strstr(midi_path, ".xmi")) {
//         xmi2midi(midi_path, &midi_data, &midi_size);
//     } else {
//         // Load MIDI directly
//         FILE* f = fopen(midi_path, "rb");
//         fseek(f, 0, SEEK_END);
//         midi_size = ftell(f);
//         rewind(f);
//         midi_data = malloc(midi_size);
//         fread(midi_data, 1, midi_size, f);
//         fclose(f);
//     }
//     // Render MIDI to PCM, feed to miniaudio
//     short pcm[44100 * 2]; // Stereo, 1 second
//     fluid_synth_write_s16(midi_synth, 44100, pcm, 0, 2, pcm, 1, 2);
//     ma_sound_init_from_data_source(&audio_engine, pcm, sizeof(pcm), NULL, NULL);
//     free(midi_data);
// }

void play_mp3(const char* path, float volume, int fade_in_ms) {
    static int current_sound = 0;
    ma_sound_uninit(&mp3_sounds[current_sound]);
    ma_result result = ma_sound_init_from_file(&audio_engine, path, MA_SOUND_FLAG_STREAM, NULL, NULL, &mp3_sounds[current_sound]);
    if (result != MA_SUCCESS) { printf("ERROR: Failed to load MP3 %s: %d\n", path, result);  return; }
    
    ma_sound_set_fade_in_milliseconds(&mp3_sounds[current_sound], 0.0f, volume, fade_in_ms);
    ma_sound_start(&mp3_sounds[current_sound]);
    current_sound = 1 - current_sound; // Toggle for crossfade
}

void play_wav(const char* path, float volume) {
    // Try to find a free slot (either unused or finished)
    int slot = -1;
    for (int i = 0; i < wav_count; i++) {
        if (!ma_sound_is_playing(&wav_sounds[i]) && ma_sound_at_end(&wav_sounds[i])) {
            ma_sound_uninit(&wav_sounds[i]);
            slot = i;
            break;
        }
    }
    
    // If no free slot, use a new one if available
    if (slot == -1 && wav_count < MAX_CHANNELS) slot = wav_count++;
    if (slot == -1) { printf("WARNING: Max WAV channels (%d) reached\n", MAX_CHANNELS); return; }

    ma_result result = ma_sound_init_from_file(&audio_engine, path, 0, NULL, NULL, &wav_sounds[slot]);
    if (result != MA_SUCCESS) {
        printf("ERROR: Failed to load WAV %s: %d\n", path, result);
        if (slot == wav_count - 1) wav_count--; // Revert count if init fails
        return;
    }
    
    ma_sound_set_volume(&wav_sounds[slot], volume);
    ma_sound_start(&wav_sounds[slot]);
}

void CleanupAudio() {
    for (int i = 0; i < wav_count; i++) { ma_sound_uninit(&wav_sounds[i]); }
    for (int i = 0; i < 2; i++) { ma_sound_uninit(&mp3_sounds[i]); }
    ma_engine_uninit(&audio_engine);
//     delete_fluid_synth(midi_synth);
}
