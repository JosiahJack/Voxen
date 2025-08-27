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
#define MAX_VERT_COUNT 21444
#define MAX_TRI_COUNT 32449

uint32_t modelVertexCounts[MODEL_COUNT];
uint32_t modelTriangleCounts[MODEL_COUNT];
uint32_t modelLuxelCounts[MODEL_COUNT];
GLuint vbos[MODEL_COUNT];
GLuint tbos[MODEL_COUNT]; // Triangle index buffers
GLuint modelBoundsID;
float modelBounds[MODEL_COUNT * BOUNDS_ATTRIBUTES_COUNT];
uint32_t largestVertCount = 0;
uint32_t largestTriangleCount = 0;
GLuint vboMasterTable;
GLuint tboMasterTable;
GLuint modelVertexOffsetsID;
GLuint modelVertexCountsID;
GLuint modelTriangleCountsID;
float * tempVertices;
uint32_t * tempTriangles;
float ** vertexDataArrays;
uint32_t ** triangleDataArrays;
uint32_t ** triEdgeDataArrays;
uint32_t totalLuxelCount = 0;
int gameObjectCount = 0;

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
    entry->modname[0] = '\0';
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
            else if (strcmp(trimmed_key, "modname") == 0)         { strncpy(entry->modname, trimmed_value, sizeof(entry->modname) - 1); entry->modname[sizeof(entry->modname) - 1] = '\0'; entry->index = 0; } // Game/Mod Definition enforces setting entry index to 0 here, at least one of these must do it.  The game definition only has one index, 0.
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
    if (!read_token(file, token, sizeof(token), '\n', &is_comment, is_eof, &is_newline, lineNum)) return false;
    
    process_key_value(parser, entry, key, token, key, *lineNum);
    *lineNum += 1;
    return true;
}

static bool ParseResourceData(DataParser *parser, const char *filename) {
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
                if (current_index < parser->count) parser->entries[current_index++] = entry;
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
            if (current_index < parser->count) parser->entries[current_index++] = entry;
            init_data_entry(&entry);
            lineNum++;
        }
    }

    // Store last entry
    if (entry.path[0] && current_index < parser->count) { parser->entries[current_index] = entry; current_index++; }
    fclose(file);
    return true;
}

bool parse_data_file(DataParser *parser, const char *filename, int type) {
    if (type == 1) return ParseSaveLevelData(parser, filename);
    else return ParseResourceData(parser, filename);
}

//-----------------------------------------------------------------------------
// Loads all 3D meshes

typedef struct { float x, y, z; } Vec3;
static inline Vec3 vec3_sub(Vec3 a, Vec3 b) { return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z}; }
static inline float vec3_length(Vec3 v) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z); }
static inline Vec3 vec3_normalize(Vec3 v) { float len = vec3_length(v); return (Vec3){v.x / len, v.y / len, v.z / len}; }
static inline Vec3 vec3_cross(Vec3 a, Vec3 b) { return (Vec3){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }
static inline float vec3_dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

//-----------------------------------------------------------------------------
// Generate lightmap UVs and luxel counts for a model
void GenerateLightmapUVs(float *vertices, uint32_t *triangles, uint32_t triCount, uint32_t *outLuxelCount, uint32_t *outTriLuxelOffsets, uint32_t *outTriLuxelDims, float *modelBounds, uint32_t modelIndex) {
    float max_dim = fmaxf(modelBounds[modelIndex * BOUNDS_ATTRIBUTES_COUNT + BOUNDS_DATA_OFFSET_MAXX] - modelBounds[modelIndex * BOUNDS_ATTRIBUTES_COUNT + BOUNDS_DATA_OFFSET_MINX],
                          fmaxf(modelBounds[modelIndex * BOUNDS_ATTRIBUTES_COUNT + BOUNDS_DATA_OFFSET_MAXY] - modelBounds[modelIndex * BOUNDS_ATTRIBUTES_COUNT + BOUNDS_DATA_OFFSET_MINY],
                                modelBounds[modelIndex * BOUNDS_ATTRIBUTES_COUNT + BOUNDS_DATA_OFFSET_MAXZ] - modelBounds[modelIndex * BOUNDS_ATTRIBUTES_COUNT + BOUNDS_DATA_OFFSET_MINZ]));
    float luxel_size = max_dim / 16.0f; // Model-specific luxel size
    if (luxel_size < 0.16f) luxel_size = 0.16f; // Minimum luxel size
    uint32_t totalLuxels = 0;
    for (uint32_t f = 0; f < triCount; f++) {
        // Get vertex indices and positions
        uint32_t idx0 = triangles[f * 3 + 0] * VERTEX_ATTRIBUTES_COUNT;
        uint32_t idx1 = triangles[f * 3 + 1] * VERTEX_ATTRIBUTES_COUNT;
        uint32_t idx2 = triangles[f * 3 + 2] * VERTEX_ATTRIBUTES_COUNT;
        Vec3 v0 = {vertices[idx0], vertices[idx0 + 1], vertices[idx0 + 2]};
        Vec3 v1 = {vertices[idx1], vertices[idx1 + 1], vertices[idx1 + 2]};
        Vec3 v2 = {vertices[idx2], vertices[idx2 + 1], vertices[idx2 + 2]};

        // Find longest edge (U-axis)
        float len01 = vec3_length(vec3_sub(v1, v0));
        float len12 = vec3_length(vec3_sub(v2, v1));
        float len20 = vec3_length(vec3_sub(v0, v2));
        Vec3 u_axis, v0_edge;
        if (len01 >= len12 && len01 >= len20) {
            u_axis = vec3_normalize(vec3_sub(v1, v0));
            v0_edge = v0;
        } else if (len12 >= len01 && len12 >= len20) {
            u_axis = vec3_normalize(vec3_sub(v2, v1));
            v0_edge = v1;
        } else {
            u_axis = vec3_normalize(vec3_sub(v0, v2));
            v0_edge = v2;
        }

        // Compute centroid and V-axis
        Vec3 centroid = { (v0.x + v1.x + v2.x) / 3.0f, (v0.y + v1.y + v2.y) / 3.0f, (v0.z + v1.z + v2.z) / 3.0f };
        Vec3 normal = vec3_normalize(vec3_cross(vec3_sub(v1, v0), vec3_sub(v2, v0)));
        Vec3 v_axis = vec3_normalize(vec3_cross(normal, u_axis));

        // Project vertices to 2D UV space
        float u0 = vec3_dot(vec3_sub(v0, v0_edge), u_axis);
        float u1 = vec3_dot(vec3_sub(v1, v0_edge), u_axis);
        float u2 = vec3_dot(vec3_sub(v2, v0_edge), u_axis);
        float v0_coord = vec3_dot(vec3_sub(v0, centroid), v_axis);
        float v1_coord = vec3_dot(vec3_sub(v1, centroid), v_axis);
        float v2_coord = vec3_dot(vec3_sub(v2, centroid), v_axis);

        // Compute 2D bounds
        float min_u = fminf(u0, fminf(u1, u2));
        float max_u = fmaxf(u0, fmaxf(u1, u2));
        float min_v = fminf(v0_coord, fminf(v1_coord, v2_coord));
        float max_v = fmaxf(v0_coord, fmaxf(v1_coord, v2_coord));

        // Compute luxel count, cap at 64x64
        float u_range = max_u - min_u < 0.001f ? 0.001f : max_u - min_u;
        float v_range = max_v - min_v < 0.001f ? 0.001f : max_v - min_v;
        uint32_t luxel_u = (uint32_t)ceilf(u_range / luxel_size);
        uint32_t luxel_v = (uint32_t)ceilf(v_range / luxel_size);
        luxel_u = luxel_u > 64 ? 64 : (luxel_u < 1 ? 1 : luxel_u);
        luxel_v = luxel_v > 64 ? 64 : (luxel_v < 1 ? 1 : luxel_v);
        uint32_t luxel_count = luxel_u * luxel_v;
        if (luxel_count > 4096) { DualLog("Warning: Triangle %u has %u luxels (u=%u, v=%u)\n", f, luxel_count, luxel_u, luxel_v); }

        // Store triangle luxel offset and dimensions
        outTriLuxelOffsets[f] = totalLuxels;
        outTriLuxelDims[f * 2 + 0] = luxel_u;
        outTriLuxelDims[f * 2 + 1] = luxel_v;
        totalLuxels += luxel_count;

        // Normalize UVs to [0,1] for the triangle’s luxel grid
        float u_scale = u_range > 0.0f ? 1.0f / u_range : 1.0f;
        float v_scale = v_range > 0.0f ? 1.0f / v_range : 1.0f;
        vertices[idx0 + 13] = (u0 - min_u) * u_scale; // u_lm
        vertices[idx0 + 14] = (v0_coord - min_v) * v_scale; // v_lm
        vertices[idx1 + 13] = (u1 - min_u) * u_scale;
        vertices[idx1 + 14] = (v1_coord - min_v) * v_scale;
        vertices[idx2 + 13] = (u2 - min_u) * u_scale;
        vertices[idx2 + 14] = (v2_coord - min_v) * v_scale;
    }
    
//     DualLog("Model %u: max_dim=%.2f, luxel_size=%.2f, totalLuxels: %d\n", modelIndex, max_dim, luxel_size, totalLuxels);
    *outLuxelCount = totalLuxels;
}

int LoadGeometry(void) {
    double start_time = get_time();
    parser_init(&model_parser, valid_mdldata_keys, NUM_MODEL_KEYS); // First parse ./Data/models.txt to see what to load to what indices
    if (!parse_data_file(&model_parser, "./Data/models.txt",0)) { DualLogError("Could not parse ./Data/models.txt!\n"); return 1; }

    int maxIndex = -1;
    for (int k = 0; k < model_parser.count; ++k) {
        if (model_parser.entries[k].index > maxIndex && model_parser.entries[k].index != UINT16_MAX) maxIndex = model_parser.entries[k].index;
    }

    DualLog("Parsing %d models...\n", model_parser.count);
    int totalVertCount = 0;
    int totalBounds = 0;
    int totalTriCount = 0;
    largestVertCount = 0;
    largestTriangleCount = 0;
    tempVertices = (float *)malloc(MAX_VERT_COUNT * VERTEX_ATTRIBUTES_COUNT * sizeof(float));
    tempTriangles = (uint32_t *)malloc(MAX_TRI_COUNT * 3 * sizeof(uint32_t));
    vertexDataArrays = (float **)calloc(MODEL_COUNT, sizeof(float *));
    triangleDataArrays = (uint32_t **)calloc(MODEL_COUNT, sizeof(uint32_t *));
    uint32_t **triLuxelOffsets = (uint32_t **)calloc(MODEL_COUNT, sizeof(uint32_t *));
    uint32_t **triLuxelDims = (uint32_t **)calloc(MODEL_COUNT, sizeof(uint32_t *));
    uint32_t *luxelOffsets = (uint32_t *)calloc(MODEL_COUNT, sizeof(uint32_t));
    totalLuxelCount = 0;

    // Generate staging buffers
    GLuint stagingVBO, stagingTBO;
    glGenBuffers(1, &stagingVBO);
    glGenBuffers(1, &stagingTBO);
    glBindBuffer(GL_ARRAY_BUFFER, stagingVBO);
    glBufferData(GL_ARRAY_BUFFER, MAX_VERT_COUNT * VERTEX_ATTRIBUTES_COUNT * sizeof(float), NULL, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stagingTBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_TRI_COUNT * 3 * sizeof(uint32_t), NULL, GL_DYNAMIC_COPY);
    for (uint32_t i = 0; i < MODEL_COUNT; ++i) {
        int matchedParserIdx = -1;
        for (int k = 0; k < model_parser.count; ++k) {
            if (model_parser.entries[k].index == i) { matchedParserIdx = k; break; }
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

        // Count vertices, triangles
        uint32_t vertexCount = 0;
        uint32_t triCount = 0;
        for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
            vertexCount += scene->mMeshes[m]->mNumVertices;
            triCount += scene->mMeshes[m]->mNumFaces;
        }

        if (vertexCount > MAX_VERT_COUNT || triCount > MAX_TRI_COUNT) { DualLogError("Model %s exceeds buffer limits: verts=%u (> %u), tris=%u (> %u)\n", model_parser.entries[matchedParserIdx].path, vertexCount, MAX_VERT_COUNT, triCount, MAX_TRI_COUNT); return 1; }

        modelVertexCounts[i] = vertexCount;
        modelTriangleCounts[i] = triCount;
        if (vertexCount > largestVertCount) largestVertCount = vertexCount;
        if (triCount > largestTriangleCount) largestTriangleCount = triCount;
        totalVertCount += vertexCount;
        totalTriCount += triCount;

        // Extract vertex and triangle data
        uint32_t vertexIndex = 0;
        float minx = 1E9f;
        float miny = 1E9f;
        float minz = 1E9f;
        float maxx = -1E9f;
        float maxy = -1E9f;
        float maxz = -1E9f;
        uint32_t triangleIndex = 0;
        uint32_t globalVertexOffset = 0;

        for (unsigned int m = 0; m < scene->mNumMeshes; ++m) {
            struct aiMesh *mesh = scene->mMeshes[m];
            for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
                tempVertices[vertexIndex++] = mesh->mVertices[v].x;
                tempVertices[vertexIndex++] = mesh->mVertices[v].y;
                tempVertices[vertexIndex++] = mesh->mVertices[v].z;
                tempVertices[vertexIndex++] = mesh->mNormals[v].x;
                tempVertices[vertexIndex++] = mesh->mNormals[v].y;
                tempVertices[vertexIndex++] = mesh->mNormals[v].z;
                float tempU = mesh->mTextureCoords[0] ? mesh->mTextureCoords[0][v].x : 0.0f;
                float tempV = mesh->mTextureCoords[0] ? mesh->mTextureCoords[0][v].y : 0.0f;
                tempVertices[vertexIndex++] = tempU;
                tempVertices[vertexIndex++] = tempV;
                tempVertices[vertexIndex++] = 0;
                tempVertices[vertexIndex++] = 0;
                tempVertices[vertexIndex++] = 0;
                tempVertices[vertexIndex++] = 0;
                tempVertices[vertexIndex++] = 0;
                tempVertices[vertexIndex++] = 0;
                tempVertices[vertexIndex++] = 0.0f;
                tempVertices[vertexIndex++] = 0.0f;
                if (mesh->mVertices[v].x < minx) minx = mesh->mVertices[v].x;
                if (mesh->mVertices[v].x > maxx) maxx = mesh->mVertices[v].x;
                if (mesh->mVertices[v].y < miny) miny = mesh->mVertices[v].y;
                if (mesh->mVertices[v].y > maxy) maxy = mesh->mVertices[v].y;
                if (mesh->mVertices[v].z < minz) minz = mesh->mVertices[v].z;
                if (mesh->mVertices[v].z > maxz) maxz = mesh->mVertices[v].z;
            }

            for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
                struct aiFace *face = &mesh->mFaces[f];
                if (face->mNumIndices != 3) { DualLogError("Non-triangular face detected in %s, face %u\n", model_parser.entries[matchedParserIdx].path, f); return 1; }
                uint32_t v[3] = {face->mIndices[0] + globalVertexOffset, face->mIndices[1] + globalVertexOffset, face->mIndices[2] + globalVertexOffset};
                if (v[0] >= vertexCount || v[1] >= vertexCount || v[2] >= vertexCount) { DualLogError("Invalid vertex index in %s, face %u: v0=%u, v1=%u, v2=%u, vertexCount=%u\n", model_parser.entries[matchedParserIdx].path, f, v[0], v[1], v[2], vertexCount); return 1; }
                tempTriangles[triangleIndex++] = v[0];
                tempTriangles[triangleIndex++] = v[1];
                tempTriangles[triangleIndex++] = v[2];
            }
            globalVertexOffset += mesh->mNumVertices;
        }

        // Generate lightmap UVs and luxel counts
        triLuxelOffsets[i] = (uint32_t *)calloc(triCount, sizeof(uint32_t));
        triLuxelDims[i] = (uint32_t *)calloc(triCount * 2, sizeof(uint32_t));
        GenerateLightmapUVs(tempVertices, tempTriangles, triCount, &modelLuxelCounts[i], triLuxelOffsets[i], triLuxelDims[i], modelBounds, i);
        luxelOffsets[i] = totalLuxelCount;
        totalLuxelCount += modelLuxelCounts[i];
        vertexDataArrays[i] = (float *)malloc(vertexCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float));
        memcpy(vertexDataArrays[i], tempVertices, vertexCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float));

        triangleDataArrays[i] = (uint32_t *)malloc(triCount * 3 * sizeof(uint32_t));
        memcpy(triangleDataArrays[i], tempTriangles, triCount * 3 * sizeof(uint32_t));

        aiReleaseImport(scene);
        malloc_trim(0);

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
    }

    // Delete staging buffers
    glDeleteBuffers(1, &stagingVBO);
    glDeleteBuffers(1, &stagingTBO);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    // Log total luxel count for debugging
    DualLog("Total luxels: %llu\n", totalLuxelCount);

    // Create lightmap SSBO (RGBA floats for HDR)
    float *luxelData = (float *)calloc(totalLuxelCount * 4, sizeof(float));
    for (uint64_t j = 0; j < totalLuxelCount; ++j) {
        luxelData[j * 4 + 0] = 1.0f;
        luxelData[j * 4 + 1] = 1.0f;
        luxelData[j * 4 + 2] = 1.0f;
        luxelData[j * 4 + 3] = 1.0f;
    }
    GLuint lightmapSSBO;
    glGenBuffers(1, &lightmapSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightmapSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, totalLuxelCount * 4 * sizeof(float), luxelData, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 21, lightmapSSBO);
    free(luxelData);

    // Create model offset SSBO
    GLuint offsetSSBO;
    glGenBuffers(1, &offsetSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, offsetSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, MODEL_COUNT * sizeof(uint32_t), luxelOffsets, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 22, offsetSSBO);

    // Create triangle luxel metadata SSBO
    uint64_t totalTriCountLM = 0;
    for (uint32_t i = 0; i < MODEL_COUNT; i++) {
        if (modelTriangleCounts[i] > 0) totalTriCountLM += modelTriangleCounts[i];
    }
    uint32_t *triLuxelMetadata = (uint32_t *)calloc(totalTriCountLM * 3, sizeof(uint32_t));
    uint64_t triOffset = 0;
    for (uint32_t i = 0; i < MODEL_COUNT; ++i) {
        if (!triLuxelOffsets[i] || modelTriangleCounts[i] == 0) continue;
        for (uint32_t f = 0; f < modelTriangleCounts[i]; ++f) {
            triLuxelMetadata[triOffset * 3 + 0] = triLuxelOffsets[i][f];
            triLuxelMetadata[triOffset * 3 + 1] = triLuxelDims[i][f * 2 + 0];
            triLuxelMetadata[triOffset * 3 + 2] = triLuxelDims[i][f * 2 + 1];
            triOffset++;
        }
        free(triLuxelOffsets[i]);
        free(triLuxelDims[i]);
    }
    GLuint triLuxelSSBO;
    glGenBuffers(1, &triLuxelSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, triLuxelSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, totalTriCountLM * 3 * sizeof(uint32_t), triLuxelMetadata, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 23, triLuxelSSBO);
    free(triLuxelMetadata);
    free(triLuxelOffsets);
    free(triLuxelDims);
    free(luxelOffsets);

    // Clean up
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    // Pass Model Type Bounds to GPU
    glGenBuffers(1, &modelBoundsID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelBoundsID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, MODEL_COUNT * BOUNDS_ATTRIBUTES_COUNT * sizeof(float), modelBounds, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, modelBoundsID);
    CHECK_GL_ERROR();
    malloc_trim(0);
    
    // Upload modelVertexOffsets
    uint32_t modelVertexOffsets[MODEL_COUNT];
    uint32_t offset = 0;
    for (uint32_t i = 0; i < MODEL_COUNT; i++) {
        modelVertexOffsets[i] = offset;
        offset += modelVertexCounts[i];
    }
    
    glGenBuffers(1, &modelVertexOffsetsID);
    CHECK_GL_ERROR();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelVertexOffsetsID);
    CHECK_GL_ERROR();
    glBufferData(GL_SHADER_STORAGE_BUFFER, MODEL_COUNT * sizeof(uint32_t), modelVertexOffsets, GL_STATIC_DRAW);
    CHECK_GL_ERROR();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 30, modelVertexOffsetsID);
    CHECK_GL_ERROR();
    
    // Upload triangle counts
    glGenBuffers(1, &tboMasterTable);
    CHECK_GL_ERROR();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, tboMasterTable);
    CHECK_GL_ERROR();
    glBufferData(GL_SHADER_STORAGE_BUFFER, MODEL_COUNT * sizeof(uint32_t), modelTriangleCounts, GL_STATIC_DRAW);
    CHECK_GL_ERROR();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 29, tboMasterTable);
    CHECK_GL_ERROR();
    
    // Pass Model Vertex Counts to GPU
    glGenBuffers(1, &modelVertexCountsID);
    CHECK_GL_ERROR();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelVertexCountsID);
    CHECK_GL_ERROR();
    glBufferData(GL_SHADER_STORAGE_BUFFER, MODEL_COUNT * sizeof(uint32_t), modelVertexCounts, GL_STATIC_DRAW);
    CHECK_GL_ERROR();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 28, modelVertexCountsID);
    CHECK_GL_ERROR();
    
    // Create duplicate of all mesh data in one flat buffer in VRAM without using RAM
    glGenBuffers(1, &vboMasterTable);
    CHECK_GL_ERROR();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vboMasterTable);
    CHECK_GL_ERROR();
    glBufferData(GL_SHADER_STORAGE_BUFFER, totalVertCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float), NULL, GL_STATIC_DRAW);
    CHECK_GL_ERROR();
    for (int i = 0; i < MODEL_COUNT; ++i) {
        glBindBuffer(GL_COPY_READ_BUFFER, vbos[i]);
        CHECK_GL_ERROR();
        glBindBuffer(GL_COPY_WRITE_BUFFER, vboMasterTable);
        CHECK_GL_ERROR();
        glCopyBufferSubData(GL_COPY_READ_BUFFER,GL_COPY_WRITE_BUFFER,0, modelVertexOffsets[i] * VERTEX_ATTRIBUTES_COUNT * sizeof(float), modelVertexCounts[i] * VERTEX_ATTRIBUTES_COUNT * sizeof(float));
        CHECK_GL_ERROR(); // Getting GL error here, 1282
    }

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 26, vboMasterTable);
    CHECK_GL_ERROR();
    
    free(tempVertices);
    free(tempTriangles);
    malloc_trim(0);
    double end_time = get_time();
    DualLog("Load Models took %f seconds\n", end_time - start_time);
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
    for (int i = 0; i < entityCount; ++i) {
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
    
    for (int i = entityCount; i < MAX_ENTITIES; ++i) {
        entities[i].modelIndex = UINT16_MAX;      
    }

    DebugRAM("after loading all entities");
    double end_time = get_time();
    DualLog("Load Entities took %f seconds\n", end_time - start_time);
    return 0;
}
#pragma GCC diagnostic pop // Ok restore string truncation warning

void GetLevel_Transform_Offsets(int curlevel, float* ofsx, float* ofsy, float* ofsz) {
    if (!global_modIsCitadel) { *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f;  return; }
    
    switch(curlevel) { // Match the parent transforms #.NAMELevel, e.g. 1.MedicalLevel
        case 0:  *ofsx = 3.6f; *ofsy = -4.10195f; *ofsz = 1.0f; break;
        case 1:  *ofsx = -5.12f; *ofsy = -48.64f; *ofsz = -15.36f; break;
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

void GetLevel_DynamicObjectsSaveableInstantiated_ContainerOffsets(int curlevel, float* ofsx, float* ofsy, float* ofsz) {
    if (!global_modIsCitadel) { *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f;  return; }

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
    if (!global_modIsCitadel) { *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f;  return; }

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
    if (!global_modIsCitadel) { *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f;  return; }

    switch(curlevel) { // Match the parent transforms #.NAMELevel, e.g. 1.LightsStaticImmutable
        case 0:  *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        case 1:  *ofsx = -5.12f; *ofsy = -48.37571f; *ofsz = -15.391001f; break;
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
    if (!global_modIsCitadel) { *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f;  return; }

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
    if (!global_modIsCitadel) { *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f;  return; }

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
    if (!global_modIsCitadel) { *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f;  return; }

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
    if (!global_modIsCitadel) { *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f;  return; }

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
// Loads level geometry and instance lightmaps
int LoadLevelGeometry(uint8_t curlevel) {
    if (curlevel >= numLevels) { DualLogError("Cannot load level %d, out of bounds 0 to %d\n", curlevel, numLevels - 1); return 1; }

    char filename[64];
    snprintf(filename, sizeof(filename), "./Data/CitadelScene_geometry_level%d.txt", curlevel);
    parser_init(&level_parser, valid_leveldata_keys, NUM_LEVDAT_KEYS);
    if (!parse_data_file(&level_parser, filename, 1)) { DualLogError("Could not parse %s!\n", filename); return 1; }

    gameObjectCount = level_parser.count;
    DualLog("Loading %d objects for Level %d...\n", gameObjectCount, curlevel);
    float correctionX, correctionY, correctionZ;
    GetLevel_Transform_Offsets(curlevel, &correctionX, &correctionY, &correctionZ);

    // Calculate instance lightmap offsets.
    uint64_t totalInstanceLuxelCount = 0;
    uint32_t *instanceLuxelOffsets = (uint32_t *)calloc(INSTANCE_COUNT, sizeof(uint32_t));
    if (!instanceLuxelOffsets) { DualLogError("Failed to allocate instanceLuxelOffsets\n"); return 1; }
    uint32_t validInstanceCount = 0;
    for (int idx = 0; idx < gameObjectCount; idx++) {
        int entIdx = level_parser.entries[idx].constIndex;
        if (entIdx < 0 || entIdx >= MAX_ENTITIES) {
//             DualLogError("Invalid entIdx %d for instance %d, skipping\n", entIdx, idx);
            continue;
        }
        int32_t modelIndex = entities[entIdx].modelIndex;
        if (modelIndex < 0 || modelIndex >= MODEL_COUNT || modelLuxelCounts[modelIndex] == 0) {
//             DualLogError("Invalid modelIndex %d for instance %d (entIdx %d), luxelCount=%u, skipping\n", modelIndex, idx, entIdx, modelLuxelCounts[modelIndex]);
            continue;
        }
        instanceLuxelOffsets[idx] = totalInstanceLuxelCount;
        totalInstanceLuxelCount += modelLuxelCounts[modelIndex];
        validInstanceCount++;
    }

    // Create instance lightmap SSBO (RGBA floats for HDR)
    float *instanceLuxelData = (float *)calloc(totalInstanceLuxelCount * 4, sizeof(float));
    if (!instanceLuxelData) { DualLogError("Failed to allocate %llu instance luxels\n", totalInstanceLuxelCount); free(instanceLuxelOffsets); return 1; }
    for (uint64_t j = 0; j < totalInstanceLuxelCount; j++) {
        instanceLuxelData[j * 4 + 0] = 1.0f;
        instanceLuxelData[j * 4 + 1] = 1.0f;
        instanceLuxelData[j * 4 + 2] = 1.0f;
        instanceLuxelData[j * 4 + 3] = 1.0f;
    }
    GLuint instanceLightmapSSBO;
    glGenBuffers(1, &instanceLightmapSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, instanceLightmapSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, totalInstanceLuxelCount * 4 * sizeof(float), instanceLuxelData, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 24, instanceLightmapSSBO);
    free(instanceLuxelData);

    // Create instance offset SSBO
    GLuint instanceOffsetSSBO;
    glGenBuffers(1, &instanceOffsetSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, instanceOffsetSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, INSTANCE_COUNT * sizeof(uint32_t), instanceLuxelOffsets, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 25, instanceOffsetSSBO);
    free(instanceLuxelOffsets);

    // Load instance data
    for (int idx = 0; idx < gameObjectCount; idx++) {
        int entIdx = level_parser.entries[idx].constIndex;
        if (entIdx < 0 || entIdx >= MAX_ENTITIES) continue;
        uint16_t modelIndex = entities[entIdx].modelIndex;
        if (modelIndex >= MODEL_COUNT || modelLuxelCounts[modelIndex] == 0u) continue; // Skip instances that don't have models

        instances[idx].modelIndex = entities[entIdx].modelIndex;
        instances[idx].texIndex = entities[entIdx].texIndex;
        instances[idx].glowIndex = entities[entIdx].glowIndex;
        instances[idx].specIndex = entities[entIdx].specIndex;
        instances[idx].normIndex = entities[entIdx].normIndex;
        instances[idx].lodIndex = entities[entIdx].lodIndex;
        instances[idx].posx = level_parser.entries[idx].localPosition.x + correctionX;
        instances[idx].posy = level_parser.entries[idx].localPosition.y + correctionY;
        instances[idx].posz = level_parser.entries[idx].localPosition.z + correctionZ;
        instances[idx].rotx = level_parser.entries[idx].localRotation.x;
        instances[idx].roty = level_parser.entries[idx].localRotation.y;
        instances[idx].rotz = level_parser.entries[idx].localRotation.z;
        instances[idx].rotw = level_parser.entries[idx].localRotation.w;
        instances[idx].sclx = level_parser.entries[idx].localScale.x;
        instances[idx].scly = level_parser.entries[idx].localScale.y;
        instances[idx].sclz = level_parser.entries[idx].localScale.z;
        Quaternion quat = {instances[idx].rotx, instances[idx].roty, instances[idx].rotz, instances[idx].rotw};
        Quaternion upQuat = {1.0f, 0.0f, 0.0f, 0.0f};
        float angle = quat_angle_deg(quat, upQuat);
        bool pointsUp = angle <= 30.0f;
        instances[idx].floorHeight = global_modIsCitadel && pointsUp && currentLevel <= 12 ? instances[idx].posy : INVALID_FLOOR_HEIGHT;
    }

    malloc_trim(0);
    return 0;
}

int LoadLevelLights(uint8_t curlevel) {
    if (curlevel >= numLevels) { DualLogError("Cannot load level lights %d, out of bounds 0 to %d\n",curlevel,numLevels - 1); return 1; }

    char filename[64];
    snprintf(filename, sizeof(filename), "./Data/CitadelScene_lights_level%d.txt", curlevel);
    parser_init(&lights_parser, valid_lightdata_keys, NUM_LIGHTDAT_KEYS);
    if (!parse_data_file(&lights_parser, filename,1)) { DualLogError("Could not parse %s!\n",filename); return 1; }

    int lightsCount = lights_parser.count;
    DualLog("Loading %d lights for Level %d...\n",lightsCount,curlevel);
    float correctionLightX, correctionLightY, correctionLightZ;
    GetLevel_LightsStaticImmutable_ContainerOffsets(curlevel,&correctionLightX,&correctionLightY,&correctionLightZ);
    for (int i=0;i<lightsCount;++i) {
        uint16_t idx = (i * LIGHT_DATA_SIZE);
        lights[idx + LIGHT_DATA_OFFSET_POSX] = lights_parser.entries[i].localPosition.x + correctionLightX;
        lights[idx + LIGHT_DATA_OFFSET_POSY] = lights_parser.entries[i].localPosition.y + correctionLightY;
        lights[idx + LIGHT_DATA_OFFSET_POSZ] = lights_parser.entries[i].localPosition.z + correctionLightZ;
        lights[idx + LIGHT_DATA_OFFSET_INTENSITY] = lights_parser.entries[i].intensity;
        lights[idx + LIGHT_DATA_OFFSET_RANGE] = lights_parser.entries[i].range;
        lightsRangeSquared[i] = lights_parser.entries[i].range * lights_parser.entries[i].range;
        lights[idx + LIGHT_DATA_OFFSET_SPOTANG] = lights_parser.entries[i].type == 0 ? 0.0f : lights_parser.entries[i].spotAngle; // If spot apply it, else get 0 for spotAng
        lights[idx + LIGHT_DATA_OFFSET_SPOTDIRX] = lights_parser.entries[i].localRotation.x;
        lights[idx + LIGHT_DATA_OFFSET_SPOTDIRY] = lights_parser.entries[i].localRotation.y;
        lights[idx + LIGHT_DATA_OFFSET_SPOTDIRZ] = lights_parser.entries[i].localRotation.z;
        lights[idx + LIGHT_DATA_OFFSET_SPOTDIRW] = lights_parser.entries[i].localRotation.w;
        lights[idx + LIGHT_DATA_OFFSET_R] = lights_parser.entries[i].color.r;
        lights[idx + LIGHT_DATA_OFFSET_G] = lights_parser.entries[i].color.g;
        lights[idx + LIGHT_DATA_OFFSET_B] = lights_parser.entries[i].color.b;
    }

    malloc_trim(0);
    return 0;
}
