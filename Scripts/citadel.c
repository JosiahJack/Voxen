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

void CyberIceOnTriggerEnter(uint16_t self, uint16_t other) {
    (void)self;
    Entity* e = &Eng_Global->instances[other];
    if (!(e->entflags & ENTFLAG_RIGIDBODY)) return;
    e->layer = 24;
    e->velocity = scale_vector3(e->velocity,-1.0f);
}

void CyberMineInitBeforeLoad(uint16_t self) {
    Entity* e = &Eng_Global->instances[self];
    e->damage = 55.0f;
    if (Eng_Global->difficultyCyber < 3) { if (random_range(0.0f,1.0f) < 0.2f) flag_set(&e->entflags,ENTFLAG_ACTIVE,false); e->damage = 33.0f; }
    if (Eng_Global->difficultyCyber < 2) { if (random_range(0.0f,1.0f) < 0.33f) flag_set(&e->entflags,ENTFLAG_ACTIVE,false); e->damage = 22.0f; }
    if (Eng_Global->difficultyCyber < 1) { if (random_range(0.0f,1.0f) < 0.50f) flag_set(&e->entflags,ENTFLAG_ACTIVE,false); e->damage = 11.0f; }
}

void CyberMineOnTriggerEnter(uint16_t self, uint16_t other) {
    Entity* e = &Eng_Global->instances[self];
    if (other != PLAYER1) return;
    PlayerTakeDamage(PLAYER1,e->damage);
    play_wav(sounds[67],1.0f,e->position,false);
    flag_set(&e->entflags,ENTFLAG_ACTIVE,false);
}


void CyberPushOnTriggerStay(uint16_t self, uint16_t other) {
    Entity* e = &Eng_Global->instances[self];
    Entity* player = &Eng_Global->instances[PLAYER1];
    if (Eng_Global->difficultyCyber < 1 || other != PLAYER1) return;
    player->inCyberTube = true;
    AddForce(PLAYER1,scale_vector3(e->direction,e->force * (float)Eng_Global->deltaTime),false);
    Sys_Music.cyberTube = true;
}

void CyberPushOnTriggerExit(uint16_t self, uint16_t other) {
    (void)self;
    if (other != PLAYER1) return;
    Eng_Global->instances[PLAYER1].inCyberTube = false;
    Sys_Music.cyberTube = false;
}

void CyberDoorOnCollisionEnter(uint16_t self, uint16_t other) {
    Entity* e = &Eng_Global->instances[self];
    if (!e->isDoor || (other != PLAYER1 && other != PLAYER2)) return;
    CenterStatusPrint("%s  %s",Eng_Text->stringTable[e->messageIndex],Eng_Text->stringTable[601]);
}


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

void FuncWallInitAfterLoad(uint16_t self) {
    Entity* e = &Eng_Global->instances[self];
    Vector3 tempVec = Vector3_A_minus_B(e->position,e->targetPosition);
    float distTotal = distance_vector3(e->startPosition,e->targetPosition);
    tempVec = scale_vector3(normalize_vector3(tempVec),-1.0f);
    if (e->funcState == FuncStates_AjarMovingTarget) tempVec = scale_vector3(tempVec,distTotal * e->percentAjar);
    else if (e->funcState == FuncStates_AjarMovingStart) tempVec = scale_vector3(tempVec,distTotal * (1.0f - e->percentAjar));
    else if (e->funcState == FuncStates_MovingStart) tempVec = scale_vector3(tempVec,distTotal * (1.0f - e->percentMoved));
    else tempVec = scale_vector3(tempVec,distTotal * e->percentMoved);
    e->position = Vector3_A_plus_B(e->position,tempVec);
}

void FuncWallMoveStart(uint16_t self) { Eng_Global->instances[self].funcState = FuncStates_MovingStart; Eng_Global->instances[self].tickFinished = Eng_Global->pauseRelativeTime + 10.0f; }
void FuncWallMoveTarget(uint16_t self) { Eng_Global->instances[self].funcState = FuncStates_MovingTarget; Eng_Global->instances[self].tickFinished = Eng_Global->pauseRelativeTime + 10.0f; }

void FuncWallTargetted(uint16_t self, uint16_t activator, const char* argvalue) {
    (void)activator; (void)argvalue;
    Entity* e = &Eng_Global->instances[self];
    if (e->funcState == FuncStates_Start || e->funcState == FuncStates_MovingStart || e->funcState == FuncStates_AjarMovingTarget) FuncWallMoveTarget(self);
    else FuncWallMoveStart(self);
    play_wav(sounds[76],1.0f,e->position,true);
    flag_set(&e->entflags,ENTFLAG_STOPSOUND_PLAYED,false);
}

void FuncWallUpdate(uint16_t self) {
    Entity* e = &Eng_Global->instances[self];
    Vector3 goal = e->funcState == FuncStates_MovingStart ? e->startPosition : e->targetPosition;
    FuncStates doneState = e->funcState == FuncStates_MovingStart ? FuncStates_Start : FuncStates_Target;
    if (e->funcState == FuncStates_Start) { e->position = e->startPosition; e->velocity = (Vector3){0.0f,0.0f,0.0f}; e->percentMoved = 0.0f; return; }
    if (e->funcState == FuncStates_Target) { e->position = e->targetPosition; e->velocity = (Vector3){0.0f,0.0f,0.0f}; e->percentMoved = 1.0f; return; }
    if (e->funcState != FuncStates_MovingStart && e->funcState != FuncStates_MovingTarget) return;
    Vector3 delta = Vector3_A_minus_B(goal,e->position);
    float distanceLeft = magnitude_vector3(delta);
    float total = distance_vector3(e->startPosition,e->targetPosition);
    float dist = e->speed * (float)Eng_Global->deltaTime;
    if (distanceLeft <= dist || e->tickFinished < Eng_Global->pauseRelativeTime) {
        e->position = goal;
        e->funcState = doneState;
        e->percentMoved = doneState == FuncStates_Target ? 1.0f : 0.0f;
        e->velocity = (Vector3){0.0f,0.0f,0.0f};
        return;
    }
    if (distanceLeft > 0.0001f) e->position = Vector3_A_plus_B(e->position,scale_vector3(normalize_vector3(delta),dist));
    if (total > 0.0001f) e->percentMoved = distance_vector3(e->startPosition,e->position) / total;
}

void ForceBridgeInitBeforeLoad(uint16_t self) {
    Entity* e = &Eng_Global->instances[self];
    e->tickTime = 0.05f;
    e->tickFinished = Eng_Global->pauseRelativeTime + e->tickTime + (double)random_range(0.0f,1.0f);
    e->lerping = true;
    if (e->activatedScale.x <= 0.02f) e->activatedScale.x = 2.56f;
    if (e->activatedScale.y <= 0.02f) e->activatedScale.y = 0.08f;
    if (e->activatedScale.z <= 0.02f) e->activatedScale.z = 2.56f;
}

void ForceBridgeInitAfterLoad(uint16_t self) {
    Entity* e = &Eng_Global->instances[self];
    if (!(e->entflags & ENTFLAG_ACTIVATED)) {
        flag_set(&e->entflags,ENTFLAG_VISIBLE,false);
        e->collider = COLLIDER_TYPE_NONE;
    }
    switch (e->fieldColor) {
        case ForceFieldColor_Red:      e->texIndex = 38; break;
        case ForceFieldColor_Green:    e->texIndex = 40; break;
        case ForceFieldColor_Blue:     e->texIndex = 39; break;
        case ForceFieldColor_Purple:   e->texIndex = 41; break;
        case ForceFieldColor_RedFaint: e->texIndex = 198; break;
    }
}

void ForceBridgeActivate(uint16_t self, bool isSilent) {
    Entity* e = &Eng_Global->instances[self];
    if (e->entflags & ENTFLAG_ACTIVATED) return;
    if (!isSilent) play_wav(sounds[102],1.0f,e->position,true);
    flag_set(&e->entflags,ENTFLAG_VISIBLE,true);
    flag_set(&e->entflags,ENTFLAG_ACTIVATED,true);
    e->lerping = true;
    e->collider = COLLIDER_TYPE_BOX;
    e->scale = (Vector3){ e->forceFieldDirectionX ? 0.1f : e->activatedScale.x, e->forceFieldDirectionY ? 0.1f : e->activatedScale.y, e->forceFieldDirectionZ ? 0.1f : e->activatedScale.z };
}

void ForceBridgeDeactivate(uint16_t self, bool isSilent) {
    Entity* e = &Eng_Global->instances[self];
    if (!(e->entflags & ENTFLAG_ACTIVATED)) return;
    if (!isSilent) play_wav(sounds[102],1.0f,e->position,true);
    flag_set(&e->entflags,ENTFLAG_ACTIVATED,false);
    e->lerping = true;
}

void ForceBridgeToggle(uint16_t self) {
    if (Eng_Global->instances[self].entflags & ENTFLAG_ACTIVATED) ForceBridgeDeactivate(self,false);
    else ForceBridgeActivate(self,false);
}

void ForceBridgeUpdate(uint16_t self) {
    Entity* e = &Eng_Global->instances[self];
    if (Eng_Global->gamePaused || Eng_Global->menuActive || e->tickFinished >= Eng_Global->pauseRelativeTime) return;
    e->tickFinished = Eng_Global->pauseRelativeTime + e->tickTime;
    if (e->entflags & ENTFLAG_ACTIVATED) {
        if (!e->lerping) return;
        float sx = e->forceFieldDirectionX ? lerp(e->scale.x,e->activatedScale.x,e->tickTime * 2.0f) : e->scale.x;
        float sy = e->forceFieldDirectionY ? lerp(e->scale.y,e->activatedScale.y,e->tickTime * 2.0f) : e->scale.y;
        float sz = e->forceFieldDirectionZ ? lerp(e->scale.z,e->activatedScale.z,e->tickTime * 2.0f) : e->scale.z;
        e->scale = (Vector3){sx,sy,sz};
        if (vabs(e->activatedScale.x - sx) < 0.08f && vabs(e->activatedScale.y - sy) < 0.08f && vabs(e->activatedScale.z - sz) < 0.08f) { e->scale = e->activatedScale; e->lerping = false; }
    } else if (e->lerping) {
        float sx = e->forceFieldDirectionX ? lerp(e->scale.x,0.0f,e->tickTime * 2.0f) : e->scale.x;
        float sy = e->forceFieldDirectionY ? lerp(e->scale.y,0.0f,e->tickTime * 2.0f) : e->scale.y;
        float sz = e->forceFieldDirectionZ ? lerp(e->scale.z,0.0f,e->tickTime * 2.0f) : e->scale.z;
        e->scale = (Vector3){sx,sy,sz};
        if (sx < 0.08f || sy < 0.08f || sz < 0.08f) { flag_set(&e->entflags,ENTFLAG_ACTIVE,false); e->collider = COLLIDER_TYPE_NONE; e->lerping = false; }
    }
}

static uint16_t TeleportTouch_allTeleportTouches[8];
static bool TeleportTouch_initialized;
void TeleportTouchInitAfterLoad(uint16_t self) {
    Entity* e = &Eng_Global->instances[self];
    if (!TeleportTouch_initialized) { for (uint8_t i = 0; i < 8; i++) TeleportTouch_allTeleportTouches[i] = UINT16_MAX; TeleportTouch_initialized = true; }
    if (e->teleportID >= 8) { DeleteInstance(self); return; }
    TeleportTouch_allTeleportTouches[e->teleportID] = self;
}

void TeleportTouchOnTriggerEnter(uint16_t self, uint16_t other) {
    Entity* e = &Eng_Global->instances[self];
    Entity* player = &Eng_Global->instances[PLAYER1];
    if (!e->touchEnabled || other != PLAYER1) return;
    if (player->health <= 0.0f || e->justUsed >= Eng_Global->pauseRelativeTime) return;
    uint16_t dest = e->targetDestinationID < 8 ? TeleportTouch_allTeleportTouches[e->targetDestinationID] : UINT16_MAX;
    if (dest == UINT16_MAX) return;
    player->position = Eng_Global->instances[dest].position;
    Eng_Global->instances[dest].justUsed = Eng_Global->pauseRelativeTime + 1.0;
    play_wav(sounds[106],1.0f,Eng_Global->instances[dest].position,false);
}

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

void TriggerCounterTarget(uint16_t self, uint16_t activator, const char* argvalue) { (void)argvalue; UseTargets(activator,Eng_Global->instances[self].argvalue,Eng_Global->instances[self].target); }
void TriggerCounterDelayedTarget(uint16_t self, uint16_t activator, const char* argvalue) { Eng_Global->instances[self].delayFinished = Eng_Global->pauseRelativeTime + Eng_Global->instances[self].delay; TriggerCounterTarget(self,activator,argvalue); }

void TriggerCounterTargetted(uint16_t self, uint16_t activator, const char* argvalue) {
    Entity* e = &Eng_Global->instances[self];
    e->counter++;
    if (e->counter != e->countToTrigger) return;
    if (e->delay <= 0.0f) TriggerCounterTarget(self,activator,argvalue); else TriggerCounterDelayedTarget(self,activator,argvalue);
    if (!e->dontReset) e->counter = 0;
}

void TextureChangerInitAfterLoad(uint16_t self) {
    Entity* e = &Eng_Global->instances[self];
    if (!e->currentTexture) return;
    e->texIndex = e->altTexIndex;
    if (e->altGlowIndex < MAX_VALID_TEXTURE) e->glowIndex = e->altGlowIndex;
}

void TextureChangerToggle(uint16_t self) {
    Entity* e = &Eng_Global->instances[self];
    if (e->currentTexture) {
        e->texIndex = Eng_Global->entities[e->index].texIndex;
        e->glowIndex = Eng_Global->entities[e->index].glowIndex;
    } else {
        e->texIndex = e->altTexIndex;
        if (e->altGlowIndex < MAX_VALID_TEXTURE) e->glowIndex = e->altGlowIndex;
    }
    e->currentTexture = !e->currentTexture;
}

void GravityLiftInitAfterLoad(uint16_t self) {
    Entity* e = &Eng_Global->instances[self];
    if (e->strength <= 0.0f) e->strength = 12.0f;
    if (e->offStrengthFactor <= 0.0f) e->offStrengthFactor = 0.3f;
    if (e->distancePaddingToTopPoint <= 0.0f) e->distancePaddingToTopPoint = 0.32f;
    e->topPoint = (Vector3){ 0.0f, e->position.y + (e->colliderSize.y * 0.5f), 0.0f };
}

void GravityLiftOnTriggerExit(uint16_t self, uint16_t other) {
    (void)self;
    if (other == PLAYER1) flag_set(&Eng_Global->instances[PLAYER1].entflags,ENTFLAG_GRAV_LIFT_STATE,false);
}

void GravityLiftOnForce(uint16_t self, uint16_t other, bool initial) {
    Entity* e = &Eng_Global->instances[self];
    Entity* o = &Eng_Global->instances[other];
    if (other == PLAYER1) flag_set(&Eng_Global->instances[PLAYER1].entflags,ENTFLAG_GRAV_LIFT_STATE,true);
    float topY = e->position.y + (e->colliderSize.y * 0.5f);
    float dist = topY - o->position.y + 0.48f;
    float velY = o->velocity.y < 0.0f ? 0.0f : o->velocity.y;
    if (dist < e->distancePaddingToTopPoint) AddForce(other,(Vector3){0.0f,9.81f - velY,0.0f},false); // TODO accel-vs-force parity
    else if (o->velocity.y < (e->strength * o->mass)) {
        float yForce = (e->strength * o->mass) - o->velocity.y;
        if (initial || e->initialBurstFinished > Eng_Global->pauseRelativeTime) yForce *= 2.0f;
        AddForce(other,(Vector3){0.0f,yForce,0.0f},false);
    }
}

void GravityLiftOffForce(uint16_t self, uint16_t other, bool initial) {
    Entity* e = &Eng_Global->instances[self];
    Entity* o = &Eng_Global->instances[other];
    if (other == PLAYER1) flag_set(&Eng_Global->instances[PLAYER1].entflags,ENTFLAG_GRAV_LIFT_STATE,true);
    if (o->velocity.y < e->offStrengthFactor) {
        float yForce = e->offStrengthFactor - o->velocity.y;
        if (initial || e->initialBurstFinished > Eng_Global->pauseRelativeTime) yForce *= 2.0f;
        AddForce(other,(Vector3){0.0f,yForce,0.0f},false);
    }
}

void GravityLiftOnTriggerEnter(uint16_t self, uint16_t other) {
    Eng_Global->instances[self].initialBurstFinished = Eng_Global->pauseRelativeTime + 1.0f;
    if (Eng_Global->instances[self].active) GravityLiftOnForce(self,other,true);
    else GravityLiftOffForce(self,other,true);
}

void GravityLiftOnTriggerStay(uint16_t self, uint16_t other) {
    if (Eng_Global->instances[self].active) GravityLiftOnForce(self,other,false);
    else GravityLiftOffForce(self,other,false);
}

void GravityLiftToggle(uint16_t self) { Eng_Global->instances[self].active = !Eng_Global->instances[self].active; }

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

void HealingBedUse(uint16_t self, uint16_t owner) {
    Entity* e = &Eng_Global->instances[self];
    if (GetCurrentLevelSecurity() <= (uint8_t)e->minSecurityLevel) {
        if (!e->broken) {
            HealthManagerHealingBed(PLAYER1,e->amount,true);
            CenterStatusPrint("%s",Eng_Text->stringTable[23],owner);
            play_wav(sounds[103],1.0f,e->position,false);
        } else CenterStatusPrint("%s",Eng_Text->stringTable[24],owner);
    } else UIBlockedBySecurity(e->position);
}

void UseTargets(uint16_t activator, const char* argvalue, const char* targetname) {
    bool succeeded = false;
    if (StringIsEmpty(targetname)) return;
    for (uint16_t i = START_INDEX_LEVEL_INSTANCES; i < Eng_Global->loadedInstances; i++) {
        if (!StringsAreEqual(Eng_Global->instances[i].targetname,targetname)) continue;
        Targetted(activator,i,argvalue);
        succeeded = true;
    }
    if (!succeeded) DualLogWarn("Failed to find a matching targetname for %s\n",targetname);
}

void Targetted(uint16_t activator, uint16_t self, const char* argvalue) {
    Entity* e = &Eng_Global->instances[self];
    Entity* a = &Eng_Global->instances[activator];
    if (argvalue && !StringIsEmpty(argvalue)) StringCopyInto_A_From_B(e->argvalue,argvalue,TARGET_STRING_LENGTH);
    if (e->index == 708) { Eng_Global->gameFinished = true; return; }
    if ((a->ioflags & TARG_IOFLAGS_SEND_EMAIL) && EntityDefIs(self,"info_email")) EmailTargetted(self,activator,argvalue);
    if (a->ioflags & TARG_IOFLAGS_TRIPTRIGGER) {
        if (e->index == 598 || e->index == 600) TriggerTargetted(self,activator);
        else if (e->index == 594) TriggerCounterTargetted(self,activator,argvalue);
    }
    if ((a->ioflags & TARG_IOFLAGS_SWITCHTRIGGER) && ConstIndexIsButtonSwitch(e->index)) ButtonSwitchTargetted(self,activator,argvalue);
    if ((a->ioflags & TARG_IOFLAGS_DOOROPEN) && ConstIndexIsDoor(e->index)) DoorForceOpen(self);
    if ((a->ioflags & TARG_IOFLAGS_DOOROPENIFUNLOCKED) && ConstIndexIsDoor(e->index) && !EntityLocked(e) && (e->requiredAccessCard == AccessCardType_None || e->accessCardUsedByPlayer || (Eng_Global->inventoryPlayer1.accessCardOwned & (1u << e->requiredAccessCard)))) DoorForceOpen(self);
    if ((a->ioflags & TARG_IOFLAGS_DOOR_TOGGLE) && ConstIndexIsDoor(e->index)) DoorActuate(self);
    if ((a->ioflags & TARG_IOFLAGS_DOORCLOSE) && ConstIndexIsDoor(e->index)) DoorForceClose(self);
    if ((a->ioflags & TARG_IOFLAGS_DOORLOCK) && ConstIndexIsDoor(e->index)) DoorLock(self);
    if ((a->ioflags & TARG_IOFLAGS_DOORUNLOCK) && ConstIndexIsDoor(e->index)) DoorUnlock(self);
    if ((a->ioflags & TARG_IOFLAGS_TOG_DORACESOVERIDE) && ConstIndexIsDoor(e->index)) DoorToggleAccessCardOverride(self);
    if (a->ioflags & TARG_IOFLAGS_FBRIDGE_ACTIVATE) ForceBridgeActivate(self,false);
    if (a->ioflags & TARG_IOFLAGS_FBRIDGE_DEACTIVATE) ForceBridgeDeactivate(self,false);
    if (a->ioflags & TARG_IOFLAGS_FBRIDGE_TOGGLE) ForceBridgeToggle(self);
    if (a->ioflags & TARG_IOFLAGS_GRAVLIFT_TOGGLE) GravityLiftToggle(self);
    if (a->ioflags & TARG_IOFLAGS_TEXTURE_CHG_TOGGLE) TextureChangerToggle(self);
    if (a->ioflags & TARG_IOFLAGS_FUNCWALL_MOVE) FuncWallTargetted(self,activator,argvalue);
    if (a->ioflags & TARG_IOFLAGS_SWITCH_LOCK_TOGGLE) ButtonSwitchToggleLocked(self);
    if (a->ioflags & TARG_IOFLAGS_UNLOCK_SWITCH) EntitySetLocked(e,false);
    if (a->ioflags & TARG_IOFLAGS_INST_ACTIVATE) flag_set(&e->entflags,ENTFLAG_ACTIVE,true);
    if (a->ioflags & TARG_IOFLAGS_INST_DEACTIVATE) flag_set(&e->entflags,ENTFLAG_ACTIVE,false);
    if (a->ioflags & TARG_IOFLAGS_INST_TOGGLE) flag_set(&e->entflags,ENTFLAG_ACTIVE,!(e->entflags & ENTFLAG_ACTIVE));
}

void VaporizeClick(void) { // TODO
//     Eng_UI->mouseClickHeldOverGUI = true;
//     if (Eng_Global->inventoryPlayer1.generalInvCurrent == 0) return; // Access Cards index.
// 
//     int cur = Eng_Global->inventoryPlayer1.generalInvCurrent;
//     Eng_Global->inventoryPlayer1.generalInventoryIndexRef[cur] = -1; // Remove item
//     Eng_Global->inventoryPlayer1.generalInvCurrent -= 1;
//     if (Eng_Global->inventoryPlayer1.generalInvCurrent < 0) {
//         Eng_Global->inventoryPlayer1.generalInvCurrent = 0; // Bound to lowest, but only
//     }									   // since it is Access Cards.
// 
// 
//     cur = Eng_Global->inventoryPlayer1.generalInvCurrent;
//     if (Eng_Global->inventoryPlayer1.generalInventoryIndexRef[cur] < 0) {
//         for (int i=13; i >= 0; i--) {
//             if (Eng_Global->inventoryPlayer1.generalInventoryIndexRef[i] >= 0) {
//                 Eng_Global->inventoryPlayer1.generalInvCurrent = i;
//                 break; // Found last item in inventory.
//             }
//         }
//     }
// 
//     cur = Eng_Global->inventoryPlayer1.generalInvCurrent;
//     int indexRef = Eng_Global->inventoryPlayer1.generalInventoryIndexRef[cur];
//     if (Eng_Global->inventoryPlayer1.generalInvCurrent == 0) {
//         if (Eng_Global->inventoryPlayer1.HasAnyAccessCards()) {
//             Eng_UI->SendInfoToItemTab(indexRef);
//         } else {
//             // If no access cards, reset item tab to show nothing.
//             Eng_UI->SendInfoToItemTab(-1);
//             PtrExit();
//         }
//     } else {
//         GeneralInvButton genbut = Eng_Global->inventoryPlayer1.genButtons[cur].GetComponent<GeneralInvButton>();
//         Eng_UI->SendInfoToItemTab(indexRef,genbut.customIndex);
//     }
}
