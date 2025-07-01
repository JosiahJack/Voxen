#include <SDL.h>
#include <miniaudio.h>
#include <fluidlite.h>
#include <libxmi.h>
#define MAX_CHANNELS 16

ma_engine audio_engine;
fluid_synth_t* midi_synth;
ma_sound mp3_sounds[2]; // For crossfading
ma_sound wav_sounds[MAX_CHANNELS];
int wav_count = 0;

void init_audio(const char* soundfont_path) {
    ma_engine_config engine_config = ma_engine_config_init();
    engine_config.channels = MAX_CHANNELS;
    ma_engine_init(&engine_config, &audio_engine);

    fluid_settings_t* settings = new_fluid_settings();
    midi_synth = new_fluid_synth(settings);
    fluid_synth_sfopen(midi_synth, soundfont_path); // e.g., "./SoundFonts/FluidR3_GM.sf2"
}

void play_midi(const char* midi_path) {
    // Convert XMI to MIDI if needed
    void* midi_data; size_t midi_size;
    if (strstr(midi_path, ".xmi")) {
        xmi2midi(midi_path, &midi_data, &midi_size);
    } else {
        // Load MIDI directly
        FILE* f = fopen(midi_path, "rb");
        fseek(f, 0, SEEK_END);
        midi_size = ftell(f);
        rewind(f);
        midi_data = malloc(midi_size);
        fread(midi_data, 1, midi_size, f);
        fclose(f);
    }
    // Render MIDI to PCM, feed to miniaudio
    short pcm[44100 * 2]; // Stereo, 1 second
    fluid_synth_write_s16(midi_synth, 44100, pcm, 0, 2, pcm, 1, 2);
    ma_sound_init_from_data_source(&audio_engine, pcm, sizeof(pcm), NULL, NULL);
    free(midi_data);
}

void play_mp3(const char* path, int fade_in_ms) {
    static int current_sound = 0;
    ma_sound_uninit(&mp3_sounds[current_sound]);
    ma_sound_init_from_file(&audio_engine, path, MA_SOUND_FLAG_STREAM, NULL, NULL, &mp3_sounds[current_sound]);
    ma_sound_set_fade_in_milliseconds(&mp3_sounds[current_sound], 0.0f, 1.0f, fade_in_ms);
    ma_sound_start(&mp3_sounds[current_sound]);
    current_sound = 1 - current_sound; // Toggle for crossfade
}

void play_wav(const char* path) {
    if (wav_count < MAX_CHANNELS) {
        ma_sound_init_from_file(&audio_engine, path, 0, NULL, NULL, &wav_sounds[wav_count++]);
        ma_sound_start(&wav_sounds[wav_count-1]);
    }
}

void cleanup_audio() {
    for (int i = 0; i < wav_count; i++) ma_sound_uninit(&wav_sounds[i]);
    for (int i = 0; i < 2; i++) ma_sound_uninit(&mp3_sounds[i]);
    ma_engine_uninit(&audio_engine);
    delete_fluid_synth(midi_synth);
}
