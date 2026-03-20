#include "mod.h"

void CyberSwitchInitAfterLoad(uint16_t self) {
    Entity* e = &Eng_Global->instances[self];
    if (e->iceActive) flag_set(&e->entflags,ENTFLAG_ACTIVE,true); // TODO Visual subobject parity removed with hierarchy removal.
}

void CyberSwitchOnTriggerEnter(uint16_t self, uint16_t other) {
    Entity* e = &Eng_Global->instances[self];
    if (e->active || other != PLAYER1) return;
    UICyberSprint((uint16_t)e->textIndex);
    e->active = true;
    UseTargets(other,e->argvalue,e->target);
}
