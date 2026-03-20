#include "mod.h"

void LogicTimerInitBeforeLoad(uint16_t self) {
    Entity* e = &Eng_Global->instances[self];
    if (e->timeInterval <= 0.0f) e->timeInterval = 0.35f;
    if (e->randomMin <= 0.0f) e->randomMin = 5.0f;
    if (e->randomMax <= 0.0f) e->randomMax = 10.0f;
    e->intervalFinished = Eng_Global->pauseRelativeTime + (e->useRandomTimes ? (double)random_range(e->randomMin,e->randomMax) : (double)e->timeInterval);
}

void LogicTimerUseTargets(uint16_t self) { UseTargets(self,Eng_Global->instances[self].argvalue,Eng_Global->instances[self].target); }

void LogicTimerUpdate(uint16_t self) {
    Entity* e = &Eng_Global->instances[self];
    if (Eng_Global->gamePaused || Eng_Global->menuActive || !e->active || e->intervalFinished >= Eng_Global->pauseRelativeTime) return;
    e->intervalFinished = Eng_Global->pauseRelativeTime + (e->useRandomTimes ? (double)random_range(e->randomMin,e->randomMax) : (double)e->timeInterval);
    LogicTimerUseTargets(self);
}

void LogicTimerTargetted(uint16_t self, uint16_t activator, const char* argvalue) { (void)activator; (void)argvalue; Eng_Global->instances[self].active = !Eng_Global->instances[self].active; }
