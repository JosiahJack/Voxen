// audio.c
#include "voxen.h"
#include "tables_audio.h"
static inline __attribute__((always_inline, noreturn)) void OS_Exit(int64_t exitCode) {
    #ifdef WINDOWS
        register uint64_t rax __asm__("rax") = 0x2C;
        register HANDLE   rcx __asm__("rcx") = (HANDLE)-1;
        register NTSTATUS rdx __asm__("rdx") = (NTSTATUS)exitCode;
        __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rcx), "r"(rdx) : "r8", "r9", "r10", "r11", "memory");
    #else
        register int64_t rax __asm__("rax") = 231;
        register int64_t rdi __asm__("rdi") = exitCode;
        __asm__ __volatile__("syscall" : "+r"(rax) : "r"(rdi) : "rcx", "r11", "memory");
    #endif
    __builtin_unreachable();
}

#include "./External/miniaudio.h"
#define BUFFER_MS 50
#define AUD_BUFFER_T 0.25f
#define MAX_CHANNELS 16
#define MAX_AMBIENT_NOISES 128
ma_engine audio_engine;
ma_sound mp3_sounds[2]; // For crossfading
int32_t mp3_slot = 0;
ma_sound wav_sounds[MAX_CHANNELS];
float wav_volumes[MAX_CHANNELS]; // Setting independent base sfx volume (e.g. dropped physics object hard or lightly volume, independent of position).
int32_t wav_count = 0;
ma_sound log_sound;
MusicSystem Sys_Music;
// Usage: play_wav("./Audio/cyborgs/yourlevelsareterrible.wav",0.1f); WORKED!

void InitializeAudio(void) {
    double startTime = get_time();
    ma_result result;
    ma_engine_config engine_config = ma_engine_config_init();
    engine_config.channels = 2; // Stereo output, adjust if needed
    result = ma_engine_init(&engine_config, &audio_engine);
    if (result != MA_SUCCESS) DualLog("ERROR: Failed to initialize miniaudio engine: %d\n", result);
    DualLog("Initialize Audio took %f secs\n",get_time() - startTime);
}

void mp3_clear(void) {
    ma_sound_stop(&mp3_sounds[0]);
    ma_sound_stop(&mp3_sounds[1]);
    mp3_slot = 0;
}

float GetSFXVolume(float volume) { return ((float)Sys_Settings.VolumeMaster/100.0f) * ((float)Sys_Settings.VolumeEffects/100.0f) * volume; }
float GetMusicVolume(void) { return ((float)Sys_Settings.VolumeMaster/100.0f) * ((float)Sys_Settings.VolumeMusic/100.0f); }
float GetMessageVolume(void) { return ((float)Sys_Settings.VolumeMaster/100.0f) * ((float)Sys_Settings.VolumeMessage/100.0f); }
void set_music_volume(void) { for (int i=0;i<2;++i) { ma_sound_set_volume(&mp3_sounds[i], GetMusicVolume()); } }
void set_sfx_volume(void) { for (int i=0;i<MAX_CHANNELS;++i) { ma_sound_set_volume(&wav_sounds[i], GetSFXVolume(wav_volumes[i])); } }
void set_message_volume(void) { ma_sound_set_volume(&log_sound, GetMessageVolume()); }
void set_master_volume(void) { set_sfx_volume(); set_music_volume(); set_message_volume(); }

void play_mp3(const char* path, int32_t fade_in_ms) {
    int32_t old_slot = mp3_slot;
    int32_t next_slot = mp3_slot ? 0 : 1;
    if (ma_sound_is_playing(&mp3_sounds[old_slot])) ma_sound_set_fade_in_milliseconds(&mp3_sounds[old_slot], GetMusicVolume(), 0.0f, fade_in_ms);
    ma_sound_uninit(&mp3_sounds[next_slot]); 
    ma_result result = ma_sound_init_from_file(&audio_engine, path, MA_SOUND_FLAG_STREAM, NULL, NULL, &mp3_sounds[next_slot]);
    if (result != MA_SUCCESS) { DualLog("ERROR: Failed to load MP3 %s: %d\n", path, result); return; }

    ma_sound_set_fade_in_milliseconds(&mp3_sounds[next_slot], 0.0f, GetMusicVolume(), fade_in_ms);
    ma_sound_start(&mp3_sounds[next_slot]);
    mp3_slot = next_slot;
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

    ma_result result = ma_sound_init_from_file(&audio_engine, path, 0, NULL, NULL, &wav_sounds[slot]);
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
    ma_result result = ma_sound_init_from_file(&audio_engine, path, 0, NULL, NULL, &log_sound);
    if (result != MA_SUCCESS) { DualLog("ERROR: Failed to load message WAV %s: %d\n", path, result); return; }
    
    ma_sound_set_spatialization_enabled(&log_sound, false);
    ma_sound_set_volume(&log_sound, GetMessageVolume());
    ma_sound_start(&log_sound);
}

// ============================================================================
uint16_t loadedAmbients = 0;
uint16_t ambientRegistry[MAX_AMBIENT_NOISES]; // For ambient_ type entities that play looped sound

typedef struct {
    uint16_t    index;
    const char* filename;          // ./Audio/ambient/…
} AmbientDef;

static const AmbientDef g_ambient_defs[] = {
    {621, "airhiss.wav"},          {622, "clicker.wav"},
    {623, "compressor.wav"},       {624, "dishwasher.wav"},
    {625, "drip_amb.wav"},         {626, "fan1.wav"},
    {627, "generator_gas.wav"},    {628, "gurgle.wav"},
    {629, "icemaker.wav"},         {630, "intake.wav"},
    {631, "lathe.wav"},            {632, "lev3loop1.wav"},
    {633, "lev3loop2.wav"},        {634, "lev3loop3.wav"},
    {635, "lev3loop4.wav"},        {636, "liquid_bubble.wav"},
    {637, "lava2.wav"},            {638, "rain.wav"},
    {639, "machgear_loop.wav"},    {640, "machine_ambience.wav"},
    {641, "machine_go.wav"},       {642, "machine_humamb7.wav"},
    {643, "machine_humlonoise.wav"},{644, "machine_loop1.wav"},
    {645, "machine_loop2.wav"},    {646, "machinea1.wav"},
    {647, "machinevat_loop.wav"},  {648, "mist.wav"},
    {649, "pipewater_loop.wav"},   {650, "powerloom.wav"},
    {651, "pump.wav"},             {652, "pump2.wav"},
    {653, "rain.wav"},             {654, "steam_loop.wav"},
    {655, "washing_machine.wav"},
};
#define AMBIENT_DEF_COUNT  (sizeof(g_ambient_defs)/sizeof(g_ambient_defs[0]))

typedef struct {
    ma_sound  sound;
    ma_bool32 loaded;
    float     length_sec;
} AmbientSlot;

static AmbientSlot ambientSlots[AMBIENT_DEF_COUNT] = {0};

static float ma_sound_get_length_sec(ma_sound* pSound) {
    if (!pSound) return 0.0f;
    
    ma_uint64 frames;
    if (ma_sound_get_length_in_pcm_frames(pSound, &frames) != MA_SUCCESS) return 0.0f;
    
    ma_uint32 sr = ma_engine_get_sample_rate(ma_sound_get_engine(pSound));
    return (sr == 0) ? 0.0f : (float)frames / (float)sr;
}

static const AmbientDef* ambient_def_by_index(uint16_t idx) {
    for (size_t i = 0; i < AMBIENT_DEF_COUNT; ++i) {
        if (g_ambient_defs[i].index == idx) return &g_ambient_defs[i];
    }
    
    return NULL;
}

void UpdateAmbientSounds(void) {
    const Vector3* player = &instances[PLAYER1].position;
    const float max_range = 7.68f;
    const float max_range_sq = max_range * max_range;
    for (uint16_t i = 0; i < loadedAmbients; ++i) {
        const uint16_t ent_idx = ambientRegistry[i];
        const Entity* ent = &instances[ent_idx];
        const AmbientDef* def = ambient_def_by_index(ent->index);
        if (!def) { DualLogError("  [SKIP] Entity %u has unknown index %u\n", ent_idx, ent->index); continue; }

        const float dist_sq = squareDistance3D(player->x, player->y, player->z, ent->position.x, ent->position.y, ent->position.z);
        const float distance = vsqrtf(dist_sq);
        bool in_range = (dist_sq < max_range_sq);
        int32_t subIdx = PosGetCellCoords(ent->position.x, ent->position.z);
        int cellIdx = (playerCellIdx * ARRSIZE);
        int flat_idx = cellIdx + subIdx;
        if (!get_cull_bit(precomputedVisibleCellsFromHere,flat_idx)) in_range = false;
        const size_t slot_idx = (size_t)(def - g_ambient_defs);
        AmbientSlot* slot = &ambientSlots[slot_idx];
        if (in_range) {
            if (!slot->loaded) {
                char path[512];
                StringFormat(path, sizeof(path), "./Audio/ambient/%s", def->filename);
                ma_sound_uninit(&slot->sound);
                ma_result r = ma_sound_init_from_file(&audio_engine, path, MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, NULL, &slot->sound);
                if (r != MA_SUCCESS) continue;

                slot->length_sec = ma_sound_get_length_sec(&slot->sound);
                if (slot->length_sec <= 0.0f) { ma_sound_uninit(&slot->sound); continue; }

                ma_sound_set_looping(&slot->sound, MA_TRUE);
                slot->loaded = MA_TRUE;
            }

            if (!ma_sound_is_playing(&slot->sound)) ma_sound_start(&slot->sound);

            // Time sync
            if (slot->length_sec > 0.0f) {
                ma_uint64 cur;
                ma_sound_get_cursor_in_pcm_frames(&slot->sound, &cur);
            }

            // Volume
            float vol_factor = (distance <= 1.0f) ? 1.0f
                               : (distance >= max_range) ? 0.0f
                                 : (max_range - distance) / (max_range - 1.0f);
                                 
            float final_vol = ent->volume * vol_factor;
            ma_sound_set_volume(&slot->sound, final_vol);
        } else {
            if (ma_sound_is_playing(&slot->sound)) ma_sound_stop(&slot->sound);
        }
    }
}

void ResetLevelAudio(void) {
    loadedAmbients = 0;
    __builtin_memset(ambientRegistry, 0, loadedAmbients * sizeof(uint16_t));
    for (uint16_t i = START_INDEX_LEVEL_INSTANCES; i<loadedInstances;++i) {
        if (ConstIndexIsAmbient(instances[i].index)) {
            ambientRegistry[loadedAmbients] = i;
            loadedAmbients++;
            if (loadedAmbients >= MAX_AMBIENT_NOISES) { DualLogError("%u exceeded max number of ambient noises %u!\n",loadedAmbients,MAX_AMBIENT_NOISES); OS_Exit(1); }
            
            instances[i].volume = entities[instances[i].index].volume * 0.5f;
        }
    }
    
    Sys_Music.levelEntry = true;             Sys_Music.inZone = Sys_Music.cyberTube = false;
    Sys_Music.clipFinished = Sys_Music.combatImpulseFinished = get_time(); Sys_Music.combatImpulseFinished += 5.0;
}

void PlayMenuMusic(void) { mp3_clear(); play_mp3("./Audio/music/TITLOOP-00_menu.mp3",1500); }
void PlayGameMusic(void) { mp3_clear(); play_mp3("./Audio/music/THM1-19_medicalstart.mp3",100); }

const char* GetCorrespondingLevelClip(TrackType ttype) {
    switch(ttype) { // Override types, return from these first before special level handling
        case TrackType_Revive:     return levelMusicRevive[Sys_Global.currentLevel];
        case TrackType_Death:      return levelMusicDeath[Sys_Global.currentLevel];
        case TrackType_Elevator:   return levelMusicElevator[Sys_Global.currentLevel];
        case TrackType_Distortion: return levelMusicDistortion[Sys_Global.currentLevel];
    }

    if (Sys_Global.currentLevel == 0 || Sys_Global.currentLevel == 5 || Sys_Global.currentLevel == 7) { // 0  REACTOR, 5 FLIGHT, 7 ENGINEERING
        if (Sys_Music.levelEntry)      return reactorMusic[6];
        if (ttype == TrackType_Combat) return reactorMusic[random_range_u8(0,6)];
        return reactorMusic[random_range_u8(6,13)];
    } else if (Sys_Global.currentLevel == 1) { // 1  MEDICAL
        if (Sys_Music.levelEntry) return medicalMusic[0];
        if (ttype == TrackType_Combat) return medicalMusic[random_range_u8(5,11)];
        return medicalMusic[random_range_u8(1,5)];
    } else if (Sys_Global.currentLevel == 2 || Sys_Global.currentLevel == 4) { // 2  SCIENCE, 4 STORAGE
        if (Sys_Music.levelEntry)      return scienceMusic[0];
        if (ttype == TrackType_Combat) return scienceMusic[random_range_u8(8,10)];
        return scienceMusic[random_range_u8(1,8)];
    } else if (Sys_Global.currentLevel == 8) { // 8 SECURITY
        if (Sys_Music.levelEntry)      return securityMusic[9];
        if (ttype == TrackType_Combat) return securityMusic[random_range_u8(0,6)];
        return securityMusic[random_range_u8(6,19)];
    } else if (Sys_Global.currentLevel == 6) { // 6 EXECUTIVE
        if (Sys_Music.levelEntry)      return executiveMusic[0];
        if (ttype == TrackType_Combat) return executiveMusic[random_range_u8(9,13)];
        return executiveMusic[random_range_u8(0,10)];
    } else if (Sys_Global.currentLevel == 10 || Sys_Global.currentLevel == 11 || Sys_Global.currentLevel == 12) { // 10, 12 GROVES
        if (Sys_Music.levelEntry)      return groveMusic[19];
        if (ttype == TrackType_Combat) return groveMusic[random_range_u8(0,9)];
        return executiveMusic[random_range_u8(9,24)];
    } else if (Sys_Global.currentLevel == 13) { // 13 CYBERSPACE
        if (Sys_Music.levelEntry)           return cyberMusic[0];
        if (Sys_Music.cyberTube)            return cyberMusic[random_range_u8(4,8)];
        if (random_range(0.0f,1.0f) < 0.5f) return cyberMusic[random_range_u8(1,5)];
        else                                return cyberMusic[8];
    }

    return levelMusicLooped[0];
}

void PlayTrack(TrackType ttype, MusicType mtype) {
    if (!Sys_Settings.DynamicMusic) { // Looped Music (Dynamic Music off)
        if (mtype == MusicType_Override) {
                 if (ttype == TrackType_Revive)     play_mp3(levelMusicRevive[Sys_Global.currentLevel],0);
            else if (ttype == TrackType_Death)      play_mp3(levelMusicDeath[Sys_Global.currentLevel],0);
            else if (ttype == TrackType_Elevator)   play_mp3(levelMusicElevator[Sys_Global.currentLevel],0);
            else if (ttype == TrackType_Distortion) play_mp3(levelMusicDistortion[Sys_Global.currentLevel],0);
        } else play_mp3(levelMusicLooped[Sys_Global.currentLevel],0);
        
        return;
    }

    // Normal Dynamic Music System
    if (mtype == MusicType_Override) mp3_clear();
    play_mp3(GetCorrespondingLevelClip(ttype),BUFFER_MS);
    if (!Sys_Music.elevator) Sys_Music.levelEntry = false; // already used by GetCorresponding... just now
}

void MusicNotifyZone(TrackType tt) {
    Sys_Music.inZone = true;
    switch(tt) {
        case TrackType_Elevator: Sys_Music.elevator = true; break;
        case TrackType_Distortion: Sys_Music.distortion = true; break;
    }
}

void MusicTriggerEnter(uint16_t self, uint16_t other) {
    if (instances[self].tickFinished < Sys_Global.pauseRelativeTime) { // Prevent flickering retrigger when player slides along glancing angle of trigger volume.
        if (other == PLAYER1 || other == PLAYER2) {
            PlayTrack(instances[self].trackType,instances[self].musicType);
            MusicNotifyZone(instances[self].trackType);
        }
        
        instances[self].tickFinished = Sys_Global.pauseRelativeTime + 0.1;
    }
}

void MusicTriggerExit(uint16_t other) {
    if (other == PLAYER1 || other == PLAYER2) { mp3_clear(); Sys_Music.inZone = Sys_Music.elevator = Sys_Music.distortion = false; } // return to normal upon leaving the trigger
}

void UpdateMusic(void) {
    ma_sound* curr = mp3_slot ? &mp3_sounds[1] : &mp3_sounds[0];
    bool currentIsPlaying = ma_sound_is_playing(curr);
    if (currentIsPlaying) {
        ma_uint64 currentFrame = ma_sound_get_time_in_pcm_frames(curr);
        ma_uint64 pcmFramesLength = 0;
        ma_sound_get_length_in_pcm_frames(curr,&pcmFramesLength);
        uint64_t deltaFrames = pcmFramesLength - currentFrame;
        float remaining = deltaFrames != 0 ? (float)deltaFrames / (float)ma_engine_get_sample_rate(&audio_engine) : 0.0f;
        if (remaining > AUD_BUFFER_T) return;
    }

    if (Sys_Music.inCombat && !Sys_Music.inZone && Sys_Music.combatImpulseFinished < Sys_Global.pauseRelativeTime) {
        Sys_Music.inCombat = false;
        PlayTrack(TrackType_Combat, MusicType_Override);
        Sys_Music.combatImpulseFinished = Sys_Global.pauseRelativeTime + 20.0;
        return;
    }

    if (Sys_Music.inZone) {
        if (Sys_Music.distortion) { PlayTrack(TrackType_Distortion, MusicType_Override); return; }
        if (Sys_Music.elevator) { PlayTrack(TrackType_Elevator, MusicType_Override); return; }
    }
    
    if (Sys_Settings.DynamicMusic) {
        if (currentIsPlaying) {
            ma_uint64 currentFrame = ma_sound_get_time_in_pcm_frames(curr);
            ma_uint64 pcmFramesLength = 0;
            ma_sound_get_length_in_pcm_frames(curr,&pcmFramesLength);
            uint64_t deltaFrames = pcmFramesLength - currentFrame;
            float remaining = deltaFrames != 0 ? (float)deltaFrames / (float)ma_engine_get_sample_rate(&audio_engine) : 0.0f;
            if (remaining <= AUD_BUFFER_T) PlayTrack(TrackType_Walking, MusicType_Walking);
        } else PlayTrack(TrackType_Walking, MusicType_Walking);
    } else PlayTrack(TrackType_Walking, MusicType_Walking);
}
