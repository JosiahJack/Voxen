// entity.c - Entity Definitions and Save Load System for levels and savegames
#include "common.h"
#define LINE_LEN_MAX 81920
#define GEOMETRY_LOD_CARD_MODEL_IDX 178
Entity* entsFromFile; V3 *posFromFile, *scaleFromFile; Quaternion *rotationFromFile; Light *lightsFromFile; LightAnimation *lanimsFromFile; u16 headmountedLanternLight;
EPerms EDefs[MAX_ENTITIES] = { // EPerms struct order: modelIndex,colMeshIndex,texIndex,glowIndex,specIndex,normIndex,mass,dynFriction,statFriction,animationNum,col,colCtr,colSz
/*0 chunk_black*/[0]={.modelIndex=178,.colMeshIndex=0,.texIndex=0,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*1 chunk_blocker*/[1]={.modelIndex=178,.colMeshIndex=0,.texIndex=1230,.glowIndex=MAX_TXRS,.specIndex=1230,.normIndex=160,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*2 chunk_bridg1_1*/[2]={.modelIndex=661,.colMeshIndex=0,.texIndex=44,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=43,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*3 chunk_bridg1_1flipx*/[3]={.modelIndex=667,.colMeshIndex=0,.texIndex=44,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*4 chunk_bridg1_2*/[4]={.modelIndex=662,.colMeshIndex=0,.texIndex=45,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*5 chunk_bridg1_3*/[5]={.modelIndex=20,.colMeshIndex=0,.texIndex=47,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*6 chunk_bridg1_3_slice45*/[6]={.modelIndex=21,.colMeshIndex=0,.texIndex=47,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*7 chunk_bridg1_3flipx*/[7]={.modelIndex=663,.colMeshIndex=0,.texIndex=47,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*8 chunk_bridg1_4*/[8]={.modelIndex=22,.colMeshIndex=0,.texIndex=48,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*9 chunk_bridg1_4_slice32*/[9]={.modelIndex=23,.colMeshIndex=0,.texIndex=48,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*10 chunk_bridg1_4_slice32flipx*/[10]={.modelIndex=24,.colMeshIndex=0,.texIndex=48,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*11 chunk_bridg1_5*/[11]={.modelIndex=25,.colMeshIndex=0,.texIndex=50,.glowIndex=49,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*12 chunk_bridg2_2*/[12]={.modelIndex=26,.colMeshIndex=0,.texIndex=MAX_ANIMS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*13 chunk_bridg2_3*/[13]={.modelIndex=27,.colMeshIndex=0,.texIndex=56,.glowIndex=54,.specIndex=MAX_TXRS,.normIndex=55,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*14 chunk_bridg2_4*/[14]={.modelIndex=28,.colMeshIndex=0,.texIndex=57,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*15 chunk_bridg2_5*/[15]={.modelIndex=29,.colMeshIndex=0,.texIndex=59,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=58,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*16 chunk_bridg2_6*/[16]={.modelIndex=30,.colMeshIndex=0,.texIndex=60,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*17 chunk_bridg2_7*/[17]={.modelIndex=664,.colMeshIndex=0,.texIndex=61,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*18 chunk_bridg2_8*/[18]={.modelIndex=31,.colMeshIndex=0,.texIndex=62,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*19 chunk_bridg2_9*/[19]={.modelIndex=32,.colMeshIndex=0,.texIndex=64,.glowIndex=63,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*20 chunk_crate_impenetrable*/[20]={.modelIndex=61,.colMeshIndex=0,.texIndex=150,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*21 chunk_cyberpanel*/[21]={.modelIndex=178,.colMeshIndex=0,.texIndex=151,.glowIndex=151,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*22 chunk_cyberpanel_slice45*/[22]={.modelIndex=180,.colMeshIndex=0,.texIndex=152,.glowIndex=152,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*23 chunk_eng1_1*/[23]={.modelIndex=96,.colMeshIndex=0,.texIndex=254,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*24 chunk_eng1_1d*/[24]={.modelIndex=95,.colMeshIndex=0,.texIndex=253,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*25 chunk_eng1_2*/[25]={.modelIndex=98,.colMeshIndex=0,.texIndex=256,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*26 chunk_eng1_2d*/[26]={.modelIndex=97,.colMeshIndex=0,.texIndex=255,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*27 chunk_eng1_3*/[27]={.modelIndex=100,.colMeshIndex=0,.texIndex=259,.glowIndex=258,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*28 chunk_eng1_3d*/[28]={.modelIndex=99,.colMeshIndex=0,.texIndex=257,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*29 chunk_eng1_4*/[29]={.modelIndex=101,.colMeshIndex=0,.texIndex=260,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*30 chunk_eng1_5*/[30]={.modelIndex=103,.colMeshIndex=0,.texIndex=262,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*31 chunk_eng1_5_slice45lh*/[31]={.modelIndex=104,.colMeshIndex=0,.texIndex=262,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*32 chunk_eng1_5_slice45rh*/[32]={.modelIndex=105,.colMeshIndex=0,.texIndex=262,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*33 chunk_eng1_5d*/[33]={.modelIndex=102,.colMeshIndex=0,.texIndex=261,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*34 chunk_eng1_6*/[34]={.modelIndex=107,.colMeshIndex=0,.texIndex=266,.glowIndex=265,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*35 chunk_eng1_6d*/[35]={.modelIndex=106,.colMeshIndex=0,.texIndex=264,.glowIndex=263,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*36 chunk_eng1_7*/[36]={.modelIndex=108,.colMeshIndex=0,.texIndex=269,.glowIndex=268,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*37 chunk_eng1_7d*/[37]={.modelIndex=665,.colMeshIndex=0,.texIndex=267,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*38 chunk_eng1_8*/[38]={.modelIndex=109,.colMeshIndex=0,.texIndex=271,.glowIndex=270,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*39 chunk_eng1_9*/[39]={.modelIndex=111,.colMeshIndex=0,.texIndex=273,.glowIndex=251,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*40 chunk_eng1_9d*/[40]={.modelIndex=110,.colMeshIndex=0,.texIndex=272,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*41 chunk_eng2_1*/[41]={.modelIndex=113,.colMeshIndex=0,.texIndex=276,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*42 chunk_eng2_1_slice45*/[42]={.modelIndex=116,.colMeshIndex=0,.texIndex=276,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*43 chunk_eng2_1_slice384high*/[43]={.modelIndex=114,.colMeshIndex=0,.texIndex=276,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*44 chunk_eng2_1_slice384highrh*/[44]={.modelIndex=115,.colMeshIndex=0,.texIndex=276,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*45 chunk_eng2_1d*/[45]={.modelIndex=112,.colMeshIndex=0,.texIndex=275,.glowIndex=274,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*46 chunk_eng2_2*/[46]={.modelIndex=117,.colMeshIndex=0,.texIndex=279,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*47 chunk_eng2_2d*/[47]={.modelIndex=666,.colMeshIndex=0,.texIndex=277,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*48 chunk_eng2_3*/[48]={.modelIndex=119,.colMeshIndex=0,.texIndex=282,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*49 chunk_eng2_3d*/[49]={.modelIndex=118,.colMeshIndex=0,.texIndex=281,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*50 chunk_eng2_4*/[50]={.modelIndex=178,.colMeshIndex=0,.texIndex=283,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*51 chunk_eng2_5*/[51]={.modelIndex=120,.colMeshIndex=0,.texIndex=285,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=284,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*52 chunk_eng2_5_slice45*/[52]={.modelIndex=121,.colMeshIndex=0,.texIndex=285,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=284,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*53 chunk_eng2_6 (wall pump)*/[53]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=141,.glowIndex=142,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=21,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*54 chunk_exec1_1*/[54]={.modelIndex=124,.colMeshIndex=0,.texIndex=287,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*55 chunk_exec1_1d*/[55]={.modelIndex=123,.colMeshIndex=0,.texIndex=286,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*56 chunk_exec1_2*/[56]={.modelIndex=126,.colMeshIndex=0,.texIndex=291,.glowIndex=290,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*57 chunk_exec1_2d*/[57]={.modelIndex=125,.colMeshIndex=0,.texIndex=289,.glowIndex=288,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*58 chunk_exec2_1*/[58]={.modelIndex=127,.colMeshIndex=0,.texIndex=292,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*59 chunk_exec2_2*/[59]={.modelIndex=129,.colMeshIndex=0,.texIndex=295,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*60 chunk_exec2_2d*/[60]={.modelIndex=128,.colMeshIndex=0,.texIndex=294,.glowIndex=293,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*61 chunk_exec2_3*/[61]={.modelIndex=130,.colMeshIndex=0,.texIndex=296,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*62 chunk_exec2_4*/[62]={.modelIndex=131,.colMeshIndex=0,.texIndex=297,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*63 chunk_exec2_4_slice45*/[63]={.modelIndex=132,.colMeshIndex=0,.texIndex=297,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*64 chunk_exec2_5*/[64]={.modelIndex=133,.colMeshIndex=0,.texIndex=298,.glowIndex=MAX_TXRS,.specIndex=1257,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*65 chunk_exec2_6*/[65]={.modelIndex=134,.colMeshIndex=0,.texIndex=299,.glowIndex=MAX_TXRS,.specIndex=1257,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*66 chunk_exec2_7*/[66]={.modelIndex=133,.colMeshIndex=0,.texIndex=300,.glowIndex=MAX_TXRS,.specIndex=1257,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*67 chunk_exec3_1*/[67]={.modelIndex=127,.colMeshIndex=0,.texIndex=303,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*68 chunk_exec3_1d*/[68]={.modelIndex=135,.colMeshIndex=0,.texIndex=302,.glowIndex=301,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*69 chunk_exec3_2*/[69]={.modelIndex=129,.colMeshIndex=0,.texIndex=304,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*70 chunk_exec3_4*/[70]={.modelIndex=178,.colMeshIndex=0,.texIndex=305,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*71 chunk_exec4_1*/[71]={.modelIndex=136,.colMeshIndex=0,.texIndex=307,.glowIndex=306,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*72 chunk_exec4_2*/[72]={.modelIndex=137,.colMeshIndex=0,.texIndex=308,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*73 chunk_exec4_3*/[73]={.modelIndex=138,.colMeshIndex=0,.texIndex=309,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*74 chunk_exec4_4*/[74]={.modelIndex=139,.colMeshIndex=0,.texIndex=311,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*75 chunk_exec4_5*/[75]={.modelIndex=178,.colMeshIndex=0,.texIndex=312,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*76 chunk_exec4_6*/[76]={.modelIndex=141,.colMeshIndex=0,.texIndex=313,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*77 chunk_exec6_1*/[77]={.modelIndex=142,.colMeshIndex=0,.texIndex=315,.glowIndex=314,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*78 chunk_exteriorpanel1*/[78]={.modelIndex=131,.colMeshIndex=0,.texIndex=1228,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*79 chunk_fan1*/[79]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=96,.glowIndex=192,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=22,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*80 chunk_flight1_1*/[80]={.modelIndex=146,.colMeshIndex=0,.texIndex=319,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*81 chunk_flight1_1b*/[81]={.modelIndex=146,.colMeshIndex=0,.texIndex=318,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*82 chunk_flight1_2*/[82]={.modelIndex=147,.colMeshIndex=0,.texIndex=320,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*83 chunk_flight1_2_slice45rh*/[83]={.modelIndex=149,.colMeshIndex=0,.texIndex=320,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*84 unused*/[84]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*85 chunk_flight1_4*/[85]={.modelIndex=151,.colMeshIndex=0,.texIndex=322,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*86 chunk_flight1_5*/[86]={.modelIndex=147,.colMeshIndex=0,.texIndex=323,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*87 chunk_flight1_5_slice45lh*/[87]={.modelIndex=148,.colMeshIndex=0,.texIndex=323,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*88 chunk_flight1_6*/[88]={.modelIndex=152,.colMeshIndex=0,.texIndex=325,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*89 chunk_flight2_1*/[89]={.modelIndex=153,.colMeshIndex=0,.texIndex=326,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*90 chunk_flight2_2*/[90]={.modelIndex=154,.colMeshIndex=0,.texIndex=327,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*91 chunk_flight2_2_slice45*/[91]={.modelIndex=155,.colMeshIndex=0,.texIndex=327,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*92 chunk_flight2_3*/[92]={.modelIndex=156,.colMeshIndex=0,.texIndex=328,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*93 chunk_grove1_1*/[93]={.modelIndex=189,.colMeshIndex=0,.texIndex=362,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*94 chunk_grove1_2*/[94]={.modelIndex=178,.colMeshIndex=0,.texIndex=363,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*95 chunk_grove1_2_slice45*/[95]={.modelIndex=180,.colMeshIndex=0,.texIndex=363,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*96 chunk_grove1_3*/[96]={.modelIndex=178,.colMeshIndex=0,.texIndex=364,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*97 chunk_grove1_4*/[97]={.modelIndex=178,.colMeshIndex=0,.texIndex=365,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*98 chunk_grove1_5*/[98]={.modelIndex=178,.colMeshIndex=0,.texIndex=367,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*99 chunk_grove1_6*/[99]={.modelIndex=178,.colMeshIndex=0,.texIndex=368,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*100 chunk_grove1_7*/[100]={.modelIndex=178,.colMeshIndex=0,.texIndex=369,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*101 chunk_grove2_1*/[101]={.modelIndex=190,.colMeshIndex=0,.texIndex=370,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*102 chunk_grove2_2*/[102]={.modelIndex=190,.colMeshIndex=0,.texIndex=371,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*103 chunk_grove2_3*/[103]={.modelIndex=191,.colMeshIndex=0,.texIndex=372,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*104 chunk_grove2_4*/[104]={.modelIndex=341,.colMeshIndex=0,.texIndex=374,.glowIndex=373,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*105 chunk_grove2_5*/[105]={.modelIndex=192,.colMeshIndex=0,.texIndex=375,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*106 chunk_grove2_6*/[106]={.modelIndex=192,.colMeshIndex=0,.texIndex=376,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*107 chunk_grove2_7*/[107]={.modelIndex=191,.colMeshIndex=0,.texIndex=378,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*108 chunk_grove2_8*/[108]={.modelIndex=191,.colMeshIndex=0,.texIndex=379,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*109 chunk_grove2_9*/[109]={.modelIndex=191,.colMeshIndex=0,.texIndex=385,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*110 chunk_grove2_9b*/[110]={.modelIndex=191,.colMeshIndex=0,.texIndex=381,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*111 chunk_grove2_9c*/[111]={.modelIndex=191,.colMeshIndex=0,.texIndex=383,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*112 chunk_lift1*/[112]={.modelIndex=213,.colMeshIndex=0,.texIndex=1246,.glowIndex=1247,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*113 chunk_maint1_1*/[113]={.modelIndex=218,.colMeshIndex=0,.texIndex=430,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*114 chunk_maint1_2*/[114]={.modelIndex=220,.colMeshIndex=0,.texIndex=432,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*115 chunk_maint1_2d*/[115]={.modelIndex=219,.colMeshIndex=0,.texIndex=431,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*116 chunk_maint1_3*/[116]={.modelIndex=222,.colMeshIndex=0,.texIndex=436,.glowIndex=435,.specIndex=437,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*117 chunk_maint1_3b*/[117]={.modelIndex=221,.colMeshIndex=0,.texIndex=434,.glowIndex=433,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*118 chunk_maint1_4*/[118]={.modelIndex=224,.colMeshIndex=0,.texIndex=441,.glowIndex=440,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*119 chunk_maint1_4b*/[119]={.modelIndex=223,.colMeshIndex=0,.texIndex=439,.glowIndex=438,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*120 chunk_maint1_5*/[120]={.modelIndex=225,.colMeshIndex=0,.texIndex=443,.glowIndex=442,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*121 chunk_maint1_6*/[121]={.modelIndex=226,.colMeshIndex=0,.texIndex=96,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*122 chunk_maint1_7*/[122]={.modelIndex=227,.colMeshIndex=0,.texIndex=447,.glowIndex=446,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*123 chunk_blockerflightbay*/[123]={.modelIndex=178,.colMeshIndex=U16_MAX,.texIndex=1230,.glowIndex=MAX_TXRS,.specIndex=1242,.normIndex=160,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_BOX,.colCtr=(V3){0,1.44f,0},.colSz=(V3){2.56f,0.32f,2.56f}},
/*124 chunk_maint1_9*/[124]={.modelIndex=606,.colMeshIndex=0,.texIndex=450,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*125 chunk_maint1_9d*/[125]={.modelIndex=620,.colMeshIndex=0,.texIndex=449,.glowIndex=448,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*126 chunk_maint2_1*/[126]={.modelIndex=230,.colMeshIndex=0,.texIndex=455,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*127 chunk_maint2_1b*/[127]={.modelIndex=228,.colMeshIndex=0,.texIndex=451,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*128 chunk_maint2_1d*/[128]={.modelIndex=229,.colMeshIndex=0,.texIndex=453,.glowIndex=452,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*129 chunk_maint2_2*/[129]={.modelIndex=230,.colMeshIndex=0,.texIndex=457,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*130 chunk_maint2_3*/[130]={.modelIndex=232,.colMeshIndex=0,.texIndex=460,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*131 chunk_maint2_3d*/[131]={.modelIndex=231,.colMeshIndex=0,.texIndex=459,.glowIndex=458,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*132 chunk_maint2_4*/[132]={.modelIndex=233,.colMeshIndex=0,.texIndex=464,.glowIndex=463,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*133 chunk_maint2_4d*/[133]={.modelIndex=233,.colMeshIndex=0,.texIndex=462,.glowIndex=461,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*134 chunk_maint2_5*/[134]={.modelIndex=235,.colMeshIndex=0,.texIndex=468,.glowIndex=467,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*135 chunk_maint2_5d*/[135]={.modelIndex=234,.colMeshIndex=0,.texIndex=466,.glowIndex=465,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*136 chunk_maint2_6*/[136]={.modelIndex=236,.colMeshIndex=0,.texIndex=472,.glowIndex=471,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*137 chunk_maint2_6d*/[137]={.modelIndex=238,.colMeshIndex=0,.texIndex=470,.glowIndex=470,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*138 chunk_maint2_7*/[138]={.modelIndex=238,.colMeshIndex=0,.texIndex=476,.glowIndex=475,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*139 chunk_maint2_7d*/[139]={.modelIndex=237,.colMeshIndex=0,.texIndex=474,.glowIndex=473,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*140 chunk_maint2_8*/[140]={.modelIndex=239,.colMeshIndex=0,.texIndex=478,.glowIndex=477,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*141 chunk_maint2_9*/[141]={.modelIndex=240,.colMeshIndex=0,.texIndex=480,.glowIndex=479,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*142 chunk_maint2_9_slice45RH*/[142]={.modelIndex=242,.colMeshIndex=0,.texIndex=480,.glowIndex=479,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*143 chunk_maint2_9_slice128_top*/[143]={.modelIndex=241,.colMeshIndex=0,.texIndex=480,.glowIndex=479,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*144 chunk_maint3_1*/[144]={.modelIndex=244,.colMeshIndex=0,.texIndex=483,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*145 chunk_maint3_1_slice32_lh*/[145]={.modelIndex=246,.colMeshIndex=0,.texIndex=483,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*146 chunk_maint3_1_slice32_rh*/[146]={.modelIndex=245,.colMeshIndex=0,.texIndex=483,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*147 chunk_maint3_1_slice45*/[147]={.modelIndex=247,.colMeshIndex=0,.texIndex=483,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*148 chunk_maint3_1d*/[148]={.modelIndex=243,.colMeshIndex=0,.texIndex=482,.glowIndex=481,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*149 chunk_med1_1*/[149]={.modelIndex=249,.colMeshIndex=0,.texIndex=486,.glowIndex=MAX_TXRS,.specIndex=1256,.normIndex=1255,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*150 chunk_med1_1_half_top*/[150]={.modelIndex=250,.colMeshIndex=0,.texIndex=486,.glowIndex=MAX_TXRS,.specIndex=1256,.normIndex=1255,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*151 chunk_med1_1_slice128high*/[151]={.modelIndex=251,.colMeshIndex=0,.texIndex=486,.glowIndex=MAX_TXRS,.specIndex=1256,.normIndex=1255,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*152 chunk_med1_1_slice192RH*/[152]={.modelIndex=252,.colMeshIndex=0,.texIndex=486,.glowIndex=MAX_TXRS,.specIndex=1256,.normIndex=1255,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*153 chunk_med1_1_slice256*/[153]={.modelIndex=253,.colMeshIndex=0,.texIndex=486,.glowIndex=MAX_TXRS,.specIndex=1256,.normIndex=1255,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*154 chunk_med1_1d*/[154]={.modelIndex=248,.colMeshIndex=0,.texIndex=485,.glowIndex=484,.specIndex=1236,.normIndex=1255,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*155 chunk_med1_2*/[155]={.modelIndex=255,.colMeshIndex=0,.texIndex=489,.glowIndex=488,.specIndex=1256,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*156 chunk_med1_2d*/[156]={.modelIndex=254,.colMeshIndex=0,.texIndex=487,.glowIndex=MAX_TXRS,.specIndex=1256,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*157 chunk_med1_3*/[157]={.modelIndex=257,.colMeshIndex=0,.texIndex=493,.glowIndex=492,.specIndex=1256,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*158 chunk_med1_3d*/[158]={.modelIndex=256,.colMeshIndex=0,.texIndex=491,.glowIndex=490,.specIndex=1256,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*159 chunk_med1_4*/[159]={.modelIndex=258,.colMeshIndex=0,.texIndex=494,.glowIndex=MAX_TXRS,.specIndex=1256,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*160 chunk_med1_5*/[160]={.modelIndex=669,.colMeshIndex=0,.texIndex=495,.glowIndex=MAX_TXRS,.specIndex=1256,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*161 chunk_med1_6*/[161]={.modelIndex=259,.colMeshIndex=0,.texIndex=496,.glowIndex=MAX_TXRS,.specIndex=1256,.normIndex=509,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*162 chunk_med1_7*/[162]={.modelIndex=262,.colMeshIndex=0,.texIndex=499,.glowIndex=MAX_TXRS,.specIndex=1268,.normIndex=498,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*163 chunk_med1_7_slice14_64*/[163]={.modelIndex=263,.colMeshIndex=0,.texIndex=499,.glowIndex=MAX_TXRS,.specIndex=1268,.normIndex=1254,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*164 chunk_med1_7_slice45_320lh*/[164]={.modelIndex=264,.colMeshIndex=0,.texIndex=499,.glowIndex=MAX_TXRS,.specIndex=1268,.normIndex=1254,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*165 chunk_med1_7_slice45_320rh*/[165]={.modelIndex=265,.colMeshIndex=0,.texIndex=499,.glowIndex=MAX_TXRS,.specIndex=1268,.normIndex=1254,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*166 chunk_med1_7_slice96high*/[166]={.modelIndex=266,.colMeshIndex=0,.texIndex=499,.glowIndex=MAX_TXRS,.specIndex=1268,.normIndex=1254,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*167 chunk_med1_7d*/[167]={.modelIndex=260,.colMeshIndex=0,.texIndex=497,.glowIndex=MAX_TXRS,.specIndex=1269,.normIndex=1270,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*168 chunk_med1_7d_slice128*/[168]={.modelIndex=261,.colMeshIndex=0,.texIndex=497,.glowIndex=MAX_TXRS,.specIndex=1269,.normIndex=1270,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*169 chunk_med1_8*/[169]={.modelIndex=268,.colMeshIndex=0,.texIndex=503,.glowIndex=MAX_TXRS,.specIndex=1242,.normIndex=502,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*170 chunk_med1_8d*/[170]={.modelIndex=267,.colMeshIndex=0,.texIndex=501,.glowIndex=MAX_TXRS,.specIndex=1242,.normIndex=163,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*171 chunk_med1_9*/[171]={.modelIndex=278,.colMeshIndex=0,.texIndex=507,.glowIndex=MAX_TXRS,.specIndex=1267,.normIndex=506,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*172*/[172]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*173*/[173]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*174 chunk_med1_9d*/[174]={.modelIndex=269,.colMeshIndex=0,.texIndex=505,.glowIndex=MAX_TXRS,.specIndex=1267,.normIndex=504,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*175 unused*/[175]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*176 chunk_med1_9d_ofs112_90*/[176]={.modelIndex=270,.colMeshIndex=0,.texIndex=505,.glowIndex=MAX_TXRS,.specIndex=1267,.normIndex=504,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*177 chunk_med1_9d_ofs144_90*/[177]={.modelIndex=272,.colMeshIndex=0,.texIndex=505,.glowIndex=MAX_TXRS,.specIndex=1267,.normIndex=504,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*178 chunk_med2_1*/[178]={.modelIndex=280,.colMeshIndex=0,.texIndex=513,.glowIndex=511,.specIndex=1254,.normIndex=512,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*179 chunk_med2_1_slice32RH*/[179]={.modelIndex=281,.colMeshIndex=0,.texIndex=513,.glowIndex=MAX_TXRS,.specIndex=1254,.normIndex=512,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*180 chunk_med2_1d*/[180]={.modelIndex=279,.colMeshIndex=0,.texIndex=510,.glowIndex=508,.specIndex=1254,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*181 chunk_med2_2*/[181]={.modelIndex=283,.colMeshIndex=0,.texIndex=517,.glowIndex=516,.specIndex=1242,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*182 chunk_med2_2_half_bottom*/[182]={.modelIndex=284,.colMeshIndex=0,.texIndex=517,.glowIndex=516,.specIndex=1242,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*183 chunk_med2_2d*/[183]={.modelIndex=282,.colMeshIndex=0,.texIndex=515,.glowIndex=516,.specIndex=1242,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*184 chunk_med2_3*/[184]={.modelIndex=286,.colMeshIndex=0,.texIndex=521,.glowIndex=520,.specIndex=1242,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*185 chunk_med2_3d*/[185]={.modelIndex=285,.colMeshIndex=0,.texIndex=519,.glowIndex=518,.specIndex=1242,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*186 chunk_med2_4*/[186]={.modelIndex=287,.colMeshIndex=0,.texIndex=523,.glowIndex=522,.specIndex=1242,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*187 chunk_med2_5*/[187]={.modelIndex=288,.colMeshIndex=0,.texIndex=527,.glowIndex=526,.specIndex=539,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_BOX,.colCtr=(V3){0,1.44f,0},.colSz=(V3){2.56f,0.32f,2.56f}},
/*188 chunk_med2_6*/[188]={.modelIndex=289,.colMeshIndex=0,.texIndex=528,.glowIndex=MAX_TXRS,.specIndex=1271,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*189 chunk_med2_7*/[189]={.modelIndex=290,.colMeshIndex=0,.texIndex=530,.glowIndex=529,.specIndex=1245,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*190 chunk_med2_8*/[190]={.modelIndex=291,.colMeshIndex=0,.texIndex=531,.glowIndex=MAX_TXRS,.specIndex=1242,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*191 chunk_med2_8_half_top*/[191]={.modelIndex=292,.colMeshIndex=0,.texIndex=531,.glowIndex=MAX_TXRS,.specIndex=1242,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*192 chunk_med2_8_slice32RH*/[192]={.modelIndex=293,.colMeshIndex=0,.texIndex=531,.glowIndex=MAX_TXRS,.specIndex=1242,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*193 chunk_med2_8_slice45*/[193]={.modelIndex=294,.colMeshIndex=0,.texIndex=531,.glowIndex=MAX_TXRS,.specIndex=1242,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*194 chunk_med2_9*/[194]={.modelIndex=296,.colMeshIndex=0,.texIndex=535,.glowIndex=534,.specIndex=1242,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*195 chunk_med2_9d*/[195]={.modelIndex=295,.colMeshIndex=0,.texIndex=533,.glowIndex=532,.specIndex=1242,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*196 chunk_med3_1*/[196]={.modelIndex=297,.colMeshIndex=0,.texIndex=536,.glowIndex=MAX_TXRS,.specIndex=1236,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*197 chunk_rad1_1*/[197]={.modelIndex=501,.colMeshIndex=0,.texIndex=660,.glowIndex=659,.specIndex=1231,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*198 chunk_rad1_2*/[198]={.modelIndex=501,.colMeshIndex=0,.texIndex=662,.glowIndex=661,.specIndex=1231,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*199 chunk_reac1_1*/[199]={.modelIndex=502,.colMeshIndex=0,.texIndex=664,.glowIndex=MAX_TXRS,.specIndex=1243,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*200 chunk_reac1_1_slice45*/[200]={.modelIndex=339,.colMeshIndex=0,.texIndex=664,.glowIndex=MAX_TXRS,.specIndex=1243,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*201 chunk_reac1_2*/[201]={.modelIndex=503,.colMeshIndex=0,.texIndex=665,.glowIndex=MAX_TXRS,.specIndex=1243,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*202 chunk_reac1_3*/[202]={.modelIndex=504,.colMeshIndex=0,.texIndex=666,.glowIndex=MAX_TXRS,.specIndex=1243,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*203 chunk_reac1_4*/[203]={.modelIndex=505,.colMeshIndex=0,.texIndex=668,.glowIndex=667,.specIndex=669,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*204 chunk_reac1_5*/[204]={.modelIndex=506,.colMeshIndex=0,.texIndex=671,.glowIndex=670,.specIndex=1239,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*205 chunk_reac1_6*/[205]={.modelIndex=507,.colMeshIndex=0,.texIndex=673,.glowIndex=672,.specIndex=1243,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*206 chunk_reac1_7*/[206]={.modelIndex=342,.colMeshIndex=0,.texIndex=676,.glowIndex=675,.specIndex=1243,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*207 chunk_reac1_8*/[207]={.modelIndex=508,.colMeshIndex=0,.texIndex=678,.glowIndex=678,.specIndex=1243,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*208 chunk_reac1_9*/[208]={.modelIndex=509,.colMeshIndex=0,.texIndex=680,.glowIndex=680,.specIndex=1243,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*209 chunk_reac2_1*/[209]={.modelIndex=512,.colMeshIndex=0,.texIndex=682,.glowIndex=MAX_TXRS,.specIndex=1235,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*210 chunk_reac2_1_slice45LH*/[210]={.modelIndex=514,.colMeshIndex=0,.texIndex=682,.glowIndex=MAX_TXRS,.specIndex=1235,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*211 chunk_reac2_1_slice45LH_up*/[211]={.modelIndex=515,.colMeshIndex=0,.texIndex=682,.glowIndex=MAX_TXRS,.specIndex=1235,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*212 chunk_reac2_1_slice45RH*/[212]={.modelIndex=516,.colMeshIndex=0,.texIndex=682,.glowIndex=MAX_TXRS,.specIndex=1235,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*213 chunk_reac2_1_slice45RH_up*/[213]={.modelIndex=517,.colMeshIndex=0,.texIndex=682,.glowIndex=MAX_TXRS,.specIndex=1235,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*214 chunk_reac2_1b*/[214]={.modelIndex=510,.colMeshIndex=0,.texIndex=681,.glowIndex=MAX_TXRS,.specIndex=1235,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*215 chunk_reac2_1bmirror*/[215]={.modelIndex=511,.colMeshIndex=0,.texIndex=681,.glowIndex=MAX_TXRS,.specIndex=1235,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*216 chunk_reac2_1mirror*/[216]={.modelIndex=513,.colMeshIndex=0,.texIndex=682,.glowIndex=MAX_TXRS,.specIndex=1235,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*217 chunk_reac2_2*/[217]={.modelIndex=518,.colMeshIndex=0,.texIndex=684,.glowIndex=683,.specIndex=1235,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*218 chunk_reac2_4*/[218]={.modelIndex=519,.colMeshIndex=0,.texIndex=685,.glowIndex=MAX_TXRS,.specIndex=1235,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*219 chunk_reac2_4_slice128lower*/[219]={.modelIndex=340,.colMeshIndex=0,.texIndex=685,.glowIndex=MAX_TXRS,.specIndex=1235,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*220 chunk_reac2_5*/[220]={.modelIndex=520,.colMeshIndex=0,.texIndex=687,.glowIndex=686,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*221 chunk_reac2_6*/[221]={.modelIndex=521,.colMeshIndex=0,.texIndex=689,.glowIndex=688,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*222 chunk_reac2_7*/[222]={.modelIndex=522,.colMeshIndex=0,.texIndex=691,.glowIndex=690,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*223 chunk_reac2_8*/[223]={.modelIndex=523,.colMeshIndex=0,.texIndex=693,.glowIndex=692,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*224 chunk_reac2_9*/[224]={.modelIndex=524,.colMeshIndex=0,.texIndex=694,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*225 chunk_reac3_1*/[225]={.modelIndex=525,.colMeshIndex=0,.texIndex=696,.glowIndex=695,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*226 chunk_reac3_2*/[226]={.modelIndex=526,.colMeshIndex=0,.texIndex=697,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*227 chunk_reac3_3*/[227]={.modelIndex=527,.colMeshIndex=0,.texIndex=698,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*228 chunk_reac3_4*/[228]={.modelIndex=528,.colMeshIndex=0,.texIndex=699,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*229 chunk_reac3_5*/[229]={.modelIndex=529,.colMeshIndex=0,.texIndex=701,.glowIndex=700,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*230 chunk_reac3_6*/[230]={.modelIndex=530,.colMeshIndex=0,.texIndex=703,.glowIndex=702,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*231 chunk_reac3_7*/[231]={.modelIndex=531,.colMeshIndex=0,.texIndex=704,.glowIndex=MAX_TXRS,.specIndex=705,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*232 chunk_reac4_1*/[232]={.modelIndex=532,.colMeshIndex=0,.texIndex=707,.glowIndex=706,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*233 chunk_reac4_1_slice45lh*/[233]={.modelIndex=533,.colMeshIndex=0,.texIndex=707,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*234 chunk_reac4_2*/[234]={.modelIndex=534,.colMeshIndex=0,.texIndex=709,.glowIndex=708,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*235 chunk_reac5_1*/[235]={.modelIndex=535,.colMeshIndex=0,.texIndex=711,.glowIndex=710,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*236 chunk_reac5_2*/[236]={.modelIndex=536,.colMeshIndex=0,.texIndex=713,.glowIndex=712,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*237 chunk_reac5_3*/[237]={.modelIndex=537,.colMeshIndex=0,.texIndex=715,.glowIndex=714,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*238 chunk_reac6_1*/[238]={.modelIndex=538,.colMeshIndex=0,.texIndex=716,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*239 chunk_reac6_2*/[239]={.modelIndex=539,.colMeshIndex=0,.texIndex=717,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*240 chunk_reac6_3*/[240]={.modelIndex=539,.colMeshIndex=0,.texIndex=719,.glowIndex=718,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*241 chunk_sci1_1*/[241]={.modelIndex=540,.colMeshIndex=0,.texIndex=722,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*242 chunk_sci1_1_slice45_toplh*/[242]={.modelIndex=542,.colMeshIndex=0,.texIndex=722,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*243 chunk_sci1_1_slice45_toprh*/[243]={.modelIndex=543,.colMeshIndex=0,.texIndex=722,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*244 chunk_sci1_1d*/[244]={.modelIndex=541,.colMeshIndex=0,.texIndex=721,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*245 chunk_sci1_2*/[245]={.modelIndex=545,.colMeshIndex=0,.texIndex=724,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*246 chunk_sci1_2_slice45lh*/[246]={.modelIndex=546,.colMeshIndex=0,.texIndex=724,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*247 chunk_sci1_2_slice45lh_up*/[247]={.modelIndex=547,.colMeshIndex=0,.texIndex=724,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*248 chunk_sci1_2_slice45rh*/[248]={.modelIndex=548,.colMeshIndex=0,.texIndex=724,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*249 chunk_sci1_2_slice45rh_up*/[249]={.modelIndex=549,.colMeshIndex=0,.texIndex=724,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*250 chunk_sci1_2d*/[250]={.modelIndex=544,.colMeshIndex=0,.texIndex=723,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*251 chunk_sci1_3*/[251]={.modelIndex=550,.colMeshIndex=0,.texIndex=726,.glowIndex=725,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*252 chunk_sci1_4*/[252]={.modelIndex=498,.colMeshIndex=0,.texIndex=727,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*253 chunk_sci1_5*/[253]={.modelIndex=551,.colMeshIndex=0,.texIndex=728,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*254 chunk_sci1_6*/[254]={.modelIndex=552,.colMeshIndex=0,.texIndex=729,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*255 chunk_sci1_6_slice45*/[255]={.modelIndex=553,.colMeshIndex=0,.texIndex=729,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*256 chunk_sci1_7*/[256]={.modelIndex=555,.colMeshIndex=0,.texIndex=731,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*257 chunk_sci1_7d*/[257]={.modelIndex=554,.colMeshIndex=0,.texIndex=730,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*258 chunk_sci1_8*/[258]={.modelIndex=557,.colMeshIndex=0,.texIndex=734,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*259 chunk_sci1_8d*/[259]={.modelIndex=556,.colMeshIndex=0,.texIndex=733,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*260 chunk_sci1_9*/[260]={.modelIndex=559,.colMeshIndex=0,.texIndex=737,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*261 chunk_sci1_9d*/[261]={.modelIndex=558,.colMeshIndex=0,.texIndex=736,.glowIndex=735,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*262 chunk_sci2_1*/[262]={.modelIndex=561,.colMeshIndex=0,.texIndex=739,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*263 chunk_sci2_1_slice45lh*/[263]={.modelIndex=563,.colMeshIndex=0,.texIndex=739,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*264 chunk_sci2_1_slice45rh*/[264]={.modelIndex=562,.colMeshIndex=0,.texIndex=739,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*265 chunk_sci2_1d*/[265]={.modelIndex=560,.colMeshIndex=0,.texIndex=738,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*266 chunk_sci2_2*/[266]={.modelIndex=565,.colMeshIndex=0,.texIndex=742,.glowIndex=741,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*267 chunk_sci2_2d*/[267]={.modelIndex=564,.colMeshIndex=0,.texIndex=740,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*268 chunk_sci2_3*/[268]={.modelIndex=566,.colMeshIndex=0,.texIndex=744,.glowIndex=743,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*269 chunk_sci2_4*/[269]={.modelIndex=567,.colMeshIndex=0,.texIndex=745,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*270 chunk_sci2_5*/[270]={.modelIndex=569,.colMeshIndex=0,.texIndex=747,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*271 chunk_sci2_5d*/[271]={.modelIndex=568,.colMeshIndex=0,.texIndex=746,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*272 chunk_sci3_1*/[272]={.modelIndex=571,.colMeshIndex=0,.texIndex=749,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*273 chunk_sci3_1d*/[273]={.modelIndex=570,.colMeshIndex=0,.texIndex=748,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*274 chunk_sci3_2*/[274]={.modelIndex=572,.colMeshIndex=0,.texIndex=750,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*275 chunk_sci3_3*/[275]={.modelIndex=573,.colMeshIndex=0,.texIndex=752,.glowIndex=751,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*276 chunk_sci3_4*/[276]={.modelIndex=574,.colMeshIndex=0,.texIndex=754,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*277 chunk_sci3_5*/[277]={.modelIndex=575,.colMeshIndex=0,.texIndex=756,.glowIndex=755,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*278 chunk_sci3_6*/[278]={.modelIndex=576,.colMeshIndex=0,.texIndex=758,.glowIndex=757,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*279 chunk_screen*/[279]={.modelIndex=5988,.colMeshIndex=0,.texIndex=881,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*280 chunk_sec1_1*/[280]={.modelIndex=178,.colMeshIndex=0,.texIndex=787,.glowIndex=MAX_TXRS,.specIndex=787,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*281 chunk_sec1_1b*/[281]={.modelIndex=178,.colMeshIndex=0,.texIndex=785,.glowIndex=MAX_TXRS,.specIndex=785,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*282 chunk_sec1_1c*/[282]={.modelIndex=577,.colMeshIndex=0,.texIndex=786,.glowIndex=MAX_TXRS,.specIndex=786,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*283 chunk_sec1_1c_slice45*/[283]={.modelIndex=580,.colMeshIndex=0,.texIndex=786,.glowIndex=MAX_TXRS,.specIndex=786,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*284 chunk_sec1_1c_slice64highlh*/[284]={.modelIndex=581,.colMeshIndex=0,.texIndex=786,.glowIndex=MAX_TXRS,.specIndex=786,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*285 chunk_sec1_1c_slice64highrh*/[285]={.modelIndex=582,.colMeshIndex=0,.texIndex=786,.glowIndex=MAX_TXRS,.specIndex=786,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*286 unused*/[286]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*287 unused*/[287]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*288 chunk_sec1_2*/[288]={.modelIndex=584,.colMeshIndex=0,.texIndex=789,.glowIndex=MAX_TXRS,.specIndex=1233,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*289 chunk_sec1_2b*/[289]={.modelIndex=583,.colMeshIndex=0,.texIndex=788,.glowIndex=MAX_TXRS,.specIndex=1233,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*290 chunk_sec1_3*/[290]={.modelIndex=585,.colMeshIndex=0,.texIndex=790,.glowIndex=MAX_TXRS,.specIndex=1233,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*291 chunk_sec1_3_slice45*/[291]={.modelIndex=586,.colMeshIndex=0,.texIndex=790,.glowIndex=MAX_TXRS,.specIndex=1233,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*292 chunk_stor1_1*/[292]={.modelIndex=597,.colMeshIndex=0,.texIndex=824,.glowIndex=823,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*293 chunk_stor1_2*/[293]={.modelIndex=598,.colMeshIndex=0,.texIndex=825,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*294 chunk_stor1_3*/[294]={.modelIndex=598,.colMeshIndex=0,.texIndex=826,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*295 chunk_stor1_4*/[295]={.modelIndex=599,.colMeshIndex=0,.texIndex=827,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*296 chunk_stor1_5*/[296]={.modelIndex=600,.colMeshIndex=0,.texIndex=828,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*297 chunk_stor1_6*/[297]={.modelIndex=601,.colMeshIndex=0,.texIndex=829,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*298 chunk_stor1_6_slice128_up_lh*/[298]={.modelIndex=602,.colMeshIndex=0,.texIndex=829,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*299 chunk_stor1_6_slice128_up_rh*/[299]={.modelIndex=603,.colMeshIndex=0,.texIndex=829,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*300 chunk_stor1_6_slice192lh*/[300]={.modelIndex=604,.colMeshIndex=0,.texIndex=829,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*301 chunk_stor1_6_slice192rh*/[301]={.modelIndex=605,.colMeshIndex=0,.texIndex=829,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*302 chunk_stor1_7*/[302]={.modelIndex=606,.colMeshIndex=0,.texIndex=833,.glowIndex=MAX_TXRS,.specIndex=834,.normIndex=832,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*303 chunk_stor1_7_slice45*/[303]={.modelIndex=607,.colMeshIndex=0,.texIndex=833,.glowIndex=MAX_TXRS,.specIndex=834,.normIndex=832,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*304 chunk_stor1_7d*/[304]={.modelIndex=620,.colMeshIndex=0,.texIndex=831,.glowIndex=830,.specIndex=834,.normIndex=832,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*305 chunk_teleporter*/[305]={.modelIndex=178,.colMeshIndex=0,.texIndex=1166,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*306 chunk_white*/[306]={.modelIndex=178,.colMeshIndex=0,.texIndex=881,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*307 item_paper_wad*/[307]={.modelIndex=487,.colMeshIndex=0,.texIndex=1250,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.06f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_SPH,.colCtr=(V3){-0.001254f,-0.001190498f,0.006335999f},.colSz=(V3){0.0451f,0,0}},
/*308 item_warecasing*/[308]={.modelIndex=637,.colMeshIndex=0,.texIndex=1251,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.8f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_BOX,.colCtr=(V3){0,0,0.09397449f},.colSz=(V3){0.540964f,0.405398f,0.187949f}},
/*309 item_beaker*/[309]={.modelIndex=14,.colMeshIndex=682,.texIndex=36,.glowIndex=MAX_TXRS,.specIndex=1242,.normIndex=MAX_TXRS,.mass=0.28f,.dynFriction=0.1f,.statFriction=0.2f,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*310 item_beverage*/[310]={.modelIndex=18,.colMeshIndex=683,.texIndex=37,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.12f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*311 item_skull*/[311]={.modelIndex=593,.colMeshIndex=70,.texIndex=816,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.451f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*312 item_arm*/[312]={.modelIndex=7,.colMeshIndex=678,.texIndex=28,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=1.0f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*313 item_audiolog*/[313]={.modelIndex=11,.colMeshIndex=679,.texIndex=52,.glowIndex=80,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.2f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*314 weapon_grenadefrag*/[314]={.modelIndex=182,.colMeshIndex=73,.texIndex=348,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=1.0f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*315 weapon_grenadeconc*/[315]={.modelIndex=165,.colMeshIndex=84,.texIndex=334,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=1.3f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*316 weapon_grenadeemp*/[316]={.modelIndex=168,.colMeshIndex=85,.texIndex=338,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.8f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*317 weapon_grenadeearth*/[317]={.modelIndex=181,.colMeshIndex=86,.texIndex=346,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=1.5f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*318 weapon_grenademine*/[318]={.modelIndex=184,.colMeshIndex=87,.texIndex=353,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=1.2f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*319 weapon_grenadenitro*/[319]={.modelIndex=300,.colMeshIndex=301,.texIndex=356,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=1.2f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*320 weapon_grenadegas*/[320]={.modelIndex=183,.colMeshIndex=89,.texIndex=349,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.9f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*321 item_patch_berserk*/[321]={.modelIndex=488,.colMeshIndex=491,.texIndex=590,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.14f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*322 item_patch_detox*/[322]={.modelIndex=488,.colMeshIndex=491,.texIndex=591,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.14f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*323 item_patch_genius*/[323]={.modelIndex=488,.colMeshIndex=491,.texIndex=592,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.14f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*324 item_patch_medi*/[324]={.modelIndex=488,.colMeshIndex=491,.texIndex=600,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.14f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*325 item_patch_reflex*/[325]={.modelIndex=488,.colMeshIndex=491,.texIndex=641,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.14f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*326 item_patch_sight*/[326]={.modelIndex=488,.colMeshIndex=491,.texIndex=646,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.14f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*327 item_patch_staminup*/[327]={.modelIndex=488,.colMeshIndex=491,.texIndex=647,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.14f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*328 item_hw_system*/[328]={.modelIndex=207,.colMeshIndex=68,.texIndex=405,.glowIndex=404,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.17f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*329 item_hw_navunit*/[329]={.modelIndex=204,.colMeshIndex=696,.texIndex=907,.glowIndex=1259,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.1f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*330 item_hw_ereader*/[330]={.modelIndex=200,.colMeshIndex=692,.texIndex=397,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.12f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*331 item_hw_sensaround*/[331]={.modelIndex=205,.colMeshIndex=697,.texIndex=402,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.12f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*332 item_hw_targetid*/[332]={.modelIndex=208,.colMeshIndex=90,.texIndex=408,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.08f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*333 item_hw_shield*/[333]={.modelIndex=206,.colMeshIndex=91,.texIndex=403,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.14f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*334 item_hw_bio*/[334]={.modelIndex=197,.colMeshIndex=689,.texIndex=393,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.1f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*335 item_hw_lantern*/[335]={.modelIndex=203,.colMeshIndex=695,.texIndex=401,.glowIndex=400,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.11f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*336 item_hw_envirosuit*/[336]={.modelIndex=199,.colMeshIndex=691,.texIndex=396,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.451f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*337 item_hw_booster*/[337]={.modelIndex=198,.colMeshIndex=690,.texIndex=395,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.16f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*338 item_hw_jumpjets*/[338]={.modelIndex=202,.colMeshIndex=694,.texIndex=399,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.32f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*339 item_hw_infrared*/[339]={.modelIndex=201,.colMeshIndex=693,.texIndex=398,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.1f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*340 item_fireextinguisher*/[340]={.modelIndex=144,.colMeshIndex=684,.texIndex=317,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=1.3f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*341 item_access_card_admin*/[341]={.modelIndex=0,.colMeshIndex=672,.texIndex=9,.glowIndex=82,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.2f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*342 item_workerhelmet*/[342]={.modelIndex=648,.colMeshIndex=94,.texIndex=886,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.8f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*343 weapon_mk3*/[343]={.modelIndex=646,.colMeshIndex=309,.texIndex=885,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.75f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*344 weapon_blaster*/[344]={.modelIndex=638,.colMeshIndex=310,.texIndex=875,.glowIndex=874,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.5f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*345 weapon_dartgun*/[345]={.modelIndex=640,.colMeshIndex=311,.texIndex=876,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.3f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*346 weapon_flechette*/[346]={.modelIndex=642,.colMeshIndex=312,.texIndex=880,.glowIndex=879,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.4f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*347 weapon_ionrifle*/[347]={.modelIndex=643,.colMeshIndex=313,.texIndex=883,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.8f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*348 weapon_rapier*/[348]={.modelIndex=653,.colMeshIndex=314,.texIndex=891,.glowIndex=890,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.3f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*349 weapon_pipe*/[349]={.modelIndex=649,.colMeshIndex=647,.texIndex=887,.glowIndex=MAX_TXRS,.specIndex=1241,.normIndex=MAX_TXRS,.mass=0.85f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*350 weapon_magnum*/[350]={.modelIndex=644,.colMeshIndex=315,.texIndex=877,.glowIndex=MAX_TXRS,.specIndex=1231,.normIndex=MAX_TXRS,.mass=0.6f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*351 weapon_magpulse*/[351]={.modelIndex=645,.colMeshIndex=316,.texIndex=884,.glowIndex=MAX_TXRS,.specIndex=1231,.normIndex=MAX_TXRS,.mass=0.65f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*352 weapon_pistol*/[352]={.modelIndex=650,.colMeshIndex=317,.texIndex=878,.glowIndex=MAX_TXRS,.specIndex=1231,.normIndex=MAX_TXRS,.mass=0.3f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*353 weapon_plasma*/[353]={.modelIndex=651,.colMeshIndex=318,.texIndex=888,.glowIndex=MAX_TXRS,.specIndex=1240,.normIndex=MAX_TXRS,.mass=1.2f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*354 weapon_railgun*/[354]={.modelIndex=652,.colMeshIndex=319,.texIndex=889,.glowIndex=MAX_TXRS,.specIndex=1231,.normIndex=MAX_TXRS,.mass=1.0f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*355 weapon_riotgun*/[355]={.modelIndex=654,.colMeshIndex=320,.texIndex=892,.glowIndex=MAX_TXRS,.specIndex=1231,.normIndex=MAX_TXRS,.mass=0.55f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*356 weapon_skorpion*/[356]={.modelIndex=655,.colMeshIndex=321,.texIndex=893,.glowIndex=MAX_TXRS,.specIndex=1231,.normIndex=MAX_TXRS,.mass=1.3f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*357 weapon_sparqbeam*/[357]={.modelIndex=656,.colMeshIndex=322,.texIndex=895,.glowIndex=894,.specIndex=1231,.normIndex=MAX_TXRS,.mass=0.3f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*358 weapon_stungun*/[358]={.modelIndex=657,.colMeshIndex=323,.texIndex=896,.glowIndex=MAX_TXRS,.specIndex=1231,.normIndex=MAX_TXRS,.mass=0.3f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*359 item_battery*/[359]={.modelIndex=13,.colMeshIndex=680,.texIndex=35,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.3f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*360 item_battery_icad*/[360]={.modelIndex=13,.colMeshIndex=680,.texIndex=34,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.35f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*361 item_logic_probe*/[361]={.modelIndex=217,.colMeshIndex=306,.texIndex=427,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.15f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*362 item_healthkit*/[362]={.modelIndex=196,.colMeshIndex=688,.texIndex=391,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.25f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*363 item_plastique*/[363]={.modelIndex=492,.colMeshIndex=308,.texIndex=599,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=1.4f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*364 item_chipset_interfacedemod*/[364]={.modelIndex=45,.colMeshIndex=325,.texIndex=78,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.3f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*365 item_flask*/[365]={.modelIndex=145,.colMeshIndex=685,.texIndex=36,.glowIndex=MAX_TXRS,.specIndex=1242,.normIndex=MAX_TXRS,.mass=0.22f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*366 item_chipset_bitflag*/[366]={.modelIndex=45,.colMeshIndex=325,.texIndex=633,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.3f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*367 item_ammo_rubber*/[367]={.modelIndex=8,.colMeshIndex=676,.texIndex=19,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.25f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*368 item_isotopex22*/[368]={.modelIndex=209,.colMeshIndex=326,.texIndex=413,.glowIndex=412,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=1.2f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*369 item_testtube*/[369]={.modelIndex=622,.colMeshIndex=612,.texIndex=36,.glowIndex=MAX_TXRS,.specIndex=1242,.normIndex=MAX_TXRS,.mass=0.21f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*370 weapon_grenadefrag_live*/[370]={.modelIndex=182,.colMeshIndex=73,.texIndex=347,.glowIndex=630,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=1.0f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*371 item_chipset_isolinear*/[371]={.modelIndex=46,.colMeshIndex=308,.texIndex=409,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.26f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*372 weapon_grenadeconc_live*/[372]={.modelIndex=165,.colMeshIndex=84,.texIndex=334,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=1.3f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*373 item_ammo_needle*/[373]={.modelIndex=4,.colMeshIndex=U16_MAX,.texIndex=15,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.15f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_BOX,.colCtr=(V3){-0.0004654949f,0.0004549972f,0.0244365f},.colSz=(V3){0.131339f,0.1442801f,0.04838703f}},
/*374 item_ammo_tranq*/[374]={.modelIndex=4,.colMeshIndex=U16_MAX,.texIndex=27,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.15f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_BOX,.colCtr=(V3){-0.0004654949f,0.0004549972f,0.0244365f},.colSz=(V3){0.131339f,0.1442801f,0.04838703f}},
/*375 item_ammo_standard*/[375]={.modelIndex=5,.colMeshIndex=U16_MAX,.texIndex=25,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.2f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_BOX,.colCtr=(V3){0.0001984993f,0.0f,0.02172501f},.colSz=(V3){0.1209471f,0.2176701f,0.04345007f}},
/*376 item_ammo_teflon*/[376]={.modelIndex=5,.colMeshIndex=U16_MAX,.texIndex=26,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.2f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_BOX,.colCtr=(V3){0.0001984993f,0.0f,0.02172501f},.colSz=(V3){0.1209471f,0.2176701f,0.04345007f}},
/*377 item_ammo_hollow*/[377]={.modelIndex=5,.colMeshIndex=U16_MAX,.texIndex=11,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.2f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_BOX,.colCtr=(V3){0.0002185023f,0.0f,0.02122951f},.colSz=(V3){0.1423431f,0.2127061f,0.04245907f}},
/*378 item_ammo_slug*/[378]={.modelIndex=3,.colMeshIndex=U16_MAX,.texIndex=23,.glowIndex=22,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.2f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_BOX,.colCtr=(V3){0.0002185023f,0.0f,0.02122951f},.colSz=(V3){0.1423431f,0.2127061f,0.04245907f}},
/*379 item_ammo_magnesium*/[379]={.modelIndex=1,.colMeshIndex=673,.texIndex=14,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.35f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*380 item_ammo_penetrator*/[380]={.modelIndex=1,.colMeshIndex=673,.texIndex=16,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.35f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*381 item_ammo_hornet*/[381]={.modelIndex=1,.colMeshIndex=673,.texIndex=12,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.35f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*382 item_ammo_splinter*/[382]={.modelIndex=1,.colMeshIndex=673,.texIndex=24,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.35f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*383 item_ammo_rail*/[383]={.modelIndex=6,.colMeshIndex=675,.texIndex=17,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.40f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*384 item_ammo_slag*/[384]={.modelIndex=1,.colMeshIndex=673,.texIndex=21,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.35f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*385 item_ammo_slaglarge*/[385]={.modelIndex=10,.colMeshIndex=677,.texIndex=20,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.40f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*386 item_ammo_magcart*/[386]={.modelIndex=2,.colMeshIndex=674,.texIndex=13,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.35f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*387 weapon_grenadeemp_live*/[387]={.modelIndex=168,.colMeshIndex=85,.texIndex=337,.glowIndex=627,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.8f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*388 item_access_card_std*/[388]={.modelIndex=0,.colMeshIndex=672,.texIndex=79,.glowIndex=867,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.2f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*389 weapon_grenadeearth_live*/[389]={.modelIndex=181,.colMeshIndex=86,.texIndex=345,.glowIndex=628,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=1.5f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*390 item_access_card_group1*/[390]={.modelIndex=0,.colMeshIndex=672,.texIndex=7,.glowIndex=159,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.2f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*391 item_access_card_science*/[391]={.modelIndex=0,.colMeshIndex=672,.texIndex=2,.glowIndex=343,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.2f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*392 item_access_card_eng*/[392]={.modelIndex=0,.colMeshIndex=672,.texIndex=3,.glowIndex=81,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.2f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*393 item_access_card_groupB*/[393]={.modelIndex=0,.colMeshIndex=672,.texIndex=7,.glowIndex=159,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.2f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*394 item_access_card_security*/[394]={.modelIndex=0,.colMeshIndex=672,.texIndex=10,.glowIndex=344,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.2f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*395 item_access_card_per5diego*/[395]={.modelIndex=0,.colMeshIndex=672,.texIndex=8,.glowIndex=341,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.2f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*396 item_access_card_medi*/[396]={.modelIndex=0,.colMeshIndex=672,.texIndex=1,.glowIndex=161,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.2f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*397 item_access_card_group3*/[397]={.modelIndex=0,.colMeshIndex=672,.texIndex=7,.glowIndex=159,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.2f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*398 item_access_card_purple*/[398]={.modelIndex=0,.colMeshIndex=672,.texIndex=5,.glowIndex=342,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.2f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*399 item_head_male*/[399]={.modelIndex=194,.colMeshIndex=194,.texIndex=389,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=1.29f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*400 item_head_female*/[400]={.modelIndex=193,.colMeshIndex=686,.texIndex=388,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=1.30f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*401 item_severedhead*/[401]={.modelIndex=590,.colMeshIndex=327,.texIndex=801,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=1.28f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*402 weapon_grenademine_live*/[402]={.modelIndex=184,.colMeshIndex=87,.texIndex=351,.glowIndex=352,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=1.2f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*403 weapon_grenadenitro_live*/[403]={.modelIndex=185,.colMeshIndex=88,.texIndex=354,.glowIndex=355,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=1.2f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*404 weapon_grenadegas_live*/[404]={.modelIndex=183,.colMeshIndex=89,.texIndex=349,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.9f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*405 to 416 unused*/[405]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*406*/[406]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*407*/[407]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*408*/[408]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*409*/[409]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*410*/[410]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*411*/[411]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*412*/[412]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*413*/[413]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*414*/[414]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*415*/[415]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*416*/[416]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*417 item_access_card_perdarcy*/[417]={.modelIndex=0,.colMeshIndex=672,.texIndex=8,.glowIndex=341,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.2f,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*418 unused*/[418]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*419 npc_autobomb*/[419]={.modelIndex=299,.colMeshIndex=328,.texIndex=542,.glowIndex=541,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=1.0f,.dynFriction=0.15f,.statFriction=1.0f,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*420 npc_cyborg_assassin*/[420]={.modelIndex=306,.colMeshIndex=0,.texIndex=545,.glowIndex=544,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=1.5f,.dynFriction=0.15f,.statFriction=1.0f,.animationNum=24,.col=COLTYPE_CAP,.colCtr=(V3){0,0.96f,0},.colSz=(V3){0.48f,2.0f,0}},
/*421 npc_avian_mutant*/[421]={.modelIndex=328,.colMeshIndex=0,.texIndex=568,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=2.0f,.dynFriction=0.15f,.statFriction=1.0f,.animationNum=35,.col=COLTYPE_CAP,.colCtr=(V3){0,0.64f,0},.colSz=(V3){0.64f,1.60f,0}},
/*422 npc_exec_bot*/[422]={.modelIndex=316,.colMeshIndex=0,.texIndex=555,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=2.2f,.dynFriction=0.15f,.statFriction=1.0f,.animationNum=29,.col=COLTYPE_CAP,.colCtr=(V3){0,0.96f,0},.colSz=(V3){0.48f,2.025f,0}},
/*423 npc_cyborg_drone*/[423]={.modelIndex=312,.colMeshIndex=0,.texIndex=547,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=1.5f,.dynFriction=0.15f,.statFriction=1.0f,.animationNum=3,.col=COLTYPE_CAP,.colCtr=(V3){0,0,0},.colSz=(V3){0.36f,2.00f,0}},
/*424 npc_cortex_reaver*/[424]={.modelIndex=300,.colMeshIndex=0,.texIndex=543,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=5.0f,.dynFriction=0.15f,.statFriction=1.0f,.animationNum=23,.col=COLTYPE_CAP,.colCtr=(V3){0,1.28f,0},.colSz=(V3){1.28f,2.5f,0}},
/*425 npc_cyborg_warrior*/[425]={.modelIndex=315,.colMeshIndex=0,.texIndex=554,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=1.5f,.dynFriction=0.15f,.statFriction=1.0f,.animationNum=28,.col=COLTYPE_CAP,.colCtr=(V3){0,0,0},.colSz=(V3){0.48f,2.00f,0}},
/*426 npc_cyborg_enforcer*/[426]={.modelIndex=314,.colMeshIndex=0,.texIndex=550,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=1.5f,.dynFriction=0.15f,.statFriction=1.0f,.animationNum=27,.col=COLTYPE_CAP,.colCtr=(V3){0,1.03f,0},.colSz=(V3){0.40f,2.08f,0}},
/*427 npc_cyborg_elite*/[427]={.modelIndex=313,.colMeshIndex=0,.texIndex=548,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=3.5f,.dynFriction=0.15f,.statFriction=1.0f,.animationNum=26,.col=COLTYPE_CAP,.colCtr=(V3){0,1.09f,0},.colSz=(V3){0.44f,2.20f,0}},
/*428 npc_cyborg_diego*/[428]={.modelIndex=309,.colMeshIndex=0,.texIndex=546,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=2.0f,.dynFriction=0.15f,.statFriction=1.0f,.animationNum=25,.col=COLTYPE_CAP,.colCtr=(V3){0,1.04f,0},.colSz=(V3){0.48f,2.12f,0}},
/*429 npc_sec1_bot*/[429]={.modelIndex=333,.colMeshIndex=0,.texIndex=573,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=1.5f,.dynFriction=0.15f,.statFriction=1.0f,.animationNum=38,.col=COLTYPE_CAP,.colCtr=(V3){0,0.76f,0},.colSz=(V3){0.76f,1.8f,0}},
/*430 npc_sec2_bot*/[430]={.modelIndex=335,.colMeshIndex=0,.texIndex=574,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=4.51f,.dynFriction=0.15f,.statFriction=1.0f,.animationNum=39,.col=COLTYPE_CAP,.colCtr=(V3){0,1.08f,0},.colSz=(V3){1.12f,2.40f,0}},
/*431 npc_maint_bot*/[431]={.modelIndex=325,.colMeshIndex=0,.texIndex=567,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=1.5f,.dynFriction=0.15f,.statFriction=1.0f,.animationNum=34,.col=COLTYPE_SPH,.colCtr=(V3){0,0.78f,0},.colSz=(V3){2.00f,0,0}},
/*432 npc_mutant_cyborg*/[432]={.modelIndex=329,.colMeshIndex=0,.texIndex=569,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=3.0f,.dynFriction=0.15f,.statFriction=1.0f,.animationNum=51,.col=COLTYPE_CAP,.colCtr=(V3){0,0.12f,0},.colSz=(V3){0.75f,2.30f,0}},
/*433 npc_hopper*/[433]={.modelIndex=322,.colMeshIndex=0,.texIndex=562,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=1.0f,.dynFriction=0.15f,.statFriction=1.0f,.animationNum=32,.col=COLTYPE_CAP,.colCtr=(V3){0,1.04f,-0.16f},.colSz=(V3){0.96f,2.38f,0}},
/*434 npc_humanoid_mutant*/[434]={.modelIndex=323,.colMeshIndex=0,.texIndex=563,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=1.4f,.dynFriction=0.15f,.statFriction=1.0f,.animationNum=2,.col=COLTYPE_CAP,.colCtr=(V3){0,0,0},.colSz=(V3){0.38f,2.00f,0}},
/*435 npc_invisomut*/[435]={.modelIndex=324,.colMeshIndex=329,.texIndex=565,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=1.3f,.dynFriction=0.15f,.statFriction=1.0f,.animationNum=33,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*436 npc_virus_mutant*/[436]={.modelIndex=330,.colMeshIndex=0,.texIndex=576,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=1.4f,.dynFriction=0.15f,.statFriction=1.0f,.animationNum=41,.col=COLTYPE_CAP,.colCtr=(V3){0,0.95f,0.16f},.colSz=(V3){0.72f,1.90f,0}},
/*437 npc_servbot*/[437]={.modelIndex=5153,.colMeshIndex=54,.texIndex=575,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=2.50f,.dynFriction=0.15f,.statFriction=1.0f,.animationNum=40,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*438 npc_flier_bot*/[438]={.modelIndex=318,.colMeshIndex=0,.texIndex=558,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=1.75f,.dynFriction=0.15f,.statFriction=1.0f,.animationNum=30,.col=COLTYPE_SPH,.colCtr=(V3){0,0.16f,0},.colSz=(V3){0.8f,0,0}},
/*439 npc_zerog_mutant*/[439]={.modelIndex=395,.colMeshIndex=0,.texIndex=1170,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=1.30f,.dynFriction=0.15f,.statFriction=1.0f,.animationNum=42,.col=COLTYPE_SPH,.colCtr=(V3){0,0,0},.colSz=(V3){1.6f,0,0}},
/*440 npc_gorilla_tiger_mutant*/[440]={.modelIndex=320,.colMeshIndex=330,.texIndex=560,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=2.00f,.dynFriction=0.15f,.statFriction=1.0f,.animationNum=31,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*441 npc_repairbot*/[441]={.modelIndex=331,.colMeshIndex=331,.texIndex=572,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=1.50f,.dynFriction=0.15f,.statFriction=1.0f,.animationNum=37,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*442 npc_plant_mutant*/[442]={.modelIndex=330,.colMeshIndex=0,.texIndex=570,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.80f,.dynFriction=0.15f,.statFriction=1.0f,.animationNum=36,.col=COLTYPE_CAP,.colCtr=(V3){0,0.72f,0},.colSz=(V3){0.6f,1.44f,0}},
/*443 npc_cyberdog*/[443]={.modelIndex=302,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=1.50f,.dynFriction=0.15f,.statFriction=1.0f,.animationNum=MAX_ANIMS,.col=COLTYPE_SPH,.colCtr=(V3){0,0,0},.colSz=(V3){0.72f,0,0}},
/*444 npc_cyberguard*/[444]={.modelIndex=303,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=2.00f,.dynFriction=0.15f,.statFriction=1.0f,.animationNum=MAX_ANIMS,.col=COLTYPE_SPH,.colCtr=(V3){0,0,0},.colSz=(V3){1.0f,0,0}},
/*445 npc_cyberram*/[445]={.modelIndex=304,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=2.00f,.dynFriction=0.15f,.statFriction=1.0f,.animationNum=MAX_ANIMS,.col=COLTYPE_SPH,.colCtr=(V3){0,0,0},.colSz=(V3){1.44f,0,0}},
/*446 npc_cyber_reaver*/[446]={.modelIndex=305,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=2.20f,.dynFriction=0.15f,.statFriction=1.0f,.animationNum=MAX_ANIMS,.col=COLTYPE_SPH,.colCtr=(V3){0,0,0},.colSz=(V3){0.72f,0,0}},
/*447 npc_cybershodan*/[447]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=4.51f,.dynFriction=0.15f,.statFriction=1.0f,.animationNum=MAX_ANIMS,.col=COLTYPE_CAP,.colCtr=(V3){0,0,0},.colSz=(V3){0.28f,2.0f,0}},
/*448 item_cyber_data*/[448]={.modelIndex=65,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_SPH,.colCtr=(V3){0,0,0},.colSz=(V3){1.5f,0,0}},
/*449 item_cyber_decoy*/[449]={.modelIndex=72,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_SPH,.colCtr=(V3){0,0,0},.colSz=(V3){1.5f,0,0}},
/*450 item_cyber_drill*/[450]={.modelIndex=68,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_SPH,.colCtr=(V3){0,0,0},.colSz=(V3){1.5f,0,0}},
/*451 item_cyber_game*/[451]={.modelIndex=65,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_SPH,.colCtr=(V3){0,0,0},.colSz=(V3){1.5f,0,0}},
/*452 item_cyber_integrity*/[452]={.modelIndex=69,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_SPH,.colCtr=(V3){0,0,0},.colSz=(V3){1.5f,0,0}},
/*453 item_cyber_keycard*/[453]={.modelIndex=70,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_SPH,.colCtr=(V3){0,0,0},.colSz=(V3){1.5f,0,0}},
/*454 item_cyber_pulser*/[454]={.modelIndex=65,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_SPH,.colCtr=(V3){0,0,0},.colSz=(V3){1.5f,0,0}},
/*455 item_cyber_recall*/[455]={.modelIndex=65,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_SPH,.colCtr=(V3){0,0,0},.colSz=(V3){1.5f,0,0}},
/*456 item_cyber_shield*/[456]={.modelIndex=65,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_SPH,.colCtr=(V3){0,0,0},.colSz=(V3){1.5f,0,0}},
/*457 item_cyber_turbo*/[457]={.modelIndex=65,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_SPH,.colCtr=(V3){0,0,0},.colSz=(V3){1.5f,0,0}},
/*458 prop_phys_barrel_chemical*/[458]={.modelIndex=12,.colMeshIndex=332,.texIndex=30,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=1.5f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*459 prop_phys_barrel_radiation*/[459]={.modelIndex=12,.colMeshIndex=332,.texIndex=31,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=1.5f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*460 prop_phys_barrel_toxic*/[460]={.modelIndex=12,.colMeshIndex=332,.texIndex=33,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=1.5f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*461 prop_phys_cart*/[461]={.modelIndex=40,.colMeshIndex=333,.texIndex=416,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=2.5f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*462 prop_phys_pot*/[462]={.modelIndex=494,.colMeshIndex=334,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.3f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*463 prop_phys_toolcart*/[463]={.modelIndex=624,.colMeshIndex=335,.texIndex=865,.glowIndex=MAX_TXRS,.specIndex=866,.normIndex=864,.mass=20.0f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_CVX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*464 se_briefcase*/[464]={.modelIndex=34,.colMeshIndex=0,.texIndex=66,.glowIndex=65,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*465 se_corpse_blueshirt*/[465]={.modelIndex=51,.colMeshIndex=0,.texIndex=126,.glowIndex=MAX_TXRS,.specIndex=127,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*466 se_corpse_brownshirt*/[466]={.modelIndex=52,.colMeshIndex=0,.texIndex=128,.glowIndex=MAX_TXRS,.specIndex=129,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*467 se_corpse_eaten*/[467]={.modelIndex=MAX_ANIMS,.colMeshIndex=0,.texIndex=130,.glowIndex=MAX_TXRS,.specIndex=131,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*468 se_corpse_labcoat*/[468]={.modelIndex=55,.colMeshIndex=0,.texIndex=132,.glowIndex=MAX_TXRS,.specIndex=133,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*469 se_corpse_security*/[469]={.modelIndex=56,.colMeshIndex=0,.texIndex=136,.glowIndex=MAX_TXRS,.specIndex=137,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*470 se_corpse_tan*/[470]={.modelIndex=57,.colMeshIndex=0,.texIndex=138,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*471 se_corpse_torso*/[471]={.modelIndex=58,.colMeshIndex=0,.texIndex=126,.glowIndex=MAX_TXRS,.specIndex=127,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*472 se_crate1*/[472]={.modelIndex=60,.colMeshIndex=0,.texIndex=145,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.75f,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_BOX,.colCtr=(V3){0,0,0},.colSz=(V3){0.684186f,0.6841861f,0.6841861f}},
/*473 se_crate2*/[473]={.modelIndex=60,.colMeshIndex=0,.texIndex=143,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.75f,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_BOX,.colCtr=(V3){0,0,0},.colSz=(V3){0.684186f,0.6841861f,0.6841861f}},
/*474 se_crate3*/[474]={.modelIndex=60,.colMeshIndex=0,.texIndex=144,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.75f,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_BOX,.colCtr=(V3){0,0,0},.colSz=(V3){0.684186f,0.6841861f,0.6841861f}},
/*475 se_crate4*/[475]={.modelIndex=60,.colMeshIndex=0,.texIndex=146,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=2.25f,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_BOX,.colCtr=(V3){0,0,0},.colSz=(V3){0.684186f,0.6841861f,0.6841861f}},
/*476 se_crate5*/[476]={.modelIndex=60,.colMeshIndex=0,.texIndex=145,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=2.25f,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_BOX,.colCtr=(V3){0,0,0},.colSz=(V3){0.684186f,0.6841861f,0.6841861f}},
/*477 sec_camera*/[477]={.modelIndex=589,.colMeshIndex=0,.texIndex=73,.glowIndex=72,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*478 sec_cpunode*/[478]={.modelIndex=587,.colMeshIndex=0,.texIndex=242,.glowIndex=248,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*479 sec_cpunode_small*/[479]={.modelIndex=588,.colMeshIndex=0,.texIndex=107,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*480 weapon_cyber_mine*/[480]={.modelIndex=71,.colMeshIndex=0,.texIndex=1224,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*481 proj_enemshot2*/[481]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.3f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*482 proj_magpulse_shot*/[482]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=807,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.3f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*483 proj_stungun_shot*/[483]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=835,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.3f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*484 proj_rail_shot*/[484]={.modelIndex=652,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.3f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*485 proj_plasmarifle_shot*/[485]={.modelIndex=651,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.3f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*486 proj_enemshot6*/[486]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.3f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*487 proj_enemshot5*/[487]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.2f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*488 proj_enemshot4*/[488]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.3f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*489 proj_throwingstar*/[489]={.modelIndex=307,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.3f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*490 proj_magpulsenpc_shot*/[490]={.modelIndex=645,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.3f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*491 proj_railnpc_shot*/[491]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.3f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*492 proj_cyberplayer_shot*/[492]={.modelIndex=72,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.3f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*493 proj_cyberdog_shot*/[493]={.modelIndex=63,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.3f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*494 proj_cyberreaver_shot*/[494]={.modelIndex=64,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.3f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*495 proj_cyberice_shot*/[495]={.modelIndex=68,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0.3f,.dynFriction=0.5f,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*496 doorA*/[496]={.modelIndex=719,.colMeshIndex=0,.texIndex=185,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=1,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*497 doorB*/[497]={.modelIndex=0,.colMeshIndex=0,.texIndex=189,.glowIndex=188,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=0,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*498 doorC*/[498]={.modelIndex=0,.colMeshIndex=0,.texIndex=184,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=5,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*499 doorD*/[499]={.modelIndex=0,.colMeshIndex=0,.texIndex=196,.glowIndex=197,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=4,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*500 doorE*/[500]={.modelIndex=0,.colMeshIndex=0,.texIndex=208,.glowIndex=207,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=9,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*501 doorF*/[501]={.modelIndex=0,.colMeshIndex=0,.texIndex=187,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=10,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*502 doorG*/[502]={.modelIndex=0,.colMeshIndex=0,.texIndex=193,.glowIndex=194,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=11,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*503 doorH*/[503]={.modelIndex=0,.colMeshIndex=0,.texIndex=190,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=12,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*504 doorI*/[504]={.modelIndex=0,.colMeshIndex=0,.texIndex=200,.glowIndex=199,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=13,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*505 doorJ*/[505]={.modelIndex=0,.colMeshIndex=0,.texIndex=215,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=6,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*506 doorK*/[506]={.modelIndex=0,.colMeshIndex=0,.texIndex=214,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=7,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*507 doorL*/[507]={.modelIndex=0,.colMeshIndex=0,.texIndex=191,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=8,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*508 door_elevator1*/[508]={.modelIndex=0,.colMeshIndex=0,.texIndex=202,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=14,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*509 door_elevator2*/[509]={.modelIndex=0,.colMeshIndex=0,.texIndex=203,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=15,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*510 door_elevator3*/[510]={.modelIndex=0,.colMeshIndex=0,.texIndex=206,.glowIndex=205,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=16,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*511 door_elevator4*/[511]={.modelIndex=0,.colMeshIndex=0,.texIndex=203,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=17,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*512 door_secret1*/[512]={.modelIndex=0,.colMeshIndex=0,.texIndex=210,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=19,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*513 door_secret2*/[513]={.modelIndex=0,.colMeshIndex=0,.texIndex=209,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=18,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*514 door_secret3*/[514]={.modelIndex=94,.colMeshIndex=0,.texIndex=211,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=20,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*515 func_forcebridge*/[515]={.modelIndex=78,.colMeshIndex=0,.texIndex=38,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_BOX,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*516 prop_lift2*/[516]={.modelIndex=215,.colMeshIndex=U16_MAX,.texIndex=155,.glowIndex=154,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_BOX,.colCtr=(V3){0.0f,0.0f,0.0f},.colSz=(V3){1.0f,1.0f,1.0f}},
/*517 func_wall*/[517]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=10.0f,.dynFriction=0.6f,.statFriction=0.6f,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*518 BulletHoleLarge*/[518]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*519 BulletHoleScorchLarge*/[519]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*520 BulletHoleScorchSmall*/[520]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*521 BulletHoleSmall*/[521]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*522 BulletHoleTiny*/[522]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*523 BulletHoleTinySpread*/[523]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*524 func_door_cyber*/[524]={.modelIndex=178,.colMeshIndex=U16_MAX,.texIndex=1224,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_BOX,.colCtr=(V3){0.0f,1.31f,0.0f},.colSz=(V3){2.56f,0.06f,2.56f}},
/*525 prop_console01*/[525]={.modelIndex=49,.colMeshIndex=0,.texIndex=100,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*526 prop_console02*/[526]={.modelIndex=50,.colMeshIndex=0,.texIndex=100,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*527 prop_grate1_1*/[527]={.modelIndex=186,.colMeshIndex=0,.texIndex=359,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*528 prop_grate1_2*/[528]={.modelIndex=187,.colMeshIndex=0,.texIndex=360,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*529 prop_grate1_3*/[529]={.modelIndex=188,.colMeshIndex=0,.texIndex=361,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*530 se_cabinet*/[530]={.modelIndex=39,.colMeshIndex=0,.texIndex=70,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*531 se_thermos*/[531]={.modelIndex=623,.colMeshIndex=0,.texIndex=863,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*532 prop_beaker_holder*/[532]={.modelIndex=15,.colMeshIndex=0,.texIndex=36,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*533 prop_bed*/[533]={.modelIndex=16,.colMeshIndex=0,.texIndex=246,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*534 prop_bed_hospital*/[534]={.modelIndex=608,.colMeshIndex=0,.texIndex=759,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*535 prop_bed_neurosurgery*/[535]={.modelIndex=17,.colMeshIndex=0,.texIndex=18,.glowIndex=MAX_TXRS,.specIndex=1238,.normIndex=29,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*536 prop_bonepile1*/[536]={.modelIndex=19,.colMeshIndex=0,.texIndex=815,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*537 prop_bridgewall1*/[537]={.modelIndex=33,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*538 prop_broken_clock*/[538]={.modelIndex=38,.colMeshIndex=0,.texIndex=1117,.glowIndex=1115,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*539 prop_brokengun*/[539]={.modelIndex=639,.colMeshIndex=0,.texIndex=878,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*540 prop_chair01*/[540]={.modelIndex=41,.colMeshIndex=0,.texIndex=195,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*541 prop_chair02*/[541]={.modelIndex=42,.colMeshIndex=0,.texIndex=195,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*542 prop_chair03*/[542]={.modelIndex=43,.colMeshIndex=0,.texIndex=195,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*543 prop_chair04*/[543]={.modelIndex=41,.colMeshIndex=0,.texIndex=195,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*544 prop_chair05*/[544]={.modelIndex=42,.colMeshIndex=0,.texIndex=195,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*545 prop_chandelier*/[545]={.modelIndex=496,.colMeshIndex=0,.texIndex=644,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*546 prop_charge_station*/[546]={.modelIndex=44,.colMeshIndex=0,.texIndex=77,.glowIndex=76,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*547 prop_clothes*/[547]={.modelIndex=47,.colMeshIndex=0,.texIndex=97,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*548 prop_computer*/[548]={.modelIndex=48,.colMeshIndex=0,.texIndex=195,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*549 prop_couch*/[549]={.modelIndex=59,.colMeshIndex=0,.texIndex=195,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*550 prop_couch2*/[550]={.modelIndex=59,.colMeshIndex=0,.texIndex=195,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*551 prop_cpuscreen*/[551]={.modelIndex=178,.colMeshIndex=0,.texIndex=768,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*552 prop_cyber_datafrag*/[552]={.modelIndex=78,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*553 prop_cyber_decoy*/[553]={.modelIndex=78,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*554 prop_cyber_exit*/[554]={.modelIndex=78,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*555 prop_cyber_switch*/[555]={.modelIndex=0,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*556 prop_cyberport*/[556]={.modelIndex=62,.colMeshIndex=0,.texIndex=117,.glowIndex=116,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*557 prop_desk01*/[557]={.modelIndex=74,.colMeshIndex=0,.texIndex=125,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*558 prop_desk02*/[558]={.modelIndex=75,.colMeshIndex=0,.texIndex=124,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*559 prop_dexmissile*/[559]={.modelIndex=76,.colMeshIndex=0,.texIndex=164,.glowIndex=162,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*560 prop_foliage_fernpoison*/[560]={.modelIndex=160,.colMeshIndex=0,.texIndex=331,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*561 prop_foliage_bush*/[561]={.modelIndex=495,.colMeshIndex=0,.texIndex=643,.glowIndex=642,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*562 prop_foliage_fern*/[562]={.modelIndex=160,.colMeshIndex=0,.texIndex=333,.glowIndex=330,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*563 prop_foliage_fernblueflower*/[563]={.modelIndex=159,.colMeshIndex=0,.texIndex=333,.glowIndex=330,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*564 prop_foliage_pinetreem*/[564]={.modelIndex=489,.colMeshIndex=0,.texIndex=594,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*565 prop_foliage_poisonbush1*/[565]={.modelIndex=493,.colMeshIndex=0,.texIndex=638,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*566 prop_gear_large*/[566]={.modelIndex=166,.colMeshIndex=0,.texIndex=335,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*567 prop_gear_small*/[567]={.modelIndex=167,.colMeshIndex=0,.texIndex=336,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*568 prop_grass1*/[568]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=329,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*569 prop_grass2*/[569]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=329,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*570 prop_grass3*/[570]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=329,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*571 prop_grass4*/[571]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=329,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*572 prop_grass5*/[572]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=329,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*573 prop_grate4*/[573]={.modelIndex=161,.colMeshIndex=0,.texIndex=329,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*574 prop_healingbed*/[574]={.modelIndex=195,.colMeshIndex=0,.texIndex=1139,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*575 prop_lamp*/[575]={.modelIndex=212,.colMeshIndex=0,.texIndex=423,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*576 prop_light_emergsignal*/[576]={.modelIndex=216,.colMeshIndex=0,.texIndex=426,.glowIndex=0,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*577 prop_microscope*/[577]={.modelIndex=298,.colMeshIndex=0,.texIndex=645,.glowIndex=MAX_TXRS,.specIndex=1241,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*578 prop_pipe*/[578]={.modelIndex=490,.colMeshIndex=0,.texIndex=595,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*579 prop_puddle*/[579]={.modelIndex=157,.colMeshIndex=0,.texIndex=648,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*580 prop_puddle_grease*/[580]={.modelIndex=157,.colMeshIndex=0,.texIndex=650,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*581 prop_puddle_oil*/[581]={.modelIndex=157,.colMeshIndex=0,.texIndex=652,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*582 prop_shelves*/[582]={.modelIndex=591,.colMeshIndex=0,.texIndex=94,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*583 prop_skeleton*/[583]={.modelIndex=592,.colMeshIndex=0,.texIndex=815,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*584 prop_sleeping_cables*/[584]={.modelIndex=595,.colMeshIndex=0,.texIndex=71,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*585 prop_sparkingwire*/[585]={.modelIndex=0,.colMeshIndex=0,.texIndex=71,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=46,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*586 prop_table*/[586]={.modelIndex=619,.colMeshIndex=0,.texIndex=92,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*587 prop_tv_on_a_post*/[587]={.modelIndex=625,.colMeshIndex=0,.texIndex=1228,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*588 prop_vendingmachines1*/[588]={.modelIndex=627,.colMeshIndex=0,.texIndex=870,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*589 prop_vendingmachines2*/[589]={.modelIndex=614,.colMeshIndex=0,.texIndex=871,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*590 prop_weapon_rack*/[590]={.modelIndex=641,.colMeshIndex=0,.texIndex=113,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*591 prop_xray*/[591]={.modelIndex=660,.colMeshIndex=0,.texIndex=153,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_MSH,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*592 text_decal*/[592]={.modelIndex=77,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*593 text_decalStopDSS1*/[593]={.modelIndex=77,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*594 trigger_counter*/[594]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*595 trigger_cyberpush*/[595]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*596 trigger_gravitylift*/[596]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*597 trigger_ladder*/[597]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*598 trigger_multiple*/[598]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*599 trigger_music*/[599]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*600 trigger_once*/[600]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*601 trigger_radiation*/[601]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*602 us_isotopepanel*/[602]={.modelIndex=0,.colMeshIndex=0,.texIndex=616,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=44,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*603 us_paperlog*/[603]={.modelIndex=486,.colMeshIndex=0,.texIndex=580,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*604 us_puz_elevatorkeypad*/[604]={.modelIndex=615,.colMeshIndex=0,.texIndex=247,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*605 us_puz_elevatorkeypad2*/[605]={.modelIndex=618,.colMeshIndex=0,.texIndex=250,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*606 us_puz_elevatorkeypad3*/[606]={.modelIndex=615,.colMeshIndex=0,.texIndex=247,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*607 us_puz_elevatorkeypad4*/[607]={.modelIndex=210,.colMeshIndex=0,.texIndex=249,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*608 us_puz_keypad*/[608]={.modelIndex=211,.colMeshIndex=0,.texIndex=414,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*609 us_puz_panel_blue_grid*/[609]={.modelIndex=0,.colMeshIndex=0,.texIndex=604,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=43,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*610 us_puz_panel_brown_grid*/[610]={.modelIndex=0,.colMeshIndex=0,.texIndex=604,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=43,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*611 us_puz_panel_gray_grid*/[611]={.modelIndex=0,.colMeshIndex=0,.texIndex=634,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=43,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*612 us_puz_panel_red_grid*/[612]={.modelIndex=0,.colMeshIndex=0,.texIndex=625,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=43,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*613 us_puz_panel_teal_grid*/[613]={.modelIndex=0,.colMeshIndex=0,.texIndex=601,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=43,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*614 us_relaypanel*/[614]={.modelIndex=0,.colMeshIndex=0,.texIndex=617,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=45,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*615 us_retinalscanner*/[615]={.modelIndex=79,.colMeshIndex=0,.texIndex=46,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*616 prop_vending1_1*/[616]={.modelIndex=627,.colMeshIndex=0,.texIndex=870,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*617 prop_vending1_2*/[617]={.modelIndex=628,.colMeshIndex=0,.texIndex=870,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*618 prop_vending1_3*/[618]={.modelIndex=629,.colMeshIndex=0,.texIndex=870,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*619 prop_vending2_1*/[619]={.modelIndex=614,.colMeshIndex=0,.texIndex=871,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*620 prop_vending2_2*/[620]={.modelIndex=621,.colMeshIndex=0,.texIndex=871,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*621 ambient_airhiss*/[621]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*622 ambient_clicker*/[622]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*623 ambient_compressor*/[623]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*624 ambient_dishwasher*/[624]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*625 ambient_drip_amb*/[625]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*626 ambient_fan*/[626]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*627 ambient_generator_gas*/[627]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*628 ambient_gurgle*/[628]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*629 ambient_icemaker*/[629]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*630 ambient_intake*/[630]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*631 ambient_lathe*/[631]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*632 ambient_lev3loop1*/[632]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*633 ambient_lev3loop2*/[633]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*634 ambient_lev3loop3*/[634]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*635 ambient_lev3loop4*/[635]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*636 ambient_liquid_bubble*/[636]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*637 ambient_liquid_lava2*/[637]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*638 ambient_looping*/[638]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*639 ambient_machgear_loop*/[639]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*640 ambient_machine_ambience*/[640]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*641 ambient_machine_go*/[641]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*642 ambient_machine_humamb7*/[642]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*643 ambient_machine_humlonoise*/[643]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*644 ambient_machine_loop1*/[644]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*645 ambient_machine_loop2*/[645]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*646 ambient_machinea1*/[646]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*647 ambient_machinevat_loop*/[647]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*648 ambient_mist*/[648]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*649 ambient_pipewater_loop*/[649]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*650 ambient_powerloom*/[650]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*651 ambient_pump*/[651]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*652 ambient_pump2*/[652]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*653 ambient_rain*/[653]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*654 ambient_steam_loop*/[654]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*655 ambient_washing_machine*/[655]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*656 decal_blood_die*/[656]={.modelIndex=77,.colMeshIndex=0,.texIndex=237,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*657 decal_blood_resist*/[657]={.modelIndex=77,.colMeshIndex=0,.texIndex=240,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*658 decal_blood_stayaway*/[658]={.modelIndex=77,.colMeshIndex=0,.texIndex=235,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*659 decal_blood_words2*/[659]={.modelIndex=77,.colMeshIndex=0,.texIndex=236,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*660 decal_bloodfonta*/[660]={.modelIndex=178,.colMeshIndex=0,.texIndex=118,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*661 decal_bloodfonte*/[661]={.modelIndex=178,.colMeshIndex=0,.texIndex=121,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*662 decal_bloodfontg*/[662]={.modelIndex=178,.colMeshIndex=0,.texIndex=122,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*663 decal_bloodfonth*/[663]={.modelIndex=178,.colMeshIndex=0,.texIndex=89,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*664 decal_bloodfontr*/[664]={.modelIndex=178,.colMeshIndex=0,.texIndex=139,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*665 decal_bloodfonty*/[665]={.modelIndex=178,.colMeshIndex=0,.texIndex=140,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*666 decal_bloodsplat2*/[666]={.modelIndex=157,.colMeshIndex=0,.texIndex=130,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*667 decal_logo_antenna*/[667]={.modelIndex=77,.colMeshIndex=0,.texIndex=182,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*668 decal_logo_armory*/[668]={.modelIndex=77,.colMeshIndex=0,.texIndex=178,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*669 decal_logo_biohazard*/[669]={.modelIndex=77,.colMeshIndex=0,.texIndex=180,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*670 decal_logo_bridge*/[670]={.modelIndex=77,.colMeshIndex=0,.texIndex=181,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*671 decal_logo_cyborg*/[671]={.modelIndex=77,.colMeshIndex=0,.texIndex=176,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*672 decal_logo_gears*/[672]={.modelIndex=77,.colMeshIndex=0,.texIndex=174,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*673 decal_logo_medical*/[673]={.modelIndex=77,.colMeshIndex=0,.texIndex=165,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*674 decal_logo_radhazard*/[674]={.modelIndex=77,.colMeshIndex=0,.texIndex=177,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*675 decal_logo_research*/[675]={.modelIndex=77,.colMeshIndex=0,.texIndex=175,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*676 decal_logo_security*/[676]={.modelIndex=77,.colMeshIndex=0,.texIndex=167,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*677 decal_painting1*/[677]={.modelIndex=77,.colMeshIndex=0,.texIndex=218,.glowIndex=216,.specIndex=MAX_TXRS,.normIndex=217,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*678 decal_painting2*/[678]={.modelIndex=77,.colMeshIndex=0,.texIndex=220,.glowIndex=219,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*679 decal_painting3*/[679]={.modelIndex=77,.colMeshIndex=0,.texIndex=222,.glowIndex=221,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*680 decal_posterbetterfuture*/[680]={.modelIndex=77,.colMeshIndex=0,.texIndex=226,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=225,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*681 decal_postergenetics*/[681]={.modelIndex=77,.colMeshIndex=0,.texIndex=224,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=223,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*682 decal_scorch1*/[682]={.modelIndex=77,.colMeshIndex=0,.texIndex=227,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*683 decal_scorch2*/[683]={.modelIndex=77,.colMeshIndex=0,.texIndex=228,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*684 decal_scorch3*/[684]={.modelIndex=77,.colMeshIndex=0,.texIndex=229,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*685 decal_scorch4*/[685]={.modelIndex=77,.colMeshIndex=0,.texIndex=230,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*686 decal_scorchtiny*/[686]={.modelIndex=77,.colMeshIndex=0,.texIndex=232,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*687 decal_blood_splat*/[687]={.modelIndex=77,.colMeshIndex=0,.texIndex=234,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*688 func_switch1*/[688]={.modelIndex=609,.colMeshIndex=0,.texIndex=837,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0.32f,0.04f,0.32f}},
/*689 func_switch2*/[689]={.modelIndex=610,.colMeshIndex=0,.texIndex=839,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){-0.0243553f,0.0f,0.000004883f},.colSz=(V3){0.0476318f,0.64f,0.64f}},
/*690 func_switch3*/[690]={.modelIndex=611,.colMeshIndex=0,.texIndex=842,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){-0.02285008f,0.000053061f,-0.000056993f},.colSz=(V3){0.02f,0.32f,0.32f}},
/*691 func_switch4*/[691]={.modelIndex=612,.colMeshIndex=0,.texIndex=846,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0.06f,0,0},.colSz=(V3){0.2f,0.64f,0.64f}},
/*692 func_switch5*/[692]={.modelIndex=614,.colMeshIndex=0,.texIndex=848,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0.64f,0.64f,0.08f}},
/*693 func_switch5broken*/[693]={.modelIndex=613,.colMeshIndex=0,.texIndex=847,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0.64f,0.64f,0.08f}},
/*694 func_switch7*/[694]={.modelIndex=612,.colMeshIndex=0,.texIndex=854,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){1.523325f,0,0},.colSz=(V3){0.2008026f,0.64f,0.64f}},
/*695 func_switch8*/[695]={.modelIndex=616,.colMeshIndex=0,.texIndex=856,.glowIndex=855,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){-0.04f,0.0f,0.0001220703f},.colSz=(V3){0.08f,0.64f,0.64f}},
/*696 func_switchbroken1*/[696]={.modelIndex=617,.colMeshIndex=0,.texIndex=618,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*697 clip_npc*/[697]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){1.005016f,0,0},.colSz=(V3){2.010033f,16.0f,16.0f}},
/*698 clip_objects*/[698]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){2.56f,2.56f,2.56f}},
/*699 logic_relay*/[699]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*700 logic_branch*/[700]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*701 logic_timer*/[701]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*702 logic_spawner*/[702]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*703 info_teleport_destination*/[703]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*704 prop_debris_panel*/[704]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*705 info_cyborgconversion*/[705]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*706 info_elev_destination*/[706]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*707 info_email*/[707]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*708 info_gameend*/[708]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*709 info_message*/[709]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*710 info_mission*/[710]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*711 info_note*/[711]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*712 info_playsound*/[712]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*713 info_ressurection_point*/[713]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*714 info_screenshake*/[714]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*715 info_spawnpoint*/[715]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*716 fx_reverbzone*/[716]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*717 ef_cyber_ice*/[717]={.modelIndex=MAX_MDLS,.colMeshIndex=U16_MAX,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=COLTYPE_SPH,.colCtr=(V3){0.0f,0.004354001f,-0.014725f},.colSz=(V3){1.0f,0.0f,0.0f}},
/*718 ef_fragexplosion*/[718]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*719 ef_line_sparqbeam*/[719]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*720 ef_mist*/[720]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*721 ef_particle_bloodspurtsmall*/[721]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*722 ef_particle_bloodspurtsmallgreen*/[722]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*723 ef_particle_bloodspurtsmallyellow*/[723]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*724 ef_particle_bloodspurttiny*/[724]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*725 ef_particle_camerahit*/[725]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*726 ef_particle_darthit*/[726]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*727 ef_particle_sec2muzburst*/[727]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*728 ef_particle_sec2rotmuzburst*/[728]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*729 ef_particle_sparksmall*/[729]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*730 ef_particle_sparksmallblue*/[730]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*731 ef_particle_sparqhit*/[731]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*732 ef_sparkspits*/[732]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*733 ef_spraydrips*/[733]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*734 ef_steam*/[734]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*735 env_sparksmall*/[735]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*736 TargetIDInstance*/[736]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*737 prop_papers01*/[737]={.modelIndex=484,.colMeshIndex=0,.texIndex=580,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*738 prop_papers02*/[738]={.modelIndex=485,.colMeshIndex=0,.texIndex=580,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*739 ef_particle_blasterhit*/[739]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*740 ef_particle_ionhit*/[740]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*741 us_puz_panel_blue_wire*/[741]={.modelIndex=0,.colMeshIndex=0,.texIndex=604,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=43,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*742 us_puz_panel_brown_wire*/[742]={.modelIndex=0,.colMeshIndex=0,.texIndex=631,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=43,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*743 us_puz_panel_gray_wire*/[743]={.modelIndex=0,.colMeshIndex=0,.texIndex=634,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=43,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*744 us_puz_panel_red_wire*/[744]={.modelIndex=0,.colMeshIndex=0,.texIndex=625,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=43,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*745 us_puz_panel_teal_wire*/[745]={.modelIndex=0,.colMeshIndex=0,.texIndex=601,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=43,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*746 weapon_grenadeenergmine_live*/[746]={.modelIndex=169,.colMeshIndex=0,.texIndex=852,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*747 decal_logo_storage*/[747]={.modelIndex=77,.colMeshIndex=0,.texIndex=169,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*748 light_animated*/[748]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*749 generic_transform*/[749]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*750 chunk_crate_impenetrable2*/[750]={.modelIndex=61,.colMeshIndex=0,.texIndex=147,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*751 chunk_crate_impenetrable3*/[751]={.modelIndex=61,.colMeshIndex=0,.texIndex=148,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*752 chunk_crate_impenetrable4*/[752]={.modelIndex=61,.colMeshIndex=0,.texIndex=149,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*753 npc_sec3_bot*/[753]={.modelIndex=681,.colMeshIndex=0,.texIndex=553,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*754 prop_shieldgenerator*/[754]={.modelIndex=143,.colMeshIndex=0,.texIndex=316,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*755 unused*/[755]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*756 ef_particle_leafburst*/[756]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*757 ef_particle_mutationburst*/[757]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*758 ef_particle_graytationburst*/[758]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*759 through 766 unused*/[759]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*760*/[760]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*761*/[761]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*762*/[762]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*763*/[763]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*764*/[764]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*765*/[765]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*766*/[766]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
/*767 player (mostly just so any index checks don't accidentally trigger against player by its index)*/[767]={.modelIndex=MAX_MDLS,.colMeshIndex=0,.texIndex=MAX_TXRS,.glowIndex=MAX_TXRS,.specIndex=MAX_TXRS,.normIndex=MAX_TXRS,.mass=0,.dynFriction=0,.statFriction=0,.animationNum=MAX_ANIMS,.col=0,.colCtr=(V3){0,0,0},.colSz=(V3){0,0,0}},
};

static bool cardChunk[307]={/*0*/1,/*1*/1,/*2*/1,/*3*/1,/*4*/1,/*5*/1,/*6*/0,/*7*/1,/*8*/1,/*9*/0,/*10*/0,/*11*/1,/*12*/1,/*13*/1,/*14*/1,/*15*/1,/*16*/1,/*17*/1,/*18*/1,/*19*/1,/*20*/0,/*21*/1,/*22*/0,/*23*/1,/*24*/1,/*25*/1,/*26*/1,/*27*/1,/*28*/1,/*29*/1,                                                                                                                                                                       
 /*30*/1,/*31*/0,/*32*/0,/*33*/1,/*34*/1,/*35*/1,/*36*/1,/*37*/1,/*38*/1,/*39*/1,/*40*/1,/*41*/1,/*42*/0,/*43*/0,/*44*/0,/*45*/1,/*46*/1,/*47*/1,/*48*/1,/*49*/1,/*50*/1,/*51*/1,/*52*/0,/*53*/1,/*54*/1,/*55*/1,/*56*/1,/*57*/1,/*58*/1,/*59*/1,/*60*/1,/*61*/1,                                                                                                                                                             
 /*62*/1,/*63*/0,/*64*/1,/*65*/1,/*66*/1,/*67*/1,/*68*/1,/*69*/1,/*70*/1,/*71*/1,/*72*/1,/*73*/1,/*74*/1,/*75*/1,/*76*/1,/*77*/1,/*78*/0,/*79*/1,/*80*/1,/*81*/1,/*82*/1,/*83*/0,/*84*/1,/*85*/1,/*86*/1,/*87*/0,/*88*/1,/*89*/1,/*90*/1,/*91*/0,/*92*/1,/*93*/1,                                                                                                                                                             
 /*94*/1,/*95*/0,/*96*/1,/*97*/1,/*98*/1,/*99*/1,/*100*/1,/*101*/1,/*102*/1,/*103*/1,/*104*/1,/*105*/1,/*106*/1,/*107*/1,/*108*/1,/*109*/1,/*110*/1,/*111*/1,/*112*/1,/*113*/1,/*114*/1,/*115*/1,/*116*/1,/*117*/1,/*118*/1,/*119*/1,/*120*/1,/*121*/1,/*122*/1,/*123*/1,                                                                                                                                                     
 /*124*/1,/*125*/1,/*126*/1,/*127*/1,/*128*/1,/*129*/1,/*130*/1,/*131*/1,/*132*/1,/*133*/1,/*134*/1,/*135*/1,/*136*/1,/*137*/1,/*138*/1,/*139*/1,/*140*/1,/*141*/1,/*142*/0,/*143*/0,/*144*/1,/*145*/0,/*146*/0,/*147*/0,/*148*/1,/*149*/1,/*150*/0,/*151*/0,/*152*/0,                                                                                                                                                        
 /*153*/0,/*154*/1,/*155*/1,/*156*/1,/*157*/1,/*158*/1,/*159*/1,/*160*/1,/*161*/1,/*162*/1,/*163*/0,/*164*/0,/*165*/0,/*166*/0,/*167*/1,/*168*/0,/*169*/1,/*170*/1,/*171*/1,/*172*/1,/*173*/1,/*174*/1,/*175*/1,/*176*/0,/*177*/0,/*178*/1,/*179*/0,/*180*/1,/*181*/1,                                                                                                                                                        
 /*182*/0,/*183*/1,/*184*/1,/*185*/1,/*186*/1,/*187*/1,/*188*/0,/*189*/1,/*190*/1,/*191*/0,/*192*/0,/*193*/0,/*194*/1,/*195*/1,/*196*/1,/*197*/1,/*198*/1,/*199*/1,/*200*/0,/*201*/1,/*202*/1,/*203*/1,/*204*/1,/*205*/1,/*206*/1,/*207*/1,/*208*/1,/*209*/1,/*210*/0,                                                                                                                                                        
 /*211*/0,/*212*/0,/*213*/0,/*214*/1,/*215*/1,/*216*/1,/*217*/1,/*218*/1,/*219*/0,/*220*/1,/*221*/1,/*222*/1,/*223*/1,/*224*/1,/*225*/1,/*226*/1,/*227*/1,/*228*/1,/*229*/1,/*230*/1,/*231*/1,/*232*/1,/*233*/0,/*234*/1,/*235*/1,/*236*/1,/*237*/1,/*238*/1,/*239*/1,                                                                                                                                                        
 /*240*/1,/*241*/1,/*242*/0,/*243*/0,/*244*/1,/*245*/1,/*246*/0,/*247*/0,/*248*/0,/*249*/0,/*250*/1,/*251*/1,/*252*/1,/*253*/1,/*254*/1,/*255*/0,/*256*/1,/*257*/1,/*258*/1,/*259*/1,/*260*/1,/*261*/1,/*262*/1,/*263*/0,/*264*/0,/*265*/1,/*266*/1,/*267*/1,/*268*/1,                                                                                                                                                        
 /*269*/1,/*270*/1,/*271*/1,/*272*/1,/*273*/1,/*274*/1,/*275*/1,/*276*/1,/*277*/1,/*278*/1,/*279*/1,/*280*/1,/*281*/1,/*282*/1,/*283*/0,/*284*/0,/*285*/0,/*286*/1,/*287*/1,/*288*/1,/*289*/1,/*290*/1,/*291*/0,/*292*/1,/*293*/1,/*294*/1,/*295*/1,/*296*/1,/*297*/1,                                                                                                                                                        
 /*298*/0,/*299*/0,/*300*/0,/*301*/0,/*302*/1,/*303*/0,/*304*/1,/*305*/1,/*306*/1};

__attribute__((noinline)) i32 parse_numberi32(const char* str, const char* line, u32 lineNum) {
    if(str == 0 || *str == '\0'){DualLogError("Invalid on line %d:%s\n",lineNum+1,line); return 0;}
    while(cEmpty((char)*str)){str++;} bool negative=false; if(*str == '+'){str++;}else if(*str == '-'){negative=true; str++;} i64 result=0; int n=0; while(*str >= '0' && *str <= '9' && n++ < 18){result=result*10L + (*str-'0'); str++;} while(*str >= '0' && *str <= '9'){str++;} return (i32)(negative ? -result : result);
}

__attribute__((noinline)) V3 GetLocalTransformedPos(Entity* originator, V3 offsetFromOriginator) { u16 idx=(u16)(originator - World.instances); V3 scaledOfs = mul_v3_v3_elementwise(offsetFromOriginator,World.scale[idx]); V3 rotatedOfs = quat_rot_v3(World.rotation[idx],scaledOfs); V3 result = V3_AplusB(World.position[idx],rotatedOfs); return result; }
__attribute__((noinline)) i16 parse_numberi16(const char* str, const char* line, u32 lineNum) {i32 v=parse_numberi32(str,line,lineNum); if(v < -32768 || v > 32767){DualLogError("%d outrange i16 on line %d:%s\n",v,lineNum+1,line); return 0;} return (i16)v;}
__attribute__((noinline)) i8 parse_numberi8(const char* str, const char* line, u32 lineNum) {i32 v=parse_numberi32(str,line,lineNum); if(v < -128 || v > 127){DualLogError("%d out range i8 on line %d:%s\n",v,lineNum+1,line); return 0;} return (i8)v;}
__attribute__((noinline)) float parse_float(const char* str, const char* line, u32 lineNum) {
    if (str == 0 || *str == '\0') { DualLogError("Blank on line %d:%s\n", lineNum+1, line); return 0.0f; }
    while (cEmpty(*str)) str++;
    bool negative = false; if (*str == '-') { negative = true; str++; } else if (*str == '+') { str++; }
    double value = 0.0; bool has_digit = false;
    while (*str >= '0' && *str <= '9') { value = value * 10.0 + (*str - '0'); str++; has_digit = true; } // Integer part
    if (*str == '.') { str++; double frac = 0.0; double place = 0.1; while (*str >= '0' && *str <= '9') { frac += (*str - '0') * place; place *= 0.1; str++; has_digit = true; } value += frac; } // Decimal part
    if (!has_digit) return 0.0f;
    if (negative) value = -value;
    return (float)value;
}

// Lights
__attribute__((noinline)) i32 AddLight(Light* lit, LightAnimation* lanim) {
    i32 i = World.loadedLights; World.loadedLights++; World.levelLoadedLights[World.currentLevel]++; if (World.loadedLights >= LIGHT_COUNT) { DualLogError("Too many lights %u added in level %d!\n",i,World.curLev); OS_Exit(1); }
    mcpy(&World.lights[i],lit,sizeof(Light)); mcpy(&World.lanims[i],lanim,sizeof(LightAnimation)); World.lightsNewPosition[i] = lit->pos; flag_set(&World.lights[i].lflags,LDIRTY,true); return i;
}

__attribute__((noinline)) u16 AddLightSimple(V3 pos, Color3 c, float r, float in, u16 lf){Light l={.pos=pos,.col=c,.range=r,.intensity=in,.maxIntensity=in,.minIntensity=0.0f,.spotAng=0.0f,.spotDir=QUAT_IDENTITY,.lflags=lf}; LightAnimation a={0}; return AddLight(&l,&a);}
__attribute__((noinline)) u16 AddOffsetLight(Entity* par, V3 offset, Color3 col, float range, float intensity) { return AddLightSimple(GetLocalTransformedPos(par,offset),col,range,intensity,LIGHT_AND_SHADOW_ON); }
bool alreadyReadLightOnOnce[LIGHT_COUNT] = {0};
void LoadFieldIntoLight(char* k, char* v, char* il, u32 ln, Light* lit, LightAnimation* lam, u16 lIdx) {
    char* br = StringFindFirstCharWithin(k,'[');
    if (br) {
        int i = parse_numberu32(br + 1,il,ln);
        if (i >= 0 && i < 32) { if (br - k == 13) { lam->intervalSteps[i] = parse_float(v,il,ln); }/*"intervalSteps[" bracket is at index 13*/ else if (br - k == 21) { lam->stepIsLerping[i] = parse_float(v,il,ln); }/*"intervalStepisLerping[" bracket is at index 21*/ }
        return;
    }
    static const struct { const char* key; u16 offset; u8 type; } map[] = {
        {"currentStep",    __builtin_offsetof(LightAnimation,currentStep),1},{"lerpValue",      __builtin_offsetof(LightAnimation,lerpValue),0},{"intervalSteps.Length",__builtin_offsetof(LightAnimation,numIntervalSteps),1},{"intervalStepisLerping.Length",__builtin_offsetof(LightAnimation, numLerpSteps),1},
        {"lP.x",__builtin_offsetof(Light,pos.x),0},{"lP.y",__builtin_offsetof(Light,pos.y),0},{"lP.z",__builtin_offsetof(Light,pos.z),0},{"lR.x",__builtin_offsetof(Light,spotDir.x),0},{"lR.y",__builtin_offsetof(Light,spotDir.y),0},{"lR.z",__builtin_offsetof(Light,spotDir.z),0},{"lR.w",__builtin_offsetof(Light,spotDir.w),0},
        {"range",__builtin_offsetof(Light,range),0},{"spotAngle",__builtin_offsetof(Light,spotAng),0},{"minIntensity",__builtin_offsetof(Light,minIntensity),0},{"maxIntensity",__builtin_offsetof(Light,maxIntensity),0},{"color.r",__builtin_offsetof(Light,col.r),0},{"color.g",__builtin_offsetof(Light,col.g),0},{"color.b",__builtin_offsetof(Light,col.b),0}
    };
    for (int i = 0; i < (int)(sizeof(map)/sizeof(map[0])); i++) {
        if (sEqual(k, map[i].key)) { // Types: 0 = float, 1 = u8.  Check key prefix to decide if pointing at 'lit' or 'lam'
            void* dest = (k[0] == 'l' && (k[1] == 'P' || k[1] == 'R')) ? (void*)lit : (void*)lam;
            if (k[0] == 'r' || k[0] == 's' || k[0] == 'm' || k[0] == 'c') {
                if (k[1] != 'u') dest = (void*)lit; // range, spot, max, color (not currentStep)
            }
            char* ptr = (char*)dest + map[i].offset;
            if (map[i].type == 0) *(float*)ptr = parse_float(v,il,ln);
            else                  *(u8*)ptr = parse_numberu8(v,il,ln);
            return;
        }
    }
         if (sEqual(k,"intensity")) {lit->intensity = lit->maxIntensity = parse_float(v,il,ln) * 0.35f;}                                                        else if (sEqual(k,"type"))   {flag_set(&lit->lflags, (v[0] == 'S') ? LSPOT : LDIR, true);}
    else if (sEqual(k,"lightOn") && !alreadyReadLightOnOnce[lIdx]) { alreadyReadLightOnOnce[lIdx] = true; flag_set(&lit->lflags,LIGHTON,parse_bool(v,il,ln)); } else if (sEqual(k,"lerpOn")) {flag_set(&lit->lflags,LERPON,parse_bool(v,il,ln));}
}

u16 headmountedLanternLight; V3 lanternPos;
#define CHGD(a,b) (vabs((a) - (b)) > 0.0001f)
void UpdateLight(u16 i, V3 pos, Color3 col, float range, float intensity, float max, float min, float spotAng, Quaternion spotDir, bool on, bool shad) {
    bool changed = ((!!(World.lights[i].lflags & SHADON) - shad) || (!!(World.lights[i].lflags & LIGHTON) -  on) || CHGD(World.lights[i].range,range) || CHGD(World.lights[i].pos.x,pos.x) || CHGD(World.lights[i].pos.y,pos.y) || CHGD(World.lights[i].pos.z,pos.z));
    World.lights[i].intensity=intensity; World.lights[i].minIntensity=min; World.lights[i].maxIntensity=max; World.lights[i].spotAng=spotAng; World.lights[i].spotDir=spotDir; World.lights[i].col=col; World.lights[i].pos=World.lightsNewPosition[i]=pos; World.lights[i].range=range;
    World.lights[i].lflags = (World.lights[i].lflags & ~(LIGHTON | SHADON | LDIRTY)) | ((World.lights[i].lflags & LDIRTY) | (changed << 4) | on | (shad << 1));
}
#undef CHGD
// Level Loading and Entity Management System
void InitNPC(u16 i);
void DeleteInstance(u16 i) { if (i <= PLAYER1 || i >= World.instCount) return; flag_set(&World.instances[i].entflags,EF_ACTIVE,false); } // Don't delete null ent, player 1, nor player 2 or already empty slots.
__attribute__((noinline)) u16 AddInstance(u16 entIdx, V3 pos) {
    if (entIdx >= MAX_ENTITIES) { DualLogError("\nEntity index when loading non-light entity was %d, exceeds max defined entity count of %d, skipped\n",entIdx,MAX_ENTITIES); return 0; }
    if (World.instCount >= INSTANCE_COUNT) { DualLogError("\nToo many instances while adding entity %u, max instance count is %u, skipped\n", entIdx, INSTANCE_COUNT); return 0; }
    u16 i = World.instCount;
    mset(&World.instances[i],0,sizeof(Entity));
    World.instances[i].entflags=EF_ACTIVE; World.layer[i]=L_Default;World.instances[i].camView=255;
    World.instances[i].modelIndex=World.instances[i].lodIndex=World.instances[i].colMeshIndex=MAX_MDLS;
    World.instances[i].texIndex=World.instances[i].glowIndex=World.instances[i].specIndex=World.instances[i].normIndex = MAX_TXRS;
    World.scale[i].x=World.scale[i].y=World.scale[i].z=World.mass[i]=World.rotation[i].w=1.0f; World.dynamicFriction[i]=0.5f; World.staticFriction[i]=0.6f;
    World.instances[i].index = entIdx;    World.position[i] = pos;
    if (entIdx == 424) { World.scale[i].x = World.scale[i].y = World.scale[i].z = 0.8f; } // npc_cortex_reaver
    if (entIdx == 430) { World.scale[i].x = World.scale[i].y = World.scale[i].z = 0.9f; } // npc_sec2_bot
    if (entIdx == 431) { World.scale[i].x = World.scale[i].y = World.scale[i].z = 0.4f; } // npc_maint_bot
    if (entIdx == 433) { World.scale[i].x = World.scale[i].y = World.scale[i].z = 0.88f; } // npc_hopper
    if (entIdx == 439) { World.scale[i].x = World.scale[i].y = World.scale[i].z = 0.4f; } // npc_zerog_mutant
    if (entIdx == 441) { World.scale[i].x = World.scale[i].y = World.scale[i].z = 0.75f; } // npc_repairbot
    if (entIdx == 444) { World.scale[i].x = World.scale[i].y = World.scale[i].z = 0.666f; } // npc_cyberguard
    if (entIdx == 445) { World.scale[i].x = World.scale[i].y = World.scale[i].z = 0.5f; } // npc_cyberram
    if (entIdx == 446) { World.scale[i].x = World.scale[i].y = World.scale[i].z = 1.1f; } // npc_cyber_reaver
    if (entIdx == 475 || entIdx == 476) World.scale[i] = (V3){1.75f,1.75f,1.75f};
    if (IdxIsNPC(entIdx)) InitNPC(i);
    if (IdxIsDoor(entIdx)) { World.instances[i].SFXIndex = 75; }
    World.instances[i].modelIndex = EDefs[entIdx].modelIndex;
    World.instances[i].colMeshIndex = EDefs[entIdx].colMeshIndex;
    World.instances[i].animationNum = EDefs[entIdx].animationNum;
    World.instances[i].texIndex = EDefs[entIdx].texIndex;
    World.instances[i].glowIndex = EDefs[entIdx].glowIndex >= MAX_TXRS ? 0 : EDefs[entIdx].glowIndex;
    World.instances[i].specIndex = EDefs[entIdx].specIndex >= MAX_TXRS ? 0 : EDefs[entIdx].specIndex;
    World.instances[i].normIndex = EDefs[entIdx].normIndex >= MAX_TXRS ? 0 : EDefs[entIdx].normIndex;
    flag_set(&World.instances[i].entflags,EF_RIGIDBODY,IdxIsDynamicObject(entIdx));
    World.col[i] = EDefs[entIdx].col;
    World.colliderCenter[i] = EDefs[entIdx].colCtr;
    World.colliderSize[i] = EDefs[entIdx].colSz;
    World.mass[i] = EDefs[entIdx].mass > 0.0f ? EDefs[entIdx].mass : 1.0f;
    World.gravity[i] = IdxIsDynamicObject(World.instances[i].index) ? 1.0f : 0.0f;
    if (IdxIsButtonSwitch(entIdx)) { World.instances[i].lockedMessageLingdex = 193; } // ButtonSwitch
    if (entIdx < 307 && cardChunk[entIdx]) { World.instances[i].lodIndex=178;/*LOD card index*/ World.col[i]=COLTYPE_BOX; World.colliderCenter[i].y=1.32f; World.colliderSize[i]=(V3){2.56f,0.08f,2.56f}; }
    World.instCount++; World.levelInstCount[World.currentLevel]=World.instCount;
    return i;
}

static const char* mm_ptr; static const char* mm_end;
#define KEY_EQ(lit) (keyLen == (int)(sizeof(lit)-1) && sCompUpToLen(key, lit, sizeof(lit)-1) == 0)
static char* MmapGetLine(char* buf, int sz) {
    if (mm_ptr >= mm_end) return NULL;
    const char* start=mm_ptr; const char* p=start;
    while (p < mm_end && *p != '\n') { ++p; }
    int lineLen = (int)(p - start);
    if (p < mm_end && *p == '\n') { mm_ptr=p + 1; }
    else { mm_ptr=mm_end; }
    if (lineLen >= sz) { lineLen = sz - 1; }
    mcpy(buf,(void*)start,lineLen);
    while (lineLen > 0 && (buf[lineLen - 1] == '\r' || buf[lineLen - 1] == '\n')) { --lineLen; }
    buf[lineLen] = '\0';
    return buf;
}

void SetLevelPointers(u8 lev) {
    if (lev >= MAX_LEVELS) return;
    World.currentLevel = lev;
    World.instances        = World.levelInstances[lev];
    World.position         = World.levelPosition[lev];
    World.scale            = World.levelScale[lev];
    World.velocity         = World.levelVelocity[lev];
    World.angularVelocity  = World.levelAngularVelocity[lev];
    World.colliderCenter   = World.levelColliderCenter[lev];
    World.colliderSize     = World.levelColliderSize[lev];
    World.col              = World.levelCollider[lev];
    World.rotation         = World.levelRotation[lev];
    World.layer            = World.levelLayer[lev];
    World.mass             = World.levelMass[lev];
    World.radius           = World.levelRadius[lev];
    World.gravity          = World.levelGravity[lev];
    World.inertiaTensor    = World.levelInertiaTensor[lev];
    World.invInertiaTensor = World.levelInvInertiaTensor[lev];
    World.dynamicFriction  = World.levelDynamicFriction[lev];
    World.staticFriction   = World.levelStaticFriction[lev];
    World.invTnsrValid     = World.levelInvTnsrValid[lev];
    World.colliding        = World.levelColliding[lev];
    World.instCount        = World.levelInstCount[lev];
    World.lights            = World.levelLights[lev];
    World.lanims            = World.levelLAnims[lev];
    World.lightsNewPosition = World.levelLightsNewPosition[lev];
    World.loadedLights      = World.levelLoadedLights[lev];
}

void CopyPlayerState(u8 srcLevel, u8 dstLevel) {
    if (srcLevel >= MAX_LEVELS || dstLevel >= MAX_LEVELS || srcLevel == dstLevel) return;
    u16 s = PLAYER1;
    World.levelInstances[dstLevel][s]            = World.levelInstances[srcLevel][s];
    World.levelPosition[dstLevel][s]             = World.levelPosition[srcLevel][s];
    World.levelScale[dstLevel][s]                = World.levelScale[srcLevel][s];
    World.levelVelocity[dstLevel][s]             = World.levelVelocity[srcLevel][s];
    World.levelAngularVelocity[dstLevel][s]      = World.levelAngularVelocity[srcLevel][s];
    World.levelColliderCenter[dstLevel][s]       = World.levelColliderCenter[srcLevel][s];
    World.levelColliderSize[dstLevel][s]         = World.levelColliderSize[srcLevel][s];
    World.levelCollider[dstLevel][s]             = World.levelCollider[srcLevel][s];
    World.levelRotation[dstLevel][s]             = World.levelRotation[srcLevel][s];
    World.levelLayer[dstLevel][s]                = World.levelLayer[srcLevel][s];
    World.levelMass[dstLevel][s]                 = World.levelMass[srcLevel][s];
    World.levelRadius[dstLevel][s]               = World.levelRadius[srcLevel][s];
    World.levelGravity[dstLevel][s]              = World.levelGravity[srcLevel][s];
    mcpy(World.levelInertiaTensor[dstLevel][s],    World.levelInertiaTensor[srcLevel][s],    6 * sizeof(float));
    mcpy(World.levelInvInertiaTensor[dstLevel][s], World.levelInvInertiaTensor[srcLevel][s], 6 * sizeof(float));
    World.levelDynamicFriction[dstLevel][s]      = World.levelDynamicFriction[srcLevel][s];
    World.levelStaticFriction[dstLevel][s]       = World.levelStaticFriction[srcLevel][s];
    World.levelInvTnsrValid[dstLevel][s]         = World.levelInvTnsrValid[srcLevel][s];
    World.levelColliding[dstLevel][s]            = World.levelColliding[srcLevel][s];
}

char lineSpace[LINE_LEN_MAX]; void AddDoorPortal(u16 entIdx, u16 parent); void TextureSequenceInit(u16 self, char* trimmed_value); void AddCamView(V3 p, Quaternion r, u8 fv, u16 w, u16 h, float nr, float fr);
void LoadLevelMod(u8 lev) {
    u8 curlevel = vclamp(lev, 0, 13); World.curLev = curlevel; World.levelCurrentlyLoading = true; World.instCount = 3;
    if (curlevel == 1) {
        AddCamView((V3){-19.2301f,-42.6604f,-49.7453f},(Quaternion){0.2375f,0.0008f,-0.0002f,0.9713f},75u,256u,256u,2.21f,11.5f);
        AddCamView((V3){7.664583f,-44.88017f,-14.26742f},(Quaternion){0.0f,0.9999f,0.0129f,0.0f},60u,256u,256u,2.192f,20.6f);
    } // TODO other level camviews
    mset(lineSpace,0,LINE_LEN_MAX * sizeof(char)); u32 lineNum = 0; i32 entCount = -1, lightsIdx = -1; char* line;
    for (u16 i=0;i<LIGHT_COUNT;++i) { mset(&lightsFromFile[i],0,sizeof(Light)); mset(&lanimsFromFile[i],0,sizeof(LightAnimation)); lightsFromFile[i].range = 5.5f; lightsFromFile[i].col = (Color3){1.0f,1.0f,1.0f}; lightsFromFile[i].spotAng=0.0f; }
    while (MmapGetLine(lineSpace, LINE_LEN_MAX)) {
        lineNum++; line = lineSpace; char* firstColon = StringFindFirstCharWithin(line, ':'); int firstKeyLen = firstColon ? (int)(firstColon - line) : 0;
        bool isLight = !(firstKeyLen == 10 && sCompUpToLen(line, "constIndex", 10) == 0);
        Entity* inst=NULL; Light* lit=NULL; LightAnimation* lanim=NULL;
        if (isLight) {
            lightsIdx++; if (lightsIdx >= LIGHT_COUNT) { DualLogError("Too many lights %u in level%d.txt!\n", lightsIdx, curlevel); continue; }
            lit=&lightsFromFile[lightsIdx]; lanim=&lanimsFromFile[lightsIdx];
            lit->lflags = LIGHT_AND_SHADOW_ON; // default per-light
        } else {
            entCount++;
            if (entCount >= INSTANCE_COUNT) { DualLogError("Too many instances %u in level%d.txt!\n", entCount, curlevel); continue; }
            inst = &entsFromFile[entCount]; mset(inst,0,sizeof(Entity)); mset(&posFromFile[entCount],0,sizeof(V3)); scaleFromFile[entCount] = (V3){1.0f, 1.0f, 1.0f}; rotationFromFile[entCount] = QUAT_IDENTITY; // Zero this entity slot only
        }
        bool activeStateRead = false;
        while (line[0] != '\0') {
            char* pipe = StringFindFirstCharWithin(line, '|'); char* kvString = line;
            if (pipe) { *pipe = '\0'; line = pipe + 1; } else { line += slen(line); }
            if (kvString[0] == '\0') continue;
            char* colon = StringFindFirstCharWithin(kvString, ':'); if (!colon || colon[1] == '\0') continue;
            *colon = '\0'; char* key = kvString; char* value = colon + 1; int keyLen = (int)(colon - key); // length is free, no slen()
            if (isLight) { LoadFieldIntoLight(key,value,lineSpace,lineNum,lit,lanim,lightsIdx);
            } else {
                     if (KEY_EQ("constIndex"))           inst->index = parse_numberu16(value, lineSpace, lineNum);
                else if (KEY_EQ("lP.x")) posFromFile[entCount].x = parse_float(value, lineSpace, lineNum);
                else if (KEY_EQ("lP.y")) posFromFile[entCount].y = parse_float(value, lineSpace, lineNum);
                else if (KEY_EQ("lP.z")) posFromFile[entCount].z = parse_float(value, lineSpace, lineNum);
                else if (KEY_EQ("lR.x")) rotationFromFile[entCount].x = parse_float(value, lineSpace, lineNum);
                else if (KEY_EQ("lR.y")) rotationFromFile[entCount].y = parse_float(value, lineSpace, lineNum);
                else if (KEY_EQ("lR.z")) rotationFromFile[entCount].z = parse_float(value, lineSpace, lineNum);
                else if (KEY_EQ("lR.w")) rotationFromFile[entCount].w = parse_float(value, lineSpace, lineNum);
                else if (KEY_EQ("lS.x"))    scaleFromFile[entCount].x = parse_float(value, lineSpace, lineNum);
                else if (KEY_EQ("lS.y"))    scaleFromFile[entCount].y = parse_float(value, lineSpace, lineNum);
                else if (KEY_EQ("lS.z"))    scaleFromFile[entCount].z = parse_float(value, lineSpace, lineNum);
                else if (KEY_EQ("go.activeSelf"))   { activeStateRead = true; flag_set(&inst->entflags, EF_ACTIVE, parse_bool(value, lineSpace, lineNum)); }
                else if (KEY_EQ("amount"))          inst->amount = parse_float(value, lineSpace, lineNum);
                else if (KEY_EQ("resetTime"))       inst->resetTime = parse_float(value, lineSpace, lineNum);
                else if (KEY_EQ("minSecurityLevel"))inst->minSecurityLevel = parse_float(value, lineSpace, lineNum);
                else if (KEY_EQ("damageOnUse"))     inst->damage = parse_float(value, lineSpace, lineNum);
                else if (KEY_EQ("target"))          scpy_to_a_from_b(inst->target, value, TARGET_STRING_LENGTH);
                else if (KEY_EQ("targetname"))      scpy_to_a_from_b(inst->targetname, value, TARGET_STRING_LENGTH);
                else if (KEY_EQ("securityThreshhold") || KEY_EQ("securityThreshold")) inst->securityThreshold = parse_numberu8(value, lineSpace, lineNum);
                else if (KEY_EQ("messageIndex"))    inst->messageIndex = parse_numberi16(value, lineSpace, lineNum);
                else if (KEY_EQ("delay"))           inst->delay = parse_float(value, lineSpace, lineNum);
                else if (KEY_EQ("locked"))          flag_set(&inst->entflags, EF_LOCKED, parse_bool(value, lineSpace, lineNum));
                else if (KEY_EQ("active"))          inst->active = parse_bool(value, lineSpace, lineNum);
                else if (KEY_EQ("onlyTargetOnce"))  inst->onlyOnce = parse_bool(value, lineSpace, lineNum);
                else if (KEY_EQ("targetAlreadyDone")) inst->targetAlreadyDone = parse_bool(value, lineSpace, lineNum);
                else if (KEY_EQ("stayOpen"))        inst->stayOpen = parse_bool(value, lineSpace, lineNum);
                else if (KEY_EQ("startOpen"))       inst->startOpen = parse_bool(value, lineSpace, lineNum);
                else if (KEY_EQ("ajar"))            inst->ajar = parse_bool(value, lineSpace, lineNum);
                else if (KEY_EQ("ajarPercentage"))  inst->ajarPercentage = parse_float(value, lineSpace, lineNum);
                else if (KEY_EQ("useTimeDelay"))    inst->useTimeDelay = parse_float(value, lineSpace, lineNum);
                else if (KEY_EQ("blocked"))         inst->blocked = parse_bool(value, lineSpace, lineNum);
                else if (KEY_EQ("timeBeforeLasersOn")) inst->timeBeforeLasersOn = parse_float(value, lineSpace, lineNum);
                else if (KEY_EQ("toggleLasers"))    inst->toggleLasers = parse_bool(value, lineSpace, lineNum);
                else if (KEY_EQ("targettingOnlyUnlocks")) inst->targettingOnlyUnlocks = parse_bool(value, lineSpace, lineNum);
                else if (KEY_EQ("changeLayerOnOpenClose")) inst->changeLayerOnOpenClose = parse_bool(value, lineSpace, lineNum);
                else if (KEY_EQ("useFinished"))     inst->useFinished = parse_float(value, lineSpace, lineNum) + World.pauseRelativeTime;
                else if (KEY_EQ("waitBeforeClose")) inst->waitBeforeClose = parse_float(value, lineSpace, lineNum) + World.pauseRelativeTime;
                else if (KEY_EQ("lasersFinished"))  inst->lasersFinished = parse_float(value, lineSpace, lineNum) + World.pauseRelativeTime;
                else if (KEY_EQ("doorOpen"))        flag_set(&inst->ioflags, TARG_IOFLAGS_DOOROPEN, parse_bool(value, lineSpace, lineNum));
                else if (KEY_EQ("doorOpenIfUnlocked") || KEY_EQ("doorToggle")) flag_set(&inst->ioflags, TARG_IOFLAGS_DOOROPENIFUNLOCKED, parse_bool(value, lineSpace, lineNum));
                else if (KEY_EQ("doorClose"))       flag_set(&inst->ioflags, TARG_IOFLAGS_DOORCLOSE, parse_bool(value, lineSpace, lineNum));
                else if (KEY_EQ("doorLock") || KEY_EQ("lockElevatorPad")) flag_set(&inst->ioflags, TARG_IOFLAGS_LOCK, parse_bool(value, lineSpace, lineNum));
                else if (KEY_EQ("doorUnlock") || KEY_EQ("unlockSwitch") || KEY_EQ("unlockElevatorPad") || KEY_EQ("unlockKeycodePad") || KEY_EQ("unlockPuzzlePad")) flag_set(&inst->ioflags, TARG_IOFLAGS_UNLOCK, parse_bool(value, lineSpace, lineNum));
                else if (KEY_EQ("switchTrigger"))   flag_set(&inst->ioflags, TARG_IOFLAGS_SWITCHTRIGGER, parse_bool(value, lineSpace, lineNum));
                else if (KEY_EQ("tripTrigger"))     flag_set(&inst->ioflags, TARG_IOFLAGS_TRIPTRIGGER, parse_bool(value, lineSpace, lineNum));
                else if (KEY_EQ("forceBridgeActivate"))   flag_set(&inst->ioflags, TARG_IOFLAGS_FBRIDGE_ACTIVATE, parse_bool(value, lineSpace, lineNum));
                else if (KEY_EQ("forceBridgeDeactivate")) flag_set(&inst->ioflags, TARG_IOFLAGS_FBRIDGE_DEACTIVATE, parse_bool(value, lineSpace, lineNum));
                else if (KEY_EQ("forceBridgeToggle"))     flag_set(&inst->ioflags, TARG_IOFLAGS_FBRIDGE_TOGGLE, parse_bool(value, lineSpace, lineNum));
                else if (KEY_EQ("gravityLiftToggle"))     flag_set(&inst->ioflags, TARG_IOFLAGS_GRAVLIFT_TOGGLE, parse_bool(value, lineSpace, lineNum));
                else if (KEY_EQ("textureChangeToggle"))   flag_set(&inst->ioflags, TARG_IOFLAGS_TEXTURE_CHG_TOGGLE, parse_bool(value, lineSpace, lineNum));
                else if (KEY_EQ("lightOn"))         flag_set(&inst->ioflags, TARG_IOFLAGS_LIGHT_ON, parse_bool(value, lineSpace, lineNum));
                else if (KEY_EQ("lightOff"))        flag_set(&inst->ioflags, TARG_IOFLAGS_LIGHT_OFF, parse_bool(value, lineSpace, lineNum));
                else if (KEY_EQ("lightToggle"))     flag_set(&inst->ioflags, TARG_IOFLAGS_LIGHT_TOGGLE, parse_bool(value, lineSpace, lineNum));
                else if (KEY_EQ("funcwallMove"))    flag_set(&inst->ioflags, TARG_IOFLAGS_FUNCWALL_MOVE, parse_bool(value, lineSpace, lineNum));
                else if (KEY_EQ("missionBitOn"))    flag_set(&inst->ioflags, TARG_IOFLAGS_MISSION_BIT_ON, parse_bool(value, lineSpace, lineNum));
                else if (KEY_EQ("missionBitOff"))   flag_set(&inst->ioflags, TARG_IOFLAGS_MISSION_BIT_OFF, parse_bool(value, lineSpace, lineNum));
                else if (KEY_EQ("missionBitToggle"))flag_set(&inst->ioflags, TARG_IOFLAGS_MISSION_BIT_TOGGLE, parse_bool(value, lineSpace, lineNum));
                else if (KEY_EQ("switchLockToggle"))flag_set(&inst->ioflags, TARG_IOFLAGS_SWITCH_LOCK_TOGGLE, parse_bool(value, lineSpace, lineNum));
                else if (KEY_EQ("GOSetActive"))     flag_set(&inst->ioflags, TARG_IOFLAGS_INST_ACTIVATE, parse_bool(value, lineSpace, lineNum));
                else if (KEY_EQ("GOSetDeactive"))   flag_set(&inst->ioflags, TARG_IOFLAGS_INST_DEACTIVATE, parse_bool(value, lineSpace, lineNum));
                else if (KEY_EQ("GOToggleActive"))  flag_set(&inst->ioflags, TARG_IOFLAGS_INST_TOGGLE, parse_bool(value, lineSpace, lineNum));
                else if (KEY_EQ("disableThisGOOnAwake")) flag_set(&inst->ioflags, TARG_IOFLAGS_DISABLE_ON_AWAKE, parse_bool(value, lineSpace, lineNum));
                else if (KEY_EQ("playSoundOnce"))   flag_set(&inst->ioflags, TARG_IOFLAGS_PLAY_SOUND_ONCE, parse_bool(value, lineSpace, lineNum));
                else if (KEY_EQ("stopSound"))       flag_set(&inst->ioflags, TARG_IOFLAGS_STOP_SOUND, parse_bool(value, lineSpace, lineNum));
                else if (KEY_EQ("startFlashingMaterials")) flag_set(&inst->ioflags, TARG_IOFLAGS_START_FLASHING_TEX, parse_bool(value, lineSpace, lineNum));
                else if (KEY_EQ("stopFlashingMaterials"))  flag_set(&inst->ioflags, TARG_IOFLAGS_STOP_FLASHING_TEX, parse_bool(value, lineSpace, lineNum));
                else if (KEY_EQ("branchFlip"))      flag_set(&inst->ioflags, TARG_IOFLAGS_BRANCH_FLIP, parse_bool(value, lineSpace, lineNum));
                else if (KEY_EQ("branchFlipOnly"))  flag_set(&inst->ioflags, TARG_IOFLAGS_BRANCH_FLIPONLY, parse_bool(value, lineSpace, lineNum));
                else if (KEY_EQ("resourceFolder") && *value) scpy_to_a_from_b(inst->texAnimResourceFolder, value, TARGET_STRING_LENGTH);
                else if (KEY_EQ("randomFrame"))     inst->texAnimRandom = parse_bool(value, lineSpace, lineNum);
                else if (KEY_EQ("reverseSequence")) inst->texAnimInReverse = parse_bool(value, lineSpace, lineNum);
                else if (KEY_EQ("messageLingdex"))  inst->messageLingdex = parse_numberi16(value, lineSpace, lineNum);
                else if (KEY_EQ("lockedMessageLingdex")) inst->lockedMessageLingdex = parse_numberi16(value, lineSpace, lineNum);
                else if (KEY_EQ("SFXIndex"))        inst->SFXIndex = (i16)parse_numberi16(value, lineSpace, lineNum);
                else if (KEY_EQ("requiredAccessCard")) inst->requiredAccessCard = parse_numberi8(value, lineSpace, lineNum);
                else if (KEY_EQ("doorOpenState"))   inst->doorOpen = parse_numberu8(value, lineSpace, lineNum);
            }
        }
        if (!isLight && !activeStateRead) flag_set(&entsFromFile[entCount].entflags,EF_ACTIVE,true); // Default active if not specified
    }
    i32 totalEnts = entCount + 1;
    for (i32 e=0;e<totalEnts;++e) {
        Entity* src = &entsFromFile[e];
        u16 entIdx = src->index;
        u16 parent = AddInstance(entIdx,posFromFile[e]);
        Entity* par = &World.instances[parent];
        par->lastPosition          = posFromFile[e];
        World.rotation[parent]    = rotationFromFile[e];
        if (!IdxIsDynamicObject(entIdx)) {World.scale[parent] = scaleFromFile[e];}
        par->entflags             |= src->entflags; // bitor `|` since AddInstance already set flags from entity definitions.
        par->ioflags               = src->ioflags;
        par->amount                = src->amount;
        par->resetTime             = src->resetTime;
        par->minSecurityLevel      = src->minSecurityLevel;
        par->damage                = src->damage;
        par->delay                 = src->delay;
        par->active                = src->active;
        par->onlyOnce              = src->onlyOnce;
        par->targetAlreadyDone     = src->targetAlreadyDone;
        par->stayOpen              = src->stayOpen;
        par->startOpen             = src->startOpen;
        par->ajar                  = src->ajar;
        par->ajarPercentage        = src->ajarPercentage;
        par->useTimeDelay          = src->useTimeDelay;
        par->blocked               = src->blocked;
        par->timeBeforeLasersOn    = src->timeBeforeLasersOn;
        par->toggleLasers          = src->toggleLasers;
        par->targettingOnlyUnlocks = src->targettingOnlyUnlocks;
        par->changeLayerOnOpenClose= src->changeLayerOnOpenClose;
        par->useFinished           = src->useFinished;
        par->waitBeforeClose       = src->waitBeforeClose;
        par->lasersFinished        = src->lasersFinished;
        par->securityThreshold     = src->securityThreshold;
        par->texAnimRandom         = src->texAnimRandom;
        par->texAnimInReverse      = src->texAnimInReverse;
        par->messageLingdex        = src->messageLingdex;
        scpy_to_a_from_b(par->target, src->target, TARGET_STRING_LENGTH);
        scpy_to_a_from_b(par->targetname, src->targetname, TARGET_STRING_LENGTH);
        scpy_to_a_from_b(par->texAnimResourceFolder, src->texAnimResourceFolder, TARGET_STRING_LENGTH);
        if (IdxIsPortalBlockingDoor(entIdx)) AddDoorPortal(entIdx,parent); // Only at load, not in AddInstance
        if (entIdx == 525) { // prop_console01
            par->texAnimLight  = AddOffsetLight(par,(V3){5.81f,2.29f,38.05f-38.3552f},(Color3){0.3531f,0.4837f,0.6509f},1.85f,0.7f);
            par->texAnimLight2 = AddOffsetLight(par,(V3){-10.1f,0.9f,18.21f-38.3552f},(Color3){0.3561f,0.3561f,0.8970f},2.0f,1.12f);
        } else if (entIdx == 279) { par->texAnimLight = AddOffsetLight(par,(V3){0.0f,-0.08f,0.0f},(Color3){0.909803922f,0.929411765f,1.0f},3.2f,1.575f); } // chunk_screen
        else if (par->index == 574) { // prop_healingbed
            Color3 green = {0.0f, 0.925490196f, 0.082352941f};
            par->texAnimLight  = AddOffsetLight(par, (V3){0.5292511f, 0.065f, 0.915f}, green, 3.0f, 0.72f);
            par->texAnimLight2 = AddOffsetLight(par, (V3){-0.5317497f, 0.065f, 1.039f}, green, 3.0f, 0.72f);
            par->textureAnimating = true; par->texAnimClip = 12; par->texFrame = 0;
            scpy_to_a_from_b(par->texAnimResourceFolder,"MedicalBed",TARGET_STRING_LENGTH);
        } else if (entIdx == 309 || entIdx == 365 || entIdx == 369) { World.position[parent].y += 0.12f; } // item_beaker || item_flask || item_testtube: Move up to account for CG mod (origin moved vs Unity version)
        else if (entIdx == 328) { World.position[parent].y += 0.04f; } // item_hw_system: Move up to account for CG mod (origin moved vs Unity version)
        else if (entIdx == 310) { World.position[parent].y += 0.0975f; } // item_beverage: Move up to account for CG mod (origin moved vs Unity version)
        else if (entIdx == 314) { World.position[parent].y += 0.095f; } // weapon_grenadefrag: Move up to account for CG mod (origin moved vs Unity version)
        else if (entIdx == 315) { World.position[parent].y += 0.065f; } // weapon_grenadeconc: Move up to account for CG mod (origin moved vs Unity version)
        else if (entIdx == 316) { World.position[parent].y += 0.08f; } // weapon_grenadeemp: Move up to account for CG mod (origin moved vs Unity version)
        else if (entIdx == 317) { World.position[parent].y += 0.12f; } // weapon_grenadeearth: Move up to account for CG mod (origin moved vs Unity version)
        else if (entIdx == 318) { World.position[parent].y += 0.02f; } // weapon_grenadeemine: Move up to account for CG mod (origin moved vs Unity version)
        else if (entIdx == 319) { World.position[parent].y += 0.09f; } // weapon_grenadegas: Move up to account for CG mod (origin moved vs Unity version)
        else if (entIdx == 332) { World.position[parent].y += 0.015f; } // item_hw_targetid: Move up to account for CG mod (origin moved vs Unity version)
        else if (entIdx == 333) { World.position[parent].y += 0.028f; } // item_hw_shield: Move up to account for CG mod (origin moved vs Unity version)
        else if (entIdx == 342) { World.position[parent].y += 0.12f; } // item_workerhelmet: Move up to account for CG mod (origin moved vs Unity version)
        //else if (entIdx == 343) { World.position[parent].y += 0.16f; } // weapon_blaster: Move up to account for CG mod (origin moved vs Unity version) TODO Check if this is needed to be done on prerotated value
        //else if (entIdx == 346) { World.position[parent].z += 0.16f; } // weapon_blaster: Move over to account for CG mod (origin moved vs Unity version) TODO Check if this is needed to be done on prerotated value, also not sure if barrel parallel axis is x or z that I slid this along
        //else if (entIdx == 348) { World.position[parent].z += 0.6f; } // weapon_blaster: Move over to account for CG mod (origin moved vs Unity version) TODO Check if this is needed to be done on prerotated value, also not sure if blade parallel axis is x or z that I slid this along
        else if (entIdx == 345) { World.scale[parent].x=World.scale[parent].y=World.scale[parent].z=1.00f; } // weapon_dartgun
        else if (entIdx == 350) { World.position[parent].y += 0.16f; } // weapon_magnum: Move over to account for CG mod (origin moved vs Unity version)
        else if (entIdx == 352) { World.position[parent].y += 0.16f; } // weapon_pistol: Move over to account for CG mod (origin moved vs Unity version)
        else if (entIdx == 358) { World.position[parent].y += 0.16f; } // weapon_stungun: Move up to account for CG mod (origin moved vs Unity version)
        else if (entIdx == 361) { World.position[parent].y += 0.05f; } // item_logic_probe: Move up to account for CG mod (origin moved vs Unity version)
        else if (entIdx == 363) { World.position[parent].y += 0.04f; } // item_plastique: Move up to account for CG mod (origin moved vs Unity version)
        else if (entIdx == 364 || entIdx == 366) { World.position[parent].y += 0.03f; } // item_chipset_isolinear: Move up to account for CG mod (origin moved vs Unity version)
        else if (entIdx == 368) { World.position[parent].y += 0.04f; } // item_isotopex22: Move up to account for CG mod (origin moved vs Unity version)
        else if (entIdx == 371) { World.position[parent].y += 0.015f; } // item_chipset_isolinear: Move up to account for CG mod (origin moved vs Unity version)
        else if (entIdx == 379 || entIdx == 380 || entIdx == 381 || entIdx == 382 || entIdx == 384) { World.position[parent].y += 0.025f; } // item_ammo_hornet, item_ammo_splinter, item_ammo_magnesium, item_ammo_penetrator: Move up to account for CG mod (origin moved vs Unity version)
        else if (entIdx == 399) { World.position[parent].y += 0.01f; } // item_head_male: Move up to account for CG mod (origin moved vs Unity version)
        else if (entIdx == 458 || entIdx == 459 || entIdx == 460) { World.position[parent].y += 0.72f; } // prop_phys_barrel_chemical, prop_phys_barrel_radiation, prop_phys_barrel_toxic: Move up to account for CG mod (origin moved vs Unity version)
        else if (entIdx == 463) { World.position[parent].y += 0.64f; } // prop_phys_toolcart: Move up to account for CG mod (origin moved vs Unity version)
        else if (entIdx >= 472 && entIdx <= 476) { World.position[parent].y += 0.342f; } // se_crate1, se_crate2, se_crate3, se_crate4, se_crate5: Move up to account for CG mod (origin moved vs Unity version)
        else if (par->index == 746) { par->textureAnimating = true; par->texAnimClip = 2; par->texFrame = 0; } // weapon_grenadeenergmine_live
        else if (entIdx == 720) { /*u16 mist=*/AddInstance(648,World.position[parent]); }// ambient_mist
        else if (entIdx == 733) { /*u16 pipewater=*/AddInstance(649,World.position[parent]);/*ambient_pipewater_loop*/ /*u16 rain=*/AddInstance(653,(V3){World.position[parent].x,World.position[parent].y - 1.26f,World.position[parent].z});/*ambient_rain*/ }
        else if (entIdx == 305) { scpy_to_a_from_b(par->texAnimResourceFolder,"Telepad",TARGET_STRING_LENGTH); } /*chunk_teleporter*/
        TextureSequenceInit(parent,par->texAnimResourceFolder);
    }
    for (int i=0;i<=lightsIdx;++i) { if (lightsFromFile[i].intensity < 0.11f && lightsFromFile[i].maxIntensity < 0.11f){continue;} lightsFromFile[i].range = vclamp(lightsFromFile[i].range,0.32f,15.36f); AddLight(&lightsFromFile[i],&lanimsFromFile[i]); }
    if (curlevel == 1 || curlevel == 2 || curlevel == 5 || curlevel == 6 || curlevel == 7) { // Shield generators
        World.shd1=AddInstance(754,(V3){-51.30664f,-47.42f,56.42651f}); World.shd2=AddInstance(754,(V3){71.5f,-47.42f,-66.6f}); World.shd3=AddInstance(754,(V3){-51.306650f,-47.42f,-66.66652f}); World.shd4=AddInstance(754,(V3){71.78664f,-47.42f,56.42651f}); 
        World.rotation[World.shd1] = World.rotation[World.shd2] = World.rotation[World.shd3] = World.rotation[World.shd4] = QUAT_IDENTITY;
    } else World.shd1=World.shd2=World.shd3=World.shd4=U16_MAX;
    headmountedLanternLight = AddLightSimple(World.position[PLAYER1],(Color3){1.0f,1.0f,1.0f},11.52f,0.0f,LIGHTON);
}
#undef KEY_EQ
void func_forcebridge(u16 self); void CyberWallInitAfterLoad(u16 self); void FuncWallInitAfterLoad(u16); void LogicTimerInitBeforeLoad(u16); void ButtonSwitchInitAfterLoad(u16);
float DoorClamp01(float v) { if (v < 0.0f) return 0.0f; if (v > 1.0f) return 1.0f; return v; }
AnimationClip DoorGetClip(const Entity* e, u8 clip) { return modelAnimationClips[e->animationNum][clip]; }
void ChangeAnim(Entity* e, u8 clip);
void DoorSetClipFrame(u16 self, u8 clip, u16 frame) { ChangeAnim(&World.instances[self],clip); (void)frame; }
u16 DoorFrameFromProgress(AnimationClip c, float t) { if(c.frameEnd <= c.frameStart){return c.frameStart;} u16 span = c.frameEnd - c.frameStart; return (u16)(c.frameStart + (u16)(DoorClamp01(t) * (float)span)); }
void TeleportTouchInitAfterLoad(u16 self); void CyberItemInitBeforeLoad(u16 self);
void GravityLiftInitAfterLoad(u16 self) {
    World.instances[self].strength =                  UsableOrDef(World.instances[self].strength, 12.0f);
    World.instances[self].offStrengthFactor =         UsableOrDef(World.instances[self].offStrengthFactor, 0.3f);
    World.instances[self].distancePaddingToTopPoint = UsableOrDef(World.instances[self].distancePaddingToTopPoint, 0.32f);
    World.instances[self].topPoint = (V3){ 0.0f, World.position[self].y + (World.colliderSize[self].y * 0.5f), 0.0f };
}

void ComputeConvexMeshInertiaTensor(u16); void CyberMineInitBeforeLoad(u16);
void LoadLevelData(u8 curlevel) {
    World.curLev = curlevel; SetLevelPointers(curlevel); // Ensures writing to correct current level
    mset(World.instances + 3,0,(INSTANCE_COUNT - 3) * sizeof(Entity)); // Clear previous level slots. Claimed slots are fully initialized by AddInstance().
    World.instCount = 3; // 0 == NULL, 1 == Player1, 2 == Player2
    mset(World.lights,0,LIGHT_COUNT * sizeof(Light)); mset(World.lanims,0,LIGHT_COUNT * sizeof(LightAnimation)); World.loadedLights=0; mset(alreadyReadLightOnOnce,0,sizeof(alreadyReadLightOnOnce));
    mset(camViews,0,64 * sizeof(CamView)); camViewCount=0;
    char filename[20]; // Minimum size for 0 through 13.
    sFormat(filename, sizeof(filename), "./Data/level%d.txt", curlevel);
    FHandle fh; int fsize; void* fbuf = OS_OpenAndAllocateFileBufferReadonly(filename, &fh, &fsize); if (!fbuf) { OS_Exit(1); }
    mm_ptr = (const char*)fbuf; mm_end = mm_ptr + fsize; LoadLevelMod(curlevel); OS_Free(fbuf,(size_t)fsize);
    for (int i = 0; i < World.loadedLights; ++i) World.lightsNewPosition[i] = World.lights[i].pos;
    for (int i = PLAYER1; i < World.instCount; ++i) {
        i32 cellIdx = PosGetCellCoords(World.position[i].x, World.position[i].z);
        World.instances[i].cellIndex = cellIdx; World.instances[i].cellX = PosGetCellCoordX(World.position[i].x); World.instances[i].cellZ = PosGetCellCoordZ(World.position[i].z);
        u16 mi = World.instances[i].modelIndex; World.radius[i] = ((mi < mdlsCnt && modelBounds[mi] > 0.0f) ? modelBounds[mi] : 2.56f) * vmax(vmax(World.scale[i].x,World.scale[i].y),World.scale[i].z);
        World.instances[i].shadRadius = World.radius[i] * 2.00f;
        ComputeConvexMeshInertiaTensor(i);
        if (World.mass[i] < 0.001f && World.col[i] != COLTYPE_NONE && World.col[i] != COLTYPE_MSH && (World.instances[i].entflags & EF_RIGIDBODY)) { World.mass[i]=0.2f;/*At least something!*/ }
    }
    for (int i=PLAYER1;i<World.instCount;++i) {
        u16 constIndex = World.instances[i].index;
        if (i == PLAYER1 || IdxIsDynamicObject(constIndex) || (IdxIsNPC(constIndex) && constIndex < 443/*not cyber*/)) World.gravity[i] = 1.0f;
        else World.gravity[i] = 0.0f;
        if (IdxIsGeometry(constIndex)) World.layer[i] = L_Geometry;
        else if (IdxIsDoor(constIndex)) World.layer[i] = L_Door;
        else if (IdxIsUsableObject(constIndex)) {
            if (World.diffPuz == 3 && World.instances[i].index == 361 && random_range(0.0f,1.0f) < 0.33f) DeleteInstance(i); // 33% chance of not spawning logic probes on Puzzle difficulty of 3
            if (World.diffMis <= 1 && IdxIsAccessCard(World.instances[i].index)) DeleteInstance(i); // Remove access cards on Mission difficulty 1 or 0
            if (World.diffMis == 0 && World.instances[i].index == 313) DeleteInstance(i); // Remove audiologs on Mission difficulty 0
        } else if (IdxIsDoor(World.instances[i].index)) {
            if (World.instances[i].startOpen) World.instances[i].stayOpen = true;
            if (World.instances[i].useTimeDelay <= 0.0f) World.instances[i].useTimeDelay = 0.15f;
            if (World.instances[i].lockedMessageLingdex <= 0) World.instances[i].lockedMessageLingdex = 3;
            if (World.instances[i].SFXIndex < 0) World.instances[i].SFXIndex = 75;
            if (World.instances[i].doorOpen > DoorState_Opening) World.instances[i].doorOpen = World.instances[i].startOpen ? DoorState_Open : DoorState_Closed;
            World.instances[i].doorState = World.instances[i].doorOpen;
            if (World.instances[i].ajar) {
                AnimationClip c = DoorGetClip(&World.instances[i],ANIM_OPENING);
                DoorSetClipFrame(i,ANIM_OPENING,DoorFrameFromProgress(c,World.instances[i].ajarPercentage));
                World.instances[i].doorOpen = World.instances[i].doorState = DoorState_Opening;
            } else {
                switch (World.instances[i].doorOpen) {
                    case DoorState_Open:    DoorSetClipFrame(i,ANIM_IDLE_OPEN,DoorGetClip(&World.instances[i],ANIM_IDLE_OPEN).frameStart); break;
                    case DoorState_Opening: DoorSetClipFrame(i,ANIM_OPENING,DoorFrameFromProgress(DoorGetClip(&World.instances[i],ANIM_OPENING),0.0f/*TODO percent of anim*/)); break;
                    case DoorState_Closing: DoorSetClipFrame(i,ANIM_CLOSING,DoorFrameFromProgress(DoorGetClip(&World.instances[i],ANIM_CLOSING),0.0f/*TODO percent of anim*/)); break;
                    default:                DoorSetClipFrame(i,ANIM_IDLE_CLOSED,DoorGetClip(&World.instances[i],ANIM_IDLE_CLOSED).frameStart); break;
                }
            }
        } else if (IdxIsNPC(constIndex)) { World.layer[i] = L_NPC; /* TODO AIInit funcion */ }
        else if (IdxIsSearchable(constIndex)) {
            if (World.instances[i].generateContents) {
                int numRandomGeneratedItems = 0;
                for(int j=0;j<4;j++) {
                    if(numRandomGeneratedItems >= World.instances[i].maxRandomItems){break;} if(World.instances[i].randomItemDropChance[j] <= 0.0f){continue;}
                    u8 tempInt = random_range_u8(0,100);
                    if(((float)tempInt / 100.0f) <= World.instances[i].randomItemDropChance[j]){World.instances[i].contents[numRandomGeneratedItems] = World.instances[i].randomItem[j]; numRandomGeneratedItems++;}
                }
            }
        } else if (constIndex == 515) func_forcebridge(i); // func_forcebridge
        else if (constIndex == 517) FuncWallInitAfterLoad(i);
        else if (constIndex == 596) GravityLiftInitAfterLoad(i);
        else if (constIndex == 701) LogicTimerInitBeforeLoad(i);
        else if (constIndex == 556) TeleportTouchInitAfterLoad(i); // prop_cyberport
        else if (constIndex == 555) { } // prop_cyber_switch CyberSwitchInitAfterLoad(i);
        else if (constIndex == 21 || constIndex == 22) CyberWallInitAfterLoad(i); // chunk_cyberpanel or chunk_cyberpanel_slice45
        else if (IdxIsButtonSwitch(World.instances[i].index)) ButtonSwitchInitAfterLoad(i);
        else if (constIndex >= 448 && constIndex <= 457) CyberItemInitBeforeLoad(i);
        else if (constIndex == 480) CyberMineInitBeforeLoad(i);
        if (!sEmpty(World.instances[i].targetname) && (World.instances[i].ioflags & TARG_IOFLAGS_DISABLE_ON_AWAKE)) flag_set(&World.instances[i].entflags,EF_ACTIVE,false);
    }
    World.levelLoadedLights[curlevel] = World.loadedLights; mcpy(levelCamViews[curlevel],camViews,64 * sizeof(CamView)); mcpy(levelCamViewTextures[curlevel],camViewTextures,64 * sizeof(u32)); levelCamViewCount[curlevel] = camViewCount; World.levelInstCount[curlevel] = World.instCount; World.levelCurrentlyLoading = false; // Coppy the counts over
}

u8 GetCurrentLevelSecurity() { return (World.diffMis < 1 || Cheats.superoverride) ? 0u : World.levelSecurity[World.curLev]; }
void RenderLoading(i32 offset, const char * restrict text); void ResetLevelAudio(); void ResetLevelMusic(); void CullInit();
void LoadAllLevels() {
    double start_time = get_time();
    DebugRAM("start of LoadAllLevels");
    RenderLoading(100,"Loading level data...");
    World.levelCurrentlyLoading = true;
    entsFromFile = (Entity*)OS_Alloc((size_t)INSTANCE_COUNT * sizeof(Entity));
    posFromFile = (V3*)OS_Alloc((size_t)INSTANCE_COUNT * sizeof(V3));
    scaleFromFile = (V3*)OS_Alloc((size_t)INSTANCE_COUNT * sizeof(V3));
    rotationFromFile = (Quaternion*)OS_Alloc((size_t)INSTANCE_COUNT * sizeof(Quaternion));
    lightsFromFile = (Light*)OS_Alloc((size_t)LIGHT_COUNT * sizeof(Light));
    lanimsFromFile = (LightAnimation*)OS_Alloc((size_t)LIGHT_COUNT * sizeof(LightAnimation));
    for (u8 lev = 0; lev < World.numLevels; ++lev) LoadLevelData(lev);
    OS_Free(entsFromFile, (size_t)INSTANCE_COUNT * sizeof(Entity));
    OS_Free(posFromFile, (size_t)INSTANCE_COUNT * sizeof(V3));
    OS_Free(scaleFromFile, (size_t)INSTANCE_COUNT * sizeof(V3));
    OS_Free(rotationFromFile, (size_t)INSTANCE_COUNT * sizeof(Quaternion));
    OS_Free(lightsFromFile, (size_t)LIGHT_COUNT * sizeof(Light));
    OS_Free(lanimsFromFile, (size_t)LIGHT_COUNT * sizeof(LightAnimation));
    entsFromFile = NULL; posFromFile = NULL; scaleFromFile = NULL; rotationFromFile = NULL; lightsFromFile = NULL; lanimsFromFile = NULL;
    DualLog("Entity counts::0:%u|1:%u|2:%u|3:%u|4:%u|5:%u|6:%u|7:%u|8:%u|9:%u|10:%u|11:%u|12:%u|13:%u\n Light counts::0:%u|1:%u|2:%u|3:%u|4:%u|5:%u|6:%u|7:%u|8:%u|9:%u|10:%u|11:%u|12:%u|13:%u\nLoad all levels... took %f secs\n",
            World.levelInstCount[0],World.levelInstCount[1],World.levelInstCount[2],World.levelInstCount[3],World.levelInstCount[4],World.levelInstCount[5],World.levelInstCount[6],World.levelInstCount[7],World.levelInstCount[8],World.levelInstCount[9],World.levelInstCount[10],World.levelInstCount[11],World.levelInstCount[12],World.levelInstCount[13],
            World.levelLoadedLights[0],World.levelLoadedLights[1],World.levelLoadedLights[2],World.levelLoadedLights[3],World.levelLoadedLights[4],World.levelLoadedLights[5],World.levelLoadedLights[6],World.levelLoadedLights[7],World.levelLoadedLights[8],World.levelLoadedLights[9],World.levelLoadedLights[10],World.levelLoadedLights[11],World.levelLoadedLights[12],World.levelLoadedLights[13],
            get_time() - start_time);
    DebugRAM("end of LoadAllLevels");
}

void LoadLevel(u8 curlevel, V3 pos) {
    DebugRAM("start of LoadLevel");
    World.levelCurrentlyLoading = true; World.paused = false; World.menuActive = false;
    RenderLoading(100,"Loading level...");
    if (World.currentLevel != curlevel) CopyPlayerState(World.currentLevel,curlevel);
    World.curLev = curlevel;
    SetLevelPointers(curlevel);
    mcpy(camViews,levelCamViews[curlevel],64 * sizeof(CamView));
    mcpy(camViewTextures,levelCamViewTextures[curlevel],64 * sizeof(u32));
    camViewCount = levelCamViewCount[curlevel];
    mset(alreadyReadLightOnOnce,0,sizeof(alreadyReadLightOnOnce));
    // world_from_mdl is now an alias for modelMatrices, no need to zero separately
    for (int i=0;i<World.loadedLights;++i) World.lightsNewPosition[i]=World.lights[i].pos;
    DualLog("Switched to Level %d\n",curlevel);
    ResetLevelAudio(); ResetLevelMusic();
    RenderLoading(110,"Loading cull system..."); CullInit(); // Must be after level!
    glUseProgram(voxelUpdateSP); glUniform2f(0,World.voxMinCtrX[World.curLev],World.voxMinCtrZ[World.curLev]); glUniform1f(1,World.farPlane[World.curLev] * World.farPlane[World.curLev]); glUniform1ui(2,World.loadedLights);
                                 glUniform2f(3,World.worldMin_x[World.curLev],World.worldMin_z[World.curLev]); glUniform1ui(4,SHADOW_MAP_SIZE); glUniform1ui(6,(u32)MAX_LIGHTS_PER_VOXEL); glUniform1ui(7,SHADOW_MAP_SIZE*SHADOW_MAP_SIZE);
    RenderLoading(120,"Loading voxel lighting data...");
    for (u16 i = 0; i < World.loadedLights; i++) { World.lightsNewPosition[i] = World.lights[i].pos; }
    mset(shadowmapIndirectionList,MAX_SHADOWMAPS + 1,World.loadedLights * sizeof(u32)); // Set to invalid values for all
    World.levelCurrentlyLoading = false;
    World.position[PLAYER1]=pos;
    DebugRAM("end of LoadLevel");
}
// Save Game System
#pragma pack(push, 1)
typedef struct { u32 magicNumber; u32 version; u32 uncompressedSize; u32 compressedSize; char savename[48]; } SaveHeader;
#pragma pack(pop)
void SaveGame(u8 slot, const char* savename) {
    if(slot > 7){return;} char path[]="./Data/sav0.bin"; path[10]='0' + slot; FHandle fd=OS_OpenWriteonly(path); if(fd == (FHandle)-1){return;}
    size_t sz=sizeof(GlobalContext); size_t maxCompSize=GetMaxCompressedSize(sz); u8* b=(u8*)OS_Alloc(maxCompSize); size_t finalCompSize=VoidSquasher((const u8*)&World,sz,b,maxCompSize);
    if (finalCompSize > 0) {
        SaveHeader header = {.magicNumber=0x56415343/*'CSAV'*/, .version=2, .uncompressedSize=(u32)sz, .compressedSize=(u32)finalCompSize};
        if (savename) { int i=0;   while(savename[i] != '\0' && i < 47){header.savename[i]=savename[i]; i++;}   header.savename[i]='\0'; }
        World.justSavedTimeStamp = get_time(); OS_Write(fd,&header,sizeof(SaveHeader),path); OS_Write(fd,b,finalCompSize,path); CenterStatusPrint("Saved to Slot %d",slot);
    } else { DualLogError("Compression failed during SaveGame!\n"); }
    OS_Free(b,maxCompSize); OS_Close(fd);
}

void LoadGame(u8 slot) {
    if(slot > 7){return;} char path[]="./Data/sav0.bin"; path[10]='0' + slot; FHandle fd=OS_OpenReadonly(path); if(fd == (FHandle)-1){return;}
    SaveHeader header; if (OS_Read(fd,&header,sizeof(SaveHeader)) != sizeof(SaveHeader) || header.magicNumber != 0x56415343 || header.version != 2 || header.uncompressedSize != sizeof(GlobalContext)) { DualLogError("Corrupted save file header!\n"); OS_Close(fd); return; } 
    u8* b = (u8*)OS_Alloc(header.compressedSize);
    if (OS_Read(fd,b,header.compressedSize) == (long)header.compressedSize) {
        size_t result = BlowBubblesOfVoid(b,header.compressedSize,(u8*)&World,header.uncompressedSize); // Decompress straight into the World struct
        if (result == header.uncompressedSize) { SetLevelPointers(World.currentLevel); CenterStatusPrint("Loaded Game: %s", header.savename); } else { DualLogError("Decompression failed! Expected %u bytes, got %u\n", header.uncompressedSize, (u32)result); }
    }
    OS_Free(b,header.compressedSize); OS_Close(fd); for (int i=0;i<World.loadedLights;++i) { flag_set(&World.lights[i].lflags,LDIRTY,true); }
}
