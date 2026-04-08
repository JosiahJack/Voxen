// physics.c - Physics
#include "voxen.h"
#include <malloc.h>
typedef __builtin_va_list va_list;
extern u16 loadedModelsMaxIndex; extern float modelBounds[MODEL_IDX_MAX]; extern u8** modelVertices; extern u16** modelTriangles;
extern u32 modelVertexCounts[MODEL_IDX_MAX]; extern u16 modelTriangleCounts[MODEL_IDX_MAX];
extern float modelMatrices[INSTANCE_COUNT * 16];
typedef u16 half;
static inline float half_to_float(half h){
    u32 s=(h&0x8000)<<16,e=(h&0x7C00)>>10,m=(h&0x03FF),out;
    if (e == 0){
        if (m == 0) out = s;
        else { // normalize subnormal
            e = 1;
            while ((m & 0x0400) == 0) { m <<= 1; e--; }
            m &= 0x03FF;
            e = e + (127 - 15);
            out = s | (e << 23) | (m << 13);
        }
    } else if (e == 31) out = s | 0x7F800000 | (m << 13);
    else { e = e + (127 - 15); out = s | (e << 23) | (m << 13); }
 
    float f;
    __builtin_memcpy(&f, &out, 4);
    return f;
}
 
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
#define SUB_STEP_DT_MAX       0.016f
#define RESTITUTION_THRESHOLD 1.8f
typedef struct { Vector3 center; Vector3 halfExtents; Quaternion rot; } ShapeBox;
typedef struct { Vector3 center; float radius; }                        ShapeSphere;
typedef struct { Vector3 tip,base; float radius; }                      ShapeCapsule;
static inline u16 CellIndex(i32 cx, i32 cz) { return (u16)((cz * WORLDX) + cx); }
 
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
 
static void SolveContact(ContactManifold *m,float dt) {
    Entity *eA=&Sys_Global.instances[m->idxA], *eB=&Sys_Global.instances[m->idxB];
    float imA=entity_invmass(eA), imB=entity_invmass(eB);
    if (imA+imB<1e-10f) return;
 
    float restitution, friction;
    {
        float rA=eA->bounciness, rB=eB->bounciness;
        restitution = (eA->bounceCombine==PHYS_COMBINE_MAX||eB->bounceCombine==PHYS_COMBINE_MAX) ? vmax(rA,rB)
                    : (eA->bounceCombine==PHYS_COMBINE_MIN||eB->bounceCombine==PHYS_COMBINE_MIN) ? vmin(rA,rB)
                    : (eA->bounceCombine==PHYS_COMBINE_MUL||eB->bounceCombine==PHYS_COMBINE_MUL) ? rA*rB
                    : (rA+rB)*0.5f;
        float fA=(eA->dynamicFriction+eA->staticFriction)*0.5f, fB=(eB->dynamicFriction+eB->staticFriction)*0.5f;
        friction = (eA->frictionCombine==PHYS_COMBINE_MAX||eB->frictionCombine==PHYS_COMBINE_MAX) ? vmax(fA,fB)
                 : (eA->frictionCombine==PHYS_COMBINE_MIN||eB->frictionCombine==PHYS_COMBINE_MIN) ? vmin(fA,fB)
                 : (eA->frictionCombine==PHYS_COMBINE_MUL||eB->frictionCombine==PHYS_COMBINE_MUL) ? fA*fB
                 : (fA+fB)*0.5f;
    }
 
    for (int ci=0;ci<(int)m->count;++ci) {
        Contact *c=&m->contacts[ci];
        Vector3 relVel = Vector3_A_minus_B(eA->velocity,eB->velocity);
        float vn = dot_vector3(relVel,c->normal);
        if (vn > SPECULATIVE_MARGIN/dt) continue;
        float effectiveRestitution = (vabs(vn)>RESTITUTION_THRESHOLD) ? restitution : 0.0f;
        float jnDenom = imA+imB;
        float jn = -(1.0f+effectiveRestitution)*vn / jnDenom;
        float pen = c->depth - BAUMGARTE_SLOP;
        if (pen>0.0f) jn += (BAUMGARTE_FACTOR*pen/dt) / jnDenom;
        float newLN = vmax(0.0f, c->lambdaN+jn);
        float dLN   = newLN-c->lambdaN;
        c->lambdaN  = newLN;
        Vector3 impulseN=scale_vector3(c->normal,dLN);
        eA->velocity=Vector3_A_plus_B(eA->velocity,scale_vector3(impulseN, imA));
        eB->velocity=Vector3_A_minus_B(eB->velocity,scale_vector3(impulseN, imB));
        Vector3 rA=Vector3_A_minus_B(c->pointWorld,Vector3_A_plus_B(eA->position,quat_rotate_vector(eA->rotation,eA->colliderCenter)));
        Vector3 rB=Vector3_A_minus_B(c->pointWorld,Vector3_A_plus_B(eB->position,quat_rotate_vector(eB->rotation,eB->colliderCenter)));
        ApplyAngularImpulse(eA,rA,impulseN, 1.0f);
        ApplyAngularImpulse(eB,rB,impulseN,-1.0f);
        relVel = Vector3_A_minus_B(eA->velocity,eB->velocity);
        Vector3 tangent = Vector3_A_minus_B(relVel,scale_vector3(c->normal,dot_vector3(relVel,c->normal)));
        float tLen=magnitude_vector3(tangent);
        if (tLen>1e-6f) {
            tangent=scale_vector3(tangent,1.0f/tLen);
            float vt=dot_vector3(relVel,tangent);
            float jt=-vt/(imA+imB);
            float maxFric=friction*newLN;
            float newLT=vclamp(c->lambdaT+jt,-maxFric,maxFric);
            float dLT=newLT-c->lambdaT; c->lambdaT=newLT;
            Vector3 impulseT=scale_vector3(tangent,dLT);
            eA->velocity=Vector3_A_plus_B(eA->velocity,scale_vector3(impulseT, imA));
            eB->velocity=Vector3_A_minus_B(eB->velocity,scale_vector3(impulseT, imB));
            ApplyAngularImpulse(eA,rA,impulseT, 1.0f);
            ApplyAngularImpulse(eB,rB,impulseT,-1.0f);
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
 
// ─── broadphase: cell-neighbor pair collection ────────────────────────────────
// Since no object exceeds CELL_SIZE (2.56f) on any axis, only the 3×3 XZ
// neighbourhood (9 cells) around a dynamic object needs to be searched.
// Static vs static pairs are skipped.  j<i dedup is done for dynamic–dynamic.
#define MAX_PAIRS 4096
static u16 g_pairA[MAX_PAIRS], g_pairB[MAX_PAIRS];
static u16 g_pairCount;
 
// Per-cell instance list — rebuilt each broadphase tick.
// Sized for the world grid; cells hold up to 32 instances before overflow
// (overflow falls back to checking without the spatial index).
#define CELL_BUCKET_MAX 32
typedef struct {
    u16 count;
    u16 idx[CELL_BUCKET_MAX];
} CellBucket;
static CellBucket g_cellBuckets[ARRSIZE]; // zero-initialised by BSS
 
static void CollectBroadphasePairs(void) {
    g_pairCount = 0;
    u16 n = Sys_Global.loadedInstances;
 
    // Clear only the cells that were touched last frame — O(instances) not O(grid).
    // We track which cells were written so we can clear them without a full memset.
    static u16 g_dirtyCells[INSTANCE_COUNT];
    static u16 g_dirtyCellCount = 0;
    for (u16 d = 0; d < g_dirtyCellCount; ++d)
        g_cellBuckets[g_dirtyCells[d]].count = 0;
    g_dirtyCellCount = 0;
 
    // Build cell buckets for ALL active, collideable instances (dynamic and static).
    for (u16 i = START_INDEX_LEVEL_INSTANCES; i < n; ++i) {
        Entity *e = &Sys_Global.instances[i];
        if (!(e->entflags & ENTFLAG_ACTIVE))    continue;
        if (  e->collider == COLLIDER_TYPE_NONE) continue;
        i32 cx = PosGetCellCoordX(e->position.x), cz = PosGetCellCoordZ(e->position.z);
        if (cx < 0 || cx >= WORLDX || cz < 0 || cz >= WORLDZ) continue;
        u16 ci = CellIndex(cx, cz);
        CellBucket *b = &g_cellBuckets[ci];
        if (b->count == 0) {
            if (g_dirtyCellCount < INSTANCE_COUNT) g_dirtyCells[g_dirtyCellCount++] = ci;
        }
        if (b->count < CELL_BUCKET_MAX) b->idx[b->count++] = i;
    }
 
    // Collect pairs: for each awake rigidbody, check its 3×3 neighbourhood.
    for (u16 i = START_INDEX_LEVEL_INSTANCES; i < n; ++i) {
        Entity *eA = &Sys_Global.instances[i];
        if (!(eA->entflags & ENTFLAG_ACTIVE))    continue;
        if (!(eA->entflags & ENTFLAG_RIGIDBODY)) continue;
        if (  eA->entflags & ENTFLAG_ASLEEP)      continue;
        if (  eA->collider == COLLIDER_TYPE_NONE)  continue;
 
        i32 acx = PosGetCellCoordX(eA->position.x), acz = PosGetCellCoordZ(eA->position.z);
        u32 maskA = GetCollisionMask(eA->layer);
 
        for (i32 dz = -1; dz <= 1; ++dz) {
            i32 ncz = acz + dz;
            if (ncz < 0 || ncz >= WORLDZ) continue;
            for (i32 dx = -1; dx <= 1; ++dx) {
                i32 ncx = acx + dx;
                if (ncx < 0 || ncx >= WORLDX) continue;
                CellBucket *b = &g_cellBuckets[CellIndex(ncx, ncz)];
                for (u16 bi = 0; bi < b->count; ++bi) {
                    u16 j = b->idx[bi];
                    if (j == i) continue;
                    Entity *eB = &Sys_Global.instances[j];
                    if (!(eB->entflags & ENTFLAG_ACTIVE))    continue;
                    if (  eB->collider == COLLIDER_TYPE_NONE) continue;
                    if (!(maskA & eB->layer))                 continue;
                    // Skip if both dynamic and j already processed against i
                    if ((eB->entflags & ENTFLAG_RIGIDBODY) && j < i) continue;
                    if (g_pairCount >= MAX_PAIRS) goto pairs_full;
                    g_pairA[g_pairCount] = i; g_pairB[g_pairCount] = j; ++g_pairCount;
                }
            }
        }
    }
pairs_full:;
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
 
    CollectBroadphasePairs();
    for (u16 m=0;m<g_manifoldCount;) {
        bool found=false;
        for (u16 p=0;p<g_pairCount;++p)
            if ((g_manifolds[m].idxA==g_pairA[p] && g_manifolds[m].idxB==g_pairB[p]) ||
                (g_manifolds[m].idxA==g_pairB[p] && g_manifolds[m].idxB==g_pairA[p]))
                { found=true; break; }
        if (!found) { g_manifolds[m]=g_manifolds[--g_manifoldCount]; }
        else ++m;
    }
    for (u16 p=0;p<g_pairCount;++p) GenerateManifold(g_pairA[p],g_pairB[p]);
 
    for (u16 i=START_INDEX_LEVEL_INSTANCES;i<Sys_Global.loadedInstances;++i)
        if (Sys_Global.instances[i].entflags & ENTFLAG_RIGIDBODY) SpeculativePreClamp(i,dt);
 
    for (int iter=0;iter<SOLVER_ITERATIONS;++iter)
        for (u16 m=0;m<g_manifoldCount;++m) SolveContact(&g_manifolds[m],dt);
 
    for (u16 i=START_INDEX_LEVEL_INSTANCES;i<Sys_Global.loadedInstances;++i)
        if (Sys_Global.instances[i].entflags & ENTFLAG_RIGIDBODY) IntegrateAngularVelocity(i,dt);
 
    for (u16 i=START_INDEX_LEVEL_INSTANCES;i<Sys_Global.loadedInstances;++i)
        if (Sys_Global.instances[i].entflags & ENTFLAG_RIGIDBODY) UpdateSleep(i,dt);
}
 
void Physics_ResetForLevelLoad(void) {
    __builtin_memset(g_manifolds,    0, sizeof(g_manifolds));
    __builtin_memset(g_sleepCounter, 0, sizeof(g_sleepCounter));
    __builtin_memset(g_pairA,        0, sizeof(g_pairA));
    __builtin_memset(g_pairB,        0, sizeof(g_pairB));
    g_manifoldCount = 0;
    g_pairCount     = 0;
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
    u32 curIDs[MAX_DEBUG_MANIFOLD_IDS];
    u16 curCount = 0;
    for (u16 m = 0; m < g_manifoldCount && curCount < MAX_DEBUG_MANIFOLD_IDS; ++m)
        curIDs[curCount++] = ManifoldID(g_manifolds[m].idxA, g_manifolds[m].idxB);
 
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
    for (u16 m = 0; m < g_manifoldCount; ++m)
        DebugDrawManifoldNormals(&g_manifolds[m]);
 
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
    for (u16 i = START_INDEX_LEVEL_INSTANCES; i < INSTANCE_COUNT; ++i) {
        if (!(layerMask & Sys_Global.instances[i].layer)) continue;
        u16 mindex = Sys_Global.instances[i].modelIndex;
        if (mindex >= loadedModelsMaxIndex) continue;
        u32 triCount = modelTriangleCounts[mindex];
        if (triCount < 1) continue;
 
        float M[16]; __builtin_memcpy(M,&modelMatrices[i * 16],16 * sizeof(float));
        float m00=M[0], m10=M[1], m20=M[2]; float m01=M[4], m11=M[5], m21=M[6];
        float m02=M[8], m12=M[9], m22=M[10]; float tx=M[12], ty=M[13], tz=M[14];
        float scl_x = vsqrtf(m00*m00 + m10*m10 + m20*m20);
        float scl_y = vsqrtf(m01*m01 + m11*m11 + m21*m21);
        float scl_z = vsqrtf(m02*m02 + m12*m12 + m22*m22);
        if (scl_x < 1e-6f || scl_y < 1e-6f || scl_z < 1e-6f) continue;
 
        Vector3 objPos = Sys_Global.instances[i].position;
        Vector3 capsuleMid = { (start.x+end.x)*0.5f, (start.y+end.y)*0.5f, (start.z+end.z)*0.5f };
        Vector3 delta = Vector3_A_minus_B(objPos, capsuleMid);
        float distSqrd = delta.x*delta.x + delta.y*delta.y + delta.z*delta.z;
        float modelRad = vmax(modelBounds[mindex], 1.81f);
        Vector3 spine = Vector3_A_minus_B(end, start);
        float spineHalf = magnitude_vector3(spine) * 0.5f;
        float combinedRad = modelRad + spineHalf + capsuleRadius + 0.1f;
        if (distSqrd > combinedRad*combinedRad) continue;
 
        Vector3 relS = { start.x-tx, start.y-ty, start.z-tz };
        Vector3 localStart = { (relS.x*m00 + relS.y*m10 + relS.z*m20) / (scl_x*scl_x), (relS.x*m01 + relS.y*m11 + relS.z*m21) / (scl_y*scl_y), (relS.x*m02 + relS.y*m12 + relS.z*m22) / (scl_z*scl_z)};
        Vector3 relE = {end.x-tx,end.y-ty,end.z-tz};
        Vector3 localEnd = {(relE.x*m00 + relE.y*m10 + relE.z*m20) / (scl_x*scl_x), (relE.x*m01 + relE.y*m11 + relE.z*m21) / (scl_y*scl_y), (relE.x*m02 + relE.y*m12 + relE.z*m22) / (scl_z*scl_z)};
        float minScl = scl_x;
        if (scl_y < minScl) { minScl = scl_y; } if (scl_z < minScl) { minScl = scl_z; }
        float localRadius = capsuleRadius / minScl;
        for (u32 j = 0; j < triCount; ++j) {
            u32 bA = (u32)modelTriangles[mindex][j*3 + 0] * VERTEX_ATTRIBUTES_SIZE, bB = (u32)modelTriangles[mindex][j*3 + 1] * VERTEX_ATTRIBUTES_SIZE, bC = (u32)modelTriangles[mindex][j*3 + 2] * VERTEX_ATTRIBUTES_SIZE;
            Vector3 posA = {half_to_float( *(half*)(modelVertices[mindex] + bA + 0) ), half_to_float( *(half*)(modelVertices[mindex] + bA + 2) ), half_to_float( *(half*)(modelVertices[mindex] + bA + 4) )};
            Vector3 posB = {half_to_float( *(half*)(modelVertices[mindex] + bB + 0) ), half_to_float( *(half*)(modelVertices[mindex] + bB + 2) ), half_to_float( *(half*)(modelVertices[mindex] + bB + 4) )};
            Vector3 posC = {half_to_float( *(half*)(modelVertices[mindex] + bC + 0) ), half_to_float( *(half*)(modelVertices[mindex] + bC + 2) ), half_to_float( *(half*)(modelVertices[mindex] + bC + 4) )};
            Vector3 cpP   = ClosestPointOnTriangle(posA,posB,posC,localStart);
            Vector3 spP   = ClosestPointOnSegment(localStart,localEnd,cpP);
            Vector3 cpP2  = ClosestPointOnTriangle(posA,posB,posC,spP);
            Vector3 cpQ   = ClosestPointOnTriangle(posA,posB,posC,localEnd);
            Vector3 spQ   = ClosestPointOnSegment(localStart,localEnd,cpQ);
            Vector3 cpQ2  = ClosestPointOnTriangle(posA,posB,posC,spQ);
            Vector3 dP = Vector3_A_minus_B(spP,cpP2); Vector3 dQ = Vector3_A_minus_B(spQ,cpQ2);
            float distP = vsqrtf(dot_vector3(dP,dP));
            float distQ = vsqrtf(dot_vector3(dQ,dQ));
            float localDist; Vector3 localContactVec;
            if (distP <= distQ) { localDist = distP; localContactVec = dP; }
            else                { localDist = distQ; localContactVec = dQ; }
 
            float localPen = localRadius - localDist;
            if (localPen <= 0.0f) continue;
 
            Vector3 localNormal;
            if (localDist > 1e-6f) {
                localNormal = (Vector3){ localContactVec.x / localDist, localContactVec.y / localDist, localContactVec.z / localDist };
            } else {
                Vector3 eAB = Vector3_A_minus_B(posB, posA); Vector3 eAC = Vector3_A_minus_B(posC, posA);
                localNormal = normalize_vector3(cross_vector3(eAB, eAC));
                Vector3 spMid = { (localStart.x+localEnd.x)*0.5f, (localStart.y+localEnd.y)*0.5f, (localStart.z+localEnd.z)*0.5f };
                Vector3 toMid = Vector3_A_minus_B(spMid, posA);
                if (dot_vector3(localNormal, toMid) < 0.0f) { localNormal.x=-localNormal.x; localNormal.y=-localNormal.y; localNormal.z=-localNormal.z; }
            }
 
            Vector3 worldNormal = {(m00/scl_x)*localNormal.x + (m01/scl_y)*localNormal.y + (m02/scl_z)*localNormal.z, (m10/scl_x)*localNormal.x + (m11/scl_y)*localNormal.y + (m12/scl_z)*localNormal.z, (m20/scl_x)*localNormal.x + (m21/scl_y)*localNormal.y + (m22/scl_z)*localNormal.z};
            worldNormal = normalize_vector3(worldNormal);
            float worldPen = localPen * minScl;
            if (worldPen > worst.depth) { worst.depth = worldPen; worst.normal = worldNormal; }
        }
    }
    return worst;
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
 
const Vector3 gravityVelocity = { 0.0f, -0.981f, 0.0f };
 
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
 
bool GridCellBlock(u16 i,Vector3 pos,Vector3 newPos);
static void IntegrateRigidbody(u16 i,float dt){
    Entity*e=&Sys_Global.instances[i];
    if(!(e->entflags&ENTFLAG_ACTIVE))return;
    if(!(e->entflags&ENTFLAG_RIGIDBODY))return;
    if(e->entflags&ENTFLAG_ASLEEP)return;
    if(e->entflags&ENTFLAG_KINEMATIC)return;
    float mag=magnitude_vector3(e->velocity);
    if(mag<0.005f)return;
 
    Vector3 pos=e->position; Vector3 newPos=Vector3_A_plus_B(pos,scale_vector3(e->velocity,dt));
    if(GridCellBlock(i,pos,newPos))return;
    u32 mask=GetCollisionMask(e->layer); Vector3 s,en; CapsuleTipsFromEye(newPos,&s,&en);
    CapsuleContact c = QueryCapsuleContact(s,en,PLAYER_RADIUS,mask);
    if(c.depth>0.0f){ newPos=Vector3_A_plus_B(newPos,scale_vector3(c.normal,c.depth+0.001f)); }
    e->lastPosition=pos; e->position=newPos; e->cellIndex=PosGetCellCoords(newPos.x,newPos.z);
    Sys_Global.dirtyInstances[i]=true; float angMag=magnitude_vector3(e->angularVelocity);
    if(angMag>0.0001f){
        float angle=angMag*dt;
        Vector3 axis=normalize_vector3(e->angularVelocity);
        Quaternion dq={axis.x*vsinf(angle*0.5f),axis.y*vsinf(angle*0.5f),axis.z*vsinf(angle*0.5f),vcosf(angle*0.5f)};
        e->rotation=quat_multiply(dq,e->rotation); float qmag=quat_dot(e->rotation,e->rotation);
        if(qmag>0.0001f){ float inv=1.0f/vsqrtf(qmag); e->rotation.x*=inv; e->rotation.y*=inv; e->rotation.z*=inv; e->rotation.w*=inv; }
        e->angularVelocity=scale_vector3(e->angularVelocity,1.0f-e->angularDrag*dt);
    }
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
    float dt=(float)Sys_Global.timeSinceLastPhysicsTick;
    for (u32 i=PLAYER1;i<PLAYER2/*Sys_Global.loadedInstances*/;++i) {
        if (i<=PLAYER2)                                              IntegratePlayer((u16)i,dt);
        else if (Sys_Global.instances[i].entflags&ENTFLAG_RIGIDBODY) IntegrateRigidbody((u16)i,dt);
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
 
void UpdateTriggers(void) {
    // Separate system, simple AABB checks.
}
 
void Physics(void) {
    UpdateVelocityFromGravity();
//     Physics_PrimitiveStep((float)Sys_Global.timeSinceLastPhysicsTick);
    ClampVelocity();
    UpdatePositions();
    UpdateTriggers();
//     Physics_DrawDebug(); // no-op when physicsDebug == 0
}
 
// ─── raycasting ───────────────────────────────────────────────────────────────
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
 
extern u16 playerCellIdx; bool SkyIsVisible(void); bool LevelSpecificHacksForClosedCellsThatProbablyShouldntBeBecauseOfInsetMeshes(u32 instCellIdx, u16 constIndex);
ENGINE_TO_MOD RaycastHit Raycast(Vector3 origin, Vector3 dir, float maxDist, u32 layerMask) {
    u32 numMeshesCheckedForRaycast = 0, numTrisCastAgainst = 0;
    RaycastHit result = { .hit = false, .distance = maxDist, .point = {0.0f, 0.0f, 0.0f}, .normal = {0.0f, 0.0f, 0.0f}, .hitInstanceIndex = INSTANCE_COUNT };
    for (u16 i = START_INDEX_LEVEL_INSTANCES; i < INSTANCE_COUNT; ++i) {
        if (!(layerMask & Sys_Global.instances[i].layer)) continue;
        u16 mindex = Sys_Global.instances[i].modelIndex;
        if (mindex >= loadedModelsMaxIndex) continue;
        Vector3 objPos = Sys_Global.instances[i].position;
        u16 instCellIdx = PosGetCellCoords(objPos.x,objPos.z);
        Vector3 delta = Vector3_A_minus_B(objPos,origin);
        float distSqrd = delta.x*delta.x + delta.y*delta.y + delta.z*delta.z;
        float radBounds = vmax(modelBounds[mindex], 1.81f);
        float maxDistToObj = vmax(maxDist - radBounds,maxDist);
        if (distSqrd >= (maxDistToObj * maxDistToObj)) continue;
        if (LevelSpecificHacksForClosedCellsThatProbablyShouldntBeBecauseOfInsetMeshes(instCellIdx,Sys_Global.instances[i].index)) continue;
        
        u32 triCount = modelTriangleCounts[mindex];
        if (triCount < 1) continue;
        float M[16];
        __builtin_memcpy(M,&modelMatrices[i * 16],16 * sizeof(float));
        float m00=M[0], m10=M[1], m20=M[2];
        float m01=M[4], m11=M[5], m21=M[6];
        float m02=M[8], m12=M[9], m22=M[10];
        float tx=M[12], ty=M[13], tz=M[14];
        float sclx = vsqrtf(m00*m00 + m10*m10 + m20*m20); float sclx2 = sclx * sclx;
        float scly = vsqrtf(m01*m01 + m11*m11 + m21*m21); float scly2 = scly * scly;
        float sclz = vsqrtf(m02*m02 + m12*m12 + m22*m22); float sclz2 = sclz * sclz;
        Vector3 rel = {origin.x - tx, origin.y - ty, origin.z - tz};
        Vector3 localOrigin = {(rel.x*m00 + rel.y*m10 + rel.z*m20) / sclx2, (rel.x*m01 + rel.y*m11 + rel.z*m21) / scly2, (rel.x*m02 + rel.y*m12 + rel.z*m22) / sclz2};
        Vector3 localDir =    {(dir.x*m00 + dir.y*m10 + dir.z*m20) / sclx2, (dir.x*m01 + dir.y*m11 + dir.z*m21) / scly2, (dir.x*m02 + dir.y*m12 + dir.z*m22) / sclz2};
        localDir = normalize_vector3(localDir);
        numMeshesCheckedForRaycast++;
        for (u32 j=0;j<triCount;++j) {
            u32 bA = (u32)modelTriangles[mindex][j*3 + 0] * VERTEX_ATTRIBUTES_SIZE, bB = (u32)modelTriangles[mindex][j*3 + 1] * VERTEX_ATTRIBUTES_SIZE, bC = (u32)modelTriangles[mindex][j*3 + 2] * VERTEX_ATTRIBUTES_SIZE;
            Vector3 posA = {half_to_float( *(half*)(modelVertices[mindex] + bA + 0) ), half_to_float( *(half*)(modelVertices[mindex] + bA + 2) ), half_to_float( *(half*)(modelVertices[mindex] + bA + 4) )};
            Vector3 posB = {half_to_float( *(half*)(modelVertices[mindex] + bB + 0) ), half_to_float( *(half*)(modelVertices[mindex] + bB + 2) ), half_to_float( *(half*)(modelVertices[mindex] + bB + 4) )};
            Vector3 posC = {half_to_float( *(half*)(modelVertices[mindex] + bC + 0) ), half_to_float( *(half*)(modelVertices[mindex] + bC + 2) ), half_to_float( *(half*)(modelVertices[mindex] + bC + 4) )};
            Vector3 normA ={half_to_float( *(half*)(modelVertices[mindex] + bA + 6) ), half_to_float( *(half*)(modelVertices[mindex] + bA + 8) ), half_to_float( *(half*)(modelVertices[mindex] + bA + 10) )};
            Vector3 normB ={half_to_float( *(half*)(modelVertices[mindex] + bB + 6) ), half_to_float( *(half*)(modelVertices[mindex] + bB + 8) ), half_to_float( *(half*)(modelVertices[mindex] + bB + 10) )};
            Vector3 normC ={half_to_float( *(half*)(modelVertices[mindex] + bC + 6) ), half_to_float( *(half*)(modelVertices[mindex] + bC + 8) ), half_to_float( *(half*)(modelVertices[mindex] + bC + 10) )};
            RaycastHit tryTri = RayTriangle(localOrigin,localDir,posA,posB,posC,normA,normB,normC);
            numTrisCastAgainst++;
            if (!tryTri.hit) continue;
            Vector3 worldPoint = {
                m00*tryTri.point.x + m01*tryTri.point.y + m02*tryTri.point.z + tx,
                m10*tryTri.point.x + m11*tryTri.point.y + m12*tryTri.point.z + ty,
                m20*tryTri.point.x + m21*tryTri.point.y + m22*tryTri.point.z + tz
            };
            Vector3 toHit = Vector3_A_minus_B(worldPoint, origin);
            float worldDist = vsqrtf(toHit.x*toHit.x + toHit.y*toHit.y + toHit.z*toHit.z);
            if (worldDist >= result.distance) continue;
            Vector3 worldNormal = {
                (m00/sclx)*tryTri.normal.x + (m01/scly)*tryTri.normal.y + (m02/sclz)*tryTri.normal.z,
                (m10/sclx)*tryTri.normal.x + (m11/scly)*tryTri.normal.y + (m12/sclz)*tryTri.normal.z,
                (m20/sclx)*tryTri.normal.x + (m21/scly)*tryTri.normal.y + (m22/sclz)*tryTri.normal.z
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
 
ENGINE_TO_MOD void RaycastAll(Vector3 origin, Vector3 dir, float distance, u32 layerMask, RaycastHit* hits, u16 maxCount) {
    for (int i=0;i<maxCount;++i) hits[i].hit = false;
    (void)origin; (void)dir; (void)distance; (void)layerMask;
}
 
ENGINE_TO_MOD RaycastHit CapsuleCast(Vector3 start, Vector3 end, float capsuleRadius, float castDist, u32 layerMask, bool hitTriggers) { RaycastHit result = { .hit = false }; (void)start; (void)end; (void)capsuleRadius; (void)castDist; (void)layerMask; (void)hitTriggers; return result; }
