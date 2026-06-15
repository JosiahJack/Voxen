#include "mod.h"
#define LINE_LEN_MAX 81920
Entity EDefs[MAX_ENTITIES];
#define GEOMETRY_LOD_CARD_MODEL_IDX 178
void* MemCpyFromBtoAForNBytes(void *dst, const void *src, size_t n) { u8 *d=(u8 *)dst; const u8 *s=(const u8 *)src; while (n--) {*d++=*s++;} return dst; } // memcpy replacement
MOD_TO_ENGINE void ModEntityDefinitionsInitAfterLoad(void) { // Global conditions for all entities.  No sense inflating the table data in entity.c
    MemSetToVForNBytes(EDefs,0,sizeof(Entity)); 
    for (int i=0;i<768;++i) { EDefs[i].index = i; EDefs[i].modelIndex = MODEL_IDX_MAX; EDefs[i].rotation = QUAT_IDENTITY; EDefs[i].lodIndex = MODEL_IDX_MAX; }
    
    // Modular Wall, Ceiling, Floor Chunks
    for (int i=0;i<=306;++i) EDefs[i].cardchunk = true; // Mark these all to have box colliders added... except for slices below:
    EDefs[  6].cardchunk = EDefs[  9].cardchunk = EDefs[ 10].cardchunk = EDefs[ 20].cardchunk = EDefs[ 22].cardchunk = EDefs[ 31].cardchunk = EDefs[ 32].cardchunk = EDefs[ 42].cardchunk = false;
    EDefs[ 43].cardchunk = EDefs[ 44].cardchunk = EDefs[ 52].cardchunk = EDefs[ 63].cardchunk = EDefs[ 78].cardchunk = EDefs[ 83].cardchunk = EDefs[ 87].cardchunk = EDefs[ 91].cardchunk = false;
    EDefs[ 95].cardchunk = EDefs[142].cardchunk = EDefs[143].cardchunk = EDefs[145].cardchunk = EDefs[146].cardchunk = EDefs[147].cardchunk = EDefs[150].cardchunk = EDefs[151].cardchunk = false;
    EDefs[152].cardchunk = EDefs[153].cardchunk = EDefs[163].cardchunk = EDefs[164].cardchunk = EDefs[165].cardchunk = EDefs[166].cardchunk = EDefs[168].cardchunk = EDefs[176].cardchunk = false;
    EDefs[177].cardchunk = EDefs[179].cardchunk = EDefs[182].cardchunk = EDefs[191].cardchunk = EDefs[192].cardchunk = EDefs[193].cardchunk = EDefs[200].cardchunk = EDefs[210].cardchunk = false;
    EDefs[211].cardchunk = EDefs[212].cardchunk = EDefs[213].cardchunk = EDefs[219].cardchunk = EDefs[233].cardchunk = EDefs[242].cardchunk = EDefs[243].cardchunk = EDefs[246].cardchunk = false;
    EDefs[247].cardchunk = EDefs[248].cardchunk = EDefs[249].cardchunk = EDefs[255].cardchunk = EDefs[263].cardchunk = EDefs[264].cardchunk = EDefs[283].cardchunk = EDefs[284].cardchunk = false;
    EDefs[285].cardchunk = EDefs[291].cardchunk = EDefs[298].cardchunk = EDefs[299].cardchunk = EDefs[300].cardchunk = EDefs[301].cardchunk = EDefs[303].cardchunk = EDefs[188].cardchunk = false;
    /*  0 chunk_black*/                EDefs[  0].modelIndex=178; EDefs[  0].texIndex=0;
    /*  1 chunk_blocker*/              EDefs[  1].modelIndex=178; EDefs[  1].texIndex=1230;EDefs[  1].normIndex=160; EDefs[  1].specIndex=1230;
    /*  2 chunk_bridg1_1*/             EDefs[  2].modelIndex=661; EDefs[  2].texIndex=44;  EDefs[  2].normIndex=43;
    /*  3 chunk_bridg1_1flipx*/        EDefs[  3].modelIndex=667; EDefs[  3].texIndex=44;
    /*  4 chunk_bridg1_2*/             EDefs[  4].modelIndex=662; EDefs[  4].texIndex=45;
    /*  5 chunk_bridg1_3*/             EDefs[  5].modelIndex=20;  EDefs[  5].texIndex=47;
    /*  6 chunk_bridg1_3_slice45*/     EDefs[  6].modelIndex=21;  EDefs[  6].texIndex=47;
    /*  7 chunk_bridg1_3flipx*/        EDefs[  7].modelIndex=663; EDefs[  7].texIndex=47;
    /*  8 chunk_bridg1_4*/             EDefs[  8].modelIndex=22;  EDefs[  8].texIndex=48;
    /*  9 chunk_bridg1_4_slice32*/     EDefs[  9].modelIndex=23;  EDefs[  9].texIndex=48;
    /* 10 chunk_bridg1_4_slice32flipx*/EDefs[ 10].modelIndex=24;  EDefs[ 10].texIndex=48;
    /* 11 chunk_bridg1_5*/             EDefs[ 11].modelIndex=25;  EDefs[ 11].texIndex=50;  EDefs[ 11].glowIndex=49;
    /* 12 chunk_bridg2_2*/             EDefs[ 12].modelIndex=26;  EDefs[ 12].texIndex=53;
    /* 13 chunk_bridg2_3*/             EDefs[ 13].modelIndex=27;  EDefs[ 13].texIndex=56;  EDefs[ 13].glowIndex=54;  EDefs[ 13].normIndex=55;
    /* 14 chunk_bridg2_4*/             EDefs[ 14].modelIndex=28;  EDefs[ 14].texIndex=57;
    /* 15 chunk_bridg2_5*/             EDefs[ 15].modelIndex=29;  EDefs[ 15].texIndex=59;  EDefs[ 15].normIndex=58;
    /* 16 chunk_bridg2_6*/             EDefs[ 16].modelIndex=30;  EDefs[ 16].texIndex=60;
    /* 17 chunk_bridg2_7*/             EDefs[ 17].modelIndex=664; EDefs[ 17].texIndex=61;
    /* 18 chunk_bridg2_8*/             EDefs[ 18].modelIndex=31;  EDefs[ 18].texIndex=62;
    /* 19 chunk_bridg2_9*/             EDefs[ 19].modelIndex=32;  EDefs[ 19].texIndex=64;  EDefs[ 19].glowIndex=63;
    /* 20 chunk_crate_impenetrable*/   EDefs[ 20].modelIndex=61;  EDefs[ 20].texIndex=150;
    /* 21 chunk_cyberpanel*/           EDefs[ 21].modelIndex=178; EDefs[ 21].texIndex=151; EDefs[ 21].glowIndex=151;
    /* 22 chunk_cyberpanel_slice45*/   EDefs[ 22].modelIndex=180; EDefs[ 22].texIndex=152; EDefs[ 22].glowIndex=152;
    /* 23 chunk_eng1_1*/               EDefs[ 23].modelIndex=96;  EDefs[ 23].texIndex=254;
    /* 24 chunk_eng1_1d*/              EDefs[ 24].modelIndex=95;  EDefs[ 24].texIndex=253;
    /* 25 chunk_eng1_2*/               EDefs[ 25].modelIndex=98;  EDefs[ 25].texIndex=256;
    /* 26 chunk_eng1_2d*/              EDefs[ 26].modelIndex=97;  EDefs[ 26].texIndex=255;
    /* 27 chunk_eng1_3*/               EDefs[ 27].modelIndex=100; EDefs[ 27].texIndex=259; EDefs[ 27].glowIndex=258;
    /* 28 chunk_eng1_3d*/              EDefs[ 28].modelIndex=99;  EDefs[ 28].texIndex=257;
    /* 29 chunk_eng1_4*/               EDefs[ 29].modelIndex=101; EDefs[ 29].texIndex=260;
    /* 30 chunk_eng1_5*/               EDefs[ 30].modelIndex=103; EDefs[ 30].texIndex=262;
    /* 31 chunk_eng1_5_slice45lh*/     EDefs[ 31].modelIndex=104; EDefs[ 31].texIndex=262;
    /* 32 chunk_eng1_5_slice45rh*/     EDefs[ 32].modelIndex=105; EDefs[ 32].texIndex=262;
    /* 33 chunk_eng1_5d*/              EDefs[ 33].modelIndex=102; EDefs[ 33].texIndex=261;
    /* 34 chunk_eng1_6*/               EDefs[ 34].modelIndex=107; EDefs[ 34].texIndex=266; EDefs[ 34].glowIndex=265;
    /* 35 chunk_eng1_6d*/              EDefs[ 35].modelIndex=106; EDefs[ 35].texIndex=264; EDefs[ 35].glowIndex=263;
    /* 36 chunk_eng1_7*/               EDefs[ 36].modelIndex=108; EDefs[ 36].texIndex=269; EDefs[ 36].glowIndex=268;
    /* 37 chunk_eng1_7d*/              EDefs[ 37].modelIndex=665; EDefs[ 37].texIndex=267;
    /* 38 chunk_eng1_8*/               EDefs[ 38].modelIndex=109; EDefs[ 38].texIndex=271; EDefs[ 38].glowIndex=270;
    /* 39 chunk_eng1_9*/               EDefs[ 39].modelIndex=111; EDefs[ 39].texIndex=273; EDefs[ 39].glowIndex=251;
    /* 40 chunk_eng1_9d*/              EDefs[ 40].modelIndex=110; EDefs[ 40].texIndex=272;
    /* 41 chunk_eng2_1*/               EDefs[ 41].modelIndex=113; EDefs[ 41].texIndex=276;
    /* 42 chunk_eng2_1_slice45*/       EDefs[ 42].modelIndex=116; EDefs[ 42].texIndex=276;
    /* 43 chunk_eng2_1_slice384high*/  EDefs[ 43].modelIndex=114; EDefs[ 43].texIndex=276;
    /* 44 chunk_eng2_1_slice384highrh*/EDefs[ 44].modelIndex=115; EDefs[ 44].texIndex=276;
    /* 45 chunk_eng2_1d*/              EDefs[ 45].modelIndex=112; EDefs[ 45].texIndex=275; EDefs[ 45].glowIndex=274;
    /* 46 chunk_eng2_2*/               EDefs[ 46].modelIndex=117; EDefs[ 46].texIndex=279;
    /* 47 chunk_eng2_2d*/              EDefs[ 47].modelIndex=666; EDefs[ 47].texIndex=277;
    /* 48 chunk_eng2_3*/               EDefs[ 48].modelIndex=119; EDefs[ 48].texIndex=282;
    /* 49 chunk_eng2_3d*/              EDefs[ 49].modelIndex=118; EDefs[ 49].texIndex=281;
    /* 50 chunk_eng2_4*/               EDefs[ 50].modelIndex=178; EDefs[ 50].texIndex=283;
    /* 51 chunk_eng2_5*/               EDefs[ 51].modelIndex=120; EDefs[ 51].texIndex=285; EDefs[ 51].normIndex=284;
    /* 52 chunk_eng2_5_slice45*/       EDefs[ 52].modelIndex=121; EDefs[ 52].texIndex=285; EDefs[ 52].normIndex=284;
    /* 53 chunk_eng2_6 (wall pump)*/                              EDefs[ 53].texIndex=141; EDefs[ 53].glowIndex=142; EDefs[ 53].numclips=1; EDefs[ 53].animationNum=21;
    /* 54 chunk_exec1_1*/              EDefs[ 54].modelIndex=124; EDefs[ 54].texIndex=287;
    /* 55 chunk_exec1_1d*/             EDefs[ 55].modelIndex=123; EDefs[ 55].texIndex=286;
    /* 56 chunk_exec1_2*/              EDefs[ 56].modelIndex=126; EDefs[ 56].texIndex=291; EDefs[ 56].glowIndex=290;
    /* 57 chunk_exec1_2d*/             EDefs[ 57].modelIndex=125; EDefs[ 57].texIndex=289; EDefs[ 57].glowIndex=288;
    /* 58 chunk_exec2_1*/              EDefs[ 58].modelIndex=127; EDefs[ 58].texIndex=292;
    /* 59 chunk_exec2_2*/              EDefs[ 59].modelIndex=129; EDefs[ 59].texIndex=295;
    /* 60 chunk_exec2_2d*/             EDefs[ 60].modelIndex=128; EDefs[ 60].texIndex=294; EDefs[ 60].glowIndex=293;
    /* 61 chunk_exec2_3*/              EDefs[ 61].modelIndex=130; EDefs[ 61].texIndex=296;
    /* 62 chunk_exec2_4*/              EDefs[ 62].modelIndex=131; EDefs[ 62].texIndex=297;
    /* 63 chunk_exec2_4_slice45*/      EDefs[ 63].modelIndex=132; EDefs[ 63].texIndex=297;
    /* 64 chunk_exec2_5*/              EDefs[ 64].modelIndex=133; EDefs[ 64].texIndex=298; EDefs[ 64].specIndex=1257;
    /* 65 chunk_exec2_6*/              EDefs[ 65].modelIndex=134; EDefs[ 65].texIndex=299; EDefs[ 65].specIndex=1257;
    /* 66 chunk_exec2_7*/              EDefs[ 66].modelIndex=133; EDefs[ 66].texIndex=300; EDefs[ 66].specIndex=1257;
    /* 67 chunk_exec3_1*/              EDefs[ 67].modelIndex=127; EDefs[ 67].texIndex=303;
    /* 68 chunk_exec3_1d*/             EDefs[ 68].modelIndex=135; EDefs[ 68].texIndex=302; EDefs[68].glowIndex=301;
    /* 69 chunk_exec3_2*/              EDefs[ 69].modelIndex=129; EDefs[ 69].texIndex=304;
    /* 70 chunk_exec3_4*/              EDefs[ 70].modelIndex=178; EDefs[ 70].texIndex=305;
    /* 71 chunk_exec4_1*/              EDefs[ 71].modelIndex=136; EDefs[ 71].texIndex=307; EDefs[ 71].glowIndex=306;
    /* 72 chunk_exec4_2*/              EDefs[ 72].modelIndex=137; EDefs[ 72].texIndex=308;
    /* 73 chunk_exec4_3*/              EDefs[ 73].modelIndex=138; EDefs[ 73].texIndex=309;
    /* 74 chunk_exec4_4*/              EDefs[ 74].modelIndex=139; EDefs[ 74].texIndex=311;
    /* 75 chunk_exec4_5*/              EDefs[ 75].modelIndex=178; EDefs[ 75].texIndex=312;
    /* 76 chunk_exec4_6*/              EDefs[ 76].modelIndex=141; EDefs[ 76].texIndex=313;
    /* 77 chunk_exec6_1*/              EDefs[ 77].modelIndex=142; EDefs[ 77].texIndex=315; EDefs[ 77].glowIndex=314;
    /* 78 chunk_exteriorpanel1*/       EDefs[ 78].modelIndex=131; EDefs[ 78].texIndex=1228;
    /* 79 chunk_fan1*/                                            EDefs[ 79].texIndex=96;  EDefs[ 79].glowIndex=192; EDefs[ 79].numclips=1; EDefs[ 79].animationNum=22;  
    /* 80 chunk_flight1_1*/            EDefs[ 80].modelIndex=146; EDefs[ 80].texIndex=319;
    /* 81 chunk_flight1_1b*/           EDefs[ 81].modelIndex=146; EDefs[ 81].texIndex=318;
    /* 82 chunk_flight1_2*/            EDefs[ 82].modelIndex=147; EDefs[ 82].texIndex=320;
    /* 83 chunk_flight1_2_slice45rh*/  EDefs[ 83].modelIndex=149; EDefs[ 83].texIndex=320;
    /* 84 unused */
    /* 85 chunk_flight1_4*/            EDefs[ 85].modelIndex=151; EDefs[ 85].texIndex=322;
    /* 86 chunk_flight1_5*/            EDefs[ 86].modelIndex=147; EDefs[ 86].texIndex=323;
    /* 87 chunk_flight1_5_slice45lh*/  EDefs[ 87].modelIndex=148; EDefs[ 87].texIndex=323;
    /* 88 chunk_flight1_6*/            EDefs[ 88].modelIndex=152; EDefs[ 88].texIndex=325;
    /* 89 chunk_flight2_1*/            EDefs[ 89].modelIndex=153; EDefs[ 89].texIndex=326;
    /* 90 chunk_flight2_2*/            EDefs[ 90].modelIndex=154; EDefs[ 90].texIndex=327;
    /* 91 chunk_flight2_2_slice45*/    EDefs[ 91].modelIndex=155; EDefs[ 91].texIndex=327;
    /* 92 chunk_flight2_3*/            EDefs[ 92].modelIndex=156; EDefs[ 92].texIndex=328;
    /* 93 chunk_grove1_1*/             EDefs[ 93].modelIndex=189; EDefs[ 93].texIndex=362;
    /* 94 chunk_grove1_2*/             EDefs[ 94].modelIndex=178; EDefs[ 94].texIndex=363;
    /* 95 chunk_grove1_2_slice45*/     EDefs[ 95].modelIndex=180; EDefs[ 95].texIndex=363;
    /* 96 chunk_grove1_3*/             EDefs[ 96].modelIndex=178; EDefs[ 96].texIndex=364;
    /* 97 chunk_grove1_4*/             EDefs[ 97].modelIndex=178; EDefs[ 97].texIndex=365;
    /* 98 chunk_grove1_5*/             EDefs[ 98].modelIndex=178; EDefs[ 98].texIndex=367;
    /* 99 chunk_grove1_6*/             EDefs[ 99].modelIndex=178; EDefs[ 99].texIndex=368;
    /*100 chunk_grove1_7*/             EDefs[100].modelIndex=178; EDefs[100].texIndex=369;
    /*101 chunk_grove2_1*/             EDefs[101].modelIndex=190; EDefs[101].texIndex=370;
    /*102 chunk_grove2_2*/             EDefs[102].modelIndex=190; EDefs[102].texIndex=371;
    /*103 chunk_grove2_3*/             EDefs[103].modelIndex=191; EDefs[103].texIndex=372;
    /*104 chunk_grove2_4*/             EDefs[104].modelIndex=341; EDefs[104].texIndex=374; EDefs[104].glowIndex=373;
    /*105 chunk_grove2_5*/             EDefs[105].modelIndex=192; EDefs[105].texIndex=375;
    /*106 chunk_grove2_6*/             EDefs[106].modelIndex=192; EDefs[106].texIndex=376;
    /*107 chunk_grove2_7*/             EDefs[107].modelIndex=191; EDefs[107].texIndex=378;
    /*108 chunk_grove2_8*/             EDefs[108].modelIndex=191; EDefs[108].texIndex=379;
    /*109 chunk_grove2_9*/             EDefs[109].modelIndex=191; EDefs[109].texIndex=385;
    /*110 chunk_grove2_9b*/            EDefs[110].modelIndex=191; EDefs[110].texIndex=381;
    /*111 chunk_grove2_9c*/            EDefs[111].modelIndex=191; EDefs[111].texIndex=383;
    /*112 chunk_lift1*/                EDefs[112].modelIndex=213; EDefs[112].texIndex=1246; EDefs[112].glowIndex=1247;
    /*113 chunk_maint1_1*/             EDefs[113].modelIndex=218; EDefs[113].texIndex=430;
    /*114 chunk_maint1_2*/             EDefs[114].modelIndex=220; EDefs[114].texIndex=432;
    /*115 chunk_maint1_2d*/            EDefs[115].modelIndex=219; EDefs[115].texIndex=431;
    /*116 chunk_maint1_3*/             EDefs[116].modelIndex=222; EDefs[116].texIndex=436; EDefs[116].glowIndex=435; EDefs[116].specIndex=437;
    /*117 chunk_maint1_3b*/            EDefs[117].modelIndex=221; EDefs[117].texIndex=434; EDefs[117].glowIndex=433;
    /*118 chunk_maint1_4*/             EDefs[118].modelIndex=224; EDefs[118].texIndex=441; EDefs[118].glowIndex=440;
    /*119 chunk_maint1_4b*/            EDefs[119].modelIndex=223; EDefs[119].texIndex=439; EDefs[119].glowIndex=438;
    /*120 chunk_maint1_5*/             EDefs[120].modelIndex=225; EDefs[120].texIndex=443; EDefs[120].glowIndex=442;
    /*121 chunk_maint1_6*/             EDefs[121].modelIndex=226; EDefs[121].texIndex=96;
    /*122 chunk_maint1_7*/             EDefs[122].modelIndex=227; EDefs[122].texIndex=447; EDefs[122].glowIndex=446;
    /*123 chunk_blockerflightbay*/     EDefs[123].modelIndex=178; EDefs[123].normIndex=160; EDefs[123].texIndex=1230; EDefs[123].specIndex=1242; EDefs[123].collider=COLTYPE_BOX; EDefs[123].colliderCenter=(Vector3){0.0f,1.44f,0.0f}; EDefs[123].colliderSize=(Vector3){2.56f,0.32f,2.56f}; EDefs[123].colliderMeshIndex=U16_MAX;
    /*124 chunk_maint1_9*/             EDefs[124].modelIndex=606; EDefs[124].texIndex=450;
    /*125 chunk_maint1_9d*/            EDefs[125].modelIndex=620; EDefs[125].texIndex=449; EDefs[125].glowIndex=448;
    /*126 chunk_maint2_1*/             EDefs[126].modelIndex=230; EDefs[126].texIndex=455;
    /*127 chunk_maint2_1b*/            EDefs[127].modelIndex=228; EDefs[127].texIndex=451;
    /*128 chunk_maint2_1d*/            EDefs[128].modelIndex=229; EDefs[128].texIndex=453; EDefs[128].glowIndex=452;
    /*129 chunk_maint2_2*/             EDefs[129].modelIndex=230; EDefs[129].texIndex=457;
    /*130 chunk_maint2_3*/             EDefs[130].modelIndex=232; EDefs[130].texIndex=460;
    /*131 chunk_maint2_3d*/            EDefs[131].modelIndex=231; EDefs[131].texIndex=459; EDefs[131].glowIndex=458;
    /*132 chunk_maint2_4*/             EDefs[132].modelIndex=233; EDefs[132].texIndex=464; EDefs[132].glowIndex=463;
    /*133 chunk_maint2_4d*/            EDefs[133].modelIndex=233; EDefs[133].texIndex=462; EDefs[133].glowIndex=461;
    /*134 chunk_maint2_5*/             EDefs[134].modelIndex=235; EDefs[134].texIndex=468; EDefs[134].glowIndex=467;
    /*135 chunk_maint2_5d*/            EDefs[135].modelIndex=234; EDefs[135].texIndex=466; EDefs[135].glowIndex=465;
    /*136 chunk_maint2_6*/             EDefs[136].modelIndex=236; EDefs[136].texIndex=472; EDefs[136].glowIndex=471;
    /*137 chunk_maint2_6d*/            EDefs[137].modelIndex=238; EDefs[137].texIndex=470; EDefs[137].glowIndex=470;
    /*138 chunk_maint2_7*/             EDefs[138].modelIndex=238; EDefs[138].texIndex=476; EDefs[138].glowIndex=475;
    /*139 chunk_maint2_7d*/            EDefs[139].modelIndex=237; EDefs[139].texIndex=474; EDefs[139].glowIndex=473;
    /*140 chunk_maint2_8*/             EDefs[140].modelIndex=239; EDefs[140].texIndex=478; EDefs[140].glowIndex=477;
    /*141 chunk_maint2_9*/             EDefs[141].modelIndex=240; EDefs[141].texIndex=480; EDefs[141].glowIndex=479;
    /*142 chunk_maint2_9_slice45RH*/   EDefs[142].modelIndex=242; EDefs[142].texIndex=480; EDefs[142].glowIndex=479;
    /*143 chunk_maint2_9_slice128_top*/EDefs[143].modelIndex=241; EDefs[143].texIndex=480; EDefs[143].glowIndex=479;
    /*144 chunk_maint3_1*/             EDefs[144].modelIndex=244; EDefs[144].texIndex=483;
    /*145 chunk_maint3_1_slice32_lh*/  EDefs[145].modelIndex=246; EDefs[145].texIndex=483;
    /*146 chunk_maint3_1_slice32_rh*/  EDefs[146].modelIndex=245; EDefs[146].texIndex=483;
    /*147 chunk_maint3_1_slice45*/     EDefs[147].modelIndex=247; EDefs[147].texIndex=483;
    /*148 chunk_maint3_1d*/            EDefs[148].modelIndex=243; EDefs[148].texIndex=482; EDefs[148].glowIndex=481;
    /*149 chunk_med1_1*/               EDefs[149].modelIndex=249; EDefs[149].texIndex=486; EDefs[149].specIndex=1256; EDefs[149].normIndex=1255;
    /*150 chunk_med1_1_half_top*/      EDefs[150].modelIndex=250; EDefs[150].texIndex=486; EDefs[150].specIndex=1256; EDefs[150].normIndex=1255;
    /*151 chunk_med1_1_slice128high*/  EDefs[151].modelIndex=251; EDefs[151].texIndex=486; EDefs[151].specIndex=1256; EDefs[151].normIndex=1255;
    /*152 chunk_med1_1_slice192RH*/    EDefs[152].modelIndex=252; EDefs[152].texIndex=486; EDefs[152].specIndex=1256; EDefs[152].normIndex=1255;
    /*153 chunk_med1_1_slice256*/      EDefs[153].modelIndex=253; EDefs[153].texIndex=486; EDefs[153].specIndex=1256; EDefs[153].normIndex=1255;
    /*154 chunk_med1_1d*/              EDefs[154].modelIndex=248; EDefs[154].texIndex=485; EDefs[154].glowIndex=484; EDefs[154].specIndex=1236; EDefs[154].normIndex=1255;
    /*155 chunk_med1_2*/               EDefs[155].modelIndex=255; EDefs[155].texIndex=489; EDefs[155].glowIndex=488; EDefs[155].specIndex=1256;
    /*156 chunk_med1_2d*/              EDefs[156].modelIndex=254; EDefs[156].texIndex=487; EDefs[156].specIndex=1256;
    /*157 chunk_med1_3*/               EDefs[157].modelIndex=257; EDefs[157].texIndex=493; EDefs[157].glowIndex=492; EDefs[157].specIndex=1256;
    /*158 chunk_med1_3d*/              EDefs[158].modelIndex=256; EDefs[158].texIndex=491; EDefs[158].glowIndex=490; EDefs[158].specIndex=1256;
    /*159 chunk_med1_4*/               EDefs[159].modelIndex=258; EDefs[159].texIndex=494; EDefs[159].specIndex=1256;
    /*160 chunk_med1_5*/               EDefs[160].modelIndex=669; EDefs[160].texIndex=495; EDefs[160].specIndex=1256;
    /*161 chunk_med1_6*/               EDefs[161].modelIndex=259; EDefs[161].texIndex=496; EDefs[161].normIndex=509; EDefs[161].specIndex=1256;
    /*162 chunk_med1_7*/               EDefs[162].modelIndex=262; EDefs[162].texIndex=499; EDefs[162].specIndex=1268; EDefs[162].normIndex=498;
    /*163 chunk_med1_7_slice14_64*/    EDefs[163].modelIndex=263; EDefs[163].texIndex=499; EDefs[163].specIndex=1268; EDefs[163].normIndex=1254;
    /*164 chunk_med1_7_slice45_320lh*/ EDefs[164].modelIndex=264; EDefs[164].texIndex=499; EDefs[164].specIndex=1268; EDefs[164].normIndex=1254;
    /*165 chunk_med1_7_slice45_320rh*/ EDefs[165].modelIndex=265; EDefs[165].texIndex=499; EDefs[165].specIndex=1268; EDefs[165].normIndex=1254;
    /*166 chunk_med1_7_slice96high*/   EDefs[166].modelIndex=266; EDefs[166].texIndex=499; EDefs[166].specIndex=1268; EDefs[166].normIndex=1254;
    /*167 chunk_med1_7d*/              EDefs[167].modelIndex=260; EDefs[167].texIndex=497; EDefs[167].specIndex=1269; EDefs[167].normIndex=1270;
    /*168 chunk_med1_7d_slice128*/     EDefs[168].modelIndex=261; EDefs[168].texIndex=497; EDefs[168].specIndex=1269; EDefs[168].normIndex=1270;
    /*169 chunk_med1_8*/               EDefs[169].modelIndex=268; EDefs[169].texIndex=503; EDefs[169].normIndex=502; EDefs[169].specIndex=1242;
    /*170 chunk_med1_8d*/              EDefs[170].modelIndex=267; EDefs[170].texIndex=501; EDefs[170].normIndex=163; EDefs[170].specIndex=1242;
    /*171 chunk_med1_9*/               EDefs[171].modelIndex=278; EDefs[171].texIndex=507; EDefs[171].normIndex=506; EDefs[171].specIndex=1267;
    /*172 unused*/
    /*173 unused*/
    /*174 chunk_med1_9d*/              EDefs[174].modelIndex=269; EDefs[174].texIndex=505; EDefs[174].normIndex=504; EDefs[174].specIndex=1267;
    /*175 unused*/
    /*176 chunk_med1_9d_ofs112_90*/    EDefs[176].modelIndex=270; EDefs[176].texIndex=505; EDefs[176].normIndex=504; EDefs[176].specIndex=1267; EDefs[176].collider=COLTYPE_MSH; EDefs[176].colliderMeshIndex=270;
    /*177 chunk_med1_9d_ofs144_90*/    EDefs[177].modelIndex=272; EDefs[177].texIndex=505; EDefs[177].normIndex=504; EDefs[177].specIndex=1267; EDefs[177].collider=COLTYPE_MSH; EDefs[177].colliderMeshIndex=272;
    /*178 chunk_med2_1*/               EDefs[178].modelIndex=280; EDefs[178].texIndex=513; EDefs[178].specIndex=1254; EDefs[178].glowIndex=511; EDefs[178].normIndex=512;
    /*179 chunk_med2_1_slice32RH*/     EDefs[179].modelIndex=281; EDefs[179].texIndex=513; EDefs[179].normIndex=512; EDefs[179].specIndex=1254;
    /*180 chunk_med2_1d*/              EDefs[180].modelIndex=279; EDefs[180].glowIndex=508; EDefs[180].texIndex=510; EDefs[180].specIndex=1254;
    /*181 chunk_med2_2*/               EDefs[181].modelIndex=283; EDefs[181].texIndex=517; EDefs[181].glowIndex=516; EDefs[181].specIndex=1242;
    /*182 chunk_med2_2_half_bottom*/   EDefs[182].modelIndex=284; EDefs[182].texIndex=517; EDefs[182].glowIndex=516; EDefs[182].specIndex=1242;
    /*183 chunk_med2_2d*/              EDefs[183].modelIndex=282; EDefs[183].texIndex=515; EDefs[183].glowIndex=516; EDefs[183].specIndex=1242;
    /*184 chunk_med2_3*/               EDefs[184].modelIndex=286; EDefs[184].texIndex=521; EDefs[184].glowIndex=520; EDefs[184].specIndex=1242;
    /*185 chunk_med2_3d*/              EDefs[185].modelIndex=285; EDefs[185].texIndex=519; EDefs[185].glowIndex=518; EDefs[185].specIndex=1242;
    /*186 chunk_med2_4*/               EDefs[186].modelIndex=287; EDefs[186].texIndex=523; EDefs[186].glowIndex=522; EDefs[186].specIndex=1242;
    /*187 chunk_med2_5*/               EDefs[187].modelIndex=288; EDefs[187].texIndex=527; EDefs[187].glowIndex=526; EDefs[187].specIndex=539; EDefs[187].collider=COLTYPE_BOX; EDefs[187].colliderCenter=(Vector3){0.0f,1.44f,0.0f}; EDefs[187].colliderSize=(Vector3){2.56f,0.32f,2.56f}; EDefs[187].colliderMeshIndex=U16_MAX;
    /*188 chunk_med2_6*/               EDefs[188].modelIndex=289; EDefs[188].texIndex=528; EDefs[188].specIndex=1271;                          EDefs[188].collider=COLTYPE_MSH; EDefs[188].colliderMeshIndex=289;
    /*189 chunk_med2_7*/               EDefs[189].modelIndex=290; EDefs[189].texIndex=530; EDefs[189].glowIndex=529; EDefs[189].specIndex=1245;
    /*190 chunk_med2_8*/               EDefs[190].modelIndex=291; EDefs[190].texIndex=531; EDefs[190].specIndex=1242;
    /*191 chunk_med2_8_half_top*/      EDefs[191].modelIndex=292; EDefs[191].texIndex=531; EDefs[191].specIndex=1242;
    /*192 chunk_med2_8_slice32RH*/     EDefs[192].modelIndex=293; EDefs[192].texIndex=531; EDefs[192].specIndex=1242;
    /*193 chunk_med2_8_slice45*/       EDefs[193].modelIndex=294; EDefs[193].texIndex=531; EDefs[193].specIndex=1242;
    /*194 chunk_med2_9*/               EDefs[194].modelIndex=296; EDefs[194].texIndex=535; EDefs[194].glowIndex=534; EDefs[194].specIndex=1242;
    /*195 chunk_med2_9d*/              EDefs[195].modelIndex=295; EDefs[195].texIndex=533; EDefs[195].glowIndex=532; EDefs[195].specIndex=1242;
    /*196 chunk_med3_1*/               EDefs[196].modelIndex=297; EDefs[196].texIndex=536; EDefs[196].specIndex=1236;
    /*197 chunk_rad1_1*/               EDefs[197].modelIndex=501; EDefs[197].texIndex=660; EDefs[197].glowIndex=659; EDefs[197].specIndex=1231;
    /*198 chunk_rad1_2*/               EDefs[198].modelIndex=501; EDefs[198].texIndex=662; EDefs[198].glowIndex=661; EDefs[198].specIndex=1231;
    /*199 chunk_reac1_1*/              EDefs[199].modelIndex=502; EDefs[199].texIndex=664; EDefs[199].specIndex=1243;
    /*200 chunk_reac1_1_slice45*/      EDefs[200].modelIndex=339; EDefs[200].texIndex=664; EDefs[200].specIndex=1243;
    /*201 chunk_reac1_2*/              EDefs[201].modelIndex=503; EDefs[201].texIndex=665; EDefs[201].specIndex=1243;
    /*202 chunk_reac1_3*/              EDefs[202].modelIndex=504; EDefs[202].texIndex=666; EDefs[202].specIndex=1243;
    /*203 chunk_reac1_4*/              EDefs[203].modelIndex=505; EDefs[203].texIndex=668; EDefs[203].glowIndex=667; EDefs[203].specIndex=669;
    /*204 chunk_reac1_5*/              EDefs[204].modelIndex=506; EDefs[204].texIndex=671; EDefs[204].glowIndex=670; EDefs[204].specIndex=1239;
    /*205 chunk_reac1_6*/              EDefs[205].modelIndex=507; EDefs[205].texIndex=673; EDefs[205].glowIndex=672; EDefs[205].specIndex=1243;
    /*206 chunk_reac1_7*/              EDefs[206].modelIndex=342; EDefs[206].texIndex=676; EDefs[206].glowIndex=675; EDefs[206].specIndex=1243;
    /*207 chunk_reac1_8*/              EDefs[207].modelIndex=508; EDefs[207].texIndex=678; EDefs[207].glowIndex=678; EDefs[207].specIndex=1243;
    /*208 chunk_reac1_9*/              EDefs[208].modelIndex=509; EDefs[208].texIndex=680; EDefs[208].glowIndex=680; EDefs[208].specIndex=1243;
    /*209 chunk_reac2_1*/              EDefs[209].modelIndex=512; EDefs[209].texIndex=682; EDefs[209].specIndex=1235;
    /*210 chunk_reac2_1_slice45LH*/    EDefs[210].modelIndex=514; EDefs[210].texIndex=682; EDefs[210].specIndex=1235;
    /*211 chunk_reac2_1_slice45LH_up*/ EDefs[211].modelIndex=515; EDefs[211].texIndex=682; EDefs[211].specIndex=1235;
    /*212 chunk_reac2_1_slice45RH*/    EDefs[212].modelIndex=516; EDefs[212].texIndex=682; EDefs[212].specIndex=1235;
    /*213 chunk_reac2_1_slice45RH_up*/ EDefs[213].modelIndex=517; EDefs[213].texIndex=682; EDefs[213].specIndex=1235;
    /*214 chunk_reac2_1b*/             EDefs[214].modelIndex=510; EDefs[214].texIndex=681; EDefs[214].specIndex=1235;
    /*215 chunk_reac2_1bmirror*/       EDefs[215].modelIndex=511; EDefs[215].texIndex=681; EDefs[215].specIndex=1235;
    /*216 chunk_reac2_1mirror*/        EDefs[216].modelIndex=513; EDefs[216].texIndex=682; EDefs[216].specIndex=1235;
    /*217 chunk_reac2_2*/              EDefs[217].modelIndex=518; EDefs[217].texIndex=684; EDefs[217].glowIndex=683; EDefs[217].specIndex=1235;
    /*218 chunk_reac2_4*/              EDefs[218].modelIndex=519; EDefs[218].texIndex=685; EDefs[218].specIndex=1235;
    /*219 chunk_reac2_4_slice128lower*/EDefs[219].modelIndex=340; EDefs[219].texIndex=685; EDefs[219].specIndex=1235;
    /*220 chunk_reac2_5*/              EDefs[220].modelIndex=520; EDefs[220].texIndex=687; EDefs[220].glowIndex=686;
    /*221 chunk_reac2_6*/              EDefs[221].modelIndex=521; EDefs[221].texIndex=689; EDefs[221].glowIndex=688;
    /*222 chunk_reac2_7*/              EDefs[222].modelIndex=522; EDefs[222].texIndex=691; EDefs[222].glowIndex=690;
    /*223 chunk_reac2_8*/              EDefs[223].modelIndex=523; EDefs[223].texIndex=693; EDefs[223].glowIndex=692;
    /*224 chunk_reac2_9*/              EDefs[224].modelIndex=524; EDefs[224].texIndex=694;
    /*225 chunk_reac2_1*/              EDefs[225].modelIndex=525; EDefs[225].texIndex=696; EDefs[225].glowIndex=695;
    /*226 chunk_reac3_2*/              EDefs[226].modelIndex=526; EDefs[226].texIndex=697;
    /*227 chunk_reac3_3*/              EDefs[227].modelIndex=527; EDefs[227].texIndex=698;
    /*228 chunk_reac3_4*/              EDefs[228].modelIndex=528; EDefs[228].texIndex=699;
    /*229 chunk_reac3_5*/              EDefs[229].modelIndex=529; EDefs[229].texIndex=701; EDefs[229].glowIndex=700;
    /*230 chunk_reac3_6*/              EDefs[230].modelIndex=530; EDefs[230].texIndex=703; EDefs[230].glowIndex=702;
    /*231 chunk_reac3_7*/              EDefs[231].modelIndex=531; EDefs[231].texIndex=704; EDefs[231].specIndex=705;
    /*232 chunk_reac4_1*/              EDefs[232].modelIndex=532; EDefs[232].texIndex=707; EDefs[232].glowIndex=706;
    /*233 chunk_reac4_1_slice45lh*/    EDefs[233].modelIndex=533; EDefs[233].texIndex=707;
    /*234 chunk_reac4_2*/              EDefs[234].modelIndex=534; EDefs[234].texIndex=709; EDefs[234].glowIndex=708;
    /*235 chunk_reac5_1*/              EDefs[235].modelIndex=535; EDefs[235].texIndex=711; EDefs[235].glowIndex=710;
    /*236 chunk_reac5_2*/              EDefs[236].modelIndex=536; EDefs[236].texIndex=713; EDefs[236].glowIndex=712;
    /*237 chunk_reac5_3*/              EDefs[237].modelIndex=537; EDefs[237].texIndex=715; EDefs[237].glowIndex=714;
    /*238 chunk_reac6_1*/              EDefs[238].modelIndex=538; EDefs[238].texIndex=716;
    /*239 chunk_reac6_2*/              EDefs[239].modelIndex=539; EDefs[239].texIndex=717;
    /*240 chunk_reac6_3*/              EDefs[240].modelIndex=539; EDefs[240].texIndex=719; EDefs[240].glowIndex=718;
    /*241 chunk_sci1_1*/               EDefs[241].modelIndex=540; EDefs[241].texIndex=722; 
    /*242 chunk_sci1_1_slice45_toplh*/ EDefs[242].modelIndex=542; EDefs[242].texIndex=722; 
    /*243 chunk_sci1_1_slice45_toprh*/ EDefs[243].modelIndex=543; EDefs[243].texIndex=722; 
    /*244 chunk_sci1_1d*/              EDefs[244].modelIndex=541; EDefs[244].texIndex=721; 
    /*245 chunk_sci1_2*/               EDefs[245].modelIndex=545; EDefs[245].texIndex=724; 
    /*246 chunk_sci1_2_slice45lh*/     EDefs[246].modelIndex=546; EDefs[246].texIndex=724; 
    /*247 chunk_sci1_2_slice45lh_up*/  EDefs[247].modelIndex=547; EDefs[247].texIndex=724; 
    /*248 chunk_sci1_2_slice45rh*/     EDefs[248].modelIndex=548; EDefs[248].texIndex=724; 
    /*249 chunk_sci1_2_slice45rh_up*/  EDefs[249].modelIndex=549; EDefs[249].texIndex=724; 
    /*250 chunk_sci1_2d*/              EDefs[250].modelIndex=544; EDefs[250].texIndex=723; 
    /*251 chunk_sci1_3*/               EDefs[251].modelIndex=550; EDefs[251].texIndex=726; EDefs[251].glowIndex=725; 
    /*252 chunk_sci1_4*/               EDefs[252].modelIndex=498; EDefs[252].texIndex=727; 
    /*253 chunk_sci1_5*/               EDefs[253].modelIndex=551; EDefs[253].texIndex=728; 
    /*254 chunk_sci1_6*/               EDefs[254].modelIndex=552; EDefs[254].texIndex=729; 
    /*255 chunk_sci1_6_slice45*/       EDefs[255].modelIndex=553; EDefs[255].texIndex=729; 
    /*256 chunk_sci1_7*/               EDefs[256].modelIndex=555; EDefs[256].texIndex=731; 
    /*257 chunk_sci1_7d*/              EDefs[257].modelIndex=554; EDefs[257].texIndex=730; 
    /*258 chunk_sci1_8*/               EDefs[258].modelIndex=557; EDefs[258].texIndex=734; 
    /*259 chunk_sci1_8d*/              EDefs[259].modelIndex=556; EDefs[259].texIndex=733; 
    /*260 chunk_sci1_9*/               EDefs[260].modelIndex=559; EDefs[260].texIndex=737; 
    /*261 chunk_sci1_9d*/              EDefs[261].modelIndex=558; EDefs[261].texIndex=736; EDefs[261].glowIndex=735; 
    /*262 chunk_sci2_1*/               EDefs[262].modelIndex=561; EDefs[262].texIndex=739; 
    /*263 chunk_sci2_1_slice45lh*/     EDefs[263].modelIndex=563; EDefs[263].texIndex=739; 
    /*264 chunk_sci2_1_slice45rh*/     EDefs[264].modelIndex=562; EDefs[264].texIndex=739; 
    /*265 chunk_sci2_1d*/              EDefs[265].modelIndex=560; EDefs[265].texIndex=738; 
    /*266 chunk_sci2_2*/               EDefs[266].modelIndex=565; EDefs[266].texIndex=742; EDefs[266].glowIndex=741; 
    /*267 chunk_sci2_2d*/              EDefs[267].modelIndex=564; EDefs[267].texIndex=740; 
    /*268 chunk_sci2_3*/               EDefs[268].modelIndex=566; EDefs[268].texIndex=744; EDefs[268].glowIndex=743; 
    /*269 chunk_sci2_4*/               EDefs[269].modelIndex=567; EDefs[269].texIndex=745; 
    /*270 chunk_sci2_5*/               EDefs[270].modelIndex=569; EDefs[270].texIndex=747; 
    /*271 chunk_sci2_5d*/              EDefs[271].modelIndex=568; EDefs[271].texIndex=746; 
    /*272 chunk_sci3_1*/               EDefs[272].modelIndex=571; EDefs[272].texIndex=749; 
    /*273 chunk_sci3_1d*/              EDefs[273].modelIndex=570; EDefs[273].texIndex=748; 
    /*274 chunk_sci3_2*/               EDefs[274].modelIndex=572; EDefs[274].texIndex=750; 
    /*275 chunk_sci3_3*/               EDefs[275].modelIndex=573; EDefs[275].texIndex=752; EDefs[275].glowIndex=751; 
    /*276 chunk_sci3_4*/               EDefs[276].modelIndex=574; EDefs[276].texIndex=754; 
    /*277 chunk_sci3_5*/               EDefs[277].modelIndex=575; EDefs[277].texIndex=756; EDefs[277].glowIndex=755; 
    /*278 chunk_sci3_6*/               EDefs[278].modelIndex=576; EDefs[278].texIndex=758; EDefs[278].glowIndex=757; 
    /*279 chunk_screen*/               EDefs[279].modelIndex=5988;EDefs[279].texIndex=881; 
    /*280 chunk_sec1_1*/               EDefs[280].modelIndex=178; EDefs[280].texIndex=787; EDefs[280].specIndex=787; 
    /*281 chunk_sec1_1b*/              EDefs[281].modelIndex=178; EDefs[281].texIndex=785; EDefs[281].specIndex=785; 
    /*282 chunk_sec1_1c*/              EDefs[282].modelIndex=577; EDefs[282].texIndex=786; EDefs[282].specIndex=786; 
    /*283 chunk_sec1_1c_slice45*/      EDefs[283].modelIndex=580; EDefs[283].texIndex=786; EDefs[283].specIndex=786; 
    /*284 chunk_sec1_1c_slice64highlh*/EDefs[284].modelIndex=581; EDefs[284].texIndex=786; EDefs[284].specIndex=786; 
    /*285 chunk_sec1_1c_slice64highrh*/EDefs[285].modelIndex=582; EDefs[285].texIndex=786; EDefs[285].specIndex=786; 
    /*286 unused*/
    /*287 unused*/
    /*288 chunk_sec1_2*/               EDefs[288].modelIndex=584; EDefs[288].texIndex=789; EDefs[288].specIndex=1233; 
    /*289 chunk_sec1_2b*/              EDefs[289].modelIndex=583; EDefs[289].texIndex=788; EDefs[289].specIndex=1233; 
    /*290 chunk_sec1_3*/               EDefs[290].modelIndex=585; EDefs[290].texIndex=790; EDefs[290].specIndex=1233; 
    /*291 chunk_sec1_3_slice45*/       EDefs[291].modelIndex=586; EDefs[291].texIndex=790; EDefs[291].specIndex=1233; 
    /*292 chunk_stor1_1*/              EDefs[292].modelIndex=597; EDefs[292].texIndex=824; EDefs[292].glowIndex=823; 
    /*293 chunk_stor1_2*/              EDefs[293].modelIndex=598; EDefs[293].texIndex=825; 
    /*294 chunk_stor1_3*/              EDefs[294].modelIndex=598; EDefs[294].texIndex=826; 
    /*295 chunk_stor1_4*/              EDefs[295].modelIndex=599; EDefs[295].texIndex=827; 
    /*296 chunk_stor1_5*/              EDefs[296].modelIndex=600; EDefs[296].texIndex=828; 
    /*297 chunk_stor1_6*/              EDefs[297].modelIndex=601; EDefs[297].texIndex=829; 
    /*298chunk_stor1_6_slice128_up_lh*/EDefs[298].modelIndex=602; EDefs[298].texIndex=829; 
    /*299chunk_stor1_6_slice128_up_rh*/EDefs[299].modelIndex=603; EDefs[299].texIndex=829; 
    /*300 chunk_stor1_6_slice192lh*/   EDefs[300].modelIndex=604; EDefs[300].texIndex=829; 
    /*301 chunk_stor1_6_slice192rh*/   EDefs[301].modelIndex=605; EDefs[301].texIndex=829; 
    /*302 chunk_stor1_7*/              EDefs[302].modelIndex=606; EDefs[302].texIndex=833; EDefs[302].specIndex=834; EDefs[302].normIndex=832; 
    /*303 chunk_stor1_7_slice45*/      EDefs[303].modelIndex=607; EDefs[303].texIndex=833; EDefs[303].specIndex=834; EDefs[303].normIndex=832; 
    /*304 chunk_stor1_7d*/             EDefs[304].modelIndex=620; EDefs[304].texIndex=831; EDefs[304].glowIndex=830; EDefs[304].normIndex=832; EDefs[304].specIndex=834; 
    /*305 chunk_teleporter*/           EDefs[305].modelIndex=178; EDefs[305].texIndex=1166; 
    /*306 chunk_white*/                EDefs[306].modelIndex=178; EDefs[306].texIndex=881;
    for (int i=307;i<=404;++i) { EDefs[i].angularDrag=0.05f; EDefs[i].dynamicFriction=0.5f; EDefs[i].staticFriction=0.6f; EDefs[i].mass=1.0f; } // Item
    /*307 item_paper_wad*/             EDefs[307].modelIndex=487; EDefs[307].texIndex=1250; EDefs[307].collider= COLTYPE_SPH; EDefs[307].colliderCenter=(Vector3){-0.001254f,-0.001190498f,0.006335999f}; EDefs[307].colliderSize.x=0.0451f; EDefs[307].mass=0.06f;
    /*308 item_warecasing*/            EDefs[308].modelIndex=637; EDefs[308].texIndex=1251; EDefs[308].mass=0.8f;
    /*309 item_beaker*/                EDefs[309].modelIndex=14;  EDefs[309].collider=COLTYPE_CVX; EDefs[309].colliderMeshIndex=682; EDefs[309].texIndex=36; EDefs[309].specIndex=1242;  EDefs[309].mass=0.28f;
    /*310 item_beverage*/              EDefs[310].modelIndex=18;  EDefs[310].collider=COLTYPE_CVX; EDefs[310].colliderMeshIndex=683; EDefs[310].texIndex=37; EDefs[310].mass=0.12f;
    /*311 item_skull*/                 EDefs[311].modelIndex=593; EDefs[311].mass=0.451f;
    /*312 item_arm*/                   EDefs[312].modelIndex=7;   EDefs[312].texIndex=28; EDefs[312].collider=COLTYPE_CVX; EDefs[312].colliderMeshIndex=678;
    /*313 item_audiolog*/              EDefs[313].modelIndex=11;  EDefs[313].collider=COLTYPE_CVX; EDefs[313].colliderMeshIndex=679; EDefs[313].texIndex=52; EDefs[313].glowIndex=80;  EDefs[313].mass=0.2f;
    /*314 weapon_grenadefrag*/         EDefs[314].modelIndex=182;
    /*315 weapon_grenadeconc*/         EDefs[315].modelIndex=165;
    /*316 weapon_grenadeemp*/          EDefs[316].modelIndex=168;
    /*317 weapon_grenadeearth*/        EDefs[317].modelIndex=181;
    /*318 weapon_grenademine*/         EDefs[318].modelIndex=184;
    /*319 weapon_grenadenitro*/        EDefs[319].modelIndex=185;
    /*320 weapon_grenadegas*/          EDefs[320].modelIndex=183;
    /*321 item_patch_berserk*/         EDefs[321].modelIndex=488; EDefs[321].texIndex=590; EDefs[321].collider=COLTYPE_CVX; EDefs[321].colliderMeshIndex=491; EDefs[321].mass=0.12f;
    /*322 item_patch_detox*/           EDefs[322].modelIndex=488; EDefs[322].texIndex=591; EDefs[322].collider=COLTYPE_CVX; EDefs[322].colliderMeshIndex=491; EDefs[322].mass=0.12f;
    /*323 item_patch_genius*/          EDefs[323].modelIndex=488; EDefs[323].texIndex=592; EDefs[323].collider=COLTYPE_CVX; EDefs[323].colliderMeshIndex=491; EDefs[323].mass=0.12f;
    /*324 item_patch_medi*/            EDefs[324].modelIndex=488; EDefs[324].texIndex=600; EDefs[324].collider=COLTYPE_CVX; EDefs[324].colliderMeshIndex=491; EDefs[324].mass=0.12f;
    /*325 item_patch_reflex*/          EDefs[325].modelIndex=488; EDefs[325].texIndex=641; EDefs[325].collider=COLTYPE_CVX; EDefs[325].colliderMeshIndex=491; EDefs[325].mass=0.12f;
    /*326 item_patch_sight*/           EDefs[326].modelIndex=488; EDefs[326].texIndex=646; EDefs[326].collider=COLTYPE_CVX; EDefs[326].colliderMeshIndex=491; EDefs[326].mass=0.12f;
    /*327 item_patch_staminup*/        EDefs[327].modelIndex=488; EDefs[327].texIndex=647; EDefs[327].collider=COLTYPE_CVX; EDefs[327].colliderMeshIndex=491; EDefs[327].mass=0.12f;
    /*328 item_hw_system*/             EDefs[328].modelIndex=207; EDefs[328].texIndex=405; EDefs[328].glowIndex=404; EDefs[328].mass=0.17f;
    /*329 item_hw_navunit*/            EDefs[329].modelIndex=204; EDefs[329].texIndex=1258;EDefs[329].glowIndex=1259;EDefs[329].collider=COLTYPE_CVX; EDefs[329].colliderMeshIndex=696; EDefs[329].mass=0.1f;
    /*330 item_hw_ereader*/            EDefs[330].modelIndex=200; EDefs[330].collider=COLTYPE_CVX; EDefs[330].colliderMeshIndex=692; EDefs[330].mass=0.12f;
    /*331 item_hw_sensaround*/         EDefs[331].modelIndex=205; EDefs[331].collider=COLTYPE_CVX; EDefs[331].colliderMeshIndex=697; EDefs[331].mass=0.12f;
    /*332 item_hw_targetid*/           EDefs[332].modelIndex=208; EDefs[332].mass=0.08f;
    /*333 item_hw_shield*/             EDefs[333].modelIndex=206; EDefs[333].mass=0.14f;
    /*334 item_hw_bio*/                EDefs[334].modelIndex=197; EDefs[334].collider=COLTYPE_CVX; EDefs[334].colliderMeshIndex=689; EDefs[334].mass=0.1f;
    /*335 item_hw_lantern*/            EDefs[335].modelIndex=203; EDefs[335].collider=COLTYPE_CVX; EDefs[335].colliderMeshIndex=695; EDefs[335].mass=0.11f;
    /*336 item_hw_envirosuit*/         EDefs[336].modelIndex=199; EDefs[336].collider=COLTYPE_CVX; EDefs[336].colliderMeshIndex=691; EDefs[336].mass=0.451f;
    /*337 item_hw_booster*/            EDefs[337].modelIndex=198; EDefs[337].collider=COLTYPE_CVX; EDefs[337].colliderMeshIndex=690; EDefs[337].mass=0.16f;
    /*338 item_hw_jumpjets*/           EDefs[338].modelIndex=202; EDefs[338].collider=COLTYPE_CVX; EDefs[338].colliderMeshIndex=694; EDefs[338].mass=0.32f;
    /*339 item_hw_infrared*/           EDefs[339].modelIndex=201; EDefs[339].collider=COLTYPE_CVX; EDefs[339].colliderMeshIndex=693; EDefs[339].mass=0.1f;
    /*340 item_fireextinguisher*/      EDefs[340].modelIndex=144; EDefs[340].collider=COLTYPE_CVX; EDefs[340].colliderMeshIndex=684; EDefs[340].mass=1.3f;
    /*341 item_access_card_admin*/     EDefs[341].modelIndex=0;   EDefs[341].texIndex=9; EDefs[341].glowIndex=82; EDefs[341].collider=COLTYPE_CVX; EDefs[341].colliderMeshIndex=672; EDefs[341].mass=0.2f;
    /*342 item_workerhelmet*/          EDefs[342].modelIndex=648; EDefs[342].mass=1.2f;
    /*343 weapon_mk3*/                 EDefs[343].modelIndex=646; EDefs[343].mass=0.75f;
    /*344 weapon_blaster*/             EDefs[344].modelIndex=638; EDefs[344].mass=0.5f;
    /*345 weapon_dartgun*/             EDefs[345].modelIndex=640; EDefs[345].texIndex=876; EDefs[345].mass=0.3f;
    /*346 weapon_flechette*/           EDefs[346].modelIndex=642; EDefs[346].mass=0.4f;
    /*347 weapon_ionrifle*/            EDefs[347].modelIndex=643; EDefs[347].mass=0.8f;
    /*348 weapon_rapier*/              EDefs[348].modelIndex=653; EDefs[348].mass=0.3f;
    /*349 weapon_pipe*/                EDefs[349].modelIndex=649; EDefs[349].texIndex=887;  EDefs[349].mass=0.85f;
    /*350 weapon_magnum*/              EDefs[350].modelIndex=644; EDefs[350].mass=0.6f;
    /*351 weapon_magpulse*/            EDefs[351].modelIndex=645; EDefs[351].mass=0.65f;
    /*352 weapon_pistol*/              EDefs[352].modelIndex=650; EDefs[352].texIndex=878;  EDefs[352].mass=0.3f;
    /*353 weapon_plasma*/              EDefs[353].modelIndex=651; EDefs[353].mass=1.2f;
    /*354 weapon_railgun*/             EDefs[354].modelIndex=652;
    /*355 weapon_riotgun*/             EDefs[355].modelIndex=654; EDefs[355].mass=0.55f;
    /*356 weapon_skorpion*/            EDefs[356].modelIndex=655; EDefs[356].mass=1.3f;
    /*357 weapon_sparqbeam*/           EDefs[357].modelIndex=656; EDefs[357].mass=0.3f;
    /*358 weapon_stungun*/             EDefs[358].modelIndex=657; EDefs[358].mass=0.3f;
    /*359 item_battery*/               EDefs[359].modelIndex=13;  EDefs[359].collider=COLTYPE_CVX;  EDefs[359].colliderMeshIndex=680;  EDefs[359].mass=0.3f;
    /*360 item_battery_icad*/          EDefs[360].modelIndex=13;  EDefs[360].collider=COLTYPE_CVX;  EDefs[360].colliderMeshIndex=680;  EDefs[360].mass=0.35f;
    /*361 item_logic_probe*/           EDefs[361].modelIndex=217; EDefs[361].texIndex=427;  EDefs[361].mass=0.15f;
    /*362 item_healthkit*/             EDefs[362].modelIndex=196; EDefs[362].collider=COLTYPE_CVX;  EDefs[362].colliderMeshIndex=688;  EDefs[362].mass=0.25f;
    /*363 item_plastique*/             EDefs[363].modelIndex=492; EDefs[363].mass=1.4f;
    /*364 item_chipset_interfacedemod*/EDefs[364].modelIndex=45;  EDefs[364].collider=COLTYPE_BOX;  EDefs[364].colliderCenter=(Vector3){0.003744498f,0.0001704991f,0.03192701f};  EDefs[364].colliderSize=(Vector3){0.459303f,0.3412231f,0.06385402f};  EDefs[364].colliderMeshIndex=U16_MAX;  EDefs[364].mass=0.3f;
    /*365 item_flask*/                 EDefs[365].modelIndex=145; EDefs[365].collider=COLTYPE_CVX;  EDefs[365].colliderMeshIndex=685;  EDefs[365].texIndex=36;  EDefs[365].specIndex=1242;  EDefs[365].mass=0.22f;
    /*366 item_chipset_bitflag*/       EDefs[366].modelIndex=45;  EDefs[366].collider=COLTYPE_BOX;  EDefs[366].colliderCenter=(Vector3){0.003744498f,0.0001704991f,0.03192701f};  EDefs[366].colliderSize=(Vector3){0.459303f,0.3412231f,0.06385402f};  EDefs[366].colliderMeshIndex=U16_MAX;  EDefs[366].mass=0.3f;
    /*367 item_ammo_rubber*/           EDefs[367].modelIndex=8;   EDefs[367].collider=COLTYPE_CVX;  EDefs[367].colliderMeshIndex=676;  EDefs[367].mass=0.25f;
    /*368 item_isotopex22*/            EDefs[368].modelIndex=209; EDefs[368].mass=1.2f;
    /*369 item_testtube*/              EDefs[369].modelIndex=622; EDefs[369].texIndex=36; EDefs[369].specIndex=1242; EDefs[369].collider=COLTYPE_CVX; EDefs[369].colliderMeshIndex=612; EDefs[369].mass=0.21f;
    /*370 weapon_grenadefrag_live*/    EDefs[370].modelIndex=182;
    /*371 item_chipset_isolinear*/     EDefs[371].modelIndex=46;  EDefs[371].collider=COLTYPE_BOX;  EDefs[371].colliderCenter=(Vector3){-0.0009825006f,-0.0129465f,0.0148115f};  EDefs[371].colliderSize=(Vector3){0.223635f,0.4175691f,0.02912301f};  EDefs[371].colliderMeshIndex=U16_MAX;  EDefs[371].mass=0.26f;
    /*372 weapon_grenadeconc_live*/    EDefs[372].modelIndex=165;
    /*373 item_ammo_needle*/           EDefs[373].modelIndex=4;   EDefs[373].texIndex=15; EDefs[373].collider=COLTYPE_BOX; EDefs[373].colliderCenter=(Vector3){-0.0004654949f,0.0004549972f,0.0244365f}; EDefs[373].colliderSize=(Vector3){0.131339f,0.1442801f,0.04838703f}; EDefs[373].colliderMeshIndex=U16_MAX; EDefs[373].mass=0.15f;
    /*374 item_ammo_tranq*/            EDefs[374].modelIndex=4;   EDefs[374].texIndex=27; EDefs[374].collider=COLTYPE_BOX; EDefs[374].colliderCenter=(Vector3){-0.0004654949f,0.0004549972f,0.0244365f}; EDefs[374].colliderSize=(Vector3){0.131339f,0.1442801f,0.04838703f}; EDefs[374].colliderMeshIndex=U16_MAX; EDefs[374].mass=0.15f;
    /*375 item_ammo_standard*/         EDefs[375].modelIndex=5;   EDefs[375].collider=COLTYPE_BOX; EDefs[375].colliderCenter=(Vector3){0.0001984993f,0.0f,0.02172501f};  EDefs[375].colliderSize=(Vector3){0.1209471f,0.2176701f,0.04345007f};  EDefs[375].colliderMeshIndex=U16_MAX;  EDefs[375].mass=0.2f;
    /*376 item_ammo_teflon*/           EDefs[376].modelIndex=5;   EDefs[376].collider=COLTYPE_BOX; EDefs[376].colliderCenter=(Vector3){0.0001984993f,0.0f,0.02172501f};  EDefs[376].colliderSize=(Vector3){0.1209471f,0.2176701f,0.04345007f};  EDefs[376].colliderMeshIndex=U16_MAX;  EDefs[376].mass=0.2f;
    /*377 item_ammo_hollow*/           EDefs[377].modelIndex=5;   EDefs[377].collider=COLTYPE_BOX; EDefs[377].colliderCenter=(Vector3){0.0002185023f,0.0f,0.02122951f};  EDefs[377].colliderSize=(Vector3){0.1423431f,0.2127061f,0.04245907f};  EDefs[377].colliderMeshIndex=U16_MAX;  EDefs[377].mass=0.2f;
    /*378 item_ammo_slug*/             EDefs[378].modelIndex=3;   EDefs[378].collider=COLTYPE_BOX; EDefs[378].colliderCenter=(Vector3){0.0002185023f,0.0f,0.02122951f};  EDefs[378].colliderSize=(Vector3){0.1423431f,0.2127061f,0.04245907f};  EDefs[378].colliderMeshIndex=U16_MAX;  EDefs[378].glowIndex=22;  EDefs[378].mass=0.2f;
    /*379 item_ammo_magnesium*/        EDefs[379].modelIndex=3;   EDefs[379].collider=COLTYPE_CVX; EDefs[379].colliderMeshIndex=673; EDefs[379].mass=0.35f;
    /*380 item_ammo_penetrator*/       EDefs[380].modelIndex=3;   EDefs[380].collider=COLTYPE_CVX; EDefs[380].colliderMeshIndex=673; EDefs[380].mass=0.35f;
    /*381 item_ammo_hornet*/           EDefs[381].modelIndex=1;   EDefs[381].collider=COLTYPE_CVX; EDefs[381].colliderMeshIndex=673; EDefs[381].mass=0.35f;
    /*382 item_ammo_splinter*/         EDefs[382].modelIndex=630; EDefs[382].collider=COLTYPE_CVX; EDefs[382].colliderMeshIndex=673; EDefs[382].mass=0.35f;
    /*383 item_ammo_rail*/             EDefs[383].modelIndex=6;   EDefs[383].collider=COLTYPE_CVX; EDefs[383].colliderMeshIndex=675; EDefs[383].mass=0.40f;
    /*384 item_ammo_slag*/             EDefs[384].modelIndex=9;   EDefs[384].collider=COLTYPE_CVX; EDefs[384].colliderMeshIndex=673; EDefs[384].mass=0.35f;
    /*385 item_ammo_slaglarge*/        EDefs[385].modelIndex=10;  EDefs[385].collider=COLTYPE_CVX; EDefs[385].colliderMeshIndex=677; EDefs[385].mass=0.40f;
    /*386 item_ammo_magcart*/          EDefs[386].modelIndex=2;   EDefs[386].collider=COLTYPE_CVX; EDefs[386].colliderMeshIndex=674; EDefs[386].mass=0.35f;
    /*387 weapon_grenadeemp_live*/     EDefs[387].modelIndex=168;
    /*388 item_access_card_std*/       EDefs[388].modelIndex=0;   EDefs[388].texIndex=79;  EDefs[388].glowIndex=867;  EDefs[388].collider=COLTYPE_CVX;  EDefs[388].colliderMeshIndex=672;  EDefs[388].mass=0.2f;
    /*389 weapon_grenadeearth_live*/   EDefs[389].modelIndex=181;
    /*390 item_access_card_group1*/    EDefs[390].modelIndex=0;   EDefs[390].texIndex=7;  EDefs[390].glowIndex=159;  EDefs[390].collider=COLTYPE_CVX;  EDefs[390].colliderMeshIndex=672;  EDefs[390].mass=0.2f;
    /*391 item_access_card_science*/   EDefs[391].modelIndex=0;   EDefs[391].texIndex=2;  EDefs[391].glowIndex=343;  EDefs[391].collider=COLTYPE_CVX;  EDefs[391].colliderMeshIndex=672;  EDefs[391].mass=0.2f;
    /*392 item_access_card_eng*/       EDefs[392].modelIndex=0;   EDefs[392].texIndex=3;  EDefs[392].glowIndex=81;  EDefs[392].collider=COLTYPE_CVX;  EDefs[392].colliderMeshIndex=672;  EDefs[392].mass=0.2f;
    /*393 item_access_card_groupB*/    EDefs[393].modelIndex=0;   EDefs[393].texIndex=7;  EDefs[393].glowIndex=159;  EDefs[393].collider=COLTYPE_CVX;  EDefs[393].colliderMeshIndex=672;  EDefs[393].mass=0.2f;
    /*394 item_access_card_security*/  EDefs[394].modelIndex=0;   EDefs[394].texIndex=10;  EDefs[394].glowIndex=344;  EDefs[394].collider=COLTYPE_CVX;  EDefs[394].colliderMeshIndex=672;  EDefs[394].mass=0.2f;
    /*395 item_access_card_per5diego*/ EDefs[395].modelIndex=0;   EDefs[395].texIndex=8;  EDefs[395].glowIndex=341;  EDefs[395].collider=COLTYPE_CVX;  EDefs[395].colliderMeshIndex=672;  EDefs[395].mass=0.2f;
    /*396 item_access_card_medi*/      EDefs[396].modelIndex=0;   EDefs[396].texIndex=1;  EDefs[396].glowIndex=161;  EDefs[396].collider=COLTYPE_CVX;  EDefs[396].colliderMeshIndex=672;  EDefs[396].mass=0.2f;
    /*397 item_access_card_group3*/    EDefs[397].modelIndex=0;   EDefs[397].texIndex=7;  EDefs[397].glowIndex=159;  EDefs[397].collider=COLTYPE_CVX;  EDefs[397].colliderMeshIndex=672;  EDefs[397].mass=0.2f;
    /*398 item_access_card_purple*/    EDefs[398].modelIndex=0;   EDefs[398].texIndex=5;  EDefs[398].glowIndex=342;  EDefs[398].collider=COLTYPE_CVX;  EDefs[398].colliderMeshIndex=672;  EDefs[398].mass=0.2f;
    /*399 item_head_male*/             EDefs[399].modelIndex=194; EDefs[399].collider=COLTYPE_CVX; EDefs[399].colliderMeshIndex=687; EDefs[399].mass=1.29f;
    /*400 item_head_female*/           EDefs[400].modelIndex=193; EDefs[400].collider=COLTYPE_CVX; EDefs[400].colliderMeshIndex=686; EDefs[400].mass=1.30f;
    /*401 item_severedhead*/           EDefs[401].modelIndex=590; EDefs[401].mass=1.28f;
    /*402 weapon_grenademine_live*/    EDefs[402].modelIndex=184;
    /*403 weapon_grenadenitro_live*/   EDefs[403].modelIndex=185;
    /*404 weapon_grenadegas_live*/     EDefs[404].modelIndex=183;
    /*405 unused*/
    /*406 unused*/
    /*407 unused*/
    /*408 unused*/
    /*409 unused*/
    /*410 unused*/
    /*411 unused*/
    /*412 unused*/
    /*413 unused*/
    /*414 unused*/
    /*415 unused*/
    /*416 unused*/
    /*417 item_access_card_perdarcy*/  EDefs[417].modelIndex=0; EDefs[417].texIndex=8; EDefs[417].glowIndex=341; EDefs[417].collider=COLTYPE_CVX; EDefs[417].colliderMeshIndex=672; EDefs[417].mass=0.2f; EDefs[417].angularDrag=0.05f; EDefs[417].kinematic=false; EDefs[417].dynamicFriction=EDefs[417].staticFriction=0.6f;
    for (int i=419;i<=447;++i) { EDefs[i].collider=COLTYPE_CAP; EDefs[i].colliderSize.z=COLLIDER_CAPSULE_DIRECTION_Y_F; EDefs[i].staticFriction=1.0f; EDefs[i].dynamicFriction=0.15f; EDefs[i].kinematic=true; EDefs[i].mass=1.0f; EDefs[i].angularDrag=2.2f; } // NPCs
    /*419 npc_autobomb*/            EDefs[419].modelIndex=299; EDefs[419].texIndex=542; EDefs[419].colliderCenter.y=0.42f; EDefs[419].colliderCenter.z=0.01848752f;                   EDefs[419].colliderSize=(Vector3){0.42f,1.48f,COLLIDER_CAPSULE_DIRECTION_Z_F};          EDefs[419].angularDrag=1.0f; EDefs[419].glowIndex=541;
    /*420 npc_cyborg_assassin*/     EDefs[420].modelIndex=306; EDefs[420].texIndex=545; EDefs[420].numclips= 8; EDefs[420].animationNum=24;                                           EDefs[419].colliderSize.x=0.48f; EDefs[419].colliderSize.y=2.0f;  EDefs[420].mass=1.5f; EDefs[420].angularDrag=1.5f; EDefs[420].glowIndex=544;
    /*421 npc_avian_mutant*/        EDefs[421].modelIndex=328; EDefs[421].texIndex=568; EDefs[421].numclips= 5; EDefs[421].animationNum=35; EDefs[421].colliderCenter.y= 0.0200f;     EDefs[421].colliderSize.x=0.40f; EDefs[421].colliderSize.y=1.60f; EDefs[421].mass=2.0f; EDefs[421].angularDrag=1.0f;
    /*422 npc_exec_bot*/            EDefs[422].modelIndex=316; EDefs[422].texIndex=555; EDefs[422].numclips= 5; EDefs[422].animationNum=29; EDefs[422].colliderCenter.y= 0.0125f;     EDefs[422].colliderSize.x=0.48f; EDefs[422].colliderSize.y=2.025f;EDefs[422].mass=2.2f; EDefs[422].angularDrag=1.5f;
    /*423 npc_cyborg_drone*/        EDefs[423].modelIndex=312; EDefs[423].texIndex=547; EDefs[423].numclips= 7; EDefs[423].animationNum=3;                                            EDefs[423].colliderSize.x=0.36f; EDefs[423].colliderSize.y=2.00f; EDefs[423].mass=1.5f; EDefs[423].angularDrag=2.0f;
    /*424 npc_cortex_reaver*/       EDefs[424].modelIndex=300; EDefs[424].texIndex=543; EDefs[424].numclips=6;  EDefs[424].animationNum=23; EDefs[424].colliderCenter.y=-0.02263292f; EDefs[424].colliderSize.x=0.451f;                                 EDefs[424].mass=5.0f; EDefs[424].angularDrag=3.0f; EDefs[424].collider=COLTYPE_SPH;
    /*425 npc_cyborg_warrior*/      EDefs[425].modelIndex=315; EDefs[425].texIndex=554; EDefs[425].numclips=7;  EDefs[425].animationNum=28;                                           EDefs[425].colliderSize.x=0.48f; EDefs[425].colliderSize.y=2.00f; EDefs[425].mass=1.5f; EDefs[425].angularDrag=2.0f;
    /*426 npc_cyborg_enforcer*/     EDefs[426].modelIndex=314; EDefs[426].texIndex=550; EDefs[426].numclips=8;  EDefs[426].animationNum=27; EDefs[426].colliderCenter.y=0.05f;        EDefs[426].colliderSize.x=0.40f; EDefs[426].colliderSize.y=2.08f; EDefs[426].mass=1.5f;
    /*427 npc_cyborg_elite*/        EDefs[427].modelIndex=313; EDefs[427].texIndex=548; EDefs[427].numclips=10; EDefs[427].animationNum=26; EDefs[427].colliderCenter.y=0.10f;        EDefs[427].colliderSize.x=0.44f; EDefs[427].colliderSize.y=2.20f; EDefs[427].mass=3.5f;
    /*428 npc_cyborg_diego*/        EDefs[428].modelIndex=309; EDefs[428].texIndex=546; EDefs[428].numclips=6;  EDefs[428].animationNum=25;                                           EDefs[428].colliderSize.x=0.48f; EDefs[428].colliderSize.y=2.12f; EDefs[428].mass=2.0f;
    /*429 npc_sec1_bot*/            EDefs[429].modelIndex=333; EDefs[429].texIndex=573; EDefs[429].numclips=2;  EDefs[429].animationNum=38; EDefs[429].colliderCenter.y=0.05f;        EDefs[429].colliderSize.x=0.64f;                                  EDefs[429].mass=1.5f; EDefs[429].angularDrag=0.8f; EDefs[429].collider=COLTYPE_SPH;
    /*430 npc_sec2_bot*/            EDefs[430].modelIndex=335; EDefs[430].texIndex=574; EDefs[430].numclips=6;  EDefs[430].animationNum=39; EDefs[430].colliderCenter.y=0.2f;         EDefs[430].colliderSize.x=0.80f; EDefs[430].colliderSize.y=2.40f; EDefs[430].mass=4.51f;
    /*431 npc_maint_bot*/           EDefs[431].modelIndex=325; EDefs[431].texIndex=567; EDefs[431].numclips=4;  EDefs[431].animationNum=34; EDefs[431].colliderCenter.y=-0.3f;        EDefs[431].colliderSize.x=0.48f;                                  EDefs[431].mass=1.5f; EDefs[431].angularDrag=1.5f; EDefs[431].collider=COLTYPE_SPH;
    /*432 npc_mutant_cyborg*/       EDefs[432].modelIndex=329; EDefs[432].texIndex=569; EDefs[432].numclips=7;  EDefs[432].animationNum=51; EDefs[432].colliderCenter.y=0.12f;        EDefs[432].colliderSize.x=0.65f; EDefs[432].colliderSize.y=2.30f; EDefs[432].mass=3.0f; // Josiah's assumption is that the Mutant Cyborg is the "toaster oven" to inspire the first ever ECS that LGS made on Thief as they experimented more with their entity management, per Mahk interview with Casey Muratori: https://www.youtube.com/watch?v=73Do0OScoOU
    /*433 npc_hopper*/              EDefs[433].modelIndex=322; EDefs[433].texIndex=562; EDefs[433].numclips=8;  EDefs[433].animationNum=32; EDefs[433].colliderCenter.z=1.0f;         EDefs[433].colliderSize.x=0.64f; EDefs[433].colliderSize.y=2.00f; EDefs[433].angularDrag=1000.0f; EDefs[433].dynamicFriction=0.005f; EDefs[433].staticFriction=0.1f;
    /*434 npc_humanoid_mutant*/     EDefs[434].modelIndex=323; EDefs[434].texIndex=563; EDefs[434].numclips=6;  EDefs[434].animationNum=2;                                            EDefs[434].colliderSize.x=0.38f; EDefs[434].colliderSize.y=2.00f; EDefs[434].mass=1.4f; EDefs[434].angularDrag=2.0f;
    /*435 npc_invisomut*/           EDefs[435].modelIndex=324; EDefs[435].texIndex=565; EDefs[435].numclips=5;  EDefs[435].animationNum=33; EDefs[435].colliderCenter.y=-0.28938290f; EDefs[435].colliderSize=(Vector3){1.5f,1.078766f,0.8f};           EDefs[435].mass=1.3f; EDefs[435].angularDrag=0.8f; EDefs[435].collider=COLTYPE_BOX;
    /*436 npc_virus_mutant*/        EDefs[436].modelIndex=330; EDefs[436].texIndex=576; EDefs[436].numclips=6;  EDefs[436].animationNum=41; EDefs[436].colliderCenter.y=-0.05f;       EDefs[436].colliderSize.x=0.40f; EDefs[436].colliderSize.y=1.90f; EDefs[436].mass=1.4f; EDefs[436].angularDrag=2.0f;
    /*437 npc_servbot*/             EDefs[437].modelIndex=5153;EDefs[437].texIndex=575; EDefs[437].numclips=5;  EDefs[437].animationNum=40; EDefs[437].colliderMeshIndex=54; EDefs[437].mass=2.50f; EDefs[437].angularDrag=1.0f; EDefs[437].collider=COLTYPE_CVX;
    /*438 npc_flier_bot*/           EDefs[438].modelIndex=318; EDefs[438].texIndex=558; EDefs[438].numclips=5;  EDefs[438].animationNum=30; EDefs[438].mass=1.75f; EDefs[438].angularDrag=0.8f;
    /*439 npc_zerog_mutant*/        EDefs[439].modelIndex=395; EDefs[439].texIndex=1170;EDefs[439].numclips=3;  EDefs[439].animationNum=42; EDefs[439].mass=1.30f; EDefs[439].angularDrag=1.0f;
    /*440 npc_gorilla_tiger_mutant*/EDefs[440].modelIndex=320; EDefs[440].texIndex=560; EDefs[440].numclips=7;  EDefs[440].animationNum=31; EDefs[440].mass=2.00f;
    /*441 npc_repairbot*/           EDefs[441].modelIndex=331; EDefs[441].texIndex=572; EDefs[441].numclips=4;  EDefs[441].animationNum=37; EDefs[441].mass=1.50f; EDefs[441].angularDrag=2.0f;
    /*442 npc_plant_mutant*/        EDefs[442].modelIndex=330; EDefs[442].texIndex=570; EDefs[442].numclips=6;  EDefs[442].animationNum=36; EDefs[442].mass=0.80f; EDefs[442].angularDrag=1.5f;
    /*443 npc_cyberdog*/            EDefs[443].modelIndex=302; EDefs[443].mass=1.50f; EDefs[443].angularDrag=3.0f;
    /*444 npc_cyberguard*/          EDefs[444].modelIndex=303; EDefs[444].mass=2.00f; EDefs[444].angularDrag=3.0f;
    /*445 npc_cyberram*/            EDefs[445].modelIndex=304; EDefs[445].mass=2.00f; EDefs[445].angularDrag=3.0f;
    /*446 npc_cyber_reaver*/        EDefs[446].modelIndex=305; EDefs[446].mass=2.20f; EDefs[446].angularDrag=3.0f;
    /*447 npc_cybershodan*/         EDefs[447].mass=4.51f; EDefs[447].angularDrag=3.0f; EDefs[447].dynamicFriction=0.6f; EDefs[447].staticFriction=0.6f;
    for (int i=448;i<=457;++i) { EDefs[i].collider=COLTYPE_SPH; EDefs[i].colliderSize=(Vector3){1.5f,1.5f,1.5f}; } // Cyber Item Definitions
    /*448 item_cyber_data*/     EDefs[448].modelIndex=65;
    /*449 item_cyber_decoy*/    EDefs[449].modelIndex=72;
    /*450 item_cyber_drill*/    EDefs[450].modelIndex=68;
    /*451 item_cyber_game*/     EDefs[451].modelIndex=65;
    /*452 item_cyber_integrity*/EDefs[452].modelIndex=69;
    /*453 item_cyber_keycard*/  EDefs[453].modelIndex=70;
    /*454 item_cyber_pulser*/   EDefs[454].modelIndex=65;
    /*455 item_cyber_recall*/   EDefs[455].modelIndex=65;
    /*456 item_cyber_shield*/   EDefs[456].modelIndex=65;
    /*457 item_cyber_turbo*/    EDefs[457].modelIndex=65;
    for (int i=458;i<=463;++i) { EDefs[i].angularDrag=0.05f; EDefs[i].dynamicFriction=0.5f; EDefs[i].staticFriction=0.6f; EDefs[i].mass=1.5f; } // Physical Generic Objects
    /*458 prop_phys_barrel_chemical*/ EDefs[458].modelIndex=12;  EDefs[458].texIndex=30;
    /*459 prop_phys_barrel_radiation*/EDefs[459].modelIndex=12;  EDefs[459].texIndex=31;
    /*460 prop_phys_barrel_toxic*/    EDefs[460].modelIndex=12;  EDefs[460].texIndex=33;
    /*461 prop_phys_cart*/            EDefs[461].modelIndex=40;  EDefs[461].texIndex=416; EDefs[461].mass=2.5f;
    /*462 prop_phys_pot*/             EDefs[462].modelIndex=494; EDefs[462].mass=0.3f;
    /*463 prop_phys_toolcart*/        EDefs[463].modelIndex=624; EDefs[463].texIndex=865; EDefs[463].normIndex=864;  EDefs[463].specIndex=866;  EDefs[463].mass=20.0f; EDefs[463].angularDrag=0.2f;
    /*464 se_briefcase*/        EDefs[464].modelIndex=34;  EDefs[464].texIndex=66;  EDefs[464].glowIndex=65; 
    /*465 se_corpse_blueshirt*/                                   EDefs[465].texIndex=126;  EDefs[465].specIndex=127; EDefs[465].modelIndex=51; 
    /*466 se_corpse_brownshirt*/EDefs[466].texIndex=128;  EDefs[466].specIndex=129;  EDefs[466].modelIndex=52; 
    /*467 se_corpse_eaten*/     EDefs[467].texIndex=130;  EDefs[467].specIndex=131;  EDefs[467].modelIndex=53; 
    /*468 se_corpse_labcoat*/   EDefs[468].texIndex=132;  EDefs[468].specIndex=133;  EDefs[468].modelIndex=55; 
    /*469 se_corpse_security*/  EDefs[469].texIndex=136;  EDefs[469].specIndex=137;  EDefs[469].modelIndex=56; 
    /*470 se_corpse_tan*/       EDefs[470].texIndex=138;  EDefs[470].modelIndex=57; 
    /*471 se_corpse_torso*/     EDefs[471].texIndex=126;  EDefs[471].specIndex=127;  EDefs[471].modelIndex=58; 
    /*472 se_crate1*/           EDefs[472].texIndex=145;  EDefs[472].modelIndex=60;  EDefs[472].collider=COLTYPE_BOX;  EDefs[472].colliderCenter=(Vector3){0.0f,0.0f,0.3420931f};  EDefs[472].colliderSize=(Vector3){0.684186f,0.6841861f,0.6841861f};  EDefs[472].colliderMeshIndex=U16_MAX;  EDefs[472].mass=0.75f; EDefs[472].gravity=1.0f; EDefs[472].dynamicFriction=0.6f; EDefs[472].staticFriction=0.6f;
    /*473 se_crate2*/           EDefs[473].texIndex=143;  EDefs[473].modelIndex=60;  EDefs[473].collider=COLTYPE_BOX;  EDefs[473].colliderCenter=(Vector3){0.0f,0.0f,0.3420931f};  EDefs[473].colliderSize=(Vector3){0.684186f,0.6841861f,0.6841861f};  EDefs[473].colliderMeshIndex=U16_MAX;  EDefs[473].mass=0.75f; EDefs[473].gravity=1.0f; EDefs[473].dynamicFriction=0.6f; EDefs[473].staticFriction=0.6f;
    /*474 se_crate3*/           EDefs[474].texIndex=144;  EDefs[474].modelIndex=60;  EDefs[474].collider=COLTYPE_BOX;  EDefs[474].colliderCenter=(Vector3){0.0f,0.0f,0.3420931f};  EDefs[474].colliderSize=(Vector3){0.684186f,0.6841861f,0.6841861f};  EDefs[474].colliderMeshIndex=U16_MAX;  EDefs[474].mass=0.75f; EDefs[474].gravity=1.0f; EDefs[474].dynamicFriction=0.6f; EDefs[474].staticFriction=0.6f;
    /*475 se_crate4*/           EDefs[475].texIndex=146;  EDefs[475].modelIndex=60;  EDefs[475].collider=COLTYPE_BOX;  EDefs[475].colliderCenter=(Vector3){0.0f,0.0f,0.3420931f};  EDefs[475].colliderSize=(Vector3){0.684186f,0.6841861f,0.6841861f};  EDefs[475].colliderMeshIndex=U16_MAX;  EDefs[475].mass=2.25f; EDefs[475].gravity=1.0f; EDefs[475].dynamicFriction=0.6f; EDefs[475].staticFriction=0.6f;
    /*476 se_crate5*/           EDefs[476].modelIndex=60;  EDefs[476].collider=COLTYPE_BOX;  EDefs[476].colliderCenter=(Vector3){0.0f,0.0f,0.3420931f};  EDefs[476].colliderSize=(Vector3){0.684186f,0.6841861f,0.6841861f};  EDefs[476].colliderMeshIndex=U16_MAX;  EDefs[476].mass=2.25f; EDefs[476].dynamicFriction=0.6f; EDefs[476].staticFriction=0.6f;
    /*477 sec_camera*/       EDefs[477].modelIndex=589;  EDefs[477].texIndex=73;  EDefs[477].glowIndex=72; 
    /*478 sec_cpunode*/      EDefs[478].modelIndex=587;  EDefs[478].texIndex=242;  EDefs[478].glowIndex=248; 
    /*479 sec_cpunode_small*/EDefs[479].modelIndex=588;  EDefs[479].texIndex=107; 
    /*480 weapon_cyber_mine*/ EDefs[480].modelIndex=71;  EDefs[480].texIndex=1224;
    /*481 proj_enemshot2*/       EDefs[481].modelIndex=MODEL_IDX_MAX;  EDefs[481].mass=0.3f; EDefs[481].angularDrag=0.05f;  EDefs[481].gravity=0.0f;  EDefs[481].kinematic=false;  EDefs[481].dynamicFriction=0.6f;  EDefs[481].staticFriction=0.6f;
    /*482 proj_magpulse_shot*/   EDefs[482].modelIndex=MODEL_IDX_MAX;  EDefs[482].texIndex=807;  EDefs[482].mass=0.3f; EDefs[482].angularDrag=0.05f;  EDefs[482].gravity=0.0f;  EDefs[482].kinematic=false;  EDefs[482].dynamicFriction=0.6f;  EDefs[482].staticFriction=0.6f;
    /*483 proj_stungun_shot*/    EDefs[483].modelIndex=MODEL_IDX_MAX;  EDefs[483].texIndex=835;  EDefs[483].mass=0.3f; EDefs[483].angularDrag=0.05f;  EDefs[483].gravity=0.0f;  EDefs[483].kinematic=false;  EDefs[483].dynamicFriction=0.6f;  EDefs[483].staticFriction=0.6f;
    /*484 proj_rail_shot*/       EDefs[484].modelIndex=652;  EDefs[484].mass=0.3f; EDefs[484].angularDrag=0.05f;  EDefs[484].gravity=0.0f;  EDefs[484].kinematic=false;  EDefs[484].dynamicFriction=0.6f;  EDefs[484].staticFriction=0.6f;
    /*485 proj_plasmarifle_shot*/EDefs[485].modelIndex=651;  EDefs[485].mass=0.3f;  EDefs[485].angularDrag=0.05f;  EDefs[485].gravity=0.0f;  EDefs[485].kinematic=false;  EDefs[485].dynamicFriction=0.1f;  EDefs[485].staticFriction=0.2f;  EDefs[485].bounciness=0.9f;
    /*486 proj_enemshot6*/       EDefs[486].modelIndex=MODEL_IDX_MAX;  EDefs[486].mass=0.3f; EDefs[486].angularDrag=0.05f;  EDefs[486].gravity=0.0f;  EDefs[486].kinematic=false;  EDefs[486].dynamicFriction=0.6f;  EDefs[486].staticFriction=0.6f;
    /*487 proj_enemshot5*/       EDefs[487].modelIndex=MODEL_IDX_MAX;  EDefs[487].mass=0.2f; EDefs[487].angularDrag=0.05f;  EDefs[487].gravity=0.0f;  EDefs[487].kinematic=false;  EDefs[487].dynamicFriction=0.6f;  EDefs[487].staticFriction=0.6f;
    /*488 proj_enemshot4*/       EDefs[488].modelIndex=MODEL_IDX_MAX;  EDefs[488].mass=0.3f; EDefs[488].angularDrag=0.05f;  EDefs[488].gravity=0.0f;  EDefs[488].kinematic=false;  EDefs[488].dynamicFriction=0.6f;  EDefs[488].staticFriction=0.6f;
    /*489 proj_throwingstar*/    EDefs[489].modelIndex=307;  EDefs[489].mass=0.3f;  EDefs[489].angularDrag=0.05f;  EDefs[489].gravity=0.0f; EDefs[489].dynamicFriction=0.6f;  EDefs[489].staticFriction=0.6f;
    /*490 proj_magpulsenpc_shot*/EDefs[490].modelIndex=645;  EDefs[490].mass=0.3f; EDefs[490].angularDrag=0.05f;  EDefs[490].gravity=0.0f; EDefs[490].dynamicFriction=0.6f;  EDefs[490].staticFriction=0.6f;
    /*491 proj_railnpc_shot*/    EDefs[491].modelIndex=MODEL_IDX_MAX;  EDefs[491].mass=0.3f;  EDefs[491].angularDrag=0.05f;  EDefs[491].gravity=0.0f; EDefs[491].dynamicFriction=0.6f;  EDefs[491].staticFriction=0.6f;
    /*492 proj_cyberplayer_shot*/EDefs[492].modelIndex=72;  EDefs[492].mass=0.3f; EDefs[492].angularDrag=0.05f;  EDefs[492].gravity=0.0f;
    /*493 proj_cyberdog_shot*/   EDefs[493].modelIndex=63;  EDefs[493].mass=0.3f;  EDefs[493].angularDrag=0.05f;  EDefs[493].gravity=0.0f; EDefs[493].dynamicFriction=0.6f;  EDefs[493].staticFriction=0.6f;
    /*494 proj_cyberreaver_shot*/EDefs[494].modelIndex=64;  EDefs[494].mass=0.3f;  EDefs[494].angularDrag=0.05f;  EDefs[494].gravity=0.0f; EDefs[494].dynamicFriction=0.6f;  EDefs[494].staticFriction=0.6f;
    /*495 proj_cyberice_shot*/   EDefs[495].modelIndex=68;  EDefs[495].mass=0.3f; EDefs[495].angularDrag=0.05f;  EDefs[495].gravity=0.0f; EDefs[495].dynamicFriction=0.6f;  EDefs[495].staticFriction=0.6f;
    for (int i=496;i<515;++i) { EDefs[i].SFXIndex = 75; EDefs[i].collider=COLTYPE_MSH;  } // Doors
    /*496 doorA*/         EDefs[496].modelIndex=719; EDefs[496].texIndex=185; EDefs[496].numclips=4;    EDefs[496].animationNum=1;                           
    /*497 doorB*/         EDefs[497].modelIndex=0;  EDefs[497].texIndex=189;  EDefs[497].glowIndex=188; EDefs[497].numclips=4;    EDefs[497].animationNum=0;
    /*498 doorC*/         EDefs[498].modelIndex=0;  EDefs[498].texIndex=184;  EDefs[498].numclips=4;    EDefs[498].animationNum=5;
    /*499 doorD*/         EDefs[499].modelIndex=0;  EDefs[499].numclips=4;  EDefs[499].animationNum=4;  EDefs[499].texIndex=196;  EDefs[499].glowIndex=197;
    /*500 doorE*/         EDefs[500].modelIndex=0;  EDefs[500].numclips=4;  EDefs[500].animationNum=9;  EDefs[500].texIndex=208;  EDefs[500].glowIndex=207;
    /*501 doorF*/         EDefs[501].modelIndex=0;  EDefs[501].numclips=4;  EDefs[501].animationNum=10; EDefs[501].texIndex=187;
    /*502 doorG*/         EDefs[502].modelIndex=0;  EDefs[502].numclips=4;  EDefs[502].animationNum=11; EDefs[502].texIndex=193;  EDefs[502].glowIndex=194;
    /*503 doorH*/         EDefs[503].modelIndex=0;  EDefs[503].numclips=4;  EDefs[503].animationNum=12; EDefs[503].texIndex=190;
    /*504 doorI*/         EDefs[504].modelIndex=0;  EDefs[504].numclips=4;  EDefs[504].animationNum=13; EDefs[504].texIndex=200;  EDefs[504].glowIndex=199;
    /*505 doorJ*/         EDefs[505].modelIndex=0;  EDefs[505].numclips=4;  EDefs[505].animationNum=6;  EDefs[505].texIndex=215;
    /*506 doorK*/         EDefs[506].modelIndex=0;  EDefs[506].numclips=4;  EDefs[506].animationNum=7;  EDefs[506].texIndex=214;
    /*507 doorL*/         EDefs[507].modelIndex=0;  EDefs[507].numclips=4;  EDefs[507].animationNum=8;  EDefs[507].texIndex=191;
    /*508 door_elevator1*/EDefs[508].modelIndex=0;  EDefs[508].numclips=4;  EDefs[508].animationNum=14;  EDefs[508].texIndex=202;
    /*509 door_elevator2*/EDefs[509].modelIndex=0;  EDefs[509].numclips=4;  EDefs[509].animationNum=15;  EDefs[509].texIndex=203;
    /*510 door_elevator3*/EDefs[510].modelIndex=0;  EDefs[510].numclips=4;  EDefs[510].animationNum=16;  EDefs[510].texIndex=206;  EDefs[510].glowIndex=205;
    /*511 door_elevator4*/EDefs[511].modelIndex=0;  EDefs[511].numclips=4;  EDefs[511].animationNum=17;  EDefs[511].texIndex=203;
    /*512 door_secret1*/  EDefs[512].modelIndex=0;  EDefs[512].numclips=4;  EDefs[512].animationNum=19;  EDefs[512].texIndex=210;
    /*513 door_secret2*/  EDefs[513].modelIndex=0;  EDefs[513].numclips=4;  EDefs[513].animationNum=18;  EDefs[513].texIndex=209;
    /*514 door_secret3*/  EDefs[514].modelIndex=94;  EDefs[514].numclips=4;  EDefs[514].animationNum=20;  EDefs[514].texIndex=211;
    /*515 func_forcebridge*/ EDefs[515].modelIndex=78;  EDefs[515].texIndex=38; EDefs[476].collider=COLTYPE_BOX;
    /*516 prop_lift2*/       EDefs[516].modelIndex=215;  EDefs[516].texIndex=155;  EDefs[516].glowIndex=154;  EDefs[516].collider=COLTYPE_BOX;  EDefs[516].colliderCenter=(Vector3){0.0f,0.0f,0.0f};  EDefs[516].colliderSize=(Vector3){1.0f,1.0f,1.0f};  EDefs[516].colliderMeshIndex=U16_MAX; 
    /*517 func_wall*/        EDefs[517].mass=10.0f;  EDefs[517].angularDrag=0.05f;  EDefs[517].gravity=0.0f;  EDefs[517].kinematic=true;  EDefs[517].dynamicFriction=0.6f;  EDefs[517].staticFriction=0.6f;
    /*518 BulletHoleLarge*/
    /*519 BulletHoleScorchLarge*/
    /*520 BulletHoleScorchSmall*/
    /*521 BulletHoleSmall*/
    /*522 BulletHoleTiny*/
    /*523 BulletHoleTinySpread*/
    /*524 func_door_cyber*/ EDefs[524].modelIndex=178;  EDefs[524].texIndex=1224;  EDefs[524].collider=COLTYPE_BOX;  EDefs[524].colliderCenter=(Vector3){0.0f,1.31f,0.0f};  EDefs[524].colliderSize=(Vector3){2.56f,0.06f,2.56f};  EDefs[524].colliderMeshIndex=U16_MAX; 
    /*525 prop_console01*/ EDefs[525].texIndex=100;  EDefs[525].modelIndex=49; 
    /*526 prop_console02*/ EDefs[526].texIndex=100;  EDefs[526].modelIndex=50; 
    /*527 prop_grate1_1*/ EDefs[527].modelIndex=186;  EDefs[527].texIndex=359; 
    /*528 prop_grate1_2*/ EDefs[528].modelIndex=187;  EDefs[528].texIndex=360; 
    /*529 prop_grate1_3*/ EDefs[529].modelIndex=188;  EDefs[529].texIndex=361; 
    MemCpyFromBtoAForNBytes(EDefs[530].path,"530 se_cabinet",11); EDefs[530].modelIndex=39;  EDefs[530].texIndex=70; 
    MemCpyFromBtoAForNBytes(EDefs[531].path,"531 se_thermos",11); EDefs[531].texIndex=863;  EDefs[531].modelIndex=623; 
    MemCpyFromBtoAForNBytes(EDefs[532].path,"532 prop_beaker_holder",19); EDefs[532].modelIndex=15;  EDefs[532].texIndex=36; 
    MemCpyFromBtoAForNBytes(EDefs[533].path,"533 prop_bed",9); EDefs[533].modelIndex=16;  EDefs[533].texIndex=246; 
    MemCpyFromBtoAForNBytes(EDefs[534].path,"534 prop_bed_hospital",18); EDefs[534].modelIndex=608;  EDefs[534].texIndex=759; 
    MemCpyFromBtoAForNBytes(EDefs[535].path,"535 prop_bed_neurosurgery",22); EDefs[535].texIndex=18;  EDefs[535].normIndex=29;  EDefs[535].specIndex=1238;  EDefs[535].modelIndex=17; 
    MemCpyFromBtoAForNBytes(EDefs[536].path,"536 prop_bonepile1",15); EDefs[536].modelIndex=19;  EDefs[536].texIndex=815; 
    MemCpyFromBtoAForNBytes(EDefs[537].path,"537 prop_bridgewall1",17); EDefs[537].modelIndex=33; 
    MemCpyFromBtoAForNBytes(EDefs[538].path,"538 prop_broken_clock",18); EDefs[538].modelIndex=38;  EDefs[538].texIndex=1117;  EDefs[538].altTexIndex=1118;  EDefs[538].glowIndex=1115;  EDefs[538].altGlowIndex=1116; 
    MemCpyFromBtoAForNBytes(EDefs[539].path,"539 prop_brokengun",15); EDefs[539].modelIndex=639;  EDefs[539].texIndex=878; 
    MemCpyFromBtoAForNBytes(EDefs[540].path,"540 prop_chair01",13); EDefs[540].modelIndex=41;  EDefs[540].texIndex=195; 
    MemCpyFromBtoAForNBytes(EDefs[541].path,"541 prop_chair02",13); EDefs[541].modelIndex=42;  EDefs[541].texIndex=195; 
    MemCpyFromBtoAForNBytes(EDefs[542].path,"542 prop_chair03",13); EDefs[542].modelIndex=43;  EDefs[542].texIndex=195; 
    MemCpyFromBtoAForNBytes(EDefs[543].path,"543 prop_chair04",13); EDefs[543].modelIndex=41;  EDefs[543].texIndex=195; 
    MemCpyFromBtoAForNBytes(EDefs[544].path,"544 prop_chair05",13); EDefs[544].modelIndex=42;  EDefs[544].texIndex=195; 
    MemCpyFromBtoAForNBytes(EDefs[545].path,"545 prop_chandelier",16); EDefs[545].modelIndex=496;  EDefs[545].texIndex=644; 
    MemCpyFromBtoAForNBytes(EDefs[546].path,"546 prop_charge_station",20); EDefs[546].modelIndex=44;  EDefs[546].texIndex=77;  EDefs[546].glowIndex=76; 
    MemCpyFromBtoAForNBytes(EDefs[547].path,"547 prop_clothes",13); EDefs[547].modelIndex=47;  EDefs[547].texIndex=97; 
    MemCpyFromBtoAForNBytes(EDefs[548].path,"548 prop_computer",14); EDefs[548].modelIndex=48; 
    MemCpyFromBtoAForNBytes(EDefs[549].path,"549 prop_couch",11); EDefs[549].modelIndex=59; 
    MemCpyFromBtoAForNBytes(EDefs[550].path,"550 prop_couch2",12); EDefs[550].modelIndex=59; 
    MemCpyFromBtoAForNBytes(EDefs[551].path,"551 prop_cpuscreen",15); EDefs[551].modelIndex=178;  EDefs[551].texIndex=768; 
    MemCpyFromBtoAForNBytes(EDefs[552].path,"552 prop_cyber_datafrag",20); EDefs[552].modelIndex=78; 
    MemCpyFromBtoAForNBytes(EDefs[553].path,"553 prop_cyber_decoy",17); EDefs[553].modelIndex=78; 
    MemCpyFromBtoAForNBytes(EDefs[554].path,"554 prop_cyber_exit",16); EDefs[554].modelIndex=78; 
    MemCpyFromBtoAForNBytes(EDefs[555].path,"555 prop_cyber_switch",18); EDefs[555].modelIndex=0; 
    MemCpyFromBtoAForNBytes(EDefs[556].path,"556 prop_cyberport",15); EDefs[556].modelIndex=62;  EDefs[556].texIndex=117;  EDefs[556].glowIndex=116; 
    MemCpyFromBtoAForNBytes(EDefs[557].path,"557 prop_desk01",12); EDefs[557].modelIndex=74;  EDefs[557].texIndex=125; 
    MemCpyFromBtoAForNBytes(EDefs[558].path,"558 prop_desk02",12); EDefs[558].modelIndex=75;  EDefs[558].texIndex=124; 
    MemCpyFromBtoAForNBytes(EDefs[559].path,"559 prop_dexmissile",16); EDefs[559].modelIndex=76;  EDefs[559].texIndex=164;  EDefs[559].glowIndex=162; 
    MemCpyFromBtoAForNBytes(EDefs[560].path,"560 prop_foliage_fernpoison",24); EDefs[560].modelIndex=160;  EDefs[560].texIndex=331; 
    MemCpyFromBtoAForNBytes(EDefs[561].path,"561 prop_foliage_bush",18); EDefs[561].modelIndex=495;  EDefs[561].texIndex=643;  EDefs[561].glowIndex=642; 
    MemCpyFromBtoAForNBytes(EDefs[562].path,"562 prop_foliage_fern",18); EDefs[562].modelIndex=160;  EDefs[562].texIndex=333;  EDefs[562].glowIndex=330; 
    MemCpyFromBtoAForNBytes(EDefs[563].path,"563 prop_foliage_fernblueflower",28); EDefs[563].modelIndex=159;  EDefs[563].texIndex=333;  EDefs[563].glowIndex=330; 
    MemCpyFromBtoAForNBytes(EDefs[564].path,"564 prop_foliage_pinetreem",23); EDefs[564].modelIndex=489;  EDefs[564].texIndex=594; 
    MemCpyFromBtoAForNBytes(EDefs[565].path,"565 prop_foliage_poisonbush1",25); EDefs[565].modelIndex=493; EDefs[565].texIndex=638;
    MemCpyFromBtoAForNBytes(EDefs[566].path,"566 prop_gear_large",16); EDefs[566].modelIndex=166; EDefs[566].texIndex=335;
    MemCpyFromBtoAForNBytes(EDefs[567].path,"567 prop_gear_small",16); EDefs[567].modelIndex=167; EDefs[567].texIndex=336;
    MemCpyFromBtoAForNBytes(EDefs[568].path,"568 prop_grass1",12); EDefs[568].texIndex=329;
    MemCpyFromBtoAForNBytes(EDefs[569].path,"569 prop_grass2",12); EDefs[569].texIndex=329;
    MemCpyFromBtoAForNBytes(EDefs[570].path,"570 prop_grass3",12); EDefs[570].texIndex=329;
    MemCpyFromBtoAForNBytes(EDefs[571].path,"571 prop_grass4",12); EDefs[571].texIndex=329;
    MemCpyFromBtoAForNBytes(EDefs[572].path,"572 prop_grass5",12); EDefs[572].texIndex=329;
    MemCpyFromBtoAForNBytes(EDefs[573].path,"573 prop_grate4",12); EDefs[573].modelIndex=161;  EDefs[573].texIndex=329; 
    MemCpyFromBtoAForNBytes(EDefs[574].path,"574 prop_healingbed",16); EDefs[574].modelIndex=195;  EDefs[574].texIndex=1139; 
    MemCpyFromBtoAForNBytes(EDefs[575].path,"575 prop_lamp",10); EDefs[575].modelIndex=212;  EDefs[575].texIndex=423; 
    MemCpyFromBtoAForNBytes(EDefs[576].path,"576 prop_light_emergsignal",23); EDefs[576].modelIndex=216;  EDefs[576].texIndex=426;  EDefs[576].altTexIndex=424;  EDefs[576].glowIndex=0;  EDefs[576].altGlowIndex=424; 
    MemCpyFromBtoAForNBytes(EDefs[577].path,"577 prop_microscope",16); EDefs[577].modelIndex=298;  EDefs[577].texIndex=645;  EDefs[577].specIndex=1241; 
    MemCpyFromBtoAForNBytes(EDefs[578].path,"578 prop_pipe",10); EDefs[578].modelIndex=490;  EDefs[578].texIndex=595; 
    MemCpyFromBtoAForNBytes(EDefs[579].path,"579 prop_puddle",12); EDefs[579].modelIndex=157;  EDefs[579].texIndex=648; 
    MemCpyFromBtoAForNBytes(EDefs[580].path,"580 prop_puddle_grease",19); EDefs[580].modelIndex=157;  EDefs[580].texIndex=650; 
    MemCpyFromBtoAForNBytes(EDefs[581].path,"581 prop_puddle_oil",16); EDefs[581].modelIndex=157;  EDefs[581].texIndex=652; 
    MemCpyFromBtoAForNBytes(EDefs[582].path,"582 prop_shelves",13); EDefs[582].modelIndex=591;  EDefs[582].texIndex=94; 
    MemCpyFromBtoAForNBytes(EDefs[583].path,"583 prop_skeleton",14); EDefs[583].modelIndex=592;  EDefs[583].texIndex=815; 
    MemCpyFromBtoAForNBytes(EDefs[584].path,"584 prop_sleeping_cables",21); EDefs[584].modelIndex=595;  EDefs[584].texIndex=71; 
    MemCpyFromBtoAForNBytes(EDefs[585].path,"585 prop_sparkingwire",18); EDefs[585].modelIndex=0;  EDefs[585].numclips=1;  EDefs[585].animationNum=46;  EDefs[585].texIndex=71; 
    MemCpyFromBtoAForNBytes(EDefs[586].path,"586 prop_table",11); EDefs[586].modelIndex=619;  EDefs[586].texIndex=92; 
    MemCpyFromBtoAForNBytes(EDefs[587].path,"587 prop_tv_on_a_post",18); EDefs[587].modelIndex=625;  EDefs[587].texIndex=1228; 
    MemCpyFromBtoAForNBytes(EDefs[588].path,"588 prop_vendingmachines1",22); EDefs[588].modelIndex=627;  EDefs[588].texIndex=870; 
    MemCpyFromBtoAForNBytes(EDefs[589].path,"589 prop_vendingmachines2",22); EDefs[589].modelIndex=614;  EDefs[589].texIndex=871; 
    MemCpyFromBtoAForNBytes(EDefs[590].path,"590 prop_weapon_rack",17); EDefs[590].modelIndex=641;  EDefs[590].texIndex=113; 
    MemCpyFromBtoAForNBytes(EDefs[591].path,"591 prop_xray",10); EDefs[591].modelIndex=660;  EDefs[591].texIndex=153; 
    MemCpyFromBtoAForNBytes(EDefs[592].path,"592 text_decal",11); EDefs[592].modelIndex=77; 
    MemCpyFromBtoAForNBytes(EDefs[593].path,"593 text_decalStopDSS1",19); EDefs[593].modelIndex=77; 
    MemCpyFromBtoAForNBytes(EDefs[594].path,"594 trigger_counter",16);
    MemCpyFromBtoAForNBytes(EDefs[595].path,"565 trigger_cyberpush",18);
    MemCpyFromBtoAForNBytes(EDefs[596].path,"596 trigger_gravitylift",20);
    MemCpyFromBtoAForNBytes(EDefs[597].path,"597 trigger_ladder",15);
    MemCpyFromBtoAForNBytes(EDefs[598].path,"598 trigger_multiple",17);
    MemCpyFromBtoAForNBytes(EDefs[599].path,"599 trigger_music",14);
    MemCpyFromBtoAForNBytes(EDefs[600].path,"600 trigger_once",13);
    MemCpyFromBtoAForNBytes(EDefs[601].path,"601 trigger_radiation",18);
    MemCpyFromBtoAForNBytes(EDefs[602].path,"602 us_isotopepanel",16); EDefs[602].modelIndex=0;  EDefs[602].texIndex=616;  EDefs[602].numclips=5;  EDefs[602].animationNum=44; 
    MemCpyFromBtoAForNBytes(EDefs[603].path,"603 us_paperlog",12); EDefs[603].modelIndex=486;  EDefs[603].texIndex=580; 
    MemCpyFromBtoAForNBytes(EDefs[604].path,"604 us_puz_elevatorkeypad",22); EDefs[604].modelIndex=615;  EDefs[604].texIndex=247; 
    MemCpyFromBtoAForNBytes(EDefs[605].path,"605 us_puz_elevatorkeypad2",23); EDefs[605].modelIndex=618;  EDefs[605].texIndex=250; 
    MemCpyFromBtoAForNBytes(EDefs[606].path,"606 us_puz_elevatorkeypad3",23); EDefs[606].modelIndex=615;  EDefs[606].texIndex=247; 
    MemCpyFromBtoAForNBytes(EDefs[607].path,"607 us_puz_elevatorkeypad4",23); EDefs[607].modelIndex=210;  EDefs[607].texIndex=249; 
    MemCpyFromBtoAForNBytes(EDefs[608].path,"608 us_puz_keypad",14); EDefs[608].modelIndex=211;  EDefs[608].texIndex=414; 
    MemCpyFromBtoAForNBytes(EDefs[609].path,"609 us_puz_panel_blue_grid",23); EDefs[609].modelIndex=0;  EDefs[609].texIndex=604;  EDefs[609].numclips=3;  EDefs[609].animationNum=43; 
    MemCpyFromBtoAForNBytes(EDefs[610].path,"610 us_puz_panel_brown_grid",24); EDefs[610].modelIndex=0;  EDefs[610].texIndex=604;  EDefs[610].numclips=3;  EDefs[610].animationNum=43; 
    MemCpyFromBtoAForNBytes(EDefs[611].path,"611 us_puz_panel_gray_grid",23); EDefs[611].modelIndex=0;  EDefs[611].texIndex=634;  EDefs[611].numclips=3;  EDefs[611].animationNum=43; 
    MemCpyFromBtoAForNBytes(EDefs[612].path,"612 us_puz_panel_red_grid",22); EDefs[612].modelIndex=0;  EDefs[612].texIndex=625;  EDefs[612].numclips=3;  EDefs[612].animationNum=43; 
    MemCpyFromBtoAForNBytes(EDefs[613].path,"613 us_puz_panel_teal_grid",23); EDefs[613].modelIndex=0;  EDefs[613].texIndex=601;  EDefs[613].numclips=3;  EDefs[613].animationNum=43; 
    MemCpyFromBtoAForNBytes(EDefs[614].path,"614 us_relaypanel",14); EDefs[614].modelIndex=0;  EDefs[614].texIndex=617;  EDefs[614].numclips=4;  EDefs[614].animationNum=45; 
    MemCpyFromBtoAForNBytes(EDefs[615].path,"615 us_retinalscanner",18); EDefs[615].modelIndex=79;  EDefs[615].texIndex=46; 
    MemCpyFromBtoAForNBytes(EDefs[616].path,"616 prop_vending1_1",16); EDefs[616].modelIndex=627;  EDefs[616].texIndex=870; 
    MemCpyFromBtoAForNBytes(EDefs[617].path,"617 prop_vending1_2",16); EDefs[617].modelIndex=628;  EDefs[617].texIndex=870; 
    MemCpyFromBtoAForNBytes(EDefs[618].path,"618 prop_vending1_3",16); EDefs[618].modelIndex=629;  EDefs[618].texIndex=870; 
    MemCpyFromBtoAForNBytes(EDefs[619].path,"619 prop_vending2_1",16); EDefs[619].modelIndex=614;  EDefs[619].texIndex=871; 
    MemCpyFromBtoAForNBytes(EDefs[620].path,"620 prop_vending2_2",16); EDefs[620].modelIndex=621;  EDefs[620].texIndex=871; 
    MemCpyFromBtoAForNBytes(EDefs[621].path,"621 ambient_airhiss",16); EDefs[621].volume=0.05f;
    MemCpyFromBtoAForNBytes(EDefs[622].path,"622 ambient_clicker",16); EDefs[622].volume=0.20f;
    MemCpyFromBtoAForNBytes(EDefs[623].path,"623 ambient_compressor",19); EDefs[623].volume=0.4f;
    MemCpyFromBtoAForNBytes(EDefs[624].path,"624 ambient_dishwasher",19); EDefs[624].volume=0.2f;
    MemCpyFromBtoAForNBytes(EDefs[625].path,"625 ambient_drip_amb",17); EDefs[625].volume=0.5f;
    MemCpyFromBtoAForNBytes(EDefs[626].path,"626 ambient_fan",12); EDefs[626].volume=0.3f;
    MemCpyFromBtoAForNBytes(EDefs[627].path,"627 ambient_generator_gas",22); EDefs[627].volume=0.3f;
    MemCpyFromBtoAForNBytes(EDefs[628].path,"628 ambient_gurgle",15); EDefs[628].volume=0.3f;
    MemCpyFromBtoAForNBytes(EDefs[629].path,"629 ambient_icemaker",17); EDefs[629].volume=0.6f;
    MemCpyFromBtoAForNBytes(EDefs[630].path,"630 ambient_intake",15); EDefs[630].volume=0.2f;
    MemCpyFromBtoAForNBytes(EDefs[631].path,"631 ambient_lathe",14); EDefs[631].volume=0.4f;
    MemCpyFromBtoAForNBytes(EDefs[632].path,"632 ambient_lev3loop1",18); EDefs[632].volume=0.1f;
    MemCpyFromBtoAForNBytes(EDefs[633].path,"633 ambient_lev3loop2",18); EDefs[633].volume=0.1f;
    MemCpyFromBtoAForNBytes(EDefs[634].path,"634 ambient_lev3loop3",18); EDefs[634].volume=0.1f;
    MemCpyFromBtoAForNBytes(EDefs[635].path,"635 ambient_lev3loop4",18); EDefs[635].volume=0.1f;
    MemCpyFromBtoAForNBytes(EDefs[636].path,"636 ambient_liquid_bubble",22); EDefs[636].volume=1.0f;
    MemCpyFromBtoAForNBytes(EDefs[637].path,"637 ambient_liquid_lava2",21); EDefs[637].volume=0.4f;
    MemCpyFromBtoAForNBytes(EDefs[638].path,"638 ambient_looping",16); EDefs[638].volume=0.4f;
    MemCpyFromBtoAForNBytes(EDefs[639].path,"639 ambient_machgear_loop",22); EDefs[639].volume=0.4f;
    MemCpyFromBtoAForNBytes(EDefs[640].path,"640 ambient_machine_ambience",25); EDefs[640].volume=0.8f;
    MemCpyFromBtoAForNBytes(EDefs[641].path,"641 ambient_machine_go",19); EDefs[641].volume=0.6f;
    MemCpyFromBtoAForNBytes(EDefs[642].path,"642 ambient_machine_humamb7",24); EDefs[642].volume=1.0f;
    MemCpyFromBtoAForNBytes(EDefs[643].path,"643 ambient_machine_humlonoise",27); EDefs[643].volume=0.4f;
    /*644 ambient_machine_loop1*/ EDefs[644].volume=0.4f;
    /*645 ambient_machine_loop2*/ EDefs[645].volume=0.4f;
    /*646 ambient_machinea1*/ EDefs[646].volume=0.4f;
    /*647 ambient_machinevat_loop*/ EDefs[647].volume=0.8f;
    /*648 ambient_mist*/ EDefs[648].volume=0.02f;
    /*649 ambient_pipewater_loop*/ EDefs[649].volume=0.65f;
    /*650 ambient_powerloom*/ EDefs[650].volume=0.3f;
    /*651 ambient_pump*/ EDefs[651].volume=0.2f;
    /*652 ambient_pump2*/ EDefs[652].volume=0.05f;
    /*653 ambient_rain*/ EDefs[653].volume=0.55f;
    /*654 ambient_steam_loop*/ EDefs[654].volume=0.1f;
    /*655 ambient_washing_machine*/ EDefs[655].volume=0.5f;
    /*656 decal_blood_die*/ EDefs[656].modelIndex=77;  EDefs[656].texIndex=237;
    /*657 decal_blood_resist*/ EDefs[657].modelIndex=77;  EDefs[657].texIndex=240;
    /*658 decal_blood_stayaway*/ EDefs[658].modelIndex=77;  EDefs[658].texIndex=235;
    /*659 decal_blood_words2*/ EDefs[659].modelIndex=77;  EDefs[659].texIndex=236;
    /*660 decal_bloodfonta*/ EDefs[660].modelIndex=178;  EDefs[660].texIndex=118;
    /*661 decal_bloodfonte*/ EDefs[661].modelIndex=178;  EDefs[661].texIndex=121;
    /*662 decal_bloodfontg*/ EDefs[662].modelIndex=178;  EDefs[662].texIndex=122;
    /*663 decal_bloodfonth*/ EDefs[663].modelIndex=178;  EDefs[663].texIndex=89;
    /*664 decal_bloodfontr*/ EDefs[664].modelIndex=178;  EDefs[664].texIndex=139;
    /*665 decal_bloodfonty*/ EDefs[665].modelIndex=178;  EDefs[665].texIndex=140;
    /*666 decal_bloodsplat2*/ EDefs[666].modelIndex=157;  EDefs[666].texIndex=130;
    /*667 decal_logo_antenna*/ EDefs[667].modelIndex=77;  EDefs[667].texIndex=182;
    /*668 decal_logo_armory*/ EDefs[668].modelIndex=77;  EDefs[668].texIndex=178;
    /*669 decal_logo_biohazard*/ EDefs[669].modelIndex=77;  EDefs[669].texIndex=180;
    /*670 decal_logo_bridge*/ EDefs[670].modelIndex=77;  EDefs[670].texIndex=181;
    /*671 decal_logo_cyborg*/ EDefs[671].modelIndex=77;  EDefs[671].texIndex=176;
    /*672 decal_logo_gears*/ EDefs[672].modelIndex=77;  EDefs[672].texIndex=174;
    /*673 decal_logo_medical*/ EDefs[673].modelIndex=77;  EDefs[673].texIndex=165;
    /*674 decal_logo_radhazard*/ EDefs[674].modelIndex=77;  EDefs[674].texIndex=177;
    /*675 decal_logo_research*/ EDefs[675].modelIndex=77;  EDefs[675].texIndex=175;
    /*676 decal_logo_security*/ EDefs[676].modelIndex=77;  EDefs[676].texIndex=167;
    /*677 decal_painting1*/ EDefs[677].modelIndex=77;  EDefs[677].texIndex=218;  EDefs[677].glowIndex=216;  EDefs[677].normIndex=217;
    /*678 decal_painting2*/ EDefs[678].modelIndex=77;  EDefs[678].texIndex=220;  EDefs[678].glowIndex=219;
    /*679 decal_painting3*/ EDefs[679].modelIndex=77;  EDefs[679].texIndex=222;  EDefs[679].glowIndex=221;
    /*680 decal_posterbetterfuture*/ EDefs[680].modelIndex=77;  EDefs[680].texIndex=226;  EDefs[680].normIndex=225;
    /*681 decal_postergenetics*/ EDefs[681].modelIndex=77;  EDefs[681].texIndex=224;  EDefs[681].normIndex=223;
    /*682 decal_scorch1*/        EDefs[682].modelIndex=77;  EDefs[682].texIndex=227;
    /*683 decal_scorch2*/        EDefs[683].modelIndex=77;  EDefs[683].texIndex=228;
    /*684 decal_scorch3*/        EDefs[684].modelIndex=77;  EDefs[684].texIndex=229;
    /*685 decal_scorch4*/        EDefs[685].modelIndex=77;  EDefs[685].texIndex=230;
    /*686 decal_scorchtiny*/     EDefs[686].modelIndex=77;  EDefs[686].texIndex=232;
    /*687 decal_blood_splat*/    EDefs[687].modelIndex=77;  EDefs[687].texIndex=234;
    /*688 func_switch1*/         EDefs[688].modelIndex=609;  EDefs[688].texIndex=837;  EDefs[688].collider=COLTYPE_BOX;  EDefs[688].colliderCenter=(Vector3){0.0f,0.0f,0.0f};  EDefs[688].colliderSize=(Vector3){0.32f,0.04f,0.32f}; EDefs[688].colliderMeshIndex=U16_MAX; 
    /*689 func_switch2*/         EDefs[689].modelIndex=610;  EDefs[689].texIndex=839;  EDefs[689].mainSwitchMaterial=839;  EDefs[689].altTexIndex=841;  EDefs[689].glowIndex=0;  EDefs[689].altGlowIndex=840;  EDefs[689].changeTexOnActive=true; EDefs[689].blinkTexOnActive=true;  EDefs[689].collider=COLTYPE_BOX;  EDefs[689].colliderCenter=(Vector3){-0.0243553f,0.0f,0.000004883f};  EDefs[689].colliderSize=(Vector3){0.0476318f,0.64f,0.64f};  EDefs[689].colliderMeshIndex=U16_MAX; 
    /*690 func_switch3*/         EDefs[690].modelIndex=611;  EDefs[690].texIndex=842;  EDefs[690].altTexIndex=844;  EDefs[690].glowIndex=0;  EDefs[690].altGlowIndex=843;  EDefs[690].changeTexOnActive=true;  EDefs[690].collider=COLTYPE_BOX;  EDefs[690].colliderCenter=(Vector3){-0.02285008f,0.000053061f,-0.000056993f};  EDefs[690].colliderSize=(Vector3){0.02f,0.32f,0.32f};  EDefs[690].colliderMeshIndex=U16_MAX; 
    /*691 func_switch4*/         EDefs[691].modelIndex=612;  EDefs[691].texIndex=846;  EDefs[691].collider=COLTYPE_BOX;  EDefs[691].colliderCenter=(Vector3){0.06f,0.0f,0.0f};  EDefs[691].colliderSize=(Vector3){0.2f,0.64f,0.64f};  EDefs[691].colliderMeshIndex=U16_MAX; 
    /*692 func_switch5*/         EDefs[692].modelIndex=614;  EDefs[692].texIndex=848;  EDefs[692].collider=COLTYPE_BOX;  EDefs[692].colliderCenter=(Vector3){0.0f,0.0f,0.0f};  EDefs[692].colliderSize=(Vector3){0.64f,0.64f,0.08f};  EDefs[692].colliderMeshIndex=U16_MAX; 
    /*693 func_switch5broken*/   EDefs[693].modelIndex=613;  EDefs[693].texIndex=847;  EDefs[693].collider=COLTYPE_BOX;  EDefs[693].colliderCenter=(Vector3){0.0f,0.0f,0.0f};  EDefs[693].colliderSize=(Vector3){0.64f,0.64f,0.08f};  EDefs[693].colliderMeshIndex=U16_MAX; 
    /*694 func_switch7*/         EDefs[694].modelIndex=612;  EDefs[694].texIndex=854;  EDefs[694].collider=COLTYPE_BOX;  EDefs[694].colliderCenter=(Vector3){1.523325f,0.0f,0.0f};  EDefs[694].colliderSize=(Vector3){0.2008026f,0.64f,0.64f};  EDefs[694].colliderMeshIndex=U16_MAX; 
    /*695 func_switch8*/         EDefs[695].modelIndex=616;  EDefs[695].texIndex=856;  EDefs[695].altTexIndex=858;  EDefs[695].glowIndex=855;  EDefs[695].altGlowIndex=857;  EDefs[695].changeTexOnActive=true;  EDefs[695].collider=COLTYPE_BOX;  EDefs[695].colliderCenter=(Vector3){-0.04f,0.0f,0.0001220703f};  EDefs[695].colliderSize=(Vector3){0.08f,0.64f,0.64f};  EDefs[695].colliderMeshIndex=U16_MAX; 
    /*696 func_switchbroken1*/   EDefs[696].modelIndex=617;  EDefs[696].texIndex=618; 
    /*697 clip_npc*/             EDefs[697].collider=COLTYPE_BOX;  EDefs[697].colliderCenter=(Vector3){1.005016f,0.0f,0.0f};  EDefs[697].colliderSize=(Vector3){2.010033f,16.0f,16.0f};  EDefs[697].colliderMeshIndex=U16_MAX;
    /*698 clip_objects*/         EDefs[698].collider=COLTYPE_BOX;  EDefs[698].colliderCenter=(Vector3){0.0f,0.0f,0.0f};  EDefs[698].colliderSize=(Vector3){2.56f,2.56f,2.56f};  EDefs[698].colliderMeshIndex=U16_MAX;
    /*699 logic_relay*/
    /*700 logic_branch*/
    /*701 logic_timer*/
    /*702 logic_spawner*/
    /*703 info_teleport_destination*/
    /*704 prop_debris_panel*/
    /*705 info_cyborgconversion*/
    /*706 info_elev_destination*/
    /*707 info_email*/
    /*708 info_gameend*/
    /*709 info_message*/
    /*710 info_mission*/
    /*711 info_note*/
    /*712 info_playsound*/
    /*713 info_ressurection_point*/
    /*714 info_screenshake*/
    /*715 info_spawnpoint*/
    /*716 fx_reverbzone*/
    /*717 ef_cyber_ice*/ EDefs[717].collider=COLTYPE_SPH;  EDefs[717].colliderCenter=(Vector3){0.0f,0.004354001f,-0.014725f};  EDefs[717].colliderSize=(Vector3){1.0f,0.0f,0.0f};  EDefs[717].colliderMeshIndex=U16_MAX; 
    /*718 ef_fragexplosion*/
    /*719 ef_line_sparqbeam*/
    /*720 ef_mist*/
    /*721 ef_particle_bloodspurtsmall*/
    /*722 ef_particle_bloodspurtsmallgreen*/
    /*723 ef_particle_bloodspurtsmallyellow*/
    /*724 ef_particle_bloodspurttiny*/
    /*725 ef_particle_camerahit*/
    /*726 ef_particle_darthit*/
    /*727 ef_particle_sec2muzburst*/
    /*728 ef_particle_sec2rotmuzburst*/
    /*729 ef_particle_sparksmall*/
    /*730 ef_particle_sparksmallblue*/
    /*731 ef_particle_sparqhit*/
    /*732 ef_sparkspits*/
    /*733 ef_spraydrips*/
    /*734 ef_steam*/
    /*735 env_sparksmall*/
    /*736 TargetIDInstance*/
    /*737 prop_papers01*/ EDefs[737].modelIndex=484;  EDefs[737].texIndex=580; 
    /*738 prop_papers02*/ EDefs[738].modelIndex=485;  EDefs[738].texIndex=580; 
    /*739 ef_particle_blasterhit*/
    /*740 ef_particle_ionhit*/
    /*741 us_puz_panel_blue_wire*/       EDefs[741].modelIndex=0;  EDefs[741].texIndex=604;  EDefs[741].numclips=3;  EDefs[741].animationNum=43; 
    /*742 us_puz_panel_brown_wire*/      EDefs[742].modelIndex=0;  EDefs[742].texIndex=631;  EDefs[742].numclips=3;  EDefs[742].animationNum=43; 
    /*743 us_puz_panel_gray_wire*/       EDefs[743].modelIndex=0;  EDefs[743].texIndex=634;  EDefs[743].numclips=3;  EDefs[743].animationNum=43; 
    /*744 us_puz_panel_red_wire*/        EDefs[744].modelIndex=0;  EDefs[744].texIndex=625;  EDefs[744].numclips=3;  EDefs[744].animationNum=43; 
    /*745 us_puz_panel_teal_wire*/       EDefs[745].modelIndex=0;  EDefs[745].texIndex=601;  EDefs[745].numclips=3;  EDefs[745].animationNum=43; 
    /*746 weapon_grenadeenergmine_live*/ EDefs[746].modelIndex=169;  EDefs[746].texIndex=852; 
    /*747 decal_logo_storage*/           EDefs[747].modelIndex=77;  EDefs[747].texIndex=169;
    /*748 light_animated*/
    /*749 generic_transform*/
    /*750 chunk_crate_impenetrable2*/    EDefs[750].modelIndex=61;  EDefs[750].texIndex=147; 
    /*751 chunk_crate_impenetrable3*/    EDefs[751].modelIndex=61;  EDefs[751].texIndex=148; 
    /*752 chunk_crate_impenetrable4*/    EDefs[752].modelIndex=61;  EDefs[752].texIndex=149; 
    /*753 npc_sec3_bot*/                 EDefs[753].modelIndex=681;  EDefs[753].texIndex=553; 
    /*754 prop_shieldgenerator*/         EDefs[754].modelIndex=143;  EDefs[754].texIndex=316; 
    /*755 unused*/
    /*756 ef_particle_leafburst*/
    /*757 ef_particle_mutationburst*/
    /*758 ef_particle_graytationburst*/
    /*759 unused*/
    /*760 unused*/
    /*761 unused*/
    /*762 unused*/
    /*763 unused*/
    /*764 unused*/
    /*765 unused*/
    /*766 unused*/
    /*767 player*/
    for (i32 i = 0; i < MAX_ENTITIES; i++) {
        if (EDefs[i].index == U16_MAX) continue;
        
        if (!EDefs[i].layer) EDefs[i].layer = Layer_Default;
        flag_set(&EDefs[i].entflags,EF_ACTIVE,true); // Individual value setting to allow mods to set custom starting flags themselves. (or here too if they want, tis your oyster).
        flag_set(&EDefs[i].entflags,EF_RIGIDBODY,ConstIndexIsDynamicObject(EDefs[i].index));
        if (EDefs[i].cardchunk) {
            EDefs[i].lodIndex = GEOMETRY_LOD_CARD_MODEL_IDX;
            EDefs[i].collider = COLTYPE_BOX;
            EDefs[i].colliderCenter.y = 1.44f;
            EDefs[i].colliderSize = (Vector3){2.56f,0.32f,2.56f};
        }
        
        EDefs[i].currentFrameFinished = Eng_Global->pauseRelativeTime + 0.1;
        if (ConstIndexIsButtonSwitch(EDefs[i].index)) { EDefs[i].lockedMessageLingdex = 193; EDefs[i].tickTime = 1.5; } // ButtonSwitch
    }
}

u16 AddInstance(u16 entIdx, Vector3 pos) {
    if (entIdx >= MAX_ENTITIES) { DualLogError("\nEntity index when loading non-light entity was %d, exceeds max defined entity count of %d, skipped\n",entIdx,MAX_ENTITIES); return INSTANCE_COUNT; }
    
    u16 i = Eng_Global->loadedInstances;
    Entity* e = &Eng_Global->instances[i];
    e->index = entIdx;
    SetPosition(e,pos,true); // Marks dirty internally, using true to force as if twere teleported.
    if (ConstIndexIsNPC(entIdx)) InitializeAIAfterLoad(i);
    e->cardchunk = EDefs[entIdx].cardchunk;
    e->modelIndex = EDefs[entIdx].modelIndex;
    e->colliderMeshIndex = EDefs[entIdx].colliderMeshIndex;
    e->numclips = EDefs[entIdx].numclips;
    e->animationNum = EDefs[entIdx].animationNum;
    e->texIndex = EDefs[entIdx].texIndex;
    e->glowIndex = EDefs[entIdx].glowIndex >= MAX_VALID_TEXTURE ? 0 : EDefs[entIdx].glowIndex;
    e->specIndex = EDefs[entIdx].specIndex >= MAX_VALID_TEXTURE ? 0 : EDefs[entIdx].specIndex;
    e->normIndex = EDefs[entIdx].normIndex >= MAX_VALID_TEXTURE ? 0 : EDefs[entIdx].normIndex;
    e->lodIndex = EDefs[entIdx].lodIndex;
    e->kinematic = EDefs[entIdx].kinematic;
    flag_set(&e->entflags,EF_RIGIDBODY,EDefs[entIdx].entflags & EF_RIGIDBODY);
    flag_set(&e->entflags,EF_NO_SHADOWS, EDefs[entIdx].entflags & EF_NO_SHADOWS);
    e->collider = EDefs[entIdx].collider; e->colliderCenter = EDefs[entIdx].colliderCenter; e->colliderSize = EDefs[entIdx].colliderSize;
    e->mass = EDefs[entIdx].mass > 0.0f ? EDefs[entIdx].mass : 1.0f; e->angularDrag = EDefs[entIdx].angularDrag > 0.0f ? EDefs[entIdx].angularDrag : 0.05f;
    e->gravity = EDefs[entIdx].gravity > 0.0f ? EDefs[entIdx].gravity : 1.0f;
    Eng_Global->instances[i].lockedMessageLingdex = EDefs[entIdx].lockedMessageLingdex;
    Eng_Global->loadedInstances++;
    return i;
}

void DeleteInstance(u16 i) {
    if (i <= PLAYER2 || i >= Eng_Global->loadedInstances) return; // Don't delete null ent, player 1, nor player 2 or already empty slots.

    MemCpyFromBtoAForNBytes(&Eng_Global->instances[i],&Eng_Global->instances[Eng_Global->loadedInstances - 1],sizeof(Entity));
    --Eng_Global->loadedInstances; // Shift final marker.  It's history!
}

static const Color fogLUT[] = {
    {0.3207547f, 0.29200783f, 0.29200783f, 0.07f},
    {0.34509805f,0.38431373f, 0.49019608f, 0.055f},
    {0.47058824f,0.3882353f,  0.3928334f,  0.05f},
    {0.32941177f,0.29411766f, 0.2509804f,  0.065f},
    {0.3882353f, 0.452415f,   0.47058824f, 0.075f},
    {0.3882353f, 0.4117647f,  0.47058824f, 0.03f},
    {0.3f,       0.24f,       0.33f,       0.07f},
    {0.38679248f,0.3471719f,  0.3302332f, 0.07f},
    {0.44708973f,0.45681614f, 0.4811321f, 0.04f},
    {0.4056604f, 0.3992963f,  0.36930403f,0.05f},
    {0.48235294f,0.58431375f, 0.5176471f, 0.04f},
    {0.52872473f,0.58431375f, 0.48235294f,0.04f},
    {0.48235294f,0.58431375f, 0.5176471f, 0.05f},
    {0.0f,       0.0f,        0.0f,       0.005f},
};

typedef struct { Vector2 worldMin; Vector2 worldMax; } LevelBounds;
static const LevelBounds levelBoundsTable[13] = {
    {{-34.8000f, -50.2000f}, {0.0f, 0.0f}},/*Level 0*/  {{-51.2400f, -61.5200f}, {0.0f, 0.0f}},/*Level 1*/  {{-43.5600f, -53.7800f}, {0.0f, 0.0f}},/*Level 2*/
    {{-48.7060f, -48.6860f}, {0.0f, 0.0f}},/*Level 3*/  {{-26.9020f, -51.2272f}, {0.0f, 0.0f}},/*Level 4*/  {{-44.8022f, -52.4800f}, {0.0f, 0.0f}},/*Level 5*/
    {{-63.3800f, -69.1233f}, {0.0f, 0.0f}},/*Level 6*/  {{-64.3389f, -79.4544f}, {0.0f, 0.0f}},/*Level 7*/  {{-41.1856f, -41.4272f}, {0.0f, 0.0f}},/*Level 8*/
    {{-48.9439f, -66.4706f}, {0.0f, 0.0f}},/*Level 9*/  {{-21.5394f, -37.2372f}, {0.0f, 0.0f}},/*Level 10*/ {{-24.6172f, -25.7794f}, {0.0f, 0.0f}},/*Level 11*/
    {{-15.4900f, -27.9400f}, {0.0f, 0.0f}} /*Level 12*/
};
//                                            R       1      2      3       4       5       6       7       8       9      10      11     12      13
static const float levelFarPlane[14] = { 56.32f, 56.32f, 51.2f, 51.2f, 40.96f, 58.88f, 79.36f, 56.32f, 69.12f, 53.76f,  51.2f,  51.2f, 38.4f, 71.68f};

extern u16 headmountedLanternLight;
Entity entsFromFile[INSTANCE_COUNT];
Light lightsFromFile[LIGHT_COUNT];
LightAnimation lanimsFromFile[LIGHT_COUNT];
char lineSpace[LINE_LEN_MAX];
char initialLine[LINE_LEN_MAX];
MOD_TO_ENGINE void LoadLevelMod(u8 curlevel) {
    Eng_Global->levelCurrentlyLoading = true;
    Eng_Global->currentLevel = curlevel;
    Eng_Global->loadedInstances = 3; // 0 == NULL, 1 == Player1, 2 == Player2
    Eng_Global->loadedLights = 0;
    Eng_Global->worldMin_x = levelBoundsTable[curlevel].worldMin.x;
    Eng_Global->worldMin_z = levelBoundsTable[curlevel].worldMin.y;
    if (curlevel == 1) {
        AddCamView((Vector3){-19.2301f,-42.6604f,-49.7453f},(Quaternion){0.2375f,0.0008f,-0.0002f,0.9713f},75u,256u,256u,2.21f,11.5f);
        AddCamView((Vector3){7.664583f,-44.88017f,-14.26742f},(Quaternion){0.0f,0.9999f,0.0129f,0.0f},60u,256u,256u,2.192f,20.6f);
    }

    Eng_Global->farPlane = levelFarPlane[curlevel];
    Eng_Global->worldMin_x -= CELL_SIZE;
    Eng_Global->worldMin_z -= CELL_SIZE;
    Eng_Global->voxelMinCenterX = Eng_Global->worldMin_x + VOXEL_HALF;
    Eng_Global->voxelMinCenterZ = Eng_Global->worldMin_z + VOXEL_HALF;
    if (curlevel >= Eng_Global->numLevels) { DualLogError("Cannot load world geometry, level number %d out of bounds 0 to %d\n", curlevel, Eng_Global->numLevels - 1); return; }

    for (u16 idx = START_INDEX_LEVEL_INSTANCES; idx < INSTANCE_COUNT; idx++) InitializeEntity(&Eng_Global->instances[idx]);
    MemSetToVForNBytes(entsFromFile,0,INSTANCE_COUNT * sizeof(Entity));
    MemSetToVForNBytes(lightsFromFile,0,LIGHT_COUNT * sizeof(Light));
    MemSetToVForNBytes(lanimsFromFile,0,LIGHT_COUNT * sizeof(LightAnimation));
    MemSetToVForNBytes(lineSpace,0,LINE_LEN_MAX * sizeof(char));
    MemSetToVForNBytes(initialLine,0,LINE_LEN_MAX * sizeof(char));
    for (int i = 0; i < LIGHT_COUNT; ++i) lightsFromFile[i].lflags = LIGHT_AND_SHADOW_ON;
    u32 lineNum = 0;
    i32 entCount = -1;  // incremented to 0 on first entity line
    i32 lightsIdx = -1; // incremented to 0 on first light line
    char* line = &lineSpace[0];
    char firstKeyCheck[11];
    while (GetLevelFileNextStringUpToNewlineOrEOF(lineSpace,LINE_LEN_MAX)) {
        size_t len = GetStringLength(lineSpace);
        while (len && (lineSpace[len - 1] == '\n' || lineSpace[len - 1] == '\r')) lineSpace[--len] = '\0';
        line = lineSpace;
        StringFormat(initialLine,sizeof(initialLine),"%s",line);
        MemCpyFromBtoAForNBytes(firstKeyCheck,line,10); firstKeyCheck[10] = '\0'; lineNum++;
        bool isLight = !StringsEqual(firstKeyCheck,"constIndex");
        if (isLight) { lightsIdx++; if (lightsIdx >= LIGHT_COUNT){DualLogError("Too many lights %u in level%d.txt!\n",lightsIdx,curlevel);continue;} }
        else { entCount++; if (entCount >= INSTANCE_COUNT){DualLogError("Too many instances %u in level%d.txt!\n",entCount,curlevel);continue;} }
        
        bool activeStateRead = false;
        while (line[0] != '\0') {
            char* pipe = StringFindFirstCharWithin(line, '|');
            char* kvString = line;
            if (pipe) { *pipe = '\0'; line = pipe + 1; }
            else { line += GetStringLength(line); }

            if (kvString[0] == '\0' || StringFindFirstCharWithin(kvString, ':') == NULL) continue;
            char* colon = StringFindFirstCharWithin(kvString, ':');
            if (colon[1] == '\0') continue;
            *colon = '\0';
            char* key   = kvString;
            char* value = colon + 1;
            if (!key) { DualLogError("Invalid key-value pair at line %u: %s\n", lineNum, initialLine); continue; }

            char trimmed_key[64];
            char trimmed_value[256];
            StringFormat(trimmed_key,   sizeof(trimmed_key),   "%s", key);
            StringFormat(trimmed_value, sizeof(trimmed_value), "%s", value);
            trimmed_key[sizeof(trimmed_key) - 1]     = '\0';
            trimmed_value[sizeof(trimmed_value) - 1] = '\0';

            if (isLight) {
                LoadFieldIntoLight((char*)&trimmed_key, (char*)&trimmed_value, initialLine, lineNum, &lightsFromFile[lightsIdx], &lanimsFromFile[lightsIdx], lightsIdx);
            } else {
                Entity* inst = &entsFromFile[entCount];
                     if (StringsEqual(trimmed_key,"constIndex"))      inst->index = parse_numberu16(trimmed_value, initialLine, lineNum);
                else if (StringsEqual(trimmed_key,"localPosition.x")) inst->position.x = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsEqual(trimmed_key,"localPosition.y")) inst->position.y = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsEqual(trimmed_key,"localPosition.z")) inst->position.z = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsEqual(trimmed_key,"localRotation.x")) inst->rotation.x = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsEqual(trimmed_key,"localRotation.y")) inst->rotation.y = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsEqual(trimmed_key,"localRotation.z")) inst->rotation.z = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsEqual(trimmed_key,"localRotation.w")) inst->rotation.w = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsEqual(trimmed_key,"localScale.x"))    inst->scale.x = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsEqual(trimmed_key,"localScale.y"))    inst->scale.y = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsEqual(trimmed_key,"localScale.z"))    inst->scale.z = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsEqual(trimmed_key,"go.activeSelf"))   { activeStateRead = true; flag_set(&inst->entflags, EF_ACTIVE, parse_bool(trimmed_value,initialLine,lineNum)); }
                else if (StringsEqual(trimmed_key,"amount"))          inst->amount = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsEqual(trimmed_key,"resetTime"))       inst->resetTime = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsEqual(trimmed_key,"minSecurityLevel"))inst->minSecurityLevel = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsEqual(trimmed_key,"damageOnUse"))     inst->damage = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsEqual(trimmed_key,"target"))          StringCopyInto_A_From_B(inst->target, trimmed_value, TARGET_STRING_LENGTH);
                else if (StringsEqual(trimmed_key,"argvalue"))        StringCopyInto_A_From_B(inst->argvalue, trimmed_value, TARGET_STRING_LENGTH);
                else if (StringsEqual(trimmed_key,"targetname"))      StringCopyInto_A_From_B(inst->targetname, trimmed_value, TARGET_STRING_LENGTH);
                else if (StringsEqual(trimmed_key,"securityThreshhold") || StringsEqual(trimmed_key,"securityThreshold")) inst->securityThreshold = parse_numberu8(trimmed_value, initialLine, lineNum);
                else if (StringsEqual(trimmed_key,"messageIndex"))    inst->messageIndex = parse_numberi16(trimmed_value,initialLine,lineNum);
                else if (StringsEqual(trimmed_key,"delay"))           inst->delay = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsEqual(trimmed_key,"locked"))          flag_set(&inst->entflags, EF_LOCKED, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"active"))          inst->active = parse_bool(trimmed_value,initialLine,lineNum);
                else if (StringsEqual(trimmed_key,"alternateOn"))     inst->alternateOn = parse_bool(trimmed_value,initialLine,lineNum);
                else if (StringsEqual(trimmed_key,"onlyTargetOnce"))  inst->onlyOnce = parse_bool(trimmed_value,initialLine,lineNum);
                else if (StringsEqual(trimmed_key,"targetAlreadyDone")) inst->targetAlreadyDone = parse_bool(trimmed_value,initialLine,lineNum);
                else if (StringsEqual(trimmed_key,"stayOpen"))        inst->stayOpen = parse_bool(trimmed_value,initialLine,lineNum);
                else if (StringsEqual(trimmed_key,"startOpen"))       inst->startOpen = parse_bool(trimmed_value,initialLine,lineNum);
                else if (StringsEqual(trimmed_key,"ajar"))            inst->ajar = parse_bool(trimmed_value,initialLine,lineNum);
                else if (StringsEqual(trimmed_key,"ajarPercentage"))  inst->ajarPercentage = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsEqual(trimmed_key,"useTimeDelay"))    inst->useTimeDelay = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsEqual(trimmed_key,"blocked"))         inst->blocked = parse_bool(trimmed_value,initialLine,lineNum);
                else if (StringsEqual(trimmed_key,"timeBeforeLasersOn")) inst->timeBeforeLasersOn = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsEqual(trimmed_key,"toggleLasers"))    inst->toggleLasers = parse_bool(trimmed_value,initialLine,lineNum);
                else if (StringsEqual(trimmed_key,"targettingOnlyUnlocks")) inst->targettingOnlyUnlocks = parse_bool(trimmed_value,initialLine,lineNum);
                else if (StringsEqual(trimmed_key,"changeLayerOnOpenClose")) inst->changeLayerOnOpenClose = parse_bool(trimmed_value,initialLine,lineNum);
                else if (StringsEqual(trimmed_key,"useFinished"))     inst->useFinished = parse_float(trimmed_value, initialLine, lineNum) + Eng_Global->pauseRelativeTime;
                else if (StringsEqual(trimmed_key,"waitBeforeClose")) inst->waitBeforeClose = parse_float(trimmed_value,initialLine, lineNum) + Eng_Global->pauseRelativeTime;
                else if (StringsEqual(trimmed_key,"lasersFinished"))  inst->lasersFinished = parse_float(trimmed_value,initialLine, lineNum) + Eng_Global->pauseRelativeTime;
                else if (StringsEqual(trimmed_key,"changeMatOnActive")) inst->changeTexOnActive = parse_bool(trimmed_value,initialLine, lineNum);
                else if (StringsEqual(trimmed_key,"blinkWhenActive")) inst->blinkTexOnActive = parse_bool(trimmed_value,initialLine, lineNum);
                else if (StringsEqual(trimmed_key,"doorOpen"))        flag_set(&inst->ioflags,TARG_IOFLAGS_DOOROPEN,parse_bool(trimmed_value, initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"doorOpenIfUnlocked")
                      || StringsEqual(trimmed_key,"doorToggle"))      flag_set(&inst->ioflags,TARG_IOFLAGS_DOOROPENIFUNLOCKED, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"doorClose"))       flag_set(&inst->ioflags,TARG_IOFLAGS_DOORCLOSE,parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"doorLock") 
                      || StringsEqual(trimmed_key,"lockElevatorPad")) flag_set(&inst->ioflags,TARG_IOFLAGS_LOCK,parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"doorUnlock")
                      || StringsEqual(trimmed_key,"unlockSwitch")
                      || StringsEqual(trimmed_key,"unlockElevatorPad")
                      || StringsEqual(trimmed_key,"unlockKeycodePad")
                      || StringsEqual(trimmed_key,"unlockPuzzlePad")) flag_set(&inst->ioflags,TARG_IOFLAGS_UNLOCK,parse_bool(trimmed_value,initialLine,lineNum));
                
                else if (StringsEqual(trimmed_key,"switchTrigger"))   flag_set(&inst->ioflags,TARG_IOFLAGS_SWITCHTRIGGER,parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"tripTrigger"))     flag_set(&inst->ioflags,TARG_IOFLAGS_TRIPTRIGGER,parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"forceBridgeActivate")) flag_set(&inst->ioflags,TARG_IOFLAGS_FBRIDGE_ACTIVATE,parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"forceBridgeDeactivate")) flag_set(&inst->ioflags,TARG_IOFLAGS_FBRIDGE_DEACTIVATE,parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"forceBridgeToggle")) flag_set(&inst->ioflags,TARG_IOFLAGS_FBRIDGE_TOGGLE,parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"gravityLiftToggle")) flag_set(&inst->ioflags,TARG_IOFLAGS_GRAVLIFT_TOGGLE,parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"textureChangeToggle")) flag_set(&inst->ioflags,TARG_IOFLAGS_TEXTURE_CHG_TOGGLE,parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"lightOn"))         flag_set(&inst->ioflags,TARG_IOFLAGS_LIGHT_ON,parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"lightOff"))        flag_set(&inst->ioflags,TARG_IOFLAGS_LIGHT_OFF,parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"lightToggle"))     flag_set(&inst->ioflags,TARG_IOFLAGS_LIGHT_TOGGLE,parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"funcwallMove"))    flag_set(&inst->ioflags,TARG_IOFLAGS_FUNCWALL_MOVE,parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"missionBitOn"))    flag_set(&inst->ioflags,TARG_IOFLAGS_MISSION_BIT_ON,parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"missionBitOff"))   flag_set(&inst->ioflags,TARG_IOFLAGS_MISSION_BIT_OFF,parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"missionBitToggle")) flag_set(&inst->ioflags,TARG_IOFLAGS_MISSION_BIT_TOGGLE,parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"switchLockToggle")) flag_set(&inst->ioflags,TARG_IOFLAGS_SWITCH_LOCK_TOGGLE, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"GOSetActive"))     flag_set(&inst->ioflags,TARG_IOFLAGS_INST_ACTIVATE, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"GOSetDeactive"))   flag_set(&inst->ioflags,TARG_IOFLAGS_INST_DEACTIVATE, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"GOToggleActive"))  flag_set(&inst->ioflags,TARG_IOFLAGS_INST_TOGGLE, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"disableThisGOOnAwake")) flag_set(&inst->ioflags,TARG_IOFLAGS_DISABLE_ON_AWAKE, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"playSoundOnce"))   flag_set(&inst->ioflags,TARG_IOFLAGS_PLAY_SOUND_ONCE, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"stopSound"))       flag_set(&inst->ioflags,TARG_IOFLAGS_STOP_SOUND, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"startFlashingMaterials")) flag_set(&inst->ioflags,TARG_IOFLAGS_START_FLASHING_TEX, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"stopFlashingMaterials")) flag_set(&inst->ioflags,TARG_IOFLAGS_STOP_FLASHING_TEX, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"branchFlip"))      flag_set(&inst->ioflags,TARG_IOFLAGS_BRANCH_FLIP, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"branchFlipOnly"))  flag_set(&inst->ioflags,TARG_IOFLAGS_BRANCH_FLIPONLY, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"resourceFolder") && *trimmed_value) StringCopyInto_A_From_B(inst->texAnimResourceFolder,trimmed_value,TARGET_STRING_LENGTH);
                else if (StringsEqual(trimmed_key,"frameDelay"))      inst->tickTime = (double)parse_float(trimmed_value,initialLine,lineNum);
                else if (StringsEqual(trimmed_key,"randomFrame"))     inst->texAnimRandom = parse_bool(trimmed_value,initialLine,lineNum);
                else if (StringsEqual(trimmed_key,"reverseSequence")) inst->texAnimInReverse = parse_bool(trimmed_value,initialLine,lineNum);
                else if (StringsEqual(trimmed_key,"messageLingdex"))  inst->messageLingdex = parse_numberi16(trimmed_value,initialLine,lineNum);
                else if (StringsEqual(trimmed_key,"lockedMessageLingdex")) inst->lockedMessageLingdex = parse_numberi16(trimmed_value, initialLine, lineNum);
                else if (StringsEqual(trimmed_key,"SFXIndex"))        inst->SFXIndex = (i16)parse_numberi16(trimmed_value, initialLine, lineNum);
                else if (StringsEqual(trimmed_key,"requiredAccessCard")) inst->requiredAccessCard = parse_numberi8(trimmed_value, initialLine, lineNum);
                else if (StringsEqual(trimmed_key,"doorOpenState"))   inst->doorOpen = parse_numberu8(trimmed_value, initialLine, lineNum);
            }
        }

        // Store activeStateRead alongside the parsed entity so the commit pass can use it.  Reuse a spare field or parallel array — here we use a bit in entflags as a sentinel.
        if (!isLight && !activeStateRead) flag_set(&entsFromFile[entCount].entflags, EF_ACTIVE, true); // Default active if not specified
    }

    i32 totalEnts = entCount + 1;
    for (i32 e=0;e<totalEnts;++e) {
        Entity* src = &entsFromFile[e];
        u16 entIdx = src->index;
        u16 parent = AddInstance(entIdx,src->position);
        Entity* par = &Eng_Global->instances[parent];
        par->lastPosition          = par->position;
        par->rotation              = src->rotation;
        par->scale                 = src->scale;
        par->entflags             |= src->entflags; // bitor `|` since AddInstance already set flags from entity definitions.
        par->ioflags               = src->ioflags;
        par->amount                = src->amount;
        par->resetTime             = src->resetTime;
        par->minSecurityLevel      = src->minSecurityLevel;
        par->damage                = src->damage;
        par->delay                 = src->delay;
        par->active                = src->active;
        par->alternateOn           = src->alternateOn;
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
        par->changeTexOnActive     = src->changeTexOnActive;
        par->blinkTexOnActive      = src->blinkTexOnActive;
        par->securityThreshold     = src->securityThreshold;
        par->tickTime              = src->tickTime;
        par->texAnimRandom         = src->texAnimRandom;
        par->texAnimInReverse      = src->texAnimInReverse;
        par->messageLingdex        = src->messageLingdex;
        StringCopyInto_A_From_B(par->target,src->target,TARGET_STRING_LENGTH);
        StringCopyInto_A_From_B(par->argvalue,src->argvalue,TARGET_STRING_LENGTH);
        StringCopyInto_A_From_B(par->targetname,src->targetname,TARGET_STRING_LENGTH);
        if (StringsEqual(par->target,"lev1door1")) DualLog("Switch that targets lev1door1 loaded with ioflags:%u\n",par->ioflags);
        StringCopyInto_A_From_B(par->texAnimResourceFolder,src->texAnimResourceFolder,TARGET_STRING_LENGTH);
        if (ConstIndexIsPortalBlockingDoor(entIdx)) AddDoorPortal(entIdx, parent); // Only at load, not in AddInstance
        if (entIdx == 525) { // prop_console01
            Vector3 ofs1 = GetEntityLocalSpawnPointFromUnrotatedOffsetVector(par,(Vector3){5.81f,2.29f,38.05f-38.3552f});
            Vector3 ofs2 = GetEntityLocalSpawnPointFromUnrotatedOffsetVector(par,(Vector3){-10.1f,0.9f,18.21f-38.3552f});
            Light lit1 = (Light){.pos=ofs1,.col=(Color3){0.3531f,0.4837f,0.6509f},.range=1.85f,.intensity=0.7f,.maxIntensity=0.7f,.minIntensity=0.0f,.spotAng=0.0f,.spotDir=QUAT_IDENTITY,.lflags=LIGHT_AND_SHADOW_ON};
            Light lit2 = (Light){.pos=ofs2,.col=(Color3){0.3561f,0.3561f,0.8970f},.range=2.0f,.intensity=1.12f,.maxIntensity=1.12f,.minIntensity=0.0f,.spotAng=0.0f,.spotDir=QUAT_IDENTITY,.lflags=LIGHT_AND_SHADOW_ON};
            LightAnimation lam={0};
            par->texAnimLight  = AddLight(&lit1,&lam);
            par->texAnimLight2 = AddLight(&lit2,&lam);
        } else if (entIdx == 309 || entIdx == 365 || entIdx == 369) par->position.y += 0.12f; // item_beaker || item_flask || item_testtube: Move up to account for CG mod (origin moved vs Unity version)
        else if (entIdx == 279) { // chunk_screen
            Vector3 ofs1 = GetEntityLocalSpawnPointFromUnrotatedOffsetVector(par,(Vector3){0.0f,-0.08f,0.0f});
            Light lit1 = (Light){.pos=ofs1,.col=(Color3){0.909803922f,0.929411765f,1.0f},.range=3.2f,.intensity=1.575f,.maxIntensity=1.575f,.minIntensity=0.0f,.spotAng=0.0f,.spotDir=QUAT_IDENTITY,.lflags=LIGHT_AND_SHADOW_ON};
            LightAnimation lam={0};
            par->texAnimLight = AddLight(&lit1,&lam);
        } else if (par->index == 574) { // prop_healingbed
            Vector3 ofs1 = GetEntityLocalSpawnPointFromUnrotatedOffsetVector(par,(Vector3){0.5292511f,0.065,0.915f});
            Vector3 ofs2 = GetEntityLocalSpawnPointFromUnrotatedOffsetVector(par,(Vector3){-0.5317497f,0.065f,1.039f});
            Light lit1 = (Light){.pos=ofs1,.col=(Color3){0.0f,0.925490196f,0.082352941f},.range=3.0f,.intensity=0.72f,.maxIntensity=0.72f,.minIntensity=0.0f,.spotAng=0.0f,.spotDir=QUAT_IDENTITY,.lflags=LIGHT_AND_SHADOW_ON};
            Light lit2 = (Light){.pos=ofs2,.col=(Color3){0.0f,0.925490196f,0.082352941f},.range=3.0f,.intensity=0.72f,.maxIntensity=0.72f,.minIntensity=0.0f,.spotAng=0.0f,.spotDir=QUAT_IDENTITY,.lflags=LIGHT_AND_SHADOW_ON};
            LightAnimation lam={0};
            par->texAnimLight  = AddLight(&lit1,&lam);
            par->texAnimLight2 = AddLight(&lit2,&lam);
            par->textureAnimating = true; par->texAnimClip = 12; par->texFrame = 0;
            StringCopyInto_A_From_B(par->texAnimResourceFolder,"MedicalBed",TARGET_STRING_LENGTH);
        } else if (par->index == 746) { // weapon_grenadeenergmine_live
            par->textureAnimating = true; par->texAnimClip = 2; par->texFrame = 0;
        } else if (entIdx == 720) {
            /*u16 mist = */AddInstance(648,par->position); // ambient_mist
        } else if (entIdx == 733) {
            /*u16 pipewater = */AddInstance(649,par->position); // ambient_pipewater_loop
            /*u16 rain = */AddInstance(653,(Vector3){par->position.x,par->position.y - 1.26f,par->position.z}); // ambient_rain
        }
        
        if (par->texAnimResourceFolder[0] != '\0' && par->tickTime <= 0.01f) par->tickTime = 0.35f;
        TextureSequenceInit(parent,par->texAnimResourceFolder);
    }
    
    for (int i = 0; i < lightsIdx; ++i) { if (!(lightsFromFile[i].lflags & LSPOT)){lightsFromFile[i].spotAng=0.0f;} AddLight(&lightsFromFile[i],&lanimsFromFile[i]); } // Add all level lights
    if (curlevel == 1 || curlevel == 2 || curlevel == 5 || curlevel == 6 || curlevel == 7) { // Shield generators
        u16 shd1 = AddInstance(754, (Vector3){-51.30664f,  -47.42f,  56.42651f}); Eng_Global->instances[shd1].rotation = (Quaternion){0.0f,0.0f,0.0f,1.0f};
        u16 shd2 = AddInstance(754, (Vector3){ 71.5f,      -47.42f, -66.6f    }); Eng_Global->instances[shd2].rotation = (Quaternion){0.0f,0.0f,0.0f,1.0f};
        u16 shd3 = AddInstance(754, (Vector3){-51.306650f, -47.42f, -66.66652f}); Eng_Global->instances[shd3].rotation = (Quaternion){0.0f,0.0f,0.0f,1.0f};
        u16 shd4 = AddInstance(754, (Vector3){ 71.78664f,  -47.42f,  56.42651f}); Eng_Global->instances[shd4].rotation = (Quaternion){0.0f,0.0f,0.0f,1.0f};
    }

    Light hl = (Light){.pos=Eng_Global->instances[PLAYER1].position,.col=(Color3){1.0f,1.0f,1.0f},.range=11.52f,.lflags=LIGHTON,.intensity=0.0f,.minIntensity=0.0f,.maxIntensity=0.0f,.spotAng=0.0f,.spotDir=QUAT_IDENTITY};
    LightAnimation lam = {0};
    headmountedLanternLight = AddLight(&hl,&lam); lightsIdx++;
    Color c = fogLUT[curlevel]; c.a *= 3.8f;
    Eng_Global->fogColor = c;
}
