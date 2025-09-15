#ifndef VOXEN_DATA_PARSER_H
#define VOXEN_DATA_PARSER_H

#include <stdint.h>
#include <stdbool.h>
#include <GL/glew.h>

// Generic Parser
#define MAX_ENTRIES 65535 // uint16_t limit
#define MAX_PATH 256
#define MODEL_IDX_MAX 1024
#define MATERIAL_IDX_MAX 2048
#define BOUNDS_ATTRIBUTES_COUNT 7
#define BOUNDS_DATA_OFFSET_MINX 0
#define BOUNDS_DATA_OFFSET_MINY 1
#define BOUNDS_DATA_OFFSET_MINZ 2
#define BOUNDS_DATA_OFFSET_MAXX 3
#define BOUNDS_DATA_OFFSET_MAXY 4
#define BOUNDS_DATA_OFFSET_MAXZ 5
#define BOUNDS_DATA_OFFSET_RADIUS 6

#define ENT_NAME_MAXLEN_NO_NULL_TERMINATOR 31

// Ordered with name last since it is accessed infrequently so doesn't need to hit cache much.
typedef struct {
    bool cardchunk;
    bool doublesided;
    uint16_t modelIndex;
    uint16_t texIndex;
    uint16_t glowIndex;
    uint16_t specIndex;
    uint16_t normIndex;
    uint16_t lodIndex;
    struct { float x, y, z; } position;
    struct { float x, y, z, w; } rotation;
    struct { float x, y, z; } scale;
    struct { float x, y, z; } velocity;
    struct { float x, y, z; } angularVelocity;
    float floorHeight;
    float intensity;
    float range;
    float spotAngle;
    struct { float r, g, b; } color;
    char name[ENT_NAME_MAXLEN_NO_NULL_TERMINATOR + 1]; // 31 characters max, nice even multiple of 4 bytes

    uint16_t index;
    uint8_t levelCount;
    uint8_t startLevel;
    uint8_t type;
    bool active;
    uint8_t saveableType;
    char path[MAX_PATH];
    char modname[MAX_PATH]; // Longest game name is 176 characters, so 256 should be aplenty.
} Entity;

#define ENTITY_FIELD_COUNT 33

typedef struct {
    Entity* entries;
    int count;        // Added to track valid entries
    int capacity;
} DataParser;

void init_data_entry(Entity *entry);
bool read_token(FILE *file, char *token, size_t max_len, char delimiter, bool *is_comment, bool *is_eof, bool *is_newline, uint32_t *lineNum);
bool process_key_value(Entity *entry, const char *key, const char *value, const char *line, uint32_t lineNum);
void parser_init(DataParser *parser);
bool parse_data_file(DataParser *parser, const char *filename, int type);

// Textures
extern GLuint colorBufferID;
extern uint16_t textureCount;

bool isDoubleSided(uint32_t texIndexToCheck);
bool isTransparent(uint32_t texIndexToCheck);
int LoadTextures(void);

// Models
#define MODEL_COUNT 680
#define VERTEX_ATTRIBUTES_COUNT 10 // x,y,z,nx,ny,nz,u,v,u_lm,v_lm

extern uint32_t modelVertexCounts[MODEL_COUNT];
extern uint32_t modelTriangleCounts[MODEL_COUNT];
extern GLuint modelBoundsID;
extern float modelBounds[MODEL_COUNT * BOUNDS_ATTRIBUTES_COUNT];
extern GLuint vbos[MODEL_COUNT];
extern GLuint tbos[MODEL_COUNT];
extern uint32_t renderableCount;
extern uint32_t loadedInstances;
extern int gameObjectCount;

int LoadModels(void);

// Entities
#define MAX_ENTITIES 768 // Unique entity types, different than INSTANCE_COUNT which is the number of instances of any of these entities.
#define INSTANCE_COUNT 5800 // Max 5454 for Citadel level 7 geometry, Max 295 for Citadel level 1 dynamic objects

extern Entity entities[MAX_ENTITIES];
extern Entity instances[INSTANCE_COUNT];
extern float modelMatrices[INSTANCE_COUNT * 16];
extern uint8_t dirtyInstances[INSTANCE_COUNT];
extern GLuint instancesBuffer;
extern GLuint matricesBuffer;
extern int startOfDoubleSidedInstances;
extern int startOfTransparentInstances;

int SetupInstances(void);
int LoadEntities(void);

// Levels
extern uint8_t startLevel;
extern uint8_t numLevels; // Can be set by gamedata.txt

int LoadLevels();
int LoadLevelGeometry(uint8_t curlevel);
int LoadLevelLights(uint8_t curlevel);
int LoadLevelDynamicObjects(uint8_t curlevel);

#endif // VOXEN_DATA_PARSER_H
