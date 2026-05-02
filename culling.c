// dynamic_culling.c - Culling functions for x,z grid based culling system ala System Shock 1 / Underworld style
#include "os.h"
#include "voxen.h"
typedef struct { u16 x,z; } PortalCell;
typedef struct { PortalCell cellA,cellB,cellA2,cellB2; bool portalNS,open,dirty,isBulkhead;} Portal;
u8 *stbi_load_from_memory(const u8* buffer, i32 len, i32* x, i32* y);
void stbi__arena_init_thread(StbiArena* arena);
extern StbiArena stbi_arena_main;
#define STBI_ARENA_SIZE 16 * 1024 * 1024
u32 gridCellStates[ARRSIZE];
u32 precomputedVisibleCellsFromHere[524288]; // 4096 * 4096 / 32
u16 playerCellIdx = 0u;
bool instanceIsLODArray[INSTANCE_COUNT];
#define MAX_CULL_FILESIZE 500000
Portal activePortals[MAX_PORTALS];
static u8 numActivePortals = 0;
__attribute__((pure)) bool get_cull_bit(const u32* arr, int idx) { return (arr[idx >> 5] >> (idx & 31)) & 1; }
static inline __attribute__((always_inline)) void set_cull_bit(u32* arr, int idx, bool val) {u32* w = arr + (idx >> 5); u32 m = 1U << (idx & 31); *w = val ? (*w | m) : (*w & ~m);}
ENGINE_TO_MOD i32 PosGetCellCoords(float x, float z) { return (PosGetCellCoordZ(z) * WORLDX) + PosGetCellCoordX(x); }
extern u16 playerCellIdx;
ENGINE_TO_MOD bool PositionVisibleFromPlayerCell(float x, float z) {
    i32 subIdx = PosGetCellCoords(x,z);
    int cellIdx = (playerCellIdx * ARRSIZE);
    int flat_idx = cellIdx + subIdx;
    return (get_cull_bit(precomputedVisibleCellsFromHere,flat_idx));
}

static inline __attribute__((always_inline)) bool XZPairInBounds(i32 x, i32 z) { return (x < WORLDX && z < WORLDZ && x >= 0 && z >= 0); }
bool SkyIsVisible(void) { return ((gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX) || Sys_Global.currentLevel == LEVEL_CYBERSPACE); }
bool SkySunIsVisible(void) { return ((gridCellStates[playerCellIdx] & CELL_SEES_SUN) && Sys_Global.currentLevel != LEVEL_CYBERSPACE); }
bool NeighborhoodInPVS(u16 cellX, u16 cellZ, int r) {
    u32 cellIdx = (cellZ * WORLDX) + cellX;
    for (int ix = (int)cellX-r; ix <= (int)cellX+r; ++ix) {
        for (int iz = (int)cellZ-r; iz <= (int)cellZ+r; ++iz) {
            if (unlikely(!XZPairInBounds(ix,iz))) continue;

            int subIdx = iz * WORLDX + ix;
            if (get_cull_bit(precomputedVisibleCellsFromHere, cellIdx * ARRSIZE + subIdx) && (gridCellStates[subIdx] & CELL_VISIBLE)) return true;
        }
    }
    return false;
}

bool GridCellBlock(u16 i,Vector3 pos,Vector3 newPos) {
    i32 ccx=PosGetCellCoordX(pos.x),    ccz=PosGetCellCoordZ(pos.z);
    i32 ncx=PosGetCellCoordX(newPos.x), ncz=PosGetCellCoordZ(newPos.z);
    i32 cc=(ccz*WORLDX)+ccx, nc=(ncz*WORLDX)+ncx;
    if (ncz>ccz && (gridCellStates[cc]&CELL_CLOSEDNORTH)) { Sys_Global.instances[i].velocity.z=-0.1f; return true; }
    if (ncz<ccz && (gridCellStates[cc]&CELL_CLOSEDSOUTH)) { Sys_Global.instances[i].velocity.z= 0.1f; return true; }
    if (ncx>ccx && (gridCellStates[cc]&CELL_CLOSEDEAST))  { Sys_Global.instances[i].velocity.x=-0.1f; return true; }
    if (ncx<ccx && (gridCellStates[cc]&CELL_CLOSEDWEST))  { Sys_Global.instances[i].velocity.x= 0.1f; return true; }
    if (!(gridCellStates[nc]&CELL_OPEN)) { Vector3 dir=normalize_vector3(Sys_Global.instances[i].velocity); Sys_Global.instances[i].velocity=scale_vector3(dir,-0.1f); return true; }
    return false;
}

static unsigned char* LoadCullPNG(const char* name, int level) {
    char path[256]; StringFormat(path, sizeof(path),"./Data/%s_%d.png",name,level);
    OsFileHandle fp = OS_OpenReadonly(path);
    OS_Seek(fp,0,SEEK_END); size_t size = OS_Tell(fp);
    if (size > MAX_CULL_FILESIZE) { DualLogError("PNG too large: %s\n",path); OS_Exit(1); }
    
    u8* cullingFileBuffer = OS_Alloc(MAX_CULL_FILESIZE * sizeof(u8));
    OS_Seek(fp,0,SEEK_SET); size_t read_size = OS_Read(fp,cullingFileBuffer,size); OS_Close(fp);
    if (read_size != size) { DualLogError("Failed to read %s\n",path); OS_Exit(1); }
    
    int w, h; unsigned char* pixels = stbi_load_from_memory(cullingFileBuffer,size,&w,&h);
    if (!pixels) { DualLogError("STB failed: %s\n",path); OS_Exit(1); }
    OS_DeallocateRAM(cullingFileBuffer,MAX_CULL_FILESIZE * sizeof(u8));
    return pixels;
}

#define PIXEL_IDX(x, z) ((x) + ((WORLDZ - 1 - (z)) * WORLDX)) * 4 // 4 channels, flip z to have desired bottom-left origin 0,0 vs stbi_load's top-left
void DetermineClosedEdges(void) {
    stbi__arena_init_thread(&stbi_arena_main); u16 totalOpenCells = 0;
    unsigned char* openPixels = LoadCullPNG("worldcellopen",Sys_Global.currentLevel);
    for (i32 x=0;x<WORLDX;++x) {
        for (i32 z=0;z<WORLDZ;++z) {
            i32 cellIdx = (z * WORLDX) + x;
            gridCellStates[cellIdx] &= ~CELL_OPEN;
            i32 pixelIdx = PIXEL_IDX(x,z);
            unsigned char or = openPixels[pixelIdx + 0], og = openPixels[pixelIdx + 1], ob = openPixels[pixelIdx + 2];
            if (or > 0 || og > 0 || ob > 0) { gridCellStates[cellIdx] |= CELL_OPEN; totalOpenCells++; }
            else gridCellStates[cellIdx] |= CELL_CLOSEDNORTH|CELL_CLOSEDEAST|CELL_CLOSEDSOUTH|CELL_CLOSEDWEST; // Also force close the edges for closed cells even if above edges image said tweren't closed edges.
        }
    }

    gridCellStates[0] |= CELL_OPEN; // Force the fallback error cell to be open (forced visible later, open is static, visible is transient)
    unsigned char* edgePixels = LoadCullPNG("worldedgesclosed",Sys_Global.currentLevel);
    for (i32 x=0;x<WORLDX;x++) {
        for (i32 z=0;z<WORLDZ;z++) {
            i32 cellIdx = (z * WORLDX) + x;
            gridCellStates[cellIdx] &= ~(CELL_CLOSEDNORTH|CELL_CLOSEDEAST|CELL_CLOSEDSOUTH|CELL_CLOSEDWEST); // Mark all edges not closed
            i32 pixelIdx = PIXEL_IDX(x,z);
            unsigned char cr = edgePixels[pixelIdx + 0], cg = edgePixels[pixelIdx + 1], cb = edgePixels[pixelIdx + 2], ca = edgePixels[pixelIdx + 3];
            if (cr > 127) gridCellStates[cellIdx] |= CELL_CLOSEDNORTH;
            if (cg > 127) gridCellStates[cellIdx] |= CELL_CLOSEDEAST;
            if (cb > 127) gridCellStates[cellIdx] |= CELL_CLOSEDSOUTH;
            if ((cr < 255 && cr > 0) || (cg < 255 && cg > 0) || (cb < 255 && cb > 0)) gridCellStates[cellIdx] |= CELL_CLOSEDWEST; // Anything that has closed west edge will be not at full 255 on at least one channel. Typical for all other edge conditions is to use full brightness 255 on the channel(s). All 4 closed would be 128 128 128 but this doesn't ever happen. None closed is 0 0 0
            if (ca > 0 && ca < 255) gridCellStates[cellIdx] |= CELL_CLOSEDNORTH|CELL_CLOSEDEAST|CELL_CLOSEDSOUTH|CELL_CLOSEDWEST;
        }
    }
        
    unsigned char* skyPixels = LoadCullPNG("worldcellskyvis",Sys_Global.currentLevel);
    for (i32 x=0;x<WORLDX;++x) {
        for (i32 z=0;z<WORLDZ;++z) {
            i32 cellIdx = (z * WORLDX) + x; i32 pixelIdx = PIXEL_IDX(x,z);
            unsigned char sr = skyPixels[pixelIdx + 0], sg = skyPixels[pixelIdx + 1], sb = skyPixels[pixelIdx + 2];
            if (sr > 127 && sg < 127 && sb < 127) gridCellStates[cellIdx] &= ~(CELL_SEES_SUN|CELL_SEES_SKYBOX); // All red cells marked as -1, no sky or sun.
            else if (sr <= 127 && sg <= 127 && sb > 127) gridCellStates[cellIdx] |= CELL_SEES_SUN|CELL_SEES_SKYBOX; // All blue cells marked as sky visible.  Sun + Sky.
            else { gridCellStates[cellIdx] &= ~CELL_SEES_SKYBOX; gridCellStates[cellIdx] |= CELL_SEES_SUN; } // All white and black cells marked as 0.  Only sees Sun.
        }
    }
    
    OS_DeallocateRAM(stbi_arena_main.base, STBI_ARENA_SIZE); stbi_arena_main.base = NULL;
    DualLog("found %d open cells...",totalOpenCells);
}

ENGINE_TO_MOD void AddDoorPortal(u16 entIdx, u16 parent) {
    if (entIdx == 499 || entIdx == 509) return; // Don't add bulkheads
    float nudgeAmount = 0.32f;
    Entity* door = &Sys_Global.instances[parent];
    door->portalIndex = numActivePortals;
    bool isOpen = (door->doorState != DoorState_Closed); // Allows for any of DoorState_Open, DoorState_Opening, or DoorState_Closing to be considered open as far as portals are concerned so we can draw objects between the door panels.
    float obj_x = door->position.x; float obj_z = door->position.z;
    u16 cellIndexCurrentX = PosGetCellCoordX(obj_x); u16 cellIndexCurrentZ = PosGetCellCoordZ(obj_z);
    u16 cellCurrent = (cellIndexCurrentZ * WORLDX) + cellIndexCurrentX;
    u16    cellIndexUp = PosGetCellCoordZ(obj_z + nudgeAmount), cellIndexDn = PosGetCellCoordZ(obj_z - nudgeAmount);
    u16 cellIndexRight = PosGetCellCoordX(obj_x + nudgeAmount), cellIndexLeft = PosGetCellCoordX(obj_x - nudgeAmount);
    u16 cellN_idx = PosGetCellCoords(obj_x, obj_z + nudgeAmount), cellS_idx = PosGetCellCoords(obj_x, obj_z - nudgeAmount);
    u16 cellE_idx = PosGetCellCoords(obj_x + nudgeAmount, obj_z), cellW_idx = PosGetCellCoords(obj_x - nudgeAmount, obj_z);
    bool isNS = (cellN_idx != cellCurrent || cellS_idx != cellCurrent);
    if (isNS) { // Portal is a North     /\
                //             South pair\/
        PortalCell cellN, cellS;
        cellN.x = cellS.x = PosGetCellCoordX(obj_x);
        cellN.z = (cellN_idx != cellCurrent) ? cellIndexUp : cellIndexCurrentZ; // Ensure that cellA is always the north cell of the pair
        cellS.z = (cellS_idx != cellCurrent) ? cellIndexDn : cellIndexCurrentZ;
        activePortals[numActivePortals] = (Portal){ .cellA=cellN, .cellB=cellS, .portalNS=true, .open=isOpen, .dirty=true };
    } else { // Portal is an East<>West pair
        PortalCell cellE, cellW;
        cellE.z = cellW.z = PosGetCellCoordZ(obj_z);
        cellE.x = (cellE_idx != cellCurrent) ? cellIndexRight : cellIndexCurrentX; // Ensure that cellA is always the east cell of the pair
        cellW.x = (cellW_idx != cellCurrent) ? cellIndexLeft : cellIndexCurrentX;
        activePortals[numActivePortals] = (Portal){ .cellA=cellE, .cellB=cellW, .portalNS=false, .open=isOpen, .dirty=true };
    }
    
    numActivePortals++;
}

ENGINE_TO_MOD bool ToggleDoorPortal(u8 portalIdx, u16 doorIdx, u16 closedModelIndex) {
    if (portalIdx >= MAX_PORTALS) return false;

    Portal* prt = &activePortals[portalIdx];
    bool currentState = prt->open; u16 mdx = Sys_Global.instances[doorIdx].modelIndex;
         if (mdx == closedModelIndex &&  currentState) { prt->open = false; prt->dirty = true; }
    else if (mdx != closedModelIndex && !currentState) { prt->open =  true; prt->dirty = true; }
    return true;
}

i32 CastRayCellCheck(i32 x, i32 z, i32 lastX, i32 lastZ) {
    if (lastX != x || lastZ != z) {
        if (XZPairInBounds(lastX, lastZ)) { 
            i32 li = (lastZ * WORLDX) + lastX;
            u32 cell = gridCellStates[li];
            i32 dx = x - lastX, dz = z - lastZ; // -1, 0, or 1 each
            if (dz == 0) { // Pure horizontal
                if ((dx >  0) && (cell & CELL_CLOSEDEAST))  return -1;
                if ((dx < 0)  && (cell & CELL_CLOSEDWEST))  return -1;
            } else if (dx == 0) { // Pure vertical
                if ((dz > 0)  && (cell & CELL_CLOSEDNORTH)) return -1;
                if ((dz < 0)  && (cell & CELL_CLOSEDSOUTH)) return -1;
            } else { // Diagonal — check cell + two axis-adjacent neighbors
                u32 cf_ew  = (dx > 0) ? CELL_CLOSEDEAST  : CELL_CLOSEDWEST; // Which closed-edge flags to test depends on direction quadrant
                u32 cf_ns  = (dz > 0) ? CELL_CLOSEDNORTH : CELL_CLOSEDSOUTH;
                bool c_ew = (cell & cf_ew) != 0;
                bool c_ns = (cell & cf_ns) != 0;
                u32 nf_ew  = (dx > 0) ? CELL_CLOSEDEAST  : CELL_CLOSEDWEST;   // Neighbor sees the opposite face, same face on neighbor in NS direction
                u32 nf_ns  = (dz > 0) ? CELL_CLOSEDNORTH : CELL_CLOSEDSOUTH;  // same face on neighbor in EW direction
                bool n_ew = false;
                bool n_ns = false;

                i32 ni_ns_coord_x = lastX;
                i32 ni_ns_coord_z = lastZ + dz;
                if (XZPairInBounds(ni_ns_coord_x, ni_ns_coord_z)) {
                    u32 nsN = gridCellStates[ni_ns_coord_z * WORLDX + ni_ns_coord_x];
                    n_ew = (nsN & nf_ew) != 0 && (nsN & CELL_OPEN);
                }

                i32 ni_ew_coord_x = lastX + dx;
                i32 ni_ew_coord_z = lastZ;
                if (XZPairInBounds(ni_ew_coord_x, ni_ew_coord_z)) {
                    u32 ewN = gridCellStates[ni_ew_coord_z * WORLDX + ni_ew_coord_x];
                    n_ns = (ewN & nf_ns) != 0 && (ewN & CELL_OPEN);
                }

                if ((c_ns && c_ew) || (c_ew && n_ew) || (c_ns && n_ns) || (n_ew && n_ns)) return -1;
            }
        }
    }
 
    if (!XZPairInBounds(x,z)) return 0;
    i32 ci = (z * WORLDX) + x;
    if (gridCellStates[ci] & CELL_OPEN) gridCellStates[ci] |=  CELL_VISIBLE;
    else                                gridCellStates[ci] &= ~CELL_VISIBLE;
    return (gridCellStates[ci] & CELL_VISIBLE) ? 1 : -1;
}

i32 CastStraightZ(i32 px, i32 pz, i32 signz) {
    if (signz > 0 && pz >= (WORLDZ - 1)) return pz; // Nowwhere to step to if right by edge, hence WORLDX - 1 here.
    if (signz < 0 && pz <= 0) return pz;
    if (!XZPairInBounds(px,pz)) return pz;
    
    i32 cellIdx = (pz * WORLDX) + px;
    if (!(gridCellStates[cellIdx] & CELL_VISIBLE)) return pz;
    
    bool currentVisible = true; i32 x=px, z=pz+signz, zabs=vabs(z);
    for (;zabs<WORLDX;z+=signz) { // Up/Down
        currentVisible = false;
        i32 cellIdx_x_zmnus1 = ((z - 1) * WORLDX) + x, cellIdx_x_zplus1 = ((z + 1) * WORLDX) + x;
        if (XZPairInBounds(x,z - signz) && XZPairInBounds(x,z)) {
            i32 cellIdx_x_zmnus_sign = ((z - signz) * WORLDX) + x;
            if (gridCellStates[cellIdx_x_zmnus_sign] & CELL_VISIBLE) {
                if (signz > 0) {
                    if (gridCellStates[cellIdx_x_zmnus1] & CELL_CLOSEDNORTH && gridCellStates[cellIdx_x_zmnus1] & CELL_OPEN) return z;
                } else if (signz < 0) {
                    if (gridCellStates[cellIdx_x_zplus1] & CELL_CLOSEDSOUTH && gridCellStates[cellIdx_x_zplus1] & CELL_OPEN) return z;
                }

                i32 subCellIdx = (z * WORLDX) + x;
                if (gridCellStates[subCellIdx] & CELL_OPEN) gridCellStates[subCellIdx] |= CELL_VISIBLE;
                else gridCellStates[subCellIdx] &= ~CELL_VISIBLE;
                
                currentVisible = true; // Would be if twas open.
            }
        }

        if (!currentVisible) break; // Hit wall!

        if (XZPairInBounds(x + 1,z)) {
            i32 cellIdx_xplus1_z = (z * WORLDX) + x + 1;
            if (CastRayCellCheck(x,z,x + 1,z) > 0) {
                if (gridCellStates[cellIdx_xplus1_z] & CELL_OPEN) gridCellStates[cellIdx_xplus1_z] |= CELL_VISIBLE;
                else gridCellStates[cellIdx_xplus1_z] &= ~CELL_VISIBLE;
            } else gridCellStates[cellIdx_xplus1_z] &= ~CELL_VISIBLE;
        }
        
        if (XZPairInBounds(x - 1,z)) {
            i32 cellIdx_xmnus1_z = (z * WORLDX) + x - 1;
            if (CastRayCellCheck(x,z,x - 1,z) > 0) {
                if (gridCellStates[cellIdx_xmnus1_z] & CELL_OPEN) gridCellStates[cellIdx_xmnus1_z] |= CELL_VISIBLE;
                else gridCellStates[cellIdx_xmnus1_z] &= ~CELL_VISIBLE;
            } else gridCellStates[cellIdx_xmnus1_z] &= ~CELL_VISIBLE;
        }
    }
    
    return WORLDX * signz;
}

i32 CastStraightX(i32 px, i32 pz, i32 signx) {
    if (signx > 0 && px >= (WORLDX - 1)) return px; // Nowwhere to step to if right by edge, hence WORLDX - 1 here.
    if (signx < 0 && px <= 0) return px;
    if (!XZPairInBounds(px,pz)) return px;
    if (!(gridCellStates[(pz * WORLDX) + px] & CELL_VISIBLE)) return px;

    i32 x = px + signx;
    i32 z = pz;
    bool currentVisible = true;
    i32 xabs = vabs(x);
    for (;xabs<WORLDX;x+=signx) { // Right/Left
        currentVisible = false;
        if (XZPairInBounds(x - signx,z) && XZPairInBounds(x,z)) {
            i32 cellIdx_xmnussign_z = (z * WORLDX) + x - signx;
            if (gridCellStates[cellIdx_xmnussign_z] & CELL_VISIBLE) {
                if (signx > 0) {
                    if ((gridCellStates[(z * WORLDX) + x - 1] & CELL_CLOSEDEAST) && gridCellStates[(z * WORLDX) + x - 1] & CELL_OPEN) return x;
                } else if (signx < 0) {
                    if ((gridCellStates[(z * WORLDX) + x + 1] & CELL_CLOSEDWEST) && gridCellStates[(z * WORLDX) + x + 1] & CELL_OPEN) return x;
                }
                
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
                if (gridCellStates[cellIdx_x_zplus1] & CELL_OPEN) gridCellStates[cellIdx_x_zplus1] |= CELL_VISIBLE;
                else gridCellStates[cellIdx_x_zplus1] &= ~CELL_VISIBLE;
            } else {
                gridCellStates[cellIdx_x_zplus1] &= ~CELL_VISIBLE;
            }
        }
        
        if (XZPairInBounds(x,z - 1)) {
            i32 cellIdx_x_zmnus1 = ((z - 1) * WORLDX) + x;
            if (CastRayCellCheck(x,z,x,z - 1) > 0) {
                if (gridCellStates[cellIdx_x_zmnus1] & CELL_OPEN) gridCellStates[cellIdx_x_zmnus1] |= CELL_VISIBLE;
                else gridCellStates[cellIdx_x_zmnus1] &= ~CELL_VISIBLE;
            } else {
                gridCellStates[cellIdx_x_zmnus1] &= ~CELL_VISIBLE;
            }
        }
    }
    
    return WORLDX * signx;
}

void CastRay(i32 x0, i32 z0, i32 x1, i32 z1) {
    i32 dx = vabs(x1 - x0);      i32 dz = vabs(z1 - z0);
    i32 sx = (x0 < x1) ? 1 : -1; i32 sz = (z0 < z1) ? 1 : -1;
    i32 x = x0;                  i32 z = z0;
    i32 lastX = x;               i32 lastZ = z;
    i32 err = dx - dz;
    i32 iter = dx > dz ? dx : dz;
    while (iter >= 0) {
        if (!XZPairInBounds(x,z) || !XZPairInBounds(lastX,lastZ)) { --iter; continue; }
        if (CastRayCellCheck(x,z,lastX,lastZ) == -1) return;

        lastX = x; lastZ = z;
        i32 e2 = 2 * err;
        if (e2 > -dz) { err -= dz; x += sx; }
        if (e2 <  dx) { err += dx; z += sz; }
        --iter;
    }
}

void CircleFanRays(i32 x0, i32 z0) { // CastRay()'s in fan from x0,z0 out to every cell around map perimeter.
    if (!XZPairInBounds(x0,z0)) return;
    if (!(gridCellStates[(z0 * WORLDX) + x0] & CELL_VISIBLE)) return;

    i32 x,z,max=WORLDX,min=0; // Reduce work slightly by not casting towards edges but 1 less = [1,63].
    for (x=min;x<max;x++) CastRay(x0,z0,x,min);
    for (x=min;x<max;x++) CastRay(x0,z0,x,max);
    for (z=min;z<max;z++) CastRay(x0,z0,min,z);
    for (z=min;z<max;z++) CastRay(x0,z0,max,z);
}

void DetermineVisibleCells(i32 startX, i32 startZ) {
    if (!XZPairInBounds(startX,startZ)) return;

    for (i32 x=0;x<WORLDX;x++) {
        for (i32 z=0;z<WORLDZ;z++) {
            i32 subCellIdx = (z * WORLDX) + x;
            gridCellStates[subCellIdx] &= ~CELL_VISIBLE; // Clear all to not visible.
        }
    }

    i32 cellIdx_start = (startZ * WORLDX) + startX;
    gridCellStates[cellIdx_start] |= CELL_VISIBLE; // Force starting player cell to visible.
    // Cast to the right (East)        [ ][3]
    CastStraightX(startX,startZ,1); // [1][2]
    i32 iter = 0;               // [ ][3]
    for (i32 march=startX;march<(WORLDX - 1);march++) {
        iter++;
        if (iter > WORLDX) break;
        
        if (XZPairInBounds(march,startZ + 1)) {
            if (gridCellStates[((startZ + 1) * WORLDX) + march] & CELL_VISIBLE) march = CastStraightX(march,startZ + 1,1);  // Above [1]
        }
    }
    
    iter = 0;
    for (i32 march=startX;march<(WORLDX - 1);march++) {
        iter++;
        if (iter > WORLDX) break;

        if (XZPairInBounds(march,startZ - 1)) {
            if (gridCellStates[((startZ - 1) * WORLDX) + march] & CELL_VISIBLE) march = CastStraightX(march,startZ - 1,1);  // Below [1]
        }
    }
    // Cast to the left (West)          [3][ ]
    CastStraightX(startX,startZ,-1); // [2][1]
    iter = 0;                        // [3][ ]
    for (i32 march=startX;march>=1;march--) {
        iter++;
        if (iter > WORLDX) break;
        
        if (XZPairInBounds(march,startZ + 1)) {
            if (gridCellStates[((startZ + 1) * WORLDX) + march] & CELL_VISIBLE) march = CastStraightX(march,startZ + 1,-1); // Above [1]
        }
    }

    iter = 0;
    for (i32 march=startX;march>=1;march--) {
        iter++;
        if (iter > WORLDX) break;

        if (XZPairInBounds(march,startZ - 1)) {
            if (gridCellStates[((startZ - 1) * WORLDX) + march] & CELL_VISIBLE) march = CastStraightX(march,startZ - 1,-1); // Below [1]
        }
    }
    // Cast down (South)                [ ][1][ ]
    CastStraightZ(startX,startZ,-1); // [3][2][3]
    iter = 0;
    for (i32 march=startZ;march>=1;march--) {
        iter++;
        if (iter > WORLDX) break;

        if (XZPairInBounds(startX + 1,march)) {
            if (gridCellStates[(march * WORLDX) + startX + 1] & CELL_VISIBLE) march = CastStraightZ(startX + 1,march,-1);
        }
    }
    
    iter = 0;
    for (i32 march=startZ;march>=1;march--) {
        iter++;
        if (iter > WORLDX) break;

        if (XZPairInBounds(startX - 1,march)) {
            if (gridCellStates[(march * WORLDX) + startX - 1] & CELL_VISIBLE) march = CastStraightZ(startX - 1,march,-1);
        }
    }
    // Cast up (North)                 [3][2][3]
    CastStraightZ(startX,startZ,1); // [ ][1][ ]
    iter = 0;
    for (i32 march=startZ;march<(WORLDX - 1);march++) {
        iter++;
        if (iter > WORLDX) break;

        if (XZPairInBounds(startX + 1,march)) {
            if (gridCellStates[(march * WORLDX) + startX + 1] & CELL_VISIBLE) march = CastStraightZ(startX + 1,march,1);
        }
    }
    
    iter = 0;
    for (i32 march=startZ;march<(WORLDX - 1);march++) {
        iter++;
        if (iter > WORLDX) break;

        if (XZPairInBounds(startX - 1,march)) {
            if (gridCellStates[(march * WORLDX) + startX - 1] & CELL_VISIBLE) march = CastStraightZ(startX - 1,march,1);
        }
    }

    CircleFanRays(startX,startZ);
    CircleFanRays(startX + 1,startZ);
    CircleFanRays(startX + 1,startZ + 1);
    CircleFanRays(startX,startZ + 1);
    CircleFanRays(startX - 1,startZ + 1);
    CircleFanRays(startX - 1,startZ);
    CircleFanRays(startX - 1,startZ - 1);
    CircleFanRays(startX,startZ - 1);
    CircleFanRays(startX + 1,startZ - 1);
    for (i32 x=0;x<WORLDX;++x) {
        for (i32 z=0;z<WORLDZ;++z) {
            i32 cellIdx_xz = (z * WORLDX) + x;
            if (Sys_Global.currentLevel == 5) { // Citadel flight level hackarounds for algorithm discrepancies at glancing angles.
                if (   (x <= 15 && startX <= 15) || (z <= 9 && startZ <= 9)
                    || (x >= 32 && startX >= 32)
                    || (z == 31 && startZ == 31 && x >= 27 && startX >= 27)
                    ||  x >= 34) {
                    
                    gridCellStates[cellIdx_xz] |= CELL_VISIBLE;
                }
                
                if (startX <=12 && x == 14 && z == 31 && startZ >= 24) gridCellStates[cellIdx_xz] |= CELL_VISIBLE;
                if (startX <=12 && x == 14 && z == 30 && startZ >= 24) gridCellStates[cellIdx_xz] |= CELL_VISIBLE;
                if (startX <=12 && x == 13 && z == 30 && startZ >= 24) gridCellStates[cellIdx_xz] |= CELL_VISIBLE;
            }
        }
    }
}

bool CullCore(void);
void CullInit(void) {
    double start_time = get_time();    
    DualLog("Culling ");
    if (Sys_Global.currentLevel == LEVEL_CYBERSPACE) return;

    DetermineClosedEdges(); // For each cell, get the visibility as though player were there and put into gridCellStates.  Then store the visibility of gridCellStates into the table of all visible cells for that cell at the appropriate offset for looking up later when actually re-assigning gridCellStates from this precalculated visibility state for the particular cell.
    for (i32 z=0;z<WORLDZ;z++) {
        for (i32 x=0;x<WORLDX;x++) {
            DetermineVisibleCells(x,z);
            i32 cellIdx = (z * WORLDX) + x;
            for (i32 z2=0;z2<WORLDZ;z2++) {
                for (i32 x2=0;x2<WORLDX;x2++) {
                    i32 subCellIdx = (z2 * WORLDX) + x2;
                    size_t flat_idx = (size_t)(cellIdx * ARRSIZE) + subCellIdx;
                    bool is_visible = (gridCellStates[subCellIdx] & CELL_VISIBLE);
                    set_cull_bit(precomputedVisibleCellsFromHere,flat_idx,is_visible);
                }
            }
            
            if (Sys_Global.currentLevel == 10) {
                if ((x == 15 || x == 16) && z == 23) { // Fix up problem cells at odd angle where ddx doesn't work.
                    size_t flat_idx = (size_t)(cellIdx * ARRSIZE) + ((11 * WORLDX) + 12);
                    set_cull_bit(precomputedVisibleCellsFromHere,flat_idx,true);
                }
            }
        }
    }
    
    playerCellIdx = PosGetCellCoords(Sys_Global.instances[PLAYER1].position.x,Sys_Global.instances[PLAYER1].position.z);
    i32 cellToCellIdx = playerCellIdx * ARRSIZE;
    for (i32 z=0;z<WORLDZ;++z) {
        for (i32 x=0;x<WORLDX;++x) {
            i32 cellIdx = (z * WORLDX) + x;
            size_t flat_idx = (size_t)(cellToCellIdx + cellIdx);
            if (get_cull_bit(precomputedVisibleCellsFromHere,flat_idx)) gridCellStates[cellIdx] |= CELL_VISIBLE; // Get visible before putting meshes into their cells so we can nudge them a little.
        }
    }

    gridCellStates[0] |= CELL_VISIBLE; // Errors default here so draw them anyways.
    CullCore(); // Do first Cull pass, forcing as player moved to new cell.
    DualLog(" took %f secs\n",get_time() - start_time);
}

extern Light lights[LIGHT_COUNT]; void UploadGridCellVisibility(void);
ENGINE_TO_MOD void PortalCulling(void) { // Called just once at end of animation loop for the frame after each frame perfect change to door models becoming either closed or not closed.
    u16 playerCellX = PosGetCellCoordX(Sys_Global.instances[PLAYER1].position.x);
    u16 playerCellZ = PosGetCellCoordZ(Sys_Global.instances[PLAYER1].position.z);
    bool previousLightVisible[LIGHT_COUNT];
    MemSetToValueForNBytes(previousLightVisible,false,LIGHT_COUNT * sizeof(bool));
    for (u16 i=0;i<Sys_Global.loadedLights;++i) {
        u16 lcell = (lights[i].pos.z * WORLDX) + lights[i].pos.x;
        if (gridCellStates[lcell] & CELL_VISIBLE) previousLightVisible[i] = true;
    }
    
    PortalCell cellA, cellB;
    for (u8 portalIdx=0;portalIdx<MAX_PORTALS;++portalIdx) {
        Portal* prt = &activePortals[portalIdx];
        if (!prt->dirty) continue;
        
        prt->dirty = false;
        cellA = prt->cellA; cellB = prt->cellB; // Guaranteed order at level load.  A = N or E, B = S or W
        bool isNS = prt->portalNS;
        u16 cellIdxA = (cellA.z * WORLDX) + cellA.x;
        u16 cellIdxB = (cellB.z * WORLDX) + cellB.x;
        if (prt->open) { // Open the edges up
            if (isNS) { gridCellStates[cellIdxA] &= ~(CELL_CLOSEDSOUTH); gridCellStates[cellIdxB] &= ~(CELL_CLOSEDNORTH); }
            else { gridCellStates[cellIdxA] &= ~(CELL_CLOSEDWEST); gridCellStates[cellIdxB] &= ~(CELL_CLOSEDEAST); }
        } else {
            if (isNS) { gridCellStates[cellIdxA] |= CELL_CLOSEDSOUTH; gridCellStates[cellIdxB] |= CELL_CLOSEDNORTH; }
            else { gridCellStates[cellIdxA] |= CELL_CLOSEDWEST; gridCellStates[cellIdxB] |= CELL_CLOSEDEAST; }
        }
    }
    
    DetermineVisibleCells(playerCellX,playerCellZ); // Recompute full PVS with new closed edges for all portal states.  So much for the precomputed set.
    for (u16 i=0;i<Sys_Global.loadedLights;++i) {
        u16 lcell = (lights[i].pos.z * WORLDX) + lights[i].pos.x;
        if (!previousLightVisible[i] && (gridCellStates[lcell] & CELL_VISIBLE)) flag_setu32(&lights[i].lflags,LDIRTY,true);
    }
    UploadGridCellVisibility();
}

static inline __attribute__((always_inline)) void CellCoordsToPos(u16 x, u16 z, float* xf, float* xz) { *xf = Sys_Global.worldMin_x + (x * CELL_SIZE); *xz = Sys_Global.worldMin_z + (z * CELL_SIZE); }
bool CullCore(void) {
    playerCellIdx = PosGetCellCoords(Sys_Global.instances[PLAYER1].position.x,Sys_Global.instances[PLAYER1].position.z);
    if (Sys_Global.currentLevel >= LEVEL_CYBERSPACE) return false;

    float pos_x,pos_z;
    u16 cellX = (u16)clamp((i32)vfloor((Sys_Global.instances[PLAYER1].position.x - Sys_Global.worldMin_x + CELLXHALF) / CELL_SIZE),0,WORLDX_0BASED);
    u16 cellZ = (u16)clamp((i32)vfloor((Sys_Global.instances[PLAYER1].position.z - Sys_Global.worldMin_z + CELLXHALF) / CELL_SIZE),0,WORLDX_0BASED);
    CellCoordsToPos(cellX,cellZ,&pos_x,&pos_z);
    for (int i=0;i<Sys_Global.loadedInstances;++i) {
        float distSqrd = squareDistance2D(Sys_Global.instances[i].position.x,Sys_Global.instances[i].position.z,pos_x,pos_z);
        instanceIsLODArray[i] = (distSqrd >= 655.36f); // 25.6f * 25.6f
    }
    
    PortalCulling(); // Update based on portal states.
    return true;
}
