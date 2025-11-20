// physics.c - Physics System
#include "voxen.h"
#include "entity.h"
#include "matvecquat.h"
#include "vmath.h"
void ProcessInput(void);
void UpdatePlayerFacingAngles(void);

// Player
#define PLAYER_CROUCH_RATIO 0.6f
#define PLAYER_PRONE_RATIO 0.2f
#define PLAYER_TRANSITION_TO_PRONE_ADD 0.1f
bool noclip = true; // Testing, TODO
bool god = true;
bool notarget = false;
bool fatigueCheat = false;
bool redbull = false;

typedef struct {
    Vector3 normal;
    float penetration;
    Vector3 contactPoint; // Max 4 for box resting on ground
} Manifold;

Quaternion mul_quaternion(const Quaternion a, const Quaternion b) {
    Quaternion res;
    res.w = a.w * b.w - dot_vector3((Vector3){a.x, a.y, a.z}, (Vector3){b.x, b.y, b.z});
    Vector3 cross_part = cross_vector3((Vector3){a.x, a.y, a.z}, (Vector3){b.x, b.y, b.z});
    Vector3 w_parts = scale_vector3((Vector3){a.x, a.y, a.z}, b.w);
    Vector3 wb_parts = scale_vector3((Vector3){b.x, b.y, b.z}, a.w);
    Vector3 xyz = add_vector3(add_vector3(cross_part, w_parts), wb_parts);
    res.x = xyz.x; res.y = xyz.y; res.z = xyz.z;
    return res;
}

Vector3 rotate_quaternion(const Quaternion q, const Vector3 v) {
    Quaternion vq = {0.0f, v.x, v.y, v.z};
    Quaternion temp = mul_quaternion(vq, conjugate_quaternion(q));
    Quaternion result = mul_quaternion(q, temp);
    return (Vector3){result.x, result.y, result.z};
}

Vector3 GetWorldCenter(const Entity* e) {
    Vector3 scaledCenter = {e->colliderCenter.x * e->scale.x, e->colliderCenter.y * e->scale.y, e->colliderCenter.z * e->scale.z};
    Vector3 rotatedCenter = rotate_quaternion(e->rotation, scaledCenter);
    return add_vector3(e->position, rotatedCenter);
}

static inline bool CollideAABB(Vector3 min1, Vector3 max1, Vector3 min2, Vector3 max2) {
    return (min1.x <= max2.x && max1.x >= min2.x &&
            min1.y <= max2.y && max1.y >= min2.y &&
            min1.z <= max2.z && max1.z >= min2.z);
}

bool GetAABB(const Entity* e, Vector3* aabb_min, Vector3* aabb_max) {
    if (e->collider == COLLIDER_TYPE_NONE) return false;

    if (e->collider == COLLIDER_TYPE_SPHERE) {
        Vector3 c = GetWorldCenter(e);
        float r = e->colliderSize.x * e->scale.x;
        *aabb_min = (Vector3){c.x - r, c.y - r, c.z - r};
        *aabb_max = (Vector3){c.x + r, c.y + r, c.z + r};
        return true;

    } else if (e->collider == COLLIDER_TYPE_BOX || e->collider == COLLIDER_TYPE_CAPSULE) {
        // Local half-extents (including scale)
        Vector3 half = {
            e->colliderSize.x * e->scale.x * 0.5f,
            e->colliderSize.y * e->scale.y * 0.5f,
            e->colliderSize.z * e->scale.z * 0.5f
        };

        // 8 corners in local space
        Vector3 corners[8] = {
            { -half.x, -half.y, -half.z },
            {  half.x, -half.y, -half.z },
            { -half.x,  half.y, -half.z },
            {  half.x,  half.y, -half.z },
            { -half.x, -half.y,  half.z },
            {  half.x, -half.y,  half.z },
            { -half.x,  half.y,  half.z },
            {  half.x,  half.y,  half.z }
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
        if (modelIdx >= loadedModels) { DualLogError("Invalid mesh for GetAABB\n"); return false; }

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

// ================================= Collision Detection ==================================
// Segment to segment squared distance (clamped)
static float dist_segment_segment_sq(Vector3 a0, Vector3 a1, Vector3 b0, Vector3 b1, Vector3* closest_a, Vector3* closest_b) {
    Vector3 d1 = sub_vector3(a1, a0);
    Vector3 d2 = sub_vector3(b1, b0);
    Vector3 d0 = sub_vector3(a0, b0);
    float a = dot_vector3(d1, d1);
    float e = dot_vector3(d2, d2);
    float f = dot_vector3(d2, d1);
    float c = dot_vector3(d1, d0);
    float d = dot_vector3(d2, d0);
    float det = a * e - f * f;
    float s = 0.5f, t = 0.5f;
    if (det > 1e-6f) {
        s = clampf((f * d - e * c) / det, 0.0f, 1.0f);
        t = clampf((a * d - f * c) / det, 0.0f, 1.0f);
    }
    *closest_a = add_vector3(a0, scale_vector3(d1, s));
    *closest_b = add_vector3(b0, scale_vector3(d2, t));
    Vector3 diff = sub_vector3(*closest_a, *closest_b);
    return dot_vector3(diff, diff);
}

// Check if instance is in 3x3 grid around object
static inline bool is_instance_in_neighbor_cells(uint32_t instanceCellIdx, uint32_t objectCellIdx) {
    int32_t dx = (instanceCellIdx % WORLDX) - (objectCellIdx % WORLDX);
    int32_t dz = (instanceCellIdx / WORLDZ) - (objectCellIdx / WORLDZ);
    return (dx * dx) <= 1 && (dz * dz) <= 1; // 0 means same cell, 1 means one of the 8 neighbors, accept all.
}

bool CollideSphereSphere(const Entity* a, const Entity* b, Manifold* m) {
    Vector3 ca = GetWorldCenter(a);
    float ra = a->colliderSize.x * a->scale.x;
    Vector3 cb = GetWorldCenter(b);
    float rb = b->colliderSize.x * b->scale.x;
    Vector3 delta = sub_vector3(ca, cb);
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
    
    // Transform to box local space
    Vector3 temp = sub_vector3(ws, box->position);
    temp = rotate_quaternion(conjugate_quaternion(box->rotation), temp);
    Vector3 local_pos = {
        temp.x / box->scale.x - box->colliderCenter.x,
        temp.y / box->scale.y - box->colliderCenter.y,
        temp.z / box->scale.z - box->colliderCenter.z
    };
    Vector3 half = scale_vector3(box->colliderSize, 0.5f);
    Vector3 closest = {
        clampf(local_pos.x, -half.x, half.x),
        clampf(local_pos.y, -half.y, half.y),
        clampf(local_pos.z, -half.z, half.z)
    };
    Vector3 delta_local = sub_vector3(local_pos, closest);
    float dist_sq = dot_vector3(delta_local, delta_local);
    if (dist_sq > rs * rs + 1e-6f) return false;
    float dist = (dist_sq > 0) ? vsqrtf(dist_sq) : 0.0f;
    
    // Penetration, stop.  Don't.  No.
    m->penetration = rs - dist;
    if (dist > 0) {
        m->normal = normalize_vector3(delta_local);
    } else {
        // Inside: choose escape direction
        float dx = half.x - vabs(local_pos.x);
        float dy = half.y - vabs(local_pos.y);
        float dz = half.z - vabs(local_pos.z);
        float min_pen = vmin(vmin(dx, dy), dz);
        m->penetration = min_pen;
        if (dx == min_pen) {
            m->normal = (Vector3){local_pos.x > 0 ? 1.0f : -1.0f, 0, 0};
        } else if (dy == min_pen) {
            m->normal = (Vector3){0, local_pos.y > 0 ? 1.0f : -1.0f, 0};
        } else {
            m->normal = (Vector3){0, 0, local_pos.z > 0 ? 1.0f : -1.0f};
        }
    }
   
    m->normal = rotate_quaternion(box->rotation, m->normal); // Normal to world
    m->contactPoint = sub_vector3(ws, scale_vector3(m->normal, rs)); // On sphere surface
    return true;
}

static bool CollideCapsuleCapsule(const Entity* a, const Entity* b, Manifold* m) {
    Vector3 ca = GetWorldCenter(a);
    float ra = a->colliderSize.x * a->scale.x;
    float ha = a->colliderSize.y * a->scale.y * 0.5f - ra;   // half-height of line segment

    Vector3 cb = GetWorldCenter(b);
    float rb = b->colliderSize.x * b->scale.x;
    float hb = b->colliderSize.y * b->scale.y * 0.5f - rb;

    Vector3 a0 = {ca.x, ca.y - ha, ca.z};
    Vector3 a1 = {ca.x, ca.y + ha, ca.z};
    Vector3 b0 = {cb.x, cb.y - hb, cb.z};
    Vector3 b1 = {cb.x, cb.y + hb, cb.z};

    Vector3 cA, cB;
    float distSq = dist_segment_segment_sq(a0, a1, b0, b1, &cA, &cB);
    float dist = vsqrtf(distSq);
    if (dist >= ra + rb + 1e-6f) return false;

    m->penetration = ra + rb - dist;
    m->normal = (dist > 0.0f) ? normalize_vector3(sub_vector3(cA, cB))
                             : (Vector3){0,1,0}; // fallback
    m->contactPoint = add_vector3(cB, scale_vector3(m->normal, rb));
    return true;
}

static bool CollideCapsuleBox(const Entity* cap, const Entity* box, Manifold* m) {
    Vector3 c = GetWorldCenter(cap);
    float r = cap->colliderSize.x * cap->scale.x;
    float h = cap->colliderSize.y * cap->scale.y * 0.5f - r;
    Vector3 bot = {c.x, c.y - h, c.z};
    Vector3 top = {c.x, c.y + h, c.z};

    // Treat capsule as two spheres + line segment; test each part
    Manifold tmp = {0};
    float bestPen = -1e30f;
    Vector3 bestNorm = {0,0,0};
    Vector3 bestCP  = {0,0,0};

    // 1) Sphere (bot) vs Box
    Entity fake = *cap;
    fake.collider = COLLIDER_TYPE_SPHERE;
    fake.colliderCenter = sub_vector3(bot, cap->position);
    if (CollideSphereBox(&fake, box, &tmp) && tmp.penetration > bestPen) {
        bestPen = tmp.penetration; bestNorm = tmp.normal; bestCP = tmp.contactPoint;
    }

    // 2) Sphere (top) vs Box
    fake.colliderCenter = sub_vector3(top, cap->position);
    if (CollideSphereBox(&fake, box, &tmp) && tmp.penetration > bestPen) {
        bestPen = tmp.penetration; bestNorm = tmp.normal; bestCP = tmp.contactPoint;
    }

    // 3) Line segment vs Box (SAT-style closest point)
    // Transform segment to box local space
    Quaternion qinv = conjugate_quaternion(box->rotation);
    Vector3 p0 = add_vector3(sub_vector3(bot, box->position), (Vector3){0,0,0});
    p0 = rotate_quaternion(qinv, p0);
    Vector3 p1 = add_vector3(sub_vector3(top, box->position), (Vector3){0,0,0});
    p1 = rotate_quaternion(qinv, p1);

    Vector3 half = scale_vector3(box->colliderSize, 0.5f);
    Vector3 localCenter = { box->colliderCenter.x, box->colliderCenter.y, box->colliderCenter.z };

    // Clamp segment endpoints to box
    Vector3 cp0 = {
        clampf(p0.x / box->scale.x - localCenter.x, -half.x, half.x),
        clampf(p0.y / box->scale.y - localCenter.y, -half.y, half.y),
        clampf(p0.z / box->scale.z - localCenter.z, -half.z, half.z)
    };

    // Closest point on segment to box interior
    Vector3 segDir = sub_vector3(p1, p0);
    float len2 = dot_vector3(segDir, segDir);
    float t = 0.0f;
    if (len2 > 1e-6f) {
        t = clampf(dot_vector3(sub_vector3(cp0, p0), segDir) / len2, 0.0f, 1.0f);
    }
    Vector3 closestOnSeg = add_vector3(p0, scale_vector3(segDir, t));
    Vector3 delta = sub_vector3(closestOnSeg, cp0);
    float d2 = dot_vector3(delta, delta);
    if (d2 < r*r + 1e-6f) {
        float d = vsqrtf(d2);
        Vector3 n = (d > 0) ? normalize_vector3(delta) : (Vector3){0,1,0};
        n = rotate_quaternion(box->rotation, n);
        float pen = r - d;
        if (pen > bestPen) {
            bestPen = pen;
            bestNorm = n;
            bestCP = add_vector3(closestOnSeg, scale_vector3(n, -r));
            bestCP = add_vector3(box->position, rotate_quaternion(box->rotation,
                     add_vector3(scale_vector3(bestCP, box->scale.x), 
                                 (Vector3){localCenter.x*box->scale.x,
                                           localCenter.y*box->scale.y,
                                           localCenter.z*box->scale.z})));
        }
    }

    if (bestPen <= 0.0f) return false;
    m->penetration = bestPen;
    m->normal = bestNorm;
    m->contactPoint = bestCP;
    return true;
}

// bool CollideConvexBox(const Entity* convex, const Entity* box, Manifold* m) {
//     uint16_t modelIdx = convex->colliderMeshIndex;
//     if (modelIdx >= loadedModels) return false;
// 
//     // Transform box into convex local space
//     Vector3 boxHalf = scale_vector3(box->colliderSize, 0.5f);
// 
//     // Get convex vertices
//     uint32_t vcount = modelVertexCounts[modelIdx];
//     float* verts = modelVertices[modelIdx];
// 
//     // We'll test: box faces + convex faces + edge cross products
//     Vector3 bestNormal = {0,0,0};
//     float bestPen = -FLT_MAX;
//     bool hit = false;
// 
//     // 1. Box face normals (6)
//     Vector3 boxFaces[6] = {
//         {1,0,0}, {-1,0,0},
//         {0,1,0}, {0,-1,0},
//         {0,0,1}, {0,0,-1}
//     };
//     for (int i = 0; i < 6; ++i) {
//         Vector3 n = rotate_quaternion(convex->rotation, boxFaces[i]);
//         float d_convex = -FLT_MAX;
//         
//         // Project the *box* onto the axis that lives in *convex local space*
//         // The axis is already rotated into convex space (n), so the box extent is just half-size.
//         float boxExtent = (i%2==0) ? boxHalf.x : (i>=2 && i<4) ? boxHalf.y : boxHalf.z;
//         float d_box = (i%2==1) ? -boxExtent : boxExtent;
// 
//         for (uint32_t v = 0; v < vcount; ++v) {
//             Vector3 p = {
//                 verts[v*VERTEX_ATTRIBUTES_COUNT + 0],
//                 verts[v*VERTEX_ATTRIBUTES_COUNT + 1],
//                 verts[v*VERTEX_ATTRIBUTES_COUNT + 2]
//             };
//             float d = dot_vector3(p, n);
//             d_convex = vmax(d_convex, d);
//         }
//         float pen = d_box - d_convex;
//         if (pen > 0 && pen > bestPen) {
//             bestPen = pen;
//             bestNormal = n;
//             hit = true;
//         }
//     }
// 
//     if (!hit) return false;
// 
//     m->penetration = bestPen;
//     m->normal = normalize_vector3(bestNormal);
//     m->contactPoint = add_vector3(convex->position, scale_vector3(m->normal, -bestPen * 0.5f));
//     return true;
// }

float Combine(const float a, const float b, PhysCombineType combine) {
    switch (combine) {
        case PHYS_COMBINE_AVG: return (a + b) * 0.5f;
        case PHYS_COMBINE_MIN: return vmin(a, b);
        case PHYS_COMBINE_MUL: return vsqrtf(a * b);
        case PHYS_COMBINE_MAX: return vmax(a, b);
        default: return a; // or average
    }
}

void ResolveCollision(Entity* a, Entity* b, Manifold* m) {
    if (a->mass <= 0.0f && b->mass <= 0.0f) return;
    // Relative velocity at contact
    Vector3 ra = sub_vector3(m->contactPoint, a->position);
    Vector3 va = add_vector3(a->velocity, cross_vector3(a->angularVelocity, ra));
    Vector3 rb = sub_vector3(m->contactPoint, b->position);
    Vector3 vb = add_vector3(b->velocity, cross_vector3(b->angularVelocity, rb));
    Vector3 rel_vel = sub_vector3(va, vb);
    float vel_n = dot_vector3(rel_vel, m->normal);
    if (vel_n > 0.0f) return; // Separating
    float inv_ma = (a->mass > 0.0f) ? 1.0f / a->mass : 0.0f;
    float inv_mb = (b->mass > 0.0f) ? 1.0f / b->mass : 0.0f;
    float inv_m_sum = inv_ma + inv_mb;
    if (inv_m_sum < 1e-6f) return;
    // Bounce
    float e = Combine(a->bounciness, b->bounciness, a->bounceCombine); // Assume same combine type
    float j = -(1.0f + e) * vel_n / inv_m_sum;
    Vector3 impulse = scale_vector3(m->normal, j);
    // Apply linear
    if (a->mass > 0.0f) a->velocity = sub_vector3(a->velocity, scale_vector3(impulse, inv_ma));
    if (b->mass > 0.0f) b->velocity = add_vector3(b->velocity, scale_vector3(impulse, inv_mb));
    // Friction (dynamic for sliding)
    Vector3 tangent_vel = sub_vector3(rel_vel, scale_vector3(m->normal, vel_n));
    float tangent_len = magnitude_vector3(tangent_vel);
    if (tangent_len > 1e-6f) {
        Vector3 tangent = normalize_vector3(tangent_vel);
        float mu_d = Combine(a->dynamicFriction, b->dynamicFriction, a->frictionCombine);
        float jt_max = vabs(j) * mu_d;
        float jt = -tangent_len / inv_m_sum;
        if (vabs(jt) > jt_max) jt = (jt > 0.0f ? jt_max : -jt_max);
        Vector3 friction_impulse = scale_vector3(tangent, jt);
        if (a->mass > 0.0f) a->velocity = sub_vector3(a->velocity, scale_vector3(friction_impulse, inv_ma));
        if (b->mass > 0.0f) b->velocity = add_vector3(b->velocity, scale_vector3(friction_impulse, inv_mb));
    }
    // Torque (only for non-NPC/player)
    if (!(ConstIndexIsNPC(a->index) || a == &instances[PLAYER1] || a == &instances[PLAYER2])) {
        if (a->inertia > 0.0f) {
            float inv_ia = 1.0f / a->inertia;
            Vector3 torque_a = cross_vector3(ra, impulse);
            a->angularVelocity = add_vector3(a->angularVelocity, scale_vector3(torque_a, inv_ia));
            // Friction torque similar, omitted for minimal.
        }
    }
    if (!(ConstIndexIsNPC(b->index) || b == &instances[PLAYER1] || b == &instances[PLAYER2])) {
        if (b->inertia > 0.0f) {
            float inv_ib = 1.0f / b->inertia;
            Vector3 torque_b = cross_vector3(rb, scale_vector3(impulse, -1.0f)); // Opposite
            b->angularVelocity = add_vector3(b->angularVelocity, scale_vector3(torque_b, inv_ib));
        }
    }
    // Positional correction
    float percent = 0.2f;
    float slop = 0.01f;
    float correction = vmax(m->penetration - slop, 0.0f) * percent / inv_m_sum;
    Vector3 correction_vec = scale_vector3(m->normal, correction);
    if (a->mass > 0.0f) a->position = sub_vector3(a->position, scale_vector3(correction_vec, inv_ma));
    if (b->mass > 0.0f) b->position = add_vector3(b->position, scale_vector3(correction_vec, inv_mb));
    // Dirty matrices
    dirtyInstances[a - instances] = true;
    dirtyInstances[b - instances] = true;
}

void ComputeInertiaFromCollider(Entity* e) {
    if (e->mass <= 0.0f || e->collider == COLLIDER_TYPE_NONE) {
        e->inertia = 0.0f;
        return;
    }
    
    Vector3 size = e->colliderSize;
    size.x *= e->scale.x; size.y *= e->scale.y; size.z *= e->scale.z;
    if (e->collider == COLLIDER_TYPE_SPHERE) {
        float r2 = size.x * size.x;
        e->inertia = 0.4f * e->mass * r2;
    } else if (e->collider == COLLIDER_TYPE_BOX) {
        float ly2 = size.y*size.y, lz2 = size.z*size.z;
        e->inertia = e->mass * (ly2 + lz2) / 12.0f; // scalar approx
    } else if (e->collider == COLLIDER_TYPE_CAPSULE) {
        // Approx as cylinder + spheres
        float r2 = size.x * size.x;
        float h = size.y;
        e->inertia = e->mass * (0.25f * r2 + h * h / 12.0f); // About perpendicular axis.
    }
    // For mesh, approx from bounds.
    // Clamp to avoid zero.
    if (e->inertia < 1e-6f) e->inertia = e->mass; // Fallback.
}

void IntegratePhysics(float dt) {
    for (uint16_t i = 0; i < INSTANCE_COUNT; ++i) {
        Entity* e = &instances[i];
        if (!(e->entflags & ENTFLAG_ACTIVE)) continue;
        if ((e->entflags & ENTFLAG_KINEMATIC)) continue;
        if (e->mass <= 0.0f) continue;  // Safety; set mass >0 for dynamics.

        // Accumulate gravity as force.
        if ((e->entflags & ENTFLAG_USEGRAVITY)) e->accumulatedForce = add_vector3(e->accumulatedForce, scale_vector3((Vector3){0.0f,-9.81f,0.0f}, e->mass));

        Vector3 accel = scale_vector3(e->accumulatedForce, 1.0f / e->mass); // Linear integration (semi-implicit Euler).
        e->velocity = add_vector3(e->velocity, scale_vector3(accel, dt));
        e->velocity = scale_vector3(e->velocity, (1.0f - e->linearDrag * dt)); // Apply linear drag (simple exponential approx).
        e->position = add_vector3(e->position, scale_vector3(e->velocity, dt)); // Integrate position.

        // Angular integration (only for non-player/NPC; skip torque/rot for them).
        if (ConstIndexIsNPC(e->index) || i == PLAYER1 || i == PLAYER2) {
            e->angularVelocity = (Vector3){0.0f,0.0f,0.0f};
            e->accumulatedTorque = (Vector3){0.0f,0.0f,0.0f};
        } else if (e->inertia > 0.0f) {
            // Angular accel (scalar I approx; for full tensor, extend later if needed).
            float invI = 1.0f / e->inertia;
            Vector3 angAccel = scale_vector3(e->accumulatedTorque, invI);
            e->angularVelocity = add_vector3(e->angularVelocity, scale_vector3(angAccel, dt));
            // Apply angular drag.
            e->angularVelocity = scale_vector3(e->angularVelocity, (1.0f - e->angularDrag * dt));
            // Integrate rotation (world-space axis-angle approx; good for small dt).
            float angMag = magnitude_vector3(e->angularVelocity);
            if (angMag > 1e-6f) {
                float angle = angMag * dt;
                Vector3 axis = normalize_vector3(e->angularVelocity);
                Quaternion deltaQ = axis_angle_quaternion(axis, angle);
                e->rotation = mul_quaternion(deltaQ, e->rotation);
                normalize_quaternion(&e->rotation);
            }
        }

        // Clear accumulators.
        e->accumulatedForce = (Vector3){0.0f,0.0f,0.0f};
        e->accumulatedTorque = (Vector3){0.0f,0.0f,0.0f};
    }
}

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

// AddRelativeForce (local -> world transform via quat).
void AddRelativeForce(uint16_t idx, Vector3 localForce, bool isImpulse) {
    Entity* e = &instances[idx];
    if ((e->entflags & ENTFLAG_KINEMATIC) || e->mass <= 0.0f) return;
    Vector3 worldForce = rotate_quaternion(e->rotation, localForce);
    AddForce(idx, worldForce, isImpulse);
}

// AddForceAtPosition (applies force + torque; mimics Unity, assumes CoM at position).
void AddForceAtPosition(uint16_t idx, Vector3 force, Vector3 position, bool isImpulse) {
    Entity* e = &instances[idx];
    if ((e->entflags & ENTFLAG_KINEMATIC) || e->mass <= 0.0f) return;
    if (isImpulse) {
        e->velocity = add_vector3(e->velocity, scale_vector3(force, 1.0f / e->mass));
    } else {
        e->accumulatedForce = add_vector3(e->accumulatedForce, force);
    }
    // Torque (world space).
    if (! (ConstIndexIsNPC(e->index) || idx == PLAYER1 || idx == PLAYER2) && e->inertia > 0.0f) {
        Vector3 relPos = sub_vector3(position, e->position);
        Vector3 torque = cross_vector3(relPos, force);
        if (isImpulse) {
            // For impulse torque, integrate angVel directly (approx).
            float invI = 1.0f / e->inertia;
            e->angularVelocity = add_vector3(e->angularVelocity, scale_vector3(torque, invI));
        } else {
            e->accumulatedTorque = add_vector3(e->accumulatedTorque, torque);
        }
    }
}

// AddExplosionForce (mimics Unity; linear falloff, upwardsModifier on Y; assume up={0,1,0}).
void AddExplosionForce(uint16_t idx, float power, Vector3 explosionPos, float radius, float upwardsModifier, bool isImpulse) {
    Entity* e = &instances[idx];
    if ((e->entflags & ENTFLAG_KINEMATIC) || e->mass <= 0.0f) return;
    Vector3 dir = sub_vector3(e->position, explosionPos);  // From explosion to entity CoM.
    float dist = magnitude_vector3(dir);
    if (dist > radius || dist < 1e-6f) return;
    dir = normalize_vector3(dir);
    float falloff = 1.0f - (dist / radius);
    if (falloff < 0.0f) falloff = 0.0f;
    Vector3 explosionForce = scale_vector3(dir, power * falloff);
    // Upwards component (Unity adds to Y).
    explosionForce.y += upwardsModifier * power * falloff;
    if (isImpulse) {
        e->velocity = add_vector3(e->velocity, scale_vector3(explosionForce, 1.0f / e->mass));
    } else {
        e->accumulatedForce = add_vector3(e->accumulatedForce, explosionForce);
    }
}

// ================================= Physics ==================================
void PlayerPhysics(void) {
    // Player Movement from Input
    if (window_has_focus) { // Move the player based on input first, then bound it below...
        UpdatePlayerFacingAngles();
        ProcessInput();
    }

    // Player Physics: Capsule-triangle naive collision
    if (noclip) {
        instances[PLAYER1].collider = COLLIDER_TYPE_NONE;
        instances[PLAYER1].entflags = (instances[PLAYER1].entflags & ~ENTFLAG_USEGRAVITY) | (-false & ENTFLAG_USEGRAVITY);
        instances[PLAYER1].velocity = (Vector3){0.0f,0.0f,0.0f};
    } else {
        instances[PLAYER1].collider = COLLIDER_TYPE_CAPSULE;
        instances[PLAYER1].entflags = (instances[PLAYER1].entflags & ~ENTFLAG_USEGRAVITY) | (-true & ENTFLAG_USEGRAVITY);
    }
}

static bool ShouldTestPair(uint16_t i, uint16_t j) {
    if (!is_instance_in_neighbor_cells(cellIndexForInstance[i], cellIndexForInstance[j]))
        return false;
    return true;
}

int32_t Physics(void) {
    if (gamePaused || menuActive) return 0;

    PlayerPhysics();
    IntegratePhysics(timeSinceLastPhysicsTick);
    for (int32_t p = PLAYER1; p < PLAYER1 + 1; ++p) {
        Entity* ea = &instances[p];
        Vector3 mina, maxa;
        if (!GetAABB(ea, &mina, &maxa)) continue;

        for (int32_t q = 1; q < loadedInstances; ++q) {
            Entity* eb = &instances[q];

            if (!ShouldTestPair(p, q)) continue;
            DualLog("  [1] cell test passed\n");

            Vector3 minb, maxb;
            if (!GetAABB(eb, &minb, &maxb)) continue;
            DualLog("  [2] AABB computed\n");
            
            DualLog("AABB A: (%.2f,%.2f,%.2f) -> (%.2f,%.2f,%.2f)\n", mina.x, mina.y, mina.z, maxa.x, maxa.y, maxa.z);
            DualLog("AABB B: (%.2f,%.2f,%.2f) -> (%.2f,%.2f,%.2f)\n", minb.x, minb.y, minb.z, maxb.x, maxb.y, maxb.z);

            if (!CollideAABB(mina, maxa, minb, maxb)) continue;
            DualLog("  [3] AABB overlap – entering narrow phase\n");

            Manifold m = {0};
            bool hit = false;

            /* ---- narrow-phase dispatcher (unchanged) ---- */
            if (ea->collider == COLLIDER_TYPE_SPHERE && eb->collider == COLLIDER_TYPE_SPHERE) {
                hit = CollideSphereSphere(ea, eb, &m);
            } else if (ea->collider == COLLIDER_TYPE_SPHERE && eb->collider == COLLIDER_TYPE_BOX) {
                hit = CollideSphereBox(ea, eb, &m);
            } else if (ea->collider == COLLIDER_TYPE_BOX && eb->collider == COLLIDER_TYPE_SPHERE) {
                hit = CollideSphereBox(eb, ea, &m); m.normal = scale_vector3(m.normal, -1.0f);
            } else if (ea->collider == COLLIDER_TYPE_CAPSULE && eb->collider == COLLIDER_TYPE_CAPSULE) {
                hit = CollideCapsuleCapsule(ea, eb, &m);
            } else if (ea->collider == COLLIDER_TYPE_CAPSULE && eb->collider == COLLIDER_TYPE_BOX) {
                hit = CollideCapsuleBox(ea, eb, &m);
            } else if (ea->collider == COLLIDER_TYPE_BOX && eb->collider == COLLIDER_TYPE_CAPSULE) {
                hit = CollideCapsuleBox(eb, ea, &m); m.normal = scale_vector3(m.normal, -1.0f);
            }/* else if (ea->collider == COLLIDER_TYPE_CONVEXMESH && eb->collider == COLLIDER_TYPE_BOX) {
                hit = CollideConvexBox(ea, eb, &m);
            } else if (ea->collider == COLLIDER_TYPE_BOX && eb->collider == COLLIDER_TYPE_CONVEXMESH) {
                hit = CollideConvexBox(eb, ea, &m); m.normal = scale_vector3(m.normal, -1.0f);
            }*/

            if (hit) {
                DualLog("  HIT! pen=%.3f  normal=(%.2f,%.2f,%.2f)\n", m.penetration, m.normal.x, m.normal.y, m.normal.z);
                ResolveCollision(ea, eb, &m);
            }
        }
    }

    /* -------------------------------------------------------------
       4. Update matrices
       ------------------------------------------------------------- */
    for (uint16_t i = 0; i < INSTANCE_COUNT; ++i)
        if (dirtyInstances[i]) { UpdateInstanceMatrix(i); dirtyInstances[i] = false; }

    return 0;
}
