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
        if (!ConstIndexIsPortalBlockingDoor(Sys_Global.instances[i].index)) {
            if (((gridCellStates[instCellIdx] & (CELL_VISIBLE | CELL_OPEN)) == CELL_OPEN) && (Sys_Global.instances[i].index != 754 || !SkyIsVisible())) continue;
        }
        
        u32 triCount = modelTriangleCounts[mindex];
        if (triCount < 1) continue;
        float M[16];
        CopyMemoryFromBtoAForNBytes(M,&modelMatrices[i * 16],16 * sizeof(float));
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
