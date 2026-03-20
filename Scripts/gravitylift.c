#include "mod.h"

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
