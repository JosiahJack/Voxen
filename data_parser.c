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
DataParser entity_parser;
DataParser lights_parser;

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

void init_data_entry(ResourceEntry *entry) {
    entry->cardchunk = entry->doublesided = entry->transparent = false;
    entry->index = UINT16_MAX;
    entry->modelIndex = entry->lodIndex = MODEL_IDX_MAX;
    entry->texIndex = entry->glowIndex = entry->specIndex = entry->normIndex = MATERIAL_IDX_MAX;
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
    ResourceEntry entry;
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

//-----------------------------------------------------------------------------
// Load all Textures
void LoadTextures(void) {
    malloc_trim(0);
    double start_time = get_time();
    DualLog("Loading textures");
    DebugRAM("start of LoadTextures");
    loadedTextures = 0;
    if (!parse_data_file(&texture_parser, "./Data/textures.txt")) { DualLogError("Could not parse ./Data/textures.txt!\n"); exit(1); }

    int32_t maxIndex = -1;
    for (int32_t k = 0; k < texture_parser.count; k++) {
        if (texture_parser.entries[k].index > maxIndex && texture_parser.entries[k].index != UINT16_MAX) maxIndex = texture_parser.entries[k].index;
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
    uint32_t totalPaletteColorsExtraSized = 80000;
    texturePalettes             = malloc(totalPaletteColorsExtraSized * sizeof(uint32_t));
    int32_t* widths             = malloc(loadedTextures * sizeof(int32_t));
    int32_t* heights            = malloc(loadedTextures * sizeof(int32_t));
    int32_t* matchedParserIdxes = malloc(loadedTextures * sizeof(int32_t));
    for (int32_t i = 0; i < loadedTextures; i++) {
        image_data[i] = NULL;
        widths[i] = heights[i] = 0;
        matchedParserIdxes[i] = -1;
    }

    for (int32_t k = 0; k < texture_parser.count; k++) { // Match parser entries to indices ahead of loops
        if (texture_parser.entries[k].index < loadedTextures) matchedParserIdxes[texture_parser.entries[k].index] = k;
    }
    
    #pragma omp parallel
    {
        #pragma omp for schedule(dynamic)
        for (int32_t i = 0; i < loadedTextures; i++) {
            if (matchedParserIdxes[i] < 0) continue;
            struct stat file_stat;
            if (stat(texture_parser.entries[matchedParserIdxes[i]].path, &file_stat) != 0) { DualLogError("Failed to stat %s for texture index %u against matchedParserIdx %u: %s\n", texture_parser.entries[matchedParserIdxes[i]].path, i, matchedParserIdxes[i], strerror(errno)); continue; }
                
            size_t file_size = file_stat.st_size;
            if (file_size > 512000) { DualLogError("PNG file %s too large (%zu bytes), larger than 512000 bytes\n", texture_parser.entries[matchedParserIdxes[i]].path, file_size); exit(1); }
            
            uint8_t* file_buffer = malloc(file_size);
            FILE* fp = fopen(texture_parser.entries[matchedParserIdxes[i]].path, "rb");
            if (!fp) { DualLogError("Failed to open %s: %s\n", texture_parser.entries[matchedParserIdxes[i]].path, strerror(errno)); exit(1); }
            
            fread(file_buffer, 1, file_size, fp);
            fclose(fp);
            int w, h, n;
            image_data[i] = stbi_load_from_memory(file_buffer, file_size, &w, &h, &n, STBI_rgb_alpha);
            if (!image_data[i]) { DualLogError("stbi_load failed for %s: %s\n", texture_parser.entries[matchedParserIdxes[i]].path, stbi_failure_reason()); exit(1); }
            
            widths[matchedParserIdxes[i]] = w;
            heights[matchedParserIdxes[i]] = h;
            doubleSidedTexture[matchedParserIdxes[i]] = texture_parser.entries[matchedParserIdxes[i]].doublesided > 0 ? 1 : 0;
            transparentTexture[matchedParserIdxes[i]] = texture_parser.entries[matchedParserIdxes[i]].transparent > 0 ? 1 : 0;
            free(file_buffer);
            malloc_trim(0);
        }
    }

    malloc_trim(0);
    GLuint stagingBuffer;
    glGenBuffers(1, &stagingBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, stagingBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, (((MAX_TEXTURE_DIMENSION * MAX_TEXTURE_DIMENSION) + 3) / 4) * sizeof(uint32_t), NULL, GL_DYNAMIC_COPY);
    uint32_t max_total_pixels = 33438148; // From colorBufferSize
    int32_t colorBufferSize = (((int32_t)max_total_pixels + 3) / 4) * sizeof(uint32_t);
    ColorEntry* color_pool = malloc(loadedTextures * MAX_PALETTE_SIZE * sizeof(ColorEntry));
    uint32_t* pool_indices = malloc(loadedTextures * sizeof(uint32_t));
    memset(pool_indices, 0, loadedTextures * sizeof(uint32_t));
    uint32_t** per_texture_palettes = malloc(loadedTextures * sizeof(uint32_t*));
    uint32_t* per_texture_palette_sizes = malloc(loadedTextures * sizeof(uint32_t));
    uint8_t* all_indices = malloc(max_total_pixels * sizeof(uint8_t));
    uint32_t* index_offsets = malloc(loadedTextures * sizeof(uint32_t));
    uint32_t current_index_offset = 0;
    for (uint16_t i = 0; i < loadedTextures; i++) {
        per_texture_palettes[i] = malloc(MAX_PALETTE_SIZE * sizeof(uint32_t));
        per_texture_palette_sizes[i] = 0;
        index_offsets[i] = current_index_offset;
        if (matchedParserIdxes[i] >= 0 && image_data[i]) current_index_offset += widths[i] * heights[i];
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
                    if (palette_size >= MAX_PALETTE_SIZE) { DualLogError("Palette size exceeded for %s\n", texture_parser.entries[matchedParserIdxes[i]].path); exit(1); }
                    
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
    
    malloc_trim(0);
    colorBufferID = SetupSSBO(colorBufferID, 12, colorBufferSize, NULL, GL_STATIC_DRAW);
    uint32_t pixel_offset = 0, palette_offset = 0;
    for (uint16_t i = 0; i < loadedTextures; i++) {
        if (matchedParserIdxes[i] < 0 || !image_data[i]) continue;
        textureOffsets[i] = totalPixels;
        texturePaletteOffsets[i] = totalPaletteColors;
        textureSizes[i * 2] = widths[i];
        textureSizes[(i * 2) + 1] = heights[i];
        uint32_t palette_size = per_texture_palette_sizes[i];
        memcpy(&texturePalettes[palette_offset], per_texture_palettes[i], palette_size * sizeof(uint32_t));
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, stagingBuffer);
        int32_t numberOfPixelsForThisTexture = widths[i] * heights[i];
        uint32_t* mapped_buffer = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, ((numberOfPixelsForThisTexture + 3) / 4) * sizeof(uint32_t), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
        uint8_t* texture_indices = &all_indices[index_offsets[i]];
        for (int32_t j = 0; j < numberOfPixelsForThisTexture; j += 4) {
            uint32_t packed = (uint32_t)texture_indices[j];
            if (j + 1 < numberOfPixelsForThisTexture) packed |= (uint32_t)texture_indices[j + 1] << 8;
            if (j + 2 < numberOfPixelsForThisTexture) packed |= (uint32_t)texture_indices[j + 2] << 16;
            if (j + 3 < numberOfPixelsForThisTexture) packed |= (uint32_t)texture_indices[j + 3] << 24;
            mapped_buffer[j / 4] = packed;
        }

        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glBindBuffer(GL_COPY_READ_BUFFER, stagingBuffer);
        glBindBuffer(GL_COPY_WRITE_BUFFER, colorBufferID);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, ((pixel_offset + 3) / 4) * sizeof(uint32_t), ((numberOfPixelsForThisTexture + 3) / 4) * sizeof(uint32_t));
        pixel_offset += numberOfPixelsForThisTexture;
        palette_offset += palette_size;
        totalPixels += numberOfPixelsForThisTexture;
        totalPaletteColors += palette_size;
        stbi_image_free(image_data[i]);
        image_data[i] = NULL;
        malloc_trim(0);
    }

    DualLog(" total pallete colors: %u, totalPixels was: %u... \n", totalPaletteColors, totalPixels);
    free(all_indices);
    free(index_offsets);
    for (uint16_t i = 0; i < loadedTextures; i++) free(per_texture_palettes[i]);
    free(per_texture_palettes);
    free(per_texture_palette_sizes);
    free(color_pool);
    free(pool_indices);
    free(image_data);
    free(widths);
    free(heights);
    free(matchedParserIdxes);
    glDeleteBuffers(1, &stagingBuffer);
    malloc_trim(0);
    texturePalettesID = SetupSSBO(texturePalettesID, 16, totalPaletteColors * sizeof(uint32_t), texturePalettes, GL_STATIC_DRAW);
    free(texturePalettes);
    textureOffsetsID = SetupSSBO(textureOffsetsID, 14, loadedTextures * sizeof(uint32_t), textureOffsets, GL_STATIC_DRAW);
    free(textureOffsets);
    textureSizesID = SetupSSBO(textureSizesID, 15, loadedTextures * 2 * sizeof(int32_t), textureSizes, GL_STATIC_DRAW);
    free(textureSizes);
    texturePaletteOffsetsID = SetupSSBO(texturePaletteOffsetsID, 17, loadedTextures * sizeof(uint32_t), texturePaletteOffsets, GL_STATIC_DRAW);
    free(texturePaletteOffsets);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glFlush();
    glFinish();
    CHECK_GL_ERROR();
    malloc_trim(0);
    double end_time = get_time();
    DualLog(" took %f seconds\n", end_time - start_time);
    DebugRAM("After LoadTextures");
}

GLuint SetupSSBO(GLuint id, GLuint bindingIndex, GLsizeiptr size, const void* data, GLenum usage) {
    if (id != 0) glDeleteBuffers(1, &id); // Clear last level's SSBO.
    GLuint new_id = 0;
    glGenBuffers(1, &new_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, new_id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, usage);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingIndex, new_id);
    return new_id;
}

//-----------------------------------------------------------------------------
// Loads all 3D meshes
void LoadModels(void) {
    double start_time = get_time();
    DebugRAM("start of LoadModels");
    loadedModels = 0;
    if (!parse_data_file(&model_parser, "./Data/models.txt")) { DualLogError("Could not parse ./Data/models.txt!\n"); exit(1); }

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
void LoadEntities(void) {
    double start_time = get_time();
    
    // Initialize parser with entity-specific keys
    if (!parse_data_file(&entity_parser, "./Data/entities.txt")) { DualLogError("Could not parse ./Data/entities.txt!\n"); exit(1); }
    
    entityCount = entity_parser.count;
    if (entityCount > MAX_ENTITIES) { DualLogError("Too many entities in parser count %d, greater than %d!\n", entityCount, MAX_ENTITIES); exit(1); }
    if (entityCount == 0) { DualLogError("No entities found in entities.txt\n"); exit(1); }

    DualLog("Loading  %d entities...", entityCount);

    // Populate entities array
    for (int32_t i = 0; i < entityCount; i++) {
        if (entity_parser.entries[i].index == UINT16_MAX) continue;

        entities[i].index = entity_parser.entries[i].index;
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
        entities[i].scale.x = 1.0f;
        entities[i].scale.y = 1.0f;
        entities[i].scale.z = 1.0f;
        entities[i].rotation.x = 0.0f;
        entities[i].rotation.y = 0.0f;
        entities[i].rotation.z = 0.0f;
        entities[i].rotation.w = 1.0f;
    }

    DualLog(" took %f seconds\n", get_time() - start_time);
    DebugRAM("after loading all entities");
}

//----------------------------------- Level -----------------------------------
void LoadLevel(uint8_t curlevel) {
    DebugRAM("start of LoadLevel");
    double start_time = get_time();
    memset(instances,0,INSTANCE_COUNT * sizeof(Entity)); // Initialize instances, the global entity array for the currently loaded level.
    for (uint16_t idx = 0;idx<INSTANCE_COUNT;idx++) {
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
    int32_t instanceIdx = -1;
    int32_t lightsIdx = -1;
    size_t lineLengthMax = 81920; 
    char lineSpace[lineLengthMax];
    char* line = &lineSpace[0];
    char firstKeyCheck[11];
    char initialLine[lineLengthMax];
    float correctionX, correctionY, correctionZ;
    GetLevel_Transform_Offsets(curlevel,&correctionX,&correctionY,&correctionZ);
//     float correctionDynamicX, correctionDynamicY, correctionDynamicZ;
//     GetLevel_Transform_Offsets(curlevel,&correctionDynamicX,&correctionDynamicY,&correctionDynamicZ);
    float correctionLightX, correctionLightY, correctionLightZ;
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
                else if (strcmp(trimmed_key, "go.activeSelf") == 0)   instances[instanceIdx].active = parse_bool(trimmed_value, initialLine, lineNum);
            }
        }
        
        if (isLight) {
            loadedLights++;
            if (lightType == 1) {
                if (lights[litIdx + LIGHT_DATA_OFFSET_SPOTANG] < 5.0f) DualLogWarn("Light %d on line %d loaded with spotAngle less than 5deg but was marked as spotlight type!\n",lightsIdx,lineNum);
            } else if (lightType == 2) {
                // TODO: Handle directional lights for cyberspace
                lights[litIdx + LIGHT_DATA_OFFSET_SPOTANG] = 0.0f; // Force to not be a spot light
            } else {
                lights[litIdx + LIGHT_DATA_OFFSET_SPOTANG] = 0.0f; // Force to not be a spot light
            }
        } else {
            loadedInstances++;
            uint16_t entIdx = instances[lineNum].index;
            if (entIdx >= MAX_ENTITIES) { DualLogError("\nEntity index when loading level geometry object %d was %d, exceeds max entity count of %d\n",lineNum,entIdx,MAX_ENTITIES); exit(1); }
            
            instances[lineNum].modelIndex = entities[entIdx].modelIndex;
            if (instances[lineNum].modelIndex < loadedModels) renderableCount++;
            instances[lineNum].texIndex = entities[entIdx].texIndex;
            instances[lineNum].glowIndex = entities[entIdx].glowIndex;
            if (instances[lineNum].glowIndex >= MATERIAL_IDX_MAX) instances[lineNum].glowIndex = 41;
            instances[lineNum].specIndex = entities[entIdx].specIndex;
            if (instances[lineNum].specIndex >= MATERIAL_IDX_MAX) instances[lineNum].specIndex = 41;
            instances[lineNum].normIndex = entities[entIdx].normIndex;
            if (instances[lineNum].normIndex >= MATERIAL_IDX_MAX) instances[lineNum].normIndex = 41;
            instances[lineNum].lodIndex = entities[entIdx].lodIndex;
            if (ConstIndexIsDoor(entIdx)) {
                instances[lineNum].position.x += correctionX + 0.6001f;
                instances[lineNum].position.y += correctionY - 0.5681f;
                instances[lineNum].position.z += correctionZ -0.905f;
//             if (ConstIndexIsDynamicObject) {
//                 instances[lineNum].position.x += correctionDynamicX; // TODO
//                 instances[lineNum].position.y += correctionDynamicY;
//                 instances[lineNum].position.z += correctionDynamicZ;
            } else {
                instances[lineNum].position.x += correctionX;
                instances[lineNum].position.y += correctionY;
                instances[lineNum].position.z += correctionZ;
            }
        }
    }

    fclose(file);
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
    for (uint32_t i = 0; i < loadedInstances; i++) {
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
}
