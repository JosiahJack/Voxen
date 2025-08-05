#include <stdio.h>
#include <stdint.h>
#include "levels.h"
#include "data_parser.h"
#include "debug.h"

int LoadLevelGeometry(uint8_t curlevel) {
    DualLog("Loading start level: %d...\n",curlevel);
//     if (curlevel < 0 || curlevel >= numLevels) { DualLogError("Cannot load level %d, out of bounds 0 to %d\n",curlevel,numLevels - 1); return 1; }
//     
//     char filename[64];
//     snprintf(filename, sizeof(filename), "CitadelScene_geometry_level%d.txt", curlevel);
//     FILE* file = fopen(filename, "r");
//     if (!file) { DualLogError("Geometry input file path invalid: %s\n", filename); return 1; }
// 
//     GameObjectArray* container = &lm->geometry_containers[curlevel];
//     container->objects = malloc(100 * sizeof(GameObject)); // Initial capacity
//     container->capacity = 100;
//     container->count = 0;
// 
//     char line[MAX_LINE_LENGTH];
//     while (fgets(line, MAX_LINE_LENGTH, file)) {
//         line[strcspn(line, "\n")] = 0;
//         char* entries[64];
//         int entry_count = 0;
//         char* token = strtok(line, "|");
//         while (token && entry_count < 64) {
//             entries[entry_count++] = token;
//             token = strtok(NULL, "|");
//         }
// 
//         GameObject go = {0};
//         go.id = atoi(entries[0]); // Assuming first entry is constIndex
//         // Parse position, rotation, etc. from entries (implement based on your file format)
//         go.mesh_id = load_mesh(entries[1]); // Example: second entry is mesh name
//         go.material_id = load_material(entries[2]); // Example: third entry is material
//         go.active = true;
// 
//         if (container->count >= container->capacity) {
//             container->capacity *= 2;
//             container->objects = realloc(container->objects, container->capacity * sizeof(GameObject));
//         }
//         container->objects[container->count++] = go;
// 
//         // Move lights to light container (simplified)
//         if (strstr(entries[0], "light")) {
//             GameObjectArray* light_container = &lm->light_containers[curlevel];
//             if (light_container->count >= light_container->capacity) {
//                 light_container->capacity = light_container->capacity ? light_container->capacity * 2 : 100;
//                 light_container->objects = realloc(light_container->objects, light_container->capacity * sizeof(GameObject));
//             }
//             light_container->objects[light_container->count++] = go;
//         }
//     }
//     
//     fclose(file);
    return 0;
}
