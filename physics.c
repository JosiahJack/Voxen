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
typedef u16 half;
void* MemSetToVForNBytes(void *dst, int c, size_t n); void* CopyMemoryFromBtoAForNBytes(void *dst, const void *src, size_t n); float half_to_float(half h);
typedef struct { bool hit; Vector3 point,normal; } OverlapResult; typedef struct { Vector3 pos,vel,avel,lastPos; Quaternion rot; float gravity,mass,inertia,adrag,dfric,sfric,bouncy; PhysCombineType fComb,bComb; } PhysicsState;
extern u8** modelVertices;
extern u16** modelTriangles;
extern u32 modelVertexCounts[MODEL_IDX_MAX];
extern u16 modelTriangleCounts[MODEL_IDX_MAX];
extern float modelMatrices[INSTANCE_COUNT*16];
extern GlobalContext Sys_Global;
static inline __attribute__((always_inline)) i32 PosGetCellCoordX(float pos_x) { return (u16)clamp((i32)vfloor((pos_x - Sys_Global.worldMin_x + CELLXHALF) / CELL_SIZE), 0, WORLDX_0BASED); }
static inline __attribute__((always_inline)) i32 PosGetCellCoordZ(float pos_z) { return (u16)clamp((i32)vfloor((pos_z - Sys_Global.worldMin_z + CELLXHALF) / CELL_SIZE), 0, WORLDX_0BASED); }
typedef struct { u16 start,end; float dt; } WorkerSlice;
#define MAX_WORKERS 62 // Threadripper 64 minus main minus audio = 62, small bit of unnecessary RAM to give wide compatibility, which compatibility trumps all else of course.
static WorkerSlice g_slices[MAX_WORKERS];
static PhysicsState g_phys[INSTANCE_COUNT]; int g_running = 0;
static Vector3 g_posSnapshot[INSTANCE_COUNT]; // Snapshot of entity positions taken on the main thread before each substep.  Workers read this for neighbour lookups so there are no write-write or read-write races between worker threads.
u16 dynamicEntities[512]; u16 dynamicEntityCount;
extern GlobalContext Sys_Global;
extern CheatsSystem Sys_Cheats; // For seeing if noclip is on and letting player go through walls (also currently used to enable debug rendering of wireframe collision mesh/primitives
extern u16 loadedModelsMaxIndex;
extern float modelBounds[MODEL_IDX_MAX];
extern Color textColors[];
static pthread_mutex_t g_dispatchMutex; static pthread_cond_t  g_dispatchCond; static pthread_mutex_t g_doneMutex; static pthread_cond_t  g_doneCond; // Dispatch gate: main raises g_dispatchGen, workers wake, process, then atomically decrement g_workRemaining.  When it hits 0 the last worker signals the completion condvar so the main thread can proceed.
static volatile u32 g_dispatchGen = 0;
static volatile u32 g_workRemaining = 0; // Generation counter — each substep the main thread increments this under g_dispatchMutex and broadcasts.  Workers compare against the generation they last processed so spurious wakeups are harmless.

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
    if (layer == Layer_TransparentFX || layer == Layer_IgnoreRaycast) return Layer_Default|Layer_TransparentFX|Layer_IgnoreRaycast|Layer_Geometry|Layer_NPC|Layer_PlayerBullets|Layer_Player|Layer_PhysObjects|Layer_Trigger|Layer_Door|Layer_InterDebris|Layer_Player2|Layer_NPCBullet|Layer_Clip;
    switch (layer) {
        case Layer_Default:           return Layer_Default|Layer_TransparentFX|Layer_IgnoreRaycast|Layer_Geometry|Layer_NPC|Layer_PlayerBullets|Layer_Player|Layer_Corpse|Layer_PhysObjects|Layer_Sky|Layer_Trigger|Layer_Door|Layer_InterDebris|Layer_Player2|Layer_Player3|Layer_Player4|Layer_NPCBullet|Layer_Clip|Layer_CorpseSearchable;
        case Layer_Geometry:          return Layer_Default|Layer_TransparentFX|Layer_IgnoreRaycast|Layer_Geometry|Layer_NPC|Layer_PlayerBullets|Layer_Player|Layer_PhysObjects|Layer_Trigger|Layer_Door|Layer_InterDebris|Layer_Player2|Layer_Clip;
        case Layer_NPC:               return Layer_Default|Layer_TransparentFX|Layer_IgnoreRaycast|Layer_Geometry|Layer_NPC|Layer_PlayerBullets|Layer_Player|Layer_PhysObjects|Layer_Trigger|Layer_NPCTrigger|Layer_Door|Layer_InterDebris|Layer_Player2|Layer_NPCBullet|Layer_NPCClip|Layer_Clip;
        case Layer_PlayerBullets:     return Layer_Default|Layer_TransparentFX|Layer_IgnoreRaycast|Layer_Geometry|Layer_NPC|Layer_PlayerBullets|Layer_Player|Layer_Corpse|Layer_PhysObjects|Layer_Door|Layer_InterDebris|Layer_Player2|Layer_NPCBullet|Layer_Clip|Layer_CorpseSearchable;
        case Layer_Player:            return Layer_Default|Layer_TransparentFX|Layer_IgnoreRaycast|Layer_Geometry|Layer_NPC|Layer_PhysObjects|Layer_PlayerTriggerOnly|Layer_Trigger|Layer_Door|Layer_Player2|Layer_NPCBullet|Layer_Clip;
        case Layer_Corpse:            return Layer_Default|Layer_Geometry|Layer_PlayerBullets|Layer_PhysObjects|Layer_Door|Layer_NPCBullet|Layer_Clip;
        case Layer_PhysObjects:       return Layer_Default|Layer_TransparentFX|Layer_IgnoreRaycast|Layer_Geometry|Layer_NPC|Layer_PlayerBullets|Layer_Player|Layer_Corpse|Layer_PhysObjects|Layer_Door|Layer_InterDebris|Layer_NPCBullet|Layer_Clip;
        case Layer_Sky:               return Layer_Default|Layer_Player;
        case Layer_PlayerTriggerOnly: return Layer_Player|Layer_Player2|Layer_Player3;
        case Layer_Trigger:           return Layer_Default|Layer_Geometry|Layer_NPC|Layer_PlayerBullets|Layer_Player|Layer_PhysObjects|Layer_Door|Layer_InterDebris|Layer_Clip;
        case Layer_Door:              return Layer_Default|Layer_TransparentFX|Layer_IgnoreRaycast|Layer_Geometry|Layer_NPC|Layer_PlayerBullets|Layer_Player|Layer_Corpse|Layer_PhysObjects|Layer_Trigger|Layer_Door|Layer_InterDebris|Layer_Player2|Layer_NPCBullet|Layer_Clip;
        case Layer_InterDebris:       return Layer_Default|Layer_Geometry|Layer_NPC|Layer_PlayerBullets|Layer_PhysObjects|Layer_Trigger|Layer_Door|Layer_NPCBullet|Layer_Clip;
        case Layer_Player2:           return Layer_Default|Layer_TransparentFX|Layer_IgnoreRaycast|Layer_Geometry|Layer_NPC|Layer_PlayerBullets|Layer_Player|Layer_PhysObjects|Layer_PlayerTriggerOnly|Layer_Trigger|Layer_Door|Layer_InterDebris|Layer_Player2|Layer_NPCBullet|Layer_Clip;
        case Layer_Player3:           return Layer_Default|Layer_TransparentFX|Layer_IgnoreRaycast|Layer_Geometry|Layer_NPC|Layer_PlayerBullets|Layer_Player|Layer_PhysObjects|Layer_PlayerTriggerOnly|Layer_Trigger|Layer_Door|Layer_InterDebris|Layer_Player3|Layer_NPCBullet|Layer_Clip;
        case Layer_Player4:           return Layer_Default|Layer_TransparentFX|Layer_IgnoreRaycast|Layer_Geometry|Layer_NPC|Layer_PlayerBullets|Layer_Player|Layer_PhysObjects|Layer_PlayerTriggerOnly|Layer_Trigger|Layer_Door|Layer_InterDebris|Layer_Player4|Layer_NPCBullet|Layer_Clip;
        case Layer_NPCBullet:         return Layer_Default|Layer_TransparentFX|Layer_IgnoreRaycast|Layer_Geometry|Layer_PlayerBullets|Layer_Player|Layer_Corpse|Layer_PhysObjects|Layer_Door|Layer_InterDebris|Layer_Player2|Layer_Clip|Layer_CorpseSearchable;
        case Layer_Clip:              return Layer_Player|Layer_Player2|Layer_Player3|Layer_Player4|Layer_NPC;
        case Layer_CorpseSearchable:  return Layer_Default|Layer_PlayerBullets;
        default:                      return 0u;
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
void DrawBoxCollider(Entity* e) {
    ShapeBox b; Entity_GetBox(e,&b);
    Vector3 ax,ay,az; obb_axes(b.rot,&ax,&ay,&az);
    Vector3 px = scale_vector3(ax,b.halfExtents.x); // Scale axes to half extents
    Vector3 py = scale_vector3(ay,b.halfExtents.y);
    Vector3 pz = scale_vector3(az,b.halfExtents.z);
    Vector3 c[8]; // 8 corners
    for (int s = 0; s < 8; s++) {
        float sx = (s&1) ? 1.f : -1.f;
        float sy = (s&2) ? 1.f : -1.f;
        float sz = (s&4) ? 1.f : -1.f;
        c[s] = Vector3_A_plus_B(b.center,Vector3_A_plus_B(Vector3_A_plus_B(scale_vector3(px, sx),scale_vector3(py, sy)),scale_vector3(pz, sz)));
    }

    AddDebugLine(c[0],c[1],textColors[TEXT_GREEN]); AddDebugLine(c[2],c[3],textColors[TEXT_GREEN]); // 12 edges
    AddDebugLine(c[4],c[5],textColors[TEXT_GREEN]); AddDebugLine(c[6],c[7],textColors[TEXT_GREEN]);
    AddDebugLine(c[0],c[2],textColors[TEXT_GREEN]); AddDebugLine(c[1],c[3],textColors[TEXT_GREEN]);
    AddDebugLine(c[4],c[6],textColors[TEXT_GREEN]); AddDebugLine(c[5],c[7],textColors[TEXT_GREEN]);
    AddDebugLine(c[0],c[4],textColors[TEXT_GREEN]); AddDebugLine(c[1],c[5],textColors[TEXT_GREEN]);
    AddDebugLine(c[2],c[6],textColors[TEXT_GREEN]); AddDebugLine(c[3],c[7],textColors[TEXT_GREEN]);
}

void DrawSphereCollider(Entity *e) {
    ShapeSphere s; Entity_GetSphere(e,&s);
    float step = 6.28318530f / 12;
    for (int seg = 0; seg < 12; seg++) {
        float a0 = seg * step, a1 = a0 + step; float c0 = vcosf(a0), s0 = vsinf(a0), c1 = vcosf(a1), s1 = vsinf(a1);
        AddDebugLine(Vector3_A_plus_B(s.center,(Vector3){c0*s.radius,0,s0*s.radius}),Vector3_A_plus_B(s.center,(Vector3){c1*s.radius,0,s1*s.radius}),textColors[TEXT_DARK_YELLOW]); // XZ plane
        AddDebugLine(Vector3_A_plus_B(s.center,(Vector3){c0*s.radius,s0*s.radius,0}),Vector3_A_plus_B(s.center,(Vector3){c1*s.radius,s1*s.radius,0}),textColors[TEXT_DARK_YELLOW]); // XY plane
        AddDebugLine(Vector3_A_plus_B(s.center,(Vector3){0,c0*s.radius,s0*s.radius}),Vector3_A_plus_B(s.center,(Vector3){0,c1*s.radius,s1*s.radius}),textColors[TEXT_DARK_YELLOW]); // YZ plane
    }
}

void DrawConvexMeshCollider(Entity *e) {
    u16 mi = e->colliderMeshIndex;
    if (mi == MODEL_IDX_MAX || mi >= loadedModelsMaxIndex) return;
    u32 triCount = modelTriangleCounts[mi];
    if (!triCount) return;

    u16 idx = (u16)(e - Sys_Global.instances);
    float M[16]; CopyMemoryFromBtoAForNBytes(M,&modelMatrices[idx * 16], 64);
    float m00=M[0],m10=M[1],m20=M[2]; float m01=M[4],m11=M[5],m21=M[6]; float m02=M[8],m12=M[9],m22=M[10]; float tx=M[12],ty=M[13],tz=M[14];
    for (u32 j = 0; j < triCount; j++) {
        u32 bA = (u32)modelTriangles[mi][j*3+0] * VERTEX_ATTRIBUTES_SIZE;
        u32 bB = (u32)modelTriangles[mi][j*3+1] * VERTEX_ATTRIBUTES_SIZE;
        u32 bC = (u32)modelTriangles[mi][j*3+2] * VERTEX_ATTRIBUTES_SIZE;
        Vector3 lA = {half_to_float(*(half*)(modelVertices[mi]+bA+0)),half_to_float(*(half*)(modelVertices[mi]+bA+2)),half_to_float(*(half*)(modelVertices[mi]+bA+4))};
        Vector3 lB = {half_to_float(*(half*)(modelVertices[mi]+bB+0)),half_to_float(*(half*)(modelVertices[mi]+bB+2)),half_to_float(*(half*)(modelVertices[mi]+bB+4))};
        Vector3 lC = {half_to_float(*(half*)(modelVertices[mi]+bC+0)),half_to_float(*(half*)(modelVertices[mi]+bC+2)),half_to_float(*(half*)(modelVertices[mi]+bC+4))};
        #define XFORM(v) (Vector3){ m00*(v).x + m01*(v).y + m02*(v).z + tx, m10*(v).x + m11*(v).y + m12*(v).z + ty, m20*(v).x + m21*(v).y + m22*(v).z + tz }
        Vector3 wA = XFORM(lA), wB = XFORM(lB), wC = XFORM(lC);
        #undef XFORM
        AddDebugLine(wA,wB,textColors[TEXT_GREEN]);
        AddDebugLine(wB,wC,textColors[TEXT_GREEN]);
        AddDebugLine(wC,wA,textColors[TEXT_GREEN]);
    }
}

void DrawCapsuleCollider(Entity *e) {
    ShapeCapsule cap; Entity_GetCapsule(e,&cap);
    Vector3 axis = normalize_vector3(Vector3_A_minus_B(cap.tip, cap.base)); // Capsule axis direction and perpendiculars
    Vector3 ref = (vabs(axis.y) < 0.9f) ? (Vector3){0,1,0} : (Vector3){1,0,0};
    Vector3 perp0 = normalize_vector3(cross_vector3(axis,ref)); Vector3 perp1 = cross_vector3(axis,perp0); // Build two vectors perpendicular to axis
    #define CAPS_SEGS 12
    float step = 6.28318530f / CAPS_SEGS, r = cap.radius;
    for (int seg = 0; seg < CAPS_SEGS; seg++) { // Full circle around capsule axis at base and tip (the "belt" lines)
        float a0 = seg * step, a1 = a0 + step; float c0 = vcosf(a0), s0 = vsinf(a0), c1 = vcosf(a1), s1 = vsinf(a1);
        Vector3 r0 = Vector3_A_plus_B(scale_vector3(perp0,c0*r),scale_vector3(perp1,s0*r));
        Vector3 r1 = Vector3_A_plus_B(scale_vector3(perp0,c1*r),scale_vector3(perp1,s1*r));
        AddDebugLine(Vector3_A_plus_B(cap.base,r0),Vector3_A_plus_B(cap.base,r1),textColors[TEXT_GREEN]); // Belt at base
        AddDebugLine(Vector3_A_plus_B(cap.tip, r0),Vector3_A_plus_B(cap.tip, r1),textColors[TEXT_GREEN]); // Belt at tip
    }

    #define HEMI_SEGS 6  // half of 12
    for (int seg = 0; seg < HEMI_SEGS; seg++) { // Hemisphere arcs — half circle only (pi), 2 perpendicular planes per end
        float a0 = seg * step, a1 = a0 + step; float c0 = vcosf(a0), s0 = vsinf(a0), c1 = vcosf(a1), s1 = vsinf(a1);
        Vector3 bA0 = Vector3_A_plus_B(scale_vector3(perp0, c0*r), scale_vector3(axis, -s0*r)); // Base hemisphere — arc curves away from tip (negative axis)
        Vector3 bA1 = Vector3_A_plus_B(scale_vector3(perp0, c1*r), scale_vector3(axis, -s1*r));
        Vector3 bB0 = Vector3_A_plus_B(scale_vector3(perp1, c0*r), scale_vector3(axis, -s0*r));
        Vector3 bB1 = Vector3_A_plus_B(scale_vector3(perp1, c1*r), scale_vector3(axis, -s1*r));
        AddDebugLine(Vector3_A_plus_B(cap.base, bA0), Vector3_A_plus_B(cap.base, bA1),textColors[TEXT_GREEN]);
        AddDebugLine(Vector3_A_plus_B(cap.base, bB0), Vector3_A_plus_B(cap.base, bB1),textColors[TEXT_GREEN]);
        Vector3 tA0 = Vector3_A_plus_B(scale_vector3(perp0, c0*r), scale_vector3(axis, s0*r)); // Tip hemisphere — arc curves away from base (positive axis)
        Vector3 tA1 = Vector3_A_plus_B(scale_vector3(perp0, c1*r), scale_vector3(axis, s1*r));
        Vector3 tB0 = Vector3_A_plus_B(scale_vector3(perp1, c0*r), scale_vector3(axis, s0*r));
        Vector3 tB1 = Vector3_A_plus_B(scale_vector3(perp1, c1*r), scale_vector3(axis, s1*r));
        AddDebugLine(Vector3_A_plus_B(cap.tip, tA0), Vector3_A_plus_B(cap.tip, tA1),textColors[TEXT_GREEN]);
        AddDebugLine(Vector3_A_plus_B(cap.tip, tB0), Vector3_A_plus_B(cap.tip, tB1),textColors[TEXT_GREEN]);
    }
    #undef HEMI_SEGS

    for (int seg = 0; seg < 4; seg++) { // 4 spine lines connecting base belt to tip belt
        float a = seg * (6.28318530f / 4.0f);
        Vector3 off = Vector3_A_plus_B(scale_vector3(perp0, vcosf(a)*r), scale_vector3(perp1, vsinf(a)*r));
        AddDebugLine(Vector3_A_plus_B(cap.base, off), Vector3_A_plus_B(cap.tip, off),textColors[TEXT_GREEN]);
    }
    #undef CAPS_SEGS
}

float GetCollisionRadius(Entity *e) { return (e->collider == COLLIDER_TYPE_BOX) ? vmax(e->colliderSize.x,vmax(e->colliderSize.y,e->colliderSize.z)) : e->colliderSize.x; }
static inline Vector3 ApplyGravity(Vector3 vel, float selfGravity, float dt) { vel.y += (-9.81f * selfGravity) * dt; return vel; }
static void* PhysicsWorkerFunc(void *arg) {
    u32 workerIdx = (u32)(uintptr_t)arg;
    u32 lastGen = 0;
    while (__atomic_load_n(&g_running,__ATOMIC_ACQUIRE)) {
        pthread_mutex_lock(&g_dispatchMutex);
        while (__atomic_load_n(&g_dispatchGen,__ATOMIC_ACQUIRE) == lastGen && __atomic_load_n(&g_running,__ATOMIC_ACQUIRE)) pthread_cond_wait(&g_dispatchCond,&g_dispatchMutex);
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
            u32 mask = GetCollisionMask(e->layer);
            bool hit = false;
            Vector3 hitNormal = {0,1,0};
            for (u16 j = 0; j < INSTANCE_COUNT && !hit; ++j) {
                if (j == idx) continue;
                Entity *o = &Sys_Global.instances[j];
                if (o->collider == COLLIDER_TYPE_NONE || !(mask & o->layer)) continue;
               
                Vector3 savedPos = e->position;  e->position = newPos; Vector3 oSavedPos = o->position; o->position = g_posSnapshot[j];
                ShapeCapsule capA,capB; ShapeBox boxA,boxB;
                OverlapResult r = {0};
                if (e->collider==COLLIDER_TYPE_CAPSULE && o->collider==COLLIDER_TYPE_CAPSULE)  { Entity_GetCapsule(e,&capA); Entity_GetCapsule(o,&capB); r = CapsuleCapsule(capA,capB); }
                else if (e->collider==COLLIDER_TYPE_CAPSULE && o->collider==COLLIDER_TYPE_BOX) { Entity_GetCapsule(e,&capA); Entity_GetBox(o,&boxB);     r = CapsuleBox(capA,boxB); }
                else if (e->collider==COLLIDER_TYPE_BOX && o->collider==COLLIDER_TYPE_CAPSULE) { Entity_GetBox(e,&boxA); Entity_GetCapsule(o,&capB);     r = CapsuleBox(capB,boxA); }
                else if (e->collider==COLLIDER_TYPE_BOX && o->collider==COLLIDER_TYPE_BOX)     { Entity_GetBox(e,&boxA); Entity_GetBox(o,&boxB);         r = BoxBox(boxA,boxB); }
                else                                                                           { r = SphereSphere(newPos,GetCollisionRadius(e),g_posSnapshot[j],GetCollisionRadius(o)); }
               
                if (r.hit) { hit = true; hitNormal = r.normal; }
                o->position = oSavedPos;
                e->position = savedPos;
            }
           
            if (!hit) { ps->lastPos = ps->pos; ps->pos = newPos; ps->vel = vel; }
            else {
                float vdn = dot_vector3(vel, hitNormal);
                if (vdn < 0) {
                    vel = Vector3_A_minus_B(vel, scale_vector3(hitNormal, vdn));
                    Vector3 tangent = Vector3_A_minus_B(vel, scale_vector3(hitNormal, dot_vector3(vel, hitNormal)));
                    float tangentSpeed = magnitude_vector3(tangent);
                    if (tangentSpeed > COLLISION_EPSILON) {
                        float frictionScale = 0.95f;
                        vel = Vector3_A_plus_B(scale_vector3(tangent, frictionScale),
                                               scale_vector3(hitNormal, dot_vector3(vel, hitNormal)));
                    } else {
                        vel = Vector3_A_plus_B(scale_vector3(vel, 0.0f), scale_vector3(hitNormal, dot_vector3(vel, hitNormal)));
                    }
                }
                newPos = Vector3_A_plus_B(ps->pos, scale_vector3(vel,sl->dt));
                ps->lastPos=ps->pos; ps->pos=newPos; ps->vel=vel;
            }
            e->accumulatedForce = (Vector3){0, 0, 0};
        }
        if (__atomic_sub_fetch(&g_workRemaining, 1, __ATOMIC_RELEASE) == 0) { pthread_mutex_lock(&g_doneMutex); pthread_cond_broadcast(&g_doneCond); pthread_mutex_unlock(&g_doneMutex); }
    }
    return NULL;
}

void InitPhysics(void) {
    u32 cores = (u32)OS_GetNumThreads();
    g_workerCnt = (cores <= 2) ? 1u : (cores - 2 < MAX_WORKERS ? cores - 2 : MAX_WORKERS); g_running = 1;
    pthread_mutex_init(&g_dispatchMutex, NULL);
    pthread_cond_init (&g_dispatchCond, NULL);
    pthread_mutex_init(&g_doneMutex, NULL);
    pthread_cond_init (&g_doneCond, NULL);
    for (u32 i = 0; i < g_workerCnt; ++i) { pthread_t t; pthread_create(&t,NULL,PhysicsWorkerFunc,(void*)(uintptr_t)i); pthread_detach(t); }
    for (u16 i = 0; i < INSTANCE_COUNT; ++i) {
        Entity *a = &Sys_Global.instances[i];
        if (a->collider == COLLIDER_TYPE_NONE) continue;
        for (u16 j = i+1; j < INSTANCE_COUNT; ++j) {
            Entity *b = &Sys_Global.instances[j];
            if (b->collider == COLLIDER_TYPE_NONE) continue;
            Vector3 savedA = a->position;
            Vector3 savedB = b->position;
            ShapeCapsule capA, capB; ShapeBox boxA, boxB;
            OverlapResult r = {0};
            if (a->collider==COLLIDER_TYPE_CAPSULE && b->collider==COLLIDER_TYPE_CAPSULE)  { Entity_GetCapsule(a,&capA); Entity_GetCapsule(b,&capB); r = CapsuleCapsule(capA,capB); }
            else if (a->collider==COLLIDER_TYPE_CAPSULE && b->collider==COLLIDER_TYPE_BOX) { Entity_GetCapsule(a,&capA); Entity_GetBox(b,&boxB);     r = CapsuleBox(capA,boxB); }
            else if (a->collider==COLLIDER_TYPE_BOX && b->collider==COLLIDER_TYPE_CAPSULE) { Entity_GetBox(a,&boxA); Entity_GetCapsule(b,&capB);     r = CapsuleBox(capB,boxA); }
            else if (a->collider==COLLIDER_TYPE_BOX && b->collider==COLLIDER_TYPE_BOX)     { Entity_GetBox(a,&boxA); Entity_GetBox(b,&boxB);         r = BoxBox(boxA,boxB); }
            else                                                                           { r = SphereSphere(a->position, GetCollisionRadius(a), b->position, GetCollisionRadius(b)); }
            
            if (r.hit) DualLogError("Initial overlap between entities %u and %u!\n",i,j);
            a->position = savedA; b->position = savedB;
        }
    }
    
    DualLog("Physics: %u workers, substep size %.4fs\n", g_workerCnt, MAX_STEP_SIZE);
}

void Physics(bool* playerMoved) {
    // 1. Step Timing.  Since physics runs on every frame (no fixed step!  fully continuous & prior to every render!), determine time elapsed since last time physics ran (using relative time when unpaused) and split into "substeps".
    Sys_Global.timeSinceLastPhysicsTick = Sys_Global.pauseRelativeTime - Sys_Global.last_physics_time;
    float dt = vclamp((float)Sys_Global.timeSinceLastPhysicsTick,0.0005f,0.1f); // Limit to sensible value to prevent issues.
    if (unlikely(Sys_Global.gamePaused || Sys_Global.menuActive)) return;
    
    Sys_Global.last_physics_time = Sys_Global.pauseRelativeTime;
    u8 substeps = (u8)vclamp((u32)(dt / MAX_STEP_SIZE + 0.5f), 1u, (u32)MAX_SUBSTEPS); // Substep count: at least 1, at most MAX_SUBSTEPS, sized so no object travels more than MIN_DIAMETER of smallest object in a single substep at MAX_SPEED.
    float dtsub = dt / (float)substeps;
    
    // 2a. Collect Dynamic Entities.  Determines and finds all objects that will need updated by the physics system.  Needs to run every time as gamecode could remove entities when picked up or killed.
    MemSetToVForNBytes(dynamicEntities,0,512 * sizeof(u16)); // none
    dynamicEntityCount = 0;
    for (int i=0;i<Sys_Global.loadedInstances;++i) {
        if (dynamicEntityCount >= 512) { dynamicEntityCount = 512; assert(false); break; }
        if (Sys_Global.instances[i].entflags&ENTFLAG_RIGIDBODY && Sys_Global.instances[i].entflags&ENTFLAG_ACTIVE) { dynamicEntities[dynamicEntityCount] = i; dynamicEntityCount++; }
    }
    
    // 2b. Update All Entity Cell/Radius Data (for Grid Based Broadphase using global xz grid, uses 2.56f * 2.56f sized cells, edges start at Sys_Global.worldMin_x and Sys_Global.worldMin_z, fixed 64 cells wide in x and z for 64*64 = 4096 cells total)
    for (int i=PLAYER1;i<Sys_Global.loadedInstances;++i) { // Starts at PLAYER1 (1), skipping null == world == 0.  Players treated same as any other dynamical.
        Entity* e = &Sys_Global.instances[i];
        i32 cellIdx = PosGetCellCoords(e->position.x,e->position.z);
        e->cellIndex = cellIdx; e->cellX=PosGetCellCoordX(e->position.x); e->cellZ=PosGetCellCoordZ(e->position.z);
        e->radius = modelBounds[e->modelIndex]*vmax(vmax(e->scale.x,e->scale.y),e->scale.z);
        e->shadRadius = e->radius * 1.41;
    }
    
    // 2c. Transfer Entity Data To Each Thread's Full World Copy
    for (u16 i = 0; i < dynamicEntityCount; ++i) {
        u16 idx = dynamicEntities[i];
        if (idx >= INSTANCE_COUNT) continue;
        Entity *e = &Sys_Global.instances[idx];
        g_phys[idx].pos     = e->position;
        g_phys[idx].rot     = e->rotation;
        g_phys[idx].gravity = e->gravity;
        g_phys[idx].lastPos = e->lastPosition;
        g_phys[idx].vel     = e->velocity;
        g_phys[idx].avel    = e->angularVelocity;
        g_phys[idx].mass    = e->mass;
        g_phys[idx].inertia = e->inertia;
        g_phys[idx].adrag   = e->angularDrag;
        g_phys[idx].dfric   = e->dynamicFriction;
        g_phys[idx].sfric   = e->staticFriction;
        g_phys[idx].bouncy  = e->bounciness;
        g_phys[idx].fComb   = e->frictionCombine;
        g_phys[idx].bComb   = e->bounceCombine;
    }
    
    for (u8 s=0;s<substeps;++s) { // Hopefully no more than 1 per frame.
        for (u16 i = 0; i < INSTANCE_COUNT; ++i) g_posSnapshot[i] = Sys_Global.instances[i].position; // Snapshot all entity positions so workers share a consistent read view.

        // Partition dynamicEntities[0..dynamicEntityCount) across workers.
        u16 total=dynamicEntityCount, per=total / g_workerCnt, rem=total % g_workerCnt, start=0;
        for (u32 i=0;i<g_workerCnt;++i) { g_slices[i].start = start; g_slices[i].end = start + per + (i < rem ? 1 : 0); g_slices[i].dt= dtsub; start = g_slices[i].end; }

        // GO WIDE!  Singlethreaded -> MULTITHREADED (While these threads are persistent after init, they should be idle until we get to here and go back to idle after finishing their own slice of the physics world in less than 13ms we hope)
        __atomic_store_n(&g_workRemaining,g_workerCnt,__ATOMIC_RELEASE); // Arm the completion counter before waking workers to avoid the race where a fast worker completes and signals done before we start waiting.
        pthread_mutex_lock(&g_dispatchMutex);
        __atomic_fetch_add(&g_dispatchGen,1,__ATOMIC_RELEASE); // Wake all workers by incrementing the generation counter.
        pthread_cond_broadcast(&g_dispatchCond);
        pthread_mutex_unlock(&g_dispatchMutex);
        pthread_mutex_lock(&g_doneMutex);
        while (__atomic_load_n(&g_workRemaining,__ATOMIC_ACQUIRE) != 0) pthread_cond_wait(&g_doneCond,&g_doneMutex); // Block until every worker has decremented g_workRemaining to 0.
        pthread_mutex_unlock(&g_doneMutex);
        // GO NARROW! MULTITHREADED -> Singlethreaded

        // Flush physics results -> live entity positions (main thread only).
        for (u16 i=0;i<dynamicEntityCount;++i) { u16 idx = dynamicEntities[i]; Sys_Global.instances[idx].position = g_phys[idx].pos; Sys_Global.instances[idx].velocity  = g_phys[idx].vel; }
    }
    
    Vector3 pDelta = Vector3_A_minus_B(Sys_Global.instances[PLAYER1].lastPosition,Sys_Global.instances[PLAYER1].position);
    *playerMoved = ((vabs(pDelta.x) + vabs(pDelta.y) + vabs(pDelta.z)) > 0.02f);
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
}
