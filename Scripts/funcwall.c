#include "mod.h"

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
