#include <SDL2/SDL.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <malloc.h>
#include <GL/glew.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <math.h>
#include "constants.h"
#include "instance.h"
#include "data_parser.h"
#include "data_models.h"
#include "data_entities.h"
#include "data_levels.h"
#include "debug.h"
#include "render.h"
#include "voxel.h"
#include "event.h"
#include "quatmatvec.h"

//-----------------------------------------------------------------------------
// Model Data Parsing
// #define DEBUG_MODEL_LOAD_DATA 1U
DataParser model_parser;
const char *valid_mdldata_keys[] = {"index"};
#define NUM_MODEL_KEYS 1
#define HASH_SIZE 65536 // Power of 2 for fast modulo
#define HASH(v0, v1) (((v0 * 31 + v1) ^ (v1 * 17)) & (HASH_SIZE - 1))
#define MAX_VERT_COUNT 21444
#define MAX_TRI_COUNT 32449
#define MAX_EDGE_COUNT 97345

#define BOUNDS_ATTRIBUTES_COUNT 7
#define BOUNDS_DATA_OFFSET_MINX 0
#define BOUNDS_DATA_OFFSET_MINY 1
#define BOUNDS_DATA_OFFSET_MINZ 2
#define BOUNDS_DATA_OFFSET_MAXX 3
#define BOUNDS_DATA_OFFSET_MAXY 4
#define BOUNDS_DATA_OFFSET_MAXZ 5
#define BOUNDS_DATA_OFFSET_RADIUS 6

uint32_t modelVertexCounts[MODEL_COUNT];
uint32_t modelTriangleCounts[MODEL_COUNT];
uint32_t modelEdgeCounts[MODEL_COUNT];
GLuint vbos[MODEL_COUNT];
GLuint tbos[MODEL_COUNT]; // Triangle index buffers
GLuint tebos[MODEL_COUNT]; // Triangle's edge indices buffers
GLuint ebos[MODEL_COUNT]; // Edge index buffers
GLuint modelBoundsID;
float modelBounds[MODEL_COUNT * BOUNDS_ATTRIBUTES_COUNT];
uint32_t largestVertCount = 0;
uint32_t largestTriangleCount = 0;
uint32_t largestEdgeCount = 0;
// GLuint vboMasterTable;
// GLuint modelVertexOffsetsID;
// GLuint modelVertexCountsID;
float * tempVertices;
uint32_t * tempTriangles;
uint32_t * tempTriEdges;

// Structure to represent an edge for building edge list
typedef struct {
    uint32_t v0, v1; // Vertex indices (sorted: v0 < v1)
    uint32_t tri0, tri1; // Triangle indices (tri1 = UINT32_MAX if unshared)
} Edge;

Edge * tempEdges;

// Simple hash table for edge lookup
typedef struct {
    uint32_t v0, v1;
    uint32_t edgeIndex;
} EdgeHashEntry;

EdgeHashEntry * edgeHash;

float ** vertexDataArrays;
uint32_t ** triangleDataArrays;
uint32_t ** triEdgeDataArrays;
uint32_t ** edgeDataArrays;
//-----------------------------------------------------------------------------
// Level Data Parsing
DataParser level_parser;
const char *valid_leveldata_keys[] = {
    "constIndex","localPosition.x","localPosition.y","localPosition.z",
    "localRotation.x","localRotation.y","localRotation.z","localRotation.w",
    "localScale.x","localScale.y","localScale.z"};
#define NUM_LEVDAT_KEYS 11
//-----------------------------------------------------------------------------

void parser_init(DataParser *parser, const char **valid_keys, int num_keys, ParserType partype) {
    parser->entries = NULL;
    parser->count = 0;
    parser->capacity = 0;
    parser->valid_keys = valid_keys;
    parser->num_keys = num_keys;
    parser->parser_type = partype;
}

uint32_t parse_numberu32(const char* str, const char* line, uint32_t lineNum) {
    if (str == NULL || *str == '\0') { fprintf(stderr, "Invalid input blank string, from line[%d]: %s\n", lineNum, line); return 0; }
    while (isspace((unsigned char)*str)) str++;
    if (*str == '-') { fprintf(stderr, "Invalid input, negative not allowed (%s), from line: %s\n", str, line); return 0; }
    char* endptr;
    errno = 0;
    unsigned long val = strtoul(str, &endptr, 10);
    if (errno != 0 || val > UINT32_MAX) { fprintf(stderr, "Invalid input %s, from line[%d]: %s\n", str, lineNum, line); return 0; }
    return (uint32_t)val;
}

uint16_t parse_numberu16(const char* str, const char* line, uint32_t lineNum) {
    uint32_t retval = parse_numberu32(str, line, lineNum);
    if (retval > UINT16_MAX) { fprintf(stderr, "Value out of range for uint16_t: %u from line[%d]: %s\n", retval, lineNum, line); return 0; }
    return (uint16_t)retval;
}

uint8_t parse_numberu8(const char* str, const char* line, uint32_t lineNum) {
    uint32_t retval = parse_numberu32(str, line, lineNum);
    if (retval > UINT8_MAX) { fprintf(stderr, "Value out of range for uint8_t: %u from line[%d]: %s\n", retval, lineNum, line); return 0; }
    return (uint8_t)retval;
}

bool parse_bool(const char* str, const char* line, uint32_t lineNum) {
    return (parse_numberu32(str, line, lineNum) > 0);
}

float parse_float(const char* str, const char* line, uint32_t lineNum) {
    if (str == NULL || *str == '\0') { fprintf(stderr, "Invalid float input blank string, from line[%d]: %s\n", lineNum, line); return 0.0f; }
    char* endptr;
    errno = 0;
    float val = strtof(str, &endptr);
    if (errno != 0 || endptr == str || *endptr != '\0') { fprintf(stderr, "Invalid float input %s, from line[%d]: %s\n", str, lineNum, line); return 0.0f; }
    return val;
}

static void init_data_entry(DataEntry *entry) {
    entry->path[0] = '\0';
    entry->index = UINT16_MAX;
    entry->modelIndex = UINT16_MAX;
    entry->texIndex = UINT16_MAX;
    entry->glowIndex = UINT16_MAX;
    entry->specIndex = UINT16_MAX;
    entry->normIndex = UINT16_MAX;
    entry->lodIndex = UINT16_MAX;
    entry->doublesided = false;
    entry->cardchunk = false;
    entry->constIndex = 0;
    entry->levelCount = 0;
    entry->startLevel = 0;
    entry->localPosition.x = 0.0f;
    entry->localPosition.y = 0.0f;
    entry->localPosition.z = 0.0f;
    entry->localRotation.x = 0.0f;
    entry->localRotation.y = 0.0f;
    entry->localRotation.z = 0.0f;
    entry->localRotation.w = 1.0f;
    entry->localScale.x = 1.0f;
    entry->localScale.y = 1.0f;
    entry->localScale.z = 1.0f;
}

static bool allocate_entries(DataParser *parser, int entry_count) {
    if (entry_count > MAX_ENTRIES) {
        DualLogError("Entry count %d exceeds %d\n", entry_count, MAX_ENTRIES);
        entry_count = MAX_ENTRIES;
    }
    if (entry_count > parser->capacity) {
        DataEntry *new_entries = realloc(parser->entries, entry_count * sizeof(DataEntry));
        if (!new_entries) {
            DualLogError("realloc failed for %d entries\n", entry_count);
            return false;
        }
        parser->entries = new_entries;
        for (int i = parser->capacity; i < entry_count; i++) {
            init_data_entry(&parser->entries[i]);
        }
        parser->capacity = entry_count;
    }
    parser->count = entry_count;
    return true;
}

static bool process_key_value(DataParser *parser, DataEntry *entry, const char *key, const char *value, const char *line, uint32_t lineNum) {
    if (!key || !value) {
        DualLogError("Invalid key-value pair at line %u: %s\n", lineNum, line);
        return false;
    }
    while (isspace((unsigned char)*key)) key++;
    while (isspace((unsigned char)*value)) value++;
    char trimmed_key[256];
    char trimmed_value[1024];
    strncpy(trimmed_key, key, sizeof(trimmed_key) - 1);
    strncpy(trimmed_value, value, sizeof(trimmed_value) - 1);
    trimmed_key[sizeof(trimmed_key) - 1] = '\0';
    trimmed_value[sizeof(trimmed_value) - 1] = '\0';
    char *key_end = trimmed_key + strlen(trimmed_key) - 1;
    char *val_end = trimmed_value + strlen(trimmed_value) - 1;
    while (key_end > trimmed_key && isspace((unsigned char)*key_end)) *key_end-- = '\0';
    while (val_end > trimmed_value && isspace((unsigned char)*val_end)) *val_end-- = '\0';

    if (strncmp(trimmed_key, "chunk_", 6) == 0) {
        strncpy(entry->path, trimmed_key, sizeof(entry->path) - 1);
        entry->path[sizeof(entry->path) - 1] = '\0';
        return true;
    }

    for (int i = 0; i < parser->num_keys; i++) {
        if (strcmp(trimmed_key, parser->valid_keys[i]) == 0) {
//             DualLog("Parsing line %d with Key: %s Value: %s\n", lineNum, trimmed_key, trimmed_value);
                 if (strcmp(trimmed_key, "index") == 0)            entry->index = parse_numberu16(trimmed_value, line, lineNum);
            else if (strcmp(trimmed_key, "model") == 0)           entry->modelIndex = parse_numberu16(trimmed_value, line, lineNum);
            else if (strcmp(trimmed_key, "texture") == 0)         entry->texIndex = parse_numberu16(trimmed_value, line, lineNum);
            else if (strcmp(trimmed_key, "glowtexture") == 0)     entry->glowIndex = parse_numberu16(trimmed_value, line, lineNum);
            else if (strcmp(trimmed_key, "spectexture") == 0)     entry->specIndex = parse_numberu16(trimmed_value, line, lineNum);
            else if (strcmp(trimmed_key, "normtexture") == 0)     entry->normIndex = parse_numberu16(trimmed_value, line, lineNum);
            else if (strcmp(trimmed_key, "doublesided") == 0)     entry->doublesided = parse_bool(trimmed_value, line, lineNum);
            else if (strcmp(trimmed_key, "cardchunk") == 0)       entry->cardchunk = parse_bool(trimmed_value, line, lineNum);
            else if (strcmp(trimmed_key, "levelcount") == 0)      { entry->levelCount = parse_numberu8(trimmed_value, line, lineNum); entry->index = 0; }
            else if (strcmp(trimmed_key, "startlevel") == 0)      { entry->startLevel = parse_numberu8(trimmed_value, line, lineNum); entry->index = 0; }
            else if (strcmp(trimmed_key, "constIndex") == 0)      entry->constIndex = parse_numberu16(trimmed_value, line, lineNum);
            else if (strcmp(trimmed_key, "lod") == 0)             entry->lodIndex = parse_numberu16(trimmed_value, line, lineNum);
            else if (strcmp(trimmed_key, "localPosition.x") == 0) entry->localPosition.x = parse_float(trimmed_value, line, lineNum);
            else if (strcmp(trimmed_key, "localPosition.y") == 0) entry->localPosition.y = parse_float(trimmed_value, line, lineNum);
            else if (strcmp(trimmed_key, "localPosition.z") == 0) entry->localPosition.z = parse_float(trimmed_value, line, lineNum);
            else if (strcmp(trimmed_key, "localRotation.x") == 0) entry->localRotation.x = parse_float(trimmed_value, line, lineNum);
            else if (strcmp(trimmed_key, "localRotation.y") == 0) entry->localRotation.y = parse_float(trimmed_value, line, lineNum);
            else if (strcmp(trimmed_key, "localRotation.z") == 0) entry->localRotation.z = parse_float(trimmed_value, line, lineNum);
            else if (strcmp(trimmed_key, "localRotation.w") == 0) entry->localRotation.w = parse_float(trimmed_value, line, lineNum);
            else if (strcmp(trimmed_key, "localScale.x") == 0)    entry->localScale.x = parse_float(trimmed_value, line, lineNum);
            else if (strcmp(trimmed_key, "localScale.y") == 0)    entry->localScale.y = parse_float(trimmed_value, line, lineNum);
            else if (strcmp(trimmed_key, "localScale.z") == 0)    entry->localScale.z = parse_float(trimmed_value, line, lineNum);
            return true;
        }
    }
    
//     DualLogError("Unknown key %s at line %u: %s\n", trimmed_key, lineNum, line);
    return false;
}

static bool read_token(FILE *file, char *token, size_t max_len, char delimiter, bool *is_comment, bool *is_eof, bool *is_newline, uint32_t *lineNum) {
    *is_comment = false;
    *is_eof = false;
    *is_newline = false;
    size_t pos = 0;
    int c;

    while ((c = fgetc(file)) != EOF && isspace(c) && c != '\n');
    if (c == EOF) {
        *is_eof = true;
        return false;
    }
    if (c == '\n') {
        *is_newline = true;
        return false;
    }
    if (c == '/' && (c = fgetc(file)) == '/') {
        *is_comment = true;
        while ((c = fgetc(file)) != EOF && c != '\n');
        return false;
    }
    if (c != EOF) {
        token[pos++] = c;
    }

    while ((c = fgetc(file)) != EOF && c != delimiter && c != '\n' && pos < max_len - 1) {
        token[pos++] = c;
    }
    token[pos] = '\0';
    if (pos >= max_len - 1) {
        DualLogError("Token truncated at line %u\n", *lineNum);
    }
    if (c == EOF) *is_eof = true;
    if (c == '\n') *is_newline = true;
    return pos > 0;
}

static bool read_key_value(FILE *file, DataParser *parser, DataEntry *entry, uint32_t *lineNum, bool *is_eof) {
    char token[1024];
    bool is_comment, is_newline;
    if (!read_token(file, token, sizeof(token), ':', &is_comment, is_eof, &is_newline, lineNum)) {
        if (is_comment || is_newline) {
            if (is_newline) (*lineNum)++;
            return false;
        }
    }
    char key[256];
    strncpy(key, token, sizeof(key) - 1);
    key[sizeof(key) - 1] = '\0';
    if (!read_token(file, token, sizeof(token), '\n', &is_comment, is_eof, &is_newline, lineNum)) {
        return false;
    }
    process_key_value(parser, entry, key, token, key, *lineNum);
    (*lineNum)++;
    return true;
}

static bool ParseResourceData(DataParser *parser, const char *filename) {
    DualLog("Parsing file of type 1, resource data, named: %s\n", filename);
    FILE *file = fopen(filename, "r");
    if (!file) {
        DualLogError("Cannot open %s\n", filename);
        return false;
    }

    // Temporary buffer and counters
    char line[1024];
    uint32_t lineNum = 0;
    int entry_count = 0;

    // First pass: count entries by counting lines starting with #
    while (fgets(line, sizeof(line), file)) {
        lineNum++;
        if (line[0] == '#') entry_count++;
    }

    if (!allocate_entries(parser, entry_count)) {
        fclose(file);
        return false;
    }

    // Second pass: parse entries
    rewind(file);
    DataEntry entry;
    init_data_entry(&entry);
    int current_index = 0;

    while (fgets(line, sizeof(line), file)) {
        lineNum++;
        // Trim leading/trailing whitespace
        char *start = line;
        while (isspace((unsigned char)*start)) start++;
        char *end = start + strlen(start) - 1;
        while (end > start && isspace((unsigned char)*end)) *end-- = '\0';

        if (*start == '\0') continue; // skip empty lines
        if (start[0] == '/' && start[1] == '/') continue; // skip empty lines and comment lines


        if (*start == '#') {
            // Store previous entry if valid
            if (entry.path[0] && current_index < parser->count) {
                parser->entries[current_index] = entry;
                current_index++;
            }
            // Start new entry
            init_data_entry(&entry);
            strncpy(entry.path, start + 1, sizeof(entry.path) - 1);
            entry.path[sizeof(entry.path) - 1] = '\0';
            continue;
        }

        // Handle key-value pair
        char *colon = strchr(start, ':');
        if (colon) {
            *colon = '\0';
            char *key = start;
            char *value = colon + 1;
            while (isspace((unsigned char)*key)) key++;
            while (isspace((unsigned char)*value)) value++;
            if (*key && *value) {
                process_key_value(parser, &entry, key, value, start, lineNum);
            }
        }
    }

    // Store last entry on EOF
    if (entry.path[0] && current_index < parser->count) {
        parser->entries[current_index] = entry;
        current_index++;
    }

    DualLog("Total entries stored for %s: %d\n", filename, current_index);
    fclose(file);
    return true;
}

static bool ParseGameDefinitions(DataParser *parser, const char *filename) {
    DualLog("Parsing file of type 2, game definition data, named: %s\n", filename);
    FILE *file = fopen(filename, "r");
    if (!file) {
        DualLogError("Cannot open %s\n", filename);
        return false;
    }

    if (!allocate_entries(parser, 1)) {
        fclose(file);
        return false;
    }

    DataEntry entry;
    init_data_entry(&entry);
    uint32_t lineNum = 0;
    bool is_eof;

    while (!feof(file)) {
        read_key_value(file, parser, &entry, &lineNum, &is_eof);
    }

//     DualLog("Loaded entry for game definition with the following values:\n  levelcount: %d\n  startLevel: %d\n",entry.levelCount, entry.startLevel);
    parser->entries[0] = entry;
    fclose(file);
    return true;
}

static bool ParseSaveLevelData(DataParser *parser, const char *filename) {
    DualLog("startLevel: %d\n",startLevel);
    DualLog("Parsing file of type 3, save/level data, named: %s\n", filename);
    FILE *file = fopen(filename, "r");
    if (!file) { DualLogError("Cannot open %s\n", filename); return false; }

    int entry_count = 0;
    bool is_comment, is_eof, is_newline;
    int c;
    uint32_t lineNum = 0;
    while ((c = fgetc(file)) != EOF) {
        if (isspace(c) || c == '\n') continue;
        if (c == '/' && (c = fgetc(file)) == '/') {
            while ((c = fgetc(file)) != EOF && c != '\n');
            continue;
        }

        entry_count++;
        while ((c = fgetc(file)) != EOF && c != '\n');
    }

    if (!allocate_entries(parser, entry_count)) {
        fclose(file);
        return false;
    }

    rewind(file);
    int current_index = 0;
    lineNum = 0;
    char token[1024];
    DataEntry entry;
    init_data_entry(&entry);

    while (!feof(file)) {
        if (!read_token(file, token, sizeof(token), '|', &is_comment, &is_eof, &is_newline, &lineNum)) {
            if (is_comment) { lineNum++; continue; }
            
            if (is_newline && entry.path[0]) {
                if (current_index < parser->count) {
                    parser->entries[current_index++] = entry;
                    DualLog("Loaded entry for save/level data %s with following values:\n  constIndex: %d\n  localPosition: %f  %f  %f\n  localRotation: %f  %f  %f  %f\n  localScale: %f  %f  %f\n",
                            entry.path, entry.constIndex, entry.localPosition.x, entry.localPosition.y, entry.localPosition.z,
                            entry.localRotation.x, entry.localRotation.y, entry.localRotation.z, entry.localRotation.w,
                            entry.localScale.x, entry.localScale.y, entry.localScale.z);
                }
                init_data_entry(&entry);
                lineNum++;
            }
            continue;
        }
        char *colon = strchr(token, ':');
        if (colon) {
            *colon = '\0';
            char *key = token;
            char *value = colon + 1;
            process_key_value(parser, &entry, key, value, token, lineNum);
        } else {
            strncpy(entry.path, token, sizeof(entry.path) - 1);
            entry.path[sizeof(entry.path) - 1] = '\0';
        }
        if (is_newline && entry.path[0]) {
            if (current_index < parser->count) {
                parser->entries[current_index++] = entry;
            }
            init_data_entry(&entry);
            lineNum++;
        }
    }

    if (entry.path[0] && current_index < parser->count) {
        parser->entries[current_index] = entry;
        DualLog("Loaded entry for save/level data %s with following values:\n  constIndex: %d\n  localPosition: %f  %f  %f\n  localRotation: %f  %f  %f  %f\n  localScale: %f  %f  %f\n",
                entry.path, entry.constIndex, entry.localPosition.x, entry.localPosition.y, entry.localPosition.z,
                entry.localRotation.x, entry.localRotation.y, entry.localRotation.z, entry.localRotation.w,
                entry.localScale.x, entry.localScale.y, entry.localScale.z);
    }

    fclose(file);
    return true;
}

bool parse_data_file(DataParser *parser, const char *filename) {
    switch (parser->parser_type) {
        case PARSER_DATA: return ParseResourceData(parser, filename);
        case PARSER_GAME: return ParseGameDefinitions(parser, filename);
        case PARSER_LEVEL: return ParseSaveLevelData(parser, filename);
        default:
            DualLogError("Unknown parser type %d for file %s\n", parser->parser_type, filename);
            return false;
    }
}

void parser_free(DataParser *parser) {
    free(parser->entries);
    parser->entries = NULL;
    parser->count = 0;
    parser->capacity = 0;
}

//-----------------------------------------------------------------------------
// Loads all 3D meshes
void CleanupModelLoad(bool isBad) {
    parser_free(&model_parser);
    if (tempVertices) { free(tempVertices); tempVertices = NULL; }
    if (tempTriangles) { free(tempTriangles); tempTriangles = NULL; }
    if (tempTriEdges) { free(tempTriEdges); tempTriEdges = NULL; }
    if (tempEdges) { free(tempEdges); tempEdges = NULL; }
    if (edgeHash) { free(edgeHash); edgeHash = NULL; }
    if (isBad) {
        if (vertexDataArrays) { free(vertexDataArrays); vertexDataArrays = NULL; }
        if (triangleDataArrays) { free(triangleDataArrays); triangleDataArrays = NULL; }
        if (triEdgeDataArrays) { free(triEdgeDataArrays); triEdgeDataArrays = NULL; }
        if (edgeDataArrays) { free(edgeDataArrays); edgeDataArrays = NULL; }
    }
    malloc_trim(0);
}

int LoadGeometry(void) {
    double start_time = get_time();
    parser_init(&model_parser, valid_mdldata_keys, NUM_MODEL_KEYS, PARSER_DATA); // First parse ./Data/models.txt to see what to load to what indices
    if (!parse_data_file(&model_parser, "./Data/models.txt")) { DualLogError("Could not parse ./Data/models.txt!\n"); parser_free(&model_parser); return 1; }

    int maxIndex = -1;
    for (int k=0;k<model_parser.count;k++) {
        if (model_parser.entries[k].index > maxIndex && model_parser.entries[k].index != UINT16_MAX) maxIndex = model_parser.entries[k].index;
    }

    DualLog("Parsing %d models...\n",model_parser.count);
    int totalVertCount = 0;
    int totalBounds = 0;
    int totalTriCount = 0;
    int totalEdgeCount = 0;
    largestVertCount = 0;
    largestTriangleCount = 0;
    largestEdgeCount = 0;

    // Allocate persistent temporary buffers
    tempVertices = (float *)malloc(MAX_VERT_COUNT * VERTEX_ATTRIBUTES_COUNT * sizeof(float));
    if (!tempVertices) { DualLogError("Failed to allocate tempVertices buffer\n"); return 1; }
    DebugRAM("tempVertices buffer");

    tempTriangles = (uint32_t *)malloc(MAX_TRI_COUNT * 3 * sizeof(uint32_t));
    if (!tempTriangles) { DualLogError("Failed to allocate tempTriangles temporary buffer\n"); CleanupModelLoad(true); return 1; }
    DebugRAM("tempTriangles buffer");

    tempTriEdges = (uint32_t *)malloc(MAX_TRI_COUNT * 3 * sizeof(uint32_t));
    if (!tempTriEdges) { DualLogError("Failed to allocate tempTriEdges temporary buffer\n"); CleanupModelLoad(true); return 1; }
    DebugRAM("tempTriEdges buffer");

    tempEdges = (Edge *)calloc(MAX_EDGE_COUNT, sizeof(Edge));
    if (!tempEdges) { DualLogError("Failed to allocate tempEdges temporary buffer\n"); CleanupModelLoad(true); return 1; }
    DebugRAM("tempEdges buffer");

    edgeHash = (EdgeHashEntry *)calloc(HASH_SIZE, sizeof(EdgeHashEntry));
    if (!edgeHash) { DualLogError("Failed to allocate edgeHash temporary buffer\n"); CleanupModelLoad(true); return 1; }
    DebugRAM("edgeHash buffer");

    vertexDataArrays = (float **)calloc(MODEL_COUNT, sizeof(float *));
    if (!vertexDataArrays) { DualLogError("Failed to allocate vertexDataArrays\n"); CleanupModelLoad(true); return 1; }
    DebugRAM("vertexDataArrays buffer");

    triangleDataArrays = (uint32_t **)calloc(MODEL_COUNT, sizeof(uint32_t *));
    if (!triangleDataArrays) { DualLogError("Failed to allocate triangleDataArrays\n"); CleanupModelLoad(true); return 1; }
    DebugRAM("triangleDataArrays buffer");

    triEdgeDataArrays = (uint32_t **)calloc(MODEL_COUNT, sizeof(uint32_t *));
    if (!triEdgeDataArrays) { DualLogError("Failed to allocate triEdgeDataArrays\n"); CleanupModelLoad(true); return 1; }
    DebugRAM("triEdgeDataArrays buffer");

    edgeDataArrays = (uint32_t **)calloc(MODEL_COUNT, sizeof(uint32_t *));
    if (!edgeDataArrays) { DualLogError("Failed to allocate edgeDataArrays\n"); CleanupModelLoad(true); return 1; }
    DebugRAM("edgeDataArrays buffer");

    // Generate staging buffers
    GLuint stagingVBO, stagingTBO, stagingTEBO, stagingEBO;
    glGenBuffers(1, &stagingVBO);
    glGenBuffers(1, &stagingTBO);
    glGenBuffers(1, &stagingTEBO);
    glGenBuffers(1, &stagingEBO);
    glBindBuffer(GL_ARRAY_BUFFER, stagingVBO);
    glBufferData(GL_ARRAY_BUFFER, MAX_VERT_COUNT * VERTEX_ATTRIBUTES_COUNT * sizeof(float), NULL, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stagingTBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_TRI_COUNT * 3 * sizeof(uint32_t), NULL, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stagingTEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_TRI_COUNT * 3 * sizeof(uint32_t), NULL, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stagingEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_EDGE_COUNT * 4 * sizeof(uint32_t), NULL, GL_DYNAMIC_COPY);
    DebugRAM("after staging buffers allocation");
    for (uint32_t i = 0; i < MODEL_COUNT; i++) {
        int matchedParserIdx = -1;
        for (int k=0;k<model_parser.count;k++) {
            if (model_parser.entries[k].index == i) {matchedParserIdx = k; break; }
        }

        if (matchedParserIdx < 0) continue;
        if (!model_parser.entries[matchedParserIdx].path || model_parser.entries[matchedParserIdx].path[0] == '\0') continue;

        struct aiPropertyStore* props = aiCreatePropertyStore(); // Disable non-essential FBX components
        aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_READ_ANIMATIONS, 0); // Disable animations
        aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_READ_MATERIALS, 0); // Disable materials
        aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_READ_TEXTURES, 0); // Disable textures
        aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_READ_LIGHTS, 0); // Disable lights
        aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_READ_CAMERAS, 0); // Disable cameras
        aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_OPTIMIZE_EMPTY_ANIMATION_CURVES, 1); // Drop empty animations
        aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_NO_SKELETON_MESHES, 1); // Disable skeleton meshes
        aiSetImportPropertyInteger(props, AI_CONFIG_PP_RVC_FLAGS, aiComponent_ANIMATIONS | aiComponent_BONEWEIGHTS | aiComponent_MATERIALS | aiComponent_TEXTURES | aiComponent_LIGHTS | aiComponent_CAMERAS); // Remove non-mesh components
        aiSetImportPropertyInteger(props, AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT); // Skip non-triangular primitives
        aiSetImportPropertyInteger(props, AI_CONFIG_PP_ICL_PTCACHE_SIZE, 12); // Optimize vertex cache
        aiSetImportPropertyInteger(props, AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4); // Limit bone weights
        aiSetImportPropertyInteger(props, AI_CONFIG_PP_FD_REMOVE, 1); // Remove degenerate primitives
        aiSetImportPropertyInteger(props, AI_CONFIG_PP_PTV_KEEP_HIERARCHY, 0); // Disable hierarchy preservation
        const struct aiScene *scene = aiImportFileExWithProperties(model_parser.entries[matchedParserIdx].path, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices, NULL, props);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) { DualLogError("Assimp failed to load %s: %s\n", model_parser.entries[matchedParserIdx].path, aiGetErrorString()); CleanupModelLoad(true); return 1; }

        // Count vertices, triangles, and estimate edges
        uint32_t vertexCount = 0;
        uint32_t triCount = 0;
        for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
            vertexCount += scene->mMeshes[m]->mNumVertices;
            triCount += scene->mMeshes[m]->mNumFaces;
        }

        if (vertexCount > MAX_VERT_COUNT || triCount > MAX_TRI_COUNT || triCount * 3 > MAX_EDGE_COUNT) { DualLogError("Model %s exceeds buffer limits: verts=%u (> %u), tris=%u (> %u), edges=%u (> %u)\n", model_parser.entries[matchedParserIdx].path, vertexCount, MAX_VERT_COUNT, triCount, MAX_TRI_COUNT, triCount * 3, MAX_EDGE_COUNT); CleanupModelLoad(true); return 1; }

        modelVertexCounts[i] = vertexCount;
        modelTriangleCounts[i] = triCount;
        uint32_t edgeCount = 0; // Will be updated after edge processing
        if (vertexCount > largestVertCount) largestVertCount = vertexCount;
        if (triCount > largestTriangleCount) largestTriangleCount = triCount;

#ifdef DEBUG_MODEL_LOAD_DATA
        if (triCount > 5000U) {
            DualLog("Model %s loaded with %d vertices, \033[1;33m%d\033[0;0m triangles\n", model_parser.entries[matchedParserIdx].path, vertexCount, triCount);
        } else {
            //DualLog("Model %s loaded with %d vertices, %d triangles\n", model_parser.entries[matchedParserIdx].path, vertexCount, triCount);
        }
#endif
        totalVertCount += vertexCount;
        totalTriCount += triCount;

        // Clear hash table
        memset(edgeHash, 0, HASH_SIZE * sizeof(EdgeHashEntry));

        // Extract vertex and triangle data, build edge list
        uint32_t vertexIndex = 0;
        float minx = 1E9f;
        float miny = 1E9f;
        float minz = 1E9f;
        float maxx = -1E9f;
        float maxy = -1E9f;
        float maxz = -1E9f;
        uint32_t triangleIndex = 0;
        uint32_t triEdgeIndex = 0;
        uint32_t globalVertexOffset = 0;

        for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
            struct aiMesh *mesh = scene->mMeshes[m];
            // Vertex data
            for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
                tempVertices[vertexIndex++] = mesh->mVertices[v].x; // Position
                tempVertices[vertexIndex++] = mesh->mVertices[v].y;
                tempVertices[vertexIndex++] = mesh->mVertices[v].z;
                tempVertices[vertexIndex++] = mesh->mNormals[v].x;
                tempVertices[vertexIndex++] = mesh->mNormals[v].y;
                tempVertices[vertexIndex++] = mesh->mNormals[v].z;
                float tempU = mesh->mTextureCoords[0] ? mesh->mTextureCoords[0][v].x : 0.0f;
                float tempV = mesh->mTextureCoords[0] ? mesh->mTextureCoords[0][v].y : 0.0f;
                tempVertices[vertexIndex++] = tempU;
                tempVertices[vertexIndex++] = tempV;
                tempVertices[vertexIndex++] = 0; // Tex Index      Indices used later
                tempVertices[vertexIndex++] = 0; // Glow Index
                tempVertices[vertexIndex++] = 0; // Spec Index
                tempVertices[vertexIndex++] = 0; // Normal Index
                tempVertices[vertexIndex++] = 0; // Model Index
                tempVertices[vertexIndex++] = 0; // Instance Index
                if (mesh->mVertices[v].x < minx) minx = mesh->mVertices[v].x;
                if (mesh->mVertices[v].x > maxx) maxx = mesh->mVertices[v].x;
                if (mesh->mVertices[v].y < miny) miny = mesh->mVertices[v].y;
                if (mesh->mVertices[v].y > maxy) maxy = mesh->mVertices[v].y;
                if (mesh->mVertices[v].z < minz) minz = mesh->mVertices[v].z;
                if (mesh->mVertices[v].z > maxz) maxz = mesh->mVertices[v].z;
            }

            // Triangle and edge data
            for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
                struct aiFace *face = &mesh->mFaces[f];
                if (face->mNumIndices != 3) { DualLogError("Non-triangular face detected in %s, face %u\n", model_parser.entries[matchedParserIdx].path, f); CleanupModelLoad(true); return 1; }

                uint32_t v[3] = {face->mIndices[0] + globalVertexOffset,
                                 face->mIndices[1] + globalVertexOffset,
                                 face->mIndices[2] + globalVertexOffset};
                uint32_t triangleIdx = triangleIndex / 3;
                uint32_t edgeIndices[3];

                // Validate vertex indices
                if (v[0] >= vertexCount || v[1] >= vertexCount || v[2] >= vertexCount) { DualLogError("Invalid vertex index in %s, face %u: v0=%u, v1=%u, v2=%u, vertexCount=%u\n", model_parser.entries[matchedParserIdx].path, f, v[0], v[1], v[2], vertexCount); CleanupModelLoad(true); return 1; }

                // Store vertex indices
                tempTriangles[triangleIndex++] = v[0];
                tempTriangles[triangleIndex++] = v[1];
                tempTriangles[triangleIndex++] = v[2];

                // Process edges with hash table
                for (int e = 0; e < 3; e++) {
                    uint32_t v0 = v[e];
                    uint32_t v1 = v[(e + 1) % 3];
                    if (v0 > v1) { uint32_t temp = v0; v0 = v1; v1 = temp; }

                    uint32_t hash = HASH(v0, v1);
                    while (edgeHash[hash].edgeIndex != 0 && (edgeHash[hash].v0 != v0 || edgeHash[hash].v1 != v1)) {
                        hash = (hash + 1) & (HASH_SIZE - 1); // Linear probing
                    }

                    int edgeFound = -1;
                    if (edgeHash[hash].edgeIndex != 0) {
                        edgeFound = edgeHash[hash].edgeIndex - 1;
//                         if (tempEdges[edgeFound].tri1 != UINT32_MAX) DualLogError("%s has >2 triangles for edge %d\n", model_parser.entries[matchedParserIdx].path, edgeFound);
                        tempEdges[edgeFound].tri1 = triangleIdx;
                    } else {
                        if (edgeCount < MAX_EDGE_COUNT) {
                            tempEdges[edgeCount].v0 = v0;
                            tempEdges[edgeCount].v1 = v1;
                            tempEdges[edgeCount].tri0 = triangleIdx;
                            tempEdges[edgeCount].tri1 = UINT32_MAX;
                            edgeHash[hash].v0 = v0;
                            edgeHash[hash].v1 = v1;
                            edgeHash[hash].edgeIndex = edgeCount + 1;
                            edgeFound = edgeCount++;
                        } else DualLogError("Edge count exceeds estimate for %s: %u >= %u\n", model_parser.entries[matchedParserIdx].path, edgeCount, MAX_EDGE_COUNT);
                    }

                    edgeIndices[e] = edgeFound;
                }

                // Store edge indices
                tempTriEdges[triEdgeIndex++] = edgeIndices[0];
                tempTriEdges[triEdgeIndex++] = edgeIndices[1];
                tempTriEdges[triEdgeIndex++] = edgeIndices[2];
            }
            globalVertexOffset += mesh->mNumVertices;
        }

        // Store vertex data in vertexDataArrays
        vertexDataArrays[i] = (float *)malloc(vertexCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float));
        if (!vertexDataArrays[i]) {
            DualLogError("Failed to allocate vertexDataArrays[%u]\n", i);
            CleanupModelLoad(true);
            return 1;
        }
        memcpy(vertexDataArrays[i], tempVertices, vertexCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float));

        // Store triangle data in triangleDataArrays
        triangleDataArrays[i] = (uint32_t *)malloc(triCount * 3 * sizeof(uint32_t));
        if (!triangleDataArrays[i]) {
            DualLogError("Failed to allocate triangleDataArrays[%u]\n", i);
            CleanupModelLoad(true);
            return 1;
        }
        memcpy(triangleDataArrays[i], tempTriangles, triCount * 3 * sizeof(uint32_t));

        // Store triangle-edge data in triEdgeDataArrays
        triEdgeDataArrays[i] = (uint32_t *)malloc(triCount * 3 * sizeof(uint32_t));
        if (!triEdgeDataArrays[i]) {
            DualLogError("Failed to allocate triEdgeDataArrays[%u]\n", i);
            CleanupModelLoad(true);
            return 1;
        }
        memcpy(triEdgeDataArrays[i], tempTriEdges, triCount * 3 * sizeof(uint32_t));

        // Store edge data in edgeDataArrays
        edgeDataArrays[i] = (uint32_t *)malloc(edgeCount * 4 * sizeof(uint32_t));
        if (!edgeDataArrays[i]) {
            DualLogError("Failed to allocate edgeDataArrays[%u]\n", i);
            CleanupModelLoad(true);
            return 1;
        }
        for (uint32_t j = 0; j < edgeCount; j++) {
            edgeDataArrays[i][j * 4 + 0] = tempEdges[j].v0;
            edgeDataArrays[i][j * 4 + 1] = tempEdges[j].v1;
            edgeDataArrays[i][j * 4 + 2] = tempEdges[j].tri0;
            edgeDataArrays[i][j * 4 + 3] = tempEdges[j].tri1;
        }

        // Update modelEdgeCounts
        modelEdgeCounts[i] = edgeCount;
        totalEdgeCount += edgeCount;
        if (edgeCount > largestEdgeCount) largestEdgeCount = edgeCount;

        aiReleaseImport(scene);
        malloc_trim(0);

        modelEdgeCounts[i] = edgeCount;
        totalEdgeCount += edgeCount;
        if (edgeCount > largestEdgeCount) largestEdgeCount = edgeCount;

        // Copy to staging buffers
        if (vertexCount > 0) {
            glBindBuffer(GL_ARRAY_BUFFER, stagingVBO);
            void *mapped_buffer = glMapBufferRange(GL_ARRAY_BUFFER, 0, vertexCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
            if (!mapped_buffer) { DualLogError("Failed to map stagingVBO for model %d\n", i); CleanupModelLoad(true); return 1; }
            memcpy(mapped_buffer, tempVertices, vertexCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float));
            glUnmapBuffer(GL_ARRAY_BUFFER);
            glGenBuffers(1, &vbos[i]);
            glBindBuffer(GL_COPY_WRITE_BUFFER, vbos[i]);
            glBufferData(GL_COPY_WRITE_BUFFER, vertexCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float), NULL, GL_STATIC_DRAW);
            glCopyBufferSubData(GL_ARRAY_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, vertexCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float));
            glFlush();
            glFinish();
        }

        if (triCount > 0) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stagingTBO);
            void *mapped_buffer = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, triCount * 3 * sizeof(uint32_t), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
            if (!mapped_buffer) { DualLogError("Failed to map stagingTBO for model %d\n", i); CleanupModelLoad(true); return 1; }
            memcpy(mapped_buffer, tempTriangles, triCount * 3 * sizeof(uint32_t));
            glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
            glGenBuffers(1, &tbos[i]);
            glBindBuffer(GL_COPY_WRITE_BUFFER, tbos[i]);
            glBufferData(GL_COPY_WRITE_BUFFER, triCount * 3 * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
            glCopyBufferSubData(GL_ELEMENT_ARRAY_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, triCount * 3 * sizeof(uint32_t));
            glFlush();
            glFinish();
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stagingTEBO);
            mapped_buffer = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, triCount * 3 * sizeof(uint32_t), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
            if (!mapped_buffer) { DualLogError("Failed to map stagingTEBO for model %d\n", i); CleanupModelLoad(true); return 1; }
            memcpy(mapped_buffer, tempTriEdges, triCount * 3 * sizeof(uint32_t));
            glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
            glGenBuffers(1, &tebos[i]);
            glBindBuffer(GL_COPY_WRITE_BUFFER, tebos[i]);
            glBufferData(GL_COPY_WRITE_BUFFER, triCount * 3 * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
            glCopyBufferSubData(GL_ELEMENT_ARRAY_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, triCount * 3 * sizeof(uint32_t));
            glFlush();
            glFinish();
        }

        if (edgeCount > 0) {
            uint32_t *tempEdgeData = (uint32_t *)malloc(edgeCount * 4 * sizeof(uint32_t));
            if (!tempEdgeData) { DualLogError("Failed to allocate edge buffer for %s\n", model_parser.entries[matchedParserIdx].path); CleanupModelLoad(true); return 1; }
            for (uint32_t j = 0; j < edgeCount; j++) {
                tempEdgeData[j * 4 + 0] = tempEdges[j].v0;
                tempEdgeData[j * 4 + 1] = tempEdges[j].v1;
                tempEdgeData[j * 4 + 2] = tempEdges[j].tri0;
                tempEdgeData[j * 4 + 3] = tempEdges[j].tri1;
            }
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stagingEBO);
            void *mapped_buffer = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, edgeCount * 4 * sizeof(uint32_t), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
            if (!mapped_buffer) { DualLogError("Failed to map stagingEBO for model %d\n", i); CleanupModelLoad(true); return 1; }
            memcpy(mapped_buffer, tempEdgeData, edgeCount * 4 * sizeof(uint32_t));
            glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
            glGenBuffers(1, &ebos[i]);
            glBindBuffer(GL_COPY_WRITE_BUFFER, ebos[i]);
            glBufferData(GL_COPY_WRITE_BUFFER, edgeCount * 4 * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
            glCopyBufferSubData(GL_ELEMENT_ARRAY_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, edgeCount * 4 * sizeof(uint32_t));
            glFlush();
            glFinish();
            free(tempEdgeData);
        }

        float minx_pos = fabs(minx);
        float miny_pos = fabs(miny);
        float minz_pos = fabs(minz);
        float boundradius = minx_pos > miny_pos ? minx_pos : miny_pos;
        boundradius = boundradius > minz_pos ? boundradius : minz_pos;
        boundradius = boundradius > maxx ? boundradius : maxx;
        boundradius = boundradius > maxy ? boundradius : maxy;
        boundradius = boundradius > maxz ? boundradius : maxz;
        modelBounds[(i * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_MINX] = minx;
        modelBounds[(i * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_MINY] = miny;
        modelBounds[(i * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_MINZ] = minz;
        modelBounds[(i * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_MAXX] = maxx;
        modelBounds[(i * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_MAXY] = maxy;
        modelBounds[(i * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_MAXZ] = maxz;
        modelBounds[(i * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_RADIUS] = boundradius;
        totalBounds += BOUNDS_ATTRIBUTES_COUNT;

        malloc_trim(0);
        DebugRAM("post GPU upload for model %s", model_parser.entries[matchedParserIdx].path);
    }

    // Delete staging buffers
    glDeleteBuffers(1, &stagingVBO);
    glDeleteBuffers(1, &stagingTBO);
    glDeleteBuffers(1, &stagingTEBO);
    glDeleteBuffers(1, &stagingEBO);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    DebugRAM("after staging buffers deleted");

#ifdef DEBUG_MODEL_LOAD_DATA
    DualLog("Largest vertex count: %d, triangle count: %d, edge count: %d\n", largestVertCount, largestTriangleCount, largestEdgeCount);
    DualLog("Total vertices: %d (", totalVertCount);
    print_bytes_no_newline(totalVertCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float));
    DualLog(")\nTotal triangles: %d (", totalTriCount);
    print_bytes_no_newline(totalTriCount * 3 * sizeof(uint32_t));
    DualLog(")\nTotal tri-edges: %d (", totalTriCount);
    print_bytes_no_newline(totalTriCount * 3 * sizeof(uint32_t));
    DualLog(")\nTotal edges: %d (", totalEdgeCount);
    print_bytes_no_newline(totalEdgeCount * 4 * sizeof(uint32_t));
    DualLog(")\nBounds (");
    print_bytes_no_newline(totalBounds * sizeof(float));
    DualLog(")\n");
#endif

    DebugRAM("post model load");

    // Clean up
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    DebugRAM("post model GPU data transfer");

    // Pass Model Type Bounds to GPU
    glGenBuffers(1, &modelBoundsID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelBoundsID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, MODEL_COUNT * BOUNDS_ATTRIBUTES_COUNT * sizeof(float), modelBounds, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, modelBoundsID);
    CHECK_GL_ERROR();
    malloc_trim(0);
    DebugRAM("post model model bounds data transfer");

    // Upload modelVertexOffsets
//     uint32_t modelVertexOffsets[MODEL_COUNT];
//     uint32_t offset = 0;
//     for (uint32_t i = 0; i < MODEL_COUNT; i++) {
//         modelVertexOffsets[i] = offset;
//         offset += modelVertexCounts[i];
//     }
//
//     glGenBuffers(1, &modelVertexOffsetsID);
//     CHECK_GL_ERROR();
//     glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelVertexOffsetsID);
//     CHECK_GL_ERROR();
//     glBufferData(GL_SHADER_STORAGE_BUFFER, MODEL_COUNT * sizeof(uint32_t), modelVertexOffsets, GL_STATIC_DRAW);
//     CHECK_GL_ERROR();
//     glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, modelVertexOffsetsID);
//     CHECK_GL_ERROR();

    // Pass Model Vertex Counts to GPU
//     glGenBuffers(1, &modelVertexCountsID);
//     CHECK_GL_ERROR();
//     glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelVertexCountsID);
//     CHECK_GL_ERROR();
//     glBufferData(GL_SHADER_STORAGE_BUFFER, MODEL_COUNT * sizeof(uint32_t), modelVertexCounts, GL_STATIC_DRAW);
//     CHECK_GL_ERROR();
//     glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, modelVertexCountsID);
//     CHECK_GL_ERROR();

    // Create duplicate of all mesh data in one flat buffer in VRAM without using RAM
//     glGenBuffers(1, &vboMasterTable);
//     CHECK_GL_ERROR();
//     glBindBuffer(GL_SHADER_STORAGE_BUFFER, vboMasterTable);
//     CHECK_GL_ERROR();
//     glBufferData(GL_SHADER_STORAGE_BUFFER, totalVertCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float), NULL, GL_STATIC_DRAW);
//     CHECK_GL_ERROR();
//     for (int i = 0; i < MODEL_COUNT; ++i) {
//         glBindBuffer(GL_COPY_READ_BUFFER, vbos[i]);
//         CHECK_GL_ERROR();
//         glBindBuffer(GL_COPY_WRITE_BUFFER, vboMasterTable);
//         CHECK_GL_ERROR();
//         glCopyBufferSubData(GL_COPY_READ_BUFFER,GL_COPY_WRITE_BUFFER,0, modelVertexOffsets[i] * VERTEX_ATTRIBUTES_COUNT * sizeof(float), modelVertexCounts[i] * VERTEX_ATTRIBUTES_COUNT * sizeof(float));
//         CHECK_GL_ERROR();
//     }
//
//     glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 20, vboMasterTable);
//     CHECK_GL_ERROR();

    CleanupModelLoad(false);
    double end_time = get_time();
    DualLog("Load Models took %f seconds\n", end_time - start_time);
    DebugRAM("After full LoadModels completed");
    return 0;
}

//-----------------------------------------------------------------------------
int LoadLevels() {
    double start_time = get_time();
    DebugRAM("before LoadLevels");
    DualLog("Loading level data...\n");
    for (int i=0;i<numLevels;++i) {
        if (LoadLevelGeometry(i)) return 1;
    }

    DebugRAM("after LoadLevels");
    double end_time = get_time();
    DualLog("Load Levels took %f seconds\n", end_time - start_time);
    return 0;
}

int LoadLevelGeometry(uint8_t curlevel) {
    if (curlevel >= numLevels) { DualLogError("Cannot load level %d, out of bounds 0 to %d\n",curlevel,numLevels - 1); return 1; }
    if (curlevel != startLevel) return 0;

    char filename[64];
    snprintf(filename, sizeof(filename), "./Data/CitadelScene_geometry_level%d.txt", curlevel);
    parser_init(&level_parser, valid_leveldata_keys, NUM_LEVDAT_KEYS, PARSER_LEVEL);
    if (!parse_data_file(&level_parser, filename)) { DualLogError("Could not parse %s!\n",filename); parser_free(&level_parser); return 1; }

    int gameObjectCount = level_parser.count;
    DualLog("Loading %d objects for Level %d...\n",gameObjectCount,curlevel);
    for (int idx=0;idx<gameObjectCount;++idx) {
        int entIdx = level_parser.entries[idx].constIndex;
        instances[idx].modelIndex = entities[entIdx].modelIndex;
        instances[idx].texIndex = entities[entIdx].texIndex;
        instances[idx].glowIndex = entities[entIdx].glowIndex;
        instances[idx].specIndex = entities[entIdx].specIndex;
        instances[idx].normIndex = entities[entIdx].normIndex;
        instances[idx].lodIndex = entities[entIdx].lodIndex;
        instances[idx].posx = level_parser.entries[idx].localPosition.x;
        instances[idx].posy = level_parser.entries[idx].localPosition.y;
        instances[idx].posz = level_parser.entries[idx].localPosition.z;
        instances[idx].rotx = level_parser.entries[idx].localRotation.x;
        instances[idx].roty = level_parser.entries[idx].localRotation.y;
        instances[idx].rotz = level_parser.entries[idx].localRotation.z;
        instances[idx].rotw = level_parser.entries[idx].localRotation.w;
        instances[idx].sclx = level_parser.entries[idx].localScale.x;
        instances[idx].scly = level_parser.entries[idx].localScale.y;
        instances[idx].sclz = level_parser.entries[idx].localScale.z;
    }

    // TEST CORNEL BOX TODO: DELETE

    // Test orienteering capability of existing quaternion code
    for (int i = 0; i < 6; ++i) {
        instances[i].posx = -2.56f;
        instances[i].posy = -2.56f;
        instances[i].posz = 0.0f;
        instances[i].modelIndex = 178; // generic lod card chunk
    }

    // Barrel
    instances[458].posx = -2.2f;
    instances[458].posy = -2.0f;
    instances[458].posz = -1.28f;
    instances[458].rotx = 0.0f;
    instances[458].roty = 0.0f;
    instances[458].rotz = 0.357f;
    instances[458].rotw = 0.934f;
    instances[458].modelIndex = 12;
    instances[458].texIndex = 30;
    instances[458].specIndex = 32;

    // Crate
    instances[472].posx = -2.2f;
    instances[472].posy = -3.4f;
    instances[472].posz = -1.28f;
    instances[472].rotx = 0.0f;
    instances[472].roty = 0.0f;
    instances[472].rotz = 0.199f;
    instances[472].rotw = 0.980f;
    instances[472].modelIndex = 60;
    instances[472].texIndex = 145;
    instances[472].glowIndex = 65535;
    instances[472].sclx = 1.0f;
    instances[472].scly = 1.0f;
    instances[472].sclz = 1.0f;

    // Cornell Box
    instances[0].modelIndex = 280;
    instances[0].texIndex = 513; // North med2_1
    instances[0].glowIndex = 511;
    instances[0].specIndex = 1242;

    // Test orienteering capability of existing quaternion code
    for (int i = 0; i < 6; ++i) {
        instances[i].posx = -2.56f;
        instances[i].posy = -2.56f;
        instances[i].posz = 0.0f;
        instances[i].modelIndex = 178; // generic lod card chunk
    }

    // Barrel
    instances[458].posx = -2.2f;
    instances[458].posy = -2.0f;
    instances[458].posz = -1.28f;
    instances[458].rotx = 0.0f;
    instances[458].roty = 0.0f;
    instances[458].rotz = 0.357f;
    instances[458].rotw = 0.934f;
    instances[458].modelIndex = 12;
    instances[458].texIndex = 30;
    instances[458].specIndex = 32;

    // Crate
    instances[472].posx = -2.2f;
    instances[472].posy = -3.4f;
    instances[472].posz = -1.28f;
    instances[472].rotx = 0.0f;
    instances[472].roty = 0.0f;
    instances[472].rotz = 0.199f;
    instances[472].rotw = 0.980f;
    instances[472].modelIndex = 60;
    instances[472].texIndex = 145;
    instances[472].glowIndex = 65535;
    instances[472].sclx = 1.0f;
    instances[472].scly = 1.0f;
    instances[472].sclz = 1.0f;

    // Cornell Box
    instances[0].modelIndex = 280;
    instances[0].texIndex = 513; // North med2_1
    instances[0].glowIndex = 511;
    instances[0].specIndex = 1242;

    instances[1].modelIndex = 282;
    instances[1].texIndex = 515; // South med2_2d
    instances[1].glowIndex = 508;
    instances[1].specIndex = 1242;

    instances[2].modelIndex = 244;
    instances[2].texIndex = 483; // East maint3_1

    instances[3].modelIndex = 243;
    instances[3].texIndex = 482; // West maint3_1d
    instances[3].glowIndex = 481;

    instances[5].modelIndex = 262;
    instances[5].texIndex = 499; // floor med1_9 bright teal light
    instances[5].specIndex = 1242;

    instances[4].modelIndex = 278;
    instances[4].texIndex = 507; // ceil med1_7 medical tile floor
    instances[4].specIndex = 1236;
    instances[1].modelIndex = 282;
    instances[1].texIndex = 515; // South med2_2d
    instances[1].glowIndex = 508;
    instances[1].specIndex = 1242;

    instances[2].modelIndex = 244;
    instances[2].texIndex = 483; // East maint3_1

    instances[3].modelIndex = 243;
    instances[3].texIndex = 482; // West maint3_1d
    instances[3].glowIndex = 481;

    instances[5].modelIndex = 262;
    instances[5].texIndex = 499; // floor med1_9 bright teal light
    instances[5].specIndex = 1242;

    instances[4].modelIndex = 278;
    instances[4].texIndex = 507; // ceil med1_7 medical tile floor
    instances[4].specIndex = 1236;

    // 0: Identity rotation (cell North side Y+ from cell center)
    Quaternion q;
    quat_identity(&q);
    instances[0].rotx = q.x;
    instances[0].roty = q.y;
    instances[0].rotz = q.z;
    instances[0].rotw = q.w;
//     printf("North cell side Y+ from cell center quat:: x: %f, y: %f, z: %f, w: %f\n",q.x,q.y,q.z,q.w);
    // Equates to Unity localRotation:
    // Inspector rotation -90, 0, 180, quaternion x: 0 y: 0.70711 z:0.70711 w: 0

    // 1: 180° around Z (cell South side Y- from cell center)
    quat_from_axis_angle(&q, 0.0f, 0.0f, 1.0f, M_PI);
    instances[1].rotx = q.x;
    instances[1].roty = q.y;
    instances[1].rotz = q.z;
    instances[1].rotw = q.w;
//     printf("South cell side Y- from cell center quat:: x: %f, y: %f, z: %f, w: %f\n",q.x,q.y,q.z,q.w);
    // Equates to Unity localRotation:
    // Inspector rotation -90, 0, 0, quaternion x: -0.70711, y: 0, z: 0, w: 0.70711

    // 2: +90° around Z (cell East side X+ from cell center)
    quat_from_axis_angle(&q, 0.0f, 0.0f, 1.0f, M_PI / 2.0f);
    instances[2].rotx = q.x;
    instances[2].roty = q.y;
    instances[2].rotz = q.z;
    instances[2].rotw = q.w;
//     printf("East cell side X+ from cell center quat:: x: %f, y: %f, z: %f, w: %f\n",q.x,q.y,q.z,q.w);

    // 3: −90° around Z (cell West side X− from cell center)
    quat_from_axis_angle(&q, 0.0f, 0.0f, 1.0f, -M_PI / 2.0f);
    instances[3].rotx = q.x;
    instances[3].roty = q.y;
    instances[3].rotz = q.z;
    instances[3].rotw = q.w;
//     printf("West cell side X- from cell center quat:: x: %f, y: %f, z: %f, w: %f\n",q.x,q.y,q.z,q.w);

    // 4: +90° around X (cell Floor Z− from cell center)
    quat_from_axis_angle(&q, 1.0f, 0.0f, 0.0f, -M_PI / 2.0f);
    instances[4].rotx = q.x;
    instances[4].roty = q.y;
    instances[4].rotz = q.z;
    instances[4].rotw = q.w;
//     printf("Down Z- from cell center quat:: x: %f, y: %f, z: %f, w: %f\n",q.x,q.y,q.z,q.w);

    // 5: −90° around X (cell Ceil Z+ from cell center)
    quat_from_axis_angle(&q, 1.0f, 0.0f, 0.0f, M_PI / 2.0f);
    instances[5].rotx = q.x;
    instances[5].roty = q.y;
    instances[5].rotz = q.z;
    instances[5].rotw = q.w;
//     printf("Up Z+ from cell center quat:: x: %f, y: %f, z: %f, w: %f\n",q.x,q.y,q.z,q.w);
    // Equates to Unity localRotation:
    // Inspector rotation -180, 0, 0, quaternion x: 1, y: 0, z: 0, w: 0

// North cell side Y+ from cell center quat:: x: 0.000000, y: 0.000000, z: 0.000000, w: 1.000000  = Unity X+, backtick toward Y+ (0deg roll?)
// South cell side Y- from cell center quat:: x: 0.000000, y: 0.000000, z: 1.000000, w: -0.000000 = Unity X-, backtick toward Y- (0deg roll?)
// East cell side X+ from cell center quat:: x: 0.000000, y: 0.000000, z: 0.707107, w: 0.707107   = Unity
// West cell side X- from cell center quat:: x: -0.000000, y: -0.000000, z: -0.707107, w: 0.707107= Unity
// Down Z- from cell center quat:: x: -0.707107, y: -0.000000, z: -0.000000, w: 0.707107          = Unity
// Up Z+ from cell center quat:: x: 0.707107, y: 0.000000, z: 0.000000, w: 0.707107               = Unity X+, backtick toward Z+ (-90deg roll??)

    parser_free(&level_parser);
    return 0;
}
