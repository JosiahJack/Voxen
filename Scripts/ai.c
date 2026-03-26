// ai.c - AI logic control for NPC's enemies in the game.
#include "mod.h"
#define MIN_WALK_SPEED_SQ    (0.32f * 0.32f)
#define ANIM_WALK_SWAP_DELAY 0.5
#define AI_TICK_TIME         0.2
#define AI_RAYCAST_TICK      0.1
#define AI_STOP_DIST         1.28f
#define AI_STOP_DIST_SQ      (AI_STOP_DIST * AI_STOP_DIST)
#define AI_POS_CHECK_DELAY   2.0
#define AI_SEARCH_TIME       5.0
#define AI_MAX_RAYCASTS      8
#define AI_WANDER_RANGE      79.0f
#define AI_ASTAR_STEP        2.56f
#define AI_TARGET_OFFSET_Y   0.24f
#define MIN_WALK_SPEED_SQ    (0.32f * 0.32f)
#define ANIM_WALK_SWAP_DELAY 0.5
const float stopDistance = 1.28f; // Constant
const float positionCheckDelay = 2.0f;
const float searchTime = 5.0f;
Vector3 targetOffset = (Vector3){0.0f, 0.24f, 0.0f};
uint16_t npcCountInWorldPerType[NUM_AI_TYPES];
// Name,AtkTyp1,2,3,Dmg1,2,3,Range1,2,3,Health,CybHealth,Percp,Disrp,Armr,Def,Movtyp,Yawspd,FOV,FOVAtk,FOVStartMov,DistToSeeBehind,SightRange,WalkSpd,RunSpd,AtkSpd1,2,3,AtkForce3,AtkRad3,TtPain,TbwPain,TtDead,TtActualAtk1,2,3,TbwAtk1,2,3,TEnemChg,TIdleSFXMin,TIdleSFXMax,TAtk1WaitMin,TAtk1WaitMax,TAtk1WaitChnc,TAtk2WaitMin,TAtk2WaitMax,TAtk2WaitChnc,TAtk3WaitMin,TAtk3WaitMax,TAtk3WaitChnc,ProjType1,2,3,ProjSpd1,2,3,HasLaser1,2,3,ExplodeOn3,PreActMeleCols,THunt,FlightHeight,FlightHeightIsPerc,SwitchMatOnDie,RangeHear,TTranq,Hops,NPCType,AtkProj1,2,3
NPCTable npcTable[NUM_AI_TYPES] = {
 { "AUTOBOMB"              ,0,0,1,  0,  0,200, 0,0,2.4,50,0,1,0.5,40,1,1,300,180,120,55,3.84,50,2.5,2.5,0,0,0,100,6,0,0,0.1,0,0,0,0,0,0,3,5,12,0.5,1,0.1,1,3,0.5,0,0,0,0,0,0,0,0,0,0,0,0,1,0,20,0,0,0,10,3,0,2,0,0,0 },
 { "CYBORG ASSASSIN"       ,0,4,7, 30, 50, 35, 3.3,10,20,65,0,2,0.6,5,4,1,180,180,80,15,3.2,50,2,2,0,0,0,0,0,0.45,5,2.083,0,0.25,0.2,0.91,0.91,1.58,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,0,3,0,0,0,0,0,60,0,0,0,10,3,0,3,0,0,489 },
 { "AVIAN MUTANT"          ,1,0,0, 40, 40,  0, 3.3,10,20,125,0,1,0.25,0,2,2,180,180,80,15,5.12,50,2,2,3.5,0,0,0,0,2,5,1,0.1,0,0,1,0,0,3,5,12,0.5,1,0.1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,60,0.65,1,0,10,3,0,1,0,0,0 },
 { "EXEC-BOT"              ,0,4,0, 30, 35,  0, 3.3,10,20,225,0,1,0.2,40,2,1,200,180,15,30,4.12,50,1.5,1.5,0,0,0,0,0,0.45,7,0.15,0,0.2,0,0,1.5,0,3,5,12,0,0,0,0.97,2,0.3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,180,0,0,0,10,3,0,2,0,0,0 },
 { "CYBORG DRONE"          ,0,4,0, 20, 20, 20, 3.3,25,50,60,0,1,0.3,0,2,1,65,180,80,15,3.2,50,1.6,2.2,0,0,0,0,0,0.542,15,0.958,0,0.1,0,0,1,0,3,20,45,0,0,0,1,2,0.5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,60,0,0,0,10,3,0,3,0,0,0 },
 { "CORTEX REAVER"         ,0,4,7, 80,325,125, 3.3,20,30,580,0,1,0.1,40,2,1,180,180,80,15,3.84,50,2,2,0,0,0,0,0,0.583,5,0.333,0,0.35244,0.324,0,1,1,3,15,30,0,0,0,0.2,1,0.5,8,15,1,0,0,0,0,0,10,0,0,0,0,0,600,0,0,0,10,3,0,2,0,0,372 },
 { "CYBORG WARRIOR"        ,0,4,7, 35, 35,150, 3.3,20,20,120,0,1,0.1,5,4,1,180,180,30,15,3.2,50,2.4,2.4,0,0,0,0,0,0.5,5,2.2,0,0.339,0.201,0,0.83,0.542,3,15,30,0,0,0,1,2,0.5,10,20,1,0,0,0,0,0,10,0,0,0,0,0,180,0,0,0,10,3,0,3,0,0,370 },
 { "CYBORG ENFORCER"       ,1,4,7, 60, 60, 80, 3.3,15,30,285,0,1,0.1,30,5,1,180,180,80,15,3.2,50,2.8,2.8,2.8,0,0.3,0,0,2,5,1.5,0.23471,0.393738,0.313266,0.958,0.958,0.958,5,15,30,0.1,0.3,0.1,0.1,0.5,0.5,10,25,1,0,0,0,0,0,10,0,0,0,0,0,600,0,0,0,10,3,0,4,0,0,387 },
 { "CYBORG ELITE GUARD"    ,1,7,4, 70, 75,  0, 3.3,10,50,380,0,1,0.05,50,6,1,180,180,80,15,3.2,50,3,3,1.5,0,0,0,0,0.4665,5,1.5,0.5,0.2653,0.117045,0.733,0.7,0.867,5,15,30,0.05,0.2,0.1,0.5,2,0.8,2,3,0.5,0,0,0,0,2,0,0,1,0,0,0,600,0,0,0,15,3,0,4,0,490,0 },
 { "CYBORG OF EDWARD DIEGO",1,7,0, 80, 95,  0, 3.3,40,50,900,0,2,0,55,6,1,180,180,80,15,3.2,50,2.8,2.8,0,0,0,0,0,0,0,0,0.28,0.363188,0.2,1.4,0.833,3,5,15,30,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,2.5,0,0,0,0,0,1,600,0,0,0,15,3,0,4,0,490,0 },
 { "SECURITY-1 ROBOT"      ,0,4,0, 35, 35,  0, 3.3,10,20,170,0,1,0.15,40,4,2,180,180,80,15,4.12,50,2.5,2.5,1.5,0,0,0,0,2,5,0.05,0.5,0.1,0.2,1.2,1.5,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,0,0,0,0,0,0,0,600,1.28,0,0,10,3,0,2,0,0,0 },
 { "SECURITY-2 ROBOT"      ,0,4,4, 65, 65, 15, 3.3,5,35,300,0,2,0.05,50,5,1,180,180,60,25,4.12,50,1.5,1.5,1.5,0,0,0,0,0.75,5,0.25,0.5,0.39,0.1,1.2,1,1.5,3,5,12,0.5,1,0.1,3,3.5,1,2.5,3.5,1,0,0,0,0,0,0,0,0,0,0,0,600,0,0,0,10,3,0,2,0,0,0 },
 { "MAINTENANCE ROBOT"     ,1,0,0, 25, 25,  0, 3.3,3.3,20,75,0,1,0.3,40,3,1,180,180,80,15,3.84,50,2.2,2.6,0.02,0.02,0,0,0,0,0,1.6,3,0.7,0.2,2,1.3,3,3,5,12,0.5,1,0.1,1,2,0.3,1,2,0.5,0,0,0,0,0,0,0,0,0,0,0,180,0,0,0,10,3,0,2,0,0,0 },
 { "MUTANT CYBORG"         ,1,7,0, 35, 75, 50, 2,30,49,340,0,1,0.2,15,6,1,180,180,60,15,3.2,50,1.5,1.5,0,0,0,0,0,0.583,3.5,3.41,0.265,0.285,0.2,0.625,0.75,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,2.8,0,0,0,0,0,0,180,0,0,0,10,3,0,5,0,491,0 },
 { "HOPPER"                ,0,4,0, 35, 35,  0, 0,17.92,17.92,150,0,1,0.25,35,4,1,180,160,80,15,3.84,50,7,7,0,0,0,0,0,0.708,5,0,0.5,0.1,0.5,0.5,0.5,0.5,3,5,12,0.5,1,0.1,0.5,1,0.5,1,2,0.5,0,0,0,0,0,0,0,1,0,0,0,180,0,0,0,10,3,1,2,0,0,0 },
 { "HUMANOID MUTANT"       ,1,0,0, 12, 12,  0, 3.3,10,20,50,0,0,0.4,0,3,1,60,180,80,15,2.56,50,1.4,2,0.5,0,0,0,0,0.42,5,0.967,0.5,0.1,0.2,1.2,1.5,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,0,0,0,0,0,0,0,20,0,0,0,10,3,0,0,0,0,0 },
 { "INVISIBLE MUTANT"      ,0,7,0, 10, 35,  0, 3.3,20,20,350,0,1,0.05,0,2,2,180,180,80,15,2.56,50,0.7,0.7,1.5,0.7,0.7,0,0,0.875,5,1.125,0.875,0.4,0.2,1.2,0.875,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,2,0,0,0,0,0,0,60,0.32,0,1,10,3,0,0,0,486,0 },
 { "VIRUS MUTANT"          ,0,7,0, 45, 30,  0, 3.3,20,20,140,0,0,0.1,0,3,1,180,180,80,15,2.56,50,2.5,2.5,2.5,0.3,0,0,0,0.542,3,1.792,0.2874,0.2874,0.2874,0.958,0.958,0.958,3,5,12,0.5,1,0.1,0.5,0,0.5,1,2,0.5,0,0,0,0,1.75,0,0,0,0,0,0,20,0,0,0,10,3,0,1,0,481,0 },
 { "SERV-BOT"              ,1,0,0,  8,  0,  0, 3.3,10,20,20,0,1,0.5,20,2,1,180,180,80,15,3.84,50,2,2,1.2,0,0,0,0,1.125,2,0.98,0.2,0.1,0.2,0.834,1.5,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,0,0,0,0,0,0,0,180,0,0,0,10,3,0,2,0,0,0 },
 { "FLIER BOT"             ,0,4,7, 30,150,  0, 3.3,35,40,75,0,1,0.3,30,2,2,180,180,80,15,5.12,50,1.5,1.5,1.5,0,0,0,0,1.375,5,0.6,0.1,0.1,0.2,1,1.5,3,3,5,12,0.5,1,0.1,1,2,0.5,10,12,1,0,0,0,0,0,10,0,0,0,0,0,180,0.85,1,0,10,3,0,2,0,0,404 },
 { "ZERO-G MUTANT"         ,0,7,0, 20, 20,  0, 3.3,20,20,90,0,1,0.5,0,2,2,180,180,80,15,2.56,50,0.8,1.4,0,0.8,0,0,0,0.1,0,0.1,0.5,0.05,0.2,1.2,1.5,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,2,0,0,0,0,0,0,60,1.96,0,0,10,3,0,0,0,488,0 },
 { "GORILLA TIGER MUTANT"  ,1,0,0, 60, 60,  0, 3.3,3.84,20,200,0,1,0.1,0,3,1,180,180,80,15,2.56,50,3,3.5,1,2,0,0,0,0.667,5,1.625,0.5,0.1,0.2,0.958,1.042,3,3,15,30,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,0,0,0,0,0,0,0,60,0,0,0,10,3,0,1,0,0,0 },
 { "REPAIR BOT"            ,0,4,0, 12, 12,  0, 3.3,3.3,20,65,0,1,0.4,25,3,1,180,180,80,15,3.84,50,2.25,3,0.5,0,0,0,0,0,0,0.05,0.2,0.1,0.2,1.25,1.5,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,0,0,0,0,0,0,0,180,0,0,0,10,3,0,2,0,0,0 },
 { "PLANT MUTANT"          ,0,7,0, 35, 25,  0, 3.3,20,20,115,0,1,0.3,0,1,1,180,180,80,15,2.56,50,0.8,1.2,0.1,0,0,0,0,0.375,2,2.208,0.89,0.82,0.2,1.91,1.027,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,3.5,0,0,0,0,0,0,20,0,0,0,10,3,0,0,0,487,0 },
 { "CYBER DOG"             ,0,7,0,  0, 25,  0, 0,20,0,0,20,1,0.5,0,1,4,250,240,50,15,20.48,25.6,2,2,0,0,0,0,0,0.1,0,0.5,0,0,0,0,0.3,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1.5,0,0,0,0,0,0,500,0.75,0,0,10,0,0,6,0,493,0 },
 { "CYBER GUARD"           ,0,7,0,  0, 25,  0, 0,20,0,0,35,1,0.4,0,1,4,250,240,50,15,20.48,25.6,2,2,0,0,0,0,0,0.1,0,0.5,0,0,0,0,0.2,0,2,998,999,0,0,0,0,0,0,0,0,0,0,0,0,0,0.8,0,0,0,0,0,0,500,0.75,0,0,10,0,0,6,0,493,0 },
 { "CYBER RAM"             ,0,7,0,  0, 35,  0, 0,20,0,0,40,1,0.25,0,1,4,80,240,50,15,20.48,25.6,4,4,0,0,0,0,0,0.1,0,0.5,0,0,0,0,0.2,0,2,998,999,0,0,0,0,0,0,0,0,0,0,0,0,0,1.2,0,0,0,0,0,0,500,0.75,0,0,10,0,0,6,0,494,0 },
 { "CYBER CORTEX REAVER"   ,0,7,0,  0, 45,  0, 0,20,0,0,80,1,0.1,0,1,4,80,240,50,15,20.48,25.6,4,4,0,0,0,0,0,0.1,0,0.5,0,0,0,0,0.2,0,2,998,999,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,500,0.75,0,0,10,0,0,6,0,494,0 },
 { "SHODAN"                ,0,7,0,  0, 55,  0, 0,20,0,0,500,2,0,0,1,4,360,280,280,15,20.48,25.6,0,0,0,0,0,0,0,0.1,0,0.5,0,0,0,0,0.05,0,2,998,999,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,500,0.75,0,0,10,0,0,6,0,494,0 }
};
//                             NPC Sounds       0,   1,   2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28
int sfxIdle[NUM_AI_TYPES] =           {  -1,  -1,  -1, -1, 58, -1, 59, -1, 59, 52, -1, -1, -1, -1, -1, -1,121, -1, -1, -1,121,118, -1, -1, -1, -1, -1, -1, -1};
int sfxSightSound[NUM_AI_TYPES] =     {  -1,  -1, 111,150, 58,150, 59,152,152, -1,150,150,151,152,150, -1,121, -1,151,150,121,119,151, -1, -1, -1, -1, -1, -1};
int sfxAttack1[NUM_AI_TYPES] =        {  -1,  -1, 108, -1, -1,146, -1,146,252,247, -1, -1, -1, -1, -1,122, -1,108,146, -1, -1,118, -1,125,258,258,258,258,258};
int sfxAttack2[NUM_AI_TYPES] =        {  -1, 256,  -1,148, 50, 50, 50, 50, 50,250, 50, 50,146,259,148, -1,121, -1, -1,147, -1, -1,146, -1,258,258,258,258,258};
int sfxAttack3[NUM_AI_TYPES] =        {  -1,  -1,  -1, -1, -1,244,244,244,245, -1, -1,149, -1, -1, -1, -1, -1, -1, -1,244, -1, -1, -1, -1,258,258,258,258,258};
int sfxDeath[NUM_AI_TYPES] =          {  -1,  48, 110,143, 48,145, 48, 51, 47, 47,142,143,144, 47,162,123,120,134,144,144,120,117,144,124, -1, -1, -1, -1, -1};
float deathBurstTimer[NUM_AI_TYPES] = {0.0f,0.0f, 0.1f,0.0f,0.1f,0.1f,0.2f,0.1f,0.1f,0.1f,0.0f,0.45f,0.75f,0.1f,0.0f,0.0f,0.1f,0.224f,0.9f,0.0f,0.1f,0.1f,0.1f,0.2f,0.1f,0.1f,0.1f,0.1f,0.1f};

void SetHuntFinished(uint16_t i) {
    uint16_t npcID = Eng_Global->instances[i].index - 419;
    Eng_Global->instances[i].huntFinished = Eng_Global->pauseRelativeTime;
    int diff = Eng_Global->difficultyCombat;
    if (npcTable[npcID].type == NPCType_Cyber) diff = Eng_Global->difficultyCyber;
    if (diff <= 1) { // More forgetful on easy.
        Eng_Global->instances[i].huntFinished += vmax((npcTable[npcID].huntTime * 0.75),60.0);
    } else if (diff >= 3) { // Good memory on hard.
        Eng_Global->instances[i].huntFinished += vmax((npcTable[npcID].huntTime * 2.00),60.0); 
    } else {
        Eng_Global->instances[i].huntFinished += vmax(npcTable[npcID].huntTime, 60.0);
    }
}

MOD_TO_ENGINE void InitializeAIAfterLoad(uint16_t i) {
    Entity* e = &Eng_Global->instances[i];
    e->layer = PhysicsLayer_NPC;
    uint16_t npcID = e->index - 419;
    e->idleTime = Eng_Global->pauseRelativeTime + (double)random_range(npcTable[npcID].timeIdleSFXMin,npcTable[npcID].timeIdleSFXMax);
    e->attack1SoundTime = e->attack2SoundTime = e->attack3SoundTime = Eng_Global->pauseRelativeTime;
    e->timeTillEnemyChangeFinished = Eng_Global->pauseRelativeTime;
    SetHuntFinished(i);
    e->attackFinished = Eng_Global->pauseRelativeTime;
    e->attack2Finished = Eng_Global->pauseRelativeTime;
    e->attack3Finished = Eng_Global->pauseRelativeTime;
    e->timeTillPainFinished = Eng_Global->pauseRelativeTime;
    e->timeTillDeadFinished = Eng_Global->pauseRelativeTime;
    e->meleeDamageFinished = Eng_Global->pauseRelativeTime;
    e->gracePeriodFinished = Eng_Global->pauseRelativeTime;
    e->randomWaitForNextAttack1Finished = Eng_Global->pauseRelativeTime;
    e->randomWaitForNextAttack2Finished = Eng_Global->pauseRelativeTime;
    e->randomWaitForNextAttack3Finished = Eng_Global->pauseRelativeTime;
    e->tranquilizeFinished = Eng_Global->pauseRelativeTime;
    e->deathBurstFinished = Eng_Global->pauseRelativeTime;
    e->wanderFinished = Eng_Global->pauseRelativeTime;
    e->posCheckFinished = Eng_Global->pauseRelativeTime;
    e->lastPosition = e->position;
    e->timeSinceMovedEnough = 0.0;
    if (e->walkWaypointsLength > 0 && (e->entflags & ENTFLAG_WALK_PATH_ON_START) && !(e->entflags & ENTFLAG_ASLEEP)) {
        e->currentDestination = e->walkWaypoints[e->currentWaypoint];
        e->currentState = AIState_Walk; // If waypoints are set, start walking
    } else {
        e->currentState = AIState_Idle; // No waypoints, stay put
    }

    if ((e->entflags & ENTFLAG_WANDERING) && (random_range(0.0f,1.0f) < 0.5f)) e->currentState = AIState_Walk;
    else flag_set(&e->entflags, ENTFLAG_WANDERING, false);

    if (e->entflags & ENTFLAG_ASLEEP) {
        e->currentState = AIState_Idle;
//         flag_set(&Eng_Global->instances[e->sleepingCables].entflags, ENTFLAG_ACTIVE, true); // TODO
    }

    e->attackFinished = Eng_Global->pauseRelativeTime + 1.0;
    e->idealTransformForward = e->forward;
    StringCopyInto_A_From_B(e->targetID,npcTable[npcID].name,TARGET_ID_LENGTH);
    StringFormat(e->targetID,TARGET_ID_LENGTH * sizeof(char),"%s %05u",npcTable[npcID].name,npcCountInWorldPerType[npcID]++);
    
    flag_set(&e->entflags,ENTFLAG_ANIM_DEAD_DONE,false);
    uint8_t c;
    switch (e->currentState) {
        case AIState_Walk:    c = ANIM_WALK;    break;
        case AIState_Run:     c = ANIM_RUN;     break;
        case AIState_Attack1: c = ANIM_ATTACK1; break;
        case AIState_Attack2: c = ANIM_ATTACK2; break;
        case AIState_Attack3: c = ANIM_ATTACK3; break;
        case AIState_Pain:    c = ANIM_PAIN;    break;
        case AIState_Dying:
        case AIState_Dead:    c = ANIM_DYING;   break;
        default:              c = ANIM_IDLE;    break;
    }
    
    e->clip = c;
    e->frame = modelAnimationClips[e->animationNum][c].frameStart;
    e->currentFrameFinished = 0.0;
}
    
float Tranquilize(uint16_t i, float amount, bool energy) {
    uint16_t npcID = Eng_Global->instances[i].index - 419;
    if (npcTable[npcID].type == NPCType_Robot && !energy) return 0.0f;

    float tranqSecs = (amount < 3.0f) ? npcTable[npcID].timeForTranquilization : amount; // If we're going to tranq, at least do it for 3 secs.
    Eng_Global->instances[i].tranquilizeFinished = vmax(Eng_Global->pauseRelativeTime + tranqSecs, Eng_Global->instances[i].tranquilizeFinished + tranqSecs);
    return tranqSecs;
}

static inline __attribute__((always_inline)) bool IsCyberNPC(uint16_t i) { uint16_t npcID = Eng_Global->instances[i].index - 419; return npcTable[npcID].type == NPCType_Cyber; }

bool HasHealth(uint16_t i) {
    if (IsCyberNPC(i)) return (Eng_Global->instances[i].cyberHealth > 0.0f);
    return (Eng_Global->instances[i].health > 0.0f);
}

static inline bool     ai_is_cyber(Entity* e)  { return npcTable[e->index - 419].type == NPCType_Cyber; }
static inline bool     ai_has_health(Entity* e){ return ai_is_cyber(e) ? e->cyberHealth > 0.0f : e->health > 0.0f; }
static inline Vector3  ai_sight_pos(Entity* e) { return Vector3_A_plus_B(e->position, e->sightPointOffset); }
static inline uint16_t ai_self_idx(Entity* e)  { return (uint16_t)(e - Eng_Global->instances); }

static inline Vector3 ai_gun_pos(Entity* e, int n) {
    Vector3 off = (n == 3) ? e->gunPointOffset2 : e->gunPointOffset;
    if (n == 2 && off.x == 0.0f && off.y == 0.0f && off.z == 0.0f) off = e->gunPointOffset2;
    return Vector3_A_plus_B(e->position, off);
}

static Quaternion quat_look_rotation(Vector3 fwd, Vector3 up) {
    fwd = normalize_vector3(fwd);
    Vector3 r = normalize_vector3(cross_vector3(up, fwd));
    up = cross_vector3(fwd, r);
    float m00=r.x, m01=r.y, m02=r.z, m10=up.x, m11=up.y, m12=up.z, m20=fwd.x, m21=fwd.y, m22=fwd.z;
    float tr = m00 + m11 + m22;
    Quaternion q;
    if (tr > 0.0f) {
        float s = 0.5f / vsqrtf(tr + 1.0f);
        q.w=(0.25f/s); q.x=(m12-m21)*s; q.y=(m20-m02)*s; q.z=(m01-m10)*s;
    } else if (m00 > m11 && m00 > m22) {
        float s = 2.0f * vsqrtf(1.0f + m00 - m11 - m22);
        q.w=(m12-m21)/s; q.x=0.25f*s; q.y=(m01+m10)/s; q.z=(m20+m02)/s;
    } else if (m11 > m22) {
        float s = 2.0f * vsqrtf(1.0f + m11 - m00 - m22);
        q.w=(m20-m02)/s; q.x=(m01+m10)/s; q.y=0.25f*s; q.z=(m12+m21)/s;
    } else {
        float s = 2.0f * vsqrtf(1.0f + m22 - m00 - m11);
        q.w=(m01-m10)/s; q.x=(m20+m02)/s; q.y=(m12+m21)/s; q.z=0.25f*s;
    }
    return q;
}

static Quaternion quat_slerp(Quaternion a, Quaternion b, float t) {
    float d = quat_dot(a, b);
    if (d < 0.0f) { b.x=-b.x; b.y=-b.y; b.z=-b.z; b.w=-b.w; d=-d; }
    if (d > 0.9995f) {
        Quaternion r = { a.x+t*(b.x-a.x), a.y+t*(b.y-a.y), a.z+t*(b.z-a.z), a.w+t*(b.w-a.w) };
        float il = 1.0f / vsqrtf(r.x*r.x + r.y*r.y + r.z*r.z + r.w*r.w);
        r.x*=il; r.y*=il; r.z*=il; r.w*=il;
        return r;
    }
    d = vclamp(d, -1.0f, 1.0f);
    float th0 = vacosf(d), th = th0*t, sth0 = vsinf(th0);
    float s0 = vsinf(th0 - th) / sth0, s1 = vsinf(th) / sth0;
    return (Quaternion){ s0*a.x+s1*b.x, s0*a.y+s1*b.y, s0*a.z+s1*b.z, s0*a.w+s1*b.w };
}

static float quat_angle_deg(Quaternion a, Quaternion b) {
    float d = vclamp(vabs(quat_dot(a, b)), 0.0f, 1.0f);
    return 2.0f * vacosf(d) * (180.0f / PI);
}

static void aiac_set_clip(Entity* self, uint8_t c) {
    if (self->clip == c) return;
    self->clip  = c;
    self->frame = modelAnimationClips[self->animationNum][c].frameStart;
    self->currentFrameFinished = 0.0;
}

static void aiac_freeze(Entity* self) {
    self->currentFrameFinished = Eng_Global->current_time + 1e9;
}

static void aiac_idle(Entity* self) {
    if ((self->entflags & ENTFLAG_ASLEEP) || self->tranquilizeFinished >= Eng_Global->current_time) {
        aiac_freeze(self); return;
    }
    aiac_set_clip(self, ANIM_IDLE);
}

static void aiac_walk(Entity* self) {
    if (self->entflags & ENTFLAG_ACT_AS_TURRET) { aiac_idle(self); return; }
    
    float spdsq = self->velocity.x*self->velocity.x + self->velocity.z*self->velocity.z;
    if (spdsq > MIN_WALK_SPEED_SQ) { aiac_set_clip(self, ANIM_WALK); return; }
    
    if (self->animSwapFinished < Eng_Global->current_time) {
        self->animSwapFinished = Eng_Global->current_time + ANIM_WALK_SWAP_DELAY;
        aiac_set_clip(self, ANIM_IDLE);
    }
}

static void aiac_run(Entity* self) {
    if (self->entflags & ENTFLAG_ACT_AS_TURRET) { aiac_idle(self); return; }
    
    aiac_set_clip(self, ANIM_RUN);
}

static void aiac_dying(Entity* self) {
    flag_set(&self->entflags, ENTFLAG_ASLEEP, false);
    if (self->entflags & ENTFLAG_NO_DYING_ANIM) return;
    
    aiac_set_clip(self, ANIM_DYING);
    AnimationClip cl = modelAnimationClips[self->animationNum][ANIM_DYING];
    uint16_t range = cl.frameEnd > cl.frameStart ? cl.frameEnd - cl.frameStart : 1;
    self->animatorPlaybackTime = (float)(self->frame - cl.frameStart) / (float)range;
    if (self->animatorPlaybackTime > 0.99f) flag_set(&self->entflags,ENTFLAG_DYING,false);
}

static void aiac_dead(Entity* self) {
    if (self->entflags & ENTFLAG_NO_DEATH_FREEZE) { aiac_freeze(self); return; }
    
    if (!(self->entflags & ENTFLAG_ANIM_DEAD_DONE)) {
        AnimationClip cl = modelAnimationClips[self->animationNum][ANIM_DYING];
        self->clip       = ANIM_DYING;
        self->frame      = cl.frameEnd;
        self->modelIndex = cl.frameStartModelIndex + (cl.frameEnd - cl.frameStart);
        flag_set(&self->entflags, ENTFLAG_ANIM_DEAD_DONE, true);
    }
    aiac_freeze(self);
}

void AIAnimationControllerUpdate(uint16_t idx) {
    Entity* self = &Eng_Global->instances[idx];
    if (!(self->entflags & ENTFLAG_ACTIVE))        return;
    if (!(self->entflags & ENTFLAG_ANIMATED))       return;
    if (!(self->entflags & ENTFLAG_VISIBLE))        return;
    if (self->animationNum >= MAX_ANIMATED_MODELS)  return;
    if (self->currentState == AIState_Dying) { aiac_dying(self); return; }
    if (self->currentState == AIState_Dead)  { aiac_dead(self);  return; }
    if (self->entflags & ENTFLAG_ASLEEP)     { aiac_idle(self);  return; }
    if (self->currentState == AIState_Run && self->tranquilizeFinished >= Eng_Global->current_time) { aiac_idle(self); return; }
    
    switch (self->currentState) {
        case AIState_Idle:    aiac_idle(self);                   break;
        case AIState_Walk:    aiac_walk(self);                   break;
        case AIState_Run:     aiac_run(self);                    break;
        case AIState_Attack1: aiac_set_clip(self, ANIM_ATTACK1); break;
        case AIState_Attack2: aiac_set_clip(self, ANIM_ATTACK2); break;
        case AIState_Attack3: aiac_set_clip(self, ANIM_ATTACK3); break;
        case AIState_Pain:    aiac_set_clip(self, ANIM_PAIN);    break;
        default:              aiac_idle(self);                   break;
    }
}

static bool AICheckIfEnemyInSight(Entity* self) {
    uint16_t eidx = self->enemey;
    if (!eidx || !ai_has_health(self)) return false;
    Entity* en = &Eng_Global->instances[eidx];
    bool enIsNPC = (en->layer & PhysicsLayer_NPC) != 0;
    int diff = ai_is_cyber(self) ? Eng_Global->difficultyCyber : Eng_Global->difficultyCombat;
    if (!ai_is_cyber(self) && !enIsNPC && !PositionVisibleFromPlayerCell(self->position.x, self->position.z)) return false;
    if (diff == 0 && (self->index - 419) != 28) return false;
    if (Eng_Cheats->notarget && !enIsNPC) {
        self->enemey = 0;
        self->posCheckFinished = Eng_Global->pauseRelativeTime + AI_POS_CHECK_DELAY;
        self->lastPosition = self->position;
        flag_set(&self->entflags, ENTFLAG_ENEM_IN_LOS, false);
        return false;
    }
    if (ai_is_cyber(self) && Eng_Global->decoyActive) { flag_set(&self->entflags, ENTFLAG_ENEM_IN_LOS, false); return false; }

    float dist = distance_vector3(en->position, ai_sight_pos(self));
    if (dist > npcTable[self->index - 419].sightRange) return false;
    if (ai_is_cyber(self) || enIsNPC) return true;

    Vector3 spos = ai_sight_pos(self);
    Vector3 lineN = normalize_vector3(Vector3_A_minus_B(en->position, spos));
    RaycastHit hit = Raycast(spos, lineN, npcTable[self->index - 419].sightRange, LAYER_MASK_NPC_SIGHT);
    if (hit.hit) {
        if (hit.hitInstanceIndex == eidx) { flag_set(&self->entflags, ENTFLAG_ENEM_IN_LOS, true); return true; }
        // Smart NPCs try to open doors blocking line-of-sight
        NPCType t = npcTable[self->index - 419].type;
        if (t != NPCType_Mutant && t != NPCType_Supermutant && t != NPCType_Cyber) {
            uint16_t hi = hit.hitInstanceIndex;
            if (hi && dist_sq_vector3(hit.point, spos) < 4.0f && ConstIndexIsDoor(Eng_Global->instances[hi].index)) {
                Entity* dr = &Eng_Global->instances[hi];
                if ((dr->doorOpen == DoorState_Closed || (dr->doorOpen == DoorState_Closing && Eng_Global->difficultyCombat > 2))
                    && !(dr->entflags & ENTFLAG_LOCKED)
                    && GetCurrentLevelSecurity() <= dr->securityThreshold
                    && (dr->requiredAccessCard == AccessCardType_None || dr->accessCardUsedByPlayer)) {
                    DoorActuate(hi);
                }
            }
        }
    }
    flag_set(&self->entflags, ENTFLAG_ENEM_IN_LOS, false);
    return false;
}

static void AISetHuntFinished(Entity* self) {
    self->huntFinished = Eng_Global->pauseRelativeTime;
    int diff = ai_is_cyber(self) ? Eng_Global->difficultyCyber : Eng_Global->difficultyCombat;
    double ht = npcTable[self->index - 419].huntTime;
    double mn = 60.0;
    if      (diff <= 1) self->huntFinished += (ht * 0.75 > mn ? ht * 0.75 : mn);
    else if (diff >= 3) self->huntFinished += (ht * 2.0  > mn ? ht * 2.0  : mn);
    else                self->huntFinished += (ht         > mn ? ht        : mn);
}

static void AISetEnemy(Entity* self, uint16_t eidx) {
    if (!eidx) return;
    
    self->enemey = eidx;
    self->posCheckFinished = Eng_Global->pauseRelativeTime + AI_POS_CHECK_DELAY;
    flag_set(&self->entflags, ENTFLAG_WANDERING, false);
    self->wanderFinished  = Eng_Global->pauseRelativeTime;
    self->lastPosition    = self->position;
    Entity* en            = &Eng_Global->instances[eidx];
    self->lastKnownEnemyPos  = en->position;
    self->targettingPosition = (Vector3){ en->position.x, en->position.y + AI_TARGET_OFFSET_Y, en->position.z };
    AISetHuntFinished(self);
}

static void AIPlaySightSound(Entity* self) {
    if (!(self->entflags & ENTFLAG_FIRST_SIGHTING)) return;
    if (!ai_has_health(self)) return;
    if (self->entflags & ENTFLAG_ACT_AS_CORPSE_ONLY) return;
    
    flag_set(&self->entflags, ENTFLAG_FIRST_SIGHTING, false);
    int16_t sfx = sfxSightSound[self->index - 419];
    if (sfx >= 0 && sfx < (int16_t)SOUNDS_COUNT) play_wav(sounds[sfx], self->volume, self->position, true);
}

static bool AICheckIfPlayerInSight(Entity* self) {
    int diff = ai_is_cyber(self) ? Eng_Global->difficultyCyber : Eng_Global->difficultyCombat;
    if (!ai_is_cyber(self) && !PositionVisibleFromPlayerCell(self->position.x, self->position.z)) return false;
    if (diff == 0 && (self->index - 419) != 28) return false;
    if (self->enemey) return AICheckIfEnemyInSight(self);

    flag_set(&self->entflags, ENTFLAG_ENEM_IN_LOS, false);
    if (ai_is_cyber(self) && Eng_Global->decoyActive) return false;
    if (Eng_Cheats->notarget) return false;

    Vector3 playerPos = Eng_Global->instances[PLAYER1].position;
    Vector3 spos      = ai_sight_pos(self);
    float dist = distance_vector3(playerPos, spos);
    NPCTable* npc = &npcTable[self->index - 419];
    if (dist > npc->sightRange) return false;

    if (ai_is_cyber(self)) { AISetEnemy(self, PLAYER1); AIPlaySightSound(self); return true; }

    Vector3 checkN = normalize_vector3(Vector3_A_minus_B(playerPos, spos));
    float cosA = vclamp(dot_vector3(checkN, self->forward), -1.0f, 1.0f);
    float angle = vacosf(cosA) * (180.0f / PI);
    bool makingNoise = (Eng_Global->instances[PLAYER1].entflags & ENTFLAG_MAKING_NOISE) != 0;

    if (angle < npc->fov * 0.5f) {
        RaycastHit hit = Raycast(spos, checkN, dist + 0.1f, LAYER_MASK_NPC_SIGHT);
        if (hit.hit && hit.hitInstanceIndex == PLAYER1) { flag_set(&self->entflags, ENTFLAG_ENEM_IN_LOS, true); AISetEnemy(self, PLAYER1); AIPlaySightSound(self); return true; }
        if (!hit.hit && makingNoise && dist < npc->hearingRange) { AISetEnemy(self, PLAYER1); AIPlaySightSound(self); return true; }
    } else {
        if (dist < npc->distToSeeBehind) {
            RaycastHit hit = Raycast(spos, checkN, dist + 0.1f, LAYER_MASK_NPC_SIGHT);
            if (hit.hit && hit.hitInstanceIndex == PLAYER1) { flag_set(&self->entflags, ENTFLAG_ENEM_IN_LOS, true); AISetEnemy(self, PLAYER1); AIPlaySightSound(self); return true; }
        }
        if (makingNoise && dist < npc->hearingRange) { AISetEnemy(self, PLAYER1); AIPlaySightSound(self); return true; }
    }
    return false;
}

static void AIEnemyInFrontChecks(Entity* self, uint16_t eidx) {
    if (!eidx) { flag_set(&self->entflags, ENTFLAG_ENEM_IN_FRONT|ENTFLAG_ENEM_IN_FOV, false); return; }
    if (ai_is_cyber(self)) { flag_set(&self->entflags, ENTFLAG_ENEM_IN_FRONT|ENTFLAG_ENEM_IN_FOV, true); return; }
    Vector3 spos = ai_sight_pos(self);
    Vector3 epos = Eng_Global->instances[eidx].position;
    Vector3 iv   = normalize_vector3((Vector3){epos.x - spos.x, 0.0f, epos.z - spos.z});
    float d = dot_vector3(iv, self->forward);
    flag_set(&self->entflags, ENTFLAG_ENEM_IN_FOV,   d > 0.800f);
    flag_set(&self->entflags, ENTFLAG_ENEM_IN_FRONT, d > 0.300f);
}

static void AIFace(Entity* self, Vector3 goal) {
    if (self->entflags & ENTFLAG_ASLEEP) return;
    Vector3 fv = Vector3_A_minus_B(goal, self->position);
    if (!ai_is_cyber(self)) fv.y = 0.0f;
    if (fv.x == 0.0f && fv.y == 0.0f && fv.z == 0.0f) return;

    uint16_t eidx = self->enemey;
    if (ai_is_cyber(self) && eidx) { self->rotation = Eng_Global->instances[eidx].rotation; return; }

    if (fv.x == 0.0f && fv.z == 0.0f) {
        if (eidx) fv = Vector3_A_minus_B(Eng_Global->instances[eidx].position, self->position);
        else fv.x += 0.001f;
    }

    Quaternion lr = quat_look_rotation(fv, (Vector3){0.0f, 1.0f, 0.0f});
    float t = (float)(AI_TICK_TIME * npcTable[self->index - 419].yawSpeed * Eng_Global->deltaTime);
    self->rotation = quat_slerp(self->rotation, lr, t);
}

static bool AIWithinAngleToTarget(Entity* self) {
    if (ai_is_cyber(self)) return true;
    if (dot_vector3(self->idealTransformForward, self->idealTransformForward) <= 1e-6f) return false;
    Quaternion lr  = quat_look_rotation(self->idealTransformForward, (Vector3){0.0f, 1.0f, 0.0f});
    float ang      = quat_angle_deg(self->rotation, lr);
    float fovMov   = npcTable[self->index - 419].fovStartMovement;
    if (ang < fovMov) return true;
    if (ang < fovMov * 1.5f && random_range(0.0f, 1.0f) < 0.5f) return true;
    return false;
}

bool AICheckPain(Entity* self) {
    if (ai_is_cyber(self)) return false;
    if (self->entflags & ENTFLAG_ASLEEP) return false;
    if (npcTable[self->index - 419].timeBetweenPain <= 0.0f) return false;
    if (!(self->entflags & ENTFLAG_GO_INTO_PAIN) || self->timeTillPainFinished >= Eng_Global->pauseRelativeTime) return false;

    self->currentState = AIState_Pain;
    uint16_t atkIdx = self->recentMostActivator;
    if (atkIdx && self->timeTillEnemyChangeFinished < Eng_Global->pauseRelativeTime) {
        self->timeTillEnemyChangeFinished = Eng_Global->pauseRelativeTime + npcTable[self->index - 419].timeToChangeEnemy;
        Entity* atk = &Eng_Global->instances[atkIdx];
        bool atkIsPlayer = (atk->layer & PhysicsLayer_Player) != 0;
        if (!atkIsPlayer && ConstIndexIsNPC(atk->index)) {
            NPCType mt = npcTable[self->index - 419].type, at = npcTable[atk->index - 419].type;
            bool canFight = false;
                 if (mt == NPCType_Robot && self->enemey) canFight = false;
            else if ((mt == NPCType_Cyborg || mt == NPCType_Supercyborg || mt == NPCType_Robot) && (at == NPCType_Cyborg || at == NPCType_Supercyborg || at == NPCType_Robot))  canFight = false;
            else if ((mt == NPCType_Mutant || mt == NPCType_Supermutant) && (at == NPCType_Mutant || at == NPCType_Supermutant)) canFight = atk->index != self->index;
            else canFight = atk->index != self->index;
            
            if (canFight) self->enemey = atkIdx;
        } else {
            self->enemey = atkIdx;
        }
        
        self->posCheckFinished = Eng_Global->pauseRelativeTime + AI_POS_CHECK_DELAY;
        flag_set(&self->entflags, ENTFLAG_WANDERING, false);
        self->wanderFinished = Eng_Global->pauseRelativeTime;
        self->lastPosition   = self->position;
        if (self->enemey) {
            Entity* en = &Eng_Global->instances[self->enemey];
            self->lastKnownEnemyPos  = en->position;
            self->currentDestination = en->position;
        }
    }
    
    flag_set(&self->entflags, ENTFLAG_GO_INTO_PAIN, false);
    self->timeTillPainFinished = Eng_Global->pauseRelativeTime + npcTable[self->index - 419].timeToPain;
    return true;
}

static void AIIdle(Entity* self) {
    if (self->enemey && ai_has_health(self)) { self->currentState = AIState_Run; return; }

    NPCTable* npc = &npcTable[self->index - 419];
    if (self->idleTime < Eng_Global->pauseRelativeTime) {
        int sidle = sfxIdle[self->index - 419];
        if (random_range(0.0f, 1.0f) < 0.5f && sidle >= 0 && sidle < (int16_t)SOUNDS_COUNT) play_wav(sounds[sidle],self->volume,self->position,true);
        self->idleTime = Eng_Global->pauseRelativeTime + random_range(npc->timeIdleSFXMin, npc->timeIdleSFXMax);
    }

    if (self->entflags & ENTFLAG_ASLEEP) { flag_set(&self->entflags, ENTFLAG_KINEMATIC, true); self->velocity = (Vector3){0,0,0}; }
    AICheckPain(self);
}

static Vector3 AIGetWanderPoint(Entity* self) {
    return (Vector3){
        self->position.x + random_range(-AI_WANDER_RANGE, AI_WANDER_RANGE),
        ai_is_cyber(self) ? self->position.y + random_range(-AI_WANDER_RANGE, AI_WANDER_RANGE) : 0.0f,
        self->position.z + random_range(-AI_WANDER_RANGE, AI_WANDER_RANGE)
    };
}

static Vector3 AIGetAStarPoint(Entity* self) {
    Vector3 ep = self->enemey ? Eng_Global->instances[self->enemey].position : self->position;
    float px = self->position.x, py = self->position.y, pz = self->position.z;
    Vector3 cands[4] = {
        {px, py, pz + AI_ASTAR_STEP}, {px, py, pz - AI_ASTAR_STEP},
        {px + AI_ASTAR_STEP, py, pz}, {px - AI_ASTAR_STEP, py, pz}
    };
    int best = -1; float bestD = 1e9f;
    for (int i = 0; i < 4; ++i) {
        if (!PositionVisibleFromPlayerCell(cands[i].x, cands[i].z)) continue;
        float d = dist_sq_vector3(ep, cands[i]);
        if (d < bestD) { bestD = d; best = i; }
    }
    return best >= 0 ? cands[best] : AIGetWanderPoint(self);
}

static Vector3 AIGetSearchPoint(Entity* self) {
    NPCType t = npcTable[self->index - 419].type;
    if (t == NPCType_Mutant || t == NPCType_Supermutant) return AIGetWanderPoint(self);
    return AIGetAStarPoint(self);
}

static void AIHopMove(Entity* self) {
    if (self->entflags & ENTFLAG_ACT_AS_TURRET) return;
    
    if (self->animatorPlaybackTime > 0.1395f) {
        if (!(self->entflags & ENTFLAG_HOP_DONE)) {
            flag_set(&self->entflags, ENTFLAG_HOP_DONE, true);
            AddForce(ai_self_idx(self), scale_vector3(self->forward, 500.0f), true);
            AddForce(ai_self_idx(self), (Vector3){0, 5.0f, 0}, true);
        }
    } else {
        flag_set(&self->entflags, ENTFLAG_HOP_DONE, false);
    }
}

static void AIWalk(Entity* self) {
    if (AICheckPain(self)) return;
    if (self->entflags & ENTFLAG_ASLEEP) return;
    if ((self->entflags & ENTFLAG_ENEM_IN_SIGHT) || self->enemey) { self->currentState = AIState_Run; return; }
    if (self->entflags & ENTFLAG_ACT_AS_TURRET) { self->currentState = AIState_Idle; return; }
    if (npcTable[self->index - 419].moveType == AIMoveType_None) return;
    if (self->tranquilizeFinished >= Eng_Global->pauseRelativeTime) return;
    if (!PositionVisibleFromPlayerCell(self->position.x, self->position.z)) return;

    float dist = distance_vector3(ai_sight_pos(self), self->currentDestination);
    if (self->entflags & ENTFLAG_WANDERING) {
        if (self->wanderFinished < Eng_Global->pauseRelativeTime || dist < AI_STOP_DIST * 0.5f) {
            self->wanderFinished = Eng_Global->pauseRelativeTime + random_range(3.0f, 8.0f);
            self->currentDestination = AIGetWanderPoint(self);
        }
    }

    if (dist > AI_STOP_DIST && AIWithinAngleToTarget(self)) {
        if (npcTable[self->index - 419].hopsOnMove) {
            AIHopMove(self);
        } else {
            float ws  = npcTable[self->index - 419].walkSpeed;
            Vector3 mv = { self->forward.x*ws, self->forward.y*ws, self->forward.z*ws };
            if (npcTable[self->index - 419].moveType != AIMoveType_Fly) {
                Vector3 spos = ai_sight_pos(self);
                Vector3 cp = { spos.x + self->forward.x*0.48f, spos.y, spos.z + self->forward.z*0.48f };
                RaycastHit gh = Raycast(cp, (Vector3){0,-1,0}, 2.56f, LAYER_MASK_NPC_COLLISION);
                if (!gh.hit) { mv.x = 0.0f; mv.z = 0.0f; }
            }
            mv.y = self->velocity.y;
            self->velocity = mv;
        }
        
        return;
    }

    if (self->walkWaypointsLength < 1) {
        if (!(self->entflags & ENTFLAG_WANDERING)) self->currentState = AIState_Idle;
        return;
    }

    if (self->entflags & ENTFLAG_VISIT_WAYPTS_RND) {
        self->currentWaypoint = random_range_i32(0, self->walkWaypointsLength);
    } else {
        self->currentWaypoint++;
    }
    if (self->currentWaypoint < 0) self->currentWaypoint = 0;
    if (self->currentWaypoint >= (self->walkWaypointsLength - 1)) {
        self->currentWaypoint = 0;
        if (self->entflags & ENTFLAG_DONT_LOOP_WAYPTS) { self->currentState = AIState_Idle; return; }
    }
    if (self->currentWaypoint < self->walkWaypointsLength)
        self->currentDestination = self->walkWaypoints[self->currentWaypoint];
}

static void AIRunMove(Entity* self) {
    if (self->entflags & ENTFLAG_ACT_AS_TURRET) return;
    
    float rs = npcTable[self->index - 419].runSpeed;
    self->velocity = (Vector3){
        self->forward.x * rs,
        (vabs(self->gravity) > 0.05f) ? self->velocity.y : self->forward.y * rs,
        self->forward.z * rs
    };
}

static void AIHunt(Entity* self) {
    uint16_t eidx = self->enemey;
    if (!eidx) return;
    self->currentDestination = ai_is_cyber(self) ? Eng_Global->instances[eidx].position : AIGetSearchPoint(self);
    if (npcTable[self->index - 419].moveType == AIMoveType_None) return;
    if (self->entflags & ENTFLAG_ACT_AS_TURRET) return;
    if (npcTable[self->index - 419].runSpeed <= 0.0f) return;
    if (dist_sq_vector3(ai_sight_pos(self), self->currentDestination) <= AI_STOP_DIST_SQ) return;
    if (!AIWithinAngleToTarget(self)) return;
    float rs = npcTable[self->index - 419].runSpeed;
    self->velocity = (Vector3){ self->forward.x*rs, self->velocity.y, self->forward.z*rs };
}

static bool AICanAttack1(Entity* self, float dsq) {
    if (self->rangeToEnemy >= dsq) return false;
    if (npcTable[self->index - 419].attackType == AttackType_None) return false;
    if (ai_is_cyber(self)) return true;
    if (!(self->entflags & ENTFLAG_ENEM_IN_FRONT)) return false;
    return self->randomWaitForNextAttack1Finished < Eng_Global->pauseRelativeTime;
}
static bool AICanAttack2(Entity* self, float dsq) {
    if (self->rangeToEnemy >= dsq) return false;
    if (npcTable[self->index - 419].attackType2 == AttackType_None) return false;
    if (ai_is_cyber(self)) return true;
    if (!(self->entflags & ENTFLAG_ENEM_IN_FRONT)) return false;
    if (!(self->entflags & ENTFLAG_ENEM_IN_FOV))   return false;
    return self->randomWaitForNextAttack2Finished < Eng_Global->pauseRelativeTime;
}
static bool AICanAttack3(Entity* self, float dsq) {
    if (self->rangeToEnemy >= dsq) return false;
    NPCTable* npc = &npcTable[self->index - 419];
    if (npc->attackType3 == AttackType_None) return false;
    if (self->rangeToEnemy < 7.0f && npc->attackType3 == AttackType_ProjectileLaunched) {
        int p = npc->projectile3Prefab;
        if (p == 370 || p == 372 || p == 387 || p == 404) return false;
    }
    if (ai_is_cyber(self)) return true;
    if (!(self->entflags & ENTFLAG_ENEM_IN_FRONT)) return false;
    if (!(self->entflags & ENTFLAG_ENEM_IN_FOV))   return false;
    return self->randomWaitForNextAttack3Finished < Eng_Global->pauseRelativeTime;
}

static void AIBrakingMovement(Entity* self) {
    uint8_t ni = self->index - 419;
    if (ni == 1 || (ni >= 3 && ni <= 9) || (ni >= 11 && ni <= 13) || ni == 17 || ni == 23) {
        self->velocity.x *= 0.15f;
        self->velocity.z *= 0.15f;
    }
}

static void AIStartAttack(Entity* self, int n) {
    AIBrakingMovement(self);
    NPCTable* npc = &npcTable[self->index - 419];
    double between, toActual;
    switch (n) {
        case 1: between = npc->timeBetweenAttack1; toActual = npc->timeToActualAttack1; break;
        case 2: between = npc->timeBetweenAttack2; toActual = npc->timeToActualAttack2; break;
        default: between = npc->timeBetweenAttack3; toActual = npc->timeToActualAttack3; break;
    }
    self->attackFinished    = Eng_Global->pauseRelativeTime + between + toActual;
    self->gracePeriodFinished = Eng_Global->pauseRelativeTime + toActual;
    self->currentState = (AIState)(AIState_Attack1 + (n - 1));
}

static void AIRun(Entity* self) {
    if (AICheckPain(self)) return;
    if (self->entflags & ENTFLAG_ASLEEP) return;
    if (!self->enemey) { self->currentState = AIState_Idle; return; }
    if (self->tranquilizeFinished >= Eng_Global->pauseRelativeTime && !ai_is_cyber(self)) return;

    if (self->posCheckFinished <= Eng_Global->pauseRelativeTime && !ai_is_cyber(self)) {
        self->posCheckFinished = Eng_Global->pauseRelativeTime + AI_POS_CHECK_DELAY;
        float dToEn   = distance_vector3(ai_sight_pos(self), Eng_Global->instances[self->enemey].position);
        float dToLast = distance_vector3(self->position, self->lastPosition);
        self->lastPosition = self->position;
        if (dToLast < 0.48f && dToEn > AI_STOP_DIST && !(self->entflags & ENTFLAG_WANDERING)) {
            self->wanderFinished = Eng_Global->pauseRelativeTime + AI_SEARCH_TIME;
            flag_set(&self->entflags, ENTFLAG_WANDERING, true);
            self->currentDestination = AIGetSearchPoint(self);
        } else {
            flag_set(&self->entflags, ENTFLAG_WANDERING, false);
        }
    }

    if (!(self->entflags & ENTFLAG_ENEM_IN_SIGHT)) {
        if (self->huntFinished > Eng_Global->pauseRelativeTime) { AIHunt(self); }
        else { self->enemey = 0; flag_set(&self->entflags,ENTFLAG_WANDERING,true); self->wanderFinished = Eng_Global->pauseRelativeTime + 1.0; self->currentState = AIState_Walk; }
        return;
    }

    if (self->enemey && !(self->entflags & ENTFLAG_WANDERING)) {
        Entity* en = &Eng_Global->instances[self->enemey];
        self->targettingPosition = (Vector3){en->position.x,en->position.y + AI_TARGET_OFFSET_Y,en->position.z};
        self->currentDestination = self->targettingPosition;
        self->lastKnownEnemyPos  = self->targettingPosition;
    }

    flag_set(&self->entflags, ENTFLAG_SHOT_FIRED, false);
    AISetHuntFinished(self);

    float nr = npcTable[self->index - 419].range,  near = nr * nr;
    float mr = npcTable[self->index - 419].range2, mid  = mr * mr;
    float fr = npcTable[self->index - 419].range3, far  = fr * fr;

    if (AICanAttack1(self, near)) { AIStartAttack(self, 1); return; }
    if (AICanAttack2(self, mid))  { AIStartAttack(self, 2); return; }
    if (AICanAttack3(self, far))  { AIStartAttack(self, 3); return; }

    if (npcTable[self->index - 419].moveType != AIMoveType_None && self->rangeToEnemy > AI_STOP_DIST_SQ) {
        if (AIWithinAngleToTarget(self)) {
            if (npcTable[self->index - 419].hopsOnMove) AIHopMove(self);
            else AIRunMove(self);
        } else if (Eng_Global->difficultyCombat >= 2 && random_range(0.0f, 1.0f) < 0.5f) {
            AIFace(self, self->currentDestination);
        }
    }
}

static void AIPain(Entity* self) {
    if (self->timeTillPainFinished < Eng_Global->pauseRelativeTime) {
        self->currentState = AIState_Run;
        flag_set(&self->entflags, ENTFLAG_GO_INTO_PAIN, false);
        self->timeTillPainFinished = Eng_Global->pauseRelativeTime + npcTable[self->index - 419].timeBetweenPain;
    }
}

static bool AIDeactivatesVisibleMeshWhileDying(Entity* self) { return self->index == 419 || self->index == 433 || self->index == 439 || (self->entflags & ENTFLAG_TELEPORT_ON_DEATH); }

static void AIDyingSetup(Entity* self) {
    self->enemey = 0;
    NPCTable* npc = &npcTable[self->index - 419];
    float dbt = deathBurstTimer[self->index - 419];
    if (dbt > 0.0f) {
        self->deathBurstFinished = Eng_Global->pauseRelativeTime + dbt;
    } else if (!(self->entflags & ENTFLAG_DEATH_BURST_DONE)) {
        // TODO Enable deathburst effects
    }

    if (!(self->entflags & ENTFLAG_ACT_AS_CORPSE_ONLY) && !(self->entflags & ENTFLAG_TELEPORT_ON_DEATH)) {
        int sded = sfxDeath[self->index - 419];
        if (sded >= 0 && sded < (int16_t)SOUNDS_COUNT) play_wav(sounds[sded], self->volume, self->position, true);
    }

    // Physics for death
    if (ai_is_cyber(self)) {
        self->gravity = 0.0f;
    } else {
        self->gravity = 1.0f;
        flag_set(&self->entflags, ENTFLAG_KINEMATIC, true);
    }

    flag_set(&self->entflags, ENTFLAG_ASLEEP, false);
    self->layer = PhysicsLayer_Corpse;
    flag_set(&self->entflags, ENTFLAG_FIRST_SIGHTING, true);
    self->timeTillDeadFinished = Eng_Global->pauseRelativeTime + npc->timeTillDead;
    if (npc->switchMaterialOnDeath && self->dyingTexture) self->texIndex = self->dyingTexture;
    if (self->index == 428 || self->index == 439) self->velocity = (Vector3){0.0f, self->velocity.z, 0.0f}; // Index-specific velocity patch (Exec bot and Zero-G mutant)
    if (self->index == 433) self->layer = PhysicsLayer_Corpse; // Hopper: enable capsule collider (implicit in layer change)
    flag_set(&self->entflags, ENTFLAG_DYING_SETUP, true);
}

static void AIDying(Entity* self) {
    if (!(self->entflags & ENTFLAG_DYING_SETUP)) AIDyingSetup(self);
    if (self->timeTillDeadFinished < Eng_Global->pauseRelativeTime) {
        flag_set(&self->entflags, ENTFLAG_DEAD,   true);
        flag_set(&self->entflags, ENTFLAG_DYING,  false);
        self->currentState = AIState_Dead;
    }

    if (AIDeactivatesVisibleMeshWhileDying(self)) flag_set(&self->entflags,ENTFLAG_VISIBLE,false);
    if (self->index == 439) self->layer = PhysicsLayer_Corpse | PhysicsLayer_CorpseSearchable; // Zero-G mutant enables search collider while still dying
}

static void AIDead(uint16_t idx) {
    Entity* self = &Eng_Global->instances[idx];   
    flag_set(&self->entflags, ENTFLAG_ASLEEP,       false);
    flag_set(&self->entflags, ENTFLAG_DEAD,         true);
    flag_set(&self->entflags, ENTFLAG_DYING,        false);
    flag_set(&self->entflags, ENTFLAG_DYING_SETUP,  false);
    if (self->entflags & ENTFLAG_DEAD_CHECKS_DONE) return;

    if (AIDeactivatesVisibleMeshWhileDying(self)) flag_set(&self->entflags,ENTFLAG_VISIBLE,false);
    self->currentState = AIState_Dead;
    self->layer = PhysicsLayer_Corpse;
    if (self->entflags & ENTFLAG_TELEPORT_ON_DEATH) {
        self->gravity = 1.0f;
        flag_set(&self->entflags,ENTFLAG_VISIBLE,false);
        // TODO: TeleportAway(ai_self_idx(self)), DeleteInstance(idx);
    } else if (ai_is_cyber(self)) {
        self->gravity = 0.0f;
        flag_set(&self->entflags, ENTFLAG_VISIBLE,false);
        // TODO: Gib(ai_self_idx(self)) — spawn gibs
        DeleteInstance(idx);
    } else {
        // Enable search collider for non-gib corpses (Avian Mutant index 2 always searchable)
        self->layer = PhysicsLayer_Corpse | PhysicsLayer_CorpseSearchable;
        self->velocity.x = 0.0f; self->velocity.z = 0.0f;
        if (self->index != 433) self->gravity = 1.0f;// Hopper deactivates itself
    }

    flag_set(&self->entflags, ENTFLAG_DEAD_CHECKS_DONE, true);
}

static DamageData SetNPCData(Entity* self, int n) {
    DamageData dd = {0};
    NPCTable* npc = &npcTable[self->index - 419];
    dd.owner = ai_self_idx(self);
    switch (n) {
        case 1: dd.damage = npc->damage;  dd.attackType = npc->attackType;  break;
        case 2: dd.damage = npc->damage2; dd.attackType = npc->attackType2; break;
        default: dd.damage = npc->damage3; dd.attackType = npc->attackType3; break;
    }
    
    dd.penetration = 0;
    dd.defense = 0;
    return dd;
}

static float ai_damage_take_amount(DamageData dd) {
    // TODO: refine formula when HealthManager is ported
    float reduction = dd.defense / (dd.defense + dd.offense + 1.0f);
    return dd.damage * (1.0f - reduction);
}

static void ai_apply_damage(DamageData dd, uint16_t hitIdx) {
    if (!hitIdx || hitIdx >= INSTANCE_COUNT) return;
    dd.hitIdx = hitIdx;
    dd.damage = ai_damage_take_amount(dd);
    if (hitIdx == PLAYER1 || hitIdx == PLAYER2) {
        PlayerTakeDamage(hitIdx, dd.damage);
    } else {
        Entity* t = &Eng_Global->instances[hitIdx];
        t->health -= dd.damage;
        if (t->health < 0.0f) t->health = 0.0f;
        t->recentMostActivator = dd.owner;
        flag_set(&t->entflags, ENTFLAG_GO_INTO_PAIN, true);
    }
}

static void AIApplyAttackMovement(Entity* self, float speed) {
    uint16_t eidx = self->enemey;
    if (!eidx) return;
    if (self->entflags & ENTFLAG_ACT_AS_TURRET) { self->currentDestination = ai_sight_pos(self); return; }
    if (speed <= 0.0f || self->tranquilizeFinished >= Eng_Global->pauseRelativeTime) return;
    self->currentDestination = Eng_Global->instances[eidx].position;
    if (dist_sq_vector3(ai_sight_pos(self), self->currentDestination) <= AI_STOP_DIST_SQ) return;
    if (!AIWithinAngleToTarget(self)) return;
    AddForce(ai_self_idx(self), scale_vector3(self->forward, speed), false);
}

static void AITransitionAttackToRun(Entity* self, int n) {
    flag_set(&self->entflags, ENTFLAG_GO_INTO_PAIN, false);
    self->currentState = AIState_Run;
    NPCTable* npc = &npcTable[self->index - 419];
    float chance, wmin, wmax;
    double* wait;
    switch (n) {
        case 1: chance=npc->timeAttack1WaitChance; wmin=npc->timeAttack1WaitMin; wmax=npc->timeAttack1WaitMax; wait=&self->randomWaitForNextAttack1Finished; break;
        case 2: chance=npc->timeAttack2WaitChance; wmin=npc->timeAttack2WaitMin; wmax=npc->timeAttack2WaitMax; wait=&self->randomWaitForNextAttack2Finished; break;
        default: chance=npc->timeAttack3WaitChance; wmin=npc->timeAttack3WaitMin; wmax=npc->timeAttack3WaitMax; wait=&self->randomWaitForNextAttack3Finished; break;
    }
    *wait = (random_range(0.0f, 1.0f) < chance) ? Eng_Global->pauseRelativeTime + random_range(wmin, wmax) : Eng_Global->pauseRelativeTime;
}

static void MuzzleBurst(Entity* self, int attackNum) {
    (void)self;
    if (attackNum < 1 || attackNum > 3) attackNum = 1;
//     if (self->index == 437) Utils.Activate(muzzleBurst); // Activate this one too.
//     switch (attackNum) { // No muzzle burst for Attack1 melee. TODO
//         case 2: Utils.Activate(muzzleBurst); break;
//         case 3: Utils.Activate(muzzleBurst2); break;
//     }
}

static void ProjectileRaycast(Entity* self, int n) {
    if (n < 1 || n > 3) n = 1;
    Vector3 spos = (n == 1) ? ai_sight_pos(self) : ai_gun_pos(self, n);
    uint16_t eidx = self->enemey;
    Vector3 targ = eidx ? self->targettingPosition : (Vector3){spos.x + self->forward.x*10.0f, spos.y, spos.z + self->forward.z*10.0f};
    Vector3 dir  = (n == 1) ? self->forward : normalize_vector3(Vector3_A_minus_B(targ, spos));
    float range;
    switch (n) {
        case 1: range = npcTable[self->index - 419].range; break;
        case 2: range = npcTable[self->index - 419].range2; break;
        default: range = npcTable[self->index - 419].range3; break;
    }

    MuzzleBurst(self,n);
    RaycastHit hit = Raycast(spos, dir, range, LAYER_MASK_NPC_ATTACK);
    if (!hit.hit) return;

    uint16_t hi = hit.hitInstanceIndex;
    // TODO: laser effect SpawnDynamicObject(408) when npc has laser on attack

    // Targeting laser (Cyborg Elite, attack3)
    if (n == 3 && self->index == 427 && eidx) AddDebugLine(ai_sight_pos(self), Eng_Global->instances[eidx].position);

    DamageData dd = SetNPCData(self, n);
    dd.hitpoint     = hit.point;
    dd.attacknormal = dir;
    dd.impactVelocity = dd.damage;
    bool hitPlayer = (hi == PLAYER1 || hi == PLAYER2);
    if (hitPlayer) dd.impactVelocity *= 0.5f;
    dd.isOtherNPC = !hitPlayer && ConstIndexIsNPC(Eng_Global->instances[hi].index);
    if (hi) ai_apply_damage(dd, hi);
    uint16_t impactCI = GetImpactType(hi);
    if (impactCI) {
        uint16_t imp = SpawnDynamicObject(impactCI, true);
        if (imp && imp < INSTANCE_COUNT) Eng_Global->instances[imp].position = hit.point;
    }
}

static void ProjectileLaunched(Entity* self, int n) {
    NPCTable* npc = &npcTable[self->index - 419];
    int masterIdx; float launchSpd;
    switch (n) {
        case 1: masterIdx = npc->projectile1Prefab; launchSpd = npc->projectileSpeedAttack1; break;
        case 2: masterIdx = npc->projectile2Prefab; launchSpd = npc->projectileSpeedAttack2; break;
        default: masterIdx = npc->projectile3Prefab; launchSpd = npc->projectileSpeedAttack3; break;
    }
    
    Vector3 spos = ai_gun_pos(self, n);
    uint16_t eidx = self->enemey;
    Vector3 targ = eidx ? self->targettingPosition : (Vector3){spos.x + self->forward.x*20.0f, spos.y, spos.z + self->forward.z*20.0f};
    Vector3 dir  = normalize_vector3(Vector3_A_minus_B(targ, spos));
    MuzzleBurst(self,n);
    uint16_t bb = SpawnDynamicObject((uint16_t)masterIdx, false);
    if (!bb || bb >= INSTANCE_COUNT) bb = SpawnDynamicObject(370, false);
    if (!bb || bb >= INSTANCE_COUNT) return;

    Entity* proj   = &Eng_Global->instances[bb];
    proj->layer    = PhysicsLayer_NPCBullet;
    proj->position = spos;
    proj->forward  = dir;
    // TODO: store damage data into projectile entity fields for deferred impact
    Vector3 shove = scale_vector3(dir, launchSpd);
    if (vabs(self->gravity) > 0.05f) { shove.x += self->velocity.x; shove.z += self->velocity.z; }
    proj->velocity = (Vector3){0,0,0};
    AddForce(bb, shove, true);
    flag_set(&proj->entflags, ENTFLAG_ACTIVE | ENTFLAG_VISIBLE | ENTFLAG_RIGIDBODY, true);
}

static void AIExplodeAttack(Entity* self) {
    float radius = npcTable[self->index - 419].attack3Radius; float force  = npcTable[self->index - 419].attack3Force;
    Vector3 epos = ai_sight_pos(self);
    DamageData dd = SetNPCData(self, 3);
    for (uint16_t i = START_INDEX_LEVEL_INSTANCES; i < Eng_Global->loadedInstances; ++i) {
        Entity* t = &Eng_Global->instances[i];
        if (!(t->entflags & ENTFLAG_ACTIVE)) continue;
        
        float dsq = dist_sq_vector3(epos, t->position);
        if (dsq >= radius * radius) continue;
        
        float dist = vsqrtf(dsq), falloff = 1.0f - dist / radius;
        DamageData tdd = dd; tdd.damage *= falloff;
        ai_apply_damage(tdd, i);
        if (dist > 0.001f) AddForce(i, scale_vector3(normalize_vector3(Vector3_A_minus_B(t->position, epos)), force * falloff), true);
    }
    
    self->health = 0.0f; // Self-destruct
}

static void AIMakeAttack(Entity* self, AttackType att, int ind) {
    if (ind < 1 || ind > 3) ind = 1; // Melee hitscan by default.
    switch (att) {
        case AttackType_Melee:              ProjectileRaycast(self,ind);  break;
        case AttackType_Projectile:         ProjectileRaycast(self,ind);  Eng_Global->fogFac += 1; break;
        case AttackType_ProjectileLaunched: ProjectileLaunched(self,ind); Eng_Global->fogFac += 1; break;
        default: break;
    }
}

static void AIAttack1(Entity* self) {
    NPCTable* npc = &npcTable[self->index - 419];
    AIApplyAttackMovement(self, npc->attack1Speed);
    if (self->gracePeriodFinished < Eng_Global->pauseRelativeTime && !(self->entflags & ENTFLAG_SHOT_FIRED)) {
        flag_set(&self->entflags, ENTFLAG_SHOT_FIRED, true);
        int sat = sfxAttack1[self->index - 419];
        if (self->attack1SoundTime < Eng_Global->pauseRelativeTime && sat >= 0 && sat < (int16_t)SOUNDS_COUNT) {
            play_wav(sounds[sat], self->volume, self->position, true);
            self->attack1SoundTime = Eng_Global->pauseRelativeTime + npc->timeBetweenAttack1;
        }

        AIMakeAttack(self, npc->attackType, 1);
    }
    if (self->attackFinished < Eng_Global->pauseRelativeTime) AITransitionAttackToRun(self, 1);
}

static void AIAttack2(Entity* self) {
    NPCTable* npc = &npcTable[self->index - 419];
    AIApplyAttackMovement(self, npc->attack2Speed);
    if (self->gracePeriodFinished < Eng_Global->pauseRelativeTime && !(self->entflags & ENTFLAG_SHOT_FIRED)) {
        flag_set(&self->entflags, ENTFLAG_SHOT_FIRED, true);
        int sat2 = sfxAttack2[self->index - 419];
        if (self->attack2SoundTime < Eng_Global->pauseRelativeTime && sat2 >= 0 && sat2 < (int16_t)SOUNDS_COUNT) {
            play_wav(sounds[sat2],self->volume,self->position,true);
            self->attack2SoundTime = Eng_Global->pauseRelativeTime + npc->timeBetweenAttack2;
        }
        AIMakeAttack(self,npc->attackType2,2);
    }
    if (self->attackFinished < Eng_Global->pauseRelativeTime) AITransitionAttackToRun(self, 2);
}

static void AIAttack3(Entity* self) {
    NPCTable* npc = &npcTable[self->index - 419];
    if (npc->explodeOnAttack3) { Eng_Global->fogFac += 5; AIExplodeAttack(self); return; }
    AIApplyAttackMovement(self, npc->attack3Speed);
    if (self->gracePeriodFinished < Eng_Global->pauseRelativeTime && !(self->entflags & ENTFLAG_SHOT_FIRED)) {
        flag_set(&self->entflags, ENTFLAG_SHOT_FIRED, true);
        int sat3 = sfxAttack3[self->index - 419];
        if (self->attack3SoundTime < Eng_Global->pauseRelativeTime && sat3 >= 0 && sat3 < (int16_t)SOUNDS_COUNT) {
            play_wav(sounds[sat3], self->volume, self->position, true);
            self->attack3SoundTime = Eng_Global->pauseRelativeTime + npc->timeBetweenAttack3;
        }
        
        AIMakeAttack(self, npc->attackType3, 3);
    }
    
    if (self->index == 427 && self->enemey) AddDebugLine(ai_sight_pos(self),Eng_Global->instances[self->enemey].position);
    if (self->attackFinished < Eng_Global->pauseRelativeTime) AITransitionAttackToRun(self, 3);
}

static void AIFlierMoveToHoverHeight(Entity* self) {
    NPCTable* npc = &npcTable[self->index - 419];
    if (npc->runSpeed <= 0.0f) return;
    uint16_t eidx = self->enemey;
    if (eidx) {
        self->idealPos.y = Eng_Global->instances[eidx].position.y + AI_TARGET_OFFSET_Y;
        self->idealPos.x = self->position.x;
        self->idealPos.z = self->position.z;
    } else {
        Vector3 sp = ai_sight_pos(self);
        RaycastHit dn = Raycast(sp, (Vector3){0,-1,0}, npc->sightRange, LAYER_MASK_NPC_SIGHT);
        RaycastHit up = Raycast(sp, (Vector3){0, 1,0}, npc->sightRange, LAYER_MASK_NPC_SIGHT);
        float dDn = dn.hit ? dn.distance : 0.0f, dUp = up.hit ? up.distance : 0.0f;
        float yH  = npc->flightHeight * (npc->flightHeightIsPercentage ? dDn + dUp : 1.0f);
        Vector3 fp = dn.hit ? dn.point : self->position;
        self->idealPos = (Vector3){ fp.x, fp.y + yH, fp.z };
    }
    
    float dy = self->idealPos.y - self->position.y;
    if (vabs(dy) < 0.16f) return;
    float spd  = npc->runSpeed * (float)Eng_Global->deltaTime;
    float step = vmin(vabs(dy), spd) * (dy < 0.0f ? -1.0f : 1.0f);
    self->position.y += step;
}

float AITranquilize(uint16_t idx, float amount, bool energy) {
    Entity* self = &Eng_Global->instances[idx];
    float secs = (amount < 3.0f) ? (float)npcTable[self->index - 419].timeForTranquilization : amount;
    if (npcTable[self->index - 419].type != NPCType_Robot || energy) {
        double a = Eng_Global->pauseRelativeTime + secs, b = self->tranquilizeFinished + secs;
        self->tranquilizeFinished = a > b ? a : b;
        return secs;
    }
    return 0.0f;
}

void AIAlert(uint16_t idx) {
    if (Eng_Global->difficultyCombat == 0) return;
    Entity* self = &Eng_Global->instances[idx];
    AISetEnemy(self, PLAYER1);
    self->currentDestination = Eng_Global->instances[PLAYER1].position;
    flag_set(&self->entflags, ENTFLAG_ENEM_IN_SIGHT, false);
}

void AIAwakeFromSleep(uint16_t idx) {
    Entity* self = &Eng_Global->instances[idx];
    flag_set(&self->entflags, ENTFLAG_ASLEEP, false);
    // TODO deactivate sleeping cables
    AIAlert(idx);
}

static void AIThink(uint16_t idx) {
    Entity* self = &Eng_Global->instances[idx];   
    if ((self->entflags & ENTFLAG_DYING_SETUP) && self->deathBurstFinished < Eng_Global->pauseRelativeTime && !(self->entflags & ENTFLAG_DEATH_BURST_DONE)) {
        // TODO activate death burst effect
        flag_set(&self->entflags, ENTFLAG_DEATH_BURST_DONE, true);
    }

    if (!ai_has_health(self)) {
        if (!(self->entflags & ENTFLAG_DYING) && !(self->entflags & ENTFLAG_DEAD)) {
            flag_set(&self->entflags, ENTFLAG_DYING, true);
            self->currentState = AIState_Dying;
        } else if ((self->entflags & ENTFLAG_DEAD) && self->currentState != AIState_Dead) {
            self->currentState = AIState_Dead;
        } else if ((self->entflags & ENTFLAG_DYING) && self->currentState != AIState_Dying) {
            self->currentState = AIState_Dying;
        }
    }

    switch (self->currentState) {
        case AIState_Idle:    AIIdle(self);    break;
        case AIState_Walk:    AIWalk(self);    break;
        case AIState_Run:     AIRun(self);     break;
        case AIState_Attack1: AIAttack1(self); break;
        case AIState_Attack2: AIAttack2(self); break;
        case AIState_Attack3: AIAttack3(self); break;
        case AIState_Pain:    AIPain(self);    break;
        case AIState_Dying:   AIDying(self);   break;
        case AIState_Dead:    AIDead(idx);    break;
        default:              AIIdle(self);    break;
    }

    if (self->currentState == AIState_Dead || self->currentState == AIState_Dying) return;
}

void AIControllerUpdate(uint16_t idx) {
    Entity* self = &Eng_Global->instances[idx];
    if (!(self->entflags & ENTFLAG_ACTIVE)) return;

    if (!ai_is_cyber(self) && npcTable[self->index - 419].moveType != AIMoveType_Fly && self->currentState != AIState_Dead && self->currentState != AIState_Dying) self->gravity = 1.0f;
    flag_set(&self->entflags, ENTFLAG_ENEM_IN_SIGHT, AICheckIfPlayerInSight(self));
    uint16_t eidx = self->enemey;
    if (eidx && ai_has_health(self)) {
        Entity* en = &Eng_Global->instances[eidx];
        bool enAlive = ai_is_cyber(self) ? en->cyberHealth > 0.0f : en->health > 0.0f;
        if (!enAlive) {
            if (ai_is_cyber(self)) {
                self->currentState = AIState_Idle;
            } else {
                flag_set(&self->entflags, ENTFLAG_WANDERING, true);
                self->wanderFinished = Eng_Global->pauseRelativeTime + random_range(3.0f, 8.0f);
                self->currentState = AIState_Walk;
            }
            
            self->enemey = 0;
            self->posCheckFinished = Eng_Global->pauseRelativeTime;
            self->lastPosition = self->position;
        } else {
            AIEnemyInFrontChecks(self, eidx);
            Vector3 d = Vector3_A_minus_B(en->position, ai_sight_pos(self));
            self->rangeToEnemy = dot_vector3(d, d);
        }
    } else {
        flag_set(&self->entflags, ENTFLAG_ENEM_IN_FRONT, false);
        flag_set(&self->entflags, ENTFLAG_ENEM_IN_FOV,   false);
        float sr = npcTable[self->index - 419].sightRange;
        self->rangeToEnemy = sr * sr;
    }

    if (self->tickFinished < Eng_Global->pauseRelativeTime) { self->tickFinished = Eng_Global->pauseRelativeTime + AI_TICK_TIME; AIThink(idx); }
    if (self->currentState != AIState_Dead && self->currentState != AIState_Idle) {
        if ((self->entflags & ENTFLAG_ACT_AS_TURRET) && eidx) {
            Entity* en = &Eng_Global->instances[eidx];
            self->currentDestination = (Vector3){ en->position.x, en->position.y + AI_TARGET_OFFSET_Y, en->position.z };
        }
        
        if (ai_is_cyber(self) && eidx) self->currentDestination = Eng_Global->instances[eidx].position;
        Vector3 toTarget = Vector3_A_minus_B(self->currentDestination, ai_sight_pos(self));
        if (!ai_is_cyber(self)) toTarget.y = 0.0f;
        self->idealTransformForward = normalize_vector3(toTarget);
        float sqmag = dot_vector3(toTarget, toTarget);
        if (sqmag > 1e-6f || ai_is_cyber(self)) AIFace(self, self->currentDestination);
    }

    if (npcTable[self->index - 419].moveType == AIMoveType_Fly && self->tranquilizeFinished < Eng_Global->pauseRelativeTime) AIFlierMoveToHoverHeight(self);
}
