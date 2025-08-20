#include <stdint.h>
#include <stdbool.h>
#include "dynamic_culling.h"
#include "render.h"
#include "instance.h"

uint8_t gridCellStates[ARRSIZE]; // 4kb
float gridCellFloorHeight[ARRSIZE]; // 16kb
bool precomputedVisibleCellsFromHere[ARRSIZE * ARRSIZE]; // 16mb
uint16_t cellIndexForInstance[INSTANCE_COUNT]; // 11.5kb

uint32_t playerCellIdx = 80000;
uint32_t playerCellIdxLast = 0;
uint32_t playerCellIdx_x = 20000;
uint32_t playerCellIdx_y = 10000;
uint32_t playerCellIdx_z = 451;
uint16_t numCellsVisible = 0;
float worldMin_x = 0.0f;
float worldMin_y = 0.0f;
float worldMin_z = 0.0f;

void PosToCellCoordsChunks(float pos_x, float pos_z, uint16_t* x, uint16_t* y) {
    uint16_t max = WORLDX - 1u; // 63
    *x = (int)((pos_x - worldMin_x + CELLXHALF) / WORLDCELL_WIDTH_F); if (*x > max) *x = max;
    *y = (int)((pos_z - worldMin_z + CELLXHALF) / WORLDCELL_WIDTH_F); if (*y > max) *y = max;
}

void PutChunksInCells() {
    uint16_t x,z;
    uint16_t cellIdx;
    for (uint16_t c=0; c < INSTANCE_COUNT; ++c) {
        PosToCellCoordsChunks(instances[c].posx, instances[c].posz, &x, &z);
        cellIdx = (z * WORLDZ) + x;
        cellIndexForInstance[c] = cellIdx;
        if (instances[c].floorHeight > INVALID_FLOOR_HEIGHT && instances[c].floorHeight > gridCellFloorHeight[cellIdx]) {
            gridCellFloorHeight[cellIdx] = instances[c].floorHeight; // Raise floor up until highest one is selected.
        }
    }
}

int Cull_Init(void) {
    for (int cellIdx = 0; cellIdx < ARRSIZE; ++cellIdx) {
        gridCellStates[cellIdx] = 0u;
        gridCellFloorHeight[cellIdx] = INVALID_FLOOR_HEIGHT;
        for (int subCellIdx = 0; subCellIdx < ARRSIZE; ++subCellIdx) {
            if (subCellIdx != cellIdx) precomputedVisibleCellsFromHere[(cellIdx * ARRSIZE) + subCellIdx] = false;
        }
    }
    
    switch(currentLevel) { // PosToCellCoords -1 on just x
        // chunk.x + (Geometry.x + Level.x),0,chunk.z + (Geometry.z + Level.z)     Sometimes just doing something by hand is the most straightforward...
        case 0: worldMin_x = -38.40f + ( 0.00000f +    3.6000f); worldMin_y = 0.0f; worldMin_z = -51.20f + (0.0f + 1.0f); break;
        case 1: worldMin_x = -76.80f + ( 0.00000f +   25.5600f); worldMin_y = 0.0f; worldMin_z = -56.32f + (0.0f + -5.2f); break;
        case 2: worldMin_x = -40.96f + ( 0.00000f +   -2.6000f); worldMin_y = 0.0f; worldMin_z = -46.08f + (0.0f + -7.7f); break;
        case 3: worldMin_x = -53.76f + (50.17400f +  -45.1200f); worldMin_y = 0.0f; worldMin_z = -46.08f + (13.714f + -16.32f); break;
        case 4: worldMin_x =  -7.68f + ( 1.17800f +  -20.4000f); worldMin_y = 0.0f; worldMin_z = -64.00f + (1.292799f + 11.48f); break;
        case 5: worldMin_x = -35.84f + ( 1.17780f +  -10.1400f); worldMin_y = 0.0f; worldMin_z = -51.20f + (-1.2417f + -0.0383f); break;
        case 6: worldMin_x = -64.00f + ( 1.29280f +   -0.6728f); worldMin_y = 0.0f; worldMin_z = -71.68f + (-1.2033f + 3.76f); break;
        case 7: worldMin_x = -58.88f + ( 1.24110f +   -6.7000f); worldMin_y = 0.0f; worldMin_z = -79.36f + (-1.2544f + 1.16f); break;
        case 8: worldMin_x = -40.96f + (-1.30560f +    1.0800f); worldMin_y = 0.0f; worldMin_z = -43.52f + (1.2928f + 0.8f); break;
        case 9: worldMin_x = -51.20f + (-1.34390f +    3.6000f); worldMin_y = 0.0f; worldMin_z = -64.0f + (-1.1906f + -1.28f); break;
        case 10:worldMin_x =-128.00f + (-0.90945f +  107.3700f); worldMin_y = 0.0f; worldMin_z = -71.68f + (-1.0372f + 35.48f); break;
        case 11:worldMin_x = -38.40f + (-1.26720f +   15.0500f); worldMin_y = 0.0f; worldMin_z =  51.2f + (0.96056f + -77.94f); break;
        case 12:worldMin_x = -34.53f + ( 0.00000f +   19.0400f); worldMin_y = 0.0f; worldMin_z = -123.74f + (0.0f + 95.8f); break;
    }
    
    worldMin_x -= 2.56f; // Add one cell gap around edges
    worldMin_z -= 2.56f;
    PutChunksInCells();
//     DetermineClosedEdges();
//     for (int y=0;y<WORLDX;y++) {
//         for (int x=0;x<WORLDX;x++) {
//             playerCellX = x;
//             playerCellY = y;
//             DetermineVisibleCells(x,y);
//             for (int y2=0;y2<WORLDX;y2++) {
//                 for (int x2=0;x2<WORLDX;x2++) {
//                     gridCells[x,y].visibleCellsFromHere[x2,y2] = gridCells[x2,y2].visible;
//                 }
//             }
//             
//             if (currentLevel == 10) {
//                 if ((x == 15 || x == 16) && y == 23) { // Fix up problem cells at odd angle where ddx doesn't work.
//                     gridCells[x,y].visibleCellsFromHere[12,11] = true;
//                 }
//             }
//         }
//     }
// 
// //     FindMeshRenderers(0); // Static Immutable
// //     FindMeshRenderers(1); // Dynamic
// //     FindMeshRenderers(2); // Doors
// //     FindMeshRenderers(3); // NPCs
// //     FindMeshRenderers(4); // Static Saveable
// //     FindMeshRenderers(5); // Lights
//     UpdatedPlayerCell();
//     for (int y=0;y<WORLDX;y++) {
//         for (int x=0;x<WORLDX;x++) {
//             gridCells[x,y].visible = gridCells[playerCellX,playerCellY].visibleCellsFromHere[x,y]; // Get visible before putting meshes into their cells so we can nudge them a little.
//             worldCellsOpen[x,y] = gridCells[x,y].open || gridCells[x,y].visible;
//         }
//     }
// 
//     gridCells[0,0].visible = true;
//     PutMeshesInCells(0); // Static Immutable
//     PutMeshesInCells(1); // Dynamic
//     PutMeshesInCells(2); // Doors
//     PutMeshesInCells(3); // NPCs
//     PutMeshesInCells(4); // Static Saveable
//     PutMeshesInCells(5); // Lights
    CullCore(); // Do first Cull pass, forcing as player moved to new cell.
    return 0;
}

void CullCore(void) {    
    if (currentLevel >= (numLevels - 1)) return;

    numCellsVisible = 0;
    uint16_t cellIdx = 0;
    uint16_t cellToCellIdx = 0;
    bool visible = false;
    uint16_t x = 0;
    for (uint16_t z=0;z<WORLDZ;++z) {
        for (x=0;x<WORLDX;++x) {
            cellIdx = (z * WORLDX) + x;
            cellToCellIdx = playerCellIdx * ARRSIZE;
            visible = precomputedVisibleCellsFromHere[cellToCellIdx + cellIdx];
            if (visible) {
                numCellsVisible++;
                if (!(gridCellStates[cellIdx] & CELL_VISIBLE)) gridCellStates[cellIdx] = gridCellStates[cellIdx] | CELL_VISIBLE;
                if (!(gridCellStates[cellIdx] & CELL_OPEN)) gridCellStates[cellIdx] = gridCellStates[cellIdx] | CELL_OPEN; // Force visible cells open if for some reason they weren't.
            } else {
                if ((gridCellStates[cellIdx] & CELL_VISIBLE)) gridCellStates[cellIdx] = gridCellStates[cellIdx] - CELL_VISIBLE;
            }
        }
    }

    // Force cell 0 and player cell to visible and open
    if (!(gridCellStates[0] & CELL_VISIBLE)) gridCellStates[0] = gridCellStates[0] | CELL_VISIBLE; // Errors default here so draw them anyways.
    if (!(gridCellStates[0] & CELL_OPEN)) gridCellStates[0] = gridCellStates[0] | CELL_OPEN;
    if (!(gridCellStates[playerCellIdx] & CELL_VISIBLE)) gridCellStates[playerCellIdx] = gridCellStates[playerCellIdx] | CELL_VISIBLE; // Errors default here so draw them anyways.
    if (!(gridCellStates[playerCellIdx] & CELL_OPEN)) gridCellStates[playerCellIdx] = gridCellStates[playerCellIdx] | CELL_OPEN;
//     CameraViewUnculling(playerCellX,playerCellY);
//     ToggleVisibility(); // Update all geometry for cells marked as dirty.
//     ToggleStaticMeshesImmutableVisibility();
//     ToggleStaticImmutableParticlesVisibility();
//     ToggleStaticMeshesSaveableVisibility();
//     ToggleDoorsVisibility();
//     ToggleLightsVisibility();
//     UpdateNPCPVS();
//     ToggleNPCPVS();
//     SetSkyVisible(skyVisType);
}

uint32_t Flatten3DIndex(int x, int y, int z, int xMax, int yMax) {
    return (uint32_t)(x + (y * xMax) + (z * xMax * yMax));
}

void WorldCellIndexToPosition(uint32_t worldIdx, float * x, float * z, float * y) {
    *x = (worldIdx % WORLDX) * WORLDCELL_WIDTH_F + WORLDCELL_WIDTH_F / 2.0f;
    *y = ((worldIdx / WORLDX) % WORLDY) * WORLDCELL_WIDTH_F + WORLDCELL_WIDTH_F / 2.0f;
    *z = (worldIdx / (WORLDX * WORLDY)) * WORLDCELL_WIDTH_F + WORLDCELL_WIDTH_F / 2.0f;
}

uint32_t PositionToWorldCellIndexX(float x) {
    float cellHalf = WORLDCELL_WIDTH_F / 2.0f;
    int xi = (int)floorf((x + cellHalf) / WORLDCELL_WIDTH_F);
    return (xi < 0 ? 0 : (xi >= WORLDX ? WORLDX - 1 : xi));
}

uint32_t PositionToWorldCellIndexY(float y) {
    float cellHalf = WORLDCELL_WIDTH_F / 2.0f;
    int yi = (int)floorf((y + cellHalf) / WORLDCELL_WIDTH_F);
    return (yi < 0 ? 0 : (yi >= WORLDY ? WORLDY - 1 : yi));
}

uint32_t PositionToWorldCellIndexZ(float z) {
    float cellHalf = WORLDCELL_WIDTH_F / 2.0f;
    int zi = (int)floorf((z + cellHalf) / WORLDCELL_WIDTH_F);
    return (zi < 0 ? 0 : (zi >= WORLDZ ? WORLDZ - 1 : zi));
}

uint32_t PositionToWorldCellIndex(float x, float y, float z) {
    float cellHalf = WORLDCELL_WIDTH_F / 2.0f;
    int xi = (int)floorf((x + cellHalf) / WORLDCELL_WIDTH_F);
    int yi = (int)floorf((y + cellHalf) / WORLDCELL_WIDTH_F);
    int zi = (int)floorf((z + cellHalf) / WORLDCELL_WIDTH_F);
    xi = xi < 0 ? 0 : (xi >= WORLDX ? WORLDX - 1 : xi);
    yi = yi < 0 ? 0 : (yi >= WORLDY ? WORLDY - 1 : yi);
    zi = zi < 0 ? 0 : (zi >= WORLDZ ? WORLDZ - 1 : zi);
    return Flatten3DIndex(xi, yi, zi, WORLDX, WORLDY);
}

void Cull(bool force) {
    if (menuActive || gamePaused || currentLevel >= 13) return;

    playerCellIdx = PositionToWorldCellIndex(cam_x, cam_y, cam_z);
    playerCellIdx_x = PositionToWorldCellIndexX(cam_x);
    playerCellIdx_y = PositionToWorldCellIndexY(cam_y);
    playerCellIdx_z = PositionToWorldCellIndexZ(cam_z);
    if (playerCellIdxLast != playerCellIdx || force) CullCore(); // NAlways do UpdatedPlayerCell to set playerCellX and playerCellY.
    playerCellIdxLast = playerCellIdx;
}
