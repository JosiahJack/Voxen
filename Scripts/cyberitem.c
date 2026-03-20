#include "mod.h"

void CyberItemInitBeforeLoad(uint16_t self) {
    Entity* e = &Eng_Global->instances[self];
    if (Eng_Global->difficultyMission == 0 && e->type == SoftwareType_Data) flag_set(&e->entflags,ENTFLAG_ACTIVE,false);
}

void CyberItemOnTriggerEnter(uint16_t self, uint16_t other) {
    Entity* e = &Eng_Global->instances[self];
    if (other != PLAYER1) return;
    if (!InventoryAddSoftwareItem(e->type,e->version)) return;
    flag_set(&e->entflags,ENTFLAG_ACTIVE,false);
}
