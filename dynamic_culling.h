#ifndef VOXEN_DYNAMIC_CULLING_H
#define VOXEN_DYNAMIC_CULLING_H

#include <stdbool.h>
#include "render.h"

typedef struct {
//     MeshRenderer meshRenderer;
//     MeshFilter meshFilter;
//     Mesh meshUsual;
//     Mesh meshLOD;
//     Material materialUsual;
//     Material materialLOD;
//     ShadowCastingMode shadCastModeUsual;
    int constIndex;
//     void SetMesh(bool useLOD) {
//         meshFilter.sharedMesh = useLOD ? meshLOD : meshUsual;
//         meshRenderer.sharedMaterial = useLOD ? materialLOD : materialUsual;
//         meshRenderer.receiveShadows = true;
//     }
} Meshenderer;

typedef struct {
    int x;
    int y;
    int constIndex;
//     GameObject go;
    Meshenderer* meshenderers;
} ChunkPrefab;

typedef struct {
    int x;
    int y;
    int constIndex;
//     GameObject go;
    Meshenderer* meshenderers;
} DynamicObject;

typedef struct {
    int x;
    int y;
    int skyVisible;
    bool open;
    bool visible;
    bool closedNorth; // For when chunk configurations are such that
    bool closedEast;  // the immediately adjacent cell at this edge
    bool closedSouth; // is not visible, consider edge as closed to
    bool closedWest;  // be able to further reduce visible cells.
    float floorHeight;
    bool visibleCellsFromHere[WORLDX * WORLDZ];
    ChunkPrefab* chunkPrefabs;
    DynamicObject* dynamicObjects;
} GridCell;

extern bool cullEnabled;

int Cull_Init(void);

#endif // VOXEN_DYNAMIC_CULLING_H
