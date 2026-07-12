// physics.c - The Jack Physics Engine, By W. Josiah Jack MIT-0 -- full rigidbody 3D with torque for sphere, box, capsule, convex mesh dynamic objects and same set plus arbitrary trisoup mesh colliders for statics.
#define FRICTION_SLIDE 1.0f // Directly apply tangential torque
#define FRICTION_ROLL 0.05f
#define PHY_EPSILON 0.0001f
#define MAX_SPEED 16.666666f // m/s fastest projectile is railgun given 5.0 impulse with 0.3 mass = 5.0 / 0.3
#define MAX_STEP_SIZE (0.08f / MAX_SPEED) // 0.01 s
#define MAX_ANGULAR_SPEED 8.0f // Kind of arbitrary, meant to keep things sane
#define MANIFOLD_MAX 4
#define MANIFOLD_TIE_MARGIN 0.005f
#define MANIFOLD_ALIGN_THRESHOLD 0.8f
#define CVXMSH_HULL_CACHE 1024
typedef struct { V3 v[4];/*Minkowski difference verts (wA - wB)*/   V3 wA[4],wB[4];/*Cached support points from Shape A,B*/ i32 n;/*Vertex count*/ } Simplex3D;
typedef struct { bool hit; V3 point,normal; float pen; } Overlap; typedef struct { V3 point; float pen; } ManifoldPt; typedef struct { V3 normal; ManifoldPt p[MANIFOLD_MAX]; i32 n; float maxPen; } Manifold;
float posBudget[INSTANCE_COUNT]; // Remaining |Δpos| entity i may receive this substep; reset every substep in Physics().
u16 dynamicEntities[512], dynamicEntityCount;
void SetPosition(u16 i, V3 newpos, bool teleport) {
    float d=V3_Dist(World.position[i],newpos); if(d < 0.001f){return;/*made it!*/} if (teleport) {World.position[i]=newpos;/*peeershuhweeee!*/ return;} float allowed=vmin(d,posBudget[i]); if (allowed < 0.001f) {return;}
    V3 dir=V3_Normalize(V3_AsubB(newpos,World.position[i])); World.position[i]=V3_AplusB(World.position[i],V3_ScaleByF(dir,allowed)); posBudget[i] -= allowed;
}

INLINE u32 PosGetCellCoordsP(i32 cx, i32 cz) { cx = clamp(cx,0,WORLDX_0BASED); cz = clamp(cz,0,WORLDX_0BASED); return (u32)cz * WORLDX + (u32)cx; }
inline Manifold OverlapToManifold(Overlap r) { Manifold m = {0}; if (r.hit && r.pen > PHY_EPSILON) { m.normal = r.normal; m.n = 1; m.p[0] = (ManifoldPt){r.point, r.pen}; m.maxPen = r.pen; } return m; }
inline Overlap SphSph(V3 a, float ar, V3 b, float br) { V3 delta=V3_AsubB(a,b); float d2=V3_dot(delta,delta), rs=ar + br; if (d2 >= rs * rs) return (Overlap){0}; float d = vsqrtf(vmax(d2, 0.0f)); V3 n = (d < PHY_EPSILON) ? (V3){0,1,0} : V3_ScaleByF(delta,1.0f/d); return (Overlap){true,V3_AplusB(b,V3_ScaleByF(n,br)),n,rs - d}; }
inline Overlap SphCap(ShapeSphere s, ShapeCapsule c) {
    V3 seg = V3_AsubB(c.tip,c.base); float segLen2 = V3_dot(seg,seg); if (segLen2 < PHY_EPSILON){return SphSph(s.ctr,s.rad,c.base,c.rad);}
    V3 toS = V3_AsubB(s.ctr,c.base); float t = V3_dot(toS,seg) / segLen2; t = vclamp(t,0.0f,1.0f); V3 closest = V3_AplusB(c.base,V3_ScaleByF(seg, t)); return SphSph(s.ctr, s.rad, closest, c.rad);
}

float ClosestSegmentSegment(V3 a0, V3 a1, V3 b0, V3 b1, float *sc, float *tc) { // Closest point between two line segments A0-A1 and B0-B1.  Returns squared distance and writes sc, tc (parameters on each segment).
    V3 d1 = V3_AsubB(a1,a0), d2 = V3_AsubB(b1,b0), r = V3_AsubB(a0,b0);
    float a = V3_dot(d1,d1), e = V3_dot(d2,d2), f = V3_dot(d2,r); if (a < PHY_EPSILON && e < PHY_EPSILON) { *sc = *tc = 0.0f; return V3_dot(r,r); }
    if (a < PHY_EPSILON) { *sc = 0.0f; *tc = vclamp(f/e,0.0f,1.0f); }
    else {
        float c = V3_dot(d1,r);
        if (e < PHY_EPSILON) { *tc = 0.0f; *sc = vclamp(-c/a,0.0f,1.0f); }
        else {
            float b = V3_dot(d1,d2), denom = a*e - b*b; *sc = (denom > PHY_EPSILON) ? vclamp((b*f - c*e)/denom,0.0f,1.0f) : 0.0f; *tc = (b * (*sc) + f) / e;
            if (*tc < 0.0f) { *tc = 0.0f; *sc = vclamp(-c/a,0.0f,1.0f); }
            else if (*tc > 1.0f) { *tc = 1.0f; *sc = vclamp((b-c)/a,0.0f,1.0f); }
        }
    }
    V3 diff = V3_AsubB(V3_AplusB(a0,V3_ScaleByF(d1,*sc)),V3_AplusB(b0,V3_ScaleByF(d2,*tc)));
    return V3_dot(diff,diff);
}

Overlap CapCap(ShapeCapsule a, ShapeCapsule b) {
    Overlap r = {0}; float sc, tc; float distSq = ClosestSegmentSegment(a.base,a.tip,b.base,b.tip,&sc,&tc); float radSum = a.rad + b.rad; if (distSq >= radSum * radSum) return r;
    float dist = vsqrtf(vmax(distSq, 0.0f)); r.pen = radSum - dist; r.hit = true;
    V3 ptA = V3_AplusB(a.base,V3_ScaleByF(V3_AsubB(a.tip,a.base),sc)); V3 ptB = V3_AplusB(b.base,V3_ScaleByF(V3_AsubB(b.tip,b.base),tc)); V3 delta = V3_AsubB(ptA,ptB);
    r.normal = (dist < PHY_EPSILON) ? (V3){0,1,0} : V3_ScaleByF(delta, 1.0f/dist);
    r.point  = V3_AplusB(ptB, V3_ScaleByF(r.normal,b.rad));
    return r;
}

void obb_axes(Quaternion q, V3 *ax, V3 *ay, V3 *az) { *ax=quat_rot_v3(q,(V3){1,0,0}); *ay=quat_rot_v3(q,(V3){0,1,0}); *az=quat_rot_v3(q,(V3){0,0,1}); }
Overlap SphBox(V3 ctr, float rad, ShapeBox box) {
    Overlap r = {0}; V3 ax,ay,az; obb_axes(box.rot,&ax,&ay,&az); V3 d = V3_AsubB(ctr,box.ctr);
    float lx = V3_dot(d,ax), ly = V3_dot(d,ay), lz = V3_dot(d,az);
    V3 closest = V3_AplusB(V3_AplusB(V3_AplusB(box.ctr,V3_ScaleByF(ax,vclamp(lx,-box.hExt.x,box.hExt.x))),V3_ScaleByF(ay,vclamp(ly,-box.hExt.y,box.hExt.y))),V3_ScaleByF(az,vclamp(lz,-box.hExt.z,box.hExt.z)));
    V3 delta = V3_AsubB(ctr,closest); float distSq = V3_dot(delta,delta); if (distSq >= rad * rad) return r;
    r.hit = true; float dist = vsqrtf(vmax(distSq, 0.0f));
    if (dist > PHY_EPSILON) { r.normal = V3_ScaleByF(delta, 1.0f / dist); r.pen = rad - dist; }
    else { // Center is inside OBB — find minimum penetration axis from pre-computed local coords
        float dx = box.hExt.x - vabs(lx), dy = box.hExt.y - vabs(ly), dz = box.hExt.z - vabs(lz);
        if (dx < dy && dx < dz) { r.normal = V3_ScaleByF(ax,lx > 0 ? 1.f : -1.f); r.pen = rad + dx; }
        else if (dy < dz)       { r.normal = V3_ScaleByF(ay,ly > 0 ? 1.f : -1.f); r.pen = rad + dy; }
        else                    { r.normal = V3_ScaleByF(az,lz > 0 ? 1.f : -1.f); r.pen = rad + dz; }
    }
    r.point=V3_AsubB(ctr,V3_ScaleByF(r.normal,rad-r.pen));
    return r;
}

u32 GetCollisionMask(u32 layer) {
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
        case L_PlayerTriggerOnly:return L_Player|L_Player2;
        case L_Trigger:          return L_Default|L_Geometry|L_NPC|L_PlayerBullets|L_Player|L_PhysObjects|L_Door|L_InterDebris|L_Clip;
        case L_Door:             return L_Default|L_TransparentFX|L_Geometry|L_NPC|L_PlayerBullets|L_Player|L_Corpse|L_PhysObjects|L_Trigger|L_Door|L_InterDebris|L_Player2|L_NPCBullet|L_Clip;
        case L_InterDebris:      return L_Default|L_Geometry|L_NPC|L_PlayerBullets|L_PhysObjects|L_Trigger|L_Door|L_NPCBullet|L_Clip;
        case L_Player2:          return L_Default|L_TransparentFX|L_Geometry|L_NPC|L_PlayerBullets|L_Player|L_PhysObjects|L_PlayerTriggerOnly|L_Trigger|L_Door|L_InterDebris|L_Player2|L_NPCBullet|L_Clip;
        case L_NPCBullet:        return L_Default|L_TransparentFX|L_Geometry|L_PlayerBullets|L_Player|L_Corpse|L_PhysObjects|L_Door|L_InterDebris|L_Player2|L_Clip|L_CorpseSearchable;
        case L_Clip:             return L_Player|L_Player2|L_NPC;
        case L_CorpseSearchable: return L_Default|L_PlayerBullets;
        default:                 return 0u;
    }
}

ShapeCapsule Entity_GetCap(u16 i) {
    float scaleMax = vmax(World.scale[i].x,vmax(World.scale[i].y,World.scale[i].z));
    float r = World.colliderSize[i].x * scaleMax; float hi = vmax(0.0f, (World.colliderSize[i].y * 0.5f * scaleMax) - r); V3 wc,axis;
    if (i == PLAYER1 || World.layer[i] == L_NPC) { wc = V3_AplusB(World.position[i], World.colliderCenter[i]); axis = (V3){0.0f,1.0f,0.0f};/*Player+NPC remain strictly upright*/ }
    else {
        wc = V3_AplusB(World.position[i], quat_rot_v3(World.rotation[i], World.colliderCenter[i]));
        axis = (World.colliderSize[i].z < 0.5f) ? quat_rot_v3(World.rotation[i], (V3){1,0,0}) : (World.colliderSize[i].z < 1.5f) ? quat_rot_v3(World.rotation[i], (V3){0,1,0}) : quat_rot_v3(World.rotation[i], (V3){0,0,1});
    }
    return (ShapeCapsule){.tip=V3_AplusB(wc,V3_ScaleByF(axis,hi)),.base=V3_AsubB(wc,V3_ScaleByF(axis,hi)),.rad=r};
}

ShapeBox Entity_GetBox(u16 i) { return (ShapeBox){.ctr=V3_AplusB(World.position[i],quat_rot_v3(World.rotation[i],World.colliderCenter[i])),.hExt=(V3){World.colliderSize[i].x*0.5f * World.scale[i].x,World.colliderSize[i].y*0.5f * World.scale[i].y,World.colliderSize[i].z*0.5f * World.scale[i].z},.rot=World.rotation[i]}; }
ShapeSphere Entity_GetSph(u16 i) { return (ShapeSphere){.ctr=V3_AplusB(World.position[i],quat_rot_v3(World.rotation[i],World.colliderCenter[i])),.rad = World.colliderSize[i].x * vmax(World.scale[i].x,vmax(World.scale[i].y,World.scale[i].z))}; }
static u16 cellLists[WORLDX*WORLDX][128],cellCounts[WORLDX*WORLDX];
float GetColRad(u16 i) {
    u8 col = World.collider[i];
    if (col == COLTYPE_MSH || col == COLTYPE_CVX) { u16 mIdx = (col == COLTYPE_CVX) ? World.instances[i].colMeshIndex : World.instances[i].modelIndex; return modelBounds[mIdx] * vmax(vmax(World.scale[i].x,World.scale[i].y),World.scale[i].z); }
    if (col == COLTYPE_BOX) { float hx=World.colliderSize[i].x *0.5f* World.scale[i].x, hy=World.colliderSize[i].y *0.5f* World.scale[i].y, hz=World.colliderSize[i].z *0.5f* World.scale[i].z; return vsqrtf(hx*hx + hy*hy + hz*hz); }
    if (col == COLTYPE_CAP) { ShapeCapsule cap=Entity_GetCap(i); V3 c=World.position[i]; float db=V3_Mag(V3_AsubB(c,cap.base)) + cap.rad; float dt=V3_Mag(V3_AsubB(c,cap.tip)) + cap.rad; return vmax(db,dt); }
    if (col == COLTYPE_SPH) { return Entity_GetSph(i).rad; }
    return World.colliderSize[i].x * vmax(World.scale[i].x, vmax(World.scale[i].y, World.scale[i].z));
}

Quaternion quat_from_axis_angle(V3 axis, float angle) { float half = angle * 0.5f; float s = vsinf(half); return (Quaternion){axis.x*s,axis.y*s,axis.z*s,vcosf(half)}; }
Quaternion quat_normalize(Quaternion q) { float len2 = q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w; if (len2 < PHY_EPSILON) {return (Quaternion){0,0,0,1};} float inv=vinvsqtf(len2); q.x*=inv; q.y*=inv; q.z*=inv; q.w*=inv; return q; }
static inline V3 MeshVert(u16 m, u32 i) { const u8* p=modelVertices[m]+i*CPU_VRT_SZ; return (V3){*(float*)(p+0),*(float*)(p+4),*(float*)(p+8)}; }
static void ComputeConvexMeshInertiaTensor(u16 i) {
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
    float sx = World.scale[i].x, sy = World.scale[i].y, sz = World.scale[i].z;
    float sd = World.mass[i] / (volAcc * 10.0f), so = World.mass[i] / (volAcc * 20.0f);
    float cx = cm[0] / (4.0f * volAcc), cy = cm[1] / (4.0f * volAcc), cz = cm[2] / (4.0f * volAcc);
    float scx = cx * sx, scy = cy * sy, scz = cz * sz, m=World.mass[i];
    float Ixx = sd*(acc[1]*sy*sy + acc[2]*sz*sz) - m*(scy*scy + scz*scz); float Iyy = sd*(acc[0]*sx*sx + acc[2]*sz*sz) - m*(scx*scx + scz*scz); float Izz = sd*(acc[0]*sx*sx + acc[1]*sy*sy) - m*(scx*scx + scy*scy);
    float Ixy = -(so*acc[3]*sx*sy - m*scx*scy); float Ixz = -(so*acc[4]*sx*sz - m*scx*scz); float Iyz = -(so*acc[5]*sy*sz - m*scy*scz);
    float r = modelBounds[mi] * vmax(vmax(sx,sy),sz); float mn = 0.04f * m * r * r;
    Ixx = vmax(Ixx,mn); Iyy = vmax(Iyy,mn); Izz = vmax(Izz,mn);
    float *IT=World.inertiaTensor[i]; IT[0]=Ixx; IT[1]=Iyy; IT[2]=Izz; IT[3]=Ixy; IT[4]=Ixz; IT[5]=Iyz;
    float det = Ixx*(Iyy*Izz - Iyz*Iyz) - Ixy*(Ixy*Izz - Ixz*Iyz) + Ixz*(Ixy*Iyz - Iyy*Ixz);
    if (vabs(det) < PHY_EPSILON) return;
    float invDet = 1.0f / det, *iI=World.invInertiaTensor[i];
    iI[0]=(Iyy*Izz - Iyz*Iyz)*invDet; iI[1]=(Ixx*Izz - Ixz*Ixz)*invDet; iI[2]=(Ixx*Iyy - Ixy*Ixy)*invDet;
    iI[3]=(Ixz*Iyz - Ixy*Izz)*invDet; iI[4]=(Ixy*Iyz - Iyy*Ixz)*invDet; iI[5]=(Ixy*Ixz - Ixx*Iyz)*invDet;
    World.invTnsrValid[i]=true;
}

static Overlap BoxBox(ShapeBox a, ShapeBox b) {
    Overlap r = {0}; V3 aAxes[3],bAxes[3]; obb_axes(a.rot,&aAxes[0],&aAxes[1],&aAxes[2]); obb_axes(b.rot,&bAxes[0],&bAxes[1],&bAxes[2]); 
    V3 T = V3_AsubB(b.ctr,a.ctr); float R[3][3],AbsR[3][3];
    for (int i=0;i<3;i++) for (int j=0;j<3;j++) { R[i][j]=V3_dot(aAxes[i],bAxes[j]); AbsR[i][j]=vabs(R[i][j])+1e-6f; }
    float minOverlap=1e9f; int bestAxis=-1; bool flipNormal=false; V3 bestEdgeAxis={0,1,0};
    for (int i=0;i<3;i++) {
        float ra=((float*)&a.hExt)[i], rb=b.hExt.x*AbsR[i][0]+b.hExt.y*AbsR[i][1]+b.hExt.z*AbsR[i][2];
        float t=vabs(V3_dot(T,aAxes[i])); if (t>ra+rb) return r;
        float ov=(ra+rb)-t; if (ov<minOverlap) { minOverlap=ov; bestAxis=i; flipNormal=(V3_dot(T,aAxes[i])<0.f); }
    }
    for (int i=0;i<3;i++) {
        float ra=a.hExt.x*AbsR[0][i]+a.hExt.y*AbsR[1][i]+a.hExt.z*AbsR[2][i], rb=((float*)&b.hExt)[i];
        float t=vabs(V3_dot(T,bAxes[i])); if (t>ra+rb) return r;
        float ov=(ra+rb)-t; if (ov<minOverlap) { minOverlap=ov; bestAxis=3+i; flipNormal=(V3_dot(T,bAxes[i])<0.f); }
    }
    for (int i=0;i<3;i++) for (int j=0;j<3;j++) {
        int i1=(i+1)%3, i2=(i+2)%3, j1=(j+1)%3, j2=(j+2)%3;
        float t=vabs(V3_dot(T,aAxes[i2])*R[i1][j] - V3_dot(T,aAxes[i1])*R[i2][j]);
        float ra=((float*)&a.hExt)[i1]*AbsR[i2][j]+((float*)&a.hExt)[i2]*AbsR[i1][j];
        float rb=((float*)&b.hExt)[j1]*AbsR[i][j2]+((float*)&b.hExt)[j2]*AbsR[i][j1];
        if (t>ra+rb) return r;
        float axLenSq=1.f-(R[i][j]*R[i][j]);
        if (axLenSq>1e-4f) { float ov=((ra+rb)-t)/vsqrtf(axLenSq); if (ov<minOverlap) { V3 ea=V3_Cross(aAxes[i],bAxes[j]); minOverlap=ov; bestAxis=6+i*3+j; bestEdgeAxis=ea; flipNormal=(V3_dot(T,ea)<0.f); } }
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

static inline V3 MvVert(const float* M, V3 v) { return (V3){ M[0]*v.x + M[4]*v.y + M[8]*v.z  + M[12], M[1]*v.x + M[5]*v.y + M[9]*v.z  + M[13], M[2]*v.x + M[6]*v.y + M[10]*v.z + M[14] }; }
static inline void MeshTri(u16 m, u32 ti, const float* mx, V3* a, V3* b, V3* c) { u32 i0=modelTriangles[m][ti*3+0],i1=modelTriangles[m][ti*3+1],i2=modelTriangles[m][ti*3+2]; *a=MvVert(mx,MeshVert(m,i0)); *b=MvVert(mx,MeshVert(m,i1)); *c=MvVert(mx,MeshVert(m,i2)); }
static V3 MeshSupport(u16 m, const float* M, V3 d) {
    u32 n = modelVertexCounts[m]; if (!n) {return (V3){0};}
    const u8* vb = modelVertices[m]; V3 b={0}; float top=-1e9f;
    for (u32 i=0;i<n;++i) {const u8* p=vb + i*CPU_VRT_SZ; V3 w=MvVert(M,(V3){*(float*)(p+0),*(float*)(p+4),*(float*)(p+8)}); float dot=V3_dot(w,d); b=(dot>top) ? (top=dot,w) : b;}
    return b;
}

typedef struct { V3 v[CVXMSH_HULL_CACHE]; u32 n; bool ok; } HullCache;
static inline HullCache CacheHull(u16 m, const float* M) { HullCache h; h.n = modelVertexCounts[m]; h.ok = h.n && h.n <= CVXMSH_HULL_CACHE; if (!h.ok) {return h;} const u8* vb = modelVertices[m]; for (u32 i=0;i<h.n;++i) { const u8* p=vb+i*CPU_VRT_SZ; h.v[i]=MvVert(M,(V3){*(float*)(p+0),*(float*)(p+4),*(float*)(p+8)}); } return h; }
static inline V3 CachedSupport(const V3* v, u32 n, V3 d) { V3 best = v[0]; float top = V3_dot(v[0],d); for (u32 i = 1; i < n; ++i) { float dot = V3_dot(v[i],d); if (dot > top) { top = dot; best = v[i]; } } return best; }
static inline V3 TP(V3 a, V3 b, V3 c) { return V3_Cross(V3_Cross(a,b),c); }
static inline V3 SphSupport(ShapeSphere b, V3 d) { float L = V3_dot(d,d); return V3_AplusB(b.ctr, (L > PHY_EPSILON) ? V3_ScaleByF(d, b.rad / vsqrtf(L)) : (V3){0,b.rad,0}); }
static inline V3 BoxSupport(ShapeBox b, V3 d) { V3 x,y,z; obb_axes(b.rot,&x,&y,&z); float kx = V3_dot(d,x) >= 0.0f ? 1.0f : -1.0f, ky = V3_dot(d,y) >= 0.0f ? 1.0f : -1.0f, kz = V3_dot(d,z) >= 0.0f ? 1.0f : -1.0f; return V3_AplusB(V3_AplusB(V3_AplusB(b.ctr,V3_ScaleByF(x,kx*b.hExt.x)),V3_ScaleByF(y,ky*b.hExt.y)),V3_ScaleByF(z,kz*b.hExt.z)); }
static inline V3 CapsuleSupport(ShapeCapsule cap, V3 d) { float db = V3_dot(cap.base,d),dt = V3_dot(cap.tip,d); V3 best = (dt > db) ? cap.tip : cap.base; float L = V3_dot(d,d); if (L < PHY_EPSILON) {return best;} return V3_AplusB(best,V3_ScaleByF(d,cap.rad / vsqrtf(L))); }
static inline V3 HullSupport(const HullCache* h, u16 m, const float* M, V3 d) { return h->ok ? CachedSupport(h->v,h->n,d) : MeshSupport(m,M,d); }
V3 GJKSupport(u16 prim, V3 d) {
    if (World.collider[prim] == COLTYPE_SPH) return SphSupport(Entity_GetSph(prim),d);
    if (World.collider[prim] == COLTYPE_BOX) return BoxSupport(Entity_GetBox(prim),d);
    return CapsuleSupport(Entity_GetCap(prim),d);
}

static bool GJKNextSimplex(Simplex3D *s, V3* dir) {
    V3 A = s->v[s->n-1], AO = {-A.x, -A.y, -A.z}; V3 wAA = s->wA[s->n-1], wBA = s->wB[s->n-1];
    #define SET_A(i) do{ s->v[i]=A; s->wA[i]=wAA; s->wB[i]=wBA; } while(0)
    #define CPY(d,src) do{ s->v[d]=s->v[src]; s->wA[d]=s->wA[src]; s->wB[d]=s->wB[src]; } while(0)
    #define SWP(i,j) do{ V3 tv=s->v[i]; s->v[i]=s->v[j]; s->v[j]=tv; V3 ta=s->wA[i]; s->wA[i]=s->wA[j]; s->wA[j]=ta; V3 tb=s->wB[i]; s->wB[i]=s->wB[j]; s->wB[j]=tb; } while(0)
    if (s->n == 2) {
        V3 AB = V3_AsubB(s->v[0], A); if (AB.x + AB.y + AB.z < PHY_EPSILON) AB = V3_AplusB(AB,V3_ScaleByF(*dir,0.001f));
        if (V3_dot(AB, AO) > 0.f) *dir = TP(AB, AO, AB); else { s->n = 1; SET_A(0); *dir = AO; }
        if (V3_dot(*dir, *dir) < PHY_EPSILON) { V3 px = (vabs(AB.x) > 0.9f) ? (V3){0,1,0} : (V3){1,0,0}; *dir = V3_Cross(AB, px); }
        return true;
    }
    if (s->n == 3) {
        V3 B = s->v[1], C = s->v[0], AB = V3_AsubB(B, A), AC = V3_AsubB(C, A), ABC = V3_Cross(AB, AC);
        if (V3_dot(V3_Cross(ABC, AC), AO) > 0.f) { if (V3_dot(AC, AO) > 0.f) { SET_A(1); s->n = 2; *dir = TP(AC, AO, AC); } else goto line_AB3; }
        else if (V3_dot(V3_Cross(AB, ABC), AO) > 0.f) { line_AB3: if (V3_dot(AB, AO) > 0.f) { CPY(0, 1); SET_A(1); s->n = 2; *dir = TP(AB, AO, AB); } else { SET_A(0); s->n = 1; *dir = AO; } }
        else { if (V3_dot(ABC, AO) > 0.f) { *dir = ABC; } else { SWP(0, 1); *dir = (V3){-ABC.x, -ABC.y, -ABC.z}; } }
        return true;
    }
    V3 B = s->v[2], C = s->v[1], D = s->v[0], AB = V3_AsubB(B, A), AC = V3_AsubB(C, A), AD = V3_AsubB(D, A);
    V3 nABC = V3_Cross(AB, AC), nACD = V3_Cross(AC, AD), nADB = V3_Cross(AD, AB);
    nABC = V3_dot(nABC, AD) > 0.f ? (V3){-nABC.x, -nABC.y, -nABC.z} : nABC;
    nACD = V3_dot(nACD, AB) > 0.f ? (V3){-nACD.x, -nACD.y, -nACD.z} : nACD;
    nADB = V3_dot(nADB, AC) > 0.f ? (V3){-nADB.x, -nADB.y, -nADB.z} : nADB;
    if (V3_dot(nABC, AO) > 0.f) { CPY(0, 1); CPY(1, 2); SET_A(2); s->n = 3; *dir = nABC; return true; }
    if (V3_dot(nACD, AO) > 0.f) { SET_A(2); s->n = 3; *dir = nACD; return true; }
    if (V3_dot(nADB, AO) > 0.f) { CPY(1, 0); CPY(0, 2); SET_A(2); s->n = 3; *dir = nADB; return true; }
    return false;
    #undef SET_A
    #undef CPY
    #undef SWP
}

#define EPA_MAX_FACES 64
#define EPA_MAX_VERTS 128
#define EPA_MAX_EDGES (EPA_MAX_FACES*3)
typedef struct { int a,b,c; V3 n; float d; } EPAFace;
typedef struct { V3 v, wA, wB; } EPAVert;
static inline EPAFace MakeEPAFace(const EPAVert* vb, int a, int b, int c) { V3 n = V3_Cross(V3_AsubB(vb[b].v,vb[a].v),V3_AsubB(vb[c].v,vb[a].v)); float L = V3_Mag(n); if(L < PHY_EPSILON){return (EPAFace){a,b,c,{0},-1.f};} n = V3_ScaleByF(n,1.f/L); float d = V3_dot(n,vb[a].v); if(d < 0.f){n=(V3){-n.x,-n.y,-n.z}; d=-d; int t=b;b=c;c=t;} return (EPAFace){a,b,c,n,d}; }
static inline V3 EPAContactPoint(const EPAVert* ev, int a, int b, int c) { V3 pa=ev[a].v, pb=ev[b].v, pc=ev[c].v; V3 v0=V3_AsubB(pb,pa), v1=V3_AsubB(pc,pa), v2=V3_AsubB((V3){0,0,0},pa); float d00 = V3_dot(v0,v0), d01 = V3_dot(v0,v1), d11 = V3_dot(v1,v1), d20 = V3_dot(v2,v0), d21 = V3_dot(v2,v1); float denom = d00*d11 - d01*d01 + PHY_EPSILON; float v = vmax((d11*d20 - d01*d21)*(1.0f/denom),0.0f), w = vmax((d00*d21 - d01*d20)*(1.0f/denom),0.0f), u = vmax(1.0f - v - w,0.0f); float sum = u + v + w; if (sum > PHY_EPSILON) {u /= sum; v /= sum; w /= sum;} return (V3){u*ev[a].wA.x + v*ev[b].wA.x + w*ev[c].wA.x,u*ev[a].wA.y + v*ev[b].wA.y + w*ev[c].wA.y,u*ev[a].wA.z + v*ev[b].wA.z + w*ev[c].wA.z}; }
static inline Manifold MakeEPAManifold(const EPAVert* ev, int a, int b, int c, V3 n, float d) { Manifold m = {0}; m.normal = n; m.maxPen = d; m.n = 1; m.p[0] = (ManifoldPt){ EPAContactPoint(ev,a,b,c), d }; return m; }
static void inline FeatureOverlap(V3 sc, float sr, V3 pt, Overlap* r) { V3 delta=V3_AsubB(sc,pt); float dist2=V3_dot(delta,delta); if (dist2 < sr*sr) { float dist=vsqrtf(vmax(dist2,0.0f)); Overlap t={true,pt,(dist>PHY_EPSILON) ? V3_ScaleByF(delta,1.0f/dist) : (V3){0.0f,1.0f,0.0f},sr - dist}; if(t.pen>r->pen) *r=t; } }
static inline void SphTriTest(V3 sc, float sr, u16 mesh, u32 ti, const float* mx, Overlap* r) {
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

#define MANIFOLD_EPA_SEED EPAVert ev[EPA_MAX_VERTS]; EPAFace ef[EPA_MAX_FACES]; int nv=0, nf=0; for (int i = 0; i < 4; i++) { ev[nv].wA = s.wA[i]; ev[nv].wB = s.wB[i]; ev[nv].v = s.v[i]; nv++; } static const int kTetFaces[4][3] = {{0,1,2}, {0,3,1}, {0,2,3}, {1,3,2}};
#define EPA_EXPAND() { \
    if (nv>=EPA_MAX_VERTS) break; ev[nv].v=sup; ev[nv].wA=wA; ev[nv].wB=wB; int edges[EPA_MAX_EDGES][2],ne=0,keep[EPA_MAX_FACES],nk=0; \
    for (int f=0;f<nf;f++) { \
        if (V3_dot(ef[f].n,V3_AsubB(sup,ev[ef[f].a].v))>0.f) { \
            int fv[3]={ef[f].a,ef[f].b,ef[f].c}; \
            for (int e=0;e<3;e++) { int ea=fv[e],eb=fv[(e+1)%3]; bool found=false; \
                for (int k=0;k<ne;k++) if(edges[k][0]==eb&&edges[k][1]==ea){edges[k][0]=edges[--ne][0];edges[k][1]=edges[ne][1];found=true;break;} \
                if (!found&&ne<EPA_MAX_EDGES){edges[ne][0]=ea;edges[ne++][1]=eb;} } \
        } else keep[nk++]=f; \
    } \
    nf=0; for(int k=0;k<nk;k++) ef[nf++]=ef[keep[k]]; \
    for(int k=0;k<ne&&nf<EPA_MAX_FACES;k++){EPAFace face=MakeEPAFace(ev,edges[k][0],edges[k][1],nv); if(face.d>=0.f) ef[nf++]=face;} \
    nv++; \
}

#define GJK_INIT(supA_expr, supB_expr) V3 dir={0,1,0}; V3 wA=(supA_expr), wB=(supB_expr); s.wA[s.n]=wA; s.wB[s.n]=wB; s.v[s.n++]=V3_AsubB(wA,wB); dir=(V3){-s.v[0].x,-s.v[0].y,-s.v[0].z}; if(V3_dot(dir,dir)<PHY_EPSILON) dir=(V3){0,1,0}; bool hit=false
#define GJK_LOOP(supA_expr, supB_expr, max_iter, on_fail) for (int it=0; it<(max_iter); ++it) { wA=(supA_expr); wB=(supB_expr); V3 sup=V3_AsubB(wA,wB); if (V3_dot(sup,dir)<0) { on_fail; } s.wA[s.n]=wA; s.wB[s.n]=wB; s.v[s.n++]=sup; if (!GJKNextSimplex(&s,&dir)) { hit=true; break; } }
#define GJK_FALLBACK(supA_expr, supB_expr) \
    static const V3 _kAx[6]={{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}}; \
    for (int d=0; s.n<4 && d<6; ++d) { wA=(supA_expr); wB=(supB_expr); V3 sup=V3_AsubB(wA,wB); bool dup=false; \
        for (int k=0;k<s.n;k++){V3 dv=V3_AsubB(sup,s.v[k]); dup|=(V3_dot(dv,dv)<PHY_EPSILON*PHY_EPSILON);} \
        if (!dup) { s.wA[s.n]=wA; s.wB[s.n]=wB; s.v[s.n++]=sup; } \
    }

#define BVH_WALK(mesh, mx, cull, retVal, ...) \
    if (BvhHasBVH(mesh)) { \
        const BvhNode* _nodes=modelBVHNodes[mesh]; const u16* _triOrder=modelBVHTriOrder[mesh]; const BvhNode* _stack[64]; int _sp=0; _stack[_sp++]=&_nodes[0]; \
        while (_sp>0) { const BvhNode* _node=_stack[--_sp]; V3 _wMn,_wMx; BvhNodeWorldAABB(_node,mx,&_wMn,&_wMx); if (!(cull)) continue; \
            if (_node->triCount>0) { for (u32 _i=0;_i<_node->triCount;_i++) { u32 ti=_triOrder[_node->triStart+_i]; __VA_ARGS__; } } \
            else { for (int _o=0;_o<8;_o++) if (_node->children[_o]>=0) _stack[_sp++]=&_nodes[_node->children[_o]]; } } \
        return retVal; \
    }

static Overlap SphMsh(V3 sc, float sr, u16 m, const float* mx) { Overlap r={0}; if(m>=MAX_MDLS)return r; u32 tc=modelTriangleCounts[m]; if(!tc)return r; BVH_WALK(m,mx,BvhSphereAABBOverlap(sc,sr,_wMn,_wMx),r,SphTriTest(sc,sr,m,ti,mx,&r)); for(u32 ti=0;ti<tc;++ti) SphTriTest(sc,sr,m,ti,mx,&r); return r; }
static Overlap CapMsh(ShapeCapsule c, u16 m, const float* mx) { Overlap best=SphMsh(c.base,c.rad,m,mx), rt=SphMsh(c.tip,c.rad,m,mx); if(rt.pen>best.pen)best=rt; V3 d=V3_AsubB(c.tip,c.base); if(V3_Mag(d)>PHY_EPSILON){for(int k=1;k<6;++k){float t=(float)k/5.0f; Overlap rm=SphMsh(V3_AplusB(c.base,V3_ScaleByF(d,t)),c.rad,m,mx); if(rm.pen>best.pen)best=rm;}} return best; }
static Manifold CvxCvx(u16 meshA, u16 meshB, const float* matA, const float* matB) {
    Manifold m={0}; if(meshA>=MAX_MDLS||meshB>=MAX_MDLS)return m; HullCache hcA=CacheHull(meshA,matA), hcB=CacheHull(meshB,matB);
    #define MSUP_A(d) HullSupport(&hcA,meshA,matA,(d))
    #define MSUP_B(d) HullSupport(&hcB,meshB,matB,(V3){-(d).x,-(d).y,-(d).z})
    Simplex3D s={0}; GJK_INIT(MSUP_A(dir),MSUP_B(dir)); GJK_LOOP(MSUP_A(dir),MSUP_B(dir),64,return m); if(!hit)return m;
    GJK_FALLBACK(MSUP_A(_kAx[d]),MSUP_B(_kAx[d])); if(s.n<4)return m; MANIFOLD_EPA_SEED
    for(int f=0;f<4;f++){EPAFace face=MakeEPAFace(ev,kTetFaces[f][0],kTetFaces[f][1],kTetFaces[f][2]); if(face.d>=0.f&&nf<EPA_MAX_FACES)ef[nf++]=face;}
    for(int it=0;it<32;++it) {
        int bf=-1; float bd=1e9f; for(int f=0;f<nf;f++)if(ef[f].d<bd){bd=ef[f].d;bf=f;} if(bf<0)break;
        V3 bn=ef[bf].n; wA=MSUP_A(bn); wB=MSUP_B(bn); V3 sup=V3_AsubB(wA,wB);
        if (V3_dot(bn,sup)-bd<PHY_EPSILON) {
            m.normal=bn; m.maxPen=bd; m.n=1; V3 deepPoint=EPAContactPoint(ev,ef[bf].a,ef[bf].b,ef[bf].c); m.p[0]=(ManifoldPt){deepPoint,bd};
            if (hcB.ok) {
                float planeDist=V3_dot(bn,deepPoint), wscaleB=V3_Mag((V3){matB[0],matB[1],matB[2]}), thicknessTolerance=vclamp(modelBounds[meshB]*wscaleB*0.06f,0.003f,0.02f);
                for(u32 i=0;i<hcB.n;++i) {
                    V3 pt=hcB.v[i]; float distToPlane=V3_dot(bn,pt)-planeDist;
                    if (vabs(distToPlane)<thicknessTolerance) { float ptPen=bd-distToPlane; if(ptPen>0.0f) { bool isDup=false; for(int k=0;k<m.n;++k){V3 diff=V3_AsubB(pt,m.p[k].point); if(V3_dot(diff,diff)<0.00001f){isDup=true;break;}} if(!isDup&&m.n<MANIFOLD_MAX){m.p[m.n++]=(ManifoldPt){pt,ptPen};} if(m.n>=MANIFOLD_MAX)break; } }
                }
            }
            return m;
        }
        EPA_EXPAND();
    }
    #undef MSUP_A
    #undef MSUP_B
    return m;
}

static Manifold PrimitiveCvx(u16 prim, u16 mesh, const float* mx) {
    Manifold m={0}; if(mesh>=MAX_MDLS||!modelVertexCounts[mesh])return m; HullCache hc=CacheHull(mesh,mx);
    #define PSUP_A(d) GJKSupport(prim,(d))
    #define PSUP_B(d) HullSupport(&hc,mesh,mx,(V3){-(d).x,-(d).y,-(d).z})
    Simplex3D s={0}; GJK_INIT(PSUP_A(dir),PSUP_B(dir)); GJK_LOOP(PSUP_A(dir),PSUP_B(dir),64,return m); if(!hit)return m;
    GJK_FALLBACK(PSUP_A(_kAx[d]),PSUP_B(_kAx[d])); if(s.n<4)return m; MANIFOLD_EPA_SEED
    for(int f=0;f<4;f++){EPAFace face=MakeEPAFace(ev,kTetFaces[f][0],kTetFaces[f][1],kTetFaces[f][2]);if(face.d>=0.f&&nf<EPA_MAX_FACES)ef[nf++]=face;}
    for(int it=0;it<32;++it){
        int bf=-1; float bd=1e9f; for(int f=0;f<nf;f++)if(ef[f].d<bd){bd=ef[f].d;bf=f;} if(bf<0)break;
        V3 bn=ef[bf].n; wA=PSUP_A(bn); wB=PSUP_B(bn); V3 sup=V3_AsubB(wA,wB);
        if(V3_dot(bn,sup)-bd<PHY_EPSILON)return MakeEPAManifold(ev,ef[bf].a,ef[bf].b,ef[bf].c,bn,bd);
        EPA_EXPAND();
    }
    #undef PSUP_A
    #undef PSUP_B
    return m;
}

static inline V3 TriSupport(V3 ta, V3 tb, V3 tc, V3 d) { float d1=V3_dot(ta,d), d2=V3_dot(tb,d), d3=V3_dot(tc,d); return d1>d2 ? (d1>d3 ? ta : tc) : (d2>d3 ? tb : tc); }
typedef struct {V3 mn,mx;} AABB3; typedef struct { HullCache hc; u16 hullMesh; const float* hullMx; AABB3 hb; float spreadEps,thicknessTolerance; Manifold best; } CvxMshCtx;
static void CvxTriTest(CvxMshCtx* ctx, V3 ta, V3 tb, V3 tc) {
    HullCache* hc=&ctx->hc; u16 hullMesh=ctx->hullMesh; const float* hullMx=ctx->hullMx; Manifold* best=&ctx->best; float spreadEps=ctx->spreadEps, thicknessTolerance=ctx->thicknessTolerance;
    if (vmin(ta.x,vmin(tb.x,tc.x))>ctx->hb.mx.x || vmax(ta.x,vmax(tb.x,tc.x))<ctx->hb.mn.x || vmin(ta.y,vmin(tb.y,tc.y))>ctx->hb.mx.y || vmax(ta.y,vmax(tb.y,tc.y))<ctx->hb.mn.y || vmin(ta.z,vmin(tb.z,tc.z))>ctx->hb.mx.z || vmax(ta.z,vmax(tb.z,tc.z))<ctx->hb.mn.z) return;
    #define HSUP(d) HullSupport(hc,hullMesh,hullMx,(d))
    #define TSUP(d) TriSupport(ta,tb,tc,(V3){-(d).x,-(d).y,-(d).z})
    Simplex3D s={0}; GJK_INIT(HSUP(dir),TSUP(dir)); GJK_LOOP(HSUP(dir),TSUP(dir),32,break); if(!hit)return;
    while (s.n<4) {
        V3 fallbackDir={0.0f,1.0f,0.0f};
        if(s.n==1) fallbackDir=(vabs(s.v[0].x)>0.5f)?(V3){0.0f,1.0f,0.0f}:(V3){1.0f,0.0f,0.0f};
        else if(s.n==2){V3 edge=V3_AsubB(s.v[1],s.v[0]); fallbackDir=V3_Cross(edge,(vabs(edge.x)>0.5f)?(V3){0.0f,1.0f,0.0f}:(V3){1.0f,0.0f,0.0f});}
        else if(s.n==3){V3 e1=V3_AsubB(s.v[1],s.v[0]), e2=V3_AsubB(s.v[2],s.v[0]); fallbackDir=V3_Cross(e1,e2);}
        float fLen=V3_Mag(fallbackDir); fallbackDir=(fLen>PHY_EPSILON)?V3_ScaleByF(fallbackDir,1.0f/fLen):(V3){0.0f,1.0f,0.0f};
        wA=HSUP(fallbackDir); wB=TSUP(fallbackDir); V3 sup=V3_AsubB(wA,wB); bool dup=false;
        for (int k=0;k<s.n;k++){V3 dv=V3_AsubB(sup,s.v[k]); dup|=(V3_dot(dv,dv)<PHY_EPSILON*PHY_EPSILON);}
        if (!dup){s.wA[s.n]=wA; s.wB[s.n]=wB; s.v[s.n++]=sup;} else {fallbackDir=(V3){-fallbackDir.x,-fallbackDir.y,-fallbackDir.z}; wA=HSUP(fallbackDir); wB=TSUP(fallbackDir); s.wA[s.n]=wA; s.wB[s.n]=wB; s.v[s.n++]=V3_AsubB(wA,wB);}
    }
    MANIFOLD_EPA_SEED
    for (int f=0;f<4;f++){EPAFace fc=MakeEPAFace(ev,kTetFaces[f][0],kTetFaces[f][1],kTetFaces[f][2]); if(fc.d>=0.f&&nf<EPA_MAX_FACES)ef[nf++]=fc;}
    if (nf<4) return; bool tHit=false; V3 tN={0}; float tD=0; V3 tP={0};
    for (int it=0;it<32;++it){
        int bf=-1; float bd=1e9f; for (int f=0;f<nf;f++)if(ef[f].d<bd){bd=ef[f].d;bf=f;} if(bf<0)break;
        V3 bn=ef[bf].n; wA=HSUP(bn); wB=TSUP(bn); V3 sup=V3_AsubB(wA,wB);
        if(V3_dot(bn,sup)-bd<PHY_EPSILON){V3 triN = V3_Normalize(V3_Cross(V3_AsubB(tb,ta),V3_AsubB(tc,ta))); if (V3_dot(bn,triN) < 0.0f) { bn = triN;/*Prevent thin object penetrations when walked on by ensuring triangles are treated as one-sided from tri normal*/} tHit=true; tN=bn; tD=bd; tP=EPAContactPoint(ev,ef[bf].a,ef[bf].b,ef[bf].c); break;}
        EPA_EXPAND();
    }
    if (tHit) {
        V3 deepPoint=tP;
        if (!best->n) { best->normal=tN; best->maxPen=tD; best->p[best->n++]=(ManifoldPt){deepPoint,tD}; }
        else {
            float align=V3_dot(tN,best->normal);
            if (align>MANIFOLD_ALIGN_THRESHOLD) {
                bool better=(tD>best->maxPen+MANIFOLD_TIE_MARGIN) || (vabs(tD-best->maxPen)<=MANIFOLD_TIE_MARGIN && V3_dot(tN,(V3){0,1,0})>V3_dot(best->normal,(V3){0,1,0}));
                if (better){best->normal=tN; best->maxPen=tD;} bool spread=true;
                for (int k=0;k<best->n;++k){V3 dv=V3_AsubB(deepPoint,best->p[k].point); if(V3_dot(dv,dv)<spreadEps*spreadEps){spread=false; if(tD>best->p[k].pen)best->p[k].pen=tD; break;}}
                if (spread&&best->n<MANIFOLD_MAX)best->p[best->n++]=(ManifoldPt){deepPoint,tD};
            } else if (tD>best->maxPen+MANIFOLD_TIE_MARGIN){best->n=0; best->normal=tN; best->maxPen=tD; best->p[best->n++]=(ManifoldPt){deepPoint,tD};}
        }
        if (hc->ok && best->n>0 && V3_dot(tN,best->normal)>MANIFOLD_ALIGN_THRESHOLD) {
            float planeDist=V3_dot(tN,deepPoint); V3 v0=V3_AsubB(tb,ta), v1=V3_AsubB(tc,ta); float d00=V3_dot(v0,v0), d01=V3_dot(v0,v1), d11=V3_dot(v1,v1), denom=d00*d11-d01*d01; bool validTri=vabs(denom)>PHY_EPSILON;
            for (u32 i=0;i<hc->n;++i) {
                V3 pt=hc->v[i]; float distToPlane=V3_dot(tN,pt)-planeDist;
                if (vabs(distToPlane)<thicknessTolerance) {
                    bool insideTri=false;
                    if (validTri){V3 projPt=V3_AsubB(pt,V3_ScaleByF(tN,distToPlane)), v2=V3_AsubB(projPt,ta); float d20=V3_dot(v2,v0), d21=V3_dot(v2,v1), v=(d11*d20-d01*d21)/denom, w=(d00*d21-d01*d20)/denom, u=1.0f-v-w; if(u>=-0.02f&&v>=-0.02f&&w>=-0.02f)insideTri=true;}
                    if (insideTri){float ptPen=tD-distToPlane; if(ptPen>0.0f){bool isDup=false; for(int k=0;k<best->n;++k){V3 diff=V3_AsubB(pt,best->p[k].point); if(V3_dot(diff,diff)<spreadEps*spreadEps){isDup=true;break;}} if(!isDup&&best->n<MANIFOLD_MAX)best->p[best->n++]=(ManifoldPt){pt,ptPen}; if(best->n>=MANIFOLD_MAX)break;}}
                }
            }
        }
    }
    #undef HSUP
    #undef TSUP
}

static Manifold CvxMsh(u16 hullMesh, const float* hullMx, u16 triMesh, const float* triMx) {
    Manifold bestZero={0}; if(hullMesh>=MAX_MDLS||triMesh>=MAX_MDLS)return bestZero;
    CvxMshCtx ctx; ctx.best=bestZero; ctx.hullMesh=hullMesh; ctx.hullMx=hullMx; ctx.hc=CacheHull(hullMesh,hullMx); if(!ctx.hc.n)return ctx.best;
    AABB3 hb={{1e9f,1e9f,1e9f},{-1e9f,-1e9f,-1e9f}};
    if (ctx.hc.ok) { for (u32 i=0;i<ctx.hc.n;++i) { V3 w=ctx.hc.v[i]; hb.mn.x=vmin(hb.mn.x,w.x); hb.mn.y=vmin(hb.mn.y,w.y); hb.mn.z=vmin(hb.mn.z,w.z); hb.mx.x=vmax(hb.mx.x,w.x); hb.mx.y=vmax(hb.mx.y,w.y); hb.mx.z=vmax(hb.mx.z,w.z); } }
    else { for (u32 i=0;i<ctx.hc.n;++i) { V3 w=MvVert(hullMx,MeshVert(hullMesh,i)); hb.mn.x=vmin(hb.mn.x,w.x); hb.mn.y=vmin(hb.mn.y,w.y); hb.mn.z=vmin(hb.mn.z,w.z); hb.mx.x=vmax(hb.mx.x,w.x); hb.mx.y=vmax(hb.mx.y,w.y); hb.mx.z=vmax(hb.mx.z,w.z); } }
    ctx.hb=hb; V3 hext=V3_AsubB(hb.mx,hb.mn); ctx.spreadEps=vmax(0.02f,vmax(hext.x,vmax(hext.y,hext.z))*0.15f);
    float wscaleH=V3_Mag((V3){hullMx[0],hullMx[1],hullMx[2]}); ctx.thicknessTolerance=vclamp(modelBounds[hullMesh]*wscaleH*0.06f,0.003f,0.02f);
    u32 triCount=modelTriangleCounts[triMesh]; if(!triCount)return ctx.best;
    BVH_WALK(triMesh,triMx,BvhAABBOverlap(_wMn,_wMx,hb.mn,hb.mx),ctx.best,{V3 ta,tb,tc; MeshTri(triMesh,ti,triMx,&ta,&tb,&tc); CvxTriTest(&ctx,ta,tb,tc);});
    for (u32 ti=0;ti<triCount;++ti) { V3 ta,tb,tc; MeshTri(triMesh,ti,triMx,&ta,&tb,&tc); CvxTriTest(&ctx,ta,tb,tc); }
    return ctx.best;
}

static Manifold BoxMsh(ShapeBox b,u16 m,const float* mx){
    CvxMshCtx c={0};if(m>=MAX_MDLS||!modelTriangleCounts[m])return c.best; c.hullMesh=m; c.hullMx=mx; c.hc.n=8; c.hc.ok=true; V3 a[3]; float h[3]={b.hExt.x,b.hExt.y,b.hExt.z}; obb_axes(b.rot,a,a+1,a+2); AABB3 q={{1e9f,1e9f,1e9f},{-1e9f,-1e9f,-1e9f}};
    for(int i=0;i<8;i++){V3 p=b.ctr;for(int k=0;k<3;k++)p=V3_AplusB(p,V3_ScaleByF(a[k],((i>>k)&1?1.f:-1.f)*h[k])); c.hc.v[i]=p; q.mn.x=vmin(q.mn.x,p.x); q.mn.y=vmin(q.mn.y,p.y); q.mn.z=vmin(q.mn.z,p.z); q.mx.x=vmax(q.mx.x,p.x); q.mx.y=vmax(q.mx.y,p.y); q.mx.z=vmax(q.mx.z,p.z);}
    V3 e=V3_AsubB(q.mx,q.mn),skin={.01f,.01f,.01f}; c.spreadEps=vmax(.02f,vmax(e.x,vmax(e.y,e.z))*.15f); c.thicknessTolerance=vclamp(V3_Mag(b.hExt)*.06f,.003f,.02f); c.hb=(AABB3){V3_AsubB(q.mn,skin),V3_AplusB(q.mx,skin)};
    BVH_WALK(m,mx,BvhAABBOverlap(_wMn,_wMx,c.hb.mn,c.hb.mx),((c.best.normal=V3_ScaleByF(c.best.normal,-1.f)),c.best),{V3 x,y,z;MeshTri(m,ti,mx,&x,&y,&z);CvxTriTest(&c,x,y,z);});
    for(u32 ti=0;ti<modelTriangleCounts[m];ti++){V3 x,y,z;MeshTri(m,ti,mx,&x,&y,&z);CvxTriTest(&c,x,y,z);} c.best.normal=V3_ScaleByF(c.best.normal,-1.f); return c.best;
}

static Overlap CapBox(ShapeCapsule c,ShapeBox b){ V3 d=V3_AsubB(c.tip,c.base); Overlap best=SphBox(c.base,c.rad,b), r=SphBox(c.tip,c.rad,b); if(r.pen>best.pen)best=r; if(V3_dot(d,d)>PHY_EPSILON*PHY_EPSILON)for(int k=1;k<8;k++){r=SphBox(V3_AplusB(c.base,V3_ScaleByF(d,k*.125f)),c.rad,b);if(r.pen>best.pen)best=r;} return best; }
static inline void quat_to_mat3(Quaternion q, float R[3][3]) {
    float x=q.x,y=q.y,z=q.z,w=q.w, xx=x*x,yy=y*y,zz=z*z, xy=x*y,xz=x*z,yz=y*z, wx=w*x,wy=w*y,wz=w*z;
    R[0][0]=1.0f-2.0f*(yy+zz); R[0][1]=2.0f*(xy-wz); R[0][2]=2.0f*(xz+wy); R[1][0]=2.0f*(xy+wz); R[1][1]=1.0f-2.0f*(xx+zz); R[1][2]=2.0f*(yz-wx); R[2][0]=2.0f*(xz-wy); R[2][1]=2.0f*(yz+wx); R[2][2]=1.0f-2.0f*(xx+yy);
}

static inline V3 ApplyInvTensor(u16 i, V3 v) {
    if (World.collider[i] == COLTYPE_BOX) {
        ShapeBox b = Entity_GetBox(i); float m = World.mass[i], hx = b.hExt.x, hy = b.hExt.y, hz = b.hExt.z;
        float Ixx=(1.0f/3.0f)*m*(hy*hy+hz*hz), Iyy=(1.0f/3.0f)*m*(hx*hx+hz*hz), Izz=(1.0f/3.0f)*m*(hx*hx+hy*hy), invIxx=1.0f/vmax(Ixx,1e-6f), invIyy=1.0f/vmax(Iyy,1e-6f), invIzz=1.0f/vmax(Izz,1e-6f);
        float R[3][3]; quat_to_mat3(World.rotation[i],R); float bx=R[0][0]*v.x+R[1][0]*v.y+R[2][0]*v.z, by=R[0][1]*v.x+R[1][1]*v.y+R[2][1]*v.z, bz=R[0][2]*v.x+R[1][2]*v.y+R[2][2]*v.z;
        float wx=invIxx*bx, wy=invIyy*by, wz=invIzz*bz; return (V3){R[0][0]*wx+R[0][1]*wy+R[0][2]*wz, R[1][0]*wx+R[1][1]*wy+R[1][2]*wz, R[2][0]*wx+R[2][1]*wy+R[2][2]*wz};
    }
    if (World.collider[i] != COLTYPE_CVX || !World.invTnsrValid[i]) { float r=(World.collider[i]==COLTYPE_MSH)?modelBounds[World.instances[i].modelIndex]:((World.collider[i]==COLTYPE_CVX)?modelBounds[World.instances[i].colMeshIndex]:GetColRad(i)); return V3_ScaleByF(v,1.0f/vmax((2.0f/5.0f)*World.mass[i]*r*r,0.0f)); }
    float R[3][3]; quat_to_mat3(World.rotation[i],R); float *I=World.invInertiaTensor[i];
    float bx=R[0][0]*v.x+R[1][0]*v.y+R[2][0]*v.z, by=R[0][1]*v.x+R[1][1]*v.y+R[2][1]*v.z, bz=R[0][2]*v.x+R[1][2]*v.y+R[2][2]*v.z;
    float wx=I[0]*bx+I[3]*by+I[4]*bz, wy=I[3]*bx+I[1]*by+I[5]*bz, wz=I[4]*bx+I[5]*by+I[2]*bz; return (V3){R[0][0]*wx+R[0][1]*wy+R[0][2]*wz, R[1][0]*wx+R[1][1]*wy+R[1][2]*wz, R[2][0]*wx+R[2][1]*wy+R[2][2]*wz};
}

static void ResolveContactVelocity(u16 a, u16 b, V3 n, V3 rAarm, V3 rBarm, float targetVn, float *accumN, float* accumT, bool bStatic, float invMassA, float invMassB, float invSumN, bool canRotateA, bool canRotateB) {
    if (invSumN < PHY_EPSILON) return;
    V3 vAtA = V3_AplusB(World.velocity[a],V3_Cross(World.angularVelocity[a],rAarm)), vAtB = bStatic ? (V3){0,0,0} : V3_AplusB(World.velocity[b],V3_Cross(World.angularVelocity[b],rBarm));
    float vn = V3_dot(V3_AsubB(vAtA,vAtB),n), j = (targetVn - vn) / invSumN, newAccumN = vmax(*accumN + j, 0.0f); j = newAccumN - *accumN; *accumN = newAccumN;
    V3 impulse = V3_ScaleByF(n,j); World.velocity[a] = V3_AplusB(World.velocity[a],V3_ScaleByF(impulse,invMassA));
    if (!bStatic) World.velocity[b] = V3_AsubB(World.velocity[b],V3_ScaleByF(impulse,invMassB));
    if (canRotateA) World.angularVelocity[a] = V3_AplusB(World.angularVelocity[a],ApplyInvTensor(a,V3_Cross(rAarm,impulse)));
    if (canRotateB) World.angularVelocity[b] = V3_AsubB(World.angularVelocity[b],ApplyInvTensor(b,V3_Cross(rBarm,impulse)));
    V3 vAtA2 = V3_AplusB(World.velocity[a],V3_Cross(World.angularVelocity[a],rAarm)), vAtB2 = bStatic ? (V3){0,0,0} : V3_AplusB(World.velocity[b],V3_Cross(World.angularVelocity[b],rBarm));
    V3 relVel2 = V3_AsubB(vAtA2,vAtB2), tangent = V3_AsubB(relVel2,V3_ScaleByF(n,V3_dot(relVel2,n))); float tLen = V3_Mag(tangent);
    if (tLen > 0.0001f) {
        tangent = V3_ScaleByF(tangent,1.0f/tLen); V3 rAxT = V3_Cross(rAarm,tangent), rBxT = V3_Cross(rBarm,tangent);
        float angTermAT = canRotateA ? V3_dot(rAxT,ApplyInvTensor(a,rAxT)) : 0.0f, angTermBT = canRotateB ? V3_dot(rBxT,ApplyInvTensor(b,rBxT)) : 0.0f, invSumT = invMassA + invMassB + angTermAT + angTermBT;
        if (invSumT > PHY_EPSILON) {
            float jt = -V3_dot(relVel2,tangent) / invSumT, friction; bool aIsSpecial = (World.collider[a] == COLTYPE_CAP && (a == PLAYER1 || IdxIsNPC(World.instances[a].index)));
            if (bStatic && aIsSpecial) { friction = 0.001f; } else { float mix = vclamp((tLen - 0.05f) / 0.10f, 0.0f, 1.0f); friction = FRICTION_ROLL + mix * (FRICTION_SLIDE - FRICTION_ROLL); }
            float maxT = friction * (*accumN), newAccumT = vclamp(*accumT + jt, -maxT, maxT); jt = newAccumT - *accumT; *accumT = newAccumT;
            V3 fImpulse = V3_ScaleByF(tangent,jt); World.velocity[a] = V3_AplusB(World.velocity[a],V3_ScaleByF(fImpulse,invMassA));
            if (!bStatic) World.velocity[b] = V3_AsubB(World.velocity[b],V3_ScaleByF(fImpulse,invMassB));
            if (canRotateA) World.angularVelocity[a] = V3_AplusB(World.angularVelocity[a],ApplyInvTensor(a,V3_Cross(rAarm,fImpulse)));
            if (canRotateB) World.angularVelocity[b] = V3_AsubB(World.angularVelocity[b],ApplyInvTensor(b,V3_Cross(rBarm,fImpulse)));
        }
    }
}

static void ApplyManifoldResponse(u16 a, u16 b, const Manifold *m) {
    if (!m->n || (World.collider[b] == COLTYPE_MSH && World.collider[a] == COLTYPE_MSH)) return;
    bool bStatic = (!(World.instances[b].entflags & EF_RIGIDBODY) || World.mass[b] < 0.001f || World.collider[b] == COLTYPE_NONE || World.collider[b] == COLTYPE_MSH);
    for (int i=0;i<m->n;++i) { if(m->p[i].pen > 0.0f){DrawSphereContact(m->p[i].point,0.02f);} }
    V3 rA[MANIFOLD_MAX], rB[MANIFOLD_MAX]; float targetVn[MANIFOLD_MAX], accumN[MANIFOLD_MAX]={0}, accumT[MANIFOLD_MAX]={0}, invSumN[MANIFOLD_MAX];
    float invMassA = World.mass[a] < 0.001f ? 1.0f : 1.0f / World.mass[a], invMassB = (bStatic || World.mass[b] < 0.001f) ? 0.0f : 1.0f / World.mass[b];
    bool canRotateA = (World.collider[a] != COLTYPE_CAP && !IdxIsNPC(World.instances[a].index)), canRotateB = (!bStatic && World.collider[b] != COLTYPE_CAP && !IdxIsNPC(World.instances[b].index));
    for (int i=0;i<m->n;++i) {
        rA[i] = V3_AsubB(m->p[i].point,World.position[a]); rB[i] = bStatic ? (V3){0,0,0} : V3_AsubB(m->p[i].point,World.position[b]);
        V3 vAtA = V3_AplusB(World.velocity[a],V3_Cross(World.angularVelocity[a],rA[i])), vAtB = bStatic ? (V3){0,0,0} : V3_AplusB(World.velocity[b],V3_Cross(World.angularVelocity[b],rB[i]));
        float vn0 = V3_dot(V3_AsubB(vAtA,vAtB),m->normal), e_r = (vn0 < -0.5f) ? vmax(World.bounciness[a],bStatic ? 0.0f : World.bounciness[b]) : 0.0f;
        targetVn[i] = (vn0 < -0.5f) ? -e_r * vn0 : 0.0f; V3 rAxN = V3_Cross(rA[i],m->normal), rBxN = V3_Cross(rB[i],m->normal);
        invSumN[i] = invMassA + invMassB + (canRotateA ? V3_dot(rAxN,ApplyInvTensor(a,rAxN)) : 0.0f) + (canRotateB ? V3_dot(rBxN,ApplyInvTensor(b,rBxN)) : 0.0f);
    }
    int iters = (m->n > 1) ? 8 : 1;
    for (int it=0;it<iters;++it) { for (int i=0;i<m->n;++i) ResolveContactVelocity(a,b,m->normal,rA[i],rB[i],targetVn[i],&accumN[i],&accumT[i],bStatic,invMassA,invMassB,invSumN[i],canRotateA,canRotateB); }
    float avgPen=0.0f; for (int i=0;i<m->n;++i) {avgPen += m->p[i].pen;} avgPen /= (float)m->n; float c = vmax(avgPen - 0.005f,0.0f) * 0.9f; // Baumgarte position correction.
    float massDiv = invMassA + invMassB + PHY_EPSILON;
    SetPosition(a,V3_AplusB(World.position[a],V3_ScaleByF(m->normal,c * invMassA / massDiv)),false); 
    if (!bStatic) SetPosition(b,V3_AsubB(World.position[b],V3_ScaleByF(m->normal,c * invMassB / massDiv)),false);
}

static void EntityColliderMatrixNow(u16 i, float M[16]) { // Convex meshes need to keep their matrix4x4 up to date.
    Quaternion q = World.rotation[i];
    V3 sx = V3_ScaleByF(quat_rot_v3(q,(V3){1,0,0}),World.scale[i].x);
    V3 sy = V3_ScaleByF(quat_rot_v3(q,(V3){0,1,0}),World.scale[i].y);
    V3 sz = V3_ScaleByF(quat_rot_v3(q,(V3){0,0,1}),World.scale[i].z);
    V3 p = World.position[i];
    M[0]=sx.x; M[1]=sx.y; M[2]=sx.z; M[3]=0.0f;
    M[4]=sy.x; M[5]=sy.y; M[6]=sy.z; M[7]=0.0f;
    M[8]=sz.x; M[9]=sz.y; M[10]=sz.z; M[11]=0.0f;
    M[12]=p.x; M[13]=p.y; M[14]=p.z; M[15]=1.0f;
}

static inline bool V3_IsSane(V3 v) { return (v.x<=1e6f && v.x>=-1e6f && v.y<=1e6f && v.y>=-1e6f && v.z<=1e6f && v.z>=-1e6f); }
void Physics(float dt) {
    u8 substeps = (u8)vclamp((u32)(dt / MAX_STEP_SIZE + 0.5f),1u,(u32)40); float dtsub = dt / (float)substeps; dynamicEntityCount = 0;
    for (u16 i=0;i<World.instCount && dynamicEntityCount < 512;++i) { if ((World.instances[i].entflags & EF_RIGIDBODY) && (World.instances[i].entflags & EF_ACTIVE) && World.collider[i] != COLTYPE_NONE) {dynamicEntities[dynamicEntityCount++] = i;} }
    for (u8 s=0;s<substeps;++s) {
        mset(cellCounts,0,sizeof(cellCounts));
        for (u16 i=0;i<World.instCount;++i) {
            posBudget[i] = 0.16f; World.instances[i].cellX=(i16)PosGetCellCoordX(World.position[i].x); World.instances[i].cellZ=(i16)PosGetCellCoordZ(World.position[i].z);
            World.instances[i].cellIndex=PosGetCellCoordsP(World.instances[i].cellX,World.instances[i].cellZ); World.radius[i] = GetColRad(i);
            u32 cell=(u32)World.instances[i].cellIndex; if(cell < WORLDX*WORLDX && cellCounts[cell] < 128){cellLists[cell][cellCounts[cell]++]=i;}
        }
        for (u16 i=0;i<dynamicEntityCount;++i) {
            u16 idx = dynamicEntities[i]; V3 acc = {0.0f,-9.81f * World.gravity[idx],0.0f}; if ((idx == PLAYER1) && Cheats.noclip) acc.y = 0.0f;
            acc = V3_AplusB(acc,V3_ScaleByF(World.instances[idx].accumulatedForce,1.0f / World.mass[idx])); World.velocity[idx] = V3_AplusB(World.velocity[idx],V3_ScaleByF(acc,dtsub));
            if (!V3_IsSane(World.velocity[idx])) { World.velocity[idx] = (V3){0.0f,0.0f,0.0f}; }
            else { float speed = V3_Mag(World.velocity[idx]); if (speed > MAX_SPEED) World.velocity[idx] = V3_ScaleByF(World.velocity[idx], MAX_SPEED / speed); }
            float linDrag = vexp(-2.0f * dtsub); World.velocity[idx] = V3_ScaleByF(World.velocity[idx],linDrag); SetPosition(idx,V3_AplusB(World.position[idx],V3_ScaleByF(World.velocity[idx],dtsub)),false);
            if (World.collider[idx] != COLTYPE_CAP) {
                if (!V3_IsSane(World.angularVelocity[idx])) { World.angularVelocity[idx] = (V3){0.0f,0.0f,0.0f}; }
                else {
                    float avel = V3_Mag(World.angularVelocity[idx]); if (avel > MAX_ANGULAR_SPEED) { World.angularVelocity[idx] = V3_ScaleByF(World.angularVelocity[idx],MAX_ANGULAR_SPEED / avel); avel = MAX_ANGULAR_SPEED; }
                    if (avel > PHY_EPSILON) { Quaternion dq = quat_from_axis_angle(V3_ScaleByF(World.angularVelocity[idx],1.f / avel),avel * dtsub); World.rotation[idx] = quat_normalize(quat_multiply(dq,World.rotation[idx])); }
                }
            } else World.angularVelocity[idx] = (V3){0.0f,0.0f,0.0f};
        }
        for (u16 i=0;i<dynamicEntityCount;++i) {
            u16 a = dynamicEntities[i]; if (World.collider[a] == COLTYPE_MSH || (Cheats.noclip && a == PLAYER1)) continue;
            i32 cx = PosGetCellCoordX(World.position[a].x), cz = PosGetCellCoordZ(World.position[a].z); u32 mask = GetCollisionMask(World.layer[a]);
            float searchRad = World.radius[a] + V3_Mag(World.velocity[a]) * dtsub + 0.5f; i32 radCells = (i32)(searchRad / CELL_SIZE) + 2;
            typedef struct { Manifold m; u16 otherIdx; } Contact; Contact contacts[32]; int contactCount = 0;
            for (i32 dx = -radCells; dx <= radCells; ++dx) {
                for (i32 dz = -radCells; dz <= radCells; ++dz) {
                    u32 cell = PosGetCellCoordsP(cx + dx,cz + dz);
                    for (u16 k = 0; k < cellCounts[cell]; ++k) {
                        u16 b = cellLists[cell][k]; if (b == a) continue;
                        if (Cheats.noclip && b == PLAYER1) continue;
                        if (!(mask & World.layer[b]) || World.collider[b] == COLTYPE_NONE) continue;
                        if ((World.instances[b].entflags & EF_RIGIDBODY) && b > a) continue;
                        V3 deltaPos = V3_AsubB(World.position[a],World.position[b]); float rr = (World.radius[a] + World.radius[b]) * 1.42f; if (V3_dot(deltaPos,deltaPos) > rr * rr) continue;
                        Manifold mf = {0};
                        float matA[16], matB[16];
                        const float *mxA = &modelMatrices[a*16];
                        const float *mxB = &modelMatrices[b*16];
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
                        else if (World.collider[a] == COLTYPE_CVX && World.collider[b] == COLTYPE_MSH) { mf = CvxMsh(World.instances[a].colMeshIndex,mxA,World.instances[b].modelIndex,mxB); if(mf.n) mf.normal=V3_ScaleByF(mf.normal,-1.0f); }
                        else if (World.collider[a] == COLTYPE_CAP && World.collider[b] == COLTYPE_CVX) { mf = PrimitiveCvx(a,World.instances[b].colMeshIndex,mxB); if(mf.n) mf.normal=V3_ScaleByF(mf.normal,-1.0f); }
                        else if (World.collider[a] == COLTYPE_CVX && World.collider[b] == COLTYPE_CAP) { mf = PrimitiveCvx(b,World.instances[a].colMeshIndex,mxA); }
                        else if (World.collider[a] == COLTYPE_SPH && World.collider[b] == COLTYPE_CVX) { mf = PrimitiveCvx(a,World.instances[b].colMeshIndex,mxB); if(mf.n) mf.normal=V3_ScaleByF(mf.normal,-1.0f); }
                        else if (World.collider[a] == COLTYPE_CVX && World.collider[b] == COLTYPE_SPH) { mf = PrimitiveCvx(b,World.instances[a].colMeshIndex,mxA); }
                        else if (World.collider[a] == COLTYPE_BOX && World.collider[b] == COLTYPE_CVX) { mf = PrimitiveCvx(a,World.instances[b].colMeshIndex,mxB); if(mf.n) mf.normal=V3_ScaleByF(mf.normal,-1.0f); }
                        else if (World.collider[a] == COLTYPE_CVX && World.collider[b] == COLTYPE_BOX) { mf = PrimitiveCvx(b,World.instances[a].colMeshIndex,mxA); }
                        else if (World.collider[a] == COLTYPE_CVX && World.collider[b] == COLTYPE_CVX) { mf = CvxCvx(World.instances[a].colMeshIndex,World.instances[b].colMeshIndex,mxA,mxB); if(mf.n) mf.normal=V3_ScaleByF(mf.normal,-1.0f); }
                        else { mf=OverlapToManifold(SphSph(World.position[a],World.colliderSize[a].x,World.position[b],World.colliderSize[b].x)); }
                        if (mf.n && contactCount < 32) contacts[contactCount++] = (Contact){mf,b};
                    }
                }
            }
            for (int cta=0;cta<contactCount-1;++cta) { for (int ctb=cta+1;ctb<contactCount;++ctb) { if (contacts[ctb].m.maxPen > contacts[cta].m.maxPen) { Contact tmp=contacts[cta]; contacts[cta]=contacts[ctb]; contacts[ctb]=tmp; } } }
            World.colliding[a]=false; flag_set(&World.instances[a].entflags,EF_GROUNDED,false);
            for (int c = 0; c < contactCount; ++c) {
                Manifold *mfp = &contacts[c].m; u16 j = contacts[c].otherIdx; World.colliding[a] = true; bool jIs = (j < INSTANCE_COUNT); if (jIs) World.colliding[j] = true;
                if (V3_dot(mfp->normal,(V3){0.0f,1.0f,0.0f}) >= 0.574f) {World.instances[a].entflags |= EF_GROUNDED;}
                if (jIs && (World.instances[j].entflags & EF_RIGIDBODY) && World.collider[j] != COLTYPE_MSH) ApplyManifoldResponse(a,j,mfp); else ApplyManifoldResponse(a,0,mfp);
            }
            World.instances[a].accumulatedForce = (V3){0.0f,0.0f,0.0f};
        }
    }
}

void AddForce(u16 i, V3 f, bool imp) { if (imp) { World.velocity[i] = V3_AplusB(World.velocity[i],V3_ScaleByF(f,1.0f / vmax(World.mass[i],0.001f))); } else { World.instances[i].accumulatedForce = V3_AplusB(World.instances[i].accumulatedForce,f); } }
float GetBasePlayerSpeed(u16 p,bool running){
    bool sprint=Sprint(); if(Cheats.noclip)return PLAYER_MAX_CYBER_SPEED*(sprint?2.5f:1.5f); if(World.curLev==LEVEL_CYBERSPACE)return PLAYER_MAX_CYBER_SPEED;
    BodyState b=World.instances[p].bodyState; float v=WALK_SPEED;
    switch(b){ case BodyState_CrouchingDown: case BodyState_Crouch:v=CROUCH_SPEED; break; case BodyState_Prone: case BodyState_ProningDown: case BodyState_ProningUp:v=PLAYER_MAX_PRONE_SPEED; break; default:break; }
    if ((sprint||World.boosterActive) && running) { v = World.invP1.fatigue > 80.0f && World.boosterActive ? SPRINT_SPEED_FATIGUED : SPRINT_SPEED;
    if (b==BodyState_Standing||b==BodyState_Crouch||b==BodyState_CrouchingDown)  v -= (WALK_SPEED-CROUCH_SPEED)*1.5f;
    else if(b==BodyState_Prone||b==BodyState_ProningDown||b==BodyState_ProningUp)v -= (WALK_SPEED-PLAYER_MAX_PRONE_SPEED)*2.f;}
    return v + (World.boosterActive ? PLAYER_BOOSTER_SPEED_BOOST : 0.0f);
}

void ApplyPlayerMovements() {
    float h = (float)Forward() - (float)Backpedal(), s = (float)StrafeRight() - (float)StrafeLeft(), vertInput = (float)SwimUp() - (float)SwimDn();
    Entity *p = &World.instances[PLAYER1]; Quaternion r = World.rotation[PLAYER1]; float y2 = r.y*r.y, xz = r.x*r.z, wy = r.w*r.y;
    p->forward=V3_Normalize((V3){ 2.0f*(xz + wy), 2.0f*(r.y*r.z - r.w*r.x), 1.0f - 2.0f*(r.x*r.x + y2) }); p->right=V3_Normalize((V3){ 1.0f - 2.0f*(y2 + r.z*r.z), 2.0f*(r.x*r.y + r.w*r.z), 2.0f*(xz - wy) });
    V3 inputDir = { p->forward.x*h + p->right.x*s, vertInput, p->forward.z*h + p->right.z*s }; float inputLenSq = V3_dot(inputDir,inputDir);
    V3 w = (inputLenSq > PHY_EPSILON) ? V3_ScaleByF(inputDir, 1.0f / vsqrtf(inputLenSq)) : (V3){0,0,0};
    float speed = GetBasePlayerSpeed(PLAYER1,inputLenSq > 0.01f) * 1.75f, accel = World.boosterActive ? 1.0f : 3.0f;
    V3 targetVel = V3_ScaleByF(w,speed); if (vabs(vertInput) < 0.001f) targetVel.y = World.velocity[PLAYER1].y;
    V3 dv = V3_AsubB(targetVel,World.velocity[PLAYER1]); dv = (V3){vclamp(dv.x,-10.0f,10.0f), vclamp(dv.y,-10.0f,10.0f), vclamp(dv.z,-10.0f,10.0f)};
    World.velocity[PLAYER1] = V3_AplusB(World.velocity[PLAYER1],V3_ScaleByF(dv,accel * vclamp((float)World.deltaTime,0.0005f,0.1f)));
}
