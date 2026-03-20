#include "mod.h"

void DelayedSpawnEnable(uint16_t self) {
    Entity* e = &Eng_Global->instances[self];
    e->timerFinished = Eng_Global->pauseRelativeTime + e->delay;
    e->active = true;
}

void DelayedSpawnUpdate(uint16_t self) {
    Entity* e = &Eng_Global->instances[self];
    if (!e->active || e->timerFinished >= Eng_Global->pauseRelativeTime) return;
    e->active = false;
    for (uint8_t i = 0; i < MAX_CHILD_COUNT; i++) {
        uint16_t child = e->child[i];
        if (child == UINT16_MAX) continue; // TODO child[] is the DelayedSpawn object list replacement in the C port.
        flag_set(&Eng_Global->instances[child].entflags,ENTFLAG_ACTIVE,!e->despawnInstead);
    }
    if (!e->doSelfAfterList) return;
    if (e->despawnInstead) {
        if (e->destroyAfterListInsteadOfDeactivate) DeleteInstance(self);
        else flag_set(&e->entflags,ENTFLAG_ACTIVE,false);
    } else flag_set(&e->entflags,ENTFLAG_ACTIVE,true);
}
