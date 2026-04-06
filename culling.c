// dynamic_culling.c - Culling functions for x,z grid based culling system ala System Shock 1 / Underworld style
#include "os.h"
#include "gl.h"
#include "voxen.h"
uint8_t *stbi_load_from_memory(const uint8_t* buffer, int32_t len, int32_t* x, int32_t* y);
extern void stbi__arena_init(void);
extern uint8_t*  stbi__arena_base;
#define STBI_ARENA_SIZE 16 * 1024 * 1024
uint32_t gridCellStates[ARRSIZE];
uint32_t precomputedVisibleCellsFromHere[(ARRSIZE * ARRSIZE) / 32]; // 4096 * 4096 / 32bits
uint16_t playerCellIdx = 0u;
bool instanceIsLODArray[INSTANCE_COUNT];
#define MAX_CULL_FILESIZE 500000
uint8_t cullingFileBuffer[MAX_CULL_FILESIZE];
Portal activePortals[MAX_PORTALS];
static uint8_t numActivePortals = 0;
__attribute__((pure)) bool get_cull_bit(const uint32_t* arr, int idx) { return (arr[idx >> 5] >> (idx & 31)) & 1; }
static inline __attribute__((always_inline)) void set_cull_bit(uint32_t* arr, int idx, bool val) {uint32_t* w = arr + (idx >> 5); uint32_t m = 1U << (idx & 31); *w = val ? (*w | m) : (*w & ~m);}
ENGINE_TO_MOD int32_t PosGetCellCoords(float x, float z) { return (PosGetCellCoordZ(z) * WORLDX) + PosGetCellCoordX(x); }
extern uint16_t playerCellIdx;
extern FrustumPlane lightFrustumPlanes[LIGHT_COUNT][6][6],playerFrustumPlanes[6];
extern CamView camViews[MAX_CAMVIEWS];
ENGINE_TO_MOD bool PositionVisibleFromPlayerCell(float x, float z) {
    int32_t subIdx = PosGetCellCoords(x,z);
    int cellIdx = (playerCellIdx * ARRSIZE);
    int flat_idx = cellIdx + subIdx;
    return (get_cull_bit(precomputedVisibleCellsFromHere,flat_idx));
}

static inline __attribute__((always_inline)) bool XZPairInBounds(int32_t x, int32_t z) { return (x < WORLDX && z < WORLDZ && x >= 0 && z >= 0); }
static inline bool vCellVisible(float x, float z) { return (gridCellStates[PosGetCellCoords(x,z)] & 1) > 0; }
bool VoxelOrNeighborVisible(float x, float z) {
    if (vCellVisible(x,z)) return true;
    if (vCellVisible(x + 1.30,z)) return true;
    if (vCellVisible(x - 1.30,z)) return true;
    if (vCellVisible(x,z + 1.30)) return true;
    if (vCellVisible(x,z - 1.30)) return true;
    return false;
}

bool NeighborhoodInPVS(uint16_t cellX, uint16_t cellZ, int r) {
    uint32_t cellIdx = (cellZ * WORLDX) + cellX;
    for (int ix = (int)cellX-r; ix <= (int)cellX+r; ++ix) {
        for (int iz = (int)cellZ-r; iz <= (int)cellZ+r; ++iz) {
            if (unlikely(!XZPairInBounds(ix,iz))) continue;
                    
            int subIdx = iz * WORLDX + ix;
            if (get_cull_bit(precomputedVisibleCellsFromHere, cellIdx * ARRSIZE + subIdx) && (gridCellStates[subIdx] & CELL_VISIBLE)) return true;
        }
    }
    return false;
}

bool CellNotVisible(uint32_t instCellIdx, uint32_t cellX, uint32_t cellZ, uint32_t cellRadiusCount) {
    bool inPVS = (gridCellStates[instCellIdx] & CELL_VISIBLE);
    if (!inPVS) inPVS = NeighborhoodInPVS(cellX,cellZ,cellRadiusCount);
    if (!inPVS) return true;
    return false;
}

bool CheckLightNotInPVS(Vector3 lightPos, float range) {
    uint16_t cellX = (uint16_t)clamp((int32_t)vfloor((lightPos.x - Sys_Global.worldMin_x + CELLXHALF) / CELL_SIZE), 0, WORLDX_0BASED);
    uint16_t cellZ = (uint16_t)clamp((int32_t)vfloor((lightPos.z - Sys_Global.worldMin_z + CELLXHALF) / CELL_SIZE), 0, WORLDX_0BASED);
    uint32_t lightCellIdx = (cellZ * WORLDX) + cellX;
    uint32_t r = vceil(range * (1.0f / CELL_SIZE));
    if (CellNotVisible(lightCellIdx,cellX,cellZ,r)) return true;
    return false;
}

bool SkyIsVisible(void) { return ((gridCellStates[playerCellIdx] & CELL_SEES_SKYBOX) || Sys_Global.currentLevel == LEVEL_CYBERSPACE); }
bool SkySunIsVisible(void) { return ((gridCellStates[playerCellIdx] & CELL_SEES_SUN) && Sys_Global.currentLevel != LEVEL_CYBERSPACE); }

extern float modelBounds[MODEL_IDX_MAX]; extern uint16_t editModeSelection,loadedTexturesMaxIndex;
bool EntNotVisible(uint16_t i, bool otherCondition) { Entity* e = &Sys_Global.instances[i]; return e->texIndex > loadedTexturesMaxIndex || !(e->entflags & ENTFLAG_ACTIVE) || e->index >= MAX_ENTITIES || e->modelIndex >= MODEL_IDX_MAX || e->texIndex >= MAX_VALID_TEXTURE || otherCondition; }
__attribute__((pure)) bool SphereInFrustum(FrustumPlane* planes, Vector3 c, float radius) { for (int i=0;i<6;++i) { if ((dot_vector3(planes[i].normal,c) + planes[i].d) < -radius) return false; } return true; }
bool DetermineIfInstanceVisible(uint16_t i, bool otherCondition, bool skyVisible, Vector3 playerPos, float* distSqrd) {
    if (EntNotVisible(i,otherCondition)) return false; // must be transparent && transparents or neither
    
    Entity* e = &Sys_Global.instances[i];
    uint16_t cellX = PosGetCellCoordX(e->position.x), cellZ = PosGetCellCoordZ(e->position.z);
    uint16_t instCellIdx = (cellZ * WORLDX) + cellX; uint16_t entIdx = e->index;
    Vector3 delta = Vector3_A_minus_B(e->position,playerPos);
    *distSqrd = delta.x*delta.x + delta.y*delta.y + delta.z*delta.z;
    float radius = modelBounds[e->modelIndex] * 2.0f * vmax(vmax(e->scale.x,e->scale.y),e->scale.z);
    if (!SphereInFrustum(playerFrustumPlanes,e->position,radius) && (entIdx != 754 || !skyVisible) && i != editModeSelection) return false;
    
    if (ConstIndexIsPortalBlockingDoor(entIdx)) { // Extra checks only needed for opaque portal blocking doors.
        if (CellNotVisible(instCellIdx,cellX,cellZ,2)) return false;
    } else {
        if (!(Sys_Global.currentLevel == 1 && (entIdx == 309 || entIdx == 532))) { // Hack for beaker and beaker holder on level 1 shelf getting culled from door portals.
            if (((gridCellStates[instCellIdx] & (CELL_VISIBLE | CELL_OPEN)) == CELL_OPEN) && (entIdx != 754 || !skyVisible)) return false; // For some shelves that are inset away from cells, need to still draw their items by checking && CELL_OPEN here, unfortunately this means they don't ever get culled :(
        }
        
        if (!(gridCellStates[instCellIdx] & CELL_OPEN) && *distSqrd >= 943.7184f && (entIdx != 754 || !skyVisible)) return false; // 30.72 * 30.72, 12 cells
    }
    
    // One frame delay is fine for cam views to become visible
    if (Sys_Global.instances[i].camView != 255) camViews[Sys_Global.instances[i].camView].visible = true;
    return true;
}


bool LevelSpecificHacksForClosedCellsThatProbablyShouldntBeBecauseOfInsetMeshes(uint32_t instCellIdx, uint16_t constIndex) {
    if (!(Sys_Global.currentLevel == 1 && (constIndex == 309 || constIndex == 532)) && !ConstIndexIsPortalBlockingDoor(constIndex)) {
        if (((gridCellStates[instCellIdx] & (CELL_VISIBLE | CELL_OPEN)) == CELL_OPEN) && (constIndex != 754 || !SkyIsVisible())) return true;
    }
    
    return false;
}

bool GridCellBlock(uint16_t i,Vector3 pos,Vector3 newPos) {
    int32_t ccx=PosGetCellCoordX(pos.x),    ccz=PosGetCellCoordZ(pos.z);
    int32_t ncx=PosGetCellCoordX(newPos.x), ncz=PosGetCellCoordZ(newPos.z);
    int32_t cc=(ccz*WORLDX)+ccx, nc=(ncz*WORLDX)+ncx;
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
    
    OS_Seek(fp,0,SEEK_SET); size_t read_size = OS_Read(fp,cullingFileBuffer,size); OS_Close(fp);
    if (read_size != size) { DualLogError("Failed to read %s\n",path); OS_Exit(1); }
    
    int w, h; unsigned char* pixels = stbi_load_from_memory(cullingFileBuffer,size,&w,&h);
    if (!pixels) { DualLogError("STB failed: %s\n",path); OS_Exit(1); }
    return pixels;
}

#define PIXEL_IDX(x, z) ((x) + ((WORLDZ - 1 - (z)) * WORLDX)) * 4 // 4 channels, flip z to have desired bottom-left origin 0,0 vs stbi_load's top-left
void DetermineClosedEdges(void) {
    stbi__arena_init(); uint16_t totalOpenCells = 0;
    unsigned char* openPixels = LoadCullPNG("worldcellopen",Sys_Global.currentLevel);
    for (int32_t x=0;x<WORLDX;++x) {
        for (int32_t z=0;z<WORLDZ;++z) {
            int32_t cellIdx = (z * WORLDX) + x;
            gridCellStates[cellIdx] &= ~CELL_OPEN;
            int32_t pixelIdx = PIXEL_IDX(x,z);
            unsigned char or = openPixels[pixelIdx + 0], og = openPixels[pixelIdx + 1], ob = openPixels[pixelIdx + 2];
            if (or > 0 || og > 0 || ob > 0) { gridCellStates[cellIdx] |= CELL_OPEN; totalOpenCells++; }
            else gridCellStates[cellIdx] |= CELL_CLOSEDNORTH|CELL_CLOSEDEAST|CELL_CLOSEDSOUTH|CELL_CLOSEDWEST; // Also force close the edges for closed cells even if above edges image said tweren't closed edges.
        }
    }

    gridCellStates[0] |= CELL_OPEN; // Force the fallback error cell to be open (forced visible later, open is static, visible is transient)
    unsigned char* edgePixels = LoadCullPNG("worldedgesclosed",Sys_Global.currentLevel);
    for (int32_t x=0;x<WORLDX;x++) {
        for (int32_t z=0;z<WORLDZ;z++) {
            int32_t cellIdx = (z * WORLDX) + x;
            gridCellStates[cellIdx] &= ~(CELL_CLOSEDNORTH|CELL_CLOSEDEAST|CELL_CLOSEDSOUTH|CELL_CLOSEDWEST); // Mark all edges not closed
            int32_t pixelIdx = PIXEL_IDX(x,z);
            unsigned char cr = edgePixels[pixelIdx + 0], cg = edgePixels[pixelIdx + 1], cb = edgePixels[pixelIdx + 2], ca = edgePixels[pixelIdx + 3];
            if (cr > 127) gridCellStates[cellIdx] |= CELL_CLOSEDNORTH;
            if (cg > 127) gridCellStates[cellIdx] |= CELL_CLOSEDEAST;
            if (cb > 127) gridCellStates[cellIdx] |= CELL_CLOSEDSOUTH;
            if ((cr < 255 && cr > 0) || (cg < 255 && cg > 0) || (cb < 255 && cb > 0)) gridCellStates[cellIdx] |= CELL_CLOSEDWEST; // Anything that has closed west edge will be not at full 255 on at least one channel. Typical for all other edge conditions is to use full brightness 255 on the channel(s). All 4 closed would be 128 128 128 but this doesn't ever happen. None closed is 0 0 0
            if (ca > 0 && ca < 255) gridCellStates[cellIdx] |= CELL_CLOSEDNORTH|CELL_CLOSEDEAST|CELL_CLOSEDSOUTH|CELL_CLOSEDWEST;
        }
    }
        
    unsigned char* skyPixels = LoadCullPNG("worldcellskyvis",Sys_Global.currentLevel);
    for (int32_t x=0;x<WORLDX;++x) {
        for (int32_t z=0;z<WORLDZ;++z) {
            int32_t cellIdx = (z * WORLDX) + x; int32_t pixelIdx = PIXEL_IDX(x,z);
            unsigned char sr = skyPixels[pixelIdx + 0], sg = skyPixels[pixelIdx + 1], sb = skyPixels[pixelIdx + 2];
            if (sr > 127 && sg < 127 && sb < 127) gridCellStates[cellIdx] &= ~(CELL_SEES_SUN|CELL_SEES_SKYBOX); // All red cells marked as -1, no sky or sun.
            else if (sr <= 127 && sg <= 127 && sb > 127) gridCellStates[cellIdx] |= CELL_SEES_SUN|CELL_SEES_SKYBOX; // All blue cells marked as sky visible.  Sun + Sky.
            else { gridCellStates[cellIdx] &= ~CELL_SEES_SKYBOX; gridCellStates[cellIdx] |= CELL_SEES_SUN; } // All white and black cells marked as 0.  Only sees Sun.
        }
    }
    
    OS_DeallocateRAM(stbi__arena_base, STBI_ARENA_SIZE); stbi__arena_base = NULL;
    DualLog("found %d open cells...",totalOpenCells);
    DebugRAM("end of dynamic culling DetermineClosedEdges");
}

ENGINE_TO_MOD void AddDoorPortal(uint16_t entIdx, uint16_t parent) {
    float nudgeAmount = entIdx == 499 || entIdx == 509 ? 3.84f : 0.32f; // Bulkhead and giant elevator door need to nudge further to be sure.
    Entity* door = &Sys_Global.instances[parent];
    door->portalIndex = numActivePortals;
    bool isOpen = (door->doorState != DoorState_Closed); // Allows for any of DoorState_Open, DoorState_Opening, or DoorState_Closing to be considered open as far as portals are concerned so we can draw objects between the door panels.
    float obj_x = door->position.x; float obj_z = door->position.z;
    uint16_t cellIndexCurrentX = PosGetCellCoordX(obj_x); uint16_t cellIndexCurrentZ = PosGetCellCoordZ(obj_z);
    uint16_t cellCurrent = (cellIndexCurrentZ * WORLDX) + cellIndexCurrentX;
    uint16_t    cellIndexUp = PosGetCellCoordZ(obj_z + nudgeAmount), cellIndexDn = PosGetCellCoordZ(obj_z - nudgeAmount);
    uint16_t cellIndexRight = PosGetCellCoordX(obj_x + nudgeAmount), cellIndexLeft = PosGetCellCoordX(obj_x - nudgeAmount);
    uint16_t cellN_idx = PosGetCellCoords(obj_x, obj_z + nudgeAmount), cellS_idx = PosGetCellCoords(obj_x, obj_z - nudgeAmount);
    uint16_t cellE_idx = PosGetCellCoords(obj_x + nudgeAmount, obj_z), cellW_idx = PosGetCellCoords(obj_x - nudgeAmount, obj_z);
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

ENGINE_TO_MOD bool ToggleDoorPortal(uint8_t portalIdx, uint16_t doorIdx, uint16_t closedModelIndex) {
    if (portalIdx >= MAX_PORTALS) return false;

    Portal* prt = &activePortals[portalIdx];
    bool currentState = prt->open; uint16_t mdx = Sys_Global.instances[doorIdx].modelIndex;
         if (mdx == closedModelIndex &&  currentState) { prt->open = false; prt->dirty = true; }
    else if (mdx != closedModelIndex && !currentState) { prt->open =  true; prt->dirty = true; }
    return true;
}

bool UpdatedPlayerCell(void) {
    uint16_t lastCell = playerCellIdx;
    playerCellIdx = PosGetCellCoords(Sys_Global.instances[PLAYER1].position.x, Sys_Global.instances[PLAYER1].position.z);
    return (playerCellIdx != lastCell);
}

int32_t CastRayCellCheck(int32_t x, int32_t z, int32_t lastX, int32_t lastZ) {
    if (lastX != x || lastZ != z) {
        if (XZPairInBounds(lastX, lastZ)) {
            int32_t li = (lastZ * WORLDX) + lastX;
            uint32_t cell = gridCellStates[li];
            int32_t dx = x - lastX, dz = z - lastZ; // -1, 0, or 1 each
            if (dz == 0) { // Pure horizontal
                if ((dx >  0) && (cell & CELL_CLOSEDEAST))  return -1;
                if ((dx < 0)  && (cell & CELL_CLOSEDWEST))  return -1;
            } else if (dx == 0) { // Pure vertical
                if ((dz > 0)  && (cell & CELL_CLOSEDNORTH)) return -1;
                if ((dz < 0)  && (cell & CELL_CLOSEDSOUTH)) return -1;
            } else { // Diagonal — check cell + two axis-adjacent neighbors
                int32_t ni_ns = (int32_t)vclamp((lastZ + dz) * WORLDX + lastX,        0, ARRSIZE - 1); // neighbor indices, clamped
                int32_t ni_ew = (int32_t)vclamp( lastZ       * WORLDX + lastX + dx,   0, ARRSIZE - 1);
                uint32_t cf_ew  = (dx > 0) ? CELL_CLOSEDEAST  : CELL_CLOSEDWEST; // Which closed-edge flags to test depends on direction quadrant
                uint32_t cf_ns  = (dz > 0) ? CELL_CLOSEDNORTH : CELL_CLOSEDSOUTH;
                uint32_t nf_ew  = (dx > 0) ? CELL_CLOSEDEAST  : CELL_CLOSEDWEST;   // Neighbor sees the opposite face, same face on neighbor in NS direction
                uint32_t nf_ns  = (dz > 0) ? CELL_CLOSEDNORTH : CELL_CLOSEDSOUTH;  // same face on neighbor in EW direction
                uint32_t nsN = gridCellStates[ni_ns];
                uint32_t ewN = gridCellStates[ni_ew];
                bool c_ew  = (cell & cf_ew)  != 0; bool c_ns  = (cell & cf_ns)  != 0;
                bool n_ew  = (nsN  & nf_ew)  != 0 && (nsN & CELL_OPEN);  // neighbor along NS-axis, check EW edge
                bool n_ns  = (ewN  & nf_ns)  != 0 && (ewN & CELL_OPEN);  // neighbor along EW-axis, check NS edge
                if ((c_ns && c_ew) || (c_ew && n_ew) || (c_ns && n_ns) || (n_ew && n_ns)) return -1;
            }
        }
    }
 
    if (!XZPairInBounds(x,z)) return 0;
    int32_t ci = (z * WORLDX) + x;
    if (gridCellStates[ci] & CELL_OPEN) gridCellStates[ci] |=  CELL_VISIBLE;
    else                                gridCellStates[ci] &= ~CELL_VISIBLE;
    return (gridCellStates[ci] & CELL_VISIBLE) ? 1 : -1;
}

int32_t CastStraightZ(int32_t px, int32_t pz, int32_t signz) {
    if (signz > 0 && pz >= (WORLDZ - 1)) return pz; // Nowwhere to step to if right by edge, hence WORLDX - 1 here.
    if (signz < 0 && pz <= 0) return pz;
    if (!XZPairInBounds(px,pz)) return pz;
    
    int32_t cellIdx = (pz * WORLDX) + px;
    if (!(gridCellStates[cellIdx] & CELL_VISIBLE)) return pz;
    
    bool currentVisible = true; int32_t x=px, z=pz+signz, zabs=vabs(z);
    for (;zabs<WORLDX;z+=signz) { // Up/Down
        currentVisible = false;
        int32_t cellIdx_x_zmnus1 = ((z - 1) * WORLDX) + x, cellIdx_x_zplus1 = ((z + 1) * WORLDX) + x;
        if (XZPairInBounds(x,z - signz) && XZPairInBounds(x,z)) {
            int32_t cellIdx_x_zmnus_sign = ((z - signz) * WORLDX) + x;
            if (gridCellStates[cellIdx_x_zmnus_sign] & CELL_VISIBLE) {
                if (signz > 0) {
                    if (gridCellStates[cellIdx_x_zmnus1] & CELL_CLOSEDNORTH && gridCellStates[cellIdx_x_zmnus1] & CELL_OPEN) return z;
                } else if (signz < 0) {
                    if (gridCellStates[cellIdx_x_zplus1] & CELL_CLOSEDSOUTH && gridCellStates[cellIdx_x_zplus1] & CELL_OPEN) return z;
                }

                int32_t subCellIdx = (z * WORLDX) + x;
                if (gridCellStates[subCellIdx] & CELL_OPEN) gridCellStates[subCellIdx] |= CELL_VISIBLE;
                else gridCellStates[subCellIdx] &= ~CELL_VISIBLE;
                
                currentVisible = true; // Would be if twas open.
            }
        }

        if (!currentVisible) break; // Hit wall!

        if (XZPairInBounds(x + 1,z)) {
            int32_t cellIdx_xplus1_z = (z * WORLDX) + x + 1;
            if (CastRayCellCheck(x,z,x + 1,z) > 0) {
                if (gridCellStates[cellIdx_xplus1_z] & CELL_OPEN) gridCellStates[cellIdx_xplus1_z] |= CELL_VISIBLE;
                else gridCellStates[cellIdx_xplus1_z] &= ~CELL_VISIBLE;
            } else gridCellStates[cellIdx_xplus1_z] &= ~CELL_VISIBLE;
        }
        
        if (XZPairInBounds(x - 1,z)) {
            int32_t cellIdx_xmnus1_z = (z * WORLDX) + x - 1;
            if (CastRayCellCheck(x,z,x - 1,z) > 0) {
                if (gridCellStates[cellIdx_xmnus1_z] & CELL_OPEN) gridCellStates[cellIdx_xmnus1_z] |= CELL_VISIBLE;
                else gridCellStates[cellIdx_xmnus1_z] &= ~CELL_VISIBLE;
            } else gridCellStates[cellIdx_xmnus1_z] &= ~CELL_VISIBLE;
        }
    }
    
    return WORLDX * signz;
}

int32_t CastStraightX(int32_t px, int32_t pz, int32_t signx) {
    if (signx > 0 && px >= (WORLDX - 1)) return px; // Nowwhere to step to if right by edge, hence WORLDX - 1 here.
    if (signx < 0 && px <= 0) return px;
    if (!XZPairInBounds(px,pz)) return px;
    if (!(gridCellStates[(pz * WORLDX) + px] & CELL_VISIBLE)) return px;

    int32_t x = px + signx;
    int32_t z = pz;
    bool currentVisible = true;
    int32_t xabs = vabs(x);
    for (;xabs<WORLDX;x+=signx) { // Right/Left
        currentVisible = false;
        if (XZPairInBounds(x - signx,z) && XZPairInBounds(x,z)) {
            int32_t cellIdx_xmnussign_z = (z * WORLDX) + x - signx;
            if (gridCellStates[cellIdx_xmnussign_z] & CELL_VISIBLE) {
                if ((gridCellStates[(z * WORLDX) + x + signx] & CELL_CLOSEDEAST) && (gridCellStates[(z * WORLDX) + x + signx] & CELL_OPEN)) return x;
                
                int32_t subCellIdx = (z * WORLDX) + x;
                if (gridCellStates[subCellIdx] & CELL_OPEN) gridCellStates[subCellIdx] |= CELL_VISIBLE;
                else gridCellStates[subCellIdx] &= ~CELL_VISIBLE;
                
                currentVisible = true; // Would be if twas open.
            }
        }

        if (!currentVisible) break; // Hit wall!
        
        if (XZPairInBounds(x,z + 1)) {
            int32_t cellIdx_x_zplus1 = ((z + 1) * WORLDX) + x;
            if (CastRayCellCheck(x,z,x,z + 1) > 0) {
                if (gridCellStates[cellIdx_x_zplus1] & CELL_OPEN) gridCellStates[cellIdx_x_zplus1] |= CELL_VISIBLE;
                else gridCellStates[cellIdx_x_zplus1] &= ~CELL_VISIBLE;
            } else {
                gridCellStates[cellIdx_x_zplus1] &= ~CELL_VISIBLE;
            }
        }
        
        if (XZPairInBounds(x,z - 1)) {
            int32_t cellIdx_x_zmnus1 = ((z - 1) * WORLDX) + x;
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

void CastRay(int32_t x0, int32_t z0, int32_t x1, int32_t z1) {
    int32_t dx = vabs(x1 - x0);      int32_t dz = vabs(z1 - z0);
    int32_t sx = (x0 < x1) ? 1 : -1; int32_t sz = (z0 < z1) ? 1 : -1;
    int32_t x = x0;                  int32_t z = z0;
    int32_t lastX = x;               int32_t lastZ = z;
    int32_t err = dx - dz;
    int32_t iter = dx > dz ? dx : dz;
    while (iter >= 0) {
        if (!XZPairInBounds(x,z) || !XZPairInBounds(lastX,lastZ)) continue;
        if (CastRayCellCheck(x,z,lastX,lastZ) == -1) return;

        lastX = x; lastZ = z;
        int32_t e2 = 2 * err;
        if (e2 > -dz) { err -= dz; x += sx; }
        if (e2 <  dx) { err += dx; z += sz; }
        iter--;
    }
}

void CircleFanRays(int32_t x0, int32_t z0) { // CastRay()'s in fan from x0,z0 out to every cell around map perimeter.
    if (!XZPairInBounds(x0,z0)) return;
    if (!(gridCellStates[(z0 * WORLDX) + x0] & CELL_VISIBLE)) return;

    int32_t x,z,max=WORLDX,min=0; // Reduce work slightly by not casting towards edges but 1 less = [1,63].
    for (x=min;x<max;x++) CastRay(x0,z0,x,min);
    for (x=min;x<max;x++) CastRay(x0,z0,x,max);
    for (z=min;z<max;z++) CastRay(x0,z0,min,z);
    for (z=min;z<max;z++) CastRay(x0,z0,max,z);
}

void DetermineVisibleCells(int32_t startX, int32_t startZ) {
    if (!XZPairInBounds(startX,startZ)) return;

    for (int32_t x=0;x<WORLDX;x++) {
        for (int32_t z=0;z<WORLDZ;z++) {
            int32_t subCellIdx = (z * WORLDX) + x;
            gridCellStates[subCellIdx] &= ~CELL_VISIBLE; // Clear all to not visible.
        }
    }

    int32_t cellIdx_start = (startZ * WORLDX) + startX;
    gridCellStates[cellIdx_start] |= CELL_VISIBLE; // Force starting player cell to visible.
    // Cast to the right (East)        [ ][3]
    CastStraightX(startX,startZ,1); // [1][2]
    int32_t iter = 0;               // [ ][3]
    for (int32_t march=startX;march<(WORLDX - 1);march++) {
        iter++;
        if (iter > WORLDX) break;
        
        if (XZPairInBounds(march,startZ + 1)) {
            if (gridCellStates[((startZ + 1) * WORLDX) + march] & CELL_VISIBLE) march = CastStraightX(march,startZ + 1,1);  // Above [1]
        }
    }
    
    iter = 0;
    for (int32_t march=startX;march<(WORLDX - 1);march++) {
        iter++;
        if (iter > WORLDX) break;

        if (XZPairInBounds(march,startZ - 1)) {
            if (gridCellStates[((startZ - 1) * WORLDX) + march] & CELL_VISIBLE) march = CastStraightX(march,startZ - 1,1);  // Below [1]
        }
    }
    // Cast to the left (West)          [3][ ]
    CastStraightX(startX,startZ,-1); // [2][1]
    iter = 0;                        // [3][ ]
    for (int32_t march=startX;march>=1;march--) {
        iter++;
        if (iter > WORLDX) break;
        
        if (XZPairInBounds(march,startZ + 1)) {
            if (gridCellStates[((startZ + 1) * WORLDX) + march] & CELL_VISIBLE) march = CastStraightX(march,startZ + 1,-1); // Above [1]
        }
    }

    iter = 0;
    for (int32_t march=startX;march>=1;march--) {
        iter++;
        if (iter > WORLDX) break;

        if (XZPairInBounds(march,startZ - 1)) {
            if (gridCellStates[((startZ - 1) * WORLDX) + march] & CELL_VISIBLE) march = CastStraightX(march,startZ - 1,-1); // Below [1]
        }
    }
    // Cast down (South)                [ ][1][ ]
    CastStraightZ(startX,startZ,-1); // [3][2][3]
    iter = 0;
    for (int32_t march=startZ;march>=1;march--) {
        iter++;
        if (iter > WORLDX) break;

        if (XZPairInBounds(startX + 1,march)) {
            if (gridCellStates[(march * WORLDX) + startX + 1] & CELL_VISIBLE) march = CastStraightZ(startX + 1,march,-1);
        }
    }
    
    iter = 0;
    for (int32_t march=startZ;march>=1;march--) {
        iter++;
        if (iter > WORLDX) break;

        if (XZPairInBounds(startX - 1,march)) {
            if (gridCellStates[(march * WORLDX) + startX - 1] & CELL_VISIBLE) march = CastStraightZ(startX - 1,march,-1);
        }
    }
    // Cast up (North)                 [3][2][3]
    CastStraightZ(startX,startZ,1); // [ ][1][ ]
    iter = 0;
    for (int32_t march=startZ;march<(WORLDX - 1);march++) {
        iter++;
        if (iter > WORLDX) break;

        if (XZPairInBounds(startX + 1,march)) {
            if (gridCellStates[(march * WORLDX) + startX + 1] & CELL_VISIBLE) march = CastStraightZ(startX + 1,march,1);
        }
    }
    
    iter = 0;
    for (int32_t march=startZ;march<(WORLDX - 1);march++) {
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
    for (int32_t x=0;x<WORLDX;++x) {
        for (int32_t z=0;z<WORLDZ;++z) {
            int32_t cellIdx_xz = (z * WORLDX) + x;
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
    
    DebugRAM("start of Cull_Init"); // For each cell, get the visibility as though player were there and put into gridCellStates.  Then store the visibility of gridCellStates into the table of all visible cells for that cell
    DetermineClosedEdges();         // at the appropriate offset for looking up later when actually re-assigning gridCellStates from this precalculated visibility state for the particular cell.
    for (int32_t z=0;z<WORLDZ;z++) {
        for (int32_t x=0;x<WORLDX;x++) {
            DetermineVisibleCells(x,z);
            int32_t cellIdx = (z * WORLDX) + x;
            for (int32_t z2=0;z2<WORLDZ;z2++) {
                for (int32_t x2=0;x2<WORLDX;x2++) {
                    int32_t subCellIdx = (z2 * WORLDX) + x2;
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
    
    (void)UpdatedPlayerCell();
    int32_t cellToCellIdx = playerCellIdx * ARRSIZE;
    for (int32_t z=0;z<WORLDZ;++z) {
        for (int32_t x=0;x<WORLDX;++x) {
            int32_t cellIdx = (z * WORLDX) + x;
            size_t flat_idx = (size_t)(cellToCellIdx + cellIdx);
            if (get_cull_bit(precomputedVisibleCellsFromHere,flat_idx)) gridCellStates[cellIdx] |= CELL_VISIBLE; // Get visible before putting meshes into their cells so we can nudge them a little.
        }
    }

    gridCellStates[0] |= CELL_VISIBLE; // Errors default here so draw them anyways.
    CullCore(); // Do first Cull pass, forcing as player moved to new cell.
    DualLog(" took %f secs\n",get_time() - start_time);
    DebugRAM("end of Cull_Init");
}

extern Light lights[LIGHT_COUNT];
ENGINE_TO_MOD void PortalCulling(void) { // Called just once at end of animation loop for the frame after each frame perfect change to door models becoming either closed or not closed.
    uint16_t playerCellX = PosGetCellCoordX(Sys_Global.instances[PLAYER1].position.x);
    uint16_t playerCellZ = PosGetCellCoordZ(Sys_Global.instances[PLAYER1].position.z);
    bool previousLightVisible[LIGHT_COUNT];
    __builtin_memset(previousLightVisible,false,LIGHT_COUNT * sizeof(bool));
    for (uint16_t i=0;i<Sys_Global.loadedLights;++i) {
        uint16_t lcell = (lights[i].pos.z * WORLDX) + lights[i].pos.x;
        if (gridCellStates[lcell] & CELL_VISIBLE) previousLightVisible[i] = true;
    }
    
    PortalCell cellA, cellB;
    for (uint8_t portalIdx=0;portalIdx<MAX_PORTALS;++portalIdx) {
        Portal* prt = &activePortals[portalIdx];
        if (!prt->dirty) continue;
        
        prt->dirty = false;
        cellA = prt->cellA; cellB = prt->cellB; // Guaranteed order at level load.  A = N or E, B = S or W
        bool isNS = prt->portalNS;
        uint16_t cellIdxA = (cellA.z * WORLDX) + cellA.x;
        uint16_t cellIdxB = (cellB.z * WORLDX) + cellB.x;
        if (prt->open) { // Open the edges up
            if (isNS) {
                gridCellStates[cellIdxA] &= ~(CELL_CLOSEDSOUTH); gridCellStates[cellIdxB] &= ~(CELL_CLOSEDNORTH);
//                 if (entIdx == 499 || entIdx == 509) // Bulkhead or giant elevator door spans 2 cells perpendicular to its isNS direction. TODO, need neighboring Portal concept
            } else {
                gridCellStates[cellIdxA] &= ~(CELL_CLOSEDWEST); gridCellStates[cellIdxB] &= ~(CELL_CLOSEDEAST);
            }
        } else {
            if (isNS) {
                gridCellStates[cellIdxA] |= CELL_CLOSEDSOUTH; gridCellStates[cellIdxB] |= CELL_CLOSEDNORTH;
            } else {
                gridCellStates[cellIdxA] |= CELL_CLOSEDWEST; gridCellStates[cellIdxB] |= CELL_CLOSEDEAST;
            }
        }
    }
    
    DetermineVisibleCells(playerCellX,playerCellZ); // Recompute full PVS with new closed edges for all portal states.  So much for the precomputed set.
    for (uint16_t i=0;i<Sys_Global.loadedLights;++i) {
        uint16_t lcell = (lights[i].pos.z * WORLDX) + lights[i].pos.x;
        if (!previousLightVisible[i] && (gridCellStates[lcell] & CELL_VISIBLE)) flag_setu32(&lights[i].lflags,LDIRTY,true);
    }
    glNamedBufferData(Sys_Render.cellVisibleDataID,ARRSIZE * sizeof(uint32_t),gridCellStates,GL_DYNAMIC_DRAW);
}

static inline __attribute__((always_inline)) void CellCoordsToPos(uint16_t x, uint16_t z, float* pos_x, float* pos_z) { *pos_x = Sys_Global.worldMin_x + (x * CELL_SIZE); *pos_z = Sys_Global.worldMin_z + (z * CELL_SIZE); }
bool CullCore(void) {
    if (Sys_Global.currentLevel >= LEVEL_CYBERSPACE) return false;

    float pos_x,pos_z;
    uint16_t cellX = (uint16_t)clamp((int32_t)vfloor((Sys_Global.instances[PLAYER1].position.x - Sys_Global.worldMin_x + CELLXHALF) / CELL_SIZE),0,WORLDX_0BASED);
    uint16_t cellZ = (uint16_t)clamp((int32_t)vfloor((Sys_Global.instances[PLAYER1].position.z - Sys_Global.worldMin_z + CELLXHALF) / CELL_SIZE),0,WORLDX_0BASED);
    CellCoordsToPos(cellX,cellZ,&pos_x,&pos_z);
    for (int i=0;i<Sys_Global.loadedInstances;++i) {
        float distSqrd = squareDistance2D(Sys_Global.instances[i].position.x,Sys_Global.instances[i].position.z,pos_x,pos_z);
        instanceIsLODArray[i] = (distSqrd >= 655.36f); // 25.6f * 25.6f
    }
    
    PortalCulling(); // Update based on portal states.
    return true;
} // 602
