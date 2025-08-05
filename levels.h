#ifndef VOXEN_LEVELS_H
#define VOXEN_LEVELS_H

#include <stdint.h>

extern uint8_t startLevel;
extern uint8_t numLevels; // Can be set by gamedata.txt

int LoadLevelGeometry(uint8_t curlevel);

#endif // VOXEN_LEVELS_H
