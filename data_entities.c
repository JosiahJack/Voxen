#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "data_entities.h"
#include "data_parser.h"
#include "constants.h"
#include "debug.h"
#include "event.h"

Entity entities[MAX_ENTITIES]; // Global array of entity definitions
int entityCount = 0;            // Number of entities loaded
DataParser entity_parser;
const char *valid_entity_keys[] = {"index", "model", "texture", "glowtexture", "spectexture", "normtexture"};
#define NUM_ENTITY_KEYS 6
bool entity_parser_initialized = false;

typedef enum {
    ENT_PARSER = 0,
    ENT_COUNT // Number of subsystems
} EntityLoadDataType;

bool loadEntityItemInitialized[ENT_COUNT] = { [0 ... ENT_COUNT - 1] = false };

// Suppress -Wformat-truncation for LoadEntities so it can share 256 length "path" and truncate it into 32 length "name".
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
int LoadEntities(void) {
    double start_time = get_time();
    
    // Initialize parser with entity-specific keys
    parser_init(&entity_parser, valid_entity_keys, NUM_ENTITY_KEYS, PARSER_DATA);
    if (!parse_data_file(&entity_parser, "./Data/entities.txt")) { DualLogError("Could not parse ./Data/entities.txt!"); parser_free(&entity_parser); return 1; }
    
    loadEntityItemInitialized[ENT_PARSER] = true;
    entity_parser_initialized = true;

    entityCount = entity_parser.count;
    if (entityCount > MAX_ENTITIES) { DualLogError("Too many entities in parser count %d, greater than %d!", entityCount, MAX_ENTITIES); CleanupEntities(true); return 1; }
    if (entityCount == 0) { DualLogError("No entities found in entities.txt"); CleanupEntities(true); return 1; }

    DualLog("Parsing %d entities...\n", entityCount);

    // Populate entities array
    for (int i = 0; i < entityCount; i++) {
        if (entity_parser.entries[i].index == UINT16_MAX) continue;

        // Copy with truncation to 31 characters to fit 32 char array for name.  Smaller for RAM constraints.
        snprintf(entities[i].name, ENT_NAME_MAXLEN_NO_NULL_TERMINATOR + 1, "%s", entity_parser.entries[i].path);
        entities[i].modelIndex = entity_parser.entries[i].modelIndex;
        entities[i].texIndex = entity_parser.entries[i].texIndex;
        entities[i].glowIndex = entity_parser.entries[i].glowIndex;
        entities[i].specIndex = entity_parser.entries[i].specIndex;
        entities[i].normIndex = entity_parser.entries[i].normIndex;
        
//         DualLog("Added entity type: %s, modelIndex: %d, texIndex: %d, "
//                 "glowIndex: %d, specIndex: %d, normIndex: %d\n",
//                 entities[i].name, entities[i].modelIndex, entities[i].texIndex,
//                 entities[i].glowIndex, entities[i].specIndex,
//                 entities[i].normIndex);
    }

//     DualLog("Loaded %d entity definitions\n", entityCount);
    CleanupEntities(false);
    DebugRAM("after loading all entities");
    double end_time = get_time();
    DualLog("Load Entities took %f seconds\n", end_time - start_time);
    return 0;
}
#pragma GCC diagnostic pop // Ok restore string truncation warning

void CleanupEntities(bool isBad) {
    if (loadEntityItemInitialized[ENT_PARSER] && entity_parser_initialized) {
        parser_free(&entity_parser);
        entity_parser_initialized = false;
    }
    if (isBad) {
        entityCount = 0; // Reset entity count on error
    }
}
