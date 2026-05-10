// physics.c - Physics System
#define MAX_COLLISION_ITERATIONS  4
#define COLLISION_EPSILON         0.0001f
#define MIN_VELOCITY_THRESHOLD    0.001f
#define STEP_MIN_NORMAL_Y         0.7f // Normals above this are walkable (vcos(45°))
#define SLOPE_CLIMB_MAX_DEG       45.0f
#define GROUND_PROBE_DIST         0.15f
#define SNAP_STEP                 0.02f
#define STEP_HEIGHT 0.32f
static const Vector3 GRAVITY_VECTOR = {0.0f,-9.81f,0.0f};
static const float GROUNDED_HYSTERESIS_TIME = 0.1f, GROUNDED_PROBE_OFFSET = 0.04f;
typedef struct { Vector3 center,halfExtents; Quaternion rot; } ShapeBox;
typedef struct { Vector3 center; float radius; } ShapeSphere;
typedef struct { Vector3 tip,base; float radius; } ShapeCapsule;
static u32 GetCollisionMask(u32 layer) {
    switch (layer) {
        case Layer_Default:         return 629079559u; case Layer_TransparentFX:    return 85876231u;  case Layer_IgnoreRaycast:return 85876231u;
        case Layer_Geometry:        return 69099015u;  case Layer_NPC:              return 127819271u; case Layer_PlayerBullets:return 622624263u;
        case Layer_Player:          return 85411335u;  case Layer_Corpse:           return 84167169u;  case Layer_PhysObjects:  return 84704775u;
        case Layer_Sky:             return 4097u;      case Layer_PlayerTriggerOnly:return 3149824u;   case Layer_Trigger:      return 67919361u;
        case Layer_Door:            return 85884423u;  case Layer_InterDebris:      return 84299265u;  case Layer_Player2:      return 85941767u;
        case Layer_Player3:         return 86990343u;  case Layer_Player4:          return 89087495u;  case Layer_NPCTrigger:   return 1024u;
        case Layer_NPCBullet:       return 605846023u; case Layer_NPCClip:          return 1024u;      case Layer_Clip:         return 7345152u;
        case Layer_CorpseSearchable:return 2049u;
    } return 0u;
}

static void Entity_GetCapsule(const Entity *e,ShapeCapsule *out) {
    float r=e->colliderSize.x, hi=vmax(0.0f,(e->colliderSize.y*0.5f)-r);
    Vector3 wc=Vector3_A_plus_B(e->position,quat_rotate_vector(e->rotation,e->colliderCenter));
    Vector3 axis=(e->colliderSize.z<0.5f) ? quat_rotate_vector(e->rotation,(Vector3){1.0f,0.0f,0.0f}) : (e->colliderSize.z<1.5f) ? quat_rotate_vector(e->rotation,(Vector3){0.0f,1.0f,0.0f}) : quat_rotate_vector(e->rotation,(Vector3){0.0f,0.0f,1.0f});
    out->radius=r; out->base=Vector3_A_minus_B(wc,scale_vector3(axis,hi)); out->tip=Vector3_A_plus_B(wc,scale_vector3(axis,hi));
}

static void Entity_GetBox(const Entity *e,ShapeBox *out) { out->center=Vector3_A_plus_B(e->position,quat_rotate_vector(e->rotation,e->colliderCenter)); out->halfExtents=scale_vector3(e->colliderSize,0.5f); out->rot=e->rotation; }
static void Entity_GetSphere(const Entity *e,ShapeSphere *out) { out->center=Vector3_A_plus_B(e->position,quat_rotate_vector(e->rotation,e->colliderCenter)); out->radius=e->colliderSize.x; }
static inline void obb_axes(Quaternion q,Vector3 *ax,Vector3 *ay,Vector3 *az) { *ax=quat_rotate_vector(q,(Vector3){1.0f,0.0f,0.0f}); *ay=quat_rotate_vector(q,(Vector3){0.0f,1.0f,0.0f}); *az=quat_rotate_vector(q,(Vector3){0.0f,0.0f,1.0f}); }
typedef struct { bool hit; float toi; Vector3 point,normal; } SweepResult;
typedef struct { float depth; Vector3 normal,point; } ProbeResult;
typedef struct { u32 type; void *shape; } Shape;
static SweepResult SweepSphereSphere(Vector3 aPos, float aRadius, Vector3 aVel, Vector3 bPos, float bRadius) {
    SweepResult r = {0};
    float cr = aRadius + bRadius;
    Vector3 rp = Vector3_A_minus_B(aPos, bPos);
    float c = dot_vector3(rp,rp) - cr*cr;
    float a = dot_vector3(aVel,aVel);
    if (a < 1e-8f) { // static overlap only
        if (c > 0.0f) return r;
        r.hit=true; r.toi=0.0f;
        r.normal = magnitude_vector3(rp)>1e-6f ? normalize_vector3(rp) : (Vector3){0,1,0};
        r.point  = Vector3_A_plus_B(bPos, scale_vector3(r.normal, bRadius));
        return r;
    }
    float b = 2.0f*dot_vector3(rp,aVel), disc = b*b - 4.0f*a*c;
    if (c <= 0.0f) { // already overlapping
        r.hit=true; r.toi=0.0f;
        r.normal = magnitude_vector3(rp)>1e-6f ? normalize_vector3(rp) : (Vector3){0,1,0};
        r.point  = Vector3_A_plus_B(bPos, scale_vector3(r.normal, bRadius));
        return r;
    }
    if (disc < 0.0f) return r;
    float t = (-b - vsqrtf(disc)) / (2.0f*a);
    if (t < 0.0f || t > 1.0f) return r;
    Vector3 hp = Vector3_A_plus_B(aPos, scale_vector3(aVel, t));
    r.hit=true; r.toi=t;
    r.normal = normalize_vector3(Vector3_A_minus_B(hp, bPos));
    r.point  = Vector3_A_minus_B(hp, scale_vector3(r.normal, aRadius));
    return r;
}

static SweepResult SweepSphereCapsule(Vector3 sPos, float sRadius, Vector3 sVel, Vector3 cBase, Vector3 cTip, float cRadius) {
    SweepResult r = {0};
    Vector3 capsuleAxis = Vector3_A_minus_B(cTip, cBase);
    int samples = 4; float closestToi = 2.0f;
    Vector3 closestNormal = (Vector3){0.0f,1.0f,0.0f}, closestPoint = (Vector3){0.0f,0.0f,0.0f};
    for (int i = 0; i < samples; ++i) {
        float t = (float)i / (float)(samples - 1);
        Vector3 pointOnCapsule = Vector3_A_plus_B(cBase, scale_vector3(capsuleAxis, t));
        SweepResult hit = SweepSphereSphere(sPos, sRadius, sVel, pointOnCapsule, cRadius);
        if (hit.hit && hit.toi < closestToi) {
            closestToi = hit.toi;
            closestNormal = hit.normal;
            closestPoint = hit.point;
            r.hit = true;
        }
    }
    
    if (r.hit) { r.toi = closestToi; r.normal = closestNormal; r.point = closestPoint; }
    return r;
}

static SweepResult SweepSphereBox(Vector3 sPos, float sRadius, Vector3 sVel, ShapeBox box) {
    Quaternion invRot={-box.rot.x,-box.rot.y,-box.rot.z,box.rot.w};
    Vector3 lPos=quat_rotate_vector(invRot,Vector3_A_minus_B(sPos,box.center));
    Vector3 lVel=quat_rotate_vector(invRot,sVel);
    Vector3 eHE={box.halfExtents.x+sRadius,box.halfExtents.y+sRadius,box.halfExtents.z+sRadius};

    // Check static overlap first (sphere center inside expanded box)
    bool inside = lPos.x>=-eHE.x&&lPos.x<=eHE.x&&lPos.y>=-eHE.y&&lPos.y<=eHE.y&&lPos.z>=-eHE.z&&lPos.z<=eHE.z;
    if (inside) {
        // Push out on minimum-penetration axis
        float dx=eHE.x-vabs(lPos.x), dy=eHE.y-vabs(lPos.y), dz=eHE.z-vabs(lPos.z);
        Vector3 n = dx<dy&&dx<dz ? (Vector3){lPos.x>0?1.f:-1.f,0,0}
                  : dy<dz       ? (Vector3){0,lPos.y>0?1.f:-1.f,0}
                                : (Vector3){0,0,lPos.z>0?1.f:-1.f};
        SweepResult r;
        r.hit=true; r.toi=0.0f;
        r.normal=normalize_vector3(quat_rotate_vector(box.rot,n));
        r.point=Vector3_A_plus_B(sPos,scale_vector3(r.normal,-sRadius));
        return r;
    }

    // Slab sweep test
    float tEnter=0.0f, tExit=1.0f;
    Vector3 hitNormalLocal={0,1,0};
    for (int ax=0; ax<3; ++ax) {
        float pos=ax==0?lPos.x:ax==1?lPos.y:lPos.z;
        float vel=ax==0?lVel.x:ax==1?lVel.y:lVel.z;
        float he =ax==0?eHE.x :ax==1?eHE.y :eHE.z;
        if (vabs(vel)<1e-8f) { if (pos<-he||pos>he) return (SweepResult){0}; continue; }
        float t0=(-he-pos)/vel, t1=(he-pos)/vel;
        float tN=vmin(t0,t1), tF=vmax(t0,t1);
        if (tN>tEnter) {
            tEnter=tN;
            float s=vel>0?-1.f:1.f;
            hitNormalLocal=ax==0?(Vector3){s,0,0}:ax==1?(Vector3){0,s,0}:(Vector3){0,0,s};
        }
        tExit=vmin(tExit,tF);
        if (tEnter>tExit) return (SweepResult){0};
    }
    if (tEnter<0.0f||tEnter>1.0f) return (SweepResult){0};

    SweepResult r;
    r.hit=true; r.toi=tEnter;
    r.normal=normalize_vector3(quat_rotate_vector(box.rot,hitNormalLocal));
    r.point=Vector3_A_plus_B(Vector3_A_plus_B(sPos,scale_vector3(sVel,tEnter)),scale_vector3(r.normal,-sRadius));
    return r;
}

static SweepResult SweepBoxSphere(ShapeBox box, Vector3 bVel, Vector3 sPos, float sRadius) { return SweepSphereBox(sPos, sRadius, scale_vector3(bVel, -1.0f), box); }
static SweepResult SweepBoxBox(ShapeBox moving, Vector3 vel, ShapeBox stat) {
    SweepResult r={0}; float closestToi=2.0f;
    Vector3 ax,ay,az; obb_axes(moving.rot,&ax,&ay,&az);
    float ox[]={-1,+1}; float oy[]={-1,+1}; float oz[]={-1,+1};
    for (int ix=0;ix<2;++ix) for (int iy=0;iy<2;++iy) for (int iz=0;iz<2;++iz) {
        Vector3 vtx=Vector3_A_plus_B(moving.center,Vector3_A_plus_B(scale_vector3(ax,ox[ix]*moving.halfExtents.x),Vector3_A_plus_B(scale_vector3(ay,oy[iy]*moving.halfExtents.y),scale_vector3(az,oz[iz]*moving.halfExtents.z))));
        ShapeBox unit={stat.center,stat.halfExtents,stat.rot};
        SweepResult hit=SweepSphereBox(vtx,0.0f,vel,unit);
        if (hit.hit && hit.toi<closestToi) { closestToi=hit.toi; r=hit; }
    }
    return r;
}

static SweepResult SweepBoxCapsule(ShapeBox box, Vector3 boxVel, Vector3 cBase, Vector3 cTip, float cRadius) {
    SweepResult r = {0};
    int samples = 5; float closestToi = 2.0f;
    Vector3 closestNormal = (Vector3){0.0f,1.0f,0.0f}, closestPoint = {0.0f,0.0f,0.0f};
    for (int i = 0; i < samples; ++i) { // Sample capsule, test each sample against box
        float t = (float)i / (float)(samples - 1);
        Vector3 samplePos = Vector3_A_plus_B(cBase,scale_vector3(Vector3_A_minus_B(cTip,cBase),t));
        SweepResult hit = SweepSphereBox(samplePos,cRadius,boxVel,box);
        if (hit.hit && hit.toi < closestToi) {
            closestToi = hit.toi;
            closestNormal = hit.normal;
            closestPoint = hit.point;
            r.hit = true;
        }
    }
    
    if (r.hit) { r.toi = closestToi; r.normal = closestNormal; r.point = closestPoint; }
    return r;
}

static SweepResult SweepCapsuleSphere(Vector3 cBase, Vector3 cTip, float cRadius, Vector3 cVel, Vector3 sPos, float sRadius) { return SweepSphereCapsule(sPos,sRadius,scale_vector3(cVel,-1.0f), cBase,cTip,cRadius); } // Reverse roles and negate velocity
static SweepResult SweepCapsuleCapsule(Vector3 aBase, Vector3 aTip, float aRadius, Vector3 aVel, Vector3 bBase, Vector3 bTip, float bRadius) {
    SweepResult r = {0};
    int samples = 4; float closestToi = 2.0f;
    Vector3 closestNormal = (Vector3){0.0f,1.0f,0.0f}; Vector3 closestPoint = (Vector3){0.0f,0.0f,0.0f};
    Vector3 aAxis = Vector3_A_minus_B(aTip, aBase);
    for (int i=0;i<samples;++i) { // Sample both capsules and test sphere samples
        float t = (float)i / (float)(samples - 1);
        Vector3 aPoint = Vector3_A_plus_B(aBase, scale_vector3(aAxis, t));
        SweepResult hit = SweepSphereCapsule(aPoint, aRadius, aVel, bBase, bTip, bRadius); // Test this point against capsule B
        if (hit.hit && hit.toi < closestToi) { closestToi = hit.toi; closestNormal = hit.normal; closestPoint = hit.point; r.hit = true; }
    }
    
    if (r.hit) { r.toi = closestToi; r.normal = closestNormal; r.point = closestPoint; }
    return r;
}

// static float SphereTriangleDistance(Vector3 sphereCenter, Vector3 a, Vector3 b, Vector3 c, Vector3 *closestPoint) {
//     Vector3 edge0 = Vector3_A_minus_B(b, a); Vector3 edge1 = Vector3_A_minus_B(c, a);
//     Vector3 toSphere = Vector3_A_minus_B(sphereCenter, a); // Find closest point on triangle to sphere center
//     float d00 = dot_vector3(edge0, edge0);
//     float d01 = dot_vector3(edge0, edge1);
//     float d11 = dot_vector3(edge1, edge1);
//     float d20 = dot_vector3(toSphere, edge0);
//     float d21 = dot_vector3(toSphere, edge1);
//     float denom = d00 * d11 - d01 * d01;
//     if (vabs(denom) < 1e-6f) { // Degenerate triangle
//         *closestPoint = a; return magnitude_vector3(Vector3_A_minus_B(sphereCenter, a));
//     }
//     
//     float u = (d11 * d20 - d01 * d21) / denom; float v = (d00 * d21 - d01 * d20) / denom;
//     u = vclamp(u, 0.0f, 1.0f); v = vclamp(v, 0.0f, 1.0f);
//     if (u + v > 1.0f) { u = vmax(0.0f, 1.0f - v); v = vmax(0.0f, 1.0f - u); }
//     Vector3 p = Vector3_A_plus_B(a, Vector3_A_plus_B(scale_vector3(edge0, u), scale_vector3(edge1, v)));
//     *closestPoint = p; Vector3 delta = Vector3_A_minus_B(sphereCenter,p);
//     return magnitude_vector3(delta);
// }

// Shared: extract model matrix components. Call once per mesh sweep.
// typedef struct { float m00,m10,m20,m01,m11,m21,m02,m12,m22,tx,ty,tz,sclx,scly,sclz,sclx2,scly2,sclz2; } MeshXform;
// static bool MeshXform_Get(Entity *me, u16 idx, MeshXform *x) {
//     if (me->collider!=COLLIDER_TYPE_MESH||me->modelIndex>=loadedModelsMaxIndex) return false;
//     float M[16]; CopyMemoryFromBtoAForNBytes(M,&modelMatrices[idx*16],16*sizeof(float));
//     x->m00=M[0];x->m10=M[1];x->m20=M[2]; x->m01=M[4];x->m11=M[5];x->m21=M[6]; x->m02=M[8];x->m12=M[9];x->m22=M[10];
//     x->tx=M[12];x->ty=M[13];x->tz=M[14];
//     x->sclx=vsqrtf(x->m00*x->m00+x->m10*x->m10+x->m20*x->m20); x->sclx2=x->sclx*x->sclx;
//     x->scly=vsqrtf(x->m01*x->m01+x->m11*x->m11+x->m21*x->m21); x->scly2=x->scly*x->scly;
//     x->sclz=vsqrtf(x->m02*x->m02+x->m12*x->m12+x->m22*x->m22); x->sclz2=x->sclz*x->sclz;
//     return true;
// }
// static Vector3 MeshXform_ToLocal(const MeshXform *x, Vector3 p) { Vector3 r={p.x-x->tx,p.y-x->ty,p.z-x->tz}; return (Vector3){(r.x*x->m00+r.y*x->m10+r.z*x->m20)/x->sclx2,(r.x*x->m01+r.y*x->m11+r.z*x->m21)/x->scly2,(r.x*x->m02+r.y*x->m12+r.z*x->m22)/x->sclz2}; }
// static Vector3 MeshXform_NormalToWorld(const MeshXform *x, Vector3 n) { return normalize_vector3((Vector3){(x->m00/x->sclx)*n.x+(x->m01/x->scly)*n.y+(x->m02/x->sclz)*n.z,(x->m10/x->sclx)*n.x+(x->m11/x->scly)*n.y+(x->m12/x->sclz)*n.z,(x->m20/x->sclx)*n.x+(x->m21/x->scly)*n.y+(x->m22/x->sclz)*n.z}); }
// static Vector3 MeshXform_PointToWorld(const MeshXform *x, Vector3 p) { return (Vector3){x->m00*p.x+x->m01*p.y+x->m02*p.z+x->tx,x->m10*p.x+x->m11*p.y+x->m12*p.z+x->ty,x->m20*p.x+x->m21*p.y+x->m22*p.z+x->tz}; }
// static void MeshTri_Get(u16 mindex, u32 j, Vector3 *a, Vector3 *b, Vector3 *c) {
//     u32 bA=(u32)modelTriangles[mindex][j*3+0]*VERTEX_ATTRIBUTES_SIZE;
//     u32 bB=(u32)modelTriangles[mindex][j*3+1]*VERTEX_ATTRIBUTES_SIZE;
//     u32 bC=(u32)modelTriangles[mindex][j*3+2]*VERTEX_ATTRIBUTES_SIZE;
//     a->x=half_to_float(*(half*)(modelVertices[mindex]+bA+0)); a->y=half_to_float(*(half*)(modelVertices[mindex]+bA+2)); a->z=half_to_float(*(half*)(modelVertices[mindex]+bA+4));
//     b->x=half_to_float(*(half*)(modelVertices[mindex]+bB+0)); b->y=half_to_float(*(half*)(modelVertices[mindex]+bB+2)); b->z=half_to_float(*(half*)(modelVertices[mindex]+bB+4));
//     c->x=half_to_float(*(half*)(modelVertices[mindex]+bC+0)); c->y=half_to_float(*(half*)(modelVertices[mindex]+bC+2)); c->z=half_to_float(*(half*)(modelVertices[mindex]+bC+4));
// }
// 
// static bool MeshCouldOverlap(Entity *me, u16 meIdx, Vector3 pos, float radius) {
//     float M[16]; CopyMemoryFromBtoAForNBytes(M,&modelMatrices[meIdx*16],16*sizeof(float));
//     Vector3 meshOrigin={M[12],M[13],M[14]};
//     float meshBound = me->colliderSize.x > 0.01f ? me->colliderSize.x : 32.0f;
//     Vector3 d=Vector3_A_minus_B(pos,meshOrigin);
//     float distSq=dot_vector3(d,d), maxReach=meshBound+radius;
//     return distSq < maxReach*maxReach;
// }

// static SweepResult SweepSphereMeshEntity(Vector3 sPos, float sRadius, Entity *me, u16 meIdx) {
//     SweepResult r={0}; MeshXform x;
//     if (!MeshXform_Get(me,meIdx,&x)) return r;
//     if (!MeshCouldOverlap(me,meIdx,sPos,sRadius)) return r;
//     u16 mindex=me->modelIndex; u32 triCount=modelTriangleCounts[mindex];
//     if (!triCount) return r;
// 
//     Vector3 localSphere=MeshXform_ToLocal(&x,sPos);
//     float localRadius=sRadius/vmin(x.sclx,vmin(x.scly,x.sclz));
//     float bestDist=localRadius;
//     Vector3 bestNormal={0,1,0}, bestPoint=localSphere;
//     for (u32 j=0; j<triCount; ++j) {
//         Vector3 a,b,c; MeshTri_Get(mindex,j,&a,&b,&c);
//         Vector3 cp; float dist=SphereTriangleDistance(localSphere,a,b,c,&cp);
//         if (dist<bestDist) {
//             bestDist=dist;
//             bestPoint=cp;
//             Vector3 n=normalize_vector3(cross_vector3(Vector3_A_minus_B(b,a),Vector3_A_minus_B(c,a)));
//             bestNormal = dot_vector3(n,Vector3_A_minus_B(localSphere,cp))>=0.0f ? n : scale_vector3(n,-1.0f);
//             r.hit=true;
//         }
//     }
//     if (!r.hit) return r;
//     r.toi=0.0f;
//     r.normal=MeshXform_NormalToWorld(&x,bestNormal);
//     r.point=MeshXform_PointToWorld(&x,bestPoint);
//     return r;
// }

// static SweepResult SweepCapsuleMeshEntity(Vector3 cBase, Vector3 cTip, float cRadius, Entity *me, u16 meIdx) {
//     SweepResult r={0}; MeshXform x;
//     if (!MeshXform_Get(me,meIdx,&x)) return r;
//     u16 mindex=me->modelIndex; u32 triCount=modelTriangleCounts[mindex];
//     if (!triCount) return r;
// 
//     Vector3 lBase=MeshXform_ToLocal(&x,cBase), lTip=MeshXform_ToLocal(&x,cTip);
//     float localRadius=cRadius/vmin(x.sclx,vmin(x.scly,x.sclz));
//     Vector3 capsAxis=Vector3_A_minus_B(lTip,lBase);
// 
//     float bestDist=localRadius;
//     Vector3 bestNormal={0,1,0}, bestPoint=lBase;
//     // Sample capsule axis and find closest triangle contact across all samples
//     for (int s=0; s<5; ++s) {
//         Vector3 sp=Vector3_A_plus_B(lBase,scale_vector3(capsAxis,(float)s*0.25f));
//         for (u32 j=0; j<triCount; ++j) {
//             Vector3 a,b,c; MeshTri_Get(mindex,j,&a,&b,&c);
//             Vector3 cp; float dist=SphereTriangleDistance(sp,a,b,c,&cp);
//             if (dist<bestDist) {
//                 bestDist=dist;
//                 bestPoint=cp;
//                 Vector3 n=normalize_vector3(cross_vector3(Vector3_A_minus_B(b,a),Vector3_A_minus_B(c,a)));
//                 bestNormal = dot_vector3(n,Vector3_A_minus_B(sp,cp))>=0.0f ? n : scale_vector3(n,-1.0f);
//                 r.hit=true;
//             }
//         }
//     }
//     if (!r.hit) return r;
//     r.toi=0.0f;
//     r.normal=MeshXform_NormalToWorld(&x,bestNormal);
//     r.point=MeshXform_PointToWorld(&x,bestPoint);
//     return r;
// }

// static SweepResult SweepBoxMesh(Vector3 boxCenter, Vector3 boxHE, Quaternion boxRot, Entity *me, u16 meIdx) {
//     SweepResult r={0}; MeshXform x;
//     if (!MeshXform_Get(me,meIdx,&x)) return r;
//     u16 mindex=me->modelIndex; u32 triCount=modelTriangleCounts[mindex];
//     if (!triCount) return r;
// 
//     // Build 8 box vertices in local mesh space
//     Vector3 ax=quat_rotate_vector(boxRot,(Vector3){1,0,0});
//     Vector3 ay=quat_rotate_vector(boxRot,(Vector3){0,1,0});
//     Vector3 az=quat_rotate_vector(boxRot,(Vector3){0,0,1});
//     Vector3 verts[8];
//     for (int i=0;i<8;++i) {
//         float sx=i&1?1.f:-1.f, sy=i&2?1.f:-1.f, sz=i&4?1.f:-1.f;
//         Vector3 w=Vector3_A_plus_B(boxCenter,Vector3_A_plus_B(scale_vector3(ax,sx*boxHE.x),Vector3_A_plus_B(scale_vector3(ay,sy*boxHE.y),scale_vector3(az,sz*boxHE.z))));
//         verts[i]=MeshXform_ToLocal(&x,w);
//     }
// 
//     float contactEps=0.01f;
//     float bestDist=contactEps;
//     Vector3 bestNormal={0,1,0}, bestPoint=verts[0];
//     for (int v=0;v<8;++v) {
//         for (u32 j=0;j<triCount;++j) {
//             Vector3 a,b,c; MeshTri_Get(mindex,j,&a,&b,&c);
//             Vector3 cp; float dist=SphereTriangleDistance(verts[v],a,b,c,&cp);
//             if (dist<bestDist) {
//                 bestDist=dist;
//                 bestPoint=cp;
//                 Vector3 n=normalize_vector3(cross_vector3(Vector3_A_minus_B(b,a),Vector3_A_minus_B(c,a)));
//                 bestNormal = dot_vector3(n,Vector3_A_minus_B(verts[v],cp))>=0.0f ? n : scale_vector3(n,-1.0f);
//                 r.hit=true;
//             }
//         }
//     }
// 
//     if (!r.hit) {
//         Vector3 zeroVel={0,0,0};
//         for (u32 j=0;j<triCount&&!r.hit;++j) {
//             Vector3 a,b,c; MeshTri_Get(mindex,j,&a,&b,&c);
//             Vector3 triVerts[3]={a,b,c};
//             for (int tv=0;tv<3;++tv) {
//                 SweepResult hit=SweepSphereBox(MeshXform_PointToWorld(&x,triVerts[tv]),0.0f,zeroVel,
//                     (ShapeBox){boxCenter,boxHE,boxRot});
//                 if (hit.hit) {
//                     r.hit=true;
//                     Vector3 n=normalize_vector3(cross_vector3(Vector3_A_minus_B(b,a),Vector3_A_minus_B(c,a)));
//                     r.normal=MeshXform_NormalToWorld(&x, dot_vector3(n,Vector3_A_minus_B(verts[0],triVerts[tv]))>=0.0f?n:scale_vector3(n,-1.0f));
//                     r.point=MeshXform_PointToWorld(&x,triVerts[tv]);
//                     r.toi=0.0f;
//                     break;
//                 }
//             }
//         }
//         return r;
//     }
//     r.toi=0.0f;
//     r.normal=MeshXform_NormalToWorld(&x,bestNormal);
//     r.point=MeshXform_PointToWorld(&x,bestPoint);
//     return r;
// }

static SweepResult SweepCapsuleBox(Vector3 sBase, Vector3 sTip, float sRadius, Vector3 sVel, ShapeBox box) {
    SweepResult r = {0}; float closestToi=2.0f;
    Vector3 axis=Vector3_A_minus_B(sTip,sBase);
    for (int i=0; i<4; ++i) {
        float t=(float)i/3.0f;
        SweepResult hit=SweepSphereBox(Vector3_A_plus_B(sBase,scale_vector3(axis,t)),sRadius,sVel,box);
        if (hit.hit && hit.toi<closestToi) { closestToi=hit.toi; r=hit; }
    }
    return r;
}

// static SweepResult SweepEntity(Entity *moving, Vector3 moveVel, Entity *stationary, u16 stationaryIdx) {
//     SweepResult r = {0};
//     if (stationary->collider == COLLIDER_TYPE_MESH) { // Handle mesh entities separately
//         ShapeSphere movingSphere; ShapeBox movingBox; ShapeCapsule movingCapsule;
//         switch (moving->collider) {
//             case COLLIDER_TYPE_SPHERE:  Entity_GetSphere(moving,&movingSphere);   return SweepSphereMeshEntity(movingSphere.center,movingSphere.radius,stationary,stationaryIdx);
//             case COLLIDER_TYPE_BOX:     Entity_GetBox(moving,&movingBox);         return SweepBoxMesh(movingBox.center,movingBox.halfExtents,movingBox.rot,stationary,stationaryIdx);
//             case COLLIDER_TYPE_CAPSULE: Entity_GetCapsule(moving,&movingCapsule); return SweepCapsuleMeshEntity(movingCapsule.base,movingCapsule.tip,movingCapsule.radius,stationary,stationaryIdx);
//             default: return r;
//         }
//     }
//     
//     if (moving->collider == COLLIDER_TYPE_NONE || stationary->collider == COLLIDER_TYPE_NONE) return r; // Non-mesh entity dispatch (existing code)
//     
//     ShapeSphere movingSphere,stationarySphere; ShapeBox movingBox,stationaryBox; ShapeCapsule movingCapsule,stationaryCapsule;
//     switch (moving->collider) {
//         case COLLIDER_TYPE_SPHERE:
//             Entity_GetSphere(moving, &movingSphere);
//             switch (stationary->collider) {
//                 case COLLIDER_TYPE_SPHERE:  Entity_GetSphere(stationary,&stationarySphere);   return SweepSphereSphere(movingSphere.center,movingSphere.radius,moveVel,stationarySphere.center,stationarySphere.radius);
//                 case COLLIDER_TYPE_BOX:     Entity_GetBox(stationary,&stationaryBox);         return SweepSphereBox(movingSphere.center,movingSphere.radius,moveVel,stationaryBox);
//                 case COLLIDER_TYPE_CAPSULE: Entity_GetCapsule(stationary,&stationaryCapsule); return SweepSphereCapsule(movingSphere.center,movingSphere.radius,moveVel,stationaryCapsule.base,stationaryCapsule.tip,stationaryCapsule.radius);
//                 default: break;
//             }
//             break;
//         case COLLIDER_TYPE_BOX:
//             Entity_GetBox(moving, &movingBox);
//             switch (stationary->collider) {
//                 case COLLIDER_TYPE_SPHERE:
//                     Entity_GetSphere(stationary, &stationarySphere);
//                     return SweepBoxSphere(movingBox, moveVel, stationarySphere.center, stationarySphere.radius);
//                 case COLLIDER_TYPE_BOX:
//                     Entity_GetBox(stationary, &stationaryBox);
//                     return SweepBoxBox(movingBox, moveVel, stationaryBox);
//                 case COLLIDER_TYPE_CAPSULE:
//                     Entity_GetCapsule(stationary, &stationaryCapsule);
//                     return SweepBoxCapsule(movingBox, moveVel, 
//                                           stationaryCapsule.base, stationaryCapsule.tip, stationaryCapsule.radius);
//                 default: break;
//             }
//             break;
//         case COLLIDER_TYPE_CAPSULE:
//             Entity_GetCapsule(moving, &movingCapsule);
//             switch (stationary->collider) {
//                 case COLLIDER_TYPE_SPHERE:
//                     Entity_GetSphere(stationary, &stationarySphere);
//                     return SweepCapsuleSphere(movingCapsule.base, movingCapsule.tip, movingCapsule.radius, moveVel,
//                                             stationarySphere.center, stationarySphere.radius);
//                 case COLLIDER_TYPE_BOX:
//                     Entity_GetBox(stationary, &stationaryBox);
//                     return SweepCapsuleBox(movingCapsule.base, movingCapsule.tip, movingCapsule.radius, 
//                                           moveVel, stationaryBox);
//                 case COLLIDER_TYPE_CAPSULE:
//                     Entity_GetCapsule(stationary, &stationaryCapsule);
//                     return SweepCapsuleCapsule(movingCapsule.base, movingCapsule.tip, movingCapsule.radius, moveVel,
//                                              stationaryCapsule.base, stationaryCapsule.tip, stationaryCapsule.radius);
//                 default: break;
//             }
//             break;
//         default: break;
//     }
//     
//     return r;
// }

static ProbeResult ProbeCapsule(Vector3 base, Vector3 tip, float radius, u32 collisionMask) {
    ProbeResult result={0};
    Vector3 downVel={0,-0.001f,0};
    for (u32 i=0; i<Sys_Global.loadedInstances; ++i) {
        Entity *other=&Sys_Global.instances[i];
        if (!(other->entflags&ENTFLAG_ACTIVE)||other->collider==COLLIDER_TYPE_NONE) continue;
        if (!(collisionMask&other->layer)) continue;
        
        Vector3 toOther=Vector3_A_minus_B(other->position,base);
        float quickDist=vabs(toOther.x)+vabs(toOther.y)+vabs(toOther.z);
        float quickMax=radius+vmax(other->colliderSize.x,vmax(other->colliderSize.y,other->colliderSize.z))*2.0f+1.0f;
        if (quickDist>quickMax) continue;

        SweepResult hit={0};
        ShapeSphere ss; ShapeBox sb; ShapeCapsule sc;
        switch (other->collider) {
            case COLLIDER_TYPE_SPHERE: Entity_GetSphere(other,&ss); hit=SweepSphereCapsule(ss.center,ss.radius,(Vector3){0},base,tip,radius); break;
            case COLLIDER_TYPE_BOX: Entity_GetBox(other,&sb); hit=SweepCapsuleBox(base,tip,radius,downVel,sb); break;
            case COLLIDER_TYPE_CAPSULE: Entity_GetCapsule(other,&sc); hit=SweepCapsuleCapsule(base,tip,radius,(Vector3){0},sc.base,sc.tip,sc.radius); break;
            //case COLLIDER_TYPE_MESH: hit=SweepCapsuleMeshEntity(base,tip,radius,other,(u16)i); break;
            default: continue;
        }
        if (hit.hit) { result.depth=radius; result.normal=hit.normal; result.point=hit.point; return result; }
    }
    return result;
}

static double g_groundedLostTime[PLAYER2 + 1];
static void UpdatePlayerGrounding(u16 playerIdx, Vector3 pos, u32 collisionMask) {
    Entity* e = &Sys_Global.instances[playerIdx];
    bool wasGrounded = (e->entflags & ENTFLAG_GROUNDED) != 0;
    float probeRange = GROUND_PROBE_DIST;
    Vector3 probeBase = pos;
    Vector3 probeTip = {pos.x, pos.y - probeRange, pos.z};
    ProbeResult probe = ProbeCapsule(probeBase, probeTip, PLAYER_RADIUS, collisionMask);
    bool isGrounded = false;
    float slopeDegrees = 91.0f;  // Default: too steep
    if (probe.depth > 0.0f && probe.normal.y > 0.1f) {
        isGrounded = true;
        slopeDegrees = (180.0f / 3.14159265f) * vacosf(vmax(-1.0f, vmin(1.0f, probe.normal.y)));
    }
    
    bool withinClimbAngle = isGrounded && (slopeDegrees <= SLOPE_CLIMB_MAX_DEG);
    if (withinClimbAngle) { g_groundedLostTime[playerIdx] = 0.0f; flag_set(&e->entflags, ENTFLAG_GROUNDED, true); } // Ground state machine with hysteresis
    else {
        if (!isGrounded && wasGrounded) { // Lost ground: check hysteresis band (slightly lower probe)
            Vector3 hProbeBase = {pos.x, pos.y - GROUNDED_PROBE_OFFSET, pos.z};
            Vector3 hProbeTip = {pos.x, pos.y - GROUNDED_PROBE_OFFSET - probeRange, pos.z};
            ProbeResult hProbe = ProbeCapsule(hProbeBase, hProbeTip, PLAYER_RADIUS, collisionMask);
            float hSlopeDeg = 91.0f;
            if (hProbe.depth > 0.0f && hProbe.normal.y > 0.1f) hSlopeDeg = (180.0f / 3.14159265f) * vacosf(vmax(-1.0f, vmin(1.0f, hProbe.normal.y)));
            if (hProbe.depth > 0.0f && hSlopeDeg <= SLOPE_CLIMB_MAX_DEG) {
                // Within hysteresis band - stay grounded
                g_groundedLostTime[playerIdx] = 0.0f;
                flag_set(&e->entflags, ENTFLAG_GROUNDED, true);
            } else if (g_groundedLostTime[playerIdx] <= 0.0f) g_groundedLostTime[playerIdx] = Sys_Global.pauseRelativeTime; // Start off-delay timer
        }
        
        // Apply off-delay grace period
        if (!withinClimbAngle && g_groundedLostTime[playerIdx] > 0.0f) {
            float elapsed = Sys_Global.pauseRelativeTime - g_groundedLostTime[playerIdx];
            if (elapsed < GROUNDED_HYSTERESIS_TIME) flag_set(&e->entflags, ENTFLAG_GROUNDED, true);  // Still in grace period
            else { g_groundedLostTime[playerIdx] = 0.0f; flag_set(&e->entflags, ENTFLAG_GROUNDED, false); }
        } else if (!withinClimbAngle && g_groundedLostTime[playerIdx] <= 0.0f && wasGrounded) flag_set(&e->entflags, ENTFLAG_GROUNDED, false);
    }
}

static void SnapPlayerToFloor(Vector3* pos, u32 collisionMask) {
    for (float d = SNAP_STEP; d <= GROUND_PROBE_DIST; d += SNAP_STEP) {
        Vector3 probePos = {pos->x, pos->y - d, pos->z};
        ProbeResult probe = ProbeCapsule(probePos,(Vector3){probePos.x,probePos.y - SNAP_STEP,probePos.z},PLAYER_RADIUS,collisionMask);
        if (probe.depth > 0.0f && probe.normal.y > STEP_MIN_NORMAL_Y) { pos->y = probePos.y; return; }
    }
}

static bool AttemptStepClimb(Vector3* pos, Vector3 moveDir, Vector3* vel, u32 collisionMask) {
    Vector3 stepPos = *pos;
    stepPos.y += STEP_HEIGHT; // Check if we can fit at step height
    ProbeResult stepProbe = ProbeCapsule(stepPos,(Vector3){stepPos.x,stepPos.y - PLAYER_HEIGHT, stepPos.z},PLAYER_RADIUS,collisionMask);
    if (stepProbe.depth > 0.0f) return false;  // Can't fit at step height
    
    // Check if there's walkable ground at step height
    Vector3 stepFloorPos = {stepPos.x + moveDir.x * 0.5f, stepPos.y, stepPos.z + moveDir.z * 0.5f};
    ProbeResult floorProbe = ProbeCapsule(stepFloorPos,(Vector3){stepFloorPos.x,stepFloorPos.y - STEP_HEIGHT,stepFloorPos.z},PLAYER_RADIUS,collisionMask);
    if (floorProbe.depth > 0.0f && floorProbe.normal.y >= STEP_MIN_NORMAL_Y) {
        SnapPlayerToFloor(&stepFloorPos,collisionMask);
        *pos = stepFloorPos;
        vel->y = 0.0f;
        return true;
    }
    
    return false;
}

static void IntegratePlayer(u16 playerIdx, float dt) {
    Entity* e = &Sys_Global.instances[playerIdx];
    Vector3 pos=e->position, vel=e->velocity, newPos=Vector3_A_plus_B(pos,scale_vector3(vel,dt));
    u32 collisionMask = GetCollisionMask(e->layer);
    if (Sys_Cheats.noclip) { e->position = newPos; return; } // Noclip mode
    if (magnitude_vector3(vel) < MIN_VELOCITY_THRESHOLD) { UpdatePlayerGrounding(playerIdx,pos,collisionMask); return; } // Skip if nearly stationary
    if (GridCellBlock(playerIdx,pos,newPos)) return;
    
    Vector3 moveDir = normalize_vector3(vel);
    if (vabs(vel.y) > 1e-6f) { // ---- VERTICAL MOVEMENT ----
        bool movingDown = vel.y < 0.0f;
        Vector3 vProbeBase = pos, vProbeTip = {pos.x, pos.y + vel.y * dt, pos.z};
        ProbeResult vProbe = ProbeCapsule(vProbeBase, vProbeTip, PLAYER_RADIUS, collisionMask);
        bool blockVertical = false;
        if (movingDown) blockVertical = (vProbe.depth > 0.0f && vProbe.normal.y > 0.1f); // Check if movement is blocked
        else blockVertical = (vProbe.depth > 0.0f && vProbe.normal.y < -0.1f);
        
        if (blockVertical) vel.y = 0.0f;
        else pos.y += vel.y * dt;
    }

    Vector3 hVel = {vel.x, 0.0f, vel.z};
    if (magnitude_vector3(hVel) > 1e-6f) { // ---- HORIZONTAL MOVEMENT ----
        Vector3 hProbeBase = pos, hProbeTip = {pos.x + hVel.x * dt, pos.y, pos.z + hVel.z * dt};
        ProbeResult hProbe = ProbeCapsule(hProbeBase, hProbeTip, PLAYER_RADIUS, collisionMask);
        if (hProbe.depth > 0.0f) {
            bool isWall = hProbe.normal.y < STEP_MIN_NORMAL_Y;
            if (isWall && hProbe.normal.y > -0.3f) { // Try step climb
                if (!AttemptStepClimb(&pos,moveDir,&vel,collisionMask)) { // Step failed: slide along wall
                    float vdn = dot_vector3(hVel,hProbe.normal);
                    if (vdn < 0.0f) { vel.x -= hProbe.normal.x * vdn; vel.z -= hProbe.normal.z * vdn; }
                }
            } else if (!isWall) {} // Floor variation - let snap handle it next frame, ignore for now
        } else { pos.x += hVel.x * dt; pos.z += hVel.z * dt; }
    }
    
    SnapPlayerToFloor(&pos,collisionMask);
    UpdatePlayerGrounding(playerIdx,pos,collisionMask);
    e->lastPosition = e->position; e->position = pos; e->velocity = vel;
    e->cellX = PosGetCellCoordX(pos.x); e->cellZ = PosGetCellCoordZ(pos.z); e->cellIndex = (e->cellZ * WORLDX) + e->cellX;
    Sys_Global.dirtyInstances[playerIdx] = true;
}

void Physics(void) {
    float dt = vclamp((float)Sys_Global.timeSinceLastPhysicsTick, 0.0005f, 0.027777778f);  // Clamp to reasonable range
    if (Sys_Global.pauseRelativeTime > 2.0f) { // Apply Gravity
        for (u32 i = 0; i < dynamicEntityCount; ++i) {
            u16 entIdx = dynamicEntities[i];
            Entity* e = &Sys_Global.instances[entIdx];
            if (vabs(e->gravity - 0.0f) < 0.01f) continue;
            if (entIdx <= PLAYER2 && Sys_Cheats.noclip) continue;

            Vector3 gravityAccel = scale_vector3(GRAVITY_VECTOR,e->gravity * dt);
            e->velocity = Vector3_A_plus_B(e->velocity,gravityAccel);
        }
    }
    
    IntegratePlayer((u16)PLAYER1,dt);// IntegratePlayer((u16)PLAYER2,dt); // Move Players (separate from normal Rigidbody)
    for (u32 i = 0; i < dynamicEntityCount; ++i) {
        u16 entIdx = dynamicEntities[i];
        if (entIdx == PLAYER1 || entIdx == PLAYER2) continue;
        
        Entity* e = &Sys_Global.instances[entIdx];
        if (e->entflags & ENTFLAG_ACTIVE && magnitude_vector3(e->velocity) >= MIN_VELOCITY_THRESHOLD) { // Move Rigidbody
            Vector3 pos = e->position, vel = e->velocity;
            u32 collisionMask = GetCollisionMask(e->layer);
            float remainingTime = dt;
            for (int iteration = 0; iteration < MAX_COLLISION_ITERATIONS; ++iteration) {
                if (magnitude_vector3(vel) < MIN_VELOCITY_THRESHOLD) break;
                
                Vector3 desiredMove = scale_vector3(vel,remainingTime);
                SweepResult bestCollision = {0};
                bestCollision.toi = 1.0f;
                for (u32 i = 0; i < Sys_Global.loadedInstances; ++i) { // Test against all other entities
                    Entity* other = &Sys_Global.instances[i];
                    if (other == e || !(other->entflags&ENTFLAG_ACTIVE) || other->collider == COLLIDER_TYPE_NONE || !(collisionMask&other->layer)) continue;

                    SweepResult hit = {0};//SweepEntity(e,desiredMove,other,(u16)i);
                    if (hit.hit && hit.toi < bestCollision.toi) bestCollision = hit;
                }
                
                if (!bestCollision.hit) { pos=Vector3_A_plus_B(pos,desiredMove); break; }

                float safeTOI = vmax(bestCollision.toi - COLLISION_EPSILON,0.0f);
                pos = Vector3_A_plus_B(pos,scale_vector3(desiredMove,safeTOI));
                float vn = dot_vector3(vel,bestCollision.normal);
                if (vn < 0.0f) vel = Vector3_A_minus_B(vel,scale_vector3(bestCollision.normal,vn));
                remainingTime *= (1.0f - safeTOI);
                if (remainingTime < 0.0001f) break;
            }
            
            e->lastPosition = e->position;
            e->position = pos; e->velocity = vel;
            e->cellX = PosGetCellCoordX(pos.x); e->cellZ = PosGetCellCoordZ(pos.z); e->cellIndex = (e->cellZ * WORLDX) + e->cellX;
            Sys_Global.dirtyInstances[entIdx] = true;
        }
        
        Vector3 v = e->velocity; float mag=magnitude_vector3(v); if (mag > TERMINAL_VELOCITY) e->velocity = scale_vector3(normalize_vector3(v),TERMINAL_VELOCITY); // Clamp velocity after bounces
    }
}

ENGINE_TO_MOD bool CheckCapsule(Vector3 start,Vector3 end,float capsuleRadius,float capsuleHeight,u32 layerMask) { (void)capsuleHeight; (void)start; (void)end; (void)capsuleRadius; (void)layerMask; return false; /*TODO*/ }
ENGINE_TO_MOD void AddForce(u16 idx,Vector3 force,bool isImpulse) {
    if (idx>=INSTANCE_COUNT) return;
    Entity *e=&Sys_Global.instances[idx]; float mass=e->mass>0.0001f?e->mass:1.0f;
    e->accumulatedForce=Vector3_A_plus_B(e->accumulatedForce,force);
    if (isImpulse) e->velocity=Vector3_A_plus_B(e->velocity,scale_vector3(force,1.0f/mass));
    else           e->velocity=Vector3_A_plus_B(e->velocity,scale_vector3(force,(float)Sys_Global.timeSinceLastPhysicsTick/mass));
}

ENGINE_TO_MOD void ApplyPlayerMovements(void) {
    Entity *p = &Sys_Global.instances[PLAYER1];
    float h = (float)Forward() - (float)Backpedal(), s = (float)StrafeRight() - (float)StrafeLeft();
    Vector3 input = normalize_vector3((Vector3){p->forward.x * h + p->right.x * s, (float)SwimUp() - (float)SwimDn(), p->forward.z * h + p->right.z * s});
    float speed=GetBasePlayerSpeed(PLAYER1,magnitude_vector3(input)>0.1f)*1.75f, accel=Sys_Global.boosterActive?1.0f:3.0f;
    Vector3 cur=p->velocity, dv=Vector3_A_minus_B(scale_vector3(input,speed),cur);
    dv.x=vmax(vmin(dv.x,10.0f),-10.0f); dv.y=vmax(vmin(dv.y,10.0f),-10.0f); dv.z=vmax(vmin(dv.z,10.0f),-10.0f);
    p->velocity=Vector3_A_plus_B(cur,scale_vector3(dv,accel*(float)Sys_Global.timeSinceLastPhysicsTick));
}
