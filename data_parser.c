#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_MAX_DIMENSIONS 2048
#include "External/stb_image.h"
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <uthash.h>
#include <omp.h>
#include <math.h>
#include "voxen.h"
#include "citadel.h"
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/version.h>

DataParser texture_parser; // Zero initialized by C default.
DataParser model_parser;
DataParser level_parser;
DataParser entity_parser;
DataParser lights_parser;
DataParser dynamics_parser;

// Textures
typedef struct {
    uint32_t color;
    uint8_t index;
    UT_hash_handle hh;
} ColorEntry;

GLuint colorBufferID = 0;
GLuint textureSizesID = 0;
GLuint textureOffsetsID = 0;
GLuint texturePalettesID = 0;
GLuint texturePaletteOffsetsID = 0;
uint32_t* textureOffsets = NULL;
uint32_t* texturePaletteOffsets = NULL;
uint32_t* texturePalettes = NULL;
uint32_t totalPixels = 0;
uint32_t totalPaletteColors = 0;
int* textureSizes = NULL;
uint16_t loadedTextures = 0;
bool* doubleSidedTexture = NULL;
bool* transparentTexture = NULL;
unsigned char** image_data = NULL;

// Models
uint32_t* modelVertexCounts = NULL;
uint32_t* modelTriangleCounts = NULL;
uint16_t* modelTypeCountsOpaque = NULL;
uint16_t* modelTypeCountsDoubleSided = NULL;
uint16_t* modelTypeCountsTransparent = NULL;
uint16_t invalidModelIndexCount;
uint16_t* modelTypeOffsetsOpaque = NULL;
uint16_t* modelTypeOffsetsDoubleSided = NULL;
uint16_t* modelTypeOffsetsTransparent = NULL;
uint16_t opaqueInstancesHead = 0;
float** modelVertices = NULL;
uint32_t** modelTriangles = NULL;
GLuint* vbos = NULL;
GLuint* tbos = NULL;
GLuint modelBoundsID;
float* modelBounds = NULL;
uint16_t renderableCount = 0;
uint16_t loadedInstances = 0;
uint16_t loadedModels = 0;
uint16_t loadedLights = 0;
uint16_t startOfDoubleSidedInstances = INSTANCE_COUNT - 1;
uint16_t startOfTransparentInstances = INSTANCE_COUNT - 1;
uint16_t doubleSidedInstancesHead = 0;
uint16_t transparentInstancesHead = 0;

// Entities
Entity entities[MAX_ENTITIES]; // Global array of entity definitions
int32_t entityCount = 0;            // Number of entities loaded
uint16_t physHead = 0;

static int data_parser_isspace(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r'; }

uint32_t parse_numberu32(const char* str, const char* line, uint32_t lineNum) {
    if (str == NULL || *str == '\0') { fprintf(stderr, "Invalid input blank string, from line[%d]: %s\n", lineNum, line); return 0; }
    while (data_parser_isspace((unsigned char)*str)) str++;
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
    uint32_t parseval = parse_numberu32(str, line, lineNum);
    if (parseval > 1) DualLogWarn("Loaded %u in place where expected a boolean from line[%u]: %s\n",lineNum,line);
    return parseval > 0 ? true : false;
}

float parse_float(const char* str, const char* line, uint32_t lineNum) {
    if (str == NULL || *str == '\0') { fprintf(stderr, "Invalid float input blank string, from line[%d]: %s\n", lineNum, line); return 0.0f; }
    char* endptr;
    errno = 0;
    float val = strtof(str, &endptr);
    if (errno != 0 || endptr == str || *endptr != '\0') { fprintf(stderr, "Invalid float input %s, from line[%d]: %s\n", str, lineNum, line); return 0.0f; }
    return val;
}

void init_data_entry(Entity *entry) {
    entry->type = 0;
    entry->cardchunk = false;
    entry->doublesided = false;
    entry->transparent = false;
    entry->index = UINT16_MAX;
    entry->modelIndex = MODEL_IDX_MAX;
    entry->texIndex = UINT16_MAX;
    entry->glowIndex = MATERIAL_IDX_MAX;
    entry->specIndex = MATERIAL_IDX_MAX;
    entry->normIndex = MATERIAL_IDX_MAX;
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

void allocate_entries(DataParser *parser, int32_t entry_count) {
    if (entry_count > MAX_ENTRIES) { DualLogWarn("\033[38;5;208mEntry count %d exceeds %d\033[0m\n", entry_count, MAX_ENTRIES); entry_count = MAX_ENTRIES; }
    
    if (entry_count > parser->capacity) {
        Entity *new_entries = realloc(parser->entries, entry_count * sizeof(Entity));  
        parser->entries = new_entries;
        for (int32_t i = parser->capacity; i < entry_count; ++i) init_data_entry(&parser->entries[i]);
        parser->capacity = entry_count;
    }
    parser->count = entry_count;
}

bool process_key_value(Entity *entry, const char *key, const char *value, const char *line, uint32_t lineNum) {
    if (!key || !value) { DualLogError("Invalid key-value pair at line %u: %s\n", lineNum, line); return false; }
    
    while (data_parser_isspace((unsigned char)*key)) key++;
    while (data_parser_isspace((unsigned char)*value)) value++;
    char trimmed_key[256];
    char trimmed_value[1024];
    strncpy(trimmed_key, key, sizeof(trimmed_key) - 1);
    strncpy(trimmed_value, value, sizeof(trimmed_value) - 1);
    trimmed_key[sizeof(trimmed_key) - 1] = '\0';
    trimmed_value[sizeof(trimmed_value) - 1] = '\0';
    char *key_end = trimmed_key + strlen(trimmed_key) - 1;
    char *val_end = trimmed_value + strlen(trimmed_value) - 1;
    while (key_end > trimmed_key && data_parser_isspace((unsigned char)*key_end)) *key_end-- = '\0';
    while (val_end > trimmed_value && data_parser_isspace((unsigned char)*val_end)) *val_end-- = '\0';
    if (strncmp(trimmed_key, "chunk_", 6) == 0) {
        strncpy(entry->path, trimmed_key, sizeof(entry->path) - 1);
        entry->path[sizeof(entry->path) - 1] = '\0';
        return true;
    }
         if (strcmp(trimmed_key, "index") == 0)           entry->index = parse_numberu16(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "constIndex") == 0)      entry->index = parse_numberu16(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "model") == 0)           entry->modelIndex = parse_numberu16(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "texture") == 0)         entry->texIndex = parse_numberu16(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "glowtexture") == 0)     entry->glowIndex = parse_numberu16(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "spectexture") == 0)     entry->specIndex = parse_numberu16(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "normtexture") == 0)     entry->normIndex = parse_numberu16(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "doublesided") == 0)     entry->doublesided = parse_bool(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "transparent") == 0)     entry->transparent = parse_bool(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "cardchunk") == 0)       entry->cardchunk = parse_bool(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "modname") == 0)         { strncpy(global_modname, trimmed_value, sizeof(global_modname) - 1); global_modname[sizeof(global_modname) - 1] = '\0'; entry->index = 0; } // Game/Mod Definition enforces setting entry index to 0 here, at least one of these must do it.  The game definition only has one index, 0.
    else if (strcmp(trimmed_key, "levelcount") == 0)      numLevels = parse_numberu8(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "startlevel") == 0)      startLevel = parse_numberu8(trimmed_value, line, lineNum);
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
    else if (strcmp(trimmed_key, "intensity") == 0)       entry->intensity = parse_float(trimmed_value, line, lineNum) * 0.4; // Adjustment, globally applied from Citadel Unity conversion.
    else if (strcmp(trimmed_key, "range") == 0)           entry->range = parse_float(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "spotAngle") == 0)       entry->spotAngle = parse_float(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "type") == 0) {
        if ((strcmp(trimmed_value, "Spot") == 0)) entry->type = 1u;
        else if ((strcmp(trimmed_value, "Directional") == 0)) entry->type = 2u;
        else entry->type = 0u;
    }
    else if (strcmp(trimmed_key, "color.r") == 0)         entry->color.r = parse_float(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "color.g") == 0)         entry->color.g = parse_float(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "color.b") == 0)         entry->color.b = parse_float(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "go.activeSelf") == 0)   entry->active = parse_bool(trimmed_value, line, lineNum);
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
    Entity entry;
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
    currentLevel = startLevel;
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
        while (end > start && data_parser_isspace((unsigned char)*end)) *end-- = '\0';
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
    Entity entry;
    init_data_entry(&entry);
    int32_t entries_stored = 0;
    lineNum = 0;
    while (fgets(line, sizeof(line), file)) {
        lineNum++;
        char *start = line;
        while (data_parser_isspace((unsigned char)*start)) start++;
        char *end = start + strlen(start) - 1;
        while (end > start && data_parser_isspace((unsigned char)*end)) *end-- = '\0';
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

static bool ParseSaveLevelData(DataParser *parser, FILE* file, const char *filename) {
    int32_t entry_count = 0;
    bool is_comment, is_eof, is_newline;
    int32_t c;
    uint32_t lineNum = 0;
    while ((c = fgetc(file)) != EOF) { // First pass: count entries
        if (data_parser_isspace(c) || c == '\n') continue;
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
    int32_t current_index = 0;
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

bool parse_data_file(DataParser *parser, const char *filename, int32_t type) {
    FILE *file = fopen(filename, "r");
    if (!file) { DualLogError("Cannot open %s: %s\n", filename, strerror(errno)); return false; }
    
    if (type == 1) return ParseSaveLevelData(parser, file, filename);
    else return ParseResourceData(parser, file, filename);
}

bool isDoubleSided(uint32_t texIndexToCheck) {
    if (texIndexToCheck > loadedTextures) return false;
    return doubleSidedTexture[texIndexToCheck] > 0 ? 1 : 0;
}
bool isTransparent(uint32_t texIndexToCheck) {
    if (texIndexToCheck > loadedTextures) return false;
    return transparentTexture[texIndexToCheck] > 0 ? 1 : 0;    
}

//-----------------------------------------------------------------------------
// Load all Textures
void LoadTextures(void) {
    double start_time = get_time();
    DualLog("Loading textures");
    DebugRAM("start of LoadTextures");
    loadedTextures = 0;
    uint16_t numTexturesOver256PaletteSize = 0;
    if (!parse_data_file(&texture_parser, "./Data/textures.txt", 0)) { DualLogError("Could not parse ./Data/textures.txt!\n"); exit(1); }

    int32_t maxIndex = -1;
    for (int32_t k = 0; k < texture_parser.count; k++) {
        if (texture_parser.entries[k].index > maxIndex && texture_parser.entries[k].index != UINT16_MAX) {
            maxIndex = texture_parser.entries[k].index;
        }
    }

    loadedTextures = maxIndex + 1;
    if (loadedTextures == 0) { DualLogError("No textures found in textures.txt\n"); exit(1); }

    DualLog("(%d) with max index %d, using stb_image version:  2.28...", loadedTextures, maxIndex);
    image_data            =   malloc(loadedTextures * sizeof(unsigned char*));
    textureOffsets        = calloc(loadedTextures, sizeof(uint32_t));
    textureSizes          = calloc(loadedTextures * 2, sizeof(int));
    texturePaletteOffsets = calloc(loadedTextures, sizeof(uint32_t));
    doubleSidedTexture    = calloc(loadedTextures,sizeof(bool));
    transparentTexture    = calloc(loadedTextures,sizeof(bool));
    size_t maxFileSize = 512000;
    uint32_t totalPaletteColorsExtraSized = 80000;
    texturePalettes             = malloc(totalPaletteColorsExtraSized * sizeof(uint32_t));
    int32_t* widths             = malloc(loadedTextures * sizeof(int32_t));
    int32_t* heights            = malloc(loadedTextures * sizeof(int32_t));
    int32_t* matchedParserIdxes = malloc(loadedTextures * sizeof(int32_t));

    // Initialize arrays
    for (int32_t i = 0; i < loadedTextures; i++) {
        image_data[i] = NULL;
        widths[i] = 0;
        heights[i] = 0;
        matchedParserIdxes[i] = -1;
    }

    // Match parser entries to indices
    for (int32_t k = 0; k < texture_parser.count; k++) {
        if (texture_parser.entries[k].index < loadedTextures) {
            matchedParserIdxes[texture_parser.entries[k].index] = k;
        }
    }
    
    int num_threads = omp_get_max_threads();
    uint8_t** file_buffer_pool = malloc(num_threads * sizeof(uint8_t*));
    for (int i = 0; i < num_threads; i++) file_buffer_pool[i] = malloc(maxFileSize);

    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        uint8_t* file_buffer = file_buffer_pool[thread_id];
        #pragma omp for schedule(dynamic)
        for (int32_t i = 0; i < loadedTextures; i++) {
            if (matchedParserIdxes[i] < 0) continue;
            struct stat file_stat;
            if (stat(texture_parser.entries[matchedParserIdxes[i]].path, &file_stat) != 0) { DualLogError("Failed to stat %s: %s\n", texture_parser.entries[matchedParserIdxes[i]].path, strerror(errno)); continue; }
                
            size_t file_size = file_stat.st_size;
            if (file_size > maxFileSize) { DualLogError("PNG file %s too large (%zu bytes)\n", texture_parser.entries[matchedParserIdxes[i]].path, file_size); continue; }
            
            FILE* fp = fopen(texture_parser.entries[matchedParserIdxes[i]].path, "rb");
            if (!fp) { DualLogError("Failed to open %s: %s\n", texture_parser.entries[matchedParserIdxes[i]].path, strerror(errno)); continue; }
            
            fread(file_buffer, 1, file_size, fp);
            fclose(fp);
            int w, h, n;
            image_data[i] = stbi_load_from_memory(file_buffer, file_size, &w, &h, &n, STBI_rgb_alpha);
            if (!image_data[i]) { DualLogError("stbi_load failed for %s: %s\n", texture_parser.entries[matchedParserIdxes[i]].path, stbi_failure_reason()); continue; }
            
            widths[matchedParserIdxes[i]] = w;
            heights[matchedParserIdxes[i]] = h;
            doubleSidedTexture[matchedParserIdxes[i]] = texture_parser.entries[matchedParserIdxes[i]].doublesided > 0 ? 1 : 0;
            transparentTexture[matchedParserIdxes[i]] = texture_parser.entries[matchedParserIdxes[i]].transparent > 0 ? 1 : 0;
        }
    }

    for (int i = 0; i < num_threads; i++) free(file_buffer_pool[i]);
    free(file_buffer_pool);
    malloc_trim(0);

    // Initialize OpenGL buffers
    GLuint stagingBuffer;
    glGenBuffers(1, &stagingBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, stagingBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, (((MAX_TEXTURE_DIMENSION * MAX_TEXTURE_DIMENSION) + 3) / 4) * sizeof(uint32_t), NULL, GL_DYNAMIC_COPY);

    glGenBuffers(1, &texturePalettesID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, texturePalettesID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, totalPaletteColorsExtraSized * sizeof(uint32_t), NULL, GL_STATIC_DRAW);

    glGenBuffers(1, &colorBufferID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, colorBufferID);
    uint32_t max_total_pixels = 45100000; // From colorBufferSize
    int32_t colorBufferSize = (((int32_t)max_total_pixels + 3) / 4) * sizeof(uint32_t);
    glBufferData(GL_SHADER_STORAGE_BUFFER, colorBufferSize, NULL, GL_STATIC_DRAW);
    ColorEntry* color_pool = malloc(loadedTextures * MAX_PALETTE_SIZE * sizeof(ColorEntry));
    uint32_t* pool_indices = malloc(loadedTextures * sizeof(uint32_t));
    memset(pool_indices, 0, loadedTextures * sizeof(uint32_t));
    uint32_t pixel_offset = 0;
    uint32_t palette_offset = 0;
    uint32_t maxPalletSize = 0;
    uint32_t** per_texture_palettes = malloc(loadedTextures * sizeof(uint32_t*));
    uint32_t* per_texture_palette_sizes = malloc(loadedTextures * sizeof(uint32_t));
    uint8_t* all_indices = malloc(max_total_pixels * sizeof(uint8_t));
    uint32_t* index_offsets = malloc(loadedTextures * sizeof(uint32_t));
    uint32_t current_index_offset = 0;
    for (uint16_t i = 0; i < loadedTextures; i++) {
        per_texture_palettes[i] = malloc(MAX_PALETTE_SIZE * sizeof(uint32_t));
        per_texture_palette_sizes[i] = 0;
        index_offsets[i] = current_index_offset;
        if (matchedParserIdxes[i] >= 0 && image_data[i]) {
            current_index_offset += widths[i] * heights[i];
        }
    }

    // Parallel loop for palette construction
    #pragma omp parallel
    {
        #pragma omp for schedule(dynamic)
        for (uint16_t i = 0; i < loadedTextures; i++) {
            if (matchedParserIdxes[i] < 0 || !image_data[i]) continue;
            ColorEntry* color_table = NULL;
            uint32_t palette_size = 0; // Oversized larger than max pallete size for catching overflows.
            uint8_t* texture_indices = &all_indices[index_offsets[i]];
            uint32_t pool_start = i * MAX_PALETTE_SIZE;
            for (int32_t j = 0; j < widths[i] * heights[i] * 4; j += 4) {
                uint32_t color = ((uint32_t)image_data[i][j] << 24) | ((uint32_t)image_data[i][j + 1] << 16) |
                                ((uint32_t)image_data[i][j + 2] << 8) | (uint32_t)image_data[i][j + 3];
                ColorEntry* entry;
                HASH_FIND_INT(color_table, &color, entry);
                if (!entry) {
                    if (palette_size >= MAX_PALETTE_SIZE) {
                        DualLogError("Palette size exceeded for %s\n", texture_parser.entries[matchedParserIdxes[i]].path);
                        palette_size = MAX_PALETTE_SIZE - 1;
                        break;
                    }
                    entry = &color_pool[pool_start + palette_size];
                    entry->color = color;
                    entry->index = (uint8_t)palette_size++;
                    HASH_ADD_INT(color_table, color, entry);
                    per_texture_palettes[i][entry->index] = color;
                }
                texture_indices[j / 4] = entry->index;
            }
            per_texture_palette_sizes[i] = palette_size;
            HASH_CLEAR(hh, color_table); // No free needed, as entries are from color_pool
        }
    }

    for (uint16_t i = 0; i < loadedTextures; i++) {
        if (matchedParserIdxes[i] < 0 || !image_data[i]) continue;
        textureOffsets[i] = totalPixels;
        texturePaletteOffsets[i] = totalPaletteColors;
        textureSizes[i * 2] = widths[i];
        textureSizes[(i * 2) + 1] = heights[i];
        uint32_t palette_size = per_texture_palette_sizes[i];
        memcpy(&texturePalettes[palette_offset], per_texture_palettes[i], palette_size * sizeof(uint32_t));
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, stagingBuffer);
        uint32_t* mapped_buffer = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0,
                                                ((widths[i] * heights[i] + 3) / 4) * sizeof(uint32_t),
                                                GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
        uint8_t* texture_indices = &all_indices[index_offsets[i]];
        for (int32_t j = 0; j < widths[i] * heights[i]; j += 4) {
            uint32_t packed = (uint32_t)texture_indices[j];
            if (j + 1 < widths[i] * heights[i]) {
                packed |= (uint32_t)texture_indices[j + 1] << 8;
            }
            if (j + 2 < widths[i] * heights[i]) {
                packed |= (uint32_t)texture_indices[j + 2] << 16;
            }
            if (j + 3 < widths[i] * heights[i]) {
                packed |= (uint32_t)texture_indices[j + 3] << 24;
            }
            mapped_buffer[j / 4] = packed;
        }
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glBindBuffer(GL_COPY_READ_BUFFER, stagingBuffer);
        glBindBuffer(GL_COPY_WRITE_BUFFER, colorBufferID);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0,
                            ((pixel_offset + 3) / 4) * sizeof(uint32_t),
                            ((widths[i] * heights[i] + 3) / 4) * sizeof(uint32_t));
        pixel_offset += widths[i] * heights[i];
        palette_offset += palette_size;
        totalPixels += widths[i] * heights[i];
        totalPaletteColors += palette_size;
        if (palette_size > 256) {
            numTexturesOver256PaletteSize++;
            DualLogWarn("Palette size of %u is greater than 256 for %s\n", palette_size, texture_parser.entries[matchedParserIdxes[i]].path);
        }
        
        #ifdef DEBUG_TEXTURE_LOAD_DATA
        DualLog("Pallete size for texture %s is %u\n",texture_parser.entries[matchedParserIdxes[i]].path,palette_size);
        #endif

        if (palette_size > maxPalletSize) maxPalletSize = palette_size;
        stbi_image_free(image_data[i]);
        image_data[i] = NULL;
    }

    free(all_indices);
    free(index_offsets);
    for (uint16_t i = 0; i < loadedTextures; i++) {
        free(per_texture_palettes[i]);
    }
    
    free(per_texture_palettes);
    free(per_texture_palette_sizes);
    free(color_pool);
    free(pool_indices);
    free(image_data);
    free(widths);
    free(heights);
    free(matchedParserIdxes);
    glDeleteBuffers(1, &stagingBuffer);

    #ifdef DEBUG_TEXTURE_LOAD_DATA
    DualLog("Largest palette size of %d, Total palletes greater than 256: %u, total pallete colors: %u\n", maxPalletSize, numTexturesOver256PaletteSize, totalPaletteColors);
    #endif

    // Upload texture palettes
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, texturePalettesID);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, totalPaletteColors * sizeof(uint32_t), texturePalettes);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 16, texturePalettesID);
    free(texturePalettes);

    // Upload texture offsets
    glGenBuffers(1, &textureOffsetsID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, textureOffsetsID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, loadedTextures * sizeof(uint32_t), textureOffsets, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 14, textureOffsetsID);
    free(textureOffsets);

    // Upload texture sizes
    glGenBuffers(1, &textureSizesID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, textureSizesID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, loadedTextures * 2 * sizeof(int32_t), textureSizes, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 15, textureSizesID);
    free(textureSizes);

    // Upload texture palette offsets
    glGenBuffers(1, &texturePaletteOffsetsID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, texturePaletteOffsetsID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, loadedTextures * sizeof(uint32_t), texturePaletteOffsets, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 17, texturePaletteOffsetsID);
    free(texturePaletteOffsets);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, colorBufferID);
    glFlush();
    CHECK_GL_ERROR();
    malloc_trim(0);
    double end_time = get_time();
    DualLog(" took %f seconds\n", end_time - start_time);
    DebugRAM("After LoadTextures");
}

//-----------------------------------------------------------------------------
// Loads all 3D meshes
void LoadModels(void) {
    double start_time = get_time();
    DebugRAM("start of LoadModels");
    loadedModels = 0;
    if (!parse_data_file(&model_parser, "./Data/models.txt", 0)) { DualLogError("Could not parse ./Data/models.txt!\n"); exit(1); }

    int32_t maxIndex = -1;
    for (int32_t k = 0; k < model_parser.count; k++) {
        if (model_parser.entries[k].index > maxIndex && model_parser.entries[k].index != UINT16_MAX) maxIndex = model_parser.entries[k].index;
    }

    loadedModels = maxIndex + 1;
    DualLog("Loading   models( %d) with max index  %d, using    Assimp version: %d.%d.%d...", model_parser.count, maxIndex, aiGetVersionMajor(), aiGetVersionMinor(), aiGetVersionPatch());
    int32_t totalVertCount = 0;
    int32_t totalBounds = 0;
    int32_t totalTriCount = 0;
    #ifdef DEBUG_MODEL_LOAD_DATA
        uint32_t largestVertCount = 0;
        uint32_t largestTriangleCount = 0;
    #endif
    modelVertexCounts   = calloc(loadedModels, sizeof(uint32_t));
    modelTriangleCounts = calloc(loadedModels, sizeof(uint32_t));
    modelVertices       = calloc(loadedModels, sizeof(float*));
    modelTriangles      = calloc(loadedModels, sizeof(uint32_t*));
    modelBounds         = calloc(loadedModels * BOUNDS_ATTRIBUTES_COUNT, sizeof(float));
    GLuint stagingVBO, stagingTBO;
    glGenBuffers(1, &stagingVBO);
    glGenBuffers(1, &stagingTBO);
    int32_t* indexToParser = calloc(loadedModels, sizeof(int32_t));
    for (int32_t k = 0; k < model_parser.count; k++) {
        if (model_parser.entries[k].index != UINT16_MAX) {
            indexToParser[model_parser.entries[k].index] = k;
        }
    }
    
    struct aiPropertyStore* props = aiCreatePropertyStore();
    aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_READ_ANIMATIONS, 1);
    aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_READ_MATERIALS, 0);
    aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_READ_TEXTURES, 0);
    aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_READ_LIGHTS, 0);
    aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_READ_CAMERAS, 0);
    aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_FBX_OPTIMIZE_EMPTY_ANIMATION_CURVES, 1);
    aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_NO_SKELETON_MESHES, 0);
    aiSetImportPropertyInteger(props, AI_CONFIG_PP_RVC_FLAGS, aiComponent_ANIMATIONS | aiComponent_BONEWEIGHTS);
    aiSetImportPropertyInteger(props, AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
    aiSetImportPropertyInteger(props, AI_CONFIG_PP_ICL_PTCACHE_SIZE, 16);
    aiSetImportPropertyInteger(props, AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4);
    aiSetImportPropertyInteger(props, AI_CONFIG_PP_FD_REMOVE, 1);
    aiSetImportPropertyInteger(props, AI_CONFIG_PP_PTV_KEEP_HIERARCHY, 0);
    #pragma omp parallel
    {
        #pragma omp for schedule(dynamic)
        for (uint32_t i = 0; i < loadedModels; i++) {
            int32_t matchedParserIdx = indexToParser[i];
            if (!model_parser.entries[matchedParserIdx].path || model_parser.entries[matchedParserIdx].path[0] == '\0') continue; // Perfectly fine to skip unused indices between 0 and max.

            
            const struct aiScene* scene = aiImportFileExWithProperties(model_parser.entries[matchedParserIdx].path, aiProcess_GenNormals | aiProcess_ImproveCacheLocality, NULL, props);
            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) { DualLogError("Assimp failed to load %s: %s\n", model_parser.entries[matchedParserIdx].path, aiGetErrorString()); aiReleasePropertyStore(props); exit(1); }

            uint32_t vertexCount = 0, triCount = 0;
            for (uint32_t m = 0; m < scene->mNumMeshes; m++) {
                vertexCount += scene->mMeshes[m]->mNumVertices;
                triCount += scene->mMeshes[m]->mNumFaces;
            }

            if (vertexCount > MAX_VERT_COUNT || triCount > MAX_TRI_COUNT || vertexCount < 1 || triCount < 1) { DualLogError("Model %s exceeds buffer limits: verts=%u (> %u), tris=%u (> %u)\n", model_parser.entries[matchedParserIdx].path, vertexCount, MAX_VERT_COUNT, triCount, MAX_TRI_COUNT); exit(1); }

            modelVertexCounts[i] = vertexCount;
            modelTriangleCounts[i] = triCount;
            #pragma omp critical
            { // Ensure that the total is atomic
                #ifdef DEBUG_MODEL_LOAD_DATA
                    if (vertexCount > largestVertCount) largestVertCount = vertexCount;
                    if (triCount > largestTriangleCount) largestTriangleCount = triCount;
                #endif
                totalVertCount += vertexCount;
                totalTriCount += triCount;
            }

            modelVertices[i]  = calloc(vertexCount * VERTEX_ATTRIBUTES_COUNT,sizeof(float));
            modelTriangles[i] = calloc(triCount * 3,sizeof(uint32_t));
            uint32_t vertexIndex = 0;
            float minx = 1E9f, miny = 1E9f, minz = 1E9f;
            float maxx = -1E9f, maxy = -1E9f, maxz = -1E9f;
            uint32_t triangleIndex = 0;
            uint32_t globalVertexOffset = 0;
            for (uint32_t m = 0; m < scene->mNumMeshes; m++) {
                struct aiMesh* mesh = scene->mMeshes[m];
                for (uint32_t v = 0; v < mesh->mNumVertices; v++) {
                    modelVertices[i][vertexIndex++] = mesh->mVertices[v].x;
                    modelVertices[i][vertexIndex++] = mesh->mVertices[v].y;
                    modelVertices[i][vertexIndex++] = mesh->mVertices[v].z;
                    modelVertices[i][vertexIndex++] = mesh->mNormals[v].x;
                    modelVertices[i][vertexIndex++] = mesh->mNormals[v].y;
                    modelVertices[i][vertexIndex++] = mesh->mNormals[v].z;
                    float tempU = mesh->mTextureCoords[0] ? mesh->mTextureCoords[0][v].x : 0.0f;
                    float tempV = mesh->mTextureCoords[0] ? mesh->mTextureCoords[0][v].y : 0.0f;
                    modelVertices[i][vertexIndex++] = tempU;
                    modelVertices[i][vertexIndex++] = tempV;
                    if (mesh->mVertices[v].x < minx) minx = mesh->mVertices[v].x;
                    if (mesh->mVertices[v].x > maxx) maxx = mesh->mVertices[v].x;
                    if (mesh->mVertices[v].y < miny) miny = mesh->mVertices[v].y;
                    if (mesh->mVertices[v].y > maxy) maxy = mesh->mVertices[v].y;
                    if (mesh->mVertices[v].z < minz) minz = mesh->mVertices[v].z;
                    if (mesh->mVertices[v].z > maxz) maxz = mesh->mVertices[v].z;
                }

                for (uint32_t f = 0; f < mesh->mNumFaces; f++) {
                    struct aiFace* face = &mesh->mFaces[f];
                    if (face->mNumIndices != 3) { DualLogError("Non-triangular face detected in %s, face %u\n", model_parser.entries[matchedParserIdx].path, f); exit(1); }
                    
                    uint32_t v[3] = {face->mIndices[0] + globalVertexOffset, face->mIndices[1] + globalVertexOffset, face->mIndices[2] + globalVertexOffset};
                    modelTriangles[i][triangleIndex++] = v[0];
                    modelTriangles[i][triangleIndex++] = v[1];
                    modelTriangles[i][triangleIndex++] = v[2];
                }
                
                globalVertexOffset += mesh->mNumVertices;
            }

            uint32_t boundsBase = (i * BOUNDS_ATTRIBUTES_COUNT);
            modelBounds[boundsBase + BOUNDS_DATA_OFFSET_MINX] = minx;
            modelBounds[boundsBase + BOUNDS_DATA_OFFSET_MINY] = miny;
            modelBounds[boundsBase + BOUNDS_DATA_OFFSET_MINZ] = minz;
            modelBounds[boundsBase + BOUNDS_DATA_OFFSET_MAXX] = maxx;
            modelBounds[boundsBase + BOUNDS_DATA_OFFSET_MAXY] = maxy;
            modelBounds[boundsBase + BOUNDS_DATA_OFFSET_MAXZ] = maxz;
            modelBounds[boundsBase + BOUNDS_DATA_OFFSET_RADIUS] = fmaxf(fmaxf(fmaxf(fmaxf(fmaxf(fabs(minx), fabs(miny)), fabs(minz)), maxx), maxy), maxz);
            aiReleaseImport(scene);
            malloc_trim(0);
        }
    }

    aiReleasePropertyStore(props);
    malloc_trim(0);
    vbos = calloc(loadedModels, sizeof(GLuint));
    tbos = calloc(loadedModels, sizeof(GLuint));
    for (uint32_t i = 0; i < loadedModels; i++) { // Sequential phase
        if (modelVertexCounts[i] == 0 || modelTriangleCounts[i] == 0) continue;

        totalBounds += BOUNDS_ATTRIBUTES_COUNT;
        size_t vertSize = modelVertexCounts[i] * VERTEX_ATTRIBUTES_COUNT * sizeof(float);
        glBindBuffer(GL_ARRAY_BUFFER, stagingVBO);
        glBufferData(GL_ARRAY_BUFFER, vertSize, NULL, GL_DYNAMIC_COPY);
        void* mapped_buffer = glMapBufferRange(GL_ARRAY_BUFFER, 0, vertSize, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
        memcpy(mapped_buffer, modelVertices[i], vertSize);
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glGenBuffers(1, &vbos[i]);
        glBindBuffer(GL_COPY_WRITE_BUFFER, vbos[i]);
        glBufferData(GL_COPY_WRITE_BUFFER, vertSize, NULL, GL_STATIC_DRAW);
        glCopyBufferSubData(GL_ARRAY_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, vertSize);

        size_t triSize = modelTriangleCounts[i] * 3 * sizeof(uint32_t);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stagingTBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, triSize, NULL, GL_DYNAMIC_COPY);
        mapped_buffer = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, triSize, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
        memcpy(mapped_buffer, modelTriangles[i], triSize);
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
        glGenBuffers(1, &tbos[i]);
        glBindBuffer(GL_COPY_WRITE_BUFFER, tbos[i]);
        glBufferData(GL_COPY_WRITE_BUFFER, triSize, NULL, GL_STATIC_DRAW);
        glCopyBufferSubData(GL_ELEMENT_ARRAY_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, triSize);
    }

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

    glGenBuffers(1, &modelBoundsID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelBoundsID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, loadedModels * BOUNDS_ATTRIBUTES_COUNT * sizeof(float), modelBounds, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, modelBoundsID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glDeleteBuffers(1, &stagingVBO);
    glDeleteBuffers(1, &stagingTBO);
    DualLog(" took %f seconds\n", get_time() - start_time);
    DebugRAM("After Load Models");
    free(indexToParser);
    malloc_trim(0);
}

//--------------------------------- Entities -------------------------------------
// Suppress -Wformat-truncation for LoadEntities so it can share 256 length "path" and truncate it into 32 length "name".
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
void LoadEntities(void) {
    double start_time = get_time();
    
    // Initialize parser with entity-specific keys
    if (!parse_data_file(&entity_parser, "./Data/entities.txt",0)) { DualLogError("Could not parse ./Data/entities.txt!\n"); exit(1); }
    
    entityCount = entity_parser.count;
    if (entityCount > MAX_ENTITIES) { DualLogError("Too many entities in parser count %d, greater than %d!\n", entityCount, MAX_ENTITIES); exit(1); }
    if (entityCount == 0) { DualLogError("No entities found in entities.txt\n"); exit(1); }

    DualLog("Loading  %d entities...", entityCount);

    // Populate entities array
    for (int32_t i = 0; i < entityCount; i++) {
        if (entity_parser.entries[i].index == UINT16_MAX) continue;

        // Copy with truncation to 31 characters to fit 32 char array for name.  Smaller for RAM constraints.
        snprintf(entities[i].path, MAX_PATH, "%s", entity_parser.entries[i].path);
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
    }

    DualLog(" took %f seconds\n", get_time() - start_time);
    DebugRAM("after loading all entities");
}
#pragma GCC diagnostic pop // Ok restore string truncation warning

void GetLevel_Transform_Offsets(int32_t curlevel, float* ofsx, float* ofsy, float* ofsz) {
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
        case LEVEL_CYBERSPACE: *ofsx = 164.7f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

void GetLevel_LightsStaticSaveable_ContainerOffsets(int32_t curlevel, float* ofsx, float* ofsy, float* ofsz) {
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
        case LEVEL_CYBERSPACE: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

void GetLevel_LightsStaticImmutable_ContainerOffsets(int32_t curlevel, float* ofsx, float* ofsy, float* ofsz) {
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
        case LEVEL_CYBERSPACE: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

void GetLevel_DoorsStaticSaveable_ContainerOffsets(int32_t curlevel, float* ofsx, float* ofsy, float* ofsz) {
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
        case LEVEL_CYBERSPACE: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

void GetLevel_StaticObjectsSaveable_ContainerOffsets(int32_t curlevel, float* ofsx, float* ofsy, float* ofsz) {
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
        case LEVEL_CYBERSPACE: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

void GetLevel_StaticObjectsImmutable_ContainerOffsets(int32_t curlevel, float* ofsx, float* ofsy, float* ofsz) {
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
        case LEVEL_CYBERSPACE: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

void GetLevel_NPCsSaveableInstantiated_ContainerOffsets(int32_t curlevel, float* ofsx, float* ofsy, float* ofsz) {
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
        case LEVEL_CYBERSPACE: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
        default: *ofsx = 0.0f; *ofsy = 0.0f; *ofsz = 0.0f; break;
    }
}

//----------------------------------- Level -----------------------------------
void LoadLevelGeometry(uint8_t curlevel) {
    double start_time = get_time();
    memset(instances,0,INSTANCE_COUNT * sizeof(Entity)); // Initialize instances
    for (uint16_t idx = 0;idx<INSTANCE_COUNT;idx++) {
        instances[idx].modelIndex = MODEL_IDX_MAX;
        instances[idx].texIndex = UINT16_MAX;
        instances[idx].glowIndex = MATERIAL_IDX_MAX;
        instances[idx].specIndex = MATERIAL_IDX_MAX;
        instances[idx].normIndex = MATERIAL_IDX_MAX;
        instances[idx].lodIndex = UINT16_MAX;
        instances[idx].scale.x = instances[idx].scale.y = instances[idx].scale.z = 1.0f; // Default scale
        instances[idx].rotation.w = 1.0f; // Quaternion identity
        instances[idx].doublesided = 0u;
        instances[idx].transparent = 0u;
        dirtyInstances[idx] = true;
    }

    memset(modelMatrices, 0, INSTANCE_COUNT * 16 * sizeof(float)); // Matrix4x4 = 16
    DebugRAM("end of SetupInstances");
    if (curlevel >= numLevels) { DualLogError("Cannot load world geometry, level number %d out of bounds 0 to %d\n",curlevel,numLevels - 1); exit(1); }

    DebugRAM("start of LoadLevelGeometry");
    char filename[64];
    snprintf(filename, sizeof(filename), "./Data/CitadelScene_geometry_level%d.txt", curlevel);
    if (!parse_data_file(&level_parser, filename,1)) { DualLogError("Could not parse %s!\n",filename); exit(1); }

    uint16_t gameObjectCount = level_parser.count;
    DualLog("Loading %d geometry chunks for Level %d...",gameObjectCount,curlevel);
    float correctionX, correctionY, correctionZ;
    GetLevel_Transform_Offsets(curlevel,&correctionX,&correctionY,&correctionZ);
    for (uint16_t idx=loadedInstances;idx<gameObjectCount;++idx) {
        loadedInstances++;
        instances[idx] = level_parser.entries[idx];
        uint16_t entIdx = level_parser.entries[idx].index;
        if (entIdx >= MAX_ENTITIES) { DualLogError("\nEntity index when loading level geometry object %d was %d, exceeds max entity count of %d\n",idx,entIdx,MAX_ENTITIES); exit(1); }
        
        instances[idx].modelIndex = entities[entIdx].modelIndex;
        if (instances[idx].modelIndex < loadedModels) renderableCount++;
        instances[idx].texIndex = entities[entIdx].texIndex;
        instances[idx].glowIndex = entities[entIdx].glowIndex;
        if (instances[idx].glowIndex >= MATERIAL_IDX_MAX) instances[idx].glowIndex = 41;
        instances[idx].specIndex = entities[entIdx].specIndex;
        if (instances[idx].specIndex >= MATERIAL_IDX_MAX) instances[idx].specIndex = 41;
        instances[idx].normIndex = entities[entIdx].normIndex;
        if (instances[idx].normIndex >= MATERIAL_IDX_MAX) instances[idx].normIndex = 41;
        instances[idx].lodIndex = entities[entIdx].lodIndex;
        instances[idx].position.x += correctionX;
        instances[idx].position.y += correctionY;
        instances[idx].position.z += correctionZ;
    }

    // Instances uploaded after loading statics and dynamics in next functions...
    DualLog(" took %f seconds\n", get_time() - start_time);
    DebugRAM("end of LoadLevelGeometry");
}

void LoadLevelLights(uint8_t curlevel) {
    double start_time = get_time();
    if (curlevel >= numLevels) { DualLogError("Cannot load level lights, level number %d out of bounds 0 to %d\n",curlevel,numLevels - 1); exit(1); }

    DebugRAM("start of LoadLevelLights");
    char filename[64];
    snprintf(filename, sizeof(filename), "./Data/CitadelScene_lights_level%d.txt", curlevel);
    if (!parse_data_file(&lights_parser, filename,1)) { DualLogError("Could not parse %s!\n",filename); exit(1); }

    loadedLights = lights_parser.count;
    DualLog("Loading  %d  statics lights for Level %d...",loadedLights,curlevel);
    float correctionLightX, correctionLightY, correctionLightZ;
    GetLevel_LightsStaticImmutable_ContainerOffsets(curlevel,&correctionLightX,&correctionLightY,&correctionLightZ);
    for (uint32_t i=0;i<loadedLights;++i) {
        uint16_t idx = (i * LIGHT_DATA_SIZE);
        lights[idx + LIGHT_DATA_OFFSET_POSX] = lights_parser.entries[i].position.x + correctionLightX;
        lights[idx + LIGHT_DATA_OFFSET_POSY] = lights_parser.entries[i].position.y + correctionLightY;
        lights[idx + LIGHT_DATA_OFFSET_POSZ] = lights_parser.entries[i].position.z + correctionLightZ;
        lights[idx + LIGHT_DATA_OFFSET_INTENSITY] = lights_parser.entries[i].intensity;
        lights[idx + LIGHT_DATA_OFFSET_RANGE] = lights_parser.entries[i].range;
        float ang = 0.0f;
        if (lights_parser.entries[i].type == 2) ang = 1000.0f; // Indicate this is directional light
        else if (lights_parser.entries[i].type == 1) ang = lights_parser.entries[i].spotAngle; // If spot apply it, else get 0 for spotAng
        
        lights[idx + LIGHT_DATA_OFFSET_SPOTANG] = ang;
        lights[idx + LIGHT_DATA_OFFSET_SPOTDIRX] = lights_parser.entries[i].rotation.x;
        lights[idx + LIGHT_DATA_OFFSET_SPOTDIRY] = lights_parser.entries[i].rotation.y;
        lights[idx + LIGHT_DATA_OFFSET_SPOTDIRZ] = lights_parser.entries[i].rotation.z;
        lights[idx + LIGHT_DATA_OFFSET_SPOTDIRW] = lights_parser.entries[i].rotation.w;
        lights[idx + LIGHT_DATA_OFFSET_R] = lights_parser.entries[i].color.r;
        lights[idx + LIGHT_DATA_OFFSET_G] = lights_parser.entries[i].color.g;
        lights[idx + LIGHT_DATA_OFFSET_B] = lights_parser.entries[i].color.b;
    }
    
    DualLog(" took %f secs\n", get_time() - start_time);
    DebugRAM("end of LoadLevelLights");
}

void LoadLevelDynamicObjects(uint8_t curlevel) {
    double start_time = get_time();
    if (curlevel >= numLevels) { DualLogError("Cannot load level dynamic objects, level number %d out of bounds 0 to %d\n",curlevel,numLevels - 1); exit(1); }

    DebugRAM("start of LoadLevelDynamicObjects");
    char filename[64];
    snprintf(filename, sizeof(filename), "./Data/CitadelScene_dynamics_level%d.txt", curlevel);
    if (!parse_data_file(&dynamics_parser, filename,1)) { DualLogError("Could not parse %s!\n",filename); exit(1); }

    int32_t dynamicObjectCount = dynamics_parser.count;
    DualLog("Loading  %d dynamic objects for Level %d...",dynamicObjectCount,curlevel);
    float correctionX, correctionY, correctionZ;
    GetLevel_Transform_Offsets(curlevel,&correctionX,&correctionY,&correctionZ);
    int32_t startingIdx = (int32_t)loadedInstances;
    physHead = 0;
    for (int32_t idx=loadedInstances, i = 0;idx<(startingIdx + dynamicObjectCount);++idx, ++i) {
        loadedInstances++;
        int32_t entIdx = dynamics_parser.entries[i].index;
        if (entIdx >= MAX_ENTITIES) { DualLogError("Entity index when loading dynamic object %d was %d, exceeds max entity count of %d\n",(idx - startingIdx),entIdx,MAX_ENTITIES); exit(1); }
        
        instances[idx] = dynamics_parser.entries[i];
        instances[idx].modelIndex = entities[entIdx].modelIndex;
        if (instances[idx].modelIndex < loadedModels) renderableCount++;
        instances[idx].texIndex = entities[entIdx].texIndex;
        instances[idx].glowIndex = entities[entIdx].glowIndex;
        if (instances[idx].glowIndex >= MATERIAL_IDX_MAX) instances[idx].glowIndex = 41;
        instances[idx].specIndex = entities[entIdx].specIndex;
        if (instances[idx].specIndex >= MATERIAL_IDX_MAX) instances[idx].specIndex = 41;
        instances[idx].normIndex = entities[entIdx].normIndex;
        if (instances[idx].normIndex >= MATERIAL_IDX_MAX) instances[idx].normIndex = 41;
        instances[idx].lodIndex = entities[entIdx].lodIndex;
        instances[idx].position.x += correctionX;
        instances[idx].position.y += correctionY;
        instances[idx].position.z += correctionZ;
    }

    DualLog(" took %f secs\n", get_time() - start_time);
    DebugRAM("end of LoadLevelDynamicObjects");
}

bool IsDynamicObject(uint16_t constIndex) {
    return (constIndex >= 307 && constIndex <= 404)
            || (constIndex >= 402 && constIndex <= 404)
            ||  constIndex == 417
            || (constIndex >= 419 && constIndex <= 428)
            || (constIndex >= 430 && constIndex <= 437)
            || (constIndex >= 440 && constIndex <= 442)
            || (constIndex >= 458 && constIndex <= 463)
            || (constIndex >= 465 && constIndex <= 476);
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
    for (uint32_t i = 0; i < loadedInstances; i++) {
        if (instances[i].texIndex >= loadedTextures && instances[i].texIndex != UINT16_MAX) { DualLogError("Invalid texIndex %u for instance %u\n", instances[i].texIndex, i); invalidModelIndexCount++; continue; }
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
    uint16_t currentOffset = 0;
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
    uint16_t targetIdx = 0;

    // Copy opaque instances
    for (uint16_t modelIdx = 0; modelIdx < loadedModels; modelIdx++) {
        for (uint16_t j = 0; j < opaqueInstancesHead; j++) {
            uint16_t i = opaqueInstances[j];
            if (tempInstances[i].modelIndex == modelIdx) {
                if (targetIdx >= startOfDoubleSidedInstances) { DualLogError("Opaque instance overflow at modelIdx %u, index %u, targetIdx %u\n", modelIdx, i, targetIdx); exit(1); }
                
                instances[targetIdx] = tempInstances[i];
                targetIdx++;
            }
        }
    }

    // Copy double-sided instances
    for (uint16_t modelIdx = 0; modelIdx < loadedModels; modelIdx++) {
        for (uint16_t j = 0; j < doubleSidedInstancesHead; j++) {
            uint16_t i = doubleSidedInstances[j];
            if (tempInstances[i].modelIndex == modelIdx) {
                if (targetIdx >= startOfTransparentInstances) { DualLogError("Double-sided instance overflow at modelIdx %u, index %u, targetIdx %u\n", modelIdx, i, targetIdx); exit(1); }
                
                instances[targetIdx] = tempInstances[i];
                targetIdx++;
            }
        }
    }

    // Copy transparent instances
    for (uint16_t modelIdx = 0; modelIdx < loadedModels; modelIdx++) {
        for (uint16_t j = 0; j < transparentInstancesHead; j++) {
            uint16_t i = transparentInstances[j];
            if (tempInstances[i].modelIndex == modelIdx) {
                if (targetIdx >= loadedInstances - invalidModelIndexCount) { DualLogError("Transparent instance overflow at modelIdx %u, index %u, targetIdx %u\n", modelIdx, i, targetIdx); exit(1); }
                
                instances[targetIdx] = tempInstances[i];
                targetIdx++;
            }
        }
    }

    // Update cellIndexForInstance
    for (uint16_t i = 0; i < loadedInstances; i++) {
        float x = instances[i].position.x;
        float z = instances[i].position.z;
        int32_t cellX = (int32_t)floorf((x - worldMin_x) / WORLDCELL_WIDTH_F);
        int32_t cellZ = (int32_t)floorf((z - worldMin_z) / WORLDCELL_WIDTH_F);
        cellX = clamp(cellX, 0, 63);
        cellZ = clamp(cellZ, 0, 63);
        cellIndexForInstance[i] = cellZ * 64 + cellX;
    }

    DualLog(" took %f secs\n", get_time() - start_time);
    DualLog("Total opaque instances: %u, double-sided: %u, transparent: %u, invisible: %u\n", opaqueInstancesHead, doubleSidedInstancesHead, transparentInstancesHead, invalidModelIndexCount);
//     DualLog("startOfDoubleSidedInstances %u, startOfTransparentInstances %u, invalidModelIndexCount %u\n", startOfDoubleSidedInstances, startOfTransparentInstances, invalidModelIndexCount);
}
