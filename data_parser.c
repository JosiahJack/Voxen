// data_parser.c - Load game definition files for mod, textures indices and metadata, model indices and metadata, level data, game save data
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include "voxen.h"
#include "citadel_enumerations.h"
int malloc_trim(size_t pad); // #include <malloc.h>
typedef unsigned char stbi_uc;
stbi_uc *stbi_load_from_memory(stbi_uc const *buffer, int len   , int *x, int *y, int *channels_in_file, int desired_channels);

DataParser entity_parser;

float correctionX, correctionY, correctionZ;
float correctionStaticSaveableX, correctionStaticSaveableY, correctionStaticSaveableZ;

DataParser lights_parser;
float correctionLightX, correctionLightY, correctionLightZ;
bool lightIsDynamic[LIGHT_COUNT];
uint16_t loadedLights = 0;

uint16_t invalidModelIndexCount;
uint16_t* modelTypeOffsetsOpaque = NULL;
uint16_t* modelTypeOffsetsDoubleSided = NULL;
uint16_t* modelTypeOffsetsTransparent = NULL;
uint16_t opaqueInstancesHead = 0;
uint16_t renderableCount = 0;
uint16_t loadedInstances = 0;
uint16_t startOfDoubleSidedInstances = INSTANCE_COUNT - 1;
uint16_t startOfTransparentInstances = INSTANCE_COUNT - 1;
uint16_t doubleSidedInstancesHead = 0;
uint16_t transparentInstancesHead = 0;

// Entities
Entity entities[MAX_ENTITIES]; // Global array of entity definitions
int32_t entityCount = 0;            // Number of entities loaded
uint16_t physHead = 0;

float voxelMinCenterX, voxelMinCenterZ;

static int data_parser_isspace(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r'; }

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
    
    entry->collider = COLLIDER_TYPE_NONE;
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

static inline void sanitize_utf8_ascii(char *s) {
    char *dst = s;
    while (*s) {
        if (!memcmp(s, "\xE2\x80\x90", 3) || !memcmp(s, "\xE2\x80\x91", 3) ||
            !memcmp(s, "\xE2\x80\x92", 3) || !memcmp(s, "\xE2\x80\x93", 3) ||
            !memcmp(s, "\xE2\x80\x94", 3) || !memcmp(s, "\xE2\x80\x95", 3) ||  // Added: Horizontal bar
            !memcmp(s, "\xE2\x88\x92", 3)) {
            dst[0] = '-'; dst++; s += 3; continue;
        }
        if (!memcmp(s, "\xC2\xAD", 2)) { dst[0] = '-'; dst++; s += 2; continue; }
        if (!memcmp(s, "\xE2\x80\x9C", 3) || !memcmp(s, "\xE2\x80\x9D", 3)) { dst[0] = '"'; dst++; s += 3; continue; }
        if (!memcmp(s, "\xE2\x80\x98", 3) || !memcmp(s, "\xE2\x80\x99", 3)) { dst[0] = '\''; dst++; s += 3; continue; }
        if (!memcmp(s, "\xEF\xBC\x8B", 3)) { dst[0] = '+'; dst++; s += 3; continue; }
        if (!memcmp(s, "\xEF\xBC\x8F", 3)) { dst[0] = '/'; dst++; s += 3; continue; }
        if (!memcmp(s, "\xEF\xBC\x88", 3)) { dst[0] = '('; dst++; s += 3; continue; }
        if (!memcmp(s, "\xEF\xBC\x89", 3)) { dst[0] = ')'; dst++; s += 3; continue; }
        if (!memcmp(s, "\xEF\xBC\x9A", 3)) { dst[0] = ':'; dst++; s += 3; continue; }
        if (!memcmp(s, "\xEF\xBC\x9B", 3)) { dst[0] = ';'; dst++; s += 3; continue; }
        if (!memcmp(s, "\xEF\xBC\x8C", 3)) { dst[0] = ','; dst++; s += 3; continue; }
        if (!memcmp(s, "\xEF\xBC\x8E", 3)) { dst[0] = '.'; dst++; s += 3; continue; }
        if (!memcmp(s, "\xEF\xBC\x8D", 3)) { dst[0] = '-'; dst++; s += 3; continue; }
        dst[0] = *s; dst++; s++;
    }
    *dst = '\0';
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
    DualLog(" loaded Game Definition for %s:: num levels: %d, start level: %d\n",global_modname,numLevels,startLevel);
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

//--------------------------------- Entities -------------------------------------
void LoadEntities(void) {
    double start_time = get_time();
    
    // Initialize parser with entity-specific keys
    if (!parse_data_file(&entity_parser, "./Data/entities.txt")) { DualLogError("Could not parse ./Data/entities.txt!\n"); exit(1); }
    
    entityCount = entity_parser.count;
    DualLog("Entity count found: %u\n",entityCount);
    if (entityCount > MAX_ENTITIES) { DualLogError("Too many entities in parser count %d, greater than %d!\n", entityCount, MAX_ENTITIES); exit(1); }
    if (entityCount == 0) { DualLogError("No entities found in entities.txt\n"); exit(1); }

    DualLog("Loading  %d entities...", entityCount);

    // Populate entities array
    for (int32_t i = 0; i < entityCount; i++) {
        if (entity_parser.entries[i].index == UINT16_MAX) continue;

        entities[i].index = entity_parser.entries[i].index;           // Different struct types, can't just wholesale assign.
        bool isCardChunk = entity_parser.entries[i].cardchunk;
        flag_enable(&entities[i].entflags, ENTFLAG_ACTIVE);
        flag_set(&entities[i].entflags,    ENTFLAG_CARDCHUNK, isCardChunk);
        flag_set(&entities[i].entflags,    ENTFLAG_GROUNDED, false);
        flag_set(&entities[i].entflags,    ENTFLAG_USEGRAVITY, entity_parser.entries[i].useGravity);
        flag_set(&entities[i].entflags,    ENTFLAG_KINEMATIC, entity_parser.entries[i].kinematic);
        flag_set(&entities[i].entflags,    ENTFLAG_RIGIDBODY, ConstIndexIsDynamicObject(entities[i].index));
        
        entities[i].modelIndex = entity_parser.entries[i].modelIndex;
        entities[i].texIndex = entity_parser.entries[i].texIndex;
        entities[i].glowIndex = entity_parser.entries[i].glowIndex;
        entities[i].specIndex = entity_parser.entries[i].specIndex;
        entities[i].normIndex = entity_parser.entries[i].normIndex;
        entities[i].lodIndex = isCardChunk ? GEOMETRY_LOD_CARD_MODEL_IDX : entity_parser.entries[i].lodIndex; // Generic LOD card

        entities[i].position.x = 0.0f;
        entities[i].position.y = 0.0f;
        entities[i].position.z = 0.0f;
        entities[i].rotation.x = 0.0f;
        entities[i].rotation.y = 0.0f;
        entities[i].rotation.z = 0.0f;
        entities[i].rotation.w = 1.0f;
        entities[i].scale.x = 1.0f;
        entities[i].scale.y = 1.0f;
        entities[i].scale.z = 1.0f;
        entities[i].velocity.x = 0.0f;
        entities[i].velocity.y = 0.0f;
        entities[i].velocity.z = 0.0f;
        entities[i].angularVelocity.x = 0.0f;
        entities[i].angularVelocity.y = 0.0f;
        entities[i].angularVelocity.z = 0.0f;
        
        entities[i].bodyState = BodyState_Standing;
        
        entities[i].collider = entity_parser.entries[i].collider;
        entities[i].colliderCenter.x = entity_parser.entries[i].colliderCenter.x;
        entities[i].colliderCenter.y = entity_parser.entries[i].colliderCenter.y;
        entities[i].colliderCenter.z = entity_parser.entries[i].colliderCenter.z;
        entities[i].colliderSize.x = entity_parser.entries[i].colliderSize.x;
        entities[i].colliderSize.y = entity_parser.entries[i].colliderSize.y;
        entities[i].colliderSize.z = entity_parser.entries[i].colliderSize.z;
        entities[i].colliderMeshIndex = entity_parser.entries[i].colliderMeshIndex;
        entities[i].mass = entity_parser.entries[i].mass;
        entities[i].linearDrag = entity_parser.entries[i].linearDrag;
        entities[i].angularDrag = entity_parser.entries[i].angularDrag;
        entities[i].inertia = 0.0f;
        entities[i].accumulatedForce.x = 0.0f;
        entities[i].accumulatedForce.y = 0.0f;
        entities[i].accumulatedForce.z = 0.0f;
        entities[i].accumulatedTorque.x = 0.0f;
        entities[i].accumulatedTorque.y = 0.0f;
        entities[i].accumulatedTorque.z = 0.0f;
        entities[i].dynamicFriction = entity_parser.entries[i].dynamicFriction;
        entities[i].staticFriction = entity_parser.entries[i].staticFriction;
        entities[i].bounciness = entity_parser.entries[i].bounciness;
        entities[i].frictionCombine = entity_parser.entries[i].frictionCombine;
        entities[i].bounceCombine = entity_parser.entries[i].bounceCombine;

        entities[i].volume = entity_parser.entries[i].volume;

        entities[i].child0 = entity_parser.entries[i].child0;
        entities[i].child0_offset.x = entity_parser.entries[i].child0_offset.x;
        entities[i].child0_offset.y = entity_parser.entries[i].child0_offset.y;
        entities[i].child0_offset.z = entity_parser.entries[i].child0_offset.z;
        entities[i].child0_rotation.x = entity_parser.entries[i].child0_rotation.x;
        entities[i].child0_rotation.y = entity_parser.entries[i].child0_rotation.y;
        entities[i].child0_rotation.z = entity_parser.entries[i].child0_rotation.z;
        entities[i].child0_rotation.w = entity_parser.entries[i].child0_rotation.w;
        entities[i].child0_scale.x = entity_parser.entries[i].child0_scale.x;
        entities[i].child0_scale.y = entity_parser.entries[i].child0_scale.y;
        entities[i].child0_scale.z = entity_parser.entries[i].child0_scale.z;
        
        entities[i].child1 = entity_parser.entries[i].child1;
        entities[i].child1_offset.x = entity_parser.entries[i].child1_offset.x;
        entities[i].child1_offset.y = entity_parser.entries[i].child1_offset.y;
        entities[i].child1_offset.z = entity_parser.entries[i].child1_offset.z;
        entities[i].child1_rotation.x = entity_parser.entries[i].child1_rotation.x;
        entities[i].child1_rotation.y = entity_parser.entries[i].child1_rotation.y;
        entities[i].child1_rotation.z = entity_parser.entries[i].child1_rotation.z;
        entities[i].child1_rotation.w = entity_parser.entries[i].child1_rotation.w;
        entities[i].child1_scale.x = entity_parser.entries[i].child1_scale.x;
        entities[i].child1_scale.y = entity_parser.entries[i].child1_scale.y;
        entities[i].child1_scale.z = entity_parser.entries[i].child1_scale.z;
    }

    DualLog(" took %f seconds\n", get_time() - start_time);
    DebugRAM("after loading all entities");
}

//----------------------------------- Level -----------------------------------
void AddInstance(uint16_t entIdx, uint16_t instanceIdx, uint32_t lineNum) {
    if (entIdx >= entityCount) { DualLogError("\nEntity index when loading level geometry object %d was %d, exceeds max entity count of %d\n",lineNum,entIdx,MAX_ENTITIES); exit(1); }
            
    instances[instanceIdx].index = entIdx;
    instances[instanceIdx].modelIndex = entities[entIdx].modelIndex;
    if (instances[instanceIdx].modelIndex < loadedModels) renderableCount++;
    instances[instanceIdx].texIndex = entities[entIdx].texIndex;
    instances[instanceIdx].glowIndex = entities[entIdx].glowIndex;
    if (instances[instanceIdx].glowIndex >= MATERIAL_IDX_MAX) instances[instanceIdx].glowIndex = BLACK_TEXTURE_IDX;
    instances[instanceIdx].specIndex = entities[entIdx].specIndex;
    if (instances[instanceIdx].specIndex >= MATERIAL_IDX_MAX) instances[instanceIdx].specIndex = BLACK_TEXTURE_IDX;
    instances[instanceIdx].normIndex = entities[entIdx].normIndex;
    if (instances[instanceIdx].normIndex >= MATERIAL_IDX_MAX) instances[instanceIdx].normIndex = BLACK_TEXTURE_IDX;
    instances[instanceIdx].lodIndex = entities[entIdx].lodIndex;
//     instances[instanceIdx].entflags = entities[entIdx].entflags; // Decided this was dangerous/error-prone, commented out in lieu of these explicit sets:
    flag_set(&instances[instanceIdx].entflags, ENTFLAG_CARDCHUNK,  entities[entIdx].entflags & ENTFLAG_CARDCHUNK);
    flag_set(&instances[instanceIdx].entflags, ENTFLAG_USEGRAVITY,  entities[entIdx].entflags & ENTFLAG_USEGRAVITY);
    flag_set(&instances[instanceIdx].entflags, ENTFLAG_KINEMATIC,  entities[entIdx].entflags & ENTFLAG_KINEMATIC);
    flag_set(&instances[instanceIdx].entflags, ENTFLAG_RIGIDBODY,  entities[entIdx].entflags & ENTFLAG_RIGIDBODY);
    instances[instanceIdx].mass = entities[entIdx].mass > 0.0f ? entities[entIdx].mass : 1.0f; // Nonzero fallback.
    instances[instanceIdx].linearDrag = entities[entIdx].linearDrag > 0.0f ? entities[entIdx].linearDrag : 0.0f;
    instances[instanceIdx].angularDrag = entities[entIdx].angularDrag > 0.0f ? entities[entIdx].angularDrag : 0.05f;

    if (entIdx != 755 && entIdx != 590) { // Adjusted for in the level data directly, no correction.
        if (ConstIndexIsDoor(entIdx)) {
            instances[instanceIdx].position.x += correctionX + 0.6001f;
            instances[instanceIdx].position.y += correctionY - 0.5681f;
            instances[instanceIdx].position.z += correctionZ - 0.905f;
//         } else if (ConstIndexIsStaticObjectSaveable(entIdx)) {
//             instances[instanceIdx].position.x += 5.12f;
//             instances[instanceIdx].position.y += 48.2291f;
//             instances[instanceIdx].position.z += -15.36f;
        } else {
            instances[instanceIdx].position.x += correctionX;   
            instances[instanceIdx].position.y += correctionY;
            instances[instanceIdx].position.z += correctionZ;
        }
    } else {
        DualLog("entity with entIdx %u located at %f, %f, %f\n",entIdx,instances[instanceIdx].position.x,instances[instanceIdx].position.y,instances[instanceIdx].position.z);
    }

    loadedInstances++;
}

void AddChild0(uint16_t child, uint16_t parent, uint16_t entIdx, int32_t* instanceIdx, uint32_t lineNum) {
    if (child == UINT16_MAX) return;
    
    (*instanceIdx)++; // Increment head of the list an extra time for the child entity
    AddInstance(child, *instanceIdx, lineNum);
    instances[*instanceIdx].index = child;
    instances[*instanceIdx].position.x = instances[parent].position.x + entities[entIdx].child0_offset.x;
    instances[*instanceIdx].position.y = instances[parent].position.y + entities[entIdx].child0_offset.y;
    instances[*instanceIdx].position.z = instances[parent].position.z + entities[entIdx].child0_offset.z;
    instances[*instanceIdx].scale.x = instances[parent].scale.x * entities[entIdx].child0_scale.x;
    instances[*instanceIdx].scale.y = instances[parent].scale.y * entities[entIdx].child0_scale.y;
    instances[*instanceIdx].scale.z = instances[parent].scale.z * entities[entIdx].child0_scale.z;
}

void AddChild1(uint16_t child, uint16_t parent, uint16_t entIdx, int32_t* instanceIdx, uint32_t lineNum) {
    if (child == UINT16_MAX) return;

    (*instanceIdx)++; // Increment head of the list an extra time for the child entity
    AddInstance(child, *instanceIdx, lineNum);
    instances[*instanceIdx].index = child;
    instances[*instanceIdx].position.x = instances[parent].position.x + entities[entIdx].child1_offset.x;
    instances[*instanceIdx].position.y = instances[parent].position.y + entities[entIdx].child1_offset.y;
    instances[*instanceIdx].position.z = instances[parent].position.z + entities[entIdx].child1_offset.z;
    instances[*instanceIdx].scale.x = instances[parent].scale.x * entities[entIdx].child1_scale.x;
    instances[*instanceIdx].scale.y = instances[parent].scale.y * entities[entIdx].child1_scale.y;
    instances[*instanceIdx].scale.z = instances[parent].scale.z * entities[entIdx].child1_scale.z;
}

void LoadLevel(uint8_t curlevel) {
    currentLevel = curlevel;
    DebugRAM("start of LoadLevel");
    double start_time = get_time();
    for (uint16_t idx = START_INDEX_LEVEL_INSTANCES;idx<INSTANCE_COUNT;idx++) { // Start AFTER player indices and NULLENT
        instances[idx].modelIndex = MODEL_IDX_MAX;
        instances[idx].texIndex = instances[idx].glowIndex = instances[idx].specIndex = instances[idx].normIndex = MATERIAL_IDX_MAX;
        instances[idx].lodIndex = UINT16_MAX;
        instances[idx].scale.x = instances[idx].scale.y = instances[idx].scale.z = 1.0f; // Default scale
        instances[idx].rotation.x = instances[idx].rotation.y = instances[idx].rotation.z = 0.0f; instances[idx].rotation.w = 1.0f; // Quaternion identity (other fields left at 0.0f)
        dirtyInstances[idx] = true;
    }

    memset(modelMatrices, 0, INSTANCE_COUNT * 16 * sizeof(float)); // Matrix4x4 = 16
    if (curlevel >= numLevels) { DualLogError("Cannot load world geometry, level number %d out of bounds 0 to %d\n",curlevel,numLevels - 1); exit(1); }
    
    char filename[20]; // Minimum size for 0 through 13.
    snprintf(filename, sizeof(filename), "./Data/level%d.txt", curlevel);
    FILE *file = fopen(filename, "r");
    if (!file) { DualLogError("Cannot open %s: %s\n", filename, strerror(errno)); exit(1); }

    int32_t lineNum = -1; // Start at 0 on first loop iteration, as it needs to iterate before each blank or commented line skip
    int32_t instanceIdx = PLAYER2;
    int32_t lightsIdx = -1;
    size_t lineLengthMax = 81920; 
    char lineSpace[lineLengthMax];
    char* line = &lineSpace[0];
    char firstKeyCheck[11];
    char initialLine[lineLengthMax];
    GetLevel_Transform_Offsets(curlevel,&correctionX,&correctionY,&correctionZ);
    GetLevel_LightsStaticImmutable_ContainerOffsets(curlevel,&correctionLightX,&correctionLightY,&correctionLightZ);
    while (fgets(lineSpace, lineLengthMax, file)) {
        size_t len = strlen(lineSpace);
        while (len && (lineSpace[len - 1] == '\n' || lineSpace[len - 1] == '\r'))
        lineSpace[--len] = '\0';

        line = lineSpace;
        snprintf(initialLine, sizeof(initialLine), "%s", line);
        memcpy(firstKeyCheck,line,10); firstKeyCheck[10] = '\0';
        lineNum++;
        
        bool isLight = true;
        if (strcmp(firstKeyCheck, "constIndex") == 0) isLight = false;  // constIndex specified indicating this is a real entity?
        if (isLight) {
            lightsIdx++;
            lightIsDynamic[lightsIdx] = false;
            if (lightsIdx >= LIGHT_COUNT) { DualLogError("Too many lights %u in level%d.txt!\n",lightsIdx,curlevel); exit(1); }
        } else {
            instanceIdx++;
            if (instanceIdx >= INSTANCE_COUNT) { DualLogError("Too many instances %u in level%d.txt!\n",instanceIdx,curlevel); exit(1); }
        }
        
        int32_t litIdx = lightsIdx * LIGHT_DATA_SIZE;
        uint8_t lightType = 0u; // Point
        while(line[0] != '\0') {
            // Guaranteed no leading whitespaces,k comments, or blank lines, so don't bother
            char* pipe = strchr(line,'|');
            char* kvString = line; // key:value pair before the pipe as a string
            if (pipe) {
                *pipe = '\0';          // Split string at the pipe
                line = pipe + 1;       // Point to rest of the line after the pipe
            } else { // Else this is the last string after the last pipe with last kv pair
                line += strlen(line);
            }
           
            if (kvString[0] == '\0' || strchr(kvString, ':') == NULL) continue;
            
            char *colon = strchr(kvString, ':');
            if (colon[1] == '\0') continue; // Don't care about the name of the Unity gameobject from when this data used to be over there.  Need to skip this in the middle, but this also handles the very end
            
            *colon = '\0';           // Split string at the colon
            char *key = kvString;    // Assign key to before colon
            char *value = colon + 1; // Assing value to after colon
            if (!key || !value) { DualLogError("Invalid key-value pair at line %u (as viewed by text editor): %s\n", lineNum+1, initialLine); exit(1); }
            
            while (data_parser_isspace((unsigned char)*key)) key++;
            while (data_parser_isspace((unsigned char)*value)) value++;
            char trimmed_key[8192];
            char trimmed_value[8192];
            snprintf(trimmed_key, sizeof(trimmed_key), "%s", key);
            snprintf(trimmed_value, sizeof(trimmed_value), "%s", value);
            trimmed_key[sizeof(trimmed_key) - 1] = '\0';
            trimmed_value[sizeof(trimmed_value) - 1] = '\0';
            char *key_end = trimmed_key + strlen(trimmed_key) - 1;
            char *val_end = trimmed_value + strlen(trimmed_value) - 1;
            while (key_end > trimmed_key && data_parser_isspace((unsigned char)*key_end)) *key_end-- = '\0';
            while (val_end > trimmed_value && data_parser_isspace((unsigned char)*val_end)) *val_end-- = '\0';
            sanitize_utf8_ascii(trimmed_key);
            sanitize_utf8_ascii(trimmed_value);
            if (isLight) {
                     if (strcmp(trimmed_key, "localPosition.x") == 0) lights[litIdx + LIGHT_DATA_OFFSET_POSX] = parse_float(trimmed_value, initialLine, lineNum) + correctionLightX;
                else if (strcmp(trimmed_key, "localPosition.y") == 0) lights[litIdx + LIGHT_DATA_OFFSET_POSY] = parse_float(trimmed_value, initialLine, lineNum) + correctionLightY;
                else if (strcmp(trimmed_key, "localPosition.z") == 0) lights[litIdx + LIGHT_DATA_OFFSET_POSZ] = parse_float(trimmed_value, initialLine, lineNum) + correctionLightZ;
                else if (strcmp(trimmed_key, "localRotation.x") == 0) lights[litIdx + LIGHT_DATA_OFFSET_SPOTDIRX] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localRotation.y") == 0) lights[litIdx + LIGHT_DATA_OFFSET_SPOTDIRY] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localRotation.z") == 0) lights[litIdx + LIGHT_DATA_OFFSET_SPOTDIRZ] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localRotation.w") == 0) lights[litIdx + LIGHT_DATA_OFFSET_SPOTDIRW] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "intensity") == 0)       lights[litIdx + LIGHT_DATA_OFFSET_INTENSITY] = parse_float(trimmed_value, initialLine, lineNum) * 0.4; // Adjustment, globally applied from Citadel's Unity to Custom Game Engine (Voxen) conversion.
                else if (strcmp(trimmed_key, "range") == 0)           lights[litIdx + LIGHT_DATA_OFFSET_RANGE] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "spotAngle") == 0)       lights[litIdx + LIGHT_DATA_OFFSET_SPOTANG] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "type") == 0) {
                    if ((strcmp(trimmed_value, "Spot") == 0)) lightType = 1u;
                    else if ((strcmp(trimmed_value, "Directional") == 0)) lightType = 2u;
                }
                else if (strcmp(trimmed_key, "color.r") == 0)         lights[litIdx + LIGHT_DATA_OFFSET_R] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "color.g") == 0)         lights[litIdx + LIGHT_DATA_OFFSET_G] = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "color.b") == 0)         lights[litIdx + LIGHT_DATA_OFFSET_B] = parse_float(trimmed_value, initialLine, lineNum);
            } else {
                     if (strcmp(trimmed_key, "index") == 0)           instances[instanceIdx].index = parse_numberu16(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "constIndex") == 0)      instances[instanceIdx].index = parse_numberu16(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localPosition.x") == 0) instances[instanceIdx].position.x = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localPosition.y") == 0) instances[instanceIdx].position.y = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localPosition.z") == 0) instances[instanceIdx].position.z = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localRotation.x") == 0) instances[instanceIdx].rotation.x = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localRotation.y") == 0) instances[instanceIdx].rotation.y = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localRotation.z") == 0) instances[instanceIdx].rotation.z = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localRotation.w") == 0) instances[instanceIdx].rotation.w = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localScale.x") == 0)    instances[instanceIdx].scale.x = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localScale.y") == 0)    instances[instanceIdx].scale.y = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "localScale.z") == 0)    instances[instanceIdx].scale.z = parse_float(trimmed_value, initialLine, lineNum);
                else if (strcmp(trimmed_key, "go.activeSelf") == 0)   flag_set(&instances[instanceIdx].entflags, ENTFLAG_ACTIVE, parse_bool(trimmed_value, initialLine, lineNum));
            }
        }
        
        if (isLight) {
            loadedLights++;
            lightsRangeSquared[lightsIdx] = lights[litIdx + LIGHT_DATA_OFFSET_RANGE] * lights[litIdx + LIGHT_DATA_OFFSET_RANGE];
            if (lightsIdx == 817) {
                testLight_x = lights[litIdx + LIGHT_DATA_OFFSET_POSX];
                testLight_y = lights[litIdx + LIGHT_DATA_OFFSET_POSY];
                testLight_z = lights[litIdx + LIGHT_DATA_OFFSET_POSZ];
                lightIsDynamic[lightsIdx] = true;
            }
            
            if (lightType == 1) {
                if (lights[litIdx + LIGHT_DATA_OFFSET_SPOTANG] < 5.0f) DualLogWarn("Light %d on line %d loaded with spotAngle less than 5deg but was marked as spotlight type!\n",lightsIdx,lineNum);
            } else if (lightType == 2) {
                // TODO: Handle directional lights for cyberspace
                lights[litIdx + LIGHT_DATA_OFFSET_SPOTANG] = 0.0f; // Force to not be a spot light
            } else {
                lights[litIdx + LIGHT_DATA_OFFSET_SPOTANG] = 0.0f; // Force to not be a spot light
            }
        } else {
            uint16_t parent = instanceIdx;
            uint16_t entIdx = instances[parent].index;
            AddInstance(entIdx, parent, lineNum);
            AddChild0(entities[entIdx].child0, parent, entIdx, &instanceIdx, lineNum);
            AddChild1(entities[entIdx].child1, parent, entIdx, &instanceIdx, lineNum);
        }
    }

    fclose(file);
    
    // Set Fog
    switch(curlevel) {
        case  0: fogColorR = 0.3207547f;  fogColorG = 0.29200783f;  fogColorB = 0.29200783f;  fogBaseDensityForLevel = 0.07f;  break;
        case  1: fogColorR = 0.34509805f; fogColorG = 0.38431373f;  fogColorB = 0.49019608f;  fogBaseDensityForLevel = 0.055f; break;
        case  2: fogColorR = 0.47058824f; fogColorG = 0.3882353f;   fogColorB = 0.3928334f;   fogBaseDensityForLevel = 0.05f;  break;
        case  3: fogColorR = 0.32941177f; fogColorG = 0.29411766f;  fogColorB = 0.2509804f;   fogBaseDensityForLevel = 0.065f; break;
        case  4: fogColorR = 0.3882353f;  fogColorG = 0.452415f;    fogColorB = 0.47058824f;  fogBaseDensityForLevel = 0.075f; break;
        case  5: fogColorR = 0.3882353f;  fogColorG = 0.4117647f;   fogColorB = 0.47058824f;  fogBaseDensityForLevel = 0.03f;  break;
        case  6: fogColorR = 0.3f;        fogColorG = 0.24f;        fogColorB = 0.33f;        fogBaseDensityForLevel = 0.07f;  break;
        case  7: fogColorR = 0.38679248f; fogColorG = 0.3471719f;   fogColorB = 0.3302332f;   fogBaseDensityForLevel = 0.07f;  break;
        case  8: fogColorR = 0.44708973f; fogColorG = 0.45681614f;  fogColorB = 0.4811321f;   fogBaseDensityForLevel = 0.04f;  break;
        case  9: fogColorR = 0.4056604f;  fogColorG = 0.3992963f;   fogColorB = 0.36930403f;  fogBaseDensityForLevel = 0.05f;  break;
        case 10: fogColorR = 0.48235294f; fogColorG = 0.58431375f;  fogColorB = 0.5176471f;   fogBaseDensityForLevel = 0.04f;  break;
        case 11: fogColorR = 0.52872473f; fogColorG = 0.58431375f;  fogColorB = 0.48235294f;  fogBaseDensityForLevel = 0.04f;  break;
        case 12: fogColorR = 0.48235294f; fogColorG = 0.58431375f;  fogColorB = 0.5176471f;   fogBaseDensityForLevel = 0.05f;  break;
        case 13: fogColorR = 0.0f;        fogColorG = 0.0f;         fogColorB = 0.0f;         fogBaseDensityForLevel = 0.005f; break;
    }

    fogBaseDensityForLevel *= 4.0f; // Global multiplier to get it to look similar to Unity's
    SetFog();
    DualLog("Loaded %d geometry chunks and %u static lights for Level %d... took %f seconds\n", loadedInstances, loadedLights, curlevel, get_time() - start_time);
    DebugRAM("end of LoadLevel");
}

void SortInstances(void) {
    double start_time = get_time();
    DualLog("Sorting instances...");
    modelTypeCountsOpaque = calloc(loadedModels,sizeof(uint16_t)); // Zero out all arrays and counters
    modelTypeCountsDoubleSided = calloc(loadedModels,sizeof(uint16_t));
    modelTypeCountsTransparent = calloc(loadedModels,sizeof(uint16_t));
    modelTypeOffsetsOpaque = calloc(loadedModels,sizeof(uint16_t));
    modelTypeOffsetsDoubleSided = calloc(loadedModels,sizeof(uint16_t));
    modelTypeOffsetsTransparent = calloc(loadedModels,sizeof(uint16_t));
    uint16_t opaqueInstances[INSTANCE_COUNT] = {0};
    uint16_t doubleSidedInstances[INSTANCE_COUNT] = {0};
    uint16_t transparentInstances[INSTANCE_COUNT] = {0};
    opaqueInstancesHead = 0;
    doubleSidedInstancesHead = 0;
    transparentInstancesHead = 0;
    invalidModelIndexCount = 0;

    // Step 1: Categorize instances and count model types per category
    for (uint32_t i = START_INDEX_LEVEL_INSTANCES; i < loadedInstances; i++) { // Skip player instances and NULLENT by starting at 3.
        if (instances[i].texIndex >= loadedTextures && instances[i].texIndex != MATERIAL_IDX_MAX) { DualLogError("Invalid texIndex %u for instance %u\n", instances[i].texIndex, i); invalidModelIndexCount++; continue; }
        if (instances[i].modelIndex >= loadedModels || instances[i].modelIndex == UINT16_MAX) { invalidModelIndexCount++; continue; }
        if (instances[i].index >= MAX_ENTITIES) { DualLogError("Invalid entity index %u for instance %u\n", instances[i].index, i); invalidModelIndexCount++; continue; }

        bool is_double_sided = isDoubleSided(instances[i].texIndex) || instances[i].scale.x < 0.0f || instances[i].scale.y < 0.0f || instances[i].scale.z < 0.0f;
        if (isTransparent(instances[i].texIndex)) {
            if (transparentInstancesHead >= INSTANCE_COUNT) { DualLogError("Transparent instances overflow at index %u\n", i); invalidModelIndexCount++; continue; }

            transparentInstances[transparentInstancesHead++] = i;
            modelTypeCountsTransparent[instances[i].modelIndex]++;
        } else if (is_double_sided) {
            if (doubleSidedInstancesHead >= INSTANCE_COUNT) { DualLogError("Double-sided instances overflow at index %u\n", i); invalidModelIndexCount++; continue; }

            doubleSidedInstances[doubleSidedInstancesHead++] = i;
            modelTypeCountsDoubleSided[instances[i].modelIndex]++;
        } else {
            if (opaqueInstancesHead >= INSTANCE_COUNT) { DualLogError("Opaque instances overflow at index %u\n", i); invalidModelIndexCount++; continue; }

            opaqueInstances[opaqueInstancesHead++] = i;
            modelTypeCountsOpaque[instances[i].modelIndex]++;
        }
    }

    // Step 2: Compute offsets
    uint16_t currentOffset = START_INDEX_LEVEL_INSTANCES;
    for (uint16_t i = 0; i < loadedModels; i++) {
        modelTypeOffsetsOpaque[i] = currentOffset;
        currentOffset += modelTypeCountsOpaque[i];
    }
    startOfDoubleSidedInstances = currentOffset;
    for (uint16_t i = 0; i < loadedModels; i++) {
        modelTypeOffsetsDoubleSided[i] = currentOffset;
        currentOffset += modelTypeCountsDoubleSided[i];
    }
    
    startOfTransparentInstances = currentOffset;
    for (uint16_t i = 0; i < loadedModels; i++) {
        modelTypeOffsetsTransparent[i] = currentOffset;
        currentOffset += modelTypeCountsTransparent[i];
    }

    if ((uint32_t)(startOfTransparentInstances + transparentInstancesHead) > (uint32_t)(loadedInstances - invalidModelIndexCount)) { DualLogError("Transparent range overflow: start %u, head %u, limit %u\n", startOfTransparentInstances, transparentInstancesHead, loadedInstances - invalidModelIndexCount); exit(1); }

    // Step 3: Reorder instances
    Entity tempInstances[INSTANCE_COUNT];
    memcpy(tempInstances, instances, loadedInstances * sizeof(Entity));
    uint16_t targetIdx = START_INDEX_LEVEL_INSTANCES;
    
    // Copy opaque instances
    for (uint16_t modelIdx = 0; modelIdx < loadedModels; modelIdx++) {
        for (uint16_t j = 0; j < opaqueInstancesHead; j++) {
            uint16_t i = opaqueInstances[j];
            if (i >= START_INDEX_LEVEL_INSTANCES) {
                if (tempInstances[i].modelIndex == modelIdx) {
                    if (targetIdx >= startOfDoubleSidedInstances) { DualLogError("Opaque instance overflow at modelIdx %u, index %u, targetIdx %u\n", modelIdx, i, targetIdx); exit(1); }
                    
                    instances[targetIdx] = tempInstances[i];
                    targetIdx++;
                }
            }
        }
    }

    // Copy double-sided instances
    for (uint16_t modelIdx = 0; modelIdx < loadedModels; ++modelIdx) {
        for (uint16_t j = 0; j < doubleSidedInstancesHead; j++) {
            uint16_t i = doubleSidedInstances[j];
            if (i >= START_INDEX_LEVEL_INSTANCES) {
                if (tempInstances[i].modelIndex == modelIdx) {
                    if (targetIdx >= startOfTransparentInstances) { DualLogError("Double-sided instance overflow at modelIdx %u, index %u, targetIdx %u\n", modelIdx, i, targetIdx); exit(1); }
                    
                    instances[targetIdx] = tempInstances[i];
                    targetIdx++;
                }
            }
        }
    }

    // Copy transparent instances
    for (uint16_t modelIdx = 0; modelIdx < loadedModels; ++modelIdx) {
        for (uint16_t j = 0; j < transparentInstancesHead; j++) {
            uint16_t i = transparentInstances[j];
            if (i >= START_INDEX_LEVEL_INSTANCES) {
                if (tempInstances[i].modelIndex == modelIdx) {
                    if (targetIdx >= loadedInstances - invalidModelIndexCount) { DualLogError("Transparent instance overflow at modelIdx %u, index %u, targetIdx %u\n", modelIdx, i, targetIdx); exit(1); }
                    
                    instances[targetIdx] = tempInstances[i];
                    targetIdx++;
                }
            }
        }
    }
    
    // Put all the invisible entities at the end of the list now
    for (uint16_t i = 0; i < loadedInstances; ++i) {
        if (tempInstances[i].modelIndex > loadedModels) {
            instances[targetIdx] = tempInstances[i];
            targetIdx++;
        }
    }

    // Update cellIndexForInstance
    for (uint16_t i = START_INDEX_LEVEL_INSTANCES; i < loadedInstances; ++i) { // Skip player index and start at 3?
        float x = instances[i].position.x;
        float z = instances[i].position.z;
        int32_t cellX = (int32_t)floorf((x - worldMin_x) / WORLDCELL_WIDTH_F);
        int32_t cellZ = (int32_t)floorf((z - worldMin_z) / WORLDCELL_WIDTH_F);
        cellX = clamp(cellX, 0, 63);
        cellZ = clamp(cellZ, 0, 63);
        cellIndexForInstance[i] = cellZ * 64 + cellX;
    }
    
    // Perform post-sort registrations:
    loadedAmbients = 0;
    for (uint16_t i = opaqueInstancesHead + doubleSidedInstancesHead + transparentInstancesHead; i<loadedInstances;++i) {
        uint16_t entIdx = instances[i].index;
        if (ConstIndexIsAmbient(entIdx)) {
            ambientRegistry[loadedAmbients] = i;
            loadedAmbients++;
            if (loadedAmbients >= MAX_AMBIENT_NOISES) { DualLogError("%u exceeded max number of ambient noises %u!\n",loadedAmbients,MAX_AMBIENT_NOISES); exit(1); }
            
            instances[i].volume = entities[entIdx].volume * 0.5f;
        }
    }

    DualLog("loaded %d ambient noises for Level %d...", loadedAmbients, currentLevel);
    DualLog(" took %f secs\n", get_time() - start_time);
    DualLog("Total opaque instances: %u, double-sided: %u, transparent: %u, invisible: %u\n", opaqueInstancesHead, doubleSidedInstancesHead, transparentInstancesHead, invalidModelIndexCount);
}
//=============================================================================
// Culling System
uint8_t gridCellStates[ARRSIZE];
uint32_t precomputedVisibleCellsFromHere[PRECOMPUTED_VISIBILITY_SIZE];
uint32_t cellIndexForInstance[INSTANCE_COUNT];
uint16_t cellIndexForLight[LIGHT_COUNT];
uint16_t cellIndexForLightX[LIGHT_COUNT];
uint16_t cellIndexForLightZ[LIGHT_COUNT];
uint16_t playerCellIdx = 0u;
uint16_t playerCellIdx_x = 0u; uint16_t playerCellIdx_y = 0u; uint16_t playerCellIdx_z = 0u;
uint16_t numCellsVisible = 0u;
float worldMin_x = 0.0f; float worldMin_z = 0.0f;

bool get_cull_bit(const uint32_t* arr, size_t idx) {
    size_t word = idx / 32;
    size_t bit = idx % 32;
    return ((arr[word] & (1U << bit)) != 0);
}

static inline void set_cull_bit(uint32_t* arr, size_t idx, bool val) {
    size_t word = idx / 32;
    size_t bit = idx % 32;
    if (val) {
        arr[word] |= (1U << bit);
    } else {
        arr[word] &= ~(1U << bit);
    }
}

void PutChunksInCells() {
    uint16_t x,z;
    uint16_t cellIdx;
    for (uint16_t c=3; c < INSTANCE_COUNT; ++c) { // Start after player instances and NULLENT
        
        PosToCellCoords(instances[c].position.x, instances[c].position.z, &x, &z);
        cellIdx = (z * WORLDX) + x;
        if (!(gridCellStates[cellIdx] & CELL_OPEN)) cellIdx = 0;
        cellIndexForInstance[c] = (uint32_t)cellIdx;
    }
}

void PutMeshesInCells(int type) {
    int count = 0;
    switch(type) {
        case 5: count = LIGHT_COUNT; break; // Lights
    }
    for (int index=0;index<count;index++) {
        uint16_t x,z;
        switch(type) {
            case 5: // Lights
                int lightIdx = (index * LIGHT_DATA_SIZE);
                PosToCellCoords(lights[lightIdx + LIGHT_DATA_OFFSET_POSX],lights[lightIdx + LIGHT_DATA_OFFSET_POSZ], &x, &z);
                cellIndexForLight[index] = (z * WORLDX) + x;
                cellIndexForLightX[index] = x;
                cellIndexForLightZ[index] = z;
                break;
        }
    }
}

void DetermineClosedEdges() {
    DebugRAM("Start of DetermineClosedEdges");
    size_t maxFileSize = 500000; // 0.5MB
    uint8_t* file_buffer = malloc(maxFileSize);
    FILE* fp;
    size_t file_size, read_size;
    int32_t wpng, hpng, channels;
    
    // ------------------- Open Cells ------------------
    char filename2[256];
    sprintf(filename2,"./Data/worldcellopen_%d.png",currentLevel);
    fp = fopen(filename2, "rb");
    if (!fp) { DualLogError("Failed to open %s\n", filename2); exit(1); }
    
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    if (file_size > maxFileSize) { DualLogError("PNG file %s too large (%zu bytes)\n", filename2, file_size); exit(1); }
    
    fseek(fp, 0, SEEK_SET);
    read_size = fread(file_buffer, 1, file_size, fp);
    fclose(fp);
    if (read_size != file_size) { DualLogError("Failed to read %s\n", filename2); exit(1); }
    unsigned char* openPixels = stbi_load_from_memory(file_buffer, file_size, &wpng, &hpng, &channels, 4); // I handmade them, well what can ya do
	if (!openPixels) { DualLogError("Failed to read %s for culling open cells\n", filename2); exit(1); }
 
    unsigned char openData_r, openData_g, openData_b;
    uint16_t totalOpenCells = 0;
    for (int32_t x=0;x<WORLDX;++x) {
        for (int32_t z=0;z<WORLDZ;++z) {
            int32_t cellIdx = (z * WORLDX) + x;
            gridCellStates[cellIdx] &= ~CELL_OPEN;
            int32_t flippedZ = (WORLDZ - 1) - z; // Flip z to match Unity's bottom-left origin for Texture2D vs stbi_load's top-left
            int32_t pixelIdx = (x + (flippedZ * WORLDX)) * 4; // 4 channels
            openData_r = openPixels[pixelIdx + 0];
            openData_g = openPixels[pixelIdx + 1];
            openData_b = openPixels[pixelIdx + 2];
            if (openData_r > 0 || openData_g > 0 || openData_b > 0) {
                gridCellStates[cellIdx] |= CELL_OPEN;
                totalOpenCells++;
            } else {
                gridCellStates[cellIdx] |= CELL_CLOSEDNORTH | CELL_CLOSEDEAST | CELL_CLOSEDSOUTH | CELL_CLOSEDWEST; // Also force close the edges for closed cells even if above edges image said tweren't closed edges.
            }
        }
    }

    gridCellStates[0] |= CELL_OPEN; // Force the fallback error cell to be open (forced visible later, open is static, visible is transient)
    free(openPixels);
    malloc_trim(0);
    
    // ------------------- Closed Edges ------------------    
    char filename[256];
    sprintf(filename,"./Data/worldedgesclosed_%d.png",currentLevel);

    fp = fopen(filename, "rb");
    if (!fp) { DualLogError("Failed to open %s\n", filename); exit(1); }
    
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    if (file_size > maxFileSize) { DualLogError("PNG file %s too large (%zu bytes)\n", filename, file_size); exit(1); }
    
    fseek(fp, 0, SEEK_SET);
    read_size = fread(file_buffer, 1, file_size, fp);
    fclose(fp);
    if (read_size != file_size) { DualLogError("Failed to read %s\n", filename); exit(1); }

    unsigned char* edgePixels = stbi_load_from_memory(file_buffer, file_size, &wpng, &hpng, &channels, 4); // I handmade them, well what can ya do
    if (!edgePixels) { DualLogError("Failed to read %s for culling closed edges\n", filename); exit(1); }

    unsigned char closedData_r, closedData_g, closedData_b, closedData_a;
    uint16_t closedCountNorth = 0, closedCountSouth = 0, closedCountEast = 0, closedCountWest = 0;
    for (int32_t x=0;x<WORLDX;x++) {
        for (int32_t z=0;z<WORLDZ;z++) {
            int32_t cellIdx = (z * WORLDX) + x;
            gridCellStates[cellIdx] &= ~(CELL_CLOSEDNORTH | CELL_CLOSEDEAST | CELL_CLOSEDSOUTH | CELL_CLOSEDWEST); // Mark all edges not closed
            int32_t flippedZ = (WORLDZ - 1) - z; // Flip z to match Unity's bottom-left origin for Texture2D vs stbi_load's top-left
            int32_t pixelIdx = (x + (flippedZ * WORLDX)) * 4; // 4 channels
            closedData_r = edgePixels[pixelIdx + 0];
            closedData_g = edgePixels[pixelIdx + 1];
            closedData_b = edgePixels[pixelIdx + 2];
            closedData_a = edgePixels[pixelIdx + 3];
            if (closedData_r > 127) { gridCellStates[cellIdx] |= CELL_CLOSEDNORTH; closedCountNorth += gridCellStates[cellIdx] & CELL_OPEN ? 1 : 0; }
            if (closedData_g > 127) { gridCellStates[cellIdx] |= CELL_CLOSEDEAST; closedCountEast += gridCellStates[cellIdx] & CELL_OPEN ? 1 : 0; }
            if (closedData_b > 127) { gridCellStates[cellIdx] |= CELL_CLOSEDSOUTH; closedCountSouth += gridCellStates[cellIdx] & CELL_OPEN ? 1 : 0; }
            if (   (closedData_r < 255 && closedData_r > 0)
                || (closedData_g < 255 && closedData_g > 0)
                || (closedData_b < 255 && closedData_b > 0)) {
                
                // Anything that has closed west edge will be not at full 255 on at least one channel.
                // Typical for all other edge conditions is to use full brightness 255 on the channel(s).
                // All 4 closed would be 128 128 128 but this doesn't ever happen.
                // None closed is 0 0 0
                gridCellStates[cellIdx] |= CELL_CLOSEDWEST; closedCountWest += gridCellStates[cellIdx] & CELL_OPEN ? 1 : 0;
            }
            
            if (closedData_a > 0 && closedData_a < 255) {
                gridCellStates[cellIdx] |= CELL_CLOSEDNORTH | CELL_CLOSEDEAST | CELL_CLOSEDSOUTH | CELL_CLOSEDWEST;
            }
        }
    }
    
    DualLog("Found %d open cells for level %d, Found closed edges north: %d, south: %d, east: %d, west: %d...",totalOpenCells,currentLevel,closedCountNorth,closedCountSouth,closedCountEast,closedCountWest);
    free(edgePixels);
    malloc_trim(0);
    
    // ------------------- Sky/Sun Visibility ------------------    
    char filename3[256];
    sprintf(filename3,"./Data/worldcellskyvis_%d.png",currentLevel);
    fp = fopen(filename3, "rb");
    if (!fp) { DualLogError("Failed to open %s\n", filename3); exit(1); }
    
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    if (file_size > maxFileSize) { DualLogError("PNG file %s too large (%zu bytes)\n", filename3, file_size); exit(1); }
    
    fseek(fp, 0, SEEK_SET);
    read_size = fread(file_buffer, 1, file_size, fp);
    fclose(fp);
    if (read_size != file_size) { DualLogError("Failed to read %s\n", filename3); exit(1); }
    unsigned char* skyPixels = stbi_load_from_memory(file_buffer, file_size, &wpng, &hpng, &channels, 4); // I handmade them, well what can ya do
    if (!skyPixels) { DualLogError("Failed to read %s for culling sky visibility\n", filename3); exit(1); }

    unsigned char skyData_r, skyData_g, skyData_b;
    for (int32_t x=0;x<WORLDX;++x) {
        for (int32_t z=0;z<WORLDZ;++z) {
            int32_t cellIdx = (z * WORLDX) + x;
            int32_t flippedZ = (WORLDZ - 1) - z; // Flip z to match Unity's bottom-left origin for Texture2D vs stbi_load's top-left
            int32_t pixelIdx = (x + (flippedZ * WORLDX)) * 4; // 4 channels
            skyData_r = skyPixels[pixelIdx + 0];
            skyData_g = skyPixels[pixelIdx + 1];
            skyData_b = skyPixels[pixelIdx + 2];
            if (skyData_r > 127 && skyData_g < 127 && skyData_b < 127) gridCellStates[cellIdx] &= ~(CELL_SEES_SUN | CELL_SEES_SKYBOX); // All red cells marked as -1, no sky or sun.
            else if (skyData_r <= 127 && skyData_g <= 127 && skyData_b > 127) gridCellStates[cellIdx] |= CELL_SEES_SUN | CELL_SEES_SKYBOX; // All blue cells marked as sky visible.  Sun + Sky.
            else { gridCellStates[cellIdx] &= ~CELL_SEES_SKYBOX; gridCellStates[cellIdx] |= CELL_SEES_SUN; } // All white and black cells marked as 0.  Only sees Sun.
        }
    }
    
    free(skyPixels);
    free(file_buffer);
    malloc_trim(0);
    DebugRAM("end of dynamic culling DetermineClosedEdges");
}

bool UpdatedPlayerCell() {
    uint16_t lastX = playerCellIdx_x;
    uint16_t lastZ = playerCellIdx_z;
    PosToCellCoords(instances[PLAYER1].position.x,instances[PLAYER1].position.z,&playerCellIdx_x,&playerCellIdx_z);
    playerCellIdx = (playerCellIdx_z * WORLDX) + playerCellIdx_x;
    if (playerCellIdx_x == lastX && playerCellIdx_z == lastZ) return false;
    return true;
}

int32_t CastRayCellCheck(int32_t x, int32_t z, int32_t lastX, int32_t lastZ) {
    if (!(lastX == x && lastZ == z)) {
        if (XZPairInBounds(lastX,lastZ)) {
            int32_t cellIdx_last = (lastZ * WORLDX) + lastX;
            if (lastZ == z) {
                if (lastX > x) { // [  x  ][lastX]
                    if (gridCellStates[cellIdx_last] & CELL_CLOSEDWEST) return -1;
                } else { // Less than x since == x was already checked.
                    if (gridCellStates[cellIdx_last] & CELL_CLOSEDEAST) return -1;
                }
            }

            if (lastX == x) {
                if (lastZ > z) { // [lastZ]
                                 // [  y  ]
                    if (gridCellStates[cellIdx_last] & CELL_CLOSEDSOUTH) return -1;
                } else { // Less than y since == y was already checked.
                    if (gridCellStates[cellIdx_last] & CELL_CLOSEDNORTH) return -1;
                }
            }

            // Diagonals
            if (lastZ != z && lastX != x) {
                int32_t cellIdx_neighborNorth = ((lastZ + 1) * WORLDX) + lastX;
                cellIdx_neighborNorth = cellIdx_neighborNorth > ARRSIZE ? ARRSIZE : cellIdx_neighborNorth;
                int32_t cellIdx_neighborSouth = ((lastZ - 1) * WORLDX) + lastX;
                cellIdx_neighborSouth = cellIdx_neighborSouth > ARRSIZE ? ARRSIZE : cellIdx_neighborSouth;
                int32_t cellIdx_neighborEast = (lastZ * WORLDX) + lastX + 1;
                cellIdx_neighborEast = cellIdx_neighborEast > ARRSIZE ? ARRSIZE : cellIdx_neighborEast;
                int32_t cellIdx_neighborWest = (lastZ * WORLDX) + lastX - 1;
                cellIdx_neighborWest = cellIdx_neighborWest > ARRSIZE ? ARRSIZE : cellIdx_neighborWest;
                
                if (lastZ > z && lastX > x) { // [Nb][ 1]
                                              // [ 2][Na]
                    bool neighborClosedWest = false;
                    bool neighborClosedSouth = false;
                    if (XZPairInBounds(lastX,lastZ - 1)) neighborClosedWest = (gridCellStates[cellIdx_neighborSouth] & CELL_CLOSEDWEST) && (gridCellStates[cellIdx_neighborSouth] & CELL_OPEN);
                    if (XZPairInBounds(lastX - 1,lastZ)) neighborClosedSouth = (gridCellStates[cellIdx_neighborWest] & CELL_CLOSEDSOUTH) && (gridCellStates[cellIdx_neighborWest] & CELL_OPEN);
                    if ((gridCellStates[cellIdx_last] & CELL_CLOSEDSOUTH) && (gridCellStates[cellIdx_last] & CELL_CLOSEDWEST)) return -1;// Check cell 1 only
                    if ((gridCellStates[cellIdx_last] & CELL_CLOSEDWEST) && neighborClosedWest) return -1; // Check cell 1 and Neighbor a (Na)
                    if ((gridCellStates[cellIdx_last] & CELL_CLOSEDSOUTH) && neighborClosedSouth) return -1; // Check cell 1 and Neighbor b (Nb)
                    if (neighborClosedWest && neighborClosedSouth) return -1; // Check Neighbor a (Na) and Neighbor b (Nb)
                } else if (lastZ < z && lastX < x) { // [ ][2]
                                                     // [1][ ]return
                    bool neighborClosedEast = false;
                    bool neighborClosedNorth = false;
                    if (XZPairInBounds(lastX,lastZ + 1)) neighborClosedEast = (gridCellStates[cellIdx_neighborNorth] & CELL_CLOSEDEAST) && (gridCellStates[cellIdx_neighborNorth] & CELL_OPEN);
                    if (XZPairInBounds(lastX + 1,lastZ)) neighborClosedNorth = (gridCellStates[cellIdx_neighborEast] & CELL_CLOSEDNORTH) && (gridCellStates[cellIdx_neighborEast] & CELL_OPEN);
                    if ((gridCellStates[cellIdx_last] & CELL_CLOSEDNORTH) && (gridCellStates[cellIdx_last] & CELL_CLOSEDEAST)) return -1;
                    if ((gridCellStates[cellIdx_last] & CELL_CLOSEDEAST) && neighborClosedEast) return -1;
                    if ((gridCellStates[cellIdx_last] & CELL_CLOSEDNORTH) && neighborClosedNorth) return -1;
                    if (neighborClosedEast && neighborClosedNorth) return -1;
                } else if (lastZ > z && lastX < x) { // [1][ ]
                                                     // [ ][2]
                    bool neighborClosedEast = false;
                    bool neighborClosedSouth = false;
                    if (XZPairInBounds(lastX,lastZ - 1)) neighborClosedEast = (gridCellStates[cellIdx_neighborSouth] & CELL_CLOSEDEAST) && (gridCellStates[cellIdx_neighborSouth] & CELL_OPEN);
                    if (XZPairInBounds(lastX + 1,lastZ)) neighborClosedSouth = (gridCellStates[cellIdx_neighborEast] & CELL_CLOSEDSOUTH) && (gridCellStates[cellIdx_neighborEast] & CELL_OPEN);
                    if ((gridCellStates[cellIdx_last] & CELL_CLOSEDSOUTH) && (gridCellStates[cellIdx_last] & CELL_CLOSEDEAST)) return -1;
                    if ((gridCellStates[cellIdx_last] & CELL_CLOSEDEAST) && neighborClosedEast) return -1;
                    if ((gridCellStates[cellIdx_last] & CELL_CLOSEDSOUTH) && neighborClosedSouth) return -1;
                    if (neighborClosedEast && neighborClosedSouth) return -1;
                } else if (lastZ < z && lastX > x) { // [2][ ]
                                                     // [ ][1]
                    bool neighborClosedWest = false;
                    bool neighborClosedNorth = false;
                    if (XZPairInBounds(lastX,lastZ + 1)) neighborClosedWest = (gridCellStates[cellIdx_neighborNorth] & CELL_CLOSEDWEST) && (gridCellStates[cellIdx_neighborNorth] & CELL_OPEN);
                    if (XZPairInBounds(lastX - 1,lastZ)) neighborClosedNorth = (gridCellStates[cellIdx_neighborWest] & CELL_CLOSEDNORTH) && (gridCellStates[cellIdx_neighborWest] & CELL_OPEN);
                    if ((gridCellStates[cellIdx_last] & CELL_CLOSEDNORTH) && (gridCellStates[cellIdx_last] & CELL_CLOSEDWEST)) return -1;
                    if ((gridCellStates[cellIdx_last] & CELL_CLOSEDWEST) && neighborClosedWest) return -1;
                    if ((gridCellStates[cellIdx_last] & CELL_CLOSEDNORTH) && neighborClosedNorth) return -1;
                    if (neighborClosedWest && neighborClosedNorth) return -1;
                }
            }
        }
    }
    
    if (XZPairInBounds(x,z)) {
        int32_t cellIdx_xz = (z * WORLDX) + x; 
        if (gridCellStates[cellIdx_xz] & CELL_OPEN) gridCellStates[cellIdx_xz] |= CELL_VISIBLE;
        else gridCellStates[cellIdx_xz] &= ~CELL_VISIBLE;
        
        if (!(gridCellStates[cellIdx_xz] & CELL_VISIBLE)) return -1;
        return 1;
    }

    return 0;
}

int32_t CastStraightZ(int32_t px, int32_t pz, int32_t signz) {
    if (signz > 0 && pz >= (WORLDZ - 1)) return pz; // Nowwhere to step to if right by edge, hence WORLDX - 1 here.
    if (signz < 0 && pz <= 0) return pz;
    if (!XZPairInBounds(px,pz)) return pz;
    
    int32_t cellIdx = (pz * WORLDX) + px;
    if (!(gridCellStates[cellIdx] & CELL_VISIBLE)) return pz;
    
    bool currentVisible = true;
    int32_t x = px;
    int32_t z = pz + signz;
    int32_t zabs = abs(z);
    for (;zabs<WORLDX;z+=signz) { // Up/Down
        currentVisible = false;
        int32_t cellIdx_x_zmnus1 = ((z - 1) * WORLDX) + x;
        int32_t cellIdx_x_zplus1 = ((z + 1) * WORLDX) + x;
        if (XZPairInBounds(x,z - signz) && XZPairInBounds(x,z)) {
            int32_t cellIdx_x_zmnus_sign = ((z - signz) * WORLDX) + x;
            if (gridCellStates[cellIdx_x_zmnus_sign] & CELL_VISIBLE) {
                if (signz > 0) {
                    if (gridCellStates[cellIdx_x_zmnus1] & CELL_CLOSEDNORTH && gridCellStates[cellIdx_x_zmnus1] & CELL_OPEN) return z;
                } else if (signz < 0) {
                    if (gridCellStates[cellIdx_x_zplus1] & CELL_CLOSEDSOUTH && gridCellStates[cellIdx_x_zplus1] & CELL_OPEN) return z;
                }

                int32_t subCellIdx = (z * WORLDX) + x;
                if (gridCellStates[subCellIdx] & CELL_OPEN) gridCellStates[subCellIdx] |= CELL_VISIBLE;
                else gridCellStates[subCellIdx] &= ~CELL_VISIBLE;
                
                currentVisible = true; // Would be if twas open.
            }
        }

        if (!currentVisible) break; // Hit wall!

        if (XZPairInBounds(x + 1,z)) {
            int32_t cellIdx_xplus1_z = (z * WORLDX) + x + 1;
            if (CastRayCellCheck(x,z,x + 1,z) > 0) {
                if (gridCellStates[cellIdx_xplus1_z] & CELL_OPEN) gridCellStates[cellIdx_xplus1_z] |= CELL_VISIBLE;
                else gridCellStates[cellIdx_xplus1_z] &= ~CELL_VISIBLE;
            } else {
                gridCellStates[cellIdx_xplus1_z] &= ~CELL_VISIBLE;
            }
        }
        
        if (XZPairInBounds(x - 1,z)) {
            int32_t cellIdx_xmnus1_z = (z * WORLDX) + x - 1;
            if (CastRayCellCheck(x,z,x - 1,z) > 0) {
                if (gridCellStates[cellIdx_xmnus1_z] & CELL_OPEN) gridCellStates[cellIdx_xmnus1_z] |= CELL_VISIBLE;
                else gridCellStates[cellIdx_xmnus1_z] &= ~CELL_VISIBLE;
            } else {
                gridCellStates[cellIdx_xmnus1_z] &= ~CELL_VISIBLE;
            }
        }
    }
    
    return WORLDX * signz;
}

int32_t CastStraightX(int32_t px, int32_t pz, int32_t signx) {
    if (signx > 0 && px >= (WORLDX - 1)) return px; // Nowwhere to step to if right by edge, hence WORLDX - 1 here.
    if (signx < 0 && px <= 0) return px;
    if (!XZPairInBounds(px,pz)) return px;
    if (!gridCellStates[(pz * WORLDX) + px] & CELL_VISIBLE) return px;

    int32_t x = px + signx;
    int32_t z = pz;
    bool currentVisible = true;
    int32_t xabs = abs(x);
    for (;xabs<WORLDX;x+=signx) { // Right/Left
        currentVisible = false;
        int32_t cellIdx_xmnus1_z = (z * WORLDX) + x - 1;
        int32_t cellIdx_xplus1_z = (z * WORLDX) + x + 1;
        if (XZPairInBounds(x - signx,z) && XZPairInBounds(x,z)) {
            int32_t cellIdx_xmnussign_z = (z * WORLDX) + x - signx;
            if (gridCellStates[cellIdx_xmnussign_z] & CELL_VISIBLE) {
                if (signx > 0) {
                    if ((gridCellStates[cellIdx_xmnus1_z] & CELL_CLOSEDEAST) && (gridCellStates[cellIdx_xmnus1_z] & CELL_OPEN)) return x;
                } else if (signx < 0) {
                    if ((gridCellStates[cellIdx_xplus1_z] & CELL_CLOSEDWEST) && (gridCellStates[cellIdx_xplus1_z] & CELL_OPEN)) return x;
                }
                
                int32_t subCellIdx = (z * WORLDX) + x;
                if (gridCellStates[subCellIdx] & CELL_OPEN) gridCellStates[subCellIdx] |= CELL_VISIBLE;
                else gridCellStates[subCellIdx] &= ~CELL_VISIBLE;
                
                currentVisible = true; // Would be if twas open.
            }
        }

        if (!currentVisible) break; // Hit wall!
        
        if (XZPairInBounds(x,z + 1)) {
            int32_t cellIdx_x_zplus1 = ((z + 1) * WORLDX) + x;
            if (CastRayCellCheck(x,z,x,z + 1) > 0) {
                if (gridCellStates[cellIdx_x_zplus1] & CELL_OPEN) gridCellStates[cellIdx_x_zplus1] |= CELL_VISIBLE;
                else gridCellStates[cellIdx_x_zplus1] &= ~CELL_VISIBLE;
            } else {
                gridCellStates[cellIdx_x_zplus1] &= ~CELL_VISIBLE;
            }
        }
        
        if (XZPairInBounds(x,z - 1)) {
            int32_t cellIdx_x_zmnus1 = ((z - 1) * WORLDX) + x;
            if (CastRayCellCheck(x,z,x,z - 1) > 0) {
                if (gridCellStates[cellIdx_x_zmnus1] & CELL_OPEN) gridCellStates[cellIdx_x_zmnus1] |= CELL_VISIBLE;
                else gridCellStates[cellIdx_x_zmnus1] &= ~CELL_VISIBLE;
            } else {
                gridCellStates[cellIdx_x_zmnus1] &= ~CELL_VISIBLE;
            }
        }
    }
    
    return WORLDX * signx;
}

void CastRay(int32_t x0, int32_t z0, int32_t x1, int32_t z1) {
    int32_t dx = abs(x1 - x0);       int32_t dz = abs(z1 - z0);
    int32_t sx = (x0 < x1) ? 1 : -1; int32_t sz = (z0 < z1) ? 1 : -1;
    int32_t x = x0;                  int32_t z = z0;
    int32_t lastX = x;               int32_t lastZ = z;
    int32_t err = dx - dz;
    int32_t iter = dx > dz ? dx : dz;
    while (iter >= 0) {
        if (!XZPairInBounds(x,z) || !XZPairInBounds(lastX,lastZ)) continue;
        if (CastRayCellCheck(x,z,lastX,lastZ) == -1) return;

        lastX = x;
        lastZ = z;
        int32_t e2 = 2 * err;
        if (e2 > -dz) { err -= dz; x += sx; }
        if (e2 <  dx) { err += dx; z += sz; }
        iter--;
    }
}

void CircleFanRays(int32_t x0, int32_t z0) { // CastRay()'s in fan from x0,z0 out to every cell around map perimeter.
    if (!XZPairInBounds(x0,z0)) return;
    if (!(gridCellStates[(z0 * WORLDX) + x0] & CELL_VISIBLE)) return;

    int32_t x,z;     
    int32_t max = WORLDX; // Reduce work slightly by not casting towards 
    int32_t min = 0;      // edges but 1 less = [1,63].
    for (x=min;x<max;x++) CastRay(x0,z0,x,min);
    for (x=min;x<max;x++) CastRay(x0,z0,x,max);
    for (z=min;z<max;z++) CastRay(x0,z0,min,z);
    for (z=min;z<max;z++) CastRay(x0,z0,max,z);
}

void DetermineVisibleCells(int32_t startX, int32_t startZ) {
    if (!XZPairInBounds(startX,startZ)) return;

    for (int32_t x=0;x<WORLDX;x++) {
        for (int32_t z=0;z<WORLDZ;z++) {
            int32_t subCellIdx = (z * WORLDX) + x;
            gridCellStates[subCellIdx] &= ~CELL_VISIBLE; // Clear all to not visible.
        }
    }

    int32_t cellIdx_start = (startZ * WORLDX) + startX;
    gridCellStates[cellIdx_start] |= CELL_VISIBLE; // Force starting player cell to visible.
    
    // Cast to the right (East)        [ ][3]
    CastStraightX(startX,startZ,1); // [1][2]
                                    // [ ][3]
    int32_t iter = 0;
    for (int32_t march=startX;march<(WORLDX - 1);march++) {
        iter++;
        if (iter > WORLDX) break;
        
        if (XZPairInBounds(march,startZ + 1)) {
            if (gridCellStates[((startZ + 1) * WORLDX) + march] & CELL_VISIBLE) {
                march = CastStraightX(march,startZ + 1,1);  // Above [1]
            }
        }
    }
    
    iter = 0;
    for (int32_t march=startX;march<(WORLDX - 1);march++) {
        iter++;
        if (iter > WORLDX) break;

        if (XZPairInBounds(march,startZ - 1)) {
            if (gridCellStates[((startZ - 1) * WORLDX) + march] & CELL_VISIBLE) {
                march = CastStraightX(march,startZ - 1,1);  // Below [1]
            }
        }
    }
    
    // Cast to the left (West)          [3][ ]
    CastStraightX(startX,startZ,-1); // [2][1]
                                     // [3][ ]
    iter = 0;
    for (int32_t march=startX;march>=1;march--) {
        iter++;
        if (iter > WORLDX) break;
        
        if (XZPairInBounds(march,startZ + 1)) {
            if (gridCellStates[((startZ + 1) * WORLDX) + march] & CELL_VISIBLE) {
                march = CastStraightX(march,startZ + 1,-1); // Above [1]
            }
        }
    }

    iter = 0;
    for (int32_t march=startX;march>=1;march--) {
        iter++;
        if (iter > WORLDX) break;

        if (XZPairInBounds(march,startZ - 1)) {
            if (gridCellStates[((startZ - 1) * WORLDX) + march] & CELL_VISIBLE) {
                march = CastStraightX(march,startZ - 1,-1); // Below [1]
            }
        }
    }

    // Cast down (South)                [ ][1][ ]
    CastStraightZ(startX,startZ,-1); // [3][2][3]
    iter = 0;
    for (int32_t march=startZ;march>=1;march--) {
        iter++;
        if (iter > WORLDX) break;

        if (XZPairInBounds(startX + 1,march)) {
            if (gridCellStates[(march * WORLDX) + startX + 1] & CELL_VISIBLE) {
                march = CastStraightZ(startX + 1,march,-1);
            }
        }
    }
    
    iter = 0;
    for (int32_t march=startZ;march>=1;march--) {
        iter++;
        if (iter > WORLDX) break;

        if (XZPairInBounds(startX - 1,march)) {
            if (gridCellStates[(march * WORLDX) + startX - 1] & CELL_VISIBLE) {
                march = CastStraightZ(startX - 1,march,-1);
            }
        }
    }

    // Cast up (North)                 [3][2][3]
    CastStraightZ(startX,startZ,1); // [ ][1][ ]
    iter = 0;
    for (int32_t march=startZ;march<(WORLDX - 1);march++) {
        iter++;
        if (iter > WORLDX) break;

        if (XZPairInBounds(startX + 1,march)) {
            if (gridCellStates[(march * WORLDX) + startX + 1] & CELL_VISIBLE) {
                march = CastStraightZ(startX + 1,march,1);
            }
        }
    }
    
    iter = 0;
    for (int32_t march=startZ;march<(WORLDX - 1);march++) {
        iter++;
        if (iter > WORLDX) break;

        if (XZPairInBounds(startX - 1,march)) {
            if (gridCellStates[(march * WORLDX) + startX - 1] & CELL_VISIBLE) {
                march = CastStraightZ(startX - 1,march,1);
            }
        }
    }

    CircleFanRays(startX,startZ);
    CircleFanRays(startX + 1,startZ);
    CircleFanRays(startX + 1,startZ + 1);
    CircleFanRays(startX,startZ + 1);
    CircleFanRays(startX - 1,startZ + 1);
    CircleFanRays(startX - 1,startZ);
    CircleFanRays(startX - 1,startZ - 1);
    CircleFanRays(startX,startZ - 1);
    CircleFanRays(startX + 1,startZ - 1);
    for (int32_t x=0;x<WORLDX;++x) {
        for (int32_t z=0;z<WORLDZ;++z) {
            int32_t cellIdx_xz = (z * WORLDX) + x;
            if (currentLevel == 5) { // Citadel flight level hackarounds for algorithm discrepancies at glancing angles.
                if (   (x <= 15 && startX <= 15) || (z <= 9 && startZ <= 9)
                    || (x >= 32 && startX >= 32)
                    || (z == 31 && startZ == 31 && x >= 27 && startX >= 27)
                    ||  x >= 34) {
                    
                    gridCellStates[cellIdx_xz] |= CELL_VISIBLE;
                }
                
                if (startX <=12 && x == 14 && z == 31 && startZ >= 24) gridCellStates[cellIdx_xz] |= CELL_VISIBLE;
                if (startX <=12 && x == 14 && z == 30 && startZ >= 24) gridCellStates[cellIdx_xz] |= CELL_VISIBLE;
                if (startX <=12 && x == 13 && z == 30 && startZ >= 24) gridCellStates[cellIdx_xz] |= CELL_VISIBLE;
            }
        }
    }
    
    int32_t numVisible = 0;
    for (int32_t x=0;x<WORLDX;++x) {
        for (int32_t z=0;z<WORLDZ;++z) {
            if (gridCellStates[(z * WORLDX) + x] & CELL_VISIBLE) numVisible++;
        }
    }
}

void CullInit(void) {
    double start_time = get_time();    
    DualLog("Culling...");
    DebugRAM("start of Cull_Init");    
    switch(currentLevel) {
        case 0: worldMin_x = -38.40f + ( 0.00000f +    3.6000f); worldMin_z = -51.20f + (0.0f + 1.0f); break;
        case 1: worldMin_x = -81.92f; worldMin_z = -71.68f; break;
        case 2: worldMin_x = -40.96f + ( 0.00000f +   -2.6000f); worldMin_z = -46.08f + (0.0f + -7.7f); break;
        case 3: worldMin_x = -53.76f + (50.17400f +  -45.1200f); worldMin_z = -46.08f + (13.714f + -16.32f); break;
        case 4: worldMin_x =  -7.68f + ( 1.17800f +  -20.4000f); worldMin_z = -64.00f + (1.292799f + 11.48f); break;
        case 5: worldMin_x = -35.84f + ( 1.17780f +  -10.1400f); worldMin_z = -51.20f + (-1.2417f + -0.0383f); break;
        case 6: worldMin_x = -64.00f + ( 1.29280f +   -0.6728f); worldMin_z = -71.68f + (-1.2033f + 3.76f); break;
        case 7: worldMin_x = -58.88f + ( 1.24110f +   -6.7000f); worldMin_z = -79.36f + (-1.2544f + 1.16f); break;
        case 8: worldMin_x = -40.96f + (-1.30560f +    1.0800f); worldMin_z = -43.52f + (1.2928f + 0.8f); break;
        case 9: worldMin_x = -51.20f + (-1.34390f +    3.6000f); worldMin_z = -64.0f + (-1.1906f + -1.28f); break;
        case 10:worldMin_x =-128.00f + (-0.90945f +  107.3700f); worldMin_z = -71.68f + (-1.0372f + 35.48f); break;
        case 11:worldMin_x = -38.40f + (-1.26720f +   15.0500f); worldMin_z =  51.2f + (0.96056f + -77.94f); break;
        case 12:worldMin_x = -34.53f + ( 0.00000f +   19.0400f); worldMin_z = -123.74f + (0.0f + 95.8f); break;
    }
    
    worldMin_x -= 2.56f; // Add one cell gap around edges
    worldMin_z -= 2.56f;
    voxelMinCenterX = worldMin_x + VOXEL_HALF;
    voxelMinCenterZ = worldMin_z + VOXEL_HALF;
    DetermineClosedEdges();
    PutChunksInCells();
    
    // For each cell, get the visibility as though player were there and put into gridCellStates
    // Then store the visibility of gridCellStates into the table of all visible cells for that cell
    // at the appropriate offset for looking up later when actually re-assigning gridCellStates
    // from this precalculated visibility state for the particular cell.
    int32_t numPrecomputedVisibleCells = 0;
    for (int32_t z=0;z<WORLDZ;z++) {
        for (int32_t x=0;x<WORLDX;x++) {
            playerCellIdx_x = x;
            playerCellIdx_z = z;
            DetermineVisibleCells(x,z);
            int32_t cellIdx = (z * WORLDX) + x;
            for (int32_t z2=0;z2<WORLDZ;z2++) {
                for (int32_t x2=0;x2<WORLDX;x2++) {
                    int32_t subCellIdx = (z2 * WORLDX) + x2;
                    size_t flat_idx = (size_t)(cellIdx * ARRSIZE) + subCellIdx;
                    bool is_visible = (gridCellStates[subCellIdx] & CELL_VISIBLE);
                    set_cull_bit(precomputedVisibleCellsFromHere,flat_idx,is_visible);
                    if (is_visible) numPrecomputedVisibleCells++;
                }
            }
            
            if (currentLevel == 10) {
                if ((x == 15 || x == 16) && z == 23) { // Fix up problem cells at odd angle where ddx doesn't work.
                    size_t flat_idx = (size_t)(cellIdx * ARRSIZE) + ((11 * WORLDX) + 12);
                    set_cull_bit(precomputedVisibleCellsFromHere,flat_idx,true);
                    numPrecomputedVisibleCells++;
                }
            }
        }
    }
    
    UpdatedPlayerCell();
    int32_t cellToCellIdx = playerCellIdx * ARRSIZE;
    int32_t numFoundVisibleCellsForPlayerStart = 0;
    for (int32_t z=0;z<WORLDZ;++z) {
        for (int32_t x=0;x<WORLDX;++x) {
            int32_t cellIdx = (z * WORLDX) + x;
            size_t flat_idx = (size_t)(cellToCellIdx + cellIdx);
            if (get_cull_bit(precomputedVisibleCellsFromHere,flat_idx)) {
                numFoundVisibleCellsForPlayerStart++;
                gridCellStates[cellIdx] |= CELL_VISIBLE; // Get visible before putting meshes into their cells so we can nudge them a little.
            }
        }
    }

    gridCellStates[0] |= CELL_VISIBLE; // Errors default here so draw them anyways.
//     PutMeshesInCells(0); // Static Immutable
//     PutMeshesInCells(1); // Dynamic
//     PutMeshesInCells(2); // Doors
//     PutMeshesInCells(3); // NPCs
//     PutMeshesInCells(4); // Static Saveable
    memset(cellIndexForLight,0,LIGHT_COUNT * sizeof(uint16_t));
    memset(cellIndexForLightX,0,LIGHT_COUNT * sizeof(uint16_t));
    memset(cellIndexForLightZ,0,LIGHT_COUNT * sizeof(uint16_t));
    PutMeshesInCells(5); // Lights
    CullCore(); // Do first Cull pass, forcing as player moved to new cell.
    malloc_trim(0);
    DualLog(" took %f seconds\n", get_time() - start_time);
    DebugRAM("end of Cull_Init");
}

void CullCore(void) {    
    if (currentLevel >= (numLevels - 1)) return;

    lightDirty[0] = true; // Force dynamic lights to update.
    numCellsVisible = 0;
    int32_t cellToCellIdx = playerCellIdx * ARRSIZE;
    for (int32_t z=0;z<WORLDZ;++z) {
        for (int32_t x=0;x<WORLDX;++x) {
            int32_t cellIdx = (z * WORLDX) + x;
            if (cellIdx == 0) { gridCellStates[0] |= CELL_VISIBLE; continue; } // Errors default here so draw them anyways.  Don't count it though.
            if (cellIdx == playerCellIdx) { gridCellStates[playerCellIdx] |= CELL_VISIBLE; numCellsVisible++; continue; } // Always at least set player's cell.

            size_t flat_idx = (size_t)(cellToCellIdx + cellIdx);
            if (get_cull_bit(precomputedVisibleCellsFromHere,flat_idx)) {
                numCellsVisible++;
                gridCellStates[cellIdx] |= CELL_VISIBLE; // Get visible before putting meshes into their cells so we can nudge them a little.
            } else {
                gridCellStates[cellIdx] &= ~CELL_VISIBLE;
            }
        }
    }

//     CameraViewUnculling(playerCellX,playerCellY);
//     UpdateNPCPVS();
//     ToggleNPCPVS();
}

void Cull() {
    if (menuActive || gamePaused || currentLevel >= LEVEL_CYBERSPACE) return;

    // Now handle player position updating PVS. Always do UpdatedPlayerCell
    // to set playerCellX and playerCellY.
    if (UpdatedPlayerCell()) CullCore();
}
