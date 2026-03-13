// animation.c - Animation System for both models and textures (in world and UI)
#include "voxen.h"
#define ANIM_LOOP_ALL 0
#define ANIM_IDLE_CLOSED 0
#define ANIM_OPENING     1
#define ANIM_IDLE_OPEN   2
#define ANIM_CLOSING     3
#define ANIM_INSTALL     4
#define ANIM_INSTALLED   5
#define ANIM_INACTIVE   0
#define ANIM_ACTIVATE   1
#define ANIM_ACTIVATED  2
#define ANIM_DEACTIVATE 3
#define ANIM_IDLE    0
#define ANIM_WALK    1
#define ANIM_RUN     2
#define ANIM_ATTACK1 3
#define ANIM_ATTACK2 4
#define ANIM_ATTACK3 5
#define ANIM_PAIN    6
#define ANIM_PAIN2   7
#define ANIM_PAIN3   8
#define ANIM_DYING   9
#define ANIM_ATTACK_MISS 1
#define ANIM_ATTACK_HIT  2

// static const float textureSequenceFrameDelay = 0.35f;
// static const float textureSequenceFrameDelayVmail = 0.09f;
const AnimationClip modelAnimationClips[MAX_ANIMATED_MODELS][MAX_ANIMATION_CLIPS_PER_MODEL] = { // speed, frameStart, frameEnd, frameStartModelIndex, framerate
    [0]={[ANIM_IDLE_CLOSED]={1.0f,2,2,699,24},[ANIM_OPENING]={1.0f,2,11,699,24},[ANIM_IDLE_OPEN]={1.0f,11,11,708,24},[ANIM_CLOSING]={1.0f,12,21,709,24}}, // doorB (door2)
    [1]={[ANIM_IDLE_CLOSED]={1.0f,2,2,719,24},[ANIM_OPENING]={1.0f,2,12,719,24},[ANIM_IDLE_OPEN]={1.0f,12,12,729,24},[ANIM_CLOSING]={1.0f,14,24,731,24}}, // doorA (door1)
    [2]={[ANIM_IDLE]={1.0f,0,37,742,30},[ANIM_WALK]={1.0f,50,99,780,30},[ANIM_RUN]={1.1f,50,99,792,30},[ANIM_ATTACK1]={0.75f,111,136,830,30},[ANIM_PAIN]={0.5f,138,150,856,30},[ANIM_DYING]={0.75f,153,176,869,30}}, // npc_humanoid_mutant
    [3]={[ANIM_IDLE]={1.0f,1,207,893,24},[ANIM_ATTACK1]={1.0f,219,239,1100,24},[ANIM_WALK]={1.0f,252,308,1121,24},[ANIM_RUN]={1.0f,252,308,1121,24},[ANIM_PAIN]={1.0f,321,330,1177,24},[ANIM_PAIN2]={1.0f,331,344,1187,24},[ANIM_DYING]={1.0f,345,369,1201,24}}, // npc_cyborg_drone 
    [4]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1234,24},[ANIM_OPENING]={1.5f,2,44,1234,24},[ANIM_IDLE_OPEN]={1.0f,44,44,1276,24},[ANIM_CLOSING]={1.75f,46,96,1277,24}}, // doorD (door4, bulkhead 1)
    [5]={[ANIM_IDLE_CLOSED]={1.0f,2,2,1328,24},[ANIM_OPENING]={1.0f,2,25,1328,24},[ANIM_IDLE_OPEN]={1.0f,25,25,1351,24},[ANIM_CLOSING]={1.0f,27,44,1352,24}}, // doorC (door3)
    [6]={[ANIM_IDLE_CLOSED]={1.0f,1,1,1444,24},[ANIM_OPENING]={1.2f,1,30,1444,24},[ANIM_IDLE_OPEN]={1.0f,30,30,1399,24},[ANIM_CLOSING]={1.2f,32,66,1400,24}}, // doorK (xdoor1)
    [7]={[ANIM_IDLE_CLOSED]={1.0f,3,3,1435,24},[ANIM_OPENING]={1.2f,3,24,1435,24},[ANIM_IDLE_OPEN]={1.0f,26,26,1457,24},[ANIM_CLOSING]={1.2f,27,49,1458,24}}, // doorJ (xdoor2)
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

void PortalCulling(void);
void UpdateAnims(void) {
    bool portalsNeedUpdated = false;
    for (uint16_t i = START_INDEX_LEVEL_INSTANCES; i < INSTANCE_COUNT; ++i) {
        if (Sys_Global.instances[i].modelIndex >= MODEL_IDX_MAX) continue;
        if (!(Sys_Global.instances[i].entflags & ENTFLAG_ACTIVE)) continue;
        
        uint16_t animNum = Sys_Global.instances[i].animationNum;
        if (animNum >= MAX_ANIMATED_MODELS) continue; // Invalid animated model index
        if (Sys_Global.instances[i].numclips >= MAX_ANIMATION_CLIPS_PER_MODEL) continue; // Invalid animation clip index
        if (Sys_Global.instances[i].numclips == 0) continue; // Invalid animation clip index
        if (!(Sys_Global.instances[i].entflags & ENTFLAG_ANIMATED)) continue;
        
        AnimationClip currentClip = modelAnimationClips[animNum][Sys_Global.instances[i].clip];
        if (Sys_Global.instances[i].currentFrameFinished >= Sys_Global.current_time) continue;
        
        Sys_Global.instances[i].currentFrameFinished = Sys_Global.current_time + ((1.0/(double)currentClip.speed) * (1.0 / (double)currentClip.framerate));
        Sys_Global.instances[i].frame++;
        if (Sys_Global.instances[i].frame > currentClip.frameEnd) Sys_Global.instances[i].frame = currentClip.frameStart;
        else if (Sys_Global.instances[i].frame < currentClip.frameStart) Sys_Global.instances[i].frame = currentClip.frameEnd;

        Sys_Global.instances[i].modelIndex = (currentClip.frameStartModelIndex + (Sys_Global.instances[i].frame - currentClip.frameStart));
        dirtyInstances[i] = true;
        if (!EntityIndexIsPortalBlockingDoor(Sys_Global.instances[i].index)) continue;
        
        uint8_t portalIdx = Sys_Global.instances[i].portalIndex;
        if (portalIdx >= MAX_PORTALS) continue;
        
        uint16_t closedModelIndex = modelAnimationClips[animNum][ANIM_IDLE_CLOSED].frameStartModelIndex;                    
        bool currentState = activePortals[portalIdx].open;
        if (Sys_Global.instances[i].modelIndex == closedModelIndex && currentState) {
            activePortals[portalIdx].open = false;
            activePortals[portalIdx].dirty = true;
            portalsNeedUpdated = true;
        } else if (Sys_Global.instances[i].modelIndex != closedModelIndex && !currentState) {
            activePortals[portalIdx].open = true;
            activePortals[portalIdx].dirty = true;
            portalsNeedUpdated = true;
        }
    }
    
    if (portalsNeedUpdated) PortalCulling();
}

// uint16_t sequenceTextures[302]={};
/*scr_exp 01 - 06
 bridg1_1 001 - 004
 broken_clock 01 - 02
 broken_clock01_glow 01 - 02
 g_energmine 00 - 06
 g_energmine_glow 00 - 07 (yes different count, supported!)
 scr_cita2 0 - 3
 scr_cita3 0 - 3
 scr_cita 0 - 7
 engscreen1_04 // index 45
 scr_static2 0 - 6, then scr_static2_a
 scr_static 0 - 5
 screen1 0 - 3
 screen2 0 - 4
 screen3 0 - 3
 screen4 0 - 5
 screen5 0 - 3
 triop1 0 - 3
 triop2 0 - 9
 triop3 0 - 7
 triop4_8 // index 105
 triop3_0 // index 106
 dna 0 - 7
 edcolor 0 - 3
 edgray 0 - 3
 ammo_magcart 00 - 10
 ammo_magcart_glow 00 - 10
 medicalbed 00 - 9
 rad1_1 00 - 04
 screencode 0 - 9
 shodanstatic 00 - 36
 telepad 00 - 03
 telepad_00_glow
 black // index 212
 black
 black
 black
 black
 black // index 217
 medscreen13 00 - 11
 medscreen24 00 - 07
 medscreen16 00 - 07
 zerog 00 - 52
 door_x1 01 - 03 // ends at index 301
 */

/*
	public int initialIndexOffset = 0;
	public bool animateGlow = false;
	public bool randomFrame = false; // randomly pick a frame instead of sequential
	public bool reverseSequence = false;
	public bool glowOnly = false;
	public bool screenDestroyed = false;
	public int[] constArrayLookup;
	public int[] constArrayLookupGlow;
	public int[] constArrayDestroyed;
	public float tickFinished;
	public GameObject lightContainer;

	private AudioSource SFX;
	private float tick;
	private bool screenDestroyedDone = false; // Delay ending animation for a few destroy frames.
	private bool screenDestroyFirstFrame = true;
	private MeshRenderer mR;
	private Material goMaterial;
	private Light lit;
	private int frameCounter = 0; // An integer to advance frames
	private int frameCounterGlow = 0;
	private PrefabIdentifier pid;

	void Awake() {
		// Get a reference to the Material of the game object this script is attached to.
		mR = GetComponent<MeshRenderer>();
		if (mR == null) { this.flag_set(&SELF.entflags, ENTFLAG_ACTIVE, false); return; }
		this.goMaterial = this.GetComponent<Renderer>().material;
		SFX = GetComponent<AudioSource>();
		pid = GetComponent<PrefabIdentifier>();
		if (lightContainer != null) {
			lit = lightContainer.GetComponent<Light>();
			if (lit != null) {
				if ((Sys_Global.instances[i].scale.x < 1.0f) || (Sys_Global.instances[i].scale.y < 1.0f) || (Sys_Global.instances[i].scale.z < 1.0f)) {
					float factor = vmin(Sys_Global.instances[i].scale.x, Sys_Global.instances[i].scale.y, Sys_Global.instances[i].scale.z);
					lit.range *= factor;
					if (lit.range < 2.0f) lit.range = 2.0f;
				}
			}
		}
	}

	// called by HealthManager.cs's ScreenDeath
	public void Destroy() {
		Utils.PlayOneShotSavable(SFX,sounds[69]); // screen_destroy
		if (lightContainer != null) lightContainer.SetActive(false);
		screenDestroyed = true; // if not already dead, say so
	}

	public void AwakeFromLoad(float health) {
		if (!this.enabled) return;

		if (health > 0) {
			screenDestroyed = screenDestroyedDone = false;
			if (lightContainer != null) lightContainer.SetActive(true);
			tickFinished = Sys_Global.pauseRelativeTime + tick;
			SetFrameIndices();
		} else {
			if (lightContainer != null) lightContainer.SetActive(false);
			screenDestroyed = true;
			goMaterial.mainTexture = Const.a.sequenceTextures[5]; // End frame of destroyed texture
			goMaterial.SetTexture("_EmissionMap", Const.a.sequenceTextures[5]);
		}
	}

	void SetFrameIndices() {
		if (reverseSequence) {
			if (constArrayLookup != null) frameCounter = constArrayLookup.Length - 1; // Start counting down from end.
			if (constArrayLookupGlow != null) frameCounterGlow = constArrayLookupGlow.Length - 1;
		} else {
			frameCounter = frameCounterGlow = 0; // Start counting up from 0.
		}

		if (constArrayLookupGlow != null) {
			if (initialIndexOffset < constArrayLookupGlow.Length  && initialIndexOffset > 0) frameCounterGlow = initialIndexOffset;
		}
		if (constArrayLookup != null) {
			if (initialIndexOffset < constArrayLookup.Length && initialIndexOffset > 0) frameCounter = initialIndexOffset;
		}
	}

	void Start () {
		if (PauseScript.a == null) { this.enabled = false; return; }

		//Load all textures found on the Sequence folder, that is placed inside the resources folder
		if (StringIsEmpty(resourceFolder)) resourceFolder = glowResourceFolder;
		if (StringIsEmpty(resourceFolder)) return;

		tick = frameDelay;
		tickFinished = Sys_Global.pauseRelativeTime + tick;

		// New method...long, but reduces overall memory load from duplicate
		// frames and reduces startup time by over 8 seconds. :party:
		if (resourceFolder == "Bridge11") {
			// Normal
			constArrayLookup = new int[8];
			constArrayLookup[0] = 6;
			constArrayLookup[1] = 7;
			constArrayLookup[2] = 8;
			constArrayLookup[3] = 9;
			constArrayLookup[4] = 9;
			constArrayLookup[5] = 8;
			constArrayLookup[6] = 7;
			constArrayLookup[7] = 6;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "BrokenClock") {
			// Normal
			constArrayLookup = new int[2];
			constArrayLookup[0] = 10;
			constArrayLookup[1] = 11;
			// Glow
			constArrayLookupGlow = new int[2];
			constArrayLookupGlow[0] = 12;
			constArrayLookupGlow[1] = 13;
		} else if (resourceFolder == "EnergMine") {
			// Normal
			constArrayLookup = new int[7];
			constArrayLookup[0] = 14;
			constArrayLookup[1] = 15;
			constArrayLookup[2] = 16;
			constArrayLookup[3] = 17;
			constArrayLookup[4] = 18;
			constArrayLookup[5] = 19;
			constArrayLookup[6] = 20;
			// Glow
			constArrayLookupGlow = new int[8];
			constArrayLookupGlow[0] = 21;
			constArrayLookupGlow[1] = 22;
			constArrayLookupGlow[2] = 23;
			constArrayLookupGlow[3] = 24;
			constArrayLookupGlow[4] = 25;
			constArrayLookupGlow[5] = 26;
			constArrayLookupGlow[6] = 27;
			constArrayLookupGlow[7] = 28;
		} else if (resourceFolder == "EngScreen1") {
			// Normal
			constArrayLookup = new int[43];
			constArrayLookup[0] = 29;
			constArrayLookup[1] = 30;
			constArrayLookup[2] = 31;
			constArrayLookup[3] = 32;
			constArrayLookup[4] = 45;
			constArrayLookup[5] = 45;
			constArrayLookup[6] = 45;
			constArrayLookup[7] = 29;
			constArrayLookup[8] = 30;
			constArrayLookup[9] = 31;
			constArrayLookup[10] = 32;
			constArrayLookup[11] = 29;
			constArrayLookup[12] = 30;
			constArrayLookup[13] = 31;
			constArrayLookup[14] = 32;
			constArrayLookup[15] = 29;
			constArrayLookup[16] = 30;
			constArrayLookup[17] = 31;
			constArrayLookup[18] = 32;
			constArrayLookup[19] = 29;
			constArrayLookup[20] = 45;
			constArrayLookup[21] = 45;
			constArrayLookup[22] = 45;
			constArrayLookup[23] = 29;
			constArrayLookup[24] = 30;
			constArrayLookup[25] = 31;
			constArrayLookup[26] = 32;
			constArrayLookup[27] = 29;
			constArrayLookup[28] = 30;
			constArrayLookup[29] = 31;
			constArrayLookup[30] = 32;
			constArrayLookup[31] = 29;
			constArrayLookup[32] = 30;
			constArrayLookup[33] = 31;
			constArrayLookup[34] = 32;
			constArrayLookup[35] = 29;
			constArrayLookup[36] = 30;
			constArrayLookup[37] = 31;
			constArrayLookup[38] = 32;
			constArrayLookup[39] = 29;
			constArrayLookup[40] = 45;
			constArrayLookup[41] = 45;
			constArrayLookup[42] = 45;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "EngScreen2") {
			// Normal
			constArrayLookup = new int[8];
			constArrayLookup[0] = 52;
			constArrayLookup[1] = 51;
			constArrayLookup[2] = 50;
			constArrayLookup[3] = 49;
			constArrayLookup[4] = 49;
			constArrayLookup[5] = 50;
			constArrayLookup[6] = 51;
			constArrayLookup[7] = 52;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "ExecScreen1") {
			// Normal
			constArrayLookup = new int[13];
			constArrayLookup[0] = 29;
			constArrayLookup[1] = 30;
			constArrayLookup[2] = 31;
			constArrayLookup[3] = 32;
			constArrayLookup[4] = 45;
			constArrayLookup[5] = 29;
			constArrayLookup[6] = 30;
			constArrayLookup[7] = 31;
			constArrayLookup[8] = 45;
			constArrayLookup[9] = 29;
			constArrayLookup[10] = 30;
			constArrayLookup[11] = 31;
			constArrayLookup[12] = 45;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "ExecScreen2") {
			// Normal
			constArrayLookup = new int[21];
			constArrayLookup[0] = 83;
			constArrayLookup[1] = 84;
			constArrayLookup[2] = 85;
			constArrayLookup[3] = 86;
			constArrayLookup[4] = 83;
			constArrayLookup[5] = 83;
			constArrayLookup[6] = 86;
			constArrayLookup[7] = 85;
			constArrayLookup[8] = 84;
			constArrayLookup[9] = 83;
			constArrayLookup[10] = 103; // 711...yep!
			constArrayLookup[11] = 83;
			constArrayLookup[12] = 84;
			constArrayLookup[13] = 85;
			constArrayLookup[14] = 86;
			constArrayLookup[15] = 83;
			constArrayLookup[16] = 83;
			constArrayLookup[17] = 86;
			constArrayLookup[18] = 85;
			constArrayLookup[19] = 84;
			constArrayLookup[20] = 83;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "ExecScreen3") {
			// Normal
			constArrayLookup = new int[3];
			constArrayLookup[0] = 115;
			constArrayLookup[1] = 115;
			constArrayLookup[2] = 117;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "ExecScreen4") {
			// Normal
			constArrayLookup = new int[7];
			constArrayLookup[0] = 115;
			constArrayLookup[1] = 115;
			constArrayLookup[2] = 115;
			constArrayLookup[3] = 115;
			constArrayLookup[4] = 116;
			constArrayLookup[5] = 117;
			constArrayLookup[6] = 118;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "MagCartridge") {
			// Normal
			constArrayLookup = new int[11];
			constArrayLookup[0] = 123;
			constArrayLookup[1] = 124;
			constArrayLookup[2] = 125;
			constArrayLookup[3] = 126;
			constArrayLookup[4] = 127;
			constArrayLookup[5] = 128;
			constArrayLookup[6] = 129;
			constArrayLookup[7] = 130;
			constArrayLookup[8] = 131;
			constArrayLookup[9] = 132;
			constArrayLookup[10] = 133;
			// Glow
			constArrayLookupGlow = new int[11];
			constArrayLookupGlow[0] = 134;
			constArrayLookupGlow[1] = 135;
			constArrayLookupGlow[2] = 136;
			constArrayLookupGlow[3] = 137;
			constArrayLookupGlow[4] = 138;
			constArrayLookupGlow[5] = 139;
			constArrayLookupGlow[6] = 140;
			constArrayLookupGlow[7] = 141;
			constArrayLookupGlow[8] = 142;
			constArrayLookupGlow[9] = 143;
			constArrayLookupGlow[10] = 144;
		} else if (resourceFolder == "MaintScreen1") {
			// Normal
			constArrayLookup = new int[5];
			constArrayLookup[0] = 29;
			constArrayLookup[1] = 30;
			constArrayLookup[2] = 31;
			constArrayLookup[3] = 32;
			constArrayLookup[4] = 37;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "MaintScreen2") {
			// Normal
			constArrayLookup = new int[6];
			constArrayLookup[0] = 33;
			constArrayLookup[1] = 34;
			constArrayLookup[2] = 35;
			constArrayLookup[3] = 36;
			constArrayLookup[4] = 32;
			constArrayLookup[5] = 29;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "MedicalBed") {
			// Normal
			constArrayLookup = new int[10];
			constArrayLookup[0] = 145;
			constArrayLookup[1] = 146;
			constArrayLookup[2] = 147;
			constArrayLookup[3] = 148;
			constArrayLookup[4] = 149;
			constArrayLookup[5] = 150;
			constArrayLookup[6] = 151;
			constArrayLookup[7] = 152;
			constArrayLookup[8] = 153;
			constArrayLookup[9] = 154;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "MedScreen1") {
			// Normal
			constArrayLookup = new int[6];
			constArrayLookup[0] = 54;
			constArrayLookup[1] = 59;
			constArrayLookup[2] = 118;
			constArrayLookup[3] = 116;
			constArrayLookup[4] = 118;
			constArrayLookup[5] = 59;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "MedScreen2") {
			// Normal
			constArrayLookup = new int[4];
			constArrayLookup[0] = 79;
			constArrayLookup[1] = 80;
			constArrayLookup[2] = 81;
			constArrayLookup[3] = 82;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "MedScreen3") {
			// Normal
			constArrayLookup = new int[4];
			constArrayLookup[0] = 99;
			constArrayLookup[1] = 98;
			constArrayLookup[2] = 97;
			constArrayLookup[3] = 92;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "MedScreen4") {
			// Normal
			constArrayLookup = new int[4];
			constArrayLookup[0] = 29;
			constArrayLookup[1] = 30;
			constArrayLookup[2] = 31;
			constArrayLookup[3] = 36;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "MedScreen5") {
			// Normal
			constArrayLookup = new int[8];
			constArrayLookup[0] = 56;
			constArrayLookup[1] = 55;
			constArrayLookup[2] = 54;
			constArrayLookup[3] = 59;
			constArrayLookup[4] = 59;
			constArrayLookup[5] = 54;
			constArrayLookup[6] = 55;
			constArrayLookup[7] = 56;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "MedScreen6") {
			// Normal
			constArrayLookup = new int[12];
			constArrayLookup[0] = 61;
			constArrayLookup[1] = 61;
			constArrayLookup[2] = 62;
			constArrayLookup[3] = 62;
			constArrayLookup[4] = 61;
			constArrayLookup[5] = 61;
			constArrayLookup[6] = 212;
			constArrayLookup[7] = 213;
			constArrayLookup[8] = 214;
			constArrayLookup[9] = 215;
			constArrayLookup[10]= 216;
			constArrayLookup[11]= 217;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "MedScreen7") {
			// Normal
			constArrayLookup = new int[4];
			constArrayLookup[0] = 119;
			constArrayLookup[1] = 120;
			constArrayLookup[2] = 121;
			constArrayLookup[3] = 122;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "MedScreen8") {
			// Normal
			constArrayLookup = new int[4];
			constArrayLookup[0] = 59;
			constArrayLookup[1] = 54;
			constArrayLookup[2] = 55;
			constArrayLookup[3] = 56;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "MedScreen9") {
			// Normal
			constArrayLookup = new int[8];
			constArrayLookup[0] = 37;
			constArrayLookup[1] = 38;
			constArrayLookup[2] = 39;
			constArrayLookup[3] = 40;
			constArrayLookup[4] = 41;
			constArrayLookup[5] = 42;
			constArrayLookup[6] = 43;
			constArrayLookup[7] = 44;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "MedScreen10") {
			// Normal
			constArrayLookup = new int[10];
			constArrayLookup[0] = 83;
			constArrayLookup[1] = 84;
			constArrayLookup[2] = 85;
			constArrayLookup[3] = 86;
			constArrayLookup[4] = 83;
			constArrayLookup[5] = 83;
			constArrayLookup[6] = 86;
			constArrayLookup[7] = 85;
			constArrayLookup[8] = 84;
			constArrayLookup[9] = 83;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "MedScreen11") {
			// Normal
			constArrayLookup = new int[8];
			constArrayLookup[0] = 67;
			constArrayLookup[1] = 66;
			constArrayLookup[2] = 66;
			constArrayLookup[3] = 67;
			constArrayLookup[4] = 79;
			constArrayLookup[5] = 80;
			constArrayLookup[6] = 80;
			constArrayLookup[7] = 79;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "MedScreen13") {
			// Normal
			constArrayLookup = new int[12];
			constArrayLookup[0] = 218;
			constArrayLookup[1] = 219;
			constArrayLookup[2] = 220;
			constArrayLookup[3] = 221;
			constArrayLookup[4] = 222;
			constArrayLookup[5] = 223;
			constArrayLookup[6] = 224;
			constArrayLookup[7] = 225;
			constArrayLookup[8] = 226;
			constArrayLookup[9] = 227;
			constArrayLookup[10]= 228;
			constArrayLookup[11]= 229;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "MedScreen16") {
			// Normal
			constArrayLookup = new int[8];
			constArrayLookup[0] = 79;
			constArrayLookup[1] = 80;
			constArrayLookup[2] = 81;
			constArrayLookup[3] = 82;
			constArrayLookup[4] = 82;
			constArrayLookup[5] = 81;
			constArrayLookup[6] = 80;
			constArrayLookup[7] = 79;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "MedScreen18") {
			// Normal
			constArrayLookup = new int[6];
			constArrayLookup[0] = 73;
			constArrayLookup[1] = 74;
			constArrayLookup[2] = 75;
			constArrayLookup[3] = 76;
			constArrayLookup[4] = 77;
			constArrayLookup[5] = 78;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "MedScreen22") {
			// Normal
			constArrayLookup = new int[8];
			constArrayLookup[0] = 73;
			constArrayLookup[1] = 74;
			constArrayLookup[2] = 76;
			constArrayLookup[3] = 75;
			constArrayLookup[4] = 77;
			constArrayLookup[5] = 76;
			constArrayLookup[6] = 78;
			constArrayLookup[7] = 73;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "MedScreen23") {
			// Normal
			constArrayLookup = new int[4];
			constArrayLookup[0] = 29;
			constArrayLookup[1] = 30;
			constArrayLookup[2] = 31;
			constArrayLookup[3] = 32;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "MedScreen24") {
			// Normal
			constArrayLookup = new int[8];
			constArrayLookup[0] = 230;
			constArrayLookup[1] = 231;
			constArrayLookup[2] = 232;
			constArrayLookup[3] = 233;
			constArrayLookup[4] = 234;
			constArrayLookup[5] = 235;
			constArrayLookup[6] = 236;
			constArrayLookup[7] = 237;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "MedScreen25") {
			// Normal
			constArrayLookup = new int[4];
			constArrayLookup[0] = 92;
			constArrayLookup[1] = 93;
			constArrayLookup[2] = 94;
			constArrayLookup[3] = 95;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "MedScreen27") {
			// Normal
			constArrayLookup = new int[8];
			constArrayLookup[0] = 238;
			constArrayLookup[1] = 239;
			constArrayLookup[2] = 240;
			constArrayLookup[3] = 241;
			constArrayLookup[4] = 242;
			constArrayLookup[5] = 243;
			constArrayLookup[6] = 244;
			constArrayLookup[7] = 245;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "MedScreen29") {
			// Normal
			constArrayLookup = new int[5];
			constArrayLookup[0] = 64;
			constArrayLookup[1] = 65;
			constArrayLookup[2] = 66;
			constArrayLookup[3] = 67;
			constArrayLookup[4] = 68;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "Rad1_1") {
			// Normal
			constArrayLookup = new int[5];
			constArrayLookup[0] = 155;
			constArrayLookup[1] = 156;
			constArrayLookup[2] = 157;
			constArrayLookup[3] = 158;
			constArrayLookup[4] = 159;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "ReacScreen4") {
			// Normal
			constArrayLookup = new int[4];
			constArrayLookup[0] = 61;
			constArrayLookup[1] = 61;
			constArrayLookup[2] = 62;
			constArrayLookup[3] = 62;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "SciScreen1") {
			// Normal
			constArrayLookup = new int[3];
			constArrayLookup[0] = 62;
			constArrayLookup[1] = 61;
			constArrayLookup[2] = 60;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "SciScreen2") {
			// Normal
			constArrayLookup = new int[7];
			constArrayLookup[0] = 107;
			constArrayLookup[1] = 108;
			constArrayLookup[2] = 109;
			constArrayLookup[3] = 111;
			constArrayLookup[4] = 112;
			constArrayLookup[5] = 113;
			constArrayLookup[6] = 114;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "SciScreen3") {
			// Normal
			constArrayLookup = new int[4];
			constArrayLookup[0] = 33;
			constArrayLookup[1] = 34;
			constArrayLookup[2] = 35;
			constArrayLookup[3] = 36;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "SciScreen4") {
			// Normal
			constArrayLookup = new int[4];
			constArrayLookup[0] = 188;
			constArrayLookup[1] = 189;
			constArrayLookup[2] = 113;
			constArrayLookup[3] = 112;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "SciScreen5") {
			// Normal
			constArrayLookup = new int[9];
			constArrayLookup[0] = 79;
			constArrayLookup[1] = 80;
			constArrayLookup[2] = 80;
			constArrayLookup[3] = 79;
			constArrayLookup[4] = 73;
			constArrayLookup[5] = 74;
			constArrayLookup[6] = 76;
			constArrayLookup[7] = 77;
			constArrayLookup[8] = 75;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "ScreenDestroyed") {
			// Normal
			constArrayLookup = new int[6];
			constArrayLookup[0] = 0;
			constArrayLookup[1] = 1;
			constArrayLookup[2] = 2;
			constArrayLookup[3] = 3;
			constArrayLookup[4] = 4;
			constArrayLookup[5] = 5;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "ScreenCodeRandom") {
			// Normal
			constArrayLookup = new int[10];
			constArrayLookup[0] = 160;
			constArrayLookup[1] = 161;
			constArrayLookup[2] = 162;
			constArrayLookup[3] = 163;
			constArrayLookup[4] = 164;
			constArrayLookup[5] = 165;
			constArrayLookup[6] = 166;
			constArrayLookup[7] = 167;
			constArrayLookup[8] = 168;
			constArrayLookup[9] = 169;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "SecScreen4") {
			// Normal
			constArrayLookup = new int[3];
			constArrayLookup[0] = 29;
			constArrayLookup[1] = 30;
			constArrayLookup[2] = 31;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "ShodanStatic") {
			// Normal
			constArrayLookup = new int[60];
			constArrayLookup[0] = 170;
			constArrayLookup[1] = 171;
			constArrayLookup[2] = 172;
			constArrayLookup[3] = 173;
			constArrayLookup[4] = 174;
			constArrayLookup[5] = 175;
			constArrayLookup[6] = 176;
			constArrayLookup[7] = 177;
			constArrayLookup[8] = 178;
			constArrayLookup[9] = 179;
			constArrayLookup[10]= 180;
			constArrayLookup[11]= 181;
			constArrayLookup[12]= 182;
			constArrayLookup[13]= 183;
			constArrayLookup[14]= 184;
			constArrayLookup[15]= 185;
			constArrayLookup[16]= 186;
			constArrayLookup[17]= 187;
			constArrayLookup[18]= 188;
			constArrayLookup[19]= 189;
			constArrayLookup[20]= 190;
			constArrayLookup[21]= 191;
			constArrayLookup[22]= 192;
			constArrayLookup[23]= 193;
			constArrayLookup[24]= 194;
			constArrayLookup[25]= 195;
			constArrayLookup[26]= 196;
			constArrayLookup[27]= 197;
			constArrayLookup[28]= 198;
			constArrayLookup[29]= 199;
			constArrayLookup[30]= 200;
			constArrayLookup[31]= 201;
			constArrayLookup[32]= 202;
			constArrayLookup[33]= 203;
			constArrayLookup[34]= 204;
			constArrayLookup[35]= 205;
			constArrayLookup[36]= 206;
			constArrayLookup[37]= 203;
			constArrayLookup[38]= 204;
			constArrayLookup[39]= 205;
			constArrayLookup[40]= 206;
			constArrayLookup[41]= 205;
			constArrayLookup[42]= 203;
			constArrayLookup[43]= 204;
			constArrayLookup[44]= 206;
			constArrayLookup[45]= 205;
			constArrayLookup[46]= 204;
			constArrayLookup[47]= 203;
			constArrayLookup[48]= 206;
			constArrayLookup[49]= 205;
			constArrayLookup[50]= 203;
			constArrayLookup[51]= 204;
			constArrayLookup[52]= 205;
			constArrayLookup[53]= 206;
			constArrayLookup[54]= 203;
			constArrayLookup[55]= 206;
			constArrayLookup[56]= 205;
			constArrayLookup[57]= 203;
			constArrayLookup[58]= 204;
			constArrayLookup[59]= 203;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "Static") {
			// Normal
			constArrayLookup = new int[17];
			constArrayLookup[0] = 203;
			constArrayLookup[1] = 204;
			constArrayLookup[2] = 205;
			constArrayLookup[3] = 206;
			constArrayLookup[4] = 203;
			constArrayLookup[5] = 206;
			constArrayLookup[6] = 205;
			constArrayLookup[7] = 203;
			constArrayLookup[8] = 204;
			constArrayLookup[9] = 205;
			constArrayLookup[10]= 206;
			constArrayLookup[11]= 203;
			constArrayLookup[12]= 206;
			constArrayLookup[13]= 205;
			constArrayLookup[14]= 203;
			constArrayLookup[15]= 204;
			constArrayLookup[16]= 203;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "Telepad") {
			// Normal
			constArrayLookup = new int[4];
			constArrayLookup[0] = 207;
			constArrayLookup[1] = 208;
			constArrayLookup[2] = 209;
			constArrayLookup[3] = 210;
			// Glow
			constArrayLookupGlow = new int[4];
			constArrayLookupGlow[0] = 207;
			constArrayLookupGlow[1] = 208;
			constArrayLookupGlow[2] = 209;
			constArrayLookupGlow[3] = 210;
		} else if (resourceFolder == "XDoor1") {
			// Normal
			constArrayLookup = new int[3];
			constArrayLookup[0] = 299;
			constArrayLookup[1] = 300;
			constArrayLookup[2] = 301;
			// Glow
			constArrayLookupGlow = null;
		} else if (resourceFolder == "ZeroGMutant") {
			// Normal
			constArrayLookup = new int[53];
			constArrayLookup[0] = 246;
			constArrayLookup[1] = 247;
			constArrayLookup[2] = 248;
			constArrayLookup[3] = 249;
			constArrayLookup[4] = 250;
			constArrayLookup[5] = 251;
			constArrayLookup[6] = 252;
			constArrayLookup[7] = 253;
			constArrayLookup[8] = 254;
			constArrayLookup[9] = 255;
			constArrayLookup[10]= 256;
			constArrayLookup[11]= 257;
			constArrayLookup[12]= 258;
			constArrayLookup[13]= 259;
			constArrayLookup[14]= 260;
			constArrayLookup[15]= 261;
			constArrayLookup[16]= 262;
			constArrayLookup[17]= 263;
			constArrayLookup[18]= 264;
			constArrayLookup[19]= 265;
			constArrayLookup[20]= 266;
			constArrayLookup[21]= 267;
			constArrayLookup[22]= 268;
			constArrayLookup[23]= 269;
			constArrayLookup[24]= 270;
			constArrayLookup[25]= 271;
			constArrayLookup[26]= 272;
			constArrayLookup[27]= 273;
			constArrayLookup[28]= 274;
			constArrayLookup[29]= 275;
			constArrayLookup[30]= 276;
			constArrayLookup[31]= 277;
			constArrayLookup[32]= 278;
			constArrayLookup[33]= 279;
			constArrayLookup[34]= 280;
			constArrayLookup[35]= 281;
			constArrayLookup[36]= 282;
			constArrayLookup[37]= 283;
			constArrayLookup[38]= 284;
			constArrayLookup[39]= 285;
			constArrayLookup[40]= 286;
			constArrayLookup[41]= 287;
			constArrayLookup[42]= 288;
			constArrayLookup[43]= 289;
			constArrayLookup[44]= 290;
			constArrayLookup[45]= 291;
			constArrayLookup[46]= 292;
			constArrayLookup[47]= 293;
			constArrayLookup[48]= 294;
			constArrayLookup[49]= 295;
			constArrayLookup[50]= 296;
			constArrayLookup[51]= 297;
			constArrayLookup[52]= 298;
			// Glow
			constArrayLookupGlow = null;
		}
		SetFrameIndices();
	}

	void Update() {
		if (!Sys_Global.gamePaused && !Sys_Global.menuActive) {
			if (StringIsEmpty(resourceFolder)) { this.enabled = false; return; }

			if (mR.isVisible) {
				if (tickFinished < Sys_Global.pauseRelativeTime) {
					Think();
					tickFinished = Sys_Global.pauseRelativeTime + tick;
				}
			}
		}
	}

	void Think () {
		// animate through the screen destruction textures once then stop once frameCounter reaches the end
		if (screenDestroyed) {
			if (screenDestroyedDone) return; // all done, yoohoo see ya bye bye

			if (screenDestroyFirstFrame) {
				frameCounter = 0; // set destroyed frame to 0 for the first frame, we were just destroyed but let's keep using same frameCounter but reset it first
				screenDestroyFirstFrame = false; // flip bit so we don't keep setting current frame to 0 endlessly
			}

			frameCounter++;
			if (frameCounter > 5) {
				screenDestroyedDone = true; // stop continuing to increment counter, all done counting
				return; // we are done, no need to continue animating frames, destruction complete, unit lost, unit ready, wait is this a Command and Conquer reference?  Yes.  Yes it is.
			}

			//Set the material's texture to the current value of the frameCounter variable
			if (frameCounter >= 0 && frameCounter <= 5) {
				goMaterial.mainTexture = Const.a.sequenceTextures[frameCounter]; // 0 thru 5
				goMaterial.SetTexture("_EmissionMap", Const.a.sequenceTextures[frameCounter]);
			}
			return;
		}

		// Oh hey we aren't destroyed yet, so let's get to actual animating!

		// Flip it, on the back. -Nathan Drake
		if (reverseSequence) {
			if (constArrayLookup != null) frameCounter = (--frameCounter) % constArrayLookup.Length-1;
			if (constArrayLookupGlow != null) frameCounterGlow = (--frameCounterGlow) % constArrayLookup.Length-1;
		} else {
			if (constArrayLookup != null) {
				frameCounter++;
				if (frameCounter > (constArrayLookup.Length - 1)) frameCounter = 0;
			}
			if (constArrayLookupGlow != null) {
				frameCounterGlow++;
				if (frameCounterGlow > (constArrayLookupGlow.Length - 1)) frameCounterGlow = 0;
			}
		}

		// We don't know where we are going, or when.
		if (randomFrame) {
			if (constArrayLookup != null) frameCounter = random_range(0, constArrayLookup.Length-1);
			if (constArrayLookupGlow != null) {
				if (frameCounter < constArrayLookupGlow.Length) frameCounterGlow = frameCounter; // Match when it makes sense.
				else frameCounterGlow = random_range(0, constArrayLookupGlow.Length-1); // Otherwise randomize it.
			}
		}

		if (constArrayLookupGlow != null) {
			if (constArrayLookupGlow.Length > 0) {
				if (frameCounterGlow < constArrayLookupGlow.Length) {
					if (constArrayLookupGlow[frameCounterGlow] < Const.a.sequenceTextures.Length && constArrayLookupGlow[frameCounterGlow] >= 0) {
						goMaterial.SetTexture("_EmissionMap", Const.a.sequenceTextures[constArrayLookupGlow[frameCounterGlow]]);
					}
				}
			}
		}

		if (glowOnly) return;

		if (constArrayLookup != null) {
			if (constArrayLookup.Length > 0 && frameCounter < constArrayLookup.Length) {
				if (constArrayLookup[frameCounter] < Const.a.sequenceTextures.Length && constArrayLookup[frameCounter] >= 0) {
					if (goMaterial.mainTexture != Const.a.sequenceTextures[constArrayLookup[frameCounter]]) goMaterial.mainTexture = Const.a.sequenceTextures[constArrayLookup[frameCounter]];
					if (pid != null) {
						if (pid.constIndex == 279) {
							goMaterial.SetTexture("_EmissionMap", Const.a.sequenceTextures[constArrayLookup[frameCounter]]);
						}
					}
				}
			}
		}
	}
}*/

// public class ImageSequenceTextureArrayUI : MonoBehaviour {
// 	private Object[] objects;
// 	private Sprite[] sprites;
// 	private Image goImage;
// 	private int frameCounter = 0;
// 	public bool stopAtEnd = false; // True for Vmail
// 	private bool playDone = false;
// 	public bool replayOnEnable = false; // True for Vmail
// 	public bool playOnMenu = false; // False for Vmail
// 	public bool deactivateAtEnd = false;
// 	
// 	void Awake() {
// 		goImage = this.GetComponent<Image>();
// 	}
// 	
// 	void Start () {
// 		objects = Resources.LoadAll(resourceFolder, typeof(Sprite)); //Load all textures found on the Sequence folder, that is placed inside the resources folder
// 		if (resourceFolder == "AAOutro") DualLog("objects.Length: " + objects.Length.ToString());
// 		sprites = new Sprite[objects.Length]; //Initialize the array of textures with the same size as the objects array
// 
// 		//Cast each Object to Texture and store the result inside the Textures array
// 		for(int i=0; i < objects.Length;i++) {
// 			this.sprites[i] = (Sprite)this.objects[i];
// 		}
// 	}
// 
// 	void OnEnable() {
// 		Start();
// 		if (replayOnEnable) {
// 			playDone = false;
// 			frameCounter = 0;
// 		}
// 	}
// 	
// 	void Update() {
// 		if (!Sys_Global.gamePaused || playOnMenu) {
// 			if (!Sys_Global.menuActive || playOnMenu) {
// 				if (deactivateAtEnd && playDone) flag_set(&SELF.entflags, ENTFLAG_ACTIVE, false);
// 				if (stopAtEnd && playDone) return;
// 
// 				if (stopAtEnd && !playDone) {
// 					StartCoroutine("Play", frameDelay);
// 				} else {
// 					StartCoroutine("PlayLoop", frameDelay); // Call the 'PlayLoop' method as a coroutine with a float delay.
// 				}
// 
// 				// Set the material's texture to current value of frameCounter.
// 				if (frameCounter < sprites.Length && frameCounter >= 0) {
// 					goImage.overrideSprite = sprites[frameCounter];
// 				}
// 			}
// 		}
// 	}
// 
// 	IEnumerator PlayLoop(float delay) {
// 		yield return new WaitForSeconds(delay); // Wait for the time defined at the delay parameter.
// TryAgain:
// 		if (Sys_Global.gamePaused && !playOnMenu) {
// 			yield return null;
// 			goto TryAgain;
// 		}
// 		
// 		frameCounter = (++frameCounter)%sprites.Length; // Advance one frame
// 		StopCoroutine("PlayLoop"); // Stop this coroutine
// 	}  
// 
// 	IEnumerator Play(float delay) {
// 		yield return new WaitForSeconds(delay); // Wait for the time defined at the delay parameter.
// 		
// 		// If the frame counter isn't at the last frame.
// 		if(frameCounter < sprites.Length-1) {
// 			++frameCounter; // Advance one frame.
// 			if (frameCounter >= sprites.Length) playDone = true;
// 		} else {
// 			playDone = true;
// 		}
// 		StopCoroutine("Play"); //Stop this coroutine
// 	} 
// 	
// 	void OnDestroy() {
// 		objects = null;
// 		sprites = null;
// 	}
// }
