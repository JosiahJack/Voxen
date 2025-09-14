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
#include <assimp/version.h>
#include <math.h>
#include "constants.h"
#include "data_parser.h"
#include "debug.h"
#include "render.h"
#include "event.h"

// #define DEBUG_MODEL_LOAD_DATA 1U
#define NUM_MODEL_KEYS 1
#define MAX_VERT_COUNT 32768
#define MAX_TRI_COUNT 32768

uint32_t modelVertexCounts[MODEL_COUNT];
uint32_t modelTriangleCounts[MODEL_COUNT];
GLuint vbos[MODEL_COUNT];
GLuint tbos[MODEL_COUNT];
GLuint modelBoundsID;
float modelBounds[MODEL_COUNT * BOUNDS_ATTRIBUTES_COUNT];
uint32_t largestVertCount = 0;
uint32_t largestTriangleCount = 0;
float * tempVertices;
uint32_t * tempTriangles;
GLuint lightmapID;
uint32_t renderableCount = 0;
uint32_t loadedInstances = 0;
int startOfDoubleSidedInstances = INSTANCE_COUNT - 1;
int startOfTransparentInstances = INSTANCE_COUNT - 1;
DataParser model_parser;
DataParser level_parser;
DataParser lights_parser;
DataParser dynamics_parser;

void parser_init(DataParser *parser) {
    parser->entries = NULL;
    parser->count = 0;
    parser->capacity = 0;
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

uint8_t parse_saveablestring(const char* value, const char* line, uint32_t lineNum) {
    if (value == NULL || *value == '\0') { fprintf(stderr, "Invalid float input blank string, from line[%d]: %s\n", lineNum, line); return 36; }

    if (strcmp(value, "Player") == 0) return 0;
    if (strcmp(value, "Useable") == 0) return 1;
    if (strcmp(value, "Grenade") == 0) return 2;
    if (strcmp(value, "NPC") == 0) return 3;
    if (strcmp(value, "Destructable") == 0) return 4;
    if (strcmp(value, "SearchableStatic") == 0) return 5;
    if (strcmp(value, "SearchableDestructable") == 0) return 6;
    if (strcmp(value, "Door") == 0) return 7;
    if (strcmp(value, "ForceBridge") == 0) return 8;
    if (strcmp(value, "Switch") == 0) return 9;
    if (strcmp(value, "FuncWall") == 0) return 10;
    if (strcmp(value, "TeleDest") == 0) return 11;
    if (strcmp(value, "LBranch") == 0) return 12;
    if (strcmp(value, "LRelay") == 0) return 13;
    if (strcmp(value, "LSpawner") == 0) return 14;
    if (strcmp(value, "InteractablePanel") == 0) return 15;
    if (strcmp(value, "ElevatorPanel") == 0) return 16;
    if (strcmp(value, "Keypad") == 0) return 17;
    if (strcmp(value, "PuzzleGrid") == 0) return 18;
    if (strcmp(value, "PuzzleWire") == 0) return 19;
    if (strcmp(value, "TCounter") == 0) return 20;
    if (strcmp(value, "TGravity") == 0) return 21;
    if (strcmp(value, "MChanger") == 0) return 22;
    if (strcmp(value, "GravPad") == 0) return 23;
    if (strcmp(value, "TransformParentless") == 0) return 24;
    if (strcmp(value, "ChargeStation") == 0) return 25;
    if (strcmp(value, "Light") == 0) return 26;
    if (strcmp(value, "LTimer") == 0) return 27;
    if (strcmp(value, "Camera") == 0) return 28;
    if (strcmp(value, "DelayedSpawn") == 0) return 29;
    if (strcmp(value, "SecurityCamera") == 0) return 30;
    if (strcmp(value, "Trigger") == 0) return 31;
    if (strcmp(value, "Projectile") == 0) return 32;
    if (strcmp(value, "NormalScreen") == 0) return 33;
    if (strcmp(value, "CyberSwitch") == 0) return 34;
    if (strcmp(value, "CyberItem") == 0) return 35;
    if (strcmp(value, "Transform") == 0) return 36;
    DualLogError("Unknown saveableType '%s' at line %u: %s\n", value, lineNum, line);
    return 36; // Default to Transform
}

void init_data_entry(Entity *entry) {
    entry->modname[0] = '\0';
    entry->levelCount = 0;
    entry->startLevel = 0;
    entry->type = 0;
    entry->cardchunk = false;
    entry->doublesided = false;
    entry->index = UINT16_MAX;
    entry->modelIndex = 1024;
    entry->texIndex = UINT16_MAX;
    entry->glowIndex = 2048;
    entry->specIndex = 2048;
    entry->normIndex = 2048;
    entry->lodIndex = UINT16_MAX;
    entry->path[0] = '\0';
    entry->position.x = 0.0f; entry->position.y = 0.0f; entry->position.z = 0.0f;
    entry->rotation.x = 0.0f; entry->rotation.y = 0.0f; entry->rotation.z = 0.0f; entry->rotation.w = 1.0f;
    entry->scale.x = 1.0f; entry->scale.y = 1.0f; entry->scale.z = 1.0f;
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
        Entity *new_entries = realloc(parser->entries, entry_count * sizeof(Entity));        
        parser->entries = new_entries;
        for (int i = parser->capacity; i < entry_count; ++i) init_data_entry(&parser->entries[i]);
        parser->capacity = entry_count;
    }
    parser->count = entry_count;
}

bool process_key_value(Entity *entry, const char *key, const char *value, const char *line, uint32_t lineNum) {
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

    for (int i = 0; i < ENTITY_FIELD_COUNT; i++) {
             if (strcmp(trimmed_key, "index") == 0)           entry->index = parse_numberu16(trimmed_value, line, lineNum);
        else if (strcmp(trimmed_key, "constIndex") == 0)      entry->index = parse_numberu16(trimmed_value, line, lineNum);
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
        else if (strcmp(trimmed_key, "lod") == 0)             entry->lodIndex = parse_numberu16(trimmed_value, line, lineNum);
        else if (strcmp(trimmed_key, "localPosition.x") == 0) entry->position.x = parse_float(trimmed_value, line, lineNum);
        else if (strcmp(trimmed_key, "localPosition.y") == 0) entry->position.y = parse_float(trimmed_value, line, lineNum);
        else if (strcmp(trimmed_key, "localPosition.z") == 0) entry->position.z = parse_float(trimmed_value, line, lineNum);
        else if (strcmp(trimmed_key, "localRotation.x") == 0) entry->rotation.x = parse_float(trimmed_value, line, lineNum);
        else if (strcmp(trimmed_key, "localRotation.y") == 0) entry->rotation.y = parse_float(trimmed_value, line, lineNum);
        else if (strcmp(trimmed_key, "localRotation.z") == 0) entry->rotation.z = parse_float(trimmed_value, line, lineNum);
        else if (strcmp(trimmed_key, "localRotation.w") == 0) entry->rotation.w = parse_float(trimmed_value, line, lineNum);
        else if (strcmp(trimmed_key, "localScale.x") == 0)    entry->scale.x = parse_float(trimmed_value, line, lineNum);
        else if (strcmp(trimmed_key, "localScale.y") == 0)    entry->scale.y = parse_float(trimmed_value, line, lineNum);
        else if (strcmp(trimmed_key, "localScale.z") == 0)    entry->scale.z = parse_float(trimmed_value, line, lineNum);
        else if (strcmp(trimmed_key, "intensity") == 0)       entry->intensity = parse_float(trimmed_value, line, lineNum);
        else if (strcmp(trimmed_key, "range") == 0)           entry->range = parse_float(trimmed_value, line, lineNum);
        else if (strcmp(trimmed_key, "spotAngle") == 0)       entry->spotAngle = parse_float(trimmed_value, line, lineNum);
        else if (strcmp(trimmed_key, "type") == 0)            entry->type = (strcmp(trimmed_value, "Spot") == 0) ? 1u : 0u;
        else if (strcmp(trimmed_key, "color.r") == 0)         entry->color.r = parse_float(trimmed_value, line, lineNum);
        else if (strcmp(trimmed_key, "color.g") == 0)         entry->color.g = parse_float(trimmed_value, line, lineNum);
        else if (strcmp(trimmed_key, "color.b") == 0)         entry->color.b = parse_float(trimmed_value, line, lineNum);
        else if (strcmp(trimmed_key, "saveableType") == 0)    entry->saveableType = parse_saveablestring(trimmed_value, line, lineNum); // TODO
        else if (strcmp(trimmed_key, "go.activeSelf") == 0)   entry->active = parse_bool(trimmed_value, line, lineNum);
        return true;
    }
    
    return false;
}

bool read_token(FILE *file, char *token, size_t max_len, char delimiter, bool *is_comment, bool *is_eof, bool *is_newline, uint32_t *lineNum) {
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

static bool ParseResourceData(DataParser *parser, FILE* file, const char *filename) {
    char line[1024];
    uint32_t lineNum = 0;
    int entry_count = 0;
    uint32_t max_index = 0;
    while (fgets(line, sizeof(line), file)) { // First pass: count entries and find max index
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

    if (max_index == 0) { DualLogWarn("No entries found in %s\n", filename); fclose(file); return true; }

    allocate_entries(parser, max_index + 1);

    // Second pass: parse entries
    rewind(file);
    Entity entry;
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
                process_key_value(&entry, key, value, start, lineNum);
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

static bool ParseSaveLevelData(DataParser *parser, FILE* file, const char *filename) {
    int entry_count = 0;
    bool is_comment, is_eof, is_newline;
    int c;
    uint32_t lineNum = 0;
    while ((c = fgetc(file)) != EOF) { // First pass: count entries
        if (isspace(c) || c == '\n') continue;
        if (c == '/' && (c = fgetc(file)) == '/') {
            while ((c = fgetc(file)) != EOF && c != '\n');
            continue;
        }
        entry_count++;
        while ((c = fgetc(file)) != EOF && c != '\n');
    }

    if (entry_count == 0) { DualLogWarn("No entries found in %s\n", filename); fclose(file); return true; }

    allocate_entries(parser, entry_count);

    // Second pass: parse entries
    rewind(file);
    int current_index = 0;
    lineNum = 0;
    char token[1024];
    Entity entry;
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
            process_key_value(&entry, key, value, token, lineNum);
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
    FILE *file = fopen(filename, "r");
    if (!file) { DualLogError("Cannot open %s: %s\n", filename, strerror(errno)); return false; }
    
    if (type == 1) return ParseSaveLevelData(parser, file, filename);
    else return ParseResourceData(parser, file, filename);
}

//-----------------------------------------------------------------------------
// Loads all 3D meshes
int LoadModels(void) {
    double start_time = get_time();
    DebugRAM("start of LoadModels");
    parser_init(&model_parser); // First parse ./Data/models.txt to see what to load to what indices
    if (!parse_data_file(&model_parser, "./Data/models.txt",0)) { DualLogError("Could not parse ./Data/models.txt!\n"); return 1; }

    int maxIndex = -1;
    for (int k=0;k<model_parser.count;k++) {
        if (model_parser.entries[k].index > maxIndex && model_parser.entries[k].index != UINT16_MAX) maxIndex = model_parser.entries[k].index;
    }

    DualLog("Loading %d models with max index of %d, using Assimp version: %d.%d.%d (rev %d, flags %d)...", model_parser.count, maxIndex, aiGetVersionMajor(), aiGetVersionMinor(), aiGetVersionPatch(), aiGetVersionRevision(), aiGetCompileFlags());
    int totalVertCount = 0;
    int totalBounds = 0;
    int totalTriCount = 0;
    largestVertCount = 0;
    largestTriangleCount = 0;
    tempVertices = (float*)malloc(MAX_VERT_COUNT * VERTEX_ATTRIBUTES_COUNT * sizeof(float));
    tempTriangles = (uint32_t*)malloc(MAX_TRI_COUNT * 3 * sizeof(uint32_t));
    uint32_t* vertexOffsets = (uint32_t*)calloc(MODEL_COUNT, sizeof(uint32_t));
    uint32_t* triangleOffsets = (uint32_t*)calloc(MODEL_COUNT, sizeof(uint32_t));
    uint32_t currentVertexOffset = 0;
    uint32_t currentTriangleOffset = 0;

    GLuint stagingVBO, stagingTBO;
    glGenBuffers(1, &stagingVBO);
    glGenBuffers(1, &stagingTBO);
    glBindBuffer(GL_ARRAY_BUFFER, stagingVBO);
    glBufferData(GL_ARRAY_BUFFER, MAX_VERT_COUNT * VERTEX_ATTRIBUTES_COUNT * sizeof(float), NULL, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stagingTBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_TRI_COUNT * 3 * sizeof(uint32_t), NULL, GL_DYNAMIC_COPY);
    for (uint32_t i = 0; i < MODEL_COUNT; i++) {
        int matchedParserIdx = -1;
        for (int k=0;k<model_parser.count;k++) {
            if (model_parser.entries[k].index == i) {matchedParserIdx = k; break; }
        }

        if (matchedParserIdx < 0) continue;

        RenderLoadingProgress(105,"Loading models [%d of %d]...",matchedParserIdx,model_parser.count);
        if (!model_parser.entries[matchedParserIdx].path || model_parser.entries[matchedParserIdx].path[0] == '\0') continue;

        struct aiPropertyStore* props = aiCreatePropertyStore(); // Disable non-essential FBX components
        aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_READ_ANIMATIONS, 1);
        aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_READ_MATERIALS, 0); // Disable materials
        aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_READ_TEXTURES, 0); // Disable textures
        aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_READ_LIGHTS, 0); // Disable lights
        aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_READ_CAMERAS, 0); // Disable cameras
        aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_OPTIMIZE_EMPTY_ANIMATION_CURVES, 1); // Drop empty animations
        aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_NO_SKELETON_MESHES, 0); // Disable skeleton meshes
        aiSetImportPropertyInteger(props, AI_CONFIG_PP_RVC_FLAGS, aiComponent_ANIMATIONS | aiComponent_BONEWEIGHTS);
        aiSetImportPropertyInteger(props, AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT); // Skip non-triangular primitives
        aiSetImportPropertyInteger(props, AI_CONFIG_PP_ICL_PTCACHE_SIZE, 12); // Optimize vertex cache
        aiSetImportPropertyInteger(props, AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4); // Limit bone weights
        aiSetImportPropertyInteger(props, AI_CONFIG_PP_FD_REMOVE, 1); // Remove degenerate primitives
        aiSetImportPropertyInteger(props, AI_CONFIG_PP_PTV_KEEP_HIERARCHY, 0); // Disable hierarchy preservation
        const struct aiScene *scene = aiImportFileExWithProperties(model_parser.entries[matchedParserIdx].path, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality, NULL, props);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) { DualLogError("Assimp failed to load %s: %s\n", model_parser.entries[matchedParserIdx].path, aiGetErrorString()); return 1; }

        // Count vertices, triangles
        uint32_t vertexCount = 0;
        uint32_t triCount = 0;
        for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
            vertexCount += scene->mMeshes[m]->mNumVertices;
            triCount += scene->mMeshes[m]->mNumFaces;
        }

        if (vertexCount > MAX_VERT_COUNT || triCount > MAX_TRI_COUNT) { DualLogError("\nModel %s exceeds buffer limits: verts=%u (> %u), tris=%u (> %u)\n", model_parser.entries[matchedParserIdx].path, vertexCount, MAX_VERT_COUNT, triCount, MAX_TRI_COUNT); return 1; }

        modelVertexCounts[i] = vertexCount;
        modelTriangleCounts[i] = triCount;
        if (vertexCount > largestVertCount) largestVertCount = vertexCount;
        if (triCount > largestTriangleCount) largestTriangleCount = triCount;

#ifdef DEBUG_MODEL_LOAD_DATA
        if (triCount > 1000U) {
            DualLog("\nModel %s loaded with %d vertices, \033[1;33m%d\033[0;0m triangles\n", model_parser.entries[matchedParserIdx].path, vertexCount, triCount);
        } else {
            DualLog("\nModel %s loaded with %d vertices, %d triangles\n", model_parser.entries[matchedParserIdx].path, vertexCount, triCount);
        }
#endif
        totalVertCount += vertexCount;
        totalTriCount += triCount;
        vertexOffsets[i] = currentVertexOffset;
        triangleOffsets[i] = currentTriangleOffset;

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
                tempVertices[vertexIndex++] = 0.0f; // u_lm for lightmap
                tempVertices[vertexIndex++] = 0.0f; // v_lm for lightmap
                if (mesh->mVertices[v].x < minx) minx = mesh->mVertices[v].x;
                if (mesh->mVertices[v].x > maxx) maxx = mesh->mVertices[v].x;
                if (mesh->mVertices[v].y < miny) miny = mesh->mVertices[v].y;
                if (mesh->mVertices[v].y > maxy) maxy = mesh->mVertices[v].y;
                if (mesh->mVertices[v].z < minz) minz = mesh->mVertices[v].z;
                if (mesh->mVertices[v].z > maxz) maxz = mesh->mVertices[v].z;
            }

            // Triangle data
            for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
                struct aiFace *face = &mesh->mFaces[f];
                if (face->mNumIndices != 3) { DualLogError("\nNon-triangular face detected in %s, face %u\n", model_parser.entries[matchedParserIdx].path, f); return 1; }

                uint32_t v[3] = {face->mIndices[0] + globalVertexOffset, face->mIndices[1] + globalVertexOffset, face->mIndices[2] + globalVertexOffset};

                // Validate vertex indices
                if (v[0] >= vertexCount || v[1] >= vertexCount || v[2] >= vertexCount) { DualLogError("\nInvalid vertex index in %s, face %u: v0=%u, v1=%u, v2=%u, vertexCount=%u\n", model_parser.entries[matchedParserIdx].path, f, v[0], v[1], v[2], vertexCount); return 1; }

                // Store vertex indices
                tempTriangles[triangleIndex++] = v[0];
                tempTriangles[triangleIndex++] = v[1];
                tempTriangles[triangleIndex++] = v[2];
            }
            
            globalVertexOffset += mesh->mNumVertices;
        }

        currentVertexOffset += vertexCount;
        currentTriangleOffset += triCount;
        aiReleaseImport(scene);
        aiReleasePropertyStore(props);
        aiDetachAllLogStreams();
        malloc_trim(0);
        if (vertexCount > 0) { // Copy to staging buffers
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

        modelBounds[(i * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_MINX] = minx;
        modelBounds[(i * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_MINY] = miny;
        modelBounds[(i * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_MINZ] = minz;
        modelBounds[(i * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_MAXX] = maxx;
        modelBounds[(i * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_MAXY] = maxy;
        modelBounds[(i * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_MAXZ] = maxz;
        modelBounds[(i * BOUNDS_ATTRIBUTES_COUNT) + BOUNDS_DATA_OFFSET_RADIUS] = fmaxf(fmaxf(fmaxf(fmaxf(fmaxf(fabs(minx), fabs(miny)), fabs(minz)), maxx), maxy), maxz);
        totalBounds += BOUNDS_ATTRIBUTES_COUNT;
    }

    // Delete staging buffers
    glDeleteBuffers(1, &stagingVBO);
    glDeleteBuffers(1, &stagingTBO);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

#ifdef DEBUG_MODEL_LOAD_DATA
    DualLog("\nLargest vertex count: %d, triangle count: %d\n", largestVertCount, largestTriangleCount);
    DualLog("Total vertices: %d (", totalVertCount);
    print_bytes_no_newline(totalVertCount * VERTEX_ATTRIBUTES_COUNT * sizeof(float));
    DualLog(")\nTotal triangles: %d (", totalTriCount);
    print_bytes_no_newline(totalTriCount * 3 * sizeof(uint32_t));
    DualLog(")\nBounds (");
    print_bytes_no_newline(totalBounds * sizeof(float));
    DualLog(")\n");
#endif
    
    RenderLoadingProgress(105,"Loading models [%d of %d]...",model_parser.count,model_parser.count);

    // Pass Model Type Bounds to GPU
    glGenBuffers(1, &modelBoundsID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelBoundsID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, MODEL_COUNT * BOUNDS_ATTRIBUTES_COUNT * sizeof(float), modelBounds, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, modelBoundsID);
    CHECK_GL_ERROR();
    free(tempVertices);
    free(tempTriangles);
    free(triangleOffsets);
    free(vertexOffsets);
    glFlush();
    glFinish();
    malloc_trim(0);
    double end_time = get_time();
    DualLog(" took %f seconds\n", end_time - start_time);
    DebugRAM("After Load Models");
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
    parser_init(&entity_parser);
    if (!parse_data_file(&entity_parser, "./Data/entities.txt",0)) { DualLogError("Could not parse ./Data/entities.txt!\n"); return 1; }
    
    loadEntityItemInitialized[ENT_PARSER] = true;
    entityCount = entity_parser.count;
    if (entityCount > MAX_ENTITIES) { DualLogError("Too many entities in parser count %d, greater than %d!\n", entityCount, MAX_ENTITIES); return 1; }
    if (entityCount == 0) { DualLogError("No entities found in entities.txt\n"); return 1; }

    DualLog("Parsing %d entities...", entityCount);

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
        entities[i].cardchunk = entity_parser.entries[i].cardchunk;
        entities[i].position.x = 0.0f;
        entities[i].position.y = 0.0f;
        entities[i].position.z = 0.0f;
        entities[i].scale.x = 0.0f;
        entities[i].scale.y = 0.0f;
        entities[i].scale.z = 0.0f;
        entities[i].rotation.x = 0.0f;
        entities[i].rotation.y = 0.0f;
        entities[i].rotation.z = 0.0f;
        entities[i].rotation.w = 0.0f;
        entities[i].floorHeight = INVALID_FLOOR_HEIGHT;
    }

    double end_time = get_time();
    DualLog(" took %f seconds\n", end_time - start_time);
    DebugRAM("after loading all entities");
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
int LoadLevelGeometry(uint8_t curlevel) {
    if (curlevel >= numLevels) { DualLogError("Cannot load world geometry, level number %d out of bounds 0 to %d\n",curlevel,numLevels - 1); return 1; }

    DebugRAM("start of LoadLevelGeometry");
    char filename[64];
    snprintf(filename, sizeof(filename), "./Data/CitadelScene_geometry_level%d.txt", curlevel);
    parser_init(&level_parser);
    if (!parse_data_file(&level_parser, filename,1)) { DualLogError("Could not parse %s!\n",filename); return 1; }

    int gameObjectCount = level_parser.count;
    DualLog("Loading %d objects for Level %d...\n",gameObjectCount,curlevel);
    float correctionX, correctionY, correctionZ;
    GetLevel_Transform_Offsets(curlevel,&correctionX,&correctionY,&correctionZ);
    for (int idx=0;idx<gameObjectCount;++idx) {
        loadedInstances++;
        instances[idx] = level_parser.entries[idx];
        int entIdx = level_parser.entries[idx].index;
        instances[idx].modelIndex = entities[entIdx].modelIndex;
        if (instances[idx].modelIndex < MODEL_COUNT) renderableCount++;
        instances[idx].texIndex = entities[entIdx].texIndex;
        instances[idx].glowIndex = entities[entIdx].glowIndex;
        instances[idx].specIndex = entities[entIdx].specIndex;
        instances[idx].normIndex = entities[entIdx].normIndex;
        instances[idx].lodIndex = entities[entIdx].lodIndex;
        instances[idx].position.x += correctionX;
        instances[idx].position.y += correctionY;
        instances[idx].position.z += correctionZ;
        if (isDoubleSided(instances[idx].texIndex) || instances[idx].scale.x < 0.0f || instances[idx].scale.y < 0.0f || instances[idx].scale.z < 0.0f) {
            doubleSidedInstances[doubleSidedInstancesHead] = idx;
            doubleSidedInstancesHead++; // Already sized to INSTANCE_COUNT, no need for bounds check.
        }

        if (isTransparent(instances[idx].texIndex)) {
            transparentInstances[transparentInstancesHead] = idx;
            transparentInstancesHead++; // Already sized to INSTANCE_COUNT, no need for bounds check.
        }

        Quaternion quat = {instances[idx].rotation.x, instances[idx].rotation.y, instances[idx].rotation.z, instances[idx].rotation.w};
        Quaternion upQuat = {1.0f, 0.0f, 0.0f, 0.0f};
        float angle = quat_angle_deg(quat,upQuat); // Get angle in degrees relative to up vector
        bool pointsUp = angle <= 30.0f;
        instances[idx].floorHeight = global_modIsCitadel && pointsUp && currentLevel <= 12 ? instances[idx].position.y : INVALID_FLOOR_HEIGHT; // TODO: Citadel specific max floor height caring level threshold of 12
    }

    // Filter transparent instances to exclude double-sided ones
    int transparentOnlyCount = 0;
    int transparentOnlyIndices[INSTANCE_COUNT];
    for (int i = 0; i < transparentInstancesHead; ++i) {
        int idx = transparentInstances[i];
        bool isDoubleSided = false;
        for (int j = 0; j < doubleSidedInstancesHead; ++j) {
            if (doubleSidedInstances[j] == idx) {
                isDoubleSided = true;
                break;
            }
        }
        if (!isDoubleSided) {
            transparentOnlyIndices[transparentOnlyCount] = idx;
            transparentOnlyCount++;
        }
    }

    startOfDoubleSidedInstances = gameObjectCount - doubleSidedInstancesHead - transparentOnlyCount; // e.g., 5453 - 19 - 42 = 5392
    startOfTransparentInstances = gameObjectCount - doubleSidedInstancesHead;
    int maxValidIdx = startOfDoubleSidedInstances;
    for (int i = 0; i < doubleSidedInstancesHead; ++i) {
        int idx = doubleSidedInstances[i];
        if (idx < 0 || idx >= gameObjectCount) continue;

        Entity tempInstance = instances[maxValidIdx];
        instances[maxValidIdx] = instances[idx];
        instances[idx] = tempInstance;
        maxValidIdx++;
    }

    for (int i = 0; i < transparentOnlyCount; ++i) {
        int idx = transparentOnlyIndices[i];
        if (idx < 0 || idx >= gameObjectCount) continue;

        Entity tempInstance = instances[maxValidIdx];
        instances[maxValidIdx] = instances[idx];
        instances[idx] = tempInstance;
        maxValidIdx++;
    }

    // Instances uploaded after loading statics and dynamics in next functions...
    
    glGenBuffers(1, &lightmapID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightmapID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, renderableCount * 64 * 64 * 4 * sizeof(float), NULL, GL_DYNAMIC_DRAW); // 256x256 lightmap per model, 4 channel rgba, HDR float
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, lightmapID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    DebugRAM("end of LoadLevelGeometry");
    return 0;
}

int LoadLevelLights(uint8_t curlevel) {
    if (curlevel >= numLevels) { DualLogError("Cannot load level lights, level number %d out of bounds 0 to %d\n",curlevel,numLevels - 1); return 1; }

    DebugRAM("start of LoadLevelLights");
    char filename[64];
    snprintf(filename, sizeof(filename), "./Data/CitadelScene_lights_level%d.txt", curlevel);
    parser_init(&lights_parser);
    if (!parse_data_file(&lights_parser, filename,1)) { DualLogError("Could not parse %s!\n",filename); return 1; }

    int lightsCount = lights_parser.count;
    DualLog("Loading %d lights for Level %d...\n",lightsCount,curlevel);
    float correctionLightX, correctionLightY, correctionLightZ;
    GetLevel_LightsStaticImmutable_ContainerOffsets(curlevel,&correctionLightX,&correctionLightY,&correctionLightZ);
    for (int i=0;i<lightsCount;++i) {
        uint16_t idx = (i * LIGHT_DATA_SIZE);
        lights[idx + LIGHT_DATA_OFFSET_POSX] = lights_parser.entries[i].position.x + correctionLightX;
        lights[idx + LIGHT_DATA_OFFSET_POSY] = lights_parser.entries[i].position.y + correctionLightY;
        lights[idx + LIGHT_DATA_OFFSET_POSZ] = lights_parser.entries[i].position.z + correctionLightZ;
        lights[idx + LIGHT_DATA_OFFSET_INTENSITY] = lights_parser.entries[i].intensity;
        lights[idx + LIGHT_DATA_OFFSET_RANGE] = lights_parser.entries[i].range;
        lights[idx + LIGHT_DATA_OFFSET_SPOTANG] = lights_parser.entries[i].type == 0 ? 0.0f : lights_parser.entries[i].spotAngle; // If spot apply it, else get 0 for spotAng
        lights[idx + LIGHT_DATA_OFFSET_SPOTDIRX] = lights_parser.entries[i].rotation.x;
        lights[idx + LIGHT_DATA_OFFSET_SPOTDIRY] = lights_parser.entries[i].rotation.y;
        lights[idx + LIGHT_DATA_OFFSET_SPOTDIRZ] = lights_parser.entries[i].rotation.z;
        lights[idx + LIGHT_DATA_OFFSET_SPOTDIRW] = lights_parser.entries[i].rotation.w;
        lights[idx + LIGHT_DATA_OFFSET_R] = lights_parser.entries[i].color.r;
        lights[idx + LIGHT_DATA_OFFSET_G] = lights_parser.entries[i].color.g;
        lights[idx + LIGHT_DATA_OFFSET_B] = lights_parser.entries[i].color.b;
    }

    DebugRAM("end of LoadLevelLights");
    return 0;
}

int LoadLevelDynamicObjects(uint8_t curlevel) {
    if (curlevel >= numLevels) { DualLogError("Cannot load level dynamic objects, level number %d out of bounds 0 to %d\n",curlevel,numLevels - 1); return 1; }

    DebugRAM("start of LoadLevelDynamicObjects");
    char filename[64];
    snprintf(filename, sizeof(filename), "./Data/CitadelScene_dynamics_level%d.txt", curlevel);
    parser_init(&dynamics_parser);
    if (!parse_data_file(&dynamics_parser, filename,1)) { DualLogError("Could not parse %s!\n",filename); return 1; }

    int dynamicObjectCount = dynamics_parser.count;
    DualLog("Loading %d dynamic objects for Level %d...\n",dynamicObjectCount,curlevel);
    float correctionX, correctionY, correctionZ;
    GetLevel_Transform_Offsets(curlevel,&correctionX,&correctionY,&correctionZ);
    int startingIdx = (int)loadedInstances;
    for (int idx=loadedInstances, i = 0;idx<(startingIdx + dynamicObjectCount);++idx, ++i) {
        RenderLoadingProgress(100,"Loading dynamic object [%d of %d]...",i,dynamicObjectCount);
        loadedInstances++;
        int entIdx = dynamics_parser.entries[i].index;
        if (entIdx >= MAX_ENTITIES) {DualLogError("Entity index when loading dynamic object %d was %d, exceeds max entity count of %d\n",(idx - startingIdx),entIdx,MAX_ENTITIES); continue; }
        
        instances[idx] = dynamics_parser.entries[i];
        instances[idx].modelIndex = entities[entIdx].modelIndex;
        if (instances[idx].modelIndex < MODEL_COUNT) renderableCount++;
        instances[idx].texIndex = entities[entIdx].texIndex;
        instances[idx].glowIndex = entities[entIdx].glowIndex;
        instances[idx].specIndex = entities[entIdx].specIndex;
        instances[idx].normIndex = entities[entIdx].normIndex;
        instances[idx].lodIndex = entities[entIdx].lodIndex;
        instances[idx].position.x += correctionX;
        instances[idx].position.y += correctionY;
        instances[idx].position.z += correctionZ;
        if (isTransparent(instances[idx].texIndex)) {
            transparentInstances[transparentInstancesHead] = idx;
            transparentInstancesHead++; // Already sized to INSTANCE_COUNT, no need for bounds check.
        }
    }

    DebugRAM("end of LoadLevelDynamicObjects");
    return 0;
}
