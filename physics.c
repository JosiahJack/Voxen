// physics.cpp - Full Jolt Physics integration for Voxen
#include "voxen.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static uint32_t GetCollisionMask(uint32_t layer) {
    switch (layer) {
        case PhysicsLayer_Default:           return PhysicsLayer_Default | PhysicsLayer_TransparentFX | PhysicsLayer_IgnoreRaycast | PhysicsLayer_Geometry | PhysicsLayer_NPC | PhysicsLayer_PlayerBullets | PhysicsLayer_Player | PhysicsLayer_Corpse | PhysicsLayer_PhysObjects | PhysicsLayer_Sky | PhysicsLayer_Trigger | PhysicsLayer_Door | PhysicsLayer_InterDebris | PhysicsLayer_Player2 | PhysicsLayer_Player3 | PhysicsLayer_Player4 | PhysicsLayer_NPCBullet | PhysicsLayer_Clip | PhysicsLayer_CorpseSearchable;
        case PhysicsLayer_TransparentFX:     return PhysicsLayer_Default | PhysicsLayer_TransparentFX | PhysicsLayer_IgnoreRaycast | PhysicsLayer_Geometry | PhysicsLayer_NPC | PhysicsLayer_PlayerBullets | PhysicsLayer_Player | PhysicsLayer_PhysObjects | PhysicsLayer_Trigger | PhysicsLayer_Door | PhysicsLayer_InterDebris | PhysicsLayer_Player2 | PhysicsLayer_NPCBullet | PhysicsLayer_Clip;
        case PhysicsLayer_IgnoreRaycast:     return PhysicsLayer_Default | PhysicsLayer_TransparentFX | PhysicsLayer_IgnoreRaycast | PhysicsLayer_Geometry | PhysicsLayer_NPC | PhysicsLayer_PlayerBullets | PhysicsLayer_Player | PhysicsLayer_PhysObjects | PhysicsLayer_Trigger | PhysicsLayer_Door | PhysicsLayer_InterDebris | PhysicsLayer_Player2 | PhysicsLayer_NPCBullet | PhysicsLayer_Clip;
        case PhysicsLayer_Water:             return 0u;
        case PhysicsLayer_UI:                return 0u;
        case PhysicsLayer_GunViewModel:      return 0u;
        case PhysicsLayer_Geometry:          return PhysicsLayer_Default | PhysicsLayer_TransparentFX | PhysicsLayer_IgnoreRaycast | PhysicsLayer_Geometry | PhysicsLayer_NPC | PhysicsLayer_PlayerBullets | PhysicsLayer_Player | PhysicsLayer_PhysObjects | PhysicsLayer_Trigger | PhysicsLayer_Door | PhysicsLayer_InterDebris | PhysicsLayer_Player2 | PhysicsLayer_Clip;
        case PhysicsLayer_NPC:               return PhysicsLayer_Default | PhysicsLayer_TransparentFX | PhysicsLayer_IgnoreRaycast | PhysicsLayer_Geometry | PhysicsLayer_NPC | PhysicsLayer_PlayerBullets | PhysicsLayer_Player | PhysicsLayer_PhysObjects | PhysicsLayer_Trigger | PhysicsLayer_NPCTrigger | PhysicsLayer_Door | PhysicsLayer_InterDebris | PhysicsLayer_Player2 | PhysicsLayer_NPCBullet | PhysicsLayer_NPCClip | PhysicsLayer_Clip;
        case PhysicsLayer_PlayerBullets:     return PhysicsLayer_Default | PhysicsLayer_TransparentFX | PhysicsLayer_IgnoreRaycast | PhysicsLayer_Geometry | PhysicsLayer_NPC | PhysicsLayer_PlayerBullets | PhysicsLayer_Player | PhysicsLayer_Corpse | PhysicsLayer_PhysObjects | PhysicsLayer_Door | PhysicsLayer_InterDebris | PhysicsLayer_Player2 | PhysicsLayer_NPCBullet | PhysicsLayer_Clip | PhysicsLayer_CorpseSearchable;
        case PhysicsLayer_Player:            return PhysicsLayer_Default | PhysicsLayer_TransparentFX | PhysicsLayer_IgnoreRaycast | PhysicsLayer_Geometry | PhysicsLayer_NPC | PhysicsLayer_PhysObjects | PhysicsLayer_PlayerTriggerOnly | PhysicsLayer_Trigger | PhysicsLayer_Door | PhysicsLayer_Player2 | PhysicsLayer_NPCBullet | PhysicsLayer_Clip;
        case PhysicsLayer_Corpse:            return PhysicsLayer_Default | PhysicsLayer_Geometry | PhysicsLayer_PlayerBullets | PhysicsLayer_PhysObjects | PhysicsLayer_Door | PhysicsLayer_NPCBullet | PhysicsLayer_Clip;
        case PhysicsLayer_PhysObjects:       return PhysicsLayer_Default | PhysicsLayer_TransparentFX | PhysicsLayer_IgnoreRaycast | PhysicsLayer_Geometry | PhysicsLayer_NPC | PhysicsLayer_PlayerBullets | PhysicsLayer_Player | PhysicsLayer_Corpse | PhysicsLayer_PhysObjects | PhysicsLayer_Door | PhysicsLayer_InterDebris | PhysicsLayer_NPCBullet | PhysicsLayer_Clip;
        case PhysicsLayer_Sky:               return PhysicsLayer_Default | PhysicsLayer_Player;
        case PhysicsLayer_PlayerTriggerOnly: return PhysicsLayer_Player | PhysicsLayer_Player2 | PhysicsLayer_Player3;
        case PhysicsLayer_Trigger:           return PhysicsLayer_Default | PhysicsLayer_Geometry | PhysicsLayer_NPC | PhysicsLayer_PlayerBullets | PhysicsLayer_Player | PhysicsLayer_PhysObjects | PhysicsLayer_Door | PhysicsLayer_InterDebris | PhysicsLayer_Clip;
        case PhysicsLayer_Door:              return PhysicsLayer_Default | PhysicsLayer_TransparentFX | PhysicsLayer_IgnoreRaycast | PhysicsLayer_Geometry | PhysicsLayer_NPC | PhysicsLayer_PlayerBullets | PhysicsLayer_Player | PhysicsLayer_Corpse | PhysicsLayer_PhysObjects | PhysicsLayer_Trigger | PhysicsLayer_Door | PhysicsLayer_InterDebris | PhysicsLayer_Player2 | PhysicsLayer_NPCBullet | PhysicsLayer_Clip;
        case PhysicsLayer_InterDebris:       return PhysicsLayer_Default | PhysicsLayer_Geometry | PhysicsLayer_NPC | PhysicsLayer_PlayerBullets | PhysicsLayer_PhysObjects | PhysicsLayer_Trigger | PhysicsLayer_Door | PhysicsLayer_NPCBullet | PhysicsLayer_Clip;
        case PhysicsLayer_Player2:           return PhysicsLayer_Default | PhysicsLayer_TransparentFX | PhysicsLayer_IgnoreRaycast | PhysicsLayer_Geometry | PhysicsLayer_NPC | PhysicsLayer_PlayerBullets | PhysicsLayer_Player | PhysicsLayer_PhysObjects | PhysicsLayer_PlayerTriggerOnly | PhysicsLayer_Trigger | PhysicsLayer_Door | PhysicsLayer_InterDebris | PhysicsLayer_Player2 | PhysicsLayer_NPCBullet | PhysicsLayer_Clip;
        case PhysicsLayer_Player3:           return PhysicsLayer_Default | PhysicsLayer_TransparentFX | PhysicsLayer_IgnoreRaycast | PhysicsLayer_Geometry | PhysicsLayer_NPC | PhysicsLayer_PlayerBullets | PhysicsLayer_Player | PhysicsLayer_PhysObjects | PhysicsLayer_PlayerTriggerOnly | PhysicsLayer_Trigger | PhysicsLayer_Door | PhysicsLayer_InterDebris | PhysicsLayer_Player3 | PhysicsLayer_NPCBullet | PhysicsLayer_Clip;
        case PhysicsLayer_Player4:           return PhysicsLayer_Default | PhysicsLayer_TransparentFX | PhysicsLayer_IgnoreRaycast | PhysicsLayer_Geometry | PhysicsLayer_NPC | PhysicsLayer_PlayerBullets | PhysicsLayer_Player | PhysicsLayer_PhysObjects | PhysicsLayer_PlayerTriggerOnly | PhysicsLayer_Trigger | PhysicsLayer_Door | PhysicsLayer_InterDebris | PhysicsLayer_Player4 | PhysicsLayer_NPCBullet | PhysicsLayer_Clip;
        case PhysicsLayer_NPCTrigger:        return PhysicsLayer_NPC;
        case PhysicsLayer_NPCBullet:         return PhysicsLayer_Default | PhysicsLayer_TransparentFX | PhysicsLayer_IgnoreRaycast | PhysicsLayer_Geometry | PhysicsLayer_PlayerBullets | PhysicsLayer_Player | PhysicsLayer_Corpse | PhysicsLayer_PhysObjects | PhysicsLayer_Door | PhysicsLayer_InterDebris | PhysicsLayer_Player2 | PhysicsLayer_Clip | PhysicsLayer_CorpseSearchable;
        case PhysicsLayer_NPCClip:           return PhysicsLayer_NPC;
        case PhysicsLayer_Clip:              return PhysicsLayer_Player | PhysicsLayer_Player2 | PhysicsLayer_Player3 | PhysicsLayer_Player4 | PhysicsLayer_NPC;
        case PhysicsLayer_Automap:           return 0u;
        case PhysicsLayer_Culling:           return 0u;
        case PhysicsLayer_CorpseSearchable:  return PhysicsLayer_Default | PhysicsLayer_PlayerBullets;
        default:                             return 0u;
    }
}
#pragma GCC diagnostic pop

ENGINE_TO_MOD void ApplyPlayerMovements(void) {
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

RaycastHit RayTriangle(Vector3 origin, Vector3 dir, Vector3 posA, Vector3 posB, Vector3 posC, Vector3 normA, Vector3 normB, Vector3 normC) {
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
    hitInfo.hit = vabs(determinant) >= 1E-8f && dst >= 0.0f && u >= 0.0f && v >= 0.0f && w >= 0.0f;
    hitInfo.point = Vector3_A_plus_B(origin,scale_vector3(dir,dst));
    hitInfo.normal = normalize_vector3(Vector3_A_plus_B(Vector3_A_plus_B(scale_vector3(normA,w),scale_vector3(normB,u)),scale_vector3(normC,v)));
    hitInfo.distance = dst;
    return hitInfo;
}

ENGINE_TO_MOD RaycastHit Raycast(Vector3 origin, Vector3 dir, float maxDist, uint32_t layerMask) {
    uint32_t numMeshesCheckedForRaycast = 0, numTrisCastAgainst = 0;
    bool skyVisible = (gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX);
    RaycastHit result = { .hit = false, .distance = maxDist, .point = {0.0f, 0.0f, 0.0f}, .normal = {0.0f, 0.0f, 0.0f}, .hitInstanceIndex = INSTANCE_COUNT };
    for (uint16_t i = START_INDEX_LEVEL_INSTANCES; i < INSTANCE_COUNT; ++i) {
        if (!(layerMask & Sys_Global.instances[i].layer)) continue;
        
        uint16_t mindex = Sys_Global.instances[i].modelIndex;
        if (mindex >= loadedModelsMaxIndex) continue;
        
        Vector3 objPos = Sys_Global.instances[i].position;
        uint16_t instCellIdx = PosGetCellCoords(objPos.x,objPos.z);
        Vector3 delta = Vector3_A_minus_B(objPos,origin);
        float distSqrd = delta.x*delta.x + delta.y*delta.y + delta.z*delta.z;
        float radBounds = vmax(modelBounds[(mindex * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_RADIUS], 1.81f); // 1.28x1.28 corner of modular chunk
        float maxDistToObj = vmax(maxDist - radBounds,maxDist);
        if (distSqrd >= (maxDistToObj * maxDistToObj)) continue;

        uint16_t constIndex = Sys_Global.instances[i].index;
        if (!(Sys_Global.currentLevel == 1 && (constIndex == 309 || constIndex == 532)) && !EntityIndexIsPortalBlockingDoor(constIndex)) { // Hack for beaker and beaker holder on level 1 shelf getting culled from door portals.
            if (((gridCellStates[instCellIdx] & (CELL_VISIBLE | CELL_OPEN)) == CELL_OPEN) && (constIndex != 754 || !skyVisible)) continue; // For some shelves that are inset away from cells, need to still draw their items by checking && CELL_OPEN here, unfortunately this means they don't ever get culled :(
        }
        
        uint32_t triCount = modelTriangleCounts[mindex];
        if (triCount < 1) continue;

        float M[16];
        __builtin_memcpy(M,&modelMatrices[i * 16],16 * sizeof(float)); // Copy values to prevent modifying it inadvertently.
        float m00=M[0], m10=M[1], m20=M[2];  // col 0
        float m01=M[4], m11=M[5], m21=M[6];  // col 1
        float m02=M[8], m12=M[9], m22=M[10]; // col 2
        float tx=M[12], ty=M[13], tz=M[14];  // translation

        // InverseTransformPoint: subtract translation, multiply by transpose of rotation part
        // (transpose = inverse for orthonormal, scale factors cancel if we also divide)
        float scl_x = vsqrtf(m00*m00 + m10*m10 + m20*m20); // = sclx
        float scl_y = vsqrtf(m01*m01 + m11*m11 + m21*m21); // = scly
        float scl_z = vsqrtf(m02*m02 + m12*m12 + m22*m22); // = sclz
        Vector3 rel = {origin.x - tx, origin.y - ty, origin.z - tz};
        Vector3 localOrigin = { // Multiply by transpose of rotation, divide out scale
            (rel.x*m00 + rel.y*m10 + rel.z*m20) / (scl_x * scl_x),
            (rel.x*m01 + rel.y*m11 + rel.z*m21) / (scl_y * scl_y),
            (rel.x*m02 + rel.y*m12 + rel.z*m22) / (scl_z * scl_z)
        };
        Vector3 localDir = {
            (dir.x*m00 + dir.y*m10 + dir.z*m20) / (scl_x * scl_x),
            (dir.x*m01 + dir.y*m11 + dir.z*m21) / (scl_y * scl_y),
            (dir.x*m02 + dir.y*m12 + dir.z*m22) / (scl_z * scl_z)
        };
        localDir = normalize_vector3(localDir);
        numMeshesCheckedForRaycast++;
        for (uint32_t j=0;j<triCount;++j) {
            uint32_t bA = modelTriangles[mindex][j * 3] * VERTEX_ATTRIBUTES_COUNT, bB = modelTriangles[mindex][(j * 3) + 1] * VERTEX_ATTRIBUTES_COUNT, bC = modelTriangles[mindex][(j * 3) + 2] * VERTEX_ATTRIBUTES_COUNT;
            Vector3 posA = (Vector3){modelVertices[mindex][bA + 0],modelVertices[mindex][bA + 1],modelVertices[mindex][bA + 2]};
            Vector3 normA =(Vector3){modelVertices[mindex][bA + 3],modelVertices[mindex][bA + 4],modelVertices[mindex][bA + 5]};
            Vector3 posB = (Vector3){modelVertices[mindex][bB + 0],modelVertices[mindex][bB + 1],modelVertices[mindex][bB + 2]};
            Vector3 normB =(Vector3){modelVertices[mindex][bB + 3],modelVertices[mindex][bB + 4],modelVertices[mindex][bB + 5]};
            Vector3 posC = (Vector3){modelVertices[mindex][bC + 0],modelVertices[mindex][bC + 1],modelVertices[mindex][bC + 2]};
            Vector3 normC =(Vector3){modelVertices[mindex][bC + 3],modelVertices[mindex][bC + 4],modelVertices[mindex][bC + 5]};
            RaycastHit tryTri = RayTriangle(localOrigin,localDir,posA,posB,posC,normA,normB,normC);
            numTrisCastAgainst++;
            if (!tryTri.hit) continue;

            // Transform hit point back: multiply by rotation columns, add translation
            Vector3 worldPoint = {
                m00*tryTri.point.x + m01*tryTri.point.y + m02*tryTri.point.z + tx,
                m10*tryTri.point.x + m11*tryTri.point.y + m12*tryTri.point.z + ty,
                m20*tryTri.point.x + m21*tryTri.point.y + m22*tryTri.point.z + tz
            };
            
            Vector3 toHit = Vector3_A_minus_B(worldPoint, origin);
            float worldDist = vsqrtf(toHit.x*toHit.x + toHit.y*toHit.y + toHit.z*toHit.z);
            if (worldDist >= result.distance) continue;

            Vector3 worldNormal = {
                (m00/scl_x)*tryTri.normal.x + (m01/scl_y)*tryTri.normal.y + (m02/scl_z)*tryTri.normal.z,
                (m10/scl_x)*tryTri.normal.x + (m11/scl_y)*tryTri.normal.y + (m12/scl_z)*tryTri.normal.z,
                (m20/scl_x)*tryTri.normal.x + (m21/scl_y)*tryTri.normal.y + (m22/scl_z)*tryTri.normal.z
            };
            worldNormal = normalize_vector3(worldNormal);
            result.hit              = true;
            result.point            = worldPoint;
            result.normal           = normalize_vector3(worldNormal);
            result.distance         = worldDist;
            result.hitInstanceIndex = i;
        }
    }
    
    if (result.hit) DualLog("[HIT] Raycast with org %f %f %f and dir %f %f %f, range %f, mask %u, tested against %u instances, tris %u, hit %u, layer %u\n",origin.x,origin.y,origin.z,dir.x,dir.y,dir.z,maxDist,layerMask,numMeshesCheckedForRaycast,numTrisCastAgainst,result.hitInstanceIndex,Sys_Global.instances[result.hitInstanceIndex].layer);
    else            DualLog("[MISS] Raycast with org %f %f %f and dir %f %f %f, range %f, mask %u, tested against %u instances, tris %u\n",origin.x,origin.y,origin.z,dir.x,dir.y,dir.z,maxDist,layerMask,numMeshesCheckedForRaycast,numTrisCastAgainst);
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
