#ifndef VOXEN_DATA_PARSER_H
#define VOXEN_DATA_PARSER_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_ENTRIES 65535 // uint16_t limit
#define MAX_PATH 256

typedef enum {
    PARSER_DATA,
    PARSER_DUALDELIMITED
} ParserType;

typedef struct {
    char path[MAX_PATH]; // File path (e.g., ./Textures/med1_1.png)
    uint16_t index; // Unique index (0–65535)
    
    // Entity specific fields
    bool doublesided;
    uint32_t modelIndex;
    uint32_t texIndex;
    uint32_t glowIndex;
    uint32_t specIndex;
    uint32_t normIndex;
} DataEntry;

typedef struct {
    DataEntry *entries;      // Dynamic array
    int count;               // Number of entries
    int capacity;            // Allocated capacity
    const char **valid_keys; // Array of allowed keys (e.g., {"index"})
    int num_keys;            // Number of valid keys
    uint8_t parser_type;
} DataParser;

void parser_init(DataParser *parser, const char **valid_keys, int num_keys, ParserType partype);
bool parse_data_file(DataParser *parser, const char *filename);
void parser_free(DataParser *parser);

#endif // VOXEN_DATA_PARSER_H
