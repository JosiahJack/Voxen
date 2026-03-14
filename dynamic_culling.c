#include "os.h"
#include "gl.h"
#include "voxen.h"
uint8_t *stbi_load_from_memory(const uint8_t* buffer, int32_t len, int32_t* x, int32_t* y);
extern void stbi__arena_init(void);
extern uint8_t*  stbi__arena_base;
#define STBI_ARENA_SIZE 16 * 1024 * 1024
uint32_t gridCellStates[ARRSIZE];
float gridCellFloorHeight[ARRSIZE];
float gridCellCeilingHeight[ARRSIZE];
uint32_t precomputedVisibleCellsFromHere[PRECOMPUTED_VISIBILITY_SIZE];
uint16_t playerCellIdx = 0u;
uint16_t numCellsVisible = 0u;
float worldMin_x, worldMin_z;
bool instanceIsLODArray[INSTANCE_COUNT];
#define MAX_CULL_FILESIZE 500000
uint8_t cullingFileBuffer[MAX_CULL_FILESIZE];
Portal activePortals[MAX_PORTALS];
uint8_t numActivePortals = 0;
uint16_t uncullingCameras[5];

__attribute__((pure)) bool get_cull_bit(const uint32_t* arr, int idx) {
    int word = idx / 32;
    int bit = idx % 32;
    return ((arr[word] & (1U << bit)) != 0);
}

static inline __attribute__((always_inline)) void set_cull_bit(uint32_t* arr, int idx, bool val) {
    int word = idx / 32;
    int bit = idx % 32;
    if (val) arr[word] |= (1U << bit);
    else arr[word] &= ~(1U << bit);
}

static unsigned char* LoadCullPNG(const char* name, int level) {
    char path[256]; StringFormat(path, sizeof(path),"./Data/%s_%d.png",name,level);
    OsFileHandle fp = OS_OpenReadonly(path);
    OS_Seek(fp, 0, SEEK_END); size_t size = OS_Tell(fp);
    if (size > MAX_CULL_FILESIZE) { DualLogError("PNG too large: %s\n", path); OS_Exit(1); }
    OS_Seek(fp, 0, SEEK_SET);
    size_t read_size = OS_Read(fp,cullingFileBuffer,size);
    OS_Close(fp);
    if (read_size != size) { DualLogError("Failed to read %s\n", path); OS_Exit(1); }
    int w, h;
    unsigned char* pixels = stbi_load_from_memory(cullingFileBuffer, size, &w, &h);
    if (!pixels) { DualLogError("STB failed: %s\n", path); OS_Exit(1); }
    return pixels;
}

#define PIXEL_IDX(x, z) ((x) + ((WORLDZ - 1 - (z)) * WORLDX)) * 4 // 4 channels, flip z to have desired bottom-left origin 0,0 vs stbi_load's top-left
void DetermineClosedEdges(void) {
    stbi__arena_init();
    unsigned char* openPixels  = LoadCullPNG("worldcellopen", Sys_Global.currentLevel);
    unsigned char openData_r, openData_g, openData_b;
    uint16_t totalOpenCells = 0;
    for (int32_t x=0;x<WORLDX;++x) {
        for (int32_t z=0;z<WORLDZ;++z) {
            int32_t cellIdx = (z * WORLDX) + x;
            gridCellStates[cellIdx] &= ~CELL_OPEN;
            int32_t pixelIdx = PIXEL_IDX(x,z);
            openData_r = openPixels[pixelIdx + 0];
            openData_g = openPixels[pixelIdx + 1];
            openData_b = openPixels[pixelIdx + 2];
            if (openData_r > 0 || openData_g > 0 || openData_b > 0) {
                gridCellStates[cellIdx] |= CELL_OPEN;
                totalOpenCells++;
            } else {
                gridCellStates[cellIdx] |= CELL_CLOSEDNORTH | CELL_CLOSEDEAST | CELL_CLOSEDSOUTH | CELL_CLOSEDWEST; // Also force close the edges for closed cells even if above edges image said tweren't closed edges.
            }
        }
    }

    gridCellStates[0] |= CELL_OPEN; // Force the fallback error cell to be open (forced visible later, open is static, visible is transient)
    unsigned char* edgePixels = LoadCullPNG("worldedgesclosed", Sys_Global.currentLevel);
    unsigned char closedData_r, closedData_g, closedData_b, closedData_a;
    uint16_t closedCountNorth = 0, closedCountSouth = 0, closedCountEast = 0, closedCountWest = 0;
    for (int32_t x=0;x<WORLDX;x++) {
        for (int32_t z=0;z<WORLDZ;z++) {
            int32_t cellIdx = (z * WORLDX) + x;
            gridCellStates[cellIdx] &= ~(CELL_CLOSEDNORTH | CELL_CLOSEDEAST | CELL_CLOSEDSOUTH | CELL_CLOSEDWEST); // Mark all edges not closed
            int32_t pixelIdx = PIXEL_IDX(x,z);
            closedData_r = edgePixels[pixelIdx + 0];
            closedData_g = edgePixels[pixelIdx + 1];
            closedData_b = edgePixels[pixelIdx + 2];
            closedData_a = edgePixels[pixelIdx + 3];
            if (closedData_r > 127) { gridCellStates[cellIdx] |= CELL_CLOSEDNORTH; closedCountNorth += gridCellStates[cellIdx] & CELL_OPEN ? 1 : 0; }
            if (closedData_g > 127) { gridCellStates[cellIdx] |= CELL_CLOSEDEAST; closedCountEast += gridCellStates[cellIdx] & CELL_OPEN ? 1 : 0; }
            if (closedData_b > 127) { gridCellStates[cellIdx] |= CELL_CLOSEDSOUTH; closedCountSouth += gridCellStates[cellIdx] & CELL_OPEN ? 1 : 0; }
            if ((closedData_r < 255 && closedData_r > 0) || (closedData_g < 255 && closedData_g > 0) || (closedData_b < 255 && closedData_b > 0)) {
                // Anything that has closed west edge will be not at full 255 on at least one channel.
                // Typical for all other edge conditions is to use full brightness 255 on the channel(s).
                // All 4 closed would be 128 128 128 but this doesn't ever happen. None closed is 0 0 0
                gridCellStates[cellIdx] |= CELL_CLOSEDWEST; closedCountWest += gridCellStates[cellIdx] & CELL_OPEN ? 1 : 0;
            }
            
            if (closedData_a > 0 && closedData_a < 255) gridCellStates[cellIdx] |= CELL_CLOSEDNORTH | CELL_CLOSEDEAST | CELL_CLOSEDSOUTH | CELL_CLOSEDWEST;
        }
    }
        
    unsigned char* skyPixels = LoadCullPNG("worldcellskyvis", Sys_Global.currentLevel);
    unsigned char skyData_r, skyData_g, skyData_b;
    for (int32_t x=0;x<WORLDX;++x) {
        for (int32_t z=0;z<WORLDZ;++z) {
            int32_t cellIdx = (z * WORLDX) + x;
            int32_t pixelIdx = PIXEL_IDX(x,z);
            skyData_r = skyPixels[pixelIdx + 0];
            skyData_g = skyPixels[pixelIdx + 1];
            skyData_b = skyPixels[pixelIdx + 2];
            if (skyData_r > 127 && skyData_g < 127 && skyData_b < 127) gridCellStates[cellIdx] &= ~(CELL_SEES_SUN | CELL_SEES_SKYBOX); // All red cells marked as -1, no sky or sun.
            else if (skyData_r <= 127 && skyData_g <= 127 && skyData_b > 127) gridCellStates[cellIdx] |= CELL_SEES_SUN | CELL_SEES_SKYBOX; // All blue cells marked as sky visible.  Sun + Sky.
            else { gridCellStates[cellIdx] &= ~CELL_SEES_SKYBOX; gridCellStates[cellIdx] |= CELL_SEES_SUN; } // All white and black cells marked as 0.  Only sees Sun.
        }
    }
    
    OS_DeallocateRAM(stbi__arena_base, STBI_ARENA_SIZE); stbi__arena_base = NULL;
    DualLog("found %d open cells, closed edges N: %d, S: %d, E: %d, W: %d...",totalOpenCells,closedCountNorth,closedCountSouth,closedCountEast,closedCountWest);
    DebugRAM("end of dynamic culling DetermineClosedEdges");
}

bool UpdatedPlayerCell(void) {
    uint16_t lastCell = playerCellIdx;
    playerCellIdx = PosGetCellCoords(Sys_Global.instances[PLAYER1].position.x, Sys_Global.instances[PLAYER1].position.z);
    return (playerCellIdx != lastCell);
}

int32_t CastRayCellCheck(int32_t x, int32_t z, int32_t lastX, int32_t lastZ) {
    if (!(lastX == x && lastZ == z)) {
        if (XZPairInBounds(lastX,lastZ)) {
            int32_t cellIdx_last = (lastZ * WORLDX) + lastX;
            uint32_t cell = gridCellStates[cellIdx_last];
            if (lastZ == z) {
                if (lastX > x) { // [  x  ][lastX]
                    if (cell & CELL_CLOSEDWEST) return -1;
                } else { // Less than x since == x was already checked.
                    if (cell & CELL_CLOSEDEAST) return -1;
                }
            }

            if (lastX == x) {
                if (lastZ > z) { // [lastZ]
                                 // [  y  ]
                    if (cell & CELL_CLOSEDSOUTH) return -1;
                } else { // Less than y since == y was already checked.
                    if (cell & CELL_CLOSEDNORTH) return -1;
                }
            }

            // Diagonals
            if (lastZ != z && lastX != x) {
                int32_t cellIdx_neighborNorth = ((lastZ + 1) * WORLDX) + lastX;
                cellIdx_neighborNorth = cellIdx_neighborNorth > ARRSIZE ? ARRSIZE : cellIdx_neighborNorth;
                int32_t cellIdx_neighborSouth = ((lastZ - 1) * WORLDX) + lastX;
                cellIdx_neighborSouth = cellIdx_neighborSouth > ARRSIZE ? ARRSIZE : cellIdx_neighborSouth;
                int32_t cellIdx_neighborEast = (lastZ * WORLDX) + lastX + 1;
                cellIdx_neighborEast = cellIdx_neighborEast > ARRSIZE ? ARRSIZE : cellIdx_neighborEast;
                int32_t cellIdx_neighborWest = (lastZ * WORLDX) + lastX - 1;
                cellIdx_neighborWest = cellIdx_neighborWest > ARRSIZE ? ARRSIZE : cellIdx_neighborWest;
                uint32_t northNeighbor = gridCellStates[cellIdx_neighborNorth];
                uint32_t southNeighbor = gridCellStates[cellIdx_neighborSouth];
                uint32_t eastNeighbor = gridCellStates[cellIdx_neighborEast];
                uint32_t westNeighbor = gridCellStates[cellIdx_neighborWest];
                if (lastZ > z && lastX > x) { // [Nb][ 1]
                                              // [ 2][Na]
                    if ((cell & CELL_CLOSEDSOUTH) && (cell & CELL_CLOSEDWEST)) return -1;// Check cell 1 only
                    
                    bool neighborClosedWest = false;
                    bool neighborClosedSouth = false;
                    if (XZPairInBounds(lastX,lastZ - 1)) neighborClosedWest = (southNeighbor & CELL_CLOSEDWEST) && (southNeighbor & CELL_OPEN);
                    if (XZPairInBounds(lastX - 1,lastZ)) neighborClosedSouth = (westNeighbor & CELL_CLOSEDSOUTH) && (westNeighbor & CELL_OPEN);
                    if ((cell & CELL_CLOSEDWEST) && neighborClosedWest) return -1; // Check cell 1 and Neighbor a (Na)
                    if ((cell & CELL_CLOSEDSOUTH) && neighborClosedSouth) return -1; // Check cell 1 and Neighbor b (Nb)
                    if (neighborClosedWest && neighborClosedSouth) return -1; // Check Neighbor a (Na) and Neighbor b (Nb)
                } else if (lastZ < z && lastX < x) { // [ ][2]
                                                     // [1][ ]return
                    if ((cell & CELL_CLOSEDNORTH) && (cell & CELL_CLOSEDEAST)) return -1;
                    
                    bool neighborClosedEast = false;
                    bool neighborClosedNorth = false;
                    if (XZPairInBounds(lastX,lastZ + 1)) neighborClosedEast = (northNeighbor & CELL_CLOSEDEAST) && (northNeighbor & CELL_OPEN);
                    if (XZPairInBounds(lastX + 1,lastZ)) neighborClosedNorth = (eastNeighbor & CELL_CLOSEDNORTH) && (eastNeighbor & CELL_OPEN);
                    if ((cell & CELL_CLOSEDEAST) && neighborClosedEast) return -1;
                    if ((cell & CELL_CLOSEDNORTH) && neighborClosedNorth) return -1;
                    if (neighborClosedEast && neighborClosedNorth) return -1;
                } else if (lastZ > z && lastX < x) { // [1][ ]
                                                     // [ ][2]
                    if ((cell & CELL_CLOSEDSOUTH) && (cell & CELL_CLOSEDEAST)) return -1;
                    
                    bool neighborClosedEast = false;
                    bool neighborClosedSouth = false;
                    if (XZPairInBounds(lastX,lastZ - 1)) neighborClosedEast = (southNeighbor & CELL_CLOSEDEAST) && (southNeighbor & CELL_OPEN);
                    if (XZPairInBounds(lastX + 1,lastZ)) neighborClosedSouth = (eastNeighbor & CELL_CLOSEDSOUTH) && (eastNeighbor & CELL_OPEN);
                    if ((cell & CELL_CLOSEDEAST) && neighborClosedEast) return -1;
                    if ((cell & CELL_CLOSEDSOUTH) && neighborClosedSouth) return -1;
                    if (neighborClosedEast && neighborClosedSouth) return -1;
                } else if (lastZ < z && lastX > x) { // [2][ ]
                                                     // [ ][1]
                    if ((cell & CELL_CLOSEDNORTH) && (cell & CELL_CLOSEDWEST)) return -1;
                    
                    bool neighborClosedWest = false;
                    bool neighborClosedNorth = false;
                    if (XZPairInBounds(lastX,lastZ + 1)) neighborClosedWest = (northNeighbor & CELL_CLOSEDWEST) && (northNeighbor & CELL_OPEN);
                    if (XZPairInBounds(lastX - 1,lastZ)) neighborClosedNorth = (westNeighbor & CELL_CLOSEDNORTH) && (westNeighbor & CELL_OPEN);
                    if ((cell & CELL_CLOSEDWEST) && neighborClosedWest) return -1;
                    if ((cell & CELL_CLOSEDNORTH) && neighborClosedNorth) return -1;
                    if (neighborClosedWest && neighborClosedNorth) return -1;
                }
            }
        }
    }
    
    if (XZPairInBounds(x,z)) {
        int32_t cellIdx_xz = (z * WORLDX) + x;
        if (gridCellStates[cellIdx_xz] & CELL_OPEN) gridCellStates[cellIdx_xz] |= CELL_VISIBLE;
        else gridCellStates[cellIdx_xz] &= ~CELL_VISIBLE;
        
        if (!(gridCellStates[cellIdx_xz] & CELL_VISIBLE)) return -1;
        return 1;
    }

    return 0;
}

int32_t CastStraightZ(int32_t px, int32_t pz, int32_t signz) {
    if (signz > 0 && pz >= (WORLDZ - 1)) return pz; // Nowwhere to step to if right by edge, hence WORLDX - 1 here.
    if (signz < 0 && pz <= 0) return pz;
    if (!XZPairInBounds(px,pz)) return pz;
    
    int32_t cellIdx = (pz * WORLDX) + px;
    if (!(gridCellStates[cellIdx] & CELL_VISIBLE)) return pz;
    
    bool currentVisible = true;
    int32_t x = px;
    int32_t z = pz + signz;
    int32_t zabs = vabs(z);
    for (;zabs<WORLDX;z+=signz) { // Up/Down
        currentVisible = false;
        int32_t cellIdx_x_zmnus1 = ((z - 1) * WORLDX) + x;
        int32_t cellIdx_x_zplus1 = ((z + 1) * WORLDX) + x;
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
            } else {
                gridCellStates[cellIdx_xplus1_z] &= ~CELL_VISIBLE;
            }
        }
        
        if (XZPairInBounds(x - 1,z)) {
            int32_t cellIdx_xmnus1_z = (z * WORLDX) + x - 1;
            if (CastRayCellCheck(x,z,x - 1,z) > 0) {
                if (gridCellStates[cellIdx_xmnus1_z] & CELL_OPEN) gridCellStates[cellIdx_xmnus1_z] |= CELL_VISIBLE;
                else gridCellStates[cellIdx_xmnus1_z] &= ~CELL_VISIBLE;
            } else {
                gridCellStates[cellIdx_xmnus1_z] &= ~CELL_VISIBLE;
            }
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
        int32_t cellIdx_xmnus1_z = (z * WORLDX) + x - 1;
        int32_t cellIdx_xplus1_z = (z * WORLDX) + x + 1;
        if (XZPairInBounds(x - signx,z) && XZPairInBounds(x,z)) {
            int32_t cellIdx_xmnussign_z = (z * WORLDX) + x - signx;
            if (gridCellStates[cellIdx_xmnussign_z] & CELL_VISIBLE) {
                if (signx > 0) {
                    if ((gridCellStates[cellIdx_xmnus1_z] & CELL_CLOSEDEAST) && (gridCellStates[cellIdx_xmnus1_z] & CELL_OPEN)) return x;
                } else if (signx < 0) {
                    if ((gridCellStates[cellIdx_xplus1_z] & CELL_CLOSEDWEST) && (gridCellStates[cellIdx_xplus1_z] & CELL_OPEN)) return x;
                }
                
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
    int32_t dx = vabs(x1 - x0);       int32_t dz = vabs(z1 - z0);
    int32_t sx = (x0 < x1) ? 1 : -1; int32_t sz = (z0 < z1) ? 1 : -1;
    int32_t x = x0;                  int32_t z = z0;
    int32_t lastX = x;               int32_t lastZ = z;
    int32_t err = dx - dz;
    int32_t iter = dx > dz ? dx : dz;
    while (iter >= 0) {
        if (!XZPairInBounds(x,z) || !XZPairInBounds(lastX,lastZ)) continue;
        if (CastRayCellCheck(x,z,lastX,lastZ) == -1) return;

        lastX = x;
        lastZ = z;
        int32_t e2 = 2 * err;
        if (e2 > -dz) { err -= dz; x += sx; }
        if (e2 <  dx) { err += dx; z += sz; }
        iter--;
    }
}

void CircleFanRays(int32_t x0, int32_t z0) { // CastRay()'s in fan from x0,z0 out to every cell around map perimeter.
    if (!XZPairInBounds(x0,z0)) return;
    if (!(gridCellStates[(z0 * WORLDX) + x0] & CELL_VISIBLE)) return;

    int32_t x,z;     
    int32_t max = WORLDX; // Reduce work slightly by not casting towards 
    int32_t min = 0;      // edges but 1 less = [1,63].
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
                                    // [ ][3]
    int32_t iter = 0;
    for (int32_t march=startX;march<(WORLDX - 1);march++) {
        iter++;
        if (iter > WORLDX) break;
        
        if (XZPairInBounds(march,startZ + 1)) {
            if (gridCellStates[((startZ + 1) * WORLDX) + march] & CELL_VISIBLE) {
                march = CastStraightX(march,startZ + 1,1);  // Above [1]
            }
        }
    }
    
    iter = 0;
    for (int32_t march=startX;march<(WORLDX - 1);march++) {
        iter++;
        if (iter > WORLDX) break;

        if (XZPairInBounds(march,startZ - 1)) {
            if (gridCellStates[((startZ - 1) * WORLDX) + march] & CELL_VISIBLE) {
                march = CastStraightX(march,startZ - 1,1);  // Below [1]
            }
        }
    }
    
    // Cast to the left (West)          [3][ ]
    CastStraightX(startX,startZ,-1); // [2][1]
                                     // [3][ ]
    iter = 0;
    for (int32_t march=startX;march>=1;march--) {
        iter++;
        if (iter > WORLDX) break;
        
        if (XZPairInBounds(march,startZ + 1)) {
            if (gridCellStates[((startZ + 1) * WORLDX) + march] & CELL_VISIBLE) {
                march = CastStraightX(march,startZ + 1,-1); // Above [1]
            }
        }
    }

    iter = 0;
    for (int32_t march=startX;march>=1;march--) {
        iter++;
        if (iter > WORLDX) break;

        if (XZPairInBounds(march,startZ - 1)) {
            if (gridCellStates[((startZ - 1) * WORLDX) + march] & CELL_VISIBLE) {
                march = CastStraightX(march,startZ - 1,-1); // Below [1]
            }
        }
    }

    // Cast down (South)                [ ][1][ ]
    CastStraightZ(startX,startZ,-1); // [3][2][3]
    iter = 0;
    for (int32_t march=startZ;march>=1;march--) {
        iter++;
        if (iter > WORLDX) break;

        if (XZPairInBounds(startX + 1,march)) {
            if (gridCellStates[(march * WORLDX) + startX + 1] & CELL_VISIBLE) {
                march = CastStraightZ(startX + 1,march,-1);
            }
        }
    }
    
    iter = 0;
    for (int32_t march=startZ;march>=1;march--) {
        iter++;
        if (iter > WORLDX) break;

        if (XZPairInBounds(startX - 1,march)) {
            if (gridCellStates[(march * WORLDX) + startX - 1] & CELL_VISIBLE) {
                march = CastStraightZ(startX - 1,march,-1);
            }
        }
    }

    // Cast up (North)                 [3][2][3]
    CastStraightZ(startX,startZ,1); // [ ][1][ ]
    iter = 0;
    for (int32_t march=startZ;march<(WORLDX - 1);march++) {
        iter++;
        if (iter > WORLDX) break;

        if (XZPairInBounds(startX + 1,march)) {
            if (gridCellStates[(march * WORLDX) + startX + 1] & CELL_VISIBLE) {
                march = CastStraightZ(startX + 1,march,1);
            }
        }
    }
    
    iter = 0;
    for (int32_t march=startZ;march<(WORLDX - 1);march++) {
        iter++;
        if (iter > WORLDX) break;

        if (XZPairInBounds(startX - 1,march)) {
            if (gridCellStates[(march * WORLDX) + startX - 1] & CELL_VISIBLE) {
                march = CastStraightZ(startX - 1,march,1);
            }
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
    
//     int32_t numVisible = 0;
//     for (int32_t x=0;x<WORLDX;++x) {
//         for (int32_t z=0;z<WORLDZ;++z) {
//             if (gridCellStates[(z * WORLDX) + x] & CELL_VISIBLE) numVisible++;
//         }
//     }
}

void CullInit(void) {
    double start_time = get_time();    
    DualLog("Culling...");
    if (Sys_Global.currentLevel == LEVEL_CYBERSPACE) return;
    
    DebugRAM("start of Cull_Init");    
    DetermineClosedEdges();
  
    // For each cell, get the visibility as though player were there and put into gridCellStates
    // Then store the visibility of gridCellStates into the table of all visible cells for that cell
    // at the appropriate offset for looking up later when actually re-assigning gridCellStates
    // from this precalculated visibility state for the particular cell.
//     int32_t numPrecomputedVisibleCells = 0;
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
//                     if (is_visible) numPrecomputedVisibleCells++;
                }
            }
            
            if (Sys_Global.currentLevel == 10) {
                if ((x == 15 || x == 16) && z == 23) { // Fix up problem cells at odd angle where ddx doesn't work.
                    size_t flat_idx = (size_t)(cellIdx * ARRSIZE) + ((11 * WORLDX) + 12);
                    set_cull_bit(precomputedVisibleCellsFromHere,flat_idx,true);
//                     numPrecomputedVisibleCells++;
                }
            }
        }
    }
    
    (void)UpdatedPlayerCell();
    int32_t cellToCellIdx = playerCellIdx * ARRSIZE;
//     int32_t numFoundVisibleCellsForPlayerStart = 0;
    for (int32_t z=0;z<WORLDZ;++z) {
        for (int32_t x=0;x<WORLDX;++x) {
            int32_t cellIdx = (z * WORLDX) + x;
            size_t flat_idx = (size_t)(cellToCellIdx + cellIdx);
            if (get_cull_bit(precomputedVisibleCellsFromHere,flat_idx)) {
//                 numFoundVisibleCellsForPlayerStart++;
                gridCellStates[cellIdx] |= CELL_VISIBLE; // Get visible before putting meshes into their cells so we can nudge them a little.
            }
        }
    }

    gridCellStates[0] |= CELL_VISIBLE; // Errors default here so draw them anyways.
    CullCore(); // Do first Cull pass, forcing as player moved to new cell.
    glUseProgram(Sys_Render.voxelUpdateShaderProgram);
    glUniform1f(0, voxelMinCenterX);
    glUniform1f(1, voxelMinCenterZ);
    glUniform1ui(2, loadedLights);
    glUniform1f(3, worldMin_x);
    glUniform1f(4, worldMin_z);
    uncullingCameras[0] = UINT16_MAX;
    uncullingCameras[1] = UINT16_MAX;
    uncullingCameras[2] = UINT16_MAX;
    uncullingCameras[3] = UINT16_MAX;
    uncullingCameras[4] = UINT16_MAX;
    DualLog(" took %f secs\n", get_time() - start_time);
    DebugRAM("end of Cull_Init");
}

void AddCameraPosition(uint16_t camIdx) {    
    for (int i=0;i<5;++i) {
        if (uncullingCameras[i] == UINT16_MAX) { uncullingCameras[i] = camIdx; return; } // Found empty slot to put it.
    }

    DualLogError("Ran out of slots for unculling cameras!\n");
}

void RemoveCameraPosition(uint16_t camIdx) {
    for (int i=4;i>=0;--i) {
        if (uncullingCameras[i] == camIdx) {
            for (int j = i;j<5;++j) {
                if (j == 4) { uncullingCameras[j] = UINT16_MAX; break; }
                
                uncullingCameras[j] = uncullingCameras[j + 1]; // Shift list to compact, overwriting slot we removed.
            }
            break;
        }
    }
}

void CameraViewUnculling(void) {
    for (int i=0;i<5;++i) {
        uint16_t camInstanceIdx = uncullingCameras[i];
        if (camInstanceIdx == UINT16_MAX) continue;

        uint32_t camCellIdx = PosGetCellCoords(Sys_Global.instances[camInstanceIdx].position.x, Sys_Global.instances[camInstanceIdx].position.z);
        gridCellStates[camCellIdx] |= CELL_VISIBLE;
        uint32_t cellToCellIdx = camCellIdx * ARRSIZE;
        for (int32_t z=0;z<WORLDZ;++z) {
            for (int32_t x=0;x<WORLDX;++x) {
                int32_t cellIdx = (z * WORLDX) + x;
                size_t flat_idx = (size_t)(cellToCellIdx + cellIdx);
                if (get_cull_bit(precomputedVisibleCellsFromHere,flat_idx)) gridCellStates[cellIdx] |= CELL_VISIBLE;
            }
        }
    }
}

void PortalCulling(void) { // Called just once at end of animation loop for the frame after each frame perfect change to door models becoming either closed or not closed.
    uint16_t playerCellX = PosGetCellCoordX(Sys_Global.instances[PLAYER1].position.x);
    uint16_t playerCellZ = PosGetCellCoordZ(Sys_Global.instances[PLAYER1].position.z);
    PortalCell cellA, cellB;
    for (uint8_t portalIdx=0;portalIdx<MAX_PORTALS;++portalIdx) {
        if (!activePortals[portalIdx].dirty) continue;
        
        activePortals[portalIdx].dirty = false;
        cellA = activePortals[portalIdx].cellA; // Guaranteed order at level load.  A = N or E, B = S or W
        cellB = activePortals[portalIdx].cellB;
        bool isNS = activePortals[portalIdx].portalNS;
        uint16_t cellIdxA = (cellA.z * WORLDX) + cellA.x;
        uint16_t cellIdxB = (cellB.z * WORLDX) + cellB.x;
        if (activePortals[portalIdx].open) { // Open the edges up
            if (isNS) {
                gridCellStates[cellIdxA] &= ~(CELL_CLOSEDSOUTH);
                gridCellStates[cellIdxB] &= ~(CELL_CLOSEDNORTH);
//                 if (entIdx == 499 || entIdx == 509) // Bulkhead or giant elevator door spans 2 cells perpendicular to its isNS direction.
            } else {
                gridCellStates[cellIdxA] &= ~(CELL_CLOSEDWEST);
                gridCellStates[cellIdxB] &= ~(CELL_CLOSEDEAST);
            }
        } else {
            if (isNS) {
                gridCellStates[cellIdxA] |= CELL_CLOSEDSOUTH;
                gridCellStates[cellIdxB] |= CELL_CLOSEDNORTH;
            } else {
                gridCellStates[cellIdxA] |= CELL_CLOSEDWEST;
                gridCellStates[cellIdxB] |= CELL_CLOSEDEAST;
            }
        }
    }
    
    DetermineVisibleCells(playerCellX,playerCellZ); // Recompute full PVS with new closed edges for all portal states.  So much for the precomputed set.
    CameraViewUnculling();
    for (uint16_t i = 0; i < loadedLights; i++) lightDirty[i] = true;
}

bool CullCore(void) {    
    if (Sys_Global.currentLevel >= LEVEL_CYBERSPACE) return false;

    numCellsVisible = 0;
    float pos_x, pos_z;
    uint16_t cellX = (uint16_t)clamp((int32_t)vfloor((Sys_Global.instances[PLAYER1].position.x - worldMin_x + CELLXHALF) / WORLDCELL_WIDTH_F), 0, WORLDX_0BASED);
	uint16_t cellZ = (uint16_t)clamp((int32_t)vfloor((Sys_Global.instances[PLAYER1].position.z - worldMin_z + CELLXHALF) / WORLDCELL_WIDTH_F), 0, WORLDX_0BASED);
    CellCoordsToPos(cellX,cellZ, &pos_x,&pos_z);
    for (int i=0;i<loadedInstances;++i) {
        float distSqrd = squareDistance2D(Sys_Global.instances[i].position.x, Sys_Global.instances[i].position.z, pos_x, pos_z);
        instanceIsLODArray[i] = (distSqrd >= 655.36f); // 25.6f * 25.6f
    }
    
    PortalCulling(); // Update based on portal states.
    glNamedBufferData(Sys_Render.cellVisibleDataID,ARRSIZE * sizeof(uint32_t), gridCellStates, GL_DYNAMIC_DRAW);
    return true;
}

