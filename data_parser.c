// data_parser.c - Load game definition files for mod, textures indices and metadata, model indices and metadata, level data, game save data
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include "voxen.h"
#include "entity.h"
#include "citadel_enumerations.h"
int malloc_trim(size_t pad); // #include <malloc.h>
typedef unsigned char stbi_uc;
stbi_uc *stbi_load_from_memory(stbi_uc const *buffer, int len   , int *x, int *y, int *channels_in_file, int desired_channels);

float voxelMinCenterX, voxelMinCenterZ;

uint32_t parse_numberu32(const char* str, const char* line, uint32_t lineNum) {
    if (str == NULL || *str == '\0') { DualLogError("Invalid input blank string, from line[%d]: %s\n", lineNum+1, line); return 0; }
    while (data_parser_isspace((unsigned char)*str)) str++;
    if (*str == '-') { DualLogError("Invalid input, negative not allowed (%s)\n      from line[%d]: %s\n", str, lineNum+1, line); return 0; }
    char* endptr;
    errno = 0;
    unsigned long val = strtoul(str, &endptr, 10);
    if (errno != 0 || val > UINT32_MAX) { DualLogError("Invalid input %s\n      from line[%d]: %s\n", str, lineNum+1, line); return 0; }
    return (uint32_t)val;
}

uint16_t parse_numberu16(const char* str, const char* line, uint32_t lineNum) {
    uint32_t retval = parse_numberu32(str, line, lineNum);
    if (retval > UINT16_MAX) { DualLogError("Value out of range for uint16_t: %u\n      from line[%d]: %s\n", retval, lineNum+1, line); return 0; }
    return (uint16_t)retval;
}

uint8_t parse_numberu8(const char* str, const char* line, uint32_t lineNum) {
    uint32_t retval = parse_numberu32(str, line, lineNum);
    if (retval > UINT8_MAX) { DualLogError("Value out of range for uint8_t: %u\n      from line[%d]: %s\n", retval, lineNum+1, line); return 0; }
    return (uint8_t)retval;
}

bool parse_bool(const char* str, const char* line, uint32_t lineNum) {
    uint32_t parseval = parse_numberu32(str, line, lineNum);
    if (parseval > 1) DualLogWarn("Loaded %u\n      in place where expected a boolean from line[%u]: %s\n",lineNum+1,line);
    return parseval > 0 ? true : false;
}

float parse_float(const char* str, const char* line, uint32_t lineNum) {
    if (str == NULL || *str == '\0') { DualLogError("Invalid float input blank string, from line[%d]: %s\n", lineNum+1, line); return 0.0f; }
    char* endptr;
    errno = 0;
    float val = strtof(str, &endptr);
    if (errno != 0) { DualLogError("Invalid float input %s\n      from line[%d]: %s\n      gave errno %u\n", str, lineNum+1, line, errno); return 0.0f; }
    if (*endptr != '\0') { DualLogError("Invalid float input %s\n      from line[%d]: %s\n      missing null terminator, *endptr: %u\n", str, lineNum+1, line, *endptr); return 0.0f; }
    if (endptr == str) { DualLogError("Invalid float input %s\n      from line[%d]: %s\n      end is equal to start\n", str, lineNum+1, line); return 0.0f; }
    return val;
}

void init_data_entry(ResourceEntry *entry) {
    entry->index = UINT16_MAX;
    entry->modelIndex = MODEL_IDX_MAX;
    entry->lodIndex  = MODEL_IDX_MAX;
    entry->texIndex  = MATERIAL_IDX_MAX;
    entry->glowIndex = MATERIAL_IDX_MAX;
    entry->specIndex = MATERIAL_IDX_MAX;
    entry->normIndex = MATERIAL_IDX_MAX;
    
    entry->doublesided = false;
    entry->transparent = false;
    entry->cardchunk = false;
    
    entry->collider = 0u;
    entry->colliderCenter.x = 0.0f;
    entry->colliderCenter.y = 0.0f;
    entry->colliderCenter.z = 0.0f;
    entry->colliderSize.x = 0.0f;
    entry->colliderSize.y = 0.0f;
    entry->colliderSize.z = 0.0f;
    entry->colliderMeshIndex = MODEL_IDX_MAX;
    entry->mass = 1.0f;
    entry->kinematic = false;
    entry->useGravity = false;
    entry->linearDrag = 0.0f;
    entry->angularDrag = 0.05f;
    entry->dynamicFriction = 0.6f;
    entry->staticFriction = 0.6f;
    entry->bounciness = 0.00f;
    entry->frictionCombine = PHYS_COMBINE_AVG;
    entry->bounceCombine = PHYS_COMBINE_AVG;
    
    entry->volume = 1.0f;
    
    entry->child0 = UINT16_MAX;
    entry->child0_offset.x = 0.0f;
    entry->child0_offset.y = 0.0f;
    entry->child0_offset.z = 0.0f;
    entry->child0_rotation.x = 0.0f;
    entry->child0_rotation.y = 0.0f;
    entry->child0_rotation.z = 0.0f;
    entry->child0_rotation.w = 1.0f;
    entry->child0_scale.x = 1.0f;
    entry->child0_scale.y = 1.0f;
    entry->child0_scale.z = 1.0f;
    
    entry->child1 = UINT16_MAX;
    entry->child1_offset.x = 0.0f;
    entry->child1_offset.y = 0.0f;
    entry->child1_offset.z = 0.0f;
    entry->child1_rotation.x = 0.0f;
    entry->child1_rotation.y = 0.0f;
    entry->child1_rotation.z = 0.0f;
    entry->child1_rotation.w = 1.0f;
    entry->child1_scale.x = 1.0f;
    entry->child1_scale.y = 1.0f;
    entry->child1_scale.z = 1.0f;
    entry->path[0] = '\0';
}

void allocate_entries(DataParser *parser, int32_t entry_count) {
    if (entry_count > MAX_ENTRIES) { DualLogWarn("\033[38;5;208mEntry count %d exceeds %d\033[0m\n", entry_count, MAX_ENTRIES); entry_count = MAX_ENTRIES; }
    
    if (entry_count > parser->capacity) {
        ResourceEntry *new_entries = realloc(parser->entries, entry_count * sizeof(ResourceEntry));  
        parser->entries = new_entries;
        for (int32_t i = parser->capacity; i < entry_count; ++i) init_data_entry(&parser->entries[i]);
        parser->capacity = entry_count;
    }
    parser->count = entry_count;
}

bool process_key_value(ResourceEntry *entry, const char *key, const char *value, const char *line, uint32_t lineNum) {
    if (!key || !value) { DualLogError("Invalid key-value pair at line %u: %s\n", lineNum, line); return false; }
    
    while (data_parser_isspace((unsigned char)*key)) key++;
    while (data_parser_isspace((unsigned char)*value)) value++;
    char trimmed_key[256];
    char trimmed_value[256];
    strncpy(trimmed_key, key, sizeof(trimmed_key) - 1);
    strncpy(trimmed_value, value, sizeof(trimmed_value) - 1);
    trimmed_key[sizeof(trimmed_key) - 1] = '\0';
    trimmed_value[sizeof(trimmed_value) - 1] = '\0';
    char *key_end = trimmed_key + strlen(trimmed_key) - 1;
    char *val_end = trimmed_value + strlen(trimmed_value) - 1;
    while (key_end > trimmed_key && data_parser_isspace((unsigned char)*key_end)) *key_end-- = '\0';
    while (val_end > trimmed_value && data_parser_isspace((unsigned char)*val_end)) *val_end-- = '\0';
    sanitize_utf8_ascii(trimmed_key);
    sanitize_utf8_ascii(trimmed_value);
    if (strncmp(trimmed_key, "chunk_", 6) == 0) {
        strncpy(entry->path, trimmed_key, sizeof(entry->path) - 1);
        entry->path[sizeof(entry->path) - 1] = '\0';
        return true;
    }
         if (strcmp(trimmed_key, "index") == 0)             entry->index = parse_numberu16(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "model") == 0)             entry->modelIndex = parse_numberu16(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "texture") == 0)           entry->texIndex = parse_numberu16(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "glowtexture") == 0)       entry->glowIndex = parse_numberu16(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "spectexture") == 0)       entry->specIndex = parse_numberu16(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "normtexture") == 0)       entry->normIndex = parse_numberu16(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "doublesided") == 0)       entry->doublesided = parse_bool(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "transparent") == 0)       entry->transparent = parse_bool(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "cardchunk") == 0)         entry->cardchunk = parse_bool(trimmed_value, line, lineNum);

    else if (strcmp(trimmed_key, "collider") == 0)          entry->collider = parse_numberu8(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "collider_centerx") == 0)  entry->colliderCenter.x = parse_float(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "collider_centery") == 0)  entry->colliderCenter.x = parse_float(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "collider_centerz") == 0)  entry->colliderCenter.x = parse_float(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "collider_sizex") == 0)    entry->colliderSize.x = parse_float(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "collider_sizey") == 0)    entry->colliderSize.y = parse_float(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "collider_sizez") == 0)    entry->colliderSize.z = parse_float(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "colliderMeshIndex") == 0) entry->colliderMeshIndex = parse_numberu16(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "mass") == 0)              entry->mass = parse_float(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "linearDrag") == 0)        entry->linearDrag = parse_float(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "angularDrag") == 0)       entry->angularDrag = parse_float(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "kinematic") == 0)         entry->kinematic = parse_bool(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "useGravity") == 0)        entry->useGravity = parse_bool(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "bounciness") == 0)        entry->bounciness = parse_float(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "dynamicFriction") == 0)   entry->dynamicFriction = parse_float(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "frictionCombine") == 0)   entry->frictionCombine = parse_numberu8(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "bounceCombine") == 0)     entry->bounceCombine = parse_numberu8(trimmed_value, line, lineNum);

    else if (strcmp(trimmed_key, "volume") == 0)            entry->volume = parse_float(trimmed_value, line, lineNum);
    
    else if (strcmp(trimmed_key, "child0") == 0)            entry->child0 = parse_numberu16(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "child0_offsetx") == 0)    entry->child0_offset.x = parse_float(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "child0_offsety") == 0)    entry->child0_offset.y = parse_float(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "child0_offsetz") == 0)    entry->child0_offset.z = parse_float(trimmed_value, line, lineNum);
    
    else if (strcmp(trimmed_key, "child1") == 0)            entry->child1 = parse_numberu16(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "child1_offsetx") == 0)    entry->child1_offset.x = parse_float(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "child1_offsety") == 0)    entry->child1_offset.y = parse_float(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "child1_offsetz") == 0)    entry->child1_offset.z = parse_float(trimmed_value, line, lineNum);
    
    else if (strcmp(trimmed_key, "modname") == 0)         { strncpy(global_modname, trimmed_value, sizeof(global_modname) - 1); global_modname[sizeof(global_modname) - 1] = '\0'; entry->index = 0; } // Game/Mod Definition enforces setting entry index to 0 here, at least one of these must do it.  The game definition only has one index, 0.
    else if (strcmp(trimmed_key, "levelcount") == 0)      numLevels = parse_numberu8(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "startlevel") == 0)      startLevel = parse_numberu8(trimmed_value, line, lineNum);
    else return false;
    return true;
}

bool read_token(FILE *file, char *token, size_t max_len, char delimiter, bool *is_comment, bool *is_eof, bool *is_newline, uint32_t *lineNum) {
    *is_comment = false;
    *is_eof = false;
    *is_newline = false;
    size_t pos = 0;
    int32_t c;
    while ((c = fgetc(file)) != EOF && data_parser_isspace(c) && c != '\n');
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

// Load Game/Mod Definition
void ParseGameData() {
    double start_time = get_time();
    const char* filename = "./Data/gamedata.txt";
    DualLog("Loading game definition from %s...",filename);    
    ResourceEntry entry;
    init_data_entry(&entry);
    FILE *gamedatfile = fopen(filename, "r");
    if (!gamedatfile) { DualLogError("\nCannot open %s\n", filename); DualLogError("Could not parse %s!\n", filename); exit(1); }
    
    uint32_t lineNum = 0;
    bool is_eof;
    while (!feof(gamedatfile)) {
        char token[1024];
        bool is_comment, is_newline;
        if (!read_token(gamedatfile, token, sizeof(token), ':', &is_comment, &is_eof, &is_newline, &lineNum)) {
            if (is_comment || is_newline) {
                if (is_newline) lineNum += 1;
                continue;
            }
        }
        
        char key[256];
        strncpy(key, token, sizeof(key) - 1);
        key[sizeof(key) - 1] = '\0';
        if (!read_token(gamedatfile, token, sizeof(token), '\n', &is_comment, &is_eof, &is_newline, &lineNum)) continue;
        
        process_key_value(&entry, key, token, key, lineNum);
        lineNum += 1;
    }
    
    fclose(gamedatfile);
    if (strcmp(global_modname, "Citadel") == 0) global_modIsCitadel = true;;
    DualLog(" loaded Game Definition for %s:: num levels: %d, start level: %d... took %f secs\n",global_modname,numLevels,startLevel,get_time() - start_time);
}

static bool ParseResourceData(DataParser *parser, FILE* file, const char *filename) {
    char line[1024];
    uint32_t lineNum = 0;
    int32_t entry_count = 0;
    uint32_t max_index = 0;
    while (fgets(line, sizeof(line), file)) { // First pass: count entries and find max index
        lineNum++;        
        char *start = line;
        while (data_parser_isspace((unsigned char)*start)) start++;
        char *end = start + strlen(start) - 1;
        while (end > start && data_parser_isspace((unsigned char)*end)) { *end = '\0'; end--; }
        if (*start == '\0' || (start[0] == '/' && start[1] == '/')) continue;
        if (line[0] == '#') { entry_count++; continue; }

        char *colon = strchr(start, ':');
        if (colon && strncmp(start, "index", colon - start) == 0) {
            char *value = colon + 1;
            while (data_parser_isspace((unsigned char)*value)) value++;
            uint32_t idx = parse_numberu32(value, line, lineNum);
            if (idx > max_index) max_index = idx;
       }
    }

    if (max_index == 0) { DualLogWarn("No entries found in %s\n", filename); fclose(file); return true; }

    allocate_entries(parser, max_index + 1);  // Second pass: parse entries
    rewind(file);
    ResourceEntry entry;
    init_data_entry(&entry);
    int32_t entries_stored = 0;
    lineNum = 0;
    while (fgets(line, sizeof(line), file)) {
        lineNum++;
        char *start = line;
        while (data_parser_isspace((unsigned char)*start)) start++;
        char *end = start + strlen(start) - 1;
        while (end > start && data_parser_isspace((unsigned char)*end)) { *end = '\0'; end--; }
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
            while (data_parser_isspace((unsigned char)*key)) key++;
            while (data_parser_isspace((unsigned char)*value)) value++;
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

bool parse_data_file(DataParser *parser, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) { DualLogError("Cannot open %s: %s\n", filename, strerror(errno)); return false; }
    return ParseResourceData(parser, file, filename);
}

bool isDoubleSided(uint32_t texIndexToCheck) {
    if (texIndexToCheck > loadedTextures) return false;
    return doubleSidedTexture[texIndexToCheck] > 0 ? 1 : 0;
}
bool isTransparent(uint32_t texIndexToCheck) {
    if (texIndexToCheck > loadedTextures) return false;
    return transparentTexture[texIndexToCheck] > 0 ? 1 : 0;    
}

#include "dynamic_culling.c" // Bit of a silly placement, but I'm optimizing for the bottleneck single compilation unit balancing across multiple .c files
