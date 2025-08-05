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
    if (!file) { DualLogError("Cannot open %s", filename); return false; }

    // First pass: Count entries and max index
    int max_index = -1;
    int entry_count = 0;
    char line[MAX_PATH];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        if (line[0] == '\0' || (line[0] == '/' && line[1] == '/')) continue;
        if (line[0] == '#') {
            entry_count++;
        } else if (strncmp(line, "index: ", 7) == 0) {
            int idx = atoi(line + 7);
            if (idx > max_index) max_index = idx;
        }
    }
    
    entry_count = max_index >= entry_count ? max_index + 1 : entry_count;
    if (entry_count > MAX_ENTRIES) {
        DualLogError("Entry count %d exceeds %d", entry_count, MAX_ENTRIES);
        entry_count = MAX_ENTRIES;
    }

    // Resize entries array
    if (entry_count > parser->capacity) {
        DataEntry *new_entries = realloc(parser->entries, entry_count * sizeof(DataEntry));
        if (!new_entries) { DualLogError("realloc failed for %d entries", entry_count); fclose(file); return false; }
        
        parser->entries = new_entries;
        for (int i = parser->capacity; i < entry_count; i++) {
            parser->entries[i].path[0] = 0;
            parser->entries[i].index = UINT16_MAX; // Mark unused
            parser->entries[i].modelIndex = UINT16_MAX;
            parser->entries[i].texIndex = UINT16_MAX;
            parser->entries[i].glowIndex = UINT16_MAX;
            parser->entries[i].specIndex = UINT16_MAX;
            parser->entries[i].normIndex = UINT16_MAX;
        }
        
        parser->capacity = entry_count;
    }
    
    parser->count = entry_count;

    // Second pass: Parse entries
    rewind(file);
    DataEntry entry = {0};
    entry.index = UINT16_MAX;
    uint32_t lineNum = 0;
    printf("Made it to here 1\n");
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        printf("Made it to here 2\n");
        bool is_pipe_delimited = (line[0] != '#');
        if (!is_pipe_delimited) {
            if (entry.path[0] && entry.index < MAX_ENTRIES) parser->entries[entry.index] = entry;
            strncpy(entry.path, line + 1, sizeof(entry.path) - 1);
            entry.index = UINT16_MAX;
            entry.modelIndex = UINT16_MAX;
            entry.texIndex = UINT16_MAX;
            entry.glowIndex = UINT16_MAX;
            entry.specIndex = UINT16_MAX;
            entry.normIndex = UINT16_MAX;
            entry.doublesided = false;
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
        } else if ((line[0] == '/' && line[1] == '/') || (line[0] == '\\' && line[1] == 'n')) {
            continue; // Skip comments and empty lines
        } else { // Pipe Delimited key:value pairs that are colon separated.  E.g. key:value|key:value|key:value
            printf("Made it to here 3\n");
            int numKVPairs = count_pipes(line);
            do { // Check valid keys
                for (int i = 0; i < parser->num_keys; i++) {
                    size_t key_len = strlen(parser->valid_keys[i]);
                    if (strncmp(line, parser->valid_keys[i], key_len) == 0 && line[key_len] == ':') {
                        const char *value = line + key_len + 1;
                        while (*value == ' ') value++; // Skip the space after :
                             if (strcmp(parser->valid_keys[i], "index") == 0)           entry.index           = parse_numberu16(value,line,lineNum);
                        else if (strcmp(parser->valid_keys[i], "model") == 0)           entry.modelIndex      = parse_numberu16(value,line,lineNum);
                        else if (strcmp(parser->valid_keys[i], "texture") == 0)         entry.texIndex        = parse_numberu16(value,line,lineNum);
                        else if (strcmp(parser->valid_keys[i], "glowtexture") == 0)     entry.glowIndex       = parse_numberu16(value,line,lineNum);
                        else if (strcmp(parser->valid_keys[i], "spectexture") == 0)     entry.specIndex       = parse_numberu16(value,line,lineNum);
                        else if (strcmp(parser->valid_keys[i], "normtexture") == 0)     entry.normIndex       = parse_numberu16(value,line,lineNum);
                        else if (strcmp(parser->valid_keys[i], "doublesided") == 0)     entry.doublesided     = parse_bool(value,line,lineNum);
                        else if (strcmp(parser->valid_keys[i], "cardchunk") == 0)       entry.cardchunk       = parse_bool(value,line,lineNum);
                        else if (strcmp(parser->valid_keys[i], "constIndex") == 0)      entry.constIndex      = parse_numberu32(value, line, lineNum);
                        else if (strcmp(parser->valid_keys[i], "localPosition.x") == 0) entry.localPosition.x = parse_float(value, line, lineNum);
                        else if (strcmp(parser->valid_keys[i], "localPosition.y") == 0) entry.localPosition.y = parse_float(value, line, lineNum);
                        else if (strcmp(parser->valid_keys[i], "localPosition.z") == 0) entry.localPosition.z = parse_float(value, line, lineNum);
                        else if (strcmp(parser->valid_keys[i], "localRotation.x") == 0) entry.localRotation.x = parse_float(value, line, lineNum);
                        else if (strcmp(parser->valid_keys[i], "localRotation.y") == 0) entry.localRotation.y = parse_float(value, line, lineNum);
                        else if (strcmp(parser->valid_keys[i], "localRotation.z") == 0) entry.localRotation.z = parse_float(value, line, lineNum);
                        else if (strcmp(parser->valid_keys[i], "localRotation.w") == 0) entry.localRotation.w = parse_float(value, line, lineNum);
                        else if (strcmp(parser->valid_keys[i], "localScale.x") == 0)    entry.localScale.x    = parse_float(value, line, lineNum);
                        else if (strcmp(parser->valid_keys[i], "localScale.y") == 0)    entry.localScale.y    = parse_float(value, line, lineNum);
                        else if (strcmp(parser->valid_keys[i], "localScale.z") == 0)    entry.localScale.z    = parse_float(value, line, lineNum);
                        
                        break;
                    }
                }
                
                numKVPairs--;
            } while (numKVPairs > 0);
        }
        
        lineNum++;
    }
    
    printf("Made it to here 4\n");

    if (entry.path[0] && entry.index < MAX_ENTRIES) parser->entries[entry.index] = entry;
    fclose(file);
    return true;
}

void parser_free(DataParser *parser) {
    free(parser->entries);
    parser->entries = NULL;
    parser->count = 0;
    parser->capacity = 0;
}
