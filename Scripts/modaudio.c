// modaudio.c
#include "mod.h"
#include "tables_audio.h"
#define MAX_AMBIENT_NOISES 80 // Equal to number used
u16 loadedAmbients = 0;
typedef struct { ma_sound sound; u32 loaded; float length_sec; } AmbientSlot;
typedef struct { u16 index; const char* filename; } AmbientDef;
u16 ambientRegistry[MAX_AMBIENT_NOISES]; // For ambient_ type entities that play looped sound
static AmbientSlot ambientSlots[MAX_AMBIENT_NOISES] = {0};

static const AmbientDef g_ambient_defs[MAX_AMBIENT_NOISES] = {
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
    {655, "washing_machine.wav"}
};

static const AmbientDef* ambient_def_by_index(u16 idx) {
    for (size_t i = 0; i < MAX_AMBIENT_NOISES; ++i) {
        if (g_ambient_defs[i].index == idx) return &g_ambient_defs[i];
    }
    
    return NULL;
}

MOD_TO_ENGINE void UpdateAmbientSounds(void) {
    const Vector3* player = &Eng_Global->instances[PLAYER1].position;
    const float max_range = 7.68f, max_range_sq = 7.68f * 7.68f;
    for (u16 i = 0; i < loadedAmbients; ++i) {
        const u16 ent_idx = ambientRegistry[i];
        const Entity* ent = &Eng_Global->instances[ent_idx];
        const AmbientDef* def = ambient_def_by_index(ent->index);
        if (!def) { DualLogError("  [SKIP] Entity %u has unknown index %u\n", ent_idx, ent->index); continue; }

        const float dist_sq = squareDistance3D(player->x, player->y, player->z, ent->position.x, ent->position.y, ent->position.z);
        const float distance = vsqrtf(dist_sq);
        bool in_range = (dist_sq < max_range_sq) && PositionVisibleFromPlayerCell(ent->position.x,ent->position.z);
        const size_t slot_idx = (size_t)(def - g_ambient_defs);
        AmbientSlot* slot = &ambientSlots[slot_idx];
        if (in_range) {
            if (!slot->loaded) {
                char path[512];
                StringFormat(path, sizeof(path), "./Audio/ambient/%s", def->filename);
                SoundUninit(&slot->sound);
                int r = SoundInit(path,&slot->sound);
                if (r != 0) continue;

                slot->length_sec = SoundGetLength(&slot->sound);
                if (slot->length_sec <= 0.0f) { SoundUninit(&slot->sound); continue; }

                SoundSetLooping(&slot->sound,MA_TRUE);
                slot->loaded = MA_TRUE;
            }

            if (!GetSoundIsPlaying(&slot->sound)) SoundStart(&slot->sound);
            if (slot->length_sec > 0.0f) { u64 cur; SoundGetCurrentFrameCursor(&slot->sound,&cur); } // Time sync
            float vol_factor = (distance <= 1.0f) ? 1.0f
                               : (distance >= max_range) ? 0.0f
                                 : (max_range - distance) / (max_range - 1.0f); // Volume
                                 
            float final_vol = ent->volume * vol_factor;
            SoundSetVolume(&slot->sound, final_vol);
        } else {
            if (GetSoundIsPlaying(&slot->sound)) SoundStop(&slot->sound);
        }
    }
}

MOD_TO_ENGINE void ResetLevelAudio(void) {
    loadedAmbients = 0;
    MemSetToValueForNBytes(ambientRegistry, 0, loadedAmbients * sizeof(u16));
    for (u16 i = START_INDEX_LEVEL_INSTANCES; i<Eng_Global->loadedInstances;++i) {
        if (ConstIndexIsAmbient(Eng_Global->instances[i].index)) {
            ambientRegistry[loadedAmbients] = i;
            loadedAmbients++;
            if (loadedAmbients >= MAX_AMBIENT_NOISES) { DualLogError("%u exceeded max number of ambient noises %u!\n",loadedAmbients,MAX_AMBIENT_NOISES); break; }
            
            Eng_Global->instances[i].volume = EntityDefinitions[Eng_Global->instances[i].index].volume * 0.5f;
        }
    }
}
