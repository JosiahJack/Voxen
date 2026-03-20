#include "mod.h"

void TriggerCounterTarget(uint16_t self, uint16_t activator, const char* argvalue) { (void)argvalue; UseTargets(activator,Eng_Global->instances[self].argvalue,Eng_Global->instances[self].target); }
void TriggerCounterDelayedTarget(uint16_t self, uint16_t activator, const char* argvalue) { Eng_Global->instances[self].delayFinished = Eng_Global->pauseRelativeTime + Eng_Global->instances[self].delay; TriggerCounterTarget(self,activator,argvalue); }

void TriggerCounterTargetted(uint16_t self, uint16_t activator, const char* argvalue) {
    Entity* e = &Eng_Global->instances[self];
    e->counter++;
    if (e->counter != e->countToTrigger) return;
    if (e->delay <= 0.0f) TriggerCounterTarget(self,activator,argvalue); else TriggerCounterDelayedTarget(self,activator,argvalue);
    if (!e->dontReset) e->counter = 0;
}
