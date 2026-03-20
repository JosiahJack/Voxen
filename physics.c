// physics.cpp - Full Jolt Physics integration for Voxen
#include "voxen.h"
#define GROUND_PROBE_DIST 0.02f
#define SNAP_STEP 0.005f
#define SNAP_MAX  0.25f
#define SLOPE_WALK_MAX_DEG   45.0f
#define SLOPE_CLIMB_MAX_DEG  55.0f
#define SLOPE_SLIDE_ACCEL        8.0f
#define SLOPE_SLIDE_ACCEL_BOOST 14.0f
#define SLOPE_FRICTION_ACCEL        1.0f
#define SLOPE_FRICTION_ACCEL_BOOST  0.3f
typedef struct {
    float    depth;        // penetration depth in world space (positive = overlap)
    Vector3  normal;       // contact normal pointing AWAY from geometry (toward player)
} CapsuleContact;
#define NO_CONTACT ((CapsuleContact){ .depth = -1.0f, .normal = {0,1,0} })

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

static inline Vector3 ClosestPointOnSegment(Vector3 p, Vector3 q, Vector3 a) {
    Vector3 pq  = Vector3_A_minus_B(q, p);
    Vector3 pa  = Vector3_A_minus_B(a, p);
    float   len2 = dot_vector3(pq, pq);
    if (len2 < 1e-10f) return p;                       // degenerate segment
    float   t    = dot_vector3(pa, pq) / len2;
    t = vmax(0.0f, vmin(1.0f, t));
    return Vector3_A_plus_B(p, scale_vector3(pq, t));
}

static inline Vector3 ClosestPointOnTriangle(Vector3 a, Vector3 b, Vector3 c, Vector3 p) {
    Vector3 ab = Vector3_A_minus_B(b, a);
    Vector3 ac = Vector3_A_minus_B(c, a);
    Vector3 ap = Vector3_A_minus_B(p, a);
    float d1 = dot_vector3(ab, ap);
    float d2 = dot_vector3(ac, ap);
    if (d1 <= 0.0f && d2 <= 0.0f) return a;            // vertex A region
 
    Vector3 bp = Vector3_A_minus_B(p, b);
    float d3 = dot_vector3(ab, bp);
    float d4 = dot_vector3(ac, bp);
    if (d3 >= 0.0f && d4 <= d3) return b;              // vertex B region
 
    Vector3 cp = Vector3_A_minus_B(p, c);
    float d5 = dot_vector3(ab, cp);
    float d6 = dot_vector3(ac, cp);
    if (d6 >= 0.0f && d5 <= d6) return c;              // vertex C region
 
    float vc = d1 * d4 - d3 * d2;
    if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f) {
        float v = d1 / (d1 - d3);
        return Vector3_A_plus_B(a, scale_vector3(ab, v)); // edge AB
    }
 
    float vb = d5 * d2 - d1 * d6;
    if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) {
        float w = d2 / (d2 - d6);
        return Vector3_A_plus_B(a, scale_vector3(ac, w)); // edge AC
    }
 
    float va = d3 * d6 - d5 * d4;
    if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f) {
        float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
        return Vector3_A_plus_B(b, scale_vector3(Vector3_A_minus_B(c, b), w)); // edge BC
    }
 
    // Interior of triangle
    float denom = 1.0f / (va + vb + vc);
    float v = vb * denom;
    float w = vc * denom;
    // = a + ab*v + ac*w
    Vector3 result = Vector3_A_plus_B(a, scale_vector3(ab, v));
    result = Vector3_A_plus_B(result, scale_vector3(ac, w));
    return result;
}

static CapsuleContact QueryCapsuleContact(Vector3 start, Vector3 end, float capsuleRadius, uint32_t layerMask) {
    CapsuleContact worst = NO_CONTACT;
 
    for (uint16_t i = START_INDEX_LEVEL_INSTANCES; i < INSTANCE_COUNT; ++i) {
        if (!(layerMask & Sys_Global.instances[i].layer)) continue;
        uint16_t mindex = Sys_Global.instances[i].modelIndex;
        if (mindex >= loadedModelsMaxIndex) continue;
        uint32_t triCount = modelTriangleCounts[mindex];
        if (triCount < 1) continue;
 
        float M[16];
        __builtin_memcpy(M, &modelMatrices[i * 16], 16 * sizeof(float));
        float m00=M[0], m10=M[1], m20=M[2];
        float m01=M[4], m11=M[5], m21=M[6];
        float m02=M[8], m12=M[9], m22=M[10];
        float tx=M[12], ty=M[13], tz=M[14];
        float scl_x = vsqrtf(m00*m00 + m10*m10 + m20*m20);
        float scl_y = vsqrtf(m01*m01 + m11*m11 + m21*m21);
        float scl_z = vsqrtf(m02*m02 + m12*m12 + m22*m22);
        if (scl_x < 1e-6f || scl_y < 1e-6f || scl_z < 1e-6f) continue;
 
        Vector3 objPos = Sys_Global.instances[i].position;
        Vector3 capsuleMid = { (start.x+end.x)*0.5f, (start.y+end.y)*0.5f, (start.z+end.z)*0.5f };
        Vector3 delta = Vector3_A_minus_B(objPos, capsuleMid);
        float distSqrd = delta.x*delta.x + delta.y*delta.y + delta.z*delta.z;
        float modelRad = vmax(modelBounds[(mindex*BOUNDS_ATTRIBUTES_COUNT)+BOUNDS_DATA_OFFSET_RADIUS], 1.81f);
        Vector3 spine = Vector3_A_minus_B(end, start);
        float spineHalf = magnitude_vector3(spine) * 0.5f;
        float combinedRad = modelRad + spineHalf + capsuleRadius + 0.1f;
        if (distSqrd > combinedRad*combinedRad) continue;
 
        Vector3 relS = { start.x-tx, start.y-ty, start.z-tz };
        Vector3 localStart = {
            (relS.x*m00 + relS.y*m10 + relS.z*m20) / (scl_x*scl_x),
            (relS.x*m01 + relS.y*m11 + relS.z*m21) / (scl_y*scl_y),
            (relS.x*m02 + relS.y*m12 + relS.z*m22) / (scl_z*scl_z)
        };
        Vector3 relE = { end.x-tx, end.y-ty, end.z-tz };
        Vector3 localEnd = {
            (relE.x*m00 + relE.y*m10 + relE.z*m20) / (scl_x*scl_x),
            (relE.x*m01 + relE.y*m11 + relE.z*m21) / (scl_y*scl_y),
            (relE.x*m02 + relE.y*m12 + relE.z*m22) / (scl_z*scl_z)
        };
        float minScl = scl_x;
        if (scl_y < minScl) minScl = scl_y;
        if (scl_z < minScl) minScl = scl_z;
        float localRadius = capsuleRadius / minScl;
 
        for (uint32_t j = 0; j < triCount; ++j) {
            uint32_t bA = modelTriangles[mindex][j*3  ] * VERTEX_ATTRIBUTES_COUNT;
            uint32_t bB = modelTriangles[mindex][j*3+1] * VERTEX_ATTRIBUTES_COUNT;
            uint32_t bC = modelTriangles[mindex][j*3+2] * VERTEX_ATTRIBUTES_COUNT;
            Vector3 posA = { modelVertices[mindex][bA], modelVertices[mindex][bA+1], modelVertices[mindex][bA+2] };
            Vector3 posB = { modelVertices[mindex][bB], modelVertices[mindex][bB+1], modelVertices[mindex][bB+2] };
            Vector3 posC = { modelVertices[mindex][bC], modelVertices[mindex][bC+1], modelVertices[mindex][bC+2] };
 
            // Closest point on triangle to spine, then closest point on spine to that.
            // This gives us the actual contact point and direction, winding-independent.
            Vector3 cpP   = ClosestPointOnTriangle(posA, posB, posC, localStart);
            Vector3 spP   = ClosestPointOnSegment(localStart, localEnd, cpP);
            Vector3 cpP2  = ClosestPointOnTriangle(posA, posB, posC, spP);
            Vector3 cpQ   = ClosestPointOnTriangle(posA, posB, posC, localEnd);
            Vector3 spQ   = ClosestPointOnSegment(localStart, localEnd, cpQ);
            Vector3 cpQ2  = ClosestPointOnTriangle(posA, posB, posC, spQ);
 
            // Pick whichever contact point has the smallest distance (deepest penetration).
            Vector3 dP = Vector3_A_minus_B(spP, cpP2);
            Vector3 dQ = Vector3_A_minus_B(spQ, cpQ2);
            float distP = vsqrtf(dot_vector3(dP, dP));
            float distQ = vsqrtf(dot_vector3(dQ, dQ));
            float localDist; Vector3 localContactVec;
            if (distP <= distQ) { localDist = distP; localContactVec = dP; }
            else                { localDist = distQ; localContactVec = dQ; }
 
            float localPen = localRadius - localDist;
            if (localPen <= 0.0f) continue; // no overlap
 
            // Contact normal in local space: direction from triangle surface toward spine point.
            // If the contact vector is near-zero (spine point IS on the triangle), use
            // the face normal but orient it toward the capsule centre.
            Vector3 localNormal;
            if (localDist > 1e-6f) {
                localNormal = (Vector3){ localContactVec.x / localDist, localContactVec.y / localDist, localContactVec.z / localDist };
            } else {
                // Degenerate: use face normal, pick orientation toward capsule mid
                Vector3 eAB = Vector3_A_minus_B(posB, posA);
                Vector3 eAC = Vector3_A_minus_B(posC, posA);
                localNormal = normalize_vector3(cross_vector3(eAB, eAC));
                Vector3 spMid = { (localStart.x+localEnd.x)*0.5f, (localStart.y+localEnd.y)*0.5f, (localStart.z+localEnd.z)*0.5f };
                Vector3 toMid = Vector3_A_minus_B(spMid, posA);
                if (dot_vector3(localNormal, toMid) < 0.0f) { localNormal.x=-localNormal.x; localNormal.y=-localNormal.y; localNormal.z=-localNormal.z; }
            }
 
            // Transform contact normal to world space (inverse-transpose = divide by scale, not scale²).
            Vector3 worldNormal = {
                (m00/scl_x)*localNormal.x + (m01/scl_y)*localNormal.y + (m02/scl_z)*localNormal.z,
                (m10/scl_x)*localNormal.x + (m11/scl_y)*localNormal.y + (m12/scl_z)*localNormal.z,
                (m20/scl_x)*localNormal.x + (m21/scl_y)*localNormal.y + (m22/scl_z)*localNormal.z
            };
            worldNormal = normalize_vector3(worldNormal);
 
            // Convert local penetration depth to approximate world-space depth.
            float worldPen = localPen * minScl;
 
            if (worldPen > worst.depth) { worst.depth = worldPen; worst.normal = worldNormal; }
        }
    }
    return worst;
}

bool CheckCapsule(Vector3 start, Vector3 end, float capsuleRadius, float capsuleHeight, uint32_t layerMask) {
    (void)capsuleHeight;
    return QueryCapsuleContact(start, end, capsuleRadius, layerMask).depth > 0.0f;
}

static inline void CapsuleTipsFromEye(Vector3 eye, Vector3 *start, Vector3 *end) {
    float innerSpine = PLAYER_HEIGHT - 2.0f * PLAYER_RADIUS;
    float centreY = eye.y - PLAYER_CAM_OFFSET_Y;
    *start = (Vector3){ eye.x, centreY - innerSpine * 0.5f, eye.z };
    *end   = (Vector3){ eye.x, centreY + innerSpine * 0.5f, eye.z };
}

static float SnapEyeAboveFloor(float eyeX, float eyeY, float eyeZ, uint32_t mask) {
    float corrected   = eyeY;
    float totalPushed = 0.0f;
    while (totalPushed < SNAP_MAX) {
        Vector3 s, e;
        CapsuleTipsFromEye((Vector3){ eyeX, corrected, eyeZ }, &s, &e);
        if (QueryCapsuleContact(s, e, PLAYER_RADIUS, mask).depth <= 0.0f) break;
        corrected   += SNAP_STEP;
        totalPushed += SNAP_STEP;
    }
    return corrected;
}

void BuildPlayerCapsule(uint16_t playerIdx, Vector3 *start, Vector3 *end) {
    Vector3 eye = Sys_Global.instances[playerIdx].position;
    float innerSpine = PLAYER_HEIGHT - 2.0f * PLAYER_RADIUS; // 1.04
    float centreY = eye.y - PLAYER_CAM_OFFSET_Y;             // eye - 0.84
    start->x = eye.x; start->y = centreY - innerSpine * 0.5f; start->z = eye.z;
    end->x   = eye.x; end->y   = centreY + innerSpine * 0.5f; end->z   = eye.z;
}

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
    float speed = GetBasePlayerSpeed(intent > 0.1f) * 1.75f; // Friction compensation term
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

const Vector3 gravityVelocity = { 0.0f, -9.81f * 2.5f, 0.0f };

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
    Vector3 pos = Sys_Global.instances[i].position;
    Sys_Global.instances[i].cellIndex = PosGetCellCoords(pos.x, pos.z);
    float mag = magnitude_vector3(Sys_Global.instances[i].velocity);
    if (i > PLAYER1) return;
    if (!(Sys_Global.instances[i].index != PLAYER1 || (Sys_Global.instances[i].entflags & ENTFLAG_RIGIDBODY))) return;
    if (mag < 0.05f) return;
 
    Vector3 vel = Sys_Global.instances[i].velocity;
    Vector3 dir = normalize_vector3(vel);
    float dt    = (float)Sys_Global.timeSinceLastPhysicsTick;
 
    Vector3 hitPos    = Vector3_A_plus_B(pos, scale_vector3(dir, PLAYER_RADIUS));
    Vector3 newHitPos = Vector3_A_plus_B(hitPos, scale_vector3(vel, dt));
 
    if (i <= PLAYER2 && Sys_Cheats.noclip) { Sys_Global.instances[i].position = Vector3_A_plus_B(pos, scale_vector3(vel, dt)); return; }
 
    // Grid cell pre-filter
    int32_t ccx = PosGetCellCoordX(pos.x),    ccz = PosGetCellCoordZ(pos.z);
    int32_t hpx = PosGetCellCoordX(hitPos.x), hpz = PosGetCellCoordZ(hitPos.z);
    if (hpx != ccx) ccx = hpx;
    if (hpz != ccz) ccz = hpz;
    int32_t ncx = PosGetCellCoordX(newHitPos.x), ncz = PosGetCellCoordZ(newHitPos.z);
    int32_t cc  = (ccz  * WORLDX) + ccx;
    int32_t nc  = (ncz  * WORLDX) + ncx;
    if (ncz > ccz && (gridCellStates[cc] & CELL_CLOSEDNORTH)) { Sys_Global.instances[i].velocity.z = -reboundVelocity; return; }
    if (ncz < ccz && (gridCellStates[cc] & CELL_CLOSEDSOUTH)) { Sys_Global.instances[i].velocity.z =  reboundVelocity; return; }
    if (ncx > ccx && (gridCellStates[cc] & CELL_CLOSEDEAST))  { Sys_Global.instances[i].velocity.x = -reboundVelocity; return; }
    if (ncx < ccx && (gridCellStates[cc] & CELL_CLOSEDWEST))  { Sys_Global.instances[i].velocity.x =  reboundVelocity; return; }
    if (!(gridCellStates[nc] & CELL_OPEN)) { Sys_Global.instances[i].velocity = scale_vector3(dir, -reboundVelocity); return; }
 
    uint32_t mask       = GetCollisionMask(Sys_Global.instances[i].layer);
    float    innerSpine = PLAYER_HEIGHT - 2.0f * PLAYER_RADIUS;
    bool     boosted    = Sys_Global.boosterActive;
 
    // Step 1: push out of any penetrated geometry
    {
        Vector3 s, e;
        CapsuleTipsFromEye(pos, &s, &e);
        CapsuleContact c = QueryCapsuleContact(s, e, PLAYER_RADIUS, mask);
        if (c.depth > 0.0f) {
            pos.x += c.normal.x * (c.depth + SNAP_STEP);
            pos.y += c.normal.y * (c.depth + SNAP_STEP);
            pos.z += c.normal.z * (c.depth + SNAP_STEP);
            CapsuleTipsFromEye(pos, &s, &e);
            if (QueryCapsuleContact(s, e, PLAYER_RADIUS, mask).depth > 0.0f)
                pos.y = SnapEyeAboveFloor(pos.x, pos.y, pos.z, mask);
            Sys_Global.instances[i].position = pos;
            vel = Sys_Global.instances[i].velocity;
        }
    }
 
    float   centreY  = pos.y - PLAYER_CAM_OFFSET_Y;
    Vector3 curStart = { pos.x, centreY - innerSpine*0.5f, pos.z };
    Vector3 curEnd   = { pos.x, centreY + innerSpine*0.5f, pos.z };
 
    // Step 2: ground probe — only snap if already moving downward or stationary,
    // NOT if vel.y > 0 (jumping) and NOT if the player just walked off an edge
    // with significant downward velocity already building (let gravity run).
    bool    isGrounded  = false;
    float   slopeDeg    = 0.0f;
    Vector3 floorNormal = { 0.0f, 1.0f, 0.0f };
 
    if (vel.y <= 0.05f) { // only probe when not actively moving upward
        // Probe just enough to bridge one gravity tick, not the full GROUND_PROBE_DIST magnet range.
        // One gravity tick at max dt (33ms): 9.81 * gravity_scale * 0.033 ~ 0.33; use 0.08 for snapping
        // to floor surface, keep GROUND_PROBE_DIST for the outer "is there a floor nearby" test only.
        float snapRange = vmin(GROUND_PROBE_DIST, vmax(0.08f, vabs(vel.y) * dt + 0.04f));
        Vector3 probeEye = { pos.x, pos.y - snapRange, pos.z };
        Vector3 pStart, pEnd;
        CapsuleTipsFromEye(probeEye, &pStart, &pEnd);
        CapsuleContact probe = QueryCapsuleContact(pStart, pEnd, PLAYER_RADIUS, mask);
        if (probe.depth > 0.0f && probe.normal.y > 0.1f) {
            floorNormal = probe.normal;
            isGrounded  = true;
            float cosA  = vmax(-1.0f, vmin(1.0f, floorNormal.y));
            slopeDeg    = (180.0f / 3.14159265f) * vacosf(cosA);
 
            // Walk eye down to exact floor contact
            float floorY = pos.y;
            for (float d = SNAP_STEP; d <= snapRange; d += SNAP_STEP) {
                Vector3 s, e;
                CapsuleTipsFromEye((Vector3){ pos.x, pos.y - d, pos.z }, &s, &e);
                if (QueryCapsuleContact(s, e, PLAYER_RADIUS, mask).depth > 0.0f) break;
                floorY = pos.y - d;
            }
            pos.y = floorY;
            Sys_Global.instances[i].position = pos;
            Sys_Global.instances[i].velocity.y = 0.0f;
            vel.y = 0.0f;
            centreY  = pos.y - PLAYER_CAM_OFFSET_Y;
            curStart = (Vector3){ pos.x, centreY - innerSpine*0.5f, pos.z };
            curEnd   = (Vector3){ pos.x, centreY + innerSpine*0.5f, pos.z };
        }
    }
 
    // Step 3: slope response
    if (isGrounded) {
        if (slopeDeg > SLOPE_CLIMB_MAX_DEG) {
            float vdn = dot_vector3(vel, floorNormal);
            if (vdn > 0.0f) { vel.x -= floorNormal.x * vdn; vel.y -= floorNormal.y * vdn; vel.z -= floorNormal.z * vdn; }
            float gdn = -9.81f * floorNormal.y;
            float accel = boosted ? SLOPE_SLIDE_ACCEL_BOOST : SLOPE_SLIDE_ACCEL;
            Vector3 slide = { -floorNormal.x * gdn, -9.81f - floorNormal.y * gdn, -floorNormal.z * gdn };
            float slen = vsqrtf(slide.x*slide.x + slide.y*slide.y + slide.z*slide.z);
            if (slen > 1e-4f) { vel.x += (slide.x/slen)*accel*dt; vel.y += (slide.y/slen)*accel*dt; vel.z += (slide.z/slen)*accel*dt; }
        } else if (slopeDeg > SLOPE_WALK_MAX_DEG) {
            float t = (slopeDeg - SLOPE_WALK_MAX_DEG) / (SLOPE_CLIMB_MAX_DEG - SLOPE_WALK_MAX_DEG);
            vel.x *= (1.0f - t); vel.z *= (1.0f - t);
        }
        float frictionAccel = boosted ? SLOPE_FRICTION_ACCEL_BOOST : SLOPE_FRICTION_ACCEL;
        float hspeed = vsqrtf(vel.x*vel.x + vel.z*vel.z);
        if (hspeed > 1e-4f) {
            float fd = frictionAccel * dt;
            if (fd >= hspeed) { vel.x = 0.0f; vel.z = 0.0f; }
            else { float s = (hspeed - fd) / hspeed; vel.x *= s; vel.z *= s; }
        }
        Sys_Global.instances[i].velocity = vel;
    }
 
    // Step 4: vertical collision
    if (vel.y * vel.y > 1e-6f) {
        bool movingDown  = vel.y < 0.0f;
        Vector3 vTestEye = { pos.x, pos.y + vel.y * dt, pos.z };
        Vector3 vStart, vEnd;
        CapsuleTipsFromEye(vTestEye, &vStart, &vEnd);
        CapsuleContact vc = QueryCapsuleContact(vStart, vEnd, PLAYER_RADIUS, mask);
        bool blockV = false;
        if (movingDown) {
            blockV = (vc.depth > 0.0f && vc.normal.y > 0.1f);
        } else if (vc.depth > 0.0f && vc.normal.y < -0.1f) {
            blockV = (vc.depth > QueryCapsuleContact(curStart, curEnd, PLAYER_RADIUS, mask).depth);
        }
        if (blockV) { Sys_Global.instances[i].velocity.y = 0.0f; vel.y = 0.0f; }
    }
 
    // Step 5: horizontal collision with sliding projection
    Vector3 hVel = { vel.x, 0.0f, vel.z };
    if (hVel.x*hVel.x + hVel.z*hVel.z > 1e-6f) {
        Vector3 hTestEye = { pos.x + hVel.x*dt, pos.y, pos.z + hVel.z*dt };
        Vector3 hStart, hEnd;
        CapsuleTipsFromEye(hTestEye, &hStart, &hEnd);
        CapsuleContact hc = QueryCapsuleContact(hStart, hEnd, PLAYER_RADIUS, mask);
        if (hc.depth > 0.0f && hc.depth > QueryCapsuleContact(curStart, curEnd, PLAYER_RADIUS, mask).depth) {
            float vdn = dot_vector3(vel, hc.normal);
            if (vdn < 0.0f) {
                vel.x -= hc.normal.x * vdn;
                vel.z -= hc.normal.z * vdn;
                Sys_Global.instances[i].velocity.x = vel.x;
                Sys_Global.instances[i].velocity.z = vel.z;
            }
        }
    }
 
    // Step 6: integrate
    vel = Sys_Global.instances[i].velocity;
    if (magnitude_vector3(vel) < 0.05f) return;
    Sys_Global.instances[i].position = Vector3_A_plus_B(pos, scale_vector3(vel, dt));
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
