#include "mod.h"

void CyberDecoyEnable(uint16_t self) { (void)self; Eng_Global->decoyActive = true; }
void CyberDecoyDisable(uint16_t self) { (void)self; Eng_Global->decoyActive = false; }

void CyberExitOnTriggerEnter(uint16_t self, uint16_t other) {
    (void)self;
    if (other != PLAYER1) return;
    UIExitCyberspace();
}

void CyberDataFragmentOnTriggerEnter(uint16_t self, uint16_t other) {
    Entity* e = &Eng_Global->instances[self];
    if (other != PLAYER1) return;
    UICyberSprint((uint16_t)e->textIndex);
}

void CyberIceOnTriggerEnter(uint16_t self, uint16_t other) {
    (void)self;
    Entity* e = &Eng_Global->instances[other];
    if (!(e->entflags & ENTFLAG_RIGIDBODY)) return;
    e->layer = 24;
    e->velocity = scale_vector3(e->velocity,-1.0f);
}

void LadderOnTriggerEnter(uint16_t self, uint16_t other) {
    (void)self;
    Entity* player = &Eng_Global->instances[PLAYER1];
    if (other != PLAYER1) return;
    player->ladderState++;
    if (player->ladderState < 1) player->ladderState = 1;
}

void LadderOnTriggerExit(uint16_t self, uint16_t other) {
    (void)self;
    Entity* player = &Eng_Global->instances[PLAYER1];
    if (other != PLAYER1) return;
    player->ladderState--;
    if (player->ladderState < 0) player->ladderState = 0;
}

void SearchFXResetEnable(uint16_t self) {
    Entity* e = &Eng_Global->instances[self];
    if (e->itemLifeTime <= 0.0f) e->itemLifeTime = 3.0f;
    e->delayFinished = Eng_Global->pauseRelativeTime + e->itemLifeTime;
}

void SearchFXResetUpdate(uint16_t self) {
    Entity* e = &Eng_Global->instances[self];
    if (e->delayFinished >= Eng_Global->pauseRelativeTime) return;
    flag_set(&e->entflags,ENTFLAG_ACTIVE,false);
}

void ExplosionLifeInitAfterLoad(uint16_t self) {
    Entity* e = &Eng_Global->instances[self];
    if (e->tickTime <= 0.0f) e->tickTime = 0.05f;
    if (e->delay <= 0.0f) e->delay = 0.8f;
    e->delayFinished = Eng_Global->pauseRelativeTime + e->delay;
}

void ExplosionLifeUpdate(uint16_t self) {
    Entity* e = &Eng_Global->instances[self];
    if (!(e->entflags & ENTFLAG_ACTIVE) || e->delayFinished >= Eng_Global->pauseRelativeTime) return;
    if (e->dontReset) flag_set(&e->entflags,ENTFLAG_ACTIVE,false);
    else DeleteInstance(self);
}

void EmailTargetted(uint16_t self, uint16_t activator, const char* argvalue) {
    (void)activator; (void)argvalue;
    Entity* e = &Eng_Global->instances[self];
    int idx = e->textIndex;
    if (idx < 0 || idx >= TEXT_LOGS_COUNT) return;
    if (Eng_Global->inventoryPlayer1.hasLog[idx]) return;
    Eng_Global->inventoryPlayer1.hasLog[idx] = true;
    Eng_Global->inventoryPlayer1.hasNewEmail = true;
    Eng_Global->inventoryPlayer1.lastAddedIndex = idx;
    if (Eng_Text->audioLogType[idx] == AudioLogType_Email) Eng_Global->inventoryPlayer1.beepDone = true;
    if (e->active) { } // TODO autoplay email/log playback path.
}
