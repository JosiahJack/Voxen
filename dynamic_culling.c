#include <stdint.h>
#include <stdbool.h>
#include "dynamic_culling.h"
#include "render.h"

bool cullEnabled = true;
bool* worldCellsOpenColumns[WORLDX];
bool worldCellsOpenRows[WORLDX];

int Cull_Init(void) {
    return 0;
}
