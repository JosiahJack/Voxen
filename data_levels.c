#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "data_levels.h"
#include "data_parser.h"
#include "data_entities.h"
#include "instance.h"
#include "debug.h"
#include "event.h"
#include "constants.h"
#include "quaternion.h"

DataParser level_parser;
const char *valid_leveldata_keys[] = {
    "constIndex","localPosition.x","localPosition.y","localPosition.z",
    "localRotation.x","localRotation.y","localRotation.z","localRotation.w",
    "localScale.x","localScale.y","localScale.z"};
#define NUM_LEVDAT_KEYS 11

int LoadLevels() {
    double start_time = get_time();
    DebugRAM("before LoadLevels");
    DualLog("Loading level data...\n");
    for (int i=0;i<numLevels;++i) {
        if (LoadLevelGeometry(i)) return 1;
    }
    
    DebugRAM("after LoadLevels");    
    double end_time = get_time();
    DualLog("Load Levels took %f seconds\n", end_time - start_time);
    return 0;
}

bool floatEquivalent(float val1, float val2) {
  return fabs(val1 - val2) < 0.001f;
}

void unity_to_engine_quat(Quaternion* out, Quaternion* in) {
    // Flip handedness: invert x, y, z
    out->x = -in->x;
    out->y = in->z;
    out->z = in->y;
    out->w = in->w;

    // Normalize
    float mag = sqrtf(out->x * out->x + out->y * out->y + out->z * out->z + out->w * out->w);
    if (mag > 0) {
        out->x /= mag;
        out->y /= mag;
        out->z /= mag;
        out->w /= mag;
    }

    Quaternion correqtion;
    correqtion.x = 0.5f;
    correqtion.y = -0.5f;
    correqtion.z = 0.5f;
    correqtion.w = 0.5f;
    quat_multiply(out, &correqtion, out);
}

int LoadLevelGeometry(uint8_t curlevel) {
    if (curlevel >= numLevels) { DualLogError("Cannot load level %d, out of bounds 0 to %d\n",curlevel,numLevels - 1); return 1; }
    if (curlevel != startLevel) return 0;

    char filename[64];
    snprintf(filename, sizeof(filename), "./Data/CitadelScene_geometry_level%d.txt", curlevel);
    parser_init(&level_parser, valid_leveldata_keys, NUM_LEVDAT_KEYS, PARSER_LEVEL);
    if (!parse_data_file(&level_parser, filename)) { DualLogError("Could not parse %s!\n",filename); parser_free(&level_parser); return 1; }
    
    int gameObjectCount = level_parser.count;
    DualLog("Loading %d objects for Level %d...\n",gameObjectCount,curlevel);
    for (int idx=0;idx<gameObjectCount;++idx) {
        int entIdx = level_parser.entries[idx].constIndex;
        instances[idx].modelIndex = entities[entIdx].modelIndex;
        instances[idx].texIndex = entities[entIdx].texIndex;
        instances[idx].glowIndex = entities[entIdx].glowIndex;
        instances[idx].specIndex = entities[entIdx].specIndex;
        instances[idx].normIndex = entities[entIdx].normIndex;
        instances[idx].lodIndex = entities[entIdx].lodIndex;
        instances[idx].posx = level_parser.entries[idx].localPosition.x;
        instances[idx].posy = level_parser.entries[idx].localPosition.z;
        instances[idx].posz = level_parser.entries[idx].localPosition.y;
        
        Quaternion qUnity = { level_parser.entries[idx].localRotation.x,
                              level_parser.entries[idx].localRotation.y,
                              level_parser.entries[idx].localRotation.z,
                              level_parser.entries[idx].localRotation.w };
        Quaternion q;
        unity_to_engine_quat(&q,&qUnity);
        instances[idx].rotx = q.x;
        instances[idx].roty = q.y;
        instances[idx].rotz = q.z;
        instances[idx].rotw = q.w;

        instances[idx].sclx = level_parser.entries[idx].localScale.x;
        instances[idx].scly = level_parser.entries[idx].localScale.z;
        instances[idx].sclz = level_parser.entries[idx].localScale.y;
//         DualLog("Loaded game object named %s with constIndex %d, at x: %f, y: %f, z: %f\n",
//                 level_parser.entries[idx].path,level_parser.entries[idx].constIndex,
//                 level_parser.entries[idx].localPosition.x,
//                 level_parser.entries[idx].localPosition.y,
//                 level_parser.entries[idx].localPosition.z);
    }

    parser_free(&level_parser);
    return 0;
}
