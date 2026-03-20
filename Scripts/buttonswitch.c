#include "mod.h"

void ButtonSwitchInitAfterLoad(uint16_t self) {
    Entity* e = &Eng_Global->instances[self];
    e->delayFinished = 0.0f;
    if (e->active) e->tickFinished = Eng_Global->pauseRelativeTime + 1.5 + (double)random_range(0.0f,1.0f);
}

void ButtonSwitchToggleLocked(uint16_t self) { Entity* e = &Eng_Global->instances[self]; EntitySetLocked(e,!EntityLocked(e)); }
void ButtonSwitchToggleMaterial(uint16_t self) { Entity* e = &Eng_Global->instances[self]; e->texIndex = e->alternateOn ? e->alternateSwitchMaterial : e->mainSwitchMaterial; }
void ButtonSwitchSetMaterialToAlternate(uint16_t self) { Entity* e = &Eng_Global->instances[self]; if (e->entflags & ENTFLAG_BLINK_TEX_ON_ACTIVE) e->texIndex = e->alternateSwitchMaterial; }
void ButtonSwitchSetMaterialToNormal(uint16_t self) { Entity* e = &Eng_Global->instances[self]; if (e->entflags & ENTFLAG_BLINK_TEX_ON_ACTIVE) e->texIndex = e->mainSwitchMaterial; }

void ButtonSwitchUseTargets(uint16_t self, uint16_t activator, const char* argvalue) {
    Entity* e = &Eng_Global->instances[self];
    UseTargets(activator,argvalue,e->target);
    e->active = !e->active;
    e->alternateOn = e->active;
    if (e->entflags & ENTFLAG_CHANGE_TEX_ON_ACTIVE) {
        ButtonSwitchToggleMaterial(self);
        if ((e->entflags & ENTFLAG_BLINK_TEX_ON_ACTIVE) && e->active) e->tickFinished = Eng_Global->pauseRelativeTime + 1.5f;
    }
}

void ButtonSwitchUse(uint16_t self, uint16_t activator, const char* argvalue) {
    Entity* e = &Eng_Global->instances[self];
    if (Eng_Cheats->superoverride || Eng_Global->difficultyMission == 0) EntitySetLocked(e,false);
    else if (GetCurrentLevelSecurity() > e->securityThreshold) { UIBlockedBySecurity(e->position); return; }
    if (EntityLocked(e)) {
        CenterStatusPrint("%s",Eng_Text->stringTable[e->lockedMessageLingdex]);
        if (e->SFXLockedIndex >= 0 && e->SFXLockedIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXLockedIndex],1.0f,e->position,true);
        return;
    }
    if (e->SFXIndex >= 0 && e->SFXIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXIndex],1.0f,e->position,true);
    CenterStatusPrint("%s",Eng_Text->stringTable[e->messageIndex]);
    if (e->delay > 0.0f) { e->recentMostActivator = activator; e->delayFinished = Eng_Global->pauseRelativeTime + e->delay; }
    else ButtonSwitchUseTargets(self,activator,argvalue);
}

void ButtonSwitchUpdate(uint16_t self) {
    Entity* e = &Eng_Global->instances[self];
    if (e->delayFinished > 0.0 && e->delayFinished < Eng_Global->pauseRelativeTime) { e->delayFinished = 0.0; ButtonSwitchUseTargets(self,e->recentMostActivator,e->argvalue); }
    if ((e->entflags & ENTFLAG_BLINK_TEX_ON_ACTIVE) && e->active && e->tickFinished < Eng_Global->pauseRelativeTime) {
        e->alternateOn = !e->alternateOn;
        ButtonSwitchToggleMaterial(self);
        e->tickFinished = Eng_Global->pauseRelativeTime + e->tickTime;
    }
}

void ButtonSwitchTargetted(uint16_t self, uint16_t activator, const char* argvalue) { ButtonSwitchUse(self,activator,argvalue); }
