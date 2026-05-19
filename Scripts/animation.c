// animation.c - Animation System for both models and textures (in world and UI)
#include "mod.h"
// static const float textureSequenceFrameDelay = 0.35f;
// static const float textureSequenceFrameDelayVmail = 0.09f; // TODO
const AnimationClip modelAnimationClips[MAX_ANIMATED_MODELS][MAX_ANIMATION_CLIPS_PER_MODEL] = { // speed, frameStart, frameEnd, frameStartModelIndex, framerate
    [0]={[ANIM_IDLE_CLOSED]={1.0f,2,2,699,24},[ANIM_OPENING]={1.0f,2,11,699,24},[ANIM_IDLE_OPEN]={1.0f,11,11,708,24},[ANIM_CLOSING]={1.0f,12,21,709,24}}, // doorB (door2)
    [1]={[ANIM_IDLE_CLOSED]={1.0f,2,2,719,24},[ANIM_OPENING]={1.0f,2,12,719,24},[ANIM_IDLE_OPEN]={1.0f,12,12,729,24},[ANIM_CLOSING]={1.0f,14,24,731,24}}, // doorA (door1)
    [2]={[ANIM_IDLE]={1.0f,0,37,742,30},[ANIM_WALK]={1.0f,50,99,780,30},[ANIM_RUN]={1.1f,50,99,792,30},[ANIM_ATTACK1]={0.75f,111,136,830,30},[ANIM_PAIN]={0.5f,138,150,856,30},[ANIM_DYING]={0.75f,153,176,869,30}}, // npc_humanoid_mutant
    [3]={[ANIM_IDLE]={1.0f,1,207,893,24},[ANIM_ATTACK1]={1.0f,219,239,1100,24},[ANIM_WALK]={1.0f,252,308,1121,24},[ANIM_RUN]={1.0f,252,308,1121,24},[ANIM_PAIN]={1.0f,321,330,1177,24},[ANIM_PAIN2]={1.0f,331,344,1187,24},[ANIM_DYING]={1.0f,345,369,1201,24}}, // npc_cyborg_drone 
    [4]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1234,24},[ANIM_OPENING]={1.5f,2,44,1234,24},[ANIM_IDLE_OPEN]={1.0f,44,44,1276,24},[ANIM_CLOSING]={1.75f,46,96,1277,24}}, // doorD (door4, bulkhead 1)
    [5]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1328,24},[ANIM_OPENING]={1.0f,2,25,1328,24},[ANIM_IDLE_OPEN]={1.0f,25,25,1351,24},[ANIM_CLOSING]={1.0f,27,44,1352,24}}, // doorC (door3)
    [6]={[ANIM_IDLE_CLOSED]={1.0f,1,1,1370,24},[ANIM_OPENING]={1.2f,1,30,1370,24},[ANIM_IDLE_OPEN]={1.0f,30,30,1399,24},[ANIM_CLOSING]={1.2f,32,66,1400,24}}, // doorJ (xdoor1)
    [7]={[ANIM_IDLE_CLOSED]={1.0f,3,3,1435,24},[ANIM_OPENING]={1.2f,3,24,1435,24},[ANIM_IDLE_OPEN]={1.0f,26,26,1457,24},[ANIM_CLOSING]={1.2f,27,49,1458,24}}, // doorK (xdoor2)
    [8]={[ANIM_IDLE_CLOSED]={1.0f,3,3,1481,24},[ANIM_OPENING]={1.2f,3,27,1481,24},[ANIM_IDLE_OPEN]={1.0f,27,27,1505,24},[ANIM_CLOSING]={1.2f,30,51,1506,24}}, // doorL (door10)
    [9]={[ANIM_IDLE_CLOSED]={1.0f,3,3,1528,24},[ANIM_OPENING]={1.0f,3,15,1528,24},[ANIM_IDLE_OPEN]={1.0f,28,28,1541,24},[ANIM_CLOSING]={1.0f,28,39,1541,24}}, // doorE (door5)
    [10]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1553,24},[ANIM_OPENING]={1.0f,2,23,1553,24},[ANIM_IDLE_OPEN]={1.0f,23,23,1574,24},[ANIM_CLOSING]={1.0f,27,45,1541,24}}, // doorF (door6)
    [11]={[ANIM_IDLE_CLOSED]={1.0f,3,3,1594,24},[ANIM_OPENING]={1.0f,3,22,1594,24},[ANIM_IDLE_OPEN]={1.0f,22,22,1613,24},[ANIM_CLOSING]={1.0f,25,42,1614,24}}, // doorG (door7)
    [12]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1632,24},[ANIM_OPENING]={1.0f,2,25,1632,24},[ANIM_IDLE_OPEN]={1.0f,25,25,1655,24},[ANIM_CLOSING]={1.0f,27,49,1656,24}}, // doorH (door8)
    [13]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1679,24},[ANIM_OPENING]={1.0f,2,24,1679,24},[ANIM_IDLE_OPEN]={1.0f,24,24,1691,24},[ANIM_CLOSING]={1.0f,26,52,1692,24}}, // doorI (door9)
    [14]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1719,24},[ANIM_OPENING]={1.0f,2,20,1719,24},[ANIM_IDLE_OPEN]={1.0f,20,20,1737,24},[ANIM_CLOSING]={1.0f,22,41,1738,24}}, // door_elevator1
    [15]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1758,24},[ANIM_OPENING]={1.5f,2,21,1758,24},[ANIM_IDLE_OPEN]={1.0f,21,21,1777,24},[ANIM_CLOSING]={1.5f,23,41,1778,24}}, // door_elevator2
    [16]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1797,24},[ANIM_OPENING]={1.0f,2,22,1797,24},[ANIM_IDLE_OPEN]={1.0f,22,22,1817,24},[ANIM_CLOSING]={1.0f,24,43,1818,24}}, // door_elevator3
    [17]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1838,24},[ANIM_OPENING]={2.0f,2,32,1838,24},[ANIM_IDLE_OPEN]={1.0f,32,32,1868,24},[ANIM_CLOSING]={2.0f,34,62,1869,24}}, // door_elevator4
    [18]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1898,24},[ANIM_OPENING]={1.0f,2,21,1898,24},[ANIM_IDLE_OPEN]={1.0f,21,21,1917,24},[ANIM_CLOSING]={1.0f,23,41,1918,24}}, // door_secret2 (door_wall1)
    [19]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1937,24},[ANIM_OPENING]={1.0f,2,21,1937,24},[ANIM_IDLE_OPEN]={1.0f,21,21,1956,24},[ANIM_CLOSING]={1.0f,23,41,1957,24}}, // door_secret1 (door_wall2)
    [20]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1976,24},[ANIM_OPENING]={1.0f,2,17,1976,24},[ANIM_IDLE_OPEN]={1.0f,17,17,1991,24},[ANIM_CLOSING]={1.0f,19,33,1992,24}}, // door_secret3 (door_wall3)
    [21]={[ANIM_LOOP_ALL]={1.0f,1,47,2007,24}}, // chunk_eng2_6 (eng_wallpump)
    [22]={[ANIM_LOOP_ALL]={1.0f,1,50,2054,24}}, // flight_fanwall
    [23]={[ANIM_IDLE]={1.0f,3,3,2104,24},[ANIM_WALK]={1.0f,3,36,2104,24},[ANIM_ATTACK1]={1.0f,38,56,2138,24},[ANIM_ATTACK2]={1.0f,58,81,2156,24},[ANIM_ATTACK3]={1.0f,58,81,2156,24},[ANIM_RUN]={1.0f,3,36,2104,24},[ANIM_PAIN]={1.0f,84,96,2180,24},[ANIM_DYING]={1.0f,99,106,2192,24}}, // npc_bot_cortex_reaver
    [24]={[ANIM_IDLE]={1.0f,1,60,2200,24},[ANIM_ATTACK2]={1.0f,62,83,2260,24},[ANIM_ATTACK3]={1.0f,86,122,2282,24},[ANIM_RUN]={1.0f,143,182,2319,24},[ANIM_WALK]={1.0f,143,182,2319,24},[ANIM_PAIN]={1.0f,204,214,2359,24},[ANIM_PAIN2]={1.0f,216,227,2370,24},[ANIM_DYING]={1.0f,229,268,2382,24}}, // npc_cyborgassassin
    [25]={[ANIM_IDLE]={1.0f,1,155,2422,30},[ANIM_RUN]={1.0f,190,243,2577,30},[ANIM_WALK]={1.0f,190,243,2577,30},[ANIM_ATTACK2]={1.0f,265,289,2631,30},[ANIM_ATTACK1]={1.0f,291,332,2656,30},[ANIM_DYING]={1.0f,334,417,2698,30}}, // npc_cyborg_diego
    [26]={[ANIM_IDLE]={1.0f,1,68,2782,30},[ANIM_WALK]={1.0f,90,173,2850,30},[ANIM_RUN]={1.0f,90,173,2850,30},[ANIM_ATTACK2]={1.0f,194,214,2934,30},[ANIM_PAIN]={1.0f,216,233,2955,24},[ANIM_PAIN2]={1.0f,235,244,2973,24},[ANIM_DYING]={1.0f,319,386,2983,30},[ANIM_ATTACK1]={1.0f,401,422,3051,30},[ANIM_ATTACK3]={1.0f,424,450,3073,24}}, // npc_cyborg_elite
    [27]={[ANIM_IDLE]={1.0f,1,219,3100,24},[ANIM_WALK]={1.0f,240,286,3319,24},[ANIM_RUN]={1.0f,240,286,3319,24},[ANIM_PAIN]={1.0f,306,327,3366,24},[ANIM_ATTACK1]={1.0f,329,351,3388,24},[ANIM_ATTACK2]={1.0f,353,377,3411,24},[ANIM_DYING]={1.0f,380,402,3436,24},[ANIM_ATTACK3]={1.0f,416,438,3459,24}}, // npc_cyborg_enforcer
    [28]={[ANIM_IDLE]={1.0f,1,66,3482,24},[ANIM_ATTACK3]={1.0f,68,80,3548,24},[ANIM_ATTACK2]={1.0f,82,101,3561,24},[ANIM_PAIN]={1.0f,103,114,3581,24},[ANIM_WALK]={1.0f,122,157,3593,24},[ANIM_RUN]={1.0f,122,157,3593,24},[ANIM_DYING]={1.0f,169,217,3629,24}}, // npc_cyborgwarrior
    [29]={[ANIM_IDLE]={1.0f,3,3,3678,24},[ANIM_WALK]={1.0f,15,68,3679,24},[ANIM_RUN]={1.0f,15,68,3679,24},[ANIM_PAIN]={1.0f,82,92,3732,24},[ANIM_ATTACK2]={1.0f,94,117,3743,24},[ANIM_DYING]={1.0f,119,127,3767,24}}, // npc_execbot
    [30]={[ANIM_IDLE]={1.0f,1,39,3776,24},[ANIM_WALK]={2.0f,1,39,3776,24},[ANIM_RUN]={2.0f,1,39,3776,24},[ANIM_PAIN]={1.0f,41,73,3815,24},[ANIM_PAIN2]={0.5384f,75,95,3848,24},[ANIM_ATTACK2]={1.0f,97,121,3869,24},[ANIM_ATTACK3]={1.0f,106,121,3878,24}}, // npc_flierbot
    [31]={[ANIM_IDLE]={1.0f,1,73,3894,24},[ANIM_WALK]={1.0f,88,130,3967,24},[ANIM_RUN]={1.0f,88,130,3967,24},[ANIM_PAIN]={1.0f,144,159,4010,24},[ANIM_ATTACK1]={1.0f,162,183,4026,24},[ANIM_ATTACK2]={1.0f,186,209,4048,24},[ANIM_DYING]={1.0f,212,237,4072,24}}, // npc_gortiger
    [32]={[ANIM_IDLE]={1.0f,1,47,4098,24},[ANIM_WALK]={1.0f,49,87,4145,24},[ANIM_RUN]={1.0f,49,87,4145,24},[ANIM_PAIN]={1.0f,88,107,4184,24},[ANIM_PAIN2]={1.0f,109,125,4204,24},[ANIM_PAIN3]={1.0f,127,144,4221,24},[ANIM_ATTACK2]={1.0f,145,157,4239,24},[ANIM_DYING]={1.0f,160,239,4252,24}}, // npc_hopper
    [33]={[ANIM_IDLE]={1.0f,1,30,4332,24},[ANIM_WALK]={1.0f,1,30,4332,24},[ANIM_RUN]={1.0f,1,30,4332,24},[ANIM_PAIN]={1.0f,35,51,4362,24},[ANIM_ATTACK2]={1.0f,52,72,4379,24},[ANIM_DYING]={1.0f,79,103,4400,24}}, // npc_invisomut
    [34]={[ANIM_IDLE]={1.0f,2,2,4425,24},[ANIM_ATTACK1]={2.0f,2,71,4425,24},[ANIM_WALK]={2.0f,80,107,4495,24},[ANIM_RUN]={2.0f,80,107,4495,24},[ANIM_DYING]={1.0f,117,150,4523,24}}, // npc_maintenancebot
    [35]={[ANIM_IDLE]={2.5f,1,59,4557,24},[ANIM_WALK]={2.5f,1,59,4557,24},[ANIM_RUN]={2.5f,1,59,4557,24},[ANIM_ATTACK1]={1.0f,61,79,4616,24},[ANIM_PAIN]={1.0f,81,93,4635,24},[ANIM_DYING]={1.0f,94,119,4648,24}}, // npc_mutant_avian
    [36]={[ANIM_IDLE]={1.0f,1,78,4674,24},[ANIM_WALK]={1.0f,90,129,4752,24},[ANIM_RUN]={1.0f,90,129,4752,24},[ANIM_ATTACK2]={1.0f,142,185,4792,24},[ANIM_DYING]={1.0f,188,225,4836,24},[ANIM_PAIN]={1.0f,227,235,4874,24}}, // npc_plantmutant
    [37]={[ANIM_IDLE]={1.0f,1,42,4883,24},[ANIM_WALK]={2.0f,58,85,4925,24},[ANIM_RUN]={2.0f,58,85,4925,24},[ANIM_ATTACK1]={1.0f,102,123,4953,24},[ANIM_ATTACK2]={1.0f,126,148,4975,24}}, // npc_repairbot
    [38]={[ANIM_IDLE]={1.0f,1,54,4998,24},[ANIM_WALK]={1.0f,1,54,4998,24},[ANIM_RUN]={1.0f,58,95,5052,24}}, // npc_sec1bot
    [39]={[ANIM_IDLE]={0.333f,1,17,5090,24},[ANIM_WALK]={0.333f,19,38,5107,24},[ANIM_RUN]={0.333f,19,38,5107,24},[ANIM_ATTACK2]={0.25f,39,48,5127,24},[ANIM_ATTACK3]={1.0f,49,56,5137,24},[ANIM_PAIN]={1.0f,58,63,5145,24},[ANIM_DYING]={0.2f,65,66,5151,24}}, // npc_sec2bot
    [40]={[ANIM_IDLE]={0.18f,1,9,5153,24},[ANIM_WALK]={0.333f,1,9,5153,24},[ANIM_RUN]={0.333f,1,9,5153,24},[ANIM_ATTACK1]={0.5f,18,28,5162,24},[ANIM_PAIN]={0.333f,54,63,5173,24},[ANIM_DYING]={0.333f,77,85,5183,24}}, // npc_servbot
    [41]={[ANIM_IDLE]={1.0f,1,66,5192,24},[ANIM_WALK]={2.0f,79,132,5258,24},[ANIM_RUN]={2.5f,79,132,5258,24},[ANIM_PAIN]={1.0f,145,157,5312,24},[ANIM_ATTACK2]={1.0f,159,181,5325,24},[ANIM_DYING]={1.0f,183,221,5348,24}}, // npc_virusmutant
    [42]={[ANIM_IDLE]={1.0f,1,121,5387,24},[ANIM_DYING]={1.0f,121,157,5507,24}}, // npc_zerogmut
    [43]={[ANIM_IDLE_CLOSED]={1.0f,1,1,5544,24},[ANIM_OPENING]={1.2f,2,21,5545,24},[ANIM_IDLE_OPEN]={1.0f,21,21,5564,24}}, // puzzlepanel1
    [44]={[ANIM_IDLE_CLOSED]={1.0f,0,0,5565,24},[ANIM_OPENING]={1.2f,1,17,5566,24},[ANIM_IDLE_OPEN]={1.0f,17,17,5582,24},[ANIM_INSTALL]={1.0f,19,30,5584,24},[ANIM_INSTALLED]={1.0f,18,18,5583,24}}, // puzzlepanel2
    [45]={[ANIM_IDLE_CLOSED]={1.0f,0,0,5596,24},[ANIM_OPENING]={1.2f,1,17,5597,24},[ANIM_IDLE_OPEN]={1.0f,17,17,5613,24},[ANIM_INSTALLED]={1.0f,18,18,5614,24}}, // puzzlepanel3
    [46]={[ANIM_LOOP_ALL]={1.0f,1,100,5615,24}}, // sparkingwire
    [47]={[ANIM_INACTIVE]={1.0f,2,2,5715,24},[ANIM_ACTIVATE]={1.2f,2,4,5715,24},[ANIM_ACTIVATED]={1.0f,4,4,5717,24},[ANIM_DEACTIVATE]={1.0f,5,6,5718,24}}, // switch4
    [48]={[ANIM_INACTIVE]={1.0f,2,2,5720,24},[ANIM_ACTIVATE]={1.2f,2,6,5720,24},[ANIM_ACTIVATED]={1.0f,6,6,5724,24},[ANIM_DEACTIVATE]={1.0f,8,10,5725,24}}, // switch5
    [49]={[ANIM_IDLE]={1.0f,1,1,5728,24},[ANIM_ATTACK_MISS]={1.0f,1,13,5728,24},[ANIM_ATTACK_HIT]={1.0f,18,24,5741,24}}, // v_pipe
    [50]={[ANIM_IDLE]={1.0f,1,1,5748,24},[ANIM_ATTACK_MISS]={0.5f,4,22,5749,24},[ANIM_ATTACK_HIT]={1.0f,4,22,5749,24}}, // v_rapier
    [51]={[ANIM_IDLE]={1.0f,1,65,5768,24},[ANIM_WALK]={1.0f,75,98,5833,24},[ANIM_RUN]={1.0f,75,98,5833,24},[ANIM_ATTACK2]={1.0f,109,126,5857,24},[ANIM_ATTACK1]={1.0f,128,142,5875,24},[ANIM_PAIN]={1.0f,144,159,5890,24},[ANIM_PAIN2]={1.0f,161,174,5906,24},[ANIM_DYING]={1.0f,176,243,5920,24}}, // npc_mutant_cyborg
};

MOD_TO_ENGINE void UpdateAnims(void) {
    bool portalsNeedUpdated = false;
    for (u16 i = START_INDEX_LEVEL_INSTANCES; i < INSTANCE_COUNT; ++i) {
        Entity* e = &Eng_Global->instances[i];
        if (e->modelIndex >= MODEL_IDX_MAX) continue;
        if (!(e->entflags & ENTFLAG_ACTIVE)) continue;

        u16 anim = e->animationNum;
        if (anim >= MAX_ANIMATED_MODELS || e->numclips == 0 || e->clip >= e->numclips) continue;

        AnimationClip* clip = (AnimationClip*)&modelAnimationClips[anim][e->clip];
        if (clip->framerate <= 0 || clip->speed <= 0) continue;

        const double timePerFrame = (1.0 / (double)clip->speed) * (1.0 / (double)clip->framerate);
        double timePassed = Eng_Global->pauseRelativeTime - e->currentFrameFinished;
        if (timePassed < timePerFrame) continue;

        u32 framesToAdvance = (u32)(timePassed / timePerFrame);           // integer frames
        double remainder     = timePassed - (framesToAdvance * timePerFrame); // keep fractional part
        u32 frameCount = clip->frameEnd - clip->frameStart + 1;
        if (frameCount <= 1) {
            e->frame = clip->frameStart;
        } else {
            u32 newFrame = (e->frame - clip->frameStart + framesToAdvance) % frameCount;
            e->frame = clip->frameStart + newFrame;
        }

        e->currentFrameFinished = Eng_Global->pauseRelativeTime - remainder;
        e->modelIndex = clip->frameStartModelIndex + (e->frame - clip->frameStart);
        Eng_Global->dirtyInstances[i] = true;
        if (ConstIndexIsPortalBlockingDoor(e->index)) {
            if (ToggleDoorPortal(e->portalIndex, i,
                modelAnimationClips[anim][ANIM_IDLE_CLOSED].frameStartModelIndex)) {
                portalsNeedUpdated = true;
            }
        }
    }

    if (portalsNeedUpdated) PortalCulling();
}

#define NUM_TEXTURE_CLIPS 48
typedef struct {
    const u16 *frames;     // pointer into sequenceTextures[]
    u8         length;     // how many frames (before wrapping / stopping)
    bool            hasGlow;    // does this clip also animate the glow map?
    const u16 *glowFrames; // can be NULL if !hasGlow
    u8         glowLength;
    const char*     name;
} TextureAnimClip;

u16 sequenceTextures[302]={
    1159,1160,881,1162,1163,1164, // scr_exp 01 - 06
    1310,1311,1312,1313, // bridg1_1 001 - 004
    1115,1116, // broken_clock01_glow 01 - 02
    1117,1118, // broken_clock 01 - 02
    1124,1125,1126,1127,1128,1129,1130, // g_energmine 00 - 06
    1131,1132,1133,1134,1135,1136,1137,1138, // g_energmine_glow 00 - 07 (yes different count, supported!)
    1314,1315,1316,1317, // scr_cita2_ 0 - 3
    1318,1319,1320,1321, // scr_cita3_ 0 - 3
    1322,1323,1324,1325,1326,1327,1328,1329, // scr_cita_ 0 - 7
    1330, // engscreen1_04 // index 45
    0,1331,1332,1333,1334,1335,1336,1337, // scr_static2 0 - 6, then scr_static2_a
    1338,1339,1340,1341,1342,1343, // scr_static 0 - 5
    1344,1345,1346,1347, // screen1 0 - 3
    1348,1349,1350,1351,1352, // screen2 0 - 4
    1353,1354,1355,1356, // screen3 0 - 3
    1357,1358,1359,1360,1361,1362, // screen4 0 - 5
    1363,1364,1365,1366, // screen5 0 - 3
    1367,1368,1369,1370, // triop1 0 - 3
    1371,1372,1373,1374,1375,1376,1377,1378,1379,1380, // triop2 0 - 9
    1381,1382,1383,1384,1385,1386,1387,1388, // triop3 0 - 7
    1389, // triop4_8 // index 105
    1381, // triop3_0 // index 106
    1390,1391,1392,1393,1394,1395,1396,1397, // dna 0 - 7
    1398,1399,1400,1401, // edcolor 0 - 3
    1402,1403,1404,1405, // edgray 0 - 3
    1406,1407,1408,1409,1410,1411,1412,1413,1414,1415,1416, // ammo_magcart 00 - 10
    1417,1418,1419,1420,1421,1422,1423,1424,1425,1426,1427, // ammo_magcart_glow 00 - 10
    1428,1429,1430,1431,1432,1433,1434,1435,1436,1437, // medicalbed 00 - 9
    1438,1439,1440,1441,1442, // rad1_1 00 - 04
    1443,1444,1445,1446,1447,1448,1449,1450,1451,1452, // screencode 0 - 9
    1453,1454,1455,1456,1457,1458,1459,1460,1461,1462,1463,1464,1465,1466,1467,1468,1469,1470,1471,1472,1473,1474,1475,1476,1477,1478,1479,1480,1481,1482,1483,1484,1485,1486,1487,1488,1489, // shodanstatic 00 - 36
    1490,1491,1492,1493, // telepad 00 - 03
    1494, // telepad_00_glow
    0, // black // index 212
    0, // black
    0, // black
    0, // black
    0, // black
    0, // black // index 217
    1495,1496,1497,1498,1499,1500,1501,1502,1503,1504,1505,1506, // medscreen13 00 - 11
    1507,1508,1509,1510,1511,1512,1513,1514, // medscreen24 00 - 07
    1515,1516,1517,1518,1519,1520,1521,1522, // medscreen16 00 - 07
    1523,1524,1525,1526,1527,1528,1529,1530,1531,1532,1533,1534,1535,1536,1537,1538,1539,1540,1541,1542,1543,1544,1545,1546,1547,1548,1549,1550,1551,1552,1553,1554,1555,1556,1557,1558,1559,1560,1561,1562,1563,1564,1565,1566,1567,1568,1569,1570,1571,1572,1573,1574, // zerog 00 - 52
    1576,1577,1578 // door_x1 01 - 03 // ends at index 301
};

static const TextureAnimClip textureAnimClips[NUM_TEXTURE_CLIPS] = {
    /*0*/{(u16[]){6,7,8,9,9,8,7,6},8,false,NULL,0,"Bridge11"},
    /*1*/{(u16[]){10,11},2,true,(u16[]){12,13},2,"BrokenClock"},
    /*2*/{(u16[]){14,15,16,17,18,19,20},7,true,(u16[]){21,22,23,24,25,26,27,28},8,"EnergMine"},
    /*3*/{(u16[]){29,30,31,32,45,45,45,29,30,31,32,29,30,31,32,29,30,31,32,29,45,45,45,29,30,31,32,29,30,31,32,29,30,31,32,29,30,31,32,29,45,45,45},43,false,NULL,0,"EngScreen1"},
    /*4*/{(u16[]){52,51,50,49,49,50,51,52},8,false,NULL,0,"EngScreen2"},
    /*5*/{(u16[]){29,30,31,32,45,29,30,31,45,29,30,31,45},13,false,NULL,0,"ExecScreen1"},
    /*6*/{(u16[]){83,84,85,86,83,83,86,85,84,83,103,83,84,85,86,83,83,86,85,84,83},21,false,NULL,0,"ExecScreen2"},
    /*7*/{(u16[]){115,115,117},3,false,NULL,0,"ExecScreen3"},
    /*8*/{(u16[]){115,115,115,115,116,117,118},7,false,NULL,0,"ExecScreen4"},
    /*9*/{(u16[]){123,124,125,126,127,128,129,130,131,132,133},11,true,(u16[]){134,135,136,137,138,139,140,141,142,143,144},11,"MagCartridge"},
    /*10*/{(u16[]){29,30,31,32,37},5,false,NULL,0,"MaintScreen1"},
    /*11*/{(u16[]){33,34,35,36,32,29},6,false,NULL,0,"MaintScreen2"},
    /*12*/{(u16[]){145,146,147,148,149,150,151,152,153,154},10,false,NULL,0,"MedicalBed"},
    /*13*/{(u16[]){54,59,118,116,118,59},6,false,NULL,0,"MedScreen1"},
    /*14*/{(u16[]){79,80,81,82},4,false,NULL,0,"MedScreen2"},
    /*15*/{(u16[]){99,98,97,92},4,false,NULL,0,"MedScreen3"},
    /*16*/{(u16[]){29,30,31,36},4,false,NULL,0,"MedScreen4"},
    /*17*/{(u16[]){56,55,54,59,59,54,55,56},8,false,NULL,0,"MedScreen5"},
    /*18*/{(u16[]){61,61,62,62,61,61,212,213,214,215,216,217},12,false,NULL,0,"MedScreen6"},
    /*19*/{(u16[]){119,120,121,122},4,false,NULL,0,"MedScreen7"},
    /*20*/{(u16[]){59,54,55,56},4,false,NULL,0,"MedScreen8"},
    /*21*/{(u16[]){37,38,39,40,41,42,43,44},8,false,NULL,0,"MedScreen9"},
    /*22*/{(u16[]){83,84,85,86,83,83,86,85,84,83},10,false,NULL,0,"MedScreen10"},
    /*23*/{(u16[]){67,66,66,67,79,80,80,79},8,false,NULL,0,"MedScreen11"},
    /*24*/{(u16[]){218,219,220,221,222,223,224,225,226,227,228,229},12,false,NULL,0,"MedScreen13"},
    /*25*/{(u16[]){79,80,81,82,82,81,80,79},8,false,NULL,0,"MedScreen16"},
    /*26*/{(u16[]){73,74,75,76,77,78},6,false,NULL,0,"MedScreen18"},
    /*27*/{(u16[]){73,74,76,75,77,76,78,73},8,false,NULL,0,"MedScreen22"},
    /*28*/{(u16[]){29,30,31,32},4,false,NULL,0,"MedScreen23"},
    /*29*/{(u16[]){230,231,232,233,234,235,236,237},8,false,NULL,0,"MedScreen24"},
    /*30*/{(u16[]){92,93,94,95},4,false,NULL,0,"MedScreen25"},
    /*31*/{(u16[]){238,239,240,241,242,243,244,245},8,false,NULL,0,"MedScreen27"},
    /*32*/{(u16[]){64,65,66,67,68},5,false,NULL,0,"MedScreen29"},
    /*33*/{(u16[]){155,156,157,158,159},5,false,NULL,0,"Rad1_1"},
    /*34*/{(u16[]){61,61,62,62},4,false,NULL,0,"ReacScreen4"},
    /*35*/{(u16[]){62,61,60},3,false,NULL,0,"SciScreen1"},
    /*36*/{(u16[]){107,108,109,111,112,113,114},7,false,NULL,0,"SciScreen2"},
    /*37*/{(u16[]){33,34,35,36},4,false,NULL,0,"SciScreen3"},
    /*38*/{(u16[]){188,189,113,112},4,false,NULL,0,"SciScreen4"},
    /*39*/{(u16[]){79,80,80,79,73,74,76,77,75},9,false,NULL,0,"SciScreen5"},
    /*40*/{(u16[]){0,1,2,3,4,5},6,false,NULL,0,"ScreenDestroyed"},
    /*41*/{(u16[]){160,161,162,163,164,165,166,167,168,169},10,false,NULL,0,"ScreenCodeRandom"},
    /*42*/{(u16[]){29,30,31},3,false,NULL,0,"SecScreen4"},
    /*43*/{(u16[]){170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,203,204,205,206,205,203,204,206,205,204,203,206,205,203,204,205,206,203,206,205,203,204,203},60,false,NULL,0,"ShodanStatic"},
    /*44*/{(u16[]){203,204,205,206,203,206,205,203,204,205,206,203,206,205,203,204,203},17,false,NULL,0,"Static"},
    /*45*/{(u16[]){207,208,209,210},4,true,(u16[]){207,208,209,210},4,"Telepad"},
    /*46*/{(u16[]){299,300,301},3,false,NULL,0,"XDoor1"},
    /*47*/{(u16[]){246,247,248,249,250,251,252,253,254,255,256,257,258,259,260,261,262,263,264,265,266,267,268,269,270,271,272,273,274,275,276,277,278,279,280,281,282,283,284,285,286,287,288,289,290,291,292,293,294,295,296,297,298},53,false,NULL,0,"ZeroGMutant"},
};

void TextureSequenceInit(u16 self, char* trimmed_value) {
    Entity* e = &Eng_Global->instances[self];
    if (e->index == 526) return; // Skip prop_console02 for now, will need to split its screen off.
    if (trimmed_value[0] == '\0') { e->textureAnimating = false; e->modelIndex = EDefs[e->index].modelIndex; return; }
    
    e->textureAnimating = true; e->textureGlowAnimating = false; e->texAnimLight = U16_MAX; e->texAnimLight2 = U16_MAX;
    e->texFrame = e->texGlowFrame = 0;
    if (StringsEqual(trimmed_value,"ScreenDestroyed")) { e->texAnimClip = NUM_TEXTURE_CLIPS - 1; return; }
    if (StringsEqual(trimmed_value,"MedCamView1")) { e->textureAnimating = false; e->camView = 0; return; } // Sensaround occupies slots 0,1,2 for center, left, right respectively.
    if (StringsEqual(trimmed_value,"MedCamView2")) { e->textureAnimating = false; e->camView = 1; return; }
    
    for (int i = 0; i < NUM_TEXTURE_CLIPS; ++i) {
        if (StringsEqual(trimmed_value,textureAnimClips[i].name)) { e->texAnimClip = i; e->textureGlowAnimating = textureAnimClips[i].hasGlow; return; }
    }
    
    e->textureAnimating = false; // Couldn't find match, just don't animate.
}

void TextureSequenceUpdate(u16 self) {
    Entity* e = &Eng_Global->instances[self];
    if (!e->textureAnimating) return;
    if (e->tickFinished >= Eng_Global->pauseRelativeTime) return;

    e->tickFinished = Eng_Global->pauseRelativeTime + e->tickTime;
    const TextureAnimClip* clip = &textureAnimClips[e->texAnimClip];
    if (e->texAnimRandom && (!e->textureAnimationStopsAtDead || e->health > 0.0f)) {
        e->texFrame = random_range_u32(0, clip->length - 1);
        if (clip->hasGlow) e->texGlowFrame = random_range_u32(0, clip->glowLength - 1);
    } else {
        if (e->texAnimInReverse) {
            if (e->texFrame == 0) e->texFrame = clip->length - 1;
            else                  e->texFrame++;

            if (clip->hasGlow) {
                if (e->texGlowFrame == 0) e->texGlowFrame = clip->glowLength - 1;
                else                      e->texGlowFrame--;
            }
        } else {
            e->texFrame = (e->texFrame + 1) % clip->length;
            if (clip->hasGlow) e->texGlowFrame = (e->texGlowFrame + 1) % clip->glowLength;
        }
    }

    if (e->textureAnimationStopsAtDead && e->health <= 0.0f && e->texFrame >= clip->length - 1) { e->textureAnimating = false; TurnLightOff(e->texAnimLight); TurnLightOff(e->texAnimLight2); }
    e->texIndex = sequenceTextures[ clip->frames[e->texFrame] ];
    if (clip->hasGlow && clip->glowFrames) e->glowIndex = sequenceTextures[ clip->glowFrames[e->texGlowFrame] ];
    if (e->index == 279 && !clip->hasGlow) e->glowIndex = e->texIndex;
}
