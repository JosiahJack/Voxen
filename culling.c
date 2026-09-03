// culling.c - XZ 2D World Grid Cell Culling System 64x64 matching System Shock 1.
#include "common.h"
u32 gridCellStates[ARRSIZE],precomputedVisibleCellsFromHere[524288]; // 4096 * 4096 / 32
u16 playerCellIdx = 0u; bool instanceIsLODArray[INSTANCE_COUNT]; Portal activePortals[MAX_PORTALS]; static u32 numActivePortals = 0;
__attribute__((pure)) bool get_cull_bit(const u32* arr, int idx) { return (arr[idx >> 5] >> (idx & 31)) & 1; }
INLINE void set_cull_bit(u32* arr, int idx, bool val) {u32* w = arr + (idx >> 5); u32 m = 1U << (idx & 31); *w = val ? (*w | m) : (*w & ~m);}
bool PositionVisibleFromPlayerCell(float x, float z) { return (get_cull_bit(precomputedVisibleCellsFromHere,((playerCellIdx * ARRSIZE)/*cellIdx*/ + PosGetCellCoords(x,z)/*subIdx*/)/*flat_idx*/)); }
INLINE bool XZPairInBounds(i32 x, i32 z) { return (x < WORLDX && z < WORLDZ && x >= 0 && z >= 0); }
bool SkyIsVisible() { return ((gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX) || World.curLev == LEVEL_CYBERSPACE); }
bool SkySunIsVisible() { return ((gridCellStates[playerCellIdx] & CELL_SEES_SUN) && World.curLev != LEVEL_CYBERSPACE); }
bool NeighborhoodInPVS(u16 cellX, u16 cellZ, u8 r) {
    u32 cellIdx = (cellZ * WORLDX) + cellX;
    for (int ix=(int)cellX-r;ix<=(int)cellX+r;++ix) { for(int iz=(int)cellZ-r;iz<=(int)cellZ+r;++iz){if (unlikely(!XZPairInBounds(ix,iz))){continue;} int subIdx=iz*WORLDX + ix; if(get_cull_bit(precomputedVisibleCellsFromHere,cellIdx * ARRSIZE + subIdx) && (gridCellStates[subIdx] & CELL_VISIBLE))return true;} }
    return false;
}

#define MAX_CULL_FILESIZE 500000
static u8* LoadCullPNG(const char* name, int level) {
    char path[256]; sFormat(path,sizeof(path),"./Data/%s_%d.png",name,level); FHandle fp=OS_OpenReadonly(path); OS_Seek(fp,0,2); size_t size = OS_Tell(fp); if (size > MAX_CULL_FILESIZE) { DualLogError("PNG too large: %s\n",path); OS_Exit(1); }
    u8* cullingFileBuffer=OS_Alloc(MAX_CULL_FILESIZE * sizeof(u8)); OS_Seek(fp,0,0); long read_size = OS_Read(fp,cullingFileBuffer,size); OS_Close(fp); if ((size_t)read_size != size) { DualLogError("Failed to read %s\n",path); OS_Exit(1); }
    i32 w,h; u8* pixels=PngLoad(cullingFileBuffer,size,&w,&h,&png_arena_main); if (!pixels) { DualLogError("STB failed: %s\n",path); OS_Exit(1); } OS_Free(cullingFileBuffer,MAX_CULL_FILESIZE * sizeof(u8)); return pixels;
}

#define PIXEL_IDX(x, z) ((x) + ((WORLDZ - 1 - (z)) * WORLDX)) * 4 // 4 channels, flip z to have desired bottom-left origin 0,0 vs png's top-left
void DetermineClosedEdges() {
    PngArenaInit(&png_arena_main); u16 totalOpenCells=0; u8* openPixels=LoadCullPNG("worldcellopen",World.curLev);
    for (i32 x=0;x<WORLDX;++x) {
        for (i32 z=0;z<WORLDZ;++z) {
            i32 cellIdx = (z * WORLDX) + x;
            gridCellStates[cellIdx] &= ~CELL_OPEN;
            i32 pixelIdx = PIXEL_IDX(x,z);
            u8 or = openPixels[pixelIdx + 0], og = openPixels[pixelIdx + 1], ob = openPixels[pixelIdx + 2];
            if (or > 0 || og > 0 || ob > 0) { gridCellStates[cellIdx] |= CELL_OPEN; totalOpenCells++; }
            else gridCellStates[cellIdx] |= CELL_CLOSEDNORTH|CELL_CLOSEDEAST|CELL_CLOSEDSOUTH|CELL_CLOSEDWEST; // Also force close the edges for closed cells even if above edges image said tweren't closed edges.
        }
    } gridCellStates[0] |= CELL_OPEN;/*Force the fallback error cell to be open (forced visible later, open is static, visible is transient)*/ u8* edgePixels = LoadCullPNG("worldedgesclosed",World.curLev);
    for (i32 x=0;x<WORLDX;x++) {
        for (i32 z=0;z<WORLDZ;z++) {
            i32 cellIdx = (z * WORLDX) + x; gridCellStates[cellIdx] &= ~(CELL_CLOSEDNORTH|CELL_CLOSEDEAST|CELL_CLOSEDSOUTH|CELL_CLOSEDWEST); // Mark all edges not closed
            i32 pixelIdx = PIXEL_IDX(x,z); u8 cr=edgePixels[pixelIdx + 0], cg=edgePixels[pixelIdx + 1], cb=edgePixels[pixelIdx + 2], ca=edgePixels[pixelIdx + 3];
            if (cr > 127){gridCellStates[cellIdx] |= CELL_CLOSEDNORTH;} if (cg > 127){gridCellStates[cellIdx] |= CELL_CLOSEDEAST;} if (cb > 127){gridCellStates[cellIdx] |= CELL_CLOSEDSOUTH;}
            if ((cr < 255 && cr > 0) || (cg < 255 && cg > 0) || (cb < 255 && cb > 0)) gridCellStates[cellIdx] |= CELL_CLOSEDWEST; // Anything that has closed west edge will be not at full 255 on at least one channel. Typical to use full brightness 255 on the channel(s). All 4 closed would be 128 128 128 but this doesn't ever happen. None closed is 0 0 0
            if (ca > 0 && ca < 255) gridCellStates[cellIdx] |= CELL_CLOSEDNORTH|CELL_CLOSEDEAST|CELL_CLOSEDSOUTH|CELL_CLOSEDWEST;
        }
    } u8* skyPixels = LoadCullPNG("worldcellskyvis",World.curLev);
    for (i32 x=0;x<WORLDX;++x) {
        for (i32 z=0;z<WORLDZ;++z) {
            i32 cellIdx = (z * WORLDX) + x; i32 pixelIdx = PIXEL_IDX(x,z); u8 sr=skyPixels[pixelIdx + 0], sg=skyPixels[pixelIdx + 1], sb=skyPixels[pixelIdx + 2];
            if (sr > 127 && sg < 127 && sb < 127) gridCellStates[cellIdx] &= ~(CELL_SEES_SUN|CELL_SEES_SKYBOX); // All red cells marked as -1, no sky or sun.
            else if (sr <= 127 && sg <= 127 && sb > 127) gridCellStates[cellIdx] |= CELL_SEES_SUN|CELL_SEES_SKYBOX; // All blue cells marked as sky visible.  Sun + Sky.
            else { gridCellStates[cellIdx] &= ~CELL_SEES_SKYBOX; gridCellStates[cellIdx] |= CELL_SEES_SUN; } // All white and black cells marked as 0.  Only sees Sun.
        }
    } OS_Free(png_arena_main.base, 16777216); png_arena_main.base = NULL; DualLog("found %d open cells...",totalOpenCells);
}

void AddDoorPortal(u16 entIdx, u16 parent) {
    if (entIdx == 499 || entIdx == 509/*Don't add bulkheads that span 2 cells, would be unnecessary complication*/){return;}
    if (numActivePortals >= MAX_PORTALS){DualLogWarn("Unable to add more door portals!\n"); return;}
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
    }
    numActivePortals++;
}

bool ToggleDoorPortal(u32 p, u16 dr, u16 closedMdx) { if (p >= MAX_PORTALS) {return false;} Portal* prt = &activePortals[p]; bool currentState=prt->open; u16 mdx=World.instances[dr].modelIndex; if (mdx == closedMdx && currentState) { prt->open=false; prt->dirty=true; } else if (mdx != closedMdx && !currentState) { prt->open=true; prt->dirty=true; } return true; }
void ForceDoorPortalOpen(u16 p) { if (p >= numActivePortals) {return;} activePortals[p].open = true; activePortals[p].dirty = true; } // Used at load for doors that start open/ajar. Bounds-checked so non-portal doors (portalIndex==0 default) can't corrupt portal 0.
i32 CastRayCellCheck(i32 x, i32 z, i32 lastX, i32 lastZ) {
    if (lastX != x || lastZ != z) {
        if (XZPairInBounds(lastX, lastZ)) { 
            i32 li = (lastZ * WORLDX) + lastX; u32 cell = gridCellStates[li]; i32 dx = x - lastX, dz = z - lastZ; // -1, 0, or 1 each
            if (dz == 0) { // Pure horizontal
                if (((dx >  0) && (cell & CELL_CLOSEDEAST)) || ((dx < 0)  && (cell & CELL_CLOSEDWEST)))  return -1;
            } else if (dx == 0) { // Pure vertical
                if (((dz > 0)  && (cell & CELL_CLOSEDNORTH)) || ((dz < 0)  && (cell & CELL_CLOSEDSOUTH))) return -1;
            } else { // Diagonal — check cell + two axis-adjacent neighbors
                u32 cf_ew=(dx > 0) ? CELL_CLOSEDEAST  : CELL_CLOSEDWEST, cf_ns=(dz > 0) ? CELL_CLOSEDNORTH : CELL_CLOSEDSOUTH; // Which closed-edge flags to test depends on direction quadrant
                bool c_ew=(cell & cf_ew) != 0, c_ns = (cell & cf_ns) != 0, n_ew=false, n_ns=false;
                u32 nf_ew=(dx > 0) ? CELL_CLOSEDEAST  : CELL_CLOSEDWEST, nf_ns=(dz > 0) ? CELL_CLOSEDNORTH : CELL_CLOSEDSOUTH;  // Neighbor sees the opposite face, same face on neighbor in NS direction same face on neighbor in EW direction
                i32 ni_ns_coord_x = lastX, ni_ns_coord_z = lastZ + dz; if (XZPairInBounds(ni_ns_coord_x, ni_ns_coord_z)) { u32 nsN = gridCellStates[ni_ns_coord_z * WORLDX + ni_ns_coord_x]; n_ew = (nsN & nf_ew) != 0 && (nsN & CELL_OPEN); }
                i32 ni_ew_coord_x = lastX + dx, ni_ew_coord_z = lastZ; if (XZPairInBounds(ni_ew_coord_x, ni_ew_coord_z)) { u32 ewN = gridCellStates[ni_ew_coord_z * WORLDX + ni_ew_coord_x]; n_ns = (ewN & nf_ns) != 0 && (ewN & CELL_OPEN); }
                if ((c_ns && c_ew) || (c_ew && n_ew) || (c_ns && n_ns) || (n_ew && n_ns)) return -1;
            }
        }
    } if(!XZPairInBounds(x,z)){return 0;} i32 ci = (z * WORLDX) + x; if (gridCellStates[ci] & CELL_OPEN) {gridCellStates[ci] |=  CELL_VISIBLE;} else {gridCellStates[ci] &= ~CELL_VISIBLE;} return (gridCellStates[ci] & CELL_VISIBLE) ? 1 : -1;
}

i32 CastStraightZ(i32 px, i32 pz, i32 signz) {
    if ((signz > 0 && pz >= (WORLDZ - 1)) || (signz < 0 && pz <= 0) || (!XZPairInBounds(px,pz))) return pz;
    i32 cellIdx = (pz * WORLDX) + px; if (!(gridCellStates[cellIdx] & CELL_VISIBLE)) return pz;
    bool currentVisible = true; i32 x=px, z=pz+signz, zabs=vabs(z);
    for (;zabs<WORLDX;z+=signz) { // Up/Down
        currentVisible = false; i32 cellIdx_x_zmnus1 = ((z - 1) * WORLDX) + x, cellIdx_x_zplus1 = ((z + 1) * WORLDX) + x;
        if (XZPairInBounds(x,z - signz) && XZPairInBounds(x,z)) {
            i32 cellIdx_x_zmnus_sign = ((z - signz) * WORLDX) + x;
            if (gridCellStates[cellIdx_x_zmnus_sign] & CELL_VISIBLE) {
                     if (signz > 0 && gridCellStates[cellIdx_x_zmnus1] & CELL_CLOSEDNORTH && gridCellStates[cellIdx_x_zmnus1] & CELL_OPEN) return z;
                else if (signz < 0 && gridCellStates[cellIdx_x_zplus1] & CELL_CLOSEDSOUTH && gridCellStates[cellIdx_x_zplus1] & CELL_OPEN) return z;
                i32 subCellIdx = (z * WORLDX) + x;
                if (gridCellStates[subCellIdx] & CELL_OPEN) gridCellStates[subCellIdx] |= CELL_VISIBLE; else gridCellStates[subCellIdx] &= ~CELL_VISIBLE;
                currentVisible = true; // Would be if twas open.
            }
        }
        if (!currentVisible) break; // Hit wall!
        if (XZPairInBounds(x + 1,z)) {
            i32 cellIdx_xplus1_z = (z * WORLDX) + x + 1;
            if (CastRayCellCheck(x,z,x + 1,z) > 0) {
                if (gridCellStates[cellIdx_xplus1_z] & CELL_OPEN) gridCellStates[cellIdx_xplus1_z] |= CELL_VISIBLE; else gridCellStates[cellIdx_xplus1_z] &= ~CELL_VISIBLE;
            } else gridCellStates[cellIdx_xplus1_z] &= ~CELL_VISIBLE;
        }
        if (XZPairInBounds(x - 1,z)) {
            i32 cellIdx_xmnus1_z = (z * WORLDX) + x - 1;
            if (CastRayCellCheck(x,z,x - 1,z) > 0) {
                if (gridCellStates[cellIdx_xmnus1_z] & CELL_OPEN) gridCellStates[cellIdx_xmnus1_z] |= CELL_VISIBLE; else gridCellStates[cellIdx_xmnus1_z] &= ~CELL_VISIBLE;
            } else gridCellStates[cellIdx_xmnus1_z] &= ~CELL_VISIBLE;
        }
    } return WORLDX * signz;
}

i32 CastStraightX(i32 px, i32 pz, i32 signx) {
    if (signx > 0 && px >= (WORLDX - 1)) return px; // Nowwhere to step to if right by edge, hence WORLDX - 1 here.
    if (signx < 0 && px <= 0) return px;
    if (!XZPairInBounds(px,pz)) return px;
    if (!(gridCellStates[(pz * WORLDX) + px] & CELL_VISIBLE)) return px;
    i32 x=px + signx, z=pz; bool currentVisible = true; i32 xabs = vabs(x);
    for (;xabs<WORLDX;x+=signx) { // Right/Left
        currentVisible = false;
        if (XZPairInBounds(x - signx,z) && XZPairInBounds(x,z)) {
            i32 cellIdx_xmnussign_z = (z * WORLDX) + x - signx;
            if (gridCellStates[cellIdx_xmnussign_z] & CELL_VISIBLE) {
                     if (signx > 0 && (gridCellStates[(z * WORLDX) + x - 1] & CELL_CLOSEDEAST) && gridCellStates[(z * WORLDX) + x - 1] & CELL_OPEN) return x;
                else if (signx < 0 && (gridCellStates[(z * WORLDX) + x + 1] & CELL_CLOSEDWEST) && gridCellStates[(z * WORLDX) + x + 1] & CELL_OPEN) return x;
                i32 subCellIdx = (z * WORLDX) + x;
                if (gridCellStates[subCellIdx] & CELL_OPEN) gridCellStates[subCellIdx] |= CELL_VISIBLE;
                else gridCellStates[subCellIdx] &= ~CELL_VISIBLE;
                currentVisible = true; // Would be if twas open.
            }
        }
        if (!currentVisible) break; // Hit wall!
        if (XZPairInBounds(x,z + 1)) {
            i32 cellIdx_x_zplus1 = ((z + 1) * WORLDX) + x;
            if (CastRayCellCheck(x,z,x,z + 1) > 0) {
                if (gridCellStates[cellIdx_x_zplus1] & CELL_OPEN) gridCellStates[cellIdx_x_zplus1] |= CELL_VISIBLE; else gridCellStates[cellIdx_x_zplus1] &= ~CELL_VISIBLE;
            } else gridCellStates[cellIdx_x_zplus1] &= ~CELL_VISIBLE;
        }
        if (XZPairInBounds(x,z - 1)) {
            i32 cellIdx_x_zmnus1 = ((z - 1) * WORLDX) + x;
            if (CastRayCellCheck(x,z,x,z - 1) > 0) {
                if (gridCellStates[cellIdx_x_zmnus1] & CELL_OPEN) gridCellStates[cellIdx_x_zmnus1] |= CELL_VISIBLE; else gridCellStates[cellIdx_x_zmnus1] &= ~CELL_VISIBLE;
            } else gridCellStates[cellIdx_x_zmnus1] &= ~CELL_VISIBLE;
        }
    }
    return WORLDX * signx;
}

void CastRay(i32 x0, i32 z0, i32 x1, i32 z1) {
    i32 dx=vabs(x1 - x0), dz=vabs(z1 - z0); i32 sx = (x0 < x1) ? 1 : -1, sz = (z0 < z1) ? 1 : -1; i32 x=x0, z=z0; i32 lastX=x, lastZ=z; i32 err=dx - dz; i32 iter=dx > dz ? dx : dz;
    while(iter >= 0){if(!XZPairInBounds(x,z) || !XZPairInBounds(lastX,lastZ)){--iter; continue;} if(CastRayCellCheck(x,z,lastX,lastZ) == -1){return;} lastX=x; lastZ=z; i32 e2=2*err; if(e2 > -dz){err-=dz; x+=sx;} if(e2 < dx){err+=dx; z+=sz;} --iter;}
}

void CircleFanRays(i32 x0, i32 z0) { if (!XZPairInBounds(x0,z0)) {return;} if (!(gridCellStates[(z0 * WORLDX) + x0] & CELL_VISIBLE)) {return;} i32 x,z,max=WORLDX,min=0; for (x=min;x<max;x++) {CastRay(x0,z0,x,min);} for (x=min;x<max;x++) {CastRay(x0,z0,x,max);} for (z=min;z<max;z++) {CastRay(x0,z0,min,z);} for (z=min;z<max;z++) {CastRay(x0,z0,max,z);} } // Cast fan out to every cell around map perimeter.
static void MarchAxis(i32 st, i32 end, i32 step, i32 ox, i32 oz, i32 sign, bool isX) { for (i32 m = st; step > 0 ? m < end : m >= end; m += step) { i32 x = isX ? m : ox, z = isX ? oz : m; if (XZPairInBounds(x,z) && (gridCellStates[(z * WORLDX) + x] & CELL_VISIBLE)) m = isX ? CastStraightX(m,oz,sign) : CastStraightZ(ox,m,sign); } }
void DetermineVisibleCells(i32 startX, i32 startZ) {
    if (!XZPairInBounds(startX,startZ)) return;
    for (i32 x=0;x<WORLDX;x++) { for (i32 z=0;z<WORLDZ;z++) { i32 subCellIdx = (z * WORLDX) + x; gridCellStates[subCellIdx] &= ~CELL_VISIBLE; } /*Clear all to not visible.*/ }
    gridCellStates[(startZ * WORLDX) + startX] |= CELL_VISIBLE; // Force starting player cell to visible.
    CastStraightX(startX,startZ,1); // Cast to the right (East) [ ][3]
    MarchAxis(startX,WORLDX - 1,1,0,startZ + 1,1,1);         // [1][2]
    MarchAxis(startX,WORLDX - 1,1,0,startZ - 1,1,1);         // [ ][3]
    CastStraightX(startX,startZ,-1); // Cast to the left (West) [3][ ]
    MarchAxis(startX,1,-1,0,startZ + 1,-1,1);                // [2][1]
    MarchAxis(startX,1,-1,0,startZ - 1,-1,1);                // [3][ ] 
    CastStraightZ(startX,startZ,-1); // Cast down (South) [ ][1][ ]
    MarchAxis(startZ,1,-1,startX + 1,0,-1,0);          // [3][2][3]
    MarchAxis(startZ,1,-1,startX - 1,0,-1,0);
    CastStraightZ(startX,startZ,1);  // Cast   up (North) [3][2][3]
    MarchAxis(startZ,WORLDX - 1,1,startX + 1,0,1,0);   // [ ][1][ ]
    MarchAxis(startZ,WORLDX - 1,1,startX - 1,0,1,0);
    CircleFanRays(startX,startZ);         CircleFanRays(startX + 1,startZ);     CircleFanRays(startX + 1,startZ + 1);
    CircleFanRays(startX,startZ + 1);     CircleFanRays(startX - 1,startZ + 1); CircleFanRays(startX - 1,startZ);
    CircleFanRays(startX - 1,startZ - 1); CircleFanRays(startX,startZ - 1);     CircleFanRays(startX + 1,startZ - 1);
    for (i32 x=0;x<WORLDX;++x) {
        for (i32 z=0;z<WORLDZ;++z) {
            i32 cellIdx_xz = (z * WORLDX) + x;
            if (World.curLev == 5) { // Citadel flight level hackarounds for algorithm discrepancies at glancing angles.
                if ((x <= 15 && startX <= 15) || (z <= 9 && startZ <= 9) || (x >= 32 && startX >= 32) || (z == 31 && startZ == 31 && x >= 27 && startX >= 27) ||  x >= 34) gridCellStates[cellIdx_xz] |= CELL_VISIBLE;                
                if (startX <=12 && x == 14 && z == 31 && startZ >= 24) gridCellStates[cellIdx_xz] |= CELL_VISIBLE; 
                if (startX <=12 && x == 14 && z == 30 && startZ >= 24) gridCellStates[cellIdx_xz] |= CELL_VISIBLE;
                if (startX <=12 && x == 13 && z == 30 && startZ >= 24) gridCellStates[cellIdx_xz] |= CELL_VISIBLE;
            }
        }
    }
}

void PortalCulling() { // Called just once at end of animation loop for the frame after each frame-perfect change to door models becoming either closed or not closed.
    bool previousLightVisible[LIGHT_COUNT]; mset(previousLightVisible,false,LIGHT_COUNT * sizeof(bool));
    for (u16 i=0;i<World.loadedLights;++i) { i32 lcell = PosGetCellCoords(World.lights[i].pos.x, World.lights[i].pos.z); if (lcell >= 0 && lcell < ARRSIZE && (gridCellStates[lcell] & CELL_VISIBLE)) {previousLightVisible[i]=true;} }
    for (u32 portalIdx=0;portalIdx<MAX_PORTALS;++portalIdx) {
        Portal* prt = &activePortals[portalIdx]; if (!prt->dirty || prt->lev != World.curLev) continue;
        prt->dirty = false; u16 cellIdxA = (prt->cellA.z * WORLDX) + prt->cellA.x; u16 cellIdxB = (prt->cellB.z * WORLDX) + prt->cellB.x; // Guaranteed order at level load.  A = N or E, B = S or W
        if (prt->open) { if (prt->portalNS) { gridCellStates[cellIdxA] &= ~(CELL_CLOSEDSOUTH); gridCellStates[cellIdxB] &= ~(CELL_CLOSEDNORTH); } else { gridCellStates[cellIdxA] &= ~(CELL_CLOSEDWEST);  gridCellStates[cellIdxB] &= ~(CELL_CLOSEDEAST); } }
        else { if (prt->portalNS) { gridCellStates[cellIdxA] |= CELL_CLOSEDSOUTH; gridCellStates[cellIdxB] |= CELL_CLOSEDNORTH; } else { gridCellStates[cellIdxA] |= CELL_CLOSEDWEST;  gridCellStates[cellIdxB] |= CELL_CLOSEDEAST; } }
    }
    DetermineVisibleCells(PosGetCellCoordX(World.position[PLAYER1].x),PosGetCellCoordZ(World.position[PLAYER1].z)); // Recompute full PVS with new closed edges for all portal states.  So much for the precomputed set.
    for (u16 i=0;i<World.loadedLights;++i) { i32 lcell=PosGetCellCoords(World.lights[i].pos.x, World.lights[i].pos.z); if (lcell >= 0 && lcell < ARRSIZE && !previousLightVisible[i] && (gridCellStates[lcell] & CELL_VISIBLE)) {flag_set(&World.lights[i].lflags,LDIRTY,true);} }
    glBindBuffer(GL_SSBO,cellVisibleDataID); glBufferData(GL_SSBO,ARRSIZE * sizeof(u32),gridCellStates,GL_DYNAMIC_DRAW);
}

void CullCore() {
    playerCellIdx = PosGetCellCoords(World.position[PLAYER1].x,World.position[PLAYER1].z); if (World.curLev >= LEVEL_CYBERSPACE) return;
    u16 cellX = PosGetCellCoordX(World.position[PLAYER1].x), cellZ = PosGetCellCoordZ(World.position[PLAYER1].z);
    float pos_x = World.worldMin_x[World.curLev] + (cellX * CELLSZ), pos_z = World.worldMin_z[World.curLev] + (cellZ * CELLSZ);
    for (int i=0;i<World.instCount;++i) { float dx = pos_x - World.position[i].x, dz = pos_z - World.position[i].z; float distSqrd = dx*dx + dz*dz; instanceIsLODArray[i] = (distSqrd >= 655.36f);/*25.6f * 25.6f*/ }
    PortalCulling(); // Update based on portal states.
}

static void ApplyOpenPortalEdges(void) { // Clears closed-edge bits for lev-matched portals currently marked open. Called from CullInit after DetermineClosedEdges() so the precomputed visibility table (and every later PVS recompute) treats open/ajar door portals as passable from the start.
    for (u32 portalIdx=0;portalIdx<MAX_PORTALS;++portalIdx) {
        Portal* prt = &activePortals[portalIdx]; if (!prt->open || prt->lev != World.curLev) continue;
        u16 cellIdxA = (prt->cellA.z * WORLDX) + prt->cellA.x; u16 cellIdxB = (prt->cellB.z * WORLDX) + prt->cellB.x;
        if (prt->portalNS) { gridCellStates[cellIdxA] &= ~(CELL_CLOSEDSOUTH); gridCellStates[cellIdxB] &= ~(CELL_CLOSEDNORTH); } else { gridCellStates[cellIdxA] &= ~(CELL_CLOSEDWEST);  gridCellStates[cellIdxB] &= ~(CELL_CLOSEDEAST); }
    }
}

void CullInit() {
    if (World.curLev == LEVEL_CYBERSPACE) return;
    double start_time = get_time(); DualLog("Culling ");
    DetermineClosedEdges(); // For each cell, get visibility as though player were there and put into gridCellStates.  Then store the visibility of gridCellStates into the table of all visible cells for that cell at the appropriate offset for looking up later when actually re-assigning gridCellStates from this precalculated visibility state for the particular cell.
    ApplyOpenPortalEdges(); // DetermineClosedEdges() just reset every edge to its static closed state; reopen doors that start open/ajar BEFORE baking the precomputed visibility table below.
    for (i32 z=0;z<WORLDZ;z++) {
        for (i32 x=0;x<WORLDX;x++) {
            DetermineVisibleCells(x,z);
            i32 cellIdx = (z * WORLDX) + x;
            for (i32 z2=0;z2<WORLDZ;z2++) { for (i32 x2=0;x2<WORLDX;x2++) { i32 subCellIdx = (z2 * WORLDX) + x2; size_t flat_idx = (size_t)(cellIdx * ARRSIZE) + subCellIdx; set_cull_bit(precomputedVisibleCellsFromHere,flat_idx,(gridCellStates[subCellIdx] & CELL_VISIBLE)); } }
            if (World.curLev == 10) { /*HacK! Fix up problem cells at odd angles.*/ if ((x == 15 || x == 16) && z == 23) { size_t flat_idx = (size_t)(cellIdx * ARRSIZE) + ((11 * WORLDX) + 12); set_cull_bit(precomputedVisibleCellsFromHere,flat_idx,true); } }
        }
    }
    playerCellIdx = PosGetCellCoords(World.position[PLAYER1].x,World.position[PLAYER1].z);
    i32 cellToCellIdx = playerCellIdx * ARRSIZE;
    for (i32 z=0;z<WORLDZ;++z) { for (i32 x=0;x<WORLDX;++x) { i32 cellIdx = (z * WORLDX) + x; size_t flat_idx = (size_t)(cellToCellIdx + cellIdx); if (get_cull_bit(precomputedVisibleCellsFromHere,flat_idx)) {gridCellStates[cellIdx] |= CELL_VISIBLE;} } } /*Get visible before putting meshes into their cells so we can nudge them a little.*/
    gridCellStates[0] |= CELL_VISIBLE;/* Errors default to 0 so draw them anyways.*/ DualLog(" took %f secs\n",get_time() - start_time);
}
