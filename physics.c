// physics.cpp - Full Jolt Physics integration for Voxen
#include "voxen.h"

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

void ApplyPlayerMovements(void) {
    Vector3 forward = Sys_Global.instances[PLAYER1].forward;
    Vector3 right = Sys_Global.instances[PLAYER1].right;
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
    Vector3 currentVel = Sys_Global.instances[PLAYER1].velocity;
    float accel = Sys_Global.boosterActive ? 1.0f : 3.0f;
    Vector3 deltaVel = Vector3_A_minus_B(wishVel, currentVel);
    deltaVel.x = vmax(vmin(deltaVel.x,10.0f),-10.0f);
    deltaVel.y = vmax(vmin(deltaVel.y,10.0f),-10.0f);
    deltaVel.z = vmax(vmin(deltaVel.z,10.0f),-10.0f);
    Vector3 appliedVel = Vector3_A_plus_B(currentVel, scale_vector3(deltaVel, (accel * (float)Sys_Global.timeSinceLastPhysicsTick)));
    Sys_Global.instances[PLAYER1].velocity = appliedVel; // Gravity applied elsewhere same as everything else.
}

const Vector3 gravityVelocity = { 0.0f, -9.81f, 0.0f };

void UpdateVelocityFromGravity(void) {
    for (int32_t i=PLAYER1;i<Sys_Global.loadedInstances;++i) {
        if (i > Sys_Global.loadedInstances) return;
        if (Sys_Global.instances[i].gravity < 0.01f && Sys_Global.instances[i].gravity > -0.01f) continue;
        if (i <= (int32_t)PLAYER2 && Sys_Cheats.noclip) continue;
        
        Sys_Global.instances[i].velocity = Vector3_A_plus_B(Sys_Global.instances[i].velocity, scale_vector3(gravityVelocity, Sys_Global.instances[i].gravity * (float)Sys_Global.timeSinceLastPhysicsTick));
    }
}

float reboundVelocity = 0.1f;

void ApplyVelocityUntilCollision(uint16_t i) {
    Vector3 currentPosition = Sys_Global.instances[i].position;
    Sys_Global.instances[i].cellIndex = PosGetCellCoords(currentPosition.x, currentPosition.z);
    float mag = magnitude_vector3(Sys_Global.instances[i].velocity);
    if (i > PLAYER1/*Sys_Global.loadedInstances*/) return;
    if (!(Sys_Global.instances[i].index != PLAYER1 || (Sys_Global.instances[i].entflags & ENTFLAG_RIGIDBODY))) return;
    if (mag < 0.05f) return;
    
    Vector3 dir = normalize_vector3(Sys_Global.instances[i].velocity);
    Vector3 currentHitPos = Vector3_A_plus_B(Sys_Global.instances[i].position, scale_vector3(dir, PLAYER_RADIUS));
                                                                                  
    Vector3 newPosition = Vector3_A_plus_B(currentPosition, scale_vector3(Sys_Global.instances[i].velocity, (float)Sys_Global.timeSinceLastPhysicsTick));
    Vector3 newHitPos = Vector3_A_plus_B(currentHitPos, scale_vector3(Sys_Global.instances[i].velocity, (float)Sys_Global.timeSinceLastPhysicsTick));
    if (i <= PLAYER2 && Sys_Cheats.noclip) { Sys_Global.instances[i].position = newPosition; return; }
    
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
    if (cellCoordsZ > cellCoordsCurrentZ && (gridCellStates[cellCoordsCurrent] & CELL_CLOSEDNORTH)) { Sys_Global.instances[i].velocity.z = -reboundVelocity; return; } // blocked north
    if (cellCoordsZ < cellCoordsCurrentZ && (gridCellStates[cellCoordsCurrent] & CELL_CLOSEDSOUTH)) { Sys_Global.instances[i].velocity.z = reboundVelocity; return; } // blocked south
    if (cellCoordsX > cellCoordsCurrentX && (gridCellStates[cellCoordsCurrent] & CELL_CLOSEDEAST)) { Sys_Global.instances[i].velocity.x = -reboundVelocity; return; } // blocked east
    if (cellCoordsX < cellCoordsCurrentX && (gridCellStates[cellCoordsCurrent] & CELL_CLOSEDWEST)) { Sys_Global.instances[i].velocity.x = reboundVelocity; return; } // blocked west
    if (!(gridCellStates[cellCoords] & CELL_OPEN)) { Sys_Global.instances[i].velocity = scale_vector3(dir,-reboundVelocity); return; } // void blocked
        
    Sys_Global.instances[i].position = newPosition; // It moves! It lives!!
    dirtyInstances[i] = true;
}

void ApplyCorpseFriction(uint16_t instanceIdx) {
    Sys_Global.instances[instanceIdx].dynamicFriction = 10.0f;
    Sys_Global.instances[instanceIdx].staticFriction = 10.0f;
    Sys_Global.instances[instanceIdx].bounciness = 0.0f;
    Sys_Global.instances[instanceIdx].frictionCombine = PHYS_COMBINE_MUL;
    Sys_Global.instances[instanceIdx].bounceCombine = PHYS_COMBINE_MAX;
}

void UpdatePositions(void) {
    for (int32_t i=PLAYER1;i<Sys_Global.loadedInstances;++i) ApplyVelocityUntilCollision(i);
}

void ClampVelocity(void) {
    for (int32_t i=START_INDEX_LEVEL_INSTANCES;i<Sys_Global.loadedInstances;++i) {
        Vector3 curvel = Sys_Global.instances[i].velocity;
        if (magnitude_vector3(curvel) > TERMINAL_VELOCITY) {
            Vector3 dir = normalize_vector3(curvel);
            Sys_Global.instances[i].velocity = scale_vector3(dir, TERMINAL_VELOCITY);
        }
    }
}

void UpdateTriggers(void) {
    
}

int32_t Physics(void) {
    UpdateVelocityFromGravity();
    ClampVelocity();
    UpdatePositions();
    UpdateTriggers();
    return 0; // Ok.
}

Vector3 quat_rotate(Quaternion q, Vector3 v) {
    float x2 = q.x + q.x;
    float y2 = q.y + q.y;
    float z2 = q.z + q.z;
    float xx2 = q.x * x2;
    float yy2 = q.y * y2;
    float zz2 = q.z * z2;
    float xy2 = q.x * y2;
    float xz2 = q.x * z2;
    float yz2 = q.y * z2;
    float wx2 = q.w * x2;
    float wy2 = q.w * y2;
    float wz2 = q.w * z2;
    return (Vector3){
        v.x * (1.0f - yy2 - zz2) + v.y * (xy2 - wz2) + v.z * (xz2 + wy2),
        v.x * (xy2 + wz2) + v.y * (1.0f - xx2 - zz2) + v.z * (yz2 - wx2),
        v.x * (xz2 - wy2) + v.y * (yz2 + wx2) + v.z * (1.0f - xx2 - yy2)
    };
}

uint16_t PointInSolid(Vector3 point, uint32_t layerMask) {
    for (uint16_t i = START_INDEX_LEVEL_INSTANCES; i < INSTANCE_COUNT; ++i) {
        if (!(layerMask & Sys_Global.instances[i].layer)) continue;
        if (Sys_Global.instances[i].collider == COLLIDER_TYPE_NONE || Sys_Global.instances[i].collider == COLLIDER_TYPE_MESH) continue;

        Vector3 pos = Sys_Global.instances[i].position;
        Quaternion rot = Sys_Global.instances[i].rotation;
        Vector3 scale = Sys_Global.instances[i].scale;
        Vector3 local = Vector3_A_minus_B(point, pos);
        Quaternion invRot = (Quaternion){-rot.x, -rot.y, -rot.z, rot.w};
        local = quat_rotate(invRot, local);
        if (scale.x != 0.0f) local.x /= scale.x;
        if (scale.y != 0.0f) local.y /= scale.y;
        if (scale.z != 0.0f) local.z /= scale.z;
        Vector3 center;
        float radius, distSq;
        switch (Sys_Global.instances[i].collider) {
            case COLLIDER_TYPE_BOX:
                center = Sys_Global.instances[i].colliderCenter;
                Vector3 size   = Sys_Global.instances[i].colliderSize; // full extents
                Vector3 half = (Vector3){ size.x * 0.5f, size.y * 0.5f, size.z * 0.5f };
                Vector3 min = Vector3_A_minus_B(center, half);
                Vector3 max = Vector3_A_plus_B(center, half);
                if (local.x >= min.x && local.x <= max.x && local.y >= min.y && local.y <= max.y && local.z >= min.z && local.z <= max.z) return i;
                break;
            case COLLIDER_TYPE_SPHERE:
                center = Sys_Global.instances[i].colliderCenter;
                radius = Sys_Global.instances[i].colliderSize.x;
                Vector3 offset = Vector3_A_minus_B(local, center);
                distSq = dot_vector3(offset, offset);
                if (distSq <= radius * radius) return i;
                break;
            case COLLIDER_TYPE_CAPSULE:
                center = Sys_Global.instances[i].colliderCenter;
                radius = Sys_Global.instances[i].colliderSize.x;
                float height = Sys_Global.instances[i].colliderSize.y; // full cylinder height
                int axis = (int)Sys_Global.instances[i].colliderSize.z; // 0=X, 1=Y, 2=Z
                Vector3 axisDir = {0.0f, 1.0f, 0.0f};
                if (axis == 0) axisDir = (Vector3){1.0f, 0.0f, 0.0f};
                else if (axis == 2) axisDir = (Vector3){0.0f, 0.0f, 1.0f};
                Vector3 halfHeightVec = scale_vector3(axisDir, height * 0.5f);
                Vector3 p1 = Vector3_A_minus_B(center, halfHeightVec);
                Vector3 p2 = Vector3_A_plus_B(center, halfHeightVec);
                Vector3 p1_to_point = Vector3_A_minus_B(local, p1);
                Vector3 p1_to_p2     = Vector3_A_minus_B(p2, p1);
                float segLenSq = dot_vector3(p1_to_p2, p1_to_p2);
                float t = (segLenSq > 0.0f) ? dot_vector3(p1_to_point, p1_to_p2) / segLenSq : 0.0f;
                t = vclamp(t, 0.0f, 1.0f);
                Vector3 closest = Vector3_A_plus_B(p1, scale_vector3(p1_to_p2, t));
                Vector3 toClosest = Vector3_A_minus_B(local, closest);
                distSq = dot_vector3(toClosest, toClosest);
                if (distSq <= radius * radius) return i;
                break;
            case COLLIDER_TYPE_CONVEXMESH: break;
        }
    }

    return UINT16_MAX;
}

RaycastHit RayTriangle(Vector3 origin, Vector3 dir, Vector3 posA, Vector3 posB, Vector3 posC,
                       Vector3 normA, Vector3 normB, Vector3 normC) {
    Vector3 edgeAB = Vector3_A_minus_B(posB,posA);
    Vector3 edgeAC = Vector3_A_minus_B(posC,posA);
    Vector3 normalVector = cross_vector3(edgeAB,edgeAC);
    Vector3 ao = Vector3_A_minus_B(origin,posA);
    Vector3 dao = cross_vector3(ao,dir);
    float determinant = -dot_vector3(dir, normalVector);
    float invDet = 1.0f / determinant;
    float dst = dot_vector3(ao, normalVector) * invDet;
    float u = dot_vector3(edgeAC, dao) * invDet;
    float v = -dot_vector3(edgeAB, dao) * invDet;
    float w = 1.0f - u - v;
    RaycastHit hitInfo;
    hitInfo.hit = determinant >= 1E-8f && dst >= 0.0f && u >= 0.0f && v >= 0.0f && w >= 0.0f;
    hitInfo.point = Vector3_A_plus_B(origin,scale_vector3(dir,dst));
    hitInfo.normal = normalize_vector3(Vector3_A_plus_B(Vector3_A_plus_B(scale_vector3(normA,w),scale_vector3(normB,u)),scale_vector3(normC,v)));
    hitInfo.distance = dst;
    return hitInfo;
}

ENGINE_TO_MOD RaycastHit Raycast(Vector3 origin, Vector3 dir, float maxDist, uint32_t layerMask) {
    RaycastHit result = {
        .hit = false,
        .distance = maxDist,
        .point = {0.0f, 0.0f, 0.0f},
        .normal = {0.0f, 0.0f, 0.0f},
        .hitInstanceIndex = INSTANCE_COUNT
    };
    
    uint16_t hitObjectIndex = UINT16_MAX;
    for (float curDist=0.0f;curDist<maxDist;curDist+=0.02f) { // 4.9 / 0.04 = 245 tries worst case empty air
        Vector3 checkPoint = Vector3_A_plus_B(origin, scale_vector3(dir,curDist));
        hitObjectIndex = PointInSolid(checkPoint, layerMask);
        if (hitObjectIndex < Sys_Global.loadedInstances) {
            result.hit = true;
            result.point = checkPoint;
            result.distance = curDist; // TODO refine the raymarch a little?  nah 0.02 good enough for effects, will apply offset along normal for bullet holes and such anyways.
            result.normal = Vector3_A_minus_B(checkPoint,origin);
            result.hitInstanceIndex = hitObjectIndex;
            return result;
        }
    }
    
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
