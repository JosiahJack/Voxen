// ai.c - NPC AI logic, ported from Unity Citadel AIController.cs.
#include "common.h"
static const float AI_STOP_DIST=1.28f, AI_STOP_DIST_SQ=(AI_STOP_DIST * AI_STOP_DIST), AI_POS_CHECK_DELAY=2.0f, AI_WANDER_RANGE=79.0f, AI_TARGET_OFFSET_Y=0.24f, AI_TICK_TIME=0.1f, AI_RAYCAST_TICK_TIME=0.2f; u16 npcCountInWorldPerType[NUM_AI_TYPES]; void DoorActuate(u16 self); void TextureSequenceStart(u16 self, u8 clipIndex); Quaternion quat_normalize(Quaternion q); bool PositionVisibleFromPlayerCell(float,float); bool XZPairInBounds(i32,i32); u16 AddLightSimple(V3,Color3,float,float,u16);
// Name,AtkTyp1,2,3,Dmg1,2,3,Range1,2,3,Health,CybHealth,Percp,Disrp,Armr,Def,Movtyp,Yawspd,FOV,FOVAtk,FOVStartMov,DistToSeeBehind,SightRange,WalkSpd,RunSpd,AtkSpd1,2,3,AtkForce3,AtkRad3,TtPain,TbwPain,TtDead,TtActualAtk1,2,3,TbwAtk1,2,3,TEnemChg,TIdleSFXMin,TIdleSFXMax,TAtk1WaitMin,TAtk1WaitMax,TAtk1WaitChnc,TAtk2WaitMin,TAtk2WaitMax,TAtk2WaitChnc,TAtk3WaitMin,TAtk3WaitMax,TAtk3WaitChnc,ProjType1,2,3,ProjSpd1,2,3,HasLaser1,2,3,ExplodeOn3,PreActMeleCols,THunt,FlightHeight,FlightHeightIsPerc,SwitchMatOnDie,RangeHear,TTranq,Hops,NPCType,AtkProj1,2,3
NPCTable npcTable[NUM_AI_TYPES] = {
/* 0*/{"AUTOBOMB"              ,0,0,1,  0,  0,200,   0,    0,2.4,50,0,1,0.5,40,1,1,300,180,120,55,3.84,50,2.5,2.5,0,0,0,100,6,0,0,0.1,0,0,0,0,0,0,3,5,12,0.5,1,0.1,1,3,0.5,0,0,0,0,0,0,0,0,0,0,0,0,1,0,20,0,0,0,10,3,0,2,0,0,0 },
/* 1*/{"CYBORG ASSASSIN"       ,0,4,7, 30, 50, 35, 3.3,   10,20,65,0,2,0.6,5,4,1,180,180,80,15,3.2,50,2,2,0,0,0,0,0,0.45,5,2.083,0,0.25,0.2,0.91,0.91,1.58,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,0,3,0,0,0,0,0,60,0,0,0,10,3,0,3,0,0,489 },
/* 2*/{"AVIAN MUTANT"          ,1,0,0, 40, 40,  0, 3.3,   10,20,125,0,1,0.25,0,2,2,180,180,80,15,5.12,50,2,2,3.5,0,0,0,0,2,5,1,0.1,0,0,1,0,0,3,5,12,0.5,1,0.1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,60,0.65,1,0,10,3,0,1,0,0,0 },
/* 3*/{"EXEC-BOT"              ,0,4,0, 30, 35,  0, 3.3,   10,20,225,0,1,0.2,40,2,1,200,180,15,30,4.12,50,1.5,1.5,0,0,0,0,0,0.45,7,0.15,0,0.2,0,0,1.5,0,3,5,12,0,0,0,0.97,2,0.3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,180,0,0,0,10,3,0,2,0,0,0 },
/* 4*/{"CYBORG DRONE"          ,0,4,0, 20, 20, 20, 3.3,   25,50,60,0,1,0.3,0,2,1,65,180,80,15,3.2,50,1.6,2.2,0,0,0,0,0,0.542,15,0.958,0,0.1,0,0,1,0,3,20,45,0,0,0,1,2,0.5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,60,0,0,0,10,3,0,3,0,0,0 },
/* 5*/{"CORTEX REAVER"         ,0,4,7, 80,325,125, 3.3,   20,30,580,0,1,0.1,40,2,1,180,180,80,15,3.84,50,2,2,0,0,0,0,0,0.583,5,0.333,0,0.35244,0.324,0,1,1,3,15,30,0,0,0,0.2,1,0.5,8,15,1,0,0,0,0,0,10,0,0,0,0,0,600,0,0,0,10,3,0,2,0,0,372 },
/* 6*/{"CYBORG WARRIOR"        ,0,4,7, 35, 35,150, 3.3,   20,20,120,0,1,0.1,5,4,1,180,180,30,15,3.2,50,2.4,2.4,0,0,0,0,0,0.5,5,2.2,0,0.339,0.201,0,0.83,0.542,3,15,30,0,0,0,1,2,0.5,10,20,1,0,0,0,0,0,10,0,0,0,0,0,180,0,0,0,10,3,0,3,0,0,370 },
/* 7*/{"CYBORG ENFORCER"       ,1,4,7, 60, 60, 80, 3.3,   15,30,285,0,1,0.1,30,5,1,180,180,80,15,3.2,50,2.8,2.8,2.8,0,0.3,0,0,2,5,1.5,0.23471,0.393738,0.313266,0.958,0.958,0.958,5,15,30,0.1,0.3,0.1,0.1,0.5,0.5,10,25,1,0,0,0,0,0,10,0,0,0,0,0,600,0,0,0,10,3,0,4,0,0,387 },
/* 8*/{"CYBORG ELITE GUARD"    ,1,7,4, 70, 75,  0, 3.3,   10,50,380,0,1,0.05,50,6,1,180,180,80,15,3.2,50,3,3,1.5,0,0,0,0,0.4665,5,1.5,0.5,0.2653,0.117045,0.733,0.7,0.867,5,15,30,0.05,0.2,0.1,0.5,2,0.8,2,3,0.5,0,0,0,0,2,0,0,1,0,0,0,600,0,0,0,15,3,0,4,0,490,0 },
/* 9*/{"CYBORG OF EDWARD DIEGO",1,7,0, 80, 95,  0, 3.3,   40,50,900,0,2,0,55,6,1,180,180,80,15,3.2,50,2.8,2.8,0,0,0,0,0,0,0,0,0.28,0.363188,0.2,1.4,0.833,3,5,15,30,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,2.5,0,0,0,0,0,1,600,0,0,0,15,3,0,4,0,490,0 },
/*10*/{"SECURITY-1 ROBOT"      ,0,4,0, 35, 35,  0, 3.3,   10,20,170,0,1,0.15,40,4,2,180,180,80,15,4.12,50,2.5,2.5,1.5,0,0,0,0,2,5,0.05,0.5,0.1,0.2,1.2,1.5,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,0,0,0,0,0,0,0,600,1.28,0,0,10,3,0,2,0,0,0 },
/*11*/{"SECURITY-2 ROBOT"      ,0,4,4, 65, 65, 15, 3.3,    5,35,300,0,2,0.05,50,5,1,180,180,60,25,4.12,50,1.5,1.5,1.5,0,0,0,0,0.75,5,0.25,0.5,0.39,0.1,1.2,1,1.5,3,5,12,0.5,1,0.1,3,3.5,1,2.5,3.5,1,0,0,0,0,0,0,0,0,0,0,0,600,0,0,0,10,3,0,2,0,0,0 },
/*12*/{"MAINTENANCE ROBOT"     ,1,0,0, 25, 25,  0, 3.3,  3.3,20,75,0,1,0.3,40,3,1,180,180,80,15,3.84,50,2.2,2.6,0.02,0.02,0,0,0,0,0,1.6,3,0.7,0.2,2,1.3,3,3,5,12,0.5,1,0.1,1,2,0.3,1,2,0.5,0,0,0,0,0,0,0,0,0,0,0,180,0,0,0,10,3,0,2,0,0,0 },
/*13*/{"MUTANT CYBORG"         ,1,7,0, 35, 75, 50,   2,   30,49,340,0,1,0.2,15,6,1,180,180,60,15,3.2,50,1.5,1.5,0,0,0,0,0,0.583,3.5,3.41,0.265,0.285,0.2,0.625,0.75,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,2.8,0,0,0,0,0,0,180,0,0,0,10,3,0,6,0,491,0 },
/*14*/{"HOPPER"                ,0,4,0, 35, 35,  0,   0,17.92,17.92,150,0,1,0.25,35,4,1,180,160,80,15,3.84,50,7,7,0,0,0,0,0,0.708,5,0,0.5,0.1,0.5,0.5,0.5,0.5,3,5,12,0.5,1,0.1,0.5,1,0.5,1,2,0.5,0,0,0,0,0,0,0,1,0,0,0,180,0,0,0,10,3,1,2,0,0,0 },
/*15*/{"HUMANOID MUTANT"       ,1,0,0, 12, 12,  0, 3.3,   10,20,50,0,0,0.4,0,3,1,60,180,80,15,2.56,50,1.4,2,0.5,0,0,0,0,0.42,5,0.967,0.5,0.1,0.2,1.2,1.5,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,0,0,0,0,0,0,0,20,0,0,0,10,3,0,0,0,0,0 },
/*16*/{"INVISIBLE MUTANT"      ,0,7,0, 10, 35,  0, 3.3,   20,20,350,0,1,0.05,0,2,2,180,180,80,15,2.56,50,0.7,0.7,1.5,0.7,0.7,0,0,0.875,5,1.125,0.875,0.4,0.2,1.2,0.875,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,2,0,0,0,0,0,0,60,0.32,0,1,10,3,0,0,0,486,0 },
/*17*/{"VIRUS MUTANT"          ,0,7,0, 45, 30,  0, 3.3,   20,20,140,0,0,0.1,0,3,1,180,180,80,15,2.56,50,2.5,2.5,2.5,0.3,0,0,0,0.542,3,1.792,0.2874,0.2874,0.2874,0.958,0.958,0.958,3,5,12,0.5,1,0.1,0.5,0,0.5,1,2,0.5,0,0,0,0,1.75,0,0,0,0,0,0,20,0,0,0,10,3,0,1,0,481,0 },
/*18*/{"SERV-BOT"              ,1,0,0,  8,  0,  0, 3.3,   10,20,20,0,1,0.5,20,2,1,180,180,80,15,3.84,50,2,2,1.2,0,0,0,0,1.125,2,0.98,0.2,0.1,0.2,0.834,1.5,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,0,0,0,0,0,0,0,180,0,0,0,10,3,0,2,0,0,0 },
/*19*/{"FLIER BOT"             ,0,4,7, 30,150,  0, 3.3,   35,40,75,0,1,0.3,30,2,2,180,180,80,15,5.12,50,1.5,1.5,1.5,0,0,0,0,1.375,5,0.6,0.1,0.1,0.2,1,1.5,3,3,5,12,0.5,1,0.1,1,2,0.5,10,12,1,0,0,0,0,0,10,0,0,0,0,0,180,0.85,1,0,10,3,0,2,0,0,404 },
/*20*/{"ZERO-G MUTANT"         ,0,7,0, 20, 20,  0, 3.3,   20,20,90,0,1,0.5,0,2,2,180,180,80,15,2.56,50,0.8,1.4,0,0.8,0,0,0,0.1,0,0.1,0.5,0.05,0.2,1.2,1.5,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,2,0,0,0,0,0,0,60,1.96,0,0,10,3,0,0,0,488,0 },
/*21*/{"GORILLA TIGER MUTANT"  ,1,0,0, 60, 60,  0, 3.3, 3.84,20,200,0,1,0.1,0,3,1,180,180,80,15,2.56,50,3,3.5,1,2,0,0,0,0.667,5,1.625,0.5,0.1,0.2,0.958,1.042,3,3,15,30,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,0,0,0,0,0,0,0,60,0,0,0,10,3,0,1,0,0,0 },
/*22*/{"REPAIR BOT"            ,0,4,0, 12, 12,  0, 3.3,  3.3,20,65,0,1,0.4,25,3,1,180,180,80,15,3.84,50,2.25,3,0.5,0,0,0,0,0,0,0.05,0.2,0.1,0.2,1.25,1.5,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,0,0,0,0,0,0,0,180,0,0,0,10,3,0,2,0,0,0 },
/*23*/{"PLANT MUTANT"          ,0,7,0, 35, 25,  0, 3.3,   20,20,115,0,1,0.3,0,1,1,180,180,80,15,2.56,50,0.8,1.2,0.1,0,0,0,0,0.375,2,2.208,0.89,0.82,0.2,1.91,1.027,3,3,5,12,0.5,1,0.1,1,2,0.5,1,2,0.5,0,0,0,0,3.5,0,0,0,0,0,0,20,0,0,0,10,3,0,0,0,487,0 },
/*24*/{"CYBER DOG"             ,0,7,0,  0, 25,  0,   0,   20,0,0,20,1,0.5,0,1,4,250,240,50,15,20.48,25.6,2,2,0,0,0,0,0,0.1,0,0.5,0,0,0,0,0.3,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1.5,0,0,0,0,0,0,500,0.75,0,0,10,0,0,5,0,493,0 },
/*25*/{"CYBER GUARD"           ,0,7,0,  0, 25,  0,   0,   20,0,0,35,1,0.4,0,1,4,250,240,50,15,20.48,25.6,2,2,0,0,0,0,0,0.1,0,0.5,0,0,0,0,0.2,0,2,998,999,0,0,0,0,0,0,0,0,0,0,0,0,0,0.8,0,0,0,0,0,0,500,0.75,0,0,10,0,0,5,0,493,0 },
/*26*/{"CYBER RAM"             ,0,7,0,  0, 35,  0,   0,   20,0,0,40,1,0.25,0,1,4,80,240,50,15,20.48,25.6,4,4,0,0,0,0,0,0.1,0,0.5,0,0,0,0,0.2,0,2,998,999,0,0,0,0,0,0,0,0,0,0,0,0,0,1.2,0,0,0,0,0,0,500,0.75,0,0,10,0,0,5,0,494,0 },
/*27*/{"CYBER CORTEX REAVER"   ,0,7,0,  0, 45,  0,   0,   20,0,0,80,1,0.1,0,1,4,80,240,50,15,20.48,25.6,4,4,0,0,0,0,0,0.1,0,0.5,0,0,0,0,0.2,0,2,998,999,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,500,0.75,0,0,10,0,0,5,0,494,0 },
/*28*/{"SHODAN"                ,0,7,0,  0, 55,  0,   0,   20,0,0,500,2,0,0,1,4,360,280,280,15,20.48,25.6,0,0,0,0,0,0,0,0.1,0,0.5,0,0,0,0,0.05,0,2,998,999,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,500,0.75,0,0,10,0,0,5,0,494,0 }};
//                  NPC Sounds 0, 1,  2, 3, 4,  5,  6,  7,  8,  9,10, 11,12,13,14, 15, 16, 17, 18, 19, 20, 21,22, 23, 24, 25, 26, 27, 28                                      0,  1,  2,  3, 4,  5, 6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28
int sfxIdle[NUM_AI_TYPES]   ={-1,-1, -1,-1,58, -1, 59, -1, 59, 52,-1, -1,-1,-1,-1, -1,121, -1, -1, -1,121,118,-1, -1, -1, -1, -1, -1, -1}; int sfxSightSound[NUM_AI_TYPES] ={-1, -1,111,150,58,150,59,152,152, -1,150,150,151,152,150, -1,121, -1,151,150,121,119,151, -1, -1, -1, -1, -1, -1};
int sfxAttack1[NUM_AI_TYPES]={-1,-1,108,-1,-1,146, -1,146,252,247,-1, -1,-1,-1,-1,122, -1,108,146, -1, -1,118,-1,125,258,258,258,258,258}; int sfxAttack2[NUM_AI_TYPES] =   {-1,256, -1,148,50, 50,50, 50, 50,250, 50, 50,146,259,148, -1,121, -1, -1,147, -1, -1,146, -1,258,258,258,258,258};
int sfxAttack3[NUM_AI_TYPES]={-1,-1, -1,-1,-1,244,244,244,245, -1,-1,149,-1,-1,-1, -1, -1, -1, -1,244, -1, -1,-1, -1,258,258,258,258,258}; int sfxDeath[NUM_AI_TYPES] =     {-1, 48,110,143,48,145,48, 51, 47, 47,142,143,144, 47,162,123,120,134,144,144,120,117,144,124, -1, -1, -1, -1, -1};
float deathBurstTimer[NUM_AI_TYPES] = {0.0f,0.0f, 0.1f,0.0f,0.1f,0.1f,0.2f,0.1f,0.1f,0.1f,0.0f,0.45f,0.75f,0.1f,0.0f,0.0f,0.1f,0.224f,0.9f,0.0f,0.1f,0.1f,0.1f,0.2f,0.1f,0.1f,0.1f,0.1f,0.1f};
static const u16 npcDeathTexture[NUM_AI_TYPES] = { [16] = 564 }; /*Textures/npc_invisomut_dead.png*/
// NPC gib ranges use the complete Citadel HealthManager gibObjects set. The
// primary member is the search collider and receives the NPC's searchable
// contents; the remaining members are visual/physical pieces of the same
// gibbed body. A zero first ID means the NPC does not gib.
typedef struct { u16 first, last, primary; } NPCGibRange;
static const NPCGibRange npcGibRanges[NUM_AI_TYPES] = {
    [3]={779,787,779},   // EXEC-BOT
    [5]={768,778,768},   // CORTEX REAVER
    [10]={814,820,814},  // SECURITY-1
    [11]={821,830,826},  // SECURITY-2
    [12]={794,803,795},  // MAINTENANCE
    [18]={831,839,839},  // SERV-BOT (search collider is gib9)
    [19]={788,793,792},  // FLIER BOT
    [22]={804,813,810},  // REPAIR BOT
};
INLINE bool ai_gibs_on_death(u16 npcID) { return npcID < NUM_AI_TYPES && npcGibRanges[npcID].first != 0; }
float GetDamageTakeAmount(DamageData* dd);
void InitNPC(u16 i) {
    World.layer[i] = L_NPC; u16 npcID = World.instances[i].index - 419; flag_set(&World.instances[i].entflags,EF_FIRST_SIGHTING,true);
    World.instances[i].currentDestination = World.instances[i].lastPosition = World.instances[i].idealPos = World.position[i]; World.instances[i].idealTransformForward = World.instances[i].forward;
    World.instances[i].tickFinished = World.pauseRelativeTime + AI_TICK_TIME + (double)random_range(0.0f, 1.0f); World.instances[i].tickTime = World.instances[i].tickFinished + (double)random_range(0.0f, 1.0f); World.instances[i].idleTime = World.pauseRelativeTime + (double)random_range(npcTable[npcID].timeIdleSFXMin,npcTable[npcID].timeIdleSFXMax);
    World.instances[i].attack1SoundTime = World.instances[i].attack2SoundTime = World.instances[i].attack3SoundTime = World.pauseRelativeTime; World.instances[i].huntFinished = World.pauseRelativeTime; int diff = (npcTable[npcID].type == NPCType_Cyber) ? World.diffCyb : World.diffCbt;
    if (diff <= 1) { World.instances[i].huntFinished += vmax((npcTable[npcID].huntTime * 0.75),60.0); }/*More forgetful on easy.*/ else if (diff >= 3) { World.instances[i].huntFinished += vmax((npcTable[npcID].huntTime * 2.00),60.0); }/*Good memory on hard.*/ else { World.instances[i].huntFinished += vmax(npcTable[npcID].huntTime,60.0); }
    World.instances[i].attackFinished = World.pauseRelativeTime + 1.0; World.instances[i].attack2Finished = World.instances[i].attack3Finished = World.instances[i].timeTillPainFinished = World.instances[i].timeTillDeadFinished = World.instances[i].meleeDamageFinished = World.instances[i].gracePeriodFinished = World.pauseRelativeTime;
    World.instances[i].randWaitAtt1Finished = World.instances[i].randWaitAtt2Finished = World.instances[i].randWaitAtt3Finished = World.instances[i].tranquilizeFinished = World.instances[i].deathBurstFinished = World.instances[i].wanderFinished = World.instances[i].posCheckFinished = World.instances[i].timeTillEnemyChangeFinished = World.pauseRelativeTime;
    World.instances[i].timeSinceMovedEnough = 0.0; World.instances[i].currentState = AIState_Idle; if (npcID == 20 && !World.instances[i].textureAnimating) { TextureSequenceStart(i, 47); } u8 c=A_IDLE; if ((World.instances[i].entflags & EF_WANDERING) && (random_range(0.0f,1.0f) < 0.5f)){World.instances[i].currentState = AIState_Walk;} else {flag_set(&World.instances[i].entflags,EF_WANDERING,false);}
    if (World.instances[i].entflags & EF_ASLEEP) { World.instances[i].currentState=AIState_Idle; /*flag_set(&World.instances[e->sleepingCables].entflags, EF_ACTIVE, true);*//*deactivated sleeping cables in AIAwakeFromSleep*/ }
    switch (World.instances[i].currentState){case AIState_Walk:c=A_WALK; break; case AIState_Run:c=A_RUN; break; case AIState_Attack1:c=A_ATTACK1; break; case AIState_Attack2:c=A_ATTACK2; break; case AIState_Attack3:c=A_ATTACK3; break; case AIState_Pain:c=A_PAIN; break; case AIState_Dying: case AIState_Dead:c=A_DYING; break;}
    World.instances[i].clip = c; World.instances[i].frame = modelAnimationClips[World.instances[i].animationNum][c].frameStart; World.instances[i].currentFrameFinished = 0.0;
}

float Tranquilize(u16 i, float amount, bool energy) { u16 n=World.instances[i].index-419; if (npcTable[n].type == NPCType_Robot && !energy){return 0.0f;} float tranqSecs=(amount<3.0f) ? npcTable[n].timeForTranquilization : amount; World.instances[i].tranquilizeFinished=vmax(World.pauseRelativeTime+tranqSecs,World.instances[i].tranquilizeFinished+tranqSecs); return tranqSecs; }
bool IsCyberNPC(u16 i) { u16 npcID = World.instances[i].index - 419; return npcTable[npcID].type == NPCType_Cyber; }
bool HasHealth(u16 i) { if(IsCyberNPC(i)){return (World.instances[i].cyberHealth > 0.0f);} return (World.instances[i].health > 0.0f); }
INLINE bool ai_is_cyber(Entity* e)  { return npcTable[e->index - 419].type == NPCType_Cyber; }
INLINE bool ai_has_health(Entity* e){ return ai_is_cyber(e) ? e->cyberHealth > 0.0f : e->health > 0.0f; }
float sightPointHeights[NUM_AI_TYPES]={0.746f,0.824f,0.261f,0.89f,0.86f,0.875f,0.736f,0.923f,1.082f,1.014f,0.578f/*10*/,0.797f,0.185f,1.155f,1.759f,0.843f,0.133f,0.651f,0.323f,0.0f,0.0f/*20*/,0.0f,-0.662f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f/*28*/};
INLINE V3 ai_sight_pos(Entity* e);
typedef struct { V3 gunPoint, gunPoint2; } AIMuzzleOffsets;
/* Rest-pose gunPoint offsets read from the Citadel npc_* prefabs. Attack2
   uses gunPoint; Attack3 uses gunPoint2 when present and otherwise gunPoint.
   Values are relative to the NPC root and rotate with its current heading. */
static const AIMuzzleOffsets aiMuzzleOffsets[NUM_AI_TYPES] = {
    [0]={{0.0000f,0.0000f,0.0000f},{0.0000f,0.0000f,0.0000f}},
    [1]={{0.2034f,0.5728f,1.4950f},{-0.3500f,0.5500f,1.0000f}},
    [2]={{0.0000f,0.0000f,0.0000f},{0.0000f,0.0000f,0.0000f}},
    [3]={{0.0000f,0.3350f,0.6800f},{0.0000f,0.3350f,0.6800f}},
    [4]={{0.0000f,0.8600f,0.0000f},{0.0000f,0.8600f,0.0000f}},
    [5]={{0.0000f,1.3170f,0.2050f},{0.0000f,1.3170f,0.2050f}},
    [6]={{0.2450f,0.6190f,0.9770f},{-0.6700f,0.9500f,0.0000f}},
    [7]={{0.1221f,0.6699f,1.2048f},{-0.1770f,0.6113f,0.6103f}},
    [8]={{0.2969f,0.4550f,0.8212f},{0.0015f,1.0694f,0.3210f}},
    [9]={{0.0000f,0.5048f,0.7103f},{0.0000f,0.5048f,0.7103f}},
    [10]={{0.0000f,-0.1740f,0.2980f},{0.0000f,-0.1740f,0.2980f}},
    [11]={{0.0000f,1.0250f,1.3230f},{0.0000f,1.2490f,0.6870f}},
    [12]={{0.0000f,0.4500f,0.5000f},{0.0000f,0.4500f,0.5000f}},
    [13]={{0.0000f,1.2330f,0.4200f},{0.0000f,1.2330f,0.4200f}},
    [14]={{0.0000f,1.7590f,0.2150f},{0.0000f,1.7590f,0.2150f}},
    [15]={{0.0000f,0.0000f,0.0000f},{0.0000f,0.0000f,0.0000f}},
    [16]={{0.0000f,-0.2840f,0.2560f},{0.0000f,-0.2840f,0.2560f}},
    [17]={{0.0896f,0.4884f,0.7545f},{0.0896f,0.4884f,0.7545f}},
    [18]={{-1.0198f,0.1295f,1.1798f},{-1.0198f,0.1295f,1.1798f}},
    [19]={{0.0000f,0.0000f,0.9000f},{0.0000f,0.0000f,0.9000f}},
    [20]={{0.0000f,0.0000f,0.4700f},{0.0000f,0.0000f,0.4700f}},
    [21]={{0.0000f,0.0000f,1.2220f},{0.0000f,0.0000f,1.2220f}},
    [22]={{0.0000f,-0.4100f,0.8460f},{0.0000f,-0.4100f,0.8460f}},
    [23]={{0.0000f,0.0000f,0.3950f},{0.0000f,0.0000f,0.3950f}},
    [24]={{0.0000f,-0.4380f,-0.6690f},{0.0000f,-0.4380f,-0.6690f}},
    [25]={{-0.4650f,0.0000f,-0.3930f},{-0.4650f,0.0000f,-0.3930f}},
    [26]={{0.0000f,-0.5590f,-0.3840f},{0.0000f,-0.5590f,-0.3840f}},
    [27]={{0.0000f,0.0000f,-0.2980f},{0.0000f,0.0000f,-0.2980f}},
    [28]={{0.0000f,0.0000f,0.5170f},{0.0000f,0.0000f,0.5170f}},
};
typedef struct { Color3 color; float unityIntensity, range; } AIMuzzleLightData;
/* Values with serialized Unity muzzle lights use their prefab color, intensity,
   and range. Entries for projectile prefabs without a muzzle Light use a
   restrained visual fallback. Unity intensity is scaled by 0.35 in Voxen's
   light loader, so the activation code applies that conversion as well. */
static const AIMuzzleLightData aiMuzzleLights[NUM_AI_TYPES][2] = {
    [1]={{{0.9044f,0.8595f,0.6717f},10.09f,2.0f},{{0.9044f,0.8595f,0.6717f},10.09f,2.0f}},
    [3]={{{0.6706f,0.8487f,0.9059f},3.46f,2.0f},{{0.6706f,0.8487f,0.9059f},3.46f,2.0f}},
    [4]={{{0.9044f,0.8595f,0.6717f},10.09f,2.0f},{{0.9044f,0.8595f,0.6717f},10.09f,2.0f}},
    [5]={{{0.0803f,0.0000f,1.0000f},4.21f,5.0f},{{0.0803f,0.0000f,1.0000f},4.21f,5.0f}},
    [6]={{{0.8679f,0.1226f,0.0000f},10.09f,2.0f},{{0.8679f,0.1226f,0.0000f},10.09f,2.0f}},
    [7]={{{0.9608f,0.9195f,0.8039f},10.09f,2.0f},{{0.9608f,0.9195f,0.8039f},10.09f,2.0f}},
    [8]={{{0.8731f,1.0000f,0.6368f},7.0f,2.0f},{{0.8679f,0.1696f,0.1187f},10.0f,2.5f}},
    [9]={{{0.8774f,0.0000f,0.0000f},2.56f,2.0f},{{0.8774f,0.0000f,0.0000f},2.56f,2.0f}},
    [10]={{{0.0387f,0.7453f,0.0387f},5.0f,2.0f},{{0.0387f,0.7453f,0.0387f},5.0f,2.0f}},
    [11]={{{1.0000f,0.9794f,0.7500f},1.0f,2.0f},{{1.0000f,0.9794f,0.7500f},2.5f,2.0f}},
    [12]={{{0.8160f,0.8844f,1.0000f},8.69f,2.0f},{{0.8160f,0.8844f,1.0000f},8.69f,2.0f}},
    [13]={{{0.8412f,1.0000f,0.3821f},3.0f,2.0f},{{0.8412f,1.0000f,0.3821f},3.0f,2.0f}},
    [14]={{{1.0000f,0.5035f,0.2784f},2.0f,7.0f},{{1.0000f,0.5035f,0.2784f},2.0f,7.0f}},
    [16]={{{0.9061f,0.5943f,1.0000f},2.5f,2.0f},{{0.9061f,0.5943f,1.0000f},2.5f,2.0f}},
    [17]={{{0.6500f,1.0000f,0.2500f},2.5f,2.0f},{{0.6500f,1.0000f,0.2500f},2.5f,2.0f}},
    [18]={{{0.9849f,0.5425f,1.0000f},2.0f,3.0f},{{0.9849f,0.5425f,1.0000f},2.0f,3.0f}},
    [19]={{{0.9245f,0.8921f,0.5190f},5.0f,2.0f},{{0.9245f,0.8921f,0.5190f},5.0f,2.0f}},
    [20]={{{1.0000f,0.9807f,0.7358f},2.5f,2.0f},{{1.0000f,0.9807f,0.7358f},2.5f,2.0f}},
    [22]={{{0.6706f,0.9059f,0.8552f},10.09f,2.0f},{{0.6706f,0.9059f,0.8552f},10.09f,2.0f}},
    [23]={{{0.5000f,1.0000f,0.3000f},3.0f,2.0f},{{0.5000f,1.0000f,0.3000f},3.0f,2.0f}},
    [24]={{{1.0000f,0.8500f,0.6500f},3.0f,2.0f},{{1.0000f,0.8500f,0.6500f},3.0f,2.0f}},
    [25]={{{1.0000f,0.8500f,0.6500f},3.0f,2.0f},{{1.0000f,0.8500f,0.6500f},3.0f,2.0f}},
    [26]={{{1.0000f,0.8500f,0.6500f},3.0f,2.0f},{{1.0000f,0.8500f,0.6500f},3.0f,2.0f}},
    [27]={{{1.0000f,0.8500f,0.6500f},3.0f,2.0f},{{1.0000f,0.8500f,0.6500f},3.0f,2.0f}},
    [28]={{{1.0000f,0.8500f,0.6500f},3.0f,2.0f},{{1.0000f,0.8500f,0.6500f},3.0f,2.0f}},
};
static const double AI_MUZZLE_FLASH_TIME=0.085;
static const u16 AI_MUZZLE_LIGHT_POOL_SIZE=24;
typedef struct { u16 lights[24], count; double expires[24]; bool ready; } AIMuzzleLightPool;
static AIMuzzleLightPool aiMuzzlePools[MAX_LEVELS];
static double aiMuzzleLightsUpdatedAt=-1.0;
static float ai_muzzle_marker(u16 slot) { return -100.0f-(float)slot; }
static void ai_ensure_muzzle_pool(void) {
    u16 lev=World.currentLevel; if (lev>=MAX_LEVELS) return; AIMuzzleLightPool* p=&aiMuzzlePools[lev];
    if (p->ready) { for (u16 i=0;i<p->count;i++) if (p->lights[i]>=World.loadedLights || World.lights[p->lights[i]].spotAng!=ai_muzzle_marker(i)) { p->ready=false; break; } }
    if (p->ready) return; p->count=0;
    while (p->count<AI_MUZZLE_LIGHT_POOL_SIZE && World.loadedLights< LIGHT_COUNT-1) { u16 i=p->count; u16 li=AddLightSimple((V3){0,0,0},(Color3){1,1,1},1.0f,0.0f,0); p->lights[i]=li; p->expires[i]=0.0; World.lights[li].spotAng=ai_muzzle_marker(i); p->count++; }
    p->ready=p->count>0;
}
static void ai_update_muzzle_lights(void) {
    if (aiMuzzleLightsUpdatedAt==World.pauseRelativeTime) return; aiMuzzleLightsUpdatedAt=World.pauseRelativeTime; u16 lev=World.currentLevel; if (lev>=MAX_LEVELS) return; AIMuzzleLightPool* p=&aiMuzzlePools[lev];
    if (!p->ready) return; for (u16 i=0;i<p->count;i++) if (p->lights[i]>=World.loadedLights || World.lights[p->lights[i]].spotAng!=ai_muzzle_marker(i)) { p->ready=false; p->count=0; return; }
    for (u16 i=0;i<p->count;i++) if (p->expires[i]>0.0 && p->expires[i]<=World.pauseRelativeTime) { u16 li=p->lights[i]; UpdateLight(li,World.lights[li].pos,World.lights[li].col,World.lights[li].range,0.0f,0.0f,0.0f,ai_muzzle_marker(i),QUAT_IDENTITY,false,false); p->expires[i]=0.0; }
}
static void ai_muzzle_flash(Entity* self, int attackNum) {
    if (attackNum<1 || attackNum>3) attackNum=1; u16 npc=(u16)(self->index-419); if (npc>=NUM_AI_TYPES) return; ai_ensure_muzzle_pool(); u16 lev=World.currentLevel; if (lev>=MAX_LEVELS) return; AIMuzzleLightPool* p=&aiMuzzlePools[lev]; if (!p->count) return;
    int liSlot=(attackNum==3)?1:0; AIMuzzleLightData d=aiMuzzleLights[npc][liSlot]; if (d.unityIntensity<=0.0f || d.range<=0.0f) return; u16 chosen=0; double oldest=1e30;
    for (u16 i=0;i<p->count;i++) { if (p->expires[i]<=World.pauseRelativeTime) { chosen=i; oldest=-1.0; break; } if (p->expires[i]<oldest) { oldest=p->expires[i]; chosen=i; } }
    u16 idx=p->lights[chosen]; u16 selfIdx=(u16)(self-World.instances); V3 pos;
    if (attackNum==1) pos=ai_sight_pos(self);
    else { V3 off=aiMuzzleOffsets[npc].gunPoint; if (attackNum==3 && (aiMuzzleOffsets[npc].gunPoint2.x!=0.0f || aiMuzzleOffsets[npc].gunPoint2.y!=0.0f || aiMuzzleOffsets[npc].gunPoint2.z!=0.0f)) off=aiMuzzleOffsets[npc].gunPoint2; if (off.x==0.0f && off.y==0.0f && off.z==0.0f) pos=ai_sight_pos(self); else pos=V3_AplusB(World.position[selfIdx],quat_rot_v3(World.rotation[selfIdx],off)); }
    float intensity=d.unityIntensity*0.35f; UpdateLight(idx,pos,d.color,vclamp(d.range,0.32f,15.36f),intensity,intensity,0.0f,ai_muzzle_marker(chosen),QUAT_IDENTITY,true,false); p->expires[chosen]=World.pauseRelativeTime+AI_MUZZLE_FLASH_TIME;
}
INLINE V3 ai_sight_pos(Entity* e) { u16 idx=(u16)(e - World.instances); return V3_AplusB(World.position[idx],(V3){0.0f,sightPointHeights[World.instances[idx].index - 419],0.0f}); }
INLINE V3 ai_gun_pos(Entity* e, int n) { u16 idx=(u16)(e - World.instances); u16 npc=World.instances[idx].index-419; V3 off=(n==3 && (aiMuzzleOffsets[npc].gunPoint2.x!=0.0f || aiMuzzleOffsets[npc].gunPoint2.y!=0.0f || aiMuzzleOffsets[npc].gunPoint2.z!=0.0f))?aiMuzzleOffsets[npc].gunPoint2:aiMuzzleOffsets[npc].gunPoint; if(off.x==0.0f && off.y==0.0f && off.z==0.0f) off=(V3){0.0f,sightPointHeights[npc]+0.3f,0.0f}; return V3_AplusB(World.position[idx],quat_rot_v3(World.rotation[idx],off)); }
INLINE V3 ai_attack_pos(Entity* e, int n) { return n==1 ? ai_sight_pos(e) : ai_gun_pos(e,n); }
Quaternion quat_look_rotation(V3 fwd, V3 up) {
    fwd=V3_Normalize(fwd); V3 r = V3_Normalize(V3_Cross(up,fwd)); up=V3_Cross(fwd,r); float m00=r.x, m01=r.y, m02=r.z, m10=up.x, m11=up.y, m12=up.z, m20=fwd.x, m21=fwd.y, m22=fwd.z; float tr = m00 + m11 + m22; Quaternion q;
    if (tr > 0.0f){float s=0.5f/vsqrtf(tr+1.0f); q.w=(0.25f/s); q.x=(m12-m21)*s; q.y=(m20-m02)*s; q.z=(m01-m10)*s;}else if(m00 > m11 && m00 > m22){float s=2.0f*vsqrtf(1.0f+m00-m11-m22); q.w=(m12-m21)/s; q.x=0.25f*s; q.y=(m01+m10)/s; q.z=(m20+m02)/s;}else if(m11 > m22){float s=2.0f*vsqrtf(1.0f+m11-m00-m22); q.w=(m20-m02)/s; q.x=(m01+m10)/s; q.y=0.25f*s; q.z=(m12+m21)/s; } else { float s = 2.0f * vsqrtf(1.0f + m22 - m00 - m11); q.w=(m01-m10)/s; q.x=(m20+m02)/s; q.y=(m12+m21)/s; q.z=0.25f*s;} return q;
}

void aiac_idle(Entity* self) { if ((self->entflags & EF_ASLEEP) || self->tranquilizeFinished >= World.pauseRelativeTime) {self->currentFrameFinished=World.pauseRelativeTime + 1e9; return;/*freeze*/} ChangeAnim(self,A_IDLE); }
void aiac_walk(Entity* self){if(self->entflags & EF_ACT_AS_TURRET){aiac_idle(self); return;} u16 idx=(u16)(self-World.instances); V3 v=World.velocity[idx]; if((v.x*v.x+v.y*v.y+v.z*v.z)>(0.32f*0.32f)){ChangeAnim(self,A_WALK); return;} if (self->animSwapFinished<World.pauseRelativeTime){self->animSwapFinished=World.pauseRelativeTime+.5f; ChangeAnim(self,A_IDLE);}}
void aiac_dying(Entity* self) { flag_set(&self->entflags,EF_ASLEEP,false); AnimationClip cl=modelAnimationClips[self->animationNum][A_DYING]; if(cl.frameEnd == cl.frameStart){ChangeAnim(self,A_DYING); return;} ChangeAnim(self,A_DYING);}
void AIAnimationControllerUpdate(u16 idx) {
    Entity* self = &World.instances[idx]; if((!(self->entflags & EF_ACTIVE)) || (self->animationNum >= MAX_ANIMS)){return;} if(self->currentState == AIState_Dying){aiac_dying(self); return;}
    if(self->currentState == AIState_Dead){AnimationClip cl=modelAnimationClips[self->animationNum][A_DYING]; self->clip=A_DYING; self->frame=cl.frameEnd; self->modelIndex=cl.frameStartModelIndex + (cl.frameEnd - cl.frameStart); self->currentFrameFinished=World.pauseRelativeTime + 1e9; return;/*freeze*/} if(self->entflags & EF_ASLEEP){aiac_idle(self); return;}
    if(self->currentState == AIState_Run && self->tranquilizeFinished >= World.pauseRelativeTime){aiac_idle(self); return;}
    switch (self->currentState) { case AIState_Walk:aiac_walk(self); break; case AIState_Run:if(self->entflags & EF_ACT_AS_TURRET){aiac_idle(self);}else{ChangeAnim(self,A_RUN);} break; case AIState_Attack1:ChangeAnim(self,A_ATTACK1); break; case AIState_Attack2:ChangeAnim(self,A_ATTACK2); break; case AIState_Attack3:ChangeAnim(self,A_ATTACK3); break; case AIState_Pain:ChangeAnim(self,A_PAIN); break; default:aiac_idle(self); break; }
}

INLINE bool NPCInPlayerPVS(u16 idx) { if (unlikely(World.curLev >= LEVEL_CYBERSPACE)) return true;/*Culling is disabled in cyberspace (CullCore early-outs), so never gate there.*/ return PositionVisibleFromPlayerCell(World.position[idx].x,World.position[idx].z); }
bool AICheckIfEnemyInSight(u16 idx) {
    u16 eidx=World.instances[idx].enemy; if (!eidx || !ai_has_health(&World.instances[idx])) return false; bool enIsNPC = (World.layer[eidx] & L_NPC) != 0; int diff = ai_is_cyber(&World.instances[idx]) ? World.diffCyb : World.diffCbt; if (!enIsNPC && !NPCInPlayerPVS(idx)) return false;
    if (diff == 0 && (World.instances[idx].index - 419) != 28) return false; if (Cheats.notarget && !enIsNPC) { World.instances[idx].enemy = 0; World.instances[idx].posCheckFinished = World.pauseRelativeTime + AI_POS_CHECK_DELAY; World.instances[idx].lastPosition = World.position[idx]; flag_set(&World.instances[idx].entflags, EF_ENEM_IN_LOS, false); return false; }
    if (ai_is_cyber(&World.instances[idx]) && World.decoyActive) { flag_set(&World.instances[idx].entflags, EF_ENEM_IN_LOS, false); return false; } float dist = V3_Dist(World.position[eidx], ai_sight_pos(&World.instances[idx])); if (dist > npcTable[World.instances[idx].index - 419].sightRange) return false; if (ai_is_cyber(&World.instances[idx]) || enIsNPC) return true;
    V3 spos=ai_sight_pos(&World.instances[idx]); V3 lineN=V3_Normalize(V3_AsubB(World.position[eidx],spos));
    RaycastHit hit=Raycast(spos,lineN,npcTable[World.instances[idx].index-419].sightRange,LMASK_NPC_SIGHT);
    if (hit.hit) {
        if (hit.hitInstanceIndex == eidx) { flag_set(&World.instances[idx].entflags, EF_ENEM_IN_LOS, true); return true; } NPCType t = npcTable[World.instances[idx].index - 419].type;
        if (t != NPCType_Mutant && t != NPCType_Supermutant && t != NPCType_Cyber) {/*Smarte npcs attempt to open doors btw them and player.*/
            u16 hi=hit.hitInstanceIndex; if (hi&&V3_SqDist(hit.point,spos)<4.0f&&IdxIsDoor(World.instances[hi].index)){Entity* dr=&World.instances[hi]; if ((dr->doorOpen == DoorState_Closed || (dr->doorOpen == DoorState_Closing && World.diffCbt > 2)) && !(dr->entflags & EF_LOCKED) && GetCurrentLevelSecurity() <= dr->securityThreshold && (dr->requiredAccessCard == ACC_None)) DoorActuate(hi);}
        }
    } flag_set(&World.instances[idx].entflags, EF_ENEM_IN_LOS, false); return false;
}

void AISetHuntFinished(u16 idx) { World.instances[idx].huntFinished = World.pauseRelativeTime; int diff = ai_is_cyber(&World.instances[idx]) ? World.diffCyb : World.diffCbt; double ht = npcTable[World.instances[idx].index - 419].huntTime, mn=60.0; if(diff <= 1){World.instances[idx].huntFinished += (ht * 0.75 > mn ? ht * 0.75 : mn);}else if(diff >= 3){World.instances[idx].huntFinished += (ht * 2.0  > mn ? ht * 2.0 : mn);}else{World.instances[idx].huntFinished += (ht > mn ? ht : mn);} }
void AISetEnemy(u16 idx, u16 eidx) {
    if(!eidx){return;} World.instances[idx].enemy=eidx; World.instances[idx].posCheckFinished=World.pauseRelativeTime + AI_POS_CHECK_DELAY; flag_set(&World.instances[idx].entflags,EF_WANDERING,false); World.instances[idx].wanderFinished=World.pauseRelativeTime;
    World.instances[idx].lastPosition = World.position[idx]; World.instances[idx].lastKnownEnemyPos = World.position[eidx]; World.instances[idx].targettingPosition = (V3){World.position[eidx].x,World.position[eidx].y + AI_TARGET_OFFSET_Y,World.position[eidx].z}; AISetHuntFinished(idx);
}

void AIPlaySightSound(u16 idx) { if ((!(World.instances[idx].entflags&EF_FIRST_SIGHTING)) || (!ai_has_health(&World.instances[idx])) || (World.instances[idx].entflags&EF_ACT_AS_CORPSE_ONLY)){return;} flag_set(&World.instances[idx].entflags,EF_FIRST_SIGHTING,false); i16 sfx = sfxSightSound[World.instances[idx].index - 419]; if (sfx >= 39 && sfx < SOUNDS_COUNT){play_wav(sounds[sfx],AppliedFXVol(1.0f),World.position[idx],true);} }
bool AICheckIfPlayerInSight(u16 idx) {
    int diff = ai_is_cyber(&World.instances[idx]) ? World.diffCyb : World.diffCbt; if (!NPCInPlayerPVS(idx) || (diff == 0 && (World.instances[idx].index - 419) != 28)) return false; if (World.instances[idx].enemy) return AICheckIfEnemyInSight(idx);
    flag_set(&World.instances[idx].entflags,EF_ENEM_IN_LOS,false); if ((ai_is_cyber(&World.instances[idx]) && World.decoyActive) || Cheats.notarget){return false;} V3 playerPos=World.position[PLAYER1], spos=ai_sight_pos(&World.instances[idx]); float dist=V3_Dist(playerPos,spos); NPCTable* npc = &npcTable[World.instances[idx].index - 419]; if (dist > npc->sightRange) return false;
    if (ai_is_cyber(&World.instances[idx])) { AISetEnemy(idx,PLAYER1); AIPlaySightSound(idx); return true; } V3 checkN = V3_Normalize(V3_AsubB(playerPos,spos)); float cosA = vclamp(V3_dot(checkN,World.instances[idx].forward), -1.0f, 1.0f); float angle = vacosf(cosA) * (180.0f / PI); bool makingNoise = World.invP1.noiseFinished > World.pauseRelativeTime;
    if (angle < npc->fov * 0.5f) { RaycastHit hit = Raycast(spos, checkN, dist + 0.1f, LMASK_NPC_SIGHT); if (hit.hit && hit.hitInstanceIndex == PLAYER1) { flag_set(&World.instances[idx].entflags, EF_ENEM_IN_LOS, true); AISetEnemy(idx,PLAYER1); AIPlaySightSound(idx); return true; } if (!hit.hit && makingNoise && dist < npc->hearingRange) { AISetEnemy(idx,PLAYER1); AIPlaySightSound(idx); return true; } }
    else { if (dist < npc->distToSeeBehind) { RaycastHit hit = Raycast(spos,checkN,dist + 0.1f,LMASK_NPC_SIGHT); if (hit.hit && hit.hitInstanceIndex == PLAYER1) { flag_set(&World.instances[idx].entflags, EF_ENEM_IN_LOS, true); AISetEnemy(idx,PLAYER1); AIPlaySightSound(idx); return true; } } if (makingNoise && dist < npc->hearingRange) { AISetEnemy(idx,PLAYER1); AIPlaySightSound(idx); return true; } }   return false;
}

static void AIEnemyInFrontChecks(Entity* e, u16 i) { if(!i){flag_set(&e->entflags,EF_ENEM_IN_FOV,false); flag_set(&e->entflags,EF_ENEM_IN_FRONT,false); return;} if(ai_is_cyber(e)){flag_set(&e->entflags,EF_ENEM_IN_FOV,true); flag_set(&e->entflags,EF_ENEM_IN_FRONT,true); return;} V3 spos=ai_sight_pos(e),epos=World.position[i]; V3 iv=V3_Normalize((V3){epos.x-spos.x,0.0f,epos.z-spos.z}); float d=V3_dot(iv,e->forward); flag_set(&e->entflags,EF_ENEM_IN_FOV,d>0.800f); flag_set(&e->entflags,EF_ENEM_IN_FRONT,d>0.300f); }
static void AIFace(Entity* self, V3 goal) {
    u16 sidx=(u16)(self - World.instances); if (self->entflags & EF_ASLEEP) return; V3 fv = V3_AsubB(goal,World.position[sidx]); if (!ai_is_cyber(self)) fv.y = 0.0f; if (fv.x == 0.0f && fv.y == 0.0f && fv.z == 0.0f) return; u16 eidx = self->enemy; if (ai_is_cyber(self) && eidx) { World.rotation[sidx] = World.rotation[eidx]; self->forward = V3_Normalize(quat_rot_v3(World.rotation[sidx],(V3){0.0f,0.0f,1.0f})); return; }
    if (fv.x == 0.0f && fv.z == 0.0f) { if (eidx){fv=V3_AsubB(World.position[eidx],World.position[sidx]);} else{fv.x += 0.001f;} }
    V3 currentForward=V3_Normalize(quat_rot_v3(World.rotation[sidx],(V3){0.0f,0.0f,1.0f})); float currentYaw=__builtin_atan2f(currentForward.x,currentForward.z), targetYaw=__builtin_atan2f(fv.x,fv.z), yaw=targetYaw-currentYaw; float t=(float)(0.2f*npcTable[self->index-419].yawSpeed*World.deltaTime*World.timeScale); yaw*=t; float half=yaw*0.5f; Quaternion yawRotation={0.0f,vsinf(half),0.0f,vcosf(half)}; World.rotation[sidx]=quat_normalize(quat_multiply(yawRotation,World.rotation[sidx])); self->forward=V3_Normalize(quat_rot_v3(World.rotation[sidx],(V3){0.0f,0.0f,1.0f}));
}

INLINE float quat_angle_deg(Quaternion a, Quaternion b) { float d = vclamp(vabs(quat_dot(a, b)), 0.0f, 1.0f); return 2.0f * vacosf(d) * (180.0f / PI); }
static bool AIWithinAngleToTarget(Entity* self) { if (ai_is_cyber(self)){return true;} if(V3_dot(self->idealTransformForward,self->idealTransformForward)<=1e-6f)return false; u16 sidx=(u16)(self - World.instances); Quaternion lr=quat_look_rotation(self->idealTransformForward,(V3){0,1,0}); float ang=quat_angle_deg(World.rotation[sidx],lr); float fovMov=npcTable[self->index - 419].fovStartMovement; if(ang<fovMov)return true; if(ang<fovMov*1.5f&&random_range(0.0f,1.0f)<0.5f){return true;} return false; }
bool AICheckPain(u16 self) {
    u16 ndx=World.instances[self].index - 419; if(ai_is_cyber(&World.instances[self]) || (World.instances[self].entflags & EF_ASLEEP) || (npcTable[ndx].timeBetweenPain <= 0.0f) || (!(World.instances[self].entflags & EF_GO_INTO_PAIN) || World.instances[self].timeTillPainFinished >= World.pauseRelativeTime)){return false;}
    World.instances[self].currentState = AIState_Pain; u16 atkIdx = World.instances[self].recentMostActivator;
    if (atkIdx && World.instances[self].timeTillEnemyChangeFinished < World.pauseRelativeTime) {
        World.instances[self].timeTillEnemyChangeFinished = World.pauseRelativeTime + npcTable[ndx].timeToChangeEnemy; bool atkIsPlayer = (World.layer[atkIdx] & L_Player) != 0;
        if (!atkIsPlayer && IdxIsNPC(World.instances[atkIdx].index)) {
            NPCType mt = npcTable[ndx].type, at = npcTable[World.instances[atkIdx].index - 419].type; bool canFight = World.instances[atkIdx].index != World.instances[self].index; if ((mt == NPCType_Robot && World.instances[self].enemy) || ((mt == NPCType_Cyborg || mt == NPCType_Supercyborg || mt == NPCType_Robot) && (at == NPCType_Cyborg || at == NPCType_Supercyborg || at == NPCType_Robot))) canFight = false; if (canFight) World.instances[self].enemy=atkIdx;
        } else World.instances[self].enemy=atkIdx;
        World.instances[self].posCheckFinished=World.pauseRelativeTime+AI_POS_CHECK_DELAY; flag_set(&World.instances[self].entflags,EF_WANDERING,false); World.instances[self].wanderFinished=World.pauseRelativeTime; World.instances[self].lastPosition=World.position[self]; if(World.instances[self].enemy){World.instances[self].lastKnownEnemyPos=World.instances[self].currentDestination=World.position[World.instances[self].enemy];}
    } flag_set(&World.instances[self].entflags, EF_GO_INTO_PAIN, false); World.instances[self].timeTillPainFinished = World.pauseRelativeTime + npcTable[ndx].timeToPain; return true;
}

static void AIIdle(u16 sidx) {
    if (World.instances[sidx].enemy && ai_has_health(&World.instances[sidx])) { World.instances[sidx].currentState = AIState_Run; return; } NPCTable* npc = &npcTable[World.instances[sidx].index - 419];
    if (World.instances[sidx].idleTime < World.pauseRelativeTime) { int sidle = sfxIdle[World.instances[sidx].index - 419]; if (random_range(0.0f, 1.0f) < 0.5f && sidle >= 0 && sidle < (i16)SOUNDS_COUNT) play_wav(sounds[sidle],AppliedFXVol(1.0f),World.position[sidx],true); World.instances[sidx].idleTime = World.pauseRelativeTime + random_range(npc->timeIdleSFXMin, npc->timeIdleSFXMax); } AICheckPain(sidx);
}

static V3 AIGetWanderPoint(Entity* self) { u16 sidx=(u16)(self - World.instances); return (V3){World.position[sidx].x + random_range(-AI_WANDER_RANGE,AI_WANDER_RANGE),ai_is_cyber(self) ? World.position[sidx].y + random_range(-AI_WANDER_RANGE,AI_WANDER_RANGE) : 0.0f,World.position[sidx].z + random_range(-AI_WANDER_RANGE,AI_WANDER_RANGE)}; }
static V3 AIGetAStarPoint(Entity* self) {
    u16 sidx=(u16)(self - World.instances); i32 cx=PosGetCellCoordX(World.position[sidx].x), cz=PosGetCellCoordZ(World.position[sidx].z); if (!XZPairInBounds(cx,cz)) return AIGetWanderPoint(self);
    u32 current=(u32)cz*WORLDX+(u32)cx; V3 ep=self->enemy ? World.position[self->enemy] : World.position[sidx]; V3 cands[4]; i32 dx[4]={0,0,1,-1}, dz[4]={1,-1,0,0}; u32 closed[4]={CELL_CLOSEDNORTH,CELL_CLOSEDSOUTH,CELL_CLOSEDEAST,CELL_CLOSEDWEST}; int count=0;
    for (int i=0;i<4;++i) { i32 nx=cx+dx[i], nz=cz+dz[i]; if (!XZPairInBounds(nx,nz) || (gridCellStates[current]&closed[i]) || !(gridCellStates[(u32)nz*WORLDX+(u32)nx]&CELL_OPEN)) continue; cands[count++]=World.position[sidx]; cands[count-1].x+=dx[i]*CELLSZ; cands[count-1].z+=dz[i]*CELLSZ; }
    int best=0; float bestD=count ? V3_SqDist(ep,cands[0]) : 1e9f; for (int i=1;i<count;++i) { float d=V3_SqDist(ep,cands[i]); if (d<bestD) { bestD=d; best=i; } } return count ? cands[best] : AIGetWanderPoint(self);
}

static V3 AIGetSearchPoint(Entity* self) { NPCType t = npcTable[self->index - 419].type; if (t == NPCType_Mutant || t == NPCType_Supermutant) {return AIGetWanderPoint(self);} return AIGetAStarPoint(self); }
static void AIHopMove(u16 self) { if (!(World.instances[self].entflags & EF_HOP_DONE)){flag_set(&World.instances[self].entflags,EF_HOP_DONE,true); AddForce(self,V3_ScaleByF(World.instances[self].forward,500.0f),true); AddForce(self,(V3){0,5.0f,0},true);} else {flag_set(&World.instances[self].entflags,EF_HOP_DONE,false);} }
static void AIWalk(u16 self) {
    if ((AICheckPain(self)) || (World.instances[self].entflags & EF_ASLEEP)){return;} if ((World.instances[self].entflags & EF_ENEM_IN_SIGHT) || World.instances[self].enemy){World.instances[self].currentState=AIState_Run; return;} if (World.instances[self].entflags & EF_ACT_AS_TURRET) {World.instances[self].currentState = AIState_Idle; return;}
    if ((npcTable[World.instances[self].index - 419].moveType == AIMoveType_None) || (World.instances[self].tranquilizeFinished >= World.pauseRelativeTime)){return;} u16 sidx = self; if (!PositionVisibleFromPlayerCell(World.position[sidx].x,World.position[sidx].z)){return;}
    float dist = V3_Dist(ai_sight_pos(&World.instances[self]),World.instances[self].currentDestination); if (World.instances[self].entflags & EF_WANDERING) { if (World.instances[self].wanderFinished < World.pauseRelativeTime || dist < AI_STOP_DIST * 0.5f) { World.instances[self].wanderFinished = World.pauseRelativeTime + random_range(3.0f, 8.0f); World.instances[self].currentDestination = AIGetWanderPoint(&World.instances[self]); } }
    if (dist > AI_STOP_DIST && AIWithinAngleToTarget(&World.instances[self])) {
        if (npcTable[World.instances[self].index - 419].hopsOnMove) { AIHopMove(self); }
        else {
            float ws  = npcTable[World.instances[self].index - 419].walkSpeed; V3 mv = { World.instances[self].forward.x*ws,World.instances[self].forward.y*ws,World.instances[self].forward.z*ws };
            if (npcTable[World.instances[self].index - 419].moveType != AIMoveType_Fly) { V3 spos = ai_sight_pos(&World.instances[self]); V3 cp = { spos.x + World.instances[self].forward.x*0.48f, spos.y, spos.z + World.instances[self].forward.z*0.48f }; RaycastHit gh = Raycast(cp,(V3){0,-1,0},CELLSZ,LMASK_NPC_COLLISION); if (!gh.hit) { mv.x = 0.0f; mv.z = 0.0f; } } mv.y = World.velocity[sidx].y; World.velocity[sidx] = mv;
        } return;
    } if (!(World.instances[self].entflags & EF_WANDERING)) World.instances[self].currentState = AIState_Idle;
}

static void AIRunMove(u16 self) { if (World.instances[self].entflags & EF_ACT_AS_TURRET){return;} float rs=npcTable[World.instances[self].index - 419].runSpeed; World.velocity[self]=(V3){World.instances[self].forward.x * rs,(vabs(World.gravity[self]) > 0.05f) ? World.velocity[self].y : World.instances[self].forward.y * rs,World.instances[self].forward.z * rs}; }
static void AIHunt(Entity* self) {
    u16 sidx=(u16)(self - World.instances); u16 eidx=self->enemy; if(!eidx){return;} self->currentDestination = ai_is_cyber(self) ? World.position[eidx] : AIGetSearchPoint(self); if ((npcTable[self->index - 419].moveType == AIMoveType_None) || (self->entflags & EF_ACT_AS_TURRET) || (npcTable[self->index - 419].runSpeed <= 0.0f) || (V3_SqDist(ai_sight_pos(self), self->currentDestination) <= AI_STOP_DIST_SQ) || (!AIWithinAngleToTarget(self))){return;}
    float rs=npcTable[self->index - 419].runSpeed; World.velocity[sidx]=(V3){self->forward.x*rs,World.velocity[sidx].y,self->forward.z*rs};
}

float DistToEnemy(u16 self, u16 enem) { if(self >= World.instCount){return 100000.0f;} if(enem >= World.instCount){return 100000.0f;} V3 d = V3_AsubB(ai_sight_pos(&World.instances[self]),World.position[enem]); return V3_dot(d,d); }
static bool AICanAttack(u16 selfIdx, float dsq, u8 type, float* rangeToEnemy) {
    Entity* self = &World.instances[selfIdx]; *rangeToEnemy = DistToEnemy(selfIdx,self->enemy); if (*rangeToEnemy >= dsq) return false; AttType att = type == 3 ? npcTable[self->index - 419].attackType3 : (type == 2 ? npcTable[self->index - 419].attackType2 : npcTable[self->index - 419].attackType); if (att == Att_None) return false;
    if (type == 3) { if (*rangeToEnemy < 7.0f && att == Att_Ball) { int p = npcTable[self->index - 419].projectile3Prefab; if(p == 370 || p == 372 || p == 387 || p == 404){return false;} } } if (ai_is_cyber(self)/*Cyber enemies are dumb but aggressive.*/) return true;
    if ((!(self->entflags & EF_ENEM_IN_FRONT)) || (type >= 2 && !(self->entflags & EF_ENEM_IN_FOV))){return false;} float wait = type == 3 ? self->randWaitAtt3Finished : (type == 2 ? self->randWaitAtt2Finished : self->randWaitAtt1Finished); return wait < World.pauseRelativeTime;
}

static void AIBrakingMovement(Entity* self) { u16 sidx=(u16)(self - World.instances); u8 ni=self->index-419; if (ni == 1 || (ni >= 3 && ni <= 9) || (ni >= 11 && ni <= 13) || ni == 17 || ni == 23) { World.velocity[sidx].x *= 0.15f; World.velocity[sidx].z *= 0.15f; } }
static void AIStartAttack(Entity* self, int n) {
    AIBrakingMovement(self); NPCTable* npc = &npcTable[self->index - 419]; double between, toActual; switch (n) { case 1: between = npc->timeBetweenAttack1; toActual = npc->timeToActualAttack1; break; case 2: between = npc->timeBetweenAttack2; toActual = npc->timeToActualAttack2; break; default: between = npc->timeBetweenAttack3; toActual = npc->timeToActualAttack3; break; }
    self->attackFinished = World.pauseRelativeTime + between + toActual; self->gracePeriodFinished = World.pauseRelativeTime + toActual; self->currentState = (AIState)(AIState_Attack1 + (n - 1));
}

static void AIRun(u16 selfIdx) {
    Entity* self = &World.instances[selfIdx]; if ((AICheckPain(selfIdx)) || (self->entflags & EF_ASLEEP)){return;} if(!self->enemy){self->currentState = AIState_Idle; return;} if(self->tranquilizeFinished >= World.pauseRelativeTime && !ai_is_cyber(self)){return;}
    if (self->posCheckFinished <= World.pauseRelativeTime && !ai_is_cyber(self)) {
        self->posCheckFinished = World.pauseRelativeTime + AI_POS_CHECK_DELAY; float dToEn=V3_Dist(ai_sight_pos(self),World.position[self->enemy]); float dToLast=V3_Dist(World.position[selfIdx],self->lastPosition); self->lastPosition=World.position[selfIdx];
        if(dToLast < 0.48f && dToEn > AI_STOP_DIST && !(self->entflags & EF_WANDERING)){self->wanderFinished = World.pauseRelativeTime + 5.0f;/*Same search time as Quake 1*/ flag_set(&self->entflags,EF_WANDERING,true); self->currentDestination=AIGetSearchPoint(self);} else {flag_set(&self->entflags,EF_WANDERING,false);}
    }
    if (!(self->entflags & EF_ENEM_IN_SIGHT)) { if(self->huntFinished > World.pauseRelativeTime){AIHunt(self);} else {self->enemy=0; flag_set(&self->entflags,EF_WANDERING,true); self->wanderFinished=World.pauseRelativeTime + 1.0; self->currentState=AIState_Walk;} return; }
    if (self->enemy && !(self->entflags & EF_WANDERING)) { self->targettingPosition=(V3){World.position[self->enemy].x,World.position[self->enemy].y + AI_TARGET_OFFSET_Y,World.position[self->enemy].z}; self->currentDestination=self->targettingPosition; self->lastKnownEnemyPos=self->targettingPosition; }
    flag_set(&self->entflags,EF_SHOT_FIRED,false); AISetHuntFinished(selfIdx); NPCTable* ndat = &npcTable[self->index - 419]; float nr = ndat->range, near = nr * nr, mr = ndat->range2, mid  = mr * mr, fr = ndat->range3, far  = fr * fr, rangeToEnemy=100000.0f;
    if (AICanAttack(selfIdx,near,1,&rangeToEnemy)) { AIStartAttack(self,1); return; } if (AICanAttack(selfIdx, mid,2,&rangeToEnemy)) { AIStartAttack(self,2); return; } if (AICanAttack(selfIdx, far,3,&rangeToEnemy)) { AIStartAttack(self,3); return; }
    if (ndat->moveType != AIMoveType_None && rangeToEnemy > AI_STOP_DIST_SQ) { if (AIWithinAngleToTarget(self)) { if (ndat->hopsOnMove && !(World.instances[selfIdx].entflags & EF_ACT_AS_TURRET)){ AIHopMove(selfIdx); } else { AIRunMove(selfIdx); } } else if (World.diffCbt >= 2 && random_range(0.0f,1.0f) < 0.5f) { AIFace(self,self->currentDestination); } }
}

static void AIPain(Entity* self) { if (self->timeTillPainFinished < World.pauseRelativeTime) { self->currentState = AIState_Run; flag_set(&self->entflags, EF_GO_INTO_PAIN, false); self->timeTillPainFinished = World.pauseRelativeTime + npcTable[self->index - 419].timeBetweenPain; } }
/* Voxen entities are the visible mesh; there is no child visibleMesh to hide. */
static double AIDeathAnimationDuration(const Entity* self) {
    if (!self || self->animationNum >= MAX_ANIMS) return 0.0;
    AnimationClip clip = modelAnimationClips[self->animationNum][A_DYING];
    if (clip.framerate <= 0 || clip.speed <= 0 || clip.frameEnd <= clip.frameStart) return 0.0;
    return (double)(clip.frameEnd - clip.frameStart + 1) / ((double)clip.framerate * clip.speed);
}
static void SpawnNPCDeathBurst(Entity* self) {
    if (!self) return;
    if (self->index == 437) { /* npc_servbot prefab deathBurst child */
        const PSysDef* preset = PSysTypeGet(147);
        if (preset) { PSysDef def = *preset; V3 p=World.position[(u16)(self - World.instances)]; p.x-=self->right.x*.005f; p.y-=.032f; p.z-=self->right.z*.005f; p.x-=self->forward.x*.078f; p.z-=self->forward.z*.078f; def.pos = p; def.textures[0] = 386; def.emitRate = 0.0f; def.duration = 1.5f; def.burstCount = 80; def.colStart = (Color){1.0f,0.84076905f,0.8349056f,1.0f}; def.colEnd = (Color){1.0f,1.0f,1.0f,1.0f}; def.rotationMode = 1; def.colorMode = 1; def.blendMode = 4; def.blendModeOverride = true; PSysAdd(&def); }
    }
    if (self->deathBurst > 0) SpawnDynamicObject(self->deathBurst, false);
}
static void AIDying(u16 i) {
    if (!(World.instances[i].entflags & EF_DYING_SETUP)) {
        World.instances[i].enemy = 0; NPCTable* npc = &npcTable[World.instances[i].index - 419]; float dbt = deathBurstTimer[World.instances[i].index - 419]; if (dbt > 0.0f) { World.instances[i].deathBurstFinished = World.pauseRelativeTime + dbt; } else if (!(World.instances[i].entflags & EF_DEATH_BURST_DONE)) { SpawnNPCDeathBurst(&World.instances[i]); flag_set(&World.instances[i].entflags, EF_DEATH_BURST_DONE, true); }
        u16 sidx = i; if (!(World.instances[i].entflags & EF_ACT_AS_CORPSE_ONLY) && !(World.instances[i].entflags & EF_TELEPORT_ON_DEATH)) { int sded=sfxDeath[World.instances[i].index - 419]; if (sded >= 0 && sded < (i16)SOUNDS_COUNT){play_wav(sounds[sded],AppliedFXVol(1.0f),World.position[sidx],true);} } { u16 _nid = World.instances[i].index - 419; World.gravity[i] = (ai_is_cyber(&World.instances[i]) || ai_gibs_on_death(_nid)) ? 0.0f : 1.0f; } // Citadel: gibbed/flier corpses don't fall while dying; cyber never falls.
        flag_set(&World.instances[i].entflags,EF_ASLEEP,false); World.layer[i] = L_Corpse; flag_set(&World.instances[i].entflags,EF_FIRST_SIGHTING,true); u16 npcID = World.instances[i].index - 419; double deathWait = npc->timeTillDead; if (ai_gibs_on_death(npcID)) { double animWait = AIDeathAnimationDuration(&World.instances[i]); if (animWait > deathWait) deathWait = animWait; } World.instances[i].timeTillDeadFinished = World.pauseRelativeTime + deathWait; if (npc->switchMaterialOnDeath && npcDeathTexture[npcID] != U16_MAX) { World.instances[i].texIndex = npcDeathTexture[npcID]; }
        /* Citadel's zero-g death object is a 25-frame sequence at 24 fps. Voxen
         * has one mesh, so keep it visible and switch only its texture. Clip 48
         * is the available zerog37..zerog52 death tail (16 source frames). */
        if (npcID == 20) { TextureSequenceStart(i, 48); }
        /* Voxen has no blend-shape hierarchy for the hopper. Preserve its mesh
         * and drive the shader's red/rim effect from this pause-relative timer. */
        if (npcID == 14) { World.instances[i].deathAnimationActive = true; World.instances[i].deathAnimationStart = (float)World.pauseRelativeTime; }
        if (World.instances[i].index == 428 || World.instances[i].index == 439) World.velocity[sidx] = (V3){0.0f,World.velocity[sidx].y,0.0f}; // Prevent gibs on Exec bot or fake melt on Zero-G mutant from having horizontal movement (looks nicer).
        if (World.instances[i].index == 433) World.layer[i] = L_Corpse; // Hopper: enable capsule collider (implicit in layer change)
        flag_set(&World.instances[i].entflags, EF_DYING_SETUP, true);
    }
    if (World.instances[i].timeTillDeadFinished <= World.pauseRelativeTime) { flag_set(&World.instances[i].entflags,EF_DEAD,true); flag_set(&World.instances[i].entflags,EF_DYING,false); World.instances[i].currentState = AIState_Dead; } if (World.instances[i].index == 439) World.layer[i] = L_Corpse | L_CorpseSearchable; // Zero-G mutant enables search collider while still dying
}

static void AIDead(u16 idx) {
    Entity* self = &World.instances[idx]; flag_set(&World.instances[idx].entflags,EF_ASLEEP,false); flag_set(&World.instances[idx].entflags,EF_DEAD,true); flag_set(&World.instances[idx].entflags,EF_DYING,false); flag_set(&World.instances[idx].entflags,EF_DYING_SETUP,false); if (World.instances[idx].entflags & EF_DEAD_GIBS_DONE){return;} flag_set(&World.instances[idx].entflags,EF_DEAD_GIBS_DONE,true);
    World.instances[idx].currentState = AIState_Dead; World.layer[idx] = L_Corpse; if (World.instances[idx].entflags & EF_TELEPORT_ON_DEATH) { World.gravity[idx] = 1.0f; DeleteInstance(idx); /* TeleportAway not yet fully implemented; keep delete for now */ }
    else if (ai_is_cyber(self)) { World.gravity[idx] = 0.0f; DeleteInstance(idx); /* Gib effect: spawn basic debris using deathBurst index if defined */ }
    else if (ai_gibs_on_death(World.instances[idx].index - 419)) {
        // Spawn the complete gibObjects set only after the death animation has
        // finished. The primary/searchable member inherits the NPC contents.
        u16 npcID = World.instances[idx].index - 419; NPCGibRange range = npcGibRanges[npcID];
        V3 gibPos = World.position[idx]; Quaternion gibRot = World.rotation[idx];
        for (u16 gibConst = range.first; gibConst <= range.last; ++gibConst) {
            u16 gib = SpawnDynamicObject(gibConst, false);
            if (gib == 0xFFFF || gib >= INSTANCE_COUNT || gib == idx) continue;
            World.position[gib] = gibPos; World.rotation[gib] = gibRot;
            World.velocity[gib] = (V3){0.0f, World.velocity[idx].y, 0.0f}; World.gravity[gib] = 1.0f;
            World.layer[gib] = L_Corpse;
            Entity* g = &World.instances[gib];
            if (gibConst == range.primary) {
                for (int s = 0; s < 4; ++s) { g->contents[s] = self->contents[s]; g->custIdx[s] = self->custIdx[s]; self->contents[s] = self->custIdx[s] = -1; }
                g->lookUpIndex = self->lookUpIndex; g->maxRandomItems = self->maxRandomItems; g->generateContents = false;
                World.layer[gib] = L_Corpse | L_CorpseSearchable;
            }
        }
        DeleteInstance(idx);
    } else { /*Enable search collider for non-gib corpses (Avian Mutant index 2 always searchable)*/ World.layer[idx] = L_Corpse | L_CorpseSearchable; World.velocity[idx].x = 0.0f; World.velocity[idx].z = 0.0f; if (World.instances[idx].index != 433) World.gravity[idx] = 1.0f;/*Hopper deactivates itself*/ }
    flag_set(&World.instances[idx].entflags, EF_DEAD_CHECKS_DONE, true);
}

static DamageData SetNPCData(Entity* self, int n){DamageData dd={0}; NPCTable* npc=&npcTable[self->index - 419]; dd.owner=(u16)(self - World.instances); switch(n){case 1:dd.damage=npc->damage; dd.attackType=npc->attackType; break; case 2:dd.damage=npc->damage2; dd.attackType=npc->attackType2; break; default:dd.damage=npc->damage3; dd.attackType=npc->attackType3; break;} dd.penetration=0; dd.defense=0; dd.offense=0; dd.armorvalue=0; dd.berserkActive=false; return dd;}
static void ai_apply_damage(DamageData dd, u16 hitIdx) {
    if(!hitIdx || hitIdx >= INSTANCE_COUNT || hitIdx >= World.instCount){return;}
    if(hitIdx == dd.owner){return;} // Never hurt self (melee/projectile origin sits inside own capsule).
    if(!(World.instances[hitIdx].entflags & EF_ACTIVE)){return;}
    dd.hitIdx=hitIdx;
    bool hitPlayer=(hitIdx == PLAYER1);
    dd.isOtherNPC=!hitPlayer && IdxIsNPC(World.instances[hitIdx].index);
    if(dd.isOtherNPC){ NPCTable* nt=&npcTable[World.instances[hitIdx].index - 419]; dd.armorvalue=nt->armorvalue; dd.defense=nt->defense; }
    dd.damage=GetDamageTakeAmount(&dd);
    TakeDamage(hitIdx, dd);
}
static void AIApplyAttackMovement(Entity* self, float speed) { u16 eidx=self->enemy; if(!eidx)return; if(self->entflags & EF_ACT_AS_TURRET){self->currentDestination=ai_sight_pos(self); return;} if(speed<=0||self->tranquilizeFinished>=World.pauseRelativeTime)return; self->currentDestination=World.position[eidx]; if(V3_SqDist(ai_sight_pos(self),self->currentDestination)<=AI_STOP_DIST_SQ)return; if(!AIWithinAngleToTarget(self))return; AddForce((u16)(self-World.instances),V3_ScaleByF(self->forward,speed),false); }
static void AITransitionAttackToRun(Entity* self, int n) {
    flag_set(&self->entflags,EF_GO_INTO_PAIN,false); self->currentState=AIState_Run; NPCTable* npc=&npcTable[self->index - 419]; float chance,wmin,wmax,*wait;
    switch (n) {
        case 1:  chance=npc->timeAttack1WaitChance; wmin=npc->timeAttack1WaitMin; wmax=npc->timeAttack1WaitMax; wait=&self->randWaitAtt1Finished; break; case 2:  chance=npc->timeAttack2WaitChance; wmin=npc->timeAttack2WaitMin; wmax=npc->timeAttack2WaitMax; wait=&self->randWaitAtt2Finished; break; default: chance=npc->timeAttack3WaitChance; wmin=npc->timeAttack3WaitMin; wmax=npc->timeAttack3WaitMax; wait=&self->randWaitAtt3Finished; break;
    } *wait = (random_range(0.0f, 1.0f) < chance) ? World.pauseRelativeTime + random_range(wmin, wmax) : World.pauseRelativeTime;
}

static void ProjectileRaycast(Entity* self, int n) {
    if (n < 1 || n > 3){n = 1;} V3 spos = (n == 1) ? ai_sight_pos(self) : ai_gun_pos(self, n); u16 selfIdx=(u16)(self - World.instances); u16 eidx = self->enemy; V3 targ = eidx ? self->targettingPosition : (V3){spos.x + self->forward.x*10.0f,spos.y,spos.z + self->forward.z*10.0f}; V3 dir=(n == 1) ? self->forward : V3_Normalize(V3_AsubB(targ,spos)); float range;
    switch (n) { case 1: range = npcTable[self->index - 419].range; break; case 2: range = npcTable[self->index - 419].range2; break; default: range = npcTable[self->index - 419].range3; break; }
    // Origin sits inside own capsule; push start forward so we don't hit ourselves.
    V3 opos = {spos.x + dir.x*0.55f, spos.y + dir.y*0.55f, spos.z + dir.z*0.55f};
    RaycastHit hit = Raycast(opos, dir, range, LMASK_NPC_ATTACK); if(!hit.hit){return;} u16 hi = hit.hitInstanceIndex;
    if (hi == selfIdx){return;} // Wrong layer mask previously let melee hurt self; never hit owner.
    if (n == 3 && self->index == 427 && eidx) DrawLine(ai_sight_pos(self), World.position[eidx],(Color){1.0f,0.15f,0.18f,0.85f}); // Targeting laser (Cyborg Elite, attack3)
    DamageData dd = SetNPCData(self,n); dd.attackType=Att_HitS; // Citadel ProjectileRaycast always uses Projectile, even for Melee.
    dd.hitpoint=hit.point; dd.attacknormal=dir; dd.impactVelocity=dd.damage; bool hitPlayer=(hi == PLAYER1); if(hitPlayer){dd.impactVelocity *= 0.5f;} dd.isOtherNPC=!hitPlayer && IdxIsNPC(World.instances[hi].index);
    if (hi){ai_apply_damage(dd,hi);} u16 impactCI = GetImpactType(hi); if(impactCI){u16 imp = SpawnDynamicObject(impactCI,true); if(imp && imp < INSTANCE_COUNT){World.position[imp]=hit.point;}}
}

static void ProjectileLaunched(Entity* self, int n) {
    u16 sidx=(u16)(self - World.instances); NPCTable* npc = &npcTable[self->index - 419]; int masterIdx; float launchSpd; switch (n) { case 1: masterIdx = npc->projectile1Prefab; launchSpd = npc->projectileSpeedAttack1; break; case 2: masterIdx = npc->projectile2Prefab; launchSpd = npc->projectileSpeedAttack2; break; default: masterIdx = npc->projectile3Prefab; launchSpd = npc->projectileSpeedAttack3; break; }
    DamageData dd = SetNPCData(self,n); dd.attackType=Att_Ball; // Citadel ProjectileLaunched always uses ProjectileLaunched.
    V3 spos=ai_attack_pos(self,n); u16 eidx=self->enemy; V3 targ=eidx ? self->targettingPosition : (V3){spos.x + self->forward.x*20.0f,spos.y,spos.z + self->forward.z*20.0f}; V3 dir=V3_Normalize(V3_AsubB(targ,spos)); u16 bb = SpawnDynamicObject(masterIdx>0?masterIdx:370,false);
    if (bb==0xFFFF || bb==0 || bb>=INSTANCE_COUNT) bb=SpawnDynamicObject(370,false); if (bb==0xFFFF || bb==0 || bb>=INSTANCE_COUNT) return; Entity* proj=&World.instances[bb]; World.layer[bb]=L_NPCBullet; World.position[bb]=spos; proj->forward=dir;
    proj->damage=dd.damage; proj->strength=dd.penetration; proj->speed=dd.offense; proj->attackType=dd.attackType; proj->recentMostActivator=sidx; ProjectileEffectImpactInitAfterLoad(bb);
    V3 shove = V3_ScaleByF(dir, launchSpd); if (vabs(World.gravity[sidx]) > 0.05f) { shove.x += World.velocity[sidx].x; shove.z += World.velocity[sidx].z; } World.velocity[bb] = (V3){0,0,0}; AddForce(bb,shove,true); flag_set(&proj->entflags,EF_ACTIVE | EF_RIGIDBODY,true);
}

static void AIExplodeAttack(Entity* self) {
    u16 selfIdx=(u16)(self - World.instances);
    float radius = npcTable[self->index - 419].attack3Radius; float force=npcTable[self->index - 419].attack3Force; V3 epos = ai_sight_pos(self); DamageData dd = SetNPCData(self, 3);
    for (u16 i = INSTS_1ST_IDX; i < World.instCount; ++i) { if (i == selfIdx) continue; Entity* t = &World.instances[i]; if (!(t->entflags & EF_ACTIVE)) continue; float dsq = V3_SqDist(epos,World.position[i]); if(dsq>=radius*radius)continue; float dist=vsqrtf(dsq), falloff=1.0f-dist/radius; DamageData tdd=dd; tdd.damage *= falloff; ai_apply_damage(tdd,i); if (dist > 0.001f) AddForce(i,V3_ScaleByF(V3_Normalize(V3_AsubB(World.position[i],epos)),force * falloff),true); }
    DamageData selfdd = SetNPCData(self, 3); TakeDamage(selfIdx, selfdd); // Self-destruct through real pipeline (Citadel healthManager.TakeDamage).
}

static void AIMakeAttack(Entity* self, AttType att, int ind) { if (ind < 1 || ind > 3){ind=1;/*Melee hitscan by default.*/} switch (att) { case Att_Melee:ProjectileRaycast(self,ind); break; case Att_HitS: case Att_PjBm:ai_muzzle_flash(self,ind); ProjectileRaycast(self,ind); World.fogFac += 1; break; case Att_Ball:ai_muzzle_flash(self,ind); ProjectileLaunched(self,ind); World.fogFac += 1; break; default: break; } }
void AIAttack(Entity* self, int slot) {
    u16 sidx = (u16)(self - World.instances); NPCTable* npc = &npcTable[self->index - 419]; if (slot == 3 && npc->explodeOnAttack3) { World.fogFac += 5; AIExplodeAttack(self); return; } AIApplyAttackMovement(self, slot == 1 ? npc->attack1Speed : slot == 2 ? npc->attack2Speed : npc->attack3Speed); int sat = slot == 1 ? sfxAttack1[self->index - 419] : slot == 2 ? sfxAttack2[self->index - 419] : sfxAttack3[self->index - 419];
    float* s_time = slot == 1 ? &self->attack1SoundTime : (slot == 2 ? &self->attack2SoundTime : &self->attack3SoundTime); u32 tb = slot == 1 ? npc->timeBetweenAttack1 : slot == 2 ? npc->timeBetweenAttack2 : npc->timeBetweenAttack3;
    int configuredAttack=slot==1 ? npc->attackType : slot==2 ? npc->attackType2 : npc->attackType3; AttType attack=(AttType)configuredAttack;
    /* NPCTable keeps the serialized IDs read from Unity's enemy table. Map
       its projectile IDs to Voxen's hit-scan and launched-projectile paths. */
    if (configuredAttack==4) attack=Att_HitS; /* Unity Projectile */
    else if (configuredAttack==5) attack=Att_PjBm; /* Unity ProjectileEnergyBeam */
    else if (configuredAttack==7) attack=Att_Ball; /* Unity ProjectileLaunched */
    (self->gracePeriodFinished < World.pauseRelativeTime && !(self->entflags & EF_SHOT_FIRED)) ? (flag_set(&self->entflags,EF_SHOT_FIRED,true),(*s_time < World.pauseRelativeTime && sat >= 0 && sat < (i16)SOUNDS_COUNT) ? (play_wav(sounds[sat],AppliedFXVol(1.0f),World.position[sidx],true), *s_time=World.pauseRelativeTime + tb) : 0,AIMakeAttack(self,attack,slot)) : 0;
    (slot == 3 && self->enemy) ? (self->index == 427 ? DrawLine(ai_sight_pos(self),World.position[self->enemy],(Color){1.0f, 0.15f, 0.18f, 0.85f}) : self->index == 433 ? DrawLine(ai_sight_pos(self),World.position[self->enemy],(Color){0.96f,1.0f,0.0f,0.88f}) : (void)0) : (void)0; if (self->attackFinished < World.pauseRelativeTime) AITransitionAttackToRun(self,slot);
}

static void AIFlierMoveToHoverHeight(Entity* self) {
    u16 sidx=(u16)(self - World.instances); NPCTable* npc = &npcTable[self->index - 419]; if (npc->runSpeed <= 0.0f) return; u16 eidx = self->enemy;
    if (eidx) { self->idealPos.y = World.position[eidx].y + AI_TARGET_OFFSET_Y; self->idealPos.x=World.position[sidx].x; self->idealPos.z=World.position[sidx].z; }
    else if (NPCInPlayerPVS(sidx)) { V3 sp=ai_sight_pos(self), fp={0.0f,0.0f,0.0f}; RaycastHit dn=Raycast(sp,(V3){0,-1,0},npc->sightRange,LMASK_NPC_SIGHT); RaycastHit up=Raycast(sp,(V3){0,1,0},npc->sightRange,LMASK_NPC_SIGHT); float dDn=0.0f, dUp=0.0f; if (dn.hit) { dDn=dn.distance; fp=dn.point; } if (up.hit) dUp=up.distance; float yH=npc->flightHeight * (npc->flightHeightIsPercentage ? dDn + dUp : 1.0f); self->idealPos=(V3){fp.x,fp.y+yH,fp.z};}
    float dy = self->idealPos.y - World.position[sidx].y; if (vabs(dy) < 0.16f) return; float spd  = npc->runSpeed * (float)World.deltaTime * World.timeScale; float step = vmin(vabs(dy), spd) * (dy < 0.0f ? -1.0f : 1.0f); World.position[sidx].y += step;
}

float AITranquilize(u16 idx, float amount, bool energy) { Entity* self = &World.instances[idx]; float secs = (amount < 3.0f) ? (float)npcTable[self->index - 419].timeForTranquilization : amount; if (npcTable[self->index - 419].type != NPCType_Robot || energy) { double a = World.pauseRelativeTime + secs, b = self->tranquilizeFinished + secs; self->tranquilizeFinished = a > b ? a : b; return secs; } return 0.0f; }
void AIAlert(u16 idx) { if (!World.diffCbt){return;} Entity* self = &World.instances[idx]; AISetEnemy(idx,PLAYER1); self->currentDestination = World.position[PLAYER1]; flag_set(&self->entflags, EF_ENEM_IN_SIGHT, false); }
void AIAwakeFromSleep(u16 idx) { flag_set(&World.instances[idx].entflags,EF_ASLEEP,false); AIAlert(idx);/*deactivate sleeping cables*/ }
static void AIThink(u16 idx) {
    Entity* self = &World.instances[idx]; if ((self->entflags & EF_DYING_SETUP) && self->deathBurstFinished < World.pauseRelativeTime && !(self->entflags & EF_DEATH_BURST_DONE)) { SpawnNPCDeathBurst(self); flag_set(&self->entflags,EF_DEATH_BURST_DONE,true); }
    if (!ai_has_health(self)) { if (!(self->entflags & EF_DYING) && !(self->entflags & EF_DEAD)){flag_set(&self->entflags,EF_DYING,true); self->currentState=AIState_Dying;}else if((self->entflags & EF_DEAD) && self->currentState != AIState_Dead){self->currentState=AIState_Dead;}else if((self->entflags & EF_DYING) && self->currentState != AIState_Dying){self->currentState=AIState_Dying;} }
    switch (self->currentState) { case AIState_Idle:AIIdle(idx); break; case AIState_Walk:AIWalk(idx); break; case AIState_Run:AIRun(idx); break; case AIState_Attack1:AIAttack(self,1); break; case AIState_Attack2:AIAttack(self,2); break; case AIState_Attack3:AIAttack(self,3); break; case AIState_Pain:AIPain(self); break; case AIState_Dying:AIDying(idx); break; case AIState_Dead:AIDead(idx); break; default:AIIdle(idx); break; }
    if (self->currentState == AIState_Dead || self->currentState == AIState_Dying) return;
    if (self->entflags & EF_ASLEEP) return;
    if (npcTable[self->index - 419].moveType == AIMoveType_Fly && self->tranquilizeFinished < World.pauseRelativeTime) AIFlierMoveToHoverHeight(self);
}

void AIControllerUpdate(u16 idx) {
    ai_update_muzzle_lights(); Entity* self=&World.instances[idx]; if(!(self->entflags & EF_ACTIVE)){return;} u16 edx=self->index; if(!IdxIsNPC(edx)){return;} u16 ndx=edx-419;
    if(npcTable[ndx].type != NPCType_Cyber && npcTable[ndx].moveType != AIMoveType_Fly && self->currentState != AIState_Dead && self->currentState != AIState_Dying) World.gravity[idx] = 1.0f;
    if (self->tickTime < World.pauseRelativeTime) {
        self->tickTime = World.pauseRelativeTime + AI_RAYCAST_TICK_TIME;
        flag_set(&self->entflags,EF_ENEM_IN_SIGHT,AICheckIfPlayerInSight(idx)); u16 eidx=self->enemy;
        if (eidx && ai_has_health(self)) {
            bool enAlive = npcTable[ndx].type == NPCType_Cyber ? World.instances[eidx].cyberHealth > 0.0f : World.instances[eidx].health > 0.0f;
            if (!enAlive) { if (npcTable[ndx].type == NPCType_Cyber) self->currentState = AIState_Idle; else { flag_set(&self->entflags, EF_WANDERING, true); self->wanderFinished = World.pauseRelativeTime + random_range(3.0f, 8.0f); self->currentState = AIState_Walk; } self->enemy = 0; self->posCheckFinished = World.pauseRelativeTime; self->lastPosition = World.position[idx]; }
            else AIEnemyInFrontChecks(self,eidx);
        }
    }
    if (self->tickFinished < World.pauseRelativeTime) {
        self->tickFinished = World.pauseRelativeTime + AI_TICK_TIME;
        AIThink(idx);
    }
    if (self->currentState == AIState_Dead || self->currentState == AIState_Idle) return;
    u16 eidx=self->enemy; if ((self->entflags & EF_ACT_AS_TURRET) && eidx) self->currentDestination = (V3){World.position[eidx].x,World.position[eidx].y + AI_TARGET_OFFSET_Y,World.position[eidx].z}; if (npcTable[ndx].type == NPCType_Cyber && eidx) self->currentDestination = World.position[eidx];
    V3 toTarget = V3_AsubB(self->currentDestination,ai_sight_pos(self)); if (npcTable[ndx].type != NPCType_Cyber) toTarget.y = 0.0f; self->idealTransformForward = V3_Normalize(toTarget); float sqmag = V3_dot(toTarget, toTarget); if (sqmag > 1e-6f || npcTable[ndx].type == NPCType_Cyber) AIFace(self,self->currentDestination);
}
