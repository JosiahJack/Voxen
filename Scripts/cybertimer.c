#include "mod.h"

void CyberTimerInitAfterLoad(uint16_t self) {
    Entity* e = &Eng_Global->instances[self];
    e->cyberTimer = 600.0f;
    e->timerFinished = Eng_Global->pauseRelativeTime + 1.0;
}

void CyberTimerReset(uint16_t self, int diff) {
    Entity* e = &Eng_Global->instances[self];
    switch (diff) {
        case 0: e->cyberTimer = 600.0f; break;
        case 1: e->cyberTimer = 300.0f; break;
        case 2: e->cyberTimer = 240.0f; break;
        case 3: e->cyberTimer = 180.0f; break;
    }
}

void CyberTimerUpdate(uint16_t self) {
    Entity* e = &Eng_Global->instances[self];
    if (Eng_Global->gamePaused || Eng_Global->menuActive) return;
    if (e->cyberTimer <= 0.0f) { UIExitCyberspace(); return; }
    if (e->timerFinished >= Eng_Global->pauseRelativeTime) return;
    
    e->cyberTimer -= 1.0f;
    e->minutes = vfloor(e->cyberTimer / 60.0f);
    e->seconds = e->cyberTimer - (e->minutes * 60.0f);
    e->timerFinished = Eng_Global->pauseRelativeTime + 1.0;
}
