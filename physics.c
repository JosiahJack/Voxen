// physics.c - Physics
#include "voxen.h"
#include <malloc.h>
typedef __builtin_va_list va_list;
extern u16 loadedModelsMaxIndex; extern float modelBounds[MODEL_IDX_MAX]; extern u8** modelVertices; extern u16** modelTriangles;
extern u32 modelVertexCounts[MODEL_IDX_MAX]; extern u16 modelTriangleCounts[MODEL_IDX_MAX]; extern float modelMatrices[INSTANCE_COUNT * 16]; extern u32 gridCellStates[ARRSIZE];

static inline Vector3 ClosestPointOnSegment(Vector3 p, Vector3 q, Vector3 a) {
    Vector3 pq  = Vector3_A_minus_B(q, p);
    Vector3 pa  = Vector3_A_minus_B(a, p);
    float   len2 = dot_vector3(pq, pq);
    if (len2 < 1e-10f) return p;
    float   t    = dot_vector3(pa, pq) / len2;
    t = vmax(0.0f, vmin(1.0f, t));
    return Vector3_A_plus_B(p, scale_vector3(pq, t));
}
 
static inline Vector3 ClosestPointOnTriangle(Vector3 a, Vector3 b, Vector3 c, Vector3 p) {
    Vector3 ab = Vector3_A_minus_B(b,a), ac = Vector3_A_minus_B(c,a), ap = Vector3_A_minus_B(p,a);
    float d1 = dot_vector3(ab,ap), d2 = dot_vector3(ac,ap);
    if (d1 <= 0.0f && d2 <= 0.0f) return a;
 
    Vector3 bp = Vector3_A_minus_B(p, b);
    float d3 = dot_vector3(ab, bp), d4 = dot_vector3(ac, bp);
    if (d3 >= 0.0f && d4 <= d3) return b;
 
    Vector3 cp = Vector3_A_minus_B(p, c);
    float d5 = dot_vector3(ab, cp), d6 = dot_vector3(ac, cp);
    if (d6 >= 0.0f && d5 <= d6) return c;
 
    float vc = d1 * d4 - d3 * d2;
    if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f) {
        float v = d1 / (d1 - d3);
        return Vector3_A_plus_B(a, scale_vector3(ab, v));
    }
 
    float vb = d5 * d2 - d1 * d6;
    if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) {
        float w = d2 / (d2 - d6);
        return Vector3_A_plus_B(a, scale_vector3(ac, w));
    }
 
    float va = d3 * d6 - d5 * d4;
    if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f) {
        float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
        return Vector3_A_plus_B(b, scale_vector3(Vector3_A_minus_B(c, b), w));
    }
 
    float denom = 1.0f / (va + vb + vc);
    float v = vb * denom;
    float w = vc * denom;
    Vector3 result = Vector3_A_plus_B(a, scale_vector3(ab, v));
    result = Vector3_A_plus_B(result, scale_vector3(ac, w));
    return result;
}
 
static u32 GetCollisionMask(u32 layer) {
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
 
// ─── contact ──────────────────────────────────────────────────────────────────
#define SOLVER_ITERATIONS     6
#define SLEEP_KE_THRESHOLD    0.004f
#define SLEEP_FRAMES_NEEDED   20
#define SPECULATIVE_MARGIN    0.005f
#define BAUMGARTE_FACTOR      0.2f
#define BAUMGARTE_SLOP        0.002f
#define SUB_STEP_DT_MAX       0.027777778f
#define RESTITUTION_THRESHOLD 1.8f
typedef struct { Vector3 center; Vector3 halfExtents; Quaternion rot; } ShapeBox;
typedef struct { Vector3 center; float radius; }                        ShapeSphere;
typedef struct { Vector3 tip,base; float radius; }                      ShapeCapsule;
 
typedef struct { Vector3 center; Vector3 halfExtents; Quaternion rot; } OBB;
static inline void obb_axes(Quaternion q,Vector3 *ax,Vector3 *ay,Vector3 *az) { *ax = quat_rotate_vector(q,(Vector3){1,0,0}); *ay = quat_rotate_vector(q,(Vector3){0,1,0}); *az = quat_rotate_vector(q,(Vector3){0,0,1}); }
static inline float obb_project(ShapeBox b,Vector3 axis) {
    Vector3 ax,ay,az; obb_axes(b.rot,&ax,&ay,&az);
    return b.halfExtents.x*vabs(dot_vector3(ax,axis)) + b.halfExtents.y*vabs(dot_vector3(ay,axis)) + b.halfExtents.z*vabs(dot_vector3(az,axis));
}
static inline Vector3 closest_point_obb(ShapeBox box,Vector3 p) {
    Vector3 ax,ay,az; obb_axes(box.rot,&ax,&ay,&az);
    Vector3 d = Vector3_A_minus_B(p,box.center);
    float tx=vclamp(dot_vector3(d,ax),-box.halfExtents.x,box.halfExtents.x),ty=vclamp(dot_vector3(d,ay),-box.halfExtents.y,box.halfExtents.y),tz=vclamp(dot_vector3(d,az),-box.halfExtents.z,box.halfExtents.z);
    return Vector3_A_plus_B(box.center,Vector3_A_plus_B(Vector3_A_plus_B(scale_vector3(ax,tx),scale_vector3(ay,ty)),scale_vector3(az,tz)));
}
static inline float seg_seg_closest(Vector3 p,Vector3 q,Vector3 a,Vector3 b, Vector3 *cpPQ,Vector3 *cpAB) {
    Vector3 d1=Vector3_A_minus_B(q,p), d2=Vector3_A_minus_B(b,a), r=Vector3_A_minus_B(p,a);
    float e=dot_vector3(d2,d2), f=dot_vector3(d2,r);
    float s,t;
    float c=dot_vector3(d1,d1);
    if (c<1e-10f && e<1e-10f) { s=t=0.0f; *cpPQ=p; *cpAB=a; goto done; }
    if (c<1e-10f) { s=0.0f; t=vclamp(f/e,0.0f,1.0f); }
    else {
        float dv=dot_vector3(d1,d2), denom=c*e-dv*dv;
        s = (denom>1e-10f) ? vclamp((dv*f-dot_vector3(d1,r)*e)/(denom),0.0f,1.0f) : 0.0f;
        t = (dv*s+f)/e;
        if (t<0.0f) { t=0.0f; s=vclamp(-dot_vector3(d1,r)/c,0.0f,1.0f); }
        else if (t>1.0f) { t=1.0f; s=vclamp((dv-dot_vector3(d1,r))/c,0.0f,1.0f); }
    }
    *cpPQ = Vector3_A_plus_B(p,scale_vector3(d1,s));
    *cpAB = Vector3_A_plus_B(a,scale_vector3(d2,t));
done:;
    Vector3 w = Vector3_A_minus_B(*cpPQ,*cpAB);
    return dot_vector3(w,w);
}
 
typedef struct {
    Vector3 pointWorld;
    Vector3 normal;
    float   depth;
    float   lambdaN;
    float   lambdaT;
} Contact;
 
#define MAX_CONTACTS_PER_PAIR 4
typedef struct {
    u16 idxA,idxB;
    u8  count;
    Contact  contacts[MAX_CONTACTS_PER_PAIR];
} ContactManifold;
 
#define MAX_MANIFOLDS 2048
ContactManifold g_manifolds[MAX_MANIFOLDS];
u16        g_manifoldCount = 0;
 
// ─── narrowphase: sphere–sphere ───────────────────────────────────────────────
static bool TestSphereSphere(ShapeSphere a,ShapeSphere b,Contact *c) {
    Vector3 d=Vector3_A_minus_B(a.center,b.center);
    float dist2=dot_vector3(d,d), rSum=a.radius+b.radius;
    if (dist2>=rSum*rSum) return false;
    float dist=vsqrtf(dist2);
    c->normal    = (dist>1e-6f) ? scale_vector3(d,1.0f/dist) : (Vector3){0,1,0};
    c->depth     = rSum-dist;
    c->pointWorld= Vector3_A_plus_B(b.center,scale_vector3(c->normal,b.radius));
    c->lambdaN=c->lambdaT=0.0f;
    return true;
}
 
static bool TestSphereCapsule(ShapeSphere s,ShapeCapsule cap,Contact *c) {
    Vector3 closest = ClosestPointOnSegment(cap.base,cap.tip,s.center);
    ShapeSphere capSphere = {closest,cap.radius};
    return TestSphereSphere(s,capSphere,c);
}
 
static bool TestCapsuleCapsule(ShapeCapsule a,ShapeCapsule b,Contact *c) {
    Vector3 cpA,cpB;
    float dist2=seg_seg_closest(a.base,a.tip,b.base,b.tip,&cpA,&cpB);
    float rSum=a.radius+b.radius;
    if (dist2>=rSum*rSum) return false;
    float dist=vsqrtf(dist2);
    c->normal    = (dist>1e-6f) ? scale_vector3(Vector3_A_minus_B(cpA,cpB),1.0f/dist) : (Vector3){0,1,0};
    c->depth     = rSum-dist;
    c->pointWorld= Vector3_A_plus_B(cpB,scale_vector3(c->normal,b.radius));
    c->lambdaN=c->lambdaT=0.0f;
    return true;
}
 
static bool TestSphereBox(ShapeSphere s,ShapeBox box,Contact *c) {
    Vector3 closest=closest_point_obb(box,s.center);
    Vector3 d=Vector3_A_minus_B(s.center,closest);
    float dist2=dot_vector3(d,d);
    if (dist2>=s.radius*s.radius) return false;
    float dist=vsqrtf(dist2);
    c->normal    = (dist>1e-6f) ? scale_vector3(d,1.0f/dist) : (Vector3){0,1,0};
    c->depth     = s.radius-dist;
    c->pointWorld= closest;
    c->lambdaN=c->lambdaT=0.0f;
    return true;
}
 
static bool TestCapsuleBox(ShapeCapsule cap,ShapeBox box,Contact *c) {
    Contact best; best.depth=-1.0f;
    float ts[3]={0.0f,0.5f,1.0f};
    for (int i=0;i<3;++i) {
        Vector3 sp = Vector3_A_plus_B(cap.base,scale_vector3(Vector3_A_minus_B(cap.tip,cap.base),ts[i]));
        ShapeSphere s={sp,cap.radius};
        Contact ct;
        if (TestSphereBox(s,box,&ct) && ct.depth>best.depth) best=ct;
    }
    if (best.depth<0.0f) return false;
    *c=best; return true;
}
 
static bool TestBoxBox(ShapeBox a,ShapeBox b,Contact *c) {
    Vector3 aax,aay,aaz,bax,bay,baz;
    obb_axes(a.rot,&aax,&aay,&aaz);
    obb_axes(b.rot,&bax,&bay,&baz);
    Vector3 axes[15];
    axes[0]=aax; axes[1]=aay; axes[2]=aaz;
    axes[3]=bax; axes[4]=bay; axes[5]=baz;
    axes[6] =normalize_vector3(cross_vector3(aax,bax));
    axes[7] =normalize_vector3(cross_vector3(aax,bay));
    axes[8] =normalize_vector3(cross_vector3(aax,baz));
    axes[9] =normalize_vector3(cross_vector3(aay,bax));
    axes[10]=normalize_vector3(cross_vector3(aay,bay));
    axes[11]=normalize_vector3(cross_vector3(aay,baz));
    axes[12]=normalize_vector3(cross_vector3(aaz,bax));
    axes[13]=normalize_vector3(cross_vector3(aaz,bay));
    axes[14]=normalize_vector3(cross_vector3(aaz,baz));
    Vector3 D=Vector3_A_minus_B(b.center,a.center);
    float minPen=1e30f; int minAxis=-1; float minSign=1.0f;
    for (int i=0;i<15;++i) {
        float axLen=dot_vector3(axes[i],axes[i]);
        if (axLen<1e-8f) continue;
        Vector3 ax = (axLen<0.999f||axLen>1.001f) ? normalize_vector3(axes[i]) : axes[i];
        float ra=obb_project(a,ax), rb=obb_project(b,ax);
        float overlap=ra+rb-vabs(dot_vector3(D,ax));
        if (overlap<=0.0f) return false;
        if (overlap<minPen) { minPen=overlap; minAxis=i; minSign=(dot_vector3(D,ax)>=0.0f)?1.0f:-1.0f; }
    }
    c->depth     = minPen;
    c->normal    = scale_vector3(axes[minAxis],minSign);
    c->pointWorld= closest_point_obb(b,Vector3_A_minus_B(a.center,scale_vector3(c->normal,minPen*0.5f)));
    c->lambdaN=c->lambdaT=0.0f;
    return true;
}
 
// ─── per-entity shape extraction ─────────────────────────────────────────────
static inline void Entity_GetCapsule(const Entity *e,ShapeCapsule *out) {
    float r=e->colliderSize.x, halfInner=(e->colliderSize.y*0.5f)-r;
    if (halfInner<0.0f) halfInner=0.0f;
    Vector3 worldCenter = Vector3_A_plus_B(e->position,quat_rotate_vector(e->rotation,e->colliderCenter));
    Vector3 axis = (e->colliderSize.z<0.5f) ? quat_rotate_vector(e->rotation,(Vector3){1,0,0})
                 : (e->colliderSize.z<1.5f) ? quat_rotate_vector(e->rotation,(Vector3){0,1,0})
                 :                             quat_rotate_vector(e->rotation,(Vector3){0,0,1});
    out->radius = r;
    out->base   = Vector3_A_minus_B(worldCenter,scale_vector3(axis,halfInner));
    out->tip    = Vector3_A_plus_B (worldCenter,scale_vector3(axis,halfInner));
}
static inline void Entity_GetBox(const Entity *e,ShapeBox *out) {
    out->center      = Vector3_A_plus_B(e->position,quat_rotate_vector(e->rotation,e->colliderCenter));
    out->halfExtents = scale_vector3(e->colliderSize,0.5f);
    out->rot         = e->rotation;
}
static inline void Entity_GetSphere(const Entity *e,ShapeSphere *out) {
    out->center = Vector3_A_plus_B(e->position,quat_rotate_vector(e->rotation,e->colliderCenter));
    out->radius = e->colliderSize.x;
}
 
// ─── mesh narrowphase (no BVH — brute force over tris in local space) ─────────
// Matches the proven pattern in QueryCapsuleContact.  Contact normal and depth
// are transformed back to world space identically to that function.
#define MAX_MESH_CONTACTS MAX_CONTACTS_PER_PAIR
static u32 g_meshContactCount;
static Contact  g_meshContacts[MAX_MESH_CONTACTS];
 
static void TestSphereMeshInstance(ShapeSphere ws, u16 instanceIdx) {
    u16 mi = Sys_Global.instances[instanceIdx].modelIndex;
    if (mi >= loadedModelsMaxIndex) return;
    u32 triCount = modelTriangleCounts[mi];
    if (!triCount) return;
 
    float M[16]; __builtin_memcpy(M, &modelMatrices[instanceIdx * 16], 64);
    float m00=M[0],m10=M[1],m20=M[2], m01=M[4],m11=M[5],m21=M[6], m02=M[8],m12=M[9],m22=M[10];
    float tx=M[12],ty=M[13],tz=M[14];
    float sx=vsqrtf(m00*m00+m10*m10+m20*m20);
    float sy=vsqrtf(m01*m01+m11*m11+m21*m21);
    float sz=vsqrtf(m02*m02+m12*m12+m22*m22);
    if (sx<1e-6f||sy<1e-6f||sz<1e-6f) return;
    float sx2=sx*sx, sy2=sy*sy, sz2=sz*sz;
    float rx=ws.center.x-tx, ry=ws.center.y-ty, rz=ws.center.z-tz;
    Vector3 localC = { (rx*m00+ry*m10+rz*m20)/sx2, (rx*m01+ry*m11+rz*m21)/sy2, (rx*m02+ry*m12+rz*m22)/sz2 };
    float minScl=sx; if(sy<minScl)minScl=sy; if(sz<minScl)minScl=sz;
    float localR = ws.radius / minScl;
 
    for (u32 j = 0; j < triCount && g_meshContactCount < MAX_MESH_CONTACTS; ++j) {
        u32 bA=(u32)modelTriangles[mi][j*3+0]*VERTEX_ATTRIBUTES_SIZE;
        u32 bB=(u32)modelTriangles[mi][j*3+1]*VERTEX_ATTRIBUTES_SIZE;
        u32 bC=(u32)modelTriangles[mi][j*3+2]*VERTEX_ATTRIBUTES_SIZE;
        Vector3 A={half_to_float(*(half*)(modelVertices[mi]+bA+0)),half_to_float(*(half*)(modelVertices[mi]+bA+2)),half_to_float(*(half*)(modelVertices[mi]+bA+4))};
        Vector3 B={half_to_float(*(half*)(modelVertices[mi]+bB+0)),half_to_float(*(half*)(modelVertices[mi]+bB+2)),half_to_float(*(half*)(modelVertices[mi]+bB+4))};
        Vector3 C={half_to_float(*(half*)(modelVertices[mi]+bC+0)),half_to_float(*(half*)(modelVertices[mi]+bC+2)),half_to_float(*(half*)(modelVertices[mi]+bC+4))};
        Vector3 closest = ClosestPointOnTriangle(A,B,C,localC);
        Vector3 d = Vector3_A_minus_B(localC, closest);
        float dist2 = dot_vector3(d,d);
        if (dist2 >= localR*localR) continue;
        float dist = vsqrtf(dist2);
        Vector3 localN;
        if (dist > 1e-6f) {
            localN = scale_vector3(d, 1.0f/dist);
        } else {
            localN = normalize_vector3(cross_vector3(Vector3_A_minus_B(B,A), Vector3_A_minus_B(C,A)));
            Vector3 toC = Vector3_A_minus_B(localC, A);
            if (dot_vector3(localN,toC) < 0.0f) { localN.x=-localN.x; localN.y=-localN.y; localN.z=-localN.z; }
        }
        Vector3 wn = normalize_vector3((Vector3){
            (m00/sx)*localN.x+(m01/sy)*localN.y+(m02/sz)*localN.z,
            (m10/sx)*localN.x+(m11/sy)*localN.y+(m12/sz)*localN.z,
            (m20/sx)*localN.x+(m21/sy)*localN.y+(m22/sz)*localN.z });
        Vector3 wClosest = {
            m00*closest.x+m01*closest.y+m02*closest.z+tx,
            m10*closest.x+m11*closest.y+m12*closest.z+ty,
            m20*closest.x+m21*closest.y+m22*closest.z+tz };
        Contact ct;
        ct.depth      = (localR - dist) * minScl;
        ct.normal     = wn;
        ct.pointWorld = wClosest;
        ct.lambdaN = ct.lambdaT = 0.0f;
        g_meshContacts[g_meshContactCount++] = ct;
    }
}
 
static void TestCapsuleMeshInstance(ShapeCapsule cap, u16 instanceIdx) {
    for (int i = 0; i <= 4 && g_meshContactCount < MAX_MESH_CONTACTS; ++i) {
        float t = (float)i * 0.25f;
        Vector3 sp = Vector3_A_plus_B(cap.base, scale_vector3(Vector3_A_minus_B(cap.tip,cap.base), t));
        ShapeSphere s = {sp, cap.radius};
        TestSphereMeshInstance(s, instanceIdx);
    }
}
 
// ─── manifold generation ──────────────────────────────────────────────────────
static ContactManifold* GenerateManifold(u16 idxA,u16 idxB) {
    Entity *eA=&Sys_Global.instances[idxA], *eB=&Sys_Global.instances[idxB];
    ColliderType ctA=eA->collider, ctB=eB->collider;
    ContactManifold *m=NULL;
    for (u16 i=0;i<g_manifoldCount;++i)
        if ((g_manifolds[i].idxA==idxA && g_manifolds[i].idxB==idxB) ||
            (g_manifolds[i].idxA==idxB && g_manifolds[i].idxB==idxA)) { m=&g_manifolds[i]; break; }
    if (!m) {
        if (g_manifoldCount>=MAX_MANIFOLDS) return NULL;
        m=&g_manifolds[g_manifoldCount++];
        m->idxA=idxA; m->idxB=idxB; m->count=0;
    }
    m->count=0;
 
    if (ctB==COLLIDER_TYPE_MESH || ctB==COLLIDER_TYPE_CONVEXMESH) {
        g_meshContactCount = 0;
        if      (ctA==COLLIDER_TYPE_SPHERE)  { ShapeSphere  s; Entity_GetSphere(eA,&s);  TestSphereMeshInstance(s,idxB); }
        else if (ctA==COLLIDER_TYPE_CAPSULE) { ShapeCapsule c; Entity_GetCapsule(eA,&c); TestCapsuleMeshInstance(c,idxB); }
        else if (ctA==COLLIDER_TYPE_BOX)     {
            ShapeBox b; Entity_GetBox(eA,&b);
            Vector3 ax,ay,az; obb_axes(b.rot,&ax,&ay,&az);
            for (int cx=-1;cx<=1;cx+=2) for (int cy=-1;cy<=1;cy+=2) for (int cz=-1;cz<=1;cz+=2) {
                Vector3 corner = Vector3_A_plus_B(b.center,
                    Vector3_A_plus_B(Vector3_A_plus_B(
                        scale_vector3(ax,b.halfExtents.x*(float)cx),
                        scale_vector3(ay,b.halfExtents.y*(float)cy)),
                        scale_vector3(az,b.halfExtents.z*(float)cz)));
                ShapeSphere cs={corner,0.004f};
                TestSphereMeshInstance(cs,idxB);
            }
        }
        for (u32 i=0;i<g_meshContactCount && m->count<MAX_CONTACTS_PER_PAIR;++i) {
            float prevLN=0.0f,prevLT=0.0f;
            for (int k=0;k<(int)m->count;++k)
                if (dist_sq_vector3(m->contacts[k].pointWorld,g_meshContacts[i].pointWorld)<0.01f)
                    { prevLN=m->contacts[k].lambdaN; prevLT=m->contacts[k].lambdaT; break; }
            g_meshContacts[i].lambdaN=prevLN; g_meshContacts[i].lambdaT=prevLT;
            m->contacts[m->count++]=g_meshContacts[i];
        }
        return m->count?m:NULL;
    }
 
    Contact ct; bool hit=false;
    if      (ctA==COLLIDER_TYPE_SPHERE  && ctB==COLLIDER_TYPE_SPHERE)  { ShapeSphere a,b; Entity_GetSphere(eA,&a); Entity_GetSphere(eB,&b); hit=TestSphereSphere(a,b,&ct); }
    else if (ctA==COLLIDER_TYPE_SPHERE  && ctB==COLLIDER_TYPE_CAPSULE) { ShapeSphere a; ShapeCapsule b; Entity_GetSphere(eA,&a); Entity_GetCapsule(eB,&b); hit=TestSphereCapsule(a,b,&ct); }
    else if (ctA==COLLIDER_TYPE_CAPSULE && ctB==COLLIDER_TYPE_SPHERE)  { ShapeCapsule a; ShapeSphere b; Entity_GetCapsule(eA,&a); Entity_GetSphere(eB,&b); hit=TestSphereCapsule(b,a,&ct); if(hit) ct.normal=scale_vector3(ct.normal,-1.0f); }
    else if (ctA==COLLIDER_TYPE_CAPSULE && ctB==COLLIDER_TYPE_CAPSULE) { ShapeCapsule a,b; Entity_GetCapsule(eA,&a); Entity_GetCapsule(eB,&b); hit=TestCapsuleCapsule(a,b,&ct); }
    else if (ctA==COLLIDER_TYPE_SPHERE  && ctB==COLLIDER_TYPE_BOX)     { ShapeSphere a; ShapeBox b; Entity_GetSphere(eA,&a); Entity_GetBox(eB,&b); hit=TestSphereBox(a,b,&ct); }
    else if (ctA==COLLIDER_TYPE_BOX     && ctB==COLLIDER_TYPE_SPHERE)  { ShapeBox a; ShapeSphere b; Entity_GetBox(eA,&a); Entity_GetSphere(eB,&b); hit=TestSphereBox(b,a,&ct); if(hit) ct.normal=scale_vector3(ct.normal,-1.0f); }
    else if (ctA==COLLIDER_TYPE_CAPSULE && ctB==COLLIDER_TYPE_BOX)     { ShapeCapsule a; ShapeBox b; Entity_GetCapsule(eA,&a); Entity_GetBox(eB,&b); hit=TestCapsuleBox(a,b,&ct); }
    else if (ctA==COLLIDER_TYPE_BOX     && ctB==COLLIDER_TYPE_CAPSULE) { ShapeBox a; ShapeCapsule b; Entity_GetBox(eA,&a); Entity_GetCapsule(eB,&b); hit=TestCapsuleBox(b,a,&ct); if(hit) ct.normal=scale_vector3(ct.normal,-1.0f); }
    else if (ctA==COLLIDER_TYPE_BOX     && ctB==COLLIDER_TYPE_BOX)     { ShapeBox a,b; Entity_GetBox(eA,&a); Entity_GetBox(eB,&b); hit=TestBoxBox(a,b,&ct); }
    if (!hit) return NULL;
    if (m->count) { ct.lambdaN=m->contacts[0].lambdaN; ct.lambdaT=m->contacts[0].lambdaT; }
    m->contacts[0]=ct; m->count=1;
    return m;
}
 
// ─── impulse solver ───────────────────────────────────────────────────────────
static inline float entity_invmass(const Entity *e) {
    if (e->entflags & (ENTFLAG_KINEMATIC)) return 0.0f;
    return (e->mass>0.001f) ? 1.0f/e->mass : 0.0f;
}
static inline void ApplyAngularImpulse(Entity *e,Vector3 r,Vector3 impulse,float sign) {
    if (e->inertia<0.0001f) return;
    Vector3 torqueImpulse=cross_vector3(r,impulse);
    float invI=1.0f/e->inertia;
    e->angularVelocity.x+=sign*torqueImpulse.x*invI;
    e->angularVelocity.y+=sign*torqueImpulse.y*invI;
    e->angularVelocity.z+=sign*torqueImpulse.z*invI;
}
 
static void SolveContact(ContactManifold *m, float dt) {
    Entity *eA = &Sys_Global.instances[m->idxA];
    Entity *eB = &Sys_Global.instances[m->idxB];
    float imA = entity_invmass(eA), imB = entity_invmass(eB);
    if (imA + imB < 1e-10f) return;

    float restitution, friction;
    {
        float rA = eA->bounciness, rB = eB->bounciness;
        restitution = (eA->bounceCombine==PHYS_COMBINE_MAX||eB->bounceCombine==PHYS_COMBINE_MAX) ? vmax(rA,rB)
                    : (eA->bounceCombine==PHYS_COMBINE_MIN||eB->bounceCombine==PHYS_COMBINE_MIN) ? vmin(rA,rB)
                    : (eA->bounceCombine==PHYS_COMBINE_MUL||eB->bounceCombine==PHYS_COMBINE_MUL) ? rA*rB
                    : (rA+rB)*0.5f;
        float fA = (eA->dynamicFriction+eA->staticFriction)*0.5f;
        float fB = (eB->dynamicFriction+eB->staticFriction)*0.5f;
        friction = (eA->frictionCombine==PHYS_COMBINE_MAX||eB->frictionCombine==PHYS_COMBINE_MAX) ? vmax(fA,fB)
                 : (eA->frictionCombine==PHYS_COMBINE_MIN||eB->frictionCombine==PHYS_COMBINE_MIN) ? vmin(fA,fB)
                 : (eA->frictionCombine==PHYS_COMBINE_MUL||eB->frictionCombine==PHYS_COMBINE_MUL) ? fA*fB
                 : (fA+fB)*0.5f;
    }

    float invIA = (eA->inertia > 0.0001f) ? 1.0f / eA->inertia : 0.0f;
    float invIB = (eB->inertia > 0.0001f) ? 1.0f / eB->inertia : 0.0f;

    for (int ci = 0; ci < (int)m->count; ++ci) {
        Contact *c = &m->contacts[ci];

        // Arms from each body's collider centre to the contact point
        Vector3 rA = Vector3_A_minus_B(c->pointWorld,
            Vector3_A_plus_B(eA->position, quat_rotate_vector(eA->rotation, eA->colliderCenter)));
        Vector3 rB = Vector3_A_minus_B(c->pointWorld,
            Vector3_A_plus_B(eB->position, quat_rotate_vector(eB->rotation, eB->colliderCenter)));

        // Relative velocity at the contact point (includes angular contribution)
        Vector3 vAtA = Vector3_A_plus_B(eA->velocity, cross_vector3(eA->angularVelocity, rA));
        Vector3 vAtB = Vector3_A_plus_B(eB->velocity, cross_vector3(eB->angularVelocity, rB));
        Vector3 relVel = Vector3_A_minus_B(vAtA, vAtB);

        float vn = dot_vector3(relVel, c->normal);
        if (vn > SPECULATIVE_MARGIN / dt) continue;

        // Angular terms:  (r × n)² / I  for each body
        Vector3 rAxN = cross_vector3(rA, c->normal);
        Vector3 rBxN = cross_vector3(rB, c->normal);
        float angTermA = dot_vector3(rAxN, rAxN) * invIA;
        float angTermB = dot_vector3(rBxN, rBxN) * invIB;
        float jnDenom  = imA + imB + angTermA + angTermB;

        float effectiveRestitution = (vabs(vn) > RESTITUTION_THRESHOLD) ? restitution : 0.0f;
        float jn = -(1.0f + effectiveRestitution) * vn / jnDenom;
        float pen = c->depth - BAUMGARTE_SLOP;
        if (pen > 0.0f) jn += (BAUMGARTE_FACTOR * pen / dt) / jnDenom;

        float newLN = vmax(0.0f, c->lambdaN + jn);
        float dLN   = newLN - c->lambdaN;
        c->lambdaN  = newLN;

        Vector3 impulseN = scale_vector3(c->normal, dLN);
        eA->velocity = Vector3_A_plus_B(eA->velocity, scale_vector3(impulseN,  imA));
        eB->velocity = Vector3_A_minus_B(eB->velocity, scale_vector3(impulseN, imB));
        ApplyAngularImpulse(eA, rA, impulseN,  1.0f);
        ApplyAngularImpulse(eB, rB, impulseN, -1.0f);

        // Recompute relative velocity after normal impulse for friction
        vAtA   = Vector3_A_plus_B(eA->velocity, cross_vector3(eA->angularVelocity, rA));
        vAtB   = Vector3_A_plus_B(eB->velocity, cross_vector3(eB->angularVelocity, rB));
        relVel = Vector3_A_minus_B(vAtA, vAtB);

        Vector3 tangent = Vector3_A_minus_B(relVel,
            scale_vector3(c->normal, dot_vector3(relVel, c->normal)));
        float tLen = magnitude_vector3(tangent);
        if (tLen > 1e-6f) {
            tangent = scale_vector3(tangent, 1.0f / tLen);

            Vector3 rAxT = cross_vector3(rA, tangent);
            Vector3 rBxT = cross_vector3(rB, tangent);
            float angTermAT = dot_vector3(rAxT, rAxT) * invIA;
            float angTermBT = dot_vector3(rBxT, rBxT) * invIB;
            float jtDenom   = imA + imB + angTermAT + angTermBT;

            float vt  = dot_vector3(relVel, tangent);
            float jt  = -vt / jtDenom;
            float maxFric = friction * newLN;
            float newLT   = vclamp(c->lambdaT + jt, -maxFric, maxFric);
            float dLT     = newLT - c->lambdaT;
            c->lambdaT    = newLT;

            Vector3 impulseT = scale_vector3(tangent, dLT);
            eA->velocity = Vector3_A_plus_B(eA->velocity, scale_vector3(impulseT,  imA));
            eB->velocity = Vector3_A_minus_B(eB->velocity, scale_vector3(impulseT, imB));
            ApplyAngularImpulse(eA, rA, impulseT,  1.0f);
            ApplyAngularImpulse(eB, rB, impulseT, -1.0f);
        }
    }
}
 
// ─── speculative pre-clamp ───────────────────────────────────────────────────
static void SpeculativePreClamp(u16 idxA,float dt) {
    Entity *eA=&Sys_Global.instances[idxA];
    if (entity_invmass(eA)<1e-10f) return;
    for (u16 m=0;m<g_manifoldCount;++m) {
        ContactManifold *mf=&g_manifolds[m];
        if (mf->idxA!=idxA && mf->idxB!=idxA) continue;
        for (int ci=0;ci<(int)mf->count;++ci) {
            Contact *c=&mf->contacts[ci];
            Vector3 n = (mf->idxA==idxA) ? c->normal : scale_vector3(c->normal,-1.0f);
            float vn=dot_vector3(eA->velocity,n);
            float gap = -c->depth;
            float maxClosingSpeed = gap/dt - SPECULATIVE_MARGIN/dt;
            if (vn < maxClosingSpeed) {
                float excess = vn - maxClosingSpeed;
                eA->velocity = Vector3_A_minus_B(eA->velocity,scale_vector3(n,excess));
            }
        }
    }
}

// ─── sleep system ─────────────────────────────────────────────────────────────
static u8 g_sleepCounter[INSTANCE_COUNT];
 
static void UpdateSleep(u16 i,float dt) {
    Entity *e=&Sys_Global.instances[i];
    (void)dt;
    if (entity_invmass(e)<1e-10f) return;
    float ke=dot_vector3(e->velocity,e->velocity);
    if (ke<SLEEP_KE_THRESHOLD) {
        if (++g_sleepCounter[i]>=SLEEP_FRAMES_NEEDED) {
            e->velocity=(Vector3){0,0,0};
            flag_set(&e->entflags,ENTFLAG_ASLEEP,true);
        }
    } else {
        g_sleepCounter[i]=0;
        flag_set(&e->entflags,ENTFLAG_ASLEEP,false);
    }
}
 
static inline Vector3 ApplyAngularDrag(Vector3 omega,float drag,float dt) {
    float factor=1.0f-vclamp(drag*dt,0.0f,1.0f);
    return scale_vector3(omega,factor);
}
static inline Quaternion IntegrateRotation(Quaternion q,Vector3 omega,float dt) {
    Quaternion dq={ omega.x*dt*0.5f, omega.y*dt*0.5f, omega.z*dt*0.5f, 0.0f };
    dq=quat_multiply(q,dq);
    q.x+=dq.x; q.y+=dq.y; q.z+=dq.z; q.w+=dq.w;
    float invLen=1.0f/vsqrtf(q.x*q.x+q.y*q.y+q.z*q.z+q.w*q.w);
    return (Quaternion){q.x*invLen,q.y*invLen,q.z*invLen,q.w*invLen};
}
static void IntegrateAngularVelocity(u16 i,float dt) {
    Entity *e=&Sys_Global.instances[i];
    if (!(e->entflags & ENTFLAG_RIGIDBODY)) return;
    if (  e->entflags & ENTFLAG_ASLEEP)     return;
    if (  entity_invmass(e)<1e-10f)         return;
    float invI=(e->inertia>0.0001f)?1.0f/e->inertia:0.0f;
    e->angularVelocity=Vector3_A_plus_B(e->angularVelocity,scale_vector3(e->accumulatedTorque,invI*dt));
    e->accumulatedTorque=(Vector3){0,0,0};
    e->angularVelocity=ApplyAngularDrag(e->angularVelocity,e->angularDrag,dt);
    float omegaLen=magnitude_vector3(e->angularVelocity);
    if (omegaLen>12.566f) e->angularVelocity=scale_vector3(e->angularVelocity,12.566f/omegaLen);
    e->rotation=IntegrateRotation(e->rotation,e->angularVelocity,dt);
    Sys_Global.dirtyInstances[i]=true;
}
 
// ─── main physics step ────────────────────────────────────────────────────────
void Physics_PrimitiveStep(float dt) {
    if (dt > SUB_STEP_DT_MAX * 4.0f) dt = SUB_STEP_DT_MAX * 4.0f;
    if (dt > SUB_STEP_DT_MAX) {
        float half = dt * 0.5f;
        Physics_PrimitiveStep(half);
        Physics_PrimitiveStep(half);
        return;
    }

    // ── Broadphase: find candidate pairs via cell neighbourhood ──────────
    // For each awake rigidbody, check instances in the same cell and the
    // 3 neighbouring cells (4 total — sufficient since no object spans a cell).
    u16 n = Sys_Global.loadedInstances;
    for (u16 i = START_INDEX_LEVEL_INSTANCES; i < n; ++i) {
        Entity *eA = &Sys_Global.instances[i];
        if (!(eA->entflags & ENTFLAG_ACTIVE))    continue;
        if (!(eA->entflags & ENTFLAG_RIGIDBODY)) continue;
        if (  eA->entflags & ENTFLAG_ASLEEP)     continue;
        if (  eA->collider == COLLIDER_TYPE_NONE) continue;

        u32 maskA = GetCollisionMask(eA->layer);
        u32 cellA = eA->cellIndex;
        i32 acx   = (i32)(cellA % WORLDX);
        i32 acz   = (i32)(cellA / WORLDX);

        for (u16 j = START_INDEX_LEVEL_INSTANCES; j < n; ++j) {
            if (j == i) continue;
            Entity *eB = &Sys_Global.instances[j];
            if (!(eB->entflags & ENTFLAG_ACTIVE))    continue;
            if (  eB->collider == COLLIDER_TYPE_NONE) continue;
            if (!(maskA & eB->layer))                 continue;
            // Avoid duplicate dynamic-vs-dynamic pairs
            if ((eB->entflags & ENTFLAG_RIGIDBODY) && j < i) continue;

            // Cell proximity: must be in the same cell or an adjacent one
            u32 cellB = eB->cellIndex;
            i32 bcx   = (i32)(cellB % WORLDX);
            i32 bcz   = (i32)(cellB / WORLDX);
            i32 dx    = bcx - acx;
            i32 dz    = bcz - acz;
            if (dx < -1 || dx > 1 || dz < -1 || dz > 1) continue;

            GenerateManifold(i, j);
        }
    }

    // Prune manifolds whose pairs are no longer adjacent
    for (u16 m = 0; m < g_manifoldCount; ) {
        Entity *eA = &Sys_Global.instances[g_manifolds[m].idxA];
        Entity *eB = &Sys_Global.instances[g_manifolds[m].idxB];
        i32 acx = (i32)(eA->cellIndex % WORLDX), acz = (i32)(eA->cellIndex / WORLDX);
        i32 bcx = (i32)(eB->cellIndex % WORLDX), bcz = (i32)(eB->cellIndex / WORLDX);
        i32 dx = bcx - acx, dz = bcz - acz;
        if (dx < -1 || dx > 1 || dz < -1 || dz > 1)
            g_manifolds[m] = g_manifolds[--g_manifoldCount];
        else
            ++m;
    }

    for (u16 i = START_INDEX_LEVEL_INSTANCES; i < n; ++i)
        if (Sys_Global.instances[i].entflags & ENTFLAG_RIGIDBODY)
            SpeculativePreClamp(i, dt);

    for (int iter = 0; iter < SOLVER_ITERATIONS; ++iter)
        for (u16 m = 0; m < g_manifoldCount; ++m)
            SolveContact(&g_manifolds[m], dt);

    for (u16 i = START_INDEX_LEVEL_INSTANCES; i < n; ++i)
        if (Sys_Global.instances[i].entflags & ENTFLAG_RIGIDBODY)
            IntegrateAngularVelocity(i, dt);

    for (u16 i = START_INDEX_LEVEL_INSTANCES; i < n; ++i)
        if (Sys_Global.instances[i].entflags & ENTFLAG_RIGIDBODY)
            UpdateSleep(i, dt);
}
 
void Physics_ResetForLevelLoad(void) {
    __builtin_memset(g_manifolds,    0, sizeof(g_manifolds));
    __builtin_memset(g_sleepCounter, 0, sizeof(g_sleepCounter));
    g_manifoldCount = 0;
}
 
static inline float DefaultInertia(const Entity *e) {
    float m=(e->mass>0.001f)?e->mass:1.0f;
    if (e->collider==COLLIDER_TYPE_SPHERE)
        return 0.4f*m*e->colliderSize.x*e->colliderSize.x;
    float hx=e->colliderSize.x*0.5f,hy=e->colliderSize.y*0.5f,hz=e->colliderSize.z*0.5f;
    return m*(hx*hx+hy*hy+hz*hz)*(1.0f/3.0f);
}
void Physics_InitEntityInertia(u16 idx) {
    Entity *e=&Sys_Global.instances[idx];
    if (e->inertia<0.0001f) e->inertia=DefaultInertia(e);
}

void SetDebugLineColor(float r, float g, float b);
#define DBG_COLOR_IDLE_R    0.0f
#define DBG_COLOR_IDLE_G    0.4f
#define DBG_COLOR_IDLE_B    0.0f
#define DBG_COLOR_STAY_R    0.0f
#define DBG_COLOR_STAY_G    1.0f
#define DBG_COLOR_STAY_B    0.0f
#define DBG_COLOR_ENTER_R   1.0f
#define DBG_COLOR_ENTER_G   0.1f
#define DBG_COLOR_ENTER_B   0.1f
#define DBG_COLOR_NORMAL_R  0.1f
#define DBG_COLOR_NORMAL_G  0.3f
#define DBG_COLOR_NORMAL_B  1.0f
static void DebugDrawBox(ShapeBox b) { // Box: draw all 12 edges of the OBB.
    Vector3 ax,ay,az; obb_axes(b.rot,&ax,&ay,&az);
    Vector3 c[8]; // 8 corners
    for (int i=0;i<8;++i) {
        float sx=(i&1)?1.0f:-1.0f, sy=(i&2)?1.0f:-1.0f, sz=(i&4)?1.0f:-1.0f;
        c[i]=Vector3_A_plus_B(b.center,
             Vector3_A_plus_B(Vector3_A_plus_B(scale_vector3(ax,b.halfExtents.x*sx),scale_vector3(ay,b.halfExtents.y*sy)),scale_vector3(az,b.halfExtents.z*sz)));
    }
   
    int edges[12][2]={{0,1},{2,3},{4,5},{6,7},{0,2},{1,3},{4,6},{5,7},{0,4},{1,5},{2,6},{3,7}}; // 12 edges (pairs share one axis)
    for (int e=0;e<12;++e) AddDebugLine(c[edges[e][0]],c[edges[e][1]]);
}
 
static void DebugDrawSphere(ShapeSphere s) { // Sphere: axis-aligned star of lines (±x, ±y, ±z) + 8 diagonal octant spokes.
    float r=s.radius;
    Vector3 o=s.center;
    // 6 cardinal spokes
    AddDebugLine(o,(Vector3){o.x+r,o.y,o.z});
    AddDebugLine(o,(Vector3){o.x-r,o.y,o.z});
    AddDebugLine(o,(Vector3){o.x,o.y+r,o.z});
    AddDebugLine(o,(Vector3){o.x,o.y-r,o.z});
    AddDebugLine(o,(Vector3){o.x,o.y,o.z+r});
    AddDebugLine(o,(Vector3){o.x,o.y,o.z-r});
    // 8 octant diagonals (length r, direction normalized to corner of unit cube)
    float d=r*0.57735026f; // r / sqrt(3)
    for (int sx=-1;sx<=1;sx+=2)
    for (int sy=-1;sy<=1;sy+=2)
    for (int sz=-1;sz<=1;sz+=2)
        AddDebugLine(o,(Vector3){o.x+sx*d,o.y+sy*d,o.z+sz*d});
}
 
// Capsule: spine segment + star at each sphere center (identical to DebugDrawSphere
// but without the y-axis spokes so the spine itself serves that role).
static void DebugDrawCapsule(ShapeCapsule cap) {
    // Spine
    AddDebugLine(cap.base, cap.tip);
    // Stars at each end (skip ±Y to avoid clutter along the spine axis — use ±axis instead)
    ShapeSphere sb={cap.base,cap.radius}, st={cap.tip,cap.radius};
    DebugDrawSphere(sb);
    DebugDrawSphere(st);
}
 
// ── per-instance collider wireframe ──────────────────────────────────────────
static void DebugDrawCollider(u16 idx, float r, float g, float b) {
    Entity *e = &Sys_Global.instances[idx];
    SetDebugLineColor(r, g, b);
    switch (e->collider) {
        case COLLIDER_TYPE_BOX: {
            ShapeBox box; Entity_GetBox(e, &box);
            DebugDrawBox(box);
            break;
        }
        case COLLIDER_TYPE_SPHERE: {
            ShapeSphere sph; Entity_GetSphere(e, &sph);
            DebugDrawSphere(sph);
            break;
        }
        case COLLIDER_TYPE_CAPSULE: {
            ShapeCapsule cap; Entity_GetCapsule(e, &cap);
            DebugDrawCapsule(cap);
            break;
        }
        case COLLIDER_TYPE_MESH:
        case COLLIDER_TYPE_CONVEXMESH: {
            // For mesh colliders draw the actual triangles as wireframe edges.
            // Limited to the model's triangle list — identical fetch path to
            // QueryCapsuleContact so it shows exactly what the collision uses.
            u16 mi = e->modelIndex;
            if (mi >= loadedModelsMaxIndex) break;
            u32 tc = modelTriangleCounts[mi];
            float M[16]; __builtin_memcpy(M, &modelMatrices[idx*16], 64);
            float m00=M[0],m10=M[1],m20=M[2],m01=M[4],m11=M[5],m21=M[6],m02=M[8],m12=M[9],m22=M[10];
            float tx=M[12],ty=M[13],tz=M[14];
            // Cap triangle wireframe to avoid flooding the debug buffer on huge meshes.
            u32 step = (tc > 256) ? (tc / 256) : 1;
            for (u32 j=0; j<tc; j+=step) {
                u32 bA=(u32)modelTriangles[mi][j*3+0]*VERTEX_ATTRIBUTES_SIZE;
                u32 bB=(u32)modelTriangles[mi][j*3+1]*VERTEX_ATTRIBUTES_SIZE;
                u32 bC=(u32)modelTriangles[mi][j*3+2]*VERTEX_ATTRIBUTES_SIZE;
                Vector3 lA={half_to_float(*(half*)(modelVertices[mi]+bA+0)),half_to_float(*(half*)(modelVertices[mi]+bA+2)),half_to_float(*(half*)(modelVertices[mi]+bA+4))};
                Vector3 lB={half_to_float(*(half*)(modelVertices[mi]+bB+0)),half_to_float(*(half*)(modelVertices[mi]+bB+2)),half_to_float(*(half*)(modelVertices[mi]+bB+4))};
                Vector3 lC={half_to_float(*(half*)(modelVertices[mi]+bC+0)),half_to_float(*(half*)(modelVertices[mi]+bC+2)),half_to_float(*(half*)(modelVertices[mi]+bC+4))};
                // Transform to world space
                #define LTW(l) (Vector3){ m00*(l).x+m01*(l).y+m02*(l).z+tx, m10*(l).x+m11*(l).y+m12*(l).z+ty, m20*(l).x+m21*(l).y+m22*(l).z+tz }
                Vector3 wA=LTW(lA), wB=LTW(lB), wC=LTW(lC);
                #undef LTW
                AddDebugLine(wA, wB);
                AddDebugLine(wB, wC);
                AddDebugLine(wC, wA);
            }
            break;
        }
        default: break;
    }
}
 
// ── contact normal rays (blue) ────────────────────────────────────────────────
#define DEBUG_NORMAL_LEN 0.16f
 
static void DebugDrawManifoldNormals(const ContactManifold *m) {
    SetDebugLineColor(DBG_COLOR_NORMAL_R, DBG_COLOR_NORMAL_G, DBG_COLOR_NORMAL_B);
    for (int ci = 0; ci < (int)m->count; ++ci) {
        const Contact *c = &m->contacts[ci];
        Vector3 tip = Vector3_A_plus_B(c->pointWorld, scale_vector3(c->normal, DEBUG_NORMAL_LEN));
        AddDebugLine(c->pointWorld, tip);
    }
}
 
// ── main debug draw entry point ───────────────────────────────────────────────
// Call once per frame, after Physics_PrimitiveStep(), before DrawDebugLines().
// Tracks which manifolds are brand-new this frame (OnColliderEnter) vs
// continuing (OnColliderStay) by comparing against last frame's set.
// No heap allocation — uses fixed arrays sized to MAX_MANIFOLDS.
 
#define MAX_DEBUG_MANIFOLD_IDS MAX_MANIFOLDS
static u32 g_prevManifoldIDs[MAX_DEBUG_MANIFOLD_IDS]; // packed (idxA<<16)|idxB
static u16 g_prevManifoldCount = 0;
 
// Pack a pair into a canonical ID regardless of A/B order.
static inline u32 ManifoldID(u16 a, u16 b) {
    return (a < b) ? ((u32)a << 16) | b : ((u32)b << 16) | a;
}
static inline bool IDInPrevSet(u32 id) {
    for (u16 i = 0; i < g_prevManifoldCount; ++i)
        if (g_prevManifoldIDs[i] == id) return true;
    return false;
}
 
void Physics_DrawDebug(void) {
    if (Sys_Global.physicsDebug <= 0) return;
 
    u16 n = Sys_Global.loadedInstances;
 
    // ── 1. Build set of active manifold IDs this frame ────────────────────────
    u32 curIDs[MAX_DEBUG_MANIFOLD_IDS]; u16 curCount = 0;
    for (u16 m = 0; m < g_manifoldCount && curCount < MAX_DEBUG_MANIFOLD_IDS; ++m) curIDs[curCount++] = ManifoldID(g_manifolds[m].idxA, g_manifolds[m].idxB);
 
    // ── 2. Identify which instance indices are in contact (stay vs enter) ─────
    // staySet / enterSet: bitmask or parallel array — use a small flag array.
    // INSTANCE_COUNT <= 7680, so a byte array is 7.5 KB — fine on stack for debug.
    // Use 0=idle, 1=stay, 2=enter.
    static u8 g_contactState[INSTANCE_COUNT]; // 0=idle,1=stay,2=enter
    __builtin_memset(g_contactState, 0, n); // only clear loaded range
    for (u16 m = 0; m < g_manifoldCount; ++m) {
        u16 a = g_manifolds[m].idxA, b = g_manifolds[m].idxB;
        u32 id = ManifoldID(a, b);
        u8 state = IDInPrevSet(id) ? 1 : 2; // 1=stay, 2=enter
        // Only flag dynamic objects (kinematic/static don't need enter/stay colouring)
        if (Sys_Global.instances[a].entflags & ENTFLAG_RIGIDBODY) {
            if (state > g_contactState[a]) g_contactState[a] = state;
        }
        if (Sys_Global.instances[b].entflags & ENTFLAG_RIGIDBODY) {
            if (state > g_contactState[b]) g_contactState[b] = state;
        }
    }
 
    // ── 3. Draw collider wireframes ───────────────────────────────────────────
    for (u16 i = START_INDEX_LEVEL_INSTANCES; i < n; ++i) {
        Entity *e = &Sys_Global.instances[i];
        if (!(e->entflags & ENTFLAG_ACTIVE))    continue;
        if (  e->collider == COLLIDER_TYPE_NONE) continue;
 
        u8 cs = g_contactState[i];
        if (cs == 2) {
            // OnColliderEnter — red
            DebugDrawCollider(i, DBG_COLOR_ENTER_R, DBG_COLOR_ENTER_G, DBG_COLOR_ENTER_B);
        } else if (cs == 1) {
            // OnColliderStay — bright green (dynamic objects touching)
            DebugDrawCollider(i, DBG_COLOR_STAY_R, DBG_COLOR_STAY_G, DBG_COLOR_STAY_B);
        } else {
            // Idle — dim green
            DebugDrawCollider(i, DBG_COLOR_IDLE_R, DBG_COLOR_IDLE_G, DBG_COLOR_IDLE_B);
        }
    }
 
    // ── 4. Draw contact normals (blue rays) for all capsule contacts ──────────
    for (u16 m = 0; m < g_manifoldCount; ++m) DebugDrawManifoldNormals(&g_manifolds[m]);
 
    // ── 5. Advance frame: current IDs become previous ─────────────────────────
    __builtin_memcpy(g_prevManifoldIDs, curIDs, curCount * sizeof(u32));
    g_prevManifoldCount = curCount;
}
 
 
// ═══════════════════════════════════════════════════════════════════════════════
// PLAYER / CAPSULE MOVEMENT (unchanged from original)
// ═══════════════════════════════════════════════════════════════════════════════
 
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
    float    depth;
    Vector3  normal;
} CapsuleContact;
#define NO_CONTACT ((CapsuleContact){ .depth = -1.0f, .normal = {0,1,0} })

static CapsuleContact QueryCapsuleContact(Vector3 start, Vector3 end, float capsuleRadius, u32 layerMask) {
    CapsuleContact worst = NO_CONTACT;
    float minX = vmin(start.x, end.x) - capsuleRadius;
    float maxX = vmax(start.x, end.x) + capsuleRadius;
    float minZ = vmin(start.z, end.z) - capsuleRadius;
    float maxZ = vmax(start.z, end.z) + capsuleRadius;
    i32 cxMin = vmax(0, vmin(WORLDX-1, PosGetCellCoordX(minX)));
    i32 cxMax = vmax(0, vmin(WORLDX-1, PosGetCellCoordX(maxX)));
    i32 czMin = vmax(0, vmin(WORLDZ-1, PosGetCellCoordZ(minZ)));
    i32 czMax = vmax(0, vmin(WORLDZ-1, PosGetCellCoordZ(maxZ)));
    for (u16 i = START_INDEX_LEVEL_INSTANCES; i < Sys_Global.loadedInstances; ++i) {
        if (!(layerMask & Sys_Global.instances[i].layer)) continue;

        // Cell culling: skip if this instance isn't in one of the cells
        // overlapping the capsule's XZ AABB.
        u32 instCell = Sys_Global.instances[i].cellIndex;
        i32 icx = (i32)(instCell % WORLDX);
        i32 icz = (i32)(instCell / WORLDX);
        if (icx < cxMin || icx > cxMax || icz < czMin || icz > czMax) continue;

        // Open-cell cull: geometry in closed cells is never reachable.
        if (!(gridCellStates[instCell] & CELL_OPEN)) continue;

        u16 mindex = Sys_Global.instances[i].modelIndex;
        if (mindex >= loadedModelsMaxIndex) continue;
        u32 triCount = modelTriangleCounts[mindex];
        if (triCount < 1) continue;

        float M[16]; __builtin_memcpy(M, &modelMatrices[i*16], 64);
        float m00=M[0],m10=M[1],m20=M[2];
        float m01=M[4],m11=M[5],m21=M[6];
        float m02=M[8],m12=M[9],m22=M[10];
        float tx=M[12],ty=M[13],tz=M[14];
        float scl_x=vsqrtf(m00*m00+m10*m10+m20*m20);
        float scl_y=vsqrtf(m01*m01+m11*m11+m21*m21);
        float scl_z=vsqrtf(m02*m02+m12*m12+m22*m22);
        if (scl_x<1e-6f||scl_y<1e-6f||scl_z<1e-6f) continue;

        // Bounding-sphere pre-reject.
        Vector3 objPos = Sys_Global.instances[i].position;
        Vector3 capsuleMid = {
            (start.x+end.x)*0.5f,
            (start.y+end.y)*0.5f,
            (start.z+end.z)*0.5f
        };
        Vector3 delta = Vector3_A_minus_B(objPos, capsuleMid);
        float distSqrd = delta.x*delta.x+delta.y*delta.y+delta.z*delta.z;
        float modelRad = vmax(modelBounds[mindex], 1.81f);
        float spineHalf = magnitude_vector3(Vector3_A_minus_B(end,start))*0.5f;
        float combinedRad = modelRad+spineHalf+capsuleRadius+0.1f;
        if (distSqrd > combinedRad*combinedRad) continue;

        float scl_x2=scl_x*scl_x, scl_y2=scl_y*scl_y, scl_z2=scl_z*scl_z;
        Vector3 relS={start.x-tx,start.y-ty,start.z-tz};
        Vector3 localStart={
            (relS.x*m00+relS.y*m10+relS.z*m20)/scl_x2,
            (relS.x*m01+relS.y*m11+relS.z*m21)/scl_y2,
            (relS.x*m02+relS.y*m12+relS.z*m22)/scl_z2
        };
        Vector3 relE={end.x-tx,end.y-ty,end.z-tz};
        Vector3 localEnd={
            (relE.x*m00+relE.y*m10+relE.z*m20)/scl_x2,
            (relE.x*m01+relE.y*m11+relE.z*m21)/scl_y2,
            (relE.x*m02+relE.y*m12+relE.z*m22)/scl_z2
        };
        float minScl=scl_x;
        if (scl_y<minScl) minScl=scl_y;
        if (scl_z<minScl) minScl=scl_z;
        float localRadius=capsuleRadius/minScl;

        for (u32 j=0; j<triCount; ++j) {
            u32 bA=(u32)modelTriangles[mindex][j*3+0]*VERTEX_ATTRIBUTES_SIZE;
            u32 bB=(u32)modelTriangles[mindex][j*3+1]*VERTEX_ATTRIBUTES_SIZE;
            u32 bC=(u32)modelTriangles[mindex][j*3+2]*VERTEX_ATTRIBUTES_SIZE;
            Vector3 posA={
                half_to_float(*(half*)(modelVertices[mindex]+bA+0)),
                half_to_float(*(half*)(modelVertices[mindex]+bA+2)),
                half_to_float(*(half*)(modelVertices[mindex]+bA+4))
            };
            Vector3 posB={
                half_to_float(*(half*)(modelVertices[mindex]+bB+0)),
                half_to_float(*(half*)(modelVertices[mindex]+bB+2)),
                half_to_float(*(half*)(modelVertices[mindex]+bB+4))
            };
            Vector3 posC={
                half_to_float(*(half*)(modelVertices[mindex]+bC+0)),
                half_to_float(*(half*)(modelVertices[mindex]+bC+2)),
                half_to_float(*(half*)(modelVertices[mindex]+bC+4))
            };

            Vector3 cpP  = ClosestPointOnTriangle(posA,posB,posC,localStart);
            Vector3 spP  = ClosestPointOnSegment(localStart,localEnd,cpP);
            Vector3 cpP2 = ClosestPointOnTriangle(posA,posB,posC,spP);
            Vector3 cpQ  = ClosestPointOnTriangle(posA,posB,posC,localEnd);
            Vector3 spQ  = ClosestPointOnSegment(localStart,localEnd,cpQ);
            Vector3 cpQ2 = ClosestPointOnTriangle(posA,posB,posC,spQ);

            Vector3 dP=Vector3_A_minus_B(spP,cpP2);
            Vector3 dQ=Vector3_A_minus_B(spQ,cpQ2);
            float distP=vsqrtf(dot_vector3(dP,dP));
            float distQ=vsqrtf(dot_vector3(dQ,dQ));
            float localDist; Vector3 localContactVec;
            if (distP<=distQ) { localDist=distP; localContactVec=dP; }
            else              { localDist=distQ; localContactVec=dQ; }

            float localPen=localRadius-localDist;
            if (localPen<=0.0f) continue;

            Vector3 localNormal;
            if (localDist>1e-6f) {
                localNormal=(Vector3){
                    localContactVec.x/localDist,
                    localContactVec.y/localDist,
                    localContactVec.z/localDist
                };
            } else {
                Vector3 eAB=Vector3_A_minus_B(posB,posA);
                Vector3 eAC=Vector3_A_minus_B(posC,posA);
                localNormal=normalize_vector3(cross_vector3(eAB,eAC));
                Vector3 spMid={
                    (localStart.x+localEnd.x)*0.5f,
                    (localStart.y+localEnd.y)*0.5f,
                    (localStart.z+localEnd.z)*0.5f
                };
                Vector3 toMid=Vector3_A_minus_B(spMid,posA);
                if (dot_vector3(localNormal,toMid)<0.0f) {
                    localNormal.x=-localNormal.x;
                    localNormal.y=-localNormal.y;
                    localNormal.z=-localNormal.z;
                }
            }

            Vector3 worldNormal=normalize_vector3((Vector3){
                (m00/scl_x)*localNormal.x+(m01/scl_y)*localNormal.y+(m02/scl_z)*localNormal.z,
                (m10/scl_x)*localNormal.x+(m11/scl_y)*localNormal.y+(m12/scl_z)*localNormal.z,
                (m20/scl_x)*localNormal.x+(m21/scl_y)*localNormal.y+(m22/scl_z)*localNormal.z
            });
            float worldPen=localPen*minScl;
            if (worldPen>worst.depth) {
                worst.depth=worldPen;
                worst.normal=worldNormal;
            }
        }
    }
    return worst;
}

// World collision response for a single rigidbody using its own collider shape.
// Returns the worst penetration contact against static mesh geometry, or NO_CONTACT.
static CapsuleContact QueryRigidbodyWorldContact(u16 i, Vector3 pos) {
    Entity *e = &Sys_Global.instances[i];
    u32 mask  = GetCollisionMask(e->layer);
    switch (e->collider) {
        case COLLIDER_TYPE_SPHERE: {
            // Recentre the sphere at `pos` (pos is the entity position, not eye)
            ShapeSphere s;
            s.center = Vector3_A_plus_B(pos,
                quat_rotate_vector(e->rotation, e->colliderCenter));
            s.radius = e->colliderSize.x;
            // We can reuse QueryCapsuleContact with a degenerate zero-length spine
            return QueryCapsuleContact(s.center, s.center, s.radius, mask);
        }
        case COLLIDER_TYPE_CAPSULE: {
            // Rebuild capsule tips at the new position
            float r         = e->colliderSize.x;
            float halfInner = (e->colliderSize.y * 0.5f) - r;
            if (halfInner < 0.0f) halfInner = 0.0f;
            Vector3 center  = Vector3_A_plus_B(pos,
                quat_rotate_vector(e->rotation, e->colliderCenter));
            Vector3 axis    = (e->colliderSize.z < 0.5f) ? quat_rotate_vector(e->rotation, (Vector3){1,0,0}) : ((e->colliderSize.z < 1.5f) ? quat_rotate_vector(e->rotation, (Vector3){0,1,0}) :  quat_rotate_vector(e->rotation, (Vector3){0,0,1}));
            Vector3 base    = Vector3_A_minus_B(center,scale_vector3(axis,halfInner));
            Vector3 tip     = Vector3_A_plus_B (center,scale_vector3(axis,halfInner));
            return QueryCapsuleContact(base, tip, r, mask);
        }
        case COLLIDER_TYPE_BOX: {
            ShapeBox b;
            b.center      = Vector3_A_plus_B(pos, quat_rotate_vector(e->rotation, e->colliderCenter));
            b.halfExtents = scale_vector3(e->colliderSize, 0.5f);
            b.rot         = e->rotation;
            Vector3 ax, ay, az;
            obb_axes(b.rot, &ax, &ay, &az);
            CapsuleContact worst = NO_CONTACT;
            for (int cx = -1; cx <= 1; cx += 2) {
                for (int cy = -1; cy <= 1; cy += 2) {
                    for (int cz = -1; cz <= 1; cz += 2) {
                        Vector3 corner = Vector3_A_plus_B(b.center,
                            Vector3_A_plus_B(
                                Vector3_A_plus_B(
                                    scale_vector3(ax, b.halfExtents.x * (float)cx),
                                    scale_vector3(ay, b.halfExtents.y * (float)cy)),
                                scale_vector3(az, b.halfExtents.z * (float)cz)));
                        CapsuleContact c = QueryCapsuleContact(corner, corner, 0.004f, mask);
                        // Only accept contacts where the normal pushes us out (not further in)
                        if (c.depth > worst.depth && c.normal.y > -0.1f)
                            worst = c;
                    }
                }
            }
            return worst;
        }
        default:
            // MESH colliders don't move as rigidbodies in practice;
            // fall back to a bounding-sphere approximation.
            return QueryCapsuleContact(pos,pos,modelBounds[e->modelIndex] > 0.01f ? modelBounds[e->modelIndex] : 0.5f,mask);
    }
}
 
ENGINE_TO_MOD bool CheckCapsule(Vector3 start, Vector3 end, float capsuleRadius, float capsuleHeight, u32 layerMask) {
    (void)capsuleHeight;
    return QueryCapsuleContact(start, end, capsuleRadius, layerMask).depth > 0.0f;
}
 
static inline void CapsuleTipsFromEye(Vector3 eye, Vector3 *start, Vector3 *end) {
    float innerSpine = PLAYER_HEIGHT - 2.0f * PLAYER_RADIUS;
    float centreY = eye.y - PLAYER_CAM_OFFSET_Y;
    *start = (Vector3){ eye.x, centreY - innerSpine * 0.5f, eye.z };
    *end   = (Vector3){ eye.x, centreY + innerSpine * 0.5f, eye.z };
}
 
static float SnapEyeAboveFloor(float eyeX, float eyeY, float eyeZ, u32 mask) {
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
 
void BuildPlayerCapsule(u16 playerIdx, Vector3 *start, Vector3 *end) {
    Vector3 eye = Sys_Global.instances[playerIdx].position;
    float innerSpine = PLAYER_HEIGHT - 2.0f * PLAYER_RADIUS;
    float centreY = eye.y - PLAYER_CAM_OFFSET_Y;
    start->x = eye.x; start->y = centreY - innerSpine * 0.5f; start->z = eye.z;
    end->x   = eye.x; end->y   = centreY + innerSpine * 0.5f; end->z   = eye.z;
}
 
ENGINE_TO_MOD void AddForce(u16 idx, Vector3 force, bool isImpulse) {
    if (idx >= INSTANCE_COUNT) return;
    Entity* e = &Sys_Global.instances[idx];
    float mass = e->mass > 0.0001f ? e->mass : 1.0f;
    e->accumulatedForce = Vector3_A_plus_B(e->accumulatedForce, force);
    if (isImpulse) e->velocity = Vector3_A_plus_B(e->velocity, scale_vector3(force,1.0f / mass));
    else e->velocity = Vector3_A_plus_B(e->velocity, scale_vector3(force,((float)Sys_Global.timeSinceLastPhysicsTick) / mass));
}
 
ENGINE_TO_MOD void ApplyPlayerMovements(void) {
    Vector3 forward = Sys_Global.instances[PLAYER1].forward;
    Vector3 right = Sys_Global.instances[PLAYER1].right;
    Vector3 input = {0};
    if (Forward())     input = Vector3_A_plus_B(input, (Vector3){forward.x, 0, forward.z});
    if (Backpedal())   input = Vector3_A_minus_B(input, (Vector3){forward.x, 0, forward.z});
    if (StrafeRight()) input = Vector3_A_plus_B(input, (Vector3){right.x, 0, right.z});
    if (StrafeLeft())  input = Vector3_A_minus_B(input, (Vector3){right.x, 0, right.z});
    if (SwimDn())      input.y -= 1.0f;
    if (SwimUp())      input.y += 1.0f;
    input = normalize_vector3(input);
    float intent = magnitude_vector3(input);
    float speed = GetBasePlayerSpeed(PLAYER1,intent > 0.1f) * 1.75f;
    Vector3 wishVel = scale_vector3(input, speed);
    Vector3 currentVel = Sys_Global.instances[PLAYER1].velocity;
    float accel = Sys_Global.boosterActive ? 1.0f : 3.0f;
    Vector3 deltaVel = Vector3_A_minus_B(wishVel, currentVel);
    deltaVel.x = vmax(vmin(deltaVel.x,10.0f),-10.0f);
    deltaVel.y = vmax(vmin(deltaVel.y,10.0f),-10.0f);
    deltaVel.z = vmax(vmin(deltaVel.z,10.0f),-10.0f);
    Vector3 appliedVel = Vector3_A_plus_B(currentVel, scale_vector3(deltaVel, (accel * (float)Sys_Global.timeSinceLastPhysicsTick)));
    Sys_Global.instances[PLAYER1].velocity = appliedVel;
}
 
const Vector3 gravityVelocity = { 0.0f, -0.0981f, 0.0f };
void UpdateVelocityFromGravity(void) {
    if (Sys_Global.pauseRelativeTime < 10.0f) return;
    for (u32 i=PLAYER1;i<INSTANCE_COUNT;++i) {
        if (i > Sys_Global.loadedInstances) return;
        if (Sys_Global.instances[i].gravity < 0.01f && Sys_Global.instances[i].gravity > -0.01f) continue;
        if (i <= (i32)PLAYER2 && Sys_Cheats.noclip) continue;
        Sys_Global.instances[i].velocity = Vector3_A_plus_B(Sys_Global.instances[i].velocity, scale_vector3(gravityVelocity, Sys_Global.instances[i].gravity * (float)Sys_Global.timeSinceLastPhysicsTick));
    }
}
 
void ApplyCorpseFriction(u16 instanceIdx) {
    Sys_Global.instances[instanceIdx].dynamicFriction = 10.0f;
    Sys_Global.instances[instanceIdx].staticFriction = 10.0f;
    Sys_Global.instances[instanceIdx].bounciness = 0.0f;
    Sys_Global.instances[instanceIdx].frictionCombine = PHYS_COMBINE_MUL;
    Sys_Global.instances[instanceIdx].bounceCombine = PHYS_COMBINE_MAX;
}

float floorMinimum = -48.6316f;
bool GridCellBlock(u16 i,Vector3 pos,Vector3 newPos);
static void IntegrateRigidbody(u16 i, float dt) {
    Entity *e = &Sys_Global.instances[i];
    if (!(e->entflags & ENTFLAG_ACTIVE))    return;
    if (!(e->entflags & ENTFLAG_RIGIDBODY)) return;
    if (  e->entflags & ENTFLAG_ASLEEP)     return;
    if (  e->entflags & ENTFLAG_KINEMATIC)  return;

    float mag = magnitude_vector3(e->velocity);
    if (mag < 0.005f) return;

    Vector3 pos    = e->position;
    Vector3 newPos = Vector3_A_plus_B(pos, scale_vector3(e->velocity, dt));

    if (GridCellBlock(i, pos, newPos)) return;

    // ── Step 1: resolve contact at the PROPOSED new position ──────────────
    // Allow multiple iterations so a corner can't tunnel through thin geometry.
    // Hard limit to avoid infinite loop on degenerate geometry.
    for (int iter = 0; iter < 4; ++iter) {
        CapsuleContact c = QueryRigidbodyWorldContact(i, newPos);
        if (c.depth <= 0.0f) break;  // clean, done

        // Only trust normals that point somewhat away from geometry
        // (away = positive dot with the direction from old to new pos).
        // This rejects inside-out normals from corners that tunnelled through.
        Vector3 motion = Vector3_A_minus_B(newPos, pos);
        float motionLen = magnitude_vector3(motion);
        if (motionLen > 1e-5f) {
            Vector3 motionDir = scale_vector3(motion, 1.0f / motionLen);
            if (dot_vector3(c.normal, motionDir) > 0.5f) {
                // Normal points the same direction as motion — we tunnelled.
                // Binary search back along the motion ray to find the surface.
                Vector3 safePos = pos;
                Vector3 testPos = newPos;
                for (int bi = 0; bi < 6; ++bi) {
                    Vector3 mid = {
                        (safePos.x + testPos.x) * 0.5f,
                        (safePos.y + testPos.y) * 0.5f,
                        (safePos.z + testPos.z) * 0.5f
                    };
                    CapsuleContact mc = QueryRigidbodyWorldContact(i, mid);
                    if (mc.depth > 0.0f) testPos = mid;
                    else                 safePos  = mid;
                }
                newPos = safePos;
                // Zero ALL velocity — tunnelling means we have no reliable normal
                e->velocity = (Vector3){0, 0, 0};
                goto position_resolved;
            }
        }

        // ── Normal push: move position to surface, no velocity injection ──
        // Push by exactly depth + tiny epsilon so next query returns clean.
        newPos = Vector3_A_plus_B(newPos, scale_vector3(c.normal, c.depth + 0.0005f));

        // ── Velocity: cancel only the into-surface component ──────────────
        // This makes the object slide along the surface instead of stopping dead.
        float vn = dot_vector3(e->velocity, c.normal);
        if (vn < 0.0f) {
            // Remove the penetrating component.  No restitution here —
            // bouncing is handled by the impulse solver, not integration.
            e->velocity = Vector3_A_minus_B(e->velocity,
                scale_vector3(c.normal, vn));
        }
        // If vn >= 0 the object is already moving away — don't touch velocity at all.
    }
position_resolved:;

    e->lastPosition = pos;
    e->position     = newPos;
    e->cellIndex    = PosGetCellCoords(newPos.x, newPos.z);
    Sys_Global.dirtyInstances[i] = true;
}
 
static void IntegratePlayer(u16 i,float dt) {
    Entity *e=&Sys_Global.instances[i];
    Vector3 pos=e->position;
    e->cellIndex=PosGetCellCoords(pos.x,pos.z);
    Vector3 vel=e->velocity;
    float mag=magnitude_vector3(vel);
 
    if (i<=PLAYER2 && Sys_Cheats.noclip) {
        e->position=Vector3_A_plus_B(pos,scale_vector3(vel,dt));
        return;
    }
    if (mag<0.05f) return;
 
    Vector3 dir=normalize_vector3(vel);
    Vector3 hitPos   =Vector3_A_plus_B(pos,scale_vector3(dir,PLAYER_RADIUS));
    Vector3 newHitPos=Vector3_A_plus_B(hitPos,scale_vector3(vel,dt));
 
    if (GridCellBlock(i,pos,newHitPos)) return;
 
    u32 mask=GetCollisionMask(e->layer);
    float innerSpine=PLAYER_HEIGHT-2.0f*PLAYER_RADIUS;
    bool boosted=Sys_Global.boosterActive;
 
    {
        Vector3 s,en; CapsuleTipsFromEye(pos,&s,&en);
        CapsuleContact c=QueryCapsuleContact(s,en,PLAYER_RADIUS,mask);
        if (c.depth>0.0f) {
            pos.x+=c.normal.x*(c.depth+SNAP_STEP);
            pos.y+=c.normal.y*(c.depth+SNAP_STEP);
            pos.z+=c.normal.z*(c.depth+SNAP_STEP);
            CapsuleTipsFromEye(pos,&s,&en);
            if (QueryCapsuleContact(s,en,PLAYER_RADIUS,mask).depth>0.0f)
                pos.y=SnapEyeAboveFloor(pos.x,pos.y,pos.z,mask);
            e->position=pos; vel=e->velocity;
        }
    }
 
    float centreY=pos.y-PLAYER_CAM_OFFSET_Y;
    Vector3 curStart={pos.x,centreY-innerSpine*0.5f,pos.z};
    Vector3 curEnd  ={pos.x,centreY+innerSpine*0.5f,pos.z};
    bool isGrounded=false; float slopeDeg=0.0f;
    Vector3 floorNormal={0,1,0};
 
    if (vel.y<=0.05f) {
        float snapRange=vmin(GROUND_PROBE_DIST,vmax(0.08f,vabs(vel.y)*dt+0.04f));
        Vector3 probeEye={pos.x,pos.y-snapRange,pos.z};
        Vector3 pS,pE; CapsuleTipsFromEye(probeEye,&pS,&pE);
        CapsuleContact probe=QueryCapsuleContact(pS,pE,PLAYER_RADIUS,mask);
        if (probe.depth>0.0f && probe.normal.y>0.1f) {
            floorNormal=probe.normal; isGrounded=true;
            float cosA=vmax(-1.0f,vmin(1.0f,floorNormal.y));
            slopeDeg=(180.0f/3.14159265f)*vacosf(cosA);
            float floorY=pos.y;
            for (float d=SNAP_STEP;d<=snapRange;d+=SNAP_STEP) {
                Vector3 s,en; CapsuleTipsFromEye((Vector3){pos.x,pos.y-d,pos.z},&s,&en);
                if (QueryCapsuleContact(s,en,PLAYER_RADIUS,mask).depth>0.0f) break;
                floorY=pos.y-d;
            }
            pos.y=floorY; e->position=pos;
            e->velocity.y=0.0f; vel.y=0.0f;
            centreY=pos.y-PLAYER_CAM_OFFSET_Y;
            curStart=(Vector3){pos.x,centreY-innerSpine*0.5f,pos.z};
            curEnd  =(Vector3){pos.x,centreY+innerSpine*0.5f,pos.z};
        }
    }
 
    if (isGrounded) {
        if (slopeDeg>SLOPE_CLIMB_MAX_DEG) {
            float vdn=dot_vector3(vel,floorNormal);
            if (vdn>0.0f) { vel.x-=floorNormal.x*vdn; vel.y-=floorNormal.y*vdn; vel.z-=floorNormal.z*vdn; }
            float gdn=-9.81f*floorNormal.y;
            float accel=boosted?SLOPE_SLIDE_ACCEL_BOOST:SLOPE_SLIDE_ACCEL;
            Vector3 slide={-floorNormal.x*gdn,-9.81f-floorNormal.y*gdn,-floorNormal.z*gdn};
            float slen=magnitude_vector3(slide);
            if (slen>1e-4f) { vel.x+=(slide.x/slen)*accel*dt; vel.y+=(slide.y/slen)*accel*dt; vel.z+=(slide.z/slen)*accel*dt; }
        } else if (slopeDeg>SLOPE_WALK_MAX_DEG) {
            float t=(slopeDeg-SLOPE_WALK_MAX_DEG)/(SLOPE_CLIMB_MAX_DEG-SLOPE_WALK_MAX_DEG);
            vel.x*=(1.0f-t); vel.z*=(1.0f-t);
        }
        float frAccel=boosted?SLOPE_FRICTION_ACCEL_BOOST:SLOPE_FRICTION_ACCEL;
        float hspeed=vsqrtf(vel.x*vel.x+vel.z*vel.z);
        if (hspeed>1e-4f) {
            float fd=frAccel*dt;
            if (fd>=hspeed) { vel.x=0.0f; vel.z=0.0f; }
            else { float s=(hspeed-fd)/hspeed; vel.x*=s; vel.z*=s; }
        }
        e->velocity=vel;
    }
 
    if (vel.y*vel.y>1e-6f) {
        bool movingDown=vel.y<0.0f;
        Vector3 vTestEye={pos.x,pos.y+vel.y*dt,pos.z};
        Vector3 vS,vE; CapsuleTipsFromEye(vTestEye,&vS,&vE);
        CapsuleContact vc=QueryCapsuleContact(vS,vE,PLAYER_RADIUS,mask);
        bool blockV=false;
        if (movingDown)                           blockV=(vc.depth>0.0f && vc.normal.y>0.1f);
        else if (vc.depth>0.0f && vc.normal.y<-0.1f)
            blockV=(vc.depth>QueryCapsuleContact(curStart,curEnd,PLAYER_RADIUS,mask).depth);
        if (blockV) { e->velocity.y=0.0f; vel.y=0.0f; }
    }
 
    Vector3 hVel={vel.x,0.0f,vel.z};
    if (hVel.x*hVel.x+hVel.z*hVel.z>1e-6f) {
        Vector3 hTestEye={pos.x+hVel.x*dt,pos.y,pos.z+hVel.z*dt};
        Vector3 hS,hE; CapsuleTipsFromEye(hTestEye,&hS,&hE);
        CapsuleContact hc=QueryCapsuleContact(hS,hE,PLAYER_RADIUS,mask);
        if (hc.depth>0.0f && hc.depth>QueryCapsuleContact(curStart,curEnd,PLAYER_RADIUS,mask).depth) {
            float vdn=dot_vector3(vel,hc.normal);
            if (vdn<0.0f) { vel.x-=hc.normal.x*vdn; vel.z-=hc.normal.z*vdn; e->velocity.x=vel.x; e->velocity.z=vel.z; }
        }
    }
 
    vel=e->velocity;
    if (magnitude_vector3(vel)<0.05f) return;
    e->lastPosition=pos;
    e->position=Vector3_A_plus_B(pos,scale_vector3(vel,dt));
    Sys_Global.dirtyInstances[i]=true;
}
 
extern ma_engine audio_engine;
void UpdatePositions(void) {
    float dt = vclamp((float)Sys_Global.timeSinceLastPhysicsTick,0.0005f,0.027777778f); // At most 4 frames at 144fps rate
    for (u32 i = PLAYER1; i <= PLAYER2; ++i) IntegratePlayer((u16)i, dt);
    
    // Every other active rigidbody in the loaded level range.
    for (u32 i = START_INDEX_LEVEL_INSTANCES; i < (u32)Sys_Global.loadedInstances; ++i) {
        if (Sys_Global.instances[i].entflags & ENTFLAG_RIGIDBODY) IntegrateRigidbody((u16)i, dt);
    }

    ma_engine_listener_set_position(&audio_engine,0,Sys_Global.instances[PLAYER1].position.x,Sys_Global.instances[PLAYER1].position.y,Sys_Global.instances[PLAYER1].position.z);
}
 
void ClampVelocity(void) {
    for (i32 i=START_INDEX_LEVEL_INSTANCES;i<Sys_Global.loadedInstances;++i) {
        Vector3 curvel = Sys_Global.instances[i].velocity;
        if (magnitude_vector3(curvel) > TERMINAL_VELOCITY) {
            Vector3 dir = normalize_vector3(curvel);
            Sys_Global.instances[i].velocity = scale_vector3(dir, TERMINAL_VELOCITY);
        }
    }
}
 
void UpdateTriggers(void);
void Physics(void) {
    UpdateVelocityFromGravity();
    Physics_PrimitiveStep((float)Sys_Global.timeSinceLastPhysicsTick);
    ClampVelocity();
    UpdatePositions();
    UpdateTriggers();
    Physics_DrawDebug();
}
