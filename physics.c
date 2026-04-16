// physics.c
#include "voxen.h"
typedef u16 half;
static inline __attribute__((always_inline)) float half_to_float(half h){
    u32 s=(h&0x8000)<<16,e=(h&0x7C00)>>10,m=(h&0x03FF),out;
    if (e == 0){
        if (m == 0) out = s;
        else { // normalize subnormal
            e = 1;
            while ((m & 0x0400) == 0) { m <<= 1; e--; }
            m &= 0x03FF; e+=(127 - 15);
            out = s | (e << 23) | (m << 13);
        }
    } else if (e == 31) { out = s | 0x7F800000 | (m << 13); }
    else { e = e + (127 - 15); out = s | (e << 23) | (m << 13); }
    float f; __builtin_memcpy(&f,&out,4);
    return f;
}

#include "ray.c"
#include "trigger.c"
extern u16 loadedModelsMaxIndex,modelTriangleCounts[MODEL_IDX_MAX]; extern u8** modelVertices; extern u16** modelTriangles;
extern u32 modelVertexCounts[MODEL_IDX_MAX]; extern float modelMatrices[INSTANCE_COUNT*16],modelBounds[MODEL_IDX_MAX];
extern u32 gridCellStates[ARRSIZE];
#define MAX_CONTACTS_PER_PAIR 4
#define MAX_MESH_CONTACTS MAX_CONTACTS_PER_PAIR
#define MAX_MANIFOLDS 2048
#define CELL_BUCKET_CAP 8
#define MANIFOLD_HT_SIZE 4096
#define MANIFOLD_HT_MASK (MANIFOLD_HT_SIZE-1)
#define SOLVER_ITERATIONS     6
#define SLEEP_KE_THRESHOLD    0.004f
#define SLEEP_FRAMES_NEEDED   20
#define SPECULATIVE_MARGIN    0.005f
#define BAUMGARTE_FACTOR      0.2f
#define BAUMGARTE_SLOP        0.002f
#define SUB_STEP_DT_MAX       0.027777778f
#define RESTITUTION_THRESHOLD 1.8f
#define GROUND_PROBE_DIST 0.02f
#define SNAP_STEP 0.005f
#define SNAP_MAX  0.25f
#define SLOPE_WALK_MAX_DEG        45.0f
#define SLOPE_CLIMB_MAX_DEG       55.0f
#define SLOPE_SLIDE_ACCEL         8.0f
#define SLOPE_SLIDE_ACCEL_BOOST  14.0f
#define SLOPE_FRICTION_ACCEL      1.0f
#define SLOPE_FRICTION_ACCEL_BOOST 0.3f
typedef struct { float depth; Vector3 normal; } CapsuleContact;
typedef struct { Vector3 center,halfExtents; Quaternion rot; } ShapeBox;
typedef struct { Vector3 center; float radius; }               ShapeSphere;
typedef struct { Vector3 tip,base; float radius; }             ShapeCapsule;
typedef struct { Vector3 pointWorld,normal; float depth,lambdaN,lambdaT; } Contact;
typedef struct { u16 idxA,idxB; u8 count; Contact contacts[MAX_CONTACTS_PER_PAIR]; } ContactManifold;
typedef struct { u16 idx[CELL_BUCKET_CAP]; u8 count; } CellBucket;
ContactManifold g_manifolds[MAX_MANIFOLDS];
u16 g_manifoldCount = 0;
static CellBucket g_cellBuckets[ARRSIZE];
static void BuildCellBuckets(u16 n) {
    __builtin_memset(g_cellBuckets,0,sizeof(g_cellBuckets));
    for (u16 i=START_INDEX_LEVEL_INSTANCES; i<n; ++i) {
        Entity *e=&Sys_Global.instances[i];
        if (!(e->entflags&ENTFLAG_ACTIVE) || e->collider==COLLIDER_TYPE_NONE) continue;
        u32 ci=e->cellIndex;
        if (ci>=ARRSIZE) continue;
        CellBucket *b=&g_cellBuckets[ci];
        if (b->count<CELL_BUCKET_CAP) b->idx[b->count++]=i;
    }
}

static u32 g_manifoldHT[MANIFOLD_HT_SIZE];
static inline u32 ManifoldKey(u16 a,u16 b) { if (a>b) { u16 t=a; a=b; b=t; } return (u32)a*7681u+b; }
static ContactManifold* FindOrCreateManifold(u16 idxA,u16 idxB) {
    u32 slot=ManifoldKey(idxA,idxB)&MANIFOLD_HT_MASK;
    for (u32 p=0; p<16; ++p, slot=(slot+1)&MANIFOLD_HT_MASK) {
        u32 v=g_manifoldHT[slot];
        if (v==0) {
            if (g_manifoldCount>=MAX_MANIFOLDS) return NULL;
            u16 mi=g_manifoldCount++;
            g_manifoldHT[slot]=(u32)mi+1;
            ContactManifold *m=&g_manifolds[mi];
            m->idxA=idxA; m->idxB=idxB; m->count=0;
            return m;
        }
        ContactManifold *m=&g_manifolds[v-1];
        if ((m->idxA==idxA&&m->idxB==idxB)||(m->idxA==idxB&&m->idxB==idxA)) return m;
    }
    return NULL;
}

static void ResetManifoldTable(void) { __builtin_memset(g_manifoldHT,0,sizeof(g_manifoldHT)); g_manifoldCount=0; }

static inline Vector3 ClosestPointOnSegment(Vector3 p,Vector3 q,Vector3 a) {
    Vector3 pq=Vector3_A_minus_B(q,p), pa=Vector3_A_minus_B(a,p);
    float len2=dot_vector3(pq,pq);
    if (len2<1e-10f) return p;
    float t=vmax(0.0f,vmin(1.0f,dot_vector3(pa,pq)/len2));
    return Vector3_A_plus_B(p,scale_vector3(pq,t));
}

static inline Vector3 ClosestPointOnTriangle(Vector3 a,Vector3 b,Vector3 c,Vector3 p) {
    Vector3 ab=Vector3_A_minus_B(b,a), ac=Vector3_A_minus_B(c,a), ap=Vector3_A_minus_B(p,a);
    float d1=dot_vector3(ab,ap), d2=dot_vector3(ac,ap);
    if (d1<=0.0f&&d2<=0.0f) return a;
    Vector3 bp=Vector3_A_minus_B(p,b);
    float d3=dot_vector3(ab,bp), d4=dot_vector3(ac,bp);
    if (d3>=0.0f&&d4<=d3) return b;
    Vector3 cp=Vector3_A_minus_B(p,c);
    float d5=dot_vector3(ab,cp), d6=dot_vector3(ac,cp);
    if (d6>=0.0f&&d5<=d6) return c;
    float vc=d1*d4-d3*d2;
    if (vc<=0.0f&&d1>=0.0f&&d3<=0.0f) return Vector3_A_plus_B(a,scale_vector3(ab,d1/(d1-d3)));
    float vb=d5*d2-d1*d6;
    if (vb<=0.0f&&d2>=0.0f&&d6<=0.0f) return Vector3_A_plus_B(a,scale_vector3(ac,d2/(d2-d6)));
    float va=d3*d6-d5*d4;
    if (va<=0.0f&&(d4-d3)>=0.0f&&(d5-d6)>=0.0f) return Vector3_A_plus_B(b,scale_vector3(Vector3_A_minus_B(c,b),(d4-d3)/((d4-d3)+(d5-d6))));
    float denom=1.0f/(va+vb+vc), v=vb*denom, w=vc*denom;
    return Vector3_A_plus_B(Vector3_A_plus_B(a,scale_vector3(ab,v)),scale_vector3(ac,w));
}

static u32 GetCollisionMask(u32 layer) {
    switch (layer) {
        case PhysicsLayer_Default:           return PhysicsLayer_Default|PhysicsLayer_TransparentFX|PhysicsLayer_IgnoreRaycast|PhysicsLayer_Geometry|PhysicsLayer_NPC|PhysicsLayer_PlayerBullets|PhysicsLayer_Player|PhysicsLayer_Corpse|PhysicsLayer_PhysObjects|PhysicsLayer_Sky|PhysicsLayer_Trigger|PhysicsLayer_Door|PhysicsLayer_InterDebris|PhysicsLayer_Player2|PhysicsLayer_Player3|PhysicsLayer_Player4|PhysicsLayer_NPCBullet|PhysicsLayer_Clip|PhysicsLayer_CorpseSearchable;
        case PhysicsLayer_TransparentFX:     return PhysicsLayer_Default|PhysicsLayer_TransparentFX|PhysicsLayer_IgnoreRaycast|PhysicsLayer_Geometry|PhysicsLayer_NPC|PhysicsLayer_PlayerBullets|PhysicsLayer_Player|PhysicsLayer_PhysObjects|PhysicsLayer_Trigger|PhysicsLayer_Door|PhysicsLayer_InterDebris|PhysicsLayer_Player2|PhysicsLayer_NPCBullet|PhysicsLayer_Clip;
        case PhysicsLayer_IgnoreRaycast:     return PhysicsLayer_Default|PhysicsLayer_TransparentFX|PhysicsLayer_IgnoreRaycast|PhysicsLayer_Geometry|PhysicsLayer_NPC|PhysicsLayer_PlayerBullets|PhysicsLayer_Player|PhysicsLayer_PhysObjects|PhysicsLayer_Trigger|PhysicsLayer_Door|PhysicsLayer_InterDebris|PhysicsLayer_Player2|PhysicsLayer_NPCBullet|PhysicsLayer_Clip;
        case PhysicsLayer_Water:             return 0u;
        case PhysicsLayer_UI:                return 0u;
        case PhysicsLayer_GunViewModel:      return 0u;
        case PhysicsLayer_Geometry:          return PhysicsLayer_Default|PhysicsLayer_TransparentFX|PhysicsLayer_IgnoreRaycast|PhysicsLayer_Geometry|PhysicsLayer_NPC|PhysicsLayer_PlayerBullets|PhysicsLayer_Player|PhysicsLayer_PhysObjects|PhysicsLayer_Trigger|PhysicsLayer_Door|PhysicsLayer_InterDebris|PhysicsLayer_Player2|PhysicsLayer_Clip;
        case PhysicsLayer_NPC:               return PhysicsLayer_Default|PhysicsLayer_TransparentFX|PhysicsLayer_IgnoreRaycast|PhysicsLayer_Geometry|PhysicsLayer_NPC|PhysicsLayer_PlayerBullets|PhysicsLayer_Player|PhysicsLayer_PhysObjects|PhysicsLayer_Trigger|PhysicsLayer_NPCTrigger|PhysicsLayer_Door|PhysicsLayer_InterDebris|PhysicsLayer_Player2|PhysicsLayer_NPCBullet|PhysicsLayer_NPCClip|PhysicsLayer_Clip;
        case PhysicsLayer_PlayerBullets:     return PhysicsLayer_Default|PhysicsLayer_TransparentFX|PhysicsLayer_IgnoreRaycast|PhysicsLayer_Geometry|PhysicsLayer_NPC|PhysicsLayer_PlayerBullets|PhysicsLayer_Player|PhysicsLayer_Corpse|PhysicsLayer_PhysObjects|PhysicsLayer_Door|PhysicsLayer_InterDebris|PhysicsLayer_Player2|PhysicsLayer_NPCBullet|PhysicsLayer_Clip|PhysicsLayer_CorpseSearchable;
        case PhysicsLayer_Player:            return PhysicsLayer_Default|PhysicsLayer_TransparentFX|PhysicsLayer_IgnoreRaycast|PhysicsLayer_Geometry|PhysicsLayer_NPC|PhysicsLayer_PhysObjects|PhysicsLayer_PlayerTriggerOnly|PhysicsLayer_Trigger|PhysicsLayer_Door|PhysicsLayer_Player2|PhysicsLayer_NPCBullet|PhysicsLayer_Clip;
        case PhysicsLayer_Corpse:            return PhysicsLayer_Default|PhysicsLayer_Geometry|PhysicsLayer_PlayerBullets|PhysicsLayer_PhysObjects|PhysicsLayer_Door|PhysicsLayer_NPCBullet|PhysicsLayer_Clip;
        case PhysicsLayer_PhysObjects:       return PhysicsLayer_Default|PhysicsLayer_TransparentFX|PhysicsLayer_IgnoreRaycast|PhysicsLayer_Geometry|PhysicsLayer_NPC|PhysicsLayer_PlayerBullets|PhysicsLayer_Player|PhysicsLayer_Corpse|PhysicsLayer_PhysObjects|PhysicsLayer_Door|PhysicsLayer_InterDebris|PhysicsLayer_NPCBullet|PhysicsLayer_Clip;
        case PhysicsLayer_Sky:               return PhysicsLayer_Default|PhysicsLayer_Player;
        case PhysicsLayer_PlayerTriggerOnly: return PhysicsLayer_Player|PhysicsLayer_Player2|PhysicsLayer_Player3;
        case PhysicsLayer_Trigger:           return PhysicsLayer_Default|PhysicsLayer_Geometry|PhysicsLayer_NPC|PhysicsLayer_PlayerBullets|PhysicsLayer_Player|PhysicsLayer_PhysObjects|PhysicsLayer_Door|PhysicsLayer_InterDebris|PhysicsLayer_Clip;
        case PhysicsLayer_Door:              return PhysicsLayer_Default|PhysicsLayer_TransparentFX|PhysicsLayer_IgnoreRaycast|PhysicsLayer_Geometry|PhysicsLayer_NPC|PhysicsLayer_PlayerBullets|PhysicsLayer_Player|PhysicsLayer_Corpse|PhysicsLayer_PhysObjects|PhysicsLayer_Trigger|PhysicsLayer_Door|PhysicsLayer_InterDebris|PhysicsLayer_Player2|PhysicsLayer_NPCBullet|PhysicsLayer_Clip;
        case PhysicsLayer_InterDebris:       return PhysicsLayer_Default|PhysicsLayer_Geometry|PhysicsLayer_NPC|PhysicsLayer_PlayerBullets|PhysicsLayer_PhysObjects|PhysicsLayer_Trigger|PhysicsLayer_Door|PhysicsLayer_NPCBullet|PhysicsLayer_Clip;
        case PhysicsLayer_Player2:           return PhysicsLayer_Default|PhysicsLayer_TransparentFX|PhysicsLayer_IgnoreRaycast|PhysicsLayer_Geometry|PhysicsLayer_NPC|PhysicsLayer_PlayerBullets|PhysicsLayer_Player|PhysicsLayer_PhysObjects|PhysicsLayer_PlayerTriggerOnly|PhysicsLayer_Trigger|PhysicsLayer_Door|PhysicsLayer_InterDebris|PhysicsLayer_Player2|PhysicsLayer_NPCBullet|PhysicsLayer_Clip;
        case PhysicsLayer_Player3:           return PhysicsLayer_Default|PhysicsLayer_TransparentFX|PhysicsLayer_IgnoreRaycast|PhysicsLayer_Geometry|PhysicsLayer_NPC|PhysicsLayer_PlayerBullets|PhysicsLayer_Player|PhysicsLayer_PhysObjects|PhysicsLayer_PlayerTriggerOnly|PhysicsLayer_Trigger|PhysicsLayer_Door|PhysicsLayer_InterDebris|PhysicsLayer_Player3|PhysicsLayer_NPCBullet|PhysicsLayer_Clip;
        case PhysicsLayer_Player4:           return PhysicsLayer_Default|PhysicsLayer_TransparentFX|PhysicsLayer_IgnoreRaycast|PhysicsLayer_Geometry|PhysicsLayer_NPC|PhysicsLayer_PlayerBullets|PhysicsLayer_Player|PhysicsLayer_PhysObjects|PhysicsLayer_PlayerTriggerOnly|PhysicsLayer_Trigger|PhysicsLayer_Door|PhysicsLayer_InterDebris|PhysicsLayer_Player4|PhysicsLayer_NPCBullet|PhysicsLayer_Clip;
        case PhysicsLayer_NPCTrigger:        return PhysicsLayer_NPC;
        case PhysicsLayer_NPCBullet:         return PhysicsLayer_Default|PhysicsLayer_TransparentFX|PhysicsLayer_IgnoreRaycast|PhysicsLayer_Geometry|PhysicsLayer_PlayerBullets|PhysicsLayer_Player|PhysicsLayer_Corpse|PhysicsLayer_PhysObjects|PhysicsLayer_Door|PhysicsLayer_InterDebris|PhysicsLayer_Player2|PhysicsLayer_Clip|PhysicsLayer_CorpseSearchable;
        case PhysicsLayer_NPCClip:           return PhysicsLayer_NPC;
        case PhysicsLayer_Clip:              return PhysicsLayer_Player|PhysicsLayer_Player2|PhysicsLayer_Player3|PhysicsLayer_Player4|PhysicsLayer_NPC;
        case PhysicsLayer_Automap:           return 0u;
        case PhysicsLayer_Culling:           return 0u;
        case PhysicsLayer_CorpseSearchable:  return PhysicsLayer_Default|PhysicsLayer_PlayerBullets;
        default:                             return 0u;
    }
}

static inline void obb_axes(Quaternion q,Vector3 *ax,Vector3 *ay,Vector3 *az) { *ax=quat_rotate_vector(q,(Vector3){1,0,0}); *ay=quat_rotate_vector(q,(Vector3){0,1,0}); *az=quat_rotate_vector(q,(Vector3){0,0,1}); }
static inline float obb_project(ShapeBox b,Vector3 axis) { Vector3 ax,ay,az; obb_axes(b.rot,&ax,&ay,&az); return b.halfExtents.x*vabs(dot_vector3(ax,axis))+b.halfExtents.y*vabs(dot_vector3(ay,axis))+b.halfExtents.z*vabs(dot_vector3(az,axis)); }
static inline Vector3 closest_point_obb(ShapeBox box,Vector3 p) {
    Vector3 ax,ay,az; obb_axes(box.rot,&ax,&ay,&az);
    Vector3 d=Vector3_A_minus_B(p,box.center);
    float tx=vclamp(dot_vector3(d,ax),-box.halfExtents.x,box.halfExtents.x), ty=vclamp(dot_vector3(d,ay),-box.halfExtents.y,box.halfExtents.y), tz=vclamp(dot_vector3(d,az),-box.halfExtents.z,box.halfExtents.z);
    return Vector3_A_plus_B(box.center,Vector3_A_plus_B(Vector3_A_plus_B(scale_vector3(ax,tx),scale_vector3(ay,ty)),scale_vector3(az,tz)));
}
static inline float seg_seg_closest(Vector3 p,Vector3 q,Vector3 a,Vector3 b,Vector3 *cpPQ,Vector3 *cpAB) {
    Vector3 d1=Vector3_A_minus_B(q,p), d2=Vector3_A_minus_B(b,a), r=Vector3_A_minus_B(p,a);
    float e=dot_vector3(d2,d2), f=dot_vector3(d2,r), c=dot_vector3(d1,d1), s, t;
    if (c<1e-10f&&e<1e-10f) { s=t=0.0f; *cpPQ=p; *cpAB=a; goto done; }
    if (c<1e-10f) { s=0.0f; t=vclamp(f/e,0.0f,1.0f); }
    else {
        float dv=dot_vector3(d1,d2), denom=c*e-dv*dv;
        s=(denom>1e-10f)?vclamp((dv*f-dot_vector3(d1,r)*e)/denom,0.0f,1.0f):0.0f;
        t=(dv*s+f)/e;
        if (t<0.0f)      { t=0.0f; s=vclamp(-dot_vector3(d1,r)/c,0.0f,1.0f); }
        else if (t>1.0f) { t=1.0f; s=vclamp((dv-dot_vector3(d1,r))/c,0.0f,1.0f); }
    }
    *cpPQ=Vector3_A_plus_B(p,scale_vector3(d1,s)); *cpAB=Vector3_A_plus_B(a,scale_vector3(d2,t));
done:;
    Vector3 w=Vector3_A_minus_B(*cpPQ,*cpAB);
    return dot_vector3(w,w);
}

static bool TestSphereSphere(ShapeSphere a,ShapeSphere b,Contact *c) {
    Vector3 d=Vector3_A_minus_B(a.center,b.center);
    float dist2=dot_vector3(d,d), rSum=a.radius+b.radius;
    if (dist2>=rSum*rSum) return false;
    float dist=vsqrtf(dist2);
    c->normal=(dist>1e-6f)?scale_vector3(d,1.0f/dist):(Vector3){0,1,0};
    c->depth=rSum-dist; c->pointWorld=Vector3_A_plus_B(b.center,scale_vector3(c->normal,b.radius)); c->lambdaN=c->lambdaT=0.0f;
    return true;
}
static bool TestSphereCapsule(ShapeSphere s,ShapeCapsule cap,Contact *c) { return TestSphereSphere(s,(ShapeSphere){ClosestPointOnSegment(cap.base,cap.tip,s.center),cap.radius},c); }
static bool TestCapsuleCapsule(ShapeCapsule a,ShapeCapsule b,Contact *c) {
    Vector3 cpA,cpB;
    float dist2=seg_seg_closest(a.base,a.tip,b.base,b.tip,&cpA,&cpB), rSum=a.radius+b.radius;
    if (dist2>=rSum*rSum) return false;
    float dist=vsqrtf(dist2);
    c->normal=(dist>1e-6f)?scale_vector3(Vector3_A_minus_B(cpA,cpB),1.0f/dist):(Vector3){0,1,0};
    c->depth=rSum-dist; c->pointWorld=Vector3_A_plus_B(cpB,scale_vector3(c->normal,b.radius)); c->lambdaN=c->lambdaT=0.0f;
    return true;
}
static bool TestSphereBox(ShapeSphere s,ShapeBox box,Contact *c) {
    Vector3 closest=closest_point_obb(box,s.center), d=Vector3_A_minus_B(s.center,closest);
    float dist2=dot_vector3(d,d);
    if (dist2>=s.radius*s.radius) return false;
    float dist=vsqrtf(dist2);
    c->normal=(dist>1e-6f)?scale_vector3(d,1.0f/dist):(Vector3){0,1,0};
    c->depth=s.radius-dist; c->pointWorld=closest; c->lambdaN=c->lambdaT=0.0f;
    return true;
}
static bool TestCapsuleBox(ShapeCapsule cap,ShapeBox box,Contact *c) {
    Contact best; best.depth=-1.0f;
    float ts[3]={0.0f,0.5f,1.0f};
    for (int i=0; i<3; ++i) {
        Contact ct; Vector3 sp=Vector3_A_plus_B(cap.base,scale_vector3(Vector3_A_minus_B(cap.tip,cap.base),ts[i]));
        if (TestSphereBox((ShapeSphere){sp,cap.radius},box,&ct)&&ct.depth>best.depth) best=ct;
    }
    if (best.depth<0.0f) return false;
    *c=best; return true;
}
static bool TestBoxBox(ShapeBox a,ShapeBox b,Contact *c) {
    Vector3 aax,aay,aaz,bax,bay,baz; obb_axes(a.rot,&aax,&aay,&aaz); obb_axes(b.rot,&bax,&bay,&baz);
    Vector3 axes[15]={aax,aay,aaz,bax,bay,baz,
        normalize_vector3(cross_vector3(aax,bax)),normalize_vector3(cross_vector3(aax,bay)),normalize_vector3(cross_vector3(aax,baz)),
        normalize_vector3(cross_vector3(aay,bax)),normalize_vector3(cross_vector3(aay,bay)),normalize_vector3(cross_vector3(aay,baz)),
        normalize_vector3(cross_vector3(aaz,bax)),normalize_vector3(cross_vector3(aaz,bay)),normalize_vector3(cross_vector3(aaz,baz))};
    Vector3 D=Vector3_A_minus_B(b.center,a.center);
    float minPen=1e30f; int minAxis=-1; float minSign=1.0f;
    for (int i=0; i<15; ++i) {
        float axLen=dot_vector3(axes[i],axes[i]);
        if (axLen<1e-8f) continue;
        Vector3 ax=(axLen<0.999f||axLen>1.001f)?normalize_vector3(axes[i]):axes[i];
        float overlap=obb_project(a,ax)+obb_project(b,ax)-vabs(dot_vector3(D,ax));
        if (overlap<=0.0f) return false;
        if (overlap<minPen) { minPen=overlap; minAxis=i; minSign=(dot_vector3(D,ax)>=0.0f)?1.0f:-1.0f; }
    }
    c->depth=minPen; c->normal=scale_vector3(axes[minAxis],minSign);
    c->pointWorld=closest_point_obb(b,Vector3_A_minus_B(a.center,scale_vector3(c->normal,minPen*0.5f))); c->lambdaN=c->lambdaT=0.0f;
    return true;
}

static inline void Entity_GetCapsule(const Entity *e,ShapeCapsule *out) {
    float r=e->colliderSize.x, hi=vmax(0.0f,(e->colliderSize.y*0.5f)-r);
    Vector3 wc=Vector3_A_plus_B(e->position,quat_rotate_vector(e->rotation,e->colliderCenter));
    Vector3 axis=(e->colliderSize.z<0.5f)?quat_rotate_vector(e->rotation,(Vector3){1,0,0}):(e->colliderSize.z<1.5f)?quat_rotate_vector(e->rotation,(Vector3){0,1,0}):quat_rotate_vector(e->rotation,(Vector3){0,0,1});
    out->radius=r; out->base=Vector3_A_minus_B(wc,scale_vector3(axis,hi)); out->tip=Vector3_A_plus_B(wc,scale_vector3(axis,hi));
}
static inline void Entity_GetBox(const Entity *e,ShapeBox *out) { out->center=Vector3_A_plus_B(e->position,quat_rotate_vector(e->rotation,e->colliderCenter)); out->halfExtents=scale_vector3(e->colliderSize,0.5f); out->rot=e->rotation; }
static inline void Entity_GetSphere(const Entity *e,ShapeSphere *out) { out->center=Vector3_A_plus_B(e->position,quat_rotate_vector(e->rotation,e->colliderCenter)); out->radius=e->colliderSize.x; }

static u32 g_meshContactCount;
static Contact g_meshContacts[MAX_MESH_CONTACTS];
static void TestSphereMeshInstance(ShapeSphere ws,u16 instanceIdx) {
    u16 mi=Sys_Global.instances[instanceIdx].modelIndex;
    if (mi>=loadedModelsMaxIndex) return;
    u32 triCount=modelTriangleCounts[mi];
    if (!triCount) return;
    float M[16]; __builtin_memcpy(M,&modelMatrices[instanceIdx*16],64);
    float m00=M[0],m10=M[1],m20=M[2],m01=M[4],m11=M[5],m21=M[6],m02=M[8],m12=M[9],m22=M[10],tx=M[12],ty=M[13],tz=M[14];
    float sx=vsqrtf(m00*m00+m10*m10+m20*m20), sy=vsqrtf(m01*m01+m11*m11+m21*m21), sz=vsqrtf(m02*m02+m12*m12+m22*m22);
    if (sx<1e-6f||sy<1e-6f||sz<1e-6f) return;
    float rx=ws.center.x-tx, ry=ws.center.y-ty, rz=ws.center.z-tz;
    Vector3 lC={(rx*m00+ry*m10+rz*m20)/(sx*sx),(rx*m01+ry*m11+rz*m21)/(sy*sy),(rx*m02+ry*m12+rz*m22)/(sz*sz)};
    float minScl=vmin(sx,vmin(sy,sz)), localR=ws.radius/minScl;
    for (u32 j=0; j<triCount&&g_meshContactCount<MAX_MESH_CONTACTS; ++j) {
        u32 bA=(u32)modelTriangles[mi][j*3+0]*VERTEX_ATTRIBUTES_SIZE, bB=(u32)modelTriangles[mi][j*3+1]*VERTEX_ATTRIBUTES_SIZE, bC=(u32)modelTriangles[mi][j*3+2]*VERTEX_ATTRIBUTES_SIZE;
        Vector3 A={half_to_float(*(half*)(modelVertices[mi]+bA+0)),half_to_float(*(half*)(modelVertices[mi]+bA+2)),half_to_float(*(half*)(modelVertices[mi]+bA+4))};
        Vector3 B={half_to_float(*(half*)(modelVertices[mi]+bB+0)),half_to_float(*(half*)(modelVertices[mi]+bB+2)),half_to_float(*(half*)(modelVertices[mi]+bB+4))};
        Vector3 C={half_to_float(*(half*)(modelVertices[mi]+bC+0)),half_to_float(*(half*)(modelVertices[mi]+bC+2)),half_to_float(*(half*)(modelVertices[mi]+bC+4))};
        Vector3 closest=ClosestPointOnTriangle(A,B,C,lC), d=Vector3_A_minus_B(lC,closest);
        float dist2=dot_vector3(d,d);
        if (dist2>=localR*localR) continue;
        float dist=vsqrtf(dist2);
        Vector3 lN;
        if (dist>1e-6f) { lN=scale_vector3(d,1.0f/dist); }
        else {
            lN=normalize_vector3(cross_vector3(Vector3_A_minus_B(B,A),Vector3_A_minus_B(C,A)));
            Vector3 toC=Vector3_A_minus_B(lC,A);
            if (dot_vector3(lN,toC)<0.0f) { lN.x=-lN.x; lN.y=-lN.y; lN.z=-lN.z; }
        }
        Contact ct;
        ct.normal=normalize_vector3((Vector3){(m00/sx)*lN.x+(m01/sy)*lN.y+(m02/sz)*lN.z,(m10/sx)*lN.x+(m11/sy)*lN.y+(m12/sz)*lN.z,(m20/sx)*lN.x+(m21/sy)*lN.y+(m22/sz)*lN.z});
        ct.pointWorld=(Vector3){m00*closest.x+m01*closest.y+m02*closest.z+tx,m10*closest.x+m11*closest.y+m12*closest.z+ty,m20*closest.x+m21*closest.y+m22*closest.z+tz};
        ct.depth=(localR-dist)*minScl; ct.lambdaN=ct.lambdaT=0.0f;
        g_meshContacts[g_meshContactCount++]=ct;
    }
}

static void TestCapsuleMeshInstance(ShapeCapsule cap,u16 instanceIdx) {
    for (int i=0; i<=4&&g_meshContactCount<MAX_MESH_CONTACTS; ++i)
        TestSphereMeshInstance((ShapeSphere){Vector3_A_plus_B(cap.base,scale_vector3(Vector3_A_minus_B(cap.tip,cap.base),(float)i*0.25f)),cap.radius},instanceIdx);
}

static ContactManifold* GenerateManifold(u16 idxA,u16 idxB) {
    Entity *eA=&Sys_Global.instances[idxA], *eB=&Sys_Global.instances[idxB];
    ColliderType ctA=eA->collider, ctB=eB->collider;
    ContactManifold *m=FindOrCreateManifold(idxA,idxB);
    if (!m) return NULL;
    m->count=0;
    if (ctB==COLLIDER_TYPE_MESH||ctB==COLLIDER_TYPE_CONVEXMESH) {
        g_meshContactCount=0;
        if      (ctA==COLLIDER_TYPE_SPHERE)  { ShapeSphere s; Entity_GetSphere(eA,&s); TestSphereMeshInstance(s,idxB); }
        else if (ctA==COLLIDER_TYPE_CAPSULE) { ShapeCapsule c; Entity_GetCapsule(eA,&c); TestCapsuleMeshInstance(c,idxB); }
        else if (ctA==COLLIDER_TYPE_BOX) {
            ShapeBox b; Entity_GetBox(eA,&b); Vector3 ax,ay,az; obb_axes(b.rot,&ax,&ay,&az);
            for (int cx=-1; cx<=1; cx+=2) for (int cy=-1; cy<=1; cy+=2) for (int cz=-1; cz<=1; cz+=2)
                TestSphereMeshInstance((ShapeSphere){Vector3_A_plus_B(b.center,Vector3_A_plus_B(Vector3_A_plus_B(scale_vector3(ax,b.halfExtents.x*(float)cx),scale_vector3(ay,b.halfExtents.y*(float)cy)),scale_vector3(az,b.halfExtents.z*(float)cz))),0.004f},idxB);
        }
        for (u32 i=0; i<g_meshContactCount&&m->count<MAX_CONTACTS_PER_PAIR; ++i) {
            float prevLN=0.0f, prevLT=0.0f;
            for (int k=0; k<(int)m->count; ++k)
                if (dist_sq_vector3(m->contacts[k].pointWorld,g_meshContacts[i].pointWorld)<0.01f) { prevLN=m->contacts[k].lambdaN; prevLT=m->contacts[k].lambdaT; break; }
            g_meshContacts[i].lambdaN=prevLN; g_meshContacts[i].lambdaT=prevLT;
            m->contacts[m->count++]=g_meshContacts[i];
        }
        return m->count?m:NULL;
    }
    Contact ct; bool hit=false;
    if      (ctA==COLLIDER_TYPE_SPHERE &&ctB==COLLIDER_TYPE_SPHERE)  { ShapeSphere a,b; Entity_GetSphere(eA,&a); Entity_GetSphere(eB,&b); hit=TestSphereSphere(a,b,&ct); }
    else if (ctA==COLLIDER_TYPE_SPHERE &&ctB==COLLIDER_TYPE_CAPSULE) { ShapeSphere a; ShapeCapsule b; Entity_GetSphere(eA,&a); Entity_GetCapsule(eB,&b); hit=TestSphereCapsule(a,b,&ct); }
    else if (ctA==COLLIDER_TYPE_CAPSULE&&ctB==COLLIDER_TYPE_SPHERE)  { ShapeCapsule a; ShapeSphere b; Entity_GetCapsule(eA,&a); Entity_GetSphere(eB,&b); hit=TestSphereCapsule(b,a,&ct); if(hit) ct.normal=scale_vector3(ct.normal,-1.0f); }
    else if (ctA==COLLIDER_TYPE_CAPSULE&&ctB==COLLIDER_TYPE_CAPSULE) { ShapeCapsule a,b; Entity_GetCapsule(eA,&a); Entity_GetCapsule(eB,&b); hit=TestCapsuleCapsule(a,b,&ct); }
    else if (ctA==COLLIDER_TYPE_SPHERE &&ctB==COLLIDER_TYPE_BOX)     { ShapeSphere a; ShapeBox b; Entity_GetSphere(eA,&a); Entity_GetBox(eB,&b); hit=TestSphereBox(a,b,&ct); }
    else if (ctA==COLLIDER_TYPE_BOX    &&ctB==COLLIDER_TYPE_SPHERE)  { ShapeBox a; ShapeSphere b; Entity_GetBox(eA,&a); Entity_GetSphere(eB,&b); hit=TestSphereBox(b,a,&ct); if(hit) ct.normal=scale_vector3(ct.normal,-1.0f); }
    else if (ctA==COLLIDER_TYPE_CAPSULE&&ctB==COLLIDER_TYPE_BOX)     { ShapeCapsule a; ShapeBox b; Entity_GetCapsule(eA,&a); Entity_GetBox(eB,&b); hit=TestCapsuleBox(a,b,&ct); }
    else if (ctA==COLLIDER_TYPE_BOX    &&ctB==COLLIDER_TYPE_CAPSULE) { ShapeBox a; ShapeCapsule b; Entity_GetBox(eA,&a); Entity_GetCapsule(eB,&b); hit=TestCapsuleBox(b,a,&ct); if(hit) ct.normal=scale_vector3(ct.normal,-1.0f); }
    else if (ctA==COLLIDER_TYPE_BOX    &&ctB==COLLIDER_TYPE_BOX)     { ShapeBox a,b; Entity_GetBox(eA,&a); Entity_GetBox(eB,&b); hit=TestBoxBox(a,b,&ct); }
    if (!hit) return NULL;
    if (m->count) { ct.lambdaN=m->contacts[0].lambdaN; ct.lambdaT=m->contacts[0].lambdaT; }
    m->contacts[0]=ct; m->count=1;
    return m;
}
 
static inline float entity_invmass(const Entity *e) { if (e->entflags&ENTFLAG_KINEMATIC) return 0.0f; return (e->mass>0.001f)?1.0f/e->mass:0.0f; }
static inline void ApplyAngularImpulse(Entity *e,Vector3 r,Vector3 impulse,float sign) {
    if (e->inertia<0.0001f) return;
    Vector3 ti=cross_vector3(r,impulse); float invI=1.0f/e->inertia;
    e->angularVelocity.x+=sign*ti.x*invI; e->angularVelocity.y+=sign*ti.y*invI; e->angularVelocity.z+=sign*ti.z*invI;
}

static void SolveContact(ContactManifold *m,float dt) {
    Entity *eA=&Sys_Global.instances[m->idxA], *eB=&Sys_Global.instances[m->idxB];
    float imA=entity_invmass(eA), imB=entity_invmass(eB);
    if (imA+imB<1e-10f) return;
    float rA=eA->bounciness, rB=eB->bounciness;
    float restitution=(eA->bounceCombine==PHYS_COMBINE_MAX||eB->bounceCombine==PHYS_COMBINE_MAX)?vmax(rA,rB):(eA->bounceCombine==PHYS_COMBINE_MIN||eB->bounceCombine==PHYS_COMBINE_MIN)?vmin(rA,rB):(eA->bounceCombine==PHYS_COMBINE_MUL||eB->bounceCombine==PHYS_COMBINE_MUL)?rA*rB:(rA+rB)*0.5f;
    float fA=(eA->dynamicFriction+eA->staticFriction)*0.5f, fB=(eB->dynamicFriction+eB->staticFriction)*0.5f;
    float friction=(eA->frictionCombine==PHYS_COMBINE_MAX||eB->frictionCombine==PHYS_COMBINE_MAX)?vmax(fA,fB):(eA->frictionCombine==PHYS_COMBINE_MIN||eB->frictionCombine==PHYS_COMBINE_MIN)?vmin(fA,fB):(eA->frictionCombine==PHYS_COMBINE_MUL||eB->frictionCombine==PHYS_COMBINE_MUL)?fA*fB:(fA+fB)*0.5f;
    float invIA=(eA->inertia>0.0001f)?1.0f/eA->inertia:0.0f, invIB=(eB->inertia>0.0001f)?1.0f/eB->inertia:0.0f;
    for (int ci=0; ci<(int)m->count; ++ci) {
        Contact *c=&m->contacts[ci];
        Vector3 rA=Vector3_A_minus_B(c->pointWorld,Vector3_A_plus_B(eA->position,quat_rotate_vector(eA->rotation,eA->colliderCenter)));
        Vector3 rB=Vector3_A_minus_B(c->pointWorld,Vector3_A_plus_B(eB->position,quat_rotate_vector(eB->rotation,eB->colliderCenter)));
        Vector3 vAtA=Vector3_A_plus_B(eA->velocity,cross_vector3(eA->angularVelocity,rA));
        Vector3 vAtB=Vector3_A_plus_B(eB->velocity,cross_vector3(eB->angularVelocity,rB));
        Vector3 relVel=Vector3_A_minus_B(vAtA,vAtB);
        float vn=dot_vector3(relVel,c->normal);
        if (vn>SPECULATIVE_MARGIN/dt) continue;
        Vector3 rAxN=cross_vector3(rA,c->normal), rBxN=cross_vector3(rB,c->normal);
        float jnDenom=imA+imB+dot_vector3(rAxN,rAxN)*invIA+dot_vector3(rBxN,rBxN)*invIB;
        float jn=-(1.0f+(vabs(vn)>RESTITUTION_THRESHOLD?restitution:0.0f))*vn/jnDenom;
        float pen=c->depth-BAUMGARTE_SLOP;
        if (pen>0.0f) jn+=(BAUMGARTE_FACTOR*pen/dt)/jnDenom;
        float newLN=vmax(0.0f,c->lambdaN+jn), dLN=newLN-c->lambdaN;
        c->lambdaN=newLN;
        Vector3 impulseN=scale_vector3(c->normal,dLN);
        eA->velocity=Vector3_A_plus_B(eA->velocity,scale_vector3(impulseN,imA));
        eB->velocity=Vector3_A_minus_B(eB->velocity,scale_vector3(impulseN,imB));
        ApplyAngularImpulse(eA,rA,impulseN,1.0f); ApplyAngularImpulse(eB,rB,impulseN,-1.0f);
        vAtA=Vector3_A_plus_B(eA->velocity,cross_vector3(eA->angularVelocity,rA));
        vAtB=Vector3_A_plus_B(eB->velocity,cross_vector3(eB->angularVelocity,rB));
        relVel=Vector3_A_minus_B(vAtA,vAtB);
        Vector3 tangent=Vector3_A_minus_B(relVel,scale_vector3(c->normal,dot_vector3(relVel,c->normal)));
        float tLen=magnitude_vector3(tangent);
        if (tLen>1e-6f) {
            tangent=scale_vector3(tangent,1.0f/tLen);
            Vector3 rAxT=cross_vector3(rA,tangent), rBxT=cross_vector3(rB,tangent);
            float jtDenom=imA+imB+dot_vector3(rAxT,rAxT)*invIA+dot_vector3(rBxT,rBxT)*invIB;
            float newLT=vclamp(c->lambdaT+(-dot_vector3(relVel,tangent)/jtDenom),-friction*newLN,friction*newLN), dLT=newLT-c->lambdaT;
            c->lambdaT=newLT;
            Vector3 impulseT=scale_vector3(tangent,dLT);
            eA->velocity=Vector3_A_plus_B(eA->velocity,scale_vector3(impulseT,imA));
            eB->velocity=Vector3_A_minus_B(eB->velocity,scale_vector3(impulseT,imB));
            ApplyAngularImpulse(eA,rA,impulseT,1.0f); ApplyAngularImpulse(eB,rB,impulseT,-1.0f);
        }
    }
}

static void SpeculativePreClamp(u16 idxA,float dt) {
    Entity *eA=&Sys_Global.instances[idxA];
    if (entity_invmass(eA)<1e-10f) return;
    for (u16 m=0; m<g_manifoldCount; ++m) {
        ContactManifold *mf=&g_manifolds[m];
        if (mf->idxA!=idxA&&mf->idxB!=idxA) continue;
        for (int ci=0; ci<(int)mf->count; ++ci) {
            Contact *c=&mf->contacts[ci];
            Vector3 n=(mf->idxA==idxA)?c->normal:scale_vector3(c->normal,-1.0f);
            float vn=dot_vector3(eA->velocity,n), maxCS=-c->depth/dt-SPECULATIVE_MARGIN/dt;
            if (vn<maxCS) eA->velocity=Vector3_A_minus_B(eA->velocity,scale_vector3(n,vn-maxCS));
        }
    }
}

static u8 g_sleepCounter[INSTANCE_COUNT];
static void UpdateSleep(u16 i,float dt) {
    (void)dt; Entity *e=&Sys_Global.instances[i];
    if (entity_invmass(e)<1e-10f) return;
    if (dot_vector3(e->velocity,e->velocity)<SLEEP_KE_THRESHOLD) {
        if (++g_sleepCounter[i]>=SLEEP_FRAMES_NEEDED) { e->velocity=(Vector3){0,0,0}; flag_set(&e->entflags,ENTFLAG_ASLEEP,true); }
    } else { g_sleepCounter[i]=0; flag_set(&e->entflags,ENTFLAG_ASLEEP,false); }
}

static inline Quaternion IntegrateRotation(Quaternion q,Vector3 omega,float dt) {
    Quaternion dq={omega.x*dt*0.5f,omega.y*dt*0.5f,omega.z*dt*0.5f,0.0f};
    dq=quat_multiply(q,dq); q.x+=dq.x; q.y+=dq.y; q.z+=dq.z; q.w+=dq.w;
    float invLen=1.0f/vsqrtf(q.x*q.x+q.y*q.y+q.z*q.z+q.w*q.w);
    return (Quaternion){q.x*invLen,q.y*invLen,q.z*invLen,q.w*invLen};
}
static void IntegrateAngularVelocity(u16 i,float dt) {
    Entity *e=&Sys_Global.instances[i];
    if (!(e->entflags&ENTFLAG_RIGIDBODY)||e->entflags&ENTFLAG_ASLEEP||entity_invmass(e)<1e-10f) return;
    float invI=(e->inertia>0.0001f)?1.0f/e->inertia:0.0f;
    e->angularVelocity=Vector3_A_plus_B(e->angularVelocity,scale_vector3(e->accumulatedTorque,invI*dt)); e->accumulatedTorque=(Vector3){0,0,0};
    e->angularVelocity=scale_vector3(e->angularVelocity,1.0f-vclamp(e->angularDrag*dt,0.0f,1.0f));
    float omegaLen=magnitude_vector3(e->angularVelocity);
    if (omegaLen>12.566f) e->angularVelocity=scale_vector3(e->angularVelocity,12.566f/omegaLen);
    e->rotation=IntegrateRotation(e->rotation,e->angularVelocity,dt); Sys_Global.dirtyInstances[i]=true;
}

void Physics_PrimitiveStep(float dt) {
    if (dt>SUB_STEP_DT_MAX*4.0f) dt=SUB_STEP_DT_MAX*4.0f;
    if (dt>SUB_STEP_DT_MAX) { float h=dt*0.5f; Physics_PrimitiveStep(h); Physics_PrimitiveStep(h); return; }
    u16 n=Sys_Global.loadedInstances;
    ResetManifoldTable(); BuildCellBuckets(n);
    for (u16 i=START_INDEX_LEVEL_INSTANCES; i<n; ++i) {
        Entity *eA=&Sys_Global.instances[i];
        if (!(eA->entflags&ENTFLAG_ACTIVE)||!(eA->entflags&ENTFLAG_RIGIDBODY)||eA->entflags&ENTFLAG_ASLEEP||eA->collider==COLLIDER_TYPE_NONE) continue;
        u32 maskA=GetCollisionMask(eA->layer), cellA=eA->cellIndex;
        i32 acx=(i32)(cellA%WORLDX), acz=(i32)(cellA/WORLDX);
        for (i32 dz=-1; dz<=1; ++dz) {
            i32 ncz=acz+dz;
            if (ncz<0||ncz>=(i32)WORLDZ) continue;
            for (i32 dx=-1; dx<=1; ++dx) {
                i32 ncx=acx+dx;
                if (ncx<0||ncx>=(i32)WORLDX) continue;
                u32 nc=(u32)ncz*WORLDX+(u32)ncx;
                if (!(gridCellStates[nc]&CELL_OPEN)) continue;
                CellBucket *bkt=&g_cellBuckets[nc];
                for (u8 bi=0; bi<bkt->count; ++bi) {
                    u16 j=bkt->idx[bi];
                    if (j==i) continue;
                    Entity *eB=&Sys_Global.instances[j];
                    if (!(eB->entflags&ENTFLAG_ACTIVE)||eB->collider==COLLIDER_TYPE_NONE||!(maskA&eB->layer)) continue;
                    if ((eB->entflags&ENTFLAG_RIGIDBODY)&&j<i) continue;
                    float dx2=eA->position.x-eB->position.x, dy=eA->position.y-eB->position.y, dz2=eA->position.z-eB->position.z;
                    if (dx2<-5.12f||dx2>5.12f||dy<-5.12f||dy>5.12f||dz2<-5.12f||dz2>5.12f) continue;
                    GenerateManifold(i,j);
                }
            }
        }
    }
    for (u16 i=START_INDEX_LEVEL_INSTANCES; i<n; ++i) {if (Sys_Global.instances[i].entflags&ENTFLAG_RIGIDBODY) SpeculativePreClamp(i,dt);}
    for (int iter=0; iter<SOLVER_ITERATIONS; ++iter) {for (u16 m=0; m<g_manifoldCount; ++m) SolveContact(&g_manifolds[m],dt);}
    for (u16 i=START_INDEX_LEVEL_INSTANCES; i<n; ++i) {if (Sys_Global.instances[i].entflags&ENTFLAG_RIGIDBODY) IntegrateAngularVelocity(i,dt);}
    for (u16 i=START_INDEX_LEVEL_INSTANCES; i<n; ++i) {if (Sys_Global.instances[i].entflags&ENTFLAG_RIGIDBODY) UpdateSleep(i,dt);}
}
 
void Physics_ResetForLevelLoad(void) {
    __builtin_memset(g_manifolds,0,sizeof(g_manifolds)); __builtin_memset(g_manifoldHT,0,sizeof(g_manifoldHT)); __builtin_memset(g_sleepCounter,0,sizeof(g_sleepCounter));
    g_manifoldCount=0;
}

static inline float DefaultInertia(const Entity *e) {
    float m=(e->mass>0.001f)?e->mass:1.0f;
    if (e->collider==COLLIDER_TYPE_SPHERE) return 0.4f*m*e->colliderSize.x*e->colliderSize.x;
    float hx=e->colliderSize.x*0.5f, hy=e->colliderSize.y*0.5f, hz=e->colliderSize.z*0.5f;
    return m*(hx*hx+hy*hy+hz*hz)*(1.0f/3.0f);
}
void Physics_InitEntityInertia(u16 idx) { Entity *e=&Sys_Global.instances[idx]; if (e->inertia<0.0001f) e->inertia=DefaultInertia(e); }

void SetDebugLineColor(float r,float g,float b);
#define DBG_IDLE  0.0f,0.4f,0.0f
#define DBG_STAY  0.0f,1.0f,0.0f
#define DBG_ENTER 1.0f,0.1f,0.1f
#define DBG_NORM  0.1f,0.3f,1.0f
#define DEBUG_NORMAL_LEN 0.16f

static void DebugDrawBox(ShapeBox b) {
    Vector3 ax,ay,az; obb_axes(b.rot,&ax,&ay,&az); Vector3 c[8];
    for (int i=0; i<8; ++i) { float sx=(i&1)?1.0f:-1.0f,sy=(i&2)?1.0f:-1.0f,sz=(i&4)?1.0f:-1.0f; c[i]=Vector3_A_plus_B(b.center,Vector3_A_plus_B(Vector3_A_plus_B(scale_vector3(ax,b.halfExtents.x*sx),scale_vector3(ay,b.halfExtents.y*sy)),scale_vector3(az,b.halfExtents.z*sz))); }
    int edges[12][2]={{0,1},{2,3},{4,5},{6,7},{0,2},{1,3},{4,6},{5,7},{0,4},{1,5},{2,6},{3,7}};
    for (int e=0; e<12; ++e) AddDebugLine(c[edges[e][0]],c[edges[e][1]]);
}
static void DebugDrawSphere(ShapeSphere s) {
    float r=s.radius, d=r*0.57735026f; Vector3 o=s.center;
    AddDebugLine(o,(Vector3){o.x+r,o.y,o.z}); AddDebugLine(o,(Vector3){o.x-r,o.y,o.z});
    AddDebugLine(o,(Vector3){o.x,o.y+r,o.z}); AddDebugLine(o,(Vector3){o.x,o.y-r,o.z});
    AddDebugLine(o,(Vector3){o.x,o.y,o.z+r}); AddDebugLine(o,(Vector3){o.x,o.y,o.z-r});
    for (int sx=-1; sx<=1; sx+=2) for (int sy=-1; sy<=1; sy+=2) for (int sz=-1; sz<=1; sz+=2) AddDebugLine(o,(Vector3){o.x+sx*d,o.y+sy*d,o.z+sz*d});
}
static void DebugDrawCapsule(ShapeCapsule cap) { AddDebugLine(cap.base,cap.tip); DebugDrawSphere((ShapeSphere){cap.base,cap.radius}); DebugDrawSphere((ShapeSphere){cap.tip,cap.radius}); }

static void DebugDrawCollider(u16 idx,float r,float g,float b) {
    Entity *e=&Sys_Global.instances[idx]; SetDebugLineColor(r,g,b);
    switch (e->collider) {
        case COLLIDER_TYPE_BOX:     { ShapeBox box; Entity_GetBox(e,&box); DebugDrawBox(box); break; }
        case COLLIDER_TYPE_SPHERE:  { ShapeSphere sph; Entity_GetSphere(e,&sph); DebugDrawSphere(sph); break; }
        case COLLIDER_TYPE_CAPSULE: { ShapeCapsule cap; Entity_GetCapsule(e,&cap); DebugDrawCapsule(cap); break; }
        case COLLIDER_TYPE_MESH: case COLLIDER_TYPE_CONVEXMESH: {
            u16 mi=e->modelIndex; if (mi>=loadedModelsMaxIndex) break;
            u32 tc=modelTriangleCounts[mi]; float M[16]; __builtin_memcpy(M,&modelMatrices[idx*16],64);
            float m00=M[0],m10=M[1],m20=M[2],m01=M[4],m11=M[5],m21=M[6],m02=M[8],m12=M[9],m22=M[10],tx=M[12],ty=M[13],tz=M[14];
            u32 step=(tc>256)?(tc/256):1;
            #define LTW(l) (Vector3){m00*(l).x+m01*(l).y+m02*(l).z+tx,m10*(l).x+m11*(l).y+m12*(l).z+ty,m20*(l).x+m21*(l).y+m22*(l).z+tz}
            for (u32 j=0; j<tc; j+=step) {
                u32 bA=(u32)modelTriangles[mi][j*3+0]*VERTEX_ATTRIBUTES_SIZE, bB=(u32)modelTriangles[mi][j*3+1]*VERTEX_ATTRIBUTES_SIZE, bC=(u32)modelTriangles[mi][j*3+2]*VERTEX_ATTRIBUTES_SIZE;
                Vector3 lA={half_to_float(*(half*)(modelVertices[mi]+bA+0)),half_to_float(*(half*)(modelVertices[mi]+bA+2)),half_to_float(*(half*)(modelVertices[mi]+bA+4))};
                Vector3 lB={half_to_float(*(half*)(modelVertices[mi]+bB+0)),half_to_float(*(half*)(modelVertices[mi]+bB+2)),half_to_float(*(half*)(modelVertices[mi]+bB+4))};
                Vector3 lC={half_to_float(*(half*)(modelVertices[mi]+bC+0)),half_to_float(*(half*)(modelVertices[mi]+bC+2)),half_to_float(*(half*)(modelVertices[mi]+bC+4))};
                Vector3 wA=LTW(lA),wB=LTW(lB),wC=LTW(lC); AddDebugLine(wA,wB); AddDebugLine(wB,wC); AddDebugLine(wC,wA);
            }
            #undef LTW
            break;
        }
        default: break;
    }
}

#define MAX_DEBUG_MANIFOLD_IDS MAX_MANIFOLDS
static u32 g_prevManifoldIDs[MAX_DEBUG_MANIFOLD_IDS];
static u16 g_prevManifoldCount=0;
static inline u32 ManifoldID(u16 a,u16 b) { return (a<b)?((u32)a<<16)|b:((u32)b<<16)|a; }
static inline bool IDInPrevSet(u32 id) { for (u16 i=0; i<g_prevManifoldCount; ++i) if (g_prevManifoldIDs[i]==id) return true; return false; }

void Physics_DrawDebug(void) {
    if (Sys_Global.physicsDebug<=0) return;
    u16 n=Sys_Global.loadedInstances;
    u32 curIDs[MAX_DEBUG_MANIFOLD_IDS]; u16 curCount=0;
    for (u16 m=0; m<g_manifoldCount&&curCount<MAX_DEBUG_MANIFOLD_IDS; ++m) curIDs[curCount++]=ManifoldID(g_manifolds[m].idxA,g_manifolds[m].idxB);
    static u8 g_contactState[INSTANCE_COUNT];
    __builtin_memset(g_contactState,0,n);
    for (u16 m=0; m<g_manifoldCount; ++m) {
        u16 a=g_manifolds[m].idxA, b=g_manifolds[m].idxB;
        u8 state=IDInPrevSet(ManifoldID(a,b))?1:2;
        if (Sys_Global.instances[a].entflags&ENTFLAG_RIGIDBODY&&state>g_contactState[a]) g_contactState[a]=state;
        if (Sys_Global.instances[b].entflags&ENTFLAG_RIGIDBODY&&state>g_contactState[b]) g_contactState[b]=state;
    }
    for (u16 i=START_INDEX_LEVEL_INSTANCES; i<n; ++i) {
        Entity *e=&Sys_Global.instances[i];
        if (!(e->entflags&ENTFLAG_ACTIVE)||e->collider==COLLIDER_TYPE_NONE) continue;
        u8 cs=g_contactState[i];
        if      (cs==2) DebugDrawCollider(i,DBG_ENTER);
        else if (cs==1) DebugDrawCollider(i,DBG_STAY);
        else            DebugDrawCollider(i,DBG_IDLE);
    }
    for (u16 m=0; m<g_manifoldCount; ++m) {
        SetDebugLineColor(DBG_NORM);
        for (int ci=0; ci<(int)g_manifolds[m].count; ++ci) { const Contact *c=&g_manifolds[m].contacts[ci]; AddDebugLine(c->pointWorld,Vector3_A_plus_B(c->pointWorld,scale_vector3(c->normal,DEBUG_NORMAL_LEN))); }
    }
    __builtin_memcpy(g_prevManifoldIDs,curIDs,curCount*sizeof(u32)); g_prevManifoldCount=curCount;
}

#define NO_CONTACT ((CapsuleContact){.depth=-1.0f,.normal={0,1,0}})
static CapsuleContact QueryCapsuleContact(Vector3 start,Vector3 end,float capsuleRadius,u32 layerMask) {
    CapsuleContact worst=NO_CONTACT;
    i32 cxMin=vmax(0,vmin(WORLDX-1,PosGetCellCoordX(vmin(start.x,end.x)-capsuleRadius)));
    i32 cxMax=vmax(0,vmin(WORLDX-1,PosGetCellCoordX(vmax(start.x,end.x)+capsuleRadius)));
    i32 czMin=vmax(0,vmin(WORLDZ-1,PosGetCellCoordZ(vmin(start.z,end.z)-capsuleRadius)));
    i32 czMax=vmax(0,vmin(WORLDZ-1,PosGetCellCoordZ(vmax(start.z,end.z)+capsuleRadius)));
    for (u16 i=START_INDEX_LEVEL_INSTANCES; i<Sys_Global.loadedInstances; ++i) {
        if (!(layerMask&Sys_Global.instances[i].layer)) continue;
        u32 instCell=Sys_Global.instances[i].cellIndex;
        i32 icx=(i32)(instCell%WORLDX), icz=(i32)(instCell/WORLDX);
        if (icx<cxMin||icx>cxMax||icz<czMin||icz>czMax) continue;
        if (!(gridCellStates[instCell]&CELL_OPEN)) continue;
        u16 mindex=Sys_Global.instances[i].modelIndex;
        if (mindex>=loadedModelsMaxIndex) continue;
        u32 triCount=modelTriangleCounts[mindex];
        if (triCount<1) continue;
        float M[16]; __builtin_memcpy(M,&modelMatrices[i*16],64);
        float m00=M[0],m10=M[1],m20=M[2],m01=M[4],m11=M[5],m21=M[6],m02=M[8],m12=M[9],m22=M[10],tx=M[12],ty=M[13],tz=M[14];
        float scl_x=vsqrtf(m00*m00+m10*m10+m20*m20), scl_y=vsqrtf(m01*m01+m11*m11+m21*m21), scl_z=vsqrtf(m02*m02+m12*m12+m22*m22);
        if (scl_x<1e-6f||scl_y<1e-6f||scl_z<1e-6f) continue;
        Vector3 objPos=Sys_Global.instances[i].position, capsuleMid={(start.x+end.x)*0.5f,(start.y+end.y)*0.5f,(start.z+end.z)*0.5f};
        Vector3 delta=Vector3_A_minus_B(objPos,capsuleMid);
        float combinedRad=vmax(modelBounds[mindex],1.81f)+magnitude_vector3(Vector3_A_minus_B(end,start))*0.5f+capsuleRadius+0.1f;
        if (delta.x*delta.x+delta.y*delta.y+delta.z*delta.z>combinedRad*combinedRad) continue;
        float sx2=scl_x*scl_x, sy2=scl_y*scl_y, sz2=scl_z*scl_z;
        Vector3 relS={start.x-tx,start.y-ty,start.z-tz};
        Vector3 lS={(relS.x*m00+relS.y*m10+relS.z*m20)/sx2,(relS.x*m01+relS.y*m11+relS.z*m21)/sy2,(relS.x*m02+relS.y*m12+relS.z*m22)/sz2};
        Vector3 relE={end.x-tx,end.y-ty,end.z-tz};
        Vector3 lE={(relE.x*m00+relE.y*m10+relE.z*m20)/sx2,(relE.x*m01+relE.y*m11+relE.z*m21)/sy2,(relE.x*m02+relE.y*m12+relE.z*m22)/sz2};
        float minScl=vmin(scl_x,vmin(scl_y,scl_z)), localRadius=capsuleRadius/minScl;
        for (u32 j=0; j<triCount; ++j) {
            u32 bA=(u32)modelTriangles[mindex][j*3+0]*VERTEX_ATTRIBUTES_SIZE, bB=(u32)modelTriangles[mindex][j*3+1]*VERTEX_ATTRIBUTES_SIZE, bC=(u32)modelTriangles[mindex][j*3+2]*VERTEX_ATTRIBUTES_SIZE;
            Vector3 pA={half_to_float(*(half*)(modelVertices[mindex]+bA+0)),half_to_float(*(half*)(modelVertices[mindex]+bA+2)),half_to_float(*(half*)(modelVertices[mindex]+bA+4))};
            Vector3 pB={half_to_float(*(half*)(modelVertices[mindex]+bB+0)),half_to_float(*(half*)(modelVertices[mindex]+bB+2)),half_to_float(*(half*)(modelVertices[mindex]+bB+4))};
            Vector3 pC={half_to_float(*(half*)(modelVertices[mindex]+bC+0)),half_to_float(*(half*)(modelVertices[mindex]+bC+2)),half_to_float(*(half*)(modelVertices[mindex]+bC+4))};
            Vector3 cpP=ClosestPointOnTriangle(pA,pB,pC,lS), spP=ClosestPointOnSegment(lS,lE,cpP), cpP2=ClosestPointOnTriangle(pA,pB,pC,spP);
            Vector3 cpQ=ClosestPointOnTriangle(pA,pB,pC,lE), spQ=ClosestPointOnSegment(lS,lE,cpQ), cpQ2=ClosestPointOnTriangle(pA,pB,pC,spQ);
            Vector3 dP=Vector3_A_minus_B(spP,cpP2), dQ=Vector3_A_minus_B(spQ,cpQ2);
            float distP=vsqrtf(dot_vector3(dP,dP)), distQ=vsqrtf(dot_vector3(dQ,dQ));
            float localDist=(distP<=distQ)?distP:distQ; Vector3 lcv=(distP<=distQ)?dP:dQ;
            float localPen=localRadius-localDist;
            if (localPen<=0.0f) continue;
            Vector3 lN;
            if (localDist>1e-6f) { lN=(Vector3){lcv.x/localDist,lcv.y/localDist,lcv.z/localDist}; }
            else {
                lN=normalize_vector3(cross_vector3(Vector3_A_minus_B(pB,pA),Vector3_A_minus_B(pC,pA)));
                Vector3 spMid={(lS.x+lE.x)*0.5f,(lS.y+lE.y)*0.5f,(lS.z+lE.z)*0.5f};
                if (dot_vector3(lN,Vector3_A_minus_B(spMid,pA))<0.0f) { lN.x=-lN.x; lN.y=-lN.y; lN.z=-lN.z; }
            }
            Vector3 wN=normalize_vector3((Vector3){(m00/scl_x)*lN.x+(m01/scl_y)*lN.y+(m02/scl_z)*lN.z,(m10/scl_x)*lN.x+(m11/scl_y)*lN.y+(m12/scl_z)*lN.z,(m20/scl_x)*lN.x+(m21/scl_y)*lN.y+(m22/scl_z)*lN.z});
            float wp=localPen*minScl;
            if (wp>worst.depth) { worst.depth=wp; worst.normal=wN; }
        }
    }
    return worst;
}

static CapsuleContact QueryRigidbodyWorldContact(u16 i,Vector3 pos) {
    Entity *e=&Sys_Global.instances[i]; u32 mask=GetCollisionMask(e->layer);
    switch (e->collider) {
        case COLLIDER_TYPE_SPHERE: { Vector3 c=Vector3_A_plus_B(pos,quat_rotate_vector(e->rotation,e->colliderCenter)); return QueryCapsuleContact(c,c,e->colliderSize.x,mask); }
        case COLLIDER_TYPE_CAPSULE: {
            float r=e->colliderSize.x, hi=vmax(0.0f,(e->colliderSize.y*0.5f)-r);
            Vector3 center=Vector3_A_plus_B(pos,quat_rotate_vector(e->rotation,e->colliderCenter));
            Vector3 axis=(e->colliderSize.z<0.5f)?quat_rotate_vector(e->rotation,(Vector3){1,0,0}):(e->colliderSize.z<1.5f)?quat_rotate_vector(e->rotation,(Vector3){0,1,0}):quat_rotate_vector(e->rotation,(Vector3){0,0,1});
            return QueryCapsuleContact(Vector3_A_minus_B(center,scale_vector3(axis,hi)),Vector3_A_plus_B(center,scale_vector3(axis,hi)),r,mask);
        }
        case COLLIDER_TYPE_BOX: {
            ShapeBox b; b.center=Vector3_A_plus_B(pos,quat_rotate_vector(e->rotation,e->colliderCenter)); b.halfExtents=scale_vector3(e->colliderSize,0.5f); b.rot=e->rotation;
            Vector3 ax,ay,az; obb_axes(b.rot,&ax,&ay,&az); CapsuleContact worst=NO_CONTACT;
            for (int cx=-1; cx<=1; cx+=2) for (int cy=-1; cy<=1; cy+=2) for (int cz=-1; cz<=1; cz+=2) {
                Vector3 corner=Vector3_A_plus_B(b.center,Vector3_A_plus_B(Vector3_A_plus_B(scale_vector3(ax,b.halfExtents.x*(float)cx),scale_vector3(ay,b.halfExtents.y*(float)cy)),scale_vector3(az,b.halfExtents.z*(float)cz)));
                CapsuleContact c=QueryCapsuleContact(corner,corner,0.004f,mask);
                if (c.depth>worst.depth&&c.normal.y>-0.1f) worst=c;
            }
            return worst;
        }
        default: return QueryCapsuleContact(pos,pos,modelBounds[e->modelIndex]>0.01f?modelBounds[e->modelIndex]:0.5f,mask);
    }
}

ENGINE_TO_MOD bool CheckCapsule(Vector3 start,Vector3 end,float capsuleRadius,float capsuleHeight,u32 layerMask) { (void)capsuleHeight; return QueryCapsuleContact(start,end,capsuleRadius,layerMask).depth>0.0f; }

static inline void CapsuleTipsFromEye(Vector3 eye,Vector3 *start,Vector3 *end) {
    float hi=(PLAYER_HEIGHT-2.0f*PLAYER_RADIUS)*0.5f, cy=eye.y-PLAYER_CAM_OFFSET_Y;
    *start=(Vector3){eye.x,cy-hi,eye.z}; *end=(Vector3){eye.x,cy+hi,eye.z};
}
static float SnapEyeAboveFloor(float ex,float ey,float ez,u32 mask) {
    float corrected=ey, pushed=0.0f;
    while (pushed<SNAP_MAX) {
        Vector3 s,e; CapsuleTipsFromEye((Vector3){ex,corrected,ez},&s,&e);
        if (QueryCapsuleContact(s,e,PLAYER_RADIUS,mask).depth<=0.0f) break;
        corrected+=SNAP_STEP; pushed+=SNAP_STEP;
    }
    return corrected;
}
void BuildPlayerCapsule(u16 playerIdx,Vector3 *start,Vector3 *end) {
    Vector3 eye=Sys_Global.instances[playerIdx].position;
    float hi=(PLAYER_HEIGHT-2.0f*PLAYER_RADIUS)*0.5f, cy=eye.y-PLAYER_CAM_OFFSET_Y;
    start->x=eye.x; start->y=cy-hi; start->z=eye.z; end->x=eye.x; end->y=cy+hi; end->z=eye.z;
}

ENGINE_TO_MOD void AddForce(u16 idx,Vector3 force,bool isImpulse) {
    if (idx>=INSTANCE_COUNT) return;
    Entity *e=&Sys_Global.instances[idx]; float mass=e->mass>0.0001f?e->mass:1.0f;
    e->accumulatedForce=Vector3_A_plus_B(e->accumulatedForce,force);
    if (isImpulse) e->velocity=Vector3_A_plus_B(e->velocity,scale_vector3(force,1.0f/mass));
    else           e->velocity=Vector3_A_plus_B(e->velocity,scale_vector3(force,(float)Sys_Global.timeSinceLastPhysicsTick/mass));
}

ENGINE_TO_MOD void ApplyPlayerMovements(void) {
    Vector3 fwd=Sys_Global.instances[PLAYER1].forward, right=Sys_Global.instances[PLAYER1].right, input={0};
    if (Forward())     input=Vector3_A_plus_B(input,(Vector3){fwd.x,0,fwd.z});
    if (Backpedal())   input=Vector3_A_minus_B(input,(Vector3){fwd.x,0,fwd.z});
    if (StrafeRight()) input=Vector3_A_plus_B(input,(Vector3){right.x,0,right.z});
    if (StrafeLeft())  input=Vector3_A_minus_B(input,(Vector3){right.x,0,right.z});
    if (SwimDn())      input.y-=1.0f;
    if (SwimUp())      input.y+=1.0f;
    input=normalize_vector3(input);
    float speed=GetBasePlayerSpeed(PLAYER1,magnitude_vector3(input)>0.1f)*1.75f, accel=Sys_Global.boosterActive?1.0f:3.0f;
    Vector3 cur=Sys_Global.instances[PLAYER1].velocity, dv=Vector3_A_minus_B(scale_vector3(input,speed),cur);
    dv.x=vmax(vmin(dv.x,10.0f),-10.0f); dv.y=vmax(vmin(dv.y,10.0f),-10.0f); dv.z=vmax(vmin(dv.z,10.0f),-10.0f);
    Sys_Global.instances[PLAYER1].velocity=Vector3_A_plus_B(cur,scale_vector3(dv,accel*(float)Sys_Global.timeSinceLastPhysicsTick));
}

const Vector3 gravityVelocity={0.0f,-9.81f,0.0f};
void UpdateVelocityFromGravity(void) {
    if (Sys_Global.pauseRelativeTime<10.0f) return;
    for (u32 i=PLAYER1; i<INSTANCE_COUNT; ++i) {
        if (i>(u32)Sys_Global.loadedInstances) return;
        if (Sys_Global.instances[i].gravity<0.01f&&Sys_Global.instances[i].gravity>-0.01f) continue;
        if (i<=(u32)PLAYER2&&Sys_Cheats.noclip) continue;
        Sys_Global.instances[i].velocity=Vector3_A_plus_B(Sys_Global.instances[i].velocity,scale_vector3(gravityVelocity,Sys_Global.instances[i].gravity*(float)Sys_Global.timeSinceLastPhysicsTick));
    }
}

void ApplyCorpseFriction(u16 idx) {
    Sys_Global.instances[idx].dynamicFriction=10.0f; Sys_Global.instances[idx].staticFriction=10.0f; Sys_Global.instances[idx].bounciness=0.0f;
    Sys_Global.instances[idx].frictionCombine=PHYS_COMBINE_MUL; Sys_Global.instances[idx].bounceCombine=PHYS_COMBINE_MAX;
}

bool GridCellBlock(u16 i,Vector3 pos,Vector3 newPos);
static void IntegrateRigidbody(u16 i,float dt) {
    Entity *e=&Sys_Global.instances[i];
    if (!(e->entflags&ENTFLAG_ACTIVE)||!(e->entflags&ENTFLAG_RIGIDBODY)||e->entflags&ENTFLAG_ASLEEP||e->entflags&ENTFLAG_KINEMATIC) return;
    if (magnitude_vector3(e->velocity)<0.005f) return;
    Vector3 pos=e->position, newPos=Vector3_A_plus_B(pos,scale_vector3(e->velocity,dt));
    if (GridCellBlock(i,pos,newPos)) return;
    for (int iter=0; iter<4; ++iter) {
        CapsuleContact c=QueryRigidbodyWorldContact(i,newPos);
        if (c.depth<=0.0f) break;
        Vector3 motion=Vector3_A_minus_B(newPos,pos); float motionLen=magnitude_vector3(motion);
        if (motionLen>1e-5f&&dot_vector3(c.normal,scale_vector3(motion,1.0f/motionLen))>0.5f) {
            Vector3 safePos=pos, testPos=newPos;
            for (int bi=0; bi<6; ++bi) {
                Vector3 mid={(safePos.x+testPos.x)*0.5f,(safePos.y+testPos.y)*0.5f,(safePos.z+testPos.z)*0.5f};
                if (QueryRigidbodyWorldContact(i,mid).depth>0.0f) testPos=mid; else safePos=mid;
            }
            newPos=safePos; e->velocity=(Vector3){0,0,0};
            goto position_resolved;
        }
        newPos=Vector3_A_plus_B(newPos,scale_vector3(c.normal,c.depth+0.0005f));
        float vn=dot_vector3(e->velocity,c.normal);
        if (vn<0.0f) e->velocity=Vector3_A_minus_B(e->velocity,scale_vector3(c.normal,vn));
    }
    position_resolved:;
    e->lastPosition=pos; e->position=newPos; e->cellIndex=PosGetCellCoords(newPos.x,newPos.z); e->cellX=PosGetCellCoordX(e->position.x); e->cellZ=PosGetCellCoordZ(e->position.z); Sys_Global.dirtyInstances[i]=true;
}

static void IntegratePlayer(u16 i,float dt) {
    Entity *e=&Sys_Global.instances[i]; Vector3 pos=e->position; e->cellIndex=PosGetCellCoords(pos.x,pos.z); e->cellX=PosGetCellCoordX(e->position.x); e->cellZ=PosGetCellCoordZ(e->position.z); Vector3 vel=e->velocity;
    if (i<=PLAYER2&&Sys_Cheats.noclip) { e->position=Vector3_A_plus_B(pos,scale_vector3(vel,dt)); return; }
    if (magnitude_vector3(vel)<0.05f) return;
    Vector3 dir=normalize_vector3(vel);
    if (GridCellBlock(i,pos,Vector3_A_plus_B(Vector3_A_plus_B(pos,scale_vector3(dir,PLAYER_RADIUS)),scale_vector3(vel,dt)))) return;
    u32 mask=GetCollisionMask(e->layer); float innerSpine=PLAYER_HEIGHT-2.0f*PLAYER_RADIUS; bool boosted=Sys_Global.boosterActive;
    {
        Vector3 s,en; CapsuleTipsFromEye(pos,&s,&en); CapsuleContact c=QueryCapsuleContact(s,en,PLAYER_RADIUS,mask);
        if (c.depth>0.0f) {
            pos.x+=c.normal.x*(c.depth+SNAP_STEP); pos.y+=c.normal.y*(c.depth+SNAP_STEP); pos.z+=c.normal.z*(c.depth+SNAP_STEP);
            CapsuleTipsFromEye(pos,&s,&en);
            if (QueryCapsuleContact(s,en,PLAYER_RADIUS,mask).depth>0.0f) pos.y=SnapEyeAboveFloor(pos.x,pos.y,pos.z,mask);
            e->position=pos; vel=e->velocity;
        }
    }
    float centreY=pos.y-PLAYER_CAM_OFFSET_Y;
    Vector3 curStart={pos.x,centreY-innerSpine*0.5f,pos.z}, curEnd={pos.x,centreY+innerSpine*0.5f,pos.z};
    bool isGrounded=false; float slopeDeg=0.0f; Vector3 floorNormal={0,1,0};
    if (vel.y<=0.05f) {
        float snapRange=vmin(GROUND_PROBE_DIST,vmax(0.08f,vabs(vel.y)*dt+0.04f));
        Vector3 pS,pE; CapsuleTipsFromEye((Vector3){pos.x,pos.y-snapRange,pos.z},&pS,&pE);
        CapsuleContact probe=QueryCapsuleContact(pS,pE,PLAYER_RADIUS,mask);
        if (probe.depth>0.0f&&probe.normal.y>0.1f) {
            floorNormal=probe.normal; isGrounded=true;
            slopeDeg=(180.0f/3.14159265f)*vacosf(vmax(-1.0f,vmin(1.0f,floorNormal.y)));
            float floorY=pos.y;
            for (float d=SNAP_STEP; d<=snapRange; d+=SNAP_STEP) {
                Vector3 s,en; CapsuleTipsFromEye((Vector3){pos.x,pos.y-d,pos.z},&s,&en);
                if (QueryCapsuleContact(s,en,PLAYER_RADIUS,mask).depth>0.0f) break;
                floorY=pos.y-d;
            }
            pos.y=floorY; e->position=pos; e->velocity.y=0.0f; vel.y=0.0f;
            centreY=pos.y-PLAYER_CAM_OFFSET_Y;
            curStart=(Vector3){pos.x,centreY-innerSpine*0.5f,pos.z}; curEnd=(Vector3){pos.x,centreY+innerSpine*0.5f,pos.z};
        }
    }
    if (isGrounded) {
        if (slopeDeg>SLOPE_CLIMB_MAX_DEG) {
            float vdn=dot_vector3(vel,floorNormal);
            if (vdn>0.0f) { vel.x-=floorNormal.x*vdn; vel.y-=floorNormal.y*vdn; vel.z-=floorNormal.z*vdn; }
            float gdn=-9.81f*floorNormal.y, accel=boosted?SLOPE_SLIDE_ACCEL_BOOST:SLOPE_SLIDE_ACCEL;
            Vector3 slide={-floorNormal.x*gdn,-9.81f-floorNormal.y*gdn,-floorNormal.z*gdn}; float slen=magnitude_vector3(slide);
            if (slen>1e-4f) { vel.x+=(slide.x/slen)*accel*dt; vel.y+=(slide.y/slen)*accel*dt; vel.z+=(slide.z/slen)*accel*dt; }
        } else if (slopeDeg>SLOPE_WALK_MAX_DEG) { float t=(slopeDeg-SLOPE_WALK_MAX_DEG)/(SLOPE_CLIMB_MAX_DEG-SLOPE_WALK_MAX_DEG); vel.x*=(1.0f-t); vel.z*=(1.0f-t); }
        float frAccel=boosted?SLOPE_FRICTION_ACCEL_BOOST:SLOPE_FRICTION_ACCEL, hspeed=vsqrtf(vel.x*vel.x+vel.z*vel.z);
        if (hspeed>1e-4f) { float fd=frAccel*dt; if (fd>=hspeed) { vel.x=0.0f; vel.z=0.0f; } else { float s=(hspeed-fd)/hspeed; vel.x*=s; vel.z*=s; } }
        e->velocity=vel;
    }
    if (vel.y*vel.y>1e-6f) {
        bool movingDown=vel.y<0.0f; Vector3 vS,vE; CapsuleTipsFromEye((Vector3){pos.x,pos.y+vel.y*dt,pos.z},&vS,&vE);
        CapsuleContact vc=QueryCapsuleContact(vS,vE,PLAYER_RADIUS,mask); bool blockV=false;
        if (movingDown) blockV=(vc.depth>0.0f&&vc.normal.y>0.1f);
        else if (vc.depth>0.0f&&vc.normal.y<-0.1f) blockV=(vc.depth>QueryCapsuleContact(curStart,curEnd,PLAYER_RADIUS,mask).depth);
        if (blockV) { e->velocity.y=0.0f; vel.y=0.0f; }
    }
    Vector3 hVel={vel.x,0.0f,vel.z};
    if (hVel.x*hVel.x+hVel.z*hVel.z>1e-6f) {
        Vector3 hS,hE; CapsuleTipsFromEye((Vector3){pos.x+hVel.x*dt,pos.y,pos.z+hVel.z*dt},&hS,&hE);
        CapsuleContact hc=QueryCapsuleContact(hS,hE,PLAYER_RADIUS,mask);
        if (hc.depth>0.0f&&hc.depth>QueryCapsuleContact(curStart,curEnd,PLAYER_RADIUS,mask).depth) {
            float vdn=dot_vector3(vel,hc.normal);
            if (vdn<0.0f) { vel.x-=hc.normal.x*vdn; vel.z-=hc.normal.z*vdn; e->velocity.x=vel.x; e->velocity.z=vel.z; }
        }
    }
    vel=e->velocity;
    if (magnitude_vector3(vel)<0.05f) return;
    e->lastPosition=pos; e->position=Vector3_A_plus_B(pos,scale_vector3(vel,dt)); Sys_Global.dirtyInstances[i]=true;
}

extern ma_engine audio_engine;
void UpdatePositions(void) {
    float dt=vclamp((float)Sys_Global.timeSinceLastPhysicsTick,0.0005f,0.027777778f);
    for (u32 i=PLAYER1; i<=PLAYER2; ++i) IntegratePlayer((u16)i,dt);
    for (u32 i=START_INDEX_LEVEL_INSTANCES; i<(u32)Sys_Global.loadedInstances; ++i)
        if (Sys_Global.instances[i].entflags&ENTFLAG_RIGIDBODY) IntegrateRigidbody((u16)i,dt);
    ma_engine_listener_set_position(&audio_engine,0,Sys_Global.instances[PLAYER1].position.x,Sys_Global.instances[PLAYER1].position.y,Sys_Global.instances[PLAYER1].position.z);
}

void ClampVelocity(void) {
    for (i32 i=START_INDEX_LEVEL_INSTANCES; i<Sys_Global.loadedInstances; ++i) {
        Vector3 v=Sys_Global.instances[i].velocity;
        if (magnitude_vector3(v)>TERMINAL_VELOCITY) Sys_Global.instances[i].velocity=scale_vector3(normalize_vector3(v),TERMINAL_VELOCITY);
    }
}

void UpdateTriggers(void);
void Physics(void) {
//     UpdateVelocityFromGravity();
//     Physics_PrimitiveStep((float)Sys_Global.timeSinceLastPhysicsTick);
    ClampVelocity(); UpdatePositions(); UpdateTriggers(); Physics_DrawDebug();
}
