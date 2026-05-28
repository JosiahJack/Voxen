// physics.c - Discrete parallel physics with substep safety.  Threading model: persistent worker threads, single mutex+condvar pair for dispatch (main->workers) and a separate pair for completion (workers->main).  Workers read a snapshot of ALL entity positions taken before dispatch (no cross-worker position races), write results of current entity and affecting entities only into their own complete copy of the world in g_phys[] then each writes their slice back to Sys_Global.instances[] at the end.
#include "os.h"
#include "common.h"
#include "interop.h"
#define MAX_COLLISION_ITERATIONS 4
#define RESTITUTION 0.5f
#define FRICTION 0.2f
#define STEP_MIN_NORMAL_Y 0.7f
#define COLLISION_EPSILON 0.0001f
#define MAX_SUBSTEPS 10
#define MIN_DIAMETER 0.1f // m
#define MAX_SPEED 10.0f // m/s
#define MAX_STEP_SIZE (MIN_DIAMETER / MAX_SPEED) // 0.01 s
#define MAX_ANGULAR_SPEED 5.0f
u16 dynamicEntities[512], dynamicEntityCount; extern GlobalContext Sys_Global; extern CheatsSystem Sys_Cheats; extern u16 loadedModelsMaxIndex; extern float modelBounds[MODEL_IDX_MAX]; extern Color textColors[];
void* MemSetToVForNBytes(void *dst, int c, size_t n); void* CopyMemoryFromBtoAForNBytes(void *dst, const void *src, size_t n); float half_to_float(half h);
typedef struct { bool hit; Vector3 point,normal; float overlapAmount; } OverlapResult;
extern u8** modelVertices; extern u16** modelTriangles; extern u32 modelVertexCounts[MODEL_IDX_MAX]; extern u16 modelTriangleCounts[MODEL_IDX_MAX]; extern float modelMatrices[INSTANCE_COUNT*16]; extern GlobalContext Sys_Global;
static inline __attribute__((always_inline)) i32 PosGetCellCoordX(float pos_x) { return (u16)clamp((i32)vfloor((pos_x - Sys_Global.worldMin_x + CELLXHALF) / CELL_SIZE), 0, WORLDX_0BASED); }
static inline __attribute__((always_inline)) i32 PosGetCellCoordZ(float pos_z) { return (u16)clamp((i32)vfloor((pos_z - Sys_Global.worldMin_z + CELLXHALF) / CELL_SIZE), 0, WORLDX_0BASED); }
static inline __attribute__((always_inline)) u32 PosGetCellCoordsP(i32 cx, i32 cz) { cx = clamp(cx,0,WORLDX_0BASED); cz = clamp(cz,0,WORLDX_0BASED); return (u32)cz * WORLDX + (u32)cx; }
static inline OverlapResult SphereSphere(Vector3 a, float ar, Vector3 b, float br) {
    Vector3 delta = V3_AsubB(a,b); float dist = V3_Mag(delta), radSum = (ar + br); Vector3 n = (dist < COLLISION_EPSILON) ? (Vector3){0,1,0} : V3_ScaleByF(delta,1.0f / dist);
    return (dist < radSum) ? (OverlapResult){true,V3_AplusB(b,V3_ScaleByF(n,br)),n,radSum - dist} : (OverlapResult){0,{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f},0.0f};
}

static float ClosestSegmentSegment(Vector3 a0, Vector3 a1, Vector3 b0, Vector3 b1, float *sc, float *tc) { // Closest point between two line segments A0-A1 and B0-B1.  Returns squared distance and writes sc, tc (parameters on each segment).
    Vector3 d1 = V3_AsubB(a1,a0), d2 = V3_AsubB(b1,b0), r = V3_AsubB(a0,b0);
    float a = V3_dot(d1,d1), e = V3_dot(d2,d2), f = V3_dot(d2,r);
    if (a < COLLISION_EPSILON && e < COLLISION_EPSILON) { *sc = *tc = 0.0f; return V3_dot(r,r); }
    if (a < COLLISION_EPSILON) { *sc = 0.0f; *tc = vclamp(f/e, 0.0f, 1.0f); }
    else {
        float c = V3_dot(d1,r);
        if (e < COLLISION_EPSILON) { *tc = 0.0f; *sc = vclamp(-c/a, 0.0f, 1.0f); }
        else {
            float b = V3_dot(d1,d2), denom = a*e - b*b;
            *sc = (denom > COLLISION_EPSILON) ? vclamp((b*f - c*e)/denom, 0.0f, 1.0f) : 0.0f;
            *tc = (b * (*sc) + f) / e;
            if (*tc < 0.0f) { *tc = 0.0f; *sc = vclamp(-c/a, 0.0f, 1.0f); }
            else if (*tc > 1.0f) { *tc = 1.0f; *sc = vclamp((b-c)/a, 0.0f, 1.0f); }
        }
    }
    Vector3 diff = V3_AsubB(V3_AplusB(a0, V3_ScaleByF(d1,*sc)), V3_AplusB(b0, V3_ScaleByF(d2,*tc)));
    return V3_dot(diff,diff);
}

static OverlapResult CapsuleCapsule(ShapeCapsule a, ShapeCapsule b) {
    OverlapResult r = {0};
    float sc, tc; float distSq = ClosestSegmentSegment(a.base, a.tip, b.base, b.tip, &sc, &tc);
    float radSum = a.radius + b.radius; if (distSq >= radSum * radSum) return r;
    
    float dist = vsqrtf(vmax(distSq, 0.0f));
    r.overlapAmount = radSum - dist; r.hit = true;
    Vector3 ptA = V3_AplusB(a.base,V3_ScaleByF(V3_AsubB(a.tip,a.base),sc)); 
    Vector3 ptB = V3_AplusB(b.base,V3_ScaleByF(V3_AsubB(b.tip,b.base),tc));
    Vector3 delta = V3_AsubB(ptA,ptB);
    r.normal = (dist < COLLISION_EPSILON) ? (Vector3){0,1,0} : V3_ScaleByF(delta, 1.0f/dist);
    r.point  = V3_AplusB(ptB, V3_ScaleByF(r.normal, b.radius));
    return r;
}

void obb_axes(Quaternion q, Vector3 *ax, Vector3 *ay, Vector3 *az) { *ax=quat_rotate_vector(q,(Vector3){1,0,0}); *ay=quat_rotate_vector(q,(Vector3){0,1,0}); *az=quat_rotate_vector(q,(Vector3){0,0,1}); }
static Vector3 ClosestPointOBB(Vector3 p, ShapeBox b) {
    Vector3 ax, ay, az; obb_axes(b.rot, &ax, &ay, &az);
    Vector3 d = V3_AsubB(p, b.center);
    float lx = V3_dot(d, ax), ly = V3_dot(d, ay), lz = V3_dot(d, az);
    lx = vclamp(lx,-b.halfExtents.x,b.halfExtents.x);
    ly = vclamp(ly,-b.halfExtents.y,b.halfExtents.y);
    lz = vclamp(lz,-b.halfExtents.z,b.halfExtents.z);
    Vector3 q = b.center;
    q = V3_AplusB(q,V3_ScaleByF(ax,lx));
    q = V3_AplusB(q,V3_ScaleByF(ay,ly));
    q = V3_AplusB(q,V3_ScaleByF(az,lz));
    return q;
}

static OverlapResult SphereOBB(Vector3 center, float radius, ShapeBox box) {
    OverlapResult r = {0};
    Vector3 closest = ClosestPointOBB(center, box);
    Vector3 delta = V3_AsubB(center, closest);
    float distSq = V3_dot(delta, delta);
    if (distSq >= radius * radius) return r;

    float dist = vsqrtf(vmax(distSq, 0.0f));
    r.hit = true;
    if (dist > COLLISION_EPSILON) { r.normal = V3_ScaleByF(delta, 1.0f / dist); r.overlapAmount = radius - dist; }
    else {
        // Center is inside OBB — find minimum penetration axis
        Vector3 ax, ay, az; obb_axes(box.rot, &ax, &ay, &az);
        Vector3 local = V3_AsubB(center, box.center);
        float lx = V3_dot(local, ax), ly = V3_dot(local, ay), lz = V3_dot(local, az);
        float dx = box.halfExtents.x - vabs(lx), dy = box.halfExtents.y - vabs(ly), dz = box.halfExtents.z - vabs(lz);
        if (dx < dy && dx < dz) { r.normal = V3_ScaleByF(ax, lx > 0 ? 1.f : -1.f); r.overlapAmount = radius + dx; }
        else if (dy < dz)       { r.normal = V3_ScaleByF(ay, ly > 0 ? 1.f : -1.f); r.overlapAmount = radius + dy; }
        else                    { r.normal = V3_ScaleByF(az, lz > 0 ? 1.f : -1.f); r.overlapAmount = radius + dz; }
    }
    r.point = closest;
    return r;
}

static OverlapResult CapsuleBox(ShapeCapsule cap, ShapeBox box) {
    OverlapResult best = {0}; OverlapResult hitBase = SphereOBB(cap.base,cap.radius,box); OverlapResult hitTip = SphereOBB(cap.tip,cap.radius,box);
    Vector3 ax,ay,az; obb_axes(box.rot,&ax,&ay,&az); // Also test closest point on segment to OBB center (catches shaft collisions missed by endpoints)
    Vector3 seg = V3_AsubB(cap.tip, cap.base); // Project segment onto each OBB axis, clamp, find closest point on segment to that
    float segLen = V3_Mag(seg);
    Vector3 segDir = segLen > COLLISION_EPSILON ? V3_ScaleByF(seg, 1.0f/segLen) : (Vector3){0,1,0};
    Vector3 segPts[3]; int segPtCount = 0; // Closest point on segment to OBB: iterate 3 axis-constrained projections
    float d_ax = V3_dot(segDir, ax), d_ay = V3_dot(segDir, ay), d_az = V3_dot(segDir, az); // For each OBB face pair, clamp segment parameter to where segment crosses slab boundary
    Vector3 toBase = V3_AsubB(cap.base, box.center);
    if (vabs(d_ax) > COLLISION_EPSILON) { float t0 = (-box.halfExtents.x - V3_dot(toBase,ax))/d_ax, t1 = (box.halfExtents.x - V3_dot(toBase,ax))/d_ax; if (t0 > t1) { float tmp=t0; t0=t1; t1=tmp; } t0=vclamp(t0,0,segLen); t1=vclamp(t1,0,segLen); segPts[segPtCount++] = V3_AplusB(cap.base, V3_ScaleByF(segDir, (t0+t1)*0.5f)); }
    if (vabs(d_ay) > COLLISION_EPSILON && segPtCount < 3) { float t0 = (-box.halfExtents.y - V3_dot(toBase,ay))/d_ay, t1 = (box.halfExtents.y - V3_dot(toBase,ay))/d_ay; if (t0 > t1) { float tmp=t0; t0=t1; t1=tmp; } t0=vclamp(t0,0,segLen); t1=vclamp(t1,0,segLen); segPts[segPtCount++] = V3_AplusB(cap.base, V3_ScaleByF(segDir, (t0+t1)*0.5f)); }
    if (vabs(d_az) > COLLISION_EPSILON && segPtCount < 3) { float t0 = (-box.halfExtents.z - V3_dot(toBase,az))/d_az, t1 = (box.halfExtents.z - V3_dot(toBase,az))/d_az; if (t0 > t1) { float tmp=t0; t0=t1; t1=tmp; } t0=vclamp(t0,0,segLen); t1=vclamp(t1,0,segLen); segPts[segPtCount++] = V3_AplusB(cap.base, V3_ScaleByF(segDir, (t0+t1)*0.5f)); }
    for (int k = 0; k < segPtCount; ++k) {
        OverlapResult rK = SphereOBB(segPts[k],cap.radius,box);
        if (rK.hit && rK.overlapAmount > best.overlapAmount) { best = rK; best.overlapAmount = rK.overlapAmount; }
    }
    
    if (hitBase.hit && hitBase.overlapAmount > best.overlapAmount) { best = hitBase; best.overlapAmount = hitBase.overlapAmount; }
    if (hitTip.hit  && hitTip.overlapAmount  > best.overlapAmount) { best = hitTip;  best.overlapAmount = hitTip.overlapAmount; }
    return best;
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
    float r = e->colliderSize.x;
    float hi = vmax(0.0f, (e->colliderSize.y * 0.5f) - r);
    Vector3 wc = V3_AplusB(e->position, quat_rotate_vector(e->rotation, e->colliderCenter));
    Vector3 axis = (e->colliderSize.z < 0.5f) ? quat_rotate_vector(e->rotation, (Vector3){1,0,0})
                 : (e->colliderSize.z < 1.5f) ? quat_rotate_vector(e->rotation, (Vector3){0,1,0})
                 : quat_rotate_vector(e->rotation, (Vector3){0,0,1});
    out->radius = r;
    out->base = V3_AsubB(wc, V3_ScaleByF(axis, hi));
    out->tip  = V3_AplusB(wc, V3_ScaleByF(axis, hi));
}
void Entity_GetBox   (const Entity *e, ShapeBox    *out) { out->center = V3_AplusB(e->position, quat_rotate_vector(e->rotation, e->colliderCenter)); out->halfExtents = V3_ScaleByF(e->colliderSize, 0.5f); out->rot = e->rotation; }
void Entity_GetSphere(const Entity *e, ShapeSphere *out) { out->center = V3_AplusB(e->position, quat_rotate_vector(e->rotation, e->colliderCenter)); out->radius = e->colliderSize.x; }
void DrawBoxCollider(Entity* e) {
    ShapeBox b; Entity_GetBox(e,&b); Vector3 ax,ay,az,c[8],px,py,pz; obb_axes(b.rot,&ax,&ay,&az);
    px = V3_ScaleByF(ax,b.halfExtents.x); py = V3_ScaleByF(ay,b.halfExtents.y); pz = V3_ScaleByF(az,b.halfExtents.z);
    for (int s = 0; s < 8; s++) {
        float sx = (s&1) ? 1.f : -1.f, sy = (s&2) ? 1.f : -1.f, sz = (s&4) ? 1.f : -1.f;
        c[s] = V3_AplusB(b.center,V3_AplusB(V3_AplusB(V3_ScaleByF(px, sx),V3_ScaleByF(py, sy)),V3_ScaleByF(pz, sz)));
    }
    AddDebugLine(c[0],c[1],textColors[TEXT_GREEN]); AddDebugLine(c[2],c[3],textColors[TEXT_GREEN]);
    AddDebugLine(c[4],c[5],textColors[TEXT_GREEN]); AddDebugLine(c[6],c[7],textColors[TEXT_GREEN]);
    AddDebugLine(c[0],c[2],textColors[TEXT_GREEN]); AddDebugLine(c[1],c[3],textColors[TEXT_GREEN]);
    AddDebugLine(c[4],c[6],textColors[TEXT_GREEN]); AddDebugLine(c[5],c[7],textColors[TEXT_GREEN]);
    AddDebugLine(c[0],c[4],textColors[TEXT_GREEN]); AddDebugLine(c[1],c[5],textColors[TEXT_GREEN]);
    AddDebugLine(c[2],c[6],textColors[TEXT_GREEN]); AddDebugLine(c[3],c[7],textColors[TEXT_GREEN]);
}

void DrawSphereCollider(Entity *e) {
    ShapeSphere s; Entity_GetSphere(e,&s); float step = 6.28318530f / 12;
    for (int seg = 0; seg < 12; seg++) {
        float a0 = seg * step, a1 = a0 + step; float c0 = vcosf(a0), s0 = vsinf(a0), c1 = vcosf(a1), s1 = vsinf(a1);
        AddDebugLine(V3_AplusB(s.center,(Vector3){c0*s.radius,0,s0*s.radius}),V3_AplusB(s.center,(Vector3){c1*s.radius,0,s1*s.radius}),textColors[TEXT_DARK_YELLOW]);
        AddDebugLine(V3_AplusB(s.center,(Vector3){c0*s.radius,s0*s.radius,0}),V3_AplusB(s.center,(Vector3){c1*s.radius,s1*s.radius,0}),textColors[TEXT_DARK_YELLOW]);
        AddDebugLine(V3_AplusB(s.center,(Vector3){0,c0*s.radius,s0*s.radius}),V3_AplusB(s.center,(Vector3){0,c1*s.radius,s1*s.radius}),textColors[TEXT_DARK_YELLOW]);
    }
}

void DrawConvexMeshCollider(Entity *e) {
    u16 mi = e->colliderMeshIndex; if (mi == MODEL_IDX_MAX || mi >= loadedModelsMaxIndex) return;
    u32 triCount = modelTriangleCounts[mi]; if (!triCount) return;
    u16 idx = (u16)(e - Sys_Global.instances);
    float M[16]; CopyMemoryFromBtoAForNBytes(M,&modelMatrices[idx * 16], 64);
    float m00=M[0],m10=M[1],m20=M[2]; float m01=M[4],m11=M[5],m21=M[6]; float m02=M[8],m12=M[9],m22=M[10]; float tx=M[12],ty=M[13],tz=M[14];
    for (u32 j = 0; j < triCount; j++) {
        u32 bA = (u32)modelTriangles[mi][j*3+0] * VERTEX_ATTRIBUTES_SIZE; u32 bB = (u32)modelTriangles[mi][j*3+1] * VERTEX_ATTRIBUTES_SIZE; u32 bC = (u32)modelTriangles[mi][j*3+2] * VERTEX_ATTRIBUTES_SIZE;
        Vector3 lA = {half_to_float(*(half*)(modelVertices[mi]+bA+0)),half_to_float(*(half*)(modelVertices[mi]+bA+2)),half_to_float(*(half*)(modelVertices[mi]+bA+4))};
        Vector3 lB = {half_to_float(*(half*)(modelVertices[mi]+bB+0)),half_to_float(*(half*)(modelVertices[mi]+bB+2)),half_to_float(*(half*)(modelVertices[mi]+bB+4))};
        Vector3 lC = {half_to_float(*(half*)(modelVertices[mi]+bC+0)),half_to_float(*(half*)(modelVertices[mi]+bC+2)),half_to_float(*(half*)(modelVertices[mi]+bC+4))};
        #define XFORM(v) (Vector3){ m00*(v).x + m01*(v).y + m02*(v).z + tx, m10*(v).x + m11*(v).y + m12*(v).z + ty, m20*(v).x + m21*(v).y + m22*(v).z + tz }
        Vector3 wA = XFORM(lA), wB = XFORM(lB), wC = XFORM(lC);
        #undef XFORM
        AddDebugLine(wA,wB,textColors[TEXT_GREEN]); AddDebugLine(wB,wC,textColors[TEXT_GREEN]); AddDebugLine(wC,wA,textColors[TEXT_GREEN]);
    }
}

void DrawCapsuleCollider(Entity *e) {
    ShapeCapsule cap; Entity_GetCapsule(e,&cap);
    Vector3 axis = V3_Normalize(V3_AsubB(cap.tip, cap.base));
    Vector3 ref = (vabs(axis.y) < 0.9f) ? (Vector3){0,1,0} : (Vector3){1,0,0};
    Vector3 perp0 = V3_Normalize(V3_Cross(axis,ref)); Vector3 perp1 = V3_Cross(axis,perp0);
    #define CAPS_SEGS 12
    float step = 6.28318530f / CAPS_SEGS, r = cap.radius;
    for (int seg = 0; seg < CAPS_SEGS; seg++) {
        float a0 = seg * step, a1 = a0 + step; float c0 = vcosf(a0), s0 = vsinf(a0), c1 = vcosf(a1), s1 = vsinf(a1);
        Vector3 r0 = V3_AplusB(V3_ScaleByF(perp0,c0*r),V3_ScaleByF(perp1,s0*r));
        Vector3 r1 = V3_AplusB(V3_ScaleByF(perp0,c1*r),V3_ScaleByF(perp1,s1*r));
        AddDebugLine(V3_AplusB(cap.base,r0),V3_AplusB(cap.base,r1),textColors[TEXT_GREEN]);
        AddDebugLine(V3_AplusB(cap.tip, r0),V3_AplusB(cap.tip, r1),textColors[TEXT_GREEN]);
    }
    #define HEMI_SEGS 6
    for (int seg = 0; seg < HEMI_SEGS; seg++) {
        float a0 = seg * step, a1 = a0 + step; float c0 = vcosf(a0), s0 = vsinf(a0), c1 = vcosf(a1), s1 = vsinf(a1);
        Vector3 bA0 = V3_AplusB(V3_ScaleByF(perp0, c0*r), V3_ScaleByF(axis, -s0*r));
        Vector3 bA1 = V3_AplusB(V3_ScaleByF(perp0, c1*r), V3_ScaleByF(axis, -s1*r));
        Vector3 bB0 = V3_AplusB(V3_ScaleByF(perp1, c0*r), V3_ScaleByF(axis, -s0*r));
        Vector3 bB1 = V3_AplusB(V3_ScaleByF(perp1, c1*r), V3_ScaleByF(axis, -s1*r));
        AddDebugLine(V3_AplusB(cap.base, bA0), V3_AplusB(cap.base, bA1),textColors[TEXT_GREEN]);
        AddDebugLine(V3_AplusB(cap.base, bB0), V3_AplusB(cap.base, bB1),textColors[TEXT_GREEN]);
        Vector3 tA0 = V3_AplusB(V3_ScaleByF(perp0, c0*r), V3_ScaleByF(axis, s0*r));
        Vector3 tA1 = V3_AplusB(V3_ScaleByF(perp0, c1*r), V3_ScaleByF(axis, s1*r));
        Vector3 tB0 = V3_AplusB(V3_ScaleByF(perp1, c0*r), V3_ScaleByF(axis, s0*r));
        Vector3 tB1 = V3_AplusB(V3_ScaleByF(perp1, c1*r), V3_ScaleByF(axis, s1*r));
        AddDebugLine(V3_AplusB(cap.tip, tA0), V3_AplusB(cap.tip, tA1),textColors[TEXT_GREEN]);
        AddDebugLine(V3_AplusB(cap.tip, tB0), V3_AplusB(cap.tip, tB1),textColors[TEXT_GREEN]);
    }
    #undef HEMI_SEGS
    for (int seg = 0; seg < 4; seg++) {
        float a = seg * (6.28318530f / 4.0f);
        Vector3 off = V3_AplusB(V3_ScaleByF(perp0, vcosf(a)*r), V3_ScaleByF(perp1, vsinf(a)*r));
        AddDebugLine(V3_AplusB(cap.base, off), V3_AplusB(cap.tip, off),textColors[TEXT_GREEN]);
    }
    #undef CAPS_SEGS
}

static u16 cellLists[WORLDX*WORLDX][128];
static u16 cellCounts[WORLDX*WORLDX];
float GetCollisionRadius(Entity *e) { return (e->collider == COLLIDER_TYPE_BOX) ? vmax(e->colliderSize.x,vmax(e->colliderSize.y,e->colliderSize.z)) : e->colliderSize.x; }
Quaternion quat_from_axis_angle(Vector3 axis, float angle) { float half = angle * 0.5f; float s = vsinf(half); return (Quaternion){axis.x * s,axis.y * s,axis.z * s,vcosf(half)}; }
Quaternion quat_normalize(Quaternion q) {
    float len2 = q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w;
    if (len2 < COLLISION_EPSILON) return (Quaternion){0,0,0,1};
    float inv = 1.0f / vsqrtf(len2); q.x *= inv; q.y *= inv; q.z *= inv; q.w *= inv; return q;
}

static void ApplyVelocity(Entity *e, float dt) {
    Vector3 acc = {0, -9.81f * e->gravity, 0};
    acc = V3_AplusB(acc,V3_ScaleByF(e->accumulatedForce, 1.0f / e->mass));
    e->position = V3_AplusB(e->position,V3_AplusB(V3_ScaleByF(e->velocity,dt),V3_ScaleByF(acc,0.5f * dt * dt))); // Semi-Implicit Euler
    e->velocity = V3_AplusB(e->velocity, V3_ScaleByF(acc,dt));
    float speed = V3_Mag(e->velocity);
    if (speed > MAX_SPEED) e->velocity = V3_ScaleByF(V3_ScaleByF(e->velocity,1.0f / speed),MAX_SPEED);
    float dragFactor = vexp(-e->angularDrag * dt);
    e->angularVelocity = V3_ScaleByF(e->angularVelocity,dragFactor);
    float avel = V3_Mag(e->angularVelocity);
    if (avel > MAX_ANGULAR_SPEED) { e->angularVelocity = V3_ScaleByF(V3_ScaleByF(e->angularVelocity, 1.0f / avel),MAX_ANGULAR_SPEED); avel = MAX_ANGULAR_SPEED; }    
    if (avel > COLLISION_EPSILON) { Quaternion dq = quat_from_axis_angle(V3_ScaleByF(e->angularVelocity, 1.0f/avel),avel * dt); e->rotation = quat_normalize(quat_multiply(dq,e->rotation)); }
}

static OverlapResult BoxBoxSAT(ShapeBox a, ShapeBox b) {
    OverlapResult r = {0}; 
    r.overlapAmount = 0.0f;
    Vector3 aAxes[3], bAxes[3];
    obb_axes(a.rot, &aAxes[0], &aAxes[1], &aAxes[2]);
    obb_axes(b.rot, &bAxes[0], &bAxes[1], &bAxes[2]);
    Vector3 T = V3_AsubB(b.center, a.center); // From A to B

    // Precompute absolute rotation matrix components to avoid recalculations
    float R[3][3], AbsR[3][3];
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            R[i][j] = V3_dot(aAxes[i], bAxes[j]);
            AbsR[i][j] = vabs(R[i][j]) + 1e-6f; // Add epsilon to handle parallel edges safely
        }
    }

    float ra, rb;
    float overlap;
    float minOverlap = 1e9f;
    int bestAxisIndex = -1;
    bool flipNormal = false;

    // 1. Test Face Axes of Box A
    for (int i = 0; i < 3; i++) {
        ra = ((float*)&a.halfExtents)[i];
        rb = b.halfExtents.x * AbsR[i][0] + b.halfExtents.y * AbsR[i][1] + b.halfExtents.z * AbsR[i][2];
        float t = vabs(V3_dot(T, aAxes[i]));
        if (t > ra + rb) return r; // Separating axis found

        overlap = (ra + rb) - t;
        if (overlap < minOverlap) { minOverlap = overlap; bestAxisIndex = i; flipNormal = (V3_dot(T, aAxes[i]) < 0.0f); }
    }

    // 2. Test Face Axes of Box B
    for (int i = 0; i < 3; i++) {
        ra = a.halfExtents.x * AbsR[0][i] + a.halfExtents.y * AbsR[1][i] + a.halfExtents.z * AbsR[2][i];
        rb = ((float*)&b.halfExtents)[i];
        float t = vabs(V3_dot(T, bAxes[i]));
        if (t > ra + rb) return r; // Separating axis found

        overlap = (ra + rb) - t;
        if (overlap < minOverlap) { minOverlap = overlap; bestAxisIndex = 3 + i; flipNormal = (V3_dot(T, bAxes[i]) < 0.0f); }
    }

    // 3. Test 9 Edge-Edge Cross Products
    // We use the cross products of Box A's axes with Box B's axes.
    // To avoid expensive cross products, we can use the R and AbsR matrices directly.
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            // Indices for the other two dimensions
            int i1 = (i + 1) % 3;
            int i2 = (i + 2) % 3;
            int j1 = (j + 1) % 3;
            int j2 = (j + 2) % 3;

            // Compute projection of the translation vector onto the cross product axis
            float t = vabs(V3_dot(T, aAxes[i2]) * R[i1][j] - V3_dot(T, aAxes[i1]) * R[i2][j]);

            // Compute projection radiuses ra and rb
            ra = ((float*)&a.halfExtents)[i1] * AbsR[i2][j] + ((float*)&a.halfExtents)[i2] * AbsR[i1][j];
            rb = ((float*)&b.halfExtents)[j1] * AbsR[i][j2] + ((float*)&b.halfExtents)[j2] * AbsR[i][j1];

            // If separated along this axis, no collision
            if (t > ra + rb) return r;

            // Calculate overlap. We divide by the axis length if it's not a unit vector.
            // For edge-edge cross products, the axis length is sin(theta).
            // A precise production engine divides by the length to get the true penetration depth.
            float axisLengthSq = 1.0f - (R[i][j] * R[i][j]);
            if (axisLengthSq > 1e-4f) { // Skip if edges are parallel
                float axisLength = vsqrtf(axisLengthSq);
                overlap = ((ra + rb) - t) / axisLength;
                if (overlap < minOverlap) {
                    minOverlap = overlap;
                    bestAxisIndex = 6 + i * 3 + j;
                    
                    // Generate the axis vector dynamically for normal calculation later
                    Vector3 edgeAxis = V3_Cross(aAxes[i], bAxes[j]);
                    flipNormal = (V3_dot(T, edgeAxis) < 0.0f);
                }
            }
        }
    }

    // Construct Result
    r.hit = true;
    r.overlapAmount = minOverlap;

    // Resolve Normal pointing from B to A
    if (bestAxisIndex < 3) {
        r.normal = flipNormal ? aAxes[bestAxisIndex] : V3_ScaleByF(aAxes[bestAxisIndex], -1.0f);
    } else {
        r.normal = flipNormal ? bAxes[bestAxisIndex - 3] : V3_ScaleByF(bAxes[bestAxisIndex - 3], -1.0f);
    }

    // Accurate Contact Point: Deepest point of A inside B
    // Get support point of Box A in the direction of the normal mapping into Box B
    Vector3 supportA = a.center;
    supportA = V3_AplusB(supportA, V3_ScaleByF(aAxes[0], (V3_dot(aAxes[0], r.normal) < 0.0f ? 1.f : -1.f) * a.halfExtents.x));
    supportA = V3_AplusB(supportA, V3_ScaleByF(aAxes[1], (V3_dot(aAxes[1], r.normal) < 0.0f ? 1.f : -1.f) * a.halfExtents.y));
    supportA = V3_AplusB(supportA, V3_ScaleByF(aAxes[2], (V3_dot(aAxes[2], r.normal) < 0.0f ? 1.f : -1.f) * a.halfExtents.z));
    r.point = V3_AplusB(supportA, V3_ScaleByF(r.normal, minOverlap * 0.5f));
    return r;
}

static Vector3 TransformVertex(float* M, Vector3 local) {
    float m00=M[0],m10=M[1],m20=M[2], m01=M[4],m11=M[5],m21=M[6],
          m02=M[8],m12=M[9],m22=M[10], tx=M[12],ty=M[13],tz=M[14];
    return (Vector3){
        m00*local.x + m01*local.y + m02*local.z + tx,
        m10*local.x + m11*local.y + m12*local.z + ty,
        m20*local.x + m21*local.y + m22*local.z + tz
    };
}

static Vector3 MeshSupport(u16 meshIdx, float* M, Vector3 dir) {
    if (meshIdx >= MODEL_IDX_MAX) return (Vector3){0,0,0};
    u32 vCount = modelVertexCounts[meshIdx]; if (vCount == 0) return (Vector3){0,0,0};
    Vector3 best = {0}; float maxDot = -1e9f;
    for (u32 i = 0; i < vCount; ++i) {
        u32 b = (u32)i * VERTEX_ATTRIBUTES_SIZE;
        Vector3 v = { half_to_float(*(half*)(modelVertices[meshIdx] + b + 0)), half_to_float(*(half*)(modelVertices[meshIdx] + b + 2)), half_to_float(*(half*)(modelVertices[meshIdx] + b + 4)) };
        Vector3 w = TransformVertex(M, v);
        float d = V3_dot(w, dir);
        if (d > maxDot) { maxDot = d; best = w; }
    }
    return best;
}

// ---- GJK + EPA (3D, proper simplex management) ----
typedef struct { Vector3 verts[4]; int count; } Simplex3D;

static Vector3 MinkowskiSupport(u16 meshA, float* matA, u16 meshB, float* matB, Vector3 dir) {
    return V3_AsubB(MeshSupport(meshA, matA, dir), MeshSupport(meshB, matB, V3_ScaleByF(dir, -1.0f)));
}

// Returns true and updates simplex + dir if we should keep searching; false if we passed origin.
// Implements the full 3D GJK nearest-feature case analysis.
static bool GJKNextSimplex(Simplex3D *s, Vector3 *dir) {
    // References: Erin Catto GDC 2010, dyn4j GJK implementation
    Vector3 A = s->verts[s->count-1]; // Most recently added point
    Vector3 AO = V3_ScaleByF(A, -1.0f); // Direction toward origin from A

    if (s->count == 2) {
        Vector3 B = s->verts[0];
        Vector3 AB = V3_AsubB(B, A);
        // Origin along AB?
        if (V3_dot(AB, AO) > 0.0f) *dir = V3_AsubB(V3_AsubB(AB, V3_ScaleByF(AO, V3_dot(AB,AO)/V3_dot(AO,AO))), AB); // triple product (AB x AO) x AB
        else { s->verts[0] = A; s->count = 1; *dir = AO; }
        // Proper triple product: (AB x AO) x AB
        *dir = V3_Cross(V3_Cross(AB, AO), AB);
        if (V3_Mag(*dir) < COLLISION_EPSILON) { // AB parallel to AO, pick perpendicular
            Vector3 perp = (vabs(AB.x) > 0.9f) ? (Vector3){0,1,0} : (Vector3){1,0,0};
            *dir = V3_Cross(AB, perp);
        }
        return true;
    }

    if (s->count == 3) {
        Vector3 B = s->verts[1], C = s->verts[0];
        Vector3 AB = V3_AsubB(B, A), AC = V3_AsubB(C, A);
        Vector3 ABC = V3_Cross(AB, AC); // Triangle normal
        Vector3 ABperp = V3_Cross(AB, ABC), ACperp = V3_Cross(ABC, AC);
        if (V3_dot(ACperp, AO) > 0.0f) {
            if (V3_dot(AC, AO) > 0.0f) { s->verts[0]=C; s->verts[1]=A; s->count=2; *dir=V3_Cross(V3_Cross(AC,AO),AC); }
            else if (V3_dot(AB, AO) > 0.0f) { s->verts[0]=B; s->verts[1]=A; s->count=2; *dir=V3_Cross(V3_Cross(AB,AO),AB); }
            else { s->verts[0]=A; s->count=1; *dir=AO; }
        } else if (V3_dot(ABperp, AO) > 0.0f) {
            if (V3_dot(AB, AO) > 0.0f) { s->verts[0]=B; s->verts[1]=A; s->count=2; *dir=V3_Cross(V3_Cross(AB,AO),AB); }
            else { s->verts[0]=A; s->count=1; *dir=AO; }
        } else {
            // Origin above or below triangle
            if (V3_dot(ABC, AO) > 0.0f) *dir = ABC;
            else { Vector3 tmp=s->verts[0]; s->verts[0]=s->verts[1]; s->verts[1]=tmp; *dir=V3_ScaleByF(ABC,-1.0f); } // Flip winding
        }
        return true;
    }

    if (s->count == 4) {
        // Tetrahedron — check if origin is inside all 4 faces
        Vector3 B=s->verts[2], C=s->verts[1], D=s->verts[0];
        Vector3 AB=V3_AsubB(B,A), AC=V3_AsubB(C,A), AD=V3_AsubB(D,A);
        Vector3 nABC=V3_Cross(AB,AC), nACD=V3_Cross(AC,AD), nADB=V3_Cross(AD,AB);
        // Ensure normals point outward from tetrahedron
        if (V3_dot(nABC, AD) > 0.0f) nABC=V3_ScaleByF(nABC,-1.0f);
        if (V3_dot(nACD, AB) > 0.0f) nACD=V3_ScaleByF(nACD,-1.0f);
        if (V3_dot(nADB, AC) > 0.0f) nADB=V3_ScaleByF(nADB,-1.0f);
        if (V3_dot(nABC,AO)>0.0f) { s->verts[0]=C; s->verts[1]=B; s->verts[2]=A; s->count=3; *dir=nABC; return true; }
        if (V3_dot(nACD,AO)>0.0f) { s->verts[0]=D; s->verts[1]=C; s->verts[2]=A; s->count=3; *dir=nACD; return true; }
        if (V3_dot(nADB,AO)>0.0f) { s->verts[0]=B; s->verts[1]=D; s->verts[2]=A; s->count=3; *dir=nADB; return true; }
        return false; // Origin inside tetrahedron — intersection confirmed
    }
    return true;
}

// EPA polytope face
typedef struct { int a,b,c; Vector3 normal; float dist; } EPAFace;
#define EPA_MAX_FACES  64
#define EPA_MAX_VERTS  32

static OverlapResult ConvexMeshOverlap(u16 meshA, u16 meshB, float* matA, float* matB) {
    OverlapResult r = {0};
    if (meshA >= MODEL_IDX_MAX || meshB >= MODEL_IDX_MAX) return r;

    // --- GJK phase ---
    Simplex3D s = {0};
    Vector3 dir = {0,1,0};
    s.verts[0] = MinkowskiSupport(meshA,matA,meshB,matB,dir); s.count=1;
    dir = V3_ScaleByF(s.verts[0], -1.0f);
    if (V3_Mag(dir) < COLLISION_EPSILON) dir = (Vector3){0,1,0};

    for (int iter=0; iter<64; ++iter) {
        Vector3 support = MinkowskiSupport(meshA,matA,meshB,matB,dir);
        if (V3_dot(support,dir) < COLLISION_EPSILON) return r; // No intersection
        s.verts[s.count++] = support;
        if (!GJKNextSimplex(&s, &dir)) { r.hit=true; break; }
        if (V3_Mag(dir) < COLLISION_EPSILON) { r.hit=true; break; }
    }
    if (!r.hit) return r;

    // Ensure we have a tetrahedron for EPA; if simplex is degenerate, expand it
    while (s.count < 4) {
        // Pick axis not parallel to existing simplex
        Vector3 expand_dirs[6] = {{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
        bool added = false;
        for (int d=0;d<6&&!added;d++) {
            Vector3 sup = MinkowskiSupport(meshA,matA,meshB,matB,expand_dirs[d]);
            // Check if not already in simplex
            bool dup = false;
            for (int k=0;k<s.count;k++) { Vector3 diff=V3_AsubB(sup,s.verts[k]); if(V3_dot(diff,diff)<COLLISION_EPSILON*COLLISION_EPSILON){dup=true;break;} }
            if (!dup) { s.verts[s.count++]=sup; added=true; }
        }
        if (!added) break;
    }
    if (s.count < 4) { r.hit=true; return r; } // Degenerate — return hit without depth

    // --- EPA phase: expand polytope to find penetration depth ---
    Vector3 epaVerts[EPA_MAX_VERTS];
    EPAFace epFaces[EPA_MAX_FACES];
    int nVerts=0, nFaces=0;

    // Seed polytope from tetrahedron, ensure consistent outward winding
    for (int i=0;i<4&&nVerts<EPA_MAX_VERTS;i++) epaVerts[nVerts++]=s.verts[i];
    // 4 faces of tetrahedron (indices into epaVerts): ensure normal points outward
    int tetFaces[4][3] = {{0,1,2},{0,3,1},{0,2,3},{1,3,2}};
    for (int f=0;f<4&&nFaces<EPA_MAX_FACES;f++) {
        int ia=tetFaces[f][0], ib=tetFaces[f][1], ic=tetFaces[f][2];
        Vector3 n = V3_Cross(V3_AsubB(epaVerts[ib],epaVerts[ia]), V3_AsubB(epaVerts[ic],epaVerts[ia]));
        float nLen = V3_Mag(n); if(nLen<COLLISION_EPSILON) continue;
        n = V3_ScaleByF(n, 1.0f/nLen);
        float d = V3_dot(n, epaVerts[ia]);
        if (d < 0.0f) { // Normal points inward — flip winding
            int tmp=ib; ib=ic; ic=tmp; n=V3_ScaleByF(n,-1.0f); d=-d;
        }
        epFaces[nFaces++] = (EPAFace){ia,ib,ic,n,d};
    }

    for (int iter=0;iter<32;++iter) {
        // Find face closest to origin
        int bestFace=-1; float bestDist=1e9f;
        for (int f=0;f<nFaces;f++) if(epFaces[f].dist < bestDist){bestDist=epFaces[f].dist; bestFace=f;}
        if (bestFace<0) break;

        Vector3 bestNormal = epFaces[bestFace].normal;
        Vector3 support = MinkowskiSupport(meshA,matA,meshB,matB,bestNormal);
        float sUpDot = V3_dot(bestNormal, support);

        if (sUpDot - bestDist < COLLISION_EPSILON) {
            // Converged
            r.normal = bestNormal;
            r.point  = V3_ScaleByF(bestNormal, bestDist);
            r.hit    = true;
            return r;
        }

        if (nVerts >= EPA_MAX_VERTS) break; // Polytope full
        int newVert = nVerts; epaVerts[nVerts++] = support;

        // Remove faces visible from support, collect silhouette edges
        // Edge represented as (a,b); silhouette = edges seen exactly once
        int edges[EPA_MAX_FACES*3][2]; int edgeCount=0;
        int newFaces[EPA_MAX_FACES]; int newFaceCount=0;
        for (int f=0;f<nFaces;f++) {
            if (V3_dot(epFaces[f].normal, V3_AsubB(support, epaVerts[epFaces[f].a])) > 0.0f) {
                // Visible — add its edges to silhouette candidate list
                int fverts[3] = {epFaces[f].a, epFaces[f].b, epFaces[f].c};
                for (int e=0;e<3;e++) {
                    int ea=fverts[e], eb=fverts[(e+1)%3];
                    // Check if reverse edge already in list (shared edge — remove both)
                    bool found=false;
                    for (int k=0;k<edgeCount;k++) {
                        if(edges[k][0]==eb && edges[k][1]==ea){ // Remove it by swapping with last
                            edges[k][0]=edges[edgeCount-1][0]; edges[k][1]=edges[edgeCount-1][1]; edgeCount--; found=true; break;
                        }
                    }
                    if (!found && edgeCount < EPA_MAX_FACES*3) { edges[edgeCount][0]=ea; edges[edgeCount][1]=eb; edgeCount++; }
                }
            } else {
                newFaces[newFaceCount++]=f; // Keep this face
            }
        }
        // Rebuild face list: keep non-visible faces, add new faces from silhouette
        nFaces=0;
        for (int k=0;k<newFaceCount;k++) epFaces[nFaces++]=epFaces[newFaces[k]];
        for (int k=0;k<edgeCount&&nFaces<EPA_MAX_FACES;k++) {
            int ia=edges[k][0], ib=edges[k][1], ic=newVert;
            Vector3 n = V3_Cross(V3_AsubB(epaVerts[ib],epaVerts[ia]), V3_AsubB(epaVerts[ic],epaVerts[ia]));
            float nLen=V3_Mag(n); if(nLen<COLLISION_EPSILON) continue;
            n=V3_ScaleByF(n,1.0f/nLen);
            float d=V3_dot(n,epaVerts[ia]);
            if(d<0.0f){n=V3_ScaleByF(n,-1.0f);d=-d;}
            epFaces[nFaces++]=(EPAFace){ia,ib,ic,n,d};
        }
    }

    r.hit=true; // Hit confirmed by GJK even if EPA didn't fully converge
    return r;
}

static void ApplyCollisionResponse(Entity *e, Entity *o, Vector3 n, float pen) {
    if (pen < COLLISION_EPSILON) return;

    bool oStatic = (!(o->entflags & ENTFLAG_RIGIDBODY) || (o->mass < 0.001f) || (o->collider == COLLIDER_TYPE_NONE));
    Vector3 relVel = oStatic ? e->velocity : V3_AsubB(e->velocity, o->velocity);
    float vn = V3_dot(relVel,n); if (vn > 0.0f) return; // Already separating

    float e_r = vmax(e->bounciness, oStatic ? 0.0f : o->bounciness) * 0.5f;
    float invMassA = 1.0f / e->mass; float invMassB = oStatic ? 0.0f : 1.0f / o->mass; float invMassSum = invMassA + invMassB;
    if (invMassSum < COLLISION_EPSILON) return;

    float j = -(1.0f + e_r) * vn / invMassSum;
    
    // Normal impulse    
    e->velocity = V3_AplusB(e->velocity, V3_ScaleByF(n, j * invMassA));
    if (!oStatic) o->velocity = V3_AsubB(o->velocity, V3_ScaleByF(n, j * invMassB));

    // Friction
    relVel = oStatic ? e->velocity : V3_AsubB(e->velocity, o->velocity);
    Vector3 tangent = V3_AsubB(relVel, V3_ScaleByF(n, V3_dot(relVel, n)));
    float tLen = V3_Mag(tangent);
    if (tLen > 0.001f) {
        tangent = V3_ScaleByF(tangent, 1.0f/tLen);
        float mu = (e->dynamicFriction + (oStatic ? 0.4f : o->dynamicFriction)) * 0.5f;
        float jt = -V3_dot(relVel, tangent) / invMassSum;
        jt = vclamp(jt, -mu * j, mu * j);

        e->velocity = V3_AplusB(e->velocity, V3_ScaleByF(tangent, jt * invMassA));
        if (!oStatic) o->velocity = V3_AsubB(o->velocity, V3_ScaleByF(tangent, jt * invMassB));
    }

    // Positional correction - ONLY move dynamic objects
    float correction = vmax(pen - 0.001f, 0.0f) * 0.4f;
    e->position = V3_AplusB(e->position, V3_ScaleByF(n, correction * invMassA / invMassSum));
    if (!oStatic) o->position = V3_AsubB(o->position, V3_ScaleByF(n, correction * invMassB / invMassSum));
}

void InitPhysics(void) {} // Not needed at the moment.
void Physics(void) {
    if (Sys_Global.gamePaused || Sys_Global.menuActive) return;

    float dt = vclamp((float)(Sys_Global.pauseRelativeTime - Sys_Global.last_physics_time), 0.0005f, 0.1f);
    Sys_Global.last_physics_time = Sys_Global.pauseRelativeTime;
    u8 substeps = (u8)vclamp((u32)(dt / MAX_STEP_SIZE + 0.5f), 1u, (u32)MAX_SUBSTEPS);
    float dtsub = dt / (float)substeps;
    dynamicEntityCount = 0;
    for (int i = 0; i < Sys_Global.loadedInstances && dynamicEntityCount < 512; ++i) { Entity* e = &Sys_Global.instances[i]; if (e->entflags & ENTFLAG_RIGIDBODY && e->entflags & ENTFLAG_ACTIVE) dynamicEntities[dynamicEntityCount++] = i; }  // Collect dynamic entities
    for (int i = 0; i < Sys_Global.loadedInstances; ++i) { // Update radii and initial cells
        Entity* e = &Sys_Global.instances[i]; e->cellX = (i16)PosGetCellCoordX(e->position.x); e->cellZ = (i16)PosGetCellCoordZ(e->position.z); e->cellIndex = PosGetCellCoordsP(e->cellX, e->cellZ);
        e->radius = (e->modelIndex < MODEL_IDX_MAX) ? modelBounds[e->modelIndex] * vmax(vmax(e->scale.x, e->scale.y), e->scale.z) : (e->collider == COLLIDER_TYPE_BOX ? vmax(e->colliderSize.x, vmax(e->colliderSize.y, e->colliderSize.z)) : e->colliderSize.x);
    }
    for (u8 s = 0; s < substeps; ++s) {
        MemSetToVForNBytes(cellCounts,0,sizeof(cellCounts)); // Build broadphase grid
        for (int i = 0; i < Sys_Global.loadedInstances; ++i) { Entity* e = &Sys_Global.instances[i]; u32 cell = (u32)e->cellIndex; if (cell < WORLDX*WORLDX && cellCounts[cell] < 127) cellLists[cell][cellCounts[cell]++] = i; }
        for (u16 i = 0; i < dynamicEntityCount; ++i) { u16 idx = dynamicEntities[i]; Entity *e = &Sys_Global.instances[idx]; ApplyVelocity(e, dtsub); } // Integrate all dynamic bodies
        for (u16 i = 0; i < dynamicEntityCount; ++i) { // Collision resolutions
            u16 idx = dynamicEntities[i]; Entity *e = &Sys_Global.instances[idx]; if (e->collider == COLLIDER_TYPE_NONE || (Sys_Cheats.noclip && idx == PLAYER1)) continue;

            i32 cx = PosGetCellCoordX(e->position.x), cz = PosGetCellCoordZ(e->position.z); 
            float searchRad = e->radius + V3_Mag(e->velocity) * dtsub + 0.5f;
            i32 radCells = (i32)(searchRad / CELL_SIZE) + 2; u32 mask = GetCollisionMask(e->layer);
            OverlapResult largestO = {0}; float largestOverlap = 0.0f; u16 deepestOther = 0xFFFF;
            for (i32 dx = -radCells; dx <= radCells; ++dx) {
                for (i32 dz = -radCells; dz <= radCells; ++dz) {
                    u32 cell = PosGetCellCoordsP(cx + dx, cz + dz); if (cell >= WORLDX*WORLDX) continue;
                    for (u16 k = 0; k < cellCounts[cell]; ++k) {
                        u16 j = cellLists[cell][k]; if (j == idx) continue;
                        Entity *o = &Sys_Global.instances[j]; if (!(mask & o->layer)) continue;

                        OverlapResult r = {0};
                        if (e->collider == COLLIDER_TYPE_CAPSULE && o->collider == COLLIDER_TYPE_CAPSULE) { ShapeCapsule ca,cb; Entity_GetCapsule(e,&ca); Entity_GetCapsule(o,&cb); r = CapsuleCapsule(ca,cb); } 
                        else if (e->collider == COLLIDER_TYPE_CAPSULE && o->collider == COLLIDER_TYPE_BOX) { ShapeCapsule ca; ShapeBox bb; Entity_GetCapsule(e,&ca); Entity_GetBox(o,&bb); r = CapsuleBox(ca,bb); }
                        else if (e->collider == COLLIDER_TYPE_BOX && o->collider == COLLIDER_TYPE_CAPSULE) { ShapeCapsule ca; ShapeBox bb; Entity_GetCapsule(o,&ca); Entity_GetBox(e,&bb); r = CapsuleBox(ca,bb); } 
                        else if (e->collider == COLLIDER_TYPE_BOX && o->collider == COLLIDER_TYPE_BOX) { ShapeBox ba,bb; Entity_GetBox(e,&ba); Entity_GetBox(o,&bb); r = BoxBoxSAT(ba,bb); }
                        else if (e->collider == COLLIDER_TYPE_SPHERE && o->collider == COLLIDER_TYPE_BOX) { ShapeSphere s; ShapeBox b; Entity_GetSphere(e,&s); Entity_GetBox(o,&b); r = SphereOBB(s.center,s.radius,b); }
                        else if (e->collider == COLLIDER_TYPE_BOX && o->collider == COLLIDER_TYPE_SPHERE) { ShapeSphere s; ShapeBox b; Entity_GetSphere(o,&s); Entity_GetBox(e,&b); r = SphereOBB(s.center,s.radius,b); }
                        else if (e->collider == COLLIDER_TYPE_SPHERE && o->collider == COLLIDER_TYPE_SPHERE) { ShapeSphere sa, sb; Entity_GetSphere(e,&sa); Entity_GetSphere(o,&sb); r = SphereSphere(sa.center,sa.radius,sb.center,sb.radius); }
                        else if ((e->collider == COLLIDER_TYPE_CONVEXMESH || e->collider == COLLIDER_TYPE_MESH) && (o->collider == COLLIDER_TYPE_CONVEXMESH || o->collider == COLLIDER_TYPE_MESH)) {
                            float matA[16],matB[16];
                            CopyMemoryFromBtoAForNBytes(matA,&modelMatrices[idx*16], 64);
                            CopyMemoryFromBtoAForNBytes(matB,&modelMatrices[j*16], 64);
                            u16 mA = (e->collider == COLLIDER_TYPE_MESH) ? e->modelIndex : e->colliderMeshIndex;
                            u16 mB = (o->collider == COLLIDER_TYPE_MESH) ? o->modelIndex : o->colliderMeshIndex;
                            r = ConvexMeshOverlap(mA, mB, matA, matB);
                        }
                        else if ((e->collider == COLLIDER_TYPE_SPHERE || e->collider == COLLIDER_TYPE_CAPSULE) && (o->collider == COLLIDER_TYPE_CONVEXMESH || o->collider == COLLIDER_TYPE_MESH)) { u16 mIdx = (o->collider == COLLIDER_TYPE_MESH) ? o->modelIndex : o->colliderMeshIndex; float rad = modelBounds[mIdx]; r = SphereSphere(e->position,GetCollisionRadius(e),o->position,rad); } 
                        else if ((e->collider == COLLIDER_TYPE_CONVEXMESH || e->collider == COLLIDER_TYPE_MESH) && (o->collider == COLLIDER_TYPE_SPHERE || o->collider == COLLIDER_TYPE_CAPSULE)) { u16 mIdx = (e->collider == COLLIDER_TYPE_MESH) ? e->modelIndex : e->colliderMeshIndex; float rad = modelBounds[mIdx]; r = SphereSphere(e->position,rad,o->position,GetCollisionRadius(o)); } 
                        else { r = SphereSphere(e->position,GetCollisionRadius(e),o->position,GetCollisionRadius(o)); }
                        if (r.hit && r.overlapAmount > largestOverlap) {
                            largestOverlap = r.overlapAmount;
                            largestO = r;
                            deepestOther = j;
                        }
                    }
                }
            }

            if (largestO.hit && largestOverlap > COLLISION_EPSILON) {
                Entity *o = (deepestOther < INSTANCE_COUNT) ? &Sys_Global.instances[deepestOther] : NULL;
                if (o && (o->entflags & ENTFLAG_RIGIDBODY)) ApplyCollisionResponse(e, o, largestO.normal, largestOverlap);
                else {
                    Entity staticProxy = {0};
                    staticProxy.mass = 0.0f; staticProxy.dynamicFriction = 0.4f; staticProxy.collider = COLLIDER_TYPE_NONE;
                    ApplyCollisionResponse(e,&staticProxy,largestO.normal,largestOverlap);
                }
            }

            e->accumulatedForce = (Vector3){0,0,0};
        }

        for (int i = 0; i < Sys_Global.loadedInstances; ++i) { Entity* e = &Sys_Global.instances[i]; e->cellX = (i16)PosGetCellCoordX(e->position.x); e->cellZ = (i16)PosGetCellCoordZ(e->position.z); e->cellIndex = PosGetCellCoordsP(e->cellX, e->cellZ); } // Update cells for next substep
    }
}

ENGINE_TO_MOD void AddForce(u16 idx, Vector3 force, bool impulse) {
    if (idx >= INSTANCE_COUNT) return;
    Entity *e = &Sys_Global.instances[idx]; float mass = e->mass > 0.001f ? e->mass : 1.0f;
    if (impulse) e->velocity         = V3_AplusB(e->velocity, V3_ScaleByF(force, 1.0f / mass));
    else         e->accumulatedForce = V3_AplusB(e->accumulatedForce, force);
}

ENGINE_TO_MOD void ApplyPlayerMovements(void) {
    Entity *p = &Sys_Global.instances[PLAYER1];
    float h = (float)Forward() - (float)Backpedal(), s = (float)StrafeRight() - (float)StrafeLeft();
    Vector3 input = V3_Normalize((Vector3){p->forward.x*h + p->right.x*s, (float)SwimUp() - (float)SwimDn(), p->forward.z*h + p->right.z*s});
    float speed = GetBasePlayerSpeed(PLAYER1, V3_Mag(input) > 0.1f) * 1.75f, accel = Sys_Global.boosterActive ? 1.0f : 3.0f;
    Vector3 cur = p->velocity; Vector3 dv = V3_AsubB(V3_ScaleByF(input,speed),cur);
    dv.x = vclamp(dv.x,-10,10); dv.y = vclamp(dv.y,-10,10); dv.z = vclamp(dv.z,-10,10);
    p->velocity = V3_AplusB(cur, V3_ScaleByF(dv, accel * vclamp((float)Sys_Global.timeSinceLastPhysicsTick,0.0005f,0.1f)));
}
