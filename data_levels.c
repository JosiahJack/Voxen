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
//     // Flip handedness: invert x, y, z
//     out->x = in->x;
//     out->y = in->z;
//     out->z = in->y;
//     out->w = in->w;
//
//     // Normalize
// //     float mag = sqrtf(out->x * out->x + out->y * out->y + out->z * out->z + out->w * out->w);
// //     if (mag > 0) {
// //         out->x /= mag;
// //         out->y /= mag;
// //         out->z /= mag;
// //         out->w /= mag;
// //     }

    Quaternion correqtion;
    correqtion.x = 0.0f;
    correqtion.y = 0.0f;
    correqtion.z = 0.70710678118654f;
    correqtion.w = 0.70710678118654f;
    quat_multiply(out, &correqtion, in);

    out->x *= -1.0f;
    out->y *= -1.0f;
    out->z *= -1.0f;
//     Quaternion correqtion2;
//     correqtion2.x = 0.0f;
//     correqtion2.y = 0.0f;
//     correqtion2.z = 0.70710678118654f;
//     correqtion2.w = 0.70710678118654f;
//     quat_multiply(out, &correqtion2, out);
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
//         Quaternion q;
//         unity_to_engine_quat(&q,&qUnity);
        instances[idx].rotx = qUnity.x;
        instances[idx].roty = qUnity.y;
        instances[idx].rotz = qUnity.z;
        instances[idx].rotw = qUnity.w;

        instances[idx].sclx = level_parser.entries[idx].localScale.x;
        instances[idx].scly = level_parser.entries[idx].localScale.z;
        instances[idx].sclz = level_parser.entries[idx].localScale.y;
//         DualLog("Loaded game object named %s with constIndex %d, at x: %f, y: %f, z: %f\n",
//                 level_parser.entries[idx].path,level_parser.entries[idx].constIndex,
//                 level_parser.entries[idx].localPosition.x,
//                 level_parser.entries[idx].localPosition.y,
//                 level_parser.entries[idx].localPosition.z);
    }

    // TEST CORNEL BOX TODO: DELETE

    // Test orienteering capability of existing quaternion code
    for (int i = 0; i < 6; ++i) {
        instances[i].posx = -2.56f;
        instances[i].posy = -2.56f;
        instances[i].posz = 0.0f;
        instances[i].modelIndex = 178; // generic lod card chunk
    }

    // Barrel
    instances[458].posx = -2.2f;
    instances[458].posy = -2.0f;
    instances[458].posz = -1.28f;
    instances[458].rotx = 0.0f;
    instances[458].roty = 0.0f;
    instances[458].rotz = 0.357f;
    instances[458].rotw = 0.934f;
    instances[458].modelIndex = 12;
    instances[458].texIndex = 30;
    instances[458].specIndex = 32;

    // Crate
    instances[472].posx = -2.2f;
    instances[472].posy = -3.4f;
    instances[472].posz = -1.28f;
    instances[472].rotx = 0.0f;
    instances[472].roty = 0.0f;
    instances[472].rotz = 0.199f;
    instances[472].rotw = 0.980f;
    instances[472].modelIndex = 60;
    instances[472].texIndex = 145;
    instances[472].glowIndex = 65535;
    instances[472].sclx = 1.0f;
    instances[472].scly = 1.0f;
    instances[472].sclz = 1.0f;

    // Cornell Box
    instances[0].modelIndex = 280;
    instances[0].texIndex = 513; // North med2_1
    instances[0].glowIndex = 511;
    instances[0].specIndex = 1242;

    // Test orienteering capability of existing quaternion code
    for (int i = 0; i < 6; ++i) {
        instances[i].posx = -2.56f;
        instances[i].posy = -2.56f;
        instances[i].posz = 0.0f;
        instances[i].modelIndex = 178; // generic lod card chunk
    }

    // Barrel
    instances[458].posx = -2.2f;
    instances[458].posy = -2.0f;
    instances[458].posz = -1.28f;
    instances[458].rotx = 0.0f;
    instances[458].roty = 0.0f;
    instances[458].rotz = 0.357f;
    instances[458].rotw = 0.934f;
    instances[458].modelIndex = 12;
    instances[458].texIndex = 30;
    instances[458].specIndex = 32;

    // Crate
    instances[472].posx = -2.2f;
    instances[472].posy = -3.4f;
    instances[472].posz = -1.28f;
    instances[472].rotx = 0.0f;
    instances[472].roty = 0.0f;
    instances[472].rotz = 0.199f;
    instances[472].rotw = 0.980f;
    instances[472].modelIndex = 60;
    instances[472].texIndex = 145;
    instances[472].glowIndex = 65535;
    instances[472].sclx = 1.0f;
    instances[472].scly = 1.0f;
    instances[472].sclz = 1.0f;

    // Cornell Box
    instances[0].modelIndex = 280;
    instances[0].texIndex = 513; // North med2_1
    instances[0].glowIndex = 511;
    instances[0].specIndex = 1242;

    instances[1].modelIndex = 282;
    instances[1].texIndex = 515; // South med2_2d
    instances[1].glowIndex = 508;
    instances[1].specIndex = 1242;

    instances[2].modelIndex = 244;
    instances[2].texIndex = 483; // East maint3_1

    instances[3].modelIndex = 243;
    instances[3].texIndex = 482; // West maint3_1d
    instances[3].glowIndex = 481;

    instances[5].modelIndex = 262;
    instances[5].texIndex = 499; // floor med1_9 bright teal light
    instances[5].specIndex = 1242;

    instances[4].modelIndex = 278;
    instances[4].texIndex = 507; // ceil med1_7 medical tile floor
    instances[4].specIndex = 1236;
    instances[1].modelIndex = 282;
    instances[1].texIndex = 515; // South med2_2d
    instances[1].glowIndex = 508;
    instances[1].specIndex = 1242;

    instances[2].modelIndex = 244;
    instances[2].texIndex = 483; // East maint3_1

    instances[3].modelIndex = 243;
    instances[3].texIndex = 482; // West maint3_1d
    instances[3].glowIndex = 481;

    instances[5].modelIndex = 262;
    instances[5].texIndex = 499; // floor med1_9 bright teal light
    instances[5].specIndex = 1242;

    instances[4].modelIndex = 278;
    instances[4].texIndex = 507; // ceil med1_7 medical tile floor
    instances[4].specIndex = 1236;


    // 0: Identity rotation (cell North side Y+ from cell center)
    Quaternion q;
    quat_identity(&q);
    instances[0].rotx = q.x;
    instances[0].roty = q.y;
    instances[0].rotz = q.z;
    instances[0].rotw = q.w;
//     printf("North cell side Y+ from cell center quat:: x: %f, y: %f, z: %f, w: %f\n",q.x,q.y,q.z,q.w);
    // Equates to Unity localRotation:
    // Inspector rotation -90, 0, 180, quaternion x: 0 y: 0.70711 z:0.70711 w: 0

    // 1: 180° around Z (cell South side Y- from cell center)
    quat_from_axis_angle(&q, 0.0f, 0.0f, 1.0f, M_PI);
    instances[1].rotx = q.x;
    instances[1].roty = q.y;
    instances[1].rotz = q.z;
    instances[1].rotw = q.w;
//     printf("South cell side Y- from cell center quat:: x: %f, y: %f, z: %f, w: %f\n",q.x,q.y,q.z,q.w);
    // Equates to Unity localRotation:
    // Inspector rotation -90, 0, 0, quaternion x: -0.70711, y: 0, z: 0, w: 0.70711

    // 2: +90° around Z (cell East side X+ from cell center)
    quat_from_axis_angle(&q, 0.0f, 0.0f, 1.0f, M_PI / 2.0f);
    instances[2].rotx = q.x;
    instances[2].roty = q.y;
    instances[2].rotz = q.z;
    instances[2].rotw = q.w;
//     printf("East cell side X+ from cell center quat:: x: %f, y: %f, z: %f, w: %f\n",q.x,q.y,q.z,q.w);

    // 3: −90° around Z (cell West side X− from cell center)
    quat_from_axis_angle(&q, 0.0f, 0.0f, 1.0f, -M_PI / 2.0f);
    instances[3].rotx = q.x;
    instances[3].roty = q.y;
    instances[3].rotz = q.z;
    instances[3].rotw = q.w;
//     printf("West cell side X- from cell center quat:: x: %f, y: %f, z: %f, w: %f\n",q.x,q.y,q.z,q.w);

    // 4: +90° around X (cell Floor Z− from cell center)
    quat_from_axis_angle(&q, 1.0f, 0.0f, 0.0f, -M_PI / 2.0f);
    instances[4].rotx = q.x;
    instances[4].roty = q.y;
    instances[4].rotz = q.z;
    instances[4].rotw = q.w;
//     printf("Down Z- from cell center quat:: x: %f, y: %f, z: %f, w: %f\n",q.x,q.y,q.z,q.w);

    // 5: −90° around X (cell Ceil Z+ from cell center)
    quat_from_axis_angle(&q, 1.0f, 0.0f, 0.0f, M_PI / 2.0f);
    instances[5].rotx = q.x;
    instances[5].roty = q.y;
    instances[5].rotz = q.z;
    instances[5].rotw = q.w;
//     printf("Up Z+ from cell center quat:: x: %f, y: %f, z: %f, w: %f\n",q.x,q.y,q.z,q.w);
    // Equates to Unity localRotation:
    // Inspector rotation -180, 0, 0, quaternion x: 1, y: 0, z: 0, w: 0

// North cell side Y+ from cell center quat:: x: 0.000000, y: 0.000000, z: 0.000000, w: 1.000000  = Unity X+, backtick toward Y+ (0deg roll?)
// South cell side Y- from cell center quat:: x: 0.000000, y: 0.000000, z: 1.000000, w: -0.000000 = Unity X-, backtick toward Y- (0deg roll?)
// East cell side X+ from cell center quat:: x: 0.000000, y: 0.000000, z: 0.707107, w: 0.707107   = Unity
// West cell side X- from cell center quat:: x: -0.000000, y: -0.000000, z: -0.707107, w: 0.707107= Unity
// Down Z- from cell center quat:: x: -0.707107, y: -0.000000, z: -0.000000, w: 0.707107          = Unity
// Up Z+ from cell center quat:: x: 0.707107, y: 0.000000, z: 0.000000, w: 0.707107               = Unity X+, backtick toward Z+ (-90deg roll??)

    parser_free(&level_parser);
    return 0;
}
