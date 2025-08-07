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
//     if (floatEquivalent(in->x, 0.0f) && floatEquivalent(in->y, 0.70711f) && floatEquivalent(in->z, 0.70711f) && floatEquivalent(in->w, 0.0f)) {
//         // North (Y+): Unity (0, 0.70711, 0.70711, 0) -> Engine (0, 0, 0, 1)
//         out->x = 0.0f; out->y = 0.0f; out->z = 0.0f; out->w = 1.0f;
//         return;
//     } else if (floatEquivalent(in->x, -0.70711f) && floatEquivalent(in->y, 0.0f) && floatEquivalent(in->z, 0.0f) && floatEquivalent(in->w, 0.70711f)) {
//         // South (Y-): Unity (-0.70711, 0, 0, 0.70711) -> Engine (0, 0, 1, 0)
//         out->x = 0.0f; out->y = 0.0f; out->z = 1.0f; out->w = 0.0f;
//         return;
//     } else if (floatEquivalent(in->x, 0.0f) && floatEquivalent(in->y, 0.70711f) && floatEquivalent(in->z, 0.0f) && floatEquivalent(in->w, 0.70711f)) {
//         // East (X+): Unity (0, 0.70711, 0, 0.70711) -> Engine (0, 0, 0.707107, 0.707107)
//         out->x = 0.0f; out->y = 0.0f; out->z = 0.707107f; out->w = 0.707107f;
//         return;
//     } else if (floatEquivalent(in->x, 0.0f) && floatEquivalent(in->y, -0.70711f) &&  floatEquivalent(in->z, 0.0f) && floatEquivalent(in->w, 0.70711f)) {
//         // West (X-): Unity (0, -0.70711, 0, 0.70711) -> Engine (0, 0, -0.707107, 0.707107)
//         out->x = 0.0f; out->y = 0.0f; out->z = -0.707107f; out->w = 0.707107f;
//         return;
//     } else if (floatEquivalent(in->x, 0.0f) && floatEquivalent(in->y, 0.0f) && floatEquivalent(in->z, -0.70711f) && floatEquivalent(in->w, 0.70711f)) {
//         // Down (Z-): Unity (0, 0, -0.70711, 0.70711) -> Engine (-0.707107, 0, 0, 0.707107)
//         out->x = -0.707107f; out->y = 0.0f; out->z = 0.0f; out->w = 0.707107f;
//         return;
//     }  else if (floatEquivalent(in->x, 1.0f) && floatEquivalent(in->y, 0.0f) && floatEquivalent(in->z, 0.0f) && floatEquivalent(in->w, 0.0f)) {
//         // Up (Z+): Unity (1, 0, 0, 0) -> Engine (0.707107, 0, 0, 0.707107)
//         out->x = 0.707107f; out->y = 0.0f; out->z = 0.0f; out->w = 0.707107f;
//         return;
//     } else {
//         // Fallback for unknown quaternions
//         // TODO: Implement a general transformation if needed
//         // For now, copy input to indicate unhandled case
//         out->x = in->x; out->y = in->y; out->z = in->z; out->w = in->w;
//         // Optional: Log a warning for debugging
//         // printf("Unhandled Unity quaternion: x: %f, y: %f, z: %f, w: %f\n", in->x, in->y, in->z, in->w);
//     }
   
    // Step 1: Invert Unity quaternion
//     Quaternion qInverse = {
//         .x = -in->x,
//         .y = -in->y,
//         .z = -in->z,
//         .w = in->w
//     };
// 
//     // Step 2: Convert to matrix
//     float m[3][3];
//     quat_to_matrix3x3(m, &qInverse);
// 
//     // Step 3: Coordinate transformation
//     float T[3][3] = {
//         { 1, 0, 0 },
//         { 0, 0, 1 },
//         { 0, 1, 0 }
//     };
// 
//     // Step 4: Apply T * m
//     float m_temp[3][3];
//     for (int i = 0; i < 3; i++) {
//         for (int j = 0; j < 3; j++) {
//             m_temp[i][j] = T[i][0] * m[0][j] + T[i][1] * m[1][j] + T[i][2] * m[2][j];
//         }
//     }
// 
//     // Step 5: Corrective rotation
//     float C[3][3] = {
//         { -1,  0,  0 },
//         {  0,  0,  1 },
//         {  0,  1,  0 }
//     };
//     float m_final[3][3];
//     for (int i = 0; i < 3; i++) {
//         for (int j = 0; j < 3; j++) {
//             m_final[i][j] = C[i][0] * m_temp[0][j] + C[i][1] * m_temp[1][j] + C[i][2] * m_temp[2][j];
//         }
//     }
// 
//     // Step 6: Convert back to quaternion
//     float trace = m_final[0][0] + m_final[1][1] + m_final[2][2];
//     if (trace > 0) {
//         float s = 0.5f / sqrtf(trace + 1.0f);
//         out->w = 0.25f / s;
//         out->x = (m_final[2][1] - m_final[1][2]) * s;
//         out->y = (m_final[0][2] - m_final[2][0]) * s;
//         out->z = (m_final[1][0] - m_final[0][1]) * s;
//     } else if (m_final[0][0] > m_final[1][1] && m_final[0][0] > m_final[2][2]) {
//         float s = 2.0f * sqrtf(1.0f + m_final[0][0] - m_final[1][1] - m_final[2][2]);
//         out->x = 0.25f * s;
//         out->y = (m_final[0][1] + m_final[1][0]) / s;
//         out->z = (m_final[0][2] + m_final[2][0]) / s;
//         out->w = (m_final[2][1] - m_final[1][2]) / s;
//     } else if (m_final[1][1] > m_final[2][2]) {
//         float s = 2.0f * sqrtf(1.0f + m_final[1][1] - m_final[0][0] - m_final[2][2]);
//         out->x = (m_final[0][1] + m_final[1][0]) / s;
//         out->y = 0.25f * s;
//         out->z = (m_final[1][2] + m_final[2][1]) / s;
//         out->w = (m_final[0][2] - m_final[2][0]) / s;
//     } else {
//         float s = 2.0f * sqrtf(1.0f + m_final[2][2] - m_final[0][0] - m_final[1][1]);
//         out->x = (m_final[0][2] + m_final[2][0]) / s;
//         out->y = (m_final[1][2] + m_final[2][1]) / s;
//         out->z = 0.25f * s;
//         out->w = (m_final[1][0] - m_final[0][1]) / s;
//     }
// 
//     // Normalize
//     float mag = sqrtf(out->x * out->x + out->y * out->y + out->z * out->z + out->w * out->w);
//     if (mag > 0) {
//         out->x /= mag;
//         out->y /= mag;
//         out->z /= mag;
//         out->w /= mag;
//     }
//     
//     if (floatEquivalent(in->x,0.0F) && floatEquivalent(in->y,0.70711F) && floatEquivalent(in->z,0.70711F) && floatEquivalent(in->w,0.0F)) {
//         printf("Rotation for North cell chunk (faces south) resulted in x %f y %f z %f w %f\n",out->x,out->y,out->z,out->w);
//     }
    
    // Flip handedness: invert x, y, z
    out->x = -in->x;
    out->y = -in->y;
    out->z = -in->z;
    out->w = in->w;

    // Normalize
    float mag = sqrtf(out->x * out->x + out->y * out->y + out->z * out->z + out->w * out->w);
    if (mag > 0) {
        out->x /= mag;
        out->y /= mag;
        out->z /= mag;
        out->w /= mag;
    }

    // Debug
    if (floatEquivalent(in->x, 0.0f) && floatEquivalent(in->y, 0.70711f) && 
        floatEquivalent(in->z, 0.70711f) && floatEquivalent(in->w, 0.0f)) {
        printf("North: x %f y %f z %f w %f\n", out->x, out->y, out->z, out->w);
    } else if (floatEquivalent(in->x, -0.70711f) && floatEquivalent(in->y, 0.0f) && 
               floatEquivalent(in->z, 0.0f) && floatEquivalent(in->w, 0.70711f)) {
        printf("South: x %f y %f z %f w %f\n", out->x, out->y, out->z, out->w);
    } else if (floatEquivalent(in->x, 1.0f) && floatEquivalent(in->y, 0.0f) && 
               floatEquivalent(in->z, 0.0f) && floatEquivalent(in->w, 0.0f)) {
        printf("Up: x %f y %f z %f w %f\n", out->x, out->y, out->z, out->w);
    }
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
