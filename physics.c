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
    float combinedRadius = aRadius + bRadius;
    Vector3 relPos = Vector3_A_minus_B(aPos, bPos);
    Vector3 relVel = aVel;  // b is assumed static
    float a = dot_vector3(relVel, relVel);
    float b = 2.0f * dot_vector3(relPos, relVel);
    float c = dot_vector3(relPos, relPos) - combinedRadius * combinedRadius;
    if (c <= 0.0f) {
        // Already overlapping - report at t=0 with current separation normal
        r.hit = true;
        r.toi = 0.0f;
        r.normal = (magnitude_vector3(relPos) > 1e-6f) ? normalize_vector3(relPos) : (Vector3){0.0f,1.0f,0.0f};
        r.point = Vector3_A_plus_B(bPos, scale_vector3(r.normal, bRadius));
        return r;
    }
    
    float discriminant = b * b - 4.0f * a * c;
    if (discriminant < 0.0f) return r;  // No collision
    
    float sqrtDisc = vsqrtf(discriminant);
    float t0 = (-b - sqrtDisc) / (2.0f * a);
    if (t0 < 0.0f || t0 > 1.0f) return r;
    
    Vector3 hitPos = Vector3_A_plus_B(aPos, scale_vector3(aVel, t0));
    r.hit = true;
    r.toi = t0;
    r.normal = normalize_vector3(Vector3_A_minus_B(hitPos, bPos));
    r.point = Vector3_A_minus_B(hitPos, scale_vector3(r.normal, aRadius));
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
    Quaternion invRot = (Quaternion){-box.rot.x,-box.rot.y,-box.rot.z,box.rot.w};
    Vector3 sLocalPos = quat_rotate_vector(invRot, Vector3_A_minus_B(sPos, box.center));
    Vector3 closestLocal = {vclamp(sLocalPos.x,-box.halfExtents.x,box.halfExtents.x),vclamp(sLocalPos.y,-box.halfExtents.y,box.halfExtents.y),vclamp(sLocalPos.z,-box.halfExtents.z,box.halfExtents.z)};
    Vector3 closestWorld = Vector3_A_plus_B(box.center,quat_rotate_vector(box.rot, closestLocal));
    return SweepSphereSphere(sPos, sRadius, sVel, closestWorld, 0.0f);
}

static SweepResult SweepBoxSphere(ShapeBox box, Vector3 bVel,
                                   Vector3 sPos, float sRadius) {
    // Swap arguments and call SweepSphereBox
    return SweepSphereBox(sPos, sRadius, scale_vector3(bVel, -1.0f), box);
}

// Box vs Box: Sample vertices of moving box, test against static box
static SweepResult SweepBoxBox(ShapeBox movingBox, Vector3 moveVel, ShapeBox staticBox) {
    SweepResult r = {0};
    Vector3 ax,ay,az; obb_axes(movingBox.rot,&ax,&ay,&az); // Get axes of moving box
    Vector3 offsets[]={{-1.0f,-1.0f,-1.0f},{-1.0f,-1.0f,1.0f},{-1.0f,1.0f,-1.0f},{-1.0f,1.0f,1.0f},{1.0f,-1.0f,-1.0f},{1.0f,-1.0f,1.0f},{1.0f,1.0f,-1.0f},{1.0f,1.0f,1.0f}}; // 8 vertices of moving box in local space
    float closestToi = 2.0f;
    Vector3 closestNormal = (Vector3){0.0f,1.0f,0.0f}, closestPoint = (Vector3){0.0f,0.0f,0.0f};
    for (int i=0;i<8;++i) {
        Vector3 vertex = (Vector3){0.0f,0.0f,0.0f};
        vertex = Vector3_A_plus_B(vertex, scale_vector3(ax, offsets[i].x * movingBox.halfExtents.x));
        vertex = Vector3_A_plus_B(vertex, scale_vector3(ay, offsets[i].y * movingBox.halfExtents.y));
        vertex = Vector3_A_plus_B(vertex, scale_vector3(az, offsets[i].z * movingBox.halfExtents.z));
        vertex = Vector3_A_plus_B(movingBox.center, vertex);
        float staticRadius = vmax(staticBox.halfExtents.x,vmax(staticBox.halfExtents.y, staticBox.halfExtents.z));
        SweepResult hit = SweepSphereSphere(vertex, 0.0f, moveVel, staticBox.center, staticRadius); // Test this vertex against static box (approximate as sphere)
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


static float SphereTriangleDistance(Vector3 sphereCenter, Vector3 a, Vector3 b, Vector3 c, Vector3 *closestPoint) {
    Vector3 edge0 = Vector3_A_minus_B(b, a); Vector3 edge1 = Vector3_A_minus_B(c, a);
    Vector3 toSphere = Vector3_A_minus_B(sphereCenter, a); // Find closest point on triangle to sphere center
    float d00 = dot_vector3(edge0, edge0);
    float d01 = dot_vector3(edge0, edge1);
    float d11 = dot_vector3(edge1, edge1);
    float d20 = dot_vector3(toSphere, edge0);
    float d21 = dot_vector3(toSphere, edge1);
    float denom = d00 * d11 - d01 * d01;
    if (vabs(denom) < 1e-6f) { // Degenerate triangle
        *closestPoint = a; return magnitude_vector3(Vector3_A_minus_B(sphereCenter, a));
    }
    
    float u = (d11 * d20 - d01 * d21) / denom; float v = (d00 * d21 - d01 * d20) / denom;
    u = vclamp(u, 0.0f, 1.0f); v = vclamp(v, 0.0f, 1.0f);
    if (u + v > 1.0f) { u = vmax(0.0f, 1.0f - v); v = vmax(0.0f, 1.0f - u); }
    Vector3 p = Vector3_A_plus_B(a, Vector3_A_plus_B(scale_vector3(edge0, u), scale_vector3(edge1, v)));
    *closestPoint = p; Vector3 delta = Vector3_A_minus_B(sphereCenter,p);
    return magnitude_vector3(delta);
}

static SweepResult SweepSphereMeshEntity(Vector3 sPos, float sRadius, Entity *meshEntity, u16 meshEntityIdx) {
    SweepResult r = {0};
    if (meshEntity->collider != COLLIDER_TYPE_MESH) return r;
    if (meshEntity->modelIndex >= loadedModelsMaxIndex) return r;
    u16 mindex = meshEntity->modelIndex;
    u32 triCount = modelTriangleCounts[mindex];
    if (triCount < 1) return r;

    float M[16]; CopyMemoryFromBtoAForNBytes(M, &modelMatrices[meshEntityIdx * 16], 16 * sizeof(float));
    float m00=M[0], m10=M[1], m20=M[2]; float m01=M[4], m11=M[5], m21=M[6]; float m02=M[8], m12=M[9], m22=M[10]; float tx=M[12], ty=M[13], tz=M[14];
    float sclx = vsqrtf(m00*m00 + m10*m10 + m20*m20); float sclx2 = sclx * sclx;
    float scly = vsqrtf(m01*m01 + m11*m11 + m21*m21); float scly2 = scly * scly;
    float sclz = vsqrtf(m02*m02 + m12*m12 + m22*m22); float sclz2 = sclz * sclz;
    Vector3 rel = {sPos.x - tx, sPos.y - ty, sPos.z - tz};
    Vector3 localSpherePos = {(rel.x*m00 + rel.y*m10 + rel.z*m20) / sclx2,(rel.x*m01 + rel.y*m11 + rel.z*m21) / scly2,(rel.x*m02 + rel.y*m12 + rel.z*m22) / sclz2}; // Transform sphere to local space
    float closestToi = 2.0f;
    Vector3 closestNormal = {0, 1, 0};
    Vector3 closestPoint = localSpherePos;
    for (u32 j = 0; j < triCount; ++j) { // Test against all triangles
        u32 bA = (u32)modelTriangles[mindex][j*3 + 0] * VERTEX_ATTRIBUTES_SIZE;
        u32 bB = (u32)modelTriangles[mindex][j*3 + 1] * VERTEX_ATTRIBUTES_SIZE;
        u32 bC = (u32)modelTriangles[mindex][j*3 + 2] * VERTEX_ATTRIBUTES_SIZE;
        Vector3 posA = {half_to_float(*(half*)(modelVertices[mindex] + bA + 0)),half_to_float(*(half*)(modelVertices[mindex] + bA + 2)),half_to_float(*(half*)(modelVertices[mindex] + bA + 4))};
        Vector3 posB = {half_to_float(*(half*)(modelVertices[mindex] + bB + 0)),half_to_float(*(half*)(modelVertices[mindex] + bB + 2)),half_to_float(*(half*)(modelVertices[mindex] + bB + 4))};
        Vector3 posC = {half_to_float(*(half*)(modelVertices[mindex] + bC + 0)),half_to_float(*(half*)(modelVertices[mindex] + bC + 2)),half_to_float(*(half*)(modelVertices[mindex] + bC + 4))};
        Vector3 closestTriPoint;
        float triDist = SphereTriangleDistance(localSpherePos, posA, posB, posC, &closestTriPoint);
        if (triDist < sRadius) {
            Vector3 edge0 = Vector3_A_minus_B(posB, posA);
            Vector3 edge1 = Vector3_A_minus_B(posC, posA);
            Vector3 triNormal = cross_vector3(edge0, edge1);
            triNormal = normalize_vector3(triNormal); // Compute normal from triangle
            if (0.0f < closestToi) { // Sphere overlaps or touches triangle.  Conservative approach: treat as hit at toi=0 (already penetrating).  More sophisticated: binary search for exact TOI
                closestToi = 0.0f;
                closestNormal = triNormal;
                closestPoint = closestTriPoint;
                r.hit = true;
            }
        }
    }
    
    if (r.hit) {
        // Transform normal and point back to world space
        Vector3 worldNormal = {(m00/sclx)*closestNormal.x + (m01/scly)*closestNormal.y + (m02/sclz)*closestNormal.z,(m10/sclx)*closestNormal.x + (m11/scly)*closestNormal.y + (m12/sclz)*closestNormal.z,(m20/sclx)*closestNormal.x + (m21/scly)*closestNormal.y + (m22/sclz)*closestNormal.z};
        worldNormal = normalize_vector3(worldNormal);
        Vector3 worldPoint = {m00*closestPoint.x + m01*closestPoint.y + m02*closestPoint.z + tx,m10*closestPoint.x + m11*closestPoint.y + m12*closestPoint.z + ty,m20*closestPoint.x + m21*closestPoint.y + m22*closestPoint.z + tz};
        r.toi = closestToi; r.normal = worldNormal; r.point = worldPoint;
    }
    
    return r;
}

static SweepResult SweepBoxMesh(Vector3 boxCenter, Vector3 boxHalfExtents, Quaternion boxRot, Entity *meshEntity, u16 meshEntityIdx) {
    SweepResult r = {0};
    if (meshEntity->collider != COLLIDER_TYPE_MESH) return r;
    if (meshEntity->modelIndex >= loadedModelsMaxIndex) return r;
    
    u16 mindex = meshEntity->modelIndex; // Simplified: test box vertices against mesh
    u32 triCount = modelTriangleCounts[mindex];
    if (triCount < 1) return r;
    
    float M[16];
    CopyMemoryFromBtoAForNBytes(M, &modelMatrices[meshEntityIdx * 16], 16 * sizeof(float));
    float m00=M[0], m10=M[1], m20=M[2]; float m01=M[4], m11=M[5], m21=M[6]; float m02=M[8], m12=M[9], m22=M[10]; float tx=M[12], ty=M[13], tz=M[14];
    float sclx = vsqrtf(m00*m00 + m10*m10 + m20*m20); float sclx2 = sclx * sclx;
    float scly = vsqrtf(m01*m01 + m11*m11 + m21*m21); float scly2 = scly * scly;
    float sclz = vsqrtf(m02*m02 + m12*m12 + m22*m22); float sclz2 = sclz * sclz;
    Vector3 ax = quat_rotate_vector(boxRot, (Vector3){1,0,0}); // Get box axes
    Vector3 ay = quat_rotate_vector(boxRot, (Vector3){0,1,0});
    Vector3 az = quat_rotate_vector(boxRot, (Vector3){0,0,1});
    Vector3 vertices[8]; Vector3 offsets[] = {{-1,-1,-1}, {-1,-1,+1}, {-1,+1,-1}, {-1,+1,+1}, {+1,-1,-1}, {+1,-1,+1}, {+1,+1,-1}, {+1,+1,+1}}; // Box vertices
    for (int i = 0; i < 8; ++i) {
        Vector3 v = (Vector3){0.0f,0.0f,0.0f};
        v = Vector3_A_plus_B(v, scale_vector3(ax, offsets[i].x * boxHalfExtents.x));
        v = Vector3_A_plus_B(v, scale_vector3(ay, offsets[i].y * boxHalfExtents.y));
        v = Vector3_A_plus_B(v, scale_vector3(az, offsets[i].z * boxHalfExtents.z));
        vertices[i] = Vector3_A_plus_B(boxCenter, v);
    }
    
    float closestToi = 2.0f;
    Vector3 closestNormal = (Vector3){0.0f,1.0f,0.0f};
    Vector3 closestPoint = boxCenter;
    for (int v_idx = 0; v_idx < 8; ++v_idx) { // Transform vertices to mesh local space and test
        Vector3 rel = {vertices[v_idx].x - tx, vertices[v_idx].y - ty, vertices[v_idx].z - tz};
        Vector3 localVertex = {(rel.x*m00 + rel.y*m10 + rel.z*m20) / sclx2,(rel.x*m01 + rel.y*m11 + rel.z*m21) / scly2,(rel.x*m02 + rel.y*m12 + rel.z*m22) / sclz2};
        for (u32 j = 0; j < triCount; ++j) {
            u32 bA = (u32)modelTriangles[mindex][j*3 + 0] * VERTEX_ATTRIBUTES_SIZE;
            u32 bB = (u32)modelTriangles[mindex][j*3 + 1] * VERTEX_ATTRIBUTES_SIZE;
            u32 bC = (u32)modelTriangles[mindex][j*3 + 2] * VERTEX_ATTRIBUTES_SIZE;
            Vector3 posA = {half_to_float(*(half*)(modelVertices[mindex] + bA + 0)),half_to_float(*(half*)(modelVertices[mindex] + bA + 2)),half_to_float(*(half*)(modelVertices[mindex] + bA + 4))};
            Vector3 posB = {half_to_float(*(half*)(modelVertices[mindex] + bB + 0)),half_to_float(*(half*)(modelVertices[mindex] + bB + 2)),half_to_float(*(half*)(modelVertices[mindex] + bB + 4))};
            Vector3 posC = {half_to_float(*(half*)(modelVertices[mindex] + bC + 0)),half_to_float(*(half*)(modelVertices[mindex] + bC + 2)),half_to_float(*(half*)(modelVertices[mindex] + bC + 4))};
            Vector3 closestTriPoint;
            float triDist = SphereTriangleDistance(localVertex, posA, posB, posC, &closestTriPoint);
            if (triDist <= 0.0f) {
                // Vertex on triangle
                Vector3 edge0 = Vector3_A_minus_B(posB, posA);
                Vector3 edge1 = Vector3_A_minus_B(posC, posA);
                Vector3 triNormal = cross_vector3(edge0, edge1);
                triNormal = normalize_vector3(triNormal);
                
                if (0.0f < closestToi) {
                    closestToi = 0.0f;
                    closestNormal = triNormal;
                    closestPoint = closestTriPoint;
                    r.hit = true;
                }
            }
        }
    }
    
    if (r.hit) {
        Vector3 worldNormal = {
            (m00/sclx)*closestNormal.x + (m01/scly)*closestNormal.y + (m02/sclz)*closestNormal.z,
            (m10/sclx)*closestNormal.x + (m11/scly)*closestNormal.y + (m12/sclz)*closestNormal.z,
            (m20/sclx)*closestNormal.x + (m21/scly)*closestNormal.y + (m22/sclz)*closestNormal.z
        };
        worldNormal = normalize_vector3(worldNormal);
        
        Vector3 worldPoint = {
            m00*closestPoint.x + m01*closestPoint.y + m02*closestPoint.z + tx,
            m10*closestPoint.x + m11*closestPoint.y + m12*closestPoint.z + ty,
            m20*closestPoint.x + m21*closestPoint.y + m22*closestPoint.z + tz
        };
        
        r.toi = closestToi;
        r.normal = worldNormal;
        r.point = worldPoint;
    }
    
    return r;
}

static SweepResult SweepCapsuleMeshEntity(Vector3 cBase, Vector3 cTip, float cRadius, Entity *meshEntity, u16 meshEntityIdx) {
    SweepResult r = {0};
    if (meshEntity->collider != COLLIDER_TYPE_MESH) return r;
    if (meshEntity->modelIndex >= loadedModelsMaxIndex) return r;
    
    u16 mindex = meshEntity->modelIndex; u32 triCount = modelTriangleCounts[mindex];
    if (triCount < 1) return r;
    
    float M[16]; CopyMemoryFromBtoAForNBytes(M,&modelMatrices[meshEntityIdx * 16], 16 * sizeof(float));
    float m00=M[0], m10=M[1], m20=M[2]; float m01=M[4], m11=M[5], m21=M[6]; float m02=M[8], m12=M[9], m22=M[10]; float tx=M[12], ty=M[13], tz=M[14];
    float sclx = vsqrtf(m00*m00 + m10*m10 + m20*m20); float sclx2 = sclx * sclx;
    float scly = vsqrtf(m01*m01 + m11*m11 + m21*m21); float scly2 = scly * scly;
    float sclz = vsqrtf(m02*m02 + m12*m12 + m22*m22); float sclz2 = sclz * sclz;
    Vector3 rel_base = {cBase.x - tx, cBase.y - ty, cBase.z - tz}; // Transform capsule to local space
    Vector3 rel_tip = {cTip.x - tx, cTip.y - ty, cTip.z - tz};
    Vector3 localCapsuleBase = {(rel_base.x*m00 + rel_base.y*m10 + rel_base.z*m20) / sclx2,(rel_base.x*m01 + rel_base.y*m11 + rel_base.z*m21) / scly2,(rel_base.x*m02 + rel_base.y*m12 + rel_base.z*m22) / sclz2};
    Vector3 localCapsuleTip = {(rel_tip.x*m00 + rel_tip.y*m10 + rel_tip.z*m20) / sclx2,(rel_tip.x*m01 + rel_tip.y*m11 + rel_tip.z*m21) / scly2,(rel_tip.x*m02 + rel_tip.y*m12 + rel_tip.z*m22) / sclz2};
    float closestToi = 2.0f; int samples = 4;
    Vector3 closestNormal = (Vector3){0.0f,1.0f,0.0f}; Vector3 closestPoint = localCapsuleBase;
    Vector3 capsuleAxis = Vector3_A_minus_B(localCapsuleTip, localCapsuleBase);
    for (int s = 0; s < samples; ++s) { // Sample capsule and test each point against all triangles
        float t = (float)s / (float)(samples - 1);
        Vector3 samplePos = Vector3_A_plus_B(localCapsuleBase, scale_vector3(capsuleAxis, t));
        for (u32 j = 0; j < triCount; ++j) {
            u32 bA = (u32)modelTriangles[mindex][j*3 + 0] * VERTEX_ATTRIBUTES_SIZE;
            u32 bB = (u32)modelTriangles[mindex][j*3 + 1] * VERTEX_ATTRIBUTES_SIZE;
            u32 bC = (u32)modelTriangles[mindex][j*3 + 2] * VERTEX_ATTRIBUTES_SIZE;
            Vector3 posA = {half_to_float(*(half*)(modelVertices[mindex] + bA + 0)),half_to_float(*(half*)(modelVertices[mindex] + bA + 2)),half_to_float(*(half*)(modelVertices[mindex] + bA + 4))};
            Vector3 posB = {half_to_float(*(half*)(modelVertices[mindex] + bB + 0)),half_to_float(*(half*)(modelVertices[mindex] + bB + 2)),half_to_float(*(half*)(modelVertices[mindex] + bB + 4))};
            Vector3 posC = {half_to_float(*(half*)(modelVertices[mindex] + bC + 0)),half_to_float(*(half*)(modelVertices[mindex] + bC + 2)),half_to_float(*(half*)(modelVertices[mindex] + bC + 4))};
            Vector3 closestTriPoint;
            float triDist = SphereTriangleDistance(samplePos,posA,posB,posC,&closestTriPoint);
            if (triDist < cRadius) {
                // Capsule sample overlaps triangle
                Vector3 edge0 = Vector3_A_minus_B(posB,posA);
                Vector3 edge1 = Vector3_A_minus_B(posC,posA);
                Vector3 triNormal = cross_vector3(edge0,edge1);
                triNormal = normalize_vector3(triNormal);
                if (0.0f < closestToi) {
                    closestToi = 0.0f;
                    closestNormal = triNormal;
                    closestPoint = closestTriPoint;
                    r.hit = true;
                }
            }
        }
    }
    
    if (r.hit) {
        // Transform back to world space
        Vector3 worldNormal = {
            (m00/sclx)*closestNormal.x + (m01/scly)*closestNormal.y + (m02/sclz)*closestNormal.z,
            (m10/sclx)*closestNormal.x + (m11/scly)*closestNormal.y + (m12/sclz)*closestNormal.z,
            (m20/sclx)*closestNormal.x + (m21/scly)*closestNormal.y + (m22/sclz)*closestNormal.z
        };
        worldNormal = normalize_vector3(worldNormal);
        
        Vector3 worldPoint = {
            m00*closestPoint.x + m01*closestPoint.y + m02*closestPoint.z + tx,
            m10*closestPoint.x + m11*closestPoint.y + m12*closestPoint.z + ty,
            m20*closestPoint.x + m21*closestPoint.y + m22*closestPoint.z + tz
        };
        
        r.toi = closestToi;
        r.normal = worldNormal;
        r.point = worldPoint;
    }
    
    return r;
}

// Capsule-box sweep (simplified: axis-aligned approach).  For oriented boxes, transform to local space.
static SweepResult SweepCapsuleBox(Vector3 sBase, Vector3 sTip, float sRadius, Vector3 sVel, ShapeBox box) {
    SweepResult r = {0};
    int numSamples = 3;
    float closestToi = 2.0f;
    Vector3 closestNormal = {0,1,0};
    Vector3 closestPoint = {0};
    for (int i = 0; i < numSamples; ++i) {
        float t = (float)i / (float)(numSamples - 1);
        Vector3 samplePos = Vector3_A_plus_B(sBase, scale_vector3(Vector3_A_minus_B(sTip, sBase), t));
        SweepResult hit = SweepSphereSphere(samplePos, sRadius, sVel, box.center, vmax(box.halfExtents.x, vmax(box.halfExtents.y, box.halfExtents.z)));
        if (hit.hit && hit.toi < closestToi) {
            closestToi = hit.toi;
            closestNormal = hit.normal;
            closestPoint = hit.point;
            r.hit = true;
        }
    }
    
    if (r.hit) {
        r.toi = closestToi;
        r.normal = closestNormal;
        r.point = closestPoint;
    }
    
    return r;
}

static SweepResult SweepEntity(Entity *moving, Vector3 moveVel, Entity *stationary, u16 stationaryIdx) {
    SweepResult r = {0};
    if (stationary->collider == COLLIDER_TYPE_MESH) { // Handle mesh entities separately
        ShapeSphere movingSphere; ShapeBox movingBox; ShapeCapsule movingCapsule;
        switch (moving->collider) {
            case COLLIDER_TYPE_SPHERE:  Entity_GetSphere(moving,&movingSphere);   return SweepSphereMeshEntity(movingSphere.center,movingSphere.radius,stationary,stationaryIdx);
            case COLLIDER_TYPE_BOX:     Entity_GetBox(moving,&movingBox);         return SweepBoxMesh(movingBox.center,movingBox.halfExtents,movingBox.rot,stationary,stationaryIdx);
            case COLLIDER_TYPE_CAPSULE: Entity_GetCapsule(moving,&movingCapsule); return SweepCapsuleMeshEntity(movingCapsule.base,movingCapsule.tip,movingCapsule.radius,stationary,stationaryIdx);
            default: return r;
        }
    }
    
    if (moving->collider == COLLIDER_TYPE_NONE || stationary->collider == COLLIDER_TYPE_NONE) return r; // Non-mesh entity dispatch (existing code)
    
    ShapeSphere movingSphere,stationarySphere; ShapeBox movingBox,stationaryBox; ShapeCapsule movingCapsule,stationaryCapsule;
    switch (moving->collider) {
        case COLLIDER_TYPE_SPHERE:
            Entity_GetSphere(moving, &movingSphere);
            switch (stationary->collider) {
                case COLLIDER_TYPE_SPHERE:  Entity_GetSphere(stationary,&stationarySphere);   return SweepSphereSphere(movingSphere.center,movingSphere.radius,moveVel,stationarySphere.center,stationarySphere.radius);
                case COLLIDER_TYPE_BOX:     Entity_GetBox(stationary,&stationaryBox);         return SweepSphereBox(movingSphere.center,movingSphere.radius,moveVel,stationaryBox);
                case COLLIDER_TYPE_CAPSULE: Entity_GetCapsule(stationary,&stationaryCapsule); return SweepSphereCapsule(movingSphere.center,movingSphere.radius,moveVel,stationaryCapsule.base,stationaryCapsule.tip,stationaryCapsule.radius);
                default: break;
            }
            break;
        case COLLIDER_TYPE_BOX:
            Entity_GetBox(moving, &movingBox);
            switch (stationary->collider) {
                case COLLIDER_TYPE_SPHERE:
                    Entity_GetSphere(stationary, &stationarySphere);
                    return SweepBoxSphere(movingBox, moveVel, stationarySphere.center, stationarySphere.radius);
                case COLLIDER_TYPE_BOX:
                    Entity_GetBox(stationary, &stationaryBox);
                    return SweepBoxBox(movingBox, moveVel, stationaryBox);
                case COLLIDER_TYPE_CAPSULE:
                    Entity_GetCapsule(stationary, &stationaryCapsule);
                    return SweepBoxCapsule(movingBox, moveVel, 
                                          stationaryCapsule.base, stationaryCapsule.tip, stationaryCapsule.radius);
                default: break;
            }
            break;
        case COLLIDER_TYPE_CAPSULE:
            Entity_GetCapsule(moving, &movingCapsule);
            switch (stationary->collider) {
                case COLLIDER_TYPE_SPHERE:
                    Entity_GetSphere(stationary, &stationarySphere);
                    return SweepCapsuleSphere(movingCapsule.base, movingCapsule.tip, movingCapsule.radius, moveVel,
                                            stationarySphere.center, stationarySphere.radius);
                case COLLIDER_TYPE_BOX:
                    Entity_GetBox(stationary, &stationaryBox);
                    return SweepCapsuleBox(movingCapsule.base, movingCapsule.tip, movingCapsule.radius, 
                                          moveVel, stationaryBox);
                case COLLIDER_TYPE_CAPSULE:
                    Entity_GetCapsule(stationary, &stationaryCapsule);
                    return SweepCapsuleCapsule(movingCapsule.base, movingCapsule.tip, movingCapsule.radius, moveVel,
                                             stationaryCapsule.base, stationaryCapsule.tip, stationaryCapsule.radius);
                default: break;
            }
            break;
        default: break;
    }
    
    return r;
}

static ProbeResult ProbeCapsule(Vector3 base, Vector3 tip, float radius, u32 collisionMask) {
    ProbeResult result = {0}; (void)tip;
    for (u32 i = 0; i < Sys_Global.loadedInstances; ++i) {
        Entity* other = &Sys_Global.instances[i];
        if (!(other->entflags & ENTFLAG_ACTIVE) || other->collider == COLLIDER_TYPE_NONE) continue;
        if (!(collisionMask & other->layer)) continue;
        
        if (other->collider == COLLIDER_TYPE_SPHERE) {
            Vector3 closestOnCapsule = base;
            Vector3 toOther = Vector3_A_minus_B(other->position, closestOnCapsule);
            float dist = magnitude_vector3(toOther);
            float minDist = radius + other->colliderSize.x;
            if (dist < minDist) {
                result.depth = minDist - dist;
                result.normal = (dist > 1e-6f) ? scale_vector3(toOther, 1.0f / dist) : (Vector3){0,1,0};
                result.point = Vector3_A_plus_B(other->position, scale_vector3(result.normal, -other->colliderSize.x));
                return result;
            }
        }
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
    //if (Sys_Global.pauseRelativeTime > 2.0f) { // Apply Gravity
        //for (u32 i = 0; i < dynamicEntityCount; ++i) {
            //u16 entIdx = dynamicEntities[i];
            //assert(entIdx >= PLAYER1 && entIdx < Sys_Global.loadedInstances);
            //Entity* e = &Sys_Global.instances[entIdx];
            //if (vabs(e->gravity - 0.0f) < 0.01f) continue;
            //if (entIdx <= PLAYER2 && Sys_Cheats.noclip) continue;

            //Vector3 gravityAccel = scale_vector3(GRAVITY_VECTOR, e->gravity * dt);
            //e->velocity = Vector3_A_plus_B(e->velocity, gravityAccel);
        //}
    //}
    
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
                
                Vector3 desiredMove = scale_vector3(vel, remainingTime);
                SweepResult bestCollision = {0};
                bestCollision.toi = 1.0f;
                for (u32 i = 0; i < Sys_Global.loadedInstances; ++i) { // Test against all other entities
                    Entity* other = &Sys_Global.instances[i];
                    if (other == e || !(other->entflags&ENTFLAG_ACTIVE) || other->collider == COLLIDER_TYPE_NONE || !(collisionMask&other->layer)) continue;

                    SweepResult hit = SweepEntity(e,desiredMove,other,(u16)i);
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
