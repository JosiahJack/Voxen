#ifndef VOXEN_DATA_PARSER_H
#define VOXEN_DATA_PARSER_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_ENTRIES 65535 // uint16_t limit
#define MAX_PATH 256
#define MAX_LINE_LENGTH 1048576

typedef enum {
    PARSER_DATA,      // For textures.txt, models.txt
    PARSER_GAME,      // For gamedata.txt
    PARSER_LEVEL      // For level data files
} ParserType;

typedef struct {
    // Game Definition specific fields
    uint8_t levelCount;
    uint8_t startLevel;
    
    // Data specific fields
    char path[MAX_PATH]; // File path (e.g., ./Textures/med1_1.png)
    uint16_t index; // Unique index (0–65535)
    
    // Entity specific fields
    uint16_t modelIndex;
    uint16_t texIndex;
    uint16_t glowIndex;
    uint16_t specIndex;
    uint16_t normIndex;
    bool cardchunk;
    bool doublesided;
    
    // Level Prefab fields
    uint16_t constIndex; // For level data
    struct { float x, y, z; } localPosition; // For level data
    struct { float x, y, z, w; } localRotation; // For level data
    struct { float x, y, z; } localScale; // For level data
} DataEntry;

typedef struct {
    DataEntry* entries;    // For PARSER_DATA and PARSER_GAME
    int count;
    int capacity;
    const char** valid_keys;
    int num_keys;
    ParserType parser_type;
} DataParser;

// Game settings from gamedata.txt
typedef struct {
    uint32_t levelCount;
    uint32_t startLevel;
} GameSettings;

// Level object data
typedef struct {
    uint32_t constIndex;
    char objectName[64];
    struct { float x, y, z; } localPosition;
    struct { float x, y, z, w; } localRotation;
    struct { float x, y, z; } localScale;
} LevelObject;

// Collection of level objects
typedef struct {
    LevelObject* objects;
    uint32_t count;
    uint32_t capacity;
} LevelData;

// static const char *valid_keys[] = {
//     "index",
//     "model",
//     "texture",
//     "glowtexture",
//     "spectexture",
//     "normtexture",
//     "doublesided",
//     "cardchunk",
//     "levelcount",
//     "startlevel",
//     "constIndex",
//     "localPosition.x",
//     "localPosition.y",
//     "localPosition.z",
//     "localRotation.x",
//     "localRotation.y",
//     "localRotation.z",
//     "localRotation.w",
//     "localScale.x",
//     "localScale.y",
//     "localScale.z"
// };
// 
// #define NUM_VALID_KEYS (sizeof(valid_keys) / sizeof(valid_keys[0]))

void parser_init(DataParser *parser, const char **valid_keys, int num_keys, ParserType partype);
bool parse_data_file(DataParser *parser, const char *filename);
void parser_free(DataParser *parser);

#endif // VOXEN_DATA_PARSER_H
