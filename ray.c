// ray.c - Raycast System.  This is an exact polygonal casting system for high accuracty separate from the physics engine entirely except for layers.
RaycastHit RayTriangle(V3 origin, V3 dir, V3 posA, V3 posB, V3 posC, V3 normA, V3 normB, V3 normC) {
    V3 edgeAB = V3_AsubB(posB,posA), edgeAC = V3_AsubB(posC,posA); V3 normalVector = V3_Cross(edgeAB,edgeAC);
    V3 ao = V3_AsubB(origin,posA); V3 dao = V3_Cross(ao,dir);
    float determinant = -V3_dot(dir,normalVector); float invDet = 1.0f / determinant; float dst = V3_dot(ao, normalVector) * invDet;
    float u = V3_dot(edgeAC,dao) * invDet, v = -V3_dot(edgeAB,dao) * invDet; float w = 1.0f - u - v;
    return (RaycastHit){.point=V3_AplusB(origin,V3_ScaleByF(dir,dst)), .normal=V3_Normalize(V3_AplusB(V3_AplusB(V3_ScaleByF(normA,w),V3_ScaleByF(normB,u)),V3_ScaleByF(normC,v))), .distance=dst, .hitInstanceIndex=INSTANCE_COUNT, .hit=vabs(determinant) >= 1E-8f && dst >= 0 && u >= 0 && v >= 0 && w >= 0};
}

RaycastHit Raycast(V3 origin, V3 dir, float maxDist, u32 layerMask) {
    RaycastHit result = { .hit = false, .distance = maxDist, .point = {0.0f, 0.0f, 0.0f}, .normal = {0.0f, 0.0f, 0.0f}, .hitInstanceIndex = INSTANCE_COUNT };
    for (u16 i = INSTS_1ST_IDX; i < INSTANCE_COUNT; ++i) {
        if (!(layerMask & World.layer[i])) continue;
        u16 mindex = World.instances[i].modelIndex; if (mindex >= mdlsCnt) continue;
        V3 objPos = World.position[i]; u16 instCellIdx = PosGetCellCoords(objPos.x,objPos.z); V3 delta = V3_AsubB(objPos,origin); float distSqrd = V3_dot(delta,delta), radBounds = vmax(modelBounds[mindex],1.81f);
        float maxDistToObj = vmax(maxDist - radBounds,maxDist); if (distSqrd >= (maxDistToObj * maxDistToObj)) continue;
        if (!IdxIsPortalBlockingDoor(World.instances[i].index)) { if(((gridCellStates[instCellIdx] & (CELL_VISIBLE | CELL_OPEN)) == CELL_OPEN) && (World.instances[i].index != 754 || !SkyIsVisible())){continue;} }
        u32 triCount = modelTriangleCounts[mindex]; if (triCount < 1) continue;
        float M[16];
        mcpy(M,&modelMatrices[i * 16],16 * sizeof(float));
        float m00=M[0], m10=M[1], m20=M[2], m01=M[4], m11=M[5], m21=M[6], m02=M[8], m12=M[9], m22=M[10], tx=M[12], ty=M[13], tz=M[14];
        float sclx = vsqrtf(m00*m00 + m10*m10 + m20*m20); float sclx2 = sclx * sclx;
        float scly = vsqrtf(m01*m01 + m11*m11 + m21*m21); float scly2 = scly * scly;
        float sclz = vsqrtf(m02*m02 + m12*m12 + m22*m22); float sclz2 = sclz * sclz;
        V3 rel = {origin.x - tx, origin.y - ty, origin.z - tz};
        V3 localOrigin = {(rel.x*m00 + rel.y*m10 + rel.z*m20) / sclx2, (rel.x*m01 + rel.y*m11 + rel.z*m21) / scly2, (rel.x*m02 + rel.y*m12 + rel.z*m22) / sclz2};
        V3 localDir =    {(dir.x*m00 + dir.y*m10 + dir.z*m20) / sclx2, (dir.x*m01 + dir.y*m11 + dir.z*m21) / scly2, (dir.x*m02 + dir.y*m12 + dir.z*m22) / sclz2};
        localDir = V3_Normalize(localDir);
        for (u32 j=0;j<triCount;++j) {
            u32 bA = (u32)modelTriangles[mindex][j*3 + 0] * CPU_VRT_SZ, bB = (u32)modelTriangles[mindex][j*3 + 1] * CPU_VRT_SZ, bC = (u32)modelTriangles[mindex][j*3 + 2] * CPU_VRT_SZ;
            V3 posA = {*(float*)(modelVertices[mindex] + bA + 0), *(float*)(modelVertices[mindex] + bA + 4), *(float*)(modelVertices[mindex] + bA + 8)};
            V3 posB = {*(float*)(modelVertices[mindex] + bB + 0), *(float*)(modelVertices[mindex] + bB + 4), *(float*)(modelVertices[mindex] + bB + 8)};
            V3 posC = {*(float*)(modelVertices[mindex] + bC + 0), *(float*)(modelVertices[mindex] + bC + 4), *(float*)(modelVertices[mindex] + bC + 8)};
            V3 normA ={*(float*)(modelVertices[mindex] + bA + 12), *(float*)(modelVertices[mindex] + bA + 16), *(float*)(modelVertices[mindex] + bA + 20)};
            V3 normB ={*(float*)(modelVertices[mindex] + bB + 12), *(float*)(modelVertices[mindex] + bB + 16), *(float*)(modelVertices[mindex] + bB + 20)};
            V3 normC ={*(float*)(modelVertices[mindex] + bC + 12), *(float*)(modelVertices[mindex] + bC + 16), *(float*)(modelVertices[mindex] + bC + 20)};
            RaycastHit tryTri = RayTriangle(localOrigin,localDir,posA,posB,posC,normA,normB,normC); if (!tryTri.hit) continue;
            V3 worldPoint = { m00*tryTri.point.x + m01*tryTri.point.y + m02*tryTri.point.z + tx, m10*tryTri.point.x + m11*tryTri.point.y + m12*tryTri.point.z + ty, m20*tryTri.point.x + m21*tryTri.point.y + m22*tryTri.point.z + tz };
            float worldDist = V3_Dist(worldPoint,origin); if (worldDist >= result.distance) continue;
            V3 worldNormal = { (m00/sclx)*tryTri.normal.x + (m01/scly)*tryTri.normal.y + (m02/sclz)*tryTri.normal.z, (m10/sclx)*tryTri.normal.x + (m11/scly)*tryTri.normal.y + (m12/sclz)*tryTri.normal.z, (m20/sclx)*tryTri.normal.x + (m21/scly)*tryTri.normal.y + (m22/sclz)*tryTri.normal.z };
            worldNormal = V3_Normalize(worldNormal);
            result.hit=true; result.point=worldPoint; result.normal=V3_Normalize(worldNormal); result.distance=worldDist; result.hitInstanceIndex=i;
        }
    }
    return result;
}
 
void RaycastAll(V3 origin, V3 dir, float distance, u32 layerMask, RaycastHit* hits, u16 maxCount) { for (int i=0;i<maxCount;++i) {hits[i].hit = false;} (void)origin; (void)dir; (void)distance; (void)layerMask; }
RaycastHit CapsuleCast(V3 start, V3 end, float capsuleRadius, float castDist, u32 layerMask, bool hitTriggers) { RaycastHit result = { .hit = false }; (void)start; (void)end; (void)capsuleRadius; (void)castDist; (void)layerMask; (void)hitTriggers; return result; }
