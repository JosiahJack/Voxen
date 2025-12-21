// physics.c - Physics System
#include "voxen.h"
#include "event.h"
#include "entity.h"
#include "matvecquat.h"
#include "vmath.h"
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

// __attribute__((pure)) Vector3 GetWorldCenter(const Entity* e) {
//     Vector3 scaledCenter = {e->colliderCenter.x * e->scale.x, e->colliderCenter.y * e->scale.y, e->colliderCenter.z * e->scale.z};
//     Vector3 rotatedCenter = rotate_quaternion(e->rotation, scaledCenter);
//     return Vector3_A_plus_B(e->position, rotatedCenter);
// }

// static inline bool CollideAABB(Vector3 min1, Vector3 max1, Vector3 min2, Vector3 max2) {
//     return (min1.x <= max2.x && max1.x >= min2.x && min1.y <= max2.y && max1.y >= min2.y && min1.z <= max2.z && max1.z >= min2.z);
// }

// bool GetAABB(const Entity* e, Vector3* aabb_min, Vector3* aabb_max) {
//     if (e->collider == COLLIDER_TYPE_NONE) return false;
// 
//     if (e->collider == COLLIDER_TYPE_SPHERE) {
//         Vector3 c = GetWorldCenter(e);
//         float r = e->colliderSize.x * e->scale.x;
//         *aabb_min = (Vector3){c.x - r, c.y - r, c.z - r};
//         *aabb_max = (Vector3){c.x + r, c.y + r, c.z + r};
//         return true;
//     } else if (e->collider == COLLIDER_TYPE_BOX) {
//         Vector3 half = { e->colliderSize.x * e->scale.x * 0.5f, e->colliderSize.y * e->scale.y * 0.5f, e->colliderSize.z * e->scale.z * 0.5f };
//         Vector3 corners[8] = {
//             {-half.x, -half.y, -half.z},
//             { half.x, -half.y, -half.z},
//             {-half.x,  half.y, -half.z},
//             { half.x,  half.y, -half.z},
//             {-half.x, -half.y,  half.z},
//             { half.x, -half.y,  half.z},
//             {-half.x,  half.y,  half.z},
//             { half.x,  half.y,  half.z}
//         };
// 
//         Vector3 world_min = {FLT_MAX, FLT_MAX, FLT_MAX};
//         Vector3 world_max = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
//         for (int i = 0; i < 8; ++i) {
//             Vector3 rotated = rotate_quaternion(e->rotation, corners[i]);
//             Vector3 world = Vector3_A_plus_B(e->position, rotated);
//             world_min = min_vector3(world_min, world);
//             world_max = max_vector3(world_max, world);
//         }
// 
//         *aabb_min = world_min;
//         *aabb_max = world_max;
//         return true;
//     } else if (e->collider == COLLIDER_TYPE_CAPSULE) {
//         float radius = e->colliderSize.x * e->scale.x;
//         float totalHeight = e->colliderSize.y * e->scale.y;
//         Vector3 local_min, local_max;
//         if (vabs(e->colliderSize.z - COLLIDER_CAPSULE_DIRECTION_Y_F) < 0.1f) { 
//             local_min = (Vector3){ -radius, -totalHeight * 0.5f, -radius };
//             local_max = (Vector3){  radius,  totalHeight * 0.5f,  radius };
//         } else if (vabs(e->colliderSize.z - COLLIDER_CAPSULE_DIRECTION_X_F) < 0.1f) {
//             local_min = (Vector3){ -totalHeight * 0.5f, -radius, -radius };
//             local_max = (Vector3){  totalHeight * 0.5f,  radius,  radius };
//         } else {
//             local_min = (Vector3){ -radius, -radius, -totalHeight * 0.5f };
//             local_max = (Vector3){  radius,  radius,  totalHeight * 0.5f };
//         }
// 
//         Vector3 corners[8] = {
//             { local_min.x, local_min.y, local_min.z },
//             { local_max.x, local_min.y, local_min.z },
//             { local_min.x, local_max.y, local_min.z },
//             { local_max.x, local_max.y, local_min.z },
//             { local_min.x, local_min.y, local_max.z },
//             { local_max.x, local_min.y, local_max.z },
//             { local_min.x, local_max.y, local_max.z },
//             { local_max.x, local_max.y, local_max.z }
//         };
// 
//         Vector3 world_min = { FLT_MAX, FLT_MAX, FLT_MAX };
//         Vector3 world_max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
//         for (int i = 0; i < 8; ++i) {
//             Vector3 rotated = rotate_quaternion(e->rotation, corners[i]);
//             Vector3 world = Vector3_A_plus_B(e->position, rotated);
//             world_min = min_vector3(world_min, world);
//             world_max = max_vector3(world_max, world);
//         }
// 
//         *aabb_min = world_min;
//         *aabb_max = world_max;
//         return true;
//     } else if (e->collider == COLLIDER_TYPE_CONVEXMESH) {
//         uint16_t modelIdx = e->colliderMeshIndex;
//         if (modelIdx >= loadedModelsMaxIndex) { /*DualLogError("Invalid mesh for GetAABB: %u on entity with index: %u\n",modelIdx,e->index);*/ return false; }
// 
//         uint32_t base = modelIdx * BOUNDS_ATTRIBUTES_COUNT;
//         Vector3 local_min = {
//             modelBounds[base + BOUNDS_DATA_OFFSET_MINX],
//             modelBounds[base + BOUNDS_DATA_OFFSET_MINY],
//             modelBounds[base + BOUNDS_DATA_OFFSET_MINZ]
//         };
//         
//         Vector3 local_max = {
//             modelBounds[base + BOUNDS_DATA_OFFSET_MAXX],
//             modelBounds[base + BOUNDS_DATA_OFFSET_MAXY],
//             modelBounds[base + BOUNDS_DATA_OFFSET_MAXZ]
//         };
// 
//         // Transform 8 corners of local AABB
//         Vector3 world_min = {FLT_MAX, FLT_MAX, FLT_MAX};
//         Vector3 world_max = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
//         float signs[2] = {-1.0f, 1.0f};
//         for (int ix = 0; ix < 2; ++ix) {
//             for (int iy = 0; iy < 2; ++iy) {
//                 for (int iz = 0; iz < 2; ++iz) {
//                     Vector3 local = {
//                         (local_min.x + local_max.x)*0.5f + signs[ix] * (local_max.x - local_min.x)*0.5f,
//                         (local_min.y + local_max.y)*0.5f + signs[iy] * (local_max.y - local_min.y)*0.5f,
//                         (local_min.z + local_max.z)*0.5f + signs[iz] * (local_max.z - local_min.z)*0.5f
//                     };
//                     Vector3 rotated = rotate_quaternion(e->rotation, local);
//                     Vector3 world = Vector3_A_plus_B(e->position, rotated);
//                     world_min = min_vector3(world_min, world);
//                     world_max = max_vector3(world_max, world);
//                 }
//             }
//         }
//         *aabb_min = world_min;
//         *aabb_max = world_max;
//         return true;
//     }
//     
//     DualLogError("Invalid collider for GetAABB\n");
//     return false;
// }

// Check if instance is in 3x3 grid around object
// static inline bool is_instance_in_neighbor_cells(uint32_t instanceCellIdx, uint32_t objectCellIdx) {
//     uint32_t dx = (instanceCellIdx % WORLDX) - (objectCellIdx % WORLDX);
//     uint32_t dz = (instanceCellIdx / WORLDZ) - (objectCellIdx / WORLDZ);
//     return (dx * dx) <= 1 && (dz * dz) <= 1; // 0 means same cell, 1 means one of the 8 neighbors, accept all.
// }
// 
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

int32_t Physics(void) {
    if (window_has_focus && !log_playback) {
        if (!voxen_globalContext.gamePaused && !voxen_Cheats.consoleActive) UpdatePlayerFacingAngles();
        ProcessInput();
    }
            
    if (voxen_globalContext.gamePaused || voxen_globalContext.menuActive) return 0;

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

        float floatWishSpeed = magnitude_vector3(input);
        if (floatWishSpeed > 0.1f) {
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
