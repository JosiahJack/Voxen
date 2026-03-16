// audio.c
#include "voxen.h"
#include "miniaudio.h"
#define MAX_CHANNELS 16
ma_sound wav_sounds[MAX_CHANNELS];
float wav_volumes[MAX_CHANNELS]; // Setting independent base sfx volume (e.g. dropped physics object hard or lightly volume, independent of position).
int32_t wav_count = 0;
ma_sound log_sound;
// Usage: play_wav("./Audio/cyborgs/yourlevelsareterrible.wav",0.1f); WORKED!

void InitializeAudio(void) {
    double startTime = get_time();
    ma_result result;
    ma_engine_config engine_config = ma_engine_config_init();
    engine_config.channels = 2; // Stereo output, adjust if needed
    result = ma_engine_init(&engine_config, &Sys_Global.audio_engine);
    if (result != MA_SUCCESS) DualLog("ERROR: Failed to initialize miniaudio engine: %d\n", result);
    DualLog("Initialize Audio took %f secs\n",get_time() - startTime);
}

ENGINE_TO_MOD bool GetSoundIsPlaying(ma_sound* sound) { return ma_sound_is_playing(sound); }
float GetSoundRemainingTime(ma_sound* pSound) {
    if (!pSound || !ma_sound_is_playing(pSound)) return 0.0f;

    ma_uint64 currentFrame = ma_sound_get_time_in_pcm_frames(pSound);
    ma_uint64 pcmFramesLength = 0;
    ma_sound_get_length_in_pcm_frames(pSound, &pcmFramesLength);
    if (currentFrame >= pcmFramesLength) return 0.0f;

    uint64_t deltaFrames = pcmFramesLength - currentFrame;
    uint32_t sampleRate = ma_engine_get_sample_rate(&Sys_Global.audio_engine);
    return (float)deltaFrames / (float)sampleRate;
}

void mp3_clear(void) {
    ma_sound_stop(&Sys_Global.mp3_sounds[0]);
    ma_sound_stop(&Sys_Global.mp3_sounds[1]);
    Sys_Global.mp3_slot = 0;
}

ENGINE_TO_MOD void SoundSetVolume(ma_sound* pSound, float volume) { ma_sound_set_volume(pSound,volume); }
float GetSFXVolume(float volume) { return ((float)Sys_Settings.VolumeMaster/100.0f) * ((float)Sys_Settings.VolumeEffects/100.0f) * volume; }
float GetMusicVolume(void) { return ((float)Sys_Settings.VolumeMaster/100.0f) * ((float)Sys_Settings.VolumeMusic/100.0f); }
float GetMessageVolume(void) { return ((float)Sys_Settings.VolumeMaster/100.0f) * ((float)Sys_Settings.VolumeMessage/100.0f); }
void set_music_volume(void) { for (int i=0;i<2;++i) { ma_sound_set_volume(&Sys_Global.mp3_sounds[i], GetMusicVolume()); } }
void set_sfx_volume(void) { for (int i=0;i<MAX_CHANNELS;++i) { ma_sound_set_volume(&wav_sounds[i], GetSFXVolume(wav_volumes[i])); } }
void set_message_volume(void) { ma_sound_set_volume(&log_sound, GetMessageVolume()); }
void set_master_volume(void) { set_sfx_volume(); set_music_volume(); set_message_volume(); }

void play_mp3(const char* path, int32_t fade_in_ms) {
    int32_t old_slot = Sys_Global.mp3_slot;
    int32_t next_slot = Sys_Global.mp3_slot ? 0 : 1;
    if (ma_sound_is_playing(&Sys_Global.mp3_sounds[old_slot])) ma_sound_set_fade_in_milliseconds(&Sys_Global.mp3_sounds[old_slot], GetMusicVolume(), 0.0f, fade_in_ms);
    ma_sound_uninit(&Sys_Global.mp3_sounds[next_slot]); 
    ma_result result = ma_sound_init_from_file(&Sys_Global.audio_engine, path, MA_SOUND_FLAG_STREAM, NULL, NULL, &Sys_Global.mp3_sounds[next_slot]);
    if (result != MA_SUCCESS) { DualLog("ERROR: Failed to load MP3 %s: %d\n", path, result); return; }

    ma_sound_set_fade_in_milliseconds(&Sys_Global.mp3_sounds[next_slot], 0.0f, GetMusicVolume(), fade_in_ms);
    ma_sound_start(&Sys_Global.mp3_sounds[next_slot]);
    Sys_Global.mp3_slot = next_slot;
}


void play_wav(const char* path, float volume, Vector3 pos, bool positional) {
    int32_t slot = -1;
    for (int32_t i = 0; i < wav_count; i++) { // Try to find a free slot (either unused or finished)
        if (!ma_sound_is_playing(&wav_sounds[i]) && ma_sound_at_end(&wav_sounds[i])) {
            ma_sound_uninit(&wav_sounds[i]);
            slot = i;
            break;
        }
    }

    if (slot == -1 && wav_count < MAX_CHANNELS) slot = wav_count++; // If no free slot, use a new one if available
    if (slot == -1) { DualLog("WARNING: Max effect WAV channels (%d) reached\n", MAX_CHANNELS); return; }

    ma_result result = ma_sound_init_from_file(&Sys_Global.audio_engine, path, 0, NULL, NULL, &wav_sounds[slot]);
    if (result != MA_SUCCESS) {
        DualLog("ERROR: Failed to load effect WAV %s: %d\n", path, result);
        if (slot == wav_count - 1) wav_count--; // Revert count if init fails
        return;
    }
    
    if (positional) ma_sound_set_position(&wav_sounds[slot], pos.x, pos.y, pos.z);
    ma_sound_set_spatialization_enabled(&wav_sounds[slot], (ma_bool32)positional);
    wav_volumes[slot] = volume;
    ma_sound_set_volume(&wav_sounds[slot], GetSFXVolume(wav_volumes[slot]));
    ma_sound_start(&wav_sounds[slot]);
}

void play_message(const char* path) {
    if (ma_sound_is_playing(&log_sound)) { ma_sound_stop(&log_sound); ma_sound_uninit(&log_sound); }
    ma_result result = ma_sound_init_from_file(&Sys_Global.audio_engine, path, 0, NULL, NULL, &log_sound);
    if (result != MA_SUCCESS) { DualLog("ERROR: Failed to load message WAV %s: %d\n", path, result); return; }
    
    ma_sound_set_spatialization_enabled(&log_sound, false);
    ma_sound_set_volume(&log_sound, GetMessageVolume());
    ma_sound_start(&log_sound);
}

ENGINE_TO_MOD void SoundUninit(ma_sound* snd) { ma_sound_uninit(snd); }
ENGINE_TO_MOD ma_result SoundInit(const char* path, ma_uint32 flags, ma_sound_group* pGroup, ma_fence* pDoneFence, ma_sound* pSound) { return ma_sound_init_from_file(&Sys_Global.audio_engine,path,flags,pGroup,pDoneFence,pSound); }
ENGINE_TO_MOD void SoundSetLooping(ma_sound* pSound, ma_bool32 isLooping) { ma_sound_set_looping(pSound,isLooping); }
ENGINE_TO_MOD ma_result SoundStart(ma_sound* pSound) { return ma_sound_start(pSound); }
ENGINE_TO_MOD ma_result SoundStop(ma_sound* pSound) { return ma_sound_stop(pSound); }

ENGINE_TO_MOD float SoundGetLength(ma_sound* pSound) {
    if (!pSound) return 0.0f;
    
    ma_uint64 frames;
    if (ma_sound_get_length_in_pcm_frames(pSound, &frames) != MA_SUCCESS) return 0.0f;
    
    ma_uint32 sr = ma_engine_get_sample_rate(ma_sound_get_engine(pSound));
    return (sr == 0) ? 0.0f : (float)frames / (float)sr;
}

ENGINE_TO_MOD ma_result SoundGetCurrentFrameCursor(const ma_sound* pSound, ma_uint64* pCursor) { return ma_sound_get_cursor_in_pcm_frames(pSound,pCursor); }
