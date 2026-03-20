#include "mod.h"

void CyberPushOnTriggerStay(uint16_t self, uint16_t other) {
    Entity* e = &Eng_Global->instances[self];
    Entity* player = &Eng_Global->instances[PLAYER1];
    if (Eng_Global->difficultyCyber < 1 || other != PLAYER1) return;
    player->inCyberTube = true;
    AddForce(PLAYER1,scale_vector3(e->direction,e->force * (float)Eng_Global->deltaTime),false);
    Sys_Music.cyberTube = true;
}

void CyberPushOnTriggerExit(uint16_t self, uint16_t other) {
    (void)self;
    if (other != PLAYER1) return;
    Eng_Global->instances[PLAYER1].inCyberTube = false;
    Sys_Music.cyberTube = false;
}
