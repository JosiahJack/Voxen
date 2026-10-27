// entity.c - Entity Definitions and Save Load System for levels and savegames
#include "common.h"
u16 teleportDestinations[MAX_LEVELS][8];
static bool teleportDestinationsInitialized;
u16 elevatorDestinations[MAX_LEVELS][MAX_ELEV_DESTS_PER_LEVEL];
u8 elevatorDestCounts[MAX_LEVELS];
Quaternion quat_from_axis_angle(V3,float);
void AddDoorPortal(u16,u16), TextureSequenceInit(u16,char*), AddCamView(V3,Quaternion,u8,u16,u16,float,float), GravityLiftSyncAllVisuals(void);
Entity* entsFromFile; V3 *posFromFile, *scaleFromFile; Quaternion *rotationFromFile; Light *lightsFromFile; LightAnimation *lanimsFromFile; static V3 *colCtrFromFile = NULL, *colSzFromFile = NULL;
u16 headmountedLanternLight;
bool alreadyReadLightOnOnce[LIGHT_COUNT] = {0};
u8 sensaroundCamViewCenter = 255, sensaroundCamViewLeft = 255, sensaroundCamViewRight = 255; // HUD sensaround cam view indices, assigned during level load
EPerms EDefs[MAX_ENTITIES] = { // EPerms struct order: modelIndex,colMeshIndex,texIndex,glowIndex,specIndex,normIndex,mass,dynFriction,statFriction,animationNum,col,colCtr,colSz
/*0 chunk_black*/[0]={178,0,0,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*1 chunk_blocker*/[1]={178,0,1230,MAXTEX,1230,160,0,0,0,MAXANIM,0,{0},{0}},
/*2 chunk_bridg1_1*/[2]={661,0,44,MAXTEX,MAXTEX,43,0,0,0,MAXANIM,0,{0},{0}},
/*3 chunk_bridg1_1flipx*/[3]={667,0,44,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*4 chunk_bridg1_2*/[4]={662,0,45,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*5 chunk_bridg1_3*/[5]={20,0,47,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*6 chunk_bridg1_3_slice45*/[6]={21,0,47,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*7 chunk_bridg1_3flipx*/[7]={663,0,47,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*8 chunk_bridg1_4*/[8]={22,0,48,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*9 chunk_bridg1_4_slice32*/[9]={23,0,48,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*10 chunk_bridg1_4_slice32flipx*/[10]={24,0,48,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*11 chunk_bridg1_5*/[11]={25,0,50,49,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*12 chunk_bridg2_2*/[12]={26,0,MAXANIM,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*13 chunk_bridg2_3*/[13]={27,0,56,54,MAXTEX,55,0,0,0,MAXANIM,0,{0},{0}},
/*14 chunk_bridg2_4*/[14]={28,0,57,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*15 chunk_bridg2_5*/[15]={29,0,59,MAXTEX,MAXTEX,58,0,0,0,MAXANIM,0,{0},{0}},
/*16 chunk_bridg2_6*/[16]={30,0,60,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*17 chunk_bridg2_7*/[17]={664,0,61,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*18 chunk_bridg2_8*/[18]={31,0,62,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*19 chunk_bridg2_9*/[19]={32,0,64,63,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*20 chunk_crate_impenetrable*/[20]={61,0,150,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*21 chunk_cyberpanel*/[21]={178,0,151,151,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*22 chunk_cyberpanel_slice45*/[22]={7488,0,152,152,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}}, /*model 7488 (grove1_2_slice45.obj): the generic slice45 card 6875 is the 180deg-rotated variant; 7488 is the same single triangle in the orientation the Unity blend specifies*/
/*23 chunk_eng1_1*/[23]={96,0,254,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*24 chunk_eng1_1d*/[24]={95,0,253,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*25 chunk_eng1_2*/[25]={98,0,256,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*26 chunk_eng1_2d*/[26]={97,0,255,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*27 chunk_eng1_3*/[27]={100,0,259,258,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*28 chunk_eng1_3d*/[28]={99,0,257,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*29 chunk_eng1_4*/[29]={101,0,260,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*30 chunk_eng1_5*/[30]={103,0,262,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*31 chunk_eng1_5_slice45lh*/[31]={104,0,262,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*32 chunk_eng1_5_slice45rh*/[32]={105,0,262,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*33 chunk_eng1_5d*/[33]={102,0,261,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*34 chunk_eng1_6*/[34]={107,0,266,265,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*35 chunk_eng1_6d*/[35]={106,0,264,263,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*36 chunk_eng1_7*/[36]={108,0,269,268,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*37 chunk_eng1_7d*/[37]={665,0,267,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*38 chunk_eng1_8*/[38]={109,0,271,270,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*39 chunk_eng1_9*/[39]={111,0,273,251,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*40 chunk_eng1_9d*/[40]={110,0,272,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*41 chunk_eng2_1*/[41]={113,0,276,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*42 chunk_eng2_1_slice45*/[42]={116,0,276,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*43 chunk_eng2_1_slice384high*/[43]={114,0,276,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*44 chunk_eng2_1_slice384highrh*/[44]={115,0,276,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*45 chunk_eng2_1d*/[45]={112,0,275,274,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*46 chunk_eng2_2*/[46]={117,0,279,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*47 chunk_eng2_2d*/[47]={666,0,277,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*48 chunk_eng2_3*/[48]={119,0,282,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*49 chunk_eng2_3d*/[49]={118,0,281,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*50 chunk_eng2_4*/[50]={178,0,283,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}}/*generic LOD card on purpose*/,
/*51 chunk_eng2_5*/[51]={120,0,285,MAXTEX,MAXTEX,284,0,0,0,MAXANIM,0,{0},{0}},
/*52 chunk_eng2_5_slice45*/[52]={121,0,285,MAXTEX,MAXTEX,284,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*53 chunk_eng2_6 (wall pump)*/[53]={0,0,141,142,MAXTEX,MAXTEX,0,0,0,21,0,{0},{0}}/*Looped static prop, anim overwrites, not a missing-model.*/,
/*54 chunk_exec1_1*/[54]={124,0,287,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*55 chunk_exec1_1d*/[55]={123,0,286,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*56 chunk_exec1_2*/[56]={126,0,291,290,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*57 chunk_exec1_2d*/[57]={125,0,289,288,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*58 chunk_exec2_1*/[58]={127,0,292,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*59 chunk_exec2_2*/[59]={129,0,295,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*60 chunk_exec2_2d*/[60]={128,0,294,293,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*61 chunk_exec2_3*/[61]={130,0,296,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*62 chunk_exec2_4*/[62]={131,0,297,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*63 chunk_exec2_4_slice45*/[63]={132,0,297,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*64 chunk_exec2_5*/[64]={133,0,298,MAXTEX,1257,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*65 chunk_exec2_6*/[65]={134,0,299,MAXTEX,1257,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*66 chunk_exec2_7*/[66]={133,0,300,MAXTEX,1257,MAXTEX,0,0,0,MAXANIM,0,{0},{0}}/*shares chunk_exec2_5's mesh*/,
/*67 chunk_exec3_1*/[67]={127,0,303,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}}/*shares chunk_exec2_1's mesh*/,
/*68 chunk_exec3_1d*/[68]={135,0,302,301,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*69 chunk_exec3_2*/[69]={129,0,304,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}}/*shares chunk_exec2_2's mesh*/,
/*70 chunk_exec3_4*/[70]={178,0,305,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}}/*generic LOD card on purpose*/,
/*71 chunk_exec4_1*/[71]={136,0,307,306,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*72 chunk_exec4_2*/[72]={137,0,308,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*73 chunk_exec4_3*/[73]={138,0,309,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*74 chunk_exec4_4*/[74]={139,0,311,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*75 chunk_exec4_5*/[75]={178,0,312,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}}/*generic LOD card on purpose*/,
/*76 chunk_exec4_6*/[76]={141,0,313,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*77 chunk_exec6_1*/[77]={142,0,315,314,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*78 chunk_exteriorpanel1*/[78]={131,0,1223,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*79 chunk_fan1*/[79]={0,0,96,192,MAXTEX,MAXTEX,0,0,0,22,0,{0},{0}}/*Looped static prop, anim overwrites, not a missing-model.*/,
/*80 chunk_flight1_1*/[80]={146,0,319,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*81 chunk_flight1_1b*/[81]={146,0,318,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}}/*shares chunk_flight1_1's mesh*/,
/*82 chunk_flight1_2*/[82]={147,0,320,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*83 chunk_flight1_2_slice45rh*/[83]={149,0,320,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*84 chunk_flight1_3*/[84]={150,0,321,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*85 chunk_flight1_4*/[85]={151,0,322,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*86 chunk_flight1_5*/[86]={147,0,323,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}}/*shares chunk_flight1_2's mesh*/,
/*87 chunk_flight1_5_slice45lh*/[87]={148,0,323,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*88 chunk_flight1_6*/[88]={152,0,325,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*89 chunk_flight2_1*/[89]={153,0,326,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*90 chunk_flight2_2*/[90]={154,0,327,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*91 chunk_flight2_2_slice45*/[91]={155,0,327,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*92 chunk_flight2_3*/[92]={156,0,328,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*93 chunk_grove1_1*/[93]={189,0,362,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}}, /*TODO: Add grove grass extrusion in-shader*/
/*94 chunk_grove1_2*/[94]={178,0,363,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}}/*generic LOD card on purpose*/,
/*95 chunk_grove1_2_slice45*/[95]={7488,0,363,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*96 chunk_grove1_3*/[96]={178,0,364,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}}/*generic LOD card on purpose*/,
/*97 chunk_grove1_4*/[97]={178,0,365,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}}/*generic LOD card on purpose*/,
/*98 chunk_grove1_5*/[98]={178,0,367,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}}/*generic LOD card on purpose*/,
/*99 chunk_grove1_6*/[99]={178,0,368,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}}/*generic LOD card on purpose*/,
/*100 chunk_grove1_7*/[100]={178,0,369,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}}/*generic LOD card on purpose*/,
/*101 chunk_grove2_1*/[101]={190,0,370,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*102 chunk_grove2_2*/[102]={191,0,371,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}}/*shares chunk_grove2_3's mesh*/,
/*103 chunk_grove2_3*/[103]={191,0,372,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*104 chunk_grove2_4*/[104]={341,0,374,373,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*105 chunk_grove2_5*/[105]={192,0,375,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*106 chunk_grove2_6*/[106]={192,0,376,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}}/*shares chunk_grove2_5's mesh*/,
/*107 chunk_grove2_7*/[107]={191,0,378,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}}/*TODO add the new shared grove2_7/grove2_8 mesh,shares chunk_grove2_3's mesh for now*/,
/*108 chunk_grove2_8*/[108]={191,0,379,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}}/*TODO add the new shared grove2_7/grove2_8 mesh,shares chunk_grove2_3's mesh for now*/,
/*109 chunk_grove2_9*/[109]={191,0,385,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}}/*shares chunk_grove2_3's mesh*/,
/*110 chunk_grove2_9b*/[110]={191,0,381,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}}/*shares chunk_grove2_3's mesh*/,
/*111 chunk_grove2_9c*/[111]={191,0,383,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}}/*shares chunk_grove2_3's mesh*/,
/*112 chunk_lift1*/[112]={213,0,1246,1247,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*113 chunk_maint1_1*/[113]={218,0,430,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*114 chunk_maint1_2*/[114]={220,0,432,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*115 chunk_maint1_2d*/[115]={219,0,431,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*116 chunk_maint1_3*/[116]={222,0,436,435,437,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*117 chunk_maint1_3b*/[117]={221,0,434,433,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*118 chunk_maint1_4*/[118]={224,0,441,440,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*119 chunk_maint1_4b*/[119]={223,0,439,438,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*120 chunk_maint1_5*/[120]={225,0,443,442,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*121 chunk_maint1_6*/[121]={226,0,445,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*122 chunk_maint1_7*/[122]={227,0,447,446,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*123 chunk_blockerflightbay*/[123]={178,U16_MAX,1230,MAXTEX,1242,160,0,0,0,MAXANIM,CT_BOX,{0,1.44f,0},{2.56f,0.32f,2.56f}},
/*124 chunk_maint1_9*/[124]={606,0,450,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}}/*shares chunk_stor1_7's mesh*/,
/*125 chunk_maint1_9d*/[125]={620,0,449,448,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}}/*shares chunk_stor1_7d's*/,
/*126 chunk_maint2_1*/[126]={178,0,455,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}}/*generic LOD card on purpose*/,
/*127 chunk_maint2_1b*/[127]={228,0,451,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*128 chunk_maint2_1d*/[128]={229,0,453,452,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*129 chunk_maint2_2*/[129]={230,0,457,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*130 chunk_maint2_3*/[130]={232,0,460,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*131 chunk_maint2_3d*/[131]={231,0,459,458,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*132 chunk_maint2_4*/[132]={233,0,464,463,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*133 chunk_maint2_4d*/[133]={235,0,462,461,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}}/*shares chunk_maint2_5's mesh*/,
/*134 chunk_maint2_5*/[134]={235,0,468,467,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*135 chunk_maint2_5d*/[135]={234,0,466,465,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*136 chunk_maint2_6*/[136]={236,0,472,471,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*137 chunk_maint2_6d*/[137]={6876,0,470,470,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*138 chunk_maint2_7*/[138]={238,0,476,475,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*139 chunk_maint2_7d*/[139]={237,0,474,473,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*140 chunk_maint2_8*/[140]={239,0,478,477,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*141 chunk_maint2_9*/[141]={240,0,480,479,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*142 chunk_maint2_9_slice45RH*/[142]={242,0,480,479,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*143 chunk_maint2_9_slice128_top*/[143]={241,0,480,479,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*144 chunk_maint3_1*/[144]={244,0,483,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*145 chunk_maint3_1_slice32_lh*/[145]={246,0,483,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*146 chunk_maint3_1_slice32_rh*/[146]={245,0,483,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*147 chunk_maint3_1_slice45*/[147]={247,0,483,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*148 chunk_maint3_1d*/[148]={243,0,482,481,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*149 chunk_med1_1*/[149]={249,0,486,MAXTEX,1256,1255,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*150 chunk_med1_1_half_top*/[150]={250,0,486,MAXTEX,1256,1255,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*151 chunk_med1_1_slice128high*/[151]={251,0,486,MAXTEX,1256,1255,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*152 chunk_med1_1_slice192RH*/[152]={252,0,486,MAXTEX,1256,1255,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*153 chunk_med1_1_slice256*/[153]={253,0,486,MAXTEX,1256,1255,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*154 chunk_med1_1d*/[154]={248,0,485,484,1236,1255,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*155 chunk_med1_2*/[155]={255,0,489,488,1256,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*156 chunk_med1_2d*/[156]={254,0,487,MAXTEX,1256,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*157 chunk_med1_3*/[157]={257,0,493,492,1256,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*158 chunk_med1_3d*/[158]={256,0,491,490,1256,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*159 chunk_med1_4*/[159]={258,0,494,MAXTEX,1256,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*160 chunk_med1_5*/[160]={669,0,495,MAXTEX,1256,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*161 chunk_med1_6*/[161]={259,0,496,MAXTEX,1256,509,0,0,0,MAXANIM,0,{0},{0}},
/*162 chunk_med1_7*/[162]={262,0,499,MAXTEX,1268,498,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*163 chunk_med1_7_slice14_64*/[163]={263,0,499,MAXTEX,1268,1254,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*164 chunk_med1_7_slice45_320lh*/[164]={264,0,499,MAXTEX,1268,1254,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*165 chunk_med1_7_slice45_320rh*/[165]={265,0,499,MAXTEX,1268,1254,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*166 chunk_med1_7_slice96high*/[166]={266,0,499,MAXTEX,1268,1254,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*167 chunk_med1_7d*/[167]={260,0,497,MAXTEX,1269,1270,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*168 chunk_med1_7d_slice128*/[168]={261,0,497,MAXTEX,1269,1270,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*169 chunk_med1_8*/[169]={268,0,503,MAXTEX,1242,502,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*170 chunk_med1_8d*/[170]={267,0,501,MAXTEX,1242,163,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*171 chunk_med1_9*/[171]={278,0,507,MAXTEX,1267,506,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*172*/[172]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*173*/[173]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*174 chunk_med1_9d*/[174]={269,0,505,MAXTEX,1267,504,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*175 unused*/[175]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*176 chunk_med1_9d_ofs112_90*/[176]={270,0,505,MAXTEX,1267,504,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*177 chunk_med1_9d_ofs144_90*/[177]={272,0,505,MAXTEX,1267,504,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*178 chunk_med2_1*/[178]={280,0,513,511,1254,512,0,0,0,MAXANIM,0,{0},{0}},
/*179 chunk_med2_1_slice32RH*/[179]={281,0,513,MAXTEX,1254,512,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*180 chunk_med2_1d*/[180]={279,0,510,508,1254,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*181 chunk_med2_2*/[181]={283,0,517,516,1242,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*182 chunk_med2_2_half_bottom*/[182]={284,0,517,516,1242,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*183 chunk_med2_2d*/[183]={282,0,515,514,1242,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*184 chunk_med2_3*/[184]={286,0,521,520,1242,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*185 chunk_med2_3d*/[185]={285,0,519,518,1242,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*186 chunk_med2_4*/[186]={287,0,523,522,524,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*187 chunk_med2_5*/[187]={288,0,527,526,539,MAXTEX,0,0,0,MAXANIM,CT_BOX,{0,1.44f,0},{2.56f,0.32f,2.56f}},
/*188 chunk_med2_6*/[188]={289,0,528,MAXTEX,1271,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*189 chunk_med2_7*/[189]={290,0,530,529,1245,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*190 chunk_med2_8*/[190]={291,0,531,MAXTEX,1242,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*191 chunk_med2_8_half_top*/[191]={292,0,531,MAXTEX,1242,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*192 chunk_med2_8_slice32RH*/[192]={293,0,531,MAXTEX,1242,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*193 chunk_med2_8_slice45*/[193]={294,0,531,MAXTEX,1242,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*194 chunk_med2_9*/[194]={296,0,535,534,1242,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*195 chunk_med2_9d*/[195]={295,0,533,532,1242,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*196 chunk_med3_1*/[196]={297,0,536,MAXTEX,1236,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*197 chunk_rad1_1*/[197]={501,0,660,659,1231,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*198 chunk_rad1_2*/[198]={501,0,662,661,1231,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*199 chunk_reac1_1*/[199]={502,0,664,MAXTEX,1243,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*200 chunk_reac1_1_slice45*/[200]={339,0,664,MAXTEX,1243,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}}/*uses the existing reac1_1_slice45 mesh*/,
/*201 chunk_reac1_2*/[201]={503,0,665,MAXTEX,1243,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*202 chunk_reac1_3*/[202]={504,0,666,MAXTEX,1243,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*203 chunk_reac1_4*/[203]={505,0,668,667,669,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*204 chunk_reac1_5*/[204]={506,0,671,670,1239,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*205 chunk_reac1_6*/[205]={507,0,673,672,1243,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*206 chunk_reac1_7*/[206]={342,0,676,675,1243,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*207 chunk_reac1_8*/[207]={508,0,678,677,1243,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*208 chunk_reac1_9*/[208]={509,0,680,680,1243,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*209 chunk_reac2_1*/[209]={512,0,682,MAXTEX,1235,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*210 chunk_reac2_1_slice45LH*/[210]={514,0,682,MAXTEX,1235,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*211 chunk_reac2_1_slice45LH_up*/[211]={515,0,682,MAXTEX,1235,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*212 chunk_reac2_1_slice45RH*/[212]={516,0,682,MAXTEX,1235,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*213 chunk_reac2_1_slice45RH_up*/[213]={517,0,682,MAXTEX,1235,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*214 chunk_reac2_1b*/[214]={510,0,681,MAXTEX,1235,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*215 chunk_reac2_1bmirror*/[215]={511,0,681,MAXTEX,1235,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*216 chunk_reac2_1mirror*/[216]={513,0,682,MAXTEX,1235,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*217 chunk_reac2_2*/[217]={518,0,684,683,1235,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*218 chunk_reac2_4*/[218]={519,0,685,MAXTEX,1235,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*219 chunk_reac2_4_slice128lower*/[219]={340,0,685,MAXTEX,1235,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*220 chunk_reac2_5*/[220]={520,0,687,686,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*221 chunk_reac2_6*/[221]={521,0,689,688,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*222 chunk_reac2_7*/[222]={522,0,691,690,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*223 chunk_reac2_8*/[223]={523,0,693,692,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*224 chunk_reac2_9*/[224]={524,0,694,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*225 chunk_reac3_1*/[225]={525,0,696,695,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*226 chunk_reac3_2*/[226]={526,0,697,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*227 chunk_reac3_3*/[227]={527,0,698,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*228 chunk_reac3_4*/[228]={528,0,699,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*229 chunk_reac3_5*/[229]={529,0,701,700,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*230 chunk_reac3_6*/[230]={530,0,703,702,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*231 chunk_reac3_7*/[231]={531,0,704,MAXTEX,705,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*232 chunk_reac4_1*/[232]={532,0,707,706,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*233 chunk_reac4_1_slice45lh*/[233]={533,0,707,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*234 chunk_reac4_2*/[234]={534,0,709,708,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*235 chunk_reac5_1*/[235]={535,0,711,710,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*236 chunk_reac5_2*/[236]={536,0,713,712,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*237 chunk_reac5_3*/[237]={537,0,715,714,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*238 chunk_reac6_1*/[238]={538,0,716,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*239 chunk_reac6_2*/[239]={539,0,717,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}}/*shares chunk_reac6_3's mesh*/,
/*240 chunk_reac6_3*/[240]={539,0,719,718,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*241 chunk_sci1_1*/[241]={540,0,722,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*242 chunk_sci1_1_slice45_toplh*/[242]={542,0,722,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*243 chunk_sci1_1_slice45_toprh*/[243]={543,0,722,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*244 chunk_sci1_1d*/[244]={541,0,721,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*245 chunk_sci1_2*/[245]={545,0,724,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*246 chunk_sci1_2_slice45lh*/[246]={546,0,724,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*247 chunk_sci1_2_slice45lh_up*/[247]={547,0,724,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*248 chunk_sci1_2_slice45rh*/[248]={548,0,724,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*249 chunk_sci1_2_slice45rh_up*/[249]={549,0,724,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*250 chunk_sci1_2d*/[250]={544,0,723,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*251 chunk_sci1_3*/[251]={550,0,726,725,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*252 chunk_sci1_4*/[252]={498,0,727,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*253 chunk_sci1_5*/[253]={551,0,728,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*254 chunk_sci1_6*/[254]={552,0,729,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*255 chunk_sci1_6_slice45*/[255]={553,0,729,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*256 chunk_sci1_7*/[256]={555,0,731,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*257 chunk_sci1_7d*/[257]={554,0,730,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*258 chunk_sci1_8*/[258]={557,0,734,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*259 chunk_sci1_8d*/[259]={556,0,733,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*260 chunk_sci1_9*/[260]={559,0,737,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*261 chunk_sci1_9d*/[261]={558,0,736,735,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*262 chunk_sci2_1*/[262]={561,0,739,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*263 chunk_sci2_1_slice45lh*/[263]={563,0,739,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*264 chunk_sci2_1_slice45rh*/[264]={562,0,739,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*265 chunk_sci2_1d*/[265]={560,0,738,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*266 chunk_sci2_2*/[266]={565,0,742,741,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*267 chunk_sci2_2d*/[267]={564,0,740,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*268 chunk_sci2_3*/[268]={566,0,744,743,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*269 chunk_sci2_4*/[269]={567,0,745,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*270 chunk_sci2_5*/[270]={569,0,747,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*271 chunk_sci2_5d*/[271]={568,0,746,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*272 chunk_sci3_1*/[272]={571,0,749,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*273 chunk_sci3_1d*/[273]={570,0,748,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*274 chunk_sci3_2*/[274]={572,0,750,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*275 chunk_sci3_3*/[275]={573,0,752,751,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*276 chunk_sci3_4*/[276]={574,0,754,753,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*277 chunk_sci3_5*/[277]={575,0,756,755,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*278 chunk_sci3_6*/[278]={576,0,758,757,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*279 chunk_screen*/[279]={5988,0,881,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*280 chunk_sec1_1*/[280]={178,0,787,MAXTEX,787,MAXTEX,0,0,0,MAXANIM,0,{0},{0}}/*generic LOD card on purpose*/,
/*281 chunk_sec1_1b*/[281]={178,0,785,MAXTEX,785,MAXTEX,0,0,0,MAXANIM,0,{0},{0}}/*generic LOD card on purpose*/,
/*282 chunk_sec1_1c*/[282]={577,0,786,MAXTEX,786,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*283 chunk_sec1_1c_slice45*/[283]={580,0,786,MAXTEX,786,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*284 chunk_sec1_1c_slice64highlh*/[284]={581,0,786,MAXTEX,786,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*285 chunk_sec1_1c_slice64highrh*/[285]={582,0,786,MAXTEX,786,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*286 unused*/[286]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*287 unused*/[287]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*288 chunk_sec1_2*/[288]={584,0,789,MAXTEX,1233,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*289 chunk_sec1_2b*/[289]={583,0,788,MAXTEX,1233,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*290 chunk_sec1_3*/[290]={585,0,790,MAXTEX,1233,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*291 chunk_sec1_3_slice45*/[291]={586,0,790,MAXTEX,1233,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*292 chunk_stor1_1*/[292]={597,0,824,823,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*293 chunk_stor1_2*/[293]={598,0,825,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}}/*shares chunk_stor1_3's mesh*/,
/*294 chunk_stor1_3*/[294]={598,0,826,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*295 chunk_stor1_4*/[295]={599,0,827,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*296 chunk_stor1_5*/[296]={600,0,828,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*297 chunk_stor1_6*/[297]={601,0,829,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*298 chunk_stor1_6_slice128_up_lh*/[298]={602,0,829,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*299 chunk_stor1_6_slice128_up_rh*/[299]={603,0,829,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*300 chunk_stor1_6_slice192lh*/[300]={604,0,829,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*301 chunk_stor1_6_slice192rh*/[301]={605,0,829,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*302 chunk_stor1_7*/[302]={606,0,833,MAXTEX,834,832,0,0,0,MAXANIM,0,{0},{0}},
/*303 chunk_stor1_7_slice45*/[303]={607,0,833,MAXTEX,834,832,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*304 chunk_stor1_7d*/[304]={620,0,831,830,834,832,0,0,0,MAXANIM,0,{0},{0}},
/*305 chunk_teleporter*/[305]={178,0,1166,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*306 chunk_white*/[306]={178,0,881,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*307 item_paper_wad*/[307]={487,0,1250,MAXTEX,MAXTEX,MAXTEX,0.06f,0.5f,0,MAXANIM,CT_SPH,{-0.001254f,-0.001190498f,0.006335999f},{0.0451f,0,0}},
/*308 item_warecasing*/[308]={637,0,1251,MAXTEX,MAXTEX,MAXTEX,0.8f,0.5f,0,MAXANIM,CT_BOX,{0,0,0.09397449f},{0.540964f,0.405398f,0.187949f}},
/*309 item_beaker*/[309]={14,682,36,MAXTEX,1242,MAXTEX,0.28f,0.1f,0.2f,MAXANIM,CT_CVX,{0},{0}},
/*310 item_beverage*/[310]={18,683,37,MAXTEX,MAXTEX,MAXTEX,0.12f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*311 item_skull*/[311]={593,70,816,MAXTEX,MAXTEX,MAXTEX,0.451f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*312 item_arm*/[312]={7,678,28,MAXTEX,MAXTEX,MAXTEX,1.0f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*313 item_audiolog*/[313]={11,679,52,80,MAXTEX,MAXTEX,0.2f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*314 weapon_grenadefrag*/[314]={182,73,348,MAXTEX,MAXTEX,MAXTEX,1.0f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*315 weapon_grenadeconc*/[315]={165,84,334,MAXTEX,MAXTEX,MAXTEX,1.3f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*316 weapon_grenadeemp*/[316]={168,85,338,MAXTEX,MAXTEX,MAXTEX,0.8f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*317 weapon_grenadeearth*/[317]={181,86,346,MAXTEX,MAXTEX,MAXTEX,1.5f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*318 weapon_grenademine*/[318]={184,87,353,MAXTEX,MAXTEX,MAXTEX,1.2f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*319 weapon_grenadenitro*/[319]={300,301,356,MAXTEX,MAXTEX,MAXTEX,1.2f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*320 weapon_grenadegas*/[320]={183,89,349,MAXTEX,MAXTEX,MAXTEX,0.9f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*321 item_patch_berserk*/[321]={488,491,590,MAXTEX,MAXTEX,MAXTEX,0.14f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*322 item_patch_detox*/[322]={488,491,591,MAXTEX,MAXTEX,MAXTEX,0.14f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*323 item_patch_genius*/[323]={488,491,592,MAXTEX,MAXTEX,MAXTEX,0.14f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*324 item_patch_medi*/[324]={488,491,600,MAXTEX,MAXTEX,MAXTEX,0.14f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*325 item_patch_reflex*/[325]={488,491,641,MAXTEX,MAXTEX,MAXTEX,0.14f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*326 item_patch_sight*/[326]={488,491,646,MAXTEX,MAXTEX,MAXTEX,0.14f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*327 item_patch_staminup*/[327]={488,491,647,MAXTEX,MAXTEX,MAXTEX,0.14f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*328 item_hw_system*/[328]={207,68,405,404,MAXTEX,MAXTEX,0.17f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*329 item_hw_navunit*/[329]={204,696,907,1259,MAXTEX,MAXTEX,0.1f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*330 item_hw_ereader*/[330]={200,692,397,MAXTEX,MAXTEX,MAXTEX,0.12f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*331 item_hw_sensaround*/[331]={205,697,402,MAXTEX,MAXTEX,MAXTEX,0.12f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*332 item_hw_targetid*/[332]={208,90,408,MAXTEX,MAXTEX,MAXTEX,0.08f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*333 item_hw_shield*/[333]={206,91,403,MAXTEX,MAXTEX,MAXTEX,0.14f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*334 item_hw_bio*/[334]={197,689,393,MAXTEX,MAXTEX,MAXTEX,0.1f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*335 item_hw_lantern*/[335]={203,695,401,400,MAXTEX,MAXTEX,0.11f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*336 item_hw_envirosuit*/[336]={199,691,396,MAXTEX,MAXTEX,MAXTEX,0.451f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*337 item_hw_booster*/[337]={198,690,395,MAXTEX,MAXTEX,MAXTEX,0.16f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*338 item_hw_jumpjets*/[338]={202,694,399,MAXTEX,MAXTEX,MAXTEX,0.32f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*339 item_hw_infrared*/[339]={201,693,398,MAXTEX,MAXTEX,MAXTEX,0.1f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*340 item_fireextinguisher*/[340]={144,684,317,MAXTEX,MAXTEX,MAXTEX,1.3f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*341 item_access_card_admin*/[341]={0,672,9,82,MAXTEX,MAXTEX,0.2f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*342 item_workerhelmet*/[342]={648,94,886,MAXTEX,MAXTEX,MAXTEX,0.8f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*343 weapon_mk3*/[343]={646,309,885,MAXTEX,MAXTEX,MAXTEX,0.75f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*344 weapon_blaster*/[344]={638,310,875,874,MAXTEX,MAXTEX,0.5f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*345 weapon_dartgun*/[345]={640,311,876,MAXTEX,MAXTEX,MAXTEX,0.3f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*346 weapon_flechette*/[346]={642,312,880,879,MAXTEX,MAXTEX,0.4f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*347 weapon_ionrifle*/[347]={643,313,883,MAXTEX,MAXTEX,MAXTEX,0.8f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*348 weapon_rapier*/[348]={653,314,891,890,MAXTEX,MAXTEX,0.3f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*349 weapon_pipe*/[349]={649,647,887,MAXTEX,1241,MAXTEX,0.85f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*350 weapon_magnum*/[350]={644,315,877,MAXTEX,1231,MAXTEX,0.6f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*351 weapon_magpulse*/[351]={645,316,884,MAXTEX,1231,MAXTEX,0.65f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*352 weapon_pistol*/[352]={650,317,878,MAXTEX,1231,MAXTEX,0.3f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*353 weapon_plasma*/[353]={651,318,888,MAXTEX,1240,MAXTEX,1.2f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*354 weapon_railgun*/[354]={652,319,889,MAXTEX,1231,MAXTEX,1.0f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*355 weapon_riotgun*/[355]={654,320,892,MAXTEX,1231,MAXTEX,0.55f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*356 weapon_skorpion*/[356]={655,321,893,MAXTEX,1231,MAXTEX,1.3f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*357 weapon_sparqbeam*/[357]={656,322,895,894,1231,MAXTEX,0.3f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*358 weapon_stungun*/[358]={657,323,896,MAXTEX,1231,MAXTEX,0.3f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*359 item_battery*/[359]={13,680,35,MAXTEX,MAXTEX,MAXTEX,0.3f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*360 item_battery_icad*/[360]={13,680,34,MAXTEX,MAXTEX,MAXTEX,0.35f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*361 item_logic_probe*/[361]={217,306,427,MAXTEX,MAXTEX,MAXTEX,0.15f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*362 item_healthkit*/[362]={196,688,391,MAXTEX,MAXTEX,MAXTEX,0.25f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*363 item_plastique*/[363]={492,308,599,MAXTEX,MAXTEX,MAXTEX,1.4f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*364 item_chipset_interfacedemod*/[364]={45,325,78,MAXTEX,MAXTEX,MAXTEX,0.3f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*365 item_flask*/[365]={145,685,36,MAXTEX,1242,MAXTEX,0.22f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*366 item_chipset_bitflag*/[366]={45,325,633,MAXTEX,MAXTEX,MAXTEX,0.3f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*367 item_ammo_rubber*/[367]={8,676,19,MAXTEX,MAXTEX,MAXTEX,0.25f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*368 item_isotopex22*/[368]={209,326,413,412,MAXTEX,MAXTEX,1.2f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*369 item_testtube*/[369]={622,612,36,MAXTEX,1242,MAXTEX,0.21f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*370 weapon_grenadefrag_live*/[370]={182,73,347,630,MAXTEX,MAXTEX,1.0f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*371 item_chipset_isolinear*/[371]={46,308,409,MAXTEX,MAXTEX,MAXTEX,0.26f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*372 weapon_grenadeconc_live*/[372]={165,84,334,MAXTEX,MAXTEX,MAXTEX,1.3f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*373 item_ammo_needle*/[373]={4,U16_MAX,15,MAXTEX,MAXTEX,MAXTEX,0.15f,0.5f,0,MAXANIM,CT_BOX,{-0.0004654949f,0.0004549972f,0.0244365f},{0.131339f,0.1442801f,0.04838703f}},
/*374 item_ammo_tranq*/[374]={4,U16_MAX,27,MAXTEX,MAXTEX,MAXTEX,0.15f,0.5f,0,MAXANIM,CT_BOX,{-0.0004654949f,0.0004549972f,0.0244365f},{0.131339f,0.1442801f,0.04838703f}},
/*375 item_ammo_standard*/[375]={5,U16_MAX,25,MAXTEX,MAXTEX,MAXTEX,0.2f,0.5f,0,MAXANIM,CT_BOX,{0.0001984993f,0.0f,0.02172501f},{0.1209471f,0.2176701f,0.04345007f}},
/*376 item_ammo_teflon*/[376]={5,U16_MAX,26,MAXTEX,MAXTEX,MAXTEX,0.2f,0.5f,0,MAXANIM,CT_BOX,{0.0001984993f,0.0f,0.02172501f},{0.1209471f,0.2176701f,0.04345007f}},
/*377 item_ammo_hollow*/[377]={5,U16_MAX,11,MAXTEX,MAXTEX,MAXTEX,0.2f,0.5f,0,MAXANIM,CT_BOX,{0.0002185023f,0.0f,0.02122951f},{0.1423431f,0.2127061f,0.04245907f}},
/*378 item_ammo_slug*/[378]={3,U16_MAX,23,22,MAXTEX,MAXTEX,0.2f,0.5f,0,MAXANIM,CT_BOX,{0.0002185023f,0.0f,0.02122951f},{0.1423431f,0.2127061f,0.04245907f}},
/*379 item_ammo_magnesium*/[379]={1,673,14,MAXTEX,MAXTEX,MAXTEX,0.35f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*380 item_ammo_penetrator*/[380]={1,673,16,MAXTEX,MAXTEX,MAXTEX,0.35f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*381 item_ammo_hornet*/[381]={1,673,12,MAXTEX,MAXTEX,MAXTEX,0.35f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*382 item_ammo_splinter*/[382]={1,673,24,MAXTEX,MAXTEX,MAXTEX,0.35f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*383 item_ammo_rail*/[383]={6,675,17,MAXTEX,MAXTEX,MAXTEX,0.40f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*384 item_ammo_slag*/[384]={1,673,21,MAXTEX,MAXTEX,MAXTEX,0.35f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*385 item_ammo_slaglarge*/[385]={10,677,20,MAXTEX,MAXTEX,MAXTEX,0.40f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*386 item_ammo_magcart*/[386]={2,674,13,MAXTEX,MAXTEX,MAXTEX,0.35f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*387 weapon_grenadeemp_live*/[387]={168,85,337,627,MAXTEX,MAXTEX,0.8f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*388 item_access_card_std*/[388]={0,672,79,867,MAXTEX,MAXTEX,0.2f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*389 weapon_grenadeearth_live*/[389]={181,86,345,628,MAXTEX,MAXTEX,1.5f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*390 item_access_card_group1*/[390]={0,672,7,159,MAXTEX,MAXTEX,0.2f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*391 item_access_card_science*/[391]={0,672,2,343,MAXTEX,MAXTEX,0.2f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*392 item_access_card_eng*/[392]={0,672,3,81,MAXTEX,MAXTEX,0.2f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*393 item_access_card_groupB*/[393]={0,672,7,159,MAXTEX,MAXTEX,0.2f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*394 item_access_card_security*/[394]={0,672,10,344,MAXTEX,MAXTEX,0.2f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*395 item_access_card_per5diego*/[395]={0,672,8,341,MAXTEX,MAXTEX,0.2f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*396 item_access_card_medi*/[396]={0,672,1,161,MAXTEX,MAXTEX,0.2f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*397 item_access_card_group3*/[397]={0,672,7,159,MAXTEX,MAXTEX,0.2f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*398 item_access_card_purple*/[398]={0,672,5,342,MAXTEX,MAXTEX,0.2f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*399 item_head_male*/[399]={194,194,389,MAXTEX,MAXTEX,MAXTEX,1.29f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*400 item_head_female*/[400]={193,686,388,MAXTEX,MAXTEX,MAXTEX,1.30f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*401 item_severedhead*/[401]={590,327,801,MAXTEX,MAXTEX,MAXTEX,1.28f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*402 weapon_grenademine_live*/[402]={184,87,351,352,MAXTEX,MAXTEX,1.2f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*403 weapon_grenadenitro_live*/[403]={185,88,354,355,MAXTEX,MAXTEX,1.2f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*404 weapon_grenadegas_live*/[404]={183,89,349,MAXTEX,MAXTEX,MAXTEX,0.9f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*405 to 416 unused*/[405]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*406*/[406]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*407*/[407]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*408*/[408]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*409*/[409]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*410*/[410]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*411*/[411]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*412*/[412]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*413*/[413]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*414*/[414]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*415*/[415]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*416*/[416]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*417 item_access_card_perdarcy*/[417]={0,672,8,341,MAXTEX,MAXTEX,0.2f,0,0,MAXANIM,CT_CVX,{0},{0}},
/*418 unused*/[418]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*419 npc_autobomb*/[419]={299,328,542,541,MAXTEX,MAXTEX,1.0f,0.15f,1.0f,MAXANIM,CT_CVX,{0},{0}},
/*420 npc_cyborg_assassin*/[420]={306,0,545,544,MAXTEX,MAXTEX,1.5f,0.15f,1.0f,24,CT_CAP,{0,0.96f,0},{0.48f,2.0f,0}},
/*421 npc_avian_mutant*/[421]={328,0,568,MAXTEX,MAXTEX,MAXTEX,2.0f,0.15f,1.0f,35,CT_CAP,{0,0.64f,0},{0.64f,1.60f,0}},
/*422 npc_exec_bot*/[422]={316,0,555,MAXTEX,MAXTEX,MAXTEX,2.2f,0.15f,1.0f,29,CT_CAP,{0,0.96f,0},{0.48f,2.025f,0}},
/*423 npc_cyborg_drone*/[423]={312,0,547,MAXTEX,MAXTEX,MAXTEX,1.5f,0.15f,1.0f,3,CT_CAP,{0},{0.36f,2.00f,0}},
/*424 npc_cortex_reaver*/[424]={300,0,543,MAXTEX,MAXTEX,MAXTEX,5.0f,0.15f,1.0f,23,CT_CAP,{0,1.28f,0},{1.28f,2.5f,0}},
/*425 npc_cyborg_warrior*/[425]={315,0,554,MAXTEX,MAXTEX,MAXTEX,1.5f,0.15f,1.0f,28,CT_CAP,{0},{0.48f,2.00f,0}},
/*426 npc_cyborg_enforcer*/[426]={314,0,550,MAXTEX,MAXTEX,MAXTEX,1.5f,0.15f,1.0f,27,CT_CAP,{0,1.03f,0},{0.40f,2.08f,0}},
/*427 npc_cyborg_elite*/[427]={313,0,548,MAXTEX,MAXTEX,MAXTEX,3.5f,0.15f,1.0f,26,CT_CAP,{0,1.09f,0},{0.44f,2.20f,0}},
/*428 npc_cyborg_diego*/[428]={309,0,546,MAXTEX,MAXTEX,MAXTEX,2.0f,0.15f,1.0f,25,CT_CAP,{0,1.04f,0},{0.48f,2.12f,0}},
/*429 npc_sec1_bot*/[429]={333,0,573,MAXTEX,MAXTEX,MAXTEX,1.5f,0.15f,1.0f,38,CT_CAP,{0,0.76f,0},{0.76f,1.8f,0}},
/*430 npc_sec2_bot*/[430]={335,0,574,MAXTEX,MAXTEX,MAXTEX,4.51f,0.15f,1.0f,39,CT_CAP,{0,1.08f,0},{1.12f,2.40f,0}},
/*431 npc_maint_bot*/[431]={325,0,567,MAXTEX,MAXTEX,MAXTEX,1.5f,0.15f,1.0f,34,CT_SPH,{0,0.78f,0},{2.00f,0,0}},
/*432 npc_mutant_cyborg*/[432]={329,0,569,MAXTEX,MAXTEX,MAXTEX,3.0f,0.15f,1.0f,51,CT_CAP,{0,0.12f,0},{0.75f,2.30f,0}},
/*433 npc_hopper*/[433]={322,0,562,MAXTEX,MAXTEX,MAXTEX,1.0f,0.15f,1.0f,32,CT_CAP,{0,1.04f,-0.16f},{0.96f,2.38f,0}},
/*434 npc_humanoid_mutant*/[434]={323,0,563,MAXTEX,MAXTEX,MAXTEX,1.4f,0.15f,1.0f,2,CT_CAP,{0},{0.38f,2.00f,0}},
/*435 npc_invisomut*/[435]={324,329,565,MAXTEX,MAXTEX,MAXTEX,1.3f,0.15f,1.0f,33,CT_CVX,{0},{0}},
/*436 npc_virus_mutant*/[436]={330,0,576,MAXTEX,MAXTEX,MAXTEX,1.4f,0.15f,1.0f,41,CT_CAP,{0,0.95f,0.16f},{0.72f,1.90f,0}},
/*437 npc_servbot*/[437]={5153,54,575,MAXTEX,MAXTEX,MAXTEX,2.50f,0.15f,1.0f,40,CT_CVX,{0},{0}},
/*438 npc_flier_bot*/[438]={318,0,558,MAXTEX,MAXTEX,MAXTEX,1.75f,0.15f,1.0f,30,CT_SPH,{0,0.16f,0},{0.8f,0,0}},
/*439 npc_zerog_mutant*/[439]={395,0,1170,MAXTEX,MAXTEX,MAXTEX,1.30f,0.15f,1.0f,42,CT_SPH,{0},{1.6f,0,0}},
/*440 npc_gorilla_tiger_mutant*/[440]={320,330,560,MAXTEX,MAXTEX,MAXTEX,2.00f,0.15f,1.0f,31,CT_CVX,{0},{0}},
/*441 npc_repairbot*/[441]={331,331,572,MAXTEX,MAXTEX,MAXTEX,1.50f,0.15f,1.0f,37,CT_CVX,{0},{0}},
/*442 npc_plant_mutant*/[442]={330,0,570,MAXTEX,MAXTEX,MAXTEX,0.80f,0.15f,1.0f,36,CT_CAP,{0,0.72f,0},{0.6f,1.44f,0}},
/*443 npc_cyberdog*/[443]={302,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,1.50f,0.15f,1.0f,MAXANIM,CT_SPH,{0},{0.72f,0,0}},
/*444 npc_cyberguard*/[444]={303,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,2.00f,0.15f,1.0f,MAXANIM,CT_SPH,{0},{1.0f,0,0}},
/*445 npc_cyberram*/[445]={304,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,2.00f,0.15f,1.0f,MAXANIM,CT_SPH,{0},{1.44f,0,0}},
/*446 npc_cyber_reaver*/[446]={305,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,2.20f,0.15f,1.0f,MAXANIM,CT_SPH,{0},{0.72f,0,0}},
/*447 npc_cybershodan*/[447]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,4.51f,0.15f,1.0f,MAXANIM,CT_CAP,{0},{0.28f,2.0f,0}},
/*448 item_cyber_data*/[448]={6076,0,3434,3434,MAXTEX,MAXTEX,0,0,0,53,0,{0},{1.5f,0,0}},
/*449 item_cyber_decoy*/[449]={6076,0,3435,3435,MAXTEX,MAXTEX,0,0,0,53,0,{0},{1.5f,0,0}},
/*450 item_cyber_drill*/[450]={6076,0,3436,3436,MAXTEX,MAXTEX,0,0,0,53,0,{0},{1.5f,0,0}},
/*451 item_cyber_game*/[451]={6076,0,3437,3437,MAXTEX,MAXTEX,0,0,0,53,0,{0},{1.5f,0,0}},
/*452 item_cyber_integrity*/[452]={69,0,3438,3438,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{1.5f,0,0}},
/*453 item_cyber_keycard*/[453]={70,0,3439,3439,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{1.5f,0,0}},
/*454 item_cyber_pulser*/[454]={6615,0,3440,3440,MAXTEX,MAXTEX,0,0,0,57,0,{0},{1.5f,0,0}},
/*455 item_cyber_recall*/[455]={6615,0,3441,3441,MAXTEX,MAXTEX,0,0,0,57,0,{0},{1.5f,0,0}},
/*456 item_cyber_shield*/[456]={6076,0,3442,3442,MAXTEX,MAXTEX,0,0,0,53,0,{0},{1.5f,0,0}},
/*457 item_cyber_turbo*/[457]={6076,0,3443,3443,MAXTEX,MAXTEX,0,0,0,53,0,{0},{1.5f,0,0}},
/*458 prop_phys_barrel_chemical*/[458]={12,332,30,MAXTEX,MAXTEX,MAXTEX,1.5f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*459 prop_phys_barrel_radiation*/[459]={12,332,31,MAXTEX,MAXTEX,MAXTEX,1.5f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*460 prop_phys_barrel_toxic*/[460]={12,332,33,MAXTEX,MAXTEX,MAXTEX,1.5f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*461 prop_phys_cart*/[461]={40,333,416,MAXTEX,MAXTEX,MAXTEX,2.5f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*462 prop_phys_pot*/[462]={494,334,640,MAXTEX,MAXTEX,MAXTEX,0.3f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*463 prop_phys_toolcart*/[463]={624,335,865,866,864,MAXTEX,20.0f,0.5f,0,MAXANIM,CT_CVX,{0},{0}},
/*464 se_briefcase*/[464]={34,0,66,65,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*465 se_corpse_blueshirt*/[465]={51,0,126,MAXTEX,127,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*466 se_corpse_brownshirt*/[466]={52,0,128,MAXTEX,129,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*467 se_corpse_eaten*/[467]={53,0,130,MAXTEX,131,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*468 se_corpse_labcoat*/[468]={55,0,132,MAXTEX,133,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*469 se_corpse_security*/[469]={56,0,136,MAXTEX,137,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*470 se_corpse_tan*/[470]={57,0,138,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*471 se_corpse_torso*/[471]={58,0,126,MAXTEX,127,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*472 se_crate1*/[472]={60,0,145,MAXTEX,MAXTEX,MAXTEX,0.75f,0,0,MAXANIM,CT_BOX,{0},{0.684186f,0.6841861f,0.6841861f}},
/*473 se_crate2*/[473]={60,0,143,MAXTEX,MAXTEX,MAXTEX,0.75f,0,0,MAXANIM,CT_BOX,{0},{0.684186f,0.6841861f,0.6841861f}},
/*474 se_crate3*/[474]={60,0,144,MAXTEX,MAXTEX,MAXTEX,0.75f,0,0,MAXANIM,CT_BOX,{0},{0.684186f,0.6841861f,0.6841861f}},
/*475 se_crate4*/[475]={60,0,146,MAXTEX,MAXTEX,MAXTEX,2.25f,0,0,MAXANIM,CT_BOX,{0},{0.684186f,0.6841861f,0.6841861f}},
/*476 se_crate5*/[476]={60,0,145,MAXTEX,MAXTEX,MAXTEX,2.25f,0,0,MAXANIM,CT_BOX,{0},{0.684186f,0.6841861f,0.6841861f}},
/*477 sec_camera*/[477]={589,0,73,72,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*478 sec_cpunode*/[478]={587,0,242,248,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*479 sec_cpunode_small*/[479]={588,0,107,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*480 weapon_cyber_mine*/[480]={71,0,1224,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*481 proj_enemshot2*/[481]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0.3f,0.5f,0,MAXANIM,0,{0},{0}},
/*482 proj_magpulse_shot*/[482]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0.3f,0.f,0,MAXANIM,CT_SPH,{0},{0.2f,0.2f,0.2f}},
/*483 proj_stungun_shot*/[483]={MAX_MDLS,0,835,MAXTEX,MAXTEX,MAXTEX,0.3f,0.5f,0,MAXANIM,CT_SPH,{0},{0.2f,0.2f,0.2f}},
/*484 proj_rail_shot*/[484]={652,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0.3f,0.5f,0,MAXANIM,CT_SPH,{0},{0.2f,0.2f,0.2f}},
/*485 proj_plasmarifle_shot*/[485]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0.3f,0.f,0,MAXANIM,CT_SPH,{0},{0.2f,0.2f,0.2f}},
/*486 proj_enemshot6*/[486]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0.3f,0.5f,0,MAXANIM,0,{0},{0}},
/*487 proj_enemshot5*/[487]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0.2f,0.5f,0,MAXANIM,0,{0},{0}},
/*488 proj_enemshot4*/[488]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0.3f,0.5f,0,MAXANIM,0,{0},{0}},
/*489 proj_throwingstar*/[489]={307,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0.3f,0.5f,0,MAXANIM,CT_SPH,{0},{0.2f,0.2f,0.2f}},
/*490 proj_magpulsenpc_shot*/[490]={645,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0.3f,0.5f,0,MAXANIM,CT_SPH,{0},{0.2f,0.2f,0.2f}},
/*491 proj_railnpc_shot*/[491]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0.3f,0.5f,0,MAXANIM,CT_SPH,{0},{0.2f,0.2f,0.2f}},
/*492 proj_cyberplayer_shot*/[492]={72,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0.3f,0.5f,0,MAXANIM,CT_SPH,{0},{0.2f,0.2f,0.2f}},
/*493 proj_cyberdog_shot*/[493]={63,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0.3f,0.5f,0,MAXANIM,CT_SPH,{0},{0.2f,0.2f,0.2f}},
/*494 proj_cyberreaver_shot*/[494]={64,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0.3f,0.5f,0,MAXANIM,CT_SPH,{0},{0.2f,0.2f,0.2f}},
/*495 proj_cyberice_shot*/[495]={68,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0.3f,0.5f,0,MAXANIM,CT_SPH,{0},{0.2f,0.2f,0.2f}},
/*496 doorA*/[496]={719,0,185,MAXTEX,MAXTEX,MAXTEX,0,0,0,1,CT_MSH,{0},{0}},
/*497 doorB*/[497]={0,0,189,188,MAXTEX,MAXTEX,0,0,0,0,CT_MSH,{0},{0}},
/*498 doorC*/[498]={0,0,184,MAXTEX,MAXTEX,MAXTEX,0,0,0,5,CT_MSH,{0},{0}},
/*499 doorD*/[499]={0,0,196,197,MAXTEX,MAXTEX,0,0,0,4,CT_MSH,{0},{0}},
/*500 doorE*/[500]={0,0,208,207,MAXTEX,MAXTEX,0,0,0,9,CT_MSH,{0},{0}},
/*501 doorF*/[501]={0,0,187,MAXTEX,MAXTEX,MAXTEX,0,0,0,10,CT_MSH,{0},{0}},
/*502 doorG*/[502]={0,0,193,194,MAXTEX,MAXTEX,0,0,0,11,CT_MSH,{0},{0}},
/*503 doorH*/[503]={0,0,190,MAXTEX,MAXTEX,MAXTEX,0,0,0,12,CT_MSH,{0},{0}},
/*504 doorI*/[504]={0,0,200,199,MAXTEX,MAXTEX,0,0,0,13,CT_MSH,{0},{0}},
/*505 doorJ*/[505]={0,0,215,MAXTEX,MAXTEX,MAXTEX,0,0,0,6,CT_MSH,{0},{0}},
/*506 doorK*/[506]={0,0,214,MAXTEX,MAXTEX,MAXTEX,0,0,0,7,CT_MSH,{0},{0}},
/*507 doorL*/[507]={0,0,191,MAXTEX,MAXTEX,MAXTEX,0,0,0,8,CT_MSH,{0},{0}},
/*508 door_elevator1*/[508]={0,0,202,MAXTEX,MAXTEX,MAXTEX,0,0,0,14,CT_MSH,{0},{0}},
/*509 door_elevator2*/[509]={0,0,203,MAXTEX,MAXTEX,MAXTEX,0,0,0,15,CT_MSH,{0},{0}},
/*510 door_elevator3*/[510]={0,0,204,MAXTEX,MAXTEX,MAXTEX,0,0,0,16,CT_MSH,{0},{0}},
/*511 door_elevator4*/[511]={0,0,206,205,MAXTEX,MAXTEX,0,0,0,17,CT_MSH,{0},{0}},
/*512 door_secret1*/[512]={0,0,210,MAXTEX,MAXTEX,MAXTEX,0,0,0,19,CT_MSH,{0},{0}},
/*513 door_secret2*/[513]={0,0,209,MAXTEX,MAXTEX,MAXTEX,0,0,0,18,CT_MSH,{0},{0}},
/*514 door_secret3*/[514]={94,0,211,MAXTEX,MAXTEX,MAXTEX,0,0,0,20,CT_MSH,{0},{0}},
/*515 func_forcebridge*/[515]={78,0,38,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_BOX,{0},{0}},
/*516 prop_lift2*/[516]={215,U16_MAX,155,154,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_BOX,{0.0f,0.0f,0.0f},{1.0f,1.0f,1.0f}},
/*517 func_wall*/[517]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,10.0f,0.6f,0.6f,MAXANIM,0,{0},{0}},
/*518 BulletHoleLarge*/[518]={5988,0,3324,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*519 BulletHoleScorchLarge*/[519]={5988,0,3326,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*520 BulletHoleScorchSmall*/[520]={5988,0,3326,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*521 BulletHoleSmall*/[521]={5988,0,3324,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*522 BulletHoleTiny*/[522]={5988,0,3325,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*523 BulletHoleTinySpread*/[523]={5988,0,3327,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*524 func_door_cyber*/[524]={178,U16_MAX,1224,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_BOX,{0.0f,1.31f,0.0f},{2.56f,0.06f,2.56f}},
/*525 prop_console01*/[525]={49,0,100,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*526 prop_console02*/[526]={50,0,100,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*527 prop_grate1_1*/[527]={186,0,359,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*528 prop_grate1_2*/[528]={187,0,360,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*529 prop_grate1_3*/[529]={188,0,361,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*530 se_cabinet*/[530]={39,0,70,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*531 se_thermos*/[531]={623,0,863,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*532 prop_beaker_holder*/[532]={15,0,36,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*533 prop_bed*/[533]={16,0,246,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*534 prop_bed_hospital*/[534]={608,0,759,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*535 prop_bed_neurosurgery*/[535]={17,0,18,MAXTEX,1238,29,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*536 prop_bonepile1*/[536]={19,0,815,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*537 prop_bridgewall1*/[537]={33,0,784,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*538 prop_broken_clock*/[538]={38,0,1117,1115,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*539 prop_brokengun*/[539]={639,0,878,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*540 prop_chair01*/[540]={41,0,195,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*541 prop_chair02*/[541]={42,0,3336,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*542 prop_chair03*/[542]={43,0,195,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*543 prop_chair04*/[543]={41,0,195,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*544 prop_chair05*/[544]={42,0,195,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*545 prop_chandelier*/[545]={496,0,644,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*546 prop_charge_station*/[546]={44,0,77,76,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*547 prop_clothes*/[547]={47,0,97,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*548 prop_computer*/[548]={48,0,195,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*549 prop_couch*/[549]={59,0,195,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*550 prop_couch2*/[550]={59,0,195,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*551 prop_cpuscreen*/[551]={5988,0,768,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*552 prop_cyber_datafrag*/[552]={6336,0,3444,3444,MAXTEX,MAXTEX,0,0,0,54,0,{0},{0}},
/*553 prop_cyber_decoy*/[553]={72,0,152,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_SPH,{0},{0.48f,0,0}},
/*554 prop_cyber_exit*/[554]={6416,0,3439,3439,MAXTEX,MAXTEX,0,0,0,55,0,{0},{0}},
/*555 prop_cyber_switch*/[555]={80,0,3439,3439,MAXTEX,MAXTEX,0,0,0,58,0,{0},{0}},
/*556 prop_cyberport*/[556]={62,0,117,116,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*557 prop_desk01*/[557]={74,0,125,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*558 prop_desk02*/[558]={75,0,124,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*559 prop_dexmissile*/[559]={76,0,164,162,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*560 prop_foliage_fernpoison*/[560]={160,0,331,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*561 prop_foliage_bush*/[561]={495,0,643,642,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*562 prop_foliage_fern*/[562]={160,0,333,330,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*563 prop_foliage_fernblueflower*/[563]={159,0,333,330,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*564 prop_foliage_pinetreem*/[564]={489,0,594,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*565 prop_foliage_poisonbush1*/[565]={493,0,638,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*566 prop_gear_large*/[566]={166,0,335,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*567 prop_gear_small*/[567]={167,0,336,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*568 prop_grass1*/[568]={161,0,329,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*569 prop_grass2*/[569]={MAX_MDLS,0,329,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*570 prop_grass3*/[570]={MAX_MDLS,0,329,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*571 prop_grass4*/[571]={MAX_MDLS,0,329,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*572 prop_grass5*/[572]={MAX_MDLS,0,329,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*573 prop_grate4*/[573]={1435,0,329,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*574 prop_healingbed*/[574]={195,0,1139,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*575 prop_lamp*/[575]={212,0,423,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*576 prop_light_emergsignal*/[576]={216,0,426,0,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*577 prop_microscope*/[577]={298,0,645,MAXTEX,1241,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*578 prop_pipe*/[578]={490,0,595,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*579 prop_puddle*/[579]={157,0,648,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*580 prop_puddle_grease*/[580]={157,0,650,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*581 prop_puddle_oil*/[581]={157,0,652,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*582 prop_shelves*/[582]={591,0,94,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*583 prop_skeleton*/[583]={592,0,815,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*584 prop_sleeping_cables*/[584]={595,0,71,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*585 prop_sparkingwire*/[585]={0,0,71,MAXTEX,MAXTEX,MAXTEX,0,0,0,46,0,{0},{0}}/*Looped static prop, anim overwrites, not a missing-model.*/,
/*586 prop_table*/[586]={619,0,92,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*587 prop_tv_on_a_post*/[587]={625,0,1228,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*588 prop_vendingmachines1*/[588]={627,0,870,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*589 prop_vendingmachines2*/[589]={614,0,871,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*590 prop_weapon_rack*/[590]={641,0,113,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*591 prop_xray*/[591]={660,0,153,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*592 text_decal*/[592]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*593 text_decalStopDSS1*/[593]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*594 trigger_counter*/[594]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*595 trigger_cyberpush*/[595]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*596 trigger_gravitylift*/[596]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*597 trigger_ladder*/[597]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*598 trigger_multiple*/[598]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*599 trigger_music*/[599]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*600 trigger_once*/[600]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*601 trigger_radiation*/[601]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*602 us_isotopepanel*/[602]={5565,0,616,MAXTEX,MAXTEX,MAXTEX,0,0,0,44,CT_MSH,{0.0f,0.0f,0.0f},{0.9f,0.9f,0.2f}},
/*603 us_paperlog*/[603]={486,0,580,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*604 us_puz_elevatorkeypad*/[604]={615,0,247,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*605 us_puz_elevatorkeypad2*/[605]={618,0,250,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*606 us_puz_elevatorkeypad3*/[606]={615,0,247,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*607 us_puz_elevatorkeypad4*/[607]={210,0,249,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*608 us_puz_keypad*/[608]={211,0,414,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*609 us_puz_panel_blue_grid*/[609]={5544,0,604,MAXTEX,MAXTEX,MAXTEX,0,0,0,43,CT_MSH,{0},{0}},
/*610 us_puz_panel_brown_grid*/[610]={5544,0,631,MAXTEX,MAXTEX,MAXTEX,0,0,0,43,CT_MSH,{0},{0}},
/*611 us_puz_panel_gray_grid*/[611]={5544,0,634,MAXTEX,MAXTEX,MAXTEX,0,0,0,43,CT_MSH,{0},{0}},
/*612 us_puz_panel_red_grid*/[612]={5544,0,625,MAXTEX,MAXTEX,MAXTEX,0,0,0,43,CT_MSH,{0},{0}},
/*613 us_puz_panel_teal_grid*/[613]={5544,0,601,MAXTEX,MAXTEX,MAXTEX,0,0,0,43,CT_MSH,{0},{0}},
/*614 us_relaypanel*/[614]={5596,0,617,MAXTEX,MAXTEX,MAXTEX,0,0,0,45,CT_MSH,{0.0f,0.0f,-0.04f},{0.64f,0.64f,0.18f}},
/*615 us_retinalscanner*/[615]={79,0,46,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*616 prop_vending1_1*/[616]={627,0,870,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*617 prop_vending1_2*/[617]={628,0,870,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*618 prop_vending1_3*/[618]={629,0,870,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*619 prop_vending2_1*/[619]={614,0,871,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*620 prop_vending2_2*/[620]={621,0,871,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}},
/*621 ambient_airhiss*/[621]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*622 ambient_clicker*/[622]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*623 ambient_compressor*/[623]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*624 ambient_dishwasher*/[624]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*625 ambient_drip_amb*/[625]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*626 ambient_fan*/[626]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*627 ambient_generator_gas*/[627]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*628 ambient_gurgle*/[628]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*629 ambient_icemaker*/[629]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*630 ambient_intake*/[630]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*631 ambient_lathe*/[631]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*632 ambient_lev3loop1*/[632]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*633 ambient_lev3loop2*/[633]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*634 ambient_lev3loop3*/[634]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*635 ambient_lev3loop4*/[635]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*636 ambient_liquid_bubble*/[636]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*637 ambient_liquid_lava2*/[637]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*638 ambient_looping*/[638]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*639 ambient_machgear_loop*/[639]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*640 ambient_machine_ambience*/[640]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*641 ambient_machine_go*/[641]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*642 ambient_machine_humamb7*/[642]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*643 ambient_machine_humlonoise*/[643]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*644 ambient_machine_loop1*/[644]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*645 ambient_machine_loop2*/[645]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*646 ambient_machinea1*/[646]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*647 ambient_machinevat_loop*/[647]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*648 ambient_mist*/[648]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*649 ambient_pipewater_loop*/[649]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*650 ambient_powerloom*/[650]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*651 ambient_pump*/[651]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*652 ambient_pump2*/[652]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*653 ambient_rain*/[653]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*654 ambient_steam_loop*/[654]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*655 ambient_washing_machine*/[655]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*656 decal_blood_die*/[656]={77,0,237,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*657 decal_blood_resist*/[657]={77,0,240,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*658 decal_blood_stayaway*/[658]={77,0,235,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*659 decal_blood_words2*/[659]={77,0,236,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*660 decal_bloodfonta*/[660]={178,0,118,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*661 decal_bloodfonte*/[661]={178,0,121,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*662 decal_bloodfontg*/[662]={178,0,122,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*663 decal_bloodfonth*/[663]={178,0,89,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*664 decal_bloodfontr*/[664]={178,0,139,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*665 decal_bloodfonty*/[665]={178,0,140,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*666 decal_bloodsplat2*/[666]={157,0,130,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*667 decal_logo_antenna*/[667]={77,0,182,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*668 decal_logo_armory*/[668]={77,0,178,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*669 decal_logo_biohazard*/[669]={77,0,180,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*670 decal_logo_bridge*/[670]={77,0,181,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*671 decal_logo_cyborg*/[671]={77,0,176,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*672 decal_logo_gears*/[672]={77,0,174,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*673 decal_logo_medical*/[673]={77,0,165,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*674 decal_logo_radhazard*/[674]={77,0,177,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*675 decal_logo_research*/[675]={77,0,175,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*676 decal_logo_security*/[676]={77,0,167,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*677 decal_painting1*/[677]={77,0,218,216,MAXTEX,217,0,0,0,MAXANIM,0,{0},{0}},
/*678 decal_painting2*/[678]={77,0,220,219,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*679 decal_painting3*/[679]={77,0,222,221,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*680 decal_posterbetterfuture*/[680]={77,0,226,MAXTEX,MAXTEX,225,0,0,0,MAXANIM,0,{0},{0}},
/*681 decal_postergenetics*/[681]={77,0,224,MAXTEX,MAXTEX,223,0,0,0,MAXANIM,0,{0},{0}},
/*682 decal_scorch1*/[682]={77,0,227,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*683 decal_scorch2*/[683]={77,0,228,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*684 decal_scorch3*/[684]={77,0,229,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*685 decal_scorch4*/[685]={77,0,230,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*686 decal_scorchtiny*/[686]={77,0,232,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*687 decal_blood_splat*/[687]={77,0,234,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*688 func_switch1*/[688]={609,0,837,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0.32f,0.04f,0.32f}},
/*689 func_switch2*/[689]={610,0,839,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{-0.0243553f,0.0f,0.000004883f},{0.0476318f,0.64f,0.64f}},
/*690 func_switch3*/[690]={611,0,842,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{-0.02285008f,0.000053061f,-0.000056993f},{0.02f,0.32f,0.32f}},
/*691 func_switch4*/[691]={5715,0,846,MAXTEX,MAXTEX,MAXTEX,0,0,0,47,0,{0.06f,0,0},{0.2f,0.64f,0.64f}},
/*692 func_switch5*/[692]={614,0,848,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0.64f,0.64f,0.08f}},
/*693 func_switch5broken*/[693]={613,0,847,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0.64f,0.64f,0.08f}},
/*694 func_switch7*/[694]={612,0,854,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{1.523325f,0,0},{0.2008026f,0.64f,0.64f}},
/*695 func_switch8*/[695]={616,0,856,855,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{-0.04f,0.0f,0.0001220703f},{0.08f,0.64f,0.64f}},
/*696 func_switchbroken1*/[696]={617,0,618,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*697 clip_npc*/[697]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_BOX,{1.005016f,0,0},{2.010033f,16.0f,16.0f}},
/*698 clip_objects*/[698]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_BOX,(V3){0},(V3){2.56f,2.56f,2.56f}},
/*699 logic_relay*/[699]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*700 logic_branch*/[700]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*701 logic_timer*/[701]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*702 logic_spawner*/[702]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*703 info_teleport_destination*/[703]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0.75f,2.0f,1.0f}},
/*704 prop_debris_panel*/[704]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*705 info_cyborgconversion*/[705]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*706 info_elev_destination*/[706]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*707 info_email*/[707]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*708 info_gameend*/[708]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*709 info_message*/[709]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*710 info_mission*/[710]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*711 info_note*/[711]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*712 info_playsound*/[712]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*713 info_ressurection_point*/[713]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*714 info_screenshake*/[714]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*715 info_spawnpoint*/[715]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*716 fx_reverbzone*/[716]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*717 ef_cyber_ice*/[717]={6516,U16_MAX,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,56,CT_SPH,{0.0f,0.004354001f,-0.014725f},{1.0f,0.0f,0.0f}},
/*718 ef_fragexplosion*/[718]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*719 ef_line_sparqbeam*/[719]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*720 ef_mist*/[720]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*721 ef_particle_bloodspurtsmall*/[721]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*722 ef_particle_bloodspurtsmallgreen*/[722]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*723 ef_particle_bloodspurtsmallyellow*/[723]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*724 ef_particle_bloodspurttiny*/[724]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*725 ef_particle_camerahit*/[725]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*726 ef_particle_darthit*/[726]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*727 ef_particle_sec2muzburst*/[727]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*728 ef_particle_sec2rotmuzburst*/[728]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*729 ef_particle_sparksmall*/[729]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*730 ef_particle_sparksmallblue*/[730]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*731 ef_particle_sparqhit*/[731]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*732 ef_sparkspits*/[732]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*733 ef_spraydrips*/[733]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*734 ef_steam*/[734]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*735 env_sparksmall*/[735]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*736 TargetIDInstance, unused now*/[736]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*737 prop_papers01*/[737]={484,0,580,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*738 prop_papers02*/[738]={485,0,580,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*739 ef_particle_blasterhit*/[739]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*740 ef_particle_ionhit*/[740]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*741 us_puz_panel_blue_wire*/[741]={0,0,604,MAXTEX,MAXTEX,MAXTEX,0,0,0,43,0,{0},{0}},
/*742 us_puz_panel_brown_wire*/[742]={0,0,631,MAXTEX,MAXTEX,MAXTEX,0,0,0,43,0,{0},{0}},
/*743 us_puz_panel_gray_wire*/[743]={0,0,634,MAXTEX,MAXTEX,MAXTEX,0,0,0,43,0,{0},{0}},
/*744 us_puz_panel_red_wire*/[744]={0,0,625,MAXTEX,MAXTEX,MAXTEX,0,0,0,43,0,{0},{0}},
/*745 us_puz_panel_teal_wire*/[745]={0,0,601,MAXTEX,MAXTEX,MAXTEX,0,0,0,43,0,{0},{0}},
/*746 weapon_grenadeenergmine_live*/[746]={7446,0,852,MAXTEX,MAXTEX,MAXTEX,0,0,0,52,0,{0},{0}},
/*747 decal_logo_storage*/[747]={77,0,169,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*748 light_animated*/[748]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*749 generic_transform*/[749]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*750 chunk_crate_impenetrable2*/[750]={61,0,147,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*751 chunk_crate_impenetrable3*/[751]={61,0,148,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*752 chunk_crate_impenetrable4*/[752]={61,0,149,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*753 npc_sec3_bot, made with my son one day to show him blender, used the sprite sheet that was cut from the game*/[753]={681,0,553,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*754 prop_shieldgenerator*/[754]={143,0,316,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*755 unused*/[755]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*756 ef_particle_leafburst*/[756]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*757 ef_particle_mutationburst*/[757]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*758 ef_particle_graytationburst*/[758]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*759 through 766 unused*/[759]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*760 chunk_maint2_2_slice45*/[760]={7488,0,457,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,CT_MSH,{0},{0}}, /*same Unity mesh as chunk_cyberpanel_slice45 (genericLOD3card_slice45.blend), so the same correctly-oriented triangle model 7488*/
/*761*/[761]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*762*/[762]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*763*/[763]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*764*/[764]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*765*/[765]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*766*/[766]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*767 player (mostly just so any index checks don't accidentally trigger against player by its index)*/[767]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*768 gib_npc_bot_cortex_reaver_gib0*/[768]={5990,5990,543,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*769 gib_npc_bot_cortex_reaver_gib1*/[769]={5991,5991,543,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*770 gib_npc_bot_cortex_reaver_gib10*/[770]={5992,5992,543,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*771 gib_npc_bot_cortex_reaver_gib2*/[771]={5993,5993,543,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*772 gib_npc_bot_cortex_reaver_gib3*/[772]={5994,5994,543,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*773 gib_npc_bot_cortex_reaver_gib4*/[773]={5995,5995,543,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*774 gib_npc_bot_cortex_reaver_gib5*/[774]={5996,5996,543,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*775 gib_npc_bot_cortex_reaver_gib6*/[775]={5997,5997,543,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*776 gib_npc_bot_cortex_reaver_gib7*/[776]={5998,5998,543,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*777 gib_npc_bot_cortex_reaver_gib8*/[777]={5999,5999,543,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*778 gib_npc_bot_cortex_reaver_gib9*/[778]={6000,6000,543,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*779 gib_npc_botexec_gib*/[779]={6001,6001,555,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*780 gib_npc_botexec_gib_head*/[780]={6002,6002,555,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*781 gib_npc_botexec_gib_leftarmupper*/[781]={6003,6003,555,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*782 gib_npc_botexec_gib_leftfoot*/[782]={6004,6004,555,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*783 gib_npc_botexec_gib_leftforearm*/[783]={6005,6005,555,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*784 gib_npc_botexec_gib_leftleg*/[784]={6006,6006,555,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*785 gib_npc_botexec_gib_rightarm*/[785]={6007,6007,555,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*786 gib_npc_botexec_gib_rightfemur*/[786]={6008,6008,555,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*787 gib_npc_botexec_gib_rightleg*/[787]={6009,6009,555,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*788 gib_npc_flierbot_gib_gun_lh_mesh*/[788]={6010,6010,558,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*789 gib_npc_flierbot_gib_gun_rh_mesh*/[789]={6011,6011,558,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*790 gib_npc_flierbot_gib_mesh_001*/[790]={6012,6012,558,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*791 gib_npc_flierbot_gib_nose_mesh*/[791]={6013,6013,558,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*792 gib_npc_flierbot_gibbody_mesh*/[792]={6014,6014,558,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*793 gib_npc_flierbot_gibwinglh_mesh*/[793]={6015,6015,558,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*794 gib_npc_maintenancebot_gib_bodyshell_lh*/[794]={6016,6016,567,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*795 gib_npc_maintenancebot_gib_bodyshell_rh*/[795]={6017,6017,567,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*796 gib_npc_maintenancebot_gib_leg*/[796]={6018,6018,567,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*797 gib_npc_maintenancebot_gib_leg_001*/[797]={6019,6019,567,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*798 gib_npc_maintenancebot_gib_leg_002*/[798]={6020,6020,567,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*799 gib_npc_maintenancebot_gib_leg_003*/[799]={6021,6021,567,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*800 gib_npc_maintenancebot_gib_npc_maintenancebot_pincers_lh*/[800]={6022,6022,567,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*801 gib_npc_maintenancebot_gib_npc_maintenancebot_pincers_rh*/[801]={6023,6023,567,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*802 gib_npc_maintenancebot_gib_npc_maintenancebot_pincers_stalk1*/[802]={6024,6024,567,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*803 gib_npc_maintenancebot_gib_npc_maintenancebot_pincers_stalk2*/[803]={6025,6025,567,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*804 gib_npc_repairbot_gib1*/[804]={6026,6026,572,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*805 gib_npc_repairbot_gib1_001*/[805]={6027,6027,572,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*806 gib_npc_repairbot_gib1_002*/[806]={6028,6028,572,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*807 gib_npc_repairbot_gib1_003*/[807]={6029,6029,572,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*808 gib_npc_repairbot_gib1_004*/[808]={6030,6030,572,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*809 gib_npc_repairbot_gib1_005*/[809]={6031,6031,572,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*810 gib_npc_repairbot_gibcenter*/[810]={6032,6032,572,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*811 gib_npc_repairbot_gibmandibleleft*/[811]={6033,6033,572,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*812 gib_npc_repairbot_gibmandiblelink*/[812]={6034,6034,572,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*813 gib_npc_repairbot_gibmandibleright*/[813]={6035,6035,572,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*814 gib_npc_sec1bot_gib1_mesh*/[814]={6036,6036,573,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*815 gib_npc_sec1bot_gib2_mesh*/[815]={6037,6037,573,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*816 gib_npc_sec1bot_gib3_mesh*/[816]={6038,6038,573,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*817 gib_npc_sec1bot_gib4_mesh*/[817]={6039,6039,573,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*818 gib_npc_sec1bot_gib5_mesh*/[818]={6040,6040,573,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*819 gib_npc_sec1bot_gib6_mesh*/[819]={6041,6041,573,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*820 gib_npc_sec1bot_gib7_mesh*/[820]={6042,6042,573,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*821 gib_npc_sec2bot_gibarmleft*/[821]={6043,6043,574,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*822 gib_npc_sec2bot_gibarmright*/[822]={6044,6044,574,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*823 gib_npc_sec2bot_gibfinger1*/[823]={6045,6045,574,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*824 gib_npc_sec2bot_gibfinger3*/[824]={6046,6046,574,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*825 gib_npc_sec2bot_gibgun*/[825]={6047,6047,574,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*826 gib_npc_sec2bot_gibmain*/[826]={6048,6048,574,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*827 gib_npc_sec2bot_gibmain_003*/[827]={6049,6049,574,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*828 gib_npc_sec2bot_gibmain_004*/[828]={6050,6050,574,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*829 gib_npc_sec2bot_gibmain_005*/[829]={6051,6051,574,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*830 gib_npc_sec2bot_gibmain_006*/[830]={6052,6052,574,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*831 gib_npc_servbotgib1*/[831]={6053,6053,575,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*832 gib_npc_servbotgib10*/[832]={6054,6054,575,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*833 gib_npc_servbotgib11*/[833]={6055,6055,575,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*834 gib_npc_servbotgib2*/[834]={6056,6056,575,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*835 gib_npc_servbotgib3*/[835]={6057,6057,575,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*836 gib_npc_servbotgib5*/[836]={6058,6058,575,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*837 gib_npc_servbotgib7*/[837]={6059,6059,575,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*838 gib_npc_servbotgib8*/[838]={6060,6060,575,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*839 gib_npc_servbotgib9*/[839]={6061,6061,575,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*840 gib_sec_cpunode_gib_base_000*/[840]={6062,6062,242,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*841 gib_sec_cpunode_gib_base_001*/[841]={6063,6063,242,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*842 gib_sec_cpunode_gib_base_002*/[842]={6064,6064,242,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*843 gib_sec_cpunode_gib_base_003*/[843]={6065,6065,242,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*844 gib_sec_cpunode_gib_base_004*/[844]={6066,6066,242,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*845 gib_sec_cpunode_gib_base_005*/[845]={6067,6067,242,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*846 gib_sec_cpunode_gib_cap1*/[846]={6068,6068,242,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*847 gib_sec_cpunode_gib_cap2*/[847]={6069,6069,242,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*848 gib_sec_cpunode_gib_cap_002*/[848]={6070,6070,242,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*849 gib_sec_cpunode_gib_panel2*/[849]={6071,6071,242,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*850 gib_sec_cpunode_gib_tower_001*/[850]={6072,6072,242,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*851 gib_sec_cpunode_gib_tower_002*/[851]={6073,6073,242,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*852 gib_sec_cpunode_gib_tower_003*/[852]={6074,6074,242,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*853 gib_sec_cpunode_gib_towerpanel1*/[853]={6075,6075,242,MAXTEX,MAXTEX,MAXTEX,0.5f,0.4f,0.5f,MAXANIM,CT_CVX,{0},{0}},
/*854 unused*/[854]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*855 unused*/[855]={MAX_MDLS,0,MAXTEX,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*856 prop_bridgewall2*/[856]={33,0,93,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*857 prop_bridgewall3*/[857]={33,0,85,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*858 prop_bridgewall4*/[858]={33,0,87,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*859 grovedome*/[859]={7487,0,316,MAXTEX,MAXTEX,160,0,0,0,MAXANIM,0,{0},{0}},
/*860 secret*/[860]={497,0,654,MAXTEX,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*861 shhh*/[861]={497,0,656,655,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}},
/*862 nothin to see here ;)*/[862]={497,0,658,657,MAXTEX,MAXTEX,0,0,0,MAXANIM,0,{0},{0}}
};

static bool cardChunk[307]={/*0*/1,/*1*/1,/*2*/1,/*3*/1,/*4*/1,/*5*/1,/*6*/0,/*7*/1,/*8*/1,/*9*/0,/*10*/0,/*11*/1,/*12*/1,/*13*/1,/*14*/1,/*15*/1,/*16*/1,/*17*/1,/*18*/1,/*19*/1,/*20*/0,/*21*/1,/*22*/0,/*23*/1,/*24*/1,/*25*/1,/*26*/1,/*27*/1,/*28*/1,/*29*/1,/*30*/1,/*31*/0,/*32*/0,/*33*/1,/*34*/1,/*35*/1,/*36*/1,/*37*/1,/*38*/1,/*39*/1,/*40*/1,/*41*/1,/*42*/0,/*43*/0,/*44*/0,/*45*/1,/*46*/1,/*47*/1,/*48*/1,/*49*/1,/*50*/1,/*51*/1,/*52*/0,/*53*/1,/*54*/1,
                            /*55*/1,/*56*/1,/*57*/1,/*58*/1,/*59*/1,/*60*/1,/*61*/1,/*62*/1,/*63*/0,/*64*/1,/*65*/1,/*66*/1,/*67*/1,/*68*/1,/*69*/1,/*70*/1,/*71*/1,/*72*/1,/*73*/1,/*74*/1,/*75*/1,/*76*/1,/*77*/1,/*78*/0,/*79*/1,/*80*/1,/*81*/1,/*82*/1,/*83*/0,/*84*/1,/*85*/1,/*86*/1,/*87*/0,/*88*/1,/*89*/1,/*90*/1,/*91*/0,/*92*/1,/*93*/1,/*94*/1,/*95*/0,/*96*/1,/*97*/1,/*98*/1,/*99*/1,/*100*/1,/*101*/1,/*102*/1,/*103*/1,/*104*/1,/*105*/1,/*106*/1,/*107*/1,
                            /*108*/1,/*109*/1,/*110*/1,/*111*/1,/*112*/1,/*113*/1,/*114*/1,/*115*/1,/*116*/1,/*117*/1,/*118*/1,/*119*/1,/*120*/1,/*121*/1,/*122*/1,/*123*/1,/*124*/1,/*125*/1,/*126*/1,/*127*/1,/*128*/1,/*129*/1,/*130*/1,/*131*/1,/*132*/1,/*133*/1,/*134*/1,/*135*/1,/*136*/1,/*137*/1,/*138*/1,/*139*/1,/*140*/1,/*141*/1,/*142*/0,/*143*/0,/*144*/1,/*145*/0,/*146*/0,/*147*/0,/*148*/1,/*149*/1,/*150*/0,/*151*/0,/*152*/0,/*153*/0,/*154*/1,/*155*/1,
                            /*156*/1,/*157*/1,/*158*/1,/*159*/1,/*160*/1,/*161*/1,/*162*/1,/*163*/0,/*164*/0,/*165*/0,/*166*/0,/*167*/1,/*168*/0,/*169*/1,/*170*/1,/*171*/1,/*172*/1,/*173*/1,/*174*/1,/*175*/1,/*176*/0,/*177*/0,/*178*/1,/*179*/0,/*180*/1,/*181*/1,/*182*/0,/*183*/1,/*184*/1,/*185*/1,/*186*/1,/*187*/1,/*188*/0,/*189*/1,/*190*/1,/*191*/0,/*192*/0,/*193*/0,/*194*/1,/*195*/1,/*196*/1,/*197*/1,/*198*/1,/*199*/1,/*200*/0,/*201*/1,/*202*/1,/*203*/1,
                            /*204*/1,/*205*/1,/*206*/1,/*207*/1,/*208*/1,/*209*/1,/*210*/0,/*211*/0,/*212*/0,/*213*/0,/*214*/1,/*215*/1,/*216*/1,/*217*/1,/*218*/1,/*219*/0,/*220*/1,/*221*/1,/*222*/1,/*223*/1,/*224*/1,/*225*/1,/*226*/1,/*227*/1,/*228*/1,/*229*/1,/*230*/1,/*231*/1,/*232*/1,/*233*/0,/*234*/1,/*235*/1,/*236*/1,/*237*/1,/*238*/1,/*239*/1,/*240*/1,/*241*/1,/*242*/0,/*243*/0,/*244*/1,/*245*/1,/*246*/0,/*247*/0,/*248*/0,/*249*/0,/*250*/1,/*251*/1,
                            /*252*/1,/*253*/1,/*254*/1,/*255*/0,/*256*/1,/*257*/1,/*258*/1,/*259*/1,/*260*/1,/*261*/1,/*262*/1,/*263*/0,/*264*/0,/*265*/1,/*266*/1,/*267*/1,/*268*/1,/*269*/1,/*270*/1,/*271*/1,/*272*/1,/*273*/1,/*274*/1,/*275*/1,/*276*/1,/*277*/1,/*278*/1,/*279*/1,/*280*/1,/*281*/1,/*282*/1,/*283*/0,/*284*/0,/*285*/0,/*286*/1,/*287*/1,/*288*/1,/*289*/1,/*290*/1,/*291*/0,/*292*/1,/*293*/1,/*294*/1,/*295*/1,/*296*/1,/*297*/1,/*298*/0,/*299*/0,
                            /*300*/0,/*301*/0,/*302*/1,/*303*/0,/*304*/1,/*305*/1,/*306*/1};
__attribute__((noinline)) i32 parse_numberi32(const char* str, const char* line, u32 ln){if(str==0||*str=='\0'){DualLogError("Invalid on line %d:%s\n",ln+1,line); return 0;} while(cEmpty((char)*str)){str++;} bool neg=false; if(*str=='+'){str++;}else if(*str=='-'){neg=true; str++;} i64 result=0; int n=0; while(*str>='0'&&*str<='9'&&n++<18){result=result*10L+(*str-'0'); str++;} while(*str>='0'&&*str<='9'){str++;} return (i32)(neg ? -result : result);}
__attribute__((noinline)) V3 GetLocalTransformedPos(Entity* originator, V3 offsetFromOriginator) { u16 idx=(u16)(originator - World.instances); V3 scaledOfs = mul_v3_v3_elementwise(offsetFromOriginator,World.scale[idx]); V3 rotatedOfs = quat_rot_v3(World.rotation[idx],scaledOfs); V3 result = V3_AplusB(World.position[idx],rotatedOfs); return result; }
__attribute__((noinline)) i16 parse_numberi16(const char* str, const char* line, u32 lineNum) {i32 v=parse_numberi32(str,line,lineNum); if(v < -32768 || v > 32767){DualLogError("%d outrange i16 on line %d:%s\n",v,lineNum+1,line); return 0;} return (i16)v;}
__attribute__((noinline)) i8 parse_numberi8(const char* str, const char* line, u32 lineNum) {i32 v=parse_numberi32(str,line,lineNum); if(v < -128 || v > 127){DualLogError("%d out range i8 on line %d:%s\n",v,lineNum+1,line); return 0;} return (i8)v;}
__attribute__((noinline)) float parse_float(const char* str, const char* line, u32 lineNum) {
    if (str == 0 || *str == '\0') { DualLogError("Blank on line %d:%s\n", lineNum+1, line); return 0.0f; } while (cEmpty(*str)){str++;} bool negative = false; if (*str == '-') { negative = true; str++; } else if (*str == '+') { str++; } double value = 0.0; bool has_digit = false; while (*str >= '0' && *str <= '9') { value = value * 10.0 + (*str - '0'); str++; has_digit = true; } // Integer part
    if (*str == '.') { str++; double frac = 0.0; double place = 0.1; while (*str >= '0' && *str <= '9') { frac += (*str - '0') * place; place *= 0.1; str++; has_digit = true; } value += frac; }/*Decimal part*/  if(!has_digit){return 0.0f;} if(negative){value = -value;} return (float)value;
}

__attribute__((noinline)) i32 AddLight(Light* lit, LightAnimation* la){ if(World.loadedLights>=LIGHT_COUNT){DualLogError("Too many lights,lev%d!\n",World.curLev); OS_Exit(1);} i32 i=World.loadedLights; World.loadedLights++; World.levelLoadedLights[World.currentLevel]++; mcpy(&World.lights[i],lit,sizeof(Light)); if (la && la->numIntervalSteps >= 1) { if (World.lanimCount < LANIM_MAX) { World.lanimLightIdx[World.lanimCount]=(u16)i; mcpy(&World.lanims[World.lanimCount],la,sizeof(LightAnimation)); World.lanimCount++; } else { DualLogError("Too many light animations,lev%d!\n",World.curLev); } } World.lightsNewPosition[i]=lit->pos; flag_set(&World.lights[i].lflags,LDIRTY,true); return i;}
__attribute__((noinline)) u16 AddLightSimple(V3 pos, Color3 c, float r, float in, u16 lf){Light l={.pos=pos,.col=c,.range=r,.intensity=in,.maxIntensity=in,.minIntensity=0.0f,.spotAng=0.0f,.spotDir=QUAT_IDENTITY,.lflags=lf}; LightAnimation a={0}; return AddLight(&l,&a);}
u16 IOInternName(const char*); static u16 lightTargetnameFile[LIGHT_COUNT];/*targetname per lightsFromFile slot, IO_NONE when anonymous.  A light is not an entity, so this is the only way a relay can address one.*/
__attribute__((noinline)) u16 AddOffsetLight(Entity* par, V3 offset, Color3 col, float range, float intensity) { return AddLightSimple(GetLocalTransformedPos(par,offset),col,range,intensity,LIGHT_AND_SHADOW_ON); }
void LoadFieldIntoLight(char* k, char* v, char* il, u32 ln, Light* lit, LightAnimation* lam, u16 lIdx) {
    char* br = StringFindFirstCharWithin(k,'['); if (br) { int i = parse_numberu32(br + 1,il,ln); if (i >= 0 && i < 32) { if (br - k == 13) { lam->intervalSteps[i] = parse_float(v,il,ln); }/*"intervalSteps[" bracket is at index 13*/ else if (br - k == 21) { lam->stepIsLerping[i] = parse_float(v,il,ln); }/*"intervalStepisLerping[" bracket is at index 21*/ } return; }
    static const struct { const char* key; u16 offset; u8 type; } map[] = {
        {"currentStep",    __builtin_offsetof(LightAnimation,currentStep),1},{"lerpValue",      __builtin_offsetof(LightAnimation,lerpValue),0},{"intervalSteps.Length",__builtin_offsetof(LightAnimation,numIntervalSteps),1},{"intervalStepisLerping.Length",__builtin_offsetof(LightAnimation, numLerpSteps),1},
        {"lP.x",__builtin_offsetof(Light,pos.x),0},{"lP.y",__builtin_offsetof(Light,pos.y),0},{"lP.z",__builtin_offsetof(Light,pos.z),0},{"lR.x",__builtin_offsetof(Light,spotDir.x),0},{"lR.y",__builtin_offsetof(Light,spotDir.y),0},{"lR.z",__builtin_offsetof(Light,spotDir.z),0},{"lR.w",__builtin_offsetof(Light,spotDir.w),0},
        {"range",__builtin_offsetof(Light,range),0},{"spotAngle",__builtin_offsetof(Light,spotAng),0},{"minIntensity",__builtin_offsetof(Light,minIntensity),0},{"maxIntensity",__builtin_offsetof(Light,maxIntensity),0},{"color.r",__builtin_offsetof(Light,col.r),0},{"color.g",__builtin_offsetof(Light,col.g),0},{"color.b",__builtin_offsetof(Light,col.b),0}
    };
    for (int i = 0; i < (int)(sizeof(map)/sizeof(map[0])); i++) {if(sEqual(k,map[i].key)/*0=float,1=u8*/){void* dest=(k[0]=='l' && (k[1]=='P' || k[1]=='R')) ? (void*)lit : (void*)lam; if (k[0]=='r' || k[0]=='s' || k[0]=='m' || k[0]=='c'){if(k[1]!='u')dest=(void*)lit;/*r,spot,max,color(not currentStep)*/} char* ptr=(char*)dest+map[i].offset; if(map[i].type == 0){*(float*)ptr=parse_float(v,il,ln);}else{*(u8*)ptr=parse_numberu8(v,il,ln);} return;}}
    if (sEqual(k,"intensity")) {lit->intensity = lit->maxIntensity = parse_float(v,il,ln) * 0.35f;}else if(sEqual(k,"type")){flag_set(&lit->lflags, (v[0] == 'S') ? LSPOT : LDIR, true);}else if (sEqual(k,"lightOn") && !alreadyReadLightOnOnce[lIdx]) { alreadyReadLightOnOnce[lIdx] = true; flag_set(&lit->lflags,LIGHTON,parse_bool(v,il,ln)); } else if (sEqual(k,"lerpOn")) {flag_set(&lit->lflags,LERPON,parse_bool(v,il,ln));}
    else if (sEqual(k,"targetname")) {lightTargetnameFile[lIdx] = IOInternName(v);}
}

u16 headmountedLanternLight;
V3 lanternPos;
#define CHGD(a,b) (vabs((a) - (b)) > 0.0001f)
void UpdateLight(u16 i, V3 pos, Color3 col, float range, float intensity, float max, float min, float spotAng, Quaternion spotDir, bool on, bool shad) {
    bool changed = ((!!(World.lights[i].lflags & SHADON) - shad) || (!!(World.lights[i].lflags & LIGHTON) -  on) || CHGD(World.lights[i].range,range) || CHGD(World.lights[i].pos.x,pos.x) || CHGD(World.lights[i].pos.y,pos.y) || CHGD(World.lights[i].pos.z,pos.z));
    World.lights[i].intensity = intensity;
    World.lights[i].minIntensity = min;
    World.lights[i].maxIntensity = max;
    World.lights[i].spotAng = spotAng;
    World.lights[i].spotDir = spotDir;
    World.lights[i].col = col;
    World.lights[i].pos = World.lightsNewPosition[i]=pos;
    World.lights[i].range = range;
    World.lights[i].lflags = (World.lights[i].lflags & ~(LIGHTON | SHADON | LDIRTY)) | ((World.lights[i].lflags & LDIRTY) | (changed << 4) | on | (shad << 1));
}
#undef CHGD

// Level Loading and Entity Management System
void InitNPC(u16 i); bool IsLiveGrenade(u16);
void CloseSearch(void);
void DeleteInstance(u16 i) {
    if (i <= PLAYER1 || i >= World.instCount) return;
    
    if (World.Sys_UI.tetheredSearchable==i) { CloseSearch(); }
    for (u16 j=INSTS_1ST_IDX;j<World.instCount;++j) { if(World.instances[j].enemy==i)World.instances[j].enemy=WORLD; }
    flag_set(&World.instances[i].entflags,EF_ACTIVE,false);
}

typedef struct { u16 entIdx; V3 scale; float health; BloodType blood; } PrefabConst;
static const PrefabConst prefabConsts[] = {
    {279, {0.00f,0.00f,0.00f},    5.0f, BloodType_None},        /*chunk_screen*/
    {424, {0.8f,0.80f,0.80f},     0.0f, BloodType_None},        /*npc_cortex_reaver*/
    {430, {0.9f,0.9f,0.90f},      0.0f, BloodType_None},        /*npc_sec2_bot*/
    {431, {0.40f,0.40f,0.40f},    0.0f, BloodType_None},        /*npc_maint_bot*/
    {433, {0.88f,0.88f,0.88f},    0.0f, BloodType_None},        /*npc_hopper*/
    {439, {0.40f,0.40f,0.40f},    0.0f, BloodType_None},        /*npc_zerog_mutant*/
    {441, {0.75f,0.75f,0.75f},    0.0f, BloodType_None},        /*npc_repairbot*/
    {444, {0.666f,0.666f,0.666f}, 0.0f, BloodType_None},        /*npc_cyberguard*/
    {445, {0.5f,0.5f,0.5f},       0.0f, BloodType_None},        /*npc_cyberram*/
    {446, {1.1f,1.1f,1.1f},       0.0f, BloodType_None},        /*npc_cyber_reaver*/
    {458, {0},               50.0f, BloodType_Robot},       /*prop_phys_barrel_chemical*/
    {459, {0},               50.0f, BloodType_Robot},       /*prop_phys_barrel_radiation*/
    {460, {0},               50.0f, BloodType_Robot},       /*prop_phys_barrel_toxic*/
    {464, {0},               14.0f, BloodType_GrayMutation},/*se_briefcase*/
    {465, {0},               50.0f, BloodType_Red},         /*se_corpse_blueshirt*/
    {466, {0},               50.0f, BloodType_Red},         /*se_corpse_brownshirt*/
    {467, {0},               50.0f, BloodType_Red},         /*se_corpse_eaten*/
    {468, {0},               50.0f, BloodType_Red},         /*se_corpse_labcoat*/
    {469, {0},               50.0f, BloodType_Red},         /*se_corpse_security*/
    {470, {0},               50.0f, BloodType_Red},         /*se_corpse_tan*/
    {471, {0},               50.0f, BloodType_Red},         /*se_corpse_torso*/
    {472, {0},               20.0f, BloodType_GrayMutation},/*se_crate1*/
    {473, {0},               20.0f, BloodType_GrayMutation},/*se_crate2*/
    {474, {0},               20.0f, BloodType_GrayMutation},/*se_crate3*/
    {475, {1.75f,1.75f,1.75f},   20.0f, BloodType_GrayMutation},/*se_crate4*/
    {476, {1.75f,1.75f,1.75f},   20.0f, BloodType_GrayMutation},/*se_crate5*/
    {477, {0},               10.0f, BloodType_Robot},       /*sec_camera*/
    {478, {0},               50.0f, BloodType_Robot},       /*sec_cpunode*/
    {479, {0},               35.0f, BloodType_Robot},       /*sec_cpunode_small*/
    {525, {0},              200.0f, BloodType_Robot},       /*prop_console01*/
    {526, {0},              200.0f, BloodType_Robot},       /*prop_console02*/
    {553, {0.5f,0.5f,0.5f}, 0.0f, BloodType_None},              /*prop_cyber_decoy*/
};

static const PrefabConst* PrefabConstFor(u16 entIdx) { for (u32 k = 0; k < sizeof(prefabConsts)/sizeof(prefabConsts[0]); ++k) { if (prefabConsts[k].entIdx == entIdx) { return &prefabConsts[k]; } } return 0; }

/*TODO Replace this offset correction factor from level data by baking it into the level*.txt data*/
typedef struct { u16 entIdx; V3 offset; } CgOffset;
static const CgOffset cgOffsets[] = {
    {309, {0,0.12f,0}},   {365, {0,0.12f,0}},   {369, {0,0.12f,0}},   /*item_beaker, item_flask, item_testtube*/
    {310, {0,0.0975f,0}}, /*item_beverage*/
    {314, {0,0.095f,0}},  /*weapon_grenadefrag*/
    {315, {0,0.065f,0}},  /*weapon_grenadeconc*/
    {316, {0,0.08f,0}},   /*weapon_grenadeemp*/
    {317, {0,0.12f,0}},   /*weapon_grenadeearth*/
    {318, {0,0.02f,0}},   /*weapon_grenadeemine*/
    {319, {0,0.09f,0}},   /*weapon_grenadegas*/
    {328, {0,0.04f,0}},   /*item_hw_system*/
    {332, {0,0.015f,0}},  /*item_hw_targetid*/
    {333, {0,0.028f,0}},  /*item_hw_shield*/
    {342, {0,0.12f,0}},   /*item_workerhelmet*/
    {343, {0,0.16f,0}},   /*weapon_blaster: up*/
    {346, {0,0,0.16f}},   /*weapon_blaster: over*/
    {348, {0,0,0.6f}},    /*weapon_blaster: over*/
    {350, {0,0.16f,0}},   /*weapon_magnum*/
    {352, {0,0.16f,0}},   /*weapon_pistol*/
    {358, {0,0.16f,0}},   /*weapon_stungun*/
    {361, {0,0.05f,0}},   /*item_logic_probe*/
    {363, {0,0.04f,0}},   /*item_plastique*/
    {364, {0,0.03f,0}},   {366, {0,0.03f,0}},   /*item_chipset_isolinear*/
    {368, {0,0.04f,0}},   /*item_isotopex22*/
    {371, {0,0.015f,0}},  /*item_chipset_isolinear*/
    {379, {0,0.025f,0}},  {380, {0,0.025f,0}},  {381, {0,0.025f,0}},  {382, {0,0.025f,0}},  {384, {0,0.025f,0}},  /*item_ammo_hornet, _splinter, _magnesium, _penetrator*/
    {399, {0,0.01f,0}},   /*item_head_male*/
    {458, {0,0.72f,0}},   {459, {0,0.72f,0}},   {460, {0,0.72f,0}},   /*prop_phys_barrel_chemical, _radiation, _toxic*/
    {463, {0,0.64f,0}},   /*prop_phys_toolcart*/
    {472, {0,0.342f,0}},  {473, {0,0.342f,0}},  {474, {0,0.342f,0}},  {475, {0,0.342f,0}},  {476, {0,0.342f,0}},  /*se_crate1..5*/
};

static const CgOffset* CgOffsetFor(u16 entIdx) { for (u32 k = 0; k < sizeof(cgOffsets)/sizeof(cgOffsets[0]); ++k) { if (cgOffsets[k].entIdx == entIdx) { return &cgOffsets[k]; } } return 0; }

__attribute__((noinline)) u16 AddInstance(u16 entIdx, V3 pos) {
    if (entIdx >= MAX_ENTITIES) { DualLogWarn("\nEntity index when loading non-light entity was %d, exceeds max defined entity count of %d, skipped\n",entIdx,MAX_ENTITIES); return WORLD; }
    if (World.instCount >= INSTANCE_COUNT) { DualLogWarn("\nToo many instances while adding entity %u, max instance count is %u, skipped\n", entIdx, INSTANCE_COUNT); return WORLD; }
    
    u16 i = World.instCount;
    mset(&World.instances[i],0,sizeof(Entity));
    World.instances[i].entflags=EF_ACTIVE;
    World.layer[i] = L_Default;World.instances[i].camView = 255; /*Sentinel value signaling that this mesh does not show a rendered camera view upon itself*/
    World.instances[i].modelIndex = World.instances[i].lodIndex = World.instances[i].colMeshIndex = MAX_MDLS; /*Invalidate all model fields*/
    World.velocity[i] = (V3){0.0f,0.0f,0.0f};
    World.angularVelocity[i] = (V3){0.0f,0.0f,0.0f};
    World.physSleep[i] = 0;
    World.invTnsrValid[i] = false;
    World.rotation[i] = (Quaternion){0.0f,0.0f,0.0f,1.0f};
    World.scale[i].x = World.scale[i].y = World.scale[i].z = World.mass[i] = 1.0f; /*Scale 1.0 for all axes, Quat Identity needs 1.0 for w*/
    World.dynamicFriction[i] = 0.5f;
    World.staticFriction[i] = 0.6f;
    for (u8 slot=0;slot<4;++slot) { World.instances[i].contents[slot] = World.instances[i].custIdx[slot] = -1; }
    for (u8 slot=0;slot<7;++slot) { World.instances[i].randomItem[slot] = World.instances[i].randomItemCustIdx[slot] = -1; }
    World.instances[i].index = entIdx;
    World.position[i] = pos;
    const PrefabConst* pc = PrefabConstFor(entIdx);
    if (pc && pc->scale.x) World.scale[i] = pc->scale;/*Default prefab scale (smaller sublist not in Edefs to save RAM).*/
    if (IdxIsNPC(entIdx)) {
        InitNPC(i); World.instances[i].npcNumber = ai_next_npc_number((u16)(entIdx - 419));
        if (npcTable[entIdx - 419].type == NPCType_Cyber) {
            if (World.instances[i].cyberHealth <= 0.0f) { World.instances[i].cyberHealth = npcTable[entIdx - 419].healthForCyberNPC; }
        } else if (World.instances[i].health <= 0.0f) { World.instances[i].health = npcTable[entIdx - 419].health;/*health/cyberHealth default when the record leaves them unset; a loaded -1 sentinel is kept off by the copy guard below*/}
    } else if (IsLiveGrenade(entIdx) && World.instances[i].health <= 0.0f) { World.instances[i].health = 15.0f; }
    else if (entIdx == 574 /*prop_healingbed*/ && World.instances[i].health <= 0.0f) World.instances[i].health = 9999999.0f;/*Only the level 7 one is not industructible*/
    World.instances[i].modelIndex = EDefs[entIdx].modelIndex; World.instances[i].colMeshIndex=EDefs[entIdx].colMeshIndex; World.instances[i].animationNum=EDefs[entIdx].animationNum; World.instances[i].texIndex=EDefs[entIdx].texIndex>=MAXTEX ? 0 : EDefs[entIdx].texIndex; World.instances[i].glowIndex=EDefs[entIdx].glowIndex>=MAXTEX ? 0 : EDefs[entIdx].glowIndex;
    World.instances[i].specIndex = EDefs[entIdx].specIndex >= MAXTEX ? 0 : EDefs[entIdx].specIndex; World.instances[i].normIndex = EDefs[entIdx].normIndex >= MAXTEX ? 0 : EDefs[entIdx].normIndex; flag_set(&World.instances[i].entflags,EF_RIGIDBODY,IdxIsDynamicObject(entIdx));
    if (entIdx == 585) { /*prop_sparkingwire: stagger the loop so co-located wires don't spark in lockstep*/
        AnimationClip* c = &mdlAnimClips[46][A_LOOP_ALL]; u32 span = (u32)(c->frameEnd - c->frameStart) + 1u, off = random_range_u32(0u, span - 1u);
        World.instances[i].clip = A_LOOP_ALL; World.instances[i].frame = (u16)(c->frameStart + off); World.instances[i].modelIndex = (u16)(c->frameStartModelIndex + off); /* Clip is a single loop; random start frame so visible pose and the frame-16 spark desync per instance. */
    }
    if (entIdx == 592 || entIdx == 593) { World.instances[i].modelIndex = U16_MAX; } // 3D text decals (no mesh)
    if (pc && pc->health > 0.0f && World.instances[i].health <= 0.0f) World.instances[i].health = pc->health;
    if (!IdxIsNPC(entIdx) && pc) World.instances[i].bloodType = pc->blood;/*Prefab constant, NOT per-instance state: HealthManager.bloodType is a serialized prefab field and no level*.txt record carries a bloodType key, so the parser has no branch for it and must never grow one.  InitNPC has already claimed the NPC types.*/
    if (entIdx == 717 /*ef_cyber_ice*/ && World.instances[i].cyberHealth <= 0.0f) World.instances[i].cyberHealth = 100.0f;
      if (entIdx == 477 /*sec_camera*/) { Entity* ce = &World.instances[i]; ce->camStartYAngle = 0.0f; ce->camEndYAngle = 180.0f; ce->camWaitTime = 0.8f; ce->camRotatePositive = true; ce->camRotateEnabled = false; }
    World.instances[i].questBitID = QB_None;
    if (entIdx == 594 /*trigger_counter*/) World.instances[i].dontReset = true;
    if (entIdx == 702 /*logic_spawner*/) { Entity* sm=&World.instances[i]; if (sm->numberToSpawn == 0) sm->numberToSpawn = 1; if (sm->maxDelayBetweenSpawns <= 0.0f) { sm->minDelayBetweenSpawns = 15.0f; sm->maxDelayBetweenSpawns = 45.0f; } if (sm->allSpawnedResetDelay <= 0.0f) sm->allSpawnedResetDelay = 120.0f; }
    if (World.levelCurrentlyLoading) { /*Tally the level's security objects so the security split and the system analyzer's node total have real denominators; guarded so runtime spawns never inflate them*/
        u8 sl=World.currentLevel; if (sl<MAX_LEVELS) { u8* dst=entIdx==477?&World.levelCameraCount[sl]:entIdx==478?&World.levelLargeNodeCount[sl]:entIdx==479?&World.levelSmallNodeCount[sl]:0; if (dst && *dst<255) (*dst)++; } }
    World.col[i]=EDefs[entIdx].col; World.colliderCenter[i]=EDefs[entIdx].colCtr; World.colliderSize[i]=EDefs[entIdx].colSz; World.mass[i]=EDefs[entIdx].mass > 0.0f ? EDefs[entIdx].mass : 1.0f; World.gravity[i]=IdxIsDynamicObject(World.instances[i].index) ? 1.0f : 0.0f; if (IdxIsButtonSwitch(entIdx)) { World.instances[i].lockedMessageLingdex = 193; World.instances[i].SFXLockedIndex = -1; }/*ButtonSwitch: every func_switch* prefab serializes SFXLockedIndex -1, i.e. no locked click. The success SFX comes from buttonSwitchSFX in citadel.c, the same prefab-constant-table shape doorSFXIndex uses in LoadLevelData.*/
    if (entIdx < 307 && cardChunk[entIdx]) { World.instances[i].lodIndex=178;/*LOD card index*/ World.col[i]=CT_BOX; World.colliderCenter[i].y=1.32f; World.colliderSize[i]=(V3){2.56f,0.08f,2.56f}; } World.instCount++; return i;/*levelInstCount[curlevel] is written once, at the end of LoadLevelData (entity.c:968), not per instance: its only readers are SetLevelPointers (right after a load), the LoadAllLevels entity-count log, and BuildTextDecalMeshes (which runs once, after every level is resident).  No reader observes a mid-load count, so the per-AddInstance store was redundant -- and it ran 8500 times a level.*/
}

static const char* mm_ptr; static const char* mm_end;
u16 TextMatIndexToTexIndex(u16 matIndex) {
    switch (matIndex) {
        case 88: case 92:                   return 1225; /*text_3dred, text_3dredStopD -> col3.png red*/
        case 89: case 95: case 98: case 99: return 1224; /*text_3dgold*, text_3dgoldStopD, text_3dgoldunlit* -> col2.png yellow*/
        case 90:                            return 1227; /*text_3dgreen -> col5.png green*/
        case 91: case 94: case 97:          return    0; /*text_3dblack* -> black.png*/
        case 96:                            return 1226; /*text_3dblueStopD -> col4.png blue*/
        default:                            return  881; /*white*/
    }
}
#define KEY_EQ(lit) (keyLen == (int)(sizeof(lit)-1) && sCompUpToLen(key, lit, sizeof(lit)-1) == 0)
static char* MmapGetLine(char* buf, int sz){if(mm_ptr>=mm_end){return NULL;} const char* start=mm_ptr; const char* p=start; while(p<mm_end&&*p!='\n'){++p;} int lineLen=(int)(p-start); if(p<mm_end&&*p =='\n'){mm_ptr=p+1;}else{mm_ptr=mm_end;}if(lineLen >= sz){lineLen=sz-1;} mcpy(buf,(void*)start,lineLen); while(lineLen>0&&(buf[lineLen-1]=='\r' || buf[lineLen - 1]=='\n')){--lineLen;} buf[lineLen]='\0'; return buf;}
static u16 fwParentSnap[MAX_LEVELS][INSTANCE_COUNT]; static bool fwSnapValid[MAX_LEVELS];/*per-level fwParentOf snapshot: fwParentOf is rewritten by every level load; SetLevelPointers restores the current level's*/
void SetLevelPointers(u8 lev) {
    if (lev >= MAX_LEVELS) return;
    if (lev != World.currentLevel && World.currentLevel < MAX_LEVELS) { World.levelInstCount[World.currentLevel] = World.instCount; World.levelLAnimCount[World.currentLevel] = World.lanimCount; }/*preserve dynamically summoned instances across temporary pointer switches (UseTargets/TargetIO level iteration)*/
    if (teleportDestinationsInitialized) mcpy(World.TeleportTouch_allTeleportTouches, teleportDestinations[lev], sizeof(World.TeleportTouch_allTeleportTouches));
    World.currentLevel=lev;
    World.instances=World.levelInstances[lev];
    World.position=World.levelPosition[lev];
    World.scale=World.levelScale[lev];
    World.velocity=World.levelVelocity[lev];
    World.angularVelocity=World.levelAngularVelocity[lev];
    World.colliderCenter=World.levelColliderCenter[lev];
    World.colliderSize=World.levelColliderSize[lev];
    World.col=World.levelCollider[lev];
    World.rotation=World.levelRotation[lev];
    World.layer=World.levelLayer[lev];
    World.mass=World.levelMass[lev];
    World.radius=World.levelRadius[lev];
    World.gravity          = World.levelGravity[lev];
    World.invInertiaTensor = World.levelInvInertiaTensor[lev];
    World.dynamicFriction  = World.levelDynamicFriction[lev];
    World.staticFriction   = World.levelStaticFriction[lev];
    World.invTnsrValid     = World.levelInvTnsrValid[lev];
    World.colliding        = World.levelColliding[lev];
    World.instCount        = World.levelInstCount[lev];
    World.lights           = World.levelLights[lev];
    World.particles        = &World.levelParticles[lev];
    if (fwSnapValid[lev]) mcpy(fwParentOf, fwParentSnap[lev], sizeof(fwParentOf));/*func_wall child links are per-level*/
    World.lanims=World.levelLAnims[lev]; World.lanimLightIdx=World.levelLAnimLightIdx[lev]; World.lanimCount=World.levelLAnimCount[lev]; World.lightTargetnames=World.levelLightTargetnames[lev]; World.lightsNewPosition=World.levelLightsNewPosition[lev]; World.loadedLights=World.levelLoadedLights[lev]; World.weaponVModelIndex=World.levelWepVModel[lev];
}

void CopyPlayerState(u8 srcLevel, u8 dstLevel) {
    if(srcLevel >= MAX_LEVELS || dstLevel >= MAX_LEVELS || srcLevel == dstLevel){return;} u16 s=PLAYER1; World.levelInstances[dstLevel][s]=World.levelInstances[srcLevel][s]; World.levelPosition[dstLevel][s]=World.levelPosition[srcLevel][s]; World.levelScale[dstLevel][s]=World.levelScale[srcLevel][s];
    World.levelVelocity[dstLevel][s]=World.levelVelocity[srcLevel][s]; World.levelAngularVelocity[dstLevel][s]=World.levelAngularVelocity[srcLevel][s]; World.levelColliderCenter[dstLevel][s]=World.levelColliderCenter[srcLevel][s]; World.levelColliderSize[dstLevel][s]=World.levelColliderSize[srcLevel][s];
    World.levelCollider[dstLevel][s]=World.levelCollider[srcLevel][s]; World.levelRotation[dstLevel][s]=World.levelRotation[srcLevel][s]; World.levelLayer[dstLevel][s]=World.levelLayer[srcLevel][s]; World.levelMass[dstLevel][s]=World.levelMass[srcLevel][s]; World.levelRadius[dstLevel][s]=World.levelRadius[srcLevel][s];
    World.levelGravity[dstLevel][s]=World.levelGravity[srcLevel][s]; mcpy(World.levelInvInertiaTensor[dstLevel][s],World.levelInvInertiaTensor[srcLevel][s],6 * sizeof(float));
    World.levelDynamicFriction[dstLevel][s]=World.levelDynamicFriction[srcLevel][s]; World.levelStaticFriction[dstLevel][s]=World.levelStaticFriction[srcLevel][s]; World.levelInvTnsrValid[dstLevel][s]=World.levelInvTnsrValid[srcLevel][s]; World.levelColliding[dstLevel][s]=World.levelColliding[srcLevel][s];
}

char lineSpace[LINE_LEN_MAX]; char ioNames[MAX_IO_NAMES][TARG_STRLEN]; u16 ioNameCount,fwParentOf[INSTANCE_COUNT]; static u16 fwSlotStart[INSTANCE_COUNT],fwSlotCount[INSTANCE_COUNT],fwPoolUsed,fwPoolPrefab[FW_POOL_MAX],fwCurChild,fwLastChunkSlot,lwPrefab[FW_MAX_CHILDREN];
static V3 fwBasePos[INSTANCE_COUNT],fwInfoLocal[INSTANCE_COUNT],fwBaseScale[INSTANCE_COUNT],fwPoolPos[FW_POOL_MAX],fwPoolScale[FW_POOL_MAX],lwPos[FW_MAX_CHILDREN],lwScale[FW_MAX_CHILDREN],fwInfoLocalTmp,fwContainerPos,fwContainerScale,fwInfoScaleDummy,*fwCurP,*fwCurS;
static Quaternion fwBaseRot[INSTANCE_COUNT],fwPoolRot[FW_POOL_MAX],lwRot[FW_MAX_CHILDREN],fwContainerRot,fwInfoRotDummy,*fwCurR; static bool fwLine,fwCollecting,fwPendingChild; static i32 fwStage; // transform block counter: 0 own, 1 func_wall container, 2 info_target, >=3 chunk children
static bool ipLine,ipSubRotSet; static i32 ipBlock; static V3 ipSubPos,ipSubScale; static Quaternion ipSubRot; static V3 *ipCurP; static Quaternion *ipCurR; static V3 *ipCurS;
static bool npcLine; static i32 npcBlock;/*NPC lines carry trailing gib/searchCollider/deathBurst sub-GO transform blocks after the NPC's own block-0 transform (same shape as the InteractablePanel problem above). npcBlock counts lP.x-delimited blocks 1-based, so block0 -- the NPC's own transform -- is npcBlock==1 and everything past it is dropped in the assignment chain below.*/
static bool npcRootRotSet[INSTANCE_COUNT];/*per-record: did block 0 carry explicit lR keys (vs the QUAT_IDENTITY default)? File-static, not World state, so saves are unaffected.*/
typedef struct { u16 ent; u8 anchor,align; float lineSp; char text[DECAL_INLINE_TEXT_LEN]; } DecalPend; // per level staging: `text:`/tA/tAl/tLs precede `lingdex`, resolved in copy loop
static DecalPend pendDecals[DECAL_INLINE_TEXT_PEND]; static u16 pendDecalCount;
static DecalPend* PendDecal(u16 ent) {
    if (pendDecalCount && pendDecals[pendDecalCount-1].ent == ent) return &pendDecals[pendDecalCount-1]; // current decal still open
    if (pendDecalCount >= DECAL_INLINE_TEXT_PEND) { DualLogError("decal staging full\n"); return NULL; }
    DecalPend* pd = &pendDecals[pendDecalCount++]; mset(pd,0,sizeof(*pd)); pd->ent=ent; pd->lineSp=1.0f; return pd;
}

u16 IOInternName(const char* name){if(!name || !*name){return IO_NONE;} for(u16 i=1;i<ioNameCount;++i){if(sEqual(ioNames[i],name)){return i;}} if(ioNameCount >= MAX_IO_NAMES){DualLogError("IO name full!\n"); return IO_NONE;} scpy_to_a_from_b(ioNames[ioNameCount],name,TARG_STRLEN); return ioNameCount++;}
static char texAnimFolders[256][TARG_STRLEN]; static u16 texAnimFolderCount = 1; /*idx 0 = "" (no folder)*/
u16 TexAnimFolderIntern(const char* name){if(!name || !*name){return 0;} for(u16 i=1;i<texAnimFolderCount;++i){if(sEqual(texAnimFolders[i],name)){return i;}} if(texAnimFolderCount >= 256){DualLogError("texAnim folder table full!\n"); return 0;} scpy_to_a_from_b(texAnimFolders[texAnimFolderCount],name,TARG_STRLEN); return texAnimFolderCount++;}
static void FWBeginBlock(i32 cnt){V3 *p,*s; Quaternion *r; if(fwCollecting&&fwLastChunkSlot<FW_MAX_CHILDREN){p=&lwPos[fwLastChunkSlot]; r=&lwRot[fwLastChunkSlot]; s=&lwScale[fwLastChunkSlot];}else{switch(fwStage++){case 1:p=&fwContainerPos; r=&fwContainerRot; s=&fwContainerScale; break; case 2:p=&fwInfoLocalTmp; r=&fwInfoRotDummy; s=&fwInfoScaleDummy; break; default:p=&posFromFile[cnt]; r=&rotationFromFile[cnt]; s=&scaleFromFile[cnt]; break;}} fwCurP=p; fwCurR=r; fwCurS=s;}
void LoadLevelMod(u8 lev) {
    u8 curlevel = vclamp(lev, 0, 13); World.levelCurrentlyLoading = true;/*precondition only, for AddInstance's level-security tally above -- NOT redundant bookkeeping. The tally must not see runtime spawns, and this flag is its only way to tell load-time from run-time. Nothing outside the loader reads it; the main loop gates level changes on the queuedLevelToLoad sentinel instead.*/
    if(curlevel == 0){AddCamView((V3){-14.16170f,-54.72389f,20.71170f},(Quaternion){0.03959f,0.41918f,-0.01830f,0.90686f},85u,256u,256u,0.84f,16.223f);/*ReacScreen1_Camera*/ AddCamView((V3){36.38030f,-57.61589f,-0.30230f},(Quaternion){0.12279f,0.69636f,-0.12279f,0.69636f},50u,256u,256u,2.0f,17.05f);/*ReacScreen1_Camera (1)*/ AddCamView((V3){-0.64170f,-54.17389f,-5.37830f},(Quaternion){0.17111f,0.16779f,-0.02959f,0.97041f},65u,256u,256u,0.513f,26.13f);/*ReacScreen1_Camera (2)*/}
    else if(curlevel == 1){AddCamView((V3){-19.2301f,-42.6604f,-49.7453f},(Quaternion){0.2375f,0.0008f,-0.0002f,0.9713f},75u,256u,256u,2.21f,11.5f);/*MedScreen14_Camera*/ AddCamView((V3){7.664583f,-44.88017f,-14.26742f},(Quaternion){0.0f,0.9999f,0.0129f,0.0f},60u,256u,256u,2.192f,20.6f);/*MedScreen15_Camera*/}
    else if(curlevel == 2) { AddCamView((V3){36.73389f,-24.66895f,9.47060f},(Quaternion){0.11769f,-0.23125f,0.02819f,0.96534f},75u,256u,256u,0.695f,13.73f);/*SciScreen6_Camera*/}
    else if(curlevel == 3) { AddCamView((V3){-12.01620f,-28.52930f,33.53500f},(Quaternion){0.54295f,0.45300f,0.54295f,-0.45300f},68u,256u,256u,0.868f,14.145f);/*MaintScreen_Camera*/}
    else if(curlevel == 4) { AddCamView((V3){-26.93000f,46.56557f,-31.18625f},(Quaternion){0.00000f,0.00000f,0.00000f,1.00000f},68u,256u,256u,0.82f,17.56f);/*StorageScreen_Camera*/}
    else if(curlevel == 5){AddCamView((V3){-30.72000f,8.96000f,-53.76000f},(Quaternion){0.00000f,0.70711f,0.70711f,0.00000f},68u,256u,256u,2.7f,24.8f);/*FlightScreen1_Camera*/}
    else if(curlevel == 6){AddCamView((V3){9.08000f,48.71000f,-47.32400f},(Quaternion){0.00000f,0.99054f,0.00000f,-0.13719f},90u,256u,256u,1.2f,11.52f);/*ExecScreen5_Camera*/ AddCamView((V3){-3.90720f,13.90800f,2.47230f},(Quaternion){-0.50000f,0.50000f,0.50000f,0.50000f},90u,256u,256u,0.06f,11.87f);/*ExecScreen6_Camera*/}
    else if(curlevel == 7){AddCamView((V3){106.87113f,-14.55570f,24.65725f},(Quaternion){0.00000f,0.00000f,0.00000f,1.00000f},68u,256u,256u,1.295f,26.01f);/*EngScreen3_Camera*/ AddCamView((V3){-57.06100f,34.70000f,1.19200f},(Quaternion){0.00000f,-0.65094f,0.00000f,0.75913f},68u,256u,256u,1.83f,19.9f);/*EngScreen4_Camera*/ AddCamView((V3){29.51000f,3.29000f,-9.93000f},(Quaternion){0,0,0,1.f},80u,256u,256u,0.77f,9.79f);/*EngScreen5_Camera*/}
    else if(curlevel == 8){AddCamView((V3){69.76093f,30.97230f,-0.84785f},(Quaternion){0.48739f,0.51230f,0.51230f,-0.48739f},85u,256u,256u,0.33f,10.05f);/*SecScreen1_Camera*/ AddCamView((V3){9.95000f,60.11100f,8.59600f},(Quaternion){0.14019f,0.00000f,0.00000f,0.99013f},68u,256u,256u,1.0f,10.44f);/*SecScreen1_Camera (1)*/ AddCamView((V3){-5.55300f,77.60500f,25.11700f},(Quaternion){0.09230f,0.70106f,-0.09230f,0.70106f},68u,256u,256u,1.2f,9.4f);/*SecScreen1_Camera (2)*/ AddCamView((V3){9.99800f,98.27100f,-14.68900f},(Quaternion){0.00000f,0.99255f,-0.12187f,0.00000f},68u,256u,256u,0.7f,12.7f);/*SecScreen1_Camera (3)*/ AddCamView((V3){36.88600f,69.74000f,-3.47000f},(Quaternion){-0.06937f,0.60309f,-0.05276f,-0.79289f},68u,256u,256u,1.34f,34.97f);/*SecScreen1_Camera (4)*/ AddCamView((V3){-24.66100f,82.29600f,2.08200f},(Quaternion){-0.00000f,0.70711f,0.00000f,-0.70711f},85u,256u,256u,0.99f,12.53f);/*SecScreen1_Camera (5)*/ AddCamView((V3){53.26900f,77.89000f,-8.30000f},(Quaternion){-0.12279f,0.69636f,-0.12279f,-0.69636f},71u,256u,256u,0.56f,16.38f);/*SecScreen1_Camera (6)*/}
    else if(curlevel == 9){AddCamView((V3){53.36000f,108.73700f,8.72300f},(Quaternion){-0.11027f,0.77231f,-0.13966f,-0.60982f},71u,256u,256u,0.5f,16.582f);/*BridgeScreen1_Camera*/}
    else if(curlevel == 10){AddCamView((V3){6.27000f,137.70000f,20.88000f},(Quaternion){0.05778f,-0.25920f,0.01554f,0.96397f},68u,256u,256u,1.3f,9.4f);/*GroveScreen1_Camera*/ AddCamView((V3){24.67000f,136.79999f,-26.02000f},(Quaternion){-0.00879f,-0.90735f,0.01901f,-0.41985f},68u,256u,256u,4.0f,13.6f);/*GroveScreen2_Camera*/}
    else if(curlevel == 12){AddCamView((V3){35.80700f,196.89301f,-4.98400f},(Quaternion){0.12369f,0.00000f,0.00000f,0.99232f},68u,256u,256u,0.7f,4.32f);/*GroveScreen3_Camera*/ AddCamView((V3){-9.23000f,199.00000f,2.98000f},(Quaternion){0.08830f,-0.69488f,0.08661f,0.70841f},68u,256u,256u,1.5f,8.04f);/*GroveScreen4_Camera*/}
    sensaroundCamViewCenter=(u8)camViewCount; AddCamView((V3){0.0f,0.0f,0.0f},QUAT_IDENTITY,100u,640u,240u,0.2f,1000.0f);/*Sensaround center rear view (always positioned/oriented per-frame in UpdateSensaroundCamViews)*/
    sensaroundCamViewLeft=(u8)camViewCount;   AddCamView((V3){0.0f,0.0f,0.0f},QUAT_IDENTITY,65u,320u,240u,0.2f,1000.0f);/*Sensaround left side view*/
    sensaroundCamViewRight=(u8)camViewCount;  AddCamView((V3){0.0f,0.0f,0.0f},QUAT_IDENTITY,65u,320u,240u,0.2f,1000.0f);/*Sensaround right side view*/
    mset(lineSpace,0,LINE_LEN_MAX * sizeof(char)); u32 lineNum = 0; i32 entCount = -1, lightsIdx = -1; char* line; pendDecalCount = 0; for (u16 i=0;i<LIGHT_COUNT;++i) { mset(&lightsFromFile[i],0,sizeof(Light)); mset(&lanimsFromFile[i],0,sizeof(LightAnimation)); lightsFromFile[i].range = 5.5f; lightsFromFile[i].col = (Color3){1.0f,1.0f,1.0f}; lightsFromFile[i].spotAng=0.0f; lightTargetnameFile[i]=IO_NONE; }
    while (MmapGetLine(lineSpace, LINE_LEN_MAX)) {
        lineNum++; line=lineSpace; char* firstColon=StringFindFirstCharWithin(line,':'); int firstKeyLen=firstColon ? (int)(firstColon - line) : 0; bool isLight=!(firstKeyLen == 10 && sCompUpToLen(line,"constIndex",10) == 0); Entity* inst=NULL; Light* lit=NULL; LightAnimation* lanim=NULL;
        if (isLight) { lightsIdx++; if (lightsIdx >= LIGHT_COUNT) { DualLogError("Too many lights %u in level%d.txt!\n", lightsIdx, curlevel); continue; } lit=&lightsFromFile[lightsIdx]; lanim=&lanimsFromFile[lightsIdx]; lit->lflags = LIGHT_AND_SHADOW_ON; }
        else {
            entCount++; if (entCount >= INSTANCE_COUNT) { DualLogError("Too many instances %u in level%d.txt!\n", entCount, curlevel); continue; } inst = &entsFromFile[entCount]; mset(inst,0,sizeof(Entity)); mset(&posFromFile[entCount],0,sizeof(V3)); scaleFromFile[entCount] = (V3){1.0f, 1.0f, 1.0f}; rotationFromFile[entCount] = QUAT_IDENTITY; colCtrFromFile[entCount] = (V3){0.0f,0.0f,0.0f}; colSzFromFile[entCount] = (V3){-1.0f,-1.0f,-1.0f}; 
            for (u8 slot=0;slot<4;++slot) inst->contents[slot]=inst->custIdx[slot]=-1; for (u8 slot=0;slot<7;++slot) inst->randomItem[slot]=inst->randomItemCustIdx[slot]=-1;
            inst->relayEnabled=true;
            fwLine=false; fwStage=0; fwCollecting=fwPendingChild=false; fwCurChild=fwLastChunkSlot=0; fwCurP=NULL; fwCurR=NULL; fwCurS=NULL; ipLine=false; ipBlock=0; ipSubRotSet=false; ipCurP=NULL; ipCurR=NULL; ipCurS=NULL; npcLine=false; npcBlock=0; npcRootRotSet[entCount]=false; fwContainerPos=(V3){0.0f,0.0f,0.0f}; fwContainerRot=QUAT_IDENTITY; fwContainerScale=(V3){1.0f,1.0f,1.0f}; fwInfoLocalTmp=(V3){0.0f,0.0f,0.0f};/*relayEnabled is seeded once, at the top of this block (line 582)*/
        }
        bool constIndexRead = false; bool activeStateRead = false; bool matIndexRead = false; u8 scaleReadMask = 0; u16 matIndexTexIdx = 881;
        while (line[0] != '\0') {
            char* pipe = StringFindFirstCharWithin(line, '|'); char* kvString = line; if (pipe) { *pipe = '\0'; line = pipe + 1; } else { line += slen(line); } if (kvString[0] == '\0') continue; char* colon = StringFindFirstCharWithin(kvString, ':'); if (!colon || colon[1] == '\0') continue; *colon = '\0'; char* key = kvString; char* value = colon + 1; int keyLen = (int)(colon - key); // length is free, no slen()
            if (isLight) { LoadFieldIntoLight(key,value,lineSpace,lineNum,lit,lanim,lightsIdx);}
            else {
                     if(KEY_EQ("constIndex")){if(!constIndexRead){/*SaveObject.Save writes the master PrefabIdentifier.constIndex as field 0, then appends per-component saves; GrenadeActivate.Save emits its own "constIndex" (the grenade type: 7 frag, 8 conc, 9 emp, 10 earth, 11 mine, 12 nitro, 13 gas, 14 plastique) onto the same record.  Both keys share a name, so keep the first.  Otherwise the 10 level-placed live landmines (402 then 11) loaded as chunk_bridg1_5 and never detonated.*/ constIndexRead=true; inst->index=parse_numberu16(value,lineSpace,lineNum); fwLine=(inst->index == 517); ipLine=(inst->index == 614 || inst->index == 615 || inst->index == 602); npcLine=IdxIsNPC(inst->index);}}
                else if(KEY_EQ("lP.x")||KEY_EQ("lP.y")||KEY_EQ("lP.z")||KEY_EQ("lR.x")||KEY_EQ("lR.y")||KEY_EQ("lR.z")||KEY_EQ("lR.w")||KEY_EQ("lS.x")||KEY_EQ("lS.y")||KEY_EQ("lS.z")) {
                    /*One block per transform: lP.x starts a new one. func_wall (517) keeps its own staged collection; InteractablePanel objects (ipLine) take block 0 only and drop the sub-GO blocks; NPC lines (npcLine) likewise take block 0 only; everything else writes straight to the object.*/
                    float v=parse_float(value,lineSpace,lineNum);
                    if (KEY_EQ("lP.x")) {
                        if (fwLine) { FWBeginBlock(entCount); }
                        else if (ipLine) {
                            if (ipBlock == 0) { ipCurP=&posFromFile[entCount]; ipCurR=&rotationFromFile[entCount]; ipCurS=&scaleFromFile[entCount]; }
                            else { ipCurP=&ipSubPos; ipCurR=&ipSubRot; ipCurS=&ipSubScale; if (ipBlock == 1) { ipSubPos=(V3){0.0f,0.0f,0.0f}; ipSubScale=(V3){1.0f,1.0f,1.0f}; ipSubRot=QUAT_IDENTITY; }/*block 1 is InteractablePanel.installationItem*/ }
                            ipBlock++;
                        }
                        else if (npcLine) { npcBlock++; }
                    }
                    if (npcLine && npcBlock > 1 && (inst->index != 437 || KEY_EQ("lP.x") || KEY_EQ("lP.y") || KEY_EQ("lP.z") || KEY_EQ("lS.x") || KEY_EQ("lS.y") || KEY_EQ("lS.z"))) { /*trailing gib/searchCollider/deathBurst sub-GO transform on an NPC line: not the NPC's own transform, so drop the whole lP/lR/lS key (same rationale as ipLine block-0 above; without this the last gib's -90deg X rotation overwrote the NPC's facing). npc_servbot (437) is excepted for lR only: its sole lR rides the trailing block on all 11 lines and is the pre-existing authored placement, so it passes through while lP/lS still drop*/ }
                    else if (KEY_EQ("lP.x")) { if(fwLine){ if(fwCurP)fwCurP->x=v; } else if(ipLine){ if(ipCurP)ipCurP->x=v; } else posFromFile[entCount].x=v; }
                    else if (KEY_EQ("lP.y")) { if(fwLine){ if(fwCurP)fwCurP->y=v; } else if(ipLine){ if(ipCurP)ipCurP->y=v; } else posFromFile[entCount].y=v; }
                    else if (KEY_EQ("lP.z")) { if(fwLine){ if(fwCurP)fwCurP->z=v; } else if(ipLine){ if(ipCurP)ipCurP->z=v; } else posFromFile[entCount].z=v; }
                    else if (KEY_EQ("lR.x")) { if(fwLine){ if(fwCurR)fwCurR->x=v; } else if(ipLine){ if(ipCurR)ipCurR->x=v; } else { if(npcLine && npcBlock<=1) npcRootRotSet[entCount]=true; rotationFromFile[entCount].x=v; } }
                    else if (KEY_EQ("lR.y")) { if(fwLine){ if(fwCurR)fwCurR->y=v; } else if(ipLine){ if(ipCurR)ipCurR->y=v; } else { if(npcLine && npcBlock<=1) npcRootRotSet[entCount]=true; rotationFromFile[entCount].y=v; } }
                    else if (KEY_EQ("lR.z")) { if(fwLine){ if(fwCurR)fwCurR->z=v; } else if(ipLine){ if(ipCurR)ipCurR->z=v; } else { if(npcLine && npcBlock<=1) npcRootRotSet[entCount]=true; rotationFromFile[entCount].z=v; } }
                    else if (KEY_EQ("lR.w")) { if(fwLine){ if(fwCurR)fwCurR->w=v; } else if(ipLine){ if(ipCurR)ipCurR->w=v; if(ipBlock>1) ipSubRotSet=true; } else { if(npcLine && npcBlock<=1) npcRootRotSet[entCount]=true; rotationFromFile[entCount].w=v; } }
                    else if (KEY_EQ("lS.x")) { if(fwLine){ if(fwCurS)fwCurS->x=v; } else if(ipLine){ if(ipCurS)ipCurS->x=v; } else { scaleFromFile[entCount].x=v; scaleReadMask|=1u; } }
                    else if (KEY_EQ("lS.y")) { if(fwLine){ if(fwCurS)fwCurS->y=v; } else if(ipLine){ if(ipCurS)ipCurS->y=v; } else { scaleFromFile[entCount].y=v; scaleReadMask|=2u; } }
                    else if (KEY_EQ("lS.z")) { if(fwLine){ if(fwCurS)fwCurS->z=v; } else if(ipLine){ if(ipCurS)ipCurS->z=v; } else { scaleFromFile[entCount].z=v; scaleReadMask|=4u; } }
                }
                else if(KEY_EQ("go.activeSelf"))   { activeStateRead = true; flag_set(&inst->entflags, EF_ACTIVE, parse_bool(value, lineSpace, lineNum)); }
                else if(KEY_EQ("health")) inst->health=parse_float(value,lineSpace,lineNum);                                       else if(KEY_EQ("cyberHealth")) inst->cyberHealth=parse_float(value,lineSpace,lineNum);/*bloodType is a prefab constant (AddInstance, DefaultPrefabBloodType) and no level record carries the key -- deliberately no parse branch.*/
                  else if(KEY_EQ("startYAngle")) inst->camStartYAngle=parse_float(value,lineSpace,lineNum); else if(KEY_EQ("endYAngle")) inst->camEndYAngle=parse_float(value,lineSpace,lineNum); else if(KEY_EQ("waitTime")) inst->camWaitTime=parse_float(value,lineSpace,lineNum);
                  else if(KEY_EQ("enabled") && inst->index == 477) inst->camRotateEnabled=parse_bool(value,lineSpace,lineNum);
                else if(KEY_EQ("keycode")) inst->keycode=parse_numberu16(value,lineSpace,lineNum);
                else if(KEY_EQ("radiationAmount")) inst->radiation=parse_float(value,lineSpace,lineNum);                           else if(KEY_EQ("ammo"))inst->ammo=parse_numberi16(value,lineSpace,lineNum);
                else if(KEY_EQ("ammo2"))inst->ammo2=parse_numberi16(value,lineSpace,lineNum);                                      else if(KEY_EQ("useableItemIndex"))inst->lookUpIndex=(u16)parse_numberi16(value,lineSpace,lineNum);
                /* Citadel serializes the AIController's whole state into the level file. These flags drive InitNPC's
                   starting state (wandering/roaming) and the early-out checks, so they have to survive the load or every
                   authored patroller, sleeping enemy and turret comes up idle/awake and mobile. */
                else if(KEY_EQ("idleTime"))       inst->idleTime=parse_float(value,lineSpace,lineNum);
                else if(KEY_EQ("lookUpIndex"))   inst->lookUpIndex=(u16)parse_numberi16(value,lineSpace,lineNum);
                else if(KEY_EQ("wandering"))     flag_set(&inst->entflags, EF_WANDERING, parse_bool(value,lineSpace,lineNum));
                else if(KEY_EQ("asleep"))        flag_set(&inst->entflags, EF_ASLEEP, parse_bool(value,lineSpace,lineNum));
                else if(KEY_EQ("actAsTurret"))   flag_set(&inst->entflags, EF_ACT_AS_TURRET, parse_bool(value,lineSpace,lineNum));
                else if(KEY_EQ("actAsCorpseOnly")) flag_set(&inst->entflags, EF_ACT_AS_CORPSE_ONLY, parse_bool(value,lineSpace,lineNum));
                else if(KEY_EQ("teleportOnDeath")) flag_set(&inst->entflags, EF_TELEPORT_ON_DEATH, parse_bool(value,lineSpace,lineNum));
                else if(KEY_EQ("firstSighting"))  flag_set(&inst->entflags, EF_FIRST_SIGHTING, parse_bool(value,lineSpace,lineNum));
                else if(KEY_EQ("shotFired"))      flag_set(&inst->entflags, EF_SHOT_FIRED, parse_bool(value,lineSpace,lineNum));
                else if(KEY_EQ("goIntoPain"))     flag_set(&inst->entflags, EF_GO_INTO_PAIN, parse_bool(value,lineSpace,lineNum));
                else if(KEY_EQ("dyingSetup"))     flag_set(&inst->entflags, EF_DYING_SETUP, parse_bool(value,lineSpace,lineNum));
                else if(KEY_EQ("ai_dying"))       flag_set(&inst->entflags, EF_DYING, parse_bool(value,lineSpace,lineNum));
                else if(KEY_EQ("ai_dead"))        flag_set(&inst->entflags, EF_DEAD, parse_bool(value,lineSpace,lineNum));
                else if(KEY_EQ("hopDone"))        flag_set(&inst->entflags, EF_HOP_DONE, parse_bool(value,lineSpace,lineNum));
                else if(KEY_EQ("deathBurstDone")) flag_set(&inst->entflags, EF_DEATH_BURST_DONE, parse_bool(value,lineSpace,lineNum));
                else if(KEY_EQ("hadEnemy"))       flag_set(&inst->entflags, EF_HAD_ENEMY, parse_bool(value,lineSpace,lineNum));
                else if(KEY_EQ("walkPathOnStart")) flag_set(&inst->entflags, EF_WALK_PATH_ON_START, parse_bool(value,lineSpace,lineNum));
                else if(KEY_EQ("inSight"))        flag_set(&inst->entflags, EF_ENEM_IN_SIGHT, parse_bool(value,lineSpace,lineNum));
                else if(KEY_EQ("infront"))        flag_set(&inst->entflags, EF_ENEM_IN_FRONT, parse_bool(value,lineSpace,lineNum));
                else if(KEY_EQ("inProjFOV"))      flag_set(&inst->entflags, EF_ENEM_IN_FOV, parse_bool(value,lineSpace,lineNum));
                else if(KEY_EQ("LOSpossible"))    flag_set(&inst->entflags, EF_ENEM_IN_LOS, parse_bool(value,lineSpace,lineNum));
                else if(KEY_EQ("currentState"))   inst->currentState=(AIState)vclampi(parse_numberi16(value,lineSpace,lineNum),0,AIState_Dead);
                else if(KEY_EQ("attack1SoundTime"))  inst->attack1SoundTime=parse_float(value,lineSpace,lineNum);
                else if(KEY_EQ("attack2SoundTime"))  inst->attack2SoundTime=parse_float(value,lineSpace,lineNum);
                else if(KEY_EQ("attack3SoundTime"))  inst->attack3SoundTime=parse_float(value,lineSpace,lineNum);
                else if(KEY_EQ("currentDestination.x")) inst->currentDestination.x=parse_float(value,lineSpace,lineNum);
                else if(KEY_EQ("currentDestination.y")) inst->currentDestination.y=parse_float(value,lineSpace,lineNum);
                else if(KEY_EQ("currentDestination.z")) inst->currentDestination.z=parse_float(value,lineSpace,lineNum);
                else if(KEY_EQ("idealTransformForward.x")) inst->idealTransformForward.x=parse_float(value,lineSpace,lineNum);
                else if(KEY_EQ("idealTransformForward.y")) inst->idealTransformForward.y=parse_float(value,lineSpace,lineNum);
                else if(KEY_EQ("idealTransformForward.z")) inst->idealTransformForward.z=parse_float(value,lineSpace,lineNum);
                else if(KEY_EQ("idealPos.x")) inst->idealPos.x=parse_float(value,lineSpace,lineNum);
                else if(KEY_EQ("idealPos.y")) inst->idealPos.y=parse_float(value,lineSpace,lineNum);
                else if(KEY_EQ("idealPos.z")) inst->idealPos.z=parse_float(value,lineSpace,lineNum);
                else if(KEY_EQ("targettingPosition.x")) inst->targettingPosition.x=parse_float(value,lineSpace,lineNum);
                else if(KEY_EQ("targettingPosition.y")) inst->targettingPosition.y=parse_float(value,lineSpace,lineNum);
                else if(KEY_EQ("targettingPosition.z")) inst->targettingPosition.z=parse_float(value,lineSpace,lineNum);
                
                else if(KEY_EQ("generateContents")) inst->generateContents = parse_bool(value, lineSpace, lineNum); else if(KEY_EQ("generationDone")) inst->generationDone = parse_bool(value, lineSpace, lineNum); else if(KEY_EQ("searchableInUse")) inst->srchInUse = parse_bool(value, lineSpace, lineNum); else if(KEY_EQ("maxRandomItems")) inst->maxRandomItems = (u8)parse_numberi16(value,lineSpace,lineNum); else if(KEY_EQ("contents[0]")) inst->contents[0] = parse_numberi16(value,lineSpace,lineNum);
                else if(KEY_EQ("contents[1]")) inst->contents[1] = parse_numberi16(value,lineSpace,lineNum); else if(KEY_EQ("contents[2]")) inst->contents[2] = parse_numberi16(value,lineSpace,lineNum); else if(KEY_EQ("contents[3]")) inst->contents[3] = parse_numberi16(value,lineSpace,lineNum);
                else if(KEY_EQ("customIndex[0]") || KEY_EQ("custIdx[0]")) inst->custIdx[0] = parse_numberi16(value,lineSpace,lineNum); else if(KEY_EQ("customIndex[1]") || KEY_EQ("custIdx[1]")) inst->custIdx[1] = parse_numberi16(value,lineSpace,lineNum);
                else if(KEY_EQ("customIndex[2]") || KEY_EQ("custIdx[2]")) inst->custIdx[2] = parse_numberi16(value,lineSpace,lineNum); else if(KEY_EQ("customIndex[3]") || KEY_EQ("custIdx[3]")) inst->custIdx[3] = parse_numberi16(value,lineSpace,lineNum);
                else if(KEY_EQ("randomItem[0]")) inst->randomItem[0] = parse_numberi16(value,lineSpace,lineNum); else if(KEY_EQ("randomItem[1]")) inst->randomItem[1] = parse_numberi16(value,lineSpace,lineNum); else if(KEY_EQ("randomItem[2]")) inst->randomItem[2] = parse_numberi16(value,lineSpace,lineNum); else if(KEY_EQ("randomItem[3]")) inst->randomItem[3] = parse_numberi16(value,lineSpace,lineNum); else if(KEY_EQ("randomItem[4]")) inst->randomItem[4] = parse_numberi16(value,lineSpace,lineNum); else if(KEY_EQ("randomItem[5]")) inst->randomItem[5] = parse_numberi16(value,lineSpace,lineNum); else if(KEY_EQ("randomItem[6]")) inst->randomItem[6] = parse_numberi16(value,lineSpace,lineNum);
                else if(KEY_EQ("randomItemDropChance[0]")) inst->randomItemDropChance[0] = parse_float(value,lineSpace,lineNum); else if(KEY_EQ("randomItemDropChance[1]")) inst->randomItemDropChance[1] = parse_float(value,lineSpace,lineNum);
                else if(KEY_EQ("randomItemDropChance[2]")) inst->randomItemDropChance[2] = parse_float(value,lineSpace,lineNum); else if(KEY_EQ("randomItemDropChance[3]")) inst->randomItemDropChance[3] = parse_float(value,lineSpace,lineNum); else if(KEY_EQ("randomItemDropChance[4]")) inst->randomItemDropChance[4] = parse_float(value,lineSpace,lineNum); else if(KEY_EQ("randomItemDropChance[5]")) inst->randomItemDropChance[5] = parse_float(value,lineSpace,lineNum); else if(KEY_EQ("randomItemDropChance[6]")) inst->randomItemDropChance[6] = parse_float(value,lineSpace,lineNum);
                else if(KEY_EQ("customIndex"))inst->customIndex=(u16)parse_numberi16(value,lineSpace,lineNum);                   else if(KEY_EQ("amount"))          inst->amount = parse_float(value, lineSpace, lineNum);
                else if(KEY_EQ("resetTime"))       inst->resetTime = parse_float(value, lineSpace, lineNum);                       else if(KEY_EQ("minSecurityLevel"))inst->minSecurityLevel = parse_float(value, lineSpace, lineNum);
                else if(KEY_EQ("rechargeMsgLingdex")) inst->rechargeMsgLingdex = parse_numberi16(value,lineSpace,lineNum);          else if(KEY_EQ("usedMsgLingdex")) inst->usedMsgLingdex = parse_numberi16(value,lineSpace,lineNum);
                else if(KEY_EQ("requireReset"))    inst->requireRecharge = parse_bool(value,lineSpace,lineNum);
                else if(KEY_EQ("nextthink"))       inst->rechargeFinished = parse_float(value,lineSpace,lineNum) + World.pauseRelativeTime; /*saved as differential against relativeTime (Citadel Utils.SaveRelativeTimeDifferential)*/
                else if(KEY_EQ("damageOnUse"))     inst->damage = parse_float(value, lineSpace, lineNum);                          else if(KEY_EQ("target"))          inst->targetIdx = IOInternName(value);
                else if(KEY_EQ("targetname"))      inst->targetnameIdx = IOInternName(value);
                  else if(KEY_EQ("targetOnDeath"))   inst->targetOnDeathIdx = IOInternName(value);/*HealthManager.targetOnDeath, fired from UseDeathTargets only; a separate name from TargetIO.target*/                                      else if(KEY_EQ("target2"))         inst->target2Idx = IOInternName(value);
                else if(KEY_EQ("targetIfFalse"))   inst->targetIfFalseIdx = IOInternName(value);                                   else if(KEY_EQ("randomItemCustomIndex[0]")) inst->randomItemCustIdx[0] = parse_numberi16(value,lineSpace,lineNum);
                else if(KEY_EQ("randomItemCustomIndex[1]")) inst->randomItemCustIdx[1] = parse_numberi16(value,lineSpace,lineNum); else if(KEY_EQ("randomItemCustomIndex[2]")) inst->randomItemCustIdx[2] = parse_numberi16(value,lineSpace,lineNum);
                else if(KEY_EQ("randomItemCustomIndex[3]")) inst->randomItemCustIdx[3] = parse_numberi16(value,lineSpace,lineNum); else if(KEY_EQ("randomItemCustomIndex[4]")) inst->randomItemCustIdx[4] = parse_numberi16(value,lineSpace,lineNum); else if(KEY_EQ("randomItemCustomIndex[5]")) inst->randomItemCustIdx[5] = parse_numberi16(value,lineSpace,lineNum); else if(KEY_EQ("randomItemCustomIndex[6]")) inst->randomItemCustIdx[6] = parse_numberi16(value,lineSpace,lineNum); else if(KEY_EQ("musicType"))       inst->musicType = (MusicType)parse_numberu8(value,lineSpace,lineNum); // trigger_music
                else if(KEY_EQ("securityThreshhold") || KEY_EQ("securityThreshold")) inst->securityThreshold=parse_numberu8(value,lineSpace,lineNum);
                else if(KEY_EQ("messageIndex"))    inst->messageIndex = parse_numberi16(value,lineSpace,lineNum);                  else if(KEY_EQ("delay"))           inst->delay = parse_float(value,lineSpace,lineNum);
                /* trigger_counter (594).  Without countToTrigger parsed, TriggerCounterTargetted compared against a
                   permanent 0, so every counter in every level silently counted up forever and never ran its target. */
                else if(KEY_EQ("counter"))           inst->counter = parse_numberu16(value,lineSpace,lineNum);          else if(KEY_EQ("countToTrigger")) inst->countToTrigger = parse_numberu16(value,lineSpace,lineNum);
                else if(KEY_EQ("dontReset"))         inst->dontReset = parse_bool(value,lineSpace,lineNum);
                else if(KEY_EQ("lockedTarget"))    inst->lockedTargetIdx = IOInternName(value);                                            else if(KEY_EQ("useQuestKeycode1")) inst->useQuestKeycode1 = parse_bool(value, lineSpace, lineNum); else if(KEY_EQ("useQuestKeycode2")) inst->useQuestKeycode2 = parse_bool(value, lineSpace, lineNum);
                else if(KEY_EQ("locked"))          flag_set(&inst->entflags,EF_LOCKED,parse_bool(value,lineSpace,lineNum));        else if(KEY_EQ("active"))          inst->active = parse_bool(value, lineSpace, lineNum);
                else if(KEY_EQ("timeInterval"))   inst->timeInterval = parse_float(value, lineSpace, lineNum);                     else if(KEY_EQ("randomMin"))      inst->randomMin = parse_float(value, lineSpace, lineNum);
                else if(KEY_EQ("randomMax"))      inst->randomMax = parse_float(value, lineSpace, lineNum);                        else if(KEY_EQ("useRandomTimes")) inst->useRandomTimes = parse_bool(value, lineSpace, lineNum);
                else if(KEY_EQ("onlyOnce"))        inst->onlyOnce = parse_bool(value, lineSpace, lineNum);                         else if(KEY_EQ("stayOpen"))        inst->stayOpen = parse_bool(value, lineSpace, lineNum);
                else if(KEY_EQ("startOpen"))       inst->startOpen = parse_bool(value, lineSpace, lineNum);                        else if(KEY_EQ("ajar"))            inst->ajar = parse_bool(value, lineSpace, lineNum);
                else if(KEY_EQ("open"))            inst->panelOpen      = parse_bool(value,lineSpace,lineNum);/*InteractablePanel cover state (only us_relaypanel/isotopepanel/retinalscanner carry these keys)*/
                else if(KEY_EQ("installed"))       inst->panelInstalled = parse_bool(value,lineSpace,lineNum);/*InteractablePanel: the required item is already in place*/
                else if(KEY_EQ("ajarPercentage"))  inst->ajarPercentage = parse_float(value, lineSpace, lineNum);                  else if(KEY_EQ("timeBeforeLasersOn")) inst->timeBeforeLasersOn = parse_float(value, lineSpace, lineNum);
                else if(KEY_EQ("toggleLasers"))    inst->toggleLasers = parse_bool(value, lineSpace, lineNum);/*TODO: parsed and copied but never read. The door's security-laser detector toggle is not wired; lev8doorx1 / lev8doorx2 (level8.txt:4580,4582) are the only two records that ask for it -- both set it on themselves via a self-target. Implement when the laser detector exists: needs a live beacon + a targetted test on the door. Skip for now.*/              else if(KEY_EQ("changeLayerOnOpenClose")) inst->changeLayerOnOpenClose=parse_bool(value,lineSpace,lineNum);
                else if(KEY_EQ("waitBeforeClose")) inst->waitBefClose=parse_float(value,lineSpace,lineNum) + World.pauseRelativeTime;                                    else if(KEY_EQ("doorOpen")) flag_set(&inst->ioflags,TARG_IOFLAGS_DOOROPEN,parse_bool(value,lineSpace,lineNum));
                else if(KEY_EQ("doorOpenIfUnlocked") || KEY_EQ("doorToggle")) flag_set(&inst->ioflags,TARG_IOFLAGS_DOOROPENIFUNLOCKED,parse_bool(value,lineSpace,lineNum)); else if(KEY_EQ("doorClose")) flag_set(&inst->ioflags,TARG_IOFLAGS_DOORCLOSE,parse_bool(value,lineSpace,lineNum));
                else if(KEY_EQ("doorLock") || KEY_EQ("lockElevatorPad")) flag_set(&inst->ioflags, TARG_IOFLAGS_LOCK, parse_bool(value, lineSpace, lineNum));
                else if(KEY_EQ("doorUnlock") || KEY_EQ("unlockSwitch") || KEY_EQ("unlockElevatorPad") || KEY_EQ("unlockKeycodePad") || KEY_EQ("unlockPuzzlePad")) flag_set(&inst->ioflags,TARG_IOFLAGS_UNLOCK,parse_bool(value,lineSpace,lineNum));
                else if(KEY_EQ("switchTrigger"))   flag_set(&inst->ioflags, TARG_IOFLAGS_SWITCHTRIGGER, parse_bool(value, lineSpace, lineNum));            else if(KEY_EQ("tripTrigger")) flag_set(&inst->ioflags, TARG_IOFLAGS_TRIPTRIGGER, parse_bool(value, lineSpace, lineNum));
                else if(KEY_EQ("forceBridgeActivate"))   flag_set(&inst->ioflags, TARG_IOFLAGS_FBRIDGE_ACTIVATE, parse_bool(value, lineSpace, lineNum));   else if(KEY_EQ("forceBridgeDeactivate")) flag_set(&inst->ioflags,TARG_IOFLAGS_FBRIDGE_DEACTIVATE,parse_bool(value,lineSpace,lineNum));
                else if(KEY_EQ("forceBridgeToggle"))     flag_set(&inst->ioflags, TARG_IOFLAGS_FBRIDGE_TOGGLE, parse_bool(value, lineSpace, lineNum));     else if(KEY_EQ("gravityLiftToggle")) flag_set(&inst->ioflags,TARG_IOFLAGS_GRAVLIFT_TOGGLE,parse_bool(value,lineSpace,lineNum));
                else if(KEY_EQ("textureChangeToggle"))   flag_set(&inst->ioflags, TARG_IOFLAGS_TEXTURE_CHG_TOGGLE, parse_bool(value, lineSpace, lineNum)); else if(KEY_EQ("lightOn")) flag_set(&inst->ioflags,TARG_IOFLAGS_LIGHT_ON,parse_bool(value,lineSpace,lineNum));
                else if(KEY_EQ("lightOff"))        flag_set(&inst->ioflags, TARG_IOFLAGS_LIGHT_OFF, parse_bool(value, lineSpace, lineNum));                else if(KEY_EQ("lightToggle"))     flag_set(&inst->ioflags, TARG_IOFLAGS_LIGHT_TOGGLE, parse_bool(value, lineSpace, lineNum));
                else if(KEY_EQ("funcwallMove"))    flag_set(&inst->ioflags, TARG_IOFLAGS_FUNCWALL_MOVE, parse_bool(value, lineSpace, lineNum));            else if(KEY_EQ("missionBitOn"))    flag_set(&inst->ioflags, TARG_IOFLAGS_MISSION_BIT_ON, parse_bool(value, lineSpace, lineNum));
                else if(KEY_EQ("missionBitOff"))   flag_set(&inst->ioflags, TARG_IOFLAGS_MISSION_BIT_OFF, parse_bool(value, lineSpace, lineNum));          else if(KEY_EQ("missionBitToggle"))flag_set(&inst->ioflags, TARG_IOFLAGS_MISSION_BIT_TOGGLE, parse_bool(value, lineSpace, lineNum));
                else if(KEY_EQ("switchLockToggle"))flag_set(&inst->ioflags, TARG_IOFLAGS_SWITCH_LOCK_TOGGLE, parse_bool(value, lineSpace, lineNum));       else if(KEY_EQ("GOSetActive"))     flag_set(&inst->ioflags, TARG_IOFLAGS_INST_ACTIVATE, parse_bool(value, lineSpace, lineNum));
                else if(KEY_EQ("enemyAlert"))       flag_set(&inst->ioflags, TARG_IOFLAGS_ENEMY_ALERT, parse_bool(value, lineSpace, lineNum));
                else if(KEY_EQ("spawnerActivate")) flag_set(&inst->ioflagsHi, TARG_IOFLAGHI_SPAWNER_ACTIVATE, parse_bool(value, lineSpace, lineNum)); else if(KEY_EQ("spawnerActivateAlerted")) flag_set(&inst->ioflagsHi, TARG_IOFLAGHI_SPAWNER_ACTIVATE_ALERTED, parse_bool(value, lineSpace, lineNum));
                else if(KEY_EQ("awakeSleepingEnemy")) flag_set(&inst->ioflagsHi, TARG_IOFLAGHI_AWAKE_SLEEPING, parse_bool(value, lineSpace, lineNum));
                else if(KEY_EQ("SpawnManager.active")) inst->active = parse_bool(value, lineSpace, lineNum);     else if(KEY_EQ("numberActive"))       inst->numberActive = parse_numberu16(value, lineSpace, lineNum);
                else if(KEY_EQ("numberToSpawn"))       inst->numberToSpawn = parse_numberu16(value, lineSpace, lineNum);  else if(KEY_EQ("countOnlySameIndex"))  inst->countOnlySameIndex = parse_bool(value, lineSpace, lineNum);
                else if(KEY_EQ("minDelayBetweenSpawns")) inst->minDelayBetweenSpawns = parse_float(value, lineSpace, lineNum); else if(KEY_EQ("maxDelayBetweenSpawns")) inst->maxDelayBetweenSpawns = parse_float(value, lineSpace, lineNum);
                else if(KEY_EQ("allSpawnedResetDelay")) inst->allSpawnedResetDelay = parse_float(value, lineSpace, lineNum); else if(KEY_EQ("alertEnemiesOnAwake"))  inst->alertEnemiesOnAwake = parse_bool(value, lineSpace, lineNum);
                else if(KEY_EQ("index") && inst->index == 702) inst->spawnIndex = parse_numberu16(value, lineSpace, lineNum);/*SpawnManager.index is the NPC constIndex it respawns; only meaningful on 702, since AIController.index means something else on every other prefab*/
                else if(KEY_EQ("GOSetDeactive"))   flag_set(&inst->ioflags, TARG_IOFLAGS_INST_DEACTIVATE, parse_bool(value, lineSpace, lineNum));          else if(KEY_EQ("GOToggleActive"))  flag_set(&inst->ioflags, TARG_IOFLAGS_INST_TOGGLE, parse_bool(value, lineSpace, lineNum));
                else if(KEY_EQ("disableThisGOOnAwake")) flag_set(&inst->ioflags, TARG_IOFLAGS_DISABLE_ON_AWAKE, parse_bool(value, lineSpace, lineNum));    else if(KEY_EQ("playSoundOnce"))   flag_set(&inst->ioflags, TARG_IOFLAGS_PLAY_SOUND_ONCE, parse_bool(value, lineSpace, lineNum)); else if(KEY_EQ("playEverywhere")) inst->playEverywhere = parse_bool(value, lineSpace, lineNum);
                else if(KEY_EQ("stopSound"))       flag_set(&inst->ioflags, TARG_IOFLAGS_STOP_SOUND, parse_bool(value, lineSpace, lineNum));               else if(KEY_EQ("startFlashingMaterials")) flag_set(&inst->ioflags, TARG_IOFLAGS_START_FLASHING_TEX, parse_bool(value, lineSpace, lineNum));
                else if(KEY_EQ("stopFlashingMaterials"))  flag_set(&inst->ioflags, TARG_IOFLAGS_STOP_FLASHING_TEX, parse_bool(value, lineSpace, lineNum)); else if(KEY_EQ("branchFlip"))      flag_set(&inst->ioflags, TARG_IOFLAGS_BRANCH_FLIP, parse_bool(value, lineSpace, lineNum));
                else if(KEY_EQ("branchFlipOnly"))  flag_set(&inst->ioflags, TARG_IOFLAGS_BRANCH_FLIPONLY, parse_bool(value, lineSpace, lineNum));          else if(KEY_EQ("resourceFolder") && *value) inst->texAnimResourceFolderIdx = TexAnimFolderIntern(value);
                else if(KEY_EQ("codeScreen"))      inst->codeScreen = (u8)parse_bool(value, lineSpace, lineNum);/*prop_cpuscreen (551): flag only, the level is implicit -- see Entity.codeScreen*/
                else if(KEY_EQ("lingdex") || KEY_EQ("messageLingdex"))  inst->messageLingdex = parse_numberi16(value, lineSpace, lineNum);                                      else if(KEY_EQ("lockedMessageLingdex")) inst->lockedMessageLingdex = parse_numberi16(value, lineSpace, lineNum);
                else if(KEY_EQ("matIndex")) { matIndexRead = true; matIndexTexIdx = TextMatIndexToTexIndex(parse_numberu16(value,lineSpace,lineNum)); }
                else if(KEY_EQ("texIndex")) inst->texIndex = parse_numberu16(value,lineSpace,lineNum);
                else if(KEY_EQ("text")) { if (inst && (inst->index == 592 || inst->index == 593)) { DecalPend* pd=PendDecal((u16)entCount); if (pd) { u16 k=0; while(value[k] && k<DECAL_INLINE_TEXT_LEN-1) { pd->text[k]=(value[k]=='#')?'\n':value[k]; ++k; } pd->text[k]='\0'; } } }
                else if(KEY_EQ("tA")) { if (inst && (inst->index == 592 || inst->index == 593)) { DecalPend* pd=PendDecal((u16)entCount); if (pd){ i16 v=parse_numberi16(value,lineSpace,lineNum); pd->anchor=(u8)(v<0?0:(v>8?8:v)); } } }
                else if(KEY_EQ("tAl")) { if (inst && (inst->index == 592 || inst->index == 593)) { DecalPend* pd=PendDecal((u16)entCount); if (pd){ i16 v=parse_numberi16(value,lineSpace,lineNum); pd->align=(u8)(v<0?0:(v>2?2:v)); } } }
                else if(KEY_EQ("tLs")) { if (inst && (inst->index == 592 || inst->index == 593)) { DecalPend* pd=PendDecal((u16)entCount); if (pd) pd->lineSp=parse_float(value,lineSpace,lineNum); } }
                else if(KEY_EQ("SFXIndex"))        inst->SFXIndex = (i16)parse_numberi16(value, lineSpace, lineNum);                                       else if(KEY_EQ("SFXClip"))         inst->SFXIndex = (i16)parse_numberi16(value, lineSpace, lineNum); else if(KEY_EQ("SFXLockedIndex")) inst->SFXLockedIndex = (i16)parse_numberi16(value, lineSpace, lineNum);
                else if(KEY_EQ("trackType"))       inst->trackType = (TrackType)parse_numberi16(value, lineSpace, lineNum);/*trigger_music (599)*/      else if(KEY_EQ("textIndex"))       inst->textIndex = (i16)parse_numberi16(value, lineSpace, lineNum);/*prop_cyber_datafrag (552), prop_cyber_switch (555)*/
                else if(KEY_EQ("logIndex"))        inst->logIndex = (i16)parse_numberi16(value, lineSpace, lineNum);
                else if(keyLen==23 && sCompUpToLen(key,"currentPositionsLeft[",21)==0 && key[21]>='0' && key[21]<='6' && key[22]==']') inst->wireCurL[key[21]-'0']=(i8)parse_numberi16(value, lineSpace, lineNum);/*us_puz_panel_*_wire (741..745)*/
                else if(keyLen==24 && sCompUpToLen(key,"currentPositionsRight[",22)==0 && key[22]>='0' && key[22]<='6' && key[23]==']') inst->wireCurR[key[22]-'0']=(i8)parse_numberi16(value, lineSpace, lineNum);
                else if(sCompUpToLen(key,"grid[",5)==0 && keyLen>=7 && key[keyLen-1]==']') { int gi=0; bool gok=true; for (int k=5;k<keyLen-1;++k) { if (key[k]<'0'||key[k]>'9') { gok=false; break; } gi=gi*10+(key[k]-'0'); } if (gok && gi<35) Entity_SetGridCell(inst,gi,parse_bool(value,lineSpace,lineNum)); }/*PuzzleGridPuzzle.grid[0..34], key width varies with the index*/
                else if(KEY_EQ("fired"))          inst->puzzleFired = parse_bool(value,lineSpace,lineNum);/*PuzzleGridPuzzle.fired: the onlyFireOnce latch*/
                else if(KEY_EQ("relayEnabled"))    inst->relayEnabled = parse_bool(value, lineSpace, lineNum);
                else if(KEY_EQ("onSecond"))        inst->branchOnSecond = parse_bool(value, lineSpace, lineNum);                                           else if(KEY_EQ("onceEver"))        inst->relayOnceEver = parse_bool(value, lineSpace, lineNum);
                else if(KEY_EQ("requiredAccessCard")) inst->requiredAccessCard = parse_numberi8(value, lineSpace, lineNum);                                else if(KEY_EQ("testQuestBitIsOn"))    inst->questTestMode = parse_bool(value,lineSpace,lineNum) ? 1 : inst->questTestMode;
                else if(KEY_EQ("testQuestBitIsOff"))   inst->questTestMode = parse_bool(value,lineSpace,lineNum) ? 2 : inst->questTestMode;                else if(KEY_EQ("ForceBridge.activated")) inst->active = parse_bool(value,lineSpace,lineNum);
                else if(KEY_EQ("x")) inst->forceFieldDirectionX = parse_bool(value, lineSpace, lineNum); else if(KEY_EQ("y")) inst->forceFieldDirectionY = parse_bool(value, lineSpace, lineNum); else if(KEY_EQ("z")) inst->forceFieldDirectionZ = parse_bool(value, lineSpace, lineNum); 
                else if(KEY_EQ("activatedScaleX")) inst->activatedScale.x = parse_float(value, lineSpace, lineNum); else if(KEY_EQ("activatedScaleY")) inst->activatedScale.y = parse_float(value, lineSpace, lineNum); else if(KEY_EQ("activatedScaleZ")) inst->activatedScale.z = parse_float(value, lineSpace, lineNum);
                else if(KEY_EQ("fieldColor"))          { u8 fc = parse_numberu8(value, lineSpace, lineNum); inst->fieldColor = (fc >= 1 && fc <= 5) ? (ForceFieldColor)(fc-1) : ForceFieldColor_Red; }                      else if(KEY_EQ("touchEnabled")) inst->touchEnabled = parse_bool(value,lineSpace,lineNum);
                else if(KEY_EQ("maxDistance")) inst->reverbMaxDist = parse_float(value, lineSpace, lineNum); else if(KEY_EQ("reverbPreset")) inst->reverbPreset = (u16)parse_numberu16(value, lineSpace, lineNum);
                else if(KEY_EQ("center.x")) colCtrFromFile[entCount].x = parse_float(value,lineSpace,lineNum); else if(KEY_EQ("center.y")) colCtrFromFile[entCount].y = parse_float(value,lineSpace,lineNum); else if(KEY_EQ("center.z")) colCtrFromFile[entCount].z = parse_float(value,lineSpace,lineNum);
                else if(KEY_EQ("size.x")) colSzFromFile[entCount].x = parse_float(value,lineSpace,lineNum); else if(KEY_EQ("size.y")) colSzFromFile[entCount].y = parse_float(value,lineSpace,lineNum); else if(KEY_EQ("size.z")) colSzFromFile[entCount].z = parse_float(value,lineSpace,lineNum);
                else if(KEY_EQ("direction.x")) inst->dir.x = parse_float(value,lineSpace,lineNum); else if(KEY_EQ("direction.y")) inst->dir.y = parse_float(value,lineSpace,lineNum); else if(KEY_EQ("direction.z")) inst->dir.z = parse_float(value,lineSpace,lineNum);
                else if(KEY_EQ("force")) inst->force = parse_float(value,lineSpace,lineNum);/*trigger_cyberpush (595)*/
                else if(KEY_EQ("emailIndex")) inst->emailIndex = parse_numberi16(value,lineSpace,lineNum); else if(KEY_EQ("autoPlayEmail")) inst->autoPlayEmail = parse_bool(value,lineSpace,lineNum);
                else if(KEY_EQ("teleportID")) inst->teleportID = parse_numberu16(value,lineSpace,lineNum);
                else if(KEY_EQ("targetDestinationID")) inst->targetDestinationID = parse_numberu16(value,lineSpace,lineNum);
                else if(KEY_EQ("justUsed")) inst->justUsed = parse_float(value,lineSpace,lineNum) + World.pauseRelativeTime;
                else if(KEY_EQ("allDone")) inst->allDone = parse_bool(value,lineSpace,lineNum);/*trigger_* onlyOnce latch (TriggerTriggerTripped, CyberSwitchUpdate)*/
                else if(KEY_EQ("iceActive")) inst->iceActive = parse_bool(value,lineSpace,lineNum);/*prop_cyber_switch (555)*/
                else if(KEY_EQ("broken")) inst->broken = parse_bool(value,lineSpace,lineNum);/*prop_healingbed (574)*/
                else if(KEY_EQ("despawnInstead")) inst->despawnInstead = parse_bool(value,lineSpace,lineNum);/*us_relaypanel (614)*/
                else if(KEY_EQ("RobotSpawnDeactivated"))       { if (parse_bool(value,lineSpace,lineNum)) inst->questBitID = QB_RobotSpawnDeactivated; }   else if(KEY_EQ("IsotopeInstalled"))            { if (parse_bool(value,lineSpace,lineNum)) inst->questBitID = QB_IsotopeInstalled; }
                else if(KEY_EQ("ShieldActivated"))             { if (parse_bool(value,lineSpace,lineNum)) inst->questBitID = QB_ShieldActivated; }         else if(KEY_EQ("LaserSafetyOverriden"))        { if (parse_bool(value,lineSpace,lineNum)) inst->questBitID = QB_LaserSafetyOverriden; }
                else if(KEY_EQ("LaserDestroyed"))              { if (parse_bool(value,lineSpace,lineNum)) inst->questBitID = QB_LaserDestroyed; }          else if(KEY_EQ("BetaGroveCyberUnlocked"))      { if (parse_bool(value,lineSpace,lineNum)) inst->questBitID = QB_BetaGroveCyberUnlocked; }
                else if(KEY_EQ("GroveAlphaJettisonEnabled"))   { if (parse_bool(value,lineSpace,lineNum)) inst->questBitID = QB_GroveAlphaJettisonEnabled;}else if(KEY_EQ("GroveBetaJettisonEnabled"))    { if (parse_bool(value,lineSpace,lineNum)) inst->questBitID = QB_GroveBetaJettisonEnabled; }
                else if(KEY_EQ("GroveDeltaJettisonEnabled"))   { if (parse_bool(value,lineSpace,lineNum)) inst->questBitID = QB_GroveDeltaJettisonEnabled;}else if(KEY_EQ("MasterJettisonBroken"))        { if (parse_bool(value,lineSpace,lineNum)) inst->questBitID = QB_MasterJettisonBroken; }
                else if(KEY_EQ("Relay428Fixed"))               { if (parse_bool(value,lineSpace,lineNum)) inst->questBitID = QB_Relay428Fixed; }           else if(KEY_EQ("MasterJettisonEnabled"))       { if (parse_bool(value,lineSpace,lineNum)) inst->questBitID = QB_MasterJettisonEnabled; }
                else if(KEY_EQ("BetaGroveJettisoned"))         { if (parse_bool(value,lineSpace,lineNum)) inst->questBitID = QB_BetaGroveJettisoned; }     else if(KEY_EQ("AntennaNorthDestroyed"))       { if (parse_bool(value,lineSpace,lineNum)) inst->questBitID = QB_AntennaNorthDestroyed; }
                else if(KEY_EQ("AntennaSouthDestroyed"))       { if (parse_bool(value,lineSpace,lineNum)) inst->questBitID = QB_AntennaSouthDestroyed; }   else if(KEY_EQ("AntennaEastDestroyed"))        { if (parse_bool(value,lineSpace,lineNum)) inst->questBitID = QB_AntennaEastDestroyed; }
                else if(KEY_EQ("AntennaWestDestroyed"))        { if (parse_bool(value,lineSpace,lineNum)) inst->questBitID = QB_AntennaWestDestroyed; }    else if(KEY_EQ("SelfDestructActivated"))       { if (parse_bool(value,lineSpace,lineNum)) inst->questBitID = QB_SelfDestructActivated; }
                else if(KEY_EQ("BridgeSeparated"))             { if (parse_bool(value,lineSpace,lineNum)) inst->questBitID = QB_BridgeSeparated; }         else if(KEY_EQ("IsolinearChipsetInstalled"))   { if (parse_bool(value,lineSpace,lineNum)) inst->questBitID = QB_IsolinearChipsetInstalled; }
                else if(KEY_EQ("doorOpenState"))   inst->doorOpen = parse_numberu8(value, lineSpace, lineNum);     else if(KEY_EQ("FuncWall.currentState")) inst->funcState = (FuncStates)parse_numberu8(value, lineSpace, lineNum);
                else if(KEY_EQ("startPosition.x")) inst->startPosition.x = parse_float(value, lineSpace, lineNum); else if(KEY_EQ("startPosition.y")) inst->startPosition.y = parse_float(value, lineSpace, lineNum); else if(KEY_EQ("startPosition.z")) inst->startPosition.z = parse_float(value, lineSpace, lineNum);
                else if(KEY_EQ("speed"))           inst->speed = parse_float(value, lineSpace, lineNum);           else if(KEY_EQ("percentAjar"))     inst->ajarPercentage = parse_float(value, lineSpace, lineNum);
                else if(KEY_EQ("percentMoved"))    inst->percentMoved = parse_float(value, lineSpace, lineNum);    else if(KEY_EQ("chunkIDs.Length")) { fwCollecting=true; fwPendingChild=false; fwCurChild=0; fwLastChunkSlot=0; }
                else if(keyLen > 9 && sCompUpToLen(key,"chunkIDs[",9) == 0) { u16 idx=parse_numberu16(value,lineSpace,lineNum); if(fwCurChild<FW_MAX_CHILDREN){lwPrefab[fwCurChild]=idx; fwLastChunkSlot=fwCurChild; fwPendingChild=true; fwCurChild++;} }
            }
        }
        /*Sub-GO block 1 of an InteractablePanel line is installationItem: keep where a placed-item entity goes.*/
        if (ipLine && ipBlock>=2) { entsFromFile[entCount].panelItemPos=ipSubPos; entsFromFile[entCount].panelItemRot=ipSubRotSet?ipSubRot:rotationFromFile[entCount]; }/*no rotation on the sub block means the item keeps the panel's*/
        if (!isLight && !activeStateRead) flag_set(&entsFromFile[entCount].entflags,EF_ACTIVE,true); // Default active if not specified
        if (!isLight && entsFromFile[entCount].index >= 595 && entsFromFile[entCount].index <= 601) {
            V3 prefabScale;
            switch (entsFromFile[entCount].index) {
                case 595:                               prefabScale = (V3){2.56f,2.56f,2.56f}; /* trigger_cyberpush */
                case 596:                               prefabScale = (V3){1.0f,1.0f,1.0f};    /* trigger_gravitylift */
                case 597:                               prefabScale = (V3){1.0f,2.4f,0.16f};   /* trigger_ladder */
                case 598: case 599: case 600: case 601: prefabScale = (V3){2.56f,2.56f,2.56f};
                default:                                prefabScale = (V3){1.0f,1.0f,1.0f};
            }
            if (!(scaleReadMask&1u)) scaleFromFile[entCount].x=prefabScale.x;
            if (!(scaleReadMask&2u)) scaleFromFile[entCount].y=prefabScale.y;
            if (!(scaleReadMask&4u)) scaleFromFile[entCount].z=prefabScale.z;
        }
        if (!isLight && (entsFromFile[entCount].index == 592 || entsFromFile[entCount].index == 593)) entsFromFile[entCount].texIndex = matIndexRead ? matIndexTexIdx : 881; // 3D text decals: material color texture, white default
        if (!isLight && entsFromFile[entCount].index == 517) { // func_wall: stash base transform + chunk children for spawn below
            fwBasePos[entCount]=fwContainerPos; fwBaseRot[entCount]=fwContainerRot; fwBaseScale[entCount]=fwContainerScale; fwInfoLocal[entCount]=fwInfoLocalTmp; u16 n=fwCurChild < FW_MAX_CHILDREN ? fwCurChild : FW_MAX_CHILDREN, start=fwPoolUsed;
            for (u16 k=0;k<n;++k) { if (fwPoolUsed >= FW_POOL_MAX) { DualLogError("FuncWall chunk pool exhausted on line %u!\n",lineNum); break; } fwPoolPrefab[fwPoolUsed]=lwPrefab[k]; fwPoolPos[fwPoolUsed]=lwPos[k]; fwPoolRot[fwPoolUsed]=lwRot[k]; fwPoolScale[fwPoolUsed]=lwScale[k]; fwPoolUsed++; }
            fwSlotStart[entCount]=start; fwSlotCount[entCount]=(u16)(fwPoolUsed-start);
        }
    }
    i32 totalEnts = entCount + 1;
    for (i32 e=0;e<totalEnts;++e) {
        Entity* src = &entsFromFile[e]; u16 entIdx = src->index; u16 parent = AddInstance(entIdx,posFromFile[e]); Entity* par = &World.instances[parent]; par->lastPosition = posFromFile[e]; World.rotation[parent] = rotationFromFile[e]; if (!IdxIsDynamicObject(entIdx)) {World.scale[parent] = scaleFromFile[e];}
        if (IdxIsNPC(entIdx) && npcTable[entIdx-419].type != NPCType_Cyber) { V3 f = src->idealTransformForward; Quaternion r = World.rotation[parent];
            if (entIdx == 437) { float half = -(SERVBOT_MESH_YAW_DEG)*(PI/180.0f)*0.5f; Quaternion rev = {0.0f,vsinf(half),0.0f,vcosf(half)}; World.rotation[parent] = quat_multiply(rev,World.rotation[parent]); }/*npc_servbot: reverse the render-only mesh yaw (SERVBOT_MESH_YAW_DEG, voxen.c) about global Y so absolute placements are unchanged; par->forward below picks this up, staying glued to the mesh. (The yaw-recovery branch below skips 437, which is why editing there has no effect on servbots.)*/
            else if (!npcRootRotSet[e]) { if (vabs(f.y) < 0.001f && (vabs(f.x) > 0.000001f || vabs(f.z) > 0.000001f)) { float yaw = __builtin_atan2f(f.x, f.z), half = yaw * 0.5f; World.rotation[parent] = (Quaternion){0.0f, vsinf(half), 0.0f, vcosf(half)}; } }
            else if (vabs(r.x) > 0.001f || vabs(r.z) > 0.001f) World.rotation[parent] = QUAT_IDENTITY;/*explicit block-0 rotation is tilted (unobserved on non-cyber records): upright default, same as a fresh AddInstance*/
            par->forward = V3_Normalize(quat_rot_v3(World.rotation[parent], (V3){0.0f,0.0f,1.0f})); }
        bool srcActive=(src->entflags & EF_ACTIVE)!=0; par->entflags|= src->entflags;/*bitor `|` since AddInstance already set flags from entity definitions.*/
        flag_set(&par->entflags,EF_ACTIVE,srcActive);
        if (IdxIsNPC(entIdx)) { par->currentState=src->currentState; par->currentDestination=src->currentDestination; par->idealPos=src->idealPos; par->idealTransformForward=src->idealTransformForward; par->targettingPosition=src->targettingPosition; par->attack1SoundTime=src->attack1SoundTime; par->attack2SoundTime=src->attack2SoundTime; par->attack3SoundTime=src->attack3SoundTime; } par->ioflags=src->ioflags; par->ammo=src->ammo; par->ammo2=src->ammo2; par->lookUpIndex=src->lookUpIndex; par->customIndex=src->customIndex;
        for (u8 slot=0;slot<4;++slot) { par->contents[slot]=src->contents[slot]; par->custIdx[slot]=src->custIdx[slot]; } for (u8 slot=0;slot<7;++slot) { par->randomItem[slot]=src->randomItem[slot]; par->randomItemCustIdx[slot]=src->randomItemCustIdx[slot]; par->randomItemDropChance[slot]=src->randomItemDropChance[slot]; }
        par->generateContents=src->generateContents; par->generationDone=src->generationDone; par->maxRandomItems=src->maxRandomItems; par->srchInUse=src->srchInUse;
        par->amount=src->amount; par->resetTime=src->resetTime; par->minSecurityLevel=src->minSecurityLevel; par->keycode=src->keycode; par->damage=src->damage; par->delay=src->delay; par->active=src->active; par->activatedScale=src->activatedScale;
        par->forceFieldDirectionX=src->forceFieldDirectionX; par->forceFieldDirectionY=src->forceFieldDirectionY; par->forceFieldDirectionZ=src->forceFieldDirectionZ; par->fieldColor=src->fieldColor; par->timeInterval=src->timeInterval;
        par->randomMin=src->randomMin; par->randomMax=src->randomMax; par->useRandomTimes=src->useRandomTimes; if(src->health>=0.0f)par->health=src->health; if(src->cyberHealth>=0.0f)par->cyberHealth=src->cyberHealth; par->radiation=src->radiation; par->idleTime=src->idleTime;/*level data writes -1 for "unset"; copying it would stomp the table default set in AddInstance()*/
        par->timeSinceMovedEnough=src->timeSinceMovedEnough; par->tickTime=src->tickTime; par->waitBefClose=src->waitBefClose; par->onlyOnce=src->onlyOnce; par->stayOpen=src->stayOpen; par->startOpen=src->startOpen; par->ajar=src->ajar; par->ajarPercentage=src->ajarPercentage; par->timeBeforeLasersOn=src->timeBeforeLasersOn;
        par->toggleLasers=src->toggleLasers; par->changeLayerOnOpenClose=src->changeLayerOnOpenClose; par->securityThreshold=src->securityThreshold; par->messageLingdex=src->messageLingdex; par->targetIdx=src->targetIdx; par->targetOnDeathIdx=src->targetOnDeathIdx; par->target2Idx=src->target2Idx; par->camStartYAngle=src->camStartYAngle; par->camEndYAngle=src->camEndYAngle; par->camWaitTime=src->camWaitTime; par->camRotatePositive=src->camRotatePositive; par->camRotateEnabled=src->camRotateEnabled;
        if (entIdx == 592 || entIdx == 593) par->texIndex = src->texIndex; // 3D text decal texture comes from level matIndex, not entity definition
        else if (entIdx == 112 && src->texIndex > 0 && src->texIndex < MAXTEX) par->texIndex = src->texIndex; // Explicit gravity-lift visual material state
        else if ((entIdx == 21 || entIdx == 22 || entIdx == 130) && src->texIndex > 0 && src->texIndex < MAXTEX) par->texIndex = src->texIndex; // Per-instance material overrides: cyberspace panel colors (21/22) and coolant pipes (130) — the Unity material for these is carried as texIndex in the level data
        if (entIdx == 21 || entIdx == 22) par->glowIndex = par->texIndex; // Cyberspace panels are essentially unlit: the glow texture must be the panel's own (possibly tinted) texture, else the untinted glow grid overdraws the tint at full strength
        if (entIdx == 592 || entIdx == 593) { DecalPend* pd=NULL; for (u16 k=0;k<pendDecalCount;++k) { if (pendDecals[k].ent == (u16)e) { pd=&pendDecals[k]; break; } } if (pd) { i16 li = (i16)src->messageLingdex;
            if (li <= 0 || li >= T_LOGSTR_CNT) { if (decalInlineTextCount < DECAL_INLINE_TEXT_MAX) { mcpy(decalInlineText[decalInlineTextCount],pd->text,DECAL_INLINE_TEXT_LEN); decalInlineTextLevel[decalInlineTextCount]=curlevel; decalInlineTextInst[decalInlineTextCount]=parent; ++decalInlineTextCount; } else DualLogError("Too many inline text decals\n"); }
            if (pd->anchor != 0 || pd->align != 0 || pd->lineSp != 1.0f) { if (decalStyleCount < DECAL_STYLE_MAX) { decalStyles[decalStyleCount]=(DecalStyle){curlevel,parent,pd->anchor,pd->align,pd->lineSp}; ++decalStyleCount; } else DualLogError("Too many decal styles\n"); } } }
        par->panelItemPos=src->panelItemPos; par->panelItemRot=src->panelItemRot; par->targetnameIdx=src->targetnameIdx; par->targetIfFalseIdx=src->targetIfFalseIdx; par->questBitID=src->questBitID; par->lockedTargetIdx=src->lockedTargetIdx; par->deferredIoflags=src->deferredIoflags; par->deferredIoflagsHi=src->deferredIoflagsHi; par->useQuestKeycode1=src->useQuestKeycode1; par->useQuestKeycode2=src->useQuestKeycode2; par->questTestMode=src->questTestMode; par->branchOnSecond=src->branchOnSecond; par->relayEnabled=src->relayEnabled;
        par->relayOnceEver=src->relayOnceEver; par->relayAlreadyDone=src->relayAlreadyDone; par->startPosition=src->startPosition; par->targetPosition=src->targetPosition; par->funcState=src->funcState; par->speed=src->speed; par->codeScreen=src->codeScreen;
        par->gridCellsBits=src->gridCellsBits; mcpy(par->wireCurL,src->wireCurL,sizeof(par->wireCurL)); mcpy(par->wireCurR,src->wireCurR,sizeof(par->wireCurR));
        par->puzzleFired=src->puzzleFired; par->emailIndex=src->emailIndex; par->autoPlayEmail=src->autoPlayEmail; par->logIndex=src->logIndex; par->textIndex=src->textIndex; par->trackType=src->trackType; par->rechargeMsgLingdex=src->rechargeMsgLingdex;
        par->teleportID=src->teleportID; par->targetDestinationID=src->targetDestinationID; par->justUsed=src->justUsed;
        par->allDone=src->allDone; par->iceActive=src->iceActive; par->broken=src->broken; par->despawnInstead=src->despawnInstead; par->panelOpen=src->panelOpen; par->panelInstalled=src->panelInstalled; par->requireRecharge=src->requireRecharge; par->rechargeFinished=src->rechargeFinished; par->SFXLockedIndex=src->SFXLockedIndex; par->usedMsgLingdex=src->usedMsgLingdex;
        par->dir.x=src->dir.x; par->dir.y=src->dir.y; par->dir.z=src->dir.z; par->force=src->force;
        par->reverbMaxDist=src->reverbMaxDist; par->reverbPreset=src->reverbPreset; par->requiredAccessCard=src->requiredAccessCard; par->musicType=src->musicType; par->messageIndex=src->messageIndex; par->counter=src->counter; par->countToTrigger=src->countToTrigger; par->dontReset=src->dontReset; par->ioflagsHi=src->ioflagsHi; par->spawnIndex=src->spawnIndex; par->numberToSpawn=src->numberToSpawn; par->numberActive=src->numberActive; par->countOnlySameIndex=src->countOnlySameIndex; par->alertEnemiesOnAwake=src->alertEnemiesOnAwake; par->minDelayBetweenSpawns=src->minDelayBetweenSpawns; par->maxDelayBetweenSpawns=src->maxDelayBetweenSpawns; par->allSpawnedResetDelay=src->allSpawnedResetDelay; par->lockedMessageLingdex=src->lockedMessageLingdex; par->SFXIndex=src->SFXIndex; par->touchEnabled=src->touchEnabled; par->doorOpen=src->doorOpen; par->percentMoved=src->percentMoved;
        par->texAnimResourceFolderIdx = src->texAnimResourceFolderIdx;
        if (entIdx == 517) { /*func_wall: anchor at startPosition (authoritative cell center); chunk children are mover-relative*/
            V3 sp = par->startPosition;
            if (sp.x == 0.0f && sp.y == 0.0f && sp.z == 0.0f) { sp = V3_AplusB(fwBasePos[e],posFromFile[e]); par->startPosition = sp; } /*fallback for entries lacking startPosition*/
            World.position[parent] = par->lastPosition = sp;
            World.rotation[parent] = quat_multiply(fwBaseRot[e],rotationFromFile[e]);
            World.scale[parent] = (V3){fwBaseScale[e].x * scaleFromFile[e].x, fwBaseScale[e].y * scaleFromFile[e].y, fwBaseScale[e].z * scaleFromFile[e].z}; par->targetPosition = V3_AplusB(sp,fwInfoLocal[e]);/*info_target offset denotes full travel*/ if (par->speed <= 0.0f) par->speed = 0.64f;
            for(u16 k=0;k<fwSlotCount[e];++k){u16 pi=fwPoolPrefab[fwSlotStart[e]+k]; if(pi>=307)continue; u16 c=AddInstance(pi,V3_AplusB(sp,quat_rot_v3(fwBaseRot[e],fwPoolPos[fwSlotStart[e]+k]))); if(c == 0)continue; World.rotation[c]=quat_multiply(fwBaseRot[e],fwPoolRot[fwSlotStart[e]+k]); V3 cs=fwPoolScale[fwSlotStart[e]+k]; World.scale[c]=(V3){World.scale[parent].x*cs.x,World.scale[parent].y*cs.y,World.scale[parent].z*cs.z}; fwParentOf[c]=parent;}
        }
        if (IdxIsPortalBlockingDoor(entIdx)) AddDoorPortal(entIdx,parent);
        if (entIdx >= 595 && entIdx <= 601) {
            if (colSzFromFile[e].x >= 0 || colSzFromFile[e].y>=0 || colSzFromFile[e].z>=0) {
                World.colliderCenter[parent] = colCtrFromFile[e];
                World.colliderSize[parent] = (V3){colSzFromFile[e].x<0.0f ? 1.0f : colSzFromFile[e].x,colSzFromFile[e].y < 0.0f ? 1.0f : colSzFromFile[e].y, colSzFromFile[e].z < 0.0f ? 1.0f : colSzFromFile[e].z};
            } else{
                World.colliderCenter[parent] = (V3){0.0f, 0.0f, 0.0f};
                World.colliderSize[parent] = (V3){1.0f, 1.0f, 1.0f};
            }
        } else if (entIdx == 706) { /* info_elev_destination */
            if (colSzFromFile[e].x >= 0 || colSzFromFile[e].y >= 0 || colSzFromFile[e].z >= 0) {
                World.colliderCenter[parent] = colCtrFromFile[e];
                World.colliderSize[parent] = (V3){colSzFromFile[e].x < 0.0f ? 2.56f : colSzFromFile[e].x, colSzFromFile[e].y < 0.0f ? 2.56f : colSzFromFile[e].y, colSzFromFile[e].z < 0.0f ? 2.56f : colSzFromFile[e].z};
            } else {
                World.colliderCenter[parent] = (V3){0.013999939f, 0.20199966f, 0.03299904f};
                World.colliderSize[parent] = (V3){2.56f, 2.56f, 2.56f};
            }
        } else if (entIdx == 697 || entIdx == 698) {
            if (colSzFromFile[e].x >= 0.0f || colSzFromFile[e].y >= 0.0f || colSzFromFile[e].z >= 0.0f) {
                World.colliderCenter[parent]=colCtrFromFile[e];
                World.colliderSize[parent]=(V3){colSzFromFile[e].x < 0.0f ? 2.56f : colSzFromFile[e].x, colSzFromFile[e].y < 0.0f ? 2.56f : colSzFromFile[e].y, colSzFromFile[e].z < 0.0f ? 2.56f : colSzFromFile[e].z};
            } else {
                World.colliderCenter[parent] = (V3){0.0f, 0.0f, 0.0f};
                World.colliderSize[parent] = (V3){2.56f, 2.56f, 2.56f};
            }
        } else if (entIdx == 716) { World.col[parent] = CT_NONE; }
        else if (entIdx == 515 && EDefs[entIdx].col == CT_BOX && EDefs[entIdx].colSz.x == 0.0f && EDefs[entIdx].colSz.y == 0.0f && EDefs[entIdx].colSz.z == 0.0f) { World.colliderCenter[parent] = (V3){0.0f,0.0f,0.0f}; World.colliderSize[parent] = (V3){1.0f,1.0f,1.0f}; }
        if (entIdx == 700) par->currentTargetIdx = par->branchOnSecond ? par->target2Idx : par->targetIdx;
        { const CgOffset* co = CgOffsetFor(entIdx); if (co) World.position[parent] = V3_AplusB(World.position[parent], co->offset); }/*moved out of the chain below: none of these constIndexes collide with the cases that remain, so an add-then-switch is the same as the else-if it replaces*/
        if (entIdx == 525) { par->texAnimLight=AddOffsetLight(par,(V3){5.81f,2.29f,38.05f-38.3552f},(Color3){0.3531f,0.4837f,0.6509f},1.85f,0.7f); par->texAnimLight2=AddOffsetLight(par,(V3){-10.1f,0.9f,18.21f-38.3552f},(Color3){0.3561f,0.3561f,0.8970f},2.0f,1.12f); } // prop_console01
        else if (entIdx == 279) { par->texAnimLight = AddOffsetLight(par,(V3){0.0f,-0.08f,0.0f},(Color3){0.909803922f,0.929411765f,1.0f},3.2f,1.575f); } // chunk_screen
        else if (par->index == 574) { // prop_healingbed
            Color3 green = {0.0f, 0.925490196f, 0.082352941f}; par->texAnimLight=AddOffsetLight(par,(V3){0.5292511f,0.065f,0.915f},green,3.0f,0.72f); par->texAnimLight2=AddOffsetLight(par,(V3){-0.5317497f,0.065f,1.039f},green,3.0f,0.72f); par->textureAnimating=true; par->texAnimClip=12; par->texFrame=0;
            par->texAnimResourceFolderIdx = TexAnimFolderIntern("MedicalBed");
        } else if (entIdx == 345) { World.scale[parent].x=World.scale[parent].y=World.scale[parent].z=1.00f; } // weapon_dartgun
        else if (entIdx == 586 || (entIdx >= 540 && entIdx <= 544)) { World.scale[parent].x=World.scale[parent].y=World.scale[parent].z=1.0f; } // prop_table, prop_chair01-05: map exports 0.04 scale; force 1.0
        else if (entIdx == 218 || entIdx == 220) { Quaternion c = quat_from_axis_angle((V3){0.0f,1.0f,0.0f},PI); World.rotation[parent] = quat_multiply(quat_multiply(c,World.rotation[parent]),(Quaternion){-c.x,-c.y,-c.z,c.w}); } // chunk_reac2_4, chunk_reac2_5: the export is a flipped basis, not a turned instance, so conjugate by 180deg about Y (which flips the mesh's local X and Z and leaves Y). Conjugation spins the rotation's axis while preserving its angle, so the level file's authored placement is untouched. Checked against level 7 instance 4180: lR (0,.7071,.7071,0) -> (0,-.7071,.7071,0) exactly, and it is non-identity on all 6 (218) / 4 (220) orientations these prefabs use, so every instance gets corrected.
        else if (entIdx == 53) { World.rotation[parent] = quat_multiply(World.rotation[parent], quat_from_axis_angle((V3){1.0f,0.0f,0.0f},PI*0.5f)); }
        else if (entIdx == 551 && par->codeScreen) { World.rotation[parent] = quat_multiply(World.rotation[parent], (Quaternion){0.0f,0.70710678f,0.70710678f,0.0f}); World.scale[parent].x*=0.390625f; World.scale[parent].y*=0.390625f; World.scale[parent].z*=0.390625f; }
        else if (par->index == 746) { par->textureAnimating = true; par->texAnimClip = 2; par->texFrame = 0; } // weapon_grenadeenergmine_live
        else if (entIdx == 720) { /*u16 mist=*/AddInstance(648,World.position[parent]); }// ambient_mist
        else if (entIdx == 733) { /*u16 pipewater=*/AddInstance(649,World.position[parent]);/*ambient_pipewater_loop*/ /*u16 rain=*/AddInstance(653,(V3){World.position[parent].x,World.position[parent].y - 1.26f,World.position[parent].z});/*ambient_rain*/ }
        else if (entIdx == 305) { par->texAnimResourceFolderIdx = TexAnimFolderIntern("Telepad"); } /*chunk_teleporter*/
        else if (entIdx == 2 || entIdx == 3) { par->texAnimResourceFolderIdx = TexAnimFolderIntern("Bridge11"); } /*chunk_bridg1_1, chunk_bridg1_1flipx*/
        else if (entIdx == 386) { par->texAnimResourceFolderIdx = TexAnimFolderIntern("MagCartridge"); } /*item_ammo_magcart*/
        else if (entIdx == 538) { par->texAnimResourceFolderIdx = TexAnimFolderIntern("BrokenClock"); } /*prop_broken_clock*/
        else if (entIdx == 551 && par->codeScreen) { par->texAnimResourceFolderIdx = TexAnimFolderIntern("ScreenCodeRandom"); } /*prop_cpuscreen*/
        TextureSequenceInit(parent,texAnimFolders[par->texAnimResourceFolderIdx < texAnimFolderCount ? par->texAnimResourceFolderIdx : 0]);
        if (entIdx == 551 && par->codeScreen) { par->texAnimRandom = true; par->texAnimStopsAtDie = false; }
    }
    for (int i=0;i<=lightsIdx;++i) { if (lightsFromFile[i].intensity < 0.11f && lightsFromFile[i].maxIntensity < 0.11f){continue;} lightsFromFile[i].range = vclamp(lightsFromFile[i].range,0.32f,15.36f); if (lanimsFromFile[i].numIntervalSteps >= 1) { lanimsFromFile[i].currentStep = 0; lanimsFromFile[i].lerpUp = true; if (lightsFromFile[i].minIntensity < 0.01f) lightsFromFile[i].minIntensity = 0.01f; }/*Unity LightAnimation.Start() init parity: animations always begin at step 0, lerping up, with a nonzero floor*/ i32 wi=AddLight(&lightsFromFile[i],&lanimsFromFile[i]); if(wi>=0&&wi<LIGHT_COUNT){World.levelLightTargetnames[World.currentLevel][wi]=lightTargetnameFile[i];} }
    if(curlevel == 1 || curlevel == 2 || curlevel == 5 || curlevel == 6 || curlevel == 7){/*Shield generators*/
        World.shd1=AddInstance(754,(V3){-51.30664f,-47.42f,56.42651f}); World.shd2=AddInstance(754,(V3){71.5f,-47.42f,-66.6f}); World.shd3=AddInstance(754,(V3){-51.306650f,-47.42f,-66.66652f}); World.shd4=AddInstance(754,(V3){71.78664f,-47.42f,56.42651f}); World.rotation[World.shd1] = World.rotation[World.shd2] = World.rotation[World.shd3] = World.rotation[World.shd4] = QUAT_IDENTITY;
    } else World.shd1=World.shd2=World.shd3=World.shd4=U16_MAX;
    headmountedLanternLight = AddLightSimple(World.position[PLAYER1],(Color3){1.0f,1.0f,1.0f},11.52f,0.0f,LIGHTON);
    if (World.instCount < INSTANCE_COUNT) {
        u16 wvi = World.instCount; mset(&World.instances[wvi],0,sizeof(Entity)); World.instances[wvi].entflags=EF_ACTIVE|EF_NO_SHADOWS; World.layer[wvi]=L_Default; World.instances[wvi].camView=255; World.instances[wvi].modelIndex=World.instances[wvi].lodIndex=World.instances[wvi].colMeshIndex=MAX_MDLS;
        World.instances[wvi].texIndex=World.instances[wvi].glowIndex=World.instances[wvi].specIndex=World.instances[wvi].normIndex=MAXTEX; World.scale[wvi].x=World.scale[wvi].y=World.scale[wvi].z=World.mass[wvi]=World.rotation[wvi].w=1.0f; World.dynamicFriction[wvi]=0.5f; World.staticFriction[wvi]=0.6f;
        World.col[wvi]=CT_NONE; flag_set(&World.instances[wvi].entflags,EF_RIGIDBODY,false);
        World.instances[wvi].index=0; World.position[wvi]=World.position[PLAYER1]; World.rotation[wvi]=QUAT_IDENTITY; World.instances[wvi].modelIndex=MAX_MDLS; World.instances[wvi].animationNum=MAXANIM; World.weaponVModelIndex=wvi; World.instCount++;
    }
    if (World.instCount < INSTANCE_COUNT) {
        u16 editTextIdx = World.instCount; mset(&World.instances[editTextIdx],0,sizeof(Entity)); World.instances[editTextIdx].entflags=EF_ACTIVE; World.layer[editTextIdx]=L_Default; World.instances[editTextIdx].camView=255; World.instances[editTextIdx].modelIndex=World.instances[editTextIdx].lodIndex=World.instances[editTextIdx].colMeshIndex=MAX_MDLS;
        World.instances[editTextIdx].index=592; World.position[editTextIdx]=(V3){0.0f,0.0f,0.0f}; World.rotation[editTextIdx]=QUAT_IDENTITY; World.scale[editTextIdx].x=World.scale[editTextIdx].y=World.scale[editTextIdx].z=World.mass[editTextIdx]=World.rotation[editTextIdx].w=1.0f;
        World.instances[editTextIdx].messageLingdex = 0;
        World.instances[editTextIdx].texIndex = 881;
        World.editTextInstanceIndex = editTextIdx; World.instCount++;
    }
}
#undef KEY_EQ
void ButtonSwitchInitAfterLoad(u16), GrenadeInit(u16), FuncWallShiftChildren(u16,V3);
void func_forcebridge(u16 i) {
    World.instances[i].tickFinished = World.pauseRelativeTime + 0.05f + (double)random_range(0.0f,1.0f);
    World.instances[i].lerping = true;
    if (World.instances[i].activatedScale.x <= 0.02f) { World.instances[i].activatedScale.x = 2.56f; }
    if (World.instances[i].activatedScale.y <= 0.02f) { World.instances[i].activatedScale.y = 0.08f; }
    if (World.instances[i].activatedScale.z <= 0.02f) { World.instances[i].activatedScale.z = 2.56f; }
    if (!World.instances[i].active) { World.instances[i].modelIndex = MAX_MDLS; World.col[i] = CT_NONE; }
    switch (World.instances[i].fieldColor) {
        case ForceFieldColor_Red:      World.instances[i].texIndex =  38; break;
        case ForceFieldColor_Green:    World.instances[i].texIndex =  40; break;
        case ForceFieldColor_Blue:     World.instances[i].texIndex =  39; break;
        case ForceFieldColor_Purple:   World.instances[i].texIndex =  41; break;
        case ForceFieldColor_RedFaint: World.instances[i].texIndex = 198; break;
    }
}

void FuncWallInitAfterLoad(u16 i) {
    V3 prev=World.position[i];
    float distTotal = V3_Dist(World.instances[i].startPosition,World.instances[i].targetPosition), f = 0;
         if (World.instances[i].funcState  > FStat_AjarMovingTarget) { f = World.instances[i].ajarPercentage; }
    else if (World.instances[i].funcState == FStat_AjarMovingTarget) { f = World.instances[i].ajarPercentage; }
    else if (World.instances[i].funcState == FStat_AjarMovingStart ) { f = 1.0f - World.instances[i].ajarPercentage; }
    
    f = vclamp(f,0.0f,1.0f);
    V3 np = (distTotal > 0.0001f) ? V3_AplusB(World.instances[i].startPosition,V3_ScaleByF(V3_Normalize(V3_AsubB(World.instances[i].targetPosition,World.instances[i].startPosition)),distTotal * f)) : World.instances[i].startPosition; World.position[i]=np;
    if ((u8)World.instances[i].funcState <= FStat_MovingTarget) { World.instances[i].funcState = FStat_Start; World.instances[i].percentMoved = 0.0f; }
    FuncWallShiftChildren(i,V3_AsubB(np,prev));
}

void LogicTimerInitBeforeLoad(u16 i) {
    if (World.instances[i].timeInterval <= 0.0f) { World.instances[i].timeInterval = 0.35f; }
    if (World.instances[i].randomMin <= 0.0f) { World.instances[i].randomMin = 5.0f; }
    if (World.instances[i].randomMax <= 0.0f) { World.instances[i].randomMax = 10.0f; }
    World.instances[i].intervalFinished = World.pauseRelativeTime + (World.instances[i].useRandomTimes ? (double)random_range(World.instances[i].randomMin,World.instances[i].randomMax) : (double)World.instances[i].timeInterval);
}

float DoorClamp01(float v) { if (v < 0.0f) return 0.0f; if (v > 1.0f) return 1.0f; return v; }
AnimationClip DoorGetClip(const Entity* e, u8 clip) { return mdlAnimClips[e->animationNum][clip]; }
u16 DoorFrameFromProgress(AnimationClip c, float t) { if(c.frameEnd <= c.frameStart){return c.frameStart;} u16 span = c.frameEnd - c.frameStart; return (u16)(c.frameStart + (u16)(DoorClamp01(t) * (float)span + 0.5f)); } // round, don't truncate: t*span truncating loses up to a full frame, so a stroke resumed at 70% lands near 22% instead of 30%. Clamped t <= 1 keeps the +0.5 from spilling past span.
void ComputeConvexMeshInertiaTensor(u16); void CyberMineInitBeforeLoad(u16);
void LoadLevelData(u8 curlevel) {
    if(!teleportDestinationsInitialized){for(u8 l=0;l<MAX_LEVELS;++l)for(u8 id=0;id<8;++id)teleportDestinations[l][id]=U16_MAX;teleportDestinationsInitialized=true;}
    if(curlevel<MAX_LEVELS)for(u8 id=0;id<8;++id)teleportDestinations[curlevel][id]=U16_MAX;
    if(curlevel<MAX_LEVELS)elevatorDestCounts[curlevel]=0;
    World.curLev = curlevel; TargetIDReset(); ai_reset_npc_numbering(); SetLevelPointers(curlevel); World.invP1.makingNoise=false;/*clear noise at level loads*/ World.decoyActive=false; World.decoyInstance=U16_MAX;/*a decoy is spawned into one level and dies with it; a stale true would leave cyber NPCs retargeting a dead instance index*/
    mset(World.instances + 3,0,(INSTANCE_COUNT - 3) * sizeof(Entity)); World.instCount = 3; mset(World.lights,0,LIGHT_COUNT * sizeof(Light)); mset(World.lanims,0,LANIM_MAX * sizeof(LightAnimation)); mset(World.lanimLightIdx,0,LANIM_MAX * sizeof(u16)); World.lanimCount=0; World.loadedLights=0; mset(alreadyReadLightOnOnce,0,sizeof(alreadyReadLightOnOnce));
    mset(camViews,0,64 * sizeof(CamView)); camViewCount=0; char filename[20]; sFormat(filename, sizeof(filename), "./Data/level%d.txt", curlevel); FHandle fh; int fsize; void* fbuf = OS_OpenAndAllocateFileBufferReadonly(filename, &fh, &fsize); if (!fbuf) { OS_Exit(1); } mm_ptr = (const char*)fbuf; mm_end = mm_ptr + fsize; mset(fwParentOf,0,sizeof(fwParentOf)); LoadLevelMod(curlevel); GravityLiftSyncAllVisuals(); PSysAddLevelLoops(); if (curlevel<MAX_LEVELS) { mcpy(fwParentSnap[curlevel],fwParentOf,sizeof(fwParentOf)); fwSnapValid[curlevel]=true; } OS_Free(fbuf,(size_t)fsize);
    for (int i = 0; i < World.loadedLights; ++i) World.lightsNewPosition[i] = World.lights[i].pos;
    for (int i = PLAYER1; i < World.instCount; ++i) {
        i32 cellIdx = PosGetCellCoords(World.position[i].x, World.position[i].z); World.instances[i].cellIndex = cellIdx; World.instances[i].cellX = PosGetCellCoordX(World.position[i].x); World.instances[i].cellZ = PosGetCellCoordZ(World.position[i].z);
        u16 mi = World.instances[i].modelIndex; World.radius[i] = ((mi < mdlsCnt && modelBounds[mi] > 0.0f) ? modelBounds[mi] : 2.56f) * vmax(vmax(World.scale[i].x,World.scale[i].y),World.scale[i].z); World.instances[i].shadRadius = World.radius[i] * 2.00f;
        ComputeConvexMeshInertiaTensor(i); if (World.mass[i] < 0.001f && World.col[i] != CT_NONE && World.col[i] != CT_MSH && (World.instances[i].entflags & EF_RIGIDBODY)) { World.mass[i]=0.2f;/*At least something!*/ }
    }
    for (int i=PLAYER1;i<World.instCount;++i) {
        u16 constIndex = World.instances[i].index;
        if (i == PLAYER1 || IdxIsDynamicObject(constIndex) || (IdxIsNPC(constIndex) && constIndex < 443/*not cyber*/)){World.gravity[i] = 1.0f;}else{World.gravity[i] = 0.0f;}
        if (IdxIsGeometry(constIndex)) World.layer[i] = L_Geometry;
        else if (IdxIsUsableObject(constIndex)) {
            if (World.diffPuz == 3 && World.instances[i].index == 361 && random_range(0.0f,1.0f) < 0.33f) DeleteInstance(i); // 33% chance of not spawning logic probes on Puzzle difficulty of 3
        } else if (IdxIsDoor(World.instances[i].index)) {
            World.layer[i] = L_Door;
            if (World.instances[i].startOpen) World.instances[i].stayOpen = true;
            if (World.instances[i].lockedMessageLingdex <= 0) World.instances[i].lockedMessageLingdex = 3; static const u16 doorSFXIndex[19]={75,75,71,72,73,75,71,73,71,70,70,75,75,72,75,75,75,75,75};
            World.instances[i].SFXIndex = doorSFXIndex[World.instances[i].index-496];
            if (World.instances[i].doorOpen > DoorState_Opening) World.instances[i].doorOpen = World.instances[i].startOpen ? DoorState_Open : DoorState_Closed;
            World.instances[i].doorState = World.instances[i].doorOpen;
            if (World.instances[i].ajar) {
                AnimationClip c = DoorGetClip(&World.instances[i],A_OPENING); u16 f = DoorFrameFromProgress(c,World.instances[i].ajarPercentage);
                World.instances[i].clip = A_OPENING; World.instances[i].frame = f; World.instances[i].modelIndex = c.frameStartModelIndex + (u16)(f - c.frameStart); World.instances[i].currentFrameFinished = 0.0; // Parked partway; UpdateAnims() holds ajar doors so it stays put.
                World.instances[i].doorOpen = World.instances[i].doorState = DoorState_Opening;
            } else { switch(World.instances[i].doorOpen){case DoorState_Open:ChangeAnim(&World.instances[i],A_IDLE_OPEN); break; case DoorState_Opening:ChangeAnim(&World.instances[i],A_OPENING); break; case DoorState_Closing:ChangeAnim(&World.instances[i],A_CLOSING); break; default:ChangeAnim(&World.instances[i],A_IDLE_CLOSED); break;} }
            if (IdxIsPortalBlockingDoor(World.instances[i].index) && World.instances[i].doorState != DoorState_Closed) ForceDoorPortalOpen(World.instances[i].portalIndex); // Open/ajar doors start with their portal open. Bounds-checked in culling.c; guard keeps non-portal doors from touching portal 0.
        } else if (IdxIsSearchable(constIndex) || IdxIsNPC(constIndex)) {
            if (World.instances[i].generateContents && !World.instances[i].generationDone) {
                int numRandomGeneratedItems = 0;
                for(int j=0;j<7;j++) {
                    if(numRandomGeneratedItems >= World.instances[i].maxRandomItems){break;} if(World.instances[i].randomItemDropChance[j] <= 0.0f){continue;} u8 tempInt = random_range_u8(0,100); if((float)tempInt <= World.instances[i].randomItemDropChance[j]){World.instances[i].contents[numRandomGeneratedItems] = World.instances[i].randomItem[j]; World.instances[i].custIdx[numRandomGeneratedItems] = World.instances[i].randomItemCustIdx[j]; numRandomGeneratedItems++;}
                }
                World.instances[i].generationDone = true;
            }
        } else switch (constIndex) {
            case 21: case 22: World.instances[i].tickFinished = World.pauseRelativeTime + 2.0; World.instances[i].animSwapFinished = 0.0; break; // chunk_cyberpanel, chunk_cyberpanel_slice45
            case 515: func_forcebridge(i); break;
            case 716: World.layer[i] = L_Trigger; break; // fx_reverbzone
            case 517: FuncWallInitAfterLoad(i); break;
            case 596: World.instances[i].strength=UsableOrDef(World.instances[i].strength,12.0f); World.instances[i].offStrengthFactor=UsableOrDef(World.instances[i].offStrengthFactor,3.0f); World.instances[i].distancePaddingToTopPoint=UsableOrDef(World.instances[i].distancePaddingToTopPoint,0.32f); break; /*trigger_gravitylift*/
            case 701: LogicTimerInitBeforeLoad(i); break;
            case 703: World.layer[i]=L_Trigger;if(World.instances[i].teleportID < 8){teleportDestinations[curlevel][World.instances[i].teleportID]=i;World.TeleportTouch_allTeleportTouches[World.instances[i].teleportID]=i;} else {DeleteInstance(i);} break;/*info_teleport_destination*/
            case 706: if(curlevel<MAX_LEVELS&&elevatorDestCounts[curlevel]<MAX_ELEV_DESTS_PER_LEVEL)elevatorDestinations[curlevel][elevatorDestCounts[curlevel]++]=i; break;
            case 555: CyberSwitchInitAfterLoad(i); break; // prop_cyber_switch
            case 480: CyberMineInitBeforeLoad(i); break;
            case 402: World.layer[i] = L_NPC; GrenadeInit(i); break;
            default: if (IdxIsButtonSwitch(constIndex)) ButtonSwitchInitAfterLoad(i); break;/*688..692, 694, 695: disjoint from every case above, so folding this into default preserves the old chain's order*/
        }
        if (World.instances[i].targetnameIdx != IO_NONE && (World.instances[i].ioflags & TARG_IOFLAGS_DISABLE_ON_AWAKE)){flag_set(&World.instances[i].entflags,EF_ACTIVE,false);}
    }
    for (int i=PLAYER1;i<World.instCount;++i){ u16 mi=World.instances[i].messageIndex; World.instances[i].messageIndex=(mi>0&&mi<T_LOGSTR_CNT)?mi:427; mi=World.instances[i].messageLingdex; World.instances[i].messageLingdex=(mi>0&&mi<T_LOGSTR_CNT)?mi:427; mi=World.instances[i].lockedMessageLingdex; World.instances[i].lockedMessageLingdex=(mi>0&&mi<T_LOGSTR_CNT)?mi:427; } // Using blank 427
    World.levelLoadedLights[curlevel] = World.loadedLights; World.levelWepVModel[curlevel]=World.weaponVModelIndex; mcpy(levelCamViews[curlevel],camViews,64 * sizeof(CamView)); mcpy(levelCamViewTextures[curlevel],camViewTextures,64 * sizeof(u32)); levelCamViewCount[curlevel] = camViewCount; World.levelInstCount[curlevel] = World.instCount; World.levelCurrentlyLoading = false; // Coppy the counts over
}

u8 GetCurrentLevelSecurity() { return (World.diffMis < 1 || Cheats.superoverride) ? 0u : World.levelSecurity[World.curLev]; }
void NewGameDifficultyPass(void) {
    for (u8 lev = 0; lev < World.numLevels; ++lev) {
        u8 entryLevel = World.currentLevel; if (entryLevel != lev) SetLevelPointers(lev);
        for (u16 i = INSTS_1ST_IDX; i < World.instCount; ++i) {
            if (!IdxIsUsableObject(World.instances[i].index)) continue;
            u16 ci = World.instances[i].index; bool del = false;
            if (World.diffMis <= 1 && (ci == 341 || ci == 388 || (ci >= 390 && ci <= 398) || ci == 417)) del = true;
            if (World.diffMis == 0 && (ci == 313 || ci == 448)) del = true;
            if (del) DeleteInstance(i);
        }
        if (World.currentLevel != entryLevel) SetLevelPointers(entryLevel);
    }
}
void RenderLoading(const char* restrict); void ResetLevelAudio(); void CullInit(); void mp3_clear();
static void DoorTypeAudit(u8 lev) {
    u32 badLayer=0, badLaser=0, badAperture=0;
    for (u16 i=PLAYER1;i<World.instCount;++i) {
        Entity* e=&World.instances[i]; if (!IdxIsDoor(e->index)) continue;
        if ((e->changeLayerOnOpenClose!=0) != IdxIsLayerDoor(e->index)) badLayer++;
        if ((e->toggleLasers!=0) != IdxIsLaserDoor(e->index)) badLaser++;
        if (IdxIsLaserDoor(e->index) && e->modelIndex < MAX_MDLS && (modelMax[e->modelIndex].x - modelMin[e->modelIndex].x) <= 0.0f) badAperture++;
    }
    if (badLayer||badLaser) DualLogError("Door type audit level %u: %u changeLayerOnOpenClose and %u toggleLasers records disagree with their prefab\n",lev,badLayer,badLaser);
    if (badAperture) DualLogError("Door laser audit level %u: %u laser doors have an empty model AABB, so their grid would never draw\n",lev,badAperture);
}
void LoadAllLevels() {
    double start_time = get_time();
    DebugRAM("start of LoadAllLevels");
    RenderLoading("Loading level data...");
    World.levelCurrentlyLoading = true;
    entsFromFile=OS_Alloc(INSTANCE_COUNT*sizeof(Entity));
    posFromFile=OS_Alloc(INSTANCE_COUNT*sizeof(V3));
    scaleFromFile=OS_Alloc(INSTANCE_COUNT*sizeof(V3));
    rotationFromFile=OS_Alloc(INSTANCE_COUNT*sizeof(Quaternion));
    colCtrFromFile=OS_Alloc(INSTANCE_COUNT*sizeof(V3));
    colSzFromFile=OS_Alloc(INSTANCE_COUNT*sizeof(V3));
    ioNameCount=1; ioNames[0][0]='\0';
    lightsFromFile=OS_Alloc(LIGHT_COUNT*sizeof(Light));
    lanimsFromFile=OS_Alloc(LIGHT_COUNT*sizeof(LightAnimation));
    for(u8 i=0;i<8;++i){World.TeleportTouch_allTeleportTouches[i]=U16_MAX;}
    fwPoolUsed = 0;
    for(u8 lev=0;lev<World.numLevels;++lev){
        for (i32 i=0;i<INSTANCE_COUNT;++i) { colCtrFromFile[i]=(V3){-1,-1,-1}; colSzFromFile[i]=(V3){-1,-1,-1}; }
        LoadLevelData(lev);
        DoorTypeAudit(lev);
    }
    
    OS_Free(entsFromFile,INSTANCE_COUNT*sizeof(Entity));
    OS_Free(colCtrFromFile,INSTANCE_COUNT*sizeof(V3));
    OS_Free(colSzFromFile,INSTANCE_COUNT*sizeof(V3));
    OS_Free(posFromFile,INSTANCE_COUNT*sizeof(V3));
    OS_Free(scaleFromFile,INSTANCE_COUNT*sizeof(V3));
    OS_Free(rotationFromFile,INSTANCE_COUNT*sizeof(Quaternion));
    OS_Free(lightsFromFile,LIGHT_COUNT*sizeof(Light));
    OS_Free(lanimsFromFile,LIGHT_COUNT*sizeof(LightAnimation));
    BuildTextDecalMeshes(); // Build world-baked 3D text decal meshes (Sys_Text.stringTable already populated for current language).
    DebugRAM("end of LoadAllLevels");
    DualLog("load all levels.................................................took %.6f secs\n",get_time() - start_time);
}

void CyberTimerReset(u16,int); /*citadel.c: Unity CyberTimer.Reset(difficultyCyber)*/
void LoadLevel(u8 curlevel, V3 pos) {
    u8 prevLevel = World.currentLevel;
    if (curlevel == LEVEL_CYBERSPACE && World.currentLevel != LEVEL_CYBERSPACE && World.curLev != LEVEL_CYBERSPACE) { World.cyberReturnLevel = (i8)World.currentLevel; World.cyberReturnPos = World.position[PLAYER1]; }/*ExitCyberspace returns here*/
    CloseSearch();
    World.Sys_UI.tetheredKeypadKeycode=World.Sys_UI.tetheredKeypadElevator=World.Sys_UI.tetheredPGP=World.Sys_UI.tetheredPWP=U16_MAX;/*panel tethers are per-level; the entity index they point at dies with the level, so don't sever against a stale one*/
    DebugRAM("start of LoadLevel");
    World.levelCurrentlyLoading = true;
    World.paused = World.menuActive = false;
    RenderLoading("Loading level...");
    if (World.currentLevel != curlevel) {
        CopyPlayerState(World.currentLevel,curlevel);
        PSysClearLevel(World.currentLevel);
        World.curLev = curlevel;
        SetLevelPointers(curlevel);
    }
    
    if (World.particles->count == 0) { PSysAddLevelLoops(); }
    
    mcpy(camViews, levelCamViews[curlevel],64 * sizeof(CamView));
    mcpy(camViewTextures, levelCamViewTextures[curlevel], 64 * sizeof(u32));
    camViewCount = levelCamViewCount[curlevel];
    for (int i = 0; i < camViewCount; ++i) { /*Initialize missing level camview entries from file data*/
        if (levelCamViews[curlevel][i].visible == false && camViews[i].visible == true) {
            mcpy(&levelCamViews[curlevel][i], &camViews[i], sizeof(CamView));
            levelCamViewTextures[curlevel][i] = camViewTextures[i];
        }
    }
    
    mset(alreadyReadLightOnOnce, 0, sizeof(alreadyReadLightOnOnce));
    for (int i=0;i<World.loadedLights;++i) { World.lightsNewPosition[i]=World.lights[i].pos; }
    ResetLevelAudio();
    mp3_clear();
    World.Sys_Music.levelEntry = true;
    World.Sys_Music.inZone = World.Sys_Music.cyberTube = false;
    World.Sys_Music.combatImpulseFinished = get_time();
    World.Sys_Music.combatImpulseFinished += 5.0;
    RenderLoading("Loading cull system...");
    CullInit(); // Must be after level!
    glUseProgram(voxelUpdateSP);
    glUniform2f(0, World.voxMinCtrX[World.curLev], World.voxMinCtrZ[World.curLev]);
    glUniform1f(1, World.farPlane[World.curLev] * World.farPlane[World.curLev]);
    glUniform1ui(2, World.loadedLights);
    glUniform2f(3, World.worldMin_x[World.curLev], World.worldMin_z[World.curLev]);
    glUniform1ui(4, SHADOW_MAP_SIZE);
    glUniform1ui(6, (u32)MAX_LIGHTS_PER_VOXEL);
    glUniform1ui(7, SHADOW_MAP_SIZE * SHADOW_MAP_SIZE);
    RenderLoading("Loading voxel lighting data...");
    for (u16 i=0;i<World.loadedLights;++i) { World.lightsNewPosition[i] = World.lights[i].pos; }
    mset(shadowmapIndirectionList, MAX_SHADOWMAPS + 1, World.loadedLights * sizeof(u32));
    for (u16 i=INSTS_1ST_IDX;i<World.instCount;++i) {
        Entity* e = &World.instances[i];
        if (IdxIsNPC(e->index) && e->enemy == PLAYER1) { e->enemy = WORLD; /*level entry: NPCs drop the player, so nothing re-aggros from the level they came from*/ }
        
        if (World.diffCbt == 0 && IdxIsNPC(e->index)) { /*Combat difficulty 0 == one hit kill; corpse-only NPCs need to stay dead.*/
            if (e->entflags & EF_ACT_AS_CORPSE_ONLY) {
                e->health = 0.0f;
                e->cyberHealth = 0.0f;
            } else {
                e->health = 1.0f;
                if (npcTable[e->index - 419].type == NPCType_Cyber) { e->cyberHealth = 1.0f; }
            }
        }
    }
    
    for (u16 i = INSTS_1ST_IDX; i < World.instCount; ++i) {
        Entity* e = &World.instances[i];
        if (e->index != 702) continue;
        
        u16 n = e->numberToSpawn;
        if (World.diffCbt == 1) { n = (u16)(n / 2); }
        else if (World.diffCbt == 3) { n = (u16)(n + n / 2); }
        else if (World.diffCbt > 3) { n = (u16)(n * 5); }
        
        e->numberToSpawn = (n < 1) ? 1 : n;
        e->delayFinished = World.pauseRelativeTime;
    }
    
    World.levelCurrentlyLoading = false;
    World.position[PLAYER1] = pos;
    if (curlevel == LEVEL_CYBERSPACE) World.cyberspaceRecallPoint = World.position[PLAYER1];/*Recall returns to the cyberspace entry point*/
    World.velocity[PLAYER1] = (V3){0.0f, 0.0f, 0.0f};
    World.invP1.lastVelY = 0.0f;
    World.invP1.wasGrounded = true;
    World.invP1.ladderState = 0;
    World.invP1.noiseFinished=World.pauseRelativeTime - 1.0;
    if (curlevel == LEVEL_CYBERSPACE) {
        World.col[PLAYER1] = CT_SPH; World.colliderSize[PLAYER1] = (V3){PLYR_RAD,PLYR_RAD,PLYR_RAD}; World.colliderCenter[PLAYER1] = (V3){0.0f,0.0f,0.0f}; World.radius[PLAYER1] = PLYR_RAD; World.gravity[PLAYER1] = 0.0f;
        if (prevLevel != LEVEL_CYBERSPACE) {
            CyberTimerReset(PLAYER1,World.diffCyb);
            World.instances[PLAYER1].timerFinished = World.pauseRelativeTime + 1.0;
            World.instances[PLAYER1].minutes = vfloor(World.instances[PLAYER1].cyberTimer / 60.0f);
            World.instances[PLAYER1].seconds = World.instances[PLAYER1].cyberTimer - (World.instances[PLAYER1].minutes * 60.0f);
        }
    } else {
        World.col[PLAYER1] = CT_CAP;
        World.colliderSize[PLAYER1] = (V3){PLYR_RAD,PLYR_HT * World.invP1.currentCrouchRatio,1.0f};
        World.colliderCenter[PLAYER1].y = -PLYR_CAM_OFS_Y + (PLYR_HT - World.colliderSize[PLAYER1].y) * 0.5f;
        World.gravity[PLAYER1] = 1.0f;
        if (prevLevel == LEVEL_CYBERSPACE) {
            V3 cyberFwd = quat_rot_v3(World.rotation[PLAYER1],(V3){0.0f,0.0f,1.0f});
            float yawDeg = __builtin_atan2f(cyberFwd.x,cyberFwd.z) * (180.0f / 3.14159265f);
            float halfYaw = deg2rad(yawDeg) * 0.5f;
            World.rotation[PLAYER1] = (Quaternion){0.0f,vsinf(halfYaw),0.0f,vcosf(halfYaw)};
            World.cam_yaw = yawDeg < 0.0f ? yawDeg + 360.0f : yawDeg;
            World.cam_pitch = World.cam_roll = 0.0f; World.invP1.leanTarget = 0.0f;
        }
    }
    DebugRAM("end of LoadLevel");
}

void UpdateSensaroundCamViews() {
    if (camViewCount < 3 || camViews[camViewCount-1].far != 1000.0f) { sensaroundCamViewCenter=sensaroundCamViewLeft=sensaroundCamViewRight=255; return; }
    sensaroundCamViewCenter = (u8)(camViewCount - 3);
    sensaroundCamViewLeft   = (u8)(camViewCount - 2);
    sensaroundCamViewRight  = (u8)(camViewCount - 1);
    bool hwActive = (World.invP1.hardwareIsActive & HW_SNS) != 0;
    u16 hwVer = World.invP1.hwVers[HW_SNS_IDX];
    bool centerOn = hwActive && hwVer > 0;
    bool sidesOn  = hwActive && hwVer > 1;
    camViews[sensaroundCamViewCenter].visible = centerOn;
    camViews[sensaroundCamViewLeft ].visible = sidesOn;
    camViews[sensaroundCamViewRight].visible = sidesOn;
    if (!hwActive) return;
    V3 fwd = World.instances[PLAYER1].forward;
    if (V3_dot(fwd,fwd) < 0.001f) fwd = quat_rot_v3(World.rotation[PLAYER1],(V3){0.0f,0.0f,1.0f}); /* fallback before physics tick */
    V3 rear = V3_ScaleByF(fwd,-1.0f); const V3 up = {0.0f,1.0f,0.0f};
    camViews[sensaroundCamViewCenter].position = World.position[PLAYER1];
    camViews[sensaroundCamViewCenter].rotation = quat_look_rotation(rear,up);
    if (sidesOn) {
        V3 leftDir  = V3_Normalize((V3){ rear.z, rear.y, -rear.x });
        V3 rightDir = V3_Normalize((V3){ -rear.z, rear.y,  rear.x });
        camViews[sensaroundCamViewLeft ].position = World.position[PLAYER1]; camViews[sensaroundCamViewLeft ].rotation = quat_look_rotation(leftDir,up);
        camViews[sensaroundCamViewRight].position = World.position[PLAYER1]; camViews[sensaroundCamViewRight].rotation = quat_look_rotation(rightDir,up);
    }
}

// Save Game System
#pragma pack(push, 1)
typedef struct { u32 magicNumber; u32 version; u32 uncompressedSize; u32 compressedSize; char savename[48]; } SaveHeader;
#pragma pack(pop)
size_t GetMaxCompressedSize(size_t srcSize) { return srcSize + (srcSize / 128) + 16; } // Worst-case buffer size for allocation
size_t VoidSquasher(const u8* src, size_t srcSize, u8* dst, size_t dstCapacity) { // Find and pop the zeroes bubbles.  Turned an otherwise 232mb save file into ~23mb.
    size_t s = 0, d = 0;
    while (s < srcSize) {/*1. Zeros*/
        size_t zeroCount = 0;
        while(s + zeroCount < srcSize && src[s + zeroCount] == 0){zeroCount++;}
        if(zeroCount > 0){
            if(zeroCount < 128){ if (d >= dstCapacity){return 0;} dst[d++]=(u8)(0x80 + (zeroCount-1));}
            else{if(d + 5 > dstCapacity){return 0;} dst[d++] = 0xFF; u32 zCount32=(u32)zeroCount; mcpy(&dst[d],&zCount32,sizeof(u32)); d+=4;}
            
            s+=zeroCount;
            continue;
        }
        
        size_t litCount = 0;/*2. Non-Zeros. It costs 2 bytes of overhead so only break a run if 3 or more zeros ahead.*/
        while (s + litCount < srcSize && litCount < 128) {
            if (src[s + litCount] == 0) {
                size_t remain = srcSize - (s + litCount);
                if (remain >= 3 && src[s + litCount + 1] == 0 && src[s + litCount + 2] == 0) { break; }
            }
            litCount++;
        }
        if (litCount > 0) {
            if (d + 1 + litCount > dstCapacity) {return 0;}
            
            dst[d++] = (u8)(litCount - 1);
            mcpy(&dst[d],&src[s],litCount);
            s+=litCount; d+=litCount;
        }
    } return d;
}

size_t BlowBubblesOfVoid(const u8* src, size_t srcSize, u8* dst, size_t dstCapacity) { // Put the bubbles of zero back.
    size_t s = 0, d = 0;
    while (s < srcSize && d < dstCapacity) {
        u8 cmd=src[s++];
        if (cmd<128){size_t litCount=cmd+1; if(s + litCount>srcSize || d+litCount>dstCapacity){return 0;} mcpy(&dst[d],&src[s],litCount); s+=litCount; d+=litCount;}/*Literal Run*/
        else if(cmd<0xFF){size_t zeroCount=cmd-128+1; if(d+zeroCount>dstCapacity){return 0;} mset(&dst[d],0,zeroCount); d+=zeroCount;}/*Short 0 Run*/
        else{if(s+4>srcSize){return 0;} u32 zeroCount; mcpy(&zeroCount,&src[s],sizeof(u32)); s+=4; if(d+zeroCount>dstCapacity){return 0;} mset(&dst[d], 0, zeroCount); d += zeroCount; }/*Long 0 Run*/
    } return d;
}

void SaveGame(u8 slot, const char* savename) {
    if(slot > 7 && !savename){return;}
    
    char path[]="./Data/sav0.bin"; path[10]='0' + slot;
    FHandle fd=OS_OpenWriteonly(path); if(fd == (FHandle)-1){return;}
    
    size_t sz=sizeof(GlobalContext);
    size_t maxCompSize=GetMaxCompressedSize(sz);
    u8* b=OS_Alloc(maxCompSize);
    size_t finalCompSize=VoidSquasher((const u8*)&World,sz,b,maxCompSize);
    if (finalCompSize > 0) {
        SaveHeader header = {.magicNumber=0x56415343/*'CSAV'*/, .version=SAVE_VERSION, .uncompressedSize=(u32)sz, .compressedSize=(u32)finalCompSize};
        int i=0;
        while(savename[i] != '\0' && i < 47){header.savename[i]=savename[i]; i++;}
        header.savename[i]='\0';
        
        World.justSavedTimeStamp = get_time();
        OS_Write(fd,&header,sizeof(SaveHeader),path);
        OS_Write(fd,b,finalCompSize,path);
        CenterStatusPrint("Saved to Slot %d",slot);
    } else { DualLogError("Compression failed during SaveGame!\n"); }    OS_Free(b,maxCompSize); OS_Close(fd);
}

void LoadGame(u8 slot) {
    if(slot > 7){return;} char path[]="./Data/sav0.bin"; path[10]='0' + slot; FHandle fd=OS_OpenReadonly(path); if(fd == (FHandle)-1){return;} SaveHeader header; if (OS_Read(fd,&header,sizeof(SaveHeader)) != sizeof(SaveHeader) || header.magicNumber != 0x56415343) { DualLogError("Corrupted save file header!\n"); OS_Close(fd); return; }
    if (header.version != SAVE_VERSION || header.uncompressedSize != sizeof(GlobalContext)) { DualLogError("Save is from an incompatible build (v%u, %u bytes; this build wants v%u, %zu bytes) -- start a new game.\n", header.version, header.uncompressedSize, (u32)SAVE_VERSION, sizeof(GlobalContext)); OS_Close(fd); return; }/*split out of the header check above, which reported a plain version or layout-size mismatch as corruption.  The two saves in Data/ are both v7 and were already unloadable by v11: this project bumps the constant rather than migrating, so name the real reason.*/
    if (header.compressedSize > GetMaxCompressedSize(header.uncompressedSize)) { DualLogError("Save has an implausible compressed size (%u bytes) -- treating as corrupt.\n", header.compressedSize); OS_Close(fd); return; }
    u8* b=OS_Alloc(header.compressedSize);
    if (OS_Read(fd,b,header.compressedSize) == (long)header.compressedSize) {
        size_t result = BlowBubblesOfVoid(b,header.compressedSize,(u8*)&World,header.uncompressedSize);/*Decompress straight into the World str uct*/ if (result == header.uncompressedSize) { SetLevelPointers(World.currentLevel); AutomapOnLoad(); CenterStatusPrint("Loaded Game: %s", header.savename); } else { DualLogError("Decompression failed! Expected %u bytes, got %u\n", header.uncompressedSize, (u32)result); }
    } OS_Free(b,header.compressedSize); OS_Close(fd); for (int i=0;i<World.loadedLights;++i) { flag_set(&World.lights[i].lflags,LDIRTY,true); }
}

/*Read just the save header for a slot (for the load/save menu slot list). Returns true and fills out (NUL-terminated) when a valid save file exists; false when the slot is unused/corrupt.*/
bool ReadSaveSlotName(u8 slot, char* out, size_t cap) {
    if (slot > 7 || !out || cap == 0) return false;
    char path[]="./Data/sav0.bin"; path[10]=(char)('0'+slot);
    FHandle fd=OS_OpenReadonly(path); if (fd == (FHandle)-1) return false;
    SaveHeader header; bool ok = OS_Read(fd,&header,sizeof(SaveHeader))==sizeof(SaveHeader) && header.magicNumber==0x56415343u && header.version==SAVE_VERSION && header.uncompressedSize==(u32)sizeof(GlobalContext);
    OS_Close(fd); if (!ok) return false;
    size_t i=0; while (i+1<cap && i<sizeof(header.savename) && header.savename[i]!='\0') { out[i]=header.savename[i]; ++i; } out[i]='\0';
    return out[0]!='\0';
}
