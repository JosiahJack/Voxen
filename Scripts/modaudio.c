// modaudio.c
#include "mod.h"
#include "tables_audio.h"
#define MAXAMB 256
static u16 ambs = 0;
typedef struct { ma_sound sound; u32 loaded; float length_sec; } AmbientSlot;
typedef struct { u16 index; const char* filename; } AmbientDef;
u16 ambReg[MAXAMB]; // For ambient_ type entities that play looped sound
static AmbientSlot ambientSlots[MAXAMB] = {0};
static const AmbientDef ambientSounds[MAXAMB] = {
    {621,"airhiss.wav"},        {622,"clicker.wav"},  {623,"compressor.wav"},    {624,"dishwasher.wav"},{625,"drip_amb.wav"},{626,"fan1.wav"},         {627,"generator_gas.wav"},   {628,"gurgle.wav"},    {629,"icemaker.wav"},       {630,"intake.wav"},            {631,"lathe.wav"},        {632,"lev3loop1.wav"},    {633,"lev3loop2.wav"},
    {634,"lev3loop3.wav"},      {635,"lev3loop4.wav"},{636,"liquid_bubble.wav"}, {637,"lava2.wav"},     {638,"rain.wav"},    {639,"machgear_loop.wav"},{640,"machine_ambience.wav"},{641,"machine_go.wav"},{642,"machine_humamb7.wav"},{643,"machine_humlonoise.wav"},{644,"machine_loop1.wav"},{645,"machine_loop2.wav"},{646,"machinea1.wav"},
    {647,"machinevat_loop.wav"},{648,"mist.wav"},     {649,"pipewater_loop.wav"},{650,"powerloom.wav"}, {651,"pump.wav"},    {652,"pump2.wav"},        {653,"rain.wav"},            {654,"steam_loop.wav"},{655,"washing_machine.wav"}};
MOD_TO_ENGINE void MixAmbs(void) {    
    for (u16 i=0;i<ambs;++i) {
        Entity* ent = &World->instances[ambReg[i]];
        const AmbientDef* def = NULL; for (size_t j=0;j<MAXAMB;++j) { if (ambientSounds[j].index==ent->index) {def = &ambientSounds[j]; break; } }
        
        float d = V3_Dist(World->instances[PLAYER1].position,ent->position);
        AmbientSlot* slot = &ambientSlots[(size_t)(def - ambientSounds)];
        if (d < 7.68f && PositionVisibleFromPlayerCell(ent->position.x,ent->position.z)) {
            if (!slot->loaded) {
                SndUninit(&slot->sound);
                char path[512]; sFormat(path,sizeof(path),"./Audio/ambient/%s",def->filename);
                int r = SndInit(path,&slot->sound); if(r != 0){continue;}
                slot->length_sec = SndLen(&slot->sound); if(slot->length_sec <= 0.0f) {SndUninit(&slot->sound); continue;}

                SoundSetLooping(&slot->sound,1);
                slot->loaded = 1;
            }

            if (!SndPlaying(&slot->sound)) SndStart(&slot->sound);
            if (slot->length_sec > 0.0f) { u64 cur; SndFrmCurpos(&slot->sound,&cur); } // Time sync
            float final_vol = ent->volume * ((d <= 1.0f) ? 1.0f : (d >= 7.68f) ? 0.0f : (7.68f - d) / (7.68f - 1.0f));
            SndSetVolume(&slot->sound,final_vol);
        } else if (SndPlaying(&slot->sound)) SndStop(&slot->sound);
    }
}
 
MOD_TO_ENGINE void ResetLevelAudio(void) { ambs=0; mset(ambReg,0,ambs * sizeof(u16)); for (u16 i = INSTS_1ST_IDX; i<World->instCount;++i) { if(IdxIsAmbient(World->instances[i].index)){ambReg[ambs]=i; ambs++; if(ambs >= MAXAMB){DualLogError("Ambient noises %u > %u!\n",ambs,MAXAMB); break;} World->instances[i].volume=EDefs[World->instances[i].index].volume * 0.5f;} } }
