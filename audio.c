// audio.c
#include "voxen.h"
#include "miniaudio.h"
#define MAX_CHANNELS 16
#define MAX_AMBIENT_NOISES 128
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

bool GetSoundIsPlaying(ma_sound* sound) { return ma_sound_is_playing(sound); }
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
    const Vector3* player = &Sys_Global.instances[PLAYER1].position;
    const float max_range = 7.68f;
    const float max_range_sq = max_range * max_range;
    for (uint16_t i = 0; i < loadedAmbients; ++i) {
        const uint16_t ent_idx = ambientRegistry[i];
        const Entity* ent = &Sys_Global.instances[ent_idx];
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
                ma_result r = ma_sound_init_from_file(&Sys_Global.audio_engine, path, MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, NULL, &slot->sound);
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
    for (uint16_t i = START_INDEX_LEVEL_INSTANCES; i<Sys_Global.loadedInstances;++i) {
        if (ConstIndexIsAmbient(Sys_Global.instances[i].index)) {
            ambientRegistry[loadedAmbients] = i;
            loadedAmbients++;
            if (loadedAmbients >= MAX_AMBIENT_NOISES) { DualLogError("%u exceeded max number of ambient noises %u!\n",loadedAmbients,MAX_AMBIENT_NOISES); break; }
            
            Sys_Global.instances[i].volume = Sys_Global.entities[Sys_Global.instances[i].index].volume * 0.5f;
        }
    }
}
