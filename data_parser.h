#ifndef VOXEN_DATA_PARSER_H
#define VOXEN_DATA_PARSER_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_ENTRIES 65535 // uint16_t limit
#define MAX_PATH 256

typedef enum {
    PARSER_DATA,      // For textures.txt, models.txt, entities.txt
    PARSER_GAME,      // For gamedata.txt
    PARSER_LEVEL      // For sav##.txt, Citadel_${dataset}_level#.txt
} ParserType;

typedef struct {
    uint8_t levelCount;
    uint8_t startLevel;
    char path[MAX_PATH];
    uint16_t index;
    uint16_t modelIndex;
    uint16_t texIndex;
    uint16_t glowIndex;
    uint16_t specIndex;
    uint16_t normIndex;
    bool cardchunk;
    bool doublesided;
    uint16_t constIndex; // Changed to uint16_t to align with MAX_ENTRIES
    struct { float x, y, z; } localPosition;
    struct { float x, y, z, w; } localRotation;
    struct { float x, y, z; } localScale;
} DataEntry;

typedef struct {
    DataEntry* entries;
    int count;        // Added to track valid entries
    int capacity;
    const char** valid_keys;
    int num_keys;
    ParserType parser_type;
} DataParser;

void parser_init(DataParser *parser, const char **valid_keys, int num_keys, ParserType partype);
bool parse_data_file(DataParser *parser, const char *filename);
void parser_free(DataParser *parser);

#endif // VOXEN_DATA_PARSER_H
