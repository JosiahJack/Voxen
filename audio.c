// audio.c - Audio System
#define MINIAUDIO_IMPLEMENTATION
#define MA_ENABLE_ONLY_SPECIFIC_BACKENDS
#define MA_NO_FLAC
#define MA_NO_SNDIO
#define MA_NO_OSS
#define MA_NO_NULL
#define MA_NO_AUDIO4
#define MA_NO_WEBAUDIO
#define MA_NO_CUSTOM
#define MA_NO_AAUDIO
#define MA_NO_COREAUDIO
#define MA_NO_JACK
#define MA_NO_WINMM
#define MA_NO_DSOUND
#define MA_NO_PULSEAUDIO
#define MA_NO_GENERATION
#define MA_NO_VFS
// #define MA_NO_RESOURCE_MANAGER
// #define MA_NO_NODE_GRAPH
// #define MA_NO_ENGINE
#ifndef _WIN32
    #define MA_NO_WASAPI
    #define MA_ENABLE_ALSA
#else
    #define MA_ENABLE_WASAPI
    #define MA_NO_ALSA
#endif
#define MA_NO_OPENSL
#define MA_NO_AVX2
#define MA_NO_NEON
#define MA_NO_ENCODING
#define MA_NO_PTHREAD_IN_HEADER
// #define MA_NO_DEVICE_IO
// #define MA_NO_THREADING
#include "os.h"
#include "voxen.h"
#include "miniaudio.h"
ma_engine audio_engine; ma_sound wav_sounds[MAX_CHANNELS];
i32 wav_count = 0; float wav_volumes[MAX_CHANNELS]; // Setting independent base sfx volume (e.g. dropped physics object hard or lightly volume, independent of position).
ma_sound log_sound;
i32 mp3_slot; ma_sound mp3_sounds[2]; // Two for crossfading
ENGINE_TO_MOD void MP3Swap(void) { mp3_slot = mp3_slot ? 0 : 1; }
ENGINE_TO_MOD bool GetSoundIsPlaying(ma_sound* sound) { return ma_sound_is_playing(sound); }
float GetMP3RemainingTime(void) {
    ma_sound* pSound = &mp3_sounds[mp3_slot];
    if (!pSound || !ma_sound_is_playing(pSound)) return 0.0f;

    ma_uint64 currentFrame = ma_sound_get_time_in_pcm_frames(pSound);
    ma_uint64 pcmFramesLength = 0;
    ma_sound_get_length_in_pcm_frames(pSound, &pcmFramesLength);
    if (currentFrame >= pcmFramesLength) return 0.0f;

    u64 deltaFrames = pcmFramesLength - currentFrame;
    u32 sampleRate = ma_engine_get_sample_rate(&audio_engine);
    return (float)deltaFrames / (float)sampleRate;
}

void mp3_clear(void) {
    ma_sound_stop(&mp3_sounds[0]);
    ma_sound_stop(&mp3_sounds[1]);
    mp3_slot = 0;
}

ENGINE_TO_MOD void SoundSetVolume(ma_sound* pSound, float volume) { ma_sound_set_volume(pSound,volume); }
float GetSFXVolume(float volume) { return ((float)Sys_Settings.VolumeMaster/100.0f) * ((float)Sys_Settings.VolumeEffects/100.0f) * volume; }
float GetMusicVolume(void) { return ((float)Sys_Settings.VolumeMaster/100.0f) * ((float)Sys_Settings.VolumeMusic/100.0f); }
float GetMessageVolume(void) { return ((float)Sys_Settings.VolumeMaster/100.0f) * ((float)Sys_Settings.VolumeMessage/100.0f); }
void set_music_volume(void) { for (int i=0;i<2;++i) { ma_sound_set_volume(&mp3_sounds[i], GetMusicVolume()); } }
void set_sfx_volume(void) { for (int i=0;i<MAX_CHANNELS;++i) { ma_sound_set_volume(&wav_sounds[i], GetSFXVolume(wav_volumes[i])); } }
void set_message_volume(void) { ma_sound_set_volume(&log_sound, GetMessageVolume()); }
void set_master_volume(void) { set_sfx_volume(); set_music_volume(); set_message_volume(); }
void play_mp3(const char* path, i32 fade_in_ms) {
    i32 old_slot = mp3_slot;
    i32 next_slot = mp3_slot ? 0 : 1;
    if (ma_sound_is_playing(&mp3_sounds[old_slot])) ma_sound_set_fade_in_milliseconds(&mp3_sounds[old_slot], GetMusicVolume(), 0.0f, fade_in_ms);
    ma_sound_uninit(&mp3_sounds[next_slot]); 
    i32 result = ma_sound_init_from_file(&audio_engine,path,MA_SOUND_FLAG_STREAM,NULL,NULL,&mp3_sounds[next_slot]);
    if (result != 0) { DualLog("ERROR: Failed to load MP3 %s: %d\n",path,result); return; }

    ma_sound_set_fade_in_milliseconds(&mp3_sounds[next_slot], 0.0f, GetMusicVolume(), fade_in_ms);
    ma_sound_start(&mp3_sounds[next_slot]);
    mp3_slot = next_slot;
}

ENGINE_TO_MOD void play_wav(const char* path, float volume, Vector3 pos, bool positional) {
    i32 slot = -1;
    for (i32 i = 0; i < wav_count; i++) { // Try to find a free slot (either unused or finished)
        if (!ma_sound_is_playing(&wav_sounds[i]) && ma_sound_at_end(&wav_sounds[i])) { ma_sound_uninit(&wav_sounds[i]); slot = i; break; }
    }

    if (slot == -1 && wav_count < MAX_CHANNELS) slot = wav_count++; // If no free slot, use a new one if available
    if (slot == -1) { DualLog("WARNING: Max effect WAV channels (%d) reached\n", MAX_CHANNELS); return; }

    i32 result = ma_sound_init_from_file(&audio_engine, path, 0, NULL, NULL, &wav_sounds[slot]);
    if (result != 0) { DualLog("ERROR: Failed to load effect WAV %s: %d\n", path, result); if (slot == wav_count - 1) {wav_count--;} return; } // Revert count if init fails
    
    if (positional) ma_sound_set_position(&wav_sounds[slot], pos.x, pos.y, pos.z);
    ma_sound_set_spatialization_enabled(&wav_sounds[slot], (ma_bool32)positional);
    wav_volumes[slot] = volume;
    ma_sound_set_volume(&wav_sounds[slot], GetSFXVolume(wav_volumes[slot]));
    ma_sound_start(&wav_sounds[slot]);
}

ENGINE_TO_MOD void play_message(const char* path) {
    if (ma_sound_is_playing(&log_sound)) { ma_sound_stop(&log_sound); ma_sound_uninit(&log_sound); }
    i32 result = ma_sound_init_from_file(&audio_engine,path,0,NULL,NULL,&log_sound);
    if (result != 0) { DualLog("ERROR: Failed to load message WAV %s: %d\n",path,result); return; }
    
    ma_sound_set_spatialization_enabled(&log_sound, false);
    ma_sound_set_volume(&log_sound, GetMessageVolume());
    ma_sound_start(&log_sound);
}

ENGINE_TO_MOD void SoundUninit(ma_sound* snd) { ma_sound_uninit(snd); }
ENGINE_TO_MOD i32 SoundInit(const char* path, ma_sound* pSound) { return ma_sound_init_from_file(&audio_engine,path,MA_SOUND_FLAG_DECODE|MA_SOUND_FLAG_NO_SPATIALIZATION,NULL,NULL,(ma_sound*)pSound); }
ENGINE_TO_MOD void SoundSetLooping(ma_sound* pSound, ma_bool32 isLooping) { ma_sound_set_looping((ma_sound*)pSound,isLooping); }
ENGINE_TO_MOD i32 SoundStart(ma_sound* pSound) { return ma_sound_start((ma_sound*)pSound); }
ENGINE_TO_MOD i32 SoundStop(ma_sound* pSound) { return ma_sound_stop((ma_sound*)pSound); }
ENGINE_TO_MOD i32 SoundGetCurrentFrameCursor(const ma_sound* pSound, u64* pCursor) { return ma_sound_get_cursor_in_pcm_frames((ma_sound*)pSound,pCursor); }
ENGINE_TO_MOD float SoundGetLength(ma_sound* pSound) {
    if (!(ma_sound*)pSound) return 0.0f;
    u64 frames; if (ma_sound_get_length_in_pcm_frames(pSound, &frames) != 0) return 0.0f;
    
    u32 sr = ma_engine_get_sample_rate(ma_sound_get_engine((ma_sound*)pSound));
    return (sr == 0) ? 0.0f : (float)frames / (float)sr;
}

void SetPlayerListenerOrientation(void) {
    ma_engine_listener_set_direction(&audio_engine,0,Sys_Global.instances[PLAYER1].forward.x,Sys_Global.instances[PLAYER1].forward.y,Sys_Global.instances[PLAYER1].forward.z);
    Vector3 up = cross_vector3(Sys_Global.instances[PLAYER1].forward,Sys_Global.instances[PLAYER1].right);
    ma_engine_listener_set_world_up(&audio_engine,0,up.x,up.y,up.z);
}

void SetPlayerListenerPos(void) {
    ma_engine_listener_set_position(&audio_engine,0,Sys_Global.instances[PLAYER1].position.x,Sys_Global.instances[PLAYER1].position.y,Sys_Global.instances[PLAYER1].position.z);
}

void InitAudio(void) {
    ma_engine_config engine_config = ma_engine_config_init();
    engine_config.channels = 2; engine_config.periodSizeInMilliseconds = 10; engine_config.periodSizeInFrames = 512;
    i32 result = ma_engine_init(&engine_config, &audio_engine); if (result != 0) DualLog("ERROR: Failed to initialize miniaudio engine: %d\n",result);
}
