#include "mod.h"

void CyberDoorOnCollisionEnter(uint16_t self, uint16_t other) {
    Entity* e = &Eng_Global->instances[self];
    if (!e->isDoor || (other != PLAYER1 && other != PLAYER2)) return;
    CenterStatusPrint("%s  %s",Eng_Text->stringTable[e->messageIndex],Eng_Text->stringTable[601]);
}
