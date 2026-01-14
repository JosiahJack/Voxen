// audio.c
#include <string.h>
#include <stdio.h>
// #include "os.h"
#include "./External/miniaudio.h"
// #include "voxen.h"

#define MAX_CHANNELS 16
#define MAX_AMBIENT_NOISES 128
ma_engine audio_engine;
ma_sound mp3_sounds[2]; // For crossfading
ma_sound wav_sounds[MAX_CHANNELS];
int32_t wav_count = 0;
// Usage: play_mp3("./Audio/music/looped/track1.mp3",0.08f,0);  WORKED! play_wav("./Audio/cyborgs/yourlevelsareterrible.wav",0.1f); WORKED!
//        play_mp3("./Audio/music/TITLOOP-00_menu.mp3",((float)Sys_Settings.VolumeMusic/100.0f) * 0.4f + 0.09f,1500);
//        play_mp3("./Audio/music/THM1-19_medicalstart.mp3",((float)Sys_Settings.VolumeMusic/100.0f) * 0.4f,100);

void InitializeAudio(void) {
    ma_result result;
    ma_engine_config engine_config = ma_engine_config_init();
    engine_config.channels = 2; // Stereo output, adjust if needed
    result = ma_engine_init(&engine_config, &audio_engine);
    if (result != MA_SUCCESS) { DualLog("ERROR: Failed to initialize miniaudio engine: %d\n", result); OS_Exit(1); }
}

void play_mp3(const char* path, float volume, int32_t fade_in_ms) {
    static int32_t current_sound = 0;
    ma_sound_uninit(&mp3_sounds[current_sound]);
    ma_result result = ma_sound_init_from_file(&audio_engine, path, MA_SOUND_FLAG_STREAM, NULL, NULL, &mp3_sounds[current_sound]);
    if (result != MA_SUCCESS) { DualLog("ERROR: Failed to load MP3 %s: %d\n", path, result);  return; }
    
    ma_sound_set_fade_in_milliseconds(&mp3_sounds[current_sound], 0.0f, volume, fade_in_ms);
    ma_sound_start(&mp3_sounds[current_sound]);
    current_sound = 1 - current_sound; // Toggle for crossfade
}

void play_wav(const char* path, float volume) {
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

inline void UpdateAmbientSounds(void) {
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
                snprintf(path, sizeof(path), "./Audio/ambient/%s", def->filename);
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
    memset(ambientRegistry, 0, loadedAmbients * sizeof(uint16_t));
    for (uint16_t i = START_INDEX_LEVEL_INSTANCES; i<loadedInstances;++i) {
        uint16_t entIdx = instances[i].index;
        if (ConstIndexIsAmbient(entIdx)) {
            ambientRegistry[loadedAmbients] = i;
            loadedAmbients++;
            if (loadedAmbients >= MAX_AMBIENT_NOISES) { DualLogError("%u exceeded max number of ambient noises %u!\n",loadedAmbients,MAX_AMBIENT_NOISES); OS_Exit(1); }
            
            instances[i].volume = entities[entIdx].volume * 0.5f;
        }
    }
}
