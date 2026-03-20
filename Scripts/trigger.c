#include "mod.h"

void TriggerUseTargets(uint16_t self, uint16_t activator) { UseTargets(activator,Eng_Global->instances[self].argvalue,Eng_Global->instances[self].target); }
void TriggerDelayedTarget(uint16_t self, uint16_t activator) { Eng_Global->instances[self].delayFireFinished = Eng_Global->pauseRelativeTime + Eng_Global->instances[self].delay; TriggerUseTargets(self,activator); }

void TriggerTriggerTripped(uint16_t self, uint16_t other, bool initialEntry) {
    Entity* e = &Eng_Global->instances[self];
    if (other != PLAYER1 && other != PLAYER2) return;
    if (e->recentMostActivator && e->ignoreSecondaryTriggers) return;
    e->recentMostActivator = other;
    if (initialEntry) e->numPlayers++;
    if (e->onlyOnce) e->allDone = true;
    if (e->delay <= 0.0f) TriggerUseTargets(self,other); else TriggerDelayedTarget(self,other);
}

void TriggerOnTriggerEnter(uint16_t self, uint16_t other) { if (!Eng_Global->instances[self].allDone) TriggerTriggerTripped(self,other,true); }
void TriggerOnTriggerStay(uint16_t self, uint16_t other) { if (!Eng_Global->instances[self].allDone) TriggerTriggerTripped(self,other,false); }
void TriggerOnTriggerExit(uint16_t self, uint16_t other) { if (!Eng_Global->instances[self].allDone && (other == PLAYER1 || other == PLAYER2)) Eng_Global->instances[self].numPlayers--; }
void TriggerTargetted(uint16_t self, uint16_t activator) { if (Eng_Global->instances[self].ignoreSecondaryTriggers) Eng_Global->instances[self].recentMostActivator = activator; }
