#ifndef VOXEN_DATA_DEFINITIONS_H
#define VOXEN_DATA_DEFINITIONS_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_ENTRIES 65535 // uint16_t limit

typedef struct {
    char path[256]; // File path (e.g., ./Textures/med1_1.png)
    uint16_t index; // Unique index (0–65535)
    // Add more fields for future key-value pairs (e.g., texture format, audio type)
} DataEntry;

typedef struct {
    DataEntry *entries;      // Dynamic array
    int count;               // Number of entries
    int capacity;            // Allocated capacity
    const char **valid_keys; // Array of allowed keys (e.g., {"index"})
    int num_keys;            // Number of valid keys
} DataParser;

void parser_init(DataParser *parser, const char **valid_keys, int num_keys);
bool parse_data_file(DataParser *parser, const char *filename);
void parser_free(DataParser *parser);

#endif // VOXEN_DATA_DEFINITIONS_H
