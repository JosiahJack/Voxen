#include "data_definitions.h"
#include <SDL2/SDL.h>
#include <string.h>
#include <stdlib.h>

void parser_init(DataParser *parser, const char **valid_keys, int num_keys) {
    parser->entries = NULL;
    parser->count = 0;
    parser->capacity = 0;
    parser->valid_keys = valid_keys;
    parser->num_keys = num_keys;
}

bool parse_data_file(DataParser *parser, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        SDL_Log("Error: Cannot open %s", filename);
        return false;
    }

    // First pass: Count entries and max index
    int max_index = -1;
    int entry_count = 0;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        if (line[0] == '#') {
            entry_count++;
        } else if (strncmp(line, "index: ", 7) == 0) {
            int idx = atoi(line + 7);
            if (idx > max_index) max_index = idx;
        }
    }
    entry_count = max_index >= entry_count ? max_index + 1 : entry_count;
    if (entry_count > MAX_ENTRIES) {
        SDL_Log("Error: Entry count %d exceeds %d", entry_count, MAX_ENTRIES);
        entry_count = MAX_ENTRIES;
    }

    // Resize entries array
    if (entry_count > parser->capacity) {
        DataEntry *new_entries = realloc(parser->entries, entry_count * sizeof(DataEntry));
        if (!new_entries) {
            SDL_Log("Error: realloc failed for %d entries", entry_count);
            fclose(file);
            return false;
        }
        parser->entries = new_entries;
        for (int i = parser->capacity; i < entry_count; i++) {
            parser->entries[i].path[0] = 0;
            parser->entries[i].index = UINT16_MAX; // Mark unused
        }
        parser->capacity = entry_count;
    }
    parser->count = entry_count;

    // Second pass: Parse entries
    rewind(file);
    DataEntry entry = {0};
    entry.index = UINT16_MAX;
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        if (line[0] == '#') {
            if (entry.path[0] && entry.index < MAX_ENTRIES) {
                parser->entries[entry.index] = entry;
            }
            strncpy(entry.path, line + 1, sizeof(entry.path) - 1);
            entry.index = UINT16_MAX;
        } else if (line[0] == '/' && line[1] == '/') {
            continue; // Skip comments
        } else {
            // Check valid keys
            for (int i = 0; i < parser->num_keys; i++) {
                size_t key_len = strlen(parser->valid_keys[i]);
                if (strncmp(line, parser->valid_keys[i], key_len) == 0 && line[key_len] == ':') {
                    if (strcmp(parser->valid_keys[i], "index") == 0) {
                        entry.index = atoi(line + key_len + 2);
                    }
                    // Add more key handlers here (e.g., format, type)
                    break;
                }
            }
        }
    }
    if (entry.path[0] && entry.index < MAX_ENTRIES) {
        parser->entries[entry.index] = entry;
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
