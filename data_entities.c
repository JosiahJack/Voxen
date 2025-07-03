#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "data_definitions.h"
#include "constants.h"

Entity entities[MAX_ENTITIES]; // Global array of entity definitions
int entityCount = 0;            // Number of entities loaded
DataParser entity_parser;
const char *valid_entity_keys[] = {"index", "model", "texture", "glowtexture", "spectexture", "normtexture"};
bool entity_parser_initialized = false;

typedef enum {
    ENT_PARSER = 0,
    ENT_COUNT // Number of subsystems
} EntityLoadDataType;

bool loadEntityItemInitialized[ENT_COUNT] = { [0 ... ENT_COUNT - 1] = false };

int LoadEntities(void) {
    // Initialize parser with entity-specific keys
    parser_init(&entity_parser, valid_entity_keys, sizeof(valid_entity_keys) / sizeof(valid_entity_keys[0]));
    if (!parse_data_file(&entity_parser, "./Data/entities.txt")) {
        SDL_Log("ERROR: Could not parse ./Data/entities.txt!");
        parser_free(&entity_parser);
        return 1;
    }
    loadEntityItemInitialized[ENT_PARSER] = true;
    entity_parser_initialized = true;

    entityCount = entity_parser.count;
    if (entityCount > MAX_ENTITIES) {
        SDL_Log("ERROR: Too many entities in parser count %d, greater than %d!", entityCount, MAX_ENTITIES);
        CleanupEntities(true);
        return 1;
    }

    if (entityCount == 0) {
        SDL_Log("ERROR: No entities found in entities.txt");
        CleanupEntities(true);
        return 1;
    }

    SDL_Log("Parsing %d entities...", entityCount);

    // Populate entities array
    for (int i = 0; i < entityCount; i++) {
        if (entity_parser.entries[i].index == UINT16_MAX) continue;

        strncpy(entities[i].name, entity_parser.entries[i].path, MAX_PATH - 1);
        entities[i].name[MAX_PATH - 1] = 0;
        entities[i].modelIndex = entity_parser.entries[i].modelIndex;
        entities[i].texIndex = entity_parser.entries[i].texIndex;
        entities[i].glowIndex = entity_parser.entries[i].glowIndex;
        entities[i].specIndex = entity_parser.entries[i].specIndex;
        entities[i].normIndex = entity_parser.entries[i].normIndex;
    }

    SDL_Log("Loaded %d entity definitions", entityCount);
    CleanupEntities(false);
    return 0;
}

void CleanupEntities(bool isBad) {
    if (loadEntityItemInitialized[ENT_PARSER] && entity_parser_initialized) {
        parser_free(&entity_parser);
        entity_parser_initialized = false;
    }
    if (isBad) {
        entityCount = 0; // Reset entity count on error
    }
}
