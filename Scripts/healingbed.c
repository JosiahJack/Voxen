#include "mod.h"

void HealingBedUse(uint16_t self, uint16_t owner) {
    Entity* e = &Eng_Global->instances[self];
    if (GetCurrentLevelSecurity() <= (uint8_t)e->minSecurityLevel) {
        if (!e->broken) {
            HealthManagerHealingBed(PLAYER1,e->amount,true);
            CenterStatusPrint("%s",Eng_Text->stringTable[23],owner);
            play_wav(sounds[103],1.0f,e->position,false);
        } else CenterStatusPrint("%s",Eng_Text->stringTable[24],owner);
    } else UIBlockedBySecurity(e->position);
}
