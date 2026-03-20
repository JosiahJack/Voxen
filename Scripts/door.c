#include "mod.h"
enum { DOOR_CLIP_IDLE_CLOSED = 0, DOOR_CLIP_OPENING = 1, DOOR_CLIP_IDLE_OPEN = 2, DOOR_CLIP_CLOSING = 3 };

static AnimationClip DoorGetClip(const Entity* e, uint8_t clip) { return modelAnimationClips[e->animationNum][clip]; }
static float DoorClamp01(float v) { if (v < 0.0f) return 0.0f; if (v > 1.0f) return 1.0f; return v; }
static bool DoorInventoryHasAccessCard(AccessCardType card) { return card == AccessCardType_None || (Eng_Global->inventoryPlayer1.accessCardOwned & (1u << card)); }
static bool DoorIsOpenish(const Entity* e) { return e->doorOpen == DoorState_Open || e->doorOpen == DoorState_Opening; }

static float DoorGetProgress(const Entity* e, uint8_t clip) {
    AnimationClip c = DoorGetClip(e,clip);
    if (c.frameEnd <= c.frameStart) return 1.0f;
    return DoorClamp01((float)(e->frame - c.frameStart) / (float)(c.frameEnd - c.frameStart));
}

static uint16_t DoorFrameFromProgress(AnimationClip c, float t) {
    if (c.frameEnd <= c.frameStart) return c.frameStart;
    uint16_t span = c.frameEnd - c.frameStart;
    return (uint16_t)(c.frameStart + (uint16_t)(DoorClamp01(t) * (float)span));
}

static void DoorSetClipFrame(uint16_t self, uint8_t clip, uint16_t frame) {
    Entity* e = &Eng_Global->instances[self];
    AnimationClip c = DoorGetClip(e,clip);
    if (c.framerate == 0) return;
    if (frame < c.frameStart) frame = c.frameStart;
    if (frame > c.frameEnd) frame = c.frameEnd;
    e->clip = clip;
    e->frame = frame;
    e->modelIndex = c.frameStartModelIndex + (frame - c.frameStart);
    e->currentFrameFinished = Eng_Global->current_time + ((1.0 / (double)c.speed) * (1.0 / (double)c.framerate));
}

static void DoorSyncLayer(uint16_t self) {
    Entity* e = &Eng_Global->instances[self];
    if (!e->changeLayerOnOpenClose) return;
    e->layer = DoorIsOpenish(e) ? PhysicsLayer_InterDebris : PhysicsLayer_Door;
}

static void DoorOpen(uint16_t self) {
    Entity* e = &Eng_Global->instances[self];
    DoorSetClipFrame(self,DOOR_CLIP_OPENING,DoorGetClip(e,DOOR_CLIP_OPENING).frameStart);
    e->doorOpen = e->doorState = DoorState_Opening;
    e->waitBeforeClose = Eng_Global->pauseRelativeTime + e->delay;
    DoorSyncLayer(self);
    if (e->SFXIndex >= 0 && e->SFXIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXIndex],1.0f,e->position,true);
}

static void DoorClose(uint16_t self) {
    Entity* e = &Eng_Global->instances[self];
    DoorSetClipFrame(self,DOOR_CLIP_CLOSING,DoorGetClip(e,DOOR_CLIP_CLOSING).frameStart);
    e->doorOpen = e->doorState = DoorState_Closing;
    DoorSyncLayer(self);
    if (e->SFXIndex >= 0 && e->SFXIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXIndex],1.0f,e->position,true);
}

void DoorLock(uint16_t self) { EntitySetLocked(&Eng_Global->instances[self],true); }
void DoorUnlock(uint16_t self) { Entity* e = &Eng_Global->instances[self]; EntitySetLocked(e,false); e->accessCardUsedByPlayer = true; }
void DoorToggleLocked(uint16_t self) { if (EntityLocked(&Eng_Global->instances[self])) DoorUnlock(self); else DoorLock(self); }
void DoorToggleAccessCardOverride(uint16_t self) { Eng_Global->instances[self].accessCardUsedByPlayer = !Eng_Global->instances[self].accessCardUsedByPlayer; }

void DoorForceOpen(uint16_t self) {
    if (Eng_Global->instances[self].doorOpen == DoorState_Open) return;
    DoorOpen(self);
}

void DoorForceClose(uint16_t self) {
    if (Eng_Global->instances[self].doorOpen == DoorState_Closed) return;
    DoorClose(self);
}

void DoorActuate(uint16_t self) {
    Entity* e = &Eng_Global->instances[self];
    if (e->doorOpen == DoorState_Open) { DoorClose(self); return; }
    if (e->doorOpen == DoorState_Closed) { DoorOpen(self); return; }
    if (e->doorOpen == DoorState_Opening) {
        float t = DoorGetProgress(e,DOOR_CLIP_OPENING);
        AnimationClip c = DoorGetClip(e,DOOR_CLIP_CLOSING);
        DoorSetClipFrame(self,DOOR_CLIP_CLOSING,DoorFrameFromProgress(c,1.0f - t));
        e->doorOpen = e->doorState = DoorState_Closing;
        DoorSyncLayer(self);
        if (e->SFXIndex >= 0 && e->SFXIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXIndex],1.0f,e->position,true);
        return;
    }
    if (e->doorOpen == DoorState_Closing) {
        float t = DoorGetProgress(e,DOOR_CLIP_CLOSING);
        AnimationClip c = DoorGetClip(e,DOOR_CLIP_OPENING);
        DoorSetClipFrame(self,DOOR_CLIP_OPENING,DoorFrameFromProgress(c,1.0f - t));
        e->doorOpen = e->doorState = DoorState_Opening;
        e->waitBeforeClose = Eng_Global->pauseRelativeTime + e->delay;
        DoorSyncLayer(self);
        if (e->SFXIndex >= 0 && e->SFXIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXIndex],1.0f,e->position,true);
    }
}

void DoorInitAfterLoad(uint16_t self) {
    Entity* e = &Eng_Global->instances[self];
    if (e->requiredAccessCard == AccessCardType_None) e->accessCardUsedByPlayer = true;
    if (e->startOpen) e->stayOpen = true;
    if (e->useTimeDelay <= 0.0f) e->useTimeDelay = 0.15f;
    if (e->lockedMessageLingdex <= 0) e->lockedMessageLingdex = 3;
    if (e->SFXIndex < 0) e->SFXIndex = 75;
    if (e->doorOpen > DoorState_Opening) e->doorOpen = e->startOpen ? DoorState_Open : DoorState_Closed;
    e->doorState = e->doorOpen;
    if (e->ajar) {
        AnimationClip c = DoorGetClip(e,DOOR_CLIP_OPENING);
        DoorSetClipFrame(self,DOOR_CLIP_OPENING,DoorFrameFromProgress(c,e->ajarPercentage));
        e->doorOpen = e->doorState = DoorState_Opening;
        DoorSyncLayer(self);
        return;
    }
    switch (e->doorOpen) {
        case DoorState_Open:    DoorSetClipFrame(self,DOOR_CLIP_IDLE_OPEN,DoorGetClip(e,DOOR_CLIP_IDLE_OPEN).frameStart); break;
        case DoorState_Opening: DoorSetClipFrame(self,DOOR_CLIP_OPENING,DoorFrameFromProgress(DoorGetClip(e,DOOR_CLIP_OPENING),e->animatorPlaybackTime)); break;
        case DoorState_Closing: DoorSetClipFrame(self,DOOR_CLIP_CLOSING,DoorFrameFromProgress(DoorGetClip(e,DOOR_CLIP_CLOSING),e->animatorPlaybackTime)); break;
        default:                DoorSetClipFrame(self,DOOR_CLIP_IDLE_CLOSED,DoorGetClip(e,DOOR_CLIP_IDLE_CLOSED).frameStart); break;
    }
    DoorSyncLayer(self);
}

void DoorUse(uint16_t self, uint16_t activator, const char* argvalue) {
    (void)argvalue;
    Entity* e = &Eng_Global->instances[self];
    if (activator == NULLENT) return;
    if (GetCurrentLevelSecurity() > e->securityThreshold) { UIBlockedBySecurity(e->position); return; }
    if (Eng_Cheats->superoverride || Eng_Global->difficultyMission <= 0) {
        EntitySetLocked(e,false);
        e->requiredAccessCard = AccessCardType_None;
        e->accessCardUsedByPlayer = true;
    }
    if (Eng_Global->difficultyMission <= 1) {
        e->requiredAccessCard = AccessCardType_None;
        e->accessCardUsedByPlayer = true;
    }
    if (e->useFinished >= Eng_Global->pauseRelativeTime) return;
    e->useFinished = Eng_Global->pauseRelativeTime + e->useTimeDelay;
    if (e->requiredAccessCard != AccessCardType_None && !e->accessCardUsedByPlayer && !DoorInventoryHasAccessCard(e->requiredAccessCard)) {
        CenterStatusPrint("%s",Eng_Text->stringTable[2]); // TODO Access-card-specific status text.
        if (e->SFXLockedIndex >= 0 && e->SFXLockedIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXLockedIndex],0.7f,e->position,true);
        return;
    }
    if (EntityLocked(e)) {
        if (e->requiredAccessCard != AccessCardType_None && DoorInventoryHasAccessCard(e->requiredAccessCard)) {
            e->accessCardUsedByPlayer = true; // TODO Access-card granted status text.
            return;
        }
        CenterStatusPrint("%s",Eng_Text->stringTable[e->lockedMessageLingdex]);
        if (e->SFXLockedIndex >= 0 && e->SFXLockedIndex < SOUNDS_COUNT) play_wav(sounds[e->SFXLockedIndex],0.55f,e->position,true);
        return;
    }
    if (e->requiredAccessCard != AccessCardType_None && DoorInventoryHasAccessCard(e->requiredAccessCard)) e->accessCardUsedByPlayer = true;
    if ((e->onlyTargetOnce && !e->targetAlreadyDone) || !e->onlyTargetOnce) {
        e->targetAlreadyDone = true;
        UseTargets(activator,e->argvalue,e->target);
    }
    if (e->ajar) e->ajar = false;
    DoorActuate(self);
}

void DoorTargetted(uint16_t self, uint16_t activator, const char* argvalue) {
    (void)argvalue;
    if (EntityLocked(&Eng_Global->instances[self])) DoorUnlock(self);
    if (!Eng_Global->instances[self].targettingOnlyUnlocks) DoorUse(self,activator,argvalue);
}

void DoorUpdate(uint16_t self) {
    Entity* e = &Eng_Global->instances[self];
    if (Eng_Global->gamePaused || Eng_Global->menuActive) return;
    if (e->blocked) return; // TODO frame-pause blocked doors instead of fully skipping.
    if (e->ajar) return;
    AnimationClip opening = DoorGetClip(e,DOOR_CLIP_OPENING);
    AnimationClip closing = DoorGetClip(e,DOOR_CLIP_CLOSING);
    if (e->doorOpen == DoorState_Opening && e->clip == DOOR_CLIP_OPENING && e->frame >= opening.frameEnd) {
        e->doorOpen = e->doorState = DoorState_Open;
        DoorSetClipFrame(self,DOOR_CLIP_IDLE_OPEN,DoorGetClip(e,DOOR_CLIP_IDLE_OPEN).frameStart);
        DoorSyncLayer(self);
    } else if (e->doorOpen == DoorState_Closing && e->clip == DOOR_CLIP_CLOSING && e->frame >= closing.frameEnd) {
        e->doorOpen = e->doorState = DoorState_Closed;
        DoorSetClipFrame(self,DOOR_CLIP_IDLE_CLOSED,DoorGetClip(e,DOOR_CLIP_IDLE_CLOSED).frameStart);
        DoorSyncLayer(self);
    }
    if (Eng_Global->pauseRelativeTime > e->waitBeforeClose && e->doorOpen == DoorState_Open && !e->stayOpen && !e->startOpen) DoorClose(self);
}
