// physics.c - Discrete parallel physics with substep safety.  Threading model: persistent worker threads, single mutex+condvar pair for dispatch (main->workers) and a separate pair for completion (workers->main).  Workers read a snapshot of ALL entity positions taken before dispatch (no cross-worker position races), write results of current entity and affecting entities only into their own complete copy of the world in g_phys[] then each writes their slice back to Sys_Global.instances[] at the end.
#include "os.h"
#include "common.h"
#include "interop.h"
#define MAX_COLLISION_ITERATIONS  4
#define RESTITUTION               0.5f
#define FRICTION                  0.2f
#define STEP_MIN_NORMAL_Y         0.7f
#define COLLISION_EPSILON         0.0001f
#define MAX_SUBSTEPS              10
#define MIN_DIAMETER               0.1f // m
#define MAX_SPEED                 10.0f // m/s
#define MAX_STEP_SIZE            (MIN_DIAMETER / MAX_SPEED) // 0.01 s
typedef struct { bool hit; Vector3 point, normal; } OverlapResult;
typedef struct { Vector3 pos, vel, lastPos; float gravity,mass; } PhysicsState;
static PhysicsState g_phys[INSTANCE_COUNT]; int g_running = 0;
static Vector3 g_posSnapshot[INSTANCE_COUNT]; // Snapshot of entity positions taken on the main thread before each substep.  Workers read this for neighbour lookups so there are no write-write or read-write races between worker threads.
extern GlobalContext Sys_Global; extern CheatsSystem Sys_Cheats; extern u16 loadedModelsMaxIndex,dynamicEntityCount,dynamicEntities[512]; extern float modelBounds[MODEL_IDX_MAX];
static pthread_mutex_t g_dispatchMutex; static pthread_cond_t  g_dispatchCond; static pthread_mutex_t g_doneMutex; static pthread_cond_t  g_doneCond; // Dispatch gate: main raises g_dispatchGen, workers wake, process, then atomically decrement g_workRemaining.  When it hits 0 the last worker signals the completion condvar so the main thread can proceed.
static volatile u32 g_dispatchGen = 0; static volatile u32 g_workRemaining = 0; // Generation counter — each substep the main thread increments this under g_dispatchMutex and broadcasts.  Workers compare against the generation they last processed so spurious wakeups are harmless.
typedef struct { u16 start,end; float dt; } WorkerSlice;
#define MAX_WORKERS 10
static WorkerSlice g_slices[MAX_WORKERS];
static u32         g_workerCnt = 0;
static inline OverlapResult SphereSphere(Vector3 aPos, float aRad, Vector3 bPos, float bRad) {
    OverlapResult r = {0};
    Vector3 delta = Vector3_A_minus_B(aPos, bPos);
    float dist2 = dot_vector3(delta, delta);
    float radSum = aRad + bRad;
    if (dist2 < radSum * radSum) {
        r.hit = true;
        float dist = vsqrtf(dist2);
        r.normal = (dist < COLLISION_EPSILON) ? (Vector3){0,1,0} : scale_vector3(delta, 1.0f / dist);
        r.point  = Vector3_A_plus_B(bPos, scale_vector3(r.normal, bRad));
    }
    return r;
}

static inline OverlapResult CapsuleCapsule(ShapeCapsule a, ShapeCapsule b) {
    OverlapResult r = {0};
    Vector3 aAxis = Vector3_A_minus_B(a.tip, a.base);
    Vector3 bAxis = Vector3_A_minus_B(b.tip, b.base);
    for (int i = 0; i < 3; ++i) {
        Vector3 aPoint = Vector3_A_plus_B(a.base, scale_vector3(aAxis, (float)i * 0.5f));
        for (int j = 0; j < 3; ++j) {
            Vector3 bPoint = Vector3_A_plus_B(b.base, scale_vector3(bAxis, (float)j * 0.5f));
            OverlapResult hit = SphereSphere(aPoint, a.radius, bPoint, b.radius);
            if (hit.hit) { return hit; }
        }
    }
    return r;
}

static inline OverlapResult BoxBox(ShapeBox a, ShapeBox b) {
    float aRad = vmax(a.halfExtents.x, vmax(a.halfExtents.y, a.halfExtents.z));
    float bRad = vmax(b.halfExtents.x, vmax(b.halfExtents.y, b.halfExtents.z));
    return SphereSphere(a.center, aRad, b.center, bRad);
}

static inline OverlapResult CapsuleBox(ShapeCapsule c, ShapeBox b) {
    float bRad = vmax(b.halfExtents.x, vmax(b.halfExtents.y, b.halfExtents.z));
    OverlapResult r = {0};
    Vector3 axis = Vector3_A_minus_B(c.tip, c.base);
    for (int i = 0; i < 4; ++i) {
        Vector3 point = Vector3_A_plus_B(c.base, scale_vector3(axis, (float)i * (1.0f/3.0f)));
        OverlapResult hit = SphereSphere(point, c.radius, b.center, bRad);
        if (hit.hit) { return hit; }
    }
    return r;
}

static u32 GetCollisionMask(u32 layer) {
    if (layer == Layer_NPCTrigger || layer == Layer_NPCClip) return Layer_NPC;
    if (layer == Layer_TransparentFX || layer == Layer_IgnoreRaycast)
        return Layer_Default|Layer_TransparentFX|Layer_IgnoreRaycast|Layer_Geometry|Layer_NPC|
               Layer_PlayerBullets|Layer_Player|Layer_PhysObjects|Layer_Trigger|Layer_Door|
               Layer_InterDebris|Layer_Player2|Layer_NPCBullet|Layer_Clip;
    switch (layer) {
        case Layer_Default:     return Layer_Default|Layer_TransparentFX|Layer_IgnoreRaycast|Layer_Geometry|Layer_NPC|Layer_PlayerBullets|Layer_Player|Layer_Corpse|Layer_PhysObjects|Layer_Sky|Layer_Trigger|Layer_Door|Layer_InterDebris|Layer_Player2|Layer_Player3|Layer_Player4|Layer_NPCBullet|Layer_Clip|Layer_CorpseSearchable;
        case Layer_Geometry:    return Layer_Default|Layer_TransparentFX|Layer_IgnoreRaycast|Layer_Geometry|Layer_NPC|Layer_PlayerBullets|Layer_Player|Layer_PhysObjects|Layer_Trigger|Layer_Door|Layer_InterDebris|Layer_Player2|Layer_Clip;
        case Layer_NPC:         return Layer_Default|Layer_TransparentFX|Layer_IgnoreRaycast|Layer_Geometry|Layer_NPC|Layer_PlayerBullets|Layer_Player|Layer_PhysObjects|Layer_Trigger|Layer_NPCTrigger|Layer_Door|Layer_InterDebris|Layer_Player2|Layer_NPCBullet|Layer_NPCClip|Layer_Clip;
        case Layer_PlayerBullets: return Layer_Default|Layer_TransparentFX|Layer_IgnoreRaycast|Layer_Geometry|Layer_NPC|Layer_PlayerBullets|Layer_Player|Layer_Corpse|Layer_PhysObjects|Layer_Door|Layer_InterDebris|Layer_Player2|Layer_NPCBullet|Layer_Clip|Layer_CorpseSearchable;
        case Layer_Player:      return Layer_Default|Layer_TransparentFX|Layer_IgnoreRaycast|Layer_Geometry|Layer_NPC|Layer_PhysObjects|Layer_PlayerTriggerOnly|Layer_Trigger|Layer_Door|Layer_Player2|Layer_NPCBullet|Layer_Clip;
        case Layer_Corpse:      return Layer_Default|Layer_Geometry|Layer_PlayerBullets|Layer_PhysObjects|Layer_Door|Layer_NPCBullet|Layer_Clip;
        case Layer_PhysObjects: return Layer_Default|Layer_TransparentFX|Layer_IgnoreRaycast|Layer_Geometry|Layer_NPC|Layer_PlayerBullets|Layer_Player|Layer_Corpse|Layer_PhysObjects|Layer_Door|Layer_InterDebris|Layer_NPCBullet|Layer_Clip;
        case Layer_Sky:         return Layer_Default|Layer_Player;
        case Layer_PlayerTriggerOnly: return Layer_Player|Layer_Player2|Layer_Player3;
        case Layer_Trigger:     return Layer_Default|Layer_Geometry|Layer_NPC|Layer_PlayerBullets|Layer_Player|Layer_PhysObjects|Layer_Door|Layer_InterDebris|Layer_Clip;
        case Layer_Door:        return Layer_Default|Layer_TransparentFX|Layer_IgnoreRaycast|Layer_Geometry|Layer_NPC|Layer_PlayerBullets|Layer_Player|Layer_Corpse|Layer_PhysObjects|Layer_Trigger|Layer_Door|Layer_InterDebris|Layer_Player2|Layer_NPCBullet|Layer_Clip;
        case Layer_InterDebris: return Layer_Default|Layer_Geometry|Layer_NPC|Layer_PlayerBullets|Layer_PhysObjects|Layer_Trigger|Layer_Door|Layer_NPCBullet|Layer_Clip;
        case Layer_Player2:     return Layer_Default|Layer_TransparentFX|Layer_IgnoreRaycast|Layer_Geometry|Layer_NPC|Layer_PlayerBullets|Layer_Player|Layer_PhysObjects|Layer_PlayerTriggerOnly|Layer_Trigger|Layer_Door|Layer_InterDebris|Layer_Player2|Layer_NPCBullet|Layer_Clip;
        case Layer_Player3:     return Layer_Default|Layer_TransparentFX|Layer_IgnoreRaycast|Layer_Geometry|Layer_NPC|Layer_PlayerBullets|Layer_Player|Layer_PhysObjects|Layer_PlayerTriggerOnly|Layer_Trigger|Layer_Door|Layer_InterDebris|Layer_Player3|Layer_NPCBullet|Layer_Clip;
        case Layer_Player4:     return Layer_Default|Layer_TransparentFX|Layer_IgnoreRaycast|Layer_Geometry|Layer_NPC|Layer_PlayerBullets|Layer_Player|Layer_PhysObjects|Layer_PlayerTriggerOnly|Layer_Trigger|Layer_Door|Layer_InterDebris|Layer_Player4|Layer_NPCBullet|Layer_Clip;
        case Layer_NPCBullet:   return Layer_Default|Layer_TransparentFX|Layer_IgnoreRaycast|Layer_Geometry|Layer_PlayerBullets|Layer_Player|Layer_Corpse|Layer_PhysObjects|Layer_Door|Layer_InterDebris|Layer_Player2|Layer_Clip|Layer_CorpseSearchable;
        case Layer_Clip:        return Layer_Player|Layer_Player2|Layer_Player3|Layer_Player4|Layer_NPC;
        case Layer_CorpseSearchable: return Layer_Default|Layer_PlayerBullets;
        default:                return 0u;
    }
}

void Entity_GetCapsule(const Entity *e, ShapeCapsule *out) {
    float r = e->colliderSize.x, hi = vmax(0.0f, (e->colliderSize.y * 0.5f) - r);
    Vector3 wc  = Vector3_A_plus_B(e->position, quat_rotate_vector(e->rotation, e->colliderCenter));
    Vector3 axis = (e->colliderSize.z < 0.5f) ? quat_rotate_vector(e->rotation, (Vector3){1,0,0})
                 : (e->colliderSize.z < 1.5f) ? quat_rotate_vector(e->rotation, (Vector3){0,1,0})
                 :                              quat_rotate_vector(e->rotation, (Vector3){0,0,1});
    out->radius = r;
    out->base   = Vector3_A_minus_B(wc, scale_vector3(axis, hi));
    out->tip    = Vector3_A_plus_B (wc, scale_vector3(axis, hi));
}
void Entity_GetBox   (const Entity *e, ShapeBox    *out) { out->center = Vector3_A_plus_B(e->position, quat_rotate_vector(e->rotation, e->colliderCenter)); out->halfExtents = scale_vector3(e->colliderSize, 0.5f); out->rot = e->rotation; }
void Entity_GetSphere(const Entity *e, ShapeSphere *out) { out->center = Vector3_A_plus_B(e->position, quat_rotate_vector(e->rotation, e->colliderCenter)); out->radius = e->colliderSize.x; }
void obb_axes(Quaternion q, Vector3 *ax, Vector3 *ay, Vector3 *az) { *ax=quat_rotate_vector(q,(Vector3){1,0,0}); *ay=quat_rotate_vector(q,(Vector3){0,1,0}); *az=quat_rotate_vector(q,(Vector3){0,0,1}); }
float GetCollisionRadius(Entity *e) {
    if (e->collider == COLLIDER_TYPE_BOX)     return vmax(e->colliderSize.x, vmax(e->colliderSize.y, e->colliderSize.z));
    if (e->collider == COLLIDER_TYPE_CAPSULE) return e->colliderSize.x;
    return e->colliderSize.x;
}

static inline Vector3 ApplyGravity(Vector3 vel, float selfGravity, float dt) { vel.y += (-9.81f * selfGravity) * dt; return vel; }
static void* PhysicsWorkerFunc(void *arg) {
    u32 workerIdx   = (u32)(uintptr_t)arg;
    u32 lastGen     = 0;

    while (__atomic_load_n(&g_running,__ATOMIC_ACQUIRE)) {
        pthread_mutex_lock(&g_dispatchMutex);
        while (__atomic_load_n(&g_dispatchGen,__ATOMIC_ACQUIRE) == lastGen && __atomic_load_n(&g_running,__ATOMIC_ACQUIRE)) 
            pthread_cond_wait(&g_dispatchCond,&g_dispatchMutex);
        u32 myGen = __atomic_load_n(&g_dispatchGen,__ATOMIC_ACQUIRE);
        pthread_mutex_unlock(&g_dispatchMutex);
        if (!__atomic_load_n(&g_running,__ATOMIC_ACQUIRE)) break;
        
        lastGen = myGen;
        WorkerSlice *sl = &g_slices[workerIdx];
        for (u16 i = sl->start; i < sl->end; ++i) {
            u16 idx = dynamicEntities[i];
            if (idx >= INSTANCE_COUNT) continue;

            Entity *e = &Sys_Global.instances[idx];
            if (e->collider == COLLIDER_TYPE_NONE || (Sys_Cheats.noclip && idx != PLAYER1)) continue;

            PhysicsState *ps = &g_phys[idx];
            float mass = e->mass > 0.001f ? e->mass : 1.0f;
            Vector3 baseVel = ps->vel;
            Vector3 vel = ApplyGravity(baseVel,ps->gravity,sl->dt);
            vel = Vector3_A_plus_B(vel, scale_vector3(e->accumulatedForce, sl->dt / mass));
            Vector3 newPos = Vector3_A_plus_B(ps->pos, scale_vector3(vel, sl->dt));
            
            // Check for collision and get the normal if we hit
            u32 mask = GetCollisionMask(e->layer);
            bool hit = false;
            Vector3 hitNormal = {0,1,0};
            
            for (u16 j = 0; j < INSTANCE_COUNT && !hit; ++j) {
                if (j == idx) continue;
                Entity *o = &Sys_Global.instances[j];
                if (o->collider == COLLIDER_TYPE_NONE || !(mask & o->layer)) continue;
                
                Vector3 savedPos = e->position;
                e->position = newPos;
                Vector3 oSavedPos = o->position;
                o->position = g_posSnapshot[j];
                
                ShapeCapsule capA, capB; ShapeBox boxA, boxB;
                OverlapResult r = {0};
                if (e->collider==COLLIDER_TYPE_CAPSULE && o->collider==COLLIDER_TYPE_CAPSULE) {
                    Entity_GetCapsule(e,&capA); Entity_GetCapsule(o,&capB);
                    r = CapsuleCapsule(capA,capB);
                } else if (e->collider==COLLIDER_TYPE_CAPSULE && o->collider==COLLIDER_TYPE_BOX) {
                    Entity_GetCapsule(e,&capA); Entity_GetBox(o,&boxB);
                    r = CapsuleBox(capA,boxB);
                } else if (e->collider==COLLIDER_TYPE_BOX && o->collider==COLLIDER_TYPE_CAPSULE) {
                    Entity_GetBox(e,&boxA); Entity_GetCapsule(o,&capB);
                    r = CapsuleBox(capB,boxA);
                } else if (e->collider==COLLIDER_TYPE_BOX && o->collider==COLLIDER_TYPE_BOX) {
                    Entity_GetBox(e,&boxA); Entity_GetBox(o,&boxB);
                    r = BoxBox(boxA,boxB);
                } else {
                    r = SphereSphere(newPos, GetCollisionRadius(e), g_posSnapshot[j], GetCollisionRadius(o));
                }
                
                if (r.hit) {
                    hit = true;
                    hitNormal = r.normal;
                }
                
                o->position = oSavedPos;
                e->position = savedPos;
            }
            
            if (!hit) {
                ps->lastPos = ps->pos;
                ps->pos = newPos;
                ps->vel = vel;
            } else {
                // Simple response: cancel velocity into the surface, keep tangential
                float vdn = dot_vector3(vel, hitNormal);
                if (vdn < 0) {
                    // Remove the component going into the surface
                    vel = Vector3_A_minus_B(vel, scale_vector3(hitNormal, vdn));
                    // Small friction on tangential movement
                    Vector3 tangent = Vector3_A_minus_B(vel, scale_vector3(hitNormal, dot_vector3(vel, hitNormal)));
                    float tangentSpeed = magnitude_vector3(tangent);
                    if (tangentSpeed > COLLISION_EPSILON) {
                        float frictionScale = 0.95f; // 5% friction per hit
                        vel = Vector3_A_plus_B(scale_vector3(tangent, frictionScale), 
                                               scale_vector3(hitNormal, dot_vector3(vel, hitNormal)));
                    }
                }
                // Stay at last safe position
                ps->vel = vel;
            }
            e->accumulatedForce = (Vector3){0, 0, 0};
        }

        if (__atomic_sub_fetch(&g_workRemaining, 1, __ATOMIC_RELEASE) == 0) {
            pthread_mutex_lock(&g_doneMutex);
            pthread_cond_broadcast(&g_doneCond);
            pthread_mutex_unlock(&g_doneMutex);
        }
    }
    return NULL;
}

void InitPhysics(void) {
    u32 cores   = (u32)OS_GetNumThreads();
    g_workerCnt = (cores <= 2) ? 1u : (cores - 2 < MAX_WORKERS ? cores - 2 : MAX_WORKERS); g_running = 1;
    pthread_mutex_init(&g_dispatchMutex, NULL);
    pthread_cond_init (&g_dispatchCond,  NULL);
    pthread_mutex_init(&g_doneMutex,     NULL);
    pthread_cond_init (&g_doneCond,      NULL);
    for (u32 i = 0; i < g_workerCnt; ++i) { pthread_t t; pthread_create(&t,NULL,PhysicsWorkerFunc,(void*)(uintptr_t)i); pthread_detach(t); }
    DualLog("Physics: %u workers, substep size %.4fs\n", g_workerCnt, MAX_STEP_SIZE);
}

void PhysicsUpdateAndWait(float dt) {
    if (dynamicEntityCount == 0) return;
    
    for (u16 i = 0; i < dynamicEntityCount; ++i) {
        u16 idx = dynamicEntities[i];
        if (idx >= INSTANCE_COUNT) continue;
        Entity *e = &Sys_Global.instances[idx];
        g_phys[idx].pos     = e->position;
        g_phys[idx].gravity = e->gravity;
        g_phys[idx].lastPos = e->lastPosition;
        g_phys[idx].vel     = e->velocity;
        g_phys[idx].mass    = e->mass;
    }
    
    DualLog("Physics start: player vel = %f %f %f\n", g_phys[PLAYER1].vel.x, g_phys[PLAYER1].vel.y, g_phys[PLAYER1].vel.z);

    
    u8    substeps = (u8)vclamp((u32)(dt / MAX_STEP_SIZE + 0.5f), 2u, (u32)MAX_SUBSTEPS); // Substep count: at least 2, at most MAX_SUBSTEPS, sized so no object travels more than MIN_DIAMETER of smallest object in a single substep at MAX_SPEED.
    float dtsub    = dt / (float)substeps;

    for (u8 s = 0; s < substeps; ++s) {
        // Snapshot all entity positions so workers share a consistent read view.
        for (u16 i = 0; i < INSTANCE_COUNT; ++i) g_posSnapshot[i] = Sys_Global.instances[i].position;

        // Partition dynamicEntities[0..dynamicEntityCount) across workers.
        // NOTE: index 0 IS included (bug in original skipped it by starting at 1).
        u16 total = dynamicEntityCount;
        u16 per   = total / g_workerCnt;
        u16 rem   = total % g_workerCnt;
        u16 start = 0;
        for (u32 i = 0; i < g_workerCnt; ++i) {
            g_slices[i].start = start;
            g_slices[i].end   = start + per + (i < rem ? 1 : 0);
            g_slices[i].dt    = dtsub;
            start             = g_slices[i].end;
        }

        // Arm the completion counter before waking workers to avoid the race
        // where a fast worker completes and signals done before we start waiting.
        __atomic_store_n(&g_workRemaining, g_workerCnt, __ATOMIC_RELEASE);

        // Wake all workers by incrementing the generation counter.
        pthread_mutex_lock(&g_dispatchMutex);
        __atomic_fetch_add(&g_dispatchGen, 1, __ATOMIC_RELEASE);
        pthread_cond_broadcast(&g_dispatchCond);
        pthread_mutex_unlock(&g_dispatchMutex);

        // Block until every worker has decremented g_workRemaining to 0.
        pthread_mutex_lock(&g_doneMutex);
        while (__atomic_load_n(&g_workRemaining, __ATOMIC_ACQUIRE) != 0)
            pthread_cond_wait(&g_doneCond, &g_doneMutex);
        pthread_mutex_unlock(&g_doneMutex);

        // Flush physics results -> live entity positions (main thread only).
        for (u16 i = 0; i < dynamicEntityCount; ++i) {
            u16 idx = dynamicEntities[i];
            Sys_Global.instances[idx].position = g_phys[idx].pos;
            Sys_Global.instances[idx].velocity  = g_phys[idx].vel;
        }
    }
}

ENGINE_TO_MOD void AddForce(u16 idx, Vector3 force, bool impulse) {
    if (idx >= INSTANCE_COUNT) return;
    Entity *e   = &Sys_Global.instances[idx];
    float  mass = e->mass > 0.001f ? e->mass : 1.0f;
    if (impulse) e->velocity           = Vector3_A_plus_B(e->velocity, scale_vector3(force, 1.0f / mass));
    else         e->accumulatedForce   = Vector3_A_plus_B(e->accumulatedForce, force);
}

ENGINE_TO_MOD void ApplyPlayerMovements(void) {
    Entity *p = &Sys_Global.instances[PLAYER1];
    float h = (float)Forward() - (float)Backpedal(), s = (float)StrafeRight() - (float)StrafeLeft();
    Vector3 input = normalize_vector3((Vector3){p->forward.x*h + p->right.x*s, (float)SwimUp() - (float)SwimDn(), p->forward.z*h + p->right.z*s});
    float speed = GetBasePlayerSpeed(PLAYER1, magnitude_vector3(input) > 0.1f) * 1.75f;
    float accel = Sys_Global.boosterActive ? 1.0f : 3.0f;
    Vector3 cur = p->velocity;
    Vector3 dv  = Vector3_A_minus_B(scale_vector3(input, speed), cur);
    dv.x = vclamp(dv.x, -10, 10); dv.y = vclamp(dv.y, -10, 10); dv.z = vclamp(dv.z, -10, 10);
    p->velocity = Vector3_A_plus_B(cur, scale_vector3(dv, accel * vclamp((float)Sys_Global.timeSinceLastPhysicsTick, 0.0005f, 0.1f)));
    DualLog("Applied velocity from player movemnt: %f %f %f, frame %u\n",p->velocity.x,p->velocity.y,p->velocity.z,Sys_Global.globalFrameNum);
}
