#ifndef VOXEN_DATA_ENTITIES_H
#define VOXEN_DATA_ENTITIES_H

#include <stdbool.h>

#define MAX_ENTITIES 1024 // Unique entity types, different than INSTANCE_COUNT which is the number of instances of any of these entities.
#define ENT_NAME_MAXLEN_NO_NULL_TERMINATOR 31

// Ordered with name last since it is accessed infrequently so doesn't need to
// hit cache much.
typedef struct {
    uint16_t modelIndex;
    uint16_t texIndex;
    uint16_t glowIndex;
    uint16_t specIndex;
    uint16_t normIndex;
    bool cardchunk;
    char name[ENT_NAME_MAXLEN_NO_NULL_TERMINATOR + 1]; // 31 characters max, nice even multiple of 4 bytes
} Entity;

extern Entity entities[MAX_ENTITIES];

int LoadEntities(void);
void CleanupEntities(bool isBad);

#endif // VOXEN_DATA_ENTITIES_H
