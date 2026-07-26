// physics.c - The Jack Physics Engine, By W. Josiah Jack MIT-0 -- full rigidbody 3D with torque for sphere, box, capsule, convex mesh dynamic objects and same set plus arbitrary trisoup mesh colliders for statics.
#include "common.h"
#include "lib.h"
#include "trigger.c" // Trigger Volumes System
u16 cellLists[WORLDX*WORLDX][128],cellCounts[WORLDX*WORLDX];
const float PHY_EPSILON=0.0001f,PHY_NEARNUFF=0.001f,MAX_SPEED=16.666666f/*m/s fastest is railgun given 5.0 impulse w/ 0.3 mass=5.0/0.3 */,MAX_STEP_SIZE=(0.08f / 16.666666f),MAX_ANGULAR_SPEED=8.0f/*arbitrary*/,MANIFOLD_TIE_MARGIN=0.005f,MANIFOLD_ALIGN_THRESHOLD=0.8f;
const float WALK_SPEED=3.6f,SPRINT_SPEED=8.8f,PLAYER_MAX_CYBER_SPEED=5.0f,SPRINT_SPEED_FATIGUED=5.5f;
#define CROUCH_SPEED 1.25f
#define PLAYER_MAX_PRONE_SPEED 0.5f
#define PLAYER_BOOSTER_SPEED_BOOST 1.2f
#define PLAYER_CROUCH_RATIO 0.6f
#define PLAYER_PRONE_RATIO 0.01f
#define PLAYER_TRANSITION_TO_PRONE_ADD 0.10f
enum {MANIFOLD_MAX=4,CVXMSH_HULL_CACHE=1024,EPA_MAX_FACES=64,EPA_MAX_VERTS=128,EPA_MAX_EDGES=EPA_MAX_FACES * 3,GJK_ITER=16,EPA_ITER=16};
typedef struct { V3 v[4];/*Minkowski difference verts (wA - wB)*/   V3 wA[4],wB[4];/*Cached support points from Shape A,B*/ i32 n;/*Vertex count*/ } Simplex3D;
typedef struct { V3 point; float pen; } ManifoldPt; typedef struct { V3 normal; ManifoldPt p[MANIFOLD_MAX]; i32 n; float maxPen; } Manifold;
float posBudget[INSTANCE_COUNT]; // Remaining |Δpos| entity i may receive this substep; reset every substep in Physics().
u16 dynamicEntities[512],dynamicEntityCount;
INLINE void SetPosition(u16 i, V3 newpos) { float d=V3_Dist(World.position[i],newpos); if(d < PHY_NEARNUFF){return;} float allowed=vmin(d,posBudget[i]); if(allowed < PHY_NEARNUFF){return;} V3 dir=V3_Normalize(V3_AsubB(newpos,World.position[i])); World.position[i]=V3_AplusB(World.position[i],V3_ScaleByF(dir,allowed)); posBudget[i] -= allowed; }
INLINE Manifold OverlapToManifold(Overlap r) { Manifold m={0}; if (r.hit && r.pen > PHY_EPSILON) { m.normal = r.normal; m.n = 1; m.p[0] = (ManifoldPt){r.point, r.pen}; m.maxPen = r.pen; } return m; }
INLINE Overlap SphSph(V3 a, float ar, V3 b, float br) { V3 dt=V3_AsubB(a,b); float d2=V3_dot(dt,dt),rs=ar+br; float h=(d2<rs*rs); float d=vsqrtf(vmax(d2,0.0f)); float m=(d<PHY_EPSILON); V3 n=V3_AplusB(V3_ScaleByF(dt,(1.0f/vmax(d,PHY_EPSILON))*(1.0f-m)),V3_ScaleByF((V3){0,1,0},m)); V3 point=V3_AplusB(b,V3_ScaleByF(n,br)); return (Overlap){(bool)h,point,n,(rs-d)*h}; }
Overlap SphCap(ShapeSphere s, ShapeCapsule c) { V3 seg=V3_AsubB(c.tip,c.base); float l=V3_dot(seg,seg); float m=(l < PHY_EPSILON); V3 b=V3_AplusB(c.base, V3_ScaleByF(seg,vclamp(V3_dot(V3_AsubB(s.ctr, c.base),seg) / vmax(l, PHY_EPSILON), 0.0f, 1.0f) * (1.0f - m))); b = V3_AplusB(V3_ScaleByF(b,1.0f - m),V3_ScaleByF(c.base,m)); return SphSph(s.ctr,s.rad,b,c.rad); }
Overlap CapCap(ShapeCapsule a, ShapeCapsule b) {
    Overlap r={0}; float sc,tc,distSq, radSum=a.rad + b.rad; V3 d1 = V3_AsubB(a.tip,a.base), d2=V3_AsubB(b.tip,b.base), vr=V3_AsubB(a.base,b.base);
    float qa=V3_dot(d1,d1), e=V3_dot(d2,d2), f=V3_dot(d2,vr); if(qa < PHY_EPSILON && e < PHY_EPSILON){sc=tc=0.0f; distSq=V3_dot(vr,vr);}else if(qa < PHY_EPSILON){sc=0.0f; tc=vclamp(f/e,0.0f,1.0f);}
    else { float c=V3_dot(d1,vr); if (e < PHY_EPSILON) { tc=0.0f; sc=vclamp(-c/qa,0.0f,1.0f); } else { float qb=V3_dot(d1,d2), denom=qa*e - qb*qb; sc=(denom > PHY_EPSILON) ? vclamp((qb*f - c*e)/denom,0.0f,1.0f) : 0.0f; tc=(qb*sc + f)/e; if(tc < 0.0f){tc=0.0f; sc=vclamp(-c/qa,0.0f,1.0f);}else if(tc > 1.0f){tc=1.0f; sc=vclamp((qb-c)/qa,0.0f,1.0f);} } }
    V3 ptA = V3_AplusB(a.base,V3_ScaleByF(d1,sc)), ptB = V3_AplusB(b.base,V3_ScaleByF(d2,tc)), diff = V3_AsubB(ptA,ptB); distSq = V3_dot(diff,diff); if(distSq >= radSum * radSum) return r;
    float dist = vsqrtf(vmax(distSq,0.0f)); r.pen = radSum - dist; r.hit = true; r.normal = (dist < PHY_EPSILON) ? (V3){0,1,0} : V3_ScaleByF(diff,1.0f/dist); r.point = V3_AplusB(ptB,V3_ScaleByF(r.normal,b.rad)); return r;
}

INLINE Overlap SphBoxAxes(V3 ctr, float rad, V3 boxCtr, V3 ax, V3 ay, V3 az, V3 hExt) {
    Overlap r={0}; V3 d = V3_AsubB(ctr,boxCtr); float lx = V3_dot(d,ax), ly = V3_dot(d,ay), lz = V3_dot(d,az);
    V3 localClosest = V3_AplusB(V3_AplusB(V3_ScaleByF(ax,vclamp(lx,-hExt.x,hExt.x)), V3_ScaleByF(ay,vclamp(ly,-hExt.y,hExt.y))), V3_ScaleByF(az,vclamp(lz,-hExt.z,hExt.z))); V3 delta = V3_AsubB(d,localClosest);
    float distSq = V3_dot(delta,delta);
    if (distSq >= rad * rad) return r;
    r.hit = true; float dist = vsqrtf(vmax(distSq, 0.0f));
    if (dist > PHY_EPSILON) { r.normal = V3_ScaleByF(delta, 1.0f/dist); r.pen=rad - dist; }
    else {
        float dx=hExt.x - vabs(lx), dy=hExt.y - vabs(ly), dz=hExt.z - vabs(lz);
        float mind=dx; V3 nAx = V3_ScaleByF(ax, lx > 0.f ? 1.f : -1.f);
        if(dy < mind){mind=dy; nAx = V3_ScaleByF(ay, ly > 0.f ? 1.f : -1.f);}
        if(dz < mind){mind=dz; nAx = V3_ScaleByF(az, lz > 0.f ? 1.f : -1.f);}
        r.normal = nAx; r.pen = rad + mind;
    }
    r.point = V3_AsubB(ctr, V3_ScaleByF(r.normal, rad - r.pen));
    return r;
}

Overlap SphBox(V3 ctr, float rad, ShapeBox box) { V3 ax=quat_rot_v3(box.rot,(V3){1,0,0}), ay=quat_rot_v3(box.rot,(V3){0,1,0}), az=quat_rot_v3(box.rot,(V3){0,0,1}); return SphBoxAxes(ctr, rad, box.ctr, ax, ay, az, box.hExt); }
Overlap CapBox(ShapeCapsule c,ShapeBox b){ 
    V3 ax=quat_rot_v3(b.rot,(V3){1,0,0}), ay=quat_rot_v3(b.rot,(V3){0,1,0}), az=quat_rot_v3(b.rot,(V3){0,0,1}); V3 d=V3_AsubB(c.tip,c.base); 
    Overlap best=SphBoxAxes(c.base,c.rad,b.ctr,ax,ay,az,b.hExt), r=SphBoxAxes(c.tip,c.rad,b.ctr,ax,ay,az,b.hExt); if(r.pen>best.pen){best=r;}
    if(V3_dot(d,d)>PHY_EPSILON*PHY_EPSILON) { for(int k=1;k<8;k++){ r=SphBoxAxes(V3_AplusB(c.base,V3_ScaleByF(d,k*.125f)),c.rad,b.ctr,ax,ay,az,b.hExt); if(r.pen>best.pen){best=r;} } }
    return best; 
}

static const u32 CollisionMaskTable[32] = {
    [0]  = L_Default|L_TransparentFX|L_Geometry|L_NPC|L_PlayerBullets|L_Player|L_Corpse|L_PhysObjects|L_Trigger|L_Door|L_InterDebris|L_Player2|L_NPCBullet|L_Clip|L_CorpseSearchable, // L_Default
    [1]  = L_Default|L_TransparentFX|L_Geometry|L_NPC|L_PlayerBullets|L_Player|L_PhysObjects|L_Trigger|L_Door|L_InterDebris|L_Player2|L_NPCBullet|L_Clip,                             // L_TransparentFX
    [9]  = L_Default|L_TransparentFX|L_Geometry|L_NPC|L_PlayerBullets|L_Player|L_PhysObjects|L_Trigger|L_Door|L_InterDebris|L_Player2|L_Clip,                                         // L_Geometry
    [10] = L_Default|L_TransparentFX|L_Geometry|L_NPC|L_PlayerBullets|L_Player|L_PhysObjects|L_Trigger|L_NPCTrigger|L_Door|L_InterDebris|L_Player2|L_NPCBullet|L_NPCClip|L_Clip,      // L_NPC
    [11] = L_Default|L_TransparentFX|L_Geometry|L_NPC|L_PlayerBullets|L_Player|L_Corpse|L_PhysObjects|L_Door|L_InterDebris|L_Player2|L_NPCBullet|L_Clip|L_CorpseSearchable,           // L_PlayerBullets
    [12] = L_Default|L_TransparentFX|L_Geometry|L_NPC|L_PhysObjects|L_PlayerTriggerOnly|L_Trigger|L_Door|L_Player2|L_NPCBullet|L_Clip,                                                // L_Player
    [13] = L_Default|L_Geometry|L_PlayerBullets|L_PhysObjects|L_Door|L_NPCBullet|L_Clip,                                                                                              // L_Corpse
    [14] = L_Default|L_TransparentFX|L_Geometry|L_NPC|L_PlayerBullets|L_Player|L_Corpse|L_PhysObjects|L_Door|L_InterDebris|L_NPCBullet|L_Clip,                                        // L_PhysObjects
    [16] = L_Player|L_Player2,                                                                                                                                                        // L_PlayerTriggerOnly
    [17] = L_Default|L_Geometry|L_NPC|L_PlayerBullets|L_Player|L_PhysObjects|L_Door|L_InterDebris|L_Clip,                                                                             // L_Trigger
    [18] = L_Default|L_TransparentFX|L_Geometry|L_NPC|L_PlayerBullets|L_Player|L_Corpse|L_PhysObjects|L_Trigger|L_Door|L_InterDebris|L_Player2|L_NPCBullet|L_Clip,                    // L_Door
    [19] = L_Default|L_Geometry|L_NPC|L_PlayerBullets|L_PhysObjects|L_Trigger|L_Door|L_NPCBullet|L_Clip,                                                                              // L_InterDebris
    [20] = L_Default|L_TransparentFX|L_Geometry|L_NPC|L_PlayerBullets|L_Player|L_PhysObjects|L_PlayerTriggerOnly|L_Trigger|L_Door|L_InterDebris|L_Player2|L_NPCBullet|L_Clip,         // L_Player2
    [23] = L_Default|L_TransparentFX|L_Geometry|L_NPC|L_PlayerBullets|L_Player|L_PhysObjects|L_Trigger|L_NPCTrigger|L_Door|L_InterDebris|L_Player2|L_NPCBullet|L_NPCClip|L_Clip,      // L_NPCTrigger (Copy of L_NPC)
    [24] = L_Default|L_TransparentFX|L_Geometry|L_PlayerBullets|L_Player|L_Corpse|L_PhysObjects|L_Door|L_InterDebris|L_Player2|L_Clip|L_CorpseSearchable,                             // L_NPCBullet
    [26] = L_Player|L_Player2|L_NPC,                                                                                                                                                  // L_Clip
    [25] = L_Default|L_TransparentFX|L_Geometry|L_NPC|L_PlayerBullets|L_Player|L_PhysObjects|L_Trigger|L_NPCTrigger|L_Door|L_InterDebris|L_Player2|L_NPCBullet|L_NPCClip|L_Clip,      // L_NPCClip (Copy of L_NPC)
    [29] = L_Default|L_PlayerBullets,                                                                                                                                                 // L_CorpseSearchable
};

u32 GetCollisionMask(u32 layer) { u32 ctz = __builtin_ctz(layer | 1); u32 valid = (ctz < 32); return ((layer == L_NPCTrigger) | (layer == L_NPCClip)) ? L_NPC : (CollisionMaskTable[ctz * valid] * valid); }
ShapeCapsule Entity_GetCap(u16 i) {
    float scaleMax = vmax(World.scale[i].x,vmax(World.scale[i].y,World.scale[i].z));
    float r = World.colliderSize[i].x * scaleMax; float hi = vmax(0.0f, (World.colliderSize[i].y * 0.5f * scaleMax) - r); V3 wc,axis;
    if (i == PLAYER1 || World.layer[i] == L_NPC) { wc = V3_AplusB(World.position[i], World.colliderCenter[i]); axis = (V3){0.0f,1.0f,0.0f};/*Player+NPC remain strictly upright*/ }
    else { wc = V3_AplusB(World.position[i], quat_rot_v3(World.rotation[i], World.colliderCenter[i])); axis = (World.colliderSize[i].z < 0.5f) ? quat_rot_v3(World.rotation[i], (V3){1,0,0}) : (World.colliderSize[i].z < 1.5f) ? quat_rot_v3(World.rotation[i], (V3){0,1,0}) : quat_rot_v3(World.rotation[i], (V3){0,0,1}); }
    return (ShapeCapsule){.tip=V3_AplusB(wc,V3_ScaleByF(axis,hi)),.base=V3_AsubB(wc,V3_ScaleByF(axis,hi)),.rad=r};
}

ShapeBox Entity_GetBox(u16 i) { return (ShapeBox){.ctr=V3_AplusB(World.position[i],quat_rot_v3(World.rotation[i],World.colliderCenter[i])),.hExt=(V3){World.colliderSize[i].x*0.5f * World.scale[i].x,World.colliderSize[i].y*0.5f * World.scale[i].y,World.colliderSize[i].z*0.5f * World.scale[i].z},.rot=World.rotation[i]}; }
ShapeSphere Entity_GetSph(u16 i) { return (ShapeSphere){.ctr=V3_AplusB(World.position[i],quat_rot_v3(World.rotation[i],World.colliderCenter[i])),.rad = World.colliderSize[i].x * vmax(World.scale[i].x,vmax(World.scale[i].y,World.scale[i].z))}; }
Quaternion quat_from_axis_angle(V3 axis, float angle) { float half = angle * 0.5f; float s = vsinf(half); return (Quaternion){axis.x*s,axis.y*s,axis.z*s,vcosf(half)}; }
Quaternion quat_normalize(Quaternion q) { float l = q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w; float m = (l < PHY_EPSILON); float inv=vinvsqtf(vmax(l,PHY_EPSILON)); q.x*=inv; q.y*=inv; q.z*=inv; q.w*=inv; q.x=q.x*(1.0f - m); q.y=q.y*(1.0f - m); q.z=q.z*(1.0f - m); q.w=q.w*(1.0f - m) + 1.0f*m; return q; }
INLINE V3 MeshVert(u16 m, u32 i) { const float* p = physPos[m] + i * 3; return (V3){p[0],p[1],p[2]}; }
void ComputeConvexMeshInertiaTensor(u16 i) {
    u16 mi = World.instances[i].colMeshIndex; World.invTnsrValid[i]=false; if (mi >= MAX_MDLS || !modelTriangleCounts[mi] || !modelVertexCounts[mi]) {return;}
    float acc[6]={0}; float cm[3]={0}; float volAcc=0.0f; u32 triCount = modelTriangleCounts[mi];
    for (u32 ti=0;ti<triCount;++ti) {
        u32 i0 = modelTriangles[mi][ti*3+0], i1 = modelTriangles[mi][ti*3+1], i2 = modelTriangles[mi][ti*3+2];
        V3 v0=MeshVert(mi,i0), v1=MeshVert(mi,i1), v2=MeshVert(mi,i2);
        float det = V3_dot(v0,V3_Cross(v1,v2)); volAcc += det;
        cm[0] += det*(v0.x + v1.x + v2.x); cm[1] += det*(v0.y + v1.y + v2.y); cm[2] += det*(v0.z + v1.z + v2.z);
        acc[0] += det*(v0.x*v0.x + v0.x*v1.x + v1.x*v1.x + v0.x*v2.x + v1.x*v2.x + v2.x*v2.x);
        acc[1] += det*(v0.y*v0.y + v0.y*v1.y + v1.y*v1.y + v0.y*v2.y + v1.y*v2.y + v2.y*v2.y);
        acc[2] += det*(v0.z*v0.z + v0.z*v1.z + v1.z*v1.z + v0.z*v2.z + v1.z*v2.z + v2.z*v2.z);
        acc[3] += det*(2.0f*(v0.x*v0.y + v1.x*v1.y + v2.x*v2.y) + v0.x*v1.y + v1.x*v0.y + v0.x*v2.y + v2.x*v0.y + v1.x*v2.y + v2.x*v1.y);
        acc[4] += det*(2.0f*(v0.x*v0.z + v1.x*v1.z + v2.x*v2.z) + v0.x*v1.z + v1.x*v0.z + v0.x*v2.z + v2.x*v0.z + v1.x*v2.z + v2.x*v1.z);
        acc[5] += det*(2.0f*(v0.y*v0.z + v1.y*v1.z + v2.y*v2.z) + v0.y*v1.z + v1.y*v0.z + v0.y*v2.z + v2.y*v0.z + v1.y*v2.z + v2.y*v1.z);
    }
    if (vabs(volAcc) < PHY_EPSILON) return;
    float sx = World.scale[i].x, sy = World.scale[i].y, sz = World.scale[i].z; float sd = World.mass[i] / (volAcc * 10.0f), so = World.mass[i] / (volAcc * 20.0f);
    float cx = cm[0] / (4.0f * volAcc), cy = cm[1] / (4.0f * volAcc), cz = cm[2] / (4.0f * volAcc); float scx = cx * sx, scy = cy * sy, scz = cz * sz, m=World.mass[i];
    float Ixx = sd*(acc[1]*sy*sy + acc[2]*sz*sz) - m*(scy*scy + scz*scz); float Iyy = sd*(acc[0]*sx*sx + acc[2]*sz*sz) - m*(scx*scx + scz*scz); float Izz = sd*(acc[0]*sx*sx + acc[1]*sy*sy) - m*(scx*scx + scy*scy);
    float Ixy = -(so*acc[3]*sx*sy - m*scx*scy); float Ixz = -(so*acc[4]*sx*sz - m*scx*scz); float Iyz = -(so*acc[5]*sy*sz - m*scy*scz);
    float r = modelBounds[mi] * vmax(vmax(sx,sy),sz); float mn = 0.04f * m * r * r;
    Ixx = vmax(Ixx,mn); Iyy = vmax(Iyy,mn); Izz = vmax(Izz,mn);
    float *IT=World.inertiaTensor[i]; IT[0]=Ixx; IT[1]=Iyy; IT[2]=Izz; IT[3]=Ixy; IT[4]=Ixz; IT[5]=Iyz;
    float det = Ixx*(Iyy*Izz - Iyz*Iyz) - Ixy*(Ixy*Izz - Ixz*Iyz) + Ixz*(Ixy*Iyz - Iyy*Ixz); if (vabs(det) < PHY_EPSILON) return;
    float invDet = 1.0f / det, *iI=World.invInertiaTensor[i];
    iI[0]=(Iyy*Izz - Iyz*Iyz)*invDet; iI[1]=(Ixx*Izz - Ixz*Ixz)*invDet; iI[2]=(Ixx*Iyy - Ixy*Ixy)*invDet;
    iI[3]=(Ixz*Iyz - Ixy*Izz)*invDet; iI[4]=(Ixy*Iyz - Iyy*Ixz)*invDet; iI[5]=(Ixy*Ixz - Ixx*Iyz)*invDet;
    World.invTnsrValid[i]=true;
}

bool PointInOBB(V3 pt, ShapeBox box) { V3 d=V3_AsubB(pt,box.ctr); V3 ax=quat_rot_v3(box.rot,(V3){1,0,0}), ay=quat_rot_v3(box.rot,(V3){0,1,0}), az=quat_rot_v3(box.rot,(V3){0,0,1}); float lx = V3_dot(d,ax), ly = V3_dot(d,ay), lz = V3_dot(d,az); return (vabs(lx) <= box.hExt.x) && (vabs(ly) <= box.hExt.y) && (vabs(lz) <= box.hExt.z); }
Overlap BoxBox(ShapeBox a, ShapeBox b) {
    Overlap r = {0}; V3 aAxes[3],bAxes[3];
    aAxes[0]=quat_rot_v3(a.rot,(V3){1,0,0}); aAxes[1]=quat_rot_v3(a.rot,(V3){0,1,0}); aAxes[2]=quat_rot_v3(a.rot,(V3){0,0,1});
    bAxes[0]=quat_rot_v3(b.rot,(V3){1,0,0}); bAxes[1]=quat_rot_v3(b.rot,(V3){0,1,0}); bAxes[2]=quat_rot_v3(b.rot,(V3){0,0,1}); 
    V3 T = V3_AsubB(b.ctr,a.ctr); float R[3][3],AbsR[3][3];
    for (int i=0;i<3;i++) for (int j=0;j<3;j++) { R[i][j]=V3_dot(aAxes[i],bAxes[j]); AbsR[i][j]=vabs(R[i][j])+1e-6f; }
    float minOverlap=1e9f; int bestAxis=-1; bool flipNormal=false; V3 bestEdgeAxis={0,1,0};
    for (int i=0;i<3;i++) { float ra=((float*)&a.hExt)[i], rb=b.hExt.x*AbsR[i][0]+b.hExt.y*AbsR[i][1]+b.hExt.z*AbsR[i][2]; float t=vabs(V3_dot(T,aAxes[i])); if(t>ra+rb){return r;} float ov=(ra+rb)-t; if(ov<minOverlap){minOverlap=ov; bestAxis=  i; flipNormal=(V3_dot(T,aAxes[i])<0.f);} }
    for (int i=0;i<3;i++) { float ra=a.hExt.x*AbsR[0][i]+a.hExt.y*AbsR[1][i]+a.hExt.z*AbsR[2][i], rb=((float*)&b.hExt)[i]; float t=vabs(V3_dot(T,bAxes[i])); if(t>ra+rb){return r;} float ov=(ra+rb)-t; if(ov<minOverlap){minOverlap=ov; bestAxis=3+i; flipNormal=(V3_dot(T,bAxes[i])<0.f);} }
    for (int i=0;i<3;i++) for (int j=0;j<3;j++) {
        int i1=(i+1)%3, i2=(i+2)%3, j1=(j+1)%3, j2=(j+2)%3; float t=vabs(V3_dot(T,aAxes[i2])*R[i1][j] - V3_dot(T,aAxes[i1])*R[i2][j]); float ra=((float*)&a.hExt)[i1]*AbsR[i2][j]+((float*)&a.hExt)[i2]*AbsR[i1][j]; float rb=((float*)&b.hExt)[j1]*AbsR[i][j2]+((float*)&b.hExt)[j2]*AbsR[i][j1];
        if(t>ra+rb){return r;} float axLenSq=1.f-(R[i][j]*R[i][j]); if (axLenSq>1e-4f) { float ov=((ra+rb)-t)/vsqrtf(axLenSq); if (ov<minOverlap) { V3 ea=V3_Cross(aAxes[i],bAxes[j]); minOverlap=ov; bestAxis=6+i*3+j; bestEdgeAxis=ea; flipNormal=(V3_dot(T,ea)<0.f); } }
    }
    if (bestAxis<0) return r;
    r.hit=true; r.pen=minOverlap;
    if      (bestAxis<3) r.normal=flipNormal ? aAxes[bestAxis]            : V3_ScaleByF(aAxes[bestAxis],-1.f);
    else if (bestAxis<6) r.normal=flipNormal ? bAxes[bestAxis-3]          : V3_ScaleByF(bAxes[bestAxis-3],-1.f);
    else                 r.normal=flipNormal ? V3_Normalize(bestEdgeAxis) : V3_ScaleByF(V3_Normalize(bestEdgeAxis),-1.f);
    V3 sA=a.ctr;
    sA=V3_AplusB(sA,V3_ScaleByF(aAxes[0],(V3_dot(aAxes[0],r.normal)<0.f?1.f:-1.f)*a.hExt.x));
    sA=V3_AplusB(sA,V3_ScaleByF(aAxes[1],(V3_dot(aAxes[1],r.normal)<0.f?1.f:-1.f)*a.hExt.y));
    sA=V3_AplusB(sA,V3_ScaleByF(aAxes[2],(V3_dot(aAxes[2],r.normal)<0.f?1.f:-1.f)*a.hExt.z));
    r.point=V3_AplusB(sA,V3_ScaleByF(r.normal,minOverlap*0.5f));
    return r;
}

V3 MvVert(const float* M, V3 v) { return (V3){ M[0]*v.x + M[4]*v.y + M[8]*v.z  + M[12], M[1]*v.x + M[5]*v.y + M[9]*v.z  + M[13], M[2]*v.x + M[6]*v.y + M[10]*v.z + M[14] }; }
INLINE void MeshTri(u16 m, u32 ti, const float* mx, V3* a, V3* b, V3* c) { u32 i0=modelTriangles[m][ti*3+0],i1=modelTriangles[m][ti*3+1],i2=modelTriangles[m][ti*3+2]; *a=MvVert(mx,MeshVert(m,i0)); *b=MvVert(mx,MeshVert(m,i1)); *c=MvVert(mx,MeshVert(m,i2)); }
INLINE V3 SphSupport(ShapeSphere b, V3 d) { float L=V3_dot(d,d); float safeL=vmax(L,PHY_EPSILON); float scale=b.rad / vsqrtf(safeL); V3 dir=V3_ScaleByF(d,scale); float mask=(L > PHY_EPSILON); return V3_AplusB(V3_ScaleByF(dir,mask),V3_ScaleByF((V3){0,b.rad,0},1.0f - mask)); }
float copysignf(float magnitude, float sign) { float result; __asm__ ("andps %[sign_mask], %[sign]\n\tandnps %[mag], %[sign_mask]\n\torps %[sign_mask], %[mag]\n\t":[mag] "+x" (magnitude), [sign] "+x" (sign), [sign_mask] "=x" (result): "2" (-0.0f)); return magnitude; } // Loads the sign bit mask (0x80000000) into sign_mask
INLINE V3 BoxSupport(ShapeBox b, V3 d) { V3 x=quat_rot_v3(b.rot,(V3){1,0,0}), y=quat_rot_v3(b.rot,(V3){0,1,0}), z=quat_rot_v3(b.rot,(V3){0,0,1}); float kx = copysignf(1.0f, V3_dot(d, x)); float ky = copysignf(1.0f, V3_dot(d, y)); float kz = copysignf(1.0f, V3_dot(d, z)); return V3_AplusB(V3_AplusB(V3_AplusB(b.ctr, V3_ScaleByF(x, kx * b.hExt.x)), V3_ScaleByF(y, ky * b.hExt.y)), V3_ScaleByF(z, kz * b.hExt.z)); }
INLINE V3 CapsuleSupport(ShapeCapsule cap, V3 d) { float db=V3_dot(cap.base,d), dt=V3_dot(cap.tip,d); float mask=(dt > db); V3 best=V3_AplusB(V3_ScaleByF(cap.tip,mask),V3_ScaleByF(cap.base,1.0f - mask)); float L = V3_dot(d,d); float safeL=vmax(L,PHY_EPSILON); V3 dir=V3_ScaleByF(d,cap.rad / vsqrtf(safeL)); float lmask=(L >= PHY_EPSILON); return V3_AplusB(best,V3_ScaleByF(dir,lmask)); }
typedef union { __m128 f; __m128i i; } m128_bits;
// INLINE __m128  _mm_set_ps(float e3, float e2, float e1, float e0) { return (__m128){e0,e1,e2,e3}; }
// INLINE __m128i _mm_setzero_si128(void) { return (__m128i)(__v4si){0, 0, 0, 0}; }
// INLINE __m128i _mm_set1_epi32(int x) { return (__m128i)(__v4si){x, x, x, x}; }
// INLINE __m128i _mm_set_epi32(int e3, int e2, int e1, int e0) { return (__m128i)(__v4si){e0, e1, e2, e3}; }
// INLINE __m128i _mm_add_epi32(__m128i a, __m128i b) { return a + b; }
// INLINE __m128i _mm_and_si128(__m128i a, __m128i b) { return a & b; }
// INLINE __m128i _mm_or_si128(__m128i a, __m128i b)  { return a | b; }
// INLINE __m128i _mm_andnot_si128(__m128i a, __m128i b) { return (~a) & b; }
// INLINE __m128i _mm_castps_si128(__m128 a) { m128_bits u; u.f = a; return u.i; }
// INLINE __m128  _mm_cmpgt_ps(__m128 a, __m128 b) { m128_bits u; u.i = (a > b); return u.f; }
// INLINE V3 exhaustiveBest_simd(const float* p, u32 n, V3 d) {
//     __m128 vx=_mm_set1_ps(d.x), vy=_mm_set1_ps(d.y), vz=_mm_set1_ps(d.z);
//     __m128 bestVal=_mm_set1_ps(-3.402823466e38F);
//     __m128i bestIdx=_mm_setzero_si128(), idx0123=_mm_set_epi32(3,2,1,0);
//     u32 i=0;
//     for (; i+4<=n; i+=4) {
//         __m128 X=_mm_set_ps(p[(i+3)*3],p[(i+2)*3],p[(i+1)*3],p[i*3]);
//         __m128 Y=_mm_set_ps(p[(i+3)*3+1],p[(i+2)*3+1],p[(i+1)*3+1],p[i*3+1]);
//         __m128 Z=_mm_set_ps(p[(i+3)*3+2],p[(i+2)*3+2],p[(i+1)*3+2],p[i*3+2]);
//         __m128 dd=_mm_add_ps(_mm_add_ps(_mm_mul_ps(X,vx),_mm_mul_ps(Y,vy)),_mm_mul_ps(Z,vz));
//         __m128 gt=_mm_cmpgt_ps(dd,bestVal);
//         bestVal=(__m128)__builtin_ia32_maxps((__v4sf)(dd),(__v4sf)(bestVal));
//         __m128i curIdx=_mm_add_epi32(_mm_set1_epi32((int)i),idx0123);
//         bestIdx=_mm_or_si128(_mm_and_si128(_mm_castps_si128(gt),curIdx),_mm_andnot_si128(_mm_castps_si128(gt),bestIdx));
//     }
//     float v[4]; int ix[4]; _mm_storeu_ps(v,bestVal); _mm_storeu_si128((__m128i*)ix,bestIdx);
//     float top=v[0]; int bi=ix[0];
//     for (int k=1;k<4;k++) if (v[k]>top){top=v[k];bi=ix[k];}
//     for (; i<n; ++i){ float dd=p[i*3]*d.x+p[i*3+1]*d.y+p[i*3+2]*d.z; if(dd>top){top=dd;bi=(int)i;} }
//     return (V3){p[bi*3],p[bi*3+1],p[bi*3+2]};
// }
typedef int          __v8si  __attribute__((__vector_size__(32)));
typedef long long    __m256i __attribute__((__vector_size__(32)));
typedef __m256i      __m256i_u __attribute__((__may_alias__, __aligned__(1)));
#define _mm256_loadu_ps(P)         (*(__m256_u const *)(P))
#define _mm256_set1_ps(A)          ((__m256){ (A),(A),(A),(A),(A),(A),(A),(A) })
#define _mm256_setr_ps(e0,e1,e2,e3,e4,e5,e6,e7) ((__m256){ (e0),(e1),(e2),(e3),(e4),(e5),(e6),(e7) })
#define _mm256_storeu_ps(P, A)     (*(__m256_u *)(P) = (A))
#define _mm256_add_ps(A, B)        ((__m256)((__v8sf)(A) + (__v8sf)(B)))
#define _mm256_sub_ps(A, B)        ((__m256)((__v8sf)(A) - (__v8sf)(B)))
#define _mm256_mul_ps(A, B)        ((__m256)((__v8sf)(A) * (__v8sf)(B)))
typedef union { __m256 f; __m256i i; } m256_bits;
INLINE __m256  _mm256_set_ps(float e7,float e6,float e5,float e4, float e3,float e2,float e1,float e0) { return (__m256){e0,e1,e2,e3,e4,e5,e6,e7}; }
INLINE __m256i _mm256_setzero_si256(void)       { return (__m256i)(__v8si){0,0,0,0,0,0,0,0}; }
INLINE __m256i _mm256_set1_epi32(int x)         { return (__m256i)(__v8si){x,x,x,x,x,x,x,x}; }
INLINE __m256i _mm256_set_epi32(int e7,int e6,int e5,int e4, int e3,int e2,int e1,int e0) { return (__m256i)(__v8si){e0,e1,e2,e3,e4,e5,e6,e7}; }
INLINE __m256i _mm256_add_epi32(__m256i a, __m256i b)    { return a + b; }
INLINE __m256i _mm256_and_si256(__m256i a, __m256i b)    { return a & b; }
INLINE __m256i _mm256_or_si256(__m256i a, __m256i b)     { return a | b; }
INLINE __m256i _mm256_andnot_si256(__m256i a, __m256i b) { return (~a) & b; }
INLINE __m256i _mm256_castps_si256(__m256 a)             { m256_bits u; u.f = a; return u.i; }
INLINE __m256  _mm256_cmpgt_ps(__m256 a, __m256 b)       { m256_bits u; u.i = (a > b); return u.f; }
INLINE V3 exhaustiveBest_simd(const float* p, u32 n, V3 d) {
    __m256  vx = _mm256_set1_ps(d.x),vy = _mm256_set1_ps(d.y),vz = _mm256_set1_ps(d.z);
    __m256  bestVal = _mm256_set1_ps(-3.402823466e38F);
    __m256i bestIdx = _mm256_setzero_si256(),idx01234567 = _mm256_set_epi32(7,6,5,4,3,2,1,0);
    u32 i = 0;
    for (; i + 8 <= n; i += 8) {
        __m256 X = _mm256_set_ps(p[(i+7)*3],  p[(i+6)*3],  p[(i+5)*3],  p[(i+4)*3],p[(i+3)*3],  p[(i+2)*3],  p[(i+1)*3],  p[ i   *3]);
        __m256 Y = _mm256_set_ps(p[(i+7)*3+1],p[(i+6)*3+1],p[(i+5)*3+1],p[(i+4)*3+1],p[(i+3)*3+1],p[(i+2)*3+1],p[(i+1)*3+1],p[ i   *3+1]);
        __m256 Z = _mm256_set_ps(p[(i+7)*3+2],p[(i+6)*3+2],p[(i+5)*3+2],p[(i+4)*3+2],p[(i+3)*3+2],p[(i+2)*3+2],p[(i+1)*3+2],p[ i   *3+2]);
        __m256 dd = _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(X,vx),_mm256_mul_ps(Y,vy)),_mm256_mul_ps(Z,vz));
        __m256 gt = _mm256_cmpgt_ps(dd, bestVal);
        bestVal = (__m256)__builtin_ia32_maxps256((__v8sf)dd, (__v8sf)bestVal);
        __m256i curIdx = _mm256_add_epi32(_mm256_set1_epi32((int)i), idx01234567);
        bestIdx = _mm256_or_si256(_mm256_and_si256(_mm256_castps_si256(gt), curIdx),_mm256_andnot_si256(_mm256_castps_si256(gt), bestIdx));
    }
    float v[8]; int ix[8]; _mm256_storeu_ps(v, bestVal); _mm256_storeu_si256((__m256i*)ix, bestIdx);
    float top = v[0]; int bi = ix[0]; for (int k = 1; k < 8; k++) if (v[k] > top) { top = v[k]; bi = ix[k]; }
    for (; i < n; ++i) { float dd = p[i*3]*d.x + p[i*3+1]*d.y + p[i*3+2]*d.z; if (dd > top) { top = dd; bi = (int)i; } }
    return (V3){p[bi*3], p[bi*3+1], p[bi*3+2]};
}

V3 HullSupport(u16 m, const float* M, u16 adjIdx, V3 dWorld) {
    V3 dLocal = (V3){M[0]*dWorld.x + M[1]*dWorld.y + M[2]*dWorld.z,M[4]*dWorld.x + M[5]*dWorld.y + M[6]*dWorld.z,M[8]*dWorld.x + M[9]*dWorld.y + M[10]*dWorld.z};
    bool haveAdj = adjIdx<uniqueCvxMeshCount && uniqueCvxMeshIndices[adjIdx]==m && cvxAdjOffsets[adjIdx] && physPos[m] && physVertCounts[m];
    u32 n = modelVertexCounts[m];
    if (unlikely(!haveAdj || V3_dot(dLocal,dLocal)<0.000001f || n <= 64)) { return MvVert(M, exhaustiveBest_simd(physPos[m], n, dLocal)); }
    const float* p=physPos[m];
    u16 curr=cvxAdjStart[adjIdx]; if (curr>=n) curr=0;
    float currDot=p[curr*3]*dLocal.x+p[curr*3+1]*dLocal.y+p[curr*3+2]*dLocal.z;
    for (u32 steps=0; steps<n; ++steps) { u16 next=curr; float nextDot=currDot; u32 s=cvxAdjOffsets[adjIdx][curr], e=cvxAdjOffsets[adjIdx][curr+1]; for (u32 i=s;i<e;++i){ u16 nb=cvxAdjLists[adjIdx][i]; float d=p[nb*3]*dLocal.x+p[nb*3+1]*dLocal.y+p[nb*3+2]*dLocal.z; if(d>nextDot){nextDot=d;next=nb;} } if (next==curr) break; curr=next; currDot=nextDot; }
    return MvVert(M, (V3){p[curr*3],p[curr*3+1],p[curr*3+2]});
}

V3 GJKSupport(u16 prim, V3 d) { if(World.collider[prim] == COLTYPE_SPH){return SphSupport(Entity_GetSph(prim),d);} if(World.collider[prim] == COLTYPE_BOX){return BoxSupport(Entity_GetBox(prim),d);} return CapsuleSupport(Entity_GetCap(prim),d); }
INLINE void GJKSet(Simplex3D *s, int i, V3 v, V3 wA, V3 wB) { s->v[i] = v; s->wA[i] = wA; s->wB[i] = wB; }
INLINE void GJKCopy(Simplex3D *s, int dst, int src) { s->v[dst] = s->v[src]; s->wA[dst] = s->wA[src]; s->wB[dst] = s->wB[src]; }
INLINE void GJKSwap(Simplex3D *s, int i, int j) { V3 t = s->v[i]; s->v[i] = s->v[j]; s->v[j] = t; t = s->wA[i]; s->wA[i] = s->wA[j]; s->wA[j] = t; t = s->wB[i]; s->wB[i] = s->wB[j]; s->wB[j] = t; }
bool GJKNextSimplex(Simplex3D *s, V3 *dir) {
    V3 A = s->v[s->n - 1], AO = {-A.x,-A.y,-A.z}; V3 wAA = s->wA[s->n - 1], wBA = s->wB[s->n - 1];
    if (s->n == 2) {
        V3 AB = V3_AsubB(s->v[0], A);
        if (AB.x + AB.y + AB.z < PHY_EPSILON) AB = V3_AplusB(AB, V3_ScaleByF(*dir, 0.001f));
        if (V3_dot(AB,AO) > 0.f){*dir = V3_Cross(V3_Cross(AB,AO),AB);} else { s->n = 1; GJKSet(s,0,A,wAA,wBA); *dir = AO; }
        if (V3_dot(*dir,*dir) < PHY_EPSILON) { V3 px = (vabs(AB.x) > 0.9f) ? (V3){0,1,0} : (V3){1,0,0}; *dir = V3_Cross(AB,px); }
        return true;
    }
    if (s->n == 3) {
        V3 B=s->v[1], C=s->v[0], AB=V3_AsubB(B,A), AC=V3_AsubB(C,A), ABC=V3_Cross(AB,AC);
        if (V3_dot(V3_Cross(ABC,AC),AO) > 0.f) { if (V3_dot(AC,AO) > 0.f) { GJKSet(s,1,A,wAA,wBA); s->n = 2; *dir = V3_Cross(V3_Cross(AC,AO),AC); } else { goto line_AB3; } }
        else if (V3_dot(V3_Cross(AB,ABC),AO) > 0.f) { line_AB3: if (V3_dot(AB,AO) > 0.f) { GJKCopy(s,0,1); GJKSet(s,1,A,wAA,wBA); s->n = 2; *dir = V3_Cross(V3_Cross(AB,AO),AB); } else { GJKSet(s,0,A,wAA,wBA); s->n = 1; *dir = AO; } }
        else { if (V3_dot(ABC,AO) > 0.f) {*dir = ABC;} else { GJKSwap(s,0,1); *dir = (V3){-ABC.x,-ABC.y,-ABC.z}; } }
        return true;
    }
    V3 B=s->v[2], C=s->v[1], D=s->v[0], AB=V3_AsubB(B,A), AC=V3_AsubB(C,A), AD=V3_AsubB(D,A);
    V3 nABC=V3_Cross(AB,AC), nACD=V3_Cross(AC,AD), nADB=V3_Cross(AD,AB);
    nABC = V3_dot(nABC,AD) > 0.f ? (V3){-nABC.x,-nABC.y,-nABC.z} : nABC;
    nACD = V3_dot(nACD,AB) > 0.f ? (V3){-nACD.x,-nACD.y,-nACD.z} : nACD;
    nADB = V3_dot(nADB,AC) > 0.f ? (V3){-nADB.x,-nADB.y,-nADB.z} : nADB;
    if (V3_dot(nABC,AO) > 0.f) { GJKCopy(s,0,1); GJKCopy(s,1,2); GJKSet(s,2,A,wAA,wBA); s->n=3; *dir=nABC; return true; }
    if (V3_dot(nACD,AO) > 0.f) { GJKSet(s,2,A,wAA,wBA); s->n = 3; *dir=nACD; return true; }
    if (V3_dot(nADB,AO) > 0.f) { GJKCopy(s,1,0); GJKCopy(s,0,2); GJKSet(s,2,A,wAA,wBA); s->n=3; *dir=nADB; return true; }
    return false;
}

typedef struct { int a,b,c; V3 n; float d; } EPAFace; typedef struct { V3 v,wA,wB; } EPAVert;
INLINE EPAFace MakeEPAFace(const EPAVert* vb, int a, int b, int c) { V3 n = V3_Cross(V3_AsubB(vb[b].v,vb[a].v),V3_AsubB(vb[c].v,vb[a].v)); float L = V3_Mag(n); if(L < PHY_EPSILON){return (EPAFace){a,b,c,{0},-1.f};} n = V3_ScaleByF(n,1.f/L); float d = V3_dot(n,vb[a].v); if(d < 0.f){n=(V3){-n.x,-n.y,-n.z}; d=-d; int t=b;b=c;c=t;} return (EPAFace){a,b,c,n,d}; }
V3 EPAContactPoint(const EPAVert* ev, int a, int b, int c) { V3 pa=ev[a].v, pb=ev[b].v, pc=ev[c].v; V3 v0=V3_AsubB(pb,pa), v1=V3_AsubB(pc,pa), v2=V3_AsubB((V3){0,0,0},pa); float d00 = V3_dot(v0,v0), d01 = V3_dot(v0,v1), d11 = V3_dot(v1,v1), d20 = V3_dot(v2,v0), d21 = V3_dot(v2,v1); float denom = d00*d11 - d01*d01 + PHY_EPSILON; float v = vmax((d11*d20 - d01*d21)*(1.0f/denom),0.0f), w = vmax((d00*d21 - d01*d20)*(1.0f/denom),0.0f), u = vmax(1.0f - v - w,0.0f); float sum = u + v + w; if (sum > PHY_EPSILON) {u /= sum; v /= sum; w /= sum;} return (V3){u*ev[a].wA.x + v*ev[b].wA.x + w*ev[c].wA.x,u*ev[a].wA.y + v*ev[b].wA.y + w*ev[c].wA.y,u*ev[a].wA.z + v*ev[b].wA.z + w*ev[c].wA.z}; }
INLINE Manifold MakeEPAManifold(const EPAVert* ev, int a, int b, int c, V3 n, float d) { Manifold m={0}; m.normal=n; m.maxPen=d; m.n=1; m.p[0]=(ManifoldPt){EPAContactPoint(ev,a,b,c),d}; return m; }
void FeatureOverlap(V3 sc, float sr, V3 pt, Overlap* r) {
    V3 delta=V3_AsubB(sc,pt); float dist2=V3_dot(delta,delta); int hit=(dist2 < sr * sr); float dist=vsqrtf(vmax(dist2,0.0f)); float nMask=(dist > PHY_EPSILON) ? 1.0f : 0.0f; float invD = 1.0f / vmax(dist, PHY_EPSILON); V3 n=V3_AplusB(V3_ScaleByF(delta,invD * nMask), V3_ScaleByF((V3){0.0f,1.0f,0.0f},1.0f - nMask)); 
    float pen = (sr - dist) * (float)hit; int better = (pen > r->pen); r->hit = r->hit | (hit & better);  r->point = better ? pt : r->point; r->normal = better ? n : r->normal; r->pen = better ? pen : r->pen; 
}

void SphTriTest(V3 sc, float sr, u16 mesh, u32 ti, const float* mx, Overlap* r) {
    V3 a,b,c; MeshTri(mesh,ti,mx,&a,&b,&c); V3 ab=V3_AsubB(b,a), ac=V3_AsubB(c,a), ap=V3_AsubB(sc,a); float d1=V3_dot(ab,ap), d2=V3_dot(ac,ap); if(d1 <= 0.0f && d2 <= 0.0f){FeatureOverlap(sc,sr,a,r); return;}
    V3 bp=V3_AsubB(sc,b); float d3=V3_dot(ab,bp), d4=V3_dot(ac,bp); if(d3 >= 0.0f && d4 <= d3){FeatureOverlap(sc,sr,b,r); return;}
    V3 cp=V3_AsubB(sc,c); float d5=V3_dot(ab,cp), d6=V3_dot(ac,cp); if(d6>=0.f && d5<=d6){FeatureOverlap(sc,sr,c,r); return;}
    float vc=d1*d4-d3*d2; if (vc<=0.f && d1>=0.f && d3<=0.f) { float v=d1/(d1-d3); V3 pt=V3_AplusB(a,V3_ScaleByF(ab,v)); FeatureOverlap(sc,sr,pt,r); return; }
    float vb=d5*d2-d1*d6; if (vb<=0.f && d2>=0.f && d6<=0.f) { float w=d2/(d2-d6); V3 pt=V3_AplusB(a,V3_ScaleByF(ac,w)); FeatureOverlap(sc,sr,pt,r); return; }
    float va=d3*d6-d5*d4; if (va<=0.f && (d4-d3)>=0.f && (d5-d6)>=0.f) { float w=(d4-d3)/((d4-d3)+(d5-d6)); V3 bc=V3_AsubB(c,b); V3 pt=V3_AplusB(b,V3_ScaleByF(bc,w)); FeatureOverlap(sc,sr,pt,r); return; }
    V3 n = V3_Cross(ab,ac); float nLen=V3_Mag(n); if(nLen<PHY_EPSILON) return; n=V3_ScaleByF(n,1.f/nLen); float dist=V3_dot(n,ap), absDist=vabs(dist);
    if (absDist < sr) { V3 fn = /*(dist >= 0.0f) ? n : Ah nope, want it to be one-sided so that if ever small objects just barely penetrate their center past the tri it doesn't pop it through the wall/floor*/ (V3){-n.x,-n.y,-n.z}; // For some reason it always needs negated to work properly.
    Overlap t={true,V3_AsubB(sc,V3_ScaleByF(fn,absDist)),fn,sr-absDist}; if(t.pen>r->pen) *r=t; }
}

INLINE V3 TriSupport(V3 ta, V3 tb, V3 tc, V3 d) { float d1=V3_dot(ta,d),d2=V3_dot(tb,d),d3=V3_dot(tc,d); return d1>d2 ? (d1>d3 ? ta : tc) : (d2>d3 ? tb : tc); }
typedef enum { SUP_HULL_HULL, SUP_PRIM_HULL, SUP_HULL_TRI } SupportType;
typedef struct SupportCtx { V3 (*supA)(const struct SupportCtx *ctx, V3 dir); V3 (*supB)(const struct SupportCtx *ctx, V3 negDir); u16 prim,meshA,meshB; const float *matA,*matB; V3 ta,tb,tc; u16 adjA,adjB; } SupportCtx;
INLINE V3 _supA_hull(const SupportCtx *ctx, V3 d) { return HullSupport(ctx->meshA, ctx->matA, ctx->adjA, d); }
INLINE V3 _supA_sph(const SupportCtx *ctx, V3 d)  { return SphSupport(Entity_GetSph(ctx->prim), d); }
INLINE V3 _supA_box(const SupportCtx *ctx, V3 d)  { return BoxSupport(Entity_GetBox(ctx->prim), d); }
INLINE V3 _supA_cap(const SupportCtx *ctx, V3 d)  { return CapsuleSupport(Entity_GetCap(ctx->prim), d); }
INLINE V3 _supB_hull(const SupportCtx *ctx, V3 nd)  { return HullSupport(ctx->meshB, ctx->matB, ctx->adjB, nd); }
INLINE V3 _supB_hullA(const SupportCtx *ctx, V3 nd) { return HullSupport(ctx->meshA, ctx->matA, ctx->adjA, nd); }
INLINE V3 _supB_tri(const SupportCtx *ctx, V3 nd)   { return TriSupport(ctx->ta, ctx->tb, ctx->tc, nd); }
INLINE void GetSupportPair(const SupportCtx *ctx, V3 dir, V3 *wA, V3 *wB) { V3 nd = {-dir.x, -dir.y, -dir.z}; *wA = ctx->supA(ctx, dir); *wB = ctx->supB(ctx, nd); }
typedef struct { Simplex3D s; V3 dir; bool hit; } GJKResult;
GJKResult RunGJK(const SupportCtx *ctx, int maxIter) {
    GJKResult res = {0}; res.dir = (V3){0, 1, 0}; V3 wA, wB;
    GetSupportPair(ctx,res.dir,&wA,&wB);
    res.s.wA[res.s.n] = wA; res.s.wB[res.s.n] = wB; res.s.v[res.s.n++] = V3_AsubB(wA, wB);
    res.dir = (V3){-res.s.v[0].x, -res.s.v[0].y, -res.s.v[0].z};
    if (V3_dot(res.dir, res.dir) < PHY_EPSILON) res.dir = (V3){0, 1, 0};
    for (int it = 0; it < maxIter; ++it) { GetSupportPair(ctx, res.dir, &wA, &wB); V3 sup = V3_AsubB(wA, wB); if (V3_dot(sup, res.dir) < 0) {break;} res.s.wA[res.s.n] = wA; res.s.wB[res.s.n] = wB; res.s.v[res.s.n++] = sup; if (!GJKNextSimplex(&res.s, &res.dir)) { res.hit = true; break; } }
    return res;
}

INLINE void RunGJKFallback(const SupportCtx *ctx, Simplex3D *s) { static const V3 kAx[6] = {{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}}; for(int d=0;s->n<4 && d<6;++d){V3 wA,wB; GetSupportPair(ctx,kAx[d],&wA,&wB); V3 sup=V3_AsubB(wA,wB); bool dup=false; for (int k=0;k<s->n;++k){V3 dv=V3_AsubB(sup,s->v[k]); dup |= (V3_dot(dv,dv) < PHY_EPSILON * PHY_EPSILON); } if(!dup){s->wA[s->n]=wA; s->wB[s->n]=wB; s->v[s->n++]=sup;}} }
typedef struct { EPAVert ev[EPA_MAX_VERTS]; EPAFace ef[EPA_MAX_FACES]; int nv, nf; } EPAState;
void SeedEPA(EPAState *epa, const Simplex3D *s) {
    static const int kTetFaces[4][3] = {{0,1,2},{0,3,1},{0,2,3},{1,3,2}}; epa->nv = 0; epa->nf = 0;
    for (int i = 0; i < 4; i++) { epa->ev[epa->nv].wA = s->wA[i]; epa->ev[epa->nv].wB = s->wB[i]; epa->ev[epa->nv].v = s->v[i]; epa->nv++; }
    for (int f = 0; f < 4; f++) { EPAFace face = MakeEPAFace(epa->ev,kTetFaces[f][0],kTetFaces[f][1],kTetFaces[f][2]); if(face.d >= 0.f && epa->nf < EPA_MAX_FACES){epa->ef[epa->nf++]=face;} }
}

bool ExpandEPA(EPAState *epa, V3 sup, V3 wA, V3 wB) {
    if(epa->nv >= EPA_MAX_VERTS){return false;}
    epa->ev[epa->nv].v=sup; epa->ev[epa->nv].wA=wA; epa->ev[epa->nv].wB=wB;
    int edges[EPA_MAX_EDGES][2], ne = 0, keep[EPA_MAX_FACES], nk = 0;
    for (int f = 0; f < epa->nf; f++) {
        if (V3_dot(epa->ef[f].n, V3_AsubB(sup, epa->ev[epa->ef[f].a].v)) > 0.f) {
            int fv[3] = {epa->ef[f].a, epa->ef[f].b, epa->ef[f].c};
            for (int e = 0; e < 3; e++) {
                int ea = fv[e], eb = fv[(e + 1) % 3]; bool found = false;
                for (int k = 0; k < ne; k++) if (edges[k][0] == eb && edges[k][1] == ea) { edges[k][0] = edges[--ne][0]; edges[k][1] = edges[ne][1]; found = true; break; }
                if (!found && ne < EPA_MAX_EDGES) { edges[ne][0] = ea; edges[ne++][1] = eb; }
            }
        } else keep[nk++] = f;
    }
    epa->nf = 0; for (int k = 0; k < nk; k++) epa->ef[epa->nf++] = epa->ef[keep[k]];
    for (int k = 0; k < ne && epa->nf < EPA_MAX_FACES; k++) { EPAFace face = MakeEPAFace(epa->ev, edges[k][0], edges[k][1], epa->nv); if (face.d >= 0.f) epa->ef[epa->nf++] = face; }
    epa->nv++; return true;
}

INLINE bool BvhSphereAABBOverlap(V3 sc, float sr, V3 mn, V3 mx) { V3 cl = {vclamp(sc.x, mn.x, mx.x), vclamp(sc.y, mn.y, mx.y), vclamp(sc.z, mn.z, mx.z)}; V3 d = V3_AsubB(sc, cl); return V3_dot(d, d) <= sr * sr; }
INLINE void BvhNodeWorldAABB(const BvhNode* node, const float* mx, V3* wMn, V3* wMx) {
    __m128 col0=_mm_loadu_ps(mx + 0); __m128 col1=_mm_loadu_ps(mx + 4); __m128 col2=_mm_loadu_ps(mx + 8); __m128 tr=_mm_loadu_ps(mx + 12);  __m128 mn_v=_mm_setr_ps(node->mn.x,node->mn.y,node->mn.z,0.0f); __m128 mx_v=_mm_setr_ps(node->mx.x,node->mx.y,node->mx.z,0.0f);
    __m128 lc = _mm_mul_ps(_mm_add_ps(mn_v,mx_v),_mm_set1_ps(0.5f)); // lc = (mn + mx) * 0.5f   
    __m128 lh = _mm_mul_ps(((__m128)((__v4sf)(mx_v) - (__v4sf)(mn_v))),_mm_set1_ps(0.5f)); // lh = (mx - mn) * 0.5f
    __m128 lc_x = __builtin_shufflevector(lc,lc,0,0,0,0); __m128 lc_y = __builtin_shufflevector(lc,lc,1,1,1,1); __m128 lc_z = __builtin_shufflevector(lc,lc,2,2,2,2); // Replicate lc.x, lc.y, lc.z across vectors
    __m128 wc = _mm_add_ps(_mm_add_ps(_mm_mul_ps(col0,lc_x), _mm_mul_ps(col1,lc_y)), _mm_add_ps(_mm_mul_ps(col2,lc_z),tr)); // wc = col0 * lc_x + col1 * lc_y + col2 * lc_z + tr
    __v4si sign_mask = (__v4si)_mm_set1_ps(-0.0f); __v4si inv_mask = ~sign_mask;
    __m128 abs_col0 = (__m128)((__v4si)col0 & inv_mask); __m128 abs_col1 = (__m128)((__v4si)col1 & inv_mask); __m128 abs_col2 = (__m128)((__v4si)col2 & inv_mask); // Take absolute value of matrix columns using bitwise AND
    __m128 lh_x = __builtin_shufflevector(lh,lh,0,0,0,0); __m128 lh_y = __builtin_shufflevector(lh,lh,1,1,1,1); __m128 lh_z = __builtin_shufflevector(lh,lh,2,2,2,2); // Replicate lh components
    __m128 wh = _mm_add_ps(_mm_add_ps(_mm_mul_ps(abs_col0,lh_x),_mm_mul_ps(abs_col1,lh_y)),_mm_mul_ps(abs_col2,lh_z)); // wh = abs_col0 * lh_x + abs_col1 * lh_y + abs_col2 * lh_z
    __m128 wMn_v=((__m128)((__v4sf)(wc) - (__v4sf)(wh))); __m128 wMx_v=_mm_add_ps(wc,wh); // wMn = wc - wh, wMx = wc + wh
    wMn->x = wMn_v[0]; wMn->y = wMn_v[1]; wMn->z = wMn_v[2]; wMx->x = wMx_v[0]; wMx->y = wMx_v[1]; wMx->z = wMx_v[2]; // Store back to V3 (avoids overwriting adjacent struct memory)
}

void BvhWalkSphMsh(V3 sc, float sr, u16 m, const float* mx, Overlap* r) {
    const BvhNode* nodes=modelBVHNodes[m]; const u16* triOrder = modelBVHTriOrder[m]; const BvhNode* stack[64]; int sp = 0; stack[sp++] = &nodes[0];
    while (sp > 0) {
        const BvhNode* node = stack[--sp]; V3 wMn,wMx; BvhNodeWorldAABB(node,mx,&wMn,&wMx); if (!BvhSphereAABBOverlap(sc,sr,wMn,wMx)) continue;
        if (node->triCount > 0) { for (u32 i = 0; i < node->triCount; i++) SphTriTest(sc, sr, m, triOrder[node->triStart + i], mx, r); } else { for (int o = 0; o < 8; o++) if (node->children[o] >= 0) stack[sp++] = &nodes[node->children[o]]; }
    }
}

Overlap SphMsh(V3 sc, float sr, u16 m, const float* mx) { Overlap r={0}; if(m>=MAX_MDLS)return r; u32 tc=modelTriangleCounts[m]; if(!tc){return r;} if (BvhHasBVH(m)) { BvhWalkSphMsh(sc,sr,m,mx,&r); return r; } for(u32 ti=0;ti<tc;++ti){SphTriTest(sc,sr,m,ti,mx,&r);} return r; }
Overlap CapMsh(ShapeCapsule c, u16 m, const float* mx) { Overlap best=SphMsh(c.base,c.rad,m,mx), rt=SphMsh(c.tip,c.rad,m,mx); if(rt.pen>best.pen)best=rt; V3 d=V3_AsubB(c.tip,c.base); if(V3_Mag(d)>PHY_EPSILON){/*Hey I was doing a snowman of just the end spheres, don't hate the simplicity, I only use capsules for npcs and player*/for(int k=1;k<6;++k){float t=(float)k/5.0f; Overlap rm=SphMsh(V3_AplusB(c.base,V3_ScaleByF(d,t)),c.rad,m,mx); if(rm.pen>best.pen)best=rm;}} return best; }
Manifold PrimitiveCvx(u16 prim, u16 mesh, const float* mx, u16 adjIdx) {
    Manifold m={0}; if(mesh>=MAX_MDLS||adjIdx>=MAX_MDLS||!modelVertexCounts[mesh])return m;
    u8 col = World.collider[prim]; V3 (*supA)(const SupportCtx*, V3) = (col == COLTYPE_SPH) ? _supA_sph : (col == COLTYPE_BOX) ? _supA_box : _supA_cap;
    SupportCtx ctx = (SupportCtx){supA, _supB_hullA, .prim=prim, .meshA=mesh, .matA=mx, .adjA=adjIdx, .adjB=adjIdx};
    GJKResult gjk = RunGJK(&ctx,GJK_ITER); if(!gjk.hit)return m;
    if(gjk.s.n<4) RunGJKFallback(&ctx,&gjk.s); if(gjk.s.n<4)return m;
    EPAState epa; SeedEPA(&epa,&gjk.s);
    for(int it=0;it<EPA_ITER;++it){
        int bf=-1; float bd=1e9f; for(int f=0;f<epa.nf;f++)if(epa.ef[f].d<bd){bd=epa.ef[f].d;bf=f;} if(bf<0)break;
        V3 bn=epa.ef[bf].n; V3 wA, wB; GetSupportPair(&ctx,bn,&wA,&wB); V3 sup=V3_AsubB(wA,wB); 
        if(V3_dot(bn,sup)-bd<PHY_EPSILON){return MakeEPAManifold(epa.ev,epa.ef[bf].a,epa.ef[bf].b,epa.ef[bf].c,bn,bd);}
        if (!ExpandEPA(&epa,sup,wA,wB)) break;
    }
    return m;
}

typedef struct {V3 mn,mx;} AABB3; typedef struct { u16 hullMesh; const float* hullMx; const V3* boxV; u32 boxN; AABB3 hb; V3 hullCenter; float hullRadius,spreadEps,thicknessTolerance; Manifold best; u16 adjHull; } CvxMshCtx;
void CvxTriTest(CvxMshCtx* ctx, V3 ta, V3 tb, V3 tc) {
    u16 hullMesh=ctx->hullMesh; Manifold* best=&ctx->best; float spreadEps=ctx->spreadEps, thicknessTolerance=ctx->thicknessTolerance;
    if (vmin(ta.x,vmin(tb.x,tc.x))>ctx->hb.mx.x || vmax(ta.x,vmax(tb.x,tc.x))<ctx->hb.mn.x || vmin(ta.y,vmin(tb.y,tc.y))>ctx->hb.mx.y || vmax(ta.y,vmax(tb.y,tc.y))<ctx->hb.mn.y || vmin(ta.z,vmin(tb.z,tc.z))>ctx->hb.mx.z || vmax(ta.z,vmax(tb.z,tc.z))<ctx->hb.mn.z) return;
    V3 triEdge1=V3_AsubB(tb,ta), triEdge2=V3_AsubB(tc,ta); V3 triN=V3_Cross(triEdge1,triEdge2); float triLenSq=V3_dot(triN,triN); if (triLenSq < PHY_EPSILON) return;
    triN = V3_ScaleByF(triN, 1.0f / vsqrtf(triLenSq));
    if (best->n >= MANIFOLD_MAX && (ctx->hullRadius - vabs(V3_dot(triN,V3_AsubB(ctx->hullCenter, ta)))) <= best->maxPen + MANIFOLD_TIE_MARGIN) return; // Fast early-out: If the manifold is full, only process triangles that can be deeper
    SupportCtx supCtx = (SupportCtx){_supA_hull, _supB_tri, .meshA=hullMesh, .matA=ctx->hullMx, .adjA=ctx->adjHull, .adjB=ctx->adjHull, .ta=ta, .tb=tb, .tc=tc};
    GJKResult gjk = RunGJK(&supCtx,GJK_ITER); if(!gjk.hit)return;
    Simplex3D *s = &gjk.s;
    while (s->n<4) {
        V3 fallbackDir={0.0f,1.0f,0.0f};
        if(s->n==1) fallbackDir=(vabs(s->v[0].x)>0.5f)?(V3){0.0f,1.0f,0.0f}:(V3){1.0f,0.0f,0.0f};
        else if(s->n==2){V3 edge=V3_AsubB(s->v[1],s->v[0]); fallbackDir=V3_Cross(edge,(vabs(edge.x)>0.5f)?(V3){0.0f,1.0f,0.0f}:(V3){1.0f,0.0f,0.0f});}
        else if(s->n==3){V3 e1=V3_AsubB(s->v[1],s->v[0]), e2=V3_AsubB(s->v[2],s->v[0]); fallbackDir=V3_Cross(e1,e2);}
        float fLen=V3_Mag(fallbackDir); fallbackDir=(fLen>PHY_EPSILON)?V3_ScaleByF(fallbackDir,1.0f/fLen):(V3){0.0f,1.0f,0.0f};
        V3 wA, wB; GetSupportPair(&supCtx, fallbackDir, &wA, &wB); V3 sup=V3_AsubB(wA,wB); bool dup=false;
        for (int k=0;k<s->n;k++){V3 dv=V3_AsubB(sup,s->v[k]); dup|=(V3_dot(dv,dv)<PHY_EPSILON*PHY_EPSILON);}
        if (!dup){s->wA[s->n]=wA; s->wB[s->n]=wB; s->v[s->n++]=sup;} else {fallbackDir=(V3){-fallbackDir.x,-fallbackDir.y,-fallbackDir.z}; GetSupportPair(&supCtx, fallbackDir, &wA, &wB); s->wA[s->n]=wA; s->wB[s->n]=wB; s->v[s->n++]=V3_AsubB(wA,wB);}
    }
    EPAState epa; SeedEPA(&epa, s); if (epa.nf<4) return;
    bool tHit=false; V3 tN={0}; float tD=0; V3 tP={0};
    for (int it=0;it<EPA_ITER;++it){
        int bf=-1; float bd=1e9f; for (int f=0;f<epa.nf;f++)if(epa.ef[f].d<bd){bd=epa.ef[f].d;bf=f;} if(bf<0)break;
        V3 bn=epa.ef[bf].n; V3 wA, wB; GetSupportPair(&supCtx, bn, &wA, &wB); V3 sup=V3_AsubB(wA,wB);
        if(V3_dot(bn,sup)-bd<PHY_EPSILON){ if(V3_dot(bn,triN) < 0.0f){bn=triN;} tHit=true; tN=bn; tD=bd; tP=EPAContactPoint(epa.ev,epa.ef[bf].a,epa.ef[bf].b,epa.ef[bf].c); break; }
        if (!ExpandEPA(&epa,sup,wA,wB)) break;
    }
    if (!tHit) return;
    V3 deepPoint=tP;
    if (!best->n) { best->normal=tN; best->maxPen=tD; best->p[best->n++]=(ManifoldPt){deepPoint,tD}; }
    else {
        float align=V3_dot(tN,best->normal);
        if (align>MANIFOLD_ALIGN_THRESHOLD) {
            bool better=(tD>best->maxPen+MANIFOLD_TIE_MARGIN) || (vabs(tD-best->maxPen)<=MANIFOLD_TIE_MARGIN && V3_dot(tN,(V3){0,1,0})>V3_dot(best->normal,(V3){0,1,0})); if (better){best->normal=tN; best->maxPen=tD;} bool spread=true;
            for (int k=0;k<best->n;++k){V3 dv=V3_AsubB(deepPoint,best->p[k].point); if(V3_dot(dv,dv)<spreadEps*spreadEps){spread=false; if(tD>best->p[k].pen)best->p[k].pen=tD; break;}}
            if (spread&&best->n<MANIFOLD_MAX)best->p[best->n++]=(ManifoldPt){deepPoint,tD};
        } else if (tD>best->maxPen+MANIFOLD_TIE_MARGIN){best->n=0; best->normal=tN; best->maxPen=tD; best->p[best->n++]=(ManifoldPt){deepPoint,tD};}
    }
    if (best->n < MANIFOLD_MAX) {
        u32 hn = ctx->boxV ? ctx->boxN : modelVertexCounts[hullMesh];
        if (hn && best->n>0 && V3_dot(tN,best->normal)>MANIFOLD_ALIGN_THRESHOLD) {
            float planeDist=V3_dot(tN,deepPoint); 
            // Reuse triEdge1 and triEdge2
            float d00=V3_dot(triEdge1,triEdge1), d01=V3_dot(triEdge1,triEdge2), d11=V3_dot(triEdge2,triEdge2), denom=d00*d11-d01*d01; 
            bool validTri=vabs(denom)>PHY_EPSILON;
            for (u32 i=0;i<hn;++i) {
                V3 pt=ctx->boxV ? ctx->boxV[i] : MvVert(ctx->hullMx,MeshVert(hullMesh,i)); float distToPlane=V3_dot(tN,pt)-planeDist;
                if (vabs(distToPlane)<thicknessTolerance) {
                    bool insideTri=false;
                    if (validTri){V3 projPt=V3_AsubB(pt,V3_ScaleByF(tN,distToPlane)), v2=V3_AsubB(projPt,ta); float d20=V3_dot(v2,triEdge1), d21=V3_dot(v2,triEdge2), v=(d11*d20-d01*d21)/denom, w=(d00*d21-d01*d20)/denom, u=1.0f-v-w; if(u>=-0.02f&&v>=-0.02f&&w>=-0.02f)insideTri=true;}
                    if (insideTri){float ptPen=tD-distToPlane; if(ptPen>0.0f){bool isDup=false; for(int k=0;k<best->n;++k){V3 diff=V3_AsubB(pt,best->p[k].point); if(V3_dot(diff,diff)<spreadEps*spreadEps){isDup=true;break;}} if(!isDup&&best->n<MANIFOLD_MAX)best->p[best->n++]=(ManifoldPt){pt,ptPen}; if(best->n>=MANIFOLD_MAX)break;}}
                }
            }
        }
    }
}

INLINE bool BvhAABBOverlap(V3 aMn, V3 aMx, V3 bMn, V3 bMx) { return (aMx.x >= bMn.x && aMn.x <= bMx.x && aMx.y >= bMn.y && aMn.y <= bMx.y && aMx.z >= bMn.z && aMn.z <= bMx.z); }
void BvhWalkAABB_CvxTri(u16 triMesh, const float* triMx, AABB3 hb, CvxMshCtx* ctx) {
    const BvhNode* nodes = modelBVHNodes[triMesh]; const u16* triOrder = modelBVHTriOrder[triMesh]; const BvhNode* stack[64]; int sp = 0; stack[sp++] = &nodes[0];
    while (sp > 0) {
        const BvhNode* node = stack[--sp]; V3 wMn, wMx; BvhNodeWorldAABB(node, triMx, &wMn, &wMx); if (!BvhAABBOverlap(wMn, wMx, hb.mn, hb.mx)) continue;
        if (node->triCount > 0) { for (u32 i = 0; i < node->triCount; i++) { V3 ta, tb, tc; MeshTri(triMesh, triOrder[node->triStart + i], triMx, &ta, &tb, &tc); CvxTriTest(ctx, ta, tb, tc); } }
        else { for (int o = 0; o < 8; o++) if (node->children[o] >= 0) stack[sp++] = &nodes[node->children[o]]; }
    }
}

Manifold CvxMsh(u16 hullMesh, const float* hullMx, u16 triMesh, const float* triMx, u16 adjHull) {
    Manifold z={0}; if(hullMesh>=MAX_MDLS||adjHull>=MAX_MDLS||triMesh>=MAX_MDLS)return z;
    u32 hn=modelVertexCounts[hullMesh]; if(!hn)return z;
    CvxMshCtx ctx={0}; ctx.hullMesh=hullMesh; ctx.hullMx=hullMx; ctx.adjHull=adjHull; AABB3 hb={{1e9f,1e9f,1e9f},{-1e9f,-1e9f,-1e9f}};
    for (u32 i=0;i<hn;++i) { V3 w=MvVert(hullMx,MeshVert(hullMesh,i)); hb.mn.x=vmin(hb.mn.x,w.x); hb.mn.y=vmin(hb.mn.y,w.y); hb.mn.z=vmin(hb.mn.z,w.z); hb.mx.x=vmax(hb.mx.x,w.x); hb.mx.y=vmax(hb.mx.y,w.y); hb.mx.z=vmax(hb.mx.z,w.z); }
    ctx.hb=hb; ctx.hullCenter = V3_ScaleByF(V3_AplusB(hb.mn, hb.mx), 0.5f); ctx.hullRadius = V3_Mag(V3_AsubB(hb.mx, hb.mn)) * 0.5f;
    V3 hext=V3_AsubB(hb.mx,hb.mn); ctx.spreadEps=vmax(0.02f,vmax(hext.x,vmax(hext.y,hext.z))*0.15f);
    float wscaleH=V3_Mag((V3){hullMx[0],hullMx[1],hullMx[2]}); ctx.thicknessTolerance=vclamp(modelBounds[hullMesh]*wscaleH*0.06f,0.003f,0.02f);
    u32 triCount=modelTriangleCounts[triMesh]; if(!triCount)return ctx.best;
    if (BvhHasBVH(triMesh)) { BvhWalkAABB_CvxTri(triMesh, triMx, hb, &ctx); return ctx.best; }
    for (u32 ti=0;ti<triCount;++ti) { V3 ta,tb,tc; MeshTri(triMesh,ti,triMx,&ta,&tb,&tc); CvxTriTest(&ctx,ta,tb,tc); }
    return ctx.best;
}

Manifold CvxCvx(u16 meshA, u16 meshB, const float* matA, const float* matB, u16 adjA, u16 adjB) {
    Manifold m={0}; if(meshA>=MAX_MDLS||adjA>=MAX_MDLS||meshB>=MAX_MDLS||adjB>=MAX_MDLS)return m;
    SupportCtx ctx = (SupportCtx){_supA_hull, _supB_hull, .meshA=meshA, .meshB=meshB, .matA=matA, .matB=matB, .adjA=adjA, .adjB=adjB};
    GJKResult gjk = RunGJK(&ctx,GJK_ITER); if(!gjk.hit)return m;
    if(gjk.s.n<4) RunGJKFallback(&ctx,&gjk.s); if(gjk.s.n<4)return m;
    EPAState epa; SeedEPA(&epa, &gjk.s);
    for(int it=0;it<EPA_ITER;++it) {
        int bf=-1; float bd=1e9f; for(int f=0;f<epa.nf;f++)if(epa.ef[f].d<bd){bd=epa.ef[f].d;bf=f;} if(bf<0)break;
        V3 bn=epa.ef[bf].n; 
        V3 wA, wB; GetSupportPair(&ctx, bn, &wA, &wB);
        V3 sup=V3_AsubB(wA,wB);
        if (V3_dot(bn,sup)-bd<PHY_EPSILON) {
            m.normal=bn; m.maxPen=bd; m.n=1; V3 deepPoint=EPAContactPoint(epa.ev,epa.ef[bf].a,epa.ef[bf].b,epa.ef[bf].c); m.p[0]=(ManifoldPt){deepPoint,bd};
            u32 nVertsB = modelVertexCounts[meshB];
            if (nVertsB > 0) {
                float planeDist=V3_dot(bn,deepPoint),wscaleB=V3_Mag((V3){matB[0],matB[1],matB[2]}),thicknessTolerance=vclamp(modelBounds[meshB]*wscaleB*0.06f,0.003f,0.02f);
                const u8* vb = (u8*)physPos[meshB];
                for(u32 i=0;i<nVertsB;++i) {
                    const u8* p = vb + i * 12; V3 ptLocal = *(V3*)p; V3 pt = MvVert(matB,ptLocal); float distToPlane=V3_dot(bn,pt)-planeDist;
                    if (vabs(distToPlane)<thicknessTolerance) { 
                        float ptPen=bd-distToPlane; 
                        if(ptPen>0.0f) { bool isDup=false; for(int k=0;k<m.n;++k){V3 diff=V3_AsubB(pt,m.p[k].point); if(V3_dot(diff,diff)<0.00001f){isDup=true; break;}} if(!isDup&&m.n<MANIFOLD_MAX){m.p[m.n++]=(ManifoldPt){pt,ptPen};} if(m.n>=MANIFOLD_MAX){break;} }
                    }
                }
            }
            return m;
        }
        if (!ExpandEPA(&epa,sup,wA,wB)) break;
    }
    return m;
}

Manifold BoxMsh(ShapeBox b,u16 m,const float* mx){
    CvxMshCtx c={0}; c.adjHull=U16_MAX; if(m>=MAX_MDLS||!modelTriangleCounts[m])return c.best; V3 bv[8]; c.hullMesh=m; c.hullMx=mx; c.boxV=bv; c.boxN=8; V3 a[3]; float h[3]={b.hExt.x,b.hExt.y,b.hExt.z}; a[0]=quat_rot_v3(b.rot,(V3){1,0,0}); a[1]=quat_rot_v3(b.rot,(V3){0,1,0}); a[2]=quat_rot_v3(b.rot,(V3){0,0,1}); AABB3 q={{1e9f,1e9f,1e9f},{-1e9f,-1e9f,-1e9f}};
    for(int i=0;i<8;i++){V3 p=b.ctr;for(int k=0;k<3;k++)p=V3_AplusB(p,V3_ScaleByF(a[k],((i>>k)&1?1.f:-1.f)*h[k])); bv[i]=p; q.mn.x=vmin(q.mn.x,p.x); q.mn.y=vmin(q.mn.y,p.y); q.mn.z=vmin(q.mn.z,p.z); q.mx.x=vmax(q.mx.x,p.x); q.mx.y=vmax(q.mx.y,p.y); q.mx.z=vmax(q.mx.z,p.z);}
    V3 e=V3_AsubB(q.mx,q.mn),skin={.01f,.01f,.01f}; c.spreadEps=vmax(.02f,vmax(e.x,vmax(e.y,e.z))*.15f); c.thicknessTolerance=vclamp(V3_Mag(b.hExt)*.06f,.003f,.02f); c.hb=(AABB3){V3_AsubB(q.mn,skin),V3_AplusB(q.mx,skin)};
    c.hullCenter = b.ctr;
    c.hullRadius = V3_Mag(b.hExt);
    if (BvhHasBVH(m)) { BvhWalkAABB_CvxTri(m, mx, c.hb, &c); c.best.normal=V3_ScaleByF(c.best.normal,-1.f); return c.best; }
    for(u32 ti=0;ti<modelTriangleCounts[m];ti++){V3 x,y,z;MeshTri(m,ti,mx,&x,&y,&z);CvxTriTest(&c,x,y,z);} c.best.normal=V3_ScaleByF(c.best.normal,-1.f); return c.best;
}

INLINE void quat_to_mat3(Quaternion q, float R[3][3]) { float x=q.x,y=q.y,z=q.z,w=q.w, xx=x*x,yy=y*y,zz=z*z, xy=x*y,xz=x*z,yz=y*z, wx=w*x,wy=w*y,wz=w*z; R[0][0]=1.0f-2.0f*(yy+zz); R[0][1]=2.0f*(xy-wz); R[0][2]=2.0f*(xz+wy); R[1][0]=2.0f*(xy+wz); R[1][1]=1.0f-2.0f*(xx+zz); R[1][2]=2.0f*(yz-wx); R[2][0]=2.0f*(xz-wy); R[2][1]=2.0f*(yz+wx); R[2][2]=1.0f-2.0f*(xx+yy); }
V3 ApplyInvTensor(u16 i, V3 v, const float R[3][3]) {
    if (World.collider[i] == COLTYPE_BOX) {
        ShapeBox b = Entity_GetBox(i); float m = World.mass[i], hx = b.hExt.x, hy = b.hExt.y, hz = b.hExt.z;
        float Ixx=(1.0f/3.0f)*m*(hy*hy+hz*hz), Iyy=(1.0f/3.0f)*m*(hx*hx+hz*hz), Izz=(1.0f/3.0f)*m*(hx*hx+hy*hy), invIxx=1.0f/vmax(Ixx,1e-6f), invIyy=1.0f/vmax(Iyy,1e-6f), invIzz=1.0f/vmax(Izz,1e-6f);
        float bx=R[0][0]*v.x+R[1][0]*v.y+R[2][0]*v.z, by=R[0][1]*v.x+R[1][1]*v.y+R[2][1]*v.z, bz=R[0][2]*v.x+R[1][2]*v.y+R[2][2]*v.z; float wx=invIxx*bx, wy=invIyy*by, wz=invIzz*bz; 
        return (V3){R[0][0]*wx+R[0][1]*wy+R[0][2]*wz, R[1][0]*wx+R[1][1]*wy+R[1][2]*wz, R[2][0]*wx+R[2][1]*wy+R[2][2]*wz};
    }
    if (World.collider[i] != COLTYPE_CVX || !World.invTnsrValid[i]) { float r=World.radius[i]; return V3_ScaleByF(v,1.0f/vmax((2.0f/5.0f)*World.mass[i]*r*r,0.0f)); }
    float *I=World.invInertiaTensor[i];
    float bx=R[0][0]*v.x+R[1][0]*v.y+R[2][0]*v.z, by=R[0][1]*v.x+R[1][1]*v.y+R[2][1]*v.z, bz=R[0][2]*v.x+R[1][2]*v.y+R[2][2]*v.z;
    float wx=I[0]*bx+I[3]*by+I[4]*bz, wy=I[3]*bx+I[1]*by+I[5]*bz, wz=I[4]*bx+I[5]*by+I[2]*bz; 
    return (V3){R[0][0]*wx+R[0][1]*wy+R[0][2]*wz, R[1][0]*wx+R[1][1]*wy+R[1][2]*wz, R[2][0]*wx+R[2][1]*wy+R[2][2]*wz};
}

void ResolveContactVelocity(u16 a, u16 b, V3 n, V3 rAarm, V3 rBarm, float targetVn, float *accumN, float* accumT, bool bStatic, float invMassA, float invMassB, float invSumN, bool canRotateA, bool canRotateB, const float Ra[3][3], const float Rb[3][3]) {
    if (invSumN < PHY_EPSILON) return;
    V3 vAtA = V3_AplusB(World.velocity[a],V3_Cross(World.angularVelocity[a],rAarm)), vAtB = bStatic ? (V3){0,0,0} : V3_AplusB(World.velocity[b],V3_Cross(World.angularVelocity[b],rBarm));
    float vn = V3_dot(V3_AsubB(vAtA,vAtB),n), j = (targetVn - vn) / invSumN, newAccumN = vmax(*accumN + j, 0.0f); j = newAccumN - *accumN; *accumN = newAccumN;
    V3 impulse = V3_ScaleByF(n,j); World.velocity[a] = V3_AplusB(World.velocity[a],V3_ScaleByF(impulse,invMassA));
    if (!bStatic) World.velocity[b] = V3_AsubB(World.velocity[b],V3_ScaleByF(impulse,invMassB));
    if (canRotateA) World.angularVelocity[a] = V3_AplusB(World.angularVelocity[a],ApplyInvTensor(a,V3_Cross(rAarm,impulse),Ra));
    if (canRotateB) World.angularVelocity[b] = V3_AsubB(World.angularVelocity[b],ApplyInvTensor(b,V3_Cross(rBarm,impulse),Rb));
    V3 vAtA2 = V3_AplusB(World.velocity[a],V3_Cross(World.angularVelocity[a],rAarm)), vAtB2 = bStatic ? (V3){0,0,0} : V3_AplusB(World.velocity[b],V3_Cross(World.angularVelocity[b],rBarm));
    V3 relVel2 = V3_AsubB(vAtA2,vAtB2), tangent = V3_AsubB(relVel2,V3_ScaleByF(n,V3_dot(relVel2,n))); float tLen = V3_Mag(tangent);
    if (tLen > 0.0001f) {
        tangent = V3_ScaleByF(tangent,1.0f/tLen); V3 rAxT = V3_Cross(rAarm,tangent), rBxT = V3_Cross(rBarm,tangent);
        float angTermAT = canRotateA ? V3_dot(rAxT,ApplyInvTensor(a,rAxT,Ra)) : 0.0f, angTermBT = canRotateB ? V3_dot(rBxT,ApplyInvTensor(b,rBxT,Rb)) : 0.0f, invSumT = invMassA + invMassB + angTermAT + angTermBT;
        if (invSumT > PHY_EPSILON) {
            float jt = -V3_dot(relVel2,tangent) / invSumT, friction; bool aIsSpecial = (World.collider[a] == COLTYPE_CAP && (a == PLAYER1 || IdxIsNPC(World.instances[a].index)));
            if (bStatic && aIsSpecial) { friction = 0.001f; } else { float mix = vclamp((tLen - 0.05f) / 0.10f, 0.0f, 1.0f); friction = 0.05f + mix * (1.0f - 0.05f); }
            float maxT = friction * (*accumN), newAccumT = vclamp(*accumT + jt, -maxT, maxT); jt = newAccumT - *accumT; *accumT = newAccumT;
            V3 fImpulse = V3_ScaleByF(tangent,jt); World.velocity[a] = V3_AplusB(World.velocity[a],V3_ScaleByF(fImpulse,invMassA));
            if (!bStatic) World.velocity[b] = V3_AsubB(World.velocity[b],V3_ScaleByF(fImpulse,invMassB));
            if (canRotateA) World.angularVelocity[a] = V3_AplusB(World.angularVelocity[a],ApplyInvTensor(a,V3_Cross(rAarm,fImpulse),Ra));
            if (canRotateB) World.angularVelocity[b] = V3_AsubB(World.angularVelocity[b],ApplyInvTensor(b,V3_Cross(rBarm,fImpulse),Rb));
        }
    }
}

void DrawSphereContact(V3 pos, float rad);
void ApplyManifoldResponse(u16 a, u16 b, const Manifold *m) {
    if (!m->n || (World.collider[b] == COLTYPE_MSH && World.collider[a] == COLTYPE_MSH)) return;
    bool bStatic = (!(World.instances[b].entflags & EF_RIGIDBODY) || World.mass[b] < 0.001f || World.collider[b] == COLTYPE_NONE || World.collider[b] == COLTYPE_MSH);
    for (int i=0;i<m->n;++i) { if(m->p[i].pen > 0.0f){DrawSphereContact(m->p[i].point,0.02f);} }
    float Ra[3][3], Rb[3][3]; quat_to_mat3(World.rotation[a],Ra);
    if (!bStatic) quat_to_mat3(World.rotation[b],Rb);
    V3 rA[MANIFOLD_MAX], rB[MANIFOLD_MAX]; float targetVn[MANIFOLD_MAX], accumN[MANIFOLD_MAX]={0}, accumT[MANIFOLD_MAX]={0}, invSumN[MANIFOLD_MAX];
    float invMassA = World.mass[a] < 0.001f ? 1.0f : 1.0f / World.mass[a], invMassB = (bStatic || World.mass[b] < 0.001f) ? 0.0f : 1.0f / World.mass[b];
    bool canRotateA = (World.collider[a] != COLTYPE_CAP && !IdxIsNPC(World.instances[a].index)), canRotateB = (!bStatic && World.collider[b] != COLTYPE_CAP && !IdxIsNPC(World.instances[b].index));
    for (int i=0;i<m->n;++i) {
        rA[i] = V3_AsubB(m->p[i].point,World.position[a]); rB[i] = bStatic ? (V3){0,0,0} : V3_AsubB(m->p[i].point,World.position[b]);
        V3 vAtA = V3_AplusB(World.velocity[a],V3_Cross(World.angularVelocity[a],rA[i])), vAtB = bStatic ? (V3){0,0,0} : V3_AplusB(World.velocity[b],V3_Cross(World.angularVelocity[b],rB[i]));
        float vn0 = V3_dot(V3_AsubB(vAtA,vAtB),m->normal), e_r = (vn0 < -0.5f) ? vmax(World.bounciness[a],bStatic ? 0.0f : World.bounciness[b]) : 0.0f;
        targetVn[i] = (vn0 < -0.5f) ? -e_r * vn0 : 0.0f; V3 rAxN = V3_Cross(rA[i],m->normal), rBxN = V3_Cross(rB[i],m->normal);
        invSumN[i] = invMassA + invMassB + (canRotateA ? V3_dot(rAxN,ApplyInvTensor(a,rAxN,Ra)) : 0.0f) + (canRotateB ? V3_dot(rBxN,ApplyInvTensor(b,rBxN,Rb)) : 0.0f);
    }
    int iters = (m->n > 1) ? 8 : 1;
    for (int it=0;it<iters;++it) { for (int i=0;i<m->n;++i) ResolveContactVelocity(a,b,m->normal,rA[i],rB[i],targetVn[i],&accumN[i],&accumT[i],bStatic,invMassA,invMassB,invSumN[i],canRotateA,canRotateB,Ra,Rb); }
    float avgPen=0.0f; for (int i=0;i<m->n;++i) {avgPen += m->p[i].pen;} avgPen /= (float)m->n; float c = vmax(avgPen - 0.005f,0.0f) * 0.9f;
    float massDiv = invMassA + invMassB + PHY_EPSILON;
    SetPosition(a,V3_AplusB(World.position[a],V3_ScaleByF(m->normal,c * invMassA / massDiv))); 
    if (!bStatic) SetPosition(b,V3_AsubB(World.position[b],V3_ScaleByF(m->normal,c * invMassB / massDiv)));
}

void EntityColliderMatrixNow(u16 i, float M[16]) { // Convex meshes need to keep their matrix4x4 up to date.
    Quaternion q = World.rotation[i]; V3 sx = V3_ScaleByF(quat_rot_v3(q,(V3){1,0,0}),World.scale[i].x); V3 sy = V3_ScaleByF(quat_rot_v3(q,(V3){0,1,0}),World.scale[i].y); V3 sz = V3_ScaleByF(quat_rot_v3(q,(V3){0,0,1}),World.scale[i].z); V3 p = World.position[i];
    M[0]=sx.x; M[1]=sx.y; M[2]=sx.z; M[3]=0.0f; M[4]=sy.x; M[5]=sy.y; M[6]=sy.z; M[7]=0.0f; M[8]=sz.x; M[9]=sz.y; M[10]=sz.z; M[11]=0.0f; M[12]=p.x; M[13]=p.y; M[14]=p.z; M[15]=1.0f;
}

INLINE int V3_IsSane(V3 v) { union { float f; unsigned int i; } ux,uy,uz; ux.f = v.x; uy.f = v.y; uz.f = v.z; return !(((ux.i & 0x7FFFFFFF) >= 0x7F800000) | ((uy.i & 0x7FFFFFFF) >= 0x7F800000) | ((uz.i & 0x7FFFFFFF) >= 0x7F800000)); }
u16 triggerVolumes[128]; u16 numTriggers;
void DrawBoxColliderColored(u16 i, Color col);
void Physics(float dt) {
    u8 substeps = (u8)vclamp((u32)(dt / MAX_STEP_SIZE + 0.5f),1u,(u32)40); float dtsub = dt / (float)substeps; dynamicEntityCount = 0;
    for (u16 i=0;i<World.instCount && dynamicEntityCount < 512;++i) {
        if (World.collider[i] == COLTYPE_MSH || World.collider[i] == COLTYPE_CVX) { World.radius[i] = modelBounds[World.collider[i] == COLTYPE_CVX ? World.instances[i].colMeshIndex : World.instances[i].modelIndex] * vmax(vmax(World.scale[i].x,World.scale[i].y),World.scale[i].z); }
        else if (likely(World.collider[i] == COLTYPE_BOX)) { World.radius[i] = vmax(World.colliderSize[i].x * 0.5f * World.scale[i].x,vmax(World.colliderSize[i].y * 0.5f * World.scale[i].y,World.colliderSize[i].z * 0.5f * World.scale[i].z)); }
        else if (World.collider[i] == COLTYPE_SPH || World.collider[i] == COLTYPE_CAP) { World.radius[i] = vmax(World.colliderSize[i].x,World.colliderSize[i].y) * vmax(World.scale[i].x,vmax(World.scale[i].y,World.scale[i].z)); }
        else World.radius[i] = World.colliderSize[i].x * vmax(World.scale[i].x,vmax(World.scale[i].y,World.scale[i].z));
        if ((World.instances[i].entflags & EF_RIGIDBODY) && (World.instances[i].entflags & EF_ACTIVE) && World.collider[i] != COLTYPE_NONE && (vabs(World.scale[i].x) > 0.01f && vabs(World.scale[i].y) > 0.01f && vabs(World.scale[i].z)) > 0.01f) {dynamicEntities[dynamicEntityCount++]=i;}
    }
    for (u8 s=0;s<substeps;++s) {
        mset(cellCounts,0,sizeof(cellCounts)); numTriggers=0;
        for (u16 t=0;t<128;++t) triggerVolumes[t]=0xFFFF;
        for (u16 i=0;i<World.instCount;++i) { // 0. Broadphase cell lists
            posBudget[i] = 0.64f; World.instances[i].cellX=(i16)PosGetCellCoordX(World.position[i].x); World.instances[i].cellZ=(i16)PosGetCellCoordZ(World.position[i].z);
            World.instances[i].cellIndex=PosGetCellCoordsP(World.instances[i].cellX,World.instances[i].cellZ);
            u32 cell=(u32)World.instances[i].cellIndex; if(cell < WORLDX*WORLDX && cellCounts[cell] < 128){cellLists[cell][cellCounts[cell]++]=i;}
            u16 idx=World.instances[i].index;
            if (unlikely(((idx >= 595 && idx <= 601) || idx == 746) && (World.instances[i].entflags & EF_ACTIVE) && numTriggers < 128)) triggerVolumes[numTriggers++] = i;
        }
        if (numTriggers >= 127) DualLogWarn("Ran out of triggers!\n");
        for (u16 i=0;i<dynamicEntityCount;++i) { // 1. Integrate velocity
            u16 a=dynamicEntities[i]; V3 acc = {0.0f,-9.81f * World.gravity[a],0.0f}; if ((a == PLAYER1) && (Cheats.noclip || World.invP1.ladderState > 0)) acc.y = 0.0f;
            acc = V3_AplusB(acc,V3_ScaleByF(World.instances[a].accumulatedForce,1.0f / World.mass[a])); World.velocity[a] = V3_AplusB(World.velocity[a],V3_ScaleByF(acc,dtsub));
            if (!V3_IsSane(World.velocity[a])) { World.velocity[a]=(V3){0.0f,0.0f,0.0f}; }
            else { float speed=V3_Mag(World.velocity[a]); if (speed > MAX_SPEED) World.velocity[a]=V3_ScaleByF(World.velocity[a],MAX_SPEED / speed); }
            float linDrag = vexp(-0.1f * dtsub); 
            V3 vel = World.velocity[a]; vel.x *= linDrag; vel.z *= linDrag; World.velocity[a] = vel; // Y axis is unaffected, so gravity accumulates infinitely until MAX_SPEED
            SetPosition(a,V3_AplusB(World.position[a],V3_ScaleByF(World.velocity[a],dtsub)));
            if (World.collider[a] != COLTYPE_CAP) {
                if (unlikely(!V3_IsSane(World.angularVelocity[a]))) { World.angularVelocity[a] = (V3){0.0f,0.0f,0.0f}; }
                else {
                    float avel = V3_Mag(World.angularVelocity[a]); if (avel > MAX_ANGULAR_SPEED) { World.angularVelocity[a] = V3_ScaleByF(World.angularVelocity[a],MAX_ANGULAR_SPEED / avel); avel = MAX_ANGULAR_SPEED; }
                    if (avel > PHY_EPSILON) { Quaternion dq = quat_from_axis_angle(V3_ScaleByF(World.angularVelocity[a],1.f / avel),avel * dtsub); World.rotation[a] = quat_normalize(quat_multiply(dq,World.rotation[a])); }
                }
            } else World.angularVelocity[a] = (V3){0.0f,0.0f,0.0f};
        }
        for (u16 i=0;i<dynamicEntityCount;++i) {
            u16 a = dynamicEntities[i]; if (unlikely(World.collider[a] == COLTYPE_MSH || (Cheats.noclip && a == PLAYER1))) continue;
            i32 cx = PosGetCellCoordX(World.position[a].x), cz = PosGetCellCoordZ(World.position[a].z); u32 mask = GetCollisionMask(World.layer[a]);
            float searchRad = World.radius[a] + V3_Mag(World.velocity[a]) * dtsub; i32 radCells = vmax((i32)(searchRad / CELLSZ),1);
            Manifold contactsMani[32]; u16 contactsOther[32]; int contactCount = 0;
            for (i32 dx = -radCells; dx <= radCells; ++dx) {
                for (i32 dz = -radCells; dz <= radCells; ++dz) { // 2. Collisions
                    u32 cell = PosGetCellCoordsP(cx + dx,cz + dz);
                    for (u16 k = 0; k < cellCounts[cell]; ++k) {
                        u16 b = cellLists[cell][k]; if (b == a || b >= World.instCount) continue;
                        if (unlikely(Cheats.noclip && b == PLAYER1)) continue;
                        if (!(mask & World.layer[b]) || World.collider[b] == COLTYPE_NONE) continue;
                        if (unlikely((World.instances[b].entflags & EF_RIGIDBODY) && b > a)) continue; // Prevent doubled restitutions causing ghosting.
                        V3 deltaPos = V3_AsubB(World.position[a],World.position[b]); float rr = (World.radius[a] + World.radius[b]) + 1.28f/*One chunk extent*/; if (V3_dot(deltaPos,deltaPos) > rr * rr) continue;
                        Manifold mf = {0}; float matA[16], matB[16]; const float *mxA = &modelMatrices[a*16], *mxB = &modelMatrices[b*16];
                        if (World.collider[a] == COLTYPE_CVX) { EntityColliderMatrixNow(a,matA); mxA = matA; } // Not MSH as only CVX is dynamically moving during physics substeps.
                        if (World.collider[b] == COLTYPE_CVX) { EntityColliderMatrixNow(b,matB); mxB = matB; }
                        if      (World.collider[a] == COLTYPE_CAP && World.collider[b] == COLTYPE_CAP) { mf = OverlapToManifold(CapCap(Entity_GetCap(a),Entity_GetCap(b))); }
                        else if (World.collider[a] == COLTYPE_CAP && World.collider[b] == COLTYPE_BOX) { mf = OverlapToManifold(CapBox(Entity_GetCap(a),Entity_GetBox(b))); }
                        else if (World.collider[a] == COLTYPE_CAP && World.collider[b] == COLTYPE_SPH) { Overlap r=SphCap(Entity_GetSph(b),Entity_GetCap(a)); if(r.hit) r.normal=V3_ScaleByF(r.normal,-1.f); mf=OverlapToManifold(r); }
                        else if (World.collider[a] == COLTYPE_SPH && World.collider[b] == COLTYPE_CAP) { mf=OverlapToManifold(SphCap(Entity_GetSph(a),Entity_GetCap(b))); }
                        else if (World.collider[a] == COLTYPE_BOX && World.collider[b] == COLTYPE_CAP) { Overlap r = CapBox(Entity_GetCap(b),Entity_GetBox(a)); if(r.hit) r.normal=V3_ScaleByF(r.normal,-1.0f); mf=OverlapToManifold(r); }
                        else if (World.collider[a] == COLTYPE_BOX && World.collider[b] == COLTYPE_BOX) { mf = OverlapToManifold(BoxBox(Entity_GetBox(a),Entity_GetBox(b))); }
                        else if (World.collider[a] == COLTYPE_SPH && World.collider[b] == COLTYPE_BOX) { ShapeSphere sa = Entity_GetSph(a); mf = OverlapToManifold(SphBox(sa.ctr,sa.rad,Entity_GetBox(b))); }
                        else if (World.collider[a] == COLTYPE_BOX && World.collider[b] == COLTYPE_SPH) { ShapeSphere sa = Entity_GetSph(b); Overlap r = SphBox(sa.ctr,sa.rad,Entity_GetBox(a)); if(r.hit) r.normal=V3_ScaleByF(r.normal,-1.0f); mf=OverlapToManifold(r); }
                        else if (World.collider[a] == COLTYPE_SPH && World.collider[b] == COLTYPE_SPH) { ShapeSphere sa = Entity_GetSph(a), sb = Entity_GetSph(b); mf = OverlapToManifold(SphSph(sa.ctr,sa.rad,sb.ctr,sb.rad)); }
                        else if (World.collider[a] == COLTYPE_CAP && World.collider[b] == COLTYPE_MSH) { mf = OverlapToManifold(CapMsh(Entity_GetCap(a),World.instances[b].modelIndex,mxB)); }
                        else if (World.collider[a] == COLTYPE_SPH && World.collider[b] == COLTYPE_MSH) { ShapeSphere sa = Entity_GetSph(a); mf = OverlapToManifold(SphMsh(sa.ctr,sa.rad,World.instances[b].modelIndex,mxB)); }
                        else if (World.collider[a] == COLTYPE_BOX && World.collider[b] == COLTYPE_MSH) { mf = BoxMsh(Entity_GetBox(a),World.instances[b].modelIndex,mxB); }
                        else if (World.collider[a] == COLTYPE_CVX && World.collider[b] == COLTYPE_MSH) { mf = CvxMsh(World.instances[a].colMeshIndex,mxA,World.instances[b].modelIndex,mxB,World.instances[a].adjacencyIdx); if(mf.n) mf.normal=V3_ScaleByF(mf.normal,-1.0f); }
                        else if (World.collider[a] == COLTYPE_CAP && World.collider[b] == COLTYPE_CVX) { mf = PrimitiveCvx(a,World.instances[b].colMeshIndex,mxB,World.instances[b].adjacencyIdx); if(mf.n) mf.normal=V3_ScaleByF(mf.normal,-1.0f); }
                        else if (World.collider[a] == COLTYPE_CVX && World.collider[b] == COLTYPE_CAP) { mf = PrimitiveCvx(b,World.instances[a].colMeshIndex,mxA,World.instances[a].adjacencyIdx); }
                        else if (World.collider[a] == COLTYPE_SPH && World.collider[b] == COLTYPE_CVX) { mf = PrimitiveCvx(a,World.instances[b].colMeshIndex,mxB,World.instances[b].adjacencyIdx); if(mf.n) mf.normal=V3_ScaleByF(mf.normal,-1.0f); }
                        else if (World.collider[a] == COLTYPE_CVX && World.collider[b] == COLTYPE_SPH) { mf = PrimitiveCvx(b,World.instances[a].colMeshIndex,mxA,World.instances[a].adjacencyIdx); }
                        else if (World.collider[a] == COLTYPE_BOX && World.collider[b] == COLTYPE_CVX) { mf = PrimitiveCvx(a,World.instances[b].colMeshIndex,mxB,World.instances[b].adjacencyIdx); if(mf.n) mf.normal=V3_ScaleByF(mf.normal,-1.0f); }
                        else if (World.collider[a] == COLTYPE_CVX && World.collider[b] == COLTYPE_BOX) { mf = PrimitiveCvx(b,World.instances[a].colMeshIndex,mxA,World.instances[a].adjacencyIdx); }
                        else if (World.collider[a] == COLTYPE_CVX && World.collider[b] == COLTYPE_CVX) { mf = CvxCvx(World.instances[a].colMeshIndex,World.instances[b].colMeshIndex,mxA,mxB,World.instances[a].adjacencyIdx,World.instances[b].adjacencyIdx); if(mf.n) mf.normal=V3_ScaleByF(mf.normal,-1.0f); }
                        else { mf=OverlapToManifold(SphSph(World.position[a],World.colliderSize[a].x,World.position[b],World.colliderSize[b].x)); }
                        if (likely(mf.n && contactCount < 32)) { contactsMani[contactCount] = mf; contactsOther[contactCount] = b; contactCount++; }
                    }
                }
            }
            World.colliding[a]=false; flag_set(&World.instances[a].entflags,EF_GROUNDED,false);
            for (int c = 0; c < contactCount; ++c) { Manifold *mfp=&contactsMani[c]; World.colliding[a]=World.colliding[contactsOther[c]]=true; if (V3_dot(mfp->normal,(V3){0.0f,1.0f,0.0f}) >= 0.574f) {World.instances[a].entflags |= EF_GROUNDED;} ApplyManifoldResponse(a,contactsOther[c],mfp); } // 3. Restitution
            World.instances[a].accumulatedForce = (V3){0.0f,0.0f,0.0f};
        }
        for (u16 i=0;i<numTriggers;++i) {
            u16 self = triggerVolumes[i];
            u16 trigdx=World.instances[self].index;
            if (Cheats.showPhys) DrawBoxColliderColored(self,(Color){1.0f,0.642f,0.0f,0.5f});
            for (u16 o=0;o<dynamicEntityCount;++o) { // 4. Triggers
                u16 other = dynamicEntities[o]; if (World.collider[other] == COLTYPE_NONE || !(World.instances[other].entflags & EF_ACTIVE)) continue;
                if (!PointInOBB(World.position[other],Entity_GetBox(self))) continue;
                if (other != PLAYER1 && trigdx == 596) { trigger_gravitylift_touch(self,other); continue; }
                World.Sys_Music.cyberTube = false; World.gravity[PLAYER1] = 1.0f; World.invP1.ladderState=0; World.Sys_Music.inZone = World.Sys_Music.elevator = World.Sys_Music.distortion = false; // Reset trigger sustained flags
                switch(trigdx) {
                    case 595/*trigger_cyberpush*/:   trigger_cyberpush_touch(self,other); break;
                    case 596/*trigger_gravitylift*/: trigger_gravitylift_touch(self,other); break;
                    case 597/*trigger_ladder*/:      World.invP1.ladderState=1; break;
                    case 598/*trigger_multiple*/: case 600/*trigger_once*/: TriggerTriggerTripped(self,other); break;
                    case 599/*trigger_music*/: { TrackType tt=World.instances[self].trackType; World.Sys_Music.inZone=true; World.Sys_Music.elevator=(tt == TT_Elevator); World.Sys_Music.distortion=(tt == TT_Distortion); break; }
                    case 601/*trigger_radiation*/:            World.invP1.radiationArea=true;World.instances[PLAYER1].radiation=World.instances[self].radiation; break; // TODO bleedoff when !radiationArea, amelioration from envirosuit, detox patch negation for 30secs
                    case 746/*weapon_grenadeenergmine_live*/: TakeEnergy(256.0f); break;
                }
            }
        }
    }
}

void AddForce(u16 i, V3 f, bool imp) { if (imp) { World.velocity[i] = V3_AplusB(World.velocity[i],V3_ScaleByF(f,1.0f / vmax(World.mass[i],0.001f))); } else { World.instances[i].accumulatedForce = V3_AplusB(World.instances[i].accumulatedForce,f); } }
// Player Movement
float GetBasePlayerSpeed(u16 p,bool running){
    bool sprint=Sprint(); if(Cheats.noclip)return PLAYER_MAX_CYBER_SPEED*(sprint?2.5f:1.5f); if(World.curLev==LEVEL_CYBERSPACE)return PLAYER_MAX_CYBER_SPEED;
    BodyState b=World.instances[p].bodyState; float v=WALK_SPEED;
    switch(b){ case BodyState_CrouchingDown: case BodyState_Crouch:v=CROUCH_SPEED; break; case BodyState_Prone: case BodyState_ProningDown: case BodyState_ProningUp:v=PLAYER_MAX_PRONE_SPEED; break; default:break; }
    if ((sprint||World.boosterActive) && running) { v = World.invP1.fatigue > 80.0f && World.boosterActive ? SPRINT_SPEED_FATIGUED : SPRINT_SPEED;
    if (b==BodyState_Standing||b==BodyState_Crouch||b==BodyState_CrouchingDown)  v -= (WALK_SPEED-CROUCH_SPEED)*1.5f;
    else if(b==BodyState_Prone||b==BodyState_ProningDown||b==BodyState_ProningUp)v -= (WALK_SPEED-PLAYER_MAX_PRONE_SPEED)*2.f;}
    return v + (World.boosterActive ? PLAYER_BOOSTER_SPEED_BOOST : 0.0f);
}

INLINE float smooth_damp(float cur, float targ, float* vel, float tm, float dt) { float o=2.0f / vmax(tm,0.0001f); float x=o * dt; float exp=1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x); float d=cur - targ; float t=(*vel + o * d) * dt; *vel=(*vel - o * t) * exp; return targ + (d + t) * exp; }
Overlap CapMsh(ShapeCapsule,u16,const float*);
bool CantStand(u16 playerIdx, float targetHeight) { // I can't stand it.
    float oldHeight = World.colliderSize[playerIdx].y; V3 oldPos = World.position[playerIdx];
    World.colliderSize[playerIdx].y = targetHeight; // Temporarily morph player into the standing capsule
    World.position[playerIdx].y += (targetHeight - oldHeight); 
    bool blocked = false;
    i32 cx = PosGetCellCoordX(World.position[playerIdx].x);
    i32 cz = PosGetCellCoordZ(World.position[playerIdx].z);
    u32 mask = GetCollisionMask(World.layer[playerIdx]);
    for (i32 dx = -1; dx <= 1 && !blocked; ++dx) {
        for (i32 dz = -1; dz <= 1 && !blocked; ++dz) {
            u32 cell = PosGetCellCoordsP(cx + dx, cz + dz);
            for (u16 k = 0; k < cellCounts[cell]; ++k) {
                u16 b = cellLists[cell][k]; if (b == playerIdx || !(mask & World.layer[b]) || World.collider[b] == COLTYPE_NONE) continue;
                if (World.collider[b] == COLTYPE_MSH) { Overlap r = CapMsh(Entity_GetCap(playerIdx),World.instances[b].modelIndex,&modelMatrices[b*16]); if (r.hit && r.pen > 0.08f) { blocked = true; break; } }
            }
        }
    }
    World.colliderSize[playerIdx].y = oldHeight;
    World.position[playerIdx] = oldPos;
    return blocked;
}

KeyState* GetCodeMapping(int settingIndex);
void ApplyPlayerMovements(float dt) {
    Entity *p = &World.instances[PLAYER1]; Quaternion r = World.rotation[PLAYER1]; float leanSpeed = 70.0f, leanMaxAngle = 35.0f; float leanInput = (float)LeanLeft() - (float)LeanRight(); bool doubleTapLean = DoubleTapLeanLeft() || DoubleTapLeanRight();
    bool movingForward = Forward() > 0.1f, leanRight = leanInput < 0.0f, leanLeft = leanInput > 0.0f;
    if (doubleTapLean) { World.invP1.leanResetting = true; World.invP1.leanVelocity = 0.0f; KeyState *kL = GetCodeMapping(7), *kR = GetCodeMapping(8); kL->pressed = kR->pressed = false; } // Double-tap lean: initiate smooth reset to upright over 0.2 seconds
    if (World.invP1.leanResetting) { 
        World.invP1.leanTarget = smooth_damp(World.invP1.leanTarget,0.0f,&World.invP1.leanVelocity,0.2f,dt); 
        if(vabs(World.invP1.leanTarget) < 0.5f){World.invP1.leanTarget=World.invP1.leanVelocity=0.0f; World.invP1.leanResetting=false;} 
    } else {
        if (leanLeft || leanRight) { if(leanLeft){World.invP1.leanRightTapFinished =0;} if(leanRight){World.invP1.leanLeftTapFinished=0;} World.invP1.leanTarget=vclamp(World.invP1.leanTarget + (leanInput * leanSpeed * dt),-leanMaxAngle,leanMaxAngle); }
        else if (movingForward) { if (vabs(World.invP1.leanTarget) < 0.5f) { World.invP1.leanTarget = 0.0f; } else { World.invP1.leanTarget -= (World.invP1.leanTarget > 0.0f ? 1.0f : -1.0f) * leanSpeed * dt; } }
    }
    World.cam_roll = World.invP1.leanTarget;
    float targetRatio=1.0f, transitionSec=0.2f; float currentRatio=World.invP1.currentCrouchRatio;
    if (Crouch()) { // Crouch key always targets crouch ratio from any state
        if (p->bodyState == BodyState_Crouch) { if (!CantStand(PLAYER1,PLAYER_HEIGHT)){p->bodyState = BodyState_StandingUp;}} // Already at crouch → toggle up to standing
        else if (currentRatio > PLAYER_CROUCH_RATIO) { p->bodyState = BodyState_CrouchingDown;} // Above crouch → go down to crouch (handles "if standing up will go back to crouched")
        else {p->bodyState=BodyState_ProningUp;} // Below crouch → go up to crouch (handles "if proning down will go back to crouched")
    } else if (Prone()) {
        if (p->bodyState == BodyState_Standing) { p->bodyState = BodyState_ProningDown; } // Standing → go to prone
        else if (currentRatio > PLAYER_CROUCH_RATIO) { if (!CantStand(PLAYER1,PLAYER_HEIGHT)){p->bodyState=BodyState_StandingUp;}else{p->bodyState = BodyState_ProningDown;} } // Between crouch and standing → up to standing
        else if (p->bodyState == BodyState_Crouch) { p->bodyState = BodyState_ProningDown; } // Crouch → go to prone
        else { p->bodyState = BodyState_ProningUp; } // Between prone and crouch, or prone → up to crouch
    }
    switch (p->bodyState) {
        case BodyState_CrouchingDown:targetRatio=-0.01f; break;                       case BodyState_StandingUp:targetRatio=1.01f;  break;      case BodyState_ProningDown:targetRatio=-0.01f; break; 
        case BodyState_ProningUp:    targetRatio=1.01f; transitionSec+=0.1f; break; case BodyState_Crouch:    targetRatio=PLAYER_CROUCH_RATIO; break; case BodyState_Prone:      targetRatio=PLAYER_PRONE_RATIO; break; default: targetRatio=1.0f; break;
    }
    float lastRatio = World.invP1.currentCrouchRatio;
    World.invP1.currentCrouchRatio = smooth_damp(lastRatio,targetRatio,&World.invP1.crouchingVelocity,transitionSec,dt);
    if (World.invP1.currentCrouchRatio >= 1.0f) { World.invP1.currentCrouchRatio = 1.0f; if(p->bodyState == BodyState_StandingUp){p->bodyState=BodyState_Standing;} }
    else if (p->bodyState == BodyState_CrouchingDown && World.invP1.currentCrouchRatio <= PLAYER_CROUCH_RATIO) { World.invP1.currentCrouchRatio = PLAYER_CROUCH_RATIO; p->bodyState = BodyState_Crouch; }
    else if (p->bodyState == BodyState_ProningUp && World.invP1.currentCrouchRatio >= PLAYER_CROUCH_RATIO) { World.invP1.currentCrouchRatio = PLAYER_CROUCH_RATIO; p->bodyState = BodyState_Crouch; }
    else if (p->bodyState == BodyState_ProningDown && World.invP1.currentCrouchRatio <= PLAYER_PRONE_RATIO) { World.invP1.currentCrouchRatio = PLAYER_PRONE_RATIO; p->bodyState = BodyState_Prone; }
    World.colliderSize[PLAYER1].y = PLAYER_HEIGHT * World.invP1.currentCrouchRatio; // Split capsule shape in the middle, camera is thus 0.16 away from top of the capsule ((2 / 2 = 1) - 0.84 which is PLAYER_CAM_OFFSET_Y)
    float h=(float)Forward() - (float)Backpedal(), s=(float)StrafeRight() - (float)StrafeLeft(), vertInput=(float)SwimUp() - (float)SwimDn();
    float y2=r.y*r.y, xz=r.x*r.z, wy=r.w*r.y;
    p->forward=V3_Normalize((V3){ 2.0f*(xz + wy),2.0f*(r.y*r.z - r.w*r.x),1.0f - 2.0f*(r.x*r.x + y2) }); p->right=V3_Normalize((V3){ 1.0f - 2.0f*(y2 + r.z*r.z),2.0f*(r.x*r.y + r.w*r.z),2.0f*(xz - wy) });
    V3 inputDir={ p->forward.x*h + p->right.x*s,vertInput,p->forward.z*h + p->right.z*s}; 
    float inputLenSq = V3_dot(inputDir,inputDir); V3 w = (inputLenSq > 0.0001f) ? V3_ScaleByF(inputDir, 1.0f / vsqrtf(inputLenSq)) : (V3){0, 0, 0};
    bool isRunning = (inputLenSq > 0.01f); float speed = GetBasePlayerSpeed(PLAYER1,isRunning) * 1.75f, accel=World.boosterActive ? 1.0f : 3.0f; V3 targetVel = V3_ScaleByF(w,speed); 
    if (World.invP1.ladderState > 0) {
        float climbSpeed = (Sprint() && isRunning) ? 1.2f : 0.4f;
        targetVel = (V3){p->right.x * s * speed * 0.3f, h * climbSpeed * 25.0f, p->right.z * s * speed * 0.3f};
        accel = 5.0f;
    } else { if (vabs(vertInput) < 0.001f) { targetVel.y = World.velocity[PLAYER1].y; } }
    V3 dv = V3_AsubB(targetVel, World.velocity[PLAYER1]); 
    dv = (V3){ vclamp(dv.x, -10.0f, 10.0f), vclamp(dv.y, -10.0f, 10.0f), vclamp(dv.z, -10.0f, 10.0f) };
    World.velocity[PLAYER1] = V3_AplusB(World.velocity[PLAYER1], V3_ScaleByF(dv, accel * vclamp(dt, 0.0005f, 0.1f)));
}
