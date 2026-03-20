#include "mod.h"

void CyberMineInitBeforeLoad(uint16_t self) {
    Entity* e = &Eng_Global->instances[self];
    e->damage = 55.0f;
    if (Eng_Global->difficultyCyber < 3) { if (random_range(0.0f,1.0f) < 0.2f) flag_set(&e->entflags,ENTFLAG_ACTIVE,false); e->damage = 33.0f; }
    if (Eng_Global->difficultyCyber < 2) { if (random_range(0.0f,1.0f) < 0.33f) flag_set(&e->entflags,ENTFLAG_ACTIVE,false); e->damage = 22.0f; }
    if (Eng_Global->difficultyCyber < 1) { if (random_range(0.0f,1.0f) < 0.50f) flag_set(&e->entflags,ENTFLAG_ACTIVE,false); e->damage = 11.0f; }
}

void CyberMineOnTriggerEnter(uint16_t self, uint16_t other) {
    Entity* e = &Eng_Global->instances[self];
    if (other != PLAYER1) return;
    PlayerTakeDamage(PLAYER1,e->damage);
    play_wav(sounds[67],1.0f,e->position,false);
    flag_set(&e->entflags,ENTFLAG_ACTIVE,false);
}
