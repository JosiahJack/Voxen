#include <SDL2/SDL.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include "data_parser.h"
#include "data_levels.h"
#include "debug.h"

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

static bool parse_type1(DataParser *parser, const char *filename) {
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

static bool parse_type2(DataParser *parser, const char *filename) {
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

static bool parse_type3(DataParser *parser, const char *filename) {
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
        case PARSER_DATA:
            return parse_type1(parser, filename);
        case PARSER_GAME:
            return parse_type2(parser, filename);
        case PARSER_LEVEL:
            return parse_type3(parser, filename);
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
