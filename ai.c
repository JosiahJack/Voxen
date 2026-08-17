// ai.c - AI logic control for NPC's enemies in the game.
#include "common.h"
static const float AI_STOP_DIST=1.28f, AI_STOP_DIST_SQ=(AI_STOP_DIST * AI_STOP_DIST), AI_POS_CHECK_DELAY=2.0f, AI_WANDER_RANGE=79.0f, AI_TARGET_OFFSET_Y=0.24f;
u16 npcCountInWorldPerType[NUM_AI_TYPES];
// Name,AtkTyp1,2,3,Dmg1,2,3,Range1,2,3,Health,CybHealth,Percp,Disrp,Armr,Def,Movtyp,Yawspd,FOV,FOVAtk,FOVStartMov,DistToSeeBehind,SightRange,WalkSpd,RunSpd,AtkSpd1,2,3,AtkForce3,AtkRad3,TtPain,TbwPain,TtDead,TtActualAtk1,2,3,TbwAtk1,2,3,TEnemChg,TIdleSFXMin,TIdleSFXMax,TAtk1WaitMin,TAtk1WaitMax,TAtk1WaitChnc,TAtk2WaitMin,TAtk2WaitMax,TAtk2WaitChnc,TAtk3WaitMin,TAtk3WaitMax,TAtk3WaitChnc,ProjType1,2,3,ProjSpd1,2,3,HasLaser1,2,3,ExplodeOn3,PreActMeleCols,THunt,FlightHeight,FlightHeightIsPerc,SwitchMatOnDie,RangeHear,TTranq,Hops,NPCType,AtkProj1,2,3
NPCTable npcTable[NUM_AI_TYPES] = {
 {"AUTOBOMB"              ,0,0,1,  0,  0,200,   0,    0,2.4,50,0,1,0.5,40,1,1,300,180,120,55,3.84,50,2.5,2.5,0,0,0,100,6,0,0,0.1,0,0,0,0,0,0,3,5,12,0.5,1,0.1,1,3,0.5,0,0,0,0,0,0,0,0,0,0,0,0,1,0,20,0,0,0,10,3,0,2,0,0,0 },
 {"CYBORG ASSASSIN"       ,0,4,7, 30, 50, 35, 3.3,   10,20,65,0,2,0.6,5,4,1,180,180,80,15,3.2,50,2,2,0,0,0,0,0,0.45,5,2.083,0,0.25,0.2,0.91,0.91,1.58,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,0,3,0,0,0,0,0,60,0,0,0,10,3,0,3,0,0,489 },
 {"AVIAN MUTANT"          ,1,0,0, 40, 40,  0, 3.3,   10,20,125,0,1,0.25,0,2,2,180,180,80,15,5.12,50,2,2,3.5,0,0,0,0,2,5,1,0.1,0,0,1,0,0,3,5,12,0.5,1,0.1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,60,0.65,1,0,10,3,0,1,0,0,0 },
 {"EXEC-BOT"              ,0,4,0, 30, 35,  0, 3.3,   10,20,225,0,1,0.2,40,2,1,200,180,15,30,4.12,50,1.5,1.5,0,0,0,0,0,0.45,7,0.15,0,0.2,0,0,1.5,0,3,5,12,0,0,0,0.97,2,0.3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,180,0,0,0,10,3,0,2,0,0,0 },
 {"CYBORG DRONE"          ,0,4,0, 20, 20, 20, 3.3,   25,50,60,0,1,0.3,0,2,1,65,180,80,15,3.2,50,1.6,2.2,0,0,0,0,0,0.542,15,0.958,0,0.1,0,0,1,0,3,20,45,0,0,0,1,2,0.5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,60,0,0,0,10,3,0,3,0,0,0 },
 {"CORTEX REAVER"         ,0,4,7, 80,325,125, 3.3,   20,30,580,0,1,0.1,40,2,1,180,180,80,15,3.84,50,2,2,0,0,0,0,0,0.583,5,0.333,0,0.35244,0.324,0,1,1,3,15,30,0,0,0,0.2,1,0.5,8,15,1,0,0,0,0,0,10,0,0,0,0,0,600,0,0,0,10,3,0,2,0,0,372 },
 {"CYBORG WARRIOR"        ,0,4,7, 35, 35,150, 3.3,   20,20,120,0,1,0.1,5,4,1,180,180,30,15,3.2,50,2.4,2.4,0,0,0,0,0,0.5,5,2.2,0,0.339,0.201,0,0.83,0.542,3,15,30,0,0,0,1,2,0.5,10,20,1,0,0,0,0,0,10,0,0,0,0,0,180,0,0,0,10,3,0,3,0,0,370 },
 {"CYBORG ENFORCER"       ,1,4,7, 60, 60, 80, 3.3,   15,30,285,0,1,0.1,30,5,1,180,180,80,15,3.2,50,2.8,2.8,2.8,0,0.3,0,0,2,5,1.5,0.23471,0.393738,0.313266,0.958,0.958,0.958,5,15,30,0.1,0.3,0.1,0.1,0.5,0.5,10,25,1,0,0,0,0,0,10,0,0,0,0,0,600,0,0,0,10,3,0,4,0,0,387 },
 {"CYBORG ELITE GUARD"    ,1,7,4, 70, 75,  0, 3.3,   10,50,380,0,1,0.05,50,6,1,180,180,80,15,3.2,50,3,3,1.5,0,0,0,0,0.4665,5,1.5,0.5,0.2653,0.117045,0.733,0.7,0.867,5,15,30,0.05,0.2,0.1,0.5,2,0.8,2,3,0.5,0,0,0,0,2,0,0,1,0,0,0,600,0,0,0,15,3,0,4,0,490,0 },
 {"CYBORG OF EDWARD DIEGO",1,7,0, 80, 95,  0, 3.3,   40,50,900,0,2,0,55,6,1,180,180,80,15,3.2,50,2.8,2.8,0,0,0,0,0,0,0,0,0.28,0.363188,0.2,1.4,0.833,3,5,15,30,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,2.5,0,0,0,0,0,1,600,0,0,0,15,3,0,4,0,490,0 },
 {"SECURITY-1 ROBOT"      ,0,4,0, 35, 35,  0, 3.3,   10,20,170,0,1,0.15,40,4,2,180,180,80,15,4.12,50,2.5,2.5,1.5,0,0,0,0,2,5,0.05,0.5,0.1,0.2,1.2,1.5,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,0,0,0,0,0,0,0,600,1.28,0,0,10,3,0,2,0,0,0 },
 {"SECURITY-2 ROBOT"      ,0,4,4, 65, 65, 15, 3.3,    5,35,300,0,2,0.05,50,5,1,180,180,60,25,4.12,50,1.5,1.5,1.5,0,0,0,0,0.75,5,0.25,0.5,0.39,0.1,1.2,1,1.5,3,5,12,0.5,1,0.1,3,3.5,1,2.5,3.5,1,0,0,0,0,0,0,0,0,0,0,0,600,0,0,0,10,3,0,2,0,0,0 },
 {"MAINTENANCE ROBOT"     ,1,0,0, 25, 25,  0, 3.3,  3.3,20,75,0,1,0.3,40,3,1,180,180,80,15,3.84,50,2.2,2.6,0.02,0.02,0,0,0,0,0,1.6,3,0.7,0.2,2,1.3,3,3,5,12,0.5,1,0.1,1,2,0.3,1,2,0.5,0,0,0,0,0,0,0,0,0,0,0,180,0,0,0,10,3,0,2,0,0,0 },
 {"MUTANT CYBORG"         ,1,7,0, 35, 75, 50,   2,   30,49,340,0,1,0.2,15,6,1,180,180,60,15,3.2,50,1.5,1.5,0,0,0,0,0,0.583,3.5,3.41,0.265,0.285,0.2,0.625,0.75,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,2.8,0,0,0,0,0,0,180,0,0,0,10,3,0,5,0,491,0 },
 {"HOPPER"                ,0,4,0, 35, 35,  0,   0,17.92,17.92,150,0,1,0.25,35,4,1,180,160,80,15,3.84,50,7,7,0,0,0,0,0,0.708,5,0,0.5,0.1,0.5,0.5,0.5,0.5,3,5,12,0.5,1,0.1,0.5,1,0.5,1,2,0.5,0,0,0,0,0,0,0,1,0,0,0,180,0,0,0,10,3,1,2,0,0,0 },
 {"HUMANOID MUTANT"       ,1,0,0, 12, 12,  0, 3.3,   10,20,50,0,0,0.4,0,3,1,60,180,80,15,2.56,50,1.4,2,0.5,0,0,0,0,0.42,5,0.967,0.5,0.1,0.2,1.2,1.5,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,0,0,0,0,0,0,0,20,0,0,0,10,3,0,0,0,0,0 },
 {"INVISIBLE MUTANT"      ,0,7,0, 10, 35,  0, 3.3,   20,20,350,0,1,0.05,0,2,2,180,180,80,15,2.56,50,0.7,0.7,1.5,0.7,0.7,0,0,0.875,5,1.125,0.875,0.4,0.2,1.2,0.875,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,2,0,0,0,0,0,0,60,0.32,0,1,10,3,0,0,0,486,0 },
 {"VIRUS MUTANT"          ,0,7,0, 45, 30,  0, 3.3,   20,20,140,0,0,0.1,0,3,1,180,180,80,15,2.56,50,2.5,2.5,2.5,0.3,0,0,0,0.542,3,1.792,0.2874,0.2874,0.2874,0.958,0.958,0.958,3,5,12,0.5,1,0.1,0.5,0,0.5,1,2,0.5,0,0,0,0,1.75,0,0,0,0,0,0,20,0,0,0,10,3,0,1,0,481,0 },
 {"SERV-BOT"              ,1,0,0,  8,  0,  0, 3.3,   10,20,20,0,1,0.5,20,2,1,180,180,80,15,3.84,50,2,2,1.2,0,0,0,0,1.125,2,0.98,0.2,0.1,0.2,0.834,1.5,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,0,0,0,0,0,0,0,180,0,0,0,10,3,0,2,0,0,0 },
 {"FLIER BOT"             ,0,4,7, 30,150,  0, 3.3,   35,40,75,0,1,0.3,30,2,2,180,180,80,15,5.12,50,1.5,1.5,1.5,0,0,0,0,1.375,5,0.6,0.1,0.1,0.2,1,1.5,3,3,5,12,0.5,1,0.1,1,2,0.5,10,12,1,0,0,0,0,0,10,0,0,0,0,0,180,0.85,1,0,10,3,0,2,0,0,404 },
 {"ZERO-G MUTANT"         ,0,7,0, 20, 20,  0, 3.3,   20,20,90,0,1,0.5,0,2,2,180,180,80,15,2.56,50,0.8,1.4,0,0.8,0,0,0,0.1,0,0.1,0.5,0.05,0.2,1.2,1.5,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,2,0,0,0,0,0,0,60,1.96,0,0,10,3,0,0,0,488,0 },
 {"GORILLA TIGER MUTANT"  ,1,0,0, 60, 60,  0, 3.3, 3.84,20,200,0,1,0.1,0,3,1,180,180,80,15,2.56,50,3,3.5,1,2,0,0,0,0.667,5,1.625,0.5,0.1,0.2,0.958,1.042,3,3,15,30,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,0,0,0,0,0,0,0,60,0,0,0,10,3,0,1,0,0,0 },
 {"REPAIR BOT"            ,0,4,0, 12, 12,  0, 3.3,  3.3,20,65,0,1,0.4,25,3,1,180,180,80,15,3.84,50,2.25,3,0.5,0,0,0,0,0,0,0.05,0.2,0.1,0.2,1.25,1.5,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,0,0,0,0,0,0,0,180,0,0,0,10,3,0,2,0,0,0 },
 {"PLANT MUTANT"          ,0,7,0, 35, 25,  0, 3.3,   20,20,115,0,1,0.3,0,1,1,180,180,80,15,2.56,50,0.8,1.2,0.1,0,0,0,0,0.375,2,2.208,0.89,0.82,0.2,1.91,1.027,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,3.5,0,0,0,0,0,0,20,0,0,0,10,3,0,0,0,487,0 },
 {"CYBER DOG"             ,0,7,0,  0, 25,  0,   0,   20,0,0,20,1,0.5,0,1,4,250,240,50,15,20.48,25.6,2,2,0,0,0,0,0,0.1,0,0.5,0,0,0,0,0.3,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1.5,0,0,0,0,0,0,500,0.75,0,0,10,0,0,6,0,493,0 },
 {"CYBER GUARD"           ,0,7,0,  0, 25,  0,   0,   20,0,0,35,1,0.4,0,1,4,250,240,50,15,20.48,25.6,2,2,0,0,0,0,0,0.1,0,0.5,0,0,0,0,0.2,0,2,998,999,0,0,0,0,0,0,0,0,0,0,0,0,0,0.8,0,0,0,0,0,0,500,0.75,0,0,10,0,0,6,0,493,0 },
 {"CYBER RAM"             ,0,7,0,  0, 35,  0,   0,   20,0,0,40,1,0.25,0,1,4,80,240,50,15,20.48,25.6,4,4,0,0,0,0,0,0.1,0,0.5,0,0,0,0,0.2,0,2,998,999,0,0,0,0,0,0,0,0,0,0,0,0,0,1.2,0,0,0,0,0,0,500,0.75,0,0,10,0,0,6,0,494,0 },
 {"CYBER CORTEX REAVER"   ,0,7,0,  0, 45,  0,   0,   20,0,0,80,1,0.1,0,1,4,80,240,50,15,20.48,25.6,4,4,0,0,0,0,0,0.1,0,0.5,0,0,0,0,0.2,0,2,998,999,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,500,0.75,0,0,10,0,0,6,0,494,0 },
 {"SHODAN"                ,0,7,0,  0, 55,  0,   0,   20,0,0,500,2,0,0,1,4,360,280,280,15,20.48,25.6,0,0,0,0,0,0,0,0.1,0,0.5,0,0,0,0,0.05,0,2,998,999,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,500,0.75,0,0,10,0,0,6,0,494,0 }};
//                             NPC Sounds 0,   1,   2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28
int sfxIdle[NUM_AI_TYPES] =           {  -1,  -1,  -1, -1, 58, -1, 59, -1, 59, 52, -1, -1, -1, -1, -1, -1,121, -1, -1, -1,121,118, -1, -1, -1, -1, -1, -1, -1};
int sfxSightSound[NUM_AI_TYPES] =     {  -1,  -1, 111,150, 58,150, 59,152,152, -1,150,150,151,152,150, -1,121, -1,151,150,121,119,151, -1, -1, -1, -1, -1, -1};
int sfxAttack1[NUM_AI_TYPES] =        {  -1,  -1, 108, -1, -1,146, -1,146,252,247, -1, -1, -1, -1, -1,122, -1,108,146, -1, -1,118, -1,125,258,258,258,258,258};
int sfxAttack2[NUM_AI_TYPES] =        {  -1, 256,  -1,148, 50, 50, 50, 50, 50,250, 50, 50,146,259,148, -1,121, -1, -1,147, -1, -1,146, -1,258,258,258,258,258};
int sfxAttack3[NUM_AI_TYPES] =        {  -1,  -1,  -1, -1, -1,244,244,244,245, -1, -1,149, -1, -1, -1, -1, -1, -1, -1,244, -1, -1, -1, -1,258,258,258,258,258};
int sfxDeath[NUM_AI_TYPES] =          {  -1,  48, 110,143, 48,145, 48, 51, 47, 47,142,143,144, 47,162,123,120,134,144,144,120,117,144,124, -1, -1, -1, -1, -1};
float deathBurstTimer[NUM_AI_TYPES] = {0.0f,0.0f, 0.1f,0.0f,0.1f,0.1f,0.2f,0.1f,0.1f,0.1f,0.0f,0.45f,0.75f,0.1f,0.0f,0.0f,0.1f,0.224f,0.9f,0.0f,0.1f,0.1f,0.1f,0.2f,0.1f,0.1f,0.1f,0.1f,0.1f};
void SetHuntFinished(u16 i) {
    u16 npcID = World.instances[i].index - 419; World.instances[i].huntFinished = World.pauseRelativeTime; int diff = (npcTable[npcID].type == NPCType_Cyber) ? World.diffCyb : World.diffCbt;
    if (diff <= 1) { World.instances[i].huntFinished += vmax((npcTable[npcID].huntTime * 0.75),60.0); } // More forgetful on easy.
    else if (diff >= 3) { World.instances[i].huntFinished += vmax((npcTable[npcID].huntTime * 2.00),60.0); } // Good memory on hard.
    else { World.instances[i].huntFinished += vmax(npcTable[npcID].huntTime, 60.0); }
}

void InitNPC(u16 i) {
    World.layer[i] = L_NPC;
    u16 npcID = World.instances[i].index - 419;
    World.instances[i].idleTime = World.pauseRelativeTime + (double)random_range(npcTable[npcID].timeIdleSFXMin,npcTable[npcID].timeIdleSFXMax);
    World.instances[i].attack1SoundTime = World.instances[i].attack2SoundTime = World.instances[i].attack3SoundTime = World.pauseRelativeTime;
    World.instances[i].timeTillEnemyChangeFinished = World.pauseRelativeTime;
    SetHuntFinished(i);
    World.instances[i].attackFinished = World.pauseRelativeTime;
    World.instances[i].attack2Finished = World.pauseRelativeTime;
    World.instances[i].attack3Finished = World.pauseRelativeTime;
    World.instances[i].timeTillPainFinished = World.pauseRelativeTime;
    World.instances[i].timeTillDeadFinished = World.pauseRelativeTime;
    World.instances[i].meleeDamageFinished = World.pauseRelativeTime;
    World.instances[i].gracePeriodFinished = World.pauseRelativeTime;
    World.instances[i].randWaitAtt1Finished = World.pauseRelativeTime;
    World.instances[i].randWaitAtt2Finished = World.pauseRelativeTime;
    World.instances[i].randWaitAtt3Finished = World.pauseRelativeTime;
    World.instances[i].tranquilizeFinished = World.pauseRelativeTime;
    World.instances[i].deathBurstFinished = World.pauseRelativeTime;
    World.instances[i].wanderFinished = World.pauseRelativeTime;
    World.instances[i].posCheckFinished = World.pauseRelativeTime;
    World.instances[i].lastPosition = World.position[i];
    World.instances[i].timeSinceMovedEnough = 0.0;
    World.instances[i].currentState = AIState_Idle;
    if ((World.instances[i].entflags & EF_WANDERING) && (random_range(0.0f,1.0f) < 0.5f)) World.instances[i].currentState = AIState_Walk;
    else flag_set(&World.instances[i].entflags,EF_WANDERING,false);
    if (World.instances[i].entflags & EF_ASLEEP) { World.instances[i].currentState = AIState_Idle; /*flag_set(&World.instances[e->sleepingCables].entflags, EF_ACTIVE, true); *//*TODO*/ }
    World.instances[i].attackFinished = World.pauseRelativeTime + 1.0;
    World.instances[i].idealTransformForward = World.instances[i].forward;
    #define TARGET_ID_LENGTH 32 // Max needed 22 + 5 for ID + 1 for space between them = 28
    //scpy_to_a_from_b(World.instances[i].targetID,npcTable[npcID].name,TARGET_ID_LENGTH);
    //TODO TARGET ID: Type-LevelNum(0#)EnemyNum(###),Example: Mutant-06003, EXCEPTIONS: Cyborg-00001 is Edward Diego
    //sFormat(World.instances[i].targetID,TARGET_ID_LENGTH * sizeof(char),"%s %05u",npcTable[npcID].name,npcCountInWorldPerType[npcID]++); // TODO
    u8 c;
    switch (World.instances[i].currentState) {
        case AIState_Walk:                     c = ANIM_WALK;    break;
        case AIState_Run:                      c = ANIM_RUN;     break;
        case AIState_Attack1:                  c = ANIM_ATTACK1; break;
        case AIState_Attack2:                  c = ANIM_ATTACK2; break;
        case AIState_Attack3:                  c = ANIM_ATTACK3; break;
        case AIState_Pain:                     c = ANIM_PAIN;    break;
        case AIState_Dying: case AIState_Dead: c = ANIM_DYING;   break;
        default:                               c = ANIM_IDLE;    break;
    }
    
    World.instances[i].clip = c;
    World.instances[i].frame = modelAnimationClips[World.instances[i].animationNum][c].frameStart;
    World.instances[i].currentFrameFinished = 0.0;
}
    
float Tranquilize(u16 i, float amount, bool energy) {
    u16 npcID = World.instances[i].index - 419;
    if (npcTable[npcID].type == NPCType_Robot && !energy) return 0.0f;

    float tranqSecs = (amount < 3.0f) ? npcTable[npcID].timeForTranquilization : amount; // If we're going to tranq, at least do it for 3 secs.
    World.instances[i].tranquilizeFinished = vmax(World.pauseRelativeTime + tranqSecs, World.instances[i].tranquilizeFinished + tranqSecs);
    return tranqSecs;
}

static bool IsCyberNPC(u16 i) { u16 npcID = World.instances[i].index - 419; return npcTable[npcID].type == NPCType_Cyber; }

bool HasHealth(u16 i) {
    if (IsCyberNPC(i)) return (World.instances[i].cyberHealth > 0.0f);
    return (World.instances[i].health > 0.0f);
}

INLINE bool ai_is_cyber(Entity* e)  { return npcTable[e->index - 419].type == NPCType_Cyber; }
INLINE bool ai_has_health(Entity* e){ return ai_is_cyber(e) ? e->cyberHealth > 0.0f : e->health > 0.0f; }
INLINE V3 ai_sight_pos(Entity* e) { u16 idx=(u16)(e - World.instances); return V3_AplusB(World.position[idx],(V3){0.0f,0.0f,0.0f}/* e->sightPointOffset*/); } // TODO table of sight point offsets
INLINE V3 ai_gun_pos(Entity* e, int n) {
    V3 off = (V3){0.0f,0.0f,0.0f};//(n == 3) ? e->gunPointOffset2 : e->gunPointOffset; // TODO table of offsets
    if (n == 2 && off.x == 0.0f && off.y == 0.0f && off.z == 0.0f) off = (V3){0.0f,0.0f,0.0f};//e->gunPointOffset2; TODO
    u16 idx=(u16)(e - World.instances);
    return V3_AplusB(World.position[idx], off);
}

static Quaternion quat_look_rotation(V3 fwd, V3 up) {
    fwd = V3_Normalize(fwd);
    V3 r = V3_Normalize(V3_Cross(up, fwd));
    up = V3_Cross(fwd, r);
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

static void aiac_set_clip(Entity* self, u8 c) {
    if (self->clip == c) return;
    self->clip  = c;
    self->frame = modelAnimationClips[self->animationNum][c].frameStart;
    self->currentFrameFinished = 0.0;
}

static void aiac_freeze(Entity* self) { self->currentFrameFinished = World.current_time + 1e9; }
static void aiac_idle(Entity* self) { if ((self->entflags & EF_ASLEEP) || self->tranquilizeFinished >= World.current_time) { aiac_freeze(self); return; } aiac_set_clip(self, ANIM_IDLE); }
static void aiac_walk(Entity* self) {
    if (self->entflags & EF_ACT_AS_TURRET) { aiac_idle(self); return; }
    u16 idx=(u16)(self - World.instances);
    float spdsq = World.velocity[idx].x*World.velocity[idx].x + World.velocity[idx].z*World.velocity[idx].z;
    if (spdsq > (0.32f*0.32f)) { aiac_set_clip(self, ANIM_WALK); return; }
    if (self->animSwapFinished < World.current_time) { self->animSwapFinished = World.current_time + 0.5f; aiac_set_clip(self, ANIM_IDLE); }
}

static void aiac_run(Entity* self) {
    if (self->entflags & EF_ACT_AS_TURRET) { aiac_idle(self); return; }
    aiac_set_clip(self, ANIM_RUN);
}

static void aiac_dying(Entity* self) {
    flag_set(&self->entflags, EF_ASLEEP, false);
    // TODO check if it has no death anim and return
//     aiac_set_clip(self, ANIM_DYING);
//     AnimationClip cl = modelAnimationClips[self->animationNum][ANIM_DYING];
//     u16 range = cl.frameEnd > cl.frameStart ? cl.frameEnd - cl.frameStart : 1;
//     self->animatorPlaybackTime = (float)(self->frame - cl.frameStart) / (float)range; // TODO
//     if (self->animatorPlaybackTime > 0.99f) flag_set(&self->entflags,EF_DYING,false);
}

static void aiac_dead(Entity* self) {
    AnimationClip cl = modelAnimationClips[self->animationNum][ANIM_DYING];
    self->clip       = ANIM_DYING;
    self->frame      = cl.frameEnd;
    self->modelIndex = cl.frameStartModelIndex + (cl.frameEnd - cl.frameStart);
    aiac_freeze(self);
}

void AIAnimationControllerUpdate(u16 idx) {
    Entity* self = &World.instances[idx];
    if (!(self->entflags & EF_ACTIVE))        return;
    if (self->animationNum >= MAX_ANIMS)  return;
    if (self->currentState == AIState_Dying) { aiac_dying(self); return; }
    if (self->currentState == AIState_Dead)  { aiac_dead(self);  return; }
    if (self->entflags & EF_ASLEEP)     { aiac_idle(self);  return; }
    if (self->currentState == AIState_Run && self->tranquilizeFinished >= World.current_time) { aiac_idle(self); return; }
    
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

void DoorActuate(u16 self);
static bool AICheckIfEnemyInSight(u16 idx) {
    u16 eidx = World.instances[idx].enemy;
    if (!eidx || !ai_has_health(&World.instances[idx])) return false;
    bool enIsNPC = (World.layer[eidx] & L_NPC) != 0;
    int diff = ai_is_cyber(&World.instances[idx]) ? World.diffCyb : World.diffCbt;
    if (!ai_is_cyber(&World.instances[idx]) && !enIsNPC && !PositionVisibleFromPlayerCell(World.position[idx].x,World.position[idx].z)) return false;
    if (diff == 0 && (World.instances[idx].index - 419) != 28) return false;
    if (Cheats.notarget && !enIsNPC) { World.instances[idx].enemy = 0; World.instances[idx].posCheckFinished = World.pauseRelativeTime + AI_POS_CHECK_DELAY; World.instances[idx].lastPosition = World.position[idx]; flag_set(&World.instances[idx].entflags, EF_ENEM_IN_LOS, false); return false; }
    if (ai_is_cyber(&World.instances[idx]) && World.decoyActive) { flag_set(&World.instances[idx].entflags, EF_ENEM_IN_LOS, false); return false; }
    float dist = V3_Dist(World.position[eidx], ai_sight_pos(&World.instances[idx]));
    if (dist > npcTable[World.instances[idx].index - 419].sightRange) return false;
    if (ai_is_cyber(&World.instances[idx]) || enIsNPC) return true;
    V3 spos = ai_sight_pos(&World.instances[idx]);
    V3 lineN = V3_Normalize(V3_AsubB(World.position[eidx], spos));
    RaycastHit hit = Raycast(spos, lineN, npcTable[World.instances[idx].index - 419].sightRange, LMASK_NPC_SIGHT);
    if (hit.hit) {
        if (hit.hitInstanceIndex == eidx) { flag_set(&World.instances[idx].entflags, EF_ENEM_IN_LOS, true); return true; }
        // Smart NPCs try to open doors blocking line-of-sight
        NPCType t = npcTable[World.instances[idx].index - 419].type;
        if (t != NPCType_Mutant && t != NPCType_Supermutant && t != NPCType_Cyber) {
            u16 hi = hit.hitInstanceIndex;
            if (hi && V3_SqDist(hit.point, spos) < 4.0f && IdxIsDoor(World.instances[hi].index)) {
                Entity* dr = &World.instances[hi];
                if ((dr->doorOpen == DoorState_Closed || (dr->doorOpen == DoorState_Closing && World.diffCbt > 2)) && !(dr->entflags & EF_LOCKED) && GetCurrentLevelSecurity() <= dr->securityThreshold && (dr->requiredAccessCard == ACC_None)) DoorActuate(hi);
            }
        }
    }
    flag_set(&World.instances[idx].entflags, EF_ENEM_IN_LOS, false);
    return false;
}

static void AISetHuntFinished(u16 idx) {
    World.instances[idx].huntFinished = World.pauseRelativeTime;
    int diff = ai_is_cyber(&World.instances[idx]) ? World.diffCyb : World.diffCbt;
    double ht = npcTable[World.instances[idx].index - 419].huntTime;
    double mn = 60.0;
    if      (diff <= 1) World.instances[idx].huntFinished += (ht * 0.75 > mn ? ht * 0.75 : mn);
    else if (diff >= 3) World.instances[idx].huntFinished += (ht * 2.0  > mn ? ht * 2.0  : mn);
    else                World.instances[idx].huntFinished += (ht         > mn ? ht        : mn);
}

static void AISetEnemy(u16 idx, u16 eidx) {
    if (!eidx) return;
    
    World.instances[idx].enemy = eidx;
    World.instances[idx].posCheckFinished = World.pauseRelativeTime + AI_POS_CHECK_DELAY;
    flag_set(&World.instances[idx].entflags,EF_WANDERING,false);
    World.instances[idx].wanderFinished = World.pauseRelativeTime;
    World.instances[idx].lastPosition = World.position[idx];
    World.instances[idx].lastKnownEnemyPos = World.position[eidx];
    World.instances[idx].targettingPosition = (V3){World.position[eidx].x,World.position[eidx].y + AI_TARGET_OFFSET_Y,World.position[eidx].z};
    AISetHuntFinished(idx);
}

static void AIPlaySightSound(u16 idx) {
    if (!(World.instances[idx].entflags & EF_FIRST_SIGHTING)) return;
    if (!ai_has_health(&World.instances[idx])) return;
    if (World.instances[idx].entflags & EF_ACT_AS_CORPSE_ONLY) return;
    flag_set(&World.instances[idx].entflags, EF_FIRST_SIGHTING,false);
    i16 sfx = sfxSightSound[World.instances[idx].index - 419];
    if (sfx >= 39 && sfx < SOUNDS_COUNT) play_wav(sounds[sfx],World.instances[idx].volume,World.position[idx],true);
}

static bool AICheckIfPlayerInSight(u16 idx) {
    int diff = ai_is_cyber(&World.instances[idx]) ? World.diffCyb : World.diffCbt;
    if (!ai_is_cyber(&World.instances[idx]) && !PositionVisibleFromPlayerCell(World.position[idx].x,World.position[idx].z)) return false;
    if (diff == 0 && (World.instances[idx].index - 419) != 28) return false;
    if (World.instances[idx].enemy) return AICheckIfEnemyInSight(idx);
    flag_set(&World.instances[idx].entflags,EF_ENEM_IN_LOS,false);
    if (ai_is_cyber(&World.instances[idx]) && World.decoyActive) return false;
    if (Cheats.notarget) return false;
    V3 playerPos = World.position[PLAYER1];
    V3 spos      = ai_sight_pos(&World.instances[idx]);
    float dist = V3_Dist(playerPos, spos);
    NPCTable* npc = &npcTable[World.instances[idx].index - 419];
    if (dist > npc->sightRange) return false;

    if (ai_is_cyber(&World.instances[idx])) { AISetEnemy(idx,PLAYER1); AIPlaySightSound(idx); return true; }

    V3 checkN = V3_Normalize(V3_AsubB(playerPos, spos));
    float cosA = vclamp(V3_dot(checkN,World.instances[idx].forward), -1.0f, 1.0f);
    float angle = vacosf(cosA) * (180.0f / PI);
    bool makingNoise = World.instances[PLAYER1].noiseFinished > World.pauseRelativeTime;
    if (angle < npc->fov * 0.5f) {
        RaycastHit hit = Raycast(spos, checkN, dist + 0.1f, LMASK_NPC_SIGHT);
        if (hit.hit && hit.hitInstanceIndex == PLAYER1) { flag_set(&World.instances[idx].entflags, EF_ENEM_IN_LOS, true); AISetEnemy(idx,PLAYER1); AIPlaySightSound(idx); return true; }
        if (!hit.hit && makingNoise && dist < npc->hearingRange) { AISetEnemy(idx,PLAYER1); AIPlaySightSound(idx); return true; }
    } else {
        if (dist < npc->distToSeeBehind) {
            RaycastHit hit = Raycast(spos,checkN,dist + 0.1f,LMASK_NPC_SIGHT);
            if (hit.hit && hit.hitInstanceIndex == PLAYER1) { flag_set(&World.instances[idx].entflags, EF_ENEM_IN_LOS, true); AISetEnemy(idx,PLAYER1); AIPlaySightSound(idx); return true; }
        }
        if (makingNoise && dist < npc->hearingRange) { AISetEnemy(idx,PLAYER1); AIPlaySightSound(idx); return true; }
    }
    return false;
}

static void AIEnemyInFrontChecks(Entity* self, u16 eidx) {
    if (!eidx) { flag_set(&self->entflags,EF_ENEM_IN_FOV,false); flag_set(&self->entflags,EF_ENEM_IN_FRONT,false);  return; }
    if (ai_is_cyber(self)) { flag_set(&self->entflags,EF_ENEM_IN_FOV,true); flag_set(&self->entflags,EF_ENEM_IN_FRONT,true); return; }
    V3 spos=ai_sight_pos(self), epos=World.position[eidx];
    V3 iv = V3_Normalize((V3){epos.x - spos.x,0.0f,epos.z - spos.z}); float d = V3_dot(iv,self->forward);
    flag_set(&self->entflags,EF_ENEM_IN_FOV,d > 0.800f); flag_set(&self->entflags,EF_ENEM_IN_FRONT,d > 0.300f);
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

static void AIFace(Entity* self, V3 goal) {
    u16 sidx=(u16)(self - World.instances);
    if (self->entflags & EF_ASLEEP) return;
    V3 fv = V3_AsubB(goal,World.position[sidx]);
    if (!ai_is_cyber(self)) fv.y = 0.0f;
    if (fv.x == 0.0f && fv.y == 0.0f && fv.z == 0.0f) return;

    u16 eidx = self->enemy;
    if (ai_is_cyber(self) && eidx) { World.rotation[sidx] = World.rotation[eidx]; return; }

    if (fv.x == 0.0f && fv.z == 0.0f) {
        if (eidx) fv = V3_AsubB(World.position[eidx],World.position[sidx]);
        else fv.x += 0.001f;
    }

    Quaternion lr = quat_look_rotation(fv, (V3){0.0f, 1.0f, 0.0f});
    float t = (float)(0.2f * npcTable[self->index - 419].yawSpeed * World.deltaTime);
    World.rotation[sidx] = quat_slerp(World.rotation[sidx],lr,t);
}

static bool AIWithinAngleToTarget(Entity* self) {
    if (ai_is_cyber(self)) return true;
    if (V3_dot(self->idealTransformForward, self->idealTransformForward) <= 1e-6f) return false;
    
    u16 sidx=(u16)(self - World.instances);
    Quaternion lr = quat_look_rotation(self->idealTransformForward,(V3){0,1,0});
    float ang = quat_angle_deg(World.rotation[sidx], lr);
    float fovMov = npcTable[self->index - 419].fovStartMovement;
    if (ang < fovMov) return true;
    if (ang < fovMov * 1.5f && random_range(0.0f, 1.0f) < 0.5f) return true;
    return false;
}

bool AICheckPain(u16 self) {
    if (ai_is_cyber(&World.instances[self])
        || (World.instances[self].entflags & EF_ASLEEP)
        || (npcTable[World.instances[self].index - 419].timeBetweenPain <= 0.0f)
        || (!(World.instances[self].entflags & EF_GO_INTO_PAIN)
        || World.instances[self].timeTillPainFinished >= World.pauseRelativeTime)) return false;
    
    World.instances[self].currentState = AIState_Pain;
    u16 atkIdx = World.instances[self].recentMostActivator;
    if (atkIdx && World.instances[self].timeTillEnemyChangeFinished < World.pauseRelativeTime) {
        World.instances[self].timeTillEnemyChangeFinished = World.pauseRelativeTime + npcTable[World.instances[self].index - 419].timeToChangeEnemy;
        bool atkIsPlayer = (World.layer[atkIdx] & L_Player) != 0;
        if (!atkIsPlayer && IdxIsNPC(World.instances[atkIdx].index)) {
            NPCType mt = npcTable[World.instances[self].index - 419].type, at = npcTable[World.instances[atkIdx].index - 419].type;
            bool canFight = World.instances[atkIdx].index != World.instances[self].index;
            if ((mt == NPCType_Robot && World.instances[self].enemy) || ((mt == NPCType_Cyborg || mt == NPCType_Supercyborg || mt == NPCType_Robot) && (at == NPCType_Cyborg || at == NPCType_Supercyborg || at == NPCType_Robot))) canFight = false;            
            if (canFight) World.instances[self].enemy = atkIdx;
        } else World.instances[self].enemy = atkIdx;
        World.instances[self].posCheckFinished = World.pauseRelativeTime + AI_POS_CHECK_DELAY;
        flag_set(&World.instances[self].entflags, EF_WANDERING, false);
        World.instances[self].wanderFinished = World.pauseRelativeTime;
        World.instances[self].lastPosition = World.position[self];
        if (World.instances[self].enemy) { World.instances[self].lastKnownEnemyPos = World.instances[self].currentDestination = World.position[World.instances[self].enemy]; }
    }
    flag_set(&World.instances[self].entflags, EF_GO_INTO_PAIN, false);
    World.instances[self].timeTillPainFinished = World.pauseRelativeTime + npcTable[World.instances[self].index - 419].timeToPain;
    return true;
}

static void AIIdle(u16 sidx) {
    if (World.instances[sidx].enemy && ai_has_health(&World.instances[sidx])) { World.instances[sidx].currentState = AIState_Run; return; }
    NPCTable* npc = &npcTable[World.instances[sidx].index - 419];
    if (World.instances[sidx].idleTime < World.pauseRelativeTime) {
        int sidle = sfxIdle[World.instances[sidx].index - 419];
        if (random_range(0.0f, 1.0f) < 0.5f && sidle >= 0 && sidle < (i16)SOUNDS_COUNT) play_wav(sounds[sidle],World.instances[sidx].volume,World.position[sidx],true);
        World.instances[sidx].idleTime = World.pauseRelativeTime + random_range(npc->timeIdleSFXMin, npc->timeIdleSFXMax);
    }
    AICheckPain(sidx);
}

static V3 AIGetWanderPoint(Entity* self) {
    u16 sidx=(u16)(self - World.instances);
    return (V3){World.position[sidx].x + random_range(-AI_WANDER_RANGE,AI_WANDER_RANGE),ai_is_cyber(self) ? World.position[sidx].y + random_range(-AI_WANDER_RANGE,AI_WANDER_RANGE) : 0.0f,World.position[sidx].z + random_range(-AI_WANDER_RANGE,AI_WANDER_RANGE)};
}

static V3 AIGetAStarPoint(Entity* self) {
    u16 sidx=(u16)(self - World.instances);
    V3 ep = self->enemy ? World.position[self->enemy] : World.position[sidx];
    float px = World.position[sidx].x, py = World.position[sidx].y, pz = World.position[sidx].z;
    V3 cands[4] = {{px,py,pz + CELLSZ},{px,py,pz - CELLSZ},{px + CELLSZ,py,pz},{px - CELLSZ,py,pz}};
    int best = -1; float bestD = 1e9f;
    for (int i = 0; i < 4; ++i) {
        if (!PositionVisibleFromPlayerCell(cands[i].x, cands[i].z)) continue;
        
        float d = V3_SqDist(ep, cands[i]);
        if (d < bestD) { bestD = d; best = i; }
    }
    return best >= 0 ? cands[best] : AIGetWanderPoint(self);
}

static V3 AIGetSearchPoint(Entity* self) { NPCType t = npcTable[self->index - 419].type; if (t == NPCType_Mutant || t == NPCType_Supermutant) {return AIGetWanderPoint(self);} return AIGetAStarPoint(self); }
static void AIHopMove(u16 self) {
    if (World.instances[self].entflags & EF_ACT_AS_TURRET) return;
//     if (World.instances[self].animatorPlaybackTime > 0.1395f) { // TODO
//         if (!(World.instances[self].entflags & EF_HOP_DONE)) {
//             flag_set(&World.instances[self].entflags,EF_HOP_DONE,true);
//             AddForce(ai_self_idx(self),V3_ScaleByF(World.instances[self].forward,500.0f),true);
//             AddForce(ai_self_idx(self),(V3){0,5.0f,0}, true);
//         }
//     } else {
//         flag_set(&World.instances[self].entflags, EF_HOP_DONE, false);
//     }
}

static void AIWalk(u16 self) {
    if (AICheckPain(self)) return;
    if (World.instances[self].entflags & EF_ASLEEP) return;
    if ((World.instances[self].entflags & EF_ENEM_IN_SIGHT) || World.instances[self].enemy) { World.instances[self].currentState = AIState_Run; return; }
    if (World.instances[self].entflags & EF_ACT_AS_TURRET) { World.instances[self].currentState = AIState_Idle; return; }
    if (npcTable[World.instances[self].index - 419].moveType == AIMoveType_None) return;
    if (World.instances[self].tranquilizeFinished >= World.pauseRelativeTime) return;
    u16 sidx = self;
    if (!PositionVisibleFromPlayerCell(World.position[sidx].x,World.position[sidx].z)) return;
    float dist = V3_Dist(ai_sight_pos(&World.instances[self]),World.instances[self].currentDestination);
    if (World.instances[self].entflags & EF_WANDERING) {
        if (World.instances[self].wanderFinished < World.pauseRelativeTime || dist < AI_STOP_DIST * 0.5f) { World.instances[self].wanderFinished = World.pauseRelativeTime + random_range(3.0f, 8.0f); World.instances[self].currentDestination = AIGetWanderPoint(&World.instances[self]); }
    }
    if (dist > AI_STOP_DIST && AIWithinAngleToTarget(&World.instances[self])) {
        if (npcTable[World.instances[self].index - 419].hopsOnMove) {
            AIHopMove(self);
        } else {
            float ws  = npcTable[World.instances[self].index - 419].walkSpeed;
            V3 mv = { World.instances[self].forward.x*ws,World.instances[self].forward.y*ws,World.instances[self].forward.z*ws };
            if (npcTable[World.instances[self].index - 419].moveType != AIMoveType_Fly) {
                V3 spos = ai_sight_pos(&World.instances[self]);
                V3 cp = { spos.x + World.instances[self].forward.x*0.48f, spos.y, spos.z + World.instances[self].forward.z*0.48f };
                RaycastHit gh = Raycast(cp,(V3){0,-1,0},CELLSZ,LMASK_NPC_COLLISION);
                if (!gh.hit) { mv.x = 0.0f; mv.z = 0.0f; }
            }
            mv.y = World.velocity[sidx].y;
            World.velocity[sidx] = mv;
        }
        return;
    }
    if (!(World.instances[self].entflags & EF_WANDERING)) World.instances[self].currentState = AIState_Idle;
}

static void AIRunMove(u16 self) {
    if (World.instances[self].entflags & EF_ACT_AS_TURRET) return;
    
    float rs = npcTable[World.instances[self].index - 419].runSpeed;
    World.velocity[self] = (V3){World.instances[self].forward.x * rs,(vabs(World.gravity[self]) > 0.05f) ? World.velocity[self].y : World.instances[self].forward.y * rs,World.instances[self].forward.z * rs};
}

static void AIHunt(Entity* self) {
    u16 sidx=(u16)(self - World.instances);
    u16 eidx = self->enemy;
    if (!eidx) return;
    self->currentDestination = ai_is_cyber(self) ? World.position[eidx] : AIGetSearchPoint(self);
    if (npcTable[self->index - 419].moveType == AIMoveType_None) return;
    if (self->entflags & EF_ACT_AS_TURRET) return;
    if (npcTable[self->index - 419].runSpeed <= 0.0f) return;
    if (V3_SqDist(ai_sight_pos(self), self->currentDestination) <= AI_STOP_DIST_SQ) return;
    if (!AIWithinAngleToTarget(self)) return;
    float rs = npcTable[self->index - 419].runSpeed;
    World.velocity[sidx] = (V3){ self->forward.x*rs,World.velocity[sidx].y, self->forward.z*rs };
}

float DistToEnemy(u16 self, u16 enem) {
    if (self >= World.instCount) return 100000.0f;
    if (enem >= World.instCount) return 100000.0f;
    V3 selfPos = World.position[self], enemPos = World.position[enem];
    V3 d = V3_AsubB(selfPos,enemPos); return V3_dot(d,d);
}

static bool AICanAttack(u16 selfIdx, float dsq, u8 type, float* rangeToEnemy) {
    Entity* self = &World.instances[selfIdx];
    *rangeToEnemy = DistToEnemy(selfIdx,self->enemy);
    if (*rangeToEnemy >= dsq) return false;
    AttType att = type == 3 ? npcTable[self->index - 419].attackType3 : (type == 2 ? npcTable[self->index - 419].attackType2 : npcTable[self->index - 419].attackType);
    if (att == Att_None) return false;
    if (type == 3) {
        if (*rangeToEnemy < 7.0f && att == Att_Ball) {
            int p = npcTable[self->index - 419].projectile3Prefab;
            if (p == 370 || p == 372 || p == 387 || p == 404) return false;
        }
    }
    if (ai_is_cyber(self)) return true; // Cyber enemies are dumb but aggressive.
    if (!(self->entflags & EF_ENEM_IN_FRONT)) return false;
    if (type >= 2 && !(self->entflags & EF_ENEM_IN_FOV)) return false;
    float wait = type == 3 ? self->randWaitAtt3Finished : (type == 2 ? self->randWaitAtt2Finished : self->randWaitAtt1Finished);
    return wait < World.pauseRelativeTime;
}

static void AIBrakingMovement(Entity* self) { u16 sidx=(u16)(self - World.instances); u8 ni = self->index - 419; if (ni == 1 || (ni >= 3 && ni <= 9) || (ni >= 11 && ni <= 13) || ni == 17 || ni == 23) { World.velocity[sidx].x *= 0.15f; World.velocity[sidx].z *= 0.15f; } }
static void AIStartAttack(Entity* self, int n) {
    AIBrakingMovement(self);
    NPCTable* npc = &npcTable[self->index - 419];
    double between, toActual;
    switch (n) {
        case 1: between = npc->timeBetweenAttack1; toActual = npc->timeToActualAttack1; break;
        case 2: between = npc->timeBetweenAttack2; toActual = npc->timeToActualAttack2; break;
        default: between = npc->timeBetweenAttack3; toActual = npc->timeToActualAttack3; break;
    }
    self->attackFinished    = World.pauseRelativeTime + between + toActual;
    self->gracePeriodFinished = World.pauseRelativeTime + toActual;
    self->currentState = (AIState)(AIState_Attack1 + (n - 1));
}

static void AIRun(u16 selfIdx) {
    Entity* self = &World.instances[selfIdx];
    if (AICheckPain(selfIdx)) return;
    if (self->entflags & EF_ASLEEP) return;
    if (!self->enemy) { self->currentState = AIState_Idle; return; }
    if (self->tranquilizeFinished >= World.pauseRelativeTime && !ai_is_cyber(self)) return;
    if (self->posCheckFinished <= World.pauseRelativeTime && !ai_is_cyber(self)) {
        self->posCheckFinished = World.pauseRelativeTime + AI_POS_CHECK_DELAY;
        float dToEn = V3_Dist(ai_sight_pos(self),World.position[self->enemy]);
        float dToLast = V3_Dist(World.position[selfIdx],self->lastPosition);
        self->lastPosition = World.position[selfIdx];
        if (dToLast < 0.48f && dToEn > AI_STOP_DIST && !(self->entflags & EF_WANDERING)) {
            self->wanderFinished = World.pauseRelativeTime + 5.0f; // Same search time as Quake 1
            flag_set(&self->entflags,EF_WANDERING,true);
            self->currentDestination = AIGetSearchPoint(self);
        } else flag_set(&self->entflags,EF_WANDERING,false);
    }
    if (!(self->entflags & EF_ENEM_IN_SIGHT)) {
        if (self->huntFinished > World.pauseRelativeTime) { AIHunt(self); }
        else { self->enemy = 0; flag_set(&self->entflags,EF_WANDERING,true); self->wanderFinished = World.pauseRelativeTime + 1.0; self->currentState = AIState_Walk; }
        return;
    }
    if (self->enemy && !(self->entflags & EF_WANDERING)) {
        self->targettingPosition = (V3){World.position[self->enemy].x,World.position[self->enemy].y + AI_TARGET_OFFSET_Y,World.position[self->enemy].z};
        self->currentDestination = self->targettingPosition;
        self->lastKnownEnemyPos  = self->targettingPosition;
    }
    flag_set(&self->entflags, EF_SHOT_FIRED, false);
    AISetHuntFinished(selfIdx);
    NPCTable* ndat = &npcTable[self->index - 419];
    float nr = ndat->range, near = nr * nr;
    float mr = ndat->range2, mid  = mr * mr;
    float fr = ndat->range3, far  = fr * fr;
    float rangeToEnemy = 100000.0f;
    if (AICanAttack(selfIdx,near,1,&rangeToEnemy)) { AIStartAttack(self,1); return; }
    if (AICanAttack(selfIdx, mid,2,&rangeToEnemy)) { AIStartAttack(self,2); return; }
    if (AICanAttack(selfIdx, far,3,&rangeToEnemy)) { AIStartAttack(self,3); return; }
    if (ndat->moveType != AIMoveType_None && rangeToEnemy > AI_STOP_DIST_SQ) {
        if (AIWithinAngleToTarget(self)) {
            if (ndat->hopsOnMove) AIHopMove(selfIdx);
            else AIRunMove(selfIdx);
        } else if (World.diffCbt >= 2 && random_range(0.0f,1.0f) < 0.5f) {
            AIFace(self,self->currentDestination);
        }
    }
}

static void AIPain(Entity* self) { if (self->timeTillPainFinished < World.pauseRelativeTime) { self->currentState = AIState_Run; flag_set(&self->entflags, EF_GO_INTO_PAIN, false); self->timeTillPainFinished = World.pauseRelativeTime + npcTable[self->index - 419].timeBetweenPain; } }
static bool AIDeactivatesVisibleMeshWhileDying(Entity* self) { return self->index == 419 || self->index == 433 || self->index == 439 || (self->entflags & EF_TELEPORT_ON_DEATH); }
static void AIDying(u16 i) {
    if (!(World.instances[i].entflags & EF_DYING_SETUP)) {
        World.instances[i].enemy = 0;
        NPCTable* npc = &npcTable[World.instances[i].index - 419];
        float dbt = deathBurstTimer[World.instances[i].index - 419];
        if (dbt > 0.0f) { World.instances[i].deathBurstFinished = World.pauseRelativeTime + dbt; }
        else if (!(World.instances[i].entflags & EF_DEATH_BURST_DONE)) { /*TODO Enable deathburst effects*/ }
        u16 sidx = i;
        if (!(World.instances[i].entflags & EF_ACT_AS_CORPSE_ONLY) && !(World.instances[i].entflags & EF_TELEPORT_ON_DEATH)) {
            int sded = sfxDeath[World.instances[i].index - 419];
            if (sded >= 0 && sded < (i16)SOUNDS_COUNT) play_wav(sounds[sded],World.instances[i].volume,World.position[sidx],true);
        }
        World.gravity[i] = ai_is_cyber(&World.instances[i]) ? 0.0f : 1.0f; // Physics for death
        flag_set(&World.instances[i].entflags,EF_ASLEEP,false);
        World.layer[i] = L_Corpse;
        flag_set(&World.instances[i].entflags,EF_FIRST_SIGHTING,true);
        World.instances[i].timeTillDeadFinished = World.pauseRelativeTime + npc->timeTillDead;
    //     if (npc->switchMaterialOnDeath && World.instances[i].dyingTexture) World.instances[i].texIndex = World.instances[i].dyingTexture; // TODO Handle hopper and zerog texture changes
        if (World.instances[i].index == 428 || World.instances[i].index == 439) World.velocity[sidx] = (V3){0.0f,World.velocity[sidx].z,0.0f}; // Index-specific velocity patch (Exec bot and Zero-G mutant)
        if (World.instances[i].index == 433) World.layer[i] = L_Corpse; // Hopper: enable capsule collider (implicit in layer change)
        flag_set(&World.instances[i].entflags, EF_DYING_SETUP, true);
    }
    if (World.instances[i].timeTillDeadFinished < World.pauseRelativeTime) { flag_set(&World.instances[i].entflags,EF_DEAD,true); flag_set(&World.instances[i].entflags,EF_DYING,false); World.instances[i].currentState = AIState_Dead; }
    if (AIDeactivatesVisibleMeshWhileDying(&World.instances[i])) World.instances[i].modelIndex = MAX_MDLS;
    if (World.instances[i].index == 439) World.layer[i] = L_Corpse | L_CorpseSearchable; // Zero-G mutant enables search collider while still dying
}

static void AIDead(u16 idx) {
    Entity* self = &World.instances[idx];   
    flag_set(&World.instances[idx].entflags, EF_ASLEEP,       false);
    flag_set(&World.instances[idx].entflags, EF_DEAD,         true);
    flag_set(&World.instances[idx].entflags, EF_DYING,        false);
    flag_set(&World.instances[idx].entflags, EF_DYING_SETUP,  false);
    if (World.instances[idx].entflags & EF_DEAD_CHECKS_DONE) return;
    if (AIDeactivatesVisibleMeshWhileDying(self)) World.instances[idx].modelIndex = MAX_MDLS;
    World.instances[idx].currentState = AIState_Dead;
    World.layer[idx] = L_Corpse;
    if (World.instances[idx].entflags & EF_TELEPORT_ON_DEATH) {
        World.gravity[idx] = 1.0f;
        World.instances[idx].modelIndex = MAX_MDLS;
        // TODO: TeleportAway(ai_self_idx(self)), DeleteInstance(idx);
    } else if (ai_is_cyber(self)) {
        World.gravity[idx] = 0.0f;
        World.instances[idx].modelIndex = MAX_MDLS;
        // TODO: Gib(ai_self_idx(self)) — spawn gibs
        DeleteInstance(idx);
    } else {
        // Enable search collider for non-gib corpses (Avian Mutant index 2 always searchable)
        World.layer[idx] = L_Corpse | L_CorpseSearchable;
        World.velocity[idx].x = 0.0f; World.velocity[idx].z = 0.0f;
        if (World.instances[idx].index != 433) World.gravity[idx] = 1.0f;// Hopper deactivates itself
    }
    flag_set(&World.instances[idx].entflags, EF_DEAD_CHECKS_DONE, true);
}

static DamageData SetNPCData(Entity* self, int n) {
    DamageData dd = {0};
    NPCTable* npc = &npcTable[self->index - 419];
    dd.owner = (u16)(self - World.instances);
    switch (n) {
        case 1: dd.damage = npc->damage;  dd.attackType = npc->attackType;  break;
        case 2: dd.damage = npc->damage2; dd.attackType = npc->attackType2; break;
        default: dd.damage = npc->damage3; dd.attackType = npc->attackType3; break;
    }
    
    dd.penetration = 0;
    dd.defense = 0;
    return dd;
}

static float ai_damage_take_amount(DamageData dd) { float reduction = dd.defense / (dd.defense + dd.offense + 1.0f); return dd.damage * (1.0f - reduction); } // TODO: refine formula when HealthManager is ported
static void ai_apply_damage(DamageData dd, u16 hitIdx) {
    if (!hitIdx || hitIdx >= INSTANCE_COUNT){return;}
    dd.hitIdx=hitIdx; dd.damage=ai_damage_take_amount(dd);
    if (hitIdx == PLAYER1) { PlayerTakeDamage(hitIdx, dd.damage); } else { Entity* t=&World.instances[hitIdx]; t->health-=dd.damage; if (t->health < 0.0f){t->health = 0.0f;} t->recentMostActivator=dd.owner; flag_set(&t->entflags,EF_GO_INTO_PAIN,true); }
}

static void AIApplyAttackMovement(Entity* self, float speed) {
    u16 eidx = self->enemy;
    if (!eidx) return;
    if (self->entflags & EF_ACT_AS_TURRET) { self->currentDestination = ai_sight_pos(self); return; }
    if (speed <= 0.0f || self->tranquilizeFinished >= World.pauseRelativeTime) return;
    self->currentDestination = World.position[eidx];
    if (V3_SqDist(ai_sight_pos(self), self->currentDestination) <= AI_STOP_DIST_SQ) return;
    if (!AIWithinAngleToTarget(self)) return;
    AddForce((u16)(self - World.instances), V3_ScaleByF(self->forward, speed), false);
}

static void AITransitionAttackToRun(Entity* self, int n) {
    flag_set(&self->entflags, EF_GO_INTO_PAIN, false);
    self->currentState = AIState_Run;
    NPCTable* npc = &npcTable[self->index - 419];
    float chance, wmin, wmax;
    float* wait;
    switch (n) {
        case 1:  chance=npc->timeAttack1WaitChance; wmin=npc->timeAttack1WaitMin; wmax=npc->timeAttack1WaitMax; wait=&self->randWaitAtt1Finished; break;
        case 2:  chance=npc->timeAttack2WaitChance; wmin=npc->timeAttack2WaitMin; wmax=npc->timeAttack2WaitMax; wait=&self->randWaitAtt2Finished; break;
        default: chance=npc->timeAttack3WaitChance; wmin=npc->timeAttack3WaitMin; wmax=npc->timeAttack3WaitMax; wait=&self->randWaitAtt3Finished; break;
    }
    *wait = (random_range(0.0f, 1.0f) < chance) ? World.pauseRelativeTime + random_range(wmin, wmax) : World.pauseRelativeTime;
}

static void MuzzleBurst(Entity* self, int attackNum) { // TODO Table of muzzleBurst entity indices
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
    V3 spos = (n == 1) ? ai_sight_pos(self) : ai_gun_pos(self, n);
    u16 eidx = self->enemy;
    V3 targ = eidx ? self->targettingPosition : (V3){spos.x + self->forward.x*10.0f, spos.y, spos.z + self->forward.z*10.0f};
    V3 dir  = (n == 1) ? self->forward : V3_Normalize(V3_AsubB(targ, spos));
    float range;
    switch (n) {
        case 1: range = npcTable[self->index - 419].range; break;
        case 2: range = npcTable[self->index - 419].range2; break;
        default: range = npcTable[self->index - 419].range3; break;
    }
    MuzzleBurst(self,n);
    RaycastHit hit = Raycast(spos, dir, range, LMASK_NPC_ATTACK);
    if (!hit.hit) return;
    u16 hi = hit.hitInstanceIndex;
    if (n == 3 && self->index == 427 && eidx) AddWireLine(ai_sight_pos(self), World.position[eidx],(Color){1.0f,0.15f,0.18f,0.85f}); // Targeting laser (Cyborg Elite, attack3)
    DamageData dd = SetNPCData(self, n);
    dd.hitpoint     = hit.point; dd.attacknormal = dir; dd.impactVelocity = dd.damage;
    bool hitPlayer = (hi == PLAYER1);
    if (hitPlayer) dd.impactVelocity *= 0.5f;
    dd.isOtherNPC = !hitPlayer && IdxIsNPC(World.instances[hi].index);
    if (hi) ai_apply_damage(dd, hi);
    u16 impactCI = GetImpactType(hi);
    if (impactCI) { u16 imp = SpawnDynamicObject(impactCI, true); if (imp && imp < INSTANCE_COUNT){World.position[imp]=hit.point;} }
}

static void ProjectileLaunched(Entity* self, int n) {
    u16 sidx=(u16)(self - World.instances);
    NPCTable* npc = &npcTable[self->index - 419];
    int masterIdx; float launchSpd;
    switch (n) {
        case 1: masterIdx = npc->projectile1Prefab; launchSpd = npc->projectileSpeedAttack1; break;
        case 2: masterIdx = npc->projectile2Prefab; launchSpd = npc->projectileSpeedAttack2; break;
        default: masterIdx = npc->projectile3Prefab; launchSpd = npc->projectileSpeedAttack3; break;
    }
    V3 spos = ai_gun_pos(self,n); u16 eidx = self->enemy;
    V3 targ = eidx ? self->targettingPosition : (V3){spos.x + self->forward.x*20.0f, spos.y, spos.z + self->forward.z*20.0f};
    V3 dir  = V3_Normalize(V3_AsubB(targ, spos));
    MuzzleBurst(self,n);
    u16 bb = SpawnDynamicObject((u16)masterIdx,false);
    if (!bb || bb >= INSTANCE_COUNT) bb = SpawnDynamicObject(370,false); // TODO validate in arg without double calling SpawnDynamicObject
    if (!bb || bb >= INSTANCE_COUNT) return;
    Entity* proj   = &World.instances[bb];
    World.layer[bb] = L_NPCBullet;
    World.position[bb] = spos;
    proj->forward  = dir;
    // TODO: store damage data into projectile entity fields for deferred impact
    V3 shove = V3_ScaleByF(dir, launchSpd);
    if (vabs(World.gravity[sidx]) > 0.05f) { shove.x += World.velocity[sidx].x; shove.z += World.velocity[sidx].z; }
    World.velocity[bb] = (V3){0,0,0};
    AddForce(bb,shove,true);
    flag_set(&proj->entflags,EF_ACTIVE | EF_RIGIDBODY,true);
}

static void AIExplodeAttack(Entity* self) {
    float radius = npcTable[self->index - 419].attack3Radius; float force  = npcTable[self->index - 419].attack3Force;
    V3 epos = ai_sight_pos(self);
    DamageData dd = SetNPCData(self, 3);
    for (u16 i = INSTS_1ST_IDX; i < World.instCount; ++i) {
        Entity* t = &World.instances[i];
        if (!(t->entflags & EF_ACTIVE)) continue;
        float dsq = V3_SqDist(epos,World.position[i]);
        if (dsq >= radius * radius) continue;
        float dist = vsqrtf(dsq), falloff = 1.0f - dist / radius;
        DamageData tdd = dd; tdd.damage *= falloff;
        ai_apply_damage(tdd, i);
        if (dist > 0.001f) AddForce(i,V3_ScaleByF(V3_Normalize(V3_AsubB(World.position[i],epos)),force * falloff),true);
    }
    self->health = 0.0f; // Self-destruct
}

static void AIMakeAttack(Entity* self, AttType att, int ind) {
    if (ind < 1 || ind > 3) ind = 1; // Melee hitscan by default.
    switch (att) {
        case Att_Melee:              ProjectileRaycast(self,ind);  break;
        case Att_HitS:         ProjectileRaycast(self,ind);  World.fogFac += 1; break;
        case Att_Ball: ProjectileLaunched(self,ind); World.fogFac += 1; break;
        default: break;
    }
}

static void AIAttack1(Entity* self) {
    u16 sidx=(u16)(self - World.instances);
    NPCTable* npc = &npcTable[self->index - 419];
    AIApplyAttackMovement(self, npc->attack1Speed);
    if (self->gracePeriodFinished < World.pauseRelativeTime && !(self->entflags & EF_SHOT_FIRED)) {
        flag_set(&self->entflags, EF_SHOT_FIRED, true);
        int sat = sfxAttack1[self->index - 419];
        if (self->attack1SoundTime < World.pauseRelativeTime && sat >= 0 && sat < (i16)SOUNDS_COUNT) { play_wav(sounds[sat], self->volume,World.position[sidx],true); self->attack1SoundTime = World.pauseRelativeTime + npc->timeBetweenAttack1; }
        AIMakeAttack(self, npc->attackType, 1);
    }
    if (self->attackFinished < World.pauseRelativeTime) AITransitionAttackToRun(self, 1);
}

static void AIAttack2(Entity* self) {
    u16 sidx=(u16)(self - World.instances);
    NPCTable* npc = &npcTable[self->index - 419];
    AIApplyAttackMovement(self, npc->attack2Speed);
    if (self->gracePeriodFinished < World.pauseRelativeTime && !(self->entflags & EF_SHOT_FIRED)) {
        flag_set(&self->entflags, EF_SHOT_FIRED, true);
        int sat2 = sfxAttack2[self->index - 419];
        if (self->attack2SoundTime < World.pauseRelativeTime && sat2 >= 0 && sat2 < (i16)SOUNDS_COUNT) { play_wav(sounds[sat2],self->volume,World.position[sidx],true); self->attack2SoundTime = World.pauseRelativeTime + npc->timeBetweenAttack2; }
        AIMakeAttack(self,npc->attackType2,2);
    }
    if (self->attackFinished < World.pauseRelativeTime) AITransitionAttackToRun(self, 2);
}

static void AIAttack3(Entity* self) {
    u16 sidx=(u16)(self - World.instances);
    NPCTable* npc = &npcTable[self->index - 419];
    if (npc->explodeOnAttack3) { World.fogFac += 5; AIExplodeAttack(self); return; }
    AIApplyAttackMovement(self, npc->attack3Speed);
    if (self->gracePeriodFinished < World.pauseRelativeTime && !(self->entflags & EF_SHOT_FIRED)) {
        flag_set(&self->entflags, EF_SHOT_FIRED, true);
        int sat3 = sfxAttack3[self->index - 419];
        if (self->attack3SoundTime < World.pauseRelativeTime && sat3 >= 0 && sat3 < (i16)SOUNDS_COUNT) { play_wav(sounds[sat3],self->volume,World.position[sidx],true); self->attack3SoundTime = World.pauseRelativeTime + npc->timeBetweenAttack3; }
        AIMakeAttack(self, npc->attackType3, 3);
    }
    if (self->index == 427 && self->enemy) AddWireLine(ai_sight_pos(self),World.position[self->enemy],(Color){1.0f,0.15f,0.18f,0.85f});
    if (self->index == 433 && self->enemy) AddWireLine(ai_sight_pos(self),World.position[self->enemy],(Color){0.96f,1.0f,0.0f,0.88f});
    if (self->attackFinished < World.pauseRelativeTime) AITransitionAttackToRun(self, 3);
}

static void AIFlierMoveToHoverHeight(Entity* self) {
    u16 sidx=(u16)(self - World.instances);
    NPCTable* npc = &npcTable[self->index - 419];
    if (npc->runSpeed <= 0.0f) return;
    u16 eidx = self->enemy;
    if (eidx) { self->idealPos.y = World.position[eidx].y + AI_TARGET_OFFSET_Y; self->idealPos.x = World.position[sidx].x; self->idealPos.z = World.position[sidx].z; }
    else {
        V3 sp = ai_sight_pos(self); RaycastHit dn = Raycast(sp,(V3){0,-1,0},npc->sightRange,LMASK_NPC_SIGHT); RaycastHit up = Raycast(sp,(V3){0, 1,0},npc->sightRange,LMASK_NPC_SIGHT);
        float dDn = dn.hit ? dn.distance : 0.0f, dUp = up.hit ? up.distance : 0.0f; float yH  = npc->flightHeight * (npc->flightHeightIsPercentage ? dDn + dUp : 1.0f);
        V3 fp = dn.hit ? dn.point : World.position[sidx]; self->idealPos = (V3){ fp.x, fp.y + yH, fp.z };
    }
    float dy = self->idealPos.y - World.position[sidx].y; if (vabs(dy) < 0.16f) return;
    float spd  = npc->runSpeed * (float)World.deltaTime;
    float step = vmin(vabs(dy), spd) * (dy < 0.0f ? -1.0f : 1.0f);
    World.position[sidx].y += step;
}

float AITranquilize(u16 idx, float amount, bool energy) { Entity* self = &World.instances[idx]; float secs = (amount < 3.0f) ? (float)npcTable[self->index - 419].timeForTranquilization : amount; if (npcTable[self->index - 419].type != NPCType_Robot || energy) { double a = World.pauseRelativeTime + secs, b = self->tranquilizeFinished + secs; self->tranquilizeFinished = a > b ? a : b; return secs; } return 0.0f; }
void AIAlert(u16 idx) { if (!World.diffCbt){return;} Entity* self = &World.instances[idx]; AISetEnemy(idx,PLAYER1); self->currentDestination = World.position[PLAYER1]; flag_set(&self->entflags, EF_ENEM_IN_SIGHT, false); }
void AIAwakeFromSleep(u16 idx) { Entity* self = &World.instances[idx]; flag_set(&self->entflags, EF_ASLEEP, false); AIAlert(idx); } // TODO deactivate sleeping cables
static void AIThink(u16 idx) {
    Entity* self = &World.instances[idx];   
    if ((self->entflags & EF_DYING_SETUP) && self->deathBurstFinished < World.pauseRelativeTime && !(self->entflags & EF_DEATH_BURST_DONE)) { flag_set(&self->entflags, EF_DEATH_BURST_DONE, true); } // TODO activate death burst effect
    if (!ai_has_health(self)) {
             if (!(self->entflags & EF_DYING) && !(self->entflags & EF_DEAD)) { flag_set(&self->entflags, EF_DYING, true); self->currentState = AIState_Dying; }
        else if ((self->entflags & EF_DEAD) && self->currentState != AIState_Dead) { self->currentState = AIState_Dead; }
        else if ((self->entflags & EF_DYING) && self->currentState != AIState_Dying) { self->currentState = AIState_Dying; }
    }
    switch (self->currentState) {
        case AIState_Idle:    AIIdle(idx);    break;
        case AIState_Walk:    AIWalk(idx);    break;
        case AIState_Run:     AIRun(idx);     break;
        case AIState_Attack1: AIAttack1(self); break;
        case AIState_Attack2: AIAttack2(self); break;
        case AIState_Attack3: AIAttack3(self); break;
        case AIState_Pain:    AIPain(self);    break;
        case AIState_Dying:   AIDying(idx);   break;
        case AIState_Dead:    AIDead(idx);    break;
        default:              AIIdle(idx);    break;
    }
    if (self->currentState == AIState_Dead || self->currentState == AIState_Dying) return;
}

void AIControllerUpdate(u16 idx) {
    if (!(World.instances[idx].entflags & EF_ACTIVE)) return;
    if (!ai_is_cyber(&World.instances[idx]) && npcTable[World.instances[idx].index - 419].moveType != AIMoveType_Fly && World.instances[idx].currentState != AIState_Dead && World.instances[idx].currentState != AIState_Dying) World.gravity[idx] = 1.0f;
    flag_set(&World.instances[idx].entflags,EF_ENEM_IN_SIGHT,AICheckIfPlayerInSight(idx));
    u16 eidx = World.instances[idx].enemy;
    if (eidx && ai_has_health(&World.instances[idx])) {
        bool enAlive = ai_is_cyber(&World.instances[idx]) ? World.instances[eidx].cyberHealth > 0.0f : World.instances[eidx].health > 0.0f;
        if (!enAlive) {
            if (ai_is_cyber(&World.instances[idx])) { World.instances[idx].currentState = AIState_Idle; }
            else { flag_set(&World.instances[idx].entflags, EF_WANDERING, true); World.instances[idx].wanderFinished = World.pauseRelativeTime + random_range(3.0f, 8.0f); World.instances[idx].currentState = AIState_Walk; }
            World.instances[idx].enemy = 0; World.instances[idx].posCheckFinished = World.pauseRelativeTime; World.instances[idx].lastPosition = World.position[idx];
        } else AIEnemyInFrontChecks(&World.instances[idx],eidx);
    }
    AIThink(idx);
    if (World.instances[idx].currentState != AIState_Dead && World.instances[idx].currentState != AIState_Idle) {
        if ((World.instances[idx].entflags & EF_ACT_AS_TURRET) && eidx) { World.instances[idx].currentDestination = (V3){World.position[eidx].x,World.position[eidx].y + AI_TARGET_OFFSET_Y,World.position[eidx].z}; }
        if (ai_is_cyber(&World.instances[idx]) && eidx) World.instances[idx].currentDestination = World.position[eidx];
        V3 toTarget = V3_AsubB(World.instances[idx].currentDestination,ai_sight_pos(&World.instances[idx]));
        if (!ai_is_cyber(&World.instances[idx])) toTarget.y = 0.0f;
        World.instances[idx].idealTransformForward = V3_Normalize(toTarget);
        float sqmag = V3_dot(toTarget, toTarget);
        if (sqmag > 1e-6f || ai_is_cyber(&World.instances[idx])) AIFace(&World.instances[idx],World.instances[idx].currentDestination);
    }
    if (npcTable[World.instances[idx].index - 419].moveType == AIMoveType_Fly && World.instances[idx].tranquilizeFinished < World.pauseRelativeTime) AIFlierMoveToHoverHeight(&World.instances[idx]);
}
