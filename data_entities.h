#ifndef VOXEN_DATA_ENTITIES_H
#define VOXEN_DATA_ENTITIES_H

#include <stdbool.h>

#define MAX_ENTITIES 1024 // Unique entity types, different than INSTANCE_COUNT which is the number of instances of any of these entities.

int LoadEntities(void);
void CleanupEntities(bool isBad);

#endif // VOXEN_DATA_ENTITIES_H
