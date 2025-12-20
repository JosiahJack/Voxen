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

float fatigue;

typedef struct {
    Vector3 normal;
    float penetration;
    Vector3 contactPoint; // Max 4 for box resting on ground
} Manifold;

__attribute__((pure)) Vector3 GetWorldCenter(const Entity* e) {
    Vector3 scaledCenter = {e->colliderCenter.x * e->scale.x, e->colliderCenter.y * e->scale.y, e->colliderCenter.z * e->scale.z};
    Vector3 rotatedCenter = rotate_quaternion(e->rotation, scaledCenter);
    return add_vector3(e->position, rotatedCenter);
}

static inline bool CollideAABB(Vector3 min1, Vector3 max1, Vector3 min2, Vector3 max2) {
    return (min1.x <= max2.x && max1.x >= min2.x && min1.y <= max2.y && max1.y >= min2.y && min1.z <= max2.z && max1.z >= min2.z);
}

bool GetAABB(const Entity* e, Vector3* aabb_min, Vector3* aabb_max) {
    if (e->collider == COLLIDER_TYPE_NONE) return false;

    if (e->collider == COLLIDER_TYPE_SPHERE) {
        Vector3 c = GetWorldCenter(e);
        float r = e->colliderSize.x * e->scale.x;
        *aabb_min = (Vector3){c.x - r, c.y - r, c.z - r};
        *aabb_max = (Vector3){c.x + r, c.y + r, c.z + r};
        return true;
    } else if (e->collider == COLLIDER_TYPE_BOX) {
        Vector3 half = { e->colliderSize.x * e->scale.x * 0.5f, e->colliderSize.y * e->scale.y * 0.5f, e->colliderSize.z * e->scale.z * 0.5f };
        Vector3 corners[8] = {
            {-half.x, -half.y, -half.z},
            { half.x, -half.y, -half.z},
            {-half.x,  half.y, -half.z},
            { half.x,  half.y, -half.z},
            {-half.x, -half.y,  half.z},
            { half.x, -half.y,  half.z},
            {-half.x,  half.y,  half.z},
            { half.x,  half.y,  half.z}
        };

        Vector3 world_min = {FLT_MAX, FLT_MAX, FLT_MAX};
        Vector3 world_max = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
        for (int i = 0; i < 8; ++i) {
            Vector3 rotated = rotate_quaternion(e->rotation, corners[i]);
            Vector3 world = add_vector3(e->position, rotated);
            world_min = min_vector3(world_min, world);
            world_max = max_vector3(world_max, world);
        }

        *aabb_min = world_min;
        *aabb_max = world_max;
        return true;
    } else if (e->collider == COLLIDER_TYPE_CAPSULE) {
        float radius = e->colliderSize.x * e->scale.x;
        float totalHeight = e->colliderSize.y * e->scale.y;
        Vector3 local_min, local_max;
        if (vabs(e->colliderSize.z - COLLIDER_CAPSULE_DIRECTION_Y_F) < 0.1f) { 
            local_min = (Vector3){ -radius, -totalHeight * 0.5f, -radius };
            local_max = (Vector3){  radius,  totalHeight * 0.5f,  radius };
        } else if (vabs(e->colliderSize.z - COLLIDER_CAPSULE_DIRECTION_X_F) < 0.1f) {
            local_min = (Vector3){ -totalHeight * 0.5f, -radius, -radius };
            local_max = (Vector3){  totalHeight * 0.5f,  radius,  radius };
        } else {
            local_min = (Vector3){ -radius, -radius, -totalHeight * 0.5f };
            local_max = (Vector3){  radius,  radius,  totalHeight * 0.5f };
        }

        Vector3 corners[8] = {
            { local_min.x, local_min.y, local_min.z },
            { local_max.x, local_min.y, local_min.z },
            { local_min.x, local_max.y, local_min.z },
            { local_max.x, local_max.y, local_min.z },
            { local_min.x, local_min.y, local_max.z },
            { local_max.x, local_min.y, local_max.z },
            { local_min.x, local_max.y, local_max.z },
            { local_max.x, local_max.y, local_max.z }
        };

        Vector3 world_min = { FLT_MAX, FLT_MAX, FLT_MAX };
        Vector3 world_max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
        for (int i = 0; i < 8; ++i) {
            Vector3 rotated = rotate_quaternion(e->rotation, corners[i]);
            Vector3 world = add_vector3(e->position, rotated);
            world_min = min_vector3(world_min, world);
            world_max = max_vector3(world_max, world);
        }

        *aabb_min = world_min;
        *aabb_max = world_max;
        return true;
    } else if (e->collider == COLLIDER_TYPE_CONVEXMESH) {
        uint16_t modelIdx = e->colliderMeshIndex;
        if (modelIdx >= loadedModelsMaxIndex) { /*DualLogError("Invalid mesh for GetAABB: %u on entity with index: %u\n",modelIdx,e->index);*/ return false; }

        uint32_t base = modelIdx * BOUNDS_ATTRIBUTES_COUNT;
        Vector3 local_min = {
            modelBounds[base + BOUNDS_DATA_OFFSET_MINX],
            modelBounds[base + BOUNDS_DATA_OFFSET_MINY],
            modelBounds[base + BOUNDS_DATA_OFFSET_MINZ]
        };
        
        Vector3 local_max = {
            modelBounds[base + BOUNDS_DATA_OFFSET_MAXX],
            modelBounds[base + BOUNDS_DATA_OFFSET_MAXY],
            modelBounds[base + BOUNDS_DATA_OFFSET_MAXZ]
        };

        // Transform 8 corners of local AABB
        Vector3 world_min = {FLT_MAX, FLT_MAX, FLT_MAX};
        Vector3 world_max = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
        float signs[2] = {-1.0f, 1.0f};
        for (int ix = 0; ix < 2; ++ix) {
            for (int iy = 0; iy < 2; ++iy) {
                for (int iz = 0; iz < 2; ++iz) {
                    Vector3 local = {
                        (local_min.x + local_max.x)*0.5f + signs[ix] * (local_max.x - local_min.x)*0.5f,
                        (local_min.y + local_max.y)*0.5f + signs[iy] * (local_max.y - local_min.y)*0.5f,
                        (local_min.z + local_max.z)*0.5f + signs[iz] * (local_max.z - local_min.z)*0.5f
                    };
                    Vector3 rotated = rotate_quaternion(e->rotation, local);
                    Vector3 world = add_vector3(e->position, rotated);
                    world_min = min_vector3(world_min, world);
                    world_max = max_vector3(world_max, world);
                }
            }
        }
        *aabb_min = world_min;
        *aabb_max = world_max;
        return true;
    }
    
    DualLogError("Invalid collider for GetAABB\n");
    return false;
}

// static float dist_segment_segment_sq(Vector3 a0, Vector3 a1, Vector3 b0, Vector3 b1, Vector3* closest_a, Vector3* closest_b) {
//     Vector3 d1 = sub_vector3(a1, a0);
//     Vector3 d2 = sub_vector3(b1, b0);
//     Vector3 d0 = sub_vector3(a0, b0);
//     float a = dot_vector3(d1, d1);
//     float e = dot_vector3(d2, d2);
//     float f = dot_vector3(d2, d1);
//     float c = dot_vector3(d1, d0);
//     float d = dot_vector3(d2, d0);
//     float det = a * e - f * f;
//     float s = 0.5f, t = 0.5f;
//     if (det > 1e-6f) {
//         s = clampf((f * d - e * c) / det, 0.0f, 1.0f);
//         t = clampf((a * d - f * c) / det, 0.0f, 1.0f);
//     }
//     
//     *closest_a = add_vector3(a0, scale_vector3(d1, s));
//     *closest_b = add_vector3(b0, scale_vector3(d2, t));
//     Vector3 diff = sub_vector3(*closest_a, *closest_b);
//     return dot_vector3(diff, diff);
// }

// Check if instance is in 3x3 grid around object
static inline bool is_instance_in_neighbor_cells(uint32_t instanceCellIdx, uint32_t objectCellIdx) {
    uint32_t dx = (instanceCellIdx % WORLDX) - (objectCellIdx % WORLDX);
    uint32_t dz = (instanceCellIdx / WORLDZ) - (objectCellIdx / WORLDZ);
    return (dx * dx) <= 1 && (dz * dz) <= 1; // 0 means same cell, 1 means one of the 8 neighbors, accept all.
}

bool CollideSphereSphere(const Entity* a, const Entity* b, Manifold* m) {
    float ra = a->colliderSize.x * a->scale.x;
    Vector3 cb = GetWorldCenter(b);
    float rb = b->colliderSize.x * b->scale.x;
    Vector3 delta = sub_vector3(GetWorldCenter(a), cb);
    float dist = magnitude_vector3(delta);
    if (dist >= ra + rb || dist < 1e-6f) return false;
    
    m->penetration = ra + rb - dist;
    m->normal = (dist > 0) ? normalize_vector3(delta) : (Vector3){1.0f, 0.0f, 0.0f};
    m->contactPoint = add_vector3(cb, scale_vector3(m->normal, rb));
    return true;
}

bool CollideSphereBox(const Entity* sphere, const Entity* box, Manifold* m) {
    Vector3 ws = GetWorldCenter(sphere);
    float rs = sphere->colliderSize.x * sphere->scale.x;
    Vector3 temp = sub_vector3(ws, box->position);
    temp = rotate_quaternion(conjugate_quaternion(box->rotation), temp);
    Vector3 local_pos = { temp.x / box->scale.x - box->colliderCenter.x, temp.y / box->scale.y - box->colliderCenter.y, temp.z / box->scale.z - box->colliderCenter.z };
    Vector3 half = scale_vector3(box->colliderSize, 0.5f);
    Vector3 closest = { clampf(local_pos.x, -half.x, half.x), clampf(local_pos.y, -half.y, half.y), clampf(local_pos.z, -half.z, half.z) };
    Vector3 delta_local = sub_vector3(local_pos, closest);
    float dist_sq = dot_vector3(delta_local, delta_local);
    if (dist_sq > rs * rs + 1e-6f) return false;
    
    float dist = (dist_sq > 0) ? vsqrtf(dist_sq) : 0.0f;    
    m->penetration = rs - dist;
    if (dist > 0) {
        m->normal = normalize_vector3(delta_local);
    } else {
        float dx = half.x - vabs(local_pos.x);
        float dy = half.y - vabs(local_pos.y);
        float dz = half.z - vabs(local_pos.z);
        float min_pen = vmin(vmin(dx, dy), dz);
        m->penetration = min_pen;
        if (dx >= min_pen)      m->normal = (Vector3){local_pos.x > 0 ? 1.0f : -1.0f, 0, 0};
        else if (dy >= min_pen) m->normal = (Vector3){0, local_pos.y > 0 ? 1.0f : -1.0f, 0};
        else                    m->normal = (Vector3){0, 0, local_pos.z > 0 ? 1.0f : -1.0f};
    }
   
    m->normal = rotate_quaternion(box->rotation, m->normal);
    m->contactPoint = sub_vector3(ws, scale_vector3(m->normal, rs));
    return true;
}

// static bool CollideCapsuleCapsule(const Entity* a, const Entity* b, Manifold* m) {
//     Vector3 ca = GetWorldCenter(a);
//     float ra = a->colliderSize.x * a->scale.x;
//     float ha = a->colliderSize.y * a->scale.y * 0.5f - ra;
//     Vector3 cb = GetWorldCenter(b);
//     float rb = b->colliderSize.x * b->scale.x;
//     float hb = b->colliderSize.y * b->scale.y * 0.5f - rb;
//     Vector3 a0 = {ca.x, ca.y - ha, ca.z};
//     Vector3 a1 = {ca.x, ca.y + ha, ca.z};
//     Vector3 b0 = {cb.x, cb.y - hb, cb.z};
//     Vector3 b1 = {cb.x, cb.y + hb, cb.z};
//     Vector3 cA, cB;
//     float distSq = dist_segment_segment_sq(a0, a1, b0, b1, &cA, &cB);
//     float dist = vsqrtf(distSq);
//     if (dist >= ra + rb + 1e-6f) return false;
// 
//     m->penetration = ra + rb - dist;
//     m->normal = (dist > 0.0f) ? normalize_vector3(sub_vector3(cA, cB)) : (Vector3){0,1,0};
//     m->contactPoint = add_vector3(cB, scale_vector3(m->normal, rb));
//     return true;
// }

bool CollideConvexBox(const Entity* convex, const Entity* box, Manifold* m) {
    uint16_t modelIdx = convex->colliderMeshIndex;
    if (modelIdx >= loadedModelsMaxIndex) return false;

    Vector3 boxHalf = scale_vector3(box->colliderSize, 0.5f);
    uint32_t vcount = modelVertexCounts[modelIdx];
    float* verts = modelVertices[modelIdx];
    Vector3 bestNormal = {0,0,0};
    float bestPen = -FLT_MAX;
    bool hit = false;
    Vector3 boxFaces[6] = { {1,0,0}, {-1,0,0}, {0,1,0}, {0,-1,0}, {0,0,1}, {0,0,-1} };
    for (int i = 0; i < 6; ++i) {
        Vector3 n = rotate_quaternion(convex->rotation, boxFaces[i]);
        float d_convex = -FLT_MAX;
        float boxExtent = (i%2==0) ? boxHalf.x : (i>=2 && i<4) ? boxHalf.y : boxHalf.z;
        float d_box = (i%2==1) ? -boxExtent : boxExtent;
        for (uint32_t v = 0; v < vcount; ++v) {
            Vector3 p = { verts[v*VERTEX_ATTRIBUTES_COUNT + 0], verts[v*VERTEX_ATTRIBUTES_COUNT + 1], verts[v*VERTEX_ATTRIBUTES_COUNT + 2] };
            float d = dot_vector3(p, n);
            d_convex = vmax(d_convex, d);
        }
        
        float pen = d_box - d_convex;
        if (pen > 0 && pen > bestPen) {
            bestPen = pen;
            bestNormal = n;
            hit = true;
        }
    }

    if (!hit) return false;

    m->penetration = bestPen;
    m->normal = normalize_vector3(bestNormal);
    m->contactPoint = add_vector3(convex->position, scale_vector3(m->normal, -bestPen * 0.5f));
    return true;
}

float Combine(const float a, const float b, PhysCombineType combine) {
    switch (combine) {
        case PHYS_COMBINE_AVG: return (a + b) * 0.5f;
        case PHYS_COMBINE_MIN: return vmin(a, b);
        case PHYS_COMBINE_MUL: return vsqrtf(a * b);
        case PHYS_COMBINE_MAX: return vmax(a, b);
        default: return a; // or average
    }
}

// void ResolveCollision(Entity* a, Entity* b, Manifold* m) {
//     if (a->mass <= 0.0f && b->mass <= 0.0f) return;
// 
//     Vector3 ra = sub_vector3(m->contactPoint, a->position);
//     Vector3 va = add_vector3(a->velocity, cross_vector3(a->angularVelocity, ra));
//     Vector3 rb = sub_vector3(m->contactPoint, b->position);
//     Vector3 vb = add_vector3(b->velocity, cross_vector3(b->angularVelocity, rb));
//     Vector3 rel_vel = sub_vector3(va, vb);
//     float vel_n = dot_vector3(rel_vel, m->normal);
//     if (vel_n > 0.0f) return;
//     
//     float inv_ma = (a->mass > 0.0f) ? 1.0f / a->mass : 0.0f;
//     float inv_mb = (b->mass > 0.0f) ? 1.0f / b->mass : 0.0f;
//     float inv_m_sum = inv_ma + inv_mb;
//     if (inv_m_sum < 1e-6f) return;
//     
//     float e = Combine(a->bounciness, b->bounciness, a->bounceCombine);
//     float j = -(1.0f + e) * vel_n / inv_m_sum;
//     Vector3 impulse = scale_vector3(m->normal, j);
//     if (a->mass > 0.0f) a->velocity = sub_vector3(a->velocity, scale_vector3(impulse, inv_ma));
//     if (b->mass > 0.0f) b->velocity = add_vector3(b->velocity, scale_vector3(impulse, inv_mb));
//     Vector3 tangent_vel = sub_vector3(rel_vel, scale_vector3(m->normal, vel_n));
//     float tangent_len = magnitude_vector3(tangent_vel);
//     if (tangent_len > 1e-6f) {
//         Vector3 tangent = normalize_vector3(tangent_vel);
//         float mu_d = Combine(a->dynamicFriction, b->dynamicFriction, a->frictionCombine);
//         float jt_max = vabs(j) * mu_d;
//         float jt = -tangent_len / inv_m_sum;
//         if (vabs(jt) > jt_max) jt = (jt > 0.0f ? jt_max : -jt_max);
//         Vector3 friction_impulse = scale_vector3(tangent, jt);
//         if (a->mass > 0.0f) a->velocity = sub_vector3(a->velocity, scale_vector3(friction_impulse, inv_ma));
//         if (b->mass > 0.0f) b->velocity = add_vector3(b->velocity, scale_vector3(friction_impulse, inv_mb));
//     }
// 
//     if (!(ConstIndexIsNPC(a->index) || a == &instances[PLAYER1] || a == &instances[PLAYER2])) {
//         if (a->inertia > 0.0f) {
//             float inv_ia = 1.0f / a->inertia;
//             Vector3 torque_a = cross_vector3(ra, impulse);
//             a->angularVelocity = add_vector3(a->angularVelocity, scale_vector3(torque_a, inv_ia));
//         }
//     }
//     
//     if (!(ConstIndexIsNPC(b->index) || b == &instances[PLAYER1] || b == &instances[PLAYER2])) {
//         if (b->inertia > 0.0f) {
//             float inv_ib = 1.0f / b->inertia;
//             Vector3 torque_b = cross_vector3(rb, scale_vector3(impulse, -1.0f));
//             b->angularVelocity = add_vector3(b->angularVelocity, scale_vector3(torque_b, inv_ib));
//         }
//     }
// 
//     float percent = 0.2f;
//     float slop = 0.01f;
//     float correction = vmax(m->penetration - slop, 0.0f) * percent / inv_m_sum;
//     Vector3 correction_vec = scale_vector3(m->normal, correction);
//     if (a->mass > 0.0f) a->position = sub_vector3(a->position, scale_vector3(correction_vec, inv_ma));
//     if (b->mass > 0.0f) b->position = add_vector3(b->position, scale_vector3(correction_vec, inv_mb));
//     dirtyInstances[a - instances] = true;
//     dirtyInstances[b - instances] = true;
// }

// AddForce (mimics Unity AddForce; isImpulse=true for ForceMode.Impulse, false for Force).
void AddForce(uint16_t idx, Vector3 force, bool isImpulse) {
    Entity* e = &instances[idx];
    if ((e->entflags & ENTFLAG_KINEMATIC) || e->mass <= 0.0f) return;
    if (isImpulse) {
        e->velocity = add_vector3(e->velocity, scale_vector3(force, 1.0f / e->mass));
    } else {
        e->accumulatedForce = add_vector3(e->accumulatedForce, force);
    }
}

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
//         e->velocity = add_vector3(e->velocity, scale_vector3(force, 1.0f / e->mass));
//     } else {
//         e->accumulatedForce = add_vector3(e->accumulatedForce, force);
//     }
//     // Torque (world space).
//     if (! (ConstIndexIsNPC(e->index) || idx == PLAYER1 || idx == PLAYER2) && e->inertia > 0.0f) {
//         Vector3 relPos = sub_vector3(position, e->position);
//         Vector3 torque = cross_vector3(relPos, force);
//         if (isImpulse) {
//             // For impulse torque, integrate angVel directly (approx).
//             float invI = 1.0f / e->inertia;
//             e->angularVelocity = add_vector3(e->angularVelocity, scale_vector3(torque, invI));
//         } else {
//             e->accumulatedTorque = add_vector3(e->accumulatedTorque, torque);
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
//     if (isImpulse) e->velocity = add_vector3(e->velocity, scale_vector3(explosionForce, 1.0f / e->mass));
//     else           e->accumulatedForce = add_vector3(e->accumulatedForce, explosionForce);
// }

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

static bool CollideCapsuleBox(const Entity* player, const Entity* box, Manifold* m) {
    float radius = PLAYER_RADIUS;                    // assumes player scale = 1,1,1 — adjust if needed
    float capsuleHalfHeight = (PLAYER_HEIGHT * 0.5f) - radius;  // distance from center to flat end
    Vector3 playerCenter = player->position;

    Vector3 bottomCenter = { playerCenter.x, playerCenter.y - capsuleHalfHeight, playerCenter.z };
    Vector3 topCenter    = { playerCenter.x, playerCenter.y + capsuleHalfHeight, playerCenter.z };

    // Transform capsule points into box local space
    Quaternion invRot = conjugate_quaternion(box->rotation);
    Vector3 localBottom = rotate_quaternion(invRot, sub_vector3(bottomCenter, box->position));
    Vector3 localTop    = rotate_quaternion(invRot, sub_vector3(topCenter,    box->position));
    Vector3 localCenter = rotate_quaternion(invRot, sub_vector3(playerCenter, box->position));

    // Box half extents (assuming colliderSize is full width/height/depth)
    Vector3 boxHalf = { box->colliderSize.x * box->scale.x * 0.5f,
                        box->colliderSize.y * box->scale.y * 0.5f,
                        box->colliderSize.z * box->scale.z * 0.5f };

    // Helper: get penetration depth for a sphere center in box local space
    float GetSpherePenetration(Vector3 localSphereCenter) {
        Vector3 closest = {
            clampf(localSphereCenter.x, -boxHalf.x, boxHalf.x),
            clampf(localSphereCenter.y, -boxHalf.y, boxHalf.y),
            clampf(localSphereCenter.z, -boxHalf.z, boxHalf.z)
        };
        Vector3 delta = sub_vector3(localSphereCenter, closest);
        float distSq = dot_vector3(delta, delta);
        if (distSq >= radius * radius) return 0.0f;
        return radius - vsqrtf(distSq);
    }

    float penBottom = GetSpherePenetration(localBottom);
    float penTop    = GetSpherePenetration(localTop);

    // Also check the cylinder part (infinite cylinder around capsule axis)
    Vector3 closestOnBox = {
        clampf(localCenter.x, -boxHalf.x, boxHalf.x),
        clampf(localCenter.y, -boxHalf.y, boxHalf.y),
        clampf(localCenter.z, -boxHalf.z, boxHalf.z)
    };
    Vector3 cylDelta = sub_vector3(localCenter, closestOnBox);
    float cylDistSq = cylDelta.x * cylDelta.x + cylDelta.z * cylDelta.z;  // ignore Y (capsule axis)
    float penCylinder = 0.0f;
    if (cylDistSq < radius * radius) {
        float cylDist = vsqrtf(cylDistSq);
        penCylinder = radius - cylDist;
    }

    // Take deepest penetration
    float bestPen = penBottom;
    Vector3 localNormal = {0,0,0};
    if (penTop > bestPen) bestPen = penTop;
    if (penCylinder > bestPen) bestPen = penCylinder;

    if (bestPen <= 0.0f) return false;

    // Determine normal based on which part penetrated deepest
    if (penBottom >= penTop && penBottom >= penCylinder) {
        Vector3 closest = { clampf(localBottom.x, -boxHalf.x, boxHalf.x),
                            clampf(localBottom.y, -boxHalf.y, boxHalf.y),
                            clampf(localBottom.z, -boxHalf.z, boxHalf.z) };
        Vector3 delta = sub_vector3(closest, localBottom);  // closest → sphere center
        localNormal = normalize_vector3(delta);
    }
    else if (penTop >= penCylinder)
    {
        Vector3 closest = { clampf(localTop.x, -boxHalf.x, boxHalf.x),
                            clampf(localTop.y, -boxHalf.y, boxHalf.y),
                            clampf(localTop.z, -boxHalf.z, boxHalf.z) };
        Vector3 delta = sub_vector3(closest, localTop);
        localNormal = normalize_vector3(delta);
    }
    else
    {
        // Cylinder hit — cylDelta already points from box surface to capsule center
        Vector3 cylNormalXZ = { cylDelta.x, 0.0f, cylDelta.z };
        float len = magnitude_vector3(cylNormalXZ);
        if (len > 1e-5f) {
            localNormal = scale_vector3(cylNormalXZ, 1.0f / len);  // NO inversion!
        } else {
            localNormal = (Vector3){0.0f, 1.0f, 0.0f};  // safe fallback
        }
    }

    // Safety: if normal is zero (shouldn't happen), default to up
    if (magnitude_vector3(localNormal) < 1e-4f) {
        localNormal = (Vector3){0.0f, 1.0f, 0.0f};
    }

    m->penetration = bestPen + 0.004f;  // small bias to prevent sinking
    m->normal = rotate_quaternion(box->rotation, localNormal);
    m->contactPoint = add_vector3(playerCenter, scale_vector3(m->normal, -radius));

    return true;
}

void IntegratePhysics(double dt) {
    if (!log_playback && !voxen_Cheats.consoleActive) {
        Entity* player = &instances[PLAYER1];
        Vector3 input = {0};
        if (keyStates[GLFW_KEY_F].down)     input = add_vector3(input, (Vector3){cam_forwardx, 0, cam_forwardz});
        if (keyStates[GLFW_KEY_S].down)     input = sub_vector3(input, (Vector3){cam_forwardx, 0, cam_forwardz});
        if (keyStates[GLFW_KEY_D].down)     input = add_vector3(input, (Vector3){cam_rightx,   0, cam_rightz});
        if (keyStates[GLFW_KEY_A].down)     input = sub_vector3(input, (Vector3){cam_rightx,   0, cam_rightz});
        if (keyStates[GLFW_KEY_C].down/* && noclip*/) input.y -= 1.0f; // Temporarily allow for now until I have collision working
        if (keyStates[GLFW_KEY_V].down/* && noclip*/) input.y += 1.0f;

        float sprintMul = keyStates[GLFW_KEY_LEFT_SHIFT].down ? 1.75f : 1.0f;
        const float moveForce = 1800.0f;

        float floatWishSpeed = magnitude_vector3(input);
        if (floatWishSpeed > 0.1f) {
            input = normalize_vector3(input);
            Vector3 force = scale_vector3(input, moveForce * sprintMul);
            player->velocity = add_vector3(player->velocity, force);
            float maxAchievableSpeed = GetBasePlayerSpeed(true);
            if (magnitude_vector3(player->velocity) > maxAchievableSpeed) player->velocity = scale_vector3(normalize_vector3(player->velocity),maxAchievableSpeed);
        }
            
        if (!voxen_Cheats.noclip && keyStates[GLFW_KEY_SPACE].pressed && (instances[PLAYER1].entflags & ENTFLAG_GROUNDED)) { // Jump
            AddForce(PLAYER1, (Vector3){0, 6.8f, 0}, FORCEMODE_IMPULSE);
            flag_set(&instances[PLAYER1].entflags, ENTFLAG_GROUNDED, false);
        }
    }
    
    for (uint16_t i = PLAYER1; i < /*loadedInstances*/ PLAYER1 + 1; ++i) { // Temporarily only update player's position
        Entity* e = &instances[i];
        if ((!(e->entflags & ENTFLAG_ACTIVE)) || (e->entflags & ENTFLAG_KINEMATIC) || e->mass <= 0.0f) continue;

        if ((e->entflags & ENTFLAG_USEGRAVITY)) e->accumulatedForce = add_vector3(e->accumulatedForce, scale_vector3((Vector3){0.0f,-9.81f,0.0f}, e->mass));
        Vector3 accel = scale_vector3(e->accumulatedForce, 1.0f / e->mass);
        e->velocity = add_vector3(e->velocity, scale_vector3(accel, (float)dt));
        if (e->linearDrag > 0.0001f) {
            float exp_factor = vexp(-(e->linearDrag) * (float)dt);
            e->velocity = scale_vector3(e->velocity, exp_factor);
        }
        e->position = add_vector3(e->position, scale_vector3(e->velocity, (float)dt));
        if (ConstIndexIsNPC(e->index) || i == PLAYER1 || i == PLAYER2) {
            e->angularVelocity = e->accumulatedTorque = (Vector3){0.0f,0.0f,0.0f};
        } else if (e->inertia > 0.0f) {
            float invI = 1.0f / e->inertia;
            Vector3 angAccel = scale_vector3(e->accumulatedTorque, invI);
            e->angularVelocity = add_vector3(e->angularVelocity, scale_vector3(angAccel, (float)dt));
            e->angularVelocity = scale_vector3(e->angularVelocity, (1.0f - e->angularDrag * (float)dt));
            float angMag = magnitude_vector3(e->angularVelocity);
            if (angMag > 1e-6f) {
                float angle = angMag * (float)dt;
                Vector3 axis = normalize_vector3(e->angularVelocity);
                Quaternion deltaQ = axis_angle_quaternion(axis, angle);
                e->rotation = mul_quaternion(deltaQ, e->rotation);
                normalize_quaternion(&e->rotation);
            }
        }

        e->accumulatedForce = e->accumulatedTorque = (Vector3){0.0f,0.0f,0.0f};
    }
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
    
    IntegratePhysics(voxen_globalContext.timeSinceLastPhysicsTick);
    if (voxen_Cheats.noclip) return 0;
    
    Entity* player = &instances[PLAYER1];
    Vector3 playerMin, playerMax;
    GetAABB(player, &playerMin, &playerMax);
//     bool wasGrounded = (player->entflags & ENTFLAG_GROUNDED) != 0;
    flag_disable(&player->entflags, ENTFLAG_GROUNDED);
    for (uint16_t i = START_INDEX_LEVEL_INSTANCES; i < loadedInstances; ++i) {
        Entity* box = &instances[i];
        if (box->collider != COLLIDER_TYPE_BOX) continue;

        Vector3 boxMin, boxMax;
        if (!GetAABB(box, &boxMin, &boxMax)) continue;
        if (!CollideAABB(playerMin, playerMax, boxMin, boxMax)) continue;

        Manifold m = {0};
        if (CollideCapsuleBox(player, box, &m)) {
            player->position = add_vector3(player->position, scale_vector3(m.normal, m.penetration)); // Push player out (strong correction since world is static)
            if (m.normal.y > 0.65f) { // Simple grounded check, fairly upright surface
                flag_enable(&player->entflags, ENTFLAG_GROUNDED);
                if (player->velocity.y < 0.0f) player->velocity.y = 0.0f;
            }
        }
    }

    // Update player matrix
    dirtyInstances[PLAYER1] = true;
    UpdateInstanceMatrix(PLAYER1);
    return 0;
}
