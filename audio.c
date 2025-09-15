// #define INCLUDE_AUDIO_LIB // Comment out for faster builds during testing
#ifdef INCLUDE_AUDIO_LIB
    #define MINIAUDIO_IMPLEMENTATION
    #define MA_ENABLE_ONLY_SPECIFIC_BACKENDS
//     #define MA_ENABLE_WASAPI // For windows
//     #define MA_ENABLE_WINMM  // For windows
//     #define MA_ENABLE_DSOUND // For windows
    #define MA_ENABLE_ALSA       // For Linux
    #define MA_ENABLE_PULSEAUDIO // For Linux
//     #define MA_ENABLE_COREAUDIO  // For Mac
    #define MA_NO_ENCODING // Don't need to save out audio files.
    #define MA_NO_GENERATION // Disable waveform/noise generation (not needed for file playback)
    #define MA_NO_FLAC
    #define MA_NO_NEON
#endif

#include <SDL2/SDL.h>
#include "voxen.h"

// ----------------------------------------------------------------------------
// Usage:
//play_mp3("./Audio/music/looped/track1.mp3",0.08f,0); // WORKED!
//play_wav("./Audio/cyborgs/yourlevelsareterrible.wav",0.1f); // WORKED!
// ----------------------------------------------------------------------------

// Temporarily disabled midi support until wav+mp3 is working.
// #include <fluidlite.h>
// #include <libxmi.h>

// fluid_synth_t* midi_synth;
ma_engine audio_engine;
ma_sound mp3_sounds[2]; // For crossfading
ma_sound wav_sounds[MAX_CHANNELS];
int32_t wav_count = 0;

#ifndef INCLUDE_AUDIO_LIB
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

// void InitializeAudio(const char* soundfont_path) {
int32_t InitializeAudio() {
#ifdef INCLUDE_AUDIO_LIB
    ma_result result;
    ma_engine_config engine_config = ma_engine_config_init();
    engine_config.channels = 2; // Stereo output, adjust if needed

    result = ma_engine_init(&engine_config, &audio_engine);
    if (result != MA_SUCCESS) {
        DualLog("ERROR: Failed to initialize miniaudio engine: %d\n", result);
        return 1;
    }
    
//     fluid_settings_t* settings = new_fluid_settings();
//     midi_synth = new_fluid_synth(settings);
//     fluid_synth_sfopen(midi_synth, soundfont_path); // e.g., "./SoundFonts/FluidR3_GM.sf2"
#endif
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

void play_mp3(const char* path, float volume, int32_t fade_in_ms) {
#ifdef INCLUDE_AUDIO_LIB
    static int32_t current_sound = 0;
    ma_sound_uninit(&mp3_sounds[current_sound]);
    ma_result result = ma_sound_init_from_file(&audio_engine, path, MA_SOUND_FLAG_STREAM, NULL, NULL, &mp3_sounds[current_sound]);
    if (result != MA_SUCCESS) { DualLog("ERROR: Failed to load MP3 %s: %d\n", path, result);  return; }
    
    ma_sound_set_fade_in_milliseconds(&mp3_sounds[current_sound], 0.0f, volume, fade_in_ms);
    ma_sound_start(&mp3_sounds[current_sound]);
    current_sound = 1 - current_sound; // Toggle for crossfade
#endif
}

void play_wav(const char* path, float volume) {
#ifdef INCLUDE_AUDIO_LIB
    // Try to find a free slot (either unused or finished)
    int32_t slot = -1;
    for (int32_t i = 0; i < wav_count; i++) {
        if (!ma_sound_is_playing(&wav_sounds[i]) && ma_sound_at_end(&wav_sounds[i])) {
            ma_sound_uninit(&wav_sounds[i]);
            slot = i;
            break;
        }
    }
    
    // If no free slot, use a new one if available
    if (slot == -1 && wav_count < MAX_CHANNELS) slot = wav_count++;
    if (slot == -1) { DualLog("WARNING: Max WAV channels (%d) reached\n", MAX_CHANNELS); return; }

    ma_result result = ma_sound_init_from_file(&audio_engine, path, 0, NULL, NULL, &wav_sounds[slot]);
    if (result != MA_SUCCESS) {
        DualLog("ERROR: Failed to load WAV %s: %d\n", path, result);
        if (slot == wav_count - 1) wav_count--; // Revert count if init fails
        return;
    }
    
    ma_sound_set_volume(&wav_sounds[slot], volume);
    ma_sound_start(&wav_sounds[slot]);
#endif
}

void CleanupAudio() {
#ifdef INCLUDE_AUDIO_LIB
    for (int32_t i = 0; i < wav_count; i++) { ma_sound_uninit(&wav_sounds[i]); }
    for (int32_t i = 0; i < 2; i++) { ma_sound_uninit(&mp3_sounds[i]); }
    ma_engine_uninit(&audio_engine);
//     delete_fluid_synth(midi_synth);
#endif
}

#ifndef INCLUDE_AUDIO_LIB
#pragma GCC diagnostic pop
#endif
