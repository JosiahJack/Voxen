#ifndef VOXEN_DATA_PARSER_H
#define VOXEN_DATA_PARSER_H

#include <stdint.h>
#include <stdbool.h>
#include <GL/glew.h>

// Generic Parser
#define MAX_ENTRIES 65535 // uint16_t limit
#define MAX_PATH 256
#define BOUNDS_ATTRIBUTES_COUNT 7
#define BOUNDS_DATA_OFFSET_MINX 0
#define BOUNDS_DATA_OFFSET_MINY 1
#define BOUNDS_DATA_OFFSET_MINZ 2
#define BOUNDS_DATA_OFFSET_MAXX 3
#define BOUNDS_DATA_OFFSET_MAXY 4
#define BOUNDS_DATA_OFFSET_MAXZ 5
#define BOUNDS_DATA_OFFSET_RADIUS 6

typedef struct {
    uint8_t levelCount;
    uint8_t startLevel;
    uint8_t type;
    bool cardchunk;
    bool doublesided;
    uint16_t index;
    uint16_t modelIndex;
    uint16_t texIndex;
    uint16_t glowIndex;
    uint16_t specIndex;
    uint16_t normIndex;
    uint16_t constIndex; // Changed to uint16_t to align with MAX_ENTRIES
    uint16_t lodIndex; // Model index for LOD to use when far away
    char path[MAX_PATH];
    char modname[MAX_PATH]; // Longest game name is 176 characters, so 256 should be aplenty.
    struct { float x, y, z; } localPosition;
    struct { float x, y, z, w; } localRotation;
    struct { float x, y, z; } localScale;
    float intensity;
    float range;
    float spotAngle;
    struct { float r, g, b; } color;
} DataEntry;

typedef struct {
    DataEntry* entries;
    int count;        // Added to track valid entries
    int capacity;
    const char** valid_keys;
    int num_keys;
} DataParser;

void init_data_entry(DataEntry *entry);
bool read_key_value(FILE *file, DataParser *parser, DataEntry *entry, uint32_t *lineNum, bool *is_eof);
void parser_init(DataParser *parser, const char **valid_keys, int num_keys);
bool parse_data_file(DataParser *parser, const char *filename, int type);

// Textures
extern GLuint colorBufferID;
extern uint16_t textureCount;

bool isDoubleSided(uint32_t texIndexToCheck);
int LoadTextures(void);

// Models
#define MODEL_COUNT 768
#define VERTEX_ATTRIBUTES_COUNT 16 // x,y,z,nx,ny,nz,u,v,texIdx,glowIdx,specIdx,normIdx,modelIdx,instanceIdx,u_lm,v_lm

extern uint32_t modelVertexCounts[MODEL_COUNT];
extern uint32_t modelTriangleCounts[MODEL_COUNT];
extern GLuint modelBoundsID;
extern float modelBounds[MODEL_COUNT * BOUNDS_ATTRIBUTES_COUNT];
extern GLuint vbos[MODEL_COUNT];
extern GLuint tbos[MODEL_COUNT];
extern uint32_t renderableCount;
extern int gameObjectCount;

int LoadGeometry(void);

// Entities
#define MAX_ENTITIES 1024 // Unique entity types, different than INSTANCE_COUNT which is the number of instances of any of these entities.
#define ENT_NAME_MAXLEN_NO_NULL_TERMINATOR 31

// Ordered with name last since it is accessed infrequently so doesn't need to hit cache much.
typedef struct {
    uint16_t modelIndex;
    uint16_t texIndex;
    uint16_t glowIndex;
    uint16_t specIndex;
    uint16_t normIndex;
    uint16_t lodIndex;
    bool cardchunk;
    char name[ENT_NAME_MAXLEN_NO_NULL_TERMINATOR + 1]; // 31 characters max, nice even multiple of 4 bytes
} Entity;

extern Entity entities[MAX_ENTITIES];

int LoadEntities(void);

// Levels
extern uint8_t startLevel;
extern uint8_t numLevels; // Can be set by gamedata.txt

int LoadLevels();
int LoadLevelGeometry(uint8_t curlevel);
int LoadLevelLights(uint8_t curlevel);

#endif // VOXEN_DATA_PARSER_H
