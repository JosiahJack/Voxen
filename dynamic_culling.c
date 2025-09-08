#include <malloc.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <GL/glew.h>
#include "External/stb_image.h"
#include "dynamic_culling.h"
#include "render.h"
#include "instance.h"
#include "debug.h"

uint8_t gridCellStates[ARRSIZE];
float gridCellFloorHeight[ARRSIZE];
uint32_t precomputedVisibleCellsFromHere[524288]; // 4096 * 4096 / 32
uint32_t cellIndexForInstance[INSTANCE_COUNT];
uint16_t cellIndexForLight[LIGHT_COUNT];
uint16_t playerCellIdx = 0u;
uint16_t playerCellIdx_x = 0u; uint16_t playerCellIdx_y = 0u; uint16_t playerCellIdx_z = 0u;
uint16_t numCellsVisible = 0u;
float worldMin_x = 0.0f; float worldMin_z = 0.0f;

static inline bool get_cull_bit(const uint32_t* arr, size_t idx) {
    size_t word = idx / 32;
    size_t bit = idx % 32;
    return ((arr[word] & (1U << bit)) != 0);
}

static inline void set_cull_bit(uint32_t* arr, size_t idx, bool val) {
    size_t word = idx / 32;
    size_t bit = idx % 32;
    if (val) {
        arr[word] |= (1U << bit);
    } else {
        arr[word] &= ~(1U << bit);
    }
}

void PosToCellCoords(float pos_x, float pos_z, uint16_t* x, uint16_t* z) {
    int max = WORLDX - 1; // 63
    int xval = (int)((pos_x - worldMin_x + CELLXHALF) / WORLDCELL_WIDTH_F);
    if (xval > max) xval = max;
    if (xval < 0) xval = 0;
    *x = (uint16_t)xval;
    
    int zval = (int)((pos_z - worldMin_z + CELLXHALF) / WORLDCELL_WIDTH_F);
    if (zval > max) zval = max;
    if (zval < 0) zval = 0;
    *z = (uint16_t)zval;
}

void PutChunksInCells() {
    uint16_t x,z;
    uint16_t cellIdx;
    for (uint16_t c=0; c < INSTANCE_COUNT; ++c) {
        PosToCellCoords(instances[c].posx, instances[c].posz, &x, &z);
        cellIdx = (z * WORLDX) + x;
        cellIndexForInstance[c] = (uint32_t)cellIdx;
        if (instances[c].floorHeight > INVALID_FLOOR_HEIGHT && instances[c].floorHeight > gridCellFloorHeight[cellIdx]) {
            gridCellFloorHeight[cellIdx] = instances[c].floorHeight; // Raise floor up until highest one is selected.
        }
    }
}

void PutMeshesInCells(int type) {
    int count = 0;
    switch(type) {
        case 5: count = LIGHT_COUNT; break; // Lights
    }
    for (int index=0;index<count;index++) {
        uint16_t x,z;
        switch(type) {
            case 5: // Lights
                int lightIdx = (index * LIGHT_DATA_SIZE);
                PosToCellCoords(lights[lightIdx + LIGHT_DATA_OFFSET_POSX],lights[lightIdx + LIGHT_DATA_OFFSET_POSZ], &x, &z);
                cellIndexForLight[index] = (z * WORLDX) + x;
                break;
        }
    }
}

int DetermineClosedEdges() {
    DebugRAM("Start of DetermineClosedEdges");
    size_t maxFileSize = 1000000; // 1MB
    uint8_t* file_buffer = malloc(maxFileSize);
    char filename[256];
    sprintf(filename,"./Data/worldedgesclosed_%d.png",currentLevel);

    FILE* fp = fopen(filename, "rb");
    if (!fp) { DualLogError("Failed to open %s\n", filename); return 1; }
    
    fseek(fp, 0, SEEK_END);
    size_t file_size = ftell(fp);
    if (file_size > maxFileSize) { DualLogError("PNG file %s too large (%zu bytes)\n", filename, file_size); return 1; }
    
    fseek(fp, 0, SEEK_SET);
    size_t read_size = fread(file_buffer, 1, file_size, fp);
    fclose(fp);
    if (read_size != file_size) { DualLogError("Failed to read %s\n", filename); return 1; }

    int wpng, hpng, channels;
    unsigned char* edgePixels = stbi_load_from_memory(file_buffer, file_size, &wpng, &hpng, &channels, STBI_rgb_alpha); // I handmade them, well what can ya do
    if (!edgePixels) { DualLogError("Failed to read %s for culling closed edges\n", filename); return 1; }

    unsigned char closedData_r, closedData_g, closedData_b, closedData_a;
    for (int x=0;x<WORLDX;x++) {
        for (int z=0;z<WORLDZ;z++) {
            int cellIdx = (z * WORLDX) + x;
            gridCellStates[cellIdx] &= ~(CELL_CLOSEDNORTH | CELL_CLOSEDEAST | CELL_CLOSEDSOUTH | CELL_CLOSEDWEST); // Mark all edges not closed
            int flippedZ = (WORLDZ - 1) - z; // Flip z to match Unity's bottom-left origin for Texture2D vs stbi_load's top-left
            int pixelIdx = (x + (flippedZ * WORLDX)) * 4; // 4 channels
            closedData_r = edgePixels[pixelIdx + 0];
            closedData_g = edgePixels[pixelIdx + 1];
            closedData_b = edgePixels[pixelIdx + 2];
            closedData_a = edgePixels[pixelIdx + 3];
            if (closedData_r > 127) gridCellStates[cellIdx] |= CELL_CLOSEDNORTH;
            if (closedData_g > 127) gridCellStates[cellIdx] |= CELL_CLOSEDEAST;
            if (closedData_b > 127) gridCellStates[cellIdx] |= CELL_CLOSEDSOUTH;
            if (   (closedData_r < 255 && closedData_r > 0)
                || (closedData_g < 255 && closedData_g > 0)
                || (closedData_b < 255 && closedData_b > 0)) {
                
                // Anything that has closed west edge will be not at full 255 on at least one channel.
                // Typical for all other edge conditions is to use full brightness 255 on the channel(s).
                // All 4 closed would be 128 128 128 but this doesn't ever happen.
                // None closed is 0 0 0
                gridCellStates[cellIdx] |= CELL_CLOSEDWEST;
            }
            
            if (closedData_a > 0 && closedData_a < 255) {
                gridCellStates[cellIdx] |= CELL_CLOSEDNORTH | CELL_CLOSEDEAST | CELL_CLOSEDSOUTH | CELL_CLOSEDWEST;
            }
        }
    }
    
    stbi_image_free(edgePixels);
    malloc_trim(0);

    char filename2[256];
    sprintf(filename2,"./Data/worldcellopen_%d.png",currentLevel);
    fp = fopen(filename2, "rb");
    if (!fp) { DualLogError("Failed to open %s\n", filename2); return 1; }
    
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    if (file_size > maxFileSize) { DualLogError("PNG file %s too large (%zu bytes)\n", filename2, file_size); return 1; }
    
    fseek(fp, 0, SEEK_SET);
    read_size = fread(file_buffer, 1, file_size, fp);
    fclose(fp);
    if (read_size != file_size) { DualLogError("Failed to read %s\n", filename2); return 1; }
    unsigned char* openPixels = stbi_load_from_memory(file_buffer, file_size, &wpng, &hpng, &channels, STBI_rgb_alpha); // I handmade them, well what can ya do
	if (!openPixels) { DualLogError("Failed to read %s for culling open cells\n", filename2); return 1; }
 
    unsigned char openData_r, openData_g, openData_b;
    for (int x=0;x<WORLDX;++x) {
        for (int z=0;z<WORLDZ;++z) {
            int cellIdx = (z * WORLDX) + x;
            gridCellStates[cellIdx] &= ~CELL_OPEN;
            int flippedZ = (WORLDZ - 1) - z; // Flip z to match Unity's bottom-left origin for Texture2D vs stbi_load's top-left
            int pixelIdx = (x + (flippedZ * WORLDX)) * 4; // 4 channels
            openData_r = openPixels[pixelIdx + 0];
            openData_g = openPixels[pixelIdx + 1];
            openData_b = openPixels[pixelIdx + 2];
            if (openData_r > 0 || openData_g > 0 || openData_b > 0) {
                gridCellStates[cellIdx] |= CELL_OPEN;
            } else {
                gridCellStates[cellIdx] |= CELL_CLOSEDNORTH | CELL_CLOSEDEAST | CELL_CLOSEDSOUTH | CELL_CLOSEDWEST; // Also force close the edges for closed cells even if above edges image said tweren't closed edges.
            }
        }
    }
    
    gridCellStates[0] |= CELL_OPEN; // Force the fallback error cell to be open (forced visible later, open is static, visible is transient)
    stbi_image_free(openPixels);
    malloc_trim(0);
    
    char filename3[256];
    sprintf(filename3,"./Data/worldcellskyvis_%d.png",currentLevel);
    fp = fopen(filename3, "rb");
    if (!fp) { DualLogError("Failed to open %s\n", filename3); return 1; }
    
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    if (file_size > maxFileSize) { DualLogError("PNG file %s too large (%zu bytes)\n", filename3, file_size); return 1; }
    
    fseek(fp, 0, SEEK_SET);
    read_size = fread(file_buffer, 1, file_size, fp);
    fclose(fp);
    if (read_size != file_size) { DualLogError("Failed to read %s\n", filename3); return 1; }
    unsigned char* skyPixels = stbi_load_from_memory(file_buffer, file_size, &wpng, &hpng, &channels, STBI_rgb_alpha); // I handmade them, well what can ya do
    if (!skyPixels) { DualLogError("Failed to read %s for culling sky visibility\n", filename3); return 1; }

    unsigned char skyData_r, skyData_g, skyData_b;
    for (int x=0;x<WORLDX;++x) {
        for (int z=0;z<WORLDZ;++z) {
            int cellIdx = (z * WORLDX) + x;
            int flippedZ = (WORLDZ - 1) - z; // Flip z to match Unity's bottom-left origin for Texture2D vs stbi_load's top-left
            int pixelIdx = (x + (flippedZ * WORLDX)) * 4; // 4 channels
            skyData_r = skyPixels[pixelIdx + 0];
            skyData_g = skyPixels[pixelIdx + 1];
            skyData_b = skyPixels[pixelIdx + 2];
            if (skyData_r > 127 && skyData_g < 127 && skyData_b < 127) gridCellStates[cellIdx] &= ~(CELL_SEES_SUN | CELL_SEES_SKYBOX); // All red cells marked as -1, no sky or sun.
            else if (skyData_r <= 127 && skyData_g <= 127 && skyData_b > 127) gridCellStates[cellIdx] |= CELL_SEES_SUN | CELL_SEES_SKYBOX; // All blue cells marked as sky visible.  Sun + Sky.
            else { gridCellStates[cellIdx] &= ~CELL_SEES_SKYBOX; gridCellStates[cellIdx] |= CELL_SEES_SUN; } // All white and black cells marked as 0.  Only sees Sun.
        }
    }
    
    stbi_image_free(skyPixels);
    malloc_trim(0);
    free(file_buffer);
    malloc_trim(0);
    DebugRAM("end of dynamic culling DetermineClosedEdges");
    return 0;
}

bool UpdatedPlayerCell() {
    uint16_t lastX = playerCellIdx_x;
    uint16_t lastZ = playerCellIdx_z;
    PosToCellCoords(cam_x,cam_z,&playerCellIdx_x,&playerCellIdx_z);
    playerCellIdx = (playerCellIdx_z * WORLDX) + playerCellIdx_x;
    if (playerCellIdx_x == lastX && playerCellIdx_z == lastZ) return false;
    return true;
}

bool XZPairInBounds(int x, int z) {
    return (x < WORLDX && z < WORLDZ && x >= 0 && z >= 0);
}

int CastRayCellCheck(int x, int z, int lastX, int lastZ) {
    if (!(lastX == x && lastZ == z)) {
        if (XZPairInBounds(lastX,lastZ)) {
            int cellIdx_last = (lastZ * WORLDX) + lastX;
            if (lastZ == z) {
                if (lastX > x) { // [  x  ][lastX]
                    if (gridCellStates[cellIdx_last] & CELL_CLOSEDWEST) return -1;
                } else { // Less than x since == x was already checked.
                    if (gridCellStates[cellIdx_last] & CELL_CLOSEDEAST) return -1;
                }
            }

            if (lastX == x) {
                if (lastZ > z) { // [lastZ]
                                 // [  y  ]
                    if (gridCellStates[cellIdx_last] & CELL_CLOSEDSOUTH) return -1;
                } else { // Less than y since == y was already checked.
                    if (gridCellStates[cellIdx_last] & CELL_CLOSEDNORTH) return -1;
                }
            }

            // Diagonals
            if (lastZ != z && lastX != x) {
                int cellIdx_neighborNorth = ((lastZ + 1) * WORLDX) + lastX;
                cellIdx_neighborNorth = cellIdx_neighborNorth > ARRSIZE ? ARRSIZE : cellIdx_neighborNorth;
                int cellIdx_neighborSouth = ((lastZ - 1) * WORLDX) + lastX;
                cellIdx_neighborSouth = cellIdx_neighborSouth > ARRSIZE ? ARRSIZE : cellIdx_neighborSouth;
                int cellIdx_neighborEast = (lastZ * WORLDX) + lastX + 1;
                cellIdx_neighborEast = cellIdx_neighborEast > ARRSIZE ? ARRSIZE : cellIdx_neighborEast;
                int cellIdx_neighborWest = (lastZ * WORLDX) + lastX - 1;
                cellIdx_neighborWest = cellIdx_neighborWest > ARRSIZE ? ARRSIZE : cellIdx_neighborWest;
                
                if (lastZ > z && lastX > x) { // [Nb][ 1]
                                              // [ 2][Na]
                    bool neighborClosedWest = false;
                    bool neighborClosedSouth = false;
                    if (XZPairInBounds(lastX,lastZ - 1)) neighborClosedWest = (gridCellStates[cellIdx_neighborSouth] & CELL_CLOSEDWEST) && (gridCellStates[cellIdx_neighborSouth] & CELL_OPEN);
                    if (XZPairInBounds(lastX - 1,lastZ)) neighborClosedSouth = (gridCellStates[cellIdx_neighborWest] & CELL_CLOSEDSOUTH) && (gridCellStates[cellIdx_neighborWest] & CELL_OPEN);
                    if ((gridCellStates[cellIdx_last] & CELL_CLOSEDSOUTH) && (gridCellStates[cellIdx_last] & CELL_CLOSEDWEST)) return -1;// Check cell 1 only
                    if ((gridCellStates[cellIdx_last] & CELL_CLOSEDWEST) && neighborClosedWest) return -1; // Check cell 1 and Neighbor a (Na)
                    if ((gridCellStates[cellIdx_last] & CELL_CLOSEDSOUTH) && neighborClosedSouth) return -1; // Check cell 1 and Neighbor b (Nb)
                    if (neighborClosedWest && neighborClosedSouth) return -1; // Check Neighbor a (Na) and Neighbor b (Nb)
                } else if (lastZ < z && lastX < x) { // [ ][2]
                                                     // [1][ ]return
                    bool neighborClosedEast = false;
                    bool neighborClosedNorth = false;
                    if (XZPairInBounds(lastX,lastZ + 1)) neighborClosedEast = (gridCellStates[cellIdx_neighborNorth] & CELL_CLOSEDEAST) && (gridCellStates[cellIdx_neighborNorth] & CELL_OPEN);
                    if (XZPairInBounds(lastX + 1,lastZ)) neighborClosedNorth = (gridCellStates[cellIdx_neighborEast] & CELL_CLOSEDNORTH) && (gridCellStates[cellIdx_neighborEast] & CELL_OPEN);
                    if ((gridCellStates[cellIdx_last] & CELL_CLOSEDNORTH) && (gridCellStates[cellIdx_last] & CELL_CLOSEDEAST)) return -1;
                    if ((gridCellStates[cellIdx_last] & CELL_CLOSEDEAST) && neighborClosedEast) return -1;
                    if ((gridCellStates[cellIdx_last] & CELL_CLOSEDNORTH) && neighborClosedNorth) return -1;
                    if (neighborClosedEast && neighborClosedNorth) return -1;
                } else if (lastZ > z && lastX < x) { // [1][ ]
                                                     // [ ][2]
                    bool neighborClosedEast = false;
                    bool neighborClosedSouth = false;
                    if (XZPairInBounds(lastX,lastZ - 1)) neighborClosedEast = (gridCellStates[cellIdx_neighborSouth] & CELL_CLOSEDEAST) && (gridCellStates[cellIdx_neighborSouth] & CELL_OPEN);
                    if (XZPairInBounds(lastX + 1,lastZ)) neighborClosedSouth = (gridCellStates[cellIdx_neighborEast] & CELL_CLOSEDSOUTH) && (gridCellStates[cellIdx_neighborEast] & CELL_OPEN);
                    if ((gridCellStates[cellIdx_last] & CELL_CLOSEDSOUTH) && (gridCellStates[cellIdx_last] & CELL_CLOSEDEAST)) return -1;
                    if ((gridCellStates[cellIdx_last] & CELL_CLOSEDEAST) && neighborClosedEast) return -1;
                    if ((gridCellStates[cellIdx_last] & CELL_CLOSEDSOUTH) && neighborClosedSouth) return -1;
                    if (neighborClosedEast && neighborClosedSouth) return -1;
                } else if (lastZ < z && lastX > x) { // [2][ ]
                                                     // [ ][1]
                    bool neighborClosedWest = false;
                    bool neighborClosedNorth = false;
                    if (XZPairInBounds(lastX,lastZ + 1)) neighborClosedWest = (gridCellStates[cellIdx_neighborNorth] & CELL_CLOSEDWEST) && (gridCellStates[cellIdx_neighborNorth] & CELL_OPEN);
                    if (XZPairInBounds(lastX - 1,lastZ)) neighborClosedNorth = (gridCellStates[cellIdx_neighborWest] & CELL_CLOSEDNORTH) && (gridCellStates[cellIdx_neighborWest] & CELL_OPEN);
                    if ((gridCellStates[cellIdx_last] & CELL_CLOSEDNORTH) && (gridCellStates[cellIdx_last] & CELL_CLOSEDWEST)) return -1;
                    if ((gridCellStates[cellIdx_last] & CELL_CLOSEDWEST) && neighborClosedWest) return -1;
                    if ((gridCellStates[cellIdx_last] & CELL_CLOSEDNORTH) && neighborClosedNorth) return -1;
                    if (neighborClosedWest && neighborClosedNorth) return -1;
                }
            }
        }
    }
    
    if (XZPairInBounds(x,z)) {
        int cellIdx_xz = (z * WORLDX) + x; 
        if (gridCellStates[cellIdx_xz] & CELL_OPEN) gridCellStates[cellIdx_xz] |= CELL_VISIBLE;
        else gridCellStates[cellIdx_xz] &= ~CELL_VISIBLE;
        
        if (!(gridCellStates[cellIdx_xz] & CELL_VISIBLE)) return -1;
        return 1;
    }

    return 0;
}

int CastStraightZ(int px, int pz, int signz) {
    if (signz > 0 && pz >= (WORLDZ - 1)) return pz; // Nowwhere to step to if right by edge, hence WORLDX - 1 here.
    if (signz < 0 && pz <= 0) return pz;
    if (!XZPairInBounds(px,pz)) return pz;
    
    int cellIdx = (pz * WORLDX) + px;
    if (!(gridCellStates[cellIdx] & CELL_VISIBLE)) return pz;
    
    bool currentVisible = true;
    int x = px;
    int z = pz + signz;
    int zabs = abs(z);
    for (;zabs<WORLDX;z+=signz) { // Up/Down
        currentVisible = false;
        int cellIdx_x_zmnus1 = ((z - 1) * WORLDX) + x;
        int cellIdx_x_zplus1 = ((z + 1) * WORLDX) + x;
        if (XZPairInBounds(x,z - signz) && XZPairInBounds(x,z)) {
            int cellIdx_x_zmnus_sign = ((z - signz) * WORLDX) + x;
            if (gridCellStates[cellIdx_x_zmnus_sign] & CELL_VISIBLE) {
                if (signz > 0) {
                    if (gridCellStates[cellIdx_x_zmnus1] & CELL_CLOSEDNORTH && gridCellStates[cellIdx_x_zmnus1] & CELL_OPEN) return z;
                } else if (signz < 0) {
                    if (gridCellStates[cellIdx_x_zplus1] & CELL_CLOSEDSOUTH && gridCellStates[cellIdx_x_zplus1] & CELL_OPEN) return z;
                }

                int subCellIdx = (z * WORLDX) + x;
                if (gridCellStates[subCellIdx] & CELL_OPEN) gridCellStates[subCellIdx] |= CELL_VISIBLE;
                else gridCellStates[subCellIdx] &= ~CELL_VISIBLE;
                
                currentVisible = true; // Would be if twas open.
            }
        }

        if (!currentVisible) break; // Hit wall!

        if (XZPairInBounds(x + 1,z)) {
            int cellIdx_xplus1_z = (z * WORLDX) + x + 1;
            if (CastRayCellCheck(x,z,x + 1,z) > 0) {
                if (gridCellStates[cellIdx_xplus1_z] & CELL_OPEN) gridCellStates[cellIdx_xplus1_z] |= CELL_VISIBLE;
                else gridCellStates[cellIdx_xplus1_z] &= ~CELL_VISIBLE;
            } else {
                gridCellStates[cellIdx_xplus1_z] &= ~CELL_VISIBLE;
            }
        }
        
        if (XZPairInBounds(x - 1,z)) {
            int cellIdx_xmnus1_z = (z * WORLDX) + x - 1;
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

int CastStraightX(int px, int pz, int signx) {
    if (signx > 0 && px >= (WORLDX - 1)) return px; // Nowwhere to step to if right by edge, hence WORLDX - 1 here.
    if (signx < 0 && px <= 0) return px;
    if (!XZPairInBounds(px,pz)) return px;
    if (!gridCellStates[(pz * WORLDX) + px] & CELL_VISIBLE) return px;

    int x = px + signx;
    int z = pz;
    bool currentVisible = true;
    int xabs = abs(x);
    for (;xabs<WORLDX;x+=signx) { // Right/Left
        currentVisible = false;
        int cellIdx_xmnus1_z = (z * WORLDX) + x - 1;
        int cellIdx_xplus1_z = (z * WORLDX) + x + 1;
        if (XZPairInBounds(x - signx,z) && XZPairInBounds(x,z)) {
            int cellIdx_xmnussign_z = (z * WORLDX) + x - signx;
            if (gridCellStates[cellIdx_xmnussign_z] & CELL_VISIBLE) {
                if (signx > 0) {
                    if ((gridCellStates[cellIdx_xmnus1_z] & CELL_CLOSEDEAST) && (gridCellStates[cellIdx_xmnus1_z] & CELL_OPEN)) return x;
                } else if (signx < 0) {
                    if ((gridCellStates[cellIdx_xplus1_z] & CELL_CLOSEDWEST) && (gridCellStates[cellIdx_xplus1_z] & CELL_OPEN)) return x;
                }
                
                int subCellIdx = (z * WORLDX) + x;
                if (gridCellStates[subCellIdx] & CELL_OPEN) gridCellStates[subCellIdx] |= CELL_VISIBLE;
                else gridCellStates[subCellIdx] &= ~CELL_VISIBLE;
                
                currentVisible = true; // Would be if twas open.
            }
        }

        if (!currentVisible) break; // Hit wall!
        
        if (XZPairInBounds(x,z + 1)) {
            int cellIdx_x_zplus1 = ((z + 1) * WORLDX) + x;
            if (CastRayCellCheck(x,z,x,z + 1) > 0) {
                if (gridCellStates[cellIdx_x_zplus1] & CELL_OPEN) gridCellStates[cellIdx_x_zplus1] |= CELL_VISIBLE;
                else gridCellStates[cellIdx_x_zplus1] &= ~CELL_VISIBLE;
            } else {
                gridCellStates[cellIdx_x_zplus1] &= ~CELL_VISIBLE;
            }
        }
        
        if (XZPairInBounds(x,z - 1)) {
            int cellIdx_x_zmnus1 = ((z - 1) * WORLDX) + x;
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

void CastRay(int x0, int z0, int x1, int z1) {
    int dx = abs(x1 - x0);       int dz = abs(z1 - z0);
    int sx = (x0 < x1) ? 1 : -1; int sz = (z0 < z1) ? 1 : -1;
    int x = x0;                  int z = z0;
    int lastX = x;               int lastZ = z;
    int err = dx - dz;
    int iter = dx > dz ? dx : dz;
    while (iter >= 0) {
        if (!XZPairInBounds(x,z) || !XZPairInBounds(lastX,lastZ)) continue;
        if (CastRayCellCheck(x,z,lastX,lastZ) == -1) return;

        lastX = x;
        lastZ = z;
        int e2 = 2 * err;
        if (e2 > -dz) { err -= dz; x += sx; }
        if (e2 <  dx) { err += dx; z += sz; }
        iter--;
    }
}

void CircleFanRays(int x0, int z0) { // CastRay()'s in fan from x0,z0 out to every cell around map perimeter.
    if (!XZPairInBounds(x0,z0)) return;
    if (!(gridCellStates[(z0 * WORLDX) + x0] & CELL_VISIBLE)) return;

    int x,z;     
    int max = WORLDX; // Reduce work slightly by not casting towards 
    int min = 0;      // edges but 1 less = [1,63].
    for (x=min;x<max;x++) CastRay(x0,z0,x,min);
    for (x=min;x<max;x++) CastRay(x0,z0,x,max);
    for (z=min;z<max;z++) CastRay(x0,z0,min,z);
    for (z=min;z<max;z++) CastRay(x0,z0,max,z);
}

void DetermineVisibleCells(int startX, int startZ) {
    if (!XZPairInBounds(startX,startZ)) return;

    for (int x=0;x<WORLDX;x++) {
        for (int z=0;z<WORLDZ;z++) {
            int subCellIdx = (z * WORLDX) + x;
            gridCellStates[subCellIdx] &= ~CELL_VISIBLE; // Clear all to not visible.
        }
    }

    int cellIdx_start = (startZ * WORLDX) + startX;
    gridCellStates[cellIdx_start] |= CELL_VISIBLE; // Force starting player cell to visible.
    
    // Cast to the right (East)        [ ][3]
    CastStraightX(startX,startZ,1); // [1][2]
                                    // [ ][3]
    int iter = 0;
    for (int march=startX;march<(WORLDX - 1);march++) {
        iter++;
        if (iter > WORLDX) break;
        
        if (XZPairInBounds(march,startZ + 1)) {
            if (gridCellStates[((startZ + 1) * WORLDX) + march] & CELL_VISIBLE) {
                march = CastStraightX(march,startZ + 1,1);  // Above [1]
            }
        }
    }
    
    iter = 0;
    for (int march=startX;march<(WORLDX - 1);march++) {
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
    for (int march=startX;march>=1;march--) {
        iter++;
        if (iter > WORLDX) break;
        
        if (XZPairInBounds(march,startZ + 1)) {
            if (gridCellStates[((startZ + 1) * WORLDX) + march] & CELL_VISIBLE) {
                march = CastStraightX(march,startZ + 1,-1); // Above [1]
            }
        }
    }

    iter = 0;
    for (int march=startX;march>=1;march--) {
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
    for (int march=startZ;march>=1;march--) {
        iter++;
        if (iter > WORLDX) break;

        if (XZPairInBounds(startX + 1,march)) {
            if (gridCellStates[(march * WORLDX) + startX + 1] & CELL_VISIBLE) {
                march = CastStraightZ(startX + 1,march,-1);
            }
        }
    }
    
    iter = 0;
    for (int march=startZ;march>=1;march--) {
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
    for (int march=startZ;march<(WORLDX - 1);march++) {
        iter++;
        if (iter > WORLDX) break;

        if (XZPairInBounds(startX + 1,march)) {
            if (gridCellStates[(march * WORLDX) + startX + 1] & CELL_VISIBLE) {
                march = CastStraightZ(startX + 1,march,1);
            }
        }
    }
    
    iter = 0;
    for (int march=startZ;march<(WORLDX - 1);march++) {
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
    for (int x=0;x<WORLDX;++x) {
        for (int z=0;z<WORLDZ;++z) {
            int cellIdx_xz = (z * WORLDX) + x;
            if (currentLevel == 5) { // Citadel flight level hackarounds for algorithm discrepancies at glancing angles.
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
    
    int numVisible = 0;
    for (int x=0;x<WORLDX;++x) {
        for (int z=0;z<WORLDZ;++z) {
            if (gridCellStates[(z * WORLDX) + x] & CELL_VISIBLE) numVisible++;
        }
    }
}

int Cull_Init(void) {
    DebugRAM("start of Cull_Init");    
    switch(currentLevel) {
        case 0: worldMin_x = -38.40f + ( 0.00000f +    3.6000f); worldMin_z = -51.20f + (0.0f + 1.0f); break;
        case 1: worldMin_x = -81.92f; worldMin_z = -71.68f; break;
        case 2: worldMin_x = -40.96f + ( 0.00000f +   -2.6000f); worldMin_z = -46.08f + (0.0f + -7.7f); break;
        case 3: worldMin_x = -53.76f + (50.17400f +  -45.1200f); worldMin_z = -46.08f + (13.714f + -16.32f); break;
        case 4: worldMin_x =  -7.68f + ( 1.17800f +  -20.4000f); worldMin_z = -64.00f + (1.292799f + 11.48f); break;
        case 5: worldMin_x = -35.84f + ( 1.17780f +  -10.1400f); worldMin_z = -51.20f + (-1.2417f + -0.0383f); break;
        case 6: worldMin_x = -64.00f + ( 1.29280f +   -0.6728f); worldMin_z = -71.68f + (-1.2033f + 3.76f); break;
        case 7: worldMin_x = -58.88f + ( 1.24110f +   -6.7000f); worldMin_z = -79.36f + (-1.2544f + 1.16f); break;
        case 8: worldMin_x = -40.96f + (-1.30560f +    1.0800f); worldMin_z = -43.52f + (1.2928f + 0.8f); break;
        case 9: worldMin_x = -51.20f + (-1.34390f +    3.6000f); worldMin_z = -64.0f + (-1.1906f + -1.28f); break;
        case 10:worldMin_x =-128.00f + (-0.90945f +  107.3700f); worldMin_z = -71.68f + (-1.0372f + 35.48f); break;
        case 11:worldMin_x = -38.40f + (-1.26720f +   15.0500f); worldMin_z =  51.2f + (0.96056f + -77.94f); break;
        case 12:worldMin_x = -34.53f + ( 0.00000f +   19.0400f); worldMin_z = -123.74f + (0.0f + 95.8f); break;
    }
    
    worldMin_x -= 2.56f; // Add one cell gap around edges
    worldMin_z -= 2.56f;
    PutChunksInCells();
    if (DetermineClosedEdges()) return 1;
    
    // For each cell, get the visibility as though player were there and put into gridCellStates
    // Then store the visibility of gridCellStates into the table of all visible cells for that cell
    // at the appropriate offset for looking up later when actually re-assigning gridCellStates
    // from this precalculated visibility state for the particular cell.
    int numPrecomputedVisibleCells = 0;
    for (int z=0;z<WORLDZ;z++) {
        for (int x=0;x<WORLDX;x++) {
            playerCellIdx_x = x;
            playerCellIdx_z = z;
            DetermineVisibleCells(x,z);
            int cellIdx = (z * WORLDX) + x;
            for (int z2=0;z2<WORLDZ;z2++) {
                for (int x2=0;x2<WORLDX;x2++) {
                    int subCellIdx = (z2 * WORLDX) + x2;
                    size_t flat_idx = (size_t)(cellIdx * ARRSIZE) + subCellIdx;
                    bool is_visible = (gridCellStates[subCellIdx] & CELL_VISIBLE);
                    set_cull_bit(precomputedVisibleCellsFromHere,flat_idx,is_visible);
                    if (is_visible) numPrecomputedVisibleCells++;
                }
            }
            
            if (currentLevel == 10) {
                if ((x == 15 || x == 16) && z == 23) { // Fix up problem cells at odd angle where ddx doesn't work.
                    size_t flat_idx = (size_t)(cellIdx * ARRSIZE) + ((11 * WORLDX) + 12);
                    set_cull_bit(precomputedVisibleCellsFromHere,flat_idx,true);
                    numPrecomputedVisibleCells++;
                }
            }
        }
    }
    
    UpdatedPlayerCell();
    int cellToCellIdx = playerCellIdx * ARRSIZE;
    int numFoundVisibleCellsForPlayerStart = 0;
    for (int z=0;z<WORLDZ;++z) {
        for (int x=0;x<WORLDX;++x) {
            int cellIdx = (z * WORLDX) + x;
            size_t flat_idx = (size_t)(cellToCellIdx + cellIdx);
            if (get_cull_bit(precomputedVisibleCellsFromHere,flat_idx)) {
                numFoundVisibleCellsForPlayerStart++;
                gridCellStates[cellIdx] |= CELL_VISIBLE; // Get visible before putting meshes into their cells so we can nudge them a little.
            }
        }
    }

    gridCellStates[0] |= CELL_VISIBLE; // Errors default here so draw them anyways.
//     PutMeshesInCells(0); // Static Immutable
//     PutMeshesInCells(1); // Dynamic
//     PutMeshesInCells(2); // Doors
//     PutMeshesInCells(3); // NPCs
//     PutMeshesInCells(4); // Static Saveable
    PutMeshesInCells(5); // Lights
    CullCore(); // Do first Cull pass, forcing as player moved to new cell.
    uint32_t numBits = ARRSIZE * ARRSIZE;          // 4096 * 4096
    uint32_t numUint32s = (numBits + 31) / 32;    // ceil(bits/32)
    uint32_t numBytes = numUint32s * sizeof(uint32_t);    
    glGenBuffers(1, &precomputedVisibleCellsFromHereID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, precomputedVisibleCellsFromHereID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, numBytes, precomputedVisibleCellsFromHere, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, precomputedVisibleCellsFromHereID);
    
    glGenBuffers(1, &cellIndexForInstanceID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellIndexForInstanceID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, INSTANCE_COUNT * sizeof(uint32_t), cellIndexForInstance, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 20, cellIndexForInstanceID);

    CHECK_GL_ERROR();
    malloc_trim(0);
    DebugRAM("end of Cull_Init");
    return 0;
}

void CullCore(void) {    
    if (currentLevel >= (numLevels - 1)) return;

    numCellsVisible = 0;
    int cellToCellIdx = playerCellIdx * ARRSIZE;
    for (int z=0;z<WORLDZ;++z) {
        for (int x=0;x<WORLDX;++x) {
            int cellIdx = (z * WORLDX) + x;
            if (cellIdx == 0) { gridCellStates[0] |= CELL_VISIBLE; continue; } // Errors default here so draw them anyways.  Don't count it though.
            if (cellIdx == playerCellIdx) { gridCellStates[playerCellIdx] |= CELL_VISIBLE; numCellsVisible++; continue; } // Always at least set player's cell.

            size_t flat_idx = (size_t)(cellToCellIdx + cellIdx);
            if (get_cull_bit(precomputedVisibleCellsFromHere,flat_idx)) {
                numCellsVisible++;
                gridCellStates[cellIdx] |= CELL_VISIBLE; // Get visible before putting meshes into their cells so we can nudge them a little.
            } else {
                gridCellStates[cellIdx] &= ~CELL_VISIBLE;
            }
        }
    }

//     CameraViewUnculling(playerCellX,playerCellY);
//     UpdateNPCPVS();
//     ToggleNPCPVS();
}

void Cull() {
    if (menuActive || gamePaused || currentLevel >= 13) return;

    // Now handle player position updating PVS. Always do UpdatedPlayerCell
    // to set playerCellX and playerCellY.
    if (UpdatedPlayerCell()) CullCore();
}
