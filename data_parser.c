#include <stdio.h>
#include <stdlib.h>
#include "os.h" // Operating System calls shim layer.
#include "voxen.h"

uint32_t parse_numberu32(const char* str, const char* line, uint32_t lineNum) {
    if (str == NULL || *str == '\0') { DualLogError("Invalid blank string from line[%d]: %s\n", lineNum+1, line); return 0; }
    while (data_parser_isspace((char)*str)) str++;
    while (data_parser_isspace(*str)) str++;
    if (*str == '+') str++;
    if (*str == '-') { DualLogError("Invalid input, negative not allowed (%s)\n      from line[%d]: %s\n", str, lineNum+1, line); return 0; }
    unsigned long result = 0;
    while (*str >= '0' && *str <= '9') {
        int digit = *str - '0';
        result = result * 10uL + (unsigned long)digit;
        str++;
    }

    return (uint32_t)result;
}

uint16_t parse_numberu16(const char* str, const char* line, uint32_t lineNum) {
    uint32_t retval = parse_numberu32(str, line, lineNum);
    if (retval > UINT16_MAX) { DualLogError("Value %u out of range for uint16_t from line[%d]: %s\n", retval, lineNum+1, line); return 0; }
    return (uint16_t)retval;
}

uint8_t parse_numberu8(const char* str, const char* line, uint32_t lineNum) {
    uint32_t retval = parse_numberu32(str, line, lineNum);
    if (retval > UINT8_MAX) { DualLogError("Value %u out of range for uint8_t from line[%d]: %s\n", retval, lineNum+1, line); return 0; }
    return (uint8_t)retval;
}

bool parse_bool(const char* str, const char* line, uint32_t lineNum) {
    uint32_t parseval = parse_numberu32(str, line, lineNum);
    if (parseval > 1) DualLogWarn("Loaded %u but expected boolean from line[%u]: %s\n",parseval, lineNum+1, line);
    return parseval > 0 ? true : false;
}

float parse_float(const char* str, const char* line, uint32_t lineNum) {
    if (str == NULL || *str == '\0') { DualLogError("Invalid blank string from line[%d]: %s\n", lineNum+1, line); return 0.0f; }
    
    while (data_parser_isspace(*str)) str++;
    bool negative = false;
    if (*str == '-') {
        negative = true;
        str++;
    } else if (*str == '+') {
        str++;
    }

    double value = 0.0;
    bool has_digit = false;
    while (*str >= '0' && *str <= '9') { // Integer part
        value = value * 10.0 + (*str - '0');
        str++;
        has_digit = true;
    }

    if (*str == '.') { // Decimal part
        str++;
        double frac = 0.0;
        double place = 0.1;
        while (*str >= '0' && *str <= '9') {
            frac += (*str - '0') * place;
            place *= 0.1;
            str++;
            has_digit = true;
        }

        value += frac;
    }

    if (!has_digit) return 0.0f;

    if (negative) value = -value;
    return (float)value;
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
    if (c == EOF) *is_eof = 1u;
    else if (c == '\n') *is_newline = 1u;
    return pos > 0;
}

// Unique set separate from savedata path and resource data to keep it focussed
bool process_gamedata_key_value(Entity *entry, const char *key, const char *value, const char *line, uint32_t lineNum) {
    if (!key || !value) { DualLogError("Invalid key-value pair at line %u: %s\n", lineNum, line); return false; }
    
    while (data_parser_isspace(*key)) key++;
    while (data_parser_isspace(*value)) value++;
    char trimmed_key[256];
    char trimmed_value[256];
    strncpy(trimmed_key, key, sizeof(trimmed_key) - 1);
    strncpy(trimmed_value, value, sizeof(trimmed_value) - 1);
    trimmed_key[sizeof(trimmed_key) - 1] = '\0';
    trimmed_value[sizeof(trimmed_value) - 1] = '\0';
    char *key_end = trimmed_key + strlen(trimmed_key) - 1;
    char *val_end = trimmed_value + strlen(trimmed_value) - 1;
    while (key_end > trimmed_key && data_parser_isspace(*key_end)) *key_end-- = '\0';
    while (val_end > trimmed_value && data_parser_isspace(*val_end)) *val_end-- = '\0';
    
         if (strcmp(trimmed_key, "modname") == 0)         { strncpy(Sys_Global.global_modname, trimmed_value, sizeof(Sys_Global.global_modname) - 1); Sys_Global.global_modname[sizeof(Sys_Global.global_modname) - 1] = '\0'; entry->index = 0; } // Game/Mod Definition enforces setting entry index to 0 here, at least one of these must do it.  The game definition only has one index, 0.
    else if (strcmp(trimmed_key, "levelcount") == 0)      Sys_Global.numLevels = parse_numberu8(trimmed_value, line, lineNum);
    else if (strcmp(trimmed_key, "startlevel") == 0)      Sys_Global.startLevel = parse_numberu8(trimmed_value, line, lineNum);
    else return false;
    return true;
}

// Load Game/Mod Definition
void ParseGameData(void) {
    double start_time = get_time();
    const char* filename = "./Data/gamedata.txt";
    DualLog("Loading game definition from %s...",filename);    
    Entity entry;
    InitializeEntity(&entry);
    FILE *gamedatfile = fopen(filename, "r");
    if (!gamedatfile) { DualLogError("\nCannot open %s\n", filename); DualLogError("Could not parse %s!\n", filename); OS_Exit(1); }
    
    uint32_t lineNum = 0;
    bool is_eof;
    while (!feof(gamedatfile)) {
        char token[256];
        bool is_comment, is_newline;
        if (!read_token(gamedatfile, token, sizeof(token), ':', &is_comment, &is_eof, &is_newline, &lineNum)) {
            if (is_comment || is_newline) { lineNum += is_newline; continue; }
        }
        
        char key[256];
        strncpy(key, token, sizeof(key) - 1);
        key[sizeof(key) - 1] = '\0';
        if (!read_token(gamedatfile, token, sizeof(token), '\n', &is_comment, &is_eof, &is_newline, &lineNum)) continue;
        
        process_gamedata_key_value(&entry, key, token, key, lineNum);
        lineNum += 1;
    }
    
    fclose(gamedatfile);
    if (strcmp(Sys_Global.global_modname, "Citadel") == 0) Sys_Global.global_modIsCitadel = true;
    DualLog(" loaded Game Definition for %s:: num levels: %d, start level: %d... took %f secs\n",Sys_Global.global_modname, Sys_Global.numLevels, Sys_Global.startLevel, get_time() - start_time);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
bool parse_data_file(DataParser *parser, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) { DualLogError("Cannot open %s\n", filename); return false; }
    
    char line[1024];
    uint32_t lineNum = 0;
    uint32_t max_index = 0;
    while (fgets(line, sizeof(line), file)) { // First pass: count entries and find max index
        lineNum++;        
        char *start = line;
        while (data_parser_isspace(*start)) start++;
        char *end = start + strlen(start) - 1;
        while (end > start && data_parser_isspace(*end)) { *end = '\0'; end--; }
        if (*start == '\0' || (start[0] == '/' && start[1] == '/')) continue;
        if (line[0] == '#') { continue; }

        char *colon = strchr(start, ':');
        if (colon && strncmp(start, "index", colon - start) == 0) {
            char *value = colon + 1;
            while (data_parser_isspace(*value)) value++;
            uint32_t idx = parse_numberu32(value, line, lineNum);
            if (idx > max_index) max_index = idx;
       }
    }

    if (max_index == 0) { DualLogWarn("No entries found in %s\n", filename); fclose(file); return true; }

    uint32_t entry_count = max_index + 1;
    if (entry_count > parser->capacity) {
        Entity *new_entries = realloc(parser->entries, entry_count * sizeof(Entity));  
        parser->entries = new_entries;
        for (uint32_t i = parser->capacity; i < entry_count; ++i) InitializeEntity(&parser->entries[i]);
        parser->capacity = entry_count;
    }
    
    parser->count = entry_count;
    rewind(file);
    Entity entry;
    InitializeEntity(&entry);
    int32_t entries_stored = 0;
    lineNum = 0;
    int32_t currentChild = -1;
    while (fgets(line, sizeof(line), file)) {
        lineNum++;
        char *start = line;
        if (strlen(start) < 3) continue; // Must have at least k:v, skip if shorter

        while (data_parser_isspace(*start)) start++;
        char *end = start + strlen(start) - 1;
        while (end > start && data_parser_isspace(*end)) { *end = '\0'; end--; }
        if (*start == '\0') continue; // Skip empty line
        if (start[0] == '/' && start[1] == '/') continue; // Skip comment(ed out) line

        if (*start == '#' && *(start + 1) != '#') {
            // Store previous entry if valid
            if (entry.path[0] && entry.index != UINT16_MAX && entry.index < parser->capacity) {
                parser->entries[entry.index] = entry;
                entries_stored++;
            }
            
            // Start new entry
            InitializeEntity(&entry);
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
            while (data_parser_isspace(*key)) key++;
            while (data_parser_isspace(*value)) value++;
            if (*key && *value) {
                while (data_parser_isspace(*key)) key++;
                while (data_parser_isspace(*value)) value++;
                char trimmed_key[256];
                char trimmed_value[256];
                strncpy(trimmed_key, key, sizeof(trimmed_key) - 1);
                strncpy(trimmed_value, value, sizeof(trimmed_value) - 1);
                trimmed_key[sizeof(trimmed_key) - 1] = '\0';
                trimmed_value[sizeof(trimmed_value) - 1] = '\0';
                char *key_end = trimmed_key + strlen(trimmed_key) - 1;
                char *val_end = trimmed_value + strlen(trimmed_value) - 1;
                while (key_end > trimmed_key && data_parser_isspace(*key_end)) *key_end-- = '\0';
                while (val_end > trimmed_value && data_parser_isspace(*val_end)) *val_end-- = '\0';
                if (strncmp(trimmed_key, "chunk_", 6) == 0) {
                    strncpy(entry.path, trimmed_key, sizeof(entry.path) - 1);
                    entry.path[sizeof(entry.path) - 1] = '\0';
                } else {
                         if (strcmp(trimmed_key, "index") == 0)             entry.index = parse_numberu16(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "persistent") == 0)        entry.persistent = parse_bool(trimmed_value, start, lineNum); // Didn't feel worthy of being considered an entflag so left separte
                    else if (strcmp(trimmed_key, "model") == 0)             entry.modelIndex = parse_numberu16(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "animated") == 0)          entry.animated = parse_numberu8(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "texture") == 0)           entry.texIndex = parse_numberu16(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "alttexture") == 0)           entry.altTexIndex = parse_numberu16(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "glowtexture") == 0)       entry.glowIndex = parse_numberu16(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "altglowtexture") == 0)    entry.altGlowIndex = parse_numberu16(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "spectexture") == 0)       entry.specIndex = parse_numberu16(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "normtexture") == 0)       entry.normIndex = parse_numberu16(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "doublesided") == 0)       flag_set(&entry.entflags,ENTFLAG_DOUBLESIDED,parse_bool(trimmed_value, start, lineNum));
                    else if (strcmp(trimmed_key, "transparent") == 0)       flag_set(&entry.entflags,ENTFLAG_TRANSPARENT,parse_bool(trimmed_value, start, lineNum));
                    else if (strcmp(trimmed_key, "cardchunk") == 0)         flag_set(&entry.entflags,ENTFLAG_CARDCHUNK,  parse_bool(trimmed_value, start, lineNum));

                    else if (strcmp(trimmed_key, "collider") == 0)          entry.collider = parse_numberu8(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "collider_centerx") == 0)  entry.colliderCenter.x = parse_float(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "collider_centery") == 0)  entry.colliderCenter.x = parse_float(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "collider_centerz") == 0)  entry.colliderCenter.x = parse_float(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "collider_sizex") == 0)    entry.colliderSize.x = parse_float(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "collider_sizey") == 0)    entry.colliderSize.y = parse_float(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "collider_sizez") == 0)    entry.colliderSize.z = parse_float(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "colliderMeshIndex") == 0) entry.colliderMeshIndex = parse_numberu16(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "mass") == 0)              entry.mass = parse_float(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "linearDrag") == 0)        entry.linearDrag = parse_float(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "angularDrag") == 0)       entry.angularDrag = parse_float(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "kinematic") == 0)         flag_set(&entry.entflags,ENTFLAG_KINEMATIC, parse_bool(trimmed_value, start, lineNum));
                    else if (strcmp(trimmed_key, "useGravity") == 0)        flag_set(&entry.entflags,ENTFLAG_USEGRAVITY,parse_bool(trimmed_value, start, lineNum));
                    else if (strcmp(trimmed_key, "bounciness") == 0)        entry.bounciness = parse_float(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "dynamicFriction") == 0)   entry.dynamicFriction = parse_float(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "frictionCombine") == 0)   entry.frictionCombine = parse_numberu8(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "bounceCombine") == 0)     entry.bounceCombine = parse_numberu8(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "numclips") == 0)          entry.numclips = parse_numberu8(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "animationNum") == 0)      entry.animationNum = parse_numberu8(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "changeMatOnActive") == 0) flag_set(&entry.entflags,ENTFLAG_CHANGE_TEX_ON_ACTIVE,parse_bool(trimmed_value, start, lineNum));
                    else if (strcmp(trimmed_key, "blinkWhenActive") == 0)   flag_set(&entry.entflags,ENTFLAG_BLINK_TEX_ON_ACTIVE,parse_bool(trimmed_value, start, lineNum));
                    else if (strcmp(trimmed_key, "noshadows") == 0)         flag_set(&entry.entflags,ENTFLAG_NO_SHADOWS,parse_bool(trimmed_value, start, lineNum));

                    else if (strcmp(trimmed_key, "volume") == 0)            entry.volume = parse_float(trimmed_value, start, lineNum);
                    
                    else if (strcmp(trimmed_key, "##child") == 0) {
                        ++currentChild;
                        if (currentChild >= MAX_CHILD_COUNT) { DualLogError("Too many children %u! Minivan is full!!\n", currentChild); OS_Exit(1); }
                        
                        entry.child[currentChild] = parse_numberu16(trimmed_value, start, lineNum);
                    } else if (strcmp(trimmed_key, "child_offsetx") == 0)    entry.child_offset[currentChild].x = parse_float(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "child_offsety") == 0)    entry.child_offset[currentChild].y = parse_float(trimmed_value, start, lineNum);
                    else if (strcmp(trimmed_key, "child_offsetz") == 0)    entry.child_offset[currentChild].z = parse_float(trimmed_value, start, lineNum);
                }
            } else DualLogWarn("Invalid key-value pair at line %u: %s\n", lineNum, start);
        } else DualLogWarn("No colon found in line %u: %s\n", lineNum, start);
    }

    // Store last entry
    if (entry.path[0] && entry.index != UINT16_MAX && entry.index < parser->capacity) {
        parser->entries[entry.index] = entry;
        entries_stored++;
    }

    fclose(file);
    return true;
}
#pragma GCC diagnostic pop
