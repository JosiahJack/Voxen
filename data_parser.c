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
#include "debug.h"
#include "render.h"
#include "event.h"

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

uint32_t modelVertexCounts[MODEL_COUNT];
uint32_t modelTriangleCounts[MODEL_COUNT];
// uint32_t modelEdgeCounts[MODEL_COUNT];
GLuint vbos[MODEL_COUNT];
GLuint tbos[MODEL_COUNT]; // Triangle index buffers
// GLuint tebos[MODEL_COUNT]; // Triangle's edge indices buffers
GLuint ebos[MODEL_COUNT]; // Edge index buffers
GLuint modelBoundsID;
float modelBounds[MODEL_COUNT * BOUNDS_ATTRIBUTES_COUNT];
uint32_t largestVertCount = 0;
uint32_t largestTriangleCount = 0;
// uint32_t largestEdgeCount = 0;
float * tempVertices;
uint32_t * tempTriangles;
// uint32_t * tempTriEdges;

// Structure to represent an edge for building edge list
// typedef struct {
//     uint32_t v0, v1; // Vertex indices (sorted: v0 < v1)
//     uint32_t tri0, tri1; // Triangle indices (tri1 = UINT32_MAX if unshared)
// } Edge;
// 
// Edge * tempEdges;

// Simple hash table for edge lookup
// typedef struct {
//     uint32_t v0, v1;
//     uint32_t edgeIndex;
// } EdgeHashEntry;
// 
// EdgeHashEntry * edgeHash;

float ** vertexDataArrays;
uint32_t ** triangleDataArrays;
uint32_t ** triEdgeDataArrays;
// uint32_t ** edgeDataArrays;
//-----------------------------------------------------------------------------
// Level Data Parsing
DataParser level_parser;
const char *valid_leveldata_keys[] = {"constIndex","localPosition.x","localPosition.y","localPosition.z",
                                      "localRotation.x","localRotation.y","localRotation.z","localRotation.w",
                                      "localScale.x","localScale.y","localScale.z"};
#define NUM_LEVDAT_KEYS 11
    
// Level Lights Parsing
DataParser lights_parser;
const char *valid_lightdata_keys[] = {"localPosition.x","localPosition.y","localPosition.z","intensity","range","type",
                                      "localRotation.x","localRotation.y","localRotation.z","localRotation.w",
                                      "color.r","color.g","color.b","spotAngle"};
#define NUM_LIGHTDAT_KEYS 14
//-----------------------------------------------------------------------------

void parser_init(DataParser *parser, const char **valid_keys, int num_keys) {
    parser->entries = NULL;
    parser->count = 0;
    parser->capacity = 0;
    parser->valid_keys = valid_keys;
    parser->num_keys = num_keys;
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

void init_data_entry(DataEntry *entry) {
    entry->levelCount = 0;
    entry->startLevel = 0;
    entry->type = 0;
    entry->cardchunk = false;
    entry->doublesided = false;
    entry->index = UINT16_MAX;
    entry->modelIndex = UINT16_MAX;
    entry->texIndex = UINT16_MAX;
    entry->glowIndex = UINT16_MAX;
    entry->specIndex = UINT16_MAX;
    entry->normIndex = UINT16_MAX;
    entry->constIndex = 0;
    entry->lodIndex = UINT16_MAX;
    entry->path[0] = '\0';
    entry->localPosition.x = 0.0f; entry->localPosition.y = 0.0f; entry->localPosition.z = 0.0f;
    entry->localRotation.x = 0.0f; entry->localRotation.y = 0.0f; entry->localRotation.z = 0.0f; entry->localRotation.w = 1.0f;
    entry->localScale.x = 1.0f; entry->localScale.y = 1.0f; entry->localScale.z = 1.0f;
    entry->intensity = 0.0f;
    entry->range = 0.0f;
    entry->spotAngle = 0.0f;
    entry->color.r = 0.0f; entry->color.g = 0.0f; entry->color.b = 0.0f;
}

void allocate_entries(DataParser *parser, int entry_count) {
    if (entry_count > MAX_ENTRIES) {
        DualLogWarn("\033[38;5;208mEntry count %d exceeds %d\033[0m\n", entry_count, MAX_ENTRIES);
        entry_count = MAX_ENTRIES;
    }
    
    if (entry_count > parser->capacity) {
        DataEntry *new_entries = realloc(parser->entries, entry_count * sizeof(DataEntry));        
        parser->entries = new_entries;
        for (int i = parser->capacity; i < entry_count; ++i) init_data_entry(&parser->entries[i]);
        parser->capacity = entry_count;
    }
    parser->count = entry_count;
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
                 if (strcmp(trimmed_key, "index") == 0)           entry->index = parse_numberu16(trimmed_value, line, lineNum);
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
            else if (strcmp(trimmed_key, "intensity") == 0)       entry->intensity = parse_float(trimmed_value, line, lineNum);
            else if (strcmp(trimmed_key, "range") == 0)           entry->range = parse_float(trimmed_value, line, lineNum);
            else if (strcmp(trimmed_key, "spotAngle") == 0)       entry->spotAngle = parse_float(trimmed_value, line, lineNum);
            else if (strcmp(trimmed_key, "type") == 0)            entry->type = (strcmp(trimmed_value, "Spot") == 0) ? 1u : 0u;
            else if (strcmp(trimmed_key, "color.r") == 0)         entry->color.r = parse_float(trimmed_value, line, lineNum);
            else if (strcmp(trimmed_key, "color.g") == 0)         entry->color.g = parse_float(trimmed_value, line, lineNum);
            else if (strcmp(trimmed_key, "color.b") == 0)         entry->color.b = parse_float(trimmed_value, line, lineNum);
            return true;
        }
    }
    
    return false;
}

static bool read_token(FILE *file, char *token, size_t max_len, char delimiter, bool *is_comment, bool *is_eof, bool *is_newline, uint32_t *lineNum) {
    *is_comment = false;
    *is_eof = false;
    *is_newline = false;
    size_t pos = 0;
    int c;
    while ((c = fgetc(file)) != EOF && isspace(c) && c != '\n');
    if (c == EOF) { *is_eof = true; return false; }
    if (c == '\n') { *is_newline = true; return false; }
    
    if (c == '/' && (c = fgetc(file)) == '/') {
        *is_comment = true;
        while ((c = fgetc(file)) != EOF && c != '\n');
        return false;
    }
    
    if (c != EOF) token[pos++] = c;
    while ((c = fgetc(file)) != EOF && c != delimiter && c != '\n' && pos < max_len - 1) { token[pos++] = c; }
    token[pos] = '\0';
    if (pos >= max_len - 1) DualLogError("Token truncated at line %u\n", *lineNum);
    if (c == EOF) *is_eof = true;
    if (c == '\n') *is_newline = true;
    return pos > 0;
}

bool read_key_value(FILE *file, DataParser *parser, DataEntry *entry, uint32_t *lineNum, bool *is_eof) {
    char token[1024];
    bool is_comment, is_newline;
    if (!read_token(file, token, sizeof(token), ':', &is_comment, is_eof, &is_newline, lineNum)) {
        if (is_comment || is_newline) {
            if (is_newline) *lineNum += 1;
            return false;
        }
    }
    
    char key[256];
    strncpy(key, token, sizeof(key) - 1);
    key[sizeof(key) - 1] = '\0';
    if (!read_token(file, token, sizeof(token), '\n', &is_comment, is_eof, &is_newline, lineNum))return false;
    
    process_key_value(parser, entry, key, token, key, *lineNum);
    *lineNum += 1;
    return true;
}

static bool ParseResourceData(DataParser *parser, const char *filename) {
//     DualLog("Starting ParseResourceData for file: %s\n", filename);
    FILE *file = fopen(filename, "r");
    if (!file) { DualLogError("Cannot open %s: %s\n", filename, strerror(errno)); return false; }

    char line[1024];
    uint32_t lineNum = 0;
    int entry_count = 0;
    uint32_t max_index = 0;

    // First pass: count entries and find max index
    while (fgets(line, sizeof(line), file)) {
        lineNum++;
        if (line[0] == '#') { entry_count++; continue; }
        
        char *start = line;
        while (isspace((unsigned char)*start)) start++;
        char *end = start + strlen(start) - 1;
        while (end > start && isspace((unsigned char)*end)) *end-- = '\0';
        if (*start == '\0' || (start[0] == '/' && start[1] == '/')) continue;

        char *colon = strchr(start, ':');
        if (colon && strncmp(start, "index", colon - start) == 0) {
            char *value = colon + 1;
            while (isspace((unsigned char)*value)) value++;
            uint32_t idx = parse_numberu32(value, line, lineNum);
            if (idx > max_index) max_index = idx;
        }
    }

    if (entry_count == 0) { DualLogWarn("No entries found in %s\n", filename); fclose(file); return true; }

    // Allocate enough space for max_index + 1
    allocate_entries(parser, max_index + 1);
    parser->count = entry_count; // Track actual number of valid entries

    // Second pass: parse entries
    rewind(file);
    DataEntry entry;
    init_data_entry(&entry);
    int entries_stored = 0;
    lineNum = 0;
    while (fgets(line, sizeof(line), file)) {
        lineNum++;
        char *start = line;
        while (isspace((unsigned char)*start)) start++;
        char *end = start + strlen(start) - 1;
        while (end > start && isspace((unsigned char)*end)) *end-- = '\0';
        if (*start == '\0') continue; // Skip empty line
        if (start[0] == '/' && start[1] == '/') continue; // Skip comment(ed out) line

        if (*start == '#') {
            // Store previous entry if valid
            if (entry.path[0] && entry.index != UINT16_MAX && entry.index < parser->capacity) {
                parser->entries[entry.index] = entry;
                entries_stored++;
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
            } else {
                DualLogWarn("Invalid key-value pair at line %u: %s\n", lineNum, start);
            }
        } else {
            DualLogWarn("No colon found in line %u: %s\n", lineNum, start);
        }
    }

    // Store last entry
    if (entry.path[0] && entry.index != UINT16_MAX && entry.index < parser->capacity) {
        parser->entries[entry.index] = entry;
        entries_stored++;
    }

    fclose(file);
    return true;
}

static bool ParseSaveLevelData(DataParser *parser, const char *filename) {
//     DualLog("Starting ParseSaveLevelData for file: %s\n", filename);
    
    // Check if file exists and is readable
    FILE *file = fopen(filename, "r");
    if (!file) {
        DualLogError("Cannot open %s: %s\n", filename, strerror(errno));
        return false;
    }

    // First pass: count entries
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
    
    if (entry_count == 0) {
        DualLogWarn("No entries found in %s\n", filename);
        fclose(file);
        return true; // Or return false if an empty file is an error
    }

    allocate_entries(parser, entry_count);

    // Second pass: parse entries
    rewind(file);
    int current_index = 0;
    lineNum = 0;
    char token[1024];
    DataEntry entry;
    init_data_entry(&entry);
    while (!feof(file)) {
        if (!read_token(file, token, sizeof(token), '|', &is_comment, &is_eof, &is_newline, &lineNum)) {
            if (is_comment) { lineNum++; continue; }
            
            if (is_newline) {
                if (current_index < parser->count) {
                    parser->entries[current_index++] = entry;
//                     DualLog("Loaded entry %d for save/level data %s with values:\n  constIndex: %d\n  localPosition: %f %f %f\n\n",
//                             current_index, entry.path, entry.constIndex,
//                             entry.localPosition.x, entry.localPosition.y, entry.localPosition.z);
                }
                init_data_entry(&entry);
                lineNum++;
            }
            
            if (is_eof) break;
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

        if (is_newline) {
            if (current_index < parser->count) {
                parser->entries[current_index++] = entry;
//                 DualLog("Loaded entry %d for %s with values:\n  constIndex: %d\n  localPosition: %f %f %f\n\n",
//                         current_index, entry.path, entry.constIndex,
//                         entry.localPosition.x, entry.localPosition.y, entry.localPosition.z);
            }
            init_data_entry(&entry);
            lineNum++;
        }
    }

    // Store last entry
    if (entry.path[0] && current_index < parser->count) {
        parser->entries[current_index] = entry;
        current_index++;
//         DualLog("Loaded final entry %d for save/level data %s with values:\n  constIndex: %d\n  localPosition: %f %f %f\n  localRotation: %f %f %f %f\n  localScale: %f %f %f\n",
//                 current_index, entry.path, entry.constIndex,
//                 entry.localPosition.x, entry.localPosition.y, entry.localPosition.z,
//                 entry.localRotation.x, entry.localRotation.y, entry.localRotation.z, entry.localRotation.w,
//                 entry.localScale.x, entry.localScale.y, entry.localScale.z);
    }

    fclose(file);
    return true;
}

bool parse_data_file(DataParser *parser, const char *filename, int type) {
    if (type == 1) return ParseSaveLevelData(parser, filename);
    else return ParseResourceData(parser, filename);
}

//-----------------------------------------------------------------------------
// Loads all 3D meshes
int LoadGeometry(void) {
    double start_time = get_time();
    parser_init(&model_parser, valid_mdldata_keys, NUM_MODEL_KEYS); // First parse ./Data/models.txt to see what to load to what indices
    if (!parse_data_file(&model_parser, "./Data/models.txt",0)) { DualLogError("Could not parse ./Data/models.txt!\n"); return 1; }

    int maxIndex = -1;
    for (int k=0;k<model_parser.count;k++) {
        if (model_parser.entries[k].index > maxIndex && model_parser.entries[k].index != UINT16_MAX) maxIndex = model_parser.entries[k].index;
    }

    DualLog("Parsing %d models...\n",model_parser.count);
    int totalVertCount = 0;
    int totalBounds = 0;
    int totalTriCount = 0;
//     int totalEdgeCount = 0;
    largestVertCount = 0;
    largestTriangleCount = 0;
//     largestEdgeCount = 0;

    // Allocate persistent temporary buffers
    tempVertices = (float *)malloc(MAX_VERT_COUNT * VERTEX_ATTRIBUTES_COUNT * sizeof(float));
    DebugRAM("tempVertices buffer");

    tempTriangles = (uint32_t *)malloc(MAX_TRI_COUNT * 3 * sizeof(uint32_t));
    DebugRAM("tempTriangles buffer");

//     tempTriEdges = (uint32_t *)malloc(MAX_TRI_COUNT * 3 * sizeof(uint32_t));
//     DebugRAM("tempTriEdges buffer");

//     tempEdges = (Edge *)calloc(MAX_EDGE_COUNT, sizeof(Edge));
//     DebugRAM("tempEdges buffer");

//     edgeHash = (EdgeHashEntry *)calloc(HASH_SIZE, sizeof(EdgeHashEntry));
//     DebugRAM("edgeHash buffer");

    vertexDataArrays = (float **)calloc(MODEL_COUNT, sizeof(float *));
    DebugRAM("vertexDataArrays buffer");

    triangleDataArrays = (uint32_t **)calloc(MODEL_COUNT, sizeof(uint32_t *));
    DebugRAM("triangleDataArrays buffer");

//     triEdgeDataArrays = (uint32_t **)calloc(MODEL_COUNT, sizeof(uint32_t *));
//     DebugRAM("triEdgeDataArrays buffer");

//     edgeDataArrays = (uint32_t **)calloc(MODEL_COUNT, sizeof(uint32_t *));
//     DebugRAM("edgeDataArrays buffer");

    // Generate staging buffers
    GLuint stagingVBO, stagingTBO;//, stagingTEBO, stagingEBO;
    glGenBuffers(1, &stagingVBO);
    glGenBuffers(1, &stagingTBO);
//     glGenBuffers(1, &stagingTEBO);
//     glGenBuffers(1, &stagingEBO);
    glBindBuffer(GL_ARRAY_BUFFER, stagingVBO);
    glBufferData(GL_ARRAY_BUFFER, MAX_VERT_COUNT * VERTEX_ATTRIBUTES_COUNT * sizeof(float), NULL, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stagingTBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_TRI_COUNT * 3 * sizeof(uint32_t), NULL, GL_DYNAMIC_COPY);
//     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stagingTEBO);
//     glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_TRI_COUNT * 3 * sizeof(uint32_t), NULL, GL_DYNAMIC_COPY);
//     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stagingEBO);
//     glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_EDGE_COUNT * 4 * sizeof(uint32_t), NULL, GL_DYNAMIC_COPY);
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
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) { DualLogError("Assimp failed to load %s: %s\n", model_parser.entries[matchedParserIdx].path, aiGetErrorString()); return 1; }

        // Count vertices, triangles, and estimate edges
        uint32_t vertexCount = 0;
        uint32_t triCount = 0;
        for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
            vertexCount += scene->mMeshes[m]->mNumVertices;
            triCount += scene->mMeshes[m]->mNumFaces;
        }

        if (vertexCount > MAX_VERT_COUNT || triCount > MAX_TRI_COUNT || triCount * 3 > MAX_EDGE_COUNT) { DualLogError("Model %s exceeds buffer limits: verts=%u (> %u), tris=%u (> %u), edges=%u (> %u)\n", model_parser.entries[matchedParserIdx].path, vertexCount, MAX_VERT_COUNT, triCount, MAX_TRI_COUNT, triCount * 3, MAX_EDGE_COUNT); return 1; }

        modelVertexCounts[i] = vertexCount;
        modelTriangleCounts[i] = triCount;
//         uint32_t edgeCount = 0; // Will be updated after edge processing
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
//         memset(edgeHash, 0, HASH_SIZE * sizeof(EdgeHashEntry));

        // Extract vertex and triangle data, build edge list
        uint32_t vertexIndex = 0;
        float minx = 1E9f;
        float miny = 1E9f;
        float minz = 1E9f;
        float maxx = -1E9f;
        float maxy = -1E9f;
        float maxz = -1E9f;
        uint32_t triangleIndex = 0;
//         uint32_t triEdgeIndex = 0;
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
                if (face->mNumIndices != 3) { DualLogError("Non-triangular face detected in %s, face %u\n", model_parser.entries[matchedParserIdx].path, f); return 1; }

                uint32_t v[3] = {face->mIndices[0] + globalVertexOffset, face->mIndices[1] + globalVertexOffset, face->mIndices[2] + globalVertexOffset};
//                 uint32_t triangleIdx = triangleIndex / 3;
//                 uint32_t edgeIndices[3];

                // Validate vertex indices
                if (v[0] >= vertexCount || v[1] >= vertexCount || v[2] >= vertexCount) { DualLogError("Invalid vertex index in %s, face %u: v0=%u, v1=%u, v2=%u, vertexCount=%u\n", model_parser.entries[matchedParserIdx].path, f, v[0], v[1], v[2], vertexCount);return 1; }

                // Store vertex indices
                tempTriangles[triangleIndex++] = v[0];
                tempTriangles[triangleIndex++] = v[1];
                tempTriangles[triangleIndex++] = v[2];

                // Process edges with hash table
//                 for (int e = 0; e < 3; e++) {
//                     uint32_t v0 = v[e];
//                     uint32_t v1 = v[(e + 1) % 3];
//                     if (v0 > v1) { uint32_t temp = v0; v0 = v1; v1 = temp; }
// 
//                     uint32_t hash = HASH(v0, v1);
//                     while (edgeHash[hash].edgeIndex != 0 && (edgeHash[hash].v0 != v0 || edgeHash[hash].v1 != v1)) {
//                         hash = (hash + 1) & (HASH_SIZE - 1); // Linear probing
//                     }
// 
//                     int edgeFound = -1;
//                     if (edgeHash[hash].edgeIndex != 0) {
//                         edgeFound = edgeHash[hash].edgeIndex - 1;
//                         tempEdges[edgeFound].tri1 = triangleIdx;
//                     } else {
//                         if (edgeCount < MAX_EDGE_COUNT) {
//                             tempEdges[edgeCount].v0 = v0;
//                             tempEdges[edgeCount].v1 = v1;
//                             tempEdges[edgeCount].tri0 = triangleIdx;
//                             tempEdges[edgeCount].tri1 = UINT32_MAX;
//                             edgeHash[hash].v0 = v0;
//                             edgeHash[hash].v1 = v1;
//                             edgeHash[hash].edgeIndex = edgeCount + 1;
//                             edgeFound = edgeCount++;
//                         } else DualLogError("Edge count exceeds estimate for %s: %u >= %u\n", model_parser.entries[matchedParserIdx].path, edgeCount, MAX_EDGE_COUNT);
//                     }
// 
//                     edgeIndices[e] = edgeFound;
//                 }

                // Store edge indices
//                 tempTriEdges[triEdgeIndex++] = edgeIndices[0];
//                 tempTriEdges[triEdgeIndex++] = edgeIndices[1];
//                 tempTriEdges[triEdgeIndex++] = edgeIndices[2];
            }
            
            globalVertexOffset += mesh->mNumVertices;
        }

        DebugRAM("Prior to index %d allocations for model %s",i, model_parser.entries[matchedParserIdx].path);
        vertexDataArrays[i] = (float *)malloc(vertexCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float)); // Store vertex data in vertexDataArrays
        memcpy(vertexDataArrays[i], tempVertices, vertexCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float));
        DebugRAM("vertexDataArrays %d alloc for model %s",i, model_parser.entries[matchedParserIdx].path);

        triangleDataArrays[i] = (uint32_t *)malloc(triCount * 3 * sizeof(uint32_t)); // Store triangle data in triangleDataArrays
        memcpy(triangleDataArrays[i], tempTriangles, triCount * 3 * sizeof(uint32_t));
        DebugRAM("triangleDataArrays %d alloc for model %s",i, model_parser.entries[matchedParserIdx].path);

//         triEdgeDataArrays[i] = (uint32_t *)malloc(triCount * 3 * sizeof(uint32_t)); // Store triangle-edge data in triEdgeDataArrays
//         memcpy(triEdgeDataArrays[i], tempTriEdges, triCount * 3 * sizeof(uint32_t));
//         DebugRAM("triEdgeDataArrays %d alloc for model %s",i, model_parser.entries[matchedParserIdx].path);
// 
//         edgeDataArrays[i] = (uint32_t *)malloc(edgeCount * 4 * sizeof(uint32_t)); // Store edge data in edgeDataArrays
//         DebugRAM("edgeDataArrays %d alloc for model %s",i, model_parser.entries[matchedParserIdx].path);
//         for (uint32_t j = 0; j < edgeCount; j++) {
//             edgeDataArrays[i][j * 4 + 0] = tempEdges[j].v0;
//             edgeDataArrays[i][j * 4 + 1] = tempEdges[j].v1;
//             edgeDataArrays[i][j * 4 + 2] = tempEdges[j].tri0;
//             edgeDataArrays[i][j * 4 + 3] = tempEdges[j].tri1;
//         }
        
//         modelEdgeCounts[i] = edgeCount;
//         totalEdgeCount += edgeCount;
//         if (edgeCount > largestEdgeCount) largestEdgeCount = edgeCount;

        aiReleaseImport(scene);
        malloc_trim(0);
        DebugRAM("post aiReleaseImport for model %s", model_parser.entries[matchedParserIdx].path);

        // Copy to staging buffers
        if (vertexCount > 0) {
            glBindBuffer(GL_ARRAY_BUFFER, stagingVBO);
            void *mapped_buffer = glMapBufferRange(GL_ARRAY_BUFFER, 0, vertexCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
            memcpy(mapped_buffer, tempVertices, vertexCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float));
            glUnmapBuffer(GL_ARRAY_BUFFER);
            glGenBuffers(1, &vbos[i]);
            glBindBuffer(GL_COPY_WRITE_BUFFER, vbos[i]);
            glBufferData(GL_COPY_WRITE_BUFFER, vertexCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float), NULL, GL_STATIC_DRAW);
            glCopyBufferSubData(GL_ARRAY_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, vertexCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float));
            glFlush();
            glFinish();
            DebugRAM("post vertex buffer vbos upload for model %s", model_parser.entries[matchedParserIdx].path);
        }

        if (triCount > 0) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stagingTBO);
            void *mapped_buffer = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, triCount * 3 * sizeof(uint32_t), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
            memcpy(mapped_buffer, tempTriangles, triCount * 3 * sizeof(uint32_t));
            glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
            
            glGenBuffers(1, &tbos[i]);
            glBindBuffer(GL_COPY_WRITE_BUFFER, tbos[i]);
            glBufferData(GL_COPY_WRITE_BUFFER, triCount * 3 * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
            glCopyBufferSubData(GL_ELEMENT_ARRAY_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, triCount * 3 * sizeof(uint32_t));
            glFlush();
            glFinish();
            DebugRAM("post tri buffer tbos upload for model %s", model_parser.entries[matchedParserIdx].path);
            
/*            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stagingTEBO);
            mapped_buffer = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, triCount * 3 * sizeof(uint32_t), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
            memcpy(mapped_buffer, tempTriEdges, triCount * 3 * sizeof(uint32_t));
            glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
            
            glGenBuffers(1, &tebos[i]);
            glBindBuffer(GL_COPY_WRITE_BUFFER, tebos[i]);
            glBufferData(GL_COPY_WRITE_BUFFER, triCount * 3 * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
            glCopyBufferSubData(GL_ELEMENT_ARRAY_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, triCount * 3 * sizeof(uint32_t));
            glFlush();
            glFinish();
            DebugRAM("post tri buffer tebos upload for model %s", model_parser.entries[matchedParserIdx].path)*/;
        }

//         if (edgeCount > 0) {
//             uint32_t *tempEdgeData = (uint32_t *)malloc(edgeCount * 4 * sizeof(uint32_t));
//             for (uint32_t j = 0; j < edgeCount; j++) {
//                 tempEdgeData[j * 4 + 0] = tempEdges[j].v0;
//                 tempEdgeData[j * 4 + 1] = tempEdges[j].v1;
//                 tempEdgeData[j * 4 + 2] = tempEdges[j].tri0;
//                 tempEdgeData[j * 4 + 3] = tempEdges[j].tri1;
//             }
// 
//             glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stagingEBO);
//             void *mapped_buffer = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, edgeCount * 4 * sizeof(uint32_t), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
//             memcpy(mapped_buffer, tempEdgeData, edgeCount * 4 * sizeof(uint32_t));
//             glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
//             glGenBuffers(1, &ebos[i]);
//             glBindBuffer(GL_COPY_WRITE_BUFFER, ebos[i]);
//             glBufferData(GL_COPY_WRITE_BUFFER, edgeCount * 4 * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
//             glCopyBufferSubData(GL_ELEMENT_ARRAY_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, edgeCount * 4 * sizeof(uint32_t));
//             glFlush();
//             glFinish();
//             free(tempEdgeData);
//             DebugRAM("post edge data free for model %s", model_parser.entries[matchedParserIdx].path);
//         }

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
//     glDeleteBuffers(1, &stagingTEBO);
//     glDeleteBuffers(1, &stagingEBO);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    DebugRAM("after staging buffers deleted");

#ifdef DEBUG_MODEL_LOAD_DATA
    DualLog("Largest vertex count: %d, triangle count: %d\n", largestVertCount, largestTriangleCount);
//     DualLog("Largest vertex count: %d, triangle count: %d, edge count: %d\n", largestVertCount, largestTriangleCount, largestEdgeCount);
    DualLog("Total vertices: %d (", totalVertCount);
    print_bytes_no_newline(totalVertCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float));
    DualLog(")\nTotal triangles: %d (", totalTriCount);
    print_bytes_no_newline(totalTriCount * 3 * sizeof(uint32_t));
//     DualLog(")\nTotal tri-edges: %d (", totalTriCount);
//     print_bytes_no_newline(totalTriCount * 3 * sizeof(uint32_t));
//     DualLog(")\nTotal edges: %d (", totalEdgeCount);
//     print_bytes_no_newline(totalEdgeCount * 4 * sizeof(uint32_t));
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
    free(tempVertices);
    free(tempTriangles);
//     free(tempTriEdges);
//     free(tempEdges);
//     free(edgeHash);
    malloc_trim(0);
    double end_time = get_time();
    DualLog("Load Models took %f seconds\n", end_time - start_time);
    DebugRAM("After full LoadModels completed");
    return 0;
}

//--------------------------------- Entities -------------------------------------
Entity entities[MAX_ENTITIES]; // Global array of entity definitions
int entityCount = 0;            // Number of entities loaded
DataParser entity_parser;
const char *valid_entity_keys[] = {"index", "model", "texture", "glowtexture", "spectexture", "normtexture", "cardchunk", "lod"};
#define NUM_ENTITY_KEYS 8

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
    parser_init(&entity_parser, valid_entity_keys, NUM_ENTITY_KEYS);
    if (!parse_data_file(&entity_parser, "./Data/entities.txt",0)) { DualLogError("Could not parse ./Data/entities.txt!\n"); return 1; }
    
    loadEntityItemInitialized[ENT_PARSER] = true;
    entityCount = entity_parser.count;
    if (entityCount > MAX_ENTITIES) { DualLogError("Too many entities in parser count %d, greater than %d!\n", entityCount, MAX_ENTITIES); return 1; }
    if (entityCount == 0) { DualLogError("No entities found in entities.txt\n"); return 1; }

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
        entities[i].lodIndex = entity_parser.entries[i].cardchunk ? 178: entity_parser.entries[i].lodIndex; // Generic LOD card
    }

    DebugRAM("after loading all entities");
    double end_time = get_time();
    DualLog("Load Entities took %f seconds\n", end_time - start_time);
    return 0;
}
#pragma GCC diagnostic pop // Ok restore string truncation warning

// TODO: If game name == Citadel
void GetLevel_Transform_Offsets(int curlevel, float* ofsx, float* ofsy, float* ofsz) {
    switch(curlevel) { // Match the parent transforms #.NAMELevel, e.g. 1.MedicalLevel
        case 0:  *ofsx = 3.6f; *ofsy = -4.10195f; *ofsz = 1.0f; break;
        case 1:  *ofsx = -5.12f; *ofsy = -48.64f; *ofsz = -5.2f; break;
//         case 1:  *ofsx = 25.56f; *ofsy = -48.64f; *ofsz = -5.2f; break;
        case 2:  *ofsx = -2.6f; *ofsy = 0.0f; *ofsz = -7.7f; break;
        case 3:  *ofsx = -45.12f; *ofsy = -0.700374f; *ofsz = -16.32f; break;
        case 4:  *ofsx = -20.4f; *ofsy = 0.0f; *ofsz = 11.48f; break;
        case 5:  *ofsx = -10.14f; *ofsy = 0.065f; *ofsz = -0.0383f; break;
        case 6:  *ofsx = -0.6728f; *ofsy = 0.1725f; *ofsz = 3.76f; break;
        case 7: *ofsx = -6.7f; *ofsy = 0.24443f; *ofsz = 1.16f; break;
        case 8:  *ofsx = 1.08f; *ofsy = -0.935f; *ofsz = 0.8f; break;
        case 9:  *ofsx = 3.6f; *ofsy = 0.0f; *ofsz = -1.28f; break;
        case 10: *ofsx = 107.37f; *ofsy = 101.2f; *ofsz = 35.48f; break;
        case 11: *ofsx = 15.05f; *ofsy = 129.9f; *ofsz = -77.94f; break;
        case 12:  *ofsx = 19.04f; *ofsy = 162.2f; *ofsz = 95.8f; break;
        case 13: *ofsx = 164.7f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

void GetLevel_Geometry_Offsets(int curlevel, float* ofsx, float* ofsy, float* ofsz) {
    switch(curlevel) { // Match the parent transforms #.NAMELevel, e.g. 1.Geometry
        case 0:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 1:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 2:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 3:  *ofsx = 50.174f; *ofsy = 0.78982f; *ofsz = 13.714f; break;
        case 4:  *ofsx = 1.178f; *ofsy = 1.28f; *ofsz = 1.292799f; break;
        case 5:  *ofsx = 1.1778f; *ofsy = -0.065f; *ofsz = -1.2417f; break;
        case 6:  *ofsx = 1.2928f; *ofsy = -0.1725f; *ofsz = -1.2033f; break;
        case 7:  *ofsx = 1.2411f; *ofsy = -0.24443f; *ofsz = -1.2544f; break;
        case 8:  *ofsx = -1.3056f; *ofsy = 0.935f; *ofsz = 1.2928f; break;
        case 9:  *ofsx = -1.3439f; *ofsy = -0.54305f; *ofsz = -1.1906f; break;
        case 10: *ofsx = -0.90945f; *ofsy = 1.7156f; *ofsz = -1.0372f; break;
        case 11: *ofsx = -1.2672f; *ofsy = 0.46112f; *ofsz = 0.96056f; break;
        case 12: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 13: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

void GetLevel_DynamicObjectsSaveableInstantiated_ContainerOffsets(int curlevel, float* ofsx, float* ofsy, float* ofsz) {
    switch(curlevel) { // Match the parent transforms #.NAMELevel, e.g. 1.DynamicObjectsSaveableInstantiated
        case 0:  *ofsx = -1.2417f; *ofsy = -0.26194f; *ofsz = -1.0883f; break;
        case 1:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 2:  *ofsx = -0.98611f; *ofsy = 0.84f; *ofsz = 1.1906f; break;
        case 3:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 4:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 5:  *ofsx = 0.0f; *ofsy = 0.07f; *ofsz = 0.0f; break;
        case 6:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 7:  *ofsx = 0.0f; *ofsy = 0.04f; *ofsz = 0.0f; break;
        case 8:  *ofsx = 0.0f; *ofsy = 0.16f; *ofsz = 0.0f; break;
        case 9:  *ofsx = 0.0f; *ofsy = 0.08f; *ofsz = 0.0f; break;
        case 10: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 11: *ofsx = 0.0f; *ofsy = 0.32f; *ofsz = 0.0f; break;
        case 12: *ofsx = 0.0f; *ofsy = 0.2f; *ofsz = 0.0f; break;
        case 13: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

void GetLevel_LightsStaticSaveable_ContainerOffsets(int curlevel, float* ofsx, float* ofsy, float* ofsz) {
    switch(curlevel) { // Match the parent transforms #.NAMELevel, e.g. 1.LightsStaticSaveable
        case 0:  *ofsx = -1.2417f; *ofsy = -0.26194f; *ofsz = -1.0883f; break;
        case 1:  *ofsx = 0.589f; *ofsy = -0.554f; *ofsz = -0.907f; break;
        case 2:  *ofsx = -0.98611f; *ofsy = 0.82105f; *ofsz = 1.1906f; break;
        case 3:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 4:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 5:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 6:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 7:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 8:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 9:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 10: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 11: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 12: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 13: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

void GetLevel_LightsStaticImmutable_ContainerOffsets(int curlevel, float* ofsx, float* ofsy, float* ofsz) {
    switch(curlevel) { // Match the parent transforms #.NAMELevel, e.g. 1.LightsStaticImmutable
        case 0:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 1:  *ofsx = -5.12f; *ofsy = -48.37571f; *ofsz = -15.391001f; break;
//         case 1:  *ofsx = 25.56008f; *ofsy = -48.64f; *ofsz = -5.2f; break; // Position offset once unparented from 1.MedicalLevel
//         case 1:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 2:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 3:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 4:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 5:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 6:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 7:  *ofsx = -14.528f; *ofsy = 48.269f; *ofsz = -26.836f; break;
        case 8:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 9:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 10: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 11: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 12: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 13: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

void GetLevel_DoorsStaticSaveable_ContainerOffsets(int curlevel, float* ofsx, float* ofsy, float* ofsz) {
    switch(curlevel) { // Match the parent transforms #.NAMELevel, e.g. 1.DoorsStaticSaveable
        case 0:  *ofsx = -1.2417f; *ofsy = -0.26194f; *ofsz = -1.0883f; break;
        case 1:  *ofsx = 0.589f; *ofsy = -0.554f; *ofsz = -0.907f; break;
        case 2:  *ofsx = -0.98611f; *ofsy = 0.82105f; *ofsz = 1.1906f; break;
        case 3:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 4:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 5:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 6:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 7:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 8:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 9:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 10: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 11: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 12: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 13: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

void GetLevel_StaticObjectsSaveable_ContainerOffsets(int curlevel, float* ofsx, float* ofsy, float* ofsz) {
    switch(curlevel) { // Match the parent transforms #.NAMELevel, e.g. 1.StaticObjectsSaveable
        case 0:  *ofsx = -1.2417f; *ofsy = -0.26194f; *ofsz = -1.0883f; break;
        case 1:  *ofsx = 0.589f; *ofsy = -0.554f; *ofsz = -0.907f; break;
        case 2:  *ofsx = -0.98611f; *ofsy = 0.82105f; *ofsz = 1.1906f; break;
        case 3:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 4:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 5:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 6:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 7:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 8:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 9:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 10: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 11: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 12: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 13: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

void GetLevel_StaticObjectsImmutable_ContainerOffsets(int curlevel, float* ofsx, float* ofsy, float* ofsz) {
    switch(curlevel) { // Match the parent transforms #.NAMELevel, e.g. 1.StaticObjectsImmutable
        case 0:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 1:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 2:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 3:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 4:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 5:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 6:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 7:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 8:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 9:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 10: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 11: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 12: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 13: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

void GetLevel_NPCsSaveableInstantiated_ContainerOffsets(int curlevel, float* ofsx, float* ofsy, float* ofsz) {
    switch(curlevel) { // Match the parent transforms #.NAMELevel, e.g. 1.NPCsSaveableInstantiated
        case 0:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 1:  *ofsx = -33.28f; *ofsy = 48.64f; *ofsz = 7.679996f; break;
        case 2:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 3:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 4:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 5:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 6:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 7:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 8:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 9:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 10: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 11: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 12: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 13: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

static inline float quat_dot(Quaternion a, Quaternion b) {
    return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
}

float quat_angle_deg(Quaternion a, Quaternion b) {
    float d = fabsf(quat_dot(a, b));
    if (d > 1.0f) d = 1.0f;
    return acosf(d) * 2.0f * (180.0f / (float)M_PI);
}

//----------------------------------- Level -----------------------------------
int LoadLevelGeometry(uint8_t curlevel) {
    if (curlevel >= numLevels) { DualLogError("Cannot load level %d, out of bounds 0 to %d\n",curlevel,numLevels - 1); return 1; }

    DebugRAM("start of LoadLevelGeometry");
    char filename[64];
    snprintf(filename, sizeof(filename), "./Data/CitadelScene_geometry_level%d.txt", curlevel);
    parser_init(&level_parser, valid_leveldata_keys, NUM_LEVDAT_KEYS);
    if (!parse_data_file(&level_parser, filename,1)) { DualLogError("Could not parse %s!\n",filename); return 1; }

    int gameObjectCount = level_parser.count;
    DualLog("Loading %d objects for Level %d...\n",gameObjectCount,curlevel);
    float correctionX, correctionY, correctionZ;
    float correctionGeoX, correctionGeoY, correctionGeoZ;
    GetLevel_Transform_Offsets(curlevel,&correctionX,&correctionY,&correctionZ);
    GetLevel_Geometry_Offsets(curlevel,&correctionGeoX,&correctionGeoY,&correctionGeoZ);
    correctionX += correctionGeoX; correctionY += correctionGeoY; correctionZ += correctionGeoZ;
    for (int idx=0;idx<gameObjectCount;++idx) {
        int entIdx = level_parser.entries[idx].constIndex;
        instances[idx].modelIndex = entities[entIdx].modelIndex;
        instances[idx].texIndex = entities[entIdx].texIndex;
        instances[idx].glowIndex = entities[entIdx].glowIndex;
        instances[idx].specIndex = entities[entIdx].specIndex;
        instances[idx].normIndex = entities[entIdx].normIndex;
        instances[idx].lodIndex = entities[entIdx].lodIndex;
        instances[idx].posx = level_parser.entries[idx].localPosition.x + -5.12f;// + correctionX;
        instances[idx].posy = level_parser.entries[idx].localPosition.y + -48.64f;// + correctionY;
        instances[idx].posz = level_parser.entries[idx].localPosition.z + -15.36f;// + correctionZ;
        instances[idx].rotx = level_parser.entries[idx].localRotation.x;
        instances[idx].roty = level_parser.entries[idx].localRotation.y;
        instances[idx].rotz = level_parser.entries[idx].localRotation.z;
        instances[idx].rotw = level_parser.entries[idx].localRotation.w;
        instances[idx].sclx = level_parser.entries[idx].localScale.x;
        instances[idx].scly = level_parser.entries[idx].localScale.y;
        instances[idx].sclz = level_parser.entries[idx].localScale.z;
        Quaternion quat = {instances[idx].rotx, instances[idx].roty, instances[idx].rotz, instances[idx].rotw};
        Quaternion upQuat = {1.0f, 0.0f, 0.0f, 0.0f};
        float angle = quat_angle_deg(quat,upQuat); // Get angle in degrees relative to up vector
        bool pointsUp = angle <= 30.0f;
        instances[idx].floorHeight = pointsUp && currentLevel <= 12 ? 0.0f : INVALID_FLOOR_HEIGHT; // TODO: Citadel specific max floor height caring level threshold of 12
//         if (pointsUp) DualLog("Found floor named %s from quat x %f, y %f, z %f, w %f\n",level_parser.entries[idx].path,quat.x,quat.y,quat.z,quat.w);
    }

    malloc_trim(0);
    DebugRAM("end of LoadLevelGeometry");
    return 0;
}

int LoadLevelLights(uint8_t curlevel) {
    if (curlevel >= numLevels) { DualLogError("Cannot load level lights %d, out of bounds 0 to %d\n",curlevel,numLevels - 1); return 1; }

    DebugRAM("start of LoadLevelLights");
    char filename[64];
    snprintf(filename, sizeof(filename), "./Data/CitadelScene_lights_level%d.txt", curlevel);
    parser_init(&lights_parser, valid_lightdata_keys, NUM_LIGHTDAT_KEYS);
    if (!parse_data_file(&lights_parser, filename,1)) { DualLogError("Could not parse %s!\n",filename); return 1; }

    int lightsCount = lights_parser.count;
    DualLog("Loading %d lights for Level %d...\n",lightsCount,curlevel);
    float correctionX = 0.0f, correctionY = 0.0f, correctionZ = 0.0f;
    float correctionLightX, correctionLightY, correctionLightZ;
//     GetLevel_Transform_Offsets(curlevel,&correctionX,&correctionY,&correctionZ);
    GetLevel_LightsStaticImmutable_ContainerOffsets(curlevel,&correctionLightX,&correctionLightY,&correctionLightZ);
    correctionX += correctionLightX; correctionY += correctionLightY; correctionZ += correctionLightZ;
    for (int i=0;i<lightsCount;++i) {
        uint16_t idx = (i * LIGHT_DATA_SIZE);
        lights[idx + LIGHT_DATA_OFFSET_POSX] = lights_parser.entries[i].localPosition.x + correctionX;
        lights[idx + LIGHT_DATA_OFFSET_POSY] = lights_parser.entries[i].localPosition.y + correctionY;
        lights[idx + LIGHT_DATA_OFFSET_POSZ] = lights_parser.entries[i].localPosition.z + correctionZ;
        lights[idx + LIGHT_DATA_OFFSET_INTENSITY] = lights_parser.entries[i].intensity;
        lights[idx + LIGHT_DATA_OFFSET_RANGE] = lights_parser.entries[i].range;
        lightsRangeSquared[i] = lights_parser.entries[i].range * lights_parser.entries[i].range;
        lights[idx + LIGHT_DATA_OFFSET_SPOTANG] = 0.0f;//lights_parser.entries[i].type == 1 ? 0.0f : lights_parser.entries[i].spotAngle; // If spot apply it, else get 0 for spotAng
        lights[idx + LIGHT_DATA_OFFSET_SPOTDIRX] = lights_parser.entries[i].localRotation.x;
        lights[idx + LIGHT_DATA_OFFSET_SPOTDIRY] = lights_parser.entries[i].localRotation.y;
        lights[idx + LIGHT_DATA_OFFSET_SPOTDIRZ] = lights_parser.entries[i].localRotation.z;
        lights[idx + LIGHT_DATA_OFFSET_SPOTDIRW] = lights_parser.entries[i].localRotation.w;
        lights[idx + LIGHT_DATA_OFFSET_R] = lights_parser.entries[i].color.r;
        lights[idx + LIGHT_DATA_OFFSET_G] = lights_parser.entries[i].color.g;
        lights[idx + LIGHT_DATA_OFFSET_B] = lights_parser.entries[i].color.b;
    }

    malloc_trim(0);
    DebugRAM("end of LoadLevelLights");
    return 0;
}
