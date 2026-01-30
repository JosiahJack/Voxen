// physics.cpp - Full Jolt Physics integration for Voxen
#include "voxen.h"
#define PLAYER_MAX_WALK_SPEED 3.2f
#define PLAYER_MAX_SPRINT_SPEED 8.8f
#define PLAYER_MAX_CYBER_SPEED 5.0f
#define PLAYER_MAX_CYBER_ULTIMATE_SPEED 12.0f
#define PLAYER_MAX_SPRINT_SPEED_FATIGUED 5.5f
#define PLAYER_MAX_CROUCH_SPEED 1.25f
#define PLAYER_MAX_PRONE_SPEED 0.5f
#define PLAYER_BOOSTER_SPEED_BOOST 1.2f
#define PLAYER_CROUCH_RATIO 0.6f
#define PLAYER_PRONE_RATIO 0.2f
#define PLAYER_TRANSITION_TO_PRONE_ADD 0.1f
#define PLAYER_RADIUS 0.48f
#define PLAYER_HEIGHT 2.00f
#define PLAYER_CAM_OFFSET_Y 0.84f // Split capsule shape in the middle, camera is thus 0.16 away from top of the capsule ((2 / 2 = 1) - 0.84)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static uint32_t GetCollisionMask(uint8_t layer) {
    switch (layer) {
        case PhysicsLayer_Default:           return 0x217efe07u;
        case PhysicsLayer_TransparentFX:     return 0x7c1e07u;
        case PhysicsLayer_IgnoreRaycast:     return 0x7c3e07u;
        case PhysicsLayer_Water:             return 0x0u;
        case PhysicsLayer_UI:                return 0x0u;
        case PhysicsLayer_GunViewModel:      return 0x0u;
        case PhysicsLayer_Geometry:          return 0x178fc07u;
        case PhysicsLayer_NPC:               return 0x6f61e07u;
        case PhysicsLayer_PlayerBullets:     return 0x217e6607u;
        case PhysicsLayer_Player:            return 0x5770607u;
        case PhysicsLayer_Corpse:            return 0x10c4a05u;
        case PhysicsLayer_PhysObjects:       return 0x17e6a01u;
        case PhysicsLayer_Sky:               return 0x201u;
        case PhysicsLayer_PlayerTriggerOnly: return 0x701000u;
        case PhysicsLayer_Trigger:           return 0x1785c01u;
        case PhysicsLayer_Door:              return 0x1707c07u;
        case PhysicsLayer_InterDebris:       return 0xa6a07u;
        case PhysicsLayer_Player2:           return 0x5675e07u;
        case PhysicsLayer_Player3:           return 0x5575e07u;
        case PhysicsLayer_Player4:           return 0x5375e07u;
        case PhysicsLayer_NPCTrigger:        return 0x400u;
        case PhysicsLayer_NPCBullet:         return 0x20767a01u;
        case PhysicsLayer_NPCClip:           return 0x400u;
        case PhysicsLayer_Clip:              return 0x701400u;
        case PhysicsLayer_Automap:           return 0x0u;
        case PhysicsLayer_Culling:           return 0x0u;
        case PhysicsLayer_CorpseSearchable:  return 0x1000801u;
    }
    
    return 0;
}
#pragma GCC diagnostic pop

float GetBasePlayerSpeed(bool running) {
    bool isSprinting = Sprint();
    if (Sys_Cheats.noclip && isSprinting) return PLAYER_MAX_CYBER_SPEED * 2.5f;
    if (Sys_Cheats.noclip) return PLAYER_MAX_CYBER_SPEED * 1.5f;
    if (Sys_Global.currentLevel == LEVEL_CYBERSPACE) return PLAYER_MAX_CYBER_SPEED; //Cyber space speed

    float retval = PLAYER_MAX_WALK_SPEED;
    float bonus = 0.0f;
    if (boosterActive) bonus = PLAYER_BOOSTER_SPEED_BOOST;
    BodyState bodyState = instances[PLAYER1].bodyState;
    switch (bodyState) {
        case BodyState_Standing:      retval = PLAYER_MAX_WALK_SPEED;   break;
        case BodyState_Crouch:        retval = PLAYER_MAX_CROUCH_SPEED; break;
        case BodyState_CrouchingDown: retval = PLAYER_MAX_CROUCH_SPEED; break;
        case BodyState_StandingUp:    retval = PLAYER_MAX_WALK_SPEED;   break;
        case BodyState_Prone:         retval = PLAYER_MAX_PRONE_SPEED;  break;
        case BodyState_ProningDown:   retval = PLAYER_MAX_PRONE_SPEED;  break;
        case BodyState_ProningUp:     retval = PLAYER_MAX_PRONE_SPEED;  break;
    }

    if ((isSprinting || boosterActive) && running) {
        if (instances[PLAYER1].fatigue > 80.0f && boosterActive) retval = PLAYER_MAX_SPRINT_SPEED_FATIGUED;
        else                                           retval = PLAYER_MAX_SPRINT_SPEED;

        if (bodyState == BodyState_Standing || bodyState == BodyState_Crouch || bodyState == BodyState_CrouchingDown) {
            retval -= ((PLAYER_MAX_WALK_SPEED - PLAYER_MAX_CROUCH_SPEED) * 1.5f); // Subtract off the difference in speed between walking and crouching from the sprint speed
        } else if (bodyState == BodyState_Prone || bodyState == BodyState_ProningDown || bodyState == BodyState_ProningUp) {
            retval -= ((PLAYER_MAX_WALK_SPEED - PLAYER_MAX_PRONE_SPEED) * 2.0f); // Subtract off the difference in speed between walking and proning from the sprint speed.
        }
    }

    return retval + bonus;
}

void ApplyPlayerMovements(void) {
    Vector3 forward = instances[PLAYER1].forward;
    Vector3 right = instances[PLAYER1].right;
    Vector3 input = {0};    
    if (Forward())     input = Vector3_A_plus_B(input, (Vector3){forward.x, 0, forward.z});
    if (Backpedal())     input = Vector3_A_minus_B(input, (Vector3){forward.x, 0, forward.z});
    if (StrafeRight())     input = Vector3_A_plus_B(input, (Vector3){right.x, 0, right.z});
    if (StrafeLeft())     input = Vector3_A_minus_B(input, (Vector3){right.x, 0, right.z});
    if (SwimDn()/* && Sys_Cheats.noclip*/) input.y -= 1.0f; // Temporarily allow for now until I have collision working
    if (SwimUp()/* && Sys_Cheats.noclip*/) input.y += 1.0f;
    input = normalize_vector3(input);
    float intent = magnitude_vector3(input);
    float speed = GetBasePlayerSpeed(intent > 0.1f);
    Vector3 wishVel = scale_vector3(input, speed);
    Vector3 currentVel = instances[PLAYER1].velocity;
    float accel = boosterActive ? 1.0f : 3.0f;
    Vector3 deltaVel = Vector3_A_minus_B(wishVel, currentVel);
    Vector3 appliedVel = Vector3_A_plus_B(currentVel, scale_vector3(deltaVel, (accel * (float)Sys_Global.timeSinceLastPhysicsTick)));
    instances[PLAYER1].velocity = appliedVel; // Gravity applied elsewhere same as everything else.
}

const Vector3 gravityVelocity = { 0.0f, -9.81f, 0.0f };

void UpdateVelocityFromGravity(void) {
    for (int32_t i=PLAYER1;i<loadedInstances;++i) {
        if (i > loadedInstances) return;
        if (instances[i].gravity < 0.01f && instances[i].gravity > -0.01f) continue;
        if (i <= (int32_t)PLAYER2 && Sys_Cheats.noclip) continue;
        
        instances[i].velocity = Vector3_A_plus_B(instances[i].velocity, scale_vector3(gravityVelocity, instances[i].gravity * (float)Sys_Global.timeSinceLastPhysicsTick));
    }
}

float reboundVelocity = 0.1f;

void ApplyVelocityUntilCollision(uint16_t i) {
    Vector3 currentPosition = instances[i].position;
    instances[i].cellIndex = PosGetCellCoords(currentPosition.x, currentPosition.z);
    float mag = magnitude_vector3(instances[i].velocity);
    if (i > PLAYER1/*loadedInstances*/) return;
    if (!(instances[i].index != PLAYER1 || ConstIndexIsDynamicObject(instances[i].index))) return;
    if (mag < 0.05f) return;
    
    Vector3 dir = normalize_vector3(instances[i].velocity);
    Vector3 currentHitPos = Vector3_A_plus_B(instances[i].position, scale_vector3(dir, PLAYER_RADIUS));
                                                                                  
    Vector3 newPosition = Vector3_A_plus_B(currentPosition, scale_vector3(instances[i].velocity, (float)Sys_Global.timeSinceLastPhysicsTick));
    Vector3 newHitPos = Vector3_A_plus_B(currentHitPos, scale_vector3(instances[i].velocity, (float)Sys_Global.timeSinceLastPhysicsTick));
    if (i <= PLAYER2 && Sys_Cheats.noclip) { instances[i].position = newPosition; return; }
    
    int32_t cellCoordsCurrentX = PosGetCellCoordX(currentPosition.x);
    int32_t cellCoordsCurrentZ = PosGetCellCoordZ(currentPosition.z);
    int32_t cellCoordsCurrentHitPosX = PosGetCellCoordX(currentHitPos.x);
    int32_t cellCoordsCurrentHitPosZ = PosGetCellCoordZ(currentHitPos.z);
    if (cellCoordsCurrentHitPosX != cellCoordsCurrentX) cellCoordsCurrentX = cellCoordsCurrentHitPosX;
    if (cellCoordsCurrentHitPosZ != cellCoordsCurrentZ) cellCoordsCurrentZ = cellCoordsCurrentHitPosZ;
    int32_t cellCoordsX = PosGetCellCoordX(newHitPos.x);
    int32_t cellCoordsZ = PosGetCellCoordZ(newHitPos.z);
    int32_t cellCoordsCurrent = (cellCoordsCurrentZ * WORLDX) + cellCoordsCurrentX;
    int32_t cellCoords = (cellCoordsZ * WORLDX) + cellCoordsX;
    if (cellCoordsZ > cellCoordsCurrentZ && (gridCellStates[cellCoordsCurrent] & CELL_CLOSEDNORTH)) { instances[i].velocity.z = -reboundVelocity; return; } // blocked north
    if (cellCoordsZ < cellCoordsCurrentZ && (gridCellStates[cellCoordsCurrent] & CELL_CLOSEDSOUTH)) { instances[i].velocity.z = reboundVelocity; return; } // blocked south
    if (cellCoordsX > cellCoordsCurrentX && (gridCellStates[cellCoordsCurrent] & CELL_CLOSEDEAST)) { instances[i].velocity.x = -reboundVelocity; return; } // blocked east
    if (cellCoordsX < cellCoordsCurrentX && (gridCellStates[cellCoordsCurrent] & CELL_CLOSEDWEST)) { instances[i].velocity.x = reboundVelocity; return; } // blocked west
    if (newHitPos.y < gridCellFloorHeight[cellCoords]) { instances[i].velocity.y = reboundVelocity; return; } // floor blocked
    if (newHitPos.y > gridCellCeilingHeight[cellCoords]) { instances[i].velocity.y = -reboundVelocity; return; } // ceiling blocked
    if (!(gridCellStates[cellCoords] & CELL_OPEN)) { instances[i].velocity = scale_vector3(dir,-reboundVelocity); return; } // void blocked
        
    instances[i].position = newPosition; // It moves! It lives!!
    dirtyInstances[i] = true;
}

void UpdatePositions(void) {
    for (int32_t i=PLAYER1;i<loadedInstances;++i) ApplyVelocityUntilCollision(i);
}

void ClampVelocity(void) {
    for (int32_t i=START_INDEX_LEVEL_INSTANCES;i<loadedInstances;++i) {
        Vector3 curvel = instances[i].velocity;
        if (magnitude_vector3(curvel) > TERMINAL_VELOCITY) {
            Vector3 dir = normalize_vector3(curvel);
            instances[i].velocity = scale_vector3(dir, TERMINAL_VELOCITY);
        }
    }
}

int32_t Physics(void) {
    UpdateVelocityFromGravity();
    ClampVelocity();
    UpdatePositions();
    return 0; // Ok.
}

RaycastHit Raycast(Vector3 origin, Vector3 dir, float distance, uint32_t layerMask) {
    RaycastHit result = { .hit = false };
    (void)origin;
    (void)dir;
    (void)distance;
    (void)layerMask;
    return result;
}

void RaycastAll(Vector3 origin, Vector3 dir, float distance, uint32_t layerMask, RaycastHit* hits, uint16_t maxCount) {
    for (int i=0;i<maxCount;++i) hits[i].hit = false;
    //uint16_t hitHead = 0;
    (void)origin;
    (void)dir;
    (void)distance;
    (void)layerMask;
}

RaycastHit CapsuleCast(Vector3 start, Vector3 end, float capsuleRadius, float castDist, uint32_t layerMask, bool hitTriggers) {
    RaycastHit result = { .hit = false };
    Vector3 dir = Vector3_A_minus_B(end, start);
    (void)capsuleRadius;
    (void)dir;
    (void)layerMask;
    (void)castDist;
    (void)hitTriggers;
    return result;
}

bool CheckCapsule(Vector3 start, Vector3 end, float capsuleRadius, float capsuleHeight, uint32_t layerMask) {
    (void)start;
    (void)end;
    (void)capsuleRadius;
    (void)capsuleHeight;
    (void)layerMask;
    return false;
}
