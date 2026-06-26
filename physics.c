// Phys Sys
#define MAX_COLLISION_ITERATIONS 64
#define RESTITUTION 0.15f
#define FRICTION 0.8f
#define STEP_MIN_NORMAL_Y 0.7f
#define PHY_EPSILON 0.0001f
#define MAX_SUBSTEPS 10
#define MAX_SPEED 8.0f // m/s
#define MAX_STEP_SIZE (0.1f / MAX_SPEED) // 0.01 s
#define MAX_ANGULAR_SPEED 5.0f
#define MANIFOLD_MAX 4
#define MANIFOLD_TIE_MARGIN 0.005f
#define MANIFOLD_ALIGN_THRESHOLD 0.8f
#define MANIFOLD_DEDUP_DIST_SQ 1e-5f
#define CVXMSH_HULL_CACHE 512
typedef struct {V3 ctr,halfExtents; Quaternion rot;} ShapeBox; typedef struct {V3 ctr; float rad;} ShapeSphere;/*Et tu brute?*/ typedef struct {V3 tip,base; float rad;} ShapeCapsule;
ENGINE_TO_MOD void SetPosition(Entity* e, V3 newpos, bool teleport) { float d = V3_Dist(e->position,newpos); if ((d > 0.001f && d < 0.1f) || teleport) {e->position = newpos;} }
u16 dynamicEntities[512], dynamicEntityCount;
typedef struct { bool hit; V3 point,normal; float overlapAmount; } OverlapResult; typedef struct { V3 point; float pen; } ManifoldPt; typedef struct { V3 normal; ManifoldPt p[MANIFOLD_MAX]; int n; float maxPen; } Manifold;
INLINE u32 PosGetCellCoordsP(i32 cx, i32 cz) { cx = clamp(cx,0,WORLDX_0BASED); cz = clamp(cz,0,WORLDX_0BASED); return (u32)cz * WORLDX + (u32)cx; }
static inline Manifold OverlapToManifold(OverlapResult r) { Manifold m = {0}; if (r.hit && r.overlapAmount > PHY_EPSILON) { m.normal = r.normal; m.n = 1; m.p[0] = (ManifoldPt){r.point, r.overlapAmount}; m.maxPen = r.overlapAmount; } return m; }
static inline OverlapResult SphSph(V3 a, float ar, V3 b, float br) {
    V3 delta = V3_AsubB(a,b); float dist = V3_Mag(delta), radSum = (ar + br); V3 n = (dist < PHY_EPSILON) ? (V3){0,1,0} : V3_ScaleByF(delta,1.0f / dist);
    return (dist < radSum) ? (OverlapResult){true,V3_AplusB(b,V3_ScaleByF(n,br)),n,radSum - dist} : (OverlapResult){0,{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f},0.0f};
}

static float ClosestSegmentSegment(V3 a0, V3 a1, V3 b0, V3 b1, float *sc, float *tc) { // Closest point between two line segments A0-A1 and B0-B1.  Returns squared distance and writes sc, tc (parameters on each segment).
    V3 d1 = V3_AsubB(a1,a0), d2 = V3_AsubB(b1,b0), r = V3_AsubB(a0,b0);
    float a = V3_dot(d1,d1), e = V3_dot(d2,d2), f = V3_dot(d2,r);
    if (a < PHY_EPSILON && e < PHY_EPSILON) { *sc = *tc = 0.0f; return V3_dot(r,r); }
    if (a < PHY_EPSILON) { *sc = 0.0f; *tc = vclamp(f/e, 0.0f, 1.0f); }
    else {
        float c = V3_dot(d1,r);
        if (e < PHY_EPSILON) { *tc = 0.0f; *sc = vclamp(-c/a, 0.0f, 1.0f); }
        else {
            float b = V3_dot(d1,d2), denom = a*e - b*b;
            *sc = (denom > PHY_EPSILON) ? vclamp((b*f - c*e)/denom, 0.0f, 1.0f) : 0.0f;
            *tc = (b * (*sc) + f) / e;
            if (*tc < 0.0f) { *tc = 0.0f; *sc = vclamp(-c/a, 0.0f, 1.0f); }
            else if (*tc > 1.0f) { *tc = 1.0f; *sc = vclamp((b-c)/a, 0.0f, 1.0f); }
        }
    }

    V3 diff = V3_AsubB(V3_AplusB(a0, V3_ScaleByF(d1,*sc)), V3_AplusB(b0, V3_ScaleByF(d2,*tc)));
    return V3_dot(diff,diff);
}

static OverlapResult CapCap(ShapeCapsule a, ShapeCapsule b) {
    OverlapResult r = {0};
    float sc, tc; float distSq = ClosestSegmentSegment(a.base, a.tip, b.base, b.tip, &sc, &tc);
    float radSum = a.rad + b.rad; if (distSq >= radSum * radSum) return r;

    float dist = vsqrtf(vmax(distSq, 0.0f));
    r.overlapAmount = radSum - dist; r.hit = true;
    V3 ptA = V3_AplusB(a.base,V3_ScaleByF(V3_AsubB(a.tip,a.base),sc)); V3 ptB = V3_AplusB(b.base,V3_ScaleByF(V3_AsubB(b.tip,b.base),tc)); V3 delta = V3_AsubB(ptA,ptB);
    r.normal = (dist < PHY_EPSILON) ? (V3){0,1,0} : V3_ScaleByF(delta, 1.0f/dist);
    r.point  = V3_AplusB(ptB, V3_ScaleByF(r.normal,b.rad));
    return r;
}

void obb_axes(Quaternion q, V3 *ax, V3 *ay, V3 *az) { *ax=quat_rot_v3(q,(V3){1,0,0}); *ay=quat_rot_v3(q,(V3){0,1,0}); *az=quat_rot_v3(q,(V3){0,0,1}); }
static V3 ClosestPointOBB(V3 p, ShapeBox b) {
    V3 ax, ay, az; obb_axes(b.rot, &ax, &ay, &az);
    V3 d = V3_AsubB(p, b.ctr);
    float lx = V3_dot(d, ax), ly = V3_dot(d, ay), lz = V3_dot(d, az);
    lx = vclamp(lx,-b.halfExtents.x,b.halfExtents.x); ly = vclamp(ly,-b.halfExtents.y,b.halfExtents.y); lz = vclamp(lz,-b.halfExtents.z,b.halfExtents.z);
    V3 q = b.ctr;
    q = V3_AplusB(q,V3_ScaleByF(ax,lx)); q = V3_AplusB(q,V3_ScaleByF(ay,ly)); q = V3_AplusB(q,V3_ScaleByF(az,lz));
    return q;
}

static OverlapResult SphBox(V3 ctr, float rad, ShapeBox box) {
    OverlapResult r = {0}; V3 closest = ClosestPointOBB(ctr,box); V3 delta = V3_AsubB(ctr,closest); float distSq = V3_dot(delta,delta); if (distSq >= rad * rad) return r;

    r.hit = true; float dist = vsqrtf(vmax(distSq, 0.0f));
    if (dist > PHY_EPSILON) { r.normal = V3_ScaleByF(delta, 1.0f / dist); r.overlapAmount = rad - dist; }
    else { // Center is inside OBB — find minimum penetration axis
        V3 ax,ay,az; obb_axes(box.rot,&ax,&ay,&az);
        V3 local = V3_AsubB(ctr,box.ctr);
        float lx = V3_dot(local,ax), ly = V3_dot(local,ay), lz = V3_dot(local,az);
        float dx = box.halfExtents.x - vabs(lx), dy = box.halfExtents.y - vabs(ly), dz = box.halfExtents.z - vabs(lz);
        if (dx < dy && dx < dz) { r.normal = V3_ScaleByF(ax,lx > 0 ? 1.f : -1.f); r.overlapAmount = rad + dx; }
        else if (dy < dz)       { r.normal = V3_ScaleByF(ay,ly > 0 ? 1.f : -1.f); r.overlapAmount = rad + dy; }
        else                    { r.normal = V3_ScaleByF(az,lz > 0 ? 1.f : -1.f); r.overlapAmount = rad + dz; }
    }
    r.point = closest;
    return r;
}

static u32 GetCollisionMask(u32 layer) {
    if (layer == L_NPCTrigger || layer == L_NPCClip) return L_NPC;
    switch (layer) {
        case L_Default:          return L_Default|L_TransparentFX|L_Geometry|L_NPC|L_PlayerBullets|L_Player|L_Corpse|L_PhysObjects|L_Trigger|L_Door|L_InterDebris|L_Player2|L_NPCBullet|L_Clip|L_CorpseSearchable;
        case L_TransparentFX:    return L_Default|L_TransparentFX|L_Geometry|L_NPC|L_PlayerBullets|L_Player|L_PhysObjects|L_Trigger|L_Door|L_InterDebris|L_Player2|L_NPCBullet|L_Clip;
        case L_Geometry:         return L_Default|L_TransparentFX|L_Geometry|L_NPC|L_PlayerBullets|L_Player|L_PhysObjects|L_Trigger|L_Door|L_InterDebris|L_Player2|L_Clip;
        case L_NPC:              return L_Default|L_TransparentFX|L_Geometry|L_NPC|L_PlayerBullets|L_Player|L_PhysObjects|L_Trigger|L_NPCTrigger|L_Door|L_InterDebris|L_Player2|L_NPCBullet|L_NPCClip|L_Clip;
        case L_PlayerBullets:    return L_Default|L_TransparentFX|L_Geometry|L_NPC|L_PlayerBullets|L_Player|L_Corpse|L_PhysObjects|L_Door|L_InterDebris|L_Player2|L_NPCBullet|L_Clip|L_CorpseSearchable;
        case L_Player:           return L_Default|L_TransparentFX|L_Geometry|L_NPC|L_PhysObjects|L_PlayerTriggerOnly|L_Trigger|L_Door|L_Player2|L_NPCBullet|L_Clip;
        case L_Corpse:           return L_Default|L_Geometry|L_PlayerBullets|L_PhysObjects|L_Door|L_NPCBullet|L_Clip;
        case L_PhysObjects:      return L_Default|L_TransparentFX|L_Geometry|L_NPC|L_PlayerBullets|L_Player|L_Corpse|L_PhysObjects|L_Door|L_InterDebris|L_NPCBullet|L_Clip;
        case L_PlayerTriggerOnly:return  L_Player|L_Player2;
        case L_Trigger:          return L_Default|L_Geometry|L_NPC|L_PlayerBullets|L_Player|L_PhysObjects|L_Door|L_InterDebris|L_Clip;
        case L_Door:             return L_Default|L_TransparentFX|L_Geometry|L_NPC|L_PlayerBullets|L_Player|L_Corpse|L_PhysObjects|L_Trigger|L_Door|L_InterDebris|L_Player2|L_NPCBullet|L_Clip;
        case L_InterDebris:      return L_Default|L_Geometry|L_NPC|L_PlayerBullets|L_PhysObjects|L_Trigger|L_Door|L_NPCBullet|L_Clip;
        case L_Player2:          return L_Default|L_TransparentFX|L_Geometry|L_NPC|L_PlayerBullets|L_Player|L_PhysObjects|L_PlayerTriggerOnly|L_Trigger|L_Door|L_InterDebris|L_Player2|L_NPCBullet|L_Clip;
        case L_NPCBullet:        return L_Default|L_TransparentFX|L_Geometry|L_PlayerBullets|L_Player|L_Corpse|L_PhysObjects|L_Door|L_InterDebris|L_Player2|L_Clip|L_CorpseSearchable;
        case L_Clip:             return  L_Player|L_Player2|L_NPC;
        case L_CorpseSearchable: return L_Default|L_PlayerBullets;
        default:                 return 0u;
    }
}

void Entity_GetCap(const Entity *e, ShapeCapsule *out) {
    float r = e->colliderSize.x; float hi = vmax(0.0f, (e->colliderSize.y * 0.5f) - r); V3 wc,axis;
    u16 edx = (u16)(e - World.instances);
    if (edx == PLAYER1 || edx == PLAYER2 || e->layer == L_NPC) { wc = V3_AplusB(e->position, e->colliderCenter); axis = (V3){0.0f,1.0f,0.0f}; } // Force player capsules to remain strictly upright, ignoring camera pitch/roll
    else { wc = V3_AplusB(e->position, quat_rot_v3(e->rotation, e->colliderCenter)); axis = (e->colliderSize.z < 0.5f) ? quat_rot_v3(e->rotation,(V3){1,0,0}) : (e->colliderSize.z < 1.5f) ? quat_rot_v3(e->rotation,(V3){0,1,0}) : quat_rot_v3(e->rotation,(V3){0,0,1}); }
    out->rad = r; out->base = V3_AsubB(wc,V3_ScaleByF(axis,hi)); out->tip  = V3_AplusB(wc,V3_ScaleByF(axis,hi));
}

void Entity_GetBox(const Entity *e, ShapeBox *out) { out->ctr=V3_AplusB(e->position,quat_rot_v3(e->rotation,e->colliderCenter)); out->halfExtents=(V3){e->colliderSize.x*0.5f * e->scale.x,e->colliderSize.y*0.5f * e->scale.y,e->colliderSize.z*0.5f * e->scale.z}; out->rot=e->rotation; }
void Entity_GetSph(const Entity *e, ShapeSphere *out) { out->ctr=V3_AplusB(e->position,quat_rot_v3(e->rotation,e->colliderCenter)); out->rad = e->colliderSize.x * vmax(e->scale.x,vmax(e->scale.y,e->scale.z)); }
static inline Color ColliderColor(Entity *e) { return (!(e->entflags & EF_RIGIDBODY)) ? textColors[T_GREEN_MENU_SHADOW] : ((e->colliding) ? textColors[T_RED] : textColors[T_GREEN]); }
static void DrawVelocityVector(Entity *e) {
    if (!(e->entflags & EF_RIGIDBODY)) return;

    V3 tip = V3_AplusB(e->position,V3_ScaleByF(e->velocity,0.25f)); AddWireLine(e->position,tip,textColors[T_ORANGE]);
    V3 perp = V3_Normalize(V3_Cross(e->velocity,(vabs(e->velocity.y/V3_Mag(e->velocity)) < 0.9f) ? (V3){0,1,0} : (V3){1,0,0}));
    AddWireLine(V3_AplusB(tip,V3_ScaleByF(perp,0.05f)),V3_AsubB(tip,V3_ScaleByF(perp,0.05f)),textColors[T_ORANGE]); // Small cross at tip so zero-length vecs are still visible when barely moving
}

void DrawBoxCollider(Entity *e) {
    Color col = ColliderColor(e);
    ShapeBox b; Entity_GetBox(e,&b); V3 ax,ay,az,c[8],px,py,pz; obb_axes(b.rot,&ax,&ay,&az);
    px=V3_ScaleByF(ax,b.halfExtents.x); py=V3_ScaleByF(ay,b.halfExtents.y); pz=V3_ScaleByF(az,b.halfExtents.z);
    for (int s=0;s<8;s++) { float sx=(s&1)?1.f:-1.f,sy=(s&2)?1.f:-1.f,sz=(s&4)?1.f:-1.f; c[s]=V3_AplusB(b.ctr,V3_AplusB(V3_AplusB(V3_ScaleByF(px,sx),V3_ScaleByF(py,sy)),V3_ScaleByF(pz,sz))); }
    AddWireLine(c[0],c[1],col); AddWireLine(c[2],c[3],col); AddWireLine(c[4],c[5],col); AddWireLine(c[6],c[7],col);
    AddWireLine(c[0],c[2],col); AddWireLine(c[1],c[3],col); AddWireLine(c[4],c[6],col); AddWireLine(c[5],c[7],col);
    AddWireLine(c[0],c[4],col); AddWireLine(c[1],c[5],col); AddWireLine(c[2],c[6],col); AddWireLine(c[3],c[7],col);
    DrawVelocityVector(e);
}

void DrawSphereWireframe(Color col, ShapeSphere s) {
    float step=6.28318530f/12;
    for (int seg=0;seg<12;seg++) {
        float a0=seg*step,a1=a0+step,c0=vcosf(a0),s0=vsinf(a0),c1=vcosf(a1),s1=vsinf(a1);
        AddWireLine(V3_AplusB(s.ctr,(V3){c0*s.rad,0,s0*s.rad}),V3_AplusB(s.ctr,(V3){c1*s.rad,0,s1*s.rad}),col);
        AddWireLine(V3_AplusB(s.ctr,(V3){c0*s.rad,s0*s.rad,0}),V3_AplusB(s.ctr,(V3){c1*s.rad,s1*s.rad,0}),col);
        AddWireLine(V3_AplusB(s.ctr,(V3){0,c0*s.rad,s0*s.rad}),V3_AplusB(s.ctr,(V3){0,c1*s.rad,s1*s.rad}),col);
    }
}

void DrawSphereCollider(Entity *e) { Color col = ColliderColor(e); ShapeSphere s; Entity_GetSph(e,&s); DrawSphereWireframe(col,s); DrawVelocityVector(e); }
void DrawSphereContact(V3 pos, float rad) { if (Cheats.showPhys) {Color col = (Color){0.0f,0.0f,1.0f,1.0f}; ShapeSphere s = (ShapeSphere){pos,rad}; DrawSphereWireframe(col,s);} }
void DrawMeshCollider(Entity *e) {
    Color col = ColliderColor(e); u16 mi= (e->collider == COLTYPE_CVX) ? e->colMeshIndex : e->modelIndex; if (mi >= MAX_MDLS || mi >= mdlsCnt) return;
    u32 triCount=modelTriangleCounts[mi]; if (!triCount) return;

    u16 idx = (u16)(e - World.instances);
    float M[16]; mcpy(M,&modelMatrices[idx*16],64);
    float m00=M[0],m10=M[1],m20=M[2],m01=M[4],m11=M[5],m21=M[6],m02=M[8],m12=M[9],m22=M[10],tx=M[12],ty=M[13],tz=M[14];
    for (u32 j=0;j<triCount;j++) {
        u32 bA=(u32)modelTriangles[mi][j*3+0]*VRT_ATT_SZ,bB=(u32)modelTriangles[mi][j*3+1]*VRT_ATT_SZ,bC=(u32)modelTriangles[mi][j*3+2]*VRT_ATT_SZ;
        #define LV(b) (V3){half_to_float(*(half*)(modelVertices[mi]+(b)+0)),half_to_float(*(half*)(modelVertices[mi]+(b)+2)),half_to_float(*(half*)(modelVertices[mi]+(b)+4))}
        #define XFORM(v) (V3){m00*(v).x+m01*(v).y+m02*(v).z+tx,m10*(v).x+m11*(v).y+m12*(v).z+ty,m20*(v).x+m21*(v).y+m22*(v).z+tz}
        V3 wA=XFORM(LV(bA)),wB=XFORM(LV(bB)),wC=XFORM(LV(bC));
        #undef LV
        #undef XFORM
        AddWireLine(wA,wB,col); AddWireLine(wB,wC,col); AddWireLine(wC,wA,col);
    }
    DrawVelocityVector(e);
}

void DrawCapsuleCollider(Entity *e) {
    Color col = ColliderColor(e); ShapeCapsule cap; Entity_GetCap(e,&cap);
    V3 axis=V3_Normalize(V3_AsubB(cap.tip,cap.base)); V3 ref=(vabs(axis.y)<0.9f)?(V3){0,1,0}:(V3){1,0,0}; V3 perp0=V3_Normalize(V3_Cross(axis,ref)),perp1=V3_Cross(axis,perp0);
    float step=6.28318530f/12,r=cap.rad;
    for (int seg=0;seg<12;seg++) {
        float a0=seg*step,a1=a0+step,c0=vcosf(a0),s0=vsinf(a0),c1=vcosf(a1),s1=vsinf(a1);
        V3 r0 = V3_AplusB(V3_ScaleByF(perp0,c0*r),V3_ScaleByF(perp1,s0*r)), r1=V3_AplusB(V3_ScaleByF(perp0,c1*r),V3_ScaleByF(perp1,s1*r));
        AddWireLine(V3_AplusB(cap.base,r0),V3_AplusB(cap.base,r1),col); AddWireLine(V3_AplusB(cap.tip,r0),V3_AplusB(cap.tip,r1),col);
    }

    for (int seg=0;seg<6;seg++) {
        float a0=seg*step,a1=a0+step,c0=vcosf(a0),s0=vsinf(a0),c1=vcosf(a1),s1=vsinf(a1);
        AddWireLine(V3_AplusB(cap.base,V3_AplusB(V3_ScaleByF(perp0,c0*r),V3_ScaleByF(axis,-s0*r))),V3_AplusB(cap.base,V3_AplusB(V3_ScaleByF(perp0,c1*r),V3_ScaleByF(axis,-s1*r))),col);
        AddWireLine(V3_AplusB(cap.base,V3_AplusB(V3_ScaleByF(perp1,c0*r),V3_ScaleByF(axis,-s0*r))),V3_AplusB(cap.base,V3_AplusB(V3_ScaleByF(perp1,c1*r),V3_ScaleByF(axis,-s1*r))),col);
        AddWireLine(V3_AplusB(cap.tip, V3_AplusB(V3_ScaleByF(perp0,c0*r),V3_ScaleByF(axis, s0*r))),V3_AplusB(cap.tip, V3_AplusB(V3_ScaleByF(perp0,c1*r),V3_ScaleByF(axis, s1*r))),col);
        AddWireLine(V3_AplusB(cap.tip, V3_AplusB(V3_ScaleByF(perp1,c0*r),V3_ScaleByF(axis, s0*r))),V3_AplusB(cap.tip, V3_AplusB(V3_ScaleByF(perp1,c1*r),V3_ScaleByF(axis, s1*r))),col);
    }

    for (int seg=0;seg<4;seg++) { float a=seg*(6.28318530f/4.f); V3 off=V3_AplusB(V3_ScaleByF(perp0,vcosf(a)*r),V3_ScaleByF(perp1,vsinf(a)*r)); AddWireLine(V3_AplusB(cap.base,off),V3_AplusB(cap.tip,off),col); }
    DrawVelocityVector(e);
}

static void DrawAngularVelocity(Entity *e) {
    if (!Cheats.showPhys) return;
    if (!(e->entflags & EF_RIGIDBODY)) return;
    if (V3_Mag(e->angularVelocity) < 0.0001f) return; // skip near-zero

    Color purple = (Color){0.5f, 0.0f, 1.0f, 1.0f};
    float scale = 0.35f;
    V3 dir = V3_Normalize(e->angularVelocity);
    V3 tip = V3_AplusB(e->position, V3_ScaleByF(e->angularVelocity, scale));
    AddWireLine(e->position, tip, purple); // Arrow (line vector)
    V3 ref = (vabs(dir.y) < 0.9f) ? (V3){0,1,0} : (V3){1,0,0};
    V3 perp = V3_Normalize(V3_Cross(dir, ref));
    V3 perp2 = V3_Cross(dir, perp);
    AddWireLine(V3_AplusB(tip, V3_ScaleByF(perp,  0.05f)),V3_AplusB(tip, V3_ScaleByF(perp, -0.05f)), purple); // Small cross at tip so zero-length vectors are still visible
    AddWireLine(V3_AplusB(tip, V3_ScaleByF(perp2, 0.05f)),V3_AplusB(tip, V3_ScaleByF(perp2,-0.05f)), purple);
    float rad = 0.6f; // Quarter circle arc (visualizes rotation plane + sense)
    float step = 1.57079632679f / 8.0f; // quarter circle divided into 8 segments
    V3 axis = dir; V3 p1 = V3_Normalize(V3_Cross(axis,ref)); V3 p2 = V3_Cross(axis,p1); // Find two vectors perpendicular to angular axis
    V3 prev = V3_AplusB(e->position, V3_ScaleByF(p1,rad));
    for (int i = 1; i <= 8; ++i) { float a = i * step; float c = vcosf(a); float s = vsinf(a); V3 cur = V3_AplusB(e->position,V3_AplusB(V3_ScaleByF(p1,c * rad),V3_ScaleByF(p2,s * rad))); AddWireLine(prev,cur,purple); prev = cur; }
}

static u16 cellLists[WORLDX*WORLDX][128],cellCounts[WORLDX*WORLDX];
float GetColRad(Entity *e) { if (e->collider == COLTYPE_BOX) { float hx = e->colliderSize.x * 0.5f * e->scale.x, hy = e->colliderSize.y * 0.5f * e->scale.y, hz = e->colliderSize.z * 0.5f * e->scale.z; return vsqrtf(hx*hx + hy*hy + hz*hz); } return e->colliderSize.x; }
Quaternion quat_from_axis_angle(V3 axis, float angle) { float half = angle * 0.5f; float s = vsinf(half); return (Quaternion){axis.x * s,axis.y * s,axis.z * s,vcosf(half)}; }
Quaternion quat_normalize(Quaternion q) { float len2 = q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w; if (len2 < PHY_EPSILON) {return (Quaternion){0,0,0,1};} float inv = 1.0f / vsqrtf(len2); q.x *= inv; q.y *= inv; q.z *= inv; q.w *= inv; return q; }
static inline bool V3_IsSane(V3 v) { return (v.x<=1e6f && v.x>=-1e6f && v.y<=1e6f && v.y>=-1e6f && v.z<=1e6f && v.z>=-1e6f); } // false for NaN/Inf too: comparisons against NaN are always false
static void ApplyVelocity(Entity *e, float dt) {
    V3 acc = {0.0f,-9.81f*e->gravity,0.0f};
    u16 edx = (u16)(e - World.instances);
    if ((edx == PLAYER1 || edx == PLAYER2) && Cheats.noclip) acc.y = 0.0f;
    acc = V3_AplusB(acc,V3_ScaleByF(e->accumulatedForce,1.0f / e->mass));
    e->velocity = V3_AplusB(e->velocity,V3_ScaleByF(acc,dt));
    float speed = V3_Mag(e->velocity);
    if (speed > MAX_SPEED) e->velocity = V3_ScaleByF(V3_ScaleByF(e->velocity, 1.0f / speed),MAX_SPEED);
    if (!V3_IsSane(e->velocity)) e->velocity = (V3){0.0f,0.0f,0.0f}; // catches what the MAX_SPEED check above can't: NaN/Inf compare false, so it falls through unclamped without this
    float linDrag = vexp(-2.0f * dt);
    e->velocity = V3_ScaleByF(e->velocity,linDrag);
    SetPosition(e,V3_AplusB(e->position,V3_ScaleByF(e->velocity,dt)),false); // pos += (d = v*t)
    if (e->collider != COLTYPE_CAP) {
        float angDrag = vexp(-e->angularDrag * dt);
        e->angularVelocity = V3_ScaleByF(e->angularVelocity,angDrag); // 1. Apply continuous angular drag over time
        float avel = V3_Mag(e->angularVelocity);
        if (avel > MAX_ANGULAR_SPEED) { e->angularVelocity = V3_ScaleByF(e->angularVelocity,MAX_ANGULAR_SPEED / avel); avel = MAX_ANGULAR_SPEED; }
        if (!V3_IsSane(e->angularVelocity)) { e->angularVelocity = (V3){0.0f,0.0f,0.0f}; avel = 0.0f; }
        if (avel > PHY_EPSILON) { Quaternion dq = quat_from_axis_angle(V3_ScaleByF(e->angularVelocity,1.f / avel),avel * dt); e->rotation = quat_normalize(quat_multiply(dq,e->rotation)); } // 2. Integrate rotation
    } else e->angularVelocity = (V3){0.0f,0.0f,0.0f};
}

static void ComputeConvexMeshInertiaTensor(Entity *e) {
    u16 mi = e->colMeshIndex; e->inertiaTensorValid = false;
    if (mi >= MAX_MDLS || !modelTriangleCounts[mi] || !modelVertexCounts[mi]) return;

    // acc[0]=x², acc[1]=y², acc[2]=z² to keep things clean and explicitly isolated
    float acc[6] = {0}; float volAcc = 0.f; u32 triCount = modelTriangleCounts[mi];
    for (u32 ti = 0; ti < triCount; ++ti) {
        u32 i0=modelTriangles[mi][ti*3+0], i1=modelTriangles[mi][ti*3+1], i2=modelTriangles[mi][ti*3+2];
        V3 v0 = (V3){half_to_float(*(half*)(modelVertices[mi]+(i0)*VRT_ATT_SZ+0)), half_to_float(*(half*)(modelVertices[mi]+(i0)*VRT_ATT_SZ+2)), half_to_float(*(half*)(modelVertices[mi]+(i0)*VRT_ATT_SZ+4))};
        V3 v1 = (V3){half_to_float(*(half*)(modelVertices[mi]+(i1)*VRT_ATT_SZ+0)), half_to_float(*(half*)(modelVertices[mi]+(i1)*VRT_ATT_SZ+2)), half_to_float(*(half*)(modelVertices[mi]+(i1)*VRT_ATT_SZ+4))};
        V3 v2 = (V3){half_to_float(*(half*)(modelVertices[mi]+(i2)*VRT_ATT_SZ+0)), half_to_float(*(half*)(modelVertices[mi]+(i2)*VRT_ATT_SZ+2)), half_to_float(*(half*)(modelVertices[mi]+(i2)*VRT_ATT_SZ+4))};

        float det = V3_dot(v0,V3_Cross(v1,v2));
        volAcc += det;

        // Pure covariance components
        acc[0] += det * (v0.x*v0.x + v0.x*v1.x + v1.x*v1.x + v0.x*v2.x + v1.x*v2.x + v2.x*v2.x); // ∫x²
        acc[1] += det * (v0.y*v0.y + v0.y*v1.y + v1.y*v1.y + v0.y*v2.y + v1.y*v2.y + v2.y*v2.y); // ∫y²
        acc[2] += det * (v0.z*v0.z + v0.z*v1.z + v1.z*v1.z + v0.z*v2.z + v1.z*v2.z + v2.z*v2.z); // ∫z²
        acc[3] += det * (2*v0.x*v0.y + v0.x*v1.y + v0.x*v2.y + v1.x*v0.y + 2*v1.x*v1.y + v1.x*v2.y + v2.x*v0.y + v2.x*v1.y + 2*v2.x*v2.y); // ∫xy
        acc[4] += det * (2*v0.x*v0.z + v0.x*v1.z + v0.x*v2.z + v1.x*v0.z + 2*v1.x*v1.z + v1.x*v2.z + v2.x*v0.z + v2.x*v1.z + 2*v2.x*v2.z); // ∫xz
        acc[5] += det * (2*v0.y*v0.z + v0.y*v1.z + v0.y*v2.z + v1.y*v0.z + 2*v1.y*v1.z + v1.y*v2.z + v2.y*v0.z + v2.y*v1.z + 2*v2.y*v2.z); // ∫yz
    }

    if (vabs(volAcc) < PHY_EPSILON) return;

    // Scale integration values by actual mesh dimensions quadratically
    float sx = e->scale.x, sy = e->scale.y, sz = e->scale.z;
    float x2 = acc[0] * sx * sx;
    float y2 = acc[1] * sy * sy;
    float z2 = acc[2] * sz * sz;
    float xy = acc[3] * sx * sy;
    float xz = acc[4] * sx * sz;
    float yz = acc[5] * sy * sz;
    float s = e->mass / (volAcc * 10.f);
    float so = e->mass / (volAcc * 20.f);
    float r = modelBounds[mi] * vmax(vmax(sx, sy), sz);
    float mn = ((2.0f / 5.0f) * e->mass * r * r) * 0.1f;

    // Construct the actual mass distribution tensor elements
    float Ixx = vmax((y2 + z2) * s, mn);
    float Iyy = vmax((x2 + z2) * s, mn);
    float Izz = vmax((x2 + y2) * s, mn);
    float Ixy = -xy * so;
    float Ixz = -xz * so;
    float Iyz = -yz * so;

    e->inertiaTensor[0] = Ixx; e->inertiaTensor[1] = Iyy; e->inertiaTensor[2] = Izz;
    e->inertiaTensor[3] = Ixy; e->inertiaTensor[4] = Ixz; e->inertiaTensor[5] = Iyz;

    float det = Ixx * (Iyy * Izz - Iyz * Iyz) - Ixy * (Ixy * Izz - Ixz * Iyz) + Ixz * (Ixy * Iyz - Iyy * Ixz);
    if (vabs(det) < PHY_EPSILON) return;
    float invDet = 1.0f / det;

    e->invInertiaTensor[0] = (Iyy * Izz - Iyz * Iyz) * invDet;
    e->invInertiaTensor[1] = (Ixx * Izz - Ixz * Ixz) * invDet;
    e->invInertiaTensor[2] = (Ixx * Iyy - Ixy * Ixy) * invDet;
    e->invInertiaTensor[3] = (Ixz * Iyz - Ixy * Izz) * invDet;
    e->invInertiaTensor[4] = (Ixy * Iyz - Ixz * Iyy) * invDet;
    e->invInertiaTensor[5] = (Ixy * Ixz - Ixx * Iyz) * invDet;
    e->inertiaTensorValid = true;
}

static OverlapResult BoxBox(ShapeBox a, ShapeBox b) {
    OverlapResult r = {0}; V3 aAxes[3],bAxes[3];
    obb_axes(a.rot,&aAxes[0],&aAxes[1],&aAxes[2]);
    obb_axes(b.rot,&bAxes[0],&bAxes[1],&bAxes[2]);
    V3 T = V3_AsubB(b.ctr,a.ctr);
    float R[3][3],AbsR[3][3];
    for (int i=0;i<3;i++) for (int j=0;j<3;j++) { R[i][j]=V3_dot(aAxes[i],bAxes[j]); AbsR[i][j]=vabs(R[i][j])+1e-6f; }
    float minOverlap=1e9f; int bestAxis=-1; bool flipNormal=false; V3 bestEdgeAxis={0,1,0};
    for (int i=0;i<3;i++) { // Face axes A
        float ra=((float*)&a.halfExtents)[i], rb=b.halfExtents.x*AbsR[i][0]+b.halfExtents.y*AbsR[i][1]+b.halfExtents.z*AbsR[i][2];
        float t=vabs(V3_dot(T,aAxes[i])); if (t>ra+rb) return r;

        float ov=(ra+rb)-t; if (ov<minOverlap) { minOverlap=ov; bestAxis=i; flipNormal=(V3_dot(T,aAxes[i])<0.f); }
    }

    for (int i=0;i<3;i++) { // Face axes B
        float ra=a.halfExtents.x*AbsR[0][i]+a.halfExtents.y*AbsR[1][i]+a.halfExtents.z*AbsR[2][i], rb=((float*)&b.halfExtents)[i];
        float t=vabs(V3_dot(T,bAxes[i])); if (t>ra+rb) return r;

        float ov=(ra+rb)-t; if (ov<minOverlap) { minOverlap=ov; bestAxis=3+i; flipNormal=(V3_dot(T,bAxes[i])<0.f); }
    }

    for (int i=0;i<3;i++) for (int j=0;j<3;j++) { // Edge-edge cross products — store winning edgeAxis when it becomes the best
        int i1=(i+1)%3, i2=(i+2)%3, j1=(j+1)%3, j2=(j+2)%3;
        float t=vabs(V3_dot(T,aAxes[i2])*R[i1][j] - V3_dot(T,aAxes[i1])*R[i2][j]);
        float ra=((float*)&a.halfExtents)[i1]*AbsR[i2][j]+((float*)&a.halfExtents)[i2]*AbsR[i1][j];
        float rb=((float*)&b.halfExtents)[j1]*AbsR[i][j2]+((float*)&b.halfExtents)[j2]*AbsR[i][j1];
        if (t>ra+rb) return r;

        float axLenSq=1.f-(R[i][j]*R[i][j]);
        if (axLenSq>1e-4f) {
            float ov=((ra+rb)-t)/vsqrtf(axLenSq);
            if (ov<minOverlap) { V3 ea=V3_Cross(aAxes[i],bAxes[j]); minOverlap=ov; bestAxis=6+i*3+j; bestEdgeAxis=ea; flipNormal=(V3_dot(T,ea)<0.f); }
        }
    }

    if (bestAxis<0) return r;

    r.hit=true; r.overlapAmount=minOverlap;
    if      (bestAxis<3) r.normal=flipNormal ? aAxes[bestAxis]            : V3_ScaleByF(aAxes[bestAxis],-1.f);
    else if (bestAxis<6) r.normal=flipNormal ? bAxes[bestAxis-3]          : V3_ScaleByF(bAxes[bestAxis-3],-1.f);
    else                 r.normal=flipNormal ? V3_Normalize(bestEdgeAxis) : V3_ScaleByF(V3_Normalize(bestEdgeAxis),-1.f);

    V3 sA=a.ctr;
    sA=V3_AplusB(sA,V3_ScaleByF(aAxes[0],(V3_dot(aAxes[0],r.normal)<0.f?1.f:-1.f)*a.halfExtents.x));
    sA=V3_AplusB(sA,V3_ScaleByF(aAxes[1],(V3_dot(aAxes[1],r.normal)<0.f?1.f:-1.f)*a.halfExtents.y));
    sA=V3_AplusB(sA,V3_ScaleByF(aAxes[2],(V3_dot(aAxes[2],r.normal)<0.f?1.f:-1.f)*a.halfExtents.z));
    r.point=V3_AplusB(sA,V3_ScaleByF(r.normal,minOverlap*0.5f));
    return r;
}

static inline V3 MvVert(const float* M, V3 v) { return (V3){ M[0]*v.x + M[4]*v.y + M[8]*v.z  + M[12], M[1]*v.x + M[5]*v.y + M[9]*v.z  + M[13], M[2]*v.x + M[6]*v.y + M[10]*v.z + M[14] }; }
static V3 MeshSupport(u16 m, const float* M, V3 d) {
    u32 n = modelVertexCounts[m]; if (!n) {return (V3){0};}
    const u8* vb = modelVertices[m]; V3 b={0}; float top=-1e9f;
    for (u32 i=0;i<n;++i) {const u8* p=vb + i*VRT_ATT_SZ; V3 w=MvVert(M,(V3){half_to_float(*(half*)(p+0)),half_to_float(*(half*)(p+2)),half_to_float(*(half*)(p+4))}); float dot=V3_dot(w,d); b=(dot>top) ? (top=dot,w) : b;}
    return b;
}

static inline V3 CachedSupport(const V3* v, u32 n, V3 d) { V3 best = v[0]; float top = V3_dot(v[0],d); for (u32 i = 1; i < n; ++i) { float dot = V3_dot(v[i],d); if (dot > top) { top = dot; best = v[i]; } } return best; }
typedef struct { V3 v[4]; int n; } Simplex3D;
static inline V3 MinkowskiSupport(u16 mA, const float* mxA, u16 mB, const float* mxB, V3 d) { return V3_AsubB(MeshSupport(mA,mxA,d), MeshSupport(mB,mxB,(V3){-d.x,-d.y,-d.z})); }
static inline V3 TP(V3 a, V3 b, V3 c) { return V3_Cross(V3_Cross(a,b),c); }
static inline V3 BoxSupport(ShapeBox b, V3 d) {
    V3 ax,ay,az; obb_axes(b.rot,&ax,&ay,&az); float kx = V3_dot(d,ax) >= 0.0f ? 1.0f : -1.0f, ky = V3_dot(d,ay) >= 0.0f ? 1.0f : -1.0f, kz = V3_dot(d,az) >= 0.0f ? 1.0f : -1.0f;
    return V3_AplusB(V3_AplusB(V3_AplusB(b.ctr,V3_ScaleByF(ax,kx * b.halfExtents.x)),V3_ScaleByF(ay,ky * b.halfExtents.y)),V3_ScaleByF(az,kz * b.halfExtents.z));
}

static inline V3 CapsuleSupport(ShapeCapsule cap, V3 d) { float db = V3_dot(cap.base,d),dt = V3_dot(cap.tip,d); V3 best = (dt > db) ? cap.tip : cap.base; float L = V3_dot(d,d); if (L < PHY_EPSILON) {return best;} return V3_AplusB(best,V3_ScaleByF(d,cap.rad / vsqrtf(L))); }
static bool GJKNextSimplex(Simplex3D *s, V3 *dir) {
    V3 A=s->v[s->n-1], AO={-A.x,-A.y,-A.z};
    if (s->n==2) {
        V3 AB=V3_AsubB(s->v[0],A);
        if (V3_dot(AB,AO)>0.f) *dir=TP(AB,AO,AB);
        else { s->n=1; s->v[0]=A; *dir=AO; }

        if (V3_dot(*dir,*dir)<PHY_EPSILON) { V3 px = (vabs(AB.x)>0.9f) ? (V3){0,1,0} : (V3){1,0,0}; *dir=V3_Cross(AB,px); } // Degenerate check applies regardless of which branch was taken
        return true;
    }
    if (s->n==3) {
        V3 B=s->v[1],C=s->v[0], AB=V3_AsubB(B,A),AC=V3_AsubB(C,A), ABC=V3_Cross(AB,AC);
        if (V3_dot(V3_Cross(ABC,AC),AO)>0.f) {
            if (V3_dot(AC,AO)>0.f) { s->v[1]=A; s->n=2; *dir=TP(AC,AO,AC); } else goto line_AB3;
        } else if (V3_dot(V3_Cross(AB,ABC),AO)>0.f) {
            line_AB3: if (V3_dot(AB,AO)>0.f) { s->v[0]=B; s->v[1]=A; s->n=2; *dir=TP(AB,AO,AB); } else { s->v[0]=A; s->n=1; *dir=AO; }
        } else {
            if (V3_dot(ABC,AO)>0.f) {*dir=ABC;} else { V3 t=s->v[0]; s->v[0]=s->v[1]; s->v[1]=t; *dir=(V3){-ABC.x,-ABC.y,-ABC.z}; }
        }
        return true;
    }

    V3 B=s->v[2],C=s->v[1],D=s->v[0];
    V3 AB=V3_AsubB(B,A),AC=V3_AsubB(C,A),AD=V3_AsubB(D,A);
    V3 nABC=V3_Cross(AB,AC),nACD=V3_Cross(AC,AD),nADB=V3_Cross(AD,AB);
    nABC=V3_dot(nABC,AD)>0.f?(V3){-nABC.x,-nABC.y,-nABC.z}:nABC;
    nACD=V3_dot(nACD,AB)>0.f?(V3){-nACD.x,-nACD.y,-nACD.z}:nACD;
    nADB=V3_dot(nADB,AC)>0.f?(V3){-nADB.x,-nADB.y,-nADB.z}:nADB;
    if (V3_dot(nABC,AO)>0.f){s->v[0]=C;s->v[1]=B;s->v[2]=A;s->n=3;*dir=nABC;return true;}
    if (V3_dot(nACD,AO)>0.f){s->v[0]=D;s->v[1]=C;s->v[2]=A;s->n=3;*dir=nACD;return true;}
    if (V3_dot(nADB,AO)>0.f){s->v[0]=B;s->v[1]=D;s->v[2]=A;s->n=3;*dir=nADB;return true;}
    return false;
}

#define EPA_MAX_FACES 16
#define EPA_MAX_VERTS 48
#define EPA_MAX_EDGES (EPA_MAX_FACES*3)
typedef struct { int a,b,c; V3 n; float d; } EPAFace;
typedef struct { V3 v, wA, wB; } EPAVert;
static inline EPAFace MakeEPAFace(const EPAVert* vb, int a, int b, int c) { V3 n = V3_Cross(V3_AsubB(vb[b].v,vb[a].v),V3_AsubB(vb[c].v,vb[a].v)); float L = V3_Mag(n); if(L < PHY_EPSILON){return (EPAFace){a,b,c,{0},0.f};} n = V3_ScaleByF(n,1.f/L); float d = V3_dot(n,vb[a].v); if(d < 0.f){n=(V3){-n.x,-n.y,-n.z}; d=-d; int t=b;b=c;c=t;} return (EPAFace){a,b,c,n,d}; }
static inline V3 EPAContactPoint(const EPAVert* ev, int a, int b, int c) {
    V3 pa=ev[a].v, pb=ev[b].v, pc=ev[c].v; V3 v0=V3_AsubB(pb,pa), v1=V3_AsubB(pc,pa), v2=V3_AsubB((V3){0,0,0},pa); float d00 = V3_dot(v0,v0); float d01 = V3_dot(v0,v1); float d11 = V3_dot(v1,v1); float d20 = V3_dot(v2,v0); float d21 = V3_dot(v2,v1);
    float denom = d00 * d11 - d01 * d01 + PHY_EPSILON; float v = vmax((d11 * d20 - d01 * d21) * (1.0f / denom),0.0f); float w = vmax((d00 * d21 - d01 * d20) * (1.0f / denom),0.0f); float u = vmax(1.0f - v - w,0.0f);
    float sum = u + v + w; if (sum > PHY_EPSILON) {u /= sum; v /= sum; w /= sum;} return (V3){u*ev[a].wA.x + v*ev[b].wA.x + w*ev[c].wA.x,u*ev[a].wA.y + v*ev[b].wA.y + w*ev[c].wA.y,u*ev[a].wA.z + v*ev[b].wA.z + w*ev[c].wA.z};
}

static inline Manifold MakeEPAManifold(const EPAVert* ev, int a, int b, int c, V3 n, float d) { Manifold m = {0};  m.normal = n;  m.maxPen = d; m.n = 1; m.p[0] = (ManifoldPt){ EPAContactPoint(ev,a,b,c), d }; return m; }
static void inline FeatureOverlap(V3 sc, float sr, V3 pt, OverlapResult* r) { V3 delta=V3_AsubB(sc,pt); float dist2=V3_dot(delta,delta); if (dist2 < sr*sr) { float dist=vsqrtf(vmax(dist2,0.0f)); OverlapResult t={true,pt,(dist>PHY_EPSILON) ? V3_ScaleByF(delta,1.0f/dist) : (V3){0.0f,1.0f,0.0f},sr - dist}; if(t.overlapAmount>r->overlapAmount) *r=t; } }
static OverlapResult SphMsh(V3 sc, float sr, u16 mesh, const float* mx) { // Triangle-soup mesh support: test sphere/capsule against all triangles of a static mesh.  Returns deepest overlapping triangle result.  Normal points from mesh toward mover. Voronoi region closest point.
    OverlapResult r = {0}; if (mesh >= MAX_MDLS) {return r;} u32 triCount = modelTriangleCounts[mesh]; if (!triCount){return r;}

    for (u32 ti = 0; ti < triCount; ++ti) {
        u32 i0 = modelTriangles[mesh][ti*3+0], i1 = modelTriangles[mesh][ti*3+1], i2 = modelTriangles[mesh][ti*3+2];
        #define RV(i) MvVert(mx,(V3){half_to_float(*(half*)(modelVertices[mesh]+(i)*VRT_ATT_SZ+0)), half_to_float(*(half*)(modelVertices[mesh]+(i)*VRT_ATT_SZ+2)), half_to_float(*(half*)(modelVertices[mesh]+(i)*VRT_ATT_SZ+4))})
        V3 a=RV(i0), b=RV(i1), c=RV(i2);
        #undef RV
        V3 ab=V3_AsubB(b,a), ac=V3_AsubB(c,a);
        V3 ap=V3_AsubB(sc,a); float d1=V3_dot(ab,ap), d2=V3_dot(ac,ap); if(d1 <= 0.0f && d2 <= 0.0f){FeatureOverlap(sc,sr,a,&r); continue;} // Vertex A region
        V3 bp=V3_AsubB(sc,b); float d3=V3_dot(ab,bp), d4=V3_dot(ac,bp); if(d3 >= 0.0f && d4 <= d3){FeatureOverlap(sc,sr,b,&r); continue;} // Vertex B region
        V3 cp=V3_AsubB(sc,c); float d5=V3_dot(ab,cp), d6=V3_dot(ac,cp); if(d6>=0.f && d5<=d6){FeatureOverlap(sc,sr,c,&r); continue;} // Vertex C region
        float vc=d1*d4-d3*d2; if (vc<=0.f && d1>=0.f && d3<=0.f) { float v=d1/(d1-d3); V3 pt=V3_AplusB(a,V3_ScaleByF(ab,v)); FeatureOverlap(sc,sr,pt,&r); continue; } // Edge AB region
        float vb=d5*d2-d1*d6; if (vb<=0.f && d2>=0.f && d6<=0.f) { float w=d2/(d2-d6); V3 pt=V3_AplusB(a,V3_ScaleByF(ac,w)); FeatureOverlap(sc,sr,pt,&r); continue; } // Edge AC region
        float va=d3*d6-d5*d4; if (va<=0.f && (d4-d3)>=0.f && (d5-d6)>=0.f) { float w=(d4-d3)/((d4-d3)+(d5-d6)); V3 bc=V3_AsubB(c,b); V3 pt=V3_AplusB(b,V3_ScaleByF(bc,w)); FeatureOverlap(sc,sr,pt,&r); continue; } // Edge BC region
        V3 n = V3_Cross(ab,ac); float nLen=V3_Mag(n); if(nLen<PHY_EPSILON) continue; // Face region — project onto triangle plane

        n=V3_ScaleByF(n,1.f/nLen); float dist=V3_dot(n,ap); float absDist=vabs(dist);
        if (absDist < sr) { V3 fn = (dist >= 0.0f) ? n : (V3){-n.x,-n.y,-n.z}; OverlapResult t={true,V3_AsubB(sc,V3_ScaleByF(fn,absDist)),fn,sr-absDist}; if(t.overlapAmount>r.overlapAmount) {r=t;} } // Back-face: if sphere is below the triangle, flip normal so response pushes it out correctly
    }
    return r;
}

static OverlapResult CapMsh(ShapeCapsule cap, u16 mesh, const float* mx) { OverlapResult rb = SphMsh(cap.base,cap.rad,mesh,mx); OverlapResult rt = SphMsh(cap.tip,cap.rad,mesh,mx); return (rt.overlapAmount > rb.overlapAmount) ? rt : rb; } // TODO: Connectivity needed or is snowman ala System Shock 1 fine enough?  Might prove funny if player can get stuck with top and bottom on either side of door while leaning like in original.
static Manifold CvxCvx(u16 meshA, u16 meshB, const float* matA, const float* matB) {
    Manifold m = {0}; if (meshA >= MAX_MDLS || meshB >= MAX_MDLS) return m;
    Simplex3D s = {0}; V3 dir = {0.0f,1.0f,0.0f}; s.v[s.n++] = MinkowskiSupport(meshA,matA,meshB,matB,dir); dir = (V3){-s.v[0].x,-s.v[0].y,-s.v[0].z};
    if (V3_dot(dir,dir) < PHY_EPSILON) dir=(V3){0.0f,1.0f,0.0f};
    bool hit=false;
    for (int it=0;it<64;++it) {
        V3 sup = MinkowskiSupport(meshA,matA,meshB,matB,dir);
        if (V3_dot(sup,dir) < 0) return m;
        s.v[s.n++] = sup;
        if (!GJKNextSimplex(&s,&dir) || (V3_dot(dir,dir) < 0)) { hit=true; break; }
    }
    if (!hit) return m;

    static const V3 kAxes[6] = {{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
    for (int d=0;s.n<4 && d<6;++d) {
        V3 sup = MinkowskiSupport(meshA,matA,meshB,matB,kAxes[d]); bool dup=false;
        for (int k=0;k<s.n;k++) { V3 dv=V3_AsubB(sup,s.v[k]); dup|=(V3_dot(dv,dv)<PHY_EPSILON*PHY_EPSILON); }
        if (!dup) s.v[s.n++]=sup;
    }
    if (s.n < 4) return m;

    EPAVert ev[EPA_MAX_VERTS]; EPAFace ef[EPA_MAX_FACES]; int nv=0, nf=0;
    for (int i=0;i<4;i++) { ev[nv].wA=MeshSupport(meshA,matA,s.v[i]); ev[nv].wB=MeshSupport(meshB,matB,(V3){-s.v[i].x,-s.v[i].y,-s.v[i].z}); ev[nv].v=s.v[i]; nv++; }
    static const int kTetFaces[4][3] = {{0,1,2},{0,3,1},{0,2,3},{1,3,2}};
    for (int f=0;f<4;f++) { EPAFace face = MakeEPAFace(ev, kTetFaces[f][0], kTetFaces[f][1], kTetFaces[f][2]); if (face.d >= 0.f && nf < EPA_MAX_FACES) ef[nf++]=face; }
    for (int it=0; it<32; ++it) {
        int bf=-1; float bd=1e9f;
        for (int f=0;f<nf;f++) if(ef[f].d<bd){bd=ef[f].d;bf=f;}
        if (bf<0) break;
        V3 bn=ef[bf].n; V3 wA=MeshSupport(meshA,matA,bn); V3 wB=MeshSupport(meshB,matB,(V3){-bn.x,-bn.y,-bn.z}); V3 sup=V3_AsubB(wA,wB);
        if (V3_dot(bn,sup) - bd < PHY_EPSILON) return MakeEPAManifold(ev,ef[bf].a,ef[bf].b,ef[bf].c,bn,bd);
        if (nv >= EPA_MAX_VERTS) break;

        ev[nv].v=sup; ev[nv].wA=wA; ev[nv].wB=wB;
        int edges[EPA_MAX_EDGES][2],ne=0,keep[EPA_MAX_FACES],nk=0;
        for (int f=0;f<nf;f++) {
            if (V3_dot(ef[f].n, V3_AsubB(sup,ev[ef[f].a].v)) > 0.f) {
                int fv[3]={ef[f].a,ef[f].b,ef[f].c};
                for (int e=0;e<3;e++) { int ea=fv[e],eb=fv[(e+1)%3]; bool found=false;
                    for (int k=0;k<ne;k++) if(edges[k][0]==eb&&edges[k][1]==ea){edges[k][0]=edges[--ne][0];edges[k][1]=edges[ne][1];found=true;break;}
                    if (!found&&ne<EPA_MAX_EDGES){edges[ne][0]=ea;edges[ne++][1]=eb;} }
            } else keep[nk++]=f;
        }
        nf=0; for(int k=0;k<nk;k++) ef[nf++]=ef[keep[k]];
        for(int k=0;k<ne&&nf<EPA_MAX_FACES;k++){EPAFace face=MakeEPAFace(ev,edges[k][0],edges[k][1],nv);if(face.d>=0.f)ef[nf++]=face;}
        nv++;
    }
    return m;
}

static Manifold CapCvx(ShapeCapsule cap, u16 mesh, const float* mx) {
    Manifold m = {0}; if (mesh >= MAX_MDLS || !modelVertexCounts[mesh]) return m;
    #define CSUP_A(d) CapsuleSupport(cap, d)
    #define CSUP_B(d) MeshSupport(mesh, mx, (V3){-(d).x,-(d).y,-(d).z})
    #define CSUP(d)   V3_AsubB(CSUP_A(d), CSUP_B(d))
    Simplex3D s = {0}; V3 dir = {0,1,0}; s.v[s.n++] = CSUP(dir); dir = (V3){-s.v[0].x,-s.v[0].y,-s.v[0].z}; if(V3_dot(dir,dir) < PHY_EPSILON){dir=(V3){0,1,0};} bool hit=false;
    for (int it=0;it<64;++it) { V3 sup=CSUP(dir); if(V3_dot(sup,dir)<0){return m;} s.v[s.n++]=sup; if(!GJKNextSimplex(&s,&dir)){hit=true; break;} if(V3_dot(dir,dir)<0){hit=true; break;} }
    if (!hit) return m;

    static const V3 kAx[6]={{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
    for (int d=0;s.n<4&&d<6;++d) { V3 sup=CSUP(kAx[d]); bool dup=false; for(int k=0;k<s.n;k++){V3 dv=V3_AsubB(sup,s.v[k]);dup|=(V3_dot(dv,dv)<PHY_EPSILON*PHY_EPSILON);} if(!dup){s.v[s.n++]=sup;} }
    if (s.n<4) return m;

    EPAVert ev[EPA_MAX_VERTS]; EPAFace ef[EPA_MAX_FACES]; int nv=0,nf=0;
    for (int i=0;i<4;i++){ev[nv].wA=CSUP_A(s.v[i]);ev[nv].wB=CSUP_B(s.v[i]);ev[nv].v=s.v[i];nv++;}
    static const int kTF[4][3]={{0,1,2},{0,3,1},{0,2,3},{1,3,2}};
    for (int f=0;f<4;f++){EPAFace face=MakeEPAFace(ev,kTF[f][0],kTF[f][1],kTF[f][2]);if(face.d>=0.f&&nf<EPA_MAX_FACES)ef[nf++]=face;}
    for (int it=0;it<32;++it){
        int bf=-1; float bd=1e9f;
        for (int f=0;f<nf;f++) if(ef[f].d<bd){bd=ef[f].d;bf=f;}
        if (bf<0) break;
        V3 bn=ef[bf].n; V3 wA=CSUP_A(bn), wB=CSUP_B(bn); V3 sup=V3_AsubB(wA,wB); if(V3_dot(bn,sup)-bd<PHY_EPSILON){return MakeEPAManifold(ev,ef[bf].a,ef[bf].b,ef[bf].c,bn,bd);}
        if (nv>=EPA_MAX_VERTS) break;

        ev[nv].v=sup; ev[nv].wA=wA; ev[nv].wB=wB;
        int edges[EPA_MAX_EDGES][2],ne=0,keep[EPA_MAX_FACES],nk=0;
        for (int f=0;f<nf;f++){
            if (V3_dot(ef[f].n,V3_AsubB(sup,ev[ef[f].a].v))>0.f){
                int fv[3]={ef[f].a,ef[f].b,ef[f].c};
                for (int e=0;e<3;e++){int ea=fv[e],eb=fv[(e+1)%3];bool found=false;
                    for (int k=0;k<ne;k++)if(edges[k][0]==eb&&edges[k][1]==ea){edges[k][0]=edges[--ne][0];edges[k][1]=edges[ne][1];found=true;break;}
                    if (!found&&ne<EPA_MAX_EDGES){edges[ne][0]=ea;edges[ne++][1]=eb;}}
            } else keep[nk++]=f;
        }
        nf=0;for(int k=0;k<nk;k++)ef[nf++]=ef[keep[k]];
        for(int k=0;k<ne&&nf<EPA_MAX_FACES;k++){EPAFace face=MakeEPAFace(ev,edges[k][0],edges[k][1],nv);if(face.d>=0.f)ef[nf++]=face;}
        nv++;
    }
    #undef CSUP_A
    #undef CSUP_B
    #undef CSUP
    return m;
}

static Manifold BoxCvx(ShapeBox box, u16 mesh, const float* mx) {
    Manifold m = {0}; if (mesh >= MAX_MDLS || !modelVertexCounts[mesh]) return m;
    #define BSUP(d) V3_AsubB(BoxSupport(box,d),MeshSupport(mesh,mx,(V3){-(d).x,-(d).y,-(d).z}))
    Simplex3D s={0}; V3 dir={0,1,0};
    s.v[s.n++]=BSUP(dir); dir=(V3){-s.v[0].x,-s.v[0].y,-s.v[0].z};
    if (V3_dot(dir,dir)<PHY_EPSILON) dir=(V3){0,1,0};
    bool hit=false;
    for (int it=0;it<64;++it){ V3 sup=BSUP(dir); if(V3_dot(sup,dir)<0){return m;} s.v[s.n++]=sup; if (!GJKNextSimplex(&s,&dir)){hit=true;break;} if (V3_dot(dir,dir)<0){hit=true;break;} }
    if (!hit) return m;
    static const V3 kAx[6]={{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
    for (int d=0;s.n<4&&d<6;++d){ V3 sup=BSUP(kAx[d]); bool dup=false; for (int k=0;k<s.n;k++){V3 dv=V3_AsubB(sup,s.v[k]);dup|=(V3_dot(dv,dv)<PHY_EPSILON*PHY_EPSILON);} if(!dup){s.v[s.n++]=sup;} }
    if (s.n<4) return m;
    EPAVert ev[EPA_MAX_VERTS]; EPAFace ef[EPA_MAX_FACES]; int nv=0,nf=0;
    for (int i=0;i<4;i++){ev[nv].wA=BoxSupport(box,s.v[i]);ev[nv].wB=MeshSupport(mesh,mx,(V3){-s.v[i].x,-s.v[i].y,-s.v[i].z});ev[nv].v=s.v[i];nv++;}
    static const int kTF[4][3]={{0,1,2},{0,3,1},{0,2,3},{1,3,2}};
    for (int f=0;f<4;f++){EPAFace face=MakeEPAFace(ev,kTF[f][0],kTF[f][1],kTF[f][2]);if(face.d>=0.f&&nf<EPA_MAX_FACES)ef[nf++]=face;}
    for (int it=0;it<32;++it){
        int bf=-1; float bd=1e9f;
        for (int f=0;f<nf;f++) if(ef[f].d<bd){bd=ef[f].d;bf=f;}
        if (bf<0) break;

        V3 bn=ef[bf].n; V3 wA=BoxSupport(box,bn); V3 wB=MeshSupport(mesh,mx,(V3){-bn.x,-bn.y,-bn.z}); V3 sup=V3_AsubB(wA,wB);
        if (V3_dot(bn,sup)-bd<0) return MakeEPAManifold(ev,ef[bf].a,ef[bf].b,ef[bf].c,bn,bd);
        if (nv>=EPA_MAX_VERTS) break;

        ev[nv].v=sup; ev[nv].wA=wA; ev[nv].wB=wB; int edges[EPA_MAX_EDGES][2],ne=0,keep[EPA_MAX_FACES],nk=0;
        for (int f=0;f<nf;f++){
            if (V3_dot(ef[f].n,V3_AsubB(sup,ev[ef[f].a].v))>0.f){
                int fv[3]={ef[f].a,ef[f].b,ef[f].c};
                for (int e=0;e<3;e++){int ea=fv[e],eb=fv[(e+1)%3];bool found=false;
                    for (int k=0;k<ne;k++)if(edges[k][0]==eb&&edges[k][1]==ea){edges[k][0]=edges[--ne][0];edges[k][1]=edges[ne][1];found=true;break;}
                    if (!found&&ne<EPA_MAX_EDGES){edges[ne][0]=ea;edges[ne++][1]=eb;}}
            } else keep[nk++]=f;
        }
        nf=0;for(int k=0;k<nk;k++)ef[nf++]=ef[keep[k]];
        for(int k=0;k<ne&&nf<EPA_MAX_FACES;k++){EPAFace face=MakeEPAFace(ev,edges[k][0],edges[k][1],nv);if(face.d>=0.f)ef[nf++]=face;}
        nv++;
    }
    #undef BSUP
    return m;
}

static Manifold SphCvx(V3 sc, float sr, u16 mesh, const float* mx) {
    Manifold m = {0}; if (mesh >= MAX_MDLS || !modelVertexCounts[mesh]) return m;
    #define SPHSUP_A(d) ({ V3 _d=(d); float _L=V3_dot(_d,_d); V3_AplusB(sc,(_L>PHY_EPSILON)?V3_ScaleByF(_d,sr/vsqrtf(_L)):(V3){0,sr,0}); })
    #define SPHSUP_B(d) MeshSupport(mesh,mx,(V3){-(d).x,-(d).y,-(d).z})
    #define MSKSUP(d)   V3_AsubB(SPHSUP_A(d),SPHSUP_B(d))
    Simplex3D s={0}; V3 dir={0,1,0}; s.v[s.n++]=MSKSUP(dir); dir=(V3){-s.v[0].x,-s.v[0].y,-s.v[0].z}; if(V3_dot(dir,dir)<PHY_EPSILON){dir=(V3){0,1,0};} bool hit=false;
    for (int it=0;it<32;++it) { V3 sup=MSKSUP(dir); if(V3_dot(sup,dir)<PHY_EPSILON){return m;} s.v[s.n++]=sup; if (!GJKNextSimplex(&s,&dir)){hit=true;break;} if (V3_dot(dir,dir)<PHY_EPSILON){hit=true;break;} }
    if (!hit) return m;

    static const V3 kAx[6]={{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
    for (int d=0;s.n<4&&d<6;++d) { V3 sup=MSKSUP(kAx[d]); bool dup=false; for (int k=0;k<s.n;k++){V3 dv=V3_AsubB(sup,s.v[k]);dup|=V3_dot(dv,dv)<PHY_EPSILON*PHY_EPSILON;} if(!dup){s.v[s.n++]=sup;} }
    if (s.n<4) return m;

    EPAVert ev[EPA_MAX_VERTS]; EPAFace ef[EPA_MAX_FACES]; int nv=0,nf=0;
    for (int i=0;i<4;i++){ev[nv].v=s.v[i];ev[nv].wA=SPHSUP_A(s.v[i]);ev[nv].wB=SPHSUP_B(s.v[i]);nv++;}
    static const int kTF[4][3]={{0,1,2},{0,3,1},{0,2,3},{1,3,2}};
    for (int f=0;f<4;f++){EPAFace face=MakeEPAFace(ev,kTF[f][0],kTF[f][1],kTF[f][2]);if(face.d>=0.f&&nf<EPA_MAX_FACES)ef[nf++]=face;}
    for (int it=0;it<32;++it){
        int bf=-1; float bd=1e9f;
        for (int f=0;f<nf;f++) if(ef[f].d<bd){bd=ef[f].d;bf=f;}
        if (bf<0) break;
        V3 bn=ef[bf].n; V3 wA=SPHSUP_A(bn); V3 wB=SPHSUP_B(bn); V3 sup=V3_AsubB(wA,wB);
        if (V3_dot(bn,sup)-bd<0) { m.normal=bn; m.maxPen=bd; m.n=1; m.p[0]=(ManifoldPt){wB,bd}; return m; }
        if (nv>=EPA_MAX_VERTS) break;

        ev[nv].v=sup;ev[nv].wA=wA;ev[nv].wB=wB;
        int edges[EPA_MAX_EDGES][2],ne=0,keep[EPA_MAX_FACES],nk=0;
        for (int f=0;f<nf;f++){
            if (V3_dot(ef[f].n,V3_AsubB(sup,ev[ef[f].a].v))>0.f){
                int fv[3]={ef[f].a,ef[f].b,ef[f].c};
                for (int e=0;e<3;e++){int ea=fv[e],eb=fv[(e+1)%3];bool found=false;
                    for (int k=0;k<ne;k++)if(edges[k][0]==eb&&edges[k][1]==ea){edges[k][0]=edges[--ne][0];edges[k][1]=edges[ne][1];found=true;break;}
                    if (!found&&ne<EPA_MAX_EDGES){edges[ne][0]=ea;edges[ne++][1]=eb;}}
            } else keep[nk++]=f;
        }
        nf=0;for(int k=0;k<nk;k++)ef[nf++]=ef[keep[k]];
        for(int k=0;k<ne&&nf<EPA_MAX_FACES;k++){EPAFace face=MakeEPAFace(ev,edges[k][0],edges[k][1],nv);if(face.d>=0.f)ef[nf++]=face;}
        nv++;
    }
    #undef SPHSUP_A
    #undef SPHSUP_B
    #undef MSKSUP
    return m;
}

typedef struct {V3 mn,mx;} AABB3;
static inline V3 TriSupport(V3 ta, V3 tb, V3 tc, V3 d) { float d1 = V3_dot(ta,d), d2 = V3_dot(tb,d), d3 = V3_dot(tc,d); return d1 > d2 ? (d1 > d3 ? ta : tc) : (d2 > d3 ? tb : tc); }
static Manifold CvxMsh(u16 hullMesh, const float* hullMx, u16 triMesh, const float* triMx) {
    Manifold best = {0}; if (hullMesh >= MAX_MDLS || triMesh >= MAX_MDLS) {return best;}
    u32 hn = modelVertexCounts[hullMesh]; if (!hn) return best;
    bool cached = hn <= CVXMSH_HULL_CACHE; // falls back to uncached path if a hull is ever bigger than this — never wrong, just slower
    V3 hv[CVXMSH_HULL_CACHE];
    AABB3 hb = { {1e9f,1e9f,1e9f}, {-1e9f,-1e9f,-1e9f} };
    const u8* vb = modelVertices[hullMesh];
    for (u32 i=0;i<hn;++i) {
        const u8* p = vb + i*VRT_ATT_SZ; V3 w = MvVert(hullMx,(V3){half_to_float(*(half*)(p+0)),half_to_float(*(half*)(p+2)),half_to_float(*(half*)(p+4))}); if (cached) hv[i]=w;
        hb.mn.x=vmin(hb.mn.x,w.x); hb.mn.y=vmin(hb.mn.y,w.y); hb.mn.z=vmin(hb.mn.z,w.z); hb.mx.x=vmax(hb.mx.x,w.x); hb.mx.y=vmax(hb.mx.y,w.y); hb.mx.z=vmax(hb.mx.z,w.z);
    }
    V3 hext = V3_AsubB(hb.mx,hb.mn); float spreadEps = vmax(0.02f, vmax(hext.x,vmax(hext.y,hext.z)) * 0.15f); // min spacing between manifold points, scaled to hull size
    #define HSUP(d) (cached ? CachedSupport(hv,hn,(d)) : MeshSupport(hullMesh,hullMx,(d)))
    u32 triCount = modelTriangleCounts[triMesh]; if (!triCount) return best;
    for (u32 ti = 0; ti < triCount; ++ti) {
        u32 i0 = modelTriangles[triMesh][ti * 3 + 0], i1 = modelTriangles[triMesh][ti * 3 + 1], i2 = modelTriangles[triMesh][ti * 3 + 2];
        V3 ta = MvVert(triMx,(V3){ half_to_float(*(half*)(modelVertices[triMesh] + i0 * VRT_ATT_SZ + 0)), half_to_float(*(half*)(modelVertices[triMesh] + i0 * VRT_ATT_SZ + 2)), half_to_float(*(half*)(modelVertices[triMesh] + i0 * VRT_ATT_SZ + 4)) });
        V3 tb = MvVert(triMx,(V3){ half_to_float(*(half*)(modelVertices[triMesh] + i1 * VRT_ATT_SZ + 0)), half_to_float(*(half*)(modelVertices[triMesh] + i1 * VRT_ATT_SZ + 2)), half_to_float(*(half*)(modelVertices[triMesh] + i1 * VRT_ATT_SZ + 4)) });
        V3 tc = MvVert(triMx,(V3){ half_to_float(*(half*)(modelVertices[triMesh] + i2 * VRT_ATT_SZ + 0)), half_to_float(*(half*)(modelVertices[triMesh] + i2 * VRT_ATT_SZ + 2)), half_to_float(*(half*)(modelVertices[triMesh] + i2 * VRT_ATT_SZ + 4)) });
        if (vmin(ta.x,vmin(tb.x,tc.x)) > hb.mx.x || vmax(ta.x,vmax(tb.x,tc.x)) < hb.mn.x || vmin(ta.y,vmin(tb.y,tc.y)) > hb.mx.y || vmax(ta.y,vmax(tb.y,tc.y)) < hb.mn.y || vmin(ta.z,vmin(tb.z,tc.z)) > hb.mx.z || vmax(ta.z,vmax(tb.z,tc.z)) < hb.mn.z) continue;

        Simplex3D s={0}; V3 dir={0.0f,1.0f,0.0f}; V3 wA0=HSUP(dir); V3 wB0=TriSupport(ta,tb,tc,(V3){-dir.x,-dir.y,-dir.z}); s.v[s.n++]=V3_AsubB(wA0,wB0); dir=(V3){-s.v[0].x, -s.v[0].y, -s.v[0].z}; if(V3_dot(dir,dir) < PHY_EPSILON){dir=(V3){0.0f,1.0f,0.0f};}
        bool hit = false;
        for (int it=0;it<32;++it) {
            V3 wA=HSUP(dir), wB=TriSupport(ta,tb,tc,(V3){-dir.x,-dir.y,-dir.z});
            V3 sup=V3_AsubB(wA,wB);
            if(V3_dot(sup,dir) < 0){break;}

            s.v[s.n++]=sup;
            if(!GJKNextSimplex(&s,&dir)){hit=true; break;}
            if(V3_dot(dir,dir) < 0){hit=true; break;} }
        if (!hit) continue;
        while (s.n < 4) {
            V3 fallbackDir = {0.0f,1.0f,0.0f};
            if (s.n == 1) fallbackDir = (vabs(s.v[0].x) > 0.5f) ? (V3){0.0f, 1.0f, 0.0f} : (V3){1.0f, 0.0f, 0.0f};
            else if (s.n == 2) { V3 edge = V3_AsubB(s.v[1], s.v[0]); fallbackDir = V3_Cross(edge, (vabs(edge.x) > 0.5f) ? (V3){0.0f, 1.0f, 0.0f} : (V3){1.0f, 0.0f, 0.0f}); }
            else if (s.n == 3) { V3 e1 = V3_AsubB(s.v[1], s.v[0]); V3 e2 = V3_AsubB(s.v[2], s.v[0]); fallbackDir = V3_Cross(e1, e2); }
            float fLen = V3_Mag(fallbackDir);
            fallbackDir = (fLen > PHY_EPSILON) ? V3_ScaleByF(fallbackDir, 1.0f / fLen) : (V3){0.0f, 1.0f, 0.0f};
            V3 wA = HSUP(fallbackDir), wB = TriSupport(ta,tb,tc,(V3){-fallbackDir.x,-fallbackDir.y,-fallbackDir.z}); V3 sup = V3_AsubB(wA,wB); bool dup = false;
            for (int k = 0; k < s.n; k++) { V3 dv = V3_AsubB(sup, s.v[k]); dup |= (V3_dot(dv, dv) < PHY_EPSILON * PHY_EPSILON); }
            if (!dup) s.v[s.n++] = sup;
            else { fallbackDir = (V3){-fallbackDir.x, -fallbackDir.y, -fallbackDir.z}; wA = HSUP(fallbackDir); wB = TriSupport(ta, tb, tc, (V3){-fallbackDir.x, -fallbackDir.y, -fallbackDir.z}); s.v[s.n++] = V3_AsubB(wA, wB); }
        }

        EPAVert ev[EPA_MAX_VERTS]; EPAFace ef[EPA_MAX_FACES]; int nv = 0, nf = 0;
        ev[nv].v = s.v[0]; ev[nv].wA = HSUP(s.v[0]); ev[nv].wB = TriSupport(ta,tb,tc,(V3){-s.v[0].x,-s.v[0].y,-s.v[0].z}); nv++;
        ev[nv].v = s.v[1]; ev[nv].wA = HSUP(s.v[1]); ev[nv].wB = TriSupport(ta,tb,tc,(V3){-s.v[1].x,-s.v[1].y,-s.v[1].z}); nv++;
        ev[nv].v = s.v[2]; ev[nv].wA = HSUP(s.v[2]); ev[nv].wB = TriSupport(ta,tb,tc,(V3){-s.v[2].x,-s.v[2].y,-s.v[2].z}); nv++;
        ev[nv].v = s.v[3]; ev[nv].wA = HSUP(s.v[3]); ev[nv].wB = TriSupport(ta,tb,tc,(V3){-s.v[3].x,-s.v[3].y,-s.v[3].z}); nv++;
        static const int kTF[4][3] = {{0,1,2}, {0,3,1}, {0,2,3}, {1,3,2}};
        EPAFace face0 = MakeEPAFace(ev,kTF[0][0],kTF[0][1],kTF[0][2]); if (face0.d >= 0.0f && nf < EPA_MAX_FACES) ef[nf++] = face0;
        EPAFace face1 = MakeEPAFace(ev,kTF[1][0],kTF[1][1],kTF[1][2]); if (face1.d >= 0.0f && nf < EPA_MAX_FACES) ef[nf++] = face1;
        EPAFace face2 = MakeEPAFace(ev,kTF[2][0],kTF[2][1],kTF[2][2]); if (face2.d >= 0.0f && nf < EPA_MAX_FACES) ef[nf++] = face2;
        EPAFace face3 = MakeEPAFace(ev,kTF[3][0],kTF[3][1],kTF[3][2]); if (face3.d >= 0.0f && nf < EPA_MAX_FACES) ef[nf++] = face3;
        if (nf < 4) continue;
        bool tHit=false; V3 tN={0}; float tD=0; V3 tP={0};
        for (int it = 0; it < 32; ++it) {
            int bf = -1; float bd = 1e9f;
            for (int f = 0; f < nf; f++) { if (ef[f].d < bd) { bd = ef[f].d; bf = f; } }
            if (bf < 0) break;
            V3 bn = ef[bf].n; V3 wA = HSUP(bn); V3 wB = TriSupport(ta,tb,tc,(V3){-bn.x,-bn.y,-bn.z}); V3 sup = V3_AsubB(wA,wB);
            if (V3_dot(bn, sup) - bd < PHY_EPSILON) { tHit=true; tN=bn; tD=bd; tP=EPAContactPoint(ev,ef[bf].a,ef[bf].b,ef[bf].c); break; }
            if (nv >= EPA_MAX_VERTS) break;

            ev[nv].v = sup; ev[nv].wA = wA; ev[nv].wB = wB; int edges[EPA_MAX_EDGES][2], ne = 0, keep[EPA_MAX_FACES], nk = 0;
            for (int f=0;f<nf;f++) {
                if (V3_dot(ef[f].n,V3_AsubB(sup,ev[ef[f].a].v)) > 0.0f) {
                    int fv[3] = {ef[f].a, ef[f].b, ef[f].c};
                    for (int e = 0; e < 3; e++) {
                        int ea = fv[e], eb = fv[(e + 1) % 3]; bool found = false;
                        for (int k = 0; k < ne; k++) { if (edges[k][0] == eb && edges[k][1] == ea) { edges[k][0] = edges[--ne][0]; edges[k][1] = edges[ne][1]; found = true; break; } }
                        if (!found && ne < EPA_MAX_EDGES) { edges[ne][0] = ea; edges[ne++][1] = eb; }
                    }
                } else keep[nk++] = f;
            }
            nf = 0; for (int k = 0; k < nk; k++) ef[nf++] = ef[keep[k]];
            for (int k = 0; k < ne && nf < EPA_MAX_FACES; k++) { EPAFace face = MakeEPAFace(ev, edges[k][0], edges[k][1], nv); if (face.d >= 0.0f) ef[nf++] = face; }
            nv++;
        }

        if (tHit) {
            if (!best.n) { best.normal=tN; best.maxPen=tD; best.p[best.n++]=(ManifoldPt){tP,tD}; }
            else {
                float align = V3_dot(tN,best.normal);
                if (align > MANIFOLD_ALIGN_THRESHOLD) {
                    bool better = (tD > best.maxPen + MANIFOLD_TIE_MARGIN) || (vabs(tD-best.maxPen) <= MANIFOLD_TIE_MARGIN && V3_dot(tN,(V3){0,1,0}) > V3_dot(best.normal,(V3){0,1,0}));
                    if (better) { best.normal=tN; best.maxPen=tD; } bool spread=true;
                    for (int k=0;k<best.n;++k) { V3 dv=V3_AsubB(tP,best.p[k].point); if (V3_dot(dv,dv)<spreadEps*spreadEps) { spread=false; if (tD>best.p[k].pen) best.p[k].pen=tD; break; } }
                    if (spread && best.n<MANIFOLD_MAX) best.p[best.n++]=(ManifoldPt){tP,tD};
                } else if (tD > best.maxPen + MANIFOLD_TIE_MARGIN) { best.n=0; best.normal=tN; best.maxPen=tD; best.p[best.n++]=(ManifoldPt){tP,tD}; }
            }
        }
    }
    #undef HSUP
    return best;
}

AABB3 BoxWorldAABB(ShapeBox b) { V3 x,y,z; obb_axes(b.rot,&x,&y,&z); V3 hx=V3_ScaleByF(x,b.halfExtents.x), hy=V3_ScaleByF(y,b.halfExtents.y), hz=V3_ScaleByF(z,b.halfExtents.z); V3 e ={vabs(hx.x)+vabs(hy.x)+vabs(hz.x),vabs(hx.y)+vabs(hy.y)+vabs(hz.y),vabs(hx.z)+vabs(hy.z)+vabs(hz.z)}; return (AABB3){V3_AsubB(b.ctr,e),V3_AplusB(b.ctr,e)}; }
static OverlapResult BoxMsh(ShapeBox box, u16 triMesh, const float* triMx) {
    OverlapResult r={0}; if(triMesh >= MAX_MDLS){return r;} u32 triCount=modelTriangleCounts[triMesh]; if(!triCount){return r;}

    AABB3 hb = BoxWorldAABB(box); float skin = 0.02f; hb.mn.x-=skin; hb.mn.y-=skin; hb.mn.z-=skin; hb.mx.x+=skin; hb.mx.y+=skin; hb.mx.z+=skin; V3 ax,ay,az; obb_axes(box.rot,&ax,&ay,&az);
    V3 hx = V3_ScaleByF(ax, box.halfExtents.x), hy = V3_ScaleByF(ay,box.halfExtents.y), hz = V3_ScaleByF(az,box.halfExtents.z);
    V3 verts[8] = {V3_AplusB(V3_AplusB(V3_AplusB(box.ctr,hx),hy),hz), V3_AplusB(V3_AsubB(V3_AplusB(box.ctr,hx),hy),hz), V3_AplusB(V3_AplusB(V3_AsubB(box.ctr,hx),hy),hz), V3_AplusB(V3_AsubB(V3_AsubB(box.ctr,hx),hy),hz), V3_AsubB(V3_AplusB(V3_AplusB(box.ctr,hx),hy),hz), V3_AsubB(V3_AsubB(V3_AplusB(box.ctr,hx),hy),hz), V3_AsubB(V3_AplusB(V3_AsubB(box.ctr,hx),hy),hz), V3_AsubB(V3_AsubB(V3_AsubB(box.ctr,hx),hy),hz)};
    for (u32 vi=0;vi<8;++vi) {
        V3 wv = verts[vi];
        for (u32 ti=0;ti<triCount;++ti) {
            u32 i0 = modelTriangles[triMesh][ti*3+0], i1 = modelTriangles[triMesh][ti*3+1], i2 = modelTriangles[triMesh][ti*3+2];
            #define TRV(i) MvVert(triMx,(V3){half_to_float(*(half*)(modelVertices[triMesh]+(i)*VRT_ATT_SZ+0)),half_to_float(*(half*)(modelVertices[triMesh]+(i)*VRT_ATT_SZ+2)),half_to_float(*(half*)(modelVertices[triMesh]+(i)*VRT_ATT_SZ+4))})
            V3 ta=TRV(i0),tb=TRV(i1),tc=TRV(i2);
            #undef TRV
            if (vmin(ta.x,vmin(tb.x,tc.x))>hb.mx.x || vmax(ta.x,vmax(tb.x,tc.x))<hb.mn.x || vmin(ta.y,vmin(tb.y,tc.y))>hb.mx.y || vmax(ta.y,vmax(tb.y,tc.y))<hb.mn.y || vmin(ta.z,vmin(tb.z,tc.z))>hb.mx.z || vmax(ta.z,vmax(tb.z,tc.z))<hb.mn.z) continue;

            V3 ab=V3_AsubB(tb,ta), ac=V3_AsubB(tc,ta), ap=V3_AsubB(wv,ta); float d1=V3_dot(ab,ap), d2=V3_dot(ac,ap); V3 bp=V3_AsubB(wv,tb); float d3=V3_dot(ab,bp), d4=V3_dot(ac,bp); V3 cp=V3_AsubB(wv,tc); float d5=V3_dot(ab,cp), d6=V3_dot(ac,cp);
            float vc=d1*d4-d3*d2, vb_=d5*d2-d1*d6, va=d3*d6-d5*d4; V3 closest; bool inFace=false;
            if      (d1 <= 0.0f && d2 <= 0.0f) closest=ta;
            else if (d3 >= 0.0f && d4 <= d3)   closest=tb;
            else if (d6 >= 0.0f && d5 <= d6)   closest=tc;
            else if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f)           { float v=d1/(d1-d3); closest=V3_AplusB(ta,V3_ScaleByF(ab,v)); }
            else if (vb_ <= 0.0f && d2 >= 0.0f && d6 <= 0.0f)          { float w=d2/(d2-d6); closest=V3_AplusB(ta,V3_ScaleByF(ac,w)); }
            else if (va <= 0.0f && (d4-d3) >= 0.0f && (d5-d6) >= 0.0f) { float w=(d4-d3)/((d4-d3)+(d5-d6)); closest=V3_AplusB(tb,V3_ScaleByF(V3_AsubB(tc,tb),w)); }
            else { inFace=true; V3 n_=V3_Cross(ab,ac); float L=V3_Mag(n_); if(L<PHY_EPSILON) continue; n_=V3_ScaleByF(n_,1.f/L); closest=V3_AsubB(wv,V3_ScaleByF(n_,V3_dot(n_,ap))); }

            V3 delta=V3_AsubB(wv,closest); float dist2=V3_dot(delta,delta); float pen=0.f; V3 fn;
            if (inFace) { V3 n_=V3_Cross(ab,ac); float L=V3_Mag(n_); if(L<PHY_EPSILON){continue;} n_=V3_ScaleByF(n_,1.f/L); float sd=V3_dot(n_,ap); if(vabs(sd)<PHY_EPSILON){continue;} pen=vabs(sd); fn=(sd>0.f)?n_:(V3){-n_.x,-n_.y,-n_.z}; }
            else { if (dist2<PHY_EPSILON*PHY_EPSILON){continue;} float dist=vsqrtf(dist2); V3 n_=V3_Cross(ab,ac); float L=V3_Mag(n_); if(L<PHY_EPSILON){continue;} n_=V3_ScaleByF(n_,1.f/L); float sd=V3_dot(n_,ap); if (sd>=0.f){continue;} pen=dist; fn=V3_ScaleByF(delta,1.f/dist); }
            if (pen>r.overlapAmount) r=(OverlapResult){true,closest,fn,pen};
        }
    }
    return r;
}

static OverlapResult CapBox(ShapeCapsule cap, ShapeBox box) {
    OverlapResult best = SphBox(cap.base,cap.rad,box), rt = SphBox(cap.tip,cap.rad,box); if (rt.hit && rt.overlapAmount > best.overlapAmount) best = rt;
    V3 ax,ay,az; obb_axes(box.rot,&ax,&ay,&az); V3 d = V3_AsubB(cap.tip,cap.base); float segLen = V3_Mag(d); if (segLen < PHY_EPSILON) return best;

    V3 dUnit=V3_ScaleByF(d, 1.f / segLen); V3 toBase=V3_AsubB(cap.base,box.ctr); float ts[6]; int nt=0; float dax=V3_dot(dUnit,ax) * segLen, day=V3_dot(dUnit,ay) * segLen, daz=V3_dot(dUnit,az) * segLen; float bax=V3_dot(toBase,ax), bay=V3_dot(toBase,ay), baz=V3_dot(toBase,az);
    if (vabs(dax) > PHY_EPSILON) { float t0 = (-box.halfExtents.x - bax) / dax, t1 = (box.halfExtents.x - bax) / dax; if (t0 > t1) { float tmp = t0; t0 = t1; t1 = tmp; } t0 = vclamp(t0,0.0f,1.0f); t1 = vclamp(t1,0.0f,1.f); ts[nt++] = t0; ts[nt++] = t1; }
    if (vabs(day) > PHY_EPSILON) { float t0 = (-box.halfExtents.y - bay) / day, t1 = (box.halfExtents.y - bay) / day; if (t0 > t1) { float tmp = t0; t0 = t1; t1 = tmp; } t0 = vclamp(t0,0.0f,1.0f); t1 = vclamp(t1,0.0f,1.f); ts[nt++] = t0; ts[nt++] = t1; }
    if (vabs(daz) > PHY_EPSILON) { float t0 = (-box.halfExtents.z - baz) / daz, t1 = (box.halfExtents.z - baz) / daz; if (t0 > t1) { float tmp = t0; t0 = t1; t1 = tmp; } t0 = vclamp(t0,0.0f,1.0f); t1 = vclamp(t1,0.0f,1.f); ts[nt++] = t0; ts[nt++] = t1; }
    if (nt == 0) ts[nt++] = 0.5f; // Only add midpoint if no slab candidates were generated (fully degenerate segment)
    for (int k = 0; k < nt; k++) { V3 pt = V3_AplusB(cap.base,V3_ScaleByF(d,ts[k])); OverlapResult rk = SphBox(pt,cap.rad,box); if (rk.hit && rk.overlapAmount > best.overlapAmount) {best = rk;} }
    return best;
}

static float GetMomentOfInertia(Entity *e, V3 n) {
    if (e->collider != COLTYPE_CVX || !e->inertiaTensorValid) { float r = (e->collider == COLTYPE_MSH) ? modelBounds[e->modelIndex] : (e->collider == COLTYPE_CVX) ? modelBounds[e->colMeshIndex] : GetColRad(e); return (2.f / 5.f) * e->mass * r * r; }

    V3 ln = quat_rot_v3((Quaternion){-e->rotation.x,-e->rotation.y,-e->rotation.z,e->rotation.w}, n); // Rotate world space normal to local space
    V3 invIn = {e->invInertiaTensor[0]*ln.x + e->invInertiaTensor[3]*ln.y + e->invInertiaTensor[4]*ln.z,e->invInertiaTensor[3]*ln.x + e->invInertiaTensor[1]*ln.y + e->invInertiaTensor[5]*ln.z,e->invInertiaTensor[4]*ln.x + e->invInertiaTensor[5]*ln.y + e->invInertiaTensor[2]*ln.z};
    float invResult = vmax(V3_dot(ln, invIn), PHY_EPSILON);
    return 1.0f / invResult; // Return the inverted form so your external (1.0f / iA) yields the mathematically exact value!
}

static void ResolveContactVelocity(Entity *e, Entity *o, V3 n, V3 contactPoint, bool oStatic) {
    V3 rAarm = V3_AsubB(contactPoint,e->position);
    V3 rBarm = oStatic ? (V3){0,0,0} : V3_AsubB(contactPoint,o->position);
    V3 rAxN = V3_Cross(rAarm, n);
    float rAxN_lenSq = V3_dot(rAxN, rAxN);
    float iA = (rAxN_lenSq > PHY_EPSILON) ? GetMomentOfInertia(e,V3_ScaleByF(rAxN,1.0f / vsqrtf(rAxN_lenSq))) : 1.0f;
    float invIA = (iA > PHY_EPSILON) ? 1.0f / iA : 0;
    float angTermA = IdxIsNPC(e->index) ? 0 : rAxN_lenSq * invIA;
    V3 rBxN = V3_Cross(rBarm,n);
    float rBxN_lenSq = V3_dot(rBxN,rBxN);
    float iB = (oStatic || rBxN_lenSq < PHY_EPSILON) ? 0 : GetMomentOfInertia(o,V3_ScaleByF(rBxN, 1.0f / vsqrtf(rBxN_lenSq)));
    float invIB = (iB > PHY_EPSILON && !oStatic) ? 1.0f / iB : 0;
    float angTermB = (oStatic || IdxIsNPC(o->index)) ? 0 : rBxN_lenSq * invIB;
    V3 vAtA = V3_AplusB(e->velocity,V3_Cross(e->angularVelocity,rAarm));
    V3 vAtB = oStatic ? (V3){0,0,0} : V3_AplusB(o->velocity,V3_Cross(o->angularVelocity,rBarm));
    V3 relVel = V3_AsubB(vAtA,vAtB);
    float vn = V3_dot(relVel,n);
    if (vn > 0.01f) return;

    float invMassA = e->mass < 0.001f ? 1.0f : 1.0f / e->mass;
    float invMassB = oStatic || o->mass < 0.001f ? 0 : 1.0f / o->mass;
    float invSum = invMassA + invMassB + angTermA + angTermB;
    if (invSum < PHY_EPSILON) return;

    float e_r = (vn < -0.1) ? vmax(e->bounciness, oStatic ? 0 : o->bounciness) : 0;
    float j = -(1.0f + e_r) * vn / invSum; j = vmax(j,0);
                  e->velocity = V3_AplusB(e->velocity,V3_ScaleByF(n, j * invMassA));
    if (!oStatic) o->velocity =  V3_AsubB(o->velocity,V3_ScaleByF(n, j * invMassB));
    V3 Jn = V3_ScaleByF(n,j); // Angular effect
    if (            e->collider != COLTYPE_CAP && !IdxIsNPC(e->index)) e->angularVelocity = V3_AplusB(e->angularVelocity,V3_ScaleByF(V3_Cross(rAarm,Jn),invIA));
    if (!oStatic && o->collider != COLTYPE_CAP && !IdxIsNPC(o->index)) o->angularVelocity =  V3_AsubB(o->angularVelocity,V3_ScaleByF(V3_Cross(rBarm,Jn),invIB));
    V3 relVel2 = V3_AsubB(V3_AplusB(e->velocity,V3_Cross(e->angularVelocity,rAarm)), (oStatic ? (V3){0,0,0} : V3_AplusB(o->velocity,V3_Cross(o->angularVelocity,rBarm))));
    V3 tangent = V3_AsubB(relVel2,V3_ScaleByF(n,V3_dot(relVel2,n)));
    float tLen = V3_Mag(tangent);
    if (tLen > 0.0001f) { // Friction
        tangent = V3_ScaleByF(tangent,1.0f/tLen);
        V3 rAxT = V3_Cross(rAarm, tangent);
        float rAxT_lenSq = V3_dot(rAxT, rAxT);
        float iAT = (rAxT_lenSq > PHY_EPSILON) ? GetMomentOfInertia(e, V3_ScaleByF(rAxT, 1.0f / vsqrtf(rAxT_lenSq))) : 1.0f;
        float invIAT = (iAT > PHY_EPSILON) ? 1.0f / iAT : 0;
        float angTermAT = IdxIsNPC(e->index) ? 0 : rAxT_lenSq * invIAT;
        V3 rBxT = V3_Cross(rBarm, tangent);
        float rBxT_lenSq = V3_dot(rBxT, rBxT);
        float iBT = (oStatic || rBxT_lenSq < PHY_EPSILON) ? 0 : GetMomentOfInertia(o, V3_ScaleByF(rBxT, 1.0f / vsqrtf(rBxT_lenSq)));
        float invIBT = (iBT > PHY_EPSILON && !oStatic) ? 1.0f / iBT : 0;
        float angTermBT = (oStatic || IdxIsNPC(o->index)) ? 0 : rBxT_lenSq * invIBT;
        float invSumT = invMassA + invMassB + angTermAT + angTermBT;
        float mu = 0.05f;
        float jt = -V3_dot(relVel2, tangent) / invSumT;
        jt = vclamp(jt, mu * -vabs(j), mu * vabs(j));
        V3 Jt = V3_ScaleByF(tangent, jt);
        e->velocity = V3_AplusB(e->velocity, V3_ScaleByF(tangent, jt * invMassA));
        if (e->collider != COLTYPE_CAP && !IdxIsNPC(e->index)) e->angularVelocity = V3_AplusB(e->angularVelocity,V3_ScaleByF(V3_Cross(rAarm,Jt),invIAT));
        if (!oStatic) { o->velocity = V3_AsubB(o->velocity,V3_ScaleByF(tangent,jt * invMassB));
        if (o->collider != COLTYPE_CAP && !IdxIsNPC(o->index)) {o->angularVelocity = V3_AsubB(o->angularVelocity,V3_ScaleByF(V3_Cross(rBarm,Jt),invIBT));} }
    }

    e->lastAngularVelocity = e->angularVelocity;
}

static void ApplyPositionalCorrection(Entity *e, Entity *o, V3 n, const Manifold *m, bool oStatic) {
    float iMA = e->mass < 0.001f ? 1.0f : 1.0f / e->mass;
    float iMB = oStatic || o->mass < 0.001f ? 0.0f : 1.0f / o->mass;

    // Average the penetration depth across all valid active contact points
    float avgPen = 0.0f;
    for (int i = 0; i < m->n; ++i) {
        avgPen += m->p[i].pen;
    }
    if (m->n > 0) avgPen /= (float)m->n;

    // Use the averaged penetration instead of the extreme maxPen
    float c = vmin(vmax(avgPen - 0.001f, 0.0f) * 0.2f, 0.04f);
    float cA = c * iMA / (iMA + iMB + PHY_EPSILON);
    float cB = c * iMB / (iMA + iMB + PHY_EPSILON);

    SetPosition(e, V3_AplusB(e->position, V3_ScaleByF(n, cA)), false);
    if (!oStatic) SetPosition(o, V3_AsubB(o->position, V3_ScaleByF(n, cB)), false);
}

static void ApplyManifoldResponse(Entity *e, Entity *o, const Manifold *m) {
    if (!m->n) return;
    bool oStatic = (!(o->entflags & EF_RIGIDBODY) || o->mass < 0.001f || o->collider == COLTYPE_NONE || o->collider == COLTYPE_MSH);
    if (o->collider == COLTYPE_MSH && e->collider == COLTYPE_MSH) return;
    for (int i = 0; i < m->n; ++i) DrawSphereContact(m->p[i].point, 0.02f);
    int iters = (m->n > 1) ? MAX_COLLISION_ITERATIONS : 1; // single-point contacts converge in one pass — no need to pay for more
    for (int it = 0; it < iters; ++it) {
        for (int i = 0; i < m->n; ++i) ResolveContactVelocity(e,o,m->normal,m->p[i].point,oStatic);
    }

    ApplyPositionalCorrection(e,o,m->normal,m,oStatic);
}

void Physics() {
    if (World.gamePaused || World.menuActive) return;

    float dt = vclamp((float)(World.pauseRelativeTime - World.last_physics_time), 0.0005f, 0.1f);
    World.last_physics_time = World.pauseRelativeTime;
    u8 substeps = (u8)vclamp((u32)(dt / MAX_STEP_SIZE + 0.5f), 1u, (u32)MAX_SUBSTEPS);
    float dtsub = dt / (float)substeps; dynamicEntityCount = 0;
    for (int i = 0; i < World.instCount && dynamicEntityCount < 512; ++i) { Entity *e = &World.instances[i]; if ((e->entflags & EF_RIGIDBODY) && (e->entflags & EF_ACTIVE) && e->collider != COLTYPE_NONE) {dynamicEntities[dynamicEntityCount++] = i;} }
    for (int i = 0; i < World.instCount; ++i) { Entity *e = &World.instances[i]; e->cellX = (i16)PosGetCellCoordX(e->position.x); e->cellZ = (i16)PosGetCellCoordZ(e->position.z); e->cellIndex = PosGetCellCoordsP(e->cellX,e->cellZ); e->radius = (e->modelIndex < MAX_MDLS) ? modelBounds[e->modelIndex] * vmax(vmax(e->scale.x,e->scale.y),e->scale.z) : GetColRad(e); e->colliding = false; } // Update cell index for all entities
    for (u8 s = 0; s < substeps; ++s) { // dynamicEntityCount found to be only 335 on level 1
        mset(cellCounts,0,sizeof(cellCounts));
        for (u16 i = 0; i < World.instCount; ++i) { Entity *e = &World.instances[i]; u32 cell = (u32)e->cellIndex; if (cell < WORLDX*WORLDX && cellCounts[cell] < 127) cellLists[cell][cellCounts[cell]++] = i; } // Build broadphase grid (~0.013ms)
        for (u16 i = 0; i < dynamicEntityCount; ++i) { u16 idx = dynamicEntities[i]; Entity *e = &World.instances[idx]; ApplyVelocity(e,dtsub); } // Integrate all dynamic bodies (~0.005ms)
        ShapeSphere sa,sb; ShapeBox ba,bb; ShapeCapsule ca,cb;
        for (u16 i = 0; i < dynamicEntityCount; ++i) { // Collision detection and resolution (~32.9ms)
            u16 a = dynamicEntities[i]; Entity *e = &World.instances[a]; if (e->collider == COLTYPE_MSH || (Cheats.noclip && a == PLAYER1)) continue;

            i32 cx = PosGetCellCoordX(e->position.x);
            i32 cz = PosGetCellCoordZ(e->position.z);
            u32 mask = GetCollisionMask(e->layer);
            float searchRad = e->radius + V3_Mag(e->velocity) * dtsub + 0.5f;
            i32 radCells = (i32)(searchRad / CELL_SIZE) + 2;
            typedef struct { Manifold m; u16 otherIdx; } Contact;
            Contact contacts[16]; int contactCount = 0;
            for (i32 dx = -radCells; dx <= radCells; ++dx) {
                for (i32 dz = -radCells; dz <= radCells; ++dz) {
                    u32 cell = PosGetCellCoordsP(cx + dx,cz + dz); if (cell >= WORLDX*WORLDX) continue;
                    for (u16 k = 0; k < cellCounts[cell]; ++k) {
                        u16 b = cellLists[cell][k];
                        if (b == a) continue;

                        Entity *o = &World.instances[b];
                        if (!(mask & o->layer) || o->collider == COLTYPE_NONE) continue;
                        if (b < a && o->collider != COLTYPE_MSH && (o->entflags & EF_RIGIDBODY) && (o->entflags & EF_ACTIVE) && !(Cheats.noclip && b == PLAYER1)) continue;

                        V3 deltaPos = V3_AsubB(e->position,o->position);
                        float distSq = V3_dot(deltaPos,deltaPos), combinedRadius = e->radius * 2.f + o->radius * 2.f;
                        if (distSq > combinedRadius * combinedRadius) continue;

                        Manifold mf = {0};
                        if      (e->collider == COLTYPE_CAP && o->collider == COLTYPE_CAP) { Entity_GetCap(e,&ca); Entity_GetCap(o,&cb); mf=OverlapToManifold(CapCap(ca,cb)); }
                        else if (e->collider == COLTYPE_CAP && o->collider == COLTYPE_BOX) { Entity_GetCap(e,&ca); Entity_GetBox(o,&bb); mf=OverlapToManifold(CapBox(ca,bb)); }
                        else if (e->collider == COLTYPE_BOX && o->collider == COLTYPE_CAP) { Entity_GetBox(e,&bb); Entity_GetCap(o,&ca); OverlapResult r=CapBox(ca,bb); if(r.hit) r.normal=V3_ScaleByF(r.normal,-1.0f); mf=OverlapToManifold(r); }
                        else if (e->collider == COLTYPE_BOX && o->collider == COLTYPE_BOX) { Entity_GetBox(e,&ba); Entity_GetBox(o,&bb); mf=OverlapToManifold(BoxBox(ba,bb)); }
                        else if (e->collider == COLTYPE_SPH && o->collider == COLTYPE_BOX) { Entity_GetSph(e,&sa); Entity_GetBox(o,&ba); mf=OverlapToManifold(SphBox(sa.ctr,sa.rad,ba)); }
                        else if (e->collider == COLTYPE_BOX && o->collider == COLTYPE_SPH) { Entity_GetBox(e,&ba); Entity_GetSph(o,&sa); OverlapResult r=SphBox(sa.ctr,sa.rad,ba); if(r.hit) r.normal=V3_ScaleByF(r.normal,-1.0f); mf=OverlapToManifold(r); }
                        else if (e->collider == COLTYPE_SPH && o->collider == COLTYPE_SPH) { Entity_GetSph(e,&sa); Entity_GetSph(o,&sb); mf=OverlapToManifold(SphSph(sa.ctr,sa.rad,sb.ctr,sb.rad)); }
                        else if (e->collider == COLTYPE_CAP && o->collider == COLTYPE_MSH) { Entity_GetCap(e,&ca); mf=OverlapToManifold(CapMsh(ca,o->modelIndex,&modelMatrices[b*16])); }
                        else if (e->collider == COLTYPE_SPH && o->collider == COLTYPE_MSH) { Entity_GetSph(e,&sa); mf=OverlapToManifold(SphMsh(sa.ctr,sa.rad,o->modelIndex,&modelMatrices[b*16])); }
                        else if (e->collider == COLTYPE_BOX && o->collider == COLTYPE_MSH) { Entity_GetBox(e,&ba); mf=OverlapToManifold(BoxMsh(ba,o->modelIndex,&modelMatrices[b*16])); }
                        else if (e->collider == COLTYPE_CVX && o->collider == COLTYPE_MSH) { mf=CvxMsh(e->colMeshIndex,&modelMatrices[a*16],o->modelIndex,&modelMatrices[b*16]); if(mf.n) mf.normal=V3_ScaleByF(mf.normal,-1.0f); }
                        else if (e->collider == COLTYPE_CAP && o->collider == COLTYPE_CVX) { Entity_GetCap(e,&ca); mf=CapCvx(ca,o->colMeshIndex,&modelMatrices[b*16]); if(mf.n) mf.normal=V3_ScaleByF(mf.normal,-1.0f); }
                        else if (e->collider == COLTYPE_CVX && o->collider == COLTYPE_CAP) { Entity_GetCap(o,&cb); mf=CapCvx(cb,e->colMeshIndex,&modelMatrices[a*16]); }
                        else if (e->collider == COLTYPE_SPH && o->collider == COLTYPE_CVX) { Entity_GetSph(e,&sa); mf=SphCvx(sa.ctr,sa.rad,o->colMeshIndex,&modelMatrices[b*16]); }
                        else if (e->collider == COLTYPE_CVX && o->collider == COLTYPE_SPH) { Entity_GetSph(o,&sb); mf=SphCvx(sb.ctr,sb.rad,e->colMeshIndex,&modelMatrices[a*16]); if(mf.n) mf.normal=V3_ScaleByF(mf.normal,-1.0f); }
                        else if (e->collider == COLTYPE_BOX && o->collider == COLTYPE_CVX) { Entity_GetBox(e,&ba); mf=BoxCvx(ba,o->colMeshIndex,&modelMatrices[b*16]); if(mf.n) mf.normal=V3_ScaleByF(mf.normal,-1.0f); }
                        else if (e->collider == COLTYPE_CVX && o->collider == COLTYPE_BOX) { Entity_GetBox(o,&bb); mf=BoxCvx(bb,e->colMeshIndex,&modelMatrices[a*16]); }
                        else if (e->collider == COLTYPE_CVX && o->collider == COLTYPE_CVX) { mf=CvxCvx(e->colMeshIndex,o->colMeshIndex,&modelMatrices[a*16],&modelMatrices[b*16]); if(mf.n) mf.normal=V3_ScaleByF(mf.normal,-1.0f); }
                        else { mf=OverlapToManifold(SphSph(e->position,GetColRad(e),o->position,GetColRad(o))); }

                        if (mf.n && contactCount < 16) contacts[contactCount++] = (Contact){mf,b};
                    }
                }
            }

            for (int cta=0;cta<contactCount-1;++cta) {
                for (int ctb=cta+1;ctb<contactCount;++ctb) {
                    if (contacts[ctb].m.maxPen > contacts[cta].m.maxPen) { Contact tmp=contacts[cta]; contacts[cta]=contacts[ctb]; contacts[ctb]=tmp; }
                }
            }

            for (int c = 0; c < contactCount; ++c) {
                Manifold *mfp = &contacts[c].m; u16 j = contacts[c].otherIdx;
                Entity *o = (j < INSTANCE_COUNT) ? &World.instances[j] : NULL; e->colliding = true; if (o) o->colliding = true;
                if (o && (o->entflags & EF_RIGIDBODY) && o->collider != COLTYPE_MSH) ApplyManifoldResponse(e,o,mfp);
                else { Entity staticProxy = {0}; staticProxy.mass=0.0f; staticProxy.dynamicFriction=0.4f; staticProxy.collider=COLTYPE_NONE; ApplyManifoldResponse(e,&staticProxy,mfp); }
            }

            e->accumulatedForce = (V3){0.0f,0.0f,0.0f};
        }
    }

    for (int i = 0; i < World.instCount; ++i) { Entity *e = &World.instances[i]; e->cellX = (i16)PosGetCellCoordX(e->position.x); e->cellZ = (i16)PosGetCellCoordZ(e->position.z); e->cellIndex = PosGetCellCoordsP(e->cellX, e->cellZ); } // Update cells for next substep
}

ENGINE_TO_MOD void AddForce(u16 i, V3 f, bool imp) { Entity *e = &World.instances[i]; if (imp) { e->velocity = V3_AplusB(e->velocity,V3_ScaleByF(f,1.0f / vmax(e->mass,0.001f))); } else { e->accumulatedForce = V3_AplusB(e->accumulatedForce,f); } }
ENGINE_TO_MOD void ApplyPlayerMovements() {
    float h = (float)Forward() - (float)Backpedal(), s = (float)StrafeRight() - (float)StrafeLeft();
    float vertInput = (float)SwimUp() - (float)SwimDn();
    Entity *p = &World.instances[PLAYER1];
    Quaternion r = p->rotation; float y2 = r.y*r.y; float xz = r.x*r.z; float wy = r.w*r.y;
    p->forward=V3_Normalize((V3){ 2.0f * (xz + wy), 2.0f * (r.y*r.z - r.w*r.x), 1.0f - 2.0f * (r.x*r.x + y2) }); p->right=V3_Normalize((V3){ 1.0f - 2.0f * (y2 + r.z*r.z), 2.0f * (r.x*r.y + r.w*r.z), 2.0f * (xz - wy) });
    V3 w = V3_Normalize((V3){p->forward.x*h + p->right.x*s,vertInput,p->forward.z*h + p->right.z*s});
    float speed = GetBasePlayerSpeed(PLAYER1,V3_Mag(w) > 0.1f) * 1.75f, accel = World.boosterActive ? 1.0f : 3.0f;
    V3 targetVel = V3_ScaleByF(w,speed); if (vabs(vertInput) < 0.001f) targetVel.y = p->velocity.y;
    V3 dv = V3_AsubB(targetVel,p->velocity);
    dv = (V3){vclamp(dv.x,-10.0f,10.0f), vclamp(dv.y,-10.0f,10.0f), vclamp(dv.z,-10.0f,10.0f)};
    p->velocity = V3_AplusB(p->velocity,V3_ScaleByF(dv,accel * vclamp((float)World.deltaTime,0.0005f,0.1f)));
}
