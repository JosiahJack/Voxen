#include <SDL2/SDL.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include "data_parser.h"
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
    
    while (isspace((unsigned char)*str)) str++; // Trim leading whitespace so we can trivially check for negative (in spite of strtoul trimming leading whitespace itself)
    if (*str == '-') { fprintf(stderr, "Invalid input, negative not allowed (%s), from line: %s\n", str, line); return 0; }
    
    char* endptr;
    errno = 0;
    unsigned long val = strtoul(str, &endptr, 10); // Base 10 for decimal.
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
    if (retval > UINT16_MAX) { fprintf(stderr, "Value out of range for uint8_t: %u from line[%d]: %s\n", retval, lineNum, line); return 0; }
    return (uint8_t)retval;
}

bool parse_bool(const char* str, const char* line, uint32_t lineNum) {
    return (parse_numberu32(str,line,lineNum) > 0);
}

float parse_float(const char* str, const char* line, uint32_t lineNum) {
    if (str == NULL || *str == '\0') { fprintf(stderr, "Invalid float input blank string, from line[%d]: %s\n", lineNum, line); return 0.0f; }

    char* endptr;
    errno = 0;
    float val = strtof(str, &endptr);
    if (errno != 0 || endptr == str || *endptr != '\0') { fprintf(stderr, "Invalid float input %s, from line[%d]: %s\n", str, lineNum, line); return 0.0f; }
    return val;
}

int count_pipes(char *str) {
    int count = 0;
    int len = strlen(str);
    for (int i = 0; i < len; i++) {
        if (str[i] == '|') count++;
    }
    
    return count;
}

bool parse_data_file(DataParser *parser, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        DualLogError("Cannot open %s\n", filename);
        return false;
    }

    // First pass: Count entries and max index
    int max_index = -1;
    int entry_count = 0;
    char line[MAX_LINE_LENGTH];
    bool is_texture_or_model = (strstr(filename, "textures.txt") != NULL || strstr(filename, "models.txt") != NULL || strstr(filename, "entities.txt") != NULL);
//     bool isGameData = (strstr(filename,"gamedata.txt") != NULL);
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        if (line[0] == '\0' || (line[0] == '/' && line[1] == '/')) continue;
        if (is_texture_or_model) {
            if (line[0] == '#') {
                entry_count++;
            } else if (strncmp(line, "index: ", 7) == 0) {
                int idx = atoi(line + 7);
                if (idx > max_index) max_index = idx;
            }
        } else {
            entry_count++; // For gamedata.txt and level data, count non-comment lines
        }
    }

    // Adjust entry count for textures/models
    if (is_texture_or_model) {
        entry_count = max_index >= entry_count ? max_index + 1 : entry_count;
    }
    if (entry_count > MAX_ENTRIES) {
        DualLogError("Entry count %d exceeds %d\n", entry_count, MAX_ENTRIES);
        entry_count = MAX_ENTRIES;
    }

    // Allocate memory
    if (entry_count > parser->capacity) {
        DataEntry *new_entries = realloc(parser->entries, entry_count * sizeof(DataEntry));
        if (!new_entries) {
            DualLogError("realloc failed for %d entries\n", entry_count);
            fclose(file);
            return false;
        }
        parser->entries = new_entries;
        for (int i = parser->capacity; i < entry_count; i++) {
            parser->entries[i].path[0] = 0;
            parser->entries[i].index = UINT16_MAX;
            parser->entries[i].modelIndex = UINT16_MAX;
            parser->entries[i].texIndex = UINT16_MAX;
            parser->entries[i].glowIndex = UINT16_MAX;
            parser->entries[i].specIndex = UINT16_MAX;
            parser->entries[i].normIndex = UINT16_MAX;
            parser->entries[i].doublesided = false;
            parser->entries[i].cardchunk = false;
            parser->entries[i].constIndex = 0;
            parser->entries[i].localPosition.x = 0.0f;
            parser->entries[i].localPosition.y = 0.0f;
            parser->entries[i].localPosition.z = 0.0f;
            parser->entries[i].localRotation.x = 0.0f;
            parser->entries[i].localRotation.y = 0.0f;
            parser->entries[i].localRotation.z = 0.0f;
            parser->entries[i].localRotation.w = 1.0f;
            parser->entries[i].localScale.x = 1.0f;
            parser->entries[i].localScale.y = 1.0f;
            parser->entries[i].localScale.z = 1.0f;
        }
        parser->capacity = entry_count;
    }
    
    parser->count = entry_count;

    // Second pass: Parse entries
    rewind(file);
    DataEntry entry = {0};
    entry.index = UINT16_MAX;
    entry.localRotation.w = 1.0f;
    entry.localScale.x = 1.0f;
    entry.localScale.y = 1.0f;
    entry.localScale.z = 1.0f;
    uint32_t lineNum = 0;
    int current_index = 0;
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        if (line[0] == '\0' || (line[0] == '/' && line[1] == '/')) continue;

        bool is_valid_entry = false;
        if (line[0] == '#') {
            bool is_pipe_delimited = (strchr(line, '|') != NULL);
            char *token = strtok(line, is_pipe_delimited ? "|" : "\n");

            // Handle # lines for textures/models
            if (entry.path[0] && entry.index < MAX_ENTRIES) {
                parser->entries[entry.index] = entry;
            }
            strncpy(entry.path, token + 1, sizeof(entry.path) - 1);
            entry.index = UINT16_MAX;
            entry.modelIndex = UINT16_MAX;
            entry.texIndex = UINT16_MAX;
            entry.glowIndex = UINT16_MAX;
            entry.specIndex = UINT16_MAX;
            entry.normIndex = UINT16_MAX;
            entry.doublesided = false;
            entry.cardchunk = false;
            entry.constIndex = 0;
            entry.localPosition.x = 0.0f;
            entry.localPosition.y = 0.0f;
            entry.localPosition.z = 0.0f;
            entry.localRotation.x = 0.0f;
            entry.localRotation.y = 0.0f;
            entry.localRotation.z = 0.0f;
            entry.localRotation.w = 1.0f;
            entry.localScale.x = 1.0f;
            entry.localScale.y = 1.0f;
            entry.localScale.z = 1.0f;
            lineNum++;
            continue;
        }

        // Process key-value pairs
        char *pipe_token = strtok(line, "|");
        while (pipe_token != NULL) {
            char *key = strtok(pipe_token, ":");
            char *value = strtok(NULL, ":");

            if (key && value) {
                while (*value == ' ') value++; // Trim leading spaces
                if (strncmp(key, "chunk_", 6) == 0) {
                    strncpy(entry.path, key, sizeof(entry.path) - 1);
                } else {
                    for (int i = 0; i < parser->num_keys; i++) {
                        if (strcmp(key, parser->valid_keys[i]) == 0) {
                            is_valid_entry = true;
                            DualLog("Parsing line %d with Key: %s Value: %s\n", lineNum, key, value);
                                 if (strcmp(key, "index") == 0)            entry.index = parse_numberu16(value, line, lineNum);
                            else if (strcmp(key, "model") == 0)           entry.modelIndex = parse_numberu16(value, line, lineNum);
                            else if (strcmp(key, "texture") == 0)         entry.texIndex = parse_numberu16(value, line, lineNum);
                            else if (strcmp(key, "glowtexture") == 0)     entry.glowIndex = parse_numberu16(value, line, lineNum);
                            else if (strcmp(key, "spectexture") == 0)     entry.specIndex = parse_numberu16(value, line, lineNum);
                            else if (strcmp(key, "normtexture") == 0)     entry.normIndex = parse_numberu16(value, line, lineNum);
                            else if (strcmp(key, "doublesided") == 0)     entry.doublesided = parse_bool(value, line, lineNum);
                            else if (strcmp(key, "cardchunk") == 0)       entry.cardchunk = parse_bool(value, line, lineNum);
                            else if (strcmp(key, "levelcount") == 0) {    entry.levelCount = parse_numberu16(value, line, lineNum); entry.index = 0; }
                            else if (strcmp(key, "startlevel") == 0) {    entry.startLevel = parse_numberu16(value, line, lineNum); entry.index = 0; }
                            else if (strcmp(key, "constIndex") == 0)      entry.constIndex = parse_numberu32(value, line, lineNum);
                            else if (strcmp(key, "localPosition.x") == 0) entry.localPosition.x = parse_float(value, line, lineNum);
                            else if (strcmp(key, "localPosition.y") == 0) entry.localPosition.y = parse_float(value, line, lineNum);
                            else if (strcmp(key, "localPosition.z") == 0) entry.localPosition.z = parse_float(value, line, lineNum);
                            else if (strcmp(key, "localRotation.x") == 0) entry.localRotation.x = parse_float(value, line, lineNum);
                            else if (strcmp(key, "localRotation.y") == 0) entry.localRotation.y = parse_float(value, line, lineNum);
                            else if (strcmp(key, "localRotation.z") == 0) entry.localRotation.z = parse_float(value, line, lineNum);
                            else if (strcmp(key, "localRotation.w") == 0) entry.localRotation.w = parse_float(value, line, lineNum);
                            else if (strcmp(key, "localScale.x") == 0)    entry.localScale.x = parse_float(value, line, lineNum);
                            else if (strcmp(key, "localScale.y") == 0)    entry.localScale.y = parse_float(value, line, lineNum);
                            else if (strcmp(key, "localScale.z") == 0)    entry.localScale.z = parse_float(value, line, lineNum);
                            break;
                        }
                    }
                }
            } else {
                DualLogError("Invalid key-value pair at line %u: %s\n", lineNum, line);
            }

            pipe_token = strtok(NULL, "|");
        }

        // Store entry for non-texture/model files
        if (!is_texture_or_model && is_valid_entry) {
            if (current_index < parser->count) {
                parser->entries[current_index++] = entry;
            }
            entry = (DataEntry){0};
            entry.index = UINT16_MAX;
            entry.localRotation.w = 1.0f;
            entry.localScale.x = 1.0f;
            entry.localScale.y = 1.0f;
            entry.localScale.z = 1.0f;
        }

        lineNum++;
    }

    if (is_texture_or_model && entry.path[0] && (entry.constIndex != 0 || entry.levelCount != 0 || entry.startLevel != 0) && current_index < parser->count) {
        parser->entries[current_index] = entry;
    }
    if (!is_texture_or_model) {
        parser->count = current_index; // Update with valid entries for gamedata.txt and level data
    }
    
    fclose(file);
    return true;
}

void parser_free(DataParser *parser) {
    free(parser->entries);
    parser->entries = NULL;
    parser->count = 0;
    parser->capacity = 0;
}
