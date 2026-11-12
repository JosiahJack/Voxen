// culling.c - XZ 2D World Grid Cell Culling System 64x64 matching System Shock 1.
#include "common.h"
typedef struct { u16 x,z; } PortalCell; typedef struct { PortalCell cellA,cellB; bool portalNS,open,dirty; u8 lev;} Portal;
u32 gridCellStates[ARRSIZE]; static u32 precomputedVisibleCellsFromHere[524288];/*4096 * 4096 / 32*/ u16 playerCellIdx = 0u; bool instanceIsLODArray[INSTANCE_COUNT]; static Portal activePortals[MAX_PORTALS]; u32 numActivePortals = 0; bool cullVisibilityValid;
bool get_cull_bit(const u32* arr, int idx) { return (arr[idx >> 5] >> (idx & 31)) & 1; }
INLINE void set_cull_bit(u32* arr, int idx, bool val) {u32* w = arr + (idx >> 5); u32 m = 1U << (idx & 31); *w = val ? (*w | m) : (*w & ~m);}
bool PositionVisibleFromPlayerCell(float x, float z) { return (get_cull_bit(precomputedVisibleCellsFromHere,((playerCellIdx * ARRSIZE)/*cellIdx*/ + PosGetCellCoords(x,z)/*subIdx*/)/*flat_idx*/)); }
bool XZPairInBounds(i32 x, i32 z) { return (x < WORLDX && z < WORLDZ && x >= 0 && z >= 0); }
bool SkyIsVisible() { return ((gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX) || World.curLev == LEVEL_CYBERSPACE); }
bool SkySunIsVisible() { return ((gridCellStates[playerCellIdx] & CELL_SEES_SUN) && World.curLev != LEVEL_CYBERSPACE); }
bool NeighborhoodInPVS(u16 x, u16 z, u8 r) { u32 cellIdx=(z*WORLDX)+x; for(int ix=(int)x-r;ix<=(int)x+r;++ix){for(int iz=(int)z-r;iz<=(int)z+r;++iz){if(unlikely(!XZPairInBounds(ix,iz))){continue;} int subIdx=iz*WORLDX + ix; if(get_cull_bit(precomputedVisibleCellsFromHere,cellIdx*ARRSIZE+subIdx)&&(gridCellStates[subIdx]&CELL_VISIBLE))return true;} } return false; }
static u8* LoadCullPNG(const char* name, int lev) { char p[256]; sFormat(p,sizeof(p),"./Data/%s_%d.png",name,lev); FHandle fd; int size; u8* data=OS_OpenAndAllocateFileBufferReadonly(p,&fd,&size); if (!data || size > 500000) { DualLogError("PNG missing/too-large: %s\n",p); OS_Exit(1); } i32 w,h; u8* pixels = PngLoad(data,size,&w,&h,&png_arena_main); OS_Free(data,size); if (!pixels) { DualLogError("PngLoad failed: %s\n",p); OS_Exit(1); } return pixels; }
#define PIXEL_IDX(x, z) ((x) + ((WORLDZ - 1 - (z)) * WORLDX)) * 4 // 4 channels, flip z to have desired bottom-left origin 0,0 vs png's top-left
void DetermineClosedEdges() {
    PngArenaInit(&png_arena_main); u16 totalOpenCells=0; u8* openPixels=LoadCullPNG("worldcellopen",World.curLev);
    for (i32 x=0;x<WORLDX;++x) {
        for (i32 z=0;z<WORLDZ;++z) {
            i32 cellIdx = (z * WORLDX) + x; gridCellStates[cellIdx] &= ~CELL_OPEN; i32 pixelIdx = PIXEL_IDX(x,z); u8 or = openPixels[pixelIdx + 0], og = openPixels[pixelIdx + 1], ob = openPixels[pixelIdx + 2];
            if (or > 0 || og > 0 || ob > 0) { gridCellStates[cellIdx] |= CELL_OPEN; totalOpenCells++; } else gridCellStates[cellIdx] |= CELL_CLOSEDNORTH|CELL_CLOSEDEAST|CELL_CLOSEDSOUTH|CELL_CLOSEDWEST; // Also force close the edges for closed cells even if above edges image said tweren't closed edges.
        }
    } gridCellStates[0] |= CELL_OPEN;/*Force the fallback error cell to be open (forced visible later, open is static, visible is transient)*/ u8* edgePixels = LoadCullPNG("worldedgesclosed",World.curLev);
    for (i32 x=0;x<WORLDX;x++) {
        for (i32 z=0;z<WORLDZ;z++) {
            i32 cellIdx = (z * WORLDX) + x; gridCellStates[cellIdx] &= ~(CELL_CLOSEDNORTH|CELL_CLOSEDEAST|CELL_CLOSEDSOUTH|CELL_CLOSEDWEST);/*Mark all edges not closed*/ i32 pixelIdx = PIXEL_IDX(x,z); u8 cr=edgePixels[pixelIdx + 0], cg=edgePixels[pixelIdx + 1], cb=edgePixels[pixelIdx + 2], ca=edgePixels[pixelIdx + 3];
            if (cr > 127){gridCellStates[cellIdx] |= CELL_CLOSEDNORTH;} if (cg > 127){gridCellStates[cellIdx] |= CELL_CLOSEDEAST;} if (cb > 127){gridCellStates[cellIdx] |= CELL_CLOSEDSOUTH;}
            if ((cr < 255 && cr > 0) || (cg < 255 && cg > 0) || (cb < 255 && cb > 0)) gridCellStates[cellIdx] |= CELL_CLOSEDWEST; // Anything that has closed west edge will be not at full 255 on at least one channel. Typical to use full brightness 255 on the channel(s). All 4 closed would be 128 128 128 but this doesn't ever happen. None closed is 0 0 0
            if (ca > 0 && ca < 255) gridCellStates[cellIdx] |= CELL_CLOSEDNORTH|CELL_CLOSEDEAST|CELL_CLOSEDSOUTH|CELL_CLOSEDWEST;
        }
    } u8* skyPixels = LoadCullPNG("worldcellskyvis",World.curLev);
    for (i32 x=0;x<WORLDX;++x) {
        for (i32 z=0;z<WORLDZ;++z) {
            i32 cellIdx = (z * WORLDX) + x; i32 pixelIdx = PIXEL_IDX(x,z); u8 sr=skyPixels[pixelIdx + 0], sg=skyPixels[pixelIdx + 1], sb=skyPixels[pixelIdx + 2];
            if (sr > 127 && sg < 127 && sb < 127) gridCellStates[cellIdx] &= ~(CELL_SEES_SUN|CELL_SEES_SKYBOX);/*All red cells marked as -1, no sky or sun.*/ else if (sr <= 127 && sg <= 127 && sb > 127) gridCellStates[cellIdx] |= CELL_SEES_SUN|CELL_SEES_SKYBOX;/*All blue cells marked as sky visible.  Sun + Sky.*/ 
            else { gridCellStates[cellIdx] &= ~CELL_SEES_SKYBOX; gridCellStates[cellIdx] |= CELL_SEES_SUN; }/*All white and black cells marked as 0.  Only sees Sun.*/
        }
    } OS_Free(png_arena_main.base, 16777216); png_arena_main.base = NULL; DualLog("found %d open cells...",totalOpenCells);
}
void AddDoorPortal(u16 entIdx, u16 parent) {
    if (entIdx == 499 || entIdx == 509/*Don't add bulkheads that span 2 cells, would be unnecessary complication*/){return;} if (numActivePortals >= MAX_PORTALS){DualLogWarn("Unable to add more door portals!\n"); return;}
    float nudgeAmount = 0.32f; Entity* door = &World.instances[parent]; door->portalIndex = numActivePortals; bool isOpen = (door->doorState != DoorState_Closed); // Allows for any of DoorState_Open, DoorState_Opening, or DoorState_Closing to be considered open as far as portals are concerned so we can draw objects between the door panels.
    float obj_x=World.position[parent].x, obj_z=World.position[parent].z; u16 cellCurrent=(PosGetCellCoordZ(obj_z)*WORLDX) + PosGetCellCoordX(obj_x), cellIndexUp=PosGetCellCoordZ(obj_z + nudgeAmount), cellIndexDn=PosGetCellCoordZ(obj_z - nudgeAmount), cellIndexRight=PosGetCellCoordX(obj_x + nudgeAmount), cellIndexLeft=PosGetCellCoordX(obj_x - nudgeAmount);
    u16 cellN_idx = PosGetCellCoords(obj_x,obj_z + nudgeAmount), cellS_idx = PosGetCellCoords(obj_x,obj_z - nudgeAmount), cellE_idx = PosGetCellCoords(obj_x + nudgeAmount,obj_z), cellW_idx = PosGetCellCoords(obj_x - nudgeAmount,obj_z);
    bool isNS = (cellN_idx != cellCurrent || cellS_idx != cellCurrent);
    if (isNS) { // Portal is a North     /\
                //             South pair\/
        PortalCell cellN = (PortalCell){PosGetCellCoordX(obj_x), (cellN_idx != cellCurrent) ? cellIndexUp : PosGetCellCoordZ(obj_z)}; // Ensure that cellA is always the north cell of the pair
        PortalCell cellS = (PortalCell){                cellN.x, (cellS_idx != cellCurrent) ? cellIndexDn : PosGetCellCoordZ(obj_z)};
        activePortals[numActivePortals] = (Portal){ .cellA=cellN, .cellB=cellS, .portalNS=true, .open=isOpen, .dirty=true, .lev=World.curLev };
    } else { // Portal is an East<>West pair
        PortalCell cellE = (PortalCell){(cellE_idx != cellCurrent) ? cellIndexRight : PosGetCellCoordX(obj_x), PosGetCellCoordZ(obj_z)}; // Ensure that cellA is always the east cell of the pair
        PortalCell cellW = (PortalCell){(cellW_idx != cellCurrent) ?  cellIndexLeft : PosGetCellCoordX(obj_x), cellE.z};
        activePortals[numActivePortals] = (Portal){ .cellA=cellE, .cellB=cellW, .portalNS=false, .open=isOpen, .dirty=true, .lev=World.curLev };
    } numActivePortals++;
}
bool ToggleDoorPortal(u32 p, u16 dr, u16 closedMdx) { if (p>=numActivePortals) return false; Portal* prt=&activePortals[p]; bool open=World.instances[dr].modelIndex!=closedMdx; if (prt->open==open) return false; prt->open=open; prt->dirty=true; return true; }
void ForceDoorPortalOpen(u16 p) { if (p >= numActivePortals) {return;} activePortals[p].open = true; activePortals[p].dirty = true; } // Used at load for doors that start open/ajar. Bounds-checked so non-portal doors (portalIndex==0 default) can't corrupt portal 0.
i32 CastRayCellCheck(i32 x, i32 z, i32 lastX, i32 lastZ, u32* cs) {
    if (lastX != x || lastZ != z) {
        if (XZPairInBounds(lastX, lastZ)) { 
            i32 li = (lastZ * WORLDX) + lastX; u32 cell = cs[li]; i32 dx = x - lastX, dz = z - lastZ; // -1, 0, or 1 each
            if (dz == 0) { /*Pure horizontal*/ if (((dx >  0) && (cell & CELL_CLOSEDEAST)) || ((dx < 0)  && (cell & CELL_CLOSEDWEST)))  return -1; }
            else if (dx == 0) { /*Pure vertical*/ if (((dz > 0)  && (cell & CELL_CLOSEDNORTH)) || ((dz < 0)  && (cell & CELL_CLOSEDSOUTH))) return -1; }
            else { /*Diagonal — check cell + two axis-adjacent neighbors*/
                u32 cf_ew=(dx > 0) ? CELL_CLOSEDEAST  : CELL_CLOSEDWEST, cf_ns=(dz > 0) ? CELL_CLOSEDNORTH : CELL_CLOSEDSOUTH; /*Which closed-edge flags to test depends on direction quadrant*/ bool c_ew=(cell & cf_ew) != 0, c_ns = (cell & cf_ns) != 0, n_ew=false, n_ns=false;
                u32 nf_ew=(dx > 0) ? CELL_CLOSEDEAST  : CELL_CLOSEDWEST, nf_ns=(dz > 0) ? CELL_CLOSEDNORTH : CELL_CLOSEDSOUTH; /*Neighbor sees the opposite face, same face on neighbor in NS direction same face on neighbor in EW direction*/
                i32 ni_ns_coord_x = lastX, ni_ns_coord_z = lastZ + dz; if (XZPairInBounds(ni_ns_coord_x, ni_ns_coord_z)) { u32 nsN = cs[ni_ns_coord_z * WORLDX + ni_ns_coord_x]; n_ew = (nsN & nf_ew) != 0 && (nsN & CELL_OPEN); }
                i32 ni_ew_coord_x = lastX + dx, ni_ew_coord_z = lastZ; if (XZPairInBounds(ni_ew_coord_x, ni_ew_coord_z)) { u32 ewN = cs[ni_ew_coord_z * WORLDX + ni_ew_coord_x]; n_ns = (ewN & nf_ns) != 0 && (ewN & CELL_OPEN); }
                if ((c_ns || n_ew) && (c_ew || n_ns)) return -1;
            }
        }
    } if(!XZPairInBounds(x,z)){return 0;} i32 ci = (z * WORLDX) + x; if (cs[ci] & CELL_OPEN) {cs[ci] |=  CELL_VISIBLE;} else {cs[ci] &= ~CELL_VISIBLE;} return (cs[ci] & CELL_VISIBLE) ? 1 : -1;
}
i32 CastStraight(i32 px, i32 pz, i32 sign, bool isX, u32* cs) {
    if ((isX && ((sign > 0 && px >= WORLDX - 1) || (sign < 0 && px <= 0))) || (!isX && ((sign > 0 && pz >= WORLDZ - 1) || (sign < 0 && pz <= 0))) || !XZPairInBounds(px, pz)){return isX ? px : pz;} i32 cellIdx = (pz * WORLDX) + px; if (!(cs[cellIdx] & CELL_VISIBLE)){return isX ? px : pz;}
    i32 crd = isX ? px + sign : pz + sign; i32 coordAbs = vabs(crd); bool curVis = true;
    for (; coordAbs < WORLDX; crd += sign) {
        curVis = false; i32 x = isX ? crd : px, z = isX ? pz : crd; i32 prevCoord = crd - sign; i32 prevX = isX ? prevCoord : px, prevZ = isX ? pz : prevCoord;
        if (XZPairInBounds(prevX, prevZ) && XZPairInBounds(x,z)) { i32 prevCellIdx = (prevZ * WORLDX) + prevX; if (cs[prevCellIdx] & CELL_VISIBLE) { u32 cf=isX ? sign>0 ? CELL_CLOSEDEAST : CELL_CLOSEDWEST : sign>0 ? CELL_CLOSEDNORTH : CELL_CLOSEDSOUTH; if((cs[prevCellIdx]&cf)&&cs[prevCellIdx]&CELL_OPEN)return crd; i32 subCellIdx=(z*WORLDX)+x; if(cs[subCellIdx]&CELL_OPEN){cs[subCellIdx]|=CELL_VISIBLE;}else{cs[subCellIdx]&=~CELL_VISIBLE;} curVis=true; } }
        if (!curVis) { break; }
        for (int offset = -1; offset <= 1; offset += 2) { /*Check perpendicular neighbors*/ i32 checkX = isX ? x : x + offset, checkZ = isX ? z + offset : z; if (XZPairInBounds(checkX, checkZ)) { i32 checkCellIdx = (checkZ * WORLDX) + checkX; if (CastRayCellCheck(x, z, checkX, checkZ, cs) > 0) { if (cs[checkCellIdx] & CELL_OPEN) cs[checkCellIdx] |= CELL_VISIBLE; else cs[checkCellIdx] &= ~CELL_VISIBLE; } else cs[checkCellIdx] &= ~CELL_VISIBLE; } }
    } return WORLDX * sign;
}
void CastRay(i32 x0, i32 z0, i32 x1, i32 z1, u32* cs) {
    i32 dx=vabs(x1 - x0), dz=vabs(z1 - z0); i32 sx = (x0 < x1) ? 1 : -1, sz = (z0 < z1) ? 1 : -1; i32 x=x0, z=z0; i32 lastX=x, lastZ=z; i32 err=dx - dz; i32 iter=dx > dz ? dx : dz; while(iter >= 0){if(!XZPairInBounds(x,z) || !XZPairInBounds(lastX,lastZ)){--iter; continue;} if(CastRayCellCheck(x,z,lastX,lastZ,cs) == -1){return;} lastX=x; lastZ=z; i32 e2=2*err; if(e2 > -dz){err-=dz; x+=sx;} if(e2 < dx){err+=dx; z+=sz;} --iter;}
}
void CircleFanRays(i32 x0, i32 z0, u32* cs) { if (!XZPairInBounds(x0,z0)) {return;} if (!(cs[(z0 * WORLDX) + x0] & CELL_VISIBLE)) {return;} i32 x,z,max=WORLDX,min=0; for (x=min;x<max;x++) {CastRay(x0,z0,x,min,cs);} for (x=min;x<max;x++) {CastRay(x0,z0,x,max,cs);} for (z=min;z<max;z++) {CastRay(x0,z0,min,z,cs);} for (z=min;z<max;z++) {CastRay(x0,z0,max,z,cs);} } // Cast fan out to every cell around map perimeter.
static void MarchAxis(i32 st, i32 end, i32 step, i32 ox, i32 oz, i32 sign, bool isX, u32* cs){for(i32 m=st;step>0 ? m < end : m>=end;m+=step){i32 x=isX ? m : ox,z=isX ? oz : m; if(XZPairInBounds(x,z) && (cs[(z * WORLDX) + x] & CELL_VISIBLE))m=CastStraight(isX ? m : ox,isX ? oz : m,sign,isX,cs);}}
void DetermineVisibleCells(i32 startX, i32 startZ, u32* cs) {
    if (!XZPairInBounds(startX,startZ)) { return; }
    for (i32 x=0;x<WORLDX;x++) { for (i32 z=0;z<WORLDZ;z++) { i32 subCellIdx = (z * WORLDX) + x; cs[subCellIdx] &= ~CELL_VISIBLE; } /*Clear all to not visible.*/ }
    cs[(startZ * WORLDX) + startX] |= CELL_VISIBLE; /*Force starting player cell to visible.*/
    CastStraight(startX,startZ,1,true,cs); // Cast to the right (East) [ ][3]
    MarchAxis(startX,WORLDX - 1,1,0,startZ + 1,1,1,cs);             // [1][2]
    MarchAxis(startX,WORLDX - 1,1,0,startZ - 1,1,1,cs);             // [ ][3]
    CastStraight(startX,startZ,-1,true,cs); // Cast to the left (West) [3][ ]
    MarchAxis(startX,1,-1,0,startZ + 1,-1,1,cs);                    // [2][1]
    MarchAxis(startX,1,-1,0,startZ - 1,-1,1,cs);                    // [3][ ] 
    CastStraight(startX,startZ,-1,false,cs); // Cast down (South) [ ][1][ ]
    MarchAxis(startZ,1,-1,startX + 1,0,-1,0,cs);               // [3][2][3]
    MarchAxis(startZ,1,-1,startX - 1,0,-1,0,cs);
    CastStraight(startX,startZ,1,false,cs);  // Cast   up (North) [3][2][3]
    MarchAxis(startZ,WORLDX - 1,1,startX + 1,0,1,0,cs);        // [ ][1][ ]
    MarchAxis(startZ,WORLDX - 1,1,startX - 1,0,1,0,cs);
    CircleFanRays(startX,startZ,cs); CircleFanRays(startX + 1,startZ,cs); CircleFanRays(startX + 1,startZ + 1,cs);    CircleFanRays(startX,startZ + 1,cs); CircleFanRays(startX - 1,startZ + 1,cs); CircleFanRays(startX - 1,startZ,cs);     CircleFanRays(startX - 1,startZ - 1,cs); CircleFanRays(startX,startZ - 1,cs); CircleFanRays(startX + 1,startZ - 1,cs);
    if (World.curLev == 5) { /*Citadel flight level hackarounds for algorithm discrepancies at glancing angles. Hoisted: this loop was run for every level but only ever did anything on lev 5, i.e. ~4096 dead iterations per DetermineVisibleCells call (~16.7M across a full bake).*/
        for (i32 x=0;x<WORLDX;++x) {
            for (i32 z=0;z<WORLDZ;++z) {
                i32 cellIdx_xz = (z * WORLDX) + x;
                if ((x <= 15 && startX <= 15) || (z <= 9 && startZ <= 9) || (x >= 32 && startX >= 32) || (z == 31 && startZ == 31 && x >= 27 && startX >= 27) ||  x >= 34) cs[cellIdx_xz] |= CELL_VISIBLE;                
                if (startX <=12 && x == 14 && z == 31 && startZ >= 24){cs[cellIdx_xz] |= CELL_VISIBLE;}  if (startX <=12 && x == 14 && z == 30 && startZ >= 24){cs[cellIdx_xz] |= CELL_VISIBLE;}  if (startX <=12 && x == 13 && z == 30 && startZ >= 24){cs[cellIdx_xz] |= CELL_VISIBLE;}
            }
        }
    }
}
void PortalCulling() { // Called just once at end of animation loop for the frame after each frame-perfect change to door models becoming either closed or not closed.
    static u16 previousCell; static u8 previousLevel; u16 cell=PosGetCellCoords(World.position[PLAYER1].x,World.position[PLAYER1].z); bool dirty=false;
    for (u32 p=0;p<numActivePortals;++p) if (activePortals[p].lev==World.curLev && activePortals[p].dirty) { dirty=true; break; }
    if (cullVisibilityValid && previousCell==cell && previousLevel==World.curLev && !dirty) return;
    cullVisibilityValid=true; previousCell=cell; previousLevel=World.curLev;
    bool previousLightVisible[LIGHT_COUNT]; mset(previousLightVisible,false,LIGHT_COUNT * sizeof(bool));
    for (u16 i=0;i<World.loadedLights;++i) { i32 lcell = PosGetCellCoords(World.lights[i].pos.x, World.lights[i].pos.z); if (lcell >= 0 && lcell < ARRSIZE && (gridCellStates[lcell] & CELL_VISIBLE)) {previousLightVisible[i]=true;} }
    for (u32 portalIdx=0;portalIdx<numActivePortals;++portalIdx) {
        Portal* prt = &activePortals[portalIdx]; if (!prt->dirty || prt->lev != World.curLev) continue;
        prt->dirty = false; u16 cellIdxA = (prt->cellA.z * WORLDX) + prt->cellA.x; u16 cellIdxB = (prt->cellB.z * WORLDX) + prt->cellB.x; // Guaranteed order at level load.  A = N or E, B = S or W
        if (prt->open) { if (prt->portalNS) { gridCellStates[cellIdxA] &= ~(CELL_CLOSEDSOUTH); gridCellStates[cellIdxB] &= ~(CELL_CLOSEDNORTH); } else { gridCellStates[cellIdxA] &= ~(CELL_CLOSEDWEST);  gridCellStates[cellIdxB] &= ~(CELL_CLOSEDEAST); } }
        else { if (prt->portalNS) { gridCellStates[cellIdxA] |= CELL_CLOSEDSOUTH; gridCellStates[cellIdxB] |= CELL_CLOSEDNORTH; } else { gridCellStates[cellIdxA] |= CELL_CLOSEDWEST;  gridCellStates[cellIdxB] |= CELL_CLOSEDEAST; } }
    }
    DetermineVisibleCells(PosGetCellCoordX(World.position[PLAYER1].x),PosGetCellCoordZ(World.position[PLAYER1].z),gridCellStates); // Recompute full PVS with new closed edges for all portal states.  So much for the precomputed set.
    for (u16 i=0;i<World.loadedLights;++i) { i32 lcell=PosGetCellCoords(World.lights[i].pos.x, World.lights[i].pos.z); if (lcell >= 0 && lcell < ARRSIZE && !previousLightVisible[i] && (gridCellStates[lcell] & CELL_VISIBLE)) {flag_set(&World.lights[i].lflags,LDIRTY,true);} }
    glBindBuffer(GL_SSBO,cellVisibleDataID); glBufferSubData(GL_SSBO,0,ARRSIZE * sizeof(u32), gridCellStates);
}
void CullCore() {
    playerCellIdx = PosGetCellCoords(World.position[PLAYER1].x,World.position[PLAYER1].z); if (World.curLev >= LEVEL_CYBERSPACE) return;
    u16 cellX = PosGetCellCoordX(World.position[PLAYER1].x), cellZ = PosGetCellCoordZ(World.position[PLAYER1].z); float pos_x = World.worldMin_x[World.curLev] + (cellX * CELLSZ), pos_z = World.worldMin_z[World.curLev] + (cellZ * CELLSZ);
    for (int i=0;i<World.instCount;++i) { float dx = pos_x - World.position[i].x, dz = pos_z - World.position[i].z; float distSqrd = dx*dx + dz*dz; instanceIsLODArray[i] = (distSqrd >= 655.36f);/*25.6f * 25.6f*/ } PortalCulling();
}
static void ApplyOpenPortalEdges() {
    for (u32 portalIdx=0;portalIdx<numActivePortals;++portalIdx) {
        Portal* prt = &activePortals[portalIdx]; if (!prt->open || prt->lev != World.curLev){continue;} u16 cellIdxA = (prt->cellA.z * WORLDX) + prt->cellA.x; u16 cellIdxB = (prt->cellB.z * WORLDX) + prt->cellB.x;
        if (prt->portalNS) { gridCellStates[cellIdxA] &= ~(CELL_CLOSEDSOUTH); gridCellStates[cellIdxB] &= ~(CELL_CLOSEDNORTH); } else { gridCellStates[cellIdxA] &= ~(CELL_CLOSEDWEST);  gridCellStates[cellIdxB] &= ~(CELL_CLOSEDEAST); }
    }
}
static void MarkSkyVisibleChunks() { for (u16 i = INSTS_1ST_IDX; i < World.instCount; ++i) { if (World.instances[i].index != 93/*chunk_grove1_1, the grove ceiling grate window*/) {continue;} u32 cell = (u32)PosGetCellCoords(World.position[i].x, World.position[i].z); gridCellStates[cell] |= CELL_SEES_SKYBOX|CELL_SEES_SUN; } }
INLINE void BakeRowFromStates(u32* row, const u32* cs) { for (u32 w=0; w<(ARRSIZE>>5); ++w) { const u32* s = cs + (w<<5); u32 acc = 0; for (u32 b=0;b<32;++b) acc |= (s[b] & 1u) << b; row[w] = acc; } }
typedef struct { u32 workerIdx, workerCnt; u32* cs; } CullBakeJob;
static void* CullBakeWorker(void* arg) {
    CullBakeJob* j = (CullBakeJob*)arg;
    const bool lev10 = (World.curLev == 10);
    for (u32 cell=j->workerIdx;cell<(u32)ARRSIZE;cell+=j->workerCnt) {
        DetermineVisibleCells((i32)(cell % WORLDX),(i32)(cell / WORLDX),j->cs); BakeRowFromStates(precomputedVisibleCellsFromHere + (size_t)cell * (ARRSIZE>>5),j->cs); if (lev10) { /*HacK! Fix up problem cells at odd angles.*/ i32 x = (i32)(cell % WORLDX), z = (i32)(cell / WORLDX); if ((x == 15 || x == 16) && z == 23) { size_t flat = (size_t)(cell * ARRSIZE) + ((11 * WORLDX) + 12); set_cull_bit(precomputedVisibleCellsFromHere,flat,true); } }
    } return NULL;
}
void CullInit() {
    cullVisibilityValid=false;
    if (World.curLev >= LEVEL_CYBERSPACE) { for (u32 i=0;i<ARRSIZE;++i) { gridCellStates[i] = CELL_OPEN|CELL_VISIBLE; } mset(precomputedVisibleCellsFromHere,0xFF,sizeof(precomputedVisibleCellsFromHere)); glBindBuffer(GL_SSBO,cellVisibleDataID); glBufferSubData(GL_SSBO,0,ARRSIZE*sizeof(u32),gridCellStates); return; }
    double start_time = get_time(); DualLog("Culling ");
    DetermineClosedEdges(); // For each cell, get visibility as though player were there and put into gridCellStates.  Then store the visibility of gridCellStates into the table of all visible cells for that cell at the appropriate offset for looking up later when actually re-assigning gridCellStates from this precalculated visibility state for the particular cell.
    ApplyOpenPortalEdges(); // DetermineClosedEdges() just reset every edge to its static closed state; reopen doors that start open/ajar BEFORE baking the precomputed visibility table below.
    MarkSkyVisibleChunks(); // Before the bake, so the sky bits are final by the time the precomputed table is written.
    u32 workerCnt = vclampi((i32)threadCnt, 1, 32);
    CullBakeJob jobs[32]; OS_Thread thr[31];
    if (workerCnt > 1) {
        u32* scratch = (u32*)OS_Alloc((size_t)workerCnt * ARRSIZE * sizeof(u32));
        u32 liveCnt = 1, failed[31], failedCnt = 0;
        for (u32 w=0; w<workerCnt; ++w) { jobs[w] = (CullBakeJob){ w, workerCnt, scratch + (size_t)w*ARRSIZE }; mcpy(jobs[w].cs, gridCellStates, sizeof(gridCellStates)); }
        for (u32 w=1; w<workerCnt; ++w) { if (OS_ThreadCreate(&thr[liveCnt-1], CullBakeWorker, &jobs[w]) == 0) { ++liveCnt; } else { failed[failedCnt++] = w; } }
        CullBakeWorker(&jobs[0]); /*Main thread takes worker 0's interleaved slice while the rest run.*/
        for (u32 t=0; t<liveCnt-1; ++t) OS_ThreadJoin(&thr[t]);
        for (u32 f=0; f<failedCnt; ++f) CullBakeWorker(&jobs[failed[f]]);
        DetermineVisibleCells(WORLDX - 1, WORLDZ - 1, gridCellStates);
        BakeRowFromStates(precomputedVisibleCellsFromHere + (size_t)(ARRSIZE-1) * (ARRSIZE>>5), gridCellStates);
        OS_Free(scratch, (size_t)workerCnt * ARRSIZE * sizeof(u32));
    } else {
        for (i32 z=0;z<WORLDZ;z++) {
            for (i32 x=0;x<WORLDX;x++) {
                DetermineVisibleCells(x,z,gridCellStates); i32 cell = (z * WORLDX) + x; BakeRowFromStates(precomputedVisibleCellsFromHere + (size_t)cell * (ARRSIZE>>5), gridCellStates); if (World.curLev == 10) { /*HacK! Fix up problem cells at odd angles.*/ if ((x == 15 || x == 16) && z == 23) { size_t flat = (size_t)(cell * ARRSIZE) + ((11 * WORLDX) + 12); set_cull_bit(precomputedVisibleCellsFromHere,flat,true); } }
            }
        }
    }
    playerCellIdx = PosGetCellCoords(World.position[PLAYER1].x,World.position[PLAYER1].z); i32 cellToCellIdx = playerCellIdx * ARRSIZE; for (i32 z=0;z<WORLDZ;++z) { for (i32 x=0;x<WORLDX;++x) { i32 cell = (z * WORLDX) + x; size_t flat = (size_t)(cellToCellIdx + cell); if (get_cull_bit(precomputedVisibleCellsFromHere,flat)) {gridCellStates[cell] |= CELL_VISIBLE;} } }
    gridCellStates[0] |= CELL_VISIBLE;/* Errors default to 0 so draw them anyways.*/ DualLog("....................................................took %.6f secs\n",get_time() - start_time);
}
