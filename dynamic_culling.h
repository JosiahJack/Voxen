#ifndef VOXEN_DYNAMIC_CULLING_H
#define VOXEN_DYNAMIC_CULLING_H

#include <stdbool.h>
#include "render.h"
#include "instance.h"

#define CELL_VISIBLE       1
#define CELL_OPEN          2
#define CELL_CLOSEDNORTH   4
#define CELL_CLOSEDEAST    8
#define CELL_CLOSEDSOUTH  16
#define CELL_CLOSEDWEST   32
#define CELL_SEES_SUN     64
#define CELL_SEES_SKYBOX 128

extern uint16_t playerCellIdx;
extern uint16_t playerCellIdx_x;
extern uint16_t playerCellIdx_y;
extern uint16_t playerCellIdx_z;
extern float cam_x;
extern float cam_y;
extern float cam_z;
extern uint16_t numCellsVisible;
extern uint8_t gridCellStates[ARRSIZE];
extern bool precomputedVisibleCellsFromHere[ARRSIZE * ARRSIZE];
extern uint16_t cellIndexForInstance[INSTANCE_COUNT];
extern uint16_t cellIndexForLight[LIGHT_COUNT];

bool XZPairInBounds(int x, int z);
int Cull_Init(void);
void CullCore(void);
void Cull();

#endif // VOXEN_DYNAMIC_CULLING_H
