// physics.c - Physics System
#include "voxen.h"
#include "entity.h"
#define FLT_MAX 3.40282306e+38f
void ProcessInput(void);
void UpdatePlayerFacingAngles(void);

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

float fatigue;

// Check if instance is in 3x3 grid around object
static inline bool is_instance_in_neighbor_cells(uint32_t instanceCellIdx, uint32_t objectCellIdx) {
    uint32_t dx = (instanceCellIdx % WORLDX) - (objectCellIdx % WORLDX);
    uint32_t dz = (instanceCellIdx / WORLDZ) - (objectCellIdx / WORLDZ);
    return (dx * dx) <= 1 && (dz * dz) <= 1; // 0 means same cell, 1 means one of the 8 neighbors, accept all.
}

// float PhysicsForceCombine(const float a, const float b, PhysCombineType combine) {
//     switch (combine) {
//         case PHYS_COMBINE_AVG: return (a + b) * 0.5f;
//         case PHYS_COMBINE_MIN: return vmin(a, b);
//         case PHYS_COMBINE_MUL: return vsqrtf(a * b);
//         case PHYS_COMBINE_MAX: return vmax(a, b);
//         default: return a; // or average
//     }
// }

// void AddRelativeForce(uint16_t idx, Vector3 localForce, bool isImpulse) {
//     Entity* e = &instances[idx];
//     if ((e->entflags & ENTFLAG_KINEMATIC) || e->mass <= 0.0f) return;
//     Vector3 worldForce = rotate_quaternion(e->rotation, localForce);
//     AddForce(idx, worldForce, isImpulse);
// }

// void AddForceAtPosition(uint16_t idx, Vector3 force, Vector3 position, bool isImpulse) {
//     Entity* e = &instances[idx];
//     if ((e->entflags & ENTFLAG_KINEMATIC) || e->mass <= 0.0f) return;
//     if (isImpulse) {
//         e->velocity = Vector3_A_plus_B(e->velocity, scale_vector3(force, 1.0f / e->mass));
//     } else {
//         e->accumulatedForce = Vector3_A_plus_B(e->accumulatedForce, force);
//     }
//     // Torque (world space).
//     if (! (ConstIndexIsNPC(e->index) || idx == PLAYER1 || idx == PLAYER2) && e->inertia > 0.0f) {
//         Vector3 relPos = sub_vector3(position, e->position);
//         Vector3 torque = cross_vector3(relPos, force);
//         if (isImpulse) {
//             // For impulse torque, integrate angVel directly (approx).
//             float invI = 1.0f / e->inertia;
//             e->angularVelocity = Vector3_A_plus_B(e->angularVelocity, scale_vector3(torque, invI));
//         } else {
//             e->accumulatedTorque = Vector3_A_plus_B(e->accumulatedTorque, torque);
//         }
//     }
// }

// void AddExplosionForce(uint16_t idx, float power, Vector3 explosionPos, float radius, float upwardsModifier, bool isImpulse) {
//     Entity* e = &instances[idx];
//     if ((e->entflags & ENTFLAG_KINEMATIC) || e->mass <= 0.0f) return;
//     Vector3 dir = sub_vector3(e->position, explosionPos);
//     float dist = magnitude_vector3(dir);
//     if (dist > radius || dist < 1e-6f) return;
//     dir = normalize_vector3(dir);
//     float falloff = 1.0f - (dist / radius);
//     if (falloff < 0.0f) falloff = 0.0f;
//     Vector3 explosionForce = scale_vector3(dir, power * falloff);
//     explosionForce.y += upwardsModifier * power * falloff;
//     if (isImpulse) e->velocity = Vector3_A_plus_B(e->velocity, scale_vector3(explosionForce, 1.0f / e->mass));
//     else           e->accumulatedForce = Vector3_A_plus_B(e->accumulatedForce, explosionForce);
// }

void AddForce(uint16_t idx, Vector3 force, bool isImpulse) { // isImpulse=true for ForceMode.Impulse, false for Force).
    if ((instances[idx].entflags & ENTFLAG_KINEMATIC) || instances[idx].mass <= 0.0f) return;
    if (isImpulse) instances[idx].velocity = Vector3_A_plus_B(instances[idx].velocity, scale_vector3(force, 1.0f / instances[idx].mass));
    else           instances[idx].accumulatedForce = Vector3_A_plus_B(instances[idx].accumulatedForce, force);
}

__attribute__((pure)) float GetBasePlayerSpeed(bool running) {
    bool isSprinting = keyStates[GLFW_KEY_LEFT_SHIFT].down; // TODO handle keybind
    if (voxen_Cheats.noclip && isSprinting) return PLAYER_MAX_CYBER_SPEED * 2.5f;
    if (voxen_Cheats.noclip) return PLAYER_MAX_CYBER_SPEED * 1.5f;
    if (voxen_globalContext.currentLevel == LEVEL_CYBERSPACE) return PLAYER_MAX_CYBER_SPEED; //Cyber space speed

    float retval = PLAYER_MAX_WALK_SPEED;
    float bonus = 0.0f;
    if (boosterActive > 0u) bonus = PLAYER_BOOSTER_SPEED_BOOST; // TODO proper booster hookup
    BodyState bodyState = instances[PLAYER1].bodyState;
    switch (bodyState) {
        case BodyState_Standing: 		retval = PLAYER_MAX_WALK_SPEED;   break;
        case BodyState_Crouch: 			retval = PLAYER_MAX_CROUCH_SPEED; break;
        case BodyState_CrouchingDown: 	retval = PLAYER_MAX_CROUCH_SPEED; break;
        case BodyState_StandingUp: 		retval = PLAYER_MAX_WALK_SPEED;   break;
        case BodyState_Prone: 			retval = PLAYER_MAX_PRONE_SPEED;  break;
        case BodyState_ProningDown: 	retval = PLAYER_MAX_PRONE_SPEED;  break;
        case BodyState_ProningUp: 		retval = PLAYER_MAX_PRONE_SPEED;  break;
    }

    if ((isSprinting/* || Inventory.a.BoosterActive()*/) && running) { // TODO proper booster hookup
        if (fatigue > 80.0f/* && !Inventory.a.BoosterActive()*/) retval = PLAYER_MAX_SPRINT_SPEED_FATIGUED; // TODO booster hookup
        else                                                 retval = PLAYER_MAX_SPRINT_SPEED;

        if (bodyState == BodyState_Standing || bodyState == BodyState_Crouch || bodyState == BodyState_CrouchingDown) {
            retval -= ((PLAYER_MAX_WALK_SPEED - PLAYER_MAX_CROUCH_SPEED) * 1.5f); // Subtract off the difference in speed between walking and crouching from the sprint speed
        } else if (bodyState == BodyState_Prone || bodyState == BodyState_ProningDown || bodyState == BodyState_ProningUp) {
            retval -= ((PLAYER_MAX_WALK_SPEED - PLAYER_MAX_PRONE_SPEED) * 2.0f); // Subtract off the difference in speed between walking and proning from the sprint speed.
        }
    }

    return retval + bonus;
}

uint16_t testPointInSolid = UINT16_MAX;
uint16_t PointInSolid(Vector3 point, uint32_t layerMask) {
    for (uint16_t i = START_INDEX_LEVEL_INSTANCES; i < INSTANCE_COUNT; ++i) {
        if (instances[i].collider == COLLIDER_TYPE_NONE || instances[i].collider == COLLIDER_TYPE_MESH) continue;
        if ((instances[i].layer & layerMask)) continue;
        
        Vector3 pos = instances[i].position;
        Quaternion rot = instances[i].rotation;
        Vector3 scale = instances[i].scale;
        Vector3 local = Vector3_A_minus_B(point, pos);
        Quaternion invRot = (Quaternion){-rot.x, -rot.y, -rot.z, rot.w};
        local = quat_rotate(invRot, local);
        if (scale.x != 0.0f) local.x /= scale.x;
        if (scale.y != 0.0f) local.y /= scale.y;
        if (scale.z != 0.0f) local.z /= scale.z;
        Vector3 center;
        float radius, distSq;
        switch (instances[i].collider) {
            case COLLIDER_TYPE_BOX:
                center = instances[i].colliderCenter;
                Vector3 size   = instances[i].colliderSize; // full extents
                Vector3 half = (Vector3){ size.x * 0.5f, size.y * 0.5f, size.z * 0.5f };
                Vector3 min = Vector3_A_minus_B(center, half);
                Vector3 max = Vector3_A_plus_B(center, half);
                if (local.x >= min.x && local.x <= max.x && local.y >= min.y && local.y <= max.y && local.z >= min.z && local.z <= max.z) return i;
                break;
            case COLLIDER_TYPE_SPHERE:
                center = instances[i].colliderCenter;
                radius = instances[i].colliderSize.x;
                Vector3 offset = Vector3_A_minus_B(local, center);
                distSq = dot_vector3(offset, offset);
                if (distSq <= radius * radius) return i;
                break;
            case COLLIDER_TYPE_CAPSULE:
                center = instances[i].colliderCenter;
                radius = instances[i].colliderSize.x;
                float height = instances[i].colliderSize.y; // full cylinder height
                int axis = (int)instances[i].colliderSize.z; // 0=X, 1=Y, 2=Z
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

int32_t Physics(void) {
    if (window_has_focus && !log_playback) {
        if (!voxen_globalContext.gamePaused && !voxen_Cheats.consoleActive) UpdatePlayerFacingAngles();
        ProcessInput();
    }
            
    if (voxen_globalContext.gamePaused || voxen_globalContext.menuActive) return 0;

    testPointInSolid = PointInSolid(instances[PLAYER1].position, LAYER_MASK_PLAYER_COLLIDESWITH);
        
    if (voxen_Cheats.noclip) {
        instances[PLAYER1].collider = COLLIDER_TYPE_NONE;
        flag_disable(&instances[PLAYER1].entflags, ENTFLAG_USEGRAVITY);
        flag_disable(&instances[PLAYER1].entflags, ENTFLAG_GROUNDED);
        instances[PLAYER1].velocity = (Vector3){0.0f,0.0f,0.0f};
    } else {
        instances[PLAYER1].collider = COLLIDER_TYPE_CAPSULE;
        flag_enable(&instances[PLAYER1].entflags, ENTFLAG_USEGRAVITY);
    }
    
    if (!log_playback && !voxen_Cheats.consoleActive) {
        Entity* player = &instances[PLAYER1];
        Vector3 input = {0};
        if (keyStates[GLFW_KEY_F].down)     input = Vector3_A_plus_B(input, (Vector3){cam_forwardx, 0, cam_forwardz});
        if (keyStates[GLFW_KEY_S].down)     input = Vector3_A_minus_B(input, (Vector3){cam_forwardx, 0, cam_forwardz});
        if (keyStates[GLFW_KEY_D].down)     input = Vector3_A_plus_B(input, (Vector3){cam_rightx,   0, cam_rightz});
        if (keyStates[GLFW_KEY_A].down)     input = Vector3_A_minus_B(input, (Vector3){cam_rightx,   0, cam_rightz});
        if (keyStates[GLFW_KEY_C].down/* && noclip*/) input.y -= 1.0f; // Temporarily allow for now until I have collision working
        if (keyStates[GLFW_KEY_V].down/* && noclip*/) input.y += 1.0f;

        float sprintMul = keyStates[GLFW_KEY_LEFT_SHIFT].down ? 1.75f : 1.0f;
        const float moveForce = 1800.0f;

        float wishSpeed = magnitude_vector3(input);
        if (wishSpeed > 0.1f) {
            input = normalize_vector3(input);
            Vector3 force = scale_vector3(input, moveForce * sprintMul);
            player->velocity = Vector3_A_plus_B(player->velocity, force);
            float maxAchievableSpeed = GetBasePlayerSpeed(true);
            if (magnitude_vector3(player->velocity) > maxAchievableSpeed) player->velocity = scale_vector3(normalize_vector3(player->velocity),maxAchievableSpeed);
        }
            
        if (!voxen_Cheats.noclip && keyStates[GLFW_KEY_SPACE].pressed && (instances[PLAYER1].entflags & ENTFLAG_GROUNDED)) { // Jump
            AddForce(PLAYER1, (Vector3){0, 6.8f, 0}, FORCEMODE_IMPULSE);
            flag_set(&instances[PLAYER1].entflags, ENTFLAG_GROUNDED, false);
        }
    }
    
    if (instances[PLAYER1].entflags & ENTFLAG_USEGRAVITY) {
        Vector3 massGravityScale = scale_vector3((Vector3){0.0f,-9.81f,0.0f}, instances[PLAYER1].mass);
        instances[PLAYER1].accumulatedForce = Vector3_A_plus_B(instances[PLAYER1].accumulatedForce, massGravityScale);
    }
    
    Vector3 accel = scale_vector3(instances[PLAYER1].accumulatedForce, 1.0f / instances[PLAYER1].mass);
    instances[PLAYER1].velocity = Vector3_A_plus_B(instances[PLAYER1].velocity, scale_vector3(accel, (float)voxen_globalContext.timeSinceLastPhysicsTick));
    instances[PLAYER1].velocity = scale_vector3(instances[PLAYER1].velocity, vexp(-(instances[PLAYER1].linearDrag) * (float)voxen_globalContext.timeSinceLastPhysicsTick));
    Vector3 wishPos = Vector3_A_plus_B(instances[PLAYER1].position, scale_vector3(instances[PLAYER1].velocity, (float)voxen_globalContext.timeSinceLastPhysicsTick));
    instances[PLAYER1].angularVelocity = instances[PLAYER1].accumulatedTorque = (Vector3){0.0f,0.0f,0.0f};
    instances[PLAYER1].accumulatedForce = instances[PLAYER1].accumulatedTorque = (Vector3){0.0f,0.0f,0.0f};
    if (voxen_Cheats.noclip) { instances[PLAYER1].position = wishPos; return 0; }
    
    flag_disable(&instances[PLAYER1].entflags, ENTFLAG_GROUNDED);
    bool hit = false;
    uint16_t hitIndex = UINT16_MAX; // Start with invalid
    Vector3 finalHitNormal = (Vector3){ 0.0f, 0.0f, 0.0f };
    for (uint16_t entityWithColliderIndex = START_INDEX_LEVEL_INSTANCES; entityWithColliderIndex < loadedInstances; ++entityWithColliderIndex) {
        if (instances[entityWithColliderIndex].collider != COLLIDER_TYPE_BOX) continue;

        float capsuleHalfHeight = (PLAYER_HEIGHT * 0.5f) - PLAYER_RADIUS;  // distance from center to flat end
        Vector3 bottomCenter = { wishPos.x, wishPos.y - capsuleHalfHeight, wishPos.z };
        Vector3 topCenter    = { wishPos.x, wishPos.y + capsuleHalfHeight, wishPos.z };
        Vector3 quatVectorImaginary = {instances[entityWithColliderIndex].rotation.x,
                                       instances[entityWithColliderIndex].rotation.y,
                                       instances[entityWithColliderIndex].rotation.z};
        float quatScalarPremultiplied = instances[entityWithColliderIndex].rotation.w * 2.0f;
        Vector3 quatVectorImaginaryConjugate = (Vector3){ -quatVectorImaginary.x, -quatVectorImaginary.y, -quatVectorImaginary.z };

        Vector3 towardBottom = Vector3_A_minus_B(bottomCenter, instances[entityWithColliderIndex].position); // Direction vector == end - start
        Vector3 uvBottom = cross_vector3(quatVectorImaginaryConjugate, towardBottom);
        Vector3 localBottom = Vector3_A_plus_B(towardBottom,
                                               Vector3_A_plus_B(scale_vector3(uvBottom, quatScalarPremultiplied),
                                                                scale_vector3(cross_vector3(quatVectorImaginaryConjugate, uvBottom), 2.0f)));      
        
        Vector3 towardTop = Vector3_A_minus_B(topCenter, instances[entityWithColliderIndex].position); // Direction vector == end - start
        Vector3 uvTop = cross_vector3(quatVectorImaginaryConjugate, towardTop);
        Vector3 localTop = Vector3_A_plus_B(towardTop,
                                            Vector3_A_plus_B(scale_vector3(uvTop, quatScalarPremultiplied),
                                                             scale_vector3(cross_vector3(quatVectorImaginaryConjugate, uvTop), 2.0f)));      
        
        Vector3 boxHalf = { instances[entityWithColliderIndex].colliderSize.x * instances[entityWithColliderIndex].scale.x * 0.5f,
                            instances[entityWithColliderIndex].colliderSize.y * instances[entityWithColliderIndex].scale.y * 0.5f,
                            instances[entityWithColliderIndex].colliderSize.z * instances[entityWithColliderIndex].scale.z * 0.5f };
        
        Vector3 closestBottom = { vclamp(localBottom.x, -boxHalf.x, boxHalf.x), vclamp(localBottom.y, -boxHalf.y, boxHalf.y), vclamp(localBottom.z, -boxHalf.z, boxHalf.z) };
        Vector3 deltaBottom = Vector3_A_minus_B(localBottom, closestBottom);
        float distSqBottom = dot_vector3(deltaBottom, deltaBottom);
        float overlapBottom = (distSqBottom >= PLAYER_RADIUS * PLAYER_RADIUS) ? 0.0f : PLAYER_RADIUS - vsqrtf(distSqBottom);
        
        Vector3 closestTop = { vclamp(localTop.x, -boxHalf.x, boxHalf.x), vclamp(localTop.y, -boxHalf.y, boxHalf.y), vclamp(localTop.z, -boxHalf.z, boxHalf.z) };
        Vector3 deltaTop = Vector3_A_minus_B(localTop, closestTop);
        float distSqTop = dot_vector3(deltaTop, deltaTop);
        float overlapTop = (distSqTop >= PLAYER_RADIUS * PLAYER_RADIUS) ? 0.0f : PLAYER_RADIUS - vsqrtf(distSqTop);
        
        float largestOverlap = vmax(overlapTop, overlapBottom);
        if (largestOverlap <= 0.000001f) continue; // No hit

        Vector3 delta = Vector3_A_minus_B(overlapBottom >= overlapTop ? localBottom   : localTop,
                                          overlapBottom >= overlapTop ? closestBottom : closestTop);    // Direction vector == end - start
        Vector3 localNormal = normalize_vector3(delta);
        Vector3 uvLocalNormal = cross_vector3(quatVectorImaginary, localNormal);
        Vector3 hitnormal = Vector3_A_plus_B(localNormal,
                                             Vector3_A_plus_B(scale_vector3(uvLocalNormal, quatScalarPremultiplied),
                                                              scale_vector3(cross_vector3(quatVectorImaginary, uvLocalNormal), 2.0f)));        
        
        Vector3 newWishPos = Vector3_A_plus_B(wishPos, scale_vector3(hitnormal, largestOverlap + 0.02f));
        float magNew =     magnitude_vector3(Vector3_A_minus_B(newWishPos, instances[PLAYER1].position));
        float magCurrent = magnitude_vector3(Vector3_A_minus_B(wishPos ,instances[PLAYER1].position));
        if (magNew < magCurrent) {
            hit = true;
            wishPos = newWishPos;
            finalHitNormal = hitnormal;
            hitIndex = entityWithColliderIndex;
        }
    }
    
    if (hit) {
        instances[PLAYER1].position = wishPos; // Teleport player to the safe position.  Never position player until we know for sure we have a new position that's clear.
        DualLog("Hit entity with index %u\n",instances[hitIndex].index);
    }
    
    if (finalHitNormal.y > 0.65f) { // Simple grounded check, fairly upright surface
        flag_enable(&instances[PLAYER1].entflags, ENTFLAG_GROUNDED);
        if (instances[PLAYER1].velocity.y < 0.0f) instances[PLAYER1].velocity.y = 0.0f;
    }
    
    return 0;
}
